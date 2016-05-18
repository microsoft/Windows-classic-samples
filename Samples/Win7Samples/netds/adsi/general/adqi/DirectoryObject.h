// DirectoryObject.h: interface for the CDirectoryObject class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DIRECTORYOBJECT_H__81709784_0672_11D2_B218_0000F87A6B50__INCLUDED_)
#define AFX_DIRECTORYOBJECT_H__81709784_0672_11D2_B218_0000F87A6B50__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000



/////////////////////////////////////////////////////////////////////////////
// CDlgIDirectoryObject dialog

class CDlgIDirectoryObject : public CDialog
{
// Construction
public:
	CDlgIDirectoryObject(LPUNKNOWN pUnk, CWnd* pParent = NULL);   // standard constructor
	~CDlgIDirectoryObject();
// Dialog Data
	//{{AFX_DATA(CDlgIDirectoryObject)
	enum { IDD = IDD_IDIRECTORYOBJECT };
	CEdit	m_cAttributes;
	CListBox	m_cValueList;
	CString	m_sDN;
	CString	m_sRDN;
	CString	m_sSchema;
	CString	m_sClass;
	CString	m_sParent;
	CString	m_sAttributes;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgIDirectoryObject)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void ShowAttributes();
	 IDirectoryObject *m_pDirObject;

	// Generated message map functions
	//{{AFX_MSG(CDlgIDirectoryObject)
	afx_msg void OnDelete();
	afx_msg void OnGet();
	afx_msg void OnChangeAttribute();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif // !defined(AFX_DIRECTORYOBJECT_H__81709784_0672_11D2_B218_0000F87A6B50__INCLUDED_)
/////////////////////////////////////////////////////////////////////////////
// CDeleteObjectDlg dialog

class CDeleteObjectDlg : public CDialog
{
// Construction
public:
	CDeleteObjectDlg(CWnd* pParent = NULL);   // standard constructor
	CString GetObjectName() { return m_sRDN; }
// Dialog Data
	//{{AFX_DATA(CDeleteObjectDlg)
	enum { IDD = IDD_DELETEOBJECT };
	CString	m_sDelete;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDeleteObjectDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CString m_sRDN;

	// Generated message map functions
	//{{AFX_MSG(CDeleteObjectDlg)
	virtual void OnOK();
	afx_msg void OnChangeDelete();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
