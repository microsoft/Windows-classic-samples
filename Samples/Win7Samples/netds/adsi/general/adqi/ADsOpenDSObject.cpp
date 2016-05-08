

//----------------------------------------------------------------------------
//
//  Microsoft Active Directory 2.5 Sample Code
//
//  Copyright (C) Microsoft Corporation, 1996 - 2000
//
//  File:       ADsOpenDSObject.cpp
//
//  Contents:   IADsOpenDSObject and ADsOpenObject usage
//
//
//----------------------------------------------------------------------------


#include "stdafx.h"
#include "ADQI.h"
#include "ADsOpenDSObject.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


void DlgProcIADsOpenDSObject( LPUNKNOWN pUnk, LPUNKNOWN *ppNew)
{
   CDlgIADsOpenDSObject dlg( pUnk, ppNew );
   dlg.DoModal();
}


/////////////////////////////////////////////////////////////////////////////
// CDlgIADsOpenDSObject dialog


CDlgIADsOpenDSObject::CDlgIADsOpenDSObject( LPUNKNOWN pUnk, LPUNKNOWN *ppNew, CWnd* pParent /*=NULL*/)
	: CDialog(CDlgIADsOpenDSObject::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgIADsOpenDSObject)
	m_sPassword = _T("");
	m_sUserName = _T("");
	m_bEncrypt = FALSE;
	m_bReadOnly = FALSE;
	m_bSecured = FALSE;
	m_bPrompt = FALSE;
	m_bNoAuthentication = FALSE;
	m_sADsPath = _T("");
	m_bClearText = FALSE;
	m_bFastBind = FALSE;
	m_bSealing = FALSE;
	m_bSigning = FALSE;
	//}}AFX_DATA_INIT
	
	HRESULT hr;
	m_pOpenDS = NULL;
	m_ppUnk = ppNew;

	
	hr = pUnk->QueryInterface( IID_IADsOpenDSObject, (void **) &m_pOpenDS );
	if ( !SUCCEEDED(hr) )
	{
		// Give warning... ADsOpenObject has the same functionality as IADsOpenDSObject,
		// the only different is the IADsOpenDSObject normally lives on the namespace object
		AfxMessageBox(_T("Warning: QI for IADsOpenDSObject failed, we will use ADsOpenObject()"));
	}

	

	CComPtr<IADs> pADs=NULL;
	BSTR bstr;
	hr = pUnk->QueryInterface( IID_IADs, (void**) &pADs );
	if ( SUCCEEDED(hr) )
	{
		pADs->get_ADsPath( &bstr );
		m_sADsPath = bstr;
   	    SysFreeString( bstr );
	}

	pUnk->Release();



	


}

CDlgIADsOpenDSObject::~CDlgIADsOpenDSObject()
{
	if ( m_pOpenDS )
	{
		m_pOpenDS->Release();
	}

}

void CDlgIADsOpenDSObject::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgIADsOpenDSObject)
	DDX_Text(pDX, IDC_PASSWORD, m_sPassword);
	DDX_Text(pDX, IDC_USERNAME, m_sUserName);
	DDX_Check(pDX, IDC_ENCRYPT, m_bEncrypt);
	DDX_Check(pDX, IDC_READONLY, m_bReadOnly);
	DDX_Check(pDX, IDC_SECURED, m_bSecured);
	DDX_Check(pDX, IDC_PROMPTCREDENTIAL, m_bPrompt);
	DDX_Check(pDX, IDC_NOAUTHENTICATION, m_bNoAuthentication);
	DDX_Text(pDX, IDC_ADSPATH, m_sADsPath);
	DDX_Check(pDX, IDC_CLEARTEXT, m_bClearText);
	DDX_Check(pDX, IDC_FASTBIND, m_bFastBind);
	DDX_Check(pDX, IDC_SEALING, m_bSealing);
	DDX_Check(pDX, IDC_SIGNING, m_bSigning);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgIADsOpenDSObject, CDialog)
	//{{AFX_MSG_MAP(CDlgIADsOpenDSObject)
	ON_EN_CHANGE(IDC_USERNAME, OnChangeUsername)
	ON_EN_CHANGE(IDC_ADSPATH, OnChangeADsPath)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgIADsOpenDSObject message handlers

void CDlgIADsOpenDSObject::OnOK() 
{
	HRESULT hr;
	DWORD dwFlag;

	UpdateData(TRUE);


	dwFlag = 0;
	if ( m_bSecured )
	{
		 dwFlag |=  ADS_SECURE_AUTHENTICATION;
	}

	if ( m_bReadOnly )
	{
		 dwFlag |= ADS_READONLY_SERVER;
	}

	if ( m_bEncrypt )
	{
		dwFlag |= 	ADS_USE_ENCRYPTION;
	}

	if ( m_bPrompt )
	{
		dwFlag |= ADS_PROMPT_CREDENTIALS;
	}

	if ( m_bNoAuthentication )
	{
		dwFlag = ADS_NO_AUTHENTICATION;	
	}

	if ( m_bFastBind )
	{
		dwFlag |= ADS_FAST_BIND;
	}

	if ( m_bSigning )
	{
		dwFlag |= ADS_USE_SIGNING;
	}

	if ( m_bSealing )
	{
		dwFlag |= ADS_USE_SEALING;
	}
	
	
	
	USES_CONVERSION;
	LPWSTR pszUser=NULL;
	LPWSTR pszPwd=NULL;
	IUnknown *pNewUnk;

	if ( !m_sUserName.IsEmpty() )
	{
		pszUser = T2OLE(m_sUserName);
	}

	pszPwd  = T2OLE(m_sPassword);



	if ( pszUser == NULL )
	{
		hr = ADsGetObject( T2OLE(m_sADsPath), IID_IUnknown, (void**) &pNewUnk );
	}
	else
	{
		if ( m_pOpenDS )
		{
	
  			hr = m_pOpenDS->OpenDSObject( T2OLE(m_sADsPath), pszUser, pszPwd, 
	   			                            dwFlag, (IDispatch**) &pNewUnk );

		}
		else
		{
			hr = ADsOpenObject( T2OLE(m_sADsPath), pszUser, pszPwd, 
	   			                dwFlag, IID_IUnknown, (void**) &pNewUnk );

		}
	}



	if ( !SUCCEEDED(hr) )
	{
		AfxMessageBox(GetErrorMessage(hr));
		return;
	}
	
	//////////////////////////////////////////////////////////////////////
	// Save it so, somebody can ask the pointer with its credentials have
	// been validated
	////////////////////////////////////////////////////////////////////
	
	if (SUCCEEDED(hr) )
	{
		*m_ppUnk = pNewUnk;  // new validate iunkown;
		////////////////////////////////////////////////////////////
		// IMPORTANT!!: DO NOT SAVE THE PASSWORD IN MEMORY
		///////////////////////////////////////////////////////////
		if ( pszUser )
		{
			App->SetCredentials( OLE2T(pszUser), dwFlag ); // save it for later use
		}
		else
		{
			App->SetCredentials( _T(""), dwFlag ); // save it for later use
		}
		

	}


	CDialog::OnOK();
}



void CDlgIADsOpenDSObject::OnChangeUsername() 
{
	
	BOOL bADsLength;
	
	//////////////////////////////////////////////////////////////
	// Turn off/on the OK button depending if ADsPath Edit box is filled.
	//////////////////////////////////////////////////////////////////////
	
	bADsLength  = GetDlgItem(IDC_ADSPATH)->GetWindowTextLength() > 0 ? TRUE : FALSE;

	GetDlgItem( IDOK )->EnableWindow( bADsLength );
}


void CDlgIADsOpenDSObject::OnChangeADsPath() 
{
   OnChangeUsername();	
}


BOOL CDlgIADsOpenDSObject::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// Get the current DN;
    OnChangeUsername();	


	UpdateData(FALSE); // update to relect the UI
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

