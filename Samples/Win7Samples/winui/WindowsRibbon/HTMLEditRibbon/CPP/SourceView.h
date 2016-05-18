// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#if !defined(AFX_SOURCEVIEW_H__0D4EF336_E1DD_11D1_AD76_0060083E86DF__INCLUDED_)
#define AFX_SOURCEVIEW_H__0D4EF336_E1DD_11D1_AD76_0060083E86DF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SourceView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSourceView view

class CSourceView : public CEditView
{
protected:
	CSourceView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CSourceView)
	CFont m_font;
// Attributes
public:

	void NewDocument()
	{
		this->DeleteContents();
	}
	void UpdateView();
// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSourceView)
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CSourceView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CSourceView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:

};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SOURCEVIEW_H__0D4EF336_E1DD_11D1_AD76_0060083E86DF__INCLUDED_)
