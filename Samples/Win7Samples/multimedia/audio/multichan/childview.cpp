// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.


#include "stdafx.h"
#include "multichan.h"
#include "dlgdest.h"
#include "ChildView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CChildView*                 g_pChildView    = 0;
CDlgDest*                   g_pdlgDest      = 0;
CList<CDlgSrc*,CDlgSrc*>    g_listSources;
HWAVEOUT                    g_hwo           = 0;

/////////////////////////////////////////////////////////////////////////////
// CChildView

CChildView::CChildView()
  : m_cChannels(0)
{
    m_sizeInitPos.cx = 100;
    m_sizeInitPos.cy = 40;

    g_pChildView = this;
}

CChildView::~CChildView()
{}


BEGIN_MESSAGE_MAP(CChildView,CWnd )
	//{{AFX_MSG_MAP(CChildView)
	ON_WM_PAINT()
	ON_COMMAND(IDM_MIXER_MERGE, OnMixerMerge)
	ON_UPDATE_COMMAND_UI(IDM_MIXER_MERGE, OnUpdateMixerMerge)
	ON_COMMAND(IDM_MIXER_PLAY, OnMixerPlay)
	ON_UPDATE_COMMAND_UI(IDM_MIXER_PLAY, OnUpdateMixerPlay)
    ON_COMMAND( IDM_MIXER_STOP, OnMixerStop )
    ON_UPDATE_COMMAND_UI( IDM_MIXER_STOP, OnUpdateMixerStop )
	ON_COMMAND(ID_FILE_SAVE, OnFileSave)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE, OnUpdateFileSave)
	ON_EN_SETFOCUS(IDC_EDIT_VALIDBITS, OnSetfocusEditValidbits)
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CChildView message handlers

// ----------------------------------------------------------------------------------------
BOOL CChildView::PreCreateWindow(CREATESTRUCT& cs) 
{
	if (!CWnd::PreCreateWindow(cs))
		return FALSE;

	cs.dwExStyle |= WS_EX_CLIENTEDGE;
	cs.style &= ~WS_BORDER;
	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, 
		::LoadCursor(NULL, IDC_ARROW), HBRUSH(COLOR_APPWORKSPACE+1), NULL);

	return TRUE;
}

// ----------------------------------------------------------------------------------------
void CChildView::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
}

// ----------------------------------------------------------------------------------
// dlgproc
// ----------------------------------------------------------------------------------
BOOL
dlgproc
(
    HWND hwndDlg,  // handle to dialog box
    UINT uMsg,     // message
    WPARAM wParam, // first message parameter
    LPARAM lParam  // second message parameter
)
{
    return 0;
}

// ----------------------------------------------------------------------------------------
void CChildView::OnFileOpen() 
{
    DWORD           dwThreadID  = 0;

    //
    // std file open dialog
    //
    CFileDialog dlg(    TRUE, 
                        "*.wav", 
                        NULL, 
                        OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST, 
                        "Wave Files (*.wav)|*.wav|All Files (*.*)|*.*||"
                   );
    if(IDOK != dlg.DoModal())
    {
        return;
    }

    //
    // parse the wave file
    //
    LPWAVEFORMATEX			pwfx	= 0;
    ULONG					cbData	= 0;
    HANDLE					hFile	= INVALID_HANDLE_VALUE;

    ZeroMemory(&pwfx, sizeof(WAVEFORMATEX));

	hFile = WaveOpenFile(dlg.GetPathName(), &pwfx, &cbData);

	ASSERT( 0 != pwfx );

    if(IsValidHandle(hFile) && pwfx && cbData)
    {

        ULONG   cbRead;
        PVOID   pvData = LocalAlloc(LPTR, cbData);
		if( 0 == pvData )
		{
			MessageBox( "Insufficient memory to complete the task.", "MultiChan : Error", MB_OK | MB_ICONSTOP );
			return;
		}

        //
        // read the data into a buffer
        //
        if
        (
            pvData &&
            ReadFile
            (
                hFile,
                pvData,
                cbData,
                &cbRead,
                NULL
            )
        )
        {
            m_cChannels = ( int )g_listSources.GetCount();

            //
            // create a src dialog for each channel in the wave file
            //
            for(int c = 0; c < pwfx->nChannels; c++)
            {
                char    szTitle[MAX_PATH];
                DWORD   dwChanMsk = 0;

                CDlgSrc* pdlgSrcNew = new CDlgSrc(this);
                g_listSources.AddTail(pdlgSrcNew);

                //
                // find an unused speaker mask
                //

                //
                //  currently only 18 speaker positions are defined
                //  any channel above 18 will be assigned a non specified speaker
                //  it is the sole responsibility of the driver to deal with 
                //  unspecified speaker masks
                //
                if(g_pdlgDest)
                {
                    for
					(	
						dwChanMsk = SPEAKER_FRONT_LEFT; 
                        dwChanMsk & g_pdlgDest->m_wfExt.dwChannelMask; 
                        dwChanMsk <<= 1
					)
                            ;

                    if( SPEAKER_NOT_SPECIFIED != dwChanMsk )
                        g_pdlgDest->m_wfExt.dwChannelMask |= dwChanMsk;

                    //
                    //  g_pdlgDest->m_wfExt.dwChannelMask &= 0x7fffffff;
                    //
                }
                else
                {
                    if( c >= SPEAKERS_USED )
                    {
                        dwChanMsk = SPEAKER_NOT_SPECIFIED;
                    }   
                    else
                    {
                        dwChanMsk = cdrSpeakers[c].dwConstant;
                    }
                }

                //
                // create the dialog
                //
                StringCchPrintfA(szTitle, MAX_PATH, "\"%s\" channel %d of %d", dlg.GetFileTitle(), c, pwfx->nChannels);
                pdlgSrcNew->Create(szTitle, m_sizeInitPos, dwChanMsk);

				//
                // pull the c-th channel of data out of this wave file
				//	also treat return codes
				//
                if( !pdlgSrcNew->AquireData(pwfx, pvData, cbData, c) )
				{
					MessageBox( "Only max 2 channel PCM formats are accepted as input.", "MultiChan : Error", MB_OK | MB_ICONSTOP );
					break;
				}

                //
                // the next dialog created should be cascaded
                //
                m_sizeInitPos.cx += 32;
                m_sizeInitPos.cy += 32;
            }

            //
            // make sure the src dialogs are consistent
            //
            UpdateDialogs();
        }

        SafeLocalFree(pvData);
    }

    SafeLocalFree(pwfx);
    SafeCloseHandle(hFile);
}

// ----------------------------------------------------------------------------------------
void CChildView::OnFileSave() 
{
    // std file open dialog
    CFileDialog dlg(FALSE, "*.wav", NULL, OFN_PATHMUSTEXIST, "Wave Files (*.wav)|*.wav|All Files (*.*)|*.*||");

    if(IDOK == dlg.DoModal())
    {
        WaveSaveFile(dlg.GetFileName(), (WAVEFORMATEX*)&g_pdlgDest->m_wfExt, g_pdlgDest->m_pbData, g_pdlgDest->m_cbData);
    }
}

// ----------------------------------------------------------------------------------------
void CChildView::OnUpdateFileSave(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(g_pdlgDest && g_pdlgDest->m_fPlayable);
}

// ----------------------------------------------------------------------------------------
BOOL CChildView::UpdateDialogs(void)
{
    //
    // set channel numbers for each SRC
    //
    ULONG       nChannel = 0;
    DWORD       dwBit;
    DWORD       dwChanMsk = 0;
    POSITION    pos;
    CDlgSrc*    pdlgSrc;

    // make a bitmask
    pos = g_listSources.GetHeadPosition();
    while(pos)
    {
        pdlgSrc = g_listSources.GetNext(pos);

        // bail if this seat is taken
        if( dwChanMsk & pdlgSrc->m_dwChannelMask & 0x7fffffff)
            return FALSE;

        dwChanMsk |= pdlgSrc->m_dwChannelMask;
    }

    // sort
    for(dwBit = SPEAKER_FRONT_LEFT; dwBit <= 0x80000000; dwBit <<= 1) //KSAUDIO_SPEAKER_TOP_REAR_RIGHT
    {
        pos = g_listSources.GetHeadPosition();
        while(pos)
        {
            pdlgSrc = g_listSources.GetNext(pos);
            if(pdlgSrc->m_dwChannelMask == dwBit)
                pdlgSrc->SetChannelNum(nChannel++);
        }

        // shift this again, and you get 0 => hangsville
        if(dwBit == 0x80000000)
            break;
    }

    // update the dest
    g_pdlgDest->Update();

    return TRUE;
}

// ----------------------------------------------------------------------------------------
void CChildView::OnMixerMerge() 
{
	g_pdlgDest->OnRemix();
}

// ----------------------------------------------------------------------------------------
void CChildView::OnUpdateMixerMerge(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( (0 != g_pdlgDest) && (0 != g_listSources.GetHeadPosition()) );
}

// ----------------------------------------------------------------------------------------
void CChildView::OnMixerPlay() 
{
    g_pdlgDest->OnPlay();
}

// ----------------------------------------------------------------------------------------
void CChildView::OnUpdateMixerPlay(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(g_pdlgDest && g_pdlgDest->m_fPlayable);
}

// ----------------------------------------------------------------------------------------
void CChildView::OnMixerStop()
{
    ASSERT( 0 );
}

// ----------------------------------------------------------------------------------------
void CChildView::OnUpdateMixerStop( CCmdUI* pCmdUI )
{}

// ----------------------------------------------------------------------------------------
void CChildView::OnSetfocusEditValidbits() 
{
	// TODO: Add your control notification handler code here
	ASSERT(0);
}

// ----------------------------------------------------------------------------------------
LRESULT CChildView::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
    switch(message)
    {
        case WM_COMMAND:
            {
                if(wParam == IDC_EDIT_VALIDBITS)
                {
                    OutputDebugString("WM_COMMAND\n");
                }
                break;
            }

        case WM_CTLCOLOR:
            {
                if ( HIWORD(lParam) == IDC_EDIT_VALIDBITS) 
                {
                    SetBkColor((HDC)wParam, RGB(128,128,128));
                    SetTextColor((HDC)wParam, RGB(255, 255, 255) );
                    return (LRESULT)GetStockObject(LTGRAY_BRUSH);
                }
                break;
            }
    }
	
	return CWnd ::WindowProc(message, wParam, lParam);
}

