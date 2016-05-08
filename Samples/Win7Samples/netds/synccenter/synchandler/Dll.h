//////////////////////////////////////////////////////////////////////////////
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//  Module Name:
//      Dll.h
//
//  Abstract:
//      Include file for standard system include files or project specific
//      include files that are used frequently but are changed infrequently.
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

extern HINSTANCE g_hmodThisDll;

STDAPI_(void) DllAddRef();
STDAPI_(void) DllRelease();
