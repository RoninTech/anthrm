
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

#include "mylcd.h"
#include "demos.h"


int main ()
{

	if (!initDemoConfig("config.cfg"))
		return 0;
		
	lSetBackgroundColour(hw, lGetRGBMask(frame, LMASK_BLACK));
	lSetForegroundColour(hw, lGetRGBMask(frame, LMASK_WHITE));
	lClearFrame(frame);
	frame->style = LSP_SET;
	
	TFRAME *txtsurface = lNewString(hw, DBPP, 0, LFT_GUNGSUHCHE24X36, " " libmylcdVERSION "   ");
	if (!txtsurface){
		demoCleanup();
		return 0;
	}

	lSetPixelWriteMode(txtsurface, LSP_AND);
	lLoadImageEx(txtsurface, L"images/somuchwin.bmp", 0, 0);

	TLSCROLLEX *s = lNewScroll(txtsurface, frame);
	if (!s){
		lDeleteFrame(txtsurface);
		demoCleanup();
		return 0;
	}
	
	
	s->dir = SCR_LEFT;
	s->desRect->x1 = 0;
	s->desRect->y1 = 0;
	s->desRect->x2 = DWIDTH-1;
	s->desRect->y2 = s->desRect->y1+txtsurface->height-1;
	s->flags = 0;

	int i=0;
	do{
		lClearFrame(frame);
		lUpdateScroll(s);
		lRefresh(frame);
		lSleep(20);
		i++;
		
	}while(i < s->srcFrm->width && !kbhit());

	lDeleteScroll(s);
	lDeleteFrame(txtsurface);
	demoCleanup();
	return 0;
}

/*	lSaveImage(s->srcFrm, L"src.bmp", IMG_BMP, 0, 0);
	lSaveImage(s->desFrm, L"des.bmp", IMG_BMP, 0, 0);
	lSaveImage(s->clientFrm, L"client.bmp", IMG_BMP, 0, 0);
*/


