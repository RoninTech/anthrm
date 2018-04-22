
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


#include "common.h"

void setAR (TVLCPLAYER *vp, int arButton)
{

	if (arButton > CFGBUTTON_AR_240)
		arButton = CFGBUTTON_AR_AUTO;

	buttonDisable(vp, PAGE_CFG, CFGBUTTON_AR_AUTO);
	buttonDisable(vp, PAGE_CFG, CFGBUTTON_AR_177);
	buttonDisable(vp, PAGE_CFG, CFGBUTTON_AR_155);
	buttonDisable(vp, PAGE_CFG, CFGBUTTON_AR_133);
	buttonDisable(vp, PAGE_CFG, CFGBUTTON_AR_125);
	buttonDisable(vp, PAGE_CFG, CFGBUTTON_AR_122);
	buttonDisable(vp, PAGE_CFG, CFGBUTTON_AR_15);
	buttonDisable(vp, PAGE_CFG, CFGBUTTON_AR_16);
	buttonDisable(vp, PAGE_CFG, CFGBUTTON_AR_167);
	buttonDisable(vp, PAGE_CFG, CFGBUTTON_AR_185);
	buttonDisable(vp, PAGE_CFG, CFGBUTTON_AR_220);
	buttonDisable(vp, PAGE_CFG, CFGBUTTON_AR_235);
	buttonDisable(vp, PAGE_CFG, CFGBUTTON_AR_240);
	buttonEnable(vp, PAGE_CFG, arButton);
	vp->gui.ratio = arButton/* - CFGBUTTON_AR_AUTO*/;
}

void reloadSkin (TVLCPLAYER *vp)
{
	if (renderLock(vp)){
		if (vp->applState){
			for (int i = 0; i < PAGE_TOTAL; i++){
				TBUTTONS *buttons = buttonsGetContainer(vp, i);
				if (buttons)
					buttonsFreeStorage(buttons->list, buttons->total);
			}
			imageCacheReloadImages(vp);			
		}
		renderUnlock(vp);
	}
}

void setVis (TVLCPLAYER *vp, int visButton)
{

	if (visButton > CFGBUTTON_VIS_GOOM_Q1)
		visButton = CFGBUTTON_VIS_DISABLED;
		
	buttonDisable(vp, PAGE_CFG, CFGBUTTON_VIS_DISABLED);
	//buttonDisable(vp, PAGE_CFG, CFGBUTTON_VIS_DISABLED_IDLE);
	buttonDisable(vp, PAGE_CFG, CFGBUTTON_VIS_VUMETER);
	buttonDisable(vp, PAGE_CFG, CFGBUTTON_VIS_SMETER);
	buttonDisable(vp, PAGE_CFG, CFGBUTTON_VIS_PINEAPPLE);
	buttonDisable(vp, PAGE_CFG, CFGBUTTON_VIS_SPECTRUM);
	buttonDisable(vp, PAGE_CFG, CFGBUTTON_VIS_SCOPE);
	buttonDisable(vp, PAGE_CFG, CFGBUTTON_VIS_GOOM_Q3);
	buttonDisable(vp, PAGE_CFG, CFGBUTTON_VIS_GOOM_Q2);
	buttonDisable(vp, PAGE_CFG, CFGBUTTON_VIS_GOOM_Q1);
		
	buttonEnable(vp, PAGE_CFG, visButton);
	vp->gui.visuals = (visButton - CFGBUTTON_VIS_DISABLED);

#if 0
	if (visButton == CFGBUTTON_VIS_DISABLED_IDLE){
		startRefreshTicker(vp, 10);
	}else if (visButton == CFGBUTTON_VIS_VUMETER){
		startRefreshTicker(vp, (float)vp->gui.targetFPS);
	}
#endif

}

void setBrightness (TVLCPLAYER *vp, int arButton)
{

	if (arButton > CFGBUTTON_BRN_100)
		arButton = CFGBUTTON_BRN_0;

	buttonDisable(vp, PAGE_CFG, CFGBUTTON_BRN_0);
	buttonDisable(vp, PAGE_CFG, CFGBUTTON_BRN_10);
	buttonDisable(vp, PAGE_CFG, CFGBUTTON_BRN_20);
	buttonDisable(vp, PAGE_CFG, CFGBUTTON_BRN_30);
	buttonDisable(vp, PAGE_CFG, CFGBUTTON_BRN_40);
	buttonDisable(vp, PAGE_CFG, CFGBUTTON_BRN_50);
	buttonDisable(vp, PAGE_CFG, CFGBUTTON_BRN_60);
	buttonDisable(vp, PAGE_CFG, CFGBUTTON_BRN_70);
	buttonDisable(vp, PAGE_CFG, CFGBUTTON_BRN_80);
	buttonDisable(vp, PAGE_CFG, CFGBUTTON_BRN_90);
	buttonDisable(vp, PAGE_CFG, CFGBUTTON_BRN_100);
	buttonEnable(vp, PAGE_CFG, arButton);
	vp->gui.brightness = arButton - CFGBUTTON_BRN_0;
	setDisplayBrightness(vp, (255.0/100.0)*(float)(vp->gui.brightness*10));
	setDisplayBrightness(vp, (255.0/100.0)*(float)(vp->gui.brightness*10));
}

void setFPS (TVLCPLAYER *vp, int fpsButton)
{

	if (fpsButton > CFGBUTTON_UR_40)
		fpsButton = CFGBUTTON_UR_5;
		
	buttonDisable(vp, PAGE_CFG, CFGBUTTON_UR_5);
	buttonDisable(vp, PAGE_CFG, CFGBUTTON_UR_10);
	buttonDisable(vp, PAGE_CFG, CFGBUTTON_UR_15);
	buttonDisable(vp, PAGE_CFG, CFGBUTTON_UR_20);
	buttonDisable(vp, PAGE_CFG, CFGBUTTON_UR_25);
	buttonDisable(vp, PAGE_CFG, CFGBUTTON_UR_30);
	buttonDisable(vp, PAGE_CFG, CFGBUTTON_UR_35);
	buttonDisable(vp, PAGE_CFG, CFGBUTTON_UR_40);
	buttonEnable(vp, PAGE_CFG, fpsButton);
	vp->gui.targetFPS = (1+fpsButton - CFGBUTTON_UR_5) * 5;
	if (vp->applState)
		startRefreshTicker(vp, (float)vp->gui.targetFPS);
}

void setRBSwap (TVLCPLAYER *vp, int state)
{
	if (state){
		buttonEnable(vp, PAGE_CFG, CFGBUTTON_SWAPRB_ON);
		buttonDisable(vp, PAGE_CFG, CFGBUTTON_SWAPRB_OFF);
	}else{
		buttonEnable(vp, PAGE_CFG, CFGBUTTON_SWAPRB_OFF);
		buttonDisable(vp, PAGE_CFG, CFGBUTTON_SWAPRB_ON);
	}
	vp->vlc->swapColourBits = state&0x01;
}

int buttonCfg (const TTOUCHCOORD *pos, TBUTTON *button, const int id, const int message, void *ptr)
{
	TVLCPLAYER *vp = (TVLCPLAYER*)ptr;
	
	switch (id){
	  case CFGBUTTON_AR_AUTO:
	  case CFGBUTTON_AR_177:
	  case CFGBUTTON_AR_155:
	  case CFGBUTTON_AR_133:
	  case CFGBUTTON_AR_125:
	  case CFGBUTTON_AR_122:
	  case CFGBUTTON_AR_15:
	  case CFGBUTTON_AR_16:
	  case CFGBUTTON_AR_167:
	  case CFGBUTTON_AR_185:
	  case CFGBUTTON_AR_220:
	  case CFGBUTTON_AR_235:
	  case CFGBUTTON_AR_240:
	    setAR(vp, id+1);
	  	break;

	  case CFGBUTTON_VIS_OFF:
	  case CFGBUTTON_VIS_DISABLED:
	  //case CFGBUTTON_VIS_DISABLED_IDLE:
	  case CFGBUTTON_VIS_VUMETER:
	  case CFGBUTTON_VIS_SMETER:
	  case CFGBUTTON_VIS_PINEAPPLE:
	  case CFGBUTTON_VIS_SPECTRUM:
	  case CFGBUTTON_VIS_SCOPE:
	  case CFGBUTTON_VIS_GOOM_Q3:
	  case CFGBUTTON_VIS_GOOM_Q2:
	  case CFGBUTTON_VIS_GOOM_Q1:
	  	setVis(vp, id+1);

	  	if (getPlayState(vp)){
	  		char path[MAX_PATH_UTF8];
	  		
	  		PLAYLISTCACHE *plcQ = getQueuedPlaylist(vp);
			if (plcQ){
	  			playlistGetPath(plcQ, plcQ->pr->playingItem, path, sizeof(path));
	  			if (*path){
	  				const int volume = vp->vlc->volume;
	  				setVolume(vp, 0);
	  				const float position = vlc_getPosition(vp->vlc); //vp->vlc->position;
	  				player_stop(vp, vp->vlc);
	  				cleanVideoBuffers(vp);
	  				browserLoadMediaFile(vp, path);
	  				lSleep(50);	// we need this or bad things happen
		  			vlc_setPosition(vp->vlc, position);
	  				setVolume(vp, volume);
	  			}
	  		}
	  	}
		break;

	  case CFGBUTTON_BRN_0:
	  case CFGBUTTON_BRN_10:
	  case CFGBUTTON_BRN_20:
	  case CFGBUTTON_BRN_30:
	  case CFGBUTTON_BRN_40:
	  case CFGBUTTON_BRN_50:
	  case CFGBUTTON_BRN_60:
	  case CFGBUTTON_BRN_70:
	  case CFGBUTTON_BRN_80:
	  case CFGBUTTON_BRN_90:
	  case CFGBUTTON_BRN_100:
		setBrightness(vp, id+1);
		break;

	  case CFGBUTTON_UR_5:
	  case CFGBUTTON_UR_10:
	  case CFGBUTTON_UR_15:
	  case CFGBUTTON_UR_20:
	  case CFGBUTTON_UR_25:
	  case CFGBUTTON_UR_30:
	  case CFGBUTTON_UR_35:
	  case CFGBUTTON_UR_40:
		setFPS(vp, id+1);
	  	break;
	  
	  case CFGBUTTON_FPS_ON:
	  	buttonEnable(vp, PAGE_CFG, CFGBUTTON_FPS_OFF);
	  	buttonDisable(vp, PAGE_CFG, CFGBUTTON_FPS_ON);
	  	disableFPS(vp);
	  	break;

	  case CFGBUTTON_FPS_OFF:
	  	buttonEnable(vp, PAGE_CFG, CFGBUTTON_FPS_ON);
	  	buttonDisable(vp, PAGE_CFG, CFGBUTTON_FPS_OFF);
	  	enableFPS(vp);
	  	break;
	  		  		
	  case CFGBUTTON_SKIN:
	  	if (++vp->gui.skin >= TOTALSKINS)
			vp->gui.skin = 0;
	  	reloadSkin(vp);
	  	break;

	  case CFGBUTTON_CLOCK:
		setPageSec(vp, PAGE_CLOCK);
	  	break;

	  case CFGBUTTON_WRITEPLAYLIST:{
  		wchar_t buffer[MAX_PATH];
	  	TCFG *cfg = getPagePtr(vp, PAGE_CFG);
	  	
	  	PLAYLISTCACHE *plcD = getDisplayPlaylist(vp);
	  	if (plcD){
	  		cfg->playlistWriteTotalWritten = writePlaylist(vp, plcD, buffer, sizeof(buffer));
			if (cfg->playlistWriteTotalWritten){
	  			cfg->playlistWriteTime = vp->fTime + 6000;	/* decides how long the filepath will remain on screen */
	  			if (cfg->playlistWriteFilename)
			  		my_free(cfg->playlistWriteFilename);
	  			cfg->playlistWriteFilename = wcsdup(buffer);
	  			if (cfg->playlistWriteFilename == NULL)
		  			cfg->playlistWriteState = 0;
	  			else
	  				cfg->playlistWriteState = 1;
	  		}
	  	}
		break;
	  }
	  case CFGBUTTON_SUBTITLE:
	  	setPage(vp, PAGE_SUB);
	  	break;

	  case CFGBUTTON_SWAPRB_ON:
		setRBSwap(vp, 0);
	  	break;

	  case CFGBUTTON_SWAPRB_OFF:
		setRBSwap(vp, 1);
	  	break;

	  case CFGBUTTON_CLOSE:
	  	setPage(vp, PAGE_NONE);
	  	break;
	}
	return 0;
}

int closeCfg (TVLCPLAYER *vp, TFRAME *frame, TCFG *cfg)
{
	buttonsDeleteContainer(vp, PAGE_CFG);
	return 1;
}

int openCfg (TVLCPLAYER *vp, TFRAME *frame, TCFG *cfg)
{
#if !G19DISPLAY
	int x = 10;
	int y = 10;
#else
	int x = 5;
	int y = 5;
#endif

	buttonsCreateContainer(vp, PAGE_CFG, CFGBUTTON_TOTAL);

	TBUTTON *button = buttonGet(vp, PAGE_CFG, CFGBUTTON_SKIN);
	button->enabled = 1;
	button->id = CFGBUTTON_SKIN;
	button->pos.x = frame->width-102;
#if !G19DISPLAY
	button->pos.y = frame->height*0.10;
#else
	button->pos.y = frame->height*0.42;
#endif
	button->canAnimate = 0;
	button->callback = buttonCfg;
	buttonSetImages(vp, button, L"title.png", NULL);

	button = buttonGet(vp, PAGE_CFG, CFGBUTTON_WRITEPLAYLIST);
	button->enabled = 1;
	button->id = CFGBUTTON_WRITEPLAYLIST;
	button->pos.x = frame->width-(64+54);
#if !G19DISPLAY
	button->pos.y = frame->height*0.27;
#else
	button->pos.y = frame->height*0.28;
#endif
	button->canAnimate = 1;
	button->callback = buttonCfg;
	buttonSetImages(vp, button, L"wplaylist.png", NULL);
			
#if 0
	button = buttonGet(vp, PAGE_CFG, CFGBUTTON_CLOCK);
	button->enabled = 1;
	button->id = CFGBUTTON_CLOCK;
	button->pos.x = frame->width-(64+6);
	button->pos.y = frame->height*0.27;
	button->canAnimate = 0;
	button->callback = buttonCfg;
	buttonSetImages(vp, button, L"clock.png", NULL);
#endif

	button = buttonGet(vp, PAGE_CFG, CFGBUTTON_SUBTITLE);
	button->enabled = 1;
	button->id = CFGBUTTON_SUBTITLE;
	button->pos.x = frame->width-85;
#if !G19DISPLAY
	button->pos.y = frame->height*0.45;
#else
	button->pos.y = frame->height*0.59;
#endif
	button->canAnimate = 0;
	button->callback = buttonCfg;
	buttonSetImages(vp, button, L"subtitles.png", NULL);
		
	button = buttonGet(vp, PAGE_CFG, CFGBUTTON_CLOSE);
	button->enabled = 1;
	button->id = CFGBUTTON_CLOSE;
	button->pos.x = frame->width-(64+6);
#if !G19DISPLAY
	button->pos.y = frame->height*0.62;
#else
	button->pos.y = frame->height*0.76;
#endif
	button->canAnimate = 0;
	button->callback = buttonCfg;
	buttonSetImages(vp, button, L"cfgclose.png", NULL);
		
	button = buttonGet(vp, PAGE_CFG, CFGBUTTON_AR_AUTO);
	button->enabled = 0;
	button->id = CFGBUTTON_AR_AUTO;
	button->pos.x = x;
	button->pos.y = y;
	button->canAnimate = 0;
	button->callback = buttonCfg;
	buttonSetImages(vp, button, L"arauto.png", NULL);
		
	button = buttonGet(vp, PAGE_CFG, CFGBUTTON_AR_177);
	button->enabled = 0;
	button->id = CFGBUTTON_AR_177;
	button->pos.x = x;
	button->pos.y = y;
	button->canAnimate = 0;
	button->callback = buttonCfg;
	buttonSetImages(vp, button, L"ar177.png", NULL);

	button = buttonGet(vp, PAGE_CFG, CFGBUTTON_AR_155);
	button->enabled = 0;
	button->id = CFGBUTTON_AR_155;
	button->pos.x = x;
	button->pos.y = y;
	button->canAnimate = 0;
	button->callback = buttonCfg;
	buttonSetImages(vp, button, L"ar155.png", NULL);		

	button = buttonGet(vp, PAGE_CFG, CFGBUTTON_AR_133);
	button->enabled = 0;
	button->id = CFGBUTTON_AR_133;
	button->pos.x = x;
	button->pos.y = y;
	button->canAnimate = 0;
	button->callback = buttonCfg;
	buttonSetImages(vp, button, L"ar133.png", NULL);		
	
	button = buttonGet(vp, PAGE_CFG, CFGBUTTON_AR_125);
	button->enabled = 0;
	button->id = CFGBUTTON_AR_125;
	button->pos.x = x;
	button->pos.y = y;
	button->canAnimate = 0;
	button->callback = buttonCfg;
	buttonSetImages(vp, button, L"ar125.png", NULL);

	button = buttonGet(vp, PAGE_CFG, CFGBUTTON_AR_122);
	button->enabled = 0;
	button->id = CFGBUTTON_AR_122;
	button->pos.x = x;
	button->pos.y = y;
	button->canAnimate = 0;
	button->callback = buttonCfg;
	buttonSetImages(vp, button, L"ar122.png", NULL);
			
	button = buttonGet(vp, PAGE_CFG, CFGBUTTON_AR_15);
	button->enabled = 0;
	button->id = CFGBUTTON_AR_15;
	button->pos.x = x;
	button->pos.y = y;
	button->canAnimate = 0;
	button->callback = buttonCfg;
	buttonSetImages(vp, button, L"ar15.png", NULL);
	
	button = buttonGet(vp, PAGE_CFG, CFGBUTTON_AR_16);
	button->enabled = 0;
	button->id = CFGBUTTON_AR_16;
	button->pos.x = x;
	button->pos.y = y;
	button->canAnimate = 0;
	button->callback = buttonCfg;
	buttonSetImages(vp, button, L"ar16.png", NULL);

	button = buttonGet(vp, PAGE_CFG, CFGBUTTON_AR_167);
	button->enabled = 0;
	button->id = CFGBUTTON_AR_167;
	button->pos.x = x;
	button->pos.y = y;
	button->canAnimate = 0;
	button->callback = buttonCfg;
	buttonSetImages(vp, button, L"ar167.png", NULL);
				
	button = buttonGet(vp, PAGE_CFG, CFGBUTTON_AR_185);
	button->enabled = 0;
	button->id = CFGBUTTON_AR_185;
	button->pos.x = x;
	button->pos.y = y;
	button->canAnimate = 0;
	button->callback = buttonCfg;
	buttonSetImages(vp, button, L"ar185.png", NULL);
	
	button = buttonGet(vp, PAGE_CFG, CFGBUTTON_AR_220);
	button->enabled = 0;
	button->id = CFGBUTTON_AR_220;
	button->pos.x = x;
	button->pos.y = y;
	button->canAnimate = 0;
	button->callback = buttonCfg;
	buttonSetImages(vp, button, L"ar220.png", NULL);
	
	button = buttonGet(vp, PAGE_CFG, CFGBUTTON_AR_235);
	button->enabled = 0;
	button->id = CFGBUTTON_AR_235;
	button->pos.x = x;
	button->pos.y = y;
	button->canAnimate = 0;
	button->callback = buttonCfg;
	buttonSetImages(vp, button, L"ar235.png", NULL);
	
	button = buttonGet(vp, PAGE_CFG, CFGBUTTON_AR_240);
	button->enabled = 0;
	button->id = CFGBUTTON_AR_240;
	button->pos.x = x;
	button->pos.y = y;
	button->canAnimate = 0;
	button->callback = buttonCfg;
	buttonSetImages(vp, button, L"ar240.png", NULL);

	button = buttonGet(vp, PAGE_CFG, CFGBUTTON_VIS_VUMETER);
	button->enabled = 0;
	button->id = CFGBUTTON_VIS_VUMETER;
	button->pos.x = x;
#if !G19DISPLAY
	button->pos.y = (y+=45);
#else
	button->pos.y = (y+=37);
#endif
	button->canAnimate = 0;
	button->callback = buttonCfg;
	buttonSetImages(vp, button, L"vvumeter.png", NULL);
	
	button = buttonGet(vp, PAGE_CFG, CFGBUTTON_VIS_SMETER);
	button->enabled = 0;
	button->id = CFGBUTTON_VIS_SMETER;
	button->pos.x = x;
	button->pos.y = y;
	button->canAnimate = 0;
	button->callback = buttonCfg;
	buttonSetImages(vp, button, L"vspectrometer.png", NULL);

	button = buttonGet(vp, PAGE_CFG, CFGBUTTON_VIS_PINEAPPLE);
	button->enabled = 0;
	button->id = CFGBUTTON_VIS_PINEAPPLE;
	button->pos.x = x;
	button->pos.y = y;
	button->canAnimate = 0;
	button->callback = buttonCfg;
	buttonSetImages(vp, button, L"vpineapple.png", NULL);
	
	button = buttonGet(vp, PAGE_CFG, CFGBUTTON_VIS_SPECTRUM);
	button->enabled = 0;
	button->id = CFGBUTTON_VIS_SPECTRUM;
	button->pos.x = x;
	button->pos.y = y;
	button->canAnimate = 0;
	button->callback = buttonCfg;
	buttonSetImages(vp, button, L"vspectrum.png", NULL);
	
	button = buttonGet(vp, PAGE_CFG, CFGBUTTON_VIS_SCOPE);
	button->enabled = 0;
	button->id = CFGBUTTON_VIS_SCOPE;
	button->pos.x = x;
	button->pos.y = y;
	button->canAnimate = 0;
	button->callback = buttonCfg;
	buttonSetImages(vp, button, L"vscope.png", NULL);

	button = buttonGet(vp, PAGE_CFG, CFGBUTTON_VIS_GOOM_Q3);
	button->enabled = 1;
	button->id = CFGBUTTON_VIS_GOOM_Q3;
	button->pos.x = x;
	button->pos.y = y;
	button->canAnimate = 0;
	button->callback = buttonCfg;
	buttonSetImages(vp, button, L"vgoomq3.png", NULL);

	button = buttonGet(vp, PAGE_CFG, CFGBUTTON_VIS_GOOM_Q2);
	button->enabled = 1;
	button->id = CFGBUTTON_VIS_GOOM_Q2;
	button->pos.x = x;
	button->pos.y = y;
	button->canAnimate = 0;
	button->callback = buttonCfg;
	buttonSetImages(vp, button, L"vgoomq2.png", NULL);
	
	button = buttonGet(vp, PAGE_CFG, CFGBUTTON_VIS_GOOM_Q1);
	button->enabled = 1;
	button->id = CFGBUTTON_VIS_GOOM_Q1;
	button->pos.x = x;
	button->pos.y = y;
	button->canAnimate = 0;
	button->callback = buttonCfg;
	buttonSetImages(vp, button, L"vgoomq1.png", NULL);
			
	button = buttonGet(vp, PAGE_CFG, CFGBUTTON_VIS_DISABLED);
	button->enabled = 0;
	button->id = CFGBUTTON_VIS_DISABLED;
	button->pos.x = x;
	button->pos.y = y;
	button->canAnimate = 0;
	button->callback = buttonCfg;
	buttonSetImages(vp, button, L"vdisabled.png", NULL);

/*	button = buttonGet(vp, PAGE_CFG, CFGBUTTON_VIS_DISABLED_IDLE);
	button->enabled = 0;
	button->id = CFGBUTTON_VIS_DISABLED_IDLE;
	button->pos.x = x;
	button->pos.y = y;
	button->canAnimate = 0;
	button->callback = buttonCfg;
	buttonSetImages(vp, button, L"vdisabled_idle.png", NULL);*/
		
	button = buttonGet(vp, PAGE_CFG, CFGBUTTON_FPS_ON);
	button->enabled = 0;
	button->id = CFGBUTTON_FPS_ON;
	button->pos.x = x;
	button->pos.y = (y+=45);
	button->canAnimate = 0;
	button->callback = buttonCfg;
	buttonSetImages(vp, button, L"fpson.png", NULL);
	
	button = buttonGet(vp, PAGE_CFG, CFGBUTTON_FPS_OFF);
	button->enabled = 1;
	button->id = CFGBUTTON_FPS_OFF;
	button->pos.x = x;
	button->pos.y = y;
	button->canAnimate = 0;
	button->callback = buttonCfg;
	buttonSetImages(vp, button, L"fpsoff.png", NULL);
			
	button = buttonGet(vp, PAGE_CFG, CFGBUTTON_SWAPRB_ON);
	button->enabled = 0;
	button->id = CFGBUTTON_SWAPRB_ON;
	button->pos.x = x;
	button->pos.y = (y+=45);
	button->canAnimate = 0;
	button->callback = buttonCfg;
	buttonSetImages(vp, button, L"swaprbon.png", NULL);

	button = buttonGet(vp, PAGE_CFG, CFGBUTTON_SWAPRB_OFF);
	button->enabled = 0;
	button->id = CFGBUTTON_SWAPRB_OFF;
	button->pos.x = x;
	button->pos.y = y;
	button->canAnimate = 0;
	button->callback = buttonCfg;
	buttonSetImages(vp, button, L"swaprboff.png", NULL);

#if !G19DISPLAY
	button = buttonGet(vp, PAGE_CFG, CFGBUTTON_BRN_0);
	button->enabled = 0;
	button->id = CFGBUTTON_BRN_0;
	button->pos.x = x;
	button->pos.y = (y+=45);
	button->canAnimate = 0;
	button->callback = buttonCfg;
	buttonSetImages(vp, button, L"brn0.png", NULL);

	button = buttonGet(vp, PAGE_CFG, CFGBUTTON_BRN_10);
	button->enabled = 0;
	button->id = CFGBUTTON_BRN_10;
	button->pos.x = x;
	button->pos.y = y;
	button->canAnimate = 0;
	button->callback = buttonCfg;
	buttonSetImages(vp, button, L"brn10.png", NULL);

	button = buttonGet(vp, PAGE_CFG, CFGBUTTON_BRN_20);
	button->enabled = 0;
	button->id = CFGBUTTON_BRN_20;
	button->pos.x = x;
	button->pos.y = y;
	button->canAnimate = 0;
	button->callback = buttonCfg;
	buttonSetImages(vp, button, L"brn20.png", NULL);
	
	button = buttonGet(vp, PAGE_CFG, CFGBUTTON_BRN_30);
	button->enabled = 0;
	button->id = CFGBUTTON_BRN_30;
	button->pos.x = x;
	button->pos.y = y;
	button->canAnimate = 0;
	button->callback = buttonCfg;
	buttonSetImages(vp, button, L"brn30.png", NULL);
	
	button = buttonGet(vp, PAGE_CFG, CFGBUTTON_BRN_40);
	button->enabled = 0;
	button->id = CFGBUTTON_BRN_40;
	button->pos.x = x;
	button->pos.y = y;
	button->canAnimate = 0;
	button->callback = buttonCfg;
	buttonSetImages(vp, button, L"brn40.png", NULL);
	
	button = buttonGet(vp, PAGE_CFG, CFGBUTTON_BRN_50);
	button->enabled = 0;
	button->id = CFGBUTTON_BRN_50;
	button->pos.x = x;
	button->pos.y = y;
	button->canAnimate = 0;
	button->callback = buttonCfg;
	buttonSetImages(vp, button, L"brn50.png", NULL);
	
	button = buttonGet(vp, PAGE_CFG, CFGBUTTON_BRN_60);
	button->enabled = 0;
	button->id = CFGBUTTON_BRN_60;
	button->pos.x = x;
	button->pos.y = y;
	button->canAnimate = 0;
	button->callback = buttonCfg;
	buttonSetImages(vp, button, L"brn60.png", NULL);
	
	button = buttonGet(vp, PAGE_CFG, CFGBUTTON_BRN_70);
	button->enabled = 0;
	button->id = CFGBUTTON_BRN_70;
	button->pos.x = x;
	button->pos.y = y;
	button->canAnimate = 0;
	button->callback = buttonCfg;
	buttonSetImages(vp, button, L"brn70.png", NULL);
	
	button = buttonGet(vp, PAGE_CFG, CFGBUTTON_BRN_80);
	button->enabled = 0;
	button->id = CFGBUTTON_BRN_80;
	button->pos.x = x;
	button->pos.y = y;
	button->canAnimate = 0;
	button->callback = buttonCfg;
	buttonSetImages(vp, button, L"brn80.png", NULL);
	
	button = buttonGet(vp, PAGE_CFG, CFGBUTTON_BRN_90);
	button->enabled = 0;
	button->id = CFGBUTTON_BRN_90;
	button->pos.x = x;
	button->pos.y = y;
	button->canAnimate = 0;
	button->callback = buttonCfg;
	buttonSetImages(vp, button, L"brn90.png", NULL);
	
	button = buttonGet(vp, PAGE_CFG, CFGBUTTON_BRN_100);
	button->enabled = 0;
	button->id = CFGBUTTON_BRN_100;
	button->pos.x = x;
	button->pos.y = y;
	button->canAnimate = 0;
	button->callback = buttonCfg;
	buttonSetImages(vp, button, L"brn100.png", NULL);
#endif  // !G19DISPLAY

	button = buttonGet(vp, PAGE_CFG, CFGBUTTON_UR_5);
	button->enabled = 0;
	button->id = CFGBUTTON_UR_5;
	button->pos.x = x;
	button->pos.y = (y+=42);
	button->canAnimate = 0;
	button->callback = buttonCfg;
	buttonSetImages(vp, button, L"ur5.png", NULL);

	button = buttonGet(vp, PAGE_CFG, CFGBUTTON_UR_10);
	button->enabled = 0;
	button->id = CFGBUTTON_UR_10;
	button->pos.x = x;
	button->pos.y = y;
	button->canAnimate = 0;
	button->callback = buttonCfg;
	buttonSetImages(vp, button, L"ur10.png", NULL);

	button = buttonGet(vp, PAGE_CFG, CFGBUTTON_UR_15);
	button->enabled = 0;
	button->id = CFGBUTTON_UR_15;
	button->pos.x = x;
	button->pos.y = y;
	button->canAnimate = 0;
	button->callback = buttonCfg;
	buttonSetImages(vp, button, L"ur15.png", NULL);
	
	button = buttonGet(vp, PAGE_CFG, CFGBUTTON_UR_20);
	button->enabled = 0;
	button->id = CFGBUTTON_UR_20;
	button->pos.x = x;
	button->pos.y = y;
	button->canAnimate = 0;
	button->callback = buttonCfg;
	buttonSetImages(vp, button, L"ur20.png", NULL);
	
	button = buttonGet(vp, PAGE_CFG, CFGBUTTON_UR_25);
	button->enabled = 0;
	button->id = CFGBUTTON_UR_25;
	button->pos.x = x;
	button->pos.y = y;
	button->canAnimate = 0;
	button->callback = buttonCfg;
	buttonSetImages(vp, button, L"ur25.png", NULL);
	
	button = buttonGet(vp, PAGE_CFG, CFGBUTTON_UR_30);
	button->enabled = 0;
	button->id = CFGBUTTON_UR_30;
	button->pos.x = x;
	button->pos.y = y;
	button->canAnimate = 0;
	button->callback = buttonCfg;
	buttonSetImages(vp, button, L"ur30.png", NULL);

	
	button = buttonGet(vp, PAGE_CFG, CFGBUTTON_UR_35);
	button->enabled = 0;
	button->id = CFGBUTTON_UR_35;
	button->pos.x = x;
	button->pos.y = y;
	button->canAnimate = 0;
	button->callback = buttonCfg;
	buttonSetImages(vp, button, L"ur35.png", NULL);
	
	button = buttonGet(vp, PAGE_CFG, CFGBUTTON_UR_40);
	button->enabled = 0;
	button->id = CFGBUTTON_UR_40;
	button->pos.x = x;
	button->pos.y = y;
	button->canAnimate = 0;
	button->callback = buttonCfg;
	buttonSetImages(vp, button, L"ur40.png", NULL);

	
#if !G19DISPLAY
	setBrightness(vp, CFGBUTTON_BRN_80);
#endif

	setVis(vp, CFGBUTTON_VIS_DISABLED);
	setAR(vp, CFGBUTTON_AR_AUTO);
	setRBSwap(vp, 0);
	setFPS(vp, CFGBUTTON_UR_25);
	return 1;
}

int drawCfg (TVLCPLAYER *vp, TFRAME *frame, TCFG *cfg)
{
	TBUTTONS *buttons = buttonsGetContainer(vp, PAGE_CFG);
	
	if (vp->vlc->spu.total > 0)
		buttonEnable(vp, PAGE_CFG, CFGBUTTON_SUBTITLE);
	else
		buttonDisable(vp, PAGE_CFG, CFGBUTTON_SUBTITLE);

	setRBSwap(vp, vp->vlc->swapColourBits);
	buttonsRender(buttons->list, buttons->total, frame);

	if (cfg->playlistWriteState){
		if ((int)vp->fTime - cfg->playlistWriteTime < 0 && cfg->playlistWriteFilename){
			lSetForegroundColour(vp->hw, 0xFFFFFFFF);
			lSetCharacterEncoding(vp->hw, CMT_UTF16);
			wchar_t *fstr;
			if (cfg->playlistWriteTotalWritten > 1)
				fstr = L"%i tracks written to '%s'";
			else
				fstr = L"%i track written to '%s'";
			TFRAME *str = lNewString(vp->hw, frame->bpp, 0, BFONT, (char*)fstr, \
				cfg->playlistWriteTotalWritten, cfg->playlistWriteFilename);
				
			if (str){
				lDrawImage(str, frame, 0, 0);
				lDeleteFrame(str);
			}
		}else{
			cfg->playlistWriteState = 0;
			if (cfg->playlistWriteFilename){
				my_free(cfg->playlistWriteFilename);
				cfg->playlistWriteFilename = NULL;
			}
		}
	}

	
#if DRAWTOUCHRECTS
	TBUTTON *button = buttons->list;
	int i;
	for (i = 0; i < buttons->total; button++, i++){
		if (button->enabled)
			lDrawRectangle(frame, button->pos.x, button->pos.y, button->pos.x+button->image->width-1, button->pos.y+button->image->height-1, 0xFF0000FF);
	}
	//lDrawRectangle(frame, 0,0,frame->width-1, frame->height-1, 0xFF0000FF);
#endif
	return 1;
}

int touchCfg (TTOUCHCOORD *pos, int flags, TVLCPLAYER *vp)
{
	TBUTTONS *buttons = buttonsGetContainer(vp, PAGE_CFG);
	TBUTTON *button = buttons->list;
	TTOUCHCOORD bpos;
	static unsigned int lastId = 0;

	if (!flags){		// pen down
		if (lastId >= pos->id)
			return 0;
		lastId = pos->id;
	}else if (lastId != pos->id){
		return 0;	
	}

	for (int i = 0; i < buttons->total; i++, button++){
		if (button->enabled){
			if (pos->x >= button->pos.x && pos->x <= button->pos.x+button->image->width){
				if (pos->y >= button->pos.y && pos->y <= button->pos.y+button->image->height){
					if (button->highlight)
						enableHighlight(vp, button);
					if (button->canAnimate)
						button->ani.state = 1;

					my_memcpy(&bpos, pos, sizeof(TTOUCHCOORD));
					bpos.x = pos->x - button->pos.x;
					bpos.y = pos->y - button->pos.y;
					if (!button->callback(&bpos, button, button->id, flags, vp))
						break;
				}
			}
		}
	}
	return 1;
}
