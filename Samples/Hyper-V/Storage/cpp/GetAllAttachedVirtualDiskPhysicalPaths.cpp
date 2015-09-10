// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <strsafe.h>
#include <virtdisk.h>

#include "Storage.h"

//
// This sample demonstrates how to get the list of all the loopback mounted virtual disks.
//

DWORD
SampleGetAllAttachedVirtualDiskPhysicalPaths(
    )
{
    LPWSTR  pathList;
    LPWSTR  pathListBuffer;
    size_t  nextPathListSize;
    DWORD   opStatus;
    ULONG   pathListSizeInBytes;
    size_t  pathListSizeRemaining;
    HRESULT stringLengthResult;

    pathListBuffer = NULL;
    pathListSizeInBytes = 0;

    do
    {
        //
        // Determine the size actually required.
        //

        opStatus = GetAllAttachedVirtualDiskPhysicalPaths(&pathListSizeInBytes,
                                                          pathListBuffer);
        if (opStatus == ERROR_SUCCESS)
        {
            break;
        }
           
        if (opStatus != ERROR_INSUFFICIENT_BUFFER)
        {
            goto Cleanup;
        }
           
        if (pathListBuffer != NULL)
        {
            free(pathListBuffer);
        }
        
        //
        // Allocate a large enough buffer.
        //

        pathListBuffer = (LPWSTR)malloc(pathListSizeInBytes);
        if (pathListBuffer == NULL)
        {
            opStatus = ERROR_OUTOFMEMORY;
            goto Cleanup;
        } 
        
    } while (opStatus == ERROR_INSUFFICIENT_BUFFER);
    
    if (pathListBuffer == NULL || pathListBuffer[0] == NULL) 
    {
        // There are no loopback mounted virtual disks.
        wprintf(L"There are no loopback mounted virtual disks.\n");
        goto Cleanup;
    }
       
    //
    // The pathList is a MULTI_SZ.
    //
        
    pathList = pathListBuffer;
    pathListSizeRemaining = (size_t) pathListSizeInBytes;
    
    while ((pathListSizeRemaining >= sizeof(pathList[0])) && (*pathList != 0))
    {        
        stringLengthResult = StringCbLengthW(pathList, 
                                             pathListSizeRemaining,
                                             &nextPathListSize);
        
        if (FAILED(stringLengthResult))
        {
            goto Cleanup;
        }
        
        wprintf(L"Path = '%s'\n", pathList);
        
        nextPathListSize += sizeof(pathList[0]);
        pathList = pathList + (nextPathListSize / sizeof(pathList[0]));
        pathListSizeRemaining -= nextPathListSize;
    }
    
Cleanup:
    if (opStatus == ERROR_SUCCESS)
    {
        wprintf(L"success\n");
    }
    else
    {
        wprintf(L"error = %u\n", opStatus);
    }

    if (pathListBuffer != NULL)
    {
        free(pathListBuffer);
    }

    return opStatus;
}
