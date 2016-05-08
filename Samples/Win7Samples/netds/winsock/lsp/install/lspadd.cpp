//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (C) 2004  Microsoft Corporation.  All Rights Reserved.
//
// Module Name: lspadd.cpp
//
// Description:
//
//    This file contains the routines used in installing an LSP into the Winsock
//    catalog. This file also contains routines for changing the Winsock provider
//    order since the catalog is typically reordered upon LSP installation.
//    See instlsp.cpp for more information on running this code.
//
#include "instlsp.h"

//
// Function: InstallLsp
//
// Description:
//    This function installs the LSP over each of the catalog IDs supplied in
//    the pdwCatalogIdArray into the given Winsock catalog (since there are
//    two catalogs on WIN64 platforms). This routine first installs the layered
//    protocol entry (the dummy hidden provider) followed by installing the
//    layered protocol chains.
//
int
InstallLsp(
    WINSOCK_CATALOG eCatalog,               // Which catalog to install LSP into
    __in_z char    *lpszLspName,            // String name of LSP
    __in_z char    *lpszLspPathAndFile,     // Location of 64-bit LSP dll and dll name
    __in_z char    *lpszLspPathAndFile32,   // Location of 32-bit LSP dll and dll name
    DWORD           dwCatalogIdArrayCount,  // Number of entries in pdwCatalogIdArray
    DWORD          *pdwCatalogIdArray,      // Array of IDs to install over
    BOOL            IfsProvider,
    BOOL            InstallOverAll
    )
{
    OSVERSIONINFOEX     osv = {0};
    WSAPROTOCOL_INFOW  *pProtocolInfo = NULL,
                       *pDummyEntry = NULL,
                       *pLayeredEntries = NULL;
    WCHAR               wszLspName[ WSAPROTOCOL_LEN ],
                        wszFullProviderPath[ MAX_PATH+1 ],
                        wszFullProviderPath32[ MAX_PATH+1 ];
    GUID                ProviderBaseGuid;
    INT                 rc = SOCKET_ERROR;

    //
    // Install the LSP over the given entries. If selected to install over all
    // entries, then enumerate the catalog to obtain all the available providers.
    //

    if ( NULL == lpszLspName )
    {
        lpszLspName = DEFAULT_LSP_NAME;
    }

    // Convert the LSP name to UNICODE since the Winsock catalog is all UNICODE
    rc = MultiByteToWideChar(
            CP_ACP, 
            0, 
            lpszLspName, 
            (int) strlen( lpszLspName ) + 1,
            wszLspName, 
            WSAPROTOCOL_LEN 
            );
    if (rc == 0)
    {
        fprintf(stderr, "InstallLsp: MultiByteToWideChar failed to convert '%s'; Error = %d\n",
                lpszLspName, GetLastError());
        goto cleanup;
    }

    rc = MultiByteToWideChar(
            CP_ACP,
            0,
            lpszLspPathAndFile,
            (int) strlen( lpszLspPathAndFile ) + 1,
            wszFullProviderPath,
            MAX_PATH
            );
    if ( 0 == rc )
    {
        fprintf( stderr, "InstallLsp: MultiByteToWidechar failed to convert '%s': Error = %d\n",
                lpszLspPathAndFile, GetLastError() );
        goto cleanup;
    }

    if (lpszLspPathAndFile32) 
    {
        rc = MultiByteToWideChar(
                CP_ACP,
                0,
                lpszLspPathAndFile32,
                (int) strlen( lpszLspPathAndFile32 ) + 1,
                wszFullProviderPath32,
                MAX_PATH
                );
        if ( 0 == rc )
        {
            fprintf( stderr, "InstallLsp: MultiByteToWidechar failed to convert '%s': Error = %d\n",
                    lpszLspPathAndFile32, GetLastError() );
            goto cleanup;
        }
    }
    else
    {
        wszFullProviderPath32[0] = '\0';
    }

    // Verify there's at least one entry to layer over
    if ( 0 == dwCatalogIdArrayCount )
    {
        fprintf(stderr, "InstallLsp: Error! Must specify at least one provider to layer over!\n\n");
        goto cleanup;
    }

    printf("LSP name is '%S'\n", wszLspName);

    // Retrieve the GUID under which the LSP is to be installed
    RetrieveLspGuid( lpszLspPathAndFile, &ProviderBaseGuid );

    osv.dwOSVersionInfoSize = sizeof(osv);
    GetVersionEx( (LPOSVERSIONINFO) &osv );

    if ( osv.dwMajorVersion >= 6 ) 
    {
        // On Windows Vista, use the new LSP install API

        rc = InstallProviderVista(
                eCatalog,
                wszLspName,
                wszFullProviderPath,
                wszFullProviderPath32,
               &ProviderBaseGuid,
                dwCatalogIdArrayCount,
                pdwCatalogIdArray,
                IfsProvider,
                InstallOverAll
                );
        if ( SOCKET_ERROR == rc )
        {
            goto cleanup;
        }

    }
    else
    {
        //
        // This isn't Vista so install the LSP the old way
        //

        // Create the 'dummy' protocol entry
        pDummyEntry = CreateDummyEntry( eCatalog, pdwCatalogIdArray[ 0 ], wszLspName, IfsProvider );
        if (pDummyEntry == NULL)
        {
            fprintf(stderr, "InstallLsp: CreateDummyEntry failed!\n");
            goto cleanup;
        }

        // Install the 'dummy' protocol entry for the LSP
        rc = InstallProvider(
                eCatalog, 
                &ProviderBaseGuid, 
                wszFullProviderPath, 
                pDummyEntry, 
                1
                );
        if ( NO_ERROR != rc )
        {
            fprintf(stderr, "InstallLsp: Unable to install the dummy LSP entry!\n");
            goto cleanup;
        }

        // Don't need this struture any more
        LspFree( pDummyEntry );
        pDummyEntry = NULL;

        if ( FALSE == IfsProvider )
        {
            rc = InstallNonIfsLspProtocolChains( eCatalog, &ProviderBaseGuid, wszLspName,
                    wszFullProviderPath, pdwCatalogIdArray, dwCatalogIdArrayCount );

        }
        else
        {
            rc = InstallIfsLspProtocolChains( eCatalog, &ProviderBaseGuid, wszLspName,
                    wszFullProviderPath, pdwCatalogIdArray, dwCatalogIdArrayCount );
        }

        if ( SOCKET_ERROR == rc )
        {
            // An error occured installing the chains so remove the dummy entry
            DeinstallProvider( eCatalog, &ProviderBaseGuid );
        }

    }

cleanup:

    if ( NULL != pProtocolInfo )
        FreeProviders( pProtocolInfo );

    if ( NULL != pDummyEntry )
        LspFree( pDummyEntry );

    if ( NULL != pLayeredEntries )
        LspFree( pLayeredEntries );

    return rc;
}

//
// Function: InstallProvider
//
// Description:
//    This is a wrapper for the WSCInstallProvider function. Depending on
//    which catalog is specified, this routine calls the correct install
//    routine.
//
int 
InstallProvider(
    WINSOCK_CATALOG     Catalog,        // Which catalog are we operating on
    GUID               *Guid,           // GUID under which provider will be installed
    WCHAR              *lpwszLspPath,   // Path to LSP's DLL
    WSAPROTOCOL_INFOW  *pProvider,      // Array of provider structures to install
    INT                 iProviderCount  // Number of providers in array
    )
{
    WSAPROTOCOL_INFOW *pEnumProviders = NULL,
                      *pEntry = NULL;
    INT                iEnumProviderCount,
                       ErrorCode,
                       rc = SOCKET_ERROR;

#ifdef _WIN64
    if ( LspCatalog32Only == Catalog )
    {
        // Can't install only in 32-bit catalog from 64-bit
        fprintf( stderr, "InstallProvider: Error! It is not possible to install only "
                "in 32-bit catalog from 64-bit process!\n\n"
                );
        goto cleanup;
    }
    else if ( LspCatalog64Only == Catalog )
    {
        // Just need to call WSCInstallProvider
        rc = WSCInstallProvider( 
                Guid, 
                lpwszLspPath, 
                pProvider, 
                iProviderCount, 
               &ErrorCode 
                );
    }
    else
    {
        // To install in both we must call WSCInstallProviderPath64_32
        rc = WSCInstallProvider64_32(
                Guid, 
                lpwszLspPath, 
                pProvider, 
                iProviderCount, 
               &ErrorCode
                );
    }
#else
    if ( LspCatalog32Only == Catalog )
    {
        // From a 32-bit process we can only install into 32-bit catalog
        rc = WSCInstallProvider(
                Guid, 
                lpwszLspPath, 
                pProvider, 
                iProviderCount, 
               &ErrorCode
                );
    }
    else
    {
        // From a 32-bit process, we can't touch the 64-bit catalog at all
        fprintf( stderr, "InstallProvider: Error! It is not possible to install into "
                "the 64-bit catalog from a 32-bit process!\n\n"
                );
        goto cleanup;
    }
#endif
    if ( SOCKET_ERROR == rc )
    {
        fprintf( stderr, "InstallProvider: WSCInstallProvider* failed: %d\n", ErrorCode );
        goto cleanup;
    }

    // Go back and enumerate what we just installed
    pEnumProviders = EnumerateProviders( Catalog, &iEnumProviderCount );
    if ( NULL == pEnumProviders )
    {
        fprintf( stderr, "InstallProvider: EnumerateProviders failed!\n" );
        goto cleanup;
    }
    
    // Make sure our entry is in the catalog
    pEntry = FindProviderByGuid( Guid, pEnumProviders, iEnumProviderCount );
    if ( pEntry )
    {
        printf( "Installed: [%4d] %S\n", 
                pEntry->dwCatalogEntryId,
                pEntry->szProtocol
                );
    }

cleanup:

    if ( NULL != pEnumProviders )
        FreeProviders( pEnumProviders );

    return rc;
}

//
// Function: CreateDummyEntry
//
// Description:
//    This creates a single WSAPROTOCOL_INFOW structure which describes the
//    hidden "dummy" entry for an LSP. This is required since the layered
//    chain entries must reference the catalog ID of the LSP and a catalog ID
//    cannot be obtained until an entry is installed.
//
WSAPROTOCOL_INFOW *
CreateDummyEntry(
    WINSOCK_CATALOG Catalog, 
    INT CatalogId, 
    WCHAR *lpwszLspName,
    BOOL IfsProvider
    )
{
    WSAPROTOCOL_INFOW *pProtocolInfo = NULL,
                      *pDummyEntry = NULL,
                      *pEntry = NULL;
    INT                iProtocolCount = 0;
    int                err;

    // Enumerate the catalog
    pProtocolInfo = EnumerateProviders( Catalog, &iProtocolCount );
    if ( NULL == pProtocolInfo )
    {
        fprintf(stderr, "CreateDummyEntry: EnumerateProviders failed!\n");
        goto cleanup;
    }

    // Find one of the providers we are layering over
    pEntry = FindProviderById( CatalogId, pProtocolInfo, iProtocolCount );
    if ( pEntry )
    {
        // Allocate space and copy the provider structure
        pDummyEntry = (WSAPROTOCOL_INFOW *) LspAlloc(
                sizeof( WSAPROTOCOL_INFOW ),
               &err
                );
        if ( NULL == pDummyEntry )
        {
            fprintf( stderr, "CreateDummyEntry: LspAlloc failed: %d\n", err );
            goto cleanup;
        }

        // Copy the entry as a basis for the dummy entry
        memcpy( pDummyEntry, pEntry, sizeof( WSAPROTOCOL_INFOW ) );
    }
    else
    {
        fprintf(stderr, "CreateDummyEntry: Error! Unable to find provider with ID of %d\n\n",
                CatalogId 
                );
        goto cleanup;
    }

    // Remove the IFS provider flag if the LSP doesn't support it
    if ( FALSE == IfsProvider )
        pDummyEntry->dwServiceFlags1 &= (~XP1_IFS_HANDLES);

    // Set the flags indicating this is a hidden ("dummy") entry
    pDummyEntry->iSocketType = 0;
    pDummyEntry->iProtocol   = 0;
    pDummyEntry->dwProviderFlags |= PFL_HIDDEN;
    pDummyEntry->dwProviderFlags &= (~PFL_MATCHES_PROTOCOL_ZERO);
    pDummyEntry->ProtocolChain.ChainLen = LAYERED_PROTOCOL;

    // Copy the LSP name
    wcsncpy_s( pDummyEntry->szProtocol, lpwszLspName, WSAPROTOCOL_LEN );

cleanup:

    if ( NULL != pProtocolInfo )
        FreeProviders( pProtocolInfo );

    return pDummyEntry;
}

//
// Function: InstallIfsLspProtocolChains
//
// Description:
//      This routine installs the layered protocol chains for an IFS based LSP. It
//      assumes the LSP dummy entry is already installed. This function first enumerates
//      the catalog to find the ID of the dummy entry. It then builds the protocol
//      entries for the IFS layered protocols. Note that an IFS entry must be installed
//      such that no non-IFS providers are layered beneath it. This means if the user
//      chooses to install the IFS LSP over a provider which includes non-IFS layers, it
//      must insert itself into the chain such that it is below all non-IFS providers.
//      This means that existing entries need to be modified in order to reflect this
//      ordering. Also in the event that the IFS LSP is inserted into an existing chain
//      this installer still builds a series of standalone entries (i.e. entries that
//      would have existed of the LSPs layered over the IFS LSP were installed after
//      the IFS LSP was).
//
int
InstallIfsLspProtocolChains(
    WINSOCK_CATALOG eCatalog,
    GUID           *Guid,
    WCHAR          *lpszLspName,
    WCHAR          *lpszLspFullPathAndFile,
    DWORD          *pdwCatalogIdArray,
    DWORD           dwCatalogIdArrayCount
    )
{
    WSAPROTOCOL_INFOW  *pProvider = NULL,
                       *pProviderNew = NULL,
                       *pLayeredEntries = NULL,
                       *pEntry = NULL,
                        TempEntry = {0};
    DWORD              *pProviderOrder = NULL,
                        dwDummyLspId;
    WCHAR               wszLspDll[ MAX_PATH ];
    BOOL                bLayeredOverNonIfs = FALSE,
                        bContainsNonIfs = FALSE;
    HRESULT             hr;
    int                 ProviderPathLen = MAX_PATH-1,
                        iProviderCount,
                        iProviderCountNew,
                        LayerIdx,
                        retval = SOCKET_ERROR,
                        err,
                        idx,
                        rc,
                        i, j, k;

    // Enumerate the catalog
    pProvider = EnumerateProviders( eCatalog, &iProviderCount );
    if ( NULL == pProvider )
    {
        fprintf( stderr, "InstallIfsLspProtocolChains: Unable to enumerate catalog\n" );
        goto cleanup;
    }

    // Find the dummy, hidden entry of our new LSP
    dwDummyLspId = GetCatalogIdForProviderGuid( Guid, pProvider, iProviderCount );

    ASSERT( dwDummyLspId != 0 );

    // Allocate space for the protocol chains of the new LSP
    pLayeredEntries = (WSAPROTOCOL_INFOW *) LspAlloc( sizeof(WSAPROTOCOL_INFOW) *
            dwCatalogIdArrayCount, &err );
    if ( NULL == pLayeredEntries )
    {
        fprintf( stderr, "InstallIfsLspProtocolChains: LspAlloc failed: %d\n", err );
        goto cleanup;
    }

    LayerIdx = 0;

    // Build the layered protocol entries as well as a list of those providers which
    // require modification. Whenever an LSP is installed, a number of protocol entries
    // are installed where the first entry in the chain array is the LSP's dummy entry.
    // Addtionally, if we're installing an IFS LSP over an provider whose protocol chain
    // includes non-IFS LSPs, the IFS LSP must be placed in the chain such that no
    // non-IFS LSPs are positioned after it in the chain.

    // Loop through each ID we're layering over
    for(i=0; i < (int)dwCatalogIdArrayCount ;i++)
    {
        for(j=0; j < iProviderCount ;j++)
        {
            printf("Matching selected ID %d to catalog %d\n",
                    pdwCatalogIdArray[ i ], pProvider[ j ].dwCatalogEntryId );

            if ( pdwCatalogIdArray[ i ] == pProvider[ j ].dwCatalogEntryId )
            {
                // Verify the entry has room enough to be layered over
                if ( pProvider[ j ].ProtocolChain.ChainLen >= ( MAX_PROTOCOL_CHAIN - 1 ) )
                {
                    fprintf( stderr, "InstallIfsLspProtocolChain: Too many LSPs installed!\n");
                    goto cleanup;
                }

                // Save off the entry which we're layering over
                memcpy( &pLayeredEntries[ LayerIdx ], &pProvider[ j ],
                        sizeof( pLayeredEntries[ 0  ] ) );

                memcpy( &TempEntry, &pProvider[ j ], sizeof( TempEntry ) );        

                // Fill in the new LSP entry's name
                hr = StringCchPrintfW( pLayeredEntries[ LayerIdx ].szProtocol, WSAPROTOCOL_LEN,
                        L"%s over [%s]",
                        lpszLspName,
                        pProvider[ j ].szProtocol 
                        );
                if ( FAILED( hr ) )
                {
                    fprintf( stderr, "InstallIfsLspProtocolChains: StringCchPrintfW failed: 0x%x\n", hr );
                    goto cleanup;
                }

                // Check whether the selected entry contains non IFS LSPs in its chain
                if ( pProvider[ j ].ProtocolChain.ChainLen >= 2 )
                {
                    for(k=pProvider[ j ].ProtocolChain.ChainLen-2 ; k >= 0 ;k--)
                    {
                        bContainsNonIfs = IsNonIfsProvider( pProvider, iProviderCount, 
                                pProvider[ j ].ProtocolChain.ChainEntries[ k ] );

                        if ( TRUE == bContainsNonIfs )
                        {
                            // Need to modify the pProvider entry to reference the
                            // added LSP entry within its chain

                            // In the 'modified' array make a space at location after 'k'
                            InsertIdIntoProtocolChain( &pProvider[ j ], k+1, UPDATE_LSP_ENTRY );

                            // Save the index to the layer which corresponds to this entry
                            pProvider[ j ].dwProviderReserved = LayerIdx + 1;

                            // Need to fix the 'pLayeredEntry' as well
                            BuildSubsetLspChain( &pLayeredEntries[ LayerIdx ], k+1, dwDummyLspId );

                            pLayeredEntries[ LayerIdx ].dwServiceFlags1 |= XP1_IFS_HANDLES;

                            bLayeredOverNonIfs = TRUE;

                            // Need to insert the IFS provider in all LSPs that  are layered
                            // above the location where the IFS provider was just inserted
                            InsertIfsLspIntoAllChains( &TempEntry, pProvider, iProviderCount, 
                                    LayerIdx + 1, k );

                            break;
                        }
                    }
                }

                // Need to setup the protocol chain in the pLayeredEntry if we haven't
                // already done so above
                if ( TRUE != bContainsNonIfs )
                {
                    InsertIdIntoProtocolChain( &pLayeredEntries[ LayerIdx ], 0, dwDummyLspId );

                    // The second entry is always the ID of the current pProvider[i]
                    //     In case of multiple LSPs then if we didn't do this the [1] index
                    //     would contain the ID of that LSP's dummy entry and not the entry
                    //     itself.
                    pLayeredEntries[ LayerIdx ].ProtocolChain.ChainEntries[ 1 ] = 
                            TempEntry.dwCatalogEntryId;

                    pLayeredEntries[ LayerIdx ].dwServiceFlags1 |= XP1_IFS_HANDLES;
                }

                LayerIdx++;
            }
        }
    }

    ASSERT( LayerIdx == (int)dwCatalogIdArrayCount );

    // Create a unique GUID for each provider to install and install it
    for(i=0;i < (int)dwCatalogIdArrayCount ;i++)
    {
        if ( RPC_S_OK != UuidCreate( &pLayeredEntries[ i ].ProviderId ) )
        {
            fprintf(stderr, "InstallIfsLspProtocolChains: UuidCreate failed: %d\n", GetLastError());
            goto cleanup;
        }

        rc = InstallProvider( eCatalog, &pLayeredEntries[ i ].ProviderId,
                lpszLspFullPathAndFile, &pLayeredEntries[ i ], 1 );
        if ( NO_ERROR != rc )
        {
            fprintf(stderr, "InstallIfsLspProtocolChains: Unable to install the dummy LSP entry!\n");
            goto cleanup;
        }
    }

    if ( TRUE == bLayeredOverNonIfs )
    {
        // Enumerate the catalog again so we can find the catalog IDs

        pProviderNew = EnumerateProviders( eCatalog, &iProviderCountNew );
        if ( NULL == pProviderNew )
        {
            fprintf( stderr, "InstallIfsLspProtocolChains: Unable to enumerate catalog\n" );
            goto cleanup;
        }

        for(i=0; i < (int)dwCatalogIdArrayCount ;i++)
        {
            pLayeredEntries[ i ].dwCatalogEntryId = GetCatalogIdForProviderGuid(
                   &pLayeredEntries[ i ].ProviderId,
                    pProviderNew,
                    iProviderCountNew
                    );

            ASSERT( pLayeredEntries[ i ].dwCatalogEntryId != 0 );
        }

        // Update the protocol chains of the modified entries to point to the just
        //    installed providers
        for(i=0; i < iProviderCount ;i++)
        {
            if ( pProvider[ i ].dwProviderReserved == 0 )
                continue;

            for(j=0; j < pProvider[ i ].ProtocolChain.ChainLen ;j++)
            {
                if ( UPDATE_LSP_ENTRY == pProvider[ i ].ProtocolChain.ChainEntries[ j ] )
                {
                    pProvider[ i ].ProtocolChain.ChainEntries[ j ] = 
                        pLayeredEntries[ pProvider[ i ].dwProviderReserved - 1 ].dwCatalogEntryId;

                    pProvider[ i ].dwProviderReserved = 0;
                }
            }

            // Get the DLL path
            ProviderPathLen = MAX_PATH-1;
            rc = WSCGetProviderPath(
                    &pProvider[ i ].ProviderId,
                     wszLspDll,
                    &ProviderPathLen,
                    &err
                     );
            if ( SOCKET_ERROR == rc )
            {
                fprintf( stderr, "InstallIfsLspProtocolChains: WSCGetProviderPath failed: %d\n", err );
                goto cleanup;
            }

            // Update the providers which were modified
            rc = UpdateProvider( eCatalog, &pProvider[ i ].ProviderId,
                    wszLspDll, &pProvider[ i ], 1, &err );
            if ( SOCKET_ERROR == rc )
            {
                fprintf( stderr, "InstallIfsLspProtocolChains: UpdateProvider failed: %d\n", err );
                goto cleanup;
            }

            printf("Updated entry ID: %d: %S (chain len = %d)\n",
                    pProvider[ i ].dwCatalogEntryId,
                    pProvider[ i ].szProtocol,
                    pProvider[ i ].ProtocolChain.ChainLen
                    );
        }

        FreeProviders( pProvider );
        pProvider = NULL;

        FreeProviders( pProviderNew );
        pProviderNew = NULL;

        
        //WSCUpdateProvider doesn't update the process' copy of the winsock catalog. 
        //By calling cleanup and startup again, it forces a refresh. Otherwise, 
        //the rest of the installer code can't see the changes that were just made. 
        {
            WSADATA wsd;

            WSACleanup();

            WSAStartup( MAKEWORD(2,2), &wsd );
        }
        

        pProvider = EnumerateProviders( eCatalog, &iProviderCount );
        if ( NULL == pProvider )
        {
            fprintf( stderr, "InstallIfsLspProtocolChains: Unable to enumerate catalog\n" );
            goto cleanup;
        }

        // Allocate an array of DWORDs to contain the new catalog ordering
        pProviderOrder = (DWORD *)LspAlloc( iProviderCount * sizeof(DWORD), &err );
        if ( NULL == pProviderOrder )
        {
            fprintf( stderr, "InstallIfsLspProtocolChains: Unable to enumerate catalog\n" );
            goto cleanup;
        }

        // First add the entries we layered over first
        idx = 0;
        for(i=0; i < (int)dwCatalogIdArrayCount ;i++)
        {
            pEntry = FindProviderById( pdwCatalogIdArray[ i ], pProvider, iProviderCount );
            if ( NULL == pEntry )
            {
                fprintf(stderr, "InstallIfsLspProtocolChain: Unable to find entry to reorder catalog!\n");
                goto cleanup;
            }

            pEntry->dwProviderReserved = 1;

            pProviderOrder[ idx++ ] = pEntry->dwCatalogEntryId;
        }

        // Now go through the protocol chain of the entries we layered over and put those
        //    LSP entries next in the new order
        for(i=0; i < (int)dwCatalogIdArrayCount ;i++)
        {
            pEntry = FindProviderById( pdwCatalogIdArray[ i ], pProvider, iProviderCount );
            if ( NULL == pEntry )
            {
                fprintf(stderr, "InstallIfsLspProtocolChain: Unable to find entry to reorder catalog!\n");
                goto cleanup;
            }

            printf("Looping through: %d: %S (chain len = %d)\n", 
                    pEntry->dwCatalogEntryId,
                    pEntry->szProtocol,
                    pEntry->ProtocolChain.ChainLen );

            for(j=1; j < pEntry->ProtocolChain.ChainLen-1 ;j++)
            {
                dwDummyLspId = FindDummyIdFromProtocolChainId(
                        pEntry->ProtocolChain.ChainEntries[ j ],
                        pProvider,
                        iProviderCount
                        );

                printf("   Finding dummy ID for chain entry: %d is %d\n", 
                        pEntry->ProtocolChain.ChainEntries[ j ],
                        dwDummyLspId
                        );

                for(k=0; k < iProviderCount ;k++)
                {
                    if ( ( pProvider[ k ].ProtocolChain.ChainLen >= 2 ) &&
                         ( pProvider[ k ].ProtocolChain.ChainEntries[ 0 ] == dwDummyLspId ) &&
                         ( pProvider[ k ].dwProviderReserved == 0 )
                       )
                    {
                        pProviderOrder[ idx++ ] = pProvider[ k ].dwCatalogEntryId;
                        pProvider[ k ].dwProviderReserved = 1;

                        printf("      Adding: %d\n", pProvider[ k ].dwCatalogEntryId );
                    }
                }
            }
        }

        // Now any catalog entry that wasn't already copied, copy it
        for(i=0; i < iProviderCount ;i++)
        {
            if ( pProvider[ i ].dwProviderReserved == 0 )
                pProviderOrder[ idx++ ] = pProvider[ i ].dwCatalogEntryId;
        }

        ASSERT( idx == iProviderCount );

        // Write the new catalog order
        rc = WriteProviderOrder( eCatalog, pProviderOrder, iProviderCount, &err );
        if ( NO_ERROR != rc )
        {
            fprintf( stderr, "InstallIfsLspProtocolChains: WriteProviderOrder failed: %d\n",
                    err );
            goto cleanup;
        }
    }
    else
    {
        //
        // Reorder the winsock catalog so the layered chain entries appear first.
        // Since we didn't have to modify any existing entries, all we need to do is
        //    move the added entries to the head of the catalog
        // 
        rc = ReorderCatalog( eCatalog, dwDummyLspId );
        if ( NO_ERROR != rc )
        {
            fprintf(stderr, "InstallIfsLspProtocolChains: Unable to reorder Winsock catalog!\n");
            goto cleanup;
        }
    }

    retval = NO_ERROR;

cleanup:
    
    if ( NULL != pProvider )
    {
        FreeProviders( pProvider );
        pProvider = NULL;
    }

    if ( NULL != pProviderNew )
    {
        FreeProviders( pProviderNew );
        pProviderNew = NULL;
    }

    if ( NULL != pProviderOrder )
    {
        LspFree( pProviderOrder );
        pProviderOrder = NULL;
    }

    return retval;
}

//
// Function: InstallNonIfsLspProtocolChains
//
// Description:
//      This function builds and install the protocol chain entries associated with
//      a non-IFS LSP. The caling routine installs the dummy, hidden entry in the
//      catalog and this routine enumerates the catalog, finds the dummy entry,
//      builds the layered chain entries, and installs them into the catalog.
//
int
InstallNonIfsLspProtocolChains(
    WINSOCK_CATALOG eCatalog,
    GUID           *Guid,
    WCHAR          *lpszLspName,
    WCHAR          *lpszLspFullPathAndFile,
    DWORD          *pdwCatalogIdArray,
    DWORD           dwCatalogIdArrayCount
    )
{
    WSAPROTOCOL_INFOW   *pProvider = NULL,
                        *pLayeredEntries = NULL;
    DWORD                dwDummyLspId = 0;
    INT                  iProviderCount = 0,
                         retval = SOCKET_ERROR,
                         idx,
                         err,
                         rc,
                         i, j;
    HRESULT              hr;

    // Enumerate the catalog
    pProvider = EnumerateProviders( eCatalog, &iProviderCount );
    if ( NULL == pProvider )
    {
        fprintf( stderr, "InstallNonIfsLspProtocolChain: Unable to enumerate catalog\n" );
        goto cleanup;
    }

    pLayeredEntries = (WSAPROTOCOL_INFOW *) LspAlloc( sizeof(WSAPROTOCOL_INFOW) *
            dwCatalogIdArrayCount, &err );
    if ( NULL == pLayeredEntries )
    {
        fprintf( stderr, "InstallNonIfsLspProtocolChain: LspAlloc failed: %d\n", err );
        goto cleanup;
    }

    // Find the dummy entry so we can extract its catalog ID
    dwDummyLspId = GetCatalogIdForProviderGuid( Guid, pProvider, iProviderCount );

    ASSERT( dwDummyLspId != 0 );

    // Go through the catalog and build the layered entries
    idx = 0;
    for(i=0; i < iProviderCount ;i++)
    {
        for(j=0; j < (int) dwCatalogIdArrayCount ;j++)
        {
            if ( pProvider[ i ].dwCatalogEntryId == pdwCatalogIdArray[ j ] )
            {
                if ( pProvider[ i ].ProtocolChain.ChainLen >= ( MAX_PROTOCOL_CHAIN - 1 ) )
                {
                    fprintf( stderr, "InstallNonIfsLspProtocolchain: Too many LSPs installed!\n");
                    goto cleanup;
                }

                memcpy( &pLayeredEntries[ idx ], &pProvider[ i ], sizeof( WSAPROTOCOL_INFOW ) );

                // Put our LSP name in the protocol field
                hr = StringCchPrintfW( pLayeredEntries[ idx ].szProtocol, WSAPROTOCOL_LEN,
                        L"%s over [%s]",
                        lpszLspName,
                        pProvider[ i ].szProtocol
                        );
                if ( FAILED( hr ) )
                {
                    fprintf( stderr, "InstallNonIfsLspProtocolChain: StringCchPrintfW failed: 0x%x\n", hr );
                    goto cleanup;
                }

                // Move all the protocol chain entries down by 1 position and insert 
                // the dummy entry id at the head
                InsertIdIntoProtocolChain( &pLayeredEntries[ idx ], 0, dwDummyLspId );

                // The second entry is always the ID of the current pProvider[i]
                //     In case of multiple LSPs then if we didn't do this the [1] index
                //     would contain the ID of that LSP's dummy entry and not the entry
                //     itself.
                pLayeredEntries[ idx ].ProtocolChain.ChainEntries[ 1 ] = 
                        pProvider[ i ].dwCatalogEntryId;

                // Remove the IFS flag 
                pLayeredEntries[ idx ].dwServiceFlags1 &= (~XP1_IFS_HANDLES);

                idx++;
            }
        }
    }

    for(i=0; i < (int)dwCatalogIdArrayCount ;i++)
    {
        // Create a GUID for the protocol chain entries
        if ( UuidCreate( &pLayeredEntries[ i ].ProviderId ) != RPC_S_OK )
        {
            fprintf(stderr, "InstallNonIfsLspProtocolChains: UuidCreate failed: %d\n", GetLastError());
            goto cleanup;
        }

        // Install the layered chain providers
        rc = InstallProvider(
                eCatalog, 
               &pLayeredEntries[ i ].ProviderId, 
                lpszLspFullPathAndFile,
               &pLayeredEntries[ i ], 
                1
                );
        if ( NO_ERROR != rc )
        {
            fprintf(stderr, "InstallNonIfsLspProtocolChains: Unable to install layered chain entries!\n");
            goto cleanup;
        }
    }

    // Reorder the winsock catalog so the layered chain entries appear first
    rc = ReorderCatalog( eCatalog, dwDummyLspId );
    if ( NO_ERROR != rc )
    {
        fprintf(stderr, "InstallNonIfsLspProtocolChains: Unable to reorder Winsock catalog!\n");
        goto cleanup;
    }

    retval = NO_ERROR;

cleanup:

    if ( NULL != pProvider )
        FreeProviders( pProvider );

    if ( NULL != pLayeredEntries )
        LspFree( pLayeredEntries );

    return retval;
}

//
// Function: InsertIfsLspIntoAllChains
//
// Description:
//    This routine takes a Winsock catalog entry where an IFS LSP entry is being
//    inserted in the middle of the chain and the same IFS LSP entry needs to be 
//    installed in the individual provider entries for every LSP referenced in the
//    chain above the IFS provider position. This function traverses the protocol
//    chain for all entries above the inserted IFS entry, looks up the provider
//    belonging to that entry, and inserts the IFS LSP into it's chain.
//
int
InsertIfsLspIntoAllChains( 
    WSAPROTOCOL_INFOW  *OriginalEntry,    // Original (unmodified) entry to follow chains
    WSAPROTOCOL_INFOW  *Catalog,          // Array of catalog entries
    int                 CatalogCount,     // Number of entries in Catalog array
    int                 IfsEntryIdx,      // Index into IFS standalone entry array
    int                 ChainIdx          // Chain index in OriginalEntry to start at
    )
{
    WSAPROTOCOL_INFOW   TempEntry = {0};
    int                 Idx, i, j, k;

    for(i=ChainIdx; i > 0 ;i--)
    {
        #ifdef DBG
        printf( "Looking for entry: %d\n", OriginalEntry->ProtocolChain.ChainEntries[ i ] );
        #endif

        for(j=0; j < CatalogCount ;j++)
        {
            if ( Catalog[ j ].dwCatalogEntryId == OriginalEntry->ProtocolChain.ChainEntries[ i ] ) 
            {
                printf( "Found match: %ws\n", Catalog[ j ].szProtocol );
                Idx = j;

                if ( Catalog[ j ].ProtocolChain.ChainLen == LAYERED_PROTOCOL )
                {
                    Idx = -1;

                    // Not good. The catalog ID in the chain points to the dummy
                    // entry. We'll need to do some other heuristic to find the
                    // "right" entry.
                    for(k=0; k < CatalogCount ;k++)
                    {
                        if ( ( OriginalEntry->iAddressFamily == Catalog[ k ].iAddressFamily ) &&
                             ( OriginalEntry->iSocketType == Catalog[ k ].iSocketType ) && 
                             ( OriginalEntry->iProtocol == Catalog[ k ].iProtocol ) &&
                             ( (i+1) == Catalog[ k ].ProtocolChain.ChainLen )
                           )
                        {
                            Idx = k;
                            break;
                        }
                    }
                }

                if ( Idx != -1 )
                {
                    // Found a match and need to insert the new IFS LSP into the chain
                    memcpy( &TempEntry, &Catalog[ Idx ], sizeof( TempEntry ) );

                    if ( Catalog[ Idx ].ProtocolChain.ChainLen >= 2 )
                    {
                        for(k=Catalog[ Idx ].ProtocolChain.ChainLen-2 ; k >= 0 ;k--)
                        {
                            if ( TRUE == IsNonIfsProvider( Catalog, CatalogCount, 
                                    Catalog[ Idx ].ProtocolChain.ChainEntries[ k ] ) )
                            {
                                // K points to first non-IFS provider - insert after
                                InsertIdIntoProtocolChain( &Catalog[ Idx ], k+1, UPDATE_LSP_ENTRY );

                                // Save the index to the layer which corresponds to this entry
                                Catalog[ Idx ].dwProviderReserved = IfsEntryIdx;
                            }
                        }
                    }
                }
                else
                {
                    printf( "????? Index not found ????\n" );
                }

                break;
            }
        }
    }

    return 0;
}

//
// Function: ReorderCatalog
//
// Description:
//    This routine reorders the Winsock catalog such that those entries
//    which reference the given catalog ID (dwLayerId) as the first entry
//    in the chain array occur at the head of the catalog. This routine
//    also operates on the specified Winsock catalog.
//
int 
ReorderCatalog(
    WINSOCK_CATALOG Catalog, 
    DWORD           dwLayeredId
    )
{
    DWORD     *pdwProtocolOrder = NULL;
    INT        iProviderCount,
               ErrorCode,
               rc = SOCKET_ERROR;

#ifdef _WIN64
    if ( ( LspCatalog32Only == Catalog ) || ( LspCatalogBoth == Catalog ) )
    {
        printf("Reordering 32-bit Winsock catalog...\n");
        pdwProtocolOrder = ReorderACatalog(
                LspCatalog32Only, 
                dwLayeredId, 
               &iProviderCount
                );
        if ( NULL == pdwProtocolOrder )
        {
            fprintf( stderr, "ReorderCatalog: ReorderACatalog failed!\n" );
            goto cleanup;
        }
        
        rc = WriteProviderOrder( LspCatalog32Only, pdwProtocolOrder, iProviderCount, &ErrorCode );
        if ( SOCKET_ERROR == rc )
        {
            fprintf( stderr, "ReorderCatalog: Reorder of 32-bit catalog failed: %d\n", rc );
        }
    }
    if ( ( LspCatalog64Only == Catalog ) || ( LspCatalogBoth == Catalog ) )
    {
        printf("Reordering 64-bit Winsock catalog...\n");
        pdwProtocolOrder = ReorderACatalog(
                LspCatalog64Only, 
                dwLayeredId, 
               &iProviderCount
                );
        if ( NULL == pdwProtocolOrder )
        {
            fprintf(stderr, "ReorderCatalog: ReorderACatalog failed!\n");
            goto cleanup;
        }
       
        rc = WriteProviderOrder( LspCatalog64Only, pdwProtocolOrder, iProviderCount, &ErrorCode );
        if ( SOCKET_ERROR == rc )
        {
            fprintf(stderr, "ReorderCatalog: Reorder of 64-bit catalog failed: %d\n", rc);
        }
    }
#else
    if ( ( LspCatalog32Only == Catalog ) || ( LspCatalogBoth == Catalog ) )
    {
        printf("Reordering 32-bit Winsock catalog...\n");
        pdwProtocolOrder = ReorderACatalog(
                LspCatalog32Only, 
                dwLayeredId, 
               &iProviderCount
                );
        if ( NULL == pdwProtocolOrder )
        {
            fprintf( stderr, "ReorderCatalog: ReorderACatalog failed!\n" );
            goto cleanup;
        }
        
        rc = WriteProviderOrder( LspCatalog32Only, pdwProtocolOrder, iProviderCount, &ErrorCode );
        if ( SOCKET_ERROR == rc )
        {
            fprintf(stderr, "ReorderCatalog: Reorder of 32-bit catalog failed: %d\n", rc);
        }
    }
#endif

cleanup:

    if ( NULL != pdwProtocolOrder )
        LspFree( pdwProtocolOrder );

    return rc;
}

//
// Function: ReorderACatalog
//
// Description:
//    This routine enumerates a single catalog (32 or 64 bit) and reorders
//    the catalog according to the supplied catalog ID. That is, any provider
//    with that ID at the head of it's protocol chain is moved to the beginning
//    of the catalog.
//
DWORD *
ReorderACatalog(
    WINSOCK_CATALOG Catalog,
    DWORD           dwLayerId,
    INT            *dwEntryCount
    )
{
    WSAPROTOCOL_INFOW   *pProvider = NULL;
    DWORD               *pdwProtocolOrder = NULL;
    INT                  iProviderCount = 0,
                         idx,
                         err,
                         i;

    // Validate parameters
    if ( ( NULL == dwEntryCount ) || ( LspCatalogBoth == Catalog ) )
        return NULL;

    // Enumerate the catalog
    pProvider = EnumerateProviders( Catalog, &iProviderCount );
    if ( NULL == pProvider )
    {
        fprintf( stderr, "ReorderACatalog: Unable to enumerate Winsock catalog!\n" );
        goto cleanup;
    }

    // Allocate space for the array of catalog IDs (the catalog order)
    pdwProtocolOrder = (DWORD *) LspAlloc(
            sizeof( DWORD ) * iProviderCount,
           &err
            );
    if ( NULL == pdwProtocolOrder )
    {
        fprintf(stderr, "ReorderACatalog: LspAlloc failed: %d\n", GetLastError());
        goto cleanup;
    }

    idx = 0;

    // First put all the layered entries at the head of the catalog
    for(i=0; i < iProviderCount ;i++)
    {
        if ( TRUE == IsIdInChain( &pProvider[ i ], dwLayerId ) )
        {
            pdwProtocolOrder[ idx++ ] = pProvider[ i ].dwCatalogEntryId;
        }
    }

    // Put the remaining entries after the layered chain entries
    for(i=0; i < iProviderCount ;i++)
    {
        if ( FALSE == IsIdInChain( &pProvider[ i ], dwLayerId ) )
        {
            pdwProtocolOrder[ idx++ ] = pProvider[ i ].dwCatalogEntryId;
        }
    }

cleanup:

    if (pProvider)
        FreeProviders(pProvider);

    // Update the count
    *dwEntryCount = iProviderCount;


    return pdwProtocolOrder;
}

//
// Function: WriteProviderOrder
//
// Description:
//    This function wraps the WSCWriteProviderOrder function and calls the
//    appropriate function depending on the Winsock catalog being modified
//    (the 32-bit and/or 64-bit Winsock catalogs).
//
int
WriteProviderOrder(
    WINSOCK_CATALOG Catalog,
    DWORD          *pdwCatalogOrder,
    DWORD           dwNumberOfEntries,
    INT            *lpErrno
    )
{
    int     rc = NO_ERROR;

#ifdef _WIN64
    if ( LspCatalog32Only == Catalog )
    {
        rc = WSCWriteProviderOrder32( pdwCatalogOrder, dwNumberOfEntries );
    }
    else if ( LspCatalog64Only == Catalog )
    {
        rc = WSCWriteProviderOrder( pdwCatalogOrder, dwNumberOfEntries );
    }
#else
    if ( LspCatalog32Only == Catalog )
    {
        rc = WSCWriteProviderOrder(pdwCatalogOrder, dwNumberOfEntries );
    }
    else
    {
        fprintf( stderr, "WriteProviderOrder: Unable to manipulate 64-bit catalog from "
                "a 32-bit process\n" );
    }
#endif
    if ( 0 != rc )
    {
        *lpErrno = rc;
        fprintf( stderr, "WriteProviderOrder: WSCWriteProviderOrder failed: %d\n", *lpErrno );
        rc = SOCKET_ERROR;
    }

    return rc;
}


//
// Function: InstallProviderVista
//
// Description:
//    On Windows Vista and later there is a new LSP install function
//    (WSCInstallProviderAndChains) that performs all necessary installation
//    steps in a single call (instead of multiple WSCInstallProvider calls).
//    This function dynamically loads the function and invokes it to install
//    the LSP. The function either expects a NULL list of WSAPROTOCOL_INFOW
//    structures in which case it installs the LSP over each unique address
//    family, socket type, and protocol base provider on the system. Otherwise,
//    if the pdwCatalogIdArray specifies exact providers, this function builds
//    a list of those provider structures in which case the install API installs
//    the LSP over each instance of the address family, socket type and protocol
//    specified in the array.
//
int
InstallProviderVista(
        WINSOCK_CATALOG eCatalog,               // Which catalog to install LSP into
        __in_z WCHAR   *lpszLspName,            // String name of LSP
        __in_z WCHAR   *lpszLspPathAndFile,     // Location of 64-bit LSP dll and dll name
        __in_z WCHAR   *lpszLspPathAndFile32,   // Location of 32-bit LSP dll and dll name
        LPGUID          providerGuid,
        DWORD           dwCatalogIdArrayCount,  // Number of entries in pdwCatalogIdArray
        DWORD          *pdwCatalogIdArray,      // Array of IDs to install over
        BOOL            IfsProvider,
        BOOL            InstallOverAll
        )
{
    LPWSCINSTALLPROVIDERANDCHAINS lpInstallProviderAndChains;
    WSAPROTOCOL_INFOW *protocolList = NULL;
    WSAPROTOCOL_INFOW *pEnumProviders = NULL;
    HMODULE hMod = NULL;
    DWORD dwEntryCount;
    char *lpInstallFunction = NULL;
    INT iEnumProviderCount;
    int rc, i, j, error;
    

    rc = SOCKET_ERROR;

    //
    // Dynamically load the function in order for this installer to run properly
    // on downlevel OSes
    //
    hMod = LoadLibrary("ws2_32.dll");
    if ( NULL == hMod )
    {
        fprintf(stderr, "Unable to load ws2_32.dll!\n");
        goto cleanup;
    }

#ifdef _WIN64
    if ( ( eCatalog == LspCatalog32Only ) || ( eCatalog == LspCatalog64Only ) )
    {
        fprintf(stderr, "New install API always installs into both catalogs!\n");
        goto cleanup;
    }
    else 
    {
        lpInstallFunction = "WSCInstallProviderAndChains64_32";
    }
#else
    UNREFERENCED_PARAMETER(lpszLspPathAndFile32);

    if ( ( eCatalog == LspCatalog64Only) || ( eCatalog == LspCatalogBoth ) )
    {
        fprintf(stderr, "Cannot install into 64-bit catalog from 32-bit process\n");
        goto cleanup;
    }
    else
    {
        lpInstallFunction = "WSCInstallProviderAndChains";
    }
#endif

    // Load the new install function
    lpInstallProviderAndChains = (LPWSCINSTALLPROVIDERANDCHAINS) GetProcAddress( 
            hMod,
            lpInstallFunction
            );
    if ( NULL == lpInstallProviderAndChains )
    {
        fprintf( stderr, "InstallLsp: Unable to load WSCInstallProviderAndChains function!\n");
        rc = SOCKET_ERROR;
        goto cleanup;
    }

    if ( InstallOverAll )
    {
        //
        // Install over all unique BSPs on the system so pass NULL for the provider list
        //

        rc = lpInstallProviderAndChains(
                providerGuid,
                lpszLspPathAndFile,
#ifdef _WIN64
                (lpszLspPathAndFile32[0] == '\0' ? lpszLspPathAndFile : lpszLspPathAndFile32),
#endif
                lpszLspName,
                ( IfsProvider ? XP1_IFS_HANDLES : 0 ),
                NULL,
                NULL,
                NULL,
               &error
                );
        if ( SOCKET_ERROR == rc )
        {
            fprintf(stderr, "InstallProviderVista: %s failed: %d\n", 
                    lpInstallFunction, error );
            goto cleanup;
        }
    }
    else
    {
        //
        // User specified a subset of providers to install over so build a list of
        //    the corresponding WSAPROTOCOL_INFOW structures to pass to install call
        //

        protocolList = (WSAPROTOCOL_INFOW *) LspAlloc( sizeof(WSAPROTOCOL_INFOW) *
                dwCatalogIdArrayCount, &error);
        if ( NULL == protocolList )
        {
            fprintf(stderr, "InstallProviderVista: Out of memory!\n");
            rc = SOCKET_ERROR;
            goto cleanup;
        }

        pEnumProviders = EnumerateProviders( eCatalog, &iEnumProviderCount );
        if ( NULL == pEnumProviders )
        {
            fprintf(stderr, "InstallProviderVista: Unable to enumerate catalog!\n");
            rc = SOCKET_ERROR;
            goto cleanup;
        }

        // Build a list of protocol structures to layer over
        dwEntryCount = 0;
        for(i=0; i < (int)dwCatalogIdArrayCount ;i++)
        {
            for(j=0; j < iEnumProviderCount ;j++)
            {
                if ( pdwCatalogIdArray[i] == pEnumProviders[j].dwCatalogEntryId )
                {
                    memcpy( &protocolList[dwEntryCount++], &pEnumProviders[j], sizeof(WSAPROTOCOL_INFOW) );
                }
            }
        }

        rc = lpInstallProviderAndChains(
                providerGuid,
                lpszLspPathAndFile,
#ifdef _WIN64
                lpszLspPathAndFile,
#endif
                lpszLspName,
                ( IfsProvider ? XP1_IFS_HANDLES : 0 ),
                protocolList,
                dwEntryCount,
                NULL,
               &error
                );
        if ( SOCKET_ERROR == rc )
        {
            fprintf(stderr, "InstallProviderVista: %s failed: %d\n", 
                    lpInstallFunction, error );
            goto cleanup;
        }
    }

    rc = NO_ERROR;

cleanup:

    if ( NULL != hMod )
        FreeLibrary( hMod );

    if ( NULL != pEnumProviders )
        FreeProviders( pEnumProviders );

    if ( NULL != protocolList )
        LspFree( protocolList );

    return rc;
}
