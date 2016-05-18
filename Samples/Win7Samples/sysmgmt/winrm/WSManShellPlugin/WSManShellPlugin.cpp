//*****************************************************************************
//  Copyright (C) 2009 Microsoft Corporation
//  All rights reserved.
//
// Shell Plugin needs to implement all following required entries
//    WSManPluginStartup
//    WSManPluginShutdown
//    WSManPluginShell
//    WSManPluginCommand
//    WSManPluginSend
//    WSManPluginReceive
//    WSManPluginSignal
//    WSManPluginReleaseShellContext
//    WSManPluginReleaseCommandContext
//
//*****************************************************************************

#include <windows.h>
#define WSMAN_API_VERSION_1_0
#include <wsman.h>
#include "PluginContext.h"

BOOL WINAPI DllMain(IN HINSTANCE instance, IN DWORD reason, PVOID /*reserved*/)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        //
        // we disable all thread attach and detach messages to minimize our working set
        //
        if (!DisableThreadLibraryCalls(instance)) 
        {
            return FALSE;
        }
    }

    return TRUE;
}

/*------------------------------------------------------------------------
typedef DWORD (WINAPI *WSMAN_PLUGIN_STARTUP)(
    __in DWORD flags,
    __in PCWSTR applicationIdentification,
    __in_opt PCWSTR extraInfo,
    __out PVOID *pluginContext
    );

Each plug-in needs to support the Startup callback.  A plug-in may be 
initialized more than once within the same process, but only once per 
applicationIdentification.
  ------------------------------------------------------------------------*/
extern "C" DWORD WINAPI WSManPluginStartup(__in DWORD flags,
                                           __in PCWSTR applicationIdentification,
                                           __in_opt PCWSTR extraInfo,
                                           __out PVOID * pluginContext)
{
    return NO_ERROR;
}

/*------------------------------------------------------------------------
typedef DWORD (WINAPI *WSMAN_PLUGIN_SHUTDOWN)(
    __in_opt PVOID pluginContext,
    __in DWORD flags,
    __in DWORD reason
    );

Each plug-in needs to support the Shutdown callback.  Each successful call 
to Startup will result in a call to Shutdown before the DLL is unloaded.
It is important to make sure the plug-in tracks the number of times the 
Startup entry point is called so the plug-in is not shutdown prematurely.
  ------------------------------------------------------------------------*/
extern "C" DWORD WINAPI WSManPluginShutdown(__in_opt PVOID pluginContext,
                                            __in DWORD flags,
                                            __in DWORD reason)
{
    return NO_ERROR;
}

/*------------------------------------------------------------------------
typedef VOID (WINAPI *WSMAN_PLUGIN_SHELL)(
    __in PVOID pluginContext,   //Relates to context returned from WSMAN_PLUGIN_STARTUP
    __in WSMAN_PLUGIN_REQUEST *requestDetails,
    __in DWORD flags,
    __in_opt WSMAN_SHELL_STARTUP_INFO *startupInfo,
    __in_opt WSMAN_DATA *inboundShellInformation
    );

A plug-in that supports the Shell operations needs to implement this callback
to allow commands to be created and to allow data to be streamed into either
a shell or command.  The plug-in must call WSManPluginReportContext to 
report the shell context.  Once the shell is completed or when it is closed
via the operationClosed boolean value or operationClosedHandle in the 
requestDetails the plug-in needs to call WSManPluginOperationComplete.
The shell is active until this time.
  ------------------------------------------------------------------------*/
extern "C" VOID WINAPI WSManPluginShell(__in PVOID pluginContext,
                                        __in WSMAN_PLUGIN_REQUEST * requestDetails,
                                        __in DWORD flags,
                                        __in_opt WSMAN_SHELL_STARTUP_INFO * startupInfo,
                                        __in_opt WSMAN_DATA * inboundShellInformation)
{
    // Shell
    CShellContext * shell = new CShellContext;
    if (shell == NULL)
    {
        WSManPluginOperationComplete(requestDetails, 
                                     0, 
                                     ERROR_OUTOFMEMORY, 
                                     L"Not enough memory to carry out the operation");
        return;
    }
    DWORD dwRet = shell->Initialize(pluginContext, 
                                    requestDetails, 
                                    flags, 
                                    startupInfo, 
                                    inboundShellInformation);
    if (dwRet != NO_ERROR)
    {
        WSManPluginOperationComplete(requestDetails, 
                                     0, 
                                     dwRet, 
                                     L"Shell::Initialize failed");
        delete shell;
        return;
    }

    return;
}

/*------------------------------------------------------------------------
typedef VOID (WINAPI *WSMAN_PLUGIN_COMMAND)(
    __in WSMAN_PLUGIN_REQUEST *requestDetails,
    __in DWORD flags,
    __in PVOID shellContext,
    __in PCWSTR commandLine,
    __in_opt WSMAN_COMMAND_ARG_SET *arguments
    );

A plug-in that supports the Shell operations and needs to create commands
that are associated with the shell needs to implement this callback.
The plug-in must call WSManPluginReportContext to 
report the command context.  Once the command is completed or when it is closed
via the operationClosed boolean value or operationClosedHandle in the 
requestDetails the plug-in needs to call WSManPluginOperationComplete.
The command is active until this time.
  ------------------------------------------------------------------------*/
extern "C" VOID WINAPI WSManPluginCommand(__in WSMAN_PLUGIN_REQUEST * requestDetails,
                                          __in DWORD flags,
                                          __in PVOID shellContext,
                                          __in PCWSTR commandLine,
                                          __in_opt WSMAN_COMMAND_ARG_SET * arguments)
{
    // Verify input parameters
    if (NULL == shellContext)
    { 
        WSManPluginOperationComplete(requestDetails, 
                                     0, 
                                     ERROR_INVALID_PARAMETER, 
                                     L"Parameter cannot be NULL");
        return;
    }

    // Command
    CShellContext * shell = (CShellContext *) shellContext;
    shell->Command(requestDetails,
                   flags,
                   commandLine,
                   arguments);

    return;
}

/*------------------------------------------------------------------------
typedef VOID (WINAPI *WSMAN_PLUGIN_SEND)(
    __in WSMAN_PLUGIN_REQUEST *requestDetails,
    __in DWORD flags,
    __in PVOID shellContext,
    __in_opt PVOID commandContext,
    __in PCWSTR stream,
    __in WSMAN_DATA *inboundData
    );

A plug-in receives an inbound data stream to either the shell or command
via this callback.  Each piece of data causes the callback to be called once.
For each piece of data the plug-in calls WSManPluginOperationComplete to 
acknowledge receipt and to allow the next piece of data to be delivered.
  ------------------------------------------------------------------------*/
extern "C" VOID WINAPI WSManPluginSend(__in WSMAN_PLUGIN_REQUEST * requestDetails,
                                       __in DWORD flags,
                                       __in PVOID shellContext,
                                       __in_opt PVOID commandContext,
                                       __in PCWSTR stream,
                                       __in WSMAN_DATA * inboundData)
{
    // Verify input parameters
    if (NULL == shellContext)
    { 
        WSManPluginOperationComplete(requestDetails, 
                                     0, 
                                     ERROR_INVALID_PARAMETER, 
                                     L"Parameter cannot be NULL");
        return;
    }

    // Send to shell or command
    CShellContext *shell = (CShellContext *) shellContext;
    CCommandContext *command = (CCommandContext *) commandContext;
 
    if (command)
    {
        command->Send(requestDetails,
                      flags,
                      stream,
                      inboundData);
    }
    else
    {
        shell->Send(requestDetails,
                    flags,
                    stream,
                    inboundData);
    }

    return;
}

/*------------------------------------------------------------------------
typedef VOID (WINAPI *WSMAN_PLUGIN_RECEIVE)(
    __in WSMAN_PLUGIN_REQUEST *requestDetails,
    __in DWORD flags,
    __in PVOID shellContext,
    __in_opt PVOID commandContext,
    __in_opt WSMAN_STREAM_ID_SET *streamSet
    ); 

A plug-in sends an outbound data stream from either the shell or command
via this callback.  This API is called when an inbound request from a client
is received.  This callback may be called against the shell and/or command
based on the client request.  Each piece of data that needs to be sent back
to the client is done so through the WSManPluginReceiveResult API.  Once 
all data has been send, when the stream is terminated via some internal means,
or if the receive call is cancelled through the operationClosed boolean 
value or operationClosedHandle, the plug-in needs to call 
WSManPluginOperationComplete.  The operation is marked as active until this 
time.
  ------------------------------------------------------------------------*/
extern "C" VOID WINAPI WSManPluginReceive(__in WSMAN_PLUGIN_REQUEST * requestDetails,
                                          __in DWORD flags,
                                          __in PVOID shellContext,
                                          __in_opt PVOID commandContext,
                                          __in_opt WSMAN_STREAM_ID_SET * streamSet)
{
    // Verify input parameters
    if (NULL == shellContext)
    { 
        WSManPluginOperationComplete(requestDetails, 
                                     0, 
                                     ERROR_INVALID_PARAMETER, 
                                     L"Parameter cannot be NULL");
        return;
    }

    // Receive from shell or command
    CShellContext *shell = (CShellContext *) shellContext;
    CCommandContext *command = (CCommandContext *) commandContext;
 
    if (command)
    {
        command->Receive(requestDetails,
                         flags,
                         streamSet);
    }
    else
    {
        shell->Receive(requestDetails,
                       flags,
                       streamSet);
    }

    return;
}

/*------------------------------------------------------------------------
typedef VOID (WINAPI *WSMAN_PLUGIN_SIGNAL)(
    __in WSMAN_PLUGIN_REQUEST *requestDetails,
    __in DWORD flags,
    __in PVOID shellContext,
    __in_opt PVOID commandContext,
    __in PCWSTR code
    );

A plug-in receives an inbound signal to either the shell or command
via this callback.  Each signal causes the callback to be called once.
For each call the plug-in calls WSManPluginOperationComplete to 
acknowledge receipt and to allow the next signal to be received.
A signal can cause the shell or command to be terminated, so the result
of this callback may be many completion calls for the Signal, Receive, Command
and Shell operations.
  ------------------------------------------------------------------------*/
extern "C" VOID WINAPI WSManPluginSignal(__in WSMAN_PLUGIN_REQUEST * requestDetails,
                                         __in DWORD flags,
                                         __in PVOID shellContext,
                                         __in_opt PVOID commandContext,
                                         __in PCWSTR code)
{
    // Verify input parameters
    if (NULL == shellContext)
    { 
        WSManPluginOperationComplete(requestDetails, 
                                     0, 
                                     ERROR_INVALID_PARAMETER, 
                                     L"Parameter cannot be NULL");
        return;
    }

    // Signal to shell or command
    CShellContext *shell = (CShellContext *) shellContext;
    CCommandContext *command = (CCommandContext *) commandContext;
 
    if (command)
    {
        command->Signal(requestDetails,
                        flags,
                        code);
    }
    else
    {
        shell->Signal(requestDetails,
                      flags,
                      code);
    }

    return;
}

/*------------------------------------------------------------------------
typedef VOID (WINAPI *WSMAN_PLUGIN_RELEASE_SHELL_CONTEXT)(
    __in PVOID shellContext
    );

WS-Man calls the WSMAN_PLUGIN_RELEASE_SHELL_CONTEXT entry point during 
shell shutdown when it is safe to delete the plug-in shell context. Any 
context reported through WSManPluginReportContext may not be deleted until 
the corresponding release function has been called. Failure to follow the 
contract will result in errors being generated.
  ------------------------------------------------------------------------*/
extern "C" VOID WINAPI WSManPluginReleaseShellContext(__in PVOID shellContext)
{
    CShellContext *shell = (CShellContext*) shellContext;
    if (shell) delete shell;
}

/*------------------------------------------------------------------------
typedef VOID (WINAPI *WSMAN_PLUGIN_RELEASE_COMMAND_CONTEXT)(
    __in PVOID shellContext,
    __in PVOID commandContext
    );

WS-Man calls the WSMAN_PLUGIN_RELEASE_COMMAND_CONTEXT entry point when it 
is safe to delete the plug-in command context. Any context reported through 
WSManPluginReportContext may not be deleted until the corresponding release 
function has been called. Failure to follow the contract will result in 
errors being generated.
  ------------------------------------------------------------------------*/
extern "C" VOID WINAPI WSManPluginReleaseCommandContext(__in PVOID shellContext, __in PVOID commandContext)
{
    CCommandContext *command = (CCommandContext*) commandContext;
    if (command) delete command;
}