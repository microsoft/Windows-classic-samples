/*++

Copyright (c) 2003  Microsoft Corporation

Module Name: IsapiString.h

Abstract:

    A simple string class build on
    the ISAPI_BUFFER class

Author:

    ISAPI developer (Microsoft employee), October 2002

--*/

#ifndef _isapistringw_h
#define _isapistringw_h

#include <IsapiTools.h>

class ISAPI_STRINGW
{
public:

    ISAPI_STRINGW(
        DWORD   dwMaxAlloc = 0
        );

    virtual
    ~ISAPI_STRINGW();

    BOOL
    Copy(
        WCHAR * szStringW,
        DWORD   cchStringW = 0
        );

    BOOL
    CopyA(
        CHAR *  szString,
        DWORD   cchString = 0
        );

    BOOL
    Append(
        WCHAR * szStringW,
        DWORD   cchStringW = 0
        );

    BOOL
    AppendA(
        CHAR *  szString,
        DWORD   cchString = 0
        );

    BOOL
    Printf(
        WCHAR * szStringW,
        ...
        );

    BOOL
    vsprintf_s(
        WCHAR * szFormatW,
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

    WCHAR *
    QueryStr(
        VOID
        );

private:

    ISAPI_BUFFER    _Buffer;

    BOOL
    CopyToOffset(
        WCHAR * szStringW,
        DWORD   cchStringW,
        DWORD   cchOffset
        );

    BOOL
    CopyAToOffset(
        CHAR *  szString,
        DWORD   cchString,
        DWORD   cchOffset
        );
};

#endif  // _isapistringw_h
