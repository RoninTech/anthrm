
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
#include <windows.h>
#include "mylcd.h"
#include "sysinfo.h"
#include "demos.h"


MYLCD_EXPORT void MYLCD_APICALL cpuid();
MYLCD_EXPORT u64 MYLCD_APICALL rdtsc();

PROCNTQSI NtQuerySystemInformation={0};
HANDLE hLibNTDLL=NULL;


int main ()
{
	if (!initDemoConfig("config.cfg"))
		return 0;
		
	int BLACK = lGetRGBMask(frame, LMASK_BLACK);
	int WHITE = lGetRGBMask(frame, LMASK_WHITE);
	hw->render->backGround = BLACK;
	hw->render->foreGround = WHITE;
		
	openQSI();

    TUPT ut;
    TMEMI mi;
    u64 freeSpace = 0;
    static char pcname[128],user[128],drives[512];
    
    ComputerName(pcname, sizeof(pcname));
    UserName(user,sizeof(user));
    u64 cpuspeed = GetCPUSpeed()/1000/1000;
    double cpu=0.0;

	do{
		lClearFrame(frame);
		
    	GetMemInfo(&mi);    
    	GetUpTime(&ut);
    	GetDrives(drives, sizeof(drives), &freeSpace);

    	lPrintf(frame, 0,0, LFT_COMICSANSMS7X8, LPRT_CPY, "PC:%s  User:%s",pcname,user);
    	lPrintf(frame, 0,11, LFT_COMICSANSMS7X8, LPRT_CPY, "Drives: %s",drives);
    	lPrintf(frame, 0,22, LFT_COMICSANSMS7X8, LPRT_CPY, "Total Free Space: %7.0fmb",freeSpace/1024.0/1024.0);
		lPrintf(frame, 0,33, LFT_COMICSANSMS7X8, LPRT_CPY, "Uptime: %iw %id %ih %im %is",ut.Weeks,ut.Days,ut.Hours,ut.Minutes,ut.Seconds);
		lPrintf(frame, 0,44, LFT_COMICSANSMS7X8, LPRT_CPY, "CPU: %I64dmhz  %4.1f%%",cpuspeed,cpu);
		lPrintf(frame, 0,55, LFT_COMICSANSMS7X8, LPRT_CPY, "Mem: %i/%i  (%4.1f%%)",mi.ramused,mi.ramtotal,mi.pctused);
    	
    	lRefresh(frame);
    	cpu = GetCPUUsage(); // GetCPUUsage() sleeps for one second, quite handy
    }while(!kbhit());
    
    closeQSI();
    demoCleanup();
    return 0;
}

void GetUpTime (TUPT *ut)
{
	unsigned long lintTicks = GetTickCount();
	ut->Seconds = (lintTicks / 1000) % 60;
	ut->Minutes = ((lintTicks / 1000) / 60) % 60;
	ut->Hours = (((lintTicks / 1000) / 60) / 60) % 24;
	ut->Days = ((((lintTicks / 1000) / 60) / 60) / 24) % 7;
    ut->Weeks = (((((lintTicks / 1000) / 60) / 60) / 24) / 7) % 52;
}   

void GetMemInfo (TMEMI *mi)
{
	MEMORYSTATUS memi;

	GlobalMemoryStatus(&memi);
	mi->ramtotal = (((memi.dwTotalPhys / 1024) / 1024) + 1);
	mi->ramused = mi->ramtotal - (((memi.dwAvailPhys / 1024) / 1024) + 1);
	mi->pctused = (((float)mi->ramused / (float)mi->ramtotal)*100);
}

u64 GetCPUSpeed()
{
	LARGE_INTEGER t1,t2,tf;
	u64 c1,c2;
	volatile int i;

	QueryPerformanceFrequency(&tf);
	QueryPerformanceCounter(&t1);

	c1 = rdtsc();
	for (i=1000000; i--;);
	QueryPerformanceCounter(&t2);
	c2 = rdtsc();

	return ((c2 - c1) * tf.QuadPart / (t2.QuadPart - t1.QuadPart));
}

void ComputerName (char *buffer, unsigned long bufferlen)
{
	unsigned long len=bufferlen;
	GetComputerNameA(buffer,&len);
	GetUserNameA(buffer,&len);
}

void UserName (char *buffer, unsigned long bufferlen)
{
	unsigned long len=bufferlen;
	GetUserNameA (buffer,&len);
}

int GetDrives (char *buffer, unsigned long len, u64 *freeSpace)
{
	int i,ct=0;
	char drives[len];
	memset(drives,0,len);
	int total = GetLogicalDriveStringsA (len, drives);
	if (freeSpace) *freeSpace = 0;
	int d=0;
	int dtype;

	for (i=0;i<total;i++){
		if (*(drives+i) == 0){
			dtype = GetDriveType(drives+i-3);
			if ((dtype != DRIVE_REMOVABLE) && (dtype != DRIVE_CDROM)){
				buffer[d++]=*(drives+i-3);
				buffer[d++]=':';
				buffer[d++]=' ';
				buffer[d] = 0;
				if (freeSpace)
					*freeSpace += GetFreeDriveSpace((drives+i-3));
			}
			*(drives+i) = 32;
			ct++;
		}
	}
	return ct;
}

u64 GetFreeDriveSpace (char *DirectoryName)
{
	u64 lpFreeBytesAvailable;
	u64 lpTotalNumberOfBytes;
	u64 lpTotalNumberOfFreeByte;
	GetDiskFreeSpaceEx(DirectoryName,(PULARGE_INTEGER)&lpFreeBytesAvailable,(PULARGE_INTEGER)&lpTotalNumberOfBytes,(PULARGE_INTEGER)&lpTotalNumberOfFreeByte);

	return lpTotalNumberOfFreeByte;
}

u64 GetTotalDriveSpace (char *DirectoryName)
{
  u64 lpFreeBytesAvailable;
  u64 lpTotalNumberOfBytes;
  u64 lpTotalNumberOfFreeByte;
  GetDiskFreeSpaceEx(DirectoryName,(PULARGE_INTEGER)&lpFreeBytesAvailable,(PULARGE_INTEGER)&lpTotalNumberOfBytes,(PULARGE_INTEGER)&lpTotalNumberOfFreeByte);
  
 return lpTotalNumberOfBytes;
}

double GetCPUUsage()
{
	SYSTEM_BASIC_INFORMATION SysBaseInfo;
	SYSTEM_TIME_INFORMATION SysTimeInfo;
	SYSTEM_PERFORMANCE_INFORMATION SysPerfInfo;
	LARGE_INTEGER liOldIdleTime={{0,0}};
	LARGE_INTEGER liOldSystemTime={{0,0}};
	LONG status;
	double dbIdleTime=0.0;
	double dbSystemTime=0.0;

	if (NtQuerySystemInformation) {
		status = NtQuerySystemInformation(SystemBasicInformation, &SysBaseInfo, 
			sizeof(SysBaseInfo), NULL);

		if (status == NO_ERROR) {
			// get system time
			status = NtQuerySystemInformation(SystemTimeInformation, &SysTimeInfo, 
				sizeof(SysTimeInfo), NULL);

			if (status == NO_ERROR) {
				// get system idle time
				status = NtQuerySystemInformation(SystemPerformanceInformation,
					&SysPerfInfo, sizeof(SysPerfInfo), NULL);

				if (status == NO_ERROR) {
					liOldIdleTime = SysPerfInfo.liIdleTime;
					liOldSystemTime = SysTimeInfo.liKeSystemTime;

					// wait one second
					lSleep(1000);

					// get new System time
					status = NtQuerySystemInformation(SystemTimeInformation, &SysTimeInfo,
						sizeof(SysTimeInfo), NULL);

					if (status == NO_ERROR) {
						// get new system idle time

						status = NtQuerySystemInformation(SystemPerformanceInformation,
							&SysPerfInfo, sizeof(SysPerfInfo), NULL);

						if (status == NO_ERROR) {
							// current value = new value - old value
							dbIdleTime = Li2Double(SysPerfInfo.liIdleTime) - Li2Double(liOldIdleTime);
							dbSystemTime = Li2Double(SysTimeInfo.liKeSystemTime) - Li2Double(liOldSystemTime);

							// currentCpuIdle = IdleTime / SystemTime;
							dbIdleTime = dbIdleTime / dbSystemTime;

							// currentCpuUsage% = 100 - (CurrentCpuIdle * 100) / NumberOfProcessors
							dbIdleTime = 100.0 - dbIdleTime * 100.0 / (double)SysBaseInfo.bKeNumberProcessors;
						}
					}
				}
			}
		}
	}

	return dbIdleTime;
}

void openQSI()
{
	hLibNTDLL = GetModuleHandle("ntdll");
	if (hLibNTDLL)
		NtQuerySystemInformation = (PROCNTQSI)GetProcAddress(hLibNTDLL, "NtQuerySystemInformation");
}

void closeQSI()
{
	CloseHandle (hLibNTDLL);
}
