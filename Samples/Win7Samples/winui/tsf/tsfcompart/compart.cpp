/**************************************************************************
   THIS CODE AND INFORMATION IS PROVIDED 'AS IS' WITHOUT WARRANTY OF
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
   PARTICULAR PURPOSE.

   Copyright 2001 Microsoft Corporation. All Rights Reserved.
**************************************************************************/

/**************************************************************************

   File:          Compart.cpp

   Description:   

**************************************************************************/

/**************************************************************************
	#include statements
**************************************************************************/

#include <windows.h>
#include <windowsx.h>
#include "resource.h"
#include "Monitor.h"
#include <tchar.h>

/**************************************************************************
	function prototypes
**************************************************************************/

INT_PTR CALLBACK DialogProc(HWND hWnd, 
                            UINT uMessage, 
                            WPARAM wParam, 
                            LPARAM lParam);
HRESULT CALLBACK MonitorCallback(   const GUID* pguidCompartment, 
                                    BOOL fStatus,
                                    LPARAM lParam);

/**************************************************************************
	global variables and definitions
**************************************************************************/

/**************************************************************************

	WinMain()

**************************************************************************/

int WINAPI WinMain( HINSTANCE hInstance,
                    HINSTANCE hPrevInstance,
                    LPSTR lpCmdLine,
                    int nCmdShow)
{
    CoInitialize(NULL);
    
    DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG), NULL, DialogProc);
    
    CoUninitialize();

    return 0;
}

/**************************************************************************

	DialogProc()

**************************************************************************/

INT_PTR CALLBACK DialogProc(    HWND hWnd, 
                                UINT uMessage, 
                                WPARAM wParam, 
                                LPARAM lParam)
{
    static CCompartmentMonitor *s_pCompartmentMonitor;

    switch(uMessage)
    {
    case WM_INITDIALOG:
        s_pCompartmentMonitor = new CCompartmentMonitor();
        if(s_pCompartmentMonitor)
        {
            HRESULT hr;
            
            hr = s_pCompartmentMonitor->Initialize(&GUID_COMPARTMENT_SPEECH_OPENCLOSE, MonitorCallback, (LPARAM)hWnd);
            if(FAILED(hr))
            {
                s_pCompartmentMonitor->Release();
                s_pCompartmentMonitor = NULL;
            }
            else
            {
                //update the status text
                BOOL    fStatus;

                s_pCompartmentMonitor->GetStatus(&fStatus);

                MonitorCallback(&GUID_COMPARTMENT_SPEECH_OPENCLOSE, fStatus, (LPARAM)hWnd);
            }
        }
        break;
    
    case WM_COMMAND:
        switch(GET_WM_COMMAND_ID(wParam, lParam))
        {
        case IDC_CLOSE:
            PostMessage(hWnd, WM_CLOSE, 0, 0);
            break;
        }
        break;
    
    case WM_CLOSE:
        EndDialog(hWnd, 0);
        break;

    case WM_DESTROY:
        if(s_pCompartmentMonitor)
        {
            s_pCompartmentMonitor->Uninitialize();
            s_pCompartmentMonitor->Release();
            s_pCompartmentMonitor = NULL;
        }
        break;
    }

    return FALSE;
}

/**************************************************************************

	MonitorCallback()

**************************************************************************/

HRESULT CALLBACK MonitorCallback(   const GUID* pguidCompartment, 
                                    BOOL fStatus, 
                                    LPARAM lParam)
{
    TCHAR   szSpeechStatus[MAX_PATH];
    
    //get the status of the speech text service
    if(fStatus)
    {
        _tcscpy_s(szSpeechStatus, ARRAYSIZE(szSpeechStatus), TEXT("Speech Is Active"));
    }
    else
    {
        _tcscpy_s(szSpeechStatus, ARRAYSIZE(szSpeechStatus), TEXT("Speech Is Inactive"));
    }
    
    SetDlgItemText((HWND)lParam, IDC_TEXT, szSpeechStatus);
    
    return S_OK;
}

