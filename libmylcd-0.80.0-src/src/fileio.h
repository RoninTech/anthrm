
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


#ifndef _FILEIO_H_
#define _FILEIO_H_


int initFileIO (THWD *hw);
void closeFileIO (THWD *hw);


INLINE FILE * l_wfopen (const wchar_t * filename, const wchar_t * mode); 
INLINE FILE * l_fopen (const ubyte * filename, const char * mode); 
INLINE int l_fclose (FILE * stream); 
INLINE uint64_t l_fread (void * buffer, size_t size, size_t count, FILE * stream);
INLINE size_t l_fwrite (const void * buffer, size_t size, size_t count, FILE * stream);
INLINE int l_fseek( FILE *stream, long offset, int whence);
INLINE uint64_t l_ftell( FILE *stream);
INLINE void l_rewind( FILE *stream);
INLINE uint64_t l_fgetpos( FILE *stream, fpos_t *pos);
INLINE int l_fsetpos( FILE *stream, fpos_t *pos);
INLINE int l_fflush(FILE *stream);

INLINE uint64_t l_lof (FILE *stream);
INLINE uint64_t getFileLength (const ubyte *path);
INLINE uint64_t getFileLengthw (const wchar_t *path);


#endif

