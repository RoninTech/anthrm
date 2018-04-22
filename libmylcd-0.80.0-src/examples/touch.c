
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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <windows.h>

#include "mylcd.h"
#include "demos.h"


#define PIXELDELTA		15		/* 15 lcd pixels*/
#define DEBOUNCETIME	100		/* 100 millisecond period*/

typedef struct {
	TTOUCHCOORD pos;
	TTOUCHCOORD d0;
	TTOUCHCOORD d1;
}TDRAGPOS;


static TDRAGPOS plpos;
TDRAGPOS *p = &plpos;
static int RED;
static int GREEN;
static int BLUE;


void drawBox (TFRAME *frame, int x, int y)
{
	lDrawRectangleFilled(frame, x-10, y-10, x+30, y+30, 0x0);
	lDrawRectangleFilled(frame, x, y, x+20, y+20, 0xFFFF);
}

void touchCB (TTOUCHCOORD *pos, int flags, void *ptr)
{
	//printf("CB: ct:%i, x:%i y:%i dt:%i, z1:%i z2:%i %i %i %i\n", pos->count, pos->x, pos->y, pos->dt, pos->z1, pos->z2, pos->pressure, flags, pos->pen);

	//if (!pos->pressure) return;

	if (!flags){
		lSetPixel(frame, pos->x, pos->y, BLUE);
		
	}else{
		if (pos->pen){
			lSetPixel(frame, pos->x, pos->y, GREEN);
			
		}else{
			lSetPixel(frame, pos->x, pos->y, RED);
			printf("CB: ct:%i, x:%i y:%i dt:%i, z1:%i z2:%i %i %i %i\n", pos->count, pos->x, pos->y, pos->dt, pos->z1, pos->z2, pos->pressure, flags, pos->pen);
		}
	}

	// detect if new drag
	if (!flags){
		p->pos.x = p->pos.x - p->d1.x;
		p->pos.y = p->pos.y - p->d1.y;
		p->d1.x = 0;
		p->d1.y = 0;
		p->d0.x = pos->x;
		p->d0.y = pos->y;
	}
	
	p->d1.x = p->d0.x - pos->x;
	p->d1.y = p->d0.y - pos->y;
	lRefreshAsync(frame, 0);
}

int main (int argc, char* argv[])
{
	if (!initDemoConfig("config.cfg"))
		return 0;

	lClearFrame(frame);
	lRefreshAsync(frame, 0);
	
	RED = lGetRGBMask(frame, LMASK_RED);
	GREEN = lGetRGBMask(frame, LMASK_GREEN);
	BLUE = lGetRGBMask(frame, LMASK_BLUE);
	

	lDISPLAY did = lDriverNameToID(hw, "USBD480:LIBUSBHID", LDRV_DISPLAY);
	if (!did)
		did = lDriverNameToID(hw, "USBD480:LIBUSB", LDRV_DISPLAY);

	if (!did)
		did = lDriverNameToID(hw, "USBD480:DLL", LDRV_DISPLAY);
				
	if (did){
		lSetDisplayOption(hw, did, lOPT_USBD480_TOUCHCB, (intptr_t*)touchCB);

		while (!kbhit())
			lSleep(500);
	}
	
	lSetDisplayOption(hw, did, lOPT_USBD480_TOUCHCB, 0);

//	lSaveImage(frame, L"touch.png", IMG_PNG, 0, 0);
	demoCleanup();

	return 1;
}
