
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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <windows.h>

#include "mylcd.h"
#include "demos.h"
#include "../src/mmx.h"


#define cpuid() __asm__ __volatile__("cpuid" : : : "ax", "bx", "cx", "dx")

MYLCD_EXPORT uint64_t rdtsc();
MYLCD_EXPORT void * my_Memcpy (void *s1, const void *s2, size_t n);




#ifdef HAVE_MMX2
#define MOVNTQ		"movntq"
#define PREFETCH	"prefetchnta"
#define EMMS    	"femms"
#else
#define MOVNTQ		"movq"
#define PREFETCH	"prefetch"
#define EMMS		"emms"
#endif


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

#define small_memcpy(to,from,n)\
{\
register unsigned long int dummy;\
__asm__ __volatile__(\
  "rep; movsb"\
  :"=&D"(to), "=&S"(from), "=&c"(dummy)\
  :"0" (to), "1" (from),"2" (n)\
  : "memory");\
}

/* linux kernel __memcpy (from: /include/asm/string.h) */
static __inline__ void * linux_kernel_memcpy_impl (
			       void * to,
			       const void * from,
			       size_t n)
{
int d0, d1, d2;

  if( n < 4 ) {
    small_memcpy(to,from,n);
  }
  else
    __asm__ __volatile__(
    "rep ; movsl\n\t"
    "testb $2,%b4\n\t"
    "je 1f\n\t"
    "movsw\n"
    "1:\ttestb $1,%b4\n\t"
    "je 2f\n\t"
    "movsb\n"
    "2:"
    : "=&c" (d0), "=&D" (d1), "=&S" (d2)
    :"0" (n/4), "q" (n),"1" ((long) to),"2" ((long) from)
    : "memory");

  return (to);
}

#define SSE_MMREG_SIZE 16
#define MMX_MMREG_SIZE 8

#define MMX1_MIN_LEN 0x800  /* 2K blocks */
#define MIN_LEN 0x40  /* 64-byte blocks */


static inline void setPixel32_NBr (const TFRAME *frm, const int x, const int row, const int value)
{
	*(uint32_t*)(frm->pixels+row+x) = value;
}

static void setpixel (TFRAME *frame)
{
	int x,y;
    for (y = 0; y < frame->height; y++){
        for (x = 0; x < frame->width; x++){
			lSetPixel_NB(frame, x, y, (255-y*(x>>8))<<16|x<<8|y);
        }
    }
}

static void setpixelr (TFRAME *frame)
{
	int x,y,row;
    for (y = 0; y < frame->height; y++){
    	row = y*frame->pitch;
        for (x = 0; x < frame->width; x++){
			lSetPixel_NBr(frame, x, row, (255-y*(x>>8))<<16|x<<8|y);
        }
    }
}

static void setpixelinline (TFRAME *frame)
{
	const int h = frame->width*4;
	int x,y,row;
    for (y = 0; y < frame->height; y++){
    	row = y*frame->pitch;
        for (x = 0; x < h; x+=4){
			setPixel32_NBr(frame, x, row, (255-y*(x>>10))<<16|x<<6|y);
        }
    }
}

static void setpixeldirect (TFRAME *frame)
{
	int x,y;
	unsigned int *des = (unsigned int*)frame->pixels;	
    
    for (y = 0; y < frame->height; y++){
        for (x = 0; x < frame->width; x++){
			*des++ = (255-y*(x>>8))<<16|x<<8|y;
        }
    }
}

static void array (TFRAME *frame)
{
	int x,y,r;
	unsigned char *image = (unsigned char*)frame->pixels;

    for (y = 0; y < frame->height; y++){
        for (x = 0; x < frame->width; x++){
        	r = (frame->width * y + x) * 4;
            image[r] = y;
            image[r+1] = x;			
            image[r+2] = 255-y*(x>>8);
        }
    }
}

static void mmxq (TFRAME *frame)
{
	uint64_t *p;
	int x,y;
	const int tmp255 = 255;
	const int zero = 0;
	
    for (y = 0; y < frame->height; y++){
    	p = lGetPixelAddress(frame, 0, y);
        for (x = 0; x < frame->width; x++){
			//colour1 = (255-y*(x>>8))<<16|x<<8|y;
			//x++;
			//colour2 = (uint64_t)colour1<<32 | (uint64_t)((255-y*(x>>8))<<16|x<<8|y);
			
			movd_m2r(zero, mm1);

			movd_m2r(x, mm0);
			pslld_i2r(8, mm0);	// shift x left by 8 (x<<8)
			movq_r2r(mm0, mm1);
			
			psrld_i2r(16, mm0);	// shift x right by 16 (x/256)
			movd_m2r(y, mm2);
			pmullw_r2r(mm2, mm0);	// y * (x/256)
			
			movd_m2r(tmp255, mm3);	// copy y in to mm3
			psubw_r2r(mm0, mm3);		// subtract 255 from (y*(x/256)
			
			pslld_i2r(16, mm3);		//shift left (255-(y*(x/256))) 
			por_r2r(mm3, mm1);

			por_r2r(mm2, mm1);	// OR pixel with y
			
			// 2nd pixel
			movd_m2r(++x, mm0);
			pslld_i2r(8, mm0);	// shift x left by 8 (x<<8)
			movq_r2r(mm0, mm4);
			
			psrld_i2r(16, mm0);	// shift x right by 16 (x/256)

			pmullw_r2r(mm2, mm0);	// y * (x/256)
			
			movd_m2r(tmp255, mm3);	// copy 255 in to mm3
			psubw_r2r(mm0, mm3);		// subtract 255 from (y*(x/256)
			
			pslld_i2r(16, mm3);		//shift left (255-(y*(x/256))) 
			por_r2r(mm3, mm4);

			por_r2r(mm2, mm4);	// OR y with pixel
			
			psllq_i2r(32, mm1); // shift first 32bit pixel left 32 
			por_r2r(mm1, mm4);	
			
			movq_r2m(mm4, *p++); // write pixel from mm0 in to p
        }
    }
	emms();
}


static void mmxw (TFRAME *frame)
{
	uint32_t *p;
	int x,y;
	const int tmp255 = 255;
	const int zero = 0;
	emms();
	
    for (y = 0; y < frame->height; y++){
    	p = lGetPixelAddress(frame, 0, y);
        for (x = 0; x < frame->width; x+=1){
			//colour = (255-(y*(x>>8))) <<16|x<<8|y;
			movd_m2r(zero, mm1);

			movd_m2r(x, mm0);
			pslld_i2r(8, mm0);	// shift x left by 8 (x<<8)
			movq_r2r(mm0, mm1);
			
			psrld_i2r(16, mm0);	// shift x right by 16 (x/256)
			movd_m2r(y, mm2);
			pmullw_r2r(mm2, mm0);	// y * (x/256)
			
			movd_m2r(tmp255, mm3);	// copy y in to mm3
			psubd_r2r(mm0, mm3);		// subtract 255 from (y*(x/256)
			
			pslld_i2r(16, mm3);		//shift left (255-(y*(x/256))) 
			por_r2r(mm3, mm1);

			por_r2r(mm2, mm1);	// OR pixel with y
			movd_r2m(mm1, *p++); // write pixel from mm0 in to p
        }
    }
	emms();
}

void sse2cpy (uint64_t * to, uint64_t * from, size_t n)
{
	n >>= 7;
	
	__asm__ __volatile__ (
		"prefetchnta (%0)\n"
		"prefetchnta 64(%0)\n"
		"prefetchnta 128(%0)\n"
		"prefetchnta 192(%0)\n"
		"prefetchnta 256(%0)\n"
	: : "r" (from) );
	
	while(n--){
		__asm__ __volatile__ (
		//"prefetchnta  128(%0)\n"
		//"prefetchnta  160(%0)\n"
		//"prefetchnta  192(%0)\n"
		//"prefetchnta  224(%0)\n"
		"prefetchnta  288(%0)\n"
		

		"movdqa (%0), %%xmm0\n"
		"movdqa 16(%0), %%xmm1\n"
		"movdqa 32(%0), %%xmm2\n"
		"movdqa 48(%0), %%xmm3\n"
		"movdqa 64(%0), %%xmm4\n"
		"movdqa 80(%0), %%xmm5\n"
		"movdqa 96(%0), %%xmm6\n"
		"movdqa 112(%0), %%xmm7\n"
		
		"movntdq  %%xmm0, (%1)\n"
		"movntdq  %%xmm1, 16(%1)\n"
		"movntdq  %%xmm2, 32(%1)\n"
		"movntdq  %%xmm3, 48(%1)\n"
		"movntdq  %%xmm4, 64(%1)\n"
		"movntdq  %%xmm5, 80(%1)\n"
		"movntdq  %%xmm6, 96(%1)\n"
		"movntdq  %%xmm7, 112(%1)\n"
		:: "r" (from), "r" (to) : "memory");
		
		from = ( void *)(((unsigned char *)from)+128);
		to = (void *) (((unsigned char *)to)+128);
	}
	__asm__ __volatile__ ("sfence":::"memory");
	__asm__ __volatile__ ("emms":::"memory");
}

void mmx2cpy (uint64_t *restrict to, uint64_t *restrict from, size_t n)
{
	n >>= 6;
	
	__asm__ __volatile__ (
		PREFETCH" (%0)\n"
		PREFETCH" 64(%0)\n"
		PREFETCH" 128(%0)\n"
		PREFETCH" 192(%0)\n"
		PREFETCH" 256(%0)\n"
	: : "r" (from) );
	
	while(n--){
		__asm__ __volatile__ (
		PREFETCH" 320(%0)\n"
		
		"movq (%0), %%mm0\n"
		"movq 8(%0), %%mm1\n"
		"movq 16(%0), %%mm2\n"
		"movq 24(%0), %%mm3\n"
		"movq 32(%0), %%mm4\n"
		"movq 40(%0), %%mm5\n"
		"movq 48(%0), %%mm6\n"
		"movq 56(%0), %%mm7\n"
		MOVNTQ" %%mm0, (%1)\n"
		MOVNTQ" %%mm1, 8(%1)\n"
		MOVNTQ" %%mm2, 16(%1)\n"
		MOVNTQ" %%mm3, 24(%1)\n"
		MOVNTQ" %%mm4, 32(%1)\n"
		MOVNTQ" %%mm5, 40(%1)\n"
		MOVNTQ" %%mm6, 48(%1)\n"
		MOVNTQ" %%mm7, 56(%1)\n"
		:: "r" (from), "r" (to) : "memory");
		from = ( void *)(((const unsigned char *)from)+64);
		to = (void *) (((unsigned char *)to)+64);
	}
	__asm__ __volatile__ ("sfence":::"memory");
	__asm__ __volatile__ (EMMS:::"memory");
}

static void * sse_memcpy(void * to, const void * from, size_t len)
{
  void *retval;
  size_t i;
  retval = to;
  //  fprintf(stderr, "sse_memcpy 1\n");
  
  /* PREFETCH has effect even for MOVSB instruction ;) */
/*  __asm__ __volatile__ (
    "   prefetchnta (%0)\n"
    "   prefetchnta 32(%0)\n"
    "   prefetchnta 64(%0)\n"
    "   prefetchnta 96(%0)\n"
    "   prefetchnta 128(%0)\n"
    "   prefetchnta 160(%0)\n"
    "   prefetchnta 192(%0)\n"
    "   prefetchnta 224(%0)\n"
    "   prefetchnta 256(%0)\n"
    "   prefetchnta 288(%0)\n"
    : : "r" (from) );*/

  if(len >= MIN_LEN)
  {
    register unsigned long int delta;
    /* Align destinition to MMREG_SIZE -boundary */
    delta = ((unsigned long int)to)&(SSE_MMREG_SIZE-1);
    if(delta)
    {
      delta=SSE_MMREG_SIZE-delta;
      len -= delta;
      small_memcpy(to, from, delta);
    }
    i = len >> 6; /* len/64 */
    len &= 63;
    
    if(((unsigned long)from) & 15)
      /* if SRC is misaligned */
      for(; i>0; i--)
      {
        __asm__ __volatile__ (
        "prefetch 320(%0)\n"
       //"prefetchnta 352(%0)\n"
       
        "movups (%0), %%xmm0\n"
        "movups 16(%0), %%xmm1\n"
        "movups 32(%0), %%xmm2\n"
        "movups 48(%0), %%xmm3\n"
        "movntps %%xmm0, (%1)\n"
        "movntps %%xmm1, 16(%1)\n"
        "movntps %%xmm2, 32(%1)\n"
        "movntps %%xmm3, 48(%1)\n"
        :: "r" (from), "r" (to) : "memory");
        from = ((const unsigned char *)from) + 64;
        to = ((unsigned char *)to) + 64;
      }
    else
      /*
         Only if SRC is aligned on 16-byte boundary.
         It allows to use movaps instead of movups, which required data
         to be aligned or a general-protection exception (#GP) is generated.
      */
      for(; i>0; i--)
      {
        __asm__ __volatile__ (
        "prefetch 320(%0)\n"
       //"prefetchnta 352(%0)\n"
       
        "movaps (%0), %%xmm0\n"
        "movaps 16(%0), %%xmm1\n"
        "movaps 32(%0), %%xmm2\n"
        "movaps 48(%0), %%xmm3\n"
        "movntps %%xmm0, (%1)\n"
        "movntps %%xmm1, 16(%1)\n"
        "movntps %%xmm2, 32(%1)\n"
        "movntps %%xmm3, 48(%1)\n"
        :: "r" (from), "r" (to) : "memory");
        from = ((const unsigned char *)from) + 64;
        to = ((unsigned char *)to) + 64;
      }
    /* since movntq is weakly-ordered, a "sfence"
     * is needed to become ordered again. */
    __asm__ __volatile__ ("sfence":::"memory");
    /* enables to use FPU */
    __asm__ __volatile__ ("emms":::"memory");
  }
  /*
   *	Now do the tail of the block
   */
  if(len) linux_kernel_memcpy_impl(to, from, len);
  return retval;
}

static void * mmx2_memcpy (void * to, void *from, size_t len)
{
  void *retval;
  size_t i;
  retval = to;

  /* PREFETCH has effect even for MOVSB instruction ;) */
/*
  __asm__ __volatile__ (
    "   prefetchnta (%0)\n"
    "   prefetchnta 32(%0)\n"
    "   prefetchnta 64(%0)\n"
    "   prefetchnta 96(%0)\n"
    "   prefetchnta 128(%0)\n"
    "   prefetchnta 160(%0)\n"
//    "   prefetchnta 192(%0)\n"
//    "   prefetchnta 224(%0)\n"
//    "   prefetchnta 256(%0)\n"
//    "   prefetchnta 288(%0)\n"
    : : "r" (from) );
*/
  if(len >= MIN_LEN)
  {
    /*register unsigned long int delta;
    // Align destinition to MMREG_SIZE -boundary 
    delta = ((unsigned long int)to)&(MMX_MMREG_SIZE-1);
    if(delta) {
      delta=MMX_MMREG_SIZE-delta;
      len -= delta;
      small_memcpy(to, from, delta);
    }*/
    
    i = len >> 6; /* len/64 */
    len &= 63;
    
    for(; i>0; i--)
    {
      __asm__ __volatile__ (
      //"prefetch 320(%0)\n"
      
      "movq (%0), %%mm0\n"
      "movq 8(%0), %%mm1\n"
      "movq 16(%0), %%mm2\n"
      "movq 24(%0), %%mm3\n"
      "movq 32(%0), %%mm4\n"
      "movq 40(%0), %%mm5\n"
      "movq 48(%0), %%mm6\n"
      "movq 56(%0), %%mm7\n"
      "movntq %%mm0, (%1)\n"
      "movntq %%mm1, 8(%1)\n"
      "movntq %%mm2, 16(%1)\n"
      "movntq %%mm3, 24(%1)\n"
      "movntq %%mm4, 32(%1)\n"
      "movntq %%mm5, 40(%1)\n"
      "movntq %%mm6, 48(%1)\n"
      "movntq %%mm7, 56(%1)\n"
      :: "r" (from), "r" (to) : "memory");
      from = ((unsigned char *)from) + 64;
      to = ((unsigned char *)to) + 64;
    }
     /* since movntq is weakly-ordered, a "sfence"
     * is needed to become ordered again. */
    __asm__ __volatile__ ("sfence":::"memory");
    __asm__ __volatile__ ("femms":::"memory");
  }
  /*
   *	Now do the tail of the block
   */

  if (len) memcpy(to, from, len);
  return retval;
}

static inline void * mmx_memcpy (void * restrict to, const void * restrict from, size_t len)
{
  void *retval;
  size_t i;
  retval = to;

  if (len >= MMX1_MIN_LEN)
  {
  /*  register unsigned long int delta;
    // Align destinition to MMREG_SIZE -boundary 
    delta = ((unsigned long int)to)&(MMX_MMREG_SIZE-1);
    if(delta)
    {
      delta=MMX_MMREG_SIZE-delta;
      len -= delta;
      small_memcpy(to, from, delta);
    }*/
    
    i = len >> 6; /* len/64 */
    len &= 63;
/*    
  __asm__ __volatile__ (
    "   prefetch (%0)\n"
    "   prefetch 32(%0)\n"
    "   prefetch 64(%0)\n"
    "   prefetch 96(%0)\n"
    "   prefetch 128(%0)\n"
    "   prefetch 160(%0)\n"
    "   prefetch 192(%0)\n"
    "   prefetch 224(%0)\n"
    "   prefetch 256(%0)\n"
    "   prefetch 288(%0)\n"
    : : "r" (from) );
    */
    for(; i>0; i--)
    {
      __asm__ __volatile__ (
  //    "prefetch 320(%0)\n"
      
      "movq (%0), %%mm0\n"
      "movq 8(%0), %%mm1\n"
      "movq 16(%0), %%mm2\n"
      "movq 24(%0), %%mm3\n"
      "movq 32(%0), %%mm4\n"
      "movq 40(%0), %%mm5\n"
      "movq 48(%0), %%mm6\n"
      "movq 56(%0), %%mm7\n"
      "movq %%mm0, (%1)\n"
      "movq %%mm1, 8(%1)\n"
      "movq %%mm2, 16(%1)\n"
      "movq %%mm3, 24(%1)\n"
      "movq %%mm4, 32(%1)\n"
      "movq %%mm5, 40(%1)\n"
      "movq %%mm6, 48(%1)\n"
      "movq %%mm7, 56(%1)\n"
      :: "r" (from), "r" (to) : "memory");
      from = ((unsigned char *)from) + 64;
      to = ((unsigned char *)to) + 64;
    }

	__asm__ __volatile__ ("sfence":::"memory");
    __asm__ __volatile__ ("emms":::"memory");
  }
  /*
   *	Now do the tail of the block
   */
  //if (len) linux_kernel_memcpy_impl(to, from, len);
  if (len) memcpy(to, from, len);
  return retval;
}


static void copyi32 (uint32_t * restrict psrc, uint32_t * restrict pdes, const size_t size)
{
	int total = size>>2;
	
/*  __asm__ __volatile__ (
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
*/
	while (total--){
		__asm__ __volatile__ (PREFETCH" 320(%0)\n" :: "r" (psrc) : "memory");
		
		*pdes++ = *psrc++;
	}
	
	__asm__ __volatile__ ("sfence":::"memory");
    //__asm__ __volatile__ (EMMS:::"memory");
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
		__asm__ __volatile__ ("prefetch 320(%0)\n" :: "r" (psrc) : "memory");
		
		*pdes++ = *psrc++;
	}
	
	//__asm__ __volatile__ ("sfence":::"memory");
    //__asm__ __volatile__ (EMMS:::"memory");
}

static void *copy (void * to, const void * from, size_t len)
{

	if (len > 7 && (len&7) == 0){
		copyi64((uint64_t*)from, (uint64_t*)to, len);
		return to;
	}else if (len > 3 && (len&3) == 0){
		copyi32((uint32_t*)from, (uint32_t*)to, len);
		return to;
	}

	return memcpy(to, from, len);
}

static inline void * __constant_memcpy(void * to, const void * from, size_t n)
{

    switch (n) {
    case 0:
	return to;
    case 1:
	*(unsigned char *)to = *(const unsigned char *)from;
	return to;
    case 2:
	*(unsigned short *)to = *(const unsigned short *)from;
	return to;
    case 3:
	*(unsigned short *)to = *(const unsigned short *)from;
	*(2+(unsigned char *)to) = *(2+(const unsigned char *)from);
	return to;
    case 4:
	*(unsigned long *)to = *(const unsigned long *)from;
	return to;
    case 6:	/* for Ethernet addresses */
	*(unsigned long *)to = *(const unsigned long *)from;
	*(2+(unsigned short *)to) = *(2+(const unsigned short *)from);
	return to;
    case 8:
	*(unsigned long *)to = *(const unsigned long *)from;
	*(1+(unsigned long *)to) = *(1+(const unsigned long *)from);
	return to;
    case 12:
	*(unsigned long *)to = *(const unsigned long *)from;
	*(1+(unsigned long *)to) = *(1+(const unsigned long *)from);
	*(2+(unsigned long *)to) = *(2+(const unsigned long *)from);
	return to;
    case 16:
	*(unsigned long *)to = *(const unsigned long *)from;
	*(1+(unsigned long *)to) = *(1+(const unsigned long *)from);
	*(2+(unsigned long *)to) = *(2+(const unsigned long *)from);
	*(3+(unsigned long *)to) = *(3+(const unsigned long *)from);
	return to;
    case 20:
	*(unsigned long *)to = *(const unsigned long *)from;
	*(1+(unsigned long *)to) = *(1+(const unsigned long *)from);
	*(2+(unsigned long *)to) = *(2+(const unsigned long *)from);
	*(3+(unsigned long *)to) = *(3+(const unsigned long *)from);
	*(4+(unsigned long *)to) = *(4+(const unsigned long *)from);
	return to;
	}

#define COMMON(x) \
	__asm__ __volatile__( \
	 "rep\n"\
	 "movsl" \
	 x \
	 : "=&c" (d0), "=&D" (d1), "=&S" (d2) \
	 : "0" (n>>2),"1" ((long) to),"2" ((long) from) \
	 : "memory");
	

    int d0, d1, d2;
	switch (n & 3) {
      case 0: COMMON(""); return to;
      case 1: COMMON("\n\tmovsb"); return to;
      case 2: COMMON("\n\tmovsw"); return to;
      default: COMMON("\n\tmovsw\n\tmovsb"); return to;
	}
#undef COMMON
}

int main (int argc, char* argv[])
{
	
	if (!initDemoConfig("config.cfg"))
		return 0;

	if (frame->bpp != LFRM_BPP_32){
		lDeleteFrame(frame);
		frame = lNewFrame(hw, DWIDTH, DHEIGHT, LFRM_BPP_32);
	}
	
	//lLoadImage(frame, L"images/256_clock_face.png");
	//lLoadImage(frame, L"images/shell.png");
	//lLoadImage(frame, L"images/test.png");
	
	TFRAME *mm = lNewFrame(hw, frame->width, frame->height, frame->bpp);

	uint64_t t1, t0;
	const uint64_t iter = 1024*10;
	volatile int i;

#if 0
	i = iter;
	cpuid();
	t0 = rdtsc();
	while (i--)
		setpixel(frame);
	t1 = rdtsc();
	printf("setpixel \t%I64d\n", (uint64_t)(t1-t0)/iter);
	//lSaveImage(frame, L"setpixel.png", IMG_PNG, 0 ,0);
	
	i = iter;
	cpuid();
	t0 = rdtsc();
	while (i--)
		setpixelr(frame);
	t1 = rdtsc();
	printf("setpixel row \t%I64d\n", (uint64_t)(t1-t0)/iter);
	//lSaveImage(frame, L"setpixelrow.png", IMG_PNG, 0 ,0);
	
	i = iter;
	cpuid();
	t0 = rdtsc();
	while (i--)
		setpixelinline(frame);
	t1 = rdtsc();
	printf("setpixel inline\t%I64d\n", (uint64_t)(t1-t0)/iter);
	//lSaveImage(frame, L"setpixelinline.png", IMG_PNG, 0 ,0);

	i = iter;
	cpuid();
	t0 = rdtsc();
	while (i--)
		setpixeldirect(frame);
	t1 = rdtsc();
	printf("setpixel mem\t%I64d\n", (uint64_t)(t1-t0)/iter);	
	//lSaveImage(frame, L"setpixelmem.png", IMG_PNG, 0 ,0);
	
	i = iter;
	cpuid();
	t0 = rdtsc();
	while (i--)
		array(frame);
	t1 = rdtsc();
	printf("setpixel array \t%I64d\n", (uint64_t)(t1-t0)/iter);
	//lSaveImage(frame, L"array.png", IMG_PNG, 0 ,0);

	i = iter;
	cpuid();
	t0 = rdtsc();
	while (i--)
		mmxq(frame);
	t1 = rdtsc();
	printf("mmx quad \t%I64d\n", (uint64_t)(t1-t0)/iter);
	//lSaveImage(frame, L"mmxquad.png", IMG_PNG, 0 ,0);

	i = iter;
	cpuid();
	t0 = rdtsc();
	while (i--)
		mmxw(frame);
	t1 = rdtsc();
	printf("mmx word \t%I64d\n", (uint64_t)(t1-t0)/iter);
	//lSaveImage(frame, L"mmxdword.png", IMG_PNG, 0 ,0);

	lClearFrame(mm);
	
	i = iter;
	cpuid();
	t0 = rdtsc();
	while (i--)
		memcpy(mm->pixels, frame->pixels, frame->frameSize);
	t1 = rdtsc();
	printf("memcpy\t\t%I64d\n", (uint64_t)(t1-t0)/iter);
	//lSaveImage(mm, L"memcpy.png", IMG_PNG, 0 ,0);

	lClearFrame(mm);

	i = iter;
	cpuid();
	t0 = rdtsc();
	while (i--){
		uint64_t *psrc = (uint64_t*)frame->pixels;
		uint64_t *pdes = (uint64_t*)mm->pixels;
		int n = (frame->height * frame->width)>>1;
		while(n--)
			*pdes++ = *psrc++;
	}
	t1 = rdtsc();
	printf("memcpy direct\t%I64d\n", (uint64_t)(t1-t0)/iter);
	//lSaveImage(mm, L"d.png", IMG_PNG, 0 ,0);

	lClearFrame(mm);
#endif


	//i = iter;
	uint64_t *from = (uint64_t*)frame->pixels;
	uint64_t *to = (uint64_t*)mm->pixels;
	const size_t size = mm->frameSize;
	volatile int n = i;
	
	cpuid();
	setRes();
	const float t0t = getTime();
	t0 = rdtsc();

	//printf("%i %i\n", size&63, size&127);
	printf("block size: %i\n", size);
	
	while(++n < 5000){
		//mmx2cpy(to, from, size);
		//sse2cpy(to, from, size);
		
		//copy(to, from, size);
		
		//mmx_memcpy(to, from, size);
		//mmx2_memcpy(to, from, size);
		//__builtin_memcpy(to, from, size);
		//sse_memcpy(to, from, size);
		my_Memcpy(to, from, size);
		//memcpy(to, from, size);
		//memmove(to, from, size);
		//__constant_memcpy(to, from, size);
	}

	t1 = rdtsc();
	const float t1t = getTime();
	printf("time: %.0fms, %.4fms for %i\n", t1t-t0t, (float)((t1t-t0t)/(float)n), n);
	printf("memcpy mmx\t%I64d ticks\n", (uint64_t)(t1-t0)/n);
	float mb = ((uint64_t)size * (uint64_t)n) / (float)((t1t-t0t)/1000.0);
	printf("%i MB/s \n", (int)(((mb*2.0)/1024.0)/1024.0));
	
	
	//lSaveImage(mm, L"mm.png", IMG_PNG, 0 ,0);	
	
	lRefresh(mm);
	lDeleteFrame(mm);
	demoCleanup();
	return 1;
}

