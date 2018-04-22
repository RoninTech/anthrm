/*
 *                          The Art Of
 *                      D E M O M A K I N G
 *
 *
 *                     by Alex J. Champandard
 *                          Base Sixteen
 *
 *
 *                  Modified by Paulo Pinto (Moondevil)
 *                         for using with SDL
 *
 *
 *                http://www.flipcode.com/demomaking
 *
 *                This file is in the public domain.
 *                      Use at your own risk.
 *
 *
 *                 Compile with DJGPP:  make plane
 *
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <math.h>
#include <conio.h>
#include "vga.h"
#include "timer.h"
#include "texture.h"
#include "vector.h"
#include "matrix.h"

using namespace std;

// this handles all the graphics
VGA *vga;

// NOTE: the FPS displayed is actually CORRECT!  Some people are mislead by
// the fact that they get 70 FPS on a PII.  It makes sense since we're only
// plotting 320x200 bytes, and the code is reasonnably simple.
TIMER *timer;

// the cool 256x256 texture we map onto the plane
TEXTURE *texture;

/*
 * draw a perspective correctly textured plane
 */
void DrawPlane( VECTOR Bp, VECTOR Up, VECTOR Vp )
{
  // compute the 9 magic constants
     float Cx = Up[1] * Vp[2] - Vp[1] * Up[2],
           Cy = Vp[0] * Up[2] - Up[0] * Vp[2],
        // the 240 represents the distance of the projection plane
        // change it to modify the field of view
           Cz = ( Up[0] * Vp[1] - Vp[0] * Up[1] ) * 240,
           Ax = Vp[1] * Bp[2] - Bp[1] * Vp[2],
           Ay = Bp[0] * Vp[2] - Vp[0] * Bp[2],
           Az = ( Vp[0] * Bp[1] - Bp[0] * Vp[1] ) * 240,
           Bx = Bp[1] * Up[2] - Up[1] * Bp[2],
           By = Up[0] * Bp[2] - Bp[0] * Up[2],
           Bz = ( Bp[0] * Up[1] - Up[0] * Bp[1] ) * 240;
  // only render the lower part of the plane, looks ugly above
     long offs = 105*320;
     for (int j=105; j<200; j++)
     {
      // compute the (U,V) coordinates for the start of the line
         float a = Az + Ay * (j-100) + Ax * -161,
               b = Bz + By * (j-100) + Bx * -161,
               c = Cz + Cy * (j-100) + Cx * -161,
               ic;
      // quick distance check, if it's too far reduce it
         if (fabs(c)>65536) ic = 1/c; else ic = 1/65536;
      // compute original (U,V)
         int u = (int)(a*16777216*ic),
             v = (int)(b*16777216*ic),
      // and the deltas we need to interpolate along this line
             du = (int)( 16777216 * Ax * ic ),
             dv = (int)( 16777216 * Bx * ic );
      // start the loop

	 
         for (int i=0; i<320; i++)
         {
         // load texel and store pixel
	   vga->page_draw[offs++] =
	     texture->location[((v>>8)&0xff00)+((u>>16)&0xff)];
         // interpolate
             u += du;
             v += dv;
         }
     }
}

/*
 * the main program
 */
int main()
{

 // load the texture
    texture = new TEXTURE( "texture2.png" );
    int i, j;
// set mode 320x200x8
    vga = new VGA;
    for (i=0; i<256; i++)
    {
        vga->SetColour( i, texture->palette[i*3]>>2, texture->palette[i*3+1]>>2,  texture->palette[i*3+2]>>2 );
    }
// setup 3D data
    VECTOR A, B, C;
// start the timer
    timer = new TIMER;
    long long startTime = timer->getCount(), frameCount = 0, currentTime;
// run the main loop

    while (!kbhit())
    {

          currentTime = timer->getCount();
       // clear the background
          vga->Clear();
       // setup the 3 control points of our plane, could do something much
       // more dynamic (with rotX also) but this looks good enough
          A = VECTOR( (float)(currentTime)/34984.0f, -16, (float)(currentTime)/43512.0f );
          B = rotY( 0.32 ) * VECTOR( 256, 0, 0 );
          C = rotY( 0.32 ) * VECTOR( 0, 0, 256 );
       // draw plane
          DrawPlane( A, B, C );
       // flush to video ram
          vga->Update();
          frameCount++;
    }
// calculate total running time
    long long totalTime = timer->getCount() - startTime;
// and turn off the timer
    delete (timer);
// now get FPS
    float FPS = (float)(frameCount)*1193180.0f / (float)(totalTime);
// return to text mode
    delete (vga);
// free memory
    delete (texture);
// make sure every one knows about flipcode :)
    cout << "                             The Art Of" << endl;
    cout << "                         D E M O M A K I N G " << endl << endl;
    cout << "                       by Alex J. Champandard" << endl << endl;
    cout << "       Go to http://www.flipcode.com/demomaking for more information." << endl;
    cout << endl << FPS << " frames per second." << endl;
// we've finished!
    return 0;
}


