// ADs.h: interface for the CADs class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ADS_H__81709782_0672_11D2_B218_0000F87A6B50__INCLUDED_)
#define AFX_ADS_H__81709782_0672_11D2_B218_0000F87A6B50__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000



/////////////////////////////////////////////////////////////////////////////
// CDlgIADs dialog

class CDlgIADs : public CDialog
{
// Construction
public:
	CDlgIADs(LPUNKNOWN, CWnd* pParent = NULL);   // standard constructor
	~CDlgIADs();
// Dialog Data
	//{{AFX_DATA(CDlgIADs)
	enum { IDD = IDD_IADS };
	CListBox	m_cValueList;
	CComboBox	m_cAttrList;
	CString	m_sADsPath;
	CString	m_sClass;
	CString	m_sName;
	CString	m_sParent;
	CString	m_sSchema;
	IADs   *m_pADs;
	CString	m_sGUID;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgIADs)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	
	// Generated message map functions
	//{{AFX_MSG(CDlgIADs)
	virtual BOOL OnInitDialog();
	afx_msg void OnGet();
	afx_msg void OnGetEx();
	afx_msg void OnGetInfo();
	afx_msg void OnSetInfo();
	afx_msg void OnGetInfoex();
	afx_msg void OnPut();
	afx_msg void OnPutEx();
	afx_msg void OnParentPath();
	afx_msg void OnSchemaPath();
	afx_msg void OnBindGuid();
	afx_msg void OnSelChangeAttrList();
	afx_msg void OnDblClkAttrValue();
	afx_msg void OnCopy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


#endif // !defined(AFX_ADS_H__81709782_0672_11D2_B218_0000F87A6B50__INCLUDED_)
/////////////////////////////////////////////////////////////////////////////
// CGetInfoExDlg dialog

class CGetInfoExDlg : public CDialog
{
// Construction
public:
	CGetInfoExDlg(CWnd* pParent = NULL);   // standard constructor
	CString GetAttribute() { return m_sAttr; }

// Dialog Data
	//{{AFX_DATA(CGetInfoExDlg)
	enum { IDD = IDD_GETINFOEX };
	CString	m_sAttrText;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGetInfoExDlg)
	protected:
		
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CString m_sAttr;
	// Generated message map functions
	//{{AFX_MSG(CGetInfoExDlg)
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	
};
