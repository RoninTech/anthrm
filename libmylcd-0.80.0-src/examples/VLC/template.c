
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



int button (const TTOUCHCOORD *pos, TBUTTON *button, const int id, const int message, void *ptr)
{
	TVLCPLAYER *vp = (TVLCPLAYER*)ptr;
		
	switch (id){
	  //case 
	 }
	return 1;
}	  
	  
int close  (TVLCPLAYER *vp, TFRAME *frame, void *ptr)
{
	return 1;
}

int open (TVLCPLAYER *vp, TFRAME *frame, void *ptr)
{	
	return 1;
}

int draw (TVLCPLAYER *vp, TFRAME *frame, void *ptr)
{
	return 1;
}

int touch (TTOUCHCOORD *pos, int flags, TVLCPLAYER *vp)
{
	TBUTTONS *buttons = buttonsGetContainer(vp, PAGE_ );
	TBUTTON *button = buttons->list;
	TTOUCHCOORD bpos;

	for (int i = 0; i < buttons->total; i++, button++){
		if (button->enabled){
			if (pos->x >= button->pos.x && pos->x <= button->pos.x+button->image->width){
				if (pos->y >= button->pos.y && pos->y <= button->pos.y+button->image->height){
					if (button->highlight)
						enableHighlight(vp, button);
					if (button->canAnimate)
						button->ani.state = 1;

					my_memcpy(&bpos, pos, sizeof(TTOUCHCOORD));
					bpos.x = pos->x - button->pos.x;
					bpos.y = pos->y - button->pos.y;
					if (!button->callback(&bpos, button, button->id, flags, vp))
						break;
				}
			}
		}
	}

	return 1;
}

