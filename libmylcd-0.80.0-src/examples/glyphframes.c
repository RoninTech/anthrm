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


#include <mylcd.h>


static THWD *hw = NULL;


int main (int argc, char* argv[])
{
    if (!(hw=lOpen(NULL, NULL)))
    	return 0;

	TFRAME *tmp = lNewFrame(hw, 8, 8, LFRM_BPP_32A);
	lSetBackgroundColour(hw, lGetRGBMask(tmp, LMASK_BLACK));
	lSetForegroundColour(hw, lGetRGBMask(tmp, LMASK_WHITE));

	TFRAME *frame = lNewString(hw, tmp->bpp, 0, LFTW_B16B, "&#9664; &#9654; &#12540;");
	lRefresh(frame);
	
	lSaveImage(frame, L"lro.png", IMG_PNG, 0, 0);
	lSaveImage(frame, L"lro.bmp", IMG_BMP, 0, 0);
	lSaveImage(frame, L"lro.tga", IMG_TGA, 0, 0);
	
	lDeleteFrame(frame);
	lDeleteFrame(tmp);
	
	lSetCharacterEncoding(hw, CMT_SHIFTJIS);
	char str[3] = {0x81, 0x5b, 0};	// a chouon make (katakana)

	TWCHAR *g = lGetGlyph(hw, str, 0, LFTW_WENQUANYI9PT);
	if (g){
		printf("chouon.bmp: w:%i h:%i unicode:%i\n",g->f->width, g->f->height, g->encoding);
		lSaveImage(g->f, L"chouon.bmp", IMG_BMP, 0, 0);
	}

	g = lGetGlyph(hw, "&#9654;", 0, LFTW_B16B);
	if (g)
		lSaveImage(g->f, L"r.bmp", IMG_BMP, 0, 0);

	lClose(hw);
	return 1;
}

