// HTMLEdDoc.cpp : implementation of the CHTMLEdDoc class
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
#include "geturl.h"
#include "mainfrm.h"
#include "sourceview.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CHTMLEdDoc

IMPLEMENT_DYNCREATE(CHTMLEdDoc, CHtmlEditDoc)

BEGIN_MESSAGE_MAP(CHTMLEdDoc, CHtmlEditDoc)
	//{{AFX_MSG_MAP(CHTMLEdDoc)
	ON_COMMAND(ID_FILE_OPENURL, OnFileOpenurl)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHTMLEdDoc construction/destruction

CHTMLEdDoc::CHTMLEdDoc()
{
	m_bDoSaveOnDocCom = FALSE;
	m_bCallNewDocument = FALSE;
	m_bCallCloseDocument = FALSE;
}

CHTMLEdDoc::~CHTMLEdDoc()
{
}


/////////////////////////////////////////////////////////////////////////////
// CHTMLEdDoc diagnostics

#ifdef _DEBUG
void CHTMLEdDoc::AssertValid() const
{
	CHtmlEditDoc::AssertValid();
}

void CHTMLEdDoc::Dump(CDumpContext& dc) const
{
	CHtmlEditDoc::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CHTMLEdDoc commands

//We want the default functionality of CHtmlEditDoc::OnNewDocument(), but just need to update
//the view with the new document contents
BOOL CHTMLEdDoc::OnNewDocument()
{
	//If the old document needs to be saved, and is waiting for a OnDocumentComplete,
	//OnNewDocument waits for it to gets done
	if(m_bDoSaveOnDocCom)
	{
		m_bCallNewDocument = TRUE;
		return FALSE;
	}

	BOOL bRet = CHtmlEditDoc::OnNewDocument();
	if (bRet)
	{
		// have to clear the source view too!
		CMainFrame *pFrm = (CMainFrame*)AfxGetMainWnd();
		if (pFrm)
		{
			CSourceView *pView = (CSourceView*)pFrm->GetSourceView();
			if (pView)
			{
				pView->NewDocument();
				pFrm->SwapView(ID_VIEW_WEB);
			}
		}
	}
	return bRet;
}

void CHTMLEdDoc::OnCloseDocument()
{
	//If the old document needs to be saved, and is waiting for a OnDocumentComplete,
	//OnCloseDocument waits for it to gets done
	if(m_bDoSaveOnDocCom)
	{
		m_bCallCloseDocument = TRUE;
		return;
	}

	CDocument::OnCloseDocument();
}

//We want the default functionality of CHtmlEditDoc::OnOpenDocument(lpszFileName), but just
//need to update the view with the opened document contents
BOOL CHTMLEdDoc::OnOpenDocument(LPCTSTR lpszFileName)
{
	BOOL bRet = CHtmlEditDoc::OnOpenDocument(lpszFileName);
	if (bRet)
	{
		// have to clear the source view too!
		CMainFrame *pFrm = (CMainFrame*)AfxGetMainWnd();		
		if (pFrm)
		{
			CSourceView *pView = (CSourceView*)pFrm->GetSourceView();
			if (pView)
			{
				pView->NewDocument();
				pFrm->SwapView(ID_VIEW_WEB);
			}
		}
	}

	return bRet;
}

void CHTMLEdDoc::OnFileOpenurl() 
{
	CGetURL dlg;
	if(IDOK==dlg.DoModal())	
		OpenURL(dlg.m_URL);	
}

BOOL CHTMLEdDoc::IsModified()
{
	//Whether in Source or Web view, whenever we edit, CDocument::SetModified get's called.
	return CDocument::IsModified();
}

//We want the default functionality of CHtmlEditDoc::OnSaveDocument(), but need to update
//the web view with the contents of the source view before saving
BOOL CHTMLEdDoc::OnSaveDocument(LPCTSTR lpszFileName)
{
	CMainFrame *pFrame = (CMainFrame*)AfxGetMainWnd();
	ASSERT_VALID(pFrame);

	if(pFrame->GetCurrentView()==ID_VIEW_SOURCE)
	{
		//UpdateView is asynchronous, so we wait for the CHTMLEdView::OnDocumentComplete event
		//to occur which indicates that the view has been updated, before saving the document
		CHTMLEdView* pWebView = (CHTMLEdView*)pFrame->GetWebView();
		ASSERT_VALID(pWebView);
		pWebView->UpdateView();
		m_sSaveFileName = lpszFileName;
		m_bDoSaveOnDocCom = TRUE;
	}
	else
	{
		//For web view, the document already contains the changes, so we directly save it
		m_sSaveFileName = lpszFileName;
		return SaveMyDocument();
	}
	return TRUE;
}

BOOL CHTMLEdDoc::SaveMyDocument()
{
	return CHtmlEditDoc::OnSaveDocument(m_sSaveFileName);
}