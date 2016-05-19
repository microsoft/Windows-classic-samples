//*****************************************************************************
//
// Microsoft Windows Media
// Copyright (C) Microsoft Corporation. All rights reserved.
//
// FileName:            SDKSamplePlaylistParser.h
//
// Abstract:
//
//*****************************************************************************

#pragma once

#ifndef __SDKSAMPLEPLAYLISTPARSER_H_
#define __SDKSAMPLEPLAYLISTPARSER_H_

#include "resource.h"       // main symbols

// Windows Media Server SDK
#include "WMSBasicPlugin.h"
#include "WMSPlaylistParser.h"

/////////////////////////////////////////////////////////////////////////////
class ATL_NO_VTABLE CSDKSamplePlaylistParser :
    public CComObjectRootEx<CComMultiThreadModel>,
    public IWMSPlaylistParser
{
public:

BEGIN_COM_MAP(CSDKSamplePlaylistParser)
    COM_INTERFACE_ENTRY(IWMSPlaylistParser)
END_COM_MAP()

    // CSDKSamplePlaylistParser
    CSDKSamplePlaylistParser();
    virtual ~CSDKSamplePlaylistParser();
    HRESULT Initialize(
        IWMSCommandContext *pCommandContext,
        IWMSContext *pUserContext,
        IWMSContext *pPresentationContext,
        IWMSContext *pServerContext,
        IWMSNamedValues *pNamedValues,
        DWORD dwFlags,
        IWMSClassObject *pClassFactory
        );
    HRESULT Shutdown();

    // IWMSPlaylistParser
    virtual HRESULT STDMETHODCALLTYPE ReadPlaylist( 
            INSSBuffer __RPC_FAR *pBuffer,
            IXMLDOMDocument __RPC_FAR *pPlayList,
            IWMSPlaylistParserCallback __RPC_FAR *pCallback,
            QWORD qwContext
            );
    virtual HRESULT STDMETHODCALLTYPE WritePlaylist( 
            IXMLDOMDocument __RPC_FAR *pPlayList,
            IWMSPlaylistParserCallback __RPC_FAR *pCallback,
            QWORD qwContext
            );
    virtual HRESULT STDMETHODCALLTYPE ReadPlaylistFromDirectory( 
            IWMSDirectory __RPC_FAR *pDirectory,
            LPWSTR pszwFilePattern,
            IXMLDOMDocument __RPC_FAR *pPlaylist,
            IWMSPlaylistParserCallback __RPC_FAR *pCallback,
            QWORD qwContext
            );

protected:

    CComPtr<IWMSCommandContext> m_spCommandContext;
    CComPtr<IWMSContext>         m_spUserContext;
    CComPtr<IWMSContext>         m_spPresentationContext;
    CComPtr<IWMSContext>         m_spServerContext;
    CComPtr<IWMSNamedValues>     m_spNamedValues;
    CComPtr<IWMSClassObject>     m_spClassFactory;

}; // End of CSDKSamplePlaylistParser

typedef CComObject< CSDKSamplePlaylistParser > CComSDKSamplePlaylistParser;




/////////////////////////////////////////////////////////////////////////////
// CSDKSamplePlaylistParserPlugin
class ATL_NO_VTABLE CSDKSamplePlaylistParserPlugin : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CSDKSamplePlaylistParserPlugin, &CLSID_SDKSamplePlaylistParser>,
    public IWMSBasicPlugin,
    public IWMSPlaylistParserPlugin
{
public:
    CSDKSamplePlaylistParserPlugin();
    ~CSDKSamplePlaylistParserPlugin();


DECLARE_REGISTRY_RESOURCEID(IDR_SDKSAMPLEPLAYLISTPARSER)
DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CSDKSamplePlaylistParserPlugin)
    COM_INTERFACE_ENTRY(IWMSBasicPlugin)
    COM_INTERFACE_ENTRY(IWMSPlaylistParserPlugin)
END_COM_MAP()

public:
    // IWMSBasicPlugin
    virtual HRESULT STDMETHODCALLTYPE InitializePlugin( 
                    IWMSContext *pServerContext,
                    IWMSNamedValues *pNamedValues,
                    IWMSClassObject *pClassFactory
                    );
    virtual HRESULT STDMETHODCALLTYPE GetCustomAdminInterface( 
                    IDispatch **ppValue
                    );
    virtual HRESULT STDMETHODCALLTYPE OnHeartbeat();

    STDMETHOD( ShutdownPlugin )();
    STDMETHOD( EnablePlugin )( long *pdwFlags, long *pdwHeartbeatPeriod );
    STDMETHOD( DisablePlugin )();

    // IWMSPlaylistParserPlugin
    virtual HRESULT STDMETHODCALLTYPE CreatePlaylistParser(
                    IWMSCommandContext *pCommandContext,
                    IWMSContext *pUser,
                    IWMSContext *pPresentation,
                    DWORD dwFlags,
                    IWMSClassObject *pFactory,
                    IWMSBufferAllocator *pBufferAllocator,
                    IWMSPlaylistParserPluginCallback *pCallback,
                    QWORD qwContext
                    );

protected:

    CComPtr<IWMSNamedValues>      m_spNamedValues;
    CComPtr<IWMSContext>          m_spServerContext; 
    CComPtr<IWMSClassObject>      m_spClassFactory;
};

#endif //__SDKSAMPLEPLAYLISTPARSER_H_
