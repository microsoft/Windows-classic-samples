// HTMLEdDoc.h : interface of the CHTMLEdDoc class
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

#if !defined(AFX_HTMLEDDOC_H__2BCEEF8B_CE4C_11D1_AD5B_0060083E86DF__INCLUDED_)
#define AFX_HTMLEDDOC_H__2BCEEF8B_CE4C_11D1_AD5B_0060083E86DF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define DELETE_EXCEPTION(e) do { e->Delete(); } while (0)

class CHTMLEdDoc : public CHtmlEditDoc
{
protected: // create from serialization only
	CHTMLEdDoc();
	DECLARE_DYNCREATE(CHTMLEdDoc)

// Attributes
private:
	CString m_sSaveFileName;
public:
	BOOL m_bDoSaveOnDocCom;
	BOOL m_bCallNewDocument;
	BOOL m_bCallCloseDocument;
	BOOL SaveMyDocument();
	BOOL OnSaveDocument(LPCTSTR lpszFileName);
	BOOL OnNewDocument();
	void OnCloseDocument();
	BOOL OnOpenDocument(LPCTSTR lpszFileName);
	BOOL IsModified();
// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHTMLEdDoc)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CHTMLEdDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
// Generated message map functions
protected:
	//{{AFX_MSG(CHTMLEdDoc)
	afx_msg void OnFileOpenurl();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HTMLEDDOC_H__2BCEEF8B_CE4C_11D1_AD5B_0060083E86DF__INCLUDED_)
