
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


void precalculate (TFRAME *buf1, TFRAME *buf2)
{
    int x, y;
    int c;
    
    const int w = buf1->width>>1;
    const int h = buf1->height>>1;
    
    for (y=0; y<buf1->height; y++){
		for (x=0; x<buf1->width; x++){
			c = (ubyte)(64 + 63 * (sin((double)hypot(h-y, w-x)/16)));
			lSetPixel_NB(buf1, x, y, c);
			
			c = (ubyte)(64 + 63 * sin((float)x/(37+15*cos((float)y/74)))
                                        * cos((float)y/(31+11*sin((float)x/57))));
			lSetPixel_NB(buf2, x, y, c);
        }
    }
}

int main (int argc, char* argv[])
{

	if (!initDemoConfig("config.cfg"))
		return 0;

	lSetBackgroundColour(hw, lGetRGBMask(frame, LMASK_BLACK));
	lSetForegroundColour(hw, lGetRGBMask(frame, LMASK_WHITE));

	if (DBPP == LFRM_BPP_32A){
		DBPP = LFRM_BPP_32;
		lDeleteFrame(frame);
		frame = lNewFrame(hw, DWIDTH, DHEIGHT, DBPP);
	}
	
	
    TFRAME *plasma1 = lNewFrame(hw, DWIDTH*2, DHEIGHT*2, DBPP);
    TFRAME *plasma2 = lNewFrame(hw, DWIDTH*2, DHEIGHT*2, DBPP);
    precalculate(plasma1, plasma2);

	TFRAME *pal = lNewFrame(hw, 8, 256, DBPP);
    //lLoadImage(pal, L"images/pal.png");

	TFRAME *image = lNewString(hw, DBPP, 0, LFTW_KOI12X24B, "libmylcd %s", lVersion());
	//TFRAME *image = lNewFrame(hw, 8, 8, DBPP);
    //lLoadImage(image, L"images/amd.bmp");

    int x1, y1, x2, y2, x3, y3;
    int i,x,y;
	int r,g,b;
    int colour, currentTime, tframes = 0;
    const float width = (frame->width/2)-1;
    const float height = (frame->height/2)-1;
    const int width2 = width/2;
    const int height2 = height/2;
    const uint64_t startTime = GetTickCount();
        
    float pifact[256];
    for (i=0; i<256; i++)
    	pifact[i] = (float)i * M_PI / 128.0;
    	
    
	while (!kbhit()){
		currentTime = GetTickCount()>>4;

		for (i=0; i<256; i++){
			r = ((ubyte)(32 + 31 * cos(pifact[i] + (double)currentTime/74.0)))<<2;
			g = ((ubyte)(32 + 31 * sin(pifact[i] + (double)currentTime/63.0)))<<2;
			b = ((ubyte)(32 - 31 * cos(pifact[i] + (double)currentTime/81.0)))<<2;
			lSetPixel_NB(pal, 1, i, r<<16|g<<8|b);
		}
		x1 = width+1 + (int)(width * cos((double)currentTime/97.0));
		x2 = width+1 + (int)(width * sin((double)-currentTime/114.0));
		x3 = width+1 + (int)(width * sin((double)-currentTime/137.0));
		y1 = height+1 + (int)(height * sin((double)currentTime/123.0));
		y2 = height+1 + (int)(height * cos((double)-currentTime/75.0));
		y3 = height+1 + (int)(height * cos((double)-currentTime/108.0));

		for (y = 0; y<frame->height; y++){
			for (x = 0; x<frame->width; x++){
				colour = lGetPixel(plasma1, x1+x, y1);
				colour += lGetPixel(plasma2, x2+x, y2);
				colour += lGetPixel(plasma2, x3+x, y3);
             	colour += lGetPixel(image, x-width2, y-height2)&(x+y);

				//colour = lGetPixel(pal, 1, (colour&0xFF)>>1);
				colour = lGetPixel_NB(pal, 1, colour&0xFF);
				lSetPixel_NB(frame, x, y, colour);
			}
			y1++; y2++; y3++;
		}
		tframes++;
		lRefreshAsync(frame, 1);
		lSleep(20);
	}
	const uint64_t endTime = GetTickCount();
	printf("%.2ffps %i %i\n", tframes/((float)(endTime-startTime)/1000.0), tframes, (int)(endTime-startTime));
	
    lDeleteFrame(pal);
    lDeleteFrame(image);
	lDeleteFrame(plasma1);
	lDeleteFrame(plasma2);
    demoCleanup();
	return 0;
}