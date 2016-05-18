// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "stdafx.h"



#if DBG
void
AssertFunc
(
    LPCSTR  pszFunc,
    LPCSTR  pszFile,
    DWORD   dwLine
)
{
    CHAR    szMsgString[1024]   =   {0};
    HRESULT hr                  =   S_OK;

    hr =
    StringCchPrintfA
    (
        szMsgString,
        sizeof( szMsgString ) - 1,
        "\n\nAssert hit:\n\tFile     : %s\n\tFunction : %s\n\tLine     : %d\n\n",
        pszFile,
        pszFunc,
        dwLine
    );

    if ( S_OK == hr )
    {
        OutputDebugStringA( szMsgString );
        printf( "%s", szMsgString );
    }
    else
    {
        OutputDebugStringA( "ERROR: assert message generation failed.\n" );
    }

    DebugBreak( );
}

#endif


typedef
VOID
(STDAPICALLTYPE *FREE_NETCON_PROPERTIES_HANDLER)
(
    NETCON_PROPERTIES*  pProps
);

typedef
struct _NSMOD_GLOBALS
{
    FREE_NETCON_PROPERTIES_HANDLER  lpfnFreeNetConProperties;
    HMODULE                         hNetshModule;
    BOOL                            bNSModInited;
}
NSMOD_GLOBALS, *PNSMOD_GLOBALS;

NSMOD_GLOBALS g_NsMod = {0};




HRESULT
NSModInit
(
)
{
    DWORD       dwError     =   ERROR_SUCCESS;
    HRESULT     hr          =   S_OK;

    ZeroMemory( &g_NsMod, sizeof(g_NsMod) );

    g_NsMod.hNetshModule = LoadLibraryW( L"netshell.dll" );
    if (NULL == g_NsMod.hNetshModule)
    {
        dwError = GetLastError( );
        hr = __HRESULT_FROM_WIN32(dwError);
        BAIL_ON_HRESULT_ERROR(hr);
    }
    g_NsMod.bNSModInited    =   TRUE;

    g_NsMod.lpfnFreeNetConProperties = 
    (FREE_NETCON_PROPERTIES_HANDLER)
    GetProcAddress
    (
        g_NsMod.hNetshModule,
        "NcFreeNetconProperties"
    );
    if (NULL == g_NsMod.lpfnFreeNetConProperties)
    {
        dwError = GetLastError( );
        hr = __HRESULT_FROM_WIN32(dwError);
        BAIL_ON_HRESULT_ERROR(hr);
    }


error:
    if (FAILED(hr))
    {
        NSModDeinit( );
    }
    return hr;
}






VOID
NSModDeinit
(
)
{
    if (g_NsMod.bNSModInited)
    {
        g_NsMod.lpfnFreeNetConProperties = NULL;

        FreeLibrary(g_NsMod.hNetshModule);

        g_NsMod.hNetshModule = NULL;

        g_NsMod.bNSModInited = FALSE;
    }
}



VOID
NSModFreeNetConProperties
(
    NETCON_PROPERTIES*  pProps
)
{
    if (g_NsMod.lpfnFreeNetConProperties && pProps )
    {
        (g_NsMod.lpfnFreeNetConProperties)(pProps);
    }
}


HRESULT
CopyString
(
    __in LPCWSTR     pszSrc,
    __in LPWSTR*     ppszDst
)
{
    HRESULT     hr          =   S_OK;
    ULONG       ulLen       =   0;
    ULONG       i           =   0;

    ASSERT(ppszDst);
    ASSERT(NULL == (*ppszDst));

    if (!pszSrc)
    {
        BAIL( );
    }

    ulLen = (ULONG) (1 + wcslen( pszSrc ));

    (*ppszDst) = new WCHAR [ ulLen + 1 ];
    if (NULL == (*ppszDst))
    {
        hr = E_OUTOFMEMORY;
        BAIL_ON_HRESULT_ERROR(hr);
    }

    CopyMemory( (*ppszDst), pszSrc, ulLen * sizeof(WCHAR) );

    (*ppszDst)[ulLen] = 0;

error:
    return hr;

}

HRESULT
NSModDuplicateNetconProperties
(
    NETCON_PROPERTIES*      pSrc,
    NETCON_PROPERTIES*      pDst
)
{
    HRESULT     hr          =   S_OK;

    ASSERT(pSrc);
    ASSERT(pDst);

    CopyMemory( pDst, pSrc, sizeof(NETCON_PROPERTIES) );

    pDst->pszwName          =   NULL;
    pDst->pszwDeviceName    =   NULL;

    hr = CopyString( pSrc->pszwName, &(pDst->pszwName) );
    BAIL_ON_HRESULT_ERROR(hr);

    hr = CopyString( pSrc->pszwDeviceName, &(pDst->pszwDeviceName) );
    BAIL_ON_HRESULT_ERROR(hr);

error:
    return hr;
}


#define CASE_ENTRY(_X)      \
    case (_X):              \
        pszResult = L#_X;   \
        break;              \


LPCWSTR GetNetConMediaTypeStr(NETCON_MEDIATYPE MediaType)
{
    LPCWSTR pszResult   =   L"Unknown Media Type";

    switch(MediaType)
    {
        CASE_ENTRY(NCM_NONE);
        CASE_ENTRY(NCM_DIRECT);
        CASE_ENTRY(NCM_ISDN);
        CASE_ENTRY(NCM_LAN);
        CASE_ENTRY(NCM_PHONE);
        CASE_ENTRY(NCM_TUNNEL);
        CASE_ENTRY(NCM_PPPOE);
        CASE_ENTRY(NCM_BRIDGE);
        CASE_ENTRY(NCM_SHAREDACCESSHOST_LAN);
        CASE_ENTRY(NCM_SHAREDACCESSHOST_RAS);

        default:
            break;

    }

    return pszResult;
}

LPCWSTR GetNetConStatusStr(NETCON_STATUS Status)
{
    LPCWSTR pszResult   =   L"Unknown Status";

    switch(Status)
    {

        CASE_ENTRY(NCS_DISCONNECTED);
        CASE_ENTRY(NCS_CONNECTING);
        CASE_ENTRY(NCS_CONNECTED);
        CASE_ENTRY(NCS_DISCONNECTING);
        CASE_ENTRY(NCS_HARDWARE_NOT_PRESENT);
        CASE_ENTRY(NCS_HARDWARE_DISABLED);
        CASE_ENTRY(NCS_HARDWARE_MALFUNCTION);
        CASE_ENTRY(NCS_MEDIA_DISCONNECTED);
        CASE_ENTRY(NCS_AUTHENTICATING);
        CASE_ENTRY(NCS_AUTHENTICATION_SUCCEEDED);
        CASE_ENTRY(NCS_AUTHENTICATION_FAILED);
        CASE_ENTRY(NCS_INVALID_ADDRESS);
        CASE_ENTRY(NCS_CREDENTIALS_REQUIRED);

        default:
            break;

    }

    return pszResult;
}
