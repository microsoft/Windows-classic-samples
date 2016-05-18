//////////////////////////////////////////////////////////////////////////
// winmain.cpp : Defines the entry point for the application.
// 
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//////////////////////////////////////////////////////////////////////////

#include "DShowPlayer.h"
#include "resource.h"

#include "MainWindow.h"

#define MAX_LOADSTRING 100




int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)	
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	MSG msg;
	HACCEL hAccelTable;
    HRESULT hr = S_OK;


	INITCOMMONCONTROLSEX icc;
	icc.dwSize = sizeof(icc);
	icc.dwICC = ICC_COOL_CLASSES | ICC_BAR_CLASSES;
	
    if (!InitCommonControlsEx(&icc))
    {
        NotifyError(NULL, TEXT("InitCommonControlsEx failed."), E_FAIL);
        return -1;
    }

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_DSHOWPLAYER));

	
	hr = CoInitialize(NULL);
    if (FAILED(hr))
    {
        NotifyError(NULL, TEXT("CoInitialize failed."), hr);
        return -1;
    }

	MainWindow *pWin = new MainWindow();

    if (pWin == NULL)
    {
        hr = E_OUTOFMEMORY;
    }

    if (SUCCEEDED(hr))
    {
    	hr = pWin->Create(hInstance);
    }

    if (SUCCEEDED(hr))
    {
    	hr = pWin->Show(nCmdShow);
    }

    if (FAILED(hr))
    {
        NotifyError(NULL, TEXT("Could not initialize the application"), hr);
        CoUninitialize();
        return -1;
    }


	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	delete pWin;

	CoUninitialize();

	return (int) msg.wParam;
}


