/*
	defs.h
	Drew Olbrich, 1992
*/

#ifndef _DEFS
#define _DEFS

#define WIN_TITLE   "distort"
#define ICON_TITLE  "distort"

#define DEFAULT_IMAGE_FN  "distort.rgb"

#define DEFAULT_EFFECT  ripple

#define WIN_SIZE_X  480
#define WIN_SIZE_Y  272

#define GRID_SIZE_X  32
#define GRID_SIZE_Y  32

#define CLIP_NEAR  0.0
#define CLIP_FAR   1000.0

/*
	The following structure defines functions unique to each
	distortion effect.  Depending on which distortion is
	selected, a different set of functions is called.

	Obviously this should all be written in C++.  Maybe.
*/

typedef struct {
  void (* init)();
  void (* dynamics)();
  void (* redraw)();
  void (* click)();
} EFFECT;

extern EFFECT ripple;
extern EFFECT rubber;

#endif
