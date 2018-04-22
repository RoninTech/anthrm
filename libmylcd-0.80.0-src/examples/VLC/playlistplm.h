
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



#ifndef _PLAYLISTPLM_H_
#define _PLAYLISTPLM_H_

typedef struct {
	int x;
	int y;
	int voffset;
	int startPosition;
	float aHScaled;
	
	int dragState;
	float artScale;				// 0 = playlist artwork display is disabled, anything else sets the scale
	uint64_t tOn;
	float decayTop;

	TFRAME *lineRender;
	TPLAYLISTART *artcache[PL2ARTCACHESIZE];
	int cacheIdx;
}TPLAYLISTPLM;

int closePlaylistPlm (TVLCPLAYER *vp, TFRAME *frame, TPLAYLISTPLM *pl);
int openPlaylistPlm (TVLCPLAYER *vp, TFRAME *frame, TPLAYLISTPLM *pl);
int drawPlaylistPlm (TVLCPLAYER *vp, TFRAME *frame, TPLAYLISTPLM *pl);
int touchPlaylistPlm (TTOUCHCOORD *pos, int flags, TVLCPLAYER *vp);


void playlistPlmSetRenderStart (TVLCPLAYER *vp, TPLAYLISTMANAGER *plm, int start);

#endif

