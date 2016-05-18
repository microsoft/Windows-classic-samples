//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (C) 2004  Microsoft Corporation.  All Rights Reserved.
//
// Module Name: instlsp.h
//
// Description:
//
//    This sample illustrates how to develop a layered service provider.
//    This LSP is simply a pass through sample which counts the bytes transfered
//    on each socket. 
//
//    This file contains function prototypes and structure definitions used in
//    the installation and removal of LSPs.
//    
//
#ifndef _INSTLSP_H_
#define _INSTLSP_H_

#include <winsock2.h>
#include <mswsock.h>
#include <ws2spi.h>
#include <objbase.h>
#include <rpc.h>
#include <rpcdce.h>
#include <sporder.h>
#include <winnt.h>
#include <windows.h>
#include <strsafe.h>
#ifndef _PSDK_BLD
#include "lspcommon.h"
#else
#include "..\common\lspcommon.h"
#endif
#include <stdio.h>
#include <stdlib.h>

#define DEFAULT_LSP_NAME        "Test LSP"

#define LAYERED_CHAIN           ( BASE_PROTOCOL + 1 )

#define UPDATE_LSP_ENTRY        0

#define MAX(a,b)                ( (a) > (b) ? (a) : (b) )
#define MIN(a,b)                ( (a) < (b) ? (a) : (b) )

typedef int (WSAAPI * LPWSCINSTALLPROVIDERANDCHAINS)(
        LPGUID lpProviderId,
        LPWSTR lpszProviderDllPath,
#ifdef _WIN64
        LPWSTR lpszProviderDllPath32,
#endif
        LPWSTR lpszLspName,
        DWORD dwServiceFlags,
        LPWSAPROTOCOL_INFOW lpProtocolInfoList,
        DWORD dwNumberOfEntries,
        LPDWORD lpdwCatalogEntryId,
        LPINT lpErrno
        );

// This structure is used to create the logical ordered LSP mappings
typedef struct _LSP_ENTRY
{
    WCHAR               wszLspDll[ MAX_PATH ];  // LSPs DLL name (and possible path)
    WSAPROTOCOL_INFOW   DummyEntry;             // Provider entry for dummy LSP entry

    BOOL                OrphanedEntries;        // Indicates this LSP entry contains
                                                // only orphaned protocol chains

    WSAPROTOCOL_INFOW  *LayeredEntries;         // All layered providers beloging to LSP
    int                 Count;                  // Number of layered providers

    int                *LayerChanged;           // Indicates if the entry was changed --
                                                //  Used when removing providers

    GUID               *LayeredGuids;           // List of GUIDs the LAYERED providers 
                                                //  are installed under (doesn't include
                                                //  the GUID the dummy entry is installed 
                                                //  under)
    int                 LayeredGuidCount;       // Number of GUIDs in the array

    int                 MaxChainLength;         // Used for sorting: the longest protocol
                                                //  chain of all the layered providers
    int                 LspOrder;               // Used for sorting: the lowest position
                                                //  within a layered entries protocol
                                                //  chain that a base provider sits
    
    int                 DependentCount;         // Number of LSPs layered over this one
    int                *DependentLspIndexArray; // Indices into LSP map of dependent LSPs

} LSP_ENTRY;

////////////////////////////////////////////////////////////////////////////////
//
// External Global Variable Definitions
//
////////////////////////////////////////////////////////////////////////////////
extern LPWSCUPDATEPROVIDER  fnWscUpdateProvider,
                            fnWscUpdateProvider32;
extern HMODULE              gModule;
extern GUID                 gProviderGuid;

////////////////////////////////////////////////////////////////////////////////
//
// LspAdd.cpp Prototypes
//
////////////////////////////////////////////////////////////////////////////////

// Install an LSP into the given Winsock catalog
int
InstallLsp(
        WINSOCK_CATALOG eCatalog,
        __in_z char    *lpszLspName,
        __in_z char    *lpszLspPathAndFile,
        __in_z char    *lpszLspPathAndFile32,
        DWORD           dwCatalogIdArrayCount,
        DWORD          *pdwCatalogIdArray,
        BOOL            IfsProvider,
        BOOL            InstallOverAll
        );

// Installs one or more protocol entries into the given Winsock catalog under a GUID
int 
InstallProvider(
        WINSOCK_CATALOG     Catalog,
        GUID               *Guid,
        __in_z WCHAR       *lpwszLspPath,
        WSAPROTOCOL_INFOW  *pProvider,
        INT                 iProviderCount
        );

int
InstallProviderVista(
        WINSOCK_CATALOG eCatalog,               // Which catalog to install LSP into
        __in_z WCHAR   *lpszLspName,            // String name of LSP
        __in_z WCHAR   *lpszLspPathAndFile,     // Location of 64-bit LSP dll and dll name
        __in_z WCHAR   *lpszLspPathAndFile32,   // Location of 32-bit LSP dll and dll name
        GUID           *providerGuid,
        DWORD           dwCatalogIdArrayCount,  // Number of entries in pdwCatalogIdArray
        DWORD          *pdwCatalogIdArray,      // Array of IDs to install over
        BOOL            IfsProvider,
        BOOL            InstallOverAll
        );

// Creates the protocol entry for the hidden dummy entry which must be installed first
WSAPROTOCOL_INFOW *
CreateDummyEntry(
        WINSOCK_CATALOG Catalog, 
        INT             CatalogId, 
        __in_z WCHAR   *lpwszLspName,
        BOOL            IfsProvider
        );

int
InstallIfsLspProtocolChains(
        WINSOCK_CATALOG eCatalog,
        GUID           *Guid,
        __in_z WCHAR   *lpszLspName,
        __in_z WCHAR   *lpszLspFullPathAndFile,
        DWORD          *pdwCatalogIdArray,
        DWORD           dwCatalogIdArrayCount
        );

int
InstallNonIfsLspProtocolChains(
        WINSOCK_CATALOG eCatalog,
        GUID           *Guid,
        __in_z WCHAR   *lpszLspName,
        __in_z WCHAR   *lpszLspFullPathAndFile,
        DWORD          *pdwCatalogIdArray,
        DWORD           dwCatalogIdArrayCount
        );

int
InsertIfsLspIntoAllChains( 
        WSAPROTOCOL_INFOW  *OriginalEntry,    // Original (unmodified) entry to follow chains
        WSAPROTOCOL_INFOW  *Catalog,          // Array of catalog entries
        int                 CatalogCount,     // Number of entries in Catalog array
        int                 IfsEntryIdx,      // Index into IFS standalone entry array
        int                 ChainIdx          // Chain index in OriginalEntry to start at
        );

// Reorder the given Winsock catalog such that the providers beloging to the given
//   dummy hidden provider are at the head of the catalog
int 
ReorderCatalog(
        WINSOCK_CATALOG Catalog, 
        DWORD           dwLayeredId
        );

// Write the Winsock catalog order according to the given list of catalog IDs
DWORD *
ReorderACatalog(
        WINSOCK_CATALOG Catalog,
        DWORD           dwLayerId,
        INT            *dwEntryCount
        );

// Rearrange the given Winsock catalog in the order specified as an array of catalog IDs
int
WriteProviderOrder(
        WINSOCK_CATALOG Catalog,
        DWORD          *pdwCatalogOrder,
        DWORD           dwNumberOfEntries,
        INT            *lpErrno
        );

////////////////////////////////////////////////////////////////////////////////
//
// LspDel.cpp Prototypes
//
////////////////////////////////////////////////////////////////////////////////

// Remove all layered service providers installed in the given catalog
int 
RemoveAllLayeredEntries(
        WINSOCK_CATALOG Catalog
        );

// Remove all provider entries associated with the given GUID from the given catalog
int 
DeinstallProvider(
        WINSOCK_CATALOG Catalog,
        GUID *Guid
        );

// Replaces/updates the protocol entries associated with the given GUID with the supplied
//   provider structures
int
UpdateProvider(
        WINSOCK_CATALOG     Catalog,
        LPGUID              ProviderId,
        WCHAR              *DllPath,
        WSAPROTOCOL_INFOW  *ProtocolInfoList,
        DWORD               NumberOfEntries,
        LPINT               lpErrno
        );

// Removes a single provider from the catalog that matches the given catalog ID
int 
RemoveProvider(
        WINSOCK_CATALOG Catalog, 
        DWORD           dwProviderId
        );

////////////////////////////////////////////////////////////////////////////////
//
// LspMap.cpp Prototypes
//
////////////////////////////////////////////////////////////////////////////////

// Prints all provider entries from the given catalog to the console
void 
PrintProviders(
        WINSOCK_CATALOG Catalog, 
        BOOL            bLayeredOnly, 
        BOOL            bVerbose
        );

// Build a map of what LSPs are installed on the system, including their order
LSP_ENTRY *
BuildLspMap(
        WSAPROTOCOL_INFOW *pProviders,
        int                iProviderCount,
        int               *pLspCount
        );

// Print the LSP map to the console
void
PrintLspMap(
        LSP_ENTRY *pLspMap,
        int        iLspCount
        );

// Free all resources associated with an already created LSP map
void
FreeLspMap(
        LSP_ENTRY *pLspMap,
        int        iLspCount
        );

// Looks for dependencies between LSPs
int
LspDependencyCheck(
        LSP_ENTRY  *pLspMap,
        int         iLspCount
        );

// Updates the catalog ID for all providers in an LSP map
void
UpdateLspMap(
        LSP_ENTRY *pLspMap,
        DWORD      dwOldValue,
        DWORD      dwNewValue
        );

// After updating the catalog map the new entries over the old ones in the LSP map
void
MapNewEntriesToOld(
        LSP_ENTRY         *pEntry, 
        WSAPROTOCOL_INFOW *pProvider, 
        int                iProviderCount
        );

// Adds a GUID into the LSP_ENTRY array of unique guids
int
AddGuidToLspEntry(
        LSP_ENTRY  *entry,
        GUID       *guid,
        int        *lpErrno
        );

// Updates the catalog IDs in the an array after a catalog entry changes
void
UpdateProviderOrder(
        LSP_ENTRY  *UpdatedEntry,
        DWORD      *OrderArray,
        int         ArrayCount
        );

// Determines the "deepest" LSP installed in the catalog
int
MaxLayeredChainCount(
        LSP_ENTRY  *pLspMap,
        int         LspCount
        );


////////////////////////////////////////////////////////////////////////////////
//
// LspUtil.cpp Prototypes
//
////////////////////////////////////////////////////////////////////////////////

// Compresses an protocol chain by removing the given ID 
BOOL 
RemoveIdFromChain(
        WSAPROTOCOL_INFOW *pInfo, 
        DWORD              dwCatalogId
        );

// Looks for the given catalog ID in the protocol chain
BOOL 
IsIdInChain(
        WSAPROTOCOL_INFOW *pInfo, 
        DWORD              dwId
        );

// Returns the number of protocol entries of the given type (base or dummy entries)
int
GetProviderCount(
        WSAPROTOCOL_INFOW *pProviders,
        int                iProviderCount,
        int                iProviderType
        );

// Returns all the catalog entries belonging to the given GUID
int
GetLayeredEntriesByGuid(
        WSAPROTOCOL_INFOW *pMatchLayers,
        int               *iLayeredCount,
        WSAPROTOCOL_INFOW *pEntries, 
        int                iEntryCount,
        GUID              *MatchGuid
        );

// Determines if two entries are the same after reinstalling an LSP (since the IDs are different now)
BOOL
IsEqualProtocolEntries(
        WSAPROTOCOL_INFOW *pInfo1,
        WSAPROTOCOL_INFOW *pInfo2
        );

// Given the full path and name of LSP, load it and call the GetLspGuid export
int
RetrieveLspGuid(
        __in_z char    *LspPath,
        GUID    *Guid
        );

// Looks up whether the given provider is an IFS provider or not
BOOL
IsNonIfsProvider(
        WSAPROTOCOL_INFOW  *pProvider,
        int                 iProviderCount,
        DWORD               dwProviderId
        );

// Loads the WSCUpdateProvider function if available
HMODULE
LoadUpdateProviderFunction(
        );

// Counts how many orphaned layered chain entries exist
int
CountOrphanedChainEntries(
        WSAPROTOCOL_INFOW  *pCatalog,
        int                 iCatalogCount
        );

WSAPROTOCOL_INFOW *
FindProviderById(
        DWORD               CatalogId,
        WSAPROTOCOL_INFOW  *Catalog,
        int                 CatalogCount
        );

WSAPROTOCOL_INFOW *
FindProviderByGuid(
        GUID               *Guid,
        WSAPROTOCOL_INFOW  *Catalog,
        int                 CatalogCount
        );

DWORD
GetCatalogIdForProviderGuid(
        GUID               *Guid,
        WSAPROTOCOL_INFOW  *Catalog,
        int                 CatalogCount
        );

DWORD
FindDummyIdFromProtocolChainId(
        DWORD               CatalogId,
        WSAPROTOCOL_INFOW  *Catalog,
        int                 CatalogCount
        );

void
InsertIdIntoProtocolChain(
        WSAPROTOCOL_INFOW  *Entry,
        int                 Index,
        DWORD               InsertId
        );

void
BuildSubsetLspChain(
        WSAPROTOCOL_INFOW  *Entry,
        int                 Index,
        DWORD               DummyId
        );
#endif
