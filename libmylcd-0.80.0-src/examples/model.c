
// All models are LightWaveObjects converted to C structs
// 15 Models included

// use notepad keys (with numlock on) left/right/up/down, page up and down, + and - to change view and axis.
// enter to reset view
// < and > to switch model
// escape exits



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
#include <math.h>
#include <sys/types.h>

#define _BenchMark_ 0


#include "mylcd.h"
#include "models/model.h"
#include "demos.h"


#ifndef DEGTORAD
#define DEGTORAD 0.034906585039
#endif

MYLCD_EXPORT void MYLCD_APICALL cpuid();
MYLCD_EXPORT uint64_t MYLCD_APICALL rdtsc();


void rotateObj (float xang, float yang, float zang, Point3 *model, Point2 *pt, int total, float fl, float zm, int dx, int dy);
void drawObj (TFRAME *frame, const Point4 *face, const Point2 *ptxy, int totalface);
void drawModel (TFRAME *frame, TMODEL *m, float xang, float yang, float zang);
int initModels (TFRAME *frame, TMODEL *m);
void addobj (TOBJECT *obj, Point3 *vert, Point4 *face, int tvert, int tface);
int keypress (TMODEL *m, ubyte chr, int *modelchange);
void deletemodels (TMODEL *m);
void resetview (TMODEL *m);


float xangle,yangle,zangle;
int SelectedModel;

static int WHITE;
static int RED;
static int GREEN;
static int BLUE;

int main ()
{
	if (!initDemoConfig("config.cfg"))
		return 0;

	printf("use console and keypad with numlock on to control\n");
	printf("model: ',' and '.' (< and >)\n");
	printf("zoom: + and -\n");
	printf("direction: 4, 8, 6 and 2\n");
	printf("tilt: 9 and 3\nenter to reset view\n");
	printf("escape to exit\n");
	
	RED = lGetRGBMask(frame, LMASK_RED);
	GREEN = lGetRGBMask(frame, LMASK_GREEN);
	BLUE = lGetRGBMask(frame, LMASK_BLUE);
	WHITE = lGetRGBMask(frame, LMASK_WHITE);

	hw->render->backGround = lGetRGBMask(frame, LMASK_BLACK);
	hw->render->foreGround = lGetRGBMask(frame, LMASK_WHITE);
	lClearFrame(frame);
	
	TMODEL models[TOTALMODELS]={0};
	initModels (frame, models);

	TMODEL *m;
	int runstatus=1;
	int modelchange;
	float fl;
	
	#if _BenchMark_
	  unsigned long long t0,t1,ttotal=0;
	  unsigned int ct=0;
	#endif 

	SelectedModel=0;
	do{
		modelchange=0;
		m = &models[SelectedModel];
		m->desty = frame->height-(frame->height>>2);
		fl = 0.0;
		resetview(m);

		#if _BenchMark_
		  t0 = rdtsc();
		  lRefresh(frame);
		  t1 = rdtsc();
		  ttotal = (t1-t0);
		  ct=0;
		#endif
				
		do{
			lClearFrame(frame);
			//c = 0;
			
			if (fl < 360) m->flength = fl++;
			//drawModel(frame,m,xangle, yangle, zangle);
			
			#if _BenchMark_
			  t0 = rdtsc();
			  drawModel(frame,m,xangle, yangle, zangle);
			  //lRefresh(frame);
			  t1 = rdtsc();
			  ttotal = (ttotal+(t1-t0))>>1;
			  if (++ct == 4000){
				runstatus=0;
				break;
			  }
			#else
			  drawModel(frame,m,xangle, yangle, zangle);
			  lRefresh(frame);
			  if (kbhit())
				 runstatus = keypress(m,getch(),&modelchange);
			   //else
				 //lSleep(1);
			#endif
		}while (runstatus && !modelchange);
	}while (runstatus);

	lRefresh(frame);
	
	
	#if _BenchMark_
	  printf ("total frames:%d\naverage ticks per frame:%d\n\n",ct,ttotal);
	#endif

	deletemodels(models);
	demoCleanup();
	return 0;
}


int keypress (TMODEL *m, ubyte chr, int *modelchange)
{
	//printf ("%i\n",chr);

	//keypad
	if (chr==52){			// left arrow 
		yangle+=1.0;
	}else if (chr==54){		// right arrow 
		yangle-=1.0;
	}else if (chr==56){		// up arrow 
		xangle-=1.0;
	}else if (chr==50){		// down arrow 
		xangle+=1.0;
			
	}else if (chr==43){		// +
		m->flength+=5.0;
	}else if (chr==45){		// -
		m->flength-=5.0;
	}else if (chr==57){		// page up
		zangle-=1.0;
	}else if (chr==51){		// page down
		zangle+=1.0;
		
	}else if (chr==27){		// escape
		return 0;

	}else if ((chr==13)||(chr==10)){ // enter
		resetview(m);

	}else if (chr==44){		// <
		if (SelectedModel>0){
			SelectedModel--; 
			*modelchange = 1;
		}
	}else if (chr==46){		// >
		if (SelectedModel<TOTALMODELS-1){
			SelectedModel++; 
			*modelchange = 1;
		}
	}
	return 1;
}

void resetview (TMODEL *m)
{
	xangle=45.0;
	yangle=180.0;
	zangle=0.0;
	m->flength = 50.0;
}

void deletemodels(TMODEL *m)
{
	int i,o;
	for (i=0;i<TOTALMODELS;i++){
		for (o=0;o<40;o++){
			if (m[i].obj[o].pt)
				free (m[i].obj[o].pt);
		}
	}
}

void drawModel (TFRAME *frame, TMODEL *m, float xang, float yang, float zang)
{
	TOBJECT *obj = m->obj;
	int o;

	for (o=0;o<m->tobj;o++){
		rotateObj(xang, yang, zang, obj[o].vert, obj[o].pt, m->obj[o].tvert, m->flength, m->zoom, m->destx, m->desty);
		drawObj(frame, obj[o].face, obj[o].pt, obj[o].tface);
	}

}

void drawObj (TFRAME *frame, const Point4 *face, const Point2 *ptxy, int totalface)
{
 	while(totalface--){
		const int ab = face[totalface].ab;
		const int ac = face[totalface].ac;
		const int bc = face[totalface].bc;
		lDrawTriangle(frame, ptxy[ab].x, ptxy[ab].y, ptxy[ac].x, ptxy[ac].y, ptxy[bc].x, ptxy[bc].y, WHITE);
		//if (++c > 4000)
		//	c = 2;
		//lDrawTriangleFilled(frame, ptxy[ab].x, ptxy[ab].y, ptxy[ac].x, ptxy[ac].y, ptxy[bc].x, ptxy[bc].y, BLUE);
	}
}

void rotateObj (float xang, float yang, float zang, Point3 *model, Point2 *pt, int total, float fl, float zm, int dx, int dy)
{

	const float ThetaX = xang * DEGTORAD;
	const float ThetaY = yang * DEGTORAD;
	const float ThetaZ = zang * DEGTORAD;

	const float cx = cos(ThetaX);
    const float cy = cos(ThetaY);
    const float cz = cos(ThetaZ);

    const float sx = sin(ThetaX);
    const float sy = sin(ThetaY);
    const float sz = sin(ThetaZ);
    	
	//X axis
	const float xx = cy*cz;
	const float xy = sx*sy*cz - cx*sz;
	const float xz = cx*sy*cz + sx*sz;

	//Y axis
	const float yx = cy*sz;
	const float yy = cx*cz + sx*sy*sz;
	const float yz = -sx*cz + cx*sy*sz;

	//Z axis
	const float zx = -sy;
	const float zy = sx*cy;
	const float zz = cx*cy;
	
	float ox,oy,oz,tmp;
	float nx,ny,nz;
	int i;

	for (i=0;i<total;i++){
		ox = model[i].x;
		oy = model[i].y;
		oz = model[i].z;

		nx = ox*xx + oy*xy + oz*xz;
		ny = ox*yx + oy*yy + oz*yz;
		nz = ox*zx + oy*zy + oz*zz;
		
		//lxyzToPoint (nx, ny, nz, fl, zm, dx, dy, &pt[i].x, &pt[i].y);
		tmp = fl/(nz-zm);
		pt[i].x = (nx*tmp)+dx;
		pt[i].y = (ny*tmp)+dy;
	}
}


int initModels (TFRAME *frame, TMODEL *m)
{
	TOBJECT *obj;

	// ent
	m->zoom = -180;
	m->flength = 260;
	m->destx = (frame->width/2);
	m->desty = (frame->height/2);
	m->tobj = 1;
	obj = m->obj;
	addobj(obj,entvert, entface,sizeof(entvert)/sizeof(Point3), sizeof(entface)/sizeof(Point4));
	
#if (!_BenchMark_)
	m++;	
	// ball
	m->zoom = -3.5;
	m->flength = 260;
	m->destx = (frame->width/2);
	m->desty = (frame->height/2);
	m->tobj = 1;
	obj = m->obj;
	addobj(obj,ballvert, ballface,sizeof(ballvert)/sizeof(Point3), sizeof(ballface)/sizeof(Point4));
	
	m++;
	// skull
	m->zoom = -700;
	m->flength = 260;
	m->destx = frame->width/2;
	m->desty = frame->height/2;
	m->tobj = 1;
	m->obj[0].vert = skullvert;
	m->obj[0].face = skullface;
	m->obj[0].tvert = sizeof(skullvert)/sizeof(Point3);
	m->obj[0].tface = sizeof(skullface)/sizeof(Point4);
	m->obj[0].pt = calloc(m->obj[0].tvert,sizeof(Point2));

	m++;
	// cube
	m->zoom = -15;
	m->flength = 260;
	m->destx = frame->width/2;
	m->desty = frame->height/2;
	m->tobj = 1;
	m->obj[0].vert = cubevert;
	m->obj[0].face = cubeface;
	m->obj[0].tvert = sizeof(cubevert)/sizeof(Point3);
	m->obj[0].tface = sizeof(cubeface)/sizeof(Point4);
	m->obj[0].pt = calloc(m->obj[0].tvert,sizeof(Point2));
	
	m++;
	// light
	m->zoom = -100;
	m->flength = 260;
	m->destx = frame->width/2;
	m->desty = frame->height/2;
	m->tobj = 1;
	m->obj[0].vert = lightvert;
	m->obj[0].face = lightface;
	m->obj[0].tvert = sizeof(lightvert)/sizeof(Point3);
	m->obj[0].tface = sizeof(lightface)/sizeof(Point4);
	m->obj[0].pt = calloc(m->obj[0].tvert,sizeof(Point2));

	m++;
	// mother1
	m->zoom = -10000;
	m->flength = 260;
	m->destx = frame->width/2;
	m->desty = frame->height/2;
	m->tobj = 1;
	m->obj[0].vert = mother1vert;
	m->obj[0].face = mother1face;
	m->obj[0].tvert = sizeof(mother1vert)/sizeof(Point3);
	m->obj[0].tface = sizeof(mother1face)/sizeof(Point4);
	m->obj[0].pt = calloc(m->obj[0].tvert,sizeof(Point2));	
	
	m++;
	// mother2
	m->zoom = -13000;
	m->flength = 260;
	m->destx = frame->width/2;
	m->desty = frame->height/2;
	m->tobj = 1;
	addobj (m->obj,mother2vert, mother2face,sizeof(mother2vert)/sizeof(Point3), sizeof(mother2face)/sizeof(Point4));

	m++;
	// tricrtps
	m->zoom = -800;
	m->flength = 260;
	m->destx = frame->width/2;
	m->desty = frame->height/2;
	m->tobj = 6;
	obj = m->obj;
	addobj(obj++,tricrtvert1, tricrtface1,sizeof(tricrtvert1)/sizeof(Point3), sizeof(tricrtface1)/sizeof(Point4));	
	addobj(obj++,tricrtvert2, tricrtface2,sizeof(tricrtvert2)/sizeof(Point3), sizeof(tricrtface2)/sizeof(Point4));
	addobj(obj++,tricrtvert3, tricrtface3,sizeof(tricrtvert3)/sizeof(Point3), sizeof(tricrtface3)/sizeof(Point4));
	addobj(obj++,tricrtvert4, tricrtface4,sizeof(tricrtvert4)/sizeof(Point3), sizeof(tricrtface4)/sizeof(Point4));
	addobj(obj++,tricrtvert5, tricrtface5,sizeof(tricrtvert5)/sizeof(Point3), sizeof(tricrtface5)/sizeof(Point4));
	addobj(obj,tricrtvert6, tricrtface6,sizeof(tricrtvert6)/sizeof(Point3), sizeof(tricrtface6)/sizeof(Point4));

	m++;
	// x29
	m->zoom = -10;
	m->flength = 40;
	m->destx = (frame->width/2);
	m->desty = (frame->height/2);
	m->tobj = 8;
	obj = m->obj;
	addobj(obj++,x29vert1, x29face1,sizeof(x29vert1)/sizeof(Point3), sizeof(x29face1)/sizeof(Point4));	
	addobj(obj++,x29vert2, x29face2,sizeof(x29vert2)/sizeof(Point3), sizeof(x29face2)/sizeof(Point4));	
	addobj(obj++,x29vert3, x29face3,sizeof(x29vert3)/sizeof(Point3), sizeof(x29face3)/sizeof(Point4));	
	addobj(obj++,x29vert4, x29face4,sizeof(x29vert4)/sizeof(Point3), sizeof(x29face4)/sizeof(Point4));	
	addobj(obj++,x29vert5, x29face5,sizeof(x29vert5)/sizeof(Point3), sizeof(x29face5)/sizeof(Point4));	
	addobj(obj++,x29vert6, x29face6,sizeof(x29vert6)/sizeof(Point3), sizeof(x29face6)/sizeof(Point4));	
	addobj(obj++,x29vert7, x29face7,sizeof(x29vert7)/sizeof(Point3), sizeof(x29face7)/sizeof(Point4));	
	addobj(obj,  x29vert8, x29face8,sizeof(x29vert8)/sizeof(Point3), sizeof(x29face8)/sizeof(Point4));	

	m++;
	// face
	m->zoom = -1000;
	m->flength = 260;
	m->destx = (frame->width/2);
	m->desty = (frame->height/2);
	m->tobj = 9;
	obj = m->obj;
	addobj(obj++,facevert1, faceface1,sizeof(facevert1)/sizeof(Point3), sizeof(faceface1)/sizeof(Point4));	
	addobj(obj++,facevert2, faceface2,sizeof(facevert2)/sizeof(Point3), sizeof(faceface2)/sizeof(Point4));	
	addobj(obj++,facevert3, faceface3,sizeof(facevert3)/sizeof(Point3), sizeof(faceface3)/sizeof(Point4));	
	addobj(obj++,facevert4, faceface4,sizeof(facevert4)/sizeof(Point3), sizeof(faceface4)/sizeof(Point4));	
	addobj(obj++,facevert5, faceface5,sizeof(facevert5)/sizeof(Point3), sizeof(faceface5)/sizeof(Point4));	
	addobj(obj++,facevert6, faceface6,sizeof(facevert6)/sizeof(Point3), sizeof(faceface6)/sizeof(Point4));	
	addobj(obj++,facevert7, faceface7,sizeof(facevert7)/sizeof(Point3), sizeof(faceface7)/sizeof(Point4));	
	addobj(obj++,facevert7, faceface8,sizeof(facevert8)/sizeof(Point3), sizeof(faceface8)/sizeof(Point4));	
	addobj(obj,  facevert8, faceface9,sizeof(facevert9)/sizeof(Point3), sizeof(faceface9)/sizeof(Point4));	

	m++;
	// plan
	m->zoom = -2000;
	m->flength = 260;
	m->destx = (frame->width/2);
	m->desty = (frame->height/2);
	m->tobj = 13;
	obj = m->obj;
	addobj(obj++,planvert1, planface1,sizeof(planvert1)/sizeof(Point3), sizeof(planface1)/sizeof(Point4));	
	addobj(obj++,planvert2, planface2,sizeof(planvert2)/sizeof(Point3), sizeof(planface2)/sizeof(Point4));	
	addobj(obj++,planvert3, planface3,sizeof(planvert3)/sizeof(Point3), sizeof(planface3)/sizeof(Point4));	
	addobj(obj++,planvert4, planface4,sizeof(planvert4)/sizeof(Point3), sizeof(planface4)/sizeof(Point4));	
	addobj(obj++,planvert5, planface5,sizeof(planvert5)/sizeof(Point3), sizeof(planface5)/sizeof(Point4));	
	addobj(obj++,planvert6, planface6,sizeof(planvert6)/sizeof(Point3), sizeof(planface6)/sizeof(Point4));	
	addobj(obj++,planvert7, planface7,sizeof(planvert7)/sizeof(Point3), sizeof(planface7)/sizeof(Point4));	
	addobj(obj++,planvert8, planface8,sizeof(planvert8)/sizeof(Point3), sizeof(planface8)/sizeof(Point4));	
	addobj(obj++,planvert9, planface9,sizeof(planvert9)/sizeof(Point3), sizeof(planface9)/sizeof(Point4));	
	addobj(obj++,planvert10, planface10,sizeof(planvert10)/sizeof(Point3), sizeof(planface10)/sizeof(Point4));	
	addobj(obj++,planvert11, planface11,sizeof(planvert11)/sizeof(Point3), sizeof(planface11)/sizeof(Point4));	
	addobj(obj++,planvert12, planface12,sizeof(planvert12)/sizeof(Point3), sizeof(planface12)/sizeof(Point4));	
	addobj(obj,  planvert13, planface13,sizeof(planvert13)/sizeof(Point3), sizeof(planface13)/sizeof(Point4));	

	m++;
	// duck
	m->zoom = -9000;
	m->flength = 260;
	m->destx = (frame->width/2);
	m->desty = (frame->height/2);
	m->tobj = 3;
	obj = m->obj;
	addobj(obj++,duckvert1, duckface1,sizeof(duckvert1)/sizeof(Point3), sizeof(duckface1)/sizeof(Point4));	
	addobj(obj++,duckvert2, duckface2,sizeof(duckvert2)/sizeof(Point3), sizeof(duckface2)/sizeof(Point4));	
	addobj(obj,duckvert3, duckface3,sizeof(duckvert3)/sizeof(Point3), sizeof(duckface3)/sizeof(Point4));	

	m++;
	// num3
	m->zoom = -125;
	m->flength = 260;
	m->destx = (frame->width/2);
	m->desty = (frame->height/2);
	m->tobj = 1;
	obj = m->obj;
	addobj(obj,num3vert, num3face,sizeof(num3vert)/sizeof(Point3), sizeof(num3face)/sizeof(Point4));

	m++;
	// chair
	m->zoom = -200;
	m->flength = 260;
	m->destx = (frame->width/2);
	m->desty = (frame->height/2);
	m->tobj = 1;
	obj = m->obj;
	addobj(obj,chairvert, chairface,sizeof(chairvert)/sizeof(Point3), sizeof(chairface)/sizeof(Point4));
	
	m++;
	// hammer
	m->zoom = -3000;
	m->flength = 260;
	m->destx = (frame->width/2);
	m->desty = (frame->height/2);
	m->tobj = 32;
	obj = m->obj;
	addobj(obj++,hamvert1, hamface1,sizeof(hamvert1)/sizeof(Point3), sizeof(hamface1)/sizeof(Point4));	
	addobj(obj++,hamvert2, hamface2,sizeof(hamvert2)/sizeof(Point3), sizeof(hamface2)/sizeof(Point4));	
	addobj(obj++,hamvert3, hamface3,sizeof(hamvert3)/sizeof(Point3), sizeof(hamface3)/sizeof(Point4));	
	addobj(obj++,hamvert4, hamface4,sizeof(hamvert4)/sizeof(Point3), sizeof(hamface4)/sizeof(Point4));	
	addobj(obj++,hamvert5, hamface5,sizeof(hamvert5)/sizeof(Point3), sizeof(hamface5)/sizeof(Point4));	
	addobj(obj++,hamvert6, hamface6,sizeof(hamvert6)/sizeof(Point3), sizeof(hamface6)/sizeof(Point4));	
	addobj(obj++,hamvert7, hamface7,sizeof(hamvert7)/sizeof(Point3), sizeof(hamface7)/sizeof(Point4));	
	addobj(obj++,hamvert8, hamface8,sizeof(hamvert8)/sizeof(Point3), sizeof(hamface8)/sizeof(Point4));	
	addobj(obj++,hamvert9, hamface9,sizeof(hamvert9)/sizeof(Point3), sizeof(hamface9)/sizeof(Point4));	
	addobj(obj++,hamvert10, hamface10,sizeof(hamvert10)/sizeof(Point3), sizeof(hamface10)/sizeof(Point4));	
	addobj(obj++,hamvert11, hamface11,sizeof(hamvert11)/sizeof(Point3), sizeof(hamface11)/sizeof(Point4));	
	addobj(obj++,hamvert12, hamface12,sizeof(hamvert12)/sizeof(Point3), sizeof(hamface12)/sizeof(Point4));	
	addobj(obj++,hamvert13, hamface13,sizeof(hamvert13)/sizeof(Point3), sizeof(hamface13)/sizeof(Point4));	
	addobj(obj++,hamvert14, hamface14,sizeof(hamvert14)/sizeof(Point3), sizeof(hamface14)/sizeof(Point4));
	addobj(obj++,hamvert15, hamface15,sizeof(hamvert15)/sizeof(Point3), sizeof(hamface15)/sizeof(Point4));
	addobj(obj++,hamvert16, hamface16,sizeof(hamvert16)/sizeof(Point3), sizeof(hamface16)/sizeof(Point4));
	addobj(obj++,hamvert17, hamface17,sizeof(hamvert17)/sizeof(Point3), sizeof(hamface17)/sizeof(Point4));
	addobj(obj++,hamvert18, hamface18,sizeof(hamvert18)/sizeof(Point3), sizeof(hamface18)/sizeof(Point4));
	addobj(obj++,hamvert19, hamface19,sizeof(hamvert19)/sizeof(Point3), sizeof(hamface19)/sizeof(Point4));
	addobj(obj++,hamvert20, hamface20,sizeof(hamvert20)/sizeof(Point3), sizeof(hamface20)/sizeof(Point4));
	addobj(obj++,hamvert21, hamface21,sizeof(hamvert21)/sizeof(Point3), sizeof(hamface21)/sizeof(Point4));
	addobj(obj++,hamvert22, hamface22,sizeof(hamvert22)/sizeof(Point3), sizeof(hamface22)/sizeof(Point4));
	addobj(obj++,hamvert23, hamface23,sizeof(hamvert23)/sizeof(Point3), sizeof(hamface23)/sizeof(Point4));
	addobj(obj++,hamvert24, hamface24,sizeof(hamvert24)/sizeof(Point3), sizeof(hamface24)/sizeof(Point4));
	addobj(obj++,hamvert25, hamface25,sizeof(hamvert25)/sizeof(Point3), sizeof(hamface25)/sizeof(Point4));
	addobj(obj++,hamvert26, hamface26,sizeof(hamvert26)/sizeof(Point3), sizeof(hamface26)/sizeof(Point4));
	addobj(obj++,hamvert27, hamface27,sizeof(hamvert27)/sizeof(Point3), sizeof(hamface27)/sizeof(Point4));
	addobj(obj++,hamvert28, hamface28,sizeof(hamvert28)/sizeof(Point3), sizeof(hamface28)/sizeof(Point4));
	addobj(obj++,hamvert29, hamface29,sizeof(hamvert29)/sizeof(Point3), sizeof(hamface29)/sizeof(Point4));
	addobj(obj++,hamvert30, hamface30,sizeof(hamvert30)/sizeof(Point3), sizeof(hamface30)/sizeof(Point4));
	addobj(obj++,hamvert31, hamface31,sizeof(hamvert31)/sizeof(Point3), sizeof(hamface31)/sizeof(Point4));
	addobj(obj,hamvert32, hamface32,sizeof(hamvert32)/sizeof(Point3), sizeof(hamface32)/sizeof(Point4));
#endif
	return 1;
}

void addobj (TOBJECT *obj, Point3 *vert, Point4 *face, int tvert, int tface)
{
	obj->vert = vert;
	obj->face = face;
	obj->tvert = tvert;
	obj->tface = tface;
	obj->pt = calloc(obj->tvert,sizeof(Point2));
}
