/*
 *                          The Art Of
 *                      D E M O M A K I N G
 *
 *                     by Alex J. Champandard
 *                          Base Sixteen
 *			
 *                      vga.cpp ported to libmylcd
 *                       by Michael McElligott
 *
 *                http://www.flipcode.com/demomaking
 *
 *                This file is in the public domain.
 *                      Use at your own risk.
 */


#include <stdio.h>
#include "demos.h"
#include "vga.h"


static TFRAME *front = NULL;
static TFRAME *frontScaled = NULL;
	
/*
 * initialize mode 320x200x8 and prepare the double buffering
 */
VGA::VGA()
{

	if (!initDemoConfig("config.cfg"))
		return;

	lSetBackgroundColour(hw, lGetRGBMask(frame, LMASK_BLACK));
	lSetForegroundColour(hw, lGetRGBMask(frame, LMASK_WHITE));
	
	lDeleteFrame(frame);
	frame = lNewFrame(hw, 320, 200, LFRM_BPP_8);
	front = lNewFrame(hw, 320, 200, DBPP);
	frontScaled = lNewFrame(hw, DWIDTH, DHEIGHT, DBPP);
	page_draw = (ubyte*)lGetPixelAddress(frame, 0, 0);

	int i = 256;
	while(i--)
		colors[i] = i;
}

/*
 * set a colour in the palette
 */
void VGA::SetColour (unsigned char i, unsigned char r, unsigned char g, unsigned char b)
{
	//colors[i].r = r<<18;	shift << 2 for 8bit, shift 16 for 24bit Red component
	//colors[i].g = g<<10;
	//colors[i].b = b<<2;

	if (front->bpp == LFRM_BPP_32 || front->bpp == LFRM_BPP_24)
		colors[i] = r<<18|g<<10|b<<2;
	else if (front->bpp == LFRM_BPP_16)
		colors[i] = (r>>1)<<11|g<<5|b>>1;
	else
		colors[i] = r|g|b;
}

/*
 * return to text mode, and clean up
 */
VGA::~VGA()
{
	lDeleteFrame(frontScaled);
	lDeleteFrame(front);
	demoCleanup();
}

unsigned char *VGA::GetSurfaceData()
{
	return page_draw;
}

void VGA::Blit (unsigned char *buffer, int bytes)
{
	memcpy(page_draw, buffer, bytes);
	Update();
}

void VGA::Lock ()
{
}

void VGA::Unlock ()
{
}


/*
 * flip the double buffer to the screen
 */
void VGA::Update()
{

	int x,y,rft,rfm;
	unsigned int c;
	
	// add palette, converting image from 8bpp to 24bpp
	for (y = 0; y < front->height; y++){
		rft = front->pitch*y;
		rfm = frame->pitch*y;
		for (x = 0; x < front->width; x++){
			c = lGetPixel_NBr(frame, x, rfm);
			if (c&0xFFFFFF00) c = 255;
			lSetPixel_NBr(front, x, rft, colors[c]);
		}
	}
	lCopyAreaScaled(front, frontScaled, 0, 0, front->width, front->height, 0, 0, frontScaled->width, frontScaled->height, LCASS_CPY);
	lRefresh(frontScaled);
}

/*
 * draw a pixel in the temporary buffer
 */
void VGA::PutPixel( int x, int y, unsigned c)
{
	lSetPixel_NB(frame, x, y, c);
}

/*
 * clear the double buffer
 */
void VGA::Clear()
{
	lClearFrame(frame);
}
