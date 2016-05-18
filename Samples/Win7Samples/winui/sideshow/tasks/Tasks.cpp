// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "stdafx.h"
#include "tasks_h.h"
#include "tasks_i.c"
#include "TasksClient.h"

// use Shell common controls v6.0
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

using namespace std;

//
// The DLL instance module
//
class CTasksModule : public CAtlDllModuleT< CTasksModule >
{
public :
    DECLARE_REGISTRY_APPID_RESOURCEID(IDR_TASKS, "{9294845F-C3E7-4602-ABA5-EBFEF33B5B6A}")
};

CTasksModule _AtlModule;

//
// Helper to get the lcation of the configuration file:
// The tasks.ini file is expected to be in the same directory where the dll resides.
//
CString GetConfigFile()
{
    CString strPath;

    HMODULE hInstance;
    WCHAR wszPath[MAX_PATH] = {0};

    if (GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCTSTR)(&_AtlModule), &hInstance))
    {
        if (GetModuleFileName(hInstance, wszPath, MAX_PATH))
        {
            if (PathRemoveFileSpec(wszPath))
            {
                if (PathAppend(wszPath, L"Tasks.ini"))
                {
                    strPath = wszPath;
                }
            }
        }
    }

    return strPath;
}

//
// Helper to get the lcation of the task file:
// The tasklist.xml file is expected to be in the same directory where the dll resides.
//
CString GetTaskFile()
{
    CString strPath;

    HMODULE hInstance;
    WCHAR wszPath[MAX_PATH] = {0};

    if (GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCTSTR)(&_AtlModule), &hInstance))
    {
        if (GetModuleFileName(hInstance, wszPath, MAX_PATH))
        {
            if (PathRemoveFileSpec(wszPath))
            {
                if (PathAppend(wszPath, L"Tasklist.xml"))
                {
                    strPath = wszPath;
                }
            }
        }
    }

    return strPath;
}


void CreateClient()
{
    HANDLE hInstanceMutex = NULL;

    if ((NULL == (hInstanceMutex = ::CreateMutex(NULL, TRUE, L"Tasks"))) &&
        (ERROR_ALREADY_EXISTS == GetLastError()))
    {
        return;
    }

    CTasksClient client;

    //
    // Register this client application with the Windows SideShow
    // platform
    //
    client.Register();

    //
    // Add content to the display
    //
    client.AddContent();

    ::MessageBox(NULL, L"WindowsSideShowTasks is running, press the OK button to exit the gadget.", L"Tasks", MB_OK);

    //
    // Remove all of the content from the display
    // so it's no longer available once this application
    // closes
    //
    client.RemoveAllContent();

    //
    // Unregister this client application from the platform
    //
    client.Unregister();

    if (NULL != hInstanceMutex)
    {
        ::CloseHandle(hInstanceMutex);
    }
}

void CALLBACK StartTasks(HWND /*hwnd*/, 
                         HINSTANCE /*hinst*/, 
                         LPSTR /*lpszCmdLine*/,
                         int /*nCmdShow*/)
{
    //
    // Initialize COM; we use the Apartment threading model
    // because our implemented COM objects are not thread-safe
    //
    ::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

    //
    // Run the client application
    //
    CreateClient();

    //
    // Finally, uninitialize COM
    //
    ::CoUninitialize();
}

// DLL Entry Point
extern "C" BOOL WINAPI DllMain(HINSTANCE /*hInstance*/, DWORD dwReason, LPVOID lpReserved)
{
    return _AtlModule.DllMain(dwReason, lpReserved); 
}

// Used to determine whether the DLL can be unloaded by OLE
STDAPI DllCanUnloadNow(void)
{
    return _AtlModule.DllCanUnloadNow();
}

// Returns a class factory to create an object of the requested type
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    return _AtlModule.DllGetClassObject(rclsid, riid, ppv);
}

// DllRegisterServer - Adds entries to the system registry
STDAPI DllRegisterServer(void)
{
    HRESULT hr = _AtlModule.DllRegisterServer(FALSE);
    return hr;
}

// DllUnregisterServer - Removes entries from the system registry
STDAPI DllUnregisterServer(void)
{
    HRESULT hr = _AtlModule.DllUnregisterServer(FALSE);
    return hr;
}
