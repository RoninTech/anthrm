
// libmylcd
// An LCD framebuffer library
// Michael McElligott
// okio@users.sourceforge.net

//  Copyright (c) 2005-2009  Michael McElligott
// 
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU LIBRARY GENERAL PUBLIC LICENSE
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU LIBRARY GENERAL PUBLIC LICENSE for details.



#include <stdio.h>
#include <stdlib.h>

#include "mylcd.h"

#if (__BUILD_JPG_SUPPORT__)

#include "../memory.h"
#include "../pixel.h"
#include "../frame.h"
#include "../fileio.h"
#include "../tga.h"
#include "../convert.h"
#include "../copy.h"
#include "jpeglib.h"
#include "jpg.h"
#include <setjmp.h>


#if (__BUILD_JPG_READ_SUPPORT__)

struct my_error_mgr {
  struct jpeg_error_mgr pub;	/* "public" fields */

  jmp_buf setjmp_buffer;	/* for return to caller */
};



int read_JPEG_file (TFRAME *frame, const wchar_t *filename, const int resize)
{

	struct jpeg_decompress_struct cinfo;
	struct my_error_mgr jerr;
	JSAMPARRAY buffer;	/* Output row buffer */
	FILE *infile;		/* source file */
	int row_stride;		/* physical row width in output buffer */

	if ((infile = l_wfopen(filename, L"rb")) == NULL) {
		wprintf(L"libjpg: can't open %s\n", filename);
		return 0;
	}

	uint64_t insize = l_lof(infile);
	if (!insize){
		wprintf(L"libjpg: file size is 0: '%s'\n", filename);
		l_fclose(infile);
		return 0;
	}

	uint8_t *inbuffer = l_malloc(insize);
	if (inbuffer == NULL){
		printf("libjpg: out of memory\n");
		l_fclose(infile);
		return 0;
	}
	
	int len = l_fread(inbuffer, 1, insize, infile);
	l_fclose(infile);
	if (len != insize)
		printf("libjpg: warning read size differs from expected length: %u != %u\n", len, insize);

	cinfo.err = jpeg_std_error(&jerr.pub);
	jpeg_create_decompress(&cinfo);
	jpeg_mem_src(&cinfo, inbuffer, insize);
	if (!jpeg_read_header(&cinfo, TRUE)){
		jpeg_destroy_decompress(&cinfo);
		free(inbuffer);
		return 0;
	}

	jpeg_start_decompress(&cinfo);
	row_stride = cinfo.output_width * cinfo.output_components;
	buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr)&cinfo, JPOOL_IMAGE, row_stride*2, 1);
	char *jrowbuffer = (char*)buffer[0];
	
	if (resize)
		resizeFrame(frame, cinfo.output_width, cinfo.output_height, 0);
   
	int y = 0;
	char *p;
	uint16_t *p16;
	int i,j,r,g,b;
	char *out8 = NULL;
	const int pitch = frame->pitch;

	if (cinfo.num_components == 1){
		out8 = l_malloc((4+frame->width)*4);
		if (!out8)
			cinfo.output_scanline = 1+cinfo.output_height; // freak out
	}

	while (cinfo.output_scanline < cinfo.output_height){
		jpeg_read_scanlines(&cinfo, buffer, 1);

		// fixup for single channel images. convert 8bpp to 24bpp
		if (cinfo.num_components == 1){
			jrowbuffer = (char*)buffer[0];
			int i = 0;
			int tpixels = cinfo.output_width;
			while (tpixels--){
				out8[i  ] = *jrowbuffer; 
				out8[i+1] = *jrowbuffer; 
				out8[i+2] = *jrowbuffer++;
				i += 3;
			}
			jrowbuffer = out8;
		}

		if (y < frame->height){
			if (frame->bpp == LFRM_BPP_24){
				p = lGetPixelAddress(frame, 0, y);
				for (i = 0; i < pitch-2; i+=3){
					p[i] = jrowbuffer[i+2];
					p[i+1] = jrowbuffer[i+1];
					p[i+2] = jrowbuffer[i];
				}
			}else if ( frame->bpp == LFRM_BPP_32A){
				p = lGetPixelAddress(frame, 0, y);
				for (i = 0, j = 0; i < pitch-2; i+=4, j+=3){
					p[i] = jrowbuffer[j+2];
					p[i+1] = jrowbuffer[j+1];
					p[i+2] = jrowbuffer[j];
					p[i+3] = 0xFF;
				}
			}else if (frame->bpp == LFRM_BPP_32){
				p = lGetPixelAddress(frame, 0, y);
				for (i = 0, j = 0; i < pitch-2; i+=4, j+=3){
					p[i] = jrowbuffer[j+2];
					p[i+1] = jrowbuffer[j+1];
					p[i+2] = jrowbuffer[j];
				}
			}else if (frame->bpp == LFRM_BPP_16){
				p16 = lGetPixelAddress(frame, 0, y);
				for (i = 0, j = 0; i < frame->width; i++){
					r = (jrowbuffer[j++]&0xF8)<<8;	// 5
					g = (jrowbuffer[j++]&0xFC)<<3;	// 6
					b = (jrowbuffer[j++]&0xF8)>>3;	// 5
					p16[i] = r|g|b;
				}
			}else if (frame->bpp == LFRM_BPP_15){
				p16 = lGetPixelAddress(frame, 0, y);
				for (i = 0, j = 0; i < frame->width; i++){
					r = (jrowbuffer[j++]&0xF8)<<7;	// 5
					g = (jrowbuffer[j++]&0xF8)<<2;	// 5
					b = (jrowbuffer[j++]&0xF8)>>3;	// 5
					p16[i] = r|g|b;
				}
			}else if (frame->bpp == LFRM_BPP_12){
				p16 = lGetPixelAddress(frame, 0, y);
				for (i = 0, j = 0; i < frame->width; i++){
					r = (jrowbuffer[j++]&0xF0)<<4;
					g = (jrowbuffer[j++]&0xF0);
					b = (jrowbuffer[j++]&0xF0)>>4;
					p16[i] = r|g|b;
				}
			}else if (frame->bpp == LFRM_BPP_8){
				p = lGetPixelAddress(frame, 0, y);
				for (i = 0, j = 0; i < frame->width; i++){
					r = (jrowbuffer[j++]&0xE0);
					g = (jrowbuffer[j++]&0xE0)>>3;
					b = (jrowbuffer[j++]&0xC0)>>6;
					p[i] = r|g|b;
				}
			}else if (frame->bpp == LFRM_BPP_1){
				for (i = 0, j = 0; i < frame->width; i++){
					if ((jrowbuffer[j]<TGATRESHOLD) || (jrowbuffer[j+1]<TGATRESHOLD) || (jrowbuffer[j+2]<TGATRESHOLD))
						setPixel_BC(frame, i, y, LSP_SET);
					j += 3;
				}
			}
			y++;		
		}
	}
	if (out8) l_free(out8);
	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
	l_free(inbuffer);
	return 1;
}

#endif

/*
 * SOME FINE POINTS:
 *
 * In the above code, we ignored the return value of jpeg_read_scanlines,
 * which is the number of scanlines actually read.  We could get away with
 * this because we asked for only one line at a time and we weren't using
 * a suspending data source.  See libjpeg.txt for more info.
 *
 * We cheated a bit by calling alloc_sarray() after jpeg_start_decompress();
 * we should have done it beforehand to ensure that the space would be
 * counted against the JPEG max_memory setting.  In some systems the above
 * code would risk an out-of-memory error.  However, in general we don't
 * know the output image dimensions before jpeg_start_decompress(), unless we
 * call jpeg_calc_output_dimensions().  See libjpeg.txt for more about this.
 *
 * Scanlines are returned in the same order as they appear in the JPEG file,
 * which is standardly top-to-bottom.  If you must emit data bottom-to-top,
 * you can use one of the virtual arrays provided by the JPEG memory manager
 * to invert the data.  See wrbmp.c for an example.
 *
 * As with compression, some operating modes may require temporary files.
 * On some systems you may need to set up a signal handler to ensure that
 * temporary files are deleted if the program is interrupted.  See libjpeg.txt.
 */

#if (__BUILD_JPG_WRITE_SUPPORT__)

int write_JPEG_file (TFRAME *src, const wchar_t *filename, const int width, const int height, const int quality)
{
	
	TFRAME *tmp = NULL;
	TFRAME *frame = src;
	
	if (width != src->width || height != src->height){
		tmp = frame = _newFrame(src->hw, width, height, 1, src->bpp);
		copyAreaScaled(src, tmp, 0, 0, src->width, src->height, 0, 0, width, height, LCASS_CPY);
	}

	const pConverterFn convert = getConverter(frame->bpp, LFRM_BPP_24);
	if (convert == NULL){
		printf("libjpg: could not acquire a converter (%i %i)\n",frame->bpp, LFRM_BPP_24);
	}
	
	const int row_stride = frame->width * 3;		/* physical row width in image buffer */
	const size_t outsize = row_stride * frame->height;
	
	unsigned char *image_buffer = malloc(outsize);
	if (!image_buffer){
		printf("libjpg: out of memory\n");
		deleteFrame(tmp);
		return 0;
	}
	
	convert(frame, (unsigned int*)image_buffer);
	
  /* This struct contains the JPEG compression parameters and pointers to
   * working space (which is allocated as needed by the JPEG library).
   * It is possible to have several such structures, representing multiple
   * compression/decompression processes, in existence at once.  We refer
   * to any one struct (and its associated working data) as a "JPEG object".
   */
  struct jpeg_compress_struct cinfo;
  /* This struct represents a JPEG error handler.  It is declared separately
   * because applications often want to supply a specialized error handler
   * (see the second half of this file for an example).  But here we just
   * take the easy way out and use the standard error handler, which will
   * print a message on stderr and call exit() if compression fails.
   * Note that this struct must live as long as the main JPEG parameter
   * struct, to avoid dangling-pointer problems.
   */
  struct jpeg_error_mgr jerr;
  /* More stuff */
  FILE * outfile;		/* target file */
  JSAMPROW row_pointer[frame->height];	/* pointer to JSAMPLE row[s] */

  /* Step 1: allocate and initialize JPEG compression object */

  /* We have to set up the error handler first, in case the initialization
   * step fails.  (Unlikely, but it could happen if you are out of memory.)
   * This routine fills in the contents of struct jerr, and returns jerr's
   * address which we place into the link field in cinfo.
   */
  cinfo.err = jpeg_std_error(&jerr);
  /* Now we can initialize the JPEG compression object. */
  jpeg_create_compress(&cinfo);

  /* Step 2: specify data destination (eg, a file) */
  /* Note: steps 2 and 3 can be done in either order. */

  /* Here we use the library-supplied code to send compressed data to a
   * stdio stream.  You can also write your own code to do something else.
   * VERY IMPORTANT: use "b" option to fopen() if you are on a machine that
   * requires it in order to write binary files.
   */
	if ((outfile = l_wfopen(filename, L"wb")) == NULL) {
		wprintf(L"libjpg: can't open '%s'\n", filename);
		deleteFrame(tmp);
		l_free(image_buffer);
		return 0;
	}
  jpeg_stdio_dest(&cinfo, outfile);

  /* Step 3: set parameters for compression */

  /* First we supply a description of the input image.
   * Four fields of the cinfo struct must be filled in:
   */
  cinfo.image_width = frame->width; 	/* image width and height, in pixels */
  cinfo.image_height = frame->height;
  cinfo.input_components = 3;		/* # of color components per pixel */
  cinfo.in_color_space = JCS_RGB; 	/* colorspace of input image */
  /* Now use the library's routine to set default compression parameters.
   * (You must set at least cinfo.in_color_space before calling this,
   * since the defaults depend on the source color space.)
   */
   
  jpeg_set_defaults(&cinfo);
  
  /* Now you can set any non-default parameters you wish to.
   * Here we just illustrate the use of quality (quantization table) scaling:
   */

  jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);

  /* Step 4: Start compressor */

  /* TRUE ensures that we will write a complete interchange-JPEG file.
   * Pass TRUE unless you are very sure of what you're doing.
   */
  jpeg_start_compress(&cinfo, TRUE);

  /* Step 5: while (scan lines remain to be written) */
  /*           jpeg_write_scanlines(...); */

  /* Here we use the library's state variable cinfo.next_scanline as the
   * loop counter, so that we don't have to keep track ourselves.
   * To keep things simple, we pass one scanline per call; you can pass
   * more if you wish, though.
   */


	for (int i = 0; i < frame->height; i++)
		row_pointer[i] = &image_buffer[i * row_stride];

//  while (cinfo.next_scanline < cinfo.image_height) {
    /* jpeg_write_scanlines expects an array of pointers to scanlines.
     * Here the array is only one element long, but you could pass
     * more than one scanline at a time if that's more convenient.
     */
    jpeg_write_scanlines(&cinfo, row_pointer, frame->height);
  //}

  /* Step 6: Finish compression */

  jpeg_finish_compress(&cinfo);
  /* After finish_compress, we can close the output file. */
  l_fclose(outfile);

  /* Step 7: release JPEG compression object */

  /* This is an important step since it will release a good deal of memory. */
  jpeg_destroy_compress(&cinfo);

  /* And we're done! */
  
  
  deleteFrame(tmp);
  l_free(image_buffer);
  return 1;
}

#endif

#endif
