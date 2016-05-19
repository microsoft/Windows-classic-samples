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
#include <stdio.h>
#include "GetProxy.h"

ProxyResolver::ProxyResolver()
{
    m_fInit = FALSE;
    m_fReturnedFirstProxy = FALSE;
    m_fReturnedLastProxy = FALSE;
    m_fProxyFailOverValid = FALSE;

    m_pwszProxyCursor = NULL;

    ZeroMemory(&m_wpiProxyInfo, sizeof(WINHTTP_PROXY_INFO));
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

    m_wpiProxyInfo.dwAccessType = WINHTTP_ACCESS_TYPE_DEFAULT_PROXY;

    m_fInit = FALSE;
    m_fReturnedFirstProxy = FALSE;
    m_fReturnedLastProxy = FALSE;
    m_fProxyFailOverValid = FALSE;

    m_pwszProxyCursor = NULL;
}

BOOL
ProxyResolver::IsWhitespace(WCHAR wcChar)
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

    if (wcChar == 32)
    {
        fResults = TRUE;
        goto quit;
    }

    //
    // Check for \t\n\v\f\r.
    //

    if (wcChar >= 9 &&
        wcChar <= 13)
    {
        fResults = TRUE;
        goto quit;
    }

quit:

    return fResults;
}

BOOL
ProxyResolver::IsRecoverableAutoProxyError(
    __in DWORD dwError
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
    __in DWORD dwError
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
}

VOID
ProxyResolver::PrintProxyData()
/*++

Routine Description:

    Prints the proxy information in for the resolver including proxy, bypass
    list, and if failover is allowed.

Arguments:

Return Value:

    None.

--*/
{
    wprintf(L"   Proxy: %s\n", m_wpiProxyInfo.lpszProxy);

    wprintf(L"   Bypass: %s\n", m_wpiProxyInfo.lpszProxyBypass);

    wprintf(L"   Failover Valid: %d\n\n", m_fProxyFailOverValid);
}

DWORD
ProxyResolver::SetNextProxySetting(
    __in HINTERNET hInternet,
    __in DWORD dwRequestError
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

    while(*pwszCursor != L'\0' &&
          *pwszCursor != L';' &&
          !IsWhitespace(*pwszCursor))
    {
        pwszCursor++;
    }

    //
    // Skip any additional separators.
    //

    while(*pwszCursor == L';' ||
          IsWhitespace(*pwszCursor))
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

    wprintf(L"Setting winhttp proxy to\n   Proxy: %s\n   Bypass: %s\n\n", NextProxyInfo.lpszProxy, NextProxyInfo.lpszProxyBypass);

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

DWORD
ProxyResolver::GetProxyForAutoSettings(
    __in HINTERNET hSession,
    __in_z PCWSTR pwszUrl,
    __in_z_opt PCWSTR pwszAutoConfigUrl,
    __out PWSTR *ppwszProxy,
    __out PWSTR *ppwszProxyBypass
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
    //

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
    __in HINTERNET hSession,
    __in_z PCWSTR pwszUrl
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

