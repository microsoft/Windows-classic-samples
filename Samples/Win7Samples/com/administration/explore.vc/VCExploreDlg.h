// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#if !defined(AFX_VCEXPLOREDLG_H__12723F27_B2F5_11D2_86A1_000000000000__INCLUDED_)
#define AFX_VCEXPLOREDLG_H__12723F27_B2F5_11D2_86A1_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CVCExploreDlg dialog

class CVCExploreDlg : public CDialog
{
// Construction
public:
	void set_ComputerName(_bstr_t bstrComputerName);
	void set_CurrentCollection(ICatalogCollection* pCollection);
	CVCExploreDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CVCExploreDlg)
	enum { IDD = IDD_VCEXPLORE };
	CButton	m_btnSetProperty;
	CEdit		m_edtPropertyValue;
	CListBox	m_lstRelatedCollections;
	CListBox	m_lstProperties;
	CListBox	m_lstParentCollections;
	CListBox	m_lstObjects;
	CStatic		m_lblComputerName;
	CStatic		m_lblCollectionName;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CVCExploreDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CVCExploreDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnTBtnAbout();
	afx_msg void OnTBtnConnect();
	afx_msg void OnTBtnDelete();
	afx_msg void OnTBtnExportApp();
	afx_msg void OnTBtnImportComponent();
	afx_msg void OnTBtnInstallApp();
	afx_msg void OnTBtnInstallComponent();
	afx_msg void OnTBtnNew();
	afx_msg void OnTBtnRefresh();
	afx_msg void OnTBtnSave();
	afx_msg void OnTBtnStartApp();
	afx_msg void OnTBtnStopApp();
	afx_msg void OnTBtnUtility();
	afx_msg void OnClose();
	afx_msg void OnSelChangeRelatedCollections();
	afx_msg void OnSelChangeParentCollections();
	afx_msg void OnSelChangeObjects();
	afx_msg void OnSelChangeProperties();
	afx_msg void OnSetProperty();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	ICOMAdminCatalog*	GetCatalog();
	HRESULT		CreateCatalog();
	void		DestroyCatalog();
	HRESULT		NavigateTo(ICatalogCollection* pCatalogCollection, BOOL bPopulate = TRUE);
	void		DisplayPropertyValue();
	int			AddParent(ICatalogCollection* pCatalogCollection);
	void		RemoveParent(int nNewCurrIndex);
  void    TryToSaveChanges();

	HWND				m_hwndToolBar;
	ICOMAdminCatalog*	m_pCatalog;
	ICatalogCollection* m_pCurrCollection;
  BOOL   m_bChangesArePending;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VCEXPLOREDLG_H__12723F27_B2F5_11D2_86A1_000000000000__INCLUDED_)
