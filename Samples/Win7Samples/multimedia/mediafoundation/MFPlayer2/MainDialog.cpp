//------------------------------------------------------------------------------
//
// File: MainDialog.h
// Implements the main dialog.
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//------------------------------------------------------------------------------

#include "MFPlayer.h"
#include "Player2.h"

// Constants 

const MFTIME    ONE_SECOND = 10000000; // One second in hns
const LONG      ONE_MSEC = 1000;       // One msec in hns 

const UINT_PTR  IDT_TIMER1 = 1;     // Timer ID
const UINT      TICK_FREQ = 250;    // Timer frequency in msec      

const LONG      MIN_VOL = 0;
const LONG      MAX_VOL = 100;


#include <initguid.h>

// CLSID of the sample video effect MFT. 
// (To use this CLSID, you must build the MFT_Grayscale sample and register the DLL.)

// {2F3DBC05-C011-4a8f-B264-E42E35C67BF4}
DEFINE_GUID(CLSID_GrayscaleMFT, 
0x2f3dbc05, 0xc011, 0x4a8f, 0xb2, 0x64, 0xe4, 0x2e, 0x35, 0xc6, 0x7b, 0xf4);


//------------------------------------------------------------------------------
// OpenUrlDialogInfo struct
// 
// Contains data passed to the "Open URL" dialog proc.
//------------------------------------------------------------------------------

struct OpenUrlDialogInfo
{
    WCHAR *pszURL;
    DWORD cch;
};


// Function declarations

INT_PTR CALLBACK OpenUrlDialogProc(HWND, UINT, WPARAM, LPARAM);

void    NotifyError(HWND hwnd, const WCHAR *sErrorMessage, HRESULT hrErr);
void    ToggleMenuItemCheck(UINT bMenuItemID, HMENU hmenu);
HRESULT AllocGetWindowText(HWND hwnd, WCHAR **pszText, DWORD *pcchLen);
void    EnableDialogControl(HWND hDlg, int nIDDlgItem, BOOL bEnable); 
BOOL    StatusBar_SetText(HWND hwnd, int iPart, const WCHAR* pszText);


inline BOOL IsMenuChecked(HMENU hmenu, UINT bMenuItemID)
{
    return GetMenuState(hmenu, bMenuItemID, MF_BYCOMMAND); 
}

inline float VolumeFromSlider(LONG pos)
{
    return (float)pos / MAX_VOL;
}

inline LONG SliderPosFromVolume(float fLevel)
{
    return (LONG)(fLevel * MAX_VOL);
}



//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------

MainDialog::MainDialog() : 
    m_nID(IDD_DIALOG1),
    m_hDlg(0),
    m_pPlayer(NULL),
    m_timerID(0),
    m_hSeekbar(NULL),
    m_hVolume(NULL),
    m_bSeeking(FALSE)
{
}


//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------

MainDialog::~MainDialog()
{
    if (m_pPlayer)
    {
        m_pPlayer->Shutdown();
    }

    SafeRelease(&m_pPlayer);
}


//-----------------------------------------------------------------------------
// ShowDialog()
// Displays the dialog
//
// Application instance
//
// Returns TRUE if successful or FALSE otherwise
//-----------------------------------------------------------------------------
BOOL MainDialog::ShowDialog(HINSTANCE hinst)
{
    // Show the dialog. Pass a pointer to ourselves as the LPARAM
    INT_PTR ret = DialogBoxParam(
        hinst, 
        MAKEINTRESOURCE(m_nID), 
        NULL, 
        DialogProc, 
        (LPARAM)this
        );

    if (ret == 0 || ret == -1)
    {
        MessageBox( NULL, L"Could not create dialog", L"Error", MB_OK | MB_ICONERROR );
        return FALSE;
    }

    return (IDOK == ret);
}



//-----------------------------------------------------------------------------
// OnInitDialog
//
// Handler for WM_INITDIALOG message.
//-----------------------------------------------------------------------------

HRESULT MainDialog::OnInitDialog()
{
    HRESULT hr = S_OK;

    // Create the player object.
    hr = MFPlayer2::CreateInstance(
        m_hDlg, 
        GetDlgItem(IDC_VIDEO),
        &m_pPlayer
        );

    if (SUCCEEDED(hr))
    {
        InitializeControls();

        // Set default UI state.
        UpdateUI(MFP_MEDIAPLAYER_STATE_EMPTY);
    }
    return hr;
}


//-----------------------------------------------------------------------------
// OnCommand
//
// Handler for WM_COMMAND messages.
//-----------------------------------------------------------------------------

INT_PTR MainDialog::OnCommand(HWND /*hControl*/, WORD idControl, WORD /*msg*/)
{
    switch (idControl)
    {
    case ID_FILE_OPENFILE:
        OnFileOpen();
        break;

    case ID_FILE_OPENURL:
        OnOpenURL();
        break;

    case ID_FILE_EXIT:
        PostQuitMessage(0);
        break;

    case IDC_MUTE:
        OnMute();
        break;

    case IDC_PLAY:
        OnPlayOrPause();
        break;

    case IDC_REWIND:
        OnRewind();
        break;

    case IDC_FASTFORWARD:
        OnFastForward();
        break;

    case ID_OPTIONS_VIDEOEFFECT: 
        ToggleMenuItemCheck(idControl, GetMenu(m_hDlg)); 
        break;

    }

    return 1;
}


//-----------------------------------------------------------------------------
// OnCommand
//
// Handler for WM_NOTIFY messages.
//-----------------------------------------------------------------------------

LRESULT MainDialog::OnNotify(NMHDR *pHdr)
{
    LRESULT result = 0; // ignored unless documented otherwise.

    switch (pHdr->idFrom)
    {
    case IDC_SEEKBAR:
        OnSeekbarNotify((NMSLIDER_INFO*)pHdr);
        break;

    case IDC_VOLUME:
        OnVolumeNotify((NMSLIDER_INFO*)pHdr);
        break;

    case IDC_MUTE:
        if (pHdr->code == NM_CUSTOMDRAW)
        {
            result = m_mute.Draw((NMCUSTOMDRAW*)pHdr);
        }
        break;

    case IDC_PLAY:
        if (pHdr->code == NM_CUSTOMDRAW)
        {
            result = m_play.Draw((NMCUSTOMDRAW*)pHdr);
        }
        break;
    }

    return result;   
}

//-----------------------------------------------------------------------------
// OnReceiveMsg
//
// Handler for any other window messages.
//-----------------------------------------------------------------------------

INT_PTR MainDialog::OnReceiveMsg(UINT msg, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;

    switch (msg)
    {
    case WM_TIMER:          // Timer message
        OnTimer();
        break;

    case WM_HSCROLL:        // Trackbar scroll
        OnScroll(LOWORD(wParam), HIWORD(wParam), (HWND)lParam);
        break;

    case WM_THEMECHANGED:   // User changed the current theme.
        m_mute.ResetTheme();
        m_play.ResetTheme();
        break;

    case WM_PAINT:

        BeginPaint(m_hDlg, &ps);

        if (m_pPlayer)
        {
            m_pPlayer->UpdateVideo();
        }

        EndPaint(m_hDlg, &ps);
        break;

    case WM_CLOSE:
        PostQuitMessage(0);
        break;

    // Private window messages from the MFPlayer class.

    case WM_APP_NOTIFY:
        OnPlayerNotify((MFP_MEDIAPLAYER_STATE)wParam);
        break;
   
    case WM_APP_ERROR:
        NotifyError(m_hDlg, L"Playback Error", (HRESULT)wParam);
        UpdateUI( );       
        break;

    case WM_AUDIO_EVENT:
        {
            // The audio level been changed.

            float fVolume = *((float *)(&wParam));
    
            m_mute.SetButtonImage((UINT)-1, (lParam ? 1 : 0));
            InvalidateRect(m_mute.Window(), NULL, FALSE);

            Slider_SetPosition(m_hVolume, SliderPosFromVolume(fVolume));
        }
        break;

    }
    return FALSE;
}

//-----------------------------------------------------------------------------
// InitializeControls
//
// Initializes the UI controls.
//-----------------------------------------------------------------------------

void MainDialog::InitializeControls()
{
    m_mute.SetWindow(GetDlgItem(IDC_MUTE));
    m_play.SetWindow(GetDlgItem(IDC_PLAY));

    // Create a brush for the seekbar background. 
    // Note: Don't use GetSysColorBrush because the slider control destroys the brush.
    HBRUSH hBrush = CreateSolidBrush(RGB(0,0,0));
    
    m_hSeekbar = GetDlgItem(IDC_SEEKBAR);

    Slider_SetBackground(m_hSeekbar, hBrush);
    Slider_SetThumbBitmap(m_hSeekbar, IDB_SLIDER);
    EnableDialogControl(m_hDlg, IDC_SEEKBAR, FALSE);

    hBrush = CreateSolidBrush(RGB(12,3,127));
    
    m_hVolume = GetDlgItem(IDC_VOLUME);

    Slider_SetBackground(m_hVolume, hBrush);
    Slider_SetThumbBitmap(m_hVolume, IDB_VOLUME);
    Slider_SetRange(m_hVolume, MIN_VOL, MAX_VOL);
    Slider_SetPosition(m_hVolume, MAX_VOL);
    EnableDialogControl(m_hDlg, IDC_VOLUME, TRUE);



    m_mute.LoadBitmap(IDB_MUTE, 2);
    m_mute.SetButtonImage((UINT)-1, 0);

    m_play.LoadBitmap(IDB_PLAY, 2);
    m_play.SetButtonImage((UINT)-1, 0);

    // Set the range for the "zoom" trackbar.
    HWND hZoom = GetDlgItem(IDC_VIDEO_ZOOM);

    SendMessage(hZoom, TBM_SETRANGEMIN, TRUE, 100);
    SendMessage(hZoom, TBM_SETRANGEMAX, TRUE, 500);
    SendMessage(hZoom, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)100);

    InitStatusBar();

    ClearMetadata();
}


//-----------------------------------------------------------------------------
// OnPlayerNotify
// 
// Called when the player object changes state.
//-----------------------------------------------------------------------------

void MainDialog::OnPlayerNotify(MFP_MEDIAPLAYER_STATE state)
{
    UpdateSeekBar();
    UpdateUI(state);

    if (state == MFP_MEDIAPLAYER_STATE_PLAYING)
    {
        UpdateMetadata();
    }
}


//-----------------------------------------------------------------------------
// OnFileOpen
//
// Open a new file for playback.
//-----------------------------------------------------------------------------

void MainDialog::OnFileOpen()
{
    const WCHAR *lpstrFilter = 
        L"Media Files\0*.aac;*.asf;*.avi;*.m4a;*.mp3;*.mp4;*.wav;*.wma;*.wmv;*.3gp;*.3g2\0"
        L"All files\0*.*\0";

    HRESULT hr = S_OK;

    OPENFILENAME ofn;
    ZeroMemory(&ofn, sizeof(ofn));

    WCHAR szFileName[MAX_PATH];
    szFileName[0] = L'\0';

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = m_hDlg;
    ofn.hInstance = GetInstance();
    ofn.lpstrFilter = lpstrFilter;
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST;

    if (GetOpenFileName(&ofn))
    {
        ApplyOptions();

        // Try to open the file.
        hr = m_pPlayer->OpenURL(szFileName);

        // Update the state of the UI. (Regardless of success/failure code)
        UpdateUI();

        // Invalidate the video window, in case there is an old video 
        // frame from the previous file and there is no video now. (eg, the
        // new file is audio only, or we failed to open this file.)
        InvalidateRect( GetDlgItem(IDC_VIDEO) , NULL, FALSE);

        if (FAILED(hr))
        {
            NotifyError(m_hDlg, L"Cannot open this file.", hr);
        }
    }
    else
    {
        // GetOpenFileName can return FALSE because the user cancelled,
        // or because it failed. Check for errors.

        DWORD err = CommDlgExtendedError();
        if (err != 0)
        {
            NotifyError(m_hDlg, L"GetOpenFileName failed.", E_FAIL);
        }
    }
}


//-----------------------------------------------------------------------------
//  OnOpenURL
//
//  Opens a media file from a URL.
//-----------------------------------------------------------------------------

void MainDialog::OnOpenURL()
{
    HRESULT hr = S_OK;
    INT_PTR result = 0;

    // Pass in an OpenUrlDialogInfo structure to the dialog. The dialog 
    // fills in this structure with the URL. The dialog proc allocates
    // the memory for the string. 

    OpenUrlDialogInfo url;
    ZeroMemory(&url, sizeof(&url));

    // Show the Open URL dialog.
    result = DialogBoxParam(GetInstance(), MAKEINTRESOURCE(IDD_OPENURL), m_hDlg, 
        OpenUrlDialogProc, (LPARAM)&url);

    if (result == IDOK)
    {
        ApplyOptions();

        // Open the file with the playback object.
        hr = m_pPlayer->OpenURL(url.pszURL);

        // Update the state of the UI. (Regardless of success/failure code)
        UpdateUI();

        SetStatusText(L"Opening...");

        // Invalidate the video window, in case there is an old video 
        // frame from the previous file and there is no video now. (eg, the
        // new file is audio only, or we failed to open this file.)
        InvalidateRect(GetDlgItem(IDC_VIDEO), NULL, FALSE);

        if (FAILED(hr))
        {
            NotifyError(m_hDlg, L"Cannot open this URL.", hr);
        }
    }

    // The app must release the string for the URL.
    CoTaskMemFree(url.pszURL);
}



//-----------------------------------------------------------------------------
//  OnScroll
//
//  Called when user selects, drags, or releases a trackbar control.
//-----------------------------------------------------------------------------

void MainDialog::OnScroll(WORD request, WORD /*position*/, HWND hControl)
{
    // We ignore the following trackbar requests:
    switch (request)
    {
    case SB_ENDSCROLL: 
    case SB_LEFT: 
    case SB_RIGHT:
    case SB_THUMBPOSITION: 
        return;
    }

    if (hControl == GetDlgItem(IDC_VIDEO_ZOOM))
    {
        if (m_pPlayer)
        {
            LONG pos = (LONG)SendMessage(hControl, TBM_GETPOS, 0, 0);

            // Scale the video.
            float fZoom = (float)pos / 100;

            m_pPlayer->SetZoom(fZoom);
        }
    }
}


//-----------------------------------------------------------------------------
// OnSeekbarNotify
//
// Called when user selects, drags, or releases a seek bar.
//-----------------------------------------------------------------------------

void MainDialog::OnSeekbarNotify(const NMSLIDER_INFO *pInfo)
{
    if (m_pPlayer)
    {
        if (pInfo->hdr.code == SLIDER_NOTIFY_SELECT)
        {
            m_bSeeking = TRUE;
        }

        // When dragging or selecting, seek to the position.
        if ((pInfo->hdr.code == SLIDER_NOTIFY_SELECT) ||
            (pInfo->hdr.code == SLIDER_NOTIFY_DRAG))
        {
            LONGLONG pos = ONE_MSEC * (LONGLONG)pInfo->position;

            (void)m_pPlayer->SetPosition(pos);

            SetStatusTime(pos);
        }

        if (pInfo->hdr.code == SLIDER_NOTIFY_RELEASE)
        {
            m_bSeeking = FALSE;
        }
    }
}

//-----------------------------------------------------------------------------
// OnPlayOrPause
//
// Called when the user clicks the play/pause button.
//-----------------------------------------------------------------------------

void MainDialog::OnPlayOrPause()
{
    // This button toggles between play and pause.

    MFP_MEDIAPLAYER_STATE state;

    m_pPlayer->GetState(&state);

    if (state == MFP_MEDIAPLAYER_STATE_PLAYING)
    {
        m_pPlayer->Pause();
    }
    else
    {
        m_pPlayer->Play();
    }
}


//-----------------------------------------------------------------------------
// OnFastForward
//
// Called when the user clicks the fast-forward button.
//-----------------------------------------------------------------------------

void MainDialog::OnFastForward()
{
    if (m_pPlayer)
    {
        MFP_MEDIAPLAYER_STATE state;

        m_pPlayer->GetState(&state);

        if (state == MFP_MEDIAPLAYER_STATE_PAUSED)
        {
            m_pPlayer->FrameStep();
        }
        else
        {
            m_pPlayer->FastForward();
        }
    }
}


//-----------------------------------------------------------------------------
// OnRewind
//
// Called when the user clicks the rewind button.
//-----------------------------------------------------------------------------

void MainDialog::OnRewind()
{
    if (m_pPlayer)
    {
        m_pPlayer->Rewind();
    }
}


//-----------------------------------------------------------------------------
// OnVolumeNotify
//
// Called when the user drags the volume slider.
//-----------------------------------------------------------------------------

void MainDialog::OnVolumeNotify(const NMSLIDER_INFO* pInfo)
{
    if (m_pPlayer)
    {
        m_pPlayer->SetVolume( VolumeFromSlider(pInfo->position) );
    }
}



//-----------------------------------------------------------------------------
// OnVolumeChanged
//
// Called when the volume changes because of actions outside of this
// application. (For example, when the user changes the volume in the 
// Volume control panel.
//-----------------------------------------------------------------------------

void MainDialog::OnVolumeChanged()
{
    HRESULT hr = S_OK;

    float fLevel = 0;
    BOOL bMute = FALSE;

    if (m_pPlayer)
    {
        // Get the new volume level and update the slider position.
        hr = m_pPlayer->GetVolume(&fLevel);

        if (SUCCEEDED(hr))
        {
            Slider_SetPosition(m_hVolume, SliderPosFromVolume(fLevel));
        }

        // Get the new mute state and update the image on the mute button.
        hr = m_pPlayer->GetMute(&bMute);
        if (SUCCEEDED(hr))
        {
            m_mute.SetButtonImage((UINT)-1, (bMute ? 1 : 0));
            InvalidateRect(m_mute.Window(), NULL, FALSE);
        }
    }
}


//-----------------------------------------------------------------------------
// OnMute
//
// Called when the user clicks the mute/unmute button.
//-----------------------------------------------------------------------------

void MainDialog::OnMute()
{
    BOOL bMute = FALSE;

    if (m_pPlayer)
    {
        HRESULT hr = m_pPlayer->GetMute(&bMute);

        if (SUCCEEDED(hr))
        {
            bMute = !bMute; // Flip the mute state.
            hr = m_pPlayer->SetMute(bMute);
        }

        if (SUCCEEDED(hr))
        {
            m_mute.SetButtonImage((UINT)-1, (bMute ? 1 : 0));
        }
    }
}




//-----------------------------------------------------------------------------
// OnTimer
//
// Called when the timer elapses.
//-----------------------------------------------------------------------------

void MainDialog::OnTimer()
{
    MFTIME timeNow;

    if (m_pPlayer && !m_bSeeking)
    {
        if (SUCCEEDED(m_pPlayer->GetCurrentPosition(&timeNow)))
        {
            Slider_SetPosition(m_hSeekbar, (LONG)(timeNow / ONE_MSEC));

            SetStatusTime(timeNow);
        }
    }
}


//-----------------------------------------------------------------------------
// ApplyOptions
//
// Applies the user's playback options, before opening a new media file.
//-----------------------------------------------------------------------------

void MainDialog::ApplyOptions()
{
    HRESULT hr = S_OK;

    HMENU hMenu = GetMenu(m_hDlg);

    BOOL bVideoFX = IsMenuChecked(hMenu, ID_OPTIONS_VIDEOEFFECT);

    if (bVideoFX)
    {
        IMFTransform *pMFT = NULL;

        hr = CoCreateInstance(
            CLSID_GrayscaleMFT, 
            NULL, 
            CLSCTX_INPROC_SERVER, 
            IID_PPV_ARGS(&pMFT)
            );

        if (FAILED(hr))
        {
            NotifyError(m_hDlg, L"Cannot create grayscale transform. Did you register the DLL?", hr);
        }
        else
        {
            hr = m_pPlayer->SetEffect(pMFT);
            pMFT->Release();
        }
    }
    else
    {
        hr = m_pPlayer->SetEffect(NULL);
    }

}


//-----------------------------------------------------------------------------
// UpdateUI
// 
// Update the dialog based on the current playback state.
//-----------------------------------------------------------------------------

void MainDialog::UpdateUI()
{
    MFP_MEDIAPLAYER_STATE state = MFP_MEDIAPLAYER_STATE_EMPTY;

    if (m_pPlayer)
    {
        m_pPlayer->GetState(&state);
    }

    UpdateUI(state);
}



//-----------------------------------------------------------------------------
// UpdateUI
//
// Update the dialog based on the current playback state.
//-----------------------------------------------------------------------------

void MainDialog::UpdateUI(MFP_MEDIAPLAYER_STATE state)
{
    BOOL bFileOpen = TRUE;
    BOOL bHasVideo = FALSE;
    BOOL bHasAudio = FALSE;
    BOOL bEnablePlay = FALSE;
    BOOL bPlay = TRUE;  
    
    // If bPlay is TRUE, the Play/Pause button shows the "play" image.
    // If bEnablePlay is TRUE, the Play/Pause button is enabled.
    
    BOOL bCanSeek = FALSE;
    BOOL bEnableSeek = FALSE;

    BOOL bCanFF = FALSE;
    BOOL bCanRewind = FALSE;
    BOOL bEnableTrickMode = FALSE;

    if (m_pPlayer)
    {
        m_pPlayer->HasVideo(&bHasVideo);
        m_pPlayer->CanSeek(&bCanSeek);
        m_pPlayer->CanFastForward(&bCanFF);
        m_pPlayer->CanRewind(&bCanRewind);

        bHasAudio = m_pPlayer->IsAudioEnabled();
    }
    switch (state)
    {
    case MFP_MEDIAPLAYER_STATE_EMPTY:
    case MFP_MEDIAPLAYER_STATE_SHUTDOWN:
        bFileOpen = FALSE;
        SetStatusText(L"Closed");
        break;

    case MFP_MEDIAPLAYER_STATE_PLAYING:
        bEnablePlay = TRUE;
        bPlay = FALSE;
        bEnableSeek = bCanSeek;
        bEnableTrickMode = TRUE;
        SetStatusText(L"Playing");
        UpdateMetadata();
        break;

    case MFP_MEDIAPLAYER_STATE_PAUSED:
        bEnablePlay = TRUE;
        bEnableSeek = bCanSeek;
        bEnableTrickMode = bCanFF = TRUE;   // Fast forward button is "frame step" while paused.
        SetStatusText(L"Paused");
        break;

    case MFP_MEDIAPLAYER_STATE_STOPPED:
        bEnablePlay = TRUE; 
        SetStatusText(L"Stopped");
        break;
    }


    EnableDialogControl(m_hDlg, IDC_VIDEO_ZOOM, bHasVideo);

    EnableDialogControl(m_hDlg, IDC_PLAY, bEnablePlay);
    EnableDialogControl(m_hDlg, IDC_SEEKBAR, bEnablePlay);

    EnableDialogControl(m_hDlg, IDC_FASTFORWARD, (bEnableTrickMode && bCanFF));
    EnableDialogControl(m_hDlg, IDC_REWIND, (bEnableTrickMode && bCanRewind));
    
    m_play.SetButtonImage((UINT)-1, (bPlay ? 0 : 1));
    InvalidateRect(m_play.Window(), NULL, FALSE);

    EnableDialogControl(m_hDlg, IDC_MUTE, bHasAudio);
    EnableDialogControl(m_hDlg, IDC_VOLUME, bHasAudio);
}


//-----------------------------------------------------------------------------
// UpdateSeekBar
//
// Update the state of the seek bar.
//-----------------------------------------------------------------------------

void MainDialog::UpdateSeekBar()
{
    HRESULT hr = S_OK;
    BOOL bCanSeek = FALSE;
    MFTIME duration = 0;

    // Stop the old timer, if any.
    StopTimer();

    if (m_pPlayer)
    {
        hr = m_pPlayer->CanSeek(&bCanSeek);
        if (FAILED(hr)) { goto done; }

        // If the player can seek, set the seekbar range and start the time.
        // Otherwise, disable the seekbar.
        if (bCanSeek)
        {
            hr = m_pPlayer->GetDuration(&duration);
            if (FAILED(hr)) { goto done; }

            Slider_SetRange(m_hSeekbar, 0, (LONG)(duration / ONE_MSEC));
            EnableDialogControl(m_hDlg, IDC_SEEKBAR, TRUE);

            // Start the timer
            m_timerID = SetTimer(m_hDlg, IDT_TIMER1, TICK_FREQ, NULL);
        }
        else
        {
            EnableDialogControl(m_hDlg, IDC_SEEKBAR, FALSE);
        }   
    }

done:
    if (FAILED(hr))
    {
        EnableDialogControl(m_hDlg, IDC_SEEKBAR, FALSE);
    }
}


//-----------------------------------------------------------------------------
// UpdateMetadata
//
// Update the metadata text.
//-----------------------------------------------------------------------------

void MainDialog::UpdateMetadata()
{
    HRESULT hr = S_OK;

    IPropertyStore *pProp = NULL;
    
    PROPVARIANT var;
    PropVariantInit(&var);

    ClearMetadata();

    if (m_pPlayer)
    {
        hr = m_pPlayer->GetMetadata(&pProp);
        if (FAILED(hr)) { goto done; }

        hr = pProp->GetValue(PKEY_Title, &var);
        if (SUCCEEDED(hr) && var.vt == VT_LPWSTR)
        {
            SetWindowText( GetDlgItem(IDC_MEDIA_TITLE), var.pwszVal );
        }
        PropVariantClear(&var);

        hr = pProp->GetValue(PKEY_Author, &var);
        if (SUCCEEDED(hr) && var.vt == VT_LPWSTR)
        {
            SetWindowText( GetDlgItem(IDC_MEDIA_AUTHOR), var.pwszVal );
        }
        PropVariantClear(&var);
    }

done:
    PropVariantClear(&var);
    SafeRelease(&pProp);
}


//-----------------------------------------------------------------------------
// ClearMetadata
//
// Clear the metadata text.
//-----------------------------------------------------------------------------

void MainDialog::ClearMetadata()
{
    SetWindowText( GetDlgItem(IDC_MEDIA_TITLE), L"" );
    SetWindowText( GetDlgItem(IDC_MEDIA_AUTHOR), L"" );
}



//-----------------------------------------------------------------------------
// StopTimer
//
// Stop the timer.
//-----------------------------------------------------------------------------

void MainDialog::StopTimer()
{
    if (m_timerID != 0)
    {
        KillTimer(m_hDlg, m_timerID);
        m_timerID = 0;
    }
}


//-----------------------------------------------------------------------------
// InitStatusBar
// 
// Initialize the dialog status bar.
//-----------------------------------------------------------------------------

void MainDialog::InitStatusBar()
{
    // NOTE: A status bar sets its own width and height.

    HWND hStatus = CreateWindowEx(
        0, 
        STATUSCLASSNAME, 
        NULL, 
        WS_CHILD | WS_VISIBLE,
        0, 0, 0, 0, 
        m_hDlg, 
        (HMENU)(INT_PTR)IDC_STATUS_BAR,
        GetInstance(), 
        NULL);


    SIZE szTimeCode;
    RECT rcStatusBar;

    HDC hdc = GetDC(hStatus);
        
    const WCHAR tmp[] = L"0000:00:00";
    GetTextExtentPoint32(hdc, tmp, ARRAYSIZE(tmp), &szTimeCode);

    ReleaseDC(GetDlgItem(IDC_STATUS_BAR), hdc);

    GetClientRect(hStatus, &rcStatusBar);

    int iParts[] = { rcStatusBar.right - szTimeCode.cx, -1};

    SendMessage(hStatus, SB_SETPARTS, (WPARAM)2, (LPARAM)iParts);
}


//-----------------------------------------------------------------------------
// SetStatusTime
//
// Display the specified presentation time in the status bar.
//-----------------------------------------------------------------------------

void MainDialog::SetStatusTime(const MFTIME& time)
{
    HRESULT hr = S_OK;
    WCHAR szTimeStamp[32];
    MFTIME hour = 0, min = 0, sec = 0;

    sec = (time / ONE_SECOND);
    min = (time / (ONE_SECOND * 60));
    hour = (time / (ONE_SECOND * 360));

    hr = StringCchPrintf(
        szTimeStamp, 
        ARRAYSIZE(szTimeStamp), 
        L"%d:%02d:%02d", 
        (DWORD)hour, (DWORD)(min % 60), (DWORD)(sec % 60)
        );

    if (SUCCEEDED(hr))
    {    
        HWND hStatus = GetDlgItem(IDC_STATUS_BAR);

        StatusBar_SetText(hStatus, 1, szTimeStamp);
    }
}


//-----------------------------------------------------------------------------
// SetStatusText
//
// Set text in the status bar.
//-----------------------------------------------------------------------------

void MainDialog::SetStatusText(const WCHAR *pszStatus)
{
    HWND hStatus = GetDlgItem(IDC_STATUS_BAR);

    StatusBar_SetText(hStatus, 0, pszStatus);
}




//-----------------------------------------------------------------------------
// Name: DialogProc()
// Desc: DialogProc for the dialog. This is a static class method.
//
// lParam: Pointer to the CBaseDialog object. 
//
// The CBaseDialog class specifies lParam when it calls DialogBoxParam. We store the 
// pointer as user data in the window. 
//
// (Note: The DirectShow CBasePropertyPage class uses the same technique.)
//-----------------------------------------------------------------------------
INT_PTR CALLBACK MainDialog::DialogProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    MainDialog *pDlg = 0;  // Pointer to the dialog class that manages the dialog 

    LRESULT lresult = 0;

    if (msg == WM_INITDIALOG)
    {
        // Get the pointer to the dialog object and store it in 
        // the window's user data

        SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)lParam);

        pDlg = (MainDialog*)lParam;
        if (pDlg)
        {
            pDlg->m_hDlg = hDlg;
            HRESULT hr = pDlg->OnInitDialog();
            if (FAILED(hr))
            {
                pDlg->EndDialog(0);
            }
        }
        return FALSE;
    }

    // Get the dialog object from the window's user data
    pDlg = (MainDialog*)(DWORD_PTR) GetWindowLongPtr(hDlg, DWLP_USER);

    if (pDlg != NULL)
    {
        switch (msg)
        {
        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
            case IDOK:
            case IDCANCEL:
                pDlg->EndDialog(LOWORD(wParam));
                return TRUE;

            default:
                return pDlg->OnCommand((HWND)lParam, LOWORD(wParam), HIWORD(wParam));
            }
            break;

        case WM_NOTIFY:
            lresult = pDlg->OnNotify((NMHDR*)lParam);

            // The LRESULT from WM_NOTIFY can be meaningful. Store the result in DWLP_MSGRESULT.
            SetWindowLongPtr(hDlg, DWLP_MSGRESULT, (LONG_PTR)lresult);
            return TRUE;

        default:
            return pDlg->OnReceiveMsg(msg, wParam, lParam);
        }
    }
    else
    {
        return FALSE;
    }
}




//-----------------------------------------------------------------------------
//  NotifyError
//
//  Show a message box with an error message.
//-----------------------------------------------------------------------------

void NotifyError(
    HWND hwnd, 
    const WCHAR *sErrorMessage,    // Error message
    HRESULT hrErr                  // Status code
    )
{
    const size_t MESSAGE_LEN = 512;
    WCHAR message[MESSAGE_LEN];

    HRESULT hr = StringCchPrintf (message, MESSAGE_LEN, L"%s (HRESULT = 0x%X)", sErrorMessage, hrErr);
    if (SUCCEEDED(hr))
    {
        MessageBox(hwnd, message, NULL, MB_OK | MB_ICONERROR);
    }
}




//-----------------------------------------------------------------------------
//  OpenUrlDialogProc
//
//  Dialog procedure for the "Open URL" window.
//-----------------------------------------------------------------------------

INT_PTR CALLBACK OpenUrlDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static OpenUrlDialogInfo *pUrl = NULL;

    BOOL result = FALSE;

    switch (message)
    {
    case WM_INITDIALOG:
        // The app sends a pointer to an OpenUrlDialogInfo structure as the lParam. 
        // We use this structure to store the URL.
        pUrl = (OpenUrlDialogInfo*)lParam;
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:
            if (pUrl)
            {
                // Get the URL from the edit box in the dialog.
                // This function allocates memory. The app must call CoTaskMemAlloc.
                HRESULT hr = AllocGetWindowText(GetDlgItem(hDlg, IDC_EDIT_URL), &pUrl->pszURL, &pUrl->cch);
                if (SUCCEEDED(hr))
                {
                    result = TRUE;
                }
            }
            EndDialog(hDlg, result ? IDOK : IDABORT);
            break;

        case IDCANCEL:
            EndDialog(hDlg, LOWORD(IDCANCEL));
            break;
        }
        return (INT_PTR)FALSE;
    }
    return (INT_PTR)FALSE;
}



//-----------------------------------------------------------------------------
// ToggleMenuItemCheck
//
// Toggle a menu item's checked state.
//-----------------------------------------------------------------------------

void ToggleMenuItemCheck(UINT bMenuItemID, HMENU hmenu) 
{ 
    BOOL bChecked = IsMenuChecked(hmenu, bMenuItemID); 

    BYTE fNewState = MF_CHECKED;

    if (bChecked & MF_CHECKED) 
    { 
        fNewState = MF_UNCHECKED;
    }
    CheckMenuItem(hmenu, (UINT) bMenuItemID, 
        MF_BYCOMMAND | fNewState); 
    
}



//--------------------------------------------------------------------------------------
// Name: AllocGetWindowText
// Description: Helper function to get text from a window.
//
// This function allocates a buffer and returns it in pszText. The caller must
// call CoTaskMemFree on the buffer.
//
// hwnd:     Handle to the window
// pszText:  Receives a pointer to the string.
// pcchLen:  Receives the length of the string, in characters, not including
//           the terminating NULL character. This parameter can be NULL.
//--------------------------------------------------------------------------------------

HRESULT AllocGetWindowText(HWND hwnd, WCHAR **pszText, DWORD *pcchLen)
{
    if (pszText == NULL)
    {
        return E_POINTER;
    }

    *pszText = NULL;  

    int cch = GetWindowTextLength(hwnd);  
    if (cch < 0) 
    {
        return E_UNEXPECTED; // This function should not return a negative value.
    }

    WCHAR *szTmp = (WCHAR*)CoTaskMemAlloc(sizeof(WCHAR) * (cch + 1)); // Include room for terminating NULL character

    if (!szTmp)
    {
        return E_OUTOFMEMORY;
    }

    if (cch == 0)
    {
        szTmp[0] = L'\0';  // No text.
    }
    else
    {
        int res = GetWindowText(hwnd, szTmp, (cch + 1));  // Size includes NULL character

        // GetWindowText returns 0 if (a) there is no text or (b) it failed.
        // We checked for (a) already, so 0 means failure here.

        if (res == 0)
        {
            CoTaskMemFree(szTmp);
            return __HRESULT_FROM_WIN32(GetLastError());
        }
    }

    // If we got here, szTmp is valid, so return it to the caller.
    *pszText = szTmp;
    if (pcchLen)
    {
        *pcchLen = static_cast<DWORD>(cch);  // Return the length NOT including the '\0' 
    }
    return S_OK;
}



void EnableDialogControl(HWND hDlg, int nIDDlgItem, BOOL bEnable) 
{ 
    HWND hwnd = GetDlgItem(hDlg, nIDDlgItem);

    if (!bEnable &&  hwnd == GetFocus())
    {
        // If we're being disabled and this control has focus,
        // set the focus to the next control.

        ::SendMessage(GetParent(hwnd), WM_NEXTDLGCTL, 0, FALSE);
    }

    EnableWindow(hwnd, bEnable); 
}


BOOL StatusBar_SetText(HWND hwnd, int iPart, const WCHAR* pszText)
{
    return (BOOL)SendMessage(hwnd, SB_SETTEXT, (WPARAM)iPart, (LPARAM)pszText);
}
