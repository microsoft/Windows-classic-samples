//*****************************************************************************
//
// Microsoft Windows Media
// Copyright (C) Microsoft Corporation. All rights reserved.
//
// FileName:            ProxyPluginImpl.h
//
// Abstract:
//
//*****************************************************************************

#pragma once

#ifndef __PROXYPLUGINIMPL_H_
#define __PROXYPLUGINIMPL_H_

#include "resource.h"       // main symbols

#include "WMSBasicPlugin.h"
#include "StreamCache.h"
#include "ProxyPlugin.h"
#include "wmscontext.h"

/////////////////////////////////////////////////////////////////////////////
//
// CProxyPlugin
//
/////////////////////////////////////////////////////////////////////////////
class ATL_NO_VTABLE CProxyPlugin : 
        public CComObjectRootEx<CComMultiThreadModel>,
        public CComCoClass<CProxyPlugin, &CLSID_ProxyPlugin>,
        public IWMSCacheProxy,
        public IWMSCacheProxyServerCallback,
        public IWMSBasicPlugin
{
public:
                    CProxyPlugin();
    virtual         ~CProxyPlugin();

DECLARE_REGISTRY_RESOURCEID(IDR_PROXYPLUGIN)
DECLARE_NOT_AGGREGATABLE(CProxyPlugin)
DECLARE_GET_CONTROLLING_UNKNOWN()
DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CProxyPlugin)
    COM_INTERFACE_ENTRY(IWMSCacheProxy)
    COM_INTERFACE_ENTRY(IWMSCacheProxyServerCallback)
    COM_INTERFACE_ENTRY(IWMSBasicPlugin)
END_COM_MAP()

    //
    // IWMSCacheProxy
    //
    HRESULT STDMETHODCALLTYPE QueryCache( 
                    BSTR bstrOriginUrl,
                    IWMSContext *pUserContext,
                    IWMSCommandContext *pCommandContext,
                    IWMSContext *pPresentationContext,
                    long QueryType,
                    IWMSCacheProxyCallback *pCallback,
                    VARIANT varContext
                    );
    HRESULT STDMETHODCALLTYPE QueryCacheMissPolicy(
                    BSTR bstrOriginUrl,
                    IWMSContext *pUserContext,
                    IWMSCommandContext *pCommandContext,
                    IWMSContext *pPresentationContext,
                    IUnknown *pCachePluginContext,
                    long QueryType,
                    IWMSCacheProxyCallback *pCallback,
                    VARIANT varContext
                    );

    HRESULT STDMETHODCALLTYPE RemoveCacheItem( 
                    BSTR bstrOriginUrl,
                    IWMSCacheProxyCallback *pCallback,
                    VARIANT varContext
                    )
    {
        return( E_NOTIMPL );
    }
    HRESULT STDMETHODCALLTYPE RemoveAllCacheItems( 
                    IWMSCacheProxyCallback *pCallback,
                    VARIANT varContext
                    )
    {
        return( E_NOTIMPL );
    }
    HRESULT STDMETHODCALLTYPE AddCacheItem(
                    BSTR bstrOriginUrl,
                    BSTR bstrPrestuffUrl,
                    long lExpiration,
                    long lBandwidth,
                    long lRemoteEventFlags,
                    IWMSCacheProxyCallback *pCallback,
                    VARIANT varContext
                    )
    {
        return( E_NOTIMPL );
    }
    HRESULT STDMETHODCALLTYPE QuerySpaceForCacheItem(
                    long lContentSizeLow,
                    long lContentSizeHigh,
                    VARIANT_BOOL *pvarfSpaceAvail
                    )
    {
        return( E_NOTIMPL );
    }
    HRESULT STDMETHODCALLTYPE FindCacheItem(
                    BSTR pszOriginUrl,
                    IWMSCacheItemDescriptor **ppCacheItemDescriptor
                    )
    {
        return( E_NOTIMPL );
    }
    HRESULT STDMETHODCALLTYPE CreateCacheItemCollection(
                    IWMSCacheItemCollection **ppCacheItemCollection
                    )
    {
        return( E_NOTIMPL );
    }
    HRESULT STDMETHODCALLTYPE OnCacheClientClose( 
                    HRESULT resultHr,
                    IWMSContext *pUserContext,
                    IWMSContext *pPresentationContext
                    );

    //
    // IWMSCacheProxyServerCallback
    //
    HRESULT STDMETHODCALLTYPE OnGetContentInformation(
                    long lHr,
                    IWMSContext *pContentInfo,
                    VARIANT varContext
                    );
    HRESULT STDMETHODCALLTYPE OnCompareContentInformation(
                    long lHr,
                    WMS_CACHE_VERSION_COMPARE_RESPONSE CompareResponse,
                    IWMSContext *pNewContentInfo,
                    VARIANT varContext
                    )
    {
        return( E_NOTIMPL );
    }
    HRESULT STDMETHODCALLTYPE OnDownloadContentProgress(
                    long lHr,
                    WMS_RECORD_PROGRESS_OPCODE opCode,
                    IWMSContext *pArchiveContext,
                    VARIANT varContext
                    )
    {
        return( E_NOTIMPL );
    }
    HRESULT STDMETHODCALLTYPE OnDownloadContentFinished(
                    long lHr,
                    SAFEARRAY *psaArchiveContexts,
                    VARIANT varContext
                    )
    {
        return( E_NOTIMPL );
    }
    HRESULT STDMETHODCALLTYPE OnCancelDownloadContent(
                    long lHr,
                    VARIANT varContext
                    )
    {
        return( E_NOTIMPL );
    }

    //
    // IWMSBasicPlugin
    //
    HRESULT STDMETHODCALLTYPE InitializePlugin( 
                    IWMSContext *pServerContext,
                    IWMSNamedValues *pNamedValues,
                    IWMSClassObject *pClassFactory
                    );
    HRESULT STDMETHODCALLTYPE ShutdownPlugin();
    HRESULT STDMETHODCALLTYPE GetCustomAdminInterface( 
                    IDispatch **ppValue
                    );
    HRESULT STDMETHODCALLTYPE OnHeartbeat();    
    HRESULT STDMETHODCALLTYPE EnablePlugin(
                    long *pdwFlags,
                    long *pdwHeartbeatPeriod );
    HRESULT STDMETHODCALLTYPE DisablePlugin();

private:
    class COpState : public IUnknown
    {
    public:
        enum OpTypes
        {
            OP_QUERY_CACHE,
            OP_QUERY_CACHE_MISS_POLICY
        };

        OpTypes                   m_Op;
        CComBSTR                  m_bstrOriginUrl;
        CComBSTR                  m_bstrProtocol;
        CComBSTR                  m_bstrHost;
        CComBSTR                  m_bstrPath;
        IWMSContext              *m_pPresentationContext;
        IWMSCacheProxyCallback   *m_pCacheProxyCallback;
        CComVariant               m_varContext;
        DWORD                     m_dwProtocolIndex;
        WORD                      m_wPort;

        COpState()
        {
            m_cRef = 1;
            m_pPresentationContext = NULL;
            m_pCacheProxyCallback = NULL;
            m_wPort = 0;
        }

        ~COpState()
        {
            if( NULL != m_pPresentationContext )
            {
                m_pPresentationContext->Release();
            }

            if( NULL != m_pCacheProxyCallback )
            {
                m_pCacheProxyCallback->Release();
            }
        }

        STDMETHOD_(ULONG, AddRef)()
        {
            ULONG cRef = InterlockedIncrement( &m_cRef );

            return( cRef );
        }

        STDMETHOD_(ULONG, Release)()
        {
            ULONG cRef = InterlockedDecrement( &m_cRef );
            if( 0 == cRef )
            {
                delete this;
                return( 0 );
            }

            return( cRef );
        }

        STDMETHOD( QueryInterface )( REFIID riid, void **ppvObject )
        {
            if( NULL == ppvObject )
            {
                return( E_POINTER );
            }

            *ppvObject = NULL;
            
            if( IID_IUnknown == riid )
            {
                AddRef();
                *ppvObject = ( IUnknown *) this;
                return( S_OK );
            }

            return( E_NOINTERFACE );
        }

    private:
        LONG m_cRef;
    };

    BOOL ParseUrl( BSTR bstrUrl,
                   BSTR *pbstrProtocol,
                   BSTR *pbstrHost,
                   BSTR *pbstrPath,
                   WORD *pwPort
                   );

    IWMSCacheProxyServer *m_pICacheProxyServer;
};

#endif //__PROXYPLUGINIMPL_H_
