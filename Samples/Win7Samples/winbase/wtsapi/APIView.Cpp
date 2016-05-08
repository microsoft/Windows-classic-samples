// APIView.cpp : implementation of the CAPIView class
//
//  Copyright 1995-1998, Citrix Systems Inc.
//  Copyright (c) 1997 - 2000  Microsoft Corporation


#include "stdafx.h"
#include "API.h"

#include "APIDoc.h"
#include "APIView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAPIView

IMPLEMENT_DYNCREATE(CAPIView, CView)

BEGIN_MESSAGE_MAP(CAPIView, CView)
	//{{AFX_MSG_MAP(CAPIView)
	ON_WM_SETFOCUS()
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAPIView construction/destruction

CAPIView::CAPIView()
{
	//g_pDoc->SetServerName("");
}

CAPIView::~CAPIView()
{
}

BOOL CAPIView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CAPIView drawing

void CAPIView::OnDraw(CDC* pDC)
{
	CAPIDoc *pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	TCHAR* serverName;
	serverName = g_pDoc->GetServerName();
	if (*serverName == NULL)
		pDoc->SetTitle("local server");
	else
		pDoc->SetTitle(serverName);		

}

/////////////////////////////////////////////////////////////////////////////
// CAPIView diagnostics

#ifdef _DEBUG
void CAPIView::AssertValid() const
{
	CView::AssertValid();
}

void CAPIView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CAPIDoc* CAPIView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CAPIDoc)));
	return (CAPIDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CAPIView message handlers


void CAPIView::OnSetFocus(CWnd* pOldWnd) 
{
	CView::OnSetFocus(pOldWnd);
	CDC* pDC=NULL;
	OnDraw(pDC);
}

void CAPIView::OnPaint() 
{
    CPaintDC dc(this); // device context for painting
	LOGFONT logFont;
	logFont.lfHeight = 30;
	logFont.lfWidth = 20;
	logFont.lfEscapement = 0;
	logFont.lfOrientation = 0;
	logFont.lfWeight= 0;
	logFont.lfItalic = 0;
	logFont.lfUnderline = 0;
	logFont.lfStrikeOut = 0;
	logFont.lfCharSet = ANSI_CHARSET;
	CFont font;
	font.CreateFontIndirect(&logFont);
	CFont* oldFont = dc.SelectObject(&font);

	_tcscpy_s(logFont.lfFaceName, LF_FACESIZE, "Times New Roman");
	dc.TextOut(50, 50, "Test Program for WTSAPI32.DLL");	
	dc.SelectObject(oldFont);
}
