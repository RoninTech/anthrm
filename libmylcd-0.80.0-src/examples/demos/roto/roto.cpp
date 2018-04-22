/*
 *                          The Art Of
 *                      D E M O M A K I N G
 *
 *
 *                     by Alex J. Champandard
 *                          Base Sixteen
 *
 *
 *                http://www.flipcode.com/demomaking
 *
 *                This file is in the public domain.
 *                      Use at your own risk.
 *
 *
 *  Compile with DJGPP:    gcc *.cpp -s -O2 -o roto.exe -lstdcxx -lpng -lz
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <iostream>
#include <math.h>
#include "vga.h"
#include "timer.h"
#include "texture.h"

#undef main

// this handles all the graphics
VGA *vga;

// our timing module
TIMER *timer;

// the texture
TEXTURE *texture;

void Init()
{
 // set mode 320x200x8
    vga = new VGA;
 // load the texture
    texture = new TEXTURE( "texture2.png");
    for (int i=0; i<256; i++)
    {
        vga->SetColour( i, texture->palette[i*3]>>2, texture->palette[i*3+1]>>2,  texture->palette[i*3+2]>>2 );
    }
};

/*
 * render a textured screen with no blocks
 * all parameters are 16.16 fixed point
 */
void TextureScreen( int Ax, int Ay, int Bx, int By, int Cx, int Cy )
{
  // compute deltas
     int dxdx = (Bx-Ax)/320,
         dydx = (By-Ay)/320,
         dxdy = (Cx-Ax)/200,
         dydy = (Cy-Ay)/200;
     long offs = 0;
  // loop for all lines
     for (int j=0; j<200; j++)
     {
          Cx = Ax;
          Cy = Ay;
       // for each pixel
          for (int i=0; i<320; i++)
          {
           // get texel and store pixel
              vga->page_draw[offs] = texture->location[((Cy>>8)&0xff00)+((Cx>>16)&0xff)];
           // interpolate to get next texel in texture space
              Cx += dxdx;
              Cy += dydx;
              offs++;
          }
       // interpolate to get start of next line in texture space
          Ax += dxdy;
          Ay += dydy;
     }
}


/*
 * render a textured screen in 8x8 blocks
 */
void BlockTextureScreen( int Ax, int Ay, int Bx, int By, int Cx, int Cy )
{
  // compute global deltas, 40 blocks along X, 25 blocks along Y
     int dxdx = (Bx-Ax)/40,
         dydx = (By-Ay)/40,
         dxdy = (Cx-Ax)/25,
         dydy = (Cy-Ay)/25;
  // compute internal block deltas
     int dxbdx = (Bx-Ax)/320,
         dybdx = (By-Ay)/320,
         dxbdy = (Cx-Ax)/200,
         dybdy = (Cy-Ay)/200;
     long baseoffs = 0, offs;
  // for all blocks along Y
     for (int j=0; j<25; j++){
          Cx = Ax;
          Cy = Ay;

       // for all blocks along X
          for (int i=0; i<40; i++){
              offs = baseoffs;
           // set original position
              int Ex = Cx;
              int Ey = Cy;

           // for each line of 8 pixels in the block
              for (int y=0; y<8; y++){

               // set original position
                  int Fx = Ex;
                  int Fy = Ey;

               // for each pixel in the block
                  for (int x=0; x<8; x++){
                   // get texel and store pixel
                      vga->page_draw[offs] =
                      texture->location[((Fy>>8)&0xff00)+((Fx>>16)&0xff)];
                   // interpolate to find next texel
                      Fx += dxbdx;
                      Fy += dybdx;
                      offs++;
                  }
               // next line in the block
                  Ex += dxbdy;
                  Ey += dybdy;
                  offs += 312;
              }

           // next block
              baseoffs += 8;
              Cx += dxdx;
              Cy += dydx;
          }

       // next line of blocks
          baseoffs += 320*7;
          Ax += dxdy;
          Ay += dydy;
     }
}

/*
 * wrapper for the TextureScreen procedure
 */
void DoRotoZoom( float cx, float cy, float radius, float angle )
{
	
	//printf("%f %f %f %f\n", cx, cy, radius, angle);
	
   int x1 = (int)( 65536.0f * (cx + radius * cos( angle )) ),
       y1 = (int)( 65536.0f * (cy + radius * sin( angle )) ),
       x2 = (int)( 65536.0f * (cx + radius * cos( angle + 2.02458 )) ),
       y2 = (int)( 65536.0f * (cy + radius * sin( angle + 2.02458 )) ),
       x3 = (int)( 65536.0f * (cx + radius * cos( angle - 1.11701 )) ),
       y3 = (int)( 65536.0f * (cy + radius * sin( angle - 1.11701 )) );
   //BlockTextureScreen( x1, y1, x2, y2, x3, y3 );
   TextureScreen( x1, y1, x2, y2, x3, y3 );
}

/*
 * the main program
 */
int main()
{
// prepare texture and video mode
    Init();
// start the timer
    timer = new TIMER;
    long long startTime = timer->getCount(), frameCount = 0, currentTime;
// run the main loop
    while (!kbhit())
    {
          currentTime = timer->getCount();
       // visual performance test:
       // set background colour to white while we are drawing
//        vga->SetColour( 0, 63, 63, 63 );
       // draw the texture
          DoRotoZoom(
          2048 * sin((float)currentTime/21547814.0f),       // X centre coord
          2048 * cos((float)currentTime/18158347.0f),       // Y centre coord
          256.0f+192.0f*cos((float)currentTime/4210470.0f), // zoom coef
          (float)(currentTime)/731287.0f );                 // angle
       // restore background colour
//        vga->SetColour( 0, texture->palette[0]>>2, texture->palette[1]>>2,  texture->palette[2]>>2 );
       // flush to video ram
          vga->Update();
          frameCount++;
    }
// calculate total running time
    long long totalTime = timer->getCount() - startTime;
// and turn off the timer
    delete timer;
// now get FPS
    float FPS = (float)(frameCount)*1193180.0f / (float)(totalTime);
// return to text mode
    delete vga;
// free memory
    delete texture;
// make sure every one knows about flipcode :)
 /*   cout << "                             The Art Of" << endl;
    cout << "                         D E M O M A K I N G " << endl << endl;
    cout << "                       by Alex J. Champandard" << endl << endl;
    cout << "       Go to http://www.flipcode.com/demomaking for more information." << endl;
    cout << endl << FPS << " frames per second." << endl;*/
    printf("%.2ffps\n",FPS);
// we've finished!
    return 0;
}


