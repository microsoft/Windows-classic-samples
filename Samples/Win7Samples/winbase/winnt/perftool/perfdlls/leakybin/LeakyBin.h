/*---------------------------------------------------------------------------
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (C) Microsoft Corporation.  All rights reserved.

LEAKYBIN.H
---------------------------------------------------------------------------*/


#ifdef SHOW_MEMORY_USAGE
#include "..\appmem\pub\appmem.h"
#define G_ALLOC         GlobalAllocP
#define G_FREE          GlobalFreeP
#define G_REALLOC       GlobalReallocP

#else
#define G_ALLOC         GlobalAlloc
#define G_FREE          GlobalFree
#define G_REALLOC       GlobalRealloc

#endif

typedef struct _MEMORY_ALLOC_BLOCK
{
   struct _MEMORY_ALLOC_BLOCK      *pNext;
} MEMORY_ALLOC_BLOCK, *PMEMORY_ALLOC_BLOCK;

#define ALLOCATION_SIZE         (4096*10)
#define TIME_INTERVAL           (100)
#define LEAK_TIMER                      13


#define IDM_EXIT           101
#define IDM_START          201
#define IDM_STOP           202
#define IDM_RESET          203

#define IDM_ABOUT          301
#define IDM_HELPTOPICS     302

#define IDC_STATIC -1

#define DLG_VERFIRST        400
#define IDC_COMPANY         DLG_VERFIRST
#define IDC_FILEDESC        DLG_VERFIRST+1
#define IDC_PRODVER         DLG_VERFIRST+2
#define IDC_COPYRIGHT       DLG_VERFIRST+3
#define IDC_OSVERSION       DLG_VERFIRST+4
#define IDC_TRADEMARK       DLG_VERFIRST+5
#define DLG_VERLAST         DLG_VERFIRST+5

#define IDC_LABEL           DLG_VERLAST+1

#define IDS_APP_TITLE       500
#define IDS_DISPLAYCHANGED  501
#define IDS_VER_INFO_LANG   502
#define IDS_VERSION_ERROR   503
#define IDS_NO_HELP         504
