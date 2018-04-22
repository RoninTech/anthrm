
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

#ifndef _DSHOWCONFIG_H_
#define _DSHOWCONFIG_H_


/***** DShow setup *****/

#define DSINPUT_CABLE	"1"
#define DSINPUT_ANTENNA	"2"

#define DSCACHING		"0"
#define DSCHROMA		"" //YUY2"
#define DSAUDIO_SAMRATE	"44100"
#define DSAUDIO_BPS		"16"
#define DSAUDIO_CHNS	"0"
#define DSINPUT_TYPE	DSINPUT_ANTENNA

#if MYDSHOWDEVICESETUP
// define to use specified setting
// undefine to use the default VLC setting
#define DSVIDEO_SIZE	"640x480"
#define DSDEVICE_VIDEO	"Pinnacle WDM PCTV Video Capture"
#define DSDEVICE_AUDIO	"Realtek AC97 Audio"

#endif

// TV channel setup
// configured below for UK freeview
#define CHN_BBC1	31
#define CHN_BBC2	27
#define CHN_ITV		24
#define CHN_C4		21
#define CHN_C5		37
#define CHN_END		999
#define CHN_COUNTRY	44

#define CHN_1		CHN_BBC1
#define CHN_2		CHN_BBC2
#define CHN_3		CHN_ITV
#define CHN_4		CHN_C4
#define CHN_5		CHN_C5
#define CHN_TOTAL	5

#if 0
static TTUNERCHANNEL channels[] = {
	{L"BBC1", CHN_1},
	{L"BBC2", CHN_2},
	{L"ITV",  CHN_3},
	{L"Channel 4", CHN_4}, 
	{L"Five", CHN_5}, 
	{L"", CHN_END}
};
#endif

#endif

