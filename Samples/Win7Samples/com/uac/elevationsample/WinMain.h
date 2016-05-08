// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#pragma once

#include "resource.h"			
#include <Windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include "WinMain.h"
#include "ElevationManager.h"
#include "Resource.h"

#define MAX_LOADSTRING 100


// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR tszTitle[MAX_LOADSTRING];					// The title bar text
TCHAR tszWindowClass[MAX_LOADSTRING];			// the main window class name
TCHAR tszDisplayText[200];						// Buffer for display text.
TCHAR tszErrSucText [250];						// Buffer for adding error codes.
TCHAR tszError		[15];						// Buffer for "ERROR" heading.
TCHAR tszSuccess	[20];						// Buffer for "SUCCESS" heading.
HWND hwndText;									// Handle to the UI intructions label.
HWND hwndFileName;								// Handle to the user input text box.
HWND hwndOk;									// Handle to the 'OK' button.
HWND hwndCancel;								// Handle to the 'Cancel' button.
HWND hwndBrowse;								// Handle to the 'Browse' button.
HWND hwndReg;									// Handle to the 'Register' radio button.
HWND hwndUnreg;									// Handle to the 'Unregister' radio button.
HWND hwndDialogTop;								// Handle to the top light colored dialog.
HWND hwndDialogBot;								// Handle to the bottom dark colored dialog.
HWND hwndMain;									// Handle to the Main window.

ElevationManager *elevMan;						// The helper class that performs elevation related functionality.

// Forward declarations of functions included in this code module:
ATOM				RegisterWindow(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
BOOL				AddControls(HWND);
void				OpenFileDialog(HWND, HWND);
void				DoPaint(HWND);
void				MessageBoxError(HRESULT);
void				MessageBoxFatalError(HRESULT);

