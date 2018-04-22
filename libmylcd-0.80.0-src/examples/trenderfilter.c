
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
#include <string.h>
#include <windows.h>

#include "mylcd.h"
//#include "../src/pixel.h"
#include "demos.h"

#define MIN(a, b) a<b?a:b
#define MAX(a, b) a>b?a:b
#define D (float)(M_PI / 180.0f);

#define	_text1 "The quick brown fox jumps over the lazy dog."
#define	_text2 "沈默之中仍克制不要呼吸\n怕觸摸空氣壞了情緒\n為你鎖住了問號　等親愛的你吻去\n欺騙自己"
char text1[] = _text1;
char text2[] = _text2;
char text3[] = _text1 "\n" _text2;
char *buffer = text3;

const int flags = PF_CLIPWRAP|PF_WORDWRAP|PF_LEFTJUSTIFY;
const int fontID = LFTW_B14;
//const int fontID = LFTW_UNICODE;

float a = 45.0f * D;




static void render (TFRAME *frame, int font, int flags, wchar_t *path)
{
	TLPRINTR rect = {0, 0, frame->width-1, frame->height-1, 40, 50, 0, 0};
	lClearFrame(frame);
	lLoadImageEx(frame, L"images/cspread.png", 0, 0);
	lPrintEx(frame, &rect, font, flags|PF_DONTFORMATBUFFER, LPRT_CPY, buffer);
	lRefresh(frame);
	if (path)
		lSaveImage(frame, path, IMG_BMP, frame->width, frame->height);
}

/*
typedef struct{
	TWCHAR *glyph;		// glyph source
	struct TFRAME *to;	// destination frame
	
	int dx;				// destination point
	int dy;				//
	
	int x1;				// source rect within glyph
	int y1;				//
	int x2;				//
	int y2;				//
	
	int style;			// copy type
	unsigned int flags;	// print flags
}TLPRINTREGION;
*/

#if 1
void renderCB (TLPRINTREGION *loc)
{
	const TFRAME *src = loc->glyph->f;
	TFRAME *des = loc->to;
	
	int x,y;
	int xx;

	loc->y2 = MIN(loc->y2+1, src->height);
	loc->x2 = MIN(loc->x2+1, src->width);
	float xr, yr;

	const int ink = 0xFFFFFFFF;
	//const int inkSm = 0xFF000000;
	
	loc->dx -= src->width/2.0;
	loc->dy -= src->height/*/2.0*/;
	
	int yy = loc->dy;
	
	for (y = loc->y1; y<loc->y2;y++,yy++){
		xx = loc->dx;
		for (x = loc->x1; x<loc->x2; x++,xx++){
			if ((xx < des->width) && (yy < des->height)){
				if (lGetPixel_NB(src, x, y)){
					lRotateX(a, x, y, &xr, &yr);
					xr += (float)src->width/2.0;
					yr += (float)src->height/*/2.0*/;

					lSetPixel(des, xx+xr, yy+yr, ink);
							
					/*lSetPixel(des, xx+xr+1, yy+yr, inkSm);
					lSetPixel(des, xx+xr, yy+yr+1, inkSm);
					lSetPixel(des, xx+xr+1, yy+yr+1, inkSm);
					lSetPixel(des, xx+xr+1, yy+yr-1, inkSm);
					
					lSetPixel(des, xx+xr-1, yy+yr+1, inkSm);
					lSetPixel(des, xx+xr-1, yy+yr, inkSm);
					lSetPixel(des, xx+xr, yy+yr-1, inkSm);
					lSetPixel(des, xx+xr-1, yy+yr-1, inkSm);*/

				}
			}
		}
	}
	return;
}
#endif


#define LTRA_SHADOW_N		(1<<31)			/* direction */
#define LTRA_SHADOW_S		(1<<30)
#define LTRA_SHADOW_W		(1<<29)
#define LTRA_SHADOW_E		(1<<28)
#define LTRA_SHADOW_S1		(1<<24)			/* size */
#define LTRA_SHADOW_S2		(1<<23)
#define LTRA_SHADOW_S3		(1<<22)
#define LTRA_SHADOW_S4		(1<<21)
#define LTRA_SHADOW_S5		(1<<20)
#define LTRA_SHADOW_OS(n)	(((n)&0xFF)<<8)	/* offset */
#define LTRA_SHADOW_TR(n)	((n)&0xFF)		/* transparency */



void renderOL (TLPRINTREGION *loc)
{
	const TFRAME *src = loc->glyph->f;
	TFRAME *des = loc->to;
	
	const int value = LTRA_SHADOW_S|LTRA_SHADOW_E | LTRA_SHADOW_S5 | LTRA_SHADOW_OS(2) | LTRA_SHADOW_TR(80);
 	const int ssize = value & (LTRA_SHADOW_S1|LTRA_SHADOW_S2|LTRA_SHADOW_S3|LTRA_SHADOW_S4|LTRA_SHADOW_S5);
	const int offset = (value>>8)&0xFF;
 	const int inkSh = ((value&0xFF)<<24) | (des->hw->render->backGround&0xFFFFFF);
	const int ink = des->hw->render->foreGround;
	int x,y,dx,dy;
	
	if (value&LTRA_SHADOW_S)
		dy = loc->dy + offset;
	else if (value&LTRA_SHADOW_N)
		dy = loc->dy - offset;
	else
		dy = loc->dy;
	
	for (y = loc->y1; y <= loc->y2; y++, dy++){
		if (value&LTRA_SHADOW_E)
			dx = loc->dx + offset;
		else if (value&LTRA_SHADOW_W)
			dx = loc->dx - offset;
		else
			dx = loc->dx;

		for (x = loc->x1; x <= loc->x2; x++, dx++){
			if (lGetPixel_NB(src, x, y)){
				switch (ssize){
			  	  case LTRA_SHADOW_S5: 
					lSetPixel(des, dx, dy-1, inkSh);
			  	  case LTRA_SHADOW_S4: 
					lSetPixel(des, dx-1, dy, inkSh);
			  	  case LTRA_SHADOW_S3: 
					lSetPixel(des, dx, dy+1, inkSh);
			  	  case LTRA_SHADOW_S2: 
					lSetPixel(des, dx+1, dy, inkSh);
			  	  case LTRA_SHADOW_S1: 
					lSetPixel(des, dx, dy, inkSh);
				}
			}
		}
	}

	if ((value&LTRA_SHADOW_W && value&LTRA_SHADOW_E) || (value&LTRA_SHADOW_N && value&LTRA_SHADOW_S)){
		if (value&LTRA_SHADOW_N)
			dy = loc->dy - offset;
		else if (value&LTRA_SHADOW_S)
			dy = loc->dy + offset;
		else
			dy = loc->dy;
		
		for (y = loc->y1; y <= loc->y2; y++, dy++){
			if (value&LTRA_SHADOW_W)
				dx = loc->dx - offset;
			else if (value&LTRA_SHADOW_E)
				dx = loc->dx + offset;
			else
				dx = loc->dx;

			for (x = loc->x1; x <= loc->x2; x++, dx++){
				if (lGetPixel_NB(src, x, y)){
					switch (ssize){
				  	  case LTRA_SHADOW_S5: 
						lSetPixel(des, dx, dy-1, ink);
				  	  case LTRA_SHADOW_S4: 
						lSetPixel(des, dx-1, dy, ink);
				  	  case LTRA_SHADOW_S3: 
						lSetPixel(des, dx, dy+1, ink);
				  	  case LTRA_SHADOW_S2: 
						lSetPixel(des, dx+1, dy, ink);
				  	  case LTRA_SHADOW_S1: 
						lSetPixel(des, dx, dy, ink);
					}
				}
			}
		}
	}

	dy = loc->dy;
	for (y = loc->y1; y <= loc->y2; y++, dy++){
		dx = loc->dx;
		for (x = loc->x1; x <= loc->x2; x++, dx++){
			if (lGetPixel_NB(src, x, y))
				lSetPixel(des, dx, dy, ink);
		}
	}

}

int main (int argc, char* argv[])
{

	if (!initDemoConfig("config.cfg"))
		return 0;


	lSetFontCharacterSpacing(hw, fontID, 4);
	lSetCharacterEncoding(hw, CMT_BIG5);
	
	lSetBackgroundColour(hw, 0xFF001F00);
	lSetForegroundColour(hw, 0xFFFFFFFF);
	lClearFrame(frame);
	
	
#if 1
	//use internal render functions
	lSetRenderEffect(hw, LTR_OUTLINE1);
	render(frame, fontID, flags, L"frame_OL1.bmp");
	lSleep(1000);

	lSetRenderEffect(hw, LTR_OUTLINE2);
	render(frame, fontID, flags, L"frame_OL2.bmp");
	lSetRenderEffect(hw, LTR_DEFAULT);
	lSleep(500);

	lSetRenderEffect(hw, LTR_0);
	render(frame, fontID, flags, L"frame_0.bmp");
	lSleep(500);

	lSetRenderEffect(hw, LTR_90);
	render(frame, fontID, flags, L"frame_90.bmp");
	lSleep(500);

	lSetRenderEffect(hw, LTR_180);
	render(frame, fontID, flags, L"frame_180.bmp");
	lSleep(500);
	
	lSetRenderEffect(hw, LTR_270);
	render(frame, fontID, flags, L"frame_270.bmp");
	lSleep(500);

	lSetRenderEffect(hw, LTR_90VFLIP);
	render(frame, fontID, flags, L"frame_90vf.bmp");
	lSleep(500);
	
	lSetRenderEffect(hw, LTR_270VFLIP);
	render(frame, fontID, flags, L"frame_270vf.bmp");
	lSleep(500);

	lSetRenderEffect(hw, LTR_HFLIP);
	render(frame, fontID, flags, L"frame_hf.bmp");
	lSleep(500);

	lSetRenderEffect(hw, LTR_VFLIP);
	render(frame, fontID, flags, L"frame_vf.bmp");
	lSleep(500);
#endif


	// set and use an external render function
	hw->render->copy = (void*)renderCB;
	
	float i;
	for (i = 0; i < 360.0; i += 1.15){
		a = i*D;
		render(frame, fontID, flags, NULL);
		lSleep(1);
	}


	lSetRenderEffect(hw, LTR_DEFAULT);
	//render(frame, fontID, flags, L"frame.bmp");
		
	hw->render->copy = (void*)renderOL;
	render(frame, fontID, flags, L"frame_5.bmp");

	demoCleanup();
	return 1;
}
	

