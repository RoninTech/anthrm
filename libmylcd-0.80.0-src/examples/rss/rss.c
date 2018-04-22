

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
#include <unistd.h>
#include <sys/types.h>
#if (!defined(__LINUX__))
 #include <wininet.h>
#endif


#include "rss.h"




static ubyte *parseChannel (TRSS *RSS, TRSSCHANNEL *C, ubyte *buffer);
static ubyte *parseItem (TRSS *RSS, TRSSITEM *I, ubyte *buffer);
static ubyte *parseImage (TRSS *RSS, TRSSIMAGE *Im, ubyte *buffer);
static ubyte *parseGuid (TRSS *RSS, TRSSGUID *G, ubyte *buffer);
static ubyte *parseEnclosureURL (TRSS *RSS, TRSSENCLOSE	*enc, ubyte *tag, ubyte *buffer);

static ubyte *getTag (ubyte *buffer, ubyte *tagbuff);
static ubyte *getText(ubyte *buffer, ubyte *tagbuff);
static ubyte *findTag (ubyte *buffer, char *tag, ubyte *tagbuff);

static ubyte *RSSStrDubStripHTML (TRSS *R, ubyte *str);
static ubyte *RSSStrAlloc(TRSS *R, int size);
static ubyte *RSSStrDub(TRSS *R, ubyte *str);

static int compare (const ubyte *s1, const char *s2);
static inline void striptagband (ubyte *str, char *open, char *close);
static inline void striptag (ubyte *str, ubyte *tag);


int freeRSS (TRSS *rss)
{
	if (!rss){
		return 0;
	}else{
		//if (rss->page) free(rss->page);
		if (rss->store) free(rss->store);
		free(rss);
		return 1;
	}
}

TRSS *newRSS (ubyte *buffer, int pageLen)
{
	TRSS *rss = (TRSS *)calloc(1, sizeof(TRSS));
	if (!rss) return NULL;

	rss->page = buffer;
	rss->ssize = pageLen+1;
	rss->store = (ubyte *)calloc(2*rss->ssize, sizeof(ubyte));
	if (rss->store){
		rss->C = rss->Channel;
		rss->C->I = rss->Channel->Item;
		return rss;
	}else{
		free(free);
		return NULL;
	}
}

int parseRSS (TRSS *RSS)
{
	ubyte *tag = calloc(2, RSS->ssize);
	ubyte *pbuffer = RSS->page;

	pbuffer = findTag(RSS->page,"xml version",tag);
	if (pbuffer == NULL){
		pbuffer = findTag(RSS->page,"?xml version",tag);
		if (pbuffer == NULL){
			free(tag);
			return 0;
		}
	}
	
	RSS->XMLtag = RSSStrDub(RSS,tag);

	#if 1
	pbuffer = findTag(pbuffer,"rss version",tag);
	if (pbuffer == NULL){
		pbuffer = findTag(RSS->page,"?rss version",tag);
		if (pbuffer == NULL)
			pbuffer = RSS->page;
	}
	#endif

	RSS->RSStag = RSSStrDub(RSS,tag);
	if (!findTag(pbuffer,"/channel",tag)){
		free(tag);
		return 0;
	}
	if (!findTag(pbuffer,"/rss",tag)){
		free(tag);
		return 0;
	}

	do{
		pbuffer = getTag(pbuffer,tag);
		if (!pbuffer){
			free(tag);
			return 0;
		}
		
		if (!compare(tag,"/rss")){
			free(tag);
			return 1;
			
		}else if (!compare(tag,"channel")){
			pbuffer = parseChannel(RSS, RSS->C, pbuffer);
			RSS->C++;
			RSS->totalChannels++;
		}
	}while(*pbuffer != 0);
	free(tag);
	return 1;
}

static ubyte *parseChannel (TRSS *RSS, TRSSCHANNEL *C, ubyte *buffer)
{
	ubyte *tag = calloc(2, RSS->ssize);
	ubyte *text = calloc(2, RSS->ssize);
	ubyte *pbuffer = buffer;

	do{
		pbuffer = getTag(pbuffer, tag);
		
		if (!compare(tag,"/channel")){
			free(tag); free(text);
			return pbuffer;

		}else if (!compare(tag,"title")){
			pbuffer = getText(pbuffer,text);
			if (C->Title == NULL)
				C->Title = RSSStrDubStripHTML(RSS,text);
			
		}else if (!compare(tag,"link")){
			pbuffer = getText(pbuffer,text);
			if (C->Link == NULL)
				C->Link = RSSStrDub(RSS,text);
			
		}else if (!compare(tag,"description")){
			pbuffer = getText(pbuffer,text);
			if (C->Descr == NULL)
				C->Descr = RSSStrDubStripHTML(RSS, text);

		}else if (!compare(tag,"language")){
			pbuffer = getText(pbuffer,text);
			C->Lang = RSSStrDub(RSS,text);
			
		}else if (!compare(tag,"pubDate")){
			pbuffer = getText(pbuffer,text);
			C->PubDate = RSSStrDub(RSS,text);
			
		}else if (!compare(tag,"lastBuildDate")){
			pbuffer = getText(pbuffer,text);
			C->LastBuild = RSSStrDub(RSS,text);
			
		}else if (!compare(tag,"docs")){
			pbuffer = getText(pbuffer,text);
			C->Docs = RSSStrDub(RSS,text);
			
		}else if (!compare(tag,"generator")){
			pbuffer = getText(pbuffer,text);
			C->Generator = RSSStrDub(RSS,text);
			
		}else if (!compare(tag,"managingEditor")){
			pbuffer = getText(pbuffer,text);
			C->ManEditor = RSSStrDub(RSS,text);
			
		}else if (!compare(tag,"webMaster")){
			pbuffer = getText(pbuffer,text);
			C->Webmaster = RSSStrDub(RSS,text);

		}else if (!compare(tag,"ttl")){
			pbuffer = getText(pbuffer,text);
			C->TTL = atoi((char*)RSSStrDub(RSS,text));

		}else if (!compare(tag,"copywrite")){
			pbuffer = getText(pbuffer,text);
			C->Copyright = RSSStrDub(RSS,text);
			
		}else if (!compare(tag,"rating")){
			pbuffer = getText(pbuffer,text);
			C->Rating = RSSStrDub(RSS,text);

		}else if (!compare(tag,"skiphours")){
			pbuffer = getText(pbuffer,text);
			C->SkipHours = RSSStrDub(RSS,text);

		}else if (!compare(tag,"skipdays")){
			pbuffer = getText(pbuffer,text);
			C->SkipDays = RSSStrDub(RSS,text);

		}else if (!compare(tag,"image")){
			pbuffer = parseImage(RSS, &C->Image,pbuffer);
						
		}else if (!compare(tag,"item")){
			pbuffer = parseItem(RSS, C->I++,pbuffer);
			C->totalItems++;
		}

		if (!pbuffer){
			free(tag); free(text);
			return 0;
		}
	}while (*pbuffer != 0);
	free(tag); free(text);
	return pbuffer-1;
}


static ubyte *parseImage (TRSS *RSS, TRSSIMAGE *Im, ubyte *buffer)
{
	ubyte *tag = calloc(2, RSS->ssize);
	ubyte *text = calloc(2, RSS->ssize);
	ubyte *pbuffer = buffer;

	do{
		pbuffer = getTag(pbuffer,tag);

		if (!compare(tag,"/image")){
			free(tag); free(text);
			return pbuffer;

		}else if (!compare(tag,"title")){
			pbuffer = getText(pbuffer,text);
			Im->Title = RSSStrDub(RSS,text);
			
		}else if (!compare(tag,"link")){
			pbuffer = getText(pbuffer,text);
			Im->Link = RSSStrDub(RSS,text);

		}else if (!compare(tag,"url")){
			pbuffer = getText(pbuffer,text);
			Im->URL = RSSStrDub(RSS,text);

		}else if (!compare(tag,"width")){
			pbuffer = getText(pbuffer,text);
			Im->width = atoi((char*)RSSStrDub(RSS,text));
			
		}else if (!compare(tag,"height")){
			pbuffer = getText(pbuffer,text);
			Im->height = atoi((char*)RSSStrDub(RSS,text));
		}

		if (!pbuffer){
			free(tag); free(text);
			return 0;
		}
	}
	while (*pbuffer != 0);
	free(tag); free(text);
	return pbuffer-1;
}

static inline void striptag (ubyte *str, ubyte *tag)
{
	ubyte *tmp = NULL;
	int i;
	
	do{
		tmp = (ubyte *)strstr((char*)str,(char*)tag);
		if (tmp){
			i = strlen((char*)tag);
			while(i--)
				*(tmp++) = 9;
		}
	}while(tmp);
}

static inline void striptagband (ubyte *str, char *open, char *close)
{
	ubyte *tmpo = NULL;
	ubyte *tmpc = NULL;
	int i;
	tmpo = str;
	size_t closeLen = strlen((char*)close);
	
	do{
		tmpo = (ubyte *)strstr((char*)tmpo,(char*)open);
		if (tmpo){
			tmpc = (ubyte *)strstr((char*)str,(char*)close);
			if (tmpc){
				i = (tmpc-tmpo) + closeLen;
				while(i--)
					*(tmpo++) = 9;	// 9 = an invalid glyph reference which mylcd will ignore
			}
		}
	}while(tmpo);
}

void replaceString (char *src, size_t srcLen, char *search, size_t serLen, char *replace, size_t repLen)
{
	char *tmp = NULL;

	do{
		tmp = strstr(src, search);
		if (tmp != NULL){
			src = tmp;
			strncpy(src, replace, repLen);
			strcpy(src+repLen, src+serLen);
		}
	}while(tmp);
}


static ubyte *RSSStrDubStripHTML (TRSS *R, ubyte *str)
{
	int len = strlen((char*)str);
	ubyte *stmp = RSSStrAlloc(R, len+1);

	if (stmp){
		replaceString((char*)str, len, "&amp;", 5, "&", 1);
		replaceString((char*)str, len, "&lt;", 4, "<", 1);
		replaceString((char*)str, len, "&gt;", 4, ">", 1);
		replaceString((char*)str, len, "&#60;", 5, "<", 1);
		replaceString((char*)str, len, "&#62;", 5, ">", 1);
		replaceString((char*)str, len, "&nbsp;", 6, " ", 1);
		replaceString((char*)str, len, "<div class", 10, " <", 2);
		replaceString((char*)str, len, "<br>", 4, "\n", 1);
		striptagband(str, "<![CDATA[", "]]>");
		striptagband(str, "<", ">");

		int j = 0;
		int i;
		for (i=0; i<len; i++){
			if (str[i] != 9)
				stmp[j++] = str[i];
		}
		stmp[j] = 0;
		return stmp;
	}else{
		return NULL;
	}
}


static ubyte *parseItem (TRSS *RSS, TRSSITEM *I, ubyte *buffer)
{
	ubyte *tag = malloc(RSS->ssize*2);
	ubyte *text = malloc(RSS->ssize*2);
	ubyte *pbuffer = buffer;


	do{
		memset(tag, 0, sizeof(RSS->ssize*2));
		memset(text, 0, sizeof(RSS->ssize*2));
			
		pbuffer = getTag(pbuffer, tag);

		if (!compare(tag,"/item")){

			free(tag); free(text);
			return pbuffer;

		}else if (!compare(tag,"title")){
			pbuffer = getText(pbuffer, text);
			if (I->Title == NULL){
				I->Title = RSSStrDubStripHTML(RSS, text);
			}
		}else if (!compare(tag,"link")){
			pbuffer = getText(pbuffer,text);
			I->Link = RSSStrDub(RSS,text);
			
		}else if (!compare(tag,"description")){
			pbuffer = getText(pbuffer, text);

			if (I->Descr == NULL){
				I->Descr = RSSStrDubStripHTML(RSS, text);
			}
		}else if (!compare(tag,"pubDate")){
			pbuffer = getText(pbuffer,text);
			I->PubDate = RSSStrDub(RSS,text);
			
		}else if (!compare(tag,"guid")){
			pbuffer = parseGuid(RSS, &I->guid, pbuffer);

		}else if (!compare(tag,"enclosure")){	
			pbuffer = parseEnclosureURL(RSS, &I->Enclosure, tag, pbuffer);
			
		}else{
			//printf("not parsed '%s'\n", tag);
		}

		if (!pbuffer){
			free(tag); free(text);
			return 0;
		}
	}while (*pbuffer != 0);

	free(tag); free(text);
	return pbuffer-1;
}


static ubyte *parseEnclosureURL (TRSS *RSS, TRSSENCLOSE	*enc, ubyte *tag, ubyte *pbuffer)
{
	if (!pbuffer) return 0;
	
	char *text = strdup((char*)tag);
	if (text){
		char *open = strchr(text, '\"');
		if (open){
			char *url = strtok(open+1, "\"");
			enc->URL = RSSStrDub(RSS, (ubyte*)url);
			enc->length = strlen((char*)enc->URL);
			open = strtok(NULL, "\"");
			if (open){
				char *type = strtok(NULL, "\"");
				enc->type = RSSStrDub(RSS, (ubyte*)type);
			}
		}
		free(text);
	}
		
	return pbuffer + enc->length;
}

static ubyte *parseGuid (TRSS *RSS, TRSSGUID *G, ubyte *buffer)
{
	if (!buffer) return 0;
	
	ubyte *pbuffer = buffer;
	ubyte *text = calloc(1, 4096);
	
	pbuffer = getText(pbuffer, text);
	G->id = RSSStrDub(RSS, text);
	free(text);
	
	return pbuffer;
}

static ubyte *findTag (ubyte *buffer, char *ftag, ubyte *tag)
{
	ubyte *pbuffer = buffer;
	do{
		pbuffer = getTag(pbuffer,tag);
		if (!pbuffer) return 0;
		else if (strstr((char*)tag,(char*)ftag)) return pbuffer;
	}while (*pbuffer != 0);

	return 0;
}

static ubyte *getText (ubyte *buffer, ubyte *tag)
{
	ubyte *open = (ubyte *)strchr((char*)buffer,'<');
	if (!open) return NULL;
	ubyte *close = (ubyte *)strchr((char*)++open,'>');

	if (close){
		int len = open-buffer-1;
		memcpy(tag,buffer,len);
		tag[len] = 0;
		return close+1;
	}

	return NULL;
}

static ubyte *getTag (ubyte *buffer, ubyte *tag)
{
	ubyte *open = (ubyte *)strchr((char*)buffer,'<');
	if (!open){
		return 0;
	}
	ubyte *close = (ubyte *)strchr((char*)++open,'>');
	if (close){
		int len = close-open;
		memcpy (tag,open,len);
		tag[len]=0;
		return close+1;
	}
	return 0;
}


//return 0 if equal, positive otherwise
static int compare (const ubyte *s1, const char *s2)
{
	return strncmp((char*)s1, s2, strlen(s2));
	
	while (*s1 == *s2){
		if (*s1++ == 0 || *s2++ == 0){
			return 0;
		}
	}
	return (*s1 - *(--s2));
}

static ubyte *RSSStrDub (TRSS *R, ubyte *str)
{
	int len = strlen((char*)str);
	ubyte *stmp = RSSStrAlloc(R,len+1);
	if (stmp){
		memcpy(stmp,str,len);
		stmp[len] = 0;
		return stmp;
	}else
		return NULL;
}

static ubyte *RSSStrAlloc (TRSS *R, int size)
{
	if (!R) return NULL;
	if (!R->store) return NULL;
	if (size+1+R->pos > R->ssize) return NULL;

	ubyte *mem = R->store + R->pos;
	R->pos += size + 1;  // +1 added to include \0 terminator
	return mem;
}

char *GetUrl (const char *url, size_t *totalRead)
{
	
	HINTERNET hOpenUrl;
	HINTERNET hSession = InternetOpen("httpGetFile", INTERNET_OPEN_TYPE_PRECONFIG, 0, 0, 0);
	if (hSession){
		hOpenUrl = InternetOpenUrl(hSession, url, 0, 0, INTERNET_FLAG_IGNORE_CERT_DATE_INVALID|INTERNET_FLAG_RELOAD|INTERNET_FLAG_EXISTING_CONNECT, 0);
		if (!hOpenUrl){
			InternetCloseHandle(hSession);
			return NULL;
		}
	}else{
		return NULL;
	}

	*totalRead = 0;
	const int allocStep = 128*1024;
	size_t allocSize = allocStep;
	char *buffer = malloc(allocSize);
	
	if (buffer){
		DWORD bread = 0;
		int status = 0;

		do {
			status = InternetReadFile(hOpenUrl, &buffer[*totalRead], allocStep, &bread);
			if (status == 1 && bread > 0){
				*totalRead += bread;
				allocSize += allocStep;
				buffer = realloc(buffer, allocSize);
			}else{
				buffer = realloc(buffer, *totalRead);
			}
		}while (buffer && status == 1 && bread > 0);

		InternetCloseHandle(hOpenUrl);
		InternetCloseHandle(hSession);
	}
	return buffer;
}

