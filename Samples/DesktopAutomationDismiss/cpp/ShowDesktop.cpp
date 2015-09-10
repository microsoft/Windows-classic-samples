// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

//**********************************************************************
//
// IMPORTANT NOTE!
// For this program to have the privileges to dismiss the desktop, it 
// must be signed and run from Program Files or Windows directory.
//
//**********************************************************************

// Includes
#include <windows.h>
#include <shobjidl.h>
#include <objbase.h>
#include <sal.h>

#include <stdarg.h>
#include <wchar.h>

// Defines
#define MAX_RETRIES 5
#define VK_D        0x44

// Static variables
HANDLE g_hFile = nullptr;
BOOL g_fOutputToFile = FALSE;

// Forward declarations and typedefs
typedef struct _ENUMDISPLAYDATA {
    BOOL fAppVisible;
    IAppVisibility *pAppVisible;
} ENUMDISPLAYDATA;

void ShowDesktop();
BOOL CALLBACK MonitorEnumProc (_In_ HMONITOR hMonitor, _In_ HDC, _In_ LPRECT, _In_ LPARAM dwData);
BOOL IsAppVisible();
BOOL WaitForDesktop();
BOOL HasUIAccess();
BOOL OutputString(_In_z_ _Null_terminated_ WCHAR *pszFormatString, ...);

//**********************************************************************
//
// Sends Win + D to toggle to the desktop
//
//**********************************************************************
void ShowDesktop()
{
    OutputString(L"Sending 'Win-D'\r\n");
    INPUT inputs[4];
    ZeroMemory(inputs, sizeof(inputs));

    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = VK_LWIN;
    
    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = VK_D;

    inputs[2].type = INPUT_KEYBOARD;
    inputs[2].ki.wVk = VK_D;
    inputs[2].ki.dwFlags = KEYEVENTF_KEYUP;

    inputs[3].type = INPUT_KEYBOARD;
    inputs[3].ki.wVk = VK_LWIN;
    inputs[3].ki.dwFlags = KEYEVENTF_KEYUP;

    UINT uSent = SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT));
    if (uSent != ARRAYSIZE(inputs))
    {
        OutputString(L"SendInput failed: 0x%x\n", HRESULT_FROM_WIN32(GetLastError()));
    }
}

//**********************************************************************
//
// Callback for EnumWindows in IsAppVisible
//
//**********************************************************************

BOOL CALLBACK MonitorEnumProc (_In_ HMONITOR hMonitor, _In_ HDC, _In_ LPRECT, _In_ LPARAM dwData)
{
    ENUMDISPLAYDATA *pData =  reinterpret_cast<ENUMDISPLAYDATA *>(dwData);
    MONITOR_APP_VISIBILITY monitorAppVisibility = MAV_UNKNOWN;

    HRESULT hr = pData->pAppVisible->GetAppVisibilityOnMonitor(hMonitor, &monitorAppVisibility);  
    if (SUCCEEDED(hr))  
    {  
        OutputString(L"\tMonitor app visibility:\t\t");
        switch (monitorAppVisibility)
        {
        case MAV_UNKNOWN:
            OutputString(L"UNKNOWN\r\n");
            break;
        case MAV_NO_APP_VISIBLE:
            OutputString(L"NO APP VISIBLE\r\n");
            break;
        case MAV_APP_VISIBLE:
            OutputString(L"APP VISIBLE\r\n");
            pData->fAppVisible = TRUE;
            break;
        default:
            OutputString(L"UNDEFINED\r\n");
            break;
        }
    }  

    return TRUE;
}

//**********************************************************************
//
// Checks to see if any apps or the Start menu is visible
//
//**********************************************************************
BOOL IsAppVisible()  
{
    OutputString(L"Checking for apps\r\n");
    BOOL fAppVisible = FALSE;  
    
    ENUMDISPLAYDATA data = {0};
    
    HRESULT hr = CoCreateInstance(CLSID_AppVisibility, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&data.pAppVisible));
    if (SUCCEEDED(hr))  
    {
        hr = data.pAppVisible->IsLauncherVisible(&fAppVisible);
        if (SUCCEEDED(hr))
        {
            OutputString(L"\tIsLauncherVisible:\t%s\r\n", fAppVisible ? L"TRUE" : L"FALSE");

            if (!fAppVisible)
            {
                if (0 == EnumDisplayMonitors(nullptr, nullptr, MonitorEnumProc, reinterpret_cast<LPARAM>(&data)))
                {
                    OutputString(L"EnumDisplayMonitors failed.\r\n");
                }
                else
                {
                    fAppVisible = data.fAppVisible;
                }
            }
        }

        data.pAppVisible->Release();
    }

    if (FAILED(hr))
    {
        OutputString(L"IsAppVisible failed: 0x%x\r\n", hr);
    }

    OutputString(L"\tApps:\t\t%s\r\n", fAppVisible ? L"FOUND" : L"NOT FOUND");
    return fAppVisible;
}

//*********************************************************************
//
// Waits for apps or Start menu to be dismissed
// Returns true if apps and Start menu are no longer visible
//
//*********************************************************************
BOOL WaitForDesktop()
{
    BOOL fAppVisible = IsAppVisible();

    for (int i = 0; i < MAX_RETRIES && fAppVisible; i++)
    {
        Sleep(1000);
        fAppVisible = IsAppVisible();
    }
    
    return !fAppVisible;
}

//***********************************************************************
//
// Checks UIAccess
//
//***********************************************************************
BOOL HasUIAccess()
{
    BOOL fUIAccess = FALSE;
    HANDLE hToken = nullptr;

    // Get process token in order to check for UIAccess
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
    {
        DWORD dwAccessInfo = 0;
        DWORD dwInfoSize = 0;

        // Verify that the process has UIAccess
        if (GetTokenInformation(hToken, TokenUIAccess, reinterpret_cast<LPVOID>(&dwAccessInfo), sizeof(dwAccessInfo), &dwInfoSize))
        {
            fUIAccess = (dwAccessInfo > 0);
        }
        else
        {
            OutputString(L"GetTokenInformation error:%d\n", GetLastError());
        }

        CloseHandle(hToken);
        hToken  = nullptr;
    }
    else
    {
        OutputString(L"Failed to open the token.\n");
    }

    return fUIAccess;
}

//***********************************************************************
//
// Outputs a string, either to a log file or to the console
//
//***********************************************************************
BOOL OutputString(_In_z_ _Null_terminated_ WCHAR *pszFormatString, ...)
{
    BOOL fRetValue = TRUE;

    // Make the string for the variadic function
    va_list args;
    WCHAR szOutputBuffer[MAX_PATH] = {0};
    va_start(args, pszFormatString);

    int nCharWritten = _vsnwprintf_s(szOutputBuffer, ARRAYSIZE(szOutputBuffer), pszFormatString, args);
    if (nCharWritten > 0)
    {
        // Output
        if (g_fOutputToFile)
        {
            CHAR szMultibyteOut[MAX_PATH];
            int nBytesWritten = WideCharToMultiByte(CP_UTF8, 0, szOutputBuffer, nCharWritten, szMultibyteOut, sizeof(szMultibyteOut), nullptr, nullptr);
            if (nBytesWritten == 0)
            {
                fRetValue = FALSE;
            }
            else
            {
                fRetValue = WriteFile(g_hFile, reinterpret_cast<LPCVOID>(szMultibyteOut), nBytesWritten, nullptr, nullptr);
            }
        }
        else
        {
            wprintf(szOutputBuffer);
        }
    }
    else
    {
        fRetValue = FALSE;
    }
    
    return fRetValue;
}

//***********************************************************************
//
// ShowDesktop entry point
//
//***********************************************************************
int __cdecl wmain(_In_ int argc, _In_reads_(argc) WCHAR *argv[], _In_reads_(argc) WCHAR *[])
{
    int nResult = ERROR_SUCCESS;

    // Check input args for flags
    BOOL fOverwrite = FALSE;
    BOOL fIgnoreUIAccess = FALSE;
    WCHAR *pszfileName = nullptr;
    for (int i = 1; i < argc; i++)
    {
        // Flag to give help
        if (_wcsicmp(argv[i], L"-?") == 0)
        {
            wprintf(L"Command line options:\n"
                    L"-o <file path>: Writes output to a the specified log file instead of the console\n"
                    L"-f: Forces the file specified in -o flag to be overwritten if the file exists\n"
                    L"-i: Ignores the return value of the check for UIAccess\n");
        }

        // Flag for force overwrite
        if (_wcsicmp(argv[i], L"-f") == 0)
        {
            fOverwrite = TRUE;
        }

        // Flag for output file path
        if (_wcsicmp(argv[i - 1], L"-o") == 0)
        {
            pszfileName = argv[i];
            g_fOutputToFile = TRUE;
        }
        
        // Flag to ignore the check for UIAccess
        if (_wcsicmp(argv[i], L"-i") == 0)
        {
            fIgnoreUIAccess = TRUE;
        }
    }

    // Open the specified file for write
    if (g_fOutputToFile && pszfileName != nullptr)
    {
        DWORD dwCreationDisposition = fOverwrite ? CREATE_ALWAYS: CREATE_NEW;
        g_hFile = CreateFile(pszfileName, GENERIC_WRITE, FILE_SHARE_READ, nullptr, dwCreationDisposition, FILE_ATTRIBUTE_NORMAL, nullptr);
        
        if (g_hFile == INVALID_HANDLE_VALUE)
        {
            nResult = GetLastError();
            wprintf(L"Cannot write to the specified file, error: 0x%x\r\n", nResult);
        }
    }

    if (nResult == ERROR_SUCCESS)
    {
        HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
        if (SUCCEEDED(hr))
        {
            if (fIgnoreUIAccess || HasUIAccess())
            {
                if (fIgnoreUIAccess)
                {
                    OutputString(L"Ignoring the check for UIAccess.\r\n");
                }
                else
                {
                    OutputString(L"UI automation access is allowed.\r\n");
                }

                if (IsAppVisible())
                {
                    ShowDesktop();

                    // Wait for the dismiss animation to complete and for the desktop to be visible.
                    Sleep(1000);
                    if (WaitForDesktop())
                    {
                        nResult = ERROR_SUCCESS;
                    }
                    else
                    {
                        nResult = ERROR_CAN_NOT_COMPLETE;
                    }
                }
                else
                {
                    // System is already in Desktop--no need to send Win + D
                    nResult = ERROR_SUCCESS;
                }
            }
            else
            {
                OutputString(L"UI automation access is NOT allowed.\r\n");
                nResult = ERROR_ACCESS_DENIED;
            }

            CoUninitialize();
        }
        else
        {
            OutputString(L"CoInitializeEx failed: 0x%x\r\n", hr);
        }

        if (g_hFile != nullptr)
        {
            CloseHandle(g_hFile);
        }
    }

    return nResult;
}