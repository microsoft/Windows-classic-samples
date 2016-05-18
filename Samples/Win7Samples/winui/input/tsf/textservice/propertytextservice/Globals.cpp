//////////////////////////////////////////////////////////////////////
//
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

/* 22a956ef-6f8a-11d7-a6ee-00065b84435c */
const CLSID c_clsidTextService = {
    0x22a956ef,
    0x6f8a,
    0x11d7,
    {0xa6, 0xee, 0x00, 0x06, 0x5b, 0x84, 0x43, 0x5c}
  };

/* 22a956f0-6f8a-11d7-a6ee-00065b84435c */
const GUID c_guidProfile = {
    0x22a956f0,
    0x6f8a,
    0x11d7,
    {0xa6, 0xee, 0x00, 0x06, 0x5b, 0x84, 0x43, 0x5c}
  };

/* 22a956f1-6f8a-11d7-a6ee-00065b84435c */
const GUID c_guidLangBarItemButton = {
    0x22a956f1,
    0x6f8a,
    0x11d7,
    {0xa6, 0xee, 0x00, 0x06, 0x5b, 0x84, 0x43, 0x5c}
  };


/* b4126de5-6f8f-11d7-a6ee-00065b84435c */
const GUID c_guidPropStaticCompact = {
    0xb4126de5,
    0x6f8f,
    0x11d7,
    {0xa6, 0xee, 0x00, 0x06, 0x5b, 0x84, 0x43, 0x5c}
  };
/* b4126de6-6f8f-11d7-a6ee-00065b84435c */
const GUID c_guidPropStatic = {
    0xb4126de6,
    0x6f8f,
    0x11d7,
    {0xa6, 0xee, 0x00, 0x06, 0x5b, 0x84, 0x43, 0x5c}
  };
/* b4126de7-6f8f-11d7-a6ee-00065b84435c */
const GUID c_guidPropCustom = {
    0xb4126de7,
    0x6f8f,
    0x11d7,
    {0xa6, 0xee, 0x00, 0x06, 0x5b, 0x84, 0x43, 0x5c}
  };
