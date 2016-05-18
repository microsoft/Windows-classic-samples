//+-------------------------------------------------------------------------
//
//  Microsoft Windows Media Technologies
//  Copyright (C) Microsoft Corporation. All rights reserved.
//
//  File:       ContextPlugin.cpp
//
//  Contents:
//
//--------------------------------------------------------------------------
#include "StdAfx.h"
#include "ContextDll.h"
#include "ContextPlugin.h"
#include "ContextAdmin.h"
#include "WMSContextNames.h"
#include <comdef.h>
#include "nserror.h"

const int NUM_HANDLED_EVENTS = 27;

const LPWSTR CONTEXT_SAMPLE_USER_CONTEXT_HEADER = L"-------------------------\r\nUser Context\r\n-------------------------\r\n";
const LPWSTR CONTEXT_SAMPLE_PRESENTATION_CONTEXT_HEADER = L"-------------------------\r\nPresentation Context\r\n-------------------------\r\n";
const LPWSTR CONTEXT_SAMPLE_COMMAND_REQUEST_CONTEXT_HEADER = L"-------------------------\r\nCommand Request Context\r\n-------------------------\r\n";
const LPWSTR CONTEXT_SAMPLE_COMMAND_RESPONSE_CONTEXT_HEADER = L"-------------------------\r\nCommand Response Context\r\n-------------------------\r\n";


const LPWSTR CONTEXT_SAMPLE_BSTR_TYPE_STRING = L"%s(%d) = %s(VT_BSTR)\r\n";
const LPWSTR CONTEXT_SAMPLE_I4_TYPE_STRING = L"%s(%d) = 0x%8.8x(%d)(VT_I4)\r\n";
const LPWSTR CONTEXT_SAMPLE_UI8_TYPE_STRING = L"%s(%d) = 0x%16.16I64x(VT_UI8)\r\n";
const LPWSTR CONTEXT_SAMPLE_CY_TYPE_STRING = L"%s(%d) = 0x%16.16I64x(VT_CY)\r\n";
const LPWSTR CONTEXT_SAMPLE_DATE_TYPE_STRING = L"%s(%d) = 0x%8.8x(VT_DATE)\r\n";
const LPWSTR CONTEXT_SAMPLE_DECIMAL_TYPE_STRING = L"%s(%d) = 0x%8.8x(VT_DECIMAL)\r\n";
const LPWSTR CONTEXT_SAMPLE_UNKNOWN_TYPE_STRING = L"%s(%d) = 0x%8.8x(VT_UNKNOWN)\r\n";
const LPWSTR CONTEXT_SAMPLE_DISPATCH_TYPE_STRING = L"%s(%d) = 0x%8.8x(VT_DISPATCH)\r\n";
const LPWSTR CONTEXT_SAMPLE_ARRAY_TYPE_STRING = L"%s(%d) = Type(%d) (VT_ARRAY?)\r\n";


const LPWSTR CONTEXT_SAMPLE_NEW_EVENT_HEADER = L"************************************************************\r\n%s Event at %4d.%02d.%02d %02d.%02d.%02d\r\n************************************************************\r\n";


WCHAR * CContextPlugin::s_wstrNamedValueOutputPath = L"OutputPath";
WCHAR * CContextPlugin::s_wstrNamedValueContextTypes = L"ContextTypes";

const ContextNameHint CContextPlugin::s_UserContextHintValues[] = 
{
    WMS_USER_AGENT, WMS_USER_AGENT_ID,
    WMS_USER_GUID, WMS_USER_GUID_ID,
    WMS_USER_NAME, WMS_USER_NAME_ID,
    WMS_USER_IP_ADDRESS, WMS_USER_IP_ADDRESS_ID,
    WMS_USER_IP_ADDRESS_STRING, WMS_USER_IP_ADDRESS_STRING_ID,
    WMS_USER_CONTROL_PROTOCOL, WMS_USER_CONTROL_PROTOCOL_ID,
    WMS_USER_AUTHENTICATOR, WMS_USER_AUTHENTICATOR_ID,
    WMS_USER_ID, WMS_USER_ID_ID,
    WMS_USER_PORT, WMS_USER_PORT_ID,
    WMS_USER_PRESENTATION_CONTEXT, WMS_USER_PRESENTATION_CONTEXT_ID,
    WMS_USER_LINK_BANDWIDTH, WMS_USER_LINK_BANDWIDTH_ID,
    WMS_USER_REFERER, WMS_USER_REFERER_ID,
    WMS_USER_VIA_UPSTREAM_PROXIES, WMS_USER_VIA_UPSTREAM_PROXIES_ID,
    WMS_USER_VIA_DOWNSTREAM_PROXIES, WMS_USER_VIA_DOWNSTREAM_PROXIES_ID,
    WMS_USER_CACHE_CLIENT_COOKIE, WMS_USER_CACHE_CLIENT_COOKIE_ID,
    WMS_USER_CACHE_SERVER_COOKIE, WMS_USER_CACHE_SERVER_COOKIE_ID,
    WMS_USER_PROXY_CLIENT_AGENT, WMS_USER_PROXY_CLIENT_AGENT_ID,
    NULL, -1          // signals end of array
};

const ContextNameHint CContextPlugin::s_PresentationContextHintValues[] = 
{
    WMS_PRESENT_STREAM_HEADERS, WMS_PRESENT_STREAM_HEADERS_ID,
    WMS_PRESENT_CONTENT_DESCRIPTION, WMS_PRESENT_CONTENT_DESCRIPTION_ID,
    WMS_PRESENT_PHYSICAL_NAME, WMS_PRESENT_PHYSICAL_NAME_ID,
    WMS_PRESENT_REQUEST_NAME, WMS_PRESENT_REQUEST_NAME_ID,
    WMS_PRESENT_BROADCAST, WMS_PRESENT_BROADCAST_ID,
    WMS_PRESENT_SEEKABLE, WMS_PRESENT_SEEKABLE_ID,
    WMS_PRESENT_RELIABLE, WMS_PRESENT_RELIABLE_ID,
    WMS_PRESENT_BITRATE, WMS_PRESENT_BITRATE_ID,
    WMS_PRESENT_DURATION_HI, WMS_PRESENT_DURATION_HI_ID,
    WMS_PRESENT_DURATION_LO, WMS_PRESENT_DURATION_LO_ID,
    WMS_PRESENT_PLAY_RATE, WMS_PRESENT_PLAY_RATE_ID,
    WMS_PRESENT_START_TIME, WMS_PRESENT_START_TIME_ID,
    WMS_PRESENT_ORIGINAL_PHYSICAL_NAME, WMS_PRESENT_ORIGINAL_PHYSICAL_NAME_ID,
    WMS_PRESENT_ORIGINAL_REQUEST_NAME, WMS_PRESENT_ORIGINAL_REQUEST_NAME_ID,
    WMS_PRESENT_TOTAL_BYTES_SENT_HI, WMS_PRESENT_TOTAL_BYTES_SENT_HI_ID,
    WMS_PRESENT_TOTAL_BYTES_SENT_LO, WMS_PRESENT_TOTAL_BYTES_SENT_LO_ID,
    WMS_PRESENT_TOTAL_PLAY_TIME_HI, WMS_PRESENT_TOTAL_PLAY_TIME_HI_ID,
    WMS_PRESENT_TOTAL_PLAY_TIME_LO, WMS_PRESENT_TOTAL_PLAY_TIME_LO_ID,
    WMS_PRESENT_PLAYLIST_ENTRY_ROLE, WMS_PRESENT_PLAYLIST_ENTRY_ROLE_ID,
    WMS_PRESENT_WMSSINK_SELECTED_BITRATE, WMS_PRESENT_WMSSINK_SELECTED_BITRATE_ID,
    WMS_PRESENT_REDIRECT_LOCATION, WMS_PRESENT_REDIRECT_LOCATION_ID,
    WMS_PRESENT_PREROLL_TIME, WMS_PRESENT_PREROLL_TIME_ID,
    NULL, -1          // signals end of array
};

const ContextNameHint CContextPlugin::s_CommandContextHintValues[] = 
{
    WMS_COMMAND_CONTEXT_URL, WMS_COMMAND_CONTEXT_URL_ID,
    WMS_COMMAND_CONTEXT_URL_SCHEME, WMS_COMMAND_CONTEXT_URL_SCHEME_ID,
    WMS_COMMAND_CONTEXT_URL_HOSTNAME, WMS_COMMAND_CONTEXT_URL_HOSTNAME_ID,
    WMS_COMMAND_CONTEXT_URL_PORT, WMS_COMMAND_CONTEXT_URL_PORT_ID,
    WMS_COMMAND_CONTEXT_URL_PATH, WMS_COMMAND_CONTEXT_URL_PATH_ID,
    WMS_COMMAND_CONTEXT_URL_EXTRAINFO, WMS_COMMAND_CONTEXT_URL_EXTRAINFO_ID,
    WMS_COMMAND_CONTEXT_BODY, WMS_COMMAND_CONTEXT_BODY_ID,
    WMS_COMMAND_CONTEXT_BODY_TYPE, WMS_COMMAND_CONTEXT_BODY_TYPE_ID,
    WMS_COMMAND_CONTEXT_START_OFFSET, WMS_COMMAND_CONTEXT_START_OFFSET_ID,
    WMS_COMMAND_CONTEXT_START_OFFSET_TYPE, WMS_COMMAND_CONTEXT_START_OFFSET_TYPE_ID,
    WMS_COMMAND_CONTEXT_RATE, WMS_COMMAND_CONTEXT_RATE_ID,
    WMS_COMMAND_CONTEXT_PUBPOINT_IDENTIFIER, WMS_COMMAND_CONTEXT_PUBPOINT_IDENTIFIER_ID,
    WMS_COMMAND_CONTEXT_EVENT, WMS_COMMAND_CONTEXT_EVENT_ID,
    WMS_COMMAND_CONTEXT_EVENT_ADMINNAME, WMS_COMMAND_CONTEXT_EVENT_ADMINNAME_ID,
    WMS_COMMAND_CONTEXT_LIMIT_CLIENTID, WMS_COMMAND_CONTEXT_LIMIT_CLIENTID_ID,
    WMS_COMMAND_CONTEXT_LIMIT_CLIENTIP, WMS_COMMAND_CONTEXT_LIMIT_CLIENTIP_ID,
    WMS_COMMAND_CONTEXT_LIMIT_OLD_VALUE, WMS_COMMAND_CONTEXT_LIMIT_OLD_VALUE_ID,
    WMS_COMMAND_CONTEXT_PLAYLIST_OBJECT, WMS_COMMAND_CONTEXT_PLAYLIST_OBJECT_ID,
    WMS_COMMAND_CONTEXT_PUBPOINT_NAME, WMS_COMMAND_CONTEXT_PUBPOINT_NAME_ID,
    WMS_COMMAND_CONTEXT_PUBPOINT_MONIKER, WMS_COMMAND_CONTEXT_PUBPOINT_MONIKER_ID,
    WMS_COMMAND_CONTEXT_EVENT_OLD_VALUE, WMS_COMMAND_CONTEXT_EVENT_OLD_VALUE_ID,
    WMS_COMMAND_CONTEXT_EVENT_NEW_VALUE, WMS_COMMAND_CONTEXT_EVENT_NEW_VALUE_ID,
    WMS_COMMAND_CONTEXT_EVENT_PROPERTY_NAME, WMS_COMMAND_CONTEXT_EVENT_PROPERTY_NAME_ID,
    WMS_COMMAND_CONTEXT_PLUGIN_NAME, WMS_COMMAND_CONTEXT_PLUGIN_NAME_ID,
    WMS_COMMAND_CONTEXT_PLUGIN_MONIKER, WMS_COMMAND_CONTEXT_PLUGIN_MONIKER_ID,
    WMS_COMMAND_CONTEXT_LIMIT_NEW_VALUE, WMS_COMMAND_CONTEXT_LIMIT_NEW_VALUE_ID,
    WMS_COMMAND_CONTEXT_CACHE_MONIKER, WMS_COMMAND_CONTEXT_CACHE_MONIKER_ID,
    WMS_COMMAND_CONTEXT_DOWNLOAD_URL, WMS_COMMAND_CONTEXT_DOWNLOAD_URL_ID,
    WMS_COMMAND_CONTEXT_REDIRECT_URL, WMS_COMMAND_CONTEXT_REDIRECT_URL_ID,
    WMS_COMMAND_CONTEXT_PUSH_DISTRIBUTION_TEMPLATE, WMS_COMMAND_CONTEXT_PUSH_DISTRIBUTION_TEMPLATE_ID,
    WMS_COMMAND_CONTEXT_PUSH_CREATING_NEW_PUBLISHING_POINT, WMS_COMMAND_CONTEXT_PUSH_CREATING_NEW_PUBLISHING_POINT_ID,
    WMS_COMMAND_CONTEXT_PLAYLIST_ENTRY_UNIQUE_RUNTIME_ID, WMS_COMMAND_CONTEXT_PLAYLIST_ENTRY_UNIQUE_RUNTIME_ID_ID,
    WMS_COMMAND_CONTEXT_REQUEST_URL, WMS_COMMAND_CONTEXT_REQUEST_URL_ID,
    NULL, -1          // signals end of array
};



/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
CContextPlugin::CContextPlugin()
{
    m_pServer = NULL;
    m_pNamedValues = NULL;
    m_bstrOutputPath = NULL;
    m_wmsContexts = WMS_CONTEXT_PLUGIN_NO_CONTEXT;
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
CContextPlugin::~CContextPlugin()
{
    if( NULL != m_bstrOutputPath )
    {
        ::SysFreeString( m_bstrOutputPath );
        m_bstrOutputPath = NULL;
    }
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CContextPlugin::InitializePlugin
(
    IWMSContext *pServerContext, 
    IWMSNamedValues *pNamedValues,
    IWMSClassObject *pClassFactory
)
{
    HRESULT hr = S_OK;

    if ( ( NULL == pServerContext )
        || ( NULL == pNamedValues )
        || ( NULL == pClassFactory ) )
    {
        return( E_INVALIDARG );
    }

    hr = pServerContext->GetAndQueryIUnknownValue( const_cast<LPWSTR>( WMS_SERVER ), WMS_SERVER_ID,
                                                   IID_IWMSServer, (IUnknown**) &m_pServer, 0 );

    if( SUCCEEDED( hr ) )
    {
        m_pNamedValues = pNamedValues;
        m_pNamedValues->AddRef();

        hr = LoadConfigValues();
    }

    return( hr );
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CContextPlugin::EnablePlugin( long *pdwFlags, long *pdwHeartbeatPeriod )
{
    if ( ( NULL == pdwFlags ) || ( NULL == pdwHeartbeatPeriod ) )
    {
        return ( E_POINTER );
    }

    // Set the heartbeat period in milliseconds
    *pdwHeartbeatPeriod = 0;

    return ( S_OK );
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CContextPlugin::DisablePlugin()
{
    return ( S_OK );
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CContextPlugin::ShutdownPlugin()
{
    HRESULT hr = S_OK;

    hr = SaveConfigValues();

    if( NULL != m_pServer )
    {
        m_pServer->Release();
        m_pServer = NULL;
    }

    if( NULL != m_pNamedValues )
    {
        m_pNamedValues->Release();
        m_pNamedValues = NULL;
    }

    return( hr );
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CContextPlugin::OnHeartbeat()
{
    HRESULT hr = S_OK;

    // TODO: Add code that should execute on every Heartbeat Period
    return( hr );
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CContextPlugin::GetCustomAdminInterface( IDispatch **ppValue )
{
    HRESULT hr = S_OK;

    if ( ( NULL == ppValue ) )
    {
        return( E_POINTER );
    }
    
    *ppValue = NULL;

    CComObject< CContextAdmin > *spContextAdmin;

    hr = CComObject< CContextAdmin >::CreateInstance( &spContextAdmin );
    if( SUCCEEDED( hr ) )
    {
        CComPtr< IDispatch > spAdmin( spContextAdmin );
        hr = spContextAdmin->Initialize( this );
        if ( SUCCEEDED( hr ) )
        {
            hr = spContextAdmin->QueryInterface( IID_IDispatch, (void **) ppValue );
        }
    }

    return( hr );
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CContextPlugin::GetHandledEvents( VARIANT *pvarHandledEvents )
{
    HRESULT hr = S_OK;
    if( NULL == pvarHandledEvents )
    {
        return( E_POINTER );
    }

    int nIndex = 0;
    WMS_EVENT_TYPE wmsHandledEvents[ NUM_HANDLED_EVENTS ];
    wmsHandledEvents[ nIndex++ ] = WMS_EVENT_CONNECT;
    wmsHandledEvents[ nIndex++ ] = WMS_EVENT_DISCONNECT;
    wmsHandledEvents[ nIndex++ ] = WMS_EVENT_BEGIN_USER_SESSION;
    wmsHandledEvents[ nIndex++ ] = WMS_EVENT_END_USER_SESSION;
    wmsHandledEvents[ nIndex++ ] = WMS_EVENT_DESCRIBE;
    wmsHandledEvents[ nIndex++ ] = WMS_EVENT_OPEN;
    wmsHandledEvents[ nIndex++ ] = WMS_EVENT_SELECT_STREAMS;
    wmsHandledEvents[ nIndex++ ] = WMS_EVENT_PLAY;
    wmsHandledEvents[ nIndex++ ] = WMS_EVENT_PAUSE;
    wmsHandledEvents[ nIndex++ ] = WMS_EVENT_STOP;
    wmsHandledEvents[ nIndex++ ] = WMS_EVENT_CLOSE;
    wmsHandledEvents[ nIndex++ ] = WMS_EVENT_SET_PARAMETER;
    wmsHandledEvents[ nIndex++ ] = WMS_EVENT_GET_PARAMETER;
    wmsHandledEvents[ nIndex++ ] = WMS_EVENT_VALIDATE_PUSH_DISTRIBUTION;
    wmsHandledEvents[ nIndex++ ] = WMS_EVENT_CREATE_DISTRIBUTION_DATA_PATH;
    wmsHandledEvents[ nIndex++ ] = WMS_EVENT_DESTROY_DISTRIBUTION_DATA_PATH;
    wmsHandledEvents[ nIndex++ ] = WMS_EVENT_LOG;
    wmsHandledEvents[ nIndex++ ] = WMS_EVENT_SERVER;
    wmsHandledEvents[ nIndex++ ] = WMS_EVENT_PUBLISHING_POINT;
    wmsHandledEvents[ nIndex++ ] = WMS_EVENT_LIMIT_CHANGE;
    wmsHandledEvents[ nIndex++ ] = WMS_EVENT_LIMIT_HIT;
    wmsHandledEvents[ nIndex++ ] = WMS_EVENT_PLUGIN;
    wmsHandledEvents[ nIndex++ ] = WMS_EVENT_PLAYLIST;
    wmsHandledEvents[ nIndex++ ] = WMS_EVENT_CACHE;
    wmsHandledEvents[ nIndex++ ] = WMS_EVENT_REMOTE_CACHE_OPEN;
    wmsHandledEvents[ nIndex++ ] = WMS_EVENT_REMOTE_CACHE_CLOSE;
    wmsHandledEvents[ nIndex++ ] = WMS_EVENT_REMOTE_CACHE_LOG;

    hr = CreateArrayOfEvents( pvarHandledEvents, wmsHandledEvents, NUM_HANDLED_EVENTS );

    return( hr );
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CContextPlugin::OnEvent
(
    WMS_EVENT *pEvent, 
    IWMSContext *pUserCtx, 
    IWMSContext *pPresentationCtx, 
    IWMSCommandContext *pCommandCtx 
)
{
    HRESULT hr = S_OK;

    //
    // The event information must be valid in order to determine what
    // type of event has happened.
    //
    if(  NULL == pEvent )
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
        hr = OnNotifyConnect( pUserCtx, pPresentationCtx, pCommandCtx );
        break;
    case WMS_EVENT_DISCONNECT:
        hr = OnNotifyDisconnect( pUserCtx, pPresentationCtx, pCommandCtx );
        break;
    case WMS_EVENT_BEGIN_USER_SESSION:
        hr = OnNotifyBeginUserSession( pUserCtx, pPresentationCtx, pCommandCtx );
        break;
    case WMS_EVENT_END_USER_SESSION:
        hr = OnNotifyEndUserSession( pUserCtx, pPresentationCtx, pCommandCtx );
        break;
    case WMS_EVENT_DESCRIBE:
        hr = OnNotifyDescribe( pUserCtx, pPresentationCtx, pCommandCtx );
        break;
    case WMS_EVENT_OPEN:
        hr = OnNotifyOpen( pUserCtx, pPresentationCtx, pCommandCtx );
        break;
    case WMS_EVENT_SELECT_STREAMS:
        hr = OnNotifySelectStreams( pUserCtx, pPresentationCtx, pCommandCtx );
        break;
    case WMS_EVENT_PLAY:
        hr = OnNotifyPlay( pUserCtx, pPresentationCtx, pCommandCtx );
        break;
    case WMS_EVENT_PAUSE:
        hr = OnNotifyPause( pUserCtx, pPresentationCtx, pCommandCtx );
        break;
    case WMS_EVENT_STOP:
        hr = OnNotifyStop( pUserCtx, pPresentationCtx, pCommandCtx );
        break;
    case WMS_EVENT_CLOSE:
        hr = OnNotifyClose( pUserCtx, pPresentationCtx, pCommandCtx );
        break;
    case WMS_EVENT_SET_PARAMETER:
        hr = OnNotifySetParameter( pUserCtx, pPresentationCtx, pCommandCtx );
        break;
    case WMS_EVENT_GET_PARAMETER:
        hr = OnNotifyGetParameter( pUserCtx, pPresentationCtx, pCommandCtx );
        break;
    case WMS_EVENT_VALIDATE_PUSH_DISTRIBUTION:
        hr = OnNotifyValidatePushDistribution( pUserCtx, pPresentationCtx, pCommandCtx );
        break;
    case WMS_EVENT_CREATE_DISTRIBUTION_DATA_PATH:
        hr = OnNotifyCreateDistributionDataPath( pUserCtx, pPresentationCtx, pCommandCtx );
        break;
    case WMS_EVENT_DESTROY_DISTRIBUTION_DATA_PATH:
        hr = OnNotifyDestroyDistributionDataPath( pUserCtx, pPresentationCtx, pCommandCtx );
        break;
    case WMS_EVENT_LOG:
        hr = OnNotifyLog( pUserCtx, pPresentationCtx, pCommandCtx );
        break;
    case WMS_EVENT_SERVER:
        hr = OnNotifyServer( pUserCtx, pPresentationCtx, pCommandCtx );
        break;
    case WMS_EVENT_PUBLISHING_POINT:		
        hr = OnNotifyPublishingPoint( pUserCtx, pPresentationCtx, pCommandCtx );
        break;
    case WMS_EVENT_LIMIT_CHANGE:
        hr = OnNotifyLimitChange( pUserCtx, pPresentationCtx, pCommandCtx );
        break;
    case WMS_EVENT_LIMIT_HIT:
        hr = OnNotifyLimitHit( pUserCtx, pPresentationCtx, pCommandCtx );
        break;
    case WMS_EVENT_PLUGIN:
        hr = OnNotifyPlugin( pUserCtx, pPresentationCtx, pCommandCtx );
        break;
    case WMS_EVENT_PLAYLIST:
        hr = OnNotifyPlaylist( pUserCtx, pPresentationCtx, pCommandCtx );
        break;
    case WMS_EVENT_CACHE:
        hr = OnNotifyCache( pUserCtx, pPresentationCtx, pCommandCtx );
        break;
    case WMS_EVENT_REMOTE_CACHE_OPEN:
        hr = OnNotifyRemoteCacheOpen( pUserCtx, pPresentationCtx, pCommandCtx );
        break;
    case WMS_EVENT_REMOTE_CACHE_CLOSE:
        hr = OnNotifyRemoteCacheClose( pUserCtx, pPresentationCtx, pCommandCtx );
        break;
    case WMS_EVENT_REMOTE_CACHE_LOG:
        hr = OnNotifyRemoteCacheLog( pUserCtx, pPresentationCtx, pCommandCtx );
        break;
    }
    
    return( hr );
}


HRESULT CContextPlugin::LoadConfigValues()
{
    HRESULT hr = S_OK;
    IWMSNamedValue *pValue = NULL;
    VARIANT var;

    if( NULL == m_pNamedValues )
    {
        return( E_UNEXPECTED );
    }

    VariantInit( &var );

    //
    // Get our properties from the server
    //
    hr = m_pNamedValues->get_Item( CComVariant(CContextPlugin::s_wstrNamedValueOutputPath), &pValue );

    //
    // Get the Output Path Name
    //
    
    if ( SUCCEEDED(hr) )
    {
        hr = pValue->get_Value(&var);

        if ( SUCCEEDED(hr) )
        {
            hr = ::VariantChangeType( &var, &var, 0, VT_BSTR );
        }

        if ( SUCCEEDED(hr) )
        {
            m_bstrOutputPath = ::SysAllocString( V_BSTR( &var ) );
            if( NULL == m_bstrOutputPath )
            {
                hr = E_OUTOFMEMORY;
                goto abort;
            }
        }
    }

    hr = S_OK; // Default is OK

    VariantClear( &var );
    if( NULL != pValue )
    {
        pValue->Release();
        pValue = NULL;
    }

    //
    // Get the flags for ContextTypes
    //

    hr = m_pNamedValues->get_Item( CComVariant(CContextPlugin::s_wstrNamedValueContextTypes), &pValue );

    if ( SUCCEEDED(hr) )
    {
        hr = pValue->get_Value( &var );

        if ( SUCCEEDED(hr) )
        {
            m_wmsContexts = (WMS_CONTEXT_PLUGIN_CONTEXT_TYPE) V_I4( &var );
        }
    }

    hr = S_OK; // Default is OK
abort:
    VariantClear( &var );
    if( NULL != pValue )
    {
        pValue->Release();
        pValue = NULL;
    }

    return( hr );
}


HRESULT CContextPlugin::SaveConfigValues()
{
    HRESULT hr = S_OK;
    IWMSNamedValue *pValue = NULL;
    VARIANT var;

    if( NULL == m_pNamedValues )
    {
        return( E_UNEXPECTED );
    }

    VariantInit( &var );

    //
    // Save our Output Path property
    //
    V_VT( &var ) = VT_BSTR;
    if( NULL == ( V_BSTR( &var ) = ::SysAllocString( m_bstrOutputPath ) ) )
    {
        return( E_OUTOFMEMORY );
    }
    hr = m_pNamedValues->Add( CComBSTR( CContextPlugin::s_wstrNamedValueOutputPath ), var, &pValue );
    VariantClear( &var );
    if( NULL != pValue )
    {
        pValue->Release();
        pValue = NULL;
    }


    
    VariantInit( &var );

    //
    // Save our Context Types property
    //
    V_VT( &var ) = VT_I4;
    V_I4( &var ) = m_wmsContexts;
    hr = m_pNamedValues->Add( CComBSTR( CContextPlugin::s_wstrNamedValueContextTypes ), var, &pValue );
    VariantClear( &var );

    if( NULL != pValue )
    {
        pValue->Release();
        pValue = NULL;
    }

    return( hr );
}


HRESULT CContextPlugin::SetOutputPath( BSTR bstrOutputPath )
{
    HRESULT hr = S_OK;

    if( NULL != m_bstrOutputPath )
    {
        ::SysFreeString( m_bstrOutputPath );
        m_bstrOutputPath = NULL;
    }

    if( NULL == bstrOutputPath )
    {
        // They just wanted to reset the OutputPath to nothing,
        // meaning no information is sent.
        return( hr );
    }

    m_bstrOutputPath = ::SysAllocString( bstrOutputPath );
    if( NULL == m_bstrOutputPath )
    {
        hr = E_OUTOFMEMORY;
        return( hr );
    }

    return( hr );
}


HRESULT CContextPlugin::GetOutputPath( BSTR *pbstrOutputPath )
{
    if( NULL == pbstrOutputPath )
    {
        return( E_POINTER );
    }
    *pbstrOutputPath = NULL;

    HRESULT hr = S_OK;

    if( NULL != m_bstrOutputPath )
    {
        *pbstrOutputPath = ::SysAllocString( m_bstrOutputPath );
    }
    else
    {
        // Our string wasn't set, so we should return the empty string.
        *pbstrOutputPath = ::SysAllocString( L"" );
    }

    if( NULL == *pbstrOutputPath )
    {
        hr = E_OUTOFMEMORY;
    }

    return( hr );
}



HRESULT CContextPlugin::CreateArrayOfEvents( VARIANT *pvarEvents, WMS_EVENT_TYPE *pWMSEvents, long nNumEvents)
{
    HRESULT hr = S_OK;
    long iEvents = 0;
    SAFEARRAY *psa = NULL;
    SAFEARRAYBOUND rgsabound[1];
    
    if( NULL == pvarEvents )
    {
        return( E_POINTER );
    }

    if( NULL == pWMSEvents || 0 >= nNumEvents )
    {
        return( E_INVALIDARG );   
    }
        
    rgsabound[0].lLbound = 0;
    rgsabound[0].cElements = nNumEvents;

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

    return ( hr );
}


HRESULT CContextPlugin::WriteContextInformation( HANDLE hFile, IWMSContext *pContext )
{
    HRESULT hr = S_OK;
    ContextNameHint *pContextHintValues = NULL;
    DWORD nValue = 0;
    WMS_CONTEXT_TYPE wmsContextType = WMS_UNKNOWN_CONTEXT_TYPE;
    WCHAR wstrBuffer[MAX_PATH];
    DWORD cbWritten = 0;
    DWORD dwRet = 0;


    if( NULL == pContext )
    {
        // There is no Context, nothing to write
        return( hr );
    }

    hr = pContext->GetContextType( &wmsContextType );

    if( FAILED( hr ) )
    {
        return( hr );
    }

    ZeroMemory( wstrBuffer, MAX_PATH * sizeof( WCHAR ) );

    switch( wmsContextType )
    {
        case WMS_USER_CONTEXT_TYPE:
            // Create a header for this context type
            wcsncpy_s( wstrBuffer,MAX_PATH, CONTEXT_SAMPLE_USER_CONTEXT_HEADER, MAX_PATH );
            pContextHintValues = const_cast<ContextNameHint *>(CContextPlugin::s_UserContextHintValues);
            break;
        case WMS_PRESENTATION_CONTEXT_TYPE:
            // Create a header for this context type
            wcsncpy_s( wstrBuffer, MAX_PATH,CONTEXT_SAMPLE_PRESENTATION_CONTEXT_HEADER, MAX_PATH );
            pContextHintValues = const_cast<ContextNameHint *>(CContextPlugin::s_PresentationContextHintValues);
            break;
        case WMS_COMMAND_REQUEST_CONTEXT_TYPE:
            // Create a header for this context type
            wcsncpy_s( wstrBuffer,MAX_PATH, CONTEXT_SAMPLE_COMMAND_REQUEST_CONTEXT_HEADER, MAX_PATH );
            pContextHintValues = const_cast<ContextNameHint *>(CContextPlugin::s_CommandContextHintValues);
            break;
        case WMS_COMMAND_RESPONSE_CONTEXT_TYPE:
            // Create a header for this context type
            wcsncpy_s( wstrBuffer,MAX_PATH, CONTEXT_SAMPLE_COMMAND_RESPONSE_CONTEXT_HEADER, MAX_PATH );
            pContextHintValues = const_cast<ContextNameHint *>(CContextPlugin::s_CommandContextHintValues);
            break;
    }

    if( !::WriteFile( hFile, (LPVOID) wstrBuffer, DWORD(wcslen( wstrBuffer ) * sizeof( WCHAR )), &cbWritten, NULL ) )
    {
        dwRet = GetLastError();
        hr = HRESULT_FROM_WIN32( dwRet );
        // Failed to write the header should we still continue
        // No!
        return( hr );
    }

    if( NULL == pContextHintValues )
    {
        return( E_UNEXPECTED );
    }

    // Now we loop until -1 and Write the data
    while( ( NULL != pContextHintValues[nValue].wstrContextName ) && ( -1 != pContextHintValues[nValue].lContextHint ) )
    {
        VARIANT varValue;
        VariantInit( &varValue );
        ZeroMemory( wstrBuffer, MAX_PATH * sizeof( WCHAR ) );

        hr = pContext->GetValue( pContextHintValues[nValue].wstrContextName, pContextHintValues[nValue].lContextHint, &varValue, 0 );

        if( SUCCEEDED( hr ) )
        {
            // Write string with data information
            switch( V_VT( &varValue ) )
            {
                case VT_BSTR:
                    _snwprintf_s( wstrBuffer,MAX_PATH, MAX_PATH, CONTEXT_SAMPLE_BSTR_TYPE_STRING, pContextHintValues[nValue].wstrContextName, pContextHintValues[nValue].lContextHint, V_BSTR( &varValue ) );
                    break;
                case VT_I4:
                    _snwprintf_s( wstrBuffer,MAX_PATH, MAX_PATH, CONTEXT_SAMPLE_I4_TYPE_STRING, pContextHintValues[nValue].wstrContextName, pContextHintValues[nValue].lContextHint, V_I4( &varValue ), V_I4( &varValue ) );
                    break;
                case VT_UI8:
                    _snwprintf_s( wstrBuffer,MAX_PATH, MAX_PATH, CONTEXT_SAMPLE_UI8_TYPE_STRING, pContextHintValues[nValue].wstrContextName, pContextHintValues[nValue].lContextHint, V_UI8( &varValue ) );
                    break;
                case VT_CY:
                    _snwprintf_s( wstrBuffer,MAX_PATH, MAX_PATH, CONTEXT_SAMPLE_CY_TYPE_STRING, pContextHintValues[nValue].wstrContextName, pContextHintValues[nValue].lContextHint, V_CY( &varValue ) );
                    break;
                case VT_DATE:
                    _snwprintf_s( wstrBuffer,MAX_PATH, MAX_PATH, CONTEXT_SAMPLE_DATE_TYPE_STRING, pContextHintValues[nValue].wstrContextName, pContextHintValues[nValue].lContextHint, V_DATE( &varValue ) );
                    break;
                case VT_DECIMAL:
                    _snwprintf_s( wstrBuffer,MAX_PATH, MAX_PATH, CONTEXT_SAMPLE_DECIMAL_TYPE_STRING, pContextHintValues[nValue].wstrContextName, pContextHintValues[nValue].lContextHint, V_DECIMAL( &varValue ) );
                    break;
                case VT_UNKNOWN:
                    _snwprintf_s( wstrBuffer,MAX_PATH, MAX_PATH, CONTEXT_SAMPLE_UNKNOWN_TYPE_STRING, pContextHintValues[nValue].wstrContextName, pContextHintValues[nValue].lContextHint, V_UNKNOWN( &varValue ) );
                    break;
                case VT_DISPATCH:
                    _snwprintf_s( wstrBuffer,MAX_PATH, MAX_PATH, CONTEXT_SAMPLE_DISPATCH_TYPE_STRING, pContextHintValues[nValue].wstrContextName, pContextHintValues[nValue].lContextHint, V_DISPATCH( &varValue ) );
                    break;
                default:
                    _snwprintf_s( wstrBuffer,MAX_PATH, MAX_PATH, CONTEXT_SAMPLE_ARRAY_TYPE_STRING, pContextHintValues[nValue].wstrContextName, pContextHintValues[nValue].lContextHint, V_ARRAY( &varValue ) );
                    break;
            }

            if( !::WriteFile( hFile, (LPVOID) wstrBuffer, DWORD(wcslen( wstrBuffer ) * sizeof( WCHAR )), &cbWritten, NULL ) )
            {
                dwRet = GetLastError();
                hr = HRESULT_FROM_WIN32( dwRet );
            }
        }
        else
        {
            // Value probably didn't exist for this event, don't worry about it
            // good place to put breakpoint
            hr = S_OK;
        }

        // It doesn't hurt to do a VariantClear on an Empty Variant, this way we won't leak anything.
        VariantClear( &varValue );

        nValue ++;
    }

    return( hr );
}


HRESULT CContextPlugin::DumpContextInformation( LPCWSTR wstrEventType, IWMSContext *pUserCtx, IWMSContext *pPresentationCtx, IWMSCommandContext *pCommandCtx )
{
    HRESULT hr = S_OK;
    UINT uiPrevErrorMode = 0;
    DWORD dwRet = 0;
    DWORD cbWritten = 0;
    SYSTEMTIME sTime;
    WCHAR wstrHeader[MAX_PATH];

    ZeroMemory( wstrHeader, MAX_PATH * sizeof( WCHAR ) );

    if( ( WMS_CONTEXT_PLUGIN_NO_CONTEXT == m_wmsContexts )
        || ( NULL == m_bstrOutputPath )
        || ( '\0' == m_bstrOutputPath[0] ) )
    {
        // We aren't interested in dumping any context information, so just exit with Success
        return( hr );
    }

    // Only keep the file open while dumping this event.
    HANDLE hFile = INVALID_HANDLE_VALUE;

    // This stops the system from popping up an error dialog box if a floppy or CD is missing.
    uiPrevErrorMode = ::SetErrorMode( SEM_FAILCRITICALERRORS );

    hFile = ::CreateFile( m_bstrOutputPath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );

    if( INVALID_HANDLE_VALUE == hFile )
    {
        dwRet = GetLastError();
        hr = HRESULT_FROM_WIN32( dwRet );
        goto abort;
    }
    else
    {
        dwRet = GetLastError();

        DWORD dwFileType = ::GetFileType( hFile );
        if( FILE_TYPE_DISK != dwFileType )
        {
            // Invalid File Type, for security reasons
            // we should only open disk files.
            hr = HRESULT_FROM_WIN32( ERROR_FILE_NOT_FOUND );
            goto abort;
            
        }

        if( ERROR_ALREADY_EXISTS != dwRet )
        {
            // We just opened a new file, we must write the Unicode File bits
            BYTE pWriteBuffer[2];
            pWriteBuffer[0] = 0xFF;
            pWriteBuffer[1] = 0xFE;
            if( !::WriteFile( hFile, (LPVOID) pWriteBuffer, 2, &cbWritten, NULL ) )
            {
                dwRet = GetLastError();
                hr = HRESULT_FROM_WIN32( dwRet );
                goto abort;
            }
        }
        else
        {
            // We need to check to see if the file is Unicode.
            BYTE pReadBuffer[2];
            pReadBuffer[0] = 0;
            pReadBuffer[1] = 0;

            DWORD cbRead = 0;
            if( !::ReadFile( hFile, (LPVOID) pReadBuffer, 2, &cbRead, NULL ) )
            {
                dwRet = GetLastError();
                hr = HRESULT_FROM_WIN32( dwRet );
                goto abort;
            }
            if( cbRead != 0 )
            {
                if( ( 1 == cbRead ) || ( 0xFF != pReadBuffer[0] ) || ( 0xFE != pReadBuffer[1] ) )
                {
                    // This file is not unicode, so error out.
                    hr = HRESULT_FROM_WIN32( ERROR_INVALID_DATA );                    
                    goto abort;
                }
            }
            else
            {
                // We just opened an empty file, we must write the Unicode File bits
                BYTE pWriteBuffer[2];
                pWriteBuffer[0] = 0xFF;
                pWriteBuffer[1] = 0xFE;
                if( !::WriteFile( hFile, (LPVOID) pWriteBuffer, 2, &cbWritten, NULL ) )
                {
                    dwRet = GetLastError();
                    hr = HRESULT_FROM_WIN32( dwRet );
                    goto abort;
                }
            }
        }
    }

    if( SUCCEEDED( hr ) )
    {
        if( !::SetFilePointer( hFile, 0, 0, FILE_END ) )
        {
            dwRet = GetLastError();
            hr = HRESULT_FROM_WIN32( dwRet );
        }

        if( SUCCEEDED( hr ) )
        {
            // Create a buffer of text to pass to WriteFile
            GetLocalTime( &sTime);
            _snwprintf_s( wstrHeader,MAX_PATH, MAX_PATH, CONTEXT_SAMPLE_NEW_EVENT_HEADER,
                            wstrEventType, sTime.wYear, sTime.wMonth, sTime.wDay, sTime.wHour, sTime.wMinute, sTime.wSecond );

            if( !::WriteFile( hFile, (LPVOID) wstrHeader,DWORD( wcslen( wstrHeader ) * sizeof( WCHAR )), &cbWritten, NULL ) )
            {
                dwRet = GetLastError();
                hr = HRESULT_FROM_WIN32( dwRet );
            }
        }
    }

    // Now do the writing of the information
    if( SUCCEEDED( hr ) && ( WMS_CONTEXT_PLUGIN_USER_CONTEXT & m_wmsContexts ) )
    {
        hr = WriteContextInformation( hFile, pUserCtx );
    }

    if( SUCCEEDED( hr ) && ( WMS_CONTEXT_PLUGIN_PRESENTATION_CONTEXT & m_wmsContexts ) )
    {
        hr = WriteContextInformation( hFile, pPresentationCtx );
    }

    if( SUCCEEDED( hr ) && ( WMS_CONTEXT_PLUGIN_COMMAND_REQUEST_CONTEXT & m_wmsContexts ) )
    {
        IWMSContext *pContext = NULL;
        hr = pCommandCtx->GetCommandRequest( &pContext );
        if( SUCCEEDED( hr ) )
        {
            hr = WriteContextInformation( hFile, pContext );
        }

        if( NULL != pContext )
        {
            pContext->Release();
            pContext = NULL;
        }
    }

    if( SUCCEEDED( hr ) && ( WMS_CONTEXT_PLUGIN_COMMAND_RESPONSE_CONTEXT & m_wmsContexts ) )
    {
        IWMSContext *pContext = NULL;
        hr = pCommandCtx->GetCommandResponse( &pContext );
        if( SUCCEEDED( hr ) )
        {
            hr = WriteContextInformation( hFile, pContext );
        }

        if( NULL != pContext )
        {
            pContext->Release();
            pContext = NULL;
        }
    }

abort:
    if( INVALID_HANDLE_VALUE != hFile )
    {
        CloseHandle( hFile );
        hFile = INVALID_HANDLE_VALUE;
    }

    // revert to previous error mode
    ::SetErrorMode( uiPrevErrorMode );

    return( hr );
}


// Event Notification Helper Functions
HRESULT CContextPlugin::OnNotifyConnect( IWMSContext *pUserCtx, IWMSContext *pPresentationCtx, IWMSCommandContext *pCommandCtx )
{
    HRESULT hr = S_OK;
    hr = DumpContextInformation( L"Connect", pUserCtx, pPresentationCtx, pCommandCtx );
    return( hr );
}


HRESULT CContextPlugin::OnNotifyDisconnect( IWMSContext *pUserCtx, IWMSContext *pPresentationCtx, IWMSCommandContext *pCommandCtx )
{
    HRESULT hr = S_OK;
    hr = DumpContextInformation( L"Disconnect", pUserCtx, pPresentationCtx, pCommandCtx );
    return( hr );
}


HRESULT CContextPlugin::OnNotifyBeginUserSession( IWMSContext *pUserCtx, IWMSContext *pPresentationCtx, IWMSCommandContext *pCommandCtx )
{
    HRESULT hr = S_OK;
    hr = DumpContextInformation( L"Begin User Session", pUserCtx, pPresentationCtx, pCommandCtx );
    return( hr );
}


HRESULT CContextPlugin::OnNotifyEndUserSession( IWMSContext *pUserCtx, IWMSContext *pPresentationCtx, IWMSCommandContext *pCommandCtx )
{
    HRESULT hr = S_OK;
    hr = DumpContextInformation( L"End User Session", pUserCtx, pPresentationCtx, pCommandCtx );
    return( hr );
}


HRESULT CContextPlugin::OnNotifyDescribe( IWMSContext *pUserCtx, IWMSContext *pPresentationCtx, IWMSCommandContext *pCommandCtx )
{
    HRESULT hr = S_OK;
    hr = DumpContextInformation( L"Describe", pUserCtx, pPresentationCtx, pCommandCtx );
    return( hr );
}


HRESULT CContextPlugin::OnNotifyOpen( IWMSContext *pUserCtx, IWMSContext *pPresentationCtx, IWMSCommandContext *pCommandCtx )
{
    HRESULT hr = S_OK;
    hr = DumpContextInformation( L"Open", pUserCtx, pPresentationCtx, pCommandCtx );
    return( hr );
}


HRESULT CContextPlugin::OnNotifySelectStreams( IWMSContext *pUserCtx, IWMSContext *pPresentationCtx, IWMSCommandContext *pCommandCtx )
{
    HRESULT hr = S_OK;
    hr = DumpContextInformation( L"Select Streams", pUserCtx, pPresentationCtx, pCommandCtx );
    return( hr );
}


HRESULT CContextPlugin::OnNotifyPlay( IWMSContext *pUserCtx, IWMSContext *pPresentationCtx, IWMSCommandContext *pCommandCtx )
{
    HRESULT hr = S_OK;
    hr = DumpContextInformation( L"Play", pUserCtx, pPresentationCtx, pCommandCtx );
    return( hr );
}


HRESULT CContextPlugin::OnNotifyPause( IWMSContext *pUserCtx, IWMSContext *pPresentationCtx, IWMSCommandContext *pCommandCtx )
{
    HRESULT hr = S_OK;
    hr = DumpContextInformation( L"Pause", pUserCtx, pPresentationCtx, pCommandCtx );
    return( hr );
}


HRESULT CContextPlugin::OnNotifyStop( IWMSContext *pUserCtx, IWMSContext *pPresentationCtx, IWMSCommandContext *pCommandCtx )
{
    HRESULT hr = S_OK;
    hr = DumpContextInformation( L"Stop", pUserCtx, pPresentationCtx, pCommandCtx );
    return( hr );
}


HRESULT CContextPlugin::OnNotifyClose( IWMSContext *pUserCtx, IWMSContext *pPresentationCtx, IWMSCommandContext *pCommandCtx )
{
    HRESULT hr = S_OK;
    hr = DumpContextInformation( L"Close", pUserCtx, pPresentationCtx, pCommandCtx );
    return( hr );
}


HRESULT CContextPlugin::OnNotifySetParameter( IWMSContext *pUserCtx, IWMSContext *pPresentationCtx, IWMSCommandContext *pCommandCtx )
{
    HRESULT hr = S_OK;
    hr = DumpContextInformation( L"Set Parameter", pUserCtx, pPresentationCtx, pCommandCtx );
    return( hr );
}


HRESULT CContextPlugin::OnNotifyGetParameter( IWMSContext *pUserCtx, IWMSContext *pPresentationCtx, IWMSCommandContext *pCommandCtx )
{
    HRESULT hr = S_OK;
    hr = DumpContextInformation( L"Get Parameter", pUserCtx, pPresentationCtx, pCommandCtx );
    return( hr );
}


HRESULT CContextPlugin::OnNotifyValidatePushDistribution( IWMSContext *pUserCtx, IWMSContext *pPresentationCtx, IWMSCommandContext *pCommandCtx )
{
    HRESULT hr = S_OK;
    hr = DumpContextInformation( L"Validate Push Distribution", pUserCtx, pPresentationCtx, pCommandCtx );
    return( hr );
}


HRESULT CContextPlugin::OnNotifyCreateDistributionDataPath( IWMSContext *pUserCtx, IWMSContext *pPresentationCtx, IWMSCommandContext *pCommandCtx )
{
    HRESULT hr = S_OK;
    hr = DumpContextInformation( L"Create Distribution Data Path", pUserCtx, pPresentationCtx, pCommandCtx );
    return( hr );
}


HRESULT CContextPlugin::OnNotifyDestroyDistributionDataPath( IWMSContext *pUserCtx, IWMSContext *pPresentationCtx, IWMSCommandContext *pCommandCtx )
{
    HRESULT hr = S_OK;
    hr = DumpContextInformation( L"Destroy Distribution Data Path", pUserCtx, pPresentationCtx, pCommandCtx );
    return( hr );
}


HRESULT CContextPlugin::OnNotifyLog( IWMSContext *pUserCtx, IWMSContext *pPresentationCtx, IWMSCommandContext *pCommandCtx )
{
    HRESULT hr = S_OK;
    hr = DumpContextInformation( L"Log", pUserCtx, pPresentationCtx, pCommandCtx );
    return( hr );
}


HRESULT CContextPlugin::OnNotifyServer( IWMSContext *pUserCtx, IWMSContext *pPresentationCtx, IWMSCommandContext *pCommandCtx )
{
    HRESULT hr = S_OK;
    hr = DumpContextInformation( L"Server", pUserCtx, pPresentationCtx, pCommandCtx );
    return( hr );
}


HRESULT CContextPlugin::OnNotifyPublishingPoint( IWMSContext *pUserCtx, IWMSContext *pPresentationCtx, IWMSCommandContext *pCommandCtx )
{
    HRESULT hr = S_OK;
    hr = DumpContextInformation( L"Publishing Point", pUserCtx, pPresentationCtx, pCommandCtx );
    return( hr );
}


HRESULT CContextPlugin::OnNotifyLimitChange( IWMSContext *pUserCtx, IWMSContext *pPresentationCtx, IWMSCommandContext *pCommandCtx )
{
    HRESULT hr = S_OK;
    hr = DumpContextInformation( L"Limit Change", pUserCtx, pPresentationCtx, pCommandCtx );
    return( hr );
}


HRESULT CContextPlugin::OnNotifyLimitHit( IWMSContext *pUserCtx, IWMSContext *pPresentationCtx, IWMSCommandContext *pCommandCtx )
{
    HRESULT hr = S_OK;
    hr = DumpContextInformation( L"Limit Hit", pUserCtx, pPresentationCtx, pCommandCtx );
    return( hr );
}


HRESULT CContextPlugin::OnNotifyPlugin( IWMSContext *pUserCtx, IWMSContext *pPresentationCtx, IWMSCommandContext *pCommandCtx )
{
    HRESULT hr = S_OK;
    hr = DumpContextInformation( L"Plugin", pUserCtx, pPresentationCtx, pCommandCtx );
    return( hr );
}


HRESULT CContextPlugin::OnNotifyPlaylist( IWMSContext *pUserCtx, IWMSContext *pPresentationCtx, IWMSCommandContext *pCommandCtx )
{
    HRESULT hr = S_OK;
    hr = DumpContextInformation( L"Playlist", pUserCtx, pPresentationCtx, pCommandCtx );
    return( hr );
}


HRESULT CContextPlugin::OnNotifyCache( IWMSContext *pUserCtx, IWMSContext *pPresentationCtx, IWMSCommandContext *pCommandCtx )
{
    HRESULT hr = S_OK;
    hr = DumpContextInformation( L"Cache", pUserCtx, pPresentationCtx, pCommandCtx );
    return( hr );
}


HRESULT CContextPlugin::OnNotifyRemoteCacheOpen( IWMSContext *pUserCtx, IWMSContext *pPresentationCtx, IWMSCommandContext *pCommandCtx )
{
    HRESULT hr = S_OK;
    hr = DumpContextInformation( L"Remote Cache Open", pUserCtx, pPresentationCtx, pCommandCtx );
    return( hr );
}


HRESULT CContextPlugin::OnNotifyRemoteCacheClose( IWMSContext *pUserCtx, IWMSContext *pPresentationCtx, IWMSCommandContext *pCommandCtx )
{
    HRESULT hr = S_OK;
    hr = DumpContextInformation( L"Remote Cache Close", pUserCtx, pPresentationCtx, pCommandCtx );
    return( hr );
}


HRESULT CContextPlugin::OnNotifyRemoteCacheLog( IWMSContext *pUserCtx, IWMSContext *pPresentationCtx, IWMSCommandContext *pCommandCtx )
{
    HRESULT hr = S_OK;
    hr = DumpContextInformation( L"Remote Cache Log", pUserCtx, pPresentationCtx, pCommandCtx );
    return( hr );
}
