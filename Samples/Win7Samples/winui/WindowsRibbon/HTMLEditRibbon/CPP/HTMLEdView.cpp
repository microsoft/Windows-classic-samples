// HTMLEdView.cpp : implementation of the CHTMLEdView class
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

#include "HTMLEdDoc.h"
#include "htmledview.h"
#include "mainfrm.h"
#include <afxpriv.h>

#include "Ribbon/ribbon.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CHTMLEdView

//Arrays for context menu

IMPLEMENT_DYNCREATE(CHTMLEdView, CHtmlEditView)
BEGIN_DHTMLEDITING_CMDMAP(CHTMLEdView)
    DHTMLEDITING_CMD_ENTRY(ID_EDIT_CUT, IDM_CUT)
    DHTMLEDITING_CMD_ENTRY(ID_EDIT_COPY, IDM_COPY)
    DHTMLEDITING_CMD_ENTRY(ID_EDIT_PASTE, IDM_PASTE)
    DHTMLEDITING_CMD_ENTRY(ID_EDIT_UNDO, IDM_UNDO)
    DHTMLEDITING_CMD_ENTRY(ID_EDIT_SELECT_ALL, IDM_SELECTALL)
    DHTMLEDITING_CMD_ENTRY(ID_EDIT_FONT, IDM_FONT)
    DHTMLEDITING_CMD_ENTRY(ID_BUTTON_INDENT, IDM_INDENT)
    DHTMLEDITING_CMD_ENTRY(ID_BUTTON_OUTDENT, IDM_OUTDENT)
    DHTMLEDITING_CMD_ENTRY(ID_BUTTON_IMAGE, IDM_IMAGE)
    DHTMLEDITING_CMD_ENTRY_FUNC(ID_FORMAT_ABSOLUTEPOSITIONELEMENT, IDM_ABSOLUTE_POSITION, &CHTMLEdView::SetAbsPos)
    DHTMLEDITING_CMD_ENTRY_FUNC(ID_FORMAT_STATICELEMENT, IDM_ABSOLUTE_POSITION, &CHTMLEdView::SetStaticPos)
    DHTMLEDITING_CMD_ENTRY_FUNC(ID_BUTTON_ABSOLUTE, IDM_ABSOLUTE_POSITION, &CHTMLEdView::SetAbsPos)
    DHTMLEDITING_CMD_ENTRY_FUNC(ID_BUTTON_STATIC, IDM_ABSOLUTE_POSITION, &CHTMLEdView::SetStaticPos)
    DHTMLEDITING_CMD_ENTRY(ID_BUTTON_HYPERLINK, IDM_HYPERLINK)
    DHTMLEDITING_CMD_ENTRY_TYPE(ID_BUTTON_BOLD, IDM_BOLD, AFX_UI_ELEMTYPE_CHECBOX)
    DHTMLEDITING_CMD_ENTRY_TYPE(ID_BUTTON_ITALIC, IDM_ITALIC, AFX_UI_ELEMTYPE_CHECBOX)
    DHTMLEDITING_CMD_ENTRY_TYPE(ID_BUTTON_UNDERLINE, IDM_UNDERLINE, AFX_UI_ELEMTYPE_CHECBOX)
    DHTMLEDITING_CMD_ENTRY_TYPE(ID_BUTTON_BULLETLIST, IDM_UNORDERLIST, AFX_UI_ELEMTYPE_CHECBOX)
    DHTMLEDITING_CMD_ENTRY_TYPE(ID_BUTTON_NUMBERLIST, IDM_ORDERLIST, AFX_UI_ELEMTYPE_CHECBOX)
    DHTMLEDITING_CMD_ENTRY_TYPE(ID_BUTTON_LEFTJUSTIFY, IDM_JUSTIFYLEFT, AFX_UI_ELEMTYPE_CHECBOX)
    DHTMLEDITING_CMD_ENTRY_TYPE(ID_BUTTON_CENTERJUSTIFY, IDM_JUSTIFYCENTER, AFX_UI_ELEMTYPE_CHECBOX)
    DHTMLEDITING_CMD_ENTRY_TYPE(ID_BUTTON_RIGHTJUSTIFY, IDM_JUSTIFYRIGHT, AFX_UI_ELEMTYPE_CHECBOX)

END_DHTMLEDITING_CMDMAP()

BEGIN_MESSAGE_MAP(CHTMLEdView, CHtmlEditView)
    ON_UPDATE_COMMAND_UI(ID_BUTTON_BLOCKFMT, OnUpdateBlockFmt)
    ON_UPDATE_COMMAND_UI(ID_BUTTON_COLOR, OnUpdateColor)
    ON_NOTIFY(TBN_DROPDOWN, AFX_IDW_TOOLBAR , OnEditBarNotify)
    ON_COMMAND_RANGE(FORMATBASE,FORMATBASE+100,OnFormatPopup)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHTMLEdView construction/destruction
void CHTMLEdView::OnUpdateBlockFmt(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(QueryStatus(IDM_BLOCKFMT) & OLECMDF_ENABLED);
}

void CHTMLEdView::OnUpdateColor(CCmdUI *pCmdUI)
{
    if (QueryStatus(IDM_FORECOLOR) & OLECMDF_ENABLED)
        pCmdUI->Enable();
}

void CHTMLEdView::SetAbsPos(UINT)
{
    SetAbsolutePosition(true);
}

void CHTMLEdView::SetStaticPos(UINT)
{
    SetAbsolutePosition(false);
}

CHTMLEdView::CHTMLEdView():m_bUserMode(true),
    m_pClrMenu(NULL),
    m_pFmtMenu(NULL),
    m_pMainMenu(NULL)
{
}

CHTMLEdView::~CHTMLEdView()
{
    if(m_pFmtMenu) delete m_pFmtMenu;
    if(m_pClrMenu) delete m_pClrMenu;
    if(m_pMainMenu) 
    {
        m_pMainMenu->DestroyMenu();
        delete m_pMainMenu;
    }
}

/////////////////////////////////////////////////////////////////////////////
// CHTMLEdView printing



/////////////////////////////////////////////////////////////////////////////
// CHTMLEdView diagnostics

#ifdef _DEBUG
void CHTMLEdView::AssertValid() const
{
    CHtmlEditView::AssertValid();
}

void CHTMLEdView::Dump(CDumpContext& dc) const
{
    CHtmlEditView::Dump(dc);
}

CHTMLEdDoc* CHTMLEdView::GetDocument() // non-debug version is inline
{
    ENSURE(m_pDocument->IsKindOf(RUNTIME_CLASS(CHTMLEdDoc)));
    return (CHTMLEdDoc*)m_pDocument;
}
#endif //_DEBUG

//Events from the triedit control
void CHTMLEdView::OnDocumentComplete()
{
    CStringArray sa;
    GetBlockFormatNames(sa);
    //When in Source View, calling UpdateView to update the web view, causes
    //OnDocumentComplete to be fired. We check if the document is waiting for
    //this event to save itself
    if(((CHTMLEdDoc*)m_pDocument)->m_bDoSaveOnDocCom)
    {
        ((CHTMLEdDoc*)m_pDocument)->m_bDoSaveOnDocCom = FALSE;
        ((CHTMLEdDoc*)m_pDocument)->SaveMyDocument();

        //Since OnDocumentComplete is asynchronous, we have postponed OnNewDocument
        //till OnDocumentComplete is done, so that the old document gets saved before
        //the new one is loaded
        if(((CHTMLEdDoc*)m_pDocument)->m_bCallNewDocument)
        {
            ((CHTMLEdDoc*)m_pDocument)->m_bCallNewDocument = FALSE;
            ((CHTMLEdDoc*)m_pDocument)->OnNewDocument();
        }

        //Same logic as above for OnNewDocument
        if(((CHTMLEdDoc*)m_pDocument)->m_bCallCloseDocument)
        {
            ((CHTMLEdDoc*)m_pDocument)->m_bCallCloseDocument = FALSE;
            ((CHTMLEdDoc*)m_pDocument)->OnCloseDocument();
        }
    }
}
HRESULT CHTMLEdView::OnUpdateUI()
{
    long id = GetDlgCtrlID();
    if(id==AFX_IDW_PANE_LAST) //we change the ID if we hide this window
        return S_OK;
    if(S_OK == GetIsDirty())
        GetDocument()->SetModifiedFlag();

    AfxGetMainWnd()->PostMessage(WM_KICKIDLE);
    return RibbonInvalidate(((CMainFrame*)AfxGetMainWnd())->m_pUIFramework);
}

BOOL CHTMLEdView::GetCommandStatus(UINT nCmdID)
{
    BOOL bHasFunc;
    UINT uiElemType;
    INT id = GetDHtmlCommandMapping(nCmdID, bHasFunc, uiElemType);
    return id > 0 ? !!(QueryStatus(id) & OLECMDF_ENABLED) : TRUE;
}

HRESULT CHTMLEdView::OnShowContextMenu(DWORD /*dwID*/,
                            LPPOINT ppt,
                            LPUNKNOWN /*pcmdtReserved*/,
                            LPDISPATCH /*pdispReserved*/)
{
    if (!m_pMainMenu)
    {
        m_pMainMenu = new CMenu();
        m_pMainMenu->LoadMenu(IDR_MAINFRAME);
    }

    CMenu *pPopup = m_pMainMenu->GetSubMenu(1);

    // enable/disable items based on the status of the command
    UINT nCount = pPopup->GetMenuItemCount();
    for (UINT i = 0; i<nCount; i++)
    {
        int nID = (int)pPopup->GetMenuItemID(i);
        if (nID != 0 && nID != -1)
        {
            BOOL bHasFunc;
            UINT uiElemType;
            UINT dhtmlCmdID = GetDHtmlCommandMapping(nID, bHasFunc, uiElemType);
            long nStatus = QueryStatus(dhtmlCmdID);
            if (!(nStatus & OLECMDF_ENABLED))
            {
                pPopup->EnableMenuItem(nID, MF_DISABLED|MF_GRAYED);
            }
            else
                pPopup->EnableMenuItem(nID, MF_ENABLED);
        }
    }
    pPopup->TrackPopupMenu(TPM_LEFTALIGN, ppt->x, ppt->y, this);
    return S_OK;
}
void CHTMLEdView::OnContextMenuAction(long itemIndex)
{
    switch(itemIndex)
    {
    case 0:
        CHtmlEditView::Cut();
        break;
    case 1:
        CHtmlEditView::Copy();
        break;
    case 2:
        CHtmlEditView::Paste();
        break;
    case 4:
        CHtmlEditView::SelectAll();
        break;
    }
}

void CHTMLEdView::OnInitialUpdate() 
{
    CString strCmdLine(AfxGetApp()->m_lpCmdLine);
    CHTMLEdDoc * pDoc = NULL;
    if(strCmdLine != _T(""))
    {
        pDoc =(CHTMLEdDoc * )GetDocument();
        ENSURE(pDoc);
        pDoc->OnOpenDocument((LPCTSTR)strCmdLine);
    }	
    CHtmlEditView::OnInitialUpdate();
}

void CHTMLEdView::UpdateFontCombos()
{	
    CString szResult,szCurFmt;
    int nSel;
    short nCurSize;

    CMainFrame *pFrame = (CMainFrame*)GetParentFrame();
    ASSERT_VALID(pFrame);


    //Set font face
    CComboBox *pCombo = (CComboBox*)pFrame->GetFontNameCombo();
    ASSERT_VALID(pCombo);
    pCombo->GetWindowText(szCurFmt);
    GetFontFace(szResult);
    //only update the combo it isn't in the dropped state
    //and the font name has changed
    if(!pCombo->GetDroppedState() && szCurFmt!=szResult)
    {

        nSel = pCombo->FindString(0,szResult);
        if(nSel != CB_ERR)
            pCombo->SetCurSel(nSel);
    }

    //Set font size
    pCombo = (CComboBox*)pFrame->GetFontSizeCombo();
    ASSERT_VALID(pCombo);
    short nSize = 0;
    nCurSize = 0;

    GetFontSize(nSize);
    //only update the size combo if it is not in the
    //dropped down state.
    if(!pCombo->GetDroppedState())
    {
        szResult.Empty();
        _itot_s(nSize,szResult.GetBuffer(5), 5, 10);
        nSel = pCombo->FindString(0,(LPCTSTR) szResult);
        szResult.ReleaseBuffer();
        if(nSel != CB_ERR)
            pCombo->SetCurSel(nSel);
    }
}

void CHTMLEdView::OnEditBarNotify(NMHDR* pNMHDR, LRESULT* /*pResult*/)
{

    TBNOTIFY* pTBNotify = (TBNOTIFY*) pNMHDR;

    switch(pTBNotify->iItem)
    {
    case ID_BUTTON_BLOCKFMT:
        PopBlockFmtMenu();
        break;

    case ID_BUTTON_COLOR:
        PopColorMenu();
        break;

    default:
        break;
    }
}

void CHTMLEdView::PopBlockFmtMenu()
{
    //re-create the popup menu every time through here
    //in case block formats change.
    if(!m_pFmtMenu)
    {
        m_pFmtMenu = new CFormatMenu;
        if(m_pFmtMenu)
        {
            if(!m_pFmtMenu->CreatePopupMenu())
                return;
        }
        else
            return;

    }
    else
    {
        m_pFmtMenu->DestroyMenu();
        if(!m_pFmtMenu->CreatePopupMenu())
            return;
    }
    
    m_pFmtMenu->ClearStringArray();
    CStringArray &ar = m_pFmtMenu->GetStringArray();
    CString curFmt;

    if(!SUCCEEDED(GetBlockFormatNames(ar)) || !SUCCEEDED(GetBlockFormat(curFmt)) )
        return;
    int nFlags;
    int arSz = (int)ar.GetSize();
    for(int i = 0; i < arSz; i++)
    {
        nFlags = MF_STRING|MF_ENABLED;
        if(ar[i] == curFmt)
            nFlags |= MF_CHECKED;
        m_pFmtMenu->AppendMenu(nFlags,FORMATBASE+i,ar[i]);
    }

    //get the rect for menu
    CMainFrame *pFrame = (CMainFrame*)GetParentFrame();
    ASSERT_VALID(pFrame);
    CRect rc;
    pFrame->m_wndEditBar.GetItemRect(2,rc);
    pFrame->m_wndEditBar.ClientToScreen(rc);

    m_pFmtMenu->TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON,
                                rc.left+3,rc.bottom,this);

}			
void CHTMLEdView::OnFormatPopup(UINT nCommandID)
{
    //format popup menu was hit
    //calculate the index into the format menu's string array
    int nIndex = nCommandID-FORMATBASE;
    CStringArray &sa = m_pFmtMenu->GetStringArray();
    SetBlockFormat(sa[nIndex]);
}


UINT_PTR CALLBACK CdlgHook(  HWND hdlg,UINT uiMsg,WPARAM /*wParam*/, LPARAM lParam)
{
    if(uiMsg == WM_INITDIALOG)
    {
        CHOOSECOLOR *pcc = (CHOOSECOLOR*)lParam;
        CRect *rc = (CRect*)pcc->lCustData;
        if(rc)
            SetWindowPos(hdlg,HWND_TOP,rc->right,rc->bottom,
            0,0,SWP_NOZORDER|SWP_NOSIZE);
        SetWindowText(hdlg, _T("Choose a Foreground Color"));
        delete rc;
    }
    return 0;
}
void CHTMLEdView::PopColorMenu()
{
    //get the rect for menu
    CMainFrame *pFrame = (CMainFrame*)GetParentFrame();
    ASSERT_VALID(pFrame);
    CRect *prc = new CRect;
    if(!prc) return;

    pFrame->m_wndEditBar.GetItemRect(6,prc);
    pFrame->m_wndEditBar.ClientToScreen(prc);

    CColorDialog dlg;
    dlg.m_cc.Flags |= CC_ENABLEHOOK;
    dlg.m_cc.lpfnHook = CdlgHook;
    dlg.m_cc.lCustData = (LONG_PTR)prc;

    if(dlg.DoModal()==IDOK)
    {
        CString szColor;
        COLORREF cr = dlg.GetColor();

        //change the COLORREF into an RGB.
        szColor.Format(_T("%.2x%.2x%.2x"),GetRValue(cr),GetGValue(cr),GetBValue(cr));
        SetForeColor(szColor);
    }
}

void CHTMLEdView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView) 
{

    if(bActivate && pActivateView==this && pDeactiveView != this)
    {
        //we're being activated get the HTML from the WebView
        UpdateView();
    }
    
    CHtmlEditView::OnActivateView(bActivate, pActivateView, pDeactiveView);
}

void CHTMLEdView::UpdateView()
{
    CMainFrame *pFrame = (CMainFrame*)AfxGetMainWnd();
    ASSERT_VALID(pFrame);

    CSourceView* pSrcView = (CSourceView*)pFrame->GetSourceView();
    ASSERT_VALID(pSrcView);
    CString szHTML;
    pSrcView->GetWindowText(szHTML);
    if(szHTML.GetLength() > 0)
        SetDocumentHTML(szHTML);
}