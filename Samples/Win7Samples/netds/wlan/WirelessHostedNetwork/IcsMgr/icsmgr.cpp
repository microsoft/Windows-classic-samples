// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "stdafx.h"

CIcsConnectionInfo::CIcsConnectionInfo(
    const CIcsConnection & Connection
    )
{
    Connection.GetConnectionGuid(m_Guid);
    Connection.GetConnectionName(m_Name);
    m_Supported = Connection.IsSupported();
    m_SharingEnabled = Connection.IsSharingEnabled();
    m_Private = Connection.IsPrivate();
    m_Public = Connection.IsPublic();
}


CIcsManager::CIcsManager( )
{
    m_pNSMgr                 =   NULL;
    m_pList                  =   NULL;
    m_lNumConns              =   0;
    m_bInstalled             =   false;
    m_lPublicICSIntfIndex    =   -1;
    m_lPrivateICSIntfIndex   =   -1;
}



CIcsManager::~CIcsManager( )
{
    FREE_CPP_ARRAY(m_pList);

    m_lNumConns = 0;

    SAFE_RELEASE(m_pNSMgr);
}

// 
// The ICS manager initialization takes 3 steps --
// 1. Make sure that ICS is supported in the current version of Windows
// 2. Retrieve all the ICS connections
// 3. Initialize each connection
//
HRESULT
CIcsManager::InitIcsManager
(
)
{
    HRESULT                                 hr                  =   S_OK;
    IUnknown*                               pUnkEnum            =   NULL;
    IEnumNetSharingEveryConnection*         pNSEConn            =   NULL;
    INetSharingEveryConnectionCollection*   pConnectionsList    =   NULL;
    INetConnection*                         pNetConnection      =   NULL;
    LONG                                    lIndex              =   0;
    VARIANT_BOOL                            bSharingEnabled     =   VARIANT_FALSE;
    VARIANT                                 varItem;
    ULONG                                   pceltFetched;

    VariantInit(&varItem);

    if (m_pNSMgr)
    {
        hr = E_UNEXPECTED;
        BAIL_ON_HRESULT_ERROR(hr);
    }

    hr =
    CoCreateInstance
    (
        __uuidof(NetSharingManager),
        NULL,
        CLSCTX_ALL,
        __uuidof(INetSharingManager),
        (void**) &m_pNSMgr
    );
    BAIL_ON_HRESULT_ERROR(hr);
    ASSERT(m_pNSMgr);


    hr =
    RefreshInstalled
    (
    );
    BAIL_ON_HRESULT_ERROR(hr);

    if (!m_bInstalled)
    {
        hr = E_UNEXPECTED;
        BAIL_ON_HRESULT_ERROR(hr);
    }

    hr =
    m_pNSMgr->get_EnumEveryConnection
    (
        &pConnectionsList
    );
    BAIL_ON_HRESULT_ERROR(hr);
    ASSERT(pConnectionsList);

    hr =
    pConnectionsList->get__NewEnum
    (
        &pUnkEnum
    );
    BAIL_ON_HRESULT_ERROR(hr);
    ASSERT(pUnkEnum);

    hr =
    pUnkEnum->QueryInterface
    (
        __uuidof(IEnumNetSharingEveryConnection),
        (void**) &pNSEConn
    );
    BAIL_ON_HRESULT_ERROR(hr);
    ASSERT(pNSEConn);

    m_lNumConns = 0;

    while (1)
    {
        VariantClear(&varItem);

        hr =
        pNSEConn->Next
        (
            1,
            &varItem,
            &pceltFetched    
        );
        BAIL_ON_HRESULT_ERROR(hr);

        if (S_FALSE == hr)
        {
            hr = S_OK;
            break;
        }

        m_lNumConns++;

        ASSERT( (V_VT(&varItem) == VT_UNKNOWN) && V_UNKNOWN(&varItem) );
    }

    hr = pNSEConn->Reset( );
    BAIL_ON_HRESULT_ERROR(hr);


    if (0 == m_lNumConns)
    {
        BAIL( );
    }

    m_pList = new(std::nothrow) CIcsConnection [m_lNumConns];
    if (!m_pList)
    {
        hr = E_OUTOFMEMORY;
        BAIL_ON_HRESULT_ERROR(hr);
    }


    lIndex = 0;
    while ( 1 )
    {
        SAFE_RELEASE(pNetConnection);
        VariantClear(&varItem);

        hr =
        pNSEConn->Next
        (
            1,
            &varItem,
            &pceltFetched
        );
        BAIL_ON_HRESULT_ERROR(hr);

        if (S_FALSE == hr)
        {
            hr = S_OK;
            break;
        }

        lIndex++;
        if (lIndex > m_lNumConns)
        {
            hr = E_UNEXPECTED;
            BAIL_ON_HRESULT_ERROR(hr);
        }


        ASSERT( (V_VT(&varItem) == VT_UNKNOWN) && V_UNKNOWN(&varItem) );

        hr =
        V_UNKNOWN(&varItem)->QueryInterface
        (
            __uuidof(INetConnection),
            (void**) &pNetConnection
        );
        BAIL_ON_HRESULT_ERROR(hr);
        ASSERT(pNetConnection);

        hr =
        m_pList[lIndex-1].InitIcsConnection
        (
            this,
            pNetConnection,
            lIndex-1
        );
        BAIL_ON_HRESULT_ERROR(hr);

    }

    ASSERT(lIndex == m_lNumConns);

error:

    SAFE_RELEASE(pNetConnection);
    VariantClear(&varItem);
    SAFE_RELEASE(pNSEConn);
    SAFE_RELEASE(pUnkEnum);
    SAFE_RELEASE(pConnectionsList);

    return hr;
}

HRESULT
CIcsManager::ResetIcsManager()
{
    FREE_CPP_ARRAY(m_pList);

    m_lNumConns = 0;

    SAFE_RELEASE(m_pNSMgr);

    return InitIcsManager();
}

//
// Find out if ICS is supported
//
HRESULT
CIcsManager::RefreshInstalled
(
)
{
    HRESULT         hr          =   S_OK;
    VARIANT_BOOL    bInstalled  =   VARIANT_FALSE;

    hr =
    m_pNSMgr->get_SharingInstalled
    (
        &bInstalled
    );
    BAIL_ON_HRESULT_ERROR(hr);

    m_bInstalled = (bInstalled == VARIANT_TRUE);

error:
    return hr;
}


void
CIcsManager::DisableIcsOnAll
(
)
{
    long        lIndex  =   0;

    ASSERT(m_pList);

    for ( lIndex = 0; lIndex < m_lNumConns; lIndex++ )
    {
        if (!(m_pList[lIndex].IsSupported( )))
        {
            continue;
        }
        m_pList[lIndex].DisableSharing( );
    }

    return;
}

HRESULT
CIcsManager::GetIndexByGuid
(
    GUID&       Guid,
    long*       plIndex
)
{
    HRESULT     hr      =   S_OK;
    long        lIndex  =   0;
    bool        bMatch  =   false;

    ASSERT(plIndex);

    ASSERT(m_pList);

    for ( lIndex = 0; lIndex < m_lNumConns; lIndex++ )
    {
        if (!(m_pList[lIndex].IsSupported( )))
        {
            continue;
        }

        bMatch = m_pList[lIndex].IsMatch( &Guid );
        if (bMatch)
        {
            (*plIndex) = lIndex;
            BAIL( );
        }
    }

    ASSERT(!bMatch);

    hr = E_INVALIDARG;
    BAIL_ON_HRESULT_ERROR(hr);


error:
    return hr;
}



//
// Enable ICS on public and private interfaces
//

HRESULT
CIcsManager::EnableIcs
(
    GUID&   PublicGuid,
    GUID&   PrivateGuid
)
{
    HRESULT     hr              =   S_OK;
    long        lPublicIndex    =   0;
    long        lPrivateIndex   =   0;

    hr = GetIndexByGuid( PublicGuid, &lPublicIndex );
    BAIL_ON_HRESULT_ERROR(hr);

    hr = GetIndexByGuid( PrivateGuid, &lPrivateIndex );
    BAIL_ON_HRESULT_ERROR(hr);

    // Check if the ICS on public/private interfaces are already enabled.
    // if so, no need to start ICS again.
    m_pList[lPublicIndex].RefreshSharingEnabled();
    m_pList[lPrivateIndex].RefreshSharingEnabled();

    if (m_pList[lPublicIndex].IsSharingEnabled() &&
        m_pList[lPublicIndex].IsPublic() &&
        m_pList[lPrivateIndex].IsSharingEnabled() &&
        m_pList[lPrivateIndex].IsPrivate())
    {
        BAIL();
    }

    //
    // Disable existing ICS first
    //
    DisableIcsOnAll();

    //
    // Enable new ICS
    //
    hr = m_pList[lPublicIndex].EnableAsPublic( );
    BAIL_ON_HRESULT_ERROR(hr);

    hr = m_pList[lPrivateIndex].EnableAsPrivate( );
    BAIL_ON_HRESULT_ERROR(hr);

error:
    return hr;
}

//
// Get the list of ICS connections
//
void
CIcsManager::GetIcsConnections(
    CRefObjList<CIcsConnectionInfo *> & ConnectionList
    )
{
    // remove all old entries
    ConnectionList.RemoveAllEntries();

    // add connections
    _ASSERT(m_pList);

    for (long i = 0; i < m_lNumConns; i++)
    {
        CIcsConnectionInfo * pConnInfo = NULL;

        pConnInfo = new(std::nothrow) CIcsConnectionInfo(m_pList[i]);

        if (NULL == pConnInfo)
        {
            break;
        }

        ConnectionList.AddTail(pConnInfo);
    }
}

//
// Cache the current ICS-enabled interfaces
// this step is taken before enabling ICS/softAP. 
// Used for recovery in case the ICS/softAP enabling
// fails
//
void
CIcsManager::CacheICSIntfIndex
(
)
{
    long lIndex = 0;
    
    ASSERT(m_pList);

    m_lPublicICSIntfIndex = -1;
    m_lPrivateICSIntfIndex = -1;

    for ( lIndex = 0; lIndex < m_lNumConns; lIndex++ )
    {
        if (!(m_pList[lIndex].IsSupported( )))
        {
            continue;
        }
        m_pList[lIndex].RefreshSharingEnabled();
        if (m_pList[lIndex].IsSharingEnabled())
        {
            if (m_pList[lIndex].IsPublic())
            {
                m_lPublicICSIntfIndex = lIndex;
            }
            if (m_pList[lIndex].IsPrivate())
            {
                m_lPrivateICSIntfIndex = lIndex;
            }
        }
    }
}

//
// This is a best-effort recovery step that restores 
// the previously enabled ICS, taken when
// starting softAP or enabling ICS on new interface
// fails
//
void
CIcsManager::EnableICSonCache
(
 )
{
    DisableIcsOnAll();

    if ((m_lPublicICSIntfIndex >= 0) && (m_lPrivateICSIntfIndex >= 0))
    {
        m_pList[m_lPublicICSIntfIndex].EnableAsPublic( );
        m_pList[m_lPrivateICSIntfIndex].EnableAsPrivate( );
    }

    m_lPublicICSIntfIndex = -1;
    m_lPrivateICSIntfIndex = -1;
}