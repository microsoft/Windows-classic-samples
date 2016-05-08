//----------------------------------------------------------------------------
//
//  Microsoft Active Directory 2.5 Sample Code
//
//  Copyright (C) Microsoft Corporation, 1996 - 2000
//
//  File:       main.cxx
//
//  Contents:   Main for adscmd
//
//
//----------------------------------------------------------------------------


#include "main.hxx"


//-------------------------------------------------------------------------
//
// main
//
//-------------------------------------------------------------------------

int __cdecl
main(int argc, char * argv[])
{
    HRESULT hr;
    int exitcode;

    if (argc != 3) {
        PrintUsage() ;
        exit(1);
    }

    hr = CoInitialize(NULL);

    if (FAILED(hr)) {
        printf("CoInitialize failed\n");
        exit(1);
    }

    if (_stricmp(argv[1],"list") == 0) {

        //
        // Call the List helper function
        //

        exitcode = DoList(argv[2]) ;
    }
    else if (_stricmp(argv[1],"dump") == 0) {

        //
        // Call the Dump helper function
        //

        exitcode = DoDump(argv[2]) ;
    }
    else {

        //
        // Unknown command
        //

        PrintUsage() ;
        exitcode = 1 ;
    }
    CoUninitialize();
    exit(exitcode) ;
    return 0;
}
