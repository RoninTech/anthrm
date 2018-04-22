
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


#ifndef _BUTTON_H_
#define _BUTTON_H_




int buttonsGetTotal (TVLCPLAYER *vp, const int page_id);
TBUTTON *buttonsCreateContainer (TVLCPLAYER *vp, const int page_id, const int total);
void buttonsDeleteContainer (TVLCPLAYER *vp, const int page_id);
TBUTTONS *buttonsGetContainer (TVLCPLAYER *vp, const int page_id);
void buttonsFreeStorage (TBUTTON *button, const int total);

void buttonsRender (TBUTTON *button, int btotal, TFRAME *frame);

TBUTTON *buttonGet (TVLCPLAYER *vp, const int page_id, const int btn_id);
int buttonSetImages (TVLCPLAYER *vp, TBUTTON *button, wchar_t *image, wchar_t *highlight);
void buttonEnable (TVLCPLAYER *vp, const int page_id, const int btn_id);
void buttonDisable (TVLCPLAYER *vp, const int page_id, const int btn_id);



#endif



