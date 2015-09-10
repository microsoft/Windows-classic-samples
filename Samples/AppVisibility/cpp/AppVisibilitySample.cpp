// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include <windows.h>
#include <objbase.h>
#include <shobjidl.h>
#include <stdio.h>
#include <wrl\client.h>
#include <wrl\implements.h>

// Simple helper function to turn a MONITOR_APP_VISIBILITY enumeration into a string
PCWSTR const _GetMonitorAppVisibilityString(MONITOR_APP_VISIBILITY monitorAppVisibility)
{
    PCWSTR pszAppVisibilityString = nullptr;
    switch (monitorAppVisibility)
    {
        case MAV_NO_APP_VISIBLE:
            pszAppVisibilityString = L"no apps visible";
            break;

        case MAV_APP_VISIBLE:
            pszAppVisibilityString = L"a visible app";
            break;

        case MAV_UNKNOWN:
        __fallthrough;
        default:
            pszAppVisibilityString = L"unknown";
            break;
    }
    return pszAppVisibilityString;
}

// This class will implement the IAppVisibilityEvents interface and will receive notifications
// from the AppVisibility COM object.
class CAppVisibilityNotificationSubscriber :
    public Microsoft::WRL::RuntimeClass<
        Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::RuntimeClassType::ClassicCom>,
        IAppVisibilityEvents>
{
public:
    CAppVisibilityNotificationSubscriber();

    // AppVisibilityOnMonitorChanged will be called when applications appear or disappear on a monitor
    IFACEMETHODIMP AppVisibilityOnMonitorChanged(_In_ HMONITOR hMonitor,
                                           MONITOR_APP_VISIBILITY previousAppVisibility,
                                           MONITOR_APP_VISIBILITY currentAppVisibility);

    // LauncherVisibilityChange will be called whenever the Start menu becomes visible or hidden
    IFACEMETHODIMP LauncherVisibilityChange(BOOL currentVisibleState);

private:
    ~CAppVisibilityNotificationSubscriber();

    // This variable will be used to trigger this program's message loop
    // to exit. The variable will be incremented when the launcher becomes visible.
    // When the launcher becomes visible five times, the program will exit.
    unsigned int _cLauncherChanges;

};

CAppVisibilityNotificationSubscriber::CAppVisibilityNotificationSubscriber() : _cLauncherChanges(0)
{
}

CAppVisibilityNotificationSubscriber::~CAppVisibilityNotificationSubscriber()
{
}

// Implementation of IAppVisibilityEvents
IFACEMETHODIMP CAppVisibilityNotificationSubscriber::AppVisibilityOnMonitorChanged(_In_ HMONITOR hMonitor,
                                                                             MONITOR_APP_VISIBILITY previousAppVisibility,
                                                                             MONITOR_APP_VISIBILITY currentAppVisibility)
{
    wprintf_s(L"Monitor %p previously had %s and now has %s\r\n",
                hMonitor,
                _GetMonitorAppVisibilityString(previousAppVisibility),
                _GetMonitorAppVisibilityString(currentAppVisibility));
    return S_OK;
}

IFACEMETHODIMP CAppVisibilityNotificationSubscriber::LauncherVisibilityChange(BOOL currentVisibleState)
{
    wprintf_s(L"The Start menu is now %s\r\n", currentVisibleState ? L"visible" : L"not visible");
    if (currentVisibleState)
    {
        _cLauncherChanges++;
        if (_cLauncherChanges == 5)
        {
            PostQuitMessage(0);
        }
    }
    return S_OK;
}

BOOL CALLBACK DisplayMonitorEnumProc(_In_ HMONITOR hMonitor,
                                         _In_ HDC /*hdcMonitor*/,
                                         _In_ LPRECT /*lprcMonitor*/,
                                         _In_ LPARAM dwData)
{
    IAppVisibility *pAppVisibility = reinterpret_cast<IAppVisibility *>(dwData);

    MONITOR_APP_VISIBILITY monitorAppVisibility;
    HRESULT hr = pAppVisibility->GetAppVisibilityOnMonitor(hMonitor, &monitorAppVisibility);
    if (SUCCEEDED(hr))
    {
        wprintf_s(L"\tMonitor %p has %s\r\n", hMonitor, _GetMonitorAppVisibilityString(monitorAppVisibility));
    }
    return TRUE;
}

int __cdecl wmain()
{
    wprintf_s(L"Toggle Start menu visibility 5 times to exit\r\n");

    // Initialization of COM is required to use the AppVisibility (CLSID_AppVisibility) object
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (SUCCEEDED(hr))
    {
        // Create the App Visibility component
        Microsoft::WRL::ComPtr<IAppVisibility> spAppVisibility;
        hr = CoCreateInstance(CLSID_AppVisibility, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&spAppVisibility));
        if (SUCCEEDED(hr))
        {
            // Enumerate the current display devices and display app visibility status
            wprintf_s(L"Current app visibility status is:\r\n");
            EnumDisplayMonitors(nullptr, nullptr, DisplayMonitorEnumProc, reinterpret_cast<LPARAM>(spAppVisibility.Get()));
            wprintf_s(L"\r\n\r\n");

            // Display the current launcher visibility
            BOOL launcherVisibility;
            if (SUCCEEDED(spAppVisibility->IsLauncherVisible(&launcherVisibility)))
            {
                wprintf_s(L"The Start menu is currently %s\r\n", launcherVisibility ? L"visible" : L"not visible");
            }

            // Create an object that implements IAppVisibilityEvents that will receive
            // callbacks when either app visibility or Start menu visibility changes.
            Microsoft::WRL::ComPtr<IAppVisibilityEvents> spSubscriber = Microsoft::WRL::Details::Make<CAppVisibilityNotificationSubscriber>();

            // Advise to receive change notifications from the AppVisibility object
            // NOTE: There must be a reference held on the AppVisibility object in order to continue
            // NOTE: receiving notifications on the implementation of the IAppVisibilityEvents object
            DWORD dwCookie;
            hr = spAppVisibility->Advise(spSubscriber.Get(), &dwCookie);
            if (SUCCEEDED(hr))
            {
                // Since the visibility notifications are delivered via COM, a message loop must
                // be employed in order to receive notifications
                MSG msg;
                while (GetMessage(&msg, nullptr, 0, 0))
                {
                    DispatchMessage(&msg);
                }

                // Unadvise from the AppVisibility component to stop receiving notifications
                spAppVisibility->Unadvise(dwCookie);
            }
        }
        CoUninitialize();
    }
    return 0;
}
