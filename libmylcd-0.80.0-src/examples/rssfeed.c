
// RSS feed
// Michael McElligott

// 'space bar' quickly scrolls to next news item
// 'esc' exits

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



#if (!defined(__LINUX__))
 #include <wininet.h>
#endif

#include "rss/rss.c"
#include "demos.h"

int updateFeed (TRSS **_rss, const char *url);
int Update_Scroll (TLSCROLLEX *s, char *txt, int font);
void scrollToEndOfItem (TLSCROLLEX *s, TLSCROLLEX *t);

#define scrollDelayPeriod	60

//char url[] = "http://newsrss.bbc.co.uk/rss/newsonline_uk_edition/front_page/rss.xml";
char url[] = "http://www.radiotimes.com/tvHighlights.xml";
char filename[] = "tvHighlights.xml";

//set character encoding and font before using
//char url[] = "http://news8.thdo.bbc.co.uk/rss/chinese/simp/china_news/rss.xml";

char *buffer = NULL;

int main ()
{
	
	if (!initDemoConfig("config.cfg"))
		return 0;

	hw->render->backGround = lGetRGBMask(frame, LMASK_BLACK);
	hw->render->foreGround = lGetRGBMask(frame, LMASK_WHITE);
	
	int item=0,i;
	int w,h;
	TRSS *rss=NULL;

	if (!updateFeed(&rss, url)){
		printf("unable to retrieve a feed\n");
		demoCleanup();
		return 0;
	}
	
	lSetCharacterEncoding(hw, CMT_GBK);
	lGetTextMetrics(hw, (char*)rss->Channel->Item[item].Descr, 0, LFTW_7x13B, &w, &h);
	TFRAME *txtsurface = lNewFrame(hw, w, h, DBPP);
	TLSCROLLEX *s = lNewScroll(txtsurface, frame);
	if (s == NULL){
		printf("libmylcd scroll api not enabled\n");
		exit(0);
	}
	
	s->desRect->y1 = 29;
	s->desRect->y2 = s->desRect->y1+h;
	s->flags = 0; // ~SCR_LOOP;
		
	lGetTextMetrics(hw, (char*)rss->Channel->Item[item].Title, 0, LFTW_7x13B, &w, &h);
	txtsurface = lNewFrame(hw,w,h, DBPP);
	TLSCROLLEX *t = lNewScroll(txtsurface, frame);
	t->desRect->y1 = 16;
	t->desRect->y2 = t->desRect->y1+h;
	t->flags = SCR_LOOP;
	
	int status =1;

	do {
		lDrawRectangleFilled(frame, 0, 0, frame->width-1, 33, hw->render->backGround);

		lPrint(frame, (char*)rss->Channel->Title,0,0,LFT_SMALLFONTS7X7, LPRT_CPY);
		if (rss->Channel->LastBuild)
			lPrint(frame, (char*)rss->Channel->LastBuild,0,8,LFT_SMALLFONTS7X7, LPRT_CPY);
		else if (rss->Channel->PubDate)
			lPrint(frame, (char*)rss->Channel->PubDate,0,8,LFT_SMALLFONTS7X7, LPRT_CPY);

		Update_Scroll(t, (char*)rss->Channel->Item[item].Title, LFTW_7x13B);
		Update_Scroll(s, (char*)rss->Channel->Item[item].Descr, LFTW_7x13B);

		
		for (i = 0; (i< s->srcFrm->width) && !kbhit(); i++){
			lUpdateScroll(s);
			lUpdateScroll(t);
			lDrawLine(frame,0,16,frame->width-1,16, LSP_SET);
			lRefreshAsync(frame, 1);
			lSleep(scrollDelayPeriod);
		}

		if (kbhit()){
			if (getch()==' ')
				scrollToEndOfItem(s,t);
			else
				break;
		}else{
			// ensure both scrolls are alligned before beginning next
			scrollToEndOfItem(s,t);
		}

		if (++item > rss->Channel->totalItems-1){
			item = 0;
			status = updateFeed(&rss, url);
		}
	}while(status);

	freeRSS(rss);
	if (buffer) free(buffer);
	
	lDeleteFrame(s->srcFrm);
	lDeleteFrame(t->srcFrm);
	lDeleteScroll(s);
	lDeleteScroll(t);
	demoCleanup();
	return 0;
}

int Update_Scroll (TLSCROLLEX *s, char *txt, int font)
{
	lDeleteFrame(s->srcFrm);
	s->srcFrm = lNewString(hw, DBPP, /*PF_DONTFORMATBUFFER*/0, font, "&bull;  %s", txt);
	s->srcRect->x2 = s->srcFrm->width-1;
	s->srcRect->y2 = s->srcFrm->height-1;
	s->pos = 0;
	return 1;
}


void scrollToEndOfItem (TLSCROLLEX *s, TLSCROLLEX *t)
{

	t->flags &= ~SCR_LOOP;
	int i;
	for (i=t->pos;i< t->srcFrm->width+8;i++){
		lUpdateScroll(t);
		lDrawLine(frame,0,16,frame->width-1,16, LSP_SET);
		lRefreshAsync(frame, 1);
		lSleep(0);
	}
	for (i=s->pos;i< s->srcFrm->width+8;i++){
		lUpdateScroll(s);
		lDrawLine(frame,0,16,frame->width-1,16, LSP_SET);
		lRefreshAsync(frame, 1);
		lSleep(0);
	}
	t->flags |= SCR_LOOP;
}

int updateFeed (TRSS **rss, const char *url)
{

	size_t len;
	
	if (buffer) free(buffer);
	char *buffer = GetUrl(url, &len);
	if (!buffer) return 0;
	
	replaceString(buffer, len, "<![CDATA[", 9, " ", 1);
	replaceString(buffer, len, "]]>", 3, " ", 1);
	
	if (*rss) freeRSS(*rss);
	*rss = newRSS((ubyte*)buffer, len);
	if (!*rss) return 0;
	
	if (!parseRSS(*rss))
		return 0;
	else
		return 1;

}

