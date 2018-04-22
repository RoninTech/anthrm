// playlist media control overlay
// (next trk, play, art size, etc..)
//
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


/*
// available meta tags
static const char *tagStrLookup[] = {
	"title",
    "artist",
    "genre",
    "copyright",
    "album",
    "track",
    "description",
    "rating",
    "date",
    "setting",
    "url",
    "language",
    "nowplaying",
    "publisher",
    "encodedby",
    "artpath",
    "trackid",
    "length",
    "filename",
    "position",
    "path",
    ""
};
*/
static const wchar_t *wtagStrTable[] = {
	L"title",
    L"artist",
    L"genre",
    L"copyright",
    L"album",
    L"track",
    L"description",
    L"rating",
    L"date",
    L"setting",
    L"url",
    L"language",
    L"nowplaying",
    L"publisher",
    L"encodedby",
    L"artpath",
    L"trackid",
    L"length",
    L"filename",
    L"position",
    L"path",
    L""
};


static int tagLookupW (const wchar_t *str)
{
	for (int mtag = 0; mtag < MTAG_TOTAL; mtag++){
		if (!wcscmp(str, wtagStrTable[mtag]))
			return mtag;
	}
	return -1;
}

static const wchar_t *getTagW (const int mtag)
{
	if (mtag >= 0 && mtag < MTAG_TOTAL)
		return wtagStrTable[mtag];
	else
		return L" ";
}

int searchPlaylist (TVLCPLAYER *vp, PLAYLISTCACHE *plc, const int mtag, const char *search, const int from)
{
	//if (mtag >= 0)
	//	dbprintf(vp, "searching for '%s = %s' in '%s'", getTag(mtag), search, plc->name);
	//else
	//	dbprintf(vp, "searching for '%s' in '%s'", search, plc->name);
				
	int trk = -1;
	if (mtag == -1){
		if (from >= 0)
			trk = playlistSearch(plc, vp->tagc, search, from+1);
		else
			trk = playlistSearch(plc, vp->tagc, search, 0);
	}else{
		if (from >= 0)
			trk = playlistSearchTag(plc, vp->tagc, mtag, search, from+1);
		else
			trk = playlistSearchTag(plc, vp->tagc, mtag, search, 0);
	}

	return trk;
}

int searchPlaylistPlm (TVLCPLAYER *vp, TPLAYLISTMANAGER *plm, const int mtag, const wchar_t *txt, const size_t ilen)
{
	
	PLAYLISTCACHE *plc;
	int trk = -1;
	int i = playlistManagerGetPlaylistNext(plm, vp->displayPlaylist-1);
	
	while(i != -1){
		plc = playlistManagerGetPlaylist(plm, i);
		if (plc && playlistGetTotal(plc)){
			char *out = convertto8(txt);
			trk = searchPlaylist(vp, plc, mtag, out, -1);
			my_free(out);
			if (trk >= 0){
				//printf("%i '%s':%i\n", i+1, plc->name, trk+1);

				TPLAYLIST2 *pl = getPagePtr(vp, PAGE_PLAYLIST2);
				plm->pr->selectedItem = i;
				vp->displayPlaylist = i;
				playlistPlmSetRenderStart(vp, plm, i);

				plc->pr->selectedItem = trk;
				playlist2SetStartTrack(pl, plc, trk);
				playlistMetaGetMeta(vp, pl->metacb, plc, trk, trk+10);
				setPage(vp, PAGE_PLAYLIST2);

				break;
			}
		}
		i = playlistManagerGetPlaylistNext(plm, i);
	}
	
	return 1;
}

int editBoxDoSearchPlaylist (TVLCPLAYER *vp, const int mtag, const wchar_t *txt, const size_t ilen)
{
	int trk = -1;
	PLAYLISTCACHE *plcD = getDisplayPlaylist(vp);
	
	if (playlistGetTotal(plcD)){
		char *out = convertto8(txt);
		trk = searchPlaylist(vp, plcD, mtag, out, plcD->pr->selectedItem);
		my_free(out);
				
		if (trk >= 0){
			TPLAYLIST2 *pl = getPagePtr(vp, PAGE_PLAYLIST2);
		
			plcD->pr->selectedItem = trk;
			playlist2SetStartTrack(pl, plcD, trk);
						
			if (getPage(vp) == PAGE_OVERLAY || getPage(vp) == PAGE_NONE)
				startPlaylistTrack(vp, plcD, trk);
			else
				playlistMetaGetMeta(vp, pl->metacb, plcD, trk, trk+10);
		}
	}
	return trk;
}

int editBoxDoSearch (TVLCPLAYER *vp, const int mtag, const wchar_t *txt, const size_t ilen)
{
	if (getPage(vp) == PAGE_PLAYLISTPLM)
		return searchPlaylistPlm(vp, vp->plm, mtag, txt, ilen);
	else
		return editBoxDoSearchPlaylist(vp, mtag, txt, ilen);
}

TEDITBOXCMD *editBoxGetCmd (TEDITBOX *input, wchar_t *cmdName)
{
	TEDITBOXCMD *cmd = input->registeredCmds;
	
	for (int i = 0; i < EDITBOXCMD_MAXCMDS && cmd->state; i++){
		if (!wcsncmp(cmd->name, cmdName, EDITBOXCMD_MAXCMDLEN))
			return cmd;
		else if (*cmd->alias && !wcsncmp(cmd->alias, cmdName, EDITBOXCMD_MAXCMDLEN))
			return cmd;
		cmd++;
	}
	return NULL;
}

int editBoxCmdExecute (TEDITBOX *input, wchar_t *cmdName, int clen, wchar_t *var, int vlen)
{
	TEDITBOXCMD *cmd = editBoxGetCmd(input, cmdName);
	if (cmd){
		wprintf(L"found cmd '%s'\n", cmd->name);
		
		cmd->pfunc(var, vlen, cmd->uptr, cmd->data1, cmd->data2);
		return 1;
	}
	return 0;
}

int editboxParseCommands (TEDITBOX *input, TVLCPLAYER *vp, wchar_t *txt, int ilen)
{
	if (ilen < 1) return -1;
	PLAYLISTCACHE *plcD = getDisplayPlaylist(vp);
		
	/*
	#n == jump to track/list n
	#abc == a command
	*/
	
	/*
	plm->pr->selectedItem = item->playlistPosition;
	vp->displayPlaylist = plm->pr->selectedItem;
	*/
	
	if (iswdigit(*txt)){		// jump to #nnn
		wchar_t *end = L"\0\0";
		int trk = wcstol(txt, &end, 0);
		
		if (getPage(vp) == PAGE_PLAYLISTPLM){
			if (trk > 0 && trk <= playlistManagerGetTotal(vp->plm)){
				vp->plm->pr->selectedItem = playlistManagerGetPlaylistNext(vp->plm, trk-2);
				vp->displayPlaylist = vp->plm->pr->selectedItem;
				playlistPlmSetRenderStart(vp, vp->plm, vp->plm->pr->selectedItem);
			}
		}else{
			if (trk > 0 && trk <= playlistGetTotal(plcD)){
				TPLAYLIST2 *pl = getPagePtr(vp, PAGE_PLAYLIST2);
			
				plcD->pr->selectedItem = --trk;						
				playlist2SetStartTrack(pl, plcD, trk);

				if (getPage(vp) == PAGE_OVERLAY)
					startPlaylistTrack(vp, plcD, trk);
				else
					playlistMetaGetMeta(vp, pl->metacb, plcD, trk, trk+10);			
			}
		}
		
		
	}else{					// execute command
		wchar_t *cmd = wcstok(txt, L" :");
		if (cmd && *cmd){
			int vlen = 0;
			const int clen = wcslen(cmd);
			wchar_t *var = wcstok(NULL, L"\0");
			if (var && *var){
				var = removeLeadingSpacesW(var);
				vlen = wcslen(var);
			}
			//printf("#%s# #%s#\n",cmd, var);
			editBoxCmdExecute(input, cmd, clen, var, vlen);
		}
	}
	
	return -1;
}

int exitboxProcessString (TEDITBOX *input, wchar_t *txt, int ilen, void *ptr)
{
	TVLCPLAYER *vp = (TVLCPLAYER*)ptr;

	// check if its a command, if not then assume a search query

	if (ilen){
		txt = removeLeadingSpacesW(txt);
		txt = removeTrailingSpacesW(txt);
	}

	//dbprintfW(vp, L"%s", txt);

	if (*txt == CMDPARSER_CMDIDENT && ilen > 1)
		return editboxParseCommands(input, vp, ++txt, --ilen);
	else if (*txt == ':')
		return 0;
	else
		return editBoxDoSearch(vp, -1, txt, ilen);
}

int editBoxRegisterCmdFunc (TEDITBOX *input, wchar_t *_cmdName, void *pfunc, void *uptr, int data1, int data2)
{
	TEDITBOXCMD *cmd = &input->registeredCmds[input->registeredCmdTotal++];
	if (input->registeredCmdTotal >= EDITBOXCMD_MAXCMDS){
		//printf("editBoxRegisterCmdFunc failed due to lack of cmd space (%i)\n", EDITBOXCMD_MAXCMDS);
		return 0;
	}

	wchar_t *cmdName = my_wcsdup(_cmdName);
	if (!cmdName) return 0;
	
	wchar_t *name = wcstok(cmdName, L" ,");
	if (!name){
		my_free(cmdName);
		return 0;
	}
	wcsncpy(cmd->name, name, EDITBOXCMD_MAXCMDLEN);
	cmd->name[EDITBOXCMD_MAXCMDLEN-1] = 0;

	wchar_t *alias = wcstok(NULL, L" ,");	
	if (alias && *alias){
		wcsncpy(cmd->alias, alias, EDITBOXCMD_MAXCMDLEN);
		cmd->alias[EDITBOXCMD_MAXCMDLEN-1] = 0;
	}else{
		cmd->alias[0] = 0;
	}

	cmd->pfunc = pfunc;
	cmd->uptr = uptr;
	cmd->data1 = data1;
	cmd->data2 = data2;
	cmd->state = 1;
	my_free(cmdName);
	return 1;
}

void editboxCmd_sort (wchar_t *var, int vlen, void *uptr, int direction, int unused)
{
	TVLCPLAYER *vp = (TVLCPLAYER*)uptr;
	PLAYLISTCACHE *plcD = getDisplayPlaylist(vp);
	
	var = wcstok(var, L" ");
	if (!var) return;
	vlen = wcslen(var);
	if (vlen < 4) return;
	
	//wprintf(L"editboxCmdSort: #%s# %i %i\n", var, vlen, direction);
	playlistSort(plcD, vp->tagc, tagLookupW(var), direction);
}

void editboxCmd_mctrl (wchar_t *var, int vlen, void *uptr, int op, int unused)
{
	TVLCPLAYER *vp = (TVLCPLAYER*)uptr;
	PLAYLISTCACHE *plcD = getDisplayPlaylist(vp);
	
	switch (op){
	  case VBUTTON_PRETRACK:
	  case VBUTTON_NEXTTRACK:
	  case VBUTTON_STOP:
	  	obuttonCB(NULL, NULL, op, 0, uptr);
		break;

	  case VBUTTON_PLAY:{
	  	var = wcstok(var, L" ");
		if (vlen && var && *var){
	  		wchar_t *end = L"\0\0";
			int trk = wcstol(var, &end, 0);

			if (trk > 0 && trk <= playlistGetTotal(plcD)){
				trk--;
				
				playlist2SetStartTrack(getPagePtr(vp, PAGE_PLAYLIST2), plcD, trk);
				if (startPlaylistTrack(vp, plcD, trk)){
					vp->queuedPlaylist = vp->displayPlaylist;
					PLAYLISTCACHE *plcQ = getQueuedPlaylist(vp);
					
					plcQ->pr->selectedItem = -1;
					plcQ->pr->playingItem = trk;
				}
			}
	  	}else{
	  		buttonPlOvr(NULL, NULL, PLOBUTTON_PLAY, 0, uptr);
	  	}
	  	break;
	  }
	  case VBUTTON_PAUSE:
	  	buttonPlOvr(NULL, NULL, PLOBUTTON_PAUSE, 0, uptr);
	  	break; 
	  	
	  case VBUTTON_SMETA:
	  	buttonPlOvr(NULL, NULL, PLOBUTTON_META, 0, uptr);
	  	break;
	}
}

void editboxCmd_shutdown (wchar_t *var, int vlen, void *uptr, int unused1, int unused2)
{
	TVLCPLAYER *vp = (TVLCPLAYER*)uptr;
	setPage(vp, PAGE_NONE);
  	setPageSec(vp, -1);
  	exitAppl(vp);
}

void editboxCmd_tetris (wchar_t *var, int vlen, void *uptr, int unused1, int unused2)
{
	TVLCPLAYER *vp = (TVLCPLAYER*)uptr;
	//setPage(vp, PAGE_NONE);
	setPageSec(vp, PAGE_TETRIS);
}

void editboxCmd_rc (wchar_t *var, int vlen, void *uptr, int unused1, int unused2)
{
	TVLCPLAYER *vp = (TVLCPLAYER*)uptr;
	setPage(vp, PAGE_NONE);
	setPageSec(vp, PAGE_RC);
}

void editboxCmd_particles (wchar_t *var, int vlen, void *uptr, int unused1, int unused2)
{
	TVLCPLAYER *vp = (TVLCPLAYER*)uptr;
	setPage(vp, PAGE_NONE);
	setPageSec(vp, PAGE_PARTICLES);
}

void editboxCmd_lorenz (wchar_t *var, int vlen, void *uptr, int unused1, int unused2)
{
	TVLCPLAYER *vp = (TVLCPLAYER*)uptr;
	setPage(vp, PAGE_NONE);
	setPageSec(vp, PAGE_LORENZ);
}

void editboxCmd_clock (wchar_t *var, int vlen, void *uptr, int unused1, int unused2)
{
	TVLCPLAYER *vp = (TVLCPLAYER*)uptr;
	setPageSec(vp, PAGE_CLOCK);
}

void editboxCmd_config (wchar_t *var, int vlen, void *uptr, int unused1, int unused2)
{
	TVLCPLAYER *vp = (TVLCPLAYER*)uptr;
	setPageSec(vp, -1);
	setPage(vp, PAGE_CFG);
}

void editboxCmd_closePages (wchar_t *var, int vlen, void *uptr, int unused1, int unused2)
{
	TVLCPLAYER *vp = (TVLCPLAYER*)uptr;
	setPageSec(vp, -1);
	setPage(vp, PAGE_NONE);
}

void editboxCmd_fps (wchar_t *var, int vlen, void *uptr, int unused1, int unused2)
{
	TVLCPLAYER *vp = (TVLCPLAYER*)uptr;

	wchar_t *state = wcstok(var, L" ");
	if (state && *state){
		if (!wcscmp(state, L"on")){
			buttonEnable(vp, PAGE_CFG, CFGBUTTON_FPS_ON);
	  		buttonDisable(vp, PAGE_CFG, CFGBUTTON_FPS_OFF);
	  		enableFPS(vp);
	  		
		}else if (!wcscmp(state, L"off")){
			buttonEnable(vp, PAGE_CFG, CFGBUTTON_FPS_OFF);
	  		buttonDisable(vp, PAGE_CFG, CFGBUTTON_FPS_ON);
	  		disableFPS(vp);
	  		
		}else{
			wchar_t *end = L"\0\0";
			int fps = wcstol(state, &end, 0);
			if (fps > 0 && fps <= 100){
				vp->gui.targetFPS = fps;
				startRefreshTicker(vp, (float)fps);
			}		
		}
	}
}

void editboxCmd_closeEditbox (wchar_t *var, int vlen, void *uptr, int unused1, int unused2)
{
	kHookOff();
	kHookUninstall();
}

void editboxCmd_mouse (wchar_t *var, int vlen, void *uptr, int unused1, int unused2)
{
	TVLCPLAYER *vp = (TVLCPLAYER*)uptr;
	
	wchar_t *state = wcstok(var, L" ");
	if (state && *state){
		if (!wcscmp(state, L"on")){
			if (!mHookGetState()){
  				mHookInstall(vp->gui.hMsgWin, vp);
  				captureMouse(vp, 1);
			}
		}else if (!wcscmp(state, L"off")){
			if (mHookGetState()){
  				mHookUninstall(vp->gui.hMsgWin, vp);
	  			captureMouse(vp, 0);
			}
		}
	}
}

void editboxCmd_aspectRatio (wchar_t *var, int vlen, void *uptr, int unused1, int unused2)
{
	TVLCPLAYER *vp = (TVLCPLAYER*)uptr;
	wchar_t *ar = wcstok(var, L" ");
	
	if (ar && *ar){
		if (!wcscmp(ar, L"auto")){
			setAR(vp, CFGBUTTON_AR_AUTO);
			dbprintf(vp, "auto aspect correction enabled");
			
		}else if (!wcscmp(ar, L"16:9") || !wcscmp(ar, L"1.77")){
			setAR(vp, CFGBUTTON_AR_177);
			dbprintf(vp, "aspect set to 16:9 (1.77)");
			
		}else if (!wcscmp(ar, L"14:9") || !wcscmp(ar, L"1.55")){
			setAR(vp, CFGBUTTON_AR_155);
			dbprintf(vp, "aspect set to 14:9 (1.55)");
			
		}else if (!wcscmp(ar, L"4:3") || !wcscmp(ar, L"1.33")){
			setAR(vp, CFGBUTTON_AR_133);
			dbprintf(vp, "aspect set to 4:3 (1.33)");
				
		}else if (!wcscmp(ar, L"5:4") || !wcscmp(ar, L"1.25")){
			setAR(vp, CFGBUTTON_AR_125);
			dbprintf(vp, "aspect set to 5:4 (1.25)");
				
		}else if (!wcscmp(ar, L"22:18") || !wcscmp(ar, L"1.22")){
			setAR(vp, CFGBUTTON_AR_122);
			dbprintf(vp, "aspect set to 22:18 (1.22)");
				
		}else if (!wcscmp(ar, L"3:2") || !wcscmp(ar, L"1.5")){
			setAR(vp, CFGBUTTON_AR_15);
			dbprintf(vp, "aspect set to 3:2 (1.5)");
				
		}else if (!wcscmp(ar, L"16:10") || !wcscmp(ar, L"1.6")){
			setAR(vp, CFGBUTTON_AR_16);
			dbprintf(vp, "aspect set to 16:10 (1.6)");
				
		}else if (!wcscmp(ar, L"5:3") || !wcscmp(ar, L"1.67")){
			setAR(vp, CFGBUTTON_AR_167);
			dbprintf(vp, "aspect set to 5:3 (1.67)");
				
		}else if (!wcscmp(ar, L"3:7") || !wcscmp(ar, L"1.85")){
			setAR(vp, CFGBUTTON_AR_185);
			dbprintf(vp, "aspect set to 3:7 (1.85)");
				
		}else if (!wcscmp(ar, L"11:5") || !wcscmp(ar, L"2.2") || !wcscmp(ar, L"2.20")){
			setAR(vp, CFGBUTTON_AR_220);
			dbprintf(vp, "aspect set to 11:5 (2.20)");

		}else if (!wcscmp(ar, L"47:20") || !wcscmp(ar, L"2.35")){
			setAR(vp, CFGBUTTON_AR_235);
			dbprintf(vp, "aspect set to 47:20 (2.35)");
				
		}else if (!wcscmp(ar, L"12:5") || !wcscmp(ar, L"2.39") || !wcscmp(ar, L"2.4") || !wcscmp(ar, L"2.40")){
			setAR(vp, CFGBUTTON_AR_240);
			dbprintf(vp, "aspect set to 12:5 (2.40)");
		}
	}
}

void editboxCmd_rgbswap (wchar_t *var, int vlen, void *uptr, int unused1, int unused2)
{
	TVLCPLAYER *vp = (TVLCPLAYER*)uptr;
	
	wchar_t *state = wcstok(var, L" ");
	if (state && *state){
		if (!wcscmp(state, L"on")){
			setRBSwap(vp, 1);
			dbwprintf(vp, L"Video R/B swap enabled");
			
		}else if (!wcscmp(state, L"off")){
			setRBSwap(vp, 0);
			dbwprintf(vp, L"Video R/B swap disabled");
		}
	}
}

void editboxCmd_search (wchar_t *var, int vlen, void *uptr, int unused1, int unused2)
{
	TVLCPLAYER *vp = (TVLCPLAYER*)uptr;

	wchar_t *tagstr = wcstok(var, L" ");
	if (tagstr && *tagstr){
		const int mtag = tagLookupW(tagstr);
		if (mtag != -1)	{
			wchar_t *query = wcstok(NULL, L"\0");
			if (query && *query){
				int qlen = wcslen(query);
				if (qlen){
					dbwprintf(vp, L"searching for %s: %s", getTagW(mtag), query);
					int trk = editBoxDoSearch(vp, mtag, query, qlen);
					if (trk >= 0){
						//dbwprintf(vp, L"found '%s:%s' at track %i", getTagW(mtag), query, trk);
						dbwprintf(vp, L"found %s: %s", getTagW(mtag), query);
					}
				}
			}
		}
	}
}

void editboxCmd_retrieveAllMeta (wchar_t *var, int vlen, void *uptr, int unused1, int unused2)
{
	TVLCPLAYER *vp = (TVLCPLAYER*)uptr;
	TPLAYLIST2 *pl = getPagePtr(vp, PAGE_PLAYLIST2);
	PLAYLISTCACHE *plc = getDisplayPlaylist(vp);
	if (!plc) plc = getPrimaryPlaylist(vp);

	if (plc){
		int trks = playlistGetTotal(plc);
		if (trks > 0){
 			dbprintf(vp, "initiating meta retrieval for %i trks in playlist '%s'", trks, plc->name);
			captureMouse(vp, 0);
			lSleep(20);
			playlistMetaGetMeta(vp, pl->metacb, plc, 0, trks-1);
			countItems(vp, pl->metacb);
		}else{
			dbprintf(vp, "playlist '%s' is empty", plc->name);
		}
	}else{
		dbprintf(vp, "no available playlists found");
	}
}

void editboxCmd_artScale (wchar_t *asize, int vlen, void *uptr, int unused1, int unused2)
{
	TVLCPLAYER *vp = (TVLCPLAYER*)uptr;

	if (asize && *asize){
		wchar_t *end = L"\0\0";
		float scale = (float)wcstol(asize, &end, 0);
		if (scale >= 0 && scale <= 20){
			TPLAYLIST2 *pl = getPagePtr(vp, PAGE_PLAYLIST2);
			pl->artScale = scale/10.0;
			if (!(int)scale)
				dbprintf(vp, "art disabled", (int)scale);
			else
				dbprintf(vp, "artsize set to %i", (int)scale);
		}
	}
}

#if !G19DISPLAY
void editboxCmd_usbd480Backlight (wchar_t *var, int vlen, void *uptr, int unused1, int unused2)
{
	TVLCPLAYER *vp = (TVLCPLAYER*)uptr;

	//var = wcstok(var, L" ");
	if (var && *var){
		wchar_t *end = L"\0\0";
		int level = (int)wcstol(var, &end, 0);
		if (level >= 0 && level <= 255)
			setDisplayBrightness(vp, level);
	}
}
#endif

void editboxCmd_title (wchar_t *var, int vlen, void *uptr, int unused1, int unused2)
{
	TVLCPLAYER *vp = (TVLCPLAYER*)uptr;
	TCHAPTER *chapt = getPagePtr(vp, PAGE_CHAPTERS);
			
	//var = wcstok(var, L" ");
	if (var && *var){
		wchar_t *end = L"\0\0";
		int title = (int)wcstol(var, &end, 0);
		if (title > 0 && title <= 100){
			dbwprintf(vp, L"Setting title %i of %i\n", title, chapt->ttitles);
			vlc_setTitle(vp->vlc, title-1);
		}
	}else{
		dbwprintf(vp, L"Titles: %i\tChapters: %i\n",  chapt->ttitles, chapt->tchapters);
	}
}

void editboxCmd_chapter (wchar_t *var, int vlen, void *uptr, int unused1, int unused2)
{
	TVLCPLAYER *vp = (TVLCPLAYER*)uptr;
	TCHAPTER *chapt = getPagePtr(vp, PAGE_CHAPTERS);
			
	//var = wcstok(var, L" ");
	if (var && *var){
		wchar_t *end = L"\0\0";
		int chapter = (int)wcstol(var, &end, 0);
		if (chapter > 0 && chapter <= 100){
			dbwprintf(vp, L"Setting chapter %i of %i\n", chapter, chapt->tchapters);
			chapt->schapter = chapter;
			vlc_setChapter(vp->vlc, chapter-1);
		}
	}else{
		dbwprintf(vp, L"Titles: %i\tChapters: %i\n",  chapt->ttitles, chapt->tchapters);
	}
}


void editboxCmd_skin (wchar_t *var, int vlen, void *uptr, int unused1, int unused2)
{
	TVLCPLAYER *vp = (TVLCPLAYER*)uptr;
	
	var = wcstok(var, L" ");
	if (var && *var){
		if (!wcscmp(var, L"reload")){
			dbwprintf(vp, L"Reloading skin...");
			reloadSkin(vp);	
			dbwprintf(vp, L"Reloading skin complete");
			
		}else if (!wcscmp(var, L"next")){
	  		if (++vp->gui.skin >= TOTALSKINS)
				vp->gui.skin = 0;
				
			dbwprintf(vp, L"Loading next skin...");
	  		reloadSkin(vp);
	  		dbwprintf(vp, L"Loading skin complete");
		}
	}
}

void editboxCmd_idle (wchar_t *var, int vlen, void *uptr, int unused1, int unused2)
{
	TVLCPLAYER *vp = (TVLCPLAYER*)uptr;
	
	wchar_t *state = wcstok(var, L" ");
	if (state && *state){
		if (!wcscmp(state, L"on")){
			vp->gui.idleDisabled = 0;
			
		}else if (!wcscmp(state, L"off")){
			vp->gui.idleDisabled = 1;
		}
	}
}

void editboxCmd_load (wchar_t *mrl, int mlen, void *uptr, int play, int unused2)
{
	TVLCPLAYER *vp = (TVLCPLAYER*)uptr;
	TPLAYLIST2 *pl = getPagePtr(vp, PAGE_PLAYLIST2);
	PLAYLISTCACHE *plcD = getDisplayPlaylist(vp);
	
	if (plcD)
		printf("editboxCmd_load '%s' ", plcD->name);
	wprintf(L" '%s'\n", mrl);
	
	
	if (mrl && mlen){
		const int from = playlistGetTotal(plcD);
		
		if (isDirectoryW(mrl)){
			if (!plcD){
				char *out = convertto8(mrl);
				plcD = playlistManagerCreatePlaylist(vp->plm, out);
				if (plcD){
					TFBROWSER *browser = getPagePtr(vp, PAGE_BROWSER);
					setExtFilter(vp, browser, EXT_MEDIA);
					browserImportPlaylistByDirW(browser, mrl, plcD, 1);
					
					if (!playlistGetTotal(plcD))
						playlistManagerDeletePlaylist(vp->plm, plcD);
				}
				my_free(out);
			}else{
				TFBROWSER *browser = getPagePtr(vp, PAGE_BROWSER);
				setExtFilter(vp, browser, EXT_MEDIA);
				browserImportPlaylistByDirW(browser, mrl, plcD, 1);
			}
		}else{
			if (!plcD){
				dbprintf(vp, "no playlists available");
				dbprintf(vp, "create a playlist first. Eg; '#plm new <name of playlist>'");
				return;
			}
			
			char *path = convertto8(mrl);
			const int pos = playlistAdd(plcD, path);
			if (pos >= 0){
				playlistChangeEvent(vp, plcD, pl, pos);
				playlist2SetStartTrack(pl, plcD, pos);
				if (play)
					startPlaylistTrack(vp, plcD, pos);
			}
			my_free(path);

		}
		
		const int to = playlistGetTotal(plcD);
		if (from != to)
			playlistMetaGetMeta(vp, pl->metacb, plcD, from, 32);
	}
}

void editboxCmd_open (wchar_t *mrl, int vlen, void *uptr, int unused1, int unused2)
{
	TVLCPLAYER *vp = (TVLCPLAYER*)uptr;

	//wchar_t *mrl = wcstok(var, L" ");
	if (mrl && *mrl){
		editboxCmd_load(mrl, wcslen(mrl), vp, 1, 0);
		
	}else{
		setPageSec(vp, -1);
		setPage(vp, PAGE_BROWSER);
	}
}

int copyPlaylistTracks (TPLAYLISTMANAGER *plm, PLAYLISTCACHE *plcF, int from, int to, PLAYLISTCACHE *plcT, const int copyMode)
{
	if (!plcF || !plcT) return 0;
	
	int itemsCopied = 0;
	
	if (playlistLock(plcF)){
		for (int i = from; i <= to; i++){
			TPLAYLISTITEM *item = playlistGetItem(plcF, i);
			if (item && item->path){
				
				if (copyMode){	// copyMode != 0, = we don't want duplicates
					if (playlistGetPositionByHash(plcT, item->hash) >= 0)
						continue;
				}

				int pos = playlistAdd(plcT, item->path);
				if (pos >= 0){
					if (item->title)
						playlistSetTitle(plcT, pos, item->title, 1);
					itemsCopied++;
				}
			}
		}
		playlistUnlock(plcF);
	}

	return itemsCopied;
}

int copyPlaylist (TPLAYLISTMANAGER *plm, int from, int to, const int copyMode)
{
	PLAYLISTCACHE *plcF = playlistManagerGetPlaylist(plm, --from);
	PLAYLISTCACHE *plcT = playlistManagerGetPlaylist(plm, --to);
	return copyPlaylistTracks(plm, plcF, 0, playlistGetTotal(plcF)-1, plcT, copyMode);
}

int playlistExcludeRecordsByFilter (TVLCPLAYER *vp, PLAYLISTCACHE *src, PLAYLISTCACHE *to, const int tag, const char *filter)
{
	playlistLock(src);
	playlistLock(to);
	
	TPLAYLISTITEM *item;
	char buffer[MAX_PATH_UTF8];
	int total = playlistGetTotal(src);
	int newTotal = 0;
	
	for (int i = 0; i < total; i++){
		item = playlistGetItem(src, i);	
		if (item){
			*buffer = 0;
			
			if (tag == MTAG_PATH)
				strncpy(buffer, item->path, MAX_PATH_UTF8);
			else
				tagRetrieveByHash(vp->tagc, item->hash, tag, buffer, MAX_PATH_UTF8);

			if (*buffer){
				if (!stristr(buffer, filter)){
					int pos = playlistAdd(to, item->path);
					if (pos >= 0 && item->title)
						playlistSetTitle(to, pos, item->title, 1);
					newTotal++;
				}
			}
		}
	}
	
	playlistUnlock(to);
	playlistUnlock(src);
	
	return newTotal;
}

int playlistIncludeRecordsByFilter (TVLCPLAYER *vp, PLAYLISTCACHE *plc, PLAYLISTCACHE *to, const int tag, const char *filter)
{
	playlistLock(plc);
	//playlistLock(to);

	TPLAYLISTITEM *item;
	char buffer[MAX_PATH_UTF8+1];
	int itemsRemoved = 0;
	int total = playlistGetTotal(plc);
	
	for (int i = 0; i < total; i++){
		item = playlistGetItem(plc, i);
		if (item){
			*buffer = 0;
			
			if (tag == MTAG_PATH)
				strncpy(buffer, item->path, MAX_PATH_UTF8);
			else
				tagRetrieveByHash(vp->tagc, item->hash, tag, buffer, MAX_PATH_UTF8);
			
			if (*buffer){
				if (stristr(buffer, filter)){
					if (plc != to){
						int pos = playlistAdd(to, item->path);
						if (pos >= 0 && item->title)
							playlistSetTitle(to, pos, item->title, 1);
					}else{
						playlistDeleteRecord(plc, i--);
						total = playlistGetTotal(plc);
					}
					itemsRemoved++;
				}
			}
		}
	}
	
	//playlistUnlock(to);
	playlistUnlock(plc);
	
	return itemsRemoved;
}

int playlistBuildSplitPlaylists (TVLCPLAYER *vp, PLAYLISTCACHE *plc, const int mtag)
{

	if (!playlistLock(plc))
		return 0;

	TPLAYLISTITEM *item;
	PLAYLISTCACHE *to = NULL;
	char buffer[MAX_PATH_UTF8+1];
	const int total = playlistGetTotal(plc);
	int plcount = 1;
	
	PLAYLISTCACHE *allothers = playlistManagerCreatePlaylist(vp->plm, "_unfiltered_");
	//playlistDelete(allothers);
	
	for (int i = 0; i < total; i++){
		item = playlistGetItem(plc, i);	
		if (item){
			buffer[0] = 0;
			
			if (mtag == MTAG_PATH){
				if (item->path)
					strncpy(buffer, item->path, MAX_PATH_UTF8);
			}else if (mtag == MTAG_Title){
				if (item->title)
					strncpy(buffer, item->title, MAX_PATH_UTF8);
			}else{
				tagRetrieveByHash(vp->tagc, item->hash, mtag, buffer, MAX_PATH_UTF8);
			}

			if (*buffer){
				char *name = removeLeadingSpaces(buffer);
				name = removeTrailingSpaces(name);
				
				to = playlistManagerGetPlaylistByName(vp->plm, name);
				if (!to){
					to = playlistManagerCreatePlaylist(vp->plm, name);
					if (!to) break;
					plcount++;
				}
			}else{
				to = allothers;
				//printf("%i ##%s##\n", i, item->path);
			}
			
			int pos = playlistAdd(to, item->path);
			if (pos >= 0 && item->title){
				playlistSetTitle(to, pos, item->title, 1);
			}
		}
	}

	if (!playlistGetTotal(allothers)){
		playlistManagerDeletePlaylist(vp->plm, allothers);
		plcount--;
	}
	playlistUnlock(plc);
	
	return plcount;
}

void editboxCmd_playlist (wchar_t *var, int vlen, void *uptr, int unused1, int unused2)
{
	TVLCPLAYER *vp = (TVLCPLAYER*)uptr;
	TPLAYLIST2 *pl = getPagePtr(vp, PAGE_PLAYLIST2);
	PLAYLISTCACHE *plcD = getDisplayPlaylist(vp);
						
	if (!var || !vlen){
		setPage(vp, PAGE_PLAYLIST2);
		return;
	}
	wchar_t *state = wcstok(var, L" ");
	if (!state) return;

	if (!wcscmp(state, L"up")){
		buttonPlOvr(NULL, NULL, PLOBUTTON_NAVUP, 0, vp);
			
	}else if (!wcscmp(state, L"down") || !wcscmp(state, L"dn")){
		buttonPlOvr(NULL, NULL, PLOBUTTON_NAVDN, 0, vp);
		
	}else if (!wcscmp(state, L"prune")){
		int removed = playlistPrune(plcD, 1);
		dbprintf(vp, "%i items removed\n", removed);
		
	}else if (!wcscmp(state, L"setname")){
		var = wcstok(NULL, L"\0");
		if (var && *var){
			var = removeLeadingSpacesW(var);
			removeTrailingSpacesW(var);

			char *out = convertto8(var);
			if (out){
				char oldname[MAX_PATH_UTF8];
				playlistGetName(plcD, oldname, MAX_PATH_UTF8);

				if (playlistSetName(plcD, out))
					dbprintf(vp, "'%s' renamed to '%s'", oldname, out);
				my_free(out);
			}
		}

	}else if (!wcscmp(state, L"copy")){
		wchar_t *end = L"\0\0";
		int from = -1, to = -1;
			
		// get tracks from
		var = wcstok(NULL, L"- ");
		if (var && *var++ == CMDPARSER_NUMIDENT){
			from = (int)wcstol(var, &end, 0);
		}

		// compute to
		var = wcstok(NULL, L" ");
		if (*var != CMDPARSER_NUMIDENT){
			to = (int)wcstol(var, &end, 0);
			var = wcstok(NULL, L" \0");
		}else{
			to = from;
		}
	
		// get dst playlist 
		if (var && *var++ == CMDPARSER_NUMIDENT){
			int destPl = (int)wcstol(var, &end, 0);
			//printf("%i %i %i\n", from, to, destPl);
			
			PLAYLISTCACHE *plc = playlistManagerGetPlaylist(vp->plm, --destPl);
			const int plcDtotal = playlistGetTotal(plcD);
			
			if (plc && from > 0 && to > 0 && from <= plcDtotal && to <= plcDtotal){
				copyPlaylistTracks(vp->plm, plcD, --from, --to, plc, 0);
			}
		}
	
	}else if (!wcscmp(state, L"clear")){
		PLAYLISTCACHE *plcQ = getQueuedPlaylist(vp);
			
		if (plcQ == plcD){
			vp->queuedPlaylist = -1;
				
			TVLCCONFIG *vlc = getConfig(vp);
			if (getPlayState(vp))
				player_stop(vp, vlc);
			if (vlc->isMediaLoaded)
				unloadMedia(vp, vlc);
		}
		playlistDelete(plcD);

	}else if (!wcscmp(state, L"del") || !wcscmp(state, L"delete")){
		var = wcstok(NULL, L"-");
		
		if (var && *var == CMDPARSER_WILDIDENT){
			int tRecs = playlistGetTotal(plcD);
			while(tRecs--)
				playlistDeleteRecord(plcD, 0);

		}else if (var && *var == CMDPARSER_NUMIDENT){
			wchar_t *end = L"\0\0";
			int from = (int)wcstol(++var, &end, 0);

			var = wcstok(NULL, L"\0");
			if (var){
				int to = (int)wcstol(var, &end, 0);
					
				if (from > 0 && from <= playlistGetTotal(plcD) && to >= from && to <= playlistGetTotal(plcD)){
					int tRecs = (to - from--) + 1;
					while(tRecs--)
						playlistDeleteRecord(plcD, from);
				}
			}else{
				if (from > 0 && from <= playlistGetTotal(plcD))
					playlistDeleteRecord(plcD, from-1);
			}
		}

		if (plcD->pr->selectedItem >= 0)
			playlistChangeEvent(vp, plcD, pl, plcD->pr->selectedItem);
		else
			playlistChangeEvent(vp, plcD, pl, plcD->pr->playingItem);

	}else if (!wcscmp(state, L"add")){
		wchar_t *mrl = wcstok(NULL, L"\0");
		if (mrl && *mrl)
			editboxCmd_load(mrl, wcslen(mrl), vp, 0, 0);

	}else if (!wcscmp(state, L"save") || !wcscmp(state, L"wr")){
		var = wcstok(NULL, L" ");
		if (var && *var++ == CMDPARSER_NUMIDENT){
			wchar_t *end = L"\0\0";
			int plIdx = (int)wcstol(var, &end, 0);
			if (plIdx > 0 && plIdx <= vp->plm->total){

				PLAYLISTCACHE *plc = playlistManagerGetPlaylist(vp->plm, --plIdx);
				if (plc){
					wchar_t *path = wcstok(NULL, L"\0");
					if (path && *path){
						dbwprintf(vp, L"%i tracks saved to '%s'", playlistGetTotal(plc), path);
					
						TM3U *m3u = m3uNew();
						if (m3uOpen(m3u, path, M3U_OPENWRITE)){
							m3uWritePlaylist(m3u, plc, vp->tagc);
							m3uClose(m3u);
						}
						m3uFree(m3u);
					}
				}
			}
		}
	}else if (!wcscmp(state, L"load") || !wcscmp(state, L"ld")){
		var = wcstok(NULL, L" ");
		if (var && *var++ == CMDPARSER_NUMIDENT){
			wchar_t *end = L"\0\0";
			int plIdx = (int)wcstol(var, &end, 0);
			if (plIdx > 0 && plIdx <= vp->plm->total){

				PLAYLISTCACHE *plc = playlistManagerGetPlaylist(vp->plm, --plIdx);
				if (plc){
					wchar_t *path = wcstok(NULL, L"\0");
					if (path && *path){
						int oldTotal = playlistGetTotal(plc);
						int newTotal = importPlaylistW(vp->plm, plc, vp->tagc, path);
						
						if (newTotal != oldTotal){
							if (plc->pr->selectedItem >= 0)
								playlistChangeEvent(vp, plc, pl, plc->pr->selectedItem);
							else
								playlistChangeEvent(vp, plc, pl, plc->pr->playingItem);
						}
						dbwprintf(vp, L" %i tracks loaded from '%s'", newTotal, path);
						resetCurrentDirectory();
					}
				}
			}else if (plIdx == 0){
				wchar_t *path = wcstok(NULL, L"\0");
				if (path && *path){
					int total = importPlaylistW(vp->plm, NULL, vp->tagc, path);
					dbwprintf(vp, L" %i tracks loaded from '%s'", total, path);
					resetCurrentDirectory();
				}
			}
		}
	}else if (!wcscmp(state, L"remove")){
		wchar_t *filter = wcstok(NULL, L" ");
		if (filter && *filter){
			int mtag = tagLookupW(filter);
			
			wchar_t *forthis = wcstok(NULL, L"\0");
			if (forthis && *forthis){
				char *out = convertto8(forthis);
				int itemsRemoved = playlistIncludeRecordsByFilter(vp, plcD, plcD, mtag, out);
				if (itemsRemoved)
					playlistChangeEvent(vp, plcD, getPagePtr(vp, PAGE_PLAYLIST2), 0);
				dbprintf(vp, "%i tracks removed from '%s'", itemsRemoved, plcD->name);
				my_free(out);
			}
		}		
		
	// filter this only
	}else if (!wcscmp(state, L"exclude")){
		wchar_t *filter = wcstok(NULL, L" ");
		if (filter && *filter){
			int mtag = tagLookupW(filter);
			
			wchar_t *forthis = wcstok(NULL, L"\0");
			if (forthis && *forthis){

				char *out = convertto8(forthis);
				PLAYLISTCACHE *to = playlistManagerCreatePlaylist(vp->plm, out);
				int newTotal = playlistExcludeRecordsByFilter(vp, plcD, to, mtag, out);
				my_free(out);

				if (playlistGetTotal(to)){
					int plIdx = playlistManagerGetPlaylistIndex(vp->plm, to);
					vp->displayPlaylist = plIdx;
					playlistChangeEvent(vp, to, getPagePtr(vp, PAGE_PLAYLIST2), 0);					
					setPage(vp, PAGE_PLAYLIST2);

				}else if (!playlistGetTotal(to)){
					playlistManagerDeletePlaylist(vp->plm, to);
				}
				dbprintf(vp, "%i tracks extracted to '%s' from '%s'", newTotal, to->name, plcD->name);
			}
		}
		
	// filter everything which is not this (everything but this)
	}else if (!wcscmp(state, L"extract")){
		wchar_t *filter = wcstok(NULL, L" ");
		if (filter && *filter){
			int mtag = tagLookupW(filter);
			
			wchar_t *forthis = wcstok(NULL, L"\0");
			if (forthis && *forthis){
				//wprintf(L"filter include #%s# (%i) for #%s#\n", filter, mtag, forthis);

				char *out = convertto8(forthis);
				PLAYLISTCACHE *to = playlistManagerCreatePlaylist(vp->plm, out);
				int newTotal = playlistIncludeRecordsByFilter(vp, plcD, to, mtag, out);
				my_free(out);
					
				if (to->total){
					int plIdx = playlistManagerGetPlaylistIndex(vp->plm, to);
					vp->displayPlaylist = plIdx;
					playlistChangeEvent(vp, to, getPagePtr(vp, PAGE_PLAYLIST2), 0);					
					setPage(vp, PAGE_PLAYLIST2);
							
				}else if (!to->total){
					playlistManagerDeletePlaylist(vp->plm, to);
				}
				dbprintf(vp, "%i tracks extracted to '%s' from '%s'", newTotal, to->name, plcD->name);
			}
		}
			
	}else if (!wcscmp(state, L"decompose") || !wcscmp(state, L"decom") || !wcscmp(state, L"split")){
		wchar_t *filter = wcstok(NULL, L" ");
		if (filter && *filter){
			int mtag = tagLookupW(filter);
			//wprintf(L"filter split #%s# (%i)\n", filter, mtag);
			int total = playlistBuildSplitPlaylists(vp, plcD, mtag);
			if (total)
				dbwprintf(vp, L"%i playlists created from '%s'", total, filter);
		}
	}
}

void editboxCmd_volume (wchar_t *var, int vlen, void *uptr, int unused1, int unused2)
{
	TVLCPLAYER *vp = (TVLCPLAYER*)uptr;

	wchar_t *state = wcstok(var, L" ");
	if (state && *state){
		if (!wcscmp(state, L"on")){
	  		setVolume(vp, 50);
			dbprintf(vp, "volume on (50%)");
			
		}else if (!wcscmp(state, L"off") || !wcscmp(state, L"mute")){
			setVolume(vp, 0);
			dbprintf(vp, "volume disabled");

		}else{
			wchar_t *end = L"\0\0";
			int vol = wcstol(state, &end, 0);
			vol = setVolume(vp, vol);
			dbprintf(vp, "volume set to %i", vol);
		}
	}
}

void editboxCmd_visual (wchar_t *var, int vlen, void *uptr, int unused1, int unused2)
{
	TVLCPLAYER *vp = (TVLCPLAYER*)uptr;

	wchar_t *state = wcstok(var, L" ");
	if (state && *state){
		if (!wcscmp(state, L"on")){
	  		buttonCfg(NULL, NULL, CFGBUTTON_VIS_GOOM_Q3-1, 0, vp);

		}else if (!wcscmp(state, L"off")){
			buttonCfg(NULL, NULL, CFGBUTTON_VIS_OFF, 0, vp);

		}else{
			wchar_t *end = L"\0\0";
			int vis = wcstol(state, &end, 0);
			if (vis >= 0 && vis < VIS_TOTAL)
				buttonCfg(NULL, NULL, CFGBUTTON_VIS_DISABLED+vis-1, 0, vp);
		}
	}
}

// take a snapshot
// remove input box from display, wait for a fresh render pass, take snapshot then re-enable input box
void editboxCmd_snapshot (wchar_t *var, int vlen, void *uptr, int writelog, int unused2)
{
	TVLCPLAYER *vp = (TVLCPLAYER*)uptr;
	TFBROWSER *fb = getPagePtr(vp, PAGE_BROWSER);
	TFRAME *clone = NULL;
	
	renderLock(vp);
	volatile int fps0 = vp->gui.fps;
	renderUnlock(vp);
	
	// force a render pass
	SetEvent(vp->hUpdateEvent);
	
	// wait for render thread to complete a new pass without the edit control
	int ttime = 1000;
	while (ttime > 0 && fps0 == vp->gui.fps){
		float t0 = getTime(vp);
		lSleep(5);
		ttime -= (int)(getTime(vp)-t0);
	}

	// pass complete, now save a snapshot	
	if (renderLock(vp)){
		clone = lCloneFrame(fb->print->frame);
		renderUnlock(vp);
	}	

	if (clone){
		wchar_t *name;
		if (var && *var && vlen)
			name = var;
		else
			name = L"vlcs_snapshot.png";
		
		if (lSaveImage(clone, name, IMG_PNG, 0, 0) && writelog)
			dbwprintf(vp, L"snapshot saved: '%s'", name);
		else if (writelog)
			dbwprintf(vp, L"snapshot failed for '%s'", name);
			
		lDeleteFrame(clone);
	}
}

#if 1

void editboxCmd_list (wchar_t *var, int vlen, void *uptr, int unused1, int unused2)
{
	TVLCPLAYER *vp = (TVLCPLAYER*)uptr;
	TEDITBOX *input = &vp->input;
	TEDITBOXCMD *cmd = input->registeredCmds;
	
	addWorkingBuffer(input);
	nextHistoryBuffer(input);
	clearWorkingBuffer(input);
	editBoxInputProc(input, vp->gui.hMsgWin, ':', 0);
	editBoxInputProc(input, vp->gui.hMsgWin, ' ', 0);

	wchar_t *keys;
	int clen;
	
	for (int i = 0; i < EDITBOXCMD_MAXCMDS && cmd->state; cmd++, i++){
		keys = cmd->name;
		clen = wcslen(keys);

		while (clen--)
			editBoxInputProc(input, vp->gui.hMsgWin, *keys++, 0);

		if (*cmd->alias){
			keys = cmd->alias;
			clen = wcslen(keys);
			
			editBoxInputProc(input, vp->gui.hMsgWin, '(', 0);
			while (clen--)
				editBoxInputProc(input, vp->gui.hMsgWin, *keys++, 0);
			editBoxInputProc(input, vp->gui.hMsgWin, ')', 0);
		}
		editBoxInputProc(input, vp->gui.hMsgWin, ' ', 0);
		editBoxInputProc(input, vp->gui.hMsgWin, ' ', 0);

	}
}
#else

void editboxCmd_list (wchar_t *var, int vlen, void *uptr, int unused1, int unused2)
{
	TVLCPLAYER *vp = (TVLCPLAYER*)uptr;
	TEDITBOX *input = &vp->input;
	TEDITBOXCMD *cmd = input->registeredCmds;

	wchar_t out[512];
	*out = 0;
	int ct = 0;
	
	for (int i = 0; i < EDITBOXCMD_MAXCMDS && cmd->state; cmd++, i++){
		wcscat(out, cmd->name);
		if (*cmd->alias){
			wcscat(out, L"(");
			wcscat(out, cmd->alias);
			wcscat(out, L")");
		}
		wcscat(out, L" ");
		wcscat(out, L" ");
		
		if (ct++ == 2){
			ct = 0;
			dbwprintf(vp, out);
			*out = 0;
		}
	}
	if (ct)
		dbwprintf(vp, out);
}
#endif

void editboxCmd_time (wchar_t *var, int vlen, void *uptr, int unused1, int unused2)
{
	TVLCPLAYER *vp = (TVLCPLAYER*)uptr;

	if (var && *var){
		if (getPlayState(vp) && getPlayState(vp) != 8){
			float tlen = vp->vlc->length;
			float tpos = (float)stringToTimeW(var, vlen);
			float pos = 1.0/(tlen/tpos);
			clipFloat(pos);
			vp->vlc->position = pos;
			vlc_setPosition(vp->vlc, vp->vlc->position);
		}
	}
}

void editboxCmd_timeJump (wchar_t *var, int vlen, void *uptr, int direction, int unused2)
{
	TVLCPLAYER *vp = (TVLCPLAYER*)uptr;
	if (var && *var){
		wchar_t *end = L"\0\0";
		const float tskip = wcstof(var, &end);

		if (tskip > 0.00000){
			if (getPlayState(vp) && getPlayState(vp) != 8){
				float pos = 0.0;
				float tlen = vp->vlc->length;
				
				if (direction == TIMRSKIP_FORWARD)
					pos = vp->vlc->position + ((1.0/tlen) * tskip);
				else if (direction == TIMRSKIP_BACK)
					pos = vp->vlc->position - ((1.0/tlen) * tskip);
					
				clipFloat(pos);
				vp->vlc->position = pos;
				vlc_setPosition(vp->vlc, vp->vlc->position);
			}
		}
	}
}

void dumpPlaylists (TVLCPLAYER *vp, TPLAYLISTMANAGER *plm)
{
	if (playlistManagerLock(plm)){
		PLAYLISTCACHE *plc;
		
		for (int i = 0; i < plm->total; i++){
			plc = playlistManagerGetPlaylist(plm, i);
			if (plc){
				if (playlistLock(plc)){
					dbprintf(vp, "%i (%i) '%s'\n", i, plc->total, plc->name);
					playlistUnlock(plc);
				}
			}
		}
		playlistManagerUnlock(plm);
	}
}

void deletePlaylist (TVLCPLAYER *vp, TPLAYLISTMANAGER *plm, const int from)
{
	const int plIdx = from-1;
	
	PLAYLISTCACHE *plc = playlistManagerGetPlaylist(plm, plIdx);
	if (!plc) return;
				
	if (plc == getQueuedPlaylist(vp)){
		vp->queuedPlaylist = -1;

		TVLCCONFIG *vlc = getConfig(vp);
		if (getPlayState(vp))
			player_stop(vp, vlc);
		if (vlc->isMediaLoaded)
			unloadMedia(vp, vlc);
	}
	if (plc == getDisplayPlaylist(vp)/* && getPage(vp) == PAGE_PLAYLIST2*/){
		vp->displayPlaylist = playlistManagerGetPlaylistNext(vp->plm, vp->displayPlaylist);
		setPage(vp, PAGE_PLAYLISTPLM);
	}
				
	// now remove the playlist
	playlistManagerDeletePlaylist(vp->plm, plc);
}

void deletePlaylists (TVLCPLAYER *vp, TPLAYLISTMANAGER *plm, int from, int to)
{
	for (int i = to; i >= from && i > 0; i--)
		deletePlaylist(vp, plm, i);
}

int plmWritePlaylist (TVLCPLAYER *vp, PLAYLISTCACHE *plc, wchar_t *path)
{
	int ret = 0;
	
	TM3U *m3u = m3uNew();
	if (m3uOpen(m3u, path, M3U_OPENWRITE)){
		ret = m3uWritePlaylist(m3u, plc, vp->tagc);
		m3uClose(m3u);
	}
	m3uFree(m3u);
	
	return ret;
}
	
int plmWriteMultilist (TVLCPLAYER *vp, TPLAYLISTMANAGER *plm, wchar_t *name, const int len)
{
	//if (playlistManagerLock(plm)){
		char buffer[MAX_PATH_UTF8+1];
		wchar_t title[MAX_PATH+1];
		wchar_t wbuffer[MAX_PATH+1];
		wchar_t drive[_MAX_DRIVE+1];
		wchar_t dir[_MAX_DIR+1];
		wchar_t filename[MAX_PATH+1];
		
		_wsplitpath(name, drive, dir, filename, NULL);
		snwprintf(wbuffer, MAX_PATH, L"%s%s%s%s", drive, dir, filename, VLCSPLAYLISTEXTW);
		dbwprintf(vp, L"creating root playlist '%s'", wbuffer);
		
		TM3U *mPLM = m3uNew();
		if (m3uOpen(mPLM, wbuffer, M3U_OPENWRITE)){
			PLAYLISTCACHE *plc;
		
			int plIdx = playlistManagerGetPlaylistNext(plm, -1);
			while (plIdx >= 0){
				plc = playlistManagerGetPlaylist(plm, plIdx);
				
				if (plc && playlistGetTotal(plc)){
					fprintf(mPLM->hFile, "#EXTALB:%s\r\n", playlistGetName(plc, buffer, MAX_PATH_UTF8));
					dbprintf(vp, "adding playlist '%s'", buffer);
					snwprintf(title, MAX_PATH, L"%s-%i%s", filename, plIdx+1, VLCSPLAYLISTEXTW);

					if (UTF16ToUTF8(title, wcslen(title), buffer, MAX_PATH_UTF8)){
						fprintf(mPLM->hFile,"%s\r\n", buffer);
						plmWritePlaylist(vp, plc, title);
					}
				}
				plIdx = playlistManagerGetPlaylistNext(plm, plIdx);
			}
			m3uClose(mPLM);
		}
		
		m3uFree(mPLM);
	//	playlistManagerUnlock(plm);
	//}
	return 0;
}

void editboxCmd_plm (wchar_t *var, int vlen, void *uptr, int unused1, int unused2)
{
	TVLCPLAYER *vp = (TVLCPLAYER*)uptr;
	TPLAYLISTMANAGER *plm = vp->plm;

	if (!var || !vlen){
		setPage(vp, PAGE_PLAYLISTPLM);
		return;
	}
	wchar_t *state = wcstok(var, L" ");
	if (!state) return;
	
	if (!wcscmp(state, L"delete") || !wcscmp(state, L"del")){
		var = wcstok(NULL, L"-");
		
		if (var && *var == CMDPARSER_WILDIDENT){
			if (playlistManagerLock(plm)){
				vp->queuedPlaylist = -1;
				vp->displayPlaylist = -1;
				deletePlaylists(vp, plm, 1, playlistManagerGetTotal(plm));
				playlistManagerUnlock(plm);
			}
			
		}else if (var && *var == CMDPARSER_NUMIDENT){
			int from, to;
			
			wchar_t *end = L"\0\0";
			from = (int)wcstol(++var, &end, 0);
			var = wcstok(NULL, L"\0");
			if (var){
				to = (int)wcstol(var, &end, 0);
				if (to >= from) 
					deletePlaylists(vp, plm, from, to);
			}else{
				deletePlaylists(vp, plm, from, from);
			}
		}
	}else if (!wcscmp(state, L"save")){
		var = wcstok(NULL, L"\0");
		if (var && *var){
			wchar_t *name = removeLeadingSpacesW(var);
			if (name){
				removeTrailingSpacesW(name);
				const int len = wcslen(name);
				if (len)
					plmWriteMultilist(vp, plm, name, len);
			}
		}

	}else if (!wcscmp(state, L"new")){
		char name[MAX_PATH_UTF8];
		
		var = wcstok(NULL, L"\0");
		if (var && *var){
			if (!UTF16ToUTF8(var, wcslen(var), name, MAX_PATH_UTF8-1))
				strcpy(name, "untitled");
		}else{
			strcpy(name, "untitled");
		}
				
		PLAYLISTCACHE *plc = playlistManagerCreatePlaylist(vp->plm, name);
		vp->displayPlaylist = playlistManagerGetPlaylistIndex(vp->plm, plc);
		playlistPlmSetRenderStart(vp, vp->plm, vp->displayPlaylist-1);
		//setPage(vp, PAGE_PLAYLISTPLM);
			
	}else if (!wcscmp(state, L"merge")){
		var = wcstok(NULL, L" ");
		if (var && *var++ == CMDPARSER_NUMIDENT){
			int from, to;
			
			wchar_t *end = L"\0\0";
			from = (int)wcstol(var, &end, 0);
			
			var = wcstok(NULL, L" ");
			if (var && *var++ == CMDPARSER_NUMIDENT){
				to = (int)wcstol(var, &end, 0);
				
				if (from > 0 && to > 0 && from != to && from <= plm->total && to <= plm->total){
					int copyMode = 0;
					
					if ((state=wcstok(NULL, L"\0"))){
						copyMode = (wcsstr(state, L"nodup") != NULL);
						wprintf(L"opt %i '%s'\n", copyMode, state);
					}
					if (copyPlaylist(plm, from, to, copyMode))
						deletePlaylist(vp, plm, from);
				}
			}
		}
	}else if (!wcscmp(state, L"copy")){
		var = wcstok(NULL, L" ");
		if (var && *var++ == CMDPARSER_NUMIDENT){
			int from, to;
			
			wchar_t *end = L"\0\0";
			from = (int)wcstol(var, &end, 0);
			
			var = wcstok(NULL, L" ");
			if (var && *var++ == CMDPARSER_NUMIDENT){
				to = (int)wcstol(var, &end, 0);

				if (from > 0 && to > 0 && from != to && from <= plm->total && to <= plm->total){
					int copyMode = 0;
					
					if ((state=wcstok(NULL, L"\0"))){
						copyMode = (wcsstr(state, L"nodup") != NULL);
						wprintf(L"opt %i '%s'\n", copyMode, state);
					}
					copyPlaylist(plm, from, to, copyMode);
				}
			}
		}
	}else if (!wcscmp(state, L"first")){
		vp->displayPlaylist = playlistManagerGetPlaylistNext(plm, -1);
		playlistPlmSetRenderStart(vp, vp->plm, vp->displayPlaylist);
		
	}else if (!wcscmp(state, L"last")){
		int first = playlistManagerGetPlaylistNext(plm, -1);
		int last = first;
		
		while (first != -1){
			first = playlistManagerGetPlaylistNext(plm, first);
			if (first > last)
				last = first;
		}
		vp->displayPlaylist = last;
		playlistPlmSetRenderStart(vp, vp->plm, vp->displayPlaylist);
		
	}else if (!wcscmp(state, L"forward") || !wcscmp(state, L"next")){
		vp->displayPlaylist = playlistManagerGetPlaylistNext(plm, vp->displayPlaylist);
		playlistPlmSetRenderStart(vp, vp->plm, vp->displayPlaylist);
							
	}else if (!wcscmp(state, L"back") || !wcscmp(state, L"prev")){
		vp->displayPlaylist = playlistManagerGetPlaylistPrev(plm, vp->displayPlaylist);
		playlistPlmSetRenderStart(vp, vp->plm, vp->displayPlaylist);
				
	}else if (!wcscmp(state, L"dump")){
		dumpPlaylists(vp, plm);
		
	}else if (*state == CMDPARSER_NUMIDENT){
		state++;
		if (iswdigit(*state)){		// jump to track #nnn
			wchar_t *end = L"\0\0";
			int playlist = wcstol(state, &end, 0);
			if (playlist > 0 && playlist <= playlistManagerGetTotal(plm)){
			
				plm->pr->selectedItem = --playlist;
				vp->displayPlaylist = playlistManagerGetPlaylistNext(plm, playlist-1);
				playlistPlmSetRenderStart(vp, plm, vp->displayPlaylist);
						
				//if (getPage(vp) == PAGE_OVERLAY || getPage(vp) == PAGE_NONE){
					PLAYLISTCACHE *plc = getDisplayPlaylist(vp);
					if (plc){
						TPLAYLIST2 *pl = getPagePtr(vp, PAGE_PLAYLIST2);
						playlistMetaGetMeta(vp, pl->metacb, plc, 0, 8);
					}
					setPage(vp, PAGE_PLAYLIST2);
				//}
			}
		}
	}
	
}

void editboxCmd_getlengths (wchar_t *var, int vlen, void *uptr, int unused1, int unused2)
{
	TVLCPLAYER *vp = (TVLCPLAYER*)uptr;
	PLAYLISTCACHE *plcD = getDisplayPlaylist(vp);
	const int total = playlistGetTotal(plcD);
	if (!total) return;
	
	unsigned int hash;
	int trk = 0;
	char buffer[64];
	int trklen = 0;

	const int volume = vp->vlc->volume;
	setVolume(vp, 0);

	while(trk < total){
		hash = playlistGetHash(plcD, trk);
		if (hash){
			tagRetrieveByHash(vp->tagc, hash, MTAG_LENGTH, buffer, sizeof(buffer));
			if (*buffer)
				trklen = (int)stringToTime(buffer, sizeof(buffer));

			if (trklen <= 0){
				plcD->pr->playingItem = trk;
				startPlaylistTrack(vp, plcD, trk);
				setVolume(vp, 0);
					
				for (int i = 0; i < 500; i += 10){
					lSleep(10);
					tagRetrieveByHash(vp->tagc, hash, MTAG_LENGTH, buffer, sizeof(buffer));
					if (*buffer){
						trklen = (int)stringToTime(buffer, sizeof(buffer));
						if (trklen > 0) break;
					}
				}
				if (trklen > 0)
					dbprintf(vp, "track %i: %is", trk+1, trklen);
			}
		}
		trklen = 0;
		trk++;
	}
	setVolume(vp, volume);
}

void editboxCmd_flushArt (wchar_t *var, int vlen, void *uptr, int unused1, int unused2)
{
	TVLCPLAYER *vp = (TVLCPLAYER*)uptr;
	TPLAYLIST2 *pl = getPagePtr(vp, PAGE_PLAYLIST2);
		
	tagFlushArt(vp->tagc);
	freeMetaSlots(pl->metacb, 3, 0);
	dbprintf(vp, "artwork flushed");
}

void editboxCmd_stats (wchar_t *var, int vlen, void *uptr, int unused1, int unused2)
{
	TVLCPLAYER *vp = (TVLCPLAYER*)uptr;
	TPLAYLIST2 *pl = getPagePtr(vp, PAGE_PLAYLIST2);
	countItems(vp, pl->metacb);
}

void editboxCmd_about (wchar_t *var, int vlen, void *uptr, int unused1, int unused2)
{
	TVLCPLAYER *vp = (TVLCPLAYER*)uptr;
	dbprintf(vp, " VlcStream by %s ", mySELF);
#if G19DISPLAY
	dbprintf(vp, " Originally designed around the touch input interface ");
	dbprintf(vp, " of the USBD480 (480x272), then ported to the G19 (320x240) ");
#endif
	dbprintf(vp, " web: mylcd.sourceforge.net ");
	dbprintf(vp, " email: okio@users.sourceforge.net ");
	dbprintf(vp, " %s_%s ",  PLAYER_VERSION, libmylcdVERSION);
}

void editboxDoCmdRegistration (TEDITBOX *input, void *vp)
{
	editBoxRegisterCmdFunc(input, L"stats", editboxCmd_stats, vp, 0, 0);
	editBoxRegisterCmdFunc(input, L"flushart", editboxCmd_flushArt, vp, 0, 0);
	editBoxRegisterCmdFunc(input, L"getlengths", editboxCmd_getlengths, vp, 0, 0);
	editBoxRegisterCmdFunc(input, L"plm", editboxCmd_plm, vp, 0, 0);
	editBoxRegisterCmdFunc(input, L"fastforward,ff", editboxCmd_timeJump, vp, TIMRSKIP_FORWARD, 0);
	editBoxRegisterCmdFunc(input, L"rewind,rw", editboxCmd_timeJump, vp, TIMRSKIP_BACK, 0);
	editBoxRegisterCmdFunc(input, L"time", editboxCmd_time, vp, 0, 0);
	editBoxRegisterCmdFunc(input, L"snapshot,ss", editboxCmd_snapshot, vp, 1, 0);
	editBoxRegisterCmdFunc(input, L"list", editboxCmd_list, vp, 0, 0);
	editBoxRegisterCmdFunc(input, L"visual,vis", editboxCmd_visual, vp, 0, 0);
	editBoxRegisterCmdFunc(input, L"load,ld", editboxCmd_load, vp, 1, 0);
	editBoxRegisterCmdFunc(input, L"playlist,pl", editboxCmd_playlist, vp, 0, 0);
	editBoxRegisterCmdFunc(input, L"volume,vol", editboxCmd_volume, vp, 0, 0);
	editBoxRegisterCmdFunc(input, L"idle", editboxCmd_idle, vp, 0, 0);
	editBoxRegisterCmdFunc(input, L"skin", editboxCmd_skin, vp, 0, 0);
	editBoxRegisterCmdFunc(input, L"artsize,as", editboxCmd_artScale, vp, 0, 0);
	editBoxRegisterCmdFunc(input, L"getmeta", editboxCmd_retrieveAllMeta, vp, 0, 0);
	editBoxRegisterCmdFunc(input, L"search,find", editboxCmd_search, vp, 0, 0);
	editBoxRegisterCmdFunc(input, L"aspect,ar", editboxCmd_aspectRatio, vp, 0, 0);
	editBoxRegisterCmdFunc(input, L"rgbswap", editboxCmd_rgbswap, vp, 0, 0);
	editBoxRegisterCmdFunc(input, L"mouse", editboxCmd_mouse, vp, 0, 0);
	editBoxRegisterCmdFunc(input, L"fps", editboxCmd_fps, vp, 0, 0);
	editBoxRegisterCmdFunc(input, L"exit", editboxCmd_closeEditbox, vp, 0, 0);
	editBoxRegisterCmdFunc(input, L"close", editboxCmd_closePages, vp, 0, 0);
	editBoxRegisterCmdFunc(input, L"quit", editboxCmd_shutdown, vp, 0, 0);
	editBoxRegisterCmdFunc(input, L"sorta,sa", editboxCmd_sort, vp, SORT_ASCENDING, 0);
	editBoxRegisterCmdFunc(input, L"sortd,sd", editboxCmd_sort, vp, SORT_DESCENDING, 0);
	editBoxRegisterCmdFunc(input, L"play", editboxCmd_mctrl, vp, VBUTTON_PLAY, 0);
	editBoxRegisterCmdFunc(input, L"pause", editboxCmd_mctrl, vp, VBUTTON_PAUSE, 0);
	editBoxRegisterCmdFunc(input, L"stop", editboxCmd_mctrl, vp, VBUTTON_STOP, 0);
	editBoxRegisterCmdFunc(input, L"back,prev", editboxCmd_mctrl, vp, VBUTTON_PRETRACK, 0);
	editBoxRegisterCmdFunc(input, L"next", editboxCmd_mctrl, vp, VBUTTON_NEXTTRACK, 0);
	editBoxRegisterCmdFunc(input, L"meta", editboxCmd_mctrl, vp, VBUTTON_SMETA, 0);
	editBoxRegisterCmdFunc(input, L"open", editboxCmd_open, vp, 0, 0);
	editBoxRegisterCmdFunc(input, L"clock", editboxCmd_clock, vp, 0, 0);
	editBoxRegisterCmdFunc(input, L"config,cfg", editboxCmd_config, vp, 0, 0);
	editBoxRegisterCmdFunc(input, L"title", editboxCmd_title, vp, 0, 0);
	editBoxRegisterCmdFunc(input, L"chapter", editboxCmd_chapter, vp, 0, 0);
	
	editBoxRegisterCmdFunc(input, L"about", editboxCmd_about, vp, 0, 0);
#if !G19DISPLAY
	editBoxRegisterCmdFunc(input, L"backlight,bl", editboxCmd_usbd480Backlight, vp, 0, 0);
#endif

	editBoxRegisterCmdFunc(input, L"lorenz", editboxCmd_lorenz, vp, 0, 0);
	editBoxRegisterCmdFunc(input, L"particles", editboxCmd_particles, vp, 0, 0);
	editBoxRegisterCmdFunc(input, L"rc", editboxCmd_rc, vp, 0, 0);
	editBoxRegisterCmdFunc(input, L"tetris", editboxCmd_tetris, vp, 0, 0);

}



