// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.


// DlgSrc.cpp : implementation file
//

/*
 *  preprocessor section
 */

#include <guiddef.h>
#include "stdafx.h"
#include "multichan.h"
#include "childview.h"
#include "DlgSrc.h"
#include <ks.h>
#include <ksmedia.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/*
 *  definition section
 */

CONST_DWORD_REP cdrSpeakers[] =
{
    {SPEAKER_FRONT_LEFT,                "SPEAKER_FRONT_LEFT" } ,
    {SPEAKER_FRONT_RIGHT,               "SPEAKER_FRONT_RIGHT" } ,
    {SPEAKER_FRONT_CENTER,              "SPEAKER_FRONT_CENTER" } ,
    {SPEAKER_LOW_FREQUENCY,             "SPEAKER_LOW_FREQUENCY" } ,
    {SPEAKER_BACK_LEFT,                 "SPEAKER_BACK_LEFT" } ,
    {SPEAKER_BACK_RIGHT,                "SPEAKER_BACK_RIGHT" } ,
    {SPEAKER_FRONT_LEFT_OF_CENTER,      "SPEAKER_FRONT_LEFT_OF_CENTER" } ,
    {SPEAKER_FRONT_RIGHT_OF_CENTER,     "SPEAKER_FRONT_RIGHT_OF_CENTER" } ,
    {SPEAKER_BACK_CENTER,               "SPEAKER_BACK_CENTER" } ,
    {SPEAKER_SIDE_LEFT,                 "SPEAKER_SIDE_LEFT" } ,
    {SPEAKER_SIDE_RIGHT,                "SPEAKER_SIDE_RIGHT" } ,
    {SPEAKER_TOP_CENTER,                "SPEAKER_TOP_CENTER" } ,
    {SPEAKER_TOP_FRONT_LEFT,            "SPEAKER_TOP_FRONT_LEFT" } ,
    {SPEAKER_TOP_FRONT_CENTER,          "SPEAKER_TOP_FRONT_CENTER" } ,
    {SPEAKER_TOP_FRONT_RIGHT,           "SPEAKER_TOP_FRONT_RIGHT" } ,
    {SPEAKER_TOP_BACK_LEFT,             "SPEAKER_TOP_BACK_LEFT" } ,
    {SPEAKER_TOP_BACK_CENTER,           "SPEAKER_TOP_BACK_CENTER" } ,
    {SPEAKER_TOP_BACK_RIGHT,            "SPEAKER_TOP_BACK_RIGHT" } ,
    {SPEAKER_NOT_SPECIFIED,             "No specified speaker" }
};

/////////////////////////////////////////////////////////////////////////////
// CDlgSrc dialog

/*
 *  MFC mechanisms
 */
void 
CDlgSrc::DoDataExchange
(
    CDataExchange*  pDX
)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgSrc)
	DDX_Control(pDX, IDOK, m_butClose);
    DDX_Control( pDX, IDC_PLAY, m_butPlay );
    DDX_Control( pDX, IDC_STOP, m_butStop );
	DDX_Control(pDX, IDC_INPUT, m_cInput);
	DDX_Control(pDX, IDC_COMBO_SPEAKER, m_comboSpeaker);
	DDX_Check(pDX, IDC_USE, m_fUse);
	DDX_CBIndex(pDX, IDC_COMBO_SPEAKER, m_nSpeaker);
	DDX_Text(pDX, IDC_INPUT, m_strName);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgSrc, CDialog)
	//{{AFX_MSG_MAP(CDlgSrc)
	ON_CBN_SELENDOK(IDC_COMBO_SPEAKER, OnComboSpeaker)
	ON_BN_CLICKED(IDC_PLAY, OnPlay)
    ON_BN_CLICKED(IDC_STOP, OnStop)
	ON_WM_CTLCOLOR()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////

/*
 *  Constructors
 */

// ----------------------------------------------------------------------------------
// CDlgSrc::CDlgSrc
//  constructor
// ----------------------------------------------------------------------------------
CDlgSrc::CDlgSrc
(
    CWnd* pParent /*=NULL*/
)
  : CDialog(CDlgSrc::IDD, pParent),
    m_strName(TEXT("")),
    m_pwndParent(pParent),
    m_cSamples(0),
    m_cbData(0),
    m_nChannel(0),
    m_dwChannelMask(SPEAKER_FRONT_LEFT),
    m_fDragging(FALSE),
    m_fPlayable( FALSE ),
    m_lpwhdr( 0 )
{
	//{{AFX_DATA_INIT(CDlgSrc)
	m_fUse = TRUE;
	m_nSpeaker = 0;
	m_strName = _T("");
	//}}AFX_DATA_INIT

    ZeroMemory( &m_wfx, sizeof( WAVEFORMATEX ) );
}

// ----------------------------------------------------------------------------------
// CDlgSrc::~CDlgSrc
//  destructor
// ----------------------------------------------------------------------------------
CDlgSrc::~CDlgSrc
()
{
	//
	//	update speaker mask
	//
	if( g_pdlgDest )
		g_pdlgDest->m_wfExt.dwChannelMask ^= m_dwChannelMask;

	//
	//	release memory
	//
	SafeLocalFree( m_pvData );
	SafeLocalFree( m_lpwhdr );
}

// ----------------------------------------------------------------------------------
// CDlgSrc::Create
//  creates an SRC dialog whose title is strName, position is sizeInitPos, and 
//  channel mask is dwChannelMask
// ----------------------------------------------------------------------------------
void 
CDlgSrc::Create
(
    LPCSTR  strName,
    SIZE    sizeInitPos,
    DWORD   dwChannelMask
)  
{
    CDialog::Create( CDlgSrc::IDD, m_pwndParent );

    //
    //  position the dialog for that nice cascade effect
    //
    CRect rect;

    GetClientRect( &rect );

    rect.OffsetRect( *(LPPOINT(&sizeInitPos)) );
    rect.right += 4;
    rect.bottom += 4;
    MoveWindow( &rect, TRUE );

    //
    //  set window text
    //
    m_strName = strName;

    //
    //  populate the speakers combo
    //  speaker positions are defined in KSMedia.h or in MMReg.h
    //
    int i;
    int c;
    for( c = 0; c < ARRAY_ELEMENTS( cdrSpeakers ); c++ )
    {
        i = m_comboSpeaker.AddString( cdrSpeakers[c].pstrRep );         //  friendly name
        m_comboSpeaker.SetItemData( i, cdrSpeakers[c].dwConstant );     //  index
        if( cdrSpeakers[c].dwConstant == dwChannelMask )                //  check out of range
            m_nSpeaker = c;
    }

    m_dwChannelMask = dwChannelMask;                                    //  overall channel mask

    //
    //  make playable
    //
    m_fPlayable = TRUE;
	m_butPlay.EnableWindow( m_fPlayable );
	m_butStop.EnableWindow( !m_fPlayable );

    //
    // update controls
    //
    UpdateData( FALSE );
}   //  CDglSrc::Create

/*
 *  Accessors
 */

// ----------------------------------------------------------------------------------
// AquireData
//  This fn extracts a single channel (nChannelFrom) from the wave data specified by
//  pwfx, pvData, and cbData
// ----------------------------------------------------------------------------------
BOOL 
CDlgSrc::AquireData
(
    WAVEFORMATEX*   pwfx, 
    PVOID           pvData,
    ULONG           cbData, 
    UINT            nChannelFrom
)
{
    ASSERT( pwfx );

    if (! (pwfx && (pwfx->cbSize == 0) && pvData && cbData))
        return FALSE;

    CopyMemory(&m_wfx, pwfx, sizeof(m_wfx));

	GUID	guid = (( WAVEFORMATEXTENSIBLE * )pwfx)->SubFormat;

	if( IS_VALID_WAVEFORMATEX_GUID( &guid ) )
	{
		m_wfx.wFormatTag = EXTRACT_WAVEFORMATEX_ID( &guid );
	}
	//
	//	else no op
	//
    m_wfx.nChannels = 1;
    m_wfx.nBlockAlign = m_wfx.wBitsPerSample / 8;
    m_wfx.nAvgBytesPerSec = m_wfx.nBlockAlign * m_wfx.nSamplesPerSec;

    // a little more UI
    char sz[100];

	StringCchPrintfA(sz, 100, "%d", pwfx->wBitsPerSample);
    SetDlgItemText(IDC_BITDEPTH, sz);

	StringCchPrintfA(sz, 100, "%d", pwfx->nSamplesPerSec);
    SetDlgItemText(IDC_SAMPLERATE, sz);

    //
    // extract nChannelFrom-th channel from the data
    // should be pretty much a no-op for already-mono formats
    //
    ULONG   nSample;
	USHORT	nBytes = pwfx->wBitsPerSample / 8;

    m_cSamples = cbData / pwfx->nBlockAlign;
    m_cbData = m_wfx.nBlockAlign * m_cSamples;
    m_pvData = LocalAlloc(LPTR, m_cbData);
    if( 0 == m_pvData)
	{
		MessageBox( "Insufficient memory to complete the task.", "MultiChan : Error", MB_OK | MB_ICONSTOP );
		return( FALSE );
	}

	//
	//  Adding support for multi-channel input	
	//
	if( 1 == pwfx->nChannels )
	{
		//
		//	trivial copy
		//
        ASSERT(m_cbData == cbData);
		CopyMemory( m_pvData, pvData, m_cbData );
	}
	else
	{
		//
		//	m channels, n bit depth
		//	extract out every other m-th group of n bits
		//	repeat for m_cSamples
		//
		BYTE * pbDest = (BYTE * )m_pvData;
		BYTE * pbSrc = ( BYTE * )pvData;

		//	
		//	start with the appropriate channel; channels are 0-based
		//
		pbSrc+= nChannelFrom * ( pwfx->wBitsPerSample / 8 );

		for( nSample = 0; nSample < m_cSamples; nSample++ )
		{
			USHORT	nCount = nBytes;
			while( nCount-- )
				*pbDest++ = *pbSrc++;

			pbSrc+= (pwfx->nChannels - 1) * nBytes;
		}
	}

	return( TRUE );
}   //  CDglSrc::AcquireData

/*
 *  UI triggered procedures
 */

// ----------------------------------------------------------------------------------
// OnComboSpeaker
//  update speaker mask of dst
// ----------------------------------------------------------------------------------
void CDlgSrc::OnComboSpeaker() 
{
    //
    //  prepare
    //
    int     nSpeaker = m_nSpeaker;

    UpdateData(TRUE);

    //
    //  make selection
    //
    m_dwChannelMask = ( DWORD )m_comboSpeaker.GetItemData(m_nSpeaker);

    //
    //  validate selection
    //
    if(! ((CChildView*)m_pwndParent)->UpdateDialogs())
    {
        MessageBox("There is already a channel with that speaker config", "MultiChan : Error!", MB_ICONSTOP | MB_OK);
        m_nSpeaker = nSpeaker;
        UpdateData( FALSE );
    }
}

// ----------------------------------------------------------------------------------
// OnPlay
//  preview button
// ----------------------------------------------------------------------------------
void CDlgSrc::OnPlay() 
{
	MMRESULT	mmRes = MMSYSERR_NOERROR;

    //
    //  ensure validity
    //
	ASSERT( 0 == g_hwo );
    ASSERT( m_fPlayable );

    //
    //  first usage ?
    //
    if( 0 == m_lpwhdr )
    {
        m_lpwhdr = ( LPWAVEHDR )LocalAlloc( LPTR, sizeof( WAVEHDR ) );
        if( 0 == m_lpwhdr )
        {
            MessageBox( "Insufficient memory for rendering !", "MultiChan : Error", MB_ICONEXCLAMATION | MB_OK );
            return;
        }
    }

    m_lpwhdr->dwBufferLength = m_cbData;
    m_lpwhdr->lpData = (char*)m_pvData;

    //
    //  using a callback mechanism to allow synch playback
    //
    mmRes = waveOutOpen(    &g_hwo, 
                            WAVE_MAPPER, 
                            &m_wfx, 
                            ( DWORD_PTR )WavePlayFileCB, 
                            ( DWORD_PTR )this, 
                            CALLBACK_FUNCTION
                        );
	if( TrapMMError(mmRes, "waveOutOpen") )
	{
		mmRes = waveOutPrepareHeader( g_hwo, m_lpwhdr, sizeof(WAVEHDR) );
		if( TrapMMError(mmRes, "waveOutPrepareHeader") )
		{
            //
            //  revert playable state
			//	disable any other dialog
            //
			WaveTogglePlayback( this, WAVE_TOGGLE_DISABLE );

            //
            //  start singing
            //
			mmRes = waveOutWrite( g_hwo, m_lpwhdr, sizeof(WAVEHDR));
			TrapMMError(mmRes, "waveOutWrite");
        }   //  prepare header
    }       //  open
}           //  CDlgSrc::OnPlay

//  -------------------------------------------------------------------------------------
//  OnStop
//
//  -------------------------------------------------------------------------------------
inline
void 
CDlgSrc::OnStop
()
{
    MMRESULT    mmRes = MMSYSERR_NOERROR;

    //  
    //  ensure validity
    //  
    ASSERT( g_hwo );
    ASSERT( !m_fPlayable );

    //
    //  stop playback
    //
    mmRes = waveOutReset( g_hwo );
    TrapMMError( mmRes, "waveOutReset" );
    
}   //  CDglSrc::OnStop

// ----------------------------------------------------------------------------------
// dummy implementations to prevent dlgs from disappearing when user hits enter or esc
// ----------------------------------------------------------------------------------
inline
void 
CDlgSrc::OnOK
() 
{
    POSITION pos = g_listSources.Find(this, NULL);
    g_listSources.RemoveAt(pos);
    g_pChildView->UpdateDialogs();
    delete this;
}   //  CDglSrc::OnOK

//
//
//
inline
void 
CDlgSrc::OnCancel
() 
{
    POSITION pos = g_listSources.Find(this, NULL);
    g_listSources.RemoveAt(pos);
    g_pChildView->UpdateDialogs();
    delete this;
}   //  CDglSrc::OnCancel

//
//
//
HBRUSH 
CDlgSrc::OnCtlColor
(
    CDC* pDC, 
    CWnd* pWnd, 
    UINT nCtlColor
) 
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

    if((nCtlColor == CTLCOLOR_STATIC))
    {
        if(pWnd->m_hWnd == m_cInput.m_hWnd)
        {
            pDC->SetTextColor(GetSysColor(COLOR_CAPTIONTEXT));
            pDC->SetBkMode(TRANSPARENT);  //BkColor(GetSysColor(COLOR_ACTIVECAPTION));
            return GetSysColorBrush(COLOR_ACTIVECAPTION);
        }
    }
    
	return hbr;
}   //  CDglSrc::OnCtlColor

//
//
//
void 
CDlgSrc::OnLButtonDown
(
    UINT nFlags, 
    CPoint point
) 
{
	RECT    rcClient;
    
    m_cInput.GetClientRect(&rcClient);
    m_cInput.MapWindowPoints(this, &rcClient);
    m_fDragging = PtInRect(&rcClient, point);

    if(m_fDragging)
    {
        SetCapture();
        m_fDragging = TRUE;
        m_ptPosInCaption = point;
    }

    CDialog::OnLButtonDown(nFlags, point);

    SetFocus();
    SetWindowPos(&wndTop, 0,0,0,0, SWP_NOMOVE | SWP_NOSIZE);
}   //  CDglSrc::OnLButtonDown

//
//
//
void 
CDlgSrc::OnLButtonUp
(
    UINT nFlags, 
    CPoint point
) 
{
	m_fDragging = FALSE;
    ReleaseCapture();

	CDialog::OnLButtonUp(nFlags, point);
}   //  CDglSrc::OnLButtonUp

//
//
//
void 
CDlgSrc::OnMouseMove
(
    UINT nFlags, 
    CPoint point
) 
{
	RECT    rcClientParent;

    GetParent()->GetClientRect(&rcClientParent);
    GetParent()->MapWindowPoints(this, &rcClientParent);
    if(PtInRect(&rcClientParent, point))
    {
        if(m_fDragging)
        {
            CRect   rcWindow;
            CRect   rcParent;
            CPoint  pt(point.x - m_ptPosInCaption.x, point.y - m_ptPosInCaption.y);

            GetWindowRect(&rcWindow);
            GetParent()->GetWindowRect(&rcParent);

            rcWindow.OffsetRect(pt);
            rcWindow -= rcParent.TopLeft();

            SetWindowPos(NULL, rcWindow.left, rcWindow.top, 0,0, SWP_NOZORDER | SWP_NOSIZE);
        }
    }
    else
    {
        ReleaseCapture();
        m_fDragging = FALSE;
    }
}   //  CDglSrc::OnMouseMove

//
//
//
BOOL 
CDlgSrc::OnInitDialog
() 
{
	CDialog::OnInitDialog();
	
    HICON hIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_CHECK));
    m_butClose.SetIcon(hIcon);	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}   //  CDlgSrc::OnInitDialog


//
//
//
LRESULT 
CDlgSrc::WindowProc
(
    UINT    nMessage,
    WPARAM  wParam,
    LPARAM  lParam
)
{
    MMRESULT    mmRes = MMSYSERR_NOERROR;

    switch( nMessage )
    {
    case    WM_START_PLAYBACK:
        {
            break;
        }
    case    WM_STOP_PLAYBACK:
        {
            //
            //  revert playable state
            //
            WaveTogglePlayback( this, WAVE_TOGGLE_ALLOW );

            //
            //  unprepare and release
            //
            mmRes = waveOutUnprepareHeader( g_hwo, m_lpwhdr, sizeof( WAVEHDR ) );
            if( TrapMMError( mmRes, "waveOutUnprepareHeader" ) )
            {
                mmRes = waveOutClose( g_hwo );
                TrapMMError( mmRes, "waveOutClose" );

                g_hwo = 0;
            }
            break;
        }
    }
    return( CDialog::WindowProc( nMessage, wParam, lParam ) );
}   //  CDlgSrc::WindowProc