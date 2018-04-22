
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
#include "utils.h"


#if 0
MYLCD_EXPORT int l_getFrameWidth (TFRAME *frame)
{
	if (frame)
		return frame->width;
	else
		return -1;
}

MYLCD_EXPORT int l_getFrameHeight (TFRAME *frame)
{
	if (frame)
		return frame->height;
	else
		return -1;
}

MYLCD_EXPORT int l_getFramePitch (TFRAME *frame)
{
	if (frame)
		return frame->pitch;
	else
		return -1;
}

MYLCD_EXPORT int l_getFrameBPP (TFRAME *frame)
{
	if (frame)
		return frame->bpp;
	else
		return -1;
}

MYLCD_EXPORT int l_getFrameSize (TFRAME *frame)
{
	if (frame)
		return frame->frameSize;
	else
		return -1;
}

MYLCD_EXPORT void *l_getFrameUData (TFRAME *frame)
{
	if (frame)
		return frame->udata;
	else
		return NULL;
}

MYLCD_EXPORT int l_setFrameUData (TFRAME *frame, void *udata)
{
	if (frame){
		frame->udata = udata;
		return 1;
	}
	return 0;
}
#endif

/*
MYLCD_EXPORT void cpuid ()
{
#if (defined(__GNUC__))
	  __asm volatile("cpuid" : : : "ax", "bx", "cx", "dx");
#endif
}
*/
MYLCD_EXPORT uint64_t rdtsc ()
{
	volatile uint32_t __a,__d;
    __asm volatile("rdtsc" : "=a" (__a), "=d" (__d));
	return ((uint64_t)__d << 32) + (uint64_t)__a;
}

