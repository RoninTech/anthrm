// http://mylcd.sourceforge.net/
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


/*
	VLCStream for VLC 1.1.x

Install:
Copy vlcstream.exe and the vsskin directory to the root VLC directory where vlc.exe is located

Start with: 'vlcstream.exe "path/to/mediafile.ext"' or use the built-in explorer to initiate media playback
*/

#include "common.h"
#include "dshowconfig.h"
#include "mylcdsetup.h"
#include "../../src/rgbmasks.h"



const TGOOMRESSTR goomres[] = {
	GOOMREZMODES
};


void closeVLCInstance (TVLCPLAYER *vp, TVLCCONFIG *vlc);
static int initVLC (TVLCPLAYER *vp);
static void closeVLC (TVLCPLAYER *vp);
void vlc_configure (TVLCPLAYER *vp, TVLCCONFIG *vlc, const int width, const int height, const int bpp, const int visMode);



void trackPrev (TVLCPLAYER *vp)
{
	PLAYLISTCACHE *plcQ = getQueuedPlaylist(vp);
	if (plcQ)
		trackLoadEvent(vp, plcQ, getPagePtr(vp, PAGE_PLAYLIST2), player_prevTrack(vp, vp->vlc));
}

void trackNext (TVLCPLAYER *vp)
{
	PLAYLISTCACHE *plcQ = getQueuedPlaylist(vp);
	if (plcQ)
		trackLoadEvent(vp, plcQ, getPagePtr(vp, PAGE_PLAYLIST2), player_nextTrack(vp, vp->vlc));
}

void trackPlayPause (TVLCPLAYER *vp)
{
	if (getPlayState(vp) == 1){
  		vp->vlc->playState = 2;
		player_pause(vp, vp->vlc);
	}else if (getPlayState(vp) == 2){
		player_play(vp, vp->vlc);
	}
}

void trackPlay (TVLCPLAYER *vp)
{
	player_play(vp, vp->vlc);
}

void trackStop (TVLCPLAYER *vp)
{
	player_stop(vp, vp->vlc);
}

void volumeUp (TVLCPLAYER *vp)
{
  	setVolume(vp, vp->vlc->volume + 5);
	enableOverlay(vp);
}

void volumeDown (TVLCPLAYER *vp)
{
	setVolume(vp, vp->vlc->volume - 5);
	enableOverlay(vp);
}
	
void setApplState (TVLCPLAYER *vp, int state)
{
	vp->applState = state;
}

static int getApplState (TVLCPLAYER *vp)
{
	return vp->applState;
}

int timerInit (TVLCPLAYER *vp, const int id, void (*func) (TVLCPLAYER *), void *ptr)
{
	if (id < TIMER_TOTAL){
		vp->timers.queue[id].func = func;
		vp->timers.queue[id].ptr = ptr;
		vp->timers.queue[id].state = 0;
		vp->timers.queue[id].time = 0;
		return 1;
	}
	return 0;
}

void timerReset (TVLCPLAYER *vp, const int id)
{
	vp->timers.queue[id].state = 0;
	vp->timers.queue[id].time = 0;
}

void timerSet (TVLCPLAYER *vp, const int id, const unsigned int ms)
{
	vp->timers.queue[id].state++;
	vp->timers.queue[id].time = (unsigned int)getTime(vp)+ms;
	if (!ms) SetEvent(vp->ctx.hEvent);
}

void timerFire (TVLCPLAYER *vp, const int id)
{
	if (getApplState(vp)){
		if (vp->timers.queue[id].state > 0){
			vp->timers.queue[id].state--;
			vp->timers.queue[id].func(vp);
		}
	}
}

void timerCheckAndFire (TVLCPLAYER *vp, const unsigned int t0)
{
	for (int i = 0; i < TIMER_TOTAL; i++){
		if (vp->timers.queue[i].state > 0){
			if (t0 >= vp->timers.queue[i].time){
				//printf("firing timer %i [%i] %i\n", i, t0-vp->timers.queue[i].time, vp->timers.queue[i].state);
				timerFire(vp, i);
			}
		}
	}
}

static int loadLock (TVLCPLAYER *vp)
{
	return (WaitForSingleObject(vp->gui.hLoadLock, INFINITE) == WAIT_OBJECT_0);
}

static void loadUnlock (TVLCPLAYER *vp)
{
	ReleaseMutex(vp->gui.hLoadLock);
}

int renderLock (TVLCPLAYER *vp)
{
	return (WaitForSingleObject(vp->gui.hRenderLock, INFINITE) == WAIT_OBJECT_0);
}

void renderUnlock (TVLCPLAYER *vp)
{
	ReleaseMutex(vp->gui.hRenderLock);
}

int getPage (TVLCPLAYER *vp)
{
	return vp->pages.active;
}

void setPage (TVLCPLAYER *vp, int id)
{
	if (kHookGetState()){
		kHookOff();
		kHookUninstall();
	}
	
	for (int i = 0; i < PAGE_TOTAL; i++)
		vp->pages.page[i].isEnabledPri = 0;

	vp->pages.active = id;
	vp->pages.page[id].isEnabledPri = 1;
	vp->pages.page[id].isEnabledSec = 0;
}

void setPageSec (TVLCPLAYER *vp, int id)
{
	if (kHookGetState()){
		kHookOff();
		kHookUninstall();
	}
	
	for (int i = 0; i < PAGE_TOTAL; i++)
		vp->pages.page[i].isEnabledSec = 0;

	if (id != -1)
		vp->pages.page[id].isEnabledSec = 1;
}

int getPageSec (TVLCPLAYER *vp)
{
	for (int i = 0; i < PAGE_TOTAL; i++){
		if (vp->pages.page[i].isEnabledSec)
			return i;
	}
	return -1;
}

float getTime (TVLCPLAYER *vp)
{
#if 1	
	uint64_t t1 = 0;
	QueryPerformanceCounter((LARGE_INTEGER*)&t1);
	return ((float)((uint64_t)(t1 - vp->tStart) * vp->resolution) * 1000.0);
#else
	return (float)timeGetTime();
#endif

}

void setAwake (TVLCPLAYER *vp)
{
	vp->gui.awakeTime = timeGetTime(); //GetTickCount();
	vp->gui.awake = 1;
}

static int isMediaLoaded (TVLCPLAYER *vp)
{
	return vp->vlc->isMediaLoaded;
}

static void setIdle (TVLCPLAYER *vp)
{
	vp->gui.awake = 0;

	PLAYLISTCACHE *plc = getPrimaryPlaylist(vp);
	
	if (playlistGetTotal(plc))
		setPage(vp, PAGE_NONE);
	else
		setPage(vp, PAGE_BROWSER);
	setPageSec(vp, PAGE_CLOCK);
}

// return 1 if idle otherwise 0
int getIdle (TVLCPLAYER *vp)
{
	return (vp->gui.awake == 0);
}


void setPlaybackMode (TVLCPLAYER *vp, int mode)
{
	/*
	0:audio or video with visualizations disabled
	1:audio with visualizations enabled
	*/
	vp->currentFType = mode;
}

int getPlaybackMode (TVLCPLAYER *vp)
{
	return vp->currentFType;
}
/*
static void setTunerChannel (TVLCPLAYER *vp, int idx)
{
	vp->tuner.channel = vp->tuner.channels[idx].channel;
}

static void setTunerCountry (TVLCPLAYER *vp, int region)
{
	vp->tuner.country = region;
}
*/
static TVLCCONFIG *selectVLCConfig (TVLCPLAYER *vp)
{
	return (vp->vlc = &vp->vlcconfig);
}

int startPlaylistTrack (TVLCPLAYER *vp, PLAYLISTCACHE *plc, const int track)
{
	int ret = 0;
	if (track >= 0){
		char path[MAX_PATH_UTF8];
		playlistGetPath(plc, track, path, sizeof(path));
		if (*path){
			
			wchar_t *out = converttow(path);
			TFBROWSER *fb = getPagePtr(vp, PAGE_BROWSER);
			if (!isVideoFile(out)){	// filter type must be set before track is started
				setExtFilter(vp, fb, EXT_AUDIO);
				setPlaybackMode(vp, 1);
			}else{
				setExtFilter(vp, fb, EXT_VIDEO);
				setPlaybackMode(vp, 0);
			}
			my_free(out);


			if (browserLoadMediaFile(vp, path)){
				//if (getPage(vp) == PAGE_PLAYLIST2)
					PLAYLISTCACHE *plc = getQueuedPlaylist(vp);
					if (plc)
						trackLoadEvent(vp, plc, getPagePtr(vp, PAGE_PLAYLIST2), track);
				ret = 1;
			}
		}
	}
	return ret;
}

void player_pause (TVLCPLAYER *vp, TVLCCONFIG *vlc)
{
	vlc_pause(vlc);
}

void player_play (TVLCPLAYER *vp, TVLCCONFIG *vlc)
{
	if (vlc->playEnded || vlc->playState != 2)
		vlc_stop(vlc);

	vlc->playState = 1;
	vlc->playEnded = 0;
	vlc_play(vlc);

}

void player_stop (TVLCPLAYER *vp, TVLCCONFIG *vlc)
{
	vlc_stop(vlc);
 	vlc->playState = 0;
 	vlc->playEnded = 0;
}

int player_prevTrack (TVLCPLAYER *vp, TVLCCONFIG *vlc)
{
	PLAYLISTCACHE *plc = getQueuedPlaylist(vp);
	PLAYLISTRENDER *pr = plc->pr;
	
	if (--pr->playingItem < 0)
		pr->playingItem = playlistGetTotal(plc)-1;
		
	startPlaylistTrack(vp, plc, pr->playingItem);
	return pr->playingItem;
}

int player_nextTrack (TVLCPLAYER *vp, TVLCCONFIG *vlc)
{
	PLAYLISTCACHE *plc = getQueuedPlaylist(vp);
	PLAYLISTRENDER *pr = plc->pr;
	
	if (++pr->playingItem >= playlistGetTotal(plc))
		pr->playingItem = 0;
		
	startPlaylistTrack(vp, plc, pr->playingItem);
	return pr->playingItem;
}
/*
// this is used to active a change to the dshow config only
static void restartVLC (TVLCPLAYER *vp)
{
	if (getPlayState(vp))
		player_stop(vp, vp->vlc);
	unloadMedia(vp, vp->vlc);
	closeVLC(vp);
	initVLC(vp);
}

int tuner_gotoChannel (TVLCPLAYER *vp, int channelIdx)
{
	if (vp->tuner.channelIdx >= CHN_TOTAL)
		vp->tuner.channelIdx = 0;
	else
		vp->tuner.channelIdx = channelIdx;
	
	restartVLC(vp);
	browserLoadMediaFile(vp, vp->currentFile);
	wprintf(L"@ %s\n", vp->tuner.channels[vp->tuner.channelIdx].name);
	return 1;
}

int tuner_prevChannel (TVLCPLAYER *vp)
{
	if (--vp->tuner.channelIdx < 0)
		vp->tuner.channelIdx = CHN_TOTAL-1;

//	TFBROWSER *fb = getPagePtr(vp, PAGE_BROWSER);
//	getPrevInPlaylist(fb->playlist);
	restartVLC(vp);
	browserLoadMediaFile(vp, vp->currentFile);
	wprintf(L"@ %s\n", vp->tuner.channels[vp->tuner.channelIdx].name);
	return 1;
}
			
int tuner_nextChannel (TVLCPLAYER *vp)
{
	if (++vp->tuner.channelIdx >= CHN_TOTAL)
		vp->tuner.channelIdx = 0;

//	TFBROWSER *fb = getPagePtr(vp, PAGE_BROWSER);
//	getNextInPlaylist(fb->playlist);
	restartVLC(vp);
	browserLoadMediaFile(vp, vp->currentFile);
	wprintf(L"@ %s\n", vp->tuner.channels[vp->tuner.channelIdx].name);
	return 1;
}
*/
static int waitForUpdateSignal (TVLCPLAYER *vp)
{
	return (WaitForSingleObject(vp->hUpdateEvent, 1500) == WAIT_OBJECT_0);
}

static int waitForVLCUpdateSignal (TVLCPLAYER *vp)
{
	return (WaitForSingleObject(vp->ctx.hEvent, 1) == WAIT_OBJECT_0);
}

void lockVLCVideoBuffer (TVLCPLAYER *vp)
{
	WaitForSingleObject(vp->ctx.hVLock, INFINITE);
}

void unlockVLCVideoBuffer (TVLCPLAYER *vp)
{
	ReleaseMutex(vp->ctx.hVLock);
}


static int isVLCFrameAvailable (TVLCPLAYER *vp)
{
	return 	(isMediaLoaded(vp)		&&
			 getApplState(vp)		&&
			(getPlayState(vp) >  0)	&&
			(getPlayState(vp) != 2));
}

void *vmem_lock (void *data, void **pp_ret)
{
	TVLCPLAYER *vp = (TVLCPLAYER*)data;
    lockVLCVideoBuffer(vp);
    *pp_ret = vp->ctx.pixels;
    return NULL;
}

void vmem_unlock (void *data, void *id, void *const *p_pixels)
{
	TVLCPLAYER *vp = (TVLCPLAYER*)data;
    unlockVLCVideoBuffer(vp);
}

void vmem_display (void *data, void *id)
{
	TVLCPLAYER *vp = (TVLCPLAYER*)data;
	SetEvent(vp->ctx.hEvent);
}

void cleanVideoBuffers (TVLCPLAYER *vp)
{
	lockVLCVideoBuffer(vp);
	memset(lGetPixelAddress(vp->ctx.vmem, 0, 0), 0, vp->ctx.vmem->frameSize);
	memset(lGetPixelAddress(vp->ctx.working, 0, 0), 0, vp->ctx.working->frameSize);
	unlockVLCVideoBuffer(vp);
}

static void closeVLC (TVLCPLAYER *vp)
{
	closeVLCInstance(vp, selectVLCConfig(vp));
}


int writePlaylist (TVLCPLAYER *vp, PLAYLISTCACHE *plc, wchar_t *buffer, size_t len)
{
	int ret = 0;
	
	if (playlistGetTotal(plc) > 0){
		wchar_t buffertime[64];
		time_t t = time(0);
		struct tm *tdate = localtime(&t);
	
		if (wcsftime(buffertime, sizeof(buffertime), L"%d%m%y-%H%M%S", tdate) > 1){
			if (snwprintf(buffer, len, L"vlcs-%s.m3u8", buffertime) > 1){
				TM3U *m3u = m3uNew();
				if (m3uOpen(m3u, buffer, M3U_OPENWRITE)){
					ret = m3uWritePlaylist(m3u, plc, vp->tagc);
					m3uClose(m3u);
				}
				m3uFree(m3u);
			}
		}
	}
	return ret;
}

#if !RELEASEBUILD
static void saveSnapshot (wchar_t *filename)
{
	lSaveImage(frame, filename, IMG_PNG, 0, 0);
}

static int getKeyPress ()
{
	int ch = 1;
	if (kbhit())
		ch = getch();

	if (ch == 27 || ch == 13)	// escape
		return 0;
	else
		return ch;
}

// for testing the various libvlc swscale modes
int swmode = 0;


// debugging tools
int processKeyPress (TVLCPLAYER *vp, int ch)
{
	static int cstate = 0;
	PLAYLISTCACHE *plcD = getDisplayPlaylist(vp);
	// PLAYLISTCACHE *plcQ = getQueuedPlaylist(vp);
	
	if (ch == 224){		// an arrow key was pressed
		cstate = 1;
		return 1;
	}

	if (cstate == 1){
		cstate = 0;
		
		if (!plcD){
			return 1;

		}else if (ch == 115 || ch == 75){				// left arrow, with or without control
			TPLAYLIST2 *pl = getPagePtr(vp, PAGE_PLAYLIST2);
			player_prevTrack(vp, vp->vlc);
			playlist2SetStartTrack(pl, plcD, plcD->pr->playingItem);
			
		}else if (ch == 116 || ch == 77){		// right arrow, with or without control
			TPLAYLIST2 *pl = getPagePtr(vp, PAGE_PLAYLIST2);
			player_nextTrack(vp, vp->vlc);
			playlist2SetStartTrack(pl, plcD, plcD->pr->playingItem);
			
		}else if (ch == 141 || ch == 72){		// up arrow, with or without control
			TPLAYLIST2 *pl = getPagePtr(vp, PAGE_PLAYLIST2);
			
			if (--plcD->pr->selectedItem < 0)
				plcD->pr->selectedItem = playlistGetTotal(plcD)-1;

			playlist2SetStartTrack(pl, plcD, plcD->pr->selectedItem);
			playlistMetaGetMeta(vp, pl->metacb, plcD, plcD->pr->selectedItem, plcD->pr->selectedItem-10);
			
		}else if (ch == 145 || ch == 80){		// down arrow, with or without control
			TPLAYLIST2 *pl = getPagePtr(vp, PAGE_PLAYLIST2);
			 if (++plcD->pr->selectedItem >= playlistGetTotal(plcD))
				plcD->pr->selectedItem = 0;

			playlist2SetStartTrack(pl, plcD, plcD->pr->selectedItem);
			playlistMetaGetMeta(vp, pl->metacb, plcD, plcD->pr->selectedItem, plcD->pr->selectedItem+10);
		}
		return 1;
	}

	if (ch == 'p'){	
		saveSnapshot(L"snapshot.png");			
		
	}else if (ch == 'l'){
		if (++swmode > 10) swmode = 0;
		printf("swmode set to %i\n", swmode);

	}else if (ch == '#'){
		PostMessage(vp->gui.hMsgWin, WM_HOTKEY, 0, 'L'<<16);
		
	}else if (ch == 'a'){

		unsigned int hash;
		int trk = 0;
		char buffer[64];
		int total = playlistGetTotal(plcD);
		int trklen = 0;
		
		while(trk < total && !kbhit()){
			hash = playlistGetHash(plcD, trk);
			if (hash){
				tagRetrieveByHash(vp->tagc, hash, MTAG_LENGTH, buffer, sizeof(buffer));
				if (*buffer)
					trklen = (int)stringToTime(buffer, sizeof(buffer));

				if (trklen <= 0){
					printf("playing track %i\n", trk+1);
					dbprintf(vp, "playing track %i", trk+1);
					
					plcD->pr->playingItem = trk;
					startPlaylistTrack(vp, plcD, trk);
					
					for (int i = 0; i < 500; i += 10){
						lSleep(10);
						tagRetrieveByHash(vp->tagc, hash, MTAG_LENGTH, buffer, sizeof(buffer));
						if (*buffer){
							trklen = (int)stringToTime(buffer, sizeof(buffer));
							if (trklen > 0) break;
						}
					}
					if (trklen > 0)
						printf("       time %i\n", trklen);
						dbprintf(vp, "       time %is", trklen);
				}
			}
			trklen = 0;
			trk++;
		}
		player_stop(vp, vp->vlc);
		//playlist2SetStartTrack(pl, plcD, --trk);

	}else if (ch == 'm'){
		wchar_t buffer[MAX_PATH];
		PLAYLISTCACHE *plc = getDisplayPlaylist(vp);
	  	writePlaylist(vp, plc, buffer, sizeof(buffer));
		
	}else if (ch == 'e'){	
		TPLAYLIST2 *pl = getPagePtr(vp, PAGE_PLAYLIST2);
		pl->voffset -= 10 ;
		plcD->pr->dragLocation.d1.y = -20;
		pl->decayTop = 10.0;
		
	}else if (ch == 'd'){	
		TPLAYLIST2 *pl = getPagePtr(vp, PAGE_PLAYLIST2);
		pl->voffset += 10;
		plcD->pr->dragLocation.d1.y = 20;
		pl->decayTop = 10.0;
				
	}else if (ch == 'n'){		
		TPLAYLIST2 *pl = getPagePtr(vp, PAGE_PLAYLIST2);
		playlistMetaGetMeta(vp, pl->metacb, plcD, 0, playlistGetTotal(plcD)-1);

	}else if (ch == 'j'){
		TPLAYLIST2 *pl = getPagePtr(vp, PAGE_PLAYLIST2);
		countItems(vp, pl->metacb);
		
	}else if (ch == 'r'){
		if (renderLock(vp)){
			if (getApplState(vp))
				imageCacheReloadImages(vp);
			renderUnlock(vp);
		}
	}else if (ch == ','){	// previous file in playlist
		int trk = player_prevTrack(vp, vp->vlc);
		trackLoadEvent(vp, plcD, getPagePtr(vp, PAGE_PLAYLIST2), trk);

	}else if (ch == '.'){	// next file in playlist
		int trk = player_nextTrack(vp, vp->vlc);
		trackLoadEvent(vp, plcD, getPagePtr(vp, PAGE_PLAYLIST2), trk);
	}
	
	return 1;
}
#endif

static void drawButtonsOvr (TVLCPLAYER *vp, TBUTTONS *buttons, TVIDEOOVERLAY *playctrl, TFRAME *frame)
{
	TBUTTON *tpt = buttonGet(vp, PAGE_OVERLAY, VBUTTON_TRACKPOSTIP);
	const TBUTTON *tp = buttonGet(vp, PAGE_OVERLAY, VBUTTON_TRACKPOS);
	const TBUTTON *bvol = buttonGet(vp, PAGE_OVERLAY, VBUTTON_VOLUME);
	
	if (vp->vlc->volume > 0){ // is not muted
		const float vol = (bvol->image->height-4.0)/100.0;
		drawVolume(bvol->activeBtn, frame, bvol->pos.x, bvol->pos.y, \
	  		0, (100.0-vp->vlc->volume)*vol, bvol->image->width, bvol->image->height, 0.75);
	}

	/*
	TCHAPTER *chapt = getPagePtr(vp, PAGE_CHAPTERS);
	if (!chapt->ttitles){
		buttonDisable(vp, PAGE_OVERLAY, VBUTTON_CHAPTERS);
		//buttonEnable(vp, PAGE_OVERLAY, VBUTTON_PLAYLIST);
	}else{
		buttonEnable(vp, PAGE_OVERLAY, VBUTTON_CHAPTERS);
		//buttonDisable(vp, PAGE_OVERLAY, VBUTTON_PLAYLIST);
	}*/
			
	buttonDisable(vp, PAGE_OVERLAY, VBUTTON_VOLUME);
	buttonDisable(vp, PAGE_OVERLAY, VBUTTON_TRACKPOSTIP);
	buttonsRender(buttons->list, buttons->total, frame);
	buttonEnable(vp, PAGE_OVERLAY, VBUTTON_VOLUME);
	buttonEnable(vp, PAGE_OVERLAY, VBUTTON_TRACKPOSTIP);

	tpt->pos.x = tp->pos.x +8+ ((float)(tp->image->width-16) * vp->vlc->position)
	  - (tpt->image->width/2.0);
	lDrawImage(tpt->activeBtn, frame, tpt->pos.x, tpt->pos.y);
}

static float getFPS (TVLCPLAYER *vp)
{
	return 1.0/(((vp->dTime[0]+vp->dTime[1]+vp->dTime[2]+vp->dTime[3]+
				  vp->dTime[4]+vp->dTime[5]+vp->dTime[6]+vp->dTime[7])/
			8.0)/1000.0);
}

static void drawFPSOverlay (TVLCPLAYER *vp, TFRAME *frame, float fps, int x, int y)
{
	lSetCharacterEncoding(vp->hw, ASCII_PAGE);
	lSetForegroundColour(vp->hw, 0xFFFFFFFF);
	outlineTextEnable(vp->hw, 0xFF000000);
	lPrintf(frame, x, y, BFONT, LPRT_CPY, "%.1f", fps);
	outlineTextDisable(vp->hw);	
}

static void drawTimeStamp (TVLCPLAYER *vp, TFRAME *frame, const float position)
{
	TFRAME *str;
	char buffer[2][64];
	TBUTTON *ts = buttonGet(vp, PAGE_OVERLAY, VBUTTON_TIMESTAMP);
	
	lSetCharacterEncoding(vp->hw, ASCII_PAGE);
	lSetBackgroundColour(vp->hw, 0x00000000);
	lSetForegroundColour(vp->hw, 0xFFFFFFFF);
	
	timeToString(position*(float)vp->vlc->length, buffer[0], sizeof(buffer[0]));
	timeToString(vp->vlc->length, buffer[1], sizeof(buffer[1]));
	if (*buffer[0] && *buffer[1])
		str = lNewString(vp->hw, LFRM_BPP_32A, 0, BFONT, "%s/%s", buffer[0], buffer[1]);
	else
		str = lNewString(vp->hw, LFRM_BPP_32A, 0, BFONT, "0:00/0:00");

	const int dx = (frame->width - str->width)/2;
	const int dy = ts->pos.y + ((ts->image->height - str->height)/2) - 1;
	lDrawImage(str, frame, dx, dy);
	lDeleteFrame(str);
}

void drawButtonPanel (TVLCPLAYER *vp, TFRAME *frame)
{
	const int panelHeight = MCTRLVIDEOBLURHEIGHT;
	huhtanenBlur(frame, 0, frame->height-panelHeight, frame->width-1, frame->height-1, 3);
	lDrawLine(frame, 0, frame->height-panelHeight-2, frame->width-1, frame->height-panelHeight-2, 200<<24 | 0x000000);
	lDrawLine(frame, 0, frame->height-panelHeight-1, frame->width-1, frame->height-panelHeight-1, 150<<24 | 0xFFFFFF);
}

void marqueeDelete (TMARQUEE *marquee)
{
	if (marquee){
		WaitForSingleObject(marquee->hLock, INFINITE);
		CloseHandle(marquee->hLock);
		my_free(marquee->entry);
		my_free(marquee);
	}	
}

TMARQUEE *marqueeNew (const int tLines, const unsigned int flags)
{
	TMARQUEE *marquee = my_calloc(1, sizeof(TMARQUEE));
	if (marquee){
		marquee->entry = (TMARQUEELINE*)my_calloc(tLines, sizeof(TMARQUEELINE));
		if (marquee->entry){
			marquee->total = tLines;
			marquee->hLock = CreateMutex(NULL, FALSE, NULL);
			marquee->flags = flags;
		}else{
			my_free(marquee);
			marquee = NULL;
		}
	}
	return marquee;
}

void marqueeAdd (TVLCPLAYER *vp, TMARQUEE *mq, const char *str, const unsigned int timeout)
{
	if (mq && WaitForSingleObject(mq->hLock, INFINITE) == WAIT_OBJECT_0){
		my_memcpy(&mq->entry[0], &mq->entry[1], sizeof(TMARQUEELINE) * (mq->total-1));
		
		strncpy(mq->entry[mq->total-1].line, str, MAX_PATH_UTF8);
		mq->entry[mq->total-1].time = timeout;
		mq->ready = 1;
		ReleaseMutex(mq->hLock);
	}
}

int marqueeDraw (TVLCPLAYER *vp, TFRAME *frame, TMARQUEE *mq)
{
	if (!mq || !mq->ready) return 0;

	int x = 2;
	int y = 2;
	const int flags = PF_CLIPDRAW | PF_DONTFORMATBUFFER;
	
	if (WaitForSingleObject(mq->hLock, INFINITE) == WAIT_OBJECT_0){
		lSetCharacterEncoding(vp->hw, CMT_UTF8);
		lSetForegroundColour(frame->hw, 0xFFFFFFFF);

		for (int i = 0; i < mq->total; i++){
			if (mq->entry[i].time > vp->fTime){
				if (mq->flags&MARQUEE_CENTER){
					printSingleLineShadow(frame, BFONT, x, y, 0xFFFFFFFF, mq->entry[i].line);
				}else{
					TFRAME *str = lNewString(frame->hw, frame->bpp, flags, BFONT, mq->entry[i].line);
					if (str){
						lDrawLine(frame, x-1, y, x-1, y+str->height-1, 0xFF000000);
						lDrawImage(str, frame, x, y);
						lDeleteFrame(str);
					}
				}
				y += 16;
			}
		}

		mq->ready = (y > 2);
		ReleaseMutex(mq->hLock);
	}
	return (y > 2);
}

int drawOverlay (TVLCPLAYER *vp, TFRAME *frame, TVIDEOOVERLAY *playctrl)
{
	TBUTTONS *buttons = buttonsGetContainer(vp, PAGE_OVERLAY);
	
	const int isMarQuDrawn = marqueeDraw(vp, frame, playctrl->marquee);
	if (isMarQuDrawn){
		buttonDisable(vp, PAGE_OVERLAY, VBUTTON_SCLOCK);
		buttonDisable(vp, PAGE_OVERLAY, VBUTTON_TIMESTAMP);
	}
	
	drawButtonPanel(vp, frame);
	drawButtonsOvr(vp, buttons, playctrl, frame);
		
	if (!isMarQuDrawn){
		drawTimeStamp(vp, frame, vp->vlc->position);
	}else{
		buttonEnable(vp, PAGE_OVERLAY, VBUTTON_SCLOCK);
		buttonEnable(vp, PAGE_OVERLAY, VBUTTON_TIMESTAMP);
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

/*###########################################################################*/
/*###########################################################################*/
/*###########################################################################*/
void (CALLBACK updateTimerCB)(UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2)
{
	TVLCPLAYER *vp = (TVLCPLAYER*)dwUser;
	if (getApplState(vp))
		SetEvent(vp->hUpdateEvent);
}

static void stopRefreshTicker (TVLCPLAYER *vp)
{
	if (vp->updateTimer){
		timeKillEvent(vp->updateTimer);
		vp->updateTimer = 0;
	}
}

void startRefreshTicker (TVLCPLAYER *vp, const float fps)
{
	if (vp->updateTimer) stopRefreshTicker(vp);
	vp->updateTimer = (int)timeSetEvent((1.0/fps)*1000, 20, updateTimerCB, (DWORD_PTR)vp, TIME_PERIODIC);
}
/*###########################################################################*/
/*###########################################################################*/
/*###########################################################################*/


void resetOverlay (TVLCPLAYER *vp)
{
	if (getApplState(vp)){
		if (getPage(vp) == PAGE_OVERLAY && !kHookGetState() && getPageSec(vp) != PAGE_CHAPTERS){
			setPage(vp, PAGE_NONE);
		}
			
		//if (getPageSec(vp) == PAGE_CHAPTERS)
      		//setPageSec(vp, -1);
	}
}

void enableOverlay (TVLCPLAYER *vp)
{
	if (getPage(vp) != PAGE_OVERLAY)
		setPage(vp, PAGE_OVERLAY);
	timerReset(vp, TIMER_OVERLAYRESET);
	timerSet(vp, TIMER_OVERLAYRESET, MENUOVERLAYPERIOD);
	SetEvent(vp->hUpdateEvent);
}

static inline void _imageBestFit (const int bg_w, const int bg_h, int fg_w, int fg_h, int *w, int *h)
{
	const int fg_sar_num = 1; const int fg_sar_den = 1;
	const int bg_sar_den = 1; const int bg_sar_num = 1;

	if (fg_w < 1 || fg_w > 4095) fg_w = bg_w;
	if (fg_h < 1 || fg_h > 4095) fg_h = bg_h;
	*w = bg_w;
	*h = (bg_w * fg_h * fg_sar_den * bg_sar_num) / (float)(fg_w * fg_sar_num * bg_sar_den);
	if (*h > bg_h){
		*w = (bg_h * fg_w * fg_sar_num * bg_sar_den) / (float)(fg_h * fg_sar_den * bg_sar_num);
		*h = bg_h;
	}
}


static void copyVideo (TVLCCONFIG *vlc, TFRAME *src, TFRAME *des, int ratio)
{
	
	#define CW(a) ((float)(dh)*(a))
	#define CH(a) ((float)(dw)/(a))
	#define CX(a) (((dw)-CW((a)))/2.0)
	#define CY(a) (((dh)-CH((a)))/2.0)

	const int dw = des->width;
	const int dh = des->height;


//printf("%f %f\n", CX(1.5), CW(1.5));

	switch (ratio){
	  case	CFGBUTTON_AR_AUTO:{
		int w, h;
		_imageBestFit(dw, dh, vlc->videoWidth, vlc->videoHeight, &w, &h);
		if (w < dw-2 || h < dh-2){
			int x = 0, y = 0;
			if (w < dw-2) x = (dw-w)/2.0;
			if (h < dh-2) y = (dh-h)/2.0;
			memset(des->pixels, 0, des->frameSize);
			copyImage(src, des, dw, dh, x, y, w, h);
			break;
		}else{// else - use CFGBUTTON_AR_177 for usbd480(16:9) or CFGBUTTON_AR_133 for g19(4:3)
#if G19DISPLAY
			my_memcpy((uint32_t*)des->pixels, (uint32_t*)src->pixels, des->frameSize);
			break;
#endif
		}
	  }
	  
	  case	CFGBUTTON_AR_177:	// fastest copy method
#if !G19DISPLAY
		//usbd480 is a 16:9 (480x272) display so no resize required
	  	my_memcpy((uint32_t*)des->pixels, (uint32_t*)src->pixels, des->frameSize);
#else
		memset(des->pixels, 0, des->frameSize);
	  	copyImage(src, des, dw, dh, 0, CY(1.777), dw, CH(1.777));
#endif
	  	break;
	  case	CFGBUTTON_AR_125:
	  	memset(des->pixels, 0, des->frameSize);
		copyImage(src, des, dw, dh, CX(1.25), 0, CW(1.25), dh);
		break;
	  case	CFGBUTTON_AR_133:
#if !G19DISPLAY
	  	memset(des->pixels, 0, des->frameSize);
		copyImage(src, des, dw, dh, CX(1.333), 0, CW(1.333), dh);
#else
		my_memcpy((uint32_t*)des->pixels, (uint32_t*)src->pixels, des->frameSize);
#endif
		break;
	  case	CFGBUTTON_AR_122:
	  	memset(des->pixels, 0, des->frameSize);
		copyImage(src, des, dw, dh, CX(1.222), 0, CW(1.222), dh);
		break;
	  case	CFGBUTTON_AR_15:
	  	
	  	memset(des->pixels, 0, des->frameSize);
#if !G19DISPLAY
		copyImage(src, des, dw, dh, CX(1.5), 0, CW(1.5), dh);
#else
		copyImage(src, des, dw, dh, 0, CY(1.5), dw, CH(1.5));
#endif
		break;
	  case	CFGBUTTON_AR_16:
	  	memset(des->pixels, 0, des->frameSize);
#if !G19DISPLAY
		copyImage(src, des, dw, dh, CX(1.6), 0, CW(1.6), dh);
#else
		copyImage(src, des, dw, dh, 0, CY(1.6), dw, CH(1.6));
#endif
		break;
	  case	CFGBUTTON_AR_167:
	  	memset(des->pixels, 0, des->frameSize);
#if !G19DISPLAY
		copyImage(src, des, dw, dh, CX(1.666), 0, CW(1.666), dh);
#else
		copyImage(src, des, dw, dh, 0, CY(1.666), dw, CH(1.666));
#endif
		break;
	  case	CFGBUTTON_AR_185:
	  	memset(des->pixels, 0, des->frameSize);
		copyImage(src, des, dw, dh, 0, CY(1.85), dw, CH(1.85));
		break;
	  case	CFGBUTTON_AR_220:
	  	memset(des->pixels, 0, des->frameSize);
		copyImage(src, des, dw, dh, 0, CY(2.20), dw, CH(2.20));
		break;
	  case	CFGBUTTON_AR_235:
	  	memset(des->pixels, 0, des->frameSize);
		copyImage(src, des, dw, dh, 0, CY(2.35), dw, CH(2.35));
		break;
	  case	CFGBUTTON_AR_240:
	  	memset(des->pixels, 0, des->frameSize);
		copyImage(src, des, dw, dh, 0, CY(2.40), dw, CH(2.40));
		break;
	  case	CFGBUTTON_AR_155:
		lCopyAreaScaled(src, frame, 0, (dh/100.0)*7.2, dw, (dh/100.0)*85.8, 0, 0, dw, dh, LCASS_CPY);
		break;
	};
}


// return 1 if media is a local file or 0 if dshow:// or screen://
int isLocalMedia (const char *utf8path)
{
	if (!strstr(utf8path, "://"))
		return 1;
	else if (strstr(utf8path, "screen://"))
		return 0;
	else if (strstr(utf8path, "dshow://"))
		return 0;
	else if (strstr(utf8path, "file://"))
		return -1;
	else
		return 0;
}

int loadSubtitles (TVLCPLAYER *vp, TVLCCONFIG *vlc)
{
	vlc->spu.total = vlc_getSubtitleCount(vlc);
	vlc->spu.desc = vlc_getSubtitleDescriptions(vlc);
	vlc->spu.selected = -1;
	return vlc->spu.total;
}

void getChapterDetails (TVLCPLAYER *vp)
{
	TCHAPTER *chapt = getPagePtr(vp, PAGE_CHAPTERS);
	
	chapt->ttitles = vlc_getTitleCount(vp->vlc);
	if (chapt->ttitles < 0) chapt->ttitles = 0;
	if (chapt->ttitles > 1)
		buttonEnable(vp, PAGE_OVERLAY, VBUTTON_CHAPTERS);
	else
		buttonDisable(vp, PAGE_OVERLAY, VBUTTON_CHAPTERS);

	chapt->title = vlc_getTitle(vp->vlc);
	chapt->ctitle = chapt->title+1;

	chapt->tchapters = vlc_getTitleChapterCount(vp->vlc, chapt->ctitle-1);
	if (chapt->tchapters > 100) chapt->tchapters = 0;
	else if (chapt->tchapters > 32) chapt->tchapters = 32;

	chapt->cchapter = vlc_getChapter(vp->vlc);
	if (chapt->cchapter > 31) chapt->cchapter = 31;
}

static void _getNewTrackVar (TVLCPLAYER *vp)
{
	vlc_getVideoSize(vp->vlc, &vp->vlc->videoWidth, &vp->vlc->videoHeight);
	vlc_mediaParseAsync(vp->vlc);
	loadSubtitles(vp, vp->vlc);
	vp->vlc->spu.selected = vlc_getSubtitle(vp->vlc);
	getChapterDetails(vp);
}

static void getNewTrackVar (TVLCPLAYER *vp)
{
	if (loadLock(vp)){
		_getNewTrackVar(vp);
		loadUnlock(vp);
	}
}

void getNewTrackVariables (TVLCPLAYER *vp) 
{
	if (getApplState(vp) && getPlayState(vp))
		getNewTrackVar(vp);
}

void initNextTrack (TVLCPLAYER *vp)
{
	if (getApplState(vp)){
		player_stop(vp, vp->vlc);
		
		if (getPlaybackMode(vp) == VISUALS_ENABLED){	// play next audio track
			PLAYLISTCACHE *plcQ = getQueuedPlaylist(vp);
			
			if (plcQ && plcQ->pr->playingItem < playlistGetTotal(plcQ)-1){
				const int trk = player_nextTrack(vp, vp->vlc);
				trackLoadEvent(vp, plcQ, getPagePtr(vp, PAGE_PLAYLIST2), trk);
			}
		}
	}
}

void vlc_eventsCallback (const libvlc_event_t *event, void *udata)
{
	if (event == NULL || udata == NULL)
		return;
	// a few sanity checks
	TVLCPLAYER *vp = (TVLCPLAYER*)udata;
	if (!getApplState(vp)) return;
	TVLCCONFIG *vlc = getConfig(vp);
	if (event->p_obj != vlc->m && event->p_obj != vlc->mp)
		return;

	// don't set idle when video is playing
	// for audio, idle will not be set when visuals are enabled
	//printf("vp->currentFType %i\n", vp->currentFType);
	//if (vp->currentFType != 1)
		setAwake(vp);

#if 0
	if (event->type != libvlc_MediaPlayerPositionChanged) // don't spam the console
    	printf("event %i: '%s' (state:%i)\n", event->type, vlc_EventTypeToName(event->type), vlc->playState);
#endif

    switch (event->type){
	  case libvlc_MediaPlayerPositionChanged:
		vlc->position = event->u.media_player_position_changed.new_position;
		clipFloat(vlc->position);
      	break;

      case libvlc_MediaDurationChanged:{
		PLAYLISTCACHE *plcQ = getQueuedPlaylist(vp);
		if (!plcQ) break;
		
		const int pos = plcQ->pr->playingItem;
		vp->vlc->length = event->u.media_duration_changed.new_duration / 1000;

		char path[MAX_PATH_UTF8];
		playlistGetPath(plcQ, pos, path, sizeof(path));
		if (*path){
			//char length[32];
			//tagRetrieve(vp->tagc, path, MTAG_LENGTH, length, sizeof(length));
			if (/**length &&*/ vp->vlc->length){
				char buffer[32];
				timeToString(vp->vlc->length, buffer, sizeof(buffer));				
				if (*buffer)
					tagAdd(vp->tagc, path, MTAG_LENGTH, buffer, 1);
			}
		}
		if (getApplState(vp))
			timerSet(vp, TIMER_NEWTRACKVARS, 100);
      	break;
	  }
      case libvlc_MediaStateChanged:
      	if (vlc_getState(vlc) == libvlc_Ended){
      	    vlc->playEnded = 1;
      		vlc->playState = 8;
			vlc->position = 0.0;
		}
      	break;

      case libvlc_MediaPlayerPlaying:
      	buttonDisable(vp, PAGE_OVERLAY, VBUTTON_PLAY);
      	buttonEnable(vp, PAGE_OVERLAY, VBUTTON_PAUSE);
      	buttonDisable(vp, PAGE_PLOVERLAY, PLOBUTTON_PLAY);
      	buttonEnable(vp, PAGE_PLOVERLAY, PLOBUTTON_PAUSE);
      	vlc->playState = 1;
      	
      	// use this to retrieve video size as calling via this (vlc) callback seems to generate a dead lock
      	if (getApplState(vp))
			timerSet(vp, TIMER_NEWTRACKVARS, 500);

      	break;

      case libvlc_MediaPlayerPaused:
      	buttonEnable(vp, PAGE_OVERLAY, VBUTTON_PLAY);
      	buttonDisable(vp, PAGE_OVERLAY, VBUTTON_PAUSE);
      	buttonEnable(vp, PAGE_PLOVERLAY, PLOBUTTON_PLAY);
      	buttonDisable(vp, PAGE_PLOVERLAY, PLOBUTTON_PAUSE);
      	vlc->playState = 2;
		break;
		
      case libvlc_MediaPlayerStopped:
      	buttonEnable(vp, PAGE_OVERLAY, VBUTTON_PLAY);
      	buttonDisable(vp, PAGE_OVERLAY, VBUTTON_PAUSE);
      	buttonEnable(vp, PAGE_PLOVERLAY, PLOBUTTON_PLAY);
      	buttonDisable(vp, PAGE_PLOVERLAY, PLOBUTTON_PAUSE);
      	//getChapterDetails(vp);
      	
      	TCHAPTER *chapt = getPagePtr(vp, PAGE_CHAPTERS);
      	chapt->ttitles = 0;

      	if (getPageSec(vp) == PAGE_CHAPTERS)
      		setPageSec(vp, -1);
      	break;

	  case libvlc_MediaPlayerEncounteredError:{
	  	PLAYLISTCACHE *plcQ = getQueuedPlaylist(vp);
	  	if (!plcQ) break;
	  	
	  	char path[MAX_PATH_UTF8];
		playlistGetPath(plcQ, plcQ->pr->playingItem, path, sizeof(path));
	  	dbprintf(vp, "\nerror while playing %i:'%s'\n\n", plcQ->pr->playingItem+1, path);

		setPageSec(vp, -1);
		vlc->playState = 8;
		// faulted on this track. try playing next
	  }
      case libvlc_MediaPlayerEndReached:	// upon error move on to next track, don't hang around
       	if (vlc->playState == 8)
      		timerSet(vp, TIMER_MCTRLNEXTTRACK, 0);

      	vlc->playEnded = 1;
      	vlc->playState = 8;
		vlc->position = 0.0;
		break;

	  case libvlc_MediaPlayerTitleChanged:
		//getChapterDetails(vp);
		if (getApplState(vp)){
			timerSet(vp, TIMER_NEWTRACKVARS, 100);
      	}
      		
		break;
	
	  case libvlc_MediaParsedChanged:{
			char buffer[MAX_PATH_UTF8];
			PLAYLISTCACHE *plcQ = getQueuedPlaylist(vp);
			if (!plcQ) break;

			playlistGetTitle(plcQ, plcQ->pr->playingItem, buffer, sizeof(buffer));
			if (!*buffer)
				playlistGetPath(plcQ, plcQ->pr->playingItem, buffer, sizeof(buffer));
				
			if (*buffer){
				TVIDEOOVERLAY *pctrl = getPagePtr(vp, PAGE_OVERLAY);
				marqueeAdd(vp, pctrl->marquee, buffer, getTime(vp)+5000);
			}
		}
	    break;
	  default:
		return;
	}
}

void detachEvents (TVLCCONFIG *vlc, TVLCPLAYER *vp)
{
	if (vlc->emp)
		for (int i = 0; i < sizeof(mp_events)/sizeof(*mp_events); i++)
			vlc_eventPlayerDetach(vlc, mp_events[i], vlc_eventsCallback, vp);
	vlc->emp = NULL;
	
	if (vlc->em)
		for (int i = 0; i < sizeof(m_events)/sizeof(*m_events); i++)
			vlc_eventMediaDetach(vlc, m_events[i], vlc_eventsCallback, vp);
	vlc->em = NULL;
}

static void attachEvents (TVLCCONFIG *vlc, TVLCPLAYER *vp)
{
	vlc_getMediaPlayerEventManager(vlc);
	for (int i = 0; i < sizeof(mp_events)/sizeof(*mp_events); i++)
		vlc_eventPlayerAttach(vlc, mp_events[i], vlc_eventsCallback, vp);

	vlc_getMediaEventManager(vlc);
	for (int i = 0; i < sizeof(m_events)/sizeof(*m_events); i++)
		vlc_eventMediaAttach(vlc, m_events[i], vlc_eventsCallback, vp);
}

void unloadMedia (TVLCPLAYER *vp, TVLCCONFIG *vlc)
{
	detachEvents(vlc, vp);
	vlc_release(vlc);
	vlc->isMediaLoaded = 0;
}

int loadMedia (TVLCPLAYER *vp, TVLCCONFIG *vlc, const char *mediaPath)
{
	vlc->m = vlc_new_path(vlc, mediaPath);
	if (vlc->m){
		vlc_configure(vp, vlc, vlc->width, vlc->height, vlc->bpp, vp->gui.visuals);
		vlc->mp = vlc_newFromMedia(vlc);
		if (vlc->mp){
			vlc_setVideoCallbacks(vlc, vmem_lock, vmem_unlock, vmem_display, vp);
			vlc_setVideoFormat(vlc, VCHROMA, vlc->width, vlc->height, VPITCH(vlc->width));
			attachEvents(vlc, vp);
			return 1;
		}
		vlc_mediaRelease(vlc);
	}
	dbprintf(vp, "libvlc could not open '%s'\n",mediaPath);
	return 0;
}

void closeVLCInstance (TVLCPLAYER *vp, TVLCCONFIG *vlc)
{
	if (vlc->openCount){
		vlc->openCount = 0;
		unloadMedia(vp, vlc);
    	vlc_releaseLib(vlc);
    }
}

void vlc_configure (TVLCPLAYER *vp, TVLCCONFIG *vlc, const int width, const int height, const int bpp, const int visMode)
{	
	//printf("vlc_configure %i\n", swmode);
	
	char buffer[64];

	//vlc_addOption(vlc, "-v");
	//vlc_addOption(vlc, "--verbose=3");

	vlc_addOption(vlc, "quiet");
	vlc_addOption(vlc, "quiet-synchro");

	//vlc_addOption(vlc, "http-album-art");
	vlc_addOption(vlc, "album-art=2");
	//vlc_addOption(vlc, "album-art-filename=cover.jpg");

	vlc_addOption(vlc, "spu");
	vlc_addOption(vlc, "sub-fps=25");
	vlc_addOption(vlc, "sub-type=auto");
	vlc_addOption(vlc, "sub-language=english");
	vlc_addOption(vlc, "sub-autodetect-file");
	//vlc_addOption(vlc, "sub-autodetect-fuzzy=1");
	vlc_addOption(vlc, "sub-autodetect-path=.\\subs");

	//vlc_addOption(vlc, "no-audio");
	//vlc_addOption(vlc, "no-video");

	//snprintf(buffer, 128, "swscale-mode=%i", swmode);
	//printf("%s\n", buffer);
	//vlc_addOption(vlc, buffer);

	vlc_addOption(vlc, "screen-fps=10.0");
	vlc_addOption(vlc, "no-overlay");
	vlc_addOption(vlc, "no-video-title-show");
	vlc_addOption(vlc, "ignore-config");
	vlc_addOption(vlc, "plugin-path=plugins");
	vlc_addOption(vlc, "no-stats");
	vlc_addOption(vlc, "no-media-library");

	//vlc_addOption(vlc, "screen-top=10");
	//vlc_addOption(vlc, "screen-left=10");
	//vlc_addOption(vlc, "screen-width=480");
	//vlc_addOption(vlc, "screen-height=272");
	//vlc_addOption(vlc, "screen-follow-mouse");
	//vlc_addOption(vlc, "screen-fragment-size=16");

#if !G19DISPLAY
	vlc_addOption(vlc, "aspect-ratio=1.77");
	vlc_addOption(vlc, "custom-aspect-ratio=1.77");
	//vlc_addOption(vlc, "monitor-par=4:3");
#else
	vlc_addOption(vlc, "aspect-ratio=1.333");
	vlc_addOption(vlc, "custom-aspect-ratio=1.333");
#endif

	if (getPlaybackMode(vp) == VIS_DISABLED/* || getPlaybackMode(vp) == VIS_DISABLED_IDLE*/){
		vlc_addOption(vlc, "audio-filter=");
	}else{

		snprintf(buffer, sizeof(buffer), "effect-width=%i", vlc->width);
		vlc_addOption(vlc, buffer);
		snprintf(buffer, sizeof(buffer), "effect-height=%i", vlc->height);
		vlc_addOption(vlc, buffer);
		
		switch (visMode){
	  	  case VIS_VUMETER:
	  		vlc_addOption(vlc, "audio-filter=visual");
	  		vlc_addOption(vlc, "effect-list=vumeter");
			break;
			
		  case VIS_SMETER:
			vlc_addOption(vlc, "audio-filter=visual");
			vlc_addOption(vlc, "effect-list=spectrometer");
			vlc_addOption(vlc, "spect-radius=42");
			vlc_addOption(vlc, "spect-sections=3");
			vlc_addOption(vlc, "spect-separ=1");
			vlc_addOption(vlc, "spect-amp=8");
			vlc_addOption(vlc, "spect-peak-width=61");
			vlc_addOption(vlc, "spect-peak-height=1");
			break;
		
	  	  case VIS_PINEAPPLE:
			vlc_addOption(vlc, "audio-filter=visual");
			vlc_addOption(vlc, "effect-list=spectrometer");
			vlc_addOption(vlc, "spect-radius=100");
			vlc_addOption(vlc, "spect-sections=5");
			vlc_addOption(vlc, "spect-separ=0");
			vlc_addOption(vlc, "spect-amp=2");
			vlc_addOption(vlc, "spect-peak-width=12");
			vlc_addOption(vlc, "spect-peak-height=50");

			break;
	  	  case VIS_SPECTRUM:
		  	vlc_addOption(vlc, "audio-filter=visual");
	  		vlc_addOption(vlc, "effect-list=spectrum");
			break;
	  	  case VIS_SCOPE:
	  		vlc_addOption(vlc, "audio-filter=visual");
	  		vlc_addOption(vlc, "effect-list=scope");
			break;
	  	  case VIS_GOOM_Q3:
	  	  case VIS_GOOM_Q2:
	  	  case VIS_GOOM_Q1:
	  	  	vlc_addOption(vlc, "audio-filter=goom");
			snprintf(buffer, sizeof(buffer), "goom-width=%s", goomres[visMode-VIS_GOOM_Q3].x);
			vlc_addOption(vlc, buffer);
			snprintf(buffer, sizeof(buffer), "goom-height=%s", goomres[visMode-VIS_GOOM_Q3].y);
			vlc_addOption(vlc, buffer);
			break;

	  	default:
		  	break;
		}
	}

#if 0
	vlc_addOption(vlc, "dshow-chroma="DSCHROMA);
	
#else
//	setTunerCountry(vp, CHN_COUNTRY);
//	setTunerChannel(vp, vp->tuner.channelIdx);

//	sprintf(vp->tuner.cchannel, "dshow-tuner-channel=%d", vp->tuner.channel);
//	sprintf(vp->tuner.ccountry, "dshow-tuner-country=%d", vp->tuner.country);
//	vlc_addOption(vlc, vp->tuner.cchannel);
//	vlc_addOption(vlc, vp->tuner.ccountry);
	vlc_addOption(vlc, "dshow-caching="DSCACHING);
	vlc_addOption(vlc, "dshow-tuner-input="DSINPUT_TYPE);
	vlc_addOption(vlc, "dshow-chroma="DSCHROMA);
	vlc_addOption(vlc, "dshow-audio-samplerate="DSAUDIO_SAMRATE);
	vlc_addOption(vlc, "dshow-audio-bitspersample="DSAUDIO_BPS);
	vlc_addOption(vlc, "dshow-audio-channels="DSAUDIO_CHNS);
	vlc_addOption(vlc, "dshow-fps=25");
	//vlc_addOption(vlc, "audio-desync=100");
	vlc_addOption(vlc, "aout-rate=44100");


#ifdef DSVIDEO_SIZE
	vlc_addOption(vlc, "dshow-size="DSVIDEO_SIZE);
#endif
#ifdef DSDEVICE_VIDEO
	vlc_addOption(vlc, "dshow-vdev="DSDEVICE_VIDEO);
#endif
#ifdef DSDEVICE_AUDIO
	vlc_addOption(vlc, "dshow-adev="DSDEVICE_AUDIO);
#endif
#endif

}

static int createVideoBuffers (TVLCPLAYER *vp)
{
	// we read from this, don't write. read only upon vlc signals so
 	vp->ctx.vmem = lNewFrame(vp->hw, vp->vlc->width, vp->vlc->height, vp->vlc->bpp);
 	
 	// copy of the above with write permitted
 	vp->ctx.working = lNewFrame(vp->hw, vp->vlc->width, vp->vlc->height, vp->vlc->bpp);
   	vp->ctx.bufferSize = vp->ctx.working->frameSize;
   	vp->ctx.pixels = lGetPixelAddress(vp->ctx.vmem, 0, 0);
 	vp->ctx.pixelBuffer = lGetPixelAddress(vp->ctx.working, 0, 0);
   	return (vp->ctx.pixels && vp->ctx.pixelBuffer);
}

static void freeVideoBuffers (TVLCPLAYER *vp)
{
	lDeleteFrame(vp->ctx.vmem);
	lDeleteFrame(vp->ctx.working);
	vp->ctx.pixels = NULL;
	vp->ctx.pixelBuffer = NULL;
	vp->ctx.bufferSize = 0;
}

int createVLCInstance (TVLCCONFIG *vlc, int visMode)
{
 	vlc->hLib = NULL;
 	vlc->m = NULL;
 	vlc->mp = NULL;
 	vlc->emp = NULL;
 	vlc->em = NULL;

	vlc->hLib = vlc_init(vlc, 0, NULL);
	if (vlc->hLib) vlc->openCount++;
	
	vlc->visMode = visMode;
	vlc->swapColourBits = 0;
	vlc->volume = 50;
	vlc->position = 0.0;
	vlc->playState = 0;
	vlc->width = DWIDTH;
 	vlc->height = DHEIGHT;
 	vlc->bpp = DVIDBUFBPP;
 	vlc->spu.total = 0;
 	vlc->spu.desc = NULL;
 	vlc->spu.selected = -1;
	
	return (vlc->hLib != NULL);
}

static void resetButtonHighlight (TBUTTON *button)
{
	if (button)
		button->activeBtn = button->image;
}

void resetHighlight (TVLCPLAYER *vp)
{
	if (getApplState(vp)){
		resetButtonHighlight(vp->gui.btncb.button);
	}
}

void enableHighlight (TVLCPLAYER *vp, TBUTTON *button)
{
	if (getApplState(vp)){
		resetButtonHighlight(vp->gui.btncb.button);
		
		vp->gui.btncb.ptr = vp;
		vp->gui.btncb.button = button;
		button->activeBtn = button->highlight;
		timerReset(vp, TIMER_HIGHLIGHTRESET);
		timerSet(vp, TIMER_HIGHLIGHTRESET, HIGHLIGHTPERIOD);
	}
}

static char *manglePath (TVLCPLAYER *vp, const char *src)
{
	// if we don't do this subtitles won't be detected
	if (isLocalMedia(src) > 0 && getPlaybackMode(vp) == VISUALS_DISABLED){
		size_t len = strlen(src);
		char *tmp = my_calloc(1, len + 16);	// enough for source path + file:///
		char *path = my_calloc(8, len + 16);	// enough space for a uri encoded utf8 path
		if (!path || !tmp) return 0;
		sprintf(tmp, "file:///%s", src);

#if 1	/* we do this because vlc/src/text/strings.c:decode_URI() fucks up utf8 encoded paths */
		encodeURI(tmp, path, strlen(tmp));
#else
		strcpy(path, tmp);
#endif
		my_free(tmp);
		return path;
	}else{
		// dshow:// and screen:// pass straight through
		return my_strdup(src);
	}
}

static int _browserLoadMediaFile (TVLCPLAYER *vp, const char *utf8path)
{
	TVLCCONFIG *vlc = getConfig(vp);

	if (getPlayState(vp))
		player_stop(vp, vlc);

	if (vlc->isMediaLoaded)
		unloadMedia(vp, vlc);

	char *path = manglePath(vp, utf8path);
	if (path == NULL) return 0;

	vlc->isMediaLoaded = loadMedia(vp, vlc, path);
	if (vlc->isMediaLoaded){
		player_play(vp, vlc);

		vlc->isMediaLoaded = vlc_willPlay(vlc);
		if (vlc->isMediaLoaded){
			_getNewTrackVar(vp);
		}else{
			player_stop(vp, vlc);
		}
	}


	my_free(path);
	return vlc->isMediaLoaded;
}

int browserLoadMediaFile (TVLCPLAYER *vp, const char *utf8path)
{
	if (loadLock(vp)){
		int ret = _browserLoadMediaFile(vp, utf8path);
		loadUnlock(vp);
		return ret;
	}
	return 0;
}

void exitAppl (TVLCPLAYER *vp)
{
	//printf("exitAppl in\n");
  	if (getPlayState(vp))
		player_stop(vp, vp->vlc);
	vp->vlc->playState = 0;
	unloadMedia(vp, vp->vlc);
	vp->vlc->mp = NULL;
	setApplState(vp, 0);
	//printf("exitAppl out\n");
}

int setVolume (TVLCPLAYER *vp, int volume)
{
	TVLCCONFIG *vlc = getConfig(vp);
	if (getPlayState(vp) >  0){
		if (volume > 100)
			volume = 100;
		else if (volume <= 0)
			volume = 0;

		vlc_setVolume(vlc, volume);
		vlc->volume = vlc_getVolume(vlc);
		
		vlc_setMute(vlc, (vlc->volume <= 0));
		if (vlc_getMute(vlc))
			vlc->volume = -1;
	}
	return vlc->volume;
}

int obuttonCB (const TTOUCHCOORD *pos, TBUTTON *button, const int id, const int flags, void *ptr)
{
	TVLCPLAYER *vp = ptr;
		
	switch (id){
	  case VBUTTON_PLAY:

	  	if (getPlayState(vp) != 1){
			vp->queuedPlaylist = vp->displayPlaylist;
	  		PLAYLISTCACHE *plcQ = getQueuedPlaylist(vp);
			if (!plcQ) break;
			
			// load a media if not already
			if (!vp->vlc->m || !vp->vlc->mp){
				if (playlistGetTotal(plcQ)){
					if (plcQ->pr->selectedItem < 0) plcQ->pr->selectedItem = 0;
					startPlaylistTrack(vp, plcQ, plcQ->pr->selectedItem);
					return 0;
				}
			}
			player_play(vp, vp->vlc);
		}
		break;
		
	  case VBUTTON_PAUSE:
	  	trackPlayPause(vp);
		/*if (getPlayState(vp) == 1){
	  		vp->vlc->playState = 2;
			player_pause(vp, vp->vlc);

		}else if (getPlayState(vp) == 2){
			player_play(vp, vp->vlc);
		}*/
		break;
		
	  case VBUTTON_STOP:
		//player_stop(vp, vp->vlc);
		trackStop(vp);
		break;
		
	  case VBUTTON_PRETRACK:{
	  	/*
	  	int trk = 0;
	  	//if (!strnicmp("dshow://", vp->currentFile, 8))
	  	//	trk = tuner_prevChannel(vp);
	  	//else
	  		trk = player_prevTrack(vp, vp->vlc);
	  		
	  	PLAYLISTCACHE *plcQ = getQueuedPlaylist(vp);
	  	if (plcQ)
	  		trackLoadEvent(vp, plcQ, getPagePtr(vp, PAGE_PLAYLIST2), trk);
	  	*/
	  	trackPrev(vp);
	  	break;
	  }
	  case VBUTTON_NEXTTRACK:{
	  	/*int trk = 0;
	  	//if (!strnicmp("dshow://", vp->currentFile, 8))
	  	//	trk = tuner_nextChannel(vp);
	  	//else
	  		trk = player_nextTrack(vp, vp->vlc);
	  		
	  	PLAYLISTCACHE *plcQ = getQueuedPlaylist(vp);
	  	if (plcQ)
	  		trackLoadEvent(vp, plcQ, getPagePtr(vp, PAGE_PLAYLIST2), trk);
	  	*/
	  	trackNext(vp);
	  	break;
	  }
	  case VBUTTON_POSBACK:
		if (getPlayState(vp)){
			vp->vlc->position = vlc_getPosition(vp->vlc);
			vp->vlc->position -= 0.00400;
			clipFloat(vp->vlc->position);
			vlc_setPosition(vp->vlc, vp->vlc->position);
		}
		break;
		
	  case VBUTTON_POSFORWARD:
		if (getPlayState(vp)){
			vp->vlc->position = vlc_getPosition(vp->vlc);
			vp->vlc->position += 0.00400;
			clipFloat(vp->vlc->position);
			vlc_setPosition(vp->vlc, vp->vlc->position);
		}
		break;
		
	  case VBUTTON_EXIT:
  		setPageSec(vp, PAGE_EXIT);
	  	break;

	  case VBUTTON_SCLOCK:
	  	//if (getPageSec(vp) == PAGE_CHAPTERS)
      	//	setPageSec(vp, -1);
	  	setPageSec(vp, PAGE_CLOCK);
	  	break;

	  case VBUTTON_SMETA:{
	  	PLAYLISTCACHE *plc = NULL;
	  	
	  	if (getPage(vp) == PAGE_OVERLAY)
	  		plc = getQueuedPlaylist(vp);

	  	if (!plc)
	  		plc = getDisplayPlaylist(vp);
	  	
		TMETA *meta = getPagePtr(vp, PAGE_META);
		if (plc->pr->playingItem >= 0)
			meta->trackPosition = plc->pr->playingItem;
		else
			meta->trackPosition = 0;

	  	metaOpenPageAndReturn(vp);
	   }
	  	break;
	  		  	
	  case VBUTTON_TIMESTAMP:
	  	//if (getPageSec(vp) == PAGE_CHAPTERS || getPageSec(vp) == PAGE_CLOCK)
      		setPageSec(vp, -1);
	  	setPage(vp, PAGE_CFG);
	  	break;
	  	
	  case VBUTTON_CHAPTERS:
	  	if (getPageSec(vp) != PAGE_CHAPTERS){
	  		//buttonDisable(vp, PAGE_OVERLAY, VBUTTON_PLAYLIST);
	  		setPageSec(vp, PAGE_CHAPTERS);
	  	}else{
	  		//buttonEnable(vp, PAGE_OVERLAY, VBUTTON_PLAYLIST);
	  		setPageSec(vp, -1);
	  	}
	  	break;
	  	
	  case VBUTTON_PLAYLIST:
	  	if (getPageSec(vp) == PAGE_CHAPTERS)
      		setPageSec(vp, -1);
		setPage(vp, PAGE_PLAYLISTPLM);
		break;

	  case VBUTTON_OPEN:
	  	//if (getPageSec(vp) == PAGE_CHAPTERS)
      		setPageSec(vp, -1);
	  	setPage(vp, PAGE_BROWSER);
		break;

	  case VBUTTON_TRACKPOSTIP:
	  	return 1;
	  	
	  case VBUTTON_TRACKPOS:
	  	if (getPlayState(vp) && getPlayState(vp) != 8){
	  		float w = buttonGet(vp, PAGE_OVERLAY, VBUTTON_TRACKPOS)->image->width-8.0;
			vp->vlc->position = (1.0/w) * (pos->x-8);
			clipFloat(vp->vlc->position);
			vlc_setPosition(vp->vlc, vp->vlc->position);
			enableOverlay(vp);
		}
		break;

	  case VBUTTON_VOLUMEBASE:
	  case VBUTTON_VOLUME:{
		#define BORDERTHICKNESS 10

		//calculate the volume based icon and touch position		
		int volume = 100 - ((100.0/((float)buttonGet(vp, PAGE_OVERLAY, VBUTTON_VOLUME)->image->height-10))
						* (float)(pos->y-BORDERTHICKNESS));
		setVolume(vp, volume);
		enableOverlay(vp);
	  }
		break;
	}
	return 0;
}

static void initCS (TVLCPLAYER *vp)
{
	vp->hUpdateEvent = CreateEvent(NULL, 0, 0, NULL);
	vp->ctx.hEvent = CreateEvent(NULL, 0, 0, NULL);
	vp->ctx.hVLock = CreateMutex(NULL, FALSE, NULL);
}

static void deleteCS (TVLCPLAYER *vp)
{
	CloseHandle(vp->hUpdateEvent);
	CloseHandle(vp->ctx.hEvent);
	CloseHandle(vp->ctx.hVLock);
}

int openDefault (TVLCPLAYER *vp, TFRAME *frame, void *usr_ptr)
{	
	vp->gui.hRenderLock = CreateMutex(NULL, FALSE, NULL);
	vp->gui.hLoadLock = CreateMutex(NULL, FALSE, NULL);
	vp->gui.marquee = marqueeNew(16, MARQUEE_LEFT);
	
	createPixConvertTables();
	createVideoBuffers(vp);
	initCS(vp);
	setPlaybackMode(vp, 1);
	return 1;
}

int closeDefault (TVLCPLAYER *vp, TFRAME *frame, void *usr_ptr)
{
	deleteCS(vp);
	freeVideoBuffers(vp);
	marqueeDelete(vp->gui.marquee);
	
	CloseHandle(vp->gui.hRenderLock);
	vp->gui.hRenderLock = NULL;
	CloseHandle(vp->gui.hLoadLock);
	vp->gui.hLoadLock = NULL;
	return 1;
}

int closeOverlay (TVLCPLAYER *vp, TFRAME *frame, TVIDEOOVERLAY *playctrl)
{
	marqueeDelete(playctrl->marquee);
	buttonsDeleteContainer(vp, PAGE_OVERLAY);
	if (playctrl->stamp){
		lDeleteFrame(playctrl->stamp);
		playctrl->stamp = NULL;
	}
	return 1;
}

int touchDefault (TTOUCHCOORD *pos, int flags, TVLCPLAYER *vp)
{
	enableOverlay(vp);
	return 1;
}

static int drawUnderlayMeta (TVLCPLAYER *vp, TFRAME *frame)
{
	if (getPage(vp) == PAGE_NONE && getPageSec(vp) != PAGE_META && getPageSec(vp) != PAGE_CLOCK){
		TVIDEOOVERLAY *playctrl = getPagePtr(vp, PAGE_OVERLAY);
		if (!playctrl->marquee->ready){
			TMETA *meta = getPagePtr(vp, PAGE_META);
			PLAYLISTCACHE *plc = getQueuedPlaylist(vp);
			if (!plc)
	  			plc = getDisplayPlaylist(vp);
	  		if (plc){
	  			meta->trackPosition = plc->pr->playingItem;
				renderMeta(vp, frame, plc, meta, 0);
			}
		}
	}
	return 1;
}


int defaultRender (TVLCPLAYER *vp, TFRAME *frame, void *unused)
{
	if (getPageSec(vp) != PAGE_META){
		TVIDEOOVERLAY *playctrl = getPagePtr(vp, PAGE_OVERLAY);
		marqueeDraw(vp, frame, playctrl->marquee);
	}
	return 1;
	
}
/*
void initTuner (TVLCPLAYER *vp)
{
	vp->tuner.channels = channels;
	vp->tuner.channelIdx = 0;
	vp->tuner.channel = CHN_1;
	vp->tuner.country = CHN_COUNTRY;
}*/

int openOverlay (TVLCPLAYER *vp, TFRAME *des, TVIDEOOVERLAY *playctrl)
{
#if G19DISPLAY
	int x = 0;
	const int y = 175;
	const int x_gap = 64;
#else
	int x = 37;
	const int y = des->height-66;
	const int x_gap = 67;
#endif

	buttonsCreateContainer(vp, PAGE_OVERLAY, VBUTTON_TOTAL);
	
	TBUTTON *button = buttonGet(vp, PAGE_OVERLAY, VBUTTON_SCLOCK);
	button->enabled = 1;
	button->acceptDrag = 0;
	button->canAnimate = 2;
	button->pos.y = 0;
	button->pos.x = 0;
	button->callback = obuttonCB;
	button->id = VBUTTON_SCLOCK;
	buttonSetImages(vp, button, L"clock/sclk.png", NULL);

	button = buttonGet(vp, PAGE_OVERLAY, VBUTTON_TIMESTAMP);
	button->enabled = 1;
	button->acceptDrag = 0;
	button->canAnimate = 0;
	button->callback = obuttonCB;
	button->id = VBUTTON_TIMESTAMP;
	buttonSetImages(vp, button, L"timestamp.png", NULL);
	button->pos.y = 4;
	button->pos.x = abs((button->image->width - des->width)/2.0);

	button = buttonGet(vp, PAGE_OVERLAY, VBUTTON_PRETRACK);
	button->pos.x = x;
	button->pos.y = y;
	button->enabled = 1;
	button->acceptDrag = 0;
	button->canAnimate = 2;
	button->callback = obuttonCB;
	button->id = VBUTTON_PRETRACK;
	buttonSetImages(vp, button, L"media/skip_backward.png", NULL);
	
	button = buttonGet(vp, PAGE_OVERLAY, VBUTTON_PLAY);
	button->pos.x = (x+=x_gap);
	button->pos.y = y;
	button->enabled = 1;
	button->acceptDrag = 0;
	button->canAnimate = 0;
	button->callback = obuttonCB;
	button->id = VBUTTON_PLAY;
	buttonSetImages(vp, button, L"media/play.png", NULL);

	button = buttonGet(vp, PAGE_OVERLAY, VBUTTON_PAUSE);
	button->pos.x = x;
	button->pos.y = y;
	button->enabled = 0;
	button->acceptDrag = 0;
	button->canAnimate = 0;
	button->callback = obuttonCB;
	button->id = VBUTTON_PAUSE;
	buttonSetImages(vp, button, L"media/pause.png", NULL);

	button = buttonGet(vp, PAGE_OVERLAY, VBUTTON_STOP);
	button->pos.x = (x+=x_gap);
	button->pos.y = y;
	button->enabled = 1;
	button->acceptDrag = 0;
	button->canAnimate = 2;
	button->callback = obuttonCB;
	button->id = VBUTTON_STOP;
	buttonSetImages(vp, button, L"media/stop.png", NULL);

	button = buttonGet(vp, PAGE_OVERLAY, VBUTTON_NEXTTRACK);
	button->pos.x = (x+=x_gap);
	button->pos.y = y;
	button->enabled = 1;
	button->acceptDrag = 0;
	button->canAnimate = 2;
	button->callback = obuttonCB;
	button->id = VBUTTON_NEXTTRACK;
	buttonSetImages(vp, button, L"media/skip_forward.png", NULL);
	
	button = buttonGet(vp, PAGE_OVERLAY, VBUTTON_OPEN);
	button->pos.x = (x+=x_gap);
	button->pos.y = y;
	button->enabled = 1;
	button->acceptDrag = 0;
	button->canAnimate = 0;
	button->callback = obuttonCB;
	button->id = VBUTTON_OPEN;
	buttonSetImages(vp, button, L"media/open.png", NULL);
	
	button = buttonGet(vp, PAGE_OVERLAY, VBUTTON_PLAYLIST);
	button->enabled = 1;
	button->acceptDrag = 0;
	button->canAnimate = 0;
	button->callback = obuttonCB;
	button->id = VBUTTON_PLAYLIST;
	buttonSetImages(vp, button, L"media/playlist1.png", NULL);
	button->pos.x = 6;
	button->pos.y = (des->height - button->image->height)/2;

#if !G19DISPLAY
	button = buttonGet(vp, PAGE_OVERLAY, VBUTTON_EXIT);
	button->enabled = 1;
	button->acceptDrag = 1;	// accept drag so we can receive the pen up message
	button->canAnimate = 2;
	button->callback = obuttonCB;
	button->id = VBUTTON_EXIT;
	buttonSetImages(vp, button, L"media/exit.png", NULL);
	button->pos.x = (x+=x_gap);
	button->pos.y = y;
#else
	button = buttonGet(vp, PAGE_OVERLAY, VBUTTON_SMETA);
	button->enabled = 1;
	button->acceptDrag = 1;
	button->canAnimate = 2;
	button->callback = obuttonCB;
	button->id = VBUTTON_SMETA;
	buttonSetImages(vp, button, L"media/metasm.png", NULL);
	button->pos.x = des->width - button->image->width;
	button->pos.y = 0;
#endif

	button = buttonGet(vp, PAGE_OVERLAY, VBUTTON_TRACKPOS);
	button->enabled = 1;
	button->acceptDrag = 1;
	button->callback = obuttonCB;
	button->id = VBUTTON_TRACKPOS;
	buttonSetImages(vp, button, L"media/trackpos.png", NULL);
#if !G19DISPLAY
	button->pos.x = abs(des->width - button->image->width)/2.0;
	button->pos.y = 44;
#else
	button->pos.x = 5;
	button->pos.y = 44;
#endif

#if !G19DISPLAY
	const int posx2 = button->image->width + button->pos.x;
#endif
	const int posx1 = button->pos.x;
	const int posy = button->pos.y;

#if !G19DISPLAY
	button = buttonGet(vp, PAGE_OVERLAY, VBUTTON_SMETA);
	button->enabled = 1;
	button->acceptDrag = 0;
	button->canAnimate = 2;
	button->callback = obuttonCB;
	button->id = VBUTTON_SMETA;
	buttonSetImages(vp, button, L"media/metasm.png", NULL);
	button->pos.x = des->width - button->image->width - 2;
	button->pos.y = 0;
#else
	button = buttonGet(vp, PAGE_OVERLAY, VBUTTON_EXIT);
	button->enabled = 1;
	button->acceptDrag = 0;
	button->canAnimate = 2;
	button->callback = obuttonCB;
	button->id = VBUTTON_EXIT;
	buttonSetImages(vp, button, L"media/exitsm.png", NULL);
	button->pos.x = ((des->width - button->image->width)/2.0);
	button->pos.y = posy + 20;
#endif
	
	button = buttonGet(vp, PAGE_OVERLAY, VBUTTON_POSFORWARD);
	button->enabled = 1;
	button->acceptDrag = 0;
	button->canAnimate = 2;
	button->callback = obuttonCB;
	button->id = VBUTTON_POSFORWARD;
	buttonSetImages(vp, button, L"media/fast_forward.png", NULL);
#if !G19DISPLAY
	button->pos.x = posx2+6;
	button->pos.y = posy-6;
#else
	button->pos.x = ((des->width - button->image->width)/2) + (button->image->width)+16;
	button->pos.y = posy + 20;
#endif
	
	button = buttonGet(vp, PAGE_OVERLAY, VBUTTON_POSBACK);
	button->enabled = 1;
	button->acceptDrag = 0;
	button->canAnimate = 2;
	button->callback = obuttonCB;
	button->id = VBUTTON_POSBACK;
	buttonSetImages(vp, button, L"media/rewind.png", NULL);
#if !G19DISPLAY
	button->pos.x = posx1-button->image->width-6;
	button->pos.y = posy-6;
#else
	button->pos.x = ((des->width - button->image->width)/2.0) - (button->image->width)-16;
	button->pos.y = posy + 20;
#endif

	button = buttonGet(vp, PAGE_OVERLAY, VBUTTON_TRACKPOSTIP);
	button->pos.x = posx1;
	button->pos.y = posy-4;
	button->enabled = 1;
	button->acceptDrag = 1;
	button->callback = obuttonCB;
	button->id = VBUTTON_TRACKPOSTIP;
	buttonSetImages(vp, button, L"media/tracktip.png", NULL);

	button = buttonGet(vp, PAGE_OVERLAY, VBUTTON_VOLUME);
	button->enabled = 1;
	button->acceptDrag = 1;
	button->callback = obuttonCB;
	button->id = VBUTTON_VOLUME;
	buttonSetImages(vp, button, L"volume.png", NULL);
	button->pos.x = abs(des->width - button->image->width) - 10;
	button->pos.y = abs(des->height - button->image->height)/2.0;
	const int volx = button->pos.x;
	
	button = buttonGet(vp, PAGE_OVERLAY, VBUTTON_CHAPTERS);
	button->enabled = 0;
	button->acceptDrag = 0;
	button->canAnimate = 0;
	button->callback = obuttonCB;
	button->id = VBUTTON_CHAPTERS;
	buttonSetImages(vp, button, L"media/playlist2.png", NULL);
	button->pos.x = volx - button->image->width - 6;
	button->pos.y = (des->height - button->image->height)/2;

	button = buttonGet(vp, PAGE_OVERLAY, VBUTTON_VOLUMEBASE);
	button->enabled = 1;
	button->acceptDrag = 0;
	button->callback = obuttonCB;
	button->id = VBUTTON_VOLUMEBASE;
	buttonSetImages(vp, button, L"volumebase.png", NULL);
	button->pos.x = abs(des->width - button->image->width) - 10;
	button->pos.y = abs(des->height - button->image->height)/2.0;
	
	imageCacheAddImage(vp, L"cursor.png", LFRM_BPP_32A, &vp->gui.image[IMGC_POINTER]);
	imageCacheAddImage(vp, L"bgimage.png", SKINFILEBPP, &vp->gui.image[IMGC_BGIMAGE]);
	playctrl->marquee = marqueeNew(12, MARQUEE_CENTER);
	return 1;
}

int touchOverlay (TTOUCHCOORD *pos, int flags, TVLCPLAYER *vp)
{
	static unsigned int lastId = 0;
	
	enableOverlay(vp);
	TBUTTONS *buttons = buttonsGetContainer(vp, PAGE_OVERLAY);
	TBUTTON *button = buttons->list; 

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
					if (button->highlight){	// if button has a contact image then enable it
						enableOverlay(vp);
						enableHighlight(vp, button);
					}
					if (button->canAnimate)
						button->ani.state = 1;

					TTOUCHCOORD bpos;
					my_memcpy(&bpos, pos, sizeof(TTOUCHCOORD));
					bpos.x = pos->x - button->pos.x;
					bpos.y = pos->y - button->pos.y;
					button->callback(&bpos, button, button->id, flags, vp);
					return 0;
				}
			}
		}
	}
	return 0;
}

int acceptsDrag (TVLCPLAYER *vp, TTOUCHCOORD *pos, int flags)
{
	TBUTTON *button;
	int tbutton;
	const int page = getPage(vp);
	
	if (!flags && page != PAGE_PLAYLIST2)
		return 0;
	
	if (page == PAGE_OVERLAY || page == PAGE_BROWSER){
		button = buttonsGetContainer(vp, page)->list;
		tbutton = buttonsGetTotal(vp, page);
	}else if (page == PAGE_PLAYLIST2 || page == PAGE_PLAYLISTPLM){
		return 1;
	}else{
		return 0;
	}

	int i;
	for (i = 0; i < tbutton; i++, button++){
		if (button->enabled && button->acceptDrag){
			if (button->image){
				if (pos->x >= button->pos.x && pos->x <= button->pos.x+button->image->width){
					if (pos->y >= button->pos.y && pos->y <= button->pos.y+button->image->height){
						return 1;
					}
				}
			}
		}
	}
	return 0;
}

void touchIn (TTOUCHCOORD *pos, int flags, void *ptr)
{
	TVLCPLAYER *vp = (TVLCPLAYER*)ptr;
	if (!vp || !getApplState(vp)) return;
	
	TTOUCHINPOS *tin = my_malloc(sizeof(TTOUCHINPOS));
	if (tin){
		my_memcpy(&tin->pos, pos, sizeof(TTOUCHCOORD));
		tin->flags = flags;
		tin->ptr = ptr;
		
		PostMessage(vp->gui.hMsgWin, WM_TOUCHIN, (WPARAM)tin, (LPARAM)vp);
	}
}

int touchDispatch (TTOUCHCOORD *pos, int flags, TVLCPLAYER *vp)
{
	static unsigned int prevId = 0;	//	... must remove these statics...
	static unsigned int id = 0;
	
	if (!vp->gui.awake)
		startRefreshTicker(vp, vp->gui.targetFPS);
	setAwake(vp);

	flags &= 0xFFF;			// remove TOUCH_VINPUT and anything thats not directly touch related
	if (!flags)				// we could use the libmylcd supplied Id but that blocks us from using TOUCH_VINPUT
		pos->id = ++id;		// so we generate our own id
	else
		pos->id = id;

	// each touch down to touch up stream is given a unique id
	// we try to use this id to detect invalid, corrupt or out of order data
	if (!flags){		// pen down
		if (prevId >= pos->id)
			return 0;
		prevId = pos->id;
	}else if (prevId != pos->id){
		return 0;
	}
	
	if (!acceptsDrag(vp, pos, flags)){
		if (flags == 1 || pos->dt < vp->gui.calltime+80.0){
			vp->gui.calltime = 0;
			return -2;
		}else{
			vp->gui.calltime = 0;
		}
	}
	uint64_t t0_64, t_64;
	QueryPerformanceCounter((LARGE_INTEGER*)&t0_64);
	int ret = 1;

	TPAGE *page = vp->pages.page;
	int pageId = getPageSec(vp);
	for (int i = 0; i < PAGE_TOTAL && ret > 0; i++){
		if (pageId == page->pageId && page->isValid && page->in.isEnabled){
				ret = page->in.callback(pos, flags, page->in.ptr);
		}
		page++;
	}
		
	page = vp->pages.page;
	pageId = getPage(vp);
	for (int i = 0; i < PAGE_TOTAL && ret > 0; i++){
		if (pageId == page->pageId && page->isValid && page->in.isEnabled){
				ret = page->in.callback(pos, flags, page->in.ptr);
		}
		page++;
	}

	QueryPerformanceCounter((LARGE_INTEGER *)&t_64);
	vp->gui.calltime = (((t_64 - t0_64)*vp->resolution)*1000.0);
	
	/*if (vp->gui.visuals == VIS_DISABLED_IDLE)
		SetEvent(vp->hUpdateEvent);*/
	return ret;
}

TPAGE *registerPageSlot (TPAGES *pages, int id)
{
	TPAGE *page = pages->page;
	int i;
	for (i = 0; i < PAGE_TOTAL; i++){
		if (!page->isValid){
			page->isValid = 1;
			page->pageId = id;
			return page;
		}
		page++;
	}
	return NULL;
}

int registerPage (TVLCPLAYER *vp, const char *title, int id, void *ocb, void *ccb, void *rcb, TFRAME *frame, size_t rptrSize, void *icb, void *iptr)
{
	TPAGE *page = registerPageSlot(&vp->pages, id);
	if (page){
		page->title = my_strdup(title);
		if (!title){
			//printf("registerPage: failed for %i:'%s'\n", id, title);
			return 0;
		}

		if (rptrSize)
			page->ptr = my_calloc(1, rptrSize);
		else
			page->ptr = NULL;
		
		page->in.callback = icb;
		page->in.ptr = iptr;
		page->in.isValid = 1;
		page->in.isEnabled = 0;

		page->render.callback = rcb;
		page->render.frame = frame;
		page->render.ptr = page->ptr;
		page->render.isValid = 1;
		page->render.isEnabled = 0;
		page->render.isOpen = 0;

		page->open.callback = ocb;
		page->open.frame = frame;
		page->open.ptr = page->ptr;
		page->open.isValid = 1;
		page->open.isEnabled = 0;
		page->open.isOpen = 0;

		page->close.callback = ccb;
		page->close.frame = frame;
		page->close.ptr = page->ptr;
		page->close.isValid = 1;
		page->close.isEnabled = 0;
		page->close.isOpen = 0;		// not used
		return 1;
	}
	return 0;
}

static void openPages (TVLCPLAYER *vp)
{
	TPAGE *page = vp->pages.page;
	int i;
	for (i = 0; i < PAGE_TOTAL; page++, i++){
		if (page->isValid){
			if (page->open.isValid && !page->open.isOpen){
				//printf("opening page '%s'..", page->title);
				
				if (page->open.callback(vp, page->open.frame, page->open.ptr)){
					page->in.isEnabled = 1;
					page->render.isEnabled = 1;
					page->close.isEnabled = 1;
					page->open.isOpen = 1;
				//	printf(". ok\n");
				}else{
					page->in.isEnabled = 0;
					page->render.isEnabled = 0;
					page->close.isEnabled = 0;
					page->open.isOpen = 0;
				//	printf(". failed\n");
				}
			}
		}
	}
}

static void closePages (TVLCPLAYER *vp)
{
	renderLock(vp);
	
	// quickly disable all pages from rendering
	TPAGE *page = vp->pages.page;
	for (int i = 0; i < PAGE_TOTAL; page++, i++)
		page->render.isEnabled = 0;

	for (int i = PAGE_TOTAL-1; i >= 0; i--){
		page = &vp->pages.page[i];
		
		if (page->close.isValid && page->open.isOpen && /*page->close.isEnabled &&*/ page->isValid){
			//printf("closing page '%s'\n", page->title);
			
			page->close.callback((TVLCPLAYER*)vp, page->close.frame, page->close.ptr);
			page->close.isEnabled = 0;
			page->close.isValid = 0;
			page->isValid = 0;
			if (page->ptr)
				my_free(page->ptr);
			if (page->title)
				my_free(page->title);
			page->ptr = NULL;
		}
	}
}

void renderPage (TVLCPLAYER *vp, TFRAME *frame, int pageId)
{
	if (pageId == -1) return;

	TPAGE *page = vp->pages.page;
	for (int i = 0; i < PAGE_TOTAL; page++, i++){
		if (pageId == page->pageId && page->render.isEnabled && page->isValid){
			if (pageId == PAGE_NONE){ // is video display only with no overlay, so this does not require locking
				page->render.callback((TVLCPLAYER*)vp, page->render.frame, page->render.ptr);
			}else{
				//if (renderLock(vp)){
				//	if (getApplState(vp))
						page->render.callback((TVLCPLAYER*)vp, page->render.frame, page->render.ptr);
				//	renderUnlock(vp);
				//}
			}
		}
	}
}

static void renderEditbox (TVLCPLAYER *vp, TFRAME *frame, int x, int y)
{
	lSetForegroundColour(vp->hw, 0xFFFFFFFF);
	lSetCharacterEncoding(vp->hw, CMT_UTF16);
	
	if (vp->input.iOffset > vp->input.caretPos-1)
		vp->input.iOffset = vp->input.caretPos;
		
	addCaret(&vp->input, vp->input.workingBuffer, vp->input.caretBuffer, EDITBOXIN_INPUTBUFFERLEN-1);
	drawEditBox(&vp->input, frame, x, y, vp->input.caretBuffer, &vp->input.iOffset);

	lSetCharacterEncoding(vp->hw, CMT_UTF8);
	PLAYLISTCACHE *plcD = getDisplayPlaylist(vp);
	PLAYLISTCACHE *plcQ = getDisplayPlaylist(vp);

	drawInt(frame, frame->width-210, y-16, playlistGetTotal(plcD), 0);
	if (plcD)
		drawInt(frame, frame->width-150, y-16, plcD->pr->selectedItem+1, 0xFF0000);
	if (plcQ)
		drawInt(frame, frame->width-90, y-16, plcQ->pr->playingItem+1, 0xFF00FF);
	drawInt(frame, frame->width-30, y-16, wcslen(vp->input.workingBuffer), 0);
	
	const int plmtotal = playlistManagerGetTotal(vp->plm);
	if (plcD && plmtotal > 0){
		TFRAME *s = lNewString(frame->hw, frame->bpp, 0, BFONT, "%s [%i/%i (%i)]", plcD->name, vp->queuedPlaylist+1, vp->displayPlaylist+1, plmtotal);
		if (s){
			lSetBackgroundColour(vp->hw, 0x00);
			lSetForegroundColour(vp->hw, 0xFFF0F0F0);
			outlineTextEnable(vp->hw, 0xFF000000);
	
			x = (frame->width - s->width) - 4;
			if (x < 0) x = 0;
			y = (frame->height - s->height) - 4;
			lDrawImage(s, frame, x, y);
			lDeleteFrame(s);
			
			outlineTextDisable(vp->hw);		
		}
	}
}

static void composeFrame (TVLCPLAYER *vp, TFRAME *frame)
{
	if ((getPlaybackMode(vp) == VISUALS_ENABLED && vp->gui.visuals) || getPlaybackMode(vp) == VISUALS_DISABLED){
		copyVideo(vp->vlc, vp->ctx.working, frame, vp->gui.ratio);
		
		if (vp->vlc->swapColourBits){
			uint32_t *restrict pixels = (uint32_t*)lGetPixelAddress(frame, 0, 0);
			const int tp = frame->frameSize>>2;
			for (int i = 0; i < tp; i++)
				pixels[i] = ((pixels[i]&RGB_32_RED)>>16) | (pixels[i]&RGB_32_GREEN) | ((pixels[i]&RGB_32_BLUE)<<16);
		}
	}else{ /*if (gePage(vp) != PAGE_NONE)*/
		if (getIdle(vp)){
			memset(frame->pixels, 0x00, frame->frameSize);
		}else{
			TFRAME *base = imageCacheGetImage(vp, vp->gui.image[IMGC_BGIMAGE]);
			if (base){
				if (getPage(vp) == PAGE_NONE && getPageSec(vp) != PAGE_CLOCK && getPageSec(vp) != PAGE_META && !mHookGetState() && !kHookGetState()){
					if (vp->gui.fps < 2){
						my_memcpy((uint32_t*)frame->pixels, (uint32_t*)base->pixels, base->frameSize);
						drawUnderlayMeta(vp, frame);
					}
				}else{
					my_memcpy((uint32_t*)frame->pixels, (uint32_t*)base->pixels, base->frameSize);
					drawUnderlayMeta(vp, frame);
				}
			}
		}
	}

	renderPage(vp, frame, getPage(vp));
	renderPage(vp, frame, getPageSec(vp));
	marqueeDraw(vp, frame, vp->gui.marquee);
	
	if (kHookGetState() && getPageSec(vp) != PAGE_TETRIS)
#if !G19DISPLAY
		renderEditbox(vp, frame, 120, 84);
#else
		renderEditbox(vp, frame, 70, 70);
#endif
		
	if (vp->gui.drawFPS)
		drawFPSOverlay(vp, frame, getFPS(vp), frame->width-30, 2);

#if MOUSEHOOKCAP
	if (vp->gui.hooked){
		//if (hookGetState()){
			TFRAME *cur = imageCacheGetImage(vp, vp->gui.image[IMGC_POINTER]);
			lDrawImage(cur, frame, vp->gui.dx, vp->gui.dy);
		//}
	}
#endif
	
}

// this allows vlcstream to find its data files
// by settings current directory to path containing self (vlcstream.exe)
// without this drag'n'drop will screw with the path
void resetCurrentDirectory ()
{
	wchar_t drive[_MAX_DRIVE+1];
	wchar_t dir[_MAX_DIR+1];
	wchar_t szPath[MAX_PATH+1];
	GetModuleFileNameW(NULL, szPath, MAX_PATH);
	_wsplitpath(szPath, drive, dir, NULL, NULL);
	swprintf(szPath, L"%s%s", drive, dir);
	SetCurrentDirectoryW(szPath);
}

static void startTouchDispatcher (TVLCPLAYER *vp, const void *fn, const void *ptr)
{

#if !G19DISPLAY
	// init libmylcd touch handling
	lSetDisplayOption(vp->hw, vp->did, lOPT_USBD480_TOUCHCB, (intptr_t*)fn);
	lSetDisplayOption(vp->hw, vp->did, lOPT_USBD480_TOUCHCBUSERPTR, (intptr_t*)ptr);
#endif

#if MOUSEHOOKCAP
	startMouseCapture(vp);
#endif
}

static void closeTouchDispatcher (TVLCPLAYER *vp)
{
#if MOUSEHOOKCAP
	endMouseCapture(vp);
#endif

#if !G19DISPLAY
	lSetDisplayOption(vp->hw, vp->did, lOPT_USBD480_TOUCHCB, NULL);
#endif

}

static int initVLC (TVLCPLAYER *vp)
{
	return createVLCInstance(selectVLCConfig(vp), VIS_DISABLED);
}

int setDisplayBrightness (TVLCPLAYER *vp, int level)
{
#if !G19DISPLAY
	intptr_t value = (intptr_t)level;
	return lSetDisplayOption(vp->hw, vp->did, lOPT_USBD480_BRIGHTNESS, &value);
#else
	return -1;
#endif
}


static int configureDisplay (TVLCPLAYER *vp, THWD *hw)
{

	for (int i = 0; i < DISPLAYMAX && !vp->did; i++){	// find active display
		vp->did = lDriverNameToID(hw, device[i], LDRV_DISPLAY);
#if !RELEASEBUILD
		if (!vp->did && i < DISPLAYMAX-1)
			printf("Display '%s' unavilable, trying '%s'\n", device[i], device[i+1]);
		else if (!vp->did)
			printf("Display '%s' unavilable\n",device[i]);
#endif
		if (vp->did){
			if (!strcmp(device[i], "DDRAW"))
				vp->isVirtDisplay = 1;
		}
	}

	if (vp->did){
		vp->hw = hw;
		setDisplayBrightness(vp, 200);
		return vp->did;
	}else{
#if !RELEASEBUILD
		printf("device '%s', '%s' or '%s' not found\n", device[0], device[1], device[2]);
#endif
		return 0;
	}
}


// attempt to play whatever has been passed through the command line
int	startCommandlinePlayback (TVLCPLAYER *vp)
{
	int ret = 0;
	int argc = 0;
	wchar_t **argv = CommandLineToArgvW(GetCommandLineW(), &argc);

	PLAYLISTCACHE *plc = getPrimaryPlaylist(vp);
	TPLAYLISTMANAGER *plm = vp->plm;
	
	if (argc > 1){	// import a utf8 encoded playlist
		if (isPlaylistW(argv[1])){
			ret = importPlaylistW(plm, plc, vp->tagc, argv[1]);
			resetCurrentDirectory();
			int p1total = playlistGetTotal(plc);
			
			if (ret && !p1total){	// we have multiple playlists but the 1st is empty, so just delete it
				playlistManagerDeletePlaylist(plm, plc);
				vp->displayPlaylist = playlistManagerGetPlaylistNext(plm, -1);
				vp->queuedPlaylist = vp->displayPlaylist;
				plc = getPrimaryPlaylist(vp);
			}
			
			if (ret && p1total){
				int startTrack = 0;					// load track 'n-1' from playlist. -1 == play last track

				if (argc > 2){
					wchar_t *end = L"\0\0";
					int trk = wcstol(argv[2], &end, 0);
					
					if (trk > 0 && trk <= ret){		// set start track
						startTrack = trk-1;
						
					}else if (trk == -1){			// set last track as start
						startTrack = ret-1;
						
					}else if (!trk){				// perform a search for argv[2]
						char *out = convertto8(argv[2]);
						if (out){
							printf("searching for '%s'\n",out);
							startTrack = playlistSearch(plc, vp->tagc, out, 0);
							my_free(out);
						}
					}
				}
				
				const unsigned int hash = playlistGetHash(plc, startTrack);
				const int pos = setPlaylistPlayingItem(vp, plc, startTrack, hash);
				char path[MAX_PATH_UTF8];
				
				playlistGetPath(plc, pos, path, sizeof(path));
				if (*path){
					wchar_t *out = converttow(path);
					TFBROWSER *fb = getPagePtr(vp, PAGE_BROWSER);
					if (!isVideoFile(out)){	// filter type must be set before track is started
						setExtFilter(vp, fb, EXT_AUDIO);
						setPlaybackMode(vp, 1);
					}else{
						setExtFilter(vp, fb, EXT_VIDEO);
						setPlaybackMode(vp, 0);
					}
					my_free(out);
						
					if (pos >= 0)
						ret = startPlaylistTrack(vp, plc, pos);
				}
			}
			LocalFree(argv);
			return ret;
		}
	}else{
		if (argv) LocalFree(argv);
		return 0;
	}

	// try to play media file if passed, if not a file then assume a directory so import it
	char *out = convertto8(argv[1]);
	setPlaybackMode(vp, isVideoFile(argv[1]) == 0);
	TFBROWSER *fb = getPagePtr(vp, PAGE_BROWSER);

	wchar_t drive[_MAX_DRIVE+1];
	wchar_t dir[_MAX_DIR+1];
	wchar_t fname[_MAX_FNAME+1];
	wchar_t ext[_MAX_EXT+1];
	wchar_t buffer[MAX_PATH+1];

	int len;		
	int ilen = wcslen(argv[1]);
	const int isDir = isDirectoryW(argv[1]);
		
	if (isDir){
		if (argv[1][ilen-1] == L'\\')
			len = _snwprintf(buffer, MAX_PATH, L"%s", argv[1]);
		else
			len = _snwprintf(buffer, MAX_PATH, L"%s\\", argv[1]);
			_wsplitpath(buffer, drive, dir, fname, ext);
	}else{
		_wsplitpath(argv[1], drive, dir, fname, ext);
		len = _snwprintf(buffer, MAX_PATH, L"%s%s", drive, dir);
	}
		
	if (len){
		int trkStart = -1, pos = -1;
			
		if (!isVideoFile(argv[1]))	// filter type must be set before playlist is built
			setExtFilter(vp, fb, EXT_AUDIO);
		else
			setExtFilter(vp, fb, EXT_VIDEO);

		if (argv[1][ilen-1] == L'\"') argv[1][ilen-1] = L'\\';
			
		if (!stricmp(out, "screen://")){
			trkStart = playlistAdd(plc, out);
			playlistSetTitle(plc, trkStart, "Desktop", 0);
				
		}else if (!stricmp(out, "dshow://")){
			trkStart = playlistAdd(plc, out);
			playlistSetTitle(plc, trkStart, "DirectShow", 0);
				
		}else if (isDir){
			trkStart = browserImportPlaylistByDirW(fb, argv[1], plc, 1);
			if (trkStart == -1){
				if (isVideoFile(argv[1]))
					setExtFilter(vp, fb, EXT_AUDIO);
				else
					setExtFilter(vp, fb, EXT_VIDEO);
				trkStart = browserImportPlaylistByDirW(fb, argv[1], plc, 1);
			}
		}else{
			trkStart = browserImportPlaylistByDirW(fb, buffer, plc, 0);
		}

		if (!isDir){
			pos = setPlaylistPlayingItem(vp, plc, trkStart, getHash(out));

		}else if (playlistGetTotal(plc)){
			if (argc > 2){
				wchar_t *end = L"\0";
				pos = wcstol(argv[2], &end, 0)-1;
				if (pos >= 0)
					setPlaylistPlayingItem(vp, plc, pos, playlistGetHash(plc, pos));
			}else{
				pos = 0;
				playlistChangeEvent(vp, plc, getPagePtr(vp, PAGE_PLAYLIST2), pos);
			}
		}

		if (pos >= 0)
			ret = startPlaylistTrack(vp, plc, pos);
			
		if (ret){
			// set browser location to the current input path
			_snwprintf(dir, MAX_PATH, L"%s%s", fname, ext);
			ret = browserNavigateToDirectoryAndFile(vp, fb, buffer/*the dir*/, dir/*the file*/);
		}
		if (!ret)
			ret = browserNavigateToDirectory(vp, fb, buffer/*argv[1]*/);
	}
	my_free(out);

	if (argv)
		LocalFree(argv);
	return ret;
}

static void getVLCFrame (TVLCPLAYER *vp)
{
	my_memcpy((uint32_t*)vp->ctx.pixelBuffer, (uint32_t*)vp->ctx.pixels, vp->ctx.bufferSize);
}

/*void clearVideoBuffers (TVLCPLAYER *vp)
{
	lockVLCVideoBuffer(vp);
	lClearFrame(vp->ctx.vmem);
	lClearFrame(vp->ctx.working);
	lClearFrame(frame);
	unlockVLCVideoBuffer(vp);
}
*/
void DoVLCStream (TVLCPLAYER *vp)
{	
#if !RELEASEBUILD
	int ch;
#endif

#if ASYNCHRONOUSREFRESH
	MSG message;
#else
	const void *addr = lGetPixelAddress(frame, 0, 0);
#endif

	do{
		if (waitForUpdateSignal(vp)){
			if (isVLCFrameAvailable(vp)){
				if (waitForVLCUpdateSignal(vp)){
					if (getApplState(vp)){
						//lockVLCVideoBuffer(vp);
						getVLCFrame(vp);
						//unlockVLCVideoBuffer(vp);
					}
				}
			}

			const float t1 = getTime(vp);
			timerCheckAndFire(vp, t1);
			if (renderLock(vp)){
				vp->dTime[vp->gui.fps&0x07] = t1 - vp->fTime;
				vp->fTime = t1;
				composeFrame(vp, frame);
				if (vp->gui.fps++&0x08) vp->gui.fps = 0;
				renderUnlock(vp);
			}

#if !RELEASEBUILD
			if (!(ch=getKeyPress()))
				exitAppl(vp);
			else if (ch > 1)
				processKeyPress(vp, ch);
#endif


#if !ASYNCHRONOUSREFRESH
       		lUpdate(frame->hw, addr, frame->frameSize);
#else
       		lRefreshAsync(frame, 1);
       		
       		if (vp->isVirtDisplay && vp->gui.fps&0x03){
       			// ensure the ddraw window is responsive
       			// if we're using a virtual display (DDraw) along with lRefreshAsync()
       			// which offloads rendering to another thread, we need to handle window updates ourselves
       			// else the window becomes unusable
       			
   				while(PeekMessage(&message, NULL, 0, 0, PM_REMOVE)){
       				TranslateMessage(&message);
        			DispatchMessage(&message);
    			}
    		}
#endif

			// set idle period
			if (!vp->gui.idleDisabled && vp->gui.awake && (!getPlayState(vp) || getPlayState(vp) == 2 ||
				((/*vp->gui.visuals == VIS_DISABLED_IDLE ||*/ vp->gui.visuals == VIS_DISABLED) &&
				vp->currentFType == 1 && getPage(vp) == PAGE_OVERLAY) )){
				if ((uint32_t)((uint32_t)timeGetTime() - vp->gui.awakeTime) > IDLETIME){
					if (!kHookGetState()){	// don't idle if editbox is enabled
						setIdle(vp);
						startRefreshTicker(vp, UPDATERATE_IDLE);
					}
				}
			}
		}
	}while(getApplState(vp));
}
    
int main (const int argc, const char *argv[])
{ 

	timeBeginPeriod(3);
	SetPriorityClass(GetCurrentProcess(), ABOVE_NORMAL_PRIORITY_CLASS);
	resetCurrentDirectory();
	if (!initMYLCD_USBD480())
		exit(EXIT_FAILURE);

	TVLCPLAYER *vp = my_calloc(1, sizeof(TVLCPLAYER));
	if (!vp) exit(EXIT_FAILURE);

	QueryPerformanceCounter((LARGE_INTEGER*)&vp->tStart);
	QueryPerformanceFrequency((LARGE_INTEGER *)&vp->freq);
	vp->resolution = 1.0 / (float)vp->freq;
	srand((float)GetTickCount()/3.333);
	
	vp->imgc = imageCacheNew(140);	// 140 because we know at compile time how many images there are
	vp->tagc = tagNew();
	vp->plm = playlistManagerNew();
	if (!vp->imgc || !vp->tagc || !vp->plm){
		printf("we're going down!!\n");
		return 0;
	}

	PLAYLISTCACHE *plcP = playlistManagerCreatePlaylist(vp->plm, PLAYLIST_PRIMARY);
	vp->displayPlaylist = playlistManagerGetPlaylistIndex(vp->plm, plcP);

	configureDisplay(vp, hw);
	//initTuner(vp);
	initVLC(vp);
	editboxDoCmdRegistration(&vp->input, vp);
	registerPage(vp, "Video", PAGE_NONE, openDefault, closeDefault, defaultRender, frame, 0, touchDefault, vp);
	registerPage(vp, "Primary media control", PAGE_OVERLAY, openOverlay, closeOverlay, drawOverlay, frame, sizeof(TVIDEOOVERLAY), touchOverlay, vp);
	registerPage(vp, "Browser", PAGE_BROWSER, openBrowser, closeBrowser, drawBrowser, frame, sizeof(TFBROWSER), touchBrowser, vp);
	registerPage(vp, "Playlist", PAGE_PLAYLIST2, openPlaylist2, closePlaylist2, drawPlaylist2, frame, sizeof(TPLAYLIST2), touchPlaylist2, vp);
	registerPage(vp, "Secondary media control", PAGE_PLOVERLAY, openPlOvr, closePlOvr, drawPlOvr, frame, sizeof(TPLOVR), touchPlOvr, vp);
	registerPage(vp, "Clock", PAGE_CLOCK, openClk, closeClk, drawClk, frame, sizeof(TCLK), touchClk, vp);
	registerPage(vp, "Image", PAGE_IMGOVR, openIOvr, closeIOvr, drawIOvr, frame, sizeof(TIOVR), touchIOvr, vp);
	registerPage(vp, "Chapter", PAGE_CHAPTERS, openChapt, closeChapt, drawChapt, frame, sizeof(TCHAPTER), touchChapt, vp);
	registerPage(vp, "Exit ctrl", PAGE_EXIT, openExit, closeExit, drawExit, frame, sizeof(TEXIT), touchExit, vp);
	registerPage(vp, "Subtitle", PAGE_SUB, openSub, closeSub, drawSub, frame, sizeof(TSUB), touchSub, vp);
	registerPage(vp, "Meta", PAGE_META, openMeta, closeMeta, drawMeta, frame, sizeof(TMETA), touchMeta, vp);
	registerPage(vp, "Config", PAGE_CFG, openCfg, closeCfg, drawCfg, frame, sizeof(TCFG), touchCfg, vp);
	registerPage(vp, "Playlists", PAGE_PLAYLISTPLM, openPlaylistPlm, closePlaylistPlm, drawPlaylistPlm, frame, sizeof(TPLAYLISTPLM), touchPlaylistPlm, vp);
	registerPage(vp, "Lorenz", PAGE_LORENZ, openLorenz, closeLorenz, drawLorenz, frame, sizeof(TLORENZ), touchLorenz, vp);
	registerPage(vp, "Particles", PAGE_PARTICLES, openParticles, closeParticles, drawParticles, frame, sizeof(TPARTICLES), touchParticles, vp);
	registerPage(vp, "RC", PAGE_RC, openRC, closeRC, drawRC, frame, sizeof(TRC), touchRC, vp);
	registerPage(vp, "Tetris", PAGE_TETRIS, openTetris, closeTetris, drawTetris, frame, sizeof(TTETRIS), touchTetris, vp);
	
	timerInit(vp, TIMER_OVERLAYRESET, resetOverlay, NULL);
	timerInit(vp, TIMER_HIGHLIGHTRESET, resetHighlight, NULL);
	timerInit(vp, TIMER_NEWTRACKVARS, getNewTrackVariables, NULL);
	timerInit(vp, TIMER_MCTRLNEXTTRACK, initNextTrack, NULL);
	timerInit(vp, TIMER_PREVTRACK, trackPrev, NULL);
	timerInit(vp, TIMER_NEXTTRACK, trackNext , NULL);
	timerInit(vp, TIMER_PLAYPAUSE, trackPlayPause, NULL);
	timerInit(vp, TIMER_STOP, trackStop, NULL);
	timerInit(vp, TIMER_VOLUP, volumeUp, NULL);
	timerInit(vp, TIMER_VOLDN, volumeDown, NULL);


	setApplState(vp, 1);
	openPages(vp);
	startTouchDispatcher(vp, touchIn, vp);
		
	// try to play whatever has been passed via the command line
	// if unsuccessful then try to load the default playlist, as created at last exit
	int cret = startCommandlinePlayback(vp);
	if (!cret){
		TM3U *m3u = m3uNew();
		if (m3u){
			if (m3uOpen(m3u, VLCSPLAYLIST, M3U_OPENREAD)){
				cret = m3uReadPlaylist(m3u, vp->plm, plcP, vp->tagc);
				dbwprintf(vp, L"%i records read from '%s'", cret, VLCSPLAYLIST);
				m3uClose(m3u);
			}
			m3uFree(m3u);
			
			if (cret){		// tell the playlist renderer to update itself as we've changed something
				playlistSetName(plcP, VLCSPLAYLISTFILE);
				playlistChangeEvent(vp, plcP, getPagePtr(vp, PAGE_PLAYLIST2), 0);
			}
		}
	}

	if (!cret)
		setPage(vp, PAGE_BROWSER);
	else
		enableOverlay(vp);

	//setPage(vp, PAGE_PLAYLISTPLM);
	//setPageSec(vp, PAGE_META);

	setAwake(vp);
	startRefreshTicker(vp, vp->gui.targetFPS);

	// for when debugging
#if 0
	wchar_t *mrl = L"R:\\Music\\Misc";
	PLAYLISTCACHE *plcT = playlistManagerCreatePlaylist(vp->plm, "testing");
	browserImportPlaylistByDirW(getPagePtr(vp, PAGE_BROWSER), mrl, plcT, 1);
	
	dbwprintf(vp, L"%i tracks imported from '%s'", playlistGetTotal(plcT), mrl);
#endif

	DoVLCStream(vp);
	stopRefreshTicker(vp);
	if (kHookGetState())
		kHookUninstall();

	PLAYLISTCACHE *plc = getPrimaryPlaylist(vp);
	if (plc && playlistGetTotal(plc)){
		TM3U *m3u = m3uNew();
		if (m3u){
			if (m3uOpen(m3u, VLCSPLAYLIST, M3U_OPENWRITE)){
				printf("\n****** writing playlist *********\n");

				cret = m3uWritePlaylist(m3u, plc, vp->tagc);
				
				wprintf(L"%i records written to %s\n", cret, VLCSPLAYLIST);
				printf("*** writing playlist complete ***\n");
				m3uClose(m3u);
			}
			m3uFree(m3u);
		}
	}

	if (getPlayState(vp)){
		player_stop(vp, vp->vlc);
		unloadMedia(vp, vp->vlc);
	}

	timeEndPeriod(3);
	closeTouchDispatcher(vp);
	closePages(vp);
	lCloseDevice(hw, vp->did);	
	closeVLC(vp);
	imageCacheFree(vp->imgc);
	tagFree(vp->tagc);
	playlistManagerDelete(vp->plm);
	my_free(vp);
	closeMYLCD_USBD480();
    exit(EXIT_SUCCESS);
}

