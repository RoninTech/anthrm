
#ifndef CLOCXPLUGIN_H
#define CLOCXPLUGIN_H


/* define macro for exporting functions and variables */
#ifdef __cplusplus
#define EXPORT extern "C" __declspec(dllexport)
#else
#define EXPORT __declspec(dllexport) 
#endif

/* structure received from OnInit */
typedef struct {
	HWND MainWindow;  //handle to ClocX main window
	const char *LangFileName;  //name of used language file (can be used for language selection)
}InitData;

/* structure received from OnSkinLoaded function */
typedef struct {
	int Width;  //width of zoomed bitmap
	int Height;  //height of zoomed bitmap
	int AMPMColor;  //color of AM/PM indicator
	int AMPMFontSize;  //zoomed font size of AM/PM indicator 
	int DateColor;  //color of date indicator
	int DateFontSize;  //zoomed size of date indicator
	int ZoomFactor;  //percentual factor of zoom (doubled if using antialiasing)
	POINT ZoomedCenter;  //zoomed coordinates of bitmap center
	POINT AMPMCenter;
	POINT DateCenter;
	HBITMAP LoadedBitmap;  //handle to skin bitmap
	const char *SkinPath;  //path of skin bitmap
}SkinLoadedData;

#endif /* CLOCXPLUGIN_H */

