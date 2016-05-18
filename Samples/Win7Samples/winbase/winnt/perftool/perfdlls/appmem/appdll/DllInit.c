/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (C) 1995 - 2000.  Microsoft Corporation.  All rights reserved.

Module Name:

    dllinit.c

Abstract:

    This module contians the DLL attach/detach event entry point for
    the Application memory usage performance DLL.

    When the dll is opened by the calling application. The initialization
    function (DllProcessAttach) opens or creates the shared memory file
    that is used to communicate data from the application to the performance
    monitor extensible counter DLL. This shared memory file is used by all
    applications that have been compiled to provide the memory statistics.

    The shared memory file is formatted by the first application to
    create it. The format of the memory is a data header (at the base
    of the address space) followed by an array of APPMEM_INSTANCE structures.
    The APPMEM_INSTANCE structures are managed as two linked lists: a
    Free List and an InUse List. The pointers to the first entry in each
    list are maintained in the data header structure. As applications are
    started, the first item in the Free list is moved to the InUse List
    and as applications finish with the data block, their entries are moved
    back to the Head of the Free List. All operations on the data in
    this shared memory file are protected by a mutex such that only
    one process can read or write to the file at a time.

    Note that references are by offset from the base of the memory
    block not absolute virtual addresses. This is because the same
    shared memory file may be mapped to different addresses in
    different processes making a virtual address valid within the
    context of a single process only.

--*/

#include <windows.h>
#include "appmema.h"

HANDLE ThisDLLHandle       = NULL;
HANDLE hAppMemSharedMemory = NULL;      // Handle of counter Shared Memory
PAPPMEM_DATA_HEADER pDataHeader = NULL; // pointer to header of shared mem
PAPPMEM_INSTANCE    pAppData    = NULL; // pointer to the app data for this app

static
BOOL
TrimProcessName(
    LPCWSTR  wcszLongName,
    LPWSTR  wcszShortName
)
/*++

Description:

    strips the leading directory path and trailing file extension from the
    input path string to return the base filename.

Arguments:

    wcszLongName    pointer to a Wide Character string containing the
                    process's image file path name

    wcszShortName   buffer to receive the "short" version of the image
                    file name, i.e. the base file name with no extension or
                    directory path.

NOTE: the buffer size is not checked!!! The caller must guarantee sufficient
    space for the conversion.

Return Value:

    TRUE if name trimmed successfully
    FALSE if error

--*/
{
   DWORD   dwLength;
   LPWSTR  pSlash;
   LPWSTR  pPeriod;
   LPWSTR  pThisChar;
   LPWSTR  pSrc, pDest;
   WORD    wThisChar;

   // validate the input arguments
   if ((wcszLongName == NULL) || (wcszShortName == 0))
   {
      return FALSE;
   }

   // get the length of the string passed in
   dwLength = lstrlenW(wcszLongName);

   if (* wcszLongName > 0)
   {   // some name has been passed in
      pSlash    =
      pPeriod   =
      pThisChar = (LPWSTR) wcszLongName;
      wThisChar = 0;

      //
      //  go from beginning to end and find last backslash and
      //  last period in name
      //

      while (* pThisChar != 0)
      { // go until null
         if (* pThisChar == L'\\')
         {
            pSlash = pThisChar;
         }
         else if (* pThisChar == L'.')
         {
            pPeriod = pThisChar;
         }
         pThisChar ++;    // point to next char
         wThisChar ++;
         if ((DWORD) wThisChar >= dwLength)
         {
            break;
         }
      }

      // if pPeriod is still pointing to the beginning of the
      // string, then no period was found

      if (pPeriod == (LPWSTR) wcszLongName)
      {
         pPeriod = pThisChar; // set to end of string;
      }
      else
      {
         // if a period was found, then see if the extension is
         // .EXE, if so leave it, if not, then use end of string
         // (i.e. include extension in name)

         // using CompareString instead of lstrcmpi due to locale issues
         if (CompareStringW( LOCALE_INVARIANT,
             NORM_IGNORECASE,
             pPeriod, MAX_PATH,
             L".EXE", 5 ) != CSTR_EQUAL)
         {
            pPeriod = pThisChar;
         }
      }

      if (pSlash != wcszLongName)
      {
         pSlash ++; // point to first char past slash
      }

      // copy characters between period (or end of string) and
      // slash (or start of string) to make image name

      pSrc  = pSlash;
      pDest = wcszShortName;

      while (pSrc < pPeriod)
      {
         * pDest ++ = * pSrc ++;
      }
      * pDest = 0;
   }
   return TRUE;
}

static
BOOL
DllProcessAttach(
    IN  HANDLE DllHandle
)
/*++

Description:

    Initializes the interface to the performance counters DLL by
    opening the Shared Memory file used to communicate statistics
    from the application to the counter DLL. If the Shared memory
    file does not exist, it is created, formatted and initialized.
    If the file has already been created and formatted, then the
    next available APPMEM_INSTANCE entry is moved from the free list
    to the InUse list and the corresponding pointer is saved for
    subsequent use by this application

--*/
{
   LONG    status;
   TCHAR   szMappedObject[] = SHARED_MEMORY_OBJECT_NAME;
   WCHAR   szLongName[MAX_PATH];

   // save this DLL handle
   ThisDLLHandle = DllHandle;

   // disable thread attach & detach calls to save the overhead
   // since we don't care about them.
   DisableThreadLibraryCalls(DllHandle);

   // open & initialize shared memory file
   SetLastError(ERROR_SUCCESS);   // just to clear it out

   // open/create shared memory used by the application to pass performance values
   status = GetSharedMemoryDataHeader(
                                     & hAppMemSharedMemory, & pDataHeader,
                                     FALSE); // read/write access is required
   // here the memory block should be initialized and ready for use
   if (status == ERROR_SUCCESS)
   {
      if (pDataHeader != NULL && pDataHeader->dwFirstFreeOffset != 0)
      {
         // then there are blocks left so get the next free
         pAppData = FIRST_FREE(pDataHeader);
         // update free list to make next item the first in list
         pDataHeader->dwFirstFreeOffset  = pAppData->dwOffsetOfNext;

         // insert the new item into the head of the in use list
         pAppData->dwOffsetOfNext        = pDataHeader->dwFirstInUseOffset;
         pDataHeader->dwFirstInUseOffset = (DWORD) ((LPBYTE) pAppData - (LPBYTE) pDataHeader);

         // now initialize this instance's data
         pAppData->dwProcessId           = GetCurrentProcessId();
         pAppData->hProcessHeap          = NULL;
         pAppData->dwApplicationBytes    = 0;
         pAppData->dwAllocCalls          = 0;
         pAppData->dwReAllocCalls        = 0;
         pAppData->dwFreeCalls           = 0;
         pAppData->dwReserved1           = 0;
         pAppData->dwReserved2           = 0;

         GetModuleFileNameW(NULL, szLongName, MAX_PATH);
         TrimProcessName(szLongName, pAppData->wcszInstanceName);

         pDataHeader->dwInstanceCount ++;    // increment count
      }
      else
      {
         // no more free slots left
      }
   }
   else
   {
      // unable to open shared memory file
      // even though this is an error we should return true so as to
      // not abort the application. No performance data will be
      // collected though.
   }
   return TRUE;
}

static
BOOL
DllProcessDetach(
    IN  HANDLE DllHandle
)
{
   PAPPMEM_INSTANCE pPrevItem;

   // remove instance for this app
   if ((pAppData != NULL) && (pDataHeader != NULL))
   {
      // lock memory block
      // zero the fields out first
      pAppData->dwProcessId        = 0;
      pAppData->hProcessHeap       = NULL;
      pAppData->dwApplicationBytes = 0;
      pAppData->dwAllocCalls       = 0;
      pAppData->dwReAllocCalls     = 0;
      pAppData->dwFreeCalls        = 0;
      memset(& pAppData->wcszInstanceName[0], 0, (MAX_SIZEOF_INSTANCE_NAME * sizeof (WCHAR)));
      pAppData->dwReserved1        = 0;
      pAppData->dwReserved2        = 0;
      // move from in use (busy) list back to the free list
      if ((pDataHeader->dwFirstFreeOffset != 0) && (pDataHeader->dwFirstInUseOffset != 0))
      {
         // find previous item in busy list
         if (FIRST_INUSE(pDataHeader) != pAppData)
         {
            // not the first so walk down the list
            pPrevItem = FIRST_INUSE(pDataHeader);
            while (APPMEM_INST(pDataHeader, pPrevItem->dwOffsetOfNext) != pAppData)
            {
               pPrevItem = APPMEM_INST(pDataHeader, pPrevItem->dwOffsetOfNext);
               if (pPrevItem->dwOffsetOfNext == 0) break; // end of list
            }
            if (APPMEM_INST(pDataHeader, pPrevItem->dwOffsetOfNext) == pAppData)
            {
               APPMEM_INST(pDataHeader, pPrevItem->dwOffsetOfNext)->dwOffsetOfNext = pAppData->dwOffsetOfNext;
            }
            else
            {
               // it was never in the busy list (?!?)
            }
         }
         else
         {
            // this is the first in the list so update it
            pDataHeader->dwFirstInUseOffset = pAppData->dwOffsetOfNext;
         }
         // here, pAppData has been removed from the InUse list and now
         // it must be inserted back at the beginning of the free list
         pAppData->dwOffsetOfNext       = pDataHeader->dwFirstFreeOffset;
         pDataHeader->dwFirstFreeOffset = (DWORD)((LPBYTE) pAppData - (LPBYTE) pDataHeader);
      }
   }

   // decrement instance counter
   if (pDataHeader != NULL)
       pDataHeader->dwInstanceCount --;    // decrement count

   // close shared memory file handle

   if (hAppMemSharedMemory != NULL) CloseHandle(hAppMemSharedMemory);

   // clear pointers
   hAppMemSharedMemory = NULL;
   pDataHeader = NULL;
   pAppData = NULL;

   return TRUE;
}

BOOL
__stdcall
DllMain(
    IN HANDLE DLLHandle,
    IN DWORD  Reason,
    IN LPVOID ReservedAndUnused
)
{
   ReservedAndUnused;

   switch (Reason)
   {
   case DLL_PROCESS_ATTACH:
      return DllProcessAttach(DLLHandle);

   case DLL_PROCESS_DETACH:
      return DllProcessDetach(DLLHandle);

   case DLL_THREAD_ATTACH:
   case DLL_THREAD_DETACH:
      return TRUE;
   }
   return TRUE;
}

