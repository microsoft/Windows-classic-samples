/*----------------------------------------------------------------------------
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (C) 1999 - 2000.  Microsoft Corporation.  All rights reserved.


DskQuota.cpp

Description:
Main module. Makes sure the COM subsystem is initialized and the
disk quota control COM object is initialized before printing out the main
menu.
----------------------------------------------------------------------------*/

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <stdio.h>
#define INITGUIDS
#include <dskquota.h>
#include "ProcessMenu.h"
#include "PrintMenu.h"
#include "Commands.h"

/*-----------------------------------------------------------------------------
wmain(int argc, WCHAR* argv[])
    Initilizes COM, initializes the IDiskQuotaControl

Parameters
    argc - The number of command line parameters
    argv[] - Array of command line parameters

Return Value
    0 - Program terminated with an error
    1 - Program terminated normally
-----------------------------------------------------------------------------*/
extern "C"
int wmain(int argc, WCHAR* argv[])
{
    HRESULT hr;

    // Get command line parameters
    if (argc != 2)
    {
        wprintf(L"\nUsage: %s <Path of Root Directory>\n", argv[0]);
        return 0;
    }

    // Initialize COM
    hr = CoInitialize(NULL);

    if (SUCCEEDED(hr))
    {
        IDiskQuotaControl* lpDiskQuotaControl;

        // Create an instance of the disk quota control
        hr = CoCreateInstance(CLSID_DiskQuotaControl,
                              NULL,
                              CLSCTX_INPROC_SERVER,
                              IID_IDiskQuotaControl,
                              (LPVOID*)&lpDiskQuotaControl);
        if (SUCCEEDED(hr))
        {
            // Initialize IDiskQuotaControl
            hr = lpDiskQuotaControl->Initialize(argv[1], TRUE);

            if (SUCCEEDED(hr))
            {
                // Print main menu stuff
                while (ProcessMainMenu(PrintMainMenu(), lpDiskQuotaControl));
            }

            // Release the IDiskQuotaControl COM object and Uninitialize COM
            lpDiskQuotaControl->Release();
        }
        CoUninitialize();
    }

    if (FAILED(hr)) PrintError(hr);

    return SUCCEEDED(hr) ? 1 : 0;
}
