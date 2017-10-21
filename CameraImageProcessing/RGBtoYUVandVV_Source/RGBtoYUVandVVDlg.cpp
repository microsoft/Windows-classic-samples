
// RGBtoYUVandVVDlg.cpp : implementation file
//

#include "stdafx.h"
#include "RGBtoYUVandVV.h"
#include "RGBtoYUVandVVDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CRGBtoYUVandVVDlg dialog




CRGBtoYUVandVVDlg::CRGBtoYUVandVVDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CRGBtoYUVandVVDlg::IDD, pParent)
    , m_csImageType(_T("Y"))
    , m_csChromaSampling(_T("4:4:4"))
    , m_csChromaSampleR( m_csChromaSampling )
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CRGBtoYUVandVVDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_CBString(pDX, IDC_COMBO1, m_csImageType);
    DDX_CBString(pDX, IDC_COMBO2, m_csChromaSampling);
    DDX_Text(pDX, IDC_EDIT1, m_csChromaSampleR );
}

BEGIN_MESSAGE_MAP(CRGBtoYUVandVVDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
    ON_WM_SIZE()
    ON_WM_CTLCOLOR()
    ON_WM_SHOWWINDOW()
    ON_BN_CLICKED(IDC_BUTTON1, &CRGBtoYUVandVVDlg::OnBnClickedButton1)
    ON_CBN_SELCHANGE(IDC_COMBO2, &CRGBtoYUVandVVDlg::OnCbnSelchangeCombo2)
END_MESSAGE_MAP()


// CRGBtoYUVandVVDlg message handlers

BOOL CRGBtoYUVandVVDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

    myBitmap.Open( L"lena.bmp" );
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CRGBtoYUVandVVDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CRGBtoYUVandVVDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

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
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CRGBtoYUVandVVDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CRGBtoYUVandVVDlg::OnSize(UINT nType, int cx, int cy)
{
    CDialogEx::OnSize(nType, cx, cy);
    if( IsWindowVisible())
    {
        RECT wndrect;
        GetDlgItem( IDC_ORG3 )->GetWindowRect( &wndrect );
    }
}


HBRUSH CRGBtoYUVandVVDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
    HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);
    if( CTLCOLOR_STATIC == nCtlColor )
    {
        RECT wndrect;
        GetDlgItem( IDC_ORG3 )->GetWindowRect( &wndrect );
        myBitmap.Draw(  GetDlgItem( IDC_ORG3 )->m_hWnd, 0, 20,
                                       wndrect.right - wndrect.left, wndrect.bottom - wndrect.top );
        myBitmap.Draw(  GetDlgItem( IDC_ORG1 )->m_hWnd, 0, 20,
                                       wndrect.right - wndrect.left, wndrect.bottom - wndrect.top );
    }

    return hbr;
}


void CRGBtoYUVandVVDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
    CDialogEx::OnShowWindow(bShow, nStatus);

    RECT wndrect;
    ::GetWindowRect( this->m_hWnd, &wndrect );
    ::SetWindowPos(  GetDlgItem( IDC_ORG3 )->m_hWnd, HWND_TOP ,
                                    20 , 415, 336, 336, NULL );
      ::SetWindowPos(  GetDlgItem( IDC_ORG1 )->m_hWnd, HWND_TOP ,
                                    20 , 35, 336, 336, NULL );
     ::SetWindowPos(  GetDlgItem( IDC_ORG2 )->m_hWnd, HWND_TOP ,
                                356 , 35, 336, 336, NULL );
    ::SetWindowPos(  GetDlgItem( IDC_ORG4 )->m_hWnd, HWND_TOP ,
                                356 , 415, 336, 336, NULL );


}


void CRGBtoYUVandVVDlg::OnBnClickedButton1()
{
    UpdateData( TRUE );
    RGBtoYUV    myRGB2YUV;
    BitmapClass LocBitmap;
    BitmapClass LocBitmap1;


    RECT rect;
    (GetDlgItem( IDC_ORG1 ))->GetWindowRect( &rect );
    LocBitmap.ResetDisplay( (GetDlgItem( IDC_ORG2 ))->m_hWnd, 0, 20, rect.right - rect.left, myBitmap.Heigth() +3 );

    myRGB2YUV.CovertRGBtoYUV( myBitmap.Data(), myBitmap.Width(), myBitmap.Heigth(), m_csChromaSampling );

    int nWidth  = 0;
    int nHeight = 0;

    if( LUMA_ONLY == m_csImageType )
    {
        nWidth = myBitmap.Width();
        nHeight =  myBitmap.Heigth();
        LocBitmap.CreateBitmap( (unsigned char *)myRGB2YUV.GetYData(), nWidth, nHeight, BMP_24 );
    }
    else if( CB_ONLY == m_csImageType )
    {
        LocBitmap.CreateBitmap( (unsigned char *)myRGB2YUV.GetCbData( nWidth, nHeight ), nWidth,nHeight, BMP_24 );
    }
    else if( CR_ONLY == m_csImageType )
    {
        LocBitmap.CreateBitmap( (unsigned char *)myRGB2YUV.GetCrData(  nWidth, nHeight ), nWidth,nHeight, BMP_24 );
    }
    else
    {
    }

    nWidth = myBitmap.Width();
    nHeight =  myBitmap.Heigth();
    LocBitmap1.CreateBitmap( (unsigned char *)myRGB2YUV.ConvertYUVtoRGB(), nWidth,nHeight, BMP_24 );;
    LocBitmap1.Draw( (GetDlgItem( IDC_ORG4 ))->m_hWnd, 0, 20, rect.right - rect.left, rect.bottom - rect.top );


    LocBitmap.Draw( (GetDlgItem( IDC_ORG2 ))->m_hWnd, 0, 20, rect.right - rect.left, rect.bottom - rect.top );
    m_csChromaSampleR = m_csChromaSampling;
    UpdateData( FALSE );
}


void CRGBtoYUVandVVDlg::OnCbnSelchangeCombo2()
{
   // UpdateData( TRUE );
   // m_csChromaSampleR = m_csChromaSampling;
   // UpdateData( FALSE );
}
