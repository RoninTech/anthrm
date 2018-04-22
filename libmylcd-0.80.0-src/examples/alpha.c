
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
#include <math.h>

#include "mylcd.h"
#include "demos.h"


typedef struct{
	ubyte r;
	ubyte g;
	ubyte b;
	ubyte a;
}__attribute__ ((packed))TRGBA;


int main (int argc, char* argv[])
{

	if (!initDemoConfig("config.cfg"))
		return 0;

	TFRAME *icon = lNewFrame(hw, 8, 8, LFRM_BPP_32A);
	TFRAME *img = lNewImage(hw, L"images/24bpp_somuchwin.tga", LFRM_BPP_32A);
	TFRAME *alp = lNewImage(hw, L"images/cube.png", LFRM_BPP_32A);
	lLoadImageEx(alp, L"images/clock_face.png", 150, 20);
	lLoadImageEx(alp, L"images/8.png", 200, 10);
	lLoadImageEx(alp, L"images/7.png", 120, 100);
	lLoadImageEx(alp, L"images/32bpp_alpha.png", 300, 30);
	lLoadImageEx(alp, L"images/play.png", 50, 0);
	lLoadImage(icon, L"images/play.png");


	// make white pixels transparent
	TRGBA *pixels = (TRGBA*)lGetPixelAddress(img, 0, 0);
	int x = img->width*img->height;
	while (x--){
		if (pixels[x].r > 200 && pixels[x].g > 200 && pixels[x].b > 200)
			pixels[x].a = 40;
	}

	lCopyArea(img, alp, 80, 20, 0, 0, img->width-1, img->height-1);
	lCopyArea(icon, alp, 120, 100, 0, 0, icon->width-1, icon->height-1);
	lDrawRectangleFilled(alp, 150, 100, 350, 200, 0x78123456);

	lRefresh(alp);

	lSaveImage(alp, L"withalpha.png", IMG_PNG|IMG_KEEPALPHA, 0, 0);
	lSaveImage(alp, L"withoutalpha.png", IMG_PNG, 0, 0);
	lSaveImage(alp, L"withoutalpha.bmp", IMG_BMP, 0, 0);
	lSaveImage(alp, L"withoutalpha.tga", IMG_TGA, 0, 0);

	lDeleteFrame(img);
	lDeleteFrame(alp);
	lDeleteFrame(icon);

	demoCleanup();
	return 1;
}
