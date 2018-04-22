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
 */


#ifndef __VGA_H_
#define __VGA_H_

#include <windows.h>
#define PI 3.141592654


typedef unsigned char Uint8;

class VGA
{
   public:

// constructor
   VGA();
// destructor
   ~VGA();
   
   void Lock ();
   void Unlock ();
   void Blit (unsigned char *buffer, int bytes);
   unsigned char *GetSurfaceData();
   
// dumps temporary buffer to the screen
   void Update();
// draw a pixel in temporary buffer
   void PutPixel( int x, int y, unsigned c );
// set a color in the palette
   void SetColour( unsigned char i, unsigned char r, unsigned char g, unsigned char b );
// clear temp buf
   void Clear();

// kids! don't try this at home :)
// private:

   unsigned char *page_draw;
   unsigned int colors[256];
};

#endif
