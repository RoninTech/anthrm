// mouse hooking
//
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




__declspec (dllexport) BOOL mHookInstall (HWND hWnd, void *ptr);
__declspec (dllexport) BOOL mHookUninstall ();
__declspec (dllexport) void mHookOn ();
__declspec (dllexport) void mHookOff ();
__declspec (dllexport) int mHookGetState ();
__declspec (dllexport) BOOL mHookSetCB (HWND hWnd, void *ptr);

__declspec (dllexport) BOOL kHookInstall (HWND hWnd, void *ptr);
__declspec (dllexport) BOOL kHookUninstall ();
__declspec (dllexport) void kHookOn ();
__declspec (dllexport) void kHookOff ();
__declspec (dllexport) int kHookGetState ();


