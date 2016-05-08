//-------------------------------------------------------------------------------------
//
// FileName:    OSD.h
//
// Abstract:    global definitions for OSD sample
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
// --------------------------------------------------------------------------------

#pragma once

typedef struct
{
    UINT    nStep;
    UINT    cSteps;
    BOOL    bMuted;
} VOLUME_INFO;

extern BOOL g_bDblBuffered;
extern HWND g_hwndOSD;

#define WM_VOLUMECHANGE     (WM_USER + 12)
#define WM_ENDPOINTCHANGE   (WM_USER + 13)