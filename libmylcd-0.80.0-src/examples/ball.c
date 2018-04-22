            
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

#define BALLWIDTH	(10)

static void DrawBall (TFRAME *frame, int x, int y);
static void DrawFrame (TFRAME *frame);


static int RED;
static int GREEN;
static int BLUE;
static int WHITE;
static int BLACK;


//#define RADIUS           5

//#define BOUNCE_WIDTH    (RADIUS * 2.1)
#define BOUNCE_WIDTH    (480.0)

//#define BOUNCE_HEIGHT   (RADIUS * 2.1)
#define BOUNCE_HEIGHT    (272.0)



/* Animation speed (50.0 mimics the original GLUT demo speed) */
#define ANIMATION_SPEED 100.0

/* Maximum allowed delta time per physics iteration */
#define MAX_DELTA_T 0.02

#define WALL_L_OFFSET   -BALLWIDTH
#define WALL_R_OFFSET   -BALLWIDTH

static float ball_x          = 50;
static float ball_y          = -50;
static float ball_x_inc      = 1.0;
static float ball_y_inc      = 2.0;

static double  t;
static double  t_old = 0.0;
static double  dt;

/* Random number generator */
#ifndef RAND_MAX
 #define RAND_MAX 4095
#endif

/*****************************************************************************
 * Truncate a degree.
 *****************************************************************************/
float TruncateDeg( float deg )
{
   if ( deg >= 360.0 )
      return (deg - 360.0);
   else
      return deg;
}

/*****************************************************************************
 * Convert a degree (360-based) into a radian.
 * 360' = 2 * PI
 *****************************************************************************/
static double deg2rad( double deg )
{
   return deg / 360 * (2 * M_PI);
}

/*****************************************************************************
 * 360' sin().
 *****************************************************************************/
static double sin_deg( double deg )
{
   return sin( deg2rad( deg ) );
}


/*****************************************************************************
 * Bounce the ball.
 *****************************************************************************/
void BounceBall (double dt)
{
   static float sign = 1.0;
   float deg;

   /* Bounce on walls */
   if ( ball_x >  (BOUNCE_WIDTH/2 + WALL_R_OFFSET ) )
   {
      ball_x_inc = -0.5 - 0.75 * (float)rand() / (float)RAND_MAX;
      //deg_rot_y_inc = -deg_rot_y_inc;
   }
   if ( ball_x < -(BOUNCE_WIDTH/2 + WALL_L_OFFSET) )
   {
      ball_x_inc =  0.5 + 0.75 * (float)rand() / (float)RAND_MAX;
      //deg_rot_y_inc = -deg_rot_y_inc;
   }

   /* Bounce on floor / roof */
   if ( ball_y > (BOUNCE_HEIGHT/2.0)-BALLWIDTH){
      ball_y_inc = -0.75 - 1.0 * (float)rand() / (float)RAND_MAX;
   }

   if ( ball_y < -(BOUNCE_HEIGHT/2.0)+BALLWIDTH){
      ball_y_inc =  0.75 + 1.0 * (float)rand() / (float)RAND_MAX;
      
   }

   /* Update ball position */
   ball_x += ball_x_inc * (dt*ANIMATION_SPEED);
   ball_y += ball_y_inc * (dt*ANIMATION_SPEED);

  /*
   * Simulate the effects of gravity on Y movement.
   */
   deg = (ball_y + BOUNCE_HEIGHT/2.0) * 90.0 / BOUNCE_HEIGHT;
   if ( deg > 80.0 ) deg = 80.0;
   if ( deg < 10.0 ) deg = 10.0;
   
	if (ball_y_inc < 0 ){
		sign = -1.0;
	}else{
		sign = 1.0;
	}

   ball_y_inc = (sign * 4.0 * sin_deg( deg ));

}


int main ()
{
	if (!initDemoConfig("config.cfg"))
		return 0;

	RED = lGetRGBMask(frame, LMASK_RED);
	GREEN = lGetRGBMask(frame, LMASK_GREEN);
	BLUE = lGetRGBMask(frame, LMASK_BLUE);
	BLACK = lGetRGBMask(frame, LMASK_BLACK);
	WHITE = lGetRGBMask(frame, LMASK_WHITE);
	
	hw->render->backGround = BLACK;
	hw->render->foreGround = WHITE;
	srand(GetTickCount());

	int x = 1000;
	double dt_total, dt2;
   
	timeBeginPeriod(1);
	int t0 = GetTickCount();
	do{
		lClearFrame(frame);
		DrawBall(frame, ball_x+(frame->width>>1), ball_y+(frame->height>>1));
		DrawFrame(frame);
		lRefresh(frame);

		t = timeGetTime();
		dt = t - t_old;
		t_old = t;
		
		dt_total = dt;
		if (dt_total > 0.0 ){
			dt2 = dt_total > MAX_DELTA_T ? MAX_DELTA_T : dt_total;
			dt_total -= dt2;
			BounceBall(dt2);
		}
		
		
		lSleep(10);
	}while(x-- && !kbhit());
	timeEndPeriod(1);
		
	int t1 = GetTickCount();
	printf("%.2f\n", (t1-t0)/1000.0);
	demoCleanup();
	return 0;
}

static void DrawFrame (TFRAME *frame)
{
	lDrawRectangle(frame, 0, 0, frame->width-1, frame->height-1, RED);
	lDrawRectangle(frame, 1, 1, frame->width-2, frame->height-2, RED);
}

static void DrawBall (TFRAME *frame, int x, int y)
{
	lDrawCircleFilled(frame, x, y, BALLWIDTH, GREEN);
}

