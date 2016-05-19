//*****************************************************************************
//
// Microsoft Windows Media
// Copyright (C) Microsoft Corporation. All rights reserved.
//
// FileName: DBAuthPlugin.cpp
//
// Abstract:
//
//*****************************************************************************
#include "stdafx.h"
#include "DBAuth.h"
#include "DBAuthPlugin.h"
#include "DBAuthAdmin.h"

#define AUTHORIZED_USERS_PROPERTY L"AuthorizedUsers"

const int NUM_AUTHORIZED_EVENTS = 1;

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
CDBAuthPlugin::CDBAuthPlugin()
    : m_pSids( NULL )
{
    InitializeCriticalSection( &m_CritSec );
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
CDBAuthPlugin::~CDBAuthPlugin()
{
    // Lock when accessing member variables
    if( NULL != m_pSids )
    {
        delete m_pSids;
    }
    DeleteCriticalSection( &m_CritSec );
}

///////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CDBAuthPlugin::Lock()
{
    EnterCriticalSection( &m_CritSec );
}

///////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CDBAuthPlugin::Unlock()
{
    LeaveCriticalSection( &m_CritSec );
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CDBAuthPlugin::InitializePlugin
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
    IWMSServer* pWMSServer = NULL;
    DWORD dwCount = 0;

    // Lock when accessing member variables
    Lock();

    do
    {
        hr = pServerContext->GetAndQueryIUnknownValue( const_cast<LPWSTR>( WMS_SERVER ), WMS_SERVER_ID,
                                                   IID_IWMSServer, (IUnknown**)&pWMSServer, 0 );
        if( FAILED( hr ) )
        {
            break;
        }

        ATLASSERT( NULL != pWMSServer );
        m_spServer.Attach( pWMSServer );
        if( NULL == m_spServer.p )
        {
            hr = E_NOINTERFACE;
            break;
        }

        m_spNamedValues = pNamedValues;
        if( NULL == m_spNamedValues.p )
        {
            hr = E_NOINTERFACE;
            break;
        }

        CSids *pSids = new CSids;
        if( NULL == pSids )
        {
            hr = E_OUTOFMEMORY;
            break;
        }

        if( NULL != m_pSids )
        {
            delete m_pSids;
        }
        m_pSids = pSids;
        pSids = NULL;

        // Get setting from namespace
        hr = m_pSids->GetAuthorizedUsers( m_spNamedValues );
        if( FAILED( hr ) )
        {
            // ignore error when not able to get setting from namespace
            hr = S_OK;
        }


        // TODO: Add any additional initialization here

    }
    while( FALSE );

    Unlock();

    return( hr );
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CDBAuthPlugin::EnablePlugin( long *pdwFlags, long *pdwHeartbeatPeriod )
{
    if( NULL == pdwFlags || NULL == pdwHeartbeatPeriod )
    {
        return ( E_POINTER );
    }

    // Set the heartbeat period in milliseconds
    *pdwHeartbeatPeriod = 0000;

    return ( S_OK );
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CDBAuthPlugin::DisablePlugin()
{
    return ( S_OK );
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CDBAuthPlugin::ShutdownPlugin()
{
    HRESULT hr = S_OK;

    // Lock when accessing member variables
    Lock();

    m_spServer = NULL;
    m_spNamedValues = NULL;

    if( NULL != m_pSids )
    {
        delete m_pSids;
    }
    m_pSids = NULL;

    Unlock();

    return( hr );
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CDBAuthPlugin::OnHeartbeat()
{
    HRESULT hr = S_OK;

    // TODO: Add code that should execute on every Heartbeat Period
    return( hr );
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CDBAuthPlugin::GetCustomAdminInterface( IDispatch **ppValue )
{
    if( ( NULL == ppValue ) )
    {
        return( E_INVALIDARG );
    }

    HRESULT hr = S_OK;

    *ppValue = NULL;

    CComObject<CDBAuthAdmin> *spDBAuthAdmin;

    hr = CComObject<CDBAuthAdmin>::CreateInstance( &spDBAuthAdmin );
    if( SUCCEEDED( hr ) )
    {
        hr = spDBAuthAdmin->Initialize( this /*, m_spNamedValues */ );
        if( SUCCEEDED( hr ) )
        {
            hr = spDBAuthAdmin->QueryInterface( IID_IDispatch, (void **) ppValue );
        }
    }

    return( hr );
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CDBAuthPlugin::GetAuthorizedEvents( VARIANT *pvarAuthorizedEvents )
{
    if( NULL == pvarAuthorizedEvents )
    {
        return( E_INVALIDARG );
    }

    HRESULT hr = S_OK;
    int nIndex = 0;
    WMS_EVENT_TYPE wmsAuthorizedEvents[ NUM_AUTHORIZED_EVENTS ];

    wmsAuthorizedEvents[ nIndex++ ] = WMS_EVENT_OPEN;

    hr = CreateArrayOfEvents( pvarAuthorizedEvents, wmsAuthorizedEvents, NUM_AUTHORIZED_EVENTS );
    return( hr );
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CDBAuthPlugin::AuthorizeEvent
(
    WMS_EVENT *pEvent,
    IWMSContext *pUserCtx,
    IWMSContext *pPresentationCtx,
    IWMSCommandContext *pCommandCtx,
    IWMSEventAuthorizationCallback *pCallback,
    VARIANT Context
)
{
    HRESULT hrAuth = HRESULT_FROM_WIN32( ERROR_NO_SUCH_USER );
    HRESULT hr = S_OK;

    //
    // The event information must be valid in order to determine what
    // type of event needs to be authorized.
    //
    if( ( NULL == pEvent ) )
    {
        return( E_INVALIDARG );
    }

    //
    // The input contexts may be NULL if they are not defined for the
    // specified event.  Verify that any context you intend to use is
    // not NULL before using it.  If you need to use context that is
    // NULL, return E_INVALIDARG to notify the server of the problem.
    //

    switch( pEvent->Type )
    {
    case WMS_EVENT_CONNECT:
        break;
    case WMS_EVENT_OPEN:
        hrAuth = OnAuthorizeOpen( pUserCtx, pPresentationCtx, pCommandCtx );
        break;
    case WMS_EVENT_PLAY:
        break;
    case WMS_EVENT_GET_PARAMETER:
        break;
    case WMS_EVENT_SET_PARAMETER:
        break;
    case WMS_EVENT_VALIDATE_PUSH_DISTRIBUTION:
        break;
    }

    hr = pCallback->OnAuthorizeEvent( hrAuth, Context );
    return( hr );
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
HRESULT CDBAuthPlugin::CreateArrayOfEvents( VARIANT *pvarEvents, WMS_EVENT_TYPE *pWMSEvents, long nNumEvents)
{
    HRESULT hr = S_OK;
    long iEvents = 0;
    SAFEARRAY *psa = NULL;
    SAFEARRAYBOUND rgsabound[ 1 ];

    if( NULL == pvarEvents )
    {
        return( E_POINTER );
    }

    if( NULL == pWMSEvents || 0 >= nNumEvents )
    {
        return( E_INVALIDARG );
    }

    rgsabound[ 0 ].lLbound = 0;
    rgsabound[ 0 ].cElements = nNumEvents;

    psa = SafeArrayCreate( VT_VARIANT, 1, rgsabound );

    if( NULL == psa )
    {
        return ( E_OUTOFMEMORY );
    }

    for( iEvents = 0; iEvents < nNumEvents && SUCCEEDED( hr ); iEvents++ )
    {
        VARIANT varElement;

        VariantInit( &varElement );

        V_VT( &varElement ) = VT_I4;
        V_I4( &varElement ) = pWMSEvents[ iEvents ];

        hr = SafeArrayPutElement( psa, &iEvents, &varElement );
        VariantClear( &varElement );
    }

    if( FAILED( hr ) )
    {
        SafeArrayDestroy( psa );
        psa = NULL;
    }
    else
    {
        V_VT( pvarEvents ) = VT_ARRAY | VT_VARIANT;
        V_ARRAY( pvarEvents ) = psa;
    }

    return( hr );
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// Event Authorization Helper Functions
HRESULT CDBAuthPlugin::OnAuthorizeOpen( IWMSContext *pUserCtx, IWMSContext *pPresentationCtx, IWMSCommandContext *pCommandCtx )
{
    // return S_OK if Open is Authorized, otherwise return E_ACCESSDENIED

    HRESULT hr = S_OK;
    BOOL bFound = FALSE;
    LPWSTR wstrUser = NULL;

    do
    {
        hr = pUserCtx->GetStringValue( const_cast<LPWSTR>( WMS_USER_NAME ),
                                       WMS_USER_NAME_ID,
                                       &wstrUser,
                                       WMS_CONTEXT_GET_PROPERTY_STRING_BY_REFERENCE );
        if( FAILED( hr ) )
        {
            break;
        }

        DWORD dwIndex = 0;
        hr = VerifyUserMembership( wstrUser, &bFound, &dwIndex );

    }
    while( FALSE );

    if( SUCCEEDED( hr ) && bFound )
    {
        return( S_OK );
    }
    else
    {
        return( E_ACCESSDENIED );
    }
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
HRESULT CDBAuthPlugin::VerifyUserMembership( LPCWSTR pwszUserName, BOOL *pfFound, DWORD *pdwIndex )
{

    if( ( NULL == pfFound ) || ( NULL == pdwIndex ) )
    {
        return( E_INVALIDARG );
    }

    if( ( NULL == pwszUserName ) || ( L'\0' == (WCHAR*) *pwszUserName ) )
    {
        return( E_INVALIDARG );
    }

    HRESULT hr = S_OK;
    PSID pSid = NULL;

    do
    {
        hr = GetSIDFromName( pwszUserName, &pSid );
        if( FAILED( hr ) )
        {
            break;
        }

        *pfFound = VerifySidMembership( pSid, pdwIndex );

    }
    while( FALSE );

    if( NULL != pSid )
    {
        LocalFree( pSid );
        pSid = NULL;
    }

    return( hr );

}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
BOOL CDBAuthPlugin::VerifySidMembership( PSID pSid, DWORD *pdwIndex )
{

    BOOL bFound = FALSE;

    if( ( NULL == pSid ) || ( NULL == pdwIndex ) )
    {
        return( bFound );
    }

    // Lock when accessing member variables
    Lock();

    if( NULL != m_pSids )
    {
        bFound = m_pSids->MatchSid( pSid, pdwIndex );
    }

    Unlock();

    return( bFound );

}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
HRESULT CDBAuthPlugin::AddUser( LPCWSTR pwszUserName )
{
    if( ( NULL == pwszUserName ) || ( L'\0' == (WCHAR*) *pwszUserName ) )
    {
        return( E_INVALIDARG );
    }

    HRESULT hr = S_OK;
    PSID pSid = NULL;
    CSids *pNewSids = NULL;

    // Lock when accessing member variables
    Lock();

    do
    {
        if( NULL == m_pSids )
        {
            hr = E_FAIL;
            break;
        }

        hr = GetSIDFromName( pwszUserName, &pSid );
        if( FAILED( hr ) )
        {
            break;
        }

        BOOL bFound = FALSE;
        DWORD dwIndex = 0;
        bFound = VerifySidMembership( pSid, &dwIndex );

        if( bFound )
        {
            // the user already in the list
            // no further action required
            break;
        }

        // create a new CSids and add the user Sid
        hr = m_pSids->CopyAndAddSid( pSid, &pNewSids );
        if( FAILED( hr ) )
        {
            break;
        }

        // persist data using the newly created Sid array
        hr = pNewSids->SetAuthorizedUsers( m_spNamedValues );
        if( FAILED( hr ) )
        {
            break;
        }

        // succeeded in creating new Sids and persist data
        // change member variables
        m_pSids->CleanUp( FALSE );
        delete m_pSids;
        m_pSids = pNewSids;

        // set pNewSids and pSid to NULL so they won't be deleted later
        pNewSids = NULL;
        pSid = NULL;

    }
    while( FALSE );

    Unlock();

    if( NULL != pNewSids )
    {
        pNewSids->CleanUp( FALSE );
        delete pNewSids;
    }

    if( NULL != pSid )
    {
        LocalFree( pSid );
        pSid = NULL;
    }

    return( hr );

}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
HRESULT CDBAuthPlugin::RemoveUser( LPCWSTR pwszUserName )
{
    if( ( NULL == pwszUserName ) || ( L'\0' == (WCHAR*) *pwszUserName ) )
    {
        return( E_INVALIDARG );
    }

    HRESULT hr = S_OK;
    BOOL bFound = FALSE;
    CSids *pNewSids = NULL;

    // Lock when accessing member variables
    Lock();

    do
    {
        if( NULL == m_pSids )
        {
            hr = E_FAIL;
            break;
        }

        if( 0 == m_pSids->m_cItems )
        {
            // no user in the list
            // no further action required
            break;
        }

        DWORD dwIndexFound = 0;
        hr = VerifyUserMembership( pwszUserName, &bFound, &dwIndexFound );
        if( FAILED( hr ) )
        {
            break;
        }

        if( !bFound )
        {
            // the user is not in the list
            // no further action required
            break;
        }

        hr = m_pSids->CopyAndRemoveItem( dwIndexFound, &pNewSids );
        if( FAILED( hr ) )
        {
            break;
        }

        // persist data using the newly created Sid array
        hr = pNewSids->SetAuthorizedUsers( m_spNamedValues );
        if( FAILED( hr ) )
        {
            break;
        }

        // succeeded in creating new Sids and persist data
        // change member variables
        LocalFree( m_pSids->m_pSidList[ dwIndexFound ] );
        m_pSids->CleanUp( FALSE );
        delete m_pSids;
        m_pSids = pNewSids;

        // set pNewSids to NULL so they won't be deleted later
        pNewSids = NULL;

    }
    while( FALSE );

    Unlock();

    if( NULL != pNewSids )
    {
        pNewSids->CleanUp( FALSE );
        delete pNewSids;
    }

    return( hr );

}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
HRESULT CDBAuthPlugin::GetUsers( BSTR *pbstrUserNames )
{
    if( NULL == pbstrUserNames )
    {
        return( E_INVALIDARG );
    }

    HRESULT hr = S_OK;

    // Lock when accessing member variables
    Lock();

    do
    {
        if( NULL == m_pSids )
        {
            hr = E_FAIL;
            break;
        }

        hr = m_pSids->GetUsers( pbstrUserNames );

    }
    while( FALSE );

    Unlock();

    return( hr );

}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
HRESULT CDBAuthPlugin::GetSIDFromName( LPCWSTR pwszUserName, PSID *ppSid )
{
    // Translate the user name into a SID

    if( NULL == ppSid )
    {
        return( E_INVALIDARG );
    }

    HRESULT hr = S_OK;
    DWORD dwRetVal = 0;
    SID_NAME_USE SidNameUse;
    DWORD cbSid = 0;
    DWORD cbDomainName = 0;
    LPWSTR wstrDomainName = NULL;
    BOOL fRetVal = FALSE;

    do
    {
        if( !LookupAccountName( NULL,
                                pwszUserName,
                                NULL,
                                &cbSid,
                                NULL,
                                &cbDomainName,
                                &SidNameUse ) )
        {
            dwRetVal = GetLastError();
            if( ERROR_INSUFFICIENT_BUFFER != dwRetVal )
            {
                hr = HRESULT_FROM_WIN32( dwRetVal );
                break;
            }
        }

        *ppSid = (SID *) LocalAlloc( LPTR, cbSid );
        if( NULL == *ppSid )
        {
            hr = E_OUTOFMEMORY;
            break;
        }

        wstrDomainName = new WCHAR[ cbDomainName ];
        if( NULL == wstrDomainName )
        {
            hr = E_OUTOFMEMORY;
            break;
        }

        fRetVal = LookupAccountName( NULL,
                                     pwszUserName,
                                     *ppSid,
                                     &cbSid,
                                     wstrDomainName,
                                     &cbDomainName,
                                     &SidNameUse );

        if( !fRetVal )
        {
            dwRetVal = GetLastError();
            hr = HRESULT_FROM_WIN32( dwRetVal );
            break;
        }

    }
    while( FALSE );

    if( NULL != wstrDomainName )
    {
        delete [] wstrDomainName;
    }

    if( FAILED( hr ) && ( NULL != *ppSid ) )
    {
        LocalFree( *ppSid );
        *ppSid = NULL;
    }

    return( hr );
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void CSids::CleanUp( BOOL fFreeSid )
{

    if( fFreeSid && ( 0 != m_cItems ) && ( NULL != m_pSidList ) )
    {
        // Free the pSidList information
        for( DWORD dwIndex = 0; dwIndex < m_cItems; dwIndex ++ )
        {
            if( NULL != m_pSidList[ dwIndex ] )
            {
                LocalFree( m_pSidList[ dwIndex ] );
            }
        }
    }

    m_cItems = 0;

    if( NULL != m_pSidList )
    {
        delete [] m_pSidList;
        m_pSidList = NULL;
    }

    return;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
BOOL CSids::MatchSid( PSID pSid, DWORD *pdwIndex )
{

    BOOL bFound = FALSE;

    do
    {
        if( NULL == m_pSidList )
        {
            break;
        }

        for( DWORD dwIndex = 0; dwIndex < m_cItems; dwIndex ++ )
        {
            if( ( NULL != m_pSidList[ dwIndex ] )
                && EqualSid( m_pSidList[ dwIndex], pSid ) )
            {
                bFound = TRUE;
                *pdwIndex = dwIndex;
                break;
            }
        }
    }
    while( FALSE );

    return( bFound );

}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
HRESULT CSids::CopyAndAddSid( PSID pSid, CSids **ppSids )
{
    HRESULT hr = S_OK;
    CSids *pNewSids = NULL;

    do
    {
        pNewSids = new CSids;
        if( NULL == pNewSids )
        {
            hr = E_OUTOFMEMORY;
            break;
        }

        pNewSids->m_pSidList = new PSID[ m_cItems + 1 ];
        if( NULL == pNewSids->m_pSidList )
        {
            hr = E_OUTOFMEMORY;
            break;
        }

        pNewSids->m_cItems = m_cItems + 1;

        // copy the Sids to the new array
        for( DWORD dwIndex = 0; dwIndex < m_cItems; dwIndex++ )
        {
            pNewSids->m_pSidList[ dwIndex ] = m_pSidList[ dwIndex ];
        }
        pNewSids->m_pSidList[ m_cItems ] = pSid;

    }
    while( FALSE );

    if( SUCCEEDED( hr ) )
    {
        *ppSids = pNewSids;
    }
    else if( NULL != pNewSids )
    {
        pNewSids->CleanUp( FALSE );
        delete pNewSids;
    }

    return( hr );
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
HRESULT CSids::CopyAndRemoveItem( DWORD dwIndexRemove, CSids **ppSids )
{
    HRESULT hr = S_OK;
    CSids *pNewSids = NULL;
    DWORD dwIndex = 0;

    do
    {
        if( ( 0 == m_cItems ) || ( dwIndex >= m_cItems ) )
        {
            hr = E_INVALIDARG;
            break;
        }

        pNewSids = new CSids;
        if( NULL == pNewSids )
        {
            hr = E_OUTOFMEMORY;
            break;
        }

        if( m_cItems > 1 )
        {
            pNewSids->m_pSidList = new PSID[ m_cItems - 1 ];
            if( NULL == pNewSids->m_pSidList )
            {
                hr = E_OUTOFMEMORY;
                break;
            }

            pNewSids->m_cItems = m_cItems - 1;

            // copy the Sids to the new array
            for( dwIndex = 0; dwIndex < dwIndexRemove; dwIndex ++ )
            {
                pNewSids->m_pSidList[ dwIndex ] = m_pSidList[ dwIndex ];
            }
            for( dwIndex = dwIndexRemove + 1; dwIndex < m_cItems; dwIndex ++ )
            {
                pNewSids->m_pSidList[ dwIndex - 1 ] = m_pSidList[ dwIndex ];
            }
        }

    }
    while( FALSE );

    if( SUCCEEDED( hr ) )
    {
        *ppSids = pNewSids;
    }
    else if( NULL != pNewSids )
    {
        pNewSids->CleanUp( FALSE );
        delete pNewSids;
    }

    return( hr );
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
HRESULT CSids::SetAuthorizedUsers( IWMSNamedValues *pNamedValues )
{
    if( NULL == pNamedValues )
    {
        return( E_INVALIDARG );
    }

    HRESULT hr = S_OK;
    DWORD dwIndex = 0;
    IWMSNamedValue * pValue = NULL;
    CComVariant varValue;
    BSTR bstrPropName = NULL;
    BYTE *pBuf = NULL;
    SAFEARRAY* psa = NULL;
    BOOL fPsaAccessed = FALSE;

    // size of m_cItems and one DWORD for each item to store the size of Sid
    DWORD dwBufSize = sizeof( m_cItems ) + m_cItems * sizeof( DWORD );

    do
    {
        // find the total buffer size required to store the Sid list
        for( dwIndex = 0; dwIndex < m_cItems; dwIndex ++ )
        {
            if( NULL != m_pSidList[ dwIndex ] )
            {
                dwBufSize += GetLengthSid( m_pSidList[ dwIndex ] );
            }
        }

        // create a safearry to store the Sid list
        SAFEARRAYBOUND rgsabound[ 1 ];
        rgsabound[ 0 ].lLbound = 0;
        rgsabound[ 0 ].cElements = dwBufSize;
        psa = SafeArrayCreate( VT_UI1, 1, rgsabound );
        if( NULL == psa )
        {
            hr = E_OUTOFMEMORY;
            break;
        }

        V_VT( &varValue ) = VT_ARRAY | VT_UI1;
        V_ARRAY( &varValue ) = psa;

        hr = SafeArrayAccessData( psa, (void **) &pBuf );
        if( FAILED( hr ) )
        {
            break;
        }

        fPsaAccessed = TRUE;

        // copy Sid list to buffer
        BYTE *p = pBuf;
        memcpy( p, &m_cItems, sizeof( m_cItems ) );
        p += sizeof( m_cItems );
        for( dwIndex = 0; dwIndex < m_cItems; dwIndex ++ )
        {
            DWORD dwSize = 0;
            if( NULL == m_pSidList[ dwIndex ] )
            {
                memcpy( p, &dwSize, sizeof( dwSize ) );
                p += sizeof( dwSize );
            }
            else
            {
                dwSize = GetLengthSid( m_pSidList[ dwIndex ] );
                memcpy( p, &dwSize, sizeof( dwSize ) );
                p += sizeof( dwSize );
                memcpy( p, m_pSidList[ dwIndex ], dwSize );
                p += dwSize;
            }
        }

        bstrPropName = SysAllocString( AUTHORIZED_USERS_PROPERTY );
        if( NULL == bstrPropName )
        {
            hr = E_OUTOFMEMORY;
            break;
        }

        // persist the sid data to namespace
        hr = pNamedValues->Add( bstrPropName, varValue, &pValue );

    }
    while( FALSE );

    //
    // cleanup local variables
    //
    if( ( NULL == psa ) && fPsaAccessed )
    {
        SafeArrayUnaccessData( psa );
    }

    if( NULL != pValue )
    {
        pValue->Release();
    }

    if( NULL != bstrPropName )
    {
        SysFreeString( bstrPropName );
    }

    return( hr );
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
HRESULT CSids::GetAuthorizedUsers( IWMSNamedValues *pNamedValues )
{
    if( NULL == pNamedValues )
    {
        return( E_INVALIDARG );
    }

    CleanUp();

    HRESULT hr = S_OK;
    IWMSNamedValue * pValue = NULL;
    SAFEARRAY* psa = NULL;
    BOOL fPsaAccessed = FALSE;
    CComVariant varValue;
    VARIANT varPropName;
    VariantInit( &varPropName );

    do
    {

        BSTR bstrPropName = NULL;
        bstrPropName = SysAllocString( AUTHORIZED_USERS_PROPERTY );
        if( NULL == bstrPropName )
        {
            hr = E_OUTOFMEMORY;
            break;
        }

        // bstrPropName will be free by varPropName
        V_VT(&varPropName) = VT_BSTR;
        V_BSTR( &varPropName ) = bstrPropName;

        // Get the authorized users from namespace
        hr = pNamedValues->get_Item( varPropName, &pValue );
        if( FAILED( hr ) )
        {
            // no value found
            break;
        }

        hr = pValue->get_Value( &varValue );
        if( FAILED( hr ) )
        {
            break;
        }

        if( ( VT_ARRAY | VT_UI1 ) != V_VT( &varValue ) )
        {
            hr = E_FAIL;
            break;
        }

        psa = V_ARRAY( &varValue );

        DWORD dwBufSize = psa->rgsabound[ 0 ].cElements;
        DWORD dwCount = 0;
        if( dwBufSize < sizeof( dwCount ) )
        {
            hr = E_FAIL;
            break;
        }

        BYTE *pBuf = NULL;
        hr = SafeArrayAccessData( psa, (void **) &pBuf );
        if( FAILED( hr ) )
        {
            break;
        }

        fPsaAccessed = TRUE;

        memcpy( &dwCount, pBuf, sizeof( dwCount ) );
        pBuf += sizeof( dwCount );
        dwBufSize -= sizeof( dwCount );

        m_pSidList = new PSID[ dwCount ];
        if( NULL == m_pSidList )
        {
            hr = E_OUTOFMEMORY;
            break;
        }

        m_cItems = dwCount;

        for( DWORD dwIndex = 0; dwIndex < dwCount; dwIndex ++ )
        {
            m_pSidList[ dwIndex ] = NULL;
        }

        for( DWORD dwIndex = 0; dwIndex < dwCount; dwIndex ++ )
        {
            DWORD dwSize = 0;
            if( dwBufSize < sizeof( dwSize ) )
            {
                hr = E_FAIL;
                break;
            }

            memcpy( &dwSize, pBuf, sizeof( dwSize ) );
            pBuf += sizeof( dwSize );
            dwBufSize -= sizeof( dwSize );

            if( 0 != dwSize )
            {
                if( dwBufSize < dwSize )
                {
                    hr = E_FAIL;
                    break;
                }

                m_pSidList[ dwIndex ] = (SID *) LocalAlloc( LPTR, dwSize );
                if( NULL == m_pSidList[ dwIndex ] )
                {
                    hr = E_OUTOFMEMORY;
                    break;
                }
                memcpy( m_pSidList[ dwIndex ], pBuf, dwSize );
                pBuf += dwSize;
                dwBufSize -= dwSize;
            }
        }
    }
    while( FALSE );

    if( FAILED( hr ) )
    {
        CleanUp();
    }

    if( ( NULL == psa ) && fPsaAccessed )
    {
        SafeArrayUnaccessData( psa );
    }

    //
    // cleanup local variables
    //
    if( NULL != pValue )
    {
        pValue->Release();
    }

    VariantClear( &varPropName );

    return( hr );
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
HRESULT CSids::GetUsers( BSTR *pbstrUserNames )
{
    if( NULL == pbstrUserNames )
    {
        return( E_INVALIDARG );
    }

    HRESULT hr = S_OK;
    PSID pSid = NULL;
    WCHAR *pwszUserNames = NULL;
    *pbstrUserNames = NULL;

    do
    {
        // Get the total bufer size needed to store all users
        DWORD dwBufSize = 0;
        for( DWORD dwIndex = 0; dwIndex < m_cItems; dwIndex ++ )
        {
            if( NULL != m_pSidList[ dwIndex ] )
            {
                DWORD cbUserName = 0;
                DWORD cbDomainName = 0;
                SID_NAME_USE eUse;
                BOOL fOk = LookupAccountSid( NULL,      // name of local or remote computer
                                m_pSidList[ dwIndex ],  // security identifier
                                NULL,                   // account name buffer
                                &cbUserName,            // size of account name buffer
                                NULL,                   // domain name
                                &cbDomainName,          // size of domain name buffer
                                &eUse                   // SID type
                                );
                dwBufSize = dwBufSize + cbUserName + cbDomainName;
            }
        }

        if( 0 == dwBufSize )
        {
            break;
        }

        pwszUserNames = new WCHAR[ dwBufSize ];
        if( NULL == pwszUserNames )
        {
            hr = E_OUTOFMEMORY;
            break;
        }

        WCHAR *p = pwszUserNames;
        for( DWORD dwIndex = 0; dwIndex < m_cItems; dwIndex ++ )
        {
            if( NULL != m_pSidList[ dwIndex ] )
            {
                DWORD cbUserName = 0;
                DWORD cbDomainName = 0;
                SID_NAME_USE eUse;
                BOOL fOk = LookupAccountSid( NULL,      // name of local or remote computer
                                m_pSidList[ dwIndex ],  // security identifier
                                NULL,                   // account name buffer
                                &cbUserName,            // size of account name buffer
                                NULL,                   // domain name
                                &cbDomainName,          // size of domain name buffer
                                &eUse                   // SID type
                                );

                fOk = LookupAccountSid( NULL,           // name of local or remote computer
                                m_pSidList[ dwIndex ],  // security identifier
                                p + cbDomainName,       // account name buffer
                                &cbUserName,            // size of account name buffer
                                p,                      // domain name
                                &cbDomainName,          // size of domain name buffer
                                &eUse                   // SID type
                                );
                if( !fOk )
                {
                    continue;
                }

                if( 0 != cbDomainName )
                {
                    p += cbDomainName;
                    *p = L'\\';
                    p++;
                }

                // seperate users with a space
                p += cbUserName;
                *p = L' ';
                p++;
            }

        }

        // null term the last user
        if( p > pwszUserNames )
        {
            *( p - 1 ) = L'\0';
        }
        else
        {
            *p = L'\0';
        }

        *pbstrUserNames = SysAllocString( pwszUserNames );
        if( NULL == *pbstrUserNames )
        {
            hr = E_OUTOFMEMORY;
        }

    }
    while( FALSE );

    if( NULL != pwszUserNames )
    {
        delete [] pwszUserNames;
    }

    return( hr );

}

