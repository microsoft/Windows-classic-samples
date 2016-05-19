// MainFrm.cpp : implementation of the CMainFrame class
//
// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#include "stdafx.h"
#include "HTMLEdit.h"

#include "MainFrm.h"
#include "htmleddoc.h"
#include "htmlEdView.h"
#include "sourceview.h"

#include "Ribbon\Ribbon.h"

#include <uiribbon.h>

CComModule _Module;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
    ON_COMMAND_EX(CG_ID_VIEW_FONTDLGBAR, OnBarCheck)
    ON_UPDATE_COMMAND_UI(CG_ID_VIEW_FONTDLGBAR, OnUpdateControlBarMenu)
    //{{AFX_MSG_MAP(CMainFrame)
    ON_WM_CREATE()
    ON_WM_DESTROY()
    ON_CBN_SELCHANGE(ID_FMTBAR_FONTNAME, OnFontNameChange)
    ON_CBN_SELCHANGE(ID_FMTBAR_FONTSIZE, OnFontSizeChange)
    ON_COMMAND(ID_VIEW_EDITINGTOOLBAR, OnViewEditingtoolbar)
    ON_UPDATE_COMMAND_UI(ID_VIEW_EDITINGTOOLBAR, OnUpdateViewEditingtoolbar)
    ON_COMMAND(ID_VIEW_SOURCE, OnViewSource)
    ON_COMMAND(ID_VIEW_WEB, OnViewWeb)
    ON_UPDATE_COMMAND_UI(ID_VIEW_WEB, OnUpdateViewWeb)
    ON_UPDATE_COMMAND_UI(ID_VIEW_SOURCE, OnUpdateViewSource)
    ON_UPDATE_COMMAND_UI(ID_FMTBAR_FONTSIZE, OnUpdateFontSize)
    ON_UPDATE_COMMAND_UI(ID_FMTBAR_FONTNAME, OnUpdateFontName)
    ON_UPDATE_COMMAND_UI (ID_INDICATOR_VIEW, OnUpdatePane)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()
void CMainFrame::OnBlockFmtDropdown( NMHDR * /*pNotifyStruct*/, LRESULT * /*result*/ )
{

}
static UINT indicators[] =
{
    ID_SEPARATOR,           // status line indicator
    ID_INDICATOR_VIEW,		// view indicator - Web/Source
    ID_INDICATOR_CAPS,
    ID_INDICATOR_NUM,
    ID_INDICATOR_SCRL,
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame():
    m_pSrcView(NULL),
    m_pWebView(NULL),
    m_dwCurrentView(ID_VIEW_WEB),
    m_pUIFramework(NULL)
{
    // TODO: add member initialization code here
}

CMainFrame::~CMainFrame()
{
}
BOOL CreateEditBar();
int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
        return -1;
    
    if (!m_RibbonBar.Create(this, WS_CHILD|WS_DISABLED|WS_VISIBLE|CBRS_TOP|CBRS_HIDE_INPLACE,0))
    {
        TRACE0("Failed to create toolbar\n");
        return -1;      // fail to create
    }

    if (!m_wndStatusBar.Create(this) ||
        !m_wndStatusBar.SetIndicators(indicators,
          sizeof(indicators)/sizeof(UINT)))
    {
        TRACE0("Failed to create status bar\n");
        return -1;      // fail to create
    }

    HRESULT hr;
    // Ribbon initialization.
    hr = InitRibbon(this, &m_pUIFramework);
    if (FAILED(hr))
    {
        return -1;
    }

    hr = SetModes(m_pUIFramework, UI_MAKEAPPMODE(0)|UI_MAKEAPPMODE(1));
    if (FAILED(hr))
    {
        return -1;
    }

    return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
    if( !CFrameWnd::PreCreateWindow(cs) )
        return FALSE;
    // TODO: Modify the Window class or styles here by modifying
    //  the CREATESTRUCT cs

    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
    CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
    CFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers
INT CALLBACK NEnumFontNameProc(LOGFONT *plf, TEXTMETRIC* /*ptm*/, INT /*nFontType*/, LPARAM lParam)
{
    CComboBox* comboBox = (CComboBox*) lParam;

    comboBox->AddString(plf->lfFaceName);

    return TRUE;
}
BOOL CMainFrame::CreateEditBar()
{

    CRect rect;
    const int nDropHeight = 200;

static int nFontSizes[] = 
    {1, 2, 3, 4, 5, 6, 7};


    if (!m_wndEditBar.Create(this) ||
        !m_wndEditBar.LoadToolBar(ID_TOOLBAR_EDITING))
    {
        TRACE0("Failed to create stylebar\n");
        return FALSE;       // fail to create
    }

    CToolBarCtrl& tbCtrl = m_wndEditBar.GetToolBarCtrl();
    tbCtrl.ModifyStyle(0, TBSTYLE_FLAT);

    m_wndEditBar.SetBarStyle(m_wndEditBar.GetBarStyle() |
        CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);

    // Create the font name combo box
    m_wndEditBar.SetButtonInfo(0, ID_FMTBAR_FONTNAME, TBBS_SEPARATOR, 150);

    // Design guide advises 12 pixel gap between combos and buttons
    m_wndEditBar.GetItemRect(0, &rect);
    rect.top = 3;
    rect.bottom = rect.top + nDropHeight;
    if (!m_wndEditBar.m_fontNameCombo.Create(
            CBS_DROPDOWN|WS_VSCROLL|CBS_SORT |WS_VISIBLE|WS_TABSTOP,
            rect, &m_wndEditBar, ID_FMTBAR_FONTNAME))
    {
        TRACE0("Failed to create combo-box\n");
        return FALSE;
    }

    //  Fill the combo box
    ::EnumFontFamilies(GetDC()->m_hDC, (LPTSTR) NULL, (FONTENUMPROC)NEnumFontNameProc, (LPARAM)&(m_wndEditBar.m_fontNameCombo));

    // Create the fontsize combo box
    m_wndEditBar.SetButtonInfo(1, ID_FMTBAR_FONTSIZE, TBBS_SEPARATOR, 50);

    m_wndEditBar.GetItemRect(1, &rect);
    rect.top = 3;
    rect.bottom = rect.top + nDropHeight;
    rect.left += 5;
    if (!m_wndEditBar.m_fontSizeCombo.Create(
            CBS_DROPDOWNLIST|WS_VISIBLE|WS_TABSTOP,
            rect, &m_wndEditBar, ID_FMTBAR_FONTSIZE))
    {
        TRACE0("Failed to create combo-box\n");
        return FALSE;
    }

    CString str;
    for (int i = 0; i < sizeof(nFontSizes)/sizeof(int); i++)
    {
        str.Format(_T("%d"), nFontSizes[i]);
        m_wndEditBar.m_fontSizeCombo.AddString(str);
    }

    // set dropdown style on tag format button and font color button
    m_wndEditBar.SetButtonStyle(3, TBSTYLE_DROPDOWN);
    m_wndEditBar.SetButtonStyle(8, TBSTYLE_DROPDOWN);

    m_wndEditBar.m_font.Attach(GetStockObject(DEFAULT_GUI_FONT));
    m_wndEditBar.m_fontNameCombo.SetFont(&m_wndEditBar.m_font);
    m_wndEditBar.m_fontSizeCombo.SetFont(&m_wndEditBar.m_font);

    m_wndEditBar.m_fontNameCombo.SetCurSel(-1);
    m_wndEditBar.m_fontSizeCombo.SetCurSel(-1);

    return TRUE;

}
CComboBox* CMainFrame::GetFontNameCombo() 
{
    return &m_wndEditBar.m_fontNameCombo;
}
CComboBox* CMainFrame::GetFontSizeCombo() 
{
    return &m_wndEditBar.m_fontSizeCombo;
}
void CMainFrame::OnFontNameChange() 
{
    CString szText;
    m_wndEditBar.m_fontNameCombo.GetLBText(
        m_wndEditBar.m_fontNameCombo.GetCurSel(),szText);

    CHTMLEdView *pView = (CHTMLEdView*)GetActiveView();
    ASSERT_VALID(pView);
    pView->SetFontFace(szText);

}

void CMainFrame::OnFontSizeChange() 
{
    CString szText;
    m_wndEditBar.m_fontSizeCombo.GetLBText(
        m_wndEditBar.m_fontSizeCombo.GetCurSel(),szText);

    CHTMLEdView *pView = (CHTMLEdView*)GetActiveView();
    ASSERT_VALID(pView);

    pView->SetFontSize((short)_ttoi(szText));
}

void CMainFrame::OnViewEditingtoolbar() 
{
    CControlBar *pBar = DYNAMIC_DOWNCAST(CControlBar,&m_wndEditBar);
    if(pBar)
    {
        ShowControlBar(pBar, (pBar->GetStyle() & WS_VISIBLE) == 0, FALSE);
    }

}

void CMainFrame::OnUpdateViewEditingtoolbar(CCmdUI* pCmdUI) 
{
    CControlBar *pBar = DYNAMIC_DOWNCAST(CControlBar,&m_wndEditBar);
    if (pBar != NULL)
    {
        pCmdUI->SetCheck((pBar->GetStyle() & WS_VISIBLE) != 0);
        return;
    }
    
}

BOOL CMainFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext) 
{	
    CDocument *pDoc=NULL;
    POSITION pos;
    BOOL bRet = FALSE;
    //create the source view for this document at this time
    CRuntimeClass *prc = RUNTIME_CLASS(CSourceView);
    ENSURE(prc);
    m_pSrcView = (CSourceView*)prc->CreateObject();
    if(m_pSrcView)
        VERIFY(m_pSrcView->Create(NULL, NULL, AFX_WS_DEFAULT_VIEW,
        CRect(0,0,0,0), this, AFX_IDW_PANE_LAST, NULL));


    //create the doc-template view and save off a pointer to it
    bRet = CFrameWnd::OnCreateClient(lpcs, pContext);
    pDoc = pContext->m_pCurrentDoc;
    if(bRet && pDoc)
    {
        pos = pDoc->GetFirstViewPosition();
        pDoc->AddView(m_pSrcView);
    }
    else
        return FALSE;

    CView *pView = pDoc->GetNextView(pos);
    ASSERT(pView->IsKindOf(RUNTIME_CLASS(CHTMLEdView)));
    if(pView)
    {
        m_pWebView = (CHTMLEdView*) pView;
    }
    else
        return FALSE;

    return bRet;
}

HRESULT CMainFrame::SwapView(int nCmdID)
{
    CDocument *pDoc;

    pDoc = GetActiveDocument();
    if(nCmdID == ID_VIEW_SOURCE)
    {
        //swap the view IDs.
        m_pWebView->SetDlgCtrlID(AFX_IDW_PANE_LAST);
        m_pSrcView->SetDlgCtrlID(AFX_IDW_PANE_FIRST);

        // show/hide the right view.
        m_pWebView->ShowWindow(SW_HIDE);
        m_pSrcView->ShowWindow(SW_SHOW);

        //re-layout everything.
        SetActiveView(m_pSrcView);
        RecalcLayout();

        m_dwCurrentView = ID_VIEW_SOURCE;

        m_wndStatusBar.SetPaneText(1, _T("SRC VIEW"), TRUE);

        return SetModes(m_pUIFramework, UI_MAKEAPPMODE(0));        
    }
    else if(nCmdID == ID_VIEW_WEB)
    {
        //swap the view IDs.
        m_pWebView->SetDlgCtrlID(AFX_IDW_PANE_FIRST);
        m_pSrcView->SetDlgCtrlID(AFX_IDW_PANE_LAST);

        // show/hide the right view.
        m_pWebView->ShowWindow(SW_SHOW);
        m_pSrcView->ShowWindow(SW_HIDE);

        //re-layout everything.
        SetActiveView(m_pWebView);
        RecalcLayout();

        m_dwCurrentView = ID_VIEW_WEB;
        m_wndStatusBar.SetPaneText(1, _T("WEB VIEW"), TRUE);

        return SetModes(m_pUIFramework, UI_MAKEAPPMODE(0)|UI_MAKEAPPMODE(1));        
    }
	return S_OK;

}
void CMainFrame::SwapView()
{
    SwapView(m_dwCurrentView == ID_VIEW_SOURCE ? ID_VIEW_WEB : ID_VIEW_SOURCE);
}


void CMainFrame::OnViewSource() 
{
    SwapView();	
}

void CMainFrame::OnViewWeb() 
{
    SwapView();
}

void CMainFrame::OnUpdateViewWeb(CCmdUI* pCmdUI) 
{
    pCmdUI->SetRadio(m_dwCurrentView==ID_VIEW_WEB);
    
}

void CMainFrame::OnUpdateViewSource(CCmdUI* pCmdUI) 
{
    pCmdUI->SetRadio(m_dwCurrentView==ID_VIEW_SOURCE);
    
}

CView* CMainFrame::GetWebView()
{
    return DYNAMIC_DOWNCAST(CView,m_pWebView);
}

CView* CMainFrame::GetSourceView()
{
    return DYNAMIC_DOWNCAST(CView,m_pSrcView);
}

DWORD CMainFrame::GetCurrentView()
{
    return m_dwCurrentView;
}

void CMainFrame::OnUpdateFontSize(CCmdUI* /*pCmdUI*/)
{
    CComboBox *pCombo = (CComboBox*)GetFontSizeCombo();
    ASSERT_VALID(pCombo);

    //If Source View, don't display the font size combobox
    if(m_dwCurrentView == ID_VIEW_SOURCE)
    {
        pCombo->EnableWindow(FALSE);
        return;
    }
    else
    {
        if(m_pWebView->QueryStatus(IDM_BLOCKFMT) & OLECMDF_ENABLED)
            pCombo->EnableWindow();
        else
            pCombo->EnableWindow(FALSE);
    }
}

void CMainFrame::OnUpdateFontName(CCmdUI* /*pCmdUI*/)
{
    CComboBox *pCombo = (CComboBox*)GetFontNameCombo();
    ASSERT_VALID(pCombo);

    //If Source View, don't display the font name combobox
    if(m_dwCurrentView == ID_VIEW_SOURCE)
    {
        pCombo->EnableWindow(FALSE);
        return;
    }
    else
    {
        if(m_pWebView->QueryStatus(IDM_BLOCKFMT) & OLECMDF_ENABLED)
            pCombo->EnableWindow();
        else
            pCombo->EnableWindow(FALSE);
    }
}

void CMainFrame::OnUpdatePane(CCmdUI *pCmdUI)
{
    pCmdUI->Enable();
}

void CMainFrame::OnDestroy()
{
    if (m_pUIFramework)
    {
        // Destroy the Ribbon and release our COM object.
        DestroyRibbon(m_pUIFramework);
        m_pUIFramework->Release();
        m_pUIFramework = NULL;

	    CFrameWnd::OnDestroy();
    }
}
