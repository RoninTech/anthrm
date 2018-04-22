
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


// keep this in sync with tags.h:enum _tags_meta and editbox.c:wtagStrLookup[]
static const char *tagStrLookup[] = {
	"Title",
    "Artist",
    "Genre",
    "Copyright",
    "Album",
    "Track #",
    "Description",
    "Rating",
    "Date",
    "Setting",
    "URL",
    "Language",
    "Now Playing",
    "Publisher",
    "Encoded By",
    "Path",
    "TrackID",
    "Length",
    "Filename",
    "Playlist #",
    "Path",
    ""
};

int metaOpenPageAndReturn (TVLCPLAYER *vp)
{
	TMETA *meta = getPagePtr(vp, PAGE_META);
	meta->pageReturn = getPage(vp);
	meta->pageSecReturn = getPageSec(vp);
	setPage(vp, PAGE_NONE);
	setPageSec(vp, PAGE_META);
	return meta->pageReturn;
}

int buttonMeta (const TTOUCHCOORD *pos, TBUTTON *button, const int id, const int message, void *ptr)
{
	TVLCPLAYER *vp = (TVLCPLAYER*)ptr;
	TPLAYLIST2 *pl = getPagePtr(vp, PAGE_PLAYLIST2);
	TMETA *meta = getPagePtr(vp, PAGE_META);

	PLAYLISTCACHE *plc;
	if (meta->pageReturn == PAGE_OVERLAY){
	  	plc = getQueuedPlaylist(vp);
	  	if (!plc)
	  		plc = getDisplayPlaylist (vp);
	}else{
		plc = getDisplayPlaylist (vp);
	}
	if (!plc) return 0;

	switch (id){
	  case METABUTTON_CLOSE:
	  	if (meta->pageReturn > PAGE_NONE){
	  		if (meta->pageReturn == PAGE_OVERLAY){
				enableOverlay(vp);
				setPageSec(vp, -1);
	  		}else{
	  			setPage(vp, meta->pageReturn);
	  			setPageSec(vp, meta->pageSecReturn);
	  		}
	  	}
  		meta->pageReturn = -1;
  		meta->pageSecReturn = -1;
  		return -1;

	  case METABUTTON_UP:
		if (--meta->trackPosition  < 0)
			meta->trackPosition = playlistGetTotal(plc)-1;

		plc->pr->selectedItem = meta->trackPosition;
		playlistMetaGetMeta(vp, pl->metacb, plc, plc->pr->selectedItem-CACHEREADAHEAD, plc->pr->selectedItem);
	  	break;	  
	  
	  case METABUTTON_LEFT:
		if (--meta->trackPosition  < 0)
			meta->trackPosition = playlistGetTotal(plc)-1;
		plc->pr->selectedItem = meta->trackPosition;
		plc->pr->playingItem = meta->trackPosition;
		startPlaylistTrack(vp, plc, meta->trackPosition);
		trackLoadEvent(vp, plc, pl, meta->trackPosition);
		playlistMetaGetMeta(vp, pl->metacb, plc, plc->pr->selectedItem-CACHEREADAHEAD, plc->pr->selectedItem);
	  	break;
	  
	  case METABUTTON_DOWN:
		if (++meta->trackPosition >= playlistGetTotal(plc))
			meta->trackPosition = 0;
		plc->pr->selectedItem = meta->trackPosition;

		playlistMetaGetMeta(vp, pl->metacb, plc, plc->pr->selectedItem, plc->pr->selectedItem+CACHEREADAHEAD);
	  	break;
	  
	  case METABUTTON_RIGHT:
		if (++meta->trackPosition >= playlistGetTotal(plc))
			meta->trackPosition = 0;
		plc->pr->selectedItem = meta->trackPosition;
		plc->pr->playingItem = meta->trackPosition;
		startPlaylistTrack(vp, plc, meta->trackPosition);
		trackLoadEvent(vp, plc, pl, meta->trackPosition);
		playlistMetaGetMeta(vp, pl->metacb, plc, plc->pr->selectedItem, plc->pr->selectedItem+CACHEREADAHEAD);
	  
	  	break;
	 };
	return 0;
}	  

int closeMeta (TVLCPLAYER *vp, TFRAME *frame, TMETA *meta)
{
	buttonsDeleteContainer(vp, PAGE_META);
	return 1;
}

int openMeta (TVLCPLAYER *vp, TFRAME *frame, TMETA *meta)
{	
	meta->trackPosition = -1;
	meta->x = 10;
	meta->y = 10;
	meta->w = frame->width-(meta->x*2);
	meta->h = frame->height-(meta->y*2);
	meta->pageReturn = -999;
	
	buttonsCreateContainer(vp, PAGE_META, METABUTTON_TOTAL);

	TBUTTON *button = buttonGet(vp, PAGE_META, METABUTTON_DOWN);
	button->id = METABUTTON_DOWN;
	button->enabled = 1;
	button->canAnimate = 0;
	button->acceptDrag = 0;
	button->callback = buttonMeta;
	buttonSetImages(vp, button, L"mdown.png", NULL);
	button->pos.x = abs(frame->width - button->image->width)/2.0;
	button->pos.y = frame->height - button->image->height;
		
	button = buttonGet(vp, PAGE_META, METABUTTON_RIGHT);
	button->id = METABUTTON_RIGHT;
	button->enabled = 1;
	button->canAnimate = 0;
	button->acceptDrag = 0;
	button->callback = buttonMeta;
	buttonSetImages(vp, button, L"mright.png", NULL);
	button->pos.x = frame->width - button->image->width;
	button->pos.y = abs(frame->height - button->image->height)/2.0;

	button = buttonGet(vp, PAGE_META, METABUTTON_UP);
	button->id = METABUTTON_UP;
	button->enabled = 1;
	button->canAnimate = 0;
	button->acceptDrag = 0;
	button->callback = buttonMeta;
	buttonSetImages(vp, button, L"mup.png", NULL);
	button->pos.x = abs(frame->width - button->image->width)/2.0;
	button->pos.y = 0;
	
	button = buttonGet(vp, PAGE_META, METABUTTON_LEFT);
	button->id = METABUTTON_LEFT;
	button->enabled = 1;
	button->canAnimate = 0;
	button->acceptDrag = 0;
	button->callback = buttonMeta;
	buttonSetImages(vp, button, L"mleft.png", NULL);
	button->pos.x = 0;
	button->pos.y = abs(frame->height - button->image->height)/2.0;

	button = buttonGet(vp, PAGE_META, METABUTTON_CLOSE);
	button->id = METABUTTON_CLOSE;
	button->enabled = 1;
	button->canAnimate = 0;
	button->acceptDrag = 1;
	button->callback = buttonMeta;
	buttonSetImages(vp, button, L"mclose.png", NULL);
	button->pos.x = (frame->width - button->image->width)/2;
	button->pos.y = (frame->height - button->image->height)/2;

	return 1;
}

int drawArtwork (TVLCPLAYER *vp, TFRAME *frame, PLAYLISTCACHE *plc, const int trackPosition, const int x, const int y, const int w, const int h)
{

	int ret = 0;

	const unsigned int hash = playlistGetHash(plc, trackPosition);
	if (hash){
		if (tagLock(vp->tagc)){
			TTAGIMG *art = g_tagArtworkGetByHash(vp->tagc, hash);
			if (art){
				if (art->enabled){
					const int gap = abs(h-art->img->height)/2.0;
					const int fh = art->img->height;
					const int fw = art->img->width;
					fadeArea(frame, (x+w)-fw, y, x+w, y+gap);			// fade area above art
					fadeArea(frame, (x+w)-fw, y+gap+fh+1, x+w, y+h);	// fade below art
					fadeArea(frame, x+w, y+gap+1, x+w, y+gap+fh);		// fade a one pixel column to right of art
					fastFrameCopy(art->img, frame, (x+w)-fw, y+gap+1);
					ret = fw+1;
				}
			}
			tagUnlock(vp->tagc);
		}
	}
	return ret;
}

int renderMeta (TVLCPLAYER *vp, TFRAME *frame, PLAYLISTCACHE *plc, TMETA *meta, const int drawui)
{
	const int x = meta->x;
	const int y = meta->y;
	const int w = meta->w;
	const int h = meta->h;
	const int font = MFONT;
	char tag[MAX_PATH_UTF8];
	TLPRINTR rt;
	
	
	if (drawui){
		buttonDisable(vp, PAGE_META, METABUTTON_CLOSE);
		const TBUTTONS *buttons = buttonsGetContainer(vp, PAGE_META);
		buttonsRender(buttons->list, buttons->total, frame);
		buttonEnable(vp, PAGE_META, METABUTTON_CLOSE);
	}
	
	const int aw = drawArtwork(vp, frame, plc, meta->trackPosition, meta->x, meta->y, meta->w, meta->h);
	
	if (drawui){
		fadeArea(frame, x, y, (x+w)-aw, y+h);
		lDrawRectangle(frame, x-1, y-1, x+w+1, y+h+1, 150<<24 | 0xFFFFFF);
	}
	lSetCharacterEncoding(vp->hw, CMT_UTF8);
		
	// confine text rendering to the inter rectangle of the meta frame
	rt.bx1 = x+3; rt.by1 = y+2;
	rt.bx2 = (x+w)-1;
	rt.by2 = (y+h)-1;
	rt.sx = rt.bx1; rt.sy = rt.by1;

	int pos;
	if (meta->trackPosition >= 0){
		pos = meta->trackPosition;
	}else{
		pos = plc->pr->playingItem;
		if (pos < 0) pos = 0;
	}

	const unsigned int hash = playlistGetHash(plc, pos);
	if (!hash) return 0;


	TFBROWSER *fb = getPagePtr(vp, PAGE_BROWSER);
	const int fh = fb->print->textHeight+4;
	
	shadowTextEnable(frame->hw, 0x000000, 250);

	for (int i = 0; i < MTAG_TOTAL; i++){
		if (i != MTAG_ArtworkURL){
			tagRetrieveByHash(vp->tagc, hash, i, tag, sizeof(tag));
			if (*tag){
				lSetForegroundColour(vp->hw, 200<<24 | 0xFFFFFF);
				lPrintEx(frame, &rt, font, PF_CLIPWRAP|PF_CLIPDRAW, LPRT_CPY, "%s: ",tagStrLookup[i]);
				lSetForegroundColour(vp->hw, 255<<24 | 0xFFFFFF);
				lPrintEx(frame, &rt, font, PF_USELASTX|PF_CLIPWRAP|PF_CLIPDRAW, LPRT_CPY, tag);
				rt.sx = rt.bx1;				
				rt.sy += fh;		// new line
			}
		}
	}
	shadowTextDisable(frame->hw);

#if DRAWTOUCHRECTS
	if (drawui){
		TBUTTONS *buttons = buttonsGetContainer(vp, PAGE_META);
		TBUTTON *button = buttons->list;
		for (int i = 0; i < buttons->total; button++, i++){
			if (button->enabled)
				lDrawRectangle(frame, button->pos.x, button->pos.y, button->pos.x+button->image->width-1, button->pos.y+button->image->height-1, 0xFF0000FF);
		}
	}
#endif					
	return 1;
}


int drawMeta (TVLCPLAYER *vp, TFRAME *frame, TMETA *meta)
{
	PLAYLISTCACHE *plc;
	if (meta->pageReturn == PAGE_OVERLAY){
	  	plc = getQueuedPlaylist(vp);
	  	if (!plc)
	  		plc = getDisplayPlaylist (vp);
	}else{
		plc = getDisplayPlaylist (vp);
	}
	if (plc)
		return renderMeta(vp, frame, plc, meta, 1);
	else 
		return 0;
}

int touchMeta (TTOUCHCOORD *pos, int flags, TVLCPLAYER *vp)
{
	TBUTTONS *buttons = buttonsGetContainer(vp, PAGE_META);
	TBUTTON *button = buttons->list;
	TTOUCHCOORD bpos;
	static unsigned int lastId = 0;
	
	// we don't want drag reports
	if (pos->dt < 80 || flags) return -2;

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
					return button->callback(&bpos, button, button->id, flags, vp);
				}
			}
		}
	}
	return 0;
}

