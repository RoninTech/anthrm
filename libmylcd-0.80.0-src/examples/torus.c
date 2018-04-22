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
 

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <windows.h>

#include "mylcd.h"
#include "demos.h"


void torus1 (TFRAME *des, int numc, int numt, float xang, float yang, float zang, float flength, float zoom, int destx, int desty);
void torus2 (TFRAME *des, float xang, float yang, float zang, float flength, float zoom, int destx, int desty);
static INLINE void xyzToPoint (const float x,const float y,const float z,const float flength,const float camz, const int x0, const  int y0, int *screenx, int *screeny);

#define TORUS_MAJOR     1.50
#define TORUS_MINOR     0.50
#define TORUS_MAJOR_RES	28		// rings
#define TORUS_MINOR_RES 28		// points
#define TORUS_1_RING	50

#ifndef DEGTORAD
#define DEGTORAD 0.034906585039
#endif

static int RED;
static int GREEN;
static int BLUE;

int main (int argc, char *argv[])
{

	if (!initDemoConfig("config.cfg"))
		return 0;

	RED = lGetRGBMask(frame, LMASK_RED);
	GREEN = lGetRGBMask(frame, LMASK_GREEN);
	BLUE = lGetRGBMask(frame, LMASK_BLUE);

	hw->render->backGround = lGetRGBMask(frame, LMASK_BLACK);
	hw->render->foreGround = lGetRGBMask(frame, LMASK_WHITE);
	lClearFrame(frame);
	
	const int destx = frame->width/2;
	const int desty = frame->height/2;
	float a;
	float step = 0.20;
 	int tframes = 0;
	int state = 0;
	int start = GetTickCount();
 
   	for (a = 0.0; a < 360.0+step && !state; a += step){
   		tframes++;
   		
   		// clear frame
   		memset(frame->pixels, 0, frame->frameSize);
   		
   		torus1(frame, 12, TORUS_1_RING, a*2.0, a/2.0, a, 200, -9.0, destx, desty);
   		torus2(frame,                   a/2.0, a*2.0, a, 200, -9.0, destx, desty);
		
		lRefresh(frame);
		// or
		//lUpdate(hw, frame->pixels, frame->frameSize);
		
    	if (kbhit()){
   			state = 1;
   			break;
    	}
	}

	int end = GetTickCount();
	float time = (float)(end-start)*0.001;
	printf("frames:%d\ntime:%.2fs\nfps:%.1f\n\n",tframes,time,tframes/time);

	//lSaveImage(frame, L"torus.tga", IMG_TGA, frame->width,frame->height);
	demoCleanup();
    return 1;
}


static INLINE void xyzToPoint (const float x,const float y,const float z,const float flength,const float camz, const int x0, const  int y0, int *screenx, int *screeny)
{
	if (z-camz < 0.0f) return;
	float tmp = flength/(z-camz);
	*screenx = (x*tmp)+x0;
	*screeny = (y*tmp)+y0;
}

void torus2 (TFRAME *des, float xang, float yang, float zang, float flength, float zoom, int destx, int desty)
{
    int i, j, k=1;
    float s, t, x, y, z, nx, ny, nz, scale;
	float x2,y2,z2;
    int sx=0,sy=0,nsx=0,nsy=0;
	
	//pre calc some values
	float twopi = M_PI*2.0;
	float ThetaX = xang * DEGTORAD;
	float ThetaY = yang * DEGTORAD;
	float ThetaZ = zang * DEGTORAD;
	float cosx = cosf(ThetaX);
	float cosy = cosf(ThetaY);
	float cosz = cosf(ThetaZ);
	float sinx = sinf(ThetaX);
	float siny = sinf(ThetaY);
	float sinz = sinf(ThetaZ);
	float twopiTORUS_MAJOR_RES = twopi/TORUS_MAJOR_RES;
	float twopiTORUS_MINOR_RES = twopi/TORUS_MINOR_RES;
	float sintres=0.0;
	float costres=0.0;
	float cosfres=0.0;

        for( i = 0; i < TORUS_MINOR_RES; i++ ) {
            for( j = 0; j <= TORUS_MAJOR_RES; j++ ) {
        //        for( k = 1; k >= 0; k-- ) {
                    s = (i + k) % TORUS_MINOR_RES + 0.5;
                    t = j % TORUS_MAJOR_RES;

                    // Calculate point on surface
                    // could put xyz into an array but thats no fun
                    sintres = sinf(t*twopiTORUS_MAJOR_RES);
                    costres = cosf(t*twopiTORUS_MAJOR_RES);
                    cosfres = cosf(s*twopiTORUS_MINOR_RES);
                    
                    x = (TORUS_MAJOR+TORUS_MINOR*cosfres)*costres;
                    y = TORUS_MINOR * sinf(s*twopiTORUS_MINOR_RES);
                    z = (TORUS_MAJOR+TORUS_MINOR*cosfres)*sintres;

                    // Calculate surface normal
                    nx = x - TORUS_MAJOR*costres;
                    ny = y;
                    nz = z - TORUS_MAJOR*sintres;
                    scale = 1.0 / sqrtf( nx*nx + ny*ny + nz*nz);
                    nx *= scale;
                    ny *= scale;
                    nz *= scale;

  					//RotateX(ThetaX, &ny, &nz);
  	  				y2 = (ny*cosx) - (nz*sinx);
					nz = (nz*cosx) + (ny*sinx);
					ny = y2;
									
  					//RotateY(ThetaY, &nx, &nz);
  					z2 = (nz*cosy) - (nx*siny);
					nx = (nx*cosy) + (nz*siny);
					nz = z2;
					  					
   					//RotateZ(ThetaZ, &nx, &ny);
					x2 = (nx*cosz) - (ny*sinz);
					ny = (ny*cosz) + (nx*sinz);
					nx = x2;


  					//RotateX(ThetaX, &y, &z);
  					y2 = (y*cosx) - (z*sinx);
					z = (z*cosx) + (y*sinx);
					y = y2;
				
  					//RotateY(ThetaY, &x, &z);
  					z2 = (z*cosy) - (x*siny);
					x = (x*cosy) + (z*siny);
					z = z2;
				
   					//RotateZ(ThetaZ, &x, &y);
					x2 = (x*cosz) - (y*sinz);
					y = (y*cosz) + (x*sinz);
					x = x2;
				
   					xyzToPoint(nx, ny, nz, flength, zoom, destx, desty, &nsx, &nsy);
          			lSetPixel(des, nsx, nsy, RED);
          			
          			//if (nz < 0.0){
            			xyzToPoint(x, y, z, flength, zoom, destx, desty, &sx, &sy);
            			lSetPixel(des, sx, sy, GREEN);
            		//}
             //   }
            }
        }
	return;
}

void torus1 (TFRAME *des, int numc, int numt, float xang, float yang, float zang, float flength, float zoom, int destx, int desty)
{
	int i, j, k;
	float s, t, x, y, z;
	float x2,y2,z2;
	int sx=0,sy=0;

	float twopi = 2.0 * M_PI;
	float ThetaX = xang * DEGTORAD;
	float ThetaY = yang * DEGTORAD;
	float ThetaZ = zang * DEGTORAD;
	float cosx = cosf(ThetaX);
	float cosy = cosf(ThetaY);
	float cosz = cosf(ThetaZ);
	float sinx = sinf(ThetaX);
	float siny = sinf(ThetaY);
	float sinz = sinf(ThetaZ);
	float cosftwonumc=0.0;
	float twonumc=0.0,twonumt=0.0;
   
	for (i = 0; i < numc; i++) {
      for (j = 0; j <= numt; j++) {
         for (k = 1; k >= 0; k--) {
            s = (i + k) % numc + 0.5;
            t = j % numt;


            x = (1.0+0.1*cos(s*twopi/numc))*cos(t*twopi/numt);
            y = (1.0+0.1*cos(s*twopi/numc))*sin(t*twopi/numt);
            z = 0.1 * sin(s * twopi / numc);


			twonumc = s*twopi/numc;
			twonumt = t*twopi/numt;
			cosftwonumc = cosf(twonumc);
            x = (1.0+0.1*cosftwonumc)*cosf(twonumt);
            y = (1.0+0.1*cosftwonumc)*sinf(twonumt);
            z = 0.1 * sinf(twonumc);

			y2 = (y*cosx) - (z*sinx);
			z = (z*cosx) + (y*sinx);
			y = y2;
				
  			//RotateY(ThetaY, &x, &z);
  			z2 = (z*cosy) - (x*siny);
			x = (x*cosy) + (z*siny);
			z = z2;
				
   			//RotateZ(ThetaZ, &x, &y);
			x2 = (x*cosz) - (y*sinz);
			y = (y*cosz) + (x*sinz);
			x = x2;
   			
            xyzToPoint(x, y, z, flength, zoom, destx, desty, &sx, &sy);
            lSetPixel(des, sx, sy, BLUE);
         }
      }
   }
}


