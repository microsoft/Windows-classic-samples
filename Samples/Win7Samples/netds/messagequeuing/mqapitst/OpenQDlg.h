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
// OpenQDlg.h : header file
//


/////////////////////////////////////////////////////////////////////////////
// COpenQueueDialog dialog
#ifndef _OPENQDLG_H
#define _OPENQDLG_H

#define OPENQDLG_CLEAR_COMBOBOX	-1
class COpenQueueDialog : public CDialog
{
// Construction
public:
	COpenQueueDialog(CArray <ARRAYQ*, ARRAYQ*>*, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(COpenQueueDialog)
	enum { IDD = IDD_OPEN_QUEUE_DIALOG };
	CComboBox	m_FormatNameCB;
	CComboBox	m_PathNameCB;
	CString	m_szPathName;
	CString	m_szFormatName;
	int		m_iAccessRight;
	//}}AFX_DATA

	/* pointer to the array with the strings for the combo box (Queues PathName). */
	CArray <ARRAYQ*, ARRAYQ*>* m_pStrArray ; 

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COpenQueueDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(COpenQueueDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeFormatnameCombo2();
	afx_msg void OnSelchangePathnameCombo();
	afx_msg void OnEditupdateFormatnameCombo2();
	afx_msg void OnEditupdatePathnameCombo();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:

public:
	void GetFormatName(TCHAR szFormatName[BUFFERSIZE], size_t sizeinChars);


		
	void GetPathName(TCHAR szPathName[BUFFERSIZE])
	{
		ASSERT(szPathName != NULL);
	
		_tcsncpy_s(szPathName, BUFFERSIZE, m_szPathName, BUFFERSIZE-1);
		
	}

	DWORD GetAccess()
	{
		switch (m_iAccessRight)
		{
			case 0:
				return MQ_SEND_ACCESS;
				break;
			case 1:
				return MQ_RECEIVE_ACCESS;
				break;
			case 2:
				return MQ_PEEK_ACCESS;
				break;
			default:
				return 0;
				break;
		}
	}
};

#endif
