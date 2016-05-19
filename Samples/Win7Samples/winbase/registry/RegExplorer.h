
/******************************************************************************
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (C) 1993 - 2000.  Microsoft Corporation.  All rights reserved.

This source code is only intended as a supplement to Microsoft Development
Tools and/or WinHelp documentation. See these sources for detailed information
regarding the Microsoft samples programs.
******************************************************************************/

#define IDE_NAME                    101
#define IDE_TITLEINDEX              102
#define IDE_ACL                     104
#define IDE_LASTWRT                 105
#define IDE_VALUE1                  106
#define IDE_VALUE2                  107
#define IDE_VALUE3                  108
#define IDE_CLASS                   109
#define IDB_NEXT                    111
#define IDB_BACK                    112
#define IDL_LISTBOX                 110
#define IDL_LISTBOX2                103
#define IDE_CVALUES                 113
#define IDB_PRINT                   114
#define IDE_TEXTOUT                 116

#define IDS_SELECTMSG               201
#define IDS_NAME                    202
#define IDS_TOPLEVEL                203
#define IDS_CANTOPENKEY             204
#define IDS_OPENKEYERR              205
#define IDS_REGERR                  206
#define IDS_DEBUG                   207
#define IDS_NONAME                  208
#define IDS_REGOPENERR              209
#define IDS_REGQUERYERR             210
#define IDS_REGENUMERR              211
#define IDS_REGNONE                 212
#define IDS_REGSZ                   213
#define IDS_REGEXPSZ                214
#define IDS_REGBIN                  215
#define IDS_MALLOCERR               216
#define IDS_DBGTITLE                217
#define IDS_REGDWORD                218
#define IDS_REGBIGEND               219
#define IDS_REGLINK                 220
#define IDS_REGMULTI                221
#define IDS_REGRESLST               222
#define IDS_REGFULLDESC             223
#define IDS_REGUNDEFINE             224


#define MAX_VALUE_NAME              128
#define MAX_DATA_LEN                1024

#define LINE_LEN                    80

#define BRANCH                      1
#define CHILD                       2
#define OTHER                       3

#define BOOT_INI                    "c:\\boot.ini"
#define REGISTRY_TXT                "registry.txt"

// MAX_PATH is defined as 260 in windef.h

#define WM_GETFIRSTKEY              WM_APP

int  APIENTRY RegExplorerDlgProc (HWND, WORD, LONG, LONG);

VOID EnumerateLevel (HWND hDlg, LPTSTR NameLBSelect,
                     LPTSTR RegPath, DWORD RegPathLength,
                     HKEY *hKeyRoot);

VOID QueryKey (HWND, HANDLE);
VOID DisplayKeyData (HWND, LPTSTR, HANDLE);
