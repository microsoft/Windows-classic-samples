/******************************************************************************
 * <copyright file="ShellClient.cpp" company="Microsoft">
 *     Copyright (c) Microsoft Corporation.  All rights reserved.
 * </copyright>                                                                
 *****************************************************************************/

#include "ShellClient.h"

// Constructor.
CShellClient::CShellClient()
    : m_apiHandle(NULL),
    m_session(NULL),
    m_shell(NULL),
    m_command(NULL),
    m_errorCode(NO_ERROR),
    m_ReceiveErrorCode(NO_ERROR),
    m_event(NULL),
    m_ReceiveEvent(NULL),
    m_bSetup(FALSE),
    m_bExecute(FALSE)
{
    ZeroMemory(&m_async, sizeof(m_async));
}

// Destructor.
CShellClient::~CShellClient()
{
    Cleanup();
}

// Initialize session for subsequent operations
BOOL CShellClient::Setup(__in PCWSTR connection,
                         DWORD authenticationMechanism,
                         __in PCWSTR username,
                         __in PCWSTR password)
{
    if (m_bSetup) return TRUE;

    // initialize the WinRM client
    m_errorCode = WSManInitialize(0, 
                                  &m_apiHandle);
    if (NO_ERROR != m_errorCode)
    {
        wprintf(L"WSManInitialize failed: %d\n", m_errorCode);
        return FALSE;
    }

    // Create a session which can be used to perform subsequent operations
    WSMAN_AUTHENTICATION_CREDENTIALS serverAuthenticationCredentials;
    serverAuthenticationCredentials.authenticationMechanism = authenticationMechanism;
    serverAuthenticationCredentials.userAccount.username = username;
    serverAuthenticationCredentials.userAccount.password = password;
    m_errorCode = WSManCreateSession(m_apiHandle, 
                                     connection, 
                                     0, 
                                     &serverAuthenticationCredentials,
                                     NULL, 
                                     &m_session);
    if (NO_ERROR != m_errorCode) 
    {
        wprintf(L"WSManCreateSession failed: %d\n", m_errorCode);
        return FALSE;
    }

    // Repeat the call to set any desired session options
    WSManSessionOption option = WSMAN_OPTION_DEFAULT_OPERATION_TIMEOUTMS;
    WSMAN_DATA data;
    data.type = WSMAN_DATA_TYPE_DWORD;
    data.number = 60000;
    m_errorCode = WSManSetSessionOption(m_session, 
                                        option, 
                                        &data);
    if (NO_ERROR != m_errorCode) 
    {
        wprintf(L"WSManSetSessionOption failed: %d\n", m_errorCode);
        return FALSE;
    }

    // Prepare async call
    m_event = CreateEvent(0, FALSE, FALSE, NULL);
    if (NULL == m_event)
    {
        m_errorCode = GetLastError();
        wprintf(L"CreateEvent failed: %d\n", m_errorCode);
        return FALSE;
    }
    m_async.operationContext = this;
    m_async.completionFunction = &WSManShellCompletionFunction;

    m_ReceiveEvent = CreateEvent(0, FALSE, FALSE, NULL);
    if (NULL == m_ReceiveEvent)
    {
        m_errorCode = GetLastError();
        wprintf(L"CreateEvent failed: %d\n", m_errorCode);
        return FALSE;
    }
    m_ReceiveAsync.operationContext = this;
    m_ReceiveAsync.completionFunction = &ReceiveCallback;

    m_bSetup = TRUE;

    return TRUE;
}

// Execute shell-related operations
BOOL CShellClient::Execute(__in PCWSTR resourceUri,
                           __in PCWSTR commandLine,
                           __in_opt PSTR sendData,
                           DWORD count)
{
    if (!m_bSetup)
    {
        wprintf(L"Setup() needs to be called first");
        return FALSE;
    }
    if (m_bExecute)
    {
        wprintf(L"Execute() can only be called once");
        return FALSE;
    }
    m_bExecute = TRUE;

    // WSManCreateShell
    WSManCreateShell(m_session,
                     0,  
                     resourceUri,
                     NULL,
                     NULL,
                     NULL,
                     &m_async,
                     &m_shell);
    WaitForSingleObject(m_event, INFINITE);
    if (NO_ERROR != m_errorCode) 
    {
        wprintf(L"WSManCreateShell failed: %d\n", m_errorCode);
        return FALSE;
    }

    // WSManRunShellCommand
    WSManRunShellCommand(m_shell,
                         0,  
                         commandLine,
                         NULL,
                         NULL,
                         &m_async,
                         &m_command);
    WaitForSingleObject(m_event, INFINITE);
    if (NO_ERROR != m_errorCode) 
    {
        wprintf(L"WSManRunShellCommand failed: %d\n", m_errorCode);
        return FALSE;
    }

    // WSManReceiveShellOutput
    WSMAN_OPERATION_HANDLE receiveOperation = NULL;
    WSManReceiveShellOutput(m_shell,
                            m_command,
                            0,
                            NULL,
                            &m_ReceiveAsync,
                            &receiveOperation);

    // Send operation can be executed many times to send large data
    if (count >= 1)
    {
        for (DWORD i = 1; i <= count; i++)
        {
            // last send operation should indicate end of stream
            if (!Send(sendData, (i == count)))
            {
                wprintf(L"Send %d failed.\n", i);
            }
        }
    }

    // Receive operation is finished
    WaitForSingleObject(m_ReceiveEvent, INFINITE);
    if (NO_ERROR != m_ReceiveErrorCode) 
    {
        wprintf(L"WSManReceiveShellOutput failed: %d\n", m_ReceiveErrorCode);
        return FALSE;
    }
    m_errorCode = WSManCloseOperation(receiveOperation,
                                      0);
    if (NO_ERROR != m_errorCode) 
    {
        wprintf(L"WSManCloseOperation failed: %d\n", m_errorCode);
        return FALSE;
    }

    return TRUE;
}

BOOL CShellClient::Send(__in_opt PSTR sendData, 
                        BOOL endOfStream)
{
    // WSManSendShellInput
    WSMAN_OPERATION_HANDLE sendOperation = NULL;
    WSMAN_DATA streamData;
    ZeroMemory(&streamData, sizeof(streamData));
    streamData.type = WSMAN_DATA_TYPE_BINARY;
    if (sendData == NULL || strlen(sendData) == 0)
    {
        streamData.binaryData.dataLength = 0;
        streamData.binaryData.data = NULL;
    }
    else
    {
        streamData.binaryData.dataLength = (strlen(sendData)) * sizeof(CHAR);
        streamData.binaryData.data = (BYTE *)sendData;
    }
    WSManSendShellInput(m_shell,
                        m_command,
                        0,
                        WSMAN_STREAM_ID_STDIN,
                        &streamData,
                        endOfStream,
                        &m_async,
                        &sendOperation);
    WaitForSingleObject(m_event, INFINITE);
    if (NO_ERROR != m_errorCode) 
    {
        wprintf(L"WSManSendShellInput failed: %d\n", m_errorCode);
        return FALSE;
    }
    m_errorCode = WSManCloseOperation(sendOperation,
                                      0);
    if (NO_ERROR != m_errorCode) 
    {
        wprintf(L"WSManCloseOperation failed: %d\n", m_errorCode);
        return FALSE;
    }

    return TRUE;
}

// Clean up the used resources
VOID CShellClient::Cleanup()
{
    if (NULL != m_command)
    {
        WSManCloseCommand(m_command,
                          0,
                          &m_async);
        WaitForSingleObject(m_event, INFINITE);
        if (NO_ERROR != m_errorCode) 
        {
            wprintf(L"WSManCloseCommand failed: %d\n", m_errorCode);
        }
        else
        {
            m_command = NULL;
        }
    }

    if (NULL != m_shell)
    {
        WSManCloseShell(m_shell,
                        0,
                        &m_async);
        WaitForSingleObject(m_event, INFINITE);
        if (NO_ERROR != m_errorCode) 
        {
            wprintf(L"WSManCloseShell failed: %d\n", m_errorCode);
        }
        else
        {
            m_shell = NULL;
        }
    }

    // Frees memory of session and closes all related operations before returning
    m_errorCode = WSManCloseSession(m_session, 0);
    if (NO_ERROR != m_errorCode)
    {
        wprintf(L"WSManCloseSession failed: %d\n", m_errorCode);
    }

    // deinitializes the Winrm client stack; all operations will 
    // finish before this API will return
    m_errorCode = WSManDeinitialize(m_apiHandle, 0);
    if (NO_ERROR != m_errorCode)
    {
        wprintf(L"WSManDeinitialize failed: %d\n", m_errorCode);
    }

    if (NULL != m_event)
    {
        CloseHandle(m_event);
        m_event = NULL;
    }
    if (NULL != m_ReceiveEvent)
    {
        CloseHandle(m_ReceiveEvent);
        m_ReceiveEvent = NULL;
    }

    m_bSetup = FALSE;
    m_bExecute = FALSE;
}

// async callback
void CALLBACK CShellClient::WSManShellCompletionFunction(
    __in_opt PVOID operationContext,
    DWORD flags,
    __in WSMAN_ERROR *error,
    __in WSMAN_SHELL_HANDLE shell,
    __in_opt WSMAN_COMMAND_HANDLE command,
    __in_opt WSMAN_OPERATION_HANDLE operationHandle,
    __in_opt WSMAN_RECEIVE_DATA_RESULT *data
    )
{
    if (operationContext)
    {
        CShellClient * context = reinterpret_cast<CShellClient *>(operationContext);
        context->m_WSManShellCompletionFunction(flags,
                                                error,
                                                shell,
                                                command,
                                                operationHandle,
                                                data);
    }
}
void CALLBACK CShellClient::m_WSManShellCompletionFunction(
    DWORD flags,
    __in WSMAN_ERROR *error,
    __in WSMAN_SHELL_HANDLE shell,
    __in_opt WSMAN_COMMAND_HANDLE command,
    __in_opt WSMAN_OPERATION_HANDLE operationHandle,
    __in_opt WSMAN_RECEIVE_DATA_RESULT *data
    )
{
    if (error && 0 != error->code)
    {
        m_errorCode = error->code;
        // NOTE: if the errorDetail needs to be used outside of the callback,
        // then need to allocate memory, copy the content to that memory
        // as error->errorDetail itself is owned by WSMan client stack and will
        // be deallocated and invalid when the callback exits
        wprintf(error->errorDetail);
    }

    // for non-receieve operation, the callback simply signals the async operation is finished
    SetEvent(m_event);
}






// Receive async callback
void CALLBACK CShellClient::ReceiveCallback(
    __in_opt PVOID operationContext,
    DWORD flags,
    __in WSMAN_ERROR *error,
    __in WSMAN_SHELL_HANDLE shell,
    __in_opt WSMAN_COMMAND_HANDLE command,
    __in_opt WSMAN_OPERATION_HANDLE operationHandle,
    __in_opt WSMAN_RECEIVE_DATA_RESULT *data
    )
{
    if (operationContext)
    {
        CShellClient * context = reinterpret_cast<CShellClient *>(operationContext);
        context->m_ReceiveCallback(flags,
                                   error,
                                   shell,
                                   command,
                                   operationHandle,
                                   data);
    }
}
void CALLBACK CShellClient::m_ReceiveCallback(
    DWORD flags,
    __in WSMAN_ERROR *error,
    __in WSMAN_SHELL_HANDLE shell,
    __in_opt WSMAN_COMMAND_HANDLE command,
    __in_opt WSMAN_OPERATION_HANDLE operationHandle,
    __in_opt WSMAN_RECEIVE_DATA_RESULT *data
    )
{
    if (error && 0 != error->code)
    {
        m_ReceiveErrorCode = error->code;
        // NOTE: if the errorDetail needs to be used outside of the callback,
        // then need to allocate memory, copy the content to that memory
        // as error->errorDetail itself is owned by WSMan client stack and will
        // be deallocated and invalid when the callback exits
        wprintf(error->errorDetail);
    }

    // Output the received data to the console
    if (data && data->streamData.type == WSMAN_DATA_TYPE_BINARY && data->streamData.binaryData.dataLength)
    {
        HANDLE hFile = ((0 == _wcsicmp(data->streamId, WSMAN_STREAM_ID_STDERR)) ? 
                         GetStdHandle(STD_ERROR_HANDLE) : GetStdHandle(STD_OUTPUT_HANDLE));

        DWORD t_BufferWriteLength = 0;
        WriteFile(hFile,
                  data->streamData.binaryData.data,
                  data->streamData.binaryData.dataLength,
                  &t_BufferWriteLength,
                  NULL);
    }

    // for WSManReceiveShellOutput, needs to wait for state to be done before signalliing the end of the operation
    if ((error && 0 != error->code) || (data && data->commandState && wcscmp(data->commandState, WSMAN_COMMAND_STATE_DONE) == 0))
    {
        SetEvent(m_ReceiveEvent);
    }
}