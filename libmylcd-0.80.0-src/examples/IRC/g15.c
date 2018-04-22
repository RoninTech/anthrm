
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


#include "irc.h"

extern THWD *hw;
extern int DWIDTH;
extern int DHEIGHT;
extern int DBPP;

int initG15 (TIRCMYLCD *irc, void *ptrG15keyCB)
{
	lDISPLAY g15id = lDriverNameToID(hw, "G15", LDRV_DISPLAY);
	if (g15id){
		TMYLCDG15 *mylcdg15 = NULL;
		intptr_t value = G15_PRIORITY_SYNC;
		lSetDisplayOption(hw, g15id, lOPT_G15_PRIORITY, &value);
		lSetDisplayOption(hw, g15id, lOPT_G15_SOFTKEYCB, (intptr_t*)ptrG15keyCB);
		lGetDisplayOption(hw, g15id, lOPT_G15_STRUCT, (intptr_t*)mylcdg15);
		if (mylcdg15 != NULL){
			mylcdg15->ptrUser = (void*)irc;
			return 1;
		}else{
			return 0;
		}
	}else{
		return -1;
	}
}

