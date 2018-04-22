            
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



typedef struct{
	TPLAYLISTRECORD *rec;
	TMETAITEM *mitem;
	int mtag;
}tqsortcb;


int playlistSortCB_tag_A (const void *a, const void *b)
{
	const tqsortcb *qscb1 = (tqsortcb *)a;
	const tqsortcb *qscb2 = (tqsortcb *)b;
	char *str1, *str2;
	const int mtag = qscb1->mtag;
	
	if (qscb1->mitem && qscb1->mitem->tag[mtag]){
		str1 = qscb1->mitem->tag[mtag];
		if (!str1) return 1;
	}else{
		return 1;
	}

	if (qscb2->mitem && qscb2->mitem->tag[mtag]){
		str2 = qscb2->mitem->tag[mtag];
		if (!str2) return 1;
	}else{
		return 1;
	}

	return stricmp(str1, str2);
}

int playlistSortCB_tag_D (const void *a, const void *b)
{
	const tqsortcb *qscb1 = (tqsortcb *)a;
	const tqsortcb *qscb2 = (tqsortcb *)b;
	char *str1, *str2;
	const int mtag = qscb1->mtag;
	
	if (qscb1->mitem && qscb1->mitem->tag[mtag]){
		str1 = qscb1->mitem->tag[mtag];
		if (!str1) return 1;
	}else{
		return 1;
	}

	if (qscb2->mitem && qscb2->mitem->tag[mtag]){
		str2 = qscb2->mitem->tag[mtag];
		if (!str2) return 1;
	}else{
		return 1;
	}

	return stricmp(str1, str2) * -1;
}

// sorts by filepath in to ascending order.
int playlistSortCB_path_A (const void *a, const void *b)
{
	const tqsortcb *qscb1 = (tqsortcb *)a;
	const tqsortcb *qscb2 = (tqsortcb *)b;
	const char *str1 = qscb1->rec->item->path;
	const char *str2 = qscb2->rec->item->path;

	return stricmp(str1, str2);
}

// sorts by filepath in to descending order.
int playlistSortCB_path_D (const void *a, const void *b)
{
	const tqsortcb *qscb1 = (tqsortcb *)a;
	const tqsortcb *qscb2 = (tqsortcb *)b;
	const char *str1 = qscb1->rec->item->path;
	const char *str2 = qscb2->rec->item->path;

	return stricmp(str1, str2) * -1;
}

// sorts by title in to ascending order.
// if a title is unavailable then use its path
int playlistSortCB_title_A (const void *a, const void *b)
{
	const tqsortcb *qscb1 = (tqsortcb *)a;
	const tqsortcb *qscb2 = (tqsortcb *)b;

	char *str1 = qscb1->rec->item->title;
	if (!str1)
		str1 = qscb1->rec->item->path;
		
	char *str2 = qscb2->rec->item->title;
	if (!str2)
		str2 = qscb2->rec->item->path;

	return stricmp(str1, str2);
}

// sorts by title in to descending order.
// if a title is unavailable then use its path
int playlistSortCB_title_D (const void *a, const void *b)
{
	const tqsortcb *qscb1 = (tqsortcb *)a;
	const tqsortcb *qscb2 = (tqsortcb *)b;

	char *str1 = qscb1->rec->item->title;
	if (!str1)
		str1 = qscb1->rec->item->path;
		
	char *str2 = qscb2->rec->item->title;
	if (!str2)
		str2 = qscb2->rec->item->path;

	return stricmp(str1, str2) * -1;
}

void playlistSort (PLAYLISTCACHE *plc, TMETATAGCACHE *tagc, const int mtag, const int direction)
{
	if (playlistLock(plc)){
		TPLAYLISTRECORD *rec = plc->first;
		const int total = plc->total;
		
		if (total > 1 && rec){
			if (tagLock(tagc)){
				tqsortcb *qsortcb = my_calloc(total, sizeof(tqsortcb));
				if (qsortcb){

					// add record handles to the array for processing through qsort()
					for (int i = 0; i < total; i++, rec=rec->next){
						qsortcb[i].rec = rec;
						qsortcb[i].mitem = g_tagFindEntryByHash(tagc, rec->item->hash);
						qsortcb[i].mtag = mtag;
					}

					// do the sort
					if (direction == SORT_ASCENDING){
						if (mtag == MTAG_PATH)
							qsort(qsortcb, total, sizeof(tqsortcb), playlistSortCB_path_A);	
						else if (mtag == MTAG_Title)
							qsort(qsortcb, total, sizeof(tqsortcb), playlistSortCB_title_A);	
						else
							qsort(qsortcb, total, sizeof(tqsortcb), playlistSortCB_tag_A);	

					}else if (direction == SORT_DESCENDING){
						if (mtag == MTAG_PATH)
							qsort(qsortcb, total, sizeof(tqsortcb), playlistSortCB_path_D);
						else if (mtag == MTAG_Title)
							qsort(qsortcb, total, sizeof(tqsortcb), playlistSortCB_title_D);
						else
							qsort(qsortcb, total, sizeof(tqsortcb), playlistSortCB_tag_D);
					}

					// sorting completed so relink the records
					plc->first = qsortcb[0].rec;
					for (int i = 0; i < total-1; i++)
						qsortcb[i].rec->next = qsortcb[i+1].rec;
					qsortcb[total-1].rec->next = NULL;

					my_free(qsortcb);
				}
				tagUnlock(tagc);
			}
		}
		playlistUnlock(plc);
	}
}
