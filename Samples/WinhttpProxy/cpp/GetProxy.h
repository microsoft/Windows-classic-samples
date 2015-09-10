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

typedef DWORD (WINAPI *PFNWINHTTPGETPROXYFORURLEX)    (HINTERNET, PCWSTR, WINHTTP_AUTOPROXY_OPTIONS *, DWORD_PTR);
typedef DWORD (WINAPI *PFNWINHTTPFREEPROXYLIST)       (WINHTTP_PROXY_RESULT *);
typedef DWORD (WINAPI *PFNWINHTTPCREATEPROXYRESOLVER) (HINTERNET, HINTERNET *);
typedef DWORD (WINAPI *PFNWINHTTPGETPROXYRESULT)      (HINTERNET, WINHTTP_PROXY_RESULT *);

#define USER_AGENT L"ProxySample"

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
    // m_fExtendedAPI - Indicates whether extended APIs are used.
    //

    BOOL m_fExtendedAPI;

    //
    // m_dwError - WIN32 Error codes returned from call back function. It's used
    //             by extended APIs.
    //

    DWORD m_dwError;

    //
    // m_hEvent - The handle to the event object. It's used after calling
    //            WinHttpGetProxyForUrlEx to wait for proxy results.
    //

    HANDLE m_hEvent;

    //
    // m_wpiProxyInfo - The initial proxy and bypass list returned by
    //                  calls to WinHttpGetIEProxyConfigForCurrentUser
    //                  and WinHttpGetProxyForUrl.
    //

    WINHTTP_PROXY_INFO m_wpiProxyInfo;

    //
    // m_wprProxyResult - The structure introduced for extended APIs.
    //                    It contains well-structured proxy results.
    //                    Used when 1) Auto-Detect if configured.
    //                              2) Auto-Config URL if configured.
    //

    WINHTTP_PROXY_RESULT m_wprProxyResult;

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
    // m_pwszProxyCursor - The current location in the m_wpiProxyInfo proxy list.
    //                     Activated when WinHttpGetProxyForUrl is used.
    //

    PWSTR m_pwszProxyCursor;

    //
    // m_dwProxyCursor - The current location in the m_wprProxyResult proxy list.
    //                   Activated when WinHttpGetProxyForUrlEx is used.
    //

    DWORD m_dwProxyCursor;

    BOOL
    IsWhitespace(
        _In_ WCHAR wcChar);

    BOOL
    IsRecoverableAutoProxyError(
        _In_ DWORD dwError);

    BOOL
    IsErrorValidForProxyFailover(
        _In_ DWORD dwError);

    DWORD
    SetNextProxySettingEx(
        _In_ HINTERNET hInternet,
        _In_ DWORD dwRequestError);

    VOID
    static
    CALLBACK
    GetProxyCallBack(
        _In_  HINTERNET hResolver,
        _In_  DWORD_PTR dwContext,
        _In_  DWORD dwInternetStatus,
        _In_  PVOID pvStatusInformation,
        _In_  DWORD dwStatusInformationLength);

    DWORD
    GetProxyForUrlEx(
        _In_ HINTERNET hSession,
        _In_z_ PCWSTR pwszUrl,
        _In_ WINHTTP_AUTOPROXY_OPTIONS *pAutoProxyOptions);

    _Success_(return == ERROR_SUCCESS)
    DWORD
    GetProxyForAutoSettings(
        _In_ HINTERNET hSession,
        _In_z_ PCWSTR pwszUrl,
        _In_opt_z_ PCWSTR pwszAutoConfigUrl,
        _Outptr_result_maybenull_ PWSTR *ppwszProxy,
        _Outptr_result_maybenull_ PWSTR *ppwszProxyBypass);

    //
    // s_pfnWinhttpGetProxyForUrlEx - Function pointer to the new extended api
    //                                WinHttpGetProxyForUrlEx
    //

    static PFNWINHTTPGETPROXYFORURLEX s_pfnWinhttpGetProxyForUrlEx;

    //
    // s_pfnWinhttpFreeProxyList - Function pointer to the new extended api
    //                             WinHttpFreeProxyResult
    //

    static PFNWINHTTPFREEPROXYLIST s_pfnWinhttpFreeProxyList;

    //
    // s_pfnWinhttpCreateProxyResolver - Function pointer to the new extended api
    //                                   WinHttpCreateProxyResolver
    //

    static PFNWINHTTPCREATEPROXYRESOLVER s_pfnWinhttpCreateProxyResolver;

    //
    // s_pfnWinhttpGetProxyResult - Function pointer to the new extended api
    //                              WinHttpGetProxyResult
    //

    static PFNWINHTTPGETPROXYRESULT s_pfnWinhttpGetProxyResult;

public:

    ProxyResolver();

    ~ProxyResolver();

    DWORD
    ResolveProxy(
            _In_ HINTERNET hSession,
            _In_z_ PCWSTR pwszUrl);

    VOID
    ResetProxyCursor();

    DWORD
    SetNextProxySetting(
        _In_ HINTERNET hInternet,
        _In_ DWORD dwRequestError);
};
