//////////////////////////////////////////////////////////////////////////////
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//////////////////////////////////////////////////////////////////////////////

#include <windows.h>
#include <shobjidl.h> //For IShellItemImageFactory
#include <stdio.h>
#include "resource.h"

#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

int LookUp(PCWSTR pwszArg)
{
    int nSize = 0;

    // The possbile sizes for the image that is requested
    struct
    {
        PCWSTR pwszSize;
        int nSize;
    } const sizeTable[] = { {L"small", 16}, {L"medium", 48}, {L"large", 96}, {L"extralarge", 256} };

    for (int i = 0; i < ARRAYSIZE(sizeTable); i++)
    {
        if (CSTR_EQUAL == CompareStringOrdinal(pwszArg, -1, sizeTable[i].pwszSize, -1, TRUE))
        {
            nSize = sizeTable[i].nSize;
            break;
        }
    }

    return nSize;
}

void DisplayUsage()
{
    wprintf(L"Usage:\n");
    wprintf(L"IShellItemImageFactory.exe <size> <Absolute Path to file>\n");
    wprintf(L"size - small, medium, large, extralarge\n");
    wprintf(L"e.g. ImageFactorySample.exe medium c:\\HelloWorld.jpg \n");
}

INT_PTR CALLBACK  DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    INT_PTR nReturn = FALSE;

    switch (message)
    {
    case WM_INITDIALOG:
        SendDlgItemMessage(hDlg, IDC_STATIC1, STM_SETIMAGE , IMAGE_BITMAP, (LPARAM)lParam);
        nReturn = TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            nReturn = TRUE;
        }
        break;
    }
    return nReturn;
}

int wmain(int argc, wchar_t *argv[])
{
    if (argc != 3)
    {
        DisplayUsage();
    }
    else
    {
        int nSize = LookUp(argv[1]);
        if (!nSize)
        {
            DisplayUsage();
        }
        else
        {
            PCWSTR pwszError = NULL;

            HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
            if (SUCCEEDED(hr))
            {
                // Getting the IShellItemImageFactory interface pointer for the file.
                IShellItemImageFactory *pImageFactory;
                hr = SHCreateItemFromParsingName(argv[2], NULL, IID_PPV_ARGS(&pImageFactory));
                if (SUCCEEDED(hr))
                {
                    SIZE size = { nSize, nSize };

                    //sz - Size of the image, SIIGBF_BIGGERSIZEOK - GetImage will stretch down the bitmap (preserving aspect ratio)
                    HBITMAP hbmp;
                    hr = pImageFactory->GetImage(size, SIIGBF_BIGGERSIZEOK, &hbmp);
                    if (SUCCEEDED(hr))
                    {
                        DialogBoxParamW(NULL, MAKEINTRESOURCEW(IDD_DIALOG1), NULL, DialogProc, (LPARAM)hbmp);
                        DeleteObject(hbmp);
                    }
                    else
                    {
                        pwszError = L"IShellItemImageFactory::GetImage failed with error code %x";
                    }

                    pImageFactory->Release();
                }
                else
                {
                    pwszError = L"SHCreateItemFromParsingName failed with error %x";
                }

                CoUninitialize();
            }
            else
            {
                pwszError = L"CoInitializeEx failed with error code %x";
            }

            if (FAILED(hr))
            {
                wprintf(pwszError, hr);
            }
        }
    }

    return 0;
}