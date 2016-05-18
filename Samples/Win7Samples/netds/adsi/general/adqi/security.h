#if !defined(AFX_SECURITY_H__99695D7A_1D58_11D2_B21E_0000F87A6B50__INCLUDED_)
#define AFX_SECURITY_H__99695D7A_1D58_11D2_B21E_0000F87A6B50__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// security.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgIADsSecurityDescriptor dialog


class CDlgIADsSecurityDescriptor : public CDialog
{
// Construction
public:
	CDlgIADsSecurityDescriptor(LPUNKNOWN, CWnd* pParent = NULL);   // standard constructor
	~CDlgIADsSecurityDescriptor();
	
// Dialog Data
	//{{AFX_DATA(CDlgIADsSecurityDescriptor)
	enum { IDD = IDD_SECURITY };
	CListBox	m_cACEList;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgIADsSecurityDescriptor)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void PopulateACL( IADsAccessControlList *pACL );
	void ResetAcePtr();
	void DisplayACE( IADsAccessControlEntry *pACE );
	void ResetAceMaskControls();
	void DisplayAceObjectType( UINT nID, CString &sObjectType );

	IADsSecurityDescriptor  *m_pSecurityDesc;
	CPtrList				 m_acePtrList;
	CADsSearch				 m_search;

	// Generated message map functions
	//{{AFX_MSG(CDlgIADsSecurityDescriptor)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelChangeAceList();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SECURITY_H__99695D7A_1D58_11D2_B21E_0000F87A6B50__INCLUDED_)
