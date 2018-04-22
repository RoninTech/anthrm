
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


int buttonExit (const TTOUCHCOORD *pos, TBUTTON *button, const int id, const int message, void *ptr)
{
	TVLCPLAYER *vp = (TVLCPLAYER*)ptr;

	switch (id){
	  case EXITBUTTON_YES:
	  	setPageSec(vp, -1);
	  	exitAppl(vp);
	  	break;

	  case EXITBUTTON_NO:
	  	setPageSec(vp, -1);
	 }
	return 0;
}	

int closeExit  (TVLCPLAYER *vp, TFRAME *frame, TEXIT *exit)
{
	buttonsDeleteContainer(vp, PAGE_EXIT);
	return 1;
}

int openExit (TVLCPLAYER *vp, TFRAME *frame, TEXIT *exit)
{	
	
	buttonsCreateContainer(vp, PAGE_EXIT, EXITBUTTON_TOTAL);
	imageCacheAddImage(vp, L"exit2.png", SKINFILEBPP, &vp->gui.image[IMGC_EXIT]);
	imageCacheAddImage(vp, L"exitbox.png", SKINFILEBPP, &vp->gui.image[IMGC_EXITBOX]);
	TFRAME *eb = imageCacheGetImage(vp, vp->gui.image[IMGC_EXITBOX]);
	
	const int w = eb->width;
	const int h = eb->height;
	exit->pos.x = (frame->width - w)/2;
	exit->pos.y = (frame->height - h)/2;
	const int x = exit->pos.x + w/2;
	const int y = exit->pos.y + h/2;

	TBUTTON *button = buttonGet(vp, PAGE_EXIT, EXITBUTTON_YES);
	button->enabled = 1;
	button->id = EXITBUTTON_YES;
	button->canAnimate = 0;
	button->callback = buttonExit;
	buttonSetImages(vp, button, L"yes.png", NULL);
	button->pos.x = (x - button->image->width/2) - button->image->width;
	button->pos.y = (y - button->image->height/2) + button->image->height/2;

	button = buttonGet(vp, PAGE_EXIT, EXITBUTTON_NO);
	button->enabled = 1;
	button->id = EXITBUTTON_NO;
	button->canAnimate = 0;
	button->callback = buttonExit;
	buttonSetImages(vp, button, L"no.png", NULL);
	button->pos.x = (x - button->image->width/2) + button->image->width;
	button->pos.y = (y - button->image->height/2) + button->image->height/2;
	
	return 1;
}

int drawExit (TVLCPLAYER *vp, TFRAME *frame, TEXIT *exit)
{
	TBUTTONS *buttons = buttonsGetContainer(vp, PAGE_EXIT);
	TFRAME *eb = imageCacheGetImage(vp, vp->gui.image[IMGC_EXITBOX]);
	TFRAME *et = imageCacheGetImage(vp, vp->gui.image[IMGC_EXIT]);
	const int x = (frame->width - et->width)/2;
	const int y = frame->height/2 - et->height;
	exit->pos.x = (frame->width - eb->width)/2;
	exit->pos.y = (frame->height - eb->height)/2;
	
	drawImage(eb, frame, exit->pos.x, exit->pos.y, eb->width-1, eb->height-1);
	drawImage(et, frame, x, y, et->width-1, et->height-1);
	buttonsRender(buttons->list, buttons->total, frame);

#if DRAWTOUCHRECTS
	TBUTTON *button = buttons->list;
	int i;
	for (i = 0; i < buttons->total; button++, i++){
		if (button->enabled)
			lDrawRectangle(frame, button->pos.x, button->pos.y, button->pos.x+button->image->width-1, button->pos.y+button->image->height-1, 0xFF0000FF);
	}
#endif
	return 1;
}

int touchExit (TTOUCHCOORD *pos, int flags, TVLCPLAYER *vp)
{
	TBUTTONS *buttons = buttonsGetContainer(vp, PAGE_EXIT);
	TBUTTON *button = buttons->list;
	TTOUCHCOORD bpos;
	static unsigned int lastId = 0;
	
	// we don't want drag reports
	if (pos->dt < 100 || flags != 0) return 0;

	if (!flags){		// pen down
		if (lastId >= pos->id)
			return 0;
		lastId = pos->id;
	}else if (lastId != pos->id){
		return 0;	
	}	
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
						return 0;
				}
			}
		}
	}

	return 0;
}

