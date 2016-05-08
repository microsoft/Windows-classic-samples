/*----------------------------------------------------------------------------
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (C) 1998 - 2000.  Microsoft Corporation.  All rights reserved.

unmount.c

This file implements a command-line utility that removes mount points on
drives on Windows 2000 and later.

Command-line syntax:
   unmount <directory>
----------------------------------------------------------------------------*/

#include <windows.h>
#include <stdio.h>

static void PrintHelp (void);

int main (int argc, char **argv)
{
   char *pszMountToDelete;
   char *pszDirName = NULL;
   int   cchDirLen;


   // Make sure user suppled proper command-line arguments.
   if (argc != 2)
   {
      PrintHelp();
      return (-1);
   }

   /*
      Make sure there are enough command-line arguments before
      dereferencing argv[1].
   */
   pszMountToDelete = argv[1];

   /*
      Add trailing backslashes to the mount point directory name
      because volume mount point APIs require them.

      If the directory name doesn't already have a trailing backslash, we
      just copy it to a new buffer and add the trailing backslash.
   */
   cchDirLen = lstrlen(pszMountToDelete);
   if (pszMountToDelete[cchDirLen - 1] != '\\')
   {
      pszDirName = new char[cchDirLen + 2]; // +2 for backslash and NULL
      strcpy_s (pszDirName, cchDirLen+2, pszMountToDelete);
      pszDirName[cchDirLen] = '\\';
      pszDirName[cchDirLen+1] = '\0';
   }
   else
      pszDirName = (char *)pszMountToDelete;  // Cast away const

   // Delete the mount point here.
   if (DeleteVolumeMountPoint (pszDirName))
      printf("unmounted %s\n", pszMountToDelete);
   else
      printf("couldn't unmount %s\n", pszMountToDelete);


   // Free pszDirName if it was allocated from free store
   if (pszDirName != pszMountToDelete)
      delete[] pszDirName;


   return (0);
}


/*-----------------------------------------------------------------------------
PrintHelp( )

Notes
   Prints usage notes for the command line syntax.  Called if the user doesn't
   specify the command line correctly.
-----------------------------------------------------------------------------*/
void PrintHelp (void)
{
   printf ("usage: unmount <directory>\n");
}
