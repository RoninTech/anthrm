
// libmylcd - http://mylcd.sourceforge.net/
// An LCD framebuffer library
// Michael McElligott
// okio@users.sourceforge.net

//  Copyright (c) 2005-2010  Michael McElligott
// 
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU LIBRARY GENERAL PUBLIC LICENSE
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU LIBRARY GENERAL PUBLIC LICENSE for more details.
//
//	You should have received a copy of the GNU Library General Public
//	License along with this library; if not, write to the Free
//	Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.



#include "mylcd.h"

#if (__BUILD_DRAW_SUPPORT__)


#include "memory.h"
#include "utils.h"
#include "pixel.h"
#include "lmath.h"
#include "copy.h"
#include "draw.h"
#include "mmx.h"

static void circlePts (TFRAME *frm, int xc, int yc, int x, int y, int colour);
static int floodFill_op (TFILL *fill, int x, int y, const int newColor, const int oldColor);
static int floodfill_init (TFILL *fill, TFRAME *frame);
static void floodfill_cleanup (TFILL *fill);
static int edgefill_op (TFRAME *frame, TFILL *stack, int ox, int oy, int newColour, int edgeColour);
static int edgefill_init (TFILL *stack, TFRAME *frame);
static void edgefill_cleanup (TFILL *stack);
static void bindCoordinates (TFRAME *frame, int *x1, int *y1, int *x2, int *y2);
static void horLine (TFRAME *frame, int y, int x1, int x2, int colour);
static int clipLine (TFRAME *frame, int x1, int y1, int x2, int y2, int *x3, int *y3, int *x4, int *y4);



static void swapint (int *a, int *b) 
{ 
	int tmp = *a;
	*a = *b;
	*b = tmp;
}

int floodFill (TFRAME *frame, const int x, const int y, const int colour)
{
	int ret = 0;
	TFILL *fill = l_malloc(sizeof(TFILL));
	if (fill != NULL){
		if (floodfill_init(fill, frame)){
			ret = floodFill_op(fill, x, y, colour, l_getPixel(frame, x, y));
			floodfill_cleanup(fill);
		}
		l_free(fill);
	}
	return ret;
}

int edgeFill (TFRAME *frame, const int x, const int y, const int fillColour, const int edgeColour)
{
	int ret = 0;
	TFILL *stack = l_malloc(sizeof(TFILL));
	if (stack != NULL){
		if (edgefill_init(stack, frame)){
			ret = edgefill_op(frame, stack, x, y, fillColour, edgeColour);
			edgefill_cleanup(stack);
		}
		l_free(stack);
	}

	return ret;
}

int drawPolyLineEx (TFRAME *frm, TLPOINTEX *pt, int n, const int colour)
{
	while(n--)
		drawLine(frm, pt[n].x1, pt[n].y1, pt[n].x2, pt[n].y2, colour);
	return n;
}

int drawPolyLineDottedTo (TFRAME *frm, T2POINT *pt, const int tPoints, const int colour)
{
	int ct;
	for (ct = 1; ct < tPoints; ct++)
		drawLineDotted(frm, pt[ct-1].x, pt[ct-1].y, pt[ct].x, pt[ct].y, colour);
	return ct;
}

int drawPolyLine (TFRAME *frm, T2POINT *pt, int tPoints, const int colour)
{
	if (!frm || !pt) return 0;
	
	int ct;
	for (ct=0;ct < tPoints-1; ct += 2)
		drawLine(frm, pt[ct].x, pt[ct].y, pt[ct+1].x, pt[ct+1].y, colour);
	return 1;
}

int drawEllipse (TFRAME *frm, int x, int y, int r1, int r2, const int colour)
{
	drawArc(frm, x, y, r1, r2, 0.0, 90.0, colour);
	return drawArc(frm, x, y, r1, r2, 90.0, 360.0, colour);
}

int drawPolyLineTo (TFRAME *frm, T2POINT *pt, int tPoints, const int colour)
{
	int ct;
	for (ct = 1; ct < tPoints; ct++)
		drawLine(frm, pt[ct-1].x, pt[ct-1].y, pt[ct].x, pt[ct].y, colour);
	return ct;
}

int drawCircleFilled (TFRAME *frame, int xc, int yc, int radius, const int colour)
{	
	if (xc + radius < 0 || xc - radius >= frame->width || yc + radius < 0 || yc - radius >= frame->height)
		return 0;
  	
	int y = radius;
	int p = 3 - (radius << 1);
	int a, b, c, d, e, f, g, h, x = 0;
	int pb = yc + radius + 1, pd = yc + radius + 1;
  
	while (x <= y){
		a = xc + x; b = yc + y;
		c = xc - x; d = yc - y;
		e = xc + y; f = yc + x;
		g = xc - y; h = yc - x;
		if (b != pb)
			horLine(frame, b, a, c, colour);
		if (d != pd)
			horLine(frame, d, a, c, colour);
		if (f != b)
			horLine(frame, f, e, g, colour);
		if (h != d && h != f)
			horLine(frame, h, e, g, colour);
		pb = b; pd = d;
		if (p < 0)
			p += (x++ << 2) + 6;
		else
			p += ((x++ - y--) << 2) + 10;
	}
	return 1;
}

int drawTriangle (TFRAME *frame, int x1, int y1, int x2, int y2, int x3, int y3, int colour)
{
	if (!frame) return 0;
	
	drawLine(frame, x1, y1, x2, y2, colour);
	drawLine(frame, x2, y2, x3, y3, colour);
	return drawLine(frame, x1, y1, x3, y3, colour);
}

static void swapd (double *a, double *b) 
{ 
	double tmp = *a;
	*a = *b;
	*b = tmp;
}

int drawTriangleFilled (TFRAME *frame, const int x0, const int y0, const int x1, const int y1, const int x2, const int y2, const int colour)
{
	double XA, XB;
  	double XA1, XB1, XC1;
  	double XA2, XB2;
  	double XAd, XBd; 
  	double HALF;
  	
	int t = y0;
	int b = y0;
	int CAS = 0;
	
	if (y1 < t){
		t = y1;
		CAS = 1;
	}
	if (y1 > b)
		b = y1;
		
	if (y2 < t){
		t = y2;
		CAS = 2;
	}
	if (y2 > b)
		b = y2;
   	
	if (CAS == 0){
		XA = x0;
		XB = x0;
		XA1 = (x1-x0)/(double)(y1-y0);
		XB1 = (x2-x0)/(double)(y2-y0);
		XC1 = (x2-x1)/(double)(y2-y1);
		
		if (y1<y2){
			HALF = y1;
      		XA2 = XC1;
      		XB2 = XB1;
    	}else{
    		HALF = y2;
      		XA2 = XA1;
      		XB2 = XC1;
    	}
		if (y0 == y1)
			XA = x1;
		if (y0 == y2)
			XB = x2;
  	}else if (CAS == 1){
    	XA = x1;
    	XB = x1;
    	XA1 = (x2-x1)/(double)(y2-y1);
    	XB1 = (x0-x1)/(double)(y0-y1);
    	XC1 = (x0-x2)/(double)(y0-y2);
    	
    	if ( y2 < y0){
    		HALF = y2;
      		XA2 = XC1;
      		XB2 = XB1;
    	}else{
    		HALF = y0;
      		XA2 = XA1;
      		XB2 = XC1;
    	} 
    	if (y1 == y2)
    		XA = x2;
    	if (y1 == y0)
			XB = x0;
	}else if (CAS == 2){
		XA = x2;
		XB = x2;
    	XA1 = (x0-x2)/(double)(y0-y2);
    	XB1 = (x1-x2)/(double)(y1-y2);
    	XC1 = (x1-x0)/(double)(y1-y0);
    	if (y0<y1){
    		HALF = y0;
      		XA2 = XC1;
      		XB2 = XB1;
    	}else{
    		HALF = y1;
      		XA2 = XA1;
      		XB2 = XC1;
    	}
    	if (y2 == y0)
    		XA = x0;
    	if (y2 == y1)
    		XB = x1;
	}
  
	if (XA1 > XB1){
		swapd(&XA, &XB);
		swapd(&XA1, &XB1);
		swapd(&XA2, &XB2);
	}
  
	int x,y;
	for (y = t; y < HALF; y++){
		XAd = XA;
		XBd = XB;
		for (x = XAd; x <= XBd; x++)
			l_setPixel(frame, x, y, colour);
		XA += XA1;
		XB += XB1;	
	}
	for (y = HALF; y <= b; y++){
		XAd = XA;
		XBd = XB;
		for (x = XAd; x <= XBd; x++)
			l_setPixel(frame, x, y, colour);
		XA += XA2;
		XB += XB2;	
	}
	return 1;
}

int drawRectangleDottedFilled (TFRAME *frm, int x1, int y1, int x2, int y2, const int colour)
{
	if (!frm) return 0;

	bindCoordinates(frm, &x1, &y1, &x2, &y2);
	
	if (x2 < x1) swapint(&x2,&x1);
	if (y2 < y1) swapint(&y2,&y1);
	int x,y;

	int swp = 0;
	for (x=x1;x<x2+1;x++){
		for (y=y1;y<y2+1;y++){
			l_setPixel_NB(frm, x+swp, y, colour);
			swp ^= 1;
		}
	}
	return 1;
}

int drawRectangleFilled (TFRAME *frm, int x1, int y1, int x2, int y2, const int colour)
{
	if (!frm) return 0;

	bindCoordinates(frm, &x1, &y1, &x2, &y2);
	
	if (x2 < x1) swapint(&x2,&x1);
	if (y2 < y1) swapint(&y2,&y1);

	if (frm->bpp == LFRM_BPP_32){	// take advantage of mmx accel
		for (int y = y1; y <= y2; y++)
			drawLine(frm, x1, y, x2, y, colour);
	}else{
		for (int y = y1; y <= y2; y++){
			for (int x = x1; x < x2+1; x++)
				l_setPixel_NB(frm, x, y, colour);
		}
	}
	
	return 1;
}

int drawRectangleDotted (TFRAME *frm, int x1, int y1, int x2, int y2, const int colour)
{
	if (!frm) return 0;

	//bindCoordinates(frm, &x1, &y1, &x2, &y2);
	
	if (x2 < x1) swapint(&x2,&x1);
	if (y2 < y1) swapint(&y2,&y1);
	int x,y;
	
	for (x = x1; x < x2; x+=2){
		l_setPixel(frm, x, y1, colour);
		l_setPixel(frm, x, y2, colour);
	}
	for (y = y1+1; y < y2; y+=2){
		l_setPixel(frm, x1, y, colour);
		l_setPixel(frm, x2, y, colour);
	}
	return 1;
}

int drawRectangle (TFRAME *frm, int x1, int y1, int x2, int y2, const int colour)
{
	// top
	drawLine(frm, x1, y1, x2, y1, colour);
	// bottom
	drawLine(frm, x1, y2, x2, y2, colour);
	// left
	drawLine(frm, x1, y1+1, x1, y2-1, colour);
	// right
	return drawLine(frm, x2, y1+1, x2, y2-1, colour);
}

int invertArea (TFRAME *frm, const int x1, const int y1, const int x2, const int y2)
{
	int ret;
	const int tmp = frm->style;
	frm->style = LSP_XOR;
	if (frm->bpp == LFRM_BPP_1)
		ret = drawRectangleFilled(frm, x1, y1, x2, y2, frm->style);
	else
		ret = drawRectangleFilled(frm, x1, y1, x2, y2, getRGBMask(frm, LMASK_WHITE));
	frm->style = tmp;
	return ret;
}

int invertFrame (TFRAME *frm)
{
	return invertArea(frm, 0, 0, frm->width-1, frm->height-1);
}

int drawLineDotted (TFRAME *frm, const int x1, const int y1, const int x2, const int y2, const int colour)
{
	if (!frm) return 0;

	//bindCoordinates(frm, &x1, &y1, &x2, &y2);
	
    int dx = x2-x1;
    int dy = y2-y1;

    if (dx || dy){
        if (l_abs(dx) >= l_abs(dy)){
            float y = y1+0.5;
            float dly = (float)dy/(float)dx;
			int xx;
            
            if (dx > 0){
                for (xx = x1; xx<=x2; xx += 2){
                    l_setPixel(frm,xx,(int)y,colour);
                    y += dly;
                }
            }else{
                for (xx = x1; xx>=x2; xx -= 2){
                    l_setPixel(frm,xx,(int)y,colour);
                    y -= dly;
                }
			}
        }else{
           	float x = x1+0.5;
           	float dlx = (float)dx/(float)dy;
           	int yy;

            if (dy > 0){
   	            for (yy = y1; yy<=y2; yy += 2){
       	            l_setPixel(frm,(int)x,yy,colour);
           	        x += dlx;
               	}
			}else{
                for (yy = y1; yy >= y2; yy -= 2){
   	                l_setPixel(frm,(int)x,yy,colour);
       	            x -= dlx;
           	    }
			}
        }
    }else if (!(dx&dy)){
    	l_setPixel(frm,x1,y1,colour);
    }

    return 1;
}

#if 1

static int drawLine32 (TFRAME *frame, int x0, int y0, int x1, int y1, const int colour)
{
	if (!clipLine(frame, x0, y0, x1, y1, &x0, &y0, &x1, &y1))
		return 1;
		
        int dy = y1 - y0;
        y0 *= frame->width;
		int *pixels = (int*)frame->pixels;
        pixels[x0+y0] = colour;
        y1 *= frame->width;
        int dx = x1 - x0;
        int stepx, stepy;
        		
        if (dy < 0) { dy = -dy;  stepy = -frame->width; } else { stepy = frame->width; }
        if (dx < 0) { dx = -dx;  stepx = -1; } else { stepx = 1; }
        dy <<= 1;
        dx <<= 1;

                
        if (dx > dy) {
            int fraction = dy - (dx >> 1);
            while (x0 != x1) {
                if (fraction >= 0) {
                    y0 += stepy;
                    fraction -= dx;
                }
                x0 += stepx;
                fraction += dy;
                pixels[x0+y0] = colour;
            }
        } else {
            int fraction = dx - (dy >> 1);
            while (y0 != y1) {
                if (fraction >= 0) {
                    x0 += stepx;
                    fraction -= dy;
                }
                y0 += stepy;
                fraction += dx;
                pixels[x0+y0] = colour;
            }
        }
	return 1;
}

static int drawLine16 (TFRAME *frame, int x0, int y0, int x1, int y1, const int16_t colour)
{
	if (!clipLine(frame, x0, y0, x1, y1, &x0, &y0, &x1, &y1))
		return 1;
		
        int dy = y1 - y0;
        int dx = x1 - x0;
        int stepx, stepy;
		int16_t *pixels = (int16_t*)frame->pixels;
		
        if (dy < 0) { dy = -dy;  stepy = -frame->width; } else { stepy = frame->width; }
        if (dx < 0) { dx = -dx;  stepx = -1; } else { stepx = 1; }
        dy <<= 1;
        dx <<= 1;

        y0 *= frame->width;
        y1 *= frame->width;
        pixels[x0+y0] = colour;
        if (dx > dy) {
            int fraction = dy - (dx >> 1);
            while (x0 != x1) {
                if (fraction >= 0) {
                    y0 += stepy;
                    fraction -= dx;
                }
                x0 += stepx;
                fraction += dy;
                pixels[x0+y0] = colour;
            }
        } else {
            int fraction = dx - (dy >> 1);
            while (y0 != y1) {
                if (fraction >= 0) {
                    x0 += stepx;
                    fraction -= dy;
                }
                y0 += stepy;
                fraction += dx;
                pixels[x0+y0] = colour;
            }
        }
	return 1;
}

static int drawLine8 (TFRAME *frame, int x0, int y0, int x1, int y1, const char colour)
{
	if (!clipLine(frame, x0, y0, x1, y1, &x0, &y0, &x1, &y1))
		return 1;
		
        int dy = y1 - y0;
        int dx = x1 - x0;
        int stepx, stepy;
		char *pixels = (char*)frame->pixels;
		
        if (dy < 0) { dy = -dy;  stepy = -frame->width; } else { stepy = frame->width; }
        if (dx < 0) { dx = -dx;  stepx = -1; } else { stepx = 1; }
        dy <<= 1;
        dx <<= 1;

        y0 *= frame->width;
        y1 *= frame->width;
        pixels[x0+y0] = colour;
        if (dx > dy) {
            int fraction = dy - (dx >> 1);
            while (x0 != x1) {
                if (fraction >= 0) {
                    y0 += stepy;
                    fraction -= dx;
                }
                x0 += stepx;
                fraction += dy;
                pixels[x0+y0] = colour;
            }
        } else {
            int fraction = dx - (dy >> 1);
            while (y0 != y1) {
                if (fraction >= 0) {
                    x0 += stepx;
                    fraction -= dy;
                }
                y0 += stepy;
                fraction += dx;
                pixels[x0+y0] = colour;
            }
        }
	return 1;
}

static int drawLineFast (TFRAME *frame, int x, int y, int x2, int y2, const int colour)
{

	if (!clipLine(frame, x, y, x2, y2, &x, &y, &x2, &y2))
		return 1;

   	int yLonger = 0;
	int shortLen = y2-y;
	int longLen = x2-x;
	
	if (abs(shortLen) > abs(longLen)){
		swapint(&shortLen, &longLen);
		yLonger = 1;
	}
	int decInc;
	
	if (longLen == 0)
		decInc = 0;
	else
		decInc = (shortLen << 16) / longLen;

	if (yLonger) {
		if (longLen>0) {
			longLen+=y;
			for (int j=0x8000+(x<<16);y<=longLen;++y) {
				l_setPixel_NB(frame,j >> 16,y, colour);
				j+=decInc;
			}
			return 1;
		}
		longLen+=y;
		for (int j=0x8000+(x<<16);y>=longLen;--y) {
			l_setPixel_NB(frame,j >> 16,y, colour);
			j-=decInc;
		}
		return 1;
	}

	if (longLen>0) {
		longLen+=x;
		for (int j=0x8000+(y<<16);x<=longLen;++x){
			l_setPixel_NB(frame,x,j >> 16, colour);
			j+=decInc;
		}
		return 1;
	}
	longLen+=x;
	for (int j=0x8000+(y<<16);x>=longLen;--x){
		l_setPixel_NB(frame,x,j >> 16, colour);
		j-=decInc;
	}
	return 1;
}

int drawLine (TFRAME *frame, int x1, int y1, int x2, int y2, const int colour)
{
	if (frame->style == LSP_SET){
		if (frame->bpp == LFRM_BPP_32)
			return drawLine32(frame, x1, y1, x2, y2, colour);
		else if (frame->bpp == LFRM_BPP_16 || frame->bpp == LFRM_BPP_15 || frame->bpp == LFRM_BPP_12)
			return drawLine16(frame, x1, y1, x2, y2, colour&0xFFFF);
		if (frame->bpp == LFRM_BPP_8)
			return drawLine8(frame, x1, y1, x2, y2, colour&0xFF);
	}
	return drawLineFast(frame, x1, y1, x2, y2, colour);
}

#else

int drawLine (TFRAME *frm, int x1, int y1, int x2, int y2, const int colour)
{
	int x3 = 0, y3 = 0, x4 = 0, y4 = 0;
	if (!clipLine(frm, x1, y1, x2, y2, &x3, &y3, &x4, &y4))
		return 1;

	x1 = x3;
	y1 = y3;
	x2 = x4;
	y2 = y4;
	
    const int dx = x2-x1;
    const int dy = y2-y1;

    if (dx || dy){
        if (l_abs(dx) >= l_abs(dy)){
            float y = y1+0.5;
            float dly = (float)dy/(float)dx;
			int xx;
            
            if (dx > 0){
                for (xx = x1; xx<=x2; xx++){
                    l_setPixel_NB(frm,xx,(int)y,colour);
                    y += dly;
                }
            }else{
                for (xx = x1; xx>=x2; xx--){
                    l_setPixel_NB(frm,xx,(int)y,colour);
                    y -= dly;
                }
			}
        }else{
           	float x = x1+0.5;
           	float dlx = (float)dx/(float)dy;
           	int yy;

            if (dy > 0){
   	            for (yy = y1; yy<=y2; yy++){
       	            l_setPixel_NB(frm,(int)x,yy,colour);
           	        x += dlx;
               	}
			}else{
                for (yy = y1; yy >= y2; yy--){
   	                l_setPixel_NB(frm,(int)x,yy,colour);
       	            x -= dlx;
           	    }
			}
        }
    }else if (!(dx&dy)){
    	l_setPixel_NB(frm,x1,y1,colour);
    }

    return 1;
}
#endif

int drawEnclosedArc (TFRAME *frm, int x, int y, int r1, int r2, float a1, float a2, const int colour)
{
	if (!frm) return 0;

	int myx, myy, lastx=0, lasty=0, N, loop;
	float a, da;

	a1 = 360 - a1;
	a2 = 360 - a2;
	N = (int)l_fabsf(a2-a1)+8;
	a = a1*2.0*M_PI/360.0;
	da = (a2-a1)*(2.0*M_PI/360.0)/(N-1);
	for (loop=0; loop < N; loop++){
		myx = x+(int)(r1*l_cosf(a+loop*da));
		myy = y+(int)(r2*l_sinf(a+loop*da));
		if (loop)
			drawLine(frm, lastx, lasty, myx, myy, colour);
		if (loop==N-1 || !loop)
			drawLine(frm, x, y, myx, myy ,colour);
		lastx = myx;
		lasty = myy;
	}
	return 1;
}

int drawArc (TFRAME *frm, int x, int y, int r1, int r2, float a1, float a2, const int colour)
{
	if (!frm) return 0;
	
	int myx, myy, lastx=0, lasty=0, N, loop;
	float a, da;

	a1 = 360.0 - a1;
	a2 = 360.0 - a2;
	N = (int) l_fabsf((a2-a1)+8.0);
	a = a1*2.0*M_PI/360.0;
	da = (a2-a1)*(2.0*M_PI/360.0)/(N-1);

	for (loop=0; loop < N; loop++){
		myx = x + (int)(r1*l_cosf(a+loop*da));
		myy = y + (int)(r2*l_sinf(a+loop*da));
		if (loop)
			drawLine(frm, lastx, lasty, myx, myy, colour);
			
		lastx = myx;
		lasty = myy;
	}
	return 1;
}

int drawMaskA (const TFRAME *src, const TFRAME *mask, TFRAME *des, const int Xoffset, const int Yoffset, const int srcX1, const int srcY1, const int srcX2, const int srcY2, const float alpha)
{
	if (!src || !mask || !des)
		return 0;

	const float afactor = 1.0/255.0;
	float a1, a2;
	float r1, r2;
	float g1, g2;
	float b1, b2;
	int x, y;

	for (y = srcY1; y<srcY2; y++){
		for (x = srcX1; x<srcX2; x++){
			a1 = alpha * (float)((l_getPixel_NB(mask, x, y)&0xFF) * afactor);
			a2 = 1.0 - a1;
			l_getPixelf(src, x, y, &r1, &g1, &b1);			// src
			l_getPixelf(des, x+Xoffset, y+Yoffset, &r2, &g2, &b2);	// des
			l_setPixelf(des, x+Xoffset, y+Yoffset, (r1 * a1) + (r2 * a2), (g1 * a1) + (g2 * a2), (b1 * a1) + (b2 * a2));
		}
	}
	return 1;
}

int drawMask (TFRAME *src, TFRAME *mask, TFRAME *des, int maskXOffset, int maskYOffset, int mode)
{
	if (!src || !mask || !des)
		return 0;

	int w = MIN(src->width, des->width);
	int h = MIN(src->height, des->height);
	int x,y;

	if (mode==LMASK_AND){
		for (y=maskYOffset;y<h;y++){
			for (x=maskXOffset;x<w;x++)
				l_setPixel(des,x,y,l_getPixel(src,x,y) & l_getPixel(mask,x-maskXOffset,y-maskYOffset));
		}
	}else if (mode==LMASK_OR){
		for (y=0;y<h;y++){
			for (x=0;x<w;x++)
				l_setPixel(des,x,y,l_getPixel(src,x,y) | l_getPixel(mask,x-maskXOffset,y-maskYOffset));
		}
	}else if (mode==LMASK_XOR && (des->bpp == LFRM_BPP_32A || mask->bpp == LFRM_BPP_32A)){
		for (y=0;y<h;y++){
			for (x=0;x<w;x++)
				l_setPixel(des,x,y,l_getPixel(src,x,y) ^ (l_getPixel(mask,x-maskXOffset,y-maskYOffset)&0xFFFFFF));
		}
	}else if (mode==LMASK_XOR){
		for (y=0;y<h;y++){
			for (x=0;x<w;x++)
				l_setPixel(des,x,y,l_getPixel(src,x,y) ^ l_getPixel(mask,x-maskXOffset,y-maskYOffset));
		}
	}else if (mode==LMASK_CPYSRC){
		for (y=0;y<h;y++){
			for (x=0;x<w;x++)
				l_setPixel(des,x,y,l_getPixel(src,x,y));
		}
	}else if (mode==LMASK_CPYMASK){
		for (y=maskYOffset;y<h;y++){
			for (x=maskXOffset;x<w;x++)
				l_setPixel(des,x,y,l_getPixel(mask,x-maskXOffset,y-maskYOffset));
		}
	}else if (mode==LMASK_CLEAR){
		copyFrame(src, des);
		for (y=maskYOffset;y<h;y++){
			for (x=maskXOffset;x<w;x++){
				if (l_getPixel(mask,x-maskXOffset,y-maskYOffset))
					l_setPixel(des,x,y,LSP_CLEAR);
			}
		}
	}else{
		return 0;
	}
	return 1;
}

int drawCircle (TFRAME *frm, const int xc, const int yc, const int radius, const int colour)
{
	if (!frm) return 0;

	float x = 0.0; 
	float y = radius;
	float p = 1.0-radius;

	circlePts(frm, xc, yc, x, y, colour);
	while (x < y){
		x += 1.0;
		if (p < 0){
			p += 2.0*x+1.0;
		}else{
			y -= 1.0;
			p += 2.0*x+1.0-2.0*y;
		}
		circlePts(frm, xc, yc, x, y, colour);
	}
	return 1;
}

static int floodfill_init (TFILL *fill, TFRAME *frame)
{
	fill->size = frame->width * frame->height;
	fill->stack = l_malloc(fill->size * sizeof(T2POINT));
	if (fill->stack != NULL){
		fill->width = frame->width;
		fill->height= frame->height;
		fill->frame = frame;
		fill->position = 0;
		return 1;
	}else{
		return 0;
	}
}

static void floodfill_cleanup (TFILL *fill)
{
	if (fill->stack != NULL)
		l_free(fill->stack);
	fill->stack = NULL;
	fill->position = 0;
}

static int floodfill_pop (TFILL *fill, int *x, int *y) 
{ 
    if (fill->position > 0){ 
        *x = fill->stack[fill->position].x;
        *y = fill->stack[fill->position].y;
        fill->position--; 
        return 1; 
    }else{ 
        return 0; 
    }    
}    

static int floodfill_push (TFILL *fill, const int x, const int y) 
{ 
    if (fill->position < fill->size - 1){ 
        fill->position++; 
        fill->stack[fill->position].x = x;
        fill->stack[fill->position].y = y;
        return 1; 
	}else{ 
        return 0; 
    }    
}     

static void emptyStack (TFILL *fill) 
{ 
    int x = 0, y = 0; 
    while(floodfill_pop(fill, &x, &y)); 
}

static int floodFill_op (TFILL *fill, int x, int y, const int newColor, const int oldColor)
{
    if (oldColor == newColor) return 0;
   
    emptyStack(fill);
    int y1, spanLeft, spanRight;
    
    if (!floodfill_push(fill, x, y))
    	return 0;
    
    while (floodfill_pop(fill, &x, &y)){    
        y1 = y;
        while (y1 >= 0 && l_getPixel(fill->frame, x, y1) == oldColor)
        	y1--;
        y1++;
        spanLeft = spanRight = 0;
        
        while (y1 < fill->height && l_getPixel(fill->frame, x, y1) == oldColor){
            l_setPixel(fill->frame, x, y1, newColor);
            
            if (!spanLeft && x > 0 && l_getPixel(fill->frame, x - 1, y1) == oldColor){
                if (!floodfill_push(fill, x - 1, y1)) return 0;
                spanLeft = 1;
            }else if (spanLeft && x > 0 && l_getPixel(fill->frame, x - 1, y1) != oldColor){
                spanLeft = 0;
            }
            
            if (!spanRight && x < fill->frame->width - 1 && l_getPixel(fill->frame, x + 1, y1) == oldColor){
                if (!floodfill_push(fill, x + 1, y1)) return 0;
                spanRight = 1;
            }else if (spanRight && x < fill->frame->width - 1 && l_getPixel(fill->frame, x + 1, y1) != oldColor){
                spanRight = 0;
            } 
            y1++;
        }
    }
    return 1;
}


static void circlePts (TFRAME *frm, int xc, int yc, int x, int y, const int colour)
{
	l_setPixel(frm, xc+y, yc-x, colour);
	l_setPixel(frm, xc-y, yc-x, colour);
	l_setPixel(frm, xc+y, yc+x, colour);
	l_setPixel(frm, xc-y, yc+x, colour);
	l_setPixel(frm, xc+x, yc+y, colour);
	l_setPixel(frm, xc-x, yc+y, colour);
	l_setPixel(frm, xc+x, yc-y, colour);
	l_setPixel(frm, xc-x, yc-y, colour);
}

// bind coordinate to frame
static void bindCoordinates (TFRAME *frame, int *x1, int *y1, int *x2, int *y2)
{
	if (*x1 > frame->width-1)
		*x1 = frame->width-1;
	else if (*x1 < 0)
		*x1 = 0;
		
	if (*x2 > frame->width-1)
		*x2 = frame->width-1;
	else if (*x2 < 0)
		*x2 = 0;

	if (*y1 > frame->height-1)
		*y1 = frame->height-1;
	else if (*y1 < 0)
		*y1 = 0;
		
	if (*y2 > frame->height-1)
		*y2 = frame->height-1;
	else if (*y2 < 0)
		*y2 = 0;
}

static int findRegion (TFRAME *frame, int x, int y)
{
	int code=0;
	
	if (y >= frame->height)
		code |= 1; //top
	else if( y < 0)
		code |= 2; //bottom
		
	if (x >= frame->width)
		code |= 4; //right
	else if ( x < 0)
		code |= 8; //left

  return(code);
}

static void horLine (TFRAME *frame, int y, int x1, int x2, int colour)
{
	drawLine(frame, x1, y, x2, y, colour);
}

static int clipLine (TFRAME *frame, int x1, int y1, int x2, int y2, int *x3, int *y3, int *x4, int *y4)
{
  
  int accept = 0, done = 0;
  int code1 = findRegion(frame, x1, y1); //the region outcodes for the endpoints
  int code2 = findRegion(frame, x2, y2);
  
  const int h = frame->height;
  const int w = frame->width;
  
  do{  //In theory, this can never end up in an infinite loop, it'll always come in one of the trivial cases eventually
    if (!(code1 | code2)){
    	accept = done = 1;  //accept because both endpoints are in screen or on the border, trivial accept
    }else if (code1 & code2){
    	done = 1; //the line isn't visible on screen, trivial reject
    }else{  //if no trivial reject or accept, continue the loop

      int x, y;
	  int codeout = code1 ? code1 : code2;
      if (codeout&1){			//top
        x = x1 + (x2 - x1) * (h - y1) / (y2 - y1);
        y = h - 1;
      }else if (codeout & 2){	//bottom
        x = x1 + (x2 - x1) * -y1 / (y2 - y1);
        y = 0;
      }else if (codeout & 4){	//right
        y = y1 + (y2 - y1) * (w - x1) / (x2 - x1);
        x = w - 1;
      }else{					//left
        y = y1 + (y2 - y1) * -x1 / (x2 - x1);
        x = 0;
      }
      
      if (codeout == code1){ //first endpoint was clipped
        x1 = x; y1 = y;
        code1 = findRegion(frame, x1, y1);
      }else{ //second endpoint was clipped
        x2 = x; y2 = y;
        code2 = findRegion(frame, x2, y2);
      }
    }
  }
  while(done == 0);

  if (accept){
    *x3 = x1;
    *x4 = x2;
    *y3 = y1;
    *y4 = y2;
    return 1;
  }else{
   // *x3 = *x4 = *y3 = *y4 = 0;
    return 0;
  }
}

static int edgefill_init (TFILL *stack, TFRAME *frame)
{
	stack->size = frame->width * frame->height;
	stack->stack = l_calloc(sizeof(T2POINT), stack->size);
	if (stack->stack){
		stack->position = 0;
		stack->width = frame->width;
		stack->height = frame->height;
		stack->frame = frame; /*unused*/
		return 1;
	}else{
		return 0;
	}
}

static void edgefill_cleanup (TFILL *stack)
{
	if (stack->stack)
		l_free(stack->stack);
	stack->stack = NULL;
	stack->size = 0;
	stack->position = 0;
}

#if 0
static int edgefill_search (TFILL *stack, int x, int y)
{
	int i;
	for (i = 0; i < stack->position; i++){
		if (stack->stack[i].x == x && stack->stack[i].y == y)
			return i+1;
	}
	return 0;
}
#endif

static int edgefill_push (TFILL *stack, const int x, const int y)
{
	if (stack->position >= stack->size-1){
		printf("edgefill_op: out of stack %i %i\n", x, y);
		return 0;
	}

	if (x < 0 || x >= stack->width || y < 0 || y >= stack->height)
		return 0;

//	if (!edgefill_search(stack, x, y)){
		stack->stack[stack->position].x = x;
		stack->stack[stack->position].y = y;
		return ++stack->position;
//	}else{
//		return stack->position;
//	}
}

static int edgefill_pop (TFILL *stack, int *x, int *y)
{
	if (!stack->position)
		return 0;
	
	*x = stack->stack[--stack->position].x;
	*y = stack->stack[stack->position].y;
	return stack->position+1;
}

static int edgefill_op (TFRAME *frame, TFILL *stack, const int ox, const int oy, const int newColour, const int edgeColour)
{
	if (l_getPixel(frame, ox, oy) == edgeColour)
		return 1;
	
	int x = 0 , y = 0;
	int colour;
	
	edgefill_push(stack, ox, oy);
	while(edgefill_pop(stack, &x, &y)){
		//printf("%i x:%i y:%i\n", stack->position, x, y);
		l_setPixel(frame, x, y, newColour);
		
		//check neighbour to the right
		colour = l_getPixel(frame, x+1, y);
		if (colour != newColour && colour != edgeColour){
			if (!edgefill_push(stack, x+1, y))
				return 0;
		}
			
		//check neighbour to the left
		colour = l_getPixel(frame, x-1, y);
		if (colour != newColour && colour != edgeColour){
			if (!edgefill_push(stack, x-1, y))
				return 0;
		}
		
		//check neighbour above
		colour = l_getPixel(frame, x, y+1);
		if (colour != newColour && colour != edgeColour){
			if (!edgefill_push(stack, x, y+1))
				return 0;
		}

		//check neighbour below
		colour = l_getPixel(frame, x, y-1);
		if (colour != newColour && colour != edgeColour){
			if (!edgefill_push(stack, x, y-1))
				return 0;
		}
	}
	return 1;
}

#else

int invertFrame (TFRAME *frm){return 0;}
int invertArea (TFRAME *frm, int x1, int y1, int x2, int y2){return 0;}
int drawRectangleDotted (TFRAME *frm, int x1, int y1, int x2, int y2, const int colour){return 0;}
int drawRectangleFilled (TFRAME *frm, int x1, int y1, int x2, int y2, const int colour){return 0;}



#endif

