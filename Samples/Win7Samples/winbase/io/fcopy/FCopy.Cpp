/*---------------------------------------------------------------------------
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (C) 1999 - 2000 Microsoft Corporation.  All rights reserved.

fcopy.cpp

This program demonstrates how to use file mappings to implement a file-copy
program.  It can copy any size file on Windows NT.

The size of the file mapping view is a multiple of the system's
allocation size. With relatively large views, this program
runs faster than if it used many small views.  The size of the view
can be adjusted up or down by changing the ALLOCATION_MULTIPLIER constant.
The only recommendation is that the view size must be no more than can fit
into the process's address space.  

Note:  Supports 64-bit file systems.
---------------------------------------------------------------------------*/

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <stdio.h>


#if defined (DEBUG)
#define DEBUG_PRINT(X) printf(X)
#else
#define DEBUG_PRINT(X)
#endif

#include <crtdbg.h>

// maximum view size
DWORD dwMaxViewSize ;    

// multiplying the system allocation size by the following constant
// determines the maximum view size
const WORD ALLOCATION_MULTIPLIER = 1 ;

const int   SUCCESS = 0;   /* for return value from main() */
const int   FAILURE = 1;   /* for return value from main() */

/*---------------------------------------------------------------------------
main (argc, argv)

The main program.  Takes the command line arguments, copies the source file
to the destination file.

Parameters
   argc
      Count of command-line arguments, including the name of the program.
   argv
      Array of pointers to strings that contain individual command-line
      arguments.

Returns
   Zero if program executed successfully, non-zero otherwise.
---------------------------------------------------------------------------*/
int main (int argc, char **argv)
{
   int   fResult = FAILURE;

   SYSTEM_INFO siSystemInfo ;
   DWORD dwMaxViewSize ;

   ULARGE_INTEGER liSrcFileSize,
                  liBytesRemaining,
                  liMapSize,
                  liOffset;

   HANDLE hSrcFile    = INVALID_HANDLE_VALUE,
          hDstFile    = INVALID_HANDLE_VALUE,
          hSrcMap     = 0,
          hDstMap     = 0;

   BYTE * pSrc = 0,
        * pDst = 0;

   char * pszSrcFileName = 0,
        * pszDstFileName = 0;

   if (argc != 3)
   {
      printf("usage: fcopy <srcfile> <dstfile>\n");
      return (FAILURE);
   }

   pszSrcFileName = argv[argc-2];  // Src is second to last argument
   pszDstFileName = argv[argc-1];  // Dst is the last argument

   // Obtain the system's allocation granularity, then multiply it by an 
   // arbitrary factor to obtain the maximum view size
   GetSystemInfo(&siSystemInfo);
   dwMaxViewSize = siSystemInfo.dwAllocationGranularity * ALLOCATION_MULTIPLIER;

   /*
      Steps to open and access a file's contents:
         1) Open the file,
         2) Create a mapping of the file,
         3) Map a view of the file.

      This yields a pointer to the file's contents, which can then be used
      to access the file, just as if it's contents were in a memory buffer.

      For the source file, open and map it as read only; for the destination
      file, open and map it as read-write.  We allow other processes to read
      the source file while we're copying it, but do not allow access to the
      destination file since we're writing it.
   */

   // Open the source and destination files
   hSrcFile = CreateFile (pszSrcFileName, GENERIC_READ, FILE_SHARE_READ,
                          0, OPEN_EXISTING, 0, 0);
   if (INVALID_HANDLE_VALUE == hSrcFile)
   {
      printf("fcopy: couldn't open source file.\n");
      goto DONE;
   }

   hDstFile = CreateFile (pszDstFileName, GENERIC_READ|GENERIC_WRITE, 0,
                          0, CREATE_ALWAYS, 0, 0);
   if (INVALID_HANDLE_VALUE == hDstFile)
   {
      printf("fcopy: couldn't create destination file.\n");
      goto DONE;
   }


   // Need source file's size to know how big to make the destination mapping.
   liSrcFileSize.LowPart = GetFileSize(hSrcFile, &liSrcFileSize.HighPart);
   if ( (-1 == liSrcFileSize.LowPart) && (GetLastError() != NO_ERROR) )
   {
      DEBUG_PRINT("couldn't get size of source file.\n");
      goto DONE;
   }

   /*
      Special case:  If the source file is zero bytes, we don't map it because
      there's no need to and CreateFileMapping cannot map a zero-length file.
      But since we've created the destination, we've successfully "copied" the
      source.
   */
   if (0 == liSrcFileSize.QuadPart)
   {
      fResult = SUCCESS;
      goto DONE;
   }


   /*
      Map the source and destination files.  A mapping size of zero means the
      whole file will be mapped.
   */
   hSrcMap = CreateFileMapping (hSrcFile, 0, PAGE_READONLY, 0, 0, 0);
   if (!hSrcMap)
   {
      DEBUG_PRINT("couldn't map source file\n");
      goto DONE;
   }

   hDstMap = CreateFileMapping (hDstFile, 0, PAGE_READWRITE,
                                liSrcFileSize.HighPart,
                                liSrcFileSize.LowPart, 0);
   if (!hDstMap)
   {
      DEBUG_PRINT("couldn't map destination file.\n");
      goto DONE;
   }


   /*
      Now that we have the source and destination mapping objects, map views
      of the source and destination files, and do the file copy.

      To minimize the amount of memory consumed for large files and make it
      possible to copy files that couldn't be mapped into our virtual address
      space entirely (those over 2GB), we limit the source and destination
      views to the smaller of the file size or a specified maximum view size
      (dwMaxViewSize, which is ALLOCATION_MULTIPLIER times the system's 
      allocation size).

      If the file is smaller than the max view size, we'll just map and copy
      it.  Otherwise, we'll map a portion of the file, copy it, then map the
      next portion, copy it, etc. until the entire file is copied.

      MAP_SIZE is 32 bits because MapViewOfFile requires a 32-bit value for
      the size of the view.  This makes sense because a Win32 process's
      address space is 4GB, of which only 2GB (2^31) bytes may be used by the
      process.  However, for the sake of making 64-bit arithmetic work below
      for file offets, we need to make sure that all 64 bits of liMapSize
      are initialized correctly.

      Note structured exception handling is used in case a MapViewOfFile call
      failed.  That should never happen in this program, but in case it does,
      we should handle it.  Since the possibility is so remote, it is faster
      to handle the exception when it occurs rather than test for failure in
      the loop.
   */
   __try
   {
      liBytesRemaining.QuadPart = liSrcFileSize.QuadPart;

      // stan bug fix
      liMapSize.QuadPart        = dwMaxViewSize;

      // Make sure that the arithmetic below is correct during debugging.
      _ASSERT(liMapSize.HighPart == 0);

      do
      {
         liMapSize.QuadPart = min(liBytesRemaining.QuadPart, liMapSize.QuadPart);

         liOffset.QuadPart = liSrcFileSize.QuadPart - liBytesRemaining.QuadPart;

         pSrc = (BYTE *)MapViewOfFile (hSrcMap, FILE_MAP_READ, liOffset.HighPart,
                                       liOffset.LowPart, liMapSize.LowPart);

         pDst = (BYTE *)MapViewOfFile (hDstMap, FILE_MAP_WRITE, liOffset.HighPart,
                                       liOffset.LowPart, liMapSize.LowPart);

         CopyMemory (pDst, pSrc, liMapSize.LowPart);

         UnmapViewOfFile (pSrc);
         UnmapViewOfFile (pDst);

         liBytesRemaining.QuadPart -= liMapSize.QuadPart;
      }
      while (liBytesRemaining.QuadPart > 0);

      fResult = SUCCESS;
   }
   __except (EXCEPTION_EXECUTE_HANDLER)
   {
      /*
         Only way we should get here is if a MapViewOfFile failed.  That
         shouldn't happen, but in case it does clean up the resources.
      */
      if (pSrc)
         UnmapViewOfFile (pSrc);

      if (pDst)
         UnmapViewOfFile (pDst);
   }


DONE:
   /* Clean up all outstanding resources.  Note views are already unmapped. */
   if (hDstMap)
      CloseHandle (hDstMap);

   if (hDstFile != INVALID_HANDLE_VALUE)
      CloseHandle (hDstFile);

   if (hSrcMap)
      CloseHandle (hSrcMap);

   if (hSrcFile != INVALID_HANDLE_VALUE)
      CloseHandle (hSrcFile);


   // Report to user only if a problem occurred.
   if (fResult != SUCCESS)
   {
      printf("fcopy: copying failed.\n");
      DeleteFile (pszDstFileName);
   }


   return (fResult);
}

