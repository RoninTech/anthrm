
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



#define MIN(a, b) ((a<b)?(a):(b))

const wchar_t *skins[] = {SKINS};


void fadeArea (TFRAME *frame, const int x1, const int y1, const int x2, const int y2)
{
	int *s = lGetPixelAddress(frame, x1, y1);
	const int pitch = frame->pitch>>2;
	int *ps;
	int x, y;
	
	for (y = y1; y <= y2; y++){
		ps = s;
		for (x = x1; x <= x2; x++, ps++)
			*ps = (*ps&0xFEFEFE)>>1;
		s += pitch;
	}
}

void enableFPS (TVLCPLAYER *vp)
{
	vp->gui.drawFPS = 1;
}

void disableFPS (TVLCPLAYER *vp)
{
	vp->gui.drawFPS = 0;
}

wchar_t *buildSkinD (TVLCPLAYER *vp, wchar_t *buffer, wchar_t *path)
{
	*buffer = 0;
	swprintf(buffer, L"%s/%s/%s", SKINDROOT, skins[vp->gui.skin], path);
	return buffer;
}

void *getPagePtr (TVLCPLAYER *vp, int page_id)
{
	TPAGE *page = vp->pages.page;
	for (int i = 0; i < PAGE_TOTAL; page++, i++){
		if (page->pageId == page_id)
			return page->ptr;
	}
	return NULL;
}

void imageBestFit (const int bg_w, const int bg_h, int fg_w, int fg_h, int *w, int *h)
{
	const int fg_sar_num = 1; const int fg_sar_den = 1;
	const int bg_sar_den = 1; const int bg_sar_num = 1;

	if (fg_w < 1 || fg_w > 4095) fg_w = bg_w;
	if (fg_h < 1 || fg_h > 4095) fg_h = bg_h;
	*w = bg_w;
	*h = (bg_w * fg_h * fg_sar_den * bg_sar_num) / (float)(fg_w * fg_sar_num * bg_sar_den);
	if (*h > bg_h){
		*w = (bg_h * fg_w * fg_sar_num * bg_sar_den) / (float)(fg_h * fg_sar_den * bg_sar_num);
		*h = bg_h;
	}
}

char *stristr (const char *string, const char *pattern)
{
	if (!string) return NULL;
	
	char *pptr, *sptr, *start;
	for (start = (char *)string; *start != 0; start++){
		for ( ; ((*start != 0) && (toupper(*start) != toupper(*pattern))); start++)
			;
		if (0 == *start)
			return NULL;

		pptr = (char *)pattern;
		sptr = (char *)start;

		while (toupper(*sptr) == toupper(*pptr)){
			sptr++;
			pptr++;
			if (0 == *pptr)
				return (start);
		}
	}
	return NULL;
}

wchar_t *wcsistr (const wchar_t *String, const wchar_t *Pattern)
{
	if (!String) return NULL;
	
	wchar_t *pptr, *sptr, *start;
	for (start = (wchar_t *)String; *start != 0; start++){
		for ( ; ((*start != 0) && (towupper(*start) != towupper(*Pattern))); start++)
			;
		if (0 == *start)
			return NULL;

		pptr = (wchar_t *)Pattern;
		sptr = (wchar_t *)start;

		while (towupper(*sptr) == towupper(*pptr)){
			sptr++;
			pptr++;
			if (0 == *pptr)
				return (start);
		}
	}
	return NULL;
}

int UTF8ToUTF16 (const char *in, const size_t ilen, wchar_t *out, size_t olen)
{
	LPWSTR abuf = NULL;
	int len = MultiByteToWideChar(CP_UTF8, 0, in, ilen, NULL, 0);
	if (len > 0)
		abuf = out;
	else
		return 0;

	int ret = MultiByteToWideChar(CP_UTF8, 0, in, ilen, abuf, sizeof(wchar_t)*(len+1));
	if (ret > 0){
		out[ret] = 0;
	}	
	return ret;
}

int UTF16ToUTF8 (const wchar_t *in, const size_t ilen, char *out, size_t olen)
{
	LPSTR abuf = NULL;
	int len = WideCharToMultiByte(CP_UTF8, 0, in, ilen, NULL, 0,  0, 0);
	if (len > 0)
		abuf = out;
	else
		return 0;

	int ret = WideCharToMultiByte(CP_UTF8, 0, in, ilen, abuf, len,  0, 0);
	if (ret > 0){
		out[ret] = 0;
	}	

	return (ret > 0);
}


static wchar_t *UTF8ToUTF16Alloc (const char *in, const size_t ilen)
{
	const size_t olen = sizeof(wchar_t) * (ilen+1);
	wchar_t *out = my_malloc(olen);
	if (out){
		if (UTF8ToUTF16(in, ilen, out, olen)){
			//out = my_realloc(out, (wcslen(out)+1) * sizeof(wchar_t));
		}else{
			my_free(out);
			return NULL;
		}
	}
	return out;
}

static char *UTF16ToUTF8Alloc (const wchar_t *in, const size_t ilen)
{
	const size_t olen = 8 * (ilen+1);
	char *out = my_malloc(olen);
	if (out){
		if (UTF16ToUTF8(in, ilen, out, olen)){
			//out = my_realloc(out, strlen(out)+1);
		}else{
			my_free(out);
			return NULL;
		}
	}
	return out;
}

wchar_t *converttow (const char *utf8)
{
	wchar_t *out = NULL;
	if (utf8 && *utf8)
		out = UTF8ToUTF16Alloc(utf8, strlen(utf8));
	if (!out){
		out = my_wcsdup(L"");
		if (!out){
			printf("no memory, bailing\n");
			abort();
		}
	}
	return out;
}

char *convertto8 (const wchar_t *wide)
{
	char *out = NULL;
	if (wide && *wide)
		out = UTF16ToUTF8Alloc(wide, wcslen(wide));
	if (!out){
		out = my_strdup("");
		if (!out){
			printf("no memory, bailing\n");
			abort();
		}
	}
	return out;
}

void copyImage (TFRAME *src, TFRAME *des, const int sw, const int sh, int dx, int dy, const int dw, const int dh)
{
	#define getPixelAddr32(f,x,y)	(f->pixels+(((y)*(const int)f->pitch)+((x)<<2)))

	const float scalex = (float)dw / (float)sw;
	const float scaley = (float)dh / (float)sh;
	int * restrict sp, * restrict dp;
	int x,y2,y,i;

	int xlookup[dw];
	for (i = 0; i < dw; i++)
		xlookup[i] = i/scalex;
		
	for (y = dy; y < dy+dh; y++){
		y2 = (y-dy)/scaley;
		sp = (int*)getPixelAddr32(src, 0, y2);
		dp = (int*)getPixelAddr32(des, dx, y);
		
		for (x = 0; x < dw; x++)
			dp[x] = /*0xFF000000 |*/sp[xlookup[x]];
	}
}

void printSingleLine (TVLCPLAYER *vp, TFRAME *frame, const wchar_t *text, int font, int x, int y, int foreColour)
{
	lSetForegroundColour(frame->hw, foreColour);
	shadowTextEnable(vp->hw, 0x000000, 180);
	TFRAME *s = lNewString(frame->hw, frame->bpp, PF_DONTFORMATBUFFER, font, (char*)text);
	shadowTextDisable(vp->hw);
	
	if (s){
		if (s->width >= frame->width-x){
			lCopyAreaEx(s, frame, x, y, s->width-(frame->width-x), 0, s->width-1, s->height-1, 1, 1, LCASS_CPY);
		}else{
			x = (((frame->width-x) - s->width)/2)+x;
			lCopyAreaEx(s, frame, x, y, 0, 0, s->width-1, s->height-1, 1, 1, LCASS_CPY);
		}
		lDeleteFrame(s);
	}
	lSetBackgroundColour(frame->hw, 0);
}

// frame copy with bound vertical clipping
void fastFrameCopy (const TFRAME *src, TFRAME *des, int dx, int dy)
{
	const int spitch = src->pitch;
	const int dpitch = des->pitch;
	void * restrict psrc, * restrict pdes;
	int dys = 0;
	
	if (dy < 0){
		dys = abs(dy);
		if (dys >= src->height) return;
		psrc = lGetPixelAddress(src, 0, dys);
		dy = 0;
	}else{
		psrc = lGetPixelAddress(src, 0, 0);
	}
	
	if (dy > des->height-1)
		dy = des->height-1;
	pdes = lGetPixelAddress(des, dx, dy);

	int r = src->height - dys;
	if (dy + r > des->height-1){
		r = (des->height - dy) - 1;
		if (r > 1) r++;
	}

	while(r-- > 0){
		my_memcpy(pdes, psrc, spitch);
		psrc += spitch;
		pdes += dpitch;
	}
}

void outlineTextEnable (THWD *hw, const int colour)
{
	lSetFilterAttribute(hw, LTR_OUTLINE2, 0, colour);
	lSetRenderEffect(hw, LTR_OUTLINE2);
}

void outlineTextDisable (THWD *hw)
{
	lSetRenderEffect(hw, LTR_DEFAULT);
}

void shadowTextEnable (THWD *hw, const int colour, const unsigned char trans)
{
	lSetRenderEffect(hw, LTR_SHADOW);
	// set direction to South-East, shadow thickness to 5, offset by 1 pixel(s) and transparency to trans
	lSetFilterAttribute(hw, LTR_SHADOW, 0, LTRA_SHADOW_S|LTRA_SHADOW_E | LTRA_SHADOW_S5 | LTRA_SHADOW_OS(1) | LTRA_SHADOW_TR(trans));
	lSetFilterAttribute(hw, LTR_SHADOW, 1, colour);
}

void shadowTextDisable (THWD *hw)
{
	lSetRenderEffect(hw, LTR_DEFAULT);
}

void printSingleLineShadow (TFRAME *frame, const int font, const int x, const int y, const int colour, const char *str)
{
	TLPRINTR rt;
	int w, h;
	
	lSetForegroundColour(frame->hw, colour);
	lGetTextMetrics(frame->hw, str, 0, font, &w, &h);
	shadowTextEnable(frame->hw, 0x000000, 120);
	
	if (w >= frame->width){
		rt.bx1 = 0; rt.by1 = y;
		rt.bx2 = frame->width-1;
		rt.by2 = frame->height-1;
		rt.sx = rt.bx1; rt.sy = y;
		lPrintEx(frame, &rt, font, PF_CLIPDRAW|PF_RIGHTJUSTIFY, LPRT_CPY, str);
	}else{
		rt.bx1 = (frame->width-w)/2.0;
		rt.by1 = y;
		rt.bx2 = frame->width-1;
		rt.by2 = frame->height-1;
		rt.sx = rt.bx1; rt.sy = y;
		lPrintEx(frame, &rt, font, PF_CLIPWRAP|PF_CLIPDRAW, LPRT_CPY, str);
	}
	shadowTextDisable(frame->hw);
}

void drawInt (TFRAME *frame, const int x, const int y, const int var, const int colour)
{
	int x2 = x;
	if (var < 10)
		x2 += 8;
	else if (var < 100)
		x2 += 15;
	else if (var < 1000)
		x2 += 22;
	else if (var < 10000)
		x2 += 29;
	else
		x2 += 36;
		
	lDrawRectangleFilled(frame, x-3, y+2, x2, y+14, 90<<24 | (colour&0xFFFFFF));
	lPrintf(frame, x, y, BFONT, LPRT_CPY, "%i", var);
}

void drawImage (TFRAME *src, TFRAME *des, const int x, const int y, const int x2, const int y2)
{
	lCopyAreaEx(src, des, x, y, 0, 0, x2, y2, 1, 1, LCASS_CPY);
}

void drawImageScaled (TFRAME *src, TFRAME *des, const int x, const int y, const float s)
{
	lCopyAreaScaled(src, des, 0, 0, src->width, src->height, x, y, src->width*s, src->height*s, LCASS_CPY);
}

void huhtanenBlur (TFRAME *des, const int c1, const int r1, const int c2, const int r2, const int iterations)
{
	int rgba[4];
	unsigned char *p;
	const int bpl = des->pitch;

	for (int loop = 0; loop < iterations; loop++){
		for (int col = c1; col < c2; col++){
			p = lGetPixelAddress(des, col, r1);
			for (int i = 0; i < 3; i++)
				rgba[i] = p[i] << 4;

			for (int j = r1; j < r2; j++){
				p += bpl;
				for (int i = 0; i < 3; i++)
					p[i] = (rgba[i] += (((p[i] << 4) - rgba[i])) >> 1) >> 4;
			}
		}
		for (int row = r1; row <= r2; row++){
			p = lGetPixelAddress(des, c1, row);
			for (int i = 0; i < 3; i++)
				rgba[i] = p[i] << 4;

			for (int j = c1; j < c2; j++){
				p += 4;
				for (int i = 0; i < 3; i++)
					p[i] = (rgba[i] += (((p[i] << 4) - rgba[i])) >> 1) >> 4;
			}
		}
		for (int col = c1; col <= c2; col++){
			p = lGetPixelAddress(des, col, r2);
			for (int i = 0; i < 3; i++)
				rgba[i] = p[i] << 4;

			for (int j = r1; j < r2; j++){
				p -= bpl;
				for (int i = 0; i < 3; i++)
					p[i] = (rgba[i] += (((p[i] << 4) - rgba[i])) >> 1) >> 4;
			}
		}
		for (int row = r1; row <= r2; row++){
			p = lGetPixelAddress(des, c2, row);
			for (int i = 0; i < 3; i++)
				rgba[i] = p[i] << 4;

			for (int j = c1; j < c2; j++){
				p -= 4;
				for (int i = 0; i < 3; i++)
					p[i] = (rgba[i] += (((p[i] << 4) - rgba[i])) >> 1) >> 4;
			}
		}
	}
}

void blurImage (TVLCPLAYER *vp, TFRAME *src, const int iterations)
{
	huhtanenBlur(src, 0, 0, src->width-1, src->height-1, iterations);
}

void blurImageAreaWithFade (TFRAME *frame, const int x1, const int y1, const int x2, const int y2, const int iterations)
{
	if (x2 <= 0 || y2 <= 0) return;
	huhtanenBlur(frame, x1, y1, x2, y2, iterations);
	fadeArea(frame, x1, y1, x2, y2);
}

static inline int countColonsW (wchar_t *str)
{
	int i = 0;
	while (*str) i += (*str++ == L':');
	return i;
}

static inline int countColons (char *str)
{
	int i = 0;
	while (*str) i += (*str++ == ':');
	return i;
}

int timeToString (const libvlc_time_t t, char *buffer, const size_t bufferLen)
{
	*buffer = 0;
	if (!(int)t) return 0;
	
	const int seconds = (int)t%60;
	const int minutes = (int)((int)t/60.0)%60;
	const int hours = (int)((int)t/60.0/60.0);
	
	if (hours)
		return snprintf(buffer, bufferLen, "%i:%02i:%02i", hours, minutes, seconds);
	else if (minutes)
		return snprintf(buffer, bufferLen, "%i:%02i", minutes, seconds);
	else
		return snprintf(buffer, bufferLen, "0:%.2i", seconds);
}

libvlc_time_t stringToTimeW (wchar_t *sztime, size_t len)
{
	int h = 0; int m = 0; int s = 0;
	
	const int colons = countColonsW(sztime);
	if (colons > 1)
		swscanf(sztime, L"%d:%d:%d", &h, &m, &s);
	else if (colons == 1)
		swscanf(sztime, L"%d:%d", &m, &s);
	else
		swscanf(sztime, L"%d", &s);
	return (h * 60 * 60) + (m * 60) + s;
}

libvlc_time_t stringToTime (char *sztime, size_t len)
{
	int h = 0; int m = 0; int s = 0;
	
	const int colons = countColons(sztime);
	if (colons > 1)
		sscanf(sztime, "%d:%d:%d", &h, &m, &s);
	else if (colons == 1)
		sscanf(sztime, "%d:%d", &m, &s);
	else
		sscanf(sztime, "%d", &s);
	return (h * 60 * 60) + (m * 60) + s;
}

#if 0
/* http://en.wikipedia.org/wiki/Adler-32 */
static uint32_t getAddler (uint8_t *data, size_t len)
{
	#define MOD_ADLER 65521

    uint32_t a = 1, b = 0;
 
    while(len){
        size_t tlen = len > 5550 ? 5550 : len;
        len -= tlen;
        
        do{
            a += *data++;
            b += a;
        }while(--tlen);
        
        a = (a&0xffff) + (a>>16) * (65536-MOD_ADLER);
        b = (b&0xffff) + (b>>16) * (65536-MOD_ADLER);
    }
 
    /* It can be shown that a <= 0x1013a here, so a single subtract will do. */
    if (a >= MOD_ADLER)
		a -= MOD_ADLER;
 
    /* It can be shown that b can reach 0xffef1 here. */
    b = (b&0xffff) + (b>>16) * (65536-MOD_ADLER);
    if (b >= MOD_ADLER) b -= MOD_ADLER;
    return (b<<16) | a;
}

#else

// 32 bit magic FNV-1a prime
// Fowler/Noll/Vo hash
#define FNV_32_PRIME ((unsigned int)0x01000193)
#define FNV1_32_INIT ((unsigned int)0x811c9dc5)

static unsigned int fnv_32a_buf (const void *buf, const size_t len, unsigned int hval)
{
    unsigned char *bp = (unsigned char *)buf;	/* start of buffer */
    unsigned char *be = bp + len;		/* beyond end of buffer */

    /*
     * FNV-1a hash each octet in the buffer
     */
    while (bp < be) {

	/* xor the bottom with the current octet */
	hval ^= (unsigned int)*bp++;

	/* multiply by the 32 bit FNV magic prime mod 2^32 */
#if defined(NO_FNV_GCC_OPTIMIZATION)
	hval *= FNV_32_PRIME;
#else
	hval += (hval<<1) + (hval<<4) + (hval<<7) + (hval<<8) + (hval<<24);
#endif
    }

    /* return our new hash value */
    return hval;
}
#endif

unsigned int generateHash (const void *data, const size_t dlen)
{
#if 0
	return getAddler((uint8_t*)data, dlen);
#else
	return fnv_32a_buf(data, dlen, FNV_32_PRIME);
#endif
}

unsigned int getHashW (const wchar_t *path)
{
	unsigned int hash = 0;
	char *out = UTF16ToUTF8Alloc(path, wcslen(path));
	if (out){
		hash = generateHash(out, strlen(out));
		my_free(out);
	}
	return hash;
}

unsigned int getHash (const char *path)
{
	return generateHash(path, strlen(path));
}

void *decodeURI (const char *arturl, const int len)
{
	static int hextodec[256];
     			
	if (!hextodec['A']){
    	int ct = 0;
		for (int i = '0'; i <= '9'; i++)
			hextodec[i] = ct++;
		ct = 10;
		for (int i = 'A'; i <= 'F'; i++){
			hextodec[i] = ct;
			hextodec[i+32] = ct++;
		}
	}
	if (len  < 10) return NULL;
	
	unsigned char *tmp = my_malloc(len+1);
	if (tmp){
		int c = 0;
    	for (int i = 8; i < len; i++){
    		if (arturl[i] == '%' && i < len-2){
    			tmp[c++] = hextodec[(int)arturl[i+1]]<<4|hextodec[(int)arturl[i+2]];
     			i += 2;
	     		continue;
			}
	    	tmp[c++] = arturl[i];
    	}     			
    	tmp[c] = 0;	
    }
    return tmp;
}


int encodeURI (const char *in, char *out, const size_t len)
{
	static const char hex[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
	
	int i = 0;
	while(i < len){
		if (*in&0x80){
			*out++ = '%';
			*out++ = hex[(in[0]&0xF0)>>4];
			*out++ = hex[ in[0]&0x0F];
			in++;
		}else{
			*out++ = *in++;
		}
		i++;
	}
	*out = '\0';
	return i;
}

int doesFileExistW (const wchar_t *path)
{
	FILE *fp = _wfopen(path, L"r");
	if (fp) fclose(fp);
	return (fp != NULL);
}

int doesFileExistUtf8 (const char *utf8path)
{
	if (utf8path){
		wchar_t *path = UTF8ToUTF16Alloc (utf8path, strlen(utf8path));
		if (path){
			int ret = doesFileExistW(path);
			my_free(path);
			return ret;
		}
	}
	return 0;
}

int isDirectoryW (wchar_t *path)
{
	return (PathIsDirectoryW(path) != 0);
}

int isDirectory (char *path)
{
	int ret = 0;
	
	wchar_t *out = UTF8ToUTF16Alloc(path, strlen(path));
	if (out){
		ret = isDirectoryW(out);
		my_free(out);
	}
	return ret;
}

int isPlaylistW (wchar_t *path)
{
	static unsigned int hash;
	static int extlen;
	
	if (!hash){
		hash = getHashW(VLCSPLAYLISTEXTW);
		extlen = wcslen(VLCSPLAYLISTEXTW);
	}

	if (path){
		const int len = wcslen(path);
		if (len > extlen)
			return (getHashW(&path[(len-extlen)]) == hash);
	}
	return 0;
}

int isPlaylist (char *path)
{
	static unsigned int hash;
	static int extlen;
	
	if (!hash){
		hash = getHash(VLCSPLAYLISTEXT);
		extlen = strlen(VLCSPLAYLISTEXT);
	}

	if (path){
		const int len = strlen(path);
		if (len > extlen)
			return (getHash(&path[(len-extlen)]) == hash);
	}
	return 0;
}

wchar_t *removeLeadingSpacesW (wchar_t *var)
{
	int i = wcsspn(var, L" ");
	if (i) var += i;
	return var;
}

wchar_t *removeTrailingSpacesW (wchar_t *var)
{
	wchar_t *eos = var + wcslen(var) - 1;
	while (eos >= var && *eos == L' ')
		*eos-- = L'\0';
	return var;
}

char *removeLeadingSpaces (char *var)
{
	int i = strspn(var, " ");
	if (i) var += i;
	return var;
}

char *removeTrailingSpaces (char *var)
{
	char *eos = var + strlen(var) - 1;
	while (eos >= var && *eos == ' ')
		*eos-- = '\0';
	return var;
}

static void addConsoleline (TVLCPLAYER *vp, const char *str)
{
	marqueeAdd(vp, vp->gui.marquee, str, getTime(vp)+(15*1000));
}

static void addConsolelineW (TVLCPLAYER *vp, const wchar_t *str)
{
	char *txt = UTF16ToUTF8Alloc(str, wcslen(str));
	if (txt){
		addConsoleline(vp, txt);
		my_free(txt);
	}
}

int vaswprintf (wchar_t **result, const wchar_t *format, va_list *args)
{
	
  const wchar_t *p = format;
  /* Add one to make sure that it is never zero, which might cause malloc
     to return NULL.  */
  int total_width = wcslen(format) + sizeof(wchar_t);
  va_list ap;

  my_memcpy(&ap, args, sizeof(va_list));

  while (*p != L'\0')
    {
      if (*p++ == L'%')
        {
          while (wcschr(L"-+ #0", *p))
            ++p;
          if (*p == L'*')
            {
              ++p;
              total_width += abs(va_arg(ap, int));
            }
          else
            total_width += wcstoul(p, (wchar_t**)&p, 10);
          if (*p == L'.')
            {
              ++p;
              if (*p == L'*')
                {
                  ++p;
                  total_width += abs(va_arg(ap, int));
                }
              else
                total_width += wcstoul(p, (wchar_t**)&p, 10);
            }
          while(wcschr(L"hlL", *p))
            ++p;
          /* Should be big enough for any format specifier except %s.  */
          total_width += 30;
          switch (*p)
            {
            case L'd':
            case L'i':
            case L'o':
            case L'u':
            case L'x':
            case L'X':
            case L'c':
              (void) va_arg(ap, int);
              break;
            case L'f':
            case L'e':
            case L'E':
            case L'g':
            case L'G':
              (void) va_arg(ap, double);
              break;
            case L's':
              total_width += wcslen(va_arg(ap, wchar_t *));
              break;
            case L'p':
            case L'n':
              (void) va_arg(ap, wchar_t *);
              break;
            }
        }
    }

  if (!total_width) return 0;
  *result = my_calloc(sizeof(wchar_t), 1+total_width);
  
  if (*result != NULL)
    return vsnwprintf(*result, total_width, format, *args);
  else
    return 0;
}

int vasprintf (char **result, const char *format, va_list *args)
{
	
  const char *p = format;
  /* Add one to make sure that it is never zero, which might cause malloc
     to return NULL.  */
  int total_width = strlen(format) + sizeof(char);
  va_list ap;

  my_memcpy(&ap, args, sizeof(va_list));

  while (*p != '\0')
    {
      if (*p++ == '%')
        {
          while (strchr("-+ #0", *p))
            ++p;
          if (*p == '*')
            {
              ++p;
              total_width += abs(va_arg(ap, int));
            }
          else
            total_width += strtoul(p, (char**)&p, 10);
          if (*p == '.')
            {
              ++p;
              if (*p == '*')
                {
                  ++p;
                  total_width += abs(va_arg(ap, int));
                }
              else
                total_width += strtoul(p, (char**)&p, 10);
            }
          while(strchr("hl", *p))
            ++p;
          /* Should be big enough for any format specifier except %s.  */
          total_width += 30;
          switch (*p)
            {
            case 'd':
            case 'i':
            case 'o':
            case 'u':
            case 'x':
            case 'X':
            case 'c':
              (void) va_arg(ap, int);
              break;
            case 'f':
            case 'e':
            case 'E':
            case 'g':
            case 'G':
              (void) va_arg(ap, double);
              break;
            case 's':
              total_width += strlen(va_arg(ap, char *));
              break;
            case 'p':
            case 'n':
              (void) va_arg(ap, char *);
              break;
            }
        }
    }

  if (!total_width) return 0;
  *result = my_calloc(sizeof(char), 1+total_width);
  
  if (*result != NULL)
    return vsnprintf(*result, total_width, format, *args);
  else
    return 0;
}

void dbprintf (TVLCPLAYER *vp, const char *str, ...)
{
	char *buffer = NULL;
	
	VA_OPEN(ap, str);
	vasprintf(&buffer, str, &ap);
	VA_CLOSE(ap);
	
	if (buffer){
		addConsoleline(vp, buffer);
		my_free(buffer);
	}
}

void dbwprintf (TVLCPLAYER *vp, const wchar_t *str, ...)
{
	wchar_t *buffer = NULL;
	
	VA_OPEN(ap, str);
	vaswprintf(&buffer, str, &ap);
	VA_CLOSE(ap);
	
	if (buffer){
		addConsolelineW(vp, buffer);
		my_free(buffer);
	}
}

// without this drag'n'drop will screw with the path
void setCurrentDirectory (const wchar_t *indir)
{
	wchar_t drive[_MAX_DRIVE+1];
	wchar_t dir[_MAX_DIR+1];
	wchar_t path[MAX_PATH+1];
	*drive = 0;
	*dir = 0;
	
	_wsplitpath(indir, drive, dir, NULL, NULL);
	swprintf(path, L"%s%s", drive, dir);
	SetCurrentDirectoryW(path);
}
