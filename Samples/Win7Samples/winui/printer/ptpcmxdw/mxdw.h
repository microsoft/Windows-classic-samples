// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.


const char gszTechnologyIdentifier[] = "http://schemas.microsoft.com/xps/2005/06";

HRESULT
IsXPSCapableDriver(
    __in HDC hdcMXDW
    );

HRESULT
PutTogetherEscapeStructureForPrintTicket(
    __in                               ULONG           OpCode,
    __in_bcount(cbInBuffer)            PBYTE           pbInBuffer,
    __in                               DWORD           cbInBuffer,
    __deref_out_bcount(*pcbOutBuffer)  PBYTE          *ppbOutBuffer,
    __out                              PDWORD          pcbOutBuffer
    );

HRESULT
PutTogetherEscapeStructureForImage(
    __in                              ULONG                       OpCode,
    __in_bcount(cbInBuffer)           PBYTE                       pbInBuffer,
    __in                              DWORD                       cbInBuffer,
    __in                              LPCSTR                      pszURI,
    __deref_out_bcount(*pcbOutBuffer) PBYTE                      *ppbOutBuffer,
    __out                             PDWORD                      pcbOutBuffer
    );

HRESULT
PutTogetherEscapeStructureForFixedPage(
    __in                              ULONG                       OpCode,
    __in_bcount(cbInBuffer)           PBYTE                       pbInBuffer,
    __in                              DWORD                       cbInBuffer,
    __deref_out_bcount(*pcbOutBuffer) PBYTE                      *ppbOutBuffer,
    __out                             PDWORD                      pcbOutBuffer
    );