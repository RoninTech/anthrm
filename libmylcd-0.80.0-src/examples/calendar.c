/***************************
* Written by: Hung Ki Chan
* Cold winterday in December 2005
* A simple calendar
***************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mylcd.h"
#include "demos.h"

char months[12][10]={"January","February","March","April","May","June","July",
		     "August","September","October","November","December"};
 
int return_time(int choice);
int printYearRotated (TFRAME *frame, int x, int y, int year);
void update_time (char *s, int len);
int renderCalendar (TFRAME *frame);


int main(int argc, char *argv[])
{
	if (!initDemoConfig("config.cfg"))
		return 0;

    do{

		renderCalendar(frame);
    	lRefresh(frame);
    	lSleep(500);

	}while(!kbhit());

    demoCleanup();

  return 0;
}

int renderCalendar (TFRAME *frame)
{
    if (return_time(5)<0 || return_time(5) > 11)
    	return 0;

    int days[7];
    char buff[9];
	TLPRINTR rect = {0,0,frame->width-1,frame->height-1,0,0,0,0};
	TLPRINTR rect2 = {95,-2,frame->width-1,frame->height-1,95,0,0,0};
   	int y = 8;
    struct tm tm;
    memset(&tm, 0, sizeof(tm));

   	tm.tm_year = return_time(6);
   	tm.tm_mon = return_time(5);
   	tm.tm_mday = 1;						/* start on the first */
    
   	lClearFrame(frame);              //Clear the old stuff
   	lPrint(frame, "Su  Mo Tu We  Th  Fr  Sat", 2, 0, LFT_SMALLFONTS7X7, LPRT_CPY); //print weekdays

   	do{
		time_t t = mktime(&tm);			/* get the time_t for now */
		tm = *(localtime(&t));
	
		days[tm.tm_wday] = tm.tm_mday;   /* Insert date to array */
		rect.sx = 2+(tm.tm_wday*14);
		rect.sy = y;

		if (days[tm.tm_wday] == return_time(4)){
			lPrintEx(frame, &rect, LFT_SMALLFONTS7X7, 0, LPRT_CPY, "%i",days[tm.tm_wday]);
			lInvertArea(frame, rect.sx-1, rect.sy-1, rect.ex-1, rect.ey);
		}else{
			lPrintEx(frame, &rect, LFT_SMALLFONTS7X7, 0, LPRT_CPY, "%i",days[tm.tm_wday]);
		}
    
   		if (tm.tm_wday==6)
   			y+=7; 				/* done sat, do sun = next row*/          
		t+=60*60*24;			/* go to the next day */
		tm = *(localtime(&t));	/* refresh time */

	}while(tm.tm_mon == return_time(5));	/* do so to next month */
    	
	update_time(buff, 9);
	lPrintEx(frame, &rect2, LFTW_WENQUANYI9PT, PF_RESETXY|PF_CLIPWRAP|PF_MIDDLEJUSTIFY, LPRT_CPY, months[return_time(5)]);
	lPrintEx(frame, &rect2, LFT_DOTUMCHE24X24, PF_NEWLINE|PF_CLIPWRAP|PF_MIDDLEJUSTIFY, LPRT_CPY, "%d", (return_time(6)+1900));
	lPrintEx(frame, &rect2, LFTW_WENQUANYI9PT, PF_NEWLINE|PF_CLIPWRAP|PF_MIDDLEJUSTIFY, LPRT_CPY, buff);

	return 1;
}


int return_time (int choice)
{
    time_t t = time(0);
    struct tm *tdate = localtime(&t);
    switch(choice)
    { 
    case 1: return tdate->tm_hour;   //return current hour
            break;
    case 2: return tdate->tm_min;    //return current minute
            break;
    case 3: return tdate->tm_sec;    //return current second
            break;
    case 4: return tdate->tm_mday;   //return day of month 1--31
            break;
    case 5: return tdate->tm_mon;    //return month 0--11
            break;
    case 6: return tdate->tm_year;   //return Year (calendar year minus 1900)
            break;
    case 7: return tdate->tm_wday;   //return weekday 0--6 0=sunday
            break;
    }
    
   return 0;
}

int printYearRotated (TFRAME *frame, int x, int y, int year)
{
	TFRAME *txt = lNewString(frame->hw, frame->bpp, 0, LFT_DOTUMCHE24X24, "%i       ", year);
	lRotate(txt, frame, x, y, 90.0);
	lDeleteFrame(txt);
	return 1;
}

void update_time (char *s, int len)     //digital clock
{
	time_t t = time(0);
	struct tm *tdate = localtime(&t);
	strftime(s,len,"%H:%M:%S",tdate);
}
