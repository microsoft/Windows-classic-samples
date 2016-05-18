/*----------------------------------------------------------------------------
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (C) 1999 - 2000.  Microsoft Corporation.  All rights reserved.


ProcessMenu.cpp

Description:
Implements the functions used to process the user's selection for the 
various menus.
----------------------------------------------------------------------------*/

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <stdio.h>
#include <dskquota.h>
#include "ProcessMenu.h"
#include "PrintMenu.h"
#include "Commands.h"

/*-----------------------------------------------------------------------------
ProcessMainMenu(WCHAR wcMenuChoice, IDiskQuotaControl* lpDiskQuotaControl)
    Prints out the main menu choices

Parameters
    wcMenuChoice - UNICODE menu choice

    lpDiskQuotaControl - Pointer to an object that implements the 
        IDiskQuotaControl interface.

Return Value
    TRUE - User's selection was processed successfully
    FALSE - User chose to exit
-----------------------------------------------------------------------------*/
BOOL ProcessMainMenu(WCHAR wcMenuChoice, IDiskQuotaControl* lpDiskQuotaControl)
{
    switch (wcMenuChoice)
    {
    case L'A':
    case L'a':
        // Print main menu stuff
        while (ProcessDefaultQuotaMenu(PrintDefaultQuotaMenu(),
                                       lpDiskQuotaControl));
        break;
    case L'B':
    case L'b':
        while (ProcessQuotaLogFlagMenu(PrintQuotaLogFlagMenu(),
                                       lpDiskQuotaControl));
        break;
    case L'C':
    case L'c':
        while (ProcessDefaultLimitsMenu(PrintDefaultLimitsMenu(),
                                        lpDiskQuotaControl));
        break;
    case L'D':
    case L'd':
        while (ProcessQuotaManagerMenu(PrintQuotaManagerMenu(),
                                       lpDiskQuotaControl));
        break;
    case L'E':
    case L'e':
        while (ProcessUserManagerMenu(PrintUserManagerMenu(),
                                      lpDiskQuotaControl));
        break;
    case L'X':
    case L'x':
        return FALSE;
        break;
    default:
        wprintf(L"\n\nInvalid selection.\n");
        break;
    }

    return TRUE;
}

/*-----------------------------------------------------------------------------
ProcessDefaultQuotaMenu(WCHAR wcMenuChoice,
                        IDiskQuotaControl* lpDiskQuotaControl)
    Processes the user's selection for getting and setting the default quota
    settings

Parameters
    wcMenuChoice - UNICODE menu choice

    lpDiskQuotaControl - Pointer to an object that implements the 
        IDiskQuotaControl interface.

Return Value
    TRUE - User's selection was processed successfully
    FALSE - User chose to exit
-----------------------------------------------------------------------------*/
BOOL ProcessDefaultQuotaMenu(WCHAR wcMenuChoice,
                             IDiskQuotaControl* lpDiskQuotaControl)
{
    DWORD dwState = 0;

    switch(wcMenuChoice)
    {
    case L'A':
    case L'a':
        GetDefaultQuota(lpDiskQuotaControl);
        break;
    case L'B':
    case L'b':
        lpDiskQuotaControl->SetQuotaState(DISKQUOTA_SET_DISABLED(dwState));
        break;
    case L'C':
    case L'c':
        lpDiskQuotaControl->SetQuotaState(DISKQUOTA_SET_TRACKED(dwState));
        break;
    case L'D':
    case L'd':
        lpDiskQuotaControl->SetQuotaState(DISKQUOTA_SET_ENFORCED(dwState));
        break;
    case L'X':
    case L'x':
        return FALSE;
        break;
    default:
        wprintf(L"\n\nInvalid selection.\n");
        break;
    }

    return TRUE;
}

/*-----------------------------------------------------------------------------
ProcessQuotaLogFlagMenu(WCHAR wcMenuChoice,
                        IDiskQuotaControl* lpDiskQuotaControl)
    Processes the user's selection for getting and setting the quota log flags

Parameters
    wcMenuChoice - UNICODE menu choice

    lpDiskQuotaControl - Pointer to an object that implements the 
        IDiskQuotaControl interface.

Return Value
    TRUE - User's selection was processed successfully
    FALSE - User chose to exit
-----------------------------------------------------------------------------*/
BOOL ProcessQuotaLogFlagMenu(WCHAR wcMenuChoice,
                             IDiskQuotaControl* lpDiskQuotaControl)
{
    DWORD dwQuotaLogFlags = 0;
    lpDiskQuotaControl->GetQuotaLogFlags(&dwQuotaLogFlags);

    switch(wcMenuChoice)
    {
    case L'A':
    case L'a':
        GetQuotaLogFlags(lpDiskQuotaControl);
        break;
    case L'B':
    case L'b':
        lpDiskQuotaControl->
            SetQuotaLogFlags(DISKQUOTA_SET_LOG_USER_THRESHOLD(dwQuotaLogFlags,
                                                              TRUE));
        break;
    case L'C':
    case L'c':
        lpDiskQuotaControl->
            SetQuotaLogFlags(DISKQUOTA_SET_LOG_USER_LIMIT(dwQuotaLogFlags,
                                                          TRUE));
        break;
    case L'D':
    case L'd':
        lpDiskQuotaControl->
            SetQuotaLogFlags(DISKQUOTA_SET_LOG_USER_THRESHOLD(dwQuotaLogFlags,
                                                              FALSE));
        break;
    case L'E':
    case L'e':
        lpDiskQuotaControl->
            SetQuotaLogFlags(DISKQUOTA_SET_LOG_USER_LIMIT(dwQuotaLogFlags,
                                                          FALSE));
        break;
    case L'X':
    case L'x':
        return FALSE;
        break;
    default:
        wprintf(L"\n\nInvalid selection.\n");
        break;
    }

    return TRUE;
}

/*-----------------------------------------------------------------------------
ProcessDefaultLimitsMenu(WCHAR wcMenuChoice,
                         IDiskQuotaControl* lpDiskQuotaControl)
    Processes the user's selection for getting and setting the default quota
    limit

Parameters
    wcMenuChoice - UNICODE menu choice

    lpDiskQuotaControl - Pointer to an object that implements the 
        IDiskQuotaControl interface.

Return Value
    TRUE - User's selection was processed successfully
    FALSE - User chose to exit
-----------------------------------------------------------------------------*/
BOOL ProcessDefaultLimitsMenu(WCHAR wcMenuChoice,
                              IDiskQuotaControl* lpDiskQuotaControl)
{
    DWORD dwQuotaLogFlags = 0;
    lpDiskQuotaControl->GetQuotaLogFlags(&dwQuotaLogFlags);

    switch(wcMenuChoice)
    {
    case L'A':
    case L'a':
        GetDefaultHardLimit(lpDiskQuotaControl);
        break;
    case L'B':
    case L'b':
        GetDefaultThreshold(lpDiskQuotaControl);
        break;
    case L'C':
    case L'c':
        SetDefaultHardLimit(lpDiskQuotaControl);
        break;
    case L'D':
    case L'd':
        SetDefaultThreshold(lpDiskQuotaControl);
        break;
    case L'X':
    case L'x':
        return FALSE;
        break;
    default:
        wprintf(L"\n\nInvalid selection.\n");
        break;
    }

    return TRUE;
}

/*-----------------------------------------------------------------------------
ProcessUserManagerMenu(WCHAR wcMenuChoice,
                       IDiskQuotaControl* lpDiskQuotaControl)
    Processes the user's selection for adding and deleting users for which to
    track disk quotas

Parameters
    wcMenuChoice - UNICODE menu choice

    lpDiskQuotaControl - Pointer to an object that implements the 
        IDiskQuotaControl interface.

Return Value
    TRUE - User's selection was processed successfully
    FALSE - User chose to exit
-----------------------------------------------------------------------------*/
BOOL ProcessUserManagerMenu(WCHAR wcMenuChoice,
                            IDiskQuotaControl* lpDiskQuotaControl)
{
    DWORD dwQuotaLogFlags = 0;
    lpDiskQuotaControl->GetQuotaLogFlags(&dwQuotaLogFlags);

    switch(wcMenuChoice)
    {
    case L'A':
    case L'a':
        AddUser(lpDiskQuotaControl);
        break;
    case L'B':
    case L'b':
        DeleteUser(lpDiskQuotaControl);
        break;
    case L'C':
    case L'c':
        EnumerateUsers(lpDiskQuotaControl);
        break;
    case L'X':
    case L'x':
        return FALSE;
        break;
    default:
        wprintf(L"\n\nInvalid selection.\n");
        break;
    }

    return TRUE;
}

/*-----------------------------------------------------------------------------
ProcessUserManagerMenu(WCHAR wcMenuChoice,
                       IDiskQuotaControl* lpDiskQuotaControl)
    Processes the user's selection for adding and deleting users for which to
    track disk quotas

Parameters
    wcMenuChoice - UNICODE menu choice

    lpDiskQuotaControl - Pointer to an object that implements the 
        IDiskQuotaControl interface.

Return Value
    TRUE - User's selection was processed successfully
    FALSE - User chose to exit
-----------------------------------------------------------------------------*/
BOOL ProcessQuotaManagerMenu(WCHAR wcMenuChoice,
                             IDiskQuotaControl* lpDiskQuotaControl)
{
    DWORD dwQuotaLogFlags = 0;
    lpDiskQuotaControl->GetQuotaLogFlags(&dwQuotaLogFlags);

    switch(wcMenuChoice)
    {
    case L'A':
    case L'a':
        EnumerateUserQuotas(lpDiskQuotaControl);
        break;
    case L'B':
    case L'b':
        SetUserHardLimit(lpDiskQuotaControl);
        break;
    case L'C':
    case L'c':
        SetUserThreshold(lpDiskQuotaControl);
        break;
    case L'X':
    case L'x':
        return FALSE;
        break;
    default:
        wprintf(L"\n\nInvalid selection.\n");
        break;
    }

    return TRUE;
}
