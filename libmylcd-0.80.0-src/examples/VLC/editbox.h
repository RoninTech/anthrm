
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


#ifndef _EDITBOX_H_
#define _EDITBOX_H_

#define EDITBOXIN_WORKINGBUFFERS	8
#define EDITBOXIN_INPUTBUFFERLEN	440
#define EDITBOXCMD_MAXCMDLEN		16
#define EDITBOXCMD_MAXCMDS			64

typedef struct{
	int state;
	
	void (*pfunc) (wchar_t *var, int vlen, void *uptr, int data1, int data2);
	wchar_t name[EDITBOXCMD_MAXCMDLEN];	/* command name */
	wchar_t alias[EDITBOXCMD_MAXCMDLEN];	/* an alias */

	void *uptr;	// user variables
	int data1;
	int data2;
}TEDITBOXCMD;

typedef struct{
	wchar_t buffers[EDITBOXIN_WORKINGBUFFERS][EDITBOXIN_INPUTBUFFERLEN+1];
	wchar_t caretBuffer[EDITBOXIN_INPUTBUFFERLEN+1];
	wchar_t workingBuffer[EDITBOXIN_INPUTBUFFERLEN+1];
	
	int historyBufferi;
	int caretChar;
	int caretPos;
	size_t tKeys;
	size_t iOffset;
	
	TEDITBOXCMD	registeredCmds[EDITBOXCMD_MAXCMDS];
	int registeredCmdTotal;
}TEDITBOX;


int addCaret (TEDITBOX *input, wchar_t *src, wchar_t *des, size_t desSize);
void drawEditBox (TEDITBOX *input, TFRAME *frame, const int x, const int y, wchar_t *ptext, unsigned int *offset);
void drawCharTotal (TEDITBOX *input, TFRAME *frame, const int x, const int y);

int editBoxInputProc (TEDITBOX *input, HWND hwnd, int key, int emoticons);

wchar_t *editboxGetString (TEDITBOX *input);

void clearWorkingBuffer (TEDITBOX *input);
void addWorkingBuffer (TEDITBOX *input);

int previousHistoryBuffer (TEDITBOX *input);
int nextHistoryBuffer (TEDITBOX *input);

int exitboxProcessString (TEDITBOX *input, wchar_t *txt, int ilen, void *ptr);
void editboxDoCmdRegistration (TEDITBOX *input, void *vp);

#endif

