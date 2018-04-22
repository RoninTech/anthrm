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
 *  Compile with DJGPP:    gcc *.cpp -s -O2 -o hole.exe -lstdcxx -lpng -lz *.o
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

void Draw_Hole( unsigned char *buf, unsigned char du, unsigned char dv );

// this handles all the graphics
VGA *vga;

// our timing module
TIMER *timer;

// segment aligned buffer containing the texture
unsigned char *texdata;
// buffer containing the (u,v) pairs at each pixel
unsigned char *texcoord;

/*
 * position of the centre of the hole along the X axis
 */
static float get_x_pos( float f )
{
      return - 16 * sin(  f * M_PI / 256 );
};

/*
 * position of the centre of the hole along the Y axis
 */
static float get_y_pos( float f )
{
      return - 16 * sin(  f * M_PI / 256 );
};

/*
 * size of the hole
 */
static float get_radius( float f )
{
      return 128;
};

void Init_Hole()
{
//    cout << "Please wait..." << endl;
 // alloc memory to store 320*200 times u, v
    texcoord = new unsigned char[128000];
    long offs = 0;
 // precalc the (u,v) coordinates
    for (int j=-100; j<100; j++) {
        for (int i=-160; i<160; i++)
        {
          // get coordinates of ray that projects through this pixel
             float dx = (float)i / 200;
             float dy = (float)-j / 200;
             float dz = 1;
          // normalize them
             float d = 20/sqrt( dx*dx + dy*dy + 1 );
             dx *= d;
             dy *= d;
             dz *= d;
          // start interpolation at origin
             float x = 0;
             float y = 0;
             float z = 0;
          // set original precision
             d = 16;
          // interpolate along ray
             while (d>0)
             {
                // continue until we hit a wall
                   while (((x-get_x_pos(z))*(x-get_x_pos(z))+(y-get_y_pos(z))*(y-get_y_pos(z)) < get_radius(z)) && (z<1024))
                   {
                       x += dx;
                       y += dy;
                       z += dz;
                   };
                // reduce precision and reverse direction
                   x -= dx;  dx /= 2;
                   y -= dy;  dy /= 2;
                   z -= dz;  dz /= 2;
                   d -= 1;
             }
          // calculate the texture coordinates
             x -= get_x_pos(z);
             y -= get_y_pos(z);
             float ang = atan2( y, x ) * 256 / M_PI;
             unsigned char u = (unsigned char)ang;
             unsigned char v = (unsigned char)z;
          // store texture coordinates
             texcoord[offs] = u;
             texcoord[offs+1] = v;
             offs += 2;
        }
    }
 // set mode 320x200x8
    vga = new VGA;
 // load the texture
    TEXTURE *texture = new TEXTURE( "texture1.png");
    for (int i=0; i<256; i++)
    {
        vga->SetColour( i, texture->palette[i*3]>>2, texture->palette[i*3+1]>>2,  texture->palette[i*3+2]>>2 );
    }
 // get a segment aligned buffer
    texdata = new unsigned char[65536*2];
    while (((long)(texdata)&0xffff)!=0) texdata++;
 // and copy the texture into it
    memcpy( texdata, texture->location, 65536 );
    delete texture;
};

/*
 * non asm main loop
 * */


void Draw_Hole( unsigned char *buf, unsigned char du, unsigned char dv )
{
   long doffs = 0, soffs = 0;
   for (int j=0; j<200; j++) {
       for (int i=0; i<320; i++) {
      // load (u,v) and add displacement
         unsigned char u = texcoord[soffs] + du;
         unsigned char v = texcoord[soffs+1] + dv;
         buf[doffs] = texdata[(v<<8)+u];
         doffs++;
         soffs+=2;
       }
   }
}


/*
 * external asm loop, see DRAW.ASM
 */
//extern void Draw_Hole( unsigned char *buf, unsigned char du, unsigned char dv );

/*
 * the main program
 */
int main()
{
// prepare data
    Init_Hole();
// start the timer
    timer = new TIMER;
    long long startTime = timer->getCount(), frameCount = 0, currentTime;
// run the main loop
    while (!kbhit())
    {
          Draw_Hole( vga->page_draw, timer->getCount()>>16, timer->getCount()>>14 );
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
//    delete [] texdata;
//    delete [] texcoord;
// make sure every one knows about flipcode :)
/*    cout << "                             The Art Of" << endl;
    cout << "                         D E M O M A K I N G " << endl << endl;
    cout << "                       by Alex J. Champandard" << endl << endl;
    cout << "       Go to http://www.flipcode.com/demomaking for more information." << endl;
    cout << endl << FPS << " frames per second." << endl;*/
// we've finished!
    return 0;
}


