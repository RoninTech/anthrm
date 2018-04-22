
// libmylcd
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
//  GNU LIBRARY GENERAL PUBLIC LICENSE for details.

#ifndef _TOUCH_H_
#define _TOUCH_H_

#include "mylcd.h"

#define MAXTINQUEUESIZE 256


typedef struct{
	TTOUCHCOORD	pos;
	int flags;
	void *ptr;
}TMYLCDTOUCHLOC;

typedef struct{
	TMYLCDTOUCHLOC queue[MAXTINQUEUESIZE];		// FIFO touch input buffer
	int queueSize;	// number of unprocessed items in queue, not the size of the buffer 
	
	uintptr_t hThread;
	HANDLE hEvent;		// signal 
	HANDLE hLock;		// locks the queue
	HANDLE hRenderLock;	// prevent render and input thread atemptting simultaneous writes
	unsigned int threadID;
	int *applState;

	struct _callback{
		void (*uFn) (TTOUCHCOORD *pos, int flags, void *uptr);
		void *uPtr;
	}callback;
	
	struct _mylcd{
		THWD *hw;
		int did;
	}mylcd;
}TMYLCDTOUCH;



void startTouchDispatcherThread (TMYLCDTOUCH *tin, int *applState, void *fn, void *ptr);
void closeTouchDispatcherThread (TMYLCDTOUCH *tin);

#endif

