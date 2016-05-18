//------------------------------------------------------------------------------
// File: persist.cpp
//
// Desc: DirectShow sample code
//       - State persistence helper functions
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#include "project.h"


// Global data
RECENTFILES aRecentFiles[MAX_RECENT_FILES]={0};
int         nRecentFiles=0;

// Global static data
static TCHAR cszWindow[] = TEXT("Window\0");
static TCHAR cszAppKey[] = TEXT("Software\\Microsoft\\Multimedia Tools\\VMRPlayer9\0");



#ifndef CheckPointer
#define CheckPointer(x, hr) if (x == NULL) { return hr; }
#endif

// Constants
const int CX_DEFAULT = 400;     /* Default window width */
const int CY_DEFAULT = 400;     /* Default window height */


/******************************Public*Routine******************************\
* GetAppKey
*
\**************************************************************************/
HKEY
GetAppKey(
    BOOL fCreate
    )
{
    HKEY hKey = 0;

    if(fCreate)
    {
        if(RegCreateKey(HKEY_CURRENT_USER, cszAppKey, &hKey) == ERROR_SUCCESS)
            return hKey;
    }
    else
    {
        if(RegOpenKey(HKEY_CURRENT_USER, cszAppKey, &hKey) == ERROR_SUCCESS)
            return hKey;
    }

    return NULL;
}


/******************************Public*Routine******************************\
* ProfileStringIn
*
\**************************************************************************/
UINT
ProfileStringIn(
    LPTSTR  szKey,
    LPTSTR  szDefault,
    LPTSTR  szProfileString,
    DWORD   cb
    )
{
    CheckPointer(szKey,0);
    CheckPointer(szDefault,0);
    CheckPointer(szProfileString,0);

    HKEY  hKey;
    DWORD dwType;

    if((hKey = GetAppKey(FALSE)) == 0)
    {
        StringCbCopy(szProfileString, cb, szDefault);
        return lstrlen(szProfileString);
    }

    if((RegQueryValueEx(hKey, szKey, NULL, &dwType, 
                       (LPBYTE)szProfileString, &cb) != ERROR_SUCCESS)
        || dwType != REG_SZ)
    {
        StringCbCopy(szProfileString, cb, szDefault);
        cb = lstrlen(szProfileString);
    }

    RegCloseKey(hKey);
    return cb;
}


/******************************Public*Routine******************************\
* ProfileStringOut
*
\**************************************************************************/
void
ProfileStringOut(
    LPTSTR  szKey,
    LPTSTR  szProfileString
    )
{
    assert(szKey);
    assert(szProfileString);

    HKEY  hKey;

    hKey = GetAppKey(TRUE);
    if (hKey)
    {
        RegSetValueEx(hKey, szKey, 0, REG_SZ, (LPBYTE)szProfileString,
                      sizeof(TCHAR) * (lstrlen(szProfileString)+1));

        RegCloseKey(hKey);
    }
}


/******************************Public*Routine******************************\
 * LoadWindowPos
 *
 * Retrieve the window position information from the registry
 *
\**************************************************************************/

#ifndef SPI_GETWORKAREA
 #define SPI_GETWORKAREA 48  // because NT doesnt have this define yet
#endif

BOOL
LoadWindowPos(
    LPRECT lprc
    )
{
    CheckPointer(lprc,FALSE);
    
    static RECT rcDefault = {0,0,CX_DEFAULT,CY_DEFAULT};
    RECT  rcScreen, rc;
    HKEY  hKey = GetAppKey(FALSE);

    // Read window placement from the registry.
    //
    *lprc = rcDefault;

    if(hKey)
    {
        DWORD cb = sizeof(rc);
        DWORD dwType;

        if(ERROR_SUCCESS == RegQueryValueEx(hKey, cszWindow, NULL, &dwType, (LPBYTE)&rc, &cb)
            && dwType == REG_BINARY && cb == sizeof(RECT))
        {
            *lprc = rc;
        }

        RegCloseKey(hKey);
    }

    // If we fail to get the working area (screen-tray), then assume
    // the screen is 640x480
    //
    if(! SystemParametersInfo(SPI_GETWORKAREA, 0, &rcScreen, FALSE))
    {
        rcScreen.top    = rcScreen.left = 0;
        rcScreen.right  = 640;
        rcScreen.bottom = 480;
    }

    // If the proposed window position is outside the screen,
    // use the default placement
    //
    if(! IntersectRect(&rc, &rcScreen, lprc))
    {
        *lprc = rcDefault;
    }

    return ! IsRectEmpty(lprc);
}


/*****************************Public*Routine******************************\
 * SaveWindowPos
 *
 * Store the window position information in the registry
 *
\**************************************************************************/
BOOL
SaveWindowPos(
    HWND hwnd
    )
{
    WINDOWPLACEMENT wpl;

    HKEY hKey = GetAppKey(TRUE);
    if(!hKey)
    {
        return FALSE;
    }

    // Save the current size and position of the window to the registry
    //
    ZeroMemory(&wpl, sizeof(wpl));
    wpl.length = sizeof(wpl);
    GetWindowPlacement(hwnd, &wpl);

    RegSetValueEx(hKey, cszWindow, 0, REG_BINARY,
                 (LPBYTE)&wpl.rcNormalPosition,
                 sizeof(wpl.rcNormalPosition));

    RegCloseKey(hKey);
    return TRUE;
}


/*****************************Private*Routine******************************\
* GetRecentFiles
*
* Reads at most MAX_RECENT_FILES from the app's registry entry. 
* Returns the number of files actually read.  
* Updates the File menu to show the "recent" files.
*
\**************************************************************************/
int
GetRecentFiles(
    int iLastCount,
    int iMenuPosition   // Menu position of start of MRU list
    )
{
    int     i;
    TCHAR   FileName[MAX_PATH];
    TCHAR   szKey[32];
    HMENU   hSubMenu;

    //
    // Delete the files from the menu
    //
    hSubMenu = GetSubMenu(GetMenu(hwndApp), 0);

    // Delete the separator at the requested position and all the other 
    // recent file entries
    if(iLastCount != 0)
    {
        DeleteMenu(hSubMenu, iMenuPosition, MF_BYPOSITION);

        for(i = 1; i <= iLastCount; i++)
        {
            DeleteMenu(hSubMenu, ID_RECENT_FILE_BASE + i, MF_BYCOMMAND);
        }
    }

    for(i = 1; i <= MAX_RECENT_FILES; i++)
    {
        DWORD   len;
        TCHAR   szMenuName[MAX_PATH + 3];

        (void)StringCchPrintf(szKey, NUMELMS(szKey), TEXT("File %d\0"), i);

        len = ProfileStringIn(szKey, TEXT(""), FileName, MAX_PATH * sizeof(TCHAR));
        if(len == 0)
        {
            i = i - 1;
            break;
        }

        StringCchCopy(aRecentFiles[i - 1], NUMELMS(aRecentFiles[i-1]), FileName);
        (void)StringCchPrintf(szMenuName, NUMELMS(szMenuName), TEXT("&%d %s\0"), i, FileName);

        if(i == 1)
        {
            InsertMenu(hSubMenu, iMenuPosition, MF_SEPARATOR | MF_BYPOSITION, (UINT)-1, NULL);
        }

        InsertMenu(hSubMenu, iMenuPosition + i, MF_STRING | MF_BYPOSITION,
            ID_RECENT_FILE_BASE + i, szMenuName);
    }

    //
    // i is the number of recent files in the array.
    //
    return i;
}


/*****************************Private*Routine******************************\
* SetRecentFiles
*
* Writes the most recent files to the registry.
* Purges the oldest file if necessary.
*
\**************************************************************************/
int
SetRecentFiles(
    TCHAR *FileName,    // File name to add
    int iCount,         // Current count of files
    int iMenuPosition   // Menu position of start of MRU list
    )
{
    TCHAR   FullPathFileName[MAX_PATH];
    TCHAR   *lpFile=0;
    TCHAR   szKey[32];
    int     iCountNew, i;

    //
    // Check for duplicates - we don't allow them
    //
    for(i = 0; i < iCount; i++)
    {
        if(0 == lstrcmpi(FileName, aRecentFiles[i]))
        {
            return iCount;
        }
    }

    //
    // Throw away the oldest entry
    //
    MoveMemory(&aRecentFiles[1], &aRecentFiles[0],
               sizeof(aRecentFiles) - sizeof(aRecentFiles[1]));

    //
    // Copy in the full path of the new file.
    //
    GetFullPathName(FileName, MAX_PATH, FullPathFileName, &lpFile);
    StringCchCopy(aRecentFiles[0], NUMELMS(aRecentFiles[0]), FullPathFileName);

    //
    // Update the count of files, saturate to MAX_RECENT_FILES.
    //
    iCountNew = min(iCount + 1, MAX_RECENT_FILES);

    //
    // Clear the old stuff and the write out the recent files to disk
    //
    for(i = 1; i <= iCountNew; i++)
    {
        (void)StringCchPrintf(szKey, NUMELMS(szKey),TEXT("File %d\0"), i);
        ProfileStringOut(szKey, aRecentFiles[i - 1]);
    }

    //
    // Update the file menu
    //
    GetRecentFiles(iCount, iMenuPosition);

    return iCountNew;  // the updated count of files.
}



