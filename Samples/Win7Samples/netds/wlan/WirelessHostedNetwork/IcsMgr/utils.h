// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once


#ifdef ASSERT
#undef ASSERT
#endif // ASSERT

#if DBG
void
AssertFunc
(
    LPCSTR  pszFunc,
    LPCSTR  pszFile,
    DWORD   dwLine
);
#define ASSERT(_X) if (!(_X)) AssertFunc(__FUNCTION__,__FILE__,__LINE__);
#else
#define ASSERT(_X)
#endif


#define BAIL( ) goto error;

#define BAIL_ON_HRESULT_ERROR(hr)   \
    if (FAILED(hr))                 \
    {                               \
        ASSERT(FALSE);              \
        BAIL( );                    \
    }                               \

#define BAIL_ON_WIN32_ERROR(_x)     \
    if (_x)                         \
    {                               \
        ASSERT(FALSE);              \
        BAIL( );                    \
    }                               \


#define SAFE_RELEASE(_x)    \
{                           \
    if (_x)                 \
    {                       \
        (_x)->Release();    \
        (_x) = NULL;        \
    }                       \
}                           \

#define SAFE_FREE_BSTR(_x)  \
{                           \
    if (_x)                 \
    {                       \
        SysFreeString(_x);  \
        (_x) = NULL;        \
    }                       \
}                           \

#define SAFE_FREE_NCP(_x)               \
{                                       \
    if (_x)                             \
    {                                   \
        NSModFreeNetConProperties(_x);  \
        (_x) = NULL;                    \
    }                                   \
}                                       \

#define FREE_CPP_ARRAY(_x)              \
{                                       \
    if (_x)                             \
    {                                   \
        delete [] (_x);                 \
        (_x) = NULL;                    \
    }                                   \
}                                       \




#define SAFE_FREE_CLSID_STRING(_x)      \
{                                       \
    if (_x)                             \
    {                                   \
        CoTaskMemFree(_x);              \
        (_x) = NULL;                    \
    }                                   \
}                                       \




#define ARRAY_SIZE( _x )    (DWORD) ( sizeof( _x ) / sizeof( (_x)[0] ) )


// __HRESULT_FROM_WIN32 is defined in winerror.h

#define WIN32_FROM_HRESULT(hr)           \
    (SUCCEEDED(hr) ? ERROR_SUCCESS :     \
        (HRESULT_FACILITY(hr) == FACILITY_WIN32 ? HRESULT_CODE(hr) : (hr)))




VOID
NSModFreeNetConProperties
(
    NETCON_PROPERTIES*  pProps
);


HRESULT
NSModDuplicateNetconProperties
(
    NETCON_PROPERTIES*      pSrc,
    NETCON_PROPERTIES*      pDst
);


