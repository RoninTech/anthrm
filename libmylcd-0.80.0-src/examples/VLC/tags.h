
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


// keep this in sync with meta.c:ctagStrLookup[]
// and libvlc_media.h:libvlc_meta_t

#ifndef _TAGS_H_
#define _TAGS_H_

#define TAGIMGCACHESIZE 512

typedef struct TMETARECORD TMETARECORD;

// keep this in sync with meta.c::tagStrLookupp[] and editbox.c:wtagStrLookup[]
enum _tags_meta {
    MTAG_Title,
    MTAG_Artist,
    MTAG_Genre,
    MTAG_Copyright,
    MTAG_Album,
    MTAG_TrackNumber,
    MTAG_Description,
    MTAG_Rating,
    MTAG_Date,
    MTAG_Setting,
    MTAG_URL,
    MTAG_Language,
    MTAG_NowPlaying,
    MTAG_Publisher,
    MTAG_EncodedBy,
    MTAG_ArtworkURL,
    MTAG_TrackID,	// end of libvlcs' tags
    
	MTAG_LENGTH,	// beginning of additional tags to libvlcs', in all block
	MTAG_FILENAME,	// filename.ext only
	MTAG_POSITION,	// position in playlist
	MTAG_PATH,		// complete track path+filename+ext
    MTAG_TOTAL
};

typedef struct{
	int enabled;
	TFRAME *img;	// final scaled artwork image
	wchar_t *path;	// location of artwork image as provided by libvlc.
					// path is promoted to utf16 as required by libmylcd
	unsigned int hash;
}TTAGIMG;

typedef struct{
	char *path;					// path to media with which these tags belong to
	char *tag[MTAG_TOTAL];		// as of vlc 1.1 there are 17 tags defined in libvlc_media.h
								// use enum MTAG_ to index
	unsigned int hash;			// path based lookup key
	TTAGIMG *art;				// a pointer to TMETATAGCACHE::art[n]
	
	unsigned int hasArt:1;			// ::art contains a valid image struct (TTAGIMG)
	unsigned int hasTitle:1;		// MTAG_Title tag is valid
	unsigned int hasArtworkURL:1;	// MTAG_ArtworkURL tag is valid
	unsigned int hasFilename:1;
	unsigned int pad:28;
}TMETAITEM;

struct TMETARECORD{
	TMETAITEM *item;
	TMETARECORD *next;
};

typedef struct{
	TMETARECORD *first;

	TTAGIMG	*art[TAGIMGCACHESIZE];	// all artwork is stored here
	HANDLE hMutex;
}TMETATAGCACHE;


int tagAdd (TMETATAGCACHE *tagc, const char *path, const int tagid, const char *tag, const int overwrite);
char *tagRetrieve (TMETATAGCACHE *tagc, const char *path, const int tagid, char *buffer, const size_t len);
char *tagRetrieveByHash (TMETATAGCACHE *tagc, const unsigned int hash, const int tagid, char *buffer, const size_t len);

TMETATAGCACHE *tagNew ();
void tagFree (TMETATAGCACHE *tagc);

void tagFlush (TMETATAGCACHE *tagc);
void tagFlushArt (TMETATAGCACHE *tagc);

int tagArtworkAdd (TMETATAGCACHE *tagc, const char *path, TTAGIMG *art);
TTAGIMG *tagArtworkGetByHash (TMETATAGCACHE *tagc, const unsigned int hash);

int tagAddByHash (TMETATAGCACHE *tagc, const char *path, const unsigned int hash, const int tagid, const char *tag, const int overwrite);

TTAGIMG *tagArtworkGet (TMETATAGCACHE *tagc, const char *path);
TTAGIMG *tagArtworkAlloc (TMETATAGCACHE *tagc, const unsigned int hash);

int tagsTitleAvailableByHash (TMETATAGCACHE *tagc, const unsigned int hash);
int tagIsFilenameAvailableByHash (TMETATAGCACHE *tagc, const unsigned int hash);
int tagArtworkIsArtworkAvailableByHash (TMETATAGCACHE *tagc, const unsigned int hash);
int tagArtworkIsURLAvailableByHash (TMETATAGCACHE *tagc, const unsigned int hash);

int tagLock (TMETATAGCACHE *tagc);
void tagUnlock (TMETATAGCACHE *tagc);

int countUsedArtSlots (TMETATAGCACHE *tagc);

TTAGIMG *g_tagArtworkGetByHash (TMETATAGCACHE *tagc, const unsigned int hash);
TTAGIMG *g_tagArtworkAlloc (TMETATAGCACHE *tagc, const unsigned int hash);
int g_tagArtworkAdd (TMETATAGCACHE *tagc, const char *path, TTAGIMG *art);
TMETAITEM *g_tagFindEntryByHash (TMETATAGCACHE *tagc, const unsigned int hash);


#endif
