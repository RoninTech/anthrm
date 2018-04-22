
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


#ifndef _IRC_H_
#define _IRC_H_


#include <pthread.h>
#include <semaphore.h>

#include "mylcd.h"
#include "../../src/sync.h"
#include "../../src/g15/g15.h"


#define IRC_NICKLENGTHMAX	64

typedef struct {
	char	network[64];		// network name, as returned by server
	char	address[64];		// irc server host address
	int		port;				// irc server host port
}THOSTINFO;

typedef struct {
	char	text[512];			// a single unprocessed IRC message
	unsigned int size;				// message length
}TMESSAGE;

typedef struct {
	pthread_t	tid;
	int			socket;			// tcp/ip socket handle
	unsigned int	lastMessageSize;
	unsigned int	recvBufferSize;
	char		*recvBuffer;
	TMESSAGE	send;			// send to socket buffer
	TMESSAGE	format;			// presend - formatting buffering
}TCONNECTION;

typedef struct {
	int			msgTotal;		// number of messages extracted from packet
	int			queueTotal;		// total queue size
	TMESSAGE	queue[256];		// storage space for incomming '\r\n' seperated messages
	TMESSAGE	incomplete;		// storage space for an incomplete message when required
}TMESSAGEQUEUE;

typedef struct {
	char	nick[IRC_NICKLENGTHMAX];			// nickname on server
	char	altnick[IRC_NICKLENGTHMAX];		// nickname on server
	char	user[IRC_NICKLENGTHMAX];			// logon username
	char	info[128];			// realname, general info about self
	char	quitmsg[128];		// message to leave on quit
	char	partmsg[128];		// message to leave on channel part
	char	version[128];		// version reply info
}TCLIENTINFO;

typedef struct {
	char	text[512];
}TLINE;

typedef struct {
	char	text[512];	// channel topic
	char	who[64];	// channel topic set by
	// date
}TTOPIC;

typedef struct {
	TFRAME	*frame;		// frame to render
	char	*buffer;	// rendering scratch pad area
	unsigned int bufferSize;
	unsigned int lineLength;	// size of each line, in bytes
	unsigned int lineTotal;	// number of lines
	TLINE	line[32];	// circular buffer
	int		position;	// next line to render on to within ring buffer.
	int		pass;		// number of passes of ring buffer
}TRENDER;				// this line will render at bottom of frame

typedef struct {
	unsigned int active:1;
	unsigned int fill:31;
	char	nick[IRC_NICKLENGTHMAX];
	//TCHANNEL	**list;
}TUSER;

typedef struct {
	unsigned int status:1;	// 
	unsigned int fill:31;	//
	TFRAME *frame;
	unsigned int time;
	unsigned int displaytime;
}TOVERLAY;

typedef struct {
	unsigned int active:1;	// is channel valid, ie, this client is in this channel
	unsigned int part:1;		// join/part status, part = 1, join = 0
	unsigned int priv:1;
	unsigned int update:1;
	unsigned int pause:1;
	unsigned int fill:27;
	int		index;		// index of this struct within channel list
	char	name[64];	// channel name
	int		font;		// libmylcd font id (LFTW_xxx or LFT__xxx)
	int		yoffset;	// view window offset
	int		userTotal;	// total people in channel
	int		userListTotal; // list total
	TUSER	**user;
	TTOPIC	topic;		//
	TRENDER	render;		// 
	TOVERLAY overlay;
	TFRAME	*infoFrame;	//general chan info
	TFRAME	*userListFrame;
}TCHANNEL;

typedef struct {
	char	master[64];			// authenticated user
	unsigned int		authenticated:1;	// a user has authenticated with bot
	unsigned int		fill:31;
}TBOT;

typedef struct {
	char	ownerNick[IRC_NICKLENGTHMAX];		// bot owner
	char	ownerAddr[64];		// address
	char	ownerIP[64];		// ip address
	char	authKey[64];		// auth password
}TCONFIGBOT;

typedef struct {
	char	channel[32][64];
	int		channels;
	int		renderflags;
	unsigned int		joins:1;
	unsigned int		quits:1;
	unsigned int		kicks:1;
	unsigned int		ctcps:1;
	unsigned int		query:1;
	unsigned int		mode:1;
	unsigned int		invite:1;
	unsigned int		autojoininvite:1;
	unsigned int		nickchange:1;
	unsigned int		notices:1;
	unsigned int		skipmotd:1;
	unsigned int		querywin:1;
	unsigned int		autorejoin:1;
	unsigned int		wordwrap:1;
	unsigned int		fill:18;
	
	TCONFIGBOT	bot;
}TCONFIG;


typedef struct {
	TMESSAGEQUEUE	cmds;
	THOSTINFO		host;
	TCLIENTINFO		client;
	TCONNECTION		connection;
	int				channelTotal;
	TCHANNEL		**channel;		// list of channels
	TCHANNEL		*toppage;		// channel in view, or status page
	TCHANNEL		status;			// overview page/window/channel
	TCONFIG			config;
	TBOT			bot;
	int				currentChanPage;
	int				state;			// runtime state
}TIRCMYLCD;


#define MIN(a, b) a<b?a:b
#define MAX(a, b) a>b?a:b


TIRCMYLCD *newIRC ();
void freeIRC (TIRCMYLCD *irc);
int initIRC (TIRCMYLCD *irc);
void closeConnection (TIRCMYLCD *irc);

void renderChanInfo (TIRCMYLCD *irc, TCHANNEL *channel, TFRAME *frame);
void renderIRC (TIRCMYLCD *irc, TCHANNEL *channel);
int renderText (TCHANNEL *channel, TFRAME *frame, char *buffer, int flags, int font);
void drawUserList (TCHANNEL *channel, TFRAME *frame);
char *formatRenderBuffer (TCHANNEL *channel, int from, int to, char *buffer);
void drawChannel (TIRCMYLCD *irc, TCHANNEL *channel);

DWORD g15keycb (int device, DWORD dwButtons, TMYLCDG15 *mylcdg15);
void messageProcesser (void *ptr);

int onKeyHit (TIRCMYLCD *irc, int key);
void onChanPrevious (TIRCMYLCD *irc);
void onChanScrollUp (TIRCMYLCD *irc);
void onChanScrollDown (TIRCMYLCD *irc);
void onChanNext (TIRCMYLCD *irc);
void onChanInfo (TIRCMYLCD *irc, int time);
void onChanPart (TIRCMYLCD *irc, TCHANNEL *channel);



#include "lock.h"
#include "bot.h"
#include "send.h"
#include "net.h"
#include "network.h"
#include "g15.h"
#include "config.h"



#endif

