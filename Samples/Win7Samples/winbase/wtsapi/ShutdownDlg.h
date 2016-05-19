//  Copyright 1995-1999, Citrix Systems Inc.
//  Copyright (c) 1997 - 2000  Microsoft Corporation
#if !defined(AFX_SHUTDOWNDLG_H__75CF0BAF_3508_11D1_8123_00C04FBBB23F__INCLUDED_)
#define AFX_SHUTDOWNDLG_H__75CF0BAF_3508_11D1_8123_00C04FBBB23F__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// ShutdownDlg.h : header file
//

#define MAXSHUTDOWNITEMS 5

/*#define WSD_LOGOFF		0x00000001
#define WSD_SHUTDOWN	0x00000002
#define WSD_REBOOT		0x00000004
#define WSD_POWEROFF	0x00000008
#define WSD_FASTREBOOT	0x00000010  */

typedef struct _Shutdown {
	TCHAR ShutdownName[35];
	DWORD ShutdownFlag;
} Shutdown;


/////////////////////////////////////////////////////////////////////////////
// CShutdownDlg dialog

class CShutdownDlg : public CDialog
{
// Construction
public:
	CShutdownDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CShutdownDlg)
	enum { IDD = IDD_SHUTDOWN };
	CEdit	m_serverName;
	CListBox	m_shutdown;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CShutdownDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CShutdownDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnShutdown();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	HANDLE serverHandle;
	TCHAR* serverName;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SHUTDOWNDLG_H__75CF0BAF_3508_11D1_8123_00C04FBBB23F__INCLUDED_)
