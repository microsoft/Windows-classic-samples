/*++

Copyright (c) 2003  Microsoft Corporation

Module Name: IsapiString.h

Abstract:

    A simple string class build on
    the ISAPI_BUFFER class

Author:

    ISAPI developer (Microsoft employee), October 2002

--*/

#ifndef _isapistring_h
#define _isapistring_h

#include <IsapiTools.h>

class ISAPI_STRING
{
public:

    ISAPI_STRING(
        DWORD   dwMaxAlloc = 0
        );

    virtual
    ~ISAPI_STRING();

    BOOL
    Copy(
        CHAR *  szString,
        DWORD   cchString = 0
        );

    BOOL
    CopyW(
        WCHAR * szStringW,
        DWORD   cchStringW = 0
        );

    BOOL
    Append(
        CHAR *  szString,
        DWORD   cchString = 0
        );

    BOOL
    AppendW(
        WCHAR * szStringW,
        DWORD   cchStringW = 0
        );

    BOOL
    Printf(
        CHAR *  szString,
        ...
        );

    BOOL
    vsprintf_s(
        CHAR *  szFormat,
        va_list args
        );

    VOID
    CalcLen(
        VOID
        );

    DWORD
    QueryCB(
        VOID
        );

    DWORD
    QueryCCH(
        VOID
        );

    BOOL
    SetLen(
        DWORD   cchNewLength
        );

    DWORD
    QueryBufferSize(
        VOID
        );

    BOOL
    ResizeBuffer(
        DWORD   cbSize
        );

    DWORD
    QueryMaxAlloc(
        VOID
        );

    VOID
    SetMaxAlloc(
        DWORD   dwMaxAlloc
        );

    CHAR *
    FindStr(
        CHAR *  szSubString,
        BOOL    fCaseInsensitive = FALSE,
        DWORD   dwStartingOffset = 0
        );

    BOOL
    Base64Encode(
        VOID *  pBuffer,
        DWORD   cbBuffer
        );

    BOOL
    Escape(
        BOOL    fAllowDoubleEscaping = FALSE
        );

    VOID
    Unescape(
        VOID
        );

    CHAR *
    QueryStr(
        VOID
        );

    VOID
    ZeroBuffer(
        VOID
        );

private:

    ISAPI_BUFFER    _Buffer;

    BOOL
    CopyToOffset(
        CHAR *  szString,
        DWORD   cchString,
        DWORD   dwOffset
        );

    BOOL
    CopyWToOffset(
        WCHAR * szStringW,
        DWORD   cchStringW,
        DWORD   dwOffset
        );
};

#endif  // _isapistring_h
