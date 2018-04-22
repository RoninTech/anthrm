
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



int main (int argc, char* argv[])
{
	if (argc > 1){
		if (!initDemoConfig("config.cfg"))
			return 0;

		// USBD480 must be enabled in config.cfg
		lDISPLAY disp = lDriverNameToID(hw, "USBD480:LIBUSBHID", LDRV_DISPLAY);
		if (!disp)
			disp = lDriverNameToID(hw, "USBD480:LIBUSB", LDRV_DISPLAY);
		if (!disp)
			disp = lDriverNameToID(hw, "USBD480:DLL", LDRV_DISPLAY);

		if (disp){
			intptr_t value = strtol(argv[1], NULL, 10)&0xFF;
			lSetDisplayOption(hw, disp, lOPT_USBD480_BRIGHTNESS, &value);
		
			/*
			TDRIVER *drv = lDisplayIDToDriver(hw, disp);	// acquire internal driver handle
			intptr_t i, v;
			for (i = 0; i < 256; i++){
				drv->dd->setOption(drv, lOPT_USBD480_BRIGHTNESS, i);
				drv->dd->getOption(drv, lOPT_USBD480_BRIGHTNESS, &v);
				printf("brightness: %i\n",v);
				lSleep(10);
			}
			*/
		}
		demoCleanup();
	}
	
	return 1;
}
