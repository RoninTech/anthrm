
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
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

#include "mylcd.h"
#include "images/child.h"
#include "images/girl.h"
#include "images/startframe.h"
#include "demos.h"



void frame12To16 (TFRAME *src, TFRAME *des)
{
	pConverterFn convert = lGetConverter(src->hw, LFRM_BPP_12, LFRM_BPP_16);
	if (convert)
		convert(src, lGetPixelAddress(des, 0, 0));
}

static wchar_t wreverse16 (wchar_t a)
{
	const wchar_t b = (a&0xFF00)>>8;
	return (b|(a&0x00FF)<<8);
}

static void loadBuffer1 (TFRAME *frame, ubyte *buffer, int size)
{
	char *des = (char*)lGetPixelAddress(frame, 0, 0);
	char *src = (char*)buffer;
	
	int tpixels = frame->height*(frame->width/8);
	while(tpixels--)
		des[tpixels] = src[tpixels];
}

static void loadBufferRev16 (TFRAME *frame, ubyte *buffer, int size)
{
	wchar_t *des = (wchar_t*)lGetPixelAddress(frame, 0, 0);
	wchar_t *src = (wchar_t*)buffer;
	
	int tpixels = frame->height*frame->width;
	while(tpixels--)
		des[tpixels] = wreverse16(src[tpixels]);
}

static void displayBuffer1 (void *buffer, size_t size, int w, int h)
{
	TFRAME *src = lNewFrame(hw, w, h, LFRM_BPP_1);
	TFRAME *des = lNewFrame(hw, w, h, DBPP);
	loadBuffer1(src, buffer, size);
	
	lFrame1BPPToRGB(src, lGetPixelAddress(des, 0, 0), DBPP, 
	  lGetRGBMask(des, LMASK_YELLOW), lGetRGBMask(des, LMASK_MAGENTA));
	
	lRefresh(des);
	lDeleteFrame(des);
	lDeleteFrame(src);
	lSleep(1500);
}

static void displayBuffer16 (void *buffer, size_t size, int w, int h, int bpp)
{
	TFRAME *tmp = lNewFrame(hw, w, h, bpp);
	loadBufferRev16(tmp, buffer, size);
	lRefresh(tmp);
	lDeleteFrame(tmp);
	lSleep(700);
}

static void displayImage (TFRAME *frame, wchar_t *path, int type)
{
	wprintf(L"'%s'\n", path);
	
	if (!lLoadImage(frame, path))
		wprintf(L"'%s' failed\n", path);
	lRefresh(frame);
	lSleep(700);
}


int main ()
{
	if (!initDemoConfig("config.cfg"))
		return 0;

	lClearFrame(frame);
	lRefresh(frame);


#if 1

	displayBuffer1(startframe160x43, sizeof(startframe160x43), 160, 43);
	displayBuffer16(child130x130_12, sizeof(child130x130_12), 130, 130, LFRM_BPP_12);
	displayBuffer16(girl130x130_12, sizeof(girl130x130_12), 130, 130, LFRM_BPP_12);
	
	displayImage(frame, L"images/RGB_24bits_palette_bird.bmp", IMG_BMP);
	displayImage(frame, L"images/RGB_8bits_palette.bmp", IMG_BMP);
	displayImage(frame, L"images/RGB_24bits_palette.bmp", IMG_BMP);
	displayImage(frame, L"images/RGB_24bits_palette.tga", IMG_TGA);
	displayImage(frame, L"images/RGB_12bits_palette.bmp", IMG_BMP);
	displayImage(frame, L"images/RGB_12bits_palette.tga", IMG_TGA);

	displayImage(frame, L"images/girl.bmp", IMG_BMP);
	displayImage(frame, L"images/apple_eye.bmp", IMG_BMP);
			
	displayImage(frame, L"images/somuchwin.bmp", IMG_BMP);
	displayImage(frame, L"images/24bpp_somuchwin.bmp", IMG_BMP);
	displayImage(frame, L"images/8bpp_somuchwin.bmp", IMG_BMP);
	
	displayImage(frame, L"images/IndexedColorSample_(Caerulea).bmp", IMG_BMP);
	displayImage(frame, L"images/IndexedColorSample_(Lapis.elephant).bmp", IMG_BMP);
	displayImage(frame, L"images/IndexedColorSample_(Lemon).bmp", IMG_BMP);
	displayImage(frame, L"images/IndexedColorSample_(Strawberries).bmp", IMG_BMP);
	displayImage(frame, L"images/IndexedColorSample_(Mosaic).bmp", IMG_BMP);
	
	displayImage(frame, L"images/Silver_Dream.bmp", IMG_BMP);
	displayImage(frame, L"images/Messenger.bmp", IMG_BMP);

	displayImage(frame, L"images/RGBR.bmp", IMG_BMP);
	displayImage(frame, L"images/img_7627_1.bmp", IMG_BMP);
	displayImage(frame, L"images/rgb.bmp", IMG_BMP);
	
	displayImage(frame, L"images/32bpp_alpha.tga", IMG_TGA);
	displayImage(frame, L"images/RGB_Space.bmp", IMG_BMP);
	displayImage(frame, L"images/elements-02.bmp", IMG_BMP);
	
	displayImage(frame, L"images/111823.bmp", IMG_BMP);
	displayImage(frame, L"images/92.bmp", IMG_BMP);
	displayImage(frame, L"images/flowers.bmp", IMG_BMP);
	displayImage(frame, L"images/child.bmp", IMG_BMP);
	
#endif	

//	lSleep(2000);
	demoCleanup();
	return 1;
	
}

