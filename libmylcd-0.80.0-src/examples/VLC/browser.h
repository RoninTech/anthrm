
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

#ifndef _BROWSER_H_
#define _BROWSER_H_


#define EXT_AUDIO	0	/* sequence should tie in with BBUTTON_FILTER_/ui.h */
#define EXT_VIDEO	1
#define EXT_IMAGE	2
#define EXT_MEDIA	3	/* all above */
#define EXT_ALL		4	/* all files (same as *.*) */
#define EXT_DEFAULT EXT_AUDIO /* startup default */

#define METRICTEST			"qwertyuiopasdfghjklxzcvbnm QWERTYUIOPASDFGHJKLZXCVBNM0123456789"
#define SCROLLBARVSPACE		6

// taken from vlc/include/vlc_interface.h
// keep in sync with playlist.h
#define EXTAUDIO \
    L".a52",\
    L".aac",\
    L".ac3",\
    L".aiff",\
    L".amr",\
    L".aob",\
    L".ape",\
    L".ay",\
    L".dts",\
    L".flac",\
    L".it",\
    L".m4a",\
    L".m4p",\
    L".mpa",\
    L".mid",\
    L".mka",\
    L".mlp",\
    L".mod",\
    L".mp1",\
    L".mp2",\
    L".mp3",\
    L".mpc",\
    L".oga",\
    L".ogg",\
    L".oma",\
    L".rmi",\
    L".s3m",\
    L".spx",\
    L".tta",\
    L".voc",\
    L".vqf",\
    L".w64",\
    L".wav",\
    L".wma",\
    L".wv",\
    L".xa",\
    L".xm"

// taken from vlc/vlc_interface.h
#define EXTVIDEO \
	L".3gp",\
	L".amv",\
	L".asf",\
	L".avi",\
	L".divx",\
	L".dv",\
	L".flv",\
	L".gxf",\
	L".iso",\
	L".m1v",\
	L".m2v",\
	L".m2t",\
	L".m2ts",\
	L".m4v",\
	L".mkv",\
	L".mov",\
	L".mp2",\
	L".mp4",\
	L".mpeg",\
	L".mpeg1",\
	L".mpeg2",\
	L".mpeg4",\
	L".mpg",\
	L".mts",\
	L".mxf",\
	L".nsv",\
	L".nuv",\
	L".ogg",\
	L".ogm",\
	L".ogv",\
	L".ogx",\
	L".ps",\
	L".rec",\
	L".rm",\
	L".rmvb",\
	L".tod",\
	L".ts",\
	L".vob",\
	L".vro",\
	L".webm",\
	L".wmv"\
  
             
#define EXTIMAGE \
	L".jpg",\
	L".png",\
	L".bmp",\
	L".tga",\
	L".jpeg"

                         
/*                       
#define EXTENSIONS_PLAYLIST "*.asx;*.b4s;*.ifo;*.m3u;*.m3u8;*.pls;*.ram;*.rar;*.sdp;*.vlc;*.xspf;*.zip"
#define EXTENSIONS_SUBTITLE "*.cdg;*.idx;*.srt;*.sub;*.utf;*.ass;*.ssa;*.aqt;*.jss;*.psb;*.rt;*.smi"
*/

/*
sortMode:		1=by name ascending, 2=by name descending
				3=by file size a, 4=by filesize d
				5=created a, 6=created d
				7=modified a, 8=modified d
*/

struct TDATAOBJECT{
	wchar_t *linkName;
	wchar_t *name;
	int dwFileAttributes;
	uint64_t fsize;				// file size only
	uint64_t creationDate;
	uint64_t modifiedDate;
	TPATHOBJECT *self;			// this parent
};

struct TPATHOBJECT{
	TDATAOBJECT dir;		// this directory
	TDATAOBJECT **file;		// file objects within this directory
	TPATHOBJECT **subdir;	// list [sub]directories within this directory
	TPATHOBJECT *parent;	
	
	int tDirObj;				// number of items in subdir list
	int tFileObj;				// number of objects is proportional to number of filtered files in directory	
	unsigned int tFileStruct;	// size of file list
	unsigned int tDirStruct;	// size of subdir list
	int firstObject;			// index to first object for display. enables scrolling
	int driveType;				// is DRIVE_CDROM, DRIVE_REMOVABLE, etc..

	unsigned int isRoot:1;		// there can only be one root in the tree
	unsigned int isLocked:1;	// 1:prevent updates to this directory structure
	unsigned int isAccessed:1;	// 1:has previously been requested through user selection
	unsigned int isDriveLetter:1;
	unsigned int hasLink:1;		// is a directory and contains a symbolic link. eg; symbolic link 'My Documents' may point to 'c:/users/somedocuments/'
	unsigned int pad:27;
};

// used by browser.c for personal links
typedef struct{
	wchar_t *path;
	wchar_t *link;
}TSTRSHORTCUT;

int setPlaylistPlayingItem (TVLCPLAYER *vp, PLAYLISTCACHE *plc, int trk, const unsigned int hash);
int browserImportPlaylistByDirW (TFBROWSER *browser, wchar_t *path, PLAYLISTCACHE *plc, const int recursive);
int browserImportPlaylistByDir (TFBROWSER *browser, char *path, PLAYLISTCACHE *plc, const int recursive);
int buildCompletePath (TPATHOBJECT *path, wchar_t *buffer, size_t bufferSize);

int isVideoFile (wchar_t *path);
int openBrowser (TVLCPLAYER *vp, TFRAME *frame, TFBROWSER *browser);
int closeBrowser (TVLCPLAYER *vp, TFRAME *frame, TFBROWSER *browser);
int drawBrowser (TVLCPLAYER *vp, TFRAME *frame, TFBROWSER *browser);
int touchBrowser (TTOUCHCOORD *pos, int flags, TVLCPLAYER *vp);

void setExtFilter (TVLCPLAYER *vp, TFBROWSER *browser, int mode);
int browserNavigateToDirectory (TVLCPLAYER *vp, TFBROWSER *cb, wchar_t *folder);
int browserNavigateToDirectoryAndFile (TVLCPLAYER *vp, TFBROWSER *fb, wchar_t *folder, const wchar_t *fname);
TFRAME *getImage (TFBROWSER *browser, int idx);

int hasPathExt (wchar_t *path, wchar_t **exts);

#endif


