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
#include <string.h>
#include <sys/types.h>

#include "mylcd.h"
#include "demos.h"

#ifndef DEGTORAD
#define DEGTORAD 0.034906585039
#endif


typedef struct{
	float x[8];
	float y[8];
	float z[8];
}TCUBE;

int initCube (TCUBE *cube);
int RotateCube (TCUBE *in, TCUBE *out, float anglex, float angley, float anglez);
void drawCube (TFRAME *frame, TCUBE *cube);

float flength=260;
float camz=-10;
int destx;
int desty;
static int BACK, INK;


int main ()
{
	if (!initDemoConfig("config.cfg"))
		return 0;

	BACK = lGetRGBMask(frame, LMASK_WHITE);
	if (frame->bpp != LFRM_BPP_1)
		INK = lGetRGBMask(frame, LMASK_RED)|lGetRGBMask(frame, LMASK_BLUE);
	else
		INK = lGetRGBMask(frame, LMASK_BLACK);

	lSetBackgroundColour(hw, BACK);
	lSetForegroundColour(hw, INK);
	lClearFrame(frame);

	destx = DWIDTH/2;
	desty = DHEIGHT/2;
	TCUBE cube;
	TCUBE cube2;
	initCube(&cube);
	
	float x0=99,y0=100,z0=99;
	RotateCube(&cube,&cube2,x0,y0,z0);
	drawCube(frame,&cube2);
	lRefresh(frame);
//	lSleep(1000);

	float a;
	const float step = .80;
 	int tframes = 0;
	int start = GetTickCount();

	for (a=0.0;a<360.0+step;a+=step){
		lClearFrame(frame);
		//memset(lGetPixelAddress(frame, 0, 0), 0xFF, frame->frameSize);
		
		RotateCube(&cube, &cube2, x0, y0+a, z0);
		drawCube(frame,&cube2);
		//lRefresh(frame);
		// or
		lUpdate(hw, lGetPixelAddress(frame, 0, 0), frame->frameSize);
		//lSleep(2);
		tframes++;
	}

	lSaveImage(frame, L"c.png", IMG_PNG, 0,0);

	int end = GetTickCount();
	float time = (float)(end-start)*0.001;
	printf("frames:%d\ntime:%.2fs\nfps:%.1f\n\n",tframes,time,tframes/time);

	demoCleanup();
	return 1;
}

void drawCube (TFRAME *frame, TCUBE *cube)
{
	int sx,sy,sx2,sy2,i;

	for (i=0;i<7;i++){
		lPoint3DTo2D(cube->x[i],-cube->y[i],cube->z[i],flength,camz,destx,desty,&sx,&sy);
		lPoint3DTo2D(cube->x[i+1],-cube->y[i+1],cube->z[i+1],flength,camz,destx,desty,&sx2,&sy2);
		lDrawLine(frame,sx,sy,sx2,sy2,INK);
	}
	
	i = 7;
	lPoint3DTo2D(cube->x[i],-cube->y[i],cube->z[i],flength,camz,destx,desty,&sx,&sy);
	i = 2;
	lPoint3DTo2D(cube->x[i],-cube->y[i],cube->z[i],flength,camz,destx,desty,&sx2,&sy2);
	lDrawLine(frame,sx,sy,sx2,sy2, INK);


	i = 0;
	lPoint3DTo2D(cube->x[i],-cube->y[i],cube->z[i],flength,camz,destx,desty,&sx,&sy);
	i = 5;
	lPoint3DTo2D(cube->x[i],-cube->y[i],cube->z[i],flength,camz,destx,desty,&sx2,&sy2);
	lDrawLine(frame,sx,sy,sx2,sy2,INK);	
	
	i = 1;
	lPoint3DTo2D(cube->x[i],-cube->y[i],cube->z[i],flength,camz,destx,desty,&sx,&sy);
	i = 6;
	lPoint3DTo2D(cube->x[i],-cube->y[i],cube->z[i],flength,camz,destx,desty,&sx2,&sy2);
	lDrawLine(frame,sx,sy,sx2,sy2,INK);

	i = 0;
	lPoint3DTo2D(cube->x[i],-cube->y[i],cube->z[i],flength,camz,destx,desty,&sx,&sy);
	i = 3;
	lPoint3DTo2D(cube->x[i],-cube->y[i],cube->z[i],flength,camz,destx,desty,&sx2,&sy2);
	lDrawLine(frame,sx,sy,sx2,sy2,INK);
	
	i = 4;
	lPoint3DTo2D(cube->x[i],-cube->y[i],cube->z[i],flength,camz,destx,desty,&sx,&sy);
	i = 7;
	lPoint3DTo2D(cube->x[i],-cube->y[i],cube->z[i],flength,camz,destx,desty,&sx2,&sy2);
	lDrawLine(frame,sx,sy,sx2,sy2,INK);
}

int RotateCube (TCUBE *in, TCUBE *out, float anglex, float angley, float anglez)
{
	int i;
	anglex *= DEGTORAD;
	angley *= DEGTORAD;
	anglez *= DEGTORAD;
	memcpy(out, in, sizeof(TCUBE));
	
	for (i=0;i<8;i++){
		lRotateX(anglex,out->y[i], out->z[i], &out->y[i], &out->z[i]);
		lRotateY(angley,out->x[i], out->z[i], &out->x[i], &out->z[i]);
		lRotateZ(anglez,out->x[i], out->y[i], &out->x[i], &out->y[i]);
	}
	return 1;
}


int initCube (TCUBE *cube)
{
	cube->x[0] = 1;
	cube->y[0] = 1;
	cube->z[0] = 1;

	cube->x[1] = 1;
	cube->y[1] = 1;
	cube->z[1] = -1;

	cube->x[2] = -1;
	cube->y[2] = 1;
	cube->z[2] = -1;

	cube->x[3] = -1;
	cube->y[3] = 1;
	cube->z[3] = 1;

	cube->x[4] = -1;
	cube->y[4] = -1;
	cube->z[4] = 1;

	cube->x[5] = 1;
	cube->y[5] = -1;
	cube->z[5] = 1;

	cube->x[6] = 1;
	cube->y[6] = -1;
	cube->z[6] = -1;

	cube->x[7] = -1;
	cube->y[7] = -1;
	cube->z[7] = -1;
	return 1;
}
