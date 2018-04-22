
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



#if G19DISPLAY
	#define ARTSCALEFUDGE (1.7)
#else
	#define ARTSCALEFUDGE (2.0)
#endif



extern int RED;
extern int GREEN;
extern int BLUE;
extern int BLACK;
extern int WHITE;




void cycleArtSize (TPLAYLIST2 *pl)
{
  	pl->artScale += 0.10;
	if (pl->artScale > 1.05)
		pl->artScale = 0.0;
}

void playlist2SetStartTrack (TPLAYLIST2 *pl, PLAYLISTCACHE *plc, const int start)
{
	pl->startPosition = start;
	pl->voffset = 0;
	pl->decayTop = 0;
	plc->pr->dragLocation.d1.y = 0;
}

int buttonPlaylist2 (const TTOUCHCOORD *pos, TBUTTON *button, const int id, const int message, void *ptr)
{
	if (pos->dt < 100) return 0;
	
	TVLCPLAYER *vp = (TVLCPLAYER*)ptr;

	switch (id){
	  case P2BUTTON_CTRL:
	  	setPageSec(vp, PAGE_PLOVERLAY);
	  	break;
	  	
	  case P2BUTTON_CLOSE:
		setPageSec(vp, -1);
		setPage(vp, PAGE_PLAYLISTPLM);
		break;
	 }
	return 1;
}	  

int closePlaylist2 (TVLCPLAYER *vp, TFRAME *frame, TPLAYLIST2 *pl)
{
	playlistMetaShutdown(pl->metacb);

	buttonsDeleteContainer(vp, PAGE_PLAYLIST2);
	lDeleteFrame(pl->lineRender);	
	for (int i = 0; i < PL2ARTCACHESIZE; i++){
		lDeleteFrame(pl->artcache[i]->frame);
		my_free(pl->artcache[i]);
	}

	return 1;
}

int openPlaylist2 (TVLCPLAYER *vp, TFRAME *frame, TPLAYLIST2 *pl)
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
	pl->dragState = 0;
	pl->artScale = DEFAULTARTSCALE;


#if !G19DISPLAY		// setup for the usbd480 - 480x272
	int x = 6;
	const int top = 6;
#else				// G19 - 320/240
	int x = 0;
	const int top = 0;
#endif
	int y = top;
		
	buttonsCreateContainer(vp, PAGE_PLAYLIST2, P2BUTTON_TOTAL);

#if !G19DISPLAY

	TBUTTON *button = buttonGet(vp, PAGE_PLAYLIST2, P2BUTTON_CLOSE);
	button->enabled = 1;
	button->id = P2BUTTON_CLOSE;
	button->pos.x = frame->width-(64+x);
	button->pos.y = y = top;
	button->canAnimate = 0;
	button->callback = buttonPlaylist2;
	buttonSetImages(vp, button, L"media/close.png", NULL);

	button = buttonGet(vp, PAGE_PLAYLIST2, P2BUTTON_CTRL);
	button->enabled = 1;
	button->id = P2BUTTON_CTRL;
	button->pos.x = frame->width-(64+64+x);
	button->pos.y = y = top;
	button->canAnimate = 0;
	button->callback = buttonPlaylist2;
	buttonSetImages(vp, button, L"media/plctrlon.png", NULL);
#else

	TBUTTON *button = buttonGet(vp, PAGE_PLAYLIST2, P2BUTTON_CTRL);
	button->enabled = 1;
	button->id = P2BUTTON_CTRL;
	button->pos.x = frame->width-(64+x);
	button->pos.y = y = top;
	button->canAnimate = 0;
	button->callback = buttonPlaylist2;
	buttonSetImages(vp, button, L"media/plctrlon.png", NULL);

	button = buttonGet(vp, PAGE_PLAYLIST2, P2BUTTON_CLOSE);
	button->enabled = 1;
	button->id = P2BUTTON_CLOSE;
	button->pos.x = frame->width-(64+x);
	button->pos.y = y + 66;
	button->canAnimate = 0;
	button->callback = buttonPlaylist2;
	buttonSetImages(vp, button, L"media/close.png", NULL);
#endif

	imageCacheAddImage(vp, L"noart128x128.png", LFRM_BPP_32A, &vp->gui.image[IMGC_NOART2]);
	
	pl->metacb = playlistMetaInit(vp);
	if (pl->metacb && pl->metacb->hThread)
		ResumeThread((HANDLE)pl->metacb->hThread);
	return 1;
}

static void clearSurface (TFRAME *sur)
{
	memset(sur->pixels, 0, sur->frameSize);
}

static int drawArtwork (TVLCPLAYER *vp, TPLAYLIST2 *pl, TFRAME *frame, const unsigned int hash, const float scale, PLAYLISTRENDERITEM *item, const int x, const int y, const int bsize, const int bg)
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

			if (bg == 0x02 || bg == 0x03)
				lDrawRectangleFilled(frame, item->art.x2, item->art.y1, frame->width-2, item->art.y2-1, (80<<24) | 0xFF00FF);
			else if (bg == 0x01)
				lDrawRectangleFilled(frame, item->art.x2, item->art.y1, frame->width-2, item->art.y2-1, (80<<24) | 0xFF0000);

			fastFrameCopy(pl->artcache[i]->frame, frame, item->art.x1, item->art.y1);
			return (w<<16)|(h&0xFFFF);
		}
	}

	if (tagLock(vp->tagc)){
		TTAGIMG *art = g_tagArtworkGetByHash(vp->tagc, hash);
		if (art){
			if (art->img){
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
				
				if (bg == 0x02 || bg == 0x03)
					lDrawRectangleFilled(frame, item->art.x2, item->art.y1, frame->width-2, item->art.y2-1, (80<<24) | 0xFF00FF);
				else if (bg == 0x01)
					lDrawRectangleFilled(frame, item->art.x2, item->art.y1, frame->width-2, item->art.y2-1, (80<<24) | 0xFF0000);
				
				drawImageScaled(art->img, artc->frame, 0, 0, scale);
				fastFrameCopy(artc->frame, frame, item->art.x1, item->art.y1);
				if (++pl->cacheIdx >= PL2ARTCACHESIZE) pl->cacheIdx = 0;
			}
		}
		tagUnlock(vp->tagc);
	}
	return (w<<16)|(h&0xFFFF);
}

static int lprint (TFRAME *frm, const char *buffer, const int x, const int y, const int font, const int style, int *w, int *h)
{
	TLPRINTR rect;
	rect.bx1 = rect.sx = rect.ex = x;
	rect.by1 = rect.sy = rect.ey = y;
	rect.bx2 = frm->width-1;
	rect.by2 = frm->height-1;
	const int flags = PF_CLIPTEXTH|PF_CLIPTEXTV|PF_NOESCAPE|PF_CLIPDRAW|PF_DONTFORMATBUFFER;
	const int ret = lPrintEx(frm, &rect, font, flags, style, buffer, NULL);
	if (ret){
		*w = (rect.ex - rect.sx)+1;
		*h = (rect.ey - rect.sy)+2;
	}
	return ret;
}

int printLine (TFRAME *frame, TFRAME *tmp, const char *buffer, const int x, const int y)
{
	int w = 0, h = 0;
	clearSurface(tmp);
	if (y >= 0 && y < frame->height-16){
		lprint(frame, buffer, x+1, y+1, BFONT, LPRT_CPY, &w, &h);
	}else{
		lprint(tmp, buffer, 1, 1, BFONT, LPRT_CPY, &w, &h);
		drawImage(tmp, frame, x, y, (w+(4-(w%4)))-1, h);
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

int renderPlaylist2 (TVLCPLAYER *vp, PLAYLISTCACHE *plc, TFRAME *frame, TPLAYLIST2 *pl, int *posFrom, int *posTo)
{

	PLAYLISTRENDERITEM *item = plc->pr->item;
	TFRAME *txt = pl->lineRender;
	
	unsigned int hash;
	const int x = pl->x;
	int pos = *posFrom;
	char title[MAX_PATH_UTF8];
	char tagbuffer[MAX_PATH_UTF8];
	int ax, ay, w, h, y;
	char buffer[MAX_PATH_UTF8];
	int pre = 0;
	const PLAYLISTCACHE *plcQ = getQueuedPlaylist(vp);
	TFBROWSER *fb = getPagePtr(vp, PAGE_BROWSER);
	int th = fb->print->textHeight;

	if (pl->decayTop > PLAYLIST_DECAYRATE){
		pl->decayTop -= PLAYLIST_DECAYRATE;
		if (plc->pr->dragLocation.d1.y > 0){
			plc->pr->dragLocation.d1.y += pl->decayTop*PLAYLIST_DECAYFACTOR;
			pl->voffset -= (pl->decayTop*PLAYLIST_DECAYFACTOR);
		}else{
			plc->pr->dragLocation.d1.y -= pl->decayTop*PLAYLIST_DECAYFACTOR;
			pl->voffset += (pl->decayTop*PLAYLIST_DECAYFACTOR);
		}
	}

	// if art is disabled then set height to text height
	if (pl->artScale > 0.05)
		pl->aHScaled = ((float)MAXARTHEIGHT*pl->artScale);
	else
		pl->aHScaled = 13.0;	
	
	if (pl->voffset < -((int)pl->aHScaled)){
		pl->voffset = pl->voffset - -(pl->aHScaled);
		pos = ++(*posFrom);
	}else if (pl->voffset > 1){
		pl->voffset = -(pl->aHScaled);
		pos = --(*posFrom);
	}

	const int playlistTotal = playlistGetTotal(plc);
	if (pos >= playlistTotal-1){
		pos = *posFrom = playlistTotal-1;
		pl->voffset = 0;
	}else if (pos < 0){
		pos = *posFrom = 0;
		pl->voffset = 0;
	}
	
	y = pl->y + pl->voffset;
	float bsize = pl->aHScaled;
	if (bsize < 13.0) bsize = 13.0;
	while(y < -bsize) y += bsize + 3;	

	do{
		w = 0; h = 0;

		hash = playlistGetHash(plc, pos);
		if (!hash){
			printf("hash == 0 for track %i (%p)\n", pos, plc);
			break;
		}
				
		playlistGetTitle(plc, pos, title, sizeof(title));
		if (!*title){
			playlistGetPath(plc, pos, title, sizeof(title));
			if (!*title) break;
		}
		
		if (bsize > 14.0){
			const int bg  = ((pos == plc->pr->playingItem && plc == plcQ) << 1) | (pos == plc->pr->selectedItem);
			w = drawArtwork(vp, pl, frame, hash, pl->artScale, item, x, y, bsize, bg);
			h = w&0xFFFF;
			w >>= 16;
			
			if (!w){
				h = w = bsize;

				TFRAME *img = imageCacheGetImage(vp, vp->gui.image[IMGC_NOART2]);
				if (img){
					// we use a 2.0 scale as normally scale 1.0 == w/h of 256
					// but as the noart icon size is w/h 128 we need the scale fudge

					item->art.x1 = x;
					item->art.y1 = y;
					item->art.x2 = item->art.x1 + ((img->width-1) * (pl->artScale*ARTSCALEFUDGE));
					item->art.y2 = item->art.y1 + ((img->height-1) * (pl->artScale*ARTSCALEFUDGE));
					if (item->art.y2 > frame->height-1)
						item->art.y2 = frame->height-1;

					if (bg == 0x02 || bg == 0x03)
						lDrawRectangleFilled(frame, item->art.x1, item->art.y1, frame->width-2, item->art.y2-1, (80<<24) | 0xFF00FF);
					else if (bg == 0x01)
						lDrawRectangleFilled(frame, item->art.x1, item->art.y1, frame->width-2, item->art.y2-1, (80<<24) | 0xFF0000);

					drawImageScaled(img, frame, x, y, pl->artScale*ARTSCALEFUDGE);
				}else{
					drawPlaceholder(frame, item, x, y, bsize);
				}
			}
			
			// draw item selection box around the image
			if (bg == 0x02){
				lDrawRectangle(frame, item->art.x1-1, item->art.y1-1, item->art.x2+1, item->art.y2+1, 0xFFFF00FF);
				lDrawRectangle(frame, item->art.x1, item->art.y1, item->art.x2, item->art.y2, 0xFFFF00FF);
			}else if (bg == 0x01 || bg == 0x03){
				lDrawRectangle(frame, item->art.x1-1, item->art.y1-1, item->art.x2+1, item->art.y2+1, 0xFFFF0000);
				lDrawRectangle(frame, item->art.x1, item->art.y1, item->art.x2, item->art.y2, 0xFFFF0000);
			}

			item->playlistPosition = pos;
			item++;
		}

		if (bsize > 14.0){	// with artwork enabled
			ax = x + w + 7;
			ay = y + (abs(bsize - h)/2) - 2;
		}else{				// with artwork disabled
			ax = x;
			ay = y;
		}
		
		if (bsize <= 100.0){
			if (bsize <= 14.0){
				if (pos == plc->pr->playingItem)
					pre = lSetForegroundColour(frame->hw, 0xFFFF00FF);
				else if (pos == plc->pr->selectedItem)
					pre = lSetForegroundColour(frame->hw, 0xFFFF0000);
			}
				
			snprintf(buffer, sizeof(buffer), "%i:", pos+1);
			w = printLine(frame, txt, buffer, ax, ay);
			h = w&0xFFFF;
			w >>= 16;
			
			if (bsize <= 14.0 && (pos == plc->pr->playingItem || pos == plc->pr->selectedItem))
				lSetForegroundColour(frame->hw, pre);

			int w2 = printLine(frame, txt, title, ax+w, ay);
			h = w2&0xFFFF;
			w2 >>= 16;
						
			if (bsize <= 14.0){
				item->art.x1 = ax;
				item->art.y1 = ay+2;
				item->art.x2 = ax+w+w2;
				item->art.y2 = ay+h;
				item->playlistPosition = pos;
				item++;
			}
		}else{
			snprintf(buffer, sizeof(buffer), "%i:", pos+1);
			printLine(frame, txt, buffer, ax, ay);
			ay += th;
			printLine(frame, txt, title, ax, ay);
		}
		if (bsize > 26.0){
			tagRetrieveByHash(vp->tagc, hash, MTAG_Artist, tagbuffer, sizeof(tagbuffer));
			if (*tagbuffer)
				ay += th;
				printLine(frame, txt, tagbuffer, ax, ay);

			if (bsize > 55.0){
				if (bsize > 80.0){
					tagRetrieveByHash(vp->tagc, hash, MTAG_Album, tagbuffer, sizeof(tagbuffer));
					if (*tagbuffer){
						ay += th;
						printLine(frame, txt, tagbuffer, ax, ay);
					}
				}
				tagRetrieveByHash(vp->tagc, hash, MTAG_LENGTH, tagbuffer, sizeof(tagbuffer));
				if (*tagbuffer && tagbuffer[1]){
					ay += th;
					printLine(frame, txt, tagbuffer, ax, ay);
				}

				// draw a playback progress bar
				if (pos == plc->pr->playingItem){
					if (plc == plcQ){
						const int x = ax + 4;
						const int w = (frame->width-8) - x;
						float tpos = (float)w * vp->vlc->position;
						if (tpos < 0.0) tpos = 0.0;
#if 0
						ay += th+5;
						lDrawRectangle(frame, x-1, ay-1, x+w+1, ay+12+1, (100<<24) | 0x00FF00);
						lDrawRectangle(frame, x, ay, x+w, ay+12, (120<<24) | 0x00FF00);
						lDrawRectangleFilled(frame, (x-4)+tpos, ay+1, (x+4)+tpos, ay+11, (120<<24) | 0x00FF00);
#elif  0
						ay += th+7;
						lDrawRectangleFilled(frame, x, ay, x+tpos, ay+7, (160<<24) | 0x00FF00);
						lDrawRectangleFilled(frame, x+tpos+1, ay, x+w, ay+7, (80<<24) | 0x00FF00);
#else
						tpos += x;
						ay += th+7;
						lDrawLine(frame, x, ay-1, tpos-1, ay-1, (80<<24) | 0x00FF00);
						lDrawRectangleFilled(frame, x, ay, tpos-1, ay+7, (160<<24) | 0x00FF00);
						lDrawLine(frame, tpos, ay-1, tpos, ay+8, (50<<24) | 0x00FF00);
						lDrawRectangleFilled(frame, tpos, ay, x+w, ay+7, (80<<24) | 0x00FF00);
						lDrawLine(frame, x, ay+8, tpos-1, ay+8, (80<<24) | 0x00FF00);
#endif
					}
				}
			}
		}
		y += bsize + 3;
	}while(y < frame->height-2 && ++pos < playlistTotal);

	*posTo = pos;
	item->playlistPosition = -1;
	return 1;
}

int drawPlaylist2 (TVLCPLAYER *vp, TFRAME *frame, TPLAYLIST2 *pl)
{
	if (getPageSec(vp) == PAGE_PLOVERLAY){
		buttonDisable(vp, PAGE_PLAYLIST2, P2BUTTON_CTRL);
		buttonDisable(vp, PAGE_PLAYLIST2, P2BUTTON_CLOSE);
	}else{
		buttonEnable(vp, PAGE_PLAYLIST2, P2BUTTON_CTRL);
		buttonEnable(vp, PAGE_PLAYLIST2, P2BUTTON_CLOSE);
	}

	PLAYLISTCACHE *plcD = getDisplayPlaylist(vp);
	if (playlistGetTotal(plcD)){
		lSetCharacterEncoding(vp->hw, CMT_UTF8);
		lSetBackgroundColour(vp->hw, BLACK);
		lSetForegroundColour(vp->hw, 0xFFF0F0F0);
		
		const int pre = pl->startPosition;
		int to = pl->startPosition+8;

		outlineTextEnable(vp->hw, 0xFF000000);	// ensure the text stands out in the foreground
		renderPlaylist2(vp, plcD, frame, pl, &pl->startPosition, &to);
		outlineTextDisable(vp->hw);

		if (pre != pl->startPosition){
			if (pre < pl->startPosition)	// forward/down
				playlistMetaGetMeta(vp, pl->metacb, plcD, to-1, to+8);
			else							// backward/up
				playlistMetaGetMeta(vp, pl->metacb, plcD, pl->startPosition-8, pl->startPosition+1);
		}

	}

	TBUTTONS *buttons = buttonsGetContainer(vp, PAGE_PLAYLIST2);
	buttonsRender(buttons->list, buttons->total, frame);

#if DRAWTOUCHRECTS
	PLAYLISTRENDERITEM *item = plcD->pr->item;
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

int touchPlaylist2 (TTOUCHCOORD *pos, const int flags, TVLCPLAYER *vp)
{
	TBUTTONS *buttons = buttonsGetContainer(vp, PAGE_PLAYLIST2);
	TBUTTON *button = buttons->list;
	TTOUCHCOORD bpos;
	TPLAYLIST2 *pl = getPagePtr(vp, PAGE_PLAYLIST2);
	
	PLAYLISTCACHE *plcD = getDisplayPlaylist(vp);
	TDRAGPOS *p = &plcD->pr->dragLocation;
	
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

	//printf("%p, %i %i %i %i %i %i dragState:%i\n", pl, pos->x, pos->y, lastId, lasty, flags, pos->dt, pl->dragState);


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
		
	//	printf("drag enabled\n");
	}
	if (pos->pen == 1){
		pl->dragState = 2;
	//	printf("drag state = 2\n");
	}

	// did we click a button
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
		PLAYLISTRENDERITEM *item = plcD->pr->item;
		int i = MAXPLRENDERITEMS;

		while(i--){
			if (item->playlistPosition >= 0){
				if (pos->y >= item->art.y1 && pos->y <= item->art.y2){
					if (pos->x >= item->art.x1 && pos->x <= item->art.x2){
						if (plcD->pr->selectedItem != item->playlistPosition){
							plcD->pr->selectedItem = item->playlistPosition;
						}else{
							plcD->pr->selectedItem = -1;
						}
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


 