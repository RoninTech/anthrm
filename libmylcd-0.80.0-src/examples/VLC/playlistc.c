
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




/*

Note:
Underscored functions (_playlist####) do not pass through the lock, either directly or indirectly and
ant may not call a locked function (non underscored).

Non-underscored functions will contain the lock
*/

#include "common.h"

static inline int _playlistLock (PLAYLISTCACHE *plc)
{
	return (WaitForSingleObject(plc->hMutex, INFINITE) == WAIT_OBJECT_0);
}

static inline void _playlistUnlock (PLAYLISTCACHE *plc)
{
	ReleaseMutex(plc->hMutex);
}

int playlistLock (PLAYLISTCACHE *plc)
{
	return _playlistLock(plc);
}

void playlistUnlock (PLAYLISTCACHE *plc)
{
	_playlistUnlock(plc);
}

static inline TPLAYLISTRECORD *_playlistGetRoot (PLAYLISTCACHE *plc)
{
	return plc->first;
}

static TPLAYLISTRECORD *_playlistGetLast (PLAYLISTCACHE *plc)
{
	TPLAYLISTRECORD *rec = _playlistGetRoot(plc);
	if (rec){
		while(rec->next)
			rec = rec->next;
	}
	return rec;
}

static int _playlistInsertLast (PLAYLISTCACHE *plc, TPLAYLISTITEM *item)
{

	TPLAYLISTRECORD *newrec = (TPLAYLISTRECORD*)my_malloc(sizeof(TPLAYLISTRECORD));
	if (!newrec){
		//printf("playlistInsertLast(): out of memory\n");
		return -1;
	}
	
	newrec->next = NULL;
	newrec->item = item;
	
	TPLAYLISTRECORD *rec = _playlistGetLast(plc);
	if (rec)
		rec->next = newrec;
	else
		plc->first = newrec;

	return plc->total++;
}

static TPLAYLISTITEM *_playlistCreateItem (PLAYLISTCACHE *plc, const char *path)
{
	TPLAYLISTITEM *item = (TPLAYLISTITEM*)my_malloc(sizeof(TPLAYLISTITEM));
	if (item){
		item->path = my_strdup(path);
		item->hash = getHash(path);
		item->title = NULL;
	}
	return item;
}


// there can be multiple instances of the same track in a playlist
static int _playlistAdd (PLAYLISTCACHE *plc, const char *path)
{
	TPLAYLISTITEM *item = _playlistCreateItem(plc, path);
	if (item)
		return _playlistInsertLast(plc, item);
	else
		return -1;
}

int playlistAdd (PLAYLISTCACHE *plc, const char *path)
{
	int ret = 0;
	if (_playlistLock(plc)){
		ret = _playlistAdd(plc, path);
		_playlistUnlock(plc);
	}
	return ret;
}

static int _playlistGetTotal (PLAYLISTCACHE *plc)
{
	return plc->total;
}

// just print everything
/*
int playlistRunList (PLAYLISTCACHE *plc)
{
	if (!_playlistLock(plc))
		return 0;
		
	int total = 0;
	char *path;
	char *title;
	
	TPLAYLISTRECORD *rec = _playlistGetRoot(plc);
	if (rec){
		do{
			path = rec->item->path;
			if (!path) path = "no path";
			title = rec->item->title;
			if (!title) title = "no title";
			
			printf("%i: %X '%s' '%s'\n", total++, rec->item->hash, path, title);
		}while((rec=rec->next));
	}
	
	_playlistUnlock(plc);
	return total;
}
*/

static TPLAYLISTRECORD * _playlistGetRecord (PLAYLISTCACHE *plc, const int pos)
{
	if (pos >= 0){
		TPLAYLISTRECORD *rec = _playlistGetRoot(plc);
		if (rec){
			int idx = 0;
			do{
				if (idx == pos)
					return rec;
				idx++;
			}while((rec=rec->next));
		}
	}
	return NULL;
}

static int _playlistUnlinkRecord (PLAYLISTCACHE *plc, TPLAYLISTRECORD *recunlink)
{
	TPLAYLISTRECORD *rec = _playlistGetRoot(plc);
	if (rec){
		if (rec == recunlink){	// handle special case that be root
			plc->first = rec->next;
			return 1;
		}

		do{
			if (rec->next == recunlink){
				rec->next = recunlink->next;
				return 1;
			}
		}while((rec=rec->next));
	}
	return 0;
}

static int _playlistSearch_tag (PLAYLISTCACHE *plc, TMETATAGCACHE *tagc, const int tag, const char *str, const int from)
{
	TPLAYLISTRECORD *rec = _playlistGetRoot(plc);
	if (rec){
		char buffer[MAX_PATH_UTF8];
		int idx = 0;
		
		if (from > 0){
			do{
				if (idx == from) break;
				idx++;
			}while((rec=rec->next));
		}
		
		do{
			tagRetrieveByHash(tagc, rec->item->hash, tag, buffer, MAX_PATH_UTF8);
			if (*buffer){
				if (stristr(buffer, str))
					return idx;
			}
			idx++;
		}while((rec=rec->next));
	}
	return -1;
}

static int _playlistSearch_path (PLAYLISTCACHE *plc, const char *str, const int from)
{
	TPLAYLISTRECORD *rec = _playlistGetRoot(plc);
	if (rec){
		int idx = 0;
		
		if (from > 0){
			do{
				if (idx == from) break;
				idx++;
			}while((rec=rec->next));
		}
		
		do{
			if (stristr(rec->item->path, str))
				return idx;
			idx++;
		}while((rec=rec->next));
	}
	return -1;
}

static int _playlistSearch_title (PLAYLISTCACHE *plc, const char *str, const int from)
{
	TPLAYLISTRECORD *rec = _playlistGetRoot(plc);
	if (rec){
		int idx = 0;
		
		if (from > 0){
			do{
				if (idx == from) break;
				idx++;
			}while((rec=rec->next));
		}
		
		do{
			if (rec->item->title && stristr(rec->item->title, str))
				return idx;
			idx++;
		}while((rec=rec->next));
	}
	return -1;
}

int playlistSearch (PLAYLISTCACHE *plc, TMETATAGCACHE *tagc, const char *str, const int from)
{
	int ret = -1;
	if (_playlistLock(plc)){
		if (from >= 0 && from < _playlistGetTotal(plc)){
			ret = _playlistSearch_title(plc, str, from);
			if (ret == -1)
				ret = _playlistSearch_path(plc, str, from);
			if (ret == -1)
				ret = _playlistSearch_tag(plc, tagc, MTAG_Artist, str, from);
			if (ret == -1)
				ret = _playlistSearch_tag(plc, tagc, MTAG_Album, str, from);
			if (ret == -1)
				ret = _playlistSearch_tag(plc, tagc, MTAG_Genre, str, from);
			if (ret == -1)
				ret = _playlistSearch_tag(plc, tagc, MTAG_Description, str, from);	
			if (ret == -1)
				ret = _playlistSearch_tag(plc, tagc, MTAG_Date, str, from);
			if (ret == -1)
				ret = _playlistSearch_tag(plc, tagc, MTAG_LENGTH, str, from);
		}
		_playlistUnlock(plc);
	}
	return ret;
}

int playlistSearchTag (PLAYLISTCACHE *plc, TMETATAGCACHE *tagc, const int mtag, const char *str, const int from)
{
	int ret = -1;
	if (_playlistLock(plc)){
		if (from >= 0 && from < _playlistGetTotal(plc)){
			if (mtag == MTAG_PATH)
				ret = _playlistSearch_path(plc, str, from);
			else if (mtag == MTAG_Title)
				ret = _playlistSearch_title(plc, str, from);
			else if (mtag >= 0 && mtag < MTAG_TOTAL)
				ret = _playlistSearch_tag(plc, tagc, mtag, str, from);
		}			
		_playlistUnlock(plc);
	}
	return ret;
}
 
int playlistGetTotal (PLAYLISTCACHE *plc)
{
	int ret = 0;

	if (plc){
		if (_playlistLock(plc)){
			ret = _playlistGetTotal(plc);
			_playlistUnlock(plc);
		}
	}
	return ret;
}

static TPLAYLISTITEM *_playlistGetItem (PLAYLISTCACHE *plc, const int pos)
{
	int idx = 0;
	TPLAYLISTRECORD *rec = _playlistGetRoot(plc);
	while(rec){
		if (idx++ == pos) return rec->item;
		rec = rec->next;
	}
	return NULL;
}

TPLAYLISTITEM *playlistGetItem (PLAYLISTCACHE *plc, const int pos)
{
	TPLAYLISTITEM *ret = NULL;
	if (_playlistLock(plc)){
		ret = _playlistGetItem(plc, pos);
		_playlistUnlock(plc);
	}
	return ret;
}


static int _playlistSetTitle (PLAYLISTCACHE *plc, const int pos, const char *title, const int overwrite)
{
	TPLAYLISTITEM *item = _playlistGetItem(plc, pos);
	if (item){
		if (item->title){
			if (overwrite)
				my_free(item->title);
			else
				return 1;
		}
		item->title = my_strdup(title);
		return 1;
	}
	return 0;
}

int playlistSetTitle (PLAYLISTCACHE *plc, const int pos, const char *title, const int overwrite)
{
	int ret = 0;
	if (_playlistLock(plc)){
		ret = _playlistSetTitle(plc, pos, title, overwrite);
		_playlistUnlock(plc);
	}
	return ret;
}

static char *_playlistGetTitle (PLAYLISTCACHE *plc, const int pos, char *buffer, const size_t len)
{
	*buffer = 0;
	TPLAYLISTITEM *item = _playlistGetItem(plc, pos);
	if (item && item->title)
		strncpy(buffer, item->title, len);
	return buffer;
}

char *playlistGetTitle (PLAYLISTCACHE *plc, const int pos, char *buffer, const size_t len)
{
	char *ret = 0;
	if (_playlistLock(plc)){
		ret = _playlistGetTitle(plc, pos, buffer, len);
		_playlistUnlock(plc);
	}
	return ret;
}

static char *_playlistGetPath (PLAYLISTCACHE *plc, const int pos, char *buffer, const size_t len)
{
	*buffer = 0;
	TPLAYLISTITEM *item = _playlistGetItem(plc, pos);
	if (item && item->path)
		strncpy(buffer, item->path, len);
	return buffer;
}

char *playlistGetPath (PLAYLISTCACHE *plc, const int pos, char *buffer, const size_t len)
{
	char *ret = 0;
	if (_playlistLock(plc)){
		ret = _playlistGetPath(plc, pos, buffer, len);
		_playlistUnlock(plc);
	}
	return ret;
}

static int _playlistSetName (PLAYLISTCACHE *plc, char *buffer)
{
	strncpy(plc->name, buffer, MAX_PATH_UTF8);
	return 1;
}

int playlistSetName (PLAYLISTCACHE *plc, char *buffer)
{
	int ret = 0;
	if (_playlistLock(plc)){
		if (buffer && strlen(buffer) > 0)
			ret = _playlistSetName(plc, buffer);
		_playlistUnlock(plc);
	}
	return ret;
}

static char *_playlistGetName (PLAYLISTCACHE *plc, char *buffer, const size_t len)
{
	*buffer = 0;
	strncpy(buffer, plc->name, len);
	return buffer;
}

char *playlistGetName (PLAYLISTCACHE *plc, char *buffer, const size_t len)
{
	char *ret = 0;
	if (_playlistLock(plc)){
		ret = _playlistGetName(plc, buffer, len);
		_playlistUnlock(plc);
	}
	return ret;
}

static unsigned int _playlistGetHash (PLAYLISTCACHE *plc, const int pos)
{
	TPLAYLISTITEM *item = _playlistGetItem(plc, pos);
	if (item)
		return item->hash;
	else
		return 0;
}

unsigned int playlistGetHash (PLAYLISTCACHE *plc, const int pos)
{
	unsigned int ret = 0;
	if (_playlistLock(plc)){
		ret = _playlistGetHash(plc, pos);
		_playlistUnlock(plc);
	}
	return ret;
}

static int _playlistGetPositionByHash (PLAYLISTCACHE *plc, const unsigned int hash)
{
	int idx = 0;
	TPLAYLISTRECORD *rec = _playlistGetRoot(plc);
	while(rec){
		if (rec->item->hash == hash) return idx;
		idx++;
		rec = rec->next;
	}
	return -1;
}

int playlistGetPositionByHash (PLAYLISTCACHE *plc, const unsigned int hash)
{
	int ret = -1;
	if (_playlistLock(plc)){
		ret = _playlistGetPositionByHash(plc, hash);
		_playlistUnlock(plc);
	}
	return ret;
}

static void _playlistFreeItem (TPLAYLISTITEM *item)
{
	if (item->path) my_free(item->path);	
	if (item->title) my_free(item->title);
	my_free(item);
}

static void _playlistFreeRecord (TPLAYLISTRECORD *rec)
{
	_playlistFreeItem(rec->item);
	my_free(rec);
}

static int _playlistDeleteRecord (PLAYLISTCACHE *plc, const int pos)
{
	TPLAYLISTRECORD *rec = _playlistGetRecord(plc, pos);
	if (rec){
		if (_playlistUnlinkRecord(plc, rec)){
			_playlistFreeRecord(rec);
			plc->total--;
			return 1;
		}
	}
	return 0;
}

int playlistDeleteRecord (PLAYLISTCACHE *plc, const int pos)
{
	int ret = 0;
	if (_playlistLock(plc)){
		ret = _playlistDeleteRecord(plc, pos);
		_playlistUnlock(plc);
	}
	return ret;
}

int _playlistPrune (PLAYLISTCACHE *plc, const int remove)
{
	int ct = 0;
	TPLAYLISTRECORD tmp;
	TPLAYLISTRECORD *rec = _playlistGetRoot(plc);
	
	if (rec){
		do{
			if (rec && rec->item){
				if (!doesFileExistUtf8(rec->item->path)){
					printf("0 '%s'\n", rec->item->path);
					if (remove){
						tmp.next = rec->next;
						if (_playlistUnlinkRecord(plc, rec)){
							_playlistFreeRecord(rec);
							plc->total--;
						}
						rec = &tmp;
					}
					ct++;
				}
			}
		}while((rec=rec->next));
	}
	return ct;
}

int playlistPrune (PLAYLISTCACHE *plc, const int remove)
{
	int ret = 0;
	if (_playlistLock(plc)){
		ret = _playlistPrune(plc, remove);
		_playlistUnlock(plc);
	}
	return ret;
}

static void _playlistDelete (PLAYLISTCACHE *plc)
{
	if (!plc->total) return;
		
	TPLAYLISTRECORD *rec = _playlistGetRoot(plc);
	if (rec){
		TPLAYLISTRECORD *next;
		do{
			next = rec->next;
			_playlistFreeRecord(rec);
		}while((rec=next));
	}
	plc->first = NULL;
	plc->total = 0;
}

void playlistDelete (PLAYLISTCACHE *plc)
{
	if (_playlistLock(plc)){
		_playlistDelete(plc);
		_playlistUnlock(plc);
	}
}

void playlistFree (PLAYLISTCACHE *plc)
{
	if (_playlistLock(plc)){
		//printf("## playlistFree: '%s'\n", plc->name);
		
		_playlistDelete(plc);
		CloseHandle(plc->hMutex);
		my_free(plc->pr);
		my_free(plc);
	}
}

PLAYLISTCACHE *playlistNew (const char *name)
{
	PLAYLISTCACHE *plc = my_calloc(1, sizeof(PLAYLISTCACHE));
	if (plc){
		plc->hMutex = CreateMutex(NULL, FALSE, NULL);
		plc->pr = my_calloc(1, sizeof(PLAYLISTRENDER));
		plc->pr->selectedItem = -1;
		plc->pr->playingItem = -1;
		plc->pr->playingItemPrevious = -2;

		plc->first = NULL;
		plc->total = 0;
		strncpy(plc->name, name, MAX_PATH_UTF8);
	}
	return plc;
}
