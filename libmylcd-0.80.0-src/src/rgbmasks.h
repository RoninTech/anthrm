
// libmylcd
// An LCD framebuffer library
// Michael McElligott
// okio@users.sourceforge.net

//  Copyright (c) 2005-2009  Michael McElligott
// 
//  This program is free software you can redistribute it and/or
//  modify it under the terms of the GNU LIBRARY GENERAL PUBLIC LICENSE
//  as published by the Free Software Foundation either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU LIBRARY GENERAL PUBLIC LICENSE for more details.


#ifndef _RGBMASKS_H_
#define _RGBMASKS_H_

// LFRM_BPP_32a - ARGB 8888
#define RGB_32A_ALPHA	0xFF000000
#define RGB_32A_RED		0x00FF0000 | RGB_32A_ALPHA
#define RGB_32A_GREEN	0x0000FF00 | RGB_32A_ALPHA
#define RGB_32A_BLUE	0x000000FF | RGB_32A_ALPHA
#define RGB_32A_WHITE	(RGB_32_RED|RGB_32_GREEN|RGB_32_BLUE|RGB_32A_ALPHA)
#define RGB_32A_BLACK	0xFF000000
#define RGB_32A_MAGENTA	(RGB_32_RED|RGB_32_BLUE|RGB_32A_ALPHA)
#define RGB_32A_YELLOW	(RGB_32_RED|RGB_32_GREEN|RGB_32A_ALPHA)
#define RGB_32A_CYAN	(RGB_32_GREEN|RGB_32_BLUE|RGB_32A_ALPHA)

// LFRM_BPP_32 - RGB 888
#define RGB_32_RED		0x00FF0000
#define RGB_32_GREEN	0x0000FF00
#define RGB_32_BLUE		0x000000FF	
#define RGB_32_WHITE	(RGB_32_RED|RGB_32_GREEN|RGB_32_BLUE)
#define RGB_32_BLACK	0x00000000
#define RGB_32_MAGENTA	(RGB_32_RED|RGB_32_BLUE)
#define RGB_32_YELLOW	(RGB_32_RED|RGB_32_GREEN)
#define RGB_32_CYAN		(RGB_32_GREEN|RGB_32_BLUE)

// LFRM_BPP_24 - RGB 888
#define RGB_24_RED		0xFF0000
#define RGB_24_GREEN	0x00FF00
#define RGB_24_BLUE		0x0000FF	
#define RGB_24_WHITE	(RGB_24_RED|RGB_24_GREEN|RGB_24_BLUE)
#define RGB_24_BLACK	0x000000
#define RGB_24_MAGENTA	(RGB_24_RED|RGB_24_BLUE)
#define RGB_24_YELLOW	(RGB_24_RED|RGB_24_GREEN)
#define RGB_24_CYAN		(RGB_24_GREEN|RGB_24_BLUE)

// LFRM_BPP_16 - RGB 565
#define RGB_16_RED		0xF800
#define RGB_16_GREEN	0x07E0
#define RGB_16_BLUE		0x001F	
#define RGB_16_WHITE	(RGB_16_RED|RGB_16_GREEN|RGB_16_BLUE)
#define RGB_16_BLACK	0x0000
#define RGB_16_MAGENTA	(RGB_16_RED|RGB_16_BLUE)
#define RGB_16_YELLOW	(RGB_16_RED|RGB_16_GREEN)
#define RGB_16_CYAN		(RGB_16_GREEN|RGB_16_BLUE)

// LFRM_BPP_15 - RGB 555
#define RGB_15_RED		0x7C00
#define RGB_15_GREEN	0x03E0
#define RGB_15_BLUE		0x001F	
#define RGB_15_WHITE	(RGB_15_RED|RGB_15_GREEN|RGB_15_BLUE)
#define RGB_15_BLACK	0x0000
#define RGB_15_MAGENTA	(RGB_15_RED|RGB_15_BLUE)
#define RGB_15_YELLOW	(RGB_15_RED|RGB_15_GREEN)
#define RGB_15_CYAN		(RGB_15_GREEN|RGB_15_BLUE)

// LFRM_BPP_12 - RGB 444
#define RGB_12_RED		0xF00
#define RGB_12_GREEN	0x0F0
#define RGB_12_BLUE		0x00F
#define RGB_12_WHITE	(RGB_12_RED|RGB_12_GREEN|RGB_12_BLUE)
#define RGB_12_BLACK	0x000
#define RGB_12_MAGENTA	(RGB_12_RED|RGB_12_BLUE)
#define RGB_12_YELLOW	(RGB_12_RED|RGB_12_GREEN)
#define RGB_12_CYAN		(RGB_12_GREEN|RGB_12_BLUE)

// LFRM_BPP_8 - RGB 332
#define RGB_8_RED		0xE0
#define RGB_8_GREEN		0x1C
#define RGB_8_BLUE		0x03
#define RGB_8_WHITE		(RGB_8_RED|RGB_8_GREEN|RGB_8_BLUE)
#define RGB_8_BLACK		0x00
#define RGB_8_MAGENTA	(RGB_8_RED|RGB_8_BLUE)
#define RGB_8_YELLOW	(RGB_8_RED|RGB_8_GREEN)
#define RGB_8_CYAN		(RGB_8_GREEN|RGB_8_BLUE)

#endif
