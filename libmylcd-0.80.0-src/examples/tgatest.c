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
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include "mylcd.h"
#include "demos.h"


typedef struct{
	int id;
	int height;
	int width;
	wchar_t file[MaxPath];
}TFID;

wchar_t pathtemp[MaxPath];

static INLINE void sortDescendingByHeight (TFID *fid, int total);
static INLINE void sortDescendingByWidth (TFID *fid, int total);
static INLINE void swapu32 (int *a, int *b);


int main (int argc, char* argv[])
{
	if (!initDemoConfig("config.cfg"))
		return 0;
    
    
	lSetBackgroundColour(hw, lGetRGBMask(frame, LMASK_WHITE));
	lSetForegroundColour(hw, lGetRGBMask(frame, LMASK_BLACK));
    
	char txt[] = "abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ";// 1234567890";// !\"œ$%^&*()-=_+[]{};'#:~,./<>?\\|`ª ¡‚¢£";
	char *txt2 = (char*)calloc(4096, sizeof(char));
	if (!txt2){
		demoCleanup();
		return 0;
	}
	int x=0,y=0;
	int w=0,h=0;
	int total;

	TENUMFONT *enf = lEnumFontsBeginA(hw);
	if (enf == NULL){
		free(txt2);
		demoCleanup();
		return 0;
	}
	
	total = enf->total;
	TFID fid[total];
	TFID *pfid = fid;
	
	do{
		sprintf(txt2,"%i %s", enf->id, /*enf->font->file,*/ txt);
		lGetTextMetrics(hw, txt2, 0, enf->id, &w, &h);
		printf("%i \n",enf->id/*, enf->font->file*/);
		y += h+1;
		if (w>x) x=w;
		
		wcscpy(pfid->file, enf->font->file);
		pfid->id = enf->id;
		pfid->width = w;
		pfid->height = h;
		pfid++;

		//lFlushFont(hw, enf->id);
	}while(lEnumFontsNextA(enf));
	lEnumFontsDeleteA(enf);

	if (argc>1){
		if (*argv[1] == '-') argv[1]++;
		if (*argv[1] == 'w' || *argv[1] == 'W')
			sortDescendingByWidth(fid, total);
		else
			sortDescendingByHeight(fid, total);
	}

	lResizeFrame(frame,++x,(++y)+total, 0);
	pfid = fid;
	y = 0;

	do{
		sprintf((char *)txt2,"%i %s",pfid->id, /*pfid->file,*/ txt);
		lPrintf(frame, 0, ++y, pfid->id, LPRT_CPY, txt2);

		y += pfid->height+1;
		pfid++;
	}while (--total);
	
    lRefresh(frame);
	printf("\nsaving image.. w:%i h:%i...",frame->width, frame->height);
	lSaveImage(frame, L"tgafonts.tga", IMG_TGA, frame->width, frame->height);
	printf(" done\n");
	
	// scroll frame through display
	x=0,y=0;
	int dirx=1;
	int diry=1;
	int right = (displays[0].right - displays[0].left);
	int btm = (displays[0].btm - displays[0].top);
	
	do{
	
    	displays[0].left = x;
       	displays[0].top = y;
    	displays[0].right = displays[0].left + right;
    	displays[0].btm = displays[0].top + btm;

    	lRefresh(frame);
		//lRefreshArea(frame, display[0].left, display[0].top, display[0].right, display[0].btm);
		//lRefreshArea(frame, x, y, x+width, y+height);
		//lSleep(1);		

		if (dirx) x++; else x--;
		if (x>600) dirx=0;
		else if (!x) dirx=1;
 
		if (diry) y++; else y--;
		if (y>(frame->height-btm)) diry=0;
		else if (!y) diry=1;

	}while ((x<(frame->width-right)) && !kbhit());

	free(txt2);
	demoCleanup();
	return 0;
}

static INLINE void swapu32 (int *a, int *b)
{
	*a ^= *b;
	*b ^= *a;
	*a ^= *b;
}

static INLINE void sortDescendingByWidth (TFID *fid, int total)
{
	if (!total) return;
	int j;
	int i=--total;
	
	while(i--){
		for (j=0; j<total; j++)
			if (fid[j].width < fid[j+1].width){
				swapu32(&fid[j].id,&fid[j+1].id);
				swapu32(&fid[j].height,&fid[j+1].height);
				swapu32(&fid[j].width,&fid[j+1].width);
				wcscpy(pathtemp, fid[j+1].file);
				wcscpy(fid[j+1].file, fid[j].file);
				wcscpy(fid[j].file, pathtemp);
			}
	}
}

static INLINE void sortDescendingByHeight (TFID *fid, int total)
{
	if (!total) return;
	int j;
	int i=--total;
	
	while(i--){
		for (j=0; j<total; j++)
			if (fid[j].height < fid[j+1].height){
				swapu32(&fid[j].id,&fid[j+1].id);
				swapu32(&fid[j].height,&fid[j+1].height);
				swapu32(&fid[j].width,&fid[j+1].width);
				wcscpy(pathtemp, fid[j+1].file);
				wcscpy(fid[j+1].file, fid[j].file);
				wcscpy(fid[j].file, pathtemp);
			}
	}
}


