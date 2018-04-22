
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


#include <windows.h>
#include "touch.h"





unsigned int __stdcall touchEventDispatcher (void *ptr)
{
	int i, qlen;
	TMYLCDTOUCH *tin = (TMYLCDTOUCH*)ptr;
	TMYLCDTOUCHLOC queue[MAXTINQUEUESIZE+1];		// FIFO
	
	while(!*tin->applState){
		if (WaitForSingleObject(tin->hEvent, 1000) == WAIT_OBJECT_0){
			if (!*tin->applState){
				if (WaitForSingleObject(tin->hLock, 1000) == WAIT_OBJECT_0){
					if (!*tin->applState && tin->queueSize < MAXTINQUEUESIZE){
						qlen = tin->queueSize;
						tin->queueSize = 0;
						memcpy(queue, tin->queue, qlen * sizeof(TMYLCDTOUCHLOC));
						ReleaseSemaphore(tin->hLock, 1, NULL);
						if (WaitForSingleObject(tin->hRenderLock, 1000) == WAIT_OBJECT_0){
							if (!*tin->applState){
								for (i = 0; i < qlen; i++)
									tin->callback.uFn(&queue[i].pos, queue[i].flags, queue[i].ptr);
							}
							ReleaseSemaphore(tin->hRenderLock, 1, NULL);
						}
					}else{
						ReleaseSemaphore(tin->hLock, 1, NULL);
						break;
					}
				}
			}
		}
	}
	_endthreadex(1);
	return 1;
}

// receive touch data from mylcd then add to FIFO to be handled by another thread
void touchIN (TTOUCHCOORD *pos, int flags, void *ptr)
{

	TMYLCDTOUCH *tin = (TMYLCDTOUCH*)ptr;
	if (*tin->applState)
		return;
		
	if (WaitForSingleObject(tin->hLock, 1000) == WAIT_OBJECT_0){
		if (!*tin->applState){
			if (tin->queueSize < MAXTINQUEUESIZE){
				memcpy(&tin->queue[tin->queueSize].pos, pos, sizeof(TTOUCHCOORD));
				tin->queue[tin->queueSize].flags = flags;
				tin->queue[tin->queueSize++].ptr = tin->callback.uPtr;
				SetEvent(tin->hEvent);
			}else{
				printf("touchIN(): fifo too short %i %i\n", tin->queueSize, MAXTINQUEUESIZE);
			}
		}
		ReleaseSemaphore(tin->hLock, 1, NULL);
	}
}

void startTouchDispatcherThread (TMYLCDTOUCH *tin, int *applState, void *fn, void *ptr)
{
	tin->applState = applState;
	tin->callback.uPtr =(void*)ptr;
	tin->callback.uFn = (void*)fn;
	tin->hEvent = CreateEvent(NULL, 0, 0, NULL);
	tin->hLock = CreateSemaphore(NULL, 1, 1, NULL);
	tin->hRenderLock = CreateSemaphore(NULL, 1, 1, NULL);
	tin->hThread = _beginthreadex(NULL, 0, touchEventDispatcher, tin, 0, &tin->threadID);
	
	// start libmylcd touch handling
	lSetDisplayOption(tin->mylcd.hw, tin->mylcd.did, lOPT_USBD480_TOUCHCBUSERPTR, (intptr_t*)tin);
	lSetDisplayOption(tin->mylcd.hw, tin->mylcd.did, lOPT_USBD480_TOUCHCB, (intptr_t*)touchIN);
}

void closeTouchDispatcherThread (TMYLCDTOUCH *tin)
{
	lSetDisplayOption(tin->mylcd.hw, tin->mylcd.did, lOPT_USBD480_TOUCHCB, 0);
	SetEvent(tin->hEvent);
	WaitForSingleObject((HANDLE)tin->hThread, 60000);
	CloseHandle((HANDLE)tin->hThread);
	CloseHandle(tin->hRenderLock);
	CloseHandle(tin->hEvent);
	CloseHandle(tin->hLock);
}


