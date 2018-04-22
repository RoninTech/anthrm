
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



/*
 -c	capture window or object under cursor
 -p	cursor size (0 to 255)
 -y	set cursor type. 0 = crosshair, 1 = triangle.
 -b	don't sleep(). Invalidates -f
 -f	target frame rate (1 to 63), default is 10fps. actual frame rate is printed on exit
 -s	stretch or fit image to LCD
 -l	capture window or object by location, eg; '-l 120 400'
 -t	retranslate cursor position to client position. should never be required.
 -i	take screenshot. last image when exiting is saved as screencap.tga
 -n	don't draw cursor
 -m follow cursor
 -d1 capture desktop display 1. may also require -s
 -d2 capture desktop display 2. may also require -s
 -r Run time period in seconds before exiting.
 
screencap.exe -c -p 20 -f 15 -s -y 0

*/


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <windows.h>

#include "mylcd.h"
#include "demos.h"

#define MIN(a, b) ((a)<(b)?(a):(b))
#define MAX(a, b) ((a)>(b)?(a):(b))

#define nMaxCount 127	/* max window title length in bytes*/

#ifndef CAPTUREBLT
#define CAPTUREBLT 0x40000000
#endif

typedef struct{
	HWND WndHandle;
	HDC hDCSrc;
	HDC hDCMemory;
	HBITMAP hBmp;
	HBITMAP hBmpPrev;
	POINT pt;
	RECT rc;
	int WidthSrc;
	int HeightSrc;
	char *image;
	int curSize;
	int fps;		// target fps
	int runPeriodLength;
	
	int getDesktop:1;
	int getDesktop1:1;
	int curTranslate:1;
	int scrShot:1;
	int fitImage:1;
	int noCursor:1;
	int curType:1;	// 0=crosshair, 1=triangle
	int noSleep:1;
	int runPeriod:1;
	int followCur:1;
}TSCREENCAP;
static TSCREENCAP sc;


static int dibWidthPadded (int width)
{
	if (width&3)
		return ((width>>2)<<2) + 4;
	else 
		return width;
}

static void setPixel16_NB (const TFRAME *frm, const int x, const int y, const int value)
{
	*(uint16_t*)(frm->pixels+((y*frm->pitch)+(x<<1))) = value;
}

static int getPixel16_NB (const TFRAME *frm, const int x, const int y)
{
	return (uint32_t)*(uint16_t*)(frm->pixels+((y*frm->pitch)+(x<<1)));
}

static void setPixel16_NBr (const TFRAME *frm, const int x, const int row, const int value)
{
	*(uint16_t*)(frm->pixels+(row+(x*2))) = value;
}

static int getPixel32_NB (const TFRAME *frm, const int x, const int y)
{
	return *(uint32_t*)(frm->pixels+((y*frm->pitch)+(x<<2)));
}

static void setPixel32_NB (const TFRAME *frm, const int x, const int y, const int value)
{
	*(uint32_t*)(frm->pixels+((y*frm->pitch)+(x<<2))) = value;
}

static void _32bppDib32bppFrame (TFRAME *frame, ubyte *data, int x1, int y1, int x2, int y2, int Width, int Height)
{
	int w = MIN(Width, frame->width);
	w = MIN(w, x2+1);
	int h = MIN(Height, frame->height);
	h = MIN(h, y2+1);

	const int p = frame->width;
	const int LineWidth = dibWidthPadded(Width<<2);
	const int xoffset = x1<<2;
	ubyte *dib;
	int r,g,b;
	int x,y;
	int32_t *pixels= (int32_t*)lGetPixelAddress(frame, 0, 0);
	int pitch = y1 * p;
	
	for (y = y1; y < h; y++){
		dib = data + (y * LineWidth) + xoffset;
		for (x = x1; x < w; x++){
			b = (*dib++);
			g = (*dib++) << 8;
			r = (*dib++) << 16;
			//setPixel32_NB(frame, x, y, r|g|b);
			//pixels[y*p + x] = r|g|b;
			pixels[pitch + x] = r|g|b;
			dib++;
		}
		pitch += p;
	}
}

static void _32bppDib16bppFrame (TFRAME *frame, ubyte *data, int x1, int y1, int x2, int y2, int Width, int Height)
{
	int w = MIN(Width, frame->width);
	w = MIN(w, x2+1);
	int h = MIN(Height, frame->height);
	h = MIN(h, y2+1);

	const int p = frame->width;
	const int LineWidth = dibWidthPadded(Width<<2);
	const int xoffset = x1<<2;
	ubyte *dib;
	int r,g,b;
	int x,y,row;
	int16_t *pixels= (int16_t*)lGetPixelAddress(frame, 0, 0);
	int pitch = y1 * p;

	for (y = y1; y < h; y++){
		dib = data + (y * LineWidth) + xoffset;
		row = y * p;
		for (x = x1; x < w; x++){
			b = (*dib++ &0xF8) >> 3;	// 5
			g = (*dib++ &0xFC) << 3;	// 6
			r = (*dib++ &0xF8) << 8;	// 5
			//setPixel16_NBr(frame, x, row, r|g|b);
			//pixels[y*p + x] = r|g|b;
			pixels[pitch + x] = r|g|b;
			dib++;
		}
		pitch += p;
	}
}

static void getWindowTitle (TSCREENCAP *sc, char *ptitle)
{
    *ptitle = 0;
    GetWindowText(sc->WndHandle, ptitle, nMaxCount);
}

static void drawCursorTriangle (TFRAME *frame, int csize, int x, int y)
{
	y += 1;
	lDrawTriangleFilled(frame, x, y, x, y+csize, x+csize, y+csize, 0xFFFF);
	y--;
	csize++;
	lDrawTriangle(frame, x, y, x, y+csize, x+csize, y+csize, 0x0000);
}

static void drawCursorCrosshair (TFRAME *frame, int csize, int x, int y)
{
	int width = csize>>1;
	lDrawRectangleFilled(frame, x-width, y-1, x+width, y+1, frame->hw->render->backGround);
	lDrawRectangleFilled(frame, x-1, y-width, x+1, y+width, frame->hw->render->backGround);
	width--;
	lDrawLine(frame, x-width, y, x+width, y, frame->hw->render->foreGround);
	lDrawLine(frame, x, y-width, x, y+width, frame->hw->render->foreGround);
}

static void drawCursor (TFRAME *frame, TSCREENCAP *sc)
{
	POINT *pt = &sc->pt;
	if (!sc->followCur)
		GetCursorPos(pt);
	if (sc->curTranslate)
		ScreenToClient(sc->WndHandle, pt);

	if (!sc->followCur && !sc->getDesktop){
		GetWindowRect(sc->WndHandle, &sc->rc);
		pt->x -= sc->rc.left;
		pt->y -= sc->rc.top;
	}

	if (sc->fitImage){
		pt->x *= (double)((double)frame->width/(double)sc->WidthSrc);
		pt->y *= (double)((double)frame->height/(double)sc->HeightSrc);
	}
	
	if (!sc->curType)
		drawCursorCrosshair(frame, sc->curSize, pt->x, pt->y);
	else
		drawCursorTriangle(frame, sc->curSize, pt->x, pt->y);
} 

int getWindowHandle (TSCREENCAP *sc, int x, int y)
{
	sc->pt.x = x;
	sc->pt.y = y;
	
	sc->WndHandle = WindowFromPoint(sc->pt);
	if (sc->WndHandle == NULL)
		return 0;

    //Get Window device context
    if (sc->getDesktop){
    	if (sc->getDesktop1)
   			sc->hDCSrc = CreateDC("\\\\.\\DISPLAY1", NULL, NULL, NULL);
   		else
   			sc->hDCSrc = CreateDC("\\\\.\\DISPLAY2", NULL, NULL, NULL);
   		if (sc->hDCSrc == NULL){
   			printf("desktop unavailable\n");
   		}
   		
   		sc->WidthSrc = GetDeviceCaps(sc->hDCSrc, HORZRES);
		sc->HeightSrc = GetDeviceCaps(sc->hDCSrc, VERTRES);
   	}else{
   		sc->hDCSrc = GetWindowDC(sc->WndHandle);
   		GetWindowRect(sc->WndHandle, &sc->rc);
   		sc->WidthSrc = sc->rc.right - sc->rc.left;
		sc->HeightSrc = sc->rc.bottom - sc->rc.top;
   	}

	sc->image = calloc(sizeof(uint32_t), sc->HeightSrc * sc->WidthSrc);
	if (sc->image == NULL) return 0;

    //create a memory device context
   	sc->hDCMemory = CreateCompatibleDC(sc->hDCSrc);

    //create a bitmap compatible with window hdc
   	sc->hBmp = CreateCompatibleBitmap(sc->hDCSrc, sc->WidthSrc, sc->HeightSrc);
	
   	return 1;
}

int getWindowImage (TSCREENCAP *sc, TFRAME *frame)
{

    //copy newly created bitmap into memory device context
    if (!sc->hBmpPrev)
   		sc->hBmpPrev = SelectObject(sc->hDCMemory, sc->hBmp);
   	else
   		SelectObject(sc->hDCMemory, sc->hBmp);
    
     // copy window hdc to memory hdc
    BitBlt(sc->hDCMemory, 0, 0, sc->WidthSrc, sc->HeightSrc, sc->hDCSrc, 0, 0, SRCCOPY|CAPTUREBLT);

    GetBitmapBits(sc->hBmp, sc->WidthSrc * sc->HeightSrc * 4, sc->image);
	if ((frame->width != sc->WidthSrc || frame->height != sc->HeightSrc) && (sc->fitImage == -1 || sc->followCur))
		lResizeFrame(frame, sc->WidthSrc, sc->HeightSrc, 0);

	//printf("%i %i %i\n",sc->WidthSrc, sc->HeightSrc, sc->fitImage );

	int x = 0, y = 0;
	int x2 = frame->width-1, y2 = frame->height-1;
	
	if (sc->followCur){
		POINT *pt = &sc->pt;
		GetCursorPos(pt);
		if (!sc->getDesktop){
			GetWindowRect(sc->WndHandle, &sc->rc);
			pt->x -= sc->rc.left;
			pt->y -= sc->rc.top;
		}
		x = pt->x - (DWIDTH>>1);
		if (x > frame->width-DWIDTH) x = frame->width-DWIDTH;
		if (x < 0) x = 0;
		x2 = x+DWIDTH-1;
	
		y = pt->y - (DHEIGHT>>1);
		if (y > frame->height-DHEIGHT) y = frame->height-DHEIGHT;
		if (y < 0) y = 0;
		y2 = y+DHEIGHT-1;
	}
	
	if (frame->bpp == LFRM_BPP_16)
		_32bppDib16bppFrame(frame, (ubyte*)sc->image, x, y, x2, y2, sc->WidthSrc, sc->HeightSrc);
	else
		_32bppDib32bppFrame(frame, (ubyte*)sc->image, x, y, x2, y2, sc->WidthSrc, sc->HeightSrc);

	//DeleteObject(sc->hBmpPrev);
	sc->hBmpPrev = NULL;
	return 1;
}

void deleteWindowHandle (TSCREENCAP *sc)
{
    // release the created objects and free memory
    if (sc->hBmp)
		DeleteObject(sc->hBmp);
	if (sc->hBmpPrev)
		DeleteObject(sc->hBmpPrev);
	if (sc->hDCMemory)
    	DeleteDC(sc->hDCMemory);
    if (sc->WndHandle)
    	ReleaseDC(sc->WndHandle, sc->hDCSrc);
    if (sc->image)
    	free(sc->image);
}

static int stretchImage16 (TFRAME *from, TFRAME *to, float src_x, float src_y, float src_width, float src_height)
{
	float x, y, x2, y2;
	const float scalex = (float)to->width / src_width;
	const float scaley = (float)to->height / src_height;
	
	for (y=0; y<to->height; y += 1.0f){
		y2 = src_y + (y/scaley);
		for (x=0; x<to->width; x += 1.0f){
			x2 = src_x + (x/scalex);
			setPixel16_NB(to, x, y, getPixel16_NB(from, x2, y2));
		}
	}
	return 1;
}

static int stretchImageAll (TFRAME *from, TFRAME *to, float src_x, float src_y, float src_width, float src_height)
{
	float x, y, x2, y2;
	const float scalex = (float)to->width / src_width;
	const float scaley = (float)to->height / src_height;
	
	for (y=0; y<to->height; y += 1.0f){
		y2 = src_y + (y/scaley);
		for (x=0; x<to->width; x += 1.0f){
			x2 = src_x + (x/scalex);
			lSetPixel_NB(to, x, y, lGetPixel_NB(from, x2, y2));
		}
	}
	return 1;
}

static int stretchImage32 (TFRAME *from, TFRAME *to, float src_x, float src_y, float src_width, float src_height)
{
	float x, y, x2, y2;
	const float scalex = (float)to->width / src_width;
	const float scaley = (float)to->height / src_height;
	
	for (y=0; y<to->height; y += 1.0f){
		y2 = src_y + (y/scaley);
		for (x=0; x<to->width; x += 1.0f){
			x2 = src_x + (x/scalex);
			setPixel32_NB(to, x, y, getPixel32_NB(from, x2, y2));
		}
	}
	return 1;
}

static int stretchImage (TFRAME *from, TFRAME *to, float src_x, float src_y, float src_w, float src_h)
{
	if (from->bpp == LFRM_BPP_16)
		return stretchImage16(from, to, src_x, src_y, src_w, src_h);
	else if (from->bpp == LFRM_BPP_32)
		return stretchImage32(from, to, src_x, src_y, src_w, src_h);
	else
		return stretchImageAll(from, to, src_x, src_y, src_w, src_h);
}

void printHelp ()
{
	printf("  Screen capture for the USBD480\n");
	printf("\n");
	printf("Usage: -option argument1 argument2\n");
	printf(" -c Capture window or object under cursor\n");
	printf(" -l Capture window or object under this location, eg; '-l 120 400'\n");
	printf(" -p Cursor size (0 to 255)\n");
	printf(" -y Cursor type (0:Crosshair, 1:Triangle)\n");
	printf(" -n Don't draw cursor\n");
	printf(" -m Follow cursor\n");
	printf(" -b Run at full speed. Invalidates -f\n");
	printf(" -f Target frame rate (0 to 63). Actual frame rate is printed on exit\n");
	printf(" -s Stretch or fit image to LCD. Invalidates -m\n");
	printf(" -t Retranslate cursor position to client position. Should not be required\n");
	printf(" -i Take screenshot. Last image when exiting is saved as 'screencap.tga'\n");
	printf(" -d1 Capture display 1. May also require -s. Invalidates -c and -l.\n");
	printf(" -d2 Capture display 2. As above.\n");
	printf(" -r Run for n seconds before exiting.\n");
	printf("Example: 'screencap -c -p 20 -f 15 -s'\n");
	printf("\nCompiled with libmylcd v%s\n",(char*)lVersion());
}

int main (int margc, char **cargv)
{
	int argc = 0;
	wchar_t **argv = CommandLineToArgvW(GetCommandLineW(), &argc);
	if (argv == NULL || argc < 2){
		printHelp();
		return 0;
	}
	
	if (!initDemoConfig("config.cfg"))
		return 0;

	TFRAME *tmp = lCloneFrame(frame);
	TFRAME *out = NULL;
	static char title[nMaxCount+1];
	int x = 0, y = 0, i;
	
	sc.fps = 10;
	sc.curSize = 10;
	sc.curTranslate = 0;
	sc.scrShot = 0;
	sc.fitImage = 0;
	sc.noCursor = 0;
	sc.noSleep = 0;
	sc.getDesktop = 0;
	sc.runPeriod = 0;
	sc.runPeriodLength = 0;
	sc.followCur = 0;
	
	for (i = 0; i < argc; i++){
		wchar_t *cmd = argv[i];
		
		if (!wcscmp(L"-l", cmd)){			// get window by location eg, '-l 120 500'
			if (i+2 >= argc) break;
			x = wcstol(argv[i+1], NULL, 10);
			y = wcstol(argv[i+2], NULL, 10);
			
		}else if (!wcscmp(L"-p", cmd)){		// set cursor size
			if (i+1 >= argc) break;
			sc.curSize = (wcstol(argv[i+1], NULL, 10))&0xFF;

		}else if (!wcscmp(L"-y", cmd)){		// set cursor type
			if (i+1 >= argc) break;
			sc.curType = (wcstol(argv[i+1], NULL, 10))&0x01;
			
		}else if (!wcscmp(L"-f", cmd)){		// set fps
			if (i+1 >= argc) break;
			sc.fps = (wcstol(argv[i+1], NULL, 10)); //&0x3F;
			if (sc.fps < 1)
				sc.fps = 10;

		}else if (!wcscmp(L"-r", cmd)){		// run time before exiting
			if (i+1 >= argc) break;
			sc.runPeriodLength = (wcstol(argv[i+1], NULL, 10))*1000;
			if (sc.runPeriodLength > 0)
				sc.runPeriod = 1;
		
		}else if (!wcscmp(L"-c", cmd)){		// get window by current cursor position, ie; window or object under cursor
			if (!sc.getDesktop){
				GetCursorPos(&sc.pt);
				x = sc.pt.x; y = sc.pt.y;
			}
		}else if (!wcscmp(L"-t", cmd)){		// translate screen cursor location to client location. Should not normally be required
			sc.curTranslate = 1;

		}else if (!wcscmp(L"-d1", cmd)){		// get desktop, ignore -c and -l
			sc.getDesktop = 1;
			sc.getDesktop1 = 1;		// get display 1
			x = sc.pt.x = 0;
			y = sc.pt.y = 0;
			
		}else if (!wcscmp(L"-d2", cmd)){		// get desktop, ignore -c and -l
			sc.getDesktop = 1;
			sc.getDesktop1 = 0;		// get display 2
			x = sc.pt.x = 0;
			y = sc.pt.y = 0;
						
		}else if (!wcscmp(L"-i", cmd)){		// save screenshot as a TGA image
			sc.scrShot = 1;
			
		}else if (!wcscmp(L"-s", cmd)){		// fit window to lcd
			sc.fitImage = 1;

		}else if (!wcscmp(L"-n", cmd)){		// don't draw cursor
			sc.noCursor = 1;
						
		}else if (!wcscmp(L"-m", cmd)){		// don't draw cursor
			sc.followCur = 1;
			
		}else if (!wcscmp(L"-b", cmd)){		// don't sleep
			sc.noSleep = 1;
		}
	}
	
	if (sc.fitImage != 0){
		sc.followCur = 0;
		out = frame;
	}else{
		out = tmp;
	}
		
	if (!getWindowHandle(&sc, x, y)){
		getWindowHandle(&sc, 0, 0);
		x = 0; y = 0;
	}	
	
	if (!sc.getDesktop)
		getWindowTitle(&sc, title);
	else
		strcpy(title, "Desktop");
	printf("\nObject at %i,%i: '%s'\n", x, y, title);

	int fps = 0, tfps = 0;
	int delay = (1.0/(float)sc.fps)*1000.0;
	int t0, t1;
	int frameTime = 0;		
	timeBeginPeriod(1);
	int tfps0 = timeGetTime();
	const unsigned int runTime0 = GetTickCount();
	POINT *pt = &sc.pt;
	
	while(!kbhit()){
		if (sc.runPeriod){
			if (GetTickCount()-runTime0 >= sc.runPeriodLength)
				break;
		}
		t0 = timeGetTime();
		getWindowImage(&sc, tmp);
		if (sc.fitImage)
			stretchImage(tmp, frame, 0, 0, tmp->width, tmp->height);
		if (!sc.noCursor)
			drawCursor(out, &sc);
		
		if (sc.followCur){
			x = pt->x - (frame->width>>1);
			if (x > out->width-DWIDTH) x = out->width-DWIDTH;
			if (x < 0) x = 0;
			
			y = pt->y - (frame->height>>1);
			if (y > out->height-DHEIGHT) y = out->height-DHEIGHT;
			if (y < 0) y = 0;

			displays[0].left = x;
			displays[0].right = x+DWIDTH-1;
			displays[0].top = y;
			displays[0].btm = y+DHEIGHT-1;
		}
		
		lRefresh(out);
	
		if (!sc.noSleep){
			t1 = timeGetTime();
			if (t1-frameTime > 1000){
				frameTime = t1;
				fps = 0;
			}
			fps++;
			t1 -= t0;
			if (t1 < 1) t1 = 0;
			else if (t1 >= delay)
				t1 = delay-1;
			lSleep(delay-t1);
		}
		tfps++;
	}
	int tfps1 = timeGetTime();
	printf("%.2f\n",tfps/(float)((tfps1-tfps0)/1000.0f));

	if (sc.scrShot)
		lSaveImage(out, L"screencap.tga", IMG_TGA, 0, 0);
		
	timeEndPeriod(1);
	deleteWindowHandle(&sc);
	lDeleteFrame(tmp);
	demoCleanup();
}

