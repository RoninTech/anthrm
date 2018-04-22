
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


#include "mylcd.h"

#if (__BUILD_APISYNC_SUPPORT__)

#include "memory.h"
#include "sync.h"
#include "apilock.h"


void *createAPILock ()
{
	TTHRDSYNCCTRL *tsc = (TTHRDSYNCCTRL*)l_calloc(1, sizeof(TTHRDSYNCCTRL));
	lock_create(tsc);
	return (void*)tsc;
}

void closeAPILock (void *tsc)
{
	lock_delete((TTHRDSYNCCTRL*)tsc);
	l_free(tsc);
}

#else

void *createAPILock (){return NULL;};
void closeAPILock (void *tsc){};

#endif

