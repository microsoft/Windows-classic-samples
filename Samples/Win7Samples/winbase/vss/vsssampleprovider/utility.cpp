/*--

Copyright (C) Microsoft Corporation, 2003

Module Name:

    Utility.cpp

Abstract:

    Various utility functions

Notes:

Revision History:

--*/

#include "stdafx.h"
#include "rpcdce.h"
#include <strsafe.h>

LPSTR
NewString(
    LPCSTR pszSource
    )
{
    LPSTR pszDest = NULL;
    
    if (pszSource) {
        size_t len = (strlen( pszSource ) + 1) * sizeof *pszSource;
    
        pszDest = static_cast<LPSTR>( ::CoTaskMemAlloc( len ) );
        if (pszDest) {
            ::CopyMemory( pszDest, pszSource, len );
        } else {
            throw (HRESULT) E_OUTOFMEMORY;
        }
    }

    return pszDest;
}

LPWSTR
NewString(
    LPCWSTR pwszSource
    )
{
    LPWSTR pwszDest = NULL;

    if (pwszSource) {
        size_t len = (wcslen( pwszSource ) + 1) * sizeof *pwszSource;
    
        pwszDest = static_cast<LPWSTR>( ::CoTaskMemAlloc( len ) );
        if (pwszDest) {
            ::CopyMemory( pwszDest, pwszSource, len );
        } else {
            throw (HRESULT) E_OUTOFMEMORY;
        }
    }

    return pwszDest;
}

LPWSTR
NewString(
    std::wstring& wsSrc
    )
{
    LPWSTR pwszDest;
    size_t len = (wsSrc.length() + 1) * sizeof( wchar_t );
    
    pwszDest = static_cast<LPWSTR>( ::CoTaskMemAlloc( len ) );
    if (pwszDest) {
        ::CopyMemory( pwszDest, wsSrc.c_str(), len );
    } else {
        throw (HRESULT) E_OUTOFMEMORY;
    }

    return pwszDest;
}

std::string
GuidToString(
    GUID& guid
    )
{
    RPC_STATUS rs;
    unsigned char* s;
    std::string r;

    //
    // Why does this function take an unsigned char* instead of a char*?
    //
    rs = ::UuidToStringA( &guid, &s );
    if (rs == RPC_S_OK) {
        r = reinterpret_cast<char*> (s);
        ::RpcStringFreeA( &s );
    }
 
    return r;
}

std::wstring
GuidToWString(
    GUID& guid
    )
{
    RPC_STATUS rs;
    wchar_t* s;
    std::wstring r;

    rs = ::UuidToStringW( &guid, &s );
    if (rs == RPC_S_OK) {
        r = s;
        ::RpcStringFreeW( &s );
    }
 
    return r;
}

GUID
WStringToGuid(
    std::wstring& value
    )
{
    RPC_STATUS rs;
    wchar_t* s = const_cast<wchar_t*>( value.c_str() );
    GUID r;

    r = GUID_NULL;
    rs = ::UuidFromString( s, &r );
    return r;
}

//
// UnicodeToAnsi converts the Unicode string pszW to an ANSI string
// and returns the ANSI string through ppszA. Space for the
// the converted string is allocated by UnicodeToAnsi.
//

HRESULT
UnicodeToAnsi(
    __in LPCWSTR pwszIn,
    __out LPSTR&  pszOut
    )
{ 
    size_t cbAnsi, cCharacters;
    DWORD dwError;

    // If input is null then just return the same.
    if (pwszIn == NULL) {
        pszOut = NULL;
        return NOERROR;
    }

    cCharacters = wcslen(pwszIn)+1;

    // Determine number of bytes to be allocated for ANSI string. An
    // ANSI string can have at most 2 bytes per character (for Double
    // Byte Character Strings.)
    cbAnsi = cCharacters*2;

    // Use of the OLE allocator is not required because the resultant
    // ANSI  string will never be passed to another COM component. You
    // can use your own allocator.
    pszOut = (LPSTR) CoTaskMemAlloc(cbAnsi);
    if (pszOut == NULL)
        return E_OUTOFMEMORY;

    // Convert to ANSI.
    if (WideCharToMultiByte(CP_ACP, 0, pwszIn, static_cast<int>(cCharacters), pszOut, static_cast<int>(cbAnsi), NULL, NULL) == 0) {
        dwError = GetLastError();
        CoTaskMemFree(pszOut);
        pszOut = NULL;
        return HRESULT_FROM_WIN32(dwError);
    }

    return NOERROR;
} 

HRESULT
AnsiToUnicode(
    __in LPCSTR pszIn,
    __out LPWSTR& pwszOut
    )
{
    size_t cbAnsi, cbWide;
    DWORD dwError;

    // If input is null then just return the same.
    if (pszIn == NULL) {
        pwszOut = NULL;
        return NOERROR;
    }

    cbAnsi = strlen(pszIn) + 1;
    cbWide = cbAnsi * sizeof(*pwszOut);

    pwszOut = (LPWSTR) CoTaskMemAlloc(cbWide);
    if (pwszOut == NULL)
        return E_OUTOFMEMORY;

    // Convert to wide.
    if (MultiByteToWideChar(CP_ACP, 0, pszIn, static_cast<int>(cbAnsi), pwszOut, static_cast<int>(cbAnsi)) == 0) {
        dwError = GetLastError();
        CoTaskMemFree(pwszOut);
        pwszOut = NULL;
        return HRESULT_FROM_WIN32(dwError);
    }

    return NOERROR;
} 

HRESULT
AnsiToGuid(
    LPCSTR szString,
    GUID& gId
    )
{
    LPWSTR wszString = NULL;
    HRESULT hr = S_OK;
    char tmp[39];

    if (szString == NULL) {
        hr = E_INVALIDARG;
    }

    if (SUCCEEDED( hr )) {
        if (*szString != '{') {
            hr = StringCchPrintfA(tmp, 39, "{%s}", szString);
            if (hr == S_OK)
                szString = tmp;
        }
    }

    if (SUCCEEDED( hr )) {
        hr = AnsiToUnicode(szString, wszString);
    }

    if (SUCCEEDED( hr )) {
        hr = CLSIDFromString(wszString, &gId);
        CoTaskMemFree(wszString);
    }

    return hr;
}

LPSTR
GuidToAnsi(
    GUID& gId
    )
{
    WCHAR tmp[39];
    char *szRet;

    StringFromGUID2(gId, tmp, sizeof tmp / sizeof *tmp);
    UnicodeToAnsi(tmp, szRet);
    
    return szRet;
}

//
// Logging functions
//

// Logs a string to the kernel debugger
void
TraceMsg(
    LPCWSTR msg,
    ...
    )
{
    WCHAR buf[4096];
    va_list args;
    va_start( args, msg );
    HRESULT hr = StringCchVPrintf( buf, NELEMENTS( buf ), msg, args );
    va_end( args );

    if (hr == S_OK)
        OutputDebugStringW( buf );
}

void
LogEvent(
    LPCWSTR pFormat,
    ...
    )
{
    WCHAR    chMsg[256];
    HANDLE   hEventSource;
    LPCWSTR  lpszStrings[1];
    va_list  pArg;

    va_start(pArg, pFormat);
    HRESULT hr = StringCchVPrintf(chMsg, NELEMENTS( chMsg ), pFormat, pArg);
    va_end(pArg);

    if (hr == S_OK) {
        lpszStrings[0] = chMsg;

#pragma prefast(push)
#pragma prefast(disable:28735, "Code existing pre-Vista. New code w/o banned Event APIs is part of next RI.")

        /* Get a handle to use with ReportEvent(). */
        hEventSource = ::RegisterEventSource(NULL, L"VssSampleProvider");
        if (hEventSource != NULL) {
            /* Write to event log. */
            ::ReportEvent(hEventSource,
                          EVENTLOG_INFORMATION_TYPE,
                          0,
                          SIMHWPRV_EVENTLOG_INFO_GENERIC_MESSAGE,
                          NULL,
                          1,
                          0,
                          &lpszStrings[0],
                          NULL);
            ::DeregisterEventSource(hEventSource);
        }

#pragma prefast(pop)

    }
}

HRESULT
GetEnvVar(
    std::wstring& var,
    std::wstring& value
    )
{
    DWORD dr;
    DWORD dwCount;
    LPCWSTR name = var.c_str();

    for (;;) {
        dwCount = static_cast<DWORD>(value.capacity());
        value.resize( dwCount );
        dr = ::GetEnvironmentVariable( name, &value[0], dwCount );
        if (dr == 0) {
            return HRESULT_FROM_WIN32( GetLastError() );
        }
        if (dr >= dwCount) {
            value.reserve( value.capacity() + 100 );
            continue;
        }
        value.resize( dr );
        break;
    }

    return S_OK;
}
