/******************************************************************************
 * <copyright file="ShellClient.h" company="Microsoft">
 *     Copyright (c) Microsoft Corporation.  All rights reserved.
 * </copyright>                                                                
 *****************************************************************************/

#ifndef _SHELLCLIENT_H_
#define _SHELLCLIENT_H_

#include <windows.h>
#include <iostream>
#define WSMAN_API_VERSION_1_0
#include <wsman.h>

/*------------------------------------------------------------------------
Implements a class to call shell client APIs
  ------------------------------------------------------------------------*/
class CShellClient
{
public:
    CShellClient();
    ~CShellClient();

    BOOL Setup(__in PCWSTR connection,
               DWORD authenticationMechanism,
               __in PCWSTR username,
               __in PCWSTR password);
    BOOL Execute(__in PCWSTR resourceUri,
                 __in PCWSTR commandLine,
                 __in_opt PSTR sendData,
                 DWORD count);

private:

    WSMAN_API_HANDLE m_apiHandle;
    WSMAN_SESSION_HANDLE m_session;
    WSMAN_SHELL_HANDLE m_shell;
    WSMAN_COMMAND_HANDLE m_command;

    DWORD m_errorCode;
    DWORD m_ReceiveErrorCode;
    WSMAN_SHELL_ASYNC m_async;
    WSMAN_SHELL_ASYNC m_ReceiveAsync;
    HANDLE m_event;
    HANDLE m_ReceiveEvent;
    BOOL m_bSetup;
    BOOL m_bExecute;

    BOOL Send(__in_opt PSTR sendData,
              BOOL endOfStream);

    static void CALLBACK WSManShellCompletionFunction(
        __in_opt PVOID operationContext,
        DWORD flags,
        __in WSMAN_ERROR *error,
        __in WSMAN_SHELL_HANDLE shell,
        __in_opt WSMAN_COMMAND_HANDLE command,
        __in_opt WSMAN_OPERATION_HANDLE operationHandle,
        __in_opt WSMAN_RECEIVE_DATA_RESULT *data
    );
    void CALLBACK m_WSManShellCompletionFunction(
        DWORD flags,
        __in WSMAN_ERROR *error,
        __in WSMAN_SHELL_HANDLE shell,
        __in_opt WSMAN_COMMAND_HANDLE command,
        __in_opt WSMAN_OPERATION_HANDLE operationHandle,
        __in_opt WSMAN_RECEIVE_DATA_RESULT *data
    );

    static void CALLBACK ReceiveCallback(
        __in_opt PVOID operationContext,
        DWORD flags,
        __in WSMAN_ERROR *error,
        __in WSMAN_SHELL_HANDLE shell,
        __in_opt WSMAN_COMMAND_HANDLE command,
        __in_opt WSMAN_OPERATION_HANDLE operationHandle,
        __in_opt WSMAN_RECEIVE_DATA_RESULT *data
    );
    void CALLBACK m_ReceiveCallback(
        DWORD flags,
        __in WSMAN_ERROR *error,
        __in WSMAN_SHELL_HANDLE shell,
        __in_opt WSMAN_COMMAND_HANDLE command,
        __in_opt WSMAN_OPERATION_HANDLE operationHandle,
        __in_opt WSMAN_RECEIVE_DATA_RESULT *data
    );

    VOID Cleanup();
};

#endif