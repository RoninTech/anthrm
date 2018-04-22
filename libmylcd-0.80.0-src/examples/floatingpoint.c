
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
#include <string.h>
#include <math.h>

#include "mylcd.h"
#include "demos.h"
                   

int main ()
{
	if (!initDemoConfig("config.cfg"))
		return 0;

	lSetForegroundColour(hw, lGetRGBMask(frame, LMASK_BLACK));
	lSetBackgroundColour(hw, lGetRGBMask(frame, LMASK_WHITE));

	int x,y;
	float r,g,b;

	float i;
	for (i = 0.001; i < 4.0; i += 0.002){
		for (y = 0; y < frame->height; ++y){
			for (x = 0; x < frame->width; ++x){
				r = 0.1f + (x/i + y*i) * 0.0015f;
				g = 0.5f + (x*i + y) * 0.0010f;
				b = 0.7f + (x + y) * 0.0005f;
				lSetPixelf(frame, x, y, r, g, b);
			}
		}
		lRefresh(frame);
	}
	lSaveImage(frame, L"fp.png", IMG_PNG, frame->width, frame->height);
	lSleep(2000);
	demoCleanup();
	return 0;
}



