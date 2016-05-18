/*************************************************************************************************
* Description: Entry point for a sample application that implements a custom control with a 
* Microsoft Active Accessibility (MSAA) server.
*
* The control itself has been kept simple. It does not support scrolling and therefore an arbitrary 
* limit has been set on the number of items it can contain. For convenience, list items are stored 
* in a deque (from the Standard Template Library).
* 
* The accessible object consists of the root element (a list box) and its children (the list items.)
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

    // Show the dialog.
    CoInitialize(NULL);
    DialogBox(hInstance, MAKEINTRESOURCE(IDD_MAINDLG), NULL, DlgProc);
    CoUninitialize();
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
        
        // Add some sample contacts to the custom control.
        SendDlgItemMessage(hDlg, IDC_CUSTOMLISTBOX, CUSTOMLB_ADDITEM, Status_Online, (LPARAM)L"Frank");
        SendDlgItemMessage(hDlg, IDC_CUSTOMLISTBOX, CUSTOMLB_ADDITEM, Status_Online, (LPARAM)L"Sandra");
        SendDlgItemMessage(hDlg, IDC_CUSTOMLISTBOX, CUSTOMLB_ADDITEM, Status_Offline, (LPARAM)L"Kim");
        SendDlgItemMessage(hDlg, IDC_CUSTOMLISTBOX, CUSTOMLB_ADDITEM, Status_Offline, (LPARAM)L"Prakesh");
        SendDlgItemMessage(hDlg, IDC_CUSTOMLISTBOX, CUSTOMLB_ADDITEM, Status_Online, (LPARAM)L"Silvio");
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
            SendDlgItemMessage(hDlg, IDC_CUSTOMLISTBOX, CUSTOMLB_DELETEITEM, 0, 0);
        }
        // "Add" button clicked.
        else if (LOWORD(wParam) == IDC_ADD)
        {
            WCHAR name[MAXNAMELENGTH+1];
            // EM_GETLINE doesn't get a null terminator, so fill buffer with nulls.
            ZeroMemory(name, sizeof(name));
            // Set buffer size for EM_GETLINE.
            name[0] = sizeof(name);
            // Get the new name from the edit control.
            LRESULT charCount =
                SendDlgItemMessage(hDlg, IDC_EDIT1, EM_GETLINE, 0, (LPARAM)name);
            // Empty string, don't do anything.
            if (charCount == 0)
            {
                break;
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
