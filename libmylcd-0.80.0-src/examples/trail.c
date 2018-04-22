/****************************
* Trails
* based on the mystify.c demo
****************************/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
//#include <windows.h>

#include "mylcd.h"
#include "demos.h"


#define TRAILLENGTH (16)
#define TOTALPOINTS (TRAILLENGTH*3)

typedef struct {
	int endpoint;
	int startpoint;
	int dx1, dy1;
	int dx2, dy2;
	TLPOINTEX lines[TOTALPOINTS];
}TTRAIL;


void drawTrail (TFRAME *frame, TTRAIL *trail);
void updateTrail (TFRAME *frame, TTRAIL *trail);
void advanceTrail (TFRAME *frame, int *x, int *y, int *dx, int *dy);
void drawBorder (TFRAME *frame);


int main ()
{

	if (!initDemoConfig("config.cfg")) return 0;

    time_t t;
    srand((unsigned int)time(&t));
	
	TTRAIL trail;
	trail.startpoint = 0;
	trail.endpoint = 0;
	trail.dx1 = 1;
	trail.dy1 = -1;
	trail.dx2 = 1;
	trail.dy2 = -1;
	trail.lines[0].x1 = rand()%(DWIDTH/4);
	trail.lines[0].y1 = rand()%(DHEIGHT/4);
	trail.lines[0].x2 = rand()%(DWIDTH/4);
	trail.lines[0].y2 = rand()%(DHEIGHT/4);

	//int tframes=0;
	//int start = GetTickCount();
	
	do{

		lClearFrame(frame);
		drawTrail(frame, &trail);
		//drawBorder(frame);		
		lRefresh(frame);
		lSleep(25);

		updateTrail(frame, &trail);
		//tframes++;

	}while(!kbhit());

    //int end = GetTickCount();
    //float time = (float)(end-start)*0.001;
    //printf("frames:%d\ntime:%.2fs\nfps:%.1f\n\n",tframes,time,tframes/time);

	demoCleanup();
	return 1;
}

void drawBorder (TFRAME *frame)
{
	lDrawRectangle(frame, 0, 0, frame->width, frame->height, hw->render->foreGround);
	return;
}


void updateTrail (TFRAME *frame, TTRAIL *trail)
{

	TLPOINTEX pt;

	if (abs(trail->endpoint-trail->startpoint) > TRAILLENGTH){
		trail->startpoint++;
		if (trail->startpoint == TOTALPOINTS)
			trail->startpoint = 0;
	}
	
	pt.x1 = trail->lines[trail->endpoint].x1;
	pt.y1 = trail->lines[trail->endpoint].y1;
	pt.x2 = trail->lines[trail->endpoint].x2;
	pt.y2 = trail->lines[trail->endpoint].y2;
	
	advanceTrail(frame, &pt.x1, &pt.y1, &trail->dx1, &trail->dy1);
	advanceTrail(frame, &pt.x2, &pt.y2, &trail->dx2, &trail->dy2);

	trail->endpoint++;
	if (trail->endpoint == TOTALPOINTS)
		trail->endpoint = 0;

	trail->lines[trail->endpoint].x1 = pt.x1;
	trail->lines[trail->endpoint].y1 = pt.y1;
	trail->lines[trail->endpoint].x2 = pt.x2;
	trail->lines[trail->endpoint].y2 = pt.y2;

	return;
}


void drawTrail (TFRAME *frame, TTRAIL *trail)
{
	if (trail->endpoint < trail->startpoint){
		lDrawPolyLineEx(frame, &trail->lines[trail->startpoint], (TOTALPOINTS-trail->startpoint), hw->render->foreGround);
		lDrawPolyLineEx(frame, trail->lines, trail->endpoint, hw->render->foreGround);
	}else
		lDrawPolyLineEx(frame, &trail->lines[trail->startpoint], (trail->endpoint-trail->startpoint)+1, hw->render->foreGround);

	return;
}

void advanceTrail (TFRAME *frm, int *x, int *y, int *dx, int *dy)
{

	*x += *dx;
	*y += *dy;
   
	if( *x < 0){
		*x = 0;
		*dx = 1+(rand()&1);
		
	}else if(*x > frm->width-1){
		*x = frm->width-1;
		*dx = -(1+(rand()&1));
	}
              
	if(*y < 0){
		*y = 0;
		*dy = (1+(rand()&1));
		
	}else if(*y  > frm->height-1){
		*y = frm->height-1;
		*dy = -(1+(rand()&1));
	}

}

