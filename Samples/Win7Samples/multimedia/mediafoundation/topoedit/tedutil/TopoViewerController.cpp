// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "stdafx.h"

#include "TopoViewerController.h"

STDAPI TEDCreateTopoViewer(ITedVideoWindowHandler* pVideoWindowHandler, ITedPropertyController* pPropertyController, ITedTopoEventHandler* pEventHandler, __out ITedTopoView** ppTopoViewer)
{
    HRESULT hr = S_OK;
    
    if(NULL == ppTopoViewer)
    {
        return E_POINTER;
    }

    CComObject<CTopoViewerController>* pController;
    hr = CComObject<CTopoViewerController>::CreateInstance(&pController);
    if(FAILED(hr))
    {
        return hr;
    }

    hr = pController->Init(pVideoWindowHandler, pPropertyController, pEventHandler);
    if(FAILED(hr)) 
    {
        pController->Release();
        return hr;
    }

    return pController->QueryInterface(IID_ITedTopoView, (void**) ppTopoViewer);
}

CTopoViewerController::CTopoViewerController()
    : m_pWindow(NULL)
    , m_pEditor(NULL)
{
}

CTopoViewerController::~CTopoViewerController()
{
    delete m_pWindow;
    delete m_pEditor;
}

HRESULT CTopoViewerController::Init(ITedVideoWindowHandler* pVideoWindowHandler, ITedPropertyController* pPropertyController, ITedTopoEventHandler* pEventHandler)
{
    HRESULT hr = S_OK;
    m_spVideoHandler = pVideoWindowHandler;
    
    m_pWindow = new CTopoViewerWindow(hr);
    if(NULL == m_pWindow)
    {
        hr = E_OUTOFMEMORY;
    }
    IFC( hr );

    m_pEditor = new CTedTopologyEditor();
    if(NULL == m_pEditor)
    {
        hr = E_OUTOFMEMORY;
    }
    IFC( hr );
    
    m_pWindow->Init(this, m_pEditor);
    m_pEditor->Init(pVideoWindowHandler, pPropertyController, pEventHandler, m_pWindow);

Cleanup:
    return hr;
}

HRESULT CTopoViewerController::CreateTopoWindow(LPCWSTR szTitle, DWORD dwStyle, DWORD x, DWORD y, DWORD width, DWORD height, LONG_PTR hWndParent, LONG_PTR* phWnd)
{
    HWND hWnd;

    RECT clientRect;
    clientRect.left = x;
    clientRect.top = y;
    clientRect.right = x + width;
    clientRect.bottom = y + height;
    
    hWnd = m_pWindow->Create((HWND) hWndParent, clientRect, szTitle, dwStyle);

    if(NULL == hWnd)
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    if(NULL != phWnd)
    {
        *phWnd = (LONG_PTR) hWnd;
    }
    
    return S_OK;
}

HRESULT CTopoViewerController::CloseTopoWindow()
{
    if(m_pWindow->m_hWnd)
    {
        m_pWindow->SendMessage(WM_CLOSE, 0, 0);
    }

    return S_OK;
}

HRESULT CTopoViewerController::SetEditable(BOOL fEditable)
{
    m_pWindow->SetEditable(fEditable);
    m_pEditor->SetEditable(fEditable);

    return S_OK;
}

HRESULT CTopoViewerController::IsSaved(BOOL* pfIsSaved)
{
    if(NULL == pfIsSaved)
    {
        return E_POINTER; 
    }

    *pfIsSaved = m_pEditor->IsSaved();

    return S_OK;
}

HRESULT CTopoViewerController::NewTopology()
{
    m_pWindow->ClearView();
    return m_pEditor->NewTopology();
}

HRESULT CTopoViewerController::ShowTopology(IMFTopology* pTopology, LPCWSTR wszSourceURL)
{
    m_pWindow->ClearView();
    return m_pEditor->ShowTopology(pTopology, wszSourceURL);
}

HRESULT CTopoViewerController::MergeTopology(IMFTopology* pTopology)
{
    return m_pEditor->MergeTopology(pTopology);
}

HRESULT CTopoViewerController::LoadTopology(LPCWSTR szFilename)
{
    return m_pEditor->LoadTopology(szFilename);
}

HRESULT CTopoViewerController::SaveTopology(LPCWSTR szFilename)
{
    return m_pEditor->SaveTopology(szFilename);
}

HRESULT CTopoViewerController::GetTopology(IMFTopology** ppTopology, BOOL* pfIsProtected)
{
    return m_pEditor->GetTopology(ppTopology, pfIsProtected);
}

HRESULT CTopoViewerController::AddSource(LPCWSTR pszSourceURL)
{
    HRESULT hr = S_OK;
    CTedSourceNode* pSourceNode;
    CTedNodeCreator* pNodeCreator = CTedNodeCreator::GetSingleton();
    IFC( pNodeCreator->CreateSource(pszSourceURL, NULL, &pSourceNode) );
    IFC( m_pEditor->AddNode(pSourceNode) );

Cleanup:
    return hr;
}

HRESULT CTopoViewerController::AddSAR()
{
    HRESULT hr = S_OK;
    CTedAudioOutputNode* pSARNode = NULL;
    CTedNodeCreator* pNodeCreator = CTedNodeCreator::GetSingleton();
    IFC( pNodeCreator->CreateSAR(&pSARNode) );
    IFC( m_pEditor->AddNode(pSARNode) );

Cleanup:
    if(FAILED(hr))
    {
        delete pSARNode;
    }
    
    return hr;
}

HRESULT CTopoViewerController::AddEVR()
{
    HRESULT hr = S_OK;
    CTedVideoOutputNode* pEVRNode = NULL;
    CTedNodeCreator* pNodeCreator = CTedNodeCreator::GetSingleton();
    IFC( pNodeCreator->CreateEVR(m_spVideoHandler, &pEVRNode) );
    IFC( m_pEditor->AddNode(pEVRNode) );

Cleanup:
    if(FAILED(hr))
    {
        delete pEVRNode;
    }
    
    return hr;
}

HRESULT CTopoViewerController::AddTransform(GUID gidTransformID, LPCWSTR szTransformName)
{
    HRESULT hr = S_OK;
    CTedTransformNode* pTransformNode = NULL;
    CTedNodeCreator* pNodeCreator = CTedNodeCreator::GetSingleton();
    IFC( pNodeCreator->CreateTransform(gidTransformID, szTransformName, &pTransformNode) );
    IFC( m_pEditor->AddNode(pTransformNode) );

Cleanup:
    if(FAILED(hr))
    {
        delete pTransformNode;
    }
    
    return hr;
}

HRESULT CTopoViewerController::AddTransformActivate(IMFActivate* pTransformActivate)
{
    HRESULT hr = S_OK;
    CTedTransformNode* pTransformNode = NULL;
    CTedNodeCreator* pNodeCreator = CTedNodeCreator::GetSingleton();
    IFC( pNodeCreator->CreateTransform(pTransformActivate, &pTransformNode) );
    IFC( m_pEditor->AddNode(pTransformNode) );

Cleanup:
    if(FAILED(hr))
    {
        delete pTransformNode;
    }
    
    return hr;
}

HRESULT CTopoViewerController::AddTee()
{
    HRESULT hr = S_OK;
    CTedTeeNode* pTeeNode = NULL;
    CTedNodeCreator* pNodeCreator = CTedNodeCreator::GetSingleton();
    IFC( pNodeCreator->CreateTee(&pTeeNode) );
    IFC( m_pEditor->AddNode(pTeeNode) );

Cleanup:
    if(FAILED(hr))
    {
        delete pTeeNode;
    }
    
    return hr;
}

HRESULT CTopoViewerController::AddSink(IMFMediaSink* pSink)
{
    HRESULT hr = S_OK;
    CTedCustomOutputNode* pSinkNode = NULL;
    CTedNodeCreator* pNodeCreator = CTedNodeCreator::GetSingleton();
    IFC( pNodeCreator->CreateCustomSink(pSink, &pSinkNode) );
    IFC( m_pEditor->AddNode(pSinkNode) );

Cleanup:
    if(FAILED(hr))
    {
        delete pSinkNode;
    }

    return hr;
}

HRESULT CTopoViewerController::AddCustomSink(GUID gidSinkID)
{
    HRESULT hr = S_OK;
    CTedCustomOutputNode* pSinkNode = NULL;
    CTedNodeCreator* pNodeCreator = CTedNodeCreator::GetSingleton();
    IFC( pNodeCreator->CreateCustomSink(gidSinkID, &pSinkNode) );
    IFC( m_pEditor->AddNode(pSinkNode) );

Cleanup:
    if(FAILED(hr))
    {
        delete pSinkNode;
    }

    return hr;
}

HRESULT CTopoViewerController::AddCaptureSource(IMFMediaSource* pSource)
{
    HRESULT hr = S_OK;
    CTedSourceNode* pSourceNode;
    CTedNodeCreator* pNodeCreator = CTedNodeCreator::GetSingleton();
    IFC( pNodeCreator->CreateCaptureSource(pSource, &pSourceNode) );
    IFC( m_pEditor->AddNode(pSourceNode) );

Cleanup:
	if(FAILED(hr))
	{
		delete pSourceNode;
	}
    
    return hr;
}

HRESULT CTopoViewerController::DeleteSelectedNode()
{
    CVisualObject* pSelectedVisual = m_pWindow->GetSelectedVisual();
    
    if(NULL == pSelectedVisual)
    {
        return S_OK;
    }

    m_pWindow->HandleDelete();
    
    return S_OK;
}

HRESULT CTopoViewerController::SpySelectedNode()
{
    return m_pWindow->SpySelectedVisual();
}
