/**************************************************************************
   THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
   PARTICULAR PURPOSE.
 
   © Copyright 2002 Microsoft Corporation.  All Rights Reserved.
**************************************************************************/
#include <windows.h>
#include <windows.h>
#include <commctrl.h>
#include <Shellapi.h>
#include "Resource.h"

#pragma comment(lib, "comctl32.lib")
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")

//Callback function called by Taskdialog
HRESULT CALLBACK TaskDialogCallbackProc(HWND hwnd,
                                     UINT uNotification,
                                     WPARAM wParam,
                                     LPARAM lParam,
                                     LONG_PTR dwRefData)
{
	HRESULT hr = S_OK;

	switch (uNotification)
	{
	case TDN_TIMER:
		//wParam is the number of milliseconds since the dialog was created or this notification returned TRUE
		int dwTickCount = (int) wParam / 200;
		
        if (dwTickCount < 100)
		{
			SendMessage(hwnd, TDM_SET_PROGRESS_BAR_POS, dwTickCount, 0);
		}
		else if( dwTickCount < 130)
		{
			SendMessage(hwnd, TDM_SET_PROGRESS_BAR_POS, 100, 0);
		}
		else
		{
			//Return S_FALSE to reset dwTickCount
			hr = TRUE;
		}

		break;
	}

	return hr;
}


HRESULT TaskDialog_Sample(HWND hwndParent, 
                          HINSTANCE hInstance, 
                          BOOL *pbVerificationFlagChecked)
{
	TASKDIALOGCONFIG tdConfig;
    
	ZeroMemory(&tdConfig, sizeof(tdConfig));
	tdConfig.cbSize = sizeof(TASKDIALOGCONFIG);
	tdConfig.hwndParent = hwndParent;
	tdConfig.hInstance = hInstance;
	
	//Set the call back function for the TaskDialog to use.
	tdConfig.pfCallback = TaskDialogCallbackProc;	

	tdConfig.pszWindowTitle = MAKEINTRESOURCE(IDS_TITLE);
	tdConfig.pszMainInstruction = MAKEINTRESOURCE(IDS_MAININSTRUCTION);
	tdConfig.pszContent = MAKEINTRESOURCE(IDS_CONTENT);

	//Set the flags to show the progress bar and for TaskDialog to call back every 200 milliseconds
	tdConfig.dwFlags = TDF_SHOW_PROGRESS_BAR | TDF_CALLBACK_TIMER;
	return TaskDialogIndirect(&tdConfig, NULL, NULL, NULL);
}

LRESULT CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lr = FALSE;
	
    switch (uMsg)
	{
	case WM_CLOSE:
		//Closes the Task Dialog Demo Window
		lr = EndDialog(hwndDlg, NULL);
		break;
	
    case WM_COMMAND:
		switch (LOWORD(wParam))
		{	
		case IDC_LAUNCH:
			//Creates Task Dialog Window
			lr = TaskDialog_Sample(hwndDlg, NULL, NULL);
			break;
		
        case IDC_EXIT:
			EndDialog(hwndDlg, NULL);
		}
	}

	return lr;
}


int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrev, LPSTR lpCmdLine, int nCmdShow)
{
	//Creates Task Dialog Demo Window

    #pragma warning(push)
    #pragma warning(disable:4244)

	return DialogBox(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, (DLGPROC) DialogProc);

    #pragma warning(pop)
}