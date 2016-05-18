//==========================================================================
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//--------------------------------------------------------------------------

//------------------------------------------------------------
//Please read the ReadME.txt which explains the purpose of the
//sample.
//-------------------------------------------------------------

#include <windows.h>
#include <objbase.h>
#include <stdio.h>
#include <tchar.h>
#include <wia.h>
#include <oleauto.h>
#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>
#include <shlwapi.h>

#define MAX_FILE_PATH 1024
#define MAX_FOLDER_PATH 1024


// Helper function to display an error message and an optional HRESULT
void ReportError( LPCTSTR pszMessage, HRESULT hr )
{
    if (S_OK != hr)
    {
        _tprintf( TEXT("\n%s: HRESULT: 0x%08X"), pszMessage, hr );
    }
    else
    {
        _tprintf( TEXT("\n%s"), pszMessage );
    }
}


// The entry function of the application
extern "C" 
int __cdecl _tmain( int, TCHAR *[] )
{
    LONG                nFiles;
    LPWSTR              *rFilePaths = NULL;
    IWiaItem2           *pItem = NULL;
    IWiaItem2           *pItemRoot = NULL;
    BSTR                devName = NULL;
    TCHAR               szFolder[MAX_FOLDER_PATH];
    TCHAR               szFileName[MAX_FILE_PATH];
    
    // Initialize COM
    HRESULT hr = CoInitialize(NULL);

    GetTempPath(MAX_FOLDER_PATH , szFolder);
    
    StringCchPrintf(szFileName, ARRAYSIZE(szFileName), TEXT("%ws"), TEXT("ScanDialogFile"));

    if (SUCCEEDED(hr))
    {
        // Create the device manager
        IWiaDevMgr2 *pWiaDevMgr2 = NULL;
        hr = CoCreateInstance( CLSID_WiaDevMgr2, NULL, CLSCTX_LOCAL_SERVER, IID_IWiaDevMgr2, (void**)&pWiaDevMgr2 );
        if (SUCCEEDED(hr))
        {
            //create a scan dialog and a select device UI
            hr = pWiaDevMgr2->GetImageDlg(0,NULL,0, szFolder, szFileName, &nFiles, &rFilePaths, &pItem); 
            if (FAILED(hr))
            {
                ReportError( TEXT("Could not create Scan Dialog"), hr );
            }

            // Release the device manager
            pWiaDevMgr2->Release();
            pWiaDevMgr2 = NULL;
        }
        else
        {
            ReportError( TEXT("CoCreateInstance() failed on CLSID_WiaDevMgr"), hr );
        }

        // Uninitialize COM
        CoUninitialize();
    }
    return 0;
}

