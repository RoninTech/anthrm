
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

#include "irc.h"


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

int sendSocket (int socket, void *buffer, int *bsize)
{
	if (socket == SOCKET_ERROR)
		return SOCKET_ERROR;

	int sent = 0;
	int total;
	for (total = 0; total < *bsize;){
		#if defined(__WIN32__)
			sent = send(socket, buffer+total, *bsize-total, 0);
		#else
			sent = write(socket, buffer+total, *bsize-total);
		#endif

		if (sent == SOCKET_ERROR){
			*bsize = total;
			return SOCKET_ERROR;
		}else{
			total += sent;
		}
	}
	*bsize = total;
	return total;
}

int readSocket (int socket, void *buffer, int bsize)
{
  #if defined(__WIN32__)
	  return recv(socket, buffer, bsize, 0);
  #else
	  return read(socket, buffer, bsize);
  #endif
}

void initSocket ()
{
  #if defined(__WIN32__)
	WSADATA wsa;
	WSAStartup(MAKEWORD(2,2),&wsa);
  #endif
}

int connectTo (char *addr, int port, int type)
{
    struct hostent *hostPtr = NULL;
    struct sockaddr_in serverName = {0};
    SOCKET fdsocket = SOCKET_ERROR;


	//use a mask
	if (!(port&0xFFFF) || !addr){
		printf("connectTo(): invalid address/port\n");
		return SOCKET_ERROR;
	}


	if (type == IPPROTO_TCP)
    	fdsocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    else if (type == IPPROTO_UDP)
    	fdsocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    else
    	return SOCKET_ERROR;

    if (fdsocket == SOCKET_ERROR) {
        perror("connectTo(): socket error\n");
        return SOCKET_ERROR;
    }

    hostPtr = gethostbyname(addr);
    if (hostPtr == NULL) {
        hostPtr = gethostbyaddr(addr, strlen(addr), PF_INET);
        if (hostPtr == NULL) {
        	printf("connectTo(): unable to resolve '%s' to an address\n",addr);
        	return SOCKET_ERROR;
        }
    }

    serverName.sin_family = PF_INET;
    serverName.sin_port = htons(port);
    (void)memcpy(&serverName.sin_addr, hostPtr->h_addr, hostPtr->h_length);

    if (connect(fdsocket,(struct sockaddr*)&serverName, sizeof(serverName)) == SOCKET_ERROR)
        return SOCKET_ERROR;

    return (int)fdsocket;
}


