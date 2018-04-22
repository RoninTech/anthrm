
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



#ifndef _PLAYLISTMANAGER_H_
#define _PLAYLISTMANAGER_H_


typedef struct{
	PLAYLISTCACHE **plc;
	int total;
	HANDLE hMutex;
	
	PLAYLISTRENDER *pr;
}TPLAYLISTMANAGER;

void playlistManagerUnlock (TPLAYLISTMANAGER *plm);
int playlistManagerLock (TPLAYLISTMANAGER *plm);

PLAYLISTCACHE *playlistManagerCreatePlaylist (TPLAYLISTMANAGER *plm, const char *name);
int playlistManagerDeletePlaylist (TPLAYLISTMANAGER *plm, PLAYLISTCACHE *plc);

TPLAYLISTMANAGER *playlistManagerNew ();
void playlistManagerDelete (TPLAYLISTMANAGER *plm);

int playlistManagerGetTotal (TPLAYLISTMANAGER *plm);

PLAYLISTCACHE *playlistManagerGetPlaylistByName (TPLAYLISTMANAGER *plm, const char *name);
PLAYLISTCACHE *playlistManagerGetPlaylist (TPLAYLISTMANAGER *plm, const int plIdx);
int playlistManagerGetPlaylistPrev (TPLAYLISTMANAGER *plm, const int plIdx);
int playlistManagerGetPlaylistNext (TPLAYLISTMANAGER *plm, const int plIdx);
int playlistManagerGetPlaylistIndex (TPLAYLISTMANAGER *plm, PLAYLISTCACHE *plc);

#endif

