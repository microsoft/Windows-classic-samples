//*****************************************************************************
//	Copyright (C) Microsoft Corporation
//  All rights reserved.
//*****************************************************************************
#include "stdafx.h"

#define WSMAN_API_VERSION_1_1
#include <wsman.h>

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

#define URI_SHELL_CMD L"http://microsoft.test/shelltestresource"

static WSMAN_OPTION noProfileOption = 
{
    WSMAN_SHELL_OPTION_NOPROFILE,
    L"TRUE",                 
    FALSE
};
static WSMAN_OPTION_SET noProfileOptionSet = 
{
    1,
    &noProfileOption,
    FALSE
};

class ShellClient
{   
public:
    ShellClient() 
        :
        shellHandle(NULL),
        shellCreated(NULL),
        shellCreatedCount(0),
        shellCreatedError(NO_ERROR),

        shellDeleted(NULL),
        shellDeletedCount(0),
        shellDeletedError(NO_ERROR),

        shellDisconnected(NULL),
        shellDisconnectedCount(0),
        shellDisconnectedError(NO_ERROR),

        shellReconnected(NULL),
        shellReconnectedCount(0),
        shellReconnectedError(NO_ERROR),

        shellConnected(NULL),
        shellConnectedCount(0),
        shellConnectedError(NO_ERROR),

        commandHandle(NULL),
        commandCreated(NULL),
        commandCreatedCount(0),
        commandCreatedError(NO_ERROR),

        commandDeleted(NULL),
        commandDeletedCount(0),
        commandDeletedError(NO_ERROR),

        commandConnected(NULL),
        commandConnectedCount(0),
        commandConnectedError(NO_ERROR),

        signalHandle(NULL),
        signal(NULL),
        signalCount(0),
        signalError(NO_ERROR),

        receiveHandle(NULL),
        receive(NULL),
        receiveCount(0),
        receiveError(NO_ERROR),
        receiveEndOfStreamCount(0),
        receiveSignalEveryResult(false),
        receiveByteCount(0),

        sendHandle(NULL),
        send(NULL),
        sendCount(0),
        sendError(NO_ERROR),

        apiHandle(NULL),
        session(NULL),
        idleTimeout(1000),
        shellUri(URI_SHELL_CMD),

        shellName(NULL)
    {
        ZeroMemory(receiveBuffer, sizeof(receiveBuffer));
    }
    ~ShellClient() 
    {
        if (session)
        {
			WSManCloseSession(session, 0);            
        }
        if (apiHandle)
        {
            WSManDeinitialize(apiHandle, 0);
        }
        if (shellCreated)
            CloseHandle(shellCreated);
        if (shellDeleted)
            CloseHandle(shellDeleted);
        if (shellDisconnected)
            CloseHandle(shellDisconnected);
        if (shellReconnected)
            CloseHandle(shellReconnected);
        if (shellConnected)
            CloseHandle(shellConnected);
        if (commandCreated)
            CloseHandle(commandCreated);
        if (commandDeleted)
            CloseHandle(commandDeleted);
        if (commandConnected)
            CloseHandle(commandConnected);
        if (signal)
            CloseHandle(signal);
        if (receive)
            CloseHandle(receive);
        if (send)
            CloseHandle(send);
    }
    bool Initialize() 
    {
		//Create Events to synchronize WSMan Async APIs
        shellCreated = CreateEvent(NULL, FALSE, FALSE, NULL);
        shellDeleted = CreateEvent(NULL, FALSE, FALSE, NULL);
        shellDisconnected = CreateEvent(NULL, FALSE, FALSE, NULL);
        shellReconnected = CreateEvent(NULL, FALSE, FALSE, NULL);
        shellConnected = CreateEvent(NULL, FALSE, FALSE, NULL);
        commandCreated = CreateEvent(NULL, FALSE, FALSE, NULL);
        commandDeleted = CreateEvent(NULL, FALSE, FALSE, NULL);
        commandConnected = CreateEvent(NULL, FALSE, FALSE, NULL);
        signal = CreateEvent(NULL, FALSE, FALSE, NULL);
        receive = CreateEvent(NULL, FALSE, FALSE, NULL);
        send = CreateEvent(NULL, FALSE, FALSE, NULL);
        
        //INITIALIZE WSMAN CLIENT
        if (!WSManInitialize(WSMAN_FLAG_REQUESTED_API_VERSION_1_1, &apiHandle))            
        {
			return false;
        }

        WSMAN_AUTHENTICATION_CREDENTIALS creds;
        ZeroMemory(&creds, sizeof(WSMAN_AUTHENTICATION_CREDENTIALS));
		/*Use the following to set authentication information
		creds.authenticationMechanism = authFlag;
		creds.userAccount.username = username;
		creds.userAccount.password = password;
		userPassCreds->certificateThumbprint = thumbprint;
		userPassCreds->authenticationMechanism = WSMAN_FLAG_AUTH_CLIENT_CERTIFICATE;
		*/
		
		//set connection string
		PCWSTR connectionString = L"http://localhost:5985";

        //CREATE SESSION
        DWORD errorCode = WSManCreateSession(apiHandle, connectionString, 0, &creds, NULL, &session);
        if (errorCode != ERROR_SUCCESS)
        {
			return false;
        }
        
		//Set timeouts
		WSMAN_DATA timeout;
        timeout.type = WSMAN_DATA_TYPE_DWORD;
        timeout.number = 10000;
        errorCode = WSManSetSessionOption(session, WSMAN_OPTION_TIMEOUTMS_CREATE_SHELL, &timeout);
        if (errorCode != ERROR_SUCCESS)
        {
			return false;
        }
        errorCode = WSManSetSessionOption(session, WSMAN_OPTION_TIMEOUTMS_RUN_SHELL_COMMAND, &timeout);
        if (errorCode != ERROR_SUCCESS)
        {
			return false;
        }
        errorCode = WSManSetSessionOption(session, WSMAN_OPTION_TIMEOUTMS_RECEIVE_SHELL_OUTPUT, &timeout);
        if (errorCode != ERROR_SUCCESS)
        {
			return false;
        }
        errorCode = WSManSetSessionOption(session, WSMAN_OPTION_TIMEOUTMS_SEND_SHELL_INPUT, &timeout);
        if (errorCode != ERROR_SUCCESS)
        {
			return false;
        }
        errorCode = WSManSetSessionOption(session, WSMAN_OPTION_TIMEOUTMS_SIGNAL_SHELL, &timeout);
        if (errorCode != ERROR_SUCCESS)
        {
			return false;
        }
        errorCode = WSManSetSessionOption(session, WSMAN_OPTION_TIMEOUTMS_CLOSE_SHELL, &timeout);
        if (errorCode != ERROR_SUCCESS)
        {
			return false;
        }

        return true;
    }    
    bool CreateShell()
    {
        WSMAN_SHELL_ASYNC createShellAsync;
        ZeroMemory(&createShellAsync, sizeof(WSMAN_SHELL_ASYNC));
        createShellAsync.operationContext = this;
        createShellAsync.completionFunction = (WSMAN_SHELL_COMPLETION_FUNCTION)ShellCreatedCallback;

        WSMAN_STREAM_ID_SET stdinSet;
        PCWSTR stdinIds[] = { WSMAN_STREAM_ID_STDIN };
        stdinSet.streamIDsCount = sizeof(stdinIds) / sizeof(PCWSTR);
        stdinSet.streamIDs = stdinIds;

        stdoutIds[0] = WSMAN_STREAM_ID_STDOUT;
        stdoutIds[1] = WSMAN_STREAM_ID_STDERR;
        stdoutSet.streamIDsCount = sizeof(stdoutIds) / sizeof(PCWSTR);
        stdoutSet.streamIDs = stdoutIds;

        WSMAN_ENVIRONMENT_VARIABLE vars[] = { { L"var1", L"val1" }, { L"var2", L"val2" }};
        WSMAN_ENVIRONMENT_VARIABLE_SET varSet;
        varSet.varsCount = sizeof(vars)/sizeof(WSMAN_ENVIRONMENT_VARIABLE);
        varSet.vars = vars;

        WSMAN_SHELL_STARTUP_INFO  startupInfo;
        ZeroMemory(&startupInfo, sizeof(startupInfo));
        startupInfo.idleTimeoutMs = idleTimeout;
        startupInfo.inputStreamSet = &stdinSet;
        startupInfo.outputStreamSet = &stdoutSet;
        startupInfo.workingDirectory = L"c:\\";
        startupInfo.variableSet = &varSet;
        startupInfo.name = shellName;

        WSMAN_DATA inboundData;
        ZeroMemory(&inboundData, sizeof(inboundData));
        inboundData.type = WSMAN_DATA_TYPE_TEXT;
        inboundData.text.buffer = L"<test><test2>hello</test2><test3></test3></test>";
        inboundData.text.bufferLength = wcslen(inboundData.text.buffer);

        WSManCreateShell(session, 0, shellUri, &startupInfo, NULL, &inboundData, &createShellAsync, &shellHandle);
        
		if (shellHandle == NULL)
		{
			return false;
		}

		if (!WaitForSingleObject(shellCreated, INFINITE) == WAIT_OBJECT_0)
		{
			return false;
		}

		return true;
    }

    bool DeleteShell()
    {
        if (shellHandle)
        {
            WSMAN_SHELL_ASYNC deleteShellAsync;
            ZeroMemory(&deleteShellAsync, sizeof(WSMAN_SHELL_ASYNC));
            deleteShellAsync.operationContext = this;
            deleteShellAsync.completionFunction = (WSMAN_SHELL_COMPLETION_FUNCTION)ShellDeletedCallback;
            
            WSManCloseShell(shellHandle, 0, &deleteShellAsync);

			if (WaitForSingleObject(shellDeleted, INFINITE) != WAIT_OBJECT_0)
			{
				false;
			}
            shellHandle = NULL;
        }
        return true;
    }

    bool DisconnectShell()
    {
        if (shellHandle)
        {
            WSMAN_SHELL_ASYNC disconnectShellAsync;
            ZeroMemory(&disconnectShellAsync, sizeof(WSMAN_SHELL_ASYNC));
            disconnectShellAsync.operationContext = this;
            disconnectShellAsync.completionFunction = (WSMAN_SHELL_COMPLETION_FUNCTION)ShellDisconnectedCallback;
            
            WSMAN_SHELL_DISCONNECT_INFO disconnectInfo;
            disconnectInfo.idleTimeoutMs = 3*60*1000;
            WSManDisconnectShell(shellHandle, 0, &disconnectInfo, &disconnectShellAsync);

			if (WaitForSingleObject(shellDisconnected, INFINITE) != WAIT_OBJECT_0)
			{
				return false;
			}
        }
        return true;
    } 
    bool ReconnectShell()
    {
        if (shellHandle)
        {
            WSMAN_SHELL_ASYNC reconnectShellAsync;
            ZeroMemory(&reconnectShellAsync, sizeof(WSMAN_SHELL_ASYNC));
            reconnectShellAsync.operationContext = this;
            reconnectShellAsync.completionFunction = (WSMAN_SHELL_COMPLETION_FUNCTION)ShellReconnectedCallback;
            
            WSManReconnectShell(shellHandle, 0, &reconnectShellAsync);

			if (WaitForSingleObject(shellReconnected, INFINITE) != WAIT_OBJECT_0)
			{
				return false;
			}
        }
        return true;
    }

    bool ConnectShell(PCWSTR shellID)
    {
        WSMAN_SHELL_ASYNC connectShellAsync;
        ZeroMemory(&connectShellAsync, sizeof(WSMAN_SHELL_ASYNC));
        connectShellAsync.operationContext = this;
        connectShellAsync.completionFunction = (WSMAN_SHELL_COMPLETION_FUNCTION)ShellConnectedCallback;

        WSMAN_DATA inboundData;
        ZeroMemory(&inboundData, sizeof(inboundData));
        inboundData.type = WSMAN_DATA_TYPE_TEXT;
        inboundData.text.buffer = L"<test><test2>hello</test2><test3></test3></test>";
        inboundData.text.bufferLength = wcslen(inboundData.text.buffer);

        WSManConnectShell(session, 0, shellUri, shellID, NULL, &inboundData, &connectShellAsync, &shellHandle);

		if (shellHandle == NULL)
		{
			return false;
		}

		if (WaitForSingleObject(shellConnected, INFINITE) != WAIT_OBJECT_0)
		{
			return false;
		}
        
        return true;
    }

    bool CreateCommand(PCWSTR command)
    {
        WSMAN_SHELL_ASYNC createCommandAsync;
        ZeroMemory(&createCommandAsync, sizeof(createCommandAsync));
        createCommandAsync.operationContext = this;
        createCommandAsync.completionFunction = (WSMAN_SHELL_COMPLETION_FUNCTION)CommandCreatedCallback;

        WSManRunShellCommand(shellHandle, 0, command, NULL, NULL, &createCommandAsync, &commandHandle);

		if (commandHandle == NULL)
		{
			return false;
		}

		if (WaitForSingleObject(commandCreated, INFINITE) != WAIT_OBJECT_0)
		{
			return false;
		}

        return true;
    }

    bool ConnectCommand(PCWSTR commandId)
    {
        WSMAN_SHELL_ASYNC connectCommandAsync;
        ZeroMemory(&connectCommandAsync, sizeof(connectCommandAsync));
        connectCommandAsync.operationContext = this;
        connectCommandAsync.completionFunction = (WSMAN_SHELL_COMPLETION_FUNCTION)CommandConnectedCallback;

        WSMAN_DATA inboundData;
        ZeroMemory(&inboundData, sizeof(inboundData));
        inboundData.type = WSMAN_DATA_TYPE_TEXT;
        inboundData.text.buffer = L"<test><test2>hello</test2><test3></test3></test>";
        inboundData.text.bufferLength = wcslen(inboundData.text.buffer);


        WSManConnectShellCommand(shellHandle, 0, commandId, NULL, &inboundData, &connectCommandAsync, &commandHandle);

		if (commandHandle == NULL)
		{
			return false;
		}

		if (WaitForSingleObject(commandConnected, INFINITE) != WAIT_OBJECT_0)
		{
			return false;
		}
		
        return true;
    }

    bool DeleteCommand(DWORD expectedError)
    {
        if (commandHandle)
        {
            WSMAN_SHELL_ASYNC deleteCommandAsync;
            ZeroMemory(&deleteCommandAsync, sizeof(deleteCommandAsync));
            deleteCommandAsync.operationContext = this;
            deleteCommandAsync.completionFunction = (WSMAN_SHELL_COMPLETION_FUNCTION)CommandDeletedCallback;

            WSManCloseCommand(commandHandle, 0, &deleteCommandAsync);

			if (WaitForSingleObject(commandDeleted, INFINITE) == WAIT_OBJECT_0)
			{
				return false;
			}
            commandHandle = NULL;
        }

        return true;
    }

    bool Signal(PCWSTR signalURL)
    {
        WSMAN_SHELL_ASYNC signalAsync;
        ZeroMemory(&signalAsync, sizeof(signalAsync));
        signalAsync.operationContext = this;
        signalAsync.completionFunction = (WSMAN_SHELL_COMPLETION_FUNCTION)SignalCallback;

        WSManSignalShell(shellHandle, commandHandle, 0, signalURL, &signalAsync, &signalHandle);

		if (signalHandle == NULL)
		{
			return false;
		}

		if (WaitForSingleObject(signal, INFINITE) != WAIT_OBJECT_0)
		{
			return false;
		}

        return true;
    }

    bool CloseSignal(DWORD expectedError)
    {
        if (signalHandle)
        {
			if (!WSManCloseOperation(signalHandle, 0))
			{
				return false;
			}
            signalHandle = NULL;
        }

        return true;
    }

    bool QueueReceive()
    {
        WSMAN_SHELL_ASYNC receiveAsync;
        ZeroMemory(&receiveAsync, sizeof(receiveAsync));
        receiveAsync.operationContext = this;
        receiveAsync.completionFunction = (WSMAN_SHELL_COMPLETION_FUNCTION)ReceiveCallback;

        WSManReceiveShellOutput(shellHandle, commandHandle, 0, &stdoutSet, &receiveAsync, &receiveHandle);
        if (NULL == receiveHandle)
		{
			return false;
		}

        return true;
    }
    
    bool WaitAndCloseReceive()
    {
		//wait for receive operation to complete
		if (WaitForSingleObject(receive, INFINITE) != WAIT_OBJECT_0)
		{
			return false;
		}

        if (!WSManCloseOperation(receiveHandle, 0))
		{
			return false;
		}
        receiveHandle = NULL;
        return true;
    }

    bool Send(PCWSTR stream, WSMAN_DATA *streamData, BOOL endOfStream)
    {
        WSMAN_SHELL_ASYNC sendAsync;
        ZeroMemory(&sendAsync, sizeof(sendAsync));
        sendAsync.operationContext = this;
        sendAsync.completionFunction = (WSMAN_SHELL_COMPLETION_FUNCTION)SendCallback;

        WSManSendShellInput(shellHandle, commandHandle, 0, stream, streamData, endOfStream, &sendAsync, &sendHandle);
        if (NULL == sendHandle)
		{
			return false;
        }

		if (WaitForSingleObject(send, INFINITE) != WAIT_OBJECT_0)
		{
			return false;
		}

		if (!WSManCloseOperation(sendHandle, 0))
		{
			return false;
		}

        return true;
    }


    static VOID ShellCreatedCallback(
        _In_ PVOID operationContext,                     //user defined context
        DWORD flags,                                         // one or more flags from WSManCallbackFlags
        _In_ WSMAN_ERROR *error,                             // error allocated and owned by the winrm stack; valid in the callback only;
        _In_ WSMAN_SHELL_HANDLE shell,                      // shell handle associated with the user context 
        _In_opt_ WSMAN_COMMAND_HANDLE command,              // command handle associated with the user context  
        _In_opt_ WSMAN_OPERATION_HANDLE operationHandle,     // valid only for Send/Receive/Signal operations; must be closed using WSManCloseOperation  
        _In_opt_ WSMAN_RESPONSE_DATA *data             // output data from command/shell; allocated internally and owned by the winrm stack
                                                             // valid only within this function
        )
    {
        ShellClient *pThis = (ShellClient*) operationContext;
        pThis->shellCreatedError = error->code;
        pThis->shellCreatedCount++;
        SetEvent(pThis->shellCreated);
    }

    static VOID ShellDeletedCallback(
        _In_ PVOID operationContext,                     //user defined context
        DWORD flags,                                         // one or more flags from WSManCallbackFlags
        _In_ WSMAN_ERROR *error,                             // error allocated and owned by the winrm stack; valid in the callback only;
        _In_ WSMAN_SHELL_HANDLE shell,                       // shell handle associated with the user context 
        _In_opt_ WSMAN_COMMAND_HANDLE command,               // command handle associated with the user context  
        _In_opt_ WSMAN_OPERATION_HANDLE operationHandle,     // valid only for Send/Receive/Signal operations; must be closed using WSManCloseOperation  
        _In_opt_ WSMAN_RESPONSE_DATA *data             // output data from command/shell; allocated internally and owned by the winrm stack
                                                             // valid only within this function
        )
    {
        ShellClient *pThis = (ShellClient*) operationContext;
        pThis->shellDeletedError = error->code;
        pThis->shellDeletedCount++;
        SetEvent(pThis->shellDeleted);
    }

    static VOID ShellDisconnectedCallback(
        _In_ PVOID operationContext,                     //user defined context
        DWORD flags,                                         // one or more flags from WSManCallbackFlags
        _In_ WSMAN_ERROR *error,                             // error allocated and owned by the winrm stack; valid in the callback only;
        _In_ WSMAN_SHELL_HANDLE shell,                       // shell handle associated with the user context 
        _In_opt_ WSMAN_COMMAND_HANDLE command,               // command handle associated with the user context  
        _In_opt_ WSMAN_OPERATION_HANDLE operationHandle,     // valid only for Send/Receive/Signal operations; must be closed using WSManCloseOperation  
        _In_opt_ WSMAN_RESPONSE_DATA *data             // output data from command/shell; allocated internally and owned by the winrm stack
                                                             // valid only within this function
        )
    {
        ShellClient *pThis = (ShellClient*) operationContext;
        pThis->shellDisconnectedError = error->code;
        pThis->shellDisconnectedCount++;
        SetEvent(pThis->shellDisconnected);
    }

    static VOID ShellReconnectedCallback(
        _In_ PVOID operationContext,                     //user defined context
        DWORD flags,                                         // one or more flags from WSManCallbackFlags
        _In_ WSMAN_ERROR *error,                             // error allocated and owned by the winrm stack; valid in the callback only;
        _In_ WSMAN_SHELL_HANDLE shell,                       // shell handle associated with the user context 
        _In_opt_ WSMAN_COMMAND_HANDLE command,               // command handle associated with the user context  
        _In_opt_ WSMAN_OPERATION_HANDLE operationHandle,     // valid only for Send/Receive/Signal operations; must be closed using WSManCloseOperation  
        _In_opt_ WSMAN_RESPONSE_DATA *data             // output data from command/shell; allocated internally and owned by the winrm stack
                                                             // valid only within this function
        )
    {
        ShellClient *pThis = (ShellClient*) operationContext;
        pThis->shellReconnectedError = error->code;
        pThis->shellReconnectedCount++;
        SetEvent(pThis->shellReconnected);
    }

    static VOID ShellConnectedCallback(
        _In_ PVOID operationContext,                     //user defined context
        DWORD flags,                                         // one or more flags from WSManCallbackFlags
        _In_ WSMAN_ERROR *error,                             // error allocated and owned by the winrm stack; valid in the callback only;
        _In_ WSMAN_SHELL_HANDLE shell,                      // shell handle associated with the user context 
        _In_opt_ WSMAN_COMMAND_HANDLE command,              // command handle associated with the user context  
        _In_opt_ WSMAN_OPERATION_HANDLE operationHandle,     // valid only for Send/Receive/Signal operations; must be closed using WSManCloseOperation  
        _In_opt_ WSMAN_RESPONSE_DATA *data             // output data from command/shell; allocated internally and owned by the winrm stack
                                                             // valid only within this function
        )
    {
        ShellClient *pThis = (ShellClient*) operationContext;
        pThis->shellConnectedError = error->code;
        pThis->shellConnectedCount++;
        SetEvent(pThis->shellConnected);
    }

    static VOID CommandCreatedCallback(
        _In_ PVOID operationContext,                     //user defined context
        DWORD flags,                                         // one or more flags from WSManCallbackFlags
        _In_ WSMAN_ERROR *error,                             // error allocated and owned by the winrm stack; valid in the callback only;
        _In_ WSMAN_SHELL_HANDLE shell,                      // shell handle associated with the user context 
        _In_opt_ WSMAN_COMMAND_HANDLE command,              // command handle associated with the user context  
        _In_opt_ WSMAN_OPERATION_HANDLE operationHandle,     // valid only for Send/Receive/Signal operations; must be closed using WSManCloseOperation  
        _In_opt_ WSMAN_RESPONSE_DATA *data             // output data from command/shell; allocated internally and owned by the winrm stack
                                                             // valid only within this function
        )
    {
        ShellClient *pThis = (ShellClient*) operationContext;
        pThis->commandCreatedError = error->code;
        pThis->commandCreatedCount++;
        SetEvent(pThis->commandCreated);
    }

    static VOID CommandDeletedCallback(
        _In_ PVOID operationContext,                     //user defined context
        DWORD flags,                                         // one or more flags from WSManCallbackFlags
        _In_ WSMAN_ERROR *error,                             // error allocated and owned by the winrm stack; valid in the callback only;
        _In_ WSMAN_SHELL_HANDLE shell,             // shell handle associated with the user context 
        _In_opt_ WSMAN_COMMAND_HANDLE command,       // command handle associated with the user context  
        _In_opt_ WSMAN_OPERATION_HANDLE operationHandle,     // valid only for Send/Receive/Signal operations; must be closed using WSManCloseOperation  
        _In_opt_ WSMAN_RESPONSE_DATA *data             // output data from command/shell; allocated internally and owned by the winrm stack
                                                             // valid only within this function
        )
    {
        ShellClient *pThis = (ShellClient*) operationContext;
        pThis->commandDeletedError = error->code;
        pThis->commandDeletedCount++;
        SetEvent(pThis->commandDeleted);
    }

    static VOID CommandConnectedCallback(
        _In_ PVOID operationContext,                     //user defined context
        DWORD flags,                                         // one or more flags from WSManCallbackFlags
        _In_ WSMAN_ERROR *error,                             // error allocated and owned by the winrm stack; valid in the callback only;
        _In_ WSMAN_SHELL_HANDLE shell,                      // shell handle associated with the user context 
        _In_opt_ WSMAN_COMMAND_HANDLE command,              // command handle associated with the user context  
        _In_opt_ WSMAN_OPERATION_HANDLE operationHandle,     // valid only for Send/Receive/Signal operations; must be closed using WSManCloseOperation  
        _In_opt_ WSMAN_RESPONSE_DATA *data             // output data from command/shell; allocated internally and owned by the winrm stack
                                                             // valid only within this function
        )
    {
        ShellClient *pThis = (ShellClient*) operationContext;
        pThis->commandConnectedError = error->code;
        pThis->commandConnectedCount++;
        SetEvent(pThis->commandConnected);
    }

    static VOID SignalCallback(
        _In_ PVOID operationContext,                     //user defined context
        DWORD flags,                                         // one or more flags from WSManCallbackFlags
        _In_ WSMAN_ERROR *error,                             // error allocated and owned by the winrm stack; valid in the callback only;
        _In_ WSMAN_SHELL_HANDLE shell,             // shell handle associated with the user context 
        _In_opt_ WSMAN_COMMAND_HANDLE command,       // command handle associated with the user context  
        _In_opt_ WSMAN_OPERATION_HANDLE operationHandle,     // valid only for Send/Receive/Signal operations; must be closed using WSManCloseOperation  
        _In_opt_ WSMAN_RESPONSE_DATA *data             // output data from command/shell; allocated internally and owned by the winrm stack
                                                             // valid only within this function
        )
    {
        ShellClient *pThis = (ShellClient*) operationContext;
        pThis->signalError = error->code;
        pThis->signalCount++;
        SetEvent(pThis->signal);
    }

    static VOID ReceiveCallback(
        _In_ PVOID operationContext,                     //user defined context
        DWORD flags,                                         // one or more flags from WSManCallbackFlags
        _In_ WSMAN_ERROR *error,                             // error allocated and owned by the winrm stack; valid in the callback only;
        _In_ WSMAN_SHELL_HANDLE shell,             // shell handle associated with the user context 
        _In_opt_ WSMAN_COMMAND_HANDLE command,       // command handle associated with the user context  
        _In_opt_ WSMAN_OPERATION_HANDLE operationHandle,     // valid only for Send/Receive/Signal operations; must be closed using WSManCloseOperation  
        _In_opt_ WSMAN_RESPONSE_DATA *data             // output data from command/shell; allocated internally and owned by the winrm stack
                                                             // valid only within this function
        )
    {
        ShellClient *pThis = (ShellClient*) operationContext;
        pThis->receiveError = error->code;
        pThis->receiveCount++;
        if(data)
        {
            WSMAN_RECEIVE_DATA_RESULT* receivedata = &(data->receiveData);            
            //TASSERT(receivedata->streamData.type == WSMAN_DATA_TYPE_BINARY, L"receive binary data");
            if (pThis->receiveByteCount + receivedata->streamData.binaryData.dataLength < 1024) //receiveBuffer is max 1024 in size
            {
                memcpy(pThis->receiveBuffer + pThis->receiveByteCount, receivedata->streamData.binaryData.data, receivedata->streamData.binaryData.dataLength);
                pThis->receiveByteCount += receivedata->streamData.binaryData.dataLength;
            }
        }
        if (flags & WSMAN_FLAG_CALLBACK_END_OF_STREAM)
        {
            pThis->receiveEndOfStreamCount++;
        }
        if ((flags & WSMAN_FLAG_CALLBACK_END_OF_OPERATION) ||
            (pThis->receiveSignalEveryResult))
        {
            SetEvent(pThis->receive);
        }
    }

    static VOID SendCallback(
        _In_ PVOID operationContext,                     //user defined context
        DWORD flags,                                         // one or more flags from WSManCallbackFlags
        _In_ WSMAN_ERROR *error,                             // error allocated and owned by the winrm stack; valid in the callback only;
        _In_ WSMAN_SHELL_HANDLE shell,             // shell handle associated with the user context 
        _In_opt_ WSMAN_COMMAND_HANDLE command,       // command handle associated with the user context  
        _In_opt_ WSMAN_OPERATION_HANDLE operationHandle,     // valid only for Send/Receive/Signal operations; must be closed using WSManCloseOperation  
        _In_opt_ WSMAN_RESPONSE_DATA *data             // output data from command/shell; allocated internally and owned by the winrm stack
                                                             // valid only within this function
        )
    {
        ShellClient *pThis = (ShellClient*) operationContext;
        pThis->sendError = error->code;
        pThis->sendCount++;
        SetEvent(pThis->send);
    }

    WSMAN_SHELL_HANDLE shellHandle;
    HANDLE shellCreated;
    DWORD  shellCreatedCount;
    DWORD  shellCreatedError;

    HANDLE shellDeleted;
    DWORD  shellDeletedCount;
    DWORD  shellDeletedError;

    HANDLE shellDisconnected;
    DWORD  shellDisconnectedCount;
    DWORD  shellDisconnectedError;

    HANDLE shellReconnected;
    DWORD  shellReconnectedCount;
    DWORD  shellReconnectedError;

    HANDLE shellConnected;
    DWORD  shellConnectedCount;
    DWORD  shellConnectedError;

    WSMAN_COMMAND_HANDLE commandHandle;
    HANDLE commandCreated;
    DWORD  commandCreatedCount;
    DWORD  commandCreatedError;

    HANDLE commandDeleted;
    DWORD  commandDeletedCount;
    DWORD  commandDeletedError;

    HANDLE commandConnected;
    DWORD  commandConnectedCount;
    DWORD  commandConnectedError;

    WSMAN_OPERATION_HANDLE signalHandle;
    HANDLE signal;
    DWORD  signalCount;
    DWORD  signalError;

    WSMAN_OPERATION_HANDLE receiveHandle;
    HANDLE receive;
    DWORD  receiveCount;
    DWORD  receiveError;
    bool   receiveEndOfStreamCount;
    bool   receiveSignalEveryResult;
    BYTE   receiveBuffer[1024];
    DWORD  receiveByteCount;

    WSMAN_OPERATION_HANDLE sendHandle;
    HANDLE send;
    DWORD  sendCount;
    DWORD  sendError;

    WSMAN_API_HANDLE apiHandle;
    WSMAN_SESSION_HANDLE session;
    DWORD idleTimeout;
    PCWSTR shellUri;
    WSMAN_STREAM_ID_SET stdoutSet;
    PCWSTR stdoutIds[2];
    PCWSTR shellName;
};


