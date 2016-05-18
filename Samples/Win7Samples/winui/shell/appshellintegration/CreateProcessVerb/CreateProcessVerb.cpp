// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
// This demonstrates how implement a shell verb using the CreateProcess method
// CreateProcess based verbs depend on running a .exe and passing it a command line.
// This method is not as powerful as the DropTarget and DelegateExecute methods
// but it does achieve the desirable out of process behavior.

#include <windows.h>
#include <shlwapi.h>
#include <shlobj.h>
#include "RegisterExtension.h"
#include <strsafe.h>

HRESULT GetShellItemFromCommandLine(REFIID riid, void **ppv)
{
    *ppv = NULL;

    HRESULT hr = E_FAIL;
    int cArgs;
    PWSTR *ppszCmd = CommandLineToArgvW(GetCommandLineW(), &cArgs);
    if (ppszCmd && cArgs > 1)
    {
        WCHAR szSpec[MAX_PATH];
        szSpec[0] = 0;

        // skip all parameters that begin with "-" or "/" to try to find the
        // file name. this enables parameters to be present on the cmd line
        // and not get in the way of this function
        for (int i = 1; (szSpec[0] == 0) && (i < cArgs); i++)
        {
            if ((*ppszCmd[i] != L'-') && (*ppszCmd[i] != L'/'))
            {
                StringCchCopyW(szSpec, ARRAYSIZE(szSpec), ppszCmd[i]);
                PathUnquoteSpacesW(szSpec);
            }
        }

        hr = szSpec[0] ? S_OK : E_FAIL; // protect against empty
        if (SUCCEEDED(hr))
        {
            hr = SHCreateItemFromParsingName(szSpec, NULL, riid, ppv);
            if (FAILED(hr))
            {
                WCHAR szFolder[MAX_PATH];
                GetCurrentDirectoryW(ARRAYSIZE(szFolder), szFolder);
                hr = PathAppendW(szFolder, szSpec) ? S_OK : E_FAIL;
                if (SUCCEEDED(hr))
                {
                    hr = SHCreateItemFromParsingName(szFolder, NULL, riid, ppv);
                }
            }
        }
    }
    return hr;
}

int APIENTRY wWinMain(HINSTANCE, HINSTANCE, PWSTR pszCmdLine, int)
{
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (SUCCEEDED(hr))
    {
        if (StrStrI(pszCmdLine, L"-Verb"))
        {
            IShellItem2 *psi;
            hr = GetShellItemFromCommandLine(IID_PPV_ARGS(&psi));
            if (SUCCEEDED(hr))
            {
                PWSTR pszName;
                hr = psi->GetDisplayName(SIGDN_PARENTRELATIVEPARSING, &pszName);
                if (SUCCEEDED(hr))
                {
                    WCHAR szMsg[128];
                    StringCchPrintf(szMsg, ARRAYSIZE(szMsg), L"Item passed via command line is named %s", pszName);

                    MessageBox(NULL, szMsg, L"Create Process Sample Verb", MB_OK);

                    CoTaskMemFree(pszName);
                }

                psi->Release();
            }
        }
        else
        {
            CRegisterExtension re(CLSID_NULL);

            WCHAR const c_szProgID[] = L"txtfile";
            WCHAR const c_szVerb[] = L"CreateProcessVerb";
            WCHAR const c_szVerbName[] = L"Create Process Verb";

            WCHAR szModule[MAX_PATH];
            GetModuleFileName(NULL, szModule, ARRAYSIZE(szModule));

            // register this verb on .txt files ProgID
            WCHAR szCmdLine[MAX_PATH + 10];
            StringCchPrintf(szCmdLine, ARRAYSIZE(szCmdLine), L"%s -Verb \"%%1\"", szModule);
            hr = re.RegisterCreateProcessVerb(c_szProgID, c_szVerb, szCmdLine, c_szVerbName);
            if (SUCCEEDED(hr))
            {
                hr = re.RegisterVerbAttribute(c_szProgID, c_szVerb, L"NeverDefault");
                if (SUCCEEDED(hr))
                {
                    MessageBox(NULL,
                        L"Installed Create Process Verb Sample for .txt files\n\n"
                        L"right click on a .txt file and choose 'Create Process Verb Sample' to see this in action"
                        , PathFindFileName(szModule), MB_OK);
                }
            }
        }
        CoUninitialize();
    }

    return 0;
}
