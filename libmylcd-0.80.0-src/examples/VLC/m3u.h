
// libmylcd - http://mylcd.sourceforge.net/
// An LCD framebuffer and text rendering API
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



#ifndef _M3U_H_
#define _M3U_H_

#define M3U_OPENREAD	1
#define M3U_OPENWRITE	2

typedef struct{
	TASCIILINE *al;
	int mode;
	FILE *hFile;
}TM3U;

TM3U *m3uNew ();
void m3uFree (TM3U *m3u);

int m3uOpen (TM3U *m3u, const wchar_t *name, const int action);
void m3uClose (TM3U *m3u);
int m3uReadPlaylist (TM3U *m3u, TPLAYLISTMANAGER *plm, PLAYLISTCACHE *plc, TMETATAGCACHE *tagc);
int m3uWritePlaylist (TM3U *m3u, PLAYLISTCACHE *plc, TMETATAGCACHE *tagc);

void setCurrentDirectory (const wchar_t *indir);

#endif

