// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (C) 2004  Microsoft Corporation.  All Rights Reserved.
//
// Module Name: provider.h
//
// Description:
//
//    This sample illustrates how to develop a layered service provider that is
//    capable of counting all bytes transmitted through a TCP/IP socket.
//
//    This file contains all datatypes and function prototypes used
//    throughout this project.
//
#ifndef _INSTALL_H_
#define _INSTALL_H_ 

#include <winsock2.h>
#include <ws2spi.h>

// Name of the Winsock DDL which is needed by the installer to determine if
// WSCUpdateProvider is available.
#define WINSOCK_DLL     "\\ws2_32.dll"

////////////////////////////////////////////////////////////////////////////////
//
// External defines from lspguid.cpp and provider.cpp
//
////////////////////////////////////////////////////////////////////////////////

// Private heap used for all allocations in LSP as well as install time
extern HANDLE       gLspHeap;

// Global GUID under which the LSP dummy entry is installed under
extern GUID         gProviderGuid;

// Critical section for printing debug info (to prevent intermingling of messages)
extern CRITICAL_SECTION gDebugCritSec;

// Function definition for the GetLspGuid export which returns an LSPs dummy provider GUID
typedef
void (*LPFN_GETLSPGUID) (GUID *lpGuid);

// For 64-bit systems, we need to know which catalog to operate on
typedef enum
{
    LspCatalogBoth = 0,
    LspCatalog32Only,
    LspCatalog64Only
} WINSOCK_CATALOG;

//
// Extended proc table containing all the Microsoft specific Winsock functions
//
typedef struct _EXT_WSPPROC_TABLE
{
    LPFN_ACCEPTEX             lpfnAcceptEx;
    LPFN_TRANSMITFILE         lpfnTransmitFile;
    LPFN_GETACCEPTEXSOCKADDRS lpfnGetAcceptExSockaddrs;
    LPFN_TRANSMITPACKETS      lpfnTransmitPackets;
    LPFN_CONNECTEX            lpfnConnectEx;
    LPFN_DISCONNECTEX         lpfnDisconnectEx;
    LPFN_WSARECVMSG           lpfnWSARecvMsg;
} EXT_WSPPROC_TABLE;

//
// Describes a single catalog entry over which this LSP is layered on. It keeps track
// of the lower provider's dispatch table as well as a list of all the sockets
// created from our provider
//
typedef struct _PROVIDER
{
    WSAPROTOCOL_INFOW   NextProvider,           // Next provider in chain
                        LayerProvider;          // This layered provider
    WSPPROC_TABLE       NextProcTable;          // Proc table of next provider
    EXT_WSPPROC_TABLE   NextProcTableExt;       // Proc table of next provider's extension

    DWORD               LspDummyId;

    WCHAR               ProviderPathW[MAX_PATH],
                        LibraryPathW[MAX_PATH];
    INT                 ProviderPathLen;

    LPWSPSTARTUP        fnWSPStartup;
    WSPDATA             WinsockVersion;
    HMODULE             Module;

    INT                 StartupCount;

    LIST_ENTRY          SocketList;             // List of socket objects belonging to LSP

    CRITICAL_SECTION    ProviderCritSec;
} PROVIDER, * LPPROVIDER;


////////////////////////////////////////////////////////////////////////////////
//
// Provider.cpp prototypes
//
////////////////////////////////////////////////////////////////////////////////

BOOL
FindLspEntries(
        PROVIDER  **lspProviders,
        int        *lspProviderCount,
        int        *lpErrno
        );

PROVIDER *
FindMatchingLspEntryForProtocolInfo(
        WSAPROTOCOL_INFOW *inInfo,
        PROVIDER          *lspProviders,
        int                lspCount,
        BOOL               fromStartup = FALSE
        );

// Initialize the given provider by calling its WSPStartup
int
InitializeProvider(
        PROVIDER *provider,
        WORD wVersion,
        WSAPROTOCOL_INFOW *lpProtocolInfo,
        WSPUPCALLTABLE UpCallTable,
        int *Error
        );

BOOL
LoadProviderPath(
        PROVIDER    *loadProvider,
        int         *lpErrno
        );

// Verifies all the function pointers in the proc table are non-NULL
int 
VerifyProcTable(
        LPWSPPROC_TABLE lpProcTable
        );

// Returns an array of protocol entries from the given Winsock catalog
LPWSAPROTOCOL_INFOW 
EnumerateProviders(
        WINSOCK_CATALOG Catalog, 
        LPINT           TotalProtocols
        );

// Enumerates the given Winsock catalog into the already allocated buffer
int
EnumerateProvidersExisting(
        WINSOCK_CATALOG     Catalog, 
        WSAPROTOCOL_INFOW  *ProtocolInfo,
        LPDWORD             ProtocolInfoSize
        );

// Free the array of protocol entries returned from EnumerateProviders
void 
FreeProviders(
        LPWSAPROTOCOL_INFOW ProtocolInfo
        );

// Prints a protocol entry to the console in a readable, formatted form
void 
PrintProtocolInfo(
        WSAPROTOCOL_INFOW  *ProtocolInfo
        );

// Allocates a buffer from the LSP private heap
void *
LspAlloc(
        SIZE_T  size,
        int    *lpErrno
        );

// Frees a buffer previously allocated by LspAlloc
void
LspFree(
        LPVOID  buf
       );

// Creates the private heap used by the LSP and installer
int
LspCreateHeap(
        int    *lpErrno
        );

// Destroys the private heap
void
LspDestroyHeap(
        );


#ifdef _PSDK_BLD

/*++

LINK list:

    Definitions for a double link list.

--*/

//
// Calculate the address of the base of the structure given its type, and an
// address of a field within the structure.
//
#ifndef CONTAINING_RECORD
#define CONTAINING_RECORD(address, type, field) \
    ((type *)((PCHAR)(address) - (ULONG_PTR)(&((type *)0)->field)))
#endif


#ifndef InitializeListHead
//
//  VOID
//  InitializeListHead(
//      PLIST_ENTRY ListHead
//      );
//

#define InitializeListHead(ListHead) (\
    (ListHead)->Flink = (ListHead)->Blink = (ListHead))

//
//  BOOLEAN
//  IsListEmpty(
//      PLIST_ENTRY ListHead
//      );
//

#define IsListEmpty(ListHead) \
    ((ListHead)->Flink == (ListHead))

//
//  PLIST_ENTRY
//  RemoveHeadList(
//      PLIST_ENTRY ListHead
//      );
//

#define RemoveHeadList(ListHead) \
    (ListHead)->Flink;\
    {RemoveEntryList((ListHead)->Flink)}

//
//  PLIST_ENTRY
//  RemoveTailList(
//      PLIST_ENTRY ListHead
//      );
//

#define RemoveTailList(ListHead) \
    (ListHead)->Blink;\
    {RemoveEntryList((ListHead)->Blink)}

//
//  VOID
//  RemoveEntryList(
//      PLIST_ENTRY Entry
//      );
//

#define RemoveEntryList(Entry) {\
    PLIST_ENTRY _EX_Blink;\
    PLIST_ENTRY _EX_Flink;\
    _EX_Flink = (Entry)->Flink;\
    _EX_Blink = (Entry)->Blink;\
    _EX_Blink->Flink = _EX_Flink;\
    _EX_Flink->Blink = _EX_Blink;\
    }

//
//  VOID
//  InsertTailList(
//      PLIST_ENTRY ListHead,
//      PLIST_ENTRY Entry
//      );
//

#define InsertTailList(ListHead,Entry) {\
    PLIST_ENTRY _EX_Blink;\
    PLIST_ENTRY _EX_ListHead;\
    _EX_ListHead = (ListHead);\
    _EX_Blink = _EX_ListHead->Blink;\
    (Entry)->Flink = _EX_ListHead;\
    (Entry)->Blink = _EX_Blink;\
    _EX_Blink->Flink = (Entry);\
    _EX_ListHead->Blink = (Entry);\
    }

//
//  VOID
//  InsertHeadList(
//      PLIST_ENTRY ListHead,
//      PLIST_ENTRY Entry
//      );
//

#define InsertHeadList(ListHead,Entry) {\
    PLIST_ENTRY _EX_Flink;\
    PLIST_ENTRY _EX_ListHead;\
    _EX_ListHead = (ListHead);\
    _EX_Flink = _EX_ListHead->Flink;\
    (Entry)->Flink = _EX_Flink;\
    (Entry)->Blink = _EX_ListHead;\
    _EX_Flink->Blink = (Entry);\
    _EX_ListHead->Flink = (Entry);\
    }



BOOL IsNodeOnList(PLIST_ENTRY ListHead, PLIST_ENTRY Entry);


#endif //InitializeListHead


#endif


////////////////////////////////////////////////////////////////////////////////
//
// Macro definitions
//
////////////////////////////////////////////////////////////////////////////////

#ifdef ASSERT
#undef ASSERT
#endif 

#ifdef DBG

// Prints a message to the debugger
void 
dbgprint(
        char *format,
        ...
        );

#define ASSERT(exp)                                             \
        if ( !(exp) )                                           \
            dbgprint("\n*** Assertion failed: %s\n"              \
                       "***      Source file: %s, line: %d\n\n", \
                       #exp,__FILE__,__LINE__), DebugBreak()
#else

// On free builds, define these to be empty
#define ASSERT(exp)
#define dbgprint

#endif

#endif
