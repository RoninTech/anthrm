
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
#include <inttypes.h>

typedef struct{
	unsigned int tlines;
	unsigned char	**lines;
	unsigned char	*data;
}TASCIILINE;



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
		if (al->lines)
			free(al->lines);
		if (al->data)
			free(al->data);
		free(al);
	}
}

TASCIILINE *readFileA (const char *filename)
{
	FILE *fp = NULL;

    if (!(fp=fopen(filename,"rb"))){
		printf(": readFileA(): unble to open '%s'\n", filename);
		return NULL;
    }

	TASCIILINE *al = (TASCIILINE *)malloc(sizeof(TASCIILINE));
	if (!al){
		fclose (fp);
		return NULL;
	}
		

	uint64_t flen = lof(fp);
	al->data = (unsigned char *)calloc(sizeof(unsigned char *),4+flen);
	if (!al->data){
		fclose (fp);
		free (al);
		return NULL;
	}

	fseek(fp, 0, SEEK_SET);
	unsigned int bread = fread(al->data, 1, flen, fp);
	fclose(fp);

	if (bread!=flen){
		free(al->data);
		free(al);
		return NULL;
	}

	al->tlines = 256;
	al->lines = (unsigned char **)calloc(1+al->tlines, sizeof(unsigned char *));
	if (!al->lines){
		free(al->data);
		free(al);
		return NULL;
	}
	
	unsigned int i = 0;
	unsigned char *str = (unsigned char*)strtok((char*)al->data, "\12\15");
	do{
		al->lines[i++] = str;
		if (i >= al->tlines){
			al->tlines <<= 1;
			al->lines = (unsigned char **)realloc(al->lines, (1+al->tlines)* sizeof(unsigned char *));
			if (al->lines == NULL){
				printf("readFileA: realloc returned null\n");
				return 0;
			}
		}
		str = (unsigned char*)strtok(NULL, "\12\15");
	}while(str && i <= al->tlines);

	if (i < al->tlines){
		al->lines = (unsigned char **)realloc(al->lines, (1+i) * sizeof(unsigned char *));
		if (al->lines == NULL){
			printf(": readFileA: realloc returned null\n");
			return NULL;
		}
	}

	al->lines[i] = NULL;
	al->tlines = i;

	return al;
}

#if 0
// wide by filepath only
TASCIILINE *readFileW (const wchar_t *filename)
{
	FILE *fp = NULL;

    if (!(fp=_wfopen(filename, L"rb"))){
    	mylogw(L"myLCD: readFileW(): unble to open %s\n", filename);
    	return NULL;
    }

	TASCIILINE *al = (TASCIILINE *)malloc(sizeof(TASCIILINE));
	if (!al){
		fclose(fp);
		return NULL;
	}

	uint64_t flen = lof(fp);
	al->data = (unsigned char *)calloc(sizeof(unsigned char *),4+flen);
	if (!al->data){
		fclose(fp);
		free(al);
		return NULL;
	}

	fseek(fp, 0, SEEK_SET);
	unsigned int bread = fread(al->data, 1, flen, fp);
	fclose(fp);

	if (bread != flen){
		free(al->data);
		free(al);
		mylogw(L"myLCD: readFileW(): unble to load %s\n", filename);
		return NULL;
	}

	al->tlines = 512;
	al->lines = (unsigned char **)calloc(1+al->tlines, sizeof(unsigned char *));
	if (!al->lines){
		free(al->data);
		free(al);
		return NULL;
	}
	
	int i = 0;
	unsigned char *str = (unsigned char*)strtok((char*)al->data, "\12\15");
	do{
		al->lines[i++] = str;
		if (i >= al->tlines){
			al->tlines <<= 1;
			al->lines = (unsigned char **)realloc(al->lines, (1+al->tlines) * sizeof(unsigned char *));
		}
		str = (unsigned char*)strtok(NULL, "\12\15");
	}while(str && i <= al->tlines);

	if (i < al->tlines)
		al->lines = (unsigned char **)realloc(al->lines, (1+i) * sizeof(unsigned char *));

	al->lines[i] = NULL;
	al->tlines = i;
	return al;
}

#endif

