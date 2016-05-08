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
//

#include "drawarea.h"

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

// Dialog Data
	//{{AFX_DATA(CDisdrawDlg)
	enum { IDD = IDD_DISDRAW_DIALOG };
	CStatic	m_cComputerLabel;
	CButton	m_CancelButton;
	CStatic	m_cQueueLabel;
	CButton	m_cMessageFrame;
	CButton	m_cDsFrame;
	CButton	m_cRadioExpress;
	CButton	m_cRadioDS;
	CEdit	m_cComputerInput;
	CEdit	m_cQueueInput;
	CButton	m_cContinueButton;
	CDrawArea	m_drawScribble;
	CButton	m_btnAttach;
	CString	m_strFriend;
	int		m_iDelivery;
	int		m_iRadioDS;
	CString	m_strRemoteComputerName;
    CButton	m_cRadioWorkgroup;
    CButton	m_cRadioRecoverable;
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
	void initializeUi(BOOL fConnectedToDS);
	HICON m_hIcon;

	BOOL OpenReceiveQueue();
    BOOL IsDsEnabledLocaly();


	// Generated message map functions
	//{{AFX_MSG(CDisdrawDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnButtonAttach();
	afx_msg void OnChangeEditFriend();
	afx_msg void OnClose();
	afx_msg void OnConnect();
	afx_msg void OnChangeEditFriendComputer();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	BOOL m_fDsEnabledLocaly;
};
