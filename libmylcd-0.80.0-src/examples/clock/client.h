
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


#if defined (BUILDDLL)
# define EXPORT __declspec(dllexport) 
  typedef unsigned char ubyte;
  static int MIN (int a, int b)
  {
	if (a < b)
		return a;
	else
		return b;
  }
#else
# define EXPORT   
#endif

EXPORT SOCKET netConnect (TMPNET *net, char *addr, int port, int proto);
EXPORT void netShutdown ();
EXPORT void netInit ();

EXPORT int netPing (TMPNET *net);	// blocking function

EXPORT int netCloseSocket (SOCKET socket);
EXPORT int netSendSocket (TMPNET *net, void *buffer, size_t *bsize);
EXPORT int netReadSocket (TMPNET *net, void *buffer, size_t bsize);
EXPORT int netReadSocketPeek (TMPNET *net, void *buffer, size_t bsize);
EXPORT int netBindPort (TMPNET *net, int port);
EXPORT SOCKET netWaitForConnection (SOCKET server, SOCKADDR_IN *sockaddrin);

EXPORT int netEmptyReadBuffer (TMPNET *net);

EXPORT size_t netIsPacketAvailable (TMPNET *net);

EXPORT int netGetMPName (TMPNET *net, void *buffer, size_t buffersize);
EXPORT int netGetMPHandle (TMPNET *net, HANDLE *handle);
EXPORT int netGetTitle (TMPNET *net, int index, int type, void *buffer, size_t buffersize);
EXPORT int netGetCurrentTrackPosition (TMPNET *net);

EXPORT int netGetPluginVersion (TMPNET *net, int *v1, int *v2, int *v3);
EXPORT int netGetMPVersion (TMPNET *net, int *version);
EXPORT int netGetPlayState (TMPNET *net, int *state);
EXPORT int netGetVolume (TMPNET *net, int *volume);
EXPORT int netSetVolume (TMPNET *net, unsigned int volume);
EXPORT int netGetVolumeEx (TMPNET *net, int *volume, int userData);

EXPORT int netMCtrlStop (TMPNET *net);
EXPORT int netMCtrlNext (TMPNET *net);
EXPORT int netMCtrlPrevious (TMPNET *net);
EXPORT int netMCtrlPlay (TMPNET *net);

EXPORT int netPlayTrackIndex (TMPNET *net, unsigned int trkindex, unsigned int plindex);
EXPORT int netTrackRewind (TMPNET *net, unsigned int time);
EXPORT int netTrackForward (TMPNET *net, unsigned int time);
EXPORT int netGetWaveData (TMPNET *net, int control, TMPCMDWAVE *wave);
EXPORT int netGetMetaData (TMPNET *net, int metaindex, int metacmd, int playlistindex, void *meta, void *buffer, size_t *buffersize);
EXPORT int netEnqueueFile (TMPNET *net, int type, char *path, int play);
EXPORT int netGetFilename (TMPNET *net, int playlistindex, int type, void *buffer, size_t buffersize);
EXPORT int netSetEQData (TMPNET *net, TMPCMDEQ *eq);
EXPORT int netGetEQData (TMPNET *net, TMPCMDEQ *eq);
EXPORT int netGetCurrentTrackInfo (TMPNET *net, TMPGENTRACKINFO *gen);


/*****************************/
/*   Multiple Playlist API   */

EXPORT int netGetTotalPlaylists (TMPNET *net, int *totalplaylist);

EXPORT int netGetPlaylistName (TMPNET *net, int playlist, wchar_t *buffer, size_t buffersize);
EXPORT int netGetPlaylistTrackTotal (TMPNET *net, int playlist, int *totaltracks);
EXPORT int netGetPlaylistTrackTitle (TMPNET *net, int playlist, wchar_t *buffer, size_t buffersize);
EXPORT int netGetPlaylistTrackPath (TMPNET *net, int playlist, wchar_t *buffer, size_t buffersize);
EXPORT int netGetPlaylistTrackAlbum (TMPNET *net, int playlist, wchar_t *buffer, size_t buffersize);
EXPORT int netGetPlaylistTrackArtist (TMPNET *net, int playlist, wchar_t *buffer, size_t buffersize);


/*****************************/


