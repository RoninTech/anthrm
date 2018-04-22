
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


// http://www.jstookey.com/arcade/rawmouse/



#include "common.h"

#if MOUSEHOOKCAP

typedef struct {
	TVLCPLAYER *vp;
	TTOUCHCOORD pos;
	int flags;
}TMBCLICK;



void (CALLBACK mbClickCB)(UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2)
{
	
	//SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_IDLE);
	
	TMBCLICK *mb = (TMBCLICK*)dwUser;
	if (mb){
		if (mb->vp->applState)
			touchDispatch(&mb->pos, mb->flags, mb->vp);
		my_free(mb);
	}
}

void touchSimulate (const TTOUCHCOORD *pos, const int flags, TVLCPLAYER *vp)
{
#if 0
	touchDispatch((TTOUCHCOORD*)pos, flags, vp);
#else
	
	TMBCLICK *mb = my_malloc(sizeof(TMBCLICK));
	if (mb){
		mb->vp = vp;
		mb->flags = flags;
		my_memcpy(&mb->pos, pos, sizeof(TTOUCHCOORD));

		timeSetEvent(1, 5, mbClickCB, (DWORD_PTR)mb, TIME_ONESHOT);
	}
#endif
}

static void centreCursor (TVLCPLAYER *vp, int x, int y)
{
	int resetCur = 0;
	  	
	if (x < 15){
	 	x = 15;
	 	resetCur = 1;
	 }else{
		int w = GetSystemMetrics(SM_CXSCREEN);
		if (x > w-15) x = w-15;
		resetCur = 1;
	}
	if (y < 15){
		y = 15;
		resetCur = 1;
	}else{
	    int h = GetSystemMetrics(SM_CYSCREEN);
	    if (y > h-15) y = h-15;
	    resetCur = 1;
	}
	if (resetCur)
		SetCursorPos(x, y);
		
  	vp->gui.x = x;
  	vp->gui.y = y;
	vp->gui.dx = vp->ctx.working->width/2;
	vp->gui.dy = vp->ctx.working->height/2;
  	vp->gui.hooked = 1;
  	vp->gui.RBState = 1;
}

void mouseMove (TVLCPLAYER *vp, int x, int y)
{

	if (vp->gui.hooked != 1)
  		centreCursor(vp, x, y);

	GetCursorPos(&vp->gui.pt);
	//dbwprintf(vp, L"%i %i %i %i", vp->gui.pt.y, vp->gui.y, y, vp->gui.y - y);

  	if (vp->gui.pt.x != vp->gui.x || vp->gui.pt.y != vp->gui.y){
  		vp->gui.x = vp->gui.pt.x;
  		x = vp->gui.x;
  		vp->gui.y = vp->gui.pt.y;
  		y = vp->gui.y;
  	}

  	vp->gui.dx -= (vp->gui.x - x);
	vp->gui.dy -= (vp->gui.y - y);

	if (vp->gui.dx > vp->ctx.working->width-1)
		vp->gui.dx = vp->ctx.working->width-1;		
	else if (vp->gui.dx < 0)
		vp->gui.dx = 0;
		
	if (vp->gui.dy > vp->ctx.working->height-1)
		vp->gui.dy = vp->ctx.working->height-1;
	else if (vp->gui.dy < 0)
		vp->gui.dy = 0;

  	if (vp->gui.LBState){	// enable mouse drag
  		TTOUCHCOORD pos;
  		pos.x = vp->gui.dx+MOFFSETX;
  		pos.y = vp->gui.dy+MOFFSETY;
  		pos.pen = 1;
  		pos.time = getTime(vp);
  		pos.dt = 5;
  		pos.z1 = 100;
  		pos.z2 = 100;
  		pos.pressure = 100;
  		vp->gui.LBState = 2;
	 	touchSimulate(&pos, TOUCH_VINPUT|1, vp);
  	}

  	if (getTime(vp)-vp->fTime > (1.0/(float)MINUIFPS)*1000.0)
  		SetEvent(vp->hUpdateEvent);
}


void mouseLBDown (TVLCPLAYER *vp, int x, int y)
{

	TTOUCHCOORD pos;
	pos.x = vp->gui.dx+MOFFSETX;
	pos.y = vp->gui.dy+MOFFSETY;
	pos.pen = 0;
	pos.time = getTime(vp);
	pos.dt = 1000;
	pos.z1 = 100;
	pos.z2 = 100;
	pos.pressure = 100;
	vp->gui.LBState = 1;
	touchSimulate(&pos, TOUCH_VINPUT|0, vp);

	pos.pen = 1;	// generate a finger up response
	pos.dt = 5;
	touchSimulate(&pos, TOUCH_VINPUT|3, vp);

	if (getTime(vp)-vp->fTime > (1.0/(float)MINUIFPS)*1000.0)
		SetEvent(vp->hUpdateEvent);
}

void mouseLBUp (TVLCPLAYER *vp, int x, int y)
{
	if (vp->gui.LBState == 2){
		TTOUCHCOORD pos;
	  	pos.x = vp->gui.dx+MOFFSETX;
	  	pos.y = vp->gui.dy+MOFFSETY;
	  	pos.pen = 1;
	  	pos.time = getTime(vp);
	  	pos.dt = 5;
	  	pos.z1 = 100;
	  	pos.z2 = 100;
	  	pos.pressure = 100;
		touchSimulate(&pos, TOUCH_VINPUT|3, vp);
	}
	vp->gui.LBState = 0;
}

void mouseMBDown (TVLCPLAYER *vp, int x, int y)
{
	captureMouse(vp, 0);
	mHookUninstall();
	vp->gui.MBState = 1;
}

void mouseMBUp (TVLCPLAYER *vp, int x, int y)
{
	vp->gui.MBState = 0;
}

void mouseRBDown (TVLCPLAYER *vp, int x, int y)
{
	vp->gui.RBState = 1;
	centreCursor(vp, x, y);
}

void mouseRBUp (TVLCPLAYER *vp, int x, int y)
{
	vp->gui.RBState = 0;
}	

int hookCB (TVLCPLAYER *vp, int msg1, int msg2, int msg3)
{
	//dbprintf(vp, "%i %i %i\n", msg1, msg2, msg3);
	if (vp && !vp->applState) return 0;
	
	if (msg1 == WM_MOUSEMOVE){
		mouseMove(vp, msg2, msg3);
		
	}else if (msg1 == WM_LBUTTONDOWN){
		mouseLBDown(vp, msg2, msg3);
		
	}else if (msg1 == WM_LBUTTONUP){
		mouseLBUp(vp, msg2, msg3);
		
	}else if (msg1 == WM_MBUTTONDOWN){
		mouseMBDown(vp, msg2, msg3);
		
	}else if (msg1 == WM_MBUTTONUP){
		mouseMBUp(vp, msg2, msg3);
		
	}else if (msg1 == WM_RBUTTONDOWN){
		mouseRBDown(vp, msg2, msg3);
		
	}else if (msg1 == WM_RBUTTONUP){
		mouseRBUp(vp, msg2, msg3);
	//}else{
	//	return 0;
	}
	return 1;
}
	
static int processWindowMessages (TVLCPLAYER *vp)
{
	MSG msg;
	int ret = 0;
	
	if ((ret=GetMessage(&msg, vp->gui.hMsgWin, 0, 0)) > 0){
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return ret;
}


LRESULT CALLBACK WindowProcedure (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	TVLCPLAYER *vp = (TVLCPLAYER*)lParam;

	switch (message){
	  case WM_TOUCHIN:{
	  	TTOUCHINPOS *tin = (TTOUCHINPOS*)wParam;
	  	if (tin){
	  		if (renderLock(vp)){
	  			if (vp->applState)
					touchDispatch(&tin->pos, tin->flags, tin->ptr);
				renderUnlock(vp);
			}
			my_free(tin);
		}
		break;
	  }


	  case WM_HOTKEY:{
	  	vp = (TVLCPLAYER*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	  	if (vp == NULL) return 0;

	  	switch (lParam>>16){
		  case VK_MEDIA_PREV_TRACK:
		  	timerSet(vp, TIMER_PREVTRACK, 0);
		    break;

		  case VK_MEDIA_NEXT_TRACK:
		  	timerSet(vp, TIMER_NEXTTRACK, 0);
	  	    break;

	  	  case VK_MEDIA_PLAY_PAUSE:
	  	  	timerSet(vp, TIMER_PLAYPAUSE, 0);
	  	    break;

	  	  case VK_MEDIA_STOP:
	  	  	timerSet(vp, TIMER_STOP, 0);
	  	  	break;

		  case VK_VOLUME_UP:
		  	timerSet(vp, TIMER_VOLUP, 0);
		  	break;

		  case VK_VOLUME_DOWN:
		  	timerSet(vp, TIMER_VOLDN, 0);
		  	break;

		  case 'S':
		  	editboxCmd_snapshot(NULL, 0, vp, 0, 0);
		  	break;
		  	
		  case 13:
	  	  case 'A':
	  	  case 'Q':
	  	  	if (!vp->gui.awake)
				startRefreshTicker(vp, vp->gui.targetFPS);
	  	  	setAwake(vp);
	  	  					
	  		if (!mHookGetState()){
	  			mHookInstall(hwnd, vp);
		  		captureMouse(vp, 1);
	  		}else{
	  			captureMouse(vp, 0);
		  		mHookUninstall();
			}
			break;

	  	  case 'L':
	  	  case 'P':
	  	  	if (!vp->gui.awake)
				startRefreshTicker(vp, vp->gui.targetFPS);
	  	  	setAwake(vp);
	  	  				
	  		if (!kHookGetState()){
	  			setPageSec(vp, -1);
	  			if (kHookInstall(hwnd, vp)){
	  				kHookOn();
	  				if (!kHookGetState())
	  					kHookUninstall();
	  			}
	  		}else{
		  		kHookUninstall();
			}
			break;
		}
	  	return 1;
	   }

	  case WM_CHAR:{
	  	if (getPageSec(vp) == PAGE_TETRIS){
	  		//hijack the hooked kb input for tetris control
	  		tetrisInputProc(vp, getPagePtr(vp, PAGE_TETRIS), wParam);
	  		return 0;
	  	}

	  	int ret = editBoxInputProc(&vp->input, vp->gui.hMsgWin, wParam, 0);
	  	if (ret == 2){
			wchar_t *txt = editboxGetString(&vp->input);
			if (txt){
				const int ilen = wcslen(txt);
				if (ilen){
					addWorkingBuffer(&vp->input);
					nextHistoryBuffer(&vp->input);
					clearWorkingBuffer(&vp->input);
					
					kHookOff();
					kHookUninstall();
					lSleep(20);
					exitboxProcessString(&vp->input, txt, ilen, vp);
					kHookInstall(hwnd, vp);
					kHookOn();
				}
				my_free(txt);
			}
	  	}
	  	break;
	  }
	  case WM_KEYDOWN:
	  	if (wParam == 27){
	  		kHookOff();
		  	kHookUninstall();
    	}else if (wParam >= VK_PRIOR && wParam <= VK_DELETE){
   			editBoxInputProc(&vp->input, vp->gui.hMsgWin, wParam|0x1000, 0);
   		}		
	  	return 0;

	  case WM_QUIT: 
		DestroyWindow(hwnd);
		return 0;

	  case WM_CLOSE: 
		return 0;

 	  case WM_DESTROY: 
		PostQuitMessage(0);
	    return 0;
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}

static HANDLE initGUI (TVLCPLAYER *vp)
{
	const char *szClassName = "vpmsgwindow";
	HANDLE hThisInstance = GetModuleHandle(0);
    WNDCLASSEX wincl;
    wincl.cbSize = sizeof (WNDCLASSEX);
    wincl.hInstance = hThisInstance;
    wincl.lpszClassName = szClassName;
    wincl.lpfnWndProc = WindowProcedure;
    wincl.hIcon = NULL;
    wincl.hIconSm =NULL;
    wincl.hCursor = NULL;
    wincl.hbrBackground = NULL;
    wincl.style = CS_DBLCLKS;
    wincl.lpszMenuName = NULL;
    wincl.cbClsExtra = 0;
    wincl.cbWndExtra = 0;
    if (!RegisterClassEx (&wincl))
        return NULL;

    HWND hMsgWin = CreateWindowEx(0, szClassName, szClassName, WS_OVERLAPPEDWINDOW,CW_USEDEFAULT,CW_USEDEFAULT,\
	  0, 0, HWND_DESKTOP, NULL, hThisInstance, NULL);

    ShowWindow(hMsgWin, SW_HIDE);
    SetWindowLongPtr(hMsgWin, GWLP_USERDATA, (LONG_PTR)vp);
    return hMsgWin;
}

int captureMouse (TVLCPLAYER *vp, int state)
{
	if (state){
		GetCursorPos(&vp->gui.pt);
		mHookSetCB(vp->gui.hMsgWin, (void*)hookCB);
		mHookOn();
		
		vp->gui.dx = vp->ctx.working->width/2;
 		vp->gui.dy = vp->ctx.working->height/2;
		vp->gui.hooked = -1;
		if (!vp->gui.awake)
			startRefreshTicker(vp, vp->gui.targetFPS);
		setAwake(vp);
	}else{
		vp->gui.hooked = 0;
		mHookOff();
		SetCursorPos(vp->gui.pt.x, vp->gui.pt.y);
	}
	return mHookGetState();
}

int captureMouseToggleState (TVLCPLAYER *vp)
{
	return captureMouse(vp, (vp->gui.hooked == 0));
}

unsigned int __stdcall winMessageThread (void *ptr)
{
	TVLCPLAYER *vp = (TVLCPLAYER*)ptr;
	ATOM kid[10];
	memset(kid, 0, sizeof(kid));
	
#if 0
	SetPriorityClass(GetCurrentProcess(), ABOVE_NORMAL_PRIORITY_CLASS);
#else
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
#endif

	if ((vp->gui.hMsgWin=initGUI(vp))){
		kid[0] = GlobalAddAtom("vlcshk_left");
		kid[1] = GlobalAddAtom("vlcshk_right");
		kid[2] = GlobalAddAtom("vlcshk_prevtrack");
		kid[3] = GlobalAddAtom("vlcshk_nexttrack");
		kid[4] = GlobalAddAtom("vlcshk_playpause");
		kid[5] = GlobalAddAtom("vlcshk_stop");
		kid[6] = GlobalAddAtom("vlcshk_volumeup");
		kid[7] = GlobalAddAtom("vlcshk_volumedn");
		kid[8] = GlobalAddAtom("vlcshk_mhookenter");
		kid[9] = GlobalAddAtom("vlcshk_ss");
		
		// set mouse hook control hotkeys
		RegisterHotKey(vp->gui.hMsgWin, kid[8], MOD_CONTROL|MOD_SHIFT, 13);
		if (!RegisterHotKey(vp->gui.hMsgWin, kid[0], MOD_CONTROL|MOD_SHIFT, 'A'))
			RegisterHotKey(vp->gui.hMsgWin, kid[0], MOD_CONTROL|MOD_SHIFT, 'Q');
			
		// set keyboard hook control hotkeys
		if (!RegisterHotKey(vp->gui.hMsgWin, kid[1], MOD_CONTROL|MOD_SHIFT, 'L'))
			RegisterHotKey(vp->gui.hMsgWin, kid[1], MOD_CONTROL|MOD_SHIFT, 'P');

		// screen shot		
		RegisterHotKey(vp->gui.hMsgWin, kid[9], MOD_CONTROL|MOD_ALT, 'S');
	
		RegisterHotKey(vp->gui.hMsgWin, kid[2], MOD_CONTROL, VK_MEDIA_PREV_TRACK);
		RegisterHotKey(vp->gui.hMsgWin, kid[3], MOD_CONTROL, VK_MEDIA_NEXT_TRACK);
		RegisterHotKey(vp->gui.hMsgWin, kid[4], MOD_CONTROL, VK_MEDIA_PLAY_PAUSE);
		RegisterHotKey(vp->gui.hMsgWin, kid[5], MOD_CONTROL, VK_MEDIA_STOP);
		RegisterHotKey(vp->gui.hMsgWin, kid[6], MOD_CONTROL, VK_VOLUME_UP);
		RegisterHotKey(vp->gui.hMsgWin, kid[7], MOD_CONTROL, VK_VOLUME_DOWN);

		while (vp->applState)
			processWindowMessages(vp);
			
		for (int i = 0; i < (sizeof(kid)/sizeof(ATOM)); i++){
			if (kid[i]){
				UnregisterHotKey(vp->gui.hMsgWin, kid[i]);
				GlobalDeleteAtom(kid[i]);
			}
		}
		if (mHookGetState())
			mHookUninstall();
		if (kHookGetState())
			kHookUninstall();
	}
	_endthreadex(1);
	return 1;
}

int startMouseCapture (TVLCPLAYER *vp)
{
	vp->gui.hWinMsgThread = _beginthreadex(NULL, 0, winMessageThread, vp, 0, &vp->gui.winMsgThreadID);
	return (int)vp->gui.winMsgThreadID;
}

void endMouseCapture (TVLCPLAYER *vp)
{
	PostMessage(vp->gui.hMsgWin, WM_QUIT,0,0); // wakeup the message thread
	WaitForSingleObject((HANDLE)vp->gui.hWinMsgThread, INFINITE);
	CloseHandle((HANDLE)vp->gui.hWinMsgThread);
}

#endif
