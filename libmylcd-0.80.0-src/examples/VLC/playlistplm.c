
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


void playlistPlmSetRenderStart (TVLCPLAYER *vp, TPLAYLISTMANAGER *plm, int start)
{
	TPLAYLISTPLM *pl = getPagePtr(vp, PAGE_PLAYLISTPLM);
	if (start < 0) start = 0;
	pl->startPosition = start;
	pl->voffset = 0;
	pl->decayTop = 0;

	plm->pr->dragLocation.d1.y = 0;
}


int buttonPlaylistPlm (const TTOUCHCOORD *pos, TBUTTON *button, const int id, const int message, void *ptr)
{
	if (pos->dt < 100) return 0;
	
	TVLCPLAYER *vp = (TVLCPLAYER*)ptr;

	switch (id){
	  case PLMBUTTON_CLOSE:
		setPageSec(vp, -1);
		setPage(vp, PAGE_NONE);
		break;
	 }
	return 1;
}	  
	  
int closePlaylistPlm (TVLCPLAYER *vp, TFRAME *frame, TPLAYLISTPLM *pl)
{
	buttonsDeleteContainer(vp, PAGE_PLAYLISTPLM);
	lDeleteFrame(pl->lineRender);	
	for (int i = 0; i < PL2ARTCACHESIZE; i++){
		lDeleteFrame(pl->artcache[i]->frame);
		my_free(pl->artcache[i]);
	}
	
	return 1;
}

int openPlaylistPlm (TVLCPLAYER *vp, TFRAME *frame, TPLAYLISTPLM *pl)
{	
	for (int i = 0;  i < PL2ARTCACHESIZE; i++){
		pl->artcache[i] = my_calloc(1, sizeof(TPLAYLISTART));
		if (pl->artcache[i])
			pl->artcache[i]->frame = lNewFrame(vp->hw, 8, 8, LFRM_BPP_32);
	}

	pl->lineRender = lNewFrame(vp->hw, frame->width, 20, LFRM_BPP_32);	
	pl->x = 1;
	pl->y = 1;
	pl->voffset = 0;
	pl->startPosition = 0;
	pl->artScale = DEFAULTARTSCALE;
	
	const int x = 6;
	const int y = 6;
	buttonsCreateContainer(vp, PAGE_PLAYLISTPLM, PLMBUTTON_TOTAL);
	
	TBUTTON *button = buttonGet(vp, PAGE_PLAYLISTPLM, PLMBUTTON_CLOSE);
	button->enabled = 1;
	button->id = PLMBUTTON_CLOSE;
	button->pos.x = frame->width-(64+x);
	button->pos.y = y;
	button->canAnimate = 0;
	button->callback = buttonPlaylistPlm;
	buttonSetImages(vp, button, L"media/close.png", NULL);

#if !G19DISPLAY
	imageCacheAddImage(vp, L"noart100x100.png", LFRM_BPP_32A, &vp->gui.image[IMGC_NOART1]);
#else
	imageCacheAddImage(vp, L"noart66x66.png", LFRM_BPP_32A, &vp->gui.image[IMGC_NOART1]);
#endif
	return 1;
}

static int drawArtwork (TVLCPLAYER *vp, TPLAYLISTPLM *pl, TFRAME *frame, const unsigned int hash, const float scale, PLAYLISTRENDERITEM *item, const int x, const int y, const int bsize)
{
	int w = 0, h = 0;

	for (int i = 0; i < PL2ARTCACHESIZE; i++){
		if (pl->artcache[i]->hash == hash && pl->artcache[i]->scale == scale){
			w = pl->artcache[i]->frame->width;
			h = pl->artcache[i]->frame->height;
			item->art.x1 = x;
			item->art.y1 = y+abs(bsize - h)/2;
			item->art.x2 = item->art.x1+w;
			item->art.y2 = item->art.y1+h;
			if (item->art.y2 > frame->height-1)
				item->art.y2 = frame->height-1;
			fastFrameCopy(pl->artcache[i]->frame, frame, item->art.x1, item->art.y1);
			return (w<<16)|(h&0xFFFF);
		}
	}

	if (tagLock(vp->tagc)){
		TTAGIMG *art = g_tagArtworkGetByHash(vp->tagc, hash);
		if (art && art->img){
			w = (float)art->img->width*scale;
			h = (float)art->img->height*scale;
				
			item->art.x1 = x;
			item->art.y1 = y+abs(bsize - h)/2;	// center the artwork vertically
			item->art.x2 = item->art.x1+w;
			item->art.y2 = item->art.y1+h;
			if (item->art.y2 > frame->height-1)
				item->art.y2 = frame->height-1;

			TPLAYLISTART *artc = pl->artcache[pl->cacheIdx];
			lResizeFrame(artc->frame, w, h, artc->frame->bpp);
			artc->hash = hash;
			artc->scale = scale;
			drawImageScaled(art->img, artc->frame, 0, 0, scale);
			fastFrameCopy(artc->frame, frame, item->art.x1, item->art.y1);
			
			if (++pl->cacheIdx >= PL2ARTCACHESIZE) pl->cacheIdx = 0;
		}
		tagUnlock(vp->tagc);
	}
	return (w<<16)|(h&0xFFFF);
}

static inline void drawPlaceholder (TFRAME *frame, PLAYLISTRENDERITEM *item, const int x, const int y, const int bsize)
{
	item->art.x1 = x;
	item->art.y1 = y;
	item->art.x2 = item->art.x1+bsize-1;
	item->art.y2 = item->art.y1+bsize-1;
				
	lDrawLine(frame, x+1, y+1, item->art.x2-1, item->art.y2-1, 0xE0FFFFFF);
	lDrawRectangle(frame, x, y, item->art.x2, item->art.y2, 0xE0FFFFFF);
				
	// limit the touch response area to within the frame
	// don't do this before rect render to prevent screwing the aspect
	if (item->art.y2 > frame->height-1)
		item->art.y2 = frame->height-1;
}

int renderPlaylistPlm (TVLCPLAYER *vp, TPLAYLISTMANAGER *plm, TFRAME *frame, TPLAYLISTPLM *pl, int *posFrom, int *posTo)
{
	PLAYLISTRENDERITEM *item = plm->pr->item;
	
	const int x = pl->x;
	int pos = *posFrom;
	int ax, ay, w, h, y;
	char buffer[MAX_PATH_UTF8];
	TFBROWSER *fb = getPagePtr(vp, PAGE_BROWSER);
	int th = fb->print->textHeight;
	
	if (pl->decayTop > PLAYLIST_DECAYRATE){
		pl->decayTop -= PLAYLIST_DECAYRATE;
		if (plm->pr->dragLocation.d1.y > 0){
			plm->pr->dragLocation.d1.y += pl->decayTop*PLAYLIST_DECAYFACTOR;
			pl->voffset -= (pl->decayTop*PLAYLIST_DECAYFACTOR);
		}else{
			plm->pr->dragLocation.d1.y -= pl->decayTop*PLAYLIST_DECAYFACTOR;
			pl->voffset += (pl->decayTop*PLAYLIST_DECAYFACTOR);
		}
	}

	pl->aHScaled = ((float)MAXARTHEIGHT*pl->artScale);
	
	if (pl->voffset < -((int)pl->aHScaled)){
		pl->voffset = pl->voffset - -(pl->aHScaled);
		//pos = ++(*posFrom);
		
		*posFrom = playlistManagerGetPlaylistNext(plm, *posFrom);
		pos = *posFrom;
		
	}else if (pl->voffset > 1){
		pl->voffset = -(pl->aHScaled);
		//pos = --(*posFrom);

		*posFrom = playlistManagerGetPlaylistPrev(plm, *posFrom);
		pos = *posFrom;
	}

	const int playlistsTotal = plm->total; //playlistManagerGetTotal(plm);
	if (pos >= playlistsTotal-1){
		pos = *posFrom = playlistsTotal-1;
		pl->voffset = 0;
	}else if (pos < 0){
		pos = *posFrom = 0;
		pl->voffset = 0;
	}
	
	y = pl->y + pl->voffset;
	float bsize = pl->aHScaled;
	while(y < -bsize) y += bsize + 3;

	do{
		pos = playlistManagerGetPlaylistNext(plm, pos-1);
		if (pos < 0) break;
		
		*posTo = pos;
		PLAYLISTCACHE *plc = playlistManagerGetPlaylist(plm, pos);

		if (plc){
			const int playlistTotal = playlistGetTotal(plc);
			w = 0; h = 0;

			// find and draw the first available artwork from playlist
			if (plc->artHash){
				w = drawArtwork(vp, pl, frame, plc->artHash, pl->artScale, item, x, y, bsize);
				h = w&0xFFFF;
				w >>= 16;
			}
						
			if (!w){
				plc->artHash = 0;
				unsigned int hash;
				
				for (int i = 0; i < playlistTotal && i < 10 && !w; i++){
					hash = playlistGetHash(plc, i);
					if (hash){
						w = drawArtwork(vp, pl, frame, hash, pl->artScale, item, x, y, bsize);
						h = w&0xFFFF;
						w >>= 16;
					}
				}
				if (w) plc->artHash = hash;
			}

			// nothing rendered above so insert a placeholder
			if (!w){
				h = w = bsize;
				
				TFRAME *img = imageCacheGetImage(vp, vp->gui.image[IMGC_NOART1]);
				if (img){
					lDrawImage(img, frame, x, y);

					item->art.x1 = x;
					item->art.y1 = y;
					item->art.x2 = item->art.x1 + img->width-1;
					item->art.y2 = item->art.y1 + img->height-1;
					if (item->art.y2 > frame->height-1)
						item->art.y2 = frame->height-1;
				}else{
					drawPlaceholder(frame, item, x, y, bsize);
				}
			}		
	
			if (pos == vp->queuedPlaylist){
				lDrawRectangle(frame, item->art.x1-1, item->art.y1-1, item->art.x2+1, item->art.y2+1, 0xFFFF00FF);
				lDrawRectangle(frame, item->art.x1, item->art.y1, item->art.x2, item->art.y2, 0xFFFF00FF);
			}else if (pos == vp->displayPlaylist){
				lDrawRectangle(frame, item->art.x1-1, item->art.y1-1, item->art.x2+1, item->art.y2+1, 0xFFFF0000);
				lDrawRectangle(frame, item->art.x1, item->art.y1, item->art.x2, item->art.y2, 0xFFFF0000);
			}
		
			item->playlistPosition = pos;
			item++;
			ax = x + w + 7;
			ay = y + (abs(bsize - h)>>1);

			snprintf(buffer, sizeof(buffer), "%i:", pos+1);
			printLine(frame, pl->lineRender, buffer, ax, ay);
			ay += th;
			
			playlistGetName(plc, buffer, sizeof(buffer));
			printLine(frame, pl->lineRender, buffer, ax, ay);
			ay += th;

			snprintf(buffer, sizeof(buffer), "(%i)", playlistTotal);
			printLine(frame, pl->lineRender, buffer, ax, ay);

			y += bsize + 3;
		}
	}while(y < frame->height-2 && ++pos < playlistsTotal);

	//*posTo = pos;
	item->playlistPosition = -1;
	return 1;
}

int drawPlaylistPlm (TVLCPLAYER *vp, TFRAME *frame, TPLAYLISTPLM *pl)
{
	lSetCharacterEncoding(vp->hw, CMT_UTF8);
	lSetBackgroundColour(vp->hw, BLACK);
	lSetForegroundColour(vp->hw, 0xFFF0F0F0);

	// only render if there is a playlist which actually contains something
	int doRender = playlistManagerGetTotal(vp->plm);
	if (doRender > 0){
		doRender = 0;

		const int total = vp->plm->total;
		for (int i = 0; i < total && !doRender; i++){
			int plIdx = playlistManagerGetPlaylistNext(vp->plm, i-1);
			if (plIdx >= 0){
				PLAYLISTCACHE *plc = playlistManagerGetPlaylist(vp->plm, plIdx);
				if (plc)
					doRender = 1; //playlistGetTotal(plc);	
			}
		}
	}
		
	if (!doRender){
		//setPage(vp, PAGE_OVERLAY);
		
	}else if (doRender){
		int to = pl->startPosition+8;
		
		outlineTextEnable(vp->hw, 0xFF000000);
		renderPlaylistPlm(vp, vp->plm, frame, pl, &pl->startPosition, &to);
		outlineTextDisable(vp->hw);	

		// ensure we have artwork available for display (per playlist)
		TPLAYLIST2 *pl2 = getPagePtr(vp, PAGE_PLAYLIST2);
		for (int i = pl->startPosition; i <= to; i++){
			PLAYLISTCACHE *plc = playlistManagerGetPlaylist(vp->plm, i);
			if (plc && !plc->artHash)
				playlistMetaGetMeta(vp, pl2->metacb, plc, 0, 8);
		}
	}


	TBUTTONS *buttons = buttonsGetContainer(vp, PAGE_PLAYLISTPLM);
	buttonsRender(buttons->list, buttons->total, frame);

#if DRAWTOUCHRECTS
	PLAYLISTRENDERITEM *item = vp->plm->pr->item;
	int i = MAXPLRENDERITEMS;
	while(i--){
		if (item->playlistPosition >= 0)
			lDrawRectangle(frame, item->art.x1, item->art.y1, item->art.x2, item->art.y2, BLUE);
		else
			break;
		item++;
	}
	
	TBUTTON *button = buttons->list;
	for (i = 0; i < buttons->total; button++, i++){
		if (button->enabled)
			lDrawRectangle(frame, button->pos.x, button->pos.y, button->pos.x+button->image->width-1, button->pos.y+button->image->height-1, BLUE);
	}
#endif
	return 1;
}

int touchPlaylistPlm (TTOUCHCOORD *pos, const int flags, TVLCPLAYER *vp)
{
	TBUTTONS *buttons = buttonsGetContainer(vp, PAGE_PLAYLISTPLM);
	TBUTTON *button = buttons->list;
	TTOUCHCOORD bpos;
	TPLAYLISTPLM *pl = getPagePtr(vp, PAGE_PLAYLISTPLM);
	TPLAYLISTMANAGER *plm = vp->plm;
	TDRAGPOS *p = &plm->pr->dragLocation;
	
	static unsigned int lastId = 0;
	static int lasty;
	

	// dump input if Id doesn't match that of the current stream
	if (!flags){		// pen down
		if (lastId >= pos->id)
			return 0;
		lastId = pos->id;
	}else if (lastId != pos->id){
		return 0;	
	}

	// hack: detect if this input is a result from closing an overlayed page (eg; PAGE_META)
	// if so then dump input and block until pen up is signalled
	if (pl->dragState == -1){
		if (pos->pen == 1) pl->dragState = 0;
		return 0;
	}
	if (abs(pos->count-p->last.count) > 15 && !pos->pen && p->last.count && pos->dt < 60){
		pl->dragState = -1;
		return 0;
	}

	my_memcpy(&p->last, pos, sizeof(TTOUCHCOORD));
	if (!flags && pos->dt > 80){
		// detect and handle new drag
		p->pos.x -= p->d1.x;
		p->pos.y -= p->d1.y;
		p->d0.x = pos->x;
		p->d0.y = pos->y;
		p->d1.x = 0;
		p->d1.y = 0;
		pl->dragState = 1;
		pl->decayTop = 0.0;
		pl->tOn = getTime(vp);

		lasty =  p->d0.y - pos->y;
	}
	if (pos->pen == 1) pl->dragState = 2;

	if (!flags){
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
						button->callback(&bpos, button, button->id, flags, vp);
						return 0;
					}
				}
			}
		}
	}

	// check if a playlist item (title or artwork) was selected
	if ((!flags || flags == 1) && abs(p->d1.y) < TOUCHDEADZONESIZE && pos->dt > 80){ // pen up without drag
		PLAYLISTRENDERITEM *item = plm->pr->item;
		int i = MAXPLRENDERITEMS;
		
		while(i--){
			if (item->playlistPosition >= 0){
				if (pos->y >= item->art.y1 && pos->y <= item->art.y2){
					if (pos->x >= item->art.x1 && pos->x <= item->art.x2){
						plm->pr->selectedItem = item->playlistPosition;
						vp->displayPlaylist = plm->pr->selectedItem;
						setPage(vp, PAGE_PLAYLIST2);
						break;
					}
				}
			}else{
				break;
			}
			item++;
		}
	}
	
	p->d1.x = p->d0.x - pos->x;
	p->d1.y = p->d0.y - pos->y;
	pl->voffset -= p->d1.y - lasty;
	lasty = p->d1.y;

	if (pl->dragState == 2){
		pl->dragState = 0;

		float t1 = getTime(vp) - pl->tOn;
		if (t1 > 0 && abs(p->d1.y) > TOUCHDEADZONESIZE){
			pl->decayTop = abs(p->d1.y)/(float)t1;
		}
	}
	return 0;
}


 