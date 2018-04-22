
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

#ifndef _VLCSTREAM_H_
#define _VLCSTREAM_H_

#include <vlc/vlc.h>
#include <vlc/libvlc_media_list.h>


#define clipFloat(x)\
	if ((x) > 1.0) (x) = 1.0;\
	else if ((x) < 0.0) (x) = 0.0

static const libvlc_event_type_t m_events[] = {
#if 1
	libvlc_MediaStateChanged,
	libvlc_MediaDurationChanged,
	libvlc_MediaParsedChanged,
#else
    libvlc_MediaMetaChanged,
    libvlc_MediaSubItemAdded,
    libvlc_MediaFreed,
#endif
};
		
static const libvlc_event_type_t mp_events[] = {
#if 1
	libvlc_MediaPlayerNothingSpecial,
    //libvlc_MediaPlayerOpening,
    //libvlc_MediaPlayerBuffering,
    libvlc_MediaPlayerPlaying,
    libvlc_MediaPlayerPaused,
    libvlc_MediaPlayerStopped,
    libvlc_MediaPlayerForward,
    libvlc_MediaPlayerBackward,
    libvlc_MediaPlayerEndReached,
	libvlc_MediaPlayerEncounteredError,
	libvlc_MediaPlayerTitleChanged,
	libvlc_MediaPlayerPositionChanged,
	//libvlc_MediaPlayerSeekableChanged,
	//libvlc_MediaPlayerPausableChanged,

#else
    libvlc_MediaPlayerMediaChanged,
    libvlc_MediaPlayerNothingSpecial,
    libvlc_MediaPlayerOpening,
    libvlc_MediaPlayerBuffering,
    libvlc_MediaPlayerPlaying,
    libvlc_MediaPlayerPaused,
    libvlc_MediaPlayerStopped,
    libvlc_MediaPlayerForward,
    libvlc_MediaPlayerBackward,
    libvlc_MediaPlayerEndReached,
    libvlc_MediaPlayerEncounteredError,
    //libvlc_MediaPlayerTimeChanged,
    libvlc_MediaPlayerPositionChanged,
    libvlc_MediaPlayerSeekableChanged,
    libvlc_MediaPlayerPausableChanged,
    libvlc_MediaPlayerTitleChanged,
    libvlc_MediaPlayerSnapshotTaken,
    //libvlc_MediaPlayerLengthChanged,

#endif
};
 


#define VISUALS_DISABLED	0
#define VISUALS_ENABLED		1
#define MAXTINQUEUESIZE		256
#define TOUCH_VINPUT		0x1000


enum _IMGC
{
	IMGC_BG1,
	IMGC_BROWSERBASE,
	IMGC_CLKFACE,
	IMGC_CLKHR,
	IMGC_CLKMIN,
	IMGC_CLKSEC,
	IMGC_EXIT,
	IMGC_EXITBOX,
	IMGC_POINTER,
	IMGC_VLC,
	IMGC_NOART1,
	IMGC_NOART2,
	IMGC_BGIMAGE,
	IMGC_TOTAL
};

enum _SORT
{	
	SORT_NONE,
	SORT_NAMEA,
	SORT_NAMED,
	SORT_MODIFIEDDATEA,
	SORT_MODIFIEDDATED,
	SORT_CREATIONDATEA,
	SORT_CREATIONDATED,
	SORT_FILESIZEA,
	SORT_FILESIZED,
	SORT_TOTAL
};


typedef struct TPATHOBJECT TPATHOBJECT;
typedef struct TDATAOBJECT TDATAOBJECT;
typedef struct TFBROWSER TFBROWSER;
typedef struct TVLCPLAYER TVLCPLAYER;


typedef struct{
	TTOUCHCOORD pos;
	int flags;
	void *ptr;
}TTOUCHINPOS;


typedef struct{
	int enabled;
	TFRAME *img;	// scaled (may or may not include a border)
	int x;			// destination render location pt x
	int y;			// destination render location pt y
	int width;		// max width (n < display width)
	int height;		// max height (n < display height)
}TIOVR;

typedef struct{
	char *x;
	char *y;
}TGOOMRESSTR;

typedef struct{
	unsigned int playlistWriteTime;
	int playlistWriteState;
	int playlistWriteTotalWritten;
	wchar_t *playlistWriteFilename;
}TCFG;

typedef struct{
	int stub;
}TSUB;

typedef struct{
	int x;
	int y;
	int w;
	int h;
	int trackPosition;
	int pageReturn;		// return to this screen when closing meta page
	int pageSecReturn;
}TMETA;

typedef struct{
	int ratio;
	char *description;
}TRATIO;

typedef struct{
    uint32_t *pixels;
    uint32_t *pixelBuffer;
    int currentBuffer;
    size_t bufferSize;
    HANDLE hEvent;
    HANDLE hVLock;
    TFRAME *vmem;
    TFRAME *working;
}TCTX;

typedef struct{
	TBUTTON *button;
	void *ptr;
}TBUTTONHLCB;

#define MARQUEE_CENTER	0x01
#define MARQUEE_LEFT	0x02
//#define MARQUEE_TOTAL	12

typedef struct{
	char line[MAX_PATH_UTF8];	// text to display
	unsigned int time;			// display until this time is reached
}TMARQUEELINE;

typedef struct{
	//TMARQUEELINE entry[MARQUEE_TOTAL];
	TMARQUEELINE *entry;
	int total;
	HANDLE hLock;
	unsigned int flags;
	int ready;
}TMARQUEE;

typedef struct{
	TFRAME *stamp;
	TMARQUEE *marquee;		// for display track titles only
}TVIDEOOVERLAY;

typedef struct{
	libvlc_track_description_t *desc;
	int total;		// total subtitles in desc
	int selected;	// manually selected subtitle, -1 for none/disabled/auto
}TVLCSPU;

typedef struct{
	libvlc_instance_t *hLib;
	libvlc_media_t *m;
	libvlc_media_player_t *mp;
	libvlc_event_manager_t *emp; // media play[er/ing] events
	libvlc_event_manager_t *em;	// media events
	libvlc_time_t length;
	TVLCSPU	spu;
	int volume;
	int swapColourBits;	// swap r and b components
	float position;

	int playState;
	int playEnded;
	int isMediaLoaded;
	int openCount;
	int width;
	int height;
	int videoWidth;		// width of source video
	int videoHeight;	// height of source video
	int bpp;
	int visMode;		// vis_disabled, vis_goom, etc..
}TVLCCONFIG;

typedef struct{
	TFRAME *frame;
	unsigned int hash;
	float scale;
}TPLAYLISTART;

typedef struct{
	TVLCPLAYER *vp;
	TVLCCONFIG *vlc;
	PLAYLISTCACHE *plc;
	char *path;
	int position;
	int status;				// 0=can use, 1=in use, 2=ready to free
	unsigned int stime;				// start time
	unsigned int etime;				// end time
	int hash;
	unsigned int cb_MediaParsedChanged:1;
	unsigned int cb_MediaMetaChanged:1;
	unsigned int cb_pad:30;
}TMETACBSTRUCT;

typedef struct{
	TMETACBSTRUCT metalist[METALISTSIZE+1];
	unsigned int threadID;
	uintptr_t hThread;
	HANDLE hEvent;		// signal 
	HANDLE hLock;		// locks the queue
	int state;
	
	TVLCPLAYER *vp;
}TMETACB;

#define PL2ARTCACHESIZE	32

typedef struct{
	int x;
	int y;
	int voffset;
	int startPosition;
	float aHScaled;
	int tmp1;
	int dragState;
	int tmp2;
	float artScale;				// 0 = playlist artwork display is disabled, anything else sets the scale
	float tOn;
	float decayTop;

	TFRAME *lineRender;
	TPLAYLISTART *artcache[PL2ARTCACHESIZE];
	int cacheIdx;
	TMETACB	*metacb;
}TPLAYLIST2;

typedef struct{
	TLPOINTEX rect;
	TLPOINTEX *loc;
	int ttitles;	// total titles
	int tchapters;	// chapters in title
	int ctitle;		// currently viewing title
	int schapter;	// selected chapter
	int cchapter;	// currently playing chapter
	int title;		// currently playing title
}TCHAPTER;

typedef struct{
	TFRAME *face;
	TFRAME *hr;
	TFRAME *min;
	TFRAME *sec;
	TFRAME *back;
	int t0;
	T2POINT pos;
}TCLK;

typedef struct{
	int x;
	int y;
	int x2;
	int y2;
	int colour;
	TDATAOBJECT *obj;
	wchar_t text[MAX_PATH+1];
}TTLINE;

typedef struct{
	int x;
	int y;
	int width;
	int height;
	
	int font;
	int textHeight;
	int lineSpace;
	TTLINE *line;
	int lineTotal;
	TFRAME *frame;
}TTPRINT;

typedef struct{
	void (*Do) (TPATHOBJECT *path);
	void (*fn[SORT_TOTAL]) (TPATHOBJECT *path);
	int mode;
}TPATHSORT;

struct TFBROWSER {
	TPATHOBJECT *ppath;	// root path
	TPATHOBJECT *path;	// current path
	TTPRINT *print;
	TDATAOBJECT *selectedObj;	
	wchar_t selectedFile[MAX_PATH+1];
	TPATHSORT sort;
	wchar_t **extStrings;
	int extType;
};

typedef struct{
	T2POINT pos;
}TEXIT;

typedef struct{
	TBUTTON	*list;	// all buttons on page
	int total;
}TBUTTONS;

typedef struct{
	TBUTTONS *buttons[PAGE_TOTAL];
	int image[IMGC_TOTAL];	// imageCache handle Id's
	float calltime;	// time taken to handle last completed touch input event
	unsigned int awakeTime;	// time since track last ended
	int awake;
	int idleDisabled;
	TBUTTONHLCB btncb;

	int drawFPS;
	int visuals;		// which visual to use for audio tracks
	int ratio;
	int brightness;
	int targetFPS;		// try to maintain this fps
	int skin;			// selected skin
	int page_gl;
	unsigned int fps;	// current frames per second
	
	unsigned int winMsgThreadID;
	uintptr_t hWinMsgThread;
	HWND hMsgWin;
	int x;
	int y;
	int dx;
	int dy;
	int hooked;
	POINT pt;		// windows cursor position before hooking
	int LBState;
	int MBState;
	int RBState;
	
	HANDLE hLoadLock;	// media load lock
	HANDLE hRenderLock;	// prevent render and input thread atemptting simultaneous writes
	
	TMARQUEE *marquee;	// a general purpose marquee
	
}TGUI;

/*
typedef struct{
	wchar_t *name;
	int	channel;
}TTUNERCHANNEL;

typedef struct{
	char cchannel[32];	// channel name
	char ccountry[32];	// country name
	int channel;
	int country;

	TTUNERCHANNEL *channels;
	int channelIdx;
}TTUNER;
*/

typedef struct{
	int (*callback) (TTOUCHCOORD *pos, int touch_flags, void *usr_ptr);
	void *ptr;
	
	int isValid:1;		// is structure valid
	int isEnabled:1;	// is page accepting input
	int	padding:30;
}TPAGEIN;

typedef struct{
	int (*callback) (TVLCPLAYER *vp, TFRAME *frame, void *usr_ptr);
	TFRAME *frame;
	void *ptr;
	
	int isValid:1;		// is structure valid
	int isEnabled:1;	// is function ready 
	int isOpen:1;		// has page been initiated
	int	padding:29;	
}TPAGECTRL;

typedef struct{
	TPAGEIN	in;
	TPAGECTRL render;
	TPAGECTRL open;
	TPAGECTRL close;
	int pageId;
	int isValid;		// is struct valid
	int isEnabledPri;	// do we render this page as a primary underlay
	int isEnabledSec;	// do we render this page as secondary overlay
	void *ptr;
	char *title;
}TPAGE;

typedef struct{
	TPAGE page[PAGE_TOTAL];
	int active;
}TPAGES;

typedef struct{
	TFRAME *image;
	int bpp;
	wchar_t path[MAX_PATH+1];
	int id;
}TIMAGECACHEITEM;

typedef struct{
	TIMAGECACHEITEM *items;
	int total;
}TIMAGECACHE;

// defines a single shot timer
typedef struct{
	int state;			// ready to fire
	unsigned int time;	// time to fire in ms
	void (*func) (TVLCPLAYER *vp);
	void *ptr;
}TTIMER;

typedef struct{
	TTIMER queue[TIMER_TOTAL];
}TTIMERS;

struct TVLCPLAYER {
	TCTX ctx;
	TGUI gui;
	TVLCCONFIG vlcconfig;	// libvlc config storage
	TVLCCONFIG *vlc;		// pointer to a libvlc config
	TPAGES pages;
	//TTUNER tuner;
	TTIMERS timers;
	TIMAGECACHE *imgc;		// image cache
	TMETATAGCACHE *tagc;	// tag cache

	// all playlists are stored here
	TPLAYLISTMANAGER *plm;		// playlist manager
	
	// this will change only if a track is selected (from elsewhere) which is not from this playlist
	int queuedPlaylist;
	int displayPlaylist;

	uint64_t freq;
	uint64_t tStart;
	float resolution;
	
	int currentFType;	// 0:video, 1:audio, 2:image
	int updateTimer;	// main refresh ticker
	int applState;
	float fTime;		// current frame time
	float dTime[16];	// frame delta time 
	
	HANDLE hUpdateEvent;
	THWD *hw;
	lDISPLAY did;
	int isVirtDisplay;

	TEDITBOX input;
};


//#define getPlayState(x) (((TVLCPLAYER*)x)->vlc->playState && ((TVLCPLAYER*)x)->vlc->m && ((TVLCPLAYER*)x)->vlc->mp)
#define getPlayState(x) (((TVLCPLAYER*)x)->vlc->playState)

// get active vlc config
#define getConfig(x) ((TVLCPLAYER*)x)->vlc

void unloadMedia (TVLCPLAYER *vp, TVLCCONFIG *vlc);
int loadMedia (TVLCPLAYER *vp, TVLCCONFIG *vlc, const char *mediaPath);

void detachEvents (TVLCCONFIG *vlc, TVLCPLAYER *vp);
int writePlaylist (TVLCPLAYER *vp, PLAYLISTCACHE *plc, wchar_t *buffer, size_t len);
int browserLoadMediaFile (TVLCPLAYER *vp, const char *utf8path);
void killOvrAniTicker (TVLCPLAYER *vp);
void setApplState (TVLCPLAYER *vp, int state);
void enableHighlight (TVLCPLAYER *vp, TBUTTON *button);
void enableOverlay (TVLCPLAYER *vp);
int togglePlayState (TVLCPLAYER *vp);
int obuttonCB (const TTOUCHCOORD *pos, TBUTTON *button, const int id, const int flags, void *ptr);
void exitAppl (TVLCPLAYER *vp);

float getTime(TVLCPLAYER *vp);

int setDisplayBrightness (TVLCPLAYER *vp, int level);
int startPlaylistTrack (TVLCPLAYER *vp, PLAYLISTCACHE *plc, const int track);
void player_play (TVLCPLAYER *vp, TVLCCONFIG *vlc);
void player_stop (TVLCPLAYER *vp, TVLCCONFIG *vlc);
void player_pause (TVLCPLAYER *vp, TVLCCONFIG *vlc);
int player_prevTrack (TVLCPLAYER *vp, TVLCCONFIG *vlc);
int player_nextTrack (TVLCPLAYER *vp, TVLCCONFIG *vlc);
int tuner_gotoChannel (TVLCPLAYER *vp, int channelIdx);
void setPlaybackMode (TVLCPLAYER *vp, int mode);
int getPlaybackMode (TVLCPLAYER *vp);
void startRefreshTicker (TVLCPLAYER *vp, const float fps);
void touchIn (TTOUCHCOORD *pos, int flags, void *ptr);
int getPage (TVLCPLAYER *vp);
void setPage (TVLCPLAYER *vp, int id);
int getPageSec (TVLCPLAYER *vp);
void setPageSec (TVLCPLAYER *vp, int id);
void setAwake (TVLCPLAYER *vp);
int getIdle (TVLCPLAYER *vp);
int setVolume (TVLCPLAYER *vp, int volume);

void cleanVideoBuffers (TVLCPLAYER *vp);

int renderLock (TVLCPLAYER *vp);
void renderUnlock (TVLCPLAYER *vp);

void lockVLCVideoBuffer (TVLCPLAYER *vp);
void unlockVLCVideoBuffer (TVLCPLAYER *vp);

void (CALLBACK loadArtworkCB) (UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2);

int touchDispatch (TTOUCHCOORD *pos, int flags, TVLCPLAYER *vp);

void initNextTrack (TVLCPLAYER *vp);

void resetOverlay (TVLCPLAYER *vp);
void getNewTrackVariables (TVLCPLAYER *vp);
void resetHighlight (TVLCPLAYER *vp);


void marqueeAdd (TVLCPLAYER *vp, TMARQUEE *mq, const char *str, const unsigned int timeout);
int marqueeDraw (TVLCPLAYER *vp, TFRAME *frame, TMARQUEE *mq);

void resetCurrentDirectory ();
void timerSet (TVLCPLAYER *vp, const int id, const unsigned int ms);

void trackPlay (TVLCPLAYER *vp);
void trackPlayPause (TVLCPLAYER *vp);

#endif



