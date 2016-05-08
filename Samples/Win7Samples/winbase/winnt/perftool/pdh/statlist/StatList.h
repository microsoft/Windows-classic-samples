/*---------------------------------------------------------------------------
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (C) Microsoft Corporation.  All rights reserved.

STATLIST.H
---------------------------------------------------------------------------*/

// Makes it easier to determine appropriate code paths:
#define IS_WIN32 TRUE
#define IS_NT      IS_WIN32 && (BOOL)(GetVersion() < 0x80000000)
#define IS_WIN32S  IS_WIN32 && (BOOL)(!(IS_NT) && (LOBYTE(LOWORD(GetVersion()))<4))
#define IS_WIN95 (BOOL)(!(IS_NT) && !(IS_WIN32S)) && IS_WIN32

#define IDM_NEW            101
#define IDM_OPEN           102
#define IDM_SAVE           103
#define IDM_SAVEAS         104
#define IDM_EXIT           105

#define IDM_ADD_COUNTERS   201
#define IDM_DELETE_COUNTER 202
#define IDM_CLEAR_ALL      203
#define IDM_SETTINGS       204

#define IDM_GET_DATA       300

#define IDM_HELPTOPICS     401
#define IDM_ABOUT          402

#define IDC_STATIC -1

#define DLG_VERFIRST        1000
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

extern char APPNAME[];

