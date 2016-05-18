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
// DelQDlg.h : header file
//



/////////////////////////////////////////////////////////////////////////////
// CDeleteQueueDialog dialog

class CDeleteQueueDialog : public CDialog
{
// Construction
public:
	CDeleteQueueDialog(CArray <ARRAYQ*, ARRAYQ*>*, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDeleteQueueDialog)
	enum { IDD = IDD_DELETE_QUEUE_DIALOG };
	CComboBox	m_PathNameCB;
	CString	m_szPathName;
	//}}AFX_DATA

	/* pointer to the array with the strings for the combo box (Queues PathName). */
	CArray <ARRAYQ*, ARRAYQ*>* m_pStrArray ; 

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDeleteQueueDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDeleteQueueDialog)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	void GetPathName(TCHAR szPathName[BUFFERSIZE])
	{
		ASSERT(szPathName != NULL);
	
		_tcsncpy_s(szPathName, BUFFERSIZE, m_szPathName, BUFFERSIZE-1);
		
	}

};
