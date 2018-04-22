

#include <windows.h>
#include <stdio.h>
#include "plugin.h"
#include "mylcd.h"


#define DUSBD480 (1)

#if DUSBD480
#define DWIDTH	480
#define DHEIGHT	272
#define DBPP	LFRM_BPP_32
const char *device[] = {"USBD480:LIBUSBHID", "USBD480:DLL", "DDRAW"};
#else
#define DWIDTH	320
#define DHEIGHT	240
#define DBPP	LFRM_BPP_32
const char *device[] = {"DDRAW", "DDRAW", "DDRAW"};
#endif

#define EXPORT	__declspec(dllexport) 

#define G15_LEFT		  0x01
#define G15_RIGHT		  0x02
#define G15_OK			  0x04
#define G15_CANCEL		  0x08
#define G15_UP			  0x10
#define G15_DOWN		  0x20
#define G15_MENU		  0x40



static THWD *hw = NULL;
static TFRAME *frame = NULL;
static TRECT display;
static lDISPLAY did;
static LcdCallbacks *LMC = NULL;

static int initialized = 0;
static LcdInfo windows;
static uint32_t lastDt = 0;

int libmylcd_init ();
void libmylcd_shutdown ();
int initLibrary ();
/*
static int RED;
static int GREEN;
static int BLUE;


typedef struct{
	TLPOINTEX rt;
	char *eventName;
	int eventArg;
}TBUTTON;

#define _W  (DWIDTH-1)
#define _H  (DHEIGHT-1)

static TBUTTON buttons[] = {
	{{2, 90, 150, 180}, "G15ButtonDown", G15_LEFT},
	{{330, 90, _W-2, 180}, "G15ButtonDown", G15_RIGHT},
	{{160, 2, 320, 80}, "G15ButtonDown", G15_UP},
	{{160, 190, 320, _H-2}, "G15ButtonDown", G15_DOWN},
	{{165, 95, 315, 175}, "G15ButtonDown", G15_OK},
	{{2, _H-60, 140, _H-2}, "G15ButtonDown", G15_CANCEL},
	{{340, _H-60, _W-2, _H-2}, "G15ButtonDown", G15_MENU},
	{{0, 0, 0, 0}, NULL, 0}
};

void triggerCallback (LcdCallbacks *LMC, TBUTTON *button, TTOUCHCOORD *pos)
{
	char eventArg[32];
	sprintf(eventArg, "%i", button->eventArg);
	LMC->TriggerEvent(LMC->id, button->eventName, eventArg);
}
*/
void CALLBACK Update (LcdInfo *info, BitmapInfo *bmp)
{
	//memcpy(frame->pixels, bmp->bitmap, frame->frameSize);
	
#if 0
	TBUTTON *button = buttons;
	while (button->eventName){
		lDrawRectangleDotted(frame, button->rt.x1, button->rt.y1, button->rt.x2, button->rt.y2, RED|GREEN);
		button++;
	}
#endif

//	lSaveImage(frame, L"lcdmisc.bmp", IMG_BMP, 0, 0);
	//lRefreshAsync(frame, 1);
	lUpdate(hw, bmp->bitmap, frame->frameSize);
}

void CALLBACK Destroy (LcdInfo *info)
{
	initialized = 0;
	LMC = NULL;
}

void InitWindow (LcdInfo *win, const char *name)
{
	memset(win, 0, sizeof(LcdInfo));
	win->bpp = 32;
	win->refreshRate = 0;
	win->id = name;
	win->width = DWIDTH;
	win->height = DHEIGHT;
	win->Update = Update;
	win->Destroy = Destroy;
	initialized = libmylcd_init();
}

EXPORT CALLBACK int lcdInit (LcdCallbacks *LCDCallbacks)
{
	LMC = LCDCallbacks;
#if DUSBD480
	InitWindow(&windows, "USBD480");
#else
	InitWindow(&windows, "G19:DDraw");
#endif
	return 1;
}

EXPORT CALLBACK void lcdUninit ()
{
	libmylcd_shutdown();
	initialized = 0;
	lastDt = GetTickCount();
}

EXPORT CALLBACK LcdInfo * lcdEnum ()
{
	if (initialized == 1){
		initialized = 2;
		return &windows;
	}
	return NULL;
}

int initLibrary ()
{
    if (!(hw=lOpen(NULL, NULL))){
    	return 0;
    }
   	return 1;
}

#if DUSBD480
void touchCB (TTOUCHCOORD *pos, int flags, void *ptr)
{
	if (pos->pen || flags || pos->dt < 125) return;
	uint32_t ticks = GetTickCount();
	pos->dt = ticks - lastDt;
	lastDt = ticks;
	if (pos->dt < 130) return;
	
	char eventArg[64];
	snprintf(eventArg, 64, "%i,%i,%i,%i", pos->x, pos->y, pos->dt, pos->time);
	LMC->TriggerEvent(LMC->id, "touchDown", eventArg);
/*	
	TBUTTON *button = buttons;
	while (button->eventName){
		if (pos->x >= button->rt.x1 && pos->x <= button->rt.x2){
			if (pos->y >= button->rt.y1 && pos->y <= button->rt.y2){
				triggerCallback(LMC, button, pos);
				return;
			}
		}
		button++;
	}
*/
}
#endif

int libmylcd_init ()
{
	if (!initLibrary())
		return 0;

	display.left = 0;
	display.top = 0;
	display.right = DWIDTH-1;
	display.btm = DHEIGHT-1;

	did = lSelectDevice(hw, device[0], "NULL", DWIDTH, DHEIGHT, DBPP, 0, &display);
	if (!did)
		did = lSelectDevice(hw, device[1], "NULL", DWIDTH, DHEIGHT, DBPP, 0, &display);
	if (!did)
		did = lSelectDevice(hw, device[2], "NULL", DWIDTH, DHEIGHT, DBPP, 0, &display);

#if DUSBD480		
	if (did)
		lSetDisplayOption(hw, did, lOPT_USBD480_TOUCHCB, (intptr_t*)touchCB);
#endif

	frame = lNewFrame(hw, DWIDTH, DHEIGHT, DBPP);
/*	RED = lGetRGBMask(frame, LMASK_RED);
	GREEN = lGetRGBMask(frame, LMASK_GREEN);
	BLUE = lGetRGBMask(frame, LMASK_BLUE);*/
	
    return (frame != NULL);
}

void libmylcd_shutdown ()
{
#if DUSBD480
	lSetDisplayOption(hw, did, lOPT_USBD480_TOUCHCB, 0);
#endif
	lDeleteFrame(frame);
	lClose(hw);
}

EXPORT BOOL WINAPI DllMain (HINSTANCE hInstance, DWORD fdwReason, void *lpvReserved)
{
	if (fdwReason == DLL_PROCESS_ATTACH) {
		DisableThreadLibraryCalls(hInstance);
	}else if (fdwReason == DLL_PROCESS_DETACH) {
		//lcdUninit();
		(void)lpvReserved;
	}
    return 1;
}
