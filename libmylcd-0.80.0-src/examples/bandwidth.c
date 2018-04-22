
// network bandwidth and scrolling clock

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
#include <unistd.h>
#include <sys/types.h>
#include <windows.h>
#include <iphlpapi.h>
#include <time.h>
#include <inttypes.h>		// #define PRId64 I64d

#include "mylcd.h"
#include "demos.h"


#define s64 signed __int64

typedef struct{
	MIB_IFTABLE *m_pTable;
	DWORD iftableSize;
	int table;
	unsigned int downTotal;
	unsigned int upTotal;
	unsigned int downSpeed;
	unsigned int upSpeed;	
	unsigned int dataDownMax;
	unsigned int dataUpMax;
	unsigned int dataMax;
	unsigned int *dataDown;
	unsigned int *dataUp;
	int tdata;
}TNetSpeed;
TNetSpeed *netSpeed;


static double btm;
static double top;
static double left;
static double right;
static char clockstr[32];
static int width;
static int movPosition;
static T2POINT *Up;
static T2POINT *Dn;
static int nameTimeOut = 3;


static void update_speeds (TNetSpeed *ns);
static void update_time (char *s, int len);
static int initBandwidth (TFRAME *frame);
void drawBandwidth (TFRAME *frame);
static void bandwidthCleanUp ();


int main ()
{
	if (!initDemoConfig("config.cfg"))
		return 0;

	initBandwidth(frame);
	lSetBackgroundColour(hw, lGetRGBMask(frame, LMASK_BLACK));
	lSetForegroundColour(hw, lGetRGBMask(frame, LMASK_WHITE));
	
	do{
		lClearFrame(frame);
		drawBandwidth(frame);
		lRefresh(frame);
		lSleep(1000);
	}while (!kbhit());

	bandwidthCleanUp();
	demoCleanup();
	return 1;
}

static void bandwidthCleanUp ()
{
	free(Up);
	free(Dn);
	free(netSpeed->dataDown);
	free(netSpeed->dataUp);
	free(netSpeed->m_pTable);
	free(netSpeed);
}


static int initBandwidth (TFRAME *frame)
{
	netSpeed = (TNetSpeed *)malloc(sizeof(TNetSpeed));
	memset (netSpeed,0,sizeof(TNetSpeed));
	netSpeed->tdata = frame->width+1;
	netSpeed->dataDown = malloc(netSpeed->tdata*sizeof(unsigned int));
	netSpeed->dataUp = malloc(netSpeed->tdata*sizeof(unsigned int));
	memset(netSpeed->dataDown,0,netSpeed->tdata*sizeof(unsigned int));
	memset(netSpeed->dataUp,0,netSpeed->tdata*sizeof(unsigned int));

	netSpeed->iftableSize = 0;
	if ((GetIfTable(netSpeed->m_pTable,&netSpeed->iftableSize,TRUE)) == ERROR_INSUFFICIENT_BUFFER){
		GlobalFree (netSpeed->m_pTable);
		netSpeed->m_pTable = calloc(sizeof(MIB_IFTABLE), netSpeed->iftableSize);
		GetIfTable(netSpeed->m_pTable,&netSpeed->iftableSize,TRUE);
	}

	netSpeed->table = 1;
	/*TLPRINTR rect={0,0,0,0,0,0,0,0};
	lPrintEx(frame, &rect, LFT_SMALLFONTS7X7, 0, LPRT_XOR,(char*)netSpeed->m_pTable->table[netSpeed->table].bDescr);
	lRefresh(frame);
	lSleep(3000);*/

	GetIfTable(netSpeed->m_pTable,&netSpeed->iftableSize,TRUE);

	btm = frame->height-23;
	top = 2;
	left = 0;
	right = frame->width;
	width = (right-left)+1;
	Up = (T2POINT *)malloc(width * sizeof(T2POINT));
	Dn = (T2POINT *)malloc(width * sizeof(T2POINT));
	movPosition = 0;

	int i;
	for (i = left; i < width; i++){
		Dn[i].x = i;
		Up[i].x = i;
	}
	return 1;
}

void drawBandwidth (TFRAME *frame)
{
	const int bandwidthFont = LFTW_10x20;
	TLPRINTR rect = {0,0,frame->width-1,frame->height-1,0,0,0,0};

	update_time(clockstr, sizeof(clockstr));
	update_speeds(netSpeed);
	
	if (!nameTimeOut){
		rect.sy = frame->height-22;
		lPrintEx(frame, &rect, bandwidthFont, PF_CLIPWRAP|PF_LEFTJUSTIFY, LPRT_OR, "&#9661;%ik", netSpeed->downSpeed/1024);
		lPrintEx(frame, &rect, bandwidthFont, PF_CLIPWRAP|PF_RIGHTJUSTIFY, LPRT_OR,"&#9651;%ik", netSpeed->upSpeed/1024);
	}else{
		nameTimeOut--;
		lPrintf(frame, 0, frame->height-23, bandwidthFont, LPRT_CPY, (char*)netSpeed->m_pTable->table[netSpeed->table].bDescr);
	}

	lPrint(frame, clockstr, 14, 100, LFT_INTERDIMENSIONAL16, LPRT_CPY);
	lMoveArea(frame, 0, 100, frame->width-1,100+18, movPosition, LMOV_LOOP, LMOV_LEFT);
	if (++movPosition == width) movPosition = 0;

	double range = ((double)netSpeed->dataMax*1.18)/1024.0;
	if (range < 1.0) range = 1.0;
	double h = range / (btm-top);
	int x = left;
	double y1;
	double down,up;
	int i;

	lPrintEx(frame, &rect, bandwidthFont, PF_RESETXY|PF_CLIPWRAP|PF_LEFTJUSTIFY, LPRT_OR, "&dArr;%.1fmb", (double)(netSpeed->downTotal/1024.0/1024.0));
	lPrintEx(frame, &rect, bandwidthFont, PF_USELASTX|PF_CLIPWRAP|PF_MIDDLEJUSTIFY, LPRT_OR, "%.1fk", range);
	lPrintEx(frame, &rect, bandwidthFont, PF_USELASTX|PF_CLIPWRAP|PF_RIGHTJUSTIFY, LPRT_OR, "&uArr;%.1fmb",(double)(netSpeed->upTotal/1024.0/1024.0));
		
	for (i=0;i<width;i++){
		down = (double)netSpeed->dataDown[i];
		y1 = btm-((down/1024.0)/h);
		if (y1 < top) y1 = top;
		else if (y1>btm) y1=btm;
		Dn[x].y = y1;

		up = (double)netSpeed->dataUp[i];
		y1 = btm-((up/1024.0)/h);
		if (y1 < top) y1 = top;
		else if (y1>btm) y1=btm;
		Up[x].y = y1;
		x++;
	}

	if (frame->bpp == LFRM_BPP_32A){
		lDrawPolyLineTo(frame, Up, width, 0xFF000000|0xFF00);
		lDrawPolyLineTo(frame, Dn, width, 0xFF000000|0x00FF);
	}else{
		lDrawPolyLineTo(frame, Up, width, 0xFF00);
		lDrawPolyLineTo(frame, Dn, width, 0x00FF);
	}
}


static void update_speeds (TNetSpeed *ns)
{

	if (ns->m_pTable == NULL) return;
	
	unsigned int down = ns->downTotal;
	unsigned int up = ns->upTotal;

	if ((!ns->downTotal)||(!ns->upTotal)){
		ns->downTotal = ns->m_pTable->table[netSpeed->table].dwInOctets;
		ns->upTotal = ns->m_pTable->table[netSpeed->table].dwOutOctets;
		ns->dataDownMax = 1;
		ns->dataUpMax = 1;
		return;
	}

	GetIfTable(ns->m_pTable,&ns->iftableSize,TRUE);

	ns->downTotal = ns->m_pTable->table[ns->table].dwInOctets;
	ns->upTotal = ns->m_pTable->table[ns->table].dwOutOctets;
	ns->downSpeed = ns->downTotal - down;
	ns->upSpeed = ns->upTotal - up;
	
//	if (ns->downSpeed > ns->dataDownMax) ns->dataDownMax = ns->downSpeed;
//	if (ns->upSpeed > ns->dataUpMax) ns->dataUpMax = ns->upSpeed;
	
	int i;
	ns->dataDownMax = 1;
	ns->dataUpMax = 1;

	for (i=ns->tdata-1;i>0;i--){
		ns->dataDown[i] = ns->dataDown[i-1];
		if (ns->dataDown[i] > ns->dataDownMax) ns->dataDownMax = ns->dataDown[i];
	}
	ns->dataDown[0] = ns->downSpeed;

	for (i=ns->tdata-1;i>0;i--){
		ns->dataUp[i] = ns->dataUp[i-1];
		if (ns->dataUp[i] > ns->dataUpMax) ns->dataUpMax = ns->dataUp[i];
	}
	ns->dataUp[0] = ns->upSpeed;
	
	if (ns->dataUpMax > ns->dataDownMax)
		ns->dataMax = ns->dataUpMax;
	else
		ns->dataMax = ns->dataDownMax;
	
//	printf ("%i %i\n",ns->downSpeed,ns->dataDownMax);
}

void update_time (char *s, int len)
{
	static struct tm *tdate;

	time_t t = time(0);
	tdate = localtime(&t);
	strftime(s,len,"%H:%M",tdate);
}

