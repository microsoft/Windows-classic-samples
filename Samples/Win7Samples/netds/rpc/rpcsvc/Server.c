// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.


//  MODULE:   server.c
//
//  PURPOSE:  Implements the body of the RPC service sample.
//
//  FUNCTIONS:
//            Called by service.c:
//            ServiceStart(DWORD dwArgc, LPTSTR *lpszArgv);
//            ServiceStop( );
//
//            Called by RPC:
//            error_status_t Ping(handle_t)
//
//  COMMENTS: The ServerStart and ServerStop functions implemented here are
//            prototyped in service.h.  The other functions are RPC manager
//            functions prototypes in rpcsvc.h.
//              



#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <tchar.h>
#include <rpc.h>
#include "service.h"
#include "rpcsvc.h"

//
// RPC configuration.
//

// This service listens to all the protseqs listed in this array.
// This should be read from the service's configuration in the
// registery.

TCHAR *ProtocolArray[] = { TEXT("ncalrpc"),
                           TEXT("ncacn_ip_tcp"),
                           TEXT("ncacn_np"),
                           TEXT("ncadg_ip_udp")
                         };

// Used in RpcServerUseProtseq, for some protseqs
// this is used as a hint for buffer size.
ULONG ProtocolBuffer = 3;

// Use in RpcServerListen().  More threads will increase performance,
// but use more memory.
ULONG MinimumThreads = 3;

//
//  FUNCTION: ServiceStart
//
//  PURPOSE: Actual code of the service
//           that does the work.
//
//  PARAMETERS:
//    dwArgc   - number of command line arguments
//    lpszArgv - array of command line arguments
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//    Starts the service listening for RPC requests.
//
VOID ServiceStart (DWORD dwArgc, LPTSTR *lpszArgv)
{
    UINT i;
    RPC_BINDING_VECTOR *pbindingVector = 0;
    RPC_STATUS status;
    BOOL fListening = FALSE;

    ///////////////////////////////////////////////////
    //
    // Service initialization
    //

    //
    // Use protocol sequences (protseqs) specified in ProtocolArray.
    //

    for(i = 0; i < sizeof(ProtocolArray)/sizeof(TCHAR *); i++)
        {

        // Report the status to the service control manager.
        if (!ReportStatusToSCMgr(
            SERVICE_START_PENDING, // service state
            NO_ERROR,              // exit code
            3000))                 // wait hint
            return;


        status = RpcServerUseProtseq(ProtocolArray[i],
                                     ProtocolBuffer,
                                     0);

        if (status == RPC_S_OK)
            {
            fListening = TRUE;
            }
        }

    if (!fListening)
        {
        // Unable to listen to any protocol!
        //
        AddToMessageLog(TEXT("RpcServerUseProtseq() failed\n"));
        return;
        }

    // Report the status to the service control manager.
    //
    if (!ReportStatusToSCMgr(
        SERVICE_START_PENDING, // service state
        NO_ERROR,              // exit code
        3000))                 // wait hint
        return;

    // Register the services interface(s).
    //

    status = RpcServerRegisterIf(RpcServiceSample_v1_0_s_ifspec,   // from rpcsvc.h
                                 0,
                                 0);


    if (status != RPC_S_OK)
        return;

    // Report the status to the service control manager.
    //
    if (!ReportStatusToSCMgr(
        SERVICE_START_PENDING, // service state
        NO_ERROR,              // exit code
        3000))                 // wait hint
        return;


    // Register interface(s) and binding(s) (endpoints) with
    // the endpoint mapper.
    //

    status = RpcServerInqBindings(&pbindingVector);

    if (status != RPC_S_OK)
        {
        return;
        }

    status = RpcEpRegister(RpcServiceSample_v1_0_s_ifspec,   // from rpcsvc.h
                           pbindingVector,
                           0,
                           0);

    if (status != RPC_S_OK)
        {
        return;
        }

    // Report the status to the service control manager.
    //
    if (!ReportStatusToSCMgr(
        SERVICE_START_PENDING, // service state
        NO_ERROR,              // exit code
        3000))                 // wait hint
        return;

    // Enable NT LM Security Support Provider (NtLmSsp service)
    //
    status = RpcServerRegisterAuthInfo(0,
                                       RPC_C_AUTHN_WINNT,
                                       0,
                                       0
                                       );
    if (status != RPC_S_OK)
        {
        return;
        }

    // Report the status to the service control manager.
    //
    if (!ReportStatusToSCMgr(
        SERVICE_START_PENDING, // service state
        NO_ERROR,              // exit code
        3000))                 // wait hint
        return;


    // Start accepting client calls.
    //
    status = RpcServerListen(MinimumThreads,
                             RPC_C_LISTEN_MAX_CALLS_DEFAULT,  // rpcdce.h
                             TRUE);                           // don't block.

    if (status != RPC_S_OK)
        {
        return;
        }

    // Report the status to the service control manager.
    //
    if (!ReportStatusToSCMgr(
        SERVICE_RUNNING,       // service state
        NO_ERROR,              // exit code
        0))                    // wait hint
        return;

    //
    // End of initialization
    //
    ////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////
    //
    // Cleanup
    //

    // RpcMgmtWaitServerListen() will block until the server has
    // stopped listening.  If this service had something better to
    // do with this thread, it would delay this call until
    // ServiceStop() had been called. (Set an event in ServiceStop()).
    //
    status = RpcMgmtWaitServerListen();

    // ASSERT(status == RPC_S_OK)

    // Remove entries from the endpoint mapper database.
    //
    RpcEpUnregister(RpcServiceSample_v1_0_s_ifspec,   // from rpcsvc.h
                    pbindingVector,
                    0);

    // Delete the binding vector
    //
    RpcBindingVectorFree(&pbindingVector);

    //
    ////////////////////////////////////////////////////////////
    return;
}


//
//  FUNCTION: ServiceStop
//
//  PURPOSE: Stops the service
//
//  PARAMETERS:
//    none
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//    If a ServiceStop procedure is going to
//    take longer than 3 seconds to execute,
//    it should spawn a thread to execute the
//    stop code, and return.  Otherwise, the
//    ServiceControlManager will believe that
//    the service has stopped responding.
//    
VOID ServiceStop()
{
    // Stop's the server, wakes the main thread.

    RpcMgmtStopServerListening(0);
}


//
//  FUNCTION: Ping
//
//  PURPOSE: Implements the Ping() operation.
//
//  PARAMETERS:
//    none
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//    Ping() operation defined in rpcsvc.idl.
//    Used by clients to test the connection.
//

error_status_t
Ping(
    handle_t h
    )
{
    return(0);
}

//
//  FUNCTIONS: BufferIn1, BufferIn2, BufferIn3
//
//  PURPOSE: Implements three different methods
//           for sending data to the server.
//
//  PARAMETERS:
//    see rpcsvc.idl
//
//  RETURN VALUE:
//    error_status_t - 0;
//

error_status_t
BufferIn1(
         handle_t h,
         byte Buffer[],
         unsigned long Length,
         unsigned long Size
         )
{
    return(0);
}

error_status_t
BufferIn2(
         handle_t h,
         byte Buffer[],
         unsigned long Length
         )
{
    return(0);
}

error_status_t
BufferIn3(
         handle_t h,
         byte Buffer[],
         unsigned long Length
         )
{
    return(0);
}

//
//  FUNCTIONS: BufferOut1, BufferOut2, BufferOut3, BufferOut4
//
//  PURPOSE: Implements four different methods
//           for reading data from the server.
//
//  PARAMETERS:
//    see rpcsvc.idl
//
//  RETURN VALUE:
//    error_status_t - 0;
//

error_status_t
BufferOut1(
         handle_t h,
         byte Buffer[],
         unsigned long *pLength
         )
{
    *pLength = BUFFER_SIZE;
    return(0);
}

error_status_t
BufferOut2(
          handle_t h,
          byte Buffer[],
          unsigned long Size,
          unsigned long *pLength
          )
{
    *pLength = BUFFER_SIZE;
    return(0);
}

error_status_t
BufferOut3(
          handle_t h,
          BUFFER *pBuffer
          )
{
    pBuffer->BufferLength = BUFFER_SIZE;
    pBuffer->Buffer = MIDL_user_allocate(BUFFER_SIZE);

    if (pBuffer->Buffer == 0)
        {
        return(RPC_S_OUT_OF_MEMORY);
        }
    return(0);
}

error_status_t
BufferOut4(
          handle_t h,
          byte Buffer[],
          unsigned long *pLength
          )
{
    *pLength = BUFFER_SIZE;
    return(0);
}

//
//  FUNCTIONS: StructsIn1, StructsIn2, StructsIn3
//
//  PURPOSE: Implements server side of the struct/enum operations.
//
//  PARAMETERS:
//    see rpcsvc.idl
//
//  RETURN VALUE:
//    error_status_t - 0;
//
//
error_status_t
StructsIn1(
          handle_t h,
          struct BAD1 array[50]
          )
{
    return(0);
}

error_status_t
StructsIn2(
          handle_t h,
          struct BAD2 array[50]
          )
{
    return(0);
}

error_status_t
StructsIn3(
          handle_t h,
          struct GOOD array[50]
          )
{
    return(0);
}

//
//  FUNCTIONS: ListIn, ListOut1, ListOut2
//
//  PURPOSE: Implements server side of linked list functions.
//
//
//  PARAMETERS:
//    see rpcsvc.idl
//
//  RETURN VALUE:
//    error_status_t - 0;
//
//  NOTES:
//    Since ListOut2 uses [enable_allocate] it
//    must allocate all memory for parameters
//    with RpcSsAllocate().
//

error_status_t
ListIn(
      handle_t h,
      PLIST pList
      )
{
    return(0);
}

error_status_t
ListOut1(
        handle_t h,
        LIST *pList
        )
{
    int i;
    for(i = 0; i < LIST_SIZE; i++)
        {
        pList->data = i;
        pList->pNext = MIDL_user_allocate(sizeof(LIST));
        if (pList->pNext == 0)
            {
            return(RPC_S_OUT_OF_MEMORY);
            }
        pList = pList->pNext;
        }

    pList->data = i;
    pList->pNext = 0;

    return(0);
}

error_status_t
ListOut2(
        handle_t h,
        LIST *pList
        )
{
    int i;
    for(i = 0; i < LIST_SIZE; i++)
        {
        pList->data = i;
        pList->pNext = RpcSsAllocate(sizeof(LIST));
        // RpcSsAllocate raises an exception when it
        // fails.  Use RpcSmAllocate is this is
        // undesirable.
        pList = pList->pNext;
        }

    pList->data = i;
    pList->pNext = 0;

    return(0);
}

//
//  FUNCTIONS: UnionCall1, UnionCall2
//
//  PURPOSE: Implements server side of the Union functions.
//
//  PARAMETERS:
//    see rpcsvc.idl
//
//  RETURN VALUE:
//    error_status_t - 0;
//
//
error_status_t
UnionCall1(
          handle_t h,
          unsigned long Length,
          BAD_UNION aUnion[]
          )
{
    return(0);
}

error_status_t
UnionCall2(
          handle_t h,
          GOOD_UNION *pUnion
          )
{
    return(0);
}

//
//  FUNCTION: CheckSecurity
//
//  PURPOSE: Demonstrates the RPC security APIs.
//
//  PARAMETERS:
//    h - binding to client which made the call.
//
//  RETURN VALUE:
//    0 - no error
//
error_status_t
CheckSecurity(
             handle_t h
             )
{
    RPC_STATUS status;

    // At this point the thread is running in the server
    // security context.  There is guarantee that the client
    // even used a secure connection.

    status = RpcImpersonateClient(h);

    if (status != RPC_S_OK)
        {
        return(RPC_S_ACCESS_DENIED);
        }

    // This thread is now running in the clients security context.

    //
    // The server should now open a file, mutex, event or its own data
    // structure which has an ACL associated with it to check that the
    // client has the right to access the server's protected data.
    //

    status = RpcRevertToSelf();

    // ASSERT(status == RPC_S_OK);

    // This thread is now running in the server's security context.

    return(0);
}


//
//  FUNCTIONS: MIDL_user_allocate and MIDL_user_free
//
//  PURPOSE: Used by stubs to allocate and free memory
//           in standard RPC calls. Not used when
//           [enable_allocate] is specified in the .acf.
//
//
//  PARAMETERS:
//    See documentations.
//
//  RETURN VALUE:
//    Exceptions on error.  This is not required,
//    you can use -error allocation on the midl.exe
//    command line instead.
//
//
void * __RPC_USER MIDL_user_allocate(size_t size)
{
    return(HeapAlloc(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, size));
}

void __RPC_USER MIDL_user_free( void *pointer)
{
    HeapFree(GetProcessHeap(), 0, pointer);
}

