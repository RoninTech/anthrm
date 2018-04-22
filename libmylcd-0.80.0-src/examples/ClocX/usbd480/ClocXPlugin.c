
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
#include "mylcd.h"

#include "mylcdsetup.h"
#include "ClocXPlugin.h"


typedef struct{
	HWND WndHandle;
	HDC hDCSrc;
	HDC hDCMemory;
	HBITMAP hBmp;
	HBITMAP hBmpPrev;
	RECT rc;
	int WidthSrc;
	int HeightSrc;
	TFRAME *tmp;
	char *image;
	POINT zc;
}TGDICAP;
TGDICAP *sc = NULL;



static int dibWidthPadded (int width)
{
	if (width&3)
		return ((width>>2)<<2) + 4;
	else 
		return width;
}

static void setPixel16_NBr (const TFRAME *frm, const int x, const int row, const int value)
{
	*(uint16_t*)(frm->frame+(row+(x*2))) = value;
}

static void _32bppDib16bppFrame (TFRAME *frame, ubyte *data, int Width, int Height)
{
	int LineWidth = dibWidthPadded(Width<<2);
	ubyte *dib = data;
	int r,g,b;
	int x,y, row;

	for (y = 0; y < Height; y++){
		dib = data + (y * LineWidth);
		row = y * frame->pitch;
		for (x = 0; x < Width; x++){
			b = (*dib++ &0xF8) >> 3;
			g = (*dib++ &0xFC) << 3;
			r = (*dib++ &0xF8) << 8;
			setPixel16_NBr(frame, x, row, r|g|b);
			dib++;
		}
	}
}

static void createBmp (HDC hdc)
{
	if (sc->hBmp)
		DeleteObject(sc->hBmp);
	if (sc->hDCMemory)
    	DeleteDC(sc->hDCMemory);
	sc->hDCSrc = hdc;    	
	sc->hDCMemory = CreateCompatibleDC(sc->hDCSrc);
	sc->hBmp = CreateCompatibleBitmap(sc->hDCSrc, sc->WidthSrc, sc->HeightSrc);
}


BOOL APIENTRY DllMain (HINSTANCE hinstDLL, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call){
	  case DLL_PROCESS_ATTACH:
	  case DLL_THREAD_ATTACH:
	  
	  case DLL_THREAD_DETACH:
	  case DLL_PROCESS_DETACH:
		break;
	}
    return TRUE;
}


// OnInit() is called when module is loaded  (but not when it is loaded only for configuration
EXPORT int OnInit (InitData *idat)
{
	if (sc == NULL){
		if (!initMYLCD_USBD480())
			return 1;
	}

	sc = (TGDICAP*)calloc(1, sizeof(TGDICAP));
	sc->image = (char*)calloc(sizeof(unsigned int), DWIDTH * DHEIGHT);
	sc->WndHandle = idat->MainWindow;
	sc->tmp = lCloneFrame(frame);
	return (sc->tmp == NULL);  /* 0 = OK,  1 = Error,  -1 = OK but unload this plugin */
}

EXPORT void OnSkinLoaded (SkinLoadedData *sld)
{
	sc->WidthSrc = sld->Width;
	sc->HeightSrc = sld->Height;
	memcpy(&sc->zc, &sld->ZoomedCenter, sizeof(POINT));
	if (sc->image)
		free(sc->image);
	sc->image = (char*)calloc(sizeof(int), sc->WidthSrc * sc->HeightSrc);

	lClearFrame(sc->tmp);
	lRefresh(sc->tmp);
}

EXPORT void OnDrawedImage (HDC hdc)
{

	ShowWindow(sc->WndHandle, SW_HIDE);

	if (sc->hDCSrc != hdc)
		createBmp(hdc);

    if (!sc->hBmpPrev)
   		sc->hBmpPrev = (HBITMAP)SelectObject(sc->hDCMemory, sc->hBmp);
   	else
   		SelectObject(sc->hDCMemory, sc->hBmp);

    BitBlt(sc->hDCMemory, 0, 0, sc->WidthSrc, sc->HeightSrc, sc->hDCSrc, 0, 0, SRCCOPY|CAPTUREBLT);
    GetBitmapBits(sc->hBmp, sc->WidthSrc * sc->HeightSrc * 4, sc->image);
	if (frame->width != sc->WidthSrc || frame->height != sc->HeightSrc)
		lResizeFrame(frame, sc->WidthSrc, sc->HeightSrc, 0);

	_32bppDib16bppFrame(frame, (ubyte*)sc->image, sc->WidthSrc, sc->HeightSrc);

	int left, top;
	int x, y;
	int x2, y2;
	
	if (frame->width < sc->tmp->width)
		left = abs(frame->width - sc->tmp->width)/2;
	else
		left = 0;
	if (frame->height < sc->tmp->height)
		top = abs(frame->height - sc->tmp->height)/2;
	else
		top = 0;

	x = sc->zc.x - (sc->tmp->width>>1);
	if (x < 0) x = 0;
	y = sc->zc.y - (sc->tmp->height>>1);
	if (y < 0) y = 0;
	
	x2 = x+frame->width-1;
	if (x2 > x+sc->tmp->width-1)
		x2 = x+sc->tmp->width-1;
	y2 = y+frame->height-1;
	if (y2 > y+sc->tmp->height-1)
		y2 = y+sc->tmp->height-1;

	lCopyArea(frame, sc->tmp, left, top, x, y, x2, y2);
	lRefresh(sc->tmp);
}

// OnDestroy is called on ClocX exit
EXPORT void OnDestroy ()
{
	//lSaveImage(sc->tmp, L"1.png", IMG_PNG, 0, 0);
	
	if (sc){
    	if (sc->hBmp)
			DeleteObject(sc->hBmp);
		if (sc->hBmpPrev)
			DeleteObject(sc->hBmpPrev);
		if (sc->hDCMemory)
    		DeleteDC(sc->hDCMemory);
	    if (sc->image)
	    	free(sc->image);
    	if (sc->tmp)
	   		lDeleteFrame(sc->tmp);
    	free(sc);
    	sc = NULL;
	}
	closeMYLCD_USBD480();
}

