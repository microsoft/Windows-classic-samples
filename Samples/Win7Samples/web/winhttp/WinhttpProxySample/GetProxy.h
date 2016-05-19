//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
//    Sample WinHttp application for determining the proxy for a particular URL.
//    This sample demonstrates the core functionality for querying the proxy
//    settings.  Additional features may be added by the application/module basing
//    their proxy code from this sample, including but not limited to:
//        1) Per URL proxy cache.
//        2) Network Change awareness.
//        3) Bad Proxy Filter.
//

#pragma once

#include <winhttp.h>

#define USER_AGENT L"ProxySample"

#if defined(DBG) || defined(_DEBUG) || defined(DEBUG)
#define ASSERT(x) {if (!(x)) {DebugBreak();}}
#else
#define ASSERT(x)
#endif

class ProxyResolver
{
    //
    // m_fInit - The proxy has been resolved.
    //

    BOOL m_fInit;

    //
    // m_fReturnedFirstProxy - The first proxy in the list has been returned
    //                         to the application.
    //

    BOOL m_fReturnedFirstProxy;

    //
    // m_fReturnedLastProxy - The end of the proxy list was reached.
    //

    BOOL m_fReturnedLastProxy;

    //
    // m_wpiProxyInfo - The initial proxy and bypass list returned by
    //                  calls to WinHttpGetIEProxyConfigForCurrentUser
    //                  and WinHttpGetProxyForUrl.
    //

    WINHTTP_PROXY_INFO m_wpiProxyInfo;

    //
    // m_fProxyFailOverValid - Indicates whether it is valid to iterate through
    //                         the proxy list for proxy failover.  Proxy
    //                         failover is valid for a list of proxies
    //                         returned by executing a proxy script.  This
    //                         occurs in auto-detect and auto-config URL proxy
    //                         detection modes.  When static proxy settings
    //                         are used fallback is not allowed.
    //

    BOOL m_fProxyFailOverValid;

    //
    // pwszCursor - The current location in the proxy list
    //

    PWSTR m_pwszProxyCursor;

    BOOL
    IsWhitespace(WCHAR wcChar);

    BOOL
    IsRecoverableAutoProxyError(
        __in DWORD dwError);

    BOOL
    IsErrorValidForProxyFailover(
        __in DWORD dwError);

    DWORD
    WinHttpGetProxyForUrlWrapper(
        __in HINTERNET hSession,
        __in_z PCWSTR pwszUrl,
        __in WINHTTP_AUTOPROXY_OPTIONS *pwaoOptions,
        __out PWSTR *ppwszProxy,
        __out PWSTR *ppwszProxyBypass);

    DWORD
    GetProxyForAutoSettings(
        __in HINTERNET hSession,
        __in_z PCWSTR pwszUrl,
        __in_z_opt PCWSTR pwszAutoConfigUrl,
        __out PWSTR *ppwszProxy,
        __out PWSTR *ppwszProxyBypass);

public:

    ProxyResolver();

    ~ProxyResolver();

    DWORD
    ResolveProxy(
            __in HINTERNET hSession,
            __in_z PCWSTR pwszUrl);

    VOID
    ResetProxyCursor();

    VOID
    PrintProxyData();

    DWORD
    SetNextProxySetting(
        __in HINTERNET hInternet,
        __in DWORD dwRequestError);
};
