
// libmylcd - http://mylcd.sourceforge.net/
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
//  GNU LIBRARY GENERAL PUBLIC LICENSE for more details.
//
//	You should have received a copy of the GNU Library General Public
//	License along with this library; if not, write to the Free
//	Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.


#include "common.h"
#include <time.h>

int closeClk (TVLCPLAYER *vp, TFRAME *frame, TCLK *clk)
{
	lDeleteFrame(clk->back);
	return 1;
}

int openClk (TVLCPLAYER *vp, TFRAME *frame, TCLK *clk)
{	
	clk->face = imageCacheAddImage(vp, L"clock/face.png", SKINFILEBPP, &vp->gui.image[IMGC_CLKFACE]);
	clk->hr = imageCacheAddImage(vp, L"clock/hr.png", SKINFILEBPP, &vp->gui.image[IMGC_CLKHR]);
	clk->min = imageCacheAddImage(vp, L"clock/min.png", SKINFILEBPP, &vp->gui.image[IMGC_CLKMIN]);
	clk->sec = imageCacheAddImage(vp, L"clock/sec.png", SKINFILEBPP, &vp->gui.image[IMGC_CLKSEC]);
	clk->back = lNewFrame(clk->face->hw, clk->face->width, clk->face->height, clk->face->bpp);
	clk->pos.x = (frame->width - clk->face->width)/2;
	clk->pos.y = (frame->height - clk->face->height)/2;
	clk->t0 = 0;	
	return (clk->face && clk->hr && clk->min && clk->sec);
}

void drawClock (TVLCPLAYER *vp, TCLK *clk, TFRAME *frame, const int x, const int y, const int drawSec)
{
	clk->face = imageCacheGetImage(vp, vp->gui.image[IMGC_CLKFACE]);
	clk->hr = imageCacheGetImage(vp, vp->gui.image[IMGC_CLKHR]);
	clk->min = imageCacheGetImage(vp, vp->gui.image[IMGC_CLKMIN]);
	clk->sec = imageCacheGetImage(vp, vp->gui.image[IMGC_CLKSEC]);
	
	const int xc = frame->width/2;
	const int yc = frame->height/2;
	const int hhh = clk->min->height/2;	// half hand height
	const int hhw = clk->min->width/2;	// half hand width

	time_t t = time(0);
    const struct tm *tdate = localtime(&t);
	const float hr = tdate->tm_hour;
	const float min = tdate->tm_min;
	const float sec = tdate->tm_sec;

	const float s = 6.0 * sec;	// 6 = (360.0 / 60.0)
	const float m = 6.0 * min;	// 30 = 360.0 / 12.0
	const float h = 30.0 * (hr+((1.0/60.0)*min));

	drawImage(clk->face, frame, x, y, clk->face->width-1, clk->face->height-1);
	if (drawSec)
		lRotate(clk->sec, frame, xc-hhw, yc-hhh, s);
	lRotate(clk->hr, frame, xc-hhw, yc-hhh, h);
	lRotate(clk->min, frame, xc-hhw, yc-hhh, m);
}

int drawClk (TVLCPLAYER *vp, TFRAME *frame, TCLK *clk)
{
#if 1
	if (vp->fTime > clk->t0){
		clk->t0 = vp->fTime + 980;
		drawClock(vp, clk, clk->back, 0, 0, !getIdle(vp));
	}
	drawImage(clk->back, frame, clk->pos.x, clk->pos.y, clk->back->width-1, clk->back->height-1);
#else
	drawClock(vp, clk, frame, clk->pos.x, clk->pos.y, !getIdle(vp));
#endif
	return 1;
}

int touchClk (TTOUCHCOORD *pos, int flags, TVLCPLAYER *vp)
{
	setPageSec(vp, -1);
	return 0;
}

