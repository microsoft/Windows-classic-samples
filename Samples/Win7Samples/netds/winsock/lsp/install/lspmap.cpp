//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (C) 2004  Microsoft Corporation.  All Rights Reserved.
//
// Module Name: lspmap.cpp
//
// Description:
//
//    This file contains the routines used to create a map of the LSPs installed.
//    This map is used to order the relationship between LSPs. It is used by the 
//    LSP uninstaller code to determine which LSPs are dependent on each other.
//    Winsock catalog. See instlsp.cpp for more information on running this code.
//
#include "instlsp.h"

////////////////////////////////////////////////////////////////////////////////
//
// Function Implementation
//
////////////////////////////////////////////////////////////////////////////////

//
// Function: PrintProviders
//
// Description: 
//    This function prints out each entry in the Winsock catalog and its
//    catalog ID if the parameter, bLayeredOnly, is FALSE. If TRUE then
//    print only those layered catalog entries.
//
void 
PrintProviders(
    WINSOCK_CATALOG Catalog, 
    BOOL            bLayeredOnly, 
    BOOL            bVerbose
    )
{
    WSAPROTOCOL_INFOW  *pProtocolInfo = NULL;
    INT                 iProtocolCount = 0,
                        i;

    // Enumerate catalog and print it
	pProtocolInfo = EnumerateProviders( Catalog, &iProtocolCount );
    if ( NULL == pProtocolInfo )
    {
        fprintf( stderr, "PrintProviders: Unable to enumerate catalog!\n" );
        goto cleanup;
    }

    for(i=0; i < iProtocolCount ;i++)
    {
        if ( FALSE == bLayeredOnly )
        {
            // Print all providers
            if ( TRUE == bVerbose )
                PrintProtocolInfo( &pProtocolInfo[ i ] );
            else
                printf("%04d - %S\n", 
                        pProtocolInfo[ i ].dwCatalogEntryId,
                        pProtocolInfo[ i ].szProtocol
                        );
        }
        else if ( LAYERED_PROTOCOL == pProtocolInfo[ i ].ProtocolChain.ChainLen )
        {
            // Print only layered providers
            if ( TRUE == bVerbose )
                PrintProtocolInfo( &pProtocolInfo[ i ] );
            else
                printf("%04d - %S\n", 
                        pProtocolInfo[ i ].dwCatalogEntryId,
                        pProtocolInfo[ i ].szProtocol
                        );
        }
    }
    
cleanup:

    if ( NULL != pProtocolInfo )
        FreeProviders( pProtocolInfo );

    return;
}

//
// Function: BuildLspMap
//
// Description:
//    This routine builds a map of all LSPs installed according to what order
//    they are in the catalog. That is, the information returned will be ordered
//    in the way the LSPs need to be installed. For example if LSP1 is installed
//    over the base TCP and UDP providers and LSP2 is installed over LSP1, then 
//    this routine will return two LSP_ENTRY structures with LSP1 first followed
//    by LSP2. The algorithm for determining the order is to first sort by where
//    a base provider ID occurs in an LSP chain with lower numbered ones first.
//    For example, LSP1 layered directly over TCP will have a base ID (TCP) in
//    chain position 1 while LSP (layered over LSP1) will have the base ID in
//    chain index 2. This is the ChainOrder field (and it is the minimum value
//    for all layered providers). After this first sort, it is possible to have
//    several LSPs with the same ChainOrder value. Within these groupings the
//    entries are sorted by the maximum LSP chain length. Each LSP has a number
//    of layered providers each with its own chain (and the chains could be
//    different lengths). The MaxChainLength value is the longest chain length
//    of all providers belonging to a given LSP. Each grouping of LspOrder is then
//    sorted by MaxChainLengthin ascending order.
//
LSP_ENTRY *
BuildLspMap(
    WSAPROTOCOL_INFOW *pProviders,
    int                iProviderCount,
    int               *pLspCount
    )
{
    LSP_ENTRY *pLsps = NULL,
               lsptmp;
    DWORD     *pBaseList = NULL;
    int        iLspCount = 0,
               iSortLspCount = 0,
               iOrphanCount = 0,
               iBaseCount = 0,
               iProviderPathLen,
               ErrorCode,
               LspOrder,
               start,
               end,
               idx,
               rc,
               i, j, k;

    // Retrieve how many orphaned chain entries are present
    iOrphanCount = CountOrphanedChainEntries( pProviders, iProviderCount );

    // Retrieve the LSP count
    iSortLspCount = iLspCount = GetProviderCount( pProviders, iProviderCount, LAYERED_PROTOCOL );

    if ( ( 0 == iOrphanCount ) && ( 0 == iLspCount ) )
    {
        fprintf( stderr, "BuildLspMap: No LSP installed on the system!\n");
        goto cleanup;
    }

    // If orphaned entries are present, create another LSP_ENTRY and put all orphaned
    //      entries there.
    if ( iOrphanCount > 0 )
        iLspCount++;

    // Allocate space for our structure which represents the LSPs installed
    pLsps = (LSP_ENTRY *) LspAlloc(
            sizeof( LSP_ENTRY ) * iLspCount,
           &ErrorCode
            );
    if ( NULL == pLsps )
    {
        fprintf( stderr, "BuildLspMap: LspAlloc failed: %d\n", ErrorCode );
        goto cleanup;
    }

    // If orphaned entries are present, allocate space to hold them
    if ( iOrphanCount > 0 )
    {
        pLsps[ iLspCount-1 ].LayeredEntries = (WSAPROTOCOL_INFOW *)LspAlloc(
                sizeof(WSAPROTOCOL_INFOW) * iOrphanCount, &ErrorCode );
        if ( NULL == pLsps[ iLspCount-1 ].LayeredEntries )
        {
            fprintf( stderr, "BuildLspMap: LspAlloc failed: %d\n", ErrorCode );
            goto cleanup;
        }

        pLsps[ iLspCount-1 ].OrphanedEntries = TRUE;
        pLsps[ iLspCount-1 ].Count = iOrphanCount;

        //
        // Find the orphaned entries and save them off
        //
        idx = 0;
        for(i=0; i < iProviderCount ;i++)
        {
            // Only investigate protocol chain entries (i.e. chainlen > 1)
            if ( pProviders[ i ].ProtocolChain.ChainLen > 1 )
            {
                // Walk the catalog and look for the dummy entry (i.e. the ID in 
                //    chain entry 0)
                for(j=0; j < iProviderCount ;j++) 
                {
                    if ( i == j )
                        continue;

                    if ( pProviders[ i ].ProtocolChain.ChainEntries[ 0 ] ==
                         pProviders[ j ].dwCatalogEntryId )
                    {
                        break;
                    }
                }
                if ( j >= iProviderCount )
                {
                    // If j is past iProviderCount, no match was found so this is
                    //    an orphaned entry...save it off
                    memcpy( &pLsps[ iLspCount-1 ].LayeredEntries[ idx ],
                            &pProviders[ i ],
                             sizeof( WSAPROTOCOL_INFOW )
                          );
                    rc = AddGuidToLspEntry( &pLsps[ iLspCount-1 ], &pProviders[ i ].ProviderId,
                            &ErrorCode );
                    if ( SOCKET_ERROR == rc )
                    {
                        fprintf( stderr, "BuildLspMap: AddGuidToLspEntry failed: %d\n", ErrorCode );
                        goto cleanup;
                    }
                        
                    idx++;
                }
            }
        }
    }

    //
    // Build a list of the valid LSPs installed on the system
    //
    idx = 0;
    for(i=0; i < iProviderCount ;i++)
    {
        if ( LAYERED_PROTOCOL == pProviders[ i ].ProtocolChain.ChainLen )
        {
            // Copy the dummy entry
            memcpy( &pLsps[ idx ].DummyEntry, &pProviders[ i ], sizeof( WSAPROTOCOL_INFOW ) );

            // Get the DLL path
            iProviderPathLen = MAX_PATH-1;
            rc = WSCGetProviderPath(
                    &pLsps[ idx ].DummyEntry.ProviderId,
                     pLsps[ idx ].wszLspDll,
                    &iProviderPathLen,
                    &ErrorCode
                     );
            if ( SOCKET_ERROR == rc )
            {
                fprintf( stderr, "BuildLspMap: WSCGetProviderPath failed: %d\n", ErrorCode );
                goto cleanup;
            }

            //
            // Now go find all the layered entries associated with the dummy provider
            //

            // First get the count
            for(j=0; j < iProviderCount ;j++)
            {
                //
                // Compare only the first entry against the dummy ID. Otherwise, 
                //    we may pick up more than the provider's owned by this LSP 
                //    (it may pick up other providers layered over this LSP.
                //
                if ( ( pProviders[ j ].ProtocolChain.ChainLen > 1 ) &&
                     ( pProviders[ j ].ProtocolChain.ChainEntries[ 0 ] ==
                       pLsps[ idx ].DummyEntry.dwCatalogEntryId ) 
                   )
                // if ( IsIdInChain( &pProviders[ j ], pLsps[ idx ].DummyEntry.dwCatalogEntryId ) )
                {
                    pLsps[idx].Count++;
                }
            }

            // Allocate space
            pLsps[ idx ].LayeredEntries = (WSAPROTOCOL_INFOW *) LspAlloc(
                    sizeof( WSAPROTOCOL_INFOW ) * pLsps[ idx ].Count,
                   &ErrorCode
                    );
            if ( NULL == pLsps[ idx ].LayeredEntries )
            {
                fprintf( stderr, "BuildLspMap: LspAlloc failed: %d\n", ErrorCode );
                goto cleanup;
            }

            pLsps[ idx ].LayerChanged = (int *) LspAlloc(
                    sizeof( int ) * pLsps[ idx ].Count,
                   &ErrorCode
                    );
            if ( NULL == pLsps[ idx ].LayerChanged )
            {
                fprintf( stderr, "BuildLspMap: LspAlloc failed: %d\n", ErrorCode );
                goto cleanup;
            }

            // Now go find the entries
            pLsps[idx].Count = 0;
            for(j=0; j < iProviderCount ;j++)
            {
                if ( ( pProviders[ j ].ProtocolChain.ChainLen > 1 ) &&
                     ( pProviders[ j ].ProtocolChain.ChainEntries[ 0 ] ==
                       pLsps[ idx ].DummyEntry.dwCatalogEntryId ) 
                   )
                {
                    memcpy( 
                           &pLsps[ idx ].LayeredEntries[pLsps[ idx ].Count],
                           &pProviders[ j ],
                            sizeof( WSAPROTOCOL_INFOW )
                            );

                    pLsps[idx].MaxChainLength = MAX( 
                            pLsps[ idx ].MaxChainLength,
                            pLsps[ idx ].LayeredEntries[ pLsps[idx].Count ].ProtocolChain.ChainLen 
                            );

                    // Mark this entry as visited
                    pProviders[ j ].dwProviderReserved = 1;

                    // Keep track of how many GUIDs are used to install the layered entries
                    rc = AddGuidToLspEntry( &pLsps[ idx ], &pProviders[ j ].ProviderId, &ErrorCode );
                    if ( SOCKET_ERROR == rc )
                    {
                        fprintf( stderr, "BuildLspMap: AddGuidToLspEntry failed: %d\n", ErrorCode );
                        goto cleanup;
                    }

                    pLsps[ idx ].Count++;
                }
            }

            pLsps[ idx ].LspOrder = MAX_PROTOCOL_CHAIN;

            idx++;      // Increment index into the map
        }
    }

    //
    // We now have an array of "LSPs" -- now order them
    //

    // First get a list of base provider IDs
    iBaseCount = GetProviderCount( pProviders, iProviderCount, BASE_PROTOCOL );
    if ( 0 == iBaseCount )
    {
        fprintf( stderr, "BuildLspMap: GetProviderCount(BASE_PROTOCOL) returned zero!\n" );
        goto cleanup;
    }

    // Allocate space for the array of base provider ID's
    pBaseList = (DWORD *) LspAlloc(
            sizeof( DWORD ) * iBaseCount,
           &ErrorCode
            );
    if ( NULL == pBaseList )
    {
        fprintf( stderr, "BuildLspMap: HeapAlloc failed: %d\n", ErrorCode );
        goto cleanup;
    }

    //
    // Copy the base provider ID's to our array -- this array contains the catalog
    // IDs of only base providers which will be used next to determine the order
    // in which LSPs were installed.
    //
    idx = 0;
    for(i=0; i < iProviderCount ;i++)
    {
        if ( BASE_PROTOCOL == pProviders[ i ].ProtocolChain.ChainLen )
        {
            pBaseList[ idx++ ] = pProviders[ i ].dwCatalogEntryId;
        }
    }

    //
    // For each layered protocol entry of an LSP find the lowest index in the protocol
    // chain where a base provider resides. A protocol chain should always terminate
    // in a base provider.
    //
    for(LspOrder = 1; LspOrder < MAX_PROTOCOL_CHAIN ;LspOrder++)
    {
        for(i=0; i < iSortLspCount ;i++)
        {
            for(j=0; j < pLsps[ i ].Count ;j++)
            {
                for(k=0; k < iBaseCount ;k++)
                {
                    if ( pLsps[ i ].LayeredEntries[ j ].ProtocolChain.ChainEntries[ LspOrder ] ==
                         pBaseList[ k ] )
                    {
                        pLsps[ i ].LspOrder = MIN( pLsps[ i ].LspOrder, LspOrder );
                        break;
                    }
                }
            }
        }
    }

    //
    // Sort the entries according to the LspOrder field
    //
    for(i=0; i < iSortLspCount ;i++)
    {
        for(j=i; j < iSortLspCount ;j++)
        {
            if ( pLsps[ i ].LspOrder > pLsps[ j ].LspOrder )
            {
                // Exchange positions
                memcpy( &lsptmp,     &pLsps[ i ], sizeof( LSP_ENTRY ) );
                memcpy( &pLsps[ i ], &pLsps[ j ], sizeof( LSP_ENTRY ) );
                memcpy( &pLsps[ j ], &lsptmp,     sizeof( LSP_ENTRY ) );
            }
        }
    }

    //
    // Now need to sort by MaxChainLength withing the LspOrder groupings
    //
    for(LspOrder=1; LspOrder < MAX_PROTOCOL_CHAIN ;LspOrder++)
    {
        // Find the start and end positions within the array for the given
        // LspOrder value
        start = -1;
        end   = -1;

        for(i=0; i < iSortLspCount ;i++)
        {
            if ( pLsps[ i ].LspOrder == LspOrder )
            {
                start = i;
                break;
            }
        }

        //
        // Find the end position which is the LSP Map entry whose LspOrder
        // value doesn't match the current one. This will give us the range
        // of LSP entries whose LspOrder value is identical -- we need to
        // sort the LSPs of the same LspOrder according to the MaxChainLength
        //
        if ( -1 != start )
        {
            for(j=start; j < iSortLspCount ;j++)
            {
                if ( pLsps[ j ].LspOrder != LspOrder )
                {
                    end = j - 1;
                    break;
                }
            }
        }
        
        //
        // If the following is true then all entries have the same order
        // value. We still need to sort by MaxChainLength so set the end
        // to the last LSP 
        //
        if ( ( -1 != start ) && ( -1 == end ) )
        {
            end = iSortLspCount - 1;
        }

        if ( ( -1 != start ) && ( -1 != end ) )
        {
            for(i=start; i < end ;i++)
            {
                for(j=i; j < end ;j++)
                {
                    if ( pLsps[ i ].MaxChainLength > pLsps[ j ].MaxChainLength )
                    {
                        memcpy( &lsptmp,     &pLsps[ i ], sizeof( LSP_ENTRY ) );
                        memcpy( &pLsps[ i ], &pLsps[ j ], sizeof( LSP_ENTRY ) );
                        memcpy( &pLsps[ j ], &lsptmp,     sizeof( LSP_ENTRY ) );
                    }
                }
            }
        }
    }

    // Add the LSP dependency info to the map
    rc = LspDependencyCheck( pLsps, iSortLspCount );
    if ( SOCKET_ERROR == rc )
    {
        FreeLspMap( pLsps, iLspCount );
        pLsps = NULL;
        iLspCount = 0;
        goto cleanup;
    }

cleanup:
    
    if ( NULL != pLspCount )
        *pLspCount = iLspCount;

    if ( NULL != pBaseList )
        LspFree( pBaseList );

    return pLsps;
}

//
// Function: PrintLspMap
//
// Description:
//    Print the array of LSP_ENTRY structures to the console. The LSP_ENTRY
//    array is ordered in the same order the LSPs were installed.
//
void
PrintLspMap(
    LSP_ENTRY *pLspMap,
    int        iLspCount
    )
{
    WCHAR   szGuidString[ MAX_PATH ];
    int     i, j, k;

    if ( NULL == pLspMap )
    {
        printf( "\tNo LSPs currently installed\n\n" );
        goto cleanup;
    }
  
    for(i=0; i < iLspCount ;i++)
    {
        if ( pLspMap[ i ].OrphanedEntries != TRUE )
        {
            // Display the LSP name and its DLL (and path)
            printf( "%3d LSP: %ws   DLL '%ws' ID: %d\n", 
                    i, 
                    pLspMap[ i ].DummyEntry.szProtocol,
                    pLspMap[ i ].wszLspDll,
                    pLspMap[ i ].DummyEntry.dwCatalogEntryId
                    );

            // Display the GUIDs under which the layered entries of this LSP are installed
            printf( "\t LSP Installed under %d GUIDs\n", pLspMap[ i ].LayeredGuidCount );
            for(k=0; k < pLspMap[ i ].LayeredGuidCount ;k++)
            {
                StringFromGUID2( pLspMap[ i ].LayeredGuids[ k ], szGuidString, MAX_PATH-1 );
                printf( "\t\t%ws\n", szGuidString );
            }
        }
        else
        {
            printf("Orphaned layered chain entries:\n");
        }

        // Display the layered entries and the protocol chains
        for(j=0; j < pLspMap[ i ].Count ;j++)
        {
            printf( "\t Layer %-5d \"%ws\" \n\t       Chain %d [ ", 
                    pLspMap[ i ].LayeredEntries[ j ].dwCatalogEntryId,
                    pLspMap[ i ].LayeredEntries[ j ].szProtocol,
                    pLspMap[ i ].LayeredEntries[ j ].ProtocolChain.ChainLen
                    );

            for(k=0; k < pLspMap[ i ].LayeredEntries[ j ].ProtocolChain.ChainLen ;k++)
            {
                printf( "%d ", pLspMap[ i ].LayeredEntries[ j ].ProtocolChain.ChainEntries[ k ] );
            }
            printf( "]\n" );
        }

        // Display any LSPs which depend on this one (i.e. other LSPs layered over this one)
        printf( "\t Dependent LSPs:\n" );
        if ( pLspMap[ i ].DependentCount == 0 )
            printf( "\t\tNone\n");
        else
        {
            for(j=0; j < pLspMap[ i ].DependentCount ;j++)
            {
                printf("\t\t%d %ws\n",
                        pLspMap[ pLspMap[ i ].DependentLspIndexArray[ j ] ].DummyEntry.dwCatalogEntryId,
                        pLspMap[ pLspMap[ i ].DependentLspIndexArray[ j ] ].DummyEntry.szProtocol
                        );
            }
        }

        printf( "\n" );
    }

cleanup:

    return;
}

//
// Function: FreeLspMap
//
// Description:
//    Frees all memory associated with an LSP_ENTRY array.
//
void
FreeLspMap(
    LSP_ENTRY *pLspMap,
    int        iLspCount
    )
{
    int     i;

    for(i=0; i < iLspCount ;i++)
    {
        // Free the layered providers first
        if ( NULL != pLspMap[ i ].LayeredEntries )
            LspFree( pLspMap[ i ].LayeredEntries );

        if ( NULL != pLspMap[ i ].LayeredGuids )
            LspFree( pLspMap[ i ].LayeredGuids );

        if ( NULL != pLspMap[ i ].LayerChanged )
            LspFree( pLspMap[ i ].LayerChanged );

        if ( NULL != pLspMap[ i ].DependentLspIndexArray )
            LspFree( pLspMap[ i ].DependentLspIndexArray );
    }
    LspFree( pLspMap );
}

//
// Function: LspDependencyCheck
//
// Description:
//    This function determines the dependencies between LSPs. If LSP A has both
//    LSP B and LSP C layered over its entries, then LSP A has dependencies with
//    B and C such that if LSP A is removed B and C need to be fixed. For each LSP
//    installed, it check whether its dummy entry ID or one of its layered entry
//    IDs is referenced by any of the other LSPs in the system. If it is, the
//    DependentLspIndexArray is created to point to the index (within the LSP
//    map array) of the dependent entries.
//
int
LspDependencyCheck(
    LSP_ENTRY  *pLspMap,
    int         iLspCount
    )
{
    BOOL        bDependent;
    int         iCheckLspIndex = 0,
                ret = SOCKET_ERROR,
               *tmpArray = NULL,
                ErrorCode,
                i, j, k, l;

    // For each LSP entry, find its dependencies
    for(i=0; i < iLspCount ;i++)
    {
        iCheckLspIndex = i;

        // Search all other LSPs for dependencies on this entry
        for(j=0; j < iLspCount ;j++)
        {
            // Skip checking against the same one were currently looking at
            if ( j == iCheckLspIndex )
                continue;

            bDependent = FALSE;

            // Check the dummy catalog entry against all the chains for the LSP we're
            // currently looking at
            for(k=0; k < pLspMap[ j ].Count ;k++)
            {
                if ( IsIdInChain(
                           &pLspMap[ j ].LayeredEntries[ k ],
                            pLspMap[ iCheckLspIndex ].DummyEntry.dwCatalogEntryId )
                   )
                {
                    // Allocate an array for the dependent LSP indices
                    tmpArray = (int *) LspAlloc(
                            sizeof( int ) * ( pLspMap[ iCheckLspIndex ].DependentCount + 1),
                           &ErrorCode
                            );
                    if ( NULL == tmpArray )
                    {
                        fprintf( stderr, "CheckLspDependency: LspAlloc failed: %d\n", ErrorCode );
                        goto cleanup;
                    }

                    // If one already exists, copy the existing array into the new one
                    if ( NULL != pLspMap[ iCheckLspIndex ].DependentLspIndexArray )
                    {
                        memcpy( 
                                tmpArray + 1,
                                pLspMap[ iCheckLspIndex ].DependentLspIndexArray,
                                sizeof( int ) * pLspMap[ iCheckLspIndex ].DependentCount
                                );

                        // Free the existing array
                        LspFree( pLspMap[ iCheckLspIndex ].DependentLspIndexArray );
                    }
                    
                    // Assign the new array and increment the count
                    pLspMap[ iCheckLspIndex ].DependentLspIndexArray = tmpArray;
                    pLspMap[ iCheckLspIndex ].DependentLspIndexArray[ 0 ] = j;
                    pLspMap[ iCheckLspIndex ].DependentCount++;

                    bDependent = TRUE;
                }
            }

            //
            // If a dependency already exists, don't bother checking the layered protocol
            // chains for one
            //
            if ( TRUE == bDependent )
                continue;

            //
            // Now check whether each layered protocol entry ID is present in any
            // of the layered protocol entry chains of the LSP we're currently
            // looking at.
            //
            for(l=0; l < pLspMap[ iCheckLspIndex ].Count ;l++)
            {
                bDependent = FALSE;

                // Check against each layered entry
                for(k=0; k < pLspMap[ j ].Count ;k++ )
                {
                    if ( IsIdInChain(
                           &pLspMap[ j ].LayeredEntries[ k ],
                            pLspMap[ iCheckLspIndex ].LayeredEntries[ l ].dwCatalogEntryId )
                       )
                    {
                        {
                            tmpArray = (int *) LspAlloc(
                                    sizeof( int ) * ( pLspMap[ iCheckLspIndex ].DependentCount + 1),
                                    &ErrorCode
                                    );
                            if ( NULL == tmpArray )
                            {
                                fprintf( stderr, "CheckLspDependency: LspAlloc failed: %d\n", ErrorCode );
                                goto cleanup;
                            }

                            if ( NULL != pLspMap[ iCheckLspIndex ].DependentLspIndexArray )
                            {
                                memcpy( 
                                        tmpArray + 1,
                                        pLspMap[ iCheckLspIndex ].DependentLspIndexArray,
                                        sizeof( int ) * pLspMap[ iCheckLspIndex ].DependentCount
                                      );

                                LspFree( pLspMap[ iCheckLspIndex ].DependentLspIndexArray );
                            }

                            pLspMap[ iCheckLspIndex ].DependentLspIndexArray = tmpArray;
                            pLspMap[ iCheckLspIndex ].DependentLspIndexArray[ 0 ] = j;
                            pLspMap[ iCheckLspIndex ].DependentCount++;

                            bDependent = TRUE;
                            break;
                        }
                    }
                }

                if ( TRUE == bDependent )  
                    break;
            }
        }
    }
    
    ret = NO_ERROR;

cleanup:

    return ret;
}


//
// Function: UpdateLspMap
//
// Description:
//   This routine updates catalog entry IDs in the LSP map. In the event
//   WSCUpdateProvider is not available, the uninstaller must remove all
//   dependent LSPs and reinstall them. This requires updating any references
//   to these reinstalled LSPs to their new values (as reinstalling a provider
//   changes the catalog ID). After a provider is LSP, the uninstaller matches
//   the older provider to the new to obtain the new provider ID (via
//   MapNewEntriesToOld). This routine takes and old provider ID and updates all
//   references within the LSP map to the new provider's ID.
//
void
UpdateLspMap(
    LSP_ENTRY *pLspMap,
    DWORD      dwOldValue,
    DWORD      dwNewValue
    )
{
    int i, j;

    // Go through all providers beloging to this LSP
    for(i=0; i < pLspMap->Count ;i++)
    {
        // Go through the protocol chain and update references if they match
        for(j=0; j < pLspMap->LayeredEntries[ i ].ProtocolChain.ChainLen ;j++)
        {
            if ( pLspMap->LayeredEntries[ i ].ProtocolChain.ChainEntries[ j ] == 
                    dwOldValue 
               )
            {
                pLspMap->LayeredEntries[ i ].ProtocolChain.ChainEntries[ j ] = 
                        dwNewValue;
            }
        }
    }

    return;
}

//
// Function: MapNewEntriesToOld
//
// Description:
//    This routine is used by the uninstaller if WSCUpdateProvider is not available.
//    In this case, after one LSP is removed there may be others that referenced
//    that LSP. The uninstaller removes those entries and reinstalls them (and
//    removes the reference the the removed LSP). Once a dependent provider is
//    reinstalled we need to see if even other LSP references the reinstalled
//    provider and if so we need to update all references (catalog IDs) to that
//    LSP. This routine takes the Winsock catalog after a dependent LSP has
//    been resinstalled (pProvider and iProviderCount) and matches the newly
//    installed providers back to the LSP map we started with. This is so we
//    can get the older catalog ID along with the new catalog ID of each LSP
//    entry.
//
void
MapNewEntriesToOld(
    LSP_ENTRY         *pEntry, 
    WSAPROTOCOL_INFOW *pProvider, 
    int                iProviderCount
    )
{
    int     i, j;

    for(i=0; i < pEntry->Count ;i++)
    {
        for(j=0; j < iProviderCount ;j++)
        {
            if ( IsEqualProtocolEntries( &pEntry->LayeredEntries[ i ], &pProvider[ j ] ) )
            {
                pEntry->LayeredEntries[ i ].dwProviderReserved = 
                        pProvider[ j ].dwCatalogEntryId;

                dbgprint( "Mapped old %d to new %d\n",
                        pEntry->LayeredEntries[ i ].dwCatalogEntryId,
                        pProvider[ j ].dwCatalogEntryId
                        );

                break;
            }
        }
    }
}

//
// Function: AddGuidToLspEntry
//
// Description:
//      This routine adds a GUID to the list of provider GUIDs for an LSP_ENTRY if
//      it is unique. If the GUID already exists in the list it is not added again.
//
int
AddGuidToLspEntry(
    LSP_ENTRY  *entry,
    GUID       *guid,
    int        *lpErrno
    )
{
    BOOL    bFound;
    int     rc,
            i;

    if ( 0 == entry->Count )
    {
        entry->LayeredGuids = (GUID *) LspAlloc(
                sizeof( GUID ),
                lpErrno 
                );
        if ( NULL == entry->LayeredGuids )
        {
            fprintf( stderr, "AddGuidToLspEntry: LspAlloc failed: %d\n", *lpErrno );
            goto cleanup;
        }

        memcpy( &entry->LayeredGuids[ 0 ], guid, sizeof( GUID ) );

        entry->LayeredGuidCount++;
    }
    else
    {
        // See if we've already seen this guid
        bFound = FALSE;
        for(i=0; i < entry->LayeredGuidCount ;i++)
        {
            rc = memcmp( &entry->LayeredGuids[ i ], guid, sizeof( GUID ) );
            if ( 0 == rc )
            {
                bFound = TRUE;
                break;
            }
        }
        if ( FALSE == bFound )
        {
            GUID    *tmpguid = NULL;

            // New GUID -- we need to add it to the array
            tmpguid = (GUID *) LspAlloc(
                    sizeof( GUID ) * ( entry->LayeredGuidCount + 1 ),
                    lpErrno
                    );
            if ( NULL == tmpguid )
            {
                fprintf( stderr, "AddGuidToLspEntry: LspAlloc failed: %d\n", *lpErrno );
                goto cleanup;
            }

            memcpy( tmpguid, entry->LayeredGuids, sizeof(GUID) * entry->LayeredGuidCount );

            memcpy( &tmpguid[ entry->LayeredGuidCount ], guid, sizeof( GUID ) );

            LspFree( entry->LayeredGuids );

            entry->LayeredGuids = tmpguid;
            entry->LayeredGuidCount++;
        }
    }

    return NO_ERROR;

cleanup:

    return SOCKET_ERROR;
}

//
// Function: UpdateProviderOrder
//
// Desciption:
//    This routine is called after removing and reinstalling an existing LSP (which
//    is done when removing an LSP that other LSPs are layered over and the 
//    WSCUpdateProvider function is not available). After the LSP is re-installed
//    the dwProviderReserved field of each of it's layered protocol entries contains
//    its new catalog ID. This routine walks the original array describing the order
//    of the Winsock catalog and updates the old ID with the new ID. This is done so
//    once all the dependent LSPs are removed and re-installed, the catalog can be
//    put back in its original order.
//
void
UpdateProviderOrder(
    LSP_ENTRY  *UpdatedEntry,
    DWORD      *OrderArray,
    int         ArrayCount
    )
{
    int     i, j;


    for(i=0; i < UpdatedEntry->Count ;i++)
    {
        for(j=0; j < ArrayCount ;j++)
        {
            // Replace an occurence of the old value with the new value
            if ( OrderArray[ j ] == UpdatedEntry->LayeredEntries[ i ].dwCatalogEntryId )
            {
                OrderArray[ j ] = UpdatedEntry->LayeredEntries[ i ].dwProviderReserved;
            }
        }
    }
}

//
// Function: MaxLayeredChainCount
//
// Description:
//    This routine examines each LSP_ENTRY in an array to find the longest layered
//    protocol chain count. This is typically used to find the maximum number of
//    WSAPROTOCOL_INFOW objects required when manipulating providers (so that only
//    one array has to be allocated which can handle all installed providers).
//
int
MaxLayeredChainCount(
    LSP_ENTRY  *pLspMap,
    int         LspCount
    )
{
    int MaxSize = 0,
        i;

    for(i=0; i < LspCount ;i++)
    {
        MaxSize = MAX( MaxSize, pLspMap[ i ].Count );
    }

    return MaxSize;
}
