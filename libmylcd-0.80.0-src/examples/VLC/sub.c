
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

int subbutton (const TTOUCHCOORD *pos, TBUTTON *button, const int id, const int message, void *ptr)
{
	TVLCPLAYER *vp = (TVLCPLAYER*)ptr;
	//TSUB *sub = &vp->gui.sub;
	
	switch (id){
	  case SUBBUTTON_SUB1:
	  case SUBBUTTON_SUB2:
	  case SUBBUTTON_SUB3:
	  case SUBBUTTON_SUB4:
	  case SUBBUTTON_SUB5:
	  case SUBBUTTON_SUB6:
	  case SUBBUTTON_SUB7:
	  case SUBBUTTON_SUB8:
	  case SUBBUTTON_SUB9:
	  case SUBBUTTON_SUB10:
	  case SUBBUTTON_SUB11:
	  case SUBBUTTON_SUB12:
	  case SUBBUTTON_SUB13:
	  case SUBBUTTON_SUB14:
	  case SUBBUTTON_SUB15:
	  case SUBBUTTON_SUB16:
	  case SUBBUTTON_SUB17:
	  case SUBBUTTON_SUB18:
	  case SUBBUTTON_SUB19:
	  case SUBBUTTON_SUB20:
	  	vlc_setSubtitle(vp->vlc, id - SUBBUTTON_SUB1);
	  	vp->vlc->spu.selected = vlc_getSubtitle(vp->vlc);
		break;
		
	  case SUBBUTTON_CLOSE:
	  	setPage(vp, PAGE_CFG);
	  	break;
	}
	return 0;
}

int touchSub (TTOUCHCOORD *pos, int flags, TVLCPLAYER *vp)
{
	TBUTTONS *buttons = buttonsGetContainer(vp, PAGE_SUB);
	TBUTTON *button = buttons->list;
	TTOUCHCOORD bpos;
	static unsigned int lastId = 0;

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
						break;
				}
			}
		}
	}

	return 1;
}


int closeSub (TVLCPLAYER *vp, TFRAME *frame, TSUB *sub)
{
	buttonsDeleteContainer(vp, PAGE_SUB);
	return 1;
}

static int addSubImage (TVLCPLAYER *vp, int id, int x, int y, int icId)
{
	TBUTTON *button = buttonGet(vp, PAGE_SUB, id);
	button->enabled = 1;
	button->id = id;
	button->pos.x = x;
	button->pos.y = y;
	button->canAnimate = 0;
	button->callback = subbutton;
	button->highlight = NULL;
	button->image = imageCacheGetImage(vp, icId);
	button->pos.x = (button->pos.x - button->image->width)/2;
	return id;
}

int openSub (TVLCPLAYER *vp, TFRAME *frame, TSUB *sub)
{
	buttonsCreateContainer(vp, PAGE_SUB, SUBBUTTON_TOTAL);

	TBUTTON *button = buttonGet(vp, PAGE_SUB, SUBBUTTON_CLOSE);
	button->enabled = 1;
	button->id = SUBBUTTON_CLOSE;
	button->pos.x = frame->width-(64+6);
	button->pos.y = frame->height*0.62;
	button->canAnimate = 0;
	button->callback = subbutton;
	buttonSetImages(vp, button, L"cfgclose.png", NULL);

	int x = frame->width;
	int y = 0;
	int id;
	imageCacheAddImage(vp, L"linebase3.png", SKINFILEBPP, &id);	
	const int h = imageCacheGetImage(vp, id)->height;

	for (int i = SUBBUTTON_SUB1; i <= SUBBUTTON_SUB20; i++)
		addSubImage(vp, i, x, y+=h, id);

	return 1;
}

int drawSub (TVLCPLAYER *vp, TFRAME *frame, TSUB *sub)
{
	TBUTTONS *buttons = buttonsGetContainer(vp, PAGE_SUB);

	// don't draw the subtitle underlays
	for (int i = SUBBUTTON_SUB1; i <= SUBBUTTON_SUB20; i++)
		buttonDisable(vp, PAGE_SUB, i);

	buttonsRender(buttons->list, buttons->total, frame);	

	// enable an underlay per subtitle
	if (vp->vlc->spu.total > 0){
		for (int i = SUBBUTTON_SUB1; i <= SUBBUTTON_SUB20; i++){
			if (i < vp->vlc->spu.total + SUBBUTTON_SUB1){
				buttonEnable(vp, PAGE_SUB, i);
#if DRAWTOUCHRECTS
				int x, y, w, h;
				TBUTTON *button = buttonGet(vp, PAGE_SUB, i);
				x = button->pos.x;
				y = button->pos.y;
				w = button->image->width;
				h = button->image->height;
				lDrawRectangle(frame, x, y, x+w, y+h, 0xFF0000FF);
#endif
			}
		}
	}

	// draw the subtitle text
	if (vp->vlc->spu.total){
		#define SUBBUFFERLEN 1023
		wchar_t *buffer = my_calloc(sizeof(wchar_t), SUBBUFFERLEN+1);
		wchar_t *subtext = my_calloc(sizeof(wchar_t), SUBBUFFERLEN+1);
		libvlc_track_description_t *st = vp->vlc->spu.desc;
		TBUTTON *button = buttonGet(vp, PAGE_SUB, SUBBUTTON_SUB1);
		lSetCharacterEncoding(vp->hw, CMT_UTF16);
		lSetForegroundColour(vp->hw, 0xFFFFFFFF);
		
		int x, y, w, h;
		int i = 0;

		do{
			if (i == vp->vlc->spu.selected){
				*buffer = 0;
				UTF8ToUTF16(st->psz_name, strlen(st->psz_name), buffer, -1);
				snwprintf(subtext, SUBBUFFERLEN, L"\u2022 %s \u2022", buffer);
			}else{
				UTF8ToUTF16(st->psz_name, strlen(st->psz_name), subtext, -1);
			}

			lGetTextMetrics(vp->hw, (char*)subtext, 0, BFONT, &w, &h);
			x = button->pos.x + (button->image->width - w)/2;
			y = button->pos.y + (button->image->height - h)/2;
			fadeArea(frame, x-3, y-1, x+w, y+h);
			lPrintf(frame, x, y, BFONT, LPRT_CPY, (char*)subtext);

			button++; i++;
		}while((st=st->p_next) && i < 20);
		
		my_free(subtext);
		my_free(buffer);
	}
	
	return 1;
}

