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

#pragma once

class MainDialog
{
public:
    MainDialog();
    ~MainDialog();

    virtual BOOL ShowDialog(HINSTANCE hinst);

    // Message handlers

    // WM_INITDIALOG
    HRESULT OnInitDialog(); 
    // WM_COMMAND
    INT_PTR OnCommand(HWND hControl, WORD idControl, WORD msg);
    // WM_NOTIFY
    LRESULT OnNotify(NMHDR *pHdr);
    // Other messages
    INT_PTR OnReceiveMsg(UINT msg, WPARAM wParam, LPARAM lParam);

private:

    // Dialog proc for the dialog we manage
    static INT_PTR CALLBACK DialogProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
    
    // Return one of our dialog controls
    HWND GetDlgItem(int nID) { return ::GetDlgItem(m_hDlg, nID); }

    void EndDialog(INT_PTR cmd)
    {
        if (m_hDlg)
        {
            ::EndDialog(m_hDlg, cmd);
        }
    }


    void    InitializeControls();
    void    InitStatusBar();
    void    UpdateUI();
    void    UpdateUI(MFP_MEDIAPLAYER_STATE state);
    void    UpdateSeekBar();

    void    UpdateMetadata();
    void    ClearMetadata();
    
    void    ApplyOptions();
    void    StopTimer();
    void    SetStatusTime(const MFTIME& time);
    void    SetStatusText(const WCHAR *szStatus);

    // Commands
    void    OnFileOpen();
    void    OnOpenURL();
    void    OnScroll(WORD request, WORD position, HWND hControl);
    void    OnSeekbarNotify(const NMSLIDER_INFO *pInfo);
    void    OnVolumeNotify(const NMSLIDER_INFO *pInfo);
    void    OnTimer();
    void    OnMute();
    void    OnPlayOrPause();
    void    OnFastForward();
    void    OnRewind();

    // Player state notifications
    void    OnPlayerNotify(MFP_MEDIAPLAYER_STATE state);

    // Player events
    void    OnVolumeChanged();

private:

    HWND            m_hDlg;     // this dialog window
    int             m_nID;      // Resource ID of the dialog window 

    MFPlayer2       *m_pPlayer;

    // UI Controls
    ThemedButton    m_mute;
    ThemedButton    m_play;

    HWND            m_hSeekbar;
    HWND            m_hVolume;

    UINT_PTR        m_timerID; 

    BOOL            m_bSeeking;
};


