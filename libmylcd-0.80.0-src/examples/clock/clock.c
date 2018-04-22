
// rewitten specifically for 320x240 displays

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



#include <time.h>
#include <math.h>
#include <wininet.h>


#include <windows.h>
#include <objbase.h>
#include <tlhelp32.h>
#include <psapi.h>

#include "mylcd.h"
#include "../demos.h"
#include "../rss/rss.h"
#include "net.h"
#include "client.h"


extern int utf8_wcstombs (ubyte *des, size_t des_n, wchar_t *wc, size_t wc_n);



#define IPC_PLAYING_FILEW 13003 
#define IPC_PLAYLIST_MODIFIED 3002 
#define IPC_CB_MISC 603
#define IPC_CB_MISC_TITLE 0
#define IPC_CB_MISC_VOLUME 1 // volume/pan
#define IPC_CB_MISC_STATUS 2
#define IPC_CB_MISC_EQ 3
#define IPC_CB_MISC_INFO 4
#define IPC_CB_MISC_VIDEOINFO 5
#define IPC_FILE_TAG_MAY_HAVE_UPDATEDW 3005 

#define METATAGTOTAL 30
ubyte metaTags[METATAGTOTAL][16]={
	"Title",	// index 0
	"Artist",
	"Album",
	"Track",
	"Year",
	"Genre",
	"Comment",
	"Length",
	"Bitrate",
	"SRate",
	"Stereo",
	"VBR",
	"BPM",
	"Gain",
	"AlbumGain",
	"Encoder",
	"AlbumArtist",
	"OriginalArtist",
	"Disc",
	"Remixer",
	"Media",
	"Lyricist",
	"Subtitle",
	"Mood",
	"Key",
	"Composer",
	"Conductor",
	"Publisher",
	"Copyright",
	"URL",
};

typedef struct{
	int hour;
	int minute;
	int second;
	int lastsecond;
	ubyte Hbin[8];
	ubyte Mbin[8];
	ubyte Sbin[8];
	TFRAME *frame;
	TFRAME *rotframe;
	TFRAME *rotframesrc;
}TCLOCK;

typedef struct {
	char module[MAX_PATH];
}TPROCMOD;

static INLINE void byte2BinStr6 (ubyte b, ubyte *str); // dec to 6bit bin string
static float nextAngle (float angle);
static INLINE void drawBorder (TFRAME *frame);
static void drawWave (TFRAME *frame, float secondmult);
static void drawBinClock (TFRAME *frame, int x, int y, TCLOCK *clock);
static void drawRssScroll (TFRAME *frame, int x, int y, TLSCROLLEX *rssscroll, int counter);
static void drawDigitClock (TFRAME *frame, TCLOCK *clock, float angle);
static void drawProcList(TFRAME *frame, int x, int y, TFRAME *procframe);
int updateTime (TCLOCK *clock);
int updateFeed (TRSS **feed);
int renderText (TFRAME *frame, char *buffer, int flags, int font);
void renderRSS (TFRAME *frame, TRSS *rss, int font, char *scratchpad);
int createScroll (TLSCROLLEX **scroll, TFRAME *rssframe);
int getProcList (HANDLE hSnap, TPROCMOD *procList, int total);
int getProcTotal (HANDLE hSnap);
int renderProcList (TFRAME *frame, int font);
void setEncoding(TRSS *rss);

static void drawWinampTrack (TFRAME *frame, TFRAME *trackInfoFrame);
int processSocketCmds (TMPNET *net);
void renderTrackinfo (TFRAME *frame, TFRAME *des);

// use static to zero the structs
static TMPCMD cmd;
static TMPCMDWAVE wave;
static TMPGENTRACKINFO gen;
static TMPCMDEQ eq;
static TMPNET mpnet;
static wchar_t title[4096];

#define RSSUPDATEPERIOD 600

#define RSSFONT LFTW_ROUGHT18
/*	other fonts worth trying:	
	LFTW_NIMBUS14
	LFTW_18x18KO
	LFTW_HELVB18
	LFTW_9x18
	LFTW_UNICODE
	LFTW_B24
	LFTW_B16
	LFTW_WENQUANYI12PT
	LFTW_ROUGHT18
*/


ubyte *rssbuffer = NULL;


//ubyte url[] = "http://news8.thdo.bbc.co.uk/rss/chinese/simp/china_news/rss.xml";
//ubyte url[] = "http://news8.thdo.bbc.co.uk/rss/chinese/trad/china_news/rss.xml";
ubyte url[] = "http://newsrss.bbc.co.uk/rss/newsonline_uk_edition/front_page/rss.xml";
//ubyte url[] = "http://sourceforge.net/export/rss2_projsummary.php?group_id=147984";

#define MAX(a, b) a>b?a:b


int main ()
{

	if (!initDemoConfig("config.cfg"))
		return 0;

	hw->render->backGround = lGetRGBMask(frame, LMASK_WHITE);
	hw->render->foreGround = lGetRGBMask(frame, LMASK_BLACK);
	lClearFrame(frame);
	
	//LFTW_ROUGHT18 is missing a few code points
	if (RSSFONT == LFTW_ROUGHT18)
		lMergeFontW(hw, LFTW_ROUGHT18, LFTW_HELVR18);

	memset(&mpnet, 0, sizeof(TMPNET));
	memset(&gen, 0, sizeof(TMPGENTRACKINFO));

	netInit();
	mpnet.socket.server = netConnect(&mpnet, "127.0.0.1", MY_PORT, IPPROTO_UDP);
	netEmptyReadBuffer(&mpnet); // clean the pipe
	netGetCurrentTrackInfo(&mpnet, NULL);

	TLSCROLLEX *rssscroll = NULL;
	TRSS *rss = NULL;
	TFRAME *rssframe = lCloneFrame(frame);
	TFRAME *trackInfoFrame = lNewFrame(hw, DWIDTH, 42, DBPP);
	TFRAME *procframe = lNewFrame(hw, 128, 72, DBPP);

	TCLOCK clock = {-1,-1,-1,-2};
	clock.rotframe = lNewFrame(hw, 224, 100, DBPP);
	clock.rotframesrc = lNewFrame(hw, clock.rotframe->width, clock.rotframe->height, clock.rotframe->bpp);
	clock.frame = NULL;

	int counter = 0;
	float angle = 0.0;
	int rssfeedcounter = 0xFFFF;

	if (!updateFeed(&rss))
		printf("RSS feed unavailable\n");

	do{
		processSocketCmds(&mpnet);

		if (++counter > 9){
			counter = 0;
		}else if (counter == 1){
			updateTime(&clock);
			if (clock.lastsecond != clock.second){
				clock.lastsecond = clock.second;

				netGetCurrentTrackInfo(&mpnet, NULL);
				// update track name once per second
				renderTrackinfo(frame, trackInfoFrame);
				
				// update feed every 10 minutes
				if ((++rssfeedcounter > RSSUPDATEPERIOD) && rss){
					if (rssfeedcounter != 0xFFFF)
						updateFeed(&rss);
					rssfeedcounter = 0;
					setEncoding(rss);
					renderRSS(rssframe, rss, RSSFONT, (char*)rssbuffer);
					createScroll(&rssscroll, rssframe);
				}
				//update process list once per second
				renderProcList(procframe, LFT_COURIERNEWCE8);
			}
		}

		lClearFrame(frame);
		drawRssScroll(frame, 1, 0, rssscroll, counter);
		drawBinClock(frame, frame->width-90, 160, &clock);
		drawProcList(frame, 5, 130, procframe);
		drawDigitClock(frame, &clock, angle);
		drawWinampTrack(frame, trackInfoFrame);
		//drawWave(frame, (float)clock.second);
		//drawBorder(frame);

		
		lRefresh(frame);
		lSleep(25);
		angle = nextAngle(angle);
	}while (!kbhit());
	
	//lSaveImage(frame,"clock.png", IMG_PNG, frame->width, frame->height);

	if (rss) freeRSS(rss);
	if (rssbuffer) free(rssbuffer);

	netShutdown();
	lDeleteScroll(rssscroll);
	lDeleteFrame(procframe);
//	lDeleteFrame(rsstextbuffer);
	lDeleteFrame(rssframe);
	lDeleteFrame(clock.frame);	
	lDeleteFrame(clock.rotframe);
	lDeleteFrame(clock.rotframesrc);
	lDeleteFrame(trackInfoFrame);
	demoCleanup();
	
	return 1;
}

void renderTrackinfo (TFRAME *frame, TFRAME *des)
{
	lClearFrame(des);
	if (*title){
		lSetCharacterEncoding(hw, CMT_UTF8);
		
		TLPRINTR trect = {0,0,des->width-1,des->height-1,0,0,0,0};
		lPrintEx(des, &trect, /*LFT_COMICSANSMS8X9*/2001, PF_MIDDLEJUSTIFY|PF_CLIPWRAP, LPRT_CPY,\
		  "Now Playing:\n%s\n(%i:%.2i/%i:%.2i)", (char*)title, gen.position/1000/60,\
		  (gen.position/1000)%60, gen.length/60, gen.length%60);
	}
}

void setEncoding (TRSS *rss)
{
	if (strstr((char*)rss->XMLtag,"GB2312") || strstr((char*)rss->XMLtag,"gb2312"))
		lSetCharacterEncoding(hw, CMT_GB18030);
	else if (strstr((char*)rss->XMLtag,"GBK") || strstr((char*)rss->XMLtag,"gbk"))
		lSetCharacterEncoding(hw, CMT_GBK);
	else if (strstr((char*)rss->XMLtag,"big5") || strstr((char*)rss->XMLtag,"BIG5"))
		lSetCharacterEncoding(hw, CMT_BIG5);
	else if (strstr((char*)rss->XMLtag,"utf-8") || strstr((char*)rss->XMLtag,"UTF-8"))
		lSetCharacterEncoding(hw, CMT_UTF8);
	else
		lSetCharacterEncoding(hw, CMT_ISO8859_1);
		
}

static void drawProcList(TFRAME *frame, int x, int y, TFRAME *procframe)
{
	lCopyAreaEx(procframe, frame, x, y, 0,0, procframe->width, procframe->height, 1, 1, LCASS_CPY);
}

int renderProcList (TFRAME *frame, int font)
{
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPALL, 0);
	if (hSnap != NULL){
		int processTotal = getProcTotal(hSnap);
		TPROCMOD procList[processTotal];
		processTotal = getProcList(hSnap, procList, processTotal);

		int i;
		TLPRINTR trect = {0,0,frame->width-1,frame->height-1,0,0,0,0};
		lClearFrame(frame);
		for (i=MAX(0,processTotal-6); i<processTotal; i++){
			lPrintEx(frame, &trect, font, PF_NEWLINE, LPRT_CPY, procList[i].module);
			trect.ey += 2;
		}

		//drawBorder(frame);
		CloseHandle(hSnap);
		return processTotal;
	}else{
		return 0;
	}
}

int getProcTotal (HANDLE hSnap)
{
	int total = 0;
	PROCESSENTRY32 proc;
	proc.dwSize = sizeof(PROCESSENTRY32);

	Process32First(hSnap, &proc);
	while(Process32Next(hSnap, &proc))
		total++;
		
	return total;
}

int getProcList (HANDLE hSnap, TPROCMOD *procList, int total)
{
	int i;
	PROCESSENTRY32 proc;
	proc.dwSize = sizeof(PROCESSENTRY32);
		
	Process32First(hSnap, &proc);
	for (i=0; i<total; i++){
		Process32Next(hSnap, &proc);
		memcpy(procList[i].module, proc.szExeFile, strlen(proc.szExeFile)+1);
	}
	return i;
}

static void drawWinampTrack (TFRAME *frame, TFRAME *trackInfoFrame)
{
	if (trackInfoFrame)
		lCopyArea(trackInfoFrame, frame, 0, 197, 0,0, trackInfoFrame->width, trackInfoFrame->height);
}

int createScroll (TLSCROLLEX **scroll, TFRAME *rssframe)
{
	if (*scroll) 
		lDeleteScroll(*scroll);

	*scroll = lNewScroll(rssframe, frame);
	if (*scroll){
		(*scroll)->desRect->x2 = frame->width-1;
		if (frame->height == 272)
			(*scroll)->desRect->y2 = 110;
		else
			(*scroll)->desRect->y2 = 84;
			
		(*scroll)->flags = SCR_LOOP;
		(*scroll)->dir = SCR_UP;
		(*scroll)->loopGapWidth = 16;	// 16 pixels between bottom and top of repeating scroll
		return 1;
	}else{
		return 0;
	}
}


void renderRSS (TFRAME *frame, TRSS *rss, int font, char *scratchpad)
{
	if (!frame||!rss||!scratchpad) return;
	
	char *rssbufferposition = scratchpad;
	int i;

	strcpy(rssbufferposition, (char*)rss->Channel->Title);
	rssbufferposition += strlen((char*)rss->Channel->Title);
	*rssbufferposition++ = '\n';
	*rssbufferposition++ = '\n';
	
	for (i=0; i<rss->Channel->totalItems; i++){
		strcpy(rssbufferposition, " &bull; ");
		rssbufferposition += 8;
		
		strcpy(rssbufferposition, (char*)rss->Channel->Item[i].Title);
		rssbufferposition += strlen((char*)rss->Channel->Item[i].Title);
		*rssbufferposition++ = ':';
		*rssbufferposition++ = '\n';

		strcpy(rssbufferposition, (char*)rss->Channel->Item[i].Descr);
		rssbufferposition += strlen((char*)rss->Channel->Item[i].Descr);
		*rssbufferposition++ = '\n';
	}

	// terminate sting buffer and remove last newline
	*(rssbufferposition-sizeof(char)) = 0;

	lClearFrame(frame);
	
	//printf("rssbuffer: #%s#\n",scratchpad);
	renderText(frame, scratchpad, PF_WORDWRAP|PF_CLIPWRAP|PF_RESETXY, font);

	// top of page marker
	lDrawLineDotted(frame,0,0,frame->width-1,0, LSP_SET);
	// bottom of page marker
	lDrawLineDotted(frame,0,frame->height-1,frame->width-1,frame->height-1, LSP_SET);
//	lSaveImage(frame,"rss.bmp", IMG_BMP, frame->width, frame->height);
}

int renderText (TFRAME *frame, char *buffer, int flags, int font)
{
	int total = lCountCharacters(hw, buffer);
	if (!total) return 0;
	unsigned int *glist = (unsigned int *)malloc(sizeof(unsigned int)*total);
	if (!glist) return 0;

	lDecodeCharacterBuffer(hw, buffer, glist, total);
	lCacheCharacterBuffer(hw, glist, total, font);
	
	// limit metrics to a max width of frame->width-1 which is usually lcd width
	TLPRINTR rect = {0, 0, frame->width-1, 0, 0, 0, 0, 0};
	
	lGetTextMetricsList(hw, glist, 0, total-1, flags, font, &rect);
	lResizeFrame(frame, frame->width, rect.by2+1, 0);
	rect.bx2 = frame->width-1;
	lPrintList(frame, glist, 0, total, &rect, font, flags, LPRT_CPY);
	free(glist);
	return total;
}

static void drawDigitClock (TFRAME *frame, TCLOCK *clock, float angle)
{
	lClearFrame(clock->rotframe);
	frame->style = LSP_AND;
	clock->rotframe->style = LSP_AND;
	lCopyAreaScaled(clock->frame, clock->rotframesrc, 0, 0, clock->frame->width, clock->frame->height, 0, 0, clock->rotframesrc->width, clock->rotframesrc->height, LCASS_CPY);
	lRotateFrameEx(clock->rotframesrc, clock->rotframe, 0, angle, 0, 260.0*3.0, -300.0*3.0, (clock->rotframe->width>>1), 46);
	lCopyAreaEx(clock->rotframe, frame,  (DWIDTH>>1)-(clock->rotframe->width>>1), (DHEIGHT>>1)-(clock->rotframe->height>>1),\
	  0, 0, clock->rotframe->width-1, clock->rotframe->height-1, 1, 1, LCASS_AND);
	frame->style = LSP_SET;
	clock->rotframe->style = LSP_SET;
}

static void drawRssScroll (TFRAME *frame, int x, int y, TLSCROLLEX *s, int counter)
{
	if (!s) return;
	if (counter&0x01)
		lUpdateScroll(s);
	else
		lCopyArea(s->desFrm, s->clientFrm, s->desRect->x1, s->desRect->y1, s->desRect->x1, s->desRect->y1, s->desRect->x2, s->desRect->y2);
}

static void drawBinClock (TFRAME *frame,int x, int y, TCLOCK *clock)
{
	TLPRINTR trect = {x,y,frame->width-1,frame->height-1,0,0,0,0};
	lPrintEx(frame, &trect, LFT_PORSCHE911, PF_RESETY|PF_CLIPWRAP, LPRT_CPY,"%s\n%s\n%s", clock->Hbin, clock->Mbin, clock->Sbin);
}

static float nextAngle (float angle)
{
	angle++;
	if ((angle>90.0) && (angle<270))
		return 270.0;
	else if (angle > 360.0)
		return 0.0;
	else
		return angle;
}

static void drawWave (TFRAME *frame, float secondmult)
{
	float twopiw = 2.00*M_PI/(float)DWIDTH;
	float twopiwfour = twopiw*4.00;
	float waveoffset = ((float)frame->width/2.5)-8.0;
	float x,y;

	for (x=0; x<DWIDTH; x+=0.1){
		y = cosf(x*twopiw) * sinf(x*twopiwfour) * secondmult;
		lSetPixel(frame, x, y+waveoffset, LSP_SET);
	}
}

int updateTime (TCLOCK *clock)
{
	char clkbuff[12];
	int lastminute = clock->minute;

	time_t t = time(0);
	struct tm *tdate = localtime(&t);

	clock->hour = tdate->tm_hour;
	clock->minute = tdate->tm_min;
	clock->second = tdate->tm_sec;

	if (clock->lastsecond != clock->second){
		byte2BinStr6(clock->hour, clock->Hbin);
		byte2BinStr6(clock->minute, clock->Mbin);
		byte2BinStr6(clock->second, clock->Sbin);
	}

	if ((lastminute == clock->minute) && clock->frame){
		return 0;
	}else{
		if (clock->frame)
			lDeleteFrame(clock->frame);
		strftime(clkbuff,6,"%H:%M",tdate);
		clock->frame = lNewString(hw, DBPP, PF_DONTFORMATBUFFER, LFT_TETRICIDE, clkbuff);
		//lSaveImage(clock->frame, L"clock.bmp", IMG_BMP, 0, 0);
		return (clock->frame != NULL);
	}
	return 0;
}

static INLINE void byte2BinStr6 (ubyte c, ubyte *str)
{
	int j=0;
	int i=6;

	while(i--){
		if (c&(1<<i))
			str[j++]='1';
		else
			str[j++]='0';
	}
	str[6]=0;
	return;
}

static INLINE void drawBorder (TFRAME *frame)
{
	//lDrawRectangle(frame, 0, 0, frame->width-1, frame->height-1, LSP_SET);
}


int updateFeed (TRSS **rss)
{

//	#if defined(__LINUX__)
//		ubyte cmd[16+strlen(url)];
//		sprintf(cmd,"wget -q -t 5 %s",url);
//		system(cmd);
//		buffer = read_file(filename);
//		if (!buffer) return 0;
//		sprintf(cmd,"rm %s",filename);
//		system(cmd);
//	#else
		//long bread=0;
		//long *pbread = &bread;
		size_t blen = 0;
	
		if (rssbuffer) free(rssbuffer);
//		rssbuffer = calloc(1,blen);
//		if (!rssbuffer) return 0;
//		if (!GetUrl(url, rssbuffer, blen)) return 0;

		rssbuffer = (ubyte*)GetUrl((char*)url, &blen);

//	#endif

	//printf("rssbuffer: #%s#\n",rssbuffer);

	if (*rss) freeRSS(*rss);
	*rss = newRSS(rssbuffer, strlen((char*)rssbuffer));
	if (!*rss) return 0;
	
	if (!parseRSS(*rss)){
		//printf("not ok\n");
		return 0;
	}else{
		//printf("ok\n");
		return 1;
	}
}




static int isWideStrCmd (int cmd)
{
	return (cmd == CMD_GET_TRACKTITLEW || cmd == CMD_GET_TRACKFILENAMEW || cmd == CMD_GET_METADATAW);
}

static int _MIN (int a, int b)
{
	if (a < b)
		return a;
	else
		return b;
}

static int cmd_readString (TMPNET *net, TMPCMD *cmd, void *buffer, size_t buffersize)
{
	//printf("string: %i %i %i (%i)\n",cmd->data1, cmd->data2, cmd->data3, cmd->command);
	
	if (cmd->data3){
		if (cmd->magic == MAGIC){
			cmd->data3 = _MIN(cmd->data3, buffersize);
			if (netReadSocket(net, buffer, cmd->data3) == SOCKET_ERROR){
				printf("error reading cmd_readString, magic = 0x%X\n", cmd->magic);
				return 0;
			}else{
				if (isWideStrCmd(cmd->command)){
					return cmd->data3/sizeof(wchar_t);
				}else{
					return cmd->data3/sizeof(char);
				}
			}
		}else{
			printf("cmd_readString(): error reading buffer, invalid magic (0x%X)\n", cmd->magic);
		}
	}else{
		printf("cmd_readString(): could not retrieve string, magic = 0x%X\n", cmd->magic);
	}

	return 0;
}

int cmd_processHostIPC (TMPNET *net, TMPCMD *cmd)
{
	//printf("ipc: %i %i\n", cmd->data1, cmd->data2);

	if (cmd->data1 == IPC_PLAYLIST_MODIFIED){
		// request a title or two
		
	}else if (cmd->data1 == IPC_PLAYING_FILEW){
		//cururent track = cmd->data2;

	}else if (cmd->data1 == IPC_CB_MISC){
		if (cmd->data2 == IPC_CB_MISC_TITLE){
			// title of current track has changed

		}else if (cmd->data2 == IPC_CB_MISC_STATUS){
			netGetPlayState(net, NULL);
		}
	}else if (cmd->data1 == IPC_FILE_TAG_MAY_HAVE_UPDATEDW){
		// do something
	}

	return 1;
}

int cmd_processCmd (TMPNET *net, TMPCMD *cmd)
{
		
	switch(cmd->command){
	  case CMD_GET_METADATAW: {
	  		wchar_t buffer[4096]; 
			if (cmd_readString(net, cmd, buffer, sizeof(buffer))){
				//printf("MetaW: playlist position:%i, tag name:\"%s\" = ", cmd->data2, metaTags[cmd->data1]);
				//wprintf(L"\"%s\" %i %i %i", buffer, cmd->data1, cmd->data2, cmd->data3);
				//printf("\n");
			}
			break;
		}	  
	  case CMD_GET_METADATAA:{
	  		char buffer[4096];
			if (cmd_readString(net, cmd, buffer, sizeof(buffer))){
				//printf("MetaA: playlist position:%i, tag name:\"%s\" = ", cmd->data2, metaTags[cmd->data1]);
				//printf("\"%s\" %i %i %i\n", buffer, cmd->data1, cmd->data2, cmd->data3);
			}
			break;
		}
	  case CMD_GET_TRACKFILENAMEW:
	  case CMD_GET_TRACKTITLEW: {
			wchar_t buffer[4096];
			if (cmd_readString(net, cmd, buffer, sizeof(buffer))){
			//	wprintf(L"TrackTitleW: playlist:%i postion:%i: \"%s\" %i", cmd->data1, cmd->data2, title, cmd->data3);
			//	printf("\n");
				 utf8_wcstombs((ubyte*)title, sizeof(title), buffer, (cmd->data3/sizeof(wchar_t))-1);
			}
			break;
		}
	  case CMD_GET_TRACKFILENAMEA:
	  case CMD_GET_TRACKTITLEA: {
	  	//	char buffer[4096];
			if (cmd_readString(net, cmd, title, sizeof(title))){
				//printf("TrackTitleA: playlist %i, postion %i: \"%s\" %i", cmd->data1, cmd->data2, buffer, cmd->data3);
			}
			break;
		}
	  case CMD_GET_MPVOLUME: 
			//printf("Media player volume is: %i (0....255)\n", cmd->data1);
			break;

	  case CMD_GET_MPNAME:{
	  		char buffer[4096]; 
			if (cmd_readString(net, cmd, buffer, sizeof(buffer))){
				//printf("Media player name: \"%s\"\n", buffer);
			}
			break;
		}
	  case CMD_HOSTIPC:
	  		cmd_processHostIPC(net, cmd);
	  		break;
			
	  case CMD_GET_TOTALPLAYLISTS:
			//printf("Total playlists: %i\n", cmd->data1);
			break;

	  case CMD_GET_PLAYSTATUS:
	  		//printf("Play state: %i\n", cmd->data1);
	  		break;
	  		
	  case CMD_GET_PLVERSION:
	  		//printf("Plugin version: 0x%x %i %i\n", cmd->data1, cmd->data2, cmd->data3);
	  		break;

	  case CMD_GET_MPVERSION:
	  		//printf("Media player version: 0x%x\n", cmd->data1);
	  		break;	  		

	  //default :
			//printf("unhandled cmd: %i\n", cmd->command);
	}
	return 0;
}

static ubyte getCmdIndex (TMPNET *net)
{
	ubyte cmd = 0;
	netReadSocketPeek(net, &cmd, sizeof(unsigned char));
	return cmd;
}

static int getStructType (TMPNET *net)
{
	ubyte cmd = getCmdIndex(net);
	if (!cmd)
		return 0;
	
	if (cmd == CMD_SET_EQDATA || cmd == CMD_GET_EQDATA)
		return CMDSTRUCT_EQ;
	else if (cmd == CMD_GET_CURRENTTRKINFO)
		return CMDSTRUCT_GEN;
	else if (cmd == CMD_GET_SPECTRUMDATA)
		return CMDSTRUCT_WAVE;
	else if (cmd == CMD_HOSTEXIT)
		return CMD_HOSTEXIT;
	else if (cmd <= CMD_TOTAL)
		return CMDSTRUCT_CMD;

	return 0;
}

static int cmd_readWave (TMPNET *net, TMPCMDWAVE *wave)
{
	size_t dataSize = sizeof(TMPCMDWAVE) - sizeof(wave->data); // header length
	int ret = netReadSocketPeek(net, wave, dataSize);
	if (ret == 1)	// has has signaled its closing connection
		return 0;
	else if (ret > 1 && ret != dataSize)
		return -1;
	else if (wave->magic != MAGIC)
		return 0;

	if (wave->len > sizeof(wave->data)/* || wave->magic != MAGIC*/){
		//printf("error reading wave data. data too large or invalid magic: %i > %i, magic = 0x%X, ret = %i\n", wave->len, sizeof(wave->data), wave->magic, ret);
		netEmptyReadBuffer(net);
		return -1;
	}
	
	dataSize = sizeof(TMPCMDWAVE) - (sizeof(wave->data)-wave->len);
	ret = netReadSocket(net, wave, dataSize);
	if (ret != dataSize || wave->magic != MAGIC){
		//printf("error reading wave data. length mismatch; expected:%i but received:%i, magic = 0x%X\n", dataSize, ret, wave->magic);
		if (ret == -1){		// host has been terminated without sending shutdown signal
			return 0;
		}else{
			return -1;
		}
	}else{
		return 1;
	}
}

static int cmd_readEq (TMPNET *net, TMPCMDEQ *eq)
{
	return netReadSocket(net, eq, sizeof(TMPCMDEQ));
}

static int cmd_readCmd (TMPNET *net, TMPCMD *cmd)
{
	return netReadSocket(net, cmd, sizeof(TMPCMD));
}

static int cmd_readGen (TMPNET *net, TMPGENTRACKINFO *gen)
{
	return netReadSocket(net, gen, sizeof(TMPGENTRACKINFO));
}

int processSocketCmds (TMPNET *net)
{
	size_t pendingdata = 0;
	pendingdata = netIsPacketAvailable(net);
	if (!pendingdata)
		return 0;
	
	do{
		switch(getStructType(net)){
		  case CMDSTRUCT_WAVE:
			cmd_readWave(net, &wave);
			//cmd_processWave(&wave); do with it what you want
			break;
			
		  case CMDSTRUCT_CMD:
			cmd_readCmd(net, &cmd);
			cmd_processCmd(net, &cmd);
			break;	

		  case CMDSTRUCT_GEN:
			cmd_readGen(net, &gen);
			//cmd_processGen(&gen);
			//printf("\nCurrent track details:\n");
			//printf(" Length:%i\n Position:%i\n Bitrate:%i\n Channels:%i\n Samplerate:%ikhz\n Playlist position:%i (0....n)\n Total tracks:%i\n\n", gen.length, gen.position, gen.bitrate, gen.channels, gen.samplerate, gen.playlistpos, gen.totaltracks);

			//netGetMetaData(net, 1/*tag index*/, CMD_GET_METADATAW, gen.playlistpos, L"Artist", NULL, 0);
			//netGetMetaData(net, 1/*tag index*/, CMD_GET_METADATAA, gen.playlistpos, "Artist", NULL, 0);
			netGetTitle(net, gen.playlistpos, CMD_GET_TRACKTITLEW, NULL, 0);
			//netGetTitle(net, gen.playlistpos, CMD_GET_TRACKTITLEA, NULL, 0);
			break;
		
		  case CMDSTRUCT_EQ:
			cmd_readEq(net, &eq);
			//cmd_processEq(&eq);
			break;
		  	
		  case CMD_HOSTEXIT:
			//printf("host exiting\n");
  			return 0;
	  		
		  default:
	 		//printf("invalid packet\n");
	  		return 0;
		}
		pendingdata = netIsPacketAvailable(net);
	}while(pendingdata > 1);
	return 1;
}

