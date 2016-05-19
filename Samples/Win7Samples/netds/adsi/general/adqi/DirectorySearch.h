#if !defined(AFX_DIRECTORYSEARCH_H__8F1F13D1_0728_11D2_A3E8_0080C7D071BF__INCLUDED_)
#define AFX_DIRECTORYSEARCH_H__8F1F13D1_0728_11D2_A3E8_0080C7D071BF__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// DirectorySearch.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgIDirectorySearch dialog

class CDlgIDirectorySearch : public CDialog
{
// Construction
public:
	CDlgIDirectorySearch(LPUNKNOWN pUnk, CWnd* pParent = NULL);   // standard constructor
	~CDlgIDirectorySearch();
// Dialog Data
	//{{AFX_DATA(CDlgIDirectorySearch)
	enum { IDD = IDD_IDIRECTORYSEARCH };
	CEdit	m_cAttrList;
	CListCtrl	m_cListView;
	CComboBox	m_cChaseReferrals;
	CButton	m_cCacheResult;
	CComboBox	m_CADsSearchScope;
	CButton	m_cEnableFilter;
	CButton	m_cExecute;
	CEdit	m_cFilter;
	CString	m_sFilter;
	BOOL	m_bEnableFilter;
	UINT	m_nTimeOut;
	BOOL	m_bCacheResult;
	UINT	m_nPageSize;
	CString	m_sSortOn;
	UINT	m_nTimeLimit;
	UINT	m_nSizeLimit;
	BOOL	m_bAsynch;
	BOOL	m_bAttrib;
	int		m_nDeref;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgIDirectorySearch)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	IDirectorySearch *m_pSearch;
	// Generated message map functions
	//{{AFX_MSG(CDlgIDirectorySearch)
	afx_msg void OnExecute();
	afx_msg void OnChangeFilter();
	virtual BOOL OnInitDialog();
	afx_msg void OnEnabled();
	virtual void OnOK();
	afx_msg void OnDblClickListView(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.



class CADsSearch 
{
#define MAX_SEARCH_PREF  12
public:
	CADsSearch( IADs *pADs );
	CADsSearch();
	virtual ~CADsSearch();
	
	
	HRESULT GetColumn( LPCTSTR pszCol, PADS_SEARCH_COLUMN pCol );
	HRESULT GetColumn( CString &s, CStringList &sList );
	HRESULT GetColumn( UINT nIndex, CStringList &sList );
	HRESULT FreeColumn( PADS_SEARCH_COLUMN pCol );
	HRESULT FormatColumn( PADS_SEARCH_COLUMN pColumn, CStringList &sList );
	HRESULT SetInterface( IUnknown *pUnk );
	PADS_SEARCHPREF_INFO FindSearchPrefence( ADS_SEARCHPREF pref );
	
	
	void AddColumn( LPCTSTR pszColumn );
	HRESULT Execute(LPCTSTR pszFilter );
	HRESULT GetNextRow();
	HRESULT SetSearchLevel( int flag );
	HRESULT SetPageSize( UINT nPageSize );
	HRESULT SetAttribOnly( BOOL bAttribOnly = TRUE);
	HRESULT SetScope( ADS_SCOPEENUM flag );
	void ResetSearchPref() { m_dwPrefCount=0; }

	HRESULT AddSearchPref( PADS_SEARCHPREF_INFO pSearchPrefs, DWORD dwNumPref = 1);
	IDirectorySearch *GetSearchIf() { m_pSearch->AddRef(); return m_pSearch; }

	

protected:
    IDirectorySearch	*m_pSearch;
	ADS_SEARCH_HANDLE	 m_hSearch;
	ADS_SEARCHPREF_INFO  m_srchInfo[MAX_SEARCH_PREF];
	DWORD			     m_dwPrefCount;
	CStringList			 m_sColumnList;
};




#endif // !defined(AFX_DIRECTORYSEARCH_H__8F1F13D1_0728_11D2_A3E8_0080C7D071BF__INCLUDED_)
