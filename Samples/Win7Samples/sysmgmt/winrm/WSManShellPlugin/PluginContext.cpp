/******************************************************************************
 * <copyright file="PluginContext.cpp" company="Microsoft">
 *     Copyright (c) Microsoft Corporation.  All rights reserved.
 * </copyright>                                                                
 *****************************************************************************/

#include "PluginContext.h"

// Constructor.
CShellContext::CShellContext()
    : shellPluginRequest(NULL),
      shellShutdownRegisterWait(NULL)
{
}

// Destructor.
CShellContext::~CShellContext()
{
}

// Initialize to start a shell.
DWORD CShellContext::Initialize(__in PVOID pluginContext,
                                __in WSMAN_PLUGIN_REQUEST * requestDetails,
                                __in DWORD flags,
                                __in_opt WSMAN_SHELL_STARTUP_INFO * startupInfo,
                                __in_opt WSMAN_DATA * inboundShellInformation)
{
    shellPluginRequest = requestDetails;
    DWORD dwRet = NO_ERROR;

    if ((RegisterWaitForSingleObject(&shellShutdownRegisterWait, 
                                     requestDetails->shutdownNotificationHandle, 
                                     _ShellCancellationCallback, 
                                     this, 
                                     INFINITE, 
                                     WT_EXECUTELONGFUNCTION | WT_EXECUTEONLYONCE) == 0))
    {
        dwRet = GetLastError();
    }
    else
    {
        dwRet = WSManPluginReportContext(requestDetails, 0, (void *) this);
        if (dwRet)
        {
            if (UnregisterWaitEx(shellShutdownRegisterWait, INVALID_HANDLE_VALUE))
            {
                shellShutdownRegisterWait = NULL;
            }
        }
    }

    return dwRet;
}

// Run a command within the shell.
VOID CShellContext::Command(__in WSMAN_PLUGIN_REQUEST * requestDetails,
                            __in DWORD flags,
                            __in PCWSTR commandLine,
                            __in_opt WSMAN_COMMAND_ARG_SET * arguments)
{
    CCommandContext * command = new CCommandContext;
    if (command == NULL)
    {
        WSManPluginOperationComplete(requestDetails, 
                                     0, 
                                     ERROR_OUTOFMEMORY, 
                                     L"Not enough memory to carry out the operation");
        return;
    }

    DWORD dwRet = command->Initialize(requestDetails,
                                      flags,
                                      this,
                                      commandLine,
                                      arguments);
    if (dwRet != NO_ERROR)
    {
        WSManPluginOperationComplete(requestDetails, 
                                     0, 
                                     dwRet, 
                                     L"Command::Initialize failed");
        delete command;
        command = NULL;
    }

    return;
}

// Send input data to shell.
VOID CShellContext::Send(__in WSMAN_PLUGIN_REQUEST * requestDetails,
                         __in DWORD flags,
                         __in PCWSTR stream,
                         __in WSMAN_DATA * inboundData)
{
    WSManPluginOperationComplete(requestDetails, 
                                 0, 
                                 ERROR_INVALID_OPERATION, 
                                 L"Inbound data not accepted for shells");
}

// Receive output data from shell.
VOID CShellContext::Receive(__in WSMAN_PLUGIN_REQUEST * requestDetails,
                            __in DWORD flags,
                            __in WSMAN_STREAM_ID_SET * streamSet)

{
    WSManPluginOperationComplete(requestDetails, 
                                 0, 
                                 ERROR_INVALID_OPERATION, 
                                 L"Outbound data not available for shells");
}

// send signal to shell.
VOID CShellContext::Signal(__in WSMAN_PLUGIN_REQUEST * requestDetails,
                           __in DWORD flags,
                           __in PCWSTR code)
{
    WSManPluginOperationComplete(requestDetails, 
                                 0, 
                                 ERROR_INVALID_OPERATION, 
                                 L"Signal not available for shells");
}

// Registered callback for shell shutdown notification.
VOID CALLBACK CShellContext::_ShellCancellationCallback(PVOID context,
                                                        BOOLEAN TimerOrWaitFired)
{
    CShellContext *shell = (CShellContext*) context;
    shell->ShellCancellationCallback();
}

// Complete plugin shell operation when shell shutdown notification happens.
// (shell context is released in WSManPluginReleaseShellContext)
VOID CShellContext::ShellCancellationCallback()
{
    if (UnregisterWaitEx(shellShutdownRegisterWait, NULL))
    {
        shellShutdownRegisterWait = NULL;
    }

    WSManPluginOperationComplete(shellPluginRequest, 
                                 0, 
                                 ERROR_CANCELLED, 
                                 L"Shell was cancelled by WSMAN");
}



// Constructor.
CCommandContext::CCommandContext()
    : commandPluginRequest(NULL),
      receivePluginRequest(NULL),
      shell(NULL),
      commandShutdownRegisterWait(NULL),
      receiveShutdownRegisterWait(NULL),
      receiveAvailable(NULL)
{
}

// Destructor.
CCommandContext::~CCommandContext()
{
}

// Initialize to run a command within a shell.
DWORD CCommandContext::Initialize(__in WSMAN_PLUGIN_REQUEST * requestDetails,
                                  __in DWORD flags,
                                  __in CShellContext * parentShell,
                                  __in PCWSTR commandLine,
                                  __in_opt WSMAN_COMMAND_ARG_SET * arguments)
{
    commandPluginRequest = requestDetails;
    shell = parentShell;

    receiveAvailable = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (receiveAvailable == NULL)
    {
        return GetLastError();
    }

    // Command ready for further operations
    DWORD dwRet = 0;
    if (RegisterWaitForSingleObject(&commandShutdownRegisterWait, 
                                    requestDetails->shutdownNotificationHandle, 
                                    _CommandCancellationCallback, 
                                    this, 
                                    INFINITE, 
                                    WT_EXECUTELONGFUNCTION | WT_EXECUTEONLYONCE) == 0)
    {
        dwRet = GetLastError();
    }
    else
    {
        dwRet = WSManPluginReportContext(requestDetails, 0, (void *) this);

        if (dwRet)
        {
            if (UnregisterWaitEx(commandShutdownRegisterWait, INVALID_HANDLE_VALUE))
            {
                commandShutdownRegisterWait = NULL;
            }
        }
    }
    return dwRet;
}

// Send input data to command.
VOID CCommandContext::Send(__in WSMAN_PLUGIN_REQUEST * requestDetails,
                           __in DWORD flags,
                           __in PCWSTR stream,
                           __in WSMAN_DATA * inboundData)
{
    if (wcscmp(stream, WSMAN_STREAM_ID_STDIN) != 0)
    {
        // only accept inbound data to be retrieved from stdin
        WSManPluginOperationComplete(requestDetails, 
                                     0, 
                                     ERROR_INVALID_OPERATION, 
                                     L"Inbound data only accepted for stdin stream");
        return;
    }

    //Send data back
    HANDLE waitHandles[] = 
    { 
        receiveAvailable, 
        requestDetails->shutdownNotificationHandle 
    };
    DWORD waitResult = WaitForMultipleObjects(2, 
                                              waitHandles, 
                                              FALSE, 
                                              INFINITE);
    if (waitResult == WAIT_OBJECT_0+1)
    {
        //Need to fail the operation
        WSManPluginOperationComplete(requestDetails, 
                                     0, 
                                     ERROR_CANCELLED, 
                                     L"Operation was cancelled");
        return;
    }
    DWORD receiveFlags = 0;
    PCWSTR commandState = NULL;
    if (flags == WSMAN_FLAG_SEND_NO_MORE_DATA)
    {
        receiveFlags = WSMAN_FLAG_RECEIVE_RESULT_NO_MORE_DATA;
        commandState = WSMAN_COMMAND_STATE_DONE;
    }
    else
    {
        commandState = WSMAN_COMMAND_STATE_RUNNING;
    }
    receiveFlags |= WSMAN_FLAG_RECEIVE_FLUSH;

    DWORD result = WSManPluginReceiveResult(receivePluginRequest, 
                                            receiveFlags, 
                                            WSMAN_STREAM_ID_STDOUT, 
                                            inboundData, 
                                            commandState, 
                                            NO_ERROR);
    if ((result != NO_ERROR) ||
        (flags == WSMAN_FLAG_SEND_NO_MORE_DATA))
    {
        HANDLE h = (HANDLE)InterlockedExchangePointer((PVOID*)&receiveShutdownRegisterWait, NULL);
        if (h)
        {
            if (UnregisterWaitEx(h, INVALID_HANDLE_VALUE))
            {
                h = NULL;
            }

            // Input is over so output is over
            WSManPluginOperationComplete(receivePluginRequest, 0, result, NULL);

            receivePluginRequest = NULL;
            ResetEvent(receiveAvailable);
        }

    }

    // done with this send
    WSManPluginOperationComplete(requestDetails, 0, NO_ERROR, NULL);
}
 
// Receive output data from command.
VOID CCommandContext::Receive(__in WSMAN_PLUGIN_REQUEST * requestDetails,
                              __in DWORD flags,
                              __in WSMAN_STREAM_ID_SET * streamSet)
{
    // support only to receive from stdout and stderr
    if (streamSet == NULL || 
        streamSet->streamIDsCount != 2)
    {
        WSManPluginOperationComplete(requestDetails, 
                                     0, 
                                     ERROR_INVALID_PARAMETER, 
                                     L"Expecting 2 streams for receive of stdout and stderr");
        return;
    }
    if (wcscmp(streamSet->streamIDs[0], L"stdout") != 0 || 
        wcscmp(streamSet->streamIDs[1], L"stderr") != 0)
    {
        WSManPluginOperationComplete(requestDetails, 
                                     0, 
                                     ERROR_INVALID_PARAMETER, 
                                     L"Expecting 2 streams for receive of stdout and stderr");
        return;
    }

    if (RegisterWaitForSingleObject(&receiveShutdownRegisterWait, 
                                    requestDetails->shutdownNotificationHandle, 
                                    _ReceiveCancellationCallback, 
                                    this, 
                                    INFINITE, 
                                    WT_EXECUTELONGFUNCTION | WT_EXECUTEONLYONCE) == 0)
    {
        WSManPluginOperationComplete(requestDetails, 
                                     0, 
                                     GetLastError(), 
                                     L"RegisterWaitForSingleObject failed");
        return;
    }

    // Set up the data so a Send can pipe data to the receive stream
    receivePluginRequest = requestDetails;
    SetEvent(receiveAvailable);
}

// send signal to command.
VOID CCommandContext::Signal(__in WSMAN_PLUGIN_REQUEST * requestDetails,
                             __in DWORD flags,
                             __in PCWSTR code)
{
    if (_wcsicmp(code, WSMAN_SIGNAL_SHELL_CODE_TERMINATE) == 0)
    {
        // Handle command signal such as to terminate running command
        WSManPluginOperationComplete(requestDetails, 0, NO_ERROR, NULL);
        HANDLE h = (HANDLE)InterlockedExchangePointer((PVOID*)&commandShutdownRegisterWait, NULL);
        if (h)
        {
            if (UnregisterWaitEx(h, INVALID_HANDLE_VALUE))
            {
                h = NULL;
            }

            WSManPluginOperationComplete(commandPluginRequest, 0, NO_ERROR, NULL);
        }
    }
    else
    {
        // Signal operation is done
        WSManPluginOperationComplete(requestDetails, 0, ERROR_INVALID_OPERATION, 
                                     L"Unsupported signal");
    }
}

// Registered callback for command shutdown notification.
VOID CALLBACK CCommandContext::_CommandCancellationCallback(PVOID context,
                                                            BOOLEAN TimerOrWaitFired)
{
    CCommandContext *command = (CCommandContext*) context;
    command->CommandCancellationCallback();
}

// Complete plugin command operation when command shutdown notification happens.
// (command context is released in WSManPluginReleaseCommandContext)
VOID CCommandContext::CommandCancellationCallback()
{
    HANDLE h = (HANDLE)InterlockedExchangePointer((PVOID*)&commandShutdownRegisterWait, NULL);
    if (h)
    {
        if (UnregisterWaitEx(h, NULL))
        {
            h = NULL;
        }

        WSManPluginOperationComplete(commandPluginRequest, 
                                     0, 
                                     ERROR_CANCELLED, 
                                     L"Command was shutdown by wsman service");
    }
}

// Registered callback for receive shutdown notification.
VOID CALLBACK CCommandContext::_ReceiveCancellationCallback(PVOID context,
                                                        BOOLEAN TimerOrWaitFired)
{
    CCommandContext * command = (CCommandContext *) context;
    command->ReceiveCancellationCallback();
}
VOID CCommandContext::ReceiveCancellationCallback()
{
    HANDLE h = (HANDLE)InterlockedExchangePointer((PVOID*)&receiveShutdownRegisterWait, NULL);
    if (h)
    {
        if (UnregisterWaitEx(h, NULL))
        {
            h = NULL;
        }

        ResetEvent(receiveAvailable);
        WSMAN_PLUGIN_REQUEST * receiveRequest = receivePluginRequest;
        receivePluginRequest = NULL;

        WSManPluginOperationComplete(receiveRequest, 
                                     0, 
                                     ERROR_CANCELLED, 
                                     L"Receive Shutdown");
    }
}