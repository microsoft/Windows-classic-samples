//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (C) 2004  Microsoft Corporation.  All Rights Reserved.
//
// Module Name: instlsp.cpp
//
// Description:
//
//    This sample illustrates how to develop a layered service provider.
//    This LSP is simply a pass through sample which counts the bytes transfered
//    on each socket. 
//
//    This file contains an installation program to insert the layered sample
//    into the Winsock catalog of providers.
//    
//
// Compile:
//
//    Compile with the Makefile:
//      nmake /f Makefile
//
// Execute:
//
//    This project produces a executable file instlsp.exe. The installation app
//    allows you to install the LSP over any provider. Note however that if you
//    choose to install over a single provider, you should install over all 
//    providers of that address family (e.g. if you install over UDP, install
//    over TCP and RAW providers as well). The arguments are:
//
//       -a             Install over all providers
//       -c Catalog     Indicates which catalog to operate on (for Win64)
//       -d             Full path and filename of LSP dll
//       -f             Remove all layered providers (last ditch recovery)
//       -i             Install the LSP
//       -l             Print the layered entries only
//       -m             Displays a logical, ordered map of the LSPs installed
//       -n "String"    Name of the layered provider (catalog name, not dll name)
//       -o  CatID      Layer over the given provider (indicated by the catalog id)
//       -p             Print the Winsock catalog (and catalog ids)
//       -r  CatID      Remove the LSP
//
//    For example, first print out the catalog:
//       instlsp.exe -p
//        1001 - MSAFD ATM AAL5
//        1002 - MSAFD Tcpip [TCP/IP]
//        1003 - MSAFD Tcpip [UDP/IP]
//        1004 - MSAFD Tcpip [RAW/IP]
//        1005 - RSVP UDP Service Provider
//        1006 - RSVP TCP Service Provider
//        1019 - MSAFD AppleTalk [ADSP]
//        1020 - MSAFD AppleTalk [ADSP] [Pseudo Stream]
//        1021 - MSAFD AppleTalk [PAP]
//        1022 - MSAFD AppleTalk [RTMP]
//        1023 - MSAFD AppleTalk [ZIP]
//
//    To install over AppleTalk
//       instlsp.exe -i -o 1019 -o 1020 -o 1021 -o 1022 -o 1023 -n "Foobar LSP" -d c:\path\lsp.dll
//
//    Print the new catalog out:
//       instlsp.exe -p
//        1041 - Foobar LSP over [MSAFD AppleTalk [ADSP]]
//        1042 - Foobar LSP over [MSAFD AppleTalk [PAP]]
//        1043 - Foobar LSP over [MSAFD AppleTalk [RTMP]]
//        1044 - Foobar LSP over [MSAFD AppleTalk [ZIP]]
//        1001 - MSAFD ATM AAL5
//        1002 - MSAFD Tcpip [TCP/IP]
//        1003 - MSAFD Tcpip [UDP/IP]
//        1004 - MSAFD Tcpip [RAW/IP]
//        1005 - RSVP UDP Service Provider
//        1006 - RSVP TCP Service Provider
//        1019 - MSAFD AppleTalk [ADSP]
//        1020 - MSAFD AppleTalk [ADSP] [Pseudo Stream]
//        1021 - MSAFD AppleTalk [PAP]
//        1022 - MSAFD AppleTalk [RTMP]
//        1023 - MSAFD AppleTalk [ZIP]
//        1040 - Foobar LSP
//
//    To remove the LSP, supply the catalog ID of the hidden entry to remove:
//       instlsp.exe -r 1040 
//
//    In case all else fails (removes all LSPs installed):
//       instlsp.exe -f
//
#include "instlsp.h"

//
// Global variable: Function pointer to WSCUpdateProvider if on Windows XP or greater.
//                  Uninstalling an LSP when other LSPs are layered over it is really
//                  difficult; however on Windows XP and greater the WSCUpdateProvider
//                  function makes this much simpler. On older platforms its a real
//                  pain.
LPWSCUPDATEPROVIDER fnWscUpdateProvider   = NULL,
                    fnWscUpdateProvider32 = NULL;
HMODULE             gModule = NULL;
GUID                gProviderGuid;

// Prototype for usage information
void usage( __in_z char *progname );

////////////////////////////////////////////////////////////////////////////////
//
// Function Implementation
//
////////////////////////////////////////////////////////////////////////////////

//
// Function: main
//
// Description:
//    Parse the command line arguments and call either the install, remove, 
//    print, etc. routines.
//
int _cdecl main(int argc, char *argv[])
{
    WSADATA             wsd;
    LPWSAPROTOCOL_INFOW pProtocolInfo   = NULL;
    LSP_ENTRY          *pLspMap=NULL;
#ifdef _WIN64
    WINSOCK_CATALOG     eCatalog        = LspCatalog64Only;
#else
    WINSOCK_CATALOG     eCatalog        = LspCatalog32Only;
#endif
    INT                 iTotalProtocols = 0,
                        iLspCount       = 0,
                        i;
    DWORD              *pdwCatalogIdArray = NULL,
                        dwCatalogIdArrayCount = 0,       // How many to install over
                        dwRemoveCatalogId = 0;
    BOOL                bInstall                   = TRUE,
                        bInstallOverAll            = FALSE,
                        bRemoveAllLayeredEntries   = FALSE,
                        bPrintProviders            = FALSE,
                        bDisplayOnlyLayeredEntries = FALSE,
                        bVerbose                   = FALSE,
                        bMapLsp                    = FALSE,
                        bArgsOkay                  = FALSE,
                        bIFSProvider               = FALSE;
    char               *lpszLspName = NULL,
                       *lpszLspPathAndFile = NULL,
                       *lpszLspPathAndFile32 = NULL;
    int                 rc;

    ////////////////////////////////////////////////////////////////////////////
    //
    // Initialization and Command Line Parsing
    //
    ////////////////////////////////////////////////////////////////////////////

    // Load Winsock
    rc = WSAStartup( MAKEWORD(2,2), &wsd );
    if ( 0 != rc )
    {
        fprintf( stderr, "Unable to load Winsock: %d\n", rc );
        return -1;
    }

    // Initialize data structures
    LspCreateHeap( &rc );

    __try
    {
        InitializeCriticalSection( &gDebugCritSec );
    }
    __except( EXCEPTION_EXECUTE_HANDLER )
    {
        goto cleanup;
    }


    // First count how many catalog parameters are supplied so we can dynamically
    // allocate the right sized buffer
    for(i=1; i < argc ;i++)
    {
        if ( strncmp( argv[ i ], "-o", 2 ) == 0 )
            dwCatalogIdArrayCount++;
    }

    // Allocate space for the array of catalog IDs
    if ( 0 < dwCatalogIdArrayCount )
    {
        pdwCatalogIdArray = (DWORD *) LspAlloc(
                sizeof( DWORD ) * dwCatalogIdArrayCount,
                &rc
                );
        if ( NULL == pdwCatalogIdArray )
        {
            goto cleanup;
        }
    }

    // Set back to zero so we can use it as the index into our array
    dwCatalogIdArrayCount = 0;

    // Parse the command line
    for(i=1; i < argc ;i++)
    {
        if ( ( 2   != strlen( argv[i] ) ) && 
             ( '-' != argv[i][0] ) && 
             ( '/' != argv[i][0] )
           )
        {
            goto cleanup;
        }

        switch ( tolower( argv[i][1] ) )
        {
            case 'a':               // Install LSP over all currently installed providers
                bInstallOverAll = TRUE;
                break;

            case 'c':               // For 64-bit: which catalog to operate on?
                if (i+1 >= argc)
                    goto cleanup;

                switch (tolower(argv[i+1][0]))
                {
                    case 'b':       // Both Winsock catalogs
                        eCatalog = LspCatalogBoth;
                        break;
                    case '6':       // 64-bit Winsock catalog only
                        eCatalog = LspCatalog64Only;
                        break;
                    case '3':       // 32-bit Winsock catalog only
                        eCatalog = LspCatalog32Only;
                        break;
                    default:
                        goto cleanup;
                        break;
                }
                i++;
                break;

            case 'd':               // Full path and filename to LSP
                if ( i+1 >= argc )
                    goto cleanup;
                if (_strnicmp(argv[i], "-d32", 4))
                    lpszLspPathAndFile32 = argv[ ++i ];
                else
                    lpszLspPathAndFile = argv[ ++i ];

                break;

            case 'f':               // Uninstall all layered providers
                bRemoveAllLayeredEntries = TRUE;
                bInstall = FALSE;
                break;

            case 'h':               // Install as an IFS provider
                bIFSProvider = TRUE;
                break;

            case 'i':               // install
                bInstall = TRUE;
                break;

            case 'l':               // print the layered providers only
                bPrintProviders = TRUE;
                bDisplayOnlyLayeredEntries = TRUE;
                break;

            case 'm':               // Map and print the LSP structure
                bMapLsp = TRUE;
                bInstall = FALSE;
                break;

            case 'n':               // name of the LSP to install (not the DLL name)
                if (i+1 >= argc)
                    goto cleanup;

                lpszLspName = argv[++i];
                break;

            case 'o':               // catalog id (to install over)
                if (i+1 >= argc)
                    goto cleanup;

                pdwCatalogIdArray[dwCatalogIdArrayCount++] = atoi(argv[++i]);
                break;

            case 'p':               // print the catalog
                bPrintProviders = TRUE;
                bDisplayOnlyLayeredEntries = FALSE;
                break;

            case 'r':               // remove an LSP
                bInstall = FALSE;
                if (i+1 >= argc)
                    goto cleanup;
                dwRemoveCatalogId = atol(argv[++i]);
                break;

            case 'v':               // verbose mode (when printing with -p option)
                bVerbose = TRUE;
                break;

            default:
                goto cleanup;
                break;
        }
    }

#ifndef _WIN64
    if ( LspCatalog64Only == eCatalog )
    {
        fprintf(stderr, "\n\nUnable to manipulate 64-bit Winsock catalog from 32-bit process!\n\n");
        goto cleanup;
    }
#endif

    bArgsOkay = TRUE;

    gModule = LoadUpdateProviderFunction();

    if ( TRUE == bPrintProviders )
    {
        // Print the 32-bit catalog
        if ( ( LspCatalogBoth == eCatalog ) || ( LspCatalog32Only == eCatalog ) )
        {
            printf( "\n\nWinsock 32-bit Catalog:\n" );
            printf( "=======================\n" );
            PrintProviders( LspCatalog32Only, bDisplayOnlyLayeredEntries, bVerbose );
        }
        // Print the 64-bit catalog
        if ( ( LspCatalogBoth == eCatalog ) || ( LspCatalog64Only == eCatalog ) )
        {
            printf( "\n\nWinsock 64-bit Catalog:\n" );
            printf( "=======================\n" );
            PrintProviders( LspCatalog64Only, bDisplayOnlyLayeredEntries, bVerbose );
        }
    }
    else if ( TRUE == bInstall )
    {
        if ( NULL == lpszLspPathAndFile )
        {
            fprintf( stderr, "\n\nError! Please specify path and filename of LSP!\n\n");
            bArgsOkay = FALSE;
            goto cleanup;
        }

        if ( TRUE == bInstallOverAll )
        {
            // Make sure user didn't specify '-a' and '-o' flags
            if ( 0 != dwCatalogIdArrayCount )
            {
                fprintf( stderr, "\n\nError! Cannot specify both '-a' and '-o' flags!\n\n" );
                goto cleanup;
            }

            // Enumerate the appropriate catalog we will be working on
            pProtocolInfo = EnumerateProviders( eCatalog, &iTotalProtocols );
            if ( NULL == pProtocolInfo )
            {
                fprintf( stderr, "%s: EnumerateProviders: Unable to enumerate Winsock catalog\n",
                        argv[ 0 ]
                        );
                goto cleanup;
            }

            // Count how many non layered protocol entries there are
            for(i=0; i < iTotalProtocols ;i++)
            {
                if ( LAYERED_PROTOCOL != pProtocolInfo[ i ].ProtocolChain.ChainLen )
                    dwCatalogIdArrayCount++;
            }

            // Allocate space for all the entries
            pdwCatalogIdArray = (DWORD *) LspAlloc(
                    sizeof( DWORD ) * dwCatalogIdArrayCount,
                   &rc
                    );
            if ( NULL == pdwCatalogIdArray )
            {
                fprintf( stderr, "%s: LspAlloc failed: %d\n", argv[ 0 ], rc );
                goto cleanup;
            }

            // Get the catalog IDs for all existing providers
            dwCatalogIdArrayCount = 0 ;
            for(i=0; i < iTotalProtocols ;i++)
            {
                if ( LAYERED_PROTOCOL != pProtocolInfo[ i ].ProtocolChain.ChainLen )
                {
                    pdwCatalogIdArray[ dwCatalogIdArrayCount++ ] = pProtocolInfo[ i ].dwCatalogEntryId;
                }
            }

            FreeProviders( pProtocolInfo );
            pProtocolInfo = NULL;
        }

        // Install the LSP with the supplied parameters
        rc = InstallLsp(
                eCatalog,
                lpszLspName,
                lpszLspPathAndFile,
                lpszLspPathAndFile32,
                dwCatalogIdArrayCount,
                pdwCatalogIdArray,
                bIFSProvider,
                bInstallOverAll
                );
    }
    else if ( TRUE == bMapLsp )
    {
        // Display the 32-bit LSP catalog map
        if ( ( LspCatalogBoth == eCatalog ) || ( LspCatalog32Only == eCatalog ) )
        {
            printf("\n32-bit Winsock LSP Map:\n\n");

            pProtocolInfo = EnumerateProviders( LspCatalog32Only, &iTotalProtocols );
            if ( NULL == pProtocolInfo )
            {
                fprintf(stderr, "%s: EnumerateProviders: Unable to enumerate Winsock catalog\n",
                        argv[ 0 ] 
                        );
                goto cleanup;
            }

            pLspMap = BuildLspMap( pProtocolInfo, iTotalProtocols, &iLspCount );
            if ( NULL == pLspMap )
            {
                printf( "\nNo LSPs are installed\n\n" );
            }
            else
            {
                PrintLspMap( pLspMap, iLspCount );

                FreeLspMap( pLspMap, iLspCount );
                pLspMap = NULL;
            }
           
            FreeProviders( pProtocolInfo );
            pProtocolInfo = NULL;
        }

        // Display the 64-bit LSP catalog map
        if ( ( LspCatalogBoth == eCatalog ) || ( LspCatalog64Only == eCatalog ) )
        {
            printf("\n64-bit Winsock LSP Map:\n\n");

            pProtocolInfo = EnumerateProviders( LspCatalog64Only, &iTotalProtocols );
            if ( NULL == pProtocolInfo )
            {
                fprintf(stderr, "%s: EnumerateProviders: Unable to enumerate Winsock catalog\n",
                        argv[ 0 ]
                        );
                goto cleanup;
            }

            pLspMap = BuildLspMap( pProtocolInfo, iTotalProtocols, &iLspCount );
            if ( NULL == pLspMap )
            {
                printf( "\nNo LSPs are installed\n\n" );
            }
            else
            {
                PrintLspMap( pLspMap, iLspCount );

                FreeLspMap( pLspMap, iLspCount );
                pLspMap = NULL;
            }

            FreeProviders( pProtocolInfo );
            pProtocolInfo = NULL;
        }
    }
    else
    {
        // We must be removing an LSP

        if ( TRUE == bRemoveAllLayeredEntries )
        {
            if ( ( LspCatalogBoth == eCatalog ) || ( LspCatalog32Only == eCatalog ) )
                RemoveAllLayeredEntries( LspCatalog32Only );

            if ( ( LspCatalogBoth == eCatalog ) || ( LspCatalog64Only == eCatalog ) )
                RemoveAllLayeredEntries( LspCatalog64Only );
        }
        else
        {

            // Make sure a catalog entry to remove was supplied
            if ( dwRemoveCatalogId == 0 )
            {
                bArgsOkay = FALSE;
                goto cleanup;
            }

            if ( ( LspCatalogBoth == eCatalog ) || ( LspCatalog32Only == eCatalog ) )
                RemoveProvider( LspCatalog32Only, dwRemoveCatalogId );

            if ( ( LspCatalogBoth == eCatalog ) || ( LspCatalog64Only == eCatalog ) )
                RemoveProvider( LspCatalog64Only, dwRemoveCatalogId );

        }
    }

cleanup:

    if ( FALSE == bArgsOkay )
        usage( argv[ 0 ] );

    //
    // When invoked on Vista under non elevated permissions, the EXE is launched in
    // a new CMD window. The following getchar stops the window from exiting 
    // immediately so you can see what its output was.
    //
    printf("Press any key to continue...\n");
    getchar();

    //
    // Free any dynamic allocations and/or handles
    //

    if ( NULL != pdwCatalogIdArray )
        LspFree( pdwCatalogIdArray );

    if ( NULL != pProtocolInfo)
        FreeProviders( pProtocolInfo );

    if ( NULL != pLspMap )
        FreeLspMap( pLspMap, iLspCount );

    if ( NULL != gModule )
        FreeLibrary( gModule );

    LspDestroyHeap( );

    DeleteCriticalSection( &gDebugCritSec );

    WSACleanup();

    return 0;
}

//
// Function: usage
//
// Description:
//    Prints usage information.
//
void usage( __in_z char *progname )
{
    printf("usage: %s -i -r [CatId] -o [CatId] -p ...\n", progname);
    printf(
           "       -a           Install over all providers (base or layered)\n"
           "                       Cannot be combined with '-o' option\n"
           "       -c Catalog   Indicates which catalog to operate on\n"
           "          b            Both 64-bit and 32-bit Winsock catalogs\n"
           "          6            64-bit Winsock catalog only\n"
           "          3            32-bit Winsock catalog only\n"
           "       -d           Full path and filename of LSP DLL to install\n"
           "       -d32         Full path and filename of 32-bit DLL to install\n"
           "                       (Only needed when installing on 64-bit OS\n"
           "       -h           LSP is an IFS provider (by default its non-IFS)\n"
           "       -i           Install LSP\n"
           "       -f           Remove all layered entries\n"
           "       -l           Print layered providers only\n"
           "       -m           Display a map of the LSPs and the order they are\n"
           "                       installed in\n"
           "       -n Str       Name of LSP\n"
           "       -o CatId     Install over specified LSP\n"
           "                       This option may be specified multiple times\n"
           "                       Cannot be combined with '-a' option\n"
           "       -p           Print all layers and their catalog IDs\n"
           "       -r CatId     Remove LSP\n"
           "       -v           Print verbose catalog information (used with -p)\n"
           "\n"
           "Example:\n\n"
           "   install:\n\tinstlsp.exe -i -o 1001 -o 1002 -n \"MyLsp\" -d c:\\lsp\\mylsp.dll\n\n"
           "   remove:\n\tinstlsp.exe -r <DUMMY_CATALOG_ID>\n\n"
           "   remove all LSPs:\n\tinstlsp.exe -f\n"
           );
}
