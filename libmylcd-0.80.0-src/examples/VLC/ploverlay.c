// playlist media control overlay
// (next trk, play, art size, etc..)
//
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

extern int RED;
extern int GREEN;
extern int BLUE;
extern int BLACK;
extern int WHITE;

int buttonPlOvr (const TTOUCHCOORD *pos, TBUTTON *button, const int id, const int message, void *ptr)
{
	TVLCPLAYER *vp = (TVLCPLAYER*)ptr;
	TPLAYLIST2 *pl = getPagePtr(vp, PAGE_PLAYLIST2);
	PLAYLISTCACHE *plcD = getDisplayPlaylist(vp);
	
	switch (id){
	  case PLOBUTTON_ARTSIZE:
	  	cycleArtSize(pl);
	  	break;
		
	  case PLOBUTTON_NAVUP:
		pl->voffset -= 5 ;
		plcD->pr->dragLocation.d1.y = -10;
		pl->decayTop = 5.0;
	  break;
	  
	  case PLOBUTTON_NAVDN:
		pl->voffset += 5;
		plcD->pr->dragLocation.d1.y = 10;
		pl->decayTop = 5.0;
	  break;
	  
	  case PLOBUTTON_PLAY:
	  	if (!getPlayState(vp)){

	  		vp->queuedPlaylist = vp->displayPlaylist;
	  		PLAYLISTCACHE *plcQ = getQueuedPlaylist(vp);

	  		int trk = 0;
	  		if (plcQ->pr->selectedItem >= 0)
	  			trk = plcQ->pr->selectedItem;
	  		else
	  			trk = plcQ->pr->playingItem;

			if (trk < 0){
				trk = 0;
				plcQ->pr->playingItem = trk;
			}

	  		playlist2SetStartTrack(pl, plcQ, trk);
	  		startPlaylistTrack(vp, plcQ, trk);
	  		
	  	}else if (getPlayState(vp) != 1){
			//player_play(vp, vp->vlc);
			trackPlay(vp);
	  	}
		break;

	  case PLOBUTTON_PAUSE:
	  	trackPlayPause(vp);
	  	
	  	/*if (getPlayState(vp) == 1){
	  		vp->vlc->playState = 2;
			player_pause(vp, vp->vlc);
			
		}else if (getPlayState(vp) == 2){
			player_play(vp, vp->vlc);
		}*/
		break;
		
	  case PLOBUTTON_META:{
	  	if (plcD->pr->selectedItem == -1)
	  		plcD->pr->selectedItem = plcD->pr->playingItem;
	  	
		TMETA *meta = getPagePtr(vp, PAGE_META);
		if (plcD->pr->selectedItem >= 0)
			meta->trackPosition = plcD->pr->selectedItem;
		else
			meta->trackPosition = 0;

		metaOpenPageAndReturn(vp);
		break;
	  }
	  case PLOBUTTON_CLOSE:
		//setPage(vp, PAGE_NONE);
		setPageSec(vp, -1);
		break;
	 }
	return 0;
}	  

int drawPlOvr (TVLCPLAYER *vp, TFRAME *frame, TPLOVR *plo)
{
	const int x1 = plo->x1;
	const int y1 = plo->y1;
	const int x2 = plo->x2;
	const int y2 = plo->y2;
	
	lDrawRectangle(frame, x1+2, y1+3, x2-2, y2-3, 220<<24 | 0xFFFFFF);
	lDrawRectangle(frame, x1, y1+1, x2, y2-1, 180<<24 | 0xFFFFFF);
	blurImageAreaWithFade(frame, x1, y1, x2, y2, 3);

	TBUTTONS *buttons = buttonsGetContainer(vp, PAGE_PLOVERLAY);
	buttonsRender(buttons->list, buttons->total, frame);
	
#if DRAWTOUCHRECTS
	TBUTTON *button = buttons->list;
	for (int i = 0; i < buttons->total; button++, i++){
		if (button->enabled)
			lDrawRectangle(frame, button->pos.x, button->pos.y, button->pos.x+button->image->width-1, button->pos.y+button->image->height-1, BLUE);
	}
#endif
	return 1;
}

int closePlOvr  (TVLCPLAYER *vp, TFRAME *frame, TPLOVR *ptr)
{
	buttonsDeleteContainer(vp, PAGE_PLOVERLAY);
	return 1;
}

int openPlOvr (TVLCPLAYER *vp, TFRAME *frame, TPLOVR *plo)
{

#if !G19DISPLAY
	const int dx = 66;
	const int dy = 66;
	plo->x1 = 120;
	plo->y1 = 50;
#else
	const int dx = 62;
	const int dy = 66;
	plo->x1 = 4;
	plo->y1 = 66;
#endif

	plo->x2 = plo->x1 + (dx*5);
	plo->y2 = plo->y1 + (dy*2);
	int x = plo->x1 + (dx/2) + 4;
	int y = plo->y1 + 4;

	buttonsCreateContainer(vp, PAGE_PLOVERLAY, PLOBUTTON_TOTAL);

	TBUTTON *button = buttonGet(vp, PAGE_PLOVERLAY, PLOBUTTON_NAVUP);
	button->enabled = 1;
	button->id = PLOBUTTON_NAVUP;
	button->pos.x = x;
	button->pos.y = y;
	button->canAnimate = 1;
	button->callback = buttonPlOvr;
	buttonSetImages(vp, button, L"media/plnavup.png", NULL);
	int navuph = button->image->height;

	button = buttonGet(vp, PAGE_PLOVERLAY, PLOBUTTON_NAVDN);
	button->enabled = 1;
	button->id = PLOBUTTON_NAVDN;
	button->pos.x = x;
	button->pos.y = y + navuph;
	button->canAnimate = 1;
	button->callback = buttonPlOvr;
	buttonSetImages(vp, button, L"media/plnavdn.png", NULL);
	
	button = buttonGet(vp, PAGE_PLOVERLAY, PLOBUTTON_ARTSIZE);
	button->enabled = 1;
	button->id = PLOBUTTON_ARTSIZE;
	button->pos.x = (x += dx);
	button->pos.y = y;
	button->canAnimate = 1;
	button->callback = buttonPlOvr;
	buttonSetImages(vp, button, L"media/art.png", NULL);

	button = buttonGet(vp, PAGE_PLOVERLAY, PLOBUTTON_META);
	button->enabled = 1;
	button->id = PLOBUTTON_META;
	button->pos.x = (x += dx);
	button->pos.y = y;
	button->canAnimate = 1;
	button->callback = buttonPlOvr;
	buttonSetImages(vp, button, L"media/meta.png", NULL);
		
	button = buttonGet(vp, PAGE_PLOVERLAY, PLOBUTTON_CLOSE);
	button->enabled = 1;
	button->id = PLOBUTTON_CLOSE;
	button->pos.x = (x += dx);
	button->pos.y = y;
	button->canAnimate = 0;
	button->callback = buttonPlOvr;
	buttonSetImages(vp, button, L"media/plctrloff.png", NULL);

	x = plo->x1;
	y += dy - 1;
		
	button = buttonGet(vp, PAGE_PLOVERLAY, PLOBUTTON_PREVTRACK);
	button->enabled = 1;
	button->id = VBUTTON_PRETRACK;
	button->pos.x = x;
	button->pos.y = y;
	button->canAnimate = 1;
	button->callback = obuttonCB;
	buttonSetImages(vp, button, L"media/skip_backward.png", NULL);
			
	button = buttonGet(vp, PAGE_PLOVERLAY, PLOBUTTON_PLAY);
	button->enabled = 1;
	button->id = PLOBUTTON_PLAY;
	button->pos.x = (x += dx);
	button->pos.y = y;
	button->canAnimate = 1;
	button->callback = buttonPlOvr;
	buttonSetImages(vp, button, L"media/play.png", NULL);

	button = buttonGet(vp, PAGE_PLOVERLAY, PLOBUTTON_PAUSE);
	button->enabled = 0;
	button->id = PLOBUTTON_PAUSE;
	button->pos.x = x;
	button->pos.y = y;
	button->canAnimate = 1;
	button->callback = buttonPlOvr;
	buttonSetImages(vp, button, L"media/pause.png", NULL);

	button = buttonGet(vp, PAGE_PLOVERLAY, PLOBUTTON_STOP);
	button->enabled = 1;
	button->id = VBUTTON_STOP;
	button->pos.x = (x += dx);
	button->pos.y = y;
	button->canAnimate = 1;
	button->callback = obuttonCB;
	buttonSetImages(vp, button, L"media/stop.png", NULL);

	button = buttonGet(vp, PAGE_PLOVERLAY, PLOBUTTON_OPEN);
	button->enabled = 1;
	button->id = VBUTTON_OPEN;
	button->pos.x = (x += dx);
	button->pos.y = y;
	button->canAnimate = 0;
	button->callback = obuttonCB;
	buttonSetImages(vp, button, L"media/open.png", NULL);
			
	button = buttonGet(vp, PAGE_PLOVERLAY, PLOBUTTON_NEXTTRACK);
	button->enabled = 1;
	button->id = VBUTTON_NEXTTRACK;
	button->pos.x = (x += dx);
	button->pos.y = y;
	button->canAnimate = 1;
	button->callback = obuttonCB;
	buttonSetImages(vp, button, L"media/skip_forward.png", NULL);
	return 1;
}

int touchPlOvr (TTOUCHCOORD *pos, int flags, TVLCPLAYER *vp)
{
	TBUTTONS *buttons = buttonsGetContainer(vp, PAGE_PLOVERLAY);
	TBUTTON *button = buttons->list;
	TTOUCHCOORD bpos;
	static unsigned int lastId = 0;

	//if (pos->dt < 80 || flags) return -2;

	// dump input if Id doesn't match that of the current stream
	if (!flags){		// pen down
		if (lastId >= pos->id)
			return 0;
		lastId = pos->id;
	}else if (lastId != pos->id){
		return 0;	
	}
	
	if (pos->dt > 80 && !flags){
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
						return button->callback(&bpos, button, button->id, flags, vp);
					}
				}
			}
		}
	}
	return 1;
}

