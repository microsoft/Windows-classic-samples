// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
#pragma once

class CVisualObject;
class CVisualConnector;
class CVisualComponent;
class CVisualTree;
class CCommandHandler;
class CTedTopologyEditor;
class CMoveComponentHandler;
class CConnectPinHandler;
class CVisualPin;

#include "tedvis.h"
#include "tededit.h"
#include "tedutilinc.h"
#include "tedutil.h"

HRESULT TEDCreateTopoViewerWindow(LPCWSTR szTitle, DWORD dwStyle, RECT clientRect, HWND hWndParent, HWND* phWnd);

///////////////////////////////////////////////////////////////////////////////
// 
class CTopoViewerWindow
    : public CWindowImpl<CTopoViewerWindow>
{
public:
    CTopoViewerWindow(HRESULT& hr);
    ~CTopoViewerWindow();

    void Init(ITedTopoView* pController, CTedTopologyEditor* pEditor);

    CVisualObject* GetSelectedVisual();
    void HandleDelete();
    void ClearView();
    void Unselect();
    void NotifyNewVisuals();
    void OnFinalMessage(HWND hwnd);
    void SetEditable(BOOL fEditable);

    CVisualTree * PTree() { return m_pTree; }

    HRESULT SpySelectedVisual();

    DECLARE_WND_CLASS_EX(L"TedTopoViewer", CS_GLOBALCLASS | CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, COLOR_BACKGROUND);
    
protected:    
        
    LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnEraseBkgnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnLButtonDoubleClick(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnHScroll(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnVScroll(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnIsSaved(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnNewTopology(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnShowTopology(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnMergeTopology(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnLoadTopology(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnSaveTopology(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnGetTopology(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnAddSource(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnAddSAR(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnAddEVR(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnAddTransform(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnAddTee(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnAddCustomSink(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnDeleteSelectedNode(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnSpySelectedNode(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    
    BEGIN_MSG_MAP(CVisualView)
        MESSAGE_HANDLER(WM_CREATE, OnCreate)
        MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
        MESSAGE_HANDLER(WM_SIZE, OnSize)
        MESSAGE_HANDLER(WM_PAINT, OnPaint)
        MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
        MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
        MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
        MESSAGE_HANDLER(WM_LBUTTONDBLCLK, OnLButtonDoubleClick)
        MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
        MESSAGE_HANDLER(WM_HSCROLL, OnHScroll)
        MESSAGE_HANDLER(WM_VSCROLL, OnVScroll)
        MESSAGE_HANDLER(WM_SHOWTOPOLOGY, OnShowTopology)
        MESSAGE_HANDLER(WM_ISSAVED, OnIsSaved)
        MESSAGE_HANDLER(WM_NEWTOPOLOGY, OnNewTopology)
        MESSAGE_HANDLER(WM_SHOWTOPOLOGY, OnShowTopology)
        MESSAGE_HANDLER(WM_MERGETOPOLOGY, OnMergeTopology)
        MESSAGE_HANDLER(WM_LOADTOPOLOGY, OnLoadTopology)
        MESSAGE_HANDLER(WM_SAVETOPOLOGY, OnSaveTopology)
        MESSAGE_HANDLER(WM_GETTOPOLOGY, OnGetTopology)
        MESSAGE_HANDLER(WM_ADDSOURCE, OnAddSource)
        MESSAGE_HANDLER(WM_ADDSAR, OnAddSAR)
        MESSAGE_HANDLER(WM_ADDEVR, OnAddEVR)
        MESSAGE_HANDLER(WM_ADDTRANSFORM, OnAddTransform)
        MESSAGE_HANDLER(WM_ADDTEE, OnAddTee)
        MESSAGE_HANDLER(WM_ADDCUSTOMSINK, OnAddCustomSink)
        MESSAGE_HANDLER(WM_DELETESELECTEDNODE, OnDeleteSelectedNode)
        MESSAGE_HANDLER(WM_SPYSELECTEDNODE, OnSpySelectedNode)
    END_MSG_MAP()


    void ResizeScrollBars();
    
private:
    ITedTopoView* m_pController;
    CTedTopologyEditor* m_pEditor;
    CVisualTree * m_pTree;

    CVisualObject * m_pFocus;
    CVisualObject * m_pSelected;
    
    CVisualCoordinateTransform m_Transform;

    UINT32 m_iLeftViewStart;
    UINT32 m_iTopViewStart;
    UINT32 m_iTopologyWidth;
    UINT32 m_iTopologyHeight;

    BOOL m_fEditable;
};
