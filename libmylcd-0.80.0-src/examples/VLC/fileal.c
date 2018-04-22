
// libmylcd - http://mylcd.sourceforge.net/
// An LCD framebuffer and text rendering API
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



#include "common.h"


static uint64_t lof (FILE *stream)
{
	fpos_t pos;
	fgetpos(stream,&pos);
	fseek(stream,0, SEEK_END);
	uint64_t fl = ftell(stream);
	fsetpos(stream,&pos);
	return fl;
}

void freeASCIILINE (TASCIILINE *al)
{
	if (al != NULL){
		if (al->line)
			my_free(al->line);
		if (al->data)
			my_free(al->data);
		my_free(al);
	}
}
#if 0
TASCIILINE *readFileA (const char *filename)
{
	FILE *fp = NULL;

    if (!(fp=fopen(filename,"rb"))){
		printf("readFileA(): unble to open %s - %s\n", filename, strerror(GetLastError()));
		return NULL;
    }

	TASCIILINE *al = (TASCIILINE *)my_malloc(sizeof(TASCIILINE));
	if (!al){
		fclose (fp);
		return NULL;
	}
		

	uint64_t flen = lof(fp);
	al->data = (ubyte *)my_calloc(sizeof(ubyte *),4+flen);
	if (!al->data){
		fclose (fp);
		my_free (al);
		return NULL;
	}

	fseek(fp, 0, SEEK_SET);
	unsigned int bread = fread(al->data, 1, flen, fp);
	fclose(fp);

	if (bread!=flen){
		my_free(al->data);
		my_free(al);
		return NULL;
	}

	al->tlines = 256;
	al->line = (ubyte **)my_calloc(1+al->tlines, sizeof(ubyte *));
	if (!al->line){
		my_free(al->data);
		my_free(al);
		return NULL;
	}
	
	int i = 0;
	ubyte *str = (ubyte*)strtok((char*)al->data, "\12\15");
	do{
		al->line[i++] = str;
		if (i >= al->tlines){
			al->tlines <<= 1;
			al->line = (ubyte **)my_realloc(al->line, (1+al->tlines)* sizeof(ubyte *));
			if (al->line == NULL){
				printf("readFileA: realloc returned null\n");
				return 0;
			}
		}
		str = (ubyte*)strtok(NULL, "\12\15");
	}while(str && i <= al->tlines);

	if (i < al->tlines){
		al->line = (ubyte **)my_realloc(al->line, (1+i) * sizeof(ubyte *));
		if (al->line == NULL){
			printf("readFileA: realloc returned null\n");
			return NULL;
		}
	}

	al->line[i] = NULL;
	al->tlines = i;

	return al;
}
#endif

#if 1
// wide by filepath only
TASCIILINE *readFileW (const wchar_t *filename)
{
	FILE *fp = NULL;

    if (!(fp=_wfopen(filename, L"rb"))){
    	mylogw(L"readFileW(): unble to open %s\n", filename);
    	return NULL;
    }

	TASCIILINE *al = (TASCIILINE *)my_malloc(sizeof(TASCIILINE));
	if (!al){
		fclose(fp);
		return NULL;
	}

	uint64_t flen = lof(fp);
	al->data = (ubyte *)my_calloc(sizeof(ubyte *),4+flen);
	if (!al->data){
		fclose(fp);
		my_free(al);
		return NULL;
	}

	fseek(fp, 0, SEEK_SET);
	unsigned int bread = fread(al->data, 1, flen, fp);
	fclose(fp);

	if (bread != flen){
		my_free(al->data);
		my_free(al);
		mylogw(L"readFileW(): unble to load %s\n", filename);
		return NULL;
	}

	al->tlines = 512;
	al->line = (ubyte **)my_calloc(1+al->tlines, sizeof(ubyte *));
	if (!al->line){
		my_free(al->data);
		my_free(al);
		return NULL;
	}
	
	int i = 0;
	ubyte *str;
	
	 // "﻿" == 0xEF,0xBB,0xBF == UTF8 BOM
	 // though a BOM isn't required pre se, my playlists and those saved via Winamp do contain one
	if (!strncmp((char*)al->data, "﻿", 3))
		str = (ubyte*)strtok((char*)al->data+3, "\12\15");
	else
		str = (ubyte*)strtok((char*)al->data, "\12\15");
		
	do{
		al->line[i++] = str;
		if (i >= al->tlines){
			al->tlines <<= 1;
			al->line = (ubyte **)my_realloc(al->line, (1+al->tlines) * sizeof(ubyte *));
		}
		str = (ubyte*)strtok(NULL, "\12\15");
	}while(str && i <= al->tlines);

	if (i < al->tlines)
		al->line = (ubyte **)my_realloc(al->line, (1+i) * sizeof(ubyte *));

	al->line[i] = NULL;
	al->tlines = i;
	return al;
}

#endif

