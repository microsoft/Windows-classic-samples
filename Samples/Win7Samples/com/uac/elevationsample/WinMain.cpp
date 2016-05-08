// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.


// WinMain.cpp : Defines the entry point for the application, Creates the
//					main window and adds the controls.  Defines WndProc and
//					DialogProc to handle all message pumping for the application.
//

#include "stdafx.h"	
#include "WinMain.h"

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	MSG msg;
	HACCEL hAccelTable;
	HRESULT hr = 0;

	if(	!LoadString(hInstance, IDS_CAP_ERROR, tszError, ARRAYSIZE(tszError))
		|| !LoadString(hInstance, IDS_CAP_SUCCESS, tszSuccess, ARRAYSIZE(tszSuccess))
		|| !LoadString(hInstance, IDS_APP_TITLE, tszTitle, ARRAYSIZE(tszTitle))
		|| !LoadString(hInstance, IDC_ElevationSample, tszWindowClass, ARRAYSIZE(tszWindowClass))
			){
				hr = HRESULT_FROM_WIN32(GetLastError());
				MessageBoxFatalError(hr);
				return hr;
		}

	elevMan = new ElevationManager(&hr);
	if(FAILED(hr)){
		delete elevMan;
		MessageBoxFatalError(hr);
		return hr;
	}

	// Initialize global strings
	if(		!RegisterWindow(hInstance)
		||	!InitInstance (hInstance, nCmdShow)
		){
		hr = HRESULT_FROM_WIN32(GetLastError());
		MessageBoxFatalError(hr);
		return hr;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_ElevationSample));
	
	if(!hAccelTable){
		hr = HRESULT_FROM_WIN32(GetLastError());
		MessageBoxFatalError(hr);
		return hr;
	}

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if(IsDialogMessage(hwndDialogTop, &msg))
			continue;
		if(IsDialogMessage(hwndDialogBot, &msg))
			continue;
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	
	delete elevMan;
	return (int) msg.wParam;
}



//
//  FUNCTION: RegisterWindow()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//		Creates the WNDCLASSEX struct and registers the window.
//	
//
ATOM RegisterWindow(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;
	ZeroMemory(&wcex, sizeof(WNDCLASSEX));
	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ElevationSample));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszClassName	= tszWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_ElevationSample));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves an instance handle and creates main window
//
//   COMMENTS: This function stores global handles for 
//		the user interface and creates the main window,
//		adds controls, shows the window, and 
//		forces an initial paint.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{  

   hInst = hInstance; 

   //Create window, don't worry about the size, it will adjust when the
   //dialogs are added.
   hwndMain = CreateWindow(tszWindowClass, tszTitle, WS_SYSMENU,
      CW_USEDEFAULT, 0, 0, 0, NULL, NULL, hInstance, NULL);

	if (!hwndMain
		|| !AddControls(hwndMain)		
		)
   {
      return FALSE;
   }
	ShowWindow(hwndMain, nCmdShow);
	UpdateWindow(hwndMain);
	
	return TRUE;
}

//
//  FUNCTION: DialogProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the dialogs used to
//				create controls.
//
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
		int wmId, wmEvent;
		HWND hwndButton = (HWND)lParam;
		HRESULT hr;
		TCHAR tszFileName[MAX_PATH];

		switch(uMsg){
			case WM_CTLCOLORDLG:
				if(hwndDlg == hwndDialogTop)
					//set top dialog window color to white.
					return (INT_PTR)CreateSolidBrush(RGB(255, 255, 255));
				break;
			case WM_CTLCOLORSTATIC:
				if(hwndDlg == hwndDialogTop)
					//set static text background to white.
					return (INT_PTR)CreateSolidBrush(RGB(255, 255, 255));
				break;
			case WM_COMMAND:

				wmId    = LOWORD(wParam);
				wmEvent = HIWORD(wParam);
			
				switch (wmEvent){
					//find the button that was clicked.
					case BN_CLICKED:				
						if(hwndButton == hwndBrowse){
								//Browse button clicked, open file browser dialog.
								OpenFileDialog(hwndMain, hwndFileName);
								return TRUE;
						}
						else if(hwndButton ==  hwndCancel){
								//Cancel button clicked, close program.
								DestroyWindow(hwndMain);
								return TRUE;
						}
						else if(hwndButton == hwndOk){
								//Ok button clicked, begin registering file.
							if(!GetWindowText(hwndFileName, tszFileName , MAX_PATH)){
									//There was an error getting the text for the filename.
								if(!LoadString(hInst, IDS_TEXT_FILE, tszDisplayText, ARRAYSIZE(tszDisplayText))){
									MessageBoxFatalError(HRESULT_FROM_WIN32(GetLastError()));
									return TRUE;
								}
								else
									MessageBox(hwndMain, tszDisplayText, tszError, MB_OK | MB_ICONERROR);
							}
							else if(SendMessage(hwndReg, BM_GETCHECK, 0, 0) == BST_CHECKED){
									//register checkbox is selected when ok clicked. 
								hr = elevMan->RegisterServer(tszFileName, hwndMain);
								if(FAILED(hr)){								
									MessageBoxError(hr);
								}
								else{
									if(!LoadString(hInst, IDS_REG_SUCCESS, tszDisplayText, ARRAYSIZE(tszDisplayText))){
										MessageBoxFatalError(HRESULT_FROM_WIN32(GetLastError()));
										return TRUE;
									}
									else
										MessageBox(hwndMain, tszDisplayText, tszSuccess, MB_OK | MB_ICONASTERISK);
								}
							}else{
									//unregister checkbox is selected when ok clicked.
								hr = elevMan->UnRegisterServer(tszFileName, hwndMain);
								if(FAILED(hr)){
									MessageBoxError(hr);
								}
								else{
									if(!LoadString(hInst, IDS_UNREG_SUCCESS, tszDisplayText, ARRAYSIZE(tszDisplayText))){
										MessageBoxFatalError(HRESULT_FROM_WIN32(GetLastError()));
										return TRUE;
									}
									else
										MessageBox(hwndMain, tszDisplayText, tszSuccess, MB_OK | MB_ICONASTERISK);
								}
							}
							return TRUE;
						}else if(hwndButton == hwndReg){
							//register checkbox was selected.
							if(!LoadString(hInst, IDS_REG_TEXT, tszDisplayText, ARRAYSIZE(tszDisplayText))){
								MessageBoxFatalError(HRESULT_FROM_WIN32(GetLastError()));
								return TRUE;
							}
							else{
								SetWindowText(hwndText, tszDisplayText);
								SendMessage(hwndUnreg, BM_SETCHECK, BST_UNCHECKED, 0);
							}
							return TRUE;
						}else if(hwndButton == hwndUnreg){
							//unregister checkbox was selected.
							if(!LoadString(hInst, IDS_UNREG_TEXT, tszDisplayText, ARRAYSIZE(tszDisplayText))){
								MessageBoxFatalError(HRESULT_FROM_WIN32(GetLastError()));
								return TRUE;
							}
							else{
								SetWindowText(hwndText, tszDisplayText);
								SendMessage(hwndReg, BM_SETCHECK, BST_UNCHECKED, 0);
							}
							return TRUE;
						}
						break;
				}
				break;
		}
		return 0;
}



//
//   FUNCTION:  AddControls(HWND)
//
//   PURPOSE: Adds all button and textbox classes to the window.
//
//   COMMENTS:  This function add all of the controls for the
//		user interface and stores a handle to each control
//		in a global variable.
//
BOOL AddControls(HWND hwnd){

	//load top dialog with text box
	hwndDialogTop = CreateDialog(hInst, MAKEINTRESOURCE(IDD_DLGTOP), hwnd, DialogProc);
	if(!hwndDialogTop
	   || !SetWindowPos(hwndDialogTop, NULL, 0, 0, 0, 0, SWP_NOSIZE)
	   )	   
	{
	   return FALSE;
	}
	
	ShowWindow(hwndDialogTop, SW_NORMAL);
	//load bottom dialog with buttons
	hwndDialogBot = CreateDialog(hInst, MAKEINTRESOURCE(IDD_DLGBOT), hwnd, DialogProc);
	RECT rcTop, rcBot;
	
	//position the dialogs
	if(!hwndDialogBot
		|| !GetWindowRect(hwndDialogTop, &rcTop)
		|| !SetWindowPos(hwndDialogBot, NULL, 0, rcTop.bottom - rcTop.top, 0, 0 , SWP_NOSIZE)
		|| !GetWindowRect(hwndDialogBot, &rcBot)
		|| !SetWindowPos(hwnd, NULL, 0, 0,rcBot.right - rcBot.left + 8, (rcBot.bottom - rcBot.top) + (rcTop.bottom - rcTop.top) + 28, SWP_NOMOVE)
		)
	{
		return FALSE;
	}
	ShowWindow(hwndDialogBot, SW_NORMAL);

	//get hwnds for items needed later
	hwndFileName = GetDlgItem(hwndDialogTop, IDC_TEXTBOX);
	hwndText = GetDlgItem(hwndDialogTop, IDC_STATICTEXT);
	hwndOk = GetDlgItem(hwndDialogBot, IDC_OK);
	hwndCancel = GetDlgItem(hwndDialogBot, IDC_CANCEL);
	hwndBrowse = GetDlgItem(hwndDialogBot, IDC_BROWSE);
	hwndReg = GetDlgItem(hwndDialogBot, IDC_REG);
	hwndUnreg = GetDlgItem(hwndDialogBot, IDC_UNREG);
	
	if(!hwndFileName || !hwndText || !hwndOk || !hwndCancel || !hwndBrowse || !hwndReg || !hwndUnreg)
		return FALSE;

	//set text for user instructinos.
	if(! LoadString(hInst, IDS_REG_TEXT, tszDisplayText, ARRAYSIZE(tszDisplayText)))
		return FALSE;
	else
		if(!SetWindowText(hwndText, tszDisplayText))
			return FALSE;
	
	//set register radio as default.
	SendMessage(hwndReg, BM_SETCHECK, BST_CHECKED, 0);

	//called in function for clarity in sample
	elevMan->SetButtonShield(hwndOk); 
	
	
	return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//
LRESULT CALLBACK WndProc(HWND hwndMain, UINT message, WPARAM wParam, LPARAM lParam)
{	
	HWND hwndButton = (HWND)lParam;

	switch (message)
	{
	case WM_CREATE:
		break;
	case WM_PAINT:
		DoPaint(hwndMain);
		break;
	case WM_DESTROY:
		DestroyWindow(hwndDialogTop);
		DestroyWindow(hwndDialogBot);
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwndMain, message, wParam, lParam);
	}
	return 0;
}

//
//   FUNCTION:  OpenFileDialog(HWND)
//
//   PURPOSE: Opens the choose file dialog and sets the text of the the 
//			hwndTextBox passed in to the file selected.
//
void OpenFileDialog(HWND hwnd, HWND hwndTextBox){
	OPENFILENAME ofn;			// common dialog box structure
	TCHAR szFile[MAX_PATH];     // buffer for file name
	const TCHAR temp [] = _T("\0*.dll;*.exe;*.ocx\0"); //filter string

	//procedure to load file type label localized.
	if(!LoadString(hInst, IDS_FILETYPES, tszDisplayText, ARRAYSIZE(tszDisplayText))){
		MessageBoxFatalError(HRESULT_FROM_WIN32(GetLastError()));
		return;
	}
	size_t i = _tcslen(tszDisplayText), j = 0, MAX = i+14;
	while(i < MAX){
		tszDisplayText[i++] = temp[j++]; 
	}


// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFile = szFile;
	//
	// Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
	// use the contents of szFile to initialize itself.
	//
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = tszDisplayText;
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

	// Display the Open dialog box. 

	if (GetOpenFileName(&ofn)==TRUE){ 
		if(!SetWindowText(hwndTextBox, ofn.lpstrFile))
			MessageBoxFatalError(HRESULT_FROM_WIN32(GetLastError()));
	}
}

//
//  FUNCTION: DoPaint(HWND)
//
//  PURPOSE: Provides all drawing functionality.
//
void DoPaint(HWND hwnd){
	PAINTSTRUCT ps;
	
	BeginPaint(hwnd, &ps);
	//Do Nothing.  This forces dialogs to paint automatically.
	EndPaint(hwnd, &ps);
}

//
//	FUNCTION: ShowMessageBoxError()
//
//	PURPOSE: Does the work of showing the messagebox
//		given an HRESULT and the headertext.
//
void ShowMessageBoxError(HRESULT dw, const LPTSTR lpsztext){
	LPVOID lpMsgBuf; 

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );
	
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, lpsztext, MB_ICONERROR);
	LocalFree(lpMsgBuf);
}

//
//	FUNCTION: MessageBoxError()
//
//	PURPOSE:	Displays a messagebox with the system text
//		that corresponds to the HRESULT.
//
void MessageBoxError(HRESULT dw){
	ShowMessageBoxError(dw, _T("ElevationSample"));
}

//
//	FUNCTION: MessageBoxFatalError()
//
//	PURPOSE:	Displays a messagebox with the system text
//		that corresponds to the HRESULT and terminates the
//		the application.
//
void MessageBoxFatalError(HRESULT dw){
	ShowMessageBoxError(dw, _T("ElevationSample - Fatal Error!"));
	PostMessage(hwndMain, WM_DESTROY, NULL, NULL);
}




