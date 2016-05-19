// HTMLEdView.h : interface of the CHTMLEdView class
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

#if !defined(AFX_HTMLEDVIEW_H__2BCEEF8D_CE4C_11D1_AD5B_0060083E86DF__INCLUDED_)
#define AFX_HTMLEDVIEW_H__2BCEEF8D_CE4C_11D1_AD5B_0060083E86DF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
interface IDHTMLEdit;

#define FORMATBASE 200
#define COLORBASE 300
class CFormatMenu : public CMenu
{
public:
    void ClearStringArray()
    {
        m_sa.RemoveAll();
    }
    CStringArray& GetStringArray(){ return m_sa; }
private:
    CStringArray m_sa;
};

class CHTMLEdView : public CHtmlEditView
{
protected: // create from serialization only
    CHTMLEdView();
    void OnEditBarNotify(NMHDR* pNMHDR, LRESULT* pResult);
    DECLARE_DYNCREATE(CHTMLEdView)
    DECLARE_DHTMLEDITING_CMDMAP(CHTMLEdView);
    bool m_bUserMode;
// Attributes
public:
    CHTMLEdDoc* GetDocument();
// Operations
public:

// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CHTMLEdView)
    public:
    virtual void OnInitialUpdate();
    protected:
    virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);
    //}}AFX_VIRTUAL

// Implementation
public:

    BOOL GetCommandStatus(UINT nCmdID);
    void OnUpdateBlockFmt(CCmdUI* pCmdUI);
    void OnUpdateColor(CCmdUI *pCmdUI);
    CFormatMenu* m_pFmtMenu;
    void OnDocumentComplete();
    void OnDisplayChanged();
    virtual HRESULT OnShowContextMenu(DWORD dwID,
                              LPPOINT ppt,
                              LPUNKNOWN pcmdtReserved,
                              LPDISPATCH pdispReserved);
    virtual HRESULT OnUpdateUI();
    void OnContextMenuAction(long itemIndex);
    void UpdateView();
    virtual ~CHTMLEdView();
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
    void SetAbsPos(UINT);
    void SetStaticPos(UINT);
    void OnFormatPopup(UINT);
    CMenu* m_pClrMenu;
    CString m_prevFormat;
    void PopColorMenu();
    void PopBlockFmtMenu();
    void UpdateFontCombos();
    //{{AFX_MSG(CHTMLEdView)

    //}}AFX_MSG
    void OnUpdateToolbarButtons(CCmdUI* pCmdUI);
    DECLARE_MESSAGE_MAP()

private:
    CMenu* m_pMainMenu;

};

#ifndef _DEBUG  // debug version in HTMLEdView.cpp
inline CHTMLEdDoc* CHTMLEdView::GetDocument()
   { return (CHTMLEdDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HTMLEDVIEW_H__2BCEEF8D_CE4C_11D1_AD5B_0060083E86DF__INCLUDED_)
