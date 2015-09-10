//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
// Abstract:
//
//     This sample demonstrates how to use HttpPrepareUrl to get a new URL so that
//     it is safe and valid to use in other HTTPAPIs.
//
//     IdnSample <inputurl>
//
//     where:
//
//     inputurl        The input URL to prepare.
//

#pragma warning(disable:4201)   // nameless struct/union
#pragma warning(disable:4214)   // bit field types other than int

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <http.h>

int
__cdecl
wmain(
    int Argc,
    _In_reads_(Argc) PWCHAR Argv[]
    )
{
    DWORD Error = ERROR_SUCCESS;
    PWSTR PreparedUrl = NULL;
    HTTPAPI_VERSION Version = HTTPAPI_VERSION_2;
    BOOL ApiInitialized = FALSE;

    if (Argc != 2)
    {
        wprintf(L"Usage: IdnSample <url>\n");
        goto exit;
    }

    //
    // Initialize HTTPAPI to use server APIs.
    //

    Error = HttpInitialize(Version, HTTP_INITIALIZE_SERVER, NULL);

    if (Error != ERROR_SUCCESS)
    {
        goto exit;
    }

    ApiInitialized = TRUE;

    //
    // Prepare the input url.
    //

    Error = HttpPrepareUrl(NULL, 0, Argv[1], &PreparedUrl);

    if (Error != ERROR_SUCCESS)
    {
        goto exit;
    }

    wprintf(L"%s prepared is: %s\n", Argv[1], PreparedUrl);

exit:

    if (PreparedUrl != NULL)
    {
        HeapFree(GetProcessHeap(), 0, PreparedUrl);
        PreparedUrl = NULL;
    }

    if (ApiInitialized)
    {
        HttpTerminate(HTTP_INITIALIZE_SERVER, NULL);
        ApiInitialized = FALSE;
    }

    if (Error != ERROR_SUCCESS)
    {
        wprintf(L"Error code = %#lx.\n", Error);
    }

    return Error;
}
