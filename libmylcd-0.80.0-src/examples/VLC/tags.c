
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


static inline int _tagLock (TMETATAGCACHE *tagc)
{
	return (WaitForSingleObject(tagc->hMutex, INFINITE) == WAIT_OBJECT_0);
}

static inline void _tagUnlock (TMETATAGCACHE *tagc)
{
	ReleaseMutex(tagc->hMutex);
}

int tagLock (TMETATAGCACHE *tagc)
{
	return _tagLock(tagc);
}

void tagUnlock (TMETATAGCACHE *tagc)
{
	_tagUnlock(tagc);
}

static inline TMETARECORD *_tagGetFirst (TMETATAGCACHE *tagc)
{
	return tagc->first;
}

static TMETARECORD *_tagGetLast (TMETATAGCACHE *tagc)
{
	TMETARECORD *restrict rec = _tagGetFirst(tagc);
	if (rec){
		while(rec->next)
			rec = rec->next;
	}
	return rec;
}
	
static TMETAITEM *_tagFindEntryByHash (TMETATAGCACHE *tagc, const unsigned int hash)
{
	TMETARECORD *restrict tag = _tagGetFirst(tagc);
	if (tag){
		do{
			if (tag->item->hash == hash)
				return tag->item;
		}while((tag=tag->next));
	}
	return NULL;
}

TMETAITEM *g_tagFindEntryByHash (TMETATAGCACHE *tagc, const unsigned int hash)
{
	return _tagFindEntryByHash(tagc, hash);
}

static TMETAITEM *_tagFindEntry (TMETATAGCACHE *tagc, const char *path)
{
	if (!path) return NULL;
	if (!*path) return NULL;
	return _tagFindEntryByHash(tagc, getHash(path));
}

static TMETAITEM *_tagCreateItem (TMETATAGCACHE *tagc, const unsigned int hash, const char *path)
{
	TMETAITEM *item = (TMETAITEM*)my_calloc(1, sizeof(TMETAITEM));
	if (item){
		item->hash = hash;
		if (path)
			item->path = my_strdup(path);
	}
	return item;
}

static TMETARECORD *_tagCreateRecord (TMETATAGCACHE *tagc)
{
	return (TMETARECORD*)my_calloc(1, sizeof(TMETARECORD));
}

// there may only be one instance of a tag per path
// if path exists then modify it, otherwise create a new entry.
static int _tagAddByHash (TMETATAGCACHE *tagc, const char *path, const unsigned int hash, const int tagid, const char *tag, const int overwrite)
{
	// search for an existing record
	TMETAITEM *item = _tagFindEntryByHash(tagc, hash);
	if (!item){
		// does not exist so create a new record
		item = _tagCreateItem(tagc, hash, path);
		TMETARECORD *newrec = _tagCreateRecord(tagc);
		if (!item || !newrec){
			printf("_tagAddByHash() out of memory %p %p\n", item, newrec);
			return 0;
		}
		newrec->item = item;
		newrec->next = NULL;
		
		TMETARECORD *rec = _tagGetLast(tagc);
		if (rec)
			rec->next = newrec;
		else
			tagc->first = newrec;
	}

	if (item->tag[tagid]){
		if (overwrite)
			my_free(item->tag[tagid]);
		else
			return 1;
	}
	item->tag[tagid] = my_strdup(tag);
	if (tagid == MTAG_Title)
		item->hasTitle = 1;
	else if (tagid == MTAG_ArtworkURL)
		item->hasArtworkURL = 1;
	else if (tagid == MTAG_FILENAME)
		item->hasFilename = 1;
	return 1;
}
/*
TMETAITEM *tagFindEntryByHash (TMETATAGCACHE *tagc, const unsigned int hash)
{
	TMETAITEM *item = NULL;
	if (_tagLock(tagc)){
		item = _tagFindEntryByHash(tagc, hash);
		_tagUnlock(tagc);
	}
	return item;
}
*/
int tagAddByHash (TMETATAGCACHE *tagc, const char *path, const unsigned int hash, const int tagid, const char *tag, const int overwrite)
{
	if (!tag || !hash || tagid < 0 || tagid >= MTAG_TOTAL)
		return 0;

	int ret = 0;
	if (_tagLock(tagc)){
		if (*tag)
			ret = _tagAddByHash(tagc, path, hash, tagid, tag, overwrite);
		_tagUnlock(tagc);
	}
	return ret;
}

int tagAdd (TMETATAGCACHE *tagc, const char *path, const int tagid, const char *tag, const int overwrite)
{
	if (!path || !tag) return 0;
	return tagAddByHash(tagc, path, getHash(path), tagid, tag, overwrite);
}

static TTAGIMG *tagArtworkGetSlot (TMETATAGCACHE *tagc, unsigned int hash)
{
	int slot = -1;
	
	// find current item from hash
	for (int i = 0; i < TAGIMGCACHESIZE; i++){
		if (tagc->art[i]){
			if (tagc->art[i]->hash == hash)
				return tagc->art[i];
		}else if (slot == -1){
			slot = i;
		}
	}
	
	// there must not be a current entry so create one
	if (slot != -1){
		const int i = slot;
		if (!tagc->art[i]){
			tagc->art[i] = my_malloc(sizeof(TTAGIMG));		
			if (tagc->art[i]){
				tagc->art[i]->enabled = 0;
				tagc->art[i]->hash = hash;
				tagc->art[i]->img = NULL;
				tagc->art[i]->path = my_calloc(MAX_PATH+1, sizeof(wchar_t));
				return tagc->art[i];
			}
		}
	}
	return NULL;
}


static void _tagArtworkFree (TTAGIMG *art)
{
	art->enabled = 0;
	if (art->img){
		lDeleteFrame(art->img);
		art->img = NULL;
	}
	if (art->path){
		my_free(art->path);
		art->path = NULL;
	}
}


int countUsedArtSlots (TMETATAGCACHE *tagc)
{
	int slots = 0;
	for (int i = 0; i < TAGIMGCACHESIZE; i++){
		if (tagc->art[i])
			slots++;
	}
	return slots;
}

TTAGIMG *tagArtworkAlloc (TMETATAGCACHE *tagc, const unsigned int hash)
{
	if (!hash || !tagc) return NULL;
	
	TTAGIMG *art = NULL;
	if (_tagLock(tagc)){
		art = tagArtworkGetSlot(tagc, hash);
		_tagUnlock(tagc);
	}
	return art;
}    					

TTAGIMG *g_tagArtworkAlloc (TMETATAGCACHE *tagc, const unsigned int hash)
{
	if (!tagc || !hash) return NULL;

	return tagArtworkGetSlot(tagc, hash);
}

static void _tagArtworkDelete (TTAGIMG *art)
{
	_tagArtworkFree(art);
	my_free(art);
}

int tagArtworkAdd (TMETATAGCACHE *tagc, const char *path, TTAGIMG *art)
{
	if (!path || !art || !tagc) return 0;
	
	TMETAITEM *item = NULL;
	if (_tagLock(tagc)){
		item = _tagFindEntry(tagc, path);
		if (item && !item->art){
			item->art = art;
			item->hasArt = 1;
		}
		_tagUnlock(tagc);
	}
	return (item != NULL);
}

int g_tagArtworkAdd (TMETATAGCACHE *tagc, const char *path, TTAGIMG *art)
{
	if (!tagc || !path|| !art) return 0;
	
	TMETAITEM *item = _tagFindEntry(tagc, path);
	if (item && !item->art){
		item->art = art;
		item->hasArt = 1;
	}
	return (item != NULL);
}

int tagIsFilenameAvailableByHash (TMETATAGCACHE *tagc, const unsigned int hash)
{
	int ret = 0;
	if (_tagLock(tagc)){
		TMETAITEM *item = _tagFindEntryByHash(tagc, hash);
		ret = (item && item->hasFilename);
		_tagUnlock(tagc);
	}
	return ret;
}

int tagIsTitleAvailableByHash (TMETATAGCACHE *tagc, const unsigned int hash)
{
	int ret = 0;
	if (_tagLock(tagc)){
		TMETAITEM *item = _tagFindEntryByHash(tagc, hash);
		ret = (item && item->hasTitle);
		_tagUnlock(tagc);
	}
	return ret;
}

int tagArtworkIsURLAvailableByHash (TMETATAGCACHE *tagc, const unsigned int hash)
{
	int ret = 0;
	if (_tagLock(tagc)){
		TMETAITEM *item = _tagFindEntryByHash(tagc, hash);
		ret = (item && item->hasArtworkURL);
		_tagUnlock(tagc);
	}
	return ret;
}

int tagArtworkIsArtworkAvailableByHash (TMETATAGCACHE *tagc, const unsigned int hash)
{
	int ret = 0;
	if (_tagLock(tagc)){
		TMETAITEM *item = _tagFindEntryByHash(tagc, hash);
		ret = (item && item->hasArt);
		_tagUnlock(tagc);
	}
	return ret;
}


TTAGIMG *g_tagArtworkGetByHash (TMETATAGCACHE *tagc, const unsigned int hash)
{
	//if (hash){
		TMETAITEM *item = _tagFindEntryByHash(tagc, hash);
		if (item) return item->art;
	//}
	return NULL;
}

TTAGIMG *tagArtworkGetByHash (TMETATAGCACHE *tagc, const unsigned int hash)
{
	if (_tagLock(tagc)){
		TTAGIMG *art = g_tagArtworkGetByHash(tagc, hash);
		_tagUnlock(tagc);
		return art;
	}
	return NULL;
}

char *tagRetrieveByHash (TMETATAGCACHE *tagc, const unsigned int hash, const int tagid, char *buffer, const size_t len)
{
	*buffer = 0;
	if (!len || !tagc || !hash || tagid < 0 || tagid >= MTAG_TOTAL) return NULL;

	if (_tagLock(tagc)){
		TMETAITEM *item = _tagFindEntryByHash(tagc, hash);
		if (item && item->tag[tagid])
			strncpy(buffer, item->tag[tagid], len);
		_tagUnlock(tagc);
	}
	return buffer;
}

char *tagRetrieve (TMETATAGCACHE *tagc, const char *path, const int tagid, char *buffer, const size_t len)
{
	*buffer = 0;
	if (!path) return buffer;
	return tagRetrieveByHash(tagc, getHash(path), tagid, buffer, len);
}

static void _tagFlushArt (TMETATAGCACHE *tagc)
{
	// free the art handles
	for (int i = 0; i < TAGIMGCACHESIZE; i++){
		if (tagc->art[i]){
			_tagArtworkDelete(tagc->art[i]);
			tagc->art[i] = NULL;
		}
	}
	
	// remove all references to artwork
	TMETARECORD *tag = _tagGetFirst(tagc);
	if (tag){
		do{
			if (tag->item){
				tag->item->art = NULL;
				tag->item->hasArt = 0;
				tag->item->hasFilename = 0;
			}
		}while((tag=tag->next));
	}
}

void tagFlushArt (TMETATAGCACHE *tagc)
{
	if (_tagLock(tagc)){
		_tagFlushArt(tagc);
		_tagUnlock(tagc);
	}
}

static void _tagFreeItem (TMETAITEM *item)
{
	for (int i = 0; i < MTAG_TOTAL; i++){
		if (item->tag[i])
			my_free(item->tag[i]);
	}
	if (item->path)
		my_free(item->path);
	my_free(item);
}

static void _tagFreeRecord (TMETARECORD *rec)
{
	_tagFreeItem(rec->item);
	my_free(rec);
}

static void _tagFlush (TMETATAGCACHE *tagc)
{
	TMETARECORD *rec = _tagGetFirst(tagc);
	if (rec){
		TMETARECORD *next;
		do{
			next = rec->next;
			_tagFreeRecord(rec);
		}while((rec=next));
	}
	tagc->first = NULL;
}

void tagFlush (TMETATAGCACHE *tagc)
{
	if (_tagLock(tagc)){
		_tagFlush(tagc);
		_tagUnlock(tagc);
	}
}

void tagFree (TMETATAGCACHE *tagc)
{
	_tagLock(tagc);
	_tagFlushArt(tagc);
	_tagFlush(tagc);
	CloseHandle(tagc->hMutex);
	my_free(tagc);
}

TMETATAGCACHE *tagNew ()
{
	TMETATAGCACHE *tagc = my_calloc(1, sizeof(TMETATAGCACHE));
	if (tagc)
		tagc->hMutex = CreateMutex(NULL, FALSE, NULL);
	return tagc;
}
