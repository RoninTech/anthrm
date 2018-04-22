
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



#ifndef _CFG_H_
#define _CFG_H_



int closeCfg (TVLCPLAYER *vp, TFRAME *frame, TCFG *cfg);
int openCfg (TVLCPLAYER *vp, TFRAME *frame, TCFG *cfg);
int drawCfg (TVLCPLAYER *vp, TFRAME *frame, TCFG *cfg);
int touchCfg (TTOUCHCOORD *pos, int flags, TVLCPLAYER *vp);
int buttonCfg (const TTOUCHCOORD *pos, TBUTTON *button, const int id, const int message, void *ptr);

void setRBSwap (TVLCPLAYER *vp, int state);
void setVis (TVLCPLAYER *vp, int visButton);
void setAR (TVLCPLAYER *vp, int arButton);
void reloadSkin (TVLCPLAYER *vp);

#endif

