
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


#ifndef _SYSINFO_H_
#define _SYSINFO_H_


#ifdef __cplusplus
extern "C" {
#endif


#define u64 __int64

// using undocumented functions and structures

#define SystemBasicInformation		0
#define	SystemPerformanceInformation	2
#define SystemTimeInformation		3

#define Li2Double(x)	((double)((x).HighPart) * 4.294967296E9 + (double)((x).LowPart))

typedef struct
{
	DWORD	dwUnknown1;
	ULONG	uKeMaximumIncrement;
	ULONG	uPageSize;
	ULONG	uMmNumberOfPhysicalPages;
	ULONG	uMmLowestPhysicalPage;
	ULONG	UMmHighestPhysicalPage;
	ULONG	uAllocationGranularity;
	PVOID	pLowestUserAddress;
	PVOID	pMmHighestUserAddress;
	ULONG	uKeActiveProcessors;
	BYTE	bKeNumberProcessors;
	BYTE	bUnknown2;
	WORD	bUnknown3;
} SYSTEM_BASIC_INFORMATION;

typedef struct
{
	LARGE_INTEGER	liIdleTime;
	DWORD		dwSpare[76];
} SYSTEM_PERFORMANCE_INFORMATION;

typedef struct
{
	LARGE_INTEGER	liKeBootTime;
	LARGE_INTEGER	liKeSystemTime;
	LARGE_INTEGER	liExpTimeZoneBias;
	ULONG			uCurrentTimeZoneID;
	DWORD			dwReserved;
} SYSTEM_TIME_INFORMATION;

// NtQuerySystemInformation
// The function copies the system information of the specified type into a buffer
// NTSYSAPI 
// NTSTATUS
// NTAPI
// NtQuerySystemInformation(
//		IN UINT SystemInformationClass,		// information type
//		OUT PVOID SystemInformation,		// pointer to buffer
//		IN ULONG SystemInformationLength,	// buffer size in bytes
//		OUT PULONG ReturnLength OPTIONAL	// pointer to a 32 bit variable that
//											// receives the number of bytes written
//											// to the buffer
// );

typedef LONG (WINAPI *PROCNTQSI) (UINT, PVOID, ULONG, PULONG);

// this functions return the CPU usage of one second using
// undocumented Windows NT API's
// this code will not run on Win 9x


typedef struct{
	unsigned int Weeks;
	unsigned int Days;
	unsigned int Hours;
	unsigned int Minutes;
	unsigned int Seconds;
}TUPT;

typedef struct{
	unsigned int ramtotal;
	unsigned int ramused;
	float pctused;
}TMEMI;


u64 GetCPUSpeed();
void GetMemInfo (TMEMI *memInfo);
void GetUpTime (TUPT *upTime);
void ComputerName (char *buffer, unsigned long bufferlen);
void UserName (char *buffer, unsigned long bufferlen);
int GetDrives (char *buffer, unsigned long len, u64 *freeSpace);
u64 GetFreeDriveSpace (char *DirectoryName);
u64 GetTotalDriveSpace (char *DirectoryName);
double GetCPUUsage();

int main ();
void openQSI();
void closeQSI();

#ifdef __cplusplus
}
#endif

#endif

