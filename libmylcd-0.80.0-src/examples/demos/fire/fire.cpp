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
 *  Compile with DJGPP:    gcc *.cpp -s -O2 -o fire.exe -lstdcxx -lpng -lz
 *
 */

#include <stdlib.h>    // for function rand()
#include <conio.h>     //              kbhit()
#include <iostream>
#include <string.h>
#include "texture.h"   // PNG file loader
#include "vga.h"
#include "timer.h"

#undef main

// this handles all the graphics
VGA *vga;

// our timing module
TIMER *timer;

// contains the PNG file's info
TEXTURE *texture;


// contains some cool text
unsigned char *image;

/*
 * create a shade of colours in the palette from s to e
 */
void Shade_Pal( int s, int e, int r1, int g1, int b1, int r2, int g2, int b2 )
{
    int i;
    float k;
    for (i=0; i<=e-s; i++)
    {
        k = (float)i/(float)(e-s);
        vga->SetColour( s+i, (int)(r1+(r2-r1)*k), (int)(g1+(g2-g1)*k), (int)(b1+(b2-b1)*k) );
    }
}


/*
 * adds some hot pixels to a buffer
 */
void Heat( unsigned char *dst )
{
     int i, j;
// add some random hot spots where the text is
     for (i=26880; i<52480; i++)
     {
         if (image[i] > dst[i])
         	dst[i] = rand()&(image[i]);
     }
     j = (rand() % 512);
// add some random hot spots at the bottom of the buffer
     for (i=0; i<j; i++)
     {
         dst[63040+(rand()%960)] = 255;
     }
}

/*
 * smooth a buffer upwards, make sure not to read pixels that are outside of
 * the buffer!
 */
void Blur_Up( unsigned char *src, unsigned char *dst )
{
     int offs = 0;
     unsigned char b;
     for (int j=0; j<198; j++)
     {
     // set first pixel of the line to 0
         dst[offs] = 0; offs++;
     // calculate the filter for all the other pixels
         for (int i=1; i<319; i++)
         {
         // calculate the average
             b = ( src[offs-1]   +               + src[offs+1]
                 + src[offs+319] + src[offs+320] + src[offs+321]
                 + src[offs+639] + src[offs+640] + src[offs+641] ) >> 3;
          // decrement the sum by one so that the fire loses intensity
             if (b>0) b--;
          // store the pixel
             dst[offs] = b;
             offs++;
         }
     // set last pixel of the line to 0
         dst[offs] = 0; offs++;
     }
// clear the last 2 lines
     memset( (void *)((long)(dst)+offs), 0, 640 );
}

/*
 * the main program
 */
int main()
{
// set mode 320x200x8
    vga = new VGA;
// setup some fire-like colours
    Shade_Pal( 0, 23, 0, 0, 0, 0, 0, 31 );
    Shade_Pal( 24, 47, 0, 0, 31, 63, 0, 0 );
    Shade_Pal( 48, 63, 63, 0, 0, 63, 63, 0 );
    Shade_Pal( 64, 127, 63, 63, 0, 63, 63, 63 );
    Shade_Pal( 128, 255, 63, 63, 63, 63, 63, 63 );
// buffer to store the image
    image = new unsigned char[64000];
// two fire buffers
    unsigned char *fire1 = new unsigned char[64000];
    unsigned char *fire2 = new unsigned char[64000];
// a temporary variable to swap the two previous buffers
    unsigned char *tmp;
// clear the buffers
    memset( fire1, 0, 64000 );
    memset( fire2, 0, 64000 );
// load the background image
   texture = new TEXTURE( "image1.png" );
   memcpy( image, texture->location, 64000 );
// start the timer
    timer = new TIMER;
    long long startTime = timer->getCount(), frameCount = 0;
// run the main loop
    while (!kbhit())
    {
    // heat the fire
       Heat( fire1 );
    // apply the filter
       Blur_Up( fire1, fire2 );
    // dump our temporary buffer to the screen
       memcpy( vga->page_draw, fire2, 63040 );
       vga->Update();
    // swap our two fire buffers
       tmp = fire1;
       fire1 = fire2;
       fire2 = tmp;
    // that's one more done!
       frameCount++;
       Sleep(5);
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
    delete [] image;
    delete [] fire1;
    delete [] fire2;
// make sure every one knows about flipcode :)
/*    cout << "                             The Art Of" << endl;
    cout << "                         D E M O M A K I N G " << endl << endl;
    cout << "                       by Alex J. Champandard" << endl << endl;
    cout << "       Go to http://www.flipcode.com/demomaking for more information." << endl;
    cout << endl << FPS << " frames per second." << endl;*/
    
    printf("%.2f\n",FPS);
// we've finished!
    return 0;
}
