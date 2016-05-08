/**************************************************************************
   THIS CODE AND INFORMATION IS PROVIDED 'AS IS' WITHOUT WARRANTY OF
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
   PARTICULAR PURPOSE.

   Copyright 1999 - 2000 Microsoft Corporation.  All Rights Reserved.
**************************************************************************/

/**************************************************************************

   File:          Tree.h

   Description:   

**************************************************************************/

/**************************************************************************
   #include statements
**************************************************************************/

#include <windows.h>

/**************************************************************************
   function prototypes
**************************************************************************/

HWND Tree_Create(HINSTANCE, HWND, HIMAGELIST);
void Tree_Init(HWND);
LRESULT Tree_Notify(HWND, LPARAM);
void Tree_DoContextMenu(HWND, LPPOINT);

/**************************************************************************
   global variables and definitions
**************************************************************************/

