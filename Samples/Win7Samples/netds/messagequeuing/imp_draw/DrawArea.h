// --------------------------------------------------------------------
//
//  Copyright (c) Microsoft Corporation.  All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// --------------------------------------------------------------------

//
// drawarea.h : header file
//


#include <afxtempl.h>

#define MAX_MSG_BODY_LEN 32

typedef struct tagLINE
{
	CPoint ptStart;
	CPoint ptEnd;
} LINE;

/////////////////////////////////////////////////////////////////////////////
// CDrawArea window

class CDrawArea : public CEdit
{
// Construction
public:
	CDrawArea();

// Attributes
public:

protected:
	CPoint m_ptLast;
	CList<LINE, LINE &> m_listLines;

// Operations
public:
	void AddLine(LINE line);
	void AddKeystroke(char *mbsKey);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDrawArea)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CDrawArea();

	// Generated message map functions
protected:
	//{{AFX_MSG(CDrawArea)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnPaint();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
