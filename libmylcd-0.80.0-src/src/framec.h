
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

#ifndef _FRAMEC_H_
#define _FRAMEC_H_

typedef struct TFRAMERECORD TFRAMERECORD;

typedef struct{
	int		groupId;
	TFRAME	*frame;
}TFRAMEITEM;

struct TFRAMERECORD{
	TFRAMEITEM *item;
	TFRAMERECORD *prev;
	TFRAMERECORD *next;
};

typedef struct{
	TFRAMERECORD *first;
	unsigned int linkCount;
	unsigned int oldCount;
	int frameCount;
	int frameRescueCount;
	int frameDeleteCount;
	
	void *hMutex;
}TFRAMECACHE;


TFRAMECACHE *framecNew ();
void framecDelete (TFRAMECACHE *frmc);

int frameAdd (TFRAMECACHE *frmc, TFRAME *frame, const int gid);
void frameDeleteAllByGroup (TFRAMECACHE *frmc, const int gid);
void frameDeleteDelink (TFRAMECACHE *frmc, TFRAME *frame);
int frameGetTotal (TFRAMECACHE *frmc);

#endif

