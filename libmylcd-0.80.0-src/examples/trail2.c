/****************************
* Trails 2
* based on mystify.c/trails.c
****************************/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
//#include <windows.h>

#include "mylcd.h"
#include "demos.h"


#define TRAILLENGTH (4)
#define TOTALPOINTS (TRAILLENGTH*3)

typedef struct {
	int endpoint;
	int startpoint;
	int dx1, dy1;
	int dx2, dy2;
	TLPOINTEX lines[TOTALPOINTS];
}TTRAIL;


void drawTrail (TFRAME *frame, TTRAIL *trail1, TTRAIL *trail2);
void updateTrail (TFRAME *frame, TTRAIL *trail1, TTRAIL *trail2);
void advanceTrail (TFRAME *frame, int *x, int *y, int *dx, int *dy);
void drawBorder (TFRAME *frame);


int main ()
{

	if (!initDemoConfig("config.cfg")) return 0;

    time_t t;
    srand((unsigned int)time(&t));
	
	TTRAIL trail1;
	trail1.startpoint = 0;
	trail1.endpoint = 0;
	trail1.dx1 = 1;
	trail1.dy1 = -1;
	trail1.dx2 = 1;
	trail1.dy2 = -1;
	trail1.lines[0].x1 = rand()%(DWIDTH/4);
	trail1.lines[0].y1 = rand()%(DHEIGHT/4);
	trail1.lines[0].x2 = rand()%(DWIDTH/4);
	trail1.lines[0].y2 = rand()%(DHEIGHT/4);

	TTRAIL trail2 = trail1;

	//int tframes=0;
	//int start = GetTickCount();
	
	do{

		lClearFrame(frame);
		drawTrail(frame, &trail1, &trail2);
		//drawBorder(frame);		
		lRefresh(frame);
		lSleep(20);

		updateTrail(frame, &trail1, &trail2);
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


void updateTrail (TFRAME *frame, TTRAIL *trail1, TTRAIL *trail2)
{

	TLPOINTEX pt1, pt2;

	if (abs(trail1->endpoint-trail1->startpoint) > TRAILLENGTH){
		trail1->startpoint++;
		if (trail1->startpoint == TOTALPOINTS)
			trail1->startpoint = 0;
	}

	if (abs(trail2->endpoint-trail2->startpoint) > TRAILLENGTH){
		trail2->startpoint++;
		if (trail2->startpoint == TOTALPOINTS)
			trail2->startpoint = 0;
	}
	
	pt1.x1 = trail1->lines[trail1->endpoint].x1;
	pt1.y1 = trail1->lines[trail1->endpoint].y1;
	pt1.x2 = trail1->lines[trail1->endpoint].x2;
	pt1.y2 = trail1->lines[trail1->endpoint].y2;
	
	pt2.x1 = trail2->lines[trail2->endpoint].x1;
	pt2.y1 = trail2->lines[trail2->endpoint].y1;
	pt2.x2 = trail2->lines[trail2->endpoint].x2;
	pt2.y2 = trail2->lines[trail2->endpoint].y2;
	
	advanceTrail(frame, &pt1.x1, &pt1.y1, &trail1->dx1, &trail1->dy1);
	advanceTrail(frame, &pt1.x2, &pt1.y2, &trail1->dx2, &trail1->dy2);
	
	advanceTrail(frame, &pt2.x1, &pt2.y1, &trail2->dx1, &trail2->dy1);
	advanceTrail(frame, &pt2.x2, &pt2.y2, &trail2->dx2, &trail2->dy2);

	trail1->endpoint++;
	if (trail1->endpoint == TOTALPOINTS)
		trail1->endpoint = 0;

	trail2->endpoint++;
	if (trail2->endpoint == TOTALPOINTS)
		trail2->endpoint = 0;

	trail1->lines[trail1->endpoint].x1 = pt1.x1;
	trail1->lines[trail1->endpoint].y1 = pt1.y1;
	trail1->lines[trail1->endpoint].x2 = pt1.x2;
	trail1->lines[trail1->endpoint].y2 = pt1.y2;

	trail2->lines[trail2->endpoint].x1 = pt2.x1;
	trail2->lines[trail2->endpoint].y1 = pt2.y1;
	trail2->lines[trail2->endpoint].x2 = pt2.x2;
	trail2->lines[trail2->endpoint].y2 = pt2.y2;

	return;
}


void drawTrail (TFRAME *frame, TTRAIL *trail1, TTRAIL *trail2)
{

	const int colour = hw->render->foreGround;
	int i;
	
	if (trail1->endpoint < trail1->startpoint){
		for(i = trail1->startpoint; i<TOTALPOINTS; i++){
			lDrawLine(frame, trail1->lines[i].x1, trail1->lines[i].y1, trail1->lines[i].x2, trail1->lines[i].y2, colour);
			lDrawLine(frame, trail1->lines[i].x1, trail1->lines[i].y1, trail2->lines[i].x1, trail2->lines[i].y1, colour);
			
			lDrawLine(frame, trail2->lines[i].x1, trail2->lines[i].y1, trail2->lines[i].x2, trail2->lines[i].y2, colour);
			lDrawLine(frame, trail2->lines[i].x2, trail2->lines[i].y2, trail1->lines[i].x2, trail1->lines[i].y2, colour);
        }

		for(i = 0; i<trail1->endpoint; i++){
			lDrawLine(frame, trail1->lines[i].x1, trail1->lines[i].y1, trail1->lines[i].x2, trail1->lines[i].y2, colour);
			lDrawLine(frame, trail1->lines[i].x1, trail1->lines[i].y1, trail2->lines[i].x1, trail2->lines[i].y1, colour);
			lDrawLine(frame, trail2->lines[i].x1, trail2->lines[i].y1, trail2->lines[i].x2, trail2->lines[i].y2, colour);
			lDrawLine(frame, trail2->lines[i].x2, trail2->lines[i].y2, trail1->lines[i].x2, trail1->lines[i].y2, colour);
        }

		return;
	}

	for(i = trail1->startpoint; i<trail1->endpoint+1; i++){
		lDrawLine(frame, trail1->lines[i].x1, trail1->lines[i].y1, trail1->lines[i].x2, trail1->lines[i].y2, colour);
		lDrawLine(frame, trail1->lines[i].x1, trail1->lines[i].y1, trail2->lines[i].x1, trail2->lines[i].y1, colour);
		lDrawLine(frame, trail2->lines[i].x1, trail2->lines[i].y1, trail2->lines[i].x2, trail2->lines[i].y2, colour);
		lDrawLine(frame, trail2->lines[i].x2, trail2->lines[i].y2, trail1->lines[i].x2, trail1->lines[i].y2, colour);
    }

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

