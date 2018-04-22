
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

#ifndef _DEMOS_H_
#define _DEMOS_H_


#include <string.h>
#include <conio.h>
#include <windows.h>
#include <fcntl.h>
 
#define DISPLAYMAX	8

typedef struct {
	int	dd;
	int	pd;
	int width;
	int height;
	int value;
}TMYLCDDEMO;

typedef struct{
	unsigned int tlines;
	ubyte	**lines;
	ubyte	*data;
}TASCIILINE;


TASCIILINE *readFileA (const char *filename);
void freeASCIILINE (TASCIILINE *al);
// #include "file.c"



THWD *hw = NULL;
TFRAME *frame = NULL;
TRECT displays[DISPLAYMAX];
TRECT *display;

static ubyte fontpath[MaxPath];
static ubyte mappath[MaxPath];

int DWIDTH = 160;
int DHEIGHT	= 43;
int DDATA = 0;
int DBPP = 0;



static void setBPP (int *bpp)
{
	switch (*bpp){
		case 1: *bpp = LFRM_BPP_1; break;
		case 8: *bpp = LFRM_BPP_8; break;
		case 12: *bpp = LFRM_BPP_12; break;
		case 15: *bpp = LFRM_BPP_15; break;
		case 16: *bpp = LFRM_BPP_16; break;
		case 24: *bpp = LFRM_BPP_24; break;
		case 32: *bpp = LFRM_BPP_32; break;
		default: *bpp = LFRM_BPP_1;
	}
}

int initLibrary ()
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
		display = displays;
		mylog("initLibrary: libmylcd started successfully\n");
    	return 1;
    }
}

void demoCleanup()
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

int initDemoConfig (char *configfile)
{
	if (!initLibrary())
		return 0;

	TASCIILINE *al = readFileA(configfile);
	if (al == NULL){
		mylog("initConfig: readFileA(%s) failed\n", configfile);
		return 0;
	}
	
	char dd[lMaxDriverNameLength*2];
	char pd[lMaxDriverNameLength*2];
	char *line = NULL;
	int data;
	int w, h;
	unsigned int i = 0;
	lDISPLAY d = 0;
	memset(displays, 0, sizeof(TRECT) * DISPLAYMAX);
	
	do{
		line = (char *)al->lines[i];
		if (!strncmp(line, "bpp=", 4)){
			sscanf(line+4, "%d", &DBPP);
			setBPP(&DBPP);
			if (frame->bpp != DBPP){
				lDeleteFrame(frame);
				frame = lNewFrame(hw, DWIDTH, DHEIGHT, DBPP);
			}
		}else if (!strncmp(line, "data=", 5)){
			sscanf(line+5, "%d", &data);
		}else if (!strncmp(line, "width=", 6)){
			sscanf(line+6, "%d", &w);
		}else if (!strncmp(line, "height=", 7)){
			sscanf(line+7, "%d", &h);
		}else if (!strncmp(line, "fontpath=", 9)){
			sscanf(line+9, "%s", (char*)&fontpath);
		}else if (!strncmp(line, "mappath=", 8)){
			sscanf(line+8, "%s", (char*)&mappath);
		}else if (!strncmp(line, "active=", 7)){
			if ((int)atoi(line+7)){
				DWIDTH = w; DHEIGHT = h; DDATA = data;
				//d += lSelectDevice(hw, (ubyte*)dd, (ubyte*)pd, DWIDTH, DHEIGHT, DBPP, DDATA, display);
				mylog("initConfig: activating display:'%s' control:'%s' width:%i height:%i\n", dd, pd, 1+display->right-display->left, 1+display->btm-display->top);
				int ret = lSelectDevice(hw, dd, pd, 1+display->right-display->left, 1+display->btm-display->top, DBPP, DDATA, display);
				if (!ret){
					mylog("initConfig: display failed to activate:'%s' control:'%s'\n", dd, pd);
				}else{
					d += ret;
				}
				display++;
			}
		}else if (!strncmp(line, "[display ", 9)){
			//j = atoi(line+9)-1;
			//if (j < DISPLAYMAX)
			//	display = &displays[j];
		}else if (!strncmp(line, "portdriver=", 11)){
			sscanf(line+11, "%s", pd);
		}else if (!strncmp(line, "displaydriver=", 14)){
			sscanf(line+14, "%s", dd);
		}else if (!strncmp(line, "displaywindow=", 14)){
			sscanf(line+14, "%d,%d,%d,%d", &display->left, &display->top, &display->right, &display->btm);
		}
	}while(++i < al->tlines);
	freeASCIILINE(al);

	if (d){
		lResizeFrame(frame, DWIDTH, DHEIGHT, 0);
		lSetCapabilities(hw, CAP_BACKBUFFER, CAP_STATE_ON);
		//lSetDisplayOption(hw, lDriverNameToID(hw, "PCD8544:SPI", LDRV_DISPLAY), lOPT_PCD8544_HFLIP, 1);
		//lSetDisplayOption(hw, lDriverNameToID(hw, "PCD8544:SIO", LDRV_DISPLAY), lOPT_PCD8544_HFLIP, 1);

		if ((i=lDriverNameToID(hw, "USB13700:EXP", LDRV_PORT))){
			TDRIVER *drv = lDisplayIDToDriver(hw, i);
			intptr_t value = 8;
			if (lDriverNameToID(hw, "PCD8544:SPI", LDRV_DISPLAY))
				drv->pd->setOption(drv, lOPT_USB13700EXP_FRMLENGTH, &value);	// set 8bit SPI for PCD8544
			
			value = 9;
			if (lDriverNameToID(hw, "PCF8814:SPI", LDRV_DISPLAY))
				drv->pd->setOption(drv, lOPT_USB13700EXP_FRMLENGTH, &value);	// set 9bit SPI for 3510 mono
			
			value = 9;
			if (lDriverNameToID(hw, "S1D15G14:SPI", LDRV_DISPLAY))
				drv->pd->setOption(drv, lOPT_USB13700EXP_FRMLENGTH, &value);	// set 9bit SPI for 3510 colour
			
			if (lDriverNameToID(hw, "PCF8833:SPI", LDRV_DISPLAY)){
				value = 8;
				drv->pd->setOption(drv, lOPT_USB13700EXP_FRMLENGTH, &value);	// set 9bit SPI for 3510 colour
				value = 9;
				drv->pd->setOption(drv, lOPT_USB13700EXP_FRMLENGTH, &value);	// set 9bit SPI for 6100
			}

			if (lDriverNameToID(hw, "S1D15G10:SPI", LDRV_DISPLAY)){
				value = 8;
				drv->pd->setOption(drv, lOPT_USB13700EXP_FRMLENGTH, &value);	// set 9bit SPI for 3510 colour
				value = 9;
				drv->pd->setOption(drv, lOPT_USB13700EXP_FRMLENGTH, &value);	// set 9bit SPI for 6100
			}
		}

		hw->render->backGround = lGetRGBMask(frame, LMASK_BLACK);
		hw->render->foreGround = lGetRGBMask(frame, LMASK_WHITE);
		frame->style = LSP_SET;
		//lClearFrame(frame);
		return d;
	}else{
		mylog("initConfig: exiting without activating a display\n");
		demoCleanup();
		return 0;
	}
}




#endif

