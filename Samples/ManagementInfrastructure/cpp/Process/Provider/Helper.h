//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//

#ifndef __HELPER_H
#define __HELPER_H

#include <mi.h>
#include <Windows.h>

#define HRESULT_TO_WIN32(hres) ((HRESULT_FACILITY(hres) == FACILITY_WIN32) ? HRESULT_CODE(hres) : (hres))

MI_Result ResultFromWin32Error(DWORD error);

#endif