//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
// Simple Wininet Cache APIs usage application (entries enumeration).
//


#include "CacheEnumerate.h"


// Global vars
DWORD g_dwEnumFilter        = URLCACHE_FIND_DEFAULT_FILTER;     // default filter.
DWORD g_dwAction            = 0;                                // action type var.
LPWSTR g_lpszSearchPattern  = NULL;                             // Search pattern
LPWSTR g_lpszURL            = NULL;                             // Store user input URL


int
__cdecl
wmain(
    int argc,
    LPWSTR *argv
)
{

    // Parse out the command line
    // and set user options
    ParseArguments(argc, argv);

    switch (g_dwAction)
    {
        // Enumerate all cache containers and delete option.
    case(CACHESAMPLE_ACTION_ENUM_ALL | CACHESAMPLE_ACTION_DELETE):
        ClearCache();
        break;

        // Get entry details on a particular URL
    case(CACHESAMPLE_ACTION_DETAIL):
        GetEntryDetail();
        break;

        // Enumerate entries from a particular containers.
    default:
        EnumerateCache();
        break;
    }

    return 0;

}


VOID
PrintUsageBlock(
    VOID
)
/*++

Routine Description:
    This routine is used to print out the usage and option
    messages.

Arguments:
    None.

Return Value:
    None.

--*/
{
    wprintf(L"Usage: CacheEnumerate [-?] | [[-a | -c | -h | -k] -d] | [-e <URL>] \n\n");
    wprintf(L"Flag Semantics: \n");
    wprintf(L"-a : Enumerate entries in all fixed containers\n");
    wprintf(L"-c : Enumerate entries in the content container\n");
    wprintf(L"-d : Delete entries option \n");
    wprintf(L"-e : Get details of a entry for a specific URL \n");
    wprintf(L"-h : Enumerate entries in the history container\n");
    wprintf(L"-k : Enumerate entries in the cookie container\n");
    wprintf(L"-? : Display usage info.\n");
    wprintf(L"\n");
    wprintf(L"E.g.\n");
    wprintf(L"\t CacheEnumerate.exe -a - Enumerate all entries in all the fixed containers\n");
    wprintf(L"\t CacheEnumerate.exe -e http://www.microsoft.com/ - get detail on an entry associated a URL\n");
    wprintf(L"\t CacheEnumerate.exe -h -d - Enumerate all the entries in the ");
    wprintf(L"history container and delete each found entry.\n");
    wprintf(L"\t CacheEnumerate.exe -a -d - Enumerate all entries in the fixed containers");
    wprintf(L"and delete each found entry.\n");

}

VOID
ParseArguments(
    int argc,
    LPWSTR *argv
)

/*++

Routine Description:
    This routine is used to Parse command line arguments. Flags are
    case sensitive.

Arguments:
    argc - Number of arguments
    argv - Pointer to the argument vector

Return Value:
    None.

--*/
{

    int i;
    BOOL bError = TRUE;

    // If there are either no option or more than
    // two options, print the usage block
    // and exit.
    if ((argc == 1) || (argc > 3))
    {
        PrintUsageBlock();
        exit(1);
    }

    for (i = 1; i < argc; ++i)
    {
        // Make sure all option starts with '-'.
        // Otherwise, print the usage block
        // and exit.
        if (wcsncmp(argv[i], L"-", 1))
        {
            printf("Invalid switch %ws\n", argv[i]);
            goto done;
        }

        switch (argv[i][1])
        {
            // Enumerate entries in all the fix containers.
        case L'a':
            // This must be the 1st and only option.
            if (i > 1)
            {
                goto done;
            }

            g_dwAction |= CACHESAMPLE_ACTION_ENUM_ALL;
            break;


            // Enumerate entries in the content container.
        case L'c':
            // This must be the 1st option.
            if (i > 1)
            {
                goto done;
            }

            g_dwAction |= CACHESAMPLE_ACTION_ENUM_CONTENT;
            break;

            // Delete option for found entries.
        case L'd':
            // Don't allow -d option as the first option.
            // If it is the first option, print usage block
            // and exit.
            if (i == 1)
            {
                goto done;
            }

            g_dwAction |= CACHESAMPLE_ACTION_DELETE;
            break;

            // Get entry detail on a particular cached URL.
        case L'e':
            // This option must be the first option.
            // And it must have a URL param ONLY.
            // Otherwise, print usage and exit.
            if ((i != 1) || (argc != 3))
            {
                goto done;
            }

            // Grab the URL param.
            g_lpszURL = argv[++i];

            // Check again to be conservative.
            if (!*g_lpszURL)
            {
                wprintf(L"You must specify an URL for detail search!.");
                goto done;
            }

            g_dwAction |= CACHESAMPLE_ACTION_DETAIL;
            break;

            // Enumerate history entries only.
        case L'h':
            // This must be the 1st option.
            if (i > 1)
            {
                goto done;
            }

            g_dwAction |= CACHESAMPLE_ACTION_ENUM_HISTORY;
            break;

            // Enumerate entries in the cookie container.
        case L'k':
            // This must be the 1st option.
            if (i > 1)
            {
                goto done;
            }

            g_dwAction |= CACHESAMPLE_ACTION_ENUM_COOKIE;
            break;

            // Display usage block.
        case L'?':
            // This must be the 1st option.
            if (i > 1)
            {
                goto done;
            }

            PrintUsageBlock();
            exit(0);

            // Unknown option print the usage block.
        default:
            wprintf(L"Unknown option: %s\n\n", argv[i]);
            goto done;

        }

        bError = FALSE;
    }

done:

    // If we have any errors, print the usage block
    // and exit.
    if (bError)
    {
        PrintUsageBlock();
        exit(1);
    }

}

VOID
ClearCache(
    VOID
)
/*++

Routine Description:
    This routine is the call that starts the clearing of WinInet cache store.
    It calls two other routines to clear the cache stores of both cache groups
    and non grouped entries.  The correct way to clear the cache of its content
    is to first delete all the cache groups and their associated cache entries.
    Then, the rest of the non grouped cache entries are deleted.

Arguments:
    None.

Return Value:
    None.

--*/
{

    wprintf(L"\t*** Deleting all entries in cache. ***\n");

    // First delete all cache groups.
    DeleteAllCacheGroups();

    // Delete the rest of the non grouped entries.
    EnumerateCache();

}

VOID
DeleteAllCacheGroups(
    VOID
)
/*++

Routine Description:
    This routine is used to delete all the cache groups in the WinInet cache
    store.  It utilize the "find first" and "find next" group enumeration
    routine to enumerate all the groups.  The delete group routine was
    called with CACHEGROUP_FLAG_FLUSHURL_ONDELETE flag to have all the
    cache entries associated with the particular group flushed/deleted.

Arguments:
    None.

Return Value:
    None.

--*/
{
    BOOL bDone          = FALSE;
    BOOL bRet           = FALSE;
    DWORD dwErr         = 0;
    HANDLE hEnumGroup   = NULL;
    GROUPID gID         = 0;

    // 1st call FindFirstUrlCacheGroup to
    // get the enumerate handle.
    hEnumGroup = FindFirstUrlCacheGroup(0,                      // Reserved. Must be zero.
                                        CACHEGROUP_SEARCH_ALL,  // Search all groups.
                                        NULL,                   // Reserved. Must be NULL.
                                        0,                      // Reserved. Must be zero.
                                        &gID,                   // Pointer to the ID of the first cache group that matches the search criteria.
                                        NULL);                  // Reserved. Must be NULL.


    if (!hEnumGroup)
    {

        dwErr = GetLastError();

        switch (dwErr)
        {
			// msdn states ERROR_NO_MORE_FILES which is wrong. Doc bug!
        case ERROR_NO_MORE_ITEMS:
        case ERROR_FILE_NOT_FOUND:
            // No more items and we are done.
            wprintf(L"There are no more cache groups.\n");
            break;

        default:
            // Something else happened.
            // This is usally not expected.
            LogInetError(dwErr, L"FindFirstUrlCacheGroup");
            break;
        }

        goto cleanup;

    }

    do
    {
        // At this point we are sure we have
        // a valid hEnum from FindFirstUrlCacheGroup or
        // FindNextUrlCacheGroup
        // Delete the cache group and flush
        // all entries tag to this group.
		wprintf(L"\t Deleting cache group ID: %d\n", gID);
        if (!DeleteUrlCacheGroup(gID,                                // ID of the cache group to be released.
                                 CACHEGROUP_FLAG_FLUSHURL_ONDELETE,  // delete all of the cache entries associated with this group, unless the entry belongs to another group.
                                 NULL))                              // Reserved. Must be NULL
        {
            // Log out any errors on delete.
            // Additional error processing may be needed
            // deppending on condition.
            LogInetError(GetLastError(), L"DeleteUrlCacheGroup");
        }

        // Now we will go thru FindNextUrlCacheGroup
        // and enumerate the rest of the groups.
        bRet = FindNextUrlCacheGroup(hEnumGroup,        // Enum handle returned by FindFirstUrlCacheGroup
                                     &gID,              // Pointer to a variable that receives the cache group identifier.
                                     NULL);             // Reserved. Must be NULL.

        if (!bRet)
        {
            dwErr = GetLastError();

            switch (dwErr)
            {
            case ERROR_NO_MORE_ITEMS:
            case ERROR_FILE_NOT_FOUND:
                // No more items and we are done.
                wprintf(L"There are no more cache groups.\n");
                bDone = TRUE;
                break;

            default:
                // Something else happened.
                // This is usually not expected.
                LogInetError(dwErr, L"FindNextUrlCacheGroup");
                goto cleanup;
            }
        }

    }
    while (!bDone);

cleanup:

    // Make sure we cleanup this enum handle.
    if (hEnumGroup)
    {
        // FindCloseUrlCache is used to close
        // all Cache enumerate function.
        FindCloseUrlCache(hEnumGroup);
    }

}

VOID
EnumerateCache(
    VOID
)
/*++

Routine Description:
    This routine is used to enumerate the WinInet cache store. It utilizes
    the Ex versions of both the "find first" and "find next" pair of cache
    enumeration routines.  It uses the default all inclusive flag for
    cache entry enumeration.

Arguments:
    None.

Return Value:
    None.

--*/
{
    BOOL bDone              = FALSE;
    BOOL bRet               = FALSE;
    HANDLE hEnum            = NULL;
    DWORD dwErr             = 0;
    DWORD cbCacheInfoSize   = 0;

    LPINTERNET_CACHE_ENTRY_INFO lpCacheEntryInfo = NULL;

    // decide which search pattern to use based on the
    // options set.
    switch (g_dwAction & CACHESAMPLE_ACTION_ENUM_MASK)
    {
    case CACHESAMPLE_ACTION_ENUM_ALL:
        g_lpszSearchPattern = ALL;
        break;

    case CACHESAMPLE_ACTION_ENUM_COOKIE:
        g_lpszSearchPattern = COOKIE;
        break;

    case CACHESAMPLE_ACTION_ENUM_HISTORY:
        g_lpszSearchPattern = HISTORY;
        break;

    case CACHESAMPLE_ACTION_ENUM_CONTENT:
        g_lpszSearchPattern = CONTENT;
        break;

	// Unknown search pattern.
	default:
		wprintf(L"Unknown search pattern: 0x%x\n\n", g_dwAction & CACHESAMPLE_ACTION_ENUM_MASK);
		goto cleanup;
    }

    // 1st call FindFirstUrlCacheEntryEx with a NULL pointer
    // to get the buffer size needed.
    hEnum = FindFirstUrlCacheEntryEx(g_lpszSearchPattern,   // Search pattern
                                     0,                     // Flags - Must be zero.
                                     g_dwEnumFilter,        // Filters
                                     0,                     // Group ID
                                     NULL,                  // NULL buffer
                                     &cbCacheInfoSize,      // buffer size
                                     NULL,                  // Reserved.
                                     NULL,                  // Reserved.
                                     NULL);                 // Reserved.

    if (!hEnum)
    {
        dwErr = GetLastError();

        switch (dwErr)
        {
            // We are done no need to enumerate further.
        case ERROR_NO_MORE_ITEMS:
            wprintf(L"There are no more entries.\n");
            goto cleanup;

            // Insufficient buffer.  Allocate the correct buffer.
        case ERROR_INSUFFICIENT_BUFFER:
            lpCacheEntryInfo = (LPINTERNET_CACHE_ENTRY_INFO)
                               Malloc(cbCacheInfoSize);

            // Make sure buffer allocation is successful.
            if (!lpCacheEntryInfo)
            {
                LogSysError(ERROR_NOT_ENOUGH_MEMORY, L"Allocating buffer");
                goto cleanup;
            }

            // zero memory the structure
            ZeroMemory(lpCacheEntryInfo, sizeof(INTERNET_CACHE_ENTRY_INFO));
            lpCacheEntryInfo->dwStructSize = cbCacheInfoSize;
            break;

            // Some other errors encountered.  This is treated as fatal.
        default:
            LogInetError(dwErr, L"FindFirstUrlCacheEntryEx");
            goto cleanup;

        }

    }

    // Retry FindFirstUrlCacheEntryEx for enumeration.
    hEnum = FindFirstUrlCacheEntryEx(g_lpszSearchPattern,   // Search pattern
                                     0,                     // Flags - Must be zero.
                                     g_dwEnumFilter,        // Filters
                                     0,                     // Group ID
                                     lpCacheEntryInfo,      // Buffer
                                     &cbCacheInfoSize,      // Buffer size
                                     NULL,                  // Reserved.
                                     NULL,                  // Reserved.
                                     NULL);                 // Reserved.

    // We shouldn't have any error for FindFirstCall.
    // If we do, we should exit.
    if (!hEnum)
    {
        LogInetError(GetLastError(), L"FindFirstUrlCacheEntryEx");
        goto cleanup;
    }


    // We will now go through the rest of the entries.
    // in the cache store.
    do
    {

        // We have a successful enum handle.
        // Plus, we have the returned entry either
        // from FindFirstUrlCacheEntryEx or from
        // FindNextUrlCacheEntryEx.  We will delete
        // the entry if the CACHESAMPLE_ACTION_DELETE
        // flag is set; otherwise, simply display
        // the source URL.
        if (lpCacheEntryInfo->lpszSourceUrlName)
        {
            wprintf(L"The cache entry's source URL is: %s\n", lpCacheEntryInfo->lpszSourceUrlName);

            // Delete the entry if delete flag is set.
            if (g_dwAction & CACHESAMPLE_ACTION_DELETE)
            {
                wprintf(L"Deleting the cache entry with URL: %s\n",
                        lpCacheEntryInfo->lpszSourceUrlName);

                // Check the success status of delete.
                // Addition error handling may be needed
                // if situation warrant it.
                if (!DeleteUrlCacheEntry(lpCacheEntryInfo->lpszSourceUrlName))
                {
                    dwErr = GetLastError();

                    // This error is returned when a delete is
                    // attempt on an entry that has been locked.
                    // In this case, the entry will be marked for
                    // delete in the next process startup in WinInet.
                    if (ERROR_SHARING_VIOLATION == dwErr)
                    {
                        wprintf(L"The entry is locked and will be deleted in the next process startup.\n");
                    }
                    else
                    {
                        LogInetError(dwErr, L"DeleteUrlCacheEntry");
                    }
                }
            }
        }

        // Now we will go through the rest
        // of the entries.
        bRet = FindNextUrlCacheEntryEx(hEnum,               // Enum handle returned from FindFirst.
                                       lpCacheEntryInfo,    // Buffer.
                                       &cbCacheInfoSize,    // Buffer size.
                                       NULL,                // Reserved.
                                       NULL,                // Reserved.
                                       NULL);               // Reserved.

        if (!bRet)
        {
            dwErr = GetLastError();
            switch (dwErr)
            {
            case ERROR_NO_MORE_ITEMS:
                // We are done.
                wprintf(L"There are no more entries.\n");
                bDone = TRUE;
                break;

            case ERROR_INSUFFICIENT_BUFFER:
                //  Free the old buffer first
                Free(lpCacheEntryInfo);

                lpCacheEntryInfo = (LPINTERNET_CACHE_ENTRY_INFO)
                                   Malloc(cbCacheInfoSize);

                // Make sure buffer allocation is successful
                if (!lpCacheEntryInfo)
                {
                    LogSysError(ERROR_NOT_ENOUGH_MEMORY, L"Allocating buffer");
                    goto cleanup;
                }

                // zero memory the structure
                ZeroMemory(lpCacheEntryInfo, sizeof(INTERNET_CACHE_ENTRY_INFO));
                lpCacheEntryInfo->dwStructSize = cbCacheInfoSize;
                continue;

                // Something else happened.
                // This is considered non recoverable.
            default:
                LogInetError(dwErr, L"FindNextUrlCacheEntryEx");
                goto cleanup;

            }

        }

    }
    while (!bDone);

cleanup:

    // Make sure we free our buffer
    if (lpCacheEntryInfo)
    {
        Free(lpCacheEntryInfo);
    }

    // Make sure we clean up this handle.
    if (hEnum)
    {
        // FindCloseUrlCache is used to close
        // all Cache enumerate function.
        FindCloseUrlCache(hEnum);
    }

}

VOID
GetEntryDetail(
    VOID
)
/*++

Routine Description:
    This routine is used to retrieve cache entry detail on a specific URL.  The
    advantage of this routine is that if one knows the specific URL for a cache
    entry, the detail of that particular entry is easily retrieve this call
    rather than using the "find first" and "find next" pair of cache apis to
    enum to the correct entry.

Arguments:
    lpszURL - Source URL for a particular cache entry.

Return Value:
    None.

--*/
{
    DWORD dwErr         = 0;
    DWORD cbEntrySize   = 0;

    LPINTERNET_CACHE_ENTRY_INFO lpCacheEntryInfo = NULL;

    // 1st call to GetUrlCacheEntryInfo with NULL buffer
    // to get the buffer size needed.
    if (!GetUrlCacheEntryInfo(g_lpszURL,         // The URL to the entry
                              NULL,              // Buffer
                              &cbEntrySize))     // Buffer size
    {
        dwErr = GetLastError();

        switch (dwErr)
        {
        case ERROR_FILE_NOT_FOUND:
            // No entry found with the specified URL.
            // We're done.
            wprintf(L"There is no cache entry associated with the requested URL : %s\n",
                    g_lpszURL);
            goto cleanup;

        case ERROR_INSUFFICIENT_BUFFER:
            // allocate buffer with the correct size.
            lpCacheEntryInfo = (LPINTERNET_CACHE_ENTRY_INFO)Malloc(cbEntrySize);

            if (!lpCacheEntryInfo)
            {
                LogSysError(ERROR_NOT_ENOUGH_MEMORY, L"Allocating buffer");
                goto cleanup;
            }

            // zero memory the structure.
            ZeroMemory(lpCacheEntryInfo, sizeof(INTERNET_CACHE_ENTRY_INFO));
            lpCacheEntryInfo->dwStructSize = cbEntrySize;
            break;

        default:
            // We shouldn't have any other errors
            LogInetError(dwErr, L"GetUrlCacheEntryInfo");
            goto cleanup;

        }
    }

    // Called GetUrlCacheEntryInfo with the correct
    // buffer size.
    if (!GetUrlCacheEntryInfo(g_lpszURL,             // The URL to the entry.
                              lpCacheEntryInfo,      // Buffer.
                              &cbEntrySize))         // Buffer size.
    {
        // If the entry is not in th cache.  The
        // previous call will resulted in ERROR_FILE_NOT_FOUND
        // even with a NULL buffer.
        // We shouldn't have any other errors.
        LogInetError(GetLastError(), L"GetUrlCacheEntrInfo");

    }
    else
    {
        // Got the entry info.
        // dump out some details for demonstration purpose.
        wprintf(L"The returned cache entry contains the following detail:\n");
        wprintf(L"The source URL is: %s\n", lpCacheEntryInfo->lpszSourceUrlName);
        wprintf(L"The local file name is: %s\n", lpCacheEntryInfo->lpszLocalFileName);

    }

cleanup:

    // Make sure we free our buffer.
    if (lpCacheEntryInfo)
    {
        Free(lpCacheEntryInfo);
    }

}

VOID *
Malloc(
    size_t size
)
/*++

Routine Description:
      Wrapper around malloc handling errors and alloc counts.

Arguments:
      size - number of bytes to allocate

Return Value:
      VOID pointer to block of memory allocated

--*/
{
    VOID *ptr;

#ifdef DEBUG
    InterlockedIncrement(&allocCount);
#endif

    ptr = malloc(size);

    if (!ptr)
    {
        fprintf(stderr, "Out of Memory\n");
        exit(1);
    }

    return ptr;
}



VOID
Free(
    VOID *memblock
)
/*++

Routine Description:
      Wrapper around free to keep up free counts.

Arguments:
      memblock - pointer to memblock to be freed

Return Value:
      None.

--*/
{
#ifdef DEBUG
    InterlockedIncrement(&freeCount);
#endif

    free(memblock);
}

VOID
LogInetError(
	DWORD err,
	LPCWSTR str
)
/*++

Routine Description:
     This routine is used to log WinInet errors in human readable form.

Arguments:
     err - Error number obtained from GetLastError()
     str - String pointer holding caller-context information

Return Value:
    None.

--*/
{
    DWORD dwResult;
    LPWSTR msgBuffer = (LPWSTR)Malloc(ERR_MSG_LEN * sizeof(WCHAR));
    // Make sure buffer allocation is successful.
    if (!msgBuffer)
    {
        fprintf(stderr, "Error %d while allocating message buffer", ERROR_NOT_ENOUGH_MEMORY);
    }

    ZeroMemory(msgBuffer, ERR_MSG_LEN);

    dwResult = FormatMessage(FORMAT_MESSAGE_FROM_HMODULE,
                          GetModuleHandle(L"wininet.dll"),
                          err,
                          MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                          msgBuffer,
                          ERR_MSG_LEN,
                          NULL);

    if (dwResult)
    {
        fprintf(stderr, "%ws: %ws\n", str, msgBuffer);
    }
    else
    {
        fprintf(stderr, "Error %d while formatting message for %d in %ws\n",
                GetLastError(), err, str);
    }

    Free(msgBuffer);
}

VOID
LogSysError(
	DWORD err,
	LPCWSTR str
)
/*++

Routine Description:
     This routine is used to log System Errors in human readable form.

Arguments:
     err - Error number obtained from GetLastError()
     str - String pointer holding caller-context information

Return Value:
    None.

--*/
{
    DWORD dwResult;
    LPWSTR msgBuffer = (LPWSTR)Malloc(ERR_MSG_LEN);
    // Make sure buffer allocation is successful.
    if (!msgBuffer)
    {
        fprintf(stderr, "Error %d while allocating message buffer", ERROR_NOT_ENOUGH_MEMORY);
    }

    ZeroMemory(msgBuffer, ERR_MSG_LEN);

    dwResult = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
						  NULL,
						  err,
                          MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                          msgBuffer,
                          ERR_MSG_LEN,
                          NULL);

    if (dwResult)
    {
        fprintf(stderr, "%ws: %ws\n", str, msgBuffer);
    }
    else
    {
        fprintf(stderr,
                "Error %d while formatting message for %d in %ws\n",
                GetLastError(), err, str);
    }

    Free(msgBuffer);
}


