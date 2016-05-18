// ADsLargeInteger.cpp : implementation file
//

#include "stdafx.h"
#include "ADQI.h"
#include "ADsLargeInteger.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgIADsLargeInteger dialog


CDlgIADsLargeInteger::CDlgIADsLargeInteger( LPUNKNOWN pUnk, CWnd* pParent /*=NULL*/)
	: CDialog(CDlgIADsLargeInteger::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgIADsLargeInteger)
	m_lHiPart = 0;
	m_lLowPart = 0;
	//}}AFX_DATA_INIT


	HRESULT hr;	
	m_pLargeInt = NULL;

	
	hr = pUnk->QueryInterface( IID_IADsLargeInteger, (void **) &m_pLargeInt );
	pUnk->Release();

	if ( !SUCCEEDED(hr) )
	{
		AfxMessageBox(_T("Fatal Error! QI for IADsLargeInteger failed"));
		return;
	}


	

}

CDlgIADsLargeInteger::~CDlgIADsLargeInteger()
{
	if ( m_pLargeInt )
	{
		m_pLargeInt->Release();
	}
}


void CDlgIADsLargeInteger::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgIADsLargeInteger)
	DDX_Text(pDX, IDC_HIPART, m_lHiPart);
	DDX_Text(pDX, IDC_LOWPART, m_lLowPart);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgIADsLargeInteger, CDialog)
	//{{AFX_MSG_MAP(CDlgIADsLargeInteger)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgIADsLargeInteger message handlers

void CDlgIADsLargeInteger::OnOK() 
{
	
	CDialog::OnOK();
}

BOOL CDlgIADsLargeInteger::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	if ( m_pLargeInt )
	{
		m_pLargeInt->get_HighPart( &m_lHiPart );
		m_pLargeInt->get_LowPart( &m_lLowPart );
		UpdateData(FALSE);
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
