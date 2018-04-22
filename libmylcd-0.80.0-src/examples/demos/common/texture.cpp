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

#if 1
#include <png.h>
#include "texture.h"

TEXTURE::TEXTURE( char *filename )
{
// variables
   unsigned char buf[4];
   png_structp png_ptr;
   png_infop info_ptr;
   png_uint_32 width, height, scanline;
   int bit_depth, color_type, interlace_type, row;

// open the file
   fp = fopen( filename, "rb" );
   if(fp == NULL)
     {
       fprintf(stderr, "Error: Could not find %s\n", filename);
       exit(1);
     }

// read signature
   if ( fread( buf, 1, 4, fp ) != 4 ) printf("couldn't read");

// check if valid png
   if (png_sig_cmp( buf, (png_size_t)0, 4 ) ) printf("not a png?");
// create the png reading structure, errors go to stderr
   png_ptr = png_create_read_struct( PNG_LIBPNG_VER_STRING, NULL, NULL, NULL );
   if (png_ptr == NULL)
   {
      fclose(fp);
      return;
   }
// allocate info struct
   info_ptr = png_create_info_struct(png_ptr);
   if (info_ptr == NULL)
   {
      fclose(fp);
      png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
      return;
   }
// set error handling
   if (setjmp(png_ptr->jmpbuf))
   {
      png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
      fclose(fp);
      return;
   }
// set input method
   png_init_io(png_ptr, fp);
// tell libpng we have already read some bytes
   png_set_sig_bytes(png_ptr, 4 );
// read all info
   png_read_info(png_ptr, info_ptr);
// get some characteristics of the file
   png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type,
       &interlace_type, NULL, NULL);
// get size of scanline
   scanline = png_get_rowbytes( png_ptr, info_ptr );
// allocate texture memory
   location = new unsigned char[scanline*height];
// read the image line by line into the user's buffer
   for (row = 0; row < height; row++)
   {
      png_read_row( png_ptr, (unsigned char *)(location+(row*scanline)), NULL );
   }
// finish reading the file
   png_read_end(png_ptr, info_ptr);
// get palette, if any
   if (info_ptr->palette)
   {
       palette = new unsigned char[768];
       memcpy( palette, info_ptr->palette, 768 );
   }
// free png structures
   png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
// close, exit
   fclose(fp);
}

/*
 * this part of the code randomly hangs on some machines
 * no idea why... the memory is freed by DJGPP when the
 * program exits anyway, so don't worry :)
 */
TEXTURE::~TEXTURE()
{
 if (location) delete( location );
 if (palette) delete( palette );
};
#endif