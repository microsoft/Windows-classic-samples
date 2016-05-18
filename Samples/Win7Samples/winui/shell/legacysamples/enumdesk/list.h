/**************************************************************************
   THIS CODE AND INFORMATION IS PROVIDED 'AS IS' WITHOUT WARRANTY OF
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
   PARTICULAR PURPOSE.

   Copyright 1999 - 2000 Microsoft Corporation.  All Rights Reserved.
**************************************************************************/

/**************************************************************************

   File:          List.h

   Description:   

**************************************************************************/

/**************************************************************************
   #include statements
**************************************************************************/

#include <windows.h>

/**************************************************************************
   function prototypes
**************************************************************************/

HWND List_Create(HINSTANCE, HWND, HIMAGELIST, HIMAGELIST);
void List_ReleaseCurrentFolder(void);
HRESULT List_DisplayFolder(HWND, LPITEMINFO);
HRESULT List_Refresh(HWND);
LRESULT List_Notify(HWND, LPARAM);
void List_DoContextMenu(HWND, LPPOINT);

/**************************************************************************
   global variables and definitions
**************************************************************************/

