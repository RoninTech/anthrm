
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
//  GNU LIBRARY GENERAL PUBLIC LICENSE for more details.

// based on TinyPTC by Gaffer, www.gaffer.org/tinyptc


#include "mylcd.h"

#if ((__BUILD_DDRAW__) && (__BUILD_WIN32__))


#include <signal.h>
#include "../device.h"
#include "../display.h"
#include "../memory.h"
#include "../convert.h"
#include "converter.h"
#include "dd.h"

#include "../sync.h"
#include "../misc.h"

static HMODULE library = NULL;

#ifdef __MDD_ALLOW_CLOSE__
static int windowTotal;
#endif


#ifdef _WIN64
#define USERDATA GWLP_USERDATA
#else
#define USERDATA GWL_USERDATA
#endif


typedef HRESULT (WINAPI *DIRECTDRAWCREATE) (GUID FAR *lpGUID, LPDIRECTDRAW FAR *lplpDD, IUnknown FAR *pUnkOuter);
static DIRECTDRAWCREATE _DirectDrawCreate_;


static void paint_primary (TMYLCDDDRAW *mddraw)
{
	if (!mddraw)
		return;
	if (!mddraw->status)
		return;

    RECT source;
    RECT destination;
    POINT point;

    if (mddraw->lpDDS){
        source.left = 0;
        source.top = 0;
        source.right = mddraw->des.x;
        source.bottom = mddraw->des.y;
        point.x = 0;
        point.y = 0;

        ClientToScreen(mddraw->wnd, &point);
        GetClientRect(mddraw->wnd, &destination);

        // offset destination rectangle
        destination.left += point.x;
        destination.top += point.y;
        destination.right += point.x;
        destination.bottom += point.y;

        // blt secondary to primary surface
        IDirectDrawSurface_Blt(mddraw->lpDDS, &destination, mddraw->lpDDS_secondary, &source, DDBLT_WAIT,0);
    }
}

static LRESULT CALLBACK WndProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    int result = 0;

    switch (message){
        case WM_PAINT:
        {
        	TMYLCDDDRAW *mddraw = (TMYLCDDDRAW*)GetWindowLongPtr(hWnd, USERDATA);
        	if (mddraw)
            	paint_primary(mddraw);
            return DefWindowProc(hWnd,message,wParam,lParam);
        }

		#ifdef __MDD_RESIZE_WINDOW__
		#ifdef __MDD_SYSTEM_MENU__

		 case WM_SYSCOMMAND:
		 {
			if (wParam == MU_DDSTATE_DISABLE){
				TMYLCDDDRAW *mddraw = (TMYLCDDDRAW *)GetWindowLongPtr(hWnd, USERDATA);
        		if (mddraw){
					pauseDisplay(mddraw->drv->dd->hw, driverNameToID(mddraw->drv->dd->hw, (char *)mddraw->drv->dd->name, LDRV_DISPLAY));
					mddraw->menuinfo.fType = MFT_STRING;
					mddraw->menuinfo.wID = MU_DDSTATE_ENABLE;
      				mddraw->menuinfo.dwTypeData = "Engage";
					SetMenuItemInfo(mddraw->menu, mddraw->mcount+3, 1, &mddraw->menuinfo);
				}
			}else if (wParam == MU_DDSTATE_ENABLE){
				TMYLCDDDRAW *mddraw = (TMYLCDDDRAW *)GetWindowLongPtr(hWnd, USERDATA);
        		if (mddraw){
					resumeDisplay(mddraw->drv->dd->hw, driverNameToID(mddraw->drv->dd->hw, (char *)mddraw->drv->dd->name, LDRV_DISPLAY));
					mddraw->menuinfo.fType = MFT_STRING;
					mddraw->menuinfo.wID = MU_DDSTATE_DISABLE;
      				mddraw->menuinfo.dwTypeData = "Disengage";
					SetMenuItemInfo(mddraw->menu, mddraw->mcount+3, 1, &mddraw->menuinfo);
				}
			}else if ((wParam&0xFFFFFFF0) == SC_ZOOM_MSK){
				TMYLCDDDRAW *mddraw = (TMYLCDDDRAW *)GetWindowLongPtr(hWnd, USERDATA);
        		if (mddraw){
					float zoom = wParam&0x7;
					if ((int)zoom == (SC_ZOOM_50 & ~SC_ZOOM_MSK))
						zoom = 0.5;
					int x = (GetSystemMetrics(SM_CXSCREEN) - mddraw->original_window_width*zoom) / 2;
					int y = (GetSystemMetrics(SM_CYSCREEN) - mddraw->original_window_height*zoom) / 2 ;
					SetWindowPos(hWnd, NULL, x, y,mddraw->original_window_width*zoom, mddraw->original_window_height*zoom, SWP_NOZORDER);
				}
            }else{
            	return DefWindowProc(hWnd, message, wParam, lParam);
            }
		 }
		#endif
		#endif

		#ifdef __MDD_CLOSE_ON_ESCAPE__
          case WM_KEYDOWN:
            // close on escape key
            if ((wParam&0xFF) != 27)
            	break;
		#endif

		case WM_QUIT:
			raise(SIGINT);
			break;

        case WM_CLOSE:
        {
        	TMYLCDDDRAW *mddraw = (TMYLCDDDRAW *)GetWindowLongPtr(hWnd, USERDATA);
        	if (mddraw)
        		mddraw->status = 0;

            #ifdef __MDD_ALLOW_CLOSE__
            	if (--windowTotal < 1)
            		SendMessage(hWnd, WM_QUIT, 0, (LPARAM)mddraw);
            #endif
        }

        default:
            result = DefWindowProc(hWnd,message,wParam,lParam);
    }
    return result;
}


int initDDraw ()
{
   	library = (HMODULE)LoadLibrary("ddraw.dll");
   	if (library){
   		_DirectDrawCreate_ = (DIRECTDRAWCREATE)GetProcAddress(library,"DirectDrawCreate");
   		if (_DirectDrawCreate_){
   			return 1;
   		}else{
   			FreeLibrary(library);
   			mylog("libmylcd: DirectDrawCreate() not found\n");
   		}
   	}else{
   		mylog("libmylcd: ddraw.dll not found\n");
   	}
    library = NULL;
    return 0;
}

int registerWC ()
{
	WNDCLASS wc;

    wc.style = CS_VREDRAW | CS_HREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
	#ifdef __MDD_ICON__
      wc.hInstance = GetModuleHandle(0);
      wc.hIcon = LoadIcon(wc.hInstance,__MDD_ICON__);
	#else
      wc.hInstance = 0;
      wc.hIcon = 0;
	#endif
    wc.hCursor = LoadCursor(0,IDC_ARROW);
    wc.hbrBackground = 0;
    wc.lpszMenuName = 0;
    wc.lpszClassName = MYLCDDDRAWWC;
    RegisterClass(&wc);

	return initDDraw();
}


int directDraw_open (TMYLCDDDRAW *mddraw, char *title, int width, int height)
{
    int x, y;
    RECT rect;
    DDPIXELFORMAT format;
    DDSURFACEDESC descriptor;

    mddraw->des.x = width;
    rect.right = width;
    mddraw->des.y = height;
    rect.bottom = height;
    rect.left = 0;
    rect.top = 0;
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, 0);

    rect.right -= rect.left;
    rect.bottom -= rect.top;
	mddraw->original_window_width = rect.right;
	mddraw->original_window_height = rect.bottom;

    // center window
    x = (GetSystemMetrics(SM_CXSCREEN) - rect.right) >> 1;
    y = (GetSystemMetrics(SM_CYSCREEN) - rect.bottom) >> 1;

	#ifdef __MDD_ALLOW_CLOSE__
	  int flag = WS_OVERLAPPEDWINDOW;
	#else
	  int flag = 0;
	#endif

	#ifdef __MDD_RESIZE_WINDOW__
      mddraw->wnd = CreateWindow(MYLCDDDRAWWC, title, flag, x, y, rect.right,rect.bottom, 0, 0, 0, 0);
	#else
      mddraw->wnd = CreateWindow(MYLCDDDRAWWC, title, flag & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME, x, y,rect.right,rect.bottom,0,0,0,0);
	#endif

	if (mddraw->wnd == NULL)
		return 0;

	#ifdef __MDD_ALLOW_CLOSE__
	  windowTotal++;
	#endif

	SetWindowLongPtr(mddraw->wnd, USERDATA, (intptr_t)mddraw);
    ShowWindow(mddraw->wnd, SW_NORMAL);

	#ifdef __MDD_RESIZE_WINDOW__
	#ifdef __MDD_SYSTEM_MENU__
      mddraw->menu = GetSystemMenu(mddraw->wnd, FALSE);
	  mddraw->mcount = GetMenuItemCount(mddraw->menu);
      mddraw->menuinfo.cbSize = sizeof(MENUITEMINFO);
	  mddraw->menuinfo.fMask = MIIM_TYPE|MIIM_ID|MIIM_STATE;
	  mddraw->menuinfo.fState = MFS_ENABLED;
	  mddraw->menuinfo.fType = MFT_STRING;

      mddraw->menuinfo.wID = SC_ZOOM_50;
      mddraw->menuinfo.dwTypeData = "Zoom 50%";
      InsertMenuItem(mddraw->menu, mddraw->mcount-1, 1, &mddraw->menuinfo);

      mddraw->menuinfo.wID = SC_ZOOM_100;
      mddraw->menuinfo.dwTypeData = "Zoom 100%";
      InsertMenuItem(mddraw->menu, mddraw->mcount, 1, &mddraw->menuinfo);

      mddraw->menuinfo.wID = SC_ZOOM_200;
      mddraw->menuinfo.dwTypeData = "Zoom 200%";
      InsertMenuItem(mddraw->menu, mddraw->mcount+1, 1, &mddraw->menuinfo);

      mddraw->menuinfo.wID = SC_ZOOM_400;
      mddraw->menuinfo.dwTypeData = "Zoom 400%";
      InsertMenuItem(mddraw->menu, mddraw->mcount+2, 1, &mddraw->menuinfo);

	#if 0
      mddraw->menuinfo.wID = MU_DDSTATE_DISABLE;
      mddraw->menuinfo.dwTypeData = "Disengage";
      InsertMenuItem(mddraw->menu, mddraw->mcount+3, 1, &mddraw->menuinfo);
    #endif

      mddraw->menuinfo.fType = MFT_SEPARATOR;
      InsertMenuItem(mddraw->menu, mddraw->mcount+3, 1, &mddraw->menuinfo);

	#endif
	#endif

    // create directdraw interface
    if (FAILED(_DirectDrawCreate_(0, &mddraw->lpDD, 0)))
    	return 0;

    // enter cooperative mode
    if (FAILED(IDirectDraw_SetCooperativeLevel(mddraw->lpDD, mddraw->wnd, DDSCL_NORMAL)))
    	return 0;

    // primary with no back buffers
    descriptor.dwSize  = sizeof(descriptor);
    descriptor.dwFlags = DDSD_CAPS;
    descriptor.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_VIDEOMEMORY;
    if (FAILED(IDirectDraw_CreateSurface(mddraw->lpDD,&descriptor,&mddraw->lpDDS,0)))
    	return 0;

    // create secondary surface
    descriptor.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
    descriptor.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
    descriptor.dwWidth = width;
    descriptor.dwHeight = height;

    if (FAILED(IDirectDraw_CreateSurface(mddraw->lpDD, &descriptor, &mddraw->lpDDS_secondary, 0)))
    	return 0;

	#ifdef __MDD_CLIPPER__
     // create clipper
     if (FAILED(IDirectDraw_CreateClipper(mddraw->lpDD, 0, &mddraw->lpDDC, 0)))
    	return 0;

     // set clipper to window
     if (FAILED(IDirectDrawClipper_SetHWnd(mddraw->lpDDC, 0, mddraw->wnd)))
     	return 0;

     // attach clipper object to primary surface
     if (FAILED(IDirectDrawSurface_SetClipper(mddraw->lpDDS, mddraw->lpDDC)))
     	return 0;
	#endif

    // set back to secondary
    mddraw->lpDDS_back = mddraw->lpDDS_secondary;

    // get pixel format
    format.dwSize = sizeof(format);
    if (FAILED(IDirectDrawSurface_GetPixelFormat(mddraw->lpDDS,&format)))
    	return 0;

    // check format is direct colour
    if (!(format.dwFlags & DDPF_RGB))
    	return 0;

    // request converter function
#if defined(_WIN64)		/* please fix me */ 
	mddraw->convert = request_converter(format.DUMMYUNIONNAME1.dwRGBBitCount, format.DUMMYUNIONNAME2.dwRBitMask, format.DUMMYUNIONNAME3.dwGBitMask, format.DUMMYUNIONNAME4.dwBBitMask);
#elif defined(NONAMELESSUNION)
    mddraw->convert = request_converter(format.u1.dwRGBBitCount, format.u2.dwRBitMask, format.u3.dwGBitMask, format.u4.dwBBitMask);
#else
	mddraw->convert = request_converter(format.dwRGBBitCount, format.dwRBitMask, format.dwGBitMask, format.dwBBitMask);
#endif
    if (!mddraw->convert)
    	return 0;

	mddraw->status = 1;
    return (int)mddraw->wnd;
}


int directDraw_update (TMYLCDDDRAW *mddraw, void *buffer, size_t bufferLength)
{

    DDSURFACEDESC descriptor;
    MSG message;

 	if (!mddraw->status)
		return 0;

	// restore surfaces
	IDirectDrawSurface_Restore(mddraw->lpDDS);
	IDirectDrawSurface_Restore(mddraw->lpDDS_secondary);

     // lock back surface
	descriptor.dwSize = sizeof(descriptor);
	if (FAILED(IDirectDrawSurface_Lock(mddraw->lpDDS_back,0,&descriptor,DDLOCK_WAIT,0)))
		return 0;

	uint8_t * restrict src = (uint8_t*)buffer;
	uint8_t * restrict dst = (uint8_t*)descriptor.lpSurface;

#if 0
	if (!(mddraw->des.x&63)){
		l_memcpy(dst, src, bufferLength);
	}else{
		const int src_pitch = mddraw->des.x << 2;
		
#if defined(_WIN64) /*fix me*/
		const int dst_pitch = descriptor.DUMMYUNIONNAME1.lPitch;
#elif defined(NONAMELESSUNION)
		const int dst_pitch = descriptor.u1.lPitch;
#else
		const int dst_pitch = descriptor.lPitch;
#endif
    	int y;
		for (y=0; y<mddraw->des.y; y++){
			mddraw->convert(src, dst, mddraw->des.x);
			src += src_pitch;
			dst += dst_pitch;
		}
	}
#else
	l_memcpy(dst, src, bufferLength);
#endif

	// unlock back surface
	IDirectDrawSurface_Unlock(mddraw->lpDDS_back, descriptor.lpSurface);
	paint_primary(mddraw);

    while(PeekMessage(&message, mddraw->wnd, 0, 0, PM_REMOVE)){
        TranslateMessage(&message);
        DispatchMessage(&message);
    }

    return 1;
}

void directDraw_close (TMYLCDDDRAW *mddraw)
{
	if (mddraw == NULL)
		return;

	if (mddraw->menu){
		DestroyMenu(mddraw->menu);
		mddraw->menu = NULL;
	}

    // check secondary
    if (mddraw->lpDDS_secondary){
        // release secondary
        IDirectDrawSurface_Release(mddraw->lpDDS_secondary);
        mddraw->lpDDS_secondary = NULL;
    }

    // check
    if (mddraw->lpDDS){
        // release primary
        IDirectDrawSurface_Release(mddraw->lpDDS);
        mddraw->lpDDS = NULL;
    }

    // check
    if (mddraw->lpDD){
        // leave display mode
        IDirectDraw_RestoreDisplayMode(mddraw->lpDD);

        // leave exclusive mode
        IDirectDraw_SetCooperativeLevel(mddraw->lpDD, mddraw->wnd, DDSCL_NORMAL);

        // free direct draw
        IDirectDraw_Release(mddraw->lpDD);
        mddraw->lpDD = NULL;
    }
    DestroyWindow(mddraw->wnd);
}

void releaseDDLib ()
{
   if (library)
        FreeLibrary(library);
	library = NULL;
	#ifdef __MDD_ALLOW_CLOSE__
	  windowTotal = 0;
	#endif
}

#endif
