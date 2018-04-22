
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

static int cool_random ( )
{
    int n = rand () % 3;
    switch ( n )
    {
    case 0:
		return -1;
    case 1:
		return 0;
    case 2:
		return 1;
    }
    return 0;
}

static void Heat (TFRAME *src, TFRAME *dst)
{
	int x,y;
	int i,c;
	// add some random hot spots
	for (y=45; y<150; y++){
		for (x=1; x<src->width-1; x++){
			c = lGetPixel_NB(src, x, y);
			//if ((uint32_t)c > (uint32_t)lGetPixel_NB(dst, x, y))
			if (c)
				lSetPixel_NB(dst, x, y, rand()&c);
		}
	}
	
	// add some random hot spots at the bottom of the buffer
	y = dst->height-1;
	x = dst->width;

	while(x--){
		i = rand()%dst->width;
		c = rand()&255;
		lSetPixel_NB(dst, i, y, c);
		lSetPixel_NB(dst, i, y-1, c-4);
	}
}

/*
 * smooth a buffer upwards, make sure not to read pixels that are outside of
 * the buffer!
 */
static void Blur_Up (TFRAME *src, TFRAME *dst)
{
	int b;
	int x,y;
	const int p = src->pitch;
	const int w = dst->width;
	const int h = dst->height;
	const ubyte *buffer = lGetPixelAddress(src, 0, 0);
	int wd = 1;
	
	for (y=1; y<h-1; y++){
		for (x=1; x<w-1; x++){
			// calculate the average
		#if 1
			b =(
				buffer[(y)  *p + (x-1)]*wd                           + buffer[(y)  *p + (x+1)]*wd
			  + buffer[(y+1)*p + (x-1)]*wd + buffer[(y+1)*p + (x)]   + buffer[(y+1)*p + (x+1)]*wd
			  + buffer[(y+2)*p + (x-1)]*wd + buffer[(y+2)*p + (x)]*5 + buffer[(y+2)*p + (x+1)]
			    )/(7+(wd*5));
			b--;
			wd ^= 1;
			
			if (b < 0) b = 0;
			lSetPixel_NB(dst, x, y, b);
		#endif

		#if 0
        	b = buffer[y*p + x];
            b += buffer[(y+1)*p + x];
            b >>= 1;
            b--;
            
            if (b < 0) b = 0;
            lSetPixel(dst, x + cool_random(), y-1, b);
       	#endif
       	
       	#if 0
       		b = buffer[y*p + x];
       		b -= 2;
       		
       		if (b < 0) b = 0;
            lSetPixel(dst, x + cool_random(), y-1, b);
		#endif

       	#if 0
       		b = buffer[y*p + x];
       		b -= 4;
       		
       		if (b < 0) b = 0;
            lSetPixel_NB(dst, x, y-1, b);
		#endif
		
		#if 0
        	b = buffer[y*p + x];
            b += buffer[(y+1)*p + x];
            b += buffer[y*p + (x+1)];
            b += buffer[y*p + (x-1)];
            b >>= 2;
            b -= 1;
            
            if (b < 0) b = 0;
			lSetPixel_NB(dst, x, y-1, b);
		#endif

			// decrement the sum by one so that the fire loses intensity
			//if (b < 0) b = 0;
			//lSetPixel_NB(dst, x, y-1, b);
			//lSetPixel(dst, x + cool_random (), y-1, b);
			
		}
		// set first pixel of the line to 0
		lSetPixel_NB(dst, 0, y, 0);
		// set last pixel of the line to 0
		lSetPixel_NB(dst, w-1, y, 0);
	}
	// clear the last 2 lines
	for (y=h-2; y<h; y++){
		for (x=0; x<w; x++){
			lSetPixel_NB(dst, x, y, 0);
		}
	}
}

static void addPalette (TFRAME *src, TFRAME *dst, TFRAME *pal)
{
	int x,y,c;
	const ubyte *srcBuffer = lGetPixelAddress(src, 0, 0);
	const int sp = src->pitch;
	//int *desBuffer = lGetPixelAddress(dst, 0, 0);
	//const int w = dst->pitch;
	
	for (y = 0; y < dst->height; y++){
		for (x = 0; x < dst->width; x++){
			c = srcBuffer[y*sp + x] << 1;
			if (c&0xFFFFFF00) c = 255;
			c = lGetPixel_NB(pal, 0, c);
			lSetPixel_NB(dst, x, y, c);
			//desBuffer[y*w + x] = c;
		}
	}
}

int main (int argc, char* argv[])
{

	if (!initDemoConfig("config.cfg"))
		return 0;

	lSetBackgroundColour(hw, lGetRGBMask(frame, LMASK_BLACK));
	lSetForegroundColour(hw, lGetRGBMask(frame, LMASK_WHITE));

	TFRAME *fire1 = lNewFrame(hw, DWIDTH, DHEIGHT, LFRM_BPP_8);
	TFRAME *fire2 = lNewFrame(hw, DWIDTH, DHEIGHT, LFRM_BPP_8);
	TFRAME *fireA = fire1;
	TFRAME *fireB = fire2;

	TFRAME *image = lNewImage(hw, L"images/amd.png", LFRM_BPP_8);

	// load 32bit palette then convert to destination bpp
	TFRAME *tmppal = lNewImage(hw, L"images/flame.png", LFRM_BPP_32A);

	
	TFRAME *pal = lNewFrame(hw, tmppal->width, tmppal->height, DBPP);
	pConverterFn fn = lGetConverter(hw, tmppal->bpp, pal->bpp);
	fn(tmppal, lGetPixelAddress(pal, 0, 0));
	lDeleteFrame(tmppal);

	const uint64_t startTime = GetTickCount();
	int tframes = 0;
	
	while (!kbhit()){
		// heat the fire
		Heat(image, fireA);
		// apply the filter
		Blur_Up(fireA, fireB);
		addPalette(fireB, frame, pal);
		lRefreshAsync(frame, 1);

		// swap the fire buffers
		if (fireA == fire1){
			fireA = fire2;
			fireB = fire1;
		}else{
			fireA = fire1;
			fireB = fire2;
		}
		tframes++;
		//lSleep(10);
    }
    const uint64_t endTime = GetTickCount();
	printf("%.2ffps %i %i\n", tframes/((float)(endTime-startTime)/1000.0), tframes, (int)(endTime-startTime));
	
	//lSaveImage(frame, L"fire.png", IMG_PNG, 0, 0);
	
	lDeleteFrame(fire2);
	lDeleteFrame(fire1);
	lDeleteFrame(pal);
	lDeleteFrame(image);
	demoCleanup();
	return 1;
}
