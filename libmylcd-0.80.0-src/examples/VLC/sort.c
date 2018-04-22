            
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


int qsDirCbA (const void *a, const void *b)
{
	TPATHOBJECT **sub1 = (TPATHOBJECT **)a;
	TPATHOBJECT **sub2 = (TPATHOBJECT **)b;
	wchar_t *str1, *str2;
	if ((*sub1)->hasLink)
		str1 = (*sub1)->dir.linkName;
	else
		str1 = (*sub1)->dir.name;

	if ((*sub2)->hasLink)
		str2 = (*sub2)->dir.linkName;
	else
		str2 = (*sub2)->dir.name;
	return _wcsnicmp(str1, str2, MAX_PATH);
}

int qsDirCbD (const void *a, const void *b)
{
	TPATHOBJECT **sub1 = (TPATHOBJECT **)a;
	TPATHOBJECT **sub2 = (TPATHOBJECT **)b;
	wchar_t *str1, *str2;
	if ((*sub1)->hasLink)
		str1 = (*sub1)->dir.linkName;
	else
		str1 = (*sub1)->dir.name;

	if ((*sub2)->hasLink)
		str2 = (*sub2)->dir.linkName;
	else
		str2 = (*sub2)->dir.name;
	return _wcsnicmp(str1, str2, MAX_PATH) * -1;
}

int qsFileCbA (const void *a, const void *b)
{
	TDATAOBJECT **file1 = (TDATAOBJECT **)a;
	TDATAOBJECT **file2 = (TDATAOBJECT **)b;
	return _wcsnicmp((*file1)->name, (*file2)->name, MAX_PATH);
}

int qsFileCbD (const void *a, const void *b)
{
	TDATAOBJECT **file1 = (TDATAOBJECT **)a;
	TDATAOBJECT **file2 = (TDATAOBJECT **)b;
	return _wcsnicmp((*file1)->name, (*file2)->name, MAX_PATH) * -1;
}

int qsFileCbSizeA (const void *a, const void *b)
{
	TDATAOBJECT **file1 = (TDATAOBJECT **)a;
	TDATAOBJECT **file2 = (TDATAOBJECT **)b;
	return (*file1)->fsize - (*file2)->fsize;
}

int qsFileCbSizeD (const void *a, const void *b)
{
	TDATAOBJECT **file1 = (TDATAOBJECT **)a;
	TDATAOBJECT **file2 = (TDATAOBJECT **)b;
	return (*file2)->fsize - (*file1)->fsize;
}

int qsFileCbCDateA (const void *a, const void *b)
{
	TDATAOBJECT **file1 = (TDATAOBJECT **)a;
	TDATAOBJECT **file2 = (TDATAOBJECT **)b;
	if ((*file1)->creationDate < (*file2)->creationDate)
		return -1;
	else if ((*file1)->creationDate > (*file2)->creationDate)
		return 1;
	else
		return 0;
}

int qsFileCbCDateD (const void *a, const void *b)
{
	TDATAOBJECT **file1 = (TDATAOBJECT **)a;
	TDATAOBJECT **file2 = (TDATAOBJECT **)b;
	if ((*file1)->creationDate < (*file2)->creationDate)
		return 1;
	else if ((*file1)->creationDate > (*file2)->creationDate)
		return -1;
	else
		return 0;
}

int qsDirCbCdateA (const void *a, const void *b)
{
	TPATHOBJECT **sub1 = (TPATHOBJECT **)a;
	TPATHOBJECT **sub2 = (TPATHOBJECT **)b;
	if ((*sub1)->dir.creationDate < (*sub2)->dir.creationDate)
		return -1;
	else if ((*sub1)->dir.creationDate > (*sub2)->dir.creationDate)
		return 1;
	else
		return 0;
}

int qsDirCbCdateD (const void *a, const void *b)
{
	TPATHOBJECT **sub1 = (TPATHOBJECT **)a;
	TPATHOBJECT **sub2 = (TPATHOBJECT **)b;
	if ((*sub1)->dir.creationDate < (*sub2)->dir.creationDate)
		return 1;
	else if ((*sub1)->dir.creationDate > (*sub2)->dir.creationDate)
		return -1;
	else
		return 0;
}

void sortFilesByNameD (TPATHOBJECT *path)
{
	if (path->tFileObj < 2)
		return;

	qsort(path->file, path->tFileObj, sizeof(TDATAOBJECT*), qsFileCbD);
}

void sortFilesByNameA (TPATHOBJECT *path)
{
	if (path->tFileObj < 2)
		return;

	qsort(path->file, path->tFileObj, sizeof(TDATAOBJECT*), qsFileCbA);
}

void sortFilesByCreationDateA (TPATHOBJECT *path)
{
	if (path->tFileObj < 2)
		return;

	qsort(path->file, path->tFileObj, sizeof(TDATAOBJECT*), qsFileCbCDateA);
}

void sortFilesByCreationDateD (TPATHOBJECT *path)
{
	if (path->tFileObj < 2)
		return;

	qsort(path->file, path->tFileObj, sizeof(TDATAOBJECT*), qsFileCbCDateD);
}

void sortSubdirsByCreationDateA (TPATHOBJECT *path)
{
	sortFilesByCreationDateA(path);

	if (path->tDirObj > 1)
		qsort(path->subdir, path->tDirObj, sizeof(TPATHOBJECT*), qsDirCbCdateA);

	int i;
	for (i = 0; i < path->tDirObj; i++)
		sortSubdirsByCreationDateA(path->subdir[i]);
}

void sortSubdirsByCreationDateD (TPATHOBJECT *path)
{
	sortFilesByCreationDateD(path);

	if (path->tDirObj > 1)
		qsort(path->subdir, path->tDirObj, sizeof(TPATHOBJECT*), qsDirCbCdateD);

	int i;
	for (i = 0; i < path->tDirObj; i++)
		sortSubdirsByCreationDateD(path->subdir[i]);
}

void sortSubdirsByNameA (TPATHOBJECT *path)
{
	sortFilesByNameA(path);

	if (path->tDirObj > 1)
		qsort(path->subdir, path->tDirObj, sizeof(TPATHOBJECT*), qsDirCbA);

	int i;
	for (i = 0; i < path->tDirObj; i++)
		sortSubdirsByNameA(path->subdir[i]);
}

void sortSubdirsByNameD (TPATHOBJECT *path)
{
	sortFilesByNameD(path);

	if (path->tDirObj > 1)
		qsort(path->subdir, path->tDirObj, sizeof(TPATHOBJECT*), qsDirCbD);

	int i;
	for (i = 0; i < path->tDirObj; i++)
		sortSubdirsByNameD(path->subdir[i]);
}

void sortFilesBySizeA (TPATHOBJECT *path)
{
	if (path->tFileObj > 1)
		qsort(path->file, path->tFileObj, sizeof(TDATAOBJECT*), qsFileCbSizeA);
		
	int i;
	for (i = 0; i < path->tDirObj; i++)
		sortFilesBySizeA(path->subdir[i]);
}

void sortFilesBySizeD (TPATHOBJECT *path)
{
	if (path->tFileObj > 1)
		qsort(path->file, path->tFileObj, sizeof(TDATAOBJECT*), qsFileCbSizeD);
		
	int i;
	for (i = 0; i < path->tDirObj; i++)
		sortFilesBySizeD(path->subdir[i]);
}


int qsFileCbMDateA (const void *a, const void *b)
{
	TDATAOBJECT **file1 = (TDATAOBJECT **)a;
	TDATAOBJECT **file2 = (TDATAOBJECT **)b;
	if ((*file1)->modifiedDate < (*file2)->modifiedDate)
		return -1;
	else if ((*file1)->modifiedDate > (*file2)->modifiedDate)
		return 1;
	else
		return 0;
}

int qsFileCbMDateD (const void *a, const void *b)
{
	TDATAOBJECT **file1 = (TDATAOBJECT **)a;
	TDATAOBJECT **file2 = (TDATAOBJECT **)b;
	if ((*file1)->modifiedDate < (*file2)->modifiedDate)
		return 1;
	else if ((*file1)->modifiedDate > (*file2)->modifiedDate)
		return -1;
	else
		return 0;
}

int qsDirCbMdateA (const void *a, const void *b)
{
	TPATHOBJECT **sub1 = (TPATHOBJECT **)a;
	TPATHOBJECT **sub2 = (TPATHOBJECT **)b;
	if ((*sub1)->dir.modifiedDate < (*sub2)->dir.modifiedDate)
		return -1;
	else if ((*sub1)->dir.modifiedDate > (*sub2)->dir.modifiedDate)
		return 1;
	else
		return 0;
}

int qsDirCbMdateD (const void *a, const void *b)
{
	TPATHOBJECT **sub1 = (TPATHOBJECT **)a;
	TPATHOBJECT **sub2 = (TPATHOBJECT **)b;
	if ((*sub1)->dir.modifiedDate < (*sub2)->dir.modifiedDate)
		return 1;
	else if ((*sub1)->dir.modifiedDate > (*sub2)->dir.modifiedDate)
		return -1;
	else
		return 0;
}

void sortFilesByModifiedDateA (TPATHOBJECT *path)
{
	if (path->tFileObj < 2)
		return;

	qsort(path->file, path->tFileObj, sizeof(TDATAOBJECT*), qsFileCbMDateA);
}

void sortFilesByModifiedDateD (TPATHOBJECT *path)
{
	if (path->tFileObj < 2)
		return;

	qsort(path->file, path->tFileObj, sizeof(TDATAOBJECT*), qsFileCbMDateD);
}

void sortSubdirsByModifiedDateA (TPATHOBJECT *path)
{
	sortFilesByModifiedDateA(path);

	if (path->tDirObj > 1)
		qsort(path->subdir, path->tDirObj, sizeof(TPATHOBJECT*), qsDirCbMdateA);

	int i;
	for (i = 0; i < path->tDirObj; i++)
		sortSubdirsByModifiedDateA(path->subdir[i]);
}

void sortSubdirsByModifiedDateD (TPATHOBJECT *path)
{
	sortFilesByModifiedDateD(path);

	if (path->tDirObj > 1)
		qsort(path->subdir, path->tDirObj, sizeof(TPATHOBJECT*), qsDirCbMdateD);

	int i;
	for (i = 0; i < path->tDirObj; i++)
		sortSubdirsByModifiedDateD(path->subdir[i]);
}

void sortByUnsorted (TPATHOBJECT *path)
{
	return;
}

	
	