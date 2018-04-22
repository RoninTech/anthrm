
// WVS
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

#ifndef _NET_H_
#define _NET_H_



const int MAGIC = 0xF0DEF235;

#define TCPPORT				43675	// set communication port (UDP)
#define TCPPORTSTR		 	"43675"

#define MY_PORT	TCPPORT
#define MY_VERSION1 0x1009	// protocol version
#define MY_VERSION2 30		// app version mj
#define MY_VERSION3 11		// app version mi
#define MY_VERSIONSTR "0.30.12b6"

#define MAXCHANNELLENGTH 576


#define CMD_GET_PLNAME			1	// get plugin name
#define CMD_GET_PLVERSION		2	// get plugin version

#define CMD_GET_MPNAME			3	// get name of media player (Winamp, XM, iTunes, etc..)
#define CMD_GET_MPVERSION		4	// get media player version
#define CMD_GET_MPHANDLE		5	// get media player window handle (if available)

#define CMD_SHUTDOWN			6	// request media player shutdown

#define CMD_GET_MPVOLUME		7	// get media player volume
#define CMD_SET_MPVOLUME		8	// set media player volume

#define CMD_GET_PLAYSTATUS		9	// is track playing (1), paused(3) or stopped(0)

//keyboard media button control (play, stop, etc..)
#define CMD_MCTL_STOP			10
#define CMD_MCTL_PLAY			11
#define CMD_MCTL_FORWARD		12
#define CMD_MCTL_REWIND			13
#define CMD_MCTL_NEXT			14
#define CMD_MCTL_PREVIOUS		15

#define CMD_GET_TRACKLENGTH		16	// get length of currently playing track (n Ms)
#define CMD_GET_BITRATE			17	// get current track bitrate
#define CMD_GET_CHANNELS		18	// get current track channels
#define CMD_GET_SAMPLERATE		19	// get current track sample rate (n Khz)
#define CMD_GET_POSITION		20	// get current track position (n Ms)
#define CMD_GET_CURRENTTRKINFO	21	// all 6 above in one response
#define CMD_SET_POSITION		22	// set current track position
// above commands should also be sent with index of track, though Winamp does not require this
// (index of currently playing track is maintained by the client)

#define CMD_GET_PLAYLISTPOSITION	23	// get playlist position
#define CMD_GET_PLAYLISTTOTAL		24	// get total tracks in playlist

#define CMD_GET_PLAYLISTTITLEW		25	// get complete playlist titles
#define CMD_GET_PLAYLISTTITLEA		26	// get complete playlist titles
#define CMD_GET_PLAYLISTFILENAMEW	27  // get complete playlist filenames
#define CMD_GET_PLAYLISTFILENAMEA	28  // get complete playlist filenames

#define CMD_GET_TRACKTITLEW			29	// get track title by index, UTF16
#define CMD_GET_TRACKTITLEA			30	// get track title by index
#define CMD_GET_TRACKFILENAMEW		31	// get track filename by index, UTF16
#define CMD_GET_TRACKFILENAMEA		32	// get track filename by index

#define CMD_PLAYTRACKINDEX			33	// play track from playlist

#define CMD_ENQUEUEFILEA			34	// enqueue file, then play if data2 = 1
#define CMD_ENQUEUEFILEW			35	// enqueue file, then play if data2 = 1

#define CMD_GET_EQDATA				36	// get EQ data
#define CMD_SET_EQDATA				37 	// set EQ data

#define CMD_GET_METADATAA			38	// get id3v2 tag info
#define CMD_GET_METADATAW			39	// get id3v2 tag info, UTF16

#define CMD_GET_SPECTRUMDATA		40	// get spectrum or/and wave data


/**********************************************************/
/* Multiple Playlist API, for players such as MediaMonkey */
/* All strings are UTF16                                  */
/* data1 = playlist index (0.. to total-1)                */
/* data2 = track index                                    */

 #define CMD_GET_TOTALPLAYLISTS		41	// get total number of playlists
 #define CMD_GET_PLAYLISTNAME		42	// get name of selected playlist

 #define CMD_GET_PLAYLISTTRKTOTAL	43	// get total tracks in playlist
 #define CMD_GET_PLAYLISTTRKTITLE	44	// get formatted title of track from selected playlist
 #define CMD_GET_PLAYLISTTRKARTIST	45	// get artist name of selected track
 #define CMD_GET_PLAYLISTTRKALBUM	46	// get album name of selected track
 #define CMD_GET_PLAYLISTTRKPATH	47	// get path to media file

/**********************************************************/

#define CMD_HOSTIPC					48
#define CMD_HOSTEXIT	 			49
#define CMD_TOTAL					CMD_HOSTEXIT


#define CMDSTRUCT_CMD		1
#define CMDSTRUCT_WAVE		2
#define CMDSTRUCT_GEN		3
#define CMDSTRUCT_EQ		4


typedef struct {
	unsigned char command;
	unsigned short data3;
	unsigned int data1;
	unsigned int data2;
	unsigned int magic;
}__attribute__ ((packed))TMPCMD;

typedef struct{
	unsigned char command;
	unsigned char control;
	unsigned char channels;		// channels in buffer stream per set
	size_t len;
	short bpc;					// bytes per channel
	int sRate;
	unsigned int magic;
	unsigned char data[2*2*MAXCHANNELLENGTH];		// 2 data sets (spec&wave) each with 2 channels with 576 bytes per channel
}__attribute__ ((packed))TMPCMDWAVE;

typedef struct {
	unsigned char command;
	unsigned int length;		// length of track
	unsigned int position;		// current playing position
	short bitrate;
	unsigned char channels;
	unsigned char samplerate;
	int	playlistpos;			// index in to playlist
	int totaltracks;
	unsigned int magic;
}__attribute__ ((packed))TMPGENTRACKINFO;

typedef struct {
	unsigned char command;
	short first;
	short last;
	unsigned char band[16];
	unsigned int magic;
}__attribute__ ((packed))TMPCMDEQ;

typedef struct {
	volatile SOCKET server;		// me/host
	volatile SOCKET client;		// 'accept'ed socket from client
}TMPSOCKET;

typedef struct {
	SOCKADDR_IN sockaddrin;
	TMPSOCKET socket;
	int port;
	volatile int serverState;	// used by host
	volatile int clientState;	// used by host
	unsigned int sendCt;
	unsigned int readCt;
}TMPNET;




#endif

