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
// SendMDlg.h : header file
//



/////////////////////////////////////////////////////////////////////////////
// CSendMessageDialog dialog

class CSendMessageDialog : public CDialog
{
// Construction
public:
	CSendMessageDialog(CArray <ARRAYQ*, ARRAYQ*>*, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSendMessageDialog)
	enum { IDD = IDD_SEND_MESSAGE_DIALOG };
	CComboBox	m_AdminPathNameCB;
	CComboBox	m_PathNameCB;
	CString	m_strBody;
	CString	m_strLabel;
	BYTE	m_bPriority;
	int		m_iAck;
	int		m_iDelivery;
	CString	m_szPathName;
	CString	m_szAdminPathName;
	BOOL	m_Journal;
	BOOL	m_DeadLetter;
	BOOL    m_Authenticated;
	BOOL    m_Encrypted;
	long	m_dwTimeToReachQueue;
	long    m_dwTimeToBeReceived;
	//}}AFX_DATA

	/* pointer to the array with the strings for the combo box (Queues PathName). */
	CArray <ARRAYQ*, ARRAYQ*>* m_pStrArray ;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSendMessageDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSendMessageDialog)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnSelchangeTargetCombo();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	void GetPathName(TCHAR szPathName[BUFFERSIZE])
	{
		ASSERT(szPathName != NULL);
			
		_tcsncpy_s(szPathName, BUFFERSIZE, m_szPathName, BUFFERSIZE-1);

	}


	void GetAdminPathName(TCHAR szAdminPathName[BUFFERSIZE])
	{
		ASSERT(szAdminPathName != NULL);
		
		_tcsncpy_s (szAdminPathName, BUFFERSIZE, m_szAdminPathName, BUFFERSIZE-1);
		
	}

	unsigned char GetDelivery()
	{
		return (m_iDelivery);
	}

    unsigned char GetJournal()
	{
		return (m_Journal);
	}

    unsigned char GetDeadLetter()
	{
		return (m_DeadLetter);
	}

	unsigned char GetAuthenticated()
	{
		return (m_Authenticated);
	}

	unsigned char GetEncrypted()
	{
		return (m_Encrypted);
	}

	unsigned char GetPriority()
	{
		return (m_bPriority);
	}

	unsigned char GetAcknowledge()
	{
		switch (m_iAck)
		{
		case 1 : return MQMSG_ACKNOWLEDGMENT_FULL_REACH_QUEUE;
		case 2 : return MQMSG_ACKNOWLEDGMENT_FULL_RECEIVE;
		case 3 : return MQMSG_ACKNOWLEDGMENT_NACK_REACH_QUEUE;
		case 4 : return MQMSG_ACKNOWLEDGMENT_NACK_RECEIVE;
		default: return MQMSG_ACKNOWLEDGMENT_NONE;
		}
	}

	void GetMessageBody(TCHAR* pszMessageBodyBuffer)
	{
		ASSERT(pszMessageBodyBuffer != NULL);
			
		_tcsncpy_s (pszMessageBodyBuffer, BUFFERSIZE, m_strBody, BUFFERSIZE-1);
		
	}

	void GetMessageLabel(TCHAR szMessageLabelBuffer[BUFFERSIZE])
	{
		ASSERT(szMessageLabelBuffer != NULL);
		
		_tcsncpy_s(szMessageLabelBuffer, BUFFERSIZE, m_strLabel, BUFFERSIZE-1);
		
	}

	DWORD GetTimeToReachQueue()
	{
		return (m_dwTimeToReachQueue);
	}

	DWORD GetTimeToBeReceived()
	{
		return (m_dwTimeToBeReceived);
	}

};

//
// Two buffers to hold the last message label and body.
//
extern TCHAR szLastMessageLabel[BUFFERSIZE];
extern TCHAR szLastMessageBody[BUFFERSIZE];
