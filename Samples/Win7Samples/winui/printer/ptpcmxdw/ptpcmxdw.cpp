// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "stdafx.h"

/*++

Routine Description:
    Parses command line arguments and sets the appropriate flag.

Arguments:
    argc             : The number of command line args.
    argv             : Command Line arguments
    pdwCmdLineOption : Pointer to DWORD where the flags are set.

Return Value:
    S_OK if parsing was succesful and the command line arguments could be meaningfully
    decoded. 

--*/

HRESULT
ParseCmdLineArgs(
    __in                INT     argc,
    __in_ecount(argc)   TCHAR   **argv,
    __out               PDWORD  pdwCmdLineOption
    )
{
    HRESULT hr = S_OK;

    if ( NULL == argv ||
         NULL == pdwCmdLineOption 
       )
    {
        hr = E_INVALIDARG;
        return hr;
    }

    *pdwCmdLineOption = 0;

    if (argc != 2)
    {
        // Only one command line option is allowed
        hr = __HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
        *pdwCmdLineOption |= DISPLAY_USAGE;
    }
    else
    {
        argv++;

        if ( !_tcsicmp(*argv, L"/s" ) ||
             !_tcsicmp(*argv, L"-s" ) 
            )
        {
            *pdwCmdLineOption |= PRINT_JOB_SIMPLE;
        }
        else if ( !_tcsicmp(*argv, L"/m" ) ||
                  !_tcsicmp(*argv, L"-m" ) 
            )
        {
            *pdwCmdLineOption |= PRINT_JOB_MULTIPLEPRINTTICKET;
        }
        else if ( !_tcsicmp(*argv, L"/i" ) ||
                  !_tcsicmp(*argv, L"-i" ) 
            )
        {
            *pdwCmdLineOption |= PRINT_JOB_IMAGEPASSTHROUGH;
        }
        else
        { 
            hr = __HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
            *pdwCmdLineOption |= DISPLAY_USAGE;
        }
    }

    return hr;
}



extern "C" int _cdecl _tmain(
    __in              INT     argc, 
    __in_ecount(argc) TCHAR*  argv[]
    )
{
    HRESULT hr               = S_OK;
    DWORD   dwCmdLineOptions = 0;

    hr = ParseCmdLineArgs(argc, argv, &dwCmdLineOptions);

    if ( FAILED(hr) )
    {
        if ( dwCmdLineOptions & DISPLAY_USAGE )
        {
            DisplayUsage();
        }
    }
    else
    {
        if (dwCmdLineOptions & PRINT_JOB_SIMPLE)
        {     
            hr = CreatePrintTicketJobSimple();
        }                       
        else if (dwCmdLineOptions & PRINT_JOB_MULTIPLEPRINTTICKET )
        {
            hr = CreatePrintJobMultiplePrintTicket();
        }
        else if (dwCmdLineOptions & PRINT_JOB_IMAGEPASSTHROUGH)
        {
            hr = CreatePrintJobWithImagePassThrough();
        }

        if ( SUCCEEDED(hr) )
        {
            vFormatAndPrint(IDS_APP_JOB_COMPLETED);
        }
        else
        {
            vFormatAndPrint(IDS_APP_JOB_NOT_COMPLETED);
        }
    }

    return SUCCEEDED(hr);
}
