//
//  This is part of the Microsoft Tablet PC Platform SDK
//  Copyright (C) 2002 Microsoft Corporation
//  All rights reserved.
//
//  This source code is only intended as a supplement to the
//  Microsoft Tablet PC Platform SDK Reference and related electronic
//  documentation provided with the Software Development Kit.
//  See these sources for more detailed information.
//
// Module:
//      TPCInfo.cpp
//
// Description:
//      This program checks the presence and configuration of the
//      Microsoft Tablet PC Platform core components.
//      It finds out whether Tablet PC components are enabled in the
//      operating system, shows the names and version info of the
//      core controls and default handwriting and speech recognizers.
//
//      This application is discussed in the Getting Started guide.
//
//--------------------------------------------------------------------------

// Windows header files
#include <windows.h>
#include <comdef.h>
#include <wchar.h>

// Headers for Tablet PC Automation interfaces
#include <msinkaut.h>
#include <msinkaut_i.c>

// Header for Safe String Operations
#include <strsafe.h>

// The sample's resource header file
#include "resource.h"

// The system metrics index for checking on Tablet PC components
#ifndef SM_TABLETPC
#define SM_TABLETPC     86
#endif

// A useful macro to calculate the number of elements in an array
#ifndef countof
// 'A' must be the name of an array, not just a pointer,
// so that sizeof(A) would give the size of the array
#define countof(A) (sizeof(A)/sizeof(A[0]))
#endif

// IDs of the dialog static controls
#define NUM_CONTROLS    2
const UINT  gc_uiCtrlId[NUM_CONTROLS][2] = {{IDC_CTL1_NAME, IDC_CTL1_VER},
                                            {IDC_CTL2_NAME, IDC_CTL2_VER}};

// ProgID's of the Tablet PC controls to check on
LPCOLESTR gc_wszProgId[NUM_CONTROLS] = {L"InkEd.InkEdit", L"msinkaut.InkPicture"};

// The key to the registry settings of the installed speech recognizers
const WCHAR* gc_wszSpeechKey = L"Software\\Microsoft\\Speech\\Recognizers";

// CLSID of the Text Services Framework's ThreadManager object
const CLSID CLSID_TF_ThreadMgr =
        { 0x529A9E6B,0x6587,0x4F23,{ 0xAB,0x9E,0x9C,0x7D,0x68,0x3E,0x3C,0x50 } };

// A helper structure, used in the GetComponentInfo function
typedef struct
{
    WCHAR wchName[256];
    WCHAR wchVersion[256];
} SInfo;

const WCHAR gc_wszAppName[] = L"TPCInfo Sample Application";


/////////////////////////////////////////////////////////
//
// GetComponentInfo
//
//        This helper function, provided with a component
//        ProgId, gathers the component's name and version
//        info and formats them into output strings
//
// Parameters:
//        CLSID  clsid  : [in] component's CLSID
//        SInfo& info   : [in, out] a structure of buffers
//                        to get the formatted strings into
//
// Return Values:
//        TRUE, if the function succeeds
//        FALSE, if it fails
//
/////////////////////////////////////////////////////////
BOOL GetComponentInfo(CLSID clsid, SInfo& info)
{
    info.wchName[0] = info.wchVersion[0] = 0;

    // Format Registry Key string
    WCHAR wszKey[45] = L"CLSID\\";  // the key buffer should be large enough for a string
                                    // like "CLSID\{xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx}"
    // Convert CLSID to String
    UINT uPos = lstrlenW(wszKey);
    if (0 == StringFromGUID2(clsid, &wszKey[uPos], countof(wszKey) - uPos))
        return FALSE;
    wszKey[countof(wszKey)-1] = 0;

    // Open key to find path of application
    HKEY hKeyRoot;
    if (RegOpenKeyExW(HKEY_CLASSES_ROOT, wszKey, 0, KEY_READ, &hKeyRoot) != ERROR_SUCCESS)
        return FALSE;

    // Query value of key to get the name of the component
    ULONG cSize = sizeof(info.wchName);  // size of the buffer in bytes
    if (RegQueryValueExW(hKeyRoot, NULL, NULL, NULL, (BYTE*)info.wchName, &cSize) != ERROR_SUCCESS)
    {
        RegCloseKey(hKeyRoot);
        return FALSE;
    }
    info.wchName[countof(info.wchName) - 1] = 0;

    // Open the version info subkey
    UINT iVersionMaxLen = countof(info.wchVersion);
    HKEY hKey = NULL;
    if (RegOpenKeyExW(hKeyRoot, L"Version", 0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {
        const WCHAR* pcwsVersion = L"version ";
        UINT iLen = lstrlenW(pcwsVersion);
        // Query value of key to get version string
        if (iLen < iVersionMaxLen)
        {
            // copy the "version " string including terminating 0
            wcsncpy_s(info.wchVersion, iVersionMaxLen, pcwsVersion, iLen + 1);

            // get the version string
            cSize = (iVersionMaxLen - iLen) * sizeof(WCHAR); // the size is in bytes
            if (RegQueryValueExW(hKey, NULL, NULL, NULL,
                                 (BYTE*)&info.wchVersion[iLen],
                                 &cSize) == ERROR_SUCCESS)
            {
                info.wchVersion[iVersionMaxLen-1] = 0;
            }
        }
        RegCloseKey(hKey);
    }

    // Open InprocServer32 subkey to get the path to the component
    if (RegOpenKeyExW(hKeyRoot, L"InprocServer32", 0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {
        // Query value of key to get the path string
        WCHAR wchPath[MAX_PATH];
        cSize = sizeof(wchPath);
        if (RegQueryValueExW(hKey, NULL, NULL, NULL, (BYTE*)wchPath, &cSize) == ERROR_SUCCESS)
        {
            // Get the build number from the file version info
            DWORD dwHandle = 0;
            cSize = GetFileVersionInfoSizeW(wchPath, &dwHandle); // returns the size in bytes
            WCHAR* pwchFileVerInfo = NULL;
            if (cSize)
            {
                pwchFileVerInfo = (WCHAR*)new BYTE[cSize];
            }
            if (NULL != pwchFileVerInfo)
            {
                // Retrieve version information for the file
                if (GetFileVersionInfoW(wchPath, 0, cSize, pwchFileVerInfo))
                {
                    // Get the default language id and code page number
                    UINT *pdwLang;
                    UINT cch = 0;
                    if (VerQueryValueW(pwchFileVerInfo, L"\\VarFileInfo\\Translation",
                                       (void**)&pdwLang, &cch) == TRUE)
                    {
                        // Read the file description for the language and code page.
                        const int MAX_SUBBLOCK = 40;
                        WCHAR wchSubBlock[MAX_SUBBLOCK];  // large enough for the string
                        StringCchPrintfExW(wchSubBlock,
                                          MAX_SUBBLOCK,
                                          NULL,
                                          NULL,
                                          STRSAFE_NULL_ON_FAILURE,
                                          L"\\StringFileInfo\\%04x%04x\\FileVersion",
                                          LOWORD(*pdwLang), HIWORD(*pdwLang));

                        WCHAR* pwchBuildVer = NULL;
                        if ((VerQueryValueW(pwchFileVerInfo, wchSubBlock,
                                            (void**)&pwchBuildVer, &cch) == TRUE)
                            && (NULL != pwchBuildVer))
                        {
                            // Format the version string
                            UINT iLen = (UINT)lstrlenW(info.wchVersion);
                            if (0 < iLen)
                            {
                                if (iLen < iVersionMaxLen)
                                {
                                    const WCHAR* pcwsBuild = L", build ";
                                    wcsncpy_s(info.wchVersion + iLen, iVersionMaxLen - iLen, pcwsBuild, iVersionMaxLen - iLen);
                                    iLen += lstrlenW(pcwsBuild);
                                    if (iLen < iVersionMaxLen)
                                    {
                                        wcsncpy_s(info.wchVersion + iLen, iVersionMaxLen - iLen, pwchBuildVer, iVersionMaxLen - iLen);
                                    }
                                }
                            }
                            else
                            {
                                wcsncpy_s(info.wchVersion, iVersionMaxLen, pwchBuildVer, iVersionMaxLen);
                            }
                            info.wchVersion[iVersionMaxLen-1] = 0;
                        }
                    }
                }
                delete [] pwchFileVerInfo;
            }

        }
        RegCloseKey(hKey);
    }

    RegCloseKey(hKeyRoot);

    return TRUE;
}

/////////////////////////////////////////////////////////
//
// DlgProc
//
//        The DlgProc function is an application-defined
//        function that processes messages sent to the dialog
//
// Parameters:
//        HWND hwnd      : [in] handle to dialog window
//        UINT uMsg      : [in] message identifier
//        WPARAM wParam  : [in] first message parameter
//        LPARAM lParam  : [in] second message parameter
//
// Return Values:
//        The return value is the result of the
//        message processing and depends on the message sent
//
/////////////////////////////////////////////////////////
BOOL CALLBACK DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM /* lParam */)
{
    BOOL bReturn = FALSE; // the value to return
    switch (uMsg)
    {
        case WM_INITDIALOG:
        {
            // Initialize the COM
            if (FAILED(CoInitialize(NULL)))
            {
                MessageBoxW(NULL, L"Unable to initialize the COM library",
                            gc_wszAppName, MB_OK | MB_ICONINFORMATION);
                PostMessage(hwnd, WM_CLOSE, 0, 0);
                break;
            }

            // Gather and show the information we're interested in

            // Check out if Microsoft Tablet PC Platform components of the
            // Microsoft Windows XP Professional Operating System are enabled
            int bTabletPC = GetSystemMetrics(SM_TABLETPC);
            SetDlgItemTextW(hwnd, IDC_TABLETPC,
                            bTabletPC ? L"Available" : L"Not available");

            // Get the version of the Text Services Framework components
            SInfo info;
            if (GetComponentInfo(CLSID_TF_ThreadMgr, info) == TRUE)
            {
                SetDlgItemTextW(hwnd, IDC_TSF, info.wchVersion);
            }

            // Find out the name and the version of the default handwriting recognizer
            // Create the enumerator for the installed recognizers
            IInkRecognizers* pIInkRecognizers = NULL;
            HRESULT hr = CoCreateInstance(CLSID_InkRecognizers,
                                          NULL,
                                          CLSCTX_INPROC_SERVER,
                                          IID_IInkRecognizers,
                                          (void **)&pIInkRecognizers);
            if (SUCCEEDED(hr))
            {
                IInkRecognizer* pIInkRecognizer = NULL;
                // The first parameter is the language id, passing 0 means that the language
                // id will be retrieved using the user default-locale identifier
                hr = pIInkRecognizers->GetDefaultRecognizer(0, &pIInkRecognizer);
                if (SUCCEEDED(hr))
                {
                    // Get the recognizer's friendly name
                    BSTR bstr;
                    if (SUCCEEDED(pIInkRecognizer->get_Name(&bstr)))
                    {
                        SetDlgItemTextW(hwnd, IDC_HWR_NAME, bstr);
                        SysFreeString(bstr);
                    }
                    else
                    {
                        SetDlgItemTextW(hwnd, IDC_HWR_NAME, L"Failed");
                    }
                    // Get the recognizer's vendor info
                    if (SUCCEEDED(pIInkRecognizer->get_Vendor(&bstr)))
                    {
                        SetDlgItemTextW(hwnd, IDC_HWR_VENDOR, bstr);
                        SysFreeString(bstr);
                    }
                    else
                    {
                        SetDlgItemTextW(hwnd, IDC_HWR_VENDOR, L"Failed");
                    }
                    // Release it
                    pIInkRecognizer->Release();
                    pIInkRecognizer = NULL;
                }
                // Release the collection object
                pIInkRecognizers->Release();
                pIInkRecognizers = NULL;
            }

            // Find out the name and the version of the default speech recognizer

            // Open key to find path of application
            HKEY hkeySpeech;
            if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, gc_wszSpeechKey, 0, KEY_READ,
                            &hkeySpeech) == ERROR_SUCCESS)
            {
                // Query value of key to get the name of the component
                WCHAR wchValue[265];
                ULONG cSize = sizeof(wchValue);
                if (RegQueryValueExW(hkeySpeech, L"DefaultDefaultTokenId", NULL, NULL,
                                    (BYTE*)wchValue, &cSize) == ERROR_SUCCESS)
                {
                    int ndx = lstrlenW(L"HKEY_LOCAL_MACHINE\\");
                    int len = lstrlenW(wchValue);
                    if (ndx < len
                        && RegOpenKeyExW(HKEY_LOCAL_MACHINE, &wchValue[ndx], 0,
                                         KEY_READ, &hkeySpeech) == ERROR_SUCCESS)
                    {
                        cSize = sizeof(wchValue);
                        if (RegQueryValueExW(hkeySpeech, NULL, NULL, NULL,
                                             (BYTE*)wchValue, &cSize) == ERROR_SUCCESS)
                        {
                            SetDlgItemTextW(hwnd, IDC_SPR_NAME, wchValue);
                        }
                    }
                }
            }

            // Find out which of the controls are installed and show their version info
            for (int i = 0, j = 0; i < NUM_CONTROLS; i++)
            {
                // Get the component info
                CLSID clsid;
                if (SUCCEEDED(CLSIDFromProgID(gc_wszProgId[i], &clsid))
                    && GetComponentInfo(clsid, info) == TRUE)
                {
                    SetDlgItemTextW(hwnd, gc_uiCtrlId[j][0], info.wchName);
                    SetDlgItemTextW(hwnd, gc_uiCtrlId[j][1], info.wchVersion);
                    j++;
                }
            }

            // Done with the COM
            CoUninitialize();

            break;
        }
        case WM_DESTROY:

            PostQuitMessage(0);
            break;

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDCLOSE:   // User clicked on the "Close" button in the dialog.
                case IDCANCEL:  // User clicked the close button ([X]) in the caption
                                // or pressed Alt+F4.
                    DestroyWindow(hwnd);
                    bReturn = TRUE;
                    break;
            }
            break;
    }

    return bReturn;
}

/////////////////////////////////////////////////////////
//
// WinMain
//
//      The WinMain function is called by the system as the
//      initial entry point for a Win32-based application.
//      It contains typical boilerplate code to create and
//      show the main window, and pump messages.
//
// Parameters:
//        HINSTANCE hInstance,      : [in] handle to current instance
//        HINSTANCE hPrevInstance,  : [in] handle to previous instance
//        LPSTR lpCmdLine,          : [in] command line
//        int nCmdShow              : [in] show state
//
// Return Values:
//        0 : The function terminated before entering the message loop.
//        non zero: Value of the wParam when receiving the WM_QUIT message
//
/////////////////////////////////////////////////////////
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
    // unused parameters
    hPrevInstance, lpCmdLine, nCmdShow;     // silence the compiler warnings

    int iRet = 0;

    // Create the application window
    HWND hwndDlg = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_DIALOG), NULL, DlgProc);
    if (NULL != hwndDlg)
    {

        ShowWindow(hwndDlg, SW_SHOW);

        // Start the message loop
        MSG msg;
        while (GetMessage(&msg, NULL, 0, 0) > 0)
        {
            if (!IsDialogMessage(hwndDlg, &msg))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }

        iRet = (int)msg.wParam;
    }
    else
    {
        MessageBoxW(NULL, L"Unable to create the dialog box",
                    gc_wszAppName, MB_OK | MB_ICONINFORMATION);
    }

    return iRet;
}


