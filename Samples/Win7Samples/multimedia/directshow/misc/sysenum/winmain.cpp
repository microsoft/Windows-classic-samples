//------------------------------------------------------------------------------
//
// File: AMCap2.cpp
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//------------------------------------------------------------------------------

#include "SysEnum.h"
#include "MainDialog.h"

HINSTANCE g_hInstance;



int WINAPI _tWinMain(HINSTANCE  hInstance,
                     HINSTANCE  /*hPrevInstance*/,
                     LPTSTR     lpstrCmdLine,
                     int        nCmdShow)
{

    g_hInstance = hInstance;

	HRESULT hr =    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

    if (FAILED(hr))
    {
        MessageBox(NULL, TEXT("CoInitialize failed."), NULL, MB_ICONSTOP);
        return 1;
    }

	MainDialog *pDlg = new MainDialog();
	if (pDlg == NULL)
	{
        MessageBox(NULL, TEXT("Out of memory."), NULL, MB_ICONSTOP);
	}
	else
	{
		pDlg->ShowDialog(hInstance, NULL);

		delete pDlg;
	}




    CoUninitialize();

    return 0;
}