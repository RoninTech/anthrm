
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



#ifndef _PLAYLIST2_H_
#define _PLAYLIST2_H_


int closePlaylist2  (TVLCPLAYER *vp, TFRAME *frame, TPLAYLIST2 *pl);
int openPlaylist2 (TVLCPLAYER *vp, TFRAME *frame, TPLAYLIST2 *pl);
int drawPlaylist2 (TVLCPLAYER *vp, TFRAME *frame, TPLAYLIST2 *pl);
int touchPlaylist2 (TTOUCHCOORD *pos, int flags, TVLCPLAYER *vp);

void playlist2SetStartTrack (TPLAYLIST2 *pl, PLAYLISTCACHE *plc, const int start);
void cycleArtSize (TPLAYLIST2 *pl);
int printLine (TFRAME *frame, TFRAME *tmp, const char *buffer, const int x, const int y);

#endif

