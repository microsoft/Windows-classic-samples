//----------------------------------------------------------------------------
//
//  Microsoft Active Directory 2.5 Sample Code
//
//  Copyright (C) Microsoft Corporation, 1996 - 2000
//
//  File:       ADsContainer.cpp
//
//  Contents:   IADsContainer usage
//
//
//----------------------------------------------------------------------------

#include "stdafx.h"
#include "ADQI.h"
#include "ads.h"
#include "ADsContainer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif






/////////////////////////////////////////////////////////////////////////////
// CDlgIADsContainer dialog


CDlgIADsContainer::CDlgIADsContainer(LPUNKNOWN pUnk, CWnd* pParent /*=NULL*/)
	: CDialog(CDlgIADsContainer::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgIADsContainer)
	m_sFilter = _T("");
	//}}AFX_DATA_INIT

	///////////////////////////////////////
	// Get the IADsContainer pointer and save it as a member
	/////////////////////////////////////
	HRESULT hr;
	m_pCont = NULL;
	hr = pUnk->QueryInterface( IID_IADsContainer, (void **) &m_pCont );
	if ( !SUCCEEDED(hr) )
	{
		AfxMessageBox(_T("Fatal Error! QI for IADs failed"));
		return;
	}
	pUnk->Release();

}

CDlgIADsContainer::~CDlgIADsContainer()
{
	
	if ( m_pCont )
	{
		 m_pCont->Release();
	}
}


void CDlgIADsContainer::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgIADsContainer)
	DDX_Control(pDX, IDC_CHILDRENLIST, m_cChildList);
	DDX_Text(pDX, IDC_FILTER, m_sFilter);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgIADsContainer, CDialog)
	//{{AFX_MSG_MAP(CDlgIADsContainer)
	ON_BN_CLICKED(IDC_VIEW, OnView)
	ON_LBN_DBLCLK(IDC_CHILDRENLIST, OnDblClkChildrenList)
	ON_BN_CLICKED(IDC_DELETE, OnDelete)
	ON_BN_CLICKED(IDC_RENAME, OnRename)
	ON_BN_CLICKED(IDC_SET, OnSet)
	ON_BN_CLICKED(IDC_MOVE, OnMove)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgIADsContainer message handlers

BOOL CDlgIADsContainer::OnInitDialog() 
{
	
	CDialog::OnInitDialog();	
	EnumerateChildren();
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//////////////////////////////////////////////////////////
// On View demonstrate the IADsContainer::GetObject()
// This is when the user double click the list box or
// when the user select an item in the list and click "View"
/////////////////////////////////////////////////////////////
void CDlgIADsContainer::OnView() 
{
	ASSERT( m_pCont );

	
	
	CString sName;
	CString sClass;
	HRESULT hr;

	
	if ( !GetClassAndName( sClass, sName) )
	{
		return;
	}


	USES_CONVERSION;
	IDispatch *pDisp;
	IUnknown  *pUnk;
	hr = m_pCont->GetObject(T2OLE(sClass), T2OLE(sName), &pDisp );
	if ( !SUCCEEDED(hr) )
	{
		return;
	}

	pDisp->QueryInterface( IID_IUnknown, (void**) &pUnk );
	pDisp->Release();


	/////////////////////////////////
	// Bring up the dialog box
	////////////////////////////////////
	pUnk->AddRef();
	CDlgIADs dlg( pUnk );
	dlg.DoModal();
	pUnk->Release();
	
}

void CDlgIADsContainer::OnDblClkChildrenList() 
{
	OnView();	
}

////////////////////////////////
// IADsContainer::Delete
/////////////////////////////////
void CDlgIADsContainer::OnDelete() 
{
	CString sName;
	CString sClass;
	HRESULT hr;

	
	if ( !GetClassAndName( sClass, sName) )
	{
		return;
	}

	///////////////////////
	// Delete Now
	////////////////////////
	USES_CONVERSION;
	///////////////////////////////////////////////////////////////
	// For example m_pCont->Delete(L"user", L"cn=jsmith");
	////////////////////////////////////////////////////////////////
	hr = m_pCont->Delete( T2OLE( sClass ), T2OLE( sName ) );
	if (!SUCCEEDED(hr) )
	{
		AfxMessageBox( GetErrorMessage(hr));
	    return;
	}
	EnumerateChildren();
}



BOOL CDlgIADsContainer::GetClassAndName(CString & sClass, CString & sName)
{
	int idx;
	POSITION pos;


	idx = m_cChildList.GetCurSel();
	if ( idx == LB_ERR )
	{
		return FALSE;
	}


	m_cChildList.GetText( idx, sName );
	pos = m_sClassList.FindIndex( idx );
	if ( pos == NULL )
	{
	  return FALSE;
	}

	sClass = m_sClassList.GetAt(pos);
    return TRUE;

}


//////////////////////////////////////////////////
// EnumerateChildren Function
// IADsContainer::get_Enum
// ADsBuildEnumerator
// ADsEnumerateNext
// ADsFreeEnumerator
//
///////////////////////////////////////////////
HRESULT CDlgIADsContainer::EnumerateChildren()
{
	HRESULT hr=S_OK;
	VARIANT var;
	ULONG   lFetch;

	m_cChildList.ResetContent();
	m_sClassList.RemoveAll();

	//////////////////////////////////////////////////////
	// Use the ADSI Helper APIs to enumerate Children
	////////////////////////////////////////////////////
	IEnumVARIANT *pEnum;
	IDispatch	 *pDisp;
	IADs		 *pADs;
	BSTR	      bstr;
	CString       s;
	
	VariantInit(&var);
	hr = ADsBuildEnumerator( m_pCont, &pEnum );
	if ( !SUCCEEDED(hr) )
	{
		return S_FALSE;
	}
	
	hr = ADsEnumerateNext( pEnum, 1, &var, &lFetch );
	while( hr == S_OK )
	{		
		if ( lFetch == 1 )
		{
		   	pDisp = V_DISPATCH(&var);
			///////////////////////////
			// Get the Child Name
			/////////////////////////////
			hr = pDisp->QueryInterface( IID_IADs, (void**)&pADs ); 
			if ( SUCCEEDED(hr) )
			{
				pADs->get_Name(&bstr);
				s = bstr;
				m_cChildList.AddString( s );
				SysFreeString(bstr);
				
			
			  ///////////////////////////////////////////////
			  // While we're holding the child's object
			  // let's get the class info for later use. 
			  // The class info will be used for getting the actual object
			  // via IADsContainer::GetObject) when the user wants to view
			  // object in detail. 
			  // We could have stored the ADsPath as well. 
			  //////////////////////////////////////////////
				pADs->get_Class(&bstr);
				s = bstr;
				m_sClassList.AddTail( s );
				SysFreeString(bstr);

				pADs->Release();
			}

			VariantClear(&var);
		}
		hr = ADsEnumerateNext( pEnum, 1, &var, &lFetch );
    };

    


	// Free Enumerator
	ADsFreeEnumerator( pEnum );
    
	return S_OK;


}



/////////////////////////////////////////
// OnRename
// Demonstrate: 
// IADsContainer::MoveHere
// IADsContainer::GetObject
//////////////////////////////////////////////
void CDlgIADsContainer::OnRename() 
{
   CString sName;
   CString sNewName;
   CString sClass;

   if ( !GetClassAndName( sClass,  sName) )
   {
	   return;
   }
   
   CRenameDlg dlg( sName );

   if ( dlg.DoModal() == IDOK )
   {
	  //////////////////////////////////////////////////////////////////
	  // To Rename we need:
	  //  1. A new name ( we'll get it from the rename dialog box )
	  //  2. ADsPath of the object to be renamed 
	  //  3. The container where the child lives.
	  /////////////////////////////////////////////////////////////////

	  sNewName = dlg.GetName();

	  //////////////////////////////////////////////////
	  // Get the child's ADsPath,
	  // First we can use IADsContainer::GetObject
	  /////////////////////////////////////////////////////
	  USES_CONVERSION;
	  IDispatch *pDisp;
	  IADs		*pADs;
	  BSTR		bstrPath;
	  CString	sADsPath;
	  HRESULT   hr;

	  hr = m_pCont->GetObject(T2OLE(sClass), T2OLE(sName), &pDisp );
	  if ( !SUCCEEDED(hr) )
	  {
		  return;
	  }
	  
	  hr = pDisp->QueryInterface( IID_IADs, (void**) &pADs );
	  pDisp->Release();

	  ////////////////////////////////
	  // Now get the child's ADsPath
	  //////////////////////////////////
	  if ( !SUCCEEDED(hr) )
	  {
		  return;
	  }

	  pADs->get_ADsPath( &bstrPath );	  
	  pADs->Release();
	
	  ////////////////////////////////////////////////////
	  // We have all the information, time to rename	//
	  ////////////////////////////////////////////////////
	  hr = m_pCont->MoveHere( bstrPath, T2OLE(sNewName), &pDisp );
	  SysFreeString( bstrPath );
	 
	  if ( !SUCCEEDED(hr) )
	  {
		  AfxMessageBox(GetErrorMessage(hr));
		  return;
	  }
	  pDisp->Release();

	  // Refresh the list
	  EnumerateChildren();
	  
	  
   }
	
}

void CDlgIADsContainer::OnMove() 
{
	CMoveDlg dlg;
	if ( dlg.DoModal() == IDOK )
	{
		CString sADsPath;
		IDispatch *pDisp;
		USES_CONVERSION;
		HRESULT hr;

		sADsPath = dlg.GetObjectPath();
		hr = m_pCont->MoveHere( T2OLE(sADsPath), NULL, &pDisp );
		pDisp->Release();
		RETURN_ON_FAILURE(hr);

		EnumerateChildren(); // Refresh the list

	}
	
}

void CDlgIADsContainer::OnOK() 
{
	m_sClassList.RemoveAll();
	CDialog::OnOK();
}

///////////////////////////////
// IADsContainer::put_Filter
//
void CDlgIADsContainer::OnSet() 
{
	CStringList sList;
	VARIANT var;
	HRESULT hr;
	
	
	UpdateData(TRUE);
	VariantInit(&var);

	// The user may enter many classes to filter 'user,group'
	StringToStringList( m_sFilter, sList );
	StringListToVariant(var, sList );

	hr = m_pCont->put_Filter( var );

	VariantClear(&var);

	if ( !SUCCEEDED(hr) )
	{
		AfxMessageBox(GetErrorMessage(hr));
	    return;
	}

	EnumerateChildren();
	
}





/////////////////////////////////////////////////////////////////////////////
// CRenameDlg dialog


CRenameDlg::CRenameDlg(CString sOldName, CWnd* pParent /*=NULL*/)
	: CDialog(CRenameDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CRenameDlg)
	m_sName = _T("");
	//}}AFX_DATA_INIT
	
	m_sOldName = _T("Rename '");
	m_sOldName += sOldName;
	m_sOldName += _T("'");
	m_sNewName = _T("");
	
}


void CRenameDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRenameDlg)
	DDX_Text(pDX, IDC_NAME, m_sName);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CRenameDlg, CDialog)
	//{{AFX_MSG_MAP(CRenameDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRenameDlg message handlers

void CRenameDlg::OnOK() 
{
	UpdateData(TRUE);
	m_sNewName = m_sName;
	CDialog::OnOK();
}



BOOL CRenameDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	SetWindowText( m_sOldName );
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}



/////////////////////////////////////////////////////////////////////////////
// CMoveDlg dialog


CMoveDlg::CMoveDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMoveDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMoveDlg)
	m_sADsPath = _T("");
	//}}AFX_DATA_INIT
}


void CMoveDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMoveDlg)
	DDX_Text(pDX, IDC_ADSPATH, m_sADsPath);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMoveDlg, CDialog)
	//{{AFX_MSG_MAP(CMoveDlg)
	ON_EN_CHANGE(IDC_ADSPATH, OnChangeADspath)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMoveDlg message handlers

void CMoveDlg::OnOK() 
{
	UpdateData(TRUE); //Retrieve
	CDialog::OnOK();
}


void CMoveDlg::OnChangeADspath() 
{
	BOOL bEnabled;
	bEnabled = GetDlgItem(IDC_ADSPATH)->GetWindowTextLength() > 0 ? TRUE : FALSE;

	GetDlgItem(IDOK)->EnableWindow( bEnabled );

	
}
