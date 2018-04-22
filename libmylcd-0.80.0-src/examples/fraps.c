/*
How can I get access to the Fraps data in realtime?

Starting with Fraps 1.9C it is possible to retreive realtime statistics such as current FPS and game name.
 Fraps maps itself into all processes that use graphics (or have a GUI).

NOTE: Fraps is ONLY present in applications that have a GUI.
Make sure you have created a GUI element such as a window or dialog before you try to query Fraps.
If you are using a console app you can fake a GUI by creating a hidden window upon startup.
Fraps will then be mapped into this process and you can query the data as normal.

The data is shared in the following structure, where a DWORD is 4 bytes, and char is 1 byte.

DWORD sizeOfStruct;
DWORD currentFPS;
DWORD totalFrames;
DWORD timeOfLastFrame;
char gameName[32];

A brief description for each element:
# sizeOfStruct - is the size of this structure in bytes.
This may increase in future versions when other variables are added.
# currentFPS - is the current frame rate as displayed on screen by Fraps.
# totalFrames - is the total number of frames rendered since the game began.
This value will be reset to 0 when manual framerate logging is used, or when another game is loaded.
# timeOfLastFrame - is the time in milliseconds that the last frame was rendered.
This is the value returned by GetTickCount. You can use this value to determine when a game is active
or not (by comparing it with the current value of GetTickCount).
# gameName - is the name of the current game executable.

All elements should be considered read only since Fraps can update them at any time.
*/


#include <stdio.h>
#include <windows.h>

#include "mylcd.h"
#include "demos.h"

typedef struct  {
   DWORD sizeOfStruct;
   DWORD currentFPS;
   DWORD totalFrames;
   DWORD timeOfLastFrame;
   char gameName[32];
}FRAPS_SHARED_DATA;


LRESULT CALLBACK WindowProcedure (HWND, UINT, WPARAM, LPARAM);
FRAPS_SHARED_DATA *(WINAPI *FrapsSharedData)();
int initGUI ();
int initFraps ();

char szClassName[] = "mylcdfrapswin";
HMODULE frapsDLL = NULL;
FRAPS_SHARED_DATA *fsd = NULL;




int main ()
{

	initGUI();
	if (!initFraps())
		return 0;
	if (!initDemoConfig("config.cfg")){
		printf("could not start myLCD\n");
		return 0;
	}

	TLPRINTR rect={0};

	while((fsd = FrapsSharedData()) && !kbhit()){
		memset(&rect, 0, sizeof(TLPRINTR));
		lClearFrame(frame);
		lPrintEx(frame, &rect, LFTW_WENQUANYI9PT, PF_MIDDLEJUSTIFY|PF_CLIPWRAP, LPRT_CPY, "%s", fsd->gameName);
		lPrintEx(frame, &rect, LFTW_B24, PF_MIDDLEJUSTIFY|PF_NEWLINE|PF_CLIPWRAP, LPRT_OR, "%d", (int)fsd->currentFPS);
   		lRefresh(frame);
   		lSleep(400);
	}
	
	demoCleanup();
	return 1;
}


int initFraps  ()
{
	frapsDLL = GetModuleHandle("FRAPS.DLL");
	if (!frapsDLL) {
		printf("fraps.dll not found\n");
		return 0;
	}

	FrapsSharedData = (void *)GetProcAddress(frapsDLL, "FrapsSharedData");
	if (!FrapsSharedData) {
		printf("fraps 1.9c or later required\n");
		return 0;
	}
	return 1;
}


int initGUI ()
{
    HWND hwnd;
    MSG messages;
    WNDCLASSEX wincl;
	HANDLE hThisInstance = GetModuleHandle(0);

    wincl.hInstance = hThisInstance;
    wincl.lpszClassName = szClassName;
    wincl.lpfnWndProc = WindowProcedure;
    wincl.cbSize = sizeof (WNDCLASSEX);
    wincl.hIcon = LoadIcon (NULL, IDI_APPLICATION);
    wincl.hIconSm = LoadIcon (NULL, IDI_APPLICATION);
    wincl.hCursor = LoadCursor (NULL, IDC_ARROW);
    wincl.hbrBackground = (HBRUSH)COLOR_BACKGROUND;
    wincl.style = CS_DBLCLKS;
    wincl.lpszMenuName = NULL;
    wincl.cbClsExtra = 0;
    wincl.cbWndExtra = 0;

    if (!RegisterClassEx (&wincl))
        return 0;

    hwnd = CreateWindowEx(0,szClassName,"fraps",WS_OVERLAPPEDWINDOW,CW_USEDEFAULT,CW_USEDEFAULT,20,20,\
           HWND_DESKTOP,
           NULL,
           hThisInstance,
           NULL
           );

    ShowWindow(hwnd, SW_HIDE);
/*
    while (GetMessage(&messages, NULL, 0, 0)){
        TranslateMessage(&messages);
        DispatchMessage(&messages);
    }
*/
    messages.wParam = 0;
    return 1;// messages.wParam;
}

LRESULT CALLBACK WindowProcedure (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	
    switch (message){
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc (hwnd, message, wParam, lParam);
    }
	
    return 0;
}
