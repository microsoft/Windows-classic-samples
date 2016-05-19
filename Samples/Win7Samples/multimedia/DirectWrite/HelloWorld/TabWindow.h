
/************************************************************************
 *
 * File: TabWindow.h
 *
 * Description: 
 * 
 * 
 *  This file is part of the Microsoft Windows SDK Code Samples.
 * 
 *  Copyright (C) Microsoft Corporation.  All rights reserved.
 * 
 * This source code is intended only as a supplement to Microsoft
 * Development Tools and/or on-line documentation.  See these other
 * materials for detailed information regarding Microsoft code samples.
 * 
 * THIS CODE AND INFORMATION ARE PROVIDED AS IS WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 * 
 ************************************************************************/

#pragma once

#include <commctrl.h>
#include "SimpleText.h"
#include "MultiformattedText.h"
#include "CustomText.h"

/******************************************************************
*                                                                 *
*  TabWindow                                                      *
*                                                                 *
******************************************************************/

class TabWindow
{
public:
    TabWindow();
    ~TabWindow();

    HRESULT Initialize();

    HWND GetHwnd() { return hwnd_; }

private:
   
    HWND CreateTabControl();
    HRESULT CreateChildWindows();

    HRESULT OnPaint(
        const PAINTSTRUCT &ps
        );

    void OnResize(
        UINT width,
        UINT height
        );

    static LRESULT CALLBACK WndProc(
        HWND hWnd,
        UINT message,
        WPARAM wParam,
        LPARAM lParam
        );

private:
    HWND hwnd_;
    HWND hwndTab_;
    HWND hwndChild_;

    SimpleText simpleText_;
    MultiformattedText multiformattedText_;
    CustomText customText_;
};

