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


static void buttonRender (TBUTTON *button, TFRAME *frame)
{
	if (!button->canAnimate || !button->ani.state){
		drawImage(button->activeBtn, frame, button->pos.x, button->pos.y, button->activeBtn->width-1, button->activeBtn->height-1);
				
	}else if (button->canAnimate == 1){		// zoom animation
		if (button->ani.state == 1){
			if (button->ani.zoom)
				zoomDelete(button->ani.zoom);
			button->ani.zoom = zoomCreate(button->activeBtn, 1.0, 0.08, 1);
			button->ani.state = 2;
		}
		if (button->ani.state == 2){
			if (!zoomRender(button->ani.zoom, frame, button->pos.x, button->pos.y)){
				zoomDelete(button->ani.zoom);
				button->ani.zoom = NULL;
				button->ani.state = 0;
			}
		}
	}else if (button->canAnimate == 2){		// image dissolve animation
		if (button->ani.state == 1){
			if (!button->ani.src){
				button->ani.src = morphCreate(button->activeBtn, 48, 128);
				if (button->ani.src)
					button->ani.morph = morphClone(button->ani.src);
			}else{
				morphReset(button->ani.morph, button->ani.src);
			}
			if (button->ani.src)
				button->ani.state = 2;
			else
				button->ani.state = 0;
		}
		if (button->ani.state == 2){
			if (!morphRender(button->ani.morph, frame, button->pos.x, button->pos.y))
				button->ani.state = 0;
		}
	}
}

void buttonsRender (TBUTTON *button, int btotal, TFRAME *frame)
{
	while(btotal--){
		if (button->enabled)
			buttonRender(button, frame);
		button++;
	}
}

static TBUTTONS *buttonsNew (const int total)
{
	TBUTTONS *buttons = my_malloc(sizeof(TBUTTONS));
	buttons->list = my_calloc(total, sizeof(TBUTTON));
	buttons->total = total;
	return buttons;
}

static void buttonsDelete (TBUTTONS *buttons)
{
	my_free(buttons->list);
	my_free(buttons);
}

static TBUTTON *buttonsGetList (TVLCPLAYER *vp, const int page_id)
{
	return vp->gui.buttons[page_id]->list;
}

TBUTTONS *buttonsGetContainer (TVLCPLAYER *vp, const int page_id)
{
	return vp->gui.buttons[page_id];
}

TBUTTON *buttonsCreateContainer (TVLCPLAYER *vp, const int page_id, const int total)
{
	vp->gui.buttons[page_id] = buttonsNew(total);
	return buttonsGetList(vp, page_id);
}

int buttonsGetTotal (TVLCPLAYER *vp, const int page_id)
{
	TBUTTONS *buttons = buttonsGetContainer(vp, page_id);
	return buttons->total;
}

void buttonEnable (TVLCPLAYER *vp, const int page_id, const int btn_id)
{
	buttonGet(vp, page_id, btn_id)->enabled = 1;
}

void buttonDisable (TVLCPLAYER *vp, const int page_id, const int btn_id)
{
	buttonGet(vp, page_id, btn_id)->enabled = 0;
}

TBUTTON *buttonGet (TVLCPLAYER *vp, const int page_id, const int btn_id)
{
	return (TBUTTON*)&(buttonsGetList(vp, page_id)[btn_id]);
}

int buttonSetImages (TVLCPLAYER *vp, TBUTTON *button, wchar_t *image, wchar_t *highlight)
{
	int ret = 0;
	
	if (image){
		button->image = imageCacheAddImage(vp, image, SKINFILEBPP, &button->icIdi);
		button->activeBtn = button->image;
		ret = (button->image != NULL);
		if (!button->image){
			wprintf(L"unable to load image '%s'", image);
			printf("\n");
		}
	}
	if (highlight){
		button->highlight = imageCacheAddImage(vp, highlight, SKINFILEBPP, &button->icIdh);
		ret |= ((button->highlight != NULL) << 1);
		if (!button->highlight){
			wprintf(L"unable to load image '%s'", highlight);
			printf("\n");
		}
	}
	return ret;
}

void buttonsFreeStorage (TBUTTON *button, const int total)
{
	for (int i = 0; i < total; button++, i++){
		if (button->canAnimate == 1){
			if (button->ani.zoom){
				zoomDelete(button->ani.zoom);
				button->ani.zoom = NULL;
			}
		}else if (button->canAnimate == 2){
			if (button->ani.src){
				morphDelete(button->ani.src);
				button->ani.src = NULL;
			}
			if (button->ani.morph){
				morphDelete(button->ani.morph);
				button->ani.morph = NULL;
			}
		}
		button->ani.state = 0;
	}
}

void buttonsDeleteContainer (TVLCPLAYER *vp, const int page_id)
{
	TBUTTONS *buttons = buttonsGetContainer(vp, page_id);
	if (buttons){
		buttonsFreeStorage(buttons->list, buttons->total);
		buttonsDelete(buttons);
	}
}
