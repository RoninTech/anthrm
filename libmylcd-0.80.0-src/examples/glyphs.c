
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

	
char txt[] = "`他们为什么不说中文'";  //= 'why don't you speak chinese'


int main (int argc, char* argv[])
{

	if (!initDemoConfig("config.cfg"))
		return 0;
		
	lSetBackgroundColour(hw, lGetRGBMask(frame, LMASK_WHITE));
	lSetForegroundColour(hw, lGetRGBMask(frame, LMASK_BLACK));

#if 1
	// lGetGlyph() will return NULL if requested code point is not supported by font
	TWCHAR *wc = lGetGlyph(hw, "&euro;", 0, LFTW_B24);
	if (wc){
		lDrawRectangle(wc->f, 0, 0, wc->f->width-1, wc->f->height-1, lGetRGBMask(wc->f, LMASK_WHITE));
		lSaveImage(wc->f, L"euro.tga", IMG_TGA, wc->f->width*2, wc->f->height*2);
		
		lCopyFrame(wc->f, frame);
		lMoveArea(frame, 0, 0, wc->f->width-1, DHEIGHT, DHEIGHT/2, LMOV_CLEAR, LMOV_DOWN);
		lRefresh(frame);
		
		int flen = wc->f->width * wc->f->height * 3;
		char *temp = (char *)calloc(1, flen+1);
		lFrame1BPPToRGB(wc->f, temp, LFRM_BPP_24, (240<<16) | (240<<8) | 240, (110<<16) | (110<<8) | 110);
		FILE *fp = fopen("euroRGB24_12x24.raw","wb");
		if (fp){
			fwrite(temp, sizeof(char), flen, fp);
			fclose(fp);
		}
		free(temp);
	}

	wc = lGetGlyph(hw, NULL, 0x2500, LFTW_WENQUANYI9PT);
	if (wc)
		lSaveImage(wc->f,L"0x2500.bmp",IMG_BMP, wc->f->width*2, wc->f->height*2);

	wc = lGetGlyph(hw, NULL, 0x9F98, LFTW_B24);
	if (wc)
		lSaveImage(wc->f,L"0x9F98.bmp",IMG_BMP, wc->f->width*2, wc->f->height*2);

	wc = lGetGlyph(hw, NULL, 0xFFFD, LFTW_UNICODE);
	if (wc)
		lSaveImage(wc->f,L"0xFFFD.bmp",IMG_BMP, wc->f->width*2, wc->f->height*2);
#endif

	lSetCharacterEncoding(hw, CMT_GB18030);

	TFRAME *gb = lNewString(hw, DBPP, PF_VERTICALRTL|PF_CLIPWRAP/*|PF_DONTFORMATBUFFER*/, LFTW_B24, txt);
	if (gb){
		lRefresh(gb);
		lSaveImage(gb, L"gb.bmp", IMG_BMP, gb->width, gb->height);
		lDeleteFrame(gb);
	}

	demoCleanup();
	return 1;
}


