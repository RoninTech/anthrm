
// libmylcd
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
//  GNU LIBRARY GENERAL PUBLIC LICENSE for details.


#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

typedef unsigned char ubyte;


// single wchar to UTF8 code point conversion
static int utf8_wctomb (ubyte *r, unsigned int wc, int n)
{
	int count;
	if (wc < 0x80)
		count = 1;
	else if (wc < 0x800)
		count = 2;
	else if (wc < 0x10000)
		count = 3;
	else if (wc < 0x200000)
		count = 4;
	else if (wc < 0x4000000)
		count = 5;
	else if (wc <= 0x7fffffff)
		count = 6;
	else
		return 0;

	if (n < count)
		return 0;

	switch (count){
	  case 6: r[5] = 0x80 | (wc & 0x3f); wc = wc >> 6; wc |= 0x4000000;
	  case 5: r[4] = 0x80 | (wc & 0x3f); wc = wc >> 6; wc |= 0x200000;
	  case 4: r[3] = 0x80 | (wc & 0x3f); wc = wc >> 6; wc |= 0x10000;
	  case 3: r[2] = 0x80 | (wc & 0x3f); wc = wc >> 6; wc |= 0x800;
	  case 2: r[1] = 0x80 | (wc & 0x3f); wc = wc >> 6; wc |= 0xc0;
	  case 1: r[0] = wc;
  }
  return count;
}

// wchar string to utf8 string
int utf8_wcstombs (ubyte *des, size_t des_n, wchar_t *wc, size_t wc_n)
{
	unsigned int total = 0;
	int i;
	for (i = 0; i < wc_n; i++){
		total = utf8_wctomb(des, (unsigned int)wc[i], des_n);
		des += total;
	}
	*des = 0;
	return i;
}

int UTF8ToUTF32 (const unsigned char *buffer, unsigned int *pwc)
{
	
	unsigned int n;// = l_strlen(buffer);
	for (n = 0; n<7; n++){
		if (!buffer[n])
			break;
	}

	unsigned char *s = (unsigned char *)buffer;
	unsigned char c = s[0];
	const unsigned int invalidChar = '?';
	
	if (c < 0x80){
		*pwc = c;
		return 1;
	}else if (c < 0xc2){
		*pwc = invalidChar;
		return 1;

	}else if (c < 0xe0){
		if (n < 2){
			*pwc = invalidChar;
			return 1;
		}
			
		if (!((s[1] ^ 0x80) < 0x40)){
			*pwc = invalidChar;
			if (s[1]&0x80)
				return 2;
			else
				return 1;
		}
			
		*pwc = ((unsigned int)(c & 0x1f) << 6) | (unsigned int)(s[1] ^ 0x80);
		return 2;
	}else if (c < 0xf0){
		if (n < 3){
			*pwc = invalidChar;
			return 2;
		}
		
		if (!((s[1] ^ 0x80) < 0x40 && (s[2] ^ 0x80) < 0x40 && (c >= 0xe1 || s[1] >= 0xa0))){
			*pwc = invalidChar;
			if (s[1]&0x80){
				if (s[2]&0x80)
					return 3;
				else
					return 2;
			}else{
				return 1;
			}
		}

		*pwc = ((unsigned int)(c & 0x0f) << 12) | ((unsigned int)(s[1] ^ 0x80) << 6) | (unsigned int)(s[2] ^ 0x80);
		return 3;
	}else if (c < 0xf8){
		if (n < 4){
			*pwc = invalidChar;
			return 3;
		}
			
		if (!((s[1] ^ 0x80) < 0x40 && (s[2] ^ 0x80) < 0x40 && (s[3] ^ 0x80) < 0x40 && (c >= 0xf1 || s[1] >= 0x90))){
			*pwc = invalidChar;
			if (s[1]&0x80){
				if (s[2]&0x80){
					if (s[3]&0x80)
						return 4;
					else
						return 3;
				}else{
					return 2;
				}
			}else{
				return 1;
			}
		}
			
		*pwc = ((unsigned int)(c & 0x07) << 18) | ((unsigned int)(s[1] ^ 0x80) << 12) | ((unsigned int)(s[2] ^ 0x80) << 6) | (unsigned int)(s[3] ^ 0x80);
		return 4;
	}else if (c < 0xfc){
		if (n < 5){
			*pwc = invalidChar;
			return 4;
		}
			
		if (!((s[1] ^ 0x80) < 0x40 && (s[2] ^ 0x80) < 0x40 && (s[3] ^ 0x80) < 0x40 && (s[4] ^ 0x80) < 0x40 && (c >= 0xf9 || s[1] >= 0x88))){
			*pwc = invalidChar;
			if (s[1]&0x80){
				if (s[2]&0x80){
					if (s[3]&0x80){
						if (s[4]&0x80){
							return 5;
						}else{
							return 4;
						}
					}else{
						return 3;
					}
				}else{
					return 2;
				}
			}else{
				return 1;
			}
		}
		
		*pwc = ((unsigned int)(c & 0x03) << 24) | ((unsigned int)(s[1] ^ 0x80) << 18) | ((unsigned int)(s[2] ^ 0x80) << 12) | ((unsigned int)(s[3] ^ 0x80) << 6) | (unsigned int)(s[4] ^ 0x80);
		return 5;
	}else if (c < 0xfe){
		if (n < 6){
			*pwc = invalidChar;
			return 5;
		}
		
		if (!((s[1] ^ 0x80) < 0x40 && (s[2] ^ 0x80) < 0x40 && (s[3] ^ 0x80) < 0x40 && (s[4] ^ 0x80) < 0x40 && (s[5] ^ 0x80) < 0x40 && (c >= 0xfd || s[1] >= 0x84))){
			*pwc = invalidChar;
			if (s[1]&0x80){
				if (s[2]&0x80){
					if (s[3]&0x80){
						if (s[4]&0x80){
							if (s[5]&0x80){
								return 6;
							}else{
								return 5;
							}
						}else{
							return 4;
						}
					}else{
						return 3;
					}
				}else{
					return 2;
				}
			}else{
				return 1;
			}
		}
		
		*pwc = ((unsigned int)(c & 0x01) << 30) | ((unsigned int)(s[1] ^ 0x80) << 24) | ((unsigned int)(s[2] ^ 0x80) << 18) | ((unsigned int)(s[3] ^ 0x80) << 12) | ((unsigned int)(s[4] ^ 0x80) << 6) | (unsigned int)(s[5] ^ 0x80);
		return 6;
	}else{
		*pwc = invalidChar;
		return 1;
	}
}

int _UTF8ToUTF32 (const unsigned char *buffer, unsigned int *pwc)
{
	
	unsigned int n;// = l_strlen(buffer);
	for (n = 0; n<7; n++){
		if (!buffer[n])
			break;
	}

	unsigned char *s = (unsigned char *)buffer;
	unsigned char c = s[0];
	const unsigned int invalidChar = '?';
	
	if (c < 0x80){
		*pwc = c;
		return 1;
	}else if (c < 0xc2){
		*pwc = invalidChar;
		return -1;

	}else if (c < 0xe0){
		if (n < 2){
			*pwc = invalidChar;
			return -1;
		}
			
		if (!((s[1] ^ 0x80) < 0x40)){
			*pwc = invalidChar;
			if (s[1]&0x80)
				return -2;
			else
				return -1;
		}
			
		*pwc = ((unsigned int)(c & 0x1f) << 6) | (unsigned int)(s[1] ^ 0x80);
		return 2;
	}else if (c < 0xf0){
		if (n < 3){
			*pwc = invalidChar;
			return -2;
		}
		
		if (!((s[1] ^ 0x80) < 0x40 && (s[2] ^ 0x80) < 0x40 && (c >= 0xe1 || s[1] >= 0xa0))){
			*pwc = invalidChar;
			if (s[1]&0x80){
				if (s[2]&0x80)
					return -3;
				else
					return -2;
			}else{
				return -1;
			}
		}

		*pwc = ((unsigned int)(c & 0x0f) << 12) | ((unsigned int)(s[1] ^ 0x80) << 6) | (unsigned int)(s[2] ^ 0x80);
		return 3;
	}else if (c < 0xf8){
		if (n < 4){
			*pwc = invalidChar;
			return -3;
		}
			
		if (!((s[1] ^ 0x80) < 0x40 && (s[2] ^ 0x80) < 0x40 && (s[3] ^ 0x80) < 0x40 && (c >= 0xf1 || s[1] >= 0x90))){
			*pwc = invalidChar;
			if (s[1]&0x80){
				if (s[2]&0x80){
					if (s[3]&0x80)
						return -4;
					else
						return -3;
				}else{
					return -2;
				}
			}else{
				return -1;
			}
		}
			
		*pwc = ((unsigned int)(c & 0x07) << 18) | ((unsigned int)(s[1] ^ 0x80) << 12) | ((unsigned int)(s[2] ^ 0x80) << 6) | (unsigned int)(s[3] ^ 0x80);
		return 4;
	}else if (c < 0xfc){
		if (n < 5){
			*pwc = invalidChar;
			return -4;
		}
			
		if (!((s[1] ^ 0x80) < 0x40 && (s[2] ^ 0x80) < 0x40 && (s[3] ^ 0x80) < 0x40 && (s[4] ^ 0x80) < 0x40 && (c >= 0xf9 || s[1] >= 0x88))){
			*pwc = invalidChar;
			if (s[1]&0x80){
				if (s[2]&0x80){
					if (s[3]&0x80){
						if (s[4]&0x80){
							return -5;
						}else{
							return -4;
						}
					}else{
						return -3;
					}
				}else{
					return -2;
				}
			}else{
				return -1;
			}
		}
		
		*pwc = ((unsigned int)(c & 0x03) << 24) | ((unsigned int)(s[1] ^ 0x80) << 18) | ((unsigned int)(s[2] ^ 0x80) << 12) | ((unsigned int)(s[3] ^ 0x80) << 6) | (unsigned int)(s[4] ^ 0x80);
		return 5;
	}else if (c < 0xfe){
		if (n < 6){
			*pwc = invalidChar;
			return -5;
		}
		
		if (!((s[1] ^ 0x80) < 0x40 && (s[2] ^ 0x80) < 0x40 && (s[3] ^ 0x80) < 0x40 && (s[4] ^ 0x80) < 0x40 && (s[5] ^ 0x80) < 0x40 && (c >= 0xfd || s[1] >= 0x84))){
			*pwc = invalidChar;
			if (s[1]&0x80){
				if (s[2]&0x80){
					if (s[3]&0x80){
						if (s[4]&0x80){
							if (s[5]&0x80){
								return -6;
							}else{
								return -5;
							}
						}else{
							return -4;
						}
					}else{
						return -3;
					}
				}else{
					return -2;
				}
			}else{
				return -1;
			}
		}
		
		*pwc = ((unsigned int)(c & 0x01) << 30) | ((unsigned int)(s[1] ^ 0x80) << 24) | ((unsigned int)(s[2] ^ 0x80) << 18) | ((unsigned int)(s[3] ^ 0x80) << 12) | ((unsigned int)(s[4] ^ 0x80) << 6) | (unsigned int)(s[5] ^ 0x80);
		return -6;
	}else{
		*pwc = invalidChar;
		return 1;
	}
}

int utf8_to_utf16 (char *src, size_t srcLen, wchar_t *des)
{

	int i = 0;
	unsigned int wc = 0;
	int chrs;
	int c = 0;

	while(i < srcLen){
		chrs = _UTF8ToUTF32((ubyte*)(src+i), &wc);
		if (chrs < 1) return 0;
		i += chrs;
		des[c++] = wc&0xFFFF;
	}
	return c;
}
