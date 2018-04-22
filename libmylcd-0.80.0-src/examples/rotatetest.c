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



#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "mylcd.h"
#include "demos.h"


int main ()
{
	
	if (!initDemoConfig("config.cfg"))
		return 0;
		
	lSetBackgroundColour(hw, lGetRGBMask(frame, LMASK_BLACK));
	lSetForegroundColour(hw, lGetRGBMask(frame, LMASK_WHITE));
	
	TFRAME *amd = lNewImage(hw, L"images/cspread.png", DBPP);

    float a;
	float zoom;
	float flength=260.0;
	int destx=DWIDTH/2;
	int desty=DHEIGHT/2;
 	int tframes = 0;
	int start = GetTickCount();
    
	for (zoom=-260;zoom>-350;zoom--){
    	lClearFrame(frame);
    	lRotateFrameEx(amd, frame, 0.0, 0.0, 0.0, flength, zoom, destx, desty);
    	lRefresh(frame);
    	//lSleep(10);
    	tframes++;
    	if (kbhit()) break;
	}

	float step = 0.50;
    for (a = 0.0; a<720.0+step; a += step){
    	lClearFrame(frame);
    	lRotateFrameEx(amd, frame, a*2.0, a/2.0, a, flength*3.0, zoom*3.0, destx, desty);
		lRefresh(frame);
		//lSleep(10);
		tframes++;
		if (kbhit()) break;
    }


	for (zoom=-350;zoom<-259;zoom++){
    	lClearFrame(frame);
    	lRotateFrameEx(amd, frame, 0.0, 0.0, 0.0, flength, zoom, destx, desty);
		lRefresh(frame);
    	//lSleep (10);
    	tframes++;
	}

  	for (a = 0.0; a <= 360.0; a += 1.0){
  		lClearFrame(frame);
    	lRotate(amd, frame, 0, 0, a);
		lRefresh(frame);
    	tframes++;
    	if (kbhit()) break;
  	}

  	int end = GetTickCount();
	float time = (float)(end-start)*0.001;
	printf("frames:%d\ntime:%.2fs\nfps:%.1f\n\n",tframes,time,tframes/time);
	
	lDeleteFrame(amd);
	demoCleanup();

    return 0;
}

