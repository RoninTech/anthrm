
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
#include <math.h>

#include "mylcd.h"
#include "demos.h"

typedef struct{
	int active;
	float time;
	int x;
	int y;
	int *colour;
}TMPOINT;

#define LIFE_SPAN 		1.0f



typedef struct{
	ubyte b;
	ubyte g;
	ubyte r;
	ubyte a;
}__attribute__ ((packed))TBGRA;		// 8888

typedef struct {
	union {
		TBGRA bgra;
		int colour;
	}u;
}TCOLOUR4;

static int getPixel32a_BL (TFRAME *frame, float x, float y)
{
	#define PSEUDO_FLOOR( V ) ((V) >= 0 ? (int)(V) : (int)((V) - 1))
	
	TCOLOUR4 clr;
	TCOLOUR4 *c = &clr;
	
	const int width = frame->width;
	const int height = frame->height;
	const int pitch = frame->pitch;
	const int spp = 4;	// samples per pixel
		
	if (x < 0) x = 0;

	float x1 = PSEUDO_FLOOR(x);
	float x2 = x1+1.0;
	
	if (x2 >= width) {
		x = x2 = (float)width-1.0;
		x1 = x2 - 1;
	}
	const int wx1 = (int)(256*(x2 - x));
	const int wx2 = 256-wx1;

	if (y < 0) y = 0;
	
	float y1 = PSEUDO_FLOOR(y);
	float y2 = y1+1.0;
	
	if (y2 >= height) {
		y = y2 = (float)height-1.0;
		y1 = y2 - 1;
	}
	const int wy1 = (int)(256*(y2 - y));
	const int wy2 = 256 - wy1;
	const int wx1y1 = wx1*wy1;
	const int wx2y1 = wx2*wy1;
	const int wx1y2 = wx1*wy2;
	const int wx2y2 = wx2*wy2;
	unsigned char *px1y1 = &frame->pixels[pitch * (int)y1 + spp * (int)x1];
	unsigned char *px2y1 = px1y1 + spp;
	unsigned char *px1y2 = px1y1 + pitch;
	unsigned char *px2y2 = px1y1 + pitch+spp;

#ifdef USE_MMX
	__asm volatile ("prefetch 64(%0)\n" :: "r" (px2y1) : "memory");
#endif

	const TCOLOUR4 *cx1y1 = (TCOLOUR4*)px1y1;
	const TCOLOUR4 *cx2y1 = (TCOLOUR4*)px2y1;
	const TCOLOUR4 *cx1y2 = (TCOLOUR4*)px1y2;
	const TCOLOUR4 *cx2y2 = (TCOLOUR4*)px2y2;
	
	c->u.bgra.r = (wx1y1 * cx1y1->u.bgra.r + wx2y1 * cx2y1->u.bgra.r + wx1y2 * cx1y2->u.bgra.r + wx2y2 * cx2y2->u.bgra.r + 128*256) / (256*256);
	c->u.bgra.g = (wx1y1 * cx1y1->u.bgra.g + wx2y1 * cx2y1->u.bgra.g + wx1y2 * cx1y2->u.bgra.g + wx2y2 * cx2y2->u.bgra.g + 128*256) / (256*256);
	c->u.bgra.b = (wx1y1 * cx1y1->u.bgra.b + wx2y1 * cx2y1->u.bgra.b + wx1y2 * cx1y2->u.bgra.b + wx2y2 * cx2y2->u.bgra.b + 128*256) / (256*256);
	c->u.bgra.a = 0xFF;//(wx1y1 * cx1y1->u.bgra.a + wx2y1 * cx2y1->u.bgra.a + wx1y2 * cx1y2->u.bgra.a + wx2y2 * cx2y2->u.bgra.a + 128*256) / (256*256);
	return c->u.colour;
}


static int getTotalPoints (TFRAME *src)
{
	int r, x, y;
	int total = 0;

	for (y = 0; y < src->height; y++){
		r = y * src->pitch;
		for (x = 0; x < src->width; x++){
			if (lGetPixel_NBr(src, x, r))
				total++;
		}
	}
	return total;
}

void translatePoints (TMPOINT *list, int total, int dx, int dy)
{
	int i;
	for (i = 0; i < total; i++, list++){
		list->x += dx;
		list->y += dy;
	}
}

static int get2DPoints (TFRAME *src, TMPOINT *list, int total, int x1, int y1, int x2, int y2)
{
	srand(GetTickCount()&255);	
	int x,y;
	int i = 0;
	for (y = y1; y < y2; y++){
		for (x = x1; x < x2; x++){
			if (lGetPixel_NB(src, x, y)){
				list[i].colour = lGetPixelAddress(src, x, y);
				list[i].x = x;
				list[i].y = y;
				list[i].time = 0.2 + (float)((rand()%150)/100.0);
				list[i].active = 1;
				if (++i >= total)
					return i;
			}
		}
	}
	return i;
}

static int countActivePoints (TMPOINT *list, int total)
{
	int ct = 0;
	for (int i = 0; i < total; i++, list++)
		ct += list->active;
	return ct;
}


static inline int blendColoursNoEmms (const unsigned int src, const unsigned int dst, unsigned int factor)
{
#ifndef _DEBUG_ 

	static unsigned short INVERT_MASK[4] = {0x00FF, 0x00FF, 0x00FF, 0x00FF};
	int returnParam;
	factor = 255 - factor;

	__asm volatile (
	  "movd %1, %%mm0\n"
	  "movd %2, %%mm1\n"
	  "pxor %%mm2, %%mm2\n"
	  "punpcklbw %%mm2, %%mm0\n"
	  "punpcklbw %%mm2, %%mm1\n"
	  
	  // Get the alpha value //
	  "movd %4, %%mm3\n"
	  "punpcklwd %%mm3, %%mm3\n"
	  "punpcklwd %%mm3, %%mm3\n"
	  
	  // (alpha * (source + 255 - dest))/255 + dest - alpha //
	  "paddw (%3), %%mm0\n"
	  "psubw %%mm1, %%mm0\n"
	  "psrlw $1, %%mm0\n"
	  "pmullw %%mm3, %%mm0\n"
	  "psrlw $7, %%mm0\n"
	  "paddw %%mm1, %%mm0\n"
	  "psubw %%mm3, %%mm0\n"
	  
	  "packuswb %%mm0, %%mm0\n"
	  "movd %%mm0, %0\n"
	  : "=&a" (returnParam)
	  : "rm" (dst), "rm" (src), "rm" (&INVERT_MASK[0]), "rm" (factor)
	  : "memory"
	);
	return returnParam;
#else
	return src;
#endif
}

static inline void callEmms ()
{
  __asm volatile("emms\n");
}

int drawPoints (TFRAME *des, TMPOINT *list, int total, int cx, int cy)
{
	int ct = 0;
	int descol, srccol;
	
	for (int i = 0; i < total; i++, list++){
		if (list->active){
			//lSetPixel(des, list->x+cx, list->y+cy, *list->colour);
			
			srccol = getPixel32a_BL(des, list->x+cx, list->y+cy);
			descol = blendColoursNoEmms(*list->colour, srccol, 200);
			lSetPixel(des, list->x+cx, list->y+cy, descol);
			
			ct++;
		}
	}
	callEmms();
	return ct;
}

static void generateRandomPoints (TMPOINT *list, TMPOINT *rlist, int total, int delta)
{
	srand(GetTickCount()&0x65535);
	const int deltad2 = delta/2.0;
	
	int i;
	for (i = 0; i < total; i++, list++, rlist++){
		rlist->x = list->x + ((rand()%delta)-deltad2);
		rlist->y = list->y + ((rand()%delta)-deltad2);
		//rlist->colour = list->colour;
		//rlist->time = list->time;
		//rlist->active = list->active;
	}
}

static void createDeltaPoints (TMPOINT *list, TMPOINT *rlist, TMPOINT *dlist, const int total, const int frameNumber, const int tFrames)
{
	int i;
	for (i = 0; i < total; i++, list++, rlist++, dlist++){
		if (list->active){
			dlist->x = list->x + frameNumber * ((list->x - rlist->x) / (float)tFrames);
			dlist->y = list->y + frameNumber * ((list->y - rlist->y) / (float)tFrames);
			dlist->colour = list->colour;
		}
		dlist->active = list->active;
	}
}

void decayTime (TMPOINT *list, int total, float t)
{
    static float t_old = 0.0;
    static float dt;

    dt = t - t_old;
    t_old = t;

	for (int i = 0; i < total; i++, list++){
		if (list->active){
			list->time = list->time - dt * (1.0f / LIFE_SPAN);
			if (list->time < 0.0f) list->active = 0;
		}
	}
}

int main (int argc, char* argv[])
{

	if (!initDemoConfig("config.cfg"))
		return 0;
		
	lSetBackgroundColour(hw, lGetRGBMask(frame, LMASK_BLACK));
	lSetForegroundColour(hw, lGetRGBMask(frame, LMASK_WHITE));
	
	TFRAME *back = lNewImage(hw, L"images/test.png", DBPP);
	TFRAME *image = lNewImage(hw, L"images/child.bmp", DBPP);
	//TFRAME *text = lNewString(hw, DBPP, 0, LFTW_KOI12X24B, "A quick fox jumps.");
	TFRAME *src = image;
	
	int total = getTotalPoints(src);
	TMPOINT *list = calloc(sizeof(TMPOINT), total);
	TMPOINT *rlist = calloc(sizeof(TMPOINT), total);
	TMPOINT *dlist = calloc(sizeof(TMPOINT), total);

	total = get2DPoints(src, list, total, 0, 0, src->width-1, src->height-1);
	generateRandomPoints(list, rlist, total, 256);

	uint64_t freq, t0_64, t_64;
	QueryPerformanceFrequency((LARGE_INTEGER *)&freq);
	float Resolution = 1.0 / (float)freq;
	QueryPerformanceCounter((LARGE_INTEGER *)&t0_64);

	int f;
	int tFrames = 96;

	for (f = 0; f < tFrames; f += 1){
		//lClearFrame(frame);
		lDrawImage(back, frame, 0, 0);
		
		QueryPerformanceCounter((LARGE_INTEGER *)&t_64);
		createDeltaPoints(list, rlist, dlist, total, f, tFrames);
		decayTime(list, total, (((float)(t_64 - t0_64)) * Resolution) / 1.0);

		/*if (f > tFrames/10)
			translatePoints(list, total, 3, 2);
		else
			translatePoints(list, total, 2, -2);
		*/
		
		if (!drawPoints(frame, dlist, total, 150, 70)){
			lRefresh(frame);
			break;
		}
		lRefresh(frame);
		//printf("%i active %i\n", f, countActivePoints(list, total));
		
		
		lSleep(30);
	}
	//printf("active %i\n", countActivePoints(list, total));
		
	free(list);
	free(rlist);
	free(dlist);

	lDeleteFrame(back);
	lDeleteFrame(image);
	//lSaveImage(frame, L"morph.png", IMG_PNG, 0, 0);
	
	demoCleanup();
	return 1;
}


void createDeltaPointsF (const TMPOINT *list, const TMPOINT *rlist, TMPOINT *dlist, const int total, const double f)
{
	if (f >= -0.0001 && f <= 0.0001){
		int i;
		for (i = 0; i < total; i++, list++, dlist++){
			dlist->x = list->x;
			dlist->y = list->y;
			dlist->colour = list->colour;
			dlist->time = list->time;
		}	
	}else{
		int i;
		for (i = 0; i < total; i++, list++, rlist++, dlist++){
			dlist->x = list->x + ((double)(list->x - rlist->x) * f);
			dlist->y = list->y + ((double)(list->y - rlist->y) * f);
			dlist->colour = list->colour;
			dlist->time = list->time;
		}
	}
}
