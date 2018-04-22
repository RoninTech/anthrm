
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
#include "demos.h"

typedef struct {
	union {
		struct {
			unsigned char b,g,r,a;
		}c;
		unsigned int val;
	}u;
}TCOLOUR4;

static void setPixel32a_NBr (const TFRAME *frm, const int x, const int row, const int value)
{
	TCOLOUR4 *dst = (TCOLOUR4*)(frm->pixels+(row+(x<<2)));
	const TCOLOUR4 *src = (TCOLOUR4*)&value;
	const unsigned int alpha = src->u.c.a + (unsigned int)(src->u.c.a>>7);
	const unsigned int odds1 = (dst->u.val>>8)&0xFF00FF;
	const unsigned int odds2 = (src->u.val>>8)&0xFF00FF;
	const unsigned int evens1 = dst->u.val&0xFF00FF;
	const unsigned int evens2 = src->u.val&0xFF00FF;
	const unsigned int evenRes = ((((evens2-evens1)*alpha)>>8) + evens1)& 0xFF00FF;
	const unsigned int oddRes = ((odds2-odds1)*alpha + (odds1<<8)) & 0xFF00FF00;
	const unsigned int colour = evenRes | oddRes;

#if 1
	dst->u.val = colour;
#else
	if (frm->style == LSP_SET)
		dst->u.val = colour;
	else if (frm->style == LSP_OR)
		dst->u.val |= colour;
	else if (frm->style == LSP_XOR)
		dst->u.val ^= colour;
	else if (frm->style == LSP_AND)
		dst->u.val &= colour;
	else if (frm->style == LSP_CLEAR)
		dst->u.val = value;
#endif
	return;
}

int main (int argc, char* argv[])
{

	if (!initDemoConfig("config.cfg"))
		return 0;

	if (frame->bpp != LFRM_BPP_32A){
		lDeleteFrame(frame);
		frame = lNewFrame(hw, DWIDTH, DHEIGHT, LFRM_BPP_32A);
	}
	lSetBackgroundColour(hw, lGetRGBMask(frame, LMASK_BLACK));
	lSetForegroundColour(hw, lGetRGBMask(frame, LMASK_WHITE));
	
	int r,g,b;
	int x,y;
	int row;
	int t;
	int frameCt = 0;
	const int start = GetTickCount();

	do{
		for (y = 0; y < frame->height; y++){
			row = y * frame->pitch;
			t = ((GetTickCount()/10) % 256);

			for (x = 0; x < frame->width; x++){
				r = (x>>1)&0xFF;
				g = y&0xFF;
				b = t&0xFF;
				//lSetPixel_NBr(frame, x, row, 0xFF000000|(r<<16)|(g<<8)|b);
				setPixel32a_NBr(frame, x, row, 0xFF000000|(r<<16)|(g<<8)|b);
			}
		}
		//lRefresh(frame);
		lUpdate(hw, frame->pixels, frame->frameSize);
		frameCt++;
	}while(!kbhit() && frameCt < 1000);

  	const int end = GetTickCount();
	const float time = (float)(end-start)*0.001;
	printf("frames:%d\ntime:%.2fs\nfps:%.1f\n\n",frameCt,time,frameCt/time);


	demoCleanup();
	return 1;
}
