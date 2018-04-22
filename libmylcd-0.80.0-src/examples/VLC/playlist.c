
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


#include "common.h"



// helps when debugging as VLC art handler spams the stdout/console
#define DISABLEARTWORK		0



#define getApplState(a) (((TVLCPLAYER*)a)->applState)
static const char *extTags[] = {EXTAUDIOA, EXTVIDEOA, ""};
void vlc_metaCB (const libvlc_event_t *event, void *udata);


static int getTicks (TVLCPLAYER *vp)
{
	return timeGetTime(); //GetTickCount();
}


static int hasPathExtA (const char *path, const char **exts)
{
	const int slen = strlen(path);
	int elen;
	
	for (int i = 0; *exts[i] == '.'; i++){
		elen = strlen(exts[i]);
		if (elen > slen) continue;
		if (!stricmp(path+slen-elen, exts[i]))
			return 1;
	}
	return 0;
}

static TMETACBSTRUCT *getMetaSlot (TVLCPLAYER *vp, TMETACB *metacb, const unsigned int hash)
{
	if (WaitForSingleObject(metacb->hLock, INFINITE) == WAIT_OBJECT_0){
		TMETACBSTRUCT *meta = metacb->metalist;
		if (getApplState(vp) && metacb->state){
			for (int i = 0; i < METALISTSIZE; i++, meta++){
				if (meta->status){
					if (meta->hash == hash){
						ReleaseMutex(metacb->hLock);
						return NULL;
					}
				}
			}
			meta = metacb->metalist;
			for (int i = 0; i < METALISTSIZE; i++, meta++){
				if (!meta->status){
					ReleaseMutex(metacb->hLock);
					return meta;
				}
			}
		}
		ReleaseMutex(metacb->hLock);
	}
	return NULL;
}
/*
void countItemsPrint (TMETACB *metacb)
{
	if (WaitForSingleObject(metacb->hLock, INFINITE) == WAIT_OBJECT_0){
		TMETACBSTRUCT *meta = metacb->metalist;
		for (int i = 0; i < METALISTSIZE; i++){
			if (meta->status)
				printf("%i: %i %i '%s'\n",i, meta->status, meta->position, meta->path);
			meta++;
		}
		ReleaseMutex(metacb->hLock);
		SetEvent(metacb->hEvent);
	}
}
*/
void countItems (TVLCPLAYER *vp, TMETACB *metacb)
{
	int state1 = 0;
	int state2 = 0;
	
	if (WaitForSingleObject(metacb->hLock, INFINITE) == WAIT_OBJECT_0){
		TMETACBSTRUCT *meta = metacb->metalist;
		for (int i = 0; i < METALISTSIZE; i++){
			state1 += ((meta->status&1) == 1);
			state2 += ((meta->status&2) == 2);
			meta++;
		}
		ReleaseMutex(metacb->hLock);
	}
	PLAYLISTCACHE *plcD = getDisplayPlaylist(vp);
	
	printf("%i %i %i %i %i\n", state1, state2, countUsedArtSlots(vp->tagc), playlistGetTotal(plcD), plcD->pr->selectedItem);
	dbprintf(vp, "%i  %i  %i  %i  %i", state1, state2, countUsedArtSlots(vp->tagc), playlistGetTotal(plcD), plcD->pr->selectedItem);
}

void freeMeta (TMETACBSTRUCT *meta)
{
	if (meta->cb_MediaMetaChanged)
		vlc_eventMediaDetach(meta->vlc, libvlc_MediaMetaChanged, vlc_metaCB, meta);
	if (meta->cb_MediaParsedChanged)
		vlc_eventMediaDetach(meta->vlc, libvlc_MediaParsedChanged, vlc_metaCB, meta);

	vlc_mediaRelease(meta->vlc);
	if (meta->vlc) my_free(meta->vlc);
	if (meta->path) my_free(meta->path);
}

void freeMetaSlots (TMETACB *metacb, const int state, const int byTime)
{
	float t1 = 0.0;
	if (byTime) t1 = getTicks(metacb->vp);
	
	if (WaitForSingleObject(metacb->hLock, INFINITE) == WAIT_OBJECT_0){
		TMETACBSTRUCT *meta = metacb->metalist;
		for (int i = 0; i < METALISTSIZE; i++){
			if (meta->status&state){
				meta->status = 0;
				meta->hash = 0;
				freeMeta(meta);
				meta->cb_MediaParsedChanged = 0;
				meta->cb_MediaMetaChanged = 0;
			}else if (meta->status && byTime){
				if (t1 - meta->stime > METATIMEOUT && !meta->etime){
					meta->status = 0;
					meta->hash = 0;
					freeMeta(meta);
					meta->cb_MediaParsedChanged = 0;
					meta->cb_MediaMetaChanged = 0;
				}
			}
			meta++;
		}
		ReleaseMutex(metacb->hLock);
	}
}

static int doesArtworkExistUtf8 (char *path)
{
	if (!path || !*path) return 0;

	int ret = 0;
	if (!strncmp(path, "file:///", 8)){
		char *out = decodeURI(path, strlen(path));
		if (out){
			ret = doesFileExistUtf8(out);
			my_free(out);
		}
	}
	return ret;
}

void vlc_metaCB (const libvlc_event_t *event, void *udata)
{
	TMETACBSTRUCT *meta = (TMETACBSTRUCT*)udata;
	if (!getApplState(meta->vp)) return;
	if (event->p_obj != meta->vlc->m) return;

#if 0
	if (event->type != libvlc_MediaDurationChanged && event->type != libvlc_MediaMetaChanged)
   		printf("%i: event %i: '%s'\n", meta->position+1, event->type, vlc_EventTypeToName(event->type));
#endif


	if (event->type == libvlc_MediaParsedChanged){
	
   		for (int i = 0; i < MTAG_TOTAL; i++){
   			if (i == MTAG_LENGTH){
				tagAddByHash(meta->vp->tagc, meta->path, meta->hash, MTAG_LENGTH, "0", 0);

			}else if (i == MTAG_POSITION){
				char buffer[64];
				if (snprintf(buffer, sizeof(buffer)-1, "%i", meta->position+1) > 0)
					tagAddByHash(meta->vp->tagc, meta->path, meta->hash, MTAG_POSITION, buffer, 1);

			}else if (i == MTAG_FILENAME){
				const int ilen = strlen(meta->path);
				char *filename = (char*)my_malloc(ilen+1);
				if (filename){
					char name[MAX_PATH_UTF8];
					char ext[MAX_PATH_UTF8];
					_splitpath(meta->path, NULL, NULL, name, ext);
					if (snprintf(filename, ilen, "%s%s", name, ext) > 0)
						tagAddByHash(meta->vp->tagc, meta->path, meta->hash, MTAG_FILENAME, filename, 1);
					my_free(filename);
				}

			}else if (i == MTAG_PATH){
				// do nothing

   			}else{
   				if (i == MTAG_ArtworkURL){
#if (!DISABLEARTWORK)
   					if (tagArtworkIsURLAvailableByHash(meta->vp->tagc, meta->hash))
#endif
   						continue;
   				}

   				char *tag = vlc_getMeta(meta->vlc, i);   				
				if (tag){
					if (*tag){
						if (i == MTAG_Title){
							//if (!tagRetrieveByHash(meta->vp->tagc, meta->hash, i)){
								tagAddByHash(meta->vp->tagc, meta->path, meta->hash, i, tag, 0);
								playlistSetTitle(meta->plc, meta->position, tag, 1);
							//}
						}else if (i == MTAG_ArtworkURL){
							if (!strncmp(tag, "file:", 5))
								tagAddByHash(meta->vp->tagc, meta->path, meta->hash, i, tag, 1);
						}else{
							tagAddByHash(meta->vp->tagc, meta->path, meta->hash, i, tag, 0);
						}
					}
					free(tag);
				}
			}
		}
	}else if (event->type == libvlc_MediaMetaChanged){

#if (!DISABLEARTWORK)
		if (event->u.media_meta_changed.meta_type == libvlc_meta_ArtworkURL){
			if (vlc_isParsed(meta->vlc) && meta->status == 1){
				char url[MAX_PATH_UTF8];
				*url = 0;

				tagRetrieveByHash(meta->vp->tagc, meta->hash, MTAG_ArtworkURL, url, sizeof(url));
				const int helloArtwork = doesArtworkExistUtf8(url);

				if (!*url || !helloArtwork){
					char *arturl = vlc_getMeta(meta->vlc, MTAG_ArtworkURL);
					if (arturl){
						//dbprintf(meta->vp, "'%s'", arturl);
						//printf("%s\n", arturl);
						
						// libVLC at first provides an external link (http://) rather than something local (file://)
						// dump URLs which don't point to a local file
						if (!strncmp(arturl, "file:", 5)){
							//strncpy(url, arturl, MAX_PATH_UTF8);
							
							tagAddByHash(meta->vp->tagc, meta->path, meta->hash, MTAG_ArtworkURL, arturl, 1);
     
     						// libVLC seems to post this artwork event msg before writing the image data to disk
     						// so wait at least 1 second before attempting to read
							meta->etime = 1600+getTicks(meta->vp);
						}
						free(arturl);
					}
				}else{
					meta->etime = helloArtwork;
					if (!meta->etime)
						printf("not found or invalid artwork; trk:%i, #%s#\n", 1+meta->position, url);
				}

				if (meta->etime){
					if (getApplState(meta->vp)){
						TPLAYLIST2 *pl = getPagePtr(meta->vp, PAGE_PLAYLIST2);
						SetEvent(pl->metacb->hEvent);
					}
				}
			}
		}
#endif
	}
}

int loadArt (TVLCPLAYER *vp, TTAGIMG *art)
{
	if (art->img) return 1;

	if (!doesFileExistW(art->path)) return 0;

	art->img = lNewImage(vp->hw, art->path, LFRM_BPP_32A);	// _BPP_32A to enable bilinear filtering in lCopyAreaScaled()
	if (art->img){
		int w, h;
		imageBestFit(MAXARTWIDTH, MAXARTHEIGHT, art->img->width, art->img->height, &w, &h);
		TFRAME *img = lNewFrame(vp->hw, w, h, LFRM_BPP_32);
		if (img){
			lCopyAreaScaled(art->img, img, 0, 0, art->img->width, art->img->height, 0, 0, w, h, LCASS_CPY);
			lDeleteFrame(art->img);
			art->img = img;
			art->enabled = 1;
			my_free(art->path);
			art->path = NULL;
			return 1;
		}
		lDeleteFrame(art->img);
		art->img = NULL;
	}
	return 0;
}

static int getArt (TVLCPLAYER *vp, TMETACBSTRUCT *meta, const char *arturl)
{
   	int ret = 0;
	if (!meta->hash) return ret;
	
   	TTAGIMG *art = g_tagArtworkGetByHash(vp->tagc, meta->hash);
   	if (!art){
   		art = g_tagArtworkAlloc(vp->tagc, getHash(arturl));
   		if (art){
   			char path[MAX_PATH_UTF8];
   			playlistGetPath(meta->plc, meta->position, path, sizeof(path));
			if (*path)
   				g_tagArtworkAdd(vp->tagc, path, art);
   		}else{
   			return ret;
   		}
   	}

	if (!art->enabled){
		char *tmp = decodeURI(arturl, strlen(arturl));
		if (tmp){
   			if (UTF8ToUTF16(tmp, strlen(tmp), art->path, MAX_PATH))
				ret = loadArt(vp, art);
	  		my_free(tmp);
		}
	}else{
		ret = 1;
	}
	return ret;
}

unsigned int __stdcall artLoader (void *ptr)
{
	TVLCPLAYER *vp = (TVLCPLAYER*)ptr;
	TPLAYLIST2 *pl = getPagePtr(vp, PAGE_PLAYLIST2);
	TMETACB *metacb = pl->metacb;
	TMETACBSTRUCT *meta;
	uint64_t currenttime, ct;
	int success = 0;
	char arturl[MAX_PATH_UTF8+1];

	
	while(getApplState(vp)){
		if (WaitForSingleObject(metacb->hEvent, INFINITE) == WAIT_OBJECT_0){
			if (getApplState(vp)){
				ct = 1;

				while(ct){
					ct = 0;
					
					if (WaitForSingleObject(metacb->hLock, INFINITE) == WAIT_OBJECT_0){
						if (getApplState(vp)){
							currenttime = getTicks(vp);
							
							if (!(meta=metacb->metalist)){
								ReleaseMutex(metacb->hLock);
								break;
							}

							for (int i = 0; i < METALISTSIZE && metacb->state; i++){
								if (meta->etime && meta->status == 1){
									if (currenttime - meta->etime >= 0){
										meta->etime = 0;
										success = 0;

										tagRetrieveByHash(vp->tagc, meta->hash, MTAG_ArtworkURL, arturl, sizeof(arturl));
										if (*arturl){
											if (tagLock(vp->tagc)){
												success = getArt(vp, meta, arturl);
												tagUnlock(vp->tagc);
											}
										}

										if (!success){
											char *arturl = vlc_getMeta(meta->vlc, MTAG_ArtworkURL);
											if (arturl){
												if (!strncmp(arturl, "file:", 5)){
													if (tagLock(vp->tagc)){
														success = getArt(vp, meta, arturl);
														tagUnlock(vp->tagc);
														if (success)
															tagAddByHash(vp->tagc, NULL, meta->hash, MTAG_ArtworkURL, arturl, 1);
													}
												}
												free(arturl);
											}
										}

										if (success) currenttime = getTicks(vp);
										meta->status = 2;
									}else{
										ct = 1;
									}
								}
								meta++;
							}
						}
						ReleaseMutex(metacb->hLock);
					}
					if (ct) lSleep(20);
				}
			}
		}
	}
	_endthreadex(1);
	return 1;
}

int playlistMetaGetTrackMeta (TVLCPLAYER *vp, TMETACB *metac, PLAYLISTCACHE *plc, const char *path, const int position)
{

	if (*path == 0) return -1;
	const unsigned int hash = getHash(path);
	int isArtReady = 0;


	// check if we've got the tags for this already
	// if so then just return if art has also been acquired (it usually has been)
	//int isTitleReady = tagArtworkIsTitleAvailableByHash(vp->tagc, hash);
	int isTitleReady = tagIsFilenameAvailableByHash(vp->tagc, hash);
	if (isTitleReady){
		isArtReady = tagArtworkIsArtworkAvailableByHash(vp->tagc, hash);
		if (isArtReady){
			//freeMetaSlots(metac, 2, 1);	// take this time to delete used meta handles
			return 1;
		}
	}

	// find an unoccupied slot and check if duplicated
	TMETACBSTRUCT *meta = getMetaSlot(vp, metac, hash);
	if (!meta) return -1;

	meta->vlc = my_calloc(1, sizeof(TVLCCONFIG));
	if (!meta->vlc) return 0;
	
	meta->vp = vp;
	meta->hash = hash;
	meta->position = position;
	meta->etime = 0;
	meta->path = my_strdup(path);
	meta->vlc->hLib = vp->vlc->hLib;
	meta->vlc->m = vlc_new_path(meta->vlc, meta->path);
	meta->plc = plc;
	
	if (meta->vlc->m && getApplState(vp)){
		if (vlc_getMediaEventManager(meta->vlc)){
			meta->status = 1;
			meta->stime = getTicks(vp);
			meta->cb_MediaParsedChanged |= ~isTitleReady;
			meta->cb_MediaMetaChanged |= ~isArtReady;
			//printf("%i:%i %i\n", meta->position, meta->cb_MediaParsedChanged, meta->cb_MediaMetaChanged);
			
			if (!isArtReady)
				vlc_eventMediaAttach(meta->vlc, libvlc_MediaMetaChanged, vlc_metaCB, meta);
			if (!isTitleReady)
				vlc_eventMediaAttach(meta->vlc, libvlc_MediaParsedChanged, vlc_metaCB, meta);
			vlc_mediaParseAsync(meta->vlc);
			return 1;
		}
	}
	meta->status = 0;
	meta->hash = 0;
	my_free(meta->path);
	my_free(meta->vlc);
	return -1;
}

void playlistMetaGetMeta (TVLCPLAYER *vp, TMETACB *metac, PLAYLISTCACHE *plc, int from, int to)
{
	char fname[MAX_PATH_UTF8];
	char ext[MAX_PATH_UTF8];
	char buffer[MAX_PATH_UTF8];
	char path[MAX_PATH_UTF8];
		
	from = MAX(0, from);
	to = MIN(playlistGetTotal(plc)-1, to);

	for (int i = from; i <= to; i++){
		playlistGetPath(plc, i, path, sizeof(path));
		if (!*path) continue;

		switch(hasPathExtA(path, extTags)){
		  case 1:
		  	if (playlistMetaGetTrackMeta(vp, metac, plc, path, i) == 1){
		  		SetEvent(metac->hEvent);
		  		break;
		  	}
		  case 0:
		  	*fname = 0; *ext = 0;
			_splitpath(path, NULL, NULL, fname, ext);
			if (snprintf(buffer, sizeof(buffer), "%s%s", fname, ext) > 0)
				playlistSetTitle(plc, i, buffer, 0);
		};
	}
	SetEvent(metac->hEvent);
}

void playlistChangeEvent (TVLCPLAYER *vp, PLAYLISTCACHE *plc, TPLAYLIST2 *pl, int trackIdx)
{
	//pl->pr->uiSelectedItem = -1;
	plc->pr->dragLocation.pos.y = 0;
	plc->pr->dragLocation.d0.y = 0;
	plc->pr->dragLocation.d1.y = 0;
	if (plc->pr->selectedItem >= playlistGetTotal(plc))
		plc->pr->selectedItem = -1;

	//tagFlushArt(vp->tagc);
	freeMetaSlots(pl->metacb, 3, 0);	//flush the playlist

	playlistMetaGetMeta(vp, pl->metacb, plc, trackIdx, trackIdx+10);
}

void trackLoadEvent (TVLCPLAYER *vp, PLAYLISTCACHE *plc, TPLAYLIST2 *pl, const int trackIdx)
{
	
	TMETA *meta = getPagePtr(vp, PAGE_META);

	// make meta display follow current track if its the currently highlighted track
	if (meta->trackPosition == plc->pr->playingItemPrevious)
		meta->trackPosition = trackIdx;

	// allow playlist2 to follow playing track when nothing else is selected
	if (plc->pr->selectedItem == -1){
		pl->startPosition = trackIdx;
		pl->voffset = 0;
	}

	plc->pr->playingItem = trackIdx;
	plc->pr->playingItemPrevious = plc->pr->playingItem;

	if (meta->trackPosition == -1 && plc->pr->playingItem >= 0)
		meta->trackPosition = plc->pr->playingItem;


	// clean up the meta tag handles that are either complete or timedout
	freeMetaSlots(pl->metacb, 2, 1);
	
	playlistMetaGetMeta(vp, pl->metacb, plc, plc->pr->playingItem-CACHEREADAHEAD/2, plc->pr->playingItem+CACHEREADAHEAD/2);
}

TMETACB *playlistMetaInit (void *user_ptr)
{
	TMETACB *meta = my_calloc(1, sizeof(TMETACB));
	if (!meta) return 0;

	meta->state = 1;
	meta->hEvent = CreateEvent(NULL, 0, 0, NULL);
	meta->hLock = CreateMutex(NULL, FALSE, NULL);
	meta->vp = user_ptr;

#if (!DISABLEARTWORK)
	meta->hThread = _beginthreadex(NULL, 0, artLoader, user_ptr, CREATE_SUSPENDED, &meta->threadID);
#endif
	return meta;

}

void playlistMetaShutdown (TMETACB *meta)
{
	meta->state = 0;
	SetEvent(meta->hEvent);
	
#if (!DISABLEARTWORK)
	WaitForSingleObject((HANDLE)meta->hThread, INFINITE);
	CloseHandle((HANDLE)meta->hThread);
#endif

	CloseHandle(meta->hEvent);
	freeMetaSlots(meta, 3, 0);
	WaitForSingleObject(meta->hLock, INFINITE);
	CloseHandle(meta->hLock);
	meta->hLock = NULL;
	my_free(meta);
	
}

int importPlaylistW (TPLAYLISTMANAGER *plm, PLAYLISTCACHE *plc, TMETATAGCACHE *tagc, const wchar_t *inpath)
{
	wchar_t drive[_MAX_DRIVE+1];
	wchar_t dir[_MAX_DIR+1];
	wchar_t path[_MAX_PATH+1];
	wchar_t name[_MAX_FNAME+1];
	wchar_t cpath[MAX_PATH+1];
	wchar_t ext[_MAX_EXT+1];
	*drive = 0;
	*dir = 0;
	*cpath = 0;

	_wsplitpath(inpath, drive, dir, name, ext);
				
	if (!*drive && *dir){
		GetCurrentDirectoryW(sizeof(cpath), cpath);
		swprintf(path, L"%s\\%s", cpath, dir);
		//wprintf(L"setting dir to: #%s#\n", cpath);
		SetCurrentDirectoryW(path);
		swprintf(path, L"%s%s", name, ext);
		//wprintf(L"opening #%s#\n", path);
		
	}else if (*drive){
		swprintf(path, L"%s%s", drive, dir);
		//wprintf(L"setting dir to: #%s#\n", path);
		SetCurrentDirectoryW(path);
		wcscpy(path, inpath);
		
	}else{
		wcscpy(path, inpath);
	}

	int total = 0;	
	TM3U *m3u = m3uNew();
	if (m3u){
		if (m3uOpen(m3u, path, M3U_OPENREAD)){
			//wprintf(L"reading #%s#\n", path);
			total = m3uReadPlaylist(m3u, plm, plc, tagc);
			m3uClose(m3u);
		}
		m3uFree(m3u);
	}
	
	if (*cpath)
		SetCurrentDirectoryW(cpath);

	return total;
}

int importPlaylist (TPLAYLISTMANAGER *plm, PLAYLISTCACHE *plc, TMETATAGCACHE *tagc, const char *path)
{
	wchar_t *out = converttow(path);
	int total = importPlaylistW(plm, plc, tagc, out);
	my_free(out);
	return total;
}

int playlistManagerRestart (TVLCPLAYER *vp)
{

	TVLCCONFIG *vlc = getConfig(vp);
	if (getPlayState(vp))
		player_stop(vp, vlc);
	if (vlc->isMediaLoaded)
		unloadMedia(vp, vlc);
	
	TPLAYLISTMANAGER *plm = vp->plm;
	
	if (playlistManagerLock(plm)){
		TPLAYLISTMANAGER *plmNew = playlistManagerNew();
				
		PLAYLISTCACHE *plcP = playlistManagerCreatePlaylist(plmNew, PLAYLIST_PRIMARY);
		vp->displayPlaylist = playlistManagerGetPlaylistIndex(plmNew, plcP);
		vp->queuedPlaylist = 0;
		vp->plm = plmNew;

		//playlistManagerUnlock(plm);
		playlistManagerDelete(plm);
	}
	
	return (vp->plm != NULL);
}


PLAYLISTCACHE *getPrimaryPlaylist (TVLCPLAYER *vp)
{
	int plIdx = playlistManagerGetPlaylistNext(vp->plm, -1);
	return playlistManagerGetPlaylist(vp->plm, plIdx);
}

PLAYLISTCACHE *getDisplayPlaylist (TVLCPLAYER *vp)
{
	PLAYLISTCACHE *plcD = playlistManagerGetPlaylist(vp->plm, vp->displayPlaylist);
	if (!plcD){
		vp->displayPlaylist = playlistManagerGetPlaylistNext(vp->plm, -1);
		plcD = playlistManagerGetPlaylist(vp->plm, vp->displayPlaylist);
	}
	//if (!plcD)
	//	printf("void display playlist %i\n", vp->displayPlaylist);

	return plcD;
}

PLAYLISTCACHE *getQueuedPlaylist (TVLCPLAYER *vp)
{
	PLAYLISTCACHE *plcQ = playlistManagerGetPlaylist(vp->plm, vp->queuedPlaylist);
	//if (!plcQ)
	//	printf("void queued Playlist %i\n", vp->queuedPlaylist);

	return plcQ;
}
