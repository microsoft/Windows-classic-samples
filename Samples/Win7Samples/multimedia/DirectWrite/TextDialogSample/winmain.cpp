
/************************************************************************
 *
 * File: winmain.cpp
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

#include "TextDialogSample.h"

// Global TextDialog class.
TextDialog textDialog;

// Forward declarations of message Handlers.
void OnPaint(HWND hwnd);
void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT code);
BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
void OnHScroll(HWND hwnd, HWND hwndCtl, UINT code, int pos);

// Message handler for dialog box.
INT_PTR CALLBACK DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        HANDLE_MSG(hDlg, WM_INITDIALOG, OnInitDialog);
        HANDLE_MSG(hDlg, WM_HSCROLL, OnHScroll);
        HANDLE_MSG(hDlg, WM_PAINT, OnPaint);
        HANDLE_MSG(hDlg, WM_COMMAND, OnCommand);
    }

    return (INT_PTR)FALSE;
}


int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR /*lpCmdLine*/, int /*nCmdShow*/)
{
    DialogBox(hInstance, MAKEINTRESOURCE(IDD_ABOUTBOX), 0, DlgProc);
    return 0;
}


// Message Handlers.
void OnPaint(HWND hwnd)
{
    textDialog.DrawD2DContent();
}

BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    WCHAR wFormattedText[255];

    HWND hwndText = GetDlgItem(hwnd, IDC_DWRITE_TEXT);

    HRESULT hr = S_OK;

    // Initialize the textDialog object by passing it a handle to the control it will draw onto.
    textDialog.Initialize(hwndText);

    // Set the slider range and default value.
    SendDlgItemMessage(hwnd, IDC_SIZE_SLIDER, TBM_SETRANGE, 1, MAKELONG(24, 144));
    SendDlgItemMessage(hwnd, IDC_SIZE_SLIDER, TBM_SETPAGESIZE,0, 4);
    SendDlgItemMessage(hwnd, IDC_SIZE_SLIDER, TBM_SETPOS, 1, 72); 

    // Enumerate fonts from the system font collection and fill the font list combo box.
    hr = textDialog.EnumerateFonts(GetDlgItem(hwnd, IDC_FONT_LIST));

    // If an error occurred, display the HRESULT and exit.
    EXIT_ON_ERROR(hr);

    // Select the "Gabriola" font family in the combo box.
    SendDlgItemMessage(hwnd, IDC_FONT_LIST, CB_SELECTSTRING, (WPARAM) -1, (LPARAM) L"Gabriola");
    
    // Fill the text box with initial text.
    wcscpy_s(wFormattedText, ARRAYSIZE(wFormattedText), L"Formatted Text");

    SendDlgItemMessage(hwnd, IDC_INPUT_TEXT, WM_SETTEXT, 0, (LPARAM) wFormattedText);
    SendDlgItemMessage(hwnd, IDC_INPUT_TEXT, EM_SETLIMITTEXT, (WPARAM) 31, 0);

	// Set the initial text.
	hr = textDialog.SetText(wFormattedText);

	// If an error occurred, display the HRESULT and exit.
    EXIT_ON_ERROR(hr);

    return (INT_PTR)TRUE;
}

void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT code)
{
    HRESULT hr = S_OK;

    switch (id)
    {
        case IDC_INPUT_TEXT:
            if (code == EN_CHANGE)
            {
                // Get the length of the string.
                UINT32 len = (UINT32) SendDlgItemMessage(hwnd, IDC_INPUT_TEXT, EM_LINELENGTH, 0, 0);
                
                PWSTR wFormattedText = new (std::nothrow) wchar_t [len + 1];
                hr = wFormattedText ? S_OK : E_OUTOFMEMORY;
                if (SUCCEEDED(hr))
                {
                    // the lParam is an in/out, with the in specifying the buffer size
                    wFormattedText[0] = static_cast<wchar_t>(len + 1);
                    SendDlgItemMessage(hwnd, IDC_INPUT_TEXT, EM_GETLINE, 0, (LPARAM) wFormattedText);

                    // Null terminate the string.
                    wFormattedText[len] = '\0';

                    // Set the text to the new string.
                    hr = textDialog.SetText(wFormattedText);
                    delete [] wFormattedText;
                }

                // If an error occurred, display the HRESULT and exit.
                EXIT_ON_ERROR(hr);
            }
            break;
        case IDC_FONT_LIST:
            if (code == CBN_SELCHANGE)
            {
                // Get the font family name from the combo box.
                UINT32 index = (UINT32) SendDlgItemMessage(hwnd, IDC_FONT_LIST, CB_GETCURSEL, 0, 0);
                UINT32 len = (UINT32) SendDlgItemMessage(hwnd, IDC_FONT_LIST, CB_GETLBTEXTLEN, index, 0);

                wchar_t *fontFamily = new wchar_t[len+1];

                if (fontFamily == NULL)
                {
                    hr = E_OUTOFMEMORY;
                }

                if (SUCCEEDED(hr))
                {
                    SendDlgItemMessage(hwnd, IDC_FONT_LIST, CB_GETLBTEXT, (WPARAM) index, (LPARAM) fontFamily);

                    // Set the font using the font family name.
                    hr = textDialog.SetFont(fontFamily);
                }

                // Note that we do not delete the fontFamily string.  The SetFont method stores the string we send.

                // If an error occurred, display the HRESULT and exit.
                EXIT_ON_ERROR(hr);
            }
            break;
        case IDC_BOLD:
            if (code == BN_CLICKED)
            {
                UINT32 state = IsDlgButtonChecked(hwnd, IDC_BOLD);

                // Set the font weight.
                if (state == BST_CHECKED)
                {
                    hr = textDialog.SetBold(true);
                }
                else
                {
                    hr = textDialog.SetBold(false);
                }

                // If an error occurred, display the HRESULT and exit.
                EXIT_ON_ERROR(hr);
            }
            break;
        case IDC_ITALIC:
            if (code == BN_CLICKED)
            {
                UINT32 state = IsDlgButtonChecked(hwnd, IDC_ITALIC);

                // Set the font style.
                if (state == BST_CHECKED)
                {
                    hr = textDialog.SetItalic(true);
                }
                else
                {
                    hr = textDialog.SetItalic(false);
                }

                // If an error occurred, display the HRESULT and exit.
                EXIT_ON_ERROR(hr);
            }
            break;
        case IDC_UNDERLINE:
            if (code == BN_CLICKED)
            {
                UINT32 state = IsDlgButtonChecked(hwnd, IDC_UNDERLINE);

                // Turn the underline on or off.
                if (state == BST_CHECKED)
                {
                    hr = textDialog.SetUnderline(true);
                }
                else
                {
                    hr = textDialog.SetUnderline(false);
                }

                // If an error occurred, display the HRESULT and exit.
                EXIT_ON_ERROR(hr);
            }
            break;
        
        case IDOK:
            break;
        case IDCANCEL:
            EndDialog(hwnd, code);
            break;
    }
}

void OnHScroll(HWND hwnd, HWND hwndCtl, UINT code, int pos)
{   
    HRESULT hr = S_OK;

    pos = (int) SendDlgItemMessage(hwnd, IDC_SIZE_SLIDER, TBM_GETPOS, 0, 0); 

    WCHAR wFontSize[5];

    // Display the font size in the dialog.
    swprintf_s(wFontSize, ARRAYSIZE(wFontSize), L"%i", (int) pos);
    SendDlgItemMessage(hwnd, IDC_SIZE_LABEL, WM_SETTEXT,  0, (LPARAM) wFontSize);

    // Change the font size of the text.
    hr = textDialog.SetFontSize((float) pos);

    // If an error occurred, display the HRESULT and exit.
    EXIT_ON_ERROR(hr);
}

