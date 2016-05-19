/////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2003 <company name>
//
//  Module Name:
//      ClipBook Server.cpp
//
//  Description:
//      Resource DLL for ClipBook Server (ClipBook Server).
//
//  Author:
//      <name> (<e-mail name>) Mmmm DD, 2003
//
//  Revision History:
//
//  Notes:
//
/////////////////////////////////////////////////////////////////////////////

#include "ClRes.h"

//
// Type and constant definitions.
//

typedef struct _CLIPBOOKSERVER_RESOURCE
{
    RESID                   resid;          // For validation.
    HCLUSTER                hCluster;
    HRESOURCE               hResource;
    SC_HANDLE               hService;
    DWORD                   dwProcessId;
    HKEY                    hkeyParameters;
    RESOURCE_HANDLE         hResourceHandle;
    LPWSTR                  pwszResourceName;
    CLUS_WORKER             cwWorkerThread;
    CLUSTER_RESOURCE_STATE  state;
} CLIPBOOKSERVER_RESOURCE, * PCLIPBOOKSERVER_RESOURCE;


///////////////////////////////////////////////////////////////////////////////
// Global data.
///////////////////////////////////////////////////////////////////////////////

//
//  Forward reference to our RESAPI function table.
//

extern CLRES_FUNCTION_TABLE g_ClipBookServerFunctionTable;

//
//  Single instance semaphore.
//

#define CLIPBOOKSERVER_SINGLE_INSTANCE_SEMAPHORE L"Cluster_ClipBookServer_Semaphore"
static HANDLE g_hSingleInstanceSemaphoreClipBookServer = NULL;
static PCLIPBOOKSERVER_RESOURCE g_pSingleInstanceResourceClipBookServer = NULL;



//
// Function prototypes.
//

RESID WINAPI
ClipBookServerOpen(
      LPCWSTR           pwszResourceNameIn
    , HKEY              hkeyResourceKeyIn
    , RESOURCE_HANDLE   hResourceHandleIn
    );

void WINAPI
ClipBookServerClose(
    RESID residIn
    );

DWORD WINAPI
ClipBookServerOnline(
      RESID     residIn
    , PHANDLE   phEventHandleInout
    );

DWORD WINAPI
ClipBookServerOnlineThread(
      PCLUS_WORKER              pWorkerIn
    , PCLIPBOOKSERVER_RESOURCE   pResourceEntryIn
    );

DWORD WINAPI
ClipBookServerOffline(
    RESID residIn
    );

DWORD WINAPI
ClipBookServerOfflineThread(
      PCLUS_WORKER              pWorkerIn
    , PCLIPBOOKSERVER_RESOURCE   pResourceEntryIn
    );

void WINAPI
ClipBookServerTerminate(
    RESID residIn
    );

BOOL WINAPI
ClipBookServerLooksAlive(
    RESID residIn
    );

BOOL WINAPI
ClipBookServerIsAlive(
    RESID residIn
    );

BOOL
ClipBookServerCheckIsAlive(
      PCLIPBOOKSERVER_RESOURCE   pResourceEntryIn
    , BOOL                      fFullCheckIn
    );

DWORD WINAPI
ClipBookServerResourceControl(
      RESID     residIn
    , DWORD     nControlCodeIn
    , PVOID     pInBufferIn
    , DWORD     cbInBufferSizeIn
    , PVOID     pOutBufferOut
    , DWORD     cbOutBufferSizeIn
    , LPDWORD   pcbBytesReturnedOut
    );

DWORD
ClipBookServerGetRequiredDependencies(
      PVOID     pOutBufferOut
    , DWORD     cbOutBufferSizeIn
    , LPDWORD   pcbBytesReturnedOut
    );

DWORD
ClipBookServerVerifyRequiredDependencies(
    PCLIPBOOKSERVER_RESOURCE pResourceEntryIn
    );

DWORD
ClipBookServerSetNameHandler(
      PCLIPBOOKSERVER_RESOURCE   pResourceEntryIn
    , LPWSTR                    pwszNameIn
    );


/////////////////////////////////////////////////////////////////////////////
//++
//
//  ClipBookServerDllMain
//
//  Description:
//      Main DLL entry point for the ClipBook Server resource type.
//
//  Arguments:
//      hDllHandleIn
//          DLL instance handle.
//
//      nReasonIn
//          Reason for being called.
//
//      ReservedIn
//          Reserved argument.
//
//  Return Value:
//      TRUE
//          Success.
//
//      FALSE
//          Failure.
//
//--
/////////////////////////////////////////////////////////////////////////////
BOOL WINAPI
ClipBookServerDllMain(
      HINSTANCE hDllHandleIn
    , DWORD     nReasonIn
    , LPVOID    ReservedIn
    )
{
    BOOL    fSuccess = TRUE;
    DWORD   sc = ERROR_SUCCESS;

    UNREFERENCED_PARAMETER( hDllHandleIn );
    UNREFERENCED_PARAMETER( ReservedIn );

    switch ( nReasonIn )
    {
        case DLL_PROCESS_ATTACH:
            g_hSingleInstanceSemaphoreClipBookServer = CreateSemaphoreW(
                  NULL
                , 0
                , 1
                , CLIPBOOKSERVER_SINGLE_INSTANCE_SEMAPHORE
                );
            sc = GetLastError();
            if ( g_hSingleInstanceSemaphoreClipBookServer == NULL )
            {
                fSuccess = FALSE;
                goto Cleanup;
            } // if: error creating semaphore

            if ( sc != ERROR_ALREADY_EXISTS )
            {
                // If the semaphore didnt exist, set its initial count to 1.
                ReleaseSemaphore( g_hSingleInstanceSemaphoreClipBookServer, 1, NULL );
            } // if: semaphore didn't already exist

            break;

        case DLL_PROCESS_DETACH:
            if ( g_hSingleInstanceSemaphoreClipBookServer != NULL )
            {
                CloseHandle( g_hSingleInstanceSemaphoreClipBookServer );
                g_hSingleInstanceSemaphoreClipBookServer = NULL;
            } // if: single instance semaphore was created

            if ( g_schSCMHandle != NULL )
            {
                CloseServiceHandle( g_schSCMHandle );
                g_schSCMHandle = NULL;
            } // if: global SCM handle was opened

            break;

    } // switch: nReason

Cleanup:

    return fSuccess;

} //*** ClipBookServerDllMain


/////////////////////////////////////////////////////////////////////////////
//++
//
//  ClipBookServerStartup
//
//  Description:
//      Startup the resource DLL for the ClipBook Server resource type.
//      This routine verifies that at least one currently supported version
//      of the resource DLL is between nMinVersionSupported and
//      nMaxVersionSupported. If not, then the resource DLL should return
//      ERROR_REVISION_MISMATCH.
//
//      If more than one version of the resource DLL interface is supported
//      by the resource DLL, then the highest version (up to
//      nMaxVersionSupported) should be returned as the resource DLL's
//      interface. If the returned version is not within range, then startup
//      fails.
//
//      The Resource Type is passed in so that if the resource DLL supports
//      more than one Resource Type, it can pass back the correct function
//      table associated with the Resource Type.
//
//  Arguments:
//      pwszResourceTypeIn
//          Type of resource requesting a function table.
//
//      nMinVersionSupportedIn
//          Minimum resource DLL interface version supported by the cluster
//          software.
//
//      nMaxVersionSupportedIn
//          Maximum resource DLL interface version supported by the cluster
//          software.
//
//      pfnSetResourceStatusIn
//          Pointer to a routine that the resource DLL should call to update
//          the state of a resource after the Online or Offline routine
//          have returned a status of ERROR_IO_PENDING.
//
//      pfnLogEventIn
//          Pointer to a routine that handles the reporting of events from
//          the resource DLL.
//
//      pFunctionTableIn
//          Returns a pointer to the function table defined for the version
//          of the resource DLL interface returned by the resource DLL.
//
//  Return Value:
//      ERROR_SUCCESS
//          The operation was successful.
//
//      ERROR_CLUSTER_RESNAME_NOT_FOUND
//          The resource type name is unknown by this DLL.
//
//      ERROR_REVISION_MISMATCH
//          The version of the cluster service doesn't match the version of
//          the DLL.
//
//      Win32 error code
//          The operation failed.
//
//--
/////////////////////////////////////////////////////////////////////////////
DWORD WINAPI
ClipBookServerStartup(
      LPCWSTR                       pwszResourceTypeIn
    , DWORD                         nMinVersionSupportedIn
    , DWORD                         nMaxVersionSupportedIn
    , PSET_RESOURCE_STATUS_ROUTINE  pfnSetResourceStatusIn
    , PLOG_EVENT_ROUTINE            pfnLogEventIn
    , PCLRES_FUNCTION_TABLE *       pFunctionTableOut
    )
{
    DWORD sc = ERROR_SUCCESS;

    // These are stored in the calling DllMain.
    UNREFERENCED_PARAMETER( pfnSetResourceStatusIn );
    UNREFERENCED_PARAMETER( pfnLogEventIn );

    if (   (nMinVersionSupportedIn > CLRES_VERSION_V1_00)
        || (nMaxVersionSupportedIn < CLRES_VERSION_V1_00) )
    {
        sc = ERROR_REVISION_MISMATCH;
    } // if: version not supported
    else if ( 0 == CompareStringW(
                        LOCALE_SYSTEM_DEFAULT,
                        NORM_IGNORECASE,
                        pwszResourceTypeIn,
                        -1,
                        RESTYPE_NAME,
                        -1
                        )
            )
    {
        *pFunctionTableOut = &g_ClipBookServerFunctionTable;
        sc = ERROR_SUCCESS;
    } // else if: we support this type of resource
    else
    {
        //
        // We don't support this resource type.
        //

        sc = ERROR_CLUSTER_RESNAME_NOT_FOUND;
    } // else: resource type name not supported

    return sc;

} //*** ClipBookServerStartup


/////////////////////////////////////////////////////////////////////////////
//++
//
//  ClipBookServerOpen
//
//  Description:
//      Open routine for ClipBook Server resources.
//
//      Open the specified resource (create an instance of the resource).
//      Allocate all structures necessary to bring the specified resource
//      online.
//
//  Arguments:
//      pwszResourceNameIn
//          Supplies the name of the resource to open.
//
//      hkeyResourceKeyIn
//                  Supplies handle to the resource's cluster database key.
//
//      hResourceHandleIn
//          A handle that is passed back to the Resource Monitor when the
//          SetResourceStatus or LogEvent method is called.  See the
//          description of the pfnSetResourceStatus and pfnLogEvent arguments
//          to the ClipBookServerStartup routine.  This handle should never be
//          closed or used for any purpose other than passing it as an
//          argument back to the Resource Monitor in the SetResourceStatus or
//          LogEvent callbacks.
//
//  Return Value:
//      resid
//          RESID of opened resource.
//
//      NULL
//          Error occurred opening the resource.  Resource Monitor may call
//          GetLastError() to get more details on the error.
//
//--
/////////////////////////////////////////////////////////////////////////////
RESID WINAPI
ClipBookServerOpen(
      LPCWSTR           pwszResourceNameIn
    , HKEY              hkeyResourceKeyIn
    , RESOURCE_HANDLE   hResourceHandleIn
    )
{
    DWORD                   sc = ERROR_SUCCESS;
    size_t                  cchBuffer = 0;
    RESID                   resid = NULL;
    HKEY                    hkeyParameters = NULL;
    PCLIPBOOKSERVER_RESOURCE pResourceEntry = NULL;
    SC_HANDLE               hService = NULL;
    HRESULT                 hr = S_OK;

    //
    // Check if more than one resource of this type.
    //

    if ( WaitForSingleObject( g_hSingleInstanceSemaphoreClipBookServer, 0 ) == WAIT_TIMEOUT )
    {
        //
        // A resource of this type is already running.
        //

        (g_pfnLogEvent)(
              hResourceHandleIn
            , LOG_ERROR
            , L"A resource of this type is already running.\n"
            );
        sc = ERROR_OBJECT_ALREADY_EXISTS;
        goto Cleanup;
    } // if: semaphore for resources of this type already already locked

    if ( g_pSingleInstanceResourceClipBookServer != NULL )
    {
        (g_pfnLogEvent)(
              hResourceHandleIn
            , LOG_ERROR
            , L"Single instance resource already set!\n"
            );
        sc = ERROR_OBJECT_ALREADY_EXISTS;
        goto Cleanup;
    } // if: resource of this type already exists

    //
    // Get a global handle to the Service Control Manager (SCM).
    //

    if ( g_schSCMHandle == NULL )
    {
        g_schSCMHandle = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );
        if ( g_schSCMHandle == NULL )
        {
            sc = GetLastError();
            (g_pfnLogEvent)(
                  hResourceHandleIn
                , LOG_ERROR
                , L"Failed to open Service Control Manager. Error: %1!u!.\n"
                , sc
                );
            goto Cleanup;
        } // if: error opening the Service Control Manager
    } // if: Service Control Manager not open yet

    //
    // Make sure the service is ready to be controlled by the cluster.
    //

    //
    // Note: We do this in Open so that as soon as the resource is created
    //       the service is configured for cluster service control.
    //

    sc = ResUtilSetResourceServiceStartParameters(
              CLIPBOOKSERVER_SVCNAME
            , g_schSCMHandle
            , &hService
            , g_pfnLogEvent
            , hResourceHandleIn
            );
    if ( sc != ERROR_SUCCESS )
    {
        (g_pfnLogEvent)(
              hResourceHandleIn
            , LOG_ERROR
            , L"Failed to set the service start parameters for the '%1!s!' service. Error: %2!u!.\n"
            , CLIPBOOKSERVER_SVCNAME
            , sc
            );
        goto Cleanup;
    } // if:  error setting service start parameters

    if ( hService != NULL )
    {
        CloseServiceHandle( hService );
        hService = NULL;
    } // if: hService is not NULL
    
    //
    // Make sure the service has been stopped.
    //

    sc = ResUtilStopResourceService( CLIPBOOKSERVER_SVCNAME );
    if ( sc != ERROR_SUCCESS )
    {
        (g_pfnLogEvent)(
              hResourceHandleIn
            , LOG_ERROR
            , L"Failed to stop the '%1!s!' service. Error: %2!u!.\n"
            , CLIPBOOKSERVER_SVCNAME
            , sc
            );
        goto Cleanup;
    } // if: error stopping the service

    //
    // Open the Parameters registry key for this resource.
    //

    sc = ClusterRegOpenKey(
              hkeyResourceKeyIn
            , L"Parameters"
            , KEY_ALL_ACCESS
            , &hkeyParameters
            );
    if ( sc != ERROR_SUCCESS )
    {
        (g_pfnLogEvent)(
              hResourceHandleIn
            , LOG_ERROR
            , L"Open: Unable to open Parameters key. Error: %1!u!.\n"
            , sc
            );
        goto Cleanup;
    } // if: error creating the Parameters key for the resource

    //
    // Allocate a resource entry.
    //

    pResourceEntry = new CLIPBOOKSERVER_RESOURCE;
    if ( pResourceEntry == NULL )
    {
        sc = GetLastError();
        (g_pfnLogEvent)(
              hResourceHandleIn
            , LOG_ERROR
            , L"Open: Unable to allocate resource entry structure. Error: %1!u!.\n"
            , sc
            );
        goto Cleanup;
    } // if: error allocating memory for the resource

    //
    // Initialize the resource entry..
    //

    ZeroMemory( pResourceEntry, sizeof( *pResourceEntry ) );

    pResourceEntry->resid = static_cast< RESID >( pResourceEntry ); // for validation
    pResourceEntry->hResourceHandle = hResourceHandleIn;
    pResourceEntry->hkeyParameters = hkeyParameters;
    pResourceEntry->state = ClusterResourceOffline;

    //
    // Save the name of the resource.
    //

    cchBuffer = lstrlenW( pwszResourceNameIn ) + 1;
    pResourceEntry->pwszResourceName = new WCHAR[ cchBuffer ];
    if ( pResourceEntry->pwszResourceName == NULL )
    {
        sc = GetLastError();
        (g_pfnLogEvent)(
              hResourceHandleIn
            , LOG_ERROR
            , L"Open: Unable to allocate the resource name buffer. Error: %1!u!.\n"
            , sc
            );
        goto Cleanup;
    } // if: error allocating memory for the name.

    hr = StringCchCopyW( pResourceEntry->pwszResourceName, cchBuffer, pwszResourceNameIn );
    if ( FAILED( hr ) )
    {
        sc = HRESULT_CODE( hr );
        (g_pfnLogEvent)(
              hResourceHandleIn
            , LOG_ERROR
            , L"Open: Unable to allocate the resource name buffer. Error: %1!u!.\n"
            , sc
            );
        goto Cleanup;
    } // if:

    //
    // Open the cluster.
    //

    pResourceEntry->hCluster = OpenCluster( NULL );
    if ( pResourceEntry->hCluster == NULL )
    {
        sc = GetLastError();
        (g_pfnLogEvent)(
              hResourceHandleIn
            , LOG_ERROR
            , L"Open: Unable to open the cluster. Error: %1!u!.\n"
            , sc
            );
        goto Cleanup;
    } // if: error opening the cluster

    //
    // Open the resource.
    //

    pResourceEntry->hResource = OpenClusterResource( pResourceEntry->hCluster, pwszResourceNameIn );
    if ( pResourceEntry->hResource == NULL )
    {
        sc = GetLastError();
        (g_pfnLogEvent)(
              hResourceHandleIn
            , LOG_ERROR
            , L"Open: Unable to open the resource. Error: %1!u!.\n"
            , sc
            );
        goto Cleanup;
    } // if: error opening the resource

    //
    // Initialization for the resource.
    //

    //
    // TODO: Add your custom resource open code here.
    //

    resid = static_cast< RESID >( pResourceEntry );
    g_pSingleInstanceResourceClipBookServer = pResourceEntry;
    sc = ERROR_SUCCESS;

Cleanup:

    if ( resid == NULL )
    {
        assert( sc != ERROR_SUCCESS );

        (g_pfnLogEvent)(
              hResourceHandleIn
            , LOG_ERROR
            , L"Open: failed with error %1!u!.\n"
            , sc
            );

        if ( hkeyParameters != NULL )
        {
            ClusterRegCloseKey( hkeyParameters );
        } // if:

        if ( pResourceEntry != NULL )
        {
            if ( pResourceEntry->hResource != NULL )
            {
                CloseClusterResource( pResourceEntry->hResource );
            } // if:

            if ( pResourceEntry->hCluster != NULL )
            {
                CloseCluster( pResourceEntry->hCluster );
            } // if:

            delete [] pResourceEntry->pwszResourceName;
            delete pResourceEntry;

        } // if: resource entry allocated
    } // if: something failed

    if ( hService != NULL )
    {
        CloseServiceHandle( hService );
        hService = NULL;
    } // if: hService is not NULL
    
    SetLastError( sc );

    return resid;

} //*** ClipBookServerOpen


/////////////////////////////////////////////////////////////////////////////
//++
//
//  ClipBookServerClose
//
//  Description:
//      Close routine for ClipBook Server resources.
//
//      Close the specified resource and deallocate all structures, etc.,
//      allocated in the Open call.  If the resource is not in the offline
//      state, then the resource should be taken offline (by calling
//      Terminate) before the close operation is performed.
//
//  Arguments:
//      residIn
//          Supplies the resource ID  of the resource to close.
//
//  Return Value:
//      None.
//
//--
/////////////////////////////////////////////////////////////////////////////
void WINAPI
ClipBookServerClose(
    RESID residIn
    )
{
    PCLIPBOOKSERVER_RESOURCE pResourceEntry = NULL;
    DWORD                   sc = ERROR_SUCCESS;

    //
    // Verify we have a valid resource ID.
    //

    pResourceEntry = static_cast< PCLIPBOOKSERVER_RESOURCE >( residIn );
    if ( pResourceEntry == NULL )
    {
        DBG_PRINT( "ClipBookServer: Close request for a nonexistent resource id.\n" );
        sc = ERROR_RESOURCE_NOT_FOUND;
        goto Cleanup;
    } // if: NULL resource ID

    if ( pResourceEntry->resid != residIn )
    {
        (g_pfnLogEvent)(
              pResourceEntry->hResourceHandle
            , LOG_ERROR
            , L"Close resource sanity check failed! resid = 0x%1!08x!.\n"
            , residIn
            );
        sc = ERROR_RESOURCE_NOT_FOUND;
        goto Cleanup;
    } // if: invalid resource ID

    if ( pResourceEntry->pwszResourceName == NULL )
    {
        (g_pfnLogEvent)(
              pResourceEntry->hResourceHandle
            , LOG_INFORMATION
            , L"Close request for resource with resid 0x%1!08x!.\n"
            , residIn
            );
    } // if: resource name is null...
    else
    {
        (g_pfnLogEvent)(
              pResourceEntry->hResourceHandle
            , LOG_INFORMATION
            , L"Close request for resource '%1!s!'.\n"
            , pResourceEntry->pwszResourceName
            );
    } // else: resource name is not null...

    //
    // Close the Parameters key.
    //

    if ( pResourceEntry->hkeyParameters != NULL )
    {
        ClusterRegCloseKey( pResourceEntry->hkeyParameters );
        pResourceEntry->hkeyParameters = NULL;
    } // if: parameters key is open

    //
    // Close the cluster handle.
    //

    if ( pResourceEntry->hCluster != NULL )
    {
        CloseCluster( pResourceEntry->hCluster );
        pResourceEntry->hCluster = NULL;
    } // if: cluster handle is open

    //
    // Close the resource handle.
    //

    if ( pResourceEntry->hResource != NULL )
    {
        CloseClusterResource( pResourceEntry->hResource );
        pResourceEntry->hResource = NULL;
    } // if: resource handle is open

    //
    // Clean up the semaphore if this is the single resource instance.
    //

    if ( pResourceEntry == g_pSingleInstanceResourceClipBookServer )
    {
        (g_pfnLogEvent)(
              pResourceEntry->hResourceHandle
            , LOG_INFORMATION
            , L"Close: Setting semaphore '%1!s!'.\n"
            , CLIPBOOKSERVER_SINGLE_INSTANCE_SEMAPHORE
            );
        g_pSingleInstanceResourceClipBookServer = NULL;
        ReleaseSemaphore( g_hSingleInstanceSemaphoreClipBookServer, 1 , NULL );
    } // if: this is the single resource instance

    //
    // Release the service handle in case it is still open.
    //

    if ( pResourceEntry->hService != NULL )
    {
        CloseServiceHandle( pResourceEntry->hService );
        pResourceEntry->hService = NULL;
    } // if:

    // ADDPARAM: Add new properties here.

    //
    // The ResUtil API's use LocalAlloc, so use LocalFree on the prop list members.
    //


    //
    // Deallocate the resource entry.
    //

    delete [] pResourceEntry->pwszResourceName;
    delete pResourceEntry;

    sc = ERROR_SUCCESS;

Cleanup:

    SetLastError( sc );

    return;

} //*** ClipBookServerClose


/////////////////////////////////////////////////////////////////////////////
//++
//
//  ClipBookServerOnline
//
//  Description:
//      Online routine for ClipBook Server resources.
//
//      Bring the specified resource online (available for use).  The resource
//      DLL should attempt to arbitrate for the resource if it is present on
//      a shared medium, like a shared SCSI bus.
//
//  Arguments:
//      residIn
//          Supplies the resource ID of the resource to be brought online
//          (available for use).
//
//      phEventHandleOut
//          Returns a signalable handle that is signaled when the resource DLL
//          detects a failure on the resource.  This argument derferences as NULL
//          on input, and the resource DLL returns NULL if asynchronous
//          notification of failurs is not supported.  Otherwise this must be
//          the address of a handle that is signaled on resource failures.
//
//  Return Value:
//      ERROR_SUCCESS
//          The operation was successful, and the resource is now online.
//
//      ERROR_RESOURCE_NOT_FOUND
//          Resource ID is not valid.
//
//      ERROR_RESOURCE_NOT_AVAILABLE
//          If the resource was arbitrated with some other systems and one of
//          the other systems won the arbitration.
//
//      ERROR_IO_PENDING
//          The request is pending.  A thread has been activated to process
//          the online request.  The thread that is processing the online
//          request will periodically report status by calling the
//          SetResourceStatus callback method until the resource is placed
//          into the ClusterResourceOnline state (or the resource monitor
//          decides to timeout the online request and Terminate the resource.
//          This pending timeout value is settable and has a default value of
//          3 minutes.).
//
//      Win32 error code
//          The operation failed.
//
//--
/////////////////////////////////////////////////////////////////////////////
DWORD WINAPI
ClipBookServerOnline(
      RESID     residIn
    , PHANDLE   phEventHandleOut
    )
{
    PCLIPBOOKSERVER_RESOURCE pResourceEntry = NULL;
    DWORD                   sc = ERROR_SUCCESS;

    UNREFERENCED_PARAMETER( phEventHandleOut );

    //
    // Verify we have a valid resource ID.
    //

    pResourceEntry = static_cast< PCLIPBOOKSERVER_RESOURCE >( residIn );
    if ( pResourceEntry == NULL )
    {
        DBG_PRINT( "ClipBook Server: Online request for a nonexistent resource id.\n" );
        return ERROR_RESOURCE_NOT_FOUND;
    } // if: NULL resource ID

    if ( pResourceEntry->resid != residIn )
    {
        (g_pfnLogEvent)(
              pResourceEntry->hResourceHandle
            , LOG_ERROR
            , L"Online sanity check failed! resid = 0x%1!08x!.\n"
            , residIn
            );
        return ERROR_RESOURCE_NOT_FOUND;
    } // if: invalid resource ID

    (g_pfnLogEvent)(
          pResourceEntry->hResourceHandle
        , LOG_INFORMATION
        , L"Online request.\n"
        );

    //
    // Start the Online thread to perform the online operation.
    //

    pResourceEntry->state = ClusterResourceOnlinePending;
    ClusWorkerTerminate( &pResourceEntry->cwWorkerThread );
    sc = ClusWorkerCreate(
              &pResourceEntry->cwWorkerThread
            , reinterpret_cast< PWORKER_START_ROUTINE >( ClipBookServerOnlineThread )
            , pResourceEntry
            );
    if ( sc != ERROR_SUCCESS )
    {
        pResourceEntry->state = ClusterResourceFailed;
        (g_pfnLogEvent)(
              pResourceEntry->hResourceHandle
            , LOG_ERROR
            , L"Online: Unable to start thread. Error: %1!u!.\n"
            , sc
            );
    } // if: error creating the worker thread
    else
    {
        sc = ERROR_IO_PENDING;
    } // if: worker thread created successfully

    return sc;

} //*** ClipBookServerOnline


/////////////////////////////////////////////////////////////////////////////
//++
//
//  ClipBookServerOnlineThread
//
//  Description:
//      Worker function which brings a resource online.
//      This function is executed in a separate thread.
//
//  Arguments:
//      pWorkerIn
//          Supplies the worker thread structure.
//
//      pResourceEntryIn
//          A pointer to the CLIPBOOKSERVER_RESOURCE block for this resource.
//
//  Return Value:
//      ERROR_SUCCESS
//          The operation completed successfully.
//
//      Win32 error code
//          The operation failed.
//
//--
/////////////////////////////////////////////////////////////////////////////
DWORD WINAPI
ClipBookServerOnlineThread(
      PCLUS_WORKER              pWorkerIn
    , PCLIPBOOKSERVER_RESOURCE   pResourceEntryIn
    )
{
    RESOURCE_STATUS         resourceStatus;
    DWORD                   sc = ERROR_SUCCESS;
    DWORD                   nPrevCheckpoint;
    DWORD                   cbBytesNeeded;
    SERVICE_STATUS_PROCESS  ServiceStatus = { 0 };
    RESOURCE_EXIT_STATE     resExitState;

    UNREFERENCED_PARAMETER( pWorkerIn );

    ResUtilInitializeResourceStatus( &resourceStatus );

    resourceStatus.ResourceState = ClusterResourceFailed;
    resourceStatus.WaitHint = 0;
    resourceStatus.CheckPoint = 1;

    //
    // Parameter checking.
    //

    if ( pResourceEntryIn == NULL )
    {
        sc = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    } // if:

    //
    // If we require dependencies them make sure we have them set.
    //

    sc = ClipBookServerVerifyRequiredDependencies( pResourceEntryIn );
    if ( sc != ERROR_SUCCESS )
    {
        (g_pfnLogEvent)(
              pResourceEntryIn->hResourceHandle
            , LOG_ERROR
            , L"OnlineThread: Required dependency not found. Error: %1!u!.\n"
            , sc
            );
        goto Cleanup;
    } // if: required dependencies are missing

    //
    // Create the new environment with the simulated net name when the
    // services queries GetComputerName.
    //

    if ( ClusWorkerCheckTerminate( pWorkerIn ) == TRUE )
    {
        goto Cleanup;
    } // if: terminating

    sc = ResUtilSetResourceServiceEnvironment(
              CLIPBOOKSERVER_SVCNAME
            , pResourceEntryIn->hResource
            , g_pfnLogEvent
            , pResourceEntryIn->hResourceHandle
            );
    if ( sc != ERROR_SUCCESS )
    {
        (g_pfnLogEvent)(
              pResourceEntryIn->hResourceHandle
            , LOG_ERROR
            , L"OnlineThread: Failed to set service environment. Error: %1!u!.\n"
            , sc
            );
        goto Cleanup;
    } // if: error setting the environment for the service

    //
    // Stop the service if it's running since we are about to change
    // its parameters.
    //

    sc = ResUtilStopResourceService( CLIPBOOKSERVER_SVCNAME );
    if ( sc != ERROR_SUCCESS )
    {
        (g_pfnLogEvent)(
              pResourceEntryIn->hResourceHandle
            , LOG_ERROR
            , L"OnlineThread: Failed to stop the '%1!s!' service. Error: %2!u!.\n"
            , CLIPBOOKSERVER_SVCNAME
            , sc
            );
        goto Cleanup;
    } // if: error stopping the service

    //
    // Make sure the service is ready to be controlled by the cluster.
    //

    if ( ClusWorkerCheckTerminate( pWorkerIn ) == TRUE )
    {
        goto Cleanup;
    } // if: terminating

    sc = ResUtilSetResourceServiceStartParameters(
              CLIPBOOKSERVER_SVCNAME
            , g_schSCMHandle
            , &pResourceEntryIn->hService
            , g_pfnLogEvent
            , pResourceEntryIn->hResourceHandle
            );
    if ( sc != ERROR_SUCCESS )
    {
        (g_pfnLogEvent)(
              pResourceEntryIn->hResourceHandle
            , LOG_ERROR
            , L"OnlineThread: Failed to set service start parameters. Error: %1!u!.\n"
            , sc
            );
        goto Cleanup;
    } // if:  error setting service start parameters

    //
    // Perform resource-specific initialization before starting the service.
    //

    //
    // TODO: Add code to initialize the resource before starting the service.
    //

    //
    // Start the service.
    //

    if ( ! StartServiceW( pResourceEntryIn->hService, 0, NULL ) )
    {
        sc = GetLastError();
        if ( sc != ERROR_SERVICE_ALREADY_RUNNING )
        {
            //
            //  TODO: Log event to the event log.
            //

            (g_pfnLogEvent)(
                  pResourceEntryIn->hResourceHandle
                , LOG_ERROR
                , L"OnlineThread: Failed to start the '%1!s!' service. Error: %2!u!.\n"
                , CLIPBOOKSERVER_SVCNAME
                , sc
                );
            sc = ERROR_SERVICE_NEVER_STARTED;
            goto Cleanup;
        } // if: error other than service already running occurred
    } // if: error starting the service

    //
    // Query the status of the service in a loop until it leaves
    // the pending state.
    //

    sc = ERROR_SUCCESS;
    resourceStatus.ResourceState = ClusterResourceOnlinePending;
    nPrevCheckpoint = 0;
    while ( ! ClusWorkerCheckTerminate( pWorkerIn ) )
    {
        //
        // Query the service status.
        //

        if ( ! QueryServiceStatusEx(
                  pResourceEntryIn->hService
                , SC_STATUS_PROCESS_INFO
                , reinterpret_cast< LPBYTE >( &ServiceStatus )
                , sizeof( SERVICE_STATUS_PROCESS )
                , &cbBytesNeeded
                )
           )
        {
            sc = GetLastError();
            (g_pfnLogEvent)(
                  pResourceEntryIn->hResourceHandle
                , LOG_ERROR
                , L"OnlineThread: Failed to query service status for the '%1!s!' service. Error: %2!u!.\n"
                , CLIPBOOKSERVER_SVCNAME
                , sc
                );
            resourceStatus.ResourceState = ClusterResourceFailed;
            goto Cleanup;
        } // if: error querying service status

        //
        // If the service is no longer pending, we are done.
        //

        if ( ServiceStatus.dwCurrentState != SERVICE_START_PENDING )
        {
            break;
        } // if: service state is not pending

        //
        // If the service checkpoint value changed, use it when notifying
        // the Resource Monitor of our current status.
        //

        if ( nPrevCheckpoint != ServiceStatus.dwCheckPoint )
        {
            nPrevCheckpoint = ServiceStatus.dwCheckPoint;
            resourceStatus.CheckPoint++;
        } // if:

        //
        // Notify the Resource Monitor of our current state.
        //

        resExitState = static_cast< RESOURCE_EXIT_STATE >(
            (g_pfnSetResourceStatus)(
                  pResourceEntryIn->hResourceHandle
                , &resourceStatus
                )
            );
        if ( resExitState != ResourceExitStateContinue )
        {
            goto Cleanup;
        } // if: resource is being terminated

        //
        // Check again in 1/2 second.
        //

        Sleep( 500 );
    } // while: not terminating while querying the status of the service

    //
    // Assume that we failed.
    //

    resourceStatus.ResourceState = ClusterResourceFailed;

    //
    // If we exited the loop before setting ServiceStatus, then return now.
    //

    if ( ClusWorkerCheckTerminate( pWorkerIn ) == TRUE )
    {
        goto Cleanup;
    } // if: being terminated

    if ( ServiceStatus.dwCurrentState != SERVICE_RUNNING )
    {
        sc = ERROR_SERVICE_NEVER_STARTED;

        //
        // TODO: Log event to the event log
        //

        (g_pfnLogEvent)(
              pResourceEntryIn->hResourceHandle
            , LOG_ERROR
            , L"OnlineThread: Failed to start the '%1!s!' service. Error: %2!u!.\n"
            , CLIPBOOKSERVER_SVCNAME
            , sc
            );
        goto Cleanup;
    } // if: service not running when loop exited

    //
    // Set status to online and save process ID of the service.
    // This is used to enable us to terminate the resource more
    // effectively.
    //

    resourceStatus.ResourceState = ClusterResourceOnline;
    if ( ! (ServiceStatus.dwServiceFlags & SERVICE_RUNS_IN_SYSTEM_PROCESS) )
    {
        pResourceEntryIn->dwProcessId = ServiceStatus.dwProcessId;
    } // if: not running in the system process

    (g_pfnLogEvent)(
          pResourceEntryIn->hResourceHandle
        , LOG_INFORMATION
        , L"The '%1!s!' service is now online.\n"
        , CLIPBOOKSERVER_SVCNAME
        );


Cleanup:

    if ( sc != ERROR_SUCCESS )
    {
        (g_pfnLogEvent)(
              pResourceEntryIn->hResourceHandle
            , LOG_ERROR
            , L"OnlineThread: Error %1!u! bringing resource online.\n"
            , sc
            );

        if ( pResourceEntryIn->hService != NULL )
        {
            CloseServiceHandle( pResourceEntryIn->hService );
            pResourceEntryIn->hService = NULL;
        } // if: service handle was opened
    } // if: error occurred

    pResourceEntryIn->state = resourceStatus.ResourceState;
    g_pfnSetResourceStatus( pResourceEntryIn->hResourceHandle, &resourceStatus );

    return sc;

} //*** ClipBookServerOnlineThread


/////////////////////////////////////////////////////////////////////////////
//++
//
//  ClipBookServerOffline
//
//  Description:
//      Offline routine for ClipBook Server resources.
//
//      Take the specified resource offline (unavailable for use).  Wait
//      for any cleanup operations to complete before returning.
//
//  Arguments:
//      residIn
//          Supplies the resource ID of the resource to be shutdown
//          gracefully.
//
//  Return Value:
//      ERROR_SUCCESS
//          The operation was successful, and the resource is now offline.
//
//      ERROR_RESOURCE_NOT_FOUND
//          Resource ID is not valid.
//
//      ERROR_RESOURCE_NOT_AVAILABLE
//          If the resource was arbitrated with some other systems and one of
//          the other systems won the arbitration.
//
//      ERROR_IO_PENDING
//          The request is still pending.  A thread has been activated to
//          process the offline request.  The thread that is processing the
//          offline request will periodically report status by calling the
//          SetResourceStatus callback method until the resource is placed
//          into the ClusterResourceOffline state (or the resource monitor
//          decides  to timeout the offline request and Terminate the
//          resource).
//
//      Win32 error code
//          The operation failed.  This will cause the Resource Monitor to
//          log an event and call the Terminate routine.
//
//--
/////////////////////////////////////////////////////////////////////////////
DWORD WINAPI
ClipBookServerOffline(
    RESID residIn
    )
{
    PCLIPBOOKSERVER_RESOURCE pResourceEntry = NULL;
    DWORD                   sc = ERROR_SUCCESS;

    //
    // Verify we have a valid resource ID.
    //

    pResourceEntry = static_cast< PCLIPBOOKSERVER_RESOURCE >( residIn );
    if ( pResourceEntry == NULL )
    {
        DBG_PRINT( "ClipBook Server: Offline request for a nonexistent resource id.\n" );
        sc = ERROR_RESOURCE_NOT_FOUND;
        goto Cleanup;
    } // if: NULL resource ID

    if ( pResourceEntry->resid != residIn )
    {
        (g_pfnLogEvent)(
              pResourceEntry->hResourceHandle
            , LOG_ERROR
            , L"Offline resource sanity check failed! resid = 0x%1!08x!.\n"
            , residIn
            );
        sc = ERROR_RESOURCE_NOT_FOUND;
        goto Cleanup;
    } // if: invalid resource ID

    (g_pfnLogEvent)(
          pResourceEntry->hResourceHandle
        , LOG_INFORMATION
        , L"Offline request.\n"
        );

    //
    // Start the Offline thread to perform the offline operation.
    //

    pResourceEntry->state = ClusterResourceOfflinePending;
    ClusWorkerTerminate( &pResourceEntry->cwWorkerThread );
    sc = ClusWorkerCreate(
              &pResourceEntry->cwWorkerThread
            , reinterpret_cast< PWORKER_START_ROUTINE >( ClipBookServerOfflineThread )
            , pResourceEntry
            );
    if ( sc != ERROR_SUCCESS )
    {
        pResourceEntry->state = ClusterResourceFailed;
        (g_pfnLogEvent)(
              pResourceEntry->hResourceHandle
            , LOG_ERROR
            , L"Offline: Unable to start thread. Error: %1!u!.\n"
            , sc
            );
    } // if: error creating the worker thread
    else
    {
        sc = ERROR_IO_PENDING;
    } // if: worker thread created successfully

Cleanup:

    return sc;

} //*** ClipBookServerOffline


/////////////////////////////////////////////////////////////////////////////
//++
//
//  ClipBookServerOfflineThread
//
//  Description:
//      Worker function which takes a resource offline.
//      This function is executed in a separate thread.
//
//  Arguments:
//      pWorkerIn
//          Supplies the worker thread structure.
//
//      pResourceEntryIn
//          A pointer to the CLIPBOOKSERVER_RESOURCE block for this resource.
//
//  Return Value:
//      ERROR_SUCCESS
//          The operation completed successfully.
//
//      Win32 error code
//          The operation failed.
//
//--
/////////////////////////////////////////////////////////////////////////////
DWORD WINAPI
ClipBookServerOfflineThread(
      PCLUS_WORKER              pWorkerIn
    , PCLIPBOOKSERVER_RESOURCE   pResourceEntryIn
    )
{
    RESOURCE_STATUS     resourceStatus;
    DWORD               sc = ERROR_SUCCESS;
    DWORD               nRetryTime = 300;     // 300 msec at a time
    BOOL                fDidStop = FALSE;
    SERVICE_STATUS      ServiceStatus;

    ResUtilInitializeResourceStatus( &resourceStatus );

    resourceStatus.ResourceState = ClusterResourceFailed;
    resourceStatus.WaitHint = 0;
    resourceStatus.CheckPoint = 1;

    //
    // If the service has gone offline or was never brought online then we're done.
    //

    if ( pResourceEntryIn->hService == NULL )
    {
        resourceStatus.ResourceState = ClusterResourceOffline;
        goto Cleanup;
    }

    //
    // Try to stop the service.  Wait for it to terminate as long as we're not asked to terminate.
    //

    while ( ClusWorkerCheckTerminate( pWorkerIn ) == FALSE )
    {
        sc = (ControlService(
                      pResourceEntryIn->hService
                    , (fDidStop
                        ? SERVICE_CONTROL_INTERROGATE
                        : SERVICE_CONTROL_STOP)
                    , &ServiceStatus
                    )
                ? ERROR_SUCCESS
                : GetLastError()
                );

        if ( sc == ERROR_SUCCESS )
        {
            fDidStop = TRUE;

            if ( ServiceStatus.dwCurrentState == SERVICE_STOPPED )
            {
                (g_pfnLogEvent)(
                      pResourceEntryIn->hResourceHandle
                    , LOG_INFORMATION
                    , L"OfflineThread: The '%1!s!' service stopped.\n"
                    , CLIPBOOKSERVER_SVCNAME
                    );

                //
                // Set the status to offline.
                //

                resourceStatus.ResourceState = ClusterResourceOffline;
                CloseServiceHandle( pResourceEntryIn->hService );
                pResourceEntryIn->hService = NULL;
                pResourceEntryIn->dwProcessId = 0;

                (g_pfnLogEvent)(
                      pResourceEntryIn->hResourceHandle
                    , LOG_INFORMATION
                    , L"OfflineThread: Service is now offline.\n"
                    );
                break;
            } // if: current service state is STOPPED
        } // if: ControlService completed successfully

        if (    ( sc == ERROR_EXCEPTION_IN_SERVICE )
            ||  ( sc == ERROR_PROCESS_ABORTED )
            ||  ( sc == ERROR_SERVICE_NOT_ACTIVE )
           )
        {
            (g_pfnLogEvent)(
                  pResourceEntryIn->hResourceHandle
                , LOG_INFORMATION
                , L"OfflineThread: The '%1!s!' service died or is not active any more; status = %2!u!.\n"
                , CLIPBOOKSERVER_SVCNAME
                , sc
                );

            //
            // Set the status to offline.
            //

            resourceStatus.ResourceState = ClusterResourceOffline;
            CloseServiceHandle( pResourceEntryIn->hService );
            pResourceEntryIn->hService = NULL;
            pResourceEntryIn->dwProcessId = 0;
            (g_pfnLogEvent)(
                  pResourceEntryIn->hResourceHandle
                , LOG_INFORMATION
                , L"OfflineThread: Service is now offline.\n"
                );
            break;
        } // if: service stopped abnormally

        (g_pfnLogEvent)(
              pResourceEntryIn->hResourceHandle
            , LOG_INFORMATION
            , L"OfflineThread: retrying...\n"
            );

        Sleep( nRetryTime );

    } // while: not asked to terminate

    //
    // Undo the temporary changes to the service config.
    //

    ResUtilRemoveResourceServiceEnvironment(
              CLIPBOOKSERVER_SVCNAME
            , g_pfnLogEvent
            , pResourceEntryIn->hResourceHandle
            );

Cleanup:

    pResourceEntryIn->state = resourceStatus.ResourceState;
    g_pfnSetResourceStatus( pResourceEntryIn->hResourceHandle, &resourceStatus );

    return sc;

} //*** ClipBookServerOfflineThread


/////////////////////////////////////////////////////////////////////////////
//++
//
//  ClipBookServerTerminate
//
//  Description:
//      Terminate routine for ClipBook Server resources.
//
//      Take the specified resource offline immediately (the resource is
//      unavailable for use).
//
//  Arguments:
//      residIn
//          Supplies the resource ID of the resource to be shutdown
//          ungracefully.
//
//  Return Value:
//      None.
//
//--
/////////////////////////////////////////////////////////////////////////////
void WINAPI
ClipBookServerTerminate(
    RESID residIn
    )
{
    PCLIPBOOKSERVER_RESOURCE pResourceEntry = NULL;
    DWORD                   sc = ERROR_SUCCESS;

    //
    // Verify we have a valid resource ID.
    //

    pResourceEntry = static_cast< PCLIPBOOKSERVER_RESOURCE >( residIn );
    if ( pResourceEntry == NULL )
    {
        DBG_PRINT( "ClipBook Server: Terminate request for a nonexistent resource id.\n" );
        return;
    } // if: NULL resource ID

    if ( pResourceEntry->resid != residIn )
    {
        (g_pfnLogEvent)(
              pResourceEntry->hResourceHandle
            , LOG_ERROR
            , L"Terminate resource sanity check failed! resid = 0x%1!08x!.\n"
            , residIn
            );
        return;
    } // if: invalid resource ID

    (g_pfnLogEvent)(
          pResourceEntry->hResourceHandle
        , LOG_INFORMATION
        , L"Terminate request.\n"
        );

    //
    // Kill off any pending threads.
    //

    ClusWorkerTerminate( &pResourceEntry->cwWorkerThread );

    if ( pResourceEntry->hService != NULL )
    {
        DWORD           nTotalRetryTime = 30*1000;  // Wait 30 secs for shutdown
        DWORD           nRetryTime = 300;           // 300 msec at a time
        BOOL            fDidStop = FALSE;
        SERVICE_STATUS  ServiceStatus;

        for ( ;; )
        {
            sc = (ControlService(
                          pResourceEntry->hService
                        , (fDidStop
                            ? SERVICE_CONTROL_INTERROGATE
                            : SERVICE_CONTROL_STOP)
                        , &ServiceStatus
                        )
                    ? ERROR_SUCCESS
                    : GetLastError()
                    );

            if ( sc == ERROR_SUCCESS )
            {
                fDidStop = TRUE;

                if ( ServiceStatus.dwCurrentState == SERVICE_STOPPED )
                {
                    (g_pfnLogEvent)(
                          pResourceEntry->hResourceHandle
                        , LOG_INFORMATION
                        , L"Terminate: The '%1!s!' service stopped.\n"
                        , CLIPBOOKSERVER_SVCNAME
                        );
                    break;
                } // if: current service state is STOPPED
            } // if: ControlService completed successfully

            if (    ( sc == ERROR_EXCEPTION_IN_SERVICE )
                ||  ( sc == ERROR_PROCESS_ABORTED )
                ||  ( sc == ERROR_SERVICE_NOT_ACTIVE )
               )
            {
                (g_pfnLogEvent)(
                      pResourceEntry->hResourceHandle
                    , LOG_INFORMATION
                    , L"Terminate: Service died; status = %1!u!.\n"
                    , sc
                    );
                break;
            } // if: service stopped abnormally

            if ( (nTotalRetryTime -= nRetryTime) == 0 )
            {
                (g_pfnLogEvent)(
                      pResourceEntry->hResourceHandle
                    , LOG_ERROR
                    , L"Terminate: Service did not stop; giving up.\n"
                    );

                break;
            } // if: retried too many times

            (g_pfnLogEvent)(
                  pResourceEntry->hResourceHandle
                , LOG_INFORMATION
                , L"Terminate: retrying...\n"
                );

            Sleep( nRetryTime );
        } // for: forever

        //
        // Declare the service offline.  It may not truly be offline, so
        // if there is a pid for this service, try and terminate that process.
        // Note that terminating a process doesnt terminate all the child
        // processes.
        //

        if ( pResourceEntry->dwProcessId != 0 )
        {
            HANDLE hSvcProcess = NULL;

            hSvcProcess = OpenProcess(
                                  PROCESS_TERMINATE
                                , FALSE
                                , pResourceEntry->dwProcessId
                                );
            if ( hSvcProcess != NULL )
            {
                (g_pfnLogEvent)(
                      pResourceEntry->hResourceHandle
                    , LOG_INFORMATION
                    , L"Terminate: Terminating processid %1!u!\n"
                    , pResourceEntry->dwProcessId
                    );
                TerminateProcess( hSvcProcess, 0 );
                CloseHandle( hSvcProcess );
            } // if: opened process successfully
        } // if: service process ID available

        CloseServiceHandle( pResourceEntry->hService );
        pResourceEntry->hService = NULL;
        pResourceEntry->dwProcessId = 0;
    } // if: service was started

    //
    // Undo the temporary changes to the service config.
    //

    ResUtilRemoveResourceServiceEnvironment(
              CLIPBOOKSERVER_SVCNAME
            , g_pfnLogEvent
            , pResourceEntry->hResourceHandle
            );

    pResourceEntry->state = ClusterResourceFailed;

    return;

} //*** ClipBookServerTerminate


/////////////////////////////////////////////////////////////////////////////
//++
//
//  ClipBookServerLooksAlive
//
//  Description:
//      LooksAlive routine for ClipBook Server resources.
//
//      Perform a quick check to determine if the specified resource is
//      probably online (available for use).  This call should not block for
//      more than 300 ms, preferably less than 50 ms.
//
//  Arguments:
//      residIn
//          Supplies the resource ID for the resource to be polled.
//
//  Return Value:
//      TRUE
//          The specified resource is probably online and available for use.
//
//      FALSE
//          The specified resource is not functioning normally.  The IsAlive
//          function will be called to perform a more thorough check.
//
//--
/////////////////////////////////////////////////////////////////////////////
BOOL WINAPI
ClipBookServerLooksAlive(
    RESID residIn
    )
{
    PCLIPBOOKSERVER_RESOURCE pResourceEntry = NULL;

    //
    // Verify we have a valid resource ID.
    //

    pResourceEntry = static_cast< PCLIPBOOKSERVER_RESOURCE >( residIn );
    if ( pResourceEntry == NULL )
    {
        DBG_PRINT( "ClipBook Server: LooksAlive request for a nonexistent resource id.\n" );
        return FALSE;
    } // if: NULL resource ID

    if ( pResourceEntry->resid != residIn )
    {
        (g_pfnLogEvent)(
              pResourceEntry->hResourceHandle
            , LOG_ERROR
            , L"LooksAlive sanity check failed! resid = 0x%1!08x!.\n"
            , residIn
            );
        return FALSE;
    } // if: invalid resource ID

#ifdef LOG_VERBOSE
    (g_pfnLogEvent)(
          pResourceEntry->hResourceHandle
        , LOG_INFORMATION
        , L"LooksAlive request.\n"
        );
#endif

    //
    // TODO: LooksAlive code
    //

    //
    // NOTE: LooksAlive should be a quick check to see if the resource is
    // available or not, whereas IsAlive should be a thorough check.  If
    // there are no differences between a quick check and a thorough check,
    // IsAlive can be called for LooksAlive, as it is below.  However, if there
    // are differences, replace the call to IsAlive below with your quick
    // check code.
    //

    //
    // Check to see if the resource is alive.
    //

    return ClipBookServerCheckIsAlive( pResourceEntry, FALSE /* fFullCheck */ );

} //*** ClipBookServerLooksAlive


/////////////////////////////////////////////////////////////////////////////
//++
//
//  ClipBookServerIsAlive
//
//  Description:
//      IsAlive routine for ClipBook Server resources.
//
//      Perform a thorough check to determine if the specified resource is
//      online (available for use).  This call should not block for more
//      more than 300 ms, preferably less than 50 ms.  If it must block for
//      longer than this, create a separate thread dedicated to polling for
//      this information and have this routine return the status of the last
//      poll performed.
//
//  Arguments:
//      residIn
//          Supplies the resource ID for the resource to be polled.
//
//  Return Value:
//      TRUE
//          The specified resource is online and functioning normally.
//
//      FALSE
//          The specified resource is not functioning normally.  The resource
//          will be terminated and then Online will be called.
//
//--
/////////////////////////////////////////////////////////////////////////////
BOOL WINAPI
ClipBookServerIsAlive(
    RESID residIn
    )
{
    PCLIPBOOKSERVER_RESOURCE pResourceEntry = NULL;

    //
    // Verify we have a valid resource ID.
    //

    pResourceEntry = static_cast< PCLIPBOOKSERVER_RESOURCE >( residIn );
    if ( pResourceEntry == NULL )
    {
        DBG_PRINT( "ClipBook Server: IsAlive request for a nonexistent resource id.\n" );
        return FALSE;
    } // if: NULL resource ID

    if ( pResourceEntry->resid != residIn )
    {
        (g_pfnLogEvent)(
              pResourceEntry->hResourceHandle
            , LOG_ERROR
            , L"IsAlive sanity check failed! resid = 0x%1!08x!.\n"
            , residIn
            );
        return FALSE;
    } // if: invalid resource ID

#ifdef LOG_VERBOSE
    (g_pfnLogEvent)(
          pResourceEntry->hResourceHandle
        , LOG_INFORMATION
        , L"IsAlive request.\n"
        );
#endif

    //
    // Check to see if the resource is alive.
    //

    return ClipBookServerCheckIsAlive( pResourceEntry, TRUE /* fFullCheck */ );

} //** ClipBookServerIsAlive


/////////////////////////////////////////////////////////////////////////////
//++
//
//  ClipBookServerCheckIsAlive
//
//  Description:
//      Check to see if the resource is alive for ClipBook Server
//      resources.
//
//  Arguments:
//      pResourceEntryIn
//          Supplies the resource entry for the resource to polled.
//
//      fFullCheckIn
//          TRUE = Perform a full check.
//          FALSE = Perform a cursory check.
//
//  Return Value:
//      TRUE
//          The specified resource is online and functioning normally.
//
//      FALSE
//          The specified resource is not functioning normally.
//
//--
/////////////////////////////////////////////////////////////////////////////
BOOL
ClipBookServerCheckIsAlive(
      PCLIPBOOKSERVER_RESOURCE  pResourceEntryIn
    , BOOL                     fFullCheckIn
    )
{
    BOOL    fIsAlive = TRUE;
    DWORD   sc;

    //
    // Check to see if the resource is alive.
    //

    sc = ResUtilVerifyService( pResourceEntryIn->hService );
    if ( sc != ERROR_SUCCESS )
    {
        (g_pfnLogEvent)(
              pResourceEntryIn->hResourceHandle
            , LOG_ERROR
            , L"CheckIsAlive: Verification of the '%1!s!' service failed. Error: %2!u!.\n"
            , CLIPBOOKSERVER_SVCNAME
            , sc
            );
        fIsAlive = FALSE;
        goto Cleanup;
    } // if: error verifying service

    if ( fFullCheckIn )
    {
        //
        // TODO: Add code to perform a full check.
        //

    } // if: performing a full check

Cleanup:

    return fIsAlive;

} //*** ClipBookServerCheckIsAlive


/////////////////////////////////////////////////////////////////////////////
//++
//
//  ClipBookServerResourceControl
//
//  Description:
//      ResourceControl routine for ClipBook Server resources.
//
//      Perform the control request specified by nControlCode on the specified
//      resource.
//
//  Arguments:
//      residIn
//          Supplies the resource ID for the specific resource.
//
//      nControlCodeIn
//          Supplies the control code that defines the action to be performed.
//
//      pInBufferIn
//          Supplies a pointer to a buffer containing input data.
//
//      cbInBufferSizeIn
//          Supplies the size, in bytes, of the data pointed to by pInBufferIn.
//
//      pOutBufferOut
//          Supplies a pointer to the output buffer to be filled in.
//
//      cbOutBufferSizeIn
//          Supplies the size, in bytes, of the available space pointed to by
//          pOutBufferOut
//
//      pcbBytesReturnedOut
//          Returns the number of bytes of pOutBufferOut actually filled in by
//          the resource.  If pOutBufferOut is too small, pcbBytesReturnedOut
//          contains the total number of bytes for the operation to succeed.
//
//  Return Value:
//      ERROR_SUCCESS
//          The function completed successfully.
//
//      ERROR_RESOURCE_NOT_FOUND
//          Resource ID is not valid.
//
//      ERROR_MORE_DATA
//          The output buffer is too small to return the data.
//          pcbBytesReturnedOut contains the required size.
//
//      ERROR_INVALID_FUNCTION
//          The requested control code is not supported.  In some cases,
//          this allows the cluster software to perform the work.
//
//      Win32 error code
//          The function failed.
//
//--
/////////////////////////////////////////////////////////////////////////////
DWORD WINAPI
ClipBookServerResourceControl(
      RESID     residIn
    , DWORD     nControlCodeIn
    , PVOID     pInBufferIn
    , DWORD     cbInBufferSizeIn
    , PVOID     pOutBufferOut
    , DWORD     cbOutBufferSizeIn
    , LPDWORD   pcbBytesReturnedOut
    )
{
    DWORD               sc = ERROR_SUCCESS;
    PCLIPBOOKSERVER_RESOURCE pResourceEntry = NULL;

    UNREFERENCED_PARAMETER( cbInBufferSizeIn );
    //
    // Verify we have a valid resource ID.
    //
    pResourceEntry = static_cast< PCLIPBOOKSERVER_RESOURCE >( residIn );
    if ( pResourceEntry == NULL )
    {
        DBG_PRINT( "ClipBook Server: ResourceControl request for a nonexistent resource id.\n" );
        return ERROR_RESOURCE_NOT_FOUND;
    } // if: NULL resource ID

    if ( pResourceEntry->resid != residIn )
    {
        (g_pfnLogEvent)(
              pResourceEntry->hResourceHandle
            , LOG_ERROR
            , L"ResourceControl sanity check failed! resid = 0x%1!08x!.\n"
            , residIn
            );
        return ERROR_RESOURCE_NOT_FOUND;
    } // if: invalid resource ID

    switch ( nControlCodeIn )
    {
        case CLUSCTL_RESOURCE_UNKNOWN:
            *pcbBytesReturnedOut = 0;
            sc = ERROR_SUCCESS;
            break;

        case CLUSCTL_RESOURCE_SET_NAME:
            sc = ClipBookServerSetNameHandler( pResourceEntry, static_cast< LPWSTR >( pInBufferIn ) );
            break;

        case CLUSCTL_RESOURCE_GET_REQUIRED_DEPENDENCIES:
            sc = ClipBookServerGetRequiredDependencies( pOutBufferOut, cbOutBufferSizeIn, pcbBytesReturnedOut );
            break;

        case CLUSCTL_RESOURCE_GET_CHARACTERISTICS:
        case CLUSCTL_RESOURCE_GET_CLASS_INFO:
        case CLUSCTL_RESOURCE_STORAGE_GET_DISK_INFO:
        case CLUSCTL_RESOURCE_STORAGE_IS_PATH_VALID:
        case CLUSCTL_RESOURCE_DELETE:
        case CLUSCTL_RESOURCE_INSTALL_NODE:
        case CLUSCTL_RESOURCE_EVICT_NODE:
        case CLUSCTL_RESOURCE_ADD_DEPENDENCY:
        case CLUSCTL_RESOURCE_REMOVE_DEPENDENCY:
        case CLUSCTL_RESOURCE_ADD_OWNER:
        case CLUSCTL_RESOURCE_REMOVE_OWNER:
        case CLUSCTL_RESOURCE_CLUSTER_NAME_CHANGED:
        case CLUSCTL_RESOURCE_CLUSTER_VERSION_CHANGED:
        default:
            sc = ERROR_INVALID_FUNCTION;
            break;
    } // switch: nControlCodeIn

    return sc;

} //*** ClipBookServerResourceControl


/////////////////////////////////////////////////////////////////////////////
//++
//
//  ClipBookServerGetRequiredDependencies
//
//  Description:
//      Processes the CLUSCTL_RESOURCE_GET_REQUIRED_DEPENDENCIES control
//      function for resources of type ClipBook Server.
//
//  Arguments:
//      pOutBufferOut
//          Supplies a pointer to the output buffer to be filled in.
//
//      cbOutBufferSizeIn
//          Supplies the size, in bytes, of the available space pointed to by
//          pOutBuffer.
//
//      pcbBytesReturnedOut
//          Returns the number of bytes of pOutBufferOut actually filled in by
//          the resource.  If pOutBufferOut is too small, pcbBytesReturnedOut
//          contains the total number of bytes for the operation to succeed.
//
//  Return Value:
//      ERROR_SUCCESS
//          The function completed successfully.
//
//      ERROR_MORE_DATA
//          The output buffer is too small to return the data.
//          pcbBytesReturnedOut contains the required size.
//
//      Win32 error code
//          The function failed.
//
//--
/////////////////////////////////////////////////////////////////////////////
DWORD
ClipBookServerGetRequiredDependencies(
      PVOID     pOutBufferOut
    , DWORD     cbOutBufferSizeIn
    , LPDWORD   pcbBytesReturnedOut
    )
{
    //
    // TODO: Specify your resource's required dependencies here.
    // The default is that the resource requires a dependency on a
    // network name resource.
    //

    struct DEP_DATA
    {
        //
        // TODO: Uncomment the following line to enable a dependency on a storage device.
        //

        //CLUSPROP_RESOURCE_CLASS rcStorage;
        CLUSPROP_SZ_DECLARE( netnameEntry, RTL_NUMBER_OF( RESOURCE_TYPE_NETWORK_NAME ) );
        CLUSPROP_SYNTAX     endmark;
    };

    DEP_DATA *  pdepdata = static_cast< DEP_DATA * >( pOutBufferOut );
    DWORD       sc = ERROR_SUCCESS;
    HRESULT     hr = S_OK;

    *pcbBytesReturnedOut = sizeof( *pdepdata );
    if ( cbOutBufferSizeIn < sizeof( *pdepdata ) )
    {
        if ( pOutBufferOut == NULL )
        {
            sc = ERROR_SUCCESS;
        } // if: no buffer specified
        else
        {
            sc = ERROR_MORE_DATA;
        } // if: buffer specified
    } // if: output buffer is too small
    else
    {
        ZeroMemory( pdepdata, sizeof( *pdepdata ) );

        //
        // TODO: Uncomment the following lines to enable a dependency on a storage device.
        //

        //
        // Add the storage device entry.
        //

        //pdepdata->rcStorage.Syntax.dw = CLUSPROP_SYNTAX_RESCLASS;
        //pdepdata->rcStorage.cbLength = sizeof( pdepdata->rcStorage.rc );
        //pdepdata->rcStorage.rc = CLUS_RESCLASS_STORAGE;

        //
        // Add the netname entry.
        //
        pdepdata->netnameEntry.Syntax.dw = CLUSPROP_SYNTAX_NAME;
        pdepdata->netnameEntry.cbLength = sizeof( RESOURCE_TYPE_NETWORK_NAME );
        hr = StringCchCopyW(
                  pdepdata->netnameEntry.sz
                , RTL_NUMBER_OF( pdepdata->netnameEntry.sz )
                , RESOURCE_TYPE_NETWORK_NAME
                );
        if ( FAILED( hr ) )
        {
            sc = HRESULT_CODE( hr );
            goto Cleanup;
        } // if:

        //
        // Add the endmark.
        //

        pdepdata->endmark.dw = CLUSPROP_SYNTAX_ENDMARK;

        sc = ERROR_SUCCESS;
    } // else: output buffer is large enough

Cleanup:

    return sc;

} //*** ClipBookServerGetRequiredDependencies


/////////////////////////////////////////////////////////////////////////////
//++
//
//  ClipBookServerVerifyRequiredDependencies
//
//  Description:
//      Verifies that all of the required dependencies have been set.  Called
//      from OnlineThread.
//
//  Arguments:
//      pResourceEntryIn
//          Supplies the resource entry on which to operate.
//
//  Return Value:
//      ERROR_SUCCESS
//          All required dependencies have been met.
//
//      ERROR_DEPENDENCY_NOT_FOUND
//          A required dependency is missing.
//
//      Win32 error code
//          The function failed.
//
//--
/////////////////////////////////////////////////////////////////////////////
DWORD ClipBookServerVerifyRequiredDependencies(
    PCLIPBOOKSERVER_RESOURCE pResourceEntryIn
    )
{ 
    DWORD                   sc = ERROR_SUCCESS;
    DWORD                   cbBuffer = 0;
    PBYTE                   pbBuffer = NULL;
    CLUSPROP_BUFFER_HELPER  props;
    HRESOURCE               hResource = NULL;

    //
    // Get the list of required dependencies.
    //

    sc = ClipBookServerGetRequiredDependencies(
                  NULL
                , 0
                , &cbBuffer
                );
    if ( sc != ERROR_SUCCESS )
    {
        (g_pfnLogEvent)(
              pResourceEntryIn->hResourceHandle
            , LOG_ERROR
            , L"VerifyRequiredDependencies: Failed to retrieve list of required dependencies. Error: %1!u!.\n"
            , sc
            );
        goto Cleanup;
    } // if:

    pbBuffer = new BYTE[ cbBuffer ];
    if ( pbBuffer == NULL )
    {
        sc = ERROR_OUTOFMEMORY;
        (g_pfnLogEvent)(
              pResourceEntryIn->hResourceHandle
            , LOG_ERROR
            , L"VerifyRequiredDependencies: Failed to allocate memory. Error: %1!u!.\n"
            , sc
            );
        goto Cleanup;
    } // if:

    sc = ClipBookServerGetRequiredDependencies(
                  pbBuffer
                , cbBuffer
                , &cbBuffer
                );
    if ( sc != ERROR_SUCCESS )
    {
        (g_pfnLogEvent)(
              pResourceEntryIn->hResourceHandle
            , LOG_ERROR
            , L"VerifyRequiredDependencies: Failed to retrieve list of required dependencies. Error: %1!u!.\n"
            , sc
            );
        goto Cleanup;
    } // if:

    //
    // Assign the list.
    //

    props.pRequiredDependencyValue = (PCLUSPROP_REQUIRED_DEPENDENCY) pbBuffer;

    //
    // Loop through each required dependency and make sure
    // there is a dependency on a resource of that type.
    //

    while ( props.pSyntax->dw != CLUSPROP_SYNTAX_ENDMARK )
    {
        //
        // Check whether the dependency exists.
        //

        if ( props.pSyntax->dw == CLUSPROP_SYNTAX_RESCLASS )
        {
            //
            // We have a dependency on a class of resource, such as storage devices.
            // This allows us to depend on a storage resource type from any vendor
            // and not just on Physical Disk.
            //

            hResource = ResUtilGetResourceDependencyByClass(
                              pResourceEntryIn->hCluster    
                            , pResourceEntryIn->hResource
                            , props.pResourceClassInfoValue
                            , TRUE  // bRecurse
                            );
            if ( hResource == NULL )
            {
                sc = GetLastError();
                if ( sc == ERROR_NO_MORE_ITEMS )
                {
                    //
                    // If we failed with ERROR_NO_MORE_ITEMS then the resource enumeration
                    // completed without finding the dependency.  Change sc to reflect the
                    // actual meaning of the error.
                    //

                    sc = ERROR_DEPENDENCY_NOT_FOUND;
                } // if:

                if ( props.pResourceClassInfoValue->rc == CLUS_RESCLASS_STORAGE )
                {
                    //
                    // We require a storage class resource dependency.
                    //

                    (g_pfnLogEvent)(
                          pResourceEntryIn->hResourceHandle
                        , LOG_ERROR
                        , L"VerifyRequiredDependencies: Missing storage resource dependency. Error: %1!u!.\n"
                        , sc
                        );
                } // if: storage resclass dependency
                else
                {
                    //
                    // We require a custom-defined resource class dependency.
                    //

                    (g_pfnLogEvent)(
                          pResourceEntryIn->hResourceHandle
                        , LOG_ERROR
                        , L"VerifyRequiredDependencies: Missing resource class dependency. Class: %1!u!. Error: %2!u!.\n"
                        , props.pResourceClassInfoValue->rc
                        , sc
                        );
                } // else: custom resclass dependency
                goto Cleanup;
            } // if: hResource == NULL

            props.pb += sizeof( *props.pResourceClassValue );
        } // if: by resclass
        else if ( props.pSyntax->dw == CLUSPROP_SYNTAX_NAME )
        {
            //
            // We have a dependency on a particular resource type.
            //

            hResource = ResUtilGetResourceDependencyByName(
                              pResourceEntryIn->hCluster
                            , pResourceEntryIn->hResource
                            , props.pStringValue->sz
                            , TRUE  // bRecurse
                            );
            if ( hResource == NULL )
            {
                sc = GetLastError();
                if ( sc == ERROR_NO_MORE_ITEMS )
                {
                    //
                    // If we failed with ERROR_NO_MORE_ITEMS then the resource enumeration
                    // completed without finding the dependency.  Change sc to reflect the
                    // actual meaning of the error.
                    //

                    sc = ERROR_DEPENDENCY_NOT_FOUND;
                } // if:

                (g_pfnLogEvent)(
                      pResourceEntryIn->hResourceHandle
                    , LOG_ERROR
                    , L"VerifyRequiredDependencies: Missing a required depencency on '%1!s!'. Error: %2!u!.\n"
                    , props.pStringValue->sz
                    , sc
                    );
                goto Cleanup;
            } // if: hResource == NULL

            props.pb += sizeof( *props.pStringValue ) + ALIGN_CLUSPROP( props.pStringValue->cbLength );
        } // else if: by name
        else
        {
            //
            // If we got here then our list of required dependencies contains an invalid entry.
            // 
            //

            sc = ERROR_DEPENDENCY_NOT_FOUND;
            (g_pfnLogEvent)(
                  pResourceEntryIn->hResourceHandle
                , LOG_ERROR
                , L"VerifyRequiredDependencies: Invalid dependency type. Error: %1!u!.\n"
                , sc
                );
            goto Cleanup;
        } // else: unknown

        CloseClusterResource( hResource );
        hResource = NULL;

        //
        // Test to make sure we haven't gone past the end of the buffer.
        //

        if ( props.pb > (pbBuffer + cbBuffer) )
        {
            sc = ERROR_INVALID_DATA;
            (g_pfnLogEvent)(
                  pResourceEntryIn->hResourceHandle
                , LOG_ERROR
                , L"VerifyRequiredDependencies: Invalid property list detected. Error: %1!u!.\n"
                , sc
                );
            goto Cleanup;
        } // if: we've gone beyond the end of the buffer

    } // while: !endmark

    sc = ERROR_SUCCESS;

Cleanup:

    if ( hResource != NULL )
    {
        CloseClusterResource( hResource );
    } // if:

    delete [] pbBuffer;

    return sc;

} //*** ClipBookServerVerifyRequiredDependencies


/////////////////////////////////////////////////////////////////////////////
//++
//
//  ClipBookServerSetNameHandler
//
//  Description:
//      Handle the CLUSCTL_RESOURCE_SET_NAME control code by saving the new
//      name of the resource.
//
//  Arguments:
//      pResourceEntryIn
//          Supplies the resource entry on which to operate.
//
//      pwszNameIn
//          The new name of the resource.
//
//  Return Value:
//      ERROR_SUCCESS
//          The function completed successfully.
//
//      Win32 error code
//          The function failed.
//
//--
/////////////////////////////////////////////////////////////////////////////
DWORD
ClipBookServerSetNameHandler(
      PCLIPBOOKSERVER_RESOURCE   pResourceEntryIn
    , LPWSTR                    pwszNameIn
    )
{
    DWORD   sc = ERROR_SUCCESS;
    DWORD   cchNameBuffer = 0;
    HRESULT hr = S_OK;
    WCHAR * pwszTemp = NULL;

    //
    // Allocate a buffer for the new name.
    //

    cchNameBuffer = lstrlenW( pwszNameIn ) + 1;
    pwszTemp = new WCHAR[ cchNameBuffer ];
    if ( pwszTemp == NULL )
    {
        sc = GetLastError();
        (g_pfnLogEvent)(
              pResourceEntryIn->hResourceHandle
            , LOG_ERROR
            , L"Failed to allocate memory for the new resource name '%1!s!'. Error: %2!u!.\n"
            , pwszNameIn
            , sc
            );
        goto Cleanup;
    } // if: error allocating memory for the name.

    //
    // Copy the new name to the new buffer.
    //

    hr = StringCchCopyW( pwszTemp, cchNameBuffer, pwszNameIn );
    if ( FAILED( hr ) )
    {
        sc = HRESULT_CODE( hr );
        goto Cleanup;
    } // if:

    //
    // All went well, free the old buffer and assign the new one.
    //

    delete [] pResourceEntryIn->pwszResourceName;
    pResourceEntryIn->pwszResourceName = pwszTemp;
    pwszTemp = NULL;

Cleanup:

    delete [] pwszTemp;

    return sc;

} //*** ClipBookServerSetNameHandler


/////////////////////////////////////////////////////////////////////////////
//
// Define Function Table
//
/////////////////////////////////////////////////////////////////////////////

CLRES_V1_FUNCTION_TABLE(
    g_ClipBookServerFunctionTable,       // Name
    CLRES_VERSION_V1_00,                // Version
    ClipBookServer,                      // Prefix
    NULL,                               // Arbitrate
    NULL,                               // Release
    ClipBookServerResourceControl,       // ResControl
    NULL                                // ResTypeControl
    );
