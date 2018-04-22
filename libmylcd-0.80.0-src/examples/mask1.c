
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

static int RED;
static int GREEN;
static int BLUE;
static int WHITE;
static int BLACK;

int main(int argc, char* argv[])
{


	if (!initDemoConfig("config.cfg"))
		return 0;

	RED = lGetRGBMask(frame, LMASK_RED);
	GREEN = lGetRGBMask(frame, LMASK_GREEN);
	BLUE = lGetRGBMask(frame, LMASK_BLUE);
	BLACK = lGetRGBMask(frame, LMASK_BLACK);
	WHITE = lGetRGBMask(frame, LMASK_WHITE);

	lSetBackgroundColour(hw, BLACK);
	lSetForegroundColour(hw, WHITE);

	const int third = frame->width/3.0;
	TFRAME *src = lNewImage(hw, L"images/cube.png", DBPP);
	TFRAME *mask = lNewFrame(hw,DWIDTH,DHEIGHT, DBPP);
		
	lDrawCircleFilled(mask,63,31,30, WHITE);
	lDrawCircleFilled(mask,63,31,15, BLACK);
	lDrawRectangleFilled(mask, 0, frame->height/2, frame->width-1, frame->height-1, WHITE);
	lDrawRectangleFilled(mask, 0.0*third, frame->height/2, (0*third)+third, frame->height-30, BLUE);
	lDrawRectangleFilled(mask, 1.0*third, frame->height/2, (1*third)+third, frame->height-30, GREEN);
	lDrawRectangleFilled(mask, 2.0*third, frame->height/2, (2*third)+third, frame->height-30, RED);
	lDrawRectangleFilled(mask, 31, frame->height-48, frame->width-30, frame->height-15, BLACK);
	lDrawRectangleDottedFilled(mask, 30, frame->height-48, frame->width-30, frame->height-15, WHITE);

	lClearFrame(frame);
	lDrawMask(src, mask, frame, 0, 0, LMASK_AND);
	lRefresh(frame);
	lDeleteFrame(src);
	lDeleteFrame(mask);

	demoCleanup();
	return 1;
}


