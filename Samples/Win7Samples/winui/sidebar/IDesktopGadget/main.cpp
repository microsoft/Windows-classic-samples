// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include <windows.h>
#include <shlwapi.h>
#include <shobjidl.h>
#include <stdio.h>

#pragma comment(lib, "shlwapi.lib")

BOOL CheckValidInstallPath(PCWSTR pszGadgetPath)
{
    BOOL fIsValid = FALSE;
    PCWSTR const rgszPaths[] = 
    {
        L"%ProgramFiles%\\Windows Sidebar\\Gadgets",
        L"%ProgramFiles%\\Windows Sidebar\\Shared Gadgets",
        L"%LocalAppData%\\Microsoft\\Windows Sidebar\\Gadgets"
    };

    for (int iPath = 0; iPath < ARRAYSIZE(rgszPaths); ++iPath)
    {
        WCHAR szPath[MAX_PATH];

        if (ExpandEnvironmentStringsW(rgszPaths[iPath], szPath, ARRAYSIZE(szPath)))
        {
            if (PathIsPrefixW(szPath, pszGadgetPath))
            {
                fIsValid = TRUE;
                break;
            }
        }
    }

    return fIsValid;
}

HRESULT AddGadget(IDesktopGadget *pDG, PCWSTR pszGadgetPath)
{
    // Note that RunGadget will return S_OK even if the gadget fails to load
    // due to a problem with the gadget itself.  The return value from
    // RunGadget just indicates whether the request succeeded or not.
    HRESULT hr = pDG->RunGadget(pszGadgetPath);
    if (FAILED(hr))
    {
        switch (hr)
        {
        case SCHED_E_ALREADY_RUNNING:
            wprintf(L"Gadget '%s' is already running!\n", pszGadgetPath);
            hr = S_FALSE;
            break;

        case __HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND):
            wprintf(L"Gadget '%s' not found!\n", pszGadgetPath);
            break;

        default:
            wprintf(L"RunGadget '%s' failed, error=%X\n", pszGadgetPath, hr);
            break;
        }
    }

    return hr;
}

extern "C" int __cdecl wmain(int argc, wchar_t *argv[])
{
    int iExitCode = -1;

    wprintf(L"IDesktopGadget Sample\n"
            L"================================\n"
            L"Demonstrates how to use the IDesktopGadget interface to programmatically add\n"
            L"Windows desktop gadgets to the current user's desktop.\n\n");

    if (argc != 2)
    {
        wprintf(L"ERROR: You need to specify a gadget name!");
        return -1;
    }

    if (CheckValidInstallPath(argv[1]))
    {
        HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
        if (SUCCEEDED(hr))
        {
            IDesktopGadget *pDG;

            hr = CoCreateInstance(CLSID_DesktopGadget, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pDG));
            if (SUCCEEDED(hr))
            {
                if (SUCCEEDED(AddGadget(pDG, argv[1])))
                {
                    iExitCode = 0;
                }

                pDG->Release();
            }
            else
            {
                wprintf(L"CoCreateInstance(CLSID_DesktopGadget) failed, error=%X\n", hr);
            }

            CoUninitialize();
        }
    }
    else
    {
        wprintf(L"Gadgets cannot be programmatically installed from the specified location.");
    }

    return iExitCode;
}