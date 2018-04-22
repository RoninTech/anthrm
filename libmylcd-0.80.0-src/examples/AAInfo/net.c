
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



#if defined (__WIN32__)
  #include <winsock2.h>
#else
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <netdb.h>
  
  typedef int SOCKET;
  #define SOCKET_ERROR -1
#endif

#include "aainfo.h"

#include <ipexport.h>


typedef HANDLE (WINAPI *Icmp_CreateFile) ();
typedef BOOL (WINAPI *Icmp_CloseHandle) (HANDLE IcmpHandle);
typedef DWORD (WINAPI *Icmp_SendEcho) (HANDLE IcmpHandle, struct in_addr, LPVOID RequestData, WORD RequestSize, PIP_OPTION_INFORMATION RequestOptns,LPVOID ReplyBuffer,DWORD ReplySize,DWORD Timeout);

static Icmp_CreateFile icmpCreateFile;
static Icmp_CloseHandle icmpCloseHandle;
static Icmp_SendEcho icmpSendEcho;

static HANDLE hLib = INVALID_HANDLE_VALUE;
static HANDLE hfile = INVALID_HANDLE_VALUE;
static char sendData[] = "echo";



int closeSocket (int socket)
{
	if (socket != SOCKET_ERROR){
		shutdown (socket,2);
		#if defined(__WIN32__)
			return closesocket(socket);
		#else
			return close(socket);
		#endif
	}else
		return SOCKET_ERROR;
}

int sendSocket (THOSTINFO *host, void *buffer, int *bsize)
{
	if (host->socket == SOCKET_ERROR){
		return SOCKET_ERROR;
	}else{
		int ret = sendto(host->socket, buffer, *bsize, 0, (SOCKADDR *)&host->serverName,  sizeof(struct sockaddr_in));
		if (ret > 0)
			host->rsent++;
		return ret;
	}

}

int readSocket (THOSTINFO *host, void *buffer, int bsize)
{
	unsigned long pendingdata = 0; 
	int ret = ioctlsocket(host->socket, FIONREAD, &pendingdata);
	if (!ret && pendingdata){
		int fromlen = sizeof(struct sockaddr_in);
		ret = recvfrom(host->socket, buffer, (size_t)bsize, 0, (SOCKADDR *)&host->serverName,  &fromlen);
		if (ret > 0)
			host->rrecv++;
		return ret;
	}else if (ret == SOCKET_ERROR){
	  	return SOCKET_ERROR;
	}else{
		return 0;
	}
}

void initSocket ()
{
	#if defined(__WIN32__)
		WSADATA wsa;
		WSAStartup(MAKEWORD(2,2),&wsa);
	#endif
}

int connectTo (char *addr, int port, struct sockaddr_in *serverName)
{
    struct hostent *hostPtr = NULL;
    SOCKET fdsocket = SOCKET_ERROR;

   	hostPtr = gethostbyname(addr);
   	if (hostPtr == NULL) {
        hostPtr = gethostbyaddr(addr, strlen(addr), PF_INET);
   	    if (hostPtr == NULL) {
       		printf("connectTo(): unable to resolve '%s' to an address\n",addr);
       		return SOCKET_ERROR;
		}
   	}
    	
	fdsocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (fdsocket == SOCKET_ERROR) {
        perror("connectTo(): socket error\n");
        return SOCKET_ERROR;
    }else {
		serverName->sin_family = AF_INET;
		serverName->sin_port = htons(port);
		memcpy(&serverName->sin_addr, hostPtr->h_addr, hostPtr->h_length);
		return (int)fdsocket;
	}
}

int initICMP ()
{
	hLib = LoadLibrary("icmp.dll");
	if (hLib){
		icmpCreateFile = (Icmp_CreateFile)GetProcAddress(hLib, "IcmpCreateFile");
		icmpCloseHandle = (Icmp_CloseHandle)GetProcAddress(hLib, "IcmpCloseHandle");
		icmpSendEcho = (Icmp_SendEcho)GetProcAddress(hLib, "IcmpSendEcho");
		
		if (icmpSendEcho && icmpSendEcho && icmpSendEcho){
			hfile = icmpCreateFile();
			return 1;
		}
	}
	return 0;
}

void closeICMP ()
{
	if (hfile != INVALID_HANDLE_VALUE)
		icmpCloseHandle(hfile);	
	hfile = INVALID_HANDLE_VALUE;
	if (hLib != INVALID_HANDLE_VALUE)
		FreeLibrary(hLib);
	hLib = INVALID_HANDLE_VALUE;
}

int getEcho (char *address, struct sockaddr_in *serverName)
{
	
#if 0
	struct hostent *hostPtr = NULL;
	hostPtr = gethostbyname(address);
	if (hostPtr == NULL) {
		hostPtr = gethostbyaddr(address, strlen(address), PF_INET);
		if (hostPtr == NULL) {
			printf("getEcho(): unable to resolve '%s' to an address\n",address);
    		return SOCKET_ERROR;
		}
	}
	struct sockaddr_in serverName;
	memcpy(&serverName.sin_addr, hostPtr->h_addr, hostPtr->h_length);
#endif

	void *ReplyBuffer = (void*)malloc(sizeof(ICMP_ECHO_REPLY) + sizeof(sendData));
	if (ReplyBuffer){
    	DWORD dwRetVal = icmpSendEcho(hfile, serverName->sin_addr, sendData, sizeof(sendData), NULL, ReplyBuffer, sizeof(sendData) + sizeof(ICMP_ECHO_REPLY), 1000);
    	if (dwRetVal != 0){
			PICMP_ECHO_REPLY pEchoReply = (PICMP_ECHO_REPLY)ReplyBuffer;
        	int triptime = (int)pEchoReply->RoundTripTime;
        	free(ReplyBuffer);
        	return triptime;
        }
    	free(ReplyBuffer);
        return -2;
    }
    return -3;
}

unsigned int getEchoTimePeriod (char *address, struct sockaddr_in *serverName)
{
	unsigned int t0 = timeGetTime();
	getEcho(address, serverName);
	return (unsigned int)timeGetTime()-t0;
}

