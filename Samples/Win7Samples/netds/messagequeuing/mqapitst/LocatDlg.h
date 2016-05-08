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
// LocatDlg.h : header file
//




/////////////////////////////////////////////////////////////////////////////
// CLocateDialog dialog

class CLocateDialog : public CDialog
{
// Construction
public:
	CLocateDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CLocateDialog)
	enum { IDD = IDD_LOCATE_DIALOG };
	CString	m_szLabel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLocateDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CLocateDialog)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	void GetLabel(TCHAR szLabelBuffer[BUFFERSIZE])
	{
		ASSERT(szLabelBuffer != NULL);
	
		_tcsncpy_s(szLabelBuffer, BUFFERSIZE, m_szLabel, BUFFERSIZE-1);
	}


};
