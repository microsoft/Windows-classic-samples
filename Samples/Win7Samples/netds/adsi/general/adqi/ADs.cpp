//----------------------------------------------------------------------------
//
//  Microsoft Active Directory 2.5 Sample Code
//
//  Copyright (C) Microsoft Corporation, 1996 - 2000
//
//  File:       ADs.cpp
//
//  Contents:   IADs and IADsPathname usage
//
//
//----------------------------------------------------------------------------

#include "stdafx.h"
#include "ADQI.h"
#include "ADs.h"
#include "ADsContainer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


/////////////////////////////////////////////////////////////////////////////
// CDlgIADs dialog

#define STRING_SEPARATOR _T("----------")

extern ADSIIF  adsiIfs[];

CDlgIADs::CDlgIADs(LPUNKNOWN pUnk, CWnd* pParent /*=NULL*/)
	: CDialog(CDlgIADs::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgIADs)
	m_sADsPath = _T("");
	m_sClass = _T("");
	m_sName = _T("");
	m_sParent = _T("");
	m_sSchema = _T("");
	m_sGUID = _T("");
	//}}AFX_DATA_INIT

	///////////////////////////////////////
	// Get the IADs pointer and save it 
	/////////////////////////////////////
	HRESULT hr;
	m_pADs = NULL;
	hr = pUnk->QueryInterface( IID_IADs, (void **) &m_pADs );
	if ( !SUCCEEDED(hr) )
	{
		AfxMessageBox(_T("Fatal Error! QI for IADs failed"));
		return;
	}
	pUnk->Release();
	

}


CDlgIADs::~CDlgIADs()
{
  if ( m_pADs )
  {
	  m_pADs->Release();
  }
}

void CDlgIADs::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgIADs)
	DDX_Control(pDX, IDC_ATTRVALUE, m_cValueList);
	DDX_Control(pDX, IDC_ATTRLIST, m_cAttrList);
	DDX_Text(pDX, IDC_ADSPATH, m_sADsPath);
	DDX_Text(pDX, IDC_CLASS, m_sClass);
	DDX_Text(pDX, IDC_NAME, m_sName);
	DDX_Text(pDX, IDC_PARENT, m_sParent);
	DDX_Text(pDX, IDC_SCHEMA, m_sSchema);
	DDX_Text(pDX, IDC_GUID, m_sGUID);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgIADs, CDialog)
	//{{AFX_MSG_MAP(CDlgIADs)
	ON_BN_CLICKED(IDC_GET, OnGet)
	ON_BN_CLICKED(IDC_GETEX, OnGetEx)
	ON_BN_CLICKED(IDC_GETINFO, OnGetInfo)
	ON_BN_CLICKED(IDC_SETINFO, OnSetInfo)
	ON_BN_CLICKED(IDC_GETINFOEX, OnGetInfoex)
	ON_BN_CLICKED(IDC_PUT, OnPut)
	ON_BN_CLICKED(IDC_PUTEX, OnPutEx)
	ON_BN_CLICKED(IDC_PARENTPATH, OnParentPath)
	ON_BN_CLICKED(IDC_SCHEMAPATH, OnSchemaPath)
	ON_BN_CLICKED(IDC_BINDGUID, OnBindGuid)
	ON_CBN_SELCHANGE(IDC_ATTRLIST, OnSelChangeAttrList)
	ON_LBN_DBLCLK(IDC_ATTRVALUE, OnDblClkAttrValue)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_COPY, OnCopy)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgIADs message handler


BOOL CDlgIADs::OnInitDialog() 
{
	HRESULT hr;
	VARIANT var;
	BSTR    bstr;
	CStringList sList;

	CDialog::OnInitDialog();


	ASSERT( m_pADs );
	VariantInit(&var);
	//////////////////////////
	// Get the IADs Properties
	/////////////////////////	

	// Path
	hr = m_pADs->get_ADsPath( &bstr );
	if ( SUCCEEDED(hr) )
	{
	   m_sADsPath = bstr;
	   SysFreeString(bstr);
	}

	// Name
	hr = m_pADs->get_Name( &bstr );
	if ( SUCCEEDED(hr) )
	{
	   m_sName = bstr;
	   SysFreeString(bstr);
	}

	// Parent
	hr = m_pADs->get_Parent( &bstr );
	if ( SUCCEEDED(hr) )
	{
	   m_sParent = bstr;
	   SysFreeString(bstr);
	}

	// Schema
	hr = m_pADs->get_Schema( &bstr );
	if ( SUCCEEDED(hr) )
	{
	   m_sSchema = bstr;
	   SysFreeString(bstr);
	}

    // Class
	hr = m_pADs->get_Class( &bstr );
	if ( SUCCEEDED(hr) )
	{
	   m_sClass = bstr;
	   SysFreeString(bstr);
	}

	// GUID
	hr = m_pADs->get_GUID( &bstr );
	if ( SUCCEEDED(hr) )
	{
	   m_sGUID = bstr;
	   SysFreeString(bstr);
	}

	if ( m_sGUID.GetLength() == 0 )
	{
		GetDlgItem(IDC_BINDGUID)->EnableWindow( FALSE );
	}

	///////////////////////////////////////////////////////////////
	//
	// Enumerate all attributes in this object
	// Steps:
	//  1. Get the object's schema information
	//  2. Bind to that schema
	//  3. QI for Class
	//  4. Enumerate the Mandatory and Optional Attributes
	//
	/////////////////////////////////////////////////////////////
	USES_CONVERSION;
	IADsClass *pClass;
	hr = App->ADsOpenObject(T2OLE(m_sSchema), IID_IADsClass, (void**) &pClass );
	if ( SUCCEEDED(hr) )
	{
	    m_cAttrList.ResetContent(); // Clear the list box content.

	   // Mandatory
	   pClass->get_MandatoryProperties( &var );
	   VariantToStringList(  var, sList );
	   PopulateComboBoxFromStringList( m_cAttrList, sList );
	   VariantClear(&var);

	   //  A separator to seperate mandatory and optional attributes
	   m_cAttrList.AddString( STRING_SEPARATOR);

	   // Optional
	   pClass->get_OptionalProperties( &var );
	   VariantToStringList(  var, sList );
	   PopulateComboBoxFromStringList( m_cAttrList, sList );
	   VariantClear(&var);
	}


	UpdateData(FALSE);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgIADs::OnGet() 
{
   CString sAttr;
   CStringList sList;
   VARIANT var;
   HRESULT hr;
   m_cAttrList.GetLBText( m_cAttrList.GetCurSel(), sAttr );



   if ( sAttr == STRING_SEPARATOR )
   {
	   MessageBeep(0);
	   return;
   }

   m_cValueList.ResetContent();


   USES_CONVERSION;
   /////////////////////////////
   // Get the attribute
   ////////////////////////////////
   hr = m_pADs->Get( T2OLE(sAttr), &var);
   

   ////////////////////////////////
   // UI: populate the listbox
   ////////////////////////////////
   if ( SUCCEEDED(hr) )
   {
		
		hr = VariantToStringList(  var, sList );
   
		if ( SUCCEEDED(hr) )
		{
			PopulateListBoxFromStringList( m_cValueList, sList );
		}


		VariantClear(&var);
   }
   else
   {
	   CString s;
	   s = GetErrorMessage( hr );
	   m_cValueList.AddString( s );
   }
	
}

////////////////////////////////////////////////////////////////////////////////
// IADs::GetEx is similar to Get. The GetEx always return an array of variant
// regardless if the attribute is single or multi value.
// This is useful for VB/VBS environment. 
////////////////////////////////////////////////////////////////////////////////
void CDlgIADs::OnGetEx() 
{

   CString sAttr;
   CStringList sList;
   VARIANT var;
   HRESULT hr;
   m_cAttrList.GetLBText( m_cAttrList.GetCurSel(), sAttr );
   m_cValueList.ResetContent();

   USES_CONVERSION;
   /////////////////////////////
   // GetEx the attribute
   ////////////////////////////////
   hr = m_pADs->GetEx( T2OLE(sAttr), &var);
   

   ////////////////////////////////
   // UI: populate the listbox
   ////////////////////////////////
   if ( SUCCEEDED(hr) )
   {
		
		hr = VariantToStringList(  var, sList );
   
		if ( SUCCEEDED(hr) )
		{
			PopulateListBoxFromStringList( m_cValueList, sList );
		}

		VariantClear(&var);
   }

	
}

//////////////////////////////////////////////////////
// GetInfo is to refresh the cache
// All attributes in this object will be sent
// to the client and be cached again
//
/////////////////////////////////////////////////////////
void CDlgIADs::OnGetInfo() 
{
	HRESULT hr;
	ASSERT(m_pADs);

	hr = m_pADs->GetInfo();	
	if (!SUCCEEDED(hr) )
	{
		AfxMessageBox(GetErrorMessage(hr));
		return;
	}
	else
	{
		AfxMessageBox(_T("Succeed"));
	}
}

/////////////////////////////////////////////////////////////
// SetInfo will commit any changes since the last SetInfo	//
//															//
//////////////////////////////////////////////////////////////
void CDlgIADs::OnSetInfo() 
{
	HRESULT hr;
	ASSERT(m_pADs);

	hr = m_pADs->GetInfo();	
	if (!SUCCEEDED(hr) )
	{
		AfxMessageBox(GetErrorMessage(hr));
		return;
	}
	else
	{
		AfxMessageBox(_T("Succeed"));
	}

	
}

//////////////////////////////////////////////////
// Similar to GetInfo. Unlike GetInfo, GetInfoEx
// only brings the specified attributes to the client
// This is for an optimization purpose.
///////////////////////////////////////////////////
void CDlgIADs::OnGetInfoex() 
{
	CGetInfoExDlg dlg;
	VARIANT var;
	HRESULT hr;

	if ( dlg.DoModal() == IDOK )
	{
		CString s;
		CString sAttr;
		CStringList sList;
		int idx;

		s = dlg.GetAttribute();
		//////////////////////////////////////////
		// Parse and put them in the string list
		//////////////////////////////////////////
		idx = s.Find(_T(","));
		while( idx != -1 )
		{
		   sAttr = s.Mid(0, idx );
		   sAttr.TrimLeft(); sAttr.TrimRight();
		   sList.AddTail( sAttr );
		   s = s.Mid( idx + 1 );
		   idx = s.Find(_T(","));
		}
		s.TrimLeft(); s.TrimRight();
		sList.AddTail( s );

		/////////////////////////////////
		// Convert stringlist to variant
		////////////////////////////////
		hr = StringListToVariant( var, sList);
		if ( SUCCEEDED(hr) )
		{
			///////////////////////////////
			// Now do the real work!!
			///////////////////////////////
			hr = m_pADs->GetInfoEx( var, 0 );
			if (!SUCCEEDED(hr) )
			{
				AfxMessageBox(GetErrorMessage(hr));
			}
			VariantClear(&var);
		}



	}
	
}


void CDlgIADs::OnPut() 
{
	// TODO: Add your control notification handler code here
	
}

void CDlgIADs::OnPutEx() 
{
	// TODO: Add your control notification handler code here
	
}












/////////////////////////////////////////////////////////////////////////////
// CGetInfoExDlg dialog


CGetInfoExDlg::CGetInfoExDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CGetInfoExDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CGetInfoExDlg)
	m_sAttrText = _T("");
	//}}AFX_DATA_INIT
}


void CGetInfoExDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGetInfoExDlg)
	DDX_Text(pDX, IDC_ATTR, m_sAttrText);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CGetInfoExDlg, CDialog)
	//{{AFX_MSG_MAP(CGetInfoExDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGetInfoExDlg message handlers

void CGetInfoExDlg::OnOK() 
{
	UpdateData(TRUE);
	m_sAttr = m_sAttrText;
	CDialog::OnOK();
}




void CDlgIADs::OnParentPath() 
{
	  HRESULT hr;
	  UpdateData(TRUE); // Retrieve from UI


      USES_CONVERSION;
	  IUnknown *pUnk;
	  CWaitCursor wait;
	  
	  hr = App->ADsOpenObject( T2OLE( m_sParent ), IID_IUnknown, (void**) &pUnk ); 
	  
	  /////////////////////////////////////
	  // Bring up the IADsContainer Dialog
	  ///////////////////////////////////////
	  if ( SUCCEEDED(hr) )
	  {
		  pUnk->AddRef();
		  CDlgIADsContainer dlg( pUnk, this );	  
		  dlg.DoModal();

		  pUnk->Release();
	  }
	  else
	  {
		  AfxMessageBox( GetErrorMessage(hr) );
	  }

	
}


void CDlgIADs::OnSchemaPath() 
{
	  HRESULT hr;
	  UpdateData(TRUE); // Retrieve from UI


      USES_CONVERSION;
	  IUnknown *pUnk;
	  IADs	   *pADs;
	  BSTR      bstr;
	  CWaitCursor wait;

	  
	  hr = App->ADsOpenObject( T2OLE( m_sSchema ), IID_IADs, (void**) &pADs ); 
	  RETURN_ON_FAILURE(hr);
	  

	  hr = pADs->get_Parent( &bstr );
	  pADs->Release();

	  RETURN_ON_FAILURE(hr);

	  hr = App->ADsOpenObject( bstr, IID_IUnknown, (void**) &pUnk ); 
	  SysFreeString( bstr );

	  /////////////////////////////////////
	  // Bring up the IADsContainer Dialog
	  ///////////////////////////////////////
	  if ( SUCCEEDED(hr) )
	  {
		  pUnk->AddRef();
		  CDlgIADsContainer dlg( pUnk, this );	  
		  dlg.DoModal();

		  pUnk->Release();
	  }


	
}

//////////////////////////////////////////////////////
//
// GUID Binding: Only works with Active Directory
//
////////////////////////////////////////////////////////
void CDlgIADs::OnBindGuid() 
{
  CWaitCursor wait;
  CComBSTR bstrPath;
  IUnknown *pUnk = NULL;
  HRESULT hr;
  UpdateData(TRUE); // Retrieve from UI


  
  CComPtr<IADsPathname> pPathname=NULL;
  BSTR bstr;
  BSTR bstrProvider;
  BSTR bstrServer;
  

  hr = CoCreateInstance( CLSID_Pathname, NULL, CLSCTX_INPROC_SERVER, IID_IADsPathname, (void**) &pPathname );

  RETURN_ON_FAILURE(hr);


  hr = m_pADs->get_ADsPath(&bstr);

  RETURN_ON_FAILURE(hr);

  
  /////////////////////////////////////////////////
  // Usage: PathName to find out the provider 
  ///////////////////////////////////////////////////
  hr = pPathname->Set( bstr, ADS_SETTYPE_FULL );
  SysFreeString( bstr );
  RETURN_ON_FAILURE(hr);

  hr = pPathname->Retrieve(ADS_FORMAT_PROVIDER, &bstrProvider );
  RETURN_ON_FAILURE(hr);

  if ( wcscmp( bstrProvider, L"LDAP") != 0 )
  {
	  AfxMessageBox(_T("Only LDAP: (Active Directory Provider) knows about <GUID=xxxx> syntax"));
	  SysFreeString( bstrProvider );
	  return;
  }

  /////////////////////////////////////////////////
  // Usage: PathName to find out the server
  ///////////////////////////////////////////////////
  hr = pPathname->Retrieve(ADS_FORMAT_SERVER, &bstrServer );

  if ( hr == E_ADS_BAD_PATHNAME ) // Serverless binding ( can work only from DS aware client )
  {
	  bstrServer = SysAllocString(L"");
  }

  
  

 
  ///////////////////////////////////////////////
  // Now build the LDAP://server/<GUID=xxx
  //////////////////////////////////////////////


  bstrPath = bstrProvider;
  bstrPath.Append(_T("://"));
  if ( wcslen( bstrServer ) ) // Server based binding ?
  {
     bstrPath.AppendBSTR( bstrServer );
	 bstrPath.Append(_T("/"));
	 SysFreeString( bstrServer );
  }
  bstrPath.Append(_T("<GUID="));
  bstrPath.Append( m_sGUID );
  bstrPath.Append(_T(">") );

  // we don't need these bstrs anymore
  SysFreeString( bstrProvider );
  


  // Now Bind using GUID
  
  hr = App->ADsOpenObject( bstrPath, IID_IUnknown, (void**) &pUnk ); 
	  
  /////////////////////////////////////
  // Bring up the IADs Dialog
  ///////////////////////////////////////
  if ( SUCCEEDED(hr) )
  {
	  pUnk->AddRef();
	  CDlgIADs dlg( pUnk, this );	  
	  dlg.DoModal();
	  pUnk->Release();
  }
  else
  {
	  AfxMessageBox( GetErrorMessage(hr) );
	  return;
  }
	  
	
}

void CDlgIADs::OnSelChangeAttrList() 
{
	OnGet();
}

void CDlgIADs::OnDblClkAttrValue() 
{
	int idx;
	HRESULT hr;

	idx = m_cValueList.GetCurSel();
	if ( idx == LB_ERR )
	{
		return;
	}

	// If someone click this it may be an interface that has the UI.
	// The interface value start with --> in the UI
	CString s;
	int xx;
	CString sArrow = ARROW_SYMBOL;
	m_cValueList.GetText( idx, s );
    
	if ( s.Find(sArrow,0 ) == 0 )
	{
		s = s.Mid( sArrow.GetLength() );

		// Find the appropriate dialog box
		xx=0;
		while( !IsEqualIID( *adsiIfs[xx].pIID, IID_NULL ) && s != adsiIfs[xx].szIf  )
		{
		   xx++;
		}

	 
	    if ( adsiIfs[xx].pFn )
		{
			
			CString sAttr;
			VARIANT var;

			// We need to get the value again
			USES_CONVERSION;
			m_cAttrList.GetLBText( m_cAttrList.GetCurSel(), sAttr );
			hr = m_pADs->Get( T2OLE(sAttr), &var);
			if ( SUCCEEDED(hr) )
			{
		      IUnknown *pUnk;
			  hr = V_DISPATCH(&var)->QueryInterface( IID_IUnknown, (void**) &pUnk );
			  if ( SUCCEEDED(hr) )
			  {
		          (*adsiIfs[xx].pFn)( pUnk, NULL );
			  }
			}
		}
	    else
		{
	       AfxMessageBox(_T("No UI implemented yet"));
		}



		return;

	}
	


}

/////////////////////////////////
// Copy to cliboard
/////////////////////////////////
void CDlgIADs::OnCopy() 
{
	CString s;
	HANDLE hMem;
	int idx;
	

	idx = m_cValueList.GetCurSel();

	if ( idx == LB_ERR )
	{
		return;
	}

	m_cValueList.GetText( idx, s );

	if ( !OpenClipboard() )
	{
		return;
	}
	EmptyClipboard();

	size_t buffSizeInChar = s.GetLength() + 1;
	hMem = GlobalAlloc( GMEM_DDESHARE, buffSizeInChar * sizeof(TCHAR) );
	if ( hMem )
	{
		LPTSTR pMem;
		pMem = (LPTSTR) GlobalLock( hMem );
		_tcsncpy_s( pMem, buffSizeInChar , s, s.GetLength()+1);
		GlobalUnlock(hMem);
		SetClipboardData( CF_TEXT, hMem );
	}

	CloseClipboard();
	
}
