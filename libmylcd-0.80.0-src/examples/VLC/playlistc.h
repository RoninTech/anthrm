
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


#ifndef _PLAYLISTC_H_
#define _PLAYLISTC_H_

typedef struct TPLAYLISTRECORD TPLAYLISTRECORD;
typedef struct PLAYLISTCACHE PLAYLISTCACHE;

typedef struct{
	TTOUCHCOORD last;	// last input location
	TTOUCHCOORD pos;	// current drag position
	TTOUCHCOORD d0;
	TTOUCHCOORD d1;
}TDRAGPOS;

typedef struct {
	//TLPOINTEX title;			// location of rendered title
	TLPOINTEX art;				// art location
	int playlistPosition;
}PLAYLISTRENDERITEM;

typedef struct {
	PLAYLISTRENDERITEM item[MAXPLRENDERITEMS+1];	// we can only display so many items at any given time
	TLPOINTEX renderArea;		// rect of rendered playlist within frame. 
	TDRAGPOS dragLocation;		// touch input data
	int selectedItem;			// current selection in playlist screen (selected via buttons prev/next, touch or mouse)
	int playingItem;			// currently playing track and/or selection via browser screen
	int playingItemPrevious;
}PLAYLISTRENDER;


typedef struct{
	char *path;				// complete utf8 path to media including filename
	char *title;			// displayed title
	unsigned int hash;		// path lookup key
}TPLAYLISTITEM;

struct TPLAYLISTRECORD{
	TPLAYLISTITEM *item;
	TPLAYLISTRECORD *next;
};

struct PLAYLISTCACHE {
	HANDLE hMutex;
	TPLAYLISTRECORD *first;
	int total;
	char name[MAX_PATH_UTF8+1];	// give the playlist a name

	// each playlist has its own independent rendering detail
	PLAYLISTRENDER *pr;
	unsigned int artHash;
};


// create an empty playlist
PLAYLISTCACHE *playlistNew (const char *name);

// flush playlist and all allocated memory
void playlistFree (PLAYLISTCACHE *plc);

// flush current playlist items
void playlistDelete (PLAYLISTCACHE *plc);

// get number of records in list
int playlistGetTotal (PLAYLISTCACHE *plc);

// create and add a new record
int playlistAdd (PLAYLISTCACHE *plc, const char *path);

// add a title to a preexisiting record
int playlistSetTitle (PLAYLISTCACHE *plc, const int pos, const char *title, const int overwrite);

int playlistDeleteRecord (PLAYLISTCACHE *plc, const int pos);

TPLAYLISTITEM *playlistGetItem (PLAYLISTCACHE *plc, const int pos);
char *playlistGetPath (PLAYLISTCACHE *plc, const int pos, char *buffer, const size_t len);
char *playlistGetTitle (PLAYLISTCACHE *plc, const int pos, char *buffer, const size_t len);
unsigned int playlistGetHash (PLAYLISTCACHE *plc, const int pos);
int playlistGetPositionByHash (PLAYLISTCACHE *plc, const unsigned int hash);

// to aid debugging
int playlistRunList (PLAYLISTCACHE *plc);

// global search for all meta data
int playlistSearch (PLAYLISTCACHE *plc, TMETATAGCACHE *tagc, const char *str, const int from);

// search in specific tags only
int playlistSearchTag (PLAYLISTCACHE *plc, TMETATAGCACHE *tagc, const int mtag, const char *str, const int from);


int playlistLock (PLAYLISTCACHE *plc);
void playlistUnlock (PLAYLISTCACHE *plc);

int playlistPrune (PLAYLISTCACHE *plc, const int remove);

char *playlistGetName (PLAYLISTCACHE *plc, char *buffer, const size_t len);
int playlistSetName (PLAYLISTCACHE *plc, char *buffer);

#endif

