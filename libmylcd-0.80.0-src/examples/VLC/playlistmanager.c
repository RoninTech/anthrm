
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




static int _playlistManagerLock (TPLAYLISTMANAGER *plm)
{
	return (WaitForSingleObject(plm->hMutex, INFINITE) == WAIT_OBJECT_0);
}

static void _playlistManagerUnlock (TPLAYLISTMANAGER *plm)
{
	ReleaseMutex(plm->hMutex);
}

int playlistManagerLock (TPLAYLISTMANAGER *plm)
{
	return _playlistManagerLock(plm);
}

void playlistManagerUnlock (TPLAYLISTMANAGER *plm)
{
	_playlistManagerUnlock(plm);
}

static PLAYLISTCACHE *_playlistManagerGetPlaylistByName (TPLAYLISTMANAGER *plm, const char *name)
{
	if (name){
		for (int i = 0; i < plm->total; i++){
			if (plm->plc[i]){
				if (!strcmp(plm->plc[i]->name, name))
					return plm->plc[i];
			}
		}
	}
	return NULL;
}

PLAYLISTCACHE *playlistManagerGetPlaylistByName (TPLAYLISTMANAGER *plm, const char *name)
{
	if (_playlistManagerLock(plm)){
		PLAYLISTCACHE *ret = _playlistManagerGetPlaylistByName(plm, name);
		_playlistManagerUnlock(plm);
		return ret;
	}
	return NULL;
}

int playlistManagerGetTotal (TPLAYLISTMANAGER *plm)
{
	int ret = 0;
	if (_playlistManagerLock(plm)){
		for (int i = 0; i < plm->total; i++)
			if (plm->plc[i]) ret++;
		_playlistManagerUnlock(plm);
	}
	return ret;
}

static int _playlistManagerGetPlaylistPrev (TPLAYLISTMANAGER *plm, const int plIdx)
{
	for (int i = plIdx-1; i >= 0; i--)
		if (plm->plc[i]) return i;
	return -1;
}

int playlistManagerGetPlaylistPrev (TPLAYLISTMANAGER *plm, const int plIdx)
{
	if (_playlistManagerLock(plm)){
		int ret = _playlistManagerGetPlaylistPrev(plm, plIdx);
		_playlistManagerUnlock(plm);
		return ret;
	}
	return -1;
}

static int _playlistManagerGetPlaylistNext (TPLAYLISTMANAGER *plm, const int plIdx)
{
	for (int i = plIdx+1; i < plm->total; i++)
		if (plm->plc[i]) return i;
	return -1;
}

int playlistManagerGetPlaylistNext (TPLAYLISTMANAGER *plm, const int plIdx)
{
	if (_playlistManagerLock(plm)){
		int ret = _playlistManagerGetPlaylistNext(plm, plIdx);
		_playlistManagerUnlock(plm);
		return ret;
	}
	return -1;
}

static PLAYLISTCACHE *_playlistManagerGetPlaylist (TPLAYLISTMANAGER *plm, const int plIdx)
{
	if (plIdx >= 0 && plIdx < plm->total)
		return plm->plc[plIdx];
	else
		return NULL;
}

PLAYLISTCACHE *playlistManagerGetPlaylist (TPLAYLISTMANAGER *plm, const int plIdx)
{
	if (_playlistManagerLock(plm)){
		PLAYLISTCACHE *ret = _playlistManagerGetPlaylist(plm, plIdx);
		_playlistManagerUnlock(plm);
		return ret;
	}
	return NULL;
}

static int _playlistManagerGetPlaylistIndex (TPLAYLISTMANAGER *plm, PLAYLISTCACHE *plc)
{
	if (plc){
		for (int i = 0; i < plm->total; i++){
			if (plm->plc[i] == plc)
				return i;
		}
	}
	return -1;
}

int playlistManagerGetPlaylistIndex (TPLAYLISTMANAGER *plm, PLAYLISTCACHE *plc)
{
	if (_playlistManagerLock(plm)){
		int ret = _playlistManagerGetPlaylistIndex(plm, plc);
		_playlistManagerUnlock(plm);
		return ret;
	}
	return -1;
}

static PLAYLISTCACHE *_playlistManagerAlloc (TPLAYLISTMANAGER *plm, const char *name)
{
	for (int i = 0; i < plm->total; i++){
		if (!plm->plc[i]){
			plm->plc[i] = playlistNew(name);
			return plm->plc[i];
		}
	}
	
	PLAYLISTCACHE **tmp = (PLAYLISTCACHE**)my_realloc(plm->plc, (plm->total+1) * sizeof(PLAYLISTCACHE**));
	if (tmp){
		plm->plc = tmp;
		plm->total++;
		plm->plc[plm->total-1] = playlistNew(name);
		return plm->plc[plm->total-1];
	}else{
		return NULL;
	}
}

PLAYLISTCACHE *playlistManagerCreatePlaylist (TPLAYLISTMANAGER *plm, const char *name)
{
	if (_playlistManagerLock(plm)){
		PLAYLISTCACHE *ret = _playlistManagerGetPlaylistByName(plm, name);
		if (!ret)
			ret = _playlistManagerAlloc(plm, name);
		_playlistManagerUnlock(plm);
		return ret;
	}
	return NULL;
}

static int _playlistManagerDeletePlaylist (TPLAYLISTMANAGER *plm, PLAYLISTCACHE *plc)
{
	if (plc){
		for (int i = 0; i < plm->total; i++){
			if (plm->plc[i] == plc){
				playlistFree(plm->plc[i]);
				my_memcpy(&plm->plc[i], &plm->plc[i+1], (plm->total-i)*sizeof(PLAYLISTCACHE*));
				return 1;
			}
		}
	}
	return 0;
}

int playlistManagerDeletePlaylist (TPLAYLISTMANAGER *plm, PLAYLISTCACHE *plc)
{
	if (_playlistManagerLock(plm)){
		int ret = _playlistManagerDeletePlaylist(plm, plc);
		if (ret) plm->total--;
		_playlistManagerUnlock(plm);
		return ret;
	}
	return 0;
}

static void _playlistManagerFree (TPLAYLISTMANAGER *plm)
{
	if (!plm) return;

	if (plm->plc){
		while(plm->total--){
			if (plm->plc[plm->total])
				playlistFree(plm->plc[plm->total]);
		}
		my_free(plm->plc);
	}
	
	plm->plc = NULL;
	plm->total = 0;
}

void playlistManagerDelete (TPLAYLISTMANAGER *plm)
{
	_playlistManagerLock(plm);
	_playlistManagerFree(plm);
	CloseHandle(plm->hMutex);
	my_free(plm->pr);
	my_free(plm);
}

TPLAYLISTMANAGER *playlistManagerNew ()
{
	TPLAYLISTMANAGER *plm = my_calloc(1, sizeof(TPLAYLISTMANAGER));
	if (plm){
		plm->hMutex = CreateMutex(NULL, FALSE, NULL);
		plm->total = 1;
		plm->plc = (PLAYLISTCACHE**)my_calloc(plm->total, sizeof(PLAYLISTCACHE**));
		
		plm->pr = my_calloc(1, sizeof(PLAYLISTRENDER));
		plm->pr->selectedItem = -1;
		plm->pr->playingItem = -1;
		plm->pr->playingItemPrevious = -2;

	}
	return plm;
}

