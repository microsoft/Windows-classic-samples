//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (C) 2004  Microsoft Corporation.  All Rights Reserved.
//
// Module Name: lsputil.cpp
//
// Description:
//
//    This file contains miscellaneous utility functions used by the other major
//    LSP installer components (install, remove, LSP map).
//    See instlsp.cpp for more information on running this code.
//
#include "instlsp.h"

//
// Function: RemoveIdFromChain
//
// Description:
//    This function removes the given CatalogId from the protocol chain 
//    for pinfo.
//
BOOL 
RemoveIdFromChain(
    WSAPROTOCOL_INFOW *pInfo, 
    DWORD              dwCatalogId
    )
{
    int     i, 
            j;

    for(i=0; i < pInfo->ProtocolChain.ChainLen ;i++)
    {
        if ( pInfo->ProtocolChain.ChainEntries[ i ] == dwCatalogId )
        {
            for(j=i; j < pInfo->ProtocolChain.ChainLen-1 ; j++)
            {
                pInfo->ProtocolChain.ChainEntries[ j ] = 
                        pInfo->ProtocolChain.ChainEntries[ j+1 ];
            }
            pInfo->ProtocolChain.ChainLen--;
            return TRUE;
        }
    }
    return FALSE;
}

//
// Function: IsIdinChain
//
// Description:
//    This function determines whether the given catalog id is referenced
//    in the protocol chain of pinfo.
//
BOOL 
IsIdInChain(
    WSAPROTOCOL_INFOW *pInfo, 
    DWORD              dwId)
{
    int     i;

    for(i=0; i < pInfo->ProtocolChain.ChainLen ;i++)
    {
        if ( pInfo->ProtocolChain.ChainEntries[ i ] == dwId )
            return TRUE;
    }
    return FALSE;
}

//
// Function: GetProviderCount
//
// Description:
//    Returns the number of providers installed matching the given Provider Type.
//    The possible provider types are BASE_PROTOCOL or LAYERED_PROTOCOL. That is,
//    this function either returns the count of base providers or the number of
//    dummy LSP providers installed.
//
int
GetProviderCount(
    WSAPROTOCOL_INFOW *pProviders,
    int                iProviderCount,
    int                iProviderType
    )
{
    int Count, i;

    Count = 0;
    for(i=0; i < iProviderCount ;i++)
    {
        if ( ( LAYERED_CHAIN == iProviderType ) && ( pProviders[ i ].ProtocolChain.ChainLen > 1 ) )
            Count++;
        else if ( ( LAYERED_CHAIN != iProviderType) && ( pProviders[ i ].ProtocolChain.ChainLen == iProviderType ) )
            Count++;
    }
    return Count;
}

//
// Function: GetLayeredEntriesByGuid
//
// Description:
//    This routine is used by the uninstaller when WSCUpdateProvider is not 
//    available. If after an LSP is removed, there are other LSPs which depend
//    on that LSP, the uninstaller must remove and reinstall all LSPs and fix up
//    the catalog ID references within the protocol chain. This routine is used
//    when reinstalling LSP providers. Because the layered entries belonging to
//    an LSP may be installed either under individual GUIDs or whole
//    groups of providers may be installed under the same GUID. This routine
//    Takes an array of WSAPROTOCOL_INFOW entries belonging to one LSP along with
//    a GUID and copies all WSAPROTOCOL_INFOW structures having the same GUID
//    to an array. The count of how many structures were copied is returned.
//
int
GetLayeredEntriesByGuid(
    WSAPROTOCOL_INFOW *pMatchLayers,
    int               *iLayeredCount,
    WSAPROTOCOL_INFOW *pEntries, 
    int                iEntryCount,
    GUID              *MatchGuid
    )
{
    int                count, 
                       err = SOCKET_ERROR,
                       i;

    // First count how many entries belong to this GUID
    count = 0;
    for(i=0; i < iEntryCount ;i++)
    {
        if ( 0 == memcmp( MatchGuid, &pEntries[i].ProviderId, sizeof( GUID ) ) )
            count++;
    }

    // Make sure the array passed in is large enough to hold the results
    if ( count > *iLayeredCount )
    {
        *iLayeredCount = count;
        goto cleanup;
    }

    // Go back and copy the matching providers into our array
    count = 0;
    for(i=0; i < iEntryCount ;i++)
    {
        if ( 0 == memcmp( MatchGuid, &pEntries[ i ].ProviderId, sizeof( GUID ) ) )
        {
            memcpy( &pMatchLayers[ count++ ], &pEntries[ i ], sizeof( WSAPROTOCOL_INFOW ) );
        }
    }

    *iLayeredCount = count;

    err = NO_ERROR;

cleanup:

    return err;
}

//
// Function: IsEqualProtocolEntries
//
// Description:
//    This routine compares two WSAPROTOCOL_INFOW structures to determine 
//    whether they are equal. This is done when a provider is uninstalled
//    and then reinstalled. After reinstallation we need to match the old
//    provider to the new (after reenumerating the catalog) so we can find
//    its new catalog ID. The fields used for determining a match are all
//    fields except the catalog ID (dwCatalogEntryId) and the protocol
//    string (szProtocol).
//
BOOL
IsEqualProtocolEntries(
    WSAPROTOCOL_INFOW *pInfo1,
    WSAPROTOCOL_INFOW *pInfo2
    )
{
    if ( (memcmp(&pInfo1->ProviderId, &pInfo2->ProviderId, sizeof(GUID)) == 0) &&
         (pInfo1->dwServiceFlags1 == pInfo2->dwServiceFlags1) &&
         (pInfo1->dwServiceFlags2 == pInfo2->dwServiceFlags2) &&
         (pInfo1->dwServiceFlags3 == pInfo2->dwServiceFlags3) &&
         (pInfo1->dwServiceFlags4 == pInfo2->dwServiceFlags4) &&
         (pInfo1->ProtocolChain.ChainLen == pInfo2->ProtocolChain.ChainLen) &&
         (pInfo1->iVersion == pInfo2->iVersion) &&
         (pInfo1->iAddressFamily == pInfo2->iAddressFamily) &&
         (pInfo1->iMaxSockAddr == pInfo2->iMaxSockAddr) &&
         (pInfo1->iMinSockAddr == pInfo2->iMinSockAddr) &&
         (pInfo1->iSocketType == pInfo2->iSocketType) &&
         (pInfo1->iProtocol == pInfo2->iProtocol) &&
         (pInfo1->iProtocolMaxOffset == pInfo2->iProtocolMaxOffset) &&
         (pInfo1->iNetworkByteOrder == pInfo2->iNetworkByteOrder) &&
         (pInfo1->iSecurityScheme == pInfo2->iSecurityScheme) &&
         (pInfo1->dwMessageSize == pInfo2->dwMessageSize)
       )
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

//
// Function: RetrieveLspGuid
//
// Description:
//      This routine loads the given DLL and retrieves its GetLspGuid function
//      which returns the GUID under which the LSP's hidden dummy entry is to be
//      installed under. By exporting this function the uninstaller is decoupled
//      from a specific LSP.
//
int
RetrieveLspGuid(
    __in_z char    *LspPath,
    GUID    *Guid
    )
{
    HMODULE         hMod = NULL;
    LPFN_GETLSPGUID fnGetLspGuid = NULL;
    int             retval = SOCKET_ERROR;

    // Load teh library
    hMod = LoadLibraryA( LspPath );
    if ( NULL == hMod )
    {
        fprintf( stderr, "RetrieveLspGuid: LoadLibraryA failed: %d\n", GetLastError() );
        goto cleanup;
    }

    // Get a pointer to the LSPs GetLspGuid function
    fnGetLspGuid = (LPFN_GETLSPGUID) GetProcAddress( hMod, "GetLspGuid" );
    if ( NULL == fnGetLspGuid )
    {
        fprintf( stderr, "RetrieveLspGuid: GetProcAddress failed: %d\n", GetLastError() );
        goto cleanup;
    }

    // Retrieve the LSPs GUID
    fnGetLspGuid( Guid );

    retval = NO_ERROR;

cleanup:

    if ( NULL != hMod )
        FreeLibrary( hMod );

    return retval;
}


#pragma warning(push)
#pragma warning(disable: 4127)

//
// Function: IsNonIfsProvider
//
// Description:
//      This routine searches the catalog for a given provider (based on catalog ID)
//      and returns whether or not it is an IFS provider. This routine is used when
//      installing an IFS provider since all IFS providers have to appear in the 
//      protocol chain beneath any non-IFS providers.
//
BOOL
IsNonIfsProvider(
    WSAPROTOCOL_INFOW  *pProvider,
    int                 iProviderCount,
    DWORD               dwProviderId
    )
{
    int     i;

    for(i=0; i < iProviderCount ;i++)
    {
        if ( pProvider[ i ].dwCatalogEntryId == dwProviderId )
        {
            return !( pProvider[ i ].dwServiceFlags1 & XP1_IFS_HANDLES );
        }
    }
    
    return FALSE;
}

#pragma warning(pop)

//
// Function: LoadUpdateProviderFunction
//
// Description:
//      This routine loads the WSCUpdateProvider function from ws2_32.dll.
//      This function is used when removing LSP entries and other LSPs are layered
//      on top of the one to be removed. If this API is present (which it is on
//      Windows XP and later), then the logic is much simpler. This routien will
//      load both the 32-bit and 64-bit versions of the function.
//
HMODULE
LoadUpdateProviderFunction()
{
    HMODULE hModule = NULL;
    HRESULT hr;
    char    WinsockLibraryPath[ MAX_PATH+1 ],
            szExpandPath[ MAX_PATH+1 ];

    //
    // See if we're on a platform that supports WSCUpdateProvider. If so then
    // uninstalling an LSP is easy; otherwise, it is very painful if you're
    // removing an LSP that has other LSPs on top if it.
    //
    if ( GetSystemDirectoryA( WinsockLibraryPath, MAX_PATH+1 ) == 0 )
    {
        hr = StringCchCopyA( szExpandPath, MAX_PATH+1, "%SYSTEMROOT%\\system32" );
        if ( FAILED( hr ) )
        {
            fprintf( stderr, "LoadUpdateProviderFunctions: StringCchCopyA failed: 0x%x\n", hr );
            goto cleanup;
        }

        if ( ExpandEnvironmentStringsA( WinsockLibraryPath, szExpandPath, MAX_PATH+1 ) == 0 )
        {
            fprintf(stderr, "LoadUpdateProviderFunctions: Unable to expand environment string: %d\n", 
                    GetLastError()
                   );
            goto cleanup;
        }
    }

    hr = StringCchCatA( WinsockLibraryPath, MAX_PATH+1, WINSOCK_DLL );
    if ( FAILED( hr ) )
    {
        fprintf( stderr, "LoadUpdateProviderFunctions: StringCchCatA failed: 0x%x\n", hr );
        goto cleanup;
    }

    hModule = LoadLibraryA( WinsockLibraryPath );
    if (hModule == NULL)
    {
        fprintf(stderr, "LoadUpdateProviderFunctions: Unable to load %s: %d\n", 
                WinsockLibraryPath, GetLastError()
                );
        goto cleanup;
    }
#ifdef _WIN64
    fnWscUpdateProvider   = (LPWSCUPDATEPROVIDER)GetProcAddress(hModule, "WSCUpdateProvider");

    fnWscUpdateProvider32 = (LPWSCUPDATEPROVIDER)GetProcAddress(hModule, "WSCUpdateProvider32");
#else
    fnWscUpdateProvider   = (LPWSCUPDATEPROVIDER)GetProcAddress(hModule, "WSCUpdateProvider");
#endif

    return hModule;

cleanup:

    if ( NULL != hModule )
    {
        FreeLibrary( hModule );
        hModule = NULL;
    }

    return NULL;
}

//
// Function: CountOrphanedChainEntries
//
// Description:
//      This routne counts how many orphaned layered protocol entries exist. An 
//      orphaned protocol entry is a protocol entry whose chain length greater
//      than one and whose dummy hidden entry (i.e. index 0 of its protocol chain
//      array) is missing. When building the LSP map, it normally finds entries
//      based upon the dummy hidden entries. This method is used to determine
//      if orphaned entries exist so they can be removed as well.
//
int
CountOrphanedChainEntries(
    WSAPROTOCOL_INFOW  *pCatalog,
    int                 iCatalogCount
    )
{
    int     orphanCount = 0,
            i, j;

    for(i=0; i < iCatalogCount ;i++)
    {
        if ( pCatalog[ i ].ProtocolChain.ChainLen > 1 )
        {
            for(j=0; j < iCatalogCount ;j++)
            {
                if ( i == j )
                    continue;
                if ( pCatalog[ j ].dwCatalogEntryId == pCatalog[ i ].ProtocolChain.ChainEntries[ 0 ] )
                {
                    break;
                }
            }
            if ( j >= iCatalogCount )
                orphanCount++;
        }
    }

    return orphanCount;
}

//
// Function: FindProviderById
//
// Description:
//    This routine searches the Winsock catalog for the provider entry which matches
//    the supplied catalog ID.
//
WSAPROTOCOL_INFOW *
FindProviderById(
    DWORD               CatalogId,
    WSAPROTOCOL_INFOW  *Catalog,
    int                 CatalogCount
    )
{
    int     i;

    for(i=0; i < CatalogCount ;i++)
    {
        if ( Catalog[ i ].dwCatalogEntryId == CatalogId )
            return &Catalog[ i ];
    }
    return NULL;
}


//
// Function: FindProviderByGuid
//
// Description:
//    This routine searches the Winsock catalog for the entry which matches the
//    supplied GUID.
//
WSAPROTOCOL_INFOW *
FindProviderByGuid(
    GUID               *Guid,
    WSAPROTOCOL_INFOW  *Catalog,
    int                 CatalogCount
    )
{
    int     i;

    for(i=0; i < CatalogCount ;i++)
    {
        if ( 0 == memcmp( &Catalog[ i ].ProviderId, Guid, sizeof( GUID ) ) )
        {
            return &Catalog[ i ];
        }
    }

    return NULL;
}

//
// Function: GetCatalogIdForProviderGuid
//
// Description:
//    This routine finds the WInsock catalog entry for the GUID and returns the
//    catalog ID for that entry.
//
DWORD
GetCatalogIdForProviderGuid(
    GUID               *Guid,
    WSAPROTOCOL_INFOW  *Catalog,
    int                 CatalogCount
    )
{
    WSAPROTOCOL_INFOW *match = NULL;

    match = FindProviderByGuid( Guid, Catalog, CatalogCount );
    if ( NULL != match )
    {
        return match->dwCatalogEntryId;
    }

    return 0;
}

#pragma warning(push)
#pragma warning(disable: 4127 )

//
// Function: FindDummyIdFromProtocolChain
//
// Description:
//    This routine searches the catalog for the dummy LSP entry associated
//    with the given catalog ID. If the catalog ID pass is actually the 
//    dummy entry, that provider is found and returned. If the ID is a
//    protocol chain, then the first entry in that provider's protocol chain
//    will reference the dummy ID of the LSP so that entry is then found
//    and returned.
//
DWORD
FindDummyIdFromProtocolChainId(
    DWORD               CatalogId,
    WSAPROTOCOL_INFOW  *Catalog,
    int                 CatalogCount
    )
{
    int     i;

    for(i=0; i < CatalogCount ;i++)
    {
        if ( CatalogId == Catalog[ i ].dwCatalogEntryId )
        {
            if ( Catalog[ i ].ProtocolChain.ChainLen == LAYERED_PROTOCOL )
                return Catalog[ i ].dwCatalogEntryId;
            else
                return Catalog[ i ].ProtocolChain.ChainEntries[ 0 ];
        }
    }

    ASSERT( 0 );

    return 0;
}

#pragma warning(pop)

//
// Function: InsertIdIntoProtocolChain
//
// Description:
//    This routine inserts the given ID into an existing catalog entry at
//    the given index location within the chain array.
//
void
InsertIdIntoProtocolChain(
    WSAPROTOCOL_INFOW  *Entry,
    int                 Index,
    DWORD               InsertId
    )
{
    int     i;

    for(i=Entry->ProtocolChain.ChainLen; i > Index ;i--)
    {
        Entry->ProtocolChain.ChainEntries[ i ] = Entry->ProtocolChain.ChainEntries[ i - 1 ];
    }

    Entry->ProtocolChain.ChainEntries[ Index ] = InsertId;
    Entry->ProtocolChain.ChainLen++;
}

//
// Function: BuildSubsetLspChain
//
// Description:
//    This routine takes a portion of an existing protocol chain (from the specified
//    index to the end) and makes it the new protocol chain while inserting the 
//    specified DummyId in index 0. For example, 
//
//       Array Indices:    0      1      2      3
//                      | 1400 | 1301 | 1201 | 1001 |       Len = 4
//
//       If this routine is called with Index == 2 and DummyId == 1500 then
//       the protocol chain would be:
//
//       Array Indices:    0      1      2      3
//                      | 1500 | 1201 | 1001 |      |       Len = 3
//
void
BuildSubsetLspChain(
    WSAPROTOCOL_INFOW  *Entry,
    int                 Index,
    DWORD               DummyId
    )
{
    int     Idx, i;

    for(i=Index,Idx=1; i < Entry->ProtocolChain.ChainLen ;i++,Idx++)
    {
        Entry->ProtocolChain.ChainEntries[ Idx ] = Entry->ProtocolChain.ChainEntries[ i ];
    }

    Entry->ProtocolChain.ChainEntries[ 0 ] = DummyId;
    Entry->ProtocolChain.ChainLen = Entry->ProtocolChain.ChainLen - Index + 1;
}
