
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


#ifndef _MEMORY_H_
#define _MEMORY_H_



int initMemory();
void closeMemory();

#ifdef __DEBUG__
#define funcname __func__
#else
#define funcname ""
#endif

#define l_malloc(n)		my_malloc(n, funcname)
#define l_calloc(n, e)	my_calloc(n, e, funcname)
#define l_realloc(p, n)	my_realloc(p, n, funcname)
#define l_free(p)		my_free(p, funcname)


void *my_malloc (size_t size, const char *func)
	__attribute__((malloc));

void *my_calloc (size_t nelem, size_t elsize, const char *func)
	__attribute__((malloc));
void *my_realloc (void *ptr, size_t size, const char *func)
	__attribute__((malloc));

void *my_free (void *ptr, const char *func);

char *my_strdup (const char *str, const char *func)
	__attribute__((nonnull(1)));

wchar_t * my_wcsdup (const wchar_t *str, const char *func)
	__attribute__((nonnull(1)));

INLINE void *l_memset (void *s, int c, size_t n)
	__attribute__((nonnull(1)));
	
INLINE int l_memcmp (void *s1, const void *s2, size_t count)
	__attribute__((nonnull(1, 2)));


#ifdef __DEBUG__
void *mmx_memcpy_dbg (void *s1, const void *s2, size_t n);
void *memcpy_dbg (void *s1, const void *s2, size_t n);
#endif

#ifdef USE_MMX

# ifndef __DEBUG__
# define l_memcpy mmx_memcpy
# else
# define l_memcpy mmx_memcpy_dbg
# endif

#else

# ifndef __DEBUG__
# define l_memcpy memcpy
# else
# define l_memcpy memcpy_dbg
#endif

#endif


#define MIN_LEN			64  /* 64-byte blocks */
#define MMX1_MIN_LEN	64

#define _mmx_memcpy(to,from,len) \
{\
  size_t i;\
  if (len >= MMX1_MIN_LEN)\
  {\
    i = len >> 6;\
    len &= 63;\
    for(; i>0; i--)\
    {\
      __asm__ __volatile__ (\
      "movq (%0), %%mm0\n"\
      "movq 8(%0), %%mm1\n"\
      "movq 16(%0), %%mm2\n"\
      "movq 24(%0), %%mm3\n"\
      "movq 32(%0), %%mm4\n"\
      "movq 40(%0), %%mm5\n"\
      "movq 48(%0), %%mm6\n"\
      "movq 56(%0), %%mm7\n"\
      "movq %%mm0, (%1)\n"\
      "movq %%mm1, 8(%1)\n"\
      "movq %%mm2, 16(%1)\n"\
      "movq %%mm3, 24(%1)\n"\
      "movq %%mm4, 32(%1)\n"\
      "movq %%mm5, 40(%1)\n"\
      "movq %%mm6, 48(%1)\n"\
      "movq %%mm7, 56(%1)\n"\
      :: "r" (from), "r" (to) : "memory");\
      from += 64;\
      to += 64;\
    }\
	__asm__ __volatile__ ("sfence":::"memory");\
    __asm__ __volatile__ ("emms":::"memory");\
  }\
  if (len) memcpy(to, from, len);\
}

#define _mmx2_memcpy(to,from,len) \
{\
	size_t i;\
	__asm__ __volatile__ (\
    "   prefetch (%0)\n"\
    "   prefetch 64(%0)\n"\
    "   prefetch 128(%0)\n"\
    "   prefetch 192(%0)\n"\
    : : "r" (from) );\
	if (len >= MIN_LEN){\
        i = len >> 6;\
    len &= 63;\
    for(; i>0; i--){\
      __asm__ __volatile__ (\
      "prefetch 224(%0)\n"\
      "movq (%0), %%mm0\n"\
      "movq 8(%0), %%mm1\n"\
      "movq 16(%0), %%mm2\n"\
      "movq 24(%0), %%mm3\n"\
      "movq 32(%0), %%mm4\n"\
      "movq 40(%0), %%mm5\n"\
      "movq 48(%0), %%mm6\n"\
      "movq 56(%0), %%mm7\n"\
	  "prefetch 256(%0)\n"\
      "movntq %%mm0, (%1)\n"\
      "movntq %%mm1, 8(%1)\n"\
      "movntq %%mm2, 16(%1)\n"\
      "movntq %%mm3, 24(%1)\n"\
      "movntq %%mm4, 32(%1)\n"\
      "movntq %%mm5, 40(%1)\n"\
      "movntq %%mm6, 48(%1)\n"\
      "movntq %%mm7, 56(%1)\n"\
      :: "r" (from), "r" (to) : "memory");\
      from += 64;\
      to += 64;\
    }\
    __asm__ __volatile__ ("sfence":::"memory");\
    __asm__ __volatile__ ("emms":::"memory");\
  }\
  if (len) memcpy(to, from, len);\
}


#define mmx_memcpy(_to,_from,_len) \
{\
  	unsigned char * restrict to = (unsigned char * restrict)(_to);\
  	unsigned char * restrict from = (unsigned char * restrict)(_from);\
	size_t len = (size_t)(_len);\
 	if (len < 128){\
		if (len&3){\
			for (int i = 0; i < len; i++)\
				to[i] = from[i];\
		}else{\
			unsigned int *restrict src = (unsigned int *restrict)from;\
			unsigned int *restrict des = (unsigned int *restrict)to;\
			len >>= 2;\
			for (int i = 0; i < len; i++)\
				des[i] = src[i];\
		}\
  	}else if (len < 293000){\
		_mmx_memcpy(to,from,len);\
	}else{\
		_mmx2_memcpy(to,from,len);\
	}\
}


#endif

