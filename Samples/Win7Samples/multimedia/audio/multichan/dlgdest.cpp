// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.


#include "stdafx.h"
#include "multichan.h"
#include "DlgDest.h"
#include "dlgsrc.h"
#include "childview.h"
#include <initguid.h>
#include <mmreg.h>
#include <msacm.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/*
 *	CDlgDest	
 *	MFC stuff
 */

/////////////////////////////////////////////////////////////////////////////
// CDlgDest dialog
void 
CDlgDest::DoDataExchange
(
	CDataExchange* pDX
)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgDest)
	DDX_Control(pDX, IDC_SPEAKERFLAGS, m_cChannelMask);
	DDX_Control(pDX, IDC_AVEBITS, m_cAveBitsPerSec);
	DDX_Control(pDX, IDC_REMIX, m_cRemix);
	DDX_Control(pDX, IDC_EDIT_VALIDBITS, m_cValidBits);
	DDX_Control(pDX, IDC_CHANNELS, m_cChannels);
	DDX_Control(pDX, IDC_OUTPUT, m_cOutput);
	DDX_Control(pDX, IDC_COMBO_BITDEPTH, m_comboBitDepth);
	DDX_Control(pDX, IDC_COMBO_SAMPLERATE, m_comboSampleRate);
	DDX_Control(pDX, IDC_COMBO_WAVEFORMAT, m_comboFormat);
	DDX_Control(pDX, IDC_PLAY, m_butPlay);
    DDX_Control(pDX, IDC_STOP, m_butStop);
	DDX_Text(pDX, IDC_EDIT_VALIDBITS, m_wValidBitsPerSample);
	DDV_MinMaxUInt(pDX, m_wValidBitsPerSample, 0, 65535);
	DDX_Text(pDX, IDC_SPEAKERFLAGS, m_strChannelMask);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgDest, CDialog)
	//{{AFX_MSG_MAP(CDlgDest)
	ON_BN_CLICKED(IDC_REMIX, OnRemix)
	ON_BN_CLICKED(IDC_PLAY, OnPlay)
    ON_BN_CLICKED(IDC_STOP, OnStop)
	ON_CBN_SELENDOK(IDC_COMBO_SAMPLERATE, OnComboSamplerate)
	ON_CBN_SELENDOK(IDC_COMBO_WAVEFORMAT, OnComboWaveformat)
	ON_CBN_SELENDOK(IDC_COMBO_BITDEPTH, OnComboBitdepth)
	ON_EN_KILLFOCUS(IDC_EDIT_VALIDBITS, OnEditValidbits)
	ON_WM_CTLCOLOR()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/*
 *	CDlgDest	
 *	construction
 */

// ----------------------------------------------------------------------------------
// constructor
// ----------------------------------------------------------------------------------
CDlgDest::CDlgDest
(
	CWnd* pParent /*=NULL*/
)
  : CDialog(CDlgDest::IDD, pParent),
    m_pwndParent(pParent),
    m_pbData(NULL),
    m_fPlayable(FALSE),
    m_fDragging(FALSE),
	m_cbData( 0 ),
    m_lpwhdr( 0 )
{
	//{{AFX_DATA_INIT(CDlgDest)
	m_wValidBitsPerSample = 0;
	m_strChannelMask = _T("0x00000000");
	//}}AFX_DATA_INIT

    m_wfExt.Format.wFormatTag = WAVE_FORMAT_PCM;
    m_wfExt.Format.cbSize = 0;
    m_wfExt.Format.wBitsPerSample = 16;
    m_wfExt.Format.nChannels = 0;
    m_wfExt.Format.nBlockAlign = 4;
    m_wfExt.Format.nSamplesPerSec = 44100;
    m_wfExt.Format.nAvgBytesPerSec = m_wfExt.Format.nBlockAlign * m_wfExt.Format.nSamplesPerSec;
    m_wfExt.Samples.wValidBitsPerSample = 16;
    m_wfExt.dwChannelMask = 0;
    m_wfExt.SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
    m_wValidBitsPerSample = m_wfExt.Samples.wValidBitsPerSample;
}

// ----------------------------------------------------------------------------------
// destructor
// ----------------------------------------------------------------------------------
CDlgDest::~CDlgDest
()
{
	SafeLocalFree( m_pbData );
    SafeLocalFree( m_lpwhdr );
}

// ----------------------------------------------------------------------------------
// Create
// ----------------------------------------------------------------------------------
void 
CDlgDest::Create
(
    void
)  
{
    CDialog::Create(CDlgDest::IDD, m_pwndParent);
}

// ----------------------------------------------------------------------------------
// OnInitDialog
// ----------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------
CONST_GUID_REP cgrFormats[] =
{
    &GUID_NULL,                 "GUID_NULL",
    &KSDATAFORMAT_SUBTYPE_PCM,  "KSDATAFORMAT_SUBTYPE_PCM"
};

UINT    rgnSampleRate[]	= { 8000, 11025, 16000, 22050, 32000, 44100, 48000, 96000 };
UINT    rgnBitDepth[]	= { 8, 16, 24, 32 };

BOOL 
CDlgDest::OnInitDialog
() 
{
	CDialog::OnInitDialog();

    int c,i;

    i = m_comboFormat.AddString("WAVE_FORMAT_PCM");
    m_comboFormat.SetItemData(i, WAVE_FORMAT_PCM);
    i = m_comboFormat.AddString("WAVE_FORMAT_EXTENSIBLE");
    m_comboFormat.SetItemData(i, WAVE_FORMAT_EXTENSIBLE);

    char sz[10];
    for(c = 0; c < ARRAY_ELEMENTS(rgnSampleRate); c++)
    {
        StringCchPrintfA(sz, 10, "%d", rgnSampleRate[c]);
        i = m_comboSampleRate.AddString(sz);
        m_comboSampleRate.SetItemData(i, rgnSampleRate[c]);
    }

    for(c = 0; c < ARRAY_ELEMENTS(rgnBitDepth); c++)
    {
        StringCchPrintfA(sz, 10, "%d", rgnBitDepth[c]);
        i = m_comboBitDepth.AddString(sz);
        m_comboBitDepth.SetItemData(i, rgnBitDepth[c]);
    }

    m_comboSampleRate.SelectString(0, "44100");
    m_comboBitDepth.SelectString(0, "16");
    m_comboFormat.SelectString(0, "WAVE_FORMAT_PCM");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

/*
 *	CDlgDest	
 *	operations
 */

// ----------------------------------------------------------------------------------
// OnRemix
//	traverses the list of source channels and mixes them according to the user settings
// ----------------------------------------------------------------------------------
void 
CDlgDest::OnRemix
() 
{
    CDlgSrc *	pdlgSrc		= 0;
    ULONG       cMaxSamples = 0;
    UINT        nSample;
    ULONG       cbSample;
    POSITION    pos;

    // move over, bacon
    SafeLocalFree(m_pbData);

    //
    // update UI
    //
    Update();

    //
    // determine the max number of samples
    //
    ULONG   cSamplesSrcThisFormat = 0;
	for
	(
		pos = g_listSources.GetHeadPosition();
		pos;
	)
    {
        pdlgSrc = g_listSources.GetNext(pos);

        cSamplesSrcThisFormat = MulDiv(
											m_wfExt.Format.nSamplesPerSec, 
											pdlgSrc->m_cSamples, 
											pdlgSrc->m_wfx.nSamplesPerSec
									);
        cMaxSamples = max(cMaxSamples, cSamplesSrcThisFormat);
    }

    //
	//	determine the length of the buffer required 
	//
    cbSample = m_wfExt.Format.nChannels * m_wfExt.Format.wBitsPerSample / 8;
    m_cbData = cbSample * cMaxSamples;
    m_pbData = (PBYTE)LocalAlloc(LPTR, m_cbData);
    if( 0 == m_pbData)
	{
		MessageBox( "Insufficient memory to complete the task.", "MultiChan : Error", MB_OK | MB_ICONSTOP );
		return;
	}

    //
    // copy data from each src to the dst
    //
	
    MMRESULT        mmRes;
    HACMSTREAM      has;
    ULONG           cbConversion;
    PVOID           pvConversion;
    ACMSTREAMHEADER ash;
    ULONG           cSamplesOut;
    BOOL            fRes = FALSE;
    WAVEFORMATEX    wfxConversion;

	wfxConversion.wFormatTag		= WAVE_FORMAT_PCM;
	wfxConversion.cbSize			= 0;
	wfxConversion.nChannels			= 1;
	wfxConversion.nSamplesPerSec	= m_wfExt.Format.nSamplesPerSec;
	wfxConversion.wBitsPerSample	= m_wfExt.Format.wBitsPerSample;
	wfxConversion.nBlockAlign		= wfxConversion.wBitsPerSample / 8;
	wfxConversion.nAvgBytesPerSec	= wfxConversion.nBlockAlign * wfxConversion.nSamplesPerSec;

	//
	// for each SRC (src dialog), convert it's buffer into 1 channel of the Dst format
	//
	for
	(
	    pos = g_listSources.GetHeadPosition();
		pos;
	)
    {
        pdlgSrc = g_listSources.GetNext(pos);

        // 
        // first, convert dlg format to dest format using ACM
        //
		
        // open an appropriate ACM driver
        mmRes = 
            acmStreamOpen
            (
                &has,
                NULL,
                &pdlgSrc->m_wfx,
                &wfxConversion,
                NULL,   //wFilter,
                NULL,
                0,
                ACM_STREAMOPENF_NONREALTIME 
            );

        if(!TrapMMError(mmRes, "acmStreamOpen"))
            break;
		
		//
        //	how big a buffer do we need to convert this one channel?
		//	in any case, acm overestimates the size of the buffer needed 
		//	if the acm size is used when copying we will exceed the boundaries of our allocated m_pbData
		//	
        mmRes = 
            acmStreamSize
            (
                has,
                pdlgSrc->m_cbData,
                &cbConversion,
                ACM_STREAMSIZEF_SOURCE
            );

        if(!TrapMMError(mmRes, "acmStreamSize"))
            break;

        // alloc
        pvConversion = LocalAlloc(LPTR, cbConversion);
        if(!(fRes = (pvConversion != NULL)))
        {
            MessageBox("Not enough memory for ACM operation", "MultiChan : Error!", MB_ICONEXCLAMATION | MB_OK);
            break;
        }

        // be prepared!
		ZeroMemory(&ash, sizeof(ACMSTREAMHEADER));
		ash.cbStruct	= sizeof(ACMSTREAMHEADER);
		ash.pbSrc		= (PBYTE)pdlgSrc->m_pvData;
		ash.cbSrcLength = pdlgSrc->m_cbData;      
		ash.pbDst		= (PBYTE)pvConversion;
		ash.cbDstLength = cbConversion;

        mmRes = acmStreamPrepareHeader(has, &ash, 0);
        if(!TrapMMError(mmRes, "acmStreamPrepareHeader"))
            break;

        // do it
        mmRes = acmStreamConvert(has, &ash, 0);
        if(!TrapMMError(mmRes, "acmStreamConvert"))
            break;

        // clean up
        mmRes = acmStreamUnprepareHeader(has, &ash, 0) |
                acmStreamClose(has, 0);
        if(!TrapMMError(mmRes, "acmStreamUnprepareHeader"))
            break;

        //
        // now copy the data from conversion buffer to final output buffer
        //
        cSamplesOut =	( cbConversion > m_cbData ) ? 
						( m_cbData / wfxConversion.nBlockAlign ) : 
						( cbConversion / wfxConversion.nBlockAlign );

        switch(wfxConversion.nBlockAlign)
        {
            case 1:     // 8-bit
            {
                // stagger dest
                PBYTE   pbDst = m_pbData + pdlgSrc->m_nChannel;
                PBYTE   pbSrc = (PBYTE)pvConversion;

                for(nSample = 0; nSample < cSamplesOut; nSample++)
                {
                    *pbDst = *pbSrc++;
                    pbDst += m_wfExt.Format.nChannels;
                }

                break;
            }

            case 2:     // 16-bit
            {
                // stagger dest
                PUSHORT pusDst = ((PUSHORT)m_pbData) + pdlgSrc->m_nChannel;
                PUSHORT pusSrc = (PUSHORT)pvConversion;

                for(nSample = 0; nSample < cSamplesOut; nSample++)
                {
                    *pusDst = *pusSrc++;
                    pusDst += m_wfExt.Format.nChannels;
                }

                break;
            }

            case 3:     // 24-bit
            {
                typedef struct {
                    BYTE b[3];
                } S24BITS, *PS24BITS;

                // stagger dest
                PS24BITS psDst = ((PS24BITS)m_pbData) + pdlgSrc->m_nChannel;
                PS24BITS psSrc = (PS24BITS)pvConversion;

                for(nSample = 0; nSample < cSamplesOut; nSample++)
                {
                    *psDst = *psSrc++;
                    psDst += m_wfExt.Format.nChannels;
                }

                break;
            }

            default:
                ASSERT(0);
        }

        // clean up
        SafeLocalFree( pvConversion );
	}

    if(fRes)
    {
        // make playable
        m_fPlayable = TRUE;
        m_butPlay.EnableWindow(m_fPlayable);
    }
}

// ----------------------------------------------------------------------------------
// OnPlay
// ----------------------------------------------------------------------------------
void 
CDlgDest::OnPlay
() 
{
	MMRESULT	mmRes       = MMSYSERR_NOERROR;

    //
    //  must not be in use
    //
    ASSERT( 0 == g_hwo );

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
    m_lpwhdr->lpData = ( char * )m_pbData;

    //
    //  open wave device
    //
    mmRes = waveOutOpen(    &g_hwo,							//  handle of the output device
                            WAVE_MAPPER,					//  as appropriate
                            ( WAVEFORMATEX* )&m_wfExt,		//  describe what's next
                            ( DWORD_PTR )WavePlayFileCB,	//  the callback
                            ( DWORD_PTR )this,				//  object issuing the request
                            CALLBACK_FUNCTION				//  callback via function
                       );
	if(TrapMMError(mmRes, "waveOutOpen"))
	{
        //
        //  prepare the header
        //
		mmRes = waveOutPrepareHeader(g_hwo, m_lpwhdr, sizeof(WAVEHDR));
		if(TrapMMError(mmRes, "waveOutPrepareHeader"))
		{
            //
            //  revert playable state
            //
            WaveTogglePlayback( WAVE_TOGGLE_DESTINATION, WAVE_TOGGLE_DISABLE );

            //
            //  start singing
            //
			mmRes = waveOutWrite(g_hwo, m_lpwhdr, sizeof(WAVEHDR));
			TrapMMError(mmRes, "waveOutWrite");

        }       //  prepare header
    }           //  open
}               //	OnPlay

// ----------------------------------------------------------------------------------
// OnStop
// ----------------------------------------------------------------------------------
void 
CDlgDest::OnStop
()
{
    MMRESULT    mmRes       = MMSYSERR_NOERROR;

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

}   //  OnStop

//
//	RecalcDependant
//
void 
CDlgDest::RecalcDependantVariables
()
{
    m_wfExt.Format.nBlockAlign		= m_wfExt.Format.wBitsPerSample * m_wfExt.Format.nChannels / 8;
    m_wfExt.Format.nAvgBytesPerSec	= m_wfExt.Format.nBlockAlign * m_wfExt.Format.nSamplesPerSec;

	// update UI
    char sz[100];
    StringCchPrintfA(sz, 100, "%d", m_wfExt.Format.nAvgBytesPerSec);
    SetDlgItemText(IDC_AVEBITS, sz);
}


/*
 *	CDlgDest	
 *	UI operations
 */


// ----------------------------------------------------------------------------------
// track combobox changes
// ----------------------------------------------------------------------------------
void 
CDlgDest::OnComboSamplerate
() 
{
    m_wfExt.Format.nSamplesPerSec = ( DWORD )m_comboSampleRate.GetItemData(m_comboSampleRate.GetCurSel());	
    RecalcDependantVariables();
}

void 
CDlgDest::OnComboBitdepth
() 
{
    m_wfExt.Format.wBitsPerSample = (USHORT)m_comboBitDepth.GetItemData(m_comboBitDepth.GetCurSel());	
    m_wfExt.Samples.wValidBitsPerSample = min(m_wfExt.Samples.wValidBitsPerSample, m_wfExt.Format.wBitsPerSample);
    m_wValidBitsPerSample = m_wfExt.Samples.wValidBitsPerSample;
    RecalcDependantVariables();
    UpdateData(FALSE);      // update m_wValidBitsPerSample display
}

void 
CDlgDest::OnComboWaveformat
() 
{
    m_wfExt.Format.wFormatTag = (WORD)m_comboFormat.GetItemData(m_comboFormat.GetCurSel());	
    switch(m_wfExt.Format.wFormatTag)
    {
        case WAVE_FORMAT_PCM:
            m_wfExt.Format.cbSize = 0;
            break;

        case WAVE_FORMAT_IEEE_FLOAT:
            m_wfExt.Format.cbSize = 0;
            break;

        case WAVE_FORMAT_EXTENSIBLE:
            m_wfExt.Format.cbSize = sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX);
            break;

        default:
            MessageBox("Format type not supported", "MultiChan : Warning", MB_ICONWARNING | MB_OK);
            break;
    }
}

// ----------------------------------------------------------------------------------
// Update
// ----------------------------------------------------------------------------------
void 
CDlgDest::Update
()
{
    // update dwChannelMask
    CDlgSrc*    pdlgSrc		= 0;
    BOOL        fGotSources = (0 != g_listSources.GetCount() );

    m_cRemix.EnableWindow( fGotSources );
    m_cValidBits.EnableWindow( fGotSources );
    m_cChannels.EnableWindow( fGotSources );
    m_cOutput.EnableWindow( fGotSources );
    m_comboBitDepth.EnableWindow( fGotSources );
    m_comboSampleRate.EnableWindow( fGotSources );
    m_cAveBitsPerSec.EnableWindow( fGotSources );
    m_cChannelMask.EnableWindow( fGotSources );
    m_comboFormat.EnableWindow( fGotSources );
    m_butPlay.EnableWindow( fGotSources  && m_pbData );
    m_butStop.EnableWindow( !fGotSources  );

    //
    //  setting channel mask - if SPEAKER_NOT_SPECIFIED, don't mind
    //
    m_wfExt.dwChannelMask = 0;

	for
	(	
		POSITION    pos		= g_listSources.GetHeadPosition();
		pos;
	)
	{
        //
		//	iterative step
		//
		pdlgSrc = g_listSources.GetNext(pos);

        if( !(SPEAKER_NOT_SPECIFIED & pdlgSrc->m_dwChannelMask) )
            m_wfExt.dwChannelMask |= pdlgSrc->m_dwChannelMask;    
	}	//	for

    m_wfExt.dwChannelMask &= 0x7fffffff;        //	correct any deviations

	//
	// set up a proper WAVEFORMATEXTENSIBLE (number of channels = the number of SRC dialogs that were created)
	//
	m_wfExt.Format.nChannels = (WORD)g_listSources.GetCount();
	if(m_wfExt.Format.nChannels > 2)
	{
		m_wfExt.Format.wFormatTag	= WAVE_FORMAT_EXTENSIBLE;
		m_wfExt.Format.cbSize		= sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX);
		m_comboFormat.SelectString(0, "WAVE_FORMAT_EXTENSIBLE");
	}

	RecalcDependantVariables();
	m_wValidBitsPerSample = m_wfExt.Samples.wValidBitsPerSample;

	// update UI
	char sz[100];
	StringCchPrintfA(sz, 100, "%d", m_wfExt.Format.nChannels);
	SetDlgItemText(IDC_CHANNELS, sz);

	m_strChannelMask.Format("0x%08x", m_wfExt.dwChannelMask);
	UpdateData(FALSE);

}

void 
CDlgDest::OnEditValidbits
() 
{
    UpdateData(TRUE);
    m_wfExt.Samples.wValidBitsPerSample = LOWORD(m_wValidBitsPerSample);
}

// ----------------------------------------------------------------------------------
// dummy implementations to prevent dlgs from disappearing when user hits enter or esc
// ----------------------------------------------------------------------------------
void CDlgDest::OnOK() 
{
    // do nothing
}

void CDlgDest::OnCancel() 
{
    // do nothing
}

HBRUSH 
CDlgDest::OnCtlColor
(
	CDC*	pDC, 
	CWnd*	pWnd, 
	UINT	nCtlColor
) 
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

    if((nCtlColor == CTLCOLOR_STATIC))
    {
        if(pWnd->m_hWnd == m_cOutput.m_hWnd)
        {
            pDC->SetBkColor(GetSysColor(COLOR_ACTIVECAPTION));
            pDC->SetTextColor(GetSysColor(COLOR_CAPTIONTEXT));
            return GetSysColorBrush(COLOR_ACTIVECAPTION);
        }
    }
    
	return hbr;
}

void 
CDlgDest::OnLButtonDown
(
	UINT	nFlags, 
	CPoint	point
) 
{
	RECT    rcClient;
    
    m_cOutput.GetClientRect(&rcClient);
    m_cOutput.MapWindowPoints(this, &rcClient);
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
}

void 
CDlgDest::OnLButtonUp
(
	UINT	nFlags, 
	CPoint	point
) 
{
	m_fDragging = FALSE;
    ReleaseCapture();

	CDialog::OnLButtonUp(nFlags, point);
}

void 
CDlgDest::OnMouseMove
(
	UINT	nFlags, 
	CPoint	point
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
}

/*
 *	CDlgDest	
 *	message processing
 */


LRESULT 
CDlgDest::WindowProc
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
                WaveTogglePlayback( WAVE_TOGGLE_DESTINATION, WAVE_TOGGLE_ALLOW );

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
    return( CWnd::WindowProc( nMessage, wParam, lParam ) );
}