// ADsContainer.h: interface for the CADsContainer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ADSCONTAINER_H__81709783_0672_11D2_B218_0000F87A6B50__INCLUDED_)
#define AFX_ADSCONTAINER_H__81709783_0672_11D2_B218_0000F87A6B50__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000



#endif // !defined(AFX_ADSCONTAINER_H__81709783_0672_11D2_B218_0000F87A6B50__INCLUDED_)
/////////////////////////////////////////////////////////////////////////////
// CDlgIADsContainer dialog

class CDlgIADsContainer : public CDialog
{
// Construction
public:
	CDlgIADsContainer(LPUNKNOWN pUnk, CWnd* pParent = NULL);   // standard constructor
	~CDlgIADsContainer();

// Dialog Data
	//{{AFX_DATA(CDlgIADsContainer)
	enum { IDD = IDD_IADSCONTAINER };
	CListBox	m_cChildList;
	CString	m_sFilter;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgIADsContainer)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HRESULT EnumerateChildren();
	IADsContainer *m_pCont;
	CStringList    m_sClassList;

	// Generated message map functions
	//{{AFX_MSG(CDlgIADsContainer)
	virtual BOOL OnInitDialog();
	afx_msg void OnView();
	afx_msg void OnDblClkChildrenList();
	afx_msg void OnDelete();
	virtual void OnOK();
	afx_msg void OnRename();
	afx_msg void OnSet();
	afx_msg void OnMove();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	BOOL GetClassAndName( CString &sClass, CString &sName );
};
/////////////////////////////////////////////////////////////////////////////
// CRenameDlg dialog

class CRenameDlg : public CDialog
{
// Construction
public:
	CRenameDlg(CString sOldName, CWnd* pParent = NULL);   // standard constructor
	CString GetName() { return m_sNewName; }

// Dialog Data
	//{{AFX_DATA(CRenameDlg)
	enum { IDD = IDD_RENAME };
	CString	m_sName;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRenameDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CString m_sNewName;
	CString m_sOldName;
	// Generated message map functions
	//{{AFX_MSG(CRenameDlg)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
/////////////////////////////////////////////////////////////////////////////
// CMoveDlg dialog

class CMoveDlg : public CDialog
{
// Construction
public:
	CMoveDlg(CWnd* pParent = NULL);   // standard constructor
	CString GetObjectPath() { return m_sADsPath; }

// Dialog Data
	//{{AFX_DATA(CMoveDlg)
	enum { IDD = IDD_MOVE };
	CString	m_sADsPath;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMoveDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CMoveDlg)
	virtual void OnOK();
	afx_msg void OnChangeADspath();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
