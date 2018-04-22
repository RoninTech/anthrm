
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


#define MAXSTARS 256

typedef struct{
	float x;
	float y;
	ubyte plane;
}TStar;

int main (int argc, char* argv[])
{

	if (!initDemoConfig("config.cfg"))
		return 0;

	lSetBackgroundColour(hw, lGetRGBMask(frame, LMASK_BLACK));
	lSetForegroundColour(hw, lGetRGBMask(frame, LMASK_WHITE));
	
	int amask;
	if (DBPP == LFRM_BPP_32A)
		amask = 0xFF000000;
	else
		amask = 0;
	
    TStar *stars = (TStar*)calloc(sizeof(TStar), MAXSTARS);

	int i;
    for (i=0; i<MAXSTARS; i++){
        stars[i].x = rand() % frame->width;
        stars[i].y = rand() % frame->height;
        stars[i].plane = rand() % 3;     // star colour between 0 and 2
    }
    
    int colour[4] = {0, 0xFFFFFF/3, 0xFFFFFF/2, 0xFFFFFF};
	
	while (!kbhit()){
		lClearFrame(frame);

		for (i=0; i<MAXSTARS; i++){
			stars[i].x += (1+(float)stars[i].plane)*0.15;
			if (stars[i].x>frame->width){
				stars[i].x = 0;
				stars[i].y = rand() % frame->height;
			}
			lSetPixel(frame, (int)stars[i].x, (int)stars[i].y, amask|colour[1+stars[i].plane]);
		}
		lRefresh(frame);
		lSleep(5);
	}
	
	//lSaveImage(frame, L"stars.png", IMG_PNG, 0, 0);
	
	free(stars);
	demoCleanup();
	return 1;
}
