
// libmylcd - http://mylcd.sourceforge.net/
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
//  GNU LIBRARY GENERAL PUBLIC LICENSE for more details.
//
//	You should have received a copy of the GNU Library General Public
//	License along with this library; if not, write to the Free
//	Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.


#include "common.h"

static float table255f[256];


void createPixConvertTables ()
{
	for (int i = 0; i < 256; i++)
		table255f[i] = (float)i/255.0f;
}

static inline int getPixel32_NBr (const TFRAME *frm, const int x, const int row)
{
	return *(uint32_t*)(frm->pixels+(row+(x<<2)));
}

static inline void setPixel32_NBr (const TFRAME *frm, const int x, const int row, const int value)
{
	*(uint32_t*)(frm->pixels+(row+(x<<2))) = value;
} 

static inline void getPixel32fr (const TFRAME *frm, const int x, const int row, float *r, float *g, float *b)
{
	uint8_t *pixel = (uint8_t*)(frm->pixels+(row+(x<<2)));
	*b = table255f[*pixel];
	*g = table255f[*(pixel+1)];
	*r = table255f[*(pixel+2)];
}

static inline int blendColoursNoEmms (const unsigned int src, const unsigned int dst, unsigned int factor)
{
#ifndef _DEBUG_ 

	static unsigned short INVERT_MASK[4] = {0x00FF, 0x00FF, 0x00FF, 0x00FF};
	int returnParam;
	factor = 255 - factor;

	__asm volatile (
	  "movd %1, %%mm0\n"
	  "movd %2, %%mm1\n"
	  "pxor %%mm2, %%mm2\n"
	  "punpcklbw %%mm2, %%mm0\n"
	  "punpcklbw %%mm2, %%mm1\n"
	  
	  // Get the alpha value //
	  "movd %4, %%mm3\n"
	  "punpcklwd %%mm3, %%mm3\n"
	  "punpcklwd %%mm3, %%mm3\n"
	  
	  // (alpha * (source + 255 - dest))/255 + dest - alpha //
	  "paddw (%3), %%mm0\n"
	  "psubw %%mm1, %%mm0\n"
	  "psrlw $1, %%mm0\n"
	  "pmullw %%mm3, %%mm0\n"
	  "psrlw $7, %%mm0\n"
	  "paddw %%mm1, %%mm0\n"
	  "psubw %%mm3, %%mm0\n"
	  
	  "packuswb %%mm0, %%mm0\n"
	  "movd %%mm0, %0\n"
	  : "=&a" (returnParam)
	  : "rm" (dst), "rm" (src), "rm" (&INVERT_MASK[0]), "rm" (factor)
	  : "memory"
	);
	return returnParam;
#else
	return src;
#endif
}

static inline void callEmms ()
{
  __asm volatile("emms\n");
}

void drawVolume (const TFRAME *src, TFRAME *des, const int Xoffset, const int Yoffset, const int srcx1, const int srcy1, const int srcWidth, const int srcHeight, const float alpha)
{
	int x, y, rowdes, rowsrc;
	unsigned int blend;
	unsigned int spix;
	unsigned int dpix;
	const int factor = alpha * 255.0;

	for (y = srcy1; y < srcHeight; y++){
		rowdes = (y+Yoffset)*des->pitch;
		rowsrc = y*src->pitch;
					
		for (x = srcx1; x < srcWidth; x++){
			spix = getPixel32_NBr(src, x, rowsrc);
			if (spix&0xFF000000){
				dpix = getPixel32_NBr(des, x+Xoffset, rowdes);
				blend = blendColoursNoEmms(spix, dpix, factor);
				setPixel32_NBr(des, x+Xoffset, rowdes, blend);
			}
		}
	}
	callEmms();
}
