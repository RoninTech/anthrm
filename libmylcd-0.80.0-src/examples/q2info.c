
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
#include <math.h>
#include <string.h>
#include <windows.h>

#include "mylcd.h"
#include "demos.h"
#include "irc/net.h"
#include "irc/net.c"

typedef struct{
	int		ping;
	int		score;
	char	name[64];
	char	buffer[64];
}TCLIENT;

typedef struct{
	int		port;
	char	address[64];
	int		socket;
	char	inbuffer[4096];
}TQ2HOSTINFO;

typedef struct{
	char	name[128];
	char	flags[2046];
	char	status[4096];
	int		clients;
}TQ2SRVINFO;

typedef struct{
	TQ2SRVINFO	info;
	TQ2HOSTINFO	server;
	TCLIENT		player[64];
}TQ2HOST;


static const char Q2CmdInfo[] = {0xff,0xff,0xff,0xff,'i','n','f','o',' ','3','4'};
static const char Q2CmdStatus[] = {0xff,0xff,0xff,0xff,'s','t','a','t','u','s',' ','3','4'};

static void Q2ParseClientInfo (TCLIENT *player, char *info);
int Q2ServerGetInfo (TQ2HOST *host);
int Q2ServerConnect (TQ2HOST *host, char *address, int port);
void Q2ServerDisconnect (TQ2HOST *host);
void dumpQ2ServerInfo (TFRAME *frame, TQ2HOST *host);


static void Q2ParseClientInfo (TCLIENT *player, char *info)
{
	player->score = atoi((char*)strtok(info," "));
	player->ping = atoi((char*)strtok(NULL,"\""));
	strcpy(player->name, (char*)strtok(NULL,"\""));

}

int Q2ServerConnect (TQ2HOST *host, char *address, int port)
{
	strncpy(host->server.address, address, 64);
	host->server.port = port;
	host->server.socket = connectTo(host->server.address, host->server.port, IPPROTO_UDP);
	return host->server.socket;
}

int Q2ServerGetInfo (TQ2HOST *host)
{
	int sendlen = sizeof(Q2CmdInfo);
	int ret = sendSocket(host->server.socket, &Q2CmdInfo, &sendlen);

	if (ret != SOCKET_ERROR){
		memset(host->server.inbuffer, 0, sizeof(host->server.inbuffer));
		ret = readSocket(host->server.socket, host->server.inbuffer, sizeof(host->server.inbuffer));
		if ((ret != SOCKET_ERROR) && (host->server.inbuffer[8] == 0x0A)){
			strncpy(host->info.name, strtok((char*)&host->server.inbuffer[9],"\n"),128);
			return ret;
		}
	}
	return 0;
}

int Q2ServerGetStatus (TQ2HOST *host)
{
	int sendlen = sizeof(Q2CmdStatus);
	int ret = sendSocket(host->server.socket, &Q2CmdStatus, &sendlen);
	if (ret != SOCKET_ERROR){
		memset(host->server.inbuffer, 0, sizeof(host->server.inbuffer));
		ret = readSocket(host->server.socket, host->server.inbuffer, sizeof(host->server.inbuffer));
		if ((ret != SOCKET_ERROR) && (host->server.inbuffer[9] == 0x0A)){
			strncpy(host->info.flags, strtok((char*)&host->server.inbuffer[9],"\n"),2046);
			char *temp = strtok(NULL,"\0");
			if (temp)
				strncpy(host->info.status, temp,4096);
			return 1;
		}
	}
	return 0;
}

void Q2ServerDisconnect (TQ2HOST *host)
{
	if (host->server.socket != SOCKET_ERROR){
		closeSocket(host->server.socket);
		host->server.socket = SOCKET_ERROR;
	}
}

int main (int argc, char* argv[])
{

	if (!initDemoConfig("config.cfg"))
		return 0;

	TQ2HOST q2host;
	memset(&q2host,0,sizeof(TQ2HOST));

	initSocket();
	int ret = Q2ServerConnect(&q2host, "pe0n.com", 22050);
	if (ret != SOCKET_ERROR){
		dumpQ2ServerInfo(frame, &q2host);
		Q2ServerDisconnect(&q2host);
	}

	lSaveImage(frame, L"q2.bmp", IMG_BMP, frame->width, frame->height);
	lRefresh(frame);
	lSleep(3000);
	demoCleanup();
	return 1;
}


void dumpQ2ServerInfo (TFRAME *frame, TQ2HOST *host)
{
	TLPRINTR rect = {0,0,frame->width-1,frame->height-1,0,0,0,0};
	
	int ret = Q2ServerGetInfo(host);
	ret = Q2ServerGetStatus(host);

	printf("%s\n", host->info.name);
	lPrintEx(frame, &rect, LFTW_5x7, PF_RESETXY|PF_LEFTJUSTIFY|PF_CLIPWRAP|PF_WORDWRAP, LPRT_CPY, host->info.name);
	
	char clients[4096];
	strncpy(clients, host->info.status, 4096);
	char *client = (char*)strtok((char*)clients,"\n");
	host->info.clients = 0;

	do{
		if (client)
			strcpy(host->player[host->info.clients++].buffer, client);
	}while((client=strtok(NULL,"\n")));

	int i;
	for (i = 0; i<host->info.clients; i++){
		Q2ParseClientInfo(&host->player[i], host->player[i].buffer);
		printf("%i\t%i\t%s\n",host->player[i].score, host->player[i].ping, host->player[i].name);
		lPrintEx(frame, &rect, LFTW_5x7, PF_NEWLINE|PF_RESETX|PF_LEFTJUSTIFY, LPRT_CPY,\
		  "%i\t%i\t%s\n",host->player[i].score, host->player[i].ping, host->player[i].name);
	}
}

