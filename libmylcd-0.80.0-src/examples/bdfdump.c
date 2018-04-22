
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
#include "demos.h"


int dumpfont (TFRAME *frame, TWFONT *font);
#define MIN(a, b) ((a)<(b)?(a):(b))
#define MAX(a, b) ((a)>(b)?(a):(b))


int main (void)
{
	int argc = 0;
	wchar_t **argv = CommandLineToArgvW(GetCommandLineW(), &argc);
	if (argv == NULL/* || argc < 2*/){
		printf("not enough arguments\n");
		return 0;
	}
	
	int i = argc;

	if (!initDemoConfig("config.cfg"))
		return 0;

	wchar_t savename[MAX_PATH];
	wchar_t name[MAX_PATH];	name[0] = 0;
	wchar_t dir[MAX_PATH];
	wchar_t drive[MAX_PATH];
	
	lSetBackgroundColour(hw, lGetRGBMask(frame, LMASK_WHITE));
	lSetForegroundColour(hw, lGetRGBMask(frame, LMASK_BLACK));
	lCombinedCharDisable(hw);
	lHTMLCharRefDisable(hw);

	if (i > 1){
		while (--i){
			wprintf(L"\n%s\n", argv[i]);

			TWFONT regwfont;
			regwfont.FontID = 10000+i;
			regwfont.CharSpace = 2;
			regwfont.LineSpace = lLineSpaceHeight;
	
			if (lRegisterFontW(hw, argv[i], &regwfont)){
				TWFONT *font = (TWFONT *)lFontIDToFont(hw, regwfont.FontID);
				if (font){
					if (lCacheCharactersAll(hw, regwfont.FontID)){
						dumpfont(frame, font);
						lRefresh(frame);

						_wsplitpath(font->file, drive, dir, name, NULL);
						swprintf(savename, L"%s%s%s.tga", drive, dir, name);
						if (lSaveImage(frame, savename, IMG_TGA, frame->width, frame->height))
							wprintf(L"-> %s\n", savename);
						else
							wprintf(L"could not write file:%s\n", savename);
					}
				}
			}
		}
		//lSleep(3000);
		demoCleanup();
		return 1;
	}

	TENUMFONT *enf = lEnumFontsBeginW(hw);

	do{
		wprintf(L"%i %s   \n", enf->id, enf->wfont->file);
		
		if (lCacheCharactersAll(hw, enf->id)){
			dumpfont(frame, enf->wfont);
			lFlushFont(hw, enf->id);
			lRefresh(frame);

			*name = 0;
			_wsplitpath(enf->wfont->file, NULL, NULL, name, NULL);
			if (*name)
				swprintf(savename,L"%s.png", name);
			else
				swprintf(savename,L"%i.png", enf->id);
			lSaveImage(frame, savename, IMG_PNG, 0, 0);
		}
	}while(!kbhit() && lEnumFontsNextW(enf));
	lEnumFontsDeleteW(enf);
	demoCleanup();
	return 1;
}

int dumpfont (TFRAME *frame, TWFONT *font)
{
	// sanity check
	if (!font->built) return 0;

	unsigned int *cachedlist = (unsigned int *)malloc(sizeof(unsigned int)*font->GlyphsInFont);
	if (!cachedlist) return 0;
	int gtotal = abs(lGetCachedGlyphs(hw, cachedlist, font->GlyphsInFont, font->FontID));
	unsigned int *glist = (unsigned int *)malloc(sizeof(unsigned int)*gtotal*4);
	if (!glist) return 0;
	
	static char buffer[16];
	unsigned int *pglist = glist;
	unsigned int *pcachedlist = cachedlist;
	unsigned int ch;
	int total=0,ct=0,w=0,h=0;
	int i;
	unsigned int gtotal_1 = gtotal-1;

	if (*pcachedlist <= 0xFFFF)
		snprintf(buffer, 16, "%.4X| ", *pcachedlist);
	else
		snprintf(buffer, 16, "%X| ", *pcachedlist);

	for (i=0; i<6; i++)
		*pglist++ = (unsigned int)buffer[i];
	total+=6;

	for (ch=0; ch < gtotal; ch++){
		*pglist++ = *pcachedlist++;

		if (++ct == 16){		// 16 glyphs per row
			if (ch < gtotal_1){
				ct=0;
				*pglist++ = 10;		// add a new line
			
				if (*pcachedlist <= 0xFFFF)
					snprintf(buffer, 16, "%.4X| ", *pcachedlist);
				else
					snprintf(buffer, 16, "%X| ", *pcachedlist);

				for (i=0; i<6; i++)
					*pglist++ = (unsigned int)buffer[i];
				total+=6;
			}
		}else{
			*pglist++ = 32;		// insert ' ' between glyphs
		}
		total += 2;
	}

	TLPRINTR rect = {0,0,0,0,0,0,0,0};
	int flags = PF_CLIPWRAP|PF_DISABLEAUTOWIDTH|PF_DONTFORMATBUFFER;
	lGetTextMetricsList(hw, glist, 0, total-1, flags, font->FontID, &rect);
	lSetCharacterEncoding(hw, CMT_UTF16);	//  is ignored by the lxxxxxList API
	lGetTextMetrics(hw, (char*)font->file, 0, font->FontID, &w, &h);		// font path

	rect.sx = rect.sy = 0;
	rect.bx2 = MAX(rect.bx2,w)+1;
	rect.by2 += h+1;

	lResizeFrame(frame, rect.bx2, rect.by2, 0);
	lPrintEx(frame, &rect, font->FontID, PF_DONTFORMATBUFFER, LPRT_CPY, (char*)font->file);
	rect.ey++;
	lPrintList(frame, glist, 0, total-1, &rect, font->FontID, flags|PF_NEWLINE, LPRT_CPY);
	free(glist);
	free(cachedlist);
	
	return total;
}



