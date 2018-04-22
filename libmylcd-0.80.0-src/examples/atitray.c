

// ATI Tray Tools plugin

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
#include <unistd.h>
#include <sys/types.h>
#include <windows.h>


#include "mylcd.h"
#include "demos.h"

/*
typedef struct tagTATTData {
	DWORD CurGPU;     			//Current GPU Speed
	DWORD CurMEM;				//Current MEM Speed
	DWORD isGameActive; 		//If game from profile is active, this field will be 1 or 0 if not.
	DWORD is3DActive;  		//1=3D mode, 0=2D mode
	DWORD isTempMonSupported; 	//1 - if temperature monitoring supported by ATT
	DWORD GPUTemp;            	//GPU Temperature 
	DWORD ENVTemp;            	//ENV Temperature 
	DWORD FanDuty;            	//FAN Duty
	DWORD MAXGpuTemp; 		   	//MAX GPU Temperature (only if Hardware Monitor enabled!)
	DWORD MINGpuTemp;         	//MIN GPU Temperature (only if Hardware Monitor enabled!)
	DWORD MAXEnvTemp;         	//MAX ENV Temperature (only if Hardware Monitor enabled!)
	DWORD MINEnvTemp;         	//MIN ENV Temperature (only if Hardware Monitor enabled!)
         			 			   	//3d settings
	DWORD CurD3DAA;           	//Direct3D Antialiasing value
	DWORD CurD3DAF;           	//Direct3D Anisotropy value
	DWORD CurOGLAA;           	//OpenGL Antialiasing value
	DWORD CurOGLAF;           	//OpenGL Anisotropy value
} TATTData, *PATTData;
*/
typedef struct tagTATTData {
	DWORD CurGPU;      //Current GPU Speed
	DWORD CurMEM;      //Current MEM Speed
	DWORD isGameActive; //If game from profile is active, this field will be 1 or 0 if not.
	DWORD is3DActive;  //1=3D mode, 0=2D mode
	DWORD isTempMonSupported; //1 - if temperature monitoring supported by ATT
	DWORD GPUTemp;            //GPU Temperature 
	DWORD ENVTemp;            //ENV Temperature 
	DWORD FanDuty;            //FAN Duty
	DWORD MAXGpuTemp;         //MAX GPU Temperature
	DWORD MINGpuTemp;         //MIN GPU Temperature
	DWORD MAXEnvTemp;         //MAX ENV Temperature
	DWORD MINEnvTemp;         //MIN ENV Temperature
	DWORD CurD3DAA;           //Direct3D Antialiasing value
	DWORD CurD3DAF;           //Direct3D Anisotropy value
	DWORD CurOGLAA;           //OpenGL Antialiasing value
	DWORD CurOGLAF;           //OpenGL Anisotropy value
	
	DWORD IsActive;           //is 3d application active
	DWORD CurFPS;                     // current FPS
	DWORD FreeVideo;  //Free Video Memory
	DWORD FreeTexture;  //Free Texture Memory
	DWORD Cur3DApi;    //Current API used in applciation
	DWORD MemUsed;
}TATTData;


static TATTData *data = NULL;
static HANDLE hMapObject = NULL;


static int initATT ()
{
   	
	hMapObject = OpenFileMapping(FILE_MAP_ALL_ACCESS,TRUE,"ATITRAY_SMEM");	
	if (hMapObject!=0){
		data = (TATTData*)MapViewOfFile(hMapObject, FILE_MAP_WRITE, 0, 0, 0);
		return 1;
	}else{
		data = NULL;
		printf ("ATI Tray Tools not found\n");
		return 0;
	}	
}

static void closeATT ()
{
	if (data)
		UnmapViewOfFile(data);
	if (hMapObject)
		CloseHandle(hMapObject);
		
	hMapObject = NULL;
	data = NULL;
}

int renderATT (TFRAME *frame)
{

	lClearFrame(frame);
	lPrintf(frame, 0, 14, LFT_COURIERNEWCE8, LPRT_CPY,"Env:%dc", (int)data->ENVTemp);
	lPrintf(frame, 0, 24, LFT_COURIERNEWCE8, LPRT_CPY,"GPU:%dmhz", (int)data->CurGPU);
	lPrintf(frame, 0, 34, LFT_COURIERNEWCE8, LPRT_CPY,"Mem:%dmhz", (int)data->CurMEM);
		
	lPrintf(frame, 70, 24, LFT_COURIERNEWCE8, LPRT_CPY,"OGL");
	lPrintf(frame, 70, 34, LFT_COURIERNEWCE8, LPRT_CPY,"D3D");
	lPrintf(frame, 96, 24, LFT_COURIERNEWCE8, LPRT_CPY,"AA:%d", (int)data->CurOGLAA);
	lPrintf(frame, 96, 34, LFT_COURIERNEWCE8, LPRT_CPY,"AA:%d", (int)data->CurD3DAA);
	lPrintf(frame, 130, 24, LFT_COURIERNEWCE8, LPRT_CPY,"AF:%d", (int)data->CurOGLAF);
	lPrintf(frame, 130, 34, LFT_COURIERNEWCE8, LPRT_CPY,"AF:%d", (int)data->CurD3DAF);
		
	lPrintf(frame, 120, 14, LFT_COURIERNEWCE8, LPRT_CPY,"Fan:%d%%", (int)data->FanDuty);

	if (data->is3DActive || data->IsActive)
		lPrint(frame,"Mode:3D", 70, 14, LFT_COURIERNEWCE8, LPRT_CPY);
	else
		lPrint(frame,"Mode:2D", 70, 14, LFT_COURIERNEWCE8, LPRT_CPY);

	lPrintf(frame, 0, -3, LFTW_ROUGHT18, LPRT_CPY, "GPU:%dc", (int)data->GPUTemp);
	lPrintf(frame, 70, -3, LFTW_ROUGHT18, LPRT_CPY, "FPS:%d", (int)data->CurFPS);
	return 1;
}

int main ()
{

	if (!initATT())
		return 0;

	if (!initDemoConfig("config.cfg")) return 0;
	//int i=0;
	do{
		renderATT(frame);
		//if (!i++) lSaveImage (frame,"ati.bmp", IMG_BMP, frame->width, frame->height);
		lRefresh(frame);
		lSleep(400);
	}while (!kbhit());
				
	
	closeATT();
	demoCleanup();
	return 1;
}

