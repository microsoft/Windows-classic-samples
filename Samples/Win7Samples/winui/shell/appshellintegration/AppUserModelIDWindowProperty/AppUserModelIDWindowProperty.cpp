// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#define NTDDI_VERSION NTDDI_WIN7  // Specifies that the minimum required platform is Windows 7.
#define WIN32_LEAN_AND_MEAN       // Exclude rarely-used stuff from Windows headers

#include <windows.h>
#include <shobjidl.h>
#include <propkey.h>
#include <propvarutil.h>
#include <shellapi.h>

#include "resource.h"

#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

WCHAR const c_szTitle[] = L"AppUserModelID Window Property Sample";
WCHAR const c_szWindowClass[] = L"APPUSERMODELIDWINDOWPROPERTY";

WCHAR const c_szAppUserModelID1[] = L"Microsoft.Samples.App1";
WCHAR const c_szAppUserModelID2[] = L"Microsoft.Samples.App2";
WCHAR const c_szAppUserModelID3[] = L"Microsoft.Samples.App3";

// Sets the specified AppUserModelID on the window, or removes the value if a negative index is provided
void SetAppID(HWND hWnd, PCWSTR pszAppID)
{
    // Obtain the window's property store.  This IPropertyStore implementation does not require
    // IPropertyStore::Commit to be called - values are updated on the window immediately.  Setting a
    // property on a window via this IPropertyStore allocates global storage space for the property value
    // that is not automatically cleaned up upon window destruction or process termination, thus all
    // properties set on a window should be removed in response to WM_DESTROY.
    IPropertyStore *pps;
    HRESULT hr = SHGetPropertyStoreForWindow(hWnd, IID_PPV_ARGS(&pps));
    if (SUCCEEDED(hr))
    {
        PROPVARIANT pv;
        if (pszAppID)
        {
            hr = InitPropVariantFromString(pszAppID, &pv);
        }
        else
        {
            // Sets the variant type as VT_EMPTY, which removes the property from the window, if present
            PropVariantInit(&pv);
        }
        if (SUCCEEDED(hr))
        {
            // Sets the PKEY_AppUserModel_ID property, which controls how windows are grouped into buttons
            // on the taskbar.  If the window needed other PKEY_AppUserModel_* properties to be set, they
            // should be set BEFORE setting PKEY_AppUserModel_ID, as the taskbar will only respond to
            // updates of PKEY_AppUserModel_ID.
            hr = pps->SetValue(PKEY_AppUserModel_ID, pv);
            PropVariantClear(&pv);
        }
        pps->Release();
    }
}

// All windows with the same AppUserModelID will group together. Changing a window's AppUserModelID will cause
// the taskbar to reassign it to a different group.
//
// NOTE:  All AppUserModelID values used here are for sample purposes only.  Applications should define their
// own values in the form of CompanyName.ProductName.SubProduct.VersionInformation.  See the "Application User
// Model IDs (AppIDs)" MSDN topic for more information.
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        // Set an initial AppUserModelID on the window, before the window is presented in the taskbar.  If
        // the value is set after this time, the window may visibly change groups in the taskbar.  This may
        // be desireable if, as this sample illustrates, the window can change identities throughout its
        // lifetime.  However, if the window's identity will not change, then the value should be set here,
        // or the window should be created hidden and shown only once the value has been set to prevent the
        // window's taskbar representation from jumping around quickly as the window is created and then
        // assigned an explicit AppUserModelID.
        //
        // All windows with the same AppUserModelID will group together. Changing a window's AppUserModelID
        // will cause the taskbar to reassign it to a different group.  Windows without an explicitly set
        // AppUserModelID will group together, but separate from any windows with an explicit value.
        SetAppID(hWnd, c_szAppUserModelID1);
        break;
    case WM_COMMAND:
        {
            int const wmId = LOWORD(wParam);
            // Parse the menu selections
            switch (wmId)
            {
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            case IDM_FILE_APPID1:
                // Set the first AppUserModelID on the window, replacing the previous value
                SetAppID(hWnd, c_szAppUserModelID1);
                break;
            case IDM_FILE_APPID2:
                // Set the second AppUserModelID on the window, replacing the previous value
                SetAppID(hWnd, c_szAppUserModelID2);
                break;
            case IDM_FILE_APPID3:
                // Set the third AppUserModelID on the window, replacing the previous value
                SetAppID(hWnd, c_szAppUserModelID3);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
            break;
        }
    case WM_DESTROY:
        // Remove any AppUserModelID value set on this window.  This must be done before the window
        // is completely destroyed, or the value will be leaked.
        SetAppID(hWnd, NULL);
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow)
{
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (SUCCEEDED(hr))
    {
        WNDCLASSEX wcex     = {sizeof(wcex)};
        wcex.style          = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc    = WndProc;
        wcex.hInstance      = hInstance;
        wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CUSTOMJUMPLISTSAMPLE));
        wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
        wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
        wcex.lpszMenuName   = MAKEINTRESOURCE(IDC_CUSTOMJUMPLISTSAMPLE);
        wcex.lpszClassName  = c_szWindowClass;

        RegisterClassEx(&wcex);

        HWND hWnd = CreateWindow(c_szWindowClass, c_szTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, 300, 200, NULL, NULL, hInstance, NULL);
        if (hWnd)
        {
            ShowWindow(hWnd, nCmdShow);

            MSG msg;
            while (GetMessage(&msg, NULL, 0, 0))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        CoUninitialize();
    }
    return 0;
}
