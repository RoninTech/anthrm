
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



#ifndef _PLAYLIST_H_
#define _PLAYLIST_H_


#define MIN(a,b) ((a<b)?(a):(b))
#define MAX(a,b) ((a>b)?(a):(b))


// taken from vlc/vlc_interface.h
// keep in sync with browser.h
#define EXTVIDEOA \
	".avi",\
	".wmv",\
	".mkv",\
	".mpg",\
	".mp4",\
	".mpeg",\
	".vob",\
	".divx",\
	".dv",\
	".flv",\
	".m1v",\
	".m2v",\
	".m2t",\
	".m2ts",\
	".m4v",\
	".mov",\
	".mp2",\
	".ts",\
	".mpeg1",\
	".mpeg2",\
	".mpeg4",\
	".gxf",\
	".iso",\
	".3gp",\
	".amv",\
	".asf",\
	".mts",\
	".mxf",\
	".nsv",\
	".nuv",\
	".ogg",\
	".ogm",\
	".ogv",\
	".ogx",\
	".ps",\
	".rec",\
	".rm",\
	".rmvb",\
	".tod",\
	".vro",\
	".webm"
  
#define EXTAUDIOA \
    ".mp3",\
    ".ogg",\
    ".wav",\
    ".wma",\
    ".aac",\
    ".ac3",\
    ".aiff",\
    ".dts",\
    ".flac",\
    ".m4a",\
    ".m4p",\
    ".mid",\
    ".mka",\
    ".a52",\
    ".ay",\
    ".mlp",\
    ".mod",\
    ".mp1",\
    ".mp2",\
    ".mpc",\
    ".oga",\
    ".oma",\
    ".rmi",\
    ".s3m",\
    ".spx",\
    ".it",\
    ".amr",\
    ".aob",\
    ".ape",\
    ".tta",\
    ".voc",\
    ".vqf",\
    ".w64",\
    ".wv",\
    ".xa",\
    ".xm"



TMETACB *playlistMetaInit (void *user_ptr);
void playlistMetaShutdown (TMETACB *meta);

void trackLoadEvent (TVLCPLAYER *vp, PLAYLISTCACHE *plc, TPLAYLIST2 *pl, int trackIdx);
void playlistChangeEvent (TVLCPLAYER *vp, PLAYLISTCACHE *plc, TPLAYLIST2 *pl, int trackIdx);

void playlistMetaGetMeta (TVLCPLAYER *vp, TMETACB *meta, PLAYLISTCACHE *plc, int from, int to);
void freeMetaSlots (TMETACB *metacb, const int state, const int byTime);
void countItems (TVLCPLAYER *vp, TMETACB *metacb);
//void countItemsPrint (TMETACB *metacb);

int importPlaylistW (TPLAYLISTMANAGER *plm, PLAYLISTCACHE *plc, TMETATAGCACHE *tagc, const wchar_t *path);
int importPlaylist (TPLAYLISTMANAGER *plm, PLAYLISTCACHE *plc, TMETATAGCACHE *tagc, const char *path);

PLAYLISTCACHE *getPrimaryPlaylist (TVLCPLAYER *vp);
PLAYLISTCACHE *getQueuedPlaylist (TVLCPLAYER *vp);
PLAYLISTCACHE *getDisplayPlaylist (TVLCPLAYER *vp);
int playlistManagerRestart (TVLCPLAYER *vp);

#endif

