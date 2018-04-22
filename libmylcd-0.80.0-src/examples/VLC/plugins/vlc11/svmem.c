/*****************************************************************************
 * svmem.c: shared memory video driver for vlc
 *****************************************************************************
 * Copyright (C) 2008 the VideoLAN team
 * $Id: a63be9b8e24c6a612d5694bca568304231c06a41 $
 *
 * Authors: Sam Hocevar <sam@zoy.org>
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
#include "svmem.h"

/*****************************************************************************
 * Module descriptor
 *****************************************************************************/
#define T_WIDTH  N_("Width")
#define LT_WIDTH N_("Video memory buffer width.")

#define T_HEIGHT  N_("Height")
#define LT_HEIGHT N_("Video memory buffer height.")

#define T_CHROMA  N_("Chroma")
#define LT_CHROMA N_("Output chroma for the memory image as a 4-character " \
                     "string, eg. \"RV32\".")

#define T_PIPE  N_("Shared pipe name")
#define LT_PIPE N_("Shared global memory pipe named. Must be 6 or more characters.")

#define T_EVENT  N_("Updated event name")
#define LT_EVENT N_("Name given to event which is triggered on each update. "\
					"Must be 6 or more characters.")

#define T_LOCK  N_("Shared semaphore lock name")
#define LT_LOCK N_("Shared global memory semaphore lock. Synchronizes data access."\
				   "Must be 6 or more characters.")

static int  Open (vlc_object_t *);
static void Close(vlc_object_t *);

vlc_module_begin()
    set_description(N_("Shared video memory output"))
    set_shortname(N_("Shared video memory"))

    set_category(CAT_VIDEO)
    set_subcategory(SUBCAT_VIDEO_VOUT)
    set_capability("vout display", 0)

    add_integer("svmem-width", 128, NULL, T_WIDTH, LT_WIDTH, false)
    add_integer("svmem-height", 128, NULL, T_HEIGHT, LT_HEIGHT, false)
    add_string("svmem-chroma", "RV24", NULL, T_CHROMA, LT_CHROMA, true)
    add_string("svmem-pipe", VLC_SMEMNAME, NULL, T_PIPE, LT_PIPE, true)
    add_string("svmem-event", VLC_SMEMEVENT, NULL, T_EVENT, LT_EVENT, true)
    add_string("svmem-lock", VLC_SMEMLOCK, NULL, T_LOCK, LT_LOCK, true)
    	
	add_shortcut("svmem")

    set_callbacks(Open, Close)
vlc_module_end()

/*****************************************************************************
 * Local prototypes
 *****************************************************************************/
struct picture_sys_t {
    void *hDataLock;
	void *hUpdateEvent;
	void *hMapFile;
	uint8_t *hMem;
	TSVMEM *svmem;
};

struct vout_display_sys_t {
    picture_pool_t *pool;
    void *p_sys;
};

static picture_pool_t *Pool  (vout_display_t *, unsigned);
static void           Display(vout_display_t *, picture_t *);
static int            Control(vout_display_t *, int, va_list);
static void           Manage (vout_display_t *);

static int            Lock(picture_t *);
static void           Unlock(picture_t *);

/*****************************************************************************
 * Open: allocates video thread
 *****************************************************************************
 * This function allocates and initializes a vout method.
 *****************************************************************************/

static void closeSharedMemory (struct picture_sys_t *p_sys)
{
	if (p_sys->hUpdateEvent != NULL)
		CloseHandle(p_sys->hUpdateEvent);

	if (p_sys->hDataLock)
		CloseHandle(p_sys->hDataLock);
	
	if (p_sys->hMem != NULL)
		UnmapViewOfFile(p_sys->hMem);
	
	if (p_sys->hMapFile != NULL)
		CloseHandle(p_sys->hMapFile);
		
	p_sys->hDataLock = NULL;
	p_sys->hUpdateEvent = NULL;
	p_sys->hMem = NULL;
	p_sys->hMapFile = NULL;
}

static int createSharedMemory (vout_display_t *vd, struct picture_sys_t *p_sys, const char *name, const size_t vsize)
{
	p_sys->hMapFile = (void*)CreateFileMapping(SYSMEMFILE, NULL, PAGE_READWRITE, 0, vsize, name);
	if (p_sys->hMapFile != NULL){
		p_sys->hMem = (uint8_t*)MapViewOfFile(p_sys->hMapFile, FILE_MAP_ALL_ACCESS, 0,0,0);
		if (p_sys->hMem != NULL){
			return VLC_SUCCESS;
		}else{
			CloseHandle(p_sys->hMapFile);
			p_sys->hMapFile = NULL;
			msg_Err(vd, "MapViewOfFile() returned NULL\n");
        	return VLC_EGENERIC;
		}
	}else{
		msg_Err(vd, "CreateFileMapping() returned NULL\n");
        return VLC_EGENERIC;
	}
}
    
static int createInst (vout_display_t *vd, picture_sys_t *p_sys)
{
	char *psz_pipe, *pipename;
    char *psz_event, *eventname;
    char *psz_lock, *lockname;
    TSVMEM *svmem;


	psz_pipe = var_CreateGetString(vd, "svmem-pipe");
    if (psz_pipe == NULL){
        msg_Err(vd, "Pipe name not found. Using \"%s\"",VLC_SMEMNAME);
        pipename = (char*)VLC_SMEMNAME;
	}else if (strlen(psz_pipe) < 6){
		pipename = (char*)VLC_SMEMNAME;
	}else{
		pipename = psz_pipe;
	}

    psz_event = var_CreateGetString(vd, "svmem-event");
    if (psz_event == NULL){
        msg_Err(vd, "Event name not found. Using \"%s\"",VLC_SMEMEVENT);
        eventname = (char*)VLC_SMEMEVENT;
	}else if (strlen(psz_event) < 6){
		eventname = (char*)VLC_SMEMEVENT;
	}else{
		eventname = psz_event;
	}

    psz_lock = var_CreateGetString(vd, "svmem-lock");
    if (psz_lock == NULL){
        msg_Err(vd, "Lock object name not found. Using \"%s\"",VLC_SMEMLOCK);
        lockname = (char*)VLC_SMEMLOCK;
	}else if (strlen(psz_lock) < 6){
		lockname = (char*)VLC_SMEMLOCK;
	}else{
		lockname = psz_lock;
	}
	
	if (VLC_SUCCESS == createSharedMemory(vd, p_sys, pipename, sizeof(TSVMEM)+SVIDEOLENGTH)){
		p_sys->hUpdateEvent = (void*)CreateEvent(NULL, 0, 0, eventname);
		p_sys->hDataLock = CreateSemaphore(NULL, 1, 1, lockname);
		
		if (WaitForSingleObject(p_sys->hDataLock, 2000) == WAIT_OBJECT_0){
			svmem = p_sys->svmem = (TSVMEM*)p_sys->hMem;
			svmem->hdr.ssize = sizeof(TSVMEMHDR);
			svmem->hdr.vsize = SVIDEOLENGTH;
			svmem->hdr.version = 2;
			svmem->hdr.swidth = vd->fmt.i_width;
			svmem->hdr.sheight = vd->fmt.i_height;
			
			svmem->hdr.fsize = SVIDEOLENGTH;
			if (svmem->hdr.rwidth < 32)
				svmem->hdr.width = 640;
			if (!svmem->hdr.rheight < 32)
				svmem->hdr.height = 480;
			if (!svmem->hdr.bpp)
				svmem->hdr.bpp = 24;
			svmem->pixels = 0;

			// tell listeners we're alive.
			svmem->hdr.time = 0;
			svmem->hdr.count = 0;
			ReleaseSemaphore(p_sys->hDataLock, 1, NULL);
			//SetEvent(p_sys->hUpdateEvent);
		}else{
			msg_Err(vd, "Could not create or obtain semaphore lock\n");
        	return VLC_EGENERIC;
		}
	}else{
		msg_Err(vd, "Could not create a shared memory block\n");
        return VLC_EGENERIC;
	}
	
	free(psz_pipe);
    free(psz_event);
    free(psz_lock);
    return VLC_SUCCESS;
}


static int Open (vlc_object_t *object)
{
    vout_display_t *vd = (vout_display_t *)object;
    TSVMEM *svmem;
    char *psz_chroma;
    vlc_fourcc_t i_chroma;
    int i_width, i_height, i_pitch;
    int desWidth, desHeight;
	int w,h;


    /* */
    video_format_t fmt = vd->fmt;

    
	picture_sys_t *cfg = calloc(1, sizeof(picture_sys_t));
	if (cfg == NULL)
		return VLC_ENOMEM;

	createInst(vd, cfg);
	
	i_width = var_CreateGetInteger(vd, "svmem-width");
	i_height = var_CreateGetInteger(vd, "svmem-height");
	psz_chroma = var_CreateGetString(vd, "svmem-chroma");
    
    if (psz_chroma){
        if (strlen( psz_chroma ) < 4){
            msg_Err( vd, "svmem-chroma should be 4 characters long");
            free(psz_chroma);
            return VLC_EGENERIC;
        }
        i_chroma = vlc_fourcc_GetCodecFromString(VIDEO_ES, psz_chroma);
        free(psz_chroma);
    }else{
        msg_Err(vd, "Cannot find chroma information.");
        return VLC_EGENERIC;
    }

	// use requested width/height if available and set
	if (WaitForSingleObject(cfg->hDataLock, 2000) == WAIT_OBJECT_0){
		svmem = cfg->svmem = (TSVMEM*)cfg->hMem;
		if (svmem->hdr.rwidth >= 32 && svmem->hdr.rwidth <= 640){
			if (svmem->hdr.rheight >= 32 && svmem->hdr.rheight <= 480){
				i_width = svmem->hdr.rwidth;
				i_height = svmem->hdr.rheight;
			}
		}
		ReleaseSemaphore(cfg->hDataLock, 1, NULL);
	}

	// set a default pitch. is updated later
    i_pitch = i_width*4;
    
    if (i_pitch*i_height > SVIDEOLENGTH){
		i_width = 640;
		i_height = 480;
	}
    if (i_width < 32) i_width = 32;
	if (i_height < 32) i_height = 32;
	
	desWidth = i_width;
	desHeight = i_height;
	video_format_t *source = &vd->source;
	const int bg_h = desHeight;
	const int bg_w = desWidth;
	const int fg_w = source->i_width;
	const int fg_h = source->i_height;
	const int fg_sar_num = source->i_sar_num;
	const int fg_sar_den = source->i_sar_den;
	const int bg_sar_den = 1;
	const int bg_sar_num = 1;

	w = bg_w;
	h = ( bg_w * fg_h * fg_sar_den * bg_sar_num ) / (float)( fg_w * fg_sar_num * bg_sar_den );
	if (h > desHeight){
		w = ( bg_h * fg_w * fg_sar_num * bg_sar_den ) / (float)( fg_h * fg_sar_den * bg_sar_num );
		h = bg_h;
	}

	if (w > desWidth)
		w = desWidth;
	if (h > desHeight)
		h = desHeight;

	if (WaitForSingleObject(cfg->hDataLock, 2000) == WAIT_OBJECT_0){
		svmem = cfg->svmem = (TSVMEM*)cfg->hMem;
		svmem->hdr.swidth = fmt.i_width;
		svmem->hdr.sheight = fmt.i_height;
			
		svmem->hdr.fsize = w*h*3;
		svmem->hdr.width = w;
		svmem->hdr.height = h;
		svmem->hdr.bpp = 24;
		svmem->pixels = 0;

		svmem->hdr.time = 0;
		svmem->hdr.count = 0;
		ReleaseSemaphore(cfg->hDataLock, 1, NULL);
	}else{
		msg_Err(vd, "Could not create or obtain semaphore lock\n");
       	return VLC_EGENERIC;
	}

    fmt.i_chroma = i_chroma;
    fmt.i_width  = w;
    fmt.i_height = h;

    /* Define the bitmasks */
    switch(i_chroma)
    {
      case VLC_CODEC_RGB15:
        fmt.i_rmask = 0x001f;
        fmt.i_gmask = 0x03e0;
        fmt.i_bmask = 0x7c00;
        svmem->hdr.bpp = 15;
        i_pitch = w*2;
        break;

      case VLC_CODEC_RGB16:
        fmt.i_rmask = 0x001f;
        fmt.i_gmask = 0x07e0;
        fmt.i_bmask = 0xf800;
        svmem->hdr.bpp = 16;
        i_pitch = w*2;
        break;

      case VLC_CODEC_RGB24:
        fmt.i_rmask = 0xff0000;
        fmt.i_gmask = 0x00ff00;
        fmt.i_bmask = 0x0000ff;
        svmem->hdr.bpp = 24;
        i_pitch = w*3;
        break;

	  case VLC_CODEC_RGBA:
      case VLC_CODEC_RGB32:
        fmt.i_rmask = 0xff0000;
        fmt.i_gmask = 0x00ff00;
        fmt.i_bmask = 0x0000ff;
        svmem->hdr.bpp = 32;
        i_pitch = w*4;
        break;
    }
	svmem->hdr.fsize = i_pitch*h;

    /* */
    vout_display_sys_t *sys;
    vd->sys = sys = calloc(1, sizeof(*sys));
    if (!sys)
        return VLC_EGENERIC;
      sys->p_sys = cfg;

    /* */
    picture_resource_t rsc;
    rsc.p_sys = cfg;
    for (int i = 0; i < PICTURE_PLANE_MAX; i++) {
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
    vout_display_SendEventDisplaySize(vd, fmt.i_width, fmt.i_height, false);
    return VLC_SUCCESS;
}

static void Close(vlc_object_t *object)
{
    vout_display_t *vd = (vout_display_t *)object;
    vout_display_sys_t *sys = vd->sys;
    picture_sys_t *p_sys = sys->p_sys;

	if (WaitForSingleObject(p_sys->hDataLock, 1000) == WAIT_OBJECT_0){
		p_sys->svmem->hdr.count = 0;
		p_sys->svmem->hdr.time = 0;
		p_sys->svmem->hdr.ssize = 0;
		ReleaseSemaphore(p_sys->hDataLock, 1, NULL);
		SetEvent(p_sys->hUpdateEvent);
	}
	closeSharedMemory(p_sys);

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

static int Control(vout_display_t *vd, int query, va_list args)
{
    switch (query) {
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
    }
}

static void Manage(vout_display_t *vd)
{
    VLC_UNUSED(vd);
}

/* */
static int Lock(picture_t *picture)
{
    picture_sys_t *p_sys = picture->p_sys;

    int i_index;
    uint8_t *pixels = (uint8_t*)&p_sys->svmem->pixels;

    for (i_index = 0; i_index < picture->i_planes; i_index++)
        picture->p[i_index].p_pixels = pixels;

	if (WaitForSingleObject(p_sys->hDataLock, 1000) == WAIT_OBJECT_0){
    	return VLC_SUCCESS;
    }else{
    	//msg_Err(p_vout, "Could not obtain data semaphore lock\n");
    	return VLC_EGENERIC;
    }
}

static void Unlock(picture_t *picture)
{
    picture_sys_t *p_sys = picture->p_sys;

    TSVMEM *svmem = p_sys->svmem;
    svmem->hdr.time = GetTickCount();
	svmem->hdr.count++;
	
	/*uint32_t *pixels = (uint32_t*)&p_vout->p_sys->svmem->pixels;
	const int tpixels = svmem->hdr.width * svmem->hdr.height;
	int i;
	for (i = 0; i < tpixels; i++){
		if ((pixels[i] & 0x00FFFFFF)){
			pixels[i] |= 0x80000000;
		}else{
			pixels[i]  = 0x80000000;
		}
	}*/

	ReleaseSemaphore(p_sys->hDataLock, 1, NULL);
	SetEvent(p_sys->hUpdateEvent);
}

