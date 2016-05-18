//+-------------------------------------------------------------------------
//
//  Microsoft Windows Media Technologies
//  Copyright (C) Microsoft Corporation. All rights reserved.
//
//  File:       ContextPlugin.h
//
//  Contents:
//
//--------------------------------------------------------------------------
#pragma once

#include "resource.h"       // main symbols

#include "wmsdefs.h"

typedef struct ContextNameHint
{
    LPCWSTR wstrContextName;
    long lContextHint;
} ContextNameHint;


/////////////////////////////////////////////////////////////////////////////
// CContextPlugin
class ATL_NO_VTABLE CContextPlugin : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CContextPlugin, &CLSID_WMSContextPlugin>,
    public IWMSEventNotificationPlugin,
    public IWMSBasicPlugin,
    public IWMSContextPlugin
{

public:
    CContextPlugin();
    ~CContextPlugin();

DECLARE_REGISTRY_RESOURCEID(IDR_CONTEXTPLUGIN)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CContextPlugin)
        COM_INTERFACE_ENTRY(IWMSEventNotificationPlugin)
        COM_INTERFACE_ENTRY(IWMSBasicPlugin)
        COM_INTERFACE_ENTRY(IWMSContextPlugin)	
END_COM_MAP()

public:

    // IWMSBasicPlugin
    STDMETHOD( InitializePlugin )( IWMSContext *pServerContext, IWMSNamedValues *pNamedValues, IWMSClassObject *pClassFactory );
    STDMETHOD( OnHeartbeat )( );
    STDMETHOD( GetCustomAdminInterface )( IDispatch **ppValue );
    STDMETHOD( ShutdownPlugin )();
    STDMETHOD( EnablePlugin ) ( long *pdwFlags, long *pdwHeartbeatPeriod );
    STDMETHOD( DisablePlugin )();

    // IWMSEventNotificationPlugin
    STDMETHOD( GetHandledEvents )( VARIANT *pvarHandledEvents );
    STDMETHOD( OnEvent )( WMS_EVENT *pEvent, IWMSContext *pUserCtx, IWMSContext *pPresentationCtx, IWMSCommandContext *pCommandCtx );

    HRESULT SetOutputPath( BSTR bstrOutputPath );
    HRESULT GetOutputPath( BSTR *pbstrOutputPath );

    HRESULT SetContextTypes( WMS_CONTEXT_PLUGIN_CONTEXT_TYPE wmsContexts )
    {
        if( ( 0 > wmsContexts )
            || ( ( 2 * WMS_CONTEXT_PLUGIN_COMMAND_RESPONSE_CONTEXT ) <= wmsContexts ) )
        {
            return( E_INVALIDARG );
        }
        m_wmsContexts = wmsContexts;
        return( S_OK );
    };
    HRESULT GetContextTypes( WMS_CONTEXT_PLUGIN_CONTEXT_TYPE *wmsContexts )
    {
        HRESULT hr = E_POINTER;
        if( NULL != wmsContexts )
        {
            *wmsContexts = m_wmsContexts;
            hr = S_OK;
        }
        return( hr );
    };

private:
    // Notification Helper Functions
    HRESULT OnNotifyConnect( IWMSContext *pUserCtx, IWMSContext *pPresentationCtx, IWMSCommandContext *pCommandCtx );
    HRESULT OnNotifyDisconnect( IWMSContext *pUserCtx, IWMSContext *pPresentationCtx, IWMSCommandContext *pCommandCtx );
    HRESULT OnNotifyBeginUserSession( IWMSContext *pUserCtx, IWMSContext *pPresentationCtx, IWMSCommandContext *pCommandCtx );
    HRESULT OnNotifyEndUserSession( IWMSContext *pUserCtx, IWMSContext *pPresentationCtx, IWMSCommandContext *pCommandCtx );
    HRESULT OnNotifyDescribe( IWMSContext *pUserCtx, IWMSContext *pPresentationCtx, IWMSCommandContext *pCommandCtx );
    HRESULT OnNotifyOpen( IWMSContext *pUserCtx, IWMSContext *pPresentationCtx, IWMSCommandContext *pCommandCtx );
    HRESULT OnNotifySelectStreams( IWMSContext *pUserCtx, IWMSContext *pPresentationCtx, IWMSCommandContext *pCommandCtx );
    HRESULT OnNotifyPlay( IWMSContext *pUserCtx, IWMSContext *pPresentationCtx, IWMSCommandContext *pCommandCtx );
    HRESULT OnNotifyPause( IWMSContext *pUserCtx, IWMSContext *pPresentationCtx, IWMSCommandContext *pCommandCtx );
    HRESULT OnNotifyStop( IWMSContext *pUserCtx, IWMSContext *pPresentationCtx, IWMSCommandContext *pCommandCtx );
    HRESULT OnNotifyClose( IWMSContext *pUserCtx, IWMSContext *pPresentationCtx, IWMSCommandContext *pCommandCtx );
    HRESULT OnNotifySetParameter( IWMSContext *pUserCtx, IWMSContext *pPresentationCtx, IWMSCommandContext *pCommandCtx );
    HRESULT OnNotifyGetParameter( IWMSContext *pUserCtx, IWMSContext *pPresentationCtx, IWMSCommandContext *pCommandCtx );
    HRESULT OnNotifyValidatePushDistribution( IWMSContext *pUserCtx, IWMSContext *pPresentationCtx, IWMSCommandContext *pCommandCtx );
    HRESULT OnNotifyCreateDistributionDataPath( IWMSContext *pUserCtx, IWMSContext *pPresentationCtx, IWMSCommandContext *pCommandCtx );
    HRESULT OnNotifyDestroyDistributionDataPath( IWMSContext *pUserCtx, IWMSContext *pPresentationCtx, IWMSCommandContext *pCommandCtx );
    HRESULT OnNotifyLog( IWMSContext *pUserCtx, IWMSContext *pPresentationCtx, IWMSCommandContext *pCommandCtx );
    HRESULT OnNotifyServer( IWMSContext *pUserCtx, IWMSContext *pPresentationCtx, IWMSCommandContext *pCommandCtx );
    HRESULT OnNotifyPublishingPoint( IWMSContext *pUserCtx, IWMSContext *pPresentationCtx, IWMSCommandContext *pCommandCtx );
    HRESULT OnNotifyLimitChange( IWMSContext *pUserCtx, IWMSContext *pPresentationCtx, IWMSCommandContext *pCommandCtx );
    HRESULT OnNotifyLimitHit( IWMSContext *pUserCtx, IWMSContext *pPresentationCtx, IWMSCommandContext *pCommandCtx );
    HRESULT OnNotifyPlugin( IWMSContext *pUserCtx, IWMSContext *pPresentationCtx, IWMSCommandContext *pCommandCtx );
    HRESULT OnNotifyPlaylist( IWMSContext *pUserCtx, IWMSContext *pPresentationCtx, IWMSCommandContext *pCommandCtx );
    HRESULT OnNotifyCache( IWMSContext *pUserCtx, IWMSContext *pPresentationCtx, IWMSCommandContext *pCommandCtx );
    HRESULT OnNotifyRemoteCacheOpen( IWMSContext *pUserCtx, IWMSContext *pPresentationCtx, IWMSCommandContext *pCommandCtx );
    HRESULT OnNotifyRemoteCacheClose( IWMSContext *pUserCtx, IWMSContext *pPresentationCtx, IWMSCommandContext *pCommandCtx );
    HRESULT OnNotifyRemoteCacheLog( IWMSContext *pUserCtx, IWMSContext *pPresentationCtx, IWMSCommandContext *pCommandCtx );

    // Private Member Functions
    HRESULT CreateArrayOfEvents( VARIANT *pvarEvents, WMS_EVENT_TYPE *pWMSEvents, long nNumEvents);

    // Helper Functions
    HRESULT DumpContextInformation( LPCWSTR wstrEventType, IWMSContext *pUserCtx, IWMSContext *pPresentationCtx, IWMSCommandContext *pCommandContext );
    HRESULT WriteContextInformation( HANDLE hFile, IWMSContext *pContext );

    // Data Storage Functions
    HRESULT LoadConfigValues();
    HRESULT SaveConfigValues();


private:
    // Private Member Variables
    IWMSServer                          *m_pServer;
    IWMSNamedValues                     *m_pNamedValues;
    WMS_CONTEXT_PLUGIN_CONTEXT_TYPE     m_wmsContexts;
    BSTR                                m_bstrOutputPath;

    static const ContextNameHint s_UserContextHintValues[]; 
    static const ContextNameHint s_PresentationContextHintValues[]; 
    static const ContextNameHint s_CommandContextHintValues[];

    static WCHAR *s_wstrNamedValueOutputPath;
    static WCHAR *s_wstrNamedValueContextTypes;

};

