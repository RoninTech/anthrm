
// myLCD
// An LCD framebuffer library
// Michael McElligott
// okio@users.sourceforge.net

//  Copyright (c) 2005-2010  Michael McElligott
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


#include <windows.h>
#include "hook.h"

LRESULT CALLBACK mHookProc (int nCode, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK kHookProc (int nCode, WPARAM wParam, LPARAM lParam);

typedef int (*hookCB) (void *ptr, int msg1, int msg2, int msg3);

typedef struct{
	HANDLE hNotifyWnd;
	HANDLE hHook;
	HANDLE hDllInstance;
	void *lptr;
	hookCB cbPtr;
	int hookState;
}THOOK;

static THOOK mhook;
static THOOK khook;


BOOL WINAPI DllMain (HINSTANCE hInstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	mhook.hDllInstance = hInstDLL;
	khook.hDllInstance = hInstDLL;
	DisableThreadLibraryCalls(hInstDLL);
	return 1;
}

__declspec (dllexport) BOOL mHookSetCB (HWND hWnd, void *userPtr)
{
	mhook.cbPtr = (hookCB)userPtr;
}

__declspec (dllexport) BOOL mHookInstall (HWND hWnd, void *userPtr)
{
	if (hWnd == NULL)
		return FALSE;

	if (mhook.hNotifyWnd != NULL)
		return FALSE;

	mhook.hHook = SetWindowsHookEx(WH_MOUSE_LL, (HOOKPROC)mHookProc, mhook.hDllInstance, 0);
	if (mhook.hHook != NULL){
		mhook.hookState = 0;
		mhook.lptr = userPtr;
		mhook.cbPtr = NULL;
		mhook.hNotifyWnd = hWnd;
		return TRUE;
	}else{
		return FALSE;
	}
}

__declspec (dllexport) BOOL mHookUninstall ()
{
	BOOL unHooked = TRUE;
	if (mhook.hNotifyWnd != NULL && mhook.hHook != NULL)
		unHooked = UnhookWindowsHookEx(mhook.hHook);

	mhook.cbPtr = NULL;
	mhook.hHook = NULL;
	mhook.hNotifyWnd = NULL;
	mhook.hookState = 0;
	return unHooked;
}

__declspec (dllexport) int mHookGetState ()
{
	return (mhook.hookState && mhook.hHook);
}

__declspec (dllexport) void mHookOn ()
{
	mhook.hookState = 1;
}

__declspec (dllexport) void mHookOff ()
{
	mhook.hookState = 0;
}

LRESULT CALLBACK mHookProc (int nCode, WPARAM wParam, LPARAM lParam)
{	
	if (nCode == HC_ACTION && mhook.hookState){
		MSLLHOOKSTRUCT *mh = (MSLLHOOKSTRUCT*)lParam;

		int ret = 1;
		if (mhook.cbPtr){
			ret = mhook.cbPtr(mhook.lptr, (int)wParam, (int)mh->pt.x, (int)mh->pt.y);
			return ret;
		}
	}
	return CallNextHookEx(mhook.hHook, nCode, wParam, lParam);
}


/*
####################################################################################################
####################################################################################################
####################################################################################################
*/


__declspec (dllexport) BOOL kHookInstall (HWND hWnd, void *userPtr)
{
	if (hWnd == NULL)
		return FALSE;

	if (khook.hNotifyWnd != NULL)
		return FALSE;

	khook.hHook = SetWindowsHookEx(WH_KEYBOARD_LL, (HOOKPROC)kHookProc, khook.hDllInstance, 0);
	if (khook.hHook != NULL){
		khook.hookState = 0;
		khook.lptr = userPtr;
		khook.cbPtr = NULL;
		khook.hNotifyWnd = hWnd;
		return TRUE;
	}else{
		return FALSE;
	}
}

__declspec (dllexport) BOOL kHookUninstall ()
{
	BOOL unHooked = TRUE;
	if (khook.hNotifyWnd != NULL && khook.hHook != NULL)
		unHooked = UnhookWindowsHookEx(khook.hHook);

	khook.hHook = NULL;
	khook.hNotifyWnd = NULL;
	khook.hookState = 0;
	return unHooked;
}

__declspec (dllexport) int kHookGetState ()
{
	return (khook.hookState && khook.hHook);
}

__declspec (dllexport) void kHookOn ()
{
	khook.hookState = 1;
}

__declspec (dllexport) void kHookOff ()
{
	khook.hookState = 0;
}

LRESULT CALLBACK kHookProc (int nCode, WPARAM wParam, LPARAM lParam)
{	
	if (nCode == HC_ACTION && khook.hookState){
		LPKBDLLHOOKSTRUCT kbhs = (LPKBDLLHOOKSTRUCT)lParam;

		if (kbhs->vkCode == VK_LSHIFT || kbhs->vkCode == VK_RSHIFT){
			PostMessage(khook.hNotifyWnd, wParam, kbhs->vkCode, (LPARAM)khook.lptr);
			
		}else if (kbhs->vkCode != VK_LWIN && kbhs->vkCode != VK_RWIN && kbhs->vkCode != VK_APPS && kbhs->vkCode != VK_LMENU && kbhs->vkCode != VK_RMENU){
			if (wParam == WM_KEYDOWN){
				PostMessage(khook.hNotifyWnd, WM_KEYDOWN, kbhs->vkCode, (LPARAM)khook.lptr);
				return 1;
			}else if (wParam == WM_KEYUP && kbhs->vkCode != VK_OEM_2 && kbhs->vkCode != VK_RCONTROL && kbhs->vkCode != VK_LCONTROL && kbhs->vkCode != VK_CONTROL){
				return 1;
			}
		}
	}
	return CallNextHookEx(khook.hHook, nCode, wParam, lParam);
}

