/*-----------------------------------------------------------------------------
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (C) 1999 - 2001 Microsoft Corporation.  All rights reserved.

DLEDIT  -- Drive Letter Assignment Editor

This program demonstrates how to add or remove persistent drive letter
assignments in Windows 2000, Windows XP, and Windows Server 2003.  These
drive letter assignments persist through machine reboots.

Platforms:
   This program requires Windows 2000 or later.

Command-line syntax:
   DLEDIT <drive letter> <NT device name>      -- Adds persistent drive letter
   DLEDIT -t <drive letter> <NT device name>   -- Adds temporary drive letter
   DLEDIT -r <drive letter>                    -- Removes a drive letter
   DLEDIT <drive letter>                       -- Shows drive letter mapping
   DLEDIT -a                                   -- Shows all drive letter mappings

Command-line examples:

   Say that E: refers to CD-ROM drive, and you want to make F: point to that
   CD-ROM drive instead.  Use the following two commands:

      DLEDIT -r E:\
      DLEDIT F:\ \Device\CdRom0

   To display what device a drive letter is mapped to, use the following
   command:

      DLEDIT f:


*******************************************************************************
WARNING: WARNING: WARNING: WARNING: WARNING: WARNING: WARNING: WARNING:

   This program really will change drive letter assignments, and the changes
   persist through reboots.  Do not remove drive letters of your hard disks if
   you don't have this program on a floppy disk or you might not be able to
   access your hard disks again!
*******************************************************************************

-----------------------------------------------------------------------------*/

/* reduce number of headers pulled in by windows.h to improve build time */
#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0500

#include <windows.h>
#pragma warning(disable:4201) // nonstandard extension used : nameless struct/union
#include <winioctl.h>
#include <TCHAR.H>
#include <stdio.h>
#include <stdlib.h>

/*-----------------------------------------------------------------------------*/
#define GPT_BASIC_DATA_ATTRIBUTE_HIDDEN  (0x4000000000000000) // from DDK's NTDDDISK.H

#define IOCTL_VOLUME_GET_GPT_ATTRIBUTES CTL_CODE(IOCTL_VOLUME_BASE, 14, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef struct _VOLUME_GET_GPT_ATTRIBUTES_INFORMATION {
    ULONGLONG   GptAttributes;
} VOLUME_GET_GPT_ATTRIBUTES_INFORMATION, *PVOLUME_GET_GPT_ATTRIBUTES_INFORMATION;

/*-----------------------------------------------------------------------------*/


#define HIDDEN_PARTITION_FLAG 0x10


/* Private error codes.  Follows guidelines in winerror.h */
const DWORD PRIV_ERROR_DRIVE_LETTER_IN_USE      = 0xE0000001;
//const DWORD PRIV_ERROR_DRIVE_LETTER_NOT_USED    = 0xE0000002;
const DWORD PRIV_ERROR_PARTITION_HIDDEN         = 0x60000001;
const DWORD PRIV_ERROR_PARTITION_NOT_RECOGNIZED = 0x60000002;
//const DWORD PRIV_ERROR_NOT_PARTITIONABLE_DEVICE = 0x60000003;



BOOL WINAPI AssignPersistentDriveLetter (LPCTSTR pszDriveLetter, LPCTSTR pszDeviceName);
BOOL WINAPI RemovePersistentDriveLetter (LPCTSTR pszDriveLetter);
BOOL WINAPI AssignTemporaryDriveLetter (LPCTSTR pszDriveLetter, LPCTSTR pszDeviceName);
BOOL WINAPI RemoveTemporaryDriveLetter (LPCTSTR pszDriveLetter);

BOOL WINAPI IsPartitionHidden (BYTE partitionType, ULONGLONG partitionAttribs);
BOOL WINAPI IsDriveLetter (LPCTSTR pszDriveLetter);
BOOL WINAPI IsWindowsXP_orLater (void);


static void PrintHelp (LPCTSTR pszAppName);


/*-----------------------------------------------------------------------------
main( IN argc, IN argv )

Parameters
   argc
      Count of the command-line arguments
   argv
      Array of pointers to the individual command-line arguments

This function is the main program.  It parses the command-line arguments and
performs the work of either removing a drive letter or adding a new one.
-----------------------------------------------------------------------------*/
extern "C"
void _tmain (int argc, TCHAR **argv)
{
   TCHAR * pszDriveLetter,
        * pszNTDevice,
        * pszOptions;

   /*
      Command-line parsing.
         1) Validate arguments
         2) Determine what user wants to do
   */
   if (3 == argc && !lstrcmpi (argv[1], _T("-r")) && IsDriveLetter (argv[2]))
   {
      /*
            User wants to remove the drive letter.  Command line should be:
               dledit -r <drive letter>
      */
      pszOptions       = argv[1];
      pszDriveLetter   = argv[2];
      pszNTDevice      = NULL;

      if (!RemovePersistentDriveLetter (pszDriveLetter))
      {
         if (!RemoveTemporaryDriveLetter (pszDriveLetter))
         {
            switch (GetLastError())
            {
            case ERROR_FILE_NOT_FOUND:
               _tprintf(_T("%s is not in use\n"), pszDriveLetter);
               break;

            default:
               _tprintf(_T("error %lu: couldn't remove %s\n"),
                     GetLastError(), pszDriveLetter);
            }
         }
      }

   } /* End if remove drive letter mapping */

   else if (3 == argc && IsDriveLetter (argv[1]))
   {
      /*
         User wants to add a persistent drive letter.  Command line should be:
            dledit <drive letter> <NT device name>
      */
      pszOptions       = NULL;
      pszDriveLetter   = argv[1];
      pszNTDevice      = argv[2];

      /*
         Try a persistent drive letter; if partition is hidden, the persistent
         mapping will fail--try a temporary mapping and tell user about it.

         Note: Hidden partitions can be assigned temporary drive letters.
         These are nothing more than symbolic links created with
         DefineDosDevice.  Temporary drive letters will be removed when the
         system is rebooted.
      */
      if (!AssignPersistentDriveLetter (pszDriveLetter, pszNTDevice))
      {
         switch (GetLastError())
         {
         case PRIV_ERROR_PARTITION_HIDDEN:
            if (AssignTemporaryDriveLetter (pszDriveLetter, pszNTDevice))
               _tprintf(_T("%s is hidden; mapped %s temporarily\n"),
                        pszNTDevice, pszDriveLetter);
            else
               _tprintf(_T("%s is hidden; couldn't map %s to it\n"),
                        pszNTDevice, pszDriveLetter);
            break;

         case PRIV_ERROR_DRIVE_LETTER_IN_USE:
            _tprintf(_T("%s is in use, can't map it to %s\n"),
                     pszDriveLetter, pszNTDevice);
            break;

         case ERROR_FILE_NOT_FOUND:
            _tprintf(_T("%s doesn't exist or can't be opened\n"),
                     pszNTDevice);
            break;

         case ERROR_INVALID_PARAMETER:
            _tprintf(_T("%s already has a drive letter; can't map %s to it\n"),
                     pszNTDevice, pszDriveLetter);
            break;

         default:
            _tprintf(_T("error %lu: couldn't map %s to %s\n"),
                  GetLastError(), pszDriveLetter, pszNTDevice);
         }
      }
   } /* End if add persistent drive letter mapping */

   else if (4 == argc && !lstrcmpi (argv[1], _T("-t")) && IsDriveLetter (argv[2]))
   {
      /*
         User wants to add a temporary drive letter.  Command line should be:
            dledit -t <drive letter> <NT device name>
      */
      pszOptions       = argv[1];
      pszDriveLetter   = argv[2];
      pszNTDevice      = argv[3];

      if (!AssignTemporaryDriveLetter (pszDriveLetter, pszNTDevice))
      {
         switch (GetLastError())
         {
          case ERROR_FILE_NOT_FOUND:
            _tprintf(_T("%s doesn't exist or can't be opened\n"),
                     pszNTDevice);
            break;

          case PRIV_ERROR_DRIVE_LETTER_IN_USE:
            _tprintf(_T("%s is in use, can't map it to %s\n"),
                     pszDriveLetter, pszNTDevice);
            break;

         default:
            _tprintf(_T("error %lu: couldn't map %s to %s\n"),
                     GetLastError(), pszDriveLetter, pszNTDevice);
         }
      }
   } /* End if add temporary drive letter mapping */

   else if (2 == argc && IsDriveLetter (argv[1]))
   {
      /*
         User wants to show what device is connected to the drive letter.
         Command line should be:
            dledit <drive letter>
      */
      pszOptions       = NULL;
      pszDriveLetter   = argv[1];
      pszNTDevice      = NULL;

      TCHAR szNtDeviceName[MAX_PATH];
      TCHAR szDriveLetter[3];

      /*
         Command-line argument for the drive letter could be in one of two
         formats:  C:\ or C:.  We normalize this to C: for QueryDosDevice.
      */
      szDriveLetter[0] = pszDriveLetter[0];
      szDriveLetter[1] = _T(':');
      szDriveLetter[2] = _T('\0');

      if (QueryDosDevice (szDriveLetter, szNtDeviceName, MAX_PATH))
      {
         _tprintf(_T("%s is mapped to %s\n"), szDriveLetter, szNtDeviceName);
      }
      else
      {
         switch (GetLastError())
         {
         case ERROR_FILE_NOT_FOUND:
            _tprintf(_T("%s is not in use\n"), pszDriveLetter);
            break;

         default:
            _tprintf(_T("error %lu: couldn't get mapping for %s\n"),
                     GetLastError(), pszDriveLetter);
         }
      }

   } /* End if show drive letter mapping */

   else if (2 == argc && !lstrcmpi (argv[1], _T("-a")))
   {
      /*
         User wants to show all mappings of drive letters to their respective
         devices.  Command line should be:
            dledit -a
      */
      pszOptions       = argv[1];
      pszDriveLetter   = NULL;
      pszNTDevice      = NULL;

      /*
         Largest possible array of drive strings is 26 drive letters
         times 4 chars per drive letter plus 1 for the terminating NULL
       */
      const DWORD MAX_DRIVE_STRING_LENGTH = 26 * 4 + 1;

      TCHAR szDriveStrings[MAX_DRIVE_STRING_LENGTH];
      DWORD cchDriveStrings;


      /*
         Get a list of all current drive letters, then for each, print the
         mapping.  Drive letters are returned in the form
         A:\<NULL>B:\<NULL>C:\<NULL>...<NULL>.
      */
      cchDriveStrings = GetLogicalDriveStrings (MAX_DRIVE_STRING_LENGTH, szDriveStrings);
      if ( (0 != cchDriveStrings) && (cchDriveStrings <= MAX_DRIVE_STRING_LENGTH) )
      {
         TCHAR * pszCurrentDriveLetter;
         TCHAR szNtDeviceName[MAX_PATH];
         TCHAR szDriveLetter[3];


         _tprintf(_T("Drive     Device\n")
                  _T("-----     ------\n"));

         pszCurrentDriveLetter = szDriveStrings;
         while (_T('\0') != *pszCurrentDriveLetter)
         {
            /* QueryDosDevice requires drive letters in the format X: */
            szDriveLetter[0] = pszCurrentDriveLetter[0];
            szDriveLetter[1] = _T(':');
            szDriveLetter[2] = _T('\0');

            if (QueryDosDevice (szDriveLetter, szNtDeviceName, MAX_PATH))
               _tprintf(_T("%-10s%s\n"), szDriveLetter, szNtDeviceName);

            pszCurrentDriveLetter += 4; /* size of X:\<NULL> */
         }
      }
      else
      {
         _tprintf(_T("couldn't list drive letters and their devices\n"));
      }
   }  /* End if show all drive letter mappings */

   else
   {
      /* User has selected an invalid operation--display help. */
      PrintHelp (argv[0]);
   }
}


/*-----------------------------------------------------------------------------
PrintHelp( IN pszAppName )

Parameters
   pszAppName
      The name of the executable.  Used in displaying the help for this app.

Prints the command-line usage help.
-----------------------------------------------------------------------------*/
static void PrintHelp (LPCTSTR pszAppName)
{
   _tprintf(_T("Adds, removes, queries drive letter assignments\n\n"));
   _tprintf(_T("usage: %s <Drive Letter> <NT device name>     add a drive letter\n"),
          pszAppName);
   _tprintf(_T("       %s -t <Drive Letter> <NT device name>  add a temporary drive letter\n"),
       pszAppName);

   _tprintf(_T("       %s -r <Drive Letter>                   remove a drive letter\n"),
          pszAppName);
   _tprintf(_T("       %s <Drive Letter>                      show mapping for drive letter\n"),
       pszAppName);
   _tprintf(_T("       %s -a                                  show all mappings\n\n"),
       pszAppName);


   _tprintf(_T("example: %s e:\\ \\Device\\CdRom0\n"), pszAppName);
   _tprintf(_T("         %s -r e:\\\n"), pszAppName);
}


/*-----------------------------------------------------------------------------
AssignPersistentDriveLetter (IN pszDriveLetter, IN pszDeviceName)

Description:
   Creates a persistent drive letter that refers to a specified device. This
   drive letter will remain even when the system is restarted.

Parameters:
   pszDriveLetter
      The new drive letter to create.  Must be in the form X: or X:\

   pszDeviceName
      The NT device name to which the drive letter will be assigned.

Return Value:
   Returns TRUE if the drive letter was added, or FALSE if it wasn't.
-----------------------------------------------------------------------------*/
BOOL WINAPI AssignPersistentDriveLetter (
   LPCTSTR pszDriveLetter,
   LPCTSTR pszDeviceName)
{
   BOOL fResult;
   TCHAR szUniqueVolumeName[MAX_PATH];
   TCHAR szDriveLetterAndSlash[4];
   TCHAR szDriveLetter[3];

   /*
      Make sure we are passed a drive letter and a device name.  lstrlen
      is useful because it will return zero if the pointer points to memory
      that can't be read or a string that causes an invalid page fault before
      the terminating NULL.
   */
   if (0 == lstrlen(pszDriveLetter) ||
       0 == lstrlen(pszDeviceName)  ||
       !IsDriveLetter (pszDriveLetter))
   {
      SetLastError (ERROR_INVALID_PARAMETER);
      return FALSE;
   }

   /*
      GetVolumeNameForVolumeMountPoint, SetVolumeMountPoint, and
      DeleteVolumeMountPoint require drive letters to have a trailing backslash.
      However, DefineDosDevice and QueryDosDevice require that the trailing
      backslash be absent.  So, we'll set up the following variables:

       szDriveLetterAndSlash     for the mount point APIs
       szDriveLetter             for DefineDosDevice
   */
   szDriveLetterAndSlash[0] = pszDriveLetter[0];
   szDriveLetterAndSlash[1] = _T(':');
   szDriveLetterAndSlash[2] = _T('\\');
   szDriveLetterAndSlash[3] = _T('\0');

   szDriveLetter[0] = szDriveLetterAndSlash[0];
   szDriveLetter[1] = _T(':');
   szDriveLetter[2] = _T('\0');

   /*
      Determine if the drive letter is currently in use.  If so, return the
      error to the caller.  NOTE:  we temporarily reuse szUniqueVolumeName
      instead of allocating a large array for this one call.
   */
   fResult = QueryDosDevice (szDriveLetter, szUniqueVolumeName, MAX_PATH);
   if (fResult)
   {
      SetLastError (PRIV_ERROR_DRIVE_LETTER_IN_USE);
      return FALSE;
   }


   /*
      To map a persistent drive letter, we must make sure that the target
      device is one of the following:

         A recognized partition and is not hidden
         A dynamic volume
         A non-partitionable device such as CD-ROM

      Start by using the drive letter as a symbolic link to the device.  Then,
      open the device to gather the information necessary to determine if the
      drive can have a persistent drive letter.
   */
   fResult = DefineDosDevice (DDD_RAW_TARGET_PATH, szDriveLetter, pszDeviceName);
   if (fResult)
   {
      HANDLE       hDevice;
      DWORD        dwBytesReturned;
      BOOL         fResult;
      TCHAR        szDriveName[7]; // holds \\.\X: plus NULL.

      VOLUME_GET_GPT_ATTRIBUTES_INFORMATION volinfo;
      PARTITION_INFORMATION                 partinfo;


      _tcsncpy_s (szDriveName, _countof(szDriveName), _T("\\\\.\\"), _TRUNCATE);
      _tcsncat_s (szDriveName, _countof(szDriveName), szDriveLetter, _TRUNCATE);

      hDevice = CreateFile (szDriveName, GENERIC_READ,
                            FILE_SHARE_READ|FILE_SHARE_WRITE,
                            NULL, OPEN_EXISTING, 0, NULL);

      if (INVALID_HANDLE_VALUE != hDevice)
      {
         /*
            See if drive is partitionable and retrieve the partition type.
            If the device doesn't have a partition, note it by setting the
            partition type to unused.
         */
         fResult = DeviceIoControl (hDevice, IOCTL_DISK_GET_PARTITION_INFO,
                                    NULL, 0,
                                    &partinfo, sizeof(partinfo),
                                    &dwBytesReturned, NULL);
         if (!fResult)
         {
            partinfo.PartitionType = PARTITION_ENTRY_UNUSED;
         }

         /*
            On Windows XP, partition entries on GUID Partition Table drives
            have an attribute that determines whether partitions are hidden.
            Therefore, we must check this bit on the target partition.

            If we're running on Windows 2000, there are no GPT drives, so
            set flags to none.  This is important for the check for hidden
            partitions later.
         */
         if (IsWindowsXP_orLater())
         {
            fResult = DeviceIoControl (hDevice, IOCTL_VOLUME_GET_GPT_ATTRIBUTES,
                                       NULL, 0,
                                       &volinfo, sizeof(volinfo),
                                       &dwBytesReturned, NULL);
            if (!fResult)
               volinfo.GptAttributes = 0;
         }
         else
         {
            volinfo.GptAttributes = 0;
         }
         CloseHandle (hDevice);
      }
      else
      {
         /*
            Remove the drive letter symbolic link we created, let caller know
            we couldn't open the drive.
         */
         DefineDosDevice (DDD_RAW_TARGET_PATH|DDD_REMOVE_DEFINITION|
                          DDD_EXACT_MATCH_ON_REMOVE, szDriveLetter,
                          pszDeviceName);
         return (FALSE);
      }

      /*
         Now, make sure drive meets requirements for receiving a persistent
         drive letter.

         Note: on Windows XP, partitions that were hidden when the system
         booted are not assigned a unique volume name.  They will not be given
         one until they are marked as not hidden and the system rebooted.
         Therefore, we cannot create a mount point hidden partitions.

         On Windows 2000, hidden partitions are assigned unique volume names
         and so can have persistent drive letters.  However, we will not allow
         that behavior in this tool as hidden partitions should not be assigned
         drive letters.
      */
      if (IsPartitionHidden (partinfo.PartitionType, volinfo.GptAttributes))
      {
         // remove the drive letter we created, let caller know what happened.
         DefineDosDevice (DDD_RAW_TARGET_PATH|DDD_REMOVE_DEFINITION|
                           DDD_EXACT_MATCH_ON_REMOVE, szDriveLetter,
                           pszDeviceName);

         SetLastError (PRIV_ERROR_PARTITION_HIDDEN);
         return (FALSE);
      }

      /*
         Verify that the drive letter must refer to a recognized partition,
         a dynamic volume, or a non-partitionable device such as CD-ROM.
      */
      if (IsRecognizedPartition(partinfo.PartitionType) ||
          PARTITION_LDM == partinfo.PartitionType ||
          PARTITION_ENTRY_UNUSED == partinfo.PartitionType)
      {
         /*
            Now add the drive letter by calling on the volume mount manager.
            Once we have the unique volume name that the new drive letter
            will point to, delete the symbolic link because the Mount Manager
            allows only one reference to a device at a time (the new one to
            be added).
         */
         fResult = GetVolumeNameForVolumeMountPoint (szDriveLetterAndSlash,
                                                     szUniqueVolumeName,
                                                     MAX_PATH);

         DefineDosDevice (DDD_RAW_TARGET_PATH|DDD_REMOVE_DEFINITION|
                          DDD_EXACT_MATCH_ON_REMOVE, szDriveLetter,
                          pszDeviceName);

         if (fResult)
            fResult = SetVolumeMountPoint (szDriveLetterAndSlash,
                                           szUniqueVolumeName);

         return (fResult);
      }
      else
      {
         /*
            Device doesn't meet the criteria for persistent drive letter.
            Remove the drive letter symbolic link we created.
         */
         DefineDosDevice (DDD_RAW_TARGET_PATH|DDD_REMOVE_DEFINITION|
                          DDD_EXACT_MATCH_ON_REMOVE, szDriveLetter,
                          pszDeviceName);

         SetLastError (PRIV_ERROR_PARTITION_NOT_RECOGNIZED);
         return (FALSE);
      }

   }  /* end if DeviceIoControl to map symbolic link*/
   else
      return (FALSE);
}


/*-----------------------------------------------------------------------------
RemovePersistentDriveLetter( IN pszDriveLetter)

Description:
   Removes a drive letter that was created by AssignPersistentDriveLetter().

Parameters:
   pszDriveLetter
      The drive letter to remove.  Must be in the format X: or X:\

Return Value:
   Returns TRUE if the drive letter was removed, or FALSE if it wasn't.
-----------------------------------------------------------------------------*/
BOOL WINAPI RemovePersistentDriveLetter (LPCTSTR pszDriveLetter)
{
   BOOL fResult;
   TCHAR szDriveLetterAndSlash[4];

   /* Make sure we have a drive letter.   */
   if (0 == lstrlen(pszDriveLetter) || !IsDriveLetter (pszDriveLetter))
   {
      SetLastError (ERROR_INVALID_PARAMETER);
      fResult = FALSE;
   }

   /*
      pszDriveLetter could be in the format X: or X:\.  DeleteVolumeMountPoint
      requires X:\, so add a trailing backslash.
   */
   szDriveLetterAndSlash[0] = pszDriveLetter[0];
   szDriveLetterAndSlash[1] = _T(':');
   szDriveLetterAndSlash[2] = _T('\\');
   szDriveLetterAndSlash[3] = _T('\0');

   fResult = DeleteVolumeMountPoint (szDriveLetterAndSlash);
   return (fResult);
}


/*-----------------------------------------------------------------------------
AssignTemporaryDriveLetter( IN pszDriveLetter, IN pszDeviceName)

Description:
   Creates a temporary drive letter that refers to a specified device.  This
   drive letter will exist only until the system is shut down or restarted.

Parameters:
   pszDriveLetter
      The new drive letter to create.  Must be in the form X: or X:\

   pszDeviceName
      The NT device name to which the drive letter will be assigned.

Return Value:
   Returns TRUE if the temporary drive letter was assigned, or FALSE if it
   could not be.

Notes:
   A temporary drive letter is just a symbolic link.  It can be removed at any
   time by deleting the symbolic link. If it exists when the system is shut
   down or restarted, it will be removed automatically.

   AssignTemporaryDriveLetter requires device to be present.
-----------------------------------------------------------------------------*/
BOOL WINAPI AssignTemporaryDriveLetter (LPCTSTR pszDriveLetter, LPCTSTR pszDeviceName)
{
   TCHAR szDevice[MAX_PATH];
   TCHAR szDriveLetter[3];
   BOOL  fResult;

   /* Verify that caller passed a drive letter and device name. */
   if (0 == lstrlen(pszDriveLetter) ||
       0 == lstrlen(pszDeviceName)  ||
       !IsDriveLetter (pszDriveLetter))
   {
      SetLastError (ERROR_INVALID_PARAMETER);
      return (FALSE);
   }

   /*
      Make sure the drive letter isn't already in use.  If not in use,
      create the symbolic link to establish the temporary drive letter.

      pszDriveLetter could be in the format X: or X:\; QueryDosDevice and
      DefineDosDevice need X:
   */
   szDriveLetter[0] = pszDriveLetter[0];
   szDriveLetter[1] = _T(':');
   szDriveLetter[2] = _T('\0');

   if (!QueryDosDevice (szDriveLetter, szDevice, MAX_PATH))
   {
      /*
         If we can create the symbolic link, verify that it points to a real
         device.  If not, remove the link and return an error.  CreateFile sets
         the last error code to ERROR_FILE_NOT_FOUND.
      */
      fResult = DefineDosDevice (DDD_RAW_TARGET_PATH, szDriveLetter, pszDeviceName);
      if (fResult)
      {
         HANDLE hDevice;
         TCHAR  szDriveName[7]; // holds \\.\X: plus NULL.

         _tcsncpy_s (szDriveName, _countof(szDriveName), _T("\\\\.\\"), _TRUNCATE);
         _tcsncat_s (szDriveName, _countof(szDriveName), szDriveLetter, _TRUNCATE);

         hDevice = CreateFile (szDriveName, GENERIC_READ,
                               FILE_SHARE_READ|FILE_SHARE_WRITE,
                               NULL, OPEN_EXISTING, 0, NULL);

         if (INVALID_HANDLE_VALUE != hDevice)
         {
            fResult = TRUE;
            CloseHandle (hDevice);
         }
         else
         {
            DefineDosDevice (DDD_RAW_TARGET_PATH|DDD_REMOVE_DEFINITION|
                             DDD_EXACT_MATCH_ON_REMOVE, szDriveLetter,
                             pszDeviceName);
            fResult = FALSE;
         }
      }
   }
   else
   {
      SetLastError (PRIV_ERROR_DRIVE_LETTER_IN_USE);
      fResult = FALSE;
   }

   return (fResult);
}


/*-----------------------------------------------------------------------------
RemoveTemporaryDriveLetter( IN pszDriveLetter)

Description:
   Removes a drive letter that was created by AssignTemporaryDriveLetter().

Parameters:
   pszDriveLetter
      The drive letter to remove.  Must be in the format X: or X:\

Return Value:
   Returns TRUE if the drive letter was removed, or FALSE if it wasn't.
-----------------------------------------------------------------------------*/
BOOL WINAPI RemoveTemporaryDriveLetter (LPCTSTR pszDriveLetter)
{
   BOOL fResult;
   TCHAR szDriveLetter[3];
   TCHAR szDeviceName[MAX_PATH];

   /* Verify that caller passed a drive letter and device name. */

   if (0 == lstrlen(pszDriveLetter) || !IsDriveLetter (pszDriveLetter))
   {
      SetLastError (ERROR_INVALID_PARAMETER);
      fResult = FALSE;

   }

   /*
      pszDriveLetter could be in the format X: or X:\; DefineDosDevice
      needs X:
   */
   szDriveLetter[0] = pszDriveLetter[0];
   szDriveLetter[1] = _T(':');
   szDriveLetter[2] = _T('\0');

   fResult = QueryDosDevice (szDriveLetter, szDeviceName, MAX_PATH);
   if (fResult)
   {
      fResult = DefineDosDevice (DDD_RAW_TARGET_PATH|DDD_REMOVE_DEFINITION|
                                 DDD_EXACT_MATCH_ON_REMOVE, szDriveLetter,
                                 szDeviceName);
   }

   return (fResult);
}


/*-----------------------------------------------------------------------------
IsPartitionHidden( IN partitionType, IN partitionAttribs)

Description:
   Determines if the partition hosting a volume is hidden or not.

Parameters:
   partitionType
      Specifies the partitition type; this value is obtained from
      IOCTL_DISK_GET_PARTITION_INFO or IOCTL_DISK_GET_DRIVE_GEOMETRY.
      A partition type that has bit 0x10 set is hidden.

   partitionAttribs
      Specifies the attributes of a GUID Partition Type (GPT)-style partition
      entry.  This value is obtained from IOCTL_VOLUME_GET_GPT_ATTRIBUTES
      or IOCTL_DISK_GET_DRIVE_GEOMETRY_EX.  A GPT partition that has
      GPT_BASIC_DATA_ATTRIBUTE_HIDDEN set is hidden.

Return Value:
   Returns TRUE if the partition is hidden or FALSE if it is not.

Notes:
   On Windows XP and Windows Server 2003, partitions that were hidden when
   the system booted are not assigned a unique volume name.  They will not
   be given one until they are marked as not hidden and the system rebooted.
   Therefore, we cannot create a mount point hidden partitions.

   On Windows 2000, hidden partitions are assigned unique volume names
   and so can have persistent drive letters.  However, we will not allow
   that behavior in this tool as hidden partitions should not be assigned
   drive letters.
-----------------------------------------------------------------------------*/
BOOL WINAPI IsPartitionHidden (BYTE partitionType, ULONGLONG partitionAttribs)
{
   if ((partitionAttribs & GPT_BASIC_DATA_ATTRIBUTE_HIDDEN) ||
       ((partitionType & HIDDEN_PARTITION_FLAG) &&
        IsRecognizedPartition (partitionType & ~HIDDEN_PARTITION_FLAG)))
   {
      return TRUE;
   }
   else
   {
      return FALSE;
   }
}


/*-----------------------------------------------------------------------------
IsDriveLetter( IN pszDriveLetter)

Description:
   Verifies string passed in is of the form X: or X:\ where X is a letter.

Parameters:
   pszDriveLetter
      A null terminated string.

Return Value:
   TRUE if the string is of the form X: or X:\ where X is a letter.  X
   may be upper-case or lower-case.  If the string isn't of this from,
   returns FALSE.
-----------------------------------------------------------------------------*/
BOOL WINAPI IsDriveLetter (LPCTSTR pszDriveLetter)
{
    /*
       format must be: X:<null> or X:\<NULL> where X is a letter.  lstrlen will
       return 0 if inaccessible pointer is supplied, or if reading bytes of
       string causes an access violation before reaching a terminating NULL.
    */
    if ( (0 < lstrlen (pszDriveLetter))      &&
         (IsCharAlpha (pszDriveLetter[0]))   &&
         (_T(':')   == pszDriveLetter[1])    &&
         ((_T('\0') == pszDriveLetter[2]) ||
          ((_T('\\') == pszDriveLetter[2]) && (_T('\0') == pszDriveLetter[3])))   
       )
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


/*-----------------------------------------------------------------------------
IsWindowsXP_orLater()

Description:
   Determines if the currently-running version of Windows is Windows XP or
   Windows Server 2003 or later.

Return Value:
   Returns TRUE if the currently-running system is Windows XP or Windows 2003
   Server or later systems. Returns FALSE otherwise.
-----------------------------------------------------------------------------*/
BOOL WINAPI IsWindowsXP_orLater (void)
{
   OSVERSIONINFOEX osvi;
   ULONGLONG       comparisonMask;

   ZeroMemory (&osvi, sizeof(osvi));
   osvi.dwOSVersionInfoSize = sizeof(osvi);
   osvi.dwMajorVersion = 5;
   osvi.dwMinorVersion = 1;

   comparisonMask = 0;
   comparisonMask = VerSetConditionMask (comparisonMask, VER_MAJORVERSION, VER_GREATER_EQUAL);
   comparisonMask = VerSetConditionMask (comparisonMask, VER_MINORVERSION, VER_GREATER_EQUAL);

   return (VerifyVersionInfo (&osvi, VER_MAJORVERSION|VER_MINORVERSION, comparisonMask));
}



