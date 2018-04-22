
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




#include "common.h"
#include "morph.h"



#define LIFE_SPAN 		10.0f
#define MIN_DELTA_T		0.03f


typedef struct {
	union {
		struct {
			unsigned char b,g,r,a;
		}c;
		unsigned int val;
	}u;
}TCOLOUR4;

#define m_getPixelAddr32(f,x,y)	(f->pixels+((y*f->pitch)+(x<<2)))


/*
typedef struct{
	ubyte b;
	ubyte g;
	ubyte r;
	ubyte a;
}__attribute__ ((packed))TBGRA;		// 8888

typedef struct {
	union {
		TBGRA bgr a;
		int colour;
	}u;
}TCOLOUR4;
*/
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

/*
#ifdef USE_MMX
	__asm volatile ("prefetch 64(%0)\n" :: "r" (px2y1) : "memory");
#endif
*/

	const TCOLOUR4 *cx1y1 = (TCOLOUR4*)px1y1;
	const TCOLOUR4 *cx2y1 = (TCOLOUR4*)px2y1;
	const TCOLOUR4 *cx1y2 = (TCOLOUR4*)px1y2;
	const TCOLOUR4 *cx2y2 = (TCOLOUR4*)px2y2;
	
	c->u.c.r = (wx1y1 * cx1y1->u.c.r + wx2y1 * cx2y1->u.c.r + wx1y2 * cx1y2->u.c.r + wx2y2 * cx2y2->u.c.r + 128*256) / (256*256);
	c->u.c.g = (wx1y1 * cx1y1->u.c.g + wx2y1 * cx2y1->u.c.g + wx1y2 * cx1y2->u.c.g + wx2y2 * cx2y2->u.c.g + 128*256) / (256*256);
	c->u.c.b = (wx1y1 * cx1y1->u.c.b + wx2y1 * cx2y1->u.c.b + wx1y2 * cx1y2->u.c.b + wx2y2 * cx2y2->u.c.b + 128*256) / (256*256);
	c->u.c.a = 255; //(wx1y1 * cx1y1->u.c.a + wx2y1 * cx2y1->u.c.a + wx1y2 * cx1y2->u.c.a + wx2y2 * cx2y2->u.c.a + 128*256) / (256*256);
	return c->u.val;
}


static void setPixel32a_NB (const TFRAME *frm, const int x, const int y, const int value)
{
	TCOLOUR4 *dst = (TCOLOUR4*)m_getPixelAddr32(frm, x, y);
	const TCOLOUR4 *src = (TCOLOUR4*)&value;
	const unsigned int alpha = src->u.c.a + (unsigned int)(src->u.c.a>>7);
	const unsigned int odds2 = (src->u.val >>8) & 0xFF00FF;
	const unsigned int odds1 = (dst->u.val>>8) & 0xFF00FF;
	const unsigned int evens1 = dst->u.val & 0xFF00FF;
	const unsigned int evens2 = src->u.val & 0xFF00FF;
	const unsigned int evenRes = ((((evens2-evens1)*alpha)>>8) + evens1)& 0xFF00FF;
	const unsigned int oddRes = ((odds2-odds1)*alpha + (odds1<<8)) & 0xFF00FF00;
	dst->u.val = evenRes | oddRes;
}

static inline int getPixel32_NBr (const TFRAME *frm, const int x, const int row)
{
	return *(uint32_t*)(frm->pixels+(row+(x<<2)));
}

static void setPixel32a (const TFRAME *frm, const int x, const int y, const int value)
{	
	if (x >= 0 && x < frm->width && y < frm->height && y >= 0)
		setPixel32a_NB(frm, x, y, value);
}

static int getTotalPoints (TFRAME *src)
{
	int r, x, y;
	int total = 0;
	
	for (y = 0; y < src->height; y++){
		r = y * src->pitch;
		for (x = 0; x < src->width; x++){
			if (getPixel32_NBr(src, x, r))
				total++;
		}
	}
	return total;
}

static int get2DPoints (TFRAME *src, TMPOINT *list, int total, int x1, int y1, int x2, int y2)
{
	int x,y;
	int i = 0;
	int r;
	int *p;
	
	for (y = y1; y < y2; y++){
		r = y * src->pitch;
		p = lGetPixelAddress(src, 0, y);
		
		for (x = x1; x < x2; x++){
			if (getPixel32_NBr(src, x, r)){
				list[i].colour = &p[x];
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

int countActivePoints (TMPOINT *list, int total)
{
	int ct = 0;
	for (int i = 0; i < total; i++, list++)
		ct += list->active;
	return ct;
}

void translatePoints (TMPOINT *list, int total, int dx, int dy)
{
	int i;
	for (i = 0; i < total; i++, list++){
		list->x += dx;
		list->y += dy;
	}
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

static int drawPoints (TFRAME *des, TMPOINT *list, int total, int cx, int cy)
{
	int ct = 0;
	int descol, srccol;

	for (int i = 0; i < total; i++, list++){
		if (list->active){
			//setPixel32a(des, list->x+cx, list->y+cy, *list->colour);
						
			srccol = getPixel32a_BL(des, list->x+cx, list->y+cy);
			descol = blendColoursNoEmms(*list->colour, srccol, 160);
			setPixel32a(des, list->x+cx, list->y+cy, descol);

			ct++;
		}
	}
	callEmms();
	return ct;
}

static void generateRandomPoints (TMPOINT *list, TMPOINT *rlist, int total, int delta)
{
	srand(((int)list&(int)rlist)+total+delta);
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
		dlist->active = list->active;
		if (list->active){
			dlist->x = list->x + frameNumber * ((list->x - rlist->x) / (float)tFrames);
			dlist->y = list->y + frameNumber * ((list->y - rlist->y) / (float)tFrames);
			dlist->colour = list->colour;
		}
	}
}

static void decayTime (TMPOINT *_list, int total, float dt)
{
	float dt2;
	TMPOINT *list;

	while (dt > 0.0f){
        // calculate delta time for this iteration
		dt2 = dt < MIN_DELTA_T ? dt : MIN_DELTA_T;

		int i;
		list = _list;
		for (i = 0; i < total; i++, list++){
			if (list->active){
				list->time -= dt2 * (1.0f / LIFE_SPAN);
				if (list->time < 0.0f) list->active = 0;
			}
		}
		dt -= dt2;
	}
}

void morphDelete (TMORPH *morph)
{
	my_free(morph->list);
	my_free(morph->rlist);
	my_free(morph->dlist);
	my_free(morph);
}

int morphRender (TMORPH *morph, TFRAME *frame, int x, int y)
{
	float dt;
	float t;
	uint64_t t1;

	createDeltaPoints(morph->list, morph->rlist, morph->dlist, morph->total, morph->fIndex, morph->tFrames);
	QueryPerformanceCounter((LARGE_INTEGER*)&t1);
			
	t = ((float)(t1 - morph->t0)) * morph->resolution;
	dt = t - morph->t_old;
    morph->t_old = t;
    
	decayTime(morph->list, morph->total, dt);
	morph->fIndex++;
	//printf("%i %i\n",morph->fIndex, countActivePoints(morph->list, morph->total));
	return drawPoints(frame, morph->dlist, morph->total, x, y);
}

TMORPH *morphCreate (TFRAME *image, int tFrames, int spread)
{
	TMORPH *morph = my_calloc(1, sizeof(TMORPH));
	if (morph){
		morph->fIndex = 0;
		morph->tFrames = tFrames;
		morph->total = getTotalPoints(image);
		morph->list = my_calloc(sizeof(TMPOINT), morph->total);
		morph->rlist = my_calloc(sizeof(TMPOINT), morph->total);
		morph->dlist = my_calloc(sizeof(TMPOINT), morph->total);
		if (morph->list && morph->rlist && morph->dlist){
			morph->total = get2DPoints(image, morph->list, morph->total, 0, 0, image->width-1, image->height-1);
			generateRandomPoints(morph->list, morph->rlist, morph->total, spread);
			QueryPerformanceFrequency((LARGE_INTEGER*)&morph->freq);
			morph->resolution = 1.0f / (float)morph->freq;
			QueryPerformanceCounter((LARGE_INTEGER*)&morph->t0);
			return morph;
		}
		my_free(morph);
	}
	return NULL;
}

TMORPH *morphClone (TMORPH *src)
{
	TMORPH *morph = my_calloc(1, sizeof(TMORPH));
	if (morph){
		morph->fIndex = 0;
		morph->tFrames = src->tFrames;
		morph->total = src->total;
		morph->list = my_calloc(sizeof(TMPOINT), src->total);
		morph->rlist = my_calloc(sizeof(TMPOINT), src->total);
		morph->dlist = my_calloc(sizeof(TMPOINT), src->total);
		if (morph->list && morph->rlist && morph->dlist){
			my_memcpy(morph->list, src->list, sizeof(TMPOINT) * src->total);
			my_memcpy(morph->rlist, src->rlist, sizeof(TMPOINT) * src->total);
			my_memcpy(morph->dlist, src->dlist, sizeof(TMPOINT) * src->total);
			morph->freq = src->freq;
			morph->t_old = src->t_old;
			morph->resolution = 1.0f / (float)morph->freq;
			QueryPerformanceCounter((LARGE_INTEGER*)&morph->t0);
			return morph;
		}
		my_free(morph);
	}
	return NULL;
}

int morphReset (TMORPH *morph, TMORPH *src)
{
	if (morph){
		morph->fIndex = 0;
		morph->tFrames = src->tFrames;
		morph->total = src->total;
		my_memcpy(morph->list, src->list, sizeof(TMPOINT) * src->total);
		my_memcpy(morph->rlist, src->rlist, sizeof(TMPOINT) * src->total);
		my_memcpy(morph->dlist, src->dlist, sizeof(TMPOINT) * src->total);
		morph->freq = src->freq;
		morph->t_old = src->t_old;
		morph->resolution = 1.0f / (float)morph->freq;
		QueryPerformanceCounter((LARGE_INTEGER*)&morph->t0);
		return 1;
	}
	return 0;
}
