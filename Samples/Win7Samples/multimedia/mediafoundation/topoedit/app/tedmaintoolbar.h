// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

#include "tedobj.h"
#include "trackbarcontrol.h"

class CTedMainToolbar 
    : public CWindowImpl<CTedMainToolbar>
{
public:
    CTedMainToolbar();
    ~CTedMainToolbar();
    
    HRESULT Init(HWND parentWnd, _U_MENUorID id);
    
    CTrackbarControl* GetSeekBar();
    CTrackbarControl* GetRateBar();

    void ShowRateBar(int nCmdShow);
    void EnableButton(int nID, BOOL fEnable);
    void EnableButtonByCommand(UINT nID, BOOL fEnable);
    void SetTrackbarScrollCallback(HANDLESCROLLPROC scrollCallback);
    void SetTrackbarRange(int minValue, int maxValue);
    void SetTimeLabel(const CAtlStringW& strLabel);
    void UpdateTimeDisplay(MFTIME time, MFTIME duration);
    void UpdateRateDisplay(float flRate);
    
    void MarkResolved(bool fResolved = true);
    
    static const int PLAY_BUTTON;
    static const int STOP_BUTTON;
    static const int PAUSE_BUTTON;

    DECLARE_WND_SUPERCLASS(NULL, TOOLBARCLASSNAME)
    
protected:
    LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    virtual LRESULT OnVScroll(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    virtual LRESULT OnHScroll(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);  
    
    BEGIN_MSG_MAP(CTedMainToolbar)
        MESSAGE_HANDLER(WM_CREATE, OnCreate)
        MESSAGE_HANDLER(WM_VSCROLL, OnVScroll)
        MESSAGE_HANDLER(WM_HSCROLL, OnHScroll)
    END_MSG_MAP()
    
private:
    TBBUTTON m_buttons[3];
    CSeekerTrackbarControl m_trackBar;
    CStatic m_timeLabel;
    UINT_PTR nTimerID;
    DWORD dwTimerInterval;
    HFONT m_hStaticFont;

    CStatic m_rateLabel;
    CTrackbarControl m_rateBar;
    
    CStatic m_resolvedLabel;
    
    static const int m_iButtonCount = 3;
    static const int m_iMarginSize = 5;
    static const int m_iToolbarButtonWidth = 75;
    static const int m_iTimeLabelWidth = 75;
    static const int m_iTrackbarWidth = 100;
    static const int m_iRateLabelWidth = 65;
    static const int m_iRateBarWidth = 50;
    static const int m_iResolvedLabelWidth = 175;
    static const int m_iSecondsInMinute = 60;
};