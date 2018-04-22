
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
#include <stddef.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <ctype.h>
#include <sys/stat.h>
#include <wchar.h>
#include <windows.h>


#include <mylcd.h>
#include "demos.h"


static void saveFrame (TFRAME *frame, wchar_t *name, wchar_t *ext, int imagetype);
int renderFile (TFRAME *frame, char *path, int flags, int font, int enc, int maxWidth);
int renderBuffer (TFRAME *frame, char *buffer, int flags, int font, int maxWidth);
uint64_t lof (FILE *stream);
uint64_t loadfile(char *path, char **buffer);

// better to use PNG
#define IMGFILETYPE	IMG_PNG
#define IMGEXT		L"png"


MYLCD_EXPORT uint64_t MYLCD_APICALL rdtsc();


int main (int argc, char* argv[])
{

	if (!initDemoConfig("config.cfg"))
		return 0;

	uint64_t t0 = GetTickCount();
	
	lSetBackgroundColour(hw, lGetRGBMask(frame, LMASK_WHITE));
	lSetForegroundColour(hw, lGetRGBMask(frame, LMASK_BLACK));
	lClearFrame(frame);
	
	int font = LFTW_MONAU16;
	lCacheCharactersAll(hw, font);
	lCombinedCharDisable(hw);


#if 1
	if (renderFile(frame, "filerendering/test.txt", PF_GLYPHBOUNDINGBOX, font, CMT_JISX0213, 0))
		saveFrame(frame, L"8140", IMGEXT, IMGFILETYPE);

	if (renderFile(frame, "filerendering/suzuri.txt", PF_VERTICALLTR|PF_DISABLEAUTOWIDTH, font, CMT_ISO2022_JP, 0))
		saveFrame(frame, L"suzuri_ltr", IMGEXT, IMGFILETYPE);

	if (renderFile(frame, "filerendering/suzuri.txt", PF_VERTICALRTL|PF_DISABLEAUTOWIDTH, font, CMT_ISO2022_JP, 0))
		saveFrame(frame, L"suzuri_rtl", IMGEXT, IMGFILETYPE);

	if (renderFile(frame, "filerendering/2022-jp", PF_DISABLEAUTOWIDTH, font, CMT_ISO2022_JP, 800)) // limit with to 800 pixels
		saveFrame(frame, L"2022-jp", IMGEXT, IMGFILETYPE);

	if (renderFile(frame, "filerendering/iso-2022-jp.txt", PF_LEFTJUSTIFY, font, CMT_ISO2022_JP, 2048)) // limit with to 2048 pixels
		saveFrame(frame, L"iso-2022-jp.txt", IMGEXT, IMGFILETYPE);

	if (renderFile(frame, "filerendering/jisx0213", PF_MIDDLEJUSTIFY, font, CMT_SHIFTJIS, 512)) // limit with to 512 pixels
		saveFrame(frame, L"jisx0213", IMGEXT, IMGFILETYPE);

	lSetFontLineSpacing(hw, font, -1);
	if (renderFile(frame, "filerendering/2ch-sjis", 0, font, CMT_SHIFTJIS, 0))
		saveFrame(frame, L"2ch-sjis", IMGEXT, IMGFILETYPE);


	lFlushFont(hw, font);	
	font = LFTW_UNICODE;
	lCacheCharactersAll(hw, font);

	if (renderFile(frame, "filerendering/euc-tw", PF_DISABLEAUTOWIDTH, font, CMT_EUC_TW, 0))
		saveFrame(frame, L"euc-tw", IMGEXT, IMGFILETYPE);

	if (renderFile(frame, "filerendering/euc-cn", PF_DISABLEAUTOWIDTH, font, CMT_EUC_CN, 0))
		saveFrame(frame, L"euc-cn", IMGEXT, IMGFILETYPE);

	if (renderFile(frame, "filerendering/hz-gb2312", PF_DISABLEAUTOWIDTH, font, CMT_HZ_GB2312, 0))
		saveFrame(frame, L"hz-gb2312", IMGEXT, IMGFILETYPE);

	if (renderFile(frame, "filerendering/GB18030.txt", PF_DISABLEAUTOWIDTH, font, CMT_GB18030, 1024)) // limit with to 1024 pixels
		saveFrame(frame, L"GB18030.txt_gb18030", IMGEXT, IMGFILETYPE);

	if (renderFile(frame, "filerendering/GB18030.txt", PF_DISABLEAUTOWIDTH, font, CMT_GBK, 1024)) // limit with to 1024 pixels
		saveFrame(frame, L"GB18030.txt_gbk_1024", IMGEXT, IMGFILETYPE);

	if (renderFile(frame, "filerendering/GB18030.txt", PF_DISABLEAUTOWIDTH, font, CMT_GBK, 0))
		saveFrame(frame, L"GB18030.txt_gbk_0", IMGEXT, IMGFILETYPE);

	if (renderFile(frame, "filerendering/faye-wong_big5.txt", PF_DISABLEAUTOWIDTH|PF_LEFTJUSTIFY, font, CMT_BIG5, 0))
		saveFrame(frame, L"faye-wong_big5_lj", IMGEXT, IMGFILETYPE);

	if (renderFile(frame, "filerendering/faye-wong_big5.txt", PF_DISABLEAUTOWIDTH|PF_MIDDLEJUSTIFY, font, CMT_BIG5, 0))
		saveFrame(frame, L"faye-wong_big5_mj", IMGEXT, IMGFILETYPE);

	if (renderFile(frame, "filerendering/faye-wong_big5.txt", PF_DISABLEAUTOWIDTH|PF_RIGHTJUSTIFY, font, CMT_BIG5, 0))
		saveFrame(frame, L"faye-wong_big5_rj", IMGEXT, IMGFILETYPE);

	if (renderFile(frame, "filerendering/faye-wong_big5.txt", PF_DISABLEAUTOWIDTH|PF_HORIZONTALRTL, font, CMT_BIG5, 0))
		saveFrame(frame, L"faye-wong_big5_hrtl", IMGEXT, IMGFILETYPE);

	if (renderFile(frame, "filerendering/faye-wong_big5.txt", PF_DISABLEAUTOWIDTH|PF_VERTICALRTL, font, CMT_BIG5, 0))
		saveFrame(frame, L"faye-wong_big5_vrtl", IMGEXT, IMGFILETYPE);

	if (renderFile(frame, "filerendering/faye-wong_big5.txt", PF_DISABLEAUTOWIDTH|PF_VERTICALLTR, font, CMT_BIG5, 0))
		saveFrame(frame, L"faye-wong_big5_vltr", IMGEXT, IMGFILETYPE);

	if (renderFile(frame, "filerendering/big5tbl", PF_DISABLEAUTOWIDTH, font, CMT_BIG5, 0))
		saveFrame(frame, L"big5tbl", IMGEXT, IMGFILETYPE);

	if (renderFile(frame, "filerendering/iso2022-kr", PF_DISABLEAUTOWIDTH, font, CMT_ISO2022_KR, 0))
		saveFrame(frame, L"iso-2022-kr", IMGEXT, IMGFILETYPE);

	if (renderFile(frame, "filerendering/euc-kr.txt", PF_DISABLEAUTOWIDTH, font, CMT_EUC_KR, 0))
		saveFrame(frame, L"euc-kr", IMGEXT, IMGFILETYPE);
		
	if (renderFile(frame, "filerendering/ja-repertoire2.txt", PF_DISABLEAUTOWIDTH, font, CMT_UTF8, 520)) // limit with to 520 pixels
		saveFrame(frame, L"ja-repertoire2", IMGEXT, IMGFILETYPE);
				
	if (renderFile(frame, "filerendering/sjis.txt", PF_DISABLEAUTOWIDTH, font, CMT_JISX0213, 1024)) // limit with to 1024 pixels
		saveFrame(frame, L"sjis_jisx0213", IMGEXT, IMGFILETYPE);

	if (renderFile(frame, "filerendering/jisx0213", PF_DISABLEAUTOWIDTH, font, CMT_JISX0213, 1024)) // limit with to 1024 pixels
		saveFrame(frame, L"jisx0213", IMGEXT, IMGFILETYPE);

	if (renderFile(frame, "filerendering/sjis.txt", PF_DISABLEAUTOWIDTH, font, CMT_AUTO_JP, 1024)) // limit with to 1024 pixels
		saveFrame(frame, L"sjis-auto_jp", IMGEXT, IMGFILETYPE);
	
	if (renderFile(frame, "filerendering/kanji_codes_sjis.txt", PF_DISABLEAUTOWIDTH, font, CMT_SHIFTJIS, 0))
		saveFrame(frame, L"kanji-codes-sjis_shiftjis", IMGEXT, IMGFILETYPE);

	if (renderFile(frame, "filerendering/vlcm.txt", PF_DISABLEAUTOWIDTH, font, CMT_BIG5, 0))
		saveFrame(frame, L"vlcm_big5", IMGEXT, IMGFILETYPE);

	//if (renderFile(frame, "filerendering/mencuis.uni", PF_DISABLEAUTOWIDTH|PF_WORDWRAP, font, CMT_UTF16, 800)) // limit width to 800 pixels
		//saveFrame(frame, L"mencuis_utf16", IMGEXT, IMGFILETYPE);

	if (renderFile(frame, "filerendering/gb2312tbl-utf16", PF_DISABLEAUTOWIDTH, font, CMT_UTF16, 0))
		saveFrame(frame, L"gb2312tbl-utf16_utf16", IMGEXT, IMGFILETYPE);
	
	if (renderFile(frame, "filerendering/gb2312tbl", PF_DISABLEAUTOWIDTH, font, CMT_GB18030, 0))
		saveFrame(frame, L"gb2312tbl_gb18030", IMGEXT, IMGFILETYPE);

	if (renderFile(frame, "filerendering/gb2312tbl", PF_DISABLEAUTOWIDTH, font, CMT_GBK, 0))
		saveFrame(frame, L"gb2312tbl_gbk", IMGEXT, IMGFILETYPE);

	lCombinedCharEnable(hw);
	if (renderFile(frame, "filerendering/ko-repertoire.txt", PF_DISABLEAUTOWIDTH, font, CMT_UTF8, 0))
		saveFrame(frame, L"ko-repertoire_utf8", IMGEXT, IMGFILETYPE);

	if (renderFile(frame, "filerendering/revelation.txt", 0, font, CMT_UTF8, 0))
		saveFrame(frame, L"revelation_utf8", IMGEXT, IMGFILETYPE);

	if (renderFile(frame, "filerendering/luki2.txt", 0, font, CMT_UTF8, 0))
		saveFrame(frame, L"luki2_utf8", IMGEXT, IMGFILETYPE);

	if (renderFile(frame, "filerendering/lyrics-ipa.txt", 0, font, CMT_UTF8, 0))
		saveFrame(frame, L"lyrics-ipa_utf8", IMGEXT, IMGFILETYPE);

	if (renderFile(frame, "filerendering/charreftbl.txt", PF_FIXEDWIDTH, font, CMT_ISO8859_11, 0))
		saveFrame(frame, L"charreftbl_iso1", IMGEXT, IMGFILETYPE);

	if (renderFile(frame, "filerendering/rune-poem.txt", 0, font, CMT_UTF8, 0))
		saveFrame(frame, L"rune-poem_utf8", IMGEXT, IMGFILETYPE);

	if (renderFile(frame, "filerendering/chineserad.txt", PF_DISABLEAUTOWIDTH, font, CMT_UTF8, 0))
		saveFrame(frame, L"chineserad_utf8", IMGEXT, IMGFILETYPE);

	if (renderFile(frame, "filerendering/korean.txt", 0, font, CMT_UTF8, 1024)) // limit with to 1024 pixels
		saveFrame(frame, L"korean_utf8", IMGEXT, IMGFILETYPE);
	
	if (renderFile(frame, "filerendering/japanese.txt", 0, font, CMT_UTF8, 0))
		saveFrame(frame, L"japanese_utf8", IMGEXT, IMGFILETYPE);
	
#endif

#if 1
	//lCombinedCharEnable(hw);
	lFlushFont(hw, font);
	font = LFTW_10x20;
	lCacheCharactersAll(hw, font);
		
	if (renderFile(frame, "filerendering/windows-1251.txt", 0, font, CMT_CP1251, 0))
		saveFrame(frame, L"windows-1251", IMGEXT, IMGFILETYPE);

	if (renderFile(frame, "filerendering/thaiutf8.txt", 0, font, CMT_UTF8, 0))
		saveFrame(frame, L"thai_utf8", IMGEXT, IMGFILETYPE);

	if (renderFile(frame, "filerendering/tis620.txt", 0, font, CMT_TIS620, 0))
		saveFrame(frame, L"tis620_internal", IMGEXT, IMGFILETYPE);
	
	if (renderFile(frame, "filerendering/tis620.txt", 0, font, CMT_ISO8859_11, 0))
		saveFrame(frame, L"tis620_iso11", IMGEXT, IMGFILETYPE);
		
	if (renderFile(frame, "filerendering/utf8decoder.txt", PF_DISABLEAUTOWIDTH, font, CMT_UTF8, 0))
		saveFrame(frame, L"utf8decoder", IMGEXT, IMGFILETYPE);
		
	lSetFontLineSpacing(hw, font, -3);

	if (renderFile(frame, "filerendering/chessboard.txt", PF_DISABLEAUTOWIDTH, font, CMT_UTF8, 0))
		saveFrame(frame, L"chessboard", IMGEXT, IMGFILETYPE);

	// http://www.cl.cam.ac.uk/~mgk25/ucs/examples/
	if (renderFile(frame, "filerendering/utf8demo.txt", PF_DISABLEAUTOWIDTH, font, CMT_UTF8, 0))
		saveFrame(frame, L"utf8demo", IMGEXT, IMGFILETYPE);
	
	if (renderFile(frame, "filerendering/quickbrown.txt", PF_DISABLEAUTOWIDTH, font, CMT_UTF8, 0))
		saveFrame(frame, L"quickbrown", IMGEXT, IMGFILETYPE);		
		
	if (renderFile(frame, "filerendering/combining-keycap.txt", PF_DISABLEAUTOWIDTH, font, CMT_UTF8, 0))
		saveFrame(frame, L"combining-keycap", IMGEXT, IMGFILETYPE);		
		
	if (renderFile(frame, "filerendering/boxdrawing.txt", PF_DISABLEAUTOWIDTH, font, CMT_UTF8, 0))
		saveFrame(frame, L"boxdrawing_utf8", IMGEXT, IMGFILETYPE);


		
		
#endif

	uint64_t t1 = GetTickCount();
	float t = (float)(t1-t0)*0.001;
	printf("time: %.2f\n", t);
	demoCleanup();
	return 1;

}

static void saveFrame (TFRAME *frame, wchar_t *name, wchar_t *ext, int imagetype)
{
#if 0
	printf(" done\n");
	return;
#endif

	printf(" %ix%i, saving...",frame->width, frame->height);
	
	wchar_t filepath[MaxPath];
	swprintf(filepath, L"%s.%s", name, ext);
	if (lSaveImage(frame, filepath, imagetype, frame->width, frame->height))
		printf(" done\n");
	else
		printf(" failed\n");
}

int renderFile (TFRAME *frame, char *path, int flags, int font, int enc, int maxWidth)
{
	printf("'%s' ...", path);
		
	char *buffer=NULL;
	if (loadfile(path,&buffer)){
		lSetCharacterEncoding(hw, enc);
		lClearFrame(frame);
		int total = renderBuffer(frame, buffer, flags, font, maxWidth);
		free(buffer);
		lRefresh(frame);
		return total;
	}else{
		printf(" failed\n");
		return 0;
	}
}

int renderBuffer (TFRAME *frame, char *buffer, int flags, int font, int maxWidth)
{
	int total = lCountCharacters(hw, buffer);
	if (!total) return 0;
	unsigned int *glist = (unsigned int *)malloc(sizeof(unsigned int)*total);
	if (!glist) return 0;

	TLPRINTR rect = {0,0,0,0,0,0,0,0};
	if (maxWidth < 0) maxWidth = 0;
	if (maxWidth)
		rect.bx2 = maxWidth-1;
	lDecodeCharacterBuffer(hw, buffer, glist, total);
	lGetTextMetricsList(hw, glist, 0, total-1, flags|PF_CLIPWRAP, font, &rect);
	rect.bx2 = abs(rect.bx2 - rect.bx1) + 1;
	lResizeFrame(frame, rect.bx2, rect.by2, 0);

	rect.bx2--;
	rect.bx1 = 0;
	rect.sx = 0;
	rect.ex = 0;
	rect.sy = 0;
	rect.ey = 0;
	rect.by1 = 0;

	lPrintList(frame, glist, 0, total, &rect, font, flags|PF_CLIPWRAP|PF_RESETXY, LPRT_CPY);
	free(glist);
	return total;
}


/*
uint64_t lof (FILE *stream)
{
	fpos_t pos;
	fgetpos(stream,&pos);
	fseek(stream,0,SEEK_END);
	uint64_t fl = (uint64_t)ftell(stream);
	fsetpos(stream,&pos);
	return fl;
}*/


uint64_t loadfile (char *path, char **buffer)
{
    FILE *fp = fopen(path,"rb");
    if (fp){
    	uint64_t len = lof(fp)+1;
    	*buffer = (char *)calloc(len,1);
		fread(*buffer,1,len-1,fp);
		fclose(fp);
		return len;
	}else{
		return 0;
	}
	
}

