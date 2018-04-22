
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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <windows.h>

#include "mylcd.h"
#include "demos.h"

#define text "The quick brown fox jumps over the lazy dog."


int main (int argc, char* argv[])
{
	if (!initDemoConfig("config.cfg"))
		return 0;

	int r,g,b,x,y;
	TLPRINTR trect = {0,1,frame->width-1,frame->height-1,0,0,0,0};
	
	lSetBackgroundColour(hw, lGetRGBMask(frame, LMASK_BLACK));
	lSetForegroundColour(hw, lGetRGBMask(frame, LMASK_WHITE));
	lClearFrame(frame);

    for (y = 0; y < frame->height; y++){
    	for (x = 0; x < frame->width; x++){
    		if (frame->bpp == LFRM_BPP_12){
       			r = x>>3;
				g = y>>3;
				b = 8; //(15-(x>>3))&0x0F;
				lSetPixel(frame, x+1, y+1, (r<<8)|(g<<4)|b);	// 12bit

			}else if (frame->bpp == LFRM_BPP_32A || frame->bpp == LFRM_BPP_32){
       			r = (x>>1)&0xFF;
				g = y&0xFF;
				b = 127;
				lSetPixel(frame, x+1, y+1, 0xFF000000|(r<<16)|(g<<8)|b);	// 32bit
								
			}else if (frame->bpp == LFRM_BPP_16){
       			r = (x>>4)&0x1F;
				g = (y>>2)&0x3F;
				b = 16&0x1F;
				lSetPixel(frame, x+1, y+1, (r<<11)|(g<<5)|b);	// 16bit
			}else{
				r = (x/16);
				g = (y/16);
				b = 1;
				frame->pops->set(frame, x, y, r<<5|g<<2|b);	// 8bit
			}
	    }
	}

	lSetPixelWriteMode(frame, LSP_XOR);
	lSetRenderEffect(hw, LTR_OUTLINE2);
	lSetFilterAttribute(hw, LTR_OUTLINE2, 0, lGetRGBMask(frame, LMASK_RED));	// set outline colour
	lPrintEx(frame, &trect, LFTW_ROUGHT18, PF_MIDDLEJUSTIFY|PF_WORDWRAP|PF_CLIPWRAP, LPRT_CPY, text);

	lSetRenderEffect(hw, LTR_OUTLINE3);
	lSetFilterAttribute(hw, LTR_OUTLINE3, 0, 0x00000000);	// set outline colour
	lPrintEx(frame, &trect, LFTW_ROUGHT18, PF_NEWLINE|PF_MIDDLEJUSTIFY|PF_WORDWRAP|PF_CLIPWRAP, LPRT_CPY, text);
	
	lSetPixelWriteMode(frame, LSP_SET);

	lSetRenderEffect(hw, LTR_DEFAULT);
	lPrintEx(frame, &trect, LFTW_ROUGHT18, PF_NEWLINE|PF_MIDDLEJUSTIFY|PF_WORDWRAP|PF_CLIPWRAP, LPRT_CPY, text);

	lSetRenderEffect(hw, LTR_SMOOTH2);
	lSetFilterAttribute(hw, LTR_SMOOTH2, 0, (128 << 24) | 0x00FF00);	// set smoothing colour
	lPrintEx(frame, &trect, LFTW_ROUGHT18, PF_NEWLINE|PF_MIDDLEJUSTIFY|PF_WORDWRAP|PF_CLIPWRAP, LPRT_CPY, text);
		
	lSetRenderEffect(hw, LTR_SKEWEDFW);		// forward skew
	lSetFilterAttribute(hw, LTR_SKEWEDFW, 0, 100);	// set slope factor (1-255)
	lPrintEx(frame, &trect, LFTW_ROUGHT18, PF_NEWLINE|PF_MIDDLEJUSTIFY|PF_WORDWRAP|PF_CLIPWRAP, LPRT_CPY, text);

	lSetRenderEffect(hw, LTR_SKEWEDFWSM1);	// with a little smoothing
	lSetFilterAttribute(hw, LTR_SKEWEDFWSM1, 0, 100);
	lPrintEx(frame, &trect, LFTW_ROUGHT18, PF_NEWLINE|PF_MIDDLEJUSTIFY|PF_WORDWRAP|PF_CLIPWRAP, LPRT_CPY, text);

	lSetRenderEffect(hw, LTR_SKEWEDFWSM2);	// with a little more smoothing
	lSetFilterAttribute(hw, LTR_SKEWEDFWSM2, 0, 100);
	lPrintEx(frame, &trect, LFTW_ROUGHT18, PF_NEWLINE|PF_MIDDLEJUSTIFY|PF_WORDWRAP|PF_CLIPWRAP, LPRT_CPY, text);
	
	lSetRenderEffect(hw, LTR_SKEWEDBK);		// backward skew
	lSetFilterAttribute(hw, LTR_SKEWEDBK, 0, 100);
	lPrintEx(frame, &trect, LFTW_ROUGHT18, PF_NEWLINE|PF_MIDDLEJUSTIFY|PF_WORDWRAP|PF_CLIPWRAP, LPRT_CPY, text);

	lSetRenderEffect(hw, LTR_SKEWEDBKSM1);
	lSetFilterAttribute(hw, LTR_SKEWEDBKSM1, 0, 100);
	lPrintEx(frame, &trect, LFTW_ROUGHT18, PF_NEWLINE|PF_MIDDLEJUSTIFY|PF_WORDWRAP|PF_CLIPWRAP, LPRT_CPY, text);

	lSetRenderEffect(hw, LTR_SKEWEDBKSM2);
	lSetFilterAttribute(hw, LTR_SKEWEDBKSM2, 0, 100);
	lPrintEx(frame, &trect, LFTW_ROUGHT18, PF_NEWLINE|PF_MIDDLEJUSTIFY|PF_WORDWRAP|PF_CLIPWRAP, LPRT_CPY, text);

	lSetRenderEffect(hw, LTR_SHADOW);
	// set direction to South-East, shadow thickness to 3, offset by 2 pixels and transparency to 31% (79 = 0.31*255)
	lSetFilterAttribute(hw, LTR_SHADOW, 0, LTRA_SHADOW_S|LTRA_SHADOW_E | LTRA_SHADOW_S3 | LTRA_SHADOW_OS(2) | LTRA_SHADOW_TR(79));
	lPrintEx(frame, &trect, LFTW_ROUGHT18, PF_NEWLINE|PF_MIDDLEJUSTIFY|PF_WORDWRAP|PF_CLIPWRAP, LPRT_CPY, text);

	lSetFilterAttribute(hw, LTR_SHADOW, 0, LTRA_SHADOW_S|LTRA_SHADOW_E | LTRA_SHADOW_S5 | LTRA_SHADOW_OS(5) | LTRA_SHADOW_TR(60));
	lSetFilterAttribute(hw, LTR_SHADOW, 1, 0xFF0000);	// set shadow colour (default is 0x00000)
	lPrintEx(frame, &trect, LFTW_ROUGHT18, PF_NEWLINE|PF_MIDDLEJUSTIFY|PF_WORDWRAP|PF_CLIPWRAP, LPRT_CPY, text);

/*	
	lSetRenderEffect(hw, LTR_VFLIP);
	lPrintEx(frame, &trect, LFTW_ROUGHT18, PF_NEWLINE|PF_MIDDLEJUSTIFY|PF_WORDWRAP|PF_CLIPWRAP, LPRT_OR, text);

	lSetRenderEffect(hw, LTR_HFLIP);
	lPrintEx(frame, &trect, LFTW_ROUGHT18, PF_NEWLINE|PF_MIDDLEJUSTIFY|PF_WORDWRAP|PF_CLIPWRAP, LPRT_OR, text);
*/		

	lDrawRectangle(frame, 0, 0, frame->width-1, frame->height-1, 0xFF000000|lGetRGBMask(frame, LMASK_WHITE));

	lRefresh(frame);
	lSaveImage(frame, L"quickfox.png", IMG_PNG, 0, 0);
	demoCleanup();
	return 1;
}

