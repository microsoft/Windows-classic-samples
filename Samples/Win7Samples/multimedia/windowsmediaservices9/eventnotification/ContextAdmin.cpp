//+-------------------------------------------------------------------------
//
//  Microsoft Windows Media Technologies
//  Copyright (C) Microsoft Corporation. All rights reserved.
//
//  File:       ContextAdmin.cpp
//
//  Contents:
//
//--------------------------------------------------------------------------
#include "StdAfx.h"
#include "ContextDll.h"
#include "ContextAdmin.h"
#include <comdef.h>
/////////////////////////////////////////////////////////////////////////////
// CContextAdmin

STDMETHODIMP CContextAdmin::InterfaceSupportsErrorInfo(REFIID riid)
{
    static const IID* arr[] = 
    {
        &IID_IWMSContextAdmin
    };
    for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
    {
        if ( IsEqualGUID( *arr[ i ], riid ) )
        {
            return S_OK;
        }
    }

    return( S_FALSE );
}

STDMETHODIMP CContextAdmin::Initialize( CContextPlugin *pPlugin )
{
    HRESULT hr = S_OK;

    if( NULL == pPlugin )
    {
        return E_INVALIDARG;
    }

    m_pContextPlugin = pPlugin;

    return( hr );
}

STDMETHODIMP CContextAdmin::put_OutputPath( BSTR bstrOutputPath )
{
    HRESULT hr = S_OK;
    if( NULL == m_pContextPlugin )
    {
        return( E_UNEXPECTED );
    }

    // The parameters should be validated in SetOutputPath
    hr = m_pContextPlugin->SetOutputPath( bstrOutputPath );

    return( hr );
}


STDMETHODIMP CContextAdmin::get_OutputPath( BSTR *pbstrOutputPath )
{
    HRESULT hr = S_OK;
    if( NULL == m_pContextPlugin )
    {
        return( E_UNEXPECTED );
    }

    // The parameters should be validated in GetOutputPath
    hr = m_pContextPlugin->GetOutputPath( pbstrOutputPath );

    return( hr );
}


STDMETHODIMP CContextAdmin::put_ContextTypes( WMS_CONTEXT_PLUGIN_CONTEXT_TYPE wmsContextTypes )
{
    HRESULT hr = S_OK;
    if( NULL == m_pContextPlugin )
    {
        return( E_UNEXPECTED );
    }

    // The parameters should be validated in SetContextTypes
    hr = m_pContextPlugin->SetContextTypes( wmsContextTypes );

    return( hr );
}


STDMETHODIMP CContextAdmin::get_ContextTypes( WMS_CONTEXT_PLUGIN_CONTEXT_TYPE *pwmsContextTypes )
{
    HRESULT hr = S_OK;
    if( NULL == m_pContextPlugin )
    {
        return( E_UNEXPECTED );
    }

    // The parameters should be validated in GetContextTypes
    hr = m_pContextPlugin->GetContextTypes( pwmsContextTypes );

    return( hr );
}
