
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


#ifndef _UI_H_
#define _UI_H_

#define PLAYER_NAME				"VLCstream player"
#define PLAYER_VERSION			PLAYER_NAME"_07082011"	/* ddmmyyyy */

#define SKINFILEBPP				LFRM_BPP_32A
#define SKINDROOT				L"vsskin"
#define FONTD(x)				SKINDROOT"/data/"x

#define VLCSPLAYLIST			L"vlcsplaylist.m3u8"
#define VLCSPLAYLISTFILE		"vlcsplaylist"
#define VLCSPLAYLISTEXT			".m3u8"
#define VLCSPLAYLISTEXTW		L".m3u8"


#define GLOBALFILEMASK			L"*.*"
#define TOUCHDEADZONESIZE		10				/* size of deadzone [pixel] legnth when testing for drag */
#define HIGHLIGHTPERIOD			200
#define MENUOVERLAYPERIOD		6000
#define MAXPLRENDERITEMS		31				/* maximum number of stacked playlist items displayable */
#define ASCII_PAGE				CMT_ISO8859_15	/* set default encoding/locale page */
#define BFONT					LFTW_B14		/* use this as primary font */
#define MFONT					LFTW_18x18KO	/* use this font for larger detail in meta page */
#define UPDATERATE_IDLE			0.1				/* fps, maintain this fps when idle (0.1 = 1 update every 10 seconds) */ 
#define IDLETIME				(120 * 1000)	/* ms, time to go idle after this length of inactivity */
#define MINUIFPS				(15.0)
#define MCTRLVIDEOBLURHEIGHT	68
#define CACHEREADAHEAD			32				/* preload the meta data from this number of tracks in advance */
#define METALISTSIZE			4096
#define METATIMEOUT				(5*60*1000)
#define MAX_PATH_UTF8			((8*MAX_PATH)+1)
#define PLAYLIST_DECAYRATE		(0.010f)
#define PLAYLIST_DECAYFACTOR	(10.00f)
#define MOFFSETX				6	/* define the cursor offset selection point */
#define MOFFSETY				2
#define PLAYLIST_PRIMARY		VLCSPLAYLISTFILE

#define WM_MYLCDTOUCHIN			(WM_USER+2000)
#define WM_TOUCHIN				(WM_MYLCDTOUCHIN+1)		// (3025)
#define WM_MCTRLNEXTTRACK		(WM_MYLCDTOUCHIN+10)	// (3034)

#define MYCOMPUTERNAME			L"My Computer"	/* default alias to use in the event actual alias could not be retrieved */
#define MYDOCUMENTS				L"My Documents"
#define MYDESKTOP				L"Desktop"


#if !G19DISPLAY			/*  usbd480 */

#define MAXARTWIDTH			(256)
#define MAXARTHEIGHT		(251)
#define DEFAULTARTSCALE		(0.4)

#define GOOMREZMODES \
	{"480","272"},\
	{"320","180"},\
	{"160","90"}

#define TOTALSKINS			(2)
#define SKINS \
	L"FlatPlain480",\
	L"Glow480"
/*	L"Spectrum480",\
	L"Neon480",\
	L"Grey480",\
*/
	
#else					/*  G19  */

#define MAXARTWIDTH			(248)
#define MAXARTHEIGHT		(219)
#define DEFAULTARTSCALE		(0.3)

#define GOOMREZMODES \
	{"320","240"},\
	{"224","168"},\
	{"160","90"}

#define TOTALSKINS				2
#define SKINS \
	L"FlatPlain320",\
	L"Glow320"
/*	L"Spectrum320",\
	L"Neon320",\
	L"Grey320",\
*/
#endif

enum _PAGE
{
	PAGE_NONE,				/* video */
	PAGE_OVERLAY,			/* video control icons */
	PAGE_BROWSER,			/* file explorer */
	PAGE_CLOCK,
	PAGE_CFG,
	PAGE_SUB,
	PAGE_IMGOVR,
	PAGE_CHAPTERS,
	PAGE_EXIT,
	PAGE_META,
	PAGE_PLAYLIST2,
	PAGE_PLOVERLAY,
	PAGE_PLAYLISTPLM,
	PAGE_LORENZ,
	PAGE_PARTICLES,
	PAGE_RC,
	PAGE_TETRIS,
	PAGE_TOTAL
};

// video overlay control
enum _VOVR
{
	VBUTTON_PLAY,
	VBUTTON_PAUSE,
	VBUTTON_STOP,
	VBUTTON_PRETRACK,
	VBUTTON_NEXTTRACK,
	VBUTTON_POSBACK,
	VBUTTON_POSFORWARD,
	VBUTTON_EXIT,
	VBUTTON_TRACKPOS,
	VBUTTON_VOLUME,
	VBUTTON_TRACKPOSTIP,
	VBUTTON_VOLUMEBASE,
	VBUTTON_OPEN,
	VBUTTON_PLAYLIST,
	VBUTTON_TIMESTAMP,
	VBUTTON_CHAPTERS,
	VBUTTON_SCLOCK,
	VBUTTON_SMETA,
	VBUTTON_TOTAL
};

// explorer
enum _EXP
{
	BBUTTON_VIEW,
	BBUTTON_PLAY,
	BBUTTON_CLOSE,		/*previous page, overlay control page*/
	BBUTTON_EXIT,
	
	BBUTTON_FILTER_AUDIO,
	BBUTTON_FILTER_VIDEO,
	BBUTTON_FILTER_IMAGE,
	BBUTTON_FILTER_MEDIA,	// video+audio+image
	BBUTTON_FILTER_ALL,		// everything in directory

	BBUTTON_SORT_UNSORTED,
	BBUTTON_SORT_UP_NAME,
	BBUTTON_SORT_DN_NAME,
	BBUTTON_SORT_UP_MODIFIED,
	BBUTTON_SORT_DN_MODIFIED,
	BBUTTON_SORT_UP_CREATED,
	BBUTTON_SORT_DN_CREATED,
	BBUTTON_SORT_UP_SIZE,
	BBUTTON_SORT_DN_SIZE,
	
	BBUTTON_PATHBACK,
	BBUTTON_SBAR_UP,
	BBUTTON_SBAR,
	BBUTTON_SBAR_DOWN,
	BBUTTON_TOTAL
};


enum _CFG
{
	CFGBUTTON_AR_AUTO,
	CFGBUTTON_AR_177,
	CFGBUTTON_AR_155,
	CFGBUTTON_AR_133,
	CFGBUTTON_AR_125,
	CFGBUTTON_AR_122,
	CFGBUTTON_AR_15,
	CFGBUTTON_AR_16,
	CFGBUTTON_AR_167,
	CFGBUTTON_AR_185,
	CFGBUTTON_AR_220,
	CFGBUTTON_AR_235,
	CFGBUTTON_AR_240,
	
	CFGBUTTON_VIS_OFF,
	CFGBUTTON_VIS_DISABLED,
	//CFGBUTTON_VIS_DISABLED_IDLE,
	CFGBUTTON_VIS_VUMETER,
	CFGBUTTON_VIS_SMETER,
	CFGBUTTON_VIS_PINEAPPLE,
	CFGBUTTON_VIS_SPECTRUM,
	CFGBUTTON_VIS_SCOPE,
	CFGBUTTON_VIS_GOOM_Q3,
	CFGBUTTON_VIS_GOOM_Q2,
	CFGBUTTON_VIS_GOOM_Q1,
		
	CFGBUTTON_BRN_0,
	CFGBUTTON_BRN_10,
	CFGBUTTON_BRN_20,
	CFGBUTTON_BRN_30,
	CFGBUTTON_BRN_40,
	CFGBUTTON_BRN_50,
	CFGBUTTON_BRN_60,
	CFGBUTTON_BRN_70,
	CFGBUTTON_BRN_80,
	CFGBUTTON_BRN_90,
	CFGBUTTON_BRN_100,
	
	CFGBUTTON_CLOSE,
	CFGBUTTON_FPS_ON,
	CFGBUTTON_FPS_OFF,
	CFGBUTTON_CLOCK,
	CFGBUTTON_WRITEPLAYLIST,
	CFGBUTTON_SUBTITLE,
	CFGBUTTON_SWAPRB_ON,
	CFGBUTTON_SWAPRB_OFF,
	CFGBUTTON_SKIN,

	CFGBUTTON_UR_5,
	CFGBUTTON_UR_10,
	CFGBUTTON_UR_15,
	CFGBUTTON_UR_20,
	CFGBUTTON_UR_25,
	CFGBUTTON_UR_30,
	CFGBUTTON_UR_35,
	CFGBUTTON_UR_40,
	CFGBUTTON_TOTAL
};

enum _sub
{
	SUBBUTTON_SUB1,
	SUBBUTTON_SUB2,
	SUBBUTTON_SUB3,
	SUBBUTTON_SUB4,
	SUBBUTTON_SUB5,
	SUBBUTTON_SUB6,
	SUBBUTTON_SUB7,
	SUBBUTTON_SUB8,
	SUBBUTTON_SUB9,
	SUBBUTTON_SUB10,
	SUBBUTTON_SUB11,
	SUBBUTTON_SUB12,
	SUBBUTTON_SUB13,
	SUBBUTTON_SUB14,
	SUBBUTTON_SUB15,
	SUBBUTTON_SUB16,
	SUBBUTTON_SUB17,
	SUBBUTTON_SUB18,
	SUBBUTTON_SUB19,
	SUBBUTTON_SUB20,
	SUBBUTTON_CLOSE,
	SUBBUTTON_TOTAL
};

enum _chap
{
	CHPBUTTON_LEFT,
	CHPBUTTON_RIGHT,
	CHPBUTTON_TOTAL
};

enum _exit
{
	EXITBUTTON_YES,
	EXITBUTTON_NO,
	EXITBUTTON_TOTAL
};

enum _meta
{
	METABUTTON_CLOSE,
	METABUTTON_UP,
	METABUTTON_DOWN,
	METABUTTON_LEFT,
	METABUTTON_RIGHT,
	METABUTTON_TOTAL
};

enum _pl2
{
	P2BUTTON_CTRL,
	P2BUTTON_CLOSE,
	P2BUTTON_TOTAL
};

enum _plm
{
	PLMBUTTON_CLOSE,
	PLMBUTTON_TOTAL
};


// playlist overlay
enum _PLOVR
{
	PLOBUTTON_META,
	PLOBUTTON_NAVUP,
	PLOBUTTON_NAVDN,
	PLOBUTTON_ARTSIZE,
	PLOBUTTON_PLAY,		// play
	PLOBUTTON_PAUSE,
	PLOBUTTON_STOP,
	PLOBUTTON_PREVTRACK,
	PLOBUTTON_NEXTTRACK,
	PLOBUTTON_OPEN,
	PLOBUTTON_CLOSE,
	PLOBUTTON_TOTAL
};

enum _vis
{
	VIS_DISABLED,
	VIS_VUMETER,
	VIS_SMETER,
	VIS_PINEAPPLE,
	VIS_SPECTRUM,
	VIS_SCOPE,
	VIS_GOOM_Q3,
	VIS_GOOM_Q2,
	VIS_GOOM_Q1,
	//VIS_DISABLED_IDLE,
	VIS_TOTAL
};

enum _timers
{
	TIMER_OVERLAYRESET,		// ui control
	TIMER_HIGHLIGHTRESET,
	
	TIMER_NEWTRACKVARS,		// internal playback control
	TIMER_MCTRLNEXTTRACK,
	
	TIMER_PREVTRACK,		// hotkeys
	TIMER_NEXTTRACK,
	TIMER_PLAYPAUSE,
	TIMER_STOP,
	TIMER_VOLUP,
	TIMER_VOLDN,

	TIMER_TOTAL
};

typedef struct{
	int active;
	float time;
	int x;
	int y;
	int *colour;
}TMPOINT;

typedef struct{	
	TMPOINT *list;
	TMPOINT *rlist;
	TMPOINT *dlist;
	int total;	// total points
	int fIndex;
	int tFrames;
	uint64_t freq;
	uint64_t t0;
	uint64_t t_old;
	float resolution;
}TMORPH;

typedef struct{
	float scaleDefault;
	float scale;		// current scale
	float scaleRate;
	TFRAME *image;
	int direction;		// 0:off, 1:in, 2:out
}TZOOM;

typedef struct{	
	TMORPH *src;	// original
	TMORPH *morph;	// working clone of original
	TZOOM *zoom;
	int state;		//  0:off, 1:create, 2:do
}TANIMATION;

typedef struct TBUTTON TBUTTON;
struct TBUTTON {
	TFRAME *activeBtn;	// pointer to active button image
	TFRAME *image;
	TFRAME *highlight;
	
	T2POINT pos;	// relative to display frame
	int (*callback) (const TTOUCHCOORD *pos, TBUTTON *button, const int id, const int message, void *vp);
	int id;
	unsigned int enabled:1;
	unsigned int acceptDrag:1;
	unsigned int canAnimate:2;	// 0:none, 1:zoom, 2:morph/dissolve
	unsigned int filler:28;
	TANIMATION ani;
	int icIdi;	// image cache storage handle
	int icIdh;	// image cache storage handle
};



#endif
