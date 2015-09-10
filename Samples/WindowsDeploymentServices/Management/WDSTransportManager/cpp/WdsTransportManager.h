/*++

THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (c) Microsoft Corporation. All rights reserved

Module Name:

    WdsTransportManager.h

Abstract:

    Include file for global app definitions, utility macros, and similar items.

--*/


//
// Constants
//
#define WDSTPTMGR_WDS_CONTENT_PROVIDER          L"WDS"


//
// Types
//

enum WdsTptAction 
{
    WdsTptEnableServer      = 0,
    WdsTptDisableServer     = 1,
    WdsTptAddNamespace      = 2,
    WdsTptRemoveNamespace   = 3
};

typedef struct 
{
    BSTR bszServer;
    PWSTR pwszAction;
    WdsTptAction Action;

    BSTR bszNamespace;
    BSTR bszWdsContentProvider;
    BSTR bszFolderPath;
    ULONG ulMinClients;

} WDSTPT_ARGS, *PWDSTPT_ARGS;


//
// Messages
//

#define WDSTPTMGR_USAGE                                                       \
    L"Usage:\n"                                                               \
    L"======\n"                                                               \
    L"%s <WDS Server> <Action> [ <Namespace Name> <Folder Path>\n"            \
    L"       <Minimum Clients> ]\n"                                           \
    L"\n"                                                                     \
    L"       where:\n"                                                        \
    L"\n"                                                                     \
    L"       <WDS Server>: Name of a WDS Transport Server\n"                  \
    L"       <Action>: One of the values EnableServer, DisableServer,\n"      \
    L"                 AddNamespace, RemoveNamespace\n"                       \
    L"       <Namespace Name>: The name of the namespace to add/remove.\n"    \
    L"       <Folder Path>: Full path to the folder to be offered for\n"      \
    L"                      download in a namespace that is being added.\n"   \
    L"       <Minimum Clients>: The minimum number of clients to auto-start\n"\
    L"                          a namespace that is being added.\n"           \
    L"\n"

#define WDSTPTMGR_SUCCESS \
    L"\n\nSuccess!\n"

#define WDSTPTMGR_FAILURE \
    L"\n\nFailed to perform operation.\nHRESULT = 0x%lX.\n"

#define WDSTPTMGR_SERVER_NOT_CONFIGURED                                     \
    L"The Windows Deployment Services Transport Server role has not been "  \
    L"configured on the server.\n"

#define WDSTPTMGR_UNKNOWN_ACTION                                                \
    L"%s: The requested action, %s (%u) is unknown to the dispatch routine.\n"  \
    L"Please report this error to your application developer.\n"


//
// Utility Macros
//

#define CLEANUP_HR( hr, CleanupLabel )  \
    if ( FAILED( hr ) )                 \
    {                                   \
        goto CleanupLabel;              \
    }

#define RELEASE( p )        \
    if ( ( p ) != NULL )    \
    {                       \
        ( p )->Release( );  \
        ( p )= NULL;        \
    }

#define BSTR_FREE( p )      \
    if ( ( p ) != NULL )    \
    {                       \
        SysFreeString( p ); \
        ( p ) = NULL;       \
    }


//
// Utility Functions
//

HRESULT
AllocBstr(
    _In_opt_ PWSTR pwszString,
    _COM_Outptr_ BSTR* pbszString
    );

_At_(pArguments->bszServer, _Post_notnull_)
HRESULT
ParseArguments( 
    _In_ int argc,
    _In_reads_(argc) wchar_t* argv[ ],
    _Out_ PWDSTPT_ARGS pArguments
    );

HRESULT
EnableServer( 
    _In_ IWdsTransportServer* pWdsTransportServer
    );

HRESULT
DisableServer( 
    _In_ IWdsTransportServer* pWdsTransportServer
    );

HRESULT
AddNamespace( 
    _In_ IWdsTransportServer* pWdsTransportServer,
    _In_ PWDSTPT_ARGS pArguments
    );

HRESULT
RemoveNamespace( 
    _In_ IWdsTransportServer* pWdsTransportServer,
    _In_ PWDSTPT_ARGS pArguments
    );
