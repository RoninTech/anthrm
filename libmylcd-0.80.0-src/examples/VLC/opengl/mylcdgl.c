
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



#include "../common.h"
#include "mylcdgl.h"



static HWND hWnd;
static HDC hDC;
static HDC hdc;
static HGLRC hRC;
static HBITMAP m_hbmOld;
static HBITMAP m_hbmp;	
static int G_flags = 0;
static int currentWinState = 0;
static int once = 0;
static unsigned char *data = NULL;


static int padTo4 (const int width)
{
	if (width&3)
		return ((width>>2)<<2) + 4;
	else
		return width;
}

void getGLFrame1 (unsigned char *data, TFRAME *frame, const int glwidth, const int glheight)
{
    if (!data || !frame)
        return;

	int *dpixels;
	int r,g,b;
	int pad = padTo4(glwidth);
	if (pad)
		pad -= glwidth;
	
	glReadPixels(0, 0, glwidth, glheight, GL_RGB, GL_UNSIGNED_BYTE, data);

	const int w = frame->width;
	const int h = frame->height;
	const int pitch = frame->width;
	const int *pixels = (int*)frame->pixels;
	
	
	for (int y = h-1; y >= 0; y--){
		dpixels = (int*)&pixels[y*pitch];
		
#ifdef USE_MMX	
		__asm volatile ("prefetch 256(%0)\n" :: "r" (data) : "memory");
#endif

		for (int x = 0; x < w; x++){
			r = (*data++)<<16;
			g = (*data++)<<8;
			b = (*data++);
			*dpixels = 0xFF000000|r|g|b;
			dpixels++;
		}
		data += pad;
	}
}

void getGLFrame2 (unsigned char *data, TFRAME *frame, const int glwidth, const int glheight)
{
    if (!data || !frame)
        return;
/*
	int *dpixels;
	int r,g,b;
	int pad = padTo4(glwidth);
	if (pad)
		pad -= glwidth;
*/
	
	glReadPixels(0, 0, glwidth, glheight, GL_BGRA, GL_UNSIGNED_BYTE, frame->pixels);
/*	
	const int w = frame->width;
	const int h = frame->height;
	
	for (int y = h-1; y >= 0; y--){
		dpixels = lGetPixelAddress(frame, 0, y);
				
#ifdef USE_MMX	
		__asm volatile ("prefetch 256(%0)\n" :: "r" (data) : "memory");
#endif

		for (int x = 0; x < w; x++){
			r = (*data++)<<16;
			g = (*data++)<<8;
			b = (*data++);
			if (r|g|b)
				*dpixels = 0xFF000000|r|g|b;
			dpixels++;
		}
		data += pad;
	}
*/
}

int mylcdgl_init (TFRAME *frame, const int flags)
{
	G_flags = flags;

	if (G_flags&MYGL_CREATECONTEXT){
		openglInit(flags, frame->width, frame->height);
		mylcdgl_SetWindowTitle("vlcsopengl");
	}
		
    once = 2;
	data = (unsigned char *)my_malloc((padTo4(frame->width) * frame->height)*3);
	return (data != NULL);
}

void mylcdgl_SetWindowTitle (const char *title)
{
	if (title)
		SetWindowText(hWnd, title);
}

static int handleMessageQueue ()
{
	MSG msg;
	while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)){
		if (msg.message == WM_QUIT){
			return 0;
		}else{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
   	}
   	return 1;
}

static void ShowWindowSetState (int state)
{	
	if (state != currentWinState){
		if (!state){
			currentWinState = 0;
			ShowWindow(hWnd, SW_HIDE);
		}else{
			currentWinState = 1;
			ShowWindow(hWnd, SW_SHOW);
		}
	}
}

static int mylcdgl_render (TFRAME *des, const int flags)
{
    if (!(once&0x02)) return 0;

	if (flags&MYGL_FRONTPIXELSONLY)
		getGLFrame2(data, des, des->width, des->height);
	else
		getGLFrame1(data, des, des->width, des->height);

	if (flags&MYGL_CREATECONTEXT){
		if (flags&MYGL_SHOWWINDOW)
			ShowWindowSetState(1);
		if (flags&MYGL_MESSAGEQUEUE)
			handleMessageQueue();
		if (currentWinState)
			SwapBuffers(hDC);
	}
	return 1;
}

int mylcdgl_draw (TFRAME *frame, const int flags)
{
	return mylcdgl_render(frame, flags);
}

void mylcdgl_shutdown ()
{
	if (!(once&0x02)) return;
	once = 0;
	
	if (G_flags&MYGL_CREATECONTEXT){
		DisableOpenGL(hWnd, hDC, hRC);
		if (m_hbmOld)
			SelectObject(hdc, m_hbmOld);
		if (m_hbmp)
			DeleteObject(m_hbmp);
		if (hdc)
			DeleteDC(hdc);
		DestroyWindow(hWnd);
	}
	if (data)
		my_free(data);
}

#if DRAW_TO_BITMAP
void *m_pBits;
#endif

void EnableOpenGL (HDC hdc, HDC * hDC, HGLRC * hRC)
{
	static PIXELFORMATDESCRIPTOR pfd;
	int format;

	
#if DRAW_TO_BITMAP
	*hDC = hdc;
	pfd.dwFlags = PFD_DRAW_TO_BITMAP | PFD_SUPPORT_OPENGL | PFD_SUPPORT_GDI;
#else
	*hDC = GetDC((HWND)hdc);
	pfd.dwFlags = PFD_DRAW_TO_WINDOW| PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
#endif
	
	// set the pixel format for the DC
	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 1;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 24;
	pfd.cDepthBits = 16;
	pfd.iLayerType = PFD_MAIN_PLANE;
	format = ChoosePixelFormat(*hDC, &pfd);
	SetPixelFormat(*hDC, format, &pfd);
	
	// create and enable the render context (RC)
	*hRC = wglCreateContext(*hDC);
	wglMakeCurrent(*hDC, *hRC);
}


void DisableOpenGL (HWND hWnd, HDC hDC, HGLRC hRC)
{
	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(hRC);
	ReleaseDC(hWnd, hDC);
}

LRESULT CALLBACK WndProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{	
	switch (message){
		
	case WM_CREATE:
		return 0;
	case WM_CLOSE:
		PostQuitMessage(0);
		return 0;
	case WM_DESTROY:
		return 0;
	case WM_KEYDOWN:
		switch (wParam){
		case VK_ESCAPE:
			PostQuitMessage(0);
			return 0;
		}
		return 0;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
}

int openglInit (const int flags, const int width, const int height)
{
	WNDCLASS wc;
	RECT rect;
	
	// register window class
	wc.style = CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = 0;
	wc.hIcon = 0;
	wc.hCursor = 0;
	wc.hbrBackground = 0;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = "vlcsgltmp";
	RegisterClass(&wc);
	
	hWnd = CreateWindowEx(0, wc.lpszClassName, "vlcsopengl", WS_POPUP|WS_CLIPSIBLINGS, 0, 0, width, height, NULL, NULL, 0, NULL);
	if (hWnd){
		GetClientRect(hWnd, &rect);
		MoveWindow(hWnd, 63, 63, (width-rect.right)+width, (height-rect.bottom)+height, 0);
	}
	
	//GetClientRect(hWnd, &rect);
	//printf("%i %i\n", rect.right, rect.bottom);
	
#if DRAW_TO_BITMAP
	BITMAPINFOHEADER BIH;
	memset(&BIH, 0, sizeof(BITMAPINFOHEADER));
	BIH.biSize = sizeof(BITMAPINFOHEADER);
	BIH.biWidth = width;
	BIH.biHeight = height;
	BIH.biPlanes = 1;
	BIH.biBitCount = 24;
	BIH.biCompression = BI_RGB;

	hdc = CreateCompatibleDC(0);
	m_hbmp = CreateDIBSection(hdc, (BITMAPINFO*)&BIH, DIB_PAL_COLORS, &m_pBits, NULL, 0);
    m_hbmOld = SelectObject(hdc, m_hbmp);

	EnableOpenGL(hdc, &hDC, &hRC);
#else
	EnableOpenGL((HDC)hWnd, &hDC, &hRC);
#endif
    
	return 1;
}
