// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "stdafx.h"

#include "tedmaintoolbar.h"
#include "resource.h"

#include <assert.h>

const int CTedMainToolbar::PLAY_BUTTON = 0;
const int CTedMainToolbar::STOP_BUTTON = 1;
const int CTedMainToolbar::PAUSE_BUTTON = 2;

#define NANO_TO_SECS(x) x / 10000000LL;

CTedMainToolbar::CTedMainToolbar() 
    : m_hStaticFont(NULL)
{

}

CTedMainToolbar::~CTedMainToolbar()
{
    if(m_hStaticFont) 
    {
        DeleteObject(m_hStaticFont);
    }
}

HRESULT CTedMainToolbar::Init(HWND parentWnd, _U_MENUorID id) 
{
    HRESULT hr = S_OK;
    if(Create(parentWnd, NULL, LoadAtlString(IDS_TOOLBAR), WS_CHILD | WS_VISIBLE, 0, id, NULL) == NULL) 
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

    return hr;
}

CTrackbarControl* CTedMainToolbar::GetSeekBar() 
{
    return &m_trackBar;
}

CTrackbarControl* CTedMainToolbar::GetRateBar() 
{
    return &m_rateBar;
}

void CTedMainToolbar::ShowRateBar(int nCmdShow) 
{
    m_rateLabel.ShowWindow(nCmdShow);
    m_rateBar.ShowWindow(nCmdShow);
}

void CTedMainToolbar::EnableButton(int nID, BOOL fEnable) 
{
    assert(nID < m_iButtonCount);
    if(nID >= m_iButtonCount) return;
    
    SendMessage(TB_ENABLEBUTTON, m_buttons[nID].idCommand, MAKELONG(fEnable, 0));
}

void CTedMainToolbar::EnableButtonByCommand(UINT nID, BOOL fEnable) 
{
    SendMessage(TB_ENABLEBUTTON, nID, MAKELONG(fEnable, 0));
}

void CTedMainToolbar::SetTrackbarScrollCallback(HANDLESCROLLPROC scrollCallback) 
{
    m_trackBar.SetScrollCallback(scrollCallback);
}

void CTedMainToolbar::SetTrackbarRange(int minValue, int maxValue) 
{
    m_trackBar.SetRange(minValue, maxValue);
}

void CTedMainToolbar::SetTimeLabel(const CAtlStringW& strLabel) 
{
    m_timeLabel.SetWindowText(strLabel);
}

void CTedMainToolbar::UpdateTimeDisplay(MFTIME time, MFTIME duration)
{
    DWORD dwMax = m_trackBar.GetMaxRange();
    
    LONG lPos = 0;
    if ( duration > 0 )
    {
        lPos = LONG( (LONGLONG) dwMax * time / duration );
    }

    if(!m_trackBar.IsTracking()) m_trackBar.SetPos(lPos);

    DWORD dwTotalSecs = (DWORD) NANO_TO_SECS(duration);
    DWORD dwTotalMins = dwTotalSecs / m_iSecondsInMinute;
    dwTotalSecs = dwTotalSecs % m_iSecondsInMinute;

    DWORD dwCurrSecs = (DWORD) NANO_TO_SECS(time);
    DWORD dwCurrMins = dwCurrSecs / m_iSecondsInMinute;
    dwCurrSecs = dwCurrSecs % m_iSecondsInMinute;

    CAtlString strTime;
    strTime.Format(L"%d:%02d/%d:%02d", dwCurrMins, dwCurrSecs, dwTotalMins, dwTotalSecs);

    m_timeLabel.SetWindowText(strTime);
}

void CTedMainToolbar::UpdateRateDisplay(float flRate)
{
    CAtlString strRateText = LoadAtlString(IDS_RATE);

    CAtlString strRate;
    strRate.Format(L"%s (%2.2f)", strRateText, flRate);
    m_rateLabel.SetWindowText(strRate);
}

void CTedMainToolbar::MarkResolved(bool fResolved)
{
    CAtlString str = LoadAtlString( fResolved ? IDS_TOPO_STATUS_RESOLVED : IDS_TOPO_STATUS_NOT_RESOLVED );
    m_resolvedLabel.SetWindowText(str);
}

LRESULT CTedMainToolbar::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) 
{
    SendMessage(TB_BUTTONSTRUCTSIZE, (WPARAM) sizeof(TBBUTTON), 0);

    TBADDBITMAP tbAddBitmap;
    tbAddBitmap.hInst = _AtlBaseModule.GetModuleInstance();
    tbAddBitmap.nID = UINT_PTR(IDB_MAINTOOL);
    SendMessage(TB_ADDBITMAP, m_iButtonCount, (LPARAM)&tbAddBitmap);

    ZeroMemory(m_buttons, sizeof(m_buttons));
    m_buttons[0].iBitmap = 0;
    m_buttons[0].fsState = 0;
    m_buttons[0].fsStyle = TBSTYLE_BUTTON;
    m_buttons[0].idCommand = ID_PLAY_PLAY;

    m_buttons[1].iBitmap = 1;
    m_buttons[1].fsState = 0;
    m_buttons[1].fsStyle = TBSTYLE_BUTTON;
    m_buttons[1].idCommand = ID_PLAY_STOP;

    m_buttons[2].iBitmap = 2;
    m_buttons[2].fsState = 0;
    m_buttons[2].fsStyle = TBSTYLE_BUTTON;
    m_buttons[2].idCommand = ID_PLAY_PAUSE;

    SendMessage(TB_ADDBUTTONS, m_iButtonCount, (LPARAM)&m_buttons);

    RECT rect;
    GetClientRect(&rect);
    rect.left += m_iToolbarButtonWidth;
    rect.right = rect.left + m_iTrackbarWidth;
        
    m_trackBar.Init(m_hWnd, rect, true, true);

    rect.left = rect.right + m_iMarginSize;
    rect.right = rect.left + m_iTimeLabelWidth;

    rect.top += m_iMarginSize;
    
    CAtlString strFontSize = LoadAtlString(IDS_FONT_SIZE_14);
    CAtlString strFontFace = LoadAtlString(IDS_FONT_FACE_ARIAL);

    LOGFONT lfLabelFont;
    lfLabelFont.lfHeight = _wtol(strFontSize);
    lfLabelFont.lfWidth = 0;
    lfLabelFont.lfEscapement = 0;
    lfLabelFont.lfOrientation = 0;
    lfLabelFont.lfWeight = FW_DONTCARE;
    lfLabelFont.lfItalic = FALSE;
    lfLabelFont.lfUnderline = FALSE;
    lfLabelFont.lfStrikeOut = FALSE;
    lfLabelFont.lfCharSet = DEFAULT_CHARSET;
    lfLabelFont.lfOutPrecision = OUT_DEFAULT_PRECIS;
    lfLabelFont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
    lfLabelFont.lfQuality = DEFAULT_QUALITY;
    lfLabelFont.lfPitchAndFamily = FF_DONTCARE | DEFAULT_PITCH;
    wcscpy_s(lfLabelFont.lfFaceName, 32, strFontFace);
    m_hStaticFont = CreateFontIndirect(&lfLabelFont);

    m_timeLabel.Create(m_hWnd, &rect, L"0:00/0:00", WS_CHILD | WS_VISIBLE);
    m_timeLabel.SetFont(m_hStaticFont);

    rect.left = rect.right + m_iMarginSize;
    rect.right = rect.left + m_iRateLabelWidth;
    m_rateLabel.Create(m_hWnd, &rect, LoadAtlString(IDS_RATE_1_0), WS_CHILD | WS_VISIBLE);
    m_rateLabel.SetFont(m_hStaticFont);

    rect.left = rect.right += m_iMarginSize;
    rect.right = rect.left + m_iRateBarWidth;
    m_rateBar.Init(m_hWnd, rect, true, true);
    
    rect.left = rect.right += m_iMarginSize;
    rect.right = rect.left + m_iResolvedLabelWidth;
    m_resolvedLabel.Create(m_hWnd, &rect, LoadAtlString(IDS_TOPO_STATUS_NOT_RESOLVED), WS_CHILD | WS_VISIBLE);
    m_resolvedLabel.SetFont(m_hStaticFont);
 
    return 0;
}

LRESULT CTedMainToolbar::OnVScroll(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) 
{
    HWND hFocused = ::GetFocus();

    if(hFocused == m_trackBar.m_hWnd)
    {
        m_trackBar.HandleScroll(LOWORD(wParam), HIWORD(wParam));
    }
    else
    {
        m_rateBar.HandleScroll(LOWORD(wParam), HIWORD(wParam));
    }
    
    return 0;
}

LRESULT CTedMainToolbar::OnHScroll(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) 
{
    HWND hFocused = ::GetFocus();

    if(hFocused == m_trackBar.m_hWnd)
    {
        m_trackBar.HandleScroll(LOWORD(wParam), HIWORD(wParam));
    }
    else
    {
        m_rateBar.HandleScroll(LOWORD(wParam), HIWORD(wParam));
    }
    
    return 0;
}
