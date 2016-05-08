// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "stdafx.h"
#include "tedplayer.h"
#include "resource.h"

CTedVideoWindow::CTedVideoWindow()
{

}

CTedVideoWindow::~CTedVideoWindow()
{

}

void CTedVideoWindow::Init(CTedApp * pApp)
{

}
    
     
LRESULT CTedVideoWindow::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    return 0;
}

LRESULT CTedVideoWindow::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    return 0;
}

LRESULT CTedVideoWindow::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    return 0;   
}
    
LRESULT CTedVideoWindow::OnSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    if(SC_CLOSE == wParam)
    {
        bHandled = TRUE;
        HWND hWndParent = GetParent();
        ::SendMessage(hWndParent, WM_COMMAND, MAKELONG(ID_PLAY_STOP, 0), 0);
    }
    else
    {
        bHandled = FALSE;
    }
    
    return 0;   
}
