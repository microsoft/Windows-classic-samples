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
// CrQDlg.h : header file
//



/////////////////////////////////////////////////////////////////////////////
// CCreateQueueDialog dialog

class CCreateQueueDialog : public CDialog
{
// Construction
public:
	CCreateQueueDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CCreateQueueDialog)
	enum { IDD = IDD_CREATE_QUEUE_DIALOG };
	CString	m_strLabel;
	CString	m_strPathName;
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCreateQueueDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CCreateQueueDialog)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	void GetPathName(TCHAR szPathNameBuffer[BUFFERSIZE])
	{
		ASSERT(szPathNameBuffer != NULL);
			
		_tcsncpy_s(szPathNameBuffer, BUFFERSIZE, LPCTSTR(m_strPathName), BUFFERSIZE-1);
	}

	void GetLabel(TCHAR szLabelBuffer[BUFFERSIZE])
	{
		ASSERT(szLabelBuffer != NULL);
		
		_tcsncpy_s(szLabelBuffer, BUFFERSIZE, LPCTSTR(m_strLabel), BUFFERSIZE-1);
		
	}

};
