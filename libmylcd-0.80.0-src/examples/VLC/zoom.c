
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




#include "common.h"



void zoomDelete (TZOOM *zoom)
{
	my_free(zoom);
}

TZOOM *zoomCreate (TFRAME *image, const float scale, const float scaleRate, const int direction)
{
	TZOOM *zoom = (TZOOM*)my_malloc(sizeof(TZOOM));
	if (zoom){
		zoom->image = image;
		zoom->scaleDefault = zoom->scale = scale;
		zoom->scaleRate = scaleRate;
		zoom->direction = direction;
		return zoom;
	}
	return NULL;
}

int zoomRender (TZOOM *zoom, TFRAME *frame, const int x, const int y)
{
	const float w = zoom->image->width;
	const float h = zoom->image->height;
	
	if (zoom->direction == 1){
		zoom->scale -= zoom->scaleRate;
		if (zoom->scale < 0.8)
			zoom->direction = 2;
	}
	if (zoom->direction == 2){
		zoom->scale += zoom->scaleRate;
		if (zoom->scale > zoom->scaleDefault){
			zoom->scale = zoom->scaleDefault;
		}
	}
	drawImageScaled(zoom->image,
		frame,
		x+((w/2.0)*(zoom->scaleDefault-zoom->scale)),
		y+((h/2.0)*(zoom->scaleDefault-zoom->scale)),
		zoom->scale
	);
	return (zoom->scale != zoom->scaleDefault);
}

