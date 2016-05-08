////////////////////////////////////////////////////////////////////// //
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
//  TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//  Copyright (C) 2003  Microsoft Corporation.  All rights reserved.
//
//  Globals.cpp
//
//          Global variables
//
//////////////////////////////////////////////////////////////////////

#include "globals.h"

HINSTANCE g_hInst;

LONG g_cRefDll = -1; // -1 /w no refs, for win95 InterlockedIncrement/Decrement compat

CRITICAL_SECTION g_cs;

/* 7c3d9fe0-6f60-11d7-a6ee-00065b84435c */
const CLSID c_clsidPropertyMonitorTextService = {
    0x7c3d9fe0,
    0x6f60,
    0x11d7,
    {0xa6, 0xee, 0x00, 0x06, 0x5b, 0x84, 0x43, 0x5c}
  };

/* 7c3d9fe1-6f60-11d7-a6ee-00065b84435c */
const GUID c_guidProfile = {
    0x7c3d9fe1,
    0x6f60,
    0x11d7,
    {0xa6, 0xee, 0x00, 0x06, 0x5b, 0x84, 0x43, 0x5c}
  };

/* 7c3d9fe2-6f60-11d7-a6ee-00065b84435c */
const GUID c_guidLangBarItemButton = {
    0x7c3d9fe2,
    0x6f60,
    0x11d7,
    {0xa6, 0xee, 0x00, 0x06, 0x5b, 0x84, 0x43, 0x5c}
  };
