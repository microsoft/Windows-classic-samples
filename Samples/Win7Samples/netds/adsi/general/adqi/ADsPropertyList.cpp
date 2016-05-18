//----------------------------------------------------------------------------
//
//  Microsoft Active Directory 2.5 Sample Code
//
//  Copyright (C) Microsoft Corporation, 1996 - 2000
//
//  File:       ADsPropertyList.cpp
//
//  Contents:   IADsPropertyList, IADsPropertyEntry and IADsPropertyValue usage
//
//
//----------------------------------------------------------------------------

#include "stdafx.h"
#include "ADQI.h"
#include "ADsPropertyList.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgIADsPropertyList dialog


CDlgIADsPropertyList::CDlgIADsPropertyList(LPUNKNOWN pUnk, CWnd* pParent /*=NULL*/)
	: CDialog(CDlgIADsPropertyList::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgIADsPropertyList)
	m_sAttribute = _T("");
	//}}AFX_DATA_INIT

	HRESULT hr;
	m_pPropList = NULL;
	hr = pUnk->QueryInterface( IID_IADsPropertyList, (void **) &m_pPropList );
	if ( !SUCCEEDED(hr) )
	{
		AfxMessageBox(_T("Fatal Error! QI for IADsPropertyList failed"));
		return;
	}
	pUnk->Release();

	
	//////////////////////////////////////////
	// Retrieve the data to the cache
	////////////////////////////////////////////
	IADs *pADs=NULL;
	hr = m_pPropList->QueryInterface( IID_IADs, (void**) &pADs );
	if ( SUCCEEDED(hr) )
	{
		hr = pADs->GetInfo();
		pADs->Release();
	}



}

CDlgIADsPropertyList::~CDlgIADsPropertyList()
{
	if ( m_pPropList )
	{
		 m_pPropList->Release();
	}

}

void CDlgIADsPropertyList::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgIADsPropertyList)
	DDX_Control(pDX, IDC_VALUELIST, m_cValueList);
	DDX_Control(pDX, IDC_ADSTYPE, m_cADsType);
	DDX_Text(pDX, IDC_ATTRIBUTE, m_sAttribute);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgIADsPropertyList, CDialog)
	//{{AFX_MSG_MAP(CDlgIADsPropertyList)
	ON_BN_CLICKED(IDC_GET, OnGet)
	ON_BN_CLICKED(IDC_PURGE, OnPurge)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgIADsPropertyList message handlers
//
//
//  IADsPropertyList->Next
//  IADsPropertyEnty
//


BOOL CDlgIADsPropertyList::OnInitDialog() 
{
	CDialog::OnInitDialog();
	m_cADsType.SetCurSel(ADSTYPE_CASE_IGNORE_STRING-1);	//Most attributes are string
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}



/////////////////////////////////////////////////////////
// Demonstrate:
// IADsPropertyList::GetPropertyItem
// IADsPropertyEntry
//
void CDlgIADsPropertyList::OnGet() 
{
	
	HRESULT hr;
	CString s;
	BSTR    bstr;
	VARIANT var;
	IDispatch *pDispatch;
	IADsPropertyEntry *pEntry;
	IADsPropertyValue *pValue;
	LONG  lADsType;
	

	UpdateData(TRUE);
	m_cValueList.ResetContent();

	bstr = m_sAttribute.AllocSysString();
	hr = m_pPropList->GetPropertyItem( bstr, m_cADsType.GetCurSel()+1, &var );
	SysFreeString( bstr );


	if ( SUCCEEDED(hr) )
	{
		pDispatch = V_DISPATCH( &var );
		hr = pDispatch->QueryInterface( IID_IADsPropertyEntry, (void**) &pEntry );
		VariantClear( &var );

		// IADsPropertyEntry
		if ( SUCCEEDED(hr) )
		{
			CPtrList dList;

			// get_Values return array of VT_DISPATH
			 hr = pEntry->get_Values( &var );
			 
			  
			 pEntry->get_ADsType( &lADsType);

			 hr = VariantToPtrList( var, dList );

			 pEntry->Release(); 

			 ////////////////////////////
			 // IADsPropertyValue
			 /////////////////////////////
			 if ( SUCCEEDED(hr) )
			 {
				POSITION pos;
				pos = dList.GetHeadPosition();
				while ( pos != NULL )
				{
					pDispatch = (IDispatch*) dList.GetAt(pos);
					hr = pDispatch->QueryInterface( IID_IADsPropertyValue, (void**) &pValue );
				
					if ( SUCCEEDED(hr) )
					{
						pValue->AddRef();
						hr = PropertyValueToString( lADsType, pValue, s );
						m_cValueList.AddString( s );
						pValue->Release();
					}
					dList.GetNext(pos);
				}
			
			 }

			 dList.RemoveAll();
			 VariantClear(&var);

		}

	}
}

void CDlgIADsPropertyList::OnPurge() 
{
  HRESULT hr;
  
  hr = m_pPropList->PurgePropertyList();

  if ( !SUCCEEDED(hr) )
  {
	  AfxMessageBox(GetErrorMessage(hr));
  }
}
