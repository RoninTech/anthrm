
// libmylcd - http://mylcd.sourceforge.net/
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
//  GNU LIBRARY GENERAL PUBLIC LICENSE for more details.
//
//	You should have received a copy of the GNU Library General Public
//	License along with this library; if not, write to the Free
//	Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#ifndef _MYLCDSETUP_H_
#define _MYLCDSETUP_H_



#define DISPLAYMAX	4

const char *device[] = {"G19", "DDRAW"};

THWD *hw = NULL;
TFRAME *frame = NULL;
static TRECT displays[DISPLAYMAX];
static TRECT *display;

int DWIDTH = 320;
int DHEIGHT	= 240;
int DDATA = 0;
int DBPP = LFRM_BPP_32;


int initMYLCD ()
{
	
	mylog("initLibrary: starting libmylcd...\n");
    if (!(hw=lOpen(NULL, NULL))){
    	mylog("initLibrary: lOpen() failed\n");
    	return 0;
    }

	mylog("initLibrary: requesting global display surface\n");
    if (!(frame=lNewFrame(hw, DWIDTH, DHEIGHT, DBPP))){
    	lClose(hw);
    	mylog("initLibrary: lNewFrame() failed\n");
    	return 0;
    }else{
    	//lClearFrame(frame);
		//lRefresh(frame);
		display = displays;
		mylog("initLibrary: libmylcd started successfully\n");
    	return 1;
    }
}

void closeMYLCD_G19 ()
{

	mylog("cleanup: mylcd.dll shutting global frame handle\n");
	if (frame){
		lDeleteFrame(frame);
		frame = NULL;
	}
	mylog("cleanup: libmylcd closing device handle\n");
	if (hw){
		lClose(hw);
		hw = NULL;
	}
	mylog("cleanup: libmylcd shutdown\n");
}

int initMYLCD_G19 ()
{
	if (!initMYLCD())
		return 0;
		
	memset(displays, 0, sizeof(TRECT) * DISPLAYMAX);
	displays[0].left = 0;
	displays[0].top = 0;
	displays[0].right = DWIDTH-1;
	displays[0].btm = DHEIGHT-1;

	mylog("starting G19..\n");
	int ret = lSelectDevice(hw, device[0], "NULL", DWIDTH, DHEIGHT, DBPP, 0, &displays[0]);
	if (!ret)
		ret = lSelectDevice(hw, device[1], "NULL", DWIDTH, DHEIGHT, DBPP, 0, &displays[0]);
		
	if (ret){
		mylog("G19 started\n");
		lSetBackgroundColour(hw, lGetRGBMask(frame, LMASK_BLACK));
		lSetForegroundColour(hw, lGetRGBMask(frame, LMASK_WHITE));
		lSetPixelWriteMode(frame, LSP_SET);
		return ret;
	}else{
		mylog("G19 start failed\n");
		return 0;
	}
}

#endif

