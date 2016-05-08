// APIView.h : interface of the CAPIView class
//
//  Copyright 1995-1999, Citrix Systems Inc.
//  Copyright (c) 1997 - 2000  Microsoft Corporation

/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_APIVIEW_H__C090CEF2_303B_11D1_8310_00C04FBEFCDA__INCLUDED_)
#define AFX_APIVIEW_H__C090CEF2_303B_11D1_8310_00C04FBEFCDA__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class CAPIView : public CView
{
protected: // create from serialization only
	CAPIView();
	DECLARE_DYNCREATE(CAPIView)

// Attributes
public:
	CAPIDoc* GetDocument();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAPIView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CAPIView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CAPIView)
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
};

#ifndef _DEBUG  // debug version in APIView.cpp
inline CAPIDoc* CAPIView::GetDocument()
   { return (CAPIDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_APIVIEW_H__C090CEF2_303B_11D1_8310_00C04FBEFCDA__INCLUDED_)
