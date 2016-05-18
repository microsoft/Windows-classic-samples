/*----------------------------------------------------------------------------
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (C) 1999 - 2000.  Microsoft Corporation.  All rights reserved.


Commands.cpp

Description:
Implements the functions called when the user selects a valid menu choice
----------------------------------------------------------------------------*/

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <stdio.h>
#include <dskquota.h>
#include "Commands.h"

static void WaitForKeyPress();

/*-----------------------------------------------------------------------------
GetDefaultQuota(IDiskQuotaControl* lpDiskQuotaControl)
    Gets the state of quota tracking on the current volume

Parameters
    lpDiskQuotaControl - Pointer to an object that implements the
                         IDiskQuotaControl interface

Return Value
    TRUE - Success
    FALSE - Failure
-----------------------------------------------------------------------------*/
BOOL GetDefaultQuota(IDiskQuotaControl* lpDiskQuotaControl)
{
    HRESULT hr;
    DWORD   dwState;

    // Get the default quota state
    hr = lpDiskQuotaControl->GetQuotaState(&dwState);

    if (FAILED(hr))
    {
        PrintError(hr);
        return FALSE;
    }

    wprintf(L"\n\nDefault Disk Quota State:\n");

    // Switch on the state of disk quota tracking
    switch(dwState & DISKQUOTA_STATE_MASK)
    {
    case DISKQUOTA_STATE_DISABLED:
        wprintf(L"Quota's are not enabled on the volume.\n");
        break;
    case DISKQUOTA_STATE_TRACK:
        wprintf(L"Quotas are enabled but the limit value is not being ");
        wprintf(L"enforced.\nUsers may exceed their quota limit.\n");
        break;
    case DISKQUOTA_STATE_ENFORCE:
        wprintf(L"Quotas are enabled and the limit value is enforced.");
        wprintf(L"\nUsers cannot exceed their quota limit.\n");
        break;
    default:
        wprintf(L"Unknown.\n");
        break;
    }

    wprintf(L"\nDefault Disk Quota File State:\n");

    switch(dwState & DISKQUOTA_FILESTATE_MASK)
    {
    case DISKQUOTA_FILESTATE_INCOMPLETE:
        wprintf(L"The volume's quota information is out of date.\n");
        wprintf(L"Quotas are probably disabled.\n");
        break;
    case DISKQUOTA_FILESTATE_REBUILDING:
        wprintf(L"The volume is rebuilding its quota information.\n");
        break;
    default:
        wprintf(L"Unknown.\n");
        break;
    }

    WaitForKeyPress();

    return TRUE;
}

/*-----------------------------------------------------------------------------
GetQuotaLogFlags(IDiskQuotaControl* lpDiskQuotaControl)
    Gets the state of the quota log flags

Parameters
    lpDiskQuotaControl - Pointer to an object that implements the
                         IDiskQuotaControl interface

Return Value
    TRUE - Success
    FALSE - Failure
-----------------------------------------------------------------------------*/
BOOL GetQuotaLogFlags(IDiskQuotaControl* lpDiskQuotaControl)
{
    HRESULT hr;
    DWORD   dwQuotaLogFlags;

    // Get the state log the quota log flags
    hr = lpDiskQuotaControl->GetQuotaLogFlags(&dwQuotaLogFlags);

    if (FAILED(hr))
    {
        PrintError(hr);
        return FALSE;
    }

    wprintf(L"\n\nDisk Quota Log State:\n");

    // Check if the system creates a log entry when a threshold limit
    // is exceeded
    if (DISKQUOTA_IS_LOGGED_USER_THRESHOLD(dwQuotaLogFlags))
    {
        wprintf(L"\nAn event log entry will be created when the user");
        wprintf(L"\nexceeds his assigned warning threshold.\n");
    }
    else
    {
        wprintf(L"\nAn event log entry will NOT be created when the");
        wprintf(L"\nuser exceeds his assigned warning threshold.\n");
    }

    // Check if the system creates a log entry when a hard limit
    // is exceeded
    if (DISKQUOTA_IS_LOGGED_USER_LIMIT(dwQuotaLogFlags))
    {
        wprintf(L"\nAn event log entry will be created when the user");
        wprintf(L"\nexceeds his assigned hard quota limit.\n");
    }
    else
    {
        wprintf(L"\nAn event log entry will NOT be created when the");
        wprintf(L"\nuser exceeds his assigned hard quota limit.\n");
    }
    WaitForKeyPress();

    return TRUE;
}

/*-----------------------------------------------------------------------------
GetDefaultHardLimit(IDiskQuotaControl* lpDiskQuotaControl)
    Get the default hard limit of the volume.

Parameters
    lpDiskQuotaControl - Pointer to an object that implements the
                         IDiskQuotaControl interface

Return Value
    TRUE - Success
    FALSE - Failure
-----------------------------------------------------------------------------*/
BOOL GetDefaultHardLimit(IDiskQuotaControl* lpDiskQuotaControl)
{
    HRESULT hr;
    WCHAR   szDefaultHardLimitText[MAX_PATH];

    // Get the default hard quota limit in text format
    hr = lpDiskQuotaControl->
        GetDefaultQuotaLimitText(szDefaultHardLimitText, MAX_PATH);

    if (FAILED(hr))
    {
        PrintError(hr);
        return FALSE;
    }

    szDefaultHardLimitText[MAX_PATH - 1] = L'\0'; // Make sure szDefaultHardLimitText is NULL terminated

    wprintf(L"\n\nDefault Quota Hard Limit: %s\n", szDefaultHardLimitText);

    WaitForKeyPress();

    return TRUE;
}

/*-----------------------------------------------------------------------------
GetDefaultThreshold(IDiskQuotaControl* lpDiskQuotaControl)
    Get the default threshold of the volume.

Parameters
    lpDiskQuotaControl - Pointer to an object that implements the
                         IDiskQuotaControl interface

Return Value
    TRUE - Success
    FALSE - Failure
-----------------------------------------------------------------------------*/
BOOL GetDefaultThreshold(IDiskQuotaControl* lpDiskQuotaControl)
{
    HRESULT hr;
    WCHAR   szDefaultThresholdText[MAX_PATH];

    // Get the default threshold
    hr = lpDiskQuotaControl->
        GetDefaultQuotaThresholdText(szDefaultThresholdText, MAX_PATH);

    if (FAILED(hr))
    {
        PrintError(hr);
        return FALSE;
    }

    szDefaultThresholdText[MAX_PATH - 1] = L'\0'; // Make sure szDefaultThresholdText is NULL terminated

    wprintf(L"\n\nDefault Quota Threshold: %s\n", szDefaultThresholdText);

    WaitForKeyPress();
    
    return TRUE;
}

/*-----------------------------------------------------------------------------
SetDefaultHardLimit(IDiskQuotaControl* lpDiskQuotaControl)
    Set the default hard limit of the volume.

Parameters
    lpDiskQuotaControl - Pointer to an object that implements the
                         IDiskQuotaControl interface

Return Value
    TRUE - Success
    FALSE - Failure
-----------------------------------------------------------------------------*/
BOOL SetDefaultHardLimit(IDiskQuotaControl* lpDiskQuotaControl)
{
    HRESULT  hr;
    DWORD    dwCharsRead;
    LONGLONG llLimit = 0;
    WCHAR    szLimit[MAX_PATH] = {0};
    HANDLE   hStdIn = GetStdHandle(STD_INPUT_HANDLE);

    wprintf(L"\n\nEnter the new default hard limit in Bytes ");
    wprintf(L"(-1 == No Limit): ");

    // Get the limit from the command prompt
    ReadConsole(hStdIn, szLimit, MAX_PATH, &dwCharsRead, NULL);
    LfcrToNull(szLimit);
    llLimit = _wtoi64(szLimit);
    
    if (llLimit < -1)
    {
        wprintf(L"\nInvalid limit!");
        return FALSE;
    }

    // Set the default hard quota limit
    hr = lpDiskQuotaControl->SetDefaultQuotaLimit(llLimit);

    if (FAILED(hr))
    {
        PrintError(hr);
        return FALSE;
    }

    return TRUE;
}

/*-----------------------------------------------------------------------------
SetDefaultThreshold(IDiskQuotaControl* lpDiskQuotaControl)
    Set the default warning threshold of the volume.

Parameters
    lpDiskQuotaControl - Pointer to an object that implements the
                         IDiskQuotaControl interface

Return Value
    TRUE - Success
    FALSE - Failure
-----------------------------------------------------------------------------*/
BOOL SetDefaultThreshold(IDiskQuotaControl* lpDiskQuotaControl)
{
    HRESULT  hr;
    LONGLONG llLimit = 0;
    WCHAR    szLimit[MAX_PATH] = {0};
    DWORD    dwCharsRead;
    HANDLE   hStdIn = GetStdHandle(STD_INPUT_HANDLE);

    wprintf(L"\n\nEnter the new default threshold in Bytes ");
    wprintf(L"(-1 == No Limit): ");

    // Get the limit from the command prompt
    ReadConsole(hStdIn, szLimit, MAX_PATH, &dwCharsRead, NULL);
    LfcrToNull(szLimit);
    llLimit = _wtoi64(szLimit);

    if (llLimit < -1)
    {
        wprintf(L"\nInvalid limit!");
        return FALSE;
    }

    // Set the default warning threshold
    hr = lpDiskQuotaControl->SetDefaultQuotaThreshold(llLimit);

    if (FAILED(hr))
    {
        PrintError(hr);
        return FALSE;
    }

    return TRUE;
}

/*-----------------------------------------------------------------------------
SetUserHardLimit(IDiskQuotaControl* lpDiskQuotaControl)
    Set the hard quota limit for a specific user

Parameters
    lpDiskQuotaControl - Pointer to an object that implements the
                         IDiskQuotaControl interface

Return Value
    TRUE - Success
    FALSE - Failure
-----------------------------------------------------------------------------*/
BOOL SetUserHardLimit(IDiskQuotaControl* lpDiskQuotaControl)
{
    HRESULT  hr;
    IDiskQuotaUser* lpDiskQuotaUser;
    WCHAR    szUser[MAX_PATH] = {0};
    WCHAR    szLimit[MAX_PATH] = {0};
    DWORD    dwCharsRead;
    LONGLONG llLimit = 0;
    HANDLE   hStdIn  = GetStdHandle(STD_INPUT_HANDLE);

    wprintf(L"\n\nEnter the logon name of the user ");
    wprintf(L"(ie. DOMAIN\\USERNAME): ");

    // Get the user for which to set a hard limit
    ReadConsole(hStdIn, szUser, MAX_PATH, &dwCharsRead, NULL);
    szUser[MAX_PATH-1] = L'\0'; // make sure szUSer is NULL terminated

    // Strip the line feed and carriage return
    LfcrToNull(szUser);

    // Check if the name is valid
    hr = lpDiskQuotaControl->FindUserName((LPCWSTR)szUser, &lpDiskQuotaUser);

    if (SUCCEEDED(hr))
    {
        wprintf(L"\nEnter the new hard limit in bytes (-1 == No Limit): ");

        // Read the new hard limit from the console
        ReadConsole(hStdIn, szLimit, MAX_PATH, &dwCharsRead, NULL);
        LfcrToNull(szLimit);
        llLimit = _wtoi64(szLimit);

        if (llLimit >= -1)
        {
            // Set the hard quota limit for the user
            hr = lpDiskQuotaUser->SetQuotaLimit(llLimit, TRUE);

            if (FAILED(hr))
            {
                wprintf(L"\nCould not set the quota limit for ");
                wprintf(L"%s to %i64 bytes\n", szUser, llLimit);
            }
        }
        else
        {
            wprintf(L"\nInvalid limit!");
        }

        lpDiskQuotaUser->Release();
    }
    else
    {
        wprintf(L"\nCould not find quota data for %s\n", szUser);
    }

    return SUCCEEDED(hr);
}

/*-----------------------------------------------------------------------------
SetUserThreshold(IDiskQuotaControl* lpDiskQuotaControl)
    Set the warning threshold limit for a specific user

Parameters
    lpDiskQuotaControl - Pointer to an object that implements the
                         IDiskQuotaControl interface

Return Value
    TRUE - Success
    FALSE - Failure
-----------------------------------------------------------------------------*/
BOOL SetUserThreshold(IDiskQuotaControl* lpDiskQuotaControl)
{
    HRESULT  hr;
    IDiskQuotaUser* lpDiskQuotaUser;
    WCHAR    szUser[MAX_PATH] = {0};
    DWORD    dwCharsRead;
    LONGLONG llLimit = 0;
    HANDLE   hStdIn  = GetStdHandle(STD_INPUT_HANDLE);

    wprintf(L"\n\nEnter the logon name of the user ");
    wprintf(L"(ie. DOMAIN\\USERNAME): ");

    // Get the user for which to set a hard limit
    ReadConsole(hStdIn, szUser, MAX_PATH, &dwCharsRead, NULL);

    szUser[MAX_PATH-1] = L'\0'; // make sure szUSer is NULL terminated

    // Strip the line feed and carriage return
    LfcrToNull(szUser);

    // Check if the name is valid
    hr = lpDiskQuotaControl->FindUserName((LPCWSTR)szUser, &lpDiskQuotaUser);

    if (SUCCEEDED(hr))
    {
        WCHAR    szLimit[MAX_PATH] = {0};
        wprintf(L"\nEnter the new hard limit in bytes (-1 == No Limit): ");

        // Read the threshold from the console
        ReadConsole(hStdIn, szLimit, MAX_PATH, &dwCharsRead, NULL);
        LfcrToNull(szLimit);
        llLimit = _wtoi64(szLimit);

        if (llLimit >= -1)
        {
            // Set the warning threshold for the user
            hr = lpDiskQuotaUser->SetQuotaThreshold(llLimit, TRUE);

            if (FAILED(hr))
            {
                wprintf(L"\nCould not set the quota limit for %s", szUser);
                wprintf(L"to %i64 bytes\n", llLimit);
            }
        }
        else
        {
            wprintf(L"\nInvalid limit!");
        }
        lpDiskQuotaUser->Release();
    }
    else
    {
        PrintError(hr);
    }

    return SUCCEEDED(hr);
}

/*-----------------------------------------------------------------------------
GetUserQuotaInfo(IDiskQuotaControl* lpDiskQuotaControl)
    Get quota information for a specific user

Parameters
    lpDiskQuotaControl - Pointer to an object that implements the
                         IDiskQuotaControl interface

Return Value
    TRUE - Success
    FALSE - Failure
-----------------------------------------------------------------------------*/
BOOL GetUserQuotaInfo(IDiskQuotaControl* lpDiskQuotaControl)
{
    WCHAR    szUser[MAX_PATH] = {0};
    IDiskQuotaUser* lpDiskQuotaUser;
    DWORD    dwCharsRead;
    HANDLE   hStdIn  = GetStdHandle(STD_INPUT_HANDLE);
    HRESULT  hr;

    wprintf(L"\n\nEnter the logon name of the user");
    wprintf(L"(ie. DOMAIN\\USERNAME): ");

    // Get the user for which to get quota information 
    ReadConsole(hStdIn, szUser, MAX_PATH, &dwCharsRead, NULL);
    szUser[MAX_PATH-1] = L'\0'; // make sure szUser is NULL terminated
    LfcrToNull(szUser);

    // Check if the name is valid
    hr = lpDiskQuotaControl->FindUserName((LPCWSTR)szUser, &lpDiskQuotaUser);
    if (SUCCEEDED(hr))
    {
        WCHAR    szQuotaUsedText[MAX_PATH] = {0};
        WCHAR    szQuotaLimitText[MAX_PATH] = {0};
        WCHAR    szQuotaThresholdText[MAX_PATH] = {0};

        if (SUCCEEDED(hr = lpDiskQuotaUser->GetQuotaThresholdText(
                                    szQuotaThresholdText, MAX_PATH))         &&
            SUCCEEDED(hr = lpDiskQuotaUser->GetQuotaLimitText(
                                    szQuotaLimitText, MAX_PATH))             &&
            SUCCEEDED(hr = lpDiskQuotaUser->GetQuotaUsedText(
                                    szQuotaUsedText, MAX_PATH)))
        {
            szQuotaUsedText[MAX_PATH-1] = L'\0'; // make sure szQuotaUsedText is NULL terminated
            szQuotaLimitText[MAX_PATH-1] = L'\0'; // make sure szQuotaLimitText is NULL terminated
            szQuotaThresholdText[MAX_PATH-1] = L'\0'; // make sure szQuotaThresholdText is NULL terminated

            wprintf(L"Amount Used        Limit    Threshold\n");
            wprintf(L"-----------        -----    ---------\n");
            wprintf(L"   %10s", szQuotaUsedText);
            wprintf(L"   %10s", szQuotaLimitText);
            wprintf(L"   %10s", szQuotaThresholdText);
            wprintf(L"\n");
        }
        else
        {
            wprintf(L"\nCould not get the quota information for %s", szUser);
        }

        lpDiskQuotaUser->Release();
    }
    else
    {
        wprintf(L"\nCould not find quota data for %s\n", szUser);
    }

    WaitForKeyPress();

    return SUCCEEDED(hr);
}

/*-----------------------------------------------------------------------------
AddUser(IDiskQuotaControl* lpDiskQuotaControl)
    Add a user for which to track quota data

Parameters
    lpDiskQuotaControl - Pointer to an object that implements the
                         IDiskQuotaControl interface

Return Value
    TRUE - Success
    FALSE - Failure
-----------------------------------------------------------------------------*/
BOOL AddUser(IDiskQuotaControl* lpDiskQuotaControl)
{
    HRESULT  hr;
    IDiskQuotaUser* lpDiskQuotaUser;
    WCHAR    szUser[MAX_PATH] = {0};
    DWORD    dwCharsRead;
    HANDLE   hStdIn = GetStdHandle(STD_INPUT_HANDLE);

    wprintf(L"\n\nEnter the logon name of the user (ie. DOMAIN\\USERNAME): ");

    // Get the user for which to track quota information 
    ReadConsole(hStdIn, szUser, MAX_PATH, &dwCharsRead, NULL);
    LfcrToNull(szUser);

    // Add the user
    hr = lpDiskQuotaControl->AddUserName(szUser,
                                DISKQUOTA_USERNAME_RESOLVE_SYNC,
                                &lpDiskQuotaUser);

    if (FAILED(hr))
    {
        PrintError(hr);
        return FALSE;
    }
    lpDiskQuotaUser->Release();

    return TRUE;
}

/*-----------------------------------------------------------------------------
DeleteUser(IDiskQuotaControl* lpDiskQuotaControl)
    Add a user for which to track quota data

Parameters
    lpDiskQuotaControl - Pointer to an object that implements the
                         IDiskQuotaControl interface

Return Value
    TRUE - Success
    FALSE - Failure
-----------------------------------------------------------------------------*/
BOOL DeleteUser(IDiskQuotaControl* lpDiskQuotaControl)
{
    HRESULT         hr;
    IDiskQuotaUser* lpDiskQuotaUser;
    WCHAR           szUser[MAX_PATH] = {0};
    DWORD           dwCharsRead;
    HANDLE hStdIn = GetStdHandle(STD_INPUT_HANDLE);

    wprintf(L"\n\nEnter the logon name of the user (ie. DOMAIN\\USERNAME): ");

    // Get the user for which to track quota information 
    ReadConsole(hStdIn, szUser, MAX_PATH, &dwCharsRead, NULL);
    LfcrToNull(szUser);

    // Check if the name is valid
    hr = lpDiskQuotaControl->FindUserName((LPCWSTR)szUser, &lpDiskQuotaUser);

    if (SUCCEEDED(hr))
    {
        // Delete the user
        hr = lpDiskQuotaControl->DeleteUser(lpDiskQuotaUser);

        if (FAILED(hr))
        {
            wprintf(L"\nCould not delete the user.");
        }

        lpDiskQuotaUser->Release();
    }
    else
    {
        PrintError(hr);
    }

    return SUCCEEDED(hr);
}

/*-----------------------------------------------------------------------------
EnumerateUsers(IDiskQuotaControl* lpDiskQuotaControl)
    Enumerate through all of the disk quota users

Parameters
    lpDiskQuotaControl - Pointer to an object that implements the
                         IDiskQuotaControl interface

Return Value
    TRUE - Success
    FALSE - Failure
-----------------------------------------------------------------------------*/
BOOL EnumerateUsers(IDiskQuotaControl* lpDiskQuotaControl)
{
    HRESULT              hr;
    IEnumDiskQuotaUsers* lpEnumDiskQuotaUsers;
    IDiskQuotaUser*      lpDiskQuotaUser;
    DWORD                dwUsersFetched;

    // Create an enumerator object to enumerate quota users on the volume. 
    hr = lpDiskQuotaControl->CreateEnumUsers(NULL, 0,
                                             DISKQUOTA_USERNAME_RESOLVE_SYNC,
                                             &lpEnumDiskQuotaUsers);
    if (SUCCEEDED(hr))
    {
        WCHAR szLogonName[MAX_PATH];

        wprintf(L"\n\nLogon name\n");
        wprintf(L"----------\n");

        // Enumerate through all of the quota users
        while(SUCCEEDED(hr) &&
              S_OK == lpEnumDiskQuotaUsers->
                      Next(1, &lpDiskQuotaUser, &dwUsersFetched))
        {
            // Retrieve the name strings associated with this disk quota user.
            if (SUCCEEDED(hr = lpDiskQuotaUser->GetName(NULL, 0, szLogonName,
                                                        MAX_PATH, NULL, 0)))
            { 
                szLogonName[MAX_PATH-1] = L'\0'; // make sure szLogonName is NULL terminated
                wprintf(L"%s\n", szLogonName);
                lpDiskQuotaUser->Release();
            }
        }
        lpEnumDiskQuotaUsers->Release();
    }

    if (FAILED(hr)) PrintError(hr);
    WaitForKeyPress();

    return SUCCEEDED(hr);
}

/*-----------------------------------------------------------------------------
EnumerateUserQuotas(IDiskQuotaControl* lpDiskQuotaControl)
    Enumerate through all user's and their quota's

Parameters
    lpDiskQuotaControl - Pointer to an object that implements the
                         IDiskQuotaControl interface

Return Value
    TRUE - Success
    FALSE - Failure
-----------------------------------------------------------------------------*/
BOOL EnumerateUserQuotas(IDiskQuotaControl* lpDiskQuotaControl)
{
    HRESULT              hr;
    IEnumDiskQuotaUsers* lpEnumDiskQuotaUsers;

    // Create an enumerator object to enumerate quota users on the volume. 
    hr = lpDiskQuotaControl->CreateEnumUsers(NULL, 0,
                                             DISKQUOTA_USERNAME_RESOLVE_SYNC,
                                             &lpEnumDiskQuotaUsers);
    if (SUCCEEDED(hr))
    {
        IDiskQuotaUser*      lpDiskQuotaUser;
        DWORD                dwUsersFetched;
        WCHAR                szLogonName[MAX_PATH];
        WCHAR                szQuotaUsedText[MAX_PATH];
        WCHAR                szQuotaLimitText[MAX_PATH];
        WCHAR                szQuotaThresholdText[MAX_PATH];
        DISKQUOTA_USER_INFORMATION dqUserInfo;

        wprintf(L"\n\nStatus               Logon Name  ");
        wprintf(L"Amount Used        Limit    Threshold\n");
        wprintf(L"------               ----------  ");
        wprintf(L"-----------        -----    ---------\n");

        // Enumerate through all of the quota users
        while(SUCCEEDED(hr) &&
              S_OK == lpEnumDiskQuotaUsers->
                        Next(1, &lpDiskQuotaUser, &dwUsersFetched))
        {
            if (SUCCEEDED(hr = lpDiskQuotaUser->
                    GetName(NULL, 0, szLogonName, MAX_PATH, NULL, 0))      &&
                SUCCEEDED(hr = lpDiskQuotaUser->
                    GetQuotaThresholdText(szQuotaThresholdText, MAX_PATH)) &&
                SUCCEEDED(hr = lpDiskQuotaUser->
                    GetQuotaLimitText(szQuotaLimitText, MAX_PATH))         &&
                SUCCEEDED(hr = lpDiskQuotaUser->
                    GetQuotaUsedText(szQuotaUsedText, MAX_PATH))           &&
                SUCCEEDED(hr = lpDiskQuotaUser->
                    GetQuotaInformation((LPVOID)&dqUserInfo,
                        sizeof(DISKQUOTA_USER_INFORMATION))))
            {
                szLogonName[MAX_PATH - 1] = L'\0'; // Make sure szLogonName is NULL terminated
                szQuotaUsedText[MAX_PATH - 1] = L'\0'; // Make sure szQuotaUsedText is NULL terminated
                szQuotaLimitText[MAX_PATH - 1] = L'\0'; // Make sure szQuotaLimitText is NULL terminated
                szQuotaThresholdText[MAX_PATH - 1] = L'\0'; // Make sure szQuotaThresholdText is NULL terminated

                // Check if the user is exceeding their threshold
                if ((dqUserInfo.QuotaUsed > dqUserInfo.QuotaThreshold) &&
                    (dqUserInfo.QuotaThreshold >= 0))
                {
                    wprintf(L"Warning ");
                }
                else
                {
                    wprintf(L"OK      ");
                }

                wprintf(L"%23s", szLogonName);
                wprintf(L"   %10s", szQuotaUsedText);
                wprintf(L"   %10s", szQuotaLimitText);
                wprintf(L"   %10s", szQuotaThresholdText);
                wprintf(L"\n");
            }
            else
            {
                szLogonName[MAX_PATH - 1] = L'\0'; // Make sure szLogonName is NULL terminated

                wprintf(L"Could not retrieve %s's quota information\n",
                         szLogonName);
            }
            lpDiskQuotaUser->Release();
        }
        lpEnumDiskQuotaUsers->Release();
    }
    else
    {
        PrintError(hr);
    }

    WaitForKeyPress();

    return SUCCEEDED(hr);
}

/*-----------------------------------------------------------------------------
PrintError(HRESULT hr)
    Print out the error message that corresponds to the error code

Parameters
    hr - Error code

Return Value
    None
-----------------------------------------------------------------------------*/
void PrintError(HRESULT hr)
{
    LPVOID lpMsgBuf;
    
    // Format the error message
    if (!FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | 
                  FORMAT_MESSAGE_FROM_SYSTEM     |
                  FORMAT_MESSAGE_IGNORE_INSERTS,
                  NULL,
                  hr,
                  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Defaut lang
                  (LPWSTR) &lpMsgBuf,
                  0,
                  NULL ))
    {
        wprintf(L"Unknown error occured\n");
        return;
    }

    // Display the string
    wprintf(L"\n%s", (LPCWSTR)lpMsgBuf);

    // Free the buffer
    LocalFree( lpMsgBuf );

    WaitForKeyPress();
}

/*-----------------------------------------------------------------------------
LfcrToNull(LPWSTR szString)
    Replace the line feed/carriage return on a string w/ a terminating NULL

Parameters
    szString - String that contains a LFCR

Return Value
    None
-----------------------------------------------------------------------------*/
void LfcrToNull(LPWSTR szString)
{
    WCHAR* szLfcr;

    szLfcr = wcsstr(szString, L"\r\n");

    if (szLfcr) *szLfcr = L'\0';
}

/*-----------------------------------------------------------------------------
WaitForKeyPress()
    Prompt the user to press a key before continuing

Parameters
    None

Return Value
    None
-----------------------------------------------------------------------------*/
void WaitForKeyPress()
{
    HANDLE hStdIn = GetStdHandle(STD_INPUT_HANDLE);
    INPUT_RECORD sInputRecord;
    DWORD        dwInputEvents;

    wprintf(L"\nPress any key to continue: ");
    ReadConsoleInput(hStdIn,&sInputRecord,1 ,&dwInputEvents);
    while (!((sInputRecord.EventType == KEY_EVENT) &&             (sInputRecord.Event.KeyEvent.bKeyDown)))
    {
        FlushConsoleInputBuffer(hStdIn);
        ReadConsoleInput(hStdIn,&sInputRecord,1 ,&dwInputEvents);
    }
}