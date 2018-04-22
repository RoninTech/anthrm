// playlist media control overlay
// (next trk, play, art size, etc..)
//
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


wchar_t *editboxGetString (TEDITBOX *input)
{
	return my_wcsdup(input->workingBuffer);
}

static void toggleCaret (TEDITBOX *input)
{
	if (input->caretChar == 5)	// custom chars added to 5x7.bdf
		input->caretChar = 6;
	else
		input->caretChar = 5;
}

int getCaretCharLeft (TEDITBOX *input, int n)
{
	return input->workingBuffer[input->caretPos-n]&0xFF;
}

int getCaretCharRight (TEDITBOX *input, int n)
{
	--n;
	return input->workingBuffer[input->caretPos+n]&0xFF;
}

int addCaret (TEDITBOX *input, wchar_t *src, wchar_t *des, size_t desSize)
{
	toggleCaret(input);
	size_t srcLen = wcslen(src);
	
	if (!srcLen){
		des[0] = input->caretChar;
		des[1] = 0;
	}else if (input->caretPos >= input->tKeys){
		wcsncpy(des, src, srcLen);
		des[srcLen] = input->caretChar;
		des[srcLen+1] = 0;
	}else{
		memset(des, 0, EDITBOXIN_INPUTBUFFERLEN*sizeof(wchar_t));
		wcsncpy(des, src, input->caretPos);
		des[input->caretPos] = input->caretChar;
		wcsncpy(&des[input->caretPos+1], src+input->caretPos, wcslen(src+input->caretPos));
	}
	return 1;
}

void drawEditBox (TEDITBOX *input, TFRAME *frame, const int x, const int y, wchar_t *ptext, unsigned int *offset)
{
	
	TLPRINTR rect = {x+2,y+2,frame->width-3,frame->height-5,x,y,x,y+7};
	TLPRINTR rect2;
	
	memcpy(&rect2, &rect, sizeof(TLPRINTR));
	const int renderFlags = PF_DONTFORMATBUFFER|PF_WORDWRAP|PF_CLIPWRAP|PF_CLIPTEXTV;
	wchar_t *text = ptext+*offset;

	lPrintEx(frame, &rect2, BFONT, renderFlags|PF_DONTRENDER, LPRT_CPY, (char*)text);
	if (!*input->workingBuffer){
		rect2.ex = x+1;	// we've got a rect now factor in a border
		rect2.ey += 1;
	}
	lDrawRectangleFilled(frame, x, y, frame->width-2, rect2.ey+1, 180<<24);
	lPrintEx(frame, &rect, BFONT, renderFlags, LPRT_CPY, (char*)text);

	if (!*input->workingBuffer){
		rect.ex = x+1;
		rect.ey += 1;
	}
	lDrawRectangle(frame, x, y, frame->width-2, rect.ey+1, 0xFFFFFFFF);
}

static int addKey (TEDITBOX *input, int key, int position)
{
	if (input->tKeys < EDITBOXIN_INPUTBUFFERLEN-1){
		if (position == input->tKeys){
			input->caretPos++;
			input->workingBuffer[input->tKeys++] = key;
			return 1;
		}else{
			wchar_t *src = input->workingBuffer;
			wchar_t *des = input->caretBuffer;
			
			memset(des, 0, EDITBOXIN_INPUTBUFFERLEN*sizeof(wchar_t));
			wcsncpy(des, src, position);
			des[position] = key;
			wcsncpy(&des[position+1], src+position, wcslen(src+position));
			wcsncpy(src, des, wcslen(des));
			
			input->caretPos++;
			input->tKeys++;
			return 1;
		}
	}
	return 0;

}

static void addKeyTab (TEDITBOX *input)
{
	addKey(input, L'\t', input->caretPos);
}

static int deleteKey (TEDITBOX *input, int position)
{
	wchar_t *src = input->workingBuffer;
	wchar_t *des = input->caretBuffer;
	
	memset(des, 0, EDITBOXIN_INPUTBUFFERLEN*sizeof(wchar_t));
	wcsncpy(des, src, input->caretPos-1);
	wcsncpy(&des[input->caretPos-1], src+input->caretPos, wcslen(src+input->caretPos));
	memset(src, 0, EDITBOXIN_INPUTBUFFERLEN*sizeof(wchar_t));
	wcsncpy(src, des, wcslen(des));
	
	if (input->iOffset) input->iOffset--;
	if (input->caretPos) input->caretPos--;
	if (input->tKeys) input->tKeys--;
	return 1;
}

#if 0
// used by emoticons
static int getPreviousKey (TEDITBOX *input)
{
	if (input->caretPos < EDITBOXIN_INPUTBUFFERLEN && input->caretPos > 0)
		return input->workingBuffer[input->caretPos-1];
	else
		return 0;
}
#endif

void pasteClipBoardText (HWND hwnd, TEDITBOX *input, int emoticons)
{
	if (OpenClipboard(hwnd)){
		HANDLE handle = GetClipboardData(CF_UNICODETEXT);
		if (handle){
			wchar_t *buffer = (wchar_t*)GlobalLock(handle);
			if (buffer){
				while(*buffer){
					if (*buffer == VK_TAB){
						addKeyTab(input);

					}else if (*buffer < 32){
						addKey(input, L' ', input->caretPos);
#if 0						
					}else if (emoticons){
						if (*buffer == L'(' && getPreviousKey(input) == L':'){
							addKey(input, 4, input->caretPos-1);
							deleteKey(input, input->caretPos);
						}else if (*buffer == L')' && getPreviousKey(input) == L':'){
							addKey(input, 3, input->caretPos-1);
							deleteKey(input, input->caretPos);
						}else{
							addKey(input, *buffer, input->caretPos);
						}
#endif
					}else{
						addKey(input, *buffer, input->caretPos);
					}
					buffer++;
				}
				GlobalUnlock(handle);
			}
		}
		CloseClipboard();
	}
}

void clearWorkingBuffer (TEDITBOX *input)
{
	memset(input->workingBuffer, 0, EDITBOXIN_INPUTBUFFERLEN*sizeof(wchar_t));
	input->iOffset = input->caretPos = input->tKeys = 0;
}

int nextHistoryBuffer (TEDITBOX *input)
{
	if (++input->historyBufferi >= EDITBOXIN_WORKINGBUFFERS)
		input->historyBufferi = 0;
	return input->historyBufferi;
}

int previousHistoryBuffer (TEDITBOX *input)
{
	if (--input->historyBufferi < 0)
		input->historyBufferi = EDITBOXIN_WORKINGBUFFERS-1;
	return input->historyBufferi;
}

void addWorkingBuffer (TEDITBOX *input)
{
	memcpy(input->buffers[input->historyBufferi], input->workingBuffer, EDITBOXIN_INPUTBUFFERLEN*sizeof(wchar_t));
}

static void addHistoryBuffer (TEDITBOX *input)
{
	memcpy(input->workingBuffer, input->buffers[input->historyBufferi], EDITBOXIN_INPUTBUFFERLEN*sizeof(wchar_t));
	input->caretPos = input->tKeys = wcslen(input->workingBuffer);
	input->iOffset = 0;
}

int editBoxInputProc (TEDITBOX *input, HWND hwnd, int key, int emoticons)
{
	if (key == VK_LSHIFT || key == VK_SHIFT || key == VK_RSHIFT)
		return 0;
			
	//printf("%i\n",key);

	if (key&0x1000){
		key &= ~0x1000;
			
		if (key >= VK_PRIOR && key <= VK_DELETE){
			//printf("%i\n",key);
			if (key == VK_LEFT){
				if (--input->caretPos < 1){
					input->caretPos = 0;
					input->iOffset = 0;
				}
			}else if (key == VK_RIGHT){
				if (++input->caretPos > input->tKeys)
					input->caretPos = input->tKeys;

			}else if (key == VK_DELETE){
				if (++input->caretPos > input->tKeys){
					input->caretPos = input->tKeys;
				}else{
					deleteKey(input, input->caretPos);
				}
			}else if (key == VK_UP){
				previousHistoryBuffer(input);
				addHistoryBuffer(input);
					
			}else if (key == VK_DOWN){
				nextHistoryBuffer(input);
				addHistoryBuffer(input);
					
			}else if (key == VK_HOME){
				input->caretPos = 0;
				input->iOffset = 0;
					
			}else if (key == VK_END){
				input->caretPos = input->tKeys;
				input->iOffset = 0;
					
			}else{
				return 0;
			}
			return 1;
		}
		return 0;
	 		
	}else if (key == 22){			// Control + V
		pasteClipBoardText(hwnd, input, emoticons);
		
	}else if (key == 6){			// Control + F
		addWorkingBuffer(input);
		nextHistoryBuffer(input);
		clearWorkingBuffer(input);
		addKey(input, CMDPARSER_CMDIDENT, input->caretPos);
		
		const wchar_t *find = L"find ";
		int len = wcslen(find);
		for (int i = 0; i < len; i++)
			addKey(input, find[i], input->caretPos);
			
		return 0;
		
	}else if (key == VK_RETURN){
		//kHookOff();
		//kHookUninstall();
		return 2;
			
	}else if (key == VK_ESCAPE){
		//kHookOff();
		//kHookUninstall();
		return 3;
			
	}else if (key == VK_TAB){
		addKeyTab(input);
			
	}else if (key == VK_BACK){
	 	if (input->tKeys && input->caretPos)
	 		deleteKey(input, input->caretPos);

#if 0
	}else if (key == G15_SOFTKEY_1 || key == G15_SOFTKEY_2 || key == G15_SOFTKEY_3 || key == G15_SOFTKEY_4){
		kbHookOff();
		return -1;
#endif

	}else if (key < 31){
		// do nothing
#if 0
	}else if (emoticons){	// do emoticons
		if (key == L'(' && getPreviousKey(input) == L':'){
			addKey(input, 4, input->caretPos-1);
			deleteKey(input, input->caretPos);
		}else if (key == L')' && getPreviousKey(input) == L':'){
			addKey(input, 3, input->caretPos-1);
			deleteKey(input, input->caretPos);
		}else{
			addKey(input, key, input->caretPos);
		}
#endif
	}else{
		addKey(input, key, input->caretPos);
	}

	return 1;
}
