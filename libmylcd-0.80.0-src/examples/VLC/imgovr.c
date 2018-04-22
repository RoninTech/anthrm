
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


int setImageOverlayImage (TVLCPLAYER *vp, TFRAME *frame, wchar_t *filename)
{
	TIOVR *iovr = getPagePtr(vp, PAGE_IMGOVR);
	TFRAME *tmp = lNewImage(frame->hw, filename, LFRM_BPP_32A/*iovr->img->bpp*/);
	if (tmp){
		int w, h;
		if (tmp->height > tmp->width) lRotateFrameL90(tmp);
		imageBestFit(iovr->width, iovr->height, tmp->width, tmp->height, &w, &h);
		lResizeFrame(iovr->img, w+2, h+2, 0);
		lCopyAreaScaled(tmp, iovr->img, 0, 0, tmp->width, tmp->height, 1, 1, w, h, LCASS_CPY);
		lDrawRectangle(iovr->img, 0, 0, iovr->img->width-1, iovr->img->height-1, 210<<24 | 0xFFFFFF);
		iovr->x = (frame->width - iovr->img->width)/2.0;
		iovr->y = (frame->height - iovr->img->height)/2.0;
		iovr->enabled = 1;
		lDeleteFrame(tmp);
		return 1;
	}else{
		iovr->enabled = 0;
		return 0;
	}
}

int closeIOvr (TVLCPLAYER *vp, TFRAME *frame, TIOVR *iovr)
{
	if (iovr->img)
		lDeleteFrame(iovr->img);
	return 1;
}

int openIOvr (TVLCPLAYER *vp, TFRAME *frame, TIOVR *iovr)
{	
	iovr->x = 5;
	iovr->y = 5;
	iovr->width = (frame->width - iovr->x - 5);
	iovr->height = frame->height - (iovr->y*2);
	iovr->img = lNewFrame(vp->hw, 8, 8, LFRM_BPP_32A);
	return (iovr->img != NULL);
}

int drawIOvr (TVLCPLAYER *vp, TFRAME *frame, TIOVR *iovr)
{
	if (iovr->enabled)
		fastFrameCopy(iovr->img, frame, iovr->x, iovr->y);
		//lDrawImage(iovr->img, frame, iovr->x, iovr->y);
	return 1;
}

int touchIOvr (TTOUCHCOORD *pos, int flags, TVLCPLAYER *vp)
{
	TIOVR *iovr = getPagePtr(vp, PAGE_IMGOVR);
	
	// check if image overlay was touched, if so then remove the overlay
	if (pos->dt > 100 || flags == 0){
		if (pos->x > iovr->x && pos->x < iovr->x+iovr->img->width){
			if (pos->y > iovr->y && pos->y < iovr->y+iovr->img->height){
				setPageSec(vp, -1);
				return 0;
			}
		}
	}
	return 0;
}

