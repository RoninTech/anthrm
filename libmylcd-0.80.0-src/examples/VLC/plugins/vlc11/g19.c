/*****************************************************************************
 * g19.c: libmylcd usbd480 video driver for vlc 1.1.x
 *****************************************************************************
 * Copyright (C) 2008 the VideoLAN team
 * $Id: 
 *
 * Authors: Sam Hocevar <sam@zoy.org>, Michael McElligott
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/

/*****************************************************************************
 * Preamble
 *****************************************************************************/

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <assert.h>

#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_vout_display.h>
#include <vlc_picture_pool.h>

#include "mylcd.h"

#include <windows.h>
//#include <conio.h>
//#include <fcntl.h>


#if 1
const char *device[] = {"G19", "DDRAW"};
#define DWIDTH		320
#define DHEIGHT		240
#else
const char *device[] = {"USBD480:LIBUSBHID", "USBD480:LIBUSB", "USBD480:DLL", "DDRAW"};
#define DWIDTH 480
#define DHEIGHT 272
#endif

#define DBPP		LFRM_BPP_32
#define DCHROMA		"RV32";

static THWD *hw = NULL;
static TFRAME *frame = NULL;
static TFRAME *frameWrk = NULL;
static TRECT display;
static int displayEnabled = 0;


static int firstLineOffset = 0;
static int firstPixelOffset = 0;

static int libmylcd_init ();
static void libmylcd_shutdown ();
static int initLibrary ();

/*****************************************************************************
 * Module descriptor
 *****************************************************************************/

static int  Open (vlc_object_t *);
static void Close(vlc_object_t *);

vlc_module_begin()
    set_description(N_("G19 video output"))
    set_shortname(N_("G19"))

    set_category(CAT_VIDEO)
    set_subcategory(SUBCAT_VIDEO_VOUT)
    set_capability("vout display", 0)
    
   // add_string("vmem-lock", "0", NULL, T_LOCK, LT_LOCK, true)
      //  change_volatile()

    set_callbacks(Open, Close)
vlc_module_end()

/*****************************************************************************
 * Local prototypes
 *****************************************************************************/
struct picture_sys_t {
    int unused;
};

struct vout_display_sys_t {
    picture_pool_t *pool;
};

static picture_pool_t *Pool  (vout_display_t *, unsigned);
static void           Display(vout_display_t *, picture_t *);
static int            Control(vout_display_t *, int, va_list);
static void           Manage (vout_display_t *);

static int            Lock(picture_t *);
static void           Unlock(picture_t *);




#if 0
void initConsole ()
{
	if (AllocConsole()){
		int hCrt = _open_osfhandle((long)GetStdHandle(STD_OUTPUT_HANDLE),_O_TEXT);
		if (hCrt){
			FILE *hf = fdopen(hCrt, "wr");
			if (hf){
				*stdout = *hf;
				setvbuf(stdout, NULL, _IONBF, 0);
			}else{
				FreeConsole();
			}
		}else{
			FreeConsole();
		}
	}
}
#endif


static int initLibrary ()
{
    if (!(hw=lOpen(NULL, NULL))){
    	return 0;
    }
   	return 1;
}

static int libmylcd_init ()
{
	if (!initLibrary())
		return 0;

	display.left = 0;
	display.top = 0;
	display.right = DWIDTH-1;
	display.btm = DHEIGHT-1;

	int ret = lSelectDevice(hw, device[0], "NULL", DWIDTH, DHEIGHT, DBPP, 0, &display);
	if (!ret)
		ret = lSelectDevice(hw, device[1], "NULL", DWIDTH, DHEIGHT, DBPP, 0, &display);
	
	frame = lNewFrame(hw, DWIDTH, DHEIGHT, DBPP);
	frameWrk = lNewFrame(hw, DWIDTH, DHEIGHT, DBPP);
	displayEnabled = (frame && frameWrk);
    return displayEnabled;
}

static void libmylcd_shutdown ()
{
	displayEnabled = 0;
	lDeleteFrame(frame);
	lDeleteFrame(frameWrk);
	lClose(hw);

}


/*****************************************************************************
 * Open: allocates video thread
 *****************************************************************************
 * This function allocates and initializes a vout method.
 *****************************************************************************/


static int Open (vlc_object_t *object)
{
    vout_display_t *vd = (vout_display_t *)object;

	int desHeight = DHEIGHT;
	int desWidth = DWIDTH;
	int w, h, i_pitch;

	video_format_t *source = &vd->source;

	//int realWidth = (source->i_width * source->i_height * source->i_sar_num)/ (float)(source->i_height * source->i_sar_den);
	const int bg_h = DHEIGHT;
	const int bg_w = DWIDTH;
	const int fg_w = source->i_width;
	const int fg_h = source->i_height;
	const int fg_sar_num = source->i_sar_num;
	const int fg_sar_den = source->i_sar_den;
	const int bg_sar_den = 1;
	const int bg_sar_num = 1;

	w = bg_w;
	h = ( bg_w * fg_h * fg_sar_den * bg_sar_num ) / (float)( fg_w * fg_sar_num * bg_sar_den );
	if (h > DHEIGHT){
		w = ( bg_h * fg_w * fg_sar_num * bg_sar_den ) / (float)( fg_h * fg_sar_den * bg_sar_num );
		h = bg_h;
	}

	if (w > desWidth)
		w = desWidth;
	if (h > desHeight)
		h = desHeight;

	//if (!displayEnabled){
		if (!libmylcd_init())
			return VLC_ENOMOD;
	//}

	lResizeFrame(frame, w, DHEIGHT, 0);
	lClearFrame(frame);
	lClearFrame(frameWrk);

	firstPixelOffset = (DWIDTH-w)/2;
	firstLineOffset = (DHEIGHT-h)/2;	
    i_pitch = w*4;

    /* */
    char *chroma_format = (char*)DCHROMA;
    vlc_fourcc_t chroma = (vlc_fourcc_t)vlc_fourcc_GetCodecFromString(VIDEO_ES, (void*)chroma_format);
    if (!chroma) {
        msg_Err(vd, "chroma should be 4 characters long");
        return VLC_EGENERIC;
    }

    /* */
    video_format_t fmt = vd->fmt;
    fmt.i_chroma = chroma;
    fmt.i_width  = w;
    fmt.i_height = h;

    /* Define the bitmasks */
    switch (chroma)
    {
    case VLC_CODEC_RGB15:
        fmt.i_rmask = 0x001f;
        fmt.i_gmask = 0x03e0;
        fmt.i_bmask = 0x7c00;
        break;
    case VLC_CODEC_RGB16:
        fmt.i_rmask = 0x001f;
        fmt.i_gmask = 0x07e0;
        fmt.i_bmask = 0xf800;
        break;
    case VLC_CODEC_RGB24:
        fmt.i_rmask = 0xff0000;
        fmt.i_gmask = 0x00ff00;
        fmt.i_bmask = 0x0000ff;
        break;
    case VLC_CODEC_RGB32:
        fmt.i_rmask = 0xff0000;
        fmt.i_gmask = 0x00ff00;
        fmt.i_bmask = 0x0000ff;
        break;
    default:
        fmt.i_rmask = 0;
        fmt.i_gmask = 0;
        fmt.i_bmask = 0;
        break;
    }

	picture_sys_t cfg;
	cfg.unused = 0;
	
    /* */
    vout_display_sys_t *sys;
    vd->sys = sys = calloc(1, sizeof(*sys));
    if (!sys)
        return VLC_EGENERIC;

    /* */

    picture_resource_t rsc;
    rsc.p_sys = malloc(sizeof(*rsc.p_sys));
    *rsc.p_sys = cfg;
    for (int i = 0; i < PICTURE_PLANE_MAX; i++) {
        /* vmem-lock is responsible for the allocation */
        rsc.p[i].p_pixels = NULL;
        rsc.p[i].i_lines  = fmt.i_height;
        rsc.p[i].i_pitch  = i_pitch;
    }
    picture_t *picture = picture_NewFromResource(&fmt, &rsc);
    if (!picture) {
        free(rsc.p_sys);
        free(sys);
        return VLC_EGENERIC;
    }

    /* */
    picture_pool_configuration_t pool;
    memset(&pool, 0, sizeof(pool));
    pool.picture_count = 1;
    pool.picture       = &picture;
    pool.lock          = Lock;
    pool.unlock        = Unlock;
    sys->pool = picture_pool_NewExtended(&pool);
    if (!sys->pool) {
        picture_Release(picture);
        free(sys);
        return VLC_SUCCESS;
    }

    /* */
    vout_display_info_t info = vd->info;
    info.has_hide_mouse = true;

    /* */
    vd->fmt     = fmt;
    vd->info    = info;
    vd->pool    = Pool;
    vd->prepare = NULL;
    vd->display = Display;
    vd->control = Control;
    vd->manage  = Manage;

    /* */
    vout_display_SendEventFullscreen(vd, false);
    vout_display_SendEventDisplaySize(vd, w, h, false);
    return VLC_SUCCESS;
}

static void Close (vlc_object_t *object)
{

#if 1
    libmylcd_shutdown();
#else
    lClearFrame(frame);
    lClearFrame(frameWrk);
    lRefreshAsync(frameWrk, 1); // swap back buffer to front
    lClearFrame(frameWrk);
    lRefresh(frameWrk);
#endif

    vout_display_t *vd = (vout_display_t *)object;
    vout_display_sys_t *sys = vd->sys;
    picture_pool_Delete(sys->pool);
    free(sys);
}

/* */
static picture_pool_t *Pool(vout_display_t *vd, unsigned count)
{
    VLC_UNUSED(count);
    return vd->sys->pool;
}

static void Display(vout_display_t *vd, picture_t *picture)
{
    VLC_UNUSED(vd);
    assert(!picture_IsReferenced(picture));
    picture_Release(picture);
}

static int Control (vout_display_t *vd, int query, va_list args)
{
	//vout_display_sys_t *sys = vd->sys;

    switch (query) {
    case VOUT_DISPLAY_CHANGE_SOURCE_ASPECT:  /* const video_format_t *p_source */
    case VOUT_DISPLAY_CHANGE_DISPLAY_SIZE:   /* const vout_display_cfg_t *p_cfg, int is_forced */
    case VOUT_DISPLAY_CHANGE_DISPLAY_FILLED: /* const vout_display_cfg_t *p_cfg */
    case VOUT_DISPLAY_CHANGE_ZOOM:           /* const vout_display_cfg_t *p_cfg */
    case VOUT_DISPLAY_CHANGE_SOURCE_CROP:  {
#if 0
		vout_display_cfg_t *cfg;
		video_format_t *source;
        //bool is_forced = true;
        if (query == VOUT_DISPLAY_CHANGE_SOURCE_CROP ||
            query == VOUT_DISPLAY_CHANGE_SOURCE_ASPECT) {
            cfg    = vd->cfg;
            source = va_arg(args, const video_format_t *);
       
        } else {
            cfg    = va_arg(args, const vout_display_cfg_t *);
            source = &vd->source;
        }

    	printf("\n%% %i %i %i %i\n",cfg->display.width, cfg->display.height, cfg->display.sar.num, cfg->display.sar.den);
    	
		printf("%% %i %i\n", source->i_width, source->i_height);
		printf("%% %i %i %i %i\n", source->i_width, source->i_height, source->i_sar_num, source->i_sar_den);
		printf("%i\n", (source->i_width * source->i_height * source->i_sar_num)/(source->i_height * source->i_sar_den));
		
		printf("%% %i %i\n", source->i_visible_width, source->i_visible_height);
		printf("%% %i %i\n\n", source->i_x_offset, source->i_y_offset);
		
#else
		(void)vd;
		(void)args;
#endif		
      } 
#if 0
    case VOUT_DISPLAY_CHANGE_FULLSCREEN:
    case VOUT_DISPLAY_CHANGE_DISPLAY_SIZE: {
        const vout_display_cfg_t *cfg = va_arg(args, const vout_display_cfg_t *);
        if (cfg->display.width  != vd->fmt.i_width ||
            cfg->display.height != vd->fmt.i_height)
            return VLC_EGENERIC;
        if (cfg->is_fullscreen)
            return VLC_EGENERIC;
        return VLC_SUCCESS;
    }
    default:
        return VLC_EGENERIC;
#endif
    }
     return VLC_SUCCESS;
}

static void Manage (vout_display_t *vd)
{
    VLC_UNUSED(vd);

}

/* */
static int Lock (picture_t *picture)
{
    for (int i = 0; i < picture->i_planes; i++)
        picture->p[i].p_pixels = lGetPixelAddress(frame,0,firstLineOffset);

    return VLC_SUCCESS;
}

static void Unlock (picture_t *picture)
{
	VLC_UNUSED(picture);
	if (firstPixelOffset < 2){
    	lRefreshAsync(frame, 1);
    }else{
    	for (int i = 0; i < frame->height; i++){
    		vlc_memcpy(lGetPixelAddress(frameWrk, firstPixelOffset, i),
    			lGetPixelAddress(frame, 0, i),
    			frame->pitch);
    	}
    	lRefreshAsync(frameWrk, 1);
    } 
}

#if 0
__declspec(dllexport) BOOL APIENTRY DllMain (HINSTANCE hDLL, DWORD reason, LPVOID reserved);

__declspec(dllexport) BOOL APIENTRY DllMain (HINSTANCE hDLL, DWORD reason, LPVOID reserved)
{
	VLC_UNUSED(hDLL);
	VLC_UNUSED(reserved);
	
	switch (reason) {
		case DLL_PROCESS_ATTACH:
			DisableThreadLibraryCalls(hDLL);
			//initConsole();
			libmylcd_init();
			break;
		case DLL_PROCESS_DETACH:
			Beep(100,100);
			libmylcd_shutdown();
			break;
	}
	return 1;
}
#endif


