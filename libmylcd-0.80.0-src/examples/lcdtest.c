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

#include "mylcd.h"
#include "demos.h"


char str[][50]		={{"欢迎光临：　　　　深圳得技通　　　　电子有限公司"},
                      {"本公司主要经营：编程器、仿真器、ＤＳＰ系列产品Ａ"},
                      {"ＲＭ、ＬＩＮＵＸ四位单片机、以及各类适配器、５１"},
                      {"系列实验板、ＰＬＣ系列实验板等等使用该套实验板不"},
                      {"仅可以学习和巩固５１单片机的基本知识各类指令，还"},
                      {"可以学习怎样用单片机驱动ＬＣＤ及ＬＣＤ中汉字的显"},
                      {"示，更为重要的是可以教会你怎样使用汉字库。在汉字"},
                      {"库中的每一个汉字都是１６Ｘ１６的点阵，一个汉字以"},
                      {"３２个字节存于字库中。只要给定一个字的区位码，便"},
                      {"可从字库中取出相应的汉字。而一个字的区位码来自哪"},
                      {"里呢？它既可以在ＤＢ中自己输入，也可以从ＰＣ中通"},
                      {"过串行口发送给单片机，单片机从ＰＣ中接收到的就是"},
                      {"区位码，从ＤＢ内容中读得的也是区位码，这样取汉字"},
                      {"就不是问题了。　　　　　　　　　　　　谢谢！　　"},
                      {"我司地址：深圳市深南中路赛格广场３７楼３７０３室"},
                      {"电话：　　　　　０７５５－８４３６１２３４　　　"}};
                      
                      


int main ()
{
	if (!initDemoConfig("config.cfg"))
		return 0;
	
	TFRAME *tmp = lNewFrame(hw,DWIDTH,DHEIGHT, frame->bpp);
	lSetCharacterEncoding(hw, CMT_GBK);

	int i=0;
	for (i=0;i<8;i++){
		lPrint(frame,str[i],0,(i*16),LFTW_WENQUANYI9PT,LPRT_CPY);
		lPrint(frame,str[i+8],0,(8+i)*16,LFTW_WENQUANYI9PT,LPRT_CPY);
	}
	
	lFlushFont(hw, LFTW_WENQUANYI9PT);
	lRefresh(frame);
	lSleep(1000);
	
	lDrawCircleFilled(frame,63,31,30, 0x0FF);
	lDrawCircleFilled(frame,63,31,15, 0xFF0);
	lDrawCircle(frame,97,31,30,LSP_SET);
		
	lRefresh(frame);
	lSleep(1000);
	
	T2POINT pts[256];
	srand(GetTickCount());

	for (i=0;i<250;i++){
		pts[i].x = rand()%(DWIDTH-1);
		pts[i].y = rand()%DHEIGHT-1;
	}

	lDrawPolyLineTo(frame, pts, 20, LSP_SET);
	lRefresh(frame);
	lSleep(1000);

	int time = 500;
	lRotateFrameL90(frame);
	lRefresh(frame);
	
	lSleep(time);
	lSaveImage(frame, L"frametest1.bmp", IMG_BMP, frame->width, frame->height);
	lRotateFrameL90(frame);
	lRefresh(frame);
	
	lSleep(time);
	lSaveImage(frame, L"frametest2.bmp", IMG_BMP, frame->width, frame->height);
	lRotateFrameL90(frame);
	lRefresh(frame);
	
	lSleep(time);
	lSaveImage(frame, L"frametest3.bmp", IMG_BMP, frame->width, frame->height);
	lRotateFrameL90(frame);
	lRefresh(frame);
	
	lSleep(time);
	lSaveImage(frame, L"frametest4.bmp", IMG_BMP, frame->width, frame->height);
	lFlipFrame(frame, tmp, FF_VERTANDHORIZ);
	lRefresh(tmp);
	
	lSleep(time);
	lSaveImage(tmp, L"frametest5.bmp", IMG_BMP, frame->width, frame->height);
	lFlipFrame(frame, tmp, FF_VERTICAL);
	lRefresh(tmp);
	
	lSleep(time);
	lSaveImage(tmp, L"frametest6.bmp", IMG_BMP, frame->width, frame->height);
	lFlipFrame(frame, tmp, FF_HORIZONTAL);
	lRefresh(tmp);
	
	lSleep(time);
	lSaveImage(tmp, L"frametest7.bmp", IMG_BMP, frame->width, frame->height);
	//lInvertFrame(frame);
	lRefresh(frame);
	
	lSaveImage(frame, L"frametest8.bmp", IMG_BMP, frame->width, frame->height);
	lSleep(time);
	

	lRefresh(frame);

	
	double c;
	for (c=0;c<361;c+=2){
		lRotate(frame,tmp,0,0,c);
		lRefresh(tmp);
		//lSleep(1);
	}

	lRefresh(frame);
	lDeleteFrame(tmp);
	
#if 1
	for (i=0;i<128;i++){
		lMoveArea(frame,64,0,127,63,1,LMOV_LOOP, LMOV_RIGHT);
		lMoveArea(frame,64,0,127,63,1,LMOV_LOOP, LMOV_UP);
		lRefreshArea(frame,64,0,127,63);
	}
	
	for (i=0;i<128;i++){
		lMoveArea(frame,0,0,63,63,1,LMOV_LOOP, LMOV_LEFT);
		lRefreshArea(frame,0,0,63,63);
	}
	
	for (i=0;i<128;i++){
		lMoveArea(frame,0,0,127,31,2,LMOV_LOOP, LMOV_LEFT);
		lRefreshArea(frame,0,0,127,31);
	}
	
	for (i=0;i<128;i++){
		lMoveArea(frame,0,32,127,63,2,LMOV_LOOP, LMOV_RIGHT);
		lRefreshArea(frame,0,32,127,63);
	}
	
	lSleep(1000);
#endif
	demoCleanup();
	return 0;
}


