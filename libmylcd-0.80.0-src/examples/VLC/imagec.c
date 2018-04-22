
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


static TIMAGECACHE *getImageCache (TVLCPLAYER *vp)
{
	return vp->imgc;
}

static int imageCacheResize (TIMAGECACHE *imgc, int newSize)
{
	imgc->items = my_realloc(imgc->items, (1+newSize) * sizeof(TIMAGECACHEITEM));
	for (int i = imgc->total; i <= newSize; i++){
		imgc->items[i].image = NULL;
		imgc->items[i].id = 0;
	}
	imgc->total = newSize;
	return (imgc->items != NULL);
}

static TIMAGECACHEITEM *imageCacheGetFreeSlot (TIMAGECACHE *imgc)
{
	TIMAGECACHEITEM *item = imgc->items;
	
	for (int i = 0; i < imgc->total; i++)
		if (!item[i].id) return &item[i];

	if (imageCacheResize(imgc, imgc->total*2))
		return imageCacheGetFreeSlot(imgc);
	else
		return NULL;
}

static int imageCacheNewId (TIMAGECACHE *imgc)
{
	TIMAGECACHEITEM *item = imgc->items;
	int id = 0;
	for (int i = 0; i < imgc->total; i++, item++){
		if (item->id > id) id = item->id;
	}
	return id + 1;
}

static void imageCacheRelease (TIMAGECACHE *imgc)
{
	TIMAGECACHEITEM *item = imgc->items;
	
	for (int i = 0; i < imgc->total; i++, item++){
		if (item->id){
			lDeleteFrame(item->image);
			item->image = NULL;
			item->id = 0;
		}
	}
}

static int imageCacheReloadImage (TVLCPLAYER *vp, TIMAGECACHEITEM *item)
{
	wchar_t buffer[MAX_PATH];
	return lLoadImage(item->image, buildSkinD(vp, buffer, item->path));
}

TFRAME *imageCacheGetImage (TVLCPLAYER *vp, int id)
{
	if (id < 1){
		//printf("getImageCacheImage: id < 1\n");
		return NULL;
	}
	
	TIMAGECACHE *imgc = getImageCache(vp);
	TIMAGECACHEITEM *item = imgc->items;
	for (int i = 0; i < imgc->total; i++, item++){
		if (item->id == id) return item->image;
	}
	return NULL;
}

int imageCacheReloadImages (TVLCPLAYER *vp)
{
	TIMAGECACHE *imgc = getImageCache(vp);
	TIMAGECACHEITEM *item = imgc->items;
	
	for (int i = 0; i < imgc->total; i++, item++){
		if (item->id)
			imageCacheReloadImage(vp, item);
	}
	return 1;
}

TFRAME *imageCacheAddImage (TVLCPLAYER *vp, wchar_t *path, int bpp, int *id)
{
	TIMAGECACHE *imgc = getImageCache(vp);
	TIMAGECACHEITEM *item = imageCacheGetFreeSlot(imgc);
	wchar_t buffer[MAX_PATH];
	
	wcsncpy(item->path, path, MAX_PATH);
	item->bpp = bpp;
	item->id = *id = imageCacheNewId(imgc);
	item->image = lNewImage(vp->hw, buildSkinD(vp, buffer, item->path), item->bpp);
	if (item->image == NULL){
		item->id = *id = 0;
		//wprintf(L"imageCacheAddImage(): image load failed for '%s'\n", path);
		//printf("\n");
	}
	return item->image;
}

TIMAGECACHE *imageCacheNew (const int initialSize)
{
	TIMAGECACHE *imgc = my_calloc(1, sizeof(TIMAGECACHE));
	imgc->total = initialSize;
	imgc->items = my_calloc(1+initialSize, sizeof(TIMAGECACHEITEM));
	return imgc;
}

void imageCacheFree (TIMAGECACHE *imgc)
{
	imageCacheRelease(imgc);
	my_free(imgc->items);
	my_free(imgc);
}

