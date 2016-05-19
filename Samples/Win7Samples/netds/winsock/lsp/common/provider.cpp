// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (C) 2004  Microsoft Corporation.  All Rights Reserved.
//
// Module Name: provider.cpp
//
// Description:
//
//    This sample illustrates how to develop a layered service provider.
//    This sample is a pass through sample which only counts the bytes
//    transfered on the socket.
//
//    This file contains support functions that are common to the lsp and
//    the instlsp sample for enumerating the Winsock catalog of service
//    providers.
//
//    Note all allocations made in this routine use the process heap and
//    not the LSP heap since these routines are also used by the LSP
//    installer (which doesn't create its own heap).
//    

#ifndef _PSDK_BLD
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#endif

#include <ws2spi.h>
#include <mswsock.h>
#include <sporder.h>
#include "lspcommon.h"

#include <stdio.h>
#include <stdlib.h>

CRITICAL_SECTION    gDebugCritSec;

// Handle to a private heap used by the LSP and the installer
HANDLE gLspHeap = NULL;

//
// Function: dbgprint
//
// Description:
//    Format and print a message to the debugger.
//

#ifdef DBG
void 
dbgprint(
    char *format,
    ...
    )
{
    static  DWORD pid=0;
    va_list vl;
    char    dbgbuf1[2048],
            dbgbuf2[2048];

    // Prepend the process ID to the message
    if ( 0 == pid )
    {
        pid = GetCurrentProcessId();
    }

    EnterCriticalSection(&gDebugCritSec);
    va_start(vl, format);
    StringCbVPrintf(dbgbuf1, sizeof(dbgbuf1),format, vl);
    StringCbPrintf(dbgbuf2, sizeof(dbgbuf2),"%lu: %s\r\n", pid, dbgbuf1);
    va_end(vl);

    OutputDebugString(dbgbuf2);
    LeaveCriticalSection(&gDebugCritSec);
}
#endif

//
// Function: GetProviders
//
// Description:
//    This enumerates the Winsock catalog via the global variable ProtocolInfo.
//
LPWSAPROTOCOL_INFOW 
EnumerateProviders(
    WINSOCK_CATALOG Catalog, 
    LPINT           TotalProtocols
    )
{
	LPWSAPROTOCOL_INFOW ProtocolInfo = NULL;
	DWORD               ProtocolInfoSize = 0;
	INT                 ErrorCode = NO_ERROR,
                        rc;
    
    if ( NULL == TotalProtocols )
        goto cleanup;

	*TotalProtocols = 0;

#ifdef _WIN64
	// Find out how many entries we need to enumerate
    if ( LspCatalog64Only == Catalog )
    {
        // Find the size of the buffer
        rc = WSCEnumProtocols( NULL, ProtocolInfo, &ProtocolInfoSize, &ErrorCode );
        if ( SOCKET_ERROR == rc )
        {
            if ( WSAENOBUFS != ErrorCode )
                goto cleanup;
            ErrorCode = NO_ERROR;
        }

        // Allocate the buffer
        ProtocolInfo = (LPWSAPROTOCOL_INFOW) LspAlloc(
                ProtocolInfoSize,
               &ErrorCode
                );
        if (ProtocolInfo == NULL)
            goto cleanup;

        // Enumerate the catalog for real
        rc = WSCEnumProtocols( NULL, ProtocolInfo, &ProtocolInfoSize, &ErrorCode );
        if ( SOCKET_ERROR == rc )
            goto cleanup;

        // Update the count
        *TotalProtocols = rc;
    }
    else if ( LspCatalog32Only == Catalog )
    {
        HMODULE            hModule;
        LPWSCENUMPROTOCOLS fnWscEnumProtocols32 = NULL;

        // Load ws2_32.dll
        hModule = LoadLibrary( TEXT( "ws2_32.dll" ) );
        if ( NULL == hModule )
            goto cleanup;

        // Find the 32-bit catalog enumerator
        fnWscEnumProtocols32 = (LPWSCENUMPROTOCOLS) GetProcAddress(
                hModule, 
                TEXT( "WSCEnumProtocols32" )
                );
        if ( NULL == fnWscEnumProtocols32 )
            goto cleanup;
        
        // Find the required buffer size
        rc = fnWscEnumProtocols32(NULL, ProtocolInfo, &ProtocolInfoSize, &ErrorCode);
        if ( SOCKET_ERROR == rc )
        {
            if ( WSAENOBUFS != ErrorCode )
                goto cleanup;
            ErrorCode = NO_ERROR;
        }

        // Allocate the buffer
        ProtocolInfo = (LPWSAPROTOCOL_INFOW) LspAlloc(
                ProtocolInfoSize,
               &ErrorCode
                );
        if ( NULL == ProtocolInfo )
            goto cleanup;

        // Enumrate the catalog for real this time
        rc = fnWscEnumProtocols32( NULL, ProtocolInfo, &ProtocolInfoSize, &ErrorCode );
        if ( SOCKET_ERROR == rc )
            goto cleanup;

        // Update the count 
        *TotalProtocols = rc;

        FreeLibrary( hModule );
    }
#else
    if ( LspCatalog32Only == Catalog )
    {
        // Find the size of the buffer
        rc = WSCEnumProtocols( NULL, ProtocolInfo, &ProtocolInfoSize, &ErrorCode );
        if ( SOCKET_ERROR == rc )
        {
            if ( WSAENOBUFS != ErrorCode )
                goto cleanup;
            ErrorCode = NO_ERROR;
        }

        // Allocate the buffer
        ProtocolInfo = (LPWSAPROTOCOL_INFOW) LspAlloc(
                ProtocolInfoSize,
               &ErrorCode
                );
        if ( NULL == ProtocolInfo )
            goto cleanup;

        // Enumerate the catalog for real
        rc = WSCEnumProtocols( NULL, ProtocolInfo, &ProtocolInfoSize, &ErrorCode );
        if ( SOCKET_ERROR == rc )
            goto cleanup;

        // Update the count
        *TotalProtocols = rc;
    }
    else if ( LspCatalog64Only == Catalog )
    {
        dbgprint( "Unable to enumerate 64-bit Winsock catalog from 32-bit process!");
    }
#endif
    else
    {
        // Find the size of the buffer
        rc = WSCEnumProtocols( NULL, ProtocolInfo, &ProtocolInfoSize, &ErrorCode );
        if ( SOCKET_ERROR == rc )
        {
            if ( WSAENOBUFS != ErrorCode )
                goto cleanup;
            ErrorCode = NO_ERROR;
        }

        // Allocate the buffer
        ProtocolInfo = (LPWSAPROTOCOL_INFOW) LspAlloc(
                ProtocolInfoSize,
               &ErrorCode
                );
        if ( NULL == ProtocolInfo )
            goto cleanup;

        // Enumerate the catalog for real
        rc = WSCEnumProtocols( NULL, ProtocolInfo, &ProtocolInfoSize, &ErrorCode );
        if ( SOCKET_ERROR == rc )
            goto cleanup;

        // Update the count
        *TotalProtocols = rc;
    }

cleanup:

    if ( ( NO_ERROR != ErrorCode ) && ( NULL != ProtocolInfo ) )
    {
        LspFree( ProtocolInfo );
        ProtocolInfo = NULL;
    }

	return ProtocolInfo;
}

//
// Function: EnumerateProvidersExisting
//
// Description:
//      This function enumerates the catalog into a preallocated buffer.
//
int
EnumerateProvidersExisting(
    WINSOCK_CATALOG     Catalog, 
    WSAPROTOCOL_INFOW  *ProtocolInfo,
    LPDWORD             ProtocolInfoSize
    )
{
	INT                 ErrorCode = NO_ERROR,
                        rc = NO_ERROR;
    
#ifdef _WIN64
    if ( LspCatalog64Only == Catalog )
    {
        rc = WSCEnumProtocols( NULL, ProtocolInfo, ProtocolInfoSize, &ErrorCode );
    }
    else if ( LspCatalog32Only == Catalog )
    {
        rc = WSCEnumProtocols32( NULL, ProtocolInfo, ProtocolInfoSize, &ErrorCode );
    }
#else
    if ( LspCatalog32Only == Catalog )
    {
        rc = WSCEnumProtocols( NULL, ProtocolInfo, ProtocolInfoSize, &ErrorCode );
    }
    else if ( LspCatalog64Only == Catalog )
    {
        dbgprint( "Unable to enumerate 64-bit Winsock catalog from 32-bit process!" );
    }
#endif
    if ( SOCKET_ERROR == rc )
    {
        dbgprint( "EnumerateProvidersExisting: WSCEnumProviders failed: %d",
                GetLastError() );
    }

	return rc;
}

//
// Function: FreeProviders
//
// Description:
//    This function frees the global catalog.
//
void 
FreeProviders(
    LPWSAPROTOCOL_INFOW ProtocolInfo
    )
{
	LspFree( ProtocolInfo );
}

//
// Function: LspAlloc
//
// Description:
//    Allocates a buffer of the requested size from the LSP created heap. If the
//    allocation fails, set the error value to WSAENOBUFS.
//
void *
LspAlloc(
    SIZE_T  size,
    int    *lpErrno
    )
{
    void *mem = NULL;
    mem = HeapAlloc( 
            gLspHeap, 
            HEAP_ZERO_MEMORY, 
            size
            );
    if ( NULL == mem )
    {
        *lpErrno = WSAENOBUFS;
    }

    return mem;
}

//
// Function: LspFree
//
// Description:
//    Frees a buffer that was previously allocated by LspAlloc.
//
void
LspFree(
    LPVOID  buf
    )
{
    HeapFree( gLspHeap, 0, buf );
}

//
// Function: LspCreateHeap
//
// Description:
//    Creates the private heap used by all LSP routines.
//
int
LspCreateHeap(
    int *lpErrno
    )
{
    gLspHeap = HeapCreate( 0, 128000, 0 );
    if ( NULL == gLspHeap )
    {
        *lpErrno = WSAEPROVIDERFAILEDINIT;
        return SOCKET_ERROR;
    }
    return NO_ERROR;
}

//
// Function: LspDestroyHeap
//
// Description:
//    Frees the private heap used by all LSP routines.
//
void
LspDestroyHeap(
    )
{
    if ( NULL != gLspHeap )
    {
        HeapDestroy( gLspHeap );
        gLspHeap = NULL;
    }
}

//
// Function: FindLspEntries
//
// Description:
//      This function searches the Winsock catalog and builds an array of the
//      WSAPROTOCOL_INFOW structures which belong to the LSP. This includes
//      all layered protocol chains as well as the LSP dummy (hidden) entry.
//      The function first finds the dummy entry using 'gProviderGuid' which
//      is the global GUID of the dummy entry. From there, the catalog is
//      searched for all entries whose first entry in the protocol chain
//      matches the dummy entry's catalog ID.
//
BOOL
FindLspEntries(
        PROVIDER  **lspProviders,
        int        *lspProviderCount,
        int        *lpErrno
        )
{
    PROVIDER           *Providers = NULL;
    LPWSAPROTOCOL_INFOW ProtocolInfo = NULL;
    DWORD               DummyLspId = 0;
    int                 ProtocolCount = 0,
                        LayerCount = 0,
                        idx,
                        i, j;

    *lspProviderCount = 0;
    *lspProviders = NULL;

    // Enumerate the catalog
    ProtocolInfo = EnumerateProviders( LspCatalogBoth, &ProtocolCount );
    if ( NULL == ProtocolInfo )
    {
        dbgprint("FindLspEntries; EnumerateProviders failed!");
        goto cleanup;
    }

    // Find our dummy LSP entry ID
    DummyLspId = 0;
    for(i=0; i < ProtocolCount ;i++)
    {
        if ( 0 == memcmp( &ProtocolInfo[ i ].ProviderId, &gProviderGuid, sizeof( gProviderGuid ) ) )
        {
            DummyLspId = ProtocolInfo[ i ].dwCatalogEntryId;
            break;
        }
    }

    ASSERT( 0 != DummyLspId );

    // Count how many LSP layered entries are present
    LayerCount = 0;
    for(i=0; i < ProtocolCount ;i++)
    {
        if ( ( ProtocolInfo[ i ].ProtocolChain.ChainLen > 1 ) &&
             ( DummyLspId == ProtocolInfo[ i ].ProtocolChain.ChainEntries[ 0 ] )
           )
        {
            LayerCount++;
        }
    }

    ASSERT( 0 != LayerCount );

    // Allocate space for the layered providers
    Providers = (PROVIDER *) LspAlloc( sizeof(PROVIDER) * LayerCount, lpErrno );
    if ( NULL == Providers )
    {
        dbgprint("FindLspEntries: LspAlloc failed: %d", *lpErrno );
        goto cleanup;
    }

    idx = 0;

    // Save the LSP layered entries
    for(i=0; i < ProtocolCount ;i++)
    {
        // The layered protocol entries for this LSP will always reference the
        //    dummy entry ID as its first entry in the chain array. Also make
        //    sure to check only LSP entries (since a base provider's chain length
        //    is 1 but the chain array entries can be garbage)
        if ( ( ProtocolInfo[ i ].ProtocolChain.ChainLen > 1 ) &&
             ( DummyLspId == ProtocolInfo[ i ].ProtocolChain.ChainEntries[ 0 ] )
           )
        {
            // Copy the new entry to the head
            memcpy( &Providers[ idx ].LayerProvider, &ProtocolInfo[ i ], sizeof(WSAPROTOCOL_INFOW) );
            Providers[ idx ].LayerProvider.szProtocol[ WSAPROTOCOL_LEN ] = '\0';

            // Copy the provider underneath this entry
            for(j=0; j < ProtocolCount ;j++)
            {
                // The next provider can either be a base, a dummy, or another layered
                //    protocol chain. If a dummy or layer then both providers will have
                //    the same DLL to load.
                if ( ProtocolInfo[ i ].ProtocolChain.ChainEntries[ 1 ] ==
                     ProtocolInfo[ j ].dwCatalogEntryId )
                {
                    memcpy( &Providers[ idx ].NextProvider, &ProtocolInfo[ j ],
                            sizeof( WSAPROTOCOL_INFOW ) );
                    Providers[ idx ].NextProvider.szProtocol[ WSAPROTOCOL_LEN ] = '\0';
                    break;
                }
            }

            // Verify we copied the lower layer
            ASSERT( j < ProtocolCount );

            InitializeCriticalSection( &Providers[ idx ].ProviderCritSec );
            InitializeListHead( &Providers[ idx ].SocketList );

            Providers[ idx ].LspDummyId = DummyLspId;

            idx++;
        }
    }

    ASSERT( idx == LayerCount );

    if ( NULL != ProtocolInfo )
        FreeProviders( ProtocolInfo );

    *lspProviders = Providers;
    *lspProviderCount = LayerCount;

    return TRUE;

cleanup:

    if ( NULL != ProtocolInfo )
        FreeProviders( ProtocolInfo );

    if ( NULL != Providers )
        LspFree( Providers );

    return FALSE;
}

#pragma warning(push)
#pragma warning(disable: 4127)

//
// Function: FindMatchingLspEntryForProtocolInfo
//
// Description:
//      This function searches for the appropriate LSP protocol chain entry which 
//      would handle a given WSAPROTOCOL_INFOW.
//
PROVIDER *
FindMatchingLspEntryForProtocolInfo(
        WSAPROTOCOL_INFOW *inInfo,
        PROVIDER          *lspProviders,
        int                lspCount,
        BOOL               fromStartup
        )
{
    WSAPROTOCOL_INFOW  *ProtocolInfo = NULL;
    DWORD               hiddenEntryId;
    int                 ProtocolCount,
                        i, j, k;

    // Two possibilites - this inInfo belongs to our LSP or its a layer over our LSP

    // First see if the inInfo is one of the LSPs entry
    for(i=0; i < lspCount ;i++)
    {
        if ( inInfo->dwCatalogEntryId == lspProviders[ i ].LayerProvider.dwCatalogEntryId )
        {
            return &lspProviders[ i ];
        }
    }

    ASSERT( inInfo->ProtocolChain.ChainLen > 1 );

    // Next check the inInfo's protocol chains for a reference to our LSP
    for(i=0; i < lspCount ;i++)
    {
        for(j=1; j < inInfo->ProtocolChain.ChainLen ;j++)
        {
            if ( inInfo->ProtocolChain.ChainEntries[ j ] == lspProviders[ i ].LspDummyId )
            {
                // Bad news, the entry passed to us references our dummy entry so we
                //    need to do some heuristic work to find the "correct" LSP entry
                //    that corresponds to the input provider
                goto next_match;
            }
            else if ( inInfo->ProtocolChain.ChainEntries[ j ] == lspProviders[ i ].LayerProvider.dwCatalogEntryId )
            {
                return &lspProviders[ i ];
            }
        }
    }

next_match:

    // If we didn't find an explicit match we'll have to guess - first try to
    //    match address family, socket type, protocol and provider flags
    for(i=0; i < lspCount ;i++)
    {
        if ( ( inInfo->iAddressFamily == lspProviders[ i ].LayerProvider.iAddressFamily ) &&
             ( inInfo->iSocketType == lspProviders[ i ].LayerProvider.iSocketType ) &&
             ( inInfo->iProtocol == lspProviders[ i ].LayerProvider.iProtocol ) &&
             ( ( ( inInfo->dwServiceFlags1 & ~XP1_IFS_HANDLES ) ==
                 ( lspProviders[ i ].LayerProvider.dwServiceFlags1 & ~XP1_IFS_HANDLES ) 
               )
             )
           )
        {
            return &lspProviders[ i ];
        }
    }

    // If this routine was called from WSPSocket and we can't find a match yet, we're
    //    in bad shape since the protocol info passed in matches no entries of this 
    //    LSPs ...

    ASSERT( fromStartup );

#ifndef DBG
    UNREFERENCED_PARAMETER( fromStartup );
#endif

    //
    // Still no match. See if the protocol info passed in belongs to another LSP.
    // Enumerate all its protocol entries and see if we reside in any of those
    // chains. This typically will happen when an LSP is layered over us and it
    // bulk loads all of the providers beneath its entries upon its first WSPStartup
    // call.
    //
    // For example consider the following catalog (where this LSP is LSP1):
    //
    //  _____________ _____________
    //  | LSP 2 TCP | | LSP 2 UDP |
    //  _____________              
    //  | LSP 1 TCP |
    //  _____________ _____________
    //  | BASE TCP  | | BASE UDP  |
    //
    // When LSP 2 is started, its WSPStartup is invoked with its UDP entry (since
    // the application created a UDP socket). Because LSP2 initializes all its layers
    // upon first WSPStartup it will load all the layers beneath it. So LSP2 will
    // load LSP1 and invoke its WSPStartup but will pass the UDP protocol info which
    // it was passed. At this point LSP1 won't know what to do since it was passed a
    // UDP entry and its not layered over any UDP entry. LSP1 cannot return any
    // entry if its an IFS LSP which implements only a subset of the Winsock
    // functions (i.e. LSP1 will return a mix of function pointers from BASE TCP
    // and its own LSP DLL). Because of this, LSP1 will try to "match" which
    // chain from LSP2 it actually resides in such that when LSP2 creates a TCP
    // socket later, it will have the correct function pointers.
    //
    // The heuristic is:
    // 1. Find all layered protocol entries belonging to the WSAPROTOCOL_INFOW passed.
    //    In the above example it would find LSP2 TCP and LSP2 UDP.
    // 2. Iterate through each provider found and walk the protocol chain in each 
    //    provider, looking for a reference to LSP1's entry.
    // 3. If found check LSP1 entry to see if it has already been loaded. If not
    //    then this is the match, if so then LSP2 could be layered over another TCP
    //    entry which is also layered over LSP1 so keep looking.
    //

    // Should never receive a base or dummy entry
    ASSERT( inInfo->ProtocolChain.ChainLen > 1 );

    ProtocolInfo = EnumerateProviders( LspCatalogBoth, &ProtocolCount );
    if ( NULL == ProtocolInfo )
    {
        dbgprint("FindMatchingLspEntryForProtocolInfo: EnumerateProviders failed!\n");
        goto cleanup;
    }

    hiddenEntryId = 0;
    for(i=0; i < ProtocolCount ;i++)
    {
        if ( inInfo->ProtocolChain.ChainEntries[ 0 ] == ProtocolInfo[ i ].dwCatalogEntryId )
        {
            hiddenEntryId = ProtocolInfo[ i ].dwCatalogEntryId;
            break;
        }
    }

    ASSERT( hiddenEntryId );

    for(i=0; i < ProtocolCount ;i++)
    {
        if ( ProtocolInfo[ i ].ProtocolChain.ChainEntries[ 0 ] == hiddenEntryId )
        {
            // This entry belongs to the LSP layered over us - walk its chains to
            //    see if it references our LSP
            for(j=1; j < ProtocolInfo[ i ].ProtocolChain.ChainLen ;j++)
            {
                for(k=0; k < lspCount ;k++)
                {
                    if ( ProtocolInfo[ i ].ProtocolChain.ChainEntries[ j ] ==
                         lspProviders[ k ].LayerProvider.ProtocolChain.ChainEntries[ 0 ]
                       )
                    {
                        // Bad news again, the protocol chain of the LSP above us
                        //   references our dummy LSP entry so we'll have to try
                        //   to match according to the protocol triplet and provider
                        //   flags
                        if ( ( ProtocolInfo[ i ].iAddressFamily == 
                               lspProviders[ k ].LayerProvider.iAddressFamily ) &&
                             ( ProtocolInfo[ i ].iSocketType ==
                               lspProviders[ k ].LayerProvider.iSocketType ) &&
                             ( ProtocolInfo[ i ].iProtocol ==
                               lspProviders[ k ].LayerProvider.iProtocol ) &&
                             ( ( ProtocolInfo[ i ].dwServiceFlags1 & ~XP1_IFS_HANDLES ) ==
                               ( lspProviders[ k ].LayerProvider.dwServiceFlags1 & ~XP1_IFS_HANDLES ) )
                           )
                        {
                            return &lspProviders[ i ];
                        }
                    }

                    if ( ( ProtocolInfo[ i ].ProtocolChain.ChainEntries[ j ] ==
                            lspProviders[ k ].LayerProvider.dwCatalogEntryId ) &&
                         ( lspProviders[ k ].StartupCount == 0 ) 
                       )
                    {
                        return &lspProviders[ i ];
                    }
                }
            }
        }
    }
    
cleanup:

    ASSERT( FALSE ); 

    return NULL;
}

#pragma warning(pop)

//
// Function: LoadProviderPath
//
// Description:
//      This function retrieves the provider's DLL path, expands any environment
//      variables, loads the DLL, and retrieves it's WSPStartup function.
//
BOOL
LoadProviderPath(
    PROVIDER    *loadProvider,
    int         *lpErrno
    )
{
    int     rc;

    *lpErrno = 0;

    // Retrieve the provider path of the lower layer
    loadProvider->ProviderPathLen = MAX_PATH - 1;
    rc = WSCGetProviderPath(
           &loadProvider->NextProvider.ProviderId,
            loadProvider->ProviderPathW,
           &loadProvider->ProviderPathLen,
            lpErrno
            );
    if ( SOCKET_ERROR == rc )
    {
        dbgprint("LoadProviderPath: WSCGetProviderPath failed: %d", *lpErrno );
        goto cleanup;
    }

    rc = ExpandEnvironmentStringsW(
            loadProvider->ProviderPathW,
            loadProvider->LibraryPathW,
            MAX_PATH - 1
            );
    if ( ( 0 != rc ) && ( MAX_PATH-1 >= rc ) )
    {
        loadProvider->Module = LoadLibraryW( loadProvider->LibraryPathW );
        if ( NULL == loadProvider->Module )
        {
            dbgprint("LoadProviderPath: LoadLibraryW failed: %d", GetLastError() );
            goto cleanup;
        }
    }
    else if ( 0 == rc )
    {
        char    ProviderPathA[ MAX_PATH ],
                LibraryPathA[ MAX_PATH ];

        // No UNICODE so we must be on Win9x
        rc = WideCharToMultiByte( CP_ACP, 0,
                loadProvider->ProviderPathW,
                loadProvider->ProviderPathLen,
                ProviderPathA,
                MAX_PATH,
                NULL,
                NULL
                );
        if ( 0 == rc )
        {
            dbgprint("LoadProviderPath: WideCharToMultiByte failed: %d", GetLastError() );
            goto cleanup;
        }

        rc = ExpandEnvironmentStringsA(
                ProviderPathA,
                LibraryPathA,
                MAX_PATH - 1
                );
        if ( ( 0 == rc ) || ( MAX_PATH - 1 < rc ) )
        {
            dbgprint("LoadProviderPath: ExpandEnvironmentStringsA failed: %d", GetLastError() );
            goto cleanup;
        }

        loadProvider->Module = LoadLibraryA( LibraryPathA );
        if ( NULL == loadProvider->Module )
        {
            dbgprint("LoadProviderPath: LoadLibraryA failed: %d", GetLastError() );
            goto cleanup;
        }
    }

    // Retrieve the next provider's WSPSTartup function
    loadProvider->fnWSPStartup = (LPWSPSTARTUP) GetProcAddress(
            loadProvider->Module,
            "WSPStartup"
            );
    if ( NULL == loadProvider->fnWSPStartup )
    {
        dbgprint("LoadProviderPath: GetProcAddress failed: %d", GetLastError() );
        goto cleanup;
    }

    return TRUE;

cleanup:

    if ( *lpErrno == 0 )
        *lpErrno = GetLastError();
    return FALSE;
}

int
InitializeProvider(
        PROVIDER *provider,
        WORD wVersion,
        WSAPROTOCOL_INFOW *lpProtocolInfo,
        WSPUPCALLTABLE UpCallTable,
        int *Error
        )
{
    WSAPROTOCOL_INFOW  *ProviderInfo = NULL;
    int                 rc;

    rc = LoadProviderPath( provider, Error );
    if ( FALSE == rc )
    {
        dbgprint("WSPStartup: LoadProviderPath failed: %d", *Error );
        goto cleanup;
    }

    // Determine which protocol structure to pass to the lower layer -
    //    if the next layer is a base provider, pass the base provider's
    //    structure; otherwise, pass whatever was given to us
    if ( BASE_PROTOCOL == provider->NextProvider.ProtocolChain.ChainLen )
        ProviderInfo = &provider->NextProvider;
    else
        ProviderInfo = lpProtocolInfo;

    rc = provider->fnWSPStartup(
            wVersion,
            &provider->WinsockVersion,
            ProviderInfo,
            UpCallTable,
            &provider->NextProcTable
            );
    if ( 0 != rc )
    {
        dbgprint("%ws::WSPStartup failed: %d", provider->NextProvider.szProtocol,
                rc );
        *Error = rc;
        goto cleanup;
    }

    // Make sure the proc table returned is valid
    rc = VerifyProcTable( &provider->NextProcTable );
    if ( SOCKET_ERROR == rc )
    {
        *Error = WSAEINVALIDPROCTABLE;
        goto cleanup;
    }

    ASSERT( NO_ERROR == rc );

    // Increment a startup count per-provider as well as an overall counter
    //    e.g. The sum of all the individual provider's startup should equal
    //         the global counter, that is, until the process starts calling
    //         WSPCleanup
    provider->StartupCount++;

cleanup:

    return rc;
}

//
// Function: VerifyProcTable
//
// Description:
//    Checks to make sure all function pointers are non-NULL. Returns SOCKET_ERROR
//    if successful, NO_ERROR otherwise.
//
int 
VerifyProcTable(
    LPWSPPROC_TABLE lpProcTable
    )
{
   if ( lpProcTable->lpWSPAccept &&
        lpProcTable->lpWSPAddressToString &&
        lpProcTable->lpWSPAsyncSelect &&
        lpProcTable->lpWSPBind &&
        lpProcTable->lpWSPCancelBlockingCall &&
        lpProcTable->lpWSPCleanup &&
        lpProcTable->lpWSPCloseSocket &&
        lpProcTable->lpWSPConnect &&
        lpProcTable->lpWSPDuplicateSocket &&
        lpProcTable->lpWSPEnumNetworkEvents &&
        lpProcTable->lpWSPEventSelect &&
        lpProcTable->lpWSPGetOverlappedResult &&
        lpProcTable->lpWSPGetPeerName &&
        lpProcTable->lpWSPGetSockOpt &&
        lpProcTable->lpWSPGetSockName &&
        lpProcTable->lpWSPGetQOSByName &&
        lpProcTable->lpWSPIoctl &&
        lpProcTable->lpWSPJoinLeaf &&
        lpProcTable->lpWSPListen &&
        lpProcTable->lpWSPRecv &&
        lpProcTable->lpWSPRecvDisconnect &&
        lpProcTable->lpWSPRecvFrom &&
        lpProcTable->lpWSPSelect &&
        lpProcTable->lpWSPSend &&
        lpProcTable->lpWSPSendDisconnect &&
        lpProcTable->lpWSPSendTo &&
        lpProcTable->lpWSPSetSockOpt &&
        lpProcTable->lpWSPShutdown &&
        lpProcTable->lpWSPSocket &&
        lpProcTable->lpWSPStringToAddress )
    {
        return NO_ERROR;
    }
    return SOCKET_ERROR;
}

