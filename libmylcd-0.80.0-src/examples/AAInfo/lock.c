
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


#include <pthread.h>
#include <semaphore.h>


static sem_t semTTHOST;
static pthread_mutex_t muteTTHOST;


void TTHOST_LOCK_CREATE ()
{
	pthread_mutex_init(&muteTTHOST, 0);
	sem_init(&semTTHOST, 0, 1);
}

void TTHOST_LOCK_DELETE ()
{
	sem_destroy(&semTTHOST);
	pthread_mutex_destroy(&muteTTHOST);
}

void TTHOST_LOCK ()
{
	pthread_mutex_lock (&muteTTHOST);
	do{
	}
	while(sem_wait(&semTTHOST) != 0);
}

void TTHOST_UNLOCK ()
{
	//if (sem_post(&semTTHOST) == -1)
		//printf("TTHOST_UNLOCK(): failed to unlock semaphore\n");

	sem_post(&semTTHOST);
	pthread_mutex_unlock (&muteTTHOST);
}



#if 0

static sem_t semRecvIRC;

void RECVIRC_LOCK_CREATE ()
{
	sem_init(&semRecvIRC, 0, 1);
}

void RECVIRC_LOCK_DELETE ()
{
	sem_destroy(&semRecvIRC);
}

void RECVIRC_LOCK ()
{
	do{
	}
	while (sem_wait(&semRecvIRC) != 0);
}

void RECVIRC_UNLOCK ()
{
	if (sem_post(&semRecvIRC) == -1)
		printf("RECVIRC_UNLOCK(): failed to unlock semaphore\n");
}
#endif

