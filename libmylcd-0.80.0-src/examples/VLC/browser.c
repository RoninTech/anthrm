            
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



#include "common.h"
//#include "dshowconfig.h"
#include <winreg.h>



#define SHELL_FOLDERS_2K	"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders\\"
#define MY_COMPUTER_2K		"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\CLSID\\{20D04FE0-3AEA-1069-A2D8-08002B30309D}\\"
#define MY_DOCUMENTS_2K		"CLSID\\{450D8FBA-AD25-11D0-98A8-0800361B1103}\\"
#define DESKTOP_2K			"CLSID\\{00021400-0000-0000-C000-000000000046}\\"

#define SHELL_FOLDERS		SHELL_FOLDERS_2K
#define MY_COMPUTER_LOCALE	MY_COMPUTER_2K
#define MY_DOCUMENTS_LOCALE MY_DOCUMENTS_2K
#define DESKTOP_LOCALE		DESKTOP_2K


#if (ADDLINKSHORTCUTS)
static const TSTRSHORTCUT myshortcuts[] = {
	MYSHORTCUTS
};
#endif

static wchar_t *extAudio[] = {
	EXTAUDIO,
	L""
};

static wchar_t *extVideo[] = {
	EXTVIDEO,
	L""
};

static wchar_t *extMedia[] = {
	EXTAUDIO,
	EXTVIDEO,
	EXTIMAGE,
	L""
};

static wchar_t *extImage[] = {
	EXTIMAGE,
	L""
};

int RED;
int GREEN;
int BLUE;
int BLACK;
int WHITE;



static wchar_t *getDesktopName (wchar_t *buffer, size_t blen)
{
	HKEY hKey = 0;
	DWORD type = REG_SZ;
	wchar_t *ret = MYDESKTOP;
	
	if ((ERROR_SUCCESS == RegOpenKeyEx(HKEY_CLASSES_ROOT, DESKTOP_LOCALE, 0, KEY_READ, &hKey))){
		if ((ERROR_SUCCESS == RegQueryValueExW(hKey, L"", 0, &type, (LPBYTE)buffer, (PDWORD)&blen)))
			ret = buffer;
		RegCloseKey(hKey);
	}	
	return ret;
}

static wchar_t *getMyDocumentsName (wchar_t *buffer, size_t blen)
{
	HKEY hKey = 0;
	DWORD type = REG_SZ;
	wchar_t *ret = MYDOCUMENTS;
	
	if ((ERROR_SUCCESS == RegOpenKeyEx(HKEY_CLASSES_ROOT, MY_DOCUMENTS_LOCALE, 0, KEY_READ, &hKey))){
		if ((ERROR_SUCCESS == RegQueryValueExW(hKey, L"", 0, &type, (LPBYTE)buffer, (PDWORD)&blen)))
			ret = buffer;
		RegCloseKey(hKey);
	}	
	return ret;
}

static wchar_t *getMyComputerName (wchar_t *buffer, size_t blen)
{
	HKEY hKey = 0;
	DWORD type = REG_SZ;
	wchar_t *ret = MYCOMPUTERNAME;
	
	if ((ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, MY_COMPUTER_LOCALE, 0, KEY_READ, &hKey))){
		if ((ERROR_SUCCESS == RegQueryValueExW(hKey, L"", 0, &type, (LPBYTE)buffer, (PDWORD)&blen)))
			ret = buffer;
		RegCloseKey(hKey);
	}	
	return ret;
}

static int getVolumeLabel (wchar_t *drive, wchar_t *buffer, size_t blen)
{
	return GetVolumeInformationW(drive, buffer, blen, 0, 0, 0, 0, 0);
}

static int getLineTotal (TTPRINT *print)
{
	return (print->height+print->lineSpace)/(float)(print->textHeight+print->lineSpace);
}

int formatObjs (TVLCPLAYER *vp, TFBROWSER *browser, TTPRINT *print, TPATHOBJECT *path, int firstObject, int x, int y, int colourDir, int colourFile)
{
	TTLINE *line = print->line;
	int i, copied;
	int lineCt = 0;
	wchar_t name[256];
	TPATHOBJECT *subdir;
	print->lineTotal = getLineTotal(browser->print);
	
	for (i = firstObject; i < path->tDirObj && lineCt < print->lineTotal; i++, lineCt++){
		subdir = path->subdir[i];
		line->colour = colourDir;
		copied = 0;

		if (subdir->isDriveLetter){
			if (getVolumeLabel(subdir->dir.name, name, sizeof(name)-1)){
				_snwprintf(line->text, MAX_PATH, L"%s (%s)", name, subdir->dir.name);
				copied = 1;
			}
		}else if (subdir->hasLink){
			_snwprintf(line->text, MAX_PATH, L"%s", subdir->dir.linkName);
			line->colour = BLUE|RED;
			copied = 1;
		}
		if (!copied)
			wcsncpy(line->text, subdir->dir.name, MAX_PATH);
					
		line->obj = &subdir->dir;		
		if (browser->selectedObj == line->obj)
			line->colour = RED;
		
		line->x = x;
		line->y = y+(lineCt*(print->textHeight+print->lineSpace));
		line++;
	}

	if (path->tDirObj-firstObject < 1)
		i = firstObject-path->tDirObj;
	else
		i = 0;

	for (; i < path->tFileObj && lineCt < print->lineTotal; i++, lineCt++){
		line->obj = path->file[i];
		
		if (line->obj->linkName)
			wcsncpy(line->text, line->obj->linkName, MAX_PATH);
		else
			wcsncpy(line->text, line->obj->name, MAX_PATH);
		
		if (browser->selectedObj == line->obj)
			line->colour = RED;
		else if (line->obj->linkName)
			line->colour = GREEN|BLUE;
		else
			line->colour = colourFile;
		
		line->x = x;
		line->y = y+(lineCt*(print->textHeight+print->lineSpace));
		line++;
	}
	return (print->lineTotal = lineCt);
}

int printObjects (TVLCPLAYER *vp, TFBROWSER *browser, TTPRINT *print)
{
	if (!print->lineTotal){
		lSetForegroundColour(print->frame->hw, WHITE);
		const wchar_t *nofiles = L"<empty>";
		lPrintf(print->frame, print->x+6, print->y+3, print->font, LPRT_CPY, (const char*)nofiles);
		return 1;
	}
	
	TLPRINTR rect = {print->x, print->y, print->x+print->width, print->y+print->height, 0,0,0,0};
	TTLINE *line = print->line;
	int i = print->lineTotal;
	
	while(i--){
		lSetForegroundColour(print->frame->hw, line->colour);
		rect.sx = line->x;
		rect.sy = line->y;
		lPrintEx(print->frame, &rect, print->font, PF_DONTFORMATBUFFER|PF_CLIPDRAW, LPRT_CPY, (char*)line->text);

  	//	line->x = rect.sx;
  	//	line->y = rect.sy;
  		line->x2 = rect.ex;
  		line->y2 = rect.ey;

#if DRAWTOUCHRECTS
		int x = line->x;
		int y = line->y;
		int x2 = line->x2;
		int y2 = line->y2;
		if (line->obj->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			lDrawRectangle(print->frame, x, y, x2, y2, BLUE);
		else
			lDrawRectangle(print->frame, x, y, x2, y2, RED|GREEN);
#endif
		line++;
	}

	return 1;
}

static void freeDirectory (TPATHOBJECT *path)
{
	if (path){
		if (path->tFileObj && path->file){
			
			TDATAOBJECT **file = path->file;
			int i = path->tFileObj;
			while(i--){
				if (*file){
					if ((*file)->name)
						my_free((*file)->name);
					if ((*file)->linkName)
						my_free((*file)->linkName);
					my_free(*file);
				}
				file++;
			};
			my_free(path->file);
		}
		if (path->tDirObj && path->subdir){
			
			TPATHOBJECT **subdir = path->subdir;
			int i = path->tDirObj;
			while(i--){
				if (*subdir){
					freeDirectory(*subdir);
					if ((*subdir)->dir.name)
						my_free((*subdir)->dir.name);
						
					if ((*subdir)->hasLink && (*subdir)->dir.linkName)
						my_free((*subdir)->dir.linkName);
					my_free(*subdir);
				}
				subdir++;
			};
			my_free(path->subdir);
		}
		path->file = NULL;
		path->subdir = NULL;
		path->tFileStruct = 0;
		path->tDirStruct = 0;
		path->tDirObj = 0;
		path->tFileObj = 0;
	}
}

static void freeRootDirectory (TPATHOBJECT *path)
{
	freeDirectory(path);
	my_free(path->dir.name);
}

static void createRootPath (TPATHOBJECT *path, wchar_t *parentName)
{
	memset(path, 0, sizeof(TPATHOBJECT));
	path->dir.name = my_wcsdup(parentName);
	path->dir.self = NULL;
	path->dir.linkName = NULL;
	path->tDirObj = 0;
	path->tFileObj = 0;
	path->isRoot = 1;
	path->isLocked = 0;
	path->isDriveLetter = 0;
	path->isAccessed = 0;
	path->hasLink = 0;
	path->parent = NULL;
	path->tFileStruct = 0;
	path->tDirStruct = 0;
}

static inline uint64_t _32BitTo64 (uint64_t high, uint64_t low)
{
	return (high<<32)|low;	
}

static int addDirectory (TPATHOBJECT *path, WIN32_FIND_DATAW *FFD)
{
	if (path->tDirObj){
		int i = path->tDirObj;
		while(i--){
			if (!wcsncmp(FFD->cFileName, path->subdir[i]->dir.name, MAX_PATH)){
				path->isLocked = 1;
				return 2;
			}
		}
	}
	if (path->subdir == NULL){
		path->tDirObj = 0;
		path->tDirStruct = 2;
		path->subdir = my_malloc(path->tDirStruct * sizeof(TPATHOBJECT*));
	}else{
		if (path->tDirObj >= path->tDirStruct){
			path->tDirStruct <<= 1;
			path->subdir = my_realloc(path->subdir, path->tDirStruct * sizeof(TPATHOBJECT*));
			int i;
			for (i = path->tDirObj; i < path->tDirStruct; i++)
				path->subdir[i] = NULL;
		}
	}

	if (path->subdir != NULL){
		path->subdir[path->tDirObj] = my_malloc(sizeof(TPATHOBJECT));
		if (path->subdir[path->tDirObj] != NULL){
			TPATHOBJECT *subdir = path->subdir[path->tDirObj];
			subdir->dir.self = subdir;
			subdir->dir.dwFileAttributes = FFD->dwFileAttributes;
			subdir->dir.name = my_wcsdup(FFD->cFileName);
			subdir->dir.fsize = 0;
			subdir->dir.creationDate = _32BitTo64(FFD->ftCreationTime.dwHighDateTime, FFD->ftCreationTime.dwLowDateTime);
			subdir->dir.modifiedDate = _32BitTo64(FFD->ftLastWriteTime.dwHighDateTime, FFD->ftLastWriteTime.dwLowDateTime);
			subdir->dir.linkName = NULL;
			subdir->tDirObj = 0;
			subdir->tFileObj = 0;
			subdir->subdir = NULL;
			subdir->file = NULL;
			subdir->isRoot = 0;
			subdir->isLocked = 0;
			subdir->isDriveLetter = 0;
			subdir->isAccessed = 0;
			subdir->hasLink = 0;
			subdir->firstObject = 0;
			subdir->parent = path;
			subdir->driveType = 0;
			path->tDirObj++;
			return 1;
		}
	}
	return 0;
}

static int addSymbolLink (TPATHOBJECT *path, WIN32_FIND_DATAW *FFD, int driveType, wchar_t *linkName)
{
	if (addDirectory(path, FFD) == 1){
		path->subdir[path->tDirObj-1]->driveType = driveType;
		path->subdir[path->tDirObj-1]->isDriveLetter = 0;
		path->subdir[path->tDirObj-1]->hasLink = 1;
		path->subdir[path->tDirObj-1]->dir.linkName = my_wcsdup(linkName);
		return 1;
	}
	return 0;
}

static int addLogicalDriveLetter (TPATHOBJECT *path, WIN32_FIND_DATAW *FFD, int driveType)
{
	if (addDirectory(path, FFD) == 1){
		path->subdir[path->tDirObj-1]->driveType = driveType;
		path->subdir[path->tDirObj-1]->isDriveLetter = 1;
		return 1;
	}
	return 0;
}

static int addFile (TPATHOBJECT *path, WIN32_FIND_DATAW *FFD)
{
	//if (file == NULL)
		//return 0;
		
	if (path->tFileObj){ // we don't allow duplicates, report success if present
		int i = path->tFileObj;
		while(i--){	
			if (!wcsncmp(FFD->cFileName, path->file[i]->name, MAX_PATH))
				return 2;
		}
	}
	if (path->file == NULL){
		path->tFileObj = 0;
		path->tFileStruct = 2;
		path->file = my_malloc(path->tFileStruct * sizeof(TDATAOBJECT*));
	}else{
		if (path->tFileObj >= path->tFileStruct){
			path->tFileStruct <<= 1;
			path->file = my_realloc(path->file, path->tFileStruct * sizeof(TDATAOBJECT*));
			int i;
			for (i = path->tFileObj; i < path->tFileStruct; i++)
				path->file[i] = NULL;
		}
	}
	if (path->file != NULL){
		path->file[path->tFileObj] = my_malloc(sizeof(TDATAOBJECT));
		if (path->file[path->tFileObj] != NULL){
			TDATAOBJECT *file = path->file[path->tFileObj];
			file->self = path;
			file->dwFileAttributes = FFD->dwFileAttributes;
			file->name = my_wcsdup(FFD->cFileName);
			file->fsize = _32BitTo64(FFD->nFileSizeHigh, FFD->nFileSizeLow);
			file->creationDate = _32BitTo64(FFD->ftCreationTime.dwHighDateTime, FFD->ftCreationTime.dwLowDateTime);
			file->modifiedDate = _32BitTo64(FFD->ftLastWriteTime.dwHighDateTime, FFD->ftLastWriteTime.dwLowDateTime);
			file->linkName = NULL;
			path->tFileObj++;
			return 1;
		}
	}
	return 0;
}

int addAccessModule (TPATHOBJECT *path, wchar_t *module, wchar_t *linkName)
{
	WIN32_FIND_DATAW FFD;
	FFD.dwFileAttributes = FILE_ATTRIBUTE_OFFLINE;
	wcsncpy(FFD.cFileName, module, MAX_PATH);
	if (addFile(path, &FFD) == 1){
		path->file[path->tFileObj-1]->linkName = my_wcsdup(linkName);
		return 1;
	}
	return 0;
}

/*static int getSortMode (TFBROWSER *browser)
{
	return browser->sort.mode;
}*/

static void setSortMode (TVLCPLAYER *vp, TFBROWSER *browser, int mode)
{
	if (mode >= SORT_TOTAL) mode = 0;
	browser->sort.mode = mode;
	browser->sort.Do = browser->sort.fn[mode];
	
	buttonDisable(vp, PAGE_BROWSER, BBUTTON_SORT_UNSORTED);
	buttonDisable(vp, PAGE_BROWSER, BBUTTON_SORT_UP_NAME);
	buttonDisable(vp, PAGE_BROWSER, BBUTTON_SORT_DN_NAME);
	buttonDisable(vp, PAGE_BROWSER, BBUTTON_SORT_UP_SIZE);
	buttonDisable(vp, PAGE_BROWSER, BBUTTON_SORT_DN_SIZE);
	buttonDisable(vp, PAGE_BROWSER, BBUTTON_SORT_UP_CREATED);
	buttonDisable(vp, PAGE_BROWSER, BBUTTON_SORT_DN_CREATED);
	buttonDisable(vp, PAGE_BROWSER, BBUTTON_SORT_UP_MODIFIED);
	buttonDisable(vp, PAGE_BROWSER, BBUTTON_SORT_DN_MODIFIED);
	buttonEnable(vp, PAGE_BROWSER, browser->sort.mode+BBUTTON_SORT_UNSORTED);
}

static void initSort (TVLCPLAYER *vp, TFBROWSER *browser, int mode)
{
	browser->sort.fn[SORT_NONE] = sortByUnsorted;
	browser->sort.fn[SORT_NAMEA] = sortSubdirsByNameA;
	browser->sort.fn[SORT_NAMED] = sortSubdirsByNameD;
	browser->sort.fn[SORT_FILESIZEA] = sortFilesBySizeA;
	browser->sort.fn[SORT_FILESIZED] = sortFilesBySizeD;
	browser->sort.fn[SORT_CREATIONDATEA] = sortSubdirsByCreationDateA;
	browser->sort.fn[SORT_CREATIONDATED] = sortSubdirsByCreationDateD;
	browser->sort.fn[SORT_MODIFIEDDATEA] = sortSubdirsByModifiedDateA;
	browser->sort.fn[SORT_MODIFIEDDATED] = sortSubdirsByModifiedDateD;
	setSortMode(vp, browser, mode);
}

static void sortPath (TFBROWSER *browser, TPATHOBJECT *path)
{
	browser->sort.Do(path);
}

void setExtFilter (TVLCPLAYER *vp, TFBROWSER *browser, int mode)
{
	switch (mode){ 
	  case EXT_VIDEO:
	  	browser->extStrings = extVideo; 
	  	browser->extType = mode;
	  	break;
	  case EXT_AUDIO:
	  	browser->extStrings = extAudio;
	  	browser->extType = mode;
	  	break;
	  case EXT_IMAGE:
	  	browser->extStrings = extImage; 
	  	browser->extType = mode;
	  	break;
	  case EXT_MEDIA:
	  	browser->extStrings = extMedia; 
	  	browser->extType = mode;
	  	break;
	  case EXT_ALL :
	  	browser->extStrings = NULL; 
	  	browser->extType = mode;
	  	break;
	}
	buttonDisable(vp, PAGE_BROWSER, BBUTTON_FILTER_VIDEO);
  	buttonDisable(vp, PAGE_BROWSER, BBUTTON_FILTER_AUDIO);
  	buttonDisable(vp, PAGE_BROWSER, BBUTTON_FILTER_IMAGE);
  	buttonDisable(vp, PAGE_BROWSER, BBUTTON_FILTER_MEDIA);
  	buttonDisable(vp, PAGE_BROWSER, BBUTTON_FILTER_ALL);
  	buttonEnable(vp, PAGE_BROWSER, browser->extType+BBUTTON_FILTER_AUDIO);
}

int hasPathExt (wchar_t *path, wchar_t **exts)
{
	if (!exts) return 1;
	
	const int slen = wcslen(path);
	int elen;
	
	for (int i = 0; *exts[i] == L'.'; i++){
		elen = wcslen(exts[i]);
		if (elen > slen) continue;
		if (!wcsicmp(path+slen-elen, exts[i]))
			return 1;
	}
	return 0;
}

static int addLocationToList (TFBROWSER *browser, TPATHOBJECT *path, WIN32_FIND_DATAW *FFD)
{
	if ((FFD->dwFileAttributes & FILE_ATTRIBUTE_OFFLINE) || (FFD->dwFileAttributes & FILE_ATTRIBUTE_SYSTEM)){
		return 0;
	}else if (FFD->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){
		if (wcscmp(FFD->cFileName, L".")){
			if (wcscmp(FFD->cFileName, L"..")){
				if (addDirectory(path, FFD)){
					return 1;
				}
			}
		}
	}else{
		if (hasPathExt(FFD->cFileName, browser->extStrings)){
			if (addFile(path, FFD)){
				return 2;
			}
		}
	}
	return 0;
}

int buildCompletePath (TPATHOBJECT *path, wchar_t *buffer, size_t bufferSize)
{
	if (path->isRoot){
		buffer[0] = 0;
		return -1;
	}
	wchar_t tmp[MAX_PATH+1];
	tmp[0] = 0;
	wcsncpy(buffer, path->dir.name, MAX_PATH);
	
	TPATHOBJECT *parent = path;
	do{
		parent = parent->parent;
		if (parent == NULL || parent->isRoot)
			break;

		_snwprintf(tmp, MAX_PATH, L"%s\\%s", parent->dir.name, buffer);
		wcsncpy(buffer, tmp, MAX_PATH);
	}while(!parent->isRoot);
	return 1;
}


int decomposePath (TPATHOBJECT *path, wchar_t **list)
{
	int total = 0;
	while(path && !path->isRoot){
		list[total++] = my_wcsdup(path->dir.name);
		path = path->parent;
	}
	return total;
}

int getFilesByDir (TFBROWSER *browser, TPATHOBJECT *path, wchar_t *mask)
{
	int ct = 0;
	WIN32_FIND_DATAW FindFileData;
	memset(&FindFileData, 0, sizeof(WIN32_FIND_DATAW));
	wchar_t *buffer = my_calloc(sizeof(wchar_t), MAX_PATH+1);
	if (buffer == NULL) return 0;
	
	if (path->parent != NULL)
		buildCompletePath(path, buffer, MAX_PATH);
	else
		wcscat(buffer, path->dir.name);
	wcscat(buffer, L"\\");
	wcscat(buffer, mask);

	HANDLE hFiles = FindFirstFileW(buffer, &FindFileData);
	if (hFiles != INVALID_HANDLE_VALUE){
		do{
			if (addLocationToList(browser, path, &FindFileData))
				ct++;
		}while(!path->isLocked && FindNextFileW(hFiles, &FindFileData));
		FindClose(hFiles);
	}
	my_free(buffer);
	return ct;
}

int getSubdirsObjsByObject (TFBROWSER *browser, TPATHOBJECT *path, wchar_t *mask, int getRMDrives)
{
	TPATHOBJECT **subdir = path->subdir;
	int total = 0;
	int i = path->tDirObj;
	
	while(i--){
		if ((*subdir)->driveType == DRIVE_REMOVABLE && !(*subdir)->isAccessed){
			subdir++;
			continue;
		}
		total += getFilesByDir(browser, *subdir, mask);
		subdir++;
	}
	return total;
}
/*
int getObjectsByPath (TFBROWSER *browser, wchar_t *root, wchar_t *mask, TPATHOBJECT *path)
{
	createRootPath(path, root);
	
	if (path->driveType == DRIVE_REMOVABLE && !path->isAccessed)
		return 0;
	else
		return getFilesByDir(browser, path, mask);
}
*/
static void printPath (TVLCPLAYER *vp, TFBROWSER *browser, TFRAME *frame, TPATHOBJECT *path, int font, int x, int y, int colour)
{
	wchar_t buffer[MAX_PATH+1];
	memset(buffer, 0, sizeof(buffer));
	if (buildCompletePath(path, buffer, MAX_PATH) == -1)
		wcsncpy(buffer, path->dir.name, MAX_PATH);
	else
		wcscat(buffer, L"\\");
	//printSingleLineShadow(frame, font, x, y, colour, (char*)buffer);
	printSingleLine(vp, frame, buffer, font, x, y, colour);
}

static char *sizeToA (uint64_t bytes, char *buffer, size_t blen)
{
	if (bytes > (uint64_t)1024*1024*1024*100){
		snprintf(buffer, blen, "%.0fG", bytes/1024.0/1024.0/1024.0);
		return buffer;
	}else if (bytes > (uint64_t)1024*1024*1024){
		snprintf(buffer, blen, "%.1fG", bytes/1024.0/1024.0/1024.0);
		return buffer;
	}else if (bytes > (uint64_t)1024*1024*100){
		snprintf(buffer, blen, "%.0fM", bytes/1024.0/1024.0);
		return buffer;
	}else if (bytes > (uint64_t)1024*1024){
		snprintf(buffer, blen, "%.1fM", bytes/1024.0/1024.0);
		return buffer;
	}else if (bytes > (uint64_t)1024*100){
		snprintf(buffer, blen, "%.0fK", bytes/1024.0);
		return buffer;
	}else if (bytes > (uint64_t)1024){
		snprintf(buffer, blen, "%.1fK", bytes/1024.0);
		return buffer;
	}else{
		snprintf(buffer, blen, "%d", (int)bytes);
		return buffer;
	}
}

static void printFileSizeColumn (TFBROWSER *browser, TTPRINT *print, const int x, const int font, const int colour)
{
	TLPRINTR rect = {0, 0, print->frame->width-1, print->frame->height-1, 0,0,0,0};
	TTLINE *line = print->line;
	char buffer[32];
	int i;

	lSetForegroundColour(print->frame->hw, colour);	
	for (i = 0; i < print->lineTotal; i++, line++){
		if (!(line->obj->dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) && !(line->obj->dwFileAttributes&FILE_ATTRIBUTE_OFFLINE)){
			rect.sx = x;
			rect.sy = line->y;
			lPrintEx(print->frame, &rect, font, PF_DONTFORMATBUFFER, LPRT_CPY,\
			  sizeToA(line->obj->fsize, buffer, sizeof(buffer)-1));
		}
	}
}

static void printSelectedFile (TVLCPLAYER *vp, TFBROWSER *browser, TTPRINT *print, int x, int y, int colour)
{
	if (browser->selectedObj)
		printSingleLine(vp, print->frame, browser->selectedFile, print->font, x, y, colour);
}

void printPage (TVLCPLAYER *vp, TFBROWSER *browser, TTPRINT *print, TPATHOBJECT *path)
{
	lSetCharacterEncoding(print->frame->hw, CMT_UTF16);
	printObjects(vp, browser, print);
	printPath(vp, browser, print->frame, path, print->font, print->x, 0, GREEN);
	printSelectedFile(vp, browser, print, print->x, print->textHeight, RED|GREEN);
	
	lSetCharacterEncoding(print->frame->hw, ASCII_PAGE);
	printFileSizeColumn(browser, print, print->x+8, print->font, WHITE);
}

static int getSBarTouchHeight (TVLCPLAYER *vp, TFBROWSER *cb)
{
	int h = buttonGet(vp, PAGE_BROWSER, BBUTTON_SBAR_UP)->image->height
		  + buttonGet(vp, PAGE_BROWSER, BBUTTON_SBAR_DOWN)->image->height;
	return (cb->print->height) - (SCROLLBARVSPACE<<1) - h;
}

void drawScrollBar (TFRAME *src, TFRAME *des, int items, int tObjects, int firstObject, int x, int y, int areaW, int areaH)
{
	if (items){
		float tabH = areaH / (float)((tObjects/(float)items));
		if (tabH < 5.0)
			tabH = 5.0;
		else if (tabH > areaH-1)
			tabH = areaH-1;

		float tabY;	
		if (firstObject > 0)
			tabY = y + ((areaH / (float)tObjects) * (float)firstObject);
		else
			tabY = y;
		lCopyArea(src, des, x, tabY, 0, 1, src->width-1, tabH+1);
	}
}

void displayFiles (TVLCPLAYER *vp, TFBROWSER *browser, TTPRINT *print, TPATHOBJECT *path, int firstObject)
{
	formatObjs(vp, browser, print, path, firstObject, print->x+44, print->y, GREEN, RED|GREEN);
	printPage(vp, browser, print, path);

	drawScrollBar(buttonGet(vp, PAGE_BROWSER, BBUTTON_SBAR)->activeBtn,
	  print->frame, 
	  print->lineTotal, // items
	  path->tDirObj+path->tFileObj, // tObjects
	  firstObject,
	  print->x - buttonGet(vp, PAGE_BROWSER, BBUTTON_SBAR_UP)->image->width-6,
	  print->y + buttonGet(vp, PAGE_BROWSER, BBUTTON_SBAR_UP)->image->height + SCROLLBARVSPACE,
	  buttonGet(vp, PAGE_BROWSER, BBUTTON_SBAR_UP)->image->width,
	  getSBarTouchHeight(vp, browser));
}

int drawBrowser (TVLCPLAYER *vp, TFRAME *frame, TFBROWSER *browser)
{
	if (!vp->gui.awake) return 1;

	TBUTTONS *buttons = buttonsGetContainer(vp, PAGE_BROWSER);
	browser->print->frame = frame;

	//if (!playlistGetTotal(getPrimaryPlaylist(vp))){
	if (playlistManagerGetTotal(vp->plm) == 1 && !playlistGetTotal(getPrimaryPlaylist(vp))){
		buttonDisable(vp, PAGE_BROWSER, BBUTTON_CLOSE);
		buttonEnable(vp, PAGE_BROWSER, BBUTTON_EXIT);
	}else{
		buttonEnable(vp, PAGE_BROWSER, BBUTTON_CLOSE);
		buttonDisable(vp, PAGE_BROWSER, BBUTTON_EXIT);
	}

	buttonDisable(vp, PAGE_BROWSER, BBUTTON_SBAR);
	blurImage(vp, frame, 1);
	
	TFRAME *base = imageCacheGetImage(vp, vp->gui.image[IMGC_BROWSERBASE]);
	lDrawImage(base, browser->print->frame, browser->print->x-4, browser->print->line->y-4);
	buttonsRender(buttons->list, buttons->total, frame);
	buttonEnable(vp, PAGE_BROWSER, BBUTTON_SBAR);
	displayFiles(vp, browser, browser->print, browser->path, browser->path->firstObject);
			
#if DRAWTOUCHRECTS
	TBUTTON *button = buttons->list;
	int i;
	for (i = 0; i < buttons->total; button++, i++){
		if (button->enabled)
			lDrawRectangle(frame, button->pos.x, button->pos.y, button->pos.x+button->image->width-1, button->pos.y+button->image->height-1, BLUE);
	}
#endif
	return 1;
}

static int addLocationShortcut (TPATHOBJECT *path, wchar_t *location, wchar_t *linkName)
{
	WIN32_FIND_DATAW FFD;
	memset(&FFD, 0, sizeof(FFD));
	FFD.dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;
	wcsncpy(FFD.cFileName, location, MAX_PATH);
	return addSymbolLink(path, &FFD, DRIVE_REMOVABLE, linkName);
}

static int addShellFoldersLinks (TPATHOBJECT *path)
{
	wchar_t location[MAX_PATH+1];
	wchar_t linkName[MAX_PATH+1];
	DWORD type = REG_SZ;
	HKEY hKey = 0;
		
	if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, SHELL_FOLDERS, 0, KEY_READ, &hKey)){
		DWORD len = MAX_PATH-1;
		if (ERROR_SUCCESS == RegQueryValueExW(hKey, L"Personal", 0, &type, (LPBYTE)location, &len))
			addLocationShortcut(path, location, getMyDocumentsName(linkName, MAX_PATH));

		len = MAX_PATH-1;
		if (ERROR_SUCCESS == RegQueryValueExW(hKey, L"Desktop", 0, &type, (LPBYTE)location, &len))
			addLocationShortcut(path, location, getDesktopName(linkName, MAX_PATH));

		RegCloseKey(hKey);
	}
	return path->tDirObj;
}

int addLogicalDrives (TPATHOBJECT *path)
{
	WIN32_FIND_DATAW FFD;
	char drives[MAX_PATH+1];
	int i, dtype;
	int total = GetLogicalDriveStringsA(MAX_PATH, drives);

	memset(&FFD, 0, sizeof(WIN32_FIND_DATAW));
	FFD.cFileName[0] = 0;
	FFD.cFileName[1] = L':';
	FFD.cFileName[2] = 0;
	
	for (i = 0; i < total; i++){
		if (*(drives+i) == 0){
			dtype = GetDriveTypeA(drives+i-3);
			if (/*dtype != DRIVE_CDROM &&*/ dtype != DRIVE_NO_ROOT_DIR){
				//if (dtype != DRIVE_REMOVABLE){
					FFD.cFileName[0] = *(drives+i-3);
					if (FFD.cFileName[0] != 'A' && FFD.cFileName[0] != 'B'){	// don't add floppy drives
						FFD.dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;
						addLogicalDriveLetter(path, &FFD, DRIVE_REMOVABLE/*dtype*/);
					}
				//}
			}
			*(drives+i) = 32;
		}
	}
	return path->tDirObj;
}

/*
static void free_Playlist (TBPLAYLIST *playlist)
{
	if (playlist->items != -1){
		playlist->items = -1;
		freeRootDirectory(playlist->path);
	}
}
*/
/*
int getPlaylistFileIndex (TBPLAYLIST *playlist, wchar_t *file)
{
	int i;
	for (i = 0; i < playlist->items; i++){
		if (!wcsncmp(playlist->path->file[i]->name, file, MAX_PATH))
			return i;
	}
	return -1;
}
*/
#if 0
int createDShowPlaylist (TFBROWSER *browser, TBPLAYLIST *playlist)
{
	wchar_t *buffer = my_calloc(sizeof(wchar_t), MAX_PATH+1);
	
	int i = 0;
	while (channels[i].channel != CHN_END){
		_snwprintf(buffer, MAX_PATH, L"dshow:// %i ", i);
  		addAccessModule(playlist->path, buffer, channels[i].name);
  		i++;
	}
	my_free(buffer);
  	return 1;
}
#endif


void handleBrowserSelection (TVLCPLAYER *vp, TFBROWSER *cb, TDATAOBJECT *obj)
{
	if (obj->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){
		cb->path = obj->self;
		if (!cb->path->isAccessed && cb->path->driveType == DRIVE_REMOVABLE){
			if (getFilesByDir(cb, cb->path, GLOBALFILEMASK))
				cb->path->isAccessed = 1;
		}
		getSubdirsObjsByObject(cb, cb->path, GLOBALFILEMASK, 1);
		sortPath(cb, cb->path);
	}else{
		wcsncpy(cb->selectedFile, obj->name, MAX_PATH);
		cb->selectedObj = obj;

		if (hasPathExt(cb->selectedFile, extImage)){
			buttonEnable(vp, PAGE_BROWSER, BBUTTON_VIEW);
			buttonDisable(vp, PAGE_BROWSER, BBUTTON_PLAY);
		}else{
			buttonEnable(vp, PAGE_BROWSER, BBUTTON_PLAY);
			buttonDisable(vp, PAGE_BROWSER, BBUTTON_VIEW);
		}
	}
}

int isVideoFile (wchar_t *path)
{
	if (hasPathExt(path, extVideo))
		return 1;
	else if (hasPathExt(path, extImage))
		return 1;
	else if ((int)wcsistr(path, L"video_ts"))
		return 1;
	else
		return (int)wcsstr(path, L"://");
}

void addLinkShortcuts (TPATHOBJECT *path)
{
#if (ADDLINKSHORTCUTS)
	int total = sizeof(myshortcuts)/sizeof(TSTRSHORTCUT);
	while (total--)
		addLocationShortcut(path, myshortcuts[total].path, myshortcuts[total].link);
#endif
}

static int buildRootTree (TFBROWSER *browser, TPATHOBJECT *path)
{
	wchar_t buffer[MAX_PATH+1];
	memset(buffer, 0, sizeof(buffer));

	createRootPath(path, getMyComputerName(buffer, MAX_PATH));
	addLogicalDrives(path);
	addAccessModule(path, L"screen://", L"Desktop module");
	addAccessModule(path, L"dshow://", L"DirectShow module");
	addShellFoldersLinks(path);	
	getSubdirsObjsByObject(browser, path, GLOBALFILEMASK, 0);
	addLinkShortcuts(path);

	sortPath(browser, path);
	return path->tFileObj + path->tDirObj;
}

static int rebuildRootTree (TVLCPLAYER *vp, TFBROWSER *browser)
{
	freeRootDirectory(browser->ppath);
	*browser->selectedFile = 0;
	browser->selectedObj = NULL;
	browser->path = browser->ppath;
	browser->path->firstObject = 0;
	setExtFilter(vp, browser, browser->extType);
	return buildRootTree(browser, browser->path);
}

static int isVideoMedia (wchar_t *name)
{
	if (hasPathExt(name, extVideo) ||
		!wcsicmp(L"screen://", name) ||
		!wcsicmp(L"dshow://", name))
		return 1;
	else
		return 0;
}

int setPlaylistPlayingItem (TVLCPLAYER *vp, PLAYLISTCACHE *plc, int trk, const unsigned int hash)
{
	if (!hash) return -1;
	TPLAYLISTITEM *item;
	TPLAYLIST2 *pl = getPagePtr(vp, PAGE_PLAYLIST2);
	int pos = -1;
	
	while ((item=playlistGetItem(plc, trk))){
		if (item->hash == hash){
			pos = plc->pr->playingItem = trk;
			playlistChangeEvent(vp, plc, pl, trk);
			trackLoadEvent(vp, plc, pl, trk);
			break;
		}
		trk++;
	}
	return pos;
}

int importPlaylistByDir (TFBROWSER *browser, wchar_t *path, PLAYLISTCACHE *plc)
{
	wchar_t *fullpath = my_malloc(sizeof(wchar_t) * MAX_PATH+1);
	if (!fullpath) return -1;
	
	TPATHOBJECT folder;
	memset(&folder, 0, sizeof(TPATHOBJECT));
	
	int insertPosition;
	int firstPosition = 0xFFFFFFF;
	
		
	int len = wcslen(path);
	if (path[len-1] == L'\\') path[len-1] = 0;
	
	createRootPath(&folder, path);	
	int ret = getFilesByDir(browser, &folder, GLOBALFILEMASK);
	if (ret){
		sortPath(browser, &folder);

		TPATHOBJECT *dir = &folder;
		for (int i = 0; i < dir->tFileObj; i++){

			ret = _snwprintf(fullpath, MAX_PATH, L"%s\\%s", path, dir->file[i]->name);
			char *utf8 = convertto8(fullpath);
			insertPosition = playlistAdd(plc, utf8);
				
			if (insertPosition < firstPosition)
				firstPosition = insertPosition;
			if (dir->file[i]->linkName){
				if (UTF16ToUTF8(dir->file[i]->linkName, wcslen(dir->file[i]->linkName), utf8, MAX_PATH_UTF8))
					playlistSetTitle(plc, insertPosition, utf8, 1);
			}
			my_free(utf8);
		}
	}
	freeRootDirectory(&folder);
	my_free(fullpath);
	if (firstPosition != 0xFFFFFFF)
		return firstPosition;
	else
		return -1;
}

int browserImportPlaylistByDirW (TFBROWSER *browser, wchar_t *path, PLAYLISTCACHE *plc, const int recursive)
{
	int pos = importPlaylistByDir(browser, path, plc);
	if (!recursive) return pos;
	
	TPATHOBJECT *folder = my_calloc(1, sizeof(TPATHOBJECT));
	if (!folder) return -1;

	createRootPath(folder, path);	
	int tobjs = getFilesByDir(browser, folder, GLOBALFILEMASK);

	if (tobjs){
		wchar_t *buffer = my_calloc(sizeof(wchar_t), MAX_PATH+1);
		
		if (buffer){
			for (int i = 0; i  < folder->tDirObj; i++){
				TPATHOBJECT *dir = folder->subdir[i];
				_snwprintf(buffer, MAX_PATH, L"%s\\%s", path, dir->dir.name);
				browserImportPlaylistByDirW(browser, buffer, plc, recursive);
			}
		}
		my_free(buffer);
	}
	freeRootDirectory(folder);
	my_free(folder);
	return pos;
}

int browserImportPlaylistByDir (TFBROWSER *browser, char *path, PLAYLISTCACHE *plc, const int recursive)
{
	wchar_t *out = converttow(path);
	int ret = browserImportPlaylistByDirW(browser, out, plc, recursive);
	my_free(out);
	return ret;
}

int browserImportPlaylistByObj (TFBROWSER *browser, TPATHOBJECT *srcdir, PLAYLISTCACHE *plc)
{
	int ret = 0;
	wchar_t *path = my_calloc(sizeof(wchar_t), MAX_PATH+1);
	if (path){
		buildCompletePath(srcdir, path, MAX_PATH);
		ret = browserImportPlaylistByDirW(browser, path, plc, 0);
		my_free(path);
	}
	return ret;
}

int browserNavigateToDirectory (TVLCPLAYER *vp, TFBROWSER *cb, wchar_t *folder)
{
	// break down current folder in to its individual components
	// this will then be used to navigate to the directory
	wchar_t *dlist[MAX_PATH];
	wchar_t *dir;
	int ret = 0;
	int dTotal = 0;
	
	do{
		if ((dir=wcstok(folder, L"\\"))){
			dlist[dTotal++] = my_wcsdup(dir);
			folder += wcslen(dir)+1;
		}
	}while(dir);
		
	if (dTotal){
		TPATHOBJECT *subdir;
		TPATHOBJECT *path = cb->path;

		rebuildRootTree(vp, cb);

		// navigate to the set directory
		for (int i = 0, j = 0; i < path->tDirObj && j < dTotal; i++){
			subdir = path->subdir[i];
			
			if (!wcsicmp(subdir->dir.name, dlist[j])){
				cb->path = subdir->dir.self;				
				if (!cb->path->isAccessed && cb->path->driveType == DRIVE_REMOVABLE){
					if (getFilesByDir(cb, subdir, GLOBALFILEMASK))
						cb->path->isAccessed = 1;
				}

				getSubdirsObjsByObject(cb, subdir, GLOBALFILEMASK, 1);
				sortPath(cb, cb->path);
				path = cb->path;
				i = -1;
				ret = ++j;
			}
		}
		while(--dTotal)
			my_free(dlist[dTotal]);
	}
	return ret;
}

int browserNavigateToFile (TVLCPLAYER *vp, TFBROWSER *fb, TPATHOBJECT *path, const wchar_t *fname)
{

	for (int i = 0; i < path->tFileObj; i++){
		if (!wcsicmp(path->file[i]->name, fname)){
			handleBrowserSelection(vp, fb, path->file[i]);
			return 1;
		}
	}
	return 0;
}

int browserNavigateToDirectoryAndFile (TVLCPLAYER *vp, TFBROWSER *fb, wchar_t *folder, const wchar_t *fname)
{
	if (folder){
		if (browserNavigateToDirectory(vp, fb, folder) && fname){
			if (*fname)
				return browserNavigateToFile(vp, fb, fb->path, fname);
			else
				return 1;
		}
	}
	return 0;
}

void doExtFilterChange (TVLCPLAYER *vp, TFBROWSER *cb, TPATHOBJECT *path)
{
	// break down current path in to its individual components
	// this will then be used to navigate back to current directory
	wchar_t *dlist[MAX_PATH];
	int dTotal = decomposePath(path, dlist);

	if (renderLock(vp)){
		rebuildRootTree(vp, cb);	// rebuild tree with new ext filter in place
		
		if (dTotal){
			TPATHOBJECT *subdir;
			TPATHOBJECT *path = cb->path;
			
			// navigate back to what was the current directory
			for (int i = 0, j = dTotal-1; i < path->tDirObj && j >= 0; i++){
				subdir = path->subdir[i];
				if (!wcscmp(subdir->dir.name, dlist[j])){
					cb->path = subdir->dir.self;				
					if (!cb->path->isAccessed && cb->path->driveType == DRIVE_REMOVABLE){
						if (getFilesByDir(cb, subdir, GLOBALFILEMASK))
							cb->path->isAccessed = 1;
					}
					getSubdirsObjsByObject(cb, subdir, GLOBALFILEMASK, 1);
					sortPath(cb, cb->path);
					path = cb->path;
					i = -1;
					j--;
				}
			}
		}
		renderUnlock(vp);
	}
	if (dTotal){
		while(--dTotal)
			my_free(dlist[dTotal]);
	}
}

int bbuttonCB (const TTOUCHCOORD *pos, TBUTTON *button, const int id, const int flags, void *ptr)
{
	TVLCPLAYER *vp = (TVLCPLAYER*)ptr;
	TFBROWSER *cb = getPagePtr(vp, PAGE_BROWSER);
	TTPRINT *print = cb->print;
	TFRAME *frame = print->frame;
	TPATHOBJECT *path = cb->path;
	
	switch(id){
	  case BBUTTON_SBAR:{ 
		const int total = path->tDirObj + path->tFileObj;
		if (total){
			const float h = getSBarTouchHeight(vp, cb);
			const float tabH = (h / (float)((total/(float)print->lineTotal)))/2.0;
			path->firstObject = (total / h) * (float)(pos->y - tabH);
				
			if (path->firstObject < 0)
				path->firstObject = 0;
			else if (path->firstObject >= total-print->lineTotal)
				path->firstObject = total-print->lineTotal;
		}
		return 1;
	  }
	  case BBUTTON_PLAY:
	  	if (cb->selectedObj){
			int trkStart;
			wchar_t in[MAX_PATH+1];
			memset(in, 0, sizeof(in));
						
			buildCompletePath(cb->selectedObj->self, in, MAX_PATH);
			if (wcsstr(cb->selectedObj->name, L"://") == NULL)
				wcsncat(in, L"\\", MAX_PATH);
			wcsncat(in, cb->selectedObj->name, MAX_PATH);
			
			char *out = convertto8(in);
			char *src = my_strdup(out);
			if (!src){
				my_free(out);
				break;
			}
			char drive[MAX_PATH_UTF8+1];
			char dir[MAX_PATH_UTF8+1];
					
			_splitpath(out, drive, dir, NULL, NULL);
			snprintf(out, MAX_PATH_UTF8, "%s%s", drive, dir);

			PLAYLISTCACHE *plc = playlistManagerCreatePlaylist(vp->plm, out);
			playlistDelete(plc);
			vp->displayPlaylist = playlistManagerGetPlaylistIndex(vp->plm, plc);
			vp->queuedPlaylist = vp->displayPlaylist;

			setPlaybackMode(vp, isVideoFile(in) == 0);				
			if (isVideoMedia(cb->selectedFile))
				setPage(vp, PAGE_NONE);
			else
				setPage(vp, PAGE_BROWSER);

			if (!stricmp(out, "screen://")){
				trkStart = playlistAdd(plc, out);
				playlistSetTitle(plc, trkStart, "Desktop", 0);
					
			}else if (!stricmp(out, "dshow://")){
				trkStart = playlistAdd(plc, out);
				playlistSetTitle(plc, trkStart, "DirectShow", 0);
					
			}else{
				browserImportPlaylistByObj(cb, cb->selectedObj->self, plc);
				trkStart = playlistGetPositionByHash(plc, getHash(src));
			}

			int pos = setPlaylistPlayingItem(vp, plc, trkStart, playlistGetHash(plc, trkStart));
			if (pos >= 0) startPlaylistTrack(vp, plc, pos);
				
			my_free(src);
			my_free(out);

			// remove PLAYLIST_PRIMARY if it's empty
			PLAYLISTCACHE *plcP = playlistManagerGetPlaylistByName(vp->plm, PLAYLIST_PRIMARY);
			if (plc){
				if (!playlistGetTotal(plcP)){
					playlistManagerDeletePlaylist(vp->plm, plcP);
					vp->displayPlaylist = playlistManagerGetPlaylistIndex(vp->plm, plc);
					vp->queuedPlaylist = vp->displayPlaylist;
				}
			}
		}
	  	return 1;

	case BBUTTON_VIEW :
		if (cb->selectedObj){
			wchar_t filename[MAX_PATH+1];
			buildCompletePath(cb->selectedObj->self, filename, MAX_PATH);
			wcsncat(filename, L"\\", MAX_PATH);
			wcsncat(filename, cb->selectedObj->name, MAX_PATH);
			setImageOverlayImage(vp, frame, filename);
			setPageSec(vp, PAGE_IMGOVR);
		}
		return 1;
	  
	  case BBUTTON_EXIT :
	  	exitAppl(vp);
	  	break;
	  	
	  case BBUTTON_CLOSE :
		setPage(vp, PAGE_NONE);
		return 1;
		
	  case BBUTTON_FILTER_VIDEO :
	  case BBUTTON_FILTER_AUDIO :
	  case BBUTTON_FILTER_IMAGE :
	  case BBUTTON_FILTER_MEDIA :
	  case BBUTTON_FILTER_ALL   :
	  	if (++cb->extType > EXT_ALL)
			cb->extType = 0;
		doExtFilterChange(vp, cb, path);
		break;
	  case BBUTTON_SORT_UNSORTED:
	  case BBUTTON_SORT_UP_NAME:
	  case BBUTTON_SORT_DN_NAME:
	  case BBUTTON_SORT_UP_SIZE:
	  case BBUTTON_SORT_DN_SIZE:
	  case BBUTTON_SORT_UP_CREATED:
	  case BBUTTON_SORT_DN_CREATED:
	  case BBUTTON_SORT_UP_MODIFIED:
	  case BBUTTON_SORT_DN_MODIFIED:
	  	if (pos->pen){	// avoid spurious touch reports by accepting only pen up events
			if (++cb->sort.mode >= SORT_TOTAL)
				cb->sort.mode = 0;
			setSortMode(vp, cb, cb->sort.mode);
			sortPath(cb, cb->ppath);
		}
		return 1;
		
	  case BBUTTON_PATHBACK :
		if (path->parent && !path->isRoot)
			cb->path = path->parent;
		return 1;
		
	  case BBUTTON_SBAR_UP :
		path->firstObject -= print->lineTotal;
		if (path->firstObject < 0)
			path->firstObject = 0;
		return 1;

	  case BBUTTON_SBAR_DOWN :
		path->firstObject += print->lineTotal;
		if (path->firstObject >= (path->tFileObj+path->tDirObj)-print->lineTotal)
			path->firstObject = (path->tFileObj+path->tDirObj)-print->lineTotal;
		return 1;
	}
	return 0;
}

int touchBrowser (TTOUCHCOORD *pos, int flags, TVLCPLAYER *vp)
{

	TFBROWSER *cb = getPagePtr(vp, PAGE_BROWSER);
	TTPRINT *print = cb->print;
	TBUTTONS *buttons = buttonsGetContainer(vp, PAGE_BROWSER);
	TBUTTON *button = buttons->list;
	TTOUCHCOORD bpos;
	static unsigned int lastId = 0;
	int handled = 0;
	
	if (!flags){		// pen down
		if (lastId >= pos->id)
			return 0;
		lastId = pos->id;
	}else if (lastId != pos->id){
		return 0;
	}
		
	for (int i = 0; i < buttons->total; i++, button++){
		if (button->enabled){
			if (pos->x >= button->pos.x && pos->x <= button->pos.x+button->image->width){
				if (pos->y >= button->pos.y && pos->y <= button->pos.y+button->image->height){
					if (button->highlight)
						enableHighlight(vp, button);

					my_memcpy(&bpos, pos, sizeof(TTOUCHCOORD));
					bpos.x = pos->x - button->pos.x;
					bpos.y = pos->y - button->pos.y;
					button->callback(&bpos, button, button->id, flags, vp);
					handled = 1;
					break;
				}
			}
		}
	}
	
	// check for file/directory/media selection
	if (!handled){
		TTLINE *line = print->line;
		for (int i = 0; i < print->lineTotal; i++, line++){
			if (pos->y >= line->y && pos->y <= line->y2){
				if (pos->x >= line->x && pos->x <= line->x2){
					handleBrowserSelection(vp, cb, line->obj);
					return 0;
				}
			}
		}
	}
	return 0;
}

int openBrowser (TVLCPLAYER *vp, TFRAME *frame, TFBROWSER *browser)
{

	RED = lGetRGBMask(frame, LMASK_RED);
	GREEN = lGetRGBMask(frame, LMASK_GREEN);
	BLUE = lGetRGBMask(frame, LMASK_BLUE);
	BLACK = lGetRGBMask(frame, LMASK_BLACK);
	WHITE = lGetRGBMask(frame, LMASK_WHITE);

	lSetBackgroundColour(vp->hw, BLACK);
	lSetForegroundColour(vp->hw, WHITE);

	imageCacheAddImage(vp, L"linebase1.png", SKINFILEBPP, &vp->gui.image[IMGC_BG1]);
	imageCacheAddImage(vp, L"browsebase.png", SKINFILEBPP, &vp->gui.image[IMGC_BROWSERBASE]);
	TFRAME *base = imageCacheGetImage(vp, vp->gui.image[IMGC_BROWSERBASE]);

	browser->print = my_calloc(1, sizeof(TTPRINT));
	if (browser->print == NULL) return 0;
	browser->ppath = my_calloc(1, sizeof(TPATHOBJECT));
	if (browser->ppath == NULL) return 0;

	browser->path = browser->ppath;
	browser->print->font = BFONT;
	//lGetTextMetrics(vp->hw, METRICTEST, 0, browser->print->font, NULL, &browser->print->textHeight);
	int fh = 0;
	lGetFontMetrics(vp->hw, browser->print->font, NULL, &fh, NULL, NULL, NULL);

	browser->print->textHeight = fh+2;
	browser->print->frame = frame;
	browser->print->x = 23;
#if G19DISPLAY
	browser->print->lineSpace = 2;
	browser->print->y = 35;
	int x = frame->width-55;
	int y = 33;
	int y_space = 49;
#else
	browser->print->lineSpace = 3;
	browser->print->y = 38;
	int x = frame->width-56;
	int y = 55;
	int y_space = 50;
#endif
	browser->print->width = base->width-14;
	browser->print->height = base->height-10;
	browser->print->lineTotal = getLineTotal(browser->print);
	browser->print->line = my_calloc(1+browser->print->lineTotal, sizeof(TTLINE));
	if (browser->print->line == NULL) return 0;
	lSetFontLineSpacing(vp->hw, browser->print->font, browser->print->lineSpace);

	buttonsCreateContainer(vp, PAGE_BROWSER, BBUTTON_TOTAL);
	TBUTTON *button = buttonGet(vp, PAGE_BROWSER, BBUTTON_VIEW);
	button->pos.x = x;
	button->pos.y = y;
	button->enabled = 0;
	button->callback = bbuttonCB;
	button->id = BBUTTON_VIEW;
	buttonSetImages(vp, button, L"view.png", L"viewhl.png");
	
	button = buttonGet(vp, PAGE_BROWSER, BBUTTON_PLAY);
	button->pos.x = x;
	button->pos.y = y;
	button->enabled = 0;
	button->callback = bbuttonCB;
	button->id = BBUTTON_PLAY;
	buttonSetImages(vp, button, L"play.png", L"playhl.png");
	
	button = buttonGet(vp, PAGE_BROWSER, BBUTTON_CLOSE);
	button->pos.x = x;
	button->pos.y = (y+=y_space);
	button->enabled = 0;
	button->acceptDrag = 0;
	button->callback = bbuttonCB;
	button->id = BBUTTON_CLOSE;
	buttonSetImages(vp, button, L"close.png", NULL);
	
	button = buttonGet(vp, PAGE_BROWSER, BBUTTON_EXIT);
	button->pos.x = x;
	button->pos.y = y; //(y+=y_space);
	button->enabled = 1;
	button->acceptDrag = 0;
	button->callback = bbuttonCB;
	button->id = BBUTTON_EXIT;
	buttonSetImages(vp, button, L"exit.png", NULL);

	button = buttonGet(vp, PAGE_BROWSER, BBUTTON_FILTER_AUDIO);
	button->pos.x = x;
	button->pos.y = (y+=y_space);
	button->enabled = 0;
	button->acceptDrag = 0;
	button->callback = bbuttonCB;
	button->id = BBUTTON_FILTER_AUDIO;
	buttonSetImages(vp, button, L"audio.png", NULL);
	
	button = buttonGet(vp, PAGE_BROWSER, BBUTTON_FILTER_VIDEO);
	button->pos.x = x;
	button->pos.y = y;
	button->enabled = 0;
	button->acceptDrag = 0;
	button->callback = bbuttonCB;
	button->id = BBUTTON_FILTER_VIDEO;
	buttonSetImages(vp, button, L"video.png", NULL);

	button = buttonGet(vp, PAGE_BROWSER, BBUTTON_FILTER_IMAGE);
	button->pos.x = x;
	button->pos.y = y;
	button->enabled = 0;
	button->acceptDrag = 0;
	button->callback = bbuttonCB;
	button->id = BBUTTON_FILTER_IMAGE;
	buttonSetImages(vp, button, L"image.png", NULL);
	
	button = buttonGet(vp, PAGE_BROWSER, BBUTTON_FILTER_MEDIA);
	button->pos.x = x;
	button->pos.y = y;
	button->enabled = 0;
	button->acceptDrag = 0;
	button->callback = bbuttonCB;
	button->id = BBUTTON_FILTER_MEDIA;
	buttonSetImages(vp, button, L"media.png", NULL);
	
	button = buttonGet(vp, PAGE_BROWSER, BBUTTON_FILTER_ALL);
	button->pos.x = x;
	button->pos.y = y;
	button->enabled = 0;
	button->acceptDrag = 0;
	button->callback = bbuttonCB;
	button->id = BBUTTON_FILTER_ALL;
	buttonSetImages(vp, button, L"all.png", NULL);
	
	button = buttonGet(vp, PAGE_BROWSER, BBUTTON_SORT_UNSORTED);
	button->pos.x = x;
	button->pos.y = (y+=y_space);
	button->enabled = 0;
	button->acceptDrag = 1;
	button->callback = bbuttonCB;
	button->id = BBUTTON_SORT_UNSORTED;
	buttonSetImages(vp, button, L"unsorted.png", NULL);

	button = buttonGet(vp, PAGE_BROWSER, BBUTTON_SORT_UP_NAME);
	button->pos.x = x;
	button->pos.y = y;
	button->enabled = 0;
	button->acceptDrag = 1;
	button->callback = bbuttonCB;
	button->id = BBUTTON_SORT_UP_NAME;
	buttonSetImages(vp, button, L"name_up.png", NULL);
	
	button = buttonGet(vp, PAGE_BROWSER, BBUTTON_SORT_DN_NAME);
	button->pos.x = x;
	button->pos.y = y;
	button->enabled = 0;
	button->acceptDrag = 1;
	button->callback = bbuttonCB;
	button->id = BBUTTON_SORT_DN_NAME;
	buttonSetImages(vp, button, L"name_dn.png", NULL);
	
	button = buttonGet(vp, PAGE_BROWSER, BBUTTON_SORT_UP_SIZE);
	button->pos.x = x;
	button->pos.y = y;
	button->enabled = 0;
	button->acceptDrag = 1;
	button->callback = bbuttonCB;
	button->id = BBUTTON_SORT_UP_SIZE;
	buttonSetImages(vp, button, L"size_up.png", NULL);
	
	button = buttonGet(vp, PAGE_BROWSER, BBUTTON_SORT_DN_SIZE);
	button->pos.x = x;
	button->pos.y = y;
	button->enabled = 0;
	button->acceptDrag = 1;
	button->callback = bbuttonCB;
	button->id = BBUTTON_SORT_DN_SIZE;
	buttonSetImages(vp, button, L"size_dn.png", NULL);
	
	button = buttonGet(vp, PAGE_BROWSER, BBUTTON_SORT_UP_CREATED);
	button->pos.x = x;
	button->pos.y = y;
	button->enabled = 0;
	button->acceptDrag = 1;
	button->callback = bbuttonCB;
	button->id = BBUTTON_SORT_UP_CREATED;
	buttonSetImages(vp, button, L"created_up.png", NULL);
	
	button = buttonGet(vp, PAGE_BROWSER, BBUTTON_SORT_DN_CREATED);
	button->pos.x = x;
	button->pos.y = y;
	button->enabled = 0;
	button->acceptDrag = 1;
	button->callback = bbuttonCB;
	button->id = BBUTTON_SORT_DN_CREATED;
	buttonSetImages(vp, button, L"created_dn.png", NULL);
	
	button = buttonGet(vp, PAGE_BROWSER, BBUTTON_SORT_UP_MODIFIED);
	button->pos.x = x;
	button->pos.y = y;
	button->enabled = 0;
	button->acceptDrag = 1;
	button->callback = bbuttonCB;
	button->id = BBUTTON_SORT_UP_MODIFIED;
	buttonSetImages(vp, button, L"modified_up.png", NULL);
	
	button = buttonGet(vp, PAGE_BROWSER, BBUTTON_SORT_DN_MODIFIED);
	button->pos.x = x;
	button->pos.y = y;
	button->enabled = 0;
	button->acceptDrag = 1;
	button->callback = bbuttonCB;
	button->id = BBUTTON_SORT_DN_MODIFIED;
	buttonSetImages(vp, button, L"modified_dn.png", NULL);
		
	button = buttonGet(vp, PAGE_BROWSER, BBUTTON_PATHBACK);
	button->pos.x = browser->print->x+5;
	button->pos.y = 0;
	button->enabled = 1;
	button->acceptDrag = 0;
	button->callback = bbuttonCB;
	button->id = BBUTTON_PATHBACK;
	buttonSetImages(vp, button, L"bpathback.png", L"bpathbackhl.png");
		
	button = buttonGet(vp, PAGE_BROWSER, BBUTTON_SBAR_UP);
	button->pos.x = 2;
	button->pos.y = browser->print->y;
	button->enabled = 1;
	button->acceptDrag = 0;
	button->callback = bbuttonCB;
	button->id = BBUTTON_SBAR_UP;
	buttonSetImages(vp, button, L"up.png", L"uphl.png");
	int h = button->image->height;
	
	button = buttonGet(vp, PAGE_BROWSER, BBUTTON_SBAR);
	button->pos.x = 1;
	button->pos.y = browser->print->y + h + SCROLLBARVSPACE;
	button->enabled = 1;
	button->acceptDrag = 1;
	button->callback = bbuttonCB;
	button->id = BBUTTON_SBAR;
	buttonSetImages(vp, button, L"bsbar.png", L"bsbarhl.png");
	
	button = buttonGet(vp, PAGE_BROWSER, BBUTTON_SBAR_DOWN);
	button->pos.x = 2;
	button->pos.y = browser->print->y + browser->print->height-24;
	button->enabled = 1;
	button->acceptDrag = 0;
	button->callback = bbuttonCB;
	button->id = BBUTTON_SBAR_DOWN;
	buttonSetImages(vp, button, L"down.png", L"downhl.png");

	initSort(vp, browser, 1);
	setExtFilter(vp, browser, EXT_DEFAULT);
	buildRootTree(browser, browser->path);
	return 1;
	
}

int closeBrowser (TVLCPLAYER *vp, TFRAME *frame, TFBROWSER *browser)
{
	buttonsDeleteContainer(vp, PAGE_BROWSER);
	
	*browser->selectedFile = 0;
	if (browser->print->line)
		my_free(browser->print->line);
	my_free(browser->print);
	freeRootDirectory(browser->ppath);
	my_free(browser->ppath);

	return 1;
}

