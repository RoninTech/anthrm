/****************************
* Trails
* based on the mystify.c demo
****************************/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>

#include "mylcd.h"
#include "demos.h"


#define TRAILLENGTH (10)
#define PLOYGONS	10

#define TOTALPOINTS (TRAILLENGTH*3)

typedef struct {
	int endpoint;
	int startpoint;
	int dx, dy;
	T2POINT point[TOTALPOINTS];
}TTRAIL;


void drawTrail (TFRAME *frame, TTRAIL *trail, int ctrl);
void updateTrail (TFRAME *frame, TTRAIL *trail, int ctrl);
static void advanceTrail (TFRAME *frm, int *x, int *y, int *dx, int *dy);
void drawBorder (TFRAME *frame);

static int RED;
static int GREEN;
static int BLUE;
static int BLACK;
static int WHITE;


int main ()
{

	if (!initDemoConfig("config.cfg"))
		return 0;
		
	RED = lGetRGBMask(frame, LMASK_RED);
	GREEN = lGetRGBMask(frame, LMASK_GREEN);
	BLUE = lGetRGBMask(frame, LMASK_BLUE);
	BLACK = lGetRGBMask(frame, LMASK_BLACK);
	WHITE = lGetRGBMask(frame, LMASK_WHITE);
	
	hw->render->backGround = BLUE;
	hw->render->foreGround = GREEN;

    time_t t;
    srand((unsigned int)time(&t));
	
	int i,ctrl = PLOYGONS;
	TTRAIL trail[ctrl];
	
	for(i=0;i<ctrl;i++){
		trail[i].startpoint = 0;
		trail[i].endpoint = 0;
		trail[i].dx = 1;
		trail[i].dy = -1;
		trail[i].point[0].x = rand()%(DWIDTH/4);
		trail[i].point[0].y = rand()%(DHEIGHT/4);
     }
                       
	int tframes=0;
	int start = GetTickCount();
	do{

		lClearFrame(frame);
		drawTrail(frame, trail, ctrl);
		drawBorder(frame);		
		lRefresh(frame);

		updateTrail(frame, trail, ctrl);
		tframes++;
		lSleep(1);

	}while(!kbhit());

    int end = GetTickCount();
    float time = (float)(end-start)*0.001;
    printf("frames:%d\ntime:%.2fs\nfps:%.1f\n\n",tframes,time,tframes/time);

	demoCleanup();
	return 1;
}

void drawBorder (TFRAME *frame)
{
	lDrawRectangle(frame, 0, 0, frame->width-1, frame->height-1, RED);
	lDrawRectangle(frame, 1, 1, frame->width-2, frame->height-2, RED);
	return;
}


void updateTrail (TFRAME *frame, TTRAIL *trail, int ctrl)
{
	int i;
	T2POINT pt[ctrl];

	for(i=0;i<ctrl;i++){
    	if (abs(trail[i].endpoint-trail[i].startpoint) > TRAILLENGTH){
			trail[i].startpoint++;
			if (trail[i].startpoint == TOTALPOINTS)
				trail[i].startpoint = 0;
		}

		pt[i].x = trail[i].point[trail[i].endpoint].x;
		pt[i].y = trail[i].point[trail[i].endpoint].y;

		advanceTrail(frame, &pt[i].x, &pt[i].y, &trail[i].dx, &trail[i].dy);

		trail[i].endpoint++;
		if (trail[i].endpoint == TOTALPOINTS)
			trail[i].endpoint = 0;

		trail[i].point[trail[i].endpoint].x = pt[i].x;
		trail[i].point[trail[i].endpoint].y = pt[i].y;
    }
	return;
}


void drawTrail (TFRAME *frame, TTRAIL *trail, int ctrl)
{

	int tmpctrl,i=0;
	int mode = GREEN;

	if (trail[0].endpoint < trail[0].startpoint){
		for(i = trail[0].startpoint; i<TOTALPOINTS; i++){
			for(tmpctrl=0; tmpctrl<ctrl-1; tmpctrl++)
				lDrawLine(frame, trail[tmpctrl].point[i].x, trail[tmpctrl].point[i].y, trail[tmpctrl+1].point[i].x, trail[tmpctrl+1].point[i].y, mode);
			lDrawLine(frame, trail[ctrl-1].point[i].x, trail[ctrl-1].point[i].y, trail[0].point[i].x, trail[0].point[i].y, mode);
        }

		for(i = 0; i<trail[0].endpoint; i++){
			for(tmpctrl=0; tmpctrl<ctrl-1; tmpctrl++)
				lDrawLine(frame, trail[tmpctrl].point[i].x, trail[tmpctrl].point[i].y, trail[tmpctrl+1].point[i].x, trail[tmpctrl+1].point[i].y, mode);
			lDrawLine(frame, trail[ctrl-1].point[i].x, trail[ctrl-1].point[i].y, trail[0].point[i].x, trail[0].point[i].y, mode);
        }

		return;
	}

	for(i = trail[0].startpoint; i<(trail[0].endpoint)+1; i++){
		for(tmpctrl=0; tmpctrl<ctrl-1; tmpctrl++)
			lDrawLine(frame, trail[tmpctrl].point[i].x, trail[tmpctrl].point[i].y, trail[tmpctrl+1].point[i].x, trail[tmpctrl+1].point[i].y, mode);
		lDrawLine(frame, trail[ctrl-1].point[i].x, trail[ctrl-1].point[i].y, trail[0].point[i].x, trail[0].point[i].y, mode);
    }
	return;
}

static void advanceTrail (TFRAME *frm, int *x, int *y, int *dx, int *dy)
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

