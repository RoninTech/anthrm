/************************************ 
* KneegoLCD with myLCD-library
* For KS0108 128x64
* Made by Hung Ki Chan
* One cloudy day in October, 2005
************************************/
/*----------------------------------
* Content:
* Analog Clock with current date MM/DD/YY
* Availible space in Mb on C: drive
* Percentage use of total Memory
*----------------------------------*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <windows.h>
#include "mylcd.h"
#include "sysinfo.h"
#include "demos.h"

void update_time (char *s, int len);
int return_time(int choice);

typedef BOOL (WINAPI *PGETDISKFREESPACEEX)(LPCSTR,
   PULARGE_INTEGER, PULARGE_INTEGER, PULARGE_INTEGER);

BOOL MyGetDiskFreeSpaceEx(LPCSTR pszDrive, char carr[13]);

PROCNTQSI NtQuerySystemInformation={0};
HANDLE hLibNTDLL=NULL;


int main (int argc, char *argv[])
{
	if (!initDemoConfig("config.cfg"))
		return 0;
		
	int BLACK = lGetRGBMask(frame, LMASK_BLACK);
	int WHITE = lGetRGBMask(frame, LMASK_WHITE);
	hw->render->backGround = BLACK;
	hw->render->foreGround = WHITE;
	
    static MEMORYSTATUS stat;
    float constant1 = -3.14159265/6;        // minus for clockwise count
    int hour, min, sec, x, y;
    double CPUuse=0;
    char date[12], charstring[12];

    openQSI();

    do{
		lClearFrame(frame);
		_strdate(date);
    	lDrawCircle(frame, 30, 30, 30, WHITE);
		lPrint(frame, date, 12, 43, LFT_SMALLFONTS7X7,  LPRT_CPY);
	
		hour=return_time(1)%12-3;
		x=ceil(30+13*cos(hour*constant1));
		y=ceil(30-13*sin(hour*constant1));
		lDrawLine(frame, 30, 30, x, y, WHITE);
	
		min=return_time(2)-15;
		x=ceil(30+20*cos(min*constant1/5.0));
		y=ceil(30-20*sin(min*constant1/5.0));
		lDrawLine(frame, 30, 30, x, y, WHITE);
	
		sec=return_time(3)-15;
		x=ceil(30+27*cos(sec*constant1/5.0));
		y=ceil(30-27*sin(sec*constant1/5.0));
		lDrawLine(frame, 30, 30, x, y, WHITE);

		MyGetDiskFreeSpaceEx ("C:",charstring);
		lPrint(frame, "Free space on C:", 64, 1, LFT_SMALLFONTS7X7, LPRT_CPY);
		lPrintf(frame, 64,10, LFT_SMALLFONTS7X7, LPRT_CPY,"%smb",charstring);
    
		GlobalMemoryStatus (&stat);
		lPrint(frame, "% Memory usage:", 64, 20, LFT_SMALLFONTS7X7, LPRT_CPY);
		lPrintf(frame, 64,29, LFT_SMALLFONTS7X7, LPRT_CPY,"%4.0f",(float)stat.dwMemoryLoad);
		//lDrawPBar(frame, 78, 29, 50, 7,(float)stat.dwMemoryLoad,PB_BORDER_HBOX|PB_MARKER_HFILL, WHITE);
    
		CPUuse=GetCPUUsage();
		lPrint(frame, "% CPU usage:", 64, 40, LFT_SMALLFONTS7X7, LPRT_CPY);
		lPrintf(frame, 64,49, LFT_SMALLFONTS7X7, LPRT_CPY, "%4.0f",CPUuse);
		//lDrawPBar(frame, 78, 49, 50, 7,(float)CPUuse,PB_BORDER_HBOX|PB_MARKER_HFILL, WHITE);

		lRefresh(frame);
    	//sleep(500); sleep contained within 'GetCPUUsage()'
    }while(!kbhit());
    
    closeQSI();
    demoCleanup();
	return 0;
}

void update_time (char *s, int len)     //digital clock, not needed
{
	time_t t = time(0);
	struct tm *tdate = localtime(&t);
	strftime(s, len, "%H:%M:%S", tdate);
}

int return_time(int choice)
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
    }
    return 0;
}

BOOL MyGetDiskFreeSpaceEx(LPCSTR pszDrive, char carr[13])
{
	PGETDISKFREESPACEEX pGetDiskFreeSpaceEx;
	__int64 i64FreeBytesToCaller, i64TotalBytes, i64FreeBytes;
	DWORD dwSectPerClust, dwBytesPerSect,  dwFreeClusters,  dwTotalClusters;
	BOOL fResult;

	HANDLE hLib = GetModuleHandle("kernel32.dll");
	pGetDiskFreeSpaceEx = (PGETDISKFREESPACEEX) GetProcAddress(GetModuleHandle("kernel32.dll"), "GetDiskFreeSpaceExA");

	if (pGetDiskFreeSpaceEx && hLib) { //for drives larger than 2GB
		fResult = pGetDiskFreeSpaceEx (pszDrive, (PULARGE_INTEGER)&i64FreeBytesToCaller, (PULARGE_INTEGER)&i64TotalBytes, (PULARGE_INTEGER)&i64FreeBytes);
		CloseHandle (hLib);	

		// Process GetDiskFreeSpaceEx results.
		if(fResult) {
			//printf("Total free bytes = %I64d\n", i64FreeBytes);   //for debug only
			int freespaceMB = 0;
			freespaceMB = i64FreeBytes / (1024*1024);
			//printf("Total free MB = %d\n", freespaceMB);   //debug
			itoa(freespaceMB, carr, 10);
		}
		return fResult;
	}else{              //old windows compatible 
		fResult = GetDiskFreeSpaceA (pszDrive,&dwSectPerClust,&dwBytesPerSect,&dwFreeClusters,&dwTotalClusters);

   		// Process GetDiskFreeSpace results.
		/*if (fResult) {
			printf("Total free bytes = I64d\n", dwFreeClusters*dwSectPerClust*dwBytesPerSect);
		}*/
		return fResult;
	}
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

					// update period in ms
					lSleep(500);     //need adjust or maybe not ???

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
