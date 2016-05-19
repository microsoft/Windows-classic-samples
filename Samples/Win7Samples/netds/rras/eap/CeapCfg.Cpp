/*

Copyright (c) 1997, Microsoft Corporation, all rights reserved

Description:

History:

*/

#include "ceapcfg.h"
#include <strsafe.h>

extern "C"
DWORD APIENTRY
RasEapInvokeConfigUI(
    IN  DWORD       dwEapTypeId,
    IN  HWND        hwndParent,
    IN  DWORD       dwFlags,
    IN  BYTE*       pConnectionDataIn,
    IN  DWORD       dwSizeOfConnectionDataIn,
    OUT BYTE**      ppConnectionDataOut,
    OUT DWORD*      pdwSizeOfConnectionDataOut
);

extern "C"
DWORD
GetIdentity(
    IN  HWND    hwndParent,
    IN  BYTE*   pUserDataIn,
    IN  DWORD   dwSizeOfUserDataIn,
    OUT BYTE**  ppUserDataOut,
    OUT DWORD*  pdwSizeOfUserDataOut,
    OUT WCHAR** ppwszIdentityOut
);

extern "C"
DWORD APIENTRY
RasEapFreeMemory(
    IN  BYTE*   pMemory
);

/*

Notes:
    Implementation of IEAPProviderConfig::Initialize
    
*/

STDMETHODIMP
CEapCfg::Initialize(
    LPCOLESTR   pwszMachineName,
    DWORD       dwEapTypeId,
    ULONG_PTR*  puConnectionParam
)
{
    DWORD       cch;
    WCHAR*      pwsz    = NULL;
    DWORD       dwErr   = NO_ERROR;

    *puConnectionParam = NULL;

    //
    // Save machine name as connection param.
    //

    //
    // How many chars are there in the name?
    //
    if ( NULL == pwszMachineName )
    {
        dwErr = ERROR_INVALID_PARAMETER;
        goto LDone;
    }
    cch = wcslen(pwszMachineName);

    pwsz = (WCHAR*) LocalAlloc(LPTR, (cch + 1) * sizeof(WCHAR));

    if (NULL == pwsz)
    {
        dwErr = GetLastError();
        goto LDone;
    }

    //
    // Copy machine name
    //

    StringCchCopyW(pwsz, cch+1,pwszMachineName);
    *puConnectionParam = (ULONG_PTR)pwsz;
    pwsz = NULL;

LDone:

    LocalFree(pwsz);

    return(HRESULT_FROM_WIN32(dwErr));
}

/*

Notes:
    Implementation of IEAPProviderConfig::Uninitialize

*/

STDMETHODIMP
CEapCfg::Uninitialize(
    DWORD       dwEapTypeId,
    ULONG_PTR   uConnectionParam
)
{
    LocalFree((VOID*)uConnectionParam);
    return(HRESULT_FROM_WIN32(NO_ERROR));
}

/*

Notes:
    Implementation of IEAPProviderConfig::ServerInvokeConfigUI
        hWnd - handle to the parent window
        dwRes1 - reserved parameter (ignore)
        dwRes2 - reserved parameter (ignore)

*/

STDMETHODIMP
CEapCfg::ServerInvokeConfigUI(
    DWORD       dwEapTypeId,
    ULONG_PTR   uConnectionParam,
    HWND        hWnd,
    DWORD_PTR   dwRes1,
    DWORD_PTR   dwRes2
)
{
    WCHAR*      pwszMachineName;
    HRESULT     hr;
    DWORD       dwErr;

    pwszMachineName = (WCHAR*)uConnectionParam;

    if (NULL == pwszMachineName)
    {
        dwErr = E_FAIL;
    }
    else
    {
        MessageBox(hWnd, pwszMachineName, L"You are configuring the EAP on...",
            MB_OK | MB_ICONINFORMATION);
    }

    hr = HRESULT_FROM_WIN32(dwErr);

    return(hr);
}

/*

Notes:
    Implementation of IEAPProviderConfig::RouterInvokeConfigUI

*/

STDMETHODIMP
CEapCfg::RouterInvokeConfigUI(
    DWORD       dwEapTypeId,
    ULONG_PTR   uConnectionParam,
    HWND        hwndParent,
    DWORD       dwFlags,
    BYTE*       pConnectionDataIn,
    DWORD       dwSizeOfConnectionDataIn,
    BYTE**      ppConnectionDataOut,
    DWORD*      pdwSizeOfConnectionDataOut
)
{
    DWORD       dwErr                       = NO_ERROR;
    BYTE*       pConnectionDataOut          = NULL;
    DWORD       dwSizeOfConnectionDataOut   = 0;

    *ppConnectionDataOut = NULL;
    *pdwSizeOfConnectionDataOut = 0;

    dwErr = RasEapInvokeConfigUI(
                dwEapTypeId,
                hwndParent,
                dwFlags,
                pConnectionDataIn,
                dwSizeOfConnectionDataIn,
                &pConnectionDataOut,
                &dwSizeOfConnectionDataOut);

    if (   (NO_ERROR == dwErr)
        && (0 != dwSizeOfConnectionDataOut))
    {
        //
        // If we got valid ConnectionDataOut, allocate memory for it...
        //

        *ppConnectionDataOut = (BYTE*)CoTaskMemAlloc(dwSizeOfConnectionDataOut);

        if (NULL == *ppConnectionDataOut)
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            goto LDone;
        }

        //
        // ... and return it in the OUT parameter
        //

        CopyMemory(*ppConnectionDataOut, pConnectionDataOut,
            dwSizeOfConnectionDataOut);
        *pdwSizeOfConnectionDataOut = dwSizeOfConnectionDataOut;
    }

LDone:

    RasEapFreeMemory(pConnectionDataOut);

    return(HRESULT_FROM_WIN32(dwErr));
}

/*

Notes:
    Implementation of IEAPProviderConfig::RouterInvokeCredentialsUI

*/

STDMETHODIMP
CEapCfg::RouterInvokeCredentialsUI(
    DWORD       dwEapTypeId,
    ULONG_PTR   uConnectionParam,
    HWND        hwndParent,
    DWORD       dwFlags,
    BYTE*       pConnectionDataIn,
    DWORD       dwSizeOfConnectionDataIn,
    BYTE*       pUserDataIn,
    DWORD       dwSizeOfUserDataIn,
    BYTE**      ppUserDataOut,
    DWORD*      pdwSizeOfUserDataOut
)
{
    BYTE*       pUserDataOut            = NULL;
    DWORD       dwSizeOfUserDataOut;
    WCHAR*      pwszIdentityOut         = NULL;
    DWORD       dwErr                   = NO_ERROR;

    *ppUserDataOut = NULL;
    *pdwSizeOfUserDataOut = 0;

    dwErr = GetIdentity(
                    hwndParent,
                    NULL /* pUserDataIn */,
                    0 /* dwSizeOfUserDataIn */,
                    &pUserDataOut,
                    &dwSizeOfUserDataOut,
                    &pwszIdentityOut);

    if (   (NO_ERROR == dwErr)
        && (0 != dwSizeOfUserDataOut))
    {
        //
        // If we got valid UserDataOut, allocate memory for it...
        //

        *ppUserDataOut = (BYTE*)CoTaskMemAlloc(dwSizeOfUserDataOut);

        if (NULL == *ppUserDataOut)
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            goto LDone;
        }

        //
        // ... and return it in the OUT parameter
        //

        CopyMemory(*ppUserDataOut, pUserDataOut, dwSizeOfUserDataOut);
        *pdwSizeOfUserDataOut = dwSizeOfUserDataOut;
    }

LDone:

    LocalFree(pUserDataOut);
    LocalFree((BYTE*)pwszIdentityOut);

    return(HRESULT_FROM_WIN32(dwErr));
}
