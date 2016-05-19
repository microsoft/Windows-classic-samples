//----------------------------------------------------------------------------
//
//  Microsoft Active Directory 2.5 Sample Code
//
//  Copyright (C) Microsoft Corporation, 1996 - 2000
//
//  File:       security.cpp
//
//  Contents:   IADsSecurityDescriptor, IADsAccessControlList, IADsAccessControlEntry usage
//
//
//----------------------------------------------------------------------------

#include "stdafx.h"
#include "ADQI.h"
#include "directorysearch.h"
#include "security.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgIADsSecurityDescriptor dialog


CDlgIADsSecurityDescriptor::CDlgIADsSecurityDescriptor(LPUNKNOWN pUnk, CWnd* pParent /*=NULL*/)
	: CDialog(CDlgIADsSecurityDescriptor::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgIADsSecurityDescriptor)
	//}}AFX_DATA_INIT


	
	HRESULT hr;	
	m_pSecurityDesc = NULL;

	
	hr = pUnk->QueryInterface( IID_IADsSecurityDescriptor, (void **) &m_pSecurityDesc );
	pUnk->Release();

	if ( !SUCCEEDED(hr) )
	{
		AfxMessageBox(_T("Fatal Error! QI for IADsSecurityDescriptor failed"));
		return;
	}
	
	

}

CDlgIADsSecurityDescriptor::~CDlgIADsSecurityDescriptor()
{
	ResetAcePtr();
	if ( m_pSecurityDesc )
	{
		m_pSecurityDesc->Release();
	}
	
}


void CDlgIADsSecurityDescriptor::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgIADsSecurityDescriptor)
	DDX_Control(pDX, IDC_ACELIST, m_cACEList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgIADsSecurityDescriptor, CDialog)
	//{{AFX_MSG_MAP(CDlgIADsSecurityDescriptor)
	ON_LBN_SELCHANGE(IDC_ACELIST, OnSelChangeAceList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgIADsSecurityDescriptor message handlers

void CDlgIADsSecurityDescriptor::PopulateACL(IADsAccessControlList * pACL)
{
	ASSERT( pACL );
	HRESULT hr;
	ULONG   lFetch;

	IEnumVARIANT *pEnum;
	LPUNKNOWN     pUnk;
	VARIANT       var;
	IDispatch	 *pDisp;
	BSTR		  bstr;
	CString		  s;

	IADsAccessControlEntry *pACE;
	
	
	m_cACEList.ResetContent();
	ResetAcePtr();

	
	hr = pACL->get__NewEnum( &pUnk );
	if ( !SUCCEEDED(hr) )
	{
		return;
	}

	hr = pUnk->QueryInterface( IID_IEnumVARIANT, (void**) &pEnum );
	if ( !SUCCEEDED(hr) )
	{
		return;
	}

	hr = pEnum->Next( 1, &var, &lFetch );
	
	while( hr == S_OK )
	{		
		if ( lFetch == 1 )
		{
			if ( VT_DISPATCH != V_VT(&var) )
			{
				break;
			}
		

		   	pDisp = V_DISPATCH(&var);
			///////////////////////////
			// Get the ACE
			/////////////////////////////
			hr = pDisp->QueryInterface( IID_IADsAccessControlEntry, (void**)&pACE ); 
			if ( SUCCEEDED(hr) )
			{
				pACE->get_Trustee(&bstr);
				s = bstr;
				SysFreeString(bstr);
				m_cACEList.AddString( s );
				m_acePtrList.AddTail( pACE ); //save the pointer for future use, 
				                           // we don't need to Release() it.

			}

			VariantClear(&var);
		}
		hr = pEnum->Next( 1, &var, &lFetch );
		
    };



	pACL->Release();
}


void CDlgIADsSecurityDescriptor::ResetAcePtr()
{
	POSITION pos;
	IADsAccessControlList *pACE;
	pos = m_acePtrList.GetHeadPosition();
	while( pos != NULL )
	{
	    pACE = (IADsAccessControlList*) m_acePtrList.GetAt( pos );
		pACE->Release();
		m_acePtrList.GetNext( pos );
	}
	m_acePtrList.RemoveAll();
}


BOOL CDlgIADsSecurityDescriptor::OnInitDialog() 
{
	CDialog::OnInitDialog();

	if ( m_pSecurityDesc == NULL )
	{
		return TRUE;
	}
	
	
	/////////////////////////////////////////////
	// Populate the DACL 
	////////////////////////////////////////////
	HRESULT hr;
	IDispatch *pDisp;

	IADsAccessControlList *pACL;
	hr = m_pSecurityDesc->get_DiscretionaryAcl( &pDisp );
	if (!SUCCEEDED(hr) )
	{
		AfxMessageBox(GetErrorMessage(hr));
		return TRUE;
	}


	hr = pDisp->QueryInterface( IID_IADsAccessControlList, (void**) &pACL );
	pDisp->Release();

	if ( SUCCEEDED(hr) )
	{
		PopulateACL( pACL );
	}

	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgIADsSecurityDescriptor::OnSelChangeAceList() 
{
	int idx=0;
	
	///////////////////////////////////////////////////
	// Seach the ACEs
	////////////////////////////////////////////////
	idx = m_cACEList.GetCurSel();
	if ( idx == LB_ERR )
	{
		return;
	}

	POSITION pos = m_acePtrList.FindIndex( idx );
	IADsAccessControlEntry *pACE = NULL;
	if ( !pos )
	{
		return;
	}

	///////////////////////////////////////////////////
	// Now we can disply the ACE characteristics
	////////////////////////////////////////////////////
    pACE = (IADsAccessControlEntry *) m_acePtrList.GetAt( pos );
	if ( pACE )
	{
		pACE->AddRef();
		DisplayACE( pACE );
	}
}



void CDlgIADsSecurityDescriptor::DisplayACE( IADsAccessControlEntry *pACE )
{
	LONG aceMask;
	LONG aceType;
	int idx;
	BSTR bstr;
	CString sObjectType;

	ASSERT( pACE );

	///////////////////////////////////////////////////
	//	Get Access Mask, Ace Type and Object Type
	////////////////////////////////////////////////////
	if( !SUCCEEDED(pACE->get_AccessMask(&aceMask)) )
	{
		return;
	}
	if ( !SUCCEEDED(pACE->get_AceType(&aceType)) )
	{
		return;
	}
	
	if ( !SUCCEEDED(pACE->get_ObjectType(&bstr)) )
	{
		return;
	}

	sObjectType = bstr;
	SysFreeString( bstr );

	

	//////////////////////////////////////
	// Display the type
	//////////////////////////////////////
	int aceTypeList[] = 
	{
		ADS_ACETYPE_ACCESS_ALLOWED,
	    ADS_ACETYPE_ACCESS_DENIED,
		ADS_ACETYPE_ACCESS_ALLOWED_OBJECT,
		ADS_ACETYPE_ACCESS_DENIED_OBJECT,
		ADS_ACETYPE_SYSTEM_AUDIT,
		ADS_ACETYPE_SYSTEM_AUDIT_OBJECT,
	};


	int sel = -1;
	for( idx=0; idx < (sizeof(aceTypeList)/sizeof(int)); idx++ )
	{
		if ( aceType  == aceTypeList[idx] )
		{
			sel = idx;
			break;
		}
	}
	((CComboBox*)GetDlgItem(IDC_ACETYPE))->SetCurSel( sel );
	
	/////////////////////////////////////
	// Display the aces mask
	/////////////////////////////////////
	ResetAceMaskControls();	// Reset the UI

	// Standard ACE Rights
	if ( aceMask & 	ADS_RIGHT_DELETE ) 
	{
		CheckDlgButton( IDC_DELETEOBJECT, TRUE );
	}

	if ( aceMask & ADS_RIGHT_READ_CONTROL )
	{
		CheckDlgButton( IDC_READPERMISSION, TRUE );
	}

	if ( aceMask & ADS_RIGHT_WRITE_DAC )
	{
		CheckDlgButton( IDC_MODIFYPERMISSION, TRUE );
	}

	if ( aceMask & ADS_RIGHT_WRITE_OWNER )
	{
		CheckDlgButton( IDC_MODIFYOWNER, TRUE );
	}


	// Directory ACE Rights
	if ( aceMask & ADS_RIGHT_DS_CREATE_CHILD )
	{
		CheckDlgButton( IDC_CREATECHILD, TRUE );
		DisplayAceObjectType( IDC_CREATECHILD_EDIT, sObjectType );
	}

	if ( aceMask & ADS_RIGHT_DS_DELETE_CHILD )
	{
	   CheckDlgButton( IDC_DELETECHILD, TRUE );
	   DisplayAceObjectType( IDC_DELETECHILD_EDIT, sObjectType );
	}

	if ( aceMask & ADS_RIGHT_ACTRL_DS_LIST )
	{
	   CheckDlgButton( IDC_LISTCONTENT, TRUE );
	}

	if ( aceMask & ADS_RIGHT_DS_SELF )
	{
		CheckDlgButton( IDC_LISTOBJECT, TRUE );
	}

	if ( aceMask & ADS_RIGHT_DS_DELETE_TREE )
	{
		CheckDlgButton( IDC_DELETETREE, TRUE );
	}

	if ( aceMask & ( ADS_RIGHT_DS_READ_PROP  |  ADS_RIGHT_DS_WRITE_PROP | 0x100) &&
		 ( (aceType == ADS_ACETYPE_ACCESS_ALLOWED_OBJECT)  | 
		   (aceType == ADS_ACETYPE_ACCESS_DENIED_OBJECT)  ) 
	   )
	{
		CheckDlgButton( IDC_EXTENDEDRIGHT, TRUE );
		DisplayAceObjectType( IDC_EXTENDEDRIGHT_EDIT, sObjectType );

	}
	else
	{
		if ( aceMask & ADS_RIGHT_DS_READ_PROP )
		{
			CheckDlgButton( IDC_READPROP, TRUE );
			DisplayAceObjectType( IDC_READPROP_EDIT, sObjectType );
		}

		if ( aceMask & ADS_RIGHT_DS_WRITE_PROP )
		{
			CheckDlgButton( IDC_WRITEPROP, TRUE );
			DisplayAceObjectType( IDC_WRITEPROP_EDIT, sObjectType );
		}
	}




	  
}


void CDlgIADsSecurityDescriptor::DisplayAceObjectType( UINT nID, CString &sObjectType )
{
	if ( sObjectType.IsEmpty() )
	{
		SetDlgItemText( nID, _T("ALL") );
	}
	else
	{
		SetDlgItemText( nID, sObjectType );
	}


}
void CDlgIADsSecurityDescriptor::ResetAceMaskControls()
{
   CheckDlgButton( IDC_DELETEOBJECT, FALSE );
   CheckDlgButton( IDC_READPERMISSION, FALSE );
   CheckDlgButton( IDC_CREATECHILD, FALSE );
   CheckDlgButton( IDC_DELETECHILD, FALSE );
   CheckDlgButton( IDC_LISTCONTENT, FALSE );
   CheckDlgButton( IDC_LISTOBJECT, FALSE );
   CheckDlgButton( IDC_DELETETREE, FALSE );
   CheckDlgButton( IDC_READPROP, FALSE );
   CheckDlgButton( IDC_WRITEPROP, FALSE );
   
   SetDlgItemText( IDC_CREATECHILD_EDIT, _T("") );
   SetDlgItemText( IDC_DELETECHILD_EDIT, _T("") );
   SetDlgItemText( IDC_READPROP_EDIT, _T("") );
   SetDlgItemText( IDC_WRITEPROP_EDIT, _T("") );
   SetDlgItemText( IDC_EXTENDEDRIGHT_EDIT, _T("") );

}
