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


#include <windows.h>
#include <Strsafe.h>
#include <stdlib.h>
#include <stdio.h>
#include "GetProxy.h"

PFNWINHTTPGETPROXYFORURLEX ProxyResolver::s_pfnWinhttpGetProxyForUrlEx = NULL;
PFNWINHTTPFREEPROXYLIST ProxyResolver::s_pfnWinhttpFreeProxyList = NULL;
PFNWINHTTPCREATEPROXYRESOLVER ProxyResolver::s_pfnWinhttpCreateProxyResolver = NULL;
PFNWINHTTPGETPROXYRESULT ProxyResolver::s_pfnWinhttpGetProxyResult = NULL;

ProxyResolver::ProxyResolver()
{
    HMODULE hWinhttp = NULL;

    m_fInit = FALSE;
    m_fReturnedFirstProxy = FALSE;
    m_fReturnedLastProxy = FALSE;
    m_fProxyFailOverValid = FALSE;

    m_hEvent = NULL;
    m_pwszProxyCursor = NULL;
    m_dwProxyCursor = 0;
    m_dwError = ERROR_SUCCESS;

    ZeroMemory(&m_wprProxyResult, sizeof(WINHTTP_PROXY_INFO));
    ZeroMemory(&m_wpiProxyInfo, sizeof(WINHTTP_PROXY_INFO));

    //
    // Maintain backward compatibility by only enabling the extended APIs when
    // they are available in winhttp.dll.
    //

    hWinhttp = GetModuleHandle(L"winhttp.dll");
    if (hWinhttp != NULL)
    {
        s_pfnWinhttpGetProxyForUrlEx = (PFNWINHTTPGETPROXYFORURLEX)GetProcAddress(hWinhttp, "WinHttpGetProxyForUrlEx");
        s_pfnWinhttpFreeProxyList = (PFNWINHTTPFREEPROXYLIST)GetProcAddress(hWinhttp, "WinHttpFreeProxyResult");
        s_pfnWinhttpCreateProxyResolver = (PFNWINHTTPCREATEPROXYRESOLVER)GetProcAddress(hWinhttp, "WinHttpCreateProxyResolver");
        s_pfnWinhttpGetProxyResult = (PFNWINHTTPGETPROXYRESULT)GetProcAddress(hWinhttp, "WinHttpGetProxyResult");
    }

    m_fExtendedAPI = s_pfnWinhttpGetProxyForUrlEx != NULL &&
                     s_pfnWinhttpFreeProxyList != NULL &&
                     s_pfnWinhttpCreateProxyResolver != NULL &&
                     s_pfnWinhttpGetProxyResult != NULL;
}

ProxyResolver::~ProxyResolver()
{
    if (m_wpiProxyInfo.lpszProxy != NULL)
    {
        GlobalFree(m_wpiProxyInfo.lpszProxy);
        m_wpiProxyInfo.lpszProxy = NULL;
    }

    if (m_wpiProxyInfo.lpszProxyBypass != NULL)
    {
        GlobalFree(m_wpiProxyInfo.lpszProxyBypass);
        m_wpiProxyInfo.lpszProxyBypass = NULL;
    }

    ZeroMemory(&m_wpiProxyInfo, sizeof(WINHTTP_PROXY_INFO));

    if (m_fExtendedAPI)
    {
        //
        // When extended APIs are used, m_wprProxyResult will be freed by using
        // the new API WinHttpFreeProxyResult.
        //

        s_pfnWinhttpFreeProxyList(&m_wprProxyResult);
        ZeroMemory(&m_wprProxyResult, sizeof(WINHTTP_PROXY_INFO));
    }

    if (m_hEvent != NULL)
    {
        CloseHandle(m_hEvent);
        m_hEvent = NULL;
    }

    m_wpiProxyInfo.dwAccessType = WINHTTP_ACCESS_TYPE_DEFAULT_PROXY;

    m_fInit = FALSE;
    m_fReturnedFirstProxy = FALSE;
    m_fReturnedLastProxy = FALSE;
    m_fProxyFailOverValid = FALSE;

    m_pwszProxyCursor = NULL;
    m_dwProxyCursor = 0;
    m_dwError = ERROR_SUCCESS;
}

BOOL
ProxyResolver::IsWhitespace(
   _In_ WCHAR wcChar
)
/*++

Routine Description:

   Determines if a wide character is a whitespace character.

Arguments:

    wcChar - The character to check for whitespace.

Return Value:

    TRUE if the character is whitespace. FALSE otherwise.

--*/
{
    BOOL fResults = FALSE;

    //
    // Check for ' '.
    //

    if (wcChar == L' ')
    {
        fResults = TRUE;
        goto quit;
    }

    //
    // Check for \t\n\v\f\r.
    //

    if (wcChar >= L'\t' &&
        wcChar <= L'\r')
    {
        fResults = TRUE;
        goto quit;
    }

quit:

    return fResults;
}

BOOL
ProxyResolver::IsRecoverableAutoProxyError(
    _In_ DWORD dwError
)
/*++

Routine Description:

    Determines whether the result of WinHttpGetProxyForUrl is recoverable,
    allowing the caller to fall back to other proxy mechanisms.

Arguments:

    dwError - The Win32 error code returned by GetLastError on
              WinHttpGetProxyForUrl failure.

Return Value:

    TRUE - The caller can continue execution safely.

    FALSE - The caller should immediately fail with dwError.

--*/
{
    BOOL fRecoverable = FALSE;

    switch (dwError)
    {
    case ERROR_SUCCESS:
    case ERROR_INVALID_PARAMETER:
    case ERROR_WINHTTP_AUTO_PROXY_SERVICE_ERROR:
    case ERROR_WINHTTP_AUTODETECTION_FAILED:
    case ERROR_WINHTTP_BAD_AUTO_PROXY_SCRIPT:
    case ERROR_WINHTTP_LOGIN_FAILURE:
    case ERROR_WINHTTP_OPERATION_CANCELLED:
    case ERROR_WINHTTP_TIMEOUT:
    case ERROR_WINHTTP_UNABLE_TO_DOWNLOAD_SCRIPT:
    case ERROR_WINHTTP_UNRECOGNIZED_SCHEME:
        fRecoverable = TRUE;
        break;

    default:
        break;
    }

    return fRecoverable;
}

BOOL
ProxyResolver::IsErrorValidForProxyFailover(
    _In_ DWORD dwError
)
/*++

Routine Description:

    Determines whether the result of WinHttpSendRequest (Sync) or the error
    from WINHTTP_CALLBACK_STATUS_REQUEST_ERROR (Async) can assume a possible
    proxy error and fallback to the next proxy.

Arguments:

    dwError - The Win32 error code from WinHttpSendRequest (Sync) or from
              WINHTTP_CALLBACK_STATUS_REQUEST_ERROR (Async)

Return Value:

    TRUE - The caller should set the next proxy and resend the request.

    FALSE - The caller should assume the request has failed.

--*/
{
    BOOL fValid = FALSE;

    switch(dwError)
    {
    case ERROR_WINHTTP_NAME_NOT_RESOLVED:
    case ERROR_WINHTTP_CANNOT_CONNECT:
    case ERROR_WINHTTP_CONNECTION_ERROR:
    case ERROR_WINHTTP_TIMEOUT:
        fValid = TRUE;
        break;

    default:
        break;
    }

    return fValid;
}

VOID
ProxyResolver::ResetProxyCursor()
/*++

Routine Description:

    Resets the proxy cursor for reuse starting at the beginning of the list.

Arguments:

Return Value:

    None.

--*/
{
    m_fReturnedFirstProxy = FALSE;
    m_fReturnedLastProxy = FALSE;
    m_pwszProxyCursor = NULL;
    m_dwProxyCursor = 0;
}

DWORD
ProxyResolver::SetNextProxySettingEx(
    _In_ HINTERNET hInternet,
    _In_ DWORD dwRequestError
)
/*++

Routine Description:

    Finds the next proxy from m_wprProxyResult queried from extended API.
    It is not safe to use this function concurrently.

    Each sequential request to the same URL should use ResetProxyCursor
    before the first call for proxy settings during a single request.

Arguments:

    hInternet - The Session or Request handle to set the proxy info on.

    dwRequestError - The Win32 error code from WinHttpSendRequest (Sync) or from
                     WINHTTP_CALLBACK_STATUS_REQUEST_ERROR (Async) or
                     ERROR_SUCCESS if this is the first usage.
Return Value:

    Win32 Errors Codes.

--*/
{
    DWORD dwError = ERROR_SUCCESS;

    //
    // Use static proxy settings if it's activated.
    //

    if (!m_fProxyFailOverValid)
    {
        if (m_fReturnedFirstProxy)
        {
            dwError = ERROR_NO_MORE_ITEMS;
            goto quit;
        }

        m_fReturnedFirstProxy = TRUE;

        if (!WinHttpSetOption(hInternet,
                              WINHTTP_OPTION_PROXY,
                              &m_wpiProxyInfo,
                              sizeof(m_wpiProxyInfo)))
        {
            dwError = GetLastError();
            goto quit;
        }

        goto quit;
    }

    if (m_dwProxyCursor >= m_wprProxyResult.cEntries)
    {
        dwError = ERROR_NO_MORE_ITEMS;
        goto quit;
    }

    //
    // The first proxy is always valid. Only check request errors after first run.
    //

    if (m_dwProxyCursor != 0 &&
        !IsErrorValidForProxyFailover(dwRequestError))
    {
        dwError = ERROR_NO_MORE_ITEMS;
        goto quit;
    }

    if (!WinHttpSetOption(hInternet,
                          WINHTTP_OPTION_PROXY_RESULT_ENTRY,
                          &m_wprProxyResult.pEntries[m_dwProxyCursor],
                          sizeof(*m_wprProxyResult.pEntries)))
    {
        dwError = GetLastError();
        goto quit;
    }

    m_dwProxyCursor++;

quit:

    return dwError;
}

DWORD
ProxyResolver::SetNextProxySetting(
    _In_ HINTERNET hInternet,
    _In_ DWORD dwRequestError
)
/*++

Routine Description:

    Finds the next proxy in a list of proxies separated by whitespace and/or
    semicolons if proxy failover is supported.  It is not safe to use this
    function concurrently, implement a concurrency mechanism for proxy lists
    if needed, such as making a copy or a separate iterator.

    Each sequential request to the same URL should use ResetProxyCursor
    before the first call for proxy settings during a single request.

Arguments:

    hInternet - The Session or Request handle to set the proxy info on.

    dwRequestError - The Win32 error code from WinHttpSendRequest (Sync) or from
                     WINHTTP_CALLBACK_STATUS_REQUEST_ERROR (Async) or
                     ERROR_SUCCESS if this is the first usage.

Return Value:

    ERROR_SUCCESS - Found the next proxy and it has been set on the HINTERNET.

    ERROR_NO_MORE_ITEMS - Reached the end of the list or failover not valid.

    ERROR_INVALID_OPERATION - The class is not initialized.  Call ResolveProxy first.

    Other Win32 Errors returned from WinHttpSetOption.

--*/
{
    DWORD dwError = ERROR_SUCCESS;
    PWSTR pwszCursor = NULL;
    WINHTTP_PROXY_INFO NextProxyInfo = {};

    if (!m_fInit)
    {
        dwError = ERROR_INVALID_OPERATION;
        goto quit;
    }

    if (m_fExtendedAPI)
    {
        dwError = SetNextProxySettingEx(hInternet,
                                        dwRequestError);
        goto quit;
    }

    if (!m_fReturnedFirstProxy)
    {
        //
        //  We have yet to set the first proxy type, the first one is always
        //  valid.
        //

        pwszCursor = m_wpiProxyInfo.lpszProxy;
        m_fReturnedFirstProxy = TRUE;
        goto commit;
    }

    //
    // Find the next proxy in the list if it is valid to do so.
    //

    if (m_fReturnedLastProxy ||
        !m_fProxyFailOverValid ||
        m_wpiProxyInfo.lpszProxy == NULL)
    {
        //
        // Already reached end, failover not valid, or type is not proxy.
        //

        dwError = ERROR_NO_MORE_ITEMS;
        goto quit;
    }

    if (!IsErrorValidForProxyFailover(dwRequestError))
    {
        dwError = ERROR_NO_MORE_ITEMS;
        goto quit;
    }

    pwszCursor = m_pwszProxyCursor;

    //
    // Skip the current entry.
    //

    while (*pwszCursor != L'\0' &&
           *pwszCursor != L';' &&
           !IsWhitespace(*pwszCursor))
    {
        pwszCursor++;
    }

    //
    // Skip any additional separators.
    //

    while (*pwszCursor != L'\0' &&
           (*pwszCursor == L';' ||
           IsWhitespace(*pwszCursor)))
    {
        pwszCursor++;
    }

    if (*pwszCursor == L'\0')
    {
        //
        // Hit the end of the list.
        //

        m_fReturnedLastProxy = TRUE;
        dwError = ERROR_NO_MORE_ITEMS;
        goto quit;
    }

commit:
    NextProxyInfo.dwAccessType = m_wpiProxyInfo.dwAccessType;
    NextProxyInfo.lpszProxy = pwszCursor;
    NextProxyInfo.lpszProxyBypass = m_wpiProxyInfo.lpszProxyBypass;

    if (!WinHttpSetOption(hInternet,
                          WINHTTP_OPTION_PROXY,
                          &NextProxyInfo,
                          sizeof(NextProxyInfo)))
    {
        dwError = GetLastError();
        goto quit;
    }

    m_pwszProxyCursor = pwszCursor;

quit:

    return dwError;
}

VOID
CALLBACK
ProxyResolver::GetProxyCallBack(
    _In_  HINTERNET hResolver,
    _In_  DWORD_PTR dwContext,
    _In_  DWORD dwInternetStatus,
    _In_  PVOID pvStatusInformation,
    _In_  DWORD dwStatusInformationLength
)
/*++

Routine Description:

    Fetch proxy query results asynchronizely. This application shows how to cope
    with new APIs. In multithreaded environment, developers must keep in mind resource
    contention.

Arguments:

    hSession - The WinHttp session to use for the proxy resolution.

    dwContext - The context value supplied by this application to associate with
                the callback handle hSession.

    dwInternetStatus - The INTERNET_STATUS_ value which specifies the status code
                       that indicates why the callback function is called.

    pvStatusInformation - A pointer to a buffer that specifies information
                          pertinent to this call to the callback function.

    dwStatusInformationLength - A value of type unsigned long integer that
                                specifies the size of the lpvStatusInformation buffer.

Return Value:

    None.

--*/
{
    ProxyResolver* pProxyResolver = NULL;
    WINHTTP_ASYNC_RESULT *pAsyncResult = NULL;

    UNREFERENCED_PARAMETER(dwStatusInformationLength);

    pProxyResolver = (ProxyResolver *)dwContext;

    if ((dwInternetStatus != WINHTTP_CALLBACK_STATUS_GETPROXYFORURL_COMPLETE &&
        dwInternetStatus != WINHTTP_CALLBACK_STATUS_REQUEST_ERROR) ||
        pProxyResolver == NULL)
    {
        goto quit;
    }

    if (dwInternetStatus == WINHTTP_CALLBACK_STATUS_REQUEST_ERROR)
    {
        pAsyncResult = (WINHTTP_ASYNC_RESULT *)pvStatusInformation;

        if (pAsyncResult->dwResult != API_GET_PROXY_FOR_URL)
        {
            goto quit;
        }

        pProxyResolver->m_dwError = pAsyncResult->dwError;
    }
    else if (dwInternetStatus == WINHTTP_CALLBACK_STATUS_GETPROXYFORURL_COMPLETE)
    {
        pProxyResolver->m_dwError = s_pfnWinhttpGetProxyResult(hResolver,
                                                               &pProxyResolver->m_wprProxyResult);
    }

    if (hResolver != NULL)
    {
        WinHttpCloseHandle(hResolver);
        hResolver = NULL;
    }

    SetEvent(pProxyResolver->m_hEvent);

quit:
    return;
}

DWORD
ProxyResolver::GetProxyForUrlEx(
    _In_ HINTERNET hSession,
    _In_z_ PCWSTR pwszUrl,
    _In_ WINHTTP_AUTOPROXY_OPTIONS *pAutoProxyOptions
)
/*++

Routine Description:

    Retrieves the proxy data with the specified option using WinhttpGetProxyForUrlEx.

Arguments:

    hSession - The WinHttp session to use for the proxy resolution.

    pwszUrl - The URL to get the proxy for.

    pAutoProxyOptions - Specifies the auto-proxy options to use.

Return Value:

    WIN32 Error codes.

--*/
{
    DWORD dwError = ERROR_SUCCESS;
    HINTERNET hResolver = NULL;
    WINHTTP_STATUS_CALLBACK wscCallback = NULL;

    //
    // Create proxy resolver handle. It's best to close the handle during call back.
    //

    dwError = s_pfnWinhttpCreateProxyResolver(hSession,
                                              &hResolver);
    if (dwError != ERROR_SUCCESS)
    {
        goto quit;
    }

    //
    // Sets up a callback function that WinHTTP can call as proxy results are resolved.
    //

    wscCallback = WinHttpSetStatusCallback(hResolver,
                                           GetProxyCallBack,
                                           WINHTTP_CALLBACK_FLAG_REQUEST_ERROR | WINHTTP_CALLBACK_FLAG_GETPROXYFORURL_COMPLETE,
                                           0);
    if(wscCallback == WINHTTP_INVALID_STATUS_CALLBACK)
    {
        dwError = GetLastError();
        goto quit;
    }

    //
    // The extended API works in asynchronous mode, therefore wait until the
    // results are set in the call back function.
    //

    dwError = s_pfnWinhttpGetProxyForUrlEx(hResolver,
                                           pwszUrl,
                                           pAutoProxyOptions,
                                           (DWORD_PTR)this);
    if (dwError != ERROR_IO_PENDING)
    {
        goto quit;
    }

    //
    // The resolver handle will get closed in the callback and cannot be used any longer.
    //

    hResolver = NULL;

    dwError = WaitForSingleObjectEx(m_hEvent,
                                    INFINITE,
                                    FALSE);
    if (dwError != WAIT_OBJECT_0)
    {
        dwError = GetLastError();
        goto quit;
    }

    dwError = m_dwError;

quit:

    if (hResolver != NULL)
    {
        WinHttpCloseHandle(hResolver);
        hResolver = NULL;
    }

    return dwError;
}

_Success_(return == ERROR_SUCCESS)
DWORD
ProxyResolver::GetProxyForAutoSettings(
    _In_ HINTERNET hSession,
    _In_z_ PCWSTR pwszUrl,
    _In_opt_z_ PCWSTR pwszAutoConfigUrl,
    _Outptr_result_maybenull_ PWSTR *ppwszProxy,
    _Outptr_result_maybenull_ PWSTR *ppwszProxyBypass
)
/*++

Routine Description:

    Uses Auto detection or AutoConfigURL to run WinHttpGetProxyForUrl.

    Additionally provides autologon by calling once without autologon, which is
    most performant, and then with autologon if logon fails.

Arguments:

    hSession - The WinHttp session to use for the proxy resolution.

    pwszUrl - The URL to get the proxy for.

    pwszAutoConfig - The autoconfig URL or NULL for Autodetection.

    ppwszProxy - Upon success, the proxy string found for pwszUrl or NULL if
                 no proxy should be used for this URL.
                 Use GlobalFree to free.

    ppwszProxyBypass - Upon success, the proxy bypass string found for pwszUrl
                       or NULL if there is no proxy bypass for the
                       configuration type.
                       Use GlobalFree to free.
Return Value:

    WIN32 Error codes.  The caller should use IsRecoverableAutoProxyError to
        decide whether execution can continue.

--*/
{
    DWORD dwError = ERROR_SUCCESS;
    WINHTTP_AUTOPROXY_OPTIONS waoOptions = {};
    WINHTTP_PROXY_INFO wpiProxyInfo = {};

    *ppwszProxy = NULL;
    *ppwszProxyBypass = NULL;

    if (pwszAutoConfigUrl)
    {
        waoOptions.dwFlags = WINHTTP_AUTOPROXY_CONFIG_URL;
        waoOptions.lpszAutoConfigUrl = pwszAutoConfigUrl;
    }
    else
    {
        waoOptions.dwFlags = WINHTTP_AUTOPROXY_AUTO_DETECT;
        waoOptions.dwAutoDetectFlags = WINHTTP_AUTO_DETECT_TYPE_DHCP | WINHTTP_AUTO_DETECT_TYPE_DNS_A;
    }

    //
    // First call with no autologon.  Autologon prevents the
    // session (in proc) or autoproxy service (out of proc) from caching
    // the proxy script.  This causes repetitive network traffic, so it is
    // best not to do autologon unless it is required according to the
    // result of WinHttpGetProxyForUrl.
    // This applies to both WinHttpGetProxyForUrl and WinhttpGetProxyForUrlEx.
    //

    if (m_fExtendedAPI)
    {
        m_hEvent = CreateEventEx(NULL,
                                 NULL,
                                 0,
                                 EVENT_ALL_ACCESS);
        if (m_hEvent == NULL)
        {
            dwError = GetLastError();
            goto quit;
        }

        dwError = GetProxyForUrlEx(hSession,
                                   pwszUrl,
                                   &waoOptions);

        if (dwError != ERROR_WINHTTP_LOGIN_FAILURE)
        {
            //
            // Unless we need to retry with auto-logon exit the function with the
            // result, on success the proxy list will be stored in m_wprProxyResult
            // by GetProxyCallBack.
            //

            goto quit;
        }

        //
        // Enable autologon if challenged.
        //

        waoOptions.fAutoLogonIfChallenged = TRUE;
        dwError = GetProxyForUrlEx(hSession,
                                   pwszUrl,
                                   &waoOptions);

        goto quit;
    }

    if (!WinHttpGetProxyForUrl(hSession,
                               pwszUrl,
                               &waoOptions,
                               &wpiProxyInfo))
    {
        dwError = GetLastError();

        if (dwError != ERROR_WINHTTP_LOGIN_FAILURE)
        {
            goto quit;
        }

        //
        // Enable autologon if challenged.
        //

        dwError = ERROR_SUCCESS;
        waoOptions.fAutoLogonIfChallenged = TRUE;
        if (!WinHttpGetProxyForUrl(hSession,
                                   pwszUrl,
                                   &waoOptions,
                                   &wpiProxyInfo))
        {
            dwError = GetLastError();
            goto quit;
        }
    }

    *ppwszProxy = wpiProxyInfo.lpszProxy;
    wpiProxyInfo.lpszProxy = NULL;

    *ppwszProxyBypass = wpiProxyInfo.lpszProxyBypass;
    wpiProxyInfo.lpszProxyBypass = NULL;

quit:

    if (wpiProxyInfo.lpszProxy)
    {
        GlobalFree(wpiProxyInfo.lpszProxy);
        wpiProxyInfo.lpszProxy = NULL;
    }

    if (wpiProxyInfo.lpszProxyBypass)
    {
        GlobalFree(wpiProxyInfo.lpszProxyBypass);
        wpiProxyInfo.lpszProxyBypass = NULL;
    }

    return dwError;
}

DWORD
ProxyResolver::ResolveProxy(
    _In_ HINTERNET hSession,
    _In_z_ PCWSTR pwszUrl
)
/*++

Routine Description:

    Uses the users IE settings to get the proxy for the URL.

Arguments:

    pwszUrl - The URL to get the proxy for.

    hSession - The session to use for the proxy resolution.

Return Value:

    WIN32 Error codes.

--*/
{
    DWORD dwError = ERROR_SUCCESS;
    WINHTTP_CURRENT_USER_IE_PROXY_CONFIG ProxyConfig = {};
    PWSTR pwszProxy = NULL;
    PWSTR pwszProxyBypass = NULL;
    BOOL fFailOverValid = FALSE;

    if (m_fInit)
    {
        dwError = ERROR_INVALID_OPERATION;
        goto quit;
    }

    if (!WinHttpGetIEProxyConfigForCurrentUser(&ProxyConfig))
    {
        dwError = GetLastError();
        if (dwError != ERROR_FILE_NOT_FOUND)
        {
            goto quit;
        }

        //
        // No IE proxy settings found, just do autodetect.
        //

        ProxyConfig.fAutoDetect = TRUE;
        dwError = ERROR_SUCCESS;
    }

    //
    // Begin processing the proxy settings in the following order:
    //  1) Auto-Detect if configured.
    //  2) Auto-Config URL if configured.
    //  3) Static Proxy Settings if configured.
    //
    // Once any of these methods succeed in finding a proxy we are finished.
    // In the event one mechanism fails with an expected error code it is
    // required to fall back to the next mechanism.  If the request fails
    // after exhausting all detected proxies, there should be no attempt
    // to discover additional proxies.
    //

    if (ProxyConfig.fAutoDetect)
    {
        fFailOverValid = TRUE;

        //
        // Detect Proxy Settings.
        //

        dwError = GetProxyForAutoSettings(hSession,
                                          pwszUrl,
                                          NULL,
                                          &pwszProxy,
                                          &pwszProxyBypass);

        if (dwError == ERROR_SUCCESS)
        {
            goto commit;
        }

        if (!IsRecoverableAutoProxyError(dwError))
        {
            goto quit;
        }

        //
        // Fall back to Autoconfig URL or Static settings.  Application can
        // optionally take some action such as logging, or creating a mechanism
        // to expose multiple error codes in the class.
        //

        dwError = ERROR_SUCCESS;
    }

    if (ProxyConfig.lpszAutoConfigUrl)
    {
        fFailOverValid = TRUE;

        //
        // Run autoproxy with AutoConfig URL.
        //

        dwError = GetProxyForAutoSettings(hSession,
                                          pwszUrl,
                                          ProxyConfig.lpszAutoConfigUrl,
                                          &pwszProxy,
                                          &pwszProxyBypass);
        if (dwError == ERROR_SUCCESS)
        {
            goto commit;
        }

        if (!IsRecoverableAutoProxyError(dwError))
        {
            goto quit;
        }

        //
        // Fall back to Static Settings.  Application can optionally take some
        // action such as logging, or creating a mechanism to to expose multiple
        // error codes in the class.
        //

        dwError = ERROR_SUCCESS;
    }

    fFailOverValid = FALSE;

    //
    // Static Proxy Config.  Failover is not valid for static proxy since
    // it is always either a single proxy or a list containing protocol
    // specific proxies such as "proxy" or http=httpproxy;https=sslproxy
    //

    pwszProxy = ProxyConfig.lpszProxy;
    ProxyConfig.lpszProxy = NULL;

    pwszProxyBypass = ProxyConfig.lpszProxyBypass;
    ProxyConfig.lpszProxyBypass = NULL;

commit:

    m_fProxyFailOverValid = fFailOverValid;

    if (pwszProxy == NULL)
    {
        m_wpiProxyInfo.dwAccessType = WINHTTP_ACCESS_TYPE_NO_PROXY;
    }
    else
    {
        m_wpiProxyInfo.dwAccessType = WINHTTP_ACCESS_TYPE_NAMED_PROXY;
    }

    m_wpiProxyInfo.lpszProxy = pwszProxy;
    pwszProxy = NULL;

    m_wpiProxyInfo.lpszProxyBypass = pwszProxyBypass;
    pwszProxyBypass = NULL;

    m_fInit = TRUE;

quit:

    if (pwszProxy != NULL)
    {
        GlobalFree(pwszProxy);
        pwszProxy = NULL;
    }

    if (pwszProxyBypass != NULL)
    {
        GlobalFree(pwszProxyBypass);
        pwszProxyBypass = NULL;
    }

    if (ProxyConfig.lpszAutoConfigUrl != NULL)
    {
        GlobalFree(ProxyConfig.lpszAutoConfigUrl);
        ProxyConfig.lpszAutoConfigUrl = NULL;
    }

    if (ProxyConfig.lpszProxy != NULL)
    {
        GlobalFree(ProxyConfig.lpszProxy);
        ProxyConfig.lpszProxy = NULL;
    }

    if (ProxyConfig.lpszProxyBypass != NULL)
    {
        GlobalFree(ProxyConfig.lpszProxyBypass);
        ProxyConfig.lpszProxyBypass = NULL;
    }

    return dwError;
}
