
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


#include "mylcd.h"
#include "memory.h"
#include "frame.h"
#include "fileio.h"
#include "lstring.h"
#include "utils.h"
#include "image.h"
#include "pixel.h"
#include "copy.h"
#include "bmp.h"
#include "tga.h"
#include "pgm.h"
#include "lpng.h"
#include "ljpg.h"

#include "sync.h"
#include "misc.h"

static int _loadImage (TFRAME *frame, const int flags, const wchar_t *filename, const int type, const int x, const int y);
static int _saveImage (TFRAME *frame, const wchar_t *filename, const int type, const int w, const int h);

static int saveRaw (TFRAME *frm, const wchar_t *filename);
static int loadRaw (TFRAME *frm, int flags, const wchar_t *filename, int x, int y);
static int guessImageType (const wchar_t *filename);


	

int saveImage (TFRAME *frame, const wchar_t *filename, const int img_type, int w, int h)
{
	if (filename == NULL){
		mylog("libmylcd: invalid filename\n");
		return -1;
	}
	if (frame == NULL){
		mylog("libmylcd: invalid input surface\n");
		return -1;
	}
	if (w < 1) w = frame->width;
	if (h < 1) h = frame->height;
	w = MAX(MINFRAMESIZEW, w);
	h = MAX(MINFRAMESIZEH, h);
	return _saveImage(frame, filename, img_type, w, h);
}

TFRAME *newImage (THWD *hw, const wchar_t *filename, const int img_type)
{
	if (filename == NULL){
		mylog("libmylcd: invalid filename\n");
		return NULL;
	}
	if (hw == NULL){
		mylog("libmylcd: invalid device handle\n");
		return NULL;
	}
	return _newImage(hw, filename, guessImageType(filename), img_type);
}
	
int loadImage (TFRAME *frame, const wchar_t *filename)
{
	if (filename == NULL){
		mylog("libmylcd: invalid filename\n");
		return -1;
	}
	if (frame == NULL){
		mylog("libmylcd: invalid input surface\n");
		return -1;
	}
	return _loadImage(frame, LOAD_RESIZE|0x02, filename, guessImageType(filename), 0, 0);
}

int loadImageEx (TFRAME *frame, const wchar_t *filename, const int ox, const int oy)
{
	if (filename == NULL){
		mylog("libmylcd: invalid filename\n");
		return -1;
	}
	if (frame == NULL){
		mylog("libmylcd: invalid input surface\n");
		return -1;
	}
	return _loadImage(frame, 0, filename, guessImageType(filename), ox, oy);
}

static int extMatch (const wchar_t *string, const wchar_t *ext)
{
	const int len = l_wcslen(string)-3;
	int i;
	for (i = len; i > len-4 && string[i]; i--){
		if (string[i] == ext[0] || string[i]-32 == ext[0]){
			if (string[i+1] == ext[1] || string[i+1]-32 == ext[1]){
				if (string[i+2] == ext[2] || string[i+2]-32 == ext[2]){
					if (string[i+3] == ext[3] || string[i+3]-32 == ext[3])
						return 1;
				}
			}
		}
	}
	return 0;
}

static int guessImageType (const wchar_t *filename)
{
    FILE *fp = l_wfopen(filename, L"rb");
    if (!fp) return 0;	
    
    int type = -1;
    ubyte data[16];
    l_memset(data, 0, sizeof(data));
    l_fread(&data, sizeof(data), 1, fp);
    l_fclose(fp);
        
    if (data[0] == 0x89 && data[1] == 'P' && data[2] == 'N' && data[3] == 'G'){
    	type = IMG_PNG;
    }else if (data[0] == 'B' && data[1] == 'M'){
    	type = IMG_BMP;
    }else if (data[0] == 0xFF && data[1] == 0xD8 && data[2] == 0xFF){
    	type = IMG_JPG;
    }else if (data[0] == 'P' && data[1] == '5' && data[2] == 0x0A){
    	type = IMG_PGM;
    }else if (data[3] == 1 || data[3] == 9 || data[3] == 2 || data[3] == 3 || data[3] == 10 || data[3] == 11){
    	type = IMG_TGA;
	}else if (data[0] == 'G' && data[1] == 'I' && data[2] == 'F'){
		type = -1;
    }else if (extMatch(filename, L".bmp")){
    	type = IMG_BMP;
	/*}else if (extMatch(filename, L".png")){
    	type = IMG_PNG;
	}else if (extMatch(filename, L".jpg")){
    	type = IMG_JPG;*/
	}else if (extMatch(filename, L".tga")){
    	type = IMG_TGA;
    }else if (extMatch(filename, L".pgm")){
    	type = IMG_PGM;
    /*}else if (extMatch(filename, L".raw")){
    	type = IMG_RAW;*/
    }
    
    if (type == -1){
    	mylogw(L"libmylcd: format of file not recognized '%s'", filename);
    	mylogw(L"\n");
    }
	return type;
}

static int _loadImage (TFRAME *frame, const int flags, const wchar_t *filename, const int type, const int ox, const int oy)
{
	int ret = 0;

	if (type == IMG_TGA){
		mylogw(L"libmylcd: loadImage() TGA: '%s'", filename); mylog("\n");
		ret = loadTga(frame, flags, filename, ox, oy);
	}else if (type == IMG_BMP){
		mylogw(L"libmylcd: loadImage() BMP: '%s'", filename); mylog("\n");
		ret = loadBmp(frame, flags, filename, ox, oy);
	}else if (type == IMG_JPG){
		mylogw(L"libmylcd: loadImage() JPG: '%s'", filename); mylog("\n");
		ret = loadJpg(frame, flags, filename, ox, oy);
	}else if (type == IMG_PNG){
		mylogw(L"libmylcd: loadImage() PNG: '%s'", filename); mylog("\n");
		ret = loadPng(frame, flags, filename, ox, oy);
	}else if (0 && type == IMG_RAW){
		mylogw(L"libmylcd: loadImage() RAW: '%s'", filename); mylog("\n");
		ret = loadRaw(frame, flags, filename, ox, oy);
	}else if (type == IMG_PGM && frame->bpp == LFRM_BPP_1){
		mylogw(L"libmylcd: loadImage() PGM: '%s'", filename); mylog("\n");
		ret = loadPgm(frame, flags, filename, ox, oy);
	}else{
		mylogw(L"libmylcd: loadImage(): unsupported: '%s'", filename); mylog("\n");
		ret = 0;
	}
	return ret;
}

TFRAME *_newImage (THWD *hw, const wchar_t *filename, const int type, const int frameBPP)
{
	TFRAME *frame = _newFrame(hw, 8, 8, 1, frameBPP);
	if (frame != NULL){
		if (_loadImage(frame, LOAD_RESIZE|0x02, filename, type, 0, 0) > 0)
			return frame;
		deleteFrame(frame);
	}
	return NULL;
}

static int _saveImage (TFRAME *frame, const wchar_t *filename, const int type, const int w, const int h)
{	
	int ret;
	
	switch(type&0xFF){
	  case IMG_BMP:
	  	mylogw(L"libmylcd: lSaveImage() BMP,%ix%i: '%s'", w, h, filename); mylog("\n");
		ret = saveBmp(frame, filename, w, h);
		break;
	  case IMG_TGA:
	  	mylogw(L"libmylcd: lSaveImage() TGA,%ix%i: '%s'", w, h, filename); mylog("\n");
		ret = saveTga(frame, filename, w, h);
		break;
	  case IMG_PNG:
	  	mylogw(L"libmylcd: lSaveImage() PNG,%ix%i: '%s'", w, h, filename); mylog("\n");
		ret = savePng(frame, filename, w, h, type&IMG_KEEPALPHA);
		break;
	  case IMG_JPG:
	  	mylogw(L"libmylcd: lSaveImage() JPG,%ix%i: '%s'", w, h, filename); mylog("\n");
	  	ret = saveJpg(frame, filename, w, h, 85);
		break;
	  case IMG_PGM:
	  	mylogw(L"libmylcd: lSaveImage() PGM,%ix%i: '%s'", w, h, filename); mylog("\n");
		ret = savePgm(frame, filename, w, h);
		break;
	  case IMG_RAW:
		mylogw(L"libmylcd: lSaveImage() RAW,%ix%i: '%s'", w, h, filename); mylog("\n");
		ret = saveRaw(frame, filename);
		break;
	  default: 
	  	ret = -2;
	}
	return ret;
}

static int loadRaw (TFRAME *frm, int flags, const wchar_t *filename, int x, int y)
{
	FILE *fp = NULL;
	unsigned int flen=0;
	ubyte *buffer=NULL;
	
    if (!(fp=l_wfopen(filename, L"r")))
    	return 0;
    
    l_fseek(fp,0, SEEK_END);
	if (!(flen=l_ftell(fp))){
		l_fclose (fp);
		return 0;
	}
	if (!(buffer=(ubyte*)l_malloc(flen*sizeof(ubyte*)))){
		l_fclose (fp);
		return 0;
	}
	unsigned int bread = l_fread(buffer,1,flen,fp);
	l_fclose(fp);
	if (bread != flen){
		l_free (buffer);
		return 0;
	}
	l_memcpy(frm->pixels, buffer, MIN(frm->frameSize, flen));
	l_free(buffer);
	return (bread);
}


static int saveRaw (TFRAME *frm, const wchar_t *filename)
{
	if (!frm || !filename)
		return 0;
	
    FILE *fp = l_wfopen(filename, L"wb");
    if (!fp)
    	return 0;

    size_t bwritten = l_fwrite(frm->pixels, 1, frm->frameSize, fp);
	l_fclose(fp);
    if (bwritten < frm->frameSize)
    	return 0;
	else
		return (int)bwritten;
}

