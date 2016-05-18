// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include <shlobj.h>
#include <shlwapi.h>            // QISearch, easy way to implement QI
#include <wininet.h>
#include <new>
#include "NonDefaultDropMenuVerb.h"

#pragma comment(lib, "shlwapi.lib")     // link to this

HRESULT CNonDefaultDropMenuVerb_CreateInstance(REFIID riid, void **ppv);

extern HINSTANCE    g_hinst;

void DllAddRef();
void DllRelease();

template <class T> void SafeRelease(T **ppT)
{
    if (*ppT)
    {
        (*ppT)->Release();
        *ppT = NULL;
    }
}

#define IDS_CHECKDROP           101
#define IDS_CHECKNETRESOURCES   102
#define IDS_MESSAGEBOXTITLE     103
#define IDS_ERRORMESSAGEFS      104
#define IDS_ERRORMESSAGENET     105
#define IDS_MESSAGETEMPLATEFS   106
#define IDS_MESSAGETEMPLATENET  107
#define IDS_ERRORMESSAGENA      108

#define IDM_CHECKDROP           0
#define IDM_CHECKNETRESOURCES   1
