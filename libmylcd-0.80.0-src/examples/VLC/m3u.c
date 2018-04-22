
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


static wchar_t *media[] = {
	EXTAUDIO,
	EXTVIDEO,
	L""
};


TM3U *m3uNew ()
{
	TM3U *m = (TM3U*)my_calloc(1, sizeof(TM3U));
	return m;
}

void m3uFree (TM3U *m3u)
{
	if (m3u)
		my_free(m3u);
}	

static int m3uOpenRead (TM3U *m3u, const wchar_t *name)
{
	if (m3u->al)
		freeASCIILINE(m3u->al);
	m3u->al = readFileW(name);
	//wprintf(L"'%s' %p\n", name, m3u->al);
	return (m3u->al != NULL);
}

static void m3uCloseRead (TM3U *m3u)
{
	if (m3u){
		if (m3u->al){
			freeASCIILINE(m3u->al);
			m3u->al = NULL;
		}
	}
}

static int m3uOpenWrite (TM3U *m3u, const wchar_t *name)
{
	m3u->hFile = _wfopen(name, L"w+b");
	if (m3u->hFile){
		fseek(m3u->hFile, 0, SEEK_SET);
		fprintf(m3u->hFile,"﻿#EXTM3U\r\n");
	}
	return (m3u->hFile != NULL);
}

static void m3uCloseWrite (TM3U *m3u)
{
	if (m3u->hFile){
		fclose(m3u->hFile);
		m3u->hFile = NULL;
	}
}

int m3uOpen (TM3U *m3u, const wchar_t *name, const int action)
{
	switch (action){
	  case M3U_OPENREAD:
	  	m3u->mode = M3U_OPENREAD;
	  	return m3uOpenRead(m3u, name);
	  case M3U_OPENWRITE:
	  	m3u->mode = M3U_OPENWRITE;
	  	return m3uOpenWrite(m3u, name);
	}
	return -1;	
}

void m3uClose (TM3U *m3u)
{
	switch (m3u->mode){
	  case M3U_OPENREAD:
	  	m3uCloseRead(m3u);
	  	break;
	  case M3U_OPENWRITE:
	  	m3uCloseWrite(m3u);
	  	break;
	}
}

int m3uWritePlaylist (TM3U *m3u, PLAYLISTCACHE *plc, TMETATAGCACHE *tagc)
{
	char path[MAX_PATH_UTF8];
	char title[MAX_PATH_UTF8];
	char artwork[MAX_PATH_UTF8];
	char artist[1024];
	char album[1024];
	char length[128];
	unsigned int hash;
	int len;
	int trk = 0;
	int itemsWritten = 0;
	
	do{
		playlistGetPath(plc, trk, path, sizeof(path));
		if (*path){
			playlistGetTitle(plc, trk, title, sizeof(title));
			hash = getHash(path);
			tagRetrieveByHash(tagc, hash, MTAG_ArtworkURL, artwork, sizeof(artwork));
			if (*artwork)
				fprintf(m3u->hFile,"#EXTART:%s\r\n", artwork);

			tagRetrieveByHash(tagc, hash, MTAG_Album, album, sizeof(album));
			if (*album)
				fprintf(m3u->hFile,"#EXTALB:%s\r\n", album);
				
			tagRetrieveByHash(tagc, hash, MTAG_LENGTH, length, sizeof(length));			
			len = (int)stringToTime(length, sizeof(length));
			
			tagRetrieveByHash(tagc, hash, MTAG_Artist, artist, sizeof(artist));
			if (*artist)
				fprintf(m3u->hFile,"#EXTINF:%i,%s - %s\r\n", len, artist, title);
			else
				fprintf(m3u->hFile,"#EXTINF:%i,%s\r\n", len, title);
			
			fprintf(m3u->hFile,"%s\r\n", path);
			itemsWritten++;
		}
		trk++;
	}while(*path);

	return itemsWritten;
}

static int m3uParseEXTINF (char *in, char *_time, char *_artist, char *_title, const int bsize)
{
	*_time = 0;
	*_artist = 0;
	*_title = 0;
	
	char *time = strtok(in, ":");
	time = strtok(NULL, ",");
	char *artist = strtok(NULL, "-");
	char *title = strtok(NULL, "\0");
	
	if (title){
		if (*(title-2) == ' ') *(title-2) = '\0';	// remove space before -
		if (*title == ' ') title++;					// remove space after -
	}else if (artist){
		title = artist;
		artist = NULL;
	}
	if (time) strncpy(_time, time, bsize);
	if (artist) strncpy(_artist, artist, bsize);
	if (title) strncpy(_title, title, bsize);
	return (time || artist || title);
}

int m3uReadPlaylist (TM3U *m3u, TPLAYLISTMANAGER *plm, PLAYLISTCACHE *plc, TMETATAGCACHE *tagc)
{
	#define BLEN 1024
	
	char time[BLEN];
	char artist[BLEN];
	char title[BLEN];
	char album[BLEN];
	char artwork[8*MAX_PATH];
	int tagIdx = -999;
	char *line;
	int pos, tsec, ct = 0;

	*artwork = 0;
	*album = 0;
	*time = 0;
	*artist = 0;
	*title = 0;

	for (int i = 0; i < m3u->al->tlines; i++){
		line = (char*)m3u->al->line[i];
		if (*line == '#'){
			line++;
			if (!strncmp(line, "EXTINF:", 7)){
				if (m3uParseEXTINF(line, time, artist, title, BLEN))
					tagIdx = i;
			}else if (!strncmp(line, "EXTART:", 7)){
				char *art = strtok(line+7, "\r\n");
				if (art)
					strncpy(artwork, art, sizeof(artwork));

			}else if (!strncmp(line, "EXTALB:", 7)){
				char *alb = strtok(line+7, "\r\n");
				if (alb)
					strncpy(album, alb, sizeof(album));
			//}else if (!strncmp(line, "#EXTM3U", 7)){
			}
			continue;
		}

		if (isDirectory(line)){
			char *atitle;
			if (*album) atitle = album;
			else atitle = line;
			
			TFBROWSER *browser = my_calloc(1, sizeof(TFBROWSER));
			if (browser){
				browser->sort.mode = SORT_NONE;
				browser->sort.fn[browser->sort.mode] = sortByUnsorted;
				browser->sort.Do = browser->sort.fn[browser->sort.mode];
				browser->extStrings = media;
				browser->extType = EXT_MEDIA;
			
				PLAYLISTCACHE *plcN = playlistManagerCreatePlaylist(plm, atitle);
				if (plcN){
					browserImportPlaylistByDir(browser, line, plcN, 1);
					int total = playlistGetTotal(plcN);
					if (!total)
						playlistManagerDeletePlaylist(plm, plcN);
					else
						ct += total;
				}
				my_free(browser);
			}
		}else if (isPlaylist(line)){
			char *atitle;
			if (*album) atitle = album;
			else atitle = line;
			
			PLAYLISTCACHE *plcN = playlistManagerCreatePlaylist(plm, atitle);
			if (plcN){
				ct += importPlaylist(plm, plcN, tagc, line);
				if (!playlistGetTotal(plcN))
					playlistManagerDeletePlaylist(plm, plcN);
			}
		}else if (plc && (pos=playlistAdd(plc, line)) >= 0){
			if (tagIdx == i-1){
				if (*title){
					tagAdd(tagc, line, MTAG_Title, title, 1);
					playlistSetTitle(plc, pos, title, 1);
					*title = 0;
				}
				if (*artist){
					tagAdd(tagc, line, MTAG_Artist, artist, 1);
					*artist = 0;
				}
				if (*time){
					if ((tsec=atol(time)) > 0){
						timeToString((libvlc_time_t)tsec, time, BLEN);
						tagAdd(tagc, line, MTAG_LENGTH, time, 0);
					}
					*time = 0;
				}
				if (*album)
					tagAdd(tagc, line, MTAG_Album, album, 1);
				if (*artwork)
					tagAdd(tagc, line, MTAG_ArtworkURL, artwork, 1);
			}
			ct++;
		}
		*artwork = 0;
		*album = 0;
	}
	return ct;
}
