//----------------------------------------------------------------------------
//
//  Microsoft Active Directory 2.5 Sample Code
//
//  Copyright (C) Microsoft Corporation, 1996 - 2000
//
//  File:       DirectorySearch.cpp
//
//  Contents:   IDirectorySearch usage
//
//
//----------------------------------------------------------------------------

#include "stdafx.h"
#include "ADQI.h"
#include "DirectorySearch.h"
#include "ads.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgIDirectorySearch dialog


CDlgIDirectorySearch::CDlgIDirectorySearch(LPUNKNOWN pUnk, CWnd* pParent /*=NULL*/)
	: CDialog(CDlgIDirectorySearch::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgIDirectorySearch)
	m_sFilter = _T("");
	m_bEnableFilter = FALSE;
	m_nTimeOut = 0;
	m_bCacheResult = FALSE;
	m_nPageSize = 0;
	m_sSortOn = _T("");
	m_nTimeLimit = 0;
	m_nSizeLimit = 0;
	m_bAsynch = FALSE;
	m_bAttrib = FALSE;
	m_nDeref = -1;
	//}}AFX_DATA_INIT
	HRESULT hr;
	m_pSearch = NULL;
	hr = pUnk->QueryInterface( IID_IDirectorySearch, (void **) &m_pSearch );
	if ( !SUCCEEDED(hr) )
	{
		AfxMessageBox(_T("Fatal Error! QI for IDirectorySearch failed"));
		return;
	}
	pUnk->Release();



}


CDlgIDirectorySearch::~CDlgIDirectorySearch()
{
	if ( m_pSearch )
	{
		m_pSearch->Release();
	}
}

void CDlgIDirectorySearch::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgIDirectorySearch)
	DDX_Control(pDX, IDC_ATTRIBUTE, m_cAttrList);
	DDX_Control(pDX, IDC_LISTVIEW, m_cListView);
	DDX_Control(pDX, IDC_CHASEREFERRAL, m_cChaseReferrals);
	DDX_Control(pDX, IDC_CACHERESULTS, m_cCacheResult);
	DDX_Control(pDX, IDC_SEARCHSCOPE, m_CADsSearchScope);
	DDX_Control(pDX, IDC_ENABLED, m_cEnableFilter);
	DDX_Control(pDX, IDC_EXECUTE, m_cExecute);
	DDX_Control(pDX, IDC_FILTER, m_cFilter);
	DDX_Text(pDX, IDC_FILTER, m_sFilter);
	DDX_Check(pDX, IDC_ENABLED, m_bEnableFilter);
	DDX_Text(pDX, IDC_TIMEOUT, m_nTimeOut);
	DDX_Check(pDX, IDC_CACHERESULTS, m_bCacheResult);
	DDX_Text(pDX, IDC_PAGESIZE, m_nPageSize);
	DDX_Text(pDX, IDC_SORTON, m_sSortOn);
	DDX_Text(pDX, IDC_TIMELIMIT, m_nTimeLimit);
	DDX_Text(pDX, IDC_SIZELIMIT, m_nSizeLimit);
	DDX_Check(pDX, IDC_ASYNCH, m_bAsynch);
	DDX_Check(pDX, IDC_ATTRIBONLY, m_bAttrib);
	DDX_CBIndex(pDX, IDC_DEREF, m_nDeref);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgIDirectorySearch, CDialog)
	//{{AFX_MSG_MAP(CDlgIDirectorySearch)
	ON_BN_CLICKED(IDC_EXECUTE, OnExecute)
	ON_EN_CHANGE(IDC_FILTER, OnChangeFilter)
	ON_BN_CLICKED(IDC_ENABLED, OnEnabled)
	ON_NOTIFY(NM_DBLCLK, IDC_LISTVIEW, OnDblClickListView)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgIDirectorySearch message handlers
 //

////////////////////////////////////////////
// OnExecute
// Demonstrate:
// IDirectorySearch::ExecuteSearch
// IDirectorySearch::GetNextRow
// IDirectorySearch::GetColumn
// IDirectorySearch::SetSearchPreference
//
/////////////////////////////////////////////
void CDlgIDirectorySearch::OnExecute() 
{
	ASSERT( m_pSearch );
	CWaitCursor wait;
	
	UpdateData(TRUE); // Get data from the Dialog Box
	HRESULT hr;
	ADS_SEARCH_HANDLE hSearch;
	ADS_SEARCH_COLUMN col;
	CString s;
	int idx=0;
	int nCount;
	LPWSTR *pszAttr=NULL;
	POSITION pos;
	USES_CONVERSION;


	


	/////////////////////////////////
	// Reset the Total Number
	//////////////////////////////////
	SetDlgItemText( IDC_TOTAL, _T(""));


	/////////////////////////////////////////////
	// Get the attribute list, and preparing..
	///////////////////////////////////////////
	CStringList sAttrList;
	m_cListView.DeleteAllItems(); // Reset the UI

    while( m_cListView.DeleteColumn(0))
	{
		;
	}

	//////////////////////////////////////////////////
	// Preparing for attribute list
	// and columns to display
	CString sTemp;
	m_cAttrList.GetWindowText(s);

	// we need to add adspath, so that we can refer to this object later when user dblclk the item
	if ( !s.IsEmpty() )
	{
		sTemp = s;
		sTemp.MakeLower();
		if ( s.Find(_T("adspath"),0) == -1 )
		{
			s += _T(",ADsPath");
		}
	}

	// convert to string list for easy manipulation
	StringToStringList( s, sAttrList );



	nCount = sAttrList.GetCount();
	idx=0;
	if ( nCount )
	{
		
		pszAttr = (LPWSTR*) AllocADsMem( nCount * sizeof(LPWSTR));
	
		pos = sAttrList.GetHeadPosition();
		while ( pos != NULL )
		{
			s = sAttrList.GetAt(pos);
			pszAttr[idx] = T2OLE(s);
			sAttrList.GetNext(pos );
			idx++;
		}
	}
	else
	{
		nCount = -1;
	}






	/////////////////////////////////////////
	// BEGIN  Set the preferences
	///////////////////////////////////////
	DWORD dwCountPref = 0;
	ADS_SEARCHPREF_INFO prefInfo[ MAX_SEARCH_PREF ];
	ADS_SORTKEY *pSortKey = NULL;

	if ( m_bEnableFilter == FALSE )
	{
		// Reset the preference
		m_pSearch->SetSearchPreference( prefInfo, dwCountPref );
	}
	else // Preferences are specified
	{
		////////////////////////
		// Timeout Pref
		////////////////////////////
		if ( m_nTimeOut != 0 )
		{
			prefInfo[dwCountPref].dwSearchPref = ADS_SEARCHPREF_TIMEOUT;
			prefInfo[dwCountPref].vValue.dwType = ADSTYPE_INTEGER;
			prefInfo[dwCountPref].vValue.Integer = m_nTimeOut;
			dwCountPref++;
		}

		//////////////////////////////
		// Search Scope
		/////////////////////////////
		idx = m_CADsSearchScope.GetCurSel();
		if ( idx != CB_ERR )
		{
			prefInfo[dwCountPref].dwSearchPref = ADS_SEARCHPREF_SEARCH_SCOPE;
			prefInfo[dwCountPref].vValue.dwType = ADSTYPE_INTEGER;
			prefInfo[dwCountPref].vValue.Integer = idx;
			dwCountPref++;
		}



		///////////////////////////////////////////////////
		// Cache Result. The default is to cache the result
		/////////////////////////////////////////////////
		if ( !m_bCacheResult )
		{
			prefInfo[dwCountPref].dwSearchPref = ADS_SEARCHPREF_SEARCH_SCOPE;
			prefInfo[dwCountPref].vValue.dwType = ADSTYPE_BOOLEAN;
			prefInfo[dwCountPref].vValue.Boolean = FALSE;
			dwCountPref++;
		}

		//////////////////////////////////////////////////
		// Page Size
		///////////////////////////////////////////////////
		if ( m_nPageSize > 0 )
		{
			prefInfo[dwCountPref].dwSearchPref = ADS_SEARCHPREF_PAGESIZE;
			prefInfo[dwCountPref].vValue.dwType = ADSTYPE_INTEGER;;
			prefInfo[dwCountPref].vValue.Integer = m_nPageSize;
			dwCountPref++;
		}


		////////////////////////////////////////////////
		// Chase Referrals
		///////////////////////////////////////////////
		idx = m_cChaseReferrals.GetCurSel();
		if ( idx != CB_ERR )
		{
			prefInfo[dwCountPref].dwSearchPref = ADS_SEARCHPREF_CHASE_REFERRALS;
			prefInfo[dwCountPref].vValue.dwType = ADSTYPE_INTEGER;
			switch( idx )
			{
			case 0:
				 prefInfo[dwCountPref].vValue.Integer = ADS_CHASE_REFERRALS_NEVER; 
				 break;
			case 1:
				 prefInfo[dwCountPref].vValue.Integer = ADS_CHASE_REFERRALS_SUBORDINATE;
				 break;
			case 2:
				 prefInfo[dwCountPref].vValue.Integer = ADS_CHASE_REFERRALS_EXTERNAL;
				 break;
			default:
				 prefInfo[dwCountPref].vValue.Integer = ADS_CHASE_REFERRALS_EXTERNAL;
				 
			}
			
			dwCountPref++;
		}


		///////////////////////////////////////////////
		// Sort On
		////////////////////////////////////////////////
		if ( !m_sSortOn.IsEmpty() )
		{
			CStringList sList;
			UINT nCount;
			prefInfo[dwCountPref].dwSearchPref = ADS_SEARCHPREF_SORT_ON;
			prefInfo[dwCountPref].vValue.dwType = ADSTYPE_PROV_SPECIFIC;
  			StringToStringList( m_sSortOn, sList );

			nCount = sList.GetCount();
			if ( nCount >  0 )
			{
				POSITION pos;
				pos= sList.GetHeadPosition();
				pSortKey = (ADS_SORTKEY *) LocalAlloc( LMEM_FIXED | LMEM_ZEROINIT, sizeof(ADS_SORTKEY) * nCount );
				idx = 0;
				while( pos != NULL )
				{
					pSortKey[idx].pszAttrType = T2OLE(sList.GetAt(pos));
					pSortKey[idx].pszReserved = NULL;
					pSortKey[idx].fReverseorder =0;

					// Next
					idx++;
					sList.GetNext( pos );
				}
				
				prefInfo[dwCountPref].vValue.ProviderSpecific.dwLength = sizeof(ADS_SORTKEY) * nCount;
				prefInfo[dwCountPref].vValue.ProviderSpecific.lpValue = (LPBYTE) pSortKey;
				dwCountPref++;
			}

		}

		//////////////////////////////////////////////
		// Size Limit
		//////////////////////////////////////////////
		if ( m_nSizeLimit > 0 )
		{
            prefInfo[dwCountPref].dwSearchPref = ADS_SEARCHPREF_SIZE_LIMIT;
            prefInfo[dwCountPref].vValue.dwType = ADSTYPE_INTEGER;
            prefInfo[dwCountPref].vValue.Integer = m_nSizeLimit;
			dwCountPref++;
		}


		//////////////////////////////////////////////////
		// A Synchronous
		///////////////////////////////////////////////////
		if ( m_bAsynch )
		{
			prefInfo[dwCountPref].dwSearchPref = ADS_SEARCHPREF_ASYNCHRONOUS;
            prefInfo[dwCountPref].vValue.dwType = ADSTYPE_BOOLEAN;
            prefInfo[dwCountPref].vValue.Integer = TRUE;
			dwCountPref++;

		}

		/////////////////////////////////////////////////////
		//  Attribute Type Only
		//////////////////////////////////////////////////////
		if ( m_bAttrib )
		{
			prefInfo[dwCountPref].dwSearchPref = ADS_SEARCHPREF_ATTRIBTYPES_ONLY;
            prefInfo[dwCountPref].vValue.dwType = ADSTYPE_BOOLEAN;
            prefInfo[dwCountPref].vValue.Integer = TRUE;
			dwCountPref++;

		}


		/////////////////////////////////////////////////////
		//  Derefence Alias
		//////////////////////////////////////////////////////
		if ( m_nDeref != CB_ERR )
		{
			prefInfo[dwCountPref].dwSearchPref = ADS_SEARCHPREF_DEREF_ALIASES;
            prefInfo[dwCountPref].vValue.dwType = ADSTYPE_INTEGER;
            prefInfo[dwCountPref].vValue.Integer = m_nDeref;
			dwCountPref++;


		}



		///////////////////////////////////////////////
		// Now Set the selected preferences
		//////////////////////////////////////////////
		hr = m_pSearch->SetSearchPreference( prefInfo, dwCountPref );


		
	}

	/////////////////////////////////////////
	// END  Set the preferences
	///////////////////////////////////////





	////////////////////////////////////////
	// Execute the Search
	//////////////////////////////////////
	DWORD dwCount=0;

	hr = m_pSearch->ExecuteSearch(T2OLE(m_sFilter), pszAttr, nCount, &hSearch );


	////////////////////////////////
	//// cleanup
	////////////////////////////////
	if ( pszAttr ) 
	{
  		FreeADsMem( pszAttr );
	}
	
	if ( pSortKey )
	{
		LocalFree( pSortKey );
	}




	if ( !SUCCEEDED(hr) )
	{
		AfxMessageBox(GetErrorMessage(hr));
		return;
	}

	////////////////////////////////////////////////////////
	// Enumerate the rows
	////////////////////////////////////////////////////////

	sAttrList.RemoveAll();

	

	
	/////////////////////////////////////////////////////////
	// Retrieve the column names returned from the server
	///////////////////////////////////////////////////////////
	LPWSTR pszColumn;
	hr = m_pSearch->GetFirstRow( hSearch );

	if ( !SUCCEEDED(hr) )
	{
		return;
	}
	
	

	idx=0;
	while( (hr=m_pSearch->GetNextColumnName( hSearch, &pszColumn )) != S_ADS_NOMORE_COLUMNS )
	{
		s = OLE2T( pszColumn );
		m_cListView.InsertColumn(idx, s, LVCFMT_LEFT, 150 ); // adding columns to the UI	
		sAttrList.AddTail(s);
		FreeADsMem( pszColumn );
		idx++;
	}



	/////////////////////////////////////////////
	// Start iterating the result set
	////////////////////////////////////////////
	int nCol;
	CStringList sValueList;

    do 
	{
		nCol=0;
		pos = sAttrList.GetHeadPosition();

		while( pos != NULL )
		{
		
		    s = sAttrList.GetAt(pos); //column name

			// Get the Name and display it in the list 
			hr = m_pSearch->GetColumn( hSearch, T2OLE(s), &col );
			if ( SUCCEEDED(hr) )
			{
				s =_T("");
				
				if ( col.dwADsType != ADSTYPE_INVALID ) // if we request for attrib only, no value will be returned
				{
					ADsToStringList(col.pADsValues, col.dwNumValues, sValueList );
					StringListToString( sValueList, s );
				}
	
				if ( nCol )
				{
					m_cListView.SetItemText(0,nCol, s);
				}
				else
				{
				   m_cListView.InsertItem(0, s);
				}
				
				m_pSearch->FreeColumn( &col );
			}


			

			nCol++;
			sAttrList.GetNext(pos);

		}
		dwCount++;
		/////////////////////////////
		//Display the total so far
		////////////////////////////////
		s.Format("%ld object(s)", dwCount );
		SetDlgItemText( IDC_TOTAL, s );
	
	} 
	while( (hr=m_pSearch->GetNextRow( hSearch)) != S_ADS_NOMORE_ROWS );	 

	

	m_pSearch->CloseSearchHandle( hSearch ); 



	


}

void CDlgIDirectorySearch::OnChangeFilter() 
{
	BOOL bExecEnabled;

	bExecEnabled = m_cFilter.GetWindowTextLength() > 0 ? TRUE :FALSE;	
	m_cExecute.EnableWindow( bExecEnabled );
}

BOOL CDlgIDirectorySearch::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// Initialize to a default search
	m_cFilter.SetWindowText(_T("(objectClass=*)"));
	m_cAttrList.SetWindowText(_T("cn"));
	
	OnChangeFilter();
	OnEnabled(); 

	m_cCacheResult.SetCheck( 1 );

	



	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgIDirectorySearch::OnEnabled() 
{
   UpdateData(TRUE);

   // Enable/Disable Edit Boxes
   GetDlgItem( IDC_TIMEOUT )->EnableWindow(m_bEnableFilter);
   GetDlgItem( IDC_SEARCHSCOPE )->EnableWindow(m_bEnableFilter);
   GetDlgItem( IDC_PAGESIZE )->EnableWindow(m_bEnableFilter);
   GetDlgItem( IDC_SORTON )->EnableWindow(m_bEnableFilter);
   GetDlgItem( IDC_CHASEREFERRAL )->EnableWindow(m_bEnableFilter);
   GetDlgItem( IDC_TIMELIMIT )->EnableWindow(m_bEnableFilter);
   GetDlgItem( IDC_SIZELIMIT )->EnableWindow(m_bEnableFilter);
   GetDlgItem( IDC_ASYNCH )->EnableWindow(m_bEnableFilter);
   GetDlgItem( IDC_CACHERESULTS )->EnableWindow(m_bEnableFilter);
   GetDlgItem( IDC_ATTRIBONLY )->EnableWindow(m_bEnableFilter);
   GetDlgItem( IDC_DEREF )->EnableWindow(m_bEnableFilter);
   
   
   // Enabled/Disabled Texts
   GetDlgItem( IDC_TIMEOUT_TXT )->EnableWindow(m_bEnableFilter);
   GetDlgItem( IDC_SEARCHSCOPE_TXT )->EnableWindow(m_bEnableFilter);
   GetDlgItem( IDC_PAGESIZE_TXT )->EnableWindow(m_bEnableFilter);
   GetDlgItem( IDC_SORTON_TXT )->EnableWindow(m_bEnableFilter);
   GetDlgItem( IDC_CHASEREFERRAL_TXT )->EnableWindow(m_bEnableFilter);
   GetDlgItem( IDC_TIMELIMIT_TXT )->EnableWindow(m_bEnableFilter);
   GetDlgItem( IDC_SIZELIMIT_TXT )->EnableWindow(m_bEnableFilter);
   GetDlgItem( IDC_DEREF_TXT )->EnableWindow(m_bEnableFilter);

   

}

void CDlgIDirectorySearch::OnOK() 
{
	CDialog::OnOK();
}

	


void CDlgIDirectorySearch::OnDblClickListView(NMHDR* pNMHDR, LRESULT* pResult) 
{
	 int idx;
	 *pResult = 0;
	 

	 idx = m_cListView.GetNextItem(-1, LVNI_SELECTED);
	 if ( idx == -1 )
	 {
		 return;
	 }

	 CHeaderCtrl *pHeader;
	 int nCount;
     pHeader = m_cListView.GetHeaderCtrl();
	 if ( !pHeader )
	 {
		 return;
	 }
	 nCount = pHeader->GetItemCount();


	 // Get the text
	 CString s;
	 s = m_cListView.GetItemText( idx, nCount-1 );

	 if ( !s.GetLength() )
	 {
		 return;
	 }

	 USES_CONVERSION;
	 HRESULT hr;
	 IUnknown *pUnk=NULL;
	 CString sUserName;

	 
	  hr = App->ADsOpenObject( T2OLE( s ), IID_IUnknown, (void**) &pUnk );
	
	  
	  /////////////////////////////////////
	  // Bring up the IADs Dialog
	  ///////////////////////////////////////
	  if ( SUCCEEDED(hr) )
	  {
		  pUnk->AddRef();
		  CDlgIADs dlg( pUnk );	  
		  dlg.DoModal();
		  pUnk->Release();
	  }
	  else
	  {
		  AfxMessageBox(GetErrorMessage(hr));
	  }

	

	
}






////////////////////////////////////////////////////////////////////
//
// CADsSearch
// This is a convient C++ wrapper for IDirectorySearch
//
/////////////////////////////////////////////////////////////////////
CADsSearch::CADsSearch( IADs *pADs )
{
	CADsSearch();
	SetInterface( pADs );

}

CADsSearch::CADsSearch()
{
   m_pSearch = NULL;
   m_hSearch = NULL;
   m_dwPrefCount = 0;

}



HRESULT CADsSearch::SetInterface( IUnknown *pUnk )
{
	if ( pUnk != NULL )
	{
		HRESULT hr;
		//////////////////////////////////////////
		// Remove the current search if any
		////////////////////////////////////////
		if ( m_pSearch )
		{
			if ( m_hSearch )
			{
				m_pSearch->CloseSearchHandle( m_hSearch );
			}
			m_pSearch->Release();
		}


		m_pSearch = NULL;
		m_hSearch = NULL;
		m_dwPrefCount = 0;
		hr = pUnk->QueryInterface( IID_IDirectorySearch, (LPVOID*) &m_pSearch );
		pUnk->Release();
		return hr;
	}

	return E_UNEXPECTED;

}




CADsSearch::~CADsSearch()
{
	if ( m_pSearch )
	{
		if ( m_hSearch )
		{
			m_pSearch->CloseSearchHandle( m_hSearch );
		}
		m_pSearch->Release();
	}
}

HRESULT CADsSearch::GetNextRow()
{
	HRESULT hr;
	if ( !m_pSearch || !m_hSearch)
	{
		return E_UNEXPECTED;
	}
	hr = m_pSearch->GetNextRow( m_hSearch );
	return hr;
}

void CADsSearch::AddColumn( LPCTSTR pszColumn )
{
	ASSERT( pszColumn );

	m_sColumnList.AddTail( pszColumn );	
}


HRESULT CADsSearch::SetScope( ADS_SCOPEENUM flag )
{

	if ( !m_pSearch )
	{
		return E_UNEXPECTED;
	}


	ADS_SEARCHPREF_INFO info;

	info.dwSearchPref = ADS_SEARCHPREF_SEARCH_SCOPE;
	info.vValue.dwType = ADSTYPE_INTEGER;
	info.vValue.Integer = flag;
	return AddSearchPref( &info );
	
}


HRESULT CADsSearch::SetAttribOnly( BOOL bAttribOnly )
{
	ADS_SEARCHPREF_INFO info;

	info.dwSearchPref = ADS_SEARCHPREF_ATTRIBTYPES_ONLY;
    info.vValue.dwType = ADSTYPE_BOOLEAN;
    info.vValue.Integer = bAttribOnly;
	
	return AddSearchPref( &info );

}

HRESULT CADsSearch::SetPageSize( UINT nPageSize )
{
	ADS_SEARCHPREF_INFO info;

	info.dwSearchPref = ADS_SEARCHPREF_PAGESIZE;
    info.vValue.dwType = ADSTYPE_INTEGER;
    info.vValue.Integer = nPageSize;

	return AddSearchPref( &info );
}



// TODO: More Options...

HRESULT CADsSearch::AddSearchPref( PADS_SEARCHPREF_INFO pSearchPrefs, DWORD dwNumPref )
{
	PADS_SEARCHPREF_INFO pInfo;

	for( UINT idx=0; idx < dwNumPref; idx++ )
	{
		pInfo = FindSearchPrefence( pSearchPrefs->dwSearchPref );
		if ( pInfo )
		{
			*(pInfo) = *(pSearchPrefs);
		}
		else
		{
			if ( m_dwPrefCount == MAX_SEARCH_PREF )
			{
				return E_FAIL;
			}

			m_srchInfo[m_dwPrefCount] = *(pSearchPrefs);
			m_dwPrefCount++;
		}

		pSearchPrefs++;
	}
	return S_OK;
}

PADS_SEARCHPREF_INFO CADsSearch::FindSearchPrefence( ADS_SEARCHPREF pref )
{
	for(UINT idx=0; idx < MAX_SEARCH_PREF; idx++ )
	{
	    if( m_srchInfo[idx].dwSearchPref == pref )
		{
			 return &m_srchInfo[idx];
		}
	}

	return NULL;
}

HRESULT CADsSearch::Execute( LPCTSTR pszFilter )
{
	USES_CONVERSION;
	HRESULT hr;


	if ( !m_pSearch  )
	{
		return E_UNEXPECTED;
	}
	
	m_hSearch	= NULL;

	//////////////////////////////////////////////
	// Set the search preference if any
	///////////////////////////////////////////////
	if ( m_dwPrefCount )
	{
		hr = m_pSearch->SetSearchPreference( m_srchInfo, m_dwPrefCount);
		if ( !SUCCEEDED(hr) )
		{
			return hr;
		}
	}

	if ( m_sColumnList.IsEmpty() )
	{
		hr = m_pSearch->ExecuteSearch( T2OLE(pszFilter), NULL, -1, &m_hSearch );
	}
	else // Columns is specified
	{
		unsigned int count, xx;
		LPWSTR    *pColumns = NULL;
		POSITION  pos; 
		CString	  s;

		count = m_sColumnList.GetCount();
		pColumns = (LPWSTR*)AllocADsMem( sizeof(LPWSTR)*count);
		pos = m_sColumnList.GetHeadPosition();
		for(xx=0; xx < count; xx++)
		{
			s = m_sColumnList.GetAt(pos);
			pColumns[xx] = T2OLE(s);
			m_sColumnList.GetNext(pos);
		}

		hr = m_pSearch->ExecuteSearch( T2OLE(pszFilter), pColumns, count, &m_hSearch );
		if ( pColumns )
		{
			FreeADsMem( pColumns );
		}

	}


	return hr;

}



HRESULT CADsSearch::GetColumn( UINT nIndex, CStringList &sList )
{
	CString s;

	if ( m_sColumnList.IsEmpty() )
	{
		return E_FAIL;
	}

	POSITION pos;
	pos = m_sColumnList.FindIndex( nIndex );

	if ( pos == NULL )
	{
		return E_FAIL;
	}

	s = m_sColumnList.GetAt(pos);
	return GetColumn( s, sList );

}


HRESULT CADsSearch::GetColumn( LPCTSTR pszCol, PADS_SEARCH_COLUMN pCol )
{
   LPWSTR pszAttrName;
   USES_CONVERSION;

   pszAttrName = T2OLE(pszCol);

   return m_pSearch->GetColumn( m_hSearch, pszAttrName, pCol );

}

HRESULT CADsSearch::FreeColumn( PADS_SEARCH_COLUMN pCol )
{
   return m_pSearch->FreeColumn( pCol );
}



HRESULT CADsSearch::GetColumn(CString &sCol, CStringList &sList )
{
   LPWSTR pszAttrName;
   ADS_SEARCH_COLUMN col;
   HRESULT hr;
   USES_CONVERSION;
   CString s;


   pszAttrName = T2OLE(sCol);

   hr = m_pSearch->GetColumn( m_hSearch, pszAttrName, &col );

   if ( !SUCCEEDED(hr) )
   {
	   return hr;
   }
   
   hr =  FormatColumn( &col, sList );
   m_pSearch->FreeColumn( &col );

   
   return hr;

}


HRESULT CADsSearch::FormatColumn(PADS_SEARCH_COLUMN pColumn, CStringList &sList)
{

	ADsToStringList( pColumn->pADsValues, pColumn->dwNumValues, sList );
    return S_OK;

}



