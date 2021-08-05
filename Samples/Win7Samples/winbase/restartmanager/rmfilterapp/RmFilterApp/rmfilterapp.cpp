/*++

THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (C) Microsoft Corporation.  All rights reserved.

Module Name:

    rmfilterapp.cpp

Abstract:

    This module contains the skeleton code for a Windows native
    API console based Restart Manager conductor process sample.

Notes:
    This example requires Windows 7 or Windows Server 2008 R2.

    The following code snippet shows an example of a primary installer starting and using a Restart Manager session.

    This example shows how a primary installer can use Restart Manager to shutdown and restart a process.
    The example assumes that Calculator is already running before starting the Restart Manager session.
--*/

#include <windows.h>
#include <restartmanager.h>
#include <memory>
#include <vector>
#include <string>
#include <algorithm>

struct RestartManagerSessionHandle
{
    RestartManagerSessionHandle(RestartManagerSessionHandle&) = delete;
    RestartManagerSessionHandle() = default;

    ~RestartManagerSessionHandle()
    {
        if (m_handle != 0xFFFFFFFF)
        {
            RmEndSession(m_handle);
        }
    }
    DWORD get() { return m_handle; }
    DWORD* putHandle() { return &m_handle; }
    PWSTR putSessionKey() { return m_sessKey; }
private:
    DWORD m_handle = 0xFFFFFFFF;
    wchar_t m_sessKey[CCH_RM_SESSION_KEY + 1]{}; // Text-Encoded session key,defined in RestartManager.h
};

void ThrowIfFailed(DWORD win32Error)
{
    if (ERROR_SUCCESS != win32Error) throw std::system_error(win32Error, std::system_category());
}

int _cdecl wmain() try
{
    RestartManagerSessionHandle sessionHandle;
    ThrowIfFailed(RmStartSession(sessionHandle.putHandle(), 0, sessionHandle.putSessionKey()));

    // These are the processes we intend to restart
    PCWSTR fileToReplace[] =
    {
        LR"(C:\Windows\explorer.exe)",
        LR"(C:\Windows\notepad.exe)",
        LR"(C:\Windows\system32\notepad.exe)",
        LR"(C:\Windows\SysWow64\notepad.exe)",
    };

    // Tell restart manager what we want to replace
    ThrowIfFailed(RmRegisterResources(sessionHandle.get(), ARRAYSIZE(fileToReplace), fileToReplace,
        0, nullptr, 0, nullptr)); // No processes or services.

    RM_REBOOT_REASON rebootReasons = RmRebootReasonNone;
    std::vector<RM_PROCESS_INFO> restartableApps;

    // Need to loop since the list may change due to concurrency
    for (;;)
    {
        DWORD reasonsAsDword = 0;
        UINT allocationSizeNeeded = 0;
        auto restartableAppSize = static_cast<UINT>(restartableApps.size()); // careful, in-out value
        const auto errorCode = RmGetList(sessionHandle.get(), &allocationSizeNeeded, &restartableAppSize,
            restartableApps.data(), &reasonsAsDword);
        if (errorCode == ERROR_MORE_DATA)
        {
            restartableApps.resize(allocationSizeNeeded); // size the buffer
        }
        else if (ERROR_SUCCESS == errorCode)
        {
            rebootReasons = static_cast<decltype(rebootReasons)>(reasonsAsDword);
            break;
        }
        else
        {
            throw std::system_error(errorCode, std::system_category());
        }
    }

    for (const auto& app : restartableApps)
    {
        wprintf(L"App: %s, %d", app.strAppName, app.Process.dwProcessId);
    }

    if (RmRebootReasonNone != rebootReasons)
    {
        return 0;
    }

    // Now restartableApps contains the affected applications 
    // and services. The number of applications and services
    // returned is restartableAppCount. The result of RmGetList can 
    // be interpreted by the user to determine subsequent  
    // action (e.g. ask user's permission to shutdown).
    //
    // CALLER CODE GOES HERE...

    // Shut down all running instances of affected 
    // applications and services.
    ThrowIfFailed(RmShutdown(sessionHandle.get(), 0, nullptr));

    // An installer can now replace or update
    // the calc executable file.
    //
    // CALLER CODE GOES HERE...
    // Restart applications and services, after the 
    // files have been replaced or updated.
    //
    ThrowIfFailed(RmRestart(sessionHandle.get(), 0, nullptr));
    return 0;
}
catch (const std::exception& e)
{
    printf(e.what());
}
