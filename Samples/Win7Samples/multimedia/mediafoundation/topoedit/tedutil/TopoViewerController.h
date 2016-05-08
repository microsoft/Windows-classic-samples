// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

#include "tedutil.h"
#include "topoviewerwindow.h"
#include "tededit.h"
#include "resource.h"

class CTopoViewerController 
    : public IDispatchImpl<ITedTopoView, &IID_ITedTopoView, &LIBID_TedUtil>
    , public CComObjectRoot
    , public CComCoClass<CTopoViewerController, &CLSID_CTopoViewerController>
{
public:
    CTopoViewerController();
    ~CTopoViewerController();

    BEGIN_COM_MAP(CTopoViewerController)
        COM_INTERFACE_ENTRY(ITedTopoView)
        COM_INTERFACE_ENTRY(IDispatch)
    END_COM_MAP()

    DECLARE_REGISTRY_RESOURCEID(IDR_TOPOVIEWERCONTROLLER);
    DECLARE_CLASSFACTORY();
    
    HRESULT STDMETHODCALLTYPE Init(ITedVideoWindowHandler* pVideoWindowHandler, ITedPropertyController* pController, ITedTopoEventHandler* pEventHandler);

    HRESULT STDMETHODCALLTYPE CreateTopoWindow(LPCWSTR szTitle, DWORD dwStyle, DWORD x, DWORD y, DWORD width, DWORD height, LONG_PTR hWndParent, LONG_PTR* phWnd);
    HRESULT STDMETHODCALLTYPE CloseTopoWindow();
    HRESULT STDMETHODCALLTYPE SetEditable(BOOL fEditable);

    HRESULT STDMETHODCALLTYPE IsSaved(BOOL* pfIsSaved);

    HRESULT STDMETHODCALLTYPE NewTopology();
    HRESULT STDMETHODCALLTYPE ShowTopology(IMFTopology* pTopology, LPCWSTR wszSourceURL);
    HRESULT STDMETHODCALLTYPE MergeTopology(IMFTopology* pTopology);
    HRESULT STDMETHODCALLTYPE LoadTopology(LPCWSTR szFilename);
    HRESULT STDMETHODCALLTYPE SaveTopology(LPCWSTR szFilename);
    HRESULT STDMETHODCALLTYPE GetTopology(IMFTopology** ppTopology, BOOL* pfIsProtected);

    HRESULT STDMETHODCALLTYPE AddSource(LPCWSTR szSourceURL);
    HRESULT STDMETHODCALLTYPE AddSAR();
    HRESULT STDMETHODCALLTYPE AddEVR();
    HRESULT STDMETHODCALLTYPE AddTransform(GUID gidTransformID, LPCWSTR szTransformName);
    HRESULT STDMETHODCALLTYPE AddTransformActivate(IMFActivate* pTransformActivate);
    HRESULT STDMETHODCALLTYPE AddTee();
    HRESULT STDMETHODCALLTYPE AddSink(IMFMediaSink* pSink);
    HRESULT STDMETHODCALLTYPE AddCustomSink(GUID gidSinkID);
    HRESULT STDMETHODCALLTYPE AddCaptureSource(IMFMediaSource* pSource);

    HRESULT STDMETHODCALLTYPE DeleteSelectedNode();
    HRESULT STDMETHODCALLTYPE SpySelectedNode();

private:
    CTopoViewerWindow* m_pWindow;
    CTedTopologyEditor* m_pEditor;
    CComPtr<ITedVideoWindowHandler> m_spVideoHandler;
};