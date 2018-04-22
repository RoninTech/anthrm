
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

#ifndef _CMAP_H_
#define _CMAP_H_


typedef struct{
	unsigned int tlines;
	ubyte	**lines;
	ubyte	*data;
}TASCIILINE;

wchar_t *getMapPath (THWD *hw);
int buildCMapTable (THWD *hw, int id);
TMAPTABLE *CMapIDToTable (THWD *hw, int mapID);
void resetTableIndex (THWD *hw);
void deleteRootCMapList (THWD *hw);
int registerCharacterMaps (THWD *hw, const wchar_t *mapPath);
int enumLanguageTables (THWD *hw, char **alias, int *langID, wchar_t **filepath);
int encodingAliasToID (THWD *hw, const char *name);


#endif

