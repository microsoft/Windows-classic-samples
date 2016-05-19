//+---------------------------------------------------------------------------
//
//  Copyright (c) Microsoft Corporation. All rights reserved. 
//
//
//  BITS Upload sample
//  ==================
//
//  Module name: 
//  main.cpp
//
//  Purpose:
//  This module contains the entry point for the uploadsample.exe (WinMain).
//  It takes care of initialization of COM and launches the User Interface.
//
//----------------------------------------------------------------------------


#include <windows.h>
#include <crtdbg.h>
#include <strsafe.h>
#include <bits.h>

#include "resource.h"
#include "main.h"
#include "util.h"
#include "cdialog.h"


CSmartComPtr<IBackgroundCopyManager> g_JobManager;
CMonitor                             g_NotificationReceiver;
CSimpleDialog                       *g_pDialog = NULL;
HANDLE                               g_hSafeToExitEvent = INVALID_HANDLE_VALUE;

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     INT       iShowState)
{
    HRESULT        hr      = S_OK;
    LRESULT        lResult = 0;

    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (FAILED(hr))
    {
        goto cleanup;
    }

    //
    // Instantiate the main BITS object, the Job Manager
    // 
    hr = CoCreateInstance(CLSID_BackgroundCopyManager, NULL, CLSCTX_LOCAL_SERVER, __uuidof(IBackgroundCopyManager), reinterpret_cast<LPVOID *>(g_JobManager.GrabOutPtr()));
    if (FAILED(hr))
    {
        DisplayErrorMessage(L"Failed to instantiate a CLSID_BackgroundCopyManager COM object. Aborting"
                            L"Check if BITS is correctly installed on your system.", hr);
        goto cleanup;
    }

    //
    // Create the dialog for our test application
    //
    g_pDialog = new CSimpleDialog(hInstance, IDD_UPLOADSAMPLECLIENTDIALOG);
    if (!g_pDialog)
    {
        DisplayErrorMessage(L"Could not create the UI for the BITS upload sample application. Aborting.", E_OUTOFMEMORY);
        goto cleanup;
    }

    hr = g_pDialog->Show(iShowState);
    if (FAILED(hr))
    {
        DisplayErrorMessage(L"Could not display the UI for the BITS upload sample application. Aborting.", hr);
        goto cleanup;
    }


    //
    // Starts the message loop for the dialog
    //

    INT iRet;
    MSG msg;

    iRet = GetMessage(&msg, NULL, 0, 0);
    while (iRet != 0)
    { 
        if (iRet == -1)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            DisplayErrorMessage(L"Error processing the application window message loop", hr);
            goto cleanup;
        }
        else
        {
            if (!IsWindow(g_pDialog->GetHwnd()) || !IsDialogMessage(g_pDialog->GetHwnd(), &msg)) 
            { 
                TranslateMessage(&msg); 
                DispatchMessage(&msg); 
            }
        } 

        iRet = GetMessage(&msg, NULL, 0, 0);
    } 

cleanup:

    if (g_pDialog)
    {
        delete g_pDialog;
    }

    if (g_hSafeToExitEvent != INVALID_HANDLE_VALUE)
    {
        //
        // We need to wait here because there's chances that COM isn't yet done with
        // releasing our callback interface. The app needs to be around in case any pending reference
        // is executed. 
        // The the last reference is released, the g_hSafeToExitEvent is going to be signaled
        // and we will unblock from this wait.
        //
        DWORD dwWait = WaitForSingleObject(g_hSafeToExitEvent, INFINITE);

        // Regardless of the reason we unblocked, go ahead and close
        // the event because we are about to quit the application
        CloseHandle(g_hSafeToExitEvent);
        g_hSafeToExitEvent = INVALID_HANDLE_VALUE;
    }


    // g_JobManager is a global variable, so force the release of the interface pointer
    g_JobManager.Clear();
    CoUninitialize();

    return 0;
}

