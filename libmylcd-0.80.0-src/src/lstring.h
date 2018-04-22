
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


#ifndef _LSTRING_H_
#define _LSTRING_H_

#include <string.h>


// needed for mingw gcc 4.4.0 and mingw-64 (please fix me)
#ifndef _WIN64
#ifndef _strnicmp
_CRTIMP int __cdecl __MINGW_NOTHROW	_strnicmp (const char*, const char*, size_t);
#endif 
#ifndef _stricmp
_CRTIMP int __cdecl __MINGW_NOTHROW	_stricmp (const char*, const char*);
#endif
#endif

#ifdef __DEBUG__
#define funcname __func__
#else
#define funcname ""
#endif

#define l_strdup(a) my_strdup(a, funcname)
#define l_wcsdup(a) my_wcsdup(a, funcname)



//INLINE size_t l_wcslen (const wchar_t *s);
//INLINE wchar_t *l_wcsncpy (wchar_t *dest, const wchar_t *src, size_t n);
//INLINE wchar_t *l_wcscpy (wchar_t *dest, const wchar_t *src);

//INLINE char *l_strcpy (char *dest, const char *src);
//INLINE char *l_strncpy (char *dest, const char *src, size_t n);


#define l_wcslen wcslen
#define l_wcscpy wcscpy
#define l_wcsncpy wcsncpy
#define l_wcstoul wcstoul
#define l_wcschr wcschr
#define l_wcsncmp wcsncmp
#define l_vsnwprintf vsnwprintf

#define l_strlen strlen
#define l_strncmp strncmp
#define l_strcmp strcmp
#define l_strchr strchr
#define l_strtoul strtoul
#define l_strcpy strcpy
#define l_strncpy strncpy
#define l_strcasecmp _stricmp
#define l_strncasecmp _strncasecmp
#define l_strtok strtok
#define l_strstr strstr
#define l_vsnprintf vsnprintf

#define l_fgets(a,b,c) fgets((a),(b),((FILE*)(c)))
#define l_atoi atoi

#define l_strtolower(_dest,_src,_srclen) \
{\
	char *dest = (char * restrict)_dest;\
	const char *src = (const char * restrict)_src;\
	int srclen = (int)_srclen;\
	while(srclen--) *dest++ = tolower(*src++);\
}


//INLINE char *l_strstr (const char *s1, const char *s2);
//INLINE void l_strtolower (char *dest, const char *src, unsigned int srclen);
INLINE char *l_stristr (const char *in, const char *find);
//INLINE char *l_strtok (char *str1, const char *str2);
//INLINE char *l_strchr (const char *s, int c);
//INLINE wchar_t *l_wcschr (const wchar_t *s, int c);
//INLINE size_t l_strlen (const char *s);

//INLINE int l_strncmp (const char *s1, const char *s2, size_t n);
//INLINE int l_wcsncmp (const wchar_t *s1, const wchar_t *s2, size_t n);

//INLINE int l_strcmp (const char *s1, const char *s2);
//INLINE int l_strncasecmp (const char *s1, const char *s2, size_t n);
//INLINE int l_strcasecmp (const char *s1, const char *s2);

//INLINE int l_vsnprintf(char *buffer, const int count, const char *format, va_list argptr);
INLINE int l_vasprintf (char **result, const char *format, va_list *zargs);
//INLINE int l_vsnwprintf (wchar_t *buffer, const int count, const wchar_t *format, va_list argptr);
INLINE int l_vaswprintf (wchar_t **result, const wchar_t *format, va_list *args);

//INLINE int l_atoi (const char *s);
INLINE int l_atoi2 (const char *str, int *value);	// returns total chars/bytes read
INLINE int l_wcatoi2 (const wchar_t *str, int *value);
//INLINE unsigned long l_strtoul (const char *a, char **b, int c);
//INLINE unsigned long l_wcstoul (const wchar_t *a, wchar_t **b, int c);

//INLINE char *l_fgets (char *s, int n, const FILE *stream);


#ifndef _WIN64
//#include <ansidecl.h>	/* for VA_OPEN and VA_CLOSE */ 
#endif

#ifndef	_ANSIDECL_H
#undef VA_OPEN
#undef VA_CLOSE
#undef VA_FIXEDARG

#define VA_OPEN(AP, VAR)	{ va_list AP; va_start(AP, VAR); { struct Qdmy
#define VA_CLOSE(AP)		} va_end(AP); }
#define VA_FIXEDARG(AP, T, N)	struct Qdmy
#endif





#endif

