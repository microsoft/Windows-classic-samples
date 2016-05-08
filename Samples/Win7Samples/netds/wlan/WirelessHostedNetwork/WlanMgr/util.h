// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

#ifndef _UTIL_H_
#define _UTIL_H_
#include "resource.h"
DWORD
StringToSsid(
    __in LPCWSTR strSsid,
    __out PDOT11_SSID pDot11Ssid
    );

__success(ERROR_SUCCESS)
DWORD
SsidToDisplayName(
    __in PDOT11_SSID pDot11Ssid,
    __in BOOL bHexFallback,
    __out_ecount_opt(*pcchDisplayName) LPWSTR strDisplayName,
    __inout DWORD *pcchDisplayName
    );

DWORD 
ConvertPassPhraseKeyStringToBuffer(
    __in_ecount(dwLength) LPCWSTR strPassKeyString,     // Unicode string 
    __in DWORD dwLength,
    __in DOT11_AUTH_ALGORITHM dot11Auth,
    __out_ecount_opt(*pdwBufLen) UCHAR* strPassKeyBuf,  // NULL to get length required
    __inout DWORD *pdwBufLen                            // in: length of buffer; out: chars copied/required
    );
#endif  // _UTIL_H_
