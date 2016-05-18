// ADQIDlg.h : header file
//

#if !defined(AFX_ADQIDLG_H__81709771_0672_11D2_B218_0000F87A6B50__INCLUDED_)
#define AFX_ADQIDLG_H__81709771_0672_11D2_B218_0000F87A6B50__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

/////////////////////////////////////////////////////////////////////////////
// CADQIDlg dialog

class CADQIDlg : public CDialog
{
// Construction
public:
	void EnumerateInterface();
	CADQIDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CADQIDlg)
	enum { IDD = IDD_ADQI_DIALOG };
	CListBox	m_cListIf;
	CButton	m_cOK;
	CEdit	m_cADsPath;
	CString	m_sADsPath;
	LPUNKNOWN m_pUnk;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CADQIDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CADQIDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	virtual void OnOK();
	afx_msg void OnChangeADsPath();
	virtual void OnCancel();
	afx_msg void OnDblClkInterfaces();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


void DlgProcIADsOpenDSObject( LPUNKNOWN pUnk, LPUNKNOWN *ppNew);




//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ADQIDLG_H__81709771_0672_11D2_B218_0000F87A6B50__INCLUDED_)







