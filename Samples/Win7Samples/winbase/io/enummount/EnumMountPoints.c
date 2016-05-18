/*----------------------------------------------------------------------------
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (C) Microsoft Corporation.  All rights reserved.


EnumMountPoints.c

This file implements a command line utility that enumerates volumes and
mount points (if any) on each volume.

----------------------------------------------------------------------------*/

#include <windows.h>
#include <stdio.h>
#include <tchar.h>

static void EnumVolumes();
static void EnumMountPoints(LPTSTR szVolume);
static void PrintMountPoint(LPTSTR szVolume, LPTSTR szMountPoint);
static void PrintDosDeviceNames(LPTSTR szVolume);

void main()
{
    EnumVolumes();
}

/*-----------------------------------------------------------------------------
EnumVolumes()

Parameters
    None

Return Value
    None

Notes
    FindFirstVolume/FindNextVolume returns the unique volume name for each.
    Since unique volume names aren't very user friendly, PrintDosDeviceNames
    prints out the Dos device name(s) that refer to the volume.
-----------------------------------------------------------------------------*/
void EnumVolumes()
{
    HANDLE hFindVolume;
    TCHAR  szVolumeName[MAX_PATH];

    // Find the first unique volume & enumerate it's mount points
    hFindVolume = FindFirstVolume(szVolumeName, MAX_PATH);

    // If we can't even find one volume, just print an error and return.
    if (hFindVolume == INVALID_HANDLE_VALUE)
    {
        _tprintf(_T("FindFirstVolume failed.  Error = %d\n"), GetLastError());
        return;
    }

    _tprintf(_T("\nUnique vol name: "));
    _tprintf(_T("%s\n"), szVolumeName);
    PrintDosDeviceNames(szVolumeName);
    EnumMountPoints(szVolumeName);

    // Find the rest of the unique volumes and enumerate each of their
    // mount points.
    while (FindNextVolume(hFindVolume, szVolumeName, MAX_PATH))
    {
        _tprintf(_T("\nUnique vol name: "));
        _tprintf(_T("%s\n"), szVolumeName);
        PrintDosDeviceNames(szVolumeName);
        EnumMountPoints(szVolumeName);
    }

    FindVolumeClose(hFindVolume);
}

/*-----------------------------------------------------------------------------
EnumMountPoints(LPTSTR szVolume)

Parameters
    szVolume
        Unique volume name of the volume to enumerate mount points for.

Return Value
    None

Notes
    Enumerates and prints the volume mount points (if any) for the unique
    volume name passed in.
-----------------------------------------------------------------------------*/
void EnumMountPoints(LPTSTR szVolume)
{
    HANDLE hFindMountPoint;
    TCHAR  szMountPoint[MAX_PATH];


    // Find and print the first mount point.
    hFindMountPoint = FindFirstVolumeMountPoint(szVolume, szMountPoint, MAX_PATH);

    // If a mount point was found, print it out, if there is not even
    // one mount point, just print "None" and return.
    if (hFindMountPoint != INVALID_HANDLE_VALUE)
    {
        PrintMountPoint(szVolume, szMountPoint);
    }
    else
    {
        _tprintf(_T("No mount points.\n"));
        return;
    }

    // Find and print the rest of the mount points
    while (FindNextVolumeMountPoint(hFindMountPoint, szMountPoint, MAX_PATH))
    {
        PrintMountPoint(szVolume, szMountPoint);
    }

    FindVolumeMountPointClose(hFindMountPoint);
}

/*-----------------------------------------------------------------------------
PrintMountPoint(LPTSTR szVolume, LPTSTR szMountPoint)

Parameters
    szVolume
        Unique volume name the mount point is located on

    szMountPoint
        Name of the mount point to print

Return Value
    None

Notes
    Prints out both the mount point and the unique volume name of the volume
    mounted at the mount point.
-----------------------------------------------------------------------------*/
void PrintMountPoint(LPTSTR szVolume, LPTSTR szMountPoint)
{
    TCHAR szMountPointPath[MAX_PATH];
    TCHAR szVolumeName[MAX_PATH];

    _tprintf(_T("  * Mount point: "));

    // Print out the mount point
    _tprintf(_T("%s\n"), szMountPoint);
    _tprintf(_T("                     ...is a mount point for...\n"));

    // Append the mount point name to the unique volume name to get the
    // complete path name for the mount point
    _tcscpy_s(szMountPointPath, MAX_PATH, szVolume);
    _tcscat_s(szMountPointPath, MAX_PATH, szMountPoint);

    // Get and print the unique volume name for the volume mounted at the
    // mount point
    if (!GetVolumeNameForVolumeMountPoint(szMountPointPath, szVolumeName, MAX_PATH))
    {
        _tprintf(_T("GetVolumeNameForVolumeMountPoint failed.  Error = %d\n"), GetLastError());
    }
    else
    {
        _tprintf(_T("                 %s\n"), szVolumeName);
    }
}

/*-----------------------------------------------------------------------------
PrintDosDeviceNames(LPTSTR szVolume)

Parameters
    szVolume
        Unique volume name to get the Dos device names for

Return Value
    None

Notes
    Prints out the Dos device name(s) for the unique volume name
-----------------------------------------------------------------------------*/
void PrintDosDeviceNames(LPTSTR szVolume)
{
    int    nStrLen;
    DWORD  dwBuffLen;
    LPTSTR szDrive;
    LPTSTR szBuffer = NULL;
    TCHAR  szVolumeName[MAX_PATH];

    // Get all logical drive strings
    dwBuffLen = GetLogicalDriveStrings(0, szBuffer);
    szBuffer = (LPTSTR)malloc(dwBuffLen*sizeof(TCHAR));
    GetLogicalDriveStrings(dwBuffLen, szBuffer);
    szDrive = szBuffer;

    _tprintf(_T("Dos drive names: "));

    nStrLen = (int)_tcslen(szDrive);

    // Get the unique volume name for each logical drive string.  If the volume
    // drive string matches the passed in volume, print out the Dos drive name
    while(nStrLen)
    {
        if (GetVolumeNameForVolumeMountPoint(szDrive, szVolumeName, MAX_PATH))
        {
            if (_tcsicmp(szVolume, szVolumeName) == 0)
            {
                _tprintf(_T("%s "), szDrive);
            }
        }
        szDrive += nStrLen+1;
        nStrLen = (int)_tcslen(szDrive);
    }

    _tprintf(_T("\n"));
    if (szBuffer) free(szBuffer);
}
