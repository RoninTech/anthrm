
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
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

#include "mylcd.h"
#include "demos.h"

static void updateMaskPosition (TFRAME *frame);
static void drawMask (TFRAME *frame);

#define BALLWIDTH	30

static int ballX, ballY;
static int deltaX, deltaY;
static int deltaValue = 1;



static int RED;
static int GREEN;
static int BLUE;
static int WHITE;
static int BLACK;


TFRAME *mask = NULL;
TFRAME *src = NULL;

int main ()
{
	if (!initDemoConfig("config.cfg"))
		return 0;

	int ALPHA;
	if (DBPP == LFRM_BPP_32A)
		ALPHA = 0xFF000000;
	else
		ALPHA = 0x00000000;
	
	RED =   ALPHA | lGetRGBMask(frame, LMASK_RED);
	GREEN = ALPHA | lGetRGBMask(frame, LMASK_GREEN);
	BLUE =  ALPHA | lGetRGBMask(frame, LMASK_BLUE);
	BLACK = ALPHA | lGetRGBMask(frame, LMASK_BLACK);
	WHITE = ALPHA | lGetRGBMask(frame, LMASK_WHITE);

	lSetBackgroundColour(hw, ALPHA|BLACK);
	lSetForegroundColour(hw, ALPHA|WHITE);
	lClearFrame(frame);
	
	src = lNewFrame(hw,DWIDTH, DHEIGHT, DBPP);
	mask = lNewFrame(hw,BALLWIDTH*2.1,BALLWIDTH*2.1, DBPP);
	lClearFrame(src);
	lClearFrame(mask);
	
	src->style = LSP_OR;
	lDrawCircleFilled(src, src->width/2.0, src->height/3.0, frame->width/3.0, ALPHA|RED);
	lDrawCircleFilled(src, src->width/3.0, src->height/1.5, frame->width/3.0, ALPHA|GREEN);
	lDrawCircleFilled(src, src->width/1.5, src->height/1.5, frame->width/3.0, ALPHA|BLUE);

	lDrawCircleFilled(mask,BALLWIDTH,BALLWIDTH,BALLWIDTH, ALPHA|WHITE);
	lDrawCircleFilled(mask,BALLWIDTH,BALLWIDTH,BALLWIDTH/2, ALPHA|BLACK);
	
	srand(GetTickCount());
	ballX = (rand()%DWIDTH);
	ballY = (rand()%DHEIGHT);

	if (ballX < BALLWIDTH)
		ballX = BALLWIDTH;
	else if (ballX > frame->width-BALLWIDTH-1)
		ballX = frame->width-BALLWIDTH-1;
	
	if (ballY < BALLWIDTH)
		ballY = BALLWIDTH;
	else if (ballY > frame->height-BALLWIDTH-1)
		ballY = frame->height-BALLWIDTH-1;
	
	deltaX = deltaValue;
	deltaY = deltaValue;
	frame->style = LSP_SET;
	
	int x=1000;
	
	do{
		drawMask(frame);
		lRefresh(frame);
		updateMaskPosition(frame);
		//lSleep(10);
	}while(x-- && !kbhit());

	lDeleteFrame(src);
	lDeleteFrame(mask);
	demoCleanup();
	return 0;
}


static void drawMask (TFRAME *frame)
{
	lDrawMask(src, mask, frame, ballX-BALLWIDTH, ballY-BALLWIDTH, LMASK_XOR);
}

static void updateMaskPosition (TFRAME *frame)
{

   ballX += deltaX;
   ballY += deltaY;

   if (ballX < BALLWIDTH){
      ballX = BALLWIDTH;
      deltaX = deltaValue;
      
   }else if (ballX+BALLWIDTH > frame->width-1){
      ballX = frame->width - BALLWIDTH;
      deltaX = -deltaValue;
   }
   
   if (ballY < BALLWIDTH){
      ballY = BALLWIDTH;
      deltaY = deltaValue;
      
   }else if (ballY+BALLWIDTH  > frame->height-1){
      ballY = frame->height - BALLWIDTH;
      deltaY = -deltaValue;
   }
}

