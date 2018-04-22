
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




int main(int argc, char* argv[])
{

	if (!initDemoConfig("config.cfg"))
		return 0;
		
	lSetBackgroundColour(hw, lGetRGBMask(frame, LMASK_WHITE));
	lSetForegroundColour(hw, lGetRGBMask(frame, LMASK_BLACK));
	
	lClearFrame(frame);
	// dnumber = display id number as returned by lSelectDisplay()
	// intptr_t mode = 0;
	// lSetDisplayOption(hw, dnumber, lOPT_LEDCARD_MODE, &mode);
	// mode = 1:fixed, 0:scroll

	//lLoadImage(frame, L"images/amd.bmp", IMG_BMP, LSP_SET);
	
	lPrint(frame, libmylcdVERSION, 4, 0, LFTW_SNAP, LPRT_CPY);
	lPrint(frame, libmylcdVERSION, 4, 10, LFTW_SNAP/*LFT_04B2_10*/, LPRT_CPY);
	lDrawRectangle(frame,0,0,frame->width-1, frame->height-1, 1);
	
	lRefresh(frame);
		
	lSaveImage(frame, L"1.bmp", IMG_BMP, 0, 0);
	lSaveImage(frame, L"1.png", IMG_PNG, 0, 0);
	lSaveImage(frame, L"1.tga", IMG_TGA, 0, 0);
	
	//lSleep(100);
	
	demoCleanup();
	return 1;
}

