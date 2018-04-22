
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




#include <string.h>

#include "mylcd.h"
#include "memory.h"
#include "utils.h"
#include "lstring.h"
#include "convert.h"
#include "sync.h"
#include "misc.h"


#define MAXMEMALLOC ((size_t)(200*1024*1024))
#ifndef ALIGN_TYPE		
#define ALIGN_TYPE int
#endif


#ifdef __DEBUG__
typedef struct{
	size_t size;	// current number of bytes allocated. should be zero at exit
	int	count;		// delta count of c/malloc and free
	int allocCount;
	int reallocCount;
	int freeCount;
	int memcpyCount;
	int memsetCount;
	size_t maxMemRequest;
	size_t peak;
}TMEMSTATS;
static TMEMSTATS memstats;


typedef union memhdr_struct{
  struct {
    size_t size;
    unsigned int id;
  }m;
  ALIGN_TYPE align;		/* included in union to ensure alignment */
}TMEMHDR;

#define HDREXTRA sizeof(TMEMHDR)
#endif


int initMemory ()
{

#ifdef __DEBUG__
	memstats.size = 0;
	memstats.count = 0;
	memstats.freeCount = 0;
	memstats.allocCount = 0;
	memstats.memcpyCount = 0;
	memstats.memsetCount = 0;
	memstats.reallocCount = 0;
	memstats.maxMemRequest = 0;
	memstats.peak = 0;
#endif
	return 1;
}

void closeMemory ()
{
#ifdef __DEBUG__
	mylog("malloc:\t%i\nfree:\t%i\nleaks:\t%i\n",memstats.allocCount,\
	  memstats.freeCount,memstats.allocCount-memstats.freeCount);
	mylog("realloc:%i\nmemcpy:\t%i\nmemset:\t%i\n",memstats.reallocCount,\
	  memstats.memcpyCount,memstats.memsetCount);
	mylog("max mem request:%ik\n",memstats.maxMemRequest>>10);
	mylog("memstat peak:%uk\n", memstats.peak>>10);
	mylog("memstat size:%u\n", memstats.size);
	mylog("memstat count:%i\n", memstats.count);
#endif
}
char * my_strdup (const char *str, const char *func)
{
	if (str){
		size_t n = l_strlen(str);
		char *p = (char*)my_malloc(sizeof(char)* n+sizeof(char), func);
		if (p){
#ifdef __DEBUG_MEMUSAGE__
			mylog("my_strdup %p %i\n", (void*)p-HDREXTRA, n);
#endif
			p[n] = 0;
			return l_strncpy(p, str, n);
		}
	}
	return NULL;
}

wchar_t * my_wcsdup (const wchar_t *str, const char *func)
{
	size_t n = l_wcslen(str);
	wchar_t *p = (wchar_t*)my_malloc(sizeof(wchar_t)*n + sizeof(wchar_t), func);
	if (p){
#ifdef __DEBUG_MEMUSAGE__
		mylog("my_wcsdup %p %i\n", (void*)p-HDREXTRA, n);
#endif
		p[n] = 0;
		return l_wcsncpy(p, str, n);
	}
	return NULL;
}

static size_t alignSize (size_t sizeofobject)
{
	size_t odd_bytes = sizeofobject % sizeof(ALIGN_TYPE);
	if (odd_bytes > 0)
		sizeofobject += sizeof(ALIGN_TYPE) - odd_bytes;
	return sizeofobject;
}

void * my_malloc (size_t size, const char *func)
{
	if (((int)size == -1) || !size){
		mylog("libmylcd: bad l_malloc request, invalid size %i\n", size);
		return NULL;
	}
#ifdef __DEBUG__
	if (size > MAXMEMALLOC){
		mylog("libmylcd: %s: l_malloc request exceed size limit: %u %u\n", func, size, MAXMEMALLOC);
		return NULL;
	}
	TMEMHDR *hdr = malloc(alignSize(size+HDREXTRA));
	if (hdr == NULL){
		mylog("libmylcd: malloc() returned a NULL pointer\n");
		return NULL;
	}
	hdr->m.size = size;
	hdr->m.id = memstats.allocCount;
	memstats.size += size;
	memstats.count++;
	memstats.allocCount++;
	memstats.maxMemRequest = MAX(memstats.maxMemRequest, size);
#ifdef __DEBUG_MEMUSAGE__
	mylog("malloc: %s: %u %p %i: %i\n", func, hdr->m.id, hdr, hdr->m.size, memstats.size);
#endif
	return (void*)hdr+HDREXTRA;
#else
	return malloc(alignSize(size));
#endif
}

void * my_calloc (size_t nelem, size_t elsize, const char *func)
{
	if (!nelem || !elsize){
		mylog("libmylcd: bad l_calloc request, invalid size %i,%i\n",nelem, elsize);
		return NULL;
	}
	size_t size = nelem*elsize;
	void *p = my_malloc(size, func);
	if (p)
		l_memset(p, 0, size);
	return p;
}

void * my_realloc (void *ptr, size_t size, const char *func)
{
#ifdef __DEBUG__
	if (!size || ((int)size == -1)){
		mylog("libmylcd: bad l_realloc request, invalid size: ptr:%p size:%u\n", ptr, size);
		return NULL;
	}else if (size > MAXMEMALLOC){
		mylog("libmylcd: l_realloc request exceed size limit: %u %u\n", size, MAXMEMALLOC);
		return NULL;
	}
	if (ptr == NULL){  // libmylcd should not pass a NULL ptr to l_realloc.
	 	mylog("libmylcd: l_realloc warning: NULL pointer supplied. size = %u\n",size);
		return my_malloc(size, func);
	}
	ptr -= HDREXTRA;
	TMEMHDR *hdr = (TMEMHDR*)ptr;
	memstats.size -= hdr->m.size;
	unsigned int tmpid = hdr->m.id;
	hdr = realloc(ptr, alignSize(size+HDREXTRA));
	if (hdr == NULL){
		mylog("libmylcd: realloc() returned a NULL pointer\n");
		return ptr+HDREXTRA;
	}
	hdr->m.size = size;
	hdr->m.id = tmpid;
	memstats.size += size;
	memstats.reallocCount++;
	memstats.maxMemRequest = MAX(memstats.maxMemRequest, size);
#ifdef __DEBUG_MEMUSAGE__
	mylog("realloc: %s: %u %p -> %p %i %u\n", func, hdr->m.id, ptr, hdr, hdr->m.size, memstats.size);
#endif
	return (void*)hdr+HDREXTRA;
#else
	return realloc(ptr, alignSize(size));
#endif
}

void * my_free (void *ptr, const  char *func)
{
	if (ptr){
#ifdef __DEBUG__
		ptr -= HDREXTRA;
		TMEMHDR *hdr = (TMEMHDR*)ptr;
		if (memstats.size > memstats.peak)
			memstats.peak = memstats.size;
		memstats.size -= hdr->m.size;
		memstats.count--;
		memstats.freeCount++;
#ifdef __DEBUG_MEMUSAGE__
		mylog("free: %s: %u %p %i %u\n", func, hdr->m.id, ptr, hdr->m.size, memstats.size);
#endif
#endif
		free(ptr);
	}
	return NULL;
}

INLINE int l_memcmp (void *s1, const void *s2, size_t count)
{
#ifdef __DEBUG__
	if (s1 == NULL || s2 == NULL || !count){
	  	mylog("libmylcd: bad l_memcmp request, invalid size or pointer\n");
	  	return 1;
	}
#endif
	return memcmp(s1, s2, count);
}

INLINE void *l_memset (void *s, int c, size_t count)
{
#ifdef __DEBUG__
	  if ((s == NULL) || !count){
	  	mylog("libmylcd: bad l_memset request, invalid size or pointer\n");
	  	return NULL;
	  }else{
		memstats.memsetCount++;
	  }
#endif
	return memset(s, c, count);
}


#ifdef __DEBUG__
void *mmx_memcpy_dbg (void *s1, const void *s2, size_t n)
{
	memstats.memcpyCount++;
	mmx_memcpy(s1, s2, n);
	return s1;
}

void *memcpy_dbg (void *s1, const void *s2, size_t n)
{
	memstats.memcpyCount++;
	return memcpy(s1, s2, n);
}
#endif
