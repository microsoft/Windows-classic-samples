//----------------------------------------------------------------------------
//
//  Microsoft Active Directory 2.5 Sample Code
//
//  Copyright (C) Microsoft Corporation, 1996 - 2000
//
//  File:       IDirectoryObject.cpp
//
//  Contents:   IDirectoryObject usage
//
//
//----------------------------------------------------------------------------

#include "stdafx.h"
#include "ADQI.h"
#include "DirectoryObject.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgIDirectoryObject dialog


CDlgIDirectoryObject::CDlgIDirectoryObject(LPUNKNOWN pUnk, CWnd* pParent /*=NULL*/)
	: CDialog(CDlgIDirectoryObject::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgIDirectoryObject)
	m_sDN = _T("");
	m_sRDN = _T("");
	m_sSchema = _T("");
	m_sClass = _T("");
	m_sParent = _T("");
	m_sAttributes = _T("");
	//}}AFX_DATA_INIT

	HRESULT hr;
	m_pDirObject = NULL;
	hr = pUnk->QueryInterface( IID_IDirectoryObject, (void **) &m_pDirObject );
	if ( !SUCCEEDED(hr) )
	{
		AfxMessageBox(_T("Fatal Error! QI for IDirectoryObject failed"));
		return;
	}
	pUnk->Release();

	ShowAttributes();

}


CDlgIDirectoryObject::~CDlgIDirectoryObject()
{
	if ( m_pDirObject )
	{
		m_pDirObject->Release();
	}
}

void CDlgIDirectoryObject::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgIDirectoryObject)
	DDX_Control(pDX, IDC_ATTRIBUTE, m_cAttributes);
	DDX_Control(pDX, IDC_VALUELIST, m_cValueList);
	DDX_Text(pDX, IDC_DN, m_sDN);
	DDX_Text(pDX, IDC_RDN, m_sRDN);
	DDX_Text(pDX, IDC_SCHEMA, m_sSchema);
	DDX_Text(pDX, IDC_CLASS, m_sClass);
	DDX_Text(pDX, IDC_PARENT, m_sParent);
	DDX_Text(pDX, IDC_ATTRIBUTE, m_sAttributes);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgIDirectoryObject, CDialog)
	//{{AFX_MSG_MAP(CDlgIDirectoryObject)
	ON_BN_CLICKED(IDC_DELETE, OnDelete)
	ON_BN_CLICKED(IDC_GET, OnGet)
	ON_EN_CHANGE(IDC_ATTRIBUTE, OnChangeAttribute)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgIDirectoryObject message handlers
// Demonstrate:
// IDirectoryObject::GetObjectInformation()
//
/////////////////////////////////////////////////////////////////////////////

void CDlgIDirectoryObject::ShowAttributes()
{
	ADS_OBJECT_INFO *pInfo;
	HRESULT hr;

	hr = m_pDirObject->GetObjectInformation(&pInfo);
	if (!SUCCEEDED(hr) )
	{
		return;
	}
	
	/////////////////////////////////////////////
	// Show the attributes in the UI
	/////////////////////////////////////////////
	USES_CONVERSION;

	m_sRDN		= OLE2T(pInfo->pszRDN);
	m_sDN		= OLE2T(pInfo->pszObjectDN);
	m_sParent	= OLE2T(pInfo->pszParentDN);
	m_sClass	= OLE2T(pInfo->pszClassName);
	m_sSchema	= OLE2T(pInfo->pszSchemaDN);
    
	///////////////////////////////////////////////////////////
	// Do not forget to clean-up the memory using FreeADsMem!
	//////////////////////////////////////////////////////////
	FreeADsMem( pInfo );

}

// 
//  IDirectoryObject::DeleteDSObject();
//
//
void CDlgIDirectoryObject::OnDelete() 
{
	CDeleteObjectDlg dlg;
	CString s, sMsg;
	HRESULT hr;

	if ( dlg.DoModal() == IDOK )
	{
		 USES_CONVERSION;
		 // Example: CN=jsmith
		 s	= dlg.GetObjectName();
		 hr = m_pDirObject->DeleteDSObject( T2OLE(s) );
	}

	if ( SUCCEEDED(hr) )
	{
		sMsg.Format(_T("Object '%s' was sucessfully deleted"), s); 
		AfxMessageBox( sMsg );
	}
	else
	{
		AfxMessageBox(GetErrorMessage(hr));
		return;
	}
	
}

///////////////////////////////////////////////////
//
// Demonstrate:
// IDirectoryObject::GetObjectAttributes
//
//////////////////////////////////////////////////


void CDlgIDirectoryObject::OnGet() 
{
   CStringList sList;
   UINT nCount;
   HRESULT hr;
   

   UpdateData(TRUE);

   StringToStringList(m_sAttributes, sList );
   nCount = sList.GetCount();
   if ( nCount == 0 )
   {
	   return;
   }

   
   
   ///////////////////////////////////////
   // Now build the Attribute Names List
   ///////////////////////////////////////
   POSITION			pos;
   DWORD			dwNumAttr;
   LPWSTR			*pAttrNames=NULL;
   ADS_ATTR_INFO	*pAttrInfo;
   DWORD			dwReturn;
   

   USES_CONVERSION;


   pAttrNames = (LPWSTR*) AllocADsMem( sizeof(LPWSTR) * nCount );
   pos = sList.GetHeadPosition();
   dwNumAttr = 0;
   while( pos != NULL )
   {
	   pAttrNames[dwNumAttr] = T2OLE(sList.GetAt(pos));
	   dwNumAttr++;
	   sList.GetNext(pos);
   }

   
 
   
   /////////////////////////////////////////
   // Get attributes value requested
   // Note: The order is not necessarily the same as
   //       requested via pAttrNames.
   ///////////////////////////////////////////
   hr = m_pDirObject->GetObjectAttributes( pAttrNames, dwNumAttr, &pAttrInfo, &dwReturn );

   

   if ( SUCCEEDED(hr) )
   {
	  DWORD idx;
	  CString sDisplay;
	  CStringList sValueList;
	  UINT nCount;
	  POSITION pos;

	  m_cValueList.ResetContent();

	  for( idx=0; idx < dwReturn; idx++) 
	  {		  
          ADsToStringList( pAttrInfo[idx].pADsValues, pAttrInfo[idx].dwNumValues, sValueList );
		  sDisplay = OLE2T(pAttrInfo[idx].pszAttrName);
		  sDisplay += _T(":");
		  m_cValueList.AddString( sDisplay );
		  nCount = sValueList.GetCount();
		  if ( nCount == 0 ) // can not find/convert the value
		  {
			  m_cValueList.AddString(_T(" > [No Value]"));
			  continue;
		  }
		  else
		  {
			  pos = sValueList.GetHeadPosition();
			  while( pos != NULL )
			  {
				  sDisplay = _T(" > ");
				  sDisplay += sValueList.GetAt(pos);
				  m_cValueList.AddString( sDisplay );
				  sValueList.GetNext(pos);
			  }

		  }
		  
	  }

	  sValueList.RemoveAll();
   }

   

   ///////////////////////////////////////////////////////////
   // Use FreADsMem for all memory obtained from ADSI call 
   /////////////////////////////////////////////////////////////
   FreeADsMem( pAttrInfo );
   FreeADsMem( pAttrNames );

	
}

void CDlgIDirectoryObject::OnChangeAttribute() 
{
	BOOL bEnabled;
	bEnabled = m_cAttributes.GetWindowTextLength() > 0 ? TRUE : FALSE;
	GetDlgItem( IDC_GET )->EnableWindow( bEnabled );
	
}

BOOL CDlgIDirectoryObject::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	OnChangeAttribute();
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
/////////////////////////////////////////////////////////////////////////////
// CDeleteObjectDlg dialog


CDeleteObjectDlg::CDeleteObjectDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDeleteObjectDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDeleteObjectDlg)
	m_sDelete = _T("");
	//}}AFX_DATA_INIT
}


void CDeleteObjectDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDeleteObjectDlg)
	DDX_Text(pDX, IDC_DELETE, m_sDelete);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDeleteObjectDlg, CDialog)
	//{{AFX_MSG_MAP(CDeleteObjectDlg)
	ON_EN_CHANGE(IDC_DELETE, OnChangeDelete)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDeleteObjectDlg message handlers

void CDeleteObjectDlg::OnOK() 
{
    UpdateData( TRUE );
	m_sRDN = m_sDelete;
	
	CDialog::OnOK();
}

void CDeleteObjectDlg::OnChangeDelete() 
{
    BOOL bEnabled;
	bEnabled = GetDlgItem( IDC_DELETE )->GetWindowTextLength() > 0 ? TRUE : FALSE;
	GetDlgItem( IDOK )->EnableWindow( bEnabled );
	
}
