/*************************************************************************************************
* Description: Entry point for a sample application that implements a custom control with a 
* UI Automation provider.
*
* The sample is aimed at demonstrating how to implement UI Automation providers for the elements 
* of a fragment that is hosted in an HWND. The control itself has been kept simple. It does not
* support scrolling and therefore an arbitrary limit has been set on the number of items it
* can contain. For convenience, list items are stored in a deque (from the Standard Template Library).
* 
* The fragment consists of the root element (a list box) and its children (the list items).
* The UI Automation provider for the list box supports only one control pattern, the Selection 
* pattern. The provider for the list items implements the SelectionItem pattern.
*
* To see the UI Automation provider at work, run the application and then open UI Spy. You can
* see the application and its controls in the Control View. Clicking on the control or on the
* list items in the UI Spy tree causes the provider's methods to be called, and the values 
* returned are displayed in the Properties pane. (Note that some values are retrieved from the
* default HWND provider, not this custom implementation.) You can also select an item in the
* list by using the SelectionItem control pattern in UI Spy.
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

#include "stdafx.h"
#include <ole2.h>
#include "resource.h"
#include "CustomControl.h"

#define MAXNAMELENGTH 15
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")


INT_PTR CALLBACK    DlgProc(HWND, UINT, WPARAM, LPARAM);

// Entry point.
int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR /*lpCmdLine*/, int /*nCmdShow*/)
{
    // Register the window class for the CustomList control.
    RegisterListControl(hInstance);

    DialogBox(hInstance, MAKEINTRESOURCE(IDD_MAINDLG), NULL, DlgProc);
    return 0;
}

// Message handler for application dialog.
INT_PTR CALLBACK DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM /*lParam*/)
{
    switch (message)
    {
    case WM_INITDIALOG:
        // Limit length of new contact names.
        SendDlgItemMessage(hDlg, IDC_EDIT1, EM_SETLIMITTEXT, MAXNAMELENGTH, 0);
        // Initialize radio buttons.
        SendDlgItemMessage(hDlg, IDC_ONLINE, BM_SETCHECK, 1, 0);
        break;

    case WM_COMMAND:
        // Exit.
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return TRUE;
        }
        // "Remove" button clicked.
        else if (LOWORD(wParam) == IDC_REMOVE)
        {
            SendDlgItemMessage(hDlg, IDC_CUSTOMLISTBOX, WM_DELETEITEM, 0, 0);
        }
        // "Add" button clicked.
        else if (LOWORD(wParam) == IDC_ADD)
        {
            WCHAR name[MAXNAMELENGTH+1];
            // EM_GETLINE doesn't get a null terminator, so fill buffer with nulls.
            ZeroMemory(name, sizeof(name));
            // Set buffer size for EM_GETLINE.
            name[0] = ARRAYSIZE(name);
            // Get the new name from the edit control.
            LRESULT charCount = SendDlgItemMessage(hDlg, IDC_EDIT1, EM_GETLINE, 0, (LPARAM)name);
            if (charCount == 0) 
            {
                return FALSE;
            }
            // Get the status
            ContactStatus status; 
            if (BST_CHECKED == SendDlgItemMessage(hDlg, IDC_ONLINE, BM_GETCHECK, 0, 0))
            {
                status = Status_Online;
            }
            else
            {
                status = Status_Offline;
            }
            // Send name and status to the list box control.
            SendDlgItemMessage(hDlg, IDC_CUSTOMLISTBOX, CUSTOMLB_ADDITEM, (WPARAM)status, (LPARAM)name);
            // Select and clear the edit text.
            SendDlgItemMessage(hDlg, IDC_EDIT1, EM_SETSEL, 0, MAXNAMELENGTH);
            SendDlgItemMessage(hDlg, IDC_EDIT1, WM_CLEAR, 0, 0);
        }
        break;
    } // switch message
    return FALSE;
}
