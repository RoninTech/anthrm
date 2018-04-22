/****************************
* Mysticfy
* Written by Hung Ki Chan
* One clear but cold day in April 2006
* Loosely based on the myLCD Ball example
****************************/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
//#include <windows.h>

#include "mylcd.h"
#include "demos.h"

void mystifyUpdateNodes (TFRAME *frm);
void mystifyDrawLineConnects (TFRAME *frm, int mode);
int mystifyInit (TFRAME *frame, int nodes);
void mystifyCleanup ();

static int *dx, *dy;
static int *px, *py;
static int ctrl;
static int RED, GREEN, BLUE, WHITE, BLACK;
	
int mystifyInit (TFRAME *frame, int nodes)
{
	//dynamic allocate, so nice! :)
	ctrl = nodes;
	dx = (int *)calloc(ctrl, sizeof(int));
	dy = (int *)calloc(ctrl, sizeof(int));
	px = (int *)calloc(ctrl, sizeof(int));
	py = (int *)calloc(ctrl, sizeof(int));
	if (!(dx&&dy&&px&&py)){
		return 0;
	}

	//randomize the start coord. for each node
	int i;
    for(i=ctrl; i--;){
		px[i]=rand()%frame->width;
		py[i]=rand()%frame->height;
		dx[i]=1;
		dy[i]=1;
    }
	mystifyUpdateNodes(frame);	
	return 1;
}


void mystifyCleanup ()
{
	free(dx); free(dy);
	free(px); free(py);
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
	lClearFrame(frame);

    time_t t;
    srand((unsigned int)time(&t));
   
	
	//must be more than 2 nodes, not nice with only 1 dot flying around
	//drawline req. more than 1 node, less is prettier
    printf("\nSelect node total (must be >2): ");
	scanf("%d",&ctrl);
	if(ctrl<3){
		demoCleanup();
		return 1;
	}
	
	mystifyInit(frame, ctrl);

	do{
        //draws each node connecting with each other then back to the start node

        lClearFrame(frame);
		//lDrawRectangle(frame, 0, 0, frame->width-1, frame->height-1, RED);
		mystifyDrawLineConnects(frame, RED);
#if 0
		int xc = (px[0] + px[1] + px[2])/3;
		int yc = (py[0] + py[1] + py[2])/3;
		lEdgeFill(frame, xc, yc, GREEN, RED);
#endif
        mystifyUpdateNodes(frame);
		lRefresh(frame);
		lSleep(20);

		//tframes++;
	}while(!kbhit());

	
    //int end = GetTickCount();
    //float time = (float)(end-start)*0.001;
    //printf("frames:%d\ntime:%.2fs\nfps:%.1f\n\n",tframes,time,tframes/time);

	mystifyCleanup();
	demoCleanup();
	return 1;
}

void mystifyDrawLineConnects (TFRAME *frm, int mode)
{
	int i;
	for(i = ctrl; i--;){
		if (i == ctrl-1)
           	lDrawLine(frm, *(px+i), *(py+i), *px, *py, mode);
		else
			lDrawLine(frm, *(px+i), *(py+i), px[i+1], py[i+1], mode);
	}
}

//update all the nodes coords. adjust each node's speed
//important, dx and dy not =0 or u get strange things happen
void mystifyUpdateNodes (TFRAME *frm)
{
	int *x = px;
	int *y = py;
	
	int i;
	for(i=ctrl; i--;){
		x[i] += dx[i];
		y[i] += dy[i];
              
		if( x[i]< 0){
			x[i] = 0;
			dx[i]=1+(rand()&1);           //different speed, very important, so u dont get same shape
		}else if(x[i]> frm->width){
			x[i] = frm->width;
			dx[i]=-1*(1+(rand()&1));       //not so sure but works, yup this should be invert
		}
              
		if(y[i] < 0){
			y[i] = 0;
			dy[i]=1+(rand()&1);
		}else if(y[i]  > frm->height){
			y[i] = frm->height;
			dy[i]=-1*(1+(rand()&1));
		}
	}
}

