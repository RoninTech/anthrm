
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


#include "common.h"



int buttonLorenz (const TTOUCHCOORD *pos, TBUTTON *button, const int id, const int message, void *ptr)
{
	//TVLCPLAYER *vp = (TVLCPLAYER*)ptr;
		
	switch (id){
	  //case 
	 }
	return 1;
}	  
	  
int closeLorenz  (TVLCPLAYER *vp, TFRAME *frame, TLORENZ *lor)
{
	if (lor->state)
		GLLorenzClose();
	lor->state = 0;
	return 1;
}

int openLorenz (TVLCPLAYER *vp, TFRAME *frame, TLORENZ *lor)
{	
	lor->state = 0;
	return 1;
}

int drawLorenz (TVLCPLAYER *vp, TFRAME *frame, TLORENZ *lor)
{
	if (vp->gui.page_gl != PAGE_LORENZ){
		vp->gui.page_gl = PAGE_LORENZ;
		
		if (lor->state)
			GLLorenzClose();
		GLLorenzInit(frame);
		lor->state = 1;
	}
	setAwake(vp);
	GLLorenzDraw(frame);
	return 1;
}

int touchLorenz (TTOUCHCOORD *pos, int flags, TVLCPLAYER *vp)
{
	//setPage(vp, PAGE_NONE);
	setPageSec(vp, -1);
	return 0;
}

