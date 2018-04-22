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
#include <sys/types.h>

#include "mylcd.h"
#include "demos.h"

void draw(TFRAME *frame);
void init (TFRAME *frame);
int keypress (TFRAME *frame,ubyte key);


static int col = 0;
static float colors[12][3] = {
 	{1.0, 0.5, 0.5},
	{1.0, 0.75, 0.5},
	{1.0, 1.0, 0.5},
	{0.75, 1.0, 0.5},
	{0.5, 1.0, 0.5},
	{0.5, 1.0, 0.75} ,
	{0.5, 1.0, 1.0} ,
	{0.5, 0.75, 1.0},
	{0.5, 0.5, 1.0} ,
	{0.75, 0.5, 1.0},
	{1.0, 0.5, 1.0},
	{1.0, 0.5, 0.75}
	};


typedef struct
{
	int	active;
	float	life;
	float	fade;
	float	r;
	float	g;
	float	b;
	float	x;
	float	y;
	float	z;
	float	xi;
	float	yi;
	float	zi;
	float	xg;
	float	yg;
	float	zg;
}tparticles;

#if defined(__PSP_)
  #define MAX_PARTICLES 400
  float slowdown=2.0f;
  float xspeed=-5.01;
  float yspeed=2.01;
  float zoom=-8.0; // camz
#else
  #define MAX_PARTICLES 1000
  float slowdown=2.0f;
  float xspeed=0.01;
  float yspeed=0.01;
  float zoom=-10.0; // camz
#endif

int flength=260;
int destx;
int desty;
  
tparticles particle[MAX_PARTICLES];

int main ()
{
	if (!initDemoConfig("config.cfg"))
		return 0;
		
	hw->render->backGround = lGetRGBMask(frame, LMASK_BLACK);
	hw->render->foreGround = lGetRGBMask(frame, LMASK_WHITE);
	
	lClearFrame(frame);
	init(frame);

	destx = DWIDTH/2;
	desty = DHEIGHT*0.35;
	
	do{
		lClearFrame(frame);
		draw(frame);
		lRefresh(frame);
		
		
		if (kbhit()){
			if (keypress(frame,getch()))
				break;
		}

		if (++col > 11)
			col = 0;
		
		lSleep (5);
	}while(1);

	demoCleanup();
	return 0;
};

int keypress (TFRAME *frame, ubyte key)
{
	if (key == ' '){
		init(frame);

	}else if (key == 'a'){
		if (xspeed>-200) xspeed-=1.0f;
	}else if (key == 'd'){
		if (xspeed<200) xspeed+=1.0f;
	}else if (key == 's'){
		if (yspeed>-200) yspeed-=1.0f;
	}else if (key == 'w'){
		if (yspeed<200) yspeed+=1.0f;

	}else if (key == '-'){
		if (slowdown<4.0f) slowdown+=0.1f;
	}else if (key == '+'){
		if (slowdown>0.1f) slowdown-=0.1f;

	}else if (key == 'A'){
		if (destx>1) destx--;
	}else if (key == 'D'){
		if (destx<frame->width) destx++;
	}else if (key == 'W'){
		if (desty>1) desty--;
	}else if (key == 'S'){
		if (desty<frame->height) desty++;
		
	}else if (key == 'r'){
		zoom +=0.4;
		
	}else if (key == 'f'){
		zoom -=0.4;

	}else if (key == 27)
		return 1;
		
	return 0;
}


void init (TFRAME *frame)
{
	
	int loop;
	for (loop=0;loop<MAX_PARTICLES;loop++)
	{
		particle[loop].active=1;
		particle[loop].life=2.0f;
		particle[loop].fade=(float)(rand()%100)/1000.0f+0.003f;
		particle[loop].r=colors[loop*(12/MAX_PARTICLES)][0];
		particle[loop].g=colors[loop*(12/MAX_PARTICLES)][1];
		particle[loop].b=colors[loop*(12/MAX_PARTICLES)][2];
		particle[loop].xi=(float)((rand()%50)-26.0f)*10.0f;
		particle[loop].yi=(float)((rand()%50)-25.0f)*10.0f;
		particle[loop].zi=(float)((rand()%50)-25.0f)*10.0f;
		particle[loop].xg=0.0f;
		particle[loop].yg=-0.8f;
		particle[loop].zg=0.0f;
	}
}


void draw (TFRAME *frame)
{
	int sx,sy,loop;
	float slowdown10 = slowdown*1000;
	
	for (loop=0;loop<MAX_PARTICLES;loop++){
		if (particle[loop].active){
			particle[loop].x+=particle[loop].xi/slowdown10;
			particle[loop].y+=particle[loop].yi/slowdown10;
			particle[loop].z+=particle[loop].zi/slowdown10;
			particle[loop].xi+=particle[loop].xg;
			particle[loop].yi+=particle[loop].yg;
			particle[loop].zi+=particle[loop].zg;
			particle[loop].life-=particle[loop].fade;
			
			lPoint3DTo2D(particle[loop].x,-particle[loop].y,particle[loop].z,flength,zoom,destx,desty,&sx,&sy);
			lSetPixelf(frame, sx, sy, particle[loop].r, particle[loop].g, particle[loop].b);

			if (particle[loop].life<0.5f){
				particle[loop].life=1.0f;
				particle[loop].fade=(float)(rand()&127)/5000.0f+0.003f;
				particle[loop].x=0.0f;
				particle[loop].y=0.0f;
				particle[loop].z=0.0f;
				particle[loop].xi=xspeed+(float)((rand()&63)-32.0f);
				particle[loop].yi=yspeed+(float)((rand()&63)-30.0f);
				particle[loop].zi=(float)((rand()&63)-30.0f);
				particle[loop].r=colors[col][0];
				particle[loop].g=colors[col][1];
				particle[loop].b=colors[col][2];
			}
		}
    }
}
