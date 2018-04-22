
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



int buttonParticles (const TTOUCHCOORD *pos, TBUTTON *button, const int id, const int message, void *ptr)
{
	//TVLCPLAYER *vp = (TVLCPLAYER*)ptr;
		
	switch (id){
	  //case 
	 }
	return 1;
}	  
	  
int closeParticles (TVLCPLAYER *vp, TFRAME *frame, TPARTICLES *part)
{
	if (part->state)
		GLParticlesClose();
	part->state = 0;
	return 1;
}

int openParticles (TVLCPLAYER *vp, TFRAME *frame, TPARTICLES *part)
{	
	part->state = 0;
	return 1;
}

int drawParticles (TVLCPLAYER *vp, TFRAME *frame, TPARTICLES *part)
{
	if (vp->gui.page_gl != PAGE_PARTICLES){
		vp->gui.page_gl = PAGE_PARTICLES;
		
		if (part->state)
			GLParticlesClose();
		GLParticlesInit(frame);
		part->state = 1;
	}
	
	setAwake(vp);
	GLParticlesDraw(frame);
	return 1;
}

int touchParticles (TTOUCHCOORD *pos, int flags, TVLCPLAYER *vp)
{
	//setPage(vp, PAGE_NONE);
	setPageSec(vp, -1);
	return 0;
}

