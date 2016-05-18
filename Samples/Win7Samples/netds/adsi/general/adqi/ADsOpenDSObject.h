#if !defined(AFX_ADSOPENDSOBJECT_H__60690A52_0936_11D2_A3EC_0080C7D071BF__INCLUDED_)
#define AFX_ADSOPENDSOBJECT_H__60690A52_0936_11D2_A3EC_0080C7D071BF__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// ADsOpenDSObject.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgIADsOpenDSObject dialog

class CDlgIADsOpenDSObject : public CDialog
{
// Construction
public:
	CDlgIADsOpenDSObject(LPUNKNOWN pUnk, LPUNKNOWN *ppNew, CWnd* pParent = NULL);   // standard constructor
	~CDlgIADsOpenDSObject();
	// Dialog Data
	//{{AFX_DATA(CDlgIADsOpenDSObject)
	enum { IDD = IDD_IADSOPENDSOBJECT };
	CString	m_sPassword;
	CString	m_sUserName;
	BOOL	m_bEncrypt;
	BOOL	m_bReadOnly;
	BOOL	m_bSecured;
	BOOL	m_bPrompt;
	BOOL	m_bNoAuthentication;
	CString	m_sADsPath;
	BOOL	m_bClearText;
	BOOL	m_bFastBind;
	BOOL	m_bSealing;
	BOOL	m_bSigning;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgIADsOpenDSObject)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	IADsOpenDSObject	*m_pOpenDS;
	LPUNKNOWN			 *m_ppUnk;
	
	// Generated message map functions
	//{{AFX_MSG(CDlgIADsOpenDSObject)
	virtual void OnOK();
	afx_msg void OnChangeUsername();
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeADsPath();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ADSOPENDSOBJECT_H__60690A52_0936_11D2_A3EC_0080C7D071BF__INCLUDED_)
