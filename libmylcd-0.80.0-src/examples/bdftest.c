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
	
    wchar_t txt[] = L"abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ 0123456789";
    wchar_t *txt2 = (wchar_t*)calloc(4096, sizeof(wchar_t));
	if (!txt2){
		demoCleanup();
		return 0;
	}
	int x=0,y=0;
	int w,h;
	int total;

	TENUMFONT *enf = lEnumFontsBeginW(hw);
	if (enf == NULL){
		demoCleanup();
		return 0;
	}

	total = enf->total;
	TFID fid[total];
	TFID *pfid = fid;

	lSetCharacterEncoding(hw, CMT_UTF16);
	do{
		w = 0;
		h = 0;
		swprintf(txt2, L"%i %s %s", enf->id, enf->wfont->file, txt);
		lGetTextMetrics(hw, (char*)txt2, PF_DONTFORMATBUFFER, enf->id, &w,&h);
		
		wprintf(L"%i %s   \n",enf->id, enf->wfont->file);
		printf("Font:%s  \n",enf->wfont->fontName);
		printf("Registry:%s  \n",enf->wfont->CharsetRegistry);
		printf("Family:%s  \n",enf->wfont->FamilyName);
		printf("Glyphs in font:%i  \n",enf->wfont->GlyphsInFont);
		printf("Glyphs built:%i  \n",enf->wfont->CharsBuilt);
		printf("Default Char:%i  \n",enf->wfont->DefaultChar);
		printf("Text w,h:%i,%i   \n\n",w,h);

		y += h;
		if (w>x) x=w;
		
		wcscpy(pfid->file, enf->wfont->file);
		pfid->id = enf->id;
		pfid->width = w;
		pfid->height = h;
		pfid++;
		
	}while(lEnumFontsNextW(enf));
	lEnumFontsDeleteW(enf);
	
	if (argc>1){
		if (*argv[1] == '-') argv[1]++;
		if (*argv[1] == 'w' || *argv[1] == 'W')
			sortDescendingByWidth(fid, total);
		else
			sortDescendingByHeight(fid, total);
	}

	lResizeFrame(frame, ++x, (++y)+enf->total, 0);
	
	pfid = fid;
	y = 0;
	//ubyte buffer[512];
	
	do{
		swprintf(txt2, L"%i %s %s",pfid->id, pfid->file, txt);
		lPrint(frame, (char*)txt2, 0, y++, pfid->id, LPRT_CPY);

		lFlushFont(hw, pfid->id);
		y += pfid->height;
		pfid++;

	}while (--total);

	printf("\nsaving image.. w:%i h:%i...",frame->width, frame->height);
	lSaveImage(frame, L"bdffonts.tga", IMG_TGA, frame->width, frame->height);
	printf(" done\n");

	// scroll frame
	x=0,y=0;
	int dirx=1;
	int diry=1;
	int width = displays[0].right - displays[0].left;
	int height = displays[0].btm - displays[0].top;
	
	do{
    	displays[0].left = x;
       	displays[0].top = y;
    	displays[0].right = displays[0].left + width;
    	displays[0].btm = displays[0].top + height;
    	lRefresh(frame);
		//lRefreshArea(frame, display1.left, display1.top, display1.right, display1.btm);
		//lSleep(1);

		if (dirx) x++; else x--;
		if (x>600) dirx=0;
		else if (!x) dirx=1;

		if (diry) y++; else y--;
		if (y>(frame->height-height)) diry=0;
		else if (!y) diry=1;

	}while ((x<(frame->width-width)) && !kbhit());

	free(txt2);
	demoCleanup();
	return 1;
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

