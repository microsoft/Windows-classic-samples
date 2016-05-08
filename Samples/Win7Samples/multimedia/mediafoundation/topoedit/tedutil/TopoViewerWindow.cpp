// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "stdafx.h"

#include "TopoViewerWindow.h"

#include <assert.h>

HRESULT TEDCreateTopoViewerWindow(LPCWSTR szTitle, DWORD dwStyle, RECT clientRect, HWND hWndParent, HWND* phWnd)
{
    ITedTopoView* pTopoView = NULL;
    HRESULT hr = TEDCreateTopoViewer(NULL, NULL, NULL, &pTopoView);
    if(FAILED(hr)) return hr;

    hr = pTopoView->CreateTopoWindow(szTitle, dwStyle, clientRect.left, clientRect.top, clientRect.right - clientRect.left, clientRect.bottom - clientRect.top, (LONG_PTR) hWndParent, (LONG_PTR*) phWnd);

    pTopoView->Release();

    return hr;
}

CTopoViewerWindow::CTopoViewerWindow(HRESULT& hr)
    : m_pTree(NULL)
    , m_pFocus(NULL)
    , m_pSelected(NULL)
    , m_pEditor(NULL)
    , m_iLeftViewStart(0)
    , m_iTopViewStart(0)
    , m_iTopologyWidth(0)
    , m_iTopologyHeight(0)
    , m_fEditable(TRUE)
{
    m_pTree = new CVisualTree();
    
    if(m_pTree == NULL)
    {
        hr = E_OUTOFMEMORY;
    }
    else
    {
        hr = S_OK;
    }
}

CTopoViewerWindow::~CTopoViewerWindow()
{
    delete m_pTree;
}

void CTopoViewerWindow::Init(ITedTopoView* pController, CTedTopologyEditor* pEditor)
{
    m_pController = pController;
    m_pController->AddRef();

    m_pEditor = pEditor;
}

void CTopoViewerWindow::ClearView() 
{
    delete m_pTree;
    m_pTree = new CVisualTree;
    m_pSelected = NULL;

    assert( m_hWnd != NULL);
    InvalidateRect(NULL);
}

LRESULT CTopoViewerWindow::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    ShowScrollBar(SB_HORZ, TRUE);
    ShowScrollBar(SB_VERT, TRUE);

    bHandled = TRUE;
    return 0;
}

LRESULT CTopoViewerWindow::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = TRUE;
	return 0;
}

LRESULT CTopoViewerWindow::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    ResizeScrollBars();
    
    bHandled = true;
    return 0;
}

LRESULT CTopoViewerWindow::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    CVisualDrawContext ctx(&m_Transform);

    if(ctx.BeginPaint(m_hWnd))
    {
        m_pTree->Draw(ctx);
        ctx.EndPaint();
    }
    else
    {
        return 1;
    }

    return 0;
}

LRESULT CTopoViewerWindow::OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    if(m_pFocus)
    {
        POINT pt;
        CVisualPoint visPt;

        pt.x = GET_X_LPARAM(lParam); 
        pt.y = GET_Y_LPARAM(lParam); 
        visPt  = m_Transform.ScreenToVisual(pt);

        if(!m_pFocus->GetHandler()->OnMouseMove(m_pFocus, visPt))
        {
            m_pFocus = NULL;
        }
        else
        {
            UINT32 maxXExtent = UINT32( m_pFocus->Rect().x() + m_pFocus->Rect().w() + 10 );
            UINT32 maxYExtent = UINT32( m_pFocus->Rect().y() + m_pFocus->Rect().h() + 10 );

            if(m_iTopologyWidth < maxXExtent) m_iTopologyWidth = maxXExtent;
            if(m_iTopologyHeight < maxYExtent) m_iTopologyHeight = maxYExtent;

            RECT rect;
            GetClientRect(&rect);

            UINT32 iScrollWidth = 0;
            if(m_iTopologyWidth > UINT32(rect.right - rect.left) || m_iTopologyHeight > UINT32(rect.bottom - rect.top))
            {
                ResizeScrollBars();
            }

            InvalidateRect(NULL);
        }
    }
    
    bHandled = TRUE;
    return 0;
}

LRESULT CTopoViewerWindow::OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    POINT pt;
    CVisualPoint visPt;
    CVisualObject * pHitObj;

    pt.x = GET_X_LPARAM(lParam); 
    pt.y = GET_Y_LPARAM(lParam); 
    visPt  = m_Transform.ScreenToVisual(pt);

    if(m_pSelected) {
        m_pSelected->Select(false);
        m_pSelected = NULL;
    }

    BOOL found = m_pTree->HitTest(visPt, &pHitObj);
    if(found) {
        m_pSelected = pHitObj;
        m_pSelected->Select(true);
        if(m_pSelected->GetHandler()) m_pSelected->GetHandler()->OnFocus(m_pSelected);
    }

    if(!m_pFocus)
    {
        if(found && pHitObj->GetHandler()) {
            m_pFocus = pHitObj;
        }
    }

    if(m_pFocus && !m_pFocus->GetHandler()->OnLButtonDown(m_pFocus, visPt))
    {
        m_pFocus = NULL;
    }

    InvalidateRect(NULL);

    bHandled = TRUE;
    return 0;
}

LRESULT CTopoViewerWindow::OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    POINT pt;
    CVisualPoint visPt;
    CVisualObject * pHitObj;

    pt.x = GET_X_LPARAM(lParam); 
    pt.y = GET_Y_LPARAM(lParam); 
    visPt  = m_Transform.ScreenToVisual(pt);

    if(!m_pFocus)
    {
        if(!m_pTree->HitTest(visPt, &pHitObj))
        {
            goto Cleanup;
        }

        if(!pHitObj->GetHandler())
        {
            goto Cleanup;
        }

        m_pFocus = pHitObj;
    }

    if(!m_pFocus->GetHandler()->OnLButtonUp(m_pFocus, visPt))
    {
        m_pFocus = NULL;
    }


    InvalidateRect(NULL);

Cleanup:

    bHandled = TRUE;
    return 0;
}

LRESULT CTopoViewerWindow::OnLButtonDoubleClick(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    POINT pt;
    CVisualPoint visPt;
    CVisualObject* pHitObj;

    pt.x = GET_X_LPARAM(lParam);
    pt.y = GET_Y_LPARAM(lParam);
    visPt = m_Transform.ScreenToVisual(pt);

    if(m_pTree->HitTest(visPt, &pHitObj))
    {
        if(!pHitObj->GetHandler())
        {
            goto Cleanup;
        }

        pHitObj->GetHandler()->OnLButtonDoubleClick(pHitObj, visPt);
    }

Cleanup:
    return 0;
}

LRESULT CTopoViewerWindow::OnEraseBkgnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    bHandled = true;
    
    return 1;
}

LRESULT CTopoViewerWindow::OnHScroll(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    SCROLLINFO si;
    
     si.cbSize = sizeof (si);
     si.fMask  = SIF_ALL;

     GetScrollInfo(SB_HORZ, &si);

     switch (LOWORD (wParam))
     {
     case SB_LINELEFT: 
          si.nPos -= 1;
          break;
     case SB_LINERIGHT: 
          si.nPos += 1;
          break;
     case SB_PAGELEFT:
          si.nPos -= si.nPage;
          break;
     case SB_PAGERIGHT:
          si.nPos += si.nPage;
          break;
     case SB_THUMBTRACK: 
          si.nPos = si.nTrackPos;
          break;
     default:
          break;
     }

     si.fMask = SIF_POS;
     SetScrollInfo(SB_HORZ, &si, TRUE);
     GetScrollInfo(SB_HORZ, &si);
     
     m_iLeftViewStart = si.nPos;

     m_Transform.SetPointOffset(-double(m_iLeftViewStart), -double(m_iTopViewStart));
     InvalidateRect(NULL);
     
     bHandled = true;
     return 0;
}

LRESULT CTopoViewerWindow::OnVScroll(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    SCROLLINFO si;
    
     si.cbSize = sizeof (si);
     si.fMask  = SIF_ALL;

     GetScrollInfo(SB_VERT, &si);

     switch (LOWORD (wParam))
     {
     case SB_LINELEFT: 
          si.nPos -= 1;
          break;
     case SB_LINERIGHT: 
          si.nPos += 1;
          break;
     case SB_PAGELEFT:
          si.nPos -= si.nPage;
          break;
     case SB_PAGERIGHT:
          si.nPos += si.nPage;
          break;
     case SB_THUMBTRACK: 
          si.nPos = si.nTrackPos;
          break;
     default:
          break;
     }

     si.fMask = SIF_POS;
     SetScrollInfo(SB_VERT, &si, TRUE);
     GetScrollInfo(SB_VERT, &si);
     
     m_iTopViewStart = si.nPos;

     m_Transform.SetPointOffset(-double(m_iLeftViewStart), -double(m_iTopViewStart));
     InvalidateRect(NULL);
     
     bHandled = true;
     return 0;
}

LRESULT CTopoViewerWindow::OnIsSaved(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    return (LRESULT) m_pEditor->IsSaved();
}

LRESULT CTopoViewerWindow::OnNewTopology(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    m_pEditor->NewTopology();

    return 0;
}

LRESULT CTopoViewerWindow::OnShowTopology(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    IMFTopology* pTopology = (IMFTopology*) lParam;

    HRESULT hr = m_pEditor->ShowTopology(pTopology, L"");

    if(FAILED(hr))
    {
        return 1;
    }

   return 0;
}

LRESULT CTopoViewerWindow::OnMergeTopology(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    IMFTopology* pTopology = (IMFTopology*) lParam;

    HRESULT hr = m_pEditor->MergeTopology(pTopology);

    if(FAILED(hr))
    {
        return 1;
    }

   return 0;
}

LRESULT CTopoViewerWindow::OnLoadTopology(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    LPWSTR szFileName = (LPWSTR) lParam;

    return (LRESULT) m_pEditor->LoadTopology(szFileName);
}

LRESULT CTopoViewerWindow::OnSaveTopology(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    LPWSTR szFileName = (LPWSTR) lParam;

    return (LRESULT) m_pEditor->SaveTopology(szFileName);
}

LRESULT CTopoViewerWindow::OnGetTopology(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    IMFTopology** ppTopology = (IMFTopology**) lParam;
    BOOL* pfIsProtected = (BOOL*) wParam;

    return (LRESULT) m_pEditor->GetTopology(ppTopology, pfIsProtected);
}

LRESULT CTopoViewerWindow::OnAddSource(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    LPWSTR szSourceURL = (LPWSTR) lParam;

    HRESULT hr = S_OK;
    CTedSourceNode* pSourceNode;
    CTedNodeCreator* pNodeCreator = CTedNodeCreator::GetSingleton();
    IFC( pNodeCreator->CreateSource(szSourceURL, NULL, &pSourceNode) );
    IFC( m_pEditor->AddNode(pSourceNode) );

Cleanup:    
    return (LRESULT) hr;
}

LRESULT CTopoViewerWindow::OnAddSAR(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    HRESULT hr = S_OK;
    CTedAudioOutputNode* pSARNode;
    CTedNodeCreator* pNodeCreator = CTedNodeCreator::GetSingleton();
    IFC( pNodeCreator->CreateSAR(&pSARNode) );
    IFC( m_pEditor->AddNode(pSARNode) );

Cleanup:
    return (LRESULT) hr;
}

LRESULT CTopoViewerWindow::OnAddEVR(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    HRESULT hr = S_OK;
    CTedVideoOutputNode* pEVRNode;
    CTedNodeCreator* pNodeCreator = CTedNodeCreator::GetSingleton();
    IFC( pNodeCreator->CreateEVR(NULL, &pEVRNode) );
    IFC( m_pEditor->AddNode(pEVRNode) );

Cleanup:
    return (LRESULT) hr;
}

LRESULT CTopoViewerWindow::OnAddTransform(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    LPWSTR szTransformName = (LPWSTR) lParam;
    GUID gidTransformID = *((GUID*) wParam);

    HRESULT hr = S_OK;
    CTedTransformNode* pTransformNode;
    CTedNodeCreator* pNodeCreator = CTedNodeCreator::GetSingleton();
    IFC( pNodeCreator->CreateTransform(gidTransformID, szTransformName, &pTransformNode) );
    IFC( m_pEditor->AddNode(pTransformNode) );
    
Cleanup:
    return (LRESULT) hr;
}

LRESULT CTopoViewerWindow::OnAddCustomSink(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    GUID gidSinkID = *((GUID*) lParam);

    HRESULT hr = S_OK;
    CTedCustomOutputNode* pSinkNode;
    CTedNodeCreator* pNodeCreator = CTedNodeCreator::GetSingleton();
    IFC( pNodeCreator->CreateCustomSink(gidSinkID, &pSinkNode) );
    IFC( m_pEditor->AddNode(pSinkNode) );
    
Cleanup:
    return (LRESULT) hr;
}

LRESULT CTopoViewerWindow::OnAddTee(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    HRESULT hr = S_OK;
    CTedTeeNode* pTeeNode;
    CTedNodeCreator* pNodeCreator = CTedNodeCreator::GetSingleton();
    IFC( pNodeCreator->CreateTee(&pTeeNode) );
    IFC( m_pEditor->AddNode(pTeeNode) );
    
Cleanup:
    return (LRESULT) hr;
}

LRESULT CTopoViewerWindow::OnDeleteSelectedNode(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    HandleDelete();

    return 0;
}

LRESULT CTopoViewerWindow::OnSpySelectedNode(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    return (LRESULT) SpySelectedVisual();
}

CVisualObject* CTopoViewerWindow::GetSelectedVisual()
{
    return m_pSelected;
}

void CTopoViewerWindow::HandleDelete() 
{
    if(!m_pSelected)
    {
        goto Cleanup;
    }

    if(m_pSelected->GetContainer())
    {
        m_pTree->RemoveVisual(m_pSelected->GetContainer());
    }
    else if(m_pSelected->Type() == CVisualObject::PIN) 
    {
        CVisualPin* pin = static_cast<CVisualPin*>(m_pSelected);
        CVisualConnector* connector = pin->GetConnector();
        m_pTree->RemoveVisual(connector);

        pin->Select(false);
    }
    else if(m_pSelected->Type() == CVisualObject::CONNECTOR)
    {
        m_pTree->RemoveVisual(m_pSelected);
    }
    else 
    {
        m_pTree->RemoveVisual(m_pSelected);
    }

    m_pSelected = NULL;
    m_pFocus = NULL;

    assert( m_hWnd != NULL);
    InvalidateRect(NULL);
Cleanup:
    ;
}

void CTopoViewerWindow::Unselect()
{
    m_pSelected = NULL;
}

void CTopoViewerWindow::ResizeScrollBars()
{
    RECT rect;
    GetClientRect(&rect);

    UINT32 iScrollWidth = 0;
    if(m_iTopologyWidth > UINT32(rect.right - rect.left))
    {
        iScrollWidth = m_iTopologyWidth - (rect.right - rect.left) + 10;
    }
    
   // Set the horizontal scrolling range and page size. 
    SCROLLINFO si;
    si.cbSize = sizeof(si); 
    si.fMask  = SIF_RANGE | SIF_PAGE; 
    si.nMin   = 0; 
    si.nMax   = iScrollWidth; 
    si.nPage  = 20; 
    SetScrollInfo(SB_HORZ, &si, TRUE); 

    UINT32 iScrollHeight = 0;
    if(m_iTopologyHeight > UINT32(rect.bottom - rect.top))
    {
        iScrollHeight = m_iTopologyHeight - (rect.bottom - rect.top) + 10;
    }
    // Set the horizontal scrolling range and page size. 
    si.cbSize = sizeof(si); 
    si.fMask  = SIF_RANGE | SIF_PAGE; 
    si.nMin   = 0; 
    si.nMax   = iScrollHeight; 
    si.nPage  = 20; 
    SetScrollInfo(SB_VERT, &si, TRUE); 
}

void CTopoViewerWindow::NotifyNewVisuals()
{
    m_pTree->GetMaxExtent(m_iTopologyWidth, m_iTopologyHeight);

    RECT rect;
    GetClientRect(&rect);

    if(m_iTopologyWidth > UINT32(rect.right - rect.left) || m_iTopologyHeight > UINT32(rect.bottom - rect.top))
    {
        ResizeScrollBars();
    }
}

void CTopoViewerWindow::OnFinalMessage(HWND hwnd)
{
    if(m_pController) m_pController->Release();
}

void CTopoViewerWindow::SetEditable(BOOL fEditable)
{
    m_fEditable = fEditable;
}

HRESULT CTopoViewerWindow::SpySelectedVisual()
{
    if(m_pSelected != NULL && (m_pSelected->Type() == CVisualObject::NODE || m_pSelected->Type() == CVisualObject::CONTAINER))
    {
        return m_pEditor->SpyNodeWithVisual(m_pSelected);
    }

    return S_OK;
}

