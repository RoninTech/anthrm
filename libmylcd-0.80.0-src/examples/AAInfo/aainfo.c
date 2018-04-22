
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
#include <windows.h>

#include "mylcd.h"
#include "../../src/sync.h"
#include "../../src/g15/g15.h"
#include "../demos.h"
#include "aainfo.h"
#include "net.h"
#include "lock.h"
#include "touch.h"


char *device[] = {"USBD480:LIBUSBHID", "USBD480:DLL", "USBD480:LIBUSB"};

static const char AAMsg[] = {0x2D, 0x00, 0x00, 0xE0, 0x5D, 0x1C, 0xA6, 0x32, 254/*0x8A*/, 0x00, 0x00, 0x00, 255/*0x94*/, 0x00, 0x00, 0x00,
							0xFF, 0xFF, 0xFF, 0xFF, 0x6F, 0x00, 0x00, 0x00, 0x0C, 0xCD, 0x17, 0x00, 0x00, 0x04, 0x00, 0x00,
							0x00, 0x4D, 0x61, 0x70, 0x65, 0x69, 0x00, 0x48, 0x45, 0x4C, 0x4C, 0x4F, 0x00};

static const char AAStatus[] = {0xFE ,0xFD ,0x00 ,0xAA ,0xAA ,0xAA ,0xAB ,0xFF ,0xFF ,0xFF};

/*static const char AAInfo[] =  {0xFE, 0xFD ,0x00 ,0x03 ,0x16 ,0x22 ,0x11 ,0x14 ,0x01 ,0x04,
						0x08 ,0x0A ,0x05 ,0x06 ,0x6F ,0x10 ,0x13 ,0x64 ,0x65 ,0x66,
						0x67 ,0x68 ,0x69 ,0x6A ,0x6B ,0x6C ,0x6D ,0x6E ,0x00 ,0x00};*/
						


int AAServerConnect (TAAHOST *host, char *address, int port);
//int AAServerGetInfo (TAAHOST *host);
void AAServerDisconnect (TAAHOST *host);
int AAServerGetStatus (TAAHOST *host);

int AAServerGetMsg (TAAHOST *host);

//void dumpAAServerInfo (TAAHOST *host);
int dumpAAServerStatus (TAAHOST *host);
int processKeyPress (TAAHOST *host, char key);
void renderPlayerList (TAAHOST *host, TFRAME *frame);
void renderServerInfoPage (TAAHOST *host, TFRAME *frame);


DWORD g15keycb (int device, DWORD dwButtons, TMYLCDG15 *mylcdg15);
int initG15 (TAAHOST *host, void *ptrG15keyCB);
int initAA (TAAHOST *host, TFRAME *frame);
void cleanupAA (TAAHOST *host);
int loadServerList (TAAHOST *host, char *serverlist);
void setActiveServer (TAAHOST *host, int serverindex);
int main (int argc, char* argv[]);

static int G15Display = 0;
const int FONT = LFTW_10x20;


typedef struct{
	int id;
	float x1;
	float y1;
	float x2;
	float y2;
}TFLOATBOX;

const TFLOATBOX area[] = {
	{1, 0.0, 0.0, 0.25 ,0.3},		// top left
	{2, 0.0, 0.7, 0.25 ,1.0},		// bottom left
	{3, 0.35, 0.35, 0.65 ,0.65},	// center
	{4, 0.75, 0.0, 1.0 ,0.3},		// top right
	{5, 0.75, 0.7, 1.0 ,1.0},		// bottom right
	{0, 0.0, 0.0, 0.0 ,0.0}
};




void touchCB (TTOUCHCOORD *pos, int flags, TAAHOST *host)
{
	if (pos->pen || (flags&0x01)) return;
	
	const float x = pos->x/(float)frame->width;
	const float y = pos->y/(float)frame->height;
	
//	printf("touchCB: %i %i %i %i, %f %f\n", pos->x, pos->y, pos->pen, flags&0x01, x, y);
	
	int i,x1,y1,x2,y2;
	for (i = 0; area[i].id; i++){
		if (x >= area[i].x1 && x <= area[i].x2){
			if (y >= area[i].y1 && y <= area[i].y2){
				x1 = area[i].x1*(float)frame->width;
				y1 = area[i].y1*(float)frame->height;
				x2 = area[i].x2*(float)frame->width;
				y2 = area[i].y2*(float)frame->height;
				TTHOST_LOCK();
				switch(i+1){
				  case 1: host->state = processKeyPress(host, '0'); break;
				  case 2: host->state = processKeyPress(host, '1'); break;
				  case 3: host->state = processKeyPress(host, '2'); break;
				  case 4: host->state = processKeyPress(host, '3'); break;
				  case 5: host->state = processKeyPress(host, '4'); break;
				}
				host->touchEventSignal = 1;
				//lDrawRectangle(host->toppage, x1, y1, x2-1, y2-1, 0xF800);
				lRefreshAsync(host->toppage, 0);
				TTHOST_UNLOCK();
			}
		}
	}
}

void updatePage (TAAHOST *host)
{
	if (!host->displayPage){
		lClearFrame(host->plist.frame);
		renderPlayerList(host, host->plist.frame);
	}else{
		lClearFrame(host->status.frame);
		renderServerInfoPage(host, host->status.frame);
	}
}

int initAA (TAAHOST *host, TFRAME *frame)
{
	initSocket();
	initICMP();
	
	host->font.main = FONT;
	host->font.player = host->font.main;
	host->status.frame= lNewFrame(frame->hw, frame->width, frame->height, frame->bpp);
	host->status.highlightPos= 0;
	host->plist.frame = lNewFrame(frame->hw, frame->width, frame->height, frame->bpp);
	host->plist.highlightPos = -1;
	host->toppage = host->plist.frame;
	host->displayPage = 0;

	host->serverTotal = 0;
	host->serverCurrent = 0;
	host->server = &host->servers[host->serverCurrent];
	host->server->rsent = 0;
	host->server->rrecv = 0;

	lSetCharacterEncoding(hw, CMT_ISO8859_15);
	lSetFontLineSpacing(hw, host->font.main, lGetFontLineSpacing(hw, host->font.main)-4);
	lSetFontLineSpacing(hw, LFTW_SNAP, lGetFontLineSpacing(hw, LFTW_SNAP)-3);
	//initG15(host, g15keycb);
	
	return 1;
}

void cleanupAA (TAAHOST *host)
{
	closeICMP();
	
	if (host->plist.frame)
		lDeleteFrame(host->plist.frame);
		
	if (host->status.frame)
		lDeleteFrame(host->status.frame);
		
	host->status.frame = NULL;
	host->plist.frame = NULL;
}

void sortByPerm (TAAHOST *host, int perm)
{

	if (host->info.tPlayers){
		TAAPLAYER player;
		int i, k, n = host->info.tPlayers;

		// sort players by score, top to bottom		
		while(n--){
			i = 0;
			while(i < host->info.tPlayers){
				k = MAX(0, i-1);
				if (atoi(host->info.sv_players[i].info[perm]) > atoi(host->info.sv_players[k].info[perm])){
					memcpy(&player, &host->info.sv_players[k], sizeof(TAAPLAYER));
					memcpy(&host->info.sv_players[k], &host->info.sv_players[i], sizeof(TAAPLAYER));
					memcpy(&host->info.sv_players[i], &player, sizeof(TAAPLAYER));
				}
				i++;
			}
		}
	}
}	

int AAServerAdd (TAAHOST *host, char *address, int port)
{
	if (host->serverTotal < SERVER_MAX){
		THOSTINFO *server = &host->servers[host->serverTotal++];
		strncpy(server->address, address, 64);
		server->port = port;
		server->socket = SOCKET_ERROR;
	}
	
	return 1;
}

int loadServerList (TAAHOST *host, char *serverlist)
{
	TASCIILINE *al = readFileA(serverlist);
	if (al == NULL) return 0;
	
	int i = 0;
	char *line = NULL;
	char *rep = NULL;
	char address[128];
	int port;
	
	do{
		line = (char *)al->lines[i];
		rep = (char*)strchr(line, ';');
		if (rep) *rep = 0;
		rep = (char*)strchr(line, 9);
		if (rep) *rep = 32;
		rep = (char*)strchr(line, ':');
		if (rep) *rep = 32;
		
		*address = 0; port = 0;
		sscanf(line, "%s %i", address, &port);
		if (*address && port)
			AAServerAdd(host, address, port);
	}while(++i < al->tlines);

	freeASCIILINE(al);
	return 1;
}

void setActiveServer (TAAHOST *host, int serverindex)
{
	if (serverindex < host->serverTotal){
		host->server = &host->servers[serverindex];
		host->server->rsent = 0;
		host->server->rrecv = 0;
		host->serverCurrent = serverindex;
		host->plist.highlightPos = -1;
		memset(&host->info, 0, sizeof(TAASRVINFO));
		if (host->server->socket == SOCKET_ERROR){
			AAServerConnect(host, host->server->address, host->server->port);
			lSleep(5);
			dumpAAServerStatus(host);
			sortByPerm(host, 2);	// sort by score
			lClearFrame(host->plist.frame);
			renderPlayerList(host, host->plist.frame);
			lRefreshAsync(host->toppage, 0);
		}
	}
}

int processKeyPress (TAAHOST *host, char key)
{

	if (key == '0'){
		if (--host->serverCurrent < 0)
			host->serverCurrent = 0;
		setActiveServer(host, host->serverCurrent);
		updatePage(host);
		
	}else if (key == '1'){
		if (++host->serverCurrent == host->serverTotal)
			host->serverCurrent = 0;
		setActiveServer(host, host->serverCurrent);
		updatePage(host);
		
	}else if (key == '2'){
		host->displayPage ^= 1;
		if (!host->displayPage)
			host->toppage = host->plist.frame;
		else{
			host->toppage = host->status.frame;
		}
		updatePage(host);
	}else if (key == '3'){
		if (!host->displayPage){
			host->plist.highlightPos--;
			if (host->plist.highlightPos < -1)
				host->plist.highlightPos = -1;		
		}else{
			host->status.highlightPos--;
			if (host->status.highlightPos < 0)
				host->status.highlightPos = 0;
		}
		updatePage(host);
	}else if (key == '4'){
		if (!host->displayPage){
			host->plist.highlightPos++;
			if (host->plist.highlightPos > host->info.tPlayers-1)
				host->plist.highlightPos = host->info.tPlayers-1;		
		}else{
			host->status.highlightPos++;
			if (host->status.highlightPos > 26)
				host->status.highlightPos = 26;
		}
		updatePage(host);
	}else if (key == 'r'){		
		host->serverCurrent = 0;
		setActiveServer(host, host->serverCurrent);
		updatePage(host);
	}else if (key == 27 || key == 13){
		return 1;
	}

	return 0;
}

DWORD g15keycb (int device, DWORD dwButtons, TMYLCDG15 *mylcdg15)
{

	TAAHOST *host = (TAAHOST *)mylcdg15->ptrUser;
	TTHOST_LOCK();
	if (host->state){
		TTHOST_UNLOCK();
		return 1;
	}

	if ((dwButtons&LGLCDBUTTON_BUTTON0) && (dwButtons&LGLCDBUTTON_BUTTON3)){
		host->state = processKeyPress(host, 27);

	}else if (dwButtons&LGLCDBUTTON_BUTTON0){
		host->state = processKeyPress(host, '1');

	}else if (dwButtons&LGLCDBUTTON_BUTTON1){
		host->state = processKeyPress(host, '2');

	}else if (dwButtons&LGLCDBUTTON_BUTTON2){
		host->state = processKeyPress(host, '3');

	}else if (dwButtons&LGLCDBUTTON_BUTTON3){
		host->state = processKeyPress(host, '4');

	}
	TTHOST_UNLOCK();
	return 1;
}

char *getSV (TAASRVINFO *info, char *sv)
{
	int i;
	for (i = 0; i < info->tStrings; i++){
		if (!stricmp(info->sv_strings[i].var, sv)){
			if (info->sv_strings[i].value)
				return info->sv_strings[i].value;
			else
				break;
		}
	}
	return " ";
}

void renderServerInfoPage (TAAHOST *host, TFRAME *frame)
{
	int start = host->status.highlightPos;
	TLPRINTR rect = {0,0,0,0,0,0,0,0};
	int pf = PF_CLIPWRAP|PF_NEWLINE;
	int h = frame->height-1;
	int font = FONT;
	
	int line = 1;
	if (start < line++ && rect.ey < h)	
		lPrintEx(frame, &rect, font, pf, LPRT_CPY,"%s:%i", host->server->address, host->server->port);
	if (start < line++ && rect.ey < h){
		lPrintEx(frame, &rect, font, pf, LPRT_CPY,"packets sent: %i  received: %i", host->server->rsent, host->server->rrecv);
		rect.ey++;
		lDrawLineDotted(frame, 0, rect.ey, frame->width-1, rect.ey, 0xFFFF);
		rect.ey += 2;
	}
	
	char *value;
	int i;
	for (i = 0; i < host->info.tStrings; i++){
		if (start < line++ && rect.ey < h){
			if (host->info.sv_strings[i].value)
				value = host->info.sv_strings[i].value;
			else
				value = " ";
			lPrintEx(frame, &rect, font, pf, LPRT_CPY, "%s: %s", host->info.sv_strings[i].var, value);
		}
	}
	
	if ((start < line++ && rect.ey < h) && (host->info.tInfoFormat)){
		int i;
		for (i=0; i < host->info.tInfoFormat; i++){
			if ((start+i < line+host->info.tInfoFormat) && rect.ey < h)
				lPrintEx(frame, &rect, font, pf, LPRT_CPY," %i: %s",i+1, host->info.sv_scoreLabels[i].value);
		}
	}

	rect.ey++;
	lDrawLine(frame, 0, rect.ey, frame->width-1, rect.ey, 0xFFFF);
	if (start < line++ && rect.ey < h)
		lPrintEx(frame, &rect, LFTW_SNAP, pf, LPRT_CPY,"libmylcd v%s\nhttp://mylcd.sourceforge.net\n%s", (char*)lVersion(), mySELF);
}

void renderPlayerList (TAAHOST *host, TFRAME *frame)
{

	int start = MIN(host->plist.highlightPos, host->info.tPlayers-1);
	int end = host->info.tPlayers;
	TLPRINTR rect={0,0,0,0,0,0,0,0};

	if (!host->server->rrecv){
		lPrintEx(frame, &rect, FONT, 0, LPRT_CPY, "connecting to: %s:%i", host->server->address, host->server->port);
		rect.ey++;
		lDrawLine(frame, 0, rect.ey, frame->width-1, rect.ey, 0xFFFF);
		return;
	}

	if (start == -1){
		start = 0;
		lPrintEx(frame, &rect, FONT, PF_NODEFAULTCHAR|PF_DONTFORMATBUFFER, LPRT_CPY, getSV(&host->info, "hostname"));
		lPrintEx(frame, &rect, FONT, PF_CLIPWRAP|PF_NEWLINE, LPRT_CPY, "Mission: %s", getSV(&host->info, "mission"));
		lPrintEx(frame, &rect, FONT, PF_NEWLINE, LPRT_CPY, "Map: %s", getSV(&host->info, "mapname"));
		rect.sx = frame->width/1.4;
		lPrintEx(frame, &rect, FONT, 0, LPRT_CPY, "Type: %s",  getSV(&host->info, "gametype"));
		lPrintEx(frame, &rect, FONT, PF_RESETX|PF_NEWLINE, LPRT_CPY, "Players: %i/%s", host->info.tPlayers, getSV(&host->info, "maxplayers"));
		
		rect.sx = frame->width/2.5;
		lPrintEx(frame, &rect, FONT, 0, LPRT_CPY, "Ping: %i", host->server->echoTime);
		
		rect.sx = frame->width/1.4;
		if (*getSV(&host->info, "password") == '1'){
			lPrintEx(frame, &rect, FONT, 0, LPRT_CPY, "Password: &#8730;");
		}else{
			lPrintEx(frame, &rect, FONT, 0, LPRT_CPY, "Password: x");
			rect.ey -= 2;
		}
	
		lPrintEx(frame, &rect, FONT, PF_CLIPWRAP|PF_RESETX|PF_NEWLINE, LPRT_CPY, "Status: %s", getSV(&host->info, "gamemode"));
		rect.sx = frame->width/1.4; // - 60;

		if (*getSV(&host->info, "dedicated") == '1'){
			lPrintEx(frame, &rect, FONT, 0, LPRT_CPY, "Dedicated: &#8730;");
		}else{
			lPrintEx(frame, &rect, FONT, 0, LPRT_CPY, "Dedicated: x");
			rect.ey += 2;
		}
		
		rect.ey++;
		lDrawLine(frame, 0, rect.ey, frame->width-1, rect.ey, 0xFFFF);
		rect.ey += 2;
	}else{
		hw->render->foreGround = 0xFFFF00;
		rect.sx = 0;
		lPrintEx(frame, &rect, FONT, PF_RIGHTJUSTIFY|PF_CLIPWRAP, LPRT_CPY, "%i", host->server->echoTime);
		memset(&rect, 0, sizeof(rect));
		hw->render->foreGround = 0x969696;
	}

	int i, j;
	for (i = start; (i < end) && (rect.ey < frame->height-1); i++){
		lPrintEx(frame, &rect, FONT, PF_NODEFAULTCHAR|PF_RESETX|PF_NEWLINE|PF_DONTFORMATBUFFER, LPRT_CPY, host->info.sv_players[i].info[0]);
		
		for (j = 1; j < host->info.tInfoFormat; j++){
			if (j == 1)
				rect.sx = frame->width/2.5;
			else if (j == 2)
				rect.sx = frame->width/1.4;
			else if (j == 3)
				rect.sx = frame->width/1.15;
			lPrintEx(frame, &rect, FONT, PF_NODEFAULTCHAR|PF_DONTFORMATBUFFER, LPRT_CPY, host->info.sv_players[i].info[j]);
		}
		rect.ey -= 4;
	}

	if ((i == host->info.tPlayers) && (rect.ey < frame->height-6)){
		rect.ey += 5;
		lDrawLine(frame, 0, rect.ey, frame->width-1, rect.ey, 0xFFFF);
		rect.ey += 1;
		rect.sx = frame->width/2.5; //-65;
		lPrintEx(frame, &rect, FONT, PF_NEWLINE, LPRT_CPY, "Team");
		rect.sx = frame->width/1.4; //-65;
		lPrintEx(frame, &rect, FONT, 0, LPRT_CPY, "Score");
		rect.sx = frame->width/1.15;
		lPrintEx(frame, &rect, FONT, 0, LPRT_CPY, "Deaths");
		rect.sx = 0;
		lPrintEx(frame, &rect, FONT, 0, LPRT_CPY, "%d players", host->info.tPlayers);
	}	
	//lSaveImage(frame, "aa.bmp", IMG_BMP, frame->width, frame->height);
}

int initG15 (TAAHOST *host, void *ptrG15keyCB)
{
	lDISPLAY g15id = lDriverNameToID(hw, "G15", LDRV_DISPLAY);
	if (g15id){
		TMYLCDG15 *mylcdg15 = NULL;
		intptr_t value = G15_PRIORITY_SYNC;
		lSetDisplayOption(hw, g15id, lOPT_G15_PRIORITY, &value);
		lSetDisplayOption(hw, g15id, lOPT_G15_SOFTKEYCB, (intptr_t*)ptrG15keyCB);
		lGetDisplayOption(hw, g15id, lOPT_G15_STRUCT, (intptr_t*)mylcdg15);
		if (mylcdg15 != NULL){
			mylcdg15->ptrUser = (void*)host;
			G15Display = 1;
			return 1;
		}else{
			G15Display = 0;
			printf("initG15(): G15 not ready\n");
			return 0;
		}
	}else{
		G15Display = 0;
		return -1;
	}
}

int getValue (char *info, char *buffer, char *var)
{

	char *pos = (char*)strstr(info, var);
	if (!pos){
		return 0;
	}else{
		int len = strlen(var)+1;
		strcpy(buffer, pos + len);
		return len + strlen(buffer)+1;
	}
}

int dumpAAServerStatus (TAAHOST *host)
{

	int ret = AAServerGetStatus(host);
	char *info = &host->info.status[5];


	//ret = AAServerGetMsg(host);
	//printf("ret = %i\n",ret);

	int len = 0;
	int i = 0;
	while (*info){
		len = strlen(info);
		if (len){
			host->info.sv_strings[i].var = info;
			info += len + 1;
			len = strlen(info);
			if (len){
				host->info.sv_strings[i].value = info;
				info += len;
			}
			info++;
			i++;
			continue;
		}
		break;
	}
	host->info.tStrings = i;
	info += 2;
	host->info.tPlayers = (int)*info++;

	i = 0;
	while (*info){
		len = strlen(info);
		if (len){
			host->info.sv_scoreLabels[i++].value = info;
			info += len + 1;
			continue;
		}
		break;
	}
	host->info.tInfoFormat = i;
	info++;
	
	int j = 0;
	i = 0;
	while (*info){
		len = strlen(info);
		if (len){
			for (j = 0; j < host->info.tInfoFormat; j++){
				host->info.sv_players[i].info[j] = info;
				if (*info)
					info += strlen(info) + 1;
				else
					info++;
			}
			i++;
			continue;
		}
		break;
	}
	host->info.tPlayers = i;

#if 0
	for (i = 0; i < host->info.tStrings; i++)
		printf("%i: '%s' '%s'\n", i, host->info.sv_strings[i].var, host->info.sv_strings[i].value);

	for (i = 0; i < host->info.tInfoFormat; i++)
		printf("%i: '%s'\n", i, host->info.sv_scoreLabels[i].value);

	for (i = 0; i < host->info.tPlayers; i++){
		for (j = 0; j < host->info.tInfoFormat; j++)
			printf("'%s' ", host->info.sv_players[i].info[j]);
		printf("\n");
	}
#endif

	return ret;
}

int AAServerConnect (TAAHOST *host, char *address, int port)
{
	//strncpy(host->server->address, address, 64);
	//host->server->port = port;
	host->server->socket = connectTo((char*)host->server->address, host->server->port, &host->server->serverName);
	unsigned long a = 1; // enable nonblocking
	ioctlsocket(host->server->socket, FIONBIO, &a);
	return host->server->socket;
}

void AAServerDisconnect (TAAHOST *host)
{
	if (host->server->socket != SOCKET_ERROR){
		closeSocket(host->server->socket);
		host->server->socket = SOCKET_ERROR;
	}
}

int AAServerGetMsg (TAAHOST *host)
{
	memset(host->inbuffer, 0, sizeof(host->inbuffer));
	int sendlen = sizeof(AAMsg);
	int ret = sendSocket(host->server, &AAMsg, &sendlen);
	if (ret > 0){
		printf("sendSocket %i\n", ret);
		lSleep(1);
		ret = readSocket(host->server, host->inbuffer, sizeof(host->inbuffer));
		printf("reaSocket %i\n", ret);
	}
	return ret;
}

int AAServerGetStatus (TAAHOST *host)
{
	memset(host->inbuffer, 0, sizeof(host->inbuffer));
	int sendlen = sizeof(AAStatus);
	int ret = sendSocket(host->server, &AAStatus, &sendlen);
	if (ret > 0){
		//lSleep(50);
		ret = readSocket(host->server, host->inbuffer, sizeof(host->inbuffer));
		if (ret > 50){
			memcpy(host->info.status, host->inbuffer, ret);
		}
	}
	return ret;
}

static void getTime (TCLOCK *clk)
{
	time_t t = time(0);
	struct tm *tdate = localtime(&t);
	clk->lastsecond = clk->second;
	clk->second = tdate->tm_sec;
//	clk->minute = tdate->tm_min;
//	clk->hour = tdate->tm_hour;
	clk->tmillisecond = clock();	
}



#if 0 // a 2nd protocol returning general server state, no player details. packet is smaller
int AAServerGetInfo (TAAHOST *host)
{
	int sendlen = sizeof(AAInfo);
	int ret = sendSocket(host->server.socket, &AAInfo, &sendlen, &host->server.serverName);
	if (ret != SOCKET_ERROR){
		memset(host->server.inbuffer, 0, sizeof(host->server.inbuffer));
		ret = readSocket(host->server.socket, host->server.inbuffer, sizeof(host->server.inbuffer), &host->server.serverName);
		if (ret > 0)
			memcpy(host->info.status, host->server.inbuffer, ret);
	}
	return ret;
}

void dumpAAServerInfo (TAAHOST *host)
{
	int ret = AAServerGetInfo(host);
	char *info = &host->info.status[5];

//\hostname\hostport\numplayers\maxplayers\mapname\gametype\mission\timelimit\password\param1\param2
//currentVersion\requiredVersion\mod\equalModRequired\gameState\dedicated\platform\language\difficulty
	char name[64];			// name of server
	char players[16];		// current number of players
	char playersMax[16];	// total number of players
	char island[32];		// island map
	char missionType[32];	// game type
	char missionName[128];	// name of mission
	char gameMode[16];		// 0-open and waiting, 15-game has begun
	char hasPassword[16];	// 0-no password, 1-is passworded
	char timeToWin[16];	// (parm1) mission timeout in seconds
	char scoreToWin[16];	// (parm2) mission score needed to win
	char currentVersion[16];	//	102 (1.02), etc..
	char requiredVersion[16];
	char mod[16];			// CA
	char equalModRequired[16]; // 0-no, 1-yes
	char gameState[16];	// 1-mission selection, 3-lobby, 5-loading mission, 6-mission briefing, 7-mission started, 8-mission end, 9-debriefing
	char dedicated[16];	// 0=in game host, 1=dedicated
	char platform[32];		// host os: win, linux, etc..
	char language[16];		// language
	char difficulty[16];	// 0=regular, 1=veteran
	

	strncpy(name, info, 64);
	info += strlen(name)+2;

	strncpy(players, info, 16);
	info += strlen(players)+1;

	strncpy(playersMax, info, 16);
	info += strlen(playersMax)+1;

	strncpy(island, info, 32);
	info += strlen(island)+1;

	strncpy(missionType, info, 64);
	info += strlen(missionType)+1;
				
	strncpy(missionName, info, 64);
	info += strlen(missionName)+1;
		
	strncpy(gameMode, info, 16);
	info += strlen(gameMode)+1;
	
	strncpy(hasPassword, info, 16);
	info += strlen(hasPassword)+1;
	
	strncpy(timeToWin, info, 16);
	info += strlen(timeToWin)+1;
	
	strncpy(scoreToWin, info, 16);
	info += strlen(scoreToWin)+1;
	
	strncpy(currentVersion, info, 16);
	info += strlen(currentVersion)+1;
	
	strncpy(requiredVersion, info, 16);
	info += strlen(requiredVersion)+1;

	strncpy(mod, info, 16);
	info += strlen(mod)+1;

	strncpy(equalModRequired, info, 16);
	info += strlen(equalModRequired)+1;
	
	strncpy(gameState, info, 16);
	info += strlen(gameState)+1;
	
	strncpy(dedicated, info, 16);
	info += strlen(dedicated)+1;
	
	strncpy(platform, info, 32);
	info += strlen(platform)+1;

	strncpy(language, info, 16);
	info += strlen(language)+1;
	
	strncpy(difficulty, info, 16);
	info += strlen(difficulty)+1;
	
	printf("%i\n-%s-\n", ret, name);
	printf("-%s-\n", players);
	printf("-%s-\n", playersMax);
	printf("-%s-\n", island);
	printf("-%s-\n", missionType);
	printf("-%s-\n", missionName);
	printf("-%s-\n", gameMode);
	printf("-%s-\n", hasPassword);
	printf("-%s-\n", timeToWin);
	printf("-%s-\n", scoreToWin);
	printf("-%s-\n", currentVersion);
	printf("-%s-\n", requiredVersion);
	printf("-%s-\n", mod);
	printf("-%s-\n", equalModRequired);
	printf("-%s-\n", gameState);
	printf("-%s-\n", dedicated);
	printf("-%s-\n", platform);
	printf("-%s-\n", language);
	printf("-%s-\n", difficulty);

}
#endif


static void waitkb (TAAHOST *host, int time)
{
	while (time > 0 && !kbhit() && !host->touchEventSignal){
		time -= 50;
		lSleep(50);
	}
	host->touchEventSignal = 0;
}


int main (int argc, char **argv)
{

	if (!initDemoConfig("config.cfg"))
		return 0;

	TMYLCDTOUCH aatin;
	TMYLCDTOUCH *tin = &aatin;
	TAAHOST aahost;
	TAAHOST *host = &aahost;
	memset(host, 0, sizeof(TAAHOST));
	memset(tin, 0, sizeof(TMYLCDTOUCH));

	TTHOST_LOCK_CREATE();
	
	tin->mylcd.hw = hw;
	tin->mylcd.did = lDriverNameToID(tin->mylcd.hw, device[0], LDRV_DISPLAY);
	if (!tin->mylcd.did)
		tin->mylcd.did = lDriverNameToID(tin->mylcd.hw, device[1], LDRV_DISPLAY);
	if (!tin->mylcd.did)
		tin->mylcd.did = lDriverNameToID(tin->mylcd.hw, device[2], LDRV_DISPLAY);
	hw->render->foreGround = 0x969696;
	
	initAA(host, frame);	
	loadServerList(host, "aaserverlist.txt");
	setActiveServer(host, 0);
	startTouchDispatcherThread(tin, &host->state, touchCB, host);
	timeBeginPeriod(1);

	do{
		getTime(&host->clock);
		if (host->clock.lastsecond != host->clock.second){
			TTHOST_LOCK();
			dumpAAServerStatus(host);
			sortByPerm(host, 2);	// sort by score
			updatePage(host);
			TTHOST_UNLOCK();
		}
		TTHOST_LOCK();
		lRefreshAsync(host->toppage, 0);
		TTHOST_UNLOCK();

		if (kbhit()){
			TTHOST_LOCK();
			host->state = processKeyPress(host, getch());
			TTHOST_UNLOCK();
			lSleep(1);
		}else{
			if (host->server->rrecv && !host->displayPage){
				host->server->echoTime = getEchoTimePeriod(host->server->address, &host->server->serverName);
				if (host->server->echoTime < 1000)
					waitkb(host, 1000-host->server->echoTime);
			}else{
				waitkb(host, 1000);
			}
		}
	}while(!host->state);

	closeTouchDispatcherThread(tin);

	TTHOST_LOCK();
	int i;
	for (i = 0; i < host->serverTotal; i++){
		host->server = &host->servers[i];
		if (host->server->socket != SOCKET_ERROR)
			AAServerDisconnect(host);
	}

	timeEndPeriod(20);
	cleanupAA(host);
	TTHOST_UNLOCK();
	TTHOST_LOCK_DELETE();
	demoCleanup();
	return 1;
}


