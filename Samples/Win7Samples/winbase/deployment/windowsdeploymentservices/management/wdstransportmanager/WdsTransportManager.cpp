/*++

THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (c) Microsoft Corporation. All rights reserved

Module Name:

    WdsTransportManager.cpp

Abstract:

    Sample application to demonstrate usage of the WDS Transport 
    Management API.

Note:

    This sample is primarily intended to illustrate usage of the WDS Transport
    Management API, sometimes referred to as WdsTptMgmt.
    For the sake of clarity and brevity, the sample does not adhere to all the
    coding guidelines expected of a modern, well-designed application. Some of
    the coding aspects that are omitted from this program are:
    1. Globalization. The code uses hardcoded English strings rather than
       placing strings in a resource file.
    2. Error Handling: 
       a. The sample provided here simply prints some of its own error messages
       and shows raw HRESULTS in case of most failures. A developer working
       with this sample can manually look up WdsTptMgmt HRESULTs in 
       WdsTptMgmtMsg.h and WdsMcErr.h.
       b. In general, an application is expected to use functions like
       FormatMessage to get a user-friendly message that corresponds to
       each error code. In the case of the WdsTptMgmt API, all error codes
       have user-friendly messages that can be looked up in WdsTptMgmt.dll
       and WdsMcErr.dll using FormatMessage.
       c. If the application needs to support Windows versions prior to
       Windows Vista, the function LoadMUILibrary should be used to get
       a handle to the WDS Transport modules above for the purpose of error
       lookup.

--*/

#include "stdafx.h"
#include "WdsTransportManager.h"



int __cdecl
wmain(
    __in int argc,
    __in_ecount(argc) wchar_t* argv[ ]
    )
/*++

Routine Description:

    This is the main application entry point.

Arguments:

    argc - Number of arguments passed to the application.
    argv - Array of null-terminated arguments passed to the application.

Return Value:

    0 - Success
    Others - Failure HRESULT.

--*/
{
    HRESULT hr = S_OK;
    WDSTPT_ARGS Arguments = { 0 };
    DWORD dwInstalledFeatures = 0;

    IWdsTransportManager* pWdsTransportManager = NULL;
    IWdsTransportServer* pServer = NULL;
    IWdsTransportSetupManager* pSetupMgr = NULL;
    CLSID clsid;


    //
    // Parse arguments.
    //
    hr = ParseArguments( argc,
                         argv,
                         &Arguments );
    CLEANUP_HR( hr, Cleanup );


    //
    // Initialize COM.
    //
    hr = CoInitializeEx( NULL,
                         COINIT_APARTMENTTHREADED );
    CLEANUP_HR( hr, Cleanup );

    //
    // Instantiate the top-level WdsTransportManager object.
    //
    hr = CLSIDFromProgID( L"WdsTptMgmt.WdsTransportManager", 
                          &clsid );
    CLEANUP_HR( hr, Cleanup );
    
    hr = CoCreateInstance( clsid,
                           NULL,
                           CLSCTX_INPROC_SERVER,
                           __uuidof( IWdsTransportManager ),
                           (PVOID*) &pWdsTransportManager );
    CLEANUP_HR( hr, Cleanup );


    //
    // Get an object for the target server
    //
    hr = pWdsTransportManager->GetWdsTransportServer( Arguments.bszServer,
                                                      &pServer );
    CLEANUP_HR( hr, Cleanup );

    //
    // Confirm that the server is a WDS Transport Server.
    // This is accomplished by querying the server's SetupManager.
    //
    hr = pServer->get_SetupManager( &pSetupMgr );
    CLEANUP_HR( hr, Cleanup );

    //
    // Check if the server is actually a WDS Transport server by seeing if
    // the Transport Server feature is installed on the server.
    //
    hr = pSetupMgr->get_InstalledFeatures( &dwInstalledFeatures );
    CLEANUP_HR( hr, Cleanup );


    if ( !( dwInstalledFeatures & WdsTptFeatureTransportServer ) )
    {
        //
        // Print a message indicating the server isn't a WDS transport server.
        // Set return value to the special WdsTptMgmt value that would have
        // been returned had we attempted other operations on the server.
        //
        wprintf( WDSTPTMGR_SERVER_NOT_CONFIGURED );
        hr = WDSTPTMGMT_E_TRANSPORT_SERVER_ROLE_NOT_CONFIGURED;
        goto Cleanup;
    }


    //
    // Dispatch to appropriate handler function based on parsed action type.
    //
    switch ( Arguments.Action )
    {
    case WdsTptEnableServer : 

        hr = EnableServer( pServer );
        break;

    case WdsTptDisableServer : 

        hr = DisableServer( pServer );
        break;
    
    case WdsTptAddNamespace :

        hr = AddNamespace( pServer,
                           &Arguments );
        break;

    case WdsTptRemoveNamespace :

        hr = RemoveNamespace( pServer,
                              &Arguments );
        break;

    default :

        //
        // This branch should never be reached unless the parser was updated
        // to recognize more actions than this dispatcher supports.
        //
        hr = E_NOTIMPL;
        wprintf( WDSTPTMGR_UNKNOWN_ACTION,
                 argv[ 0 ],
                 Arguments.pwszAction,
                 Arguments.Action );

        break;
    }


Cleanup:


    //
    // Print an overall result message.
    //
    if ( SUCCEEDED( hr ) )
    {
        wprintf( WDSTPTMGR_SUCCESS );
    }
    else
    {
        wprintf( WDSTPTMGR_FAILURE, hr );
    }
    

    //
    // Release/Free resources
    //
    RELEASE( pWdsTransportManager );
    RELEASE( pServer );
    RELEASE( pSetupMgr );
    BSTR_FREE( Arguments.bszServer );
    BSTR_FREE( Arguments.bszNamespace );
    BSTR_FREE( Arguments.bszWdsContentProvider );
    BSTR_FREE( Arguments.bszFolderPath );


    //
    // Uninitialize COM.
    //
    CoUninitialize( );

    //
    // Return the current HRESULT as the program return
    //
    return (int) hr;
}


//
// Utility Functions
//

HRESULT
AllocBstr(
    __in_opt PWSTR pwszString,
    __deref_out BSTR* pbszString
    )
/*++

Routine Description:

    HRESULT-based version of SysAllocString that returns an HR status
    rather than the string.

    Note that if pwszString is NULL, this function returns an empty
    BSTR.

Arguments:

    pwszString - PWSTR string to be converted to a BSTR
    pbszString - Recevies a BSTR version of the input string. 
                 Use SysFreeString( *pbszString ) when done.

Return Value:

    S_OK - Success
    E_OUTOFMEMORY - Not enough memory to perform the allocation.

--*/
{
    HRESULT hr = S_OK;

    //
    // Validate parameters.
    //
    assert( pbszString );

    //
    // Allocate a BSTR using the specified PWSTR as input.
    // If the input string is NULL, use an empty string.
    //
    *pbszString = SysAllocString( ( NULL == pwszString ) ? L"" : pwszString );

    if ( NULL == *pbszString )
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}


HRESULT
ParseArguments( 
    __in int argc,
    __in_ecount(argc) wchar_t *argv[ ],
    __out PWDSTPT_ARGS pArguments
    )
/*++

Routine Description:

    Parses the arguments passed on from the application main function.
    The results are returned in the output variable, pArguments.

Arguments:

    argc - Number of arguments passed to the application.
    argv - Array of null-terminated arguments passed to the application.
    pArguments - Recevies parsed and constructed arguments.

Return Value:

    S_OK - Success
    Others - Failure

Note:

    If the function succeeds, the caller should remember to free
    the various BSTR fields inside the pArguments structure.

--*/
{
    HRESULT hr = S_OK;

    //
    // Validate parameters
    //
    assert( argc > 0 );
    assert( argv );
    assert( pArguments );

    ZeroMemory( pArguments, sizeof( WDSTPT_ARGS ) );

    //
    // Verify that the correct number of arguments was passed
    // At this point we'll ensure we have at least 3 arguments
    // (application, server name, and action).
    //
    if ( argc < 3 )
    {
        hr = E_INVALIDARG;
        goto Cleanup;
    }

    //
    // Read server name and convert it to a BSTR.
    //
    hr = AllocBstr( argv[ 1 ],
                    &pArguments->bszServer );
    CLEANUP_HR( hr, Cleanup );

    //
    // Read action.
    //
    pArguments->pwszAction = argv[ 2 ];

    //
    // Map the action to one of the supported types and parse the remaining
    // arguments accordingly
    //
    if ( 0 == _wcsicmp( pArguments->pwszAction, L"EnableServer" ) )
    {
        pArguments->Action = WdsTptEnableServer;
    }
    else if ( 0 == _wcsicmp( pArguments->pwszAction, L"DisableServer" ) )
    {
        pArguments->Action = WdsTptDisableServer;
    }
    else if ( 0 == _wcsicmp( pArguments->pwszAction, L"AddNamespace" ) )
    {
        pArguments->Action = WdsTptAddNamespace;
        
        //
        // This action requires a total of 6 arguments.
        // Check for and read the remaining 3.
        //
        if ( argc < 6 )
        {
            hr = E_INVALIDARG;
            goto Cleanup;
        }

        //
        // Read namespace name and convert it to a BSTR.
        //
        hr = AllocBstr( argv[ 3 ],
                        &pArguments->bszNamespace );
        CLEANUP_HR( hr, Cleanup );

        //
        // Construct a BSTR for the WDS content provider.
        // This provider is hardcoded here because it is guaranteed to be
        // installed by default on a WDS transport server.
        //
        hr = AllocBstr( WDSTPTMGR_WDS_CONTENT_PROVIDER,
                        &pArguments->bszWdsContentProvider );
        CLEANUP_HR( hr, Cleanup );

        //
        // Read folder path and convert it to a BSTR.
        //
        hr = AllocBstr( argv[ 4 ],
                        &pArguments->bszFolderPath );
        CLEANUP_HR( hr, Cleanup );

        //
        // Read minimum clients as a ULONG.
        //
        // NOTE: Validation should be done here but is omitted for brevity.
        //
        pArguments->ulMinClients = (ULONG) _wtol( argv[ 5 ] );
    }
    else if ( 0 == _wcsicmp( pArguments->pwszAction, L"RemoveNamespace" ) )
    {
        pArguments->Action = WdsTptRemoveNamespace;
        
        //
        // This action requires a total of 4 arguments.
        // Check for and read the remaining one.
        //
        if ( argc < 4 )
        {
            hr = E_INVALIDARG;
            goto Cleanup;
        }

        //
        // Read namespace name and convert it to a BSTR.
        //
        hr = AllocBstr( argv[ 3 ],
                        &pArguments->bszNamespace );
        CLEANUP_HR( hr, Cleanup );
    }
    else
    {
        //
        // Invalid action specified
        //
        hr = E_INVALIDARG;
        goto Cleanup;
    }


Cleanup:

    //
    // If parsing failed, free any allocated resources and print
    // a usage message.
    //
    if ( FAILED( hr ) )
    {
        BSTR_FREE( pArguments->bszServer );
        BSTR_FREE( pArguments->bszNamespace );
        BSTR_FREE( pArguments->bszWdsContentProvider );
        BSTR_FREE( pArguments->bszFolderPath );
        wprintf( WDSTPTMGR_USAGE, argv[0] );
    }

    return hr;
}


HRESULT
EnableServer( 
    __in IWdsTransportServer* pServer
    )
/*++

Routine Description:

    Enables the WDS Transport Server for operation.
    This is done by enabling and starting the WDS transport services.
    Note that if the services are already enabled/running, the
    WdsTptMgmt API methods succeed and so this function does as well.

Arguments:

    pServer - Pointer to the server to be enabled.

Return Value:

    S_OK - Success
    Others - Failure

--*/
{
    HRESULT hr = S_OK;
    IWdsTransportConfigurationManager* pConfigMgr = NULL;

    assert( pServer );

    //
    // Get a reference to the server's Configuration Manager.
    //
    hr = pServer->get_ConfigurationManager( &pConfigMgr );
    CLEANUP_HR( hr, Cleanup );

    //
    // Enable WDS Tranport Services.
    //
    hr = pConfigMgr->EnableWdsTransportServices( );
    CLEANUP_HR( hr, Cleanup );

    //
    // Start WDS Tranport Services.
    //
    hr = pConfigMgr->StartWdsTransportServices( );
    CLEANUP_HR( hr, Cleanup );


Cleanup:

    RELEASE( pConfigMgr );

    return hr;
}


HRESULT
DisableServer( 
    __in IWdsTransportServer* pServer
    )
/*++

Routine Description:

    Disables the WDS Transport Server.
    This is done by stopping and disabling the WDS transport services.
    Note that if the services are already stopped/disabled, the
    WdsTptMgmt API methods succeed and so this function does as well.

Arguments:

    pServer - Pointer to the server to be disabled.

Return Value:

    S_OK - Success
    Others - Failure

--*/
{
    HRESULT hr = S_OK;
    IWdsTransportConfigurationManager* pConfigMgr = NULL;

    assert( pServer );

    //
    // Get a reference to the server's Configuration Manager.
    //
    hr = pServer->get_ConfigurationManager( &pConfigMgr );
    CLEANUP_HR( hr, Cleanup );

    //
    // Stop WDS Tranport Services.
    //
    hr = pConfigMgr->StopWdsTransportServices( );
    CLEANUP_HR( hr, Cleanup );

    //
    // Disable WDS Tranport Services.
    //
    hr = pConfigMgr->DisableWdsTransportServices( );
    CLEANUP_HR( hr, Cleanup );


Cleanup:

    RELEASE( pConfigMgr );

    return hr;
}


HRESULT
AddNamespace( 
    __in IWdsTransportServer* pServer,
    __in PWDSTPT_ARGS pArguments
    )
/*++

Routine Description:

    Adds the specified namespace to the specified WDS Transport Server.
    Note that there are vaiours WDS transport namespace types but this
    function only demonstrates the creation of a namespace of type
    WdsTransportNamespaceScheduledCastAutoStart. Other types can be
    created in a very similar manner.

Arguments:

    pServer - Pointer to the server to be modified.
    pArguments - Pointer to a structure with parsed arguments.

Return Value:

    S_OK - Success
    Others - Failure

--*/
{
    HRESULT hr = S_OK;
    IWdsTransportNamespaceManager* pNamespaceMgr = NULL;
    IWdsTransportNamespace* pNamespace = NULL;
    IWdsTransportNamespaceScheduledCastAutoStart* pNamespaceAutoStart = NULL;

    assert( pServer );
    assert( pArguments );

    //
    // Get a reference to the server's Namespace Manager.
    //
    hr = pServer->get_NamespaceManager( &pNamespaceMgr );
    CLEANUP_HR( hr, Cleanup );

    //
    // Create an auto-start namespace object using specified arguments.
    //
    hr = pNamespaceMgr->CreateNamespace( WdsTptNamespaceTypeScheduledCastAutoStart,
                                         pArguments->bszNamespace, 
                                         pArguments->bszWdsContentProvider, 
                                         pArguments->bszFolderPath, 
                                         &pNamespace );
    CLEANUP_HR( hr, Cleanup );

    //
    // Get a type-specific reference so we can set properties unique to
    // auto-start namespaces.
    //
    hr = pNamespace->QueryInterface( __uuidof(IWdsTransportNamespaceScheduledCastAutoStart),
                                     (PVOID*) &pNamespaceAutoStart );
    CLEANUP_HR( hr, Cleanup );

    //
    // Set the MinimumClients property to the user-specified value.
    //
    hr = pNamespaceAutoStart->put_MinimumClients( pArguments->ulMinClients );
    CLEANUP_HR( hr, Cleanup );

    //
    // Register the namespace on the server.
    //
    // NOTE: This may fail with an appropriate HRESULT for various reasons.
    //       E.g. If the namespace name is already in use by another
    //       regtistered namespace on the server, this method may return a
    //       "duplicate namespace" error code from WdsTptMgmt.h or WdsMcErr.h.
    //       This sample simply bubbles up the error code, but a real-world 
    //       application may want to handle such an error, e.g. by looking up
    //       and displaying its associated user-friendly message and/or
    //       prompting the user to try a different namespace name.
    //
    hr = pNamespaceAutoStart->Register( );
    CLEANUP_HR( hr, Cleanup );


Cleanup:

    //
    // Release all references
    //
    RELEASE( pNamespaceMgr );
    RELEASE( pNamespace );
    RELEASE( pNamespaceAutoStart );

    return hr;
}


HRESULT
RemoveNamespace( 
    __in IWdsTransportServer* pServer,
    __in PWDSTPT_ARGS pArguments
    )
/*++

Routine Description:

    Removes the specified namespace from the specified WDS Transport Server.

Arguments:

    pServer - Pointer to the server to be modified.
    pArguments - Pointer to a structure with parsed arguments.

Return Value:

    S_OK - Success
    Others - Failure

--*/
{
    HRESULT hr = S_OK;
    IWdsTransportNamespaceManager* pNamespaceMgr = NULL;
    IWdsTransportNamespace* pNamespace = NULL;

    assert( pServer );
    assert( pArguments );

    //
    // Get a reference to the server's Namespace Manager.
    //
    hr = pServer->get_NamespaceManager( &pNamespaceMgr );
    CLEANUP_HR( hr, Cleanup );

    //
    // Retrieve the namespace object corresponding to the specified namespace.
    //
    // NOTE: If this fails indicating no such namespace is registered on
    //       the server, this sample simply bubbles up the error code. 
    //       In a real-world scenario, the application may look up the error
    //       message in the WDS transport modules, display its own
    //       user-friendly message, prompt the user to re-enter the name,
    //       and so on.
    //
    hr = pNamespaceMgr->RetrieveNamespace( pArguments->bszNamespace, 
                                           &pNamespace );
    CLEANUP_HR( hr, Cleanup );

    //
    // Deregister the namespace on the server.
    //
    // NOTE: This sample utilizes the deregistration option that immediately
    //       terminates any active transport sessions under the namespace.
    //       In a real-world scenario, the application may give the user
    //       the option to deregister the namespace but allow existing
    //       sessions to run to completion.
    //
    hr = pNamespace->Deregister( VARIANT_TRUE );
    CLEANUP_HR( hr, Cleanup );


Cleanup:

    //
    // Release all references
    //
    RELEASE( pNamespaceMgr );
    RELEASE( pNamespace );

    return hr;
}
