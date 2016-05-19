//+-------------------------------------------------------------------------
//
//  Microsoft Windows Media Technologies
//  Copyright (C) Microsoft Corporation. All rights reserved.
//
//  File:       AuthenticateAdmin.cpp
//
//  Contents:
//
//--------------------------------------------------------------------------

#include "stdafx.h"
#include "Authenticate.h"
#include "AuthenticateAdmin.h"

/////////////////////////////////////////////////////////////////////////////
// CAuthenticateAdmin

STDMETHODIMP CAuthenticateAdmin::InterfaceSupportsErrorInfo
(
    REFIID riid
)
{
    static const IID* arr[] = 
    {
        &IID_IAuthenticateAdmin
    };

    for( int i = 0; i < sizeof( arr ) / sizeof( arr[ 0 ] ); i++ )
    {
        if( IsEqualGUID( *arr[i], riid ) )
            return( S_OK );
    }
    return( S_FALSE );
}

STDMETHODIMP CAuthenticateAdmin::Initialize
(
    CAuthenticatePlugin *pPlugin
)
{
    HRESULT hr = S_OK;

    if( NULL == pPlugin )
    {
        return( E_INVALIDARG );
    }

    m_pAuthenticatePlugin = pPlugin;

    return( hr );
}
