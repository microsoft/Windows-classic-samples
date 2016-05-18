/////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2003 <company name>
//
//  Module Name:
//      File Share Sample.cpp
//
//  Description:
//      Resource DLL for File Share Sample (File Share Sample).
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
#include <lmerr.h>
#include <lmshare.h>
#include <LMAPIbuf.h>

//
// Type and constant definitions.
//

#define CLUSCTL_RESOURCE_FILESHARESAMPLE_CALL_ISALIVE   CLUSCTL_USER_CODE( 1, CLUS_OBJECT_RESOURCE )

// ADDPARAM: Add new properties here.
#define PROP_NAME__SHARENAME L"ShareName"
#define PROP_NAME__PATH L"Path"
#define PROP_NAME__REMARK L"Remark"
#define PROP_NAME__MAXUSERS L"MaxUsers"

#define PROP_MIN__MAXUSERS      (0)
#define PROP_MAX__MAXUSERS      (4294967295)
#define PROP_DEFAULT__MAXUSERS  (4294967295)

// ADDPARAM: Add new properties here.
typedef struct _FILESHARESAMPLE_PROPS
{
    PWSTR           pwszShareName;
    PWSTR           pwszPath;
    PWSTR           pwszRemark;
    DWORD           nMaxUsers;
} FILESHARESAMPLE_PROPS, * PFILESHARESAMPLE_PROPS;

typedef struct _FILESHARESAMPLE_RESOURCE
{
    RESID                   resid;          // For validation.
    FILESHARESAMPLE_PROPS   propsActive;    // The active props.  Used for program flow and control when the resource is online.
    FILESHARESAMPLE_PROPS   props;          // The props in cluster DB.  May differ from propsActive until OnlineThread reloads them as propsActive.
    HCLUSTER                hCluster;
    HRESOURCE               hResource;
    HKEY                    hkeyParameters;
    RESOURCE_HANDLE         hResourceHandle;
    LPWSTR                  pwszResourceName;
    LPWSTR                  pwszComputerName;   // The current node name.
    LPWSTR                  pwszExpandedPath;   // The expanded path (in case there were env. variables embedded).
    LPWSTR                  pwszUNCSharedPath;  // UNC path to our share - used to test for aliveness.
    CLUS_WORKER             cwWorkerThread;
    CLUSTER_RESOURCE_STATE  state;
    BOOL                    fIsAliveFailed;
} FILESHARESAMPLE_RESOURCE, * PFILESHARESAMPLE_RESOURCE;


///////////////////////////////////////////////////////////////////////////////
// Global data.
///////////////////////////////////////////////////////////////////////////////

//
//  Forward reference to our RESAPI function table.
//

extern CLRES_FUNCTION_TABLE g_FileShareSampleFunctionTable;


//
// File Share Sample resource read-write private properties.
//

RESUTIL_PROPERTY_ITEM
FileShareSampleResourcePrivateProperties[] =
{
    { PROP_NAME__SHARENAME, NULL, CLUSPROP_FORMAT_SZ, 0, 0, 0, RESUTIL_PROPITEM_REQUIRED, FIELD_OFFSET( FILESHARESAMPLE_PROPS, pwszShareName ) },
    { PROP_NAME__PATH, NULL, CLUSPROP_FORMAT_EXPAND_SZ, 0, 0, 0, RESUTIL_PROPITEM_REQUIRED, FIELD_OFFSET( FILESHARESAMPLE_PROPS, pwszPath ) },
    { PROP_NAME__REMARK, NULL, CLUSPROP_FORMAT_SZ, 0, 0, 0, 0, FIELD_OFFSET( FILESHARESAMPLE_PROPS, pwszRemark ) },
    { PROP_NAME__MAXUSERS, NULL, CLUSPROP_FORMAT_DWORD, PROP_DEFAULT__MAXUSERS, PROP_MIN__MAXUSERS, PROP_MAX__MAXUSERS, 0, FIELD_OFFSET( FILESHARESAMPLE_PROPS, nMaxUsers ) },
    { 0 }
};


//
// Function prototypes.
//

RESID WINAPI
FileShareSampleOpen(
      LPCWSTR           pwszResourceNameIn
    , HKEY              hkeyResourceKeyIn
    , RESOURCE_HANDLE   hResourceHandleIn
    );

void WINAPI
FileShareSampleClose(
    RESID residIn
    );

DWORD WINAPI
FileShareSampleOnline(
      RESID     residIn
    , PHANDLE   phEventHandleInout
    );

DWORD WINAPI
FileShareSampleOnlineThread(
      PCLUS_WORKER              pWorkerIn
    , PFILESHARESAMPLE_RESOURCE pResourceEntryIn
    );

DWORD WINAPI
FileShareSampleOffline(
    RESID residIn
    );

DWORD WINAPI
FileShareSampleOfflineThread(
      PCLUS_WORKER              pWorkerIn
    , PFILESHARESAMPLE_RESOURCE pResourceEntryIn
    );

void WINAPI
FileShareSampleTerminate(
    RESID residIn
    );

BOOL WINAPI
FileShareSampleLooksAlive(
    RESID residIn
    );

BOOL WINAPI
FileShareSampleIsAlive(
    RESID residIn
    );

BOOL
FileShareSampleCheckIsAlive(
      PFILESHARESAMPLE_RESOURCE pResourceEntryIn
    , BOOL                      fFullCheckIn
    );

DWORD WINAPI
FileShareSampleResourceControl(
      RESID     residIn
    , DWORD     nControlCodeIn
    , PVOID     pInBufferIn
    , DWORD     cbInBufferSizeIn
    , PVOID     pOutBufferOut
    , DWORD     cbOutBufferSizeIn
    , LPDWORD   pcbBytesReturnedOut
    );

DWORD WINAPI
FileShareSampleResourceTypeControl(
      LPCWSTR   pwszResourceTypeNameIn
    , DWORD     nControlCodeIn
    , PVOID     pInBufferIn
    , DWORD     cbInBufferSizeIn
    , PVOID     pOutBufferOut
    , DWORD     cbOutBufferSizeIn
    , LPDWORD   pcbBytesReturnedOut
    );

DWORD
FileShareSampleGetPrivateResProperties(
      PFILESHARESAMPLE_RESOURCE pResourceEntryIn
    , PVOID                     pOutBufferOut
    , DWORD                     cbOutBufferSizeIn
    , LPDWORD                   pcbBytesReturnedOut
    );

DWORD
FileShareSampleValidatePrivateResProperties(
      PFILESHARESAMPLE_RESOURCE pResourceEntryIn
    , const PVOID               pInBufferIn
    , DWORD                     cbInBufferSizeIn
    , PFILESHARESAMPLE_PROPS    pPropsOut
    );

DWORD
FileShareSampleSetPrivateResProperties(
      PFILESHARESAMPLE_RESOURCE pResourceEntryIn
    , const PVOID               pInBufferIn
    , DWORD                     cbInBufferSizeIn
    );

DWORD
FileShareSampleSetNameHandler(
      PFILESHARESAMPLE_RESOURCE pResourceEntryIn
    , LPWSTR                    pwszNameIn
    );


/////////////////////////////////////////////////////////////////////////////
//++
//
//  FileShareSampleDllMain
//
//  Description:
//      Main DLL entry point for the File Share Sample resource type.
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
FileShareSampleDllMain(
      HINSTANCE hDllHandleIn
    , DWORD     nReasonIn
    , LPVOID    ReservedIn
    )
{
    BOOL    fSuccess = TRUE;

    UNREFERENCED_PARAMETER( hDllHandleIn );
    UNREFERENCED_PARAMETER( ReservedIn );

    switch ( nReasonIn )
    {
        case DLL_PROCESS_ATTACH:
            break;

        case DLL_PROCESS_DETACH:
            break;

    } // switch: nReason

    return fSuccess;

} //*** FileShareSampleDllMain


/////////////////////////////////////////////////////////////////////////////
//++
//
//  FileShareSampleStartup
//
//  Description:
//      Startup the resource DLL for the File Share Sample resource type.
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
FileShareSampleStartup(
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
        *pFunctionTableOut = &g_FileShareSampleFunctionTable;
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

} //*** FileShareSampleStartup


/////////////////////////////////////////////////////////////////////////////
//++
//
//  FileShareSampleOpen
//
//  Description:
//      Open routine for File Share Sample resources.
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
//          to the FileShareSampleStartup routine.  This handle should never be
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
FileShareSampleOpen(
      LPCWSTR           pwszResourceNameIn
    , HKEY              hkeyResourceKeyIn
    , RESOURCE_HANDLE   hResourceHandleIn
    )
{
    DWORD                   sc = ERROR_SUCCESS;
    size_t                  cchBuffer = 0;
    DWORD                   cchComputerName = 0;
    RESID                   resid = NULL;
    HKEY                    hkeyParameters = NULL;
    PFILESHARESAMPLE_RESOURCE pResourceEntry = NULL;
    HRESULT                 hr = S_OK;

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

    pResourceEntry = new FILESHARESAMPLE_RESOURCE;
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
    // Startup for the resource.
    //

    //
    // Retrieve the computer name.
    //

    cchComputerName = 0;
    GetComputerNameEx( ComputerNamePhysicalDnsHostname, NULL, &cchComputerName );
    sc = GetLastError();
    if ( sc != ERROR_MORE_DATA )
    {
        //
        // Something went wrong, we should've gotten ERROR_MORE_DATA.
        //

        (g_pfnLogEvent)(
              hResourceHandleIn
            , LOG_ERROR
            , L"Open: failed to retrieve the computer name with error %1!u!.\n"
            , sc
            );
        goto Cleanup;
    } // if: GetComputerNameEx failed

    pResourceEntry->pwszComputerName = new WCHAR[ cchComputerName ];
    if ( pResourceEntry->pwszComputerName == NULL )
    {
        sc = ERROR_OUTOFMEMORY;
        (g_pfnLogEvent)(
              hResourceHandleIn
            , LOG_ERROR
            , L"Open: failed to allocate computer name buffer with error %1!u!.\n"
            , sc
            );
        goto Cleanup;
    } // if: buffer allocation failed

    if ( GetComputerNameEx(
                  ComputerNamePhysicalDnsHostname
                , pResourceEntry->pwszComputerName
                , &cchComputerName
                ) == 0 )
    {
        sc = GetLastError();
        (g_pfnLogEvent)(
              hResourceHandleIn
            , LOG_ERROR
            , L"Open: failed to retrieve the computer name with error %1!u!.\n"
            , sc
            );
        goto Cleanup;
    } // if: GetComputerNameEx failed

    resid = static_cast< RESID >( pResourceEntry );

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
            delete [] pResourceEntry->pwszComputerName;
            delete pResourceEntry;

        } // if: resource entry allocated
    } // if: something failed

    SetLastError( sc );

    return resid;

} //*** FileShareSampleOpen


/////////////////////////////////////////////////////////////////////////////
//++
//
//  FileShareSampleClose
//
//  Description:
//      Close routine for File Share Sample resources.
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
FileShareSampleClose(
    RESID residIn
    )
{
    PFILESHARESAMPLE_RESOURCE pResourceEntry = NULL;
    DWORD                   sc = ERROR_SUCCESS;

    //
    // Verify we have a valid resource ID.
    //

    pResourceEntry = static_cast< PFILESHARESAMPLE_RESOURCE >( residIn );
    if ( pResourceEntry == NULL )
    {
        DBG_PRINT( "FileShareSample: Close request for a nonexistent resource id.\n" );
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

    // ADDPARAM: Add new properties here.

    //
    // The ResUtil API's use LocalAlloc, so use LocalFree on the prop list members.
    //

    LocalFree( pResourceEntry->propsActive.pwszShareName );
    LocalFree( pResourceEntry->props.pwszShareName );
    LocalFree( pResourceEntry->propsActive.pwszPath );
    LocalFree( pResourceEntry->props.pwszPath );
    LocalFree( pResourceEntry->propsActive.pwszRemark );
    LocalFree( pResourceEntry->props.pwszRemark );
    LocalFree( pResourceEntry->pwszExpandedPath );

    //
    // Deallocate the resource entry.
    //

    delete [] pResourceEntry->pwszResourceName;
    delete [] pResourceEntry->pwszComputerName;
    delete [] pResourceEntry->pwszUNCSharedPath;

    delete pResourceEntry;

    sc = ERROR_SUCCESS;

Cleanup:

    SetLastError( sc );

    return;

} //*** FileShareSampleClose


/////////////////////////////////////////////////////////////////////////////
//++
//
//  FileShareSampleOnline
//
//  Description:
//      Online routine for File Share Sample resources.
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
FileShareSampleOnline(
      RESID     residIn
    , PHANDLE   phEventHandleOut
    )
{
    PFILESHARESAMPLE_RESOURCE pResourceEntry = NULL;
    DWORD                   sc = ERROR_SUCCESS;

    UNREFERENCED_PARAMETER( phEventHandleOut );

    //
    // Verify we have a valid resource ID.
    //

    pResourceEntry = static_cast< PFILESHARESAMPLE_RESOURCE >( residIn );
    if ( pResourceEntry == NULL )
    {
        DBG_PRINT( "File Share Sample: Online request for a nonexistent resource id.\n" );
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
                , reinterpret_cast< PWORKER_START_ROUTINE >( FileShareSampleOnlineThread )
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

} //*** FileShareSampleOnline


/////////////////////////////////////////////////////////////////////////////
//++
//
//  FileShareSampleOnlineThread
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
//          A pointer to the FILESHARESAMPLE_RESOURCE block for this resource.
//
//  Return Value:
//      ERROR_SUCCESS
//          The operation completed successfully.
//
//      Win32 error code
//          The operation failed.
//
//  Notes:
//      When using properties in this routine it is recommended that you
//      use the properties in propsActive of the FILESHARESAMPLE_RESOURCE struct
//      instead of the properties in props.  The primary reason you should
//      use propsActive is that the properties in props could be changed by
//      the SetPrivateResProperties() routine.  Using propsActive allows
//      the online state of the resource to be steady while still allowing
//      an administrator to change the stored value of the properties.
//
//--
/////////////////////////////////////////////////////////////////////////////
DWORD WINAPI
FileShareSampleOnlineThread(
      PCLUS_WORKER              pWorkerIn
    , PFILESHARESAMPLE_RESOURCE   pResourceEntryIn
    )
{
    RESOURCE_STATUS resourceStatus;
    DWORD           sc = ERROR_SUCCESS;
    size_t          cch = 0;
    LPWSTR          pwszNameOfPropInError = NULL;
    SHARE_INFO_2    shareInfo;
    HRESULT         hr = S_OK;

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
    // Read properties.
    //

    sc = ResUtilGetPropertiesToParameterBlock(
              pResourceEntryIn->hkeyParameters
            , FileShareSampleResourcePrivateProperties
            , reinterpret_cast< LPBYTE >( &pResourceEntryIn->propsActive )
            , TRUE // CheckForRequiredProperties
            , &pwszNameOfPropInError
            );
    if ( sc != ERROR_SUCCESS )
    {
        (g_pfnLogEvent)(
              pResourceEntryIn->hResourceHandle
            , LOG_ERROR
            , L"OnlineThread: Unable to read the '%1!s!' property. Error: %2!u!.\n"
            , (pwszNameOfPropInError == NULL ? L"" : pwszNameOfPropInError)
            , sc
            );
        goto Cleanup;
    } // if: error getting properties

    //
    // Start any services that we depend on.
    // The call to ClusWorkerCheckTerminate checks to see if this resource
    // has been told to stop the online process, in which case we should
    // not start the service.
    //

    if ( ClusWorkerCheckTerminate( pWorkerIn ) == TRUE )
    {
        goto Cleanup;
    } // if: terminating

    sc = ResUtilStartResourceService( FILESHARESAMPLE_SVCNAME, NULL );
    if ( sc == ERROR_SERVICE_ALREADY_RUNNING )
    {
        sc = ERROR_SUCCESS;
    } // if: service was already started
    else if ( sc != ERROR_SUCCESS )
    {
        (g_pfnLogEvent)(
              pResourceEntryIn->hResourceHandle
            , LOG_ERROR
            , L"OnlineThread: Failed to bring required service '%1!s!' online.  Error: %2!u!.\n"
            , FILESHARESAMPLE_SVCNAME
            , sc
            );
        goto Cleanup;
    } // else if: error starting the service

    //
    // TODO: Add code to start the resource.
    //

    if ( ClusWorkerCheckTerminate( pWorkerIn ) == TRUE )
    {
        goto Cleanup;
    } // if: terminating

    //
    // The path may contain environment variables so we need to expand them.
    //

    delete [] pResourceEntryIn->pwszUNCSharedPath;
    pResourceEntryIn->pwszUNCSharedPath = NULL;

    LocalFree( pResourceEntryIn->pwszExpandedPath );
    pResourceEntryIn->pwszExpandedPath = ResUtilExpandEnvironmentStrings( pResourceEntryIn->propsActive.pwszPath );
    if ( pResourceEntryIn->pwszExpandedPath == NULL )
    {
        sc = GetLastError();
        (g_pfnLogEvent)(
              pResourceEntryIn->hResourceHandle
            , LOG_ERROR
            , L"OnlineThread: Error %1!u! expanding path '%2!ws!' for share '%3!ws!'.\n"
            , sc
            , pResourceEntryIn->propsActive.pwszPath
            , pResourceEntryIn->propsActive.pwszShareName
            );
        goto Cleanup;
    } // if:

    //
    // Make sure the path doesn't end in a trailing backslash or it will fail to come online.
    // Accept paths such as E:\ though.
    //
    
    cch = wcslen( pResourceEntryIn->pwszExpandedPath );
    if ( ( cch > 3 ) && (pResourceEntryIn->pwszExpandedPath[ cch - 1 ] == L'\\' ) )
    {
        pResourceEntryIn->pwszExpandedPath[ cch - 1 ] = L'\0';
    } // if:

    //
    // Initialize the shareInfo struct and create the share.
    //

    ZeroMemory( &shareInfo, sizeof( shareInfo ) );
    shareInfo.shi2_netname =    pResourceEntryIn->propsActive.pwszShareName;
    shareInfo.shi2_type =       STYPE_DISKTREE;
    shareInfo.shi2_remark =     pResourceEntryIn->propsActive.pwszRemark;
    shareInfo.shi2_max_uses =   pResourceEntryIn->propsActive.nMaxUsers;
    shareInfo.shi2_path =       pResourceEntryIn->pwszExpandedPath;

    sc = NetShareAdd( NULL, 2, reinterpret_cast< PBYTE >( &shareInfo ), NULL );
    if ( sc != NERR_Success )
    {
        (g_pfnLogEvent)(
              pResourceEntryIn->hResourceHandle
            , LOG_ERROR
            , L"OnlineThread: Error %1!u! creating share '%2!ws!' for path '%3!ws!'.\n"
            , sc
            , pResourceEntryIn->propsActive.pwszShareName
            , pResourceEntryIn->pwszExpandedPath
            );
        goto Cleanup;
    } // if: error creating share

    //
    // Calculate the UNC path in order to test for aliveness.  e.g. \\node1\myshare
    //

    // Figure out how big of a buffer we need and allocate it.
    // The magic "8" comes from this: - \\x\y\*.* - excluding the x & y and adding the null.
    cch = 8 + wcslen( pResourceEntryIn->pwszComputerName ) + wcslen( pResourceEntryIn->propsActive.pwszShareName );
    pResourceEntryIn->pwszUNCSharedPath = new WCHAR[ cch ];
    if ( pResourceEntryIn->pwszUNCSharedPath == NULL )
    {
        sc = ERROR_OUTOFMEMORY;
        (g_pfnLogEvent)(
              pResourceEntryIn->hResourceHandle
            , LOG_ERROR
            , L"OnlineThread: Error %1!u! allocating memory for local path.\n"
            , sc
            );
        goto Cleanup;
    } // if:

    // Format the path into pwszUNCSharedPath.
    hr = StringCchPrintf(
              pResourceEntryIn->pwszUNCSharedPath
            , cch
            , L"\\\\%ws\\%ws\\*.*"
            , pResourceEntryIn->pwszComputerName
            , pResourceEntryIn->propsActive.pwszShareName
            );
    if ( FAILED( hr ) )
    {
        sc = SCODE( hr );
        (g_pfnLogEvent)(
              pResourceEntryIn->hResourceHandle
            , LOG_ERROR
            , L"OnlineThread: Error %1!u! formatting share path from computer name '%2!s!' and share name '%3!s!'.\n"
            , sc
            , pResourceEntryIn->pwszComputerName
            , pResourceEntryIn->propsActive.pwszShareName
            );
        goto Cleanup;
    } // if:
                
    sc = ERROR_SUCCESS;
    resourceStatus.ResourceState = ClusterResourceOnline;

Cleanup:

    if ( sc != ERROR_SUCCESS )
    {
        (g_pfnLogEvent)(
              pResourceEntryIn->hResourceHandle
            , LOG_ERROR
            , L"OnlineThread: Error %1!u! bringing resource online.\n"
            , sc
            );
    } // if: error occurred

    pResourceEntryIn->state = resourceStatus.ResourceState;
    g_pfnSetResourceStatus( pResourceEntryIn->hResourceHandle, &resourceStatus );

    return sc;

} //*** FileShareSampleOnlineThread


/////////////////////////////////////////////////////////////////////////////
//++
//
//  FileShareSampleOffline
//
//  Description:
//      Offline routine for File Share Sample resources.
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
//  Notes:
//      When using properties in this routine it is recommended that you
//      use the properties in propsActive of the FILESHARESAMPLE_RESOURCE struct
//      instead of the properties in props.  The primary reason you should
//      use propsActive is that the properties in props could be changed by
//      the SetPrivateResProperties() routine.  Using propsActive allows
//      the online state of the resource to be steady while still allowing
//      an administrator to change the stored value of the properties.
//
//--
/////////////////////////////////////////////////////////////////////////////
DWORD WINAPI
FileShareSampleOffline(
    RESID residIn
    )
{
    PFILESHARESAMPLE_RESOURCE pResourceEntry = NULL;
    DWORD                   sc = ERROR_SUCCESS;

    //
    // Verify we have a valid resource ID.
    //

    pResourceEntry = static_cast< PFILESHARESAMPLE_RESOURCE >( residIn );
    if ( pResourceEntry == NULL )
    {
        DBG_PRINT( "File Share Sample: Offline request for a nonexistent resource id.\n" );
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
                , reinterpret_cast< PWORKER_START_ROUTINE >( FileShareSampleOfflineThread )
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

} //*** FileShareSampleOffline


/////////////////////////////////////////////////////////////////////////////
//++
//
//  FileShareSampleOfflineThread
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
//          A pointer to the FILESHARESAMPLE_RESOURCE block for this resource.
//
//  Return Value:
//      ERROR_SUCCESS
//          The operation completed successfully.
//
//      Win32 error code
//          The operation failed.
//
//  Notes:
//      When using properties in this routine it is recommended that you
//      use the properties in propsActive of the FILESHARESAMPLE_RESOURCE struct
//      instead of the properties in props.  The primary reason you should
//      use propsActive is that the properties in props could be changed by
//      the SetPrivateResProperties() routine.  Using propsActive allows
//      the online state of the resource to be steady while still allowing
//      an administrator to change the stored value of the properties.
//
//--
/////////////////////////////////////////////////////////////////////////////
DWORD WINAPI
FileShareSampleOfflineThread(
      PCLUS_WORKER              pWorkerIn
    , PFILESHARESAMPLE_RESOURCE pResourceEntryIn
    )
{
    RESOURCE_STATUS     resourceStatus;
    DWORD               sc = ERROR_SUCCESS;

    ResUtilInitializeResourceStatus( &resourceStatus );

    resourceStatus.ResourceState = ClusterResourceFailed;
    resourceStatus.WaitHint = 0;
    resourceStatus.CheckPoint = 1;

    //
    // Take the resource offline.
    //

    //
    // The call to ClusWorkerCheckTerminate checks to see if this
    // resource has been terminated or not.
    //

    if ( ClusWorkerCheckTerminate( pWorkerIn ) == TRUE )
    {
        goto Cleanup;
    } // if: resource terminated

    sc = NetShareDel( NULL, pResourceEntryIn->propsActive.pwszShareName, 0 );
    if ( (sc != NERR_Success) && (sc != NERR_NetNameNotFound) )
    {
        (g_pfnLogEvent)(
              pResourceEntryIn->hResourceHandle
            , LOG_ERROR
            , L"OfflineThread: Error %1!u! taking share '%2!s!' offline.\n"
            , sc
            , pResourceEntryIn->propsActive.pwszShareName
            );
        goto Cleanup;
    } // if: error taking the resource offline

    sc = ERROR_SUCCESS;
    resourceStatus.ResourceState = ClusterResourceOffline;

Cleanup:

    pResourceEntryIn->state = resourceStatus.ResourceState;
    g_pfnSetResourceStatus( pResourceEntryIn->hResourceHandle, &resourceStatus );

    return sc;

} //*** FileShareSampleOfflineThread


/////////////////////////////////////////////////////////////////////////////
//++
//
//  FileShareSampleTerminate
//
//  Description:
//      Terminate routine for File Share Sample resources.
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
//  Notes:
//      When using properties in this routine it is recommended that you
//      use the properties in propsActive of the FILESHARESAMPLE_RESOURCE struct
//      instead of the properties in props.  The primary reason you should
//      use propsActive is that the properties in props could be changed by
//      the SetPrivateResProperties() routine.  Using propsActive allows
//      the online state of the resource to be steady while still allowing
//      an administrator to change the stored value of the properties.
//
//--
/////////////////////////////////////////////////////////////////////////////
void WINAPI
FileShareSampleTerminate(
    RESID residIn
    )
{
    DWORD                       sc = ERROR_SUCCESS;
    PFILESHARESAMPLE_RESOURCE   pResourceEntry = NULL;

    //
    // Verify we have a valid resource ID.
    //

    pResourceEntry = static_cast< PFILESHARESAMPLE_RESOURCE >( residIn );
    if ( pResourceEntry == NULL )
    {
        DBG_PRINT( "File Share Sample: Terminate request for a nonexistent resource id.\n" );
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

    //
    // Terminate the resource.
    //

    sc = NetShareDel( NULL, pResourceEntry->propsActive.pwszShareName, 0 );
    if ( sc != NERR_Success && sc != NERR_NetNameNotFound )
    {
        (g_pfnLogEvent)(
              pResourceEntry->hResourceHandle
            , LOG_ERROR
            , L"Terminate: Error %1!u! removing share '%2!ws!'.\n"
            , sc
            , pResourceEntry->propsActive.pwszShareName
            );
    } // if:  error taking the resource offline

    pResourceEntry->state = ClusterResourceFailed;

    return;

} //*** FileShareSampleTerminate


/////////////////////////////////////////////////////////////////////////////
//++
//
//  FileShareSampleLooksAlive
//
//  Description:
//      LooksAlive routine for File Share Sample resources.
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
//  Notes:
//      When using properties in this routine it is recommended that you
//      use the properties in propsActive of the FILESHARESAMPLE_RESOURCE struct
//      instead of the properties in props.  The primary reason you should
//      use propsActive is that the properties in props could be changed by
//      the SetPrivateResProperties() routine.  Using propsActive allows
//      the online state of the resource to be steady while still allowing
//      an administrator to change the stored value of the properties.
//
//--
/////////////////////////////////////////////////////////////////////////////
BOOL WINAPI
FileShareSampleLooksAlive(
    RESID residIn
    )
{
    PFILESHARESAMPLE_RESOURCE   pResourceEntry = NULL;
    BOOL                        fRet = FALSE;

    //
    // Verify we have a valid resource ID.
    //

    pResourceEntry = static_cast< PFILESHARESAMPLE_RESOURCE >( residIn );
    if ( pResourceEntry == NULL )
    {
        DBG_PRINT( "File Share Sample: LooksAlive request for a nonexistent resource id.\n" );
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

    if ( pResourceEntry->fIsAliveFailed == TRUE )
    {
        //
        // Somebody called ClusterResourceControl with our custom control code.
        // That had the same effect as calling IsAlive and the result was false,
        // so we can skip doing the LooksAlive check and return false.  This
        // will cause IsAlive to be called and we'll do another check just in
        // case we encountered a hiccup the first time.
        //

        fRet = FALSE;
    } // if:
    else
    {
        fRet = FileShareSampleCheckIsAlive( pResourceEntry, FALSE /* fFullCheck */ );
    } // else:

    return fRet;

} //*** FileShareSampleLooksAlive


/////////////////////////////////////////////////////////////////////////////
//++
//
//  FileShareSampleIsAlive
//
//  Description:
//      IsAlive routine for File Share Sample resources.
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
//  Notes:
//      When using properties in this routine it is recommended that you
//      use the properties in propsActive of the FILESHARESAMPLE_RESOURCE struct
//      instead of the properties in props.  The primary reason you should
//      use propsActive is that the properties in props could be changed by
//      the SetPrivateResProperties() routine.  Using propsActive allows
//      the online state of the resource to be steady while still allowing
//      an administrator to change the stored value of the properties.
//
//--
/////////////////////////////////////////////////////////////////////////////
BOOL WINAPI
FileShareSampleIsAlive(
    RESID residIn
    )
{
    PFILESHARESAMPLE_RESOURCE pResourceEntry = NULL;

    //
    // Verify we have a valid resource ID.
    //

    pResourceEntry = static_cast< PFILESHARESAMPLE_RESOURCE >( residIn );
    if ( pResourceEntry == NULL )
    {
        DBG_PRINT( "File Share Sample: IsAlive request for a nonexistent resource id.\n" );
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

    return FileShareSampleCheckIsAlive( pResourceEntry, TRUE /* fFullCheck */ );

} //** FileShareSampleIsAlive


/////////////////////////////////////////////////////////////////////////////
//++
//
//  FileShareSampleCheckIsAlive
//
//  Description:
//      Check to see if the resource is alive for File Share Sample
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
//  Notes:
//      When using properties in this routine it is recommended that you
//      use the properties in propsActive of the FILESHARESAMPLE_RESOURCE struct
//      instead of the properties in props.  The primary reason you should
//      use propsActive is that the properties in props could be changed by
//      the SetPrivateResProperties() routine.  Using propsActive allows
//      the online state of the resource to be steady while still allowing
//      an administrator to change the stored value of the properties.
//
//--
/////////////////////////////////////////////////////////////////////////////
BOOL
FileShareSampleCheckIsAlive(
      PFILESHARESAMPLE_RESOURCE pResourceEntryIn
    , BOOL                      fFullCheckIn
    )
{
    BOOL            fIsAlive = FALSE;
    DWORD           sc = ERROR_SUCCESS;
    SHARE_INFO_2 *  pshareInfo = NULL;
    HANDLE          hFile = NULL;
    WIN32_FIND_DATA fileData;

    ZeroMemory( &fileData, sizeof( fileData ) );

    //
    // Reset the variable.
    //

    pResourceEntryIn->fIsAliveFailed = FALSE;

    //
    // Check to see if the resource is alive.
    //

    sc = NetShareGetInfo(
              NULL
            , pResourceEntryIn->propsActive.pwszShareName
            , 2     // return a SHARE_INFO_2 structure
            , reinterpret_cast< LPBYTE * >( &pshareInfo )
            );
    if ( sc == NERR_NetNameNotFound )
    {
        (g_pfnLogEvent)(
              pResourceEntryIn->hResourceHandle
            , LOG_ERROR
            , L"CheckIsAlive: Error, share '%1!ws!' went away.\n"
            , pResourceEntryIn->propsActive.pwszShareName
            );
        goto Cleanup;
    } // if:  share name not found
    else if ( sc != NERR_Success )
    {
        (g_pfnLogEvent)(
              pResourceEntryIn->hResourceHandle
            , LOG_ERROR
            , L"CheckIsAlive: Error %1!u! checking for share '%2!ws!'.\n"
            , sc
            , pResourceEntryIn->propsActive.pwszShareName
            );
        goto Cleanup;
    } // else if: error getting share info

    if ( fFullCheckIn == TRUE )
    {
        //
        // Do a full check by trying to open a file on the share.
        //

        hFile = FindFirstFileW( pResourceEntryIn->pwszUNCSharedPath, &fileData );

        //
        // If we fail on the first attempt, try again.  There seems to be
        // a bug in the RDR where the first attempt to read a share after
        // it has been deleted and reinstated fails.
        //

        if ( hFile == INVALID_HANDLE_VALUE )
        {
            hFile = FindFirstFileW( pResourceEntryIn->pwszUNCSharedPath, &fileData );
        } // if:  error finding file

        //
        // If we succeeded in finding a file, or there were no files in
        // the path, then return success, otherwise we had a failure.
        //

        sc = GetLastError();

        if (    ( hFile == INVALID_HANDLE_VALUE )
             && ( sc    != ERROR_FILE_NOT_FOUND )
             && ( sc    != ERROR_ACCESS_DENIED )
           )
        {
            (g_pfnLogEvent)(
                  pResourceEntryIn->hResourceHandle
                , LOG_ERROR
                , L"CheckIsAlive: Share has gone offline!\n"
                );
        } // if:  error finding the file
        else
        {
            FindClose( hFile );
            sc = ERROR_SUCCESS;     // There are other non-failure status codes possible, reset to success.
        } // else:  found file
    } // if: performing a full check

Cleanup:

    NetApiBufferFree( reinterpret_cast< LPBYTE * >( pshareInfo ) );
    pshareInfo = NULL;

    if ( sc == ERROR_SUCCESS )
    {
        fIsAlive = TRUE;
    } // if:

    return fIsAlive;

} //*** FileShareSampleCheckIsAlive


/////////////////////////////////////////////////////////////////////////////
//++
//
//  FileShareSampleResourceControl
//
//  Description:
//      ResourceControl routine for File Share Sample resources.
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
FileShareSampleResourceControl(
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
    PFILESHARESAMPLE_RESOURCE pResourceEntry = NULL;

    //
    // Verify we have a valid resource ID.
    //
    pResourceEntry = static_cast< PFILESHARESAMPLE_RESOURCE >( residIn );
    if ( pResourceEntry == NULL )
    {
        DBG_PRINT( "File Share Sample: ResourceControl request for a nonexistent resource id.\n" );
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

        case CLUSCTL_RESOURCE_ENUM_PRIVATE_PROPERTIES:
        {
            DWORD cbRequired = 0;
            sc = ResUtilEnumProperties(
                      FileShareSampleResourcePrivateProperties
                    , static_cast< LPWSTR >( pOutBufferOut )
                    , cbOutBufferSizeIn
                    , pcbBytesReturnedOut
                    , &cbRequired
                    );
            if ( sc == ERROR_MORE_DATA )
            {
                *pcbBytesReturnedOut = cbRequired;
            } // if: output buffer is too small
            break;
        }

        case CLUSCTL_RESOURCE_GET_PRIVATE_PROPERTIES:
            sc = FileShareSampleGetPrivateResProperties(
                      pResourceEntry
                    , pOutBufferOut
                    , cbOutBufferSizeIn
                    , pcbBytesReturnedOut
                    );
            break;

        case CLUSCTL_RESOURCE_VALIDATE_PRIVATE_PROPERTIES:
            sc = FileShareSampleValidatePrivateResProperties(
                      pResourceEntry
                    , pInBufferIn
                    , cbInBufferSizeIn
                    , NULL
                    );
            break;

        case CLUSCTL_RESOURCE_SET_PRIVATE_PROPERTIES:
            sc = FileShareSampleSetPrivateResProperties(
                      pResourceEntry
                    , pInBufferIn
                    , cbInBufferSizeIn
                    );
            break;

        case CLUSCTL_RESOURCE_SET_NAME:
            sc = FileShareSampleSetNameHandler( pResourceEntry, static_cast< LPWSTR >( pInBufferIn ) );
            break;

        case CLUSCTL_RESOURCE_FILESHARESAMPLE_CALL_ISALIVE:

            //
            // This has the same effect as the periodic IsAlive check.
            //

            if ( pResourceEntry->state != ClusterResourceOnline )
            {
                sc = ERROR_CLUSTER_INVALID_REQUEST;
            } // if:
            else if ( FileShareSampleCheckIsAlive( pResourceEntry, TRUE ) == FALSE )
            {
                sc = ERROR_RESOURCE_FAILED;
                pResourceEntry->fIsAliveFailed = TRUE;
                (g_pfnLogEvent)(
                      pResourceEntry->hResourceHandle
                    , LOG_ERROR
                    , L"ResourceControl CALL_ISALIVE failed.\n"
                    );
            } // else if:
            else
            {
                sc = ERROR_SUCCESS;
                (g_pfnLogEvent)(
                      pResourceEntry->hResourceHandle
                    , LOG_INFORMATION
                    , L"ResourceControl CALL_ISALIVE succeeded.\n"
                    );
            } // else:

            break;

        case CLUSCTL_RESOURCE_GET_REQUIRED_DEPENDENCIES:
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

} //*** FileShareSampleResourceControl


/////////////////////////////////////////////////////////////////////////////
//++
//
//  FileShareSampleResourceTypeControl
//
//  Description:
//      ResourceTypeControl routine for File Share Sample resources.
//
//      Perform the control request specified by nControlCode.
//
//  Arguments:
//      pwszResourceTypeNameIn
//          Supplies the name of the resource type.
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
//          pOutBufferOut.
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
//          pcbBytesReturned contains the required size.
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
FileShareSampleResourceTypeControl(
      LPCWSTR   pwszResourceTypeNameIn
    , DWORD     nControlCodeIn
    , PVOID     pInBufferIn
    , DWORD     cbInBufferSizeIn
    , PVOID     pOutBufferOut
    , DWORD     cbOutBufferSizeIn
    , LPDWORD   pcbBytesReturnedOut
    )
{
    DWORD   sc = ERROR_SUCCESS;

    UNREFERENCED_PARAMETER( pwszResourceTypeNameIn );
    UNREFERENCED_PARAMETER( pInBufferIn );
    UNREFERENCED_PARAMETER( cbInBufferSizeIn );

    switch ( nControlCodeIn )
    {

        case CLUSCTL_RESOURCE_TYPE_UNKNOWN:
            *pcbBytesReturnedOut = 0;
            sc = ERROR_SUCCESS;
            break;

        case CLUSCTL_RESOURCE_TYPE_ENUM_PRIVATE_PROPERTIES:
        {
            DWORD cbRequired = 0;
            sc = ResUtilEnumProperties(
                      FileShareSampleResourcePrivateProperties
                    , static_cast< LPWSTR >( pOutBufferOut )
                    , cbOutBufferSizeIn
                    , pcbBytesReturnedOut
                    , &cbRequired
                    );
            if ( sc == ERROR_MORE_DATA )
            {
                *pcbBytesReturnedOut = cbRequired;
            } // if: output buffer is too small
            break;
        }

        case CLUSCTL_RESOURCE_TYPE_GET_REQUIRED_DEPENDENCIES:
        case CLUSCTL_RESOURCE_TYPE_GET_CHARACTERISTICS:
        case CLUSCTL_RESOURCE_TYPE_GET_CLASS_INFO:
        case CLUSCTL_RESOURCE_TYPE_STORAGE_GET_AVAILABLE_DISKS:
        case CLUSCTL_RESOURCE_TYPE_INSTALL_NODE:
        case CLUSCTL_RESOURCE_TYPE_EVICT_NODE:
        default:
            sc = ERROR_INVALID_FUNCTION;
            break;
    } // switch: nControlCode

    return sc;

} //*** FileShareSampleResourceTypeControl


/////////////////////////////////////////////////////////////////////////////
//++
//
//  FileShareSampleGetPrivateResProperties
//
//  Description:
//      Processes the CLUSCTL_RESOURCE_GET_PRIVATE_PROPERTIES control
//      function for resources of type File Share Sample.
//
//  Arguments:
//      pResourceEntryIn
//          Supplies the resource entry on which to operate.
//
//      pOutBufferOut
//          Supplies a pointer to the output buffer to be filled in.
//
//      cbOutBufferSizeIn
//          Supplies the size, in bytes, of the available space pointed to by
//          pOutBuffer.
//
//      pcbBytesReturnedOut
//          Returns the number of bytes of pOutBuffer actually filled in by
//          the resource.  If pOutBuffer is too small, pcbBytesReturnedOut
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
//      ERROR_INVALID_PARAMETER
//          The data is formatted incorrectly.
//
//      ERROR_NOT_ENOUGH_MEMORY
//          An error occurred allocating memory.
//
//      Win32 error code
//          The function failed.
//
//--
/////////////////////////////////////////////////////////////////////////////
DWORD
FileShareSampleGetPrivateResProperties(
      PFILESHARESAMPLE_RESOURCE   pResourceEntryIn
    , PVOID                     pOutBufferOut
    , DWORD                     cbOutBufferSizeIn
    , LPDWORD                   pcbBytesReturnedOut
    )
{
    DWORD   sc;
    DWORD   cbRequired = 0;

    sc = ResUtilGetAllProperties(
              pResourceEntryIn->hkeyParameters
            , FileShareSampleResourcePrivateProperties
            , pOutBufferOut
            , cbOutBufferSizeIn
            , pcbBytesReturnedOut
            , &cbRequired
            );
    if ( sc == ERROR_MORE_DATA )
    {
        *pcbBytesReturnedOut = cbRequired;
    } // if: output buffer is too small

    return sc;

} //*** FileShareSampleGetPrivateResProperties


/////////////////////////////////////////////////////////////////////////////
//++
//
//  FileShareSampleValidatePrivateResProperties
//
//  Description:
//      Processes the CLUSCTL_RESOURCE_VALIDATE_PRIVATE_PROPERTIES control
//      function for resources of type File Share Sample.
//
//  Arguments:
//      pResourceEntryIn
//          Supplies the resource entry on which to operate.
//
//      pInBufferIn
//          Supplies a pointer to a buffer containing input data.
//
//      cbOutBufferSizeIn
//          Supplies the size, in bytes, of the data pointed to by pInBufferIn.
//
//      pPropsOut
//          Supplies the parameter block to fill in (optional).
//
//  Return Value:
//      ERROR_SUCCESS
//          The function completed successfully.
//
//      ERROR_INVALID_PARAMETER
//          The data is formatted incorrectly.
//
//      ERROR_NOT_ENOUGH_MEMORY
//          An error occurred allocating memory.
//
//      Win32 error code
//          The function failed.
//
//--
/////////////////////////////////////////////////////////////////////////////
DWORD
FileShareSampleValidatePrivateResProperties(
      PFILESHARESAMPLE_RESOURCE   pResourceEntryIn
    , PVOID                     pInBufferIn
    , DWORD                     cbInBufferSizeIn
    , PFILESHARESAMPLE_PROPS      pPropsOut
    )
{
    DWORD                   sc = ERROR_SUCCESS;
    FILESHARESAMPLE_PROPS   propsCurrent;
    FILESHARESAMPLE_PROPS   propsNew;
    PFILESHARESAMPLE_PROPS  pLocalProps = NULL;
    LPWSTR                  pwszNameOfPropInError;
    BOOL                    fRetrievedProps = FALSE;

    //
    // Check if there is input data.
    //

    if (    (pInBufferIn == NULL)
        ||  (cbInBufferSizeIn < sizeof( DWORD )) )
    {
        sc = ERROR_INVALID_DATA;
        goto Cleanup;
    } // if: no input buffer or input buffer not big enough to contain property list

    //
    // Retrieve the current set of private properties from the
    // cluster database.
    //

    ZeroMemory( &propsCurrent, sizeof( propsCurrent ) );

    sc = ResUtilGetPropertiesToParameterBlock(
               pResourceEntryIn->hkeyParameters
             , FileShareSampleResourcePrivateProperties
             , reinterpret_cast< LPBYTE >( &propsCurrent )
             , FALSE /*CheckForRequiredProperties*/
             , &pwszNameOfPropInError
             );
    if ( sc != ERROR_SUCCESS )
    {
        (g_pfnLogEvent)(
              pResourceEntryIn->hResourceHandle
            , LOG_ERROR
            , L"Unable to read the '%1!s!' property. Error: %2!u!.\n"
            , (pwszNameOfPropInError == NULL ? L"" : pwszNameOfPropInError)
            , sc
            );
        goto Cleanup;
    } // if: error getting properties
    fRetrievedProps = TRUE;

    //
    // Duplicate the resource parameter block.
    //

    if ( pPropsOut == NULL )
    {
        pLocalProps = &propsNew;
    } // if: no parameter block passed in
    else
    {
        pLocalProps = pPropsOut;
    } // else: parameter block passed in

    ZeroMemory( pLocalProps, sizeof( *pLocalProps ) );

    sc = ResUtilDupParameterBlock(
              reinterpret_cast< LPBYTE >( pLocalProps )
            , reinterpret_cast< LPBYTE >( &propsCurrent )
            , FileShareSampleResourcePrivateProperties
            );
    if ( sc != ERROR_SUCCESS )
    {
        goto Cleanup;
    } // if: error duplicating the parameter block

    //
    // Parse and validate the properties.
    //

    sc = ResUtilVerifyPropertyTable(
              FileShareSampleResourcePrivateProperties
            , NULL
            , TRUE // AllowUnknownProperties
            , pInBufferIn
            , cbInBufferSizeIn
            , reinterpret_cast< LPBYTE >( pLocalProps )
            );
    if ( sc != ERROR_SUCCESS )
    {
        goto Cleanup;
    } // if: property list not validated successfully

    //
    // Validate the property values.
    //

    //
    // TODO: Code to validate interactions between properties goes here.
    //

Cleanup:

    //
    // Cleanup our parameter block.
    //

    if (    ( pLocalProps == &propsNew )
        ||  (   ( sc != ERROR_SUCCESS)
            &&  ( pLocalProps != NULL )
            )
       )
    {
        ResUtilFreeParameterBlock(
              reinterpret_cast< LPBYTE >( pLocalProps )
            , reinterpret_cast< LPBYTE >( &propsCurrent )
            , FileShareSampleResourcePrivateProperties
            );
    } // if: we duplicated the parameter block

    if ( fRetrievedProps )
    {
        ResUtilFreeParameterBlock(
              reinterpret_cast< LPBYTE >( &propsCurrent )
            , NULL
            , FileShareSampleResourcePrivateProperties
            );
    } // if: properties were retrieved

    return sc;

} // FileShareSampleValidatePrivateResProperties


/////////////////////////////////////////////////////////////////////////////
//++
//
//  FileShareSampleSetPrivateResProperties
//
//  Description:
//      Processes the CLUSCTL_RESOURCE_SET_PRIVATE_PROPERTIES control
//      function for resources of type File Share Sample.
//
//  Arguments:
//      pResourceEntryIn
//          Supplies the resource entry on which to operate.
//
//      pInBufferIn
//          Supplies a pointer to a buffer containing input data.
//
//      cbOutBufferSizeIn
//          Supplies the size, in bytes, of the data pointed to by pInBufferIn.
//
//  Return Value:
//      ERROR_SUCCESS
//          The function completed successfully.
//
//      ERROR_INVALID_PARAMETER
//          The data is formatted incorrectly.
//
//      ERROR_NOT_ENOUGH_MEMORY
//          An error occurred allocating memory.
//
//      Win32 error code
//          The function failed.
//
//--
/////////////////////////////////////////////////////////////////////////////
DWORD
FileShareSampleSetPrivateResProperties(
      PFILESHARESAMPLE_RESOURCE pResourceEntryIn
    , PVOID                     pInBufferIn
    , DWORD                     cbInBufferSizeIn
    )
{
    DWORD                   sc = ERROR_SUCCESS;
    FILESHARESAMPLE_PROPS   props;
    BOOL                    fAllowDynamicChanges = FALSE;
    PSHARE_INFO_2           psiOld = NULL;
    SHARE_INFO_2            siNew;
    DWORD                   nInvalidParam;

    //
    // Parse the properties so they can be validated together.
    // This routine does individual property validation.
    //

    sc = FileShareSampleValidatePrivateResProperties( pResourceEntryIn, pInBufferIn, cbInBufferSizeIn, &props );
    if ( sc != ERROR_SUCCESS )
    {
        goto Cleanup;
    } // if: error validating properties

    //
    // Only update the share if the name and the path didn't change.
    //

    if (   (    ( pResourceEntryIn->props.pwszShareName != NULL )
             && ( 0 != CompareStringW(
                                    LOCALE_SYSTEM_DEFAULT,
                                    NORM_IGNORECASE,
                                    props.pwszShareName,
                                    -1,
                                    pResourceEntryIn->props.pwszShareName,
                                    -1
                                    )
                )
           )
        || (    ( pResourceEntryIn->props.pwszPath != NULL )
             && ( 0 != CompareStringW(
                                    LOCALE_SYSTEM_DEFAULT,
                                    NORM_IGNORECASE,
                                    props.pwszPath,
                                    -1,
                                    pResourceEntryIn->props.pwszPath,
                                    -1
                                    )
                )
           )
       )
    {
        fAllowDynamicChanges = FALSE; 
    } // if: share or path changed
    else
    {
        fAllowDynamicChanges = TRUE;
    } // else: share and path

    //
    // Save the property values.
    //

    sc = ResUtilSetPropertyParameterBlock(
              pResourceEntryIn->hkeyParameters
            , FileShareSampleResourcePrivateProperties
            , NULL
            , reinterpret_cast< LPBYTE >( &props )
            , pInBufferIn
            , cbInBufferSizeIn
            , reinterpret_cast< LPBYTE >( &pResourceEntryIn->props )
            );

    ResUtilFreeParameterBlock(
              reinterpret_cast< LPBYTE >( &props )
            , reinterpret_cast< LPBYTE >( &pResourceEntryIn->props )
            , FileShareSampleResourcePrivateProperties
            );

    if ( sc != ERROR_SUCCESS )
    {
        goto Cleanup;
    } // if: properties not set successfully

    //
    // If the resource is online, return a non-success status.
    //

    if (    ( pResourceEntryIn->state == ClusterResourceOnline )
         && ( fAllowDynamicChanges == TRUE )
       )
    {
        //
        // Get the current share info.
        //

        sc = NetShareGetInfo(
                      NULL      // pwszServer
                    , pResourceEntryIn->props.pwszShareName
                    , 2         // level
                    , reinterpret_cast< LPBYTE * >( &psiOld )
                    );
        if ( sc != NERR_Success )
        {
            sc = ERROR_RESOURCE_PROPERTIES_STORED;
            goto Cleanup;
        } // if:

        //
        // Set new share info.
        //

        //
        // Note: NetShareSetInfo ignores all but shi2_remark and shi2_max_uses.
        //

        CopyMemory( &siNew, psiOld, sizeof( siNew ) );
        siNew.shi2_remark =     pResourceEntryIn->props.pwszRemark;
        siNew.shi2_max_uses =   pResourceEntryIn->props.nMaxUsers;

        sc = NetShareSetInfo(
                      NULL      // pwszServer
                    , pResourceEntryIn->props.pwszShareName
                    , 2         // level
                    , reinterpret_cast< LPBYTE >( &siNew )
                    , &nInvalidParam
                    );
        if ( sc != NERR_Success )
        {
            (g_pfnLogEvent)(
                  pResourceEntryIn->hResourceHandle
                , LOG_ERROR
                , L"SetPrivateProps: Error setting info on share '%1!ws!'. Error %2!u!, property # %3!d!.\n"
                , pResourceEntryIn->props.pwszShareName
                , sc
                , nInvalidParam
                );
            sc = ERROR_RESOURCE_PROPERTIES_STORED;
            goto Cleanup;
        } // if:  error setting new share info

        sc = ERROR_SUCCESS;

    } // if: resource is currently online
    else if (    ( pResourceEntryIn->state == ClusterResourceOnline )
              || ( pResourceEntryIn->state == ClusterResourceOnlinePending )
            )
    {
        sc = ERROR_RESOURCE_PROPERTIES_STORED;
    } // else if: resource is currently online or online pending
    else
    {
        sc = ERROR_SUCCESS;
    } // else: resource is in some other state

Cleanup:

    if ( psiOld != NULL )
    {
        NetApiBufferFree( psiOld );
    } // if:

    return sc;

} //*** FileShareSampleSetPrivateResProperties


/////////////////////////////////////////////////////////////////////////////
//++
//
//  FileShareSampleSetNameHandler
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
FileShareSampleSetNameHandler(
      PFILESHARESAMPLE_RESOURCE   pResourceEntryIn
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

} //*** FileShareSampleSetNameHandler


/////////////////////////////////////////////////////////////////////////////
//
// Define Function Table
//
/////////////////////////////////////////////////////////////////////////////

CLRES_V1_FUNCTION_TABLE(
    g_FileShareSampleFunctionTable,       // Name
    CLRES_VERSION_V1_00,                // Version
    FileShareSample,                      // Prefix
    NULL,                               // Arbitrate
    NULL,                               // Release
    FileShareSampleResourceControl,       // ResControl
    FileShareSampleResourceTypeControl    // ResTypeControl
    );
