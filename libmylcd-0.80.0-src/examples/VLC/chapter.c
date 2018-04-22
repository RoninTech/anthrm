
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


int buttonChapt (const TTOUCHCOORD *pos, TBUTTON *button, const int id, const int message, void *ptr)
{
	TVLCPLAYER *vp = (TVLCPLAYER*)ptr;
	TCHAPTER *chapt = getPagePtr(vp, PAGE_CHAPTERS);
	
	switch (id){
	  case CHPBUTTON_LEFT:
	  	if (!chapt->ttitles) break;

		if (--chapt->ctitle < 1) chapt->ctitle = 1;
		chapt->schapter = -1;
		
		chapt->tchapters = vlc_getTitleChapterCount(vp->vlc, chapt->ctitle-1);
		if (chapt->tchapters > 32) chapt->tchapters = 32;
		
		// if we've no chapters in this title then attempt to play this title
		if (!chapt->tchapters){
			vlc_setTitle(vp->vlc, chapt->ctitle-1);
		}
		
	  	break;
	  	
	  case CHPBUTTON_RIGHT:
	  	if (!chapt->ttitles) break;
	  	
	  	if (++chapt->ctitle > chapt->ttitles)
	  		chapt->ctitle = chapt->ttitles;
		chapt->schapter = -1;
		chapt->tchapters = vlc_getTitleChapterCount(vp->vlc, chapt->ctitle-1);
		if (chapt->tchapters > 32) chapt->tchapters = 32;

		// if we've no chapters in this title then attempt to play this title
		if (!chapt->tchapters){
			vlc_setTitle(vp->vlc, chapt->ctitle-1);
		}
	  	break;
	}
	return 0;
}


int closeChapt  (TVLCPLAYER *vp, TFRAME *frame, TCHAPTER *chapt)
{
	if (chapt->loc) my_free(chapt->loc);
	chapt->loc = NULL;
	
	buttonsDeleteContainer(vp, PAGE_CHAPTERS);
	return 1;
}

int openChapt (TVLCPLAYER *vp, TFRAME *frame, TCHAPTER *chapt)
{
	
	chapt->loc = (TLPOINTEX*)my_calloc(32, sizeof(TLPOINTEX));
	if (!chapt->loc) return 0;
	
	chapt->rect.x1 = 80;	
	chapt->rect.y1 = 64;
	chapt->rect.x2 = chapt->rect.x1 + 150;
	chapt->rect.y2 = chapt->rect.y1 + 16;
	chapt->ttitles = 0;
	chapt->tchapters = 0;
	chapt->ctitle = 0;
	chapt->schapter = -1;
	chapt->cchapter = -1;
		
	int x = chapt->rect.x1+3;
	int y = chapt->rect.y1+3;

	buttonsCreateContainer(vp, PAGE_CHAPTERS, CHPBUTTON_TOTAL);
	
	TBUTTON *button = buttonGet(vp, PAGE_CHAPTERS, CHPBUTTON_LEFT);
	button->enabled = 1;
	button->id = CHPBUTTON_LEFT;
	button->pos.x = x;
	button->pos.y = y;
	button->canAnimate = 0;
	button->callback = buttonChapt;
	buttonSetImages(vp, button, L"left.png", L"lefthl.png");
	int w = button->image->width;

	button = buttonGet(vp, PAGE_CHAPTERS, CHPBUTTON_RIGHT);
	button->enabled = 1;
	button->id = CHPBUTTON_RIGHT;
	button->pos.x = x+w+16;
	button->pos.y = y;
	button->canAnimate = 0;
	button->callback = buttonChapt;
	buttonSetImages(vp, button, L"right.png", L"righthl.png");
	return 1;
}

static void printCLine (TFRAME *frame, const char *text, int c, int font, int x, int y, int foreColour, TLPOINTEX *pos)
{
	TLPRINTR rt;
	rt.bx1 = x; rt.by1 = y;
	rt.bx2 = frame->width-1;
	rt.by2 = frame->height-1;
	rt.sx = x; rt.sy = y;
	
	lSetForegroundColour(frame->hw, foreColour);
	if (c >= 0)
		lPrintEx(frame, &rt, BFONT, 0, LPRT_CPY, "%s %i", text, c);
	else
		lPrintEx(frame, &rt, BFONT, 0, LPRT_CPY, text);
	
	pos->x1 = rt.sx-3;
	pos->y1 = rt.sy;
	pos->x2 = rt.ex+3;
	pos->y2 = rt.ey;
}

int drawChapt (TVLCPLAYER *vp, TFRAME *frame, TCHAPTER *chapt)
{
	TBUTTONS *buttons = buttonsGetContainer(vp, PAGE_CHAPTERS);
	const int fonth = 17;
	const int cwidth = 90;

	if (chapt->tchapters > 8){
		chapt->rect.x2 = chapt->rect.x1 + ((((chapt->tchapters%8)>0)+(chapt->tchapters/8)) * cwidth);
		chapt->rect.y2 = chapt->rect.y1 + (9*fonth) + 6;
	}else{
		chapt->rect.x2 = chapt->rect.x1 + 150;
		chapt->rect.y2 = chapt->rect.y1 + ((1+chapt->tchapters)*fonth) + 6;
		if (chapt->rect.y2 < chapt->rect.y1+fonth)
			chapt->rect.y2 = chapt->rect.y1 + fonth*2 + 6;
	}

	fadeArea(frame, chapt->rect.x1, chapt->rect.y1, chapt->rect.x2, chapt->rect.y2);
	if (chapt->tchapters > 0)
		lDrawRectangle(frame, chapt->rect.x1-1, chapt->rect.y1-1, chapt->rect.x2+1, chapt->rect.y2+1, 0xC8FFFFFF);
	buttonsRender(buttons->list, buttons->total, frame);
	
	// print title
	int x = chapt->rect.x1 + 80;
	int y = chapt->rect.y1 + 3;
	lSetCharacterEncoding(vp->hw, ASCII_PAGE);
	lSetForegroundColour(vp->hw, 0xFFFFFFFF);
	
	char *title = vlc_getTitleDescription(vp->vlc, chapt->ctitle-1);
	if (title){
		lPrintf(frame, x-3, y, BFONT, LPRT_CPY, "%i: %s",chapt->ctitle, title);
		my_free(title);
	}else{
		lPrintf(frame, x, y, BFONT, LPRT_CPY, "Title %i/%i", chapt->ctitle, chapt->ttitles);
	}
	if (chapt->ttitles < 1){
		lPrintf(frame, chapt->rect.x1+10, chapt->rect.y1+fonth+4, BFONT, LPRT_CPY, "<no media>");
		chapt->schapter = -1;
		return 1;
	}

	// print chapters
	y = chapt->rect.y1+4;
	int colour;
	for (int c = 0; c < chapt->tchapters; c++){
		x = 10 + chapt->rect.x1 + ((c/8) * cwidth);
		y += fonth;
		
		if (c == chapt->cchapter && chapt->title+1 == chapt->ctitle)
			colour = 0xFFFF00FF;
		else if (c == chapt->schapter)
			colour = 0xFFFF0000;
		else
			colour = 0xFFFFFFFF;
		
		char *chapter = vlc_getChapterDescription(vp->vlc, c, chapt->ctitle-1);
		if (chapter){
			printCLine(frame, chapter, -1, BFONT, x, y, colour, &chapt->loc[c]);
			my_free(chapter);
		}else{
			printCLine(frame, "Chapter", c+1, BFONT, x, y, colour, &chapt->loc[c]);
		}
		if (y >= chapt->rect.y1 + fonth*8)
			y = chapt->rect.y1+2;


#if DRAWTOUCHRECTS
		lDrawRectangle(frame, chapt->loc[c].x1, chapt->loc[c].y1, chapt->loc[c].x2, chapt->loc[c].y2, 0xFF0000FF);
		
		/*TBUTTON *button = buttons->list;
		for (int i = 0; i < buttons->total; button++, i++){
			if (button->enabled)
				lDrawRectangle(frame, button->pos.x, button->pos.y, button->pos.x+button->image->width-1, button->pos.y+button->image->height-1, 0xFF0000FF);
		}*/
#endif
	}


#if DRAWTOUCHRECTS
	TBUTTON *button = buttons->list;
	for (int i = 0; i < buttons->total; button++, i++){
		if (button->enabled)
			lDrawRectangle(frame, button->pos.x, button->pos.y, button->pos.x+button->image->width-1, button->pos.y+button->image->height-1, 0xFF0000FF);
	}
#endif
	
	return 1;
}

int touchChapt (TTOUCHCOORD *pos, int flags, TVLCPLAYER *vp)
{

	TCHAPTER *chapt = getPagePtr(vp, PAGE_CHAPTERS);
	TBUTTONS *buttons = buttonsGetContainer(vp, PAGE_CHAPTERS);
	TBUTTON *button = buttons->list;
	TLPOINTEX *loc = chapt->loc;
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
		
	// check chapter text
	for (int c = 0; c < chapt->tchapters; c++){
		if (pos->x >= loc->x1 && pos->x <= loc->x2){
			if (pos->y >= loc->y1 && pos->y <= loc->y2){
				chapt->schapter = c;
				vlc_setTitle(vp->vlc, chapt->ctitle-1);
				vlc_setChapter(vp->vlc, c);
				return 0;
			}
		}
		loc++;
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

		
	// check if touch is within the faded area
	if (pos->x >= chapt->rect.x1 && pos->x <= chapt->rect.x2){
		if (pos->y >= chapt->rect.y1 && pos->y <= chapt->rect.y2){
			return 0;
		}
	}
	return 1;
}

