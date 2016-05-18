#if !defined(AFX_ADSLARGEINTEGER_H__AAEF26E2_E0AA_11D2_BC85_00C04FD430AF__INCLUDED_)
#define AFX_ADSLARGEINTEGER_H__AAEF26E2_E0AA_11D2_BC85_00C04FD430AF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ADsLargeInteger.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgIADsLargeInteger dialog

class CDlgIADsLargeInteger : public CDialog
{
// Construction
public:
	CDlgIADsLargeInteger(LPUNKNOWN, CWnd* pParent = NULL);   // standard constructor
	~CDlgIADsLargeInteger();

// Dialog Data
	//{{AFX_DATA(CDlgIADsLargeInteger)
	enum { IDD = IDD_LARGEINTEGER };
	long	m_lHiPart;
	long	m_lLowPart;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgIADsLargeInteger)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	IADsLargeInteger *m_pLargeInt;

	// Generated message map functions
	//{{AFX_MSG(CDlgIADsLargeInteger)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ADSLARGEINTEGER_H__AAEF26E2_E0AA_11D2_BC85_00C04FD430AF__INCLUDED_)
