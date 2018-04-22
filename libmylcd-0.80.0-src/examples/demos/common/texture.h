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


#ifndef __TEXTURE_H_
#define __TEXTURE_H_

#include <stdio.h>
#include <stdlib.h>

class TEXTURE
{
   public:

// constructor
   TEXTURE( char *filename );
// destructor
   ~TEXTURE();

// memory pointers, should be private and accessible via a wrapper
// but i'm a naughty boy :)
   unsigned char *location, *palette;

   private:
// file handle
   FILE *fp;

};

#endif
