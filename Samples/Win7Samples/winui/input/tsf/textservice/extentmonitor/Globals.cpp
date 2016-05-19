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

/* a5f08fb0-b586-11d8-b6b4-00065b84435c */
const CLSID c_clsidExtentMonitorTextService = {
    0xa5f08fb0,
    0xb586,
    0x11d8,
    {0xb6, 0xb4, 0x00, 0x06, 0x5b, 0x84, 0x43, 0x5c}
  };
/* a5f08fb1-b586-11d8-b6b4-00065b84435c */
const GUID c_guidProfile = {
    0xa5f08fb1,
    0xb586,
    0x11d8,
    {0xb6, 0xb4, 0x00, 0x06, 0x5b, 0x84, 0x43, 0x5c}
  };
/* a5f08fb2-b586-11d8-b6b4-00065b84435c */
const GUID c_guidLangBarItemButton = {
    0xa5f08fb2,
    0xb586,
    0x11d8,
    {0xb6, 0xb4, 0x00, 0x06, 0x5b, 0x84, 0x43, 0x5c}
  };
