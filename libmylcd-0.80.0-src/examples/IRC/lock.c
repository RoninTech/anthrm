
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




#include "irc.h"


static sem_t semTTIRC;
static pthread_mutex_t muteTTIRC;


void TTIRC_LOCK_CREATE ()
{
	pthread_mutex_init(&muteTTIRC, 0);
	sem_init(&semTTIRC, 0, 1);
}

void TTIRC_LOCK_DELETE ()
{
	sem_destroy(&semTTIRC);
	pthread_mutex_destroy (&muteTTIRC);
}

void TTIRC_LOCK ()
{
	pthread_mutex_lock (&muteTTIRC);
	do{
	}
	while(sem_wait(&semTTIRC) != 0);
}

void TTIRC_UNLOCK ()
{
	//if (sem_post(&semTTIRC) == -1)
		//printf("TTIRC_UNLOCK(): failed to unlock semaphore\n");

	sem_post(&semTTIRC);
	pthread_mutex_unlock (&muteTTIRC);
}

