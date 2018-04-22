
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



#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mylcd.h"

#if (__BUILD_PGM_SUPPORT__)

#include "memory.h"
#include "fileio.h"
#include "utils.h"
#include "lstring.h"
#include "frame.h"
#include "pixel.h"
#include "pgm.h"
#include "image.h"


static int pgmGetHeaderInfo (FILE *f, int *width, int *height, int *maxval);



int savePgm (TFRAME *frame, const wchar_t *filename, int width, int height)
{
	if (!frame || !filename)
		return 0;

	FILE *fp = l_wfopen(filename, L"wb");
	if (!fp){
		return 0;
	}

	ubyte *pgmdata = l_malloc(height * width);
	if (!pgmdata ){
		l_fclose(fp);
		return 0;
	}

	double yscale = frame->height / (double)height;
	double xscale = frame->width / (double)width;
	double x,y;
	int pos=0;
	
	for (y=0;y<(double)frame->height;y+=yscale){
		for (x=0;x<(double)frame->width;x+=xscale){
			if (l_getPixel_NB(frame,x,y))
				*(pgmdata+pos++) = lPGMPIXELSET;
			else
				*(pgmdata+pos++) = lPGMPIXELCLEAR;
		}
	}
	
	fprintf(fp,"P5\n%i %i\n%i\n",width,height,lPGMPIXELTOTAL);
	l_fwrite(pgmdata, height * width, 1, fp);
	l_free(pgmdata);
	l_fclose(fp);
	return 1;	
}

int loadPgm (TFRAME *frame, const int flags, const wchar_t *filename, int ox, int oy)
{
	if (!frame){
		return 0;
	}

	FILE *f = l_wfopen(filename, L"r");
	if (!f){
		return 0;
	}

	int width,height,maxval;
	
	int ret = pgmGetHeaderInfo(f, &width, &height, &maxval);
	if (!ret || !width || !height || !maxval){
		l_fclose(f);
		return 0;
	}
	
	ubyte *pgmStorage = (ubyte *)l_malloc(height*width);
	if (!pgmStorage){
		l_fclose(f);
		return 0;
	}
	
	l_fread(pgmStorage,1,height*width,f);
	l_fclose(f);
	
	if (flags & LOAD_RESIZE)
		_resizeFrame(frame, width, height, 0);
	
	int x,y,i=0;
	maxval = (maxval>>1)+1;

	for (y=0;y<frame->height;y++){
		for (x=0;x<frame->width;x++){
			if (pgmStorage[i++] < maxval)
				l_setPixel(frame, x+ox, y+oy, LSP_SET);
		}
	}
	l_free(pgmStorage);
	return 1;
	
}

static int getline (FILE *f, char *text, unsigned int len)
{
	fgets(text,len,f);
	text[l_strlen(text)-1] = 0;
	return 1;
}

static int pgmGetHeaderInfo (FILE *f, int *width, int *height, int *maxval)
{
	static char text[128]={0};
	
	getline (f,text,128);
	if (!l_strncmp(text, "P5", 2)){
		getline(f,text,128);
		sscanf(text,"%i %i",width,height);
		if (*width && *height){
			getline(f,text,128);
			sscanf(text,"%i",maxval);
			return 1;
		}
	}
	return 0;
}


#else


int savePgm (TFRAME *frame, const wchar_t *filename, int width, int height){return 0;}
int loadPgm (TFRAME *frame, const wchar_t *filename, int style){return 0;}

#endif

