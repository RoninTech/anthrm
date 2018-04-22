
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



#ifndef _COMMON_H_
#define _COMMON_H_

#include <shlwapi.h>
#include <inttypes.h>		// #define PRId64 I64d
#include <windows.h>
#include <time.h>
#include "mylcd.h"

#define	G19DISPLAY			0	// 0:usbd480/480x272, 1:G19/320x240
#define RELEASEBUILD		1	// disables console input when set
#define MOUSEHOOKCAP		1	// enable mouse hooking capability. keys: shift+control+A or Q, L or P (creates a new thread and hidden window)
#define ASYNCHRONOUSREFRESH 1
#define DRAWTOUCHRECTS		0	// draw blue rectangles around touch contact areas
#define ADDLINKSHORTCUTS	1	// add my personal folder shortcuts to the file browser menu
#define MYDSHOWDEVICESETUP	0	// my personal DShow setup


#define MYSHORTCUTS \
	{L"r:\\music", L"Music"},\
	{L"p:\\movies", L"Movies"},\
	{L"r:\\download", L"Downloads"}\



#ifndef	_ANSIDECL_H
#undef VA_OPEN
#undef VA_CLOSE
#undef VA_FIXEDARG

#define VA_OPEN(AP, VAR)	{ va_list AP; va_start(AP, VAR); { struct Qdmy
#define VA_CLOSE(AP)		} va_end(AP); }
#define VA_FIXEDARG(AP, T, N)	struct Qdmy
#endif


#if 0

//MYLCD_EXPORT void * my_Memcpy (void *s1, const void *s2, size_t n)
//	__attribute__((nonnull(1, 2)));

MYLCD_EXPORT void * my_Malloc (size_t size, const char *func)
__attribute__((malloc));

MYLCD_EXPORT void * my_Calloc (size_t nelem, size_t elsize, const char *func)
__attribute__((malloc));

MYLCD_EXPORT void * my_Realloc (void *ptr, size_t size, const char *func)
__attribute__((malloc));

MYLCD_EXPORT char * my_Strdup (const char *str, const char *func)
__attribute__((nonnull(1)));

MYLCD_EXPORT wchar_t * my_Wcsdup (const wchar_t *str, const char *func)
__attribute__((nonnull(1)));

MYLCD_EXPORT void * my_Free (void *ptr, const char *func);

#define funcname __func__

#undef malloc
#undef calloc
#undef realloc
#undef free
#undef strdup
#undef wcsdup
//#undef memcpy

//#define my_memcpy(s1,s2,n)	my_Memcpy(s1, s2, n)
#define my_memcpy			mmx_memcpy
#define my_malloc(n)		my_Malloc(n, funcname)
#define my_calloc(n,e)		my_Calloc(n, e, funcname)
#define my_realloc(p,n)		my_Realloc(p, n, funcname)
#define my_free(p)			my_Free(p, funcname)
#define my_strdup(p)		my_Strdup(p, funcname)
#define my_wcsdup(p)		my_Wcsdup(p, funcname)


#else

//#define my_memcpy(s1, s2, n) memcpy(s1, s2, n)
#define my_memcpy			mmx_memcpy
#define my_malloc(n)		malloc(n)
#define my_calloc(n, e)		calloc(n, e)
#define my_realloc(p, n)	realloc(p, n)
#define my_free(p)			free(p)
#define my_strdup(p)		strdup(p)
#define my_wcsdup(p)		wcsdup(p)

#endif


#if 0	/* use the libmylcd mmx2 memcpy routine */

//#undef memcpy
//#undef my_memcpy
//MYLCD_EXPORT void * my_Memcpy (void *s1, const void *s2, size_t n)
//	__attribute__((nonnull(1, 2)));
//#define my_memcpy(s1, s2, n) my_Memcpy(s1, s2, n)
#endif


#include "ui.h"
#include "tags.h"
#include "playlistc.h"
#include "fileal.h"
#include "playlistsort.h"
#include "editbox.h"
#include "cmdparser.h"
#include "playlistManager.h"
#include "m3u.h"
#include "vlcstream.h"
#include "playlistPlm.h"
#include "ploverlay.h"
#include "drawvolume.h"
#include "m3u.h"
#include "winm.h"
#include "imagec.h"
#include "zoom.h"
#include "button.h"
#include "browser.h"
#include "imgovr.h"
#include "chapter.h"
#include "exit.h"
#include "sort.h"
#include "vlc.h"
#include "cfg.h"
#include "sub.h"
#include "meta.h"
#include "playlist.h"
#include "playlist2.h"
#include "clock.h"
#include "morph.h"
#include "lorenz.h"
#include "particles.h"
#include "rollc.h"
#include "tetris.h"
#include "hook/hook.h"



void fadeArea (TFRAME *frame, const int x1, const int y1, const int x2, const int y2);


void copyImage (TFRAME *src, TFRAME *des, const int sw, const int sh, const int dx, const int dy, const int dw, const int dh);

wchar_t *buildSkinD (TVLCPLAYER *vp, wchar_t *buffer, wchar_t *path);

char *stristr (const char *String, const char *Pattern);
wchar_t *wcsistr (const wchar_t *String, const wchar_t *Pattern);
int UTF8ToUTF16 (const char *in, const size_t ilen, wchar_t *out, size_t olen);
int UTF16ToUTF8 (const wchar_t *in, const size_t ilen, char *out, size_t olen);
//char *UTF16ToUTF8Alloc (const wchar_t *in, const size_t ilen);
//wchar_t *UTF8ToUTF16Alloc (const char *in, const size_t ilen);
char *convertto8 (const wchar_t *wide);
wchar_t *converttow (const char *utf8);
unsigned int getHash (const char *path);		// ascii/utf8
unsigned int getHashW (const wchar_t *path);	// utf16

int timeToString (const libvlc_time_t t, char *buffer, const size_t bufferLen);
libvlc_time_t stringToTime (char *sztime, size_t len);
libvlc_time_t stringToTimeW (wchar_t *sztime, size_t len);

wchar_t *removeLeadingSpacesW (wchar_t *var);
wchar_t *removeTrailingSpacesW (wchar_t *var);
char *removeLeadingSpaces (char *var);
char *removeTrailingSpaces (char *var);

void *getPagePtr (TVLCPLAYER *vp, int page_id);
void setPagePtr (TVLCPLAYER *vp, int page_id, void *ptr);

void printSingleLineShadow (TFRAME *frame, const int font, const int x, const int y, const int colour, const char *str);
void printSingleLineEx (TFRAME *frame, const void *text, int font, int x, int y, int foreColour, TLPOINTEX *pos);
void printSingleLine (TVLCPLAYER *vp, TFRAME *frame, const wchar_t *text, int font, int x, int y, int foreColour);
void imageBestFit (const int bg_w, const int bg_h, int fg_w, int fg_h, int *w, int *h);
void fastFrameCopy (const TFRAME *src, TFRAME *des, int dx, int dy);
void disableFPS (TVLCPLAYER *vp);
void enableFPS (TVLCPLAYER *vp);

void outlineTextEnable (THWD *hw, const int colour);
void outlineTextDisable (THWD *hw);
void shadowTextEnable (THWD *hw, const int colour, const unsigned char trans);
void shadowTextDisable (THWD *hw);

void drawInt (TFRAME *frame, const int x, const int y, const int var, const int colour);

void drawImage (TFRAME *src, TFRAME *des, const int x, const int y, const int x2, const int y2);
void drawImageScaled (TFRAME *src, TFRAME *des, const int x, const int y, const float s);

int zoomRender (TZOOM *zoom, TFRAME *frame, int x, int y);
TZOOM *zoomCreate (TFRAME *image, float scale, float scaleRate, int direction);
void zoomDelete (TZOOM *zoom);

void blurImage (TVLCPLAYER *vp, TFRAME *src, const int radius);
void blurImageAreaWithFade (TFRAME *frame, const int x1, const int y1, const int x2, const int y2, const int iterations);
void huhtanenBlur (TFRAME *des, const int c1, const int r1, const int c2, const int r2, const int iterations);

void *decodeURI (const char *arturl, const int len);
int encodeURI (const char *in, char *out, const size_t len);

int doesFileExistW (const wchar_t *path);
int doesFileExistUtf8 (const char *utf8path);
int isDirectoryW (wchar_t *path);
int isDirectory (char *path);
int isPlaylistW (wchar_t *path);
int isPlaylist (char *path);

unsigned int generateHash (const void *data, const size_t dlen);
int vaswprintf (wchar_t **result, const wchar_t *format, va_list *args);
void dbwprintf (TVLCPLAYER *vp, const wchar_t *str, ...);
void dbprintf (TVLCPLAYER *vp, const char *str, ...);


#endif


#define MIN_LEN 		64 		 /* 64-byte blocks */
#define MMX1_MIN_LEN	64

#define _mmx_memcpy(to,from,len) \
{\
  size_t i;\
  if (len >= MMX1_MIN_LEN)\
  {\
    i = len >> 6;\
    len &= 63;\
    for(; i>0; i--)\
    {\
      __asm__ __volatile__ (\
      "movq (%0), %%mm0\n"\
      "movq 8(%0), %%mm1\n"\
      "movq 16(%0), %%mm2\n"\
      "movq 24(%0), %%mm3\n"\
      "movq 32(%0), %%mm4\n"\
      "movq 40(%0), %%mm5\n"\
      "movq 48(%0), %%mm6\n"\
      "movq 56(%0), %%mm7\n"\
      "movq %%mm0, (%1)\n"\
      "movq %%mm1, 8(%1)\n"\
      "movq %%mm2, 16(%1)\n"\
      "movq %%mm3, 24(%1)\n"\
      "movq %%mm4, 32(%1)\n"\
      "movq %%mm5, 40(%1)\n"\
      "movq %%mm6, 48(%1)\n"\
      "movq %%mm7, 56(%1)\n"\
      :: "r" (from), "r" (to) : "memory");\
      from += 64;\
      to += 64;\
    }\
	__asm__ __volatile__ ("sfence":::"memory");\
    __asm__ __volatile__ ("emms":::"memory");\
  }\
  if (len) memcpy(to, from, len);\
}

#define _mmx2_memcpy(to,from,len) \
{\
	size_t i;\
	__asm__ __volatile__ (\
    "   prefetch (%0)\n"\
    "   prefetch 64(%0)\n"\
    "   prefetch 128(%0)\n"\
    "   prefetch 192(%0)\n"\
    : : "r" (from) );\
	if (len >= MIN_LEN){\
        i = len >> 6;\
    len &= 63;\
    for(; i>0; i--){\
      __asm__ __volatile__ (\
      "prefetch 224(%0)\n"\
      "movq (%0), %%mm0\n"\
      "movq 8(%0), %%mm1\n"\
      "movq 16(%0), %%mm2\n"\
      "movq 24(%0), %%mm3\n"\
      "movq 32(%0), %%mm4\n"\
      "movq 40(%0), %%mm5\n"\
      "movq 48(%0), %%mm6\n"\
      "movq 56(%0), %%mm7\n"\
	  "prefetch 256(%0)\n"\
      "movntq %%mm0, (%1)\n"\
      "movntq %%mm1, 8(%1)\n"\
      "movntq %%mm2, 16(%1)\n"\
      "movntq %%mm3, 24(%1)\n"\
      "movntq %%mm4, 32(%1)\n"\
      "movntq %%mm5, 40(%1)\n"\
      "movntq %%mm6, 48(%1)\n"\
      "movntq %%mm7, 56(%1)\n"\
      :: "r" (from), "r" (to) : "memory");\
      from += 64;\
      to += 64;\
    }\
    __asm__ __volatile__ ("sfence":::"memory");\
    __asm__ __volatile__ ("emms":::"memory");\
  }\
  if (len) memcpy(to, from, len);\
}


#define mmx_memcpy(_to,_from,_len) \
{\
  	unsigned char * restrict to = (unsigned char * restrict)(_to);\
  	unsigned char * restrict from = (unsigned char * restrict)(_from);\
	size_t len = (size_t)(_len);\
 	if (len < 128){\
		if (len&3){\
			for (int i = 0; i < len; i++)\
				to[i] = from[i];\
		}else{\
			unsigned int *restrict src = (unsigned int *restrict)from;\
			unsigned int *restrict des = (unsigned int *restrict)to;\
			len >>= 2;\
			for (int i = 0; i < len; i++)\
				des[i] = src[i];\
		}\
  	}else if (len < 293000){\
		_mmx_memcpy(to,from,len);\
	}else{\
		_mmx2_memcpy(to,from,len);\
	}\
}


