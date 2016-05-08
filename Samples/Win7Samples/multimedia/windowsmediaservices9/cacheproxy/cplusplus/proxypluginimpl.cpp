//*****************************************************************************
//
// Microsoft Windows Media
// Copyright (C) Microsoft Corporation. All rights reserved.
//
// FileName:            ProxyPluginImpl.cpp
//
// Abstract:            
//
//*****************************************************************************

#include "stdafx.h"

#include <limits.h>
#include "WMSBasicPlugin.h"

#include "nserror.h"
#include "WMSContextNames.h"
#include "ProxyPlugin.h"
#include "ProxyPluginImpl.h"


// Since the plugin supports protocol rollver, we have a hard-coded list of the 
// protocols which are supported.  The last entry signifies the end of the array.
LPWSTR g_ProxyProtocols[] = { L"HTTP", L"RTSP", NULL };


/////////////////////////////////////////////////////////////////////////////
//
// [CProxyPlugin]
//
/////////////////////////////////////////////////////////////////////////////
CProxyPlugin::CProxyPlugin()
{
    m_pICacheProxyServer = NULL;
}



/////////////////////////////////////////////////////////////////////////////
//
// [~CProxyPlugin]
//
/////////////////////////////////////////////////////////////////////////////
CProxyPlugin::~CProxyPlugin()
{
}



/////////////////////////////////////////////////////////////////////////////
//
// [InitializePlugin]
//
/////////////////////////////////////////////////////////////////////////////
HRESULT STDMETHODCALLTYPE 
CProxyPlugin::InitializePlugin( 
                        IWMSContext *pServerContext,
                        IWMSNamedValues *pNamedValues,
                        IWMSClassObject *pClassFactory
                        )
{
    HRESULT hr = S_OK;

    if( NULL == pNamedValues )
    {
        return ( E_INVALIDARG );
    }

    // A pointer to the IWMSCacheProxyServer is located in the server context
    // passed into the plugin when it is initialized.  The plugin will need to
    // query for this value and store it.

    hr = pServerContext->GetAndQueryIUnknownValue( 
                                    WMS_SERVER_CACHE_MANAGER,
                                    WMS_SERVER_CACHE_MANAGER_ID,
                                    IID_IWMSCacheProxyServer,
                                    ( IUnknown ** ) &m_pICacheProxyServer,
                                    0 );
    if( ( FAILED( hr ) ) || ( NULL == m_pICacheProxyServer ) )
    {
        hr = FAILED( hr ) ? hr : E_FAIL;
        goto abort;
    }

abort:
    return( hr );
}



/////////////////////////////////////////////////////////////////////////////
//
// [EnablePlugin]
//
/////////////////////////////////////////////////////////////////////////////
HRESULT STDMETHODCALLTYPE 
CProxyPlugin::EnablePlugin( long *pdwFlags, long *pdwHeartbeatPeriod )
{
    if( ( NULL == pdwFlags ) || ( NULL == pdwHeartbeatPeriod ) )
    {
        return( E_INVALIDARG );
    }

    *pdwFlags = 0;
    *pdwHeartbeatPeriod = 0;

    return( S_OK );
}



/////////////////////////////////////////////////////////////////////////////
//
// [DisablePlugin]
//
/////////////////////////////////////////////////////////////////////////////
HRESULT STDMETHODCALLTYPE 
CProxyPlugin::DisablePlugin()
{
    return( S_OK );
}



/////////////////////////////////////////////////////////////////////////////
//
// [ShutdownPlugin]
//
/////////////////////////////////////////////////////////////////////////////
HRESULT STDMETHODCALLTYPE 
CProxyPlugin::ShutdownPlugin()
{
    if( NULL != m_pICacheProxyServer )
    {
        m_pICacheProxyServer->Release();
        m_pICacheProxyServer = NULL;
    }

    return( S_OK );
}



/////////////////////////////////////////////////////////////////////////////
//
// [GetCustomAdminInterface]
//
/////////////////////////////////////////////////////////////////////////////
HRESULT STDMETHODCALLTYPE 
CProxyPlugin::GetCustomAdminInterface( IDispatch **ppValue )
{
    if( NULL == ppValue )
    {
        return( E_INVALIDARG );
    }

    *ppValue = NULL;
    return( S_OK );
}



/////////////////////////////////////////////////////////////////////////////
//
// [OnHeartbeat]
//
/////////////////////////////////////////////////////////////////////////////
HRESULT STDMETHODCALLTYPE 
CProxyPlugin::OnHeartbeat()
{
    return( S_OK );
}


/////////////////////////////////////////////////////////////////////////////
//
// [QueryCache]
//
/////////////////////////////////////////////////////////////////////////////
HRESULT STDMETHODCALLTYPE 
CProxyPlugin::QueryCache( BSTR bstrOriginUrl,
                          IWMSContext *pUserContext,
                          IWMSCommandContext *pCommandContext,
                          IWMSContext *pPresentationContext,
                          long QueryType, 
                          IWMSCacheProxyCallback *pCallback,
                          VARIANT varContext
                          )
{
    HRESULT hr = S_OK;
    COpState *pOpState = NULL;
    LPWSTR pszHostName = NULL;

    if( ( NULL == bstrOriginUrl) ||
        ( NULL == pCommandContext ) ||
        ( NULL == pPresentationContext ) ||
        ( NULL == pCallback ) )
    {
        return( E_INVALIDARG );
    }

    // Since this plugin just supports proxy, we always return (in the callback)
    // WMS_CACHE_QUERY_MISS.  If this plugin were also to do caching, this is 
    // where the cache database would be queried.  If a hit is made, then the
    // expiration of the cache entry will be checked, and if it expired a 
    // freshness check would need to be done.  A freshness check is done
    // through CompareContentInformation, an async call.  Rather than waiting
    // for the response, the plugin should return from this method and 
    // call the callback (OnQueryCache) in OnCompareContentInformation.
    //
    // If OnCompareContentInformation reports the cached entry is up-to-date,
    // a hit could be reported.  Otherwise a miss will need to be reported,
    // and the plugin will need to wait till QueryCacheMissPolicy.

    hr = pCallback->OnQueryCache( hr,
                                  WMS_CACHE_QUERY_MISS,
                                  NULL,
                                  NULL,
                                  NULL,
                                  varContext );

    return( S_OK );
}


/////////////////////////////////////////////////////////////////////////////
//
// [QueryCacheMissPolicy]
//
/////////////////////////////////////////////////////////////////////////////
HRESULT STDMETHODCALLTYPE 
CProxyPlugin::QueryCacheMissPolicy( BSTR bstrOriginUrl,
                                    IWMSContext *pUserContext,
                                    IWMSCommandContext *pCommandContext,
                                    IWMSContext *pPresentationContext,
                                    IUnknown *pCachePluginContext,
                                    long QueryType, 
                                    IWMSCacheProxyCallback *pCallback,
                                    VARIANT varContext
                                    )
{
    VARIANT varContext2;
    COpState *pOpState = NULL;
    HRESULT hr = S_OK;
    BOOL fRet;    

    if( ( NULL == bstrOriginUrl) ||
        ( NULL == pCommandContext ) ||
        ( NULL == pPresentationContext ) ||
        ( NULL == pCallback ) )
    {
        return( E_INVALIDARG );
    }

    if( QueryType & WMS_CACHE_QUERY_REVERSE_PROXY )
    {
        hr = pCallback->OnQueryCacheMissPolicy( S_OK,
                                                WMS_CACHE_QUERY_MISS_SKIP,
                                                NULL,
                                                NULL,
                                                NULL,
                                                varContext );
        return( hr );
    }

    if( ( QueryType & WMS_CACHE_QUERY_CACHE_EVENT ) || 
        ( QueryType & WMS_CACHE_QUERY_GET_CONTENT_INFO ) )
    {
        hr = pCallback->OnQueryCacheMissPolicy( S_OK,
                                                WMS_CACHE_QUERY_MISS_FORWARD_REQUEST,
                                                NULL,
                                                NULL,
                                                NULL,
                                                varContext );         
        return( hr );
    }

    pOpState = new COpState;
    if( NULL == pOpState )
    {
        return( E_OUTOFMEMORY );
    }

    pOpState->m_Op = COpState::OP_QUERY_CACHE_MISS_POLICY;
    pOpState->m_bstrOriginUrl = bstrOriginUrl;
    pOpState->m_pPresentationContext = pPresentationContext;
    pOpState->m_pCacheProxyCallback = pCallback;
    pOpState->m_varContext = varContext;

    pOpState->m_pPresentationContext->AddRef();
    pOpState->m_pCacheProxyCallback->AddRef();

    if( NULL == pOpState->m_bstrOriginUrl.m_str )
    {
        pOpState->Release();
        return( E_OUTOFMEMORY );
    }

    fRet = ParseUrl( bstrOriginUrl,
                     &pOpState->m_bstrProtocol,
                     &pOpState->m_bstrHost,
                     &pOpState->m_bstrPath,
                     &pOpState->m_wPort );

    if( !fRet )
    {
        pOpState->Release();
        return( E_FAIL );
    }
    
    // Initiate protocol rollover by calling OnGetContentInformation with an
    // appropriate HRESULT

    VariantInit( &varContext2 );
    V_VT( &varContext2 ) = VT_UNKNOWN;
    V_UNKNOWN( &varContext2 ) = (IUnknown *) pOpState;

    pOpState->m_dwProtocolIndex = -1;

    OnGetContentInformation( NS_E_CONNECTION_FAILURE, NULL, varContext2 );

    return( S_OK );
}

/////////////////////////////////////////////////////////////////////////////
//
// [OnGetContentInformation]
//
// IWMSCacheProxyServerCallback
//
/////////////////////////////////////////////////////////////////////////////
HRESULT STDMETHODCALLTYPE 
CProxyPlugin::OnGetContentInformation(
                    long lHr,
                    IWMSContext *pContentInfo,
                    VARIANT varContext
                    )
{
    HRESULT returnHr = (HRESULT) lHr;
    HRESULT hr = S_OK;
    COpState *pOpState = NULL;
    WMS_CACHE_QUERY_MISS_RESPONSE CacheMissPolicy = WMS_CACHE_QUERY_MISS_SKIP;
    IWMSDataContainerVersion *pContentVersion = NULL;
    CComBSTR bstrUrl;
    BOOL fDownload = FALSE;
    long lContentType = 0;
    DWORD dwCacheFlags;
    WCHAR szPort[ 20 ];

    if( ( VT_UNKNOWN != V_VT( &varContext ) ) || 
        ( NULL == V_UNKNOWN( &varContext ) ) )
    {
        return( E_INVALIDARG );
    }

    pOpState = ( COpState *) V_UNKNOWN( &varContext );

    if( FAILED( returnHr ) )
    {
        if( NS_E_CONNECTION_FAILURE == returnHr )
        {
            // Do protocol rollover
            pOpState->m_dwProtocolIndex++;

            if( NULL == g_ProxyProtocols[ pOpState->m_dwProtocolIndex ] )
            {
                // we have tried all the protocols and failed
                hr = E_NOINTERFACE;
                goto abort;
            }

            bstrUrl = g_ProxyProtocols[ pOpState->m_dwProtocolIndex ];
            bstrUrl.Append( L"://" );
            bstrUrl.Append( pOpState->m_bstrHost );
            
            // if we we are using the same protocol requested by the client, then we should
            // also use the port specified by the client (if one was specified)
            if( ( 0 != pOpState->m_wPort ) &&
                ( 0 == _wcsicmp( g_ProxyProtocols[ pOpState->m_dwProtocolIndex ], pOpState->m_bstrProtocol ) ) )
            {
                bstrUrl.Append( L":" );
				_itow_s( pOpState->m_wPort, szPort,sizeof(szPort)/sizeof(WCHAR), 10 );
                bstrUrl.Append( szPort );
            }

            bstrUrl.Append( L"/" );
            bstrUrl.Append( pOpState->m_bstrPath );

            hr = m_pICacheProxyServer->GetContentInformation(
                                            bstrUrl,
                                            pOpState->m_pPresentationContext,
                                            this,
                                            NULL,
                                            (IWMSCacheProxyServerCallback *) this,
                                            varContext
                                            );
            if( FAILED( hr ) )
            {
                goto abort;
            }

            return( S_OK );
        }
        else if( E_ACCESSDENIED == returnHr )
        {
            // the origin server requires authentication to provide information about this content.
            // since we don't have the credentials, we can either proxy this stream on-demand (in
            // which case the player will be prompted for the credentials) or simply fail this request.
            // let's opt for proxying the stream.
            CacheMissPolicy = WMS_CACHE_QUERY_MISS_PLAY_ON_DEMAND;
        }
        else
        {
            hr = returnHr;
            goto abort;
        }
    }
    else
    {
        hr = pContentInfo->GetLongValue( WMS_CACHE_CONTENT_INFORMATION_CONTENT_TYPE,
                                         WMS_CACHE_CONTENT_INFORMATION_CONTENT_TYPE_ID,
                                         (long *) &lContentType,
                                         0 );
        if( FAILED( hr ) )
        {
            goto abort;
        }
        
        if( WMS_CACHE_CONTENT_TYPE_BROADCAST & lContentType )
        {
            hr = pContentInfo->GetAndQueryIUnknownValue( WMS_CACHE_CONTENT_INFORMATION_DATA_CONTAINER_VERSION,
                                                         WMS_CACHE_CONTENT_INFORMATION_DATA_CONTAINER_VERSION_ID,
                                                         IID_IWMSDataContainerVersion,
                                                         (IUnknown **) &pContentVersion,
                                                         0 );
            if( ( FAILED( hr ) ) || ( NULL == pContentVersion ) )
            {
                hr = FAILED( hr ) ? hr : E_UNEXPECTED;
                goto abort;
            }

            hr = pContentVersion->GetCacheFlags( (long *) &dwCacheFlags );
            if( FAILED( hr ) )
            {
                goto abort;
            }

            if( dwCacheFlags & WMS_DATA_CONTAINER_VERSION_ALLOW_STREAM_SPLITTING )
            {
                CacheMissPolicy = WMS_CACHE_QUERY_MISS_PLAY_BROADCAST;
            }
            else
            {
                CacheMissPolicy = WMS_CACHE_QUERY_MISS_PLAY_ON_DEMAND;
            }
        }
        else  // It is an on-demand publishing point
        {
            CacheMissPolicy = WMS_CACHE_QUERY_MISS_PLAY_ON_DEMAND;
        }
    }

    hr = S_OK;

    bstrUrl = g_ProxyProtocols[ pOpState->m_dwProtocolIndex ];
    bstrUrl.Append( L"://" );
    bstrUrl.Append( pOpState->m_bstrHost );
    
    // if we we are using the same protocol requested by the client, then we should
    // also use the port specified by the client (if one was specified)
    if( ( 0 != pOpState->m_wPort ) &&
        ( 0 == _wcsicmp( g_ProxyProtocols[ pOpState->m_dwProtocolIndex ], pOpState->m_bstrProtocol ) ) )
    {
        bstrUrl.Append( L":" );
		_itow_s( pOpState->m_wPort, szPort,sizeof(szPort)/sizeof(WCHAR), 10 );
        bstrUrl.Append( szPort );
    }

    bstrUrl.Append( L"/" );
    bstrUrl.Append( pOpState->m_bstrPath );

abort:
    hr = pOpState->m_pCacheProxyCallback->OnQueryCacheMissPolicy( hr,
                                                                  CacheMissPolicy,
                                                                  bstrUrl,
                                                                  NULL,
                                                                  pContentInfo,
                                                                  pOpState->m_varContext );
    if( FAILED( hr ) )
    {
        hr = S_OK;
    }

    if( NULL != pOpState )
    {
        pOpState->Release();
    }

    if( NULL != pContentVersion )
    {
        pContentVersion->Release();
    }

    return( S_OK );
}


/////////////////////////////////////////////////////////////////////////////
//
//
//
/////////////////////////////////////////////////////////////////////////////
BOOL
CProxyPlugin::ParseUrl( BSTR bstrUrl,
                        BSTR *pbstrProtocol,
                        BSTR *pbstrHost,
                        BSTR *pbstrPath,
                        WORD *pwPort
                        )
{
    LPWSTR pszSchemeless;
    LPWSTR pszProtocol = L"";
    LPWSTR pszHost = L"";
    LPWSTR pszPath = L"";
    LPWSTR pszUrlCopy = NULL;
    LPWSTR pszHostEnd = NULL;;
    LPWSTR psz = NULL;;
    BOOL fRet = TRUE;

    if( ( NULL == bstrUrl ) ||
        ( NULL == pbstrProtocol ) ||
        ( NULL == pbstrHost ) ||
        ( NULL == pbstrPath ) ||
        ( NULL == pwPort ) )
    {
        return( FALSE );
    }

    *pbstrProtocol = NULL;
    *pbstrHost = NULL;
    *pbstrPath = NULL;
    
    pszUrlCopy = new WCHAR[ wcslen( bstrUrl ) + 1 ];
    if( NULL == pszUrlCopy )
    {
        fRet = FALSE;
        goto done;
    }

    wcscpy_s( pszUrlCopy,wcslen( bstrUrl ) + 1, bstrUrl );

    // skip the protocol
    pszSchemeless = wcsstr( pszUrlCopy, L"://" );
    if( NULL != pszSchemeless )
    {
        pszProtocol = pszUrlCopy;
        *pszSchemeless = L'\0'; // null terminate the protocol name
        pszSchemeless += 3;
    }
    else
    {
        pszSchemeless = pszUrlCopy;

        // Remove any leading '/'.
        while ( L'/' == *pszSchemeless )
        {
            pszSchemeless++;
        }
    }

    pszHost = pszSchemeless;

    // find the beginning of the path
    psz = wcschr( pszSchemeless, L'/' );
    if( NULL != psz )
    {
        *psz = L'\0'; // null terminate the host name
        pszPath = psz + 1;
        pszHostEnd = psz;
    }
    else
    {
        // there is no path, only the host name
        pszHostEnd = pszSchemeless + wcslen( pszSchemeless );
    }

    // see if a port number was specified at the end of the host name
    for( psz = pszHostEnd; psz > pszUrlCopy; psz-- )
    {
        if( L':' == *psz )
        {
            *psz = L'\0'; // null terminate the host name
            *pwPort = ( WORD ) _wtoi( psz + 1 );
            break;
        }
    }
    
    *pbstrProtocol = SysAllocString( pszProtocol );
    if( NULL == *pbstrProtocol )
    {
        fRet = FALSE;
        goto done;
    }
    
    *pbstrHost = SysAllocString( pszHost );
    if( NULL == *pbstrHost )
    {
        fRet = FALSE;
        goto done;
    }
    
    *pbstrPath = SysAllocString( pszPath );
    if( NULL == *pbstrPath )
    {
        fRet = FALSE;
        goto done;
    }

done:
    
    if( NULL != pszUrlCopy )
    {
        delete [] pszUrlCopy;
    }

    if( !fRet )
    {
        if( NULL != *pbstrProtocol )
        {
            SysFreeString( *pbstrProtocol );
        }

        if( NULL != *pbstrHost )
        {
            SysFreeString( *pbstrHost );
        }

        if( NULL != *pbstrPath )
        {
            SysFreeString( *pbstrPath );
        }
    }

    return( fRet );
}





/////////////////////////////////////////////////////////////////////////////
//
// [OnCacheClientClose]
//
/////////////////////////////////////////////////////////////////////////////
HRESULT STDMETHODCALLTYPE 
CProxyPlugin::OnCacheClientClose( 
                        HRESULT resultHr,
                        IWMSContext *pUserContext,
                        IWMSContext *pPresentationContext
                        )
{
    return( S_OK );
} // OnCacheClientClose

