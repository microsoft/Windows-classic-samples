#if !defined(AFX_ADSPROPERTYLIST_H__FA1B9781_0880_11D2_A3EA_0080C7D071BF__INCLUDED_)
#define AFX_ADSPROPERTYLIST_H__FA1B9781_0880_11D2_A3EA_0080C7D071BF__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// ADsPropertyList.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgIADsPropertyList dialog

class CDlgIADsPropertyList : public CDialog
{
// Construction
public:
	CDlgIADsPropertyList( LPUNKNOWN pUnk, CWnd* pParent = NULL);   // standard constructor
	~CDlgIADsPropertyList();
// Dialog Data
	//{{AFX_DATA(CDlgIADsPropertyList)
	enum { IDD = IDD_PROPERTYLIST };
	CListBox	m_cValueList;
	CComboBox	m_cADsType;
	CString	m_sAttribute;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgIADsPropertyList)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	IADsPropertyList *m_pPropList;

	// Generated message map functions
	//{{AFX_MSG(CDlgIADsPropertyList)
	virtual BOOL OnInitDialog();
	afx_msg void OnGet();
	afx_msg void OnPurge();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ADSPROPERTYLIST_H__FA1B9781_0880_11D2_A3EA_0080C7D071BF__INCLUDED_)
