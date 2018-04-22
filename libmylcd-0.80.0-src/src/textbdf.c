
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

#include "mylcd.h"

#if (__BUILD_BDF_FONT_SUPPORT__)

#include <string.h>

#include "memory.h"
#include "apilock.h"
#include "pixel.h"
#include "copy.h"
#include "fonts.h"
#include "textbdf.h"
#include "draw.h"
#include "chardecode.h"
#include "combmarks.h"
#include "lmath.h"
#include "api.h"

#include "sync.h"
#include "misc.h"

static int textRenderCheckWordBounds (const UTF32 *glist, int flags, int first, int last, TLPRINTR *rect, TWFONT *font);
static int textRenderGetCharWidth (TWCHAR *chr, int *left, int *right);
static int textRenderGetCharHeight (TWCHAR *chr);
static int textMetricsRect (THWD *hw, const UTF32 *glist, int first, int last, int flags, TWFONT *font, TLPRINTR *rect);
static int textWrapRenderLJ (TFRAME *frame, THWD *hw, const UTF32 *glist, int first, int last, TLPRINTR *rect, TWFONT *font, int flags, TLPOINTEX *tbound, int style);
static int textWrapRenderMJ (TFRAME *frame, THWD *hw, const UTF32 *glist, int first, int last, TLPRINTR *rect, TWFONT *font, int flags, TLPOINTEX *tbound, int style);
static int textWrapRenderRJ (TFRAME *frame, THWD *hw, const UTF32 *glist, int first, int last, TLPRINTR *rect, TWFONT *font, int flags, TLPOINTEX *tbound, int style);
static int textWrapRenderVRTL (TFRAME *frame, THWD *hw, const UTF32 *glist, int first, int last, TLPRINTR *rect, TWFONT *font, int flags, TLPOINTEX *tbound, int style);
static int textWrapRenderVLTR (TFRAME *frame, THWD *hw, const UTF32 *glist, int first, int last, TLPRINTR *rect, TWFONT *font, int flags, TLPOINTEX *tbound, int style);
static int textWrapRenderHRTL (TFRAME *frame, THWD *hw, const UTF32 *glist, int first, int last, TLPRINTR *rect, TWFONT *font, int flags, TLPOINTEX *tbound, int style);

static int _cacheCharacterBuffer (const UTF32 *glist, int ctotal, TWFONT *font);
static TCHARBOX *createCharBox (TFRAME *frame);



int cacheCharacterBuffer (THWD *hw, const UTF32 *glist, int total, int fontid)
{
	TWFONT *font = fontIDToFontW(hw, fontid);
	return _cacheCharacterBuffer(glist, total, font);
}

int getTextMetricsList (THWD *hw, const UTF32 *glist, int first, int last, int flags, int fontid, TLPRINTR *rect)
{
	TWFONT *font = fontIDToFontW(hw, fontid);
	return textMetricsRect(hw, glist, first, last, flags|PF_CLIPTEXTH, font, rect);
}

int textGlueRender (TFRAME *frm, THWD *hw, const char *str, TLPRINTR *rect, int fontid, int flags, int style)
{
	TWFONT *font;
	int gindex = 0;

	int total = countCharacters(hw, str);
	if (total){
		UTF32 *glist = (UTF32*)l_malloc(sizeof(UTF32)*total);
		if (glist){
			decodeCharacterBuffer(hw, str, glist, total);
			if ((font=fontIDToFontW(hw, fontid))){
				_cacheCharacterBuffer(glist, total--, font);
				if (flags&PF_CLIPWRAP)
					gindex = textWrapRender(frm, hw, glist, 0, total, rect, font, flags, style);
				else
					gindex = textRender(frm, hw, glist, 0, total, rect, font, flags, style);
			}
			l_free(glist);
		}
	}
	return gindex;
}

static int textWrapRenderLJ (TFRAME *frame, THWD *hw, const UTF32 *glist, int first, int last, TLPRINTR *rect, TWFONT *font, int flags, TLPOINTEX *tbound, int style)
{
	const int textboundflag = (flags&PF_GETTEXTBOUNDS) | (flags&PF_TEXTBOUNDINGBOX) | (flags&PF_INVERTTEXTRECT);
	int passcount = 1;
	int next = first;
			
	rect->ey = rect->sy;

	if (!frame){
		tbound->x1 = 0;//MAXFRAMESIZEW-1;
		tbound->y1 = MAXFRAMESIZEH-1;
	}else{
		tbound->x1 = frame->width-1;
		tbound->y1 = frame->height-1;
	}

	do{
		if (!passcount){
			rect->sy = rect->ey + font->LineSpace;	
		}else{
			passcount = 0;
			rect->sy = rect->ey;
		}

    	rect->ex = rect->sx = MAX(rect->sx, 0);
		rect->ey = rect->sy = MAX(rect->sy, 0);
		first = next;
		next = textRender(frame, hw, glist, first, last, rect, font, flags, style);
		rect->sx = MIN(rect->sx, rect->bx1);

		if (&glist[first] == glist){
			if (glist[first] == '\n'){
				rect->ey += 2;
				rect->sy += 2;
			}
		}
		if (textboundflag){
			tbound->x1 = MIN(tbound->x1, rect->sx);
			tbound->y1 = MIN(tbound->y1, rect->sy);
			tbound->x2 = MAX(tbound->x2, rect->ex);
			tbound->y2 = MAX(tbound->y2, rect->ey);
		}
	}while((next>first) && (next<=last) && next);

	if (glist[last] == '\n'){
		rect->ey += (font->PixelSize - font->fontDescent);
		rect->sy += (font->PixelSize - font->fontDescent);
	}

	if (textboundflag){
		tbound->x1 = MIN(tbound->x1, rect->sx);
		tbound->y1 = MIN(tbound->y1, rect->sy);
		tbound->x2 = MAX(tbound->x2, rect->ex);
		tbound->y2 = MAX(tbound->y2, rect->ey);
	}
	return next;
}


static int textWrapRenderRJ (TFRAME *frame, THWD *hw, const UTF32 *glist, int first, int last, TLPRINTR *rect, TWFONT *font, int flags, TLPOINTEX *tbound, int style)
{
	int width = 0;
	int passcount = 1;
	int next = first;
	int textboundflag = (flags&PF_GETTEXTBOUNDS) | (flags&PF_TEXTBOUNDINGBOX) | (flags&PF_INVERTTEXTRECT);
	rect->ey = rect->sy;

	if (!frame){
		tbound->x1 = 0;//MAXFRAMESIZEW-1;
		tbound->y1 = MAXFRAMESIZEH-1;
	}else{
		tbound->x1 = frame->width-1;
		tbound->y1 = frame->height-1;
	}


	do{
		if (!passcount){
			rect->sy = rect->ey + font->LineSpace;
		}else{
			passcount = 0;
			rect->sy = rect->ey;
		}

		rect->ex = rect->sx = 0;
		first = next;
		textRender(NULL, hw, glist, first, last, rect, font, flags|PF_DONTRENDER, style);

		if (glist[first] == '\n')
			rect->ey -= (font->PixelSize - font->fontDescent) - font->LineSpace;

		if (textboundflag){
			tbound->x2 = MAX(tbound->x2, rect->ex);
			tbound->y2 = MAX(tbound->y2, rect->ey);
		}
		width = (rect->ex - rect->sx);
		rect->ex = rect->sx = MAX(rect->bx2 - width, rect->bx1);
		//rect->ex = rect->sx = rect->bx2 - width;

		if (textboundflag){
			tbound->x1 = MIN(tbound->x1, rect->sx);
			tbound->y1 = MIN(tbound->y1, rect->sy);
		}
		next = textRender(frame, hw, glist, first, last, rect, font, flags, style);
	}while((next>first) && (next<=last) && next);

	if (glist[last] == '\n'){
		rect->ey += (font->PixelSize - font->fontDescent);
		tbound->y2 = MAX(tbound->y2, rect->ey);
		rect->sy += (font->PixelSize - font->fontDescent);
		tbound->y1 = MIN(tbound->y1, rect->sy);
	}
	return next;	
}

static int textWrapRenderMJ (TFRAME *frame, THWD *hw, const UTF32 *glist, int first, int last, TLPRINTR *rect, TWFONT *font, int flags, TLPOINTEX *tbound, int style)
{
	int width = 0;
	int passcount = 1;
	int next = first;
	int rectwidth = (rect->bx2 - rect->bx1) + 1;
	int textboundflag = (flags&PF_GETTEXTBOUNDS) | (flags&PF_TEXTBOUNDINGBOX) | (flags&PF_INVERTTEXTRECT);
	rect->ey = rect->sy;

	if (!frame){
		tbound->x1 = 0;//MAXFRAMESIZEW-1;
		tbound->y1 = MAXFRAMESIZEH-1;
	}else{
		tbound->x1 = frame->width-1;
		tbound->y1 = frame->height-1;
	}

	do{
		if (!passcount){
			rect->sy = rect->ey + font->LineSpace;
		}else{
			passcount = 0;
			rect->sy = rect->ey;
		}
		
		rect->ex = rect->sx = 0;
		first = next;
		textRender(NULL, hw, glist, first, last, rect, font, flags|PF_DONTRENDER, style);

		if (glist[first] == '\n')
			rect->ey -= (font->PixelSize - font->fontDescent)- font->LineSpace;

		if (textboundflag){
			tbound->x2 = MAX(tbound->x2, rect->ex);
			tbound->y2 = MAX(tbound->y2, rect->ey);
		}

		width = (rect->ex - rect->sx) + 1;
		if (width >= rectwidth){
			rect->ex = rect->sx = rect->bx1;
		}else{
			rect->ex = rect->sx = rect->bx1 + l_abs((rectwidth>>1)-((1+width)>>1)-2);  // was '((width)>>1)'
		}

		if (textboundflag){
			tbound->x1 = MIN(tbound->x1, rect->sx);
			tbound->y1 = MIN(tbound->y1, rect->sy);
		}

		next = textRender(frame, hw, glist, first, last, rect, font, flags, style);
		//next = ret;
	}while((next>first) && (next<=last) && next);

	if (glist[last] == '\n'){
		rect->ey += (font->PixelSize - font->fontDescent);
		tbound->y2 = MAX(tbound->y2, rect->ey);
		rect->sy += (font->PixelSize - font->fontDescent);
		tbound->y1 = MIN(tbound->y1, rect->sy);
	}
	return next;	
}

static int textWrapRenderHRTL (TFRAME *frame, THWD *hw, const UTF32 *glist, int first, int last, TLPRINTR *rect, TWFONT *font, int flags, TLPOINTEX *tbound, int style)
{
	int width = 0;
	int next = first;
	int rectwidth = (rect->bx2 - rect->bx1) + 1;
	int textboundflag = (flags&PF_GETTEXTBOUNDS) | (flags&PF_TEXTBOUNDINGBOX) | (flags&PF_INVERTTEXTRECT);
	TLPRINTR metrect;
	TLPRINTR *mrect = &metrect;

	
	if (!frame){
		tbound->x1 = MAXFRAMESIZEW-1;
		tbound->y1 = MAXFRAMESIZEH-1;
	}else{
		tbound->x1 = frame->width-1;
		tbound->y1 = frame->height-1;
	}

	rect->ey = rect->sy;
	rect->sx = rect->bx2 - rect->sx;

	if (textboundflag){
		tbound->x2 = MAX(tbound->x2, rect->ex);
		tbound->y2 = MAX(tbound->y2, rect->ey);
		tbound->x1 = MIN(tbound->x1, rect->sx);
		tbound->y1 = MIN(tbound->y1, rect->sy);
	}

	do{
		l_memcpy(mrect, rect, sizeof(TLPRINTR));
		mrect->ex = mrect->sx = 0;
		first = next;
		textRender(NULL, hw, glist, first, first, mrect, font, flags|PF_DONTRENDER, style);

		if (glist[first] == '\n'){
			if (&glist[first] == glist)
				rect->ey = mrect->ey += font->LineSpace + 2;
			else
				rect->ey = mrect->ey -= (font->PixelSize - font->fontDescent) - font->LineSpace;
				
			rect->sy =  mrect->ey;
			rect->sx = rect->bx2;
		}else{
			width = mrect->ex - mrect->sx;
			if (width >= rectwidth)
				rect->ex = rect->sx = rect->bx2;
			else
				rect->ex = rect->sx -= width;
		}
		next = textRender(frame, hw, glist, first, first, rect, font, flags, style);

		if (textboundflag){
			tbound->x2 = MAX(tbound->x2, rect->ex);
			tbound->y2 = MAX(tbound->y2, rect->ey);
			tbound->x1 = MIN(tbound->x1, rect->sx);
			tbound->y1 = MIN(tbound->y1, rect->sy);
		}
	}while((next>first) && (next<=last) && next);

	if (glist[last] == '\n'){
		rect->ey -= font->LineSpace;
		tbound->y2 -= font->LineSpace;
	}

	if (textboundflag){
		tbound->x2 = MAX(tbound->x2, rect->ex);
		tbound->y2 = MAX(tbound->y2, rect->ey);
		tbound->x1 = MIN(tbound->x1, rect->sx);
		tbound->y1 = MIN(tbound->y1, rect->sy);
	}
	return next;	
}

static int textWrapRenderVLTR (TFRAME *frame, THWD *hw, const UTF32 *glist, int first, int last, TLPRINTR *rect, TWFONT *font, int flags, TLPOINTEX *tbound, int style)
{

	int next = first;
	int textboundflag = (flags&PF_GETTEXTBOUNDS) | (flags&PF_TEXTBOUNDINGBOX) | (flags&PF_INVERTTEXTRECT);

	if (!frame){
		tbound->x1 = MAXFRAMESIZEW-1;
		tbound->y1 = MAXFRAMESIZEH-1;
	}else{
		tbound->x1 = frame->width-1;
		tbound->y1 = frame->height-1;
	}

	rect->ey = rect->sy;
	rect->ex = rect->sx = MAX(rect->bx1, rect->sx);

	if (textboundflag){
		tbound->x1 = MIN(tbound->x1, rect->sx);
		tbound->x2 = MAX(tbound->x2, rect->ex);
		tbound->y1 = MIN(tbound->y1, rect->sy);
		tbound->y2 = MAX(tbound->y2, rect->ey);
	}

	do{
		if (rect->sx >= rect->bx2)
			break;

		if (glist[next] == '\n'){
			if (rect->sx > rect->bx2)
				break;

			rect->sy = rect->ey = rect->by1;
			rect->ex = rect->sx += font->QuadWidth + font->LineSpace;
			first = next++;
		}else{
    		rect->ex = rect->sx = MIN(rect->sx, rect->bx2);
			rect->ey = rect->sy = MAX(rect->sy, 0);
			first = next;
			next = textRender(frame, hw, glist, first, first, rect, font, flags, style);
		}

		if (textboundflag){
			tbound->x1 = MIN(tbound->x1, rect->sx);
			tbound->x2 = MAX(tbound->x2, rect->ex);
			tbound->y1 = MIN(tbound->y1, rect->sy);
			tbound->y2 = MAX(tbound->y2, rect->ey);
		}
		rect->sy = rect->ey + font->CharSpace;
	}while((next>first) && (next<=last));

	if (glist[last] == '\n'){
		rect->ex += font->QuadWidth;
		rect->sx += font->QuadWidth;
	}

	if (textboundflag){
		tbound->x1 = MIN(tbound->x1, rect->sx);
		tbound->x2 = MAX(tbound->x2, rect->ex);
		tbound->y1 = MIN(tbound->y1, rect->sy);
		tbound->y2 = MAX(tbound->y2, rect->ey);
	}

	tbound->y2++;
	return next;	
}

static int textWrapRenderVRTL (TFRAME *frame, THWD *hw, const UTF32 *glist, int first, int last, TLPRINTR *rect, TWFONT *font, int flags, TLPOINTEX *tbound, int style)
{
	int next = first;
	int textboundflag = (flags&PF_GETTEXTBOUNDS) | (flags&PF_TEXTBOUNDINGBOX) | (flags&PF_INVERTTEXTRECT);

	if (!frame){
		tbound->x1 = MAXFRAMESIZEW-1;
		tbound->y1 = MAXFRAMESIZEH-1;
	}else{
		tbound->x1 = frame->width-1;
		tbound->y1 = frame->height-1;
	}

	rect->ey = rect->sy;
	rect->sx = MIN(rect->bx2 - rect->sx - font->QuadWidth, rect->bx2);
//	if ((int)rect->sx < 0) rect->sx = 0;
//	if ((int)rect->sy < 0) rect->sy = 0;
	rect->ex = rect->sx + font->QuadWidth;

	if (textboundflag){
		tbound->x1 = MIN(tbound->x1, rect->sx);
		tbound->x2 = MAX(tbound->x2, rect->ex);
		tbound->y1 = MIN(tbound->y1, rect->sy);
		tbound->y2 = MAX(tbound->y2, rect->ey);
	}

	do{
		if (rect->sx < rect->bx1)
			break;

		if (glist[next] == '\n'){
			if (rect->sx < rect->bx1)
				break;

			rect->sy = rect->ey = rect->by1;
			rect->ex = rect->sx -= font->QuadWidth + font->LineSpace;
			first = next++;
		}else{

    		rect->ex = rect->sx = MAX(rect->sx, 0);
			rect->ey = rect->sy = MAX(rect->sy, 0);
			first = next;
			next = textRender(frame, hw, glist, first, first, rect, font, flags, style);
		}
	
		if (textboundflag){
			tbound->x1 = MIN(tbound->x1, rect->sx);
			tbound->x2 = MAX(tbound->x2, rect->ex);
			tbound->y1 = MIN(tbound->y1, rect->sy);
			tbound->y2 = MAX(tbound->y2, rect->ey);
		}
		rect->sy = rect->ey + font->CharSpace;
	}while((next>first) && (next<=last));

	if (textboundflag){
		tbound->x1 = MIN(tbound->x1, rect->sx);
		tbound->x2 = MAX(tbound->x2, rect->ex);
		tbound->y1 = MIN(tbound->y1, rect->sy);
		tbound->y2 = MAX(tbound->y2, rect->ey);
	}
	tbound->y2++;
	return next;	
}


int textWrapRender (TFRAME *frame, THWD *hw, const UTF32 *glist, int first, int last, TLPRINTR *rect, TWFONT *font, int flags, int style)
{
	int next = first;
	TLPOINTEX tbound = {0, 0, 0, 0};
	flags |= PF_CLIPDRAW | PF_CLIPWRAP;

    if (flags&PF_MIDDLEJUSTIFY)
		next = textWrapRenderMJ(frame, hw, glist, first, last, rect, font, flags, &tbound, style);
	else if (flags&PF_RIGHTJUSTIFY)
		next = textWrapRenderRJ(frame, hw, glist, first, last, rect, font, flags, &tbound, style);
	else if (flags&PF_VERTICALRTL)
		next = textWrapRenderVRTL(frame, hw, glist, first, last, rect, font, flags, &tbound, style);
	else if (flags&PF_VERTICALLTR)
		next = textWrapRenderVLTR(frame, hw, glist, first, last, rect, font, flags, &tbound, style);
	else if (flags&PF_HORIZONTALRTL)
		next = textWrapRenderHRTL(frame, hw, glist, first, last, rect, font, flags, &tbound, style);
	else	// left justified by default
		next = textWrapRenderLJ(frame, hw, glist, first, last, rect, font, flags, &tbound, style);

	if (flags&PF_GETTEXTBOUNDS){
		rect->bx1 = tbound.x1;
		rect->by1 = tbound.y1;
		rect->bx2 = MIN(tbound.x2, rect->bx2);
		rect->by2 = MIN(tbound.y2, rect->by2);
	}

	if (frame){
		if (flags&PF_INVERTTEXTRECT){
			invertArea(frame, tbound.x1, tbound.y1, tbound.x1+tbound.x2, tbound.y2);
		}else if (flags&PF_TEXTBOUNDINGBOX){
			if (frame->bpp == LFRM_BPP_1){
				drawRectangleDotted(frame, tbound.x1, tbound.y1, tbound.x1+tbound.x2, tbound.y2, LSP_XOR);
			}else{
				int tmp = frame->style;
				frame->style = LSP_XOR;
				drawRectangleDotted(frame, tbound.x1, tbound.y1, tbound.x1+tbound.x2, tbound.y2, getRGBMask(frame, LMASK_WHITE));
				frame->style = tmp;
			}
		}
	}

	tbound.x2 += tbound.x1;
	return next;
}

static int textRenderGetCharHeight (TWCHAR *chr)
{
	int bottom = getBottomPixel(chr->f);
	if (bottom == -1){
		return 7;
	}else{
	 	return (bottom - getTopPixel(chr->f)) + 3;
	}
}

static int textRenderGetCharWidth (TWCHAR *chr, int *left, int *right)
{
	TCHARBOX *charbox;
	int w = chr->w;

	if (!chr->f->udata)
		charbox = createCharBox(chr->f);

	if (chr->f->udata){
		charbox = (TCHARBOX*)chr->f->udata;
		if (charbox->left == -1){
			*left = *right = 0;
			w = MIN(chr->dwidth, 1);	// '1' because it works
		}else{
			*left = charbox->left;
			*right = charbox->right;
			w = MIN(w, (*right-*left)+1);
		}
	}
	
	return w;
}

static int isWordBreak (UTF32 ch)
{
	if (ch == '-' || ch == ',' || ch == '.' || ch == '|' || ch == '\\' || ch == '\"' || ch == ':' || ch == '/')
		return 1;
	else
		return 0;
}

static int textRenderCheckWordBounds (const UTF32 *glist, int flags, int first, int last, TLPRINTR *rect, TWFONT *font)
{
	TWCHAR *chr = NULL;
	int left, right;
	int w = 0;
	int status = 0;
	int fixedwflag = flags&PF_FIXEDWIDTH;
	int noautowidthflag = flags&PF_DISABLEAUTOWIDTH;
	int i = first;
	int quadWidth;
	
	if 	(font->CharSpace < 0)
		quadWidth = font->QuadWidth + font->CharSpace;
	else
		quadWidth = font->QuadWidth;

	do{
		if (i<last){
			if (!isWordBreak(glist[i]) && (glist[i] != ' ') && (glist[i] != '\n')){
				if (!fixedwflag){
					if (glist[i] < font->maxCharIdx){
						if ((chr=font->chr[glist[i]])){
							if (!noautowidthflag)
								w += textRenderGetCharWidth(chr, &left, &right) + font->CharSpace + chr->xoffset;
							else
								w += chr->w-1+font->CharSpace;
						}
					}		
				}else{
					w += quadWidth;
				}
			}else{
				status = 0xFF;
			}
		}else{
			status = 0xFF;
		}
		i++;
	}while(!status);

	if (w && ((rect->ex+w+1) >= ((rect->bx2 - font->CharSpace)))){
		if (first == (i-1))
			return 0;
		else
			return w;
	}else{
		return 0;
	}
}

// returns next char index to render
// returns 0 on error
int textRender (TFRAME *to, THWD *hw, const UTF32 *glist, int first, int last, TLPRINTR *rect, TWFONT *font, int flags, int style)
{
	TWCHAR *chr = NULL;
	TWCHAR *defaultchr = NULL;
	TCHARBOX *charbox = NULL;
	TLPRINTREGION *loc = NULL;
	UTF32 ch = 0;

	int w=0,h=0;
	int yoffset=0;
	int left=0, right=0, top, btm;
	int copymode = style;
	int quadWidth;
	int nodefaultcharflag = flags&PF_NODEFAULTCHAR;
	const int dontrenderflag = flags&PF_DONTRENDER;
	const int cliptexthflag = flags&PF_CLIPTEXTH;
	const int cliptextvflag = flags&PF_CLIPTEXTV;
	const int clipdrawflag = PF_CLIPDRAW-(flags&PF_CLIPDRAW);
	const int wordwrapflag = PF_WORDWRAP-(flags&PF_WORDWRAP);
	const int noescapeflag = flags&PF_NOESCAPE;
	const int invertglyph1 = PF_INVERTGLYPH1-(flags&PF_INVERTGLYPH1);
	const int invertglyph2 = PF_INVERTGLYPH2-(flags&PF_INVERTGLYPH2);
	const int fixedwflag = flags&PF_FIXEDWIDTH;
	const int boundingboxflag = (flags&PF_GLYPHBOUNDINGBOX  && to);
	const int fixedautospace = !fixedwflag && !(flags&PF_DISABLEAUTOWIDTH) && (font->spacing != 'P');

	if 	(font->CharSpace < 0)
		quadWidth = font->QuadWidth + font->CharSpace;
	else
		quadWidth = font->QuadWidth;
		
	if (!dontrenderflag){
		if (to->hw->render->copy == NULL){
			mylog("libmylcd: NULL print render function\n");
			return 0;
		}
		loc = to->hw->render->loc;
		loc->to = to;
		loc->flags = flags;
		loc->y1 = 0;
	}
	if (font->DefaultChar < font->maxCharIdx){
		defaultchr = font->chr[font->DefaultChar];
	}else{
		nodefaultcharflag = 0;
	}

	if (glist[first] == 0x0A){
		rect->ey += (font->PixelSize - font->fontDescent);
		rect->ey = MIN(rect->ey, rect->by2);
	}

	last++;
	while (first < last){
		ch = glist[first];

		if (!noescapeflag)
			if (ch == 0x0A){
				return ++first;
			}

		if (!wordwrapflag){
			if (ch == ' '){ // shouldn't just compute 32
				if ((w=textRenderCheckWordBounds(glist, flags, first+1, last, rect, font))){
					if (w < (rect->bx2 - rect->bx1)+1){
						return ++first;
					}
				}
			}
		}

		if (ch >= font->maxCharIdx){
			if (!nodefaultcharflag)
				ch = font->DefaultChar; //'?';
		}
		if (ch < font->maxCharIdx){
			chr = font->chr[ch];
			if (chr == NULL && !nodefaultcharflag)
				chr = defaultchr;

			if (chr){
				if (chr->f){
					yoffset = MAX(font->PixelSize - chr->h - chr->yoffset-1, 0);
					if (!fixedautospace){
						h = chr->h;
						w = chr->w-1;
						right = w;
						left = 0;
					}else{
						if (flags&PF_VERTICALRTL)
							h = textRenderGetCharHeight(chr);
						else
							h = chr->h;
						w = textRenderGetCharWidth(chr, &left, &right);
					}
					
					if (!clipdrawflag){
						if (rect->ex+w+chr->xoffset > rect->bx2){
							if (!cliptexthflag){
								w -= (rect->ex+chr->xoffset+w) - rect->bx2;
							}else{
								return first;
							}
						}
						if (rect->sy+yoffset+h > rect->by2){
							if (!cliptextvflag){
								h -= (rect->sy+yoffset+h) - rect->by2;
							}else{
								return first;
							}
						}
					}

					if (!dontrenderflag){	// render glyph to destination frame to
						loc->glyph = chr;
						loc->dx = rect->ex+chr->xoffset;
						loc->dy = rect->sy+yoffset;
						loc->x1 = left;
						loc->x2 = right;
						loc->y2 = h;
						loc->style = copymode;
						to->hw->render->copy(loc);

						if (!invertglyph1)
							invertArea(to, rect->ex+chr->xoffset, rect->sy+yoffset, rect->ex+chr->f->width-1, rect->sy+chr->f->height-1);
						if (!invertglyph2)
							invertArea(to, rect->ex+chr->xoffset-1, rect->sy+yoffset-1, rect->ex+chr->f->width+1, rect->sy+chr->f->height+1);
					}
					if (boundingboxflag){
						const int tmp = to->style;
						int c;
						
						if (to->bpp == LFRM_BPP_1){
							c = LSP_XOR;
						}else{
							to->style = LSP_XOR;
							c = getRGBMask(to, LMASK_WHITE);
						}
						if ((charbox=(TCHARBOX*)chr->f->udata)){
							if (charbox->top != -999){
								top = charbox->top;
								btm = charbox->bottom;
							}else{
								top = charbox->top = getTopPixel(chr->f);
								btm = charbox->bottom = getBottomPixel(chr->f);
							}
						}else{
							top = getTopPixel(chr->f);
							if (top != -1)
								btm = getBottomPixel(chr->f);
						}
						if (top == -1) // empty glyph; a space?
							drawRectangleDotted(to, rect->ex+chr->xoffset, rect->sy+yoffset, rect->ex+w-1, rect->sy+h-1, c);
						else
							drawRectangleDotted(to, rect->ex+chr->xoffset, rect->sy+yoffset+top, rect->ex+chr->xoffset+w, rect->sy+yoffset+btm, c);
						to->style = tmp;
					}
				}
				rect->ey = MAX(rect->ey, rect->sy+h+yoffset);
				
				if (!isCombinedMark(hw, ch)){
					if (fixedwflag)
						rect->ex += quadWidth;
					else
						rect->ex += w + font->CharSpace + chr->xoffset;

					copymode = style;
				}else{
					copymode = LPRT_CPY;
				}
			}
			if (!clipdrawflag){
				if ((rect->ex > rect->bx2) || (rect->ey > rect->by2)){
					//if (fixedwflag)
						return ++first;
					//break;
				}
			}
			if (!wordwrapflag){
				if (isWordBreak(ch)){
					if ((w=textRenderCheckWordBounds(glist, flags, first+1, last, rect, font))){
						if (w < (rect->bx2 - rect->bx1)+1){
							return ++first;
						}
					}
				}
			}
		}
		first++;
	}
	return first;
}

// return dimensions of multiline text if rendered
static int textMetricsRect (THWD *hw, const UTF32 *glist, int first, int last, int flags, TWFONT *font, TLPRINTR *rect)
{
	rect->bx1 = rect->sx = rect->ex = 0;
	rect->by1 = rect->sy = rect->ey = 0;
	if (!rect->bx2)
		rect->bx2 = MAXFRAMESIZEW-1;
	if (!rect->by2)
		rect->by2 = MAXFRAMESIZEH-1;

	if (flags&PF_CLIPWRAP){
		return textWrapRender(NULL, hw, glist, first, last, rect, font, flags|PF_GETTEXTBOUNDS|PF_DONTRENDER, 0);
	}else{
		int pos = textRender(NULL, hw, glist, first, last, rect, font, flags|PF_DONTRENDER, 0);
		rect->bx2 = rect->ex;
		rect->by2 = rect->ey;
		return pos;
	}
}

int _cacheCharacterBuffer (const UTF32 *clist, int ctotal, TWFONT *font)
{
	int total = 0, cached = 0;

	UTF32 *glist = (UTF32*)l_malloc(sizeof(UTF32)*ctotal);
	if (glist){
		l_memcpy(glist,clist,sizeof(UTF32)*ctotal);
		if ((total=stripCharacterList(font->hw, glist, ctotal, font)))
			cached = cacheGlyphs(glist, total, font);
		l_free(glist);
	}
	return cached;
}

static TCHARBOX *createCharBox (TFRAME *frame)
{
	TCHARBOX *charbox = (TCHARBOX*)l_malloc(sizeof(TCHARBOX));
	if (charbox){
		frame->udata = (TCHARBOX*)charbox;
		charbox->top = -999;
		charbox->bottom = -999;
		charbox->left = getLeftPixel(frame);
		if (charbox->left < 0){
			charbox->left = 0;
			charbox->right = 2; // pick a value, 2 works
		}else{
			charbox->right = getRightPixel(frame);
		}
	}
	return charbox;
}


#else

int textGlueRender (TFRAME *frm, THWD *hw, char *str, TLPRINTR *rect, int fontid, int flags, int style){return 0;}
int textRender (TFRAME *frm, THWD *hw, UTF32 *glist, int first, int last, TLPRINTR *rect, TWFONT *font, int flags, int style){return 0;}
int textWrapRender (TFRAME *frm, THWD *hw, UTF32 *glist, int first, int last, TLPRINTR *rect, TWFONT *font, int flags, int style){return 0;}

#endif

