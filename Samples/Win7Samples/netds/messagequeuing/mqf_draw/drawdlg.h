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
// drawdlg.h : header file
//


#include "drawarea.h"

//#define FORMAT_NAME_LIST_LEN 1024

/////////////////////////////////////////////////////////////////////////////
// CDisdrawDlg dialog

class CDisdrawDlg : public CDialog
{
// Construction
public:
	
	
	CDisdrawDlg(CWnd* pParent = NULL);	// standard constructor

	void SendMouseMovement(LINE lineNew);
	void SendKeystroke(UINT uChar);

// Data
	CString m_strLogin;
	HANDLE m_hqIncoming;
	HANDLE m_hqOutgoing;
	int		m_iRadioDS;
	WCHAR  * m_wcsFormatNameList;
	BOOL m_fDsEnabledLocaly;
	unsigned int m_iFormatNameList;


// Dialog Data
	//{{AFX_DATA(CDisdrawDlg)
	enum { IDD = IDD_DISDRAW_DIALOG };
	CButton	m_cFriendFrame;
	CStatic	m_cComputerLabel;
	CButton	m_CancelButton;
	CStatic	m_cQueueLabel;
	CButton	m_cMessageFrame;
	CButton m_cQueueFrame;
	CButton	m_cRadioExpress;
	CEdit	m_cComputerInput;
	CEdit	m_cQueueInput;
	CDrawArea	m_drawScribble;
	CButton	m_btnAttach;
	CString	m_strFriend;
	int		m_iDelivery;
	int		m_iType;
	CString	m_strRemoteComputerName;
    CButton	m_cRadioRecoverable;
	CButton m_cRadioPublic;
	CButton m_cRadioPrivate;
	CButton m_btnAddQueue;
	CStatic m_cConnectionMode;
	//}}AFX_DATA
    


	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDisdrawDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void startReceiveUpdateThread();
	BOOL OpenPrivateReceiveQueue();
	HICON m_hIcon;
	BOOL OpenReceiveQueue();


	// Generated message map functions
	//{{AFX_MSG(CDisdrawDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnButtonAttach();
	afx_msg void OnButtonAddQueue();
	afx_msg void OnClose();
	afx_msg void OnRadioPrivatequeue();
	afx_msg void OnRadioPublicqueue();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
//private:

	
};
