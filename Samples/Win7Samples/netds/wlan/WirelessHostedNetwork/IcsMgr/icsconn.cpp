// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "stdafx.h"

#include <NetCon.h>


CIcsConnection::CIcsConnection( )
{
    m_pIcsMgr           =   NULL;
    m_pNetConnection    =   NULL;
    m_pNSConfig         =   NULL;
    m_lIndex            =   0;
    m_bSharingEnabled   =   false;
    m_bPublic           =   false;
    m_bPrivate          =   false;
    m_bSupported        =   false;

    ZeroMemory( &m_NetConnProps, sizeof(m_NetConnProps) );
}


CIcsConnection::~CIcsConnection( )
{
    FREE_CPP_ARRAY(m_NetConnProps.pszwName);
    FREE_CPP_ARRAY(m_NetConnProps.pszwDeviceName);

    SAFE_RELEASE(m_pNSConfig);
    SAFE_RELEASE(m_pNetConnection);
}


//
// Initialize an ICS connection object
// 1. Make sure that ICS is supported and hardware is available
// 2. Retrieve the ICS configuration from the connection
//
HRESULT
CIcsConnection::InitIcsConnection
(
    CIcsManager*        pIcsMgr,
    INetConnection*     pNetConnection,
    LONG                lIndex
)
{
    HRESULT             hr          =   S_OK;
    NETCON_PROPERTIES*  pNCProps    =   NULL;

    if (m_pIcsMgr)
    {
        hr = E_UNEXPECTED;
        BAIL_ON_HRESULT_ERROR(hr);
    }

    if (!(pIcsMgr && pNetConnection))
    {
        hr = E_INVALIDARG;
        BAIL_ON_HRESULT_ERROR(hr);
    }

    // No need to AddRef
    m_pIcsMgr   =   pIcsMgr;
    m_lIndex    =   lIndex;

    m_pNetConnection = pNetConnection;
    pNetConnection->AddRef( );

    hr =
    m_pNetConnection->GetProperties
    (
        &pNCProps
    );
    BAIL_ON_HRESULT_ERROR(hr);
    ASSERT(pNCProps);

    hr = NSModDuplicateNetconProperties(pNCProps, &m_NetConnProps);
    BAIL_ON_HRESULT_ERROR(hr);

    if ( NCM_LAN != m_NetConnProps.MediaType )
    {
        // Unsupported connection.
        hr = S_FALSE;
        BAIL( );
    }

    if (NCS_DISCONNECTED == m_NetConnProps.Status)
    {
        // Hardware is disabled
        hr = S_FALSE;
        BAIL();
    }

    hr =
    m_pIcsMgr->m_pNSMgr->get_INetSharingConfigurationForINetConnection
    (
        pNetConnection,
        &m_pNSConfig
    );
    BAIL_ON_HRESULT_ERROR(hr);
    ASSERT(m_pNSConfig);

    hr = RefreshSharingEnabled( );
    BAIL_ON_HRESULT_ERROR(hr);

    m_bSupported = true;

error:
    SAFE_FREE_NCP(pNCProps);
    return hr;
}



//
// Retrieve the current ICS setting for an interface
//
HRESULT
CIcsConnection::RefreshSharingEnabled
(
)
{
    HRESULT                 hr          =   S_OK;
    VARIANT_BOOL            bEnabled    =   VARIANT_FALSE;
    SHARINGCONNECTIONTYPE   Type        =   ICSSHARINGTYPE_PUBLIC;

    hr =
    m_pNSConfig->get_SharingEnabled
    (
        &bEnabled
    );
    BAIL_ON_HRESULT_ERROR(hr);

    m_bSharingEnabled   =   (bEnabled == VARIANT_TRUE);
    m_bPublic           =   false;
    m_bPrivate          =   false;

    if (m_bSharingEnabled)
    {
        hr =
        m_pNSConfig->get_SharingConnectionType
        (
            &Type
        );
        BAIL_ON_HRESULT_ERROR(hr);

        m_bPublic   = (Type == ICSSHARINGTYPE_PUBLIC);
        m_bPrivate  = (Type == ICSSHARINGTYPE_PRIVATE);

        ASSERT( m_bPublic || m_bPrivate );
    }

error:
    return hr;
}


// disable ICS only when ICS is enabled
HRESULT
CIcsConnection::DisableSharing
(
)
{
    HRESULT     hr  =   S_OK;

    hr = RefreshSharingEnabled( );
    BAIL_ON_HRESULT_ERROR(hr);

    if (!m_bSharingEnabled)
    {
        hr = S_FALSE;
        BAIL( );
    }

    hr = m_pNSConfig->DisableSharing();
    BAIL_ON_HRESULT_ERROR(hr);

    (VOID) RefreshSharingEnabled( );

error:
    return hr;
}

// Enable ICS on public interface
HRESULT
CIcsConnection::EnableAsPublic
(
)
{
    HRESULT     hr  =   S_OK;

    hr = DisableSharing( );
    BAIL_ON_HRESULT_ERROR(hr);

    hr = m_pNSConfig->EnableSharing( ICSSHARINGTYPE_PUBLIC );
    BAIL_ON_HRESULT_ERROR(hr);

    (VOID) RefreshSharingEnabled( );

error:
    return hr;
}

// Enable ICS on private interface
HRESULT
CIcsConnection::EnableAsPrivate
(
)
{
    HRESULT     hr  =   S_OK;

    hr = DisableSharing( );
    BAIL_ON_HRESULT_ERROR(hr);

    hr = m_pNSConfig->EnableSharing( ICSSHARINGTYPE_PRIVATE );
    BAIL_ON_HRESULT_ERROR(hr);

    (VOID) RefreshSharingEnabled( );

error:
    return hr;
}



bool
CIcsConnection::IsMatch
(
    GUID*   pGuid
)
{
    bool    bMatch  =   false;

    if (!pGuid)
    {
        BAIL( );
    }

    bMatch = !memcmp( pGuid, &(m_NetConnProps.guidId), sizeof(GUID) );


error:
    return bMatch;
}
