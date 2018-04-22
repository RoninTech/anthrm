
// libmylcd
// An LCD framebuffer library
// Michael McElligott
// okio@users.sourceforge.net

//  Copyright(c) 2005-2009  Michael McElligott
// 
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU LIBRARY GENERAL PUBLIC LICENSE
//  as published by the Free Software Foundation; either version 2
//  of the License, or(at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU LIBRARY GENERAL PUBLIC LICENSE for details.


#include <string.h>
#include "mylcd.h"
#include "demos.h"


typedef struct{
	THWD 	*hw;
	TFRAME	*frame;
	TLPOINTEX loc;
	TRECT	rect;
	
	int		imageType;
	char	image[1024];	
	int		WinW;
	int		WinH;
	int		Port;
	int		Font;
	int		pf_flags;
	int		Style;
	int		sleepms;
	int		debug;
	float	RotA;
	
	int		arg;
	char	*line;
	int		tlines;
	char	**lines;
}TSTATE;

#define MIN(a, b) a<b?a:b
#define MAX(a, b) a>b?a:b

int initDefaultState(TSTATE *s, int argc, char **argv);
int lstart(TSTATE *s);
void printArgs(int argc, char* argv[]);
void cleanUp(TSTATE *s);
char *nextLine(TSTATE *s);
int lprintline(TSTATE *s, char *text);
int loadimage(TSTATE *s);
void printversion(TSTATE *s);
void printusage();
int saveimage(TSTATE *s);
int lprintex(TSTATE *s, char *text);


int main(int argc, char* argv[])
{

	if (argc<2){
		printusage();
		return 0;
	}
	
	TSTATE *state =(TSTATE *)calloc(1, sizeof(TSTATE));
	if (state == NULL) return 0;
	initDefaultState(state,argc, argv);
	if (!lstart(state))
		cleanUp(state);
	
	do{
		nextLine(state);
		
		if (state->debug)
			printf("processing command: %s\n",state->line);

		if(!strncmp(state->line,"--printex",9)){
		 	lprintex(state,nextLine(state));

		}else if(!strncmp(state->line,"--print",7)){
		 	lprintline(state,nextLine(state));

		}else if(!strncmp(state->line,"--lprint",8)){
		 	sscanf(nextLine(state),"%i,%i",&state->loc.x1,&state->loc.y1);
		 	lprintline(state,nextLine(state));

		}else if(!strncmp(state->line,"--flags",7)){
			sscanf(nextLine(state),"%i",&state->pf_flags);
					 			 	
		}else if(!strncmp(state->line,"--loc",5)){
			sscanf(nextLine(state),"%i,%i,%i,%i",&state->loc.x1,&state->loc.y1,&state->loc.x2,&state->loc.y2);

		}else if(!strncmp(state->line,"--line",6)){
			sscanf(nextLine(state),"%i,%i,%i,%i",&state->loc.x1,&state->loc.y1,&state->loc.x2,&state->loc.y2);
			lDrawLine(state->frame,state->loc.x1,state->loc.y1,state->loc.x2,state->loc.y2,state->Style);

		}else if(!strncmp(state->line,"--dottedline",12)){
			sscanf(nextLine(state),"%i,%i,%i,%i",&state->loc.x1,&state->loc.y1,&state->loc.x2,&state->loc.y2);
			lDrawLineDotted(state->frame,state->loc.x1,state->loc.y1,state->loc.x2,state->loc.y2,state->Style);

		}else if(!strncmp(state->line,"--box",5)){
			sscanf(nextLine(state),"%i,%i,%i,%i",&state->loc.x1,&state->loc.y1,&state->loc.x2,&state->loc.y2);
			lDrawRectangle(state->frame,state->loc.x1,state->loc.y1, state->loc.x2, state->loc.y2, state->Style);

		}else if(!strncmp(state->line,"--dottedbox",11)){
			sscanf(nextLine(state),"%i,%i,%i,%i",&state->loc.x1,&state->loc.y1,&state->loc.x2,&state->loc.y2);
			lDrawRectangleDotted(state->frame,state->loc.x1,state->loc.y1,state->loc.x2,state->loc.y2,state->Style);

		}else if(!strncmp(state->line,"--filledbox",11)){
			sscanf(nextLine(state),"%i,%i,%i,%i",&state->loc.x1,&state->loc.y1,&state->loc.x2,&state->loc.y2);
			lDrawRectangleFilled(state->frame,state->loc.x1,state->loc.y1,state->loc.x2,state->loc.y2,state->Style);
			
		}else if(!strncmp(state->line,"--lpixel",8)){
			sscanf(nextLine(state),"%i,%i,%i",&state->loc.x1,&state->loc.y1,&state->Style);
			lSetPixel(state->frame,state->loc.x1,state->loc.y1,state->Style);

		}else if(!strncmp(state->line,"--pixel",7)){
			lSetPixel(state->frame,state->loc.x1,state->loc.y1,state->Style);
									
		}else if(!strncmp(state->line,"--update",8)){
			lRefresh(state->frame);
			
		}else if(!strncmp(state->line,"--clear",7)){
			lClearFrame(state->frame);
			
		}else if(!strncmp(state->line,"--sleep",7)){
			sscanf(nextLine(state),"%i",&state->sleepms);
			lSleep(state->sleepms);
			
		}else if(!strncmp(state->line,"--font",6)){
			nextLine(state);
			sscanf(state->line,"%i",&state->Font);
			
		#if 0
		}else if(!strncmp(state->line,"--open",6)){
			sscanf(nextLine(state),"%i,%i,%i",&state->WinW,&state->WinH,&state->Port);
			if(!lstart(state)) break;
		#endif

		}else if(!strncmp(state->line,"--rotate",8)){
			sscanf(nextLine(state),"%f",&state->RotA);
			
		}else if(!strncmp(state->line,"--image",7)){
			//sscanf(nextLine(state),"%i",&state->imageType);
			sscanf(nextLine(state),"%s",state->image);
			loadimage(state);

		}else if(!strncmp(state->line,"--save",6)){
			sscanf(nextLine(state),"%i",&state->imageType);
			sscanf(nextLine(state),"%s",state->image);
			saveimage(state);
			
		}else if(!strncmp(state->line,"--debug",7)){
			sscanf(nextLine(state),"%i",&state->debug);
			
		}else if(!strncmp(state->line,"--style",7)){
			sscanf(nextLine(state),"%i",&state->Style);
			
		}else if(!strncmp(state->line,"--version",9)){
			printversion(state);
			
		}else if(!strncmp(state->line,"--help",6)){
			printusage();
		}
		
	}while(state->arg < state->tlines);

	cleanUp(state);
	return 1;
}

void printusage()
{
	printf("\nlibmylcd v%s\n",(char*)lVersion());
	printf("\nUsage: --cmd	argument,argument	[default]\n");
//	printf("  --open	Width,Height,Port	[128,64,0x378]\n");
	printf("  --loc		x,y			[0,0]\n");
	printf("  --print	\"text\"\n");
	printf("  --printex	\"text\"\n");
	printf("  --lprint	x,y \"text\"		[0,0 \"\"]\n");
	printf("  --rotate	angle			[0]\n");
	printf("  --image	IMG_type \"path\"		[0 \"\"]\n");
	printf("  --save	IMG_type \"path\"		[0 \"\"]\n");
	printf("  --sleep	mstime			[20]\n");
	printf("  --font	font			[%i]\n",LFT_COMICSANSMS7X8);
	printf("  --flags	PF_flags		[0x37]\n");
	printf("  --style	style			[0]\n");
	printf("  --line	x1,y1,x2,y2		[0,0,0,0]\n");
	printf("  --dottedline	x1,y1,x2,y2		[0,0,0,0]\n");
	printf("  --box		x,y,width,height	[0,0,0,0]\n");
	printf("  --filledbox	x,y,width,height	[0,0,0,0]\n");
	printf("  --dottedbox	x,y,width,height	[0,0,0,0]\n");
	printf("  --lpixel	x,y,style		[0,0,0]\n");
	printf("  --pixel\n");
	printf("  --clear\n");
	printf("  --update\n");		
	printf("  --version\n");
	printf("  --help\n");
	printf("  --debug				[0]\n");
	
	printf("\nexample:\n");
	printf("--debug 1 --clear --font 1033 --loc 2,0 --image 3 \"images/smallbox.bmp\""\
	  " --loc 2,0 --style 3 --print \"Hong\" --loc 79,0 --print \"Kong\" --update --sleep 1000"\
	  " --style 1 --box 0,0,160,43\n");
	printf("\nexample:\n");
	printf("--lprint 30,20 \"Hello World\"\n");
	printf("\nexample:\n");
	printf("--lprint 30,20 \"Hello World\" --update --sleep 1000\n");
	printf("\nexample:\n");
	printf("--save 4 \"frame.tga\"\n");	
}

void printversion(TSTATE *s)
{
	printf("%s\n",(char*)lVersion());
}

static int asci_to_utf16(char *src, size_t srcLen, wchar_t *des)
{
	int i = 0;

	while(i < srcLen && src[i]){
		des[i] = src[i]&0x00FF;
		i++;
	}
	return i;
}

int saveimage (TSTATE *s)
{
	wchar_t buffer[MAX_PATH+1];
	memset(buffer, 0, sizeof(buffer));
	asci_to_utf16(s->image, strlen(s->image), buffer);
	return lSaveImage(s->frame, buffer, s->imageType, s->WinW, s->WinH);
}

int loadimage (TSTATE *s)
{
	wchar_t buffer[MAX_PATH+1];
	memset(buffer, 0, sizeof(buffer));
	asci_to_utf16(s->image, strlen(s->image), buffer);
	
	TFRAME *img = lNewFrame(s->hw, s->WinW, s->WinH, s->frame->bpp);
	if (s->RotA == 0.000){
		if (lLoadImageEx(img, buffer, 0, 0))
			lCopyAreaEx(img, s->frame, s->loc.x1, s->loc.y1, 0, 0, MIN(s->WinW-1,img->width-1), MIN(s->WinH-1,img->height-1), 1, 1, s->Style);
	}else{
		TFRAME *rot = lNewImage(s->hw, buffer, s->frame->bpp);
		if (rot){
			img->style = s->Style;
			lRotate(rot, img, 0, 0, s->RotA);
			lCopyAreaEx(img, s->frame, s->loc.x1, s->loc.y1, 0, 0, MIN(s->WinW-1,img->width-1), MIN(s->WinH-1,img->height-1), 1, 1, s->Style);
			lDeleteFrame(rot);
		}
	}

	lDeleteFrame(img);
	return 1;
}

int lprintex(TSTATE *s, char *text)
{
	TLPRINTR rect;
	rect.bx1=0;
	rect.by1=0;
	rect.sx=rect.ex=s->loc.x1;
	rect.sy=rect.ey=s->loc.y1;
	rect.bx2=s->frame->width-1;
	rect.by2=s->frame->height-1;
    lPrintEx(s->frame, &rect, s->Font, s->pf_flags|PF_DONTFORMATBUFFER, s->Style, text);
	return 1;
}

int lprintline(TSTATE *s, char *text)
{
	if (!s->RotA){
		lPrint(s->frame, text, s->loc.x1, s->loc.y1, s->Font, s->Style);
	}else{
		TFRAME *txt = lNewString(s->hw, s->frame->bpp, s->pf_flags|PF_DONTFORMATBUFFER, s->Font, text);
		lRotate(txt, s->frame, s->loc.x1, s->loc.y1, s->RotA);
		lDeleteFrame(txt);
	}
	return 1;
}

char *nextLine(TSTATE *s)
{
	if(s->arg < s->tlines){
		if(s->lines[++s->arg]){
			s->line = s->lines[s->arg];
			return s->line;
		}
	}
	return " ";
}

int initDefaultState (TSTATE *s, int argc, char **argv)
{
	s->hw = NULL;
	s->frame = NULL;
	s->WinW	= DWIDTH;
	s->WinH	= DHEIGHT;
	s->Port = DDATA;
	s->Font = LFT_COMICSANSMS7X8;
	s->Style = LPRT_CPY;
	s->pf_flags = PF_NOESCAPE|PF_CLIPTEXTH|PF_CLIPDRAW|PF_CLIPWRAP;
	s->loc.x1 = 0;
	s->loc.y1 = 0;
	s->loc.x2 = 0;
	s->loc.y2 = 0;
	s->sleepms = 20;
	s->debug = 0;
	s->image[0]= 0;
	s->imageType = IMG_BMP;
	s->RotA = 0.000;
	s->lines =(char **)argv;
	s->tlines = --argc;
	s->arg = 0;
	s->line = s->lines[s->arg];
	return 1;
}

void cleanUp (TSTATE *s)
{
	if(s)
		lRefresh(s->frame);
	demoCleanup();
}

int lstart (TSTATE *s)
{
   	if(!initDemoConfig("config.cfg"))
		return 0;

    s->hw = hw;
    s->frame = frame;
	s->WinW = DWIDTH;
	s->WinH = DHEIGHT;
	s->Port = DDATA;
	s->rect.left = display->left;
    s->rect.top = display->top;
    s->rect.right = display->right;
    s->rect.btm = display->btm;
    
   	//hw->render->backGround = 0xFF00;
	//hw->render->foreGround = 0x00FF;
    
	lSetCapabilities(s->hw, CAP_BACKBUFFER, CAP_STATE_OFF);
	//lClearDisplay(s->hw);
    return 1;
}


void printArgs(int argc, char* argv[])
{	
	int i;
	for(i=0;i<argc;i++)
		printf("%i:%s\n",i+1,argv[i]);
}



