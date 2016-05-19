// MainFrm.h : interface of the CMainFrame class
//
// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINFRM_H__2BCEEF89_CE4C_11D1_AD5B_0060083E86DF__INCLUDED_)
#define AFX_MAINFRM_H__2BCEEF89_CE4C_11D1_AD5B_0060083E86DF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "edittoolbar.h"
#include "SourceView.h"	// Added by ClassView
#include "Ribbon/ribbon.h"

class CSourceView;
class CHTMLEdView;

class CMainFrame : public CFrameWnd
{
    
protected: // create from serialization only
    CMainFrame();
    DECLARE_DYNCREATE(CMainFrame)

// Attributes
public:
    CComboBox* GetFontNameCombo();
    CComboBox* GetFontSizeCombo();
    void OnBlockFmtDropdown( NMHDR * pNotifyStruct, LRESULT * result );
// Operations
public:

// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CMainFrame)
    public:
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    protected:
    virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
    //}}AFX_VIRTUAL

// Implementation
public:
    CView* GetSourceView();
    CView* GetWebView();    
	void SwapView();
    HRESULT SwapView(int nCmdId);
    virtual ~CMainFrame();
    CEditToolbar m_wndEditBar;
    BOOL CreateEditBar();
    DWORD GetCurrentView();
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

public:  // control bar embedded members
    CStatusBar  m_wndStatusBar;
    CRibbonBar  m_RibbonBar;

    IUnknown*    m_pUIFramework;

// Generated message map functions
protected:
    CSourceView *m_pSrcView;

    //{{AFX_MSG(CMainFrame)
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnDestroy();
    afx_msg void OnFontNameChange();
    afx_msg void OnFontSizeChange();
    afx_msg void OnViewEditingtoolbar();
    afx_msg void OnUpdateViewEditingtoolbar(CCmdUI* pCmdUI);
    afx_msg void OnViewSource();
    afx_msg void OnViewWeb();
    afx_msg void OnUpdateViewWeb(CCmdUI* pCmdUI);
    afx_msg void OnUpdateViewSource(CCmdUI* pCmdUI);
    afx_msg void OnUpdateFontSize(CCmdUI* pCmdUI);
    afx_msg void OnUpdateFontName(CCmdUI* pCmdUI);
    afx_msg void OnUpdatePane (CCmdUI *pCmdUI);
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
private:
//	CView *m_pWebView;
    CHTMLEdView *m_pWebView;
    DWORD m_dwCurrentView;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRM_H__2BCEEF89_CE4C_11D1_AD5B_0060083E86DF__INCLUDED_)
