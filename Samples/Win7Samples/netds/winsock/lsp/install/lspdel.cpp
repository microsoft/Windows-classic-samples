//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (C) 2004  Microsoft Corporation.  All Rights Reserved.
//
// Module Name: lspdel.cpp
//
// Description:
//
//    This file contains the routines used to remove provider entries from the 
//    Winsock catalog. See instlsp.cpp for more information on running this code.
//
#include "instlsp.h"

//
// Function: RemoveAllLayeredEntries
//
// Description:
//    In the event that the layered entries become totally hosed up. This
//    function will remove any non base provider.
//
int RemoveAllLayeredEntries(
    WINSOCK_CATALOG Catalog         // Catalog to remove all LSPs from
    )
{
    WSAPROTOCOL_INFOW   *pProviders = NULL,
                        *pAssociated = NULL;
    WCHAR                szGuidString[ MAX_PATH ];
    LSP_ENTRY           *pLspMap = NULL;
    INT                  iProviderCount,
                         iAssociatedCount,
                         iMaxCount,
                         iLspCount = 0,
                         Status,
                         rc,
                         i, j, k;

    Status = SOCKET_ERROR;

    // First enumerate the catalog
    pProviders = EnumerateProviders( Catalog, &iProviderCount );
    if ( NULL == pProviders )
    {
        fprintf(stderr, "RemoveAllLayeredEntries: Unable to enumerate catalog!\n");
        goto cleanup;
    }

    // Build a mapping of the LSPs installed on the system
    pLspMap = BuildLspMap( pProviders, iProviderCount, &iLspCount );
    if ( NULL == pLspMap )
    {
        printf("\nNo LSPs to remove!\n");
        goto cleanup;
    }

    iMaxCount = MaxLayeredChainCount( pLspMap, iLspCount );

    pAssociated = (WSAPROTOCOL_INFOW *) LspAlloc(
            sizeof( WSAPROTOCOL_INFOW ) * iMaxCount,
           &rc
            );
    if ( NULL == pAssociated )
    {
        fprintf( stderr, "RemoveAllLayeredEntries: LspAlloc failed: %d\n", rc );
        goto cleanup;
    }

    printf( "\n%d LSPs installed:\n", iLspCount );
    for(i=0; i < iLspCount ;i++)
    {
        if ( pLspMap[ i ].OrphanedEntries != TRUE )
        {
            printf("   %d: %ws with %d layered entries\n",
                    pLspMap[ i ].DummyEntry.dwCatalogEntryId, 
                    pLspMap[ i ].DummyEntry.szProtocol,
                    pLspMap[ i ].Count
                    );
        }
        else
        {
            printf("   Orphaned LSP chain entries:\n");
            for(j=0; j < pLspMap[ i ].Count ;j++)
            {
                printf("\t   %d %ws\n",
                    pLspMap[ i ].LayeredEntries[ j ].dwCatalogEntryId,
                    pLspMap[ i ].LayeredEntries[ j ].szProtocol
                    );
            }
        }
    }

    printf("\nRemoving LSPs...\n\n");

    for(i=0; i < iLspCount ;i++)
    {
        if ( pLspMap[ i ].OrphanedEntries != TRUE )
        {
            // First remove the dummy entry
            printf( "Removing dummy entry for: %ws\n", pLspMap[ i ].DummyEntry.szProtocol );

            rc = DeinstallProvider( Catalog, &pLspMap[ i ].DummyEntry.ProviderId );

            if ( pLspMap[ i ].LayeredGuidCount > 0 )
                printf("Removing the associated layered entries with GUIDs:\n");

            for(j=0; j < pLspMap[ i ].LayeredGuidCount ;j++)
            {
                StringFromGUID2( pLspMap[ i ].LayeredGuids[ j ], szGuidString, MAX_PATH-1 );
                printf( "\tGUID: %ws\n", szGuidString );

                iAssociatedCount = iMaxCount;

                // Get a list of all providers under this GUID so we can print it out
                rc = GetLayeredEntriesByGuid(
                        pAssociated,
                        &iAssociatedCount,
                        pLspMap[ i ].LayeredEntries, 
                        pLspMap[ i ].Count,
                        &pLspMap[ i ].LayeredGuids[ j ]
                        );
                if ( SOCKET_ERROR == rc )
                {
                    fprintf( stderr, "RemoveAllLayeredProviders: GetLayeredEntriesByGuid failed!\n" );
                    goto cleanup;
                }

                for(k=0; k < iAssociatedCount ;k++)
                {
                    printf("\t  %d: %ws\n", 
                            pAssociated[ k ].dwCatalogEntryId,
                            pAssociated[ k ].szProtocol
                          );
                }

                rc = DeinstallProvider( Catalog, &pLspMap[ i ].LayeredGuids[ j ] );
                if ( SOCKET_ERROR == rc )
                {
                    fprintf( stderr, "RemoveAllLayeredProviders: DeinstallProvider failed!\n" );
                }
                else
                {
                    printf( "   Uninstalled providers for %ws\n", szGuidString );
                }
            }
        }
        else
        {
            printf("Removing the following orphaned entries:\n");
            for(j=0; j < pLspMap[ i ].Count ;j++)
            {
                printf("\t  %d: %ws\n",
                        pLspMap[ i ].LayeredEntries[ j ].dwCatalogEntryId,
                        pLspMap[ i ].LayeredEntries[ j ].szProtocol
                        );
            }

            for(j=0; j < pLspMap[ i ].LayeredGuidCount ;j++)
            {
                StringFromGUID2( pLspMap[ i ].LayeredGuids[ j ], szGuidString, MAX_PATH-1 );

                rc = DeinstallProvider( Catalog, &pLspMap[ i ].LayeredGuids[ j ] );
                if ( SOCKET_ERROR == rc )
                {
                    fprintf( stderr, "RemoveAllLayeredProviders: DeinstallProvider failed!\n");
                }
                else
                {
                    printf("\tUninstalled providers for %ws\n", szGuidString );
                }
            }
        }
    }

    Status = NO_ERROR;

cleanup:

    if ( NULL != pProviders )
        FreeProviders( pProviders );

    if ( NULL != pLspMap )
        FreeLspMap( pLspMap, iLspCount );

    if ( NULL != pAssociated )
        LspFree( pAssociated );

    return Status;
}

//
// Function: DeinstallProvider
//
// Description:
//    This is a wrapper for the WSCDeinstallProvider* functions. Depending on
//    which Winsock catalog is specified, this routine calls the right uninstall
//    function.
//
int 
DeinstallProvider(
    WINSOCK_CATALOG Catalog,            // Which Winsock catalog to operate on
    GUID           *Guid                // GUID of provider to remove
    )
{
    INT     ErrorCode,
            rc;

#ifdef _WIN64
    if ( LspCatalogBoth == Catalog )
    {
        // Remove from 64-bit catalog
        rc = WSCDeinstallProvider( Guid, &ErrorCode );
        if ( SOCKET_ERROR == rc )
        {
            fprintf( stderr, "DeinstallProvider: WSCDeinstallProvider failed: %d\n", 
                    ErrorCode 
                    );
        }

        // Remove from the 32-bit catalog
        rc = WSCDeinstallProvider32( Guid, &ErrorCode );
        if ( SOCKET_ERROR == rc )
        {
            fprintf( stderr, "DeinstallProvider: WSCDeinstallProvider32 failed: %d\n", 
                    ErrorCode 
                    );
        }
    }
    else if ( LspCatalog64Only == Catalog )
    {
        // Remove from 64-bit catalog
        rc = WSCDeinstallProvider( Guid, &ErrorCode );
        if ( SOCKET_ERROR == rc )
        {
            fprintf( stderr, "DeinstallProvider: WSCDeinstallProvider failed: %d\n", 
                    ErrorCode 
                    );
        }
    }
    else
    {
        // Remove from the 32-bit catalog
        rc = WSCDeinstallProvider32( Guid, &ErrorCode );
        if ( SOCKET_ERROR == rc )
        {
            fprintf( stderr, "DeinstallProvider: WSCDeinstallProvider32 failed: %d\n", 
                    ErrorCode 
                    );
        }
    }
#else
    if ( LspCatalog32Only == Catalog )
    {
        // Remove from the 32-bit catalog
        rc = WSCDeinstallProvider( Guid, &ErrorCode );
        if ( SOCKET_ERROR == rc )
        {
            fprintf( stderr, "DeinstallProvider: WSCDeinstallProvider failed: %d\n", 
                    ErrorCode 
                    );
        }
    }
    else
    {
        fprintf( stderr, "Unable to remove providers in 64-bit catalog from 32-bit process!\n" );
        return SOCKET_ERROR;
    }
#endif

    return NO_ERROR;
}

//
// Function: UpdateProvider
// 
// Description:
//    This function is a wrapper for the WSCUpdateProvider function and calls the
//    correct version depending on the Winsock catalog being manipulated.
//
int
UpdateProvider(
    WINSOCK_CATALOG     Catalog,            // Catalog to perform the udpate in
    LPGUID              ProviderId,         // Guid of provider(s) to update
    WCHAR              *DllPath,            // DLL path of LSP being updated
    WSAPROTOCOL_INFOW  *ProtocolInfoList,   // Array of provider structures to update
    DWORD               NumberOfEntries,    // Number of providers in the array
    LPINT               lpErrno             // Error value returned on failure
    )
{
    int     rc = SOCKET_ERROR;

#ifdef _WIN64
    if ( LspCatalog64Only == Catalog )
    {
        rc = fnWscUpdateProvider(
                ProviderId,
                DllPath,
                ProtocolInfoList,
                NumberOfEntries,
                lpErrno
                );
    }
    else if ( LspCatalog32Only == Catalog )
    {
        rc = fnWscUpdateProvider32(
                ProviderId,
                DllPath,
                ProtocolInfoList,
                NumberOfEntries,
                lpErrno
                );
    }
#else
    if ( LspCatalog32Only == Catalog )
    {
        rc = fnWscUpdateProvider(
                ProviderId,
                DllPath,
                ProtocolInfoList,
                NumberOfEntries,
                lpErrno
                );
    }
    else
    {
        fprintf( stderr, "UpdateProvider: Unable to manipulate 64-bit catalog from a 32"
                "-bit process\n" );
    }
#endif

    if ( SOCKET_ERROR == rc )
    {
        fprintf( stderr, "UpdateProvider: WSCUpdateProvider failed: %d\n",
                *lpErrno );
    }

    return rc;
}

//
// Function: RemoveProvider
//
// Description:
//    This function removes a layered provider. Things can get tricky if
//    we're removing a layered provider which has been layered over by 
//    another provider. This routine first creates the LSP map to determine
//    if other LSPs on the system reference the LSP we want to remove. If
//    there are we must fix those LSPs before deleting the target LSP.
//    If we're on a platform that supports WSCUpdateProvider its simply a
//    matter of removing any reference to the target LSP's layered protocol
//    chains and the dummy hidden entry. 
//
//    If we're not on a WSCUpdateProvider enabled system, then its very tricky.
//    We must uninstall the dependent LSPs first followed by reinstalling them
//    in the same order they were originally installed. For example if LSP1,
//    LSP2, and LSP3 are installed (in that order) and this routine is invoked
//    to remove LSP1, we must uninstall LSP3 and LSP2 followed by re-installing 
//    (LSP2 first then LSP3). For each LSP added back we must fix up the protocol
//    chains of the next higher LSP so the reference the new catalog IDs (since
//    the action of installing an LSP assigns a new catalog ID).
//
//    NOTE: If WSCUpdateProvider is not supported there is the possiblity of
//          another process changing the Winsock catalog at the same time we're
//          trying to fix it back up. If this occurs it is possible for the 
//          corruption to occur.
//
int 
RemoveProvider(
    WINSOCK_CATALOG Catalog,            // Catalog to remove an LSP from
    DWORD           dwProviderId        // Catalog ID of LSPs hidden entry
    )
{
    WSAPROTOCOL_INFOW   *pProvider = NULL,
                        *pLayeredEntries = NULL;
    LSP_ENTRY           *pLspMap = NULL,
                        *pLspMapEntryDel = NULL;
    DWORD               *pdwCatalogOrder = NULL;
    INT                  iProviderCount = 0,
                         iLayerCount = 0,
                         iLspCount = 0,
                         ErrorCode,
                         Status,
                         rc, 
                         i, j, k, l;

    Status = SOCKET_ERROR;

    // Enumerate the catalog
    pProvider = EnumerateProviders( Catalog, &iProviderCount );
    if ( pProvider == NULL )
    {
        fprintf( stderr, "RemoveProvider: Unable to enumerate catalog!\n" );
        goto cleanup;
    }

    // Allocate an array to save of the provider order in case we have to
    //    do uninstall and reinstall providers
    pdwCatalogOrder = (DWORD *) LspAlloc(
            sizeof( DWORD ) * iProviderCount,
           &ErrorCode
            );
    if ( NULL == pdwCatalogOrder )
    {
        fprintf( stderr, "RemoveProvider: LspAlloc failed: %d\n", ErrorCode );
        goto cleanup;
    }

    for(i=0; i < iProviderCount ;i++)
    {
        pdwCatalogOrder[ i ] = pProvider[ i ].dwCatalogEntryId;
    }

    // Build a map of the LSPs installed on the system
    pLspMap = BuildLspMap( pProvider, iProviderCount, &iLspCount );
    if ( NULL == pLspMap )
    {
        fprintf( stderr, "RemoveProvider: Unable to build LSP map!\n" );
        goto cleanup;
    }

    // Validate the catalog entry ID to remove
    pLspMapEntryDel = NULL;

    for(i=0; ( i < iLspCount ) && ( NULL == pLspMapEntryDel ) ;i++)
    {
        if ( dwProviderId == pLspMap[ i ].DummyEntry.dwCatalogEntryId )
        {
            pLspMapEntryDel = &pLspMap[ i ];
        }
        else
        {
            for(j=0; j < pLspMap[ i ].Count ;j++)
            {
                if ( dwProviderId == pLspMap[ i ].LayeredEntries[ j ].dwCatalogEntryId )
                {
                    // In this case the user supplied the catalog ID of an LSP protocol
                    // chain entry -- not the hidden layered entry (dummy). Here we'll
                    // reset the dwProviderId to that of the dummy hidden entry.
                    //
                    if ( pLspMap[ i ].OrphanedEntries != TRUE )
                    {
                        printf( "Catalog ID %d is a layered protocol entry and not the hidden\n"
                                "provider representing the entire LSP. The LSP which owns this\n"
                                "provider is ID %d (%ws). This entire LSP will be removed!\n",
                                dwProviderId,
                                pLspMap[ i ].DummyEntry.dwCatalogEntryId,
                                pLspMap[ i ].DummyEntry.szProtocol
                              );
                        dwProviderId = pLspMap[ i ].DummyEntry.dwCatalogEntryId;
                        pLspMapEntryDel = &pLspMap[ i ];
                    }
                    else
                    {
                        printf( "Catalog ID %d is one of %d orphaned protocol entries.\n"
                                "These entries could be causing serious problems and\n"
                                "will be removed. The following providers are to be\n"
                                "deleted:\n",
                                pLspMap[ i ].LayeredEntries[ j ].dwCatalogEntryId,
                                pLspMap[ i ].Count
                                );
                        for(k=0; k < pLspMap[ i ].Count ;k++)
                        {
                            printf("   %d: %ws\n",
                                    pLspMap[ i ].LayeredEntries[ k ].dwCatalogEntryId,
                                    pLspMap[ i ].LayeredEntries[ k ].szProtocol
                                    );
                        }
                        pLspMapEntryDel = &pLspMap[ i ];
                    }
                    break;
                }
            }
        }
    }

    // Make sure we found a provider to remove
    if ( NULL == pLspMapEntryDel )
    {
        fprintf( stderr, "\n\nError! Invalid Winsock catalog ID supplied: %d\n",
                dwProviderId
                );
        goto cleanup;
    }

    //
    // Print which entries are being removed
    //

    printf( "\nThe following LSP entries will be removed:\n" );
    if ( pLspMapEntryDel->OrphanedEntries != TRUE )
    {
        printf( "LSP Hidden ID: %6d Name %ws\n",
                pLspMapEntryDel->DummyEntry.dwCatalogEntryId,
                pLspMapEntryDel->DummyEntry.szProtocol
                );
    }
    else
    {
        printf( "Orphaned LSP protocol chain entries:\n");
    }
    for(i=0; i < pLspMapEntryDel->Count ;i++)
    {
        printf( "LSP Layer  ID: %6d Name %ws\n",
                pLspMapEntryDel->LayeredEntries[ i ].dwCatalogEntryId,
                pLspMapEntryDel->LayeredEntries[ i ].szProtocol
                );
    }

    printf( "\n\nTo remove press a key, otherwise CTRL+C now! ");
    getchar();
    printf( "\n" );

    ErrorCode = NO_ERROR;

    if ( 0 != pLspMapEntryDel->DependentCount )
    {
        int iLspIdx;

        printf( "\n\nOther LSPs are dependent on this one! "
                "Additional cleanup is required..\n\n" );

        for(i=0; i < pLspMapEntryDel->DependentCount ;i++)
        {
            iLspIdx =  pLspMapEntryDel->DependentLspIndexArray[ i ];

            printf( "Fixing LSP index %d: %ws\n", 
                    pLspMap[ iLspIdx ].DummyEntry.dwCatalogEntryId,
                    pLspMap[ iLspIdx ].DummyEntry.szProtocol
                    );

            // Remove any reference to the deleted LSPs dummy catalog ID
            for(j=0; j < pLspMap[ iLspIdx ].Count ;j++)
            {
                if ( IsIdInChain( &pLspMap[ iLspIdx ].LayeredEntries[ j ],
                            pLspMapEntryDel->DummyEntry.dwCatalogEntryId ) 
                   )
                {
                    printf( "Removing ID %d from layered chain %d: %ws\n",
                            pLspMapEntryDel->DummyEntry.dwCatalogEntryId,
                            pLspMap[ iLspIdx ].LayeredEntries[ j ].dwCatalogEntryId,
                            pLspMap[ iLspIdx ].LayeredEntries[ j ].szProtocol
                            );

                    // Remove the deleted LSPs ID from the chain
                    rc = RemoveIdFromChain(
                           &pLspMap[ iLspIdx ].LayeredEntries[ j ],
                            pLspMapEntryDel->DummyEntry.dwCatalogEntryId
                            );
                    if ( FALSE == rc )
                    {
                        fprintf( stderr, "RemoveProvider: ID not found in chain!\n" );
                        continue;
                    }

                    pLspMap[ iLspIdx ].LayerChanged[ j ] = TRUE;
                }
            }

            // Remove any reference to the deleted LSPs layered entries catalog
            // IDs from the layers of the dependent LSP
            for(l=0; l < pLspMapEntryDel->Count ;l++)
            {
                for(j=0; j < pLspMap[ iLspIdx ].Count ;j++)
                {
                    if ( IsIdInChain( &pLspMap[ iLspIdx ].LayeredEntries[ j ],
                            pLspMapEntryDel->LayeredEntries[ l ].dwCatalogEntryId )
                       )
                    {
                        printf( "Removing ID %d from layered chain %d: %ws\n",
                                pLspMapEntryDel->DummyEntry.dwCatalogEntryId,
                                pLspMap[ iLspIdx ].LayeredEntries[ j ].dwCatalogEntryId,
                                pLspMap[ iLspIdx ].LayeredEntries[ j ].szProtocol
                                );

                        // Remove the deleted LSPs ID from the chain
                        rc = RemoveIdFromChain(
                               &pLspMap[ iLspIdx ].LayeredEntries[ j ],
                                pLspMapEntryDel->LayeredEntries[ l ].dwCatalogEntryId
                                );
                        if ( FALSE == rc )
                        {
                            fprintf( stderr, "RemoveProvider: ID not found in chain!\n" );
                            continue;
                        }

                        pLspMap[ iLspIdx ].LayerChanged[ j ] = TRUE;
                    }
                }
            }
        }

        //
        // All dependent LSPs should no longer reference any of the LSPs IDs which is
        //    to be removed. Now we must write our changes back to the catalog. Life
        //    is easy if we're on a system that supports WSCUpdateProvider.
        //

        if ( NULL != fnWscUpdateProvider )
        {
            //
            // Life is good, simply call UpdateProvider on each entry in the LSP map
            //    that was updated.
            //
            for(i=0; i < pLspMapEntryDel->DependentCount ;i++)
            {
                iLspIdx = pLspMapEntryDel->DependentLspIndexArray[ i ];

                for(j=0; j < pLspMap[ iLspIdx ].Count; j++)
                {
                    if ( TRUE == pLspMap[ iLspIdx ].LayerChanged[ j ] )
                    {
                        rc = UpdateProvider(
                                Catalog,
                               &pLspMap[ iLspIdx ].LayeredEntries[ j ].ProviderId,
                                pLspMap[ iLspIdx ].wszLspDll,
                               &pLspMap[ iLspIdx ].LayeredEntries[ j ],
                                1,
                               &ErrorCode
                                );
                    }
                }
            }
        }
        else        // fnWscUpdateProvider == NULL
        {
            int                 MaxLayers = 0;

            //
            // Life isn't so good. We need to remove all dependent LSPs first in the
            //    reverse order they were installed so that if something fails, we
            //    won't leave the catalog in a bad state. Then we need to reinstall
            //    them in the same order they were originally installed and fix any
            //    of the remaining dependent LSPs to reference the correct catalog IDs
            //    before they are also reinstalled.
            //

            // Find the maximum protocol chain length of all the LSPs since we need
            //    scratch space. We do the allocation first before making changes to
            //    the catalog.
            MaxLayers = MaxLayeredChainCount(
                    pLspMap,
                    iLspCount
                    );
            
            pLayeredEntries = (WSAPROTOCOL_INFOW *) LspAlloc(
                    sizeof( WSAPROTOCOL_INFOW ) * MaxLayers,
                   &ErrorCode
                    );
            if ( NULL == pLayeredEntries )
            {
                fprintf( stderr, "RemoveProvider: LspAlloc failed: %d\n",
                        ErrorCode );
                goto cleanup;
            }

            // Remove the dependent LSPs in reverse order. NOTE: We don't have to
            //    remove the dummy hidden entries since there is no information
            //    in those providers that need updating.
            for(i=0; i < pLspMapEntryDel->DependentCount ;i++)
            {
                iLspIdx = pLspMapEntryDel->DependentLspIndexArray[ i ];

                for(j=0; j < pLspMap[ iLspIdx ].LayeredGuidCount ;j++)
                {
                    rc = DeinstallProvider(
                            Catalog,
                           &pLspMap[ iLspIdx ].LayeredGuids[ j ]
                            );
                    if ( SOCKET_ERROR == rc )
                    {
                        fprintf( stderr, 
                                "RemoveProvider: An error occured trying to remove an LSP.\n"
                                "\t\tThis may be due to another process changing the Catalog\n"
                                "\t\tAborting...\n"
                                );
                        goto cleanup;
                    }
                }
            }

            // All the dependent LSP layers have been removed, now add them
            // back in reverse order
            for(i=pLspMapEntryDel->DependentCount-1; i >= 0 ;i--)
            {
                iLspIdx = pLspMapEntryDel->DependentLspIndexArray[ i ];

                // Install the layered entries
                for(j=0; j < pLspMap[ iLspIdx ].LayeredGuidCount ;j++)
                {
                    iLayerCount = MaxLayers;

                    rc = GetLayeredEntriesByGuid(
                            pLayeredEntries,
                           &iLayerCount,
                            pLspMap[ iLspIdx ].LayeredEntries, 
                            pLspMap[ iLspIdx ].Count,
                           &pLspMap[ iLspIdx ].LayeredGuids[ j ]
                            );

                    rc = InstallProvider(
                            Catalog, 
                           &pLspMap[ iLspIdx ].LayeredGuids[ j ],
                            pLspMap[ iLspIdx ].wszLspDll,
                            pLayeredEntries,
                            iLayerCount
                            );

                }

                // Enumerate catalog to find new IDs

                DWORD ProviderLen = iProviderCount * sizeof( WSAPROTOCOL_INFOW );

                int NewProviderCount = EnumerateProvidersExisting( 
                        Catalog, 
                        pProvider, 
                       &ProviderLen
                        );
                if ( SOCKET_ERROR == NewProviderCount )
                {
                    fprintf( stderr, "RemoveProvider: EnumerateProvidersExisting failed: %d\n",
                            GetLastError() );
                }

                // Update the old references to the new
                MapNewEntriesToOld(
                       &pLspMap[ iLspIdx ],
                        pProvider,
                        NewProviderCount
                        );

                // Update the provider order array with the new provider values
                UpdateProviderOrder(
                       &pLspMap[ iLspIdx ],
                        pdwCatalogOrder,
                        iProviderCount
                        );
                
                // For the remaining LSPs which we still need to install, update any
                //    references to the removed LSPs with their new IDs
                for(k=i-1; k >= 0 ;k--)
                {
                    int iLspIdx2 = pLspMapEntryDel->DependentLspIndexArray[ k ];

                    printf( "Updating IDs for index %d\n", iLspIdx2 );

                    for(l=0; l < pLspMap[ iLspIdx ].Count ;l++)
                    {
                        UpdateLspMap(
                               &pLspMap[ iLspIdx2 ],
                                pLspMap[ iLspIdx ].LayeredEntries[ l ].dwCatalogEntryId,
                                pLspMap[ iLspIdx ].LayeredEntries[ l ].dwProviderReserved
                                );
                    }
                }
            }

            // Reorder the catalog back to what it was before. Since we've added
            //    back all the LSPs we removed earlier, the catalog should be the
            //    same size as when we started.
            rc = WriteProviderOrder(
                    Catalog,
                    pdwCatalogOrder,
                    iProviderCount,
                   &ErrorCode
                    );
            if ( SOCKET_ERROR == rc )
            {
                fprintf( stderr, "RemoveProvider: WriteProviderOrder failed: %d\n",
                        ErrorCode );
            }
        }
    }

    //
    // Now all dependencies have been fixed, remove the specified provider
    //

    // Remove the layered protocol entries
    for(i=0; i < pLspMapEntryDel->LayeredGuidCount ;i++)
    {
        rc = DeinstallProvider(
                Catalog,
               &pLspMapEntryDel->LayeredGuids[ i ]
                );
    }

    // Remove the dummy entry
    rc = DeinstallProvider(
            Catalog,
           &pLspMapEntryDel->DummyEntry.ProviderId
            );

    Status = NO_ERROR;

cleanup:

    //
    // Cleanup allocations
    //

    if ( NULL != pLayeredEntries )
        LspFree( pLayeredEntries );

    if ( NULL != pProvider )
        FreeProviders(pProvider);

    if ( NULL != pLspMap )
        FreeLspMap( pLspMap, iLspCount );

    if ( NULL != pdwCatalogOrder )
        LspFree( pdwCatalogOrder );

    return Status;
}
