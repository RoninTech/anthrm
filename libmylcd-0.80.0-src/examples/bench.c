
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
#include "../src/mmx.h"
#include "../src/mmx_rgb.h"

MYLCD_EXPORT uint64_t rdtsc();
MYLCD_EXPORT void * my_Memcpy (void *s1, const void *s2, size_t n)
	__attribute__((nonnull(1, 2)));


typedef struct{
	ubyte r;
	ubyte g;
	ubyte b;
	ubyte a;
}__attribute__ ((packed))TRGBA;


static uint64_t freq;
static uint64_t tStart;
static float resolution;

static void setRes ()
{
	QueryPerformanceCounter((LARGE_INTEGER*)&tStart);
	QueryPerformanceFrequency((LARGE_INTEGER *)&freq);
	resolution = 1.0 / (float)freq;
}


static float getTime ()
{
	uint64_t t1 = 0;
	QueryPerformanceCounter((LARGE_INTEGER*)&t1);
	return ((float)((uint64_t)(t1 - tStart) * resolution) * 1000.0);
}

static inline void frame32ToBuffer16 (TFRAME *frm, uint16_t * restrict out)
{
	int x,y;
	TRGBA *restrict in = (TRGBA*restrict)frm->pixels;
	TRGBA *p;
	const int h = frm->height;
	const int w = frm->width;
	const int cpitch = w<<1;
	const int fpitch = frm->pitch>>2;

	for (y = 0; y < h; y++){
		p = in+(y*fpitch);

		for (x = 0; x < w; x++){
			*(out+x) = (p[x].r&0xF8)>>3|(p[x].g&0xFC)<<3|(p[x].b&0xF8)<<8;
			__asm__ __volatile__ (PREFETCH" 128(%0)\n" :: "r" (p+x) : "memory");
		}
		out = (uint16_t*)((uint8_t*)out+cpitch);
	}
}

static inline void frame32ToBuffer16b (TFRAME *frm, uint16_t * restrict out)
{
	int y;
	unsigned int * restrict p = (unsigned int * restrict)frm->pixels;
	const int pixels = frm->width*frm->height;
	uint16_t r,g,b;

	for (y = pixels; y ; --y){
		__asm__ __volatile__ (PREFETCH" 512(%0)\n" :: "r" (p) : "memory");
		
		b = (*p&0xF80000)>>8;
		g = (*p&0xFC00)>>5;
		r = (*p&0xF8)>>3;
		*out++ = r|g|b;
		p++;
	}
}

#define DUFFS_LOOP4(pixel_copy_increment, width)	\
{   int n = (width+3)/4;							\
	 switch (width&3) {								\
	case 0: do {	pixel_copy_increment;			\
	case 3:		pixel_copy_increment;				\
	case 2:		pixel_copy_increment;				\
	case 1:		pixel_copy_increment;				\
		} while ( --n > 0 );						\
	}												\
}


#define DUFFS_LOOP8(pixel_copy_increment, width)	\
{	int n = (width+7)>>3;							\
	switch (width&7) {								\
	case 0: do {pixel_copy_increment;				\
	case 7:		pixel_copy_increment;				\
	case 6:		pixel_copy_increment;				\
	case 5:		pixel_copy_increment;				\
	case 4:		pixel_copy_increment;				\
	case 3:		pixel_copy_increment;				\
	case 2:		pixel_copy_increment;				\
	case 1:		pixel_copy_increment;				\
		} while ( --n > 0 );						\
	}												\
}

static inline int BlitRGBtoRGBPixelAlphaMMX3DNOW8 (TFRAME *src, TFRAME *des, int s_x1, int s_y1, int s_x2, int s_y2, int d_x, int d_y)
{
	if (d_x < 0){ 
		s_x1 += abs(d_x);
		d_x = 0;
	}
	if (s_x1 < 0) s_x1 = 0;
	int width = (s_x2-s_x1)+1;
	if (d_x+width >= des->width-1) width = des->width - d_x;
	if (width&7) return -1;
	
	if (d_y < 0){
		s_y1 += abs(d_y);
		d_y = 0;
	}
	if (s_y1 < 0) s_y1 = 0;
	int height = (s_y2-s_y1)+1;
	if (d_y+height >= des->height-1) height = des->height - d_y;
	if (height < 0) return -2;

	const uint32_t spitch = src->pitch>>2;
	const uint32_t dpitch = des->pitch>>2;	
	uint32_t *_srcp = (uint32_t*)lGetPixelAddress(src, s_x1, s_y1);
	uint32_t *_dstp = (uint32_t*)lGetPixelAddress(des, d_x, d_y);
	uint32_t *srcp = _srcp;
	uint32_t *dstp = _dstp;
	uint32_t amask = 0xFF000000;
	uint32_t Ashift = 24;
	
	__asm volatile (
	/* make mm6 all zeros. */
	"pxor       %%mm6, %%mm6\n"
	
	/* Make a mask to preserve the alpha. */
	"movd      %0, %%mm7\n\t"           /* 0000F000 -> mm7 */
	"punpcklbw %%mm7, %%mm7\n\t"        /* FF000000 -> mm7 */
	"pcmpeqb   %%mm4, %%mm4\n\t"        /* FFFFFFFF -> mm4 */
	"movq      %%mm4, %%mm3\n\t"        /* FFFFFFFF -> mm3 (for later) */
	"pxor      %%mm4, %%mm7\n\t"        /* 00FFFFFF -> mm7 (mult mask) */

	/* form channel masks */
	"movq      %%mm7, %%mm4\n\t"        /* 00FFFFFF -> mm4 */
	"packsswb  %%mm6, %%mm4\n\t"        /* 00000FFF -> mm4 (channel mask) */
	"packsswb  %%mm6, %%mm3\n\t"        /* 0000FFFF -> mm3 */
	"pxor      %%mm4, %%mm3\n\t"        /* 0000F000 -> mm3 (~channel mask) */
	
	/* get alpha channel shift */
	"movd      %1, %%mm5\n\t" /* Ashift -> mm5 */
	  : /* nothing */ : "rm" (amask), "rm" ((uint32_t) Ashift));


	while (height--) {
		srcp = _srcp;
		dstp = _dstp;
		
	    DUFFS_LOOP8({
		uint32_t alpha;

		alpha = *srcp & amask;
		/* FIXME: Here we special-case opaque alpha since the
		   compositioning used (>>8 instead of /255) doesn't handle
		   it correctly. Also special-case alpha=0 for speed?
		   Benchmark this! */
		   
		  __asm volatile (PREFETCH" 160(%0)\n" :: "r" (srcp) : "memory");
		   
		if (!alpha){
		    /* do nothing */
#if 1
		}else if (alpha == amask){
			/* opaque alpha -- copy RGB, keep alpha */
		    /* using MMX here to free up regular registers for other things */
			    __asm volatile (
		    "movd      (%0),  %%mm0\n\t" /* src(ARGB) -> mm0 (0000ARGB)*/
		    "movd      (%1),  %%mm1\n\t" /* dst(ARGB) -> mm1 (0000ARGB)*/
#if 1  /* keep src alpha */
		    "movd       %2,   %%mm1\n"	 
#else  /* keep dst alpha */
		    "pand      %%mm4, %%mm0\n\t" /* src & chanmask -> mm0 */
		    "pand      %%mm3, %%mm1\n\t" /* dst & ~chanmask -> mm2 */
#endif
		    "por       %%mm0, %%mm1\n\t" /* src | dst -> mm1 */
		    "movd      %%mm1, (%1) \n\t" /* mm1 -> dst */
		    
		     : : "r" (srcp), "r" (dstp), "r"(alpha));
#endif
		}else{

			    __asm volatile(

		    /* load in the source, and dst. */
		    "movd      (%0), %%mm0\n"		    /* mm0(s) = 0 0 0 0 | As Rs Gs Bs */
		    "movd      (%1), %%mm1\n"		    /* mm1(d) = 0 0 0 0 | Ad Rd Gd Bd */

		    /* Move the src alpha into mm2 */

			/* if supporting pshufw */
#if 1
		    "pshufw     $0x55, %%mm0, %%mm2\n"  /* mm2 = 0 As 0 As |  0 As  0  As */
		    "psrlw     $8, %%mm2\n" 
		    
#else
		    "movd       %2,    %%mm2\n"
		    "psrld      %%mm5, %%mm2\n"                /* mm2 = 0 0 0 0 | 0  0  0  As */
		    "punpcklwd	%%mm2, %%mm2\n"	            /* mm2 = 0 0 0 0 |  0 As  0  As */
		    "punpckldq	%%mm2, %%mm2\n"             /* mm2 = 0 As 0 As |  0 As  0  As */
		   	"pand       %%mm7, %%mm2\n"              /* to preserve dest alpha */
#endif
		    /* move the colors into words. */
		    "punpcklbw %%mm6, %%mm0\n"		    /* mm0 = 0 As 0 Rs | 0 Gs 0 Bs */
		    "punpcklbw %%mm6, %%mm1\n"              /* mm0 = 0 Ad 0 Rd | 0 Gd 0 Bd */

		    /* src - dst */
		    "psubw    %%mm1, %%mm0\n"		    /* mm0 = As-Ad Rs-Rd | Gs-Gd  Bs-Bd */

		    /* A * (src-dst) */
		    "pmullw    %%mm2, %%mm0\n"		    /* mm0 = 0*As-d As*Rs-d | As*Gs-d  As*Bs-d */
		    "psrlw     $8,    %%mm0\n"		    /* mm0 = 0>>8 Rc>>8 | Gc>>8  Bc>>8 */
		    "paddb     %%mm1, %%mm0\n"		    /* mm0 = 0+Ad Rc+Rd | Gc+Gd  Bc+Bd */

		    "packuswb  %%mm0, %%mm0\n"              /* mm0 =             | Ac Rc Gc Bc */
		    
		    "movd      %%mm0, (%1)\n"               /* result in mm0 */

		     : : "r" (srcp), "r" (dstp), "r" (alpha) );

		}
		++srcp;
		++dstp;
	    }, width);
	    _srcp += spitch;
	    _dstp += dpitch;

		__asm__ __volatile__ ("prefetch 256(%0)\n" :: "r" (dstp) : "memory");
	}

	__asm__ __volatile__ ("sfence":::"memory");
    __asm__ __volatile__ ("femms":::"memory");
    return 1;
}

static inline void BlitRGBtoRGBPixelAlphaMMX3DNOW (TFRAME *src, TFRAME *des, int s_x1, int s_y1, int s_x2, int s_y2, int d_x, int d_y)
{
	if (d_x < 0){ 
		s_x1 += abs(d_x);
		d_x = 0;
	}
	if (s_x1 < 0) s_x1 = 0;
	int width = (s_x2-s_x1)+1;
	if (d_x+width >= des->width-1) width = des->width - d_x;
	if (width&7){
		printf("exit 1\n");
		return;
	}
	
	if (d_y < 0){
		s_y1 += abs(d_y);
		d_y = 0;
	}
	if (s_y1 < 0) s_y1 = 0;
	int height = (s_y2-s_y1)+1;
	if (d_y+height >= des->height-1) height = des->height - d_y;
	if (height < 0){
		printf("exit 2\n");
		return;
	}
	
//	printf("%i %i: %i %i %i %i %i %i\n", width, height, s_x1, s_y1, s_x2, s_y2, d_x, d_y);
	
	const uint32_t spitch = src->pitch>>2;
	const uint32_t dpitch = des->pitch>>2;	
	uint32_t *_srcp = (uint32_t*)lGetPixelAddress(src, s_x1, s_y1);
	uint32_t *_dstp = (uint32_t*)lGetPixelAddress(des, d_x, d_y);
	uint32_t *srcp = _srcp;
	uint32_t *dstp = _dstp;
	uint32_t amask = 0xFF000000;
	uint32_t Ashift = 24;
	
	__asm volatile (
	/* make mm6 all zeros. */
	"pxor       %%mm6, %%mm6\n"
	
	/* Make a mask to preserve the alpha. */
	"movd      %0, %%mm7\n\t"           /* 0000F000 -> mm7 */
	"punpcklbw %%mm7, %%mm7\n\t"        /* FF000000 -> mm7 */
	"pcmpeqb   %%mm4, %%mm4\n\t"        /* FFFFFFFF -> mm4 */
	"movq      %%mm4, %%mm3\n\t"        /* FFFFFFFF -> mm3 (for later) */
	"pxor      %%mm4, %%mm7\n\t"        /* 00FFFFFF -> mm7 (mult mask) */

	/* form channel masks */
	"movq      %%mm7, %%mm4\n\t"        /* 00FFFFFF -> mm4 */
	"packsswb  %%mm6, %%mm4\n\t"        /* 00000FFF -> mm4 (channel mask) */
	"packsswb  %%mm6, %%mm3\n\t"        /* 0000FFFF -> mm3 */
	"pxor      %%mm4, %%mm3\n\t"        /* 0000F000 -> mm3 (~channel mask) */
	
	/* get alpha channel shift */
	"movd      %1, %%mm5\n\t" /* Ashift -> mm5 */

	  : /* nothing */ : "rm" (amask), "rm" ((uint32_t) Ashift) );

	while (height--) {
		srcp = _srcp;
		dstp = _dstp;

	    DUFFS_LOOP4({
		uint32_t alpha;

		alpha = *srcp & amask;
		/* FIXME: Here we special-case opaque alpha since the
		   compositioning used (>>8 instead of /255) doesn't handle
		   it correctly. Also special-case alpha=0 for speed?
		   Benchmark this! */

		__asm volatile (PREFETCH" 160(%0)\n" :: "r" (srcp) : "memory");
		
		if (!alpha){
		    /* do nothing */
#if 1
		}else if (alpha == amask){
	
			/* opaque alpha -- copy RGB, keep dst alpha */
		    /* using MMX here to free up regular registers for other things */
			    __asm volatile (
		    "movd      (%0),  %%mm0\n\t" /* src(ARGB) -> mm0 (0000ARGB)*/
		    "movd      (%1),  %%mm1\n\t" /* dst(ARGB) -> mm1 (0000ARGB)*/
#if 1  /* keep src alpha */
		    "movd       %2,   %%mm1\n"	 
#else  /* keep dst alpha */
		    "pand      %%mm4, %%mm0\n\t" /* src & chanmask -> mm0 */
		    "pand      %%mm3, %%mm1\n\t" /* dst & ~chanmask -> mm2 */
#endif
		    "por       %%mm0, %%mm1\n\t" /* src | dst -> mm1 */
		    "movd      %%mm1, (%1) \n\t" /* mm1 -> dst */

		     : : "r" (srcp), "r" (dstp), "r" (alpha) );
#endif
		}else{

			    __asm volatile(

		    /* load in the source, and dst. */
		    "movd      (%0), %%mm0\n"		    /* mm0(s) = 0 0 0 0 | As Rs Gs Bs */
		    "movd      (%1), %%mm1\n"		    /* mm1(d) = 0 0 0 0 | Ad Rd Gd Bd */

		    /* Move the src alpha into mm2 */

		    /* if supporting pshufw */
#if 1
		    "pshufw     $0x55, %%mm0, %%mm2\n"  /* mm2 = 0 As 0 As |  0 As  0  As */
		    "psrlw     $8, %%mm2\n" 
		    
#else
		    "movd       %2,    %%mm2\n"
		    "psrld      %%mm5, %%mm2\n"                /* mm2 = 0 0 0 0 | 0  0  0  As */
		    "punpcklwd	%%mm2, %%mm2\n"	            /* mm2 = 0 0 0 0 |  0 As  0  As */
		    "punpckldq	%%mm2, %%mm2\n"             /* mm2 = 0 As 0 As |  0 As  0  As */
		    "pand       %%mm7, %%mm2\n"              /* to preserve dest alpha */
#endif
		    /* move the colors into words. */
		    "punpcklbw %%mm6, %%mm0\n"		    /* mm0 = 0 As 0 Rs | 0 Gs 0 Bs */
		    "punpcklbw %%mm6, %%mm1\n"              /* mm0 = 0 Ad 0 Rd | 0 Gd 0 Bd */

		    /* src - dst */
		    "psubw    %%mm1, %%mm0\n"		    /* mm0 = As-Ad Rs-Rd | Gs-Gd  Bs-Bd */

		    /* A * (src-dst) */
		    "pmullw    %%mm2, %%mm0\n"		    /* mm0 = 0*As-d As*Rs-d | As*Gs-d  As*Bs-d */
		    "psrlw     $8,    %%mm0\n"		    /* mm0 = 0>>8 Rc>>8 | Gc>>8  Bc>>8 */
		    "paddb     %%mm1, %%mm0\n"		    /* mm0 = 0+Ad Rc+Rd | Gc+Gd  Bc+Bd */

		    "packuswb  %%mm0, %%mm0\n"              /* mm0 =             | Ac Rc Gc Bc */
		    
		    "movd      %%mm0, (%1)\n"               /* result in mm0 */

		     : : "r" (srcp), "r" (dstp), "r" (alpha) );
		     
		}
		++srcp;
		++dstp;
	    }, width);
	    _srcp += spitch;
	    _dstp += dpitch;

		__asm volatile ("prefetch 256(%0)\n" :: "r" (dstp) : "memory");
	}

	__asm__ __volatile__ ("sfence":::"memory");
    __asm__ __volatile__ ("femms":::"memory");
}

void fastFrameCopy (const TFRAME *src, TFRAME *des, int dx, int dy)
{
	const int spitch = src->pitch;
	const int dpitch = des->pitch;
	void * restrict psrc, * restrict pdes;
	int dys = 0;
	
	if (dy < 0){
		dys = abs(dy);
		if (dys >= src->height) return;
		psrc = lGetPixelAddress(src, 0, dys);
		dy = 0;
	}else{
		psrc = lGetPixelAddress(src, 0, 0);
	}
	
	if (dy > des->height-1)
		dy = des->height-1;
	pdes = lGetPixelAddress(des, dx, dy);

	int r = src->height - dys;
	if (dy + r >= des->height-1)
		r = des->height - dy;
    	
	while(r--){
		my_Memcpy(pdes, psrc, spitch);
		psrc += spitch;
		pdes += dpitch;
	}
}

static void copyi64 (uint64_t * restrict psrc, uint64_t * restrict pdes, const size_t size)
{
	int total = size>>3;
	
	  __asm__ __volatile__ (
       PREFETCH" (%0)\n"
       PREFETCH" 32(%0)\n"
       PREFETCH" 64(%0)\n"
       PREFETCH" 96(%0)\n"
       PREFETCH" 128(%0)\n"
       PREFETCH" 160(%0)\n"
       PREFETCH" 192(%0)\n"
       PREFETCH" 224(%0)\n"
       PREFETCH" 256(%0)\n"
       PREFETCH" 288(%0)\n"
    	: : "r" (psrc) );

	while (total--){
		__asm__ __volatile__ (PREFETCH" 320(%0)\n" :: "r" (psrc) : "memory");
		*pdes++ = *psrc++;
	}
	__asm__ __volatile__ ("sfence":::"memory");
    __asm__ __volatile__ (EMMS:::"memory");
}

int main (int argc, char* argv[])
{

	if (!initDemoConfig("config.cfg"))
		return 0;

	lSetBackgroundColour(hw, lGetRGBMask(frame, LMASK_BLACK));
	lSetForegroundColour(hw, lGetRGBMask(frame, LMASK_WHITE));
	
	lDeleteFrame(frame);
	frame = lNewFrame(hw, DWIDTH, DHEIGHT, LFRM_BPP_32A);
	//lLoadImage(frame, L"images/32bpp_alpha.png");
	//lLoadImage(frame, L"images/test.png");
	lLoadImageEx(frame, L"images/clock_face.png",50,0);
	//lLoadImageEx(frame, L"images/32bpp_alpha.tga", 0, 0);

	TFRAME *des = lNewFrame(hw,  frame->width, frame->height, LFRM_BPP_32A);
	lLoadImage(des, L"images/test.png");

	printf("src:%ix%i, des:%ix%i\n", frame->width, frame->height, des->width, des->height);
	
	pConverterFn converter = lGetConverter(hw, frame->bpp, des->bpp);		
	volatile int n = 0;
	const size_t size = frame->frameSize;

	setRes();
	uint64_t t1, t0;
	const float t0t = getTime();
	t0 = rdtsc();

	while(n++ < 500){
		//lDrawRectangleFilled(des, 0, 0, des->width-1, des->height-1, 0x98123456);

		//frame32ToBuffer16(frame, (uint16_t*)des->pixels);
		//frame32ToBuffer16b(frame, (uint16_t*)des->pixels);
		//rgb_32_to_16_mmx(frame->pixels, frame->width, frame->height, des->pixels, frame->pitch, des->pitch);
		converter(frame, (uint32_t*)des->pixels);
		
		//BlitRGBtoRGBPixelAlphaMMX3DNOW(frame, des, 0, 0, frame->width-1, frame->height-1, 0, 0);
		//BlitRGBtoRGBPixelAlphaMMX3DNOW8(frame, des, 0, 0, frame->width-1, frame->height-1, 0, 0);
		//lDrawImage(frame, des, 0, 0);
		//lCopyAreaScaled(frame, des, 0, 0, frame->width, frame->height, 0, 0, des->width, des->height, LCASS_CPY);
		
		//fastFrameCopy(frame, des, 0, 0);
		//my_Memcpy(des->pixels, frame->pixels, frame->frameSize);
		//memcpy(des->pixels, frame->pixels, frame->frameSize);
		//copyi64(frame->pixels, des->pixels, frame->frameSize);
		
		//r += lDrawLine(des, -1, -1, -1, -1, 0x98123456);
	}

	t1 = rdtsc();
	const float t1t = getTime();
	printf("time: %.0fms, %.4fms for %i\n", t1t-t0t, (float)((t1t-t0t)/(float)n), n);
	printf("%I64d ticks\n", (uint64_t)(t1-t0)/n);
	float mb = ((uint64_t)size * (uint64_t)n) / (float)((t1t-t0t)/1000.0);
	printf("%i MB/s \n", (int)(((mb*2.0)/1024.0)/1024.0));
	
	
	lRefresh(des);
	//lSaveImage(des, L"des.png", IMG_PNG, 0 ,0);

	demoCleanup();
	return 1;
}
