
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



#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "mylcd.h"
#include "../src/html.h"
#include "demos.h"


typedef struct{
	int		first;		// first unicode code char, begin here
	int		selected;	// selected glyph, index in to glist array (first+selected)
	unsigned int code;		// actual highlighted code point
}TBGHIGHLIGHT;

typedef struct{
	TFRAME	*frame;
	int		flags;
	int		trow;		// glyphs per row
}TBGPRINT;

typedef struct{
	TFRAME	*toppage;
	TWFONT	*font;
	unsigned int *glist;
	unsigned int gtotal;
	TBGPRINT print;
	TBGHIGHLIGHT hl;
	int		state;
	
	unsigned int ch;
	int		tch;
}TBDFGLYPH;


#define MIN(a, b) a<b?a:b
#define MAX(a, b) a>b?a:b

int processKeyPress (TBDFGLYPH *bg, char key);
int dumpfont (TBDFGLYPH *bg, TFRAME *frame);
static unsigned int UniToIndex (TBDFGLYPH *bg, unsigned int code);
static int isEntity (int code);


	
int main (int argc, char*argv[])
{
	
	if (!initDemoConfig("config.cfg"))
		return 0;


	//wchar_t bdffile[] = L"../fonts/bdf/wenquanyi_9pt.bdf";
	wchar_t bdffile[] = L"../fonts/bdf/monau16.bdf";
	//wchar_t bdffile[] = L"../fonts/bdf/b14.bdf";
	
	TBDFGLYPH bdfglyph;
	TBDFGLYPH *bg = &bdfglyph;
	memset(bg, 0, sizeof(TBDFGLYPH));

	bg->print.frame = frame; // PF_FIXEDWIDTH|
	bg->print.flags = PF_CLIPTEXTV|PF_FIXEDWIDTH|PF_DONTFORMATBUFFER|PF_USELASTX;
	bg->print.trow = 1;
	bg->hl.first = 0;
	bg->hl.selected = bg->hl.first;
	bg->hl.code = 0;
	bg->font = NULL;
	bg->toppage = bg->print.frame;
	bg->glist = NULL;
	bg->gtotal = 0;
	bg->state = 0;

	TWFONT regwfont;
	regwfont.FontID = 10001;
	regwfont.CharSpace = 2;
	regwfont.LineSpace = lLineSpaceHeight;

	if (lRegisterFontW(hw, bdffile, &regwfont)){
		bg->font = (TWFONT *)lFontIDToFont(hw, regwfont.FontID);
		if (bg->font){
			lCombinedCharDisable(hw);
			if (lCacheCharactersAll(hw, regwfont.FontID)){
				
				bg->glist = (unsigned int *)malloc(sizeof(unsigned int)*bg->font->GlyphsInFont);
				if (!bg->glist) return 0;
				
				int w;
				// get a list of glyphs successfully loaded via lCacheCharactersAll()
				bg->gtotal = abs(lGetCachedGlyphs(hw, bg->glist, bg->font->GlyphsInFont, bg->font->FontID));
				lGetFontMetrics(hw, bg->font->FontID, &w, NULL, NULL, NULL, NULL);
				bg->print.trow = bg->print.frame->width/(++w);
				bg->hl.first = UniToIndex(bg, 0);
				bg->hl.selected = 0;

				lCacheCharacterRange(hw, 0, 255, LFTW_MONAU14);
				//dumpfont(bg, bg->print.frame);
				lClearFrame(bg->print.frame);
				dumpfont(bg, bg->print.frame);

				do{
					lRefresh(bg->toppage);
					if (kbhit())
						bg->state = processKeyPress(bg, getch());
					else
						lSleep(30);
				}while(!bg->state);
			
				free(bg->glist);
			}
		}
	}

	
//	wchar_t savename[MaxPath];
//	swprintf(savename,"%s.tga", (char*)strtok(bdffile,"."));
//	lSaveImage(frame, savename, IMG_TGA, frame->width, frame->height);
	demoCleanup();
	return 1;
}


static int isEntity (int code)
{
	int i;
	for (i = 0; entities[i].len; i++){
		if (entities[i].encoding == code)
			return i;
	}
	return -1;
}

int dumpfont (TBDFGLYPH *bg, TFRAME *frame)
{
	TLPRINTR rect = {0,0,0,0,0,0,0,0};
	int ct = 0;
	int w,h;
	bg->ch = 0;
	bg->tch = 0;
	int maxh = 0;
	unsigned int first = 0, last = 0;
	TLPOINTEX box={0,0,-1,-1};	
	

	TWFONT *titlefont = (TWFONT*)lFontIDToFont(hw, LFTW_MONAU14);
	if (bg->font->PixelSize > titlefont->PixelSize)
		rect.sy = rect.ey = bg->font->PixelSize + 3;
	else
		rect.sy = rect.ey = titlefont->PixelSize + 3;
	
	lDrawLineDotted(frame, 0, rect.ey, frame->width-1, rect.ey, LSP_SET);
	rect.sy += 2;

	for (bg->ch = bg->hl.first;  rect.sy < frame->height-bg->font->QuadHeight; bg->ch++, bg->tch++){
		if (bg->ch >= bg->gtotal) bg->ch = 0;
		if (!bg->tch) first = bg->ch;
		last = bg->ch;

		if (bg->tch == bg->hl.selected){
			bg->hl.code = bg->glist[bg->ch];
			if (!lPrintList(frame, bg->glist, bg->ch, 1, &rect, bg->font->FontID, bg->print.flags|PF_INVERTGLYPH2, LPRT_CPY))
				break;
		}else{
			if (!lPrintList(frame, bg->glist, bg->ch, 1, &rect, bg->font->FontID, bg->print.flags, LPRT_CPY))
				break;
		}

		maxh = MAX(maxh, rect.ey);
		if (++ct == bg->print.trow){		// glyphs per row
			ct=0;
			rect.ex = rect.sx = 0;
			rect.sy = maxh + 1;
		}else{
			rect.ex++;
		}
	}

	TWCHAR *glyph = lGetGlyph(hw, NULL, bg->hl.code, bg->font->FontID);
	if (glyph){
		w = glyph->w;
		h = glyph->h;
		if (glyph->f){
			lGetImageBoundingBox(glyph->f, &box);
			if (box.x1 == -1){		// is empty
				box.x1 = 0;	box.x2 = -1;
				box.y1 = 0;	box.y2 = -1;
			}
		}
	}else{
		w = h = 0;
	}

	ct = isEntity(bg->hl.code);
	if (ct<0){
		lPrintEx(frame, &rect, LFTW_MONAU14, PF_RESETXY|PF_MIDDLEJUSTIFY|PF_CLIPWRAP, LPRT_CPY,\
		  " u%i x%.4X %ix%i (%ix%i)", bg->hl.code,  bg->hl.code, w, h, (box.x2-box.x1)+1, (box.y2-box.y1)+1);
	}else{
		lPrintEx(frame, &rect, LFTW_MONAU14, PF_RESETXY|PF_MIDDLEJUSTIFY|PF_CLIPWRAP, LPRT_CPY,\
		  " u%i x%.4X %ix%i (%ix%i) &amp;%s", bg->hl.code,  bg->hl.code, w, h, (box.x2-box.x1)+1,\
		  (box.y2-box.y1)+1, entities[ct].entity);
	}
	lPrintEx(frame, &rect, LFTW_MONAU14, PF_RESETXY, LPRT_CPY, "%i", bg->glist[first]); //, bg->glist[last]);
	rect.sx = rect.ex = (frame->width - bg->font->QuadWidth) - 4;
	lPrintEx(frame, &rect, bg->font->FontID, PF_RESETY, LPRT_CPY, "&#%d; ", bg->hl.code);
	return 1;
}



static unsigned int UniToIndex (TBDFGLYPH *bg, unsigned int code)
{
	unsigned int ch = code+1;
	while (ch--)
		if (bg->glist[ch] == code)
			return ch;

	for (ch = code; ch < bg->gtotal; ch++)
		if (bg->glist[ch] == code)
			return ch;

	return code;
}


int processKeyPress (TBDFGLYPH *bg, char key)
{
	if (key == '1'){
		if (--bg->hl.selected < 0){
			if (bg->hl.first - bg->tch-1 < 0)
				bg->hl.first = bg->gtotal - abs(bg->hl.first - bg->tch);
			else
				bg->hl.first -= bg->tch - 1;			
			bg->hl.selected = 0;
		}
		
		bg->hl.selected = MIN(bg->hl.selected,  bg->tch - 1);
		lClearFrame(bg->print.frame);
		dumpfont(bg, bg->print.frame);

	}else if (key == '2'){
		if (++bg->hl.selected == bg->tch){
			bg->hl.first = bg->ch-1;
			bg->hl.selected -= 1;
		}
		bg->hl.selected = MIN(bg->hl.selected,  bg->tch - 1);
		lClearFrame(bg->print.frame);
		dumpfont(bg, bg->print.frame);

		if (bg->hl.selected > bg->tch-1){
			bg->hl.selected = bg->tch-1;
			lClearFrame(bg->print.frame);
			dumpfont(bg, bg->print.frame);
		}
	}else if (key == '3'){
		bg->hl.selected = 0;
		lClearFrame(bg->print.frame);
		dumpfont(bg, bg->print.frame);

	}else if (key == '4'){
		bg->hl.selected = bg->tch-1;
		lClearFrame(bg->print.frame);
		dumpfont(bg, bg->print.frame);
		
		if (bg->hl.selected > bg->tch-1){
			bg->hl.selected = bg->tch-1;
			lClearFrame(bg->print.frame);
			dumpfont(bg, bg->print.frame);
		}
	}else if (key == 27){
		return 1;
	}
	return 0;
}
