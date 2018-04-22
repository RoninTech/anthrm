
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

#ifndef _AAINFO_H_
#define _AAINFO_H_

#include "mylcd.h"
#include <time.h>



#define INBUFFER_MAX	32767
#define SERVER_MAX		512
#define PLAYER_MAX		128


typedef struct{
	int		port;
	char	address[128];
	int		socket;
	struct	sockaddr_in serverName;
	unsigned int rsent;
	unsigned int rrecv;
	int	echoTime;
}THOSTINFO;

typedef struct{
	char *var;
	char *value;
}TAASERVVAR;

typedef struct{
	char *value;
}TAAPLAYERLABEL;

typedef struct{
	char *info[16]; // player score labels (name, team, score, deaths, etc..)
}TAAPLAYER;

typedef struct{
	char status[INBUFFER_MAX+1];
	TAASERVVAR sv_strings[64];
	TAAPLAYERLABEL sv_scoreLabels[16];
	TAAPLAYER sv_players[PLAYER_MAX];
	int tStrings;
	int tInfoFormat;
	int tPlayers;
}TAASRVINFO;
	
typedef struct{
	int		hour;
	int		minute;
	int		second;
	int		lastsecond;
	clock_t	tmillisecond;
}TCLOCK;

typedef struct{
	int player;		// player list font
	int main;		// font used everywhere else
}TFONTS;

typedef struct{
	int		highlightPos;
	TFRAME	*frame;
}TINFOPAGE;

typedef struct{
	THOSTINFO	servers[SERVER_MAX];
	THOSTINFO	*server;
	char		inbuffer[INBUFFER_MAX+1];
	TFRAME		*toppage;
	TAASRVINFO	info;
	TFONTS		font;
	TCLOCK		clock;
	TINFOPAGE	plist;
	TINFOPAGE	status;
	int serverCurrent;
	int	serverTotal;
	int	state;
	int	displayPage;
	
	int touchEventSignal;
	
}TAAHOST;



#define MIN(a, b) a<b?a:b
#define MAX(a, b) a>b?a:b


#endif

