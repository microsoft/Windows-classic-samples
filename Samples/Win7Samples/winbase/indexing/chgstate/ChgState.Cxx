//+-------------------------------------------------------------------------
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
// PROGRAM:  ChgState.cxx
//
// PURPOSE:  To change the state of a catalog (on local machine) 
//
// PLATFORM: Windows  2000
//
//--------------------------------------------------------------------------

//+-------------------------------------------------------------------------
//
//  Function:   wmain
//
//  Synopsis:   Entry point for the app.  
//
//  Arguments:  [argc]     - Argument count
//              [argv]     - Arguments
//
//--------------------------------------------------------------------------

#include <stdio.h>
#include <windows.h>

#include <ntquery.h>

//+-------------------------------------------------------------------------
//
//  Function:   Usage
//
//  Synopsis:   Displays information about how to use the app and exits
//
//--------------------------------------------------------------------------

void Usage()
{
    printf( "Usage: ChgState /a:<RO|RW|Stop|GetState> /c:<Catalog Name>\n" ); 
    printf( "    /m:<Machine Name>\n\n" );
    printf( "    ChgState            Change the state of a catalog\n" );
    printf( "    /a:<Action>         the action to be taken, default is RO\n" );
    printf( "    The three actions are states RO (Read Only), RW (Read\\Write) and Stop (Stopped)\n" );
    printf( "    You can also specify GetState to check the state of a catalog\n" ); 
    printf( "    /c:<Catalog Name>   name of the catalog, default is SYSTEM\n" );
    printf( "    /m:<Machine Name>   name of the machine, default is local machine\n" );
    exit( -1 );
} //Usage

extern "C" int __cdecl wmain( int argc, WCHAR * argv[] )
{
    WCHAR const * pwcsCatalog     = L"system"; // default: system
    WCHAR const * pwcsMachine     = L".";      // default: local machine
    WCHAR const * pwcsAction      = L"RO";     // default: ReadOnly
    DWORD dwNewState              = CICAT_READONLY;
    DWORD dwOldState;

    SCODE sc = S_OK;

    // Parse the command for arguments
    if ( argc > 1 )
    {
        for ( int i = 1; i < argc; i++ )
        {
            if ( L'/' == argv[i][0] )
            {
                WCHAR wc = (WCHAR) toupper( argv[i][1] );

                if ( ':' != argv[i][2] )
                    Usage();

                if ( 'A' == wc )  
                    pwcsAction = argv[i] + 3;
                else if ( 'C' == wc )
                    pwcsCatalog = argv[i] + 3;
                else if ( 'M' == wc )
                    pwcsMachine = argv[i] + 3;
                else
                    Usage();
            }
            else 
                Usage();
        }
    }
    else
    {
        Usage();
    }

    if ( !wcscmp( pwcsAction, L"RO" ) )          // ReadOnly
        dwNewState = CICAT_READONLY;
    else if ( !wcscmp( pwcsAction, L"RW" ) )     // ReadWrite
        dwNewState = CICAT_WRITABLE;   
    else if ( !wcscmp( pwcsAction, L"Stop" ) )   // Stop
        dwNewState = CICAT_STOPPED;
    else if ( !wcscmp( pwcsAction, L"GetState" ) ) // Get the current state 
        dwNewState = CICAT_GET_STATE;
    else
    {
        fprintf( stderr, "Action undefined!\n" );
        exit(-1);
    }

    // call the API
    sc = SetCatalogState ( pwcsCatalog,
                           pwcsMachine,  
                           dwNewState,
                           &dwOldState );   

    if ( FAILED( sc ) )
    {
        printf( "ChangeState for catalog %ws failed with error %#x\n", pwcsCatalog ,sc );
        return -1;
    }

    printf(" Old State is " );
    if ( CICAT_STOPPED == dwOldState ) 
        printf( "CICAT_STOPPED.\n" );
    else
    {
        if ( CICAT_WRITABLE & dwOldState ) 
            printf( "CICAT_WRITABLE.\n" );
        else if ( CICAT_READONLY & dwOldState )
            printf( "CICAT_READONLY.\n" );
        else printf( "Error obtaining oldState. The return value is %d\n", dwOldState );
    }

    return 0;
} //wmain
