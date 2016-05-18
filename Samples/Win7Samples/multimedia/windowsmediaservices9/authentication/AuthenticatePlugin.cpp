//+-------------------------------------------------------------------------
//
//  Microsoft Windows Media Technologies
//  Copyright (C) Microsoft Corporation. All rights reserved.
//
//  File:       AuthenticatePlugin.cpp
//
//  Contents:
//
//--------------------------------------------------------------------------

#include "stdafx.h"
#include "Authenticate.h"
#include "AuthenticatePlugin.h"
#include "AuthenticateAdmin.h"
#include "AuthenticateContext.h"

#define PACKET_NAME     "MyPackage"
#define PROTOCOL_NAME   "Basic"

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
CAuthenticatePlugin::CAuthenticatePlugin()
{
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
CAuthenticatePlugin::~CAuthenticatePlugin()
{
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CAuthenticatePlugin::InitializePlugin
(
    IWMSContext *pServerContext,
    IWMSNamedValues *pNamedValues,
    IWMSClassObject *pClassFactory
)
{
    if( ( NULL == pServerContext )
        || ( NULL == pNamedValues )
        || ( NULL == pClassFactory ) )
    {
        return( E_INVALIDARG );
    }

    HRESULT hr = S_OK;

    if( NULL != pNamedValues )
    {
        m_spNamedValues = pNamedValues;
    }

    // TODO: Add any additional initialization here

    return( hr );
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CAuthenticatePlugin::EnablePlugin
(
    long *pdwFlags,
    long *pdwHeartbeatPeriod
)
{
    if( NULL == pdwFlags || NULL == pdwHeartbeatPeriod )
    {
        return( E_POINTER );
    }

    HRESULT hr = S_OK;

    // Set the heartbeat period in milliseconds
    *pdwHeartbeatPeriod = 0000;

    return( hr );
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CAuthenticatePlugin::DisablePlugin()
{
    HRESULT hr = S_OK;

    // TODO: Add any additional code here

    return( hr );
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CAuthenticatePlugin::ShutdownPlugin()
{
    HRESULT hr = S_OK;

    m_spNamedValues = NULL;

    // TODO: Add any additional code here

    return( hr );
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CAuthenticatePlugin::OnHeartbeat()
{
    HRESULT hr = S_OK;

    // TODO: Add code that should execute on every Heartbeat Period

    return( hr );
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CAuthenticatePlugin::GetCustomAdminInterface
(
    IDispatch **ppValue
)
{
    if( ( NULL == ppValue ) )
    {
        return( E_POINTER );
    }

    HRESULT hr = S_OK;

    *ppValue = NULL;

    CComObject<CAuthenticateAdmin> *pAuthenticateAdmin = NULL;

    hr = CComObject<CAuthenticateAdmin>::CreateInstance( &pAuthenticateAdmin );

    if( SUCCEEDED( hr ) )
    {
        pAuthenticateAdmin->AddRef();
        hr = pAuthenticateAdmin->Initialize( this );
    }

    if( SUCCEEDED(hr) )
    {
        hr = pAuthenticateAdmin->QueryInterface( IID_IDispatch, (void **) ppValue );
    }

    if( NULL != pAuthenticateAdmin )
    {
        pAuthenticateAdmin->Release();
    }

    return( hr );
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CAuthenticatePlugin::GetPackageName
(
    BSTR *PackageName
)
{
    if( NULL == PackageName )
    {
        return( E_POINTER );
    }

    HRESULT hr = S_OK;

    *PackageName = SysAllocString( _T( PACKET_NAME ) );

    if( NULL == *PackageName )
    {
        hr = E_OUTOFMEMORY;
    }

    return( hr );
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CAuthenticatePlugin::GetProtocolName
(
    BSTR *ProtocolName
)
{
    if( NULL == ProtocolName )
    {
        return( E_POINTER );
    }

    HRESULT hr = S_OK;

    *ProtocolName = SysAllocString( _T( PROTOCOL_NAME ) );

    if( NULL == *ProtocolName )
    {
        hr = E_OUTOFMEMORY;
    }

    return( hr );
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CAuthenticatePlugin::GetFlags
(
    long *Flags
)
{
    if( NULL == Flags )
    {
        return( E_POINTER );
    }

    HRESULT hr = S_OK;

    *Flags = WMS_AUTHENTICATION_TEXT_CHALLENGE
           | WMS_AUTHENTICATION_CLIENT_SHOWS_UI
           | WMS_AUTHENTICATION_CHALLENGE_FIRST;

    return( hr );
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CAuthenticatePlugin::CreateAuthenticationContext
(
    IWMSAuthenticationContext **ppAuthenCtx
)
{
    if( NULL == ppAuthenCtx )
    {
        return( E_POINTER );
    }

    HRESULT hr = S_OK;

    CComObject<CAuthenticateContext> *pAuthenticateContext = NULL;

    if( SUCCEEDED( hr ) )
    {
        hr = CComObject<CAuthenticateContext>::CreateInstance( &pAuthenticateContext );
    }

    if( SUCCEEDED( hr ) )
    {
        pAuthenticateContext->AddRef();
        hr = pAuthenticateContext->Initialize( this );
    }

    if( SUCCEEDED( hr ) )
    {
        hr = pAuthenticateContext->QueryInterface( IID_IWMSAuthenticationContext, (void **) ppAuthenCtx );
    }

    if( NULL != pAuthenticateContext )
    {
        pAuthenticateContext->Release();
    }

    return( hr );
}

