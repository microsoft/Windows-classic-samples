/*************************************************************************************************
 * Description: Entry point for a sample application that displays a dialog box containing
 * a custom contol that supports UI Automation.
 *
 * The control is a simple button-like control that supports InvokePattern. Clicking the 
 * button causes it to change color. You can also tab to the button and click it by pressing
 * the spacebar.
 * 
 * To test the functionality of InvokePattern, you can use the UISpy tool. Click on the control
 * in the UI Automation raw view or control view and then select Control Patterns from the
 * View menu. In the Control Patterns dialog box, you can call the InvokePattern::Invoke method.
 * 
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
 *************************************************************************************************/

#pragma once

#include "resource.h"
#include "stdafx.h"
#include "Control.h"
#include <ole2.h>

#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

// Forward declarations of functions included in this code module.
INT_PTR CALLBACK    DlgProc(HWND, UINT, WPARAM, LPARAM);

// Entry point.
int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR /*lpCmdLine*/, int /*nCmdShow*/)
{
    CoInitialize(NULL);

    // Register the window class for the CustomButton control.
    CustomButton::RegisterControl(hInstance);

    DialogBox(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, DlgProc);

    CoUninitialize();
    
    return 0;
}

// Message handler for application dialog.
INT_PTR CALLBACK DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM /*lParam*/)
{
    switch (message)
    {
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return TRUE;
        }
        break;
    }
    return FALSE;
}

