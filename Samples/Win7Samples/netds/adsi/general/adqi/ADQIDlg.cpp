//----------------------------------------------------------------------------
//
//  Microsoft Active Directory 2.5 Sample Code
//
//  Copyright (C) Microsoft Corporation, 1996 - 2000
//
//  File:       ADQIDlg.cpp
//
//  Contents:   Main ADQI User Interface Implementation
//
//
//----------------------------------------------------------------------------

#include "stdafx.h"
#include "ADQI.h"
#include "ADQIDlg.h"
#include "lmcons.h"
#include "ads.h"
#include "adscontainer.h"
#include "directorysearch.h"
#include "directoryobject.h"
#include "adspropertylist.h"
#include "adsopendsobject.h"
#include "adslargeinteger.h"
#include "security.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


DECLAREADSPROC(IADs)
DECLAREADSPROC(IADsContainer)
DECLAREADSPROC(IDirectorySearch)
DECLAREADSPROC(IDirectoryObject)
DECLAREADSPROC(IADsPropertyList)
DECLAREADSPROC(IADsSecurityDescriptor)
DECLAREADSPROC(IADsLargeInteger)





ADSIIF  adsiIfs[] = { 
	MAKEADSENTRY(IADs), DlgProcIADs,
	MAKEADSENTRY(IADsContainer), DlgProcIADsContainer,
	MAKEADSENTRY(IADsCollection), NULL,
	MAKEADSENTRY(IADsMembers), NULL,
	MAKEADSENTRY(IADsPropertyList),	DlgProcIADsPropertyList,
	MAKEADSENTRY(IADsPropertyEntry), NULL,
	MAKEADSENTRY(IADsPropertyValue), NULL,
	MAKEADSENTRY(IADsExtension), NULL,
	MAKEADSENTRY(IADsNamespaces), NULL,
	MAKEADSENTRY(IADsClass), NULL,
	MAKEADSENTRY(IADsProperty), NULL,
	MAKEADSENTRY(IADsSyntax), NULL,
	MAKEADSENTRY(IADsLocality), NULL,
	MAKEADSENTRY(IADsO), NULL,
	MAKEADSENTRY(IADsOU), NULL,
	MAKEADSENTRY(IADsDomain), NULL,
	MAKEADSENTRY(IADsComputer), NULL,
	MAKEADSENTRY(IADsComputerOperations), NULL,
	MAKEADSENTRY(IADsGroup), NULL,
	MAKEADSENTRY(IADsUser), NULL,
	MAKEADSENTRY(IADsPrintQueue), NULL,
	MAKEADSENTRY(IADsPrintQueueOperations), NULL,
	MAKEADSENTRY(IADsPrintJob), NULL,
	MAKEADSENTRY(IADsPrintJobOperations), NULL,
	MAKEADSENTRY(IADsService), NULL,
	MAKEADSENTRY(IADsServiceOperations), NULL,
	MAKEADSENTRY(IADsFileService), NULL,
	MAKEADSENTRY(IADsFileServiceOperations), NULL,
	MAKEADSENTRY(IADsFileShare), NULL,
	MAKEADSENTRY(IADsSession), NULL,
	MAKEADSENTRY(IADsResource), NULL,
	MAKEADSENTRY(IADsOpenDSObject), DlgProcIADsOpenDSObject,
	MAKEADSENTRY(IDirectoryObject), DlgProcIDirectoryObject,
	MAKEADSENTRY(IDirectorySearch), DlgProcIDirectorySearch,
	MAKEADSENTRY(IADsAccessControlEntry), NULL,
	MAKEADSENTRY(IADsSecurityDescriptor), DlgProcIADsSecurityDescriptor,
	MAKEADSENTRY(IADsLargeInteger), DlgProcIADsLargeInteger,
	MAKEADSENTRY(IADsObjectOptions), NULL,
	MAKEADSENTRY(IADsPropertyValue2), NULL,
//The following interfaces is CoCreateable, not living off on any object,
//so we comment this out, because QI won't return any of these interfaces
//	MAKEADSENTRY(IADsPathname), NULL, 
//	MAKEADSENTRY(IADsNameTranslate),  NULL,
	MAKEADSENTRY(NULL), NULL
};



/////////////////////////////////////////////////////////////////////////////
// CADQIDlg dialog

CADQIDlg::CADQIDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CADQIDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CADQIDlg)
	m_sADsPath = _T("");
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_pUnk = NULL;
}

void CADQIDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CADQIDlg)
	DDX_Control(pDX, IDC_INTERFACES, m_cListIf);
	DDX_Control(pDX, IDOK, m_cOK);
	DDX_Control(pDX, IDC_ADSPATH, m_cADsPath);
	DDX_Text(pDX, IDC_ADSPATH, m_sADsPath);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CADQIDlg, CDialog)
	//{{AFX_MSG_MAP(CADQIDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_EN_CHANGE(IDC_ADSPATH, OnChangeADsPath)
	ON_LBN_DBLCLK(IDC_INTERFACES, OnDblClkInterfaces)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CADQIDlg message handlers

BOOL CADQIDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	OnChangeADsPath(); 

	
	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}


// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CADQIDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CADQIDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CADQIDlg::OnOK() 
{
	
	HRESULT   hr;
	CWaitCursor wait;
	IUnknown *pSave = NULL;

	UpdateData( TRUE );


	if ( m_pUnk ) 
	{
		pSave = m_pUnk; // save it first
	}

	USES_CONVERSION;
	hr = App->ADsOpenObject(T2OLE(m_sADsPath), IID_IUnknown, (void**) &m_pUnk );


	if ( !SUCCEEDED(hr) )
	{
	   AfxMessageBox(GetErrorMessage(hr));
	   m_pUnk = NULL;
	   return;
	}


	if ( pSave ) // release the previous pointer
	{
		pSave->Release();

	}
	

	EnumerateInterface();

	

}

void CADQIDlg::OnChangeADsPath() 
{
	BOOL bEnabled;
	bEnabled = m_cADsPath.GetWindowTextLength() > 0 ? TRUE : FALSE;

	m_cOK.EnableWindow( bEnabled );
	
}

void CADQIDlg::OnCancel() 
{	
	CDialog::OnCancel();
}

void CADQIDlg::EnumerateInterface()
{
	int xx=0;
	HRESULT hr;
	LPUNKNOWN pQI;

	m_cListIf.ResetContent();

	///////////////////////////////////////////////////////////////
	// Query Interface all known ADSI Interfaces
	////////////////////////////////////////////////////////////////
	while( !IsEqualIID( *adsiIfs[xx].pIID, IID_NULL ) )
	{
		hr = m_pUnk->QueryInterface( *adsiIfs[xx].pIID, (void**) &pQI );
		if ( SUCCEEDED(hr) )
		{
			m_cListIf.AddString( adsiIfs[xx].szIf );
			pQI->Release();
		}
		xx++;
	}
	 

	
}

void CADQIDlg::OnDblClkInterfaces() 
{
	CString s;
	int xx=0;
	int idx;
	IUnknown *pNewUnk = NULL;


	idx = m_cListIf.GetCurSel();
	if ( idx == LB_ERR )
	{
		MessageBeep(0);
		return;
	}


	CWaitCursor wait;
	m_cListIf.GetText( idx, s );


	//////////////////////////////////////////////////////////////
	// 
	// Find the appropriate dialog box to display
	//
	/////////////////////////////////////////////////////////////////
	while( !IsEqualIID( *adsiIfs[xx].pIID, IID_NULL ) && s != adsiIfs[xx].szIf  )
	{
		xx++;
	}

	ASSERT( !IsEqualIID( *adsiIfs[xx].pIID, IID_NULL ) );
	if ( adsiIfs[xx].pFn )
	{
		m_pUnk->AddRef();
		(*adsiIfs[xx].pFn)( m_pUnk, &pNewUnk );
	}
	else
	{
	   wait.Restore();
	   AfxMessageBox(_T("No UI implemented yet"));
	}



	////////////////////////////////////////////////////
	// if IADsOpenObject is selected, special care
	///////////////////////////////////////////////////
	if ( pNewUnk )
	{

		HRESULT hr;
		BSTR  bstr;
		IADs  *pADs;

		hr = pNewUnk->QueryInterface( IID_IADs, (void**) &pADs );
		if ( SUCCEEDED(hr) )
		{
			pADs->get_ADsPath( &bstr );
		}
		pADs->Release();

		m_sADsPath = bstr;
		SysFreeString( bstr );
		

		m_pUnk->Release(); // old ads iunknown path;
		m_pUnk = pNewUnk;

		UpdateData(FALSE);
		EnumerateInterface();

	}
	

}



