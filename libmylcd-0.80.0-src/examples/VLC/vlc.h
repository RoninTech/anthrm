
// libmylcd - http://mylcd.sourceforge.net/
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
//  GNU LIBRARY GENERAL PUBLIC LICENSE for more details.
//
//	You should have received a copy of the GNU Library General Public
//	License along with this library; if not, write to the Free
//	Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.



#ifndef _VLC__H_
#define _VLC__H_

const unsigned char *vlc_getVersion ();
const unsigned char *vlc_getCompiler ();
int vlc_getVolume (TVLCCONFIG *vlc);
void vlc_setMute (TVLCCONFIG *vlc, const int status);
int vlc_getMute (TVLCCONFIG *vlc);
void vlc_pause (TVLCCONFIG *vlc);
void vlc_play (TVLCCONFIG *vlc);
void vlc_stop (TVLCCONFIG *vlc);
void vlc_setPosition (TVLCCONFIG *vlc, float position);
float vlc_getPosition (TVLCCONFIG *vlc);
int vlc_getChapter (TVLCCONFIG *vlc);
void vlc_setChapter (TVLCCONFIG *vlc, int chapter);
void vlc_setVolume (TVLCCONFIG *vlc, int volume);
int vlc_getState (TVLCCONFIG *vlc);
libvlc_time_t vlc_getLength (TVLCCONFIG *vlc);
libvlc_media_t *vlc_new_mrl (TVLCCONFIG *vlc, const char *mediaPath);
libvlc_media_t *vlc_new_path (TVLCCONFIG *vlc, const char *mediaPath);
libvlc_media_t *vlc_new_node (TVLCCONFIG *vlc, const char *mediaPath);
libvlc_media_player_t *vlc_newFromMedia (TVLCCONFIG *vlc);
void vlc_release (TVLCCONFIG *vlc);
void vlc_mediaRelease (TVLCCONFIG *vlc);
int vlc_canPause (TVLCCONFIG *vlc);
int vlc_isSeekable (TVLCCONFIG *vlc);
int vlc_getChapterCount (TVLCCONFIG *vlc);
libvlc_instance_t *vlc_init (TVLCCONFIG *vlc, int argc, char **argv);
void vlc_releaseLib (TVLCCONFIG *vlc);

char *vlc_EventTypeToName (int event);

libvlc_track_description_t *vlc_getSubtitleDescriptions (TVLCCONFIG *vlc);
int vlc_getSubtitleCount (TVLCCONFIG *vlc);
int vlc_setSubtitle (TVLCCONFIG *vlc, int idx);
int vlc_getSubtitle (TVLCCONFIG *vlc);
void vlc_previousChapter (TVLCCONFIG *vlc);
void vlc_nextChapter (TVLCCONFIG *vlc);
int vlc_willPlay (TVLCCONFIG *vlc);

libvlc_event_manager_t *vlc_getMediaPlayerEventManager (TVLCCONFIG *vlc);
libvlc_event_manager_t *vlc_getMediaEventManager (TVLCCONFIG *vlc);
void vlc_eventPlayerAttach (TVLCCONFIG *vlc, libvlc_event_type_t eventType, libvlc_callback_t callBack, void *udata);
void vlc_eventPlayerDetach (TVLCCONFIG *vlc, libvlc_event_type_t eventType, libvlc_callback_t callBack, void *udata);
void vlc_eventMediaAttach (TVLCCONFIG *vlc, libvlc_event_type_t eventType, libvlc_callback_t callBack, void *udata);
void vlc_eventMediaDetach (TVLCCONFIG *vlc, libvlc_event_type_t eventType, libvlc_callback_t callBack, void *udata);
char *vlc_getMeta (TVLCCONFIG *vlc, libvlc_meta_t e_meta);
int vlc_isParsed (TVLCCONFIG *vlc);
int vlc_getVideoSize (TVLCCONFIG *vlc, int *w, int *h);


char *vlc_getAspectRatio (TVLCCONFIG *vlc);
void vlc_setAspectRatio (TVLCCONFIG *vlc, const char *aspect);

void vlc_setTitle (TVLCCONFIG *vlc, int title);
int vlc_getTitle (TVLCCONFIG *vlc);
int vlc_getTitleChapterCount (TVLCCONFIG *vlc, int title);
int vlc_getTitleCount (TVLCCONFIG *vlc);

libvlc_track_description_t *vlc_getTitleDescriptions (TVLCCONFIG *vlc);
char *vlc_getTitleDescription (TVLCCONFIG *vlc, int titleId);
char *vlc_getChapterDescription (TVLCCONFIG *vlc, int chapterId, int titleId);
libvlc_track_description_t *vlc_getChapterDescriptions (TVLCCONFIG *vlc, int title);

void vlc_setVideoCallbacks (TVLCCONFIG *vlc,
    void *(*lock) (void *data, void **plane),
    void (*unlock) (void *data, void *picture, void *const *plane),
    void (*display) (void *data, void *picture),
    void *data);
    
void vlc_setVideoFormat (TVLCCONFIG *vlc, const char *chroma,
						 unsigned int width, unsigned int height,
						 unsigned int pitch);

void vlc_setMarqueeInt (TVLCCONFIG *vlc, unsigned int option, int val);
void vlc_setMarqueeStr (TVLCCONFIG *vlc, unsigned int option, char *str);

void vlc_addOption (TVLCCONFIG *vlc, const char *ppsz_options);

void vlc_mediaParse (TVLCCONFIG *vlc);
void vlc_mediaParseAsync (TVLCCONFIG *vlc);

#endif

