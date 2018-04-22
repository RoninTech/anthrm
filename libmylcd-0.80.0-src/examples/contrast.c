
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


#include "mylcd.h"
#include "demos.h"

int main (int argc, char* argv[])
{
	if (!initDemoConfig("config.cfg"))
		return 0;


	/*
	set contrast.
	PCF8833 range: 0 to 63.
	S1D15G10 range: 0 to 63 for coarse and 0 to 7 for fine. contrast = ((coarse<<8)|fine)
	*/
	intptr_t value = 0;	
	int disp = lDriverNameToID(hw, "PCF8833:SPI", LDRV_DISPLAY);
	if (disp){
		lSetDisplayOption(hw, disp, lOPT_PCF8833_CONTRAST, &value);	// remove contrast before updating display
		lSleep(1000);
		lLoadImage(frame, L"images/elements-02.bmp");
		lRefresh(frame);
		TDRIVER *drv = lDisplayIDToDriver(hw, disp);	// acquire internal driver handle
		
		intptr_t i, v = 0;
		for (i = 20; i < 64; i++){
			drv->dd->setOption(drv, lOPT_PCF8833_CONTRAST, &i);
			drv->dd->getOption(drv, lOPT_PCF8833_CONTRAST, &v);
			printf("contrast: %i\n",v);
			lSleep(100);
		}

		lSleep(1000);
		value = 58;
		lSetDisplayOption(hw, disp, lOPT_PCF8833_CONTRAST, &value);	// 58 works for me
	}
	demoCleanup();
	return 1;
}
