/*----------------------------------------------------------------------------
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (C) 1999 - 2000.  Microsoft Corporation.  All rights reserved.


PrintMenu.cpp

Description:
Implements the functions used to print out the various menus and return
the user's selection.
----------------------------------------------------------------------------*/

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <stdio.h>

static WCHAR GetUserInput();

/*-----------------------------------------------------------------------------
PrintMainMenu()
    Prints out the main menu choices

Parameters
    None

Return Value
    UNICODE value of the user's menu selection
-----------------------------------------------------------------------------*/
WCHAR PrintMainMenu()
{
    wprintf(L"\n\nMain Menu\n");
    wprintf(L"---------\n");
    wprintf(L"A) Get/Set default volume quota settings\n");
    wprintf(L"B) Get/Set quota log flags\n");
    wprintf(L"C) Get/Set default quota limits\n");
    wprintf(L"D) Get/Set user quotas\n");
    wprintf(L"E) Add/Delete a user for which to keep track of disk quotas\n");
    wprintf(L"X) Exit \n");
    wprintf(L"Please enter your selection: ");

    return (GetUserInput());
}

/*-----------------------------------------------------------------------------
PrintDefaultQuotaMenu()
    Prints out the choices for getting and setting the default volume
    quota settings

Parameters
    None

Return Value
    UNICODE value of the user's menu selection
-----------------------------------------------------------------------------*/
WCHAR PrintDefaultQuotaMenu()
{
    wprintf(L"\n\nDefault Quota Menu\n");
    wprintf(L"------------------\n");
    wprintf(L"A) Get default volume quota settings\n");
    wprintf(L"B) Disable disk quotas\n");
    wprintf(L"C) Track disk quotas\n");
    wprintf(L"D) Enforce disk quotas\n");
    wprintf(L"X) Exit back to main menu\n");
    wprintf(L"Please enter your selection: ");

    return (GetUserInput());
}

/*-----------------------------------------------------------------------------
PrintQuotaLogFlagMenu()
    Prints out the choices for getting and setting the quota log flags

Parameters
    None

Return Value
    UNICODE value of the user's menu selection
-----------------------------------------------------------------------------*/
WCHAR PrintQuotaLogFlagMenu()
{
    wprintf(L"\n\nQuota Log Flag Menu\n");
    wprintf(L"-------------------\n");
    wprintf(L"A) Get default volume log flags\n");
    wprintf(L"B) Enable logging of threshold violations\n");
    wprintf(L"C) Enable logging of hard quota limit violations\n");
    wprintf(L"D) Disable logging of threshold violations\n");
    wprintf(L"E) Disable logging of hard quota limit violations\n");
    wprintf(L"X) Exit back to main menu\n");
    wprintf(L"Please enter your selection: ");

    return (GetUserInput());
}

/*-----------------------------------------------------------------------------
PrintDefaultLimitsMenu()
    Prints out the choices for getting and setting the default quota limits

Parameters
    None

Return Value
    UNICODE value of the user's menu selection
-----------------------------------------------------------------------------*/
WCHAR PrintDefaultLimitsMenu()
{
    wprintf(L"\n\nDefault Limits Menu\n");
    wprintf(L"-------------------\n");
    wprintf(L"A) Get default quota hard limit\n");
    wprintf(L"B) Get default quota threshold\n");
    wprintf(L"C) Set default quota hard limit\n");
    wprintf(L"D) Set default quota threshold\n");
    wprintf(L"X) Exit back to main menu\n");
    wprintf(L"Please enter your selection: ");

    return (GetUserInput());
}

/*-----------------------------------------------------------------------------
PrintUserManagerMenu()
    Prints out the choices for adding and deleting users for which to keep
    track of disk quotas

Parameters
    None

Return Value
    UNICODE value of the user's menu selection
-----------------------------------------------------------------------------*/
WCHAR PrintUserManagerMenu()
{
    wprintf(L"\n\nUser Manager Menu\n");
    wprintf(L"-----------------\n");
    wprintf(L"A) Add a user for which to keep track of quotas\n");
    wprintf(L"B) Delete a user for which you no longer which to track quotas\n");
    wprintf(L"C) Enumerate users for which quotas are tracked\n");
    wprintf(L"X) Exit back to main menu\n");
    wprintf(L"Please enter your selection: ");

    return (GetUserInput());
}

/*-----------------------------------------------------------------------------
PrintQuotaManagerMenu()
    Prints out the choices for getting quotas for all users and for setting
    quotas for a particular user

Parameters
    None

Return Value
    UNICODE value of the user's menu selection
-----------------------------------------------------------------------------*/
WCHAR PrintQuotaManagerMenu()
{
    wprintf(L"\n\nQuota Manager Menu\n");
    wprintf(L"------------------\n");
    wprintf(L"A) Get the quota information for all users\n");
    wprintf(L"B) Set the quota hard limit for a user\n");
    wprintf(L"C) Set the quota threshold for a user\n");
    wprintf(L"X) Exit back to main menu\n");
    wprintf(L"Please enter your selection: ");

    return (GetUserInput());
}

/*-----------------------------------------------------------------------------
GetUserInput()
    Gets the user's menu selection and returns the UNICODE value

Parameters
    None

Return Value
    UNICODE value of the user's menu selection
-----------------------------------------------------------------------------*/
WCHAR GetUserInput()
{
    HANDLE hStdIn = GetStdHandle(STD_INPUT_HANDLE);
    INPUT_RECORD sInputRecord;
    DWORD        dwInputEvents;

    ReadConsoleInput(hStdIn,&sInputRecord,1 ,&dwInputEvents);
    while (!((sInputRecord.EventType == KEY_EVENT) && (sInputRecord.Event.KeyEvent.bKeyDown)))
    {
        FlushConsoleInputBuffer(hStdIn);
        ReadConsoleInput(hStdIn,&sInputRecord,1 ,&dwInputEvents);
    }

    return sInputRecord.Event.KeyEvent.uChar.UnicodeChar;
}
