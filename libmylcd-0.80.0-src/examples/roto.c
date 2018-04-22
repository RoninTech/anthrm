
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


#define DEGTORAD 0.034906585039


static void textureScreen (TFRAME *src, TFRAME *des, int Ax, int Ay, int Bx, int By, int Cx, int Cy)
{
	const int dxdx = (Bx-Ax)/des->width;
	const int dydx = (By-Ay)/des->width;
	const int dxdy = (Cx-Ax)/des->height;
	const int dydy = (Cy-Ay)/des->height;
	const int w = src->width;
	const int h = src->height;
	const int wh = w*h;
	int colour,x,y;
	

	for (y=0; y<des->height; y++){
		Cx = Ax;
		Cy = Ay;
		for (x=0; x<des->width; x++){
			colour = lGetPixel_NB(src, ((Cy/h)/w)&511, (Cx/wh)&511); // 511 for texture size of 512x512
			//x2 = (Cx/wh)&511;
			//y2 = ((Cy/h)/w)&511;
			//buffer2[y*p + x] = buffer[y2*w + x2];
			lSetPixel_NB(des, x, y, colour);
              
			//interpolate to get next texel in texture space
			Cx += dxdx;
			Cy += dydx;
		}
		// interpolate to get start of next line in texture space
		Ax += dxdy;
		Ay += dydy;
	}
}

static void doRotoZoom (TFRAME *src, TFRAME *des, float cx, float cy, float radius, float angle)
{
	const float fact = des->width*des->height;
	const int x1 = (int)(fact * (cx + radius * cosf(angle)));
	const int y1 = (int)(fact * (cy + radius * sinf(angle)));
	const int x2 = (int)(fact * (cx + radius * cosf(angle + 2.0/*2.02458*/)));
	const int y2 = (int)(fact * (cy + radius * sinf(angle + 2.0/*2.02458*/)));
	const int x3 = (int)(fact * (cx + radius * cosf(angle - 1.0/*1.11701*/)));
	const int y3 = (int)(fact * (cy + radius * sinf(angle - 1.0/*1.11701*/)));
	textureScreen(src, des, x1, y1, x2, y2, x3, y3);
}

int main (int argc, char* argv[])
{

	if (!initDemoConfig("config.cfg"))
		return 0;

	lSetBackgroundColour(hw, lGetRGBMask(frame, LMASK_BLACK));
	lSetForegroundColour(hw, lGetRGBMask(frame, LMASK_WHITE));

	TFRAME *src = lNewImage(hw, L"images/usm.png", DBPP);
	
	const uint64_t startTime = GetTickCount();
	uint64_t currentTime;
	int tframes = 0;

	while (!kbhit()){
		currentTime = GetTickCount()<<3;
		
	  #if 0
		doRotoZoom(src, frame, 256, 256, 1096.0, 360.0*DEGTORAD);
	  #else
		doRotoZoom(src, frame,
		  4096 * sinf((float)currentTime/21547814.0f),		// X centre coord
		  4096 * cosf((float)currentTime/18158347.0f),		// Y centre coord
		  1024.0f+384.0f*cos((float)currentTime/42104.0f),	// zoom coef
		  (float)(currentTime)/7312.0f);					// angle
	  #endif
          lRefresh(frame);
          tframes++;
    }
	const uint64_t endTime = GetTickCount();
	printf("%.2ffps %i %i\n", tframes/((float)(endTime-startTime)/1000.0), tframes, (int)(endTime-startTime));
    demoCleanup();
	return 0;
}



