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


typedef struct Point4Struct {
	int total;
	int ab;
	int ac;
	int bc;
} Point4;

typedef struct Point3Struct {
	double x;
	double y;
	double z;
} Point3;

typedef struct Point2Struct {
	int x;
	int y;
} Point2;

typedef struct {
	Point3	*vert;
	Point4	*face;
	Point2	*pt;
	
	int		tvert;
	int		tface;
} TOBJECT;

typedef struct {
	TOBJECT	obj[40];
	int		tobj;
	
	double	zoom;
	double	flength;
	int		destx;
	int		desty;
} TMODEL;


#include "ncc1701d.h"

#if (_BenchMark_)

#define TOTALMODELS 1

#else

#include "mother1.h"
#include "mother2.h"
#include "light.h"
#include "skull.h"
#include "tricrtps.h"
#include "x29.h"
#include "face.h"
#include "plan.h"
#include "duck.h"
#include "num3.h"
#include "hammer.h"
#include "kitchenchair.h"
#include "ball.h"
#include "cube.h"

#define TOTALMODELS 15

#endif
