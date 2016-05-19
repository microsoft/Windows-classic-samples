//*****************************************************************************
//
// Microsoft Windows Media
// Copyright (C) Microsoft Corporation. All rights reserved.
//
// FileName: DBAuthAdmin.cpp
//
// Abstract:
//
//*****************************************************************************
#include "stdafx.h"
#include "DBAuth.h"
#include "DBAuthAdmin.h"

/////////////////////////////////////////////////////////////////////////////
// CDBAuthAdmin
/////////////////////////////////////////////////////////////////////////////

STDMETHODIMP CDBAuthAdmin::InterfaceSupportsErrorInfo( REFIID riid )
{
    static const IID* arr[] =
    {
        &IID_IDBAuthAdmin
    };
    for ( int i = 0; i < sizeof( arr ) / sizeof( arr[ 0 ] ); i++ )
    {
        if( IsEqualGUID( *arr[ i ], riid ) )
        {
            return( S_OK );
        }
    }

    return( S_FALSE );
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CDBAuthAdmin::Initialize( CDBAuthPlugin *pPlugin )
{
    HRESULT hr = S_OK;

    if( NULL == pPlugin )
    {
        return( E_INVALIDARG );
    }

    m_pDBAuthPlugin = pPlugin;

    return( hr );
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CDBAuthAdmin::AddUser( BSTR bstrUserName )
{
    if( NULL == bstrUserName )
    {
        return( E_INVALIDARG );
    }

    if( L'\0' == (WCHAR*) *bstrUserName )
    {
        return( S_OK );
    }

    return m_pDBAuthPlugin->AddUser( (LPCWSTR) bstrUserName );
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CDBAuthAdmin::RemoveUser( BSTR bstrUserName )
{
    if( NULL == bstrUserName )
    {
        return( E_INVALIDARG );
    }

    if( L'\0' == (WCHAR*) *bstrUserName )
    {
        return( S_OK );
    }

    return m_pDBAuthPlugin->RemoveUser( (LPCWSTR) bstrUserName );
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
HRESULT CDBAuthAdmin::VerifyUser( BSTR bstrUserName, VARIANT_BOOL *pbFound )
{
    if( NULL == bstrUserName )
    {
        return( E_INVALIDARG );
    }

    if( NULL == pbFound )
    {
        return( E_POINTER );
    }

    BOOL bFound = FALSE;
    DWORD dwIndex = 0;
    HRESULT hr = m_pDBAuthPlugin->VerifyUserMembership( ( LPCWSTR ) bstrUserName, &bFound, &dwIndex );

    *pbFound = ( SUCCEEDED( hr ) && bFound ) ? VARIANT_TRUE : VARIANT_FALSE;

    return( hr );
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
HRESULT CDBAuthAdmin::GetUsers( BSTR *pbstrUserNames )
{
    if( NULL == pbstrUserNames )
    {
        return( E_POINTER );
    }

    return( m_pDBAuthPlugin->GetUsers( pbstrUserNames ) );

}

