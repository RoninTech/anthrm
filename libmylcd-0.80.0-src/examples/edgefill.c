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



int main (int argc, char* argv[])
{

	if (!initDemoConfig("config.cfg"))
		return 0;


	int RED = lGetRGBMask(frame, LMASK_RED);
	int GREEN = lGetRGBMask(frame, LMASK_GREEN);
	//int BLUE = lGetRGBMask(frame, LMASK_BLUE);
	int BLACK = lGetRGBMask(frame, LMASK_BLACK);
	int WHITE = lGetRGBMask(frame, LMASK_WHITE);

	lSetBackgroundColour(hw, BLACK);
	lSetForegroundColour(hw, WHITE);
	lClearFrame(frame);
	lLoadImage(frame, L"images/elements-02.bmp");
	lDrawTriangle(frame, 30, 30, 120, 120, 30, 120, RED);
	lDrawRectangle(frame, 60, 68, 40, 100, RED);
	lDrawCircle(frame, 95, 95, 20, RED);	

	lEdgeFill(frame, 31, 60, GREEN, RED);
	lEdgeFill(frame, 95, 90, RED, RED);

	lRefresh(frame);
	lSaveImage(frame,L"edgefill.bmp", IMG_BMP,0, 0);

	lSleep(2000);
	demoCleanup();
	return 1;
}

