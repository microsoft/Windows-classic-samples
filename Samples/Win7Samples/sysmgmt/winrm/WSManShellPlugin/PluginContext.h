/******************************************************************************
 * <copyright file="PluginContext.h" company="Microsoft">
 *     Copyright (c) Microsoft Corporation.  All rights reserved.
 * </copyright>                                                                
 *****************************************************************************/

#ifndef _PLUGINCONTEXT_H_
#define _PLUGINCONTEXT_H_

#include <windows.h>
#define WSMAN_API_VERSION_1_0
#include <wsman.h>

class CCommandContext;

/*------------------------------------------------------------------------
Implements a class to handle shell operations such as 
Command / Send / Receive / Signal operations targeted at Shell
  ------------------------------------------------------------------------*/
class CShellContext
{
public:
    CShellContext();
    ~CShellContext();

    DWORD Initialize(__in PVOID pluginContext,
                     __in WSMAN_PLUGIN_REQUEST * requestDetails,
                     __in DWORD flags,
                     __in_opt WSMAN_SHELL_STARTUP_INFO * startupInfo,
                     __in_opt WSMAN_DATA * inboundShellInformation);

    VOID Command(__in WSMAN_PLUGIN_REQUEST * requestDetails,
                 __in DWORD flags,
                 __in PCWSTR commandLine,
                 __in_opt WSMAN_COMMAND_ARG_SET * arguments);

    VOID Send(__in WSMAN_PLUGIN_REQUEST * requestDetails,
              __in DWORD flags,
              __in PCWSTR stream,
              __in WSMAN_DATA * inboundData);

    VOID Receive(__in WSMAN_PLUGIN_REQUEST * requestDetails,
                 __in DWORD flags,
                 __in WSMAN_STREAM_ID_SET * streamSet);

    VOID Signal(__in WSMAN_PLUGIN_REQUEST * requestDetails,
                __in DWORD flags,
                __in PCWSTR code);

private:

    static VOID CALLBACK _ShellCancellationCallback(PVOID context,
                                                    BOOLEAN TimerOrWaitFired);
    VOID ShellCancellationCallback();


    WSMAN_PLUGIN_REQUEST * shellPluginRequest;
    HANDLE shellShutdownRegisterWait;
};

/*------------------------------------------------------------------------
Implements a class to handle command operations such as 
Send / Receive / Signal operations targeted at command (not Shell)
  ------------------------------------------------------------------------*/
class CCommandContext
{
public:
    CCommandContext();
    ~CCommandContext();

    DWORD Initialize(__in WSMAN_PLUGIN_REQUEST * requestDetails,
                     __in DWORD flags,
                     __in CShellContext * shell,
                     __in PCWSTR commandLine,
                     __in_opt WSMAN_COMMAND_ARG_SET * arguments);

    VOID Send(__in WSMAN_PLUGIN_REQUEST * requestDetails,
              __in DWORD flags,
              __in PCWSTR stream,
              __in WSMAN_DATA * inboundData);

    VOID Receive(__in WSMAN_PLUGIN_REQUEST * requestDetails,
                 __in DWORD flags,
                 __in WSMAN_STREAM_ID_SET * streamSet);

    VOID Signal(__in WSMAN_PLUGIN_REQUEST * requestDetails,
                __in DWORD flags,
                __in PCWSTR code);

private:
    static VOID CALLBACK _CommandCancellationCallback(PVOID context,
                                                      BOOLEAN TimerOrWaitFired);
    VOID CommandCancellationCallback();

    static VOID CALLBACK _ReceiveCancellationCallback(PVOID context,
                                                      BOOLEAN TimerOrWaitFired);
    VOID ReceiveCancellationCallback();

    WSMAN_PLUGIN_REQUEST * commandPluginRequest;
    WSMAN_PLUGIN_REQUEST * receivePluginRequest;
    CShellContext * shell;
    HANDLE commandShutdownRegisterWait;
    HANDLE receiveShutdownRegisterWait;
    HANDLE receiveAvailable;
};

#endif