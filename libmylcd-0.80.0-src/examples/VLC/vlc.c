            
// vlc.c. libvlc 1.1.0 wrapper
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


#include "common.h"
/*
#include <windows.h>
#include "mylcd.h"

#include "ui.h"
#include "tags.h"
#include "playlistc.h"
#include "vlcstream.h"
#include "vlc.h"
*/

const unsigned char *vlc_getVersion ()
{
	return (unsigned char*)libvlc_get_version();
}

const unsigned char *vlc_getCompiler ()
{
	return (unsigned char*)libvlc_get_compiler();
}

libvlc_instance_t *vlc_init (TVLCCONFIG *vlc, int argc, char **argv)
{
	libvlc_instance_t *hLib = libvlc_new(argc, (const char **)argv);
	if (hLib)
		libvlc_set_user_agent(hLib, PLAYER_NAME, PLAYER_VERSION);
	return hLib;
}	

int vlc_getMute (TVLCCONFIG *vlc)
{
	return libvlc_audio_get_mute(vlc->mp);
}

void vlc_setMute (TVLCCONFIG *vlc, const int status)
{
	if (vlc->mp)
		libvlc_audio_set_mute(vlc->mp, status);
}

int vlc_getVolume (TVLCCONFIG *vlc)
{
	if (vlc->mp)
		return libvlc_audio_get_volume(vlc->mp);
	else
		return 0;
}

void vlc_setVolume (TVLCCONFIG *vlc, int volume)
{
	if (vlc->mp)
		libvlc_audio_set_volume(vlc->mp, volume);
}

void vlc_pause (TVLCCONFIG *vlc)
{
	if (vlc->mp)
		libvlc_media_player_pause(vlc->mp);
}

void vlc_play (TVLCCONFIG *vlc)
{
	if (vlc->mp)
		libvlc_media_player_play(vlc->mp);
}

void vlc_stop (TVLCCONFIG *vlc)
{
	if (vlc->mp)
		libvlc_media_player_stop(vlc->mp);
}

float vlc_getPosition (TVLCCONFIG *vlc)
{
	if (vlc->mp)
		return libvlc_media_player_get_position(vlc->mp);
	else
		return 0.0f;
}

void vlc_setPosition (TVLCCONFIG *vlc, float position)
{
	if (vlc->mp)
		libvlc_media_player_set_position(vlc->mp, position);
}

int vlc_getChapterCount (TVLCCONFIG *vlc)
{
	if (vlc->mp)
		return libvlc_media_player_get_chapter_count(vlc->mp);
	else
		return -1;
}

int vlc_getChapter (TVLCCONFIG *vlc)
{
	if (vlc->mp)
		return libvlc_media_player_get_chapter(vlc->mp);
	else
		return -1;
}

void vlc_setChapter (TVLCCONFIG *vlc, int chapter)
{
	libvlc_media_player_set_chapter(vlc->mp, chapter);
}

libvlc_time_t vlc_getLength (TVLCCONFIG *vlc)
{
	if (vlc->mp)
		return libvlc_media_player_get_length(vlc->mp);
	else
		return 0;
}

int vlc_getState (TVLCCONFIG *vlc)
{
	return libvlc_media_player_get_state(vlc->mp);
}

libvlc_media_t *vlc_new_mrl (TVLCCONFIG *vlc, const char *mediaPath)
{
	return libvlc_media_new_location(vlc->hLib, mediaPath);
}

libvlc_media_t *vlc_new_path (TVLCCONFIG *vlc, const char *mediaPath)
{
	return libvlc_media_new_path(vlc->hLib, mediaPath);
}

libvlc_media_t *vlc_new_node (TVLCCONFIG *vlc, const char *mediaPath)
{
	return libvlc_media_new_as_node(vlc->hLib, mediaPath);
}

libvlc_media_player_t *vlc_newFromMedia (TVLCCONFIG *vlc)
{
	if (vlc->m)
		return libvlc_media_player_new_from_media(vlc->m);
	else
		return NULL;
}

void vlc_mediaRelease (TVLCCONFIG *vlc)
{
	if (vlc->m){
		libvlc_media_release(vlc->m);
		vlc->m = NULL;
	}
}

void vlc_release (TVLCCONFIG *vlc)
{
	if (vlc->mp){
		libvlc_media_player_release(vlc->mp);
		vlc->mp = NULL;
	}
	vlc_mediaRelease(vlc);
}

void vlc_releaseLib (TVLCCONFIG *vlc)
{
	if (vlc->hLib){
		libvlc_release(vlc->hLib);
		vlc->hLib = NULL;
	}
}
int vlc_isSeekable (TVLCCONFIG *vlc)
{
	return libvlc_media_player_is_seekable(vlc->mp);
}

int vlc_canPause (TVLCCONFIG *vlc)
{
	return libvlc_media_player_can_pause(vlc->mp);
}

char *vlc_EventTypeToName (int event)
{
	return (char*)libvlc_event_type_name(event);
}

libvlc_event_manager_t *vlc_getMediaPlayerEventManager (TVLCCONFIG *vlc)
{
	if (vlc->mp)
		return vlc->emp = libvlc_media_player_event_manager(vlc->mp);
	else
		return NULL;
}

libvlc_event_manager_t *vlc_getMediaEventManager (TVLCCONFIG *vlc)
{
	if (vlc->m)
		return vlc->em = libvlc_media_event_manager(vlc->m);
	else
		return NULL;
}

void vlc_eventMediaAttach (TVLCCONFIG *vlc, libvlc_event_type_t eventType, libvlc_callback_t callBack, void *udata)
{
	if (vlc->em)
		libvlc_event_attach(vlc->em, eventType, callBack, udata);
}

void vlc_eventMediaDetach (TVLCCONFIG *vlc, libvlc_event_type_t eventType, libvlc_callback_t callBack, void *udata)
{
	if (vlc->em)
		libvlc_event_detach(vlc->em, eventType, callBack, udata);
}

void vlc_eventPlayerAttach (TVLCCONFIG *vlc, libvlc_event_type_t eventType, libvlc_callback_t callBack, void *udata)
{
	if (vlc->emp)
		libvlc_event_attach(vlc->emp, eventType, callBack, udata);
}

void vlc_eventPlayerDetach (TVLCCONFIG *vlc, libvlc_event_type_t eventType, libvlc_callback_t callBack, void *udata)
{
	if (vlc->emp)
		libvlc_event_detach(vlc->emp, eventType, callBack, udata);
}

char *vlc_getMeta (TVLCCONFIG *vlc, libvlc_meta_t e_meta)
{
	if (vlc->m)
		return libvlc_media_get_meta(vlc->m, e_meta);
	else
		return "invalid";
}

void vlc_mediaParse (TVLCCONFIG *vlc)
{
	if (vlc->m)
		libvlc_media_parse(vlc->m);
}

void vlc_mediaParseAsync (TVLCCONFIG *vlc)
{
	if (vlc->m)
		libvlc_media_parse_async(vlc->m);
}

int vlc_getVideoSize (TVLCCONFIG *vlc, int *w, int *h)
{
	if (vlc->mp)
		return libvlc_video_get_size(vlc->mp, 0, (unsigned int*)w, (unsigned int*)h);
	else
		return -1;
}

int vlc_getSubtitleCount (TVLCCONFIG *vlc)
{
	if (vlc->mp)
		return libvlc_video_get_spu_count(vlc->mp);
	else
		return -1;
}

libvlc_track_description_t *vlc_getSubtitleDescriptions (TVLCCONFIG *vlc)
{
	if (vlc->mp)
		return libvlc_video_get_spu_description(vlc->mp);
	else
		return NULL;
}

int vlc_setSubtitle (TVLCCONFIG *vlc, int idx)
{
	if (vlc->mp)
		return libvlc_video_set_spu(vlc->mp, idx);
	else
		return -1;
}

int vlc_getSubtitle (TVLCCONFIG *vlc)
{
	if (vlc->mp)
		return libvlc_video_get_spu(vlc->mp);
	else
		return -1;
}

int vlc_willPlay (TVLCCONFIG *vlc)
{
	if (vlc->mp)
		return libvlc_media_player_will_play(vlc->mp);
	else
		return -1;
}

void vlc_nextChapter (TVLCCONFIG *vlc)
{
	if (vlc->mp)
		libvlc_media_player_next_chapter(vlc->mp);
} 

void vlc_previousChapter (TVLCCONFIG *vlc)
{
	if (vlc->mp)
		libvlc_media_player_previous_chapter(vlc->mp);
} 

void vlc_setAspectRatio (TVLCCONFIG *vlc, const char *aspect)
{
	if (vlc->mp)
		libvlc_video_set_aspect_ratio(vlc->mp, aspect);
}

char *vlc_getAspectRatio (TVLCCONFIG *vlc)
{
	if (vlc->mp)
		return libvlc_video_get_aspect_ratio(vlc->mp);
	else
		return NULL;
}

void vlc_setTitle (TVLCCONFIG *vlc, int title)
{
	if (vlc->mp)
		libvlc_media_player_set_title(vlc->mp, title);
}

int vlc_getTitle (TVLCCONFIG *vlc)
{
	if (vlc->mp)
		return libvlc_media_player_get_title(vlc->mp);
	else
		return 0;
}

int vlc_getTitleChapterCount (TVLCCONFIG *vlc, int title)
{
	if (vlc->mp)
		return libvlc_media_player_get_chapter_count_for_title(vlc->mp, title);
	else
		return 0;
}

int vlc_getTitleCount (TVLCCONFIG *vlc)
{
	if (vlc->mp)
		return libvlc_media_player_get_title_count(vlc->mp);
	else
		return 0;
}

libvlc_track_description_t *vlc_getTitleDescriptions (TVLCCONFIG *vlc)
{
	if (vlc->mp)
		return libvlc_video_get_title_description(vlc->mp);
	else
		return NULL;
}

char *vlc_getTitleDescription (TVLCCONFIG *vlc, int titleId)
{
	libvlc_track_description_t *d = vlc_getTitleDescriptions(vlc);
	if (d){
		for (int i = 0; d; i++){
			if (i == titleId)
				return my_strdup(d->psz_name);
			d = d->p_next;
		}
		libvlc_track_description_release(d);
	}
	return NULL;
}

libvlc_track_description_t *vlc_getChapterDescriptions (TVLCCONFIG *vlc, int title)
{
	if (vlc->mp)
		return libvlc_video_get_chapter_description(vlc->mp, title);
	else
		return NULL;
}

char *vlc_getChapterDescription (TVLCCONFIG *vlc, int chapterId, int titleId)
{
	libvlc_track_description_t *d = vlc_getChapterDescriptions(vlc, titleId);
	if (d){
		for (int i = 0; d; i++){
			if (i == chapterId)
				return my_strdup(d->psz_name);
			d = d->p_next;
		}
		libvlc_track_description_release(d);
	}
	return NULL;
}

void vlc_setVideoCallbacks (TVLCCONFIG *vlc,
    void *(*lock) (void *data, void **plane),
    void (*unlock) (void *data, void *picture, void *const *plane),
    void (*display) (void *data, void *picture),
    void *data)
{

	if (vlc->mp)
		libvlc_video_set_callbacks(vlc->mp, lock, unlock, display, data);
}

void vlc_setVideoFormat (TVLCCONFIG *vlc, const char *chroma,
						unsigned int width, unsigned int height,
						unsigned int pitch)
{
	if (vlc->mp)
		libvlc_video_set_format(vlc->mp, chroma, width, height, pitch);
}

void vlc_addOption (TVLCCONFIG *vlc, const char *ppsz_options)
{
	if (vlc->m)
		libvlc_media_add_option(vlc->m, ppsz_options);
}

void vlc_setMarqueeInt (TVLCCONFIG *vlc, unsigned int option, int val)
{
	if (vlc->mp)
		libvlc_video_set_marquee_int(vlc->mp, option, val);
}

void vlc_setMarqueeStr (TVLCCONFIG *vlc, unsigned int option, char *str)
{
	if (vlc->mp)
		libvlc_video_set_marquee_string(vlc->mp, option, str);
}

int vlc_isParsed (TVLCCONFIG *vlc)
{
	if (vlc->m)
		return libvlc_media_is_parsed(vlc->m);
	else
		return 0;
}
