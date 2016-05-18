// SourceView.cpp : implementation file
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
#include "SourceView.h"
#include "htmleddoc.h"
#include "mainfrm.h"
#include "htmledview.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSourceView

IMPLEMENT_DYNCREATE(CSourceView, CEditView)

CSourceView::CSourceView()
{
}

CSourceView::~CSourceView()
{
}


BEGIN_MESSAGE_MAP(CSourceView, CEditView)
	//{{AFX_MSG_MAP(CSourceView)
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSourceView drawing

void CSourceView::OnDraw(CDC* /*pDC*/)
{
}

/////////////////////////////////////////////////////////////////////////////
// CSourceView diagnostics

#ifdef _DEBUG
void CSourceView::AssertValid() const
{
	CEditView::AssertValid();
}

void CSourceView::Dump(CDumpContext& dc) const
{
	CEditView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CSourceView message handlers

int CSourceView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CEditView::OnCreate(lpCreateStruct) == -1)
		return -1;
	CEdit &edit = GetEditCtrl();

	//try for courier, if that fails, go for the ANSI fixed font
	if(!m_font.CreateFont(-MulDiv(10, GetDeviceCaps(edit.GetDC()->m_hDC, LOGPIXELSY), 72)
			,0,0,0,FW_DONTCARE,0,0,0,ANSI_CHARSET,OUT_TT_PRECIS,CLIP_DEFAULT_PRECIS
			,DEFAULT_QUALITY,DEFAULT_PITCH,_T("COURIER")))	
	{
		m_font.CreateStockObject(ANSI_FIXED_FONT);
	}
	edit.SetFont(&m_font);
	
	return 0;
}

void CSourceView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView) 
{
	if(bActivate && pActivateView==this && pDeactiveView != this)
	{
		//we're being activated get the HTML from the WebView
		UpdateView();
	}
	
	CEditView::OnActivateView(bActivate, pActivateView, pDeactiveView);
}

void CSourceView::UpdateView()
{
	CMainFrame *pFrame = (CMainFrame*)AfxGetMainWnd();
	ASSERT_VALID(pFrame);

	CHTMLEdView* pWebView = (CHTMLEdView*)pFrame->GetWebView();
	ASSERT_VALID(pWebView);
	CString strHTML;
	pWebView->GetDocumentHTML(strHTML);
	SetWindowText(strHTML);
}