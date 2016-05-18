/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (C) 1995 - 2000.  Microsoft Corporation.  All rights reserved.

    sharemem.c

    shared memory functions used by both application and performance
    dlls

--*/
//
//  Include Files
//
#include <windows.h>
#include <string.h>
#include <winperf.h>
#include "..\inc\appmemi.h"     // definitions shared with the app. dll
#include "memctrs.h"            // error message definition

LONG
_stdcall
GetSharedMemoryDataHeader(
    IN  HANDLE              * phAppMemSharedMemory,
    IN  PAPPMEM_DATA_HEADER * pHeader,
    IN  BOOL                  bReadOnlyAccess
)
{
   LONG             Status, MemStatus;
   TCHAR            szMappedObject[] = SHARED_MEMORY_OBJECT_NAME;
   PAPPMEM_INSTANCE pLastItem;
   PAPPMEM_INSTANCE pAppData;
   DWORD            dwInstance;

   SetLastError(ERROR_SUCCESS);   // just to clear it out

   * phAppMemSharedMemory = CreateFileMapping(
                                              (HANDLE) INVALID_HANDLE_VALUE,
                                              NULL,                      // no security
                                              PAGE_READWRITE,            // to allow read & write access
                                              0,
                                              SHARED_MEMORY_OBJECT_SIZE, // file size
                                              szMappedObject);           // object name

   MemStatus = GetLastError();    // to see if this is the first opening

   // return error if unsuccessful
   if (* phAppMemSharedMemory == NULL)
   {
      Status = APPMEM_OPEN_FILE_MAPPING_ERROR;
      // this is fatal, if we can't get data then there's no
      // point in continuing.
   }
   else
   {
      if (MemStatus != ERROR_ALREADY_EXISTS)
      {
         // this is the first access to the file so initialize the
         // instance count
         * pHeader = (PAPPMEM_DATA_HEADER) MapViewOfFile(
                                                      * phAppMemSharedMemory,  // shared mem handle
                                                      FILE_MAP_WRITE,          // access desired
                                                      0,                       // starting offset
                                                      0,
                                                      0);                      // map the entire object
         if (* pHeader != NULL)
         {
            // if here, then pdwInstanceCount should be valid
            // so initialize the shared memory structure
            // clear memory block
            memset(* pHeader, 0, SHARED_MEMORY_OBJECT_SIZE);

            // initialize application data header
            (* pHeader)->dwInstanceCount    = 0;    // no instances
            (* pHeader)->dwFirstInUseOffset = 0;    // no busy items yet
            (* pHeader)->dwFirstFreeOffset  = (DWORD) ((LPBYTE) & (* pHeader)[1] - (LPBYTE) * pHeader);

            pLastItem = FIRST_FREE((* pHeader));

            // set pointers in each item of block
            pAppData = pLastItem + 1;
            for (dwInstance = (SHARED_MEMORY_ITEM_COUNT - 1); dwInstance > 0; dwInstance --)
            {
               pLastItem->dwOffsetOfNext = (DWORD)((LPBYTE) pAppData - (LPBYTE)(* pHeader));
               pLastItem = pAppData;
               pAppData ++;
            }
            pAppData->dwOffsetOfNext = 0; // end of list
            Status = ERROR_SUCCESS;
         }
         else
         {
            Status = APPMEM_UNABLE_MAP_VIEW_OF_FILE;
         }
      }
      else
      {
         // the status is ERROR_ALREADY_EXISTS which is successful
         Status = ERROR_SUCCESS;
      }
      // see if Read Only access is required
      if (Status == ERROR_SUCCESS)
      {
         // by now the shared memory has already been initialized so
         // we if we don't need write access any more or if it has
         // already been opened, then open with the desired access
         * pHeader = (PAPPMEM_DATA_HEADER) MapViewOfFile(
                         * phAppMemSharedMemory,                             // shared mem handle
                         (bReadOnlyAccess ? FILE_MAP_READ : FILE_MAP_WRITE), // access desired
                         0,                                                  // starting offset
                         0,
                         0);                                                 // map the entire object
         if (* pHeader == NULL)
         {
            Status = APPMEM_UNABLE_MAP_VIEW_OF_FILE;
            // this is fatal, if we can't get data then there's no
            // point in continuing.
         }
         else
         {
            Status = ERROR_SUCCESS;
         }
      }
   }
   return Status;
}
