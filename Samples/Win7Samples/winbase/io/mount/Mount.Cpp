/*----------------------------------------------------------------------------
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (C) 1998 - 2000.  Microsoft Corporation.  All rights reserved.


mount.c

This file implements a command-line utility that creates mount points for
drives on Windows 2000 and later.

Command-line syntax:
   mount [-o] <drive> <directory>
      -o   overwrite existing mount point on <directory>
----------------------------------------------------------------------------*/

#include <windows.h>
#include <stdio.h>

static void PrintHelp (void);
static BOOL CreateMountPoint (LPCSTR pszDriveToMount, LPCSTR pszDirToMount);


#if defined (DEBUG)
   static void DebugPrint (LPCSTR pszMsg, DWORD dwErr);
   #define DEBUG_PRINT(pszMsg, dwErr) DebugPrint(pszMsg, dwErr)
#else
   #define DEBUG_PRINT(pszMsg, dwErr) NULL
#endif


int main (int argc, char **argv)
{
   bool  bOverwriteMount;
   char *pszMountDir;
   char *pszDriveToMount;
   char *pszOptions;

   // Make sure user has supplied required number of command-line arguments.
   if (argc != 3 && argc != 4)
   {
      PrintHelp ();
      return (-1);
   }

   /*
      Since we have at least 3 args, we can initialize the pointers to them.
      We use pointers to explicitly refer to the arguments to make the rest
      of the code more understandable.
   */
   pszMountDir     = argv[argc-1];
   pszDriveToMount = argv[argc-2];
   pszOptions      = argv[argc-3];

   // See if "-o" is present in command line.  It must be the second argument.
   bOverwriteMount = (argc == 4 && !lstrcmpi (pszOptions, "-o"));

   /*
      If bOverwriteMount != TRUE (i.e. user wants to keep an existing
      mount point), then need to check destination to see if it exists and
      if it is a mount point. If so, don't create a volume mount point on it.

      The way to tell if a directory is a mount point is to:

       1) Call FindFirstFile().
       2) If WIN32_FIND_DATA.dwFileAttributes contains
          FILE_ATTRIBUTE_REPARSE_POINT see if WIN32_FIND_DATA.dwReserved0
          is IO_REPARSE_TAG_MOUNT_POINT.
       3) If so, then the directory is a mount point.
   */
   if (!bOverwriteMount)
   {
      WIN32_FIND_DATA fileInfo;
      HANDLE          hFind;

      hFind = FindFirstFile (pszMountDir, &fileInfo);
      if (INVALID_HANDLE_VALUE != hFind)
      {
         FindClose(hFind);  // Don't need the find handle anymore.

         /*
            If the destination is a mount point, tell user we're not going
            to replace it, and then exit.
         */
         if ((fileInfo.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) &&
             (fileInfo.dwReserved0 == IO_REPARSE_TAG_MOUNT_POINT))
         {
            printf("%s is already a mount point; it will not be replaced\n",
                   pszMountDir);
            return (0);
         }
      }
      /*
         If hFind == INVALID_HANDLE_VALUE here, we didn't find the directory
         to be the mount point. We could exit here, but we won't.  We'll
         just keep going because CreateMountPoint will fail the creation of
         the mount point on the non-existant directory.
      */
   }

   /*
      Create the mount point.  Report whether we succeeded or failed.
   */
   if (CreateMountPoint (pszDriveToMount, pszMountDir))
      printf("mounted %s to %s\n", pszDriveToMount, pszMountDir);
   else
   {
      printf("couldn't mount %s to %s\n", pszDriveToMount, pszMountDir);
      DEBUG_PRINT ("CreateMountPoint failed with error", GetLastError());
   }

   return (0);
}


/*-----------------------------------------------------------------------------
CreateMountPoint( pszDriveToMount, pszDirToMount )

Parameters
   pszDriveToMount
      The drive that will be associated with the mount point directory.

   pszDirToMount
      The location that pszDriveToMount is to be mounted.  This must be an
      empty directory.  It can also be a current mount point; if it is, then
      the existing mount point will automatically be unmounted by
      SetVolumeMountPoint.

Return Value
   Returns TRUE if successful, or FALSE otherwwise.

Notes
   Since GetVolumeNameForVolumeMountPoint and SetVolumeMountPoint require
   trailing backslashes, we'll add them if necessary.
-----------------------------------------------------------------------------*/
BOOL CreateMountPoint (LPCSTR pszDriveToMount, LPCSTR pszDirToMount)
{
   const int VOL_NAME_MAX = 80;
   BOOL fResult;
   char szUniqueVolumeName[VOL_NAME_MAX];

   char szDriveName[4];
   char *pszDirName = NULL;
   int   cchDirLen;

   __try
   {
      /*
         Add trailing backslashes to drive letter and mount point directory name
         because volume mount point APIs require them.

         Since drive letters are of the format C:\ or C:, we know that the max
         drive letter string is 4 chars long.  We can thus use array addressing
         to do a faster equivalent of:

            lstrcpyn (szDriveName, pszDriveToMount, 3);
            lstrcat (szDriveName, "\\");

         If the directory name doesn't already have a trailing backslash, we
         just copy it to a new buffer and add the trailing backslash.
      */
      __try
      {
         szDriveName[0] = pszDriveToMount[0];
         szDriveName[1] = pszDriveToMount[1];
         szDriveName[2] = '\\';
         szDriveName[3] = '\0';

         // now the directory name
         cchDirLen = lstrlen(pszDirToMount);
         if (pszDirToMount[cchDirLen - 1] != '\\')
         {
            pszDirName = new char[cchDirLen + 2]; // +2 for backslash and NULL
            strcpy_s (pszDirName, cchDirLen+2, pszDirToMount);
            pszDirName[cchDirLen] = '\\';
            pszDirName[cchDirLen+1] = '\0';
         }
         else
            pszDirName = (char *)pszDirToMount;  // Cast away const
      }
      __except(EXCEPTION_EXECUTE_HANDLER)
      {
         fResult = FALSE;
         __leave;
      }

      // Create the mount point...
      fResult = GetVolumeNameForVolumeMountPoint (szDriveName,
                                                  szUniqueVolumeName,
                                                  VOL_NAME_MAX);
      if (!fResult)
      {
         DEBUG_PRINT("GetVolumeNameForVolumeMountPoint failed with error",
                     GetLastError());
         __leave;
      }

      fResult = SetVolumeMountPoint (pszDirName, szUniqueVolumeName);
      if (!fResult)
      {
         DEBUG_PRINT("SetVolumeMountPoint failed with error", GetLastError());
         __leave;
      }

   }
   __finally
   {
      // Free pszDirName if it was allocated from free store
      if (pszDirName != pszDirToMount)
         delete[] pszDirName;
   }

   return (fResult);
}


/*-----------------------------------------------------------------------------
PrintHelp( )

Notes
   Prints usage notes for the command line syntax.  Called if the user doesn't
   specify the command line correctly.
-----------------------------------------------------------------------------*/
void PrintHelp (void)
{
   printf ("usage: mount [-o] <drive> <directory>\n"
           "\t-o   overwrite existing mount point on <directory>\n");
}


/*-----------------------------------------------------------------------------
DebugPrint( pszMsg, dwErr )

Parameters
   pszMsg
      The string to be printed to STDOUT
   dwErr
      The error code; usually obtained from GetLastError.  If dwErr is zero,
      then no error code is added to the error string.  If dwErr is non-zero,
      then the error code will be printed in the error string.
-----------------------------------------------------------------------------*/
void DebugPrint (LPCSTR pszMsg, DWORD dwErr)
{
   if (dwErr)
      printf("%s: %lu\n", pszMsg, dwErr);
   else
      printf("%s\n", pszMsg);
}

