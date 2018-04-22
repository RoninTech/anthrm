
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



#include <stdio.h>
#include <stdlib.h>

#include "mylcd.h"
#include "demos.h"
#include "vlc/plugins/vlc11/svmem.h"


typedef struct {
	int width;
	int height;
	int bpp;
	TFRAME *frame;
	
	HANDLE hMapFile;
	uint8_t *hMem;
}TSIMAGE;

const char bpplookup[] = {1, 8, 12, 15, 16, 24, 32, 32};


static int getBpp (int bpp)
{
	int i;
	for (i = 0; i < 7; i++){
		if (bpplookup[i] == bpp)
			return i;
	}
	return LFRM_BPP_32;
}

// initiate VLC playback before calling this
static int openSharedMemory (TSIMAGE *img, const char *name)
{
	img->hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, TRUE, VLC_SMEMNAME);
	if (img->hMapFile != NULL){
		img->hMem = (uint8_t*)MapViewOfFile(img->hMapFile, FILE_MAP_ALL_ACCESS, 0,0,0);
		if (img->hMem != NULL){
			printf("MapViewOfFile() ok, %p\n",img->hMem);
			return 1;
		}else{
			printf("MapViewOfFile() failed with error %d\n", (int)GetLastError());
			CloseHandle(img->hMapFile);
		}
	}else{
		printf("OpenFileMapping() failed with error %d\n", (int)GetLastError());
	}
	return 0;
}

	
static void closeSharedMemory (TSIMAGE *img)
{
	UnmapViewOfFile(img->hMem);
	CloseHandle(img->hMapFile);
}
			
int main (int argc, char* argv[])
{

	if (!initDemoConfig("config.cfg"))
		return 0;

	HANDLE hUpdateEvent = NULL;
	HANDLE hDataLock = NULL;
	TSVMEM *svmem = NULL;
	TSIMAGE img;

	if (openSharedMemory(&img, VLC_SMEMNAME)){
		svmem = (TSVMEM*)img.hMem;
		
		// VLC will set a global event, VLC_SMEMEVENT, on each update
		hUpdateEvent = CreateEvent(NULL, 0, 0, VLC_SMEMEVENT);
		
		// data access is synchronized through a semaphore 
		hDataLock = CreateSemaphore(NULL, 0, 1, VLC_SMEMLOCK);

		img.width = -1;
		img.height = -1;
		img.bpp = -1;
		img.frame = lNewFrame(hw, 8, 8, getBpp(24));
		//uint32_t ftime = 0;
		
		// tell VLC we want a width/height of 320/240
		// playback may need to be restarted for any change to these values to take effect
		svmem->hdr.rwidth = DWIDTH;
		svmem->hdr.rheight = DHEIGHT;
		
		while(!kbhit()){
			// wait for the frame ready signal from the plugin
			if (WaitForSingleObject(hUpdateEvent, 2000) == WAIT_OBJECT_0){
				
				//printf("%i %u\n", svmem->hdr.count, svmem->hdr.time-ftime);
				//ftime = svmem->hdr.time;

				// lock the IPC. 
				// because this also blocks VLC from updating the buffer, intensive operations
				// should be avoided inside of the lock.
				// best to copy out the video frame, release the semaphore then perform your ops on the buffer.
				if (WaitForSingleObject(hDataLock, 1000) == WAIT_OBJECT_0){
					if (svmem->hdr.count < 2)
						lRefresh(frame); // clear display

					// always verify bpp, width and height as they're likely to change between playbacks
					if (img.bpp != svmem->hdr.bpp){
						img.bpp = svmem->hdr.bpp;
						img.width = svmem->hdr.width;
						img.height = svmem->hdr.height;
						lDeleteFrame(img.frame);
						img.frame = lNewFrame(hw, img.width, img.height, getBpp(img.bpp));
					}else if (img.width != svmem->hdr.width || img.height != svmem->hdr.height){
						lClearFrame(img.frame);	// clear front buffer
						lRefreshAsync(img.frame, 1);	// swap buffers
						lClearFrame(img.frame);	// clear front buffer (was back buffer before the swap)
						memset((void*)&svmem->pixels, 0, svmem->hdr.vsize);
						
						img.width = svmem->hdr.width;
						img.height = svmem->hdr.height;
						lResizeFrame(img.frame, img.width, img.height, 0);
					}

					// copy the vlc image
					memcpy(lGetPixelAddress(img.frame,0,0), (void*)&svmem->pixels, svmem->hdr.fsize);
					ReleaseSemaphore(hDataLock, 1, NULL);
					
					// do processing
					lRefreshAsync(img.frame, 1);
				}
			}else{
				printf("sleeping..\n");
				lSleep(30);
			}
		}
		lDeleteFrame(img.frame);
		CloseHandle(hUpdateEvent);
		CloseHandle(hDataLock);
		closeSharedMemory(&img);	
	}

	demoCleanup();
	return 1;
}
