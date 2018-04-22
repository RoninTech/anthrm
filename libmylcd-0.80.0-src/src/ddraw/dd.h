//
// TinyPTC by Gaffer
// www.gaffer.org/tinyptc
//

#ifndef _DD_H_
#define _DD_H_

#define NONAMELESSUNION

#include <ddraw.h>
#include "converter.h"

typedef unsigned short short16;
typedef unsigned char char8;

#define MYLCDDDRAWWC "DDraw"

// configuration
#define __MDD_CLIPPER__		/*Z depth clip*/
#define __MDD_RESIZE_WINDOW__
#define __MDD_SYSTEM_MENU__
#define __MDD_ALLOW_CLOSE__
#define __MDD_CLOSE_ON_ESCAPE__
//#define __MDD_ICON__ "IDI_MAIN"


// converter configuration
#define __MDD_CONVERTER_32_TO_32_RGB888
#define __MDD_CONVERTER_32_TO_32_BGR888
#define __MDD_CONVERTER_32_TO_24_RGB888
#define __MDD_CONVERTER_32_TO_24_BGR888
#define __MDD_CONVERTER_32_TO_16_RGB565
#define __MDD_CONVERTER_32_TO_16_BGR565
#define __MDD_CONVERTER_32_TO_16_RGB555
#define __MDD_CONVERTER_32_TO_16_BGR555

//#define __MDD_MAIN_CRT__


// menu option identifier
#define SC_ZOOM_MSK			0x400
#define SC_ZOOM_100			0x401
#define SC_ZOOM_200			0x402
#define SC_ZOOM_400			0x404
#define SC_ZOOM_50			0x405
#define MU_DDSTATE_ENABLE	0x801
#define MU_DDSTATE_DISABLE	0x802


// typedef void (*CONVERTER) (void *src, void *dst, int pixels);

typedef struct {
	TDRIVER				*drv;
	HWND				wnd;			// handle to this window
	HMENU				menu;			// handle to menu
	MENUITEMINFO		menuinfo;		// menu item info
	int					mcount;			// menu item count
	LPDIRECTDRAW		lpDD;
	LPDIRECTDRAWSURFACE	lpDDS;
	LPDIRECTDRAWSURFACE	lpDDS_back;
	LPDIRECTDRAWSURFACE lpDDS_secondary;
    LPDIRECTDRAWCLIPPER lpDDC;
    CONVERTER			convert;
	unsigned int		*buffer;		// 32 bits per pixel buffer
	unsigned int		bufferLength;	// size of ^^ buffer
	T2POINT				des;
	int					status;			// active or not
	int original_window_width;
	int original_window_height;
}TMYLCDDDRAW;


int directDraw_open (TMYLCDDDRAW *mddraw, char *title, int width, int height);
int directDraw_update (TMYLCDDDRAW *mddraw, void *buffer, size_t bufferLength);
void directDraw_close (TMYLCDDDRAW *mddraw);

int registerWC ();
void releaseDDLib ();


#endif
