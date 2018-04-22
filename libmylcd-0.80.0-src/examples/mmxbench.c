
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

#include "../include/mylcd.h"
//#include "demos.h"
#include "../src/mmx.h"

#define MMX1_MIN_LEN 64 //0x800  /* 2K blocks */
#define MIN_LEN 0x40  /* 64-byte blocks */


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


static inline void * mmx2_memcpy (void * restrict to, void * restrict from, size_t len)
{

  void *retval = to;
  size_t i;
  
  /* PREFETCH has effect even for MOVSB instruction ;) */
/*  __asm__ __volatile__ (
    "   prefetch (%0)\n"
    "   prefetch 64(%0)\n"
    "   prefetch 128(%0)\n"
    "   prefetch 192(%0)\n"
    : : "r" (from) );
*/
 	if (len < 128){
		if (len&3){
			unsigned char *restrict src = (unsigned char *restrict)from;
			unsigned char *restrict des = (unsigned char *restrict)to;

			for (int i = 0; i < len; i++)
				*des++ = *src++;

		}else{
			unsigned int *restrict src = (unsigned int *restrict)from;
			unsigned int *restrict des = (unsigned int *restrict)to;
			len >>= 2;
			for (int i = 0; i < len; i++)
				*des++ = *src++;
		}
		return retval;
		
  	}else if(len >= MIN_LEN)
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
      "prefetch 224(%0)\n"
      
      "movq (%0), %%mm0\n"
      "movq 8(%0), %%mm1\n"
      "movq 16(%0), %%mm2\n"
      "movq 24(%0), %%mm3\n"
      "movq 32(%0), %%mm4\n"
      "movq 40(%0), %%mm5\n"
      "movq 48(%0), %%mm6\n"
      "movq 56(%0), %%mm7\n"
	  "prefetch 256(%0)\n"
      "movntq %%mm0, (%1)\n"
      "movntq %%mm1, 8(%1)\n"
      "movntq %%mm2, 16(%1)\n"
      "movntq %%mm3, 24(%1)\n"
      "movntq %%mm4, 32(%1)\n"
      "movntq %%mm5, 40(%1)\n"
      "movntq %%mm6, 48(%1)\n"
      "movntq %%mm7, 56(%1)\n"
      :: "r" (from), "r" (to) : "memory");
      
      from +=  64;
      to += 64;
    }
     /* since movntq is weakly-ordered, a "sfence"
     * is needed to become ordered again. */
    __asm__ __volatile__ ("sfence":::"memory");
    __asm__ __volatile__ ("emms":::"memory");
  }
  /*
   *	Now do the tail of the block
   */

  if (len) memcpy(to, from, len);
  return retval;
}

static inline void * mmx_memcpy (void * restrict to, void * restrict from, size_t len)
{
  void *retval;
  size_t i;
  retval = to;

	if (len < 128){
		if (len&3){
			unsigned char *restrict src = (unsigned char *restrict)from;
			unsigned char *restrict des = (unsigned char *restrict)to;

			for (int i = 0; i < len; i++)
				des[i] = src[i];

		}else{
			unsigned int *restrict src = (unsigned int *restrict)from;
			unsigned int *restrict des = (unsigned int *restrict)to;
			len >>= 2;
			for (int i = 0; i < len; i++)
				des[i] = src[i];
		}
		return retval;
		
  	}else if (len >= MMX1_MIN_LEN){
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
      //"prefetch 64(%0)\n"
      //"prefetch 96(%0)\n"
      //"prefetch 320(%0)\n"
      
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



void * mmx2_memcpy_interl (void * to, void *from, size_t len)
{

  void *retval = to;
  size_t i;
  
  /* PREFETCH has effect even for MOVSB instruction ;) */

  __asm__ __volatile__ (
    "   prefetchnta (%0)\n"
//    "   prefetchnta 32(%0)\n"
    "   prefetchnta 64(%0)\n"
//    "   prefetchnta 96(%0)\n"
    "   prefetchnta 128(%0)\n"
//    "   prefetchnta 160(%0)\n"
    "   prefetchnta 192(%0)\n"
//    "   prefetchnta 224(%0)\n"
    "   prefetchnta 256(%0)\n"
//    "   prefetchnta 288(%0)\n"
    : : "r" (from) );

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
      //"prefetch 64(%0)\n"
      //"prefetch 96(%0)\n"
      "prefetch 320(%0)\n"
      
      "movq (%0), %%mm0\n"
      "movntq %%mm0, (%1)\n"
      
      "movq 8(%0), %%mm1\n"
      "movntq %%mm1, 8(%1)\n"
      
      "movq 16(%0), %%mm2\n"
      "movntq %%mm2, 16(%1)\n"
      
      "movq 24(%0), %%mm3\n"
      "movntq %%mm2, 24(%1)\n"
      
      "movq 32(%0), %%mm4\n"
      "movntq %%mm2, 32(%1)\n"
      
      "movq 40(%0), %%mm5\n"
      "movntq %%mm2, 40(%1)\n"
      
      "movq 48(%0), %%mm6\n"
      "movntq %%mm2, 48(%1)\n"
      
      "movq 56(%0), %%mm7\n"
  	  "movntq %%mm2, 56(%1)\n"
	  
      :: "r" (from), "r" (to) : "memory");
      
      from += /*((unsigned char *)from) +*/ 64;
      to += /*((unsigned char *)to) + */64;
    }
     /* since movntq is weakly-ordered, a "sfence"
     * is needed to become ordered again. */
    //__asm__ __volatile__ ("sfence":::"memory");
    __asm__ __volatile__ ("femms":::"memory");
  }
  /*
   *	Now do the tail of the block
   */

  if (len) memcpy(to, from, len);
  return retval;
}

void * memcpy_movusb (void *to, const void *from, size_t n)
{
	size_t size;

#define STEP 0x20
#define ALIGN 0x10

	if ((unsigned long)to & (ALIGN-1)) {
		size = ALIGN - ((unsigned long)to & (ALIGN-1));
		__asm__ __volatile__("movups (%0),%%xmm0\n\t"
				     "movups %%xmm0,(%1)\n\t"
				     :
				     : "r" (from),
				     "r" (to));
		n -= size;
		from += size;
		to += size;
	}
/*
 * If the copy would have tailings, take care of them
 * now instead of later
 */
	if (n & (ALIGN-1)) {
		size = n - ALIGN;
		__asm__ __volatile__("movups (%0),%%xmm0\n\t"
				     "movups %%xmm0,(%1)\n\t"
				     :
				     : "r" (from + size),
				     "r" (to + size));
		n &= ~(ALIGN-1);
	}
/*
 * Prefetch the first two cachelines now.
 */
	__asm__ __volatile__("prefetchnta 0x00(%0)\n\t"
			     "prefetchnta 0x20(%0)\n\t"
			     :
			     : "r" (from));
	  
	while (n >= STEP) {
		
		__asm__ __volatile__(
			"movups 0x00(%0),%%xmm0\n\t"
			"movups 0x10(%0),%%xmm1\n\t"
			"movntps %%xmm0,0x00(%1)\n\t"
			"movntps %%xmm1,0x10(%1)\n\t"
			: 
			: "r" (from), "r" (to)
			: "memory");
		from += STEP;
		/*
		 * Note: Intermixing the prefetch at *exactly* this point
		 * in time has been shown to be the fastest possible.
		 * Timing these prefetch instructions is a complete black
		 * art with nothing but trial and error showing the way.
		 * To that extent, this optimum version was found by using
		 * a userland version of this routine that we clocked for
		 * lots of runs.  We then fiddled with ordering until we
		 * settled on our highest speen routines.  So, the long
		 * and short of this is, don't mess with instruction ordering
		 * here or suffer permance penalties you will.
		 */
		__asm__ __volatile__(
			"prefetchnta 32(%0)\n\t"
			: 
			: "r" (from));
			
		to += STEP;
		n -= STEP;
	}
	
	//__asm__ __volatile__ ("sfence":::"memory");
	//__asm__ __volatile__ ("femms":::"memory");
	return to;
}

static inline void * memcpy_01000101_v2 (void *s1, const void *s2, size_t n)
{
    size_t i;

    char *p1 = (char *)s1;
    const char *p2 = (const char *)s2;
    
    // this cache line will be used regardless of the count
	__asm__ __volatile__("  prefetch (%0);           \
		            prefetch 64(%0);         \
		            prefetch 128(%0);         \
		            prefetch 192(%0);         "
		         ::"r"(p2));
		         
		         
    size_t o1 = ((size_t)p1 & (16-1));
    size_t o2 = ((size_t)p2 & (16-1));

    // see if it's worth setting up the SSE loops
    if(n >= 128)
    {
        // check for the alignment and optimize the instructions accordingly. 
        if(!o1 && !o2)
        {
            /* prefetch the next iteration's cache line, 
               load 128-bytes into 8 XMM registers, 
               then store 128-bytes from those 8 XMM registers, 
               update the pointers (add 128) */
            for(i = 0; i < (n / 128); i++)
            {
                __asm__ __volatile__("  prefetch 256(%0);        \
                                prefetchnta 320(%0);        \
                                movdqa   0(%0), %%xmm0;     \
                                movdqa  16(%0), %%xmm1;     \
                                movdqa  32(%0), %%xmm2;     \
                                movdqa  48(%0), %%xmm3;     \
                                movdqa  64(%0), %%xmm4;     \
                                movdqa  80(%0), %%xmm5;     \
                                movdqa  96(%0), %%xmm6;     \
                                movdqa 112(%0), %%xmm7;     \
                                movntdq %%xmm0,   0(%1);    \
                                movntdq %%xmm1,  16(%1);    \
                                movntdq %%xmm2,  32(%1);    \
                                movntdq %%xmm3,  48(%1);    \
                                movntdq %%xmm4,  64(%1);    \
                                movntdq %%xmm5,  80(%1);    \
                                movntdq %%xmm6,  96(%1);    \
                                movntdq %%xmm7, 112(%1);    \
                                add $128, %0;               \
                                add $128, %1;               "
                             :"+S"(p2), "+D"(p1)
                             :
                             :);
            }
            
            n %= 128;
        }
        else if(o1 && !o2)
        {
            for(i = 0; i < (n / 128); i++)
            {
                __asm__ __volatile__("  prefetch 256(%0);        \
                                prefetchnta 320(%0);        \
                                movdqa   0(%0), %%xmm0;     \
                                movdqa  16(%0), %%xmm1;     \
                                movdqa  32(%0), %%xmm2;     \
                                movdqa  48(%0), %%xmm3;     \
                                movdqa  64(%0), %%xmm4;     \
                                movdqa  80(%0), %%xmm5;     \
                                movdqa  96(%0), %%xmm6;     \
                                movdqa 112(%0), %%xmm7;     \
                                movdqu %%xmm0,   0(%1);     \
                                movdqu %%xmm1,  16(%1);     \
                                movdqu %%xmm2,  32(%1);     \
                                movdqu %%xmm3,  48(%1);     \
                                movdqu %%xmm4,  64(%1);     \
                                movdqu %%xmm5,  80(%1);     \
                                movdqu %%xmm6,  96(%1);     \
                                movdqu %%xmm7, 112(%1);     \
                                add $128, %0;               \
                                add $128, %1;               "
                             :"+S"(p2), "+D"(p1)
                             :
                             :);
            }
            
            n %= 128;
        }
        else if(!o1 && o2)
        {
            for(i = 0; i < (n / 128); i++)
            {
                __asm__ __volatile__("  prefetch 256(%0);        \
                                prefetchnta 320(%0);        \
                                movdqu 0x00(%0), %%xmm0;    \
                                movdqu 0x10(%0), %%xmm1;    \
                                movdqu 0x20(%0), %%xmm2;    \
                                movdqu 0x30(%0), %%xmm3;    \
                                movdqu 0x40(%0), %%xmm4;    \
                                movdqu 0x50(%0), %%xmm5;    \
                                movdqu 0x60(%0), %%xmm6;    \
                                movdqu 0x70(%0), %%xmm7;    \
                                movntdq %%xmm0,   0(%1);    \
                                movntdq %%xmm1,  16(%1);    \
                                movntdq %%xmm2,  32(%1);    \
                                movntdq %%xmm3,  48(%1);    \
                                movntdq %%xmm4,  64(%1);    \
                                movntdq %%xmm5,  80(%1);    \
                                movntdq %%xmm6,  96(%1);    \
                                movntdq %%xmm7, 112(%1);    \
                                add $128, %0;               \
                                add $128, %1;               "
                             :"+S"(p2), "+D"(p1)
                             :
                             :);
            }
            
            n %= 128;
        }
        else
        {
            for(i = 0; i < (n / 128); i++)
            {
                __asm__ __volatile__("  prefetch 256(%0);        \
                                prefetchnta 320(%0);        \
                                movdqu   0(%0), %%xmm0;     \
                                movdqu  16(%0), %%xmm1;     \
                                movdqu  32(%0), %%xmm2;     \
                                movdqu  48(%0), %%xmm3;     \
                                movdqu  64(%0), %%xmm4;     \
                                movdqu  80(%0), %%xmm5;     \
                                movdqu  96(%0), %%xmm6;     \
                                movdqu 112(%0), %%xmm7;     \
                                movdqu %%xmm0,   0(%1);     \
                                movdqu %%xmm1,  16(%1);     \
                                movdqu %%xmm2,  32(%1);     \
                                movdqu %%xmm3,  48(%1);     \
                                movdqu %%xmm4,  64(%1);     \
                                movdqu %%xmm5,  80(%1);     \
                                movdqu %%xmm6,  96(%1);     \
                                movdqu %%xmm7, 112(%1);     \
                                add $128, %0;               \
                                add $128, %1;               "
                             :"+S"(p2), "+D"(p1)
                             :
                             :);
            }
            
            n %= 128;
        } 
        
        if(!n) return s1;
    }

    while(n != 0)
    {
        *p1++ = *p2++;
        n--;
    }
    
	return s1;
}


void bench_memcpy_01000101_v2 (void *to, void *from, size_t len, const int iter)
{
	const float t0t = getTime();
	volatile int ct = iter;
	
	while(ct--)
		memcpy_01000101_v2(to, from, len);

	const float t1t = getTime();

	uint64_t mb = ((uint64_t)len * (uint64_t)iter) / (float)((t1t-t0t)/1000.0);
	printf("%i,%i ", len, (int)(((mb<<1)>>10)>>10));
}

void bench_memcpy_movusb (void *to, void *from, size_t len, const int iter)
{
	const float t0t = getTime();
	volatile int ct = iter;
	
	while(ct--)
		memcpy_movusb(to, from, len);

	const float t1t = getTime();

	uint64_t mb = ((uint64_t)len * (uint64_t)iter) / (float)((t1t-t0t)/1000.0);
	printf("%i,%i ", len, (int)(((mb<<1)>>10)>>10));
}



void bench_mmx2_memcpy_interl (void *to, void *from, size_t len, const int iter)
{
	const float t0t = getTime();
	volatile int ct = iter;
	
	while(ct--)
		mmx2_memcpy_interl(to, from, len);

	const float t1t = getTime();

	uint64_t mb = ((uint64_t)len * (uint64_t)iter) / (float)((t1t-t0t)/1000.0);
	printf("%i,%i ", len, (int)(((mb<<1)>>10)>>10));
}

void bench_mmx_memcpy (void *to, void *from, size_t len, const int iter)
{
	const float t0t = getTime();
	volatile int ct = iter;
	
	while(ct--)
		mmx_memcpy(to, from, len);

	const float t1t = getTime();

	uint64_t mb = ((uint64_t)len * (uint64_t)iter) / (float)((t1t-t0t)/1000.0);
	printf("%i,%i ", len, (int)(((mb<<1)>>10)>>10));
}

void bench_mmx2_memcpy (void *to, void *from, size_t len, const int iter)
{
	const float t0t = getTime();
	volatile int ct = iter;
	
	while(ct--)
		mmx2_memcpy(to, from, len);

	const float t1t = getTime();

	uint64_t mb = ((uint64_t)len * (uint64_t)iter) / (float)((t1t-t0t)/1000.0);
	printf("%i,%i ", len, (int)(((mb<<1)>>10)>>10));
}

void bench_memcpy (void *to, void *from, size_t len, const int iter)
{
	const float t0t = getTime();
	volatile int ct = iter;
	
	while(ct--)
		memcpy(to, from, len);

	const float t1t = getTime();

	uint64_t mb = ((uint64_t)len * (uint64_t)iter) / (float)((t1t-t0t)/1000.0);
	printf("%i,%i ", len, (int)(((mb<<1)>>10)>>10));
}

int main (int argc, char* argv[])
{
	setRes();
	
	volatile int iter = 6000000;
	volatile int step = 4;
	volatile size_t maxSize = 128;
	char *from = malloc(maxSize);
	char *to = malloc(maxSize);
	
	for (volatile int n = step; n <= maxSize; n += step){
	//	bench_mmx_memcpy(to, from, n, iter);
		
		bench_mmx2_memcpy(to, from, n, iter);
		//bench_mmx2_memcpy_interl(to, from, n, iter);
		
		//bench_memcpy(to, from, n, iter);
		
		//bench_memcpy_movusb(to, from, n, iter);
		//bench_memcpy_01000101_v2(to, from, n, iter);
		
		printf("\n");
	}
	
	free(from);
	free(to);
	return EXIT_SUCCESS;
}
