#include <Windows.h>
#include <Websocket.h>
#include <Assert.h>
#include <StdIo.h>
#include "Transport.h"

void DumpHeaders(
    _In_reads_(headerCount) WEB_SOCKET_HTTP_HEADER* headers,
    _In_ ULONG headerCount)
{
    for (ULONG i = 0; i < headerCount; i++)
    {
        wprintf(L"%.*S: %.*S\n", headers[i].ulNameLength, headers[i].pcName, headers[i].ulValueLength, headers[i].pcValue);
    }
}

void DumpData(
    _In_reads_bytes_opt_(dataLength) BYTE* data,
    _In_ ULONG dataLength)
{
    if (data == NULL)
    {
        return;
    }

    for (ULONG i = 0; i < dataLength; i++)
    {
        wprintf(L"0x%x ", data[i]);
    }

    wprintf(L"\n");
}

HRESULT Initialize(
    _Out_ WEB_SOCKET_HANDLE* outClientHandle,
    _Out_ WEB_SOCKET_HANDLE* outServerHandle)
{
    HRESULT hr = S_OK;
    WEB_SOCKET_HANDLE clientHandle = NULL;
    WEB_SOCKET_HANDLE serverHandle = NULL;

    // Create a client side websocket handle.
    hr = WebSocketCreateClientHandle(NULL, 0, &clientHandle);
    if (FAILED(hr))
    {
        goto quit;
    }

    // Create a server side websocket handle.
    hr = WebSocketCreateServerHandle(NULL, 0, &serverHandle);
    if (FAILED(hr))
    {
        goto quit;
    }

    *outClientHandle = clientHandle;
    *outServerHandle = serverHandle;
    clientHandle = NULL;
    serverHandle = NULL;

quit:

    if (clientHandle != NULL)
    {
        WebSocketDeleteHandle(clientHandle);
        clientHandle = NULL;
    }

    if (serverHandle != NULL)
    {
        WebSocketDeleteHandle(serverHandle);
        serverHandle = NULL;
    }

    return hr;
}

HRESULT PerformHandshake(
    _In_ WEB_SOCKET_HANDLE clientHandle,
    _In_ WEB_SOCKET_HANDLE serverHandle)
{
    HRESULT hr = S_OK;
    ULONG clientAdditionalHeaderCount = 0;
    WEB_SOCKET_HTTP_HEADER* clientAdditionalHeaders = NULL;
    ULONG serverAdditionalHeaderCount = 0;
    WEB_SOCKET_HTTP_HEADER* serverAdditionalHeaders = NULL;
    ULONG clientHeaderCount = 0;
    WEB_SOCKET_HTTP_HEADER* clientHeaders = NULL;

    // Static "Host" header.
    const static WEB_SOCKET_HTTP_HEADER host =
    {
        "Host",
        ARRAYSIZE("Host") - 1, // Length of "Host" string without the NULL-terminator.
        "localhost",
        ARRAYSIZE("localhost") - 1 // Length of "localhost" string without the NULL-terminator.
    };

    // Start a client side of the handshake - 'additionalHeaders' will hold an array of websocket specific headers.
    // Production applications must add these headers to the outgoing HTTP request.
    hr = WebSocketBeginClientHandshake(
        clientHandle,
        NULL,
        0,
        NULL,
        0,
        NULL,
        0,
        &clientAdditionalHeaders,
        &clientAdditionalHeaderCount);
    if (FAILED(hr))
    {
        goto quit;
    }

    // Concatenate list of headers that the HTTP stack must send (the Host header) with
    // a list returned by WebSocketBeginClientHandshake.
    clientHeaderCount = clientAdditionalHeaderCount + 1;
    clientHeaders = new WEB_SOCKET_HTTP_HEADER[clientHeaderCount];
    if (clientHeaders == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto quit;
    }

    CopyMemory(clientHeaders, clientAdditionalHeaders, clientAdditionalHeaderCount * sizeof(WEB_SOCKET_HTTP_HEADER));
    clientHeaders[clientAdditionalHeaderCount] = host;

    wprintf(L"-- Client side headers that need to be send with a request --\n");
    DumpHeaders(clientHeaders, clientHeaderCount);

    // Start a server side of the handshake. Production applications must parse the incoming
    // HTTP request and pass all headers to the function. The function will return an array websocket
    // specific headers that must be added to the outgoing HTTP response.
    hr = WebSocketBeginServerHandshake(
        serverHandle,
        NULL,
        NULL,
        0,
        clientHeaders,
        clientHeaderCount,
        &serverAdditionalHeaders,
        &serverAdditionalHeaderCount);
    if (FAILED(hr))
    {
        goto quit;
    }

    wprintf(L"\n-- Server side headers that need to be send with a response --\n");
    DumpHeaders(serverAdditionalHeaders, serverAdditionalHeaderCount);

    // Finish handshake. Once the client/server handshake is completed, memory allocated by
    // the *Begin* functions is reclaimed and must not be used by the application.
    hr = WebSocketEndClientHandshake(
        clientHandle,
        serverAdditionalHeaders,
        serverAdditionalHeaderCount,
        NULL,
        0,
        NULL);
    if (FAILED(hr))
    {
        goto quit;
    }

    hr = WebSocketEndServerHandshake(serverHandle);
    if (FAILED(hr))
    {
        wprintf(L"4\n");
        goto quit;
    }

quit:

    if (clientHeaders != NULL)
    {
        delete[] clientHeaders;
        clientHeaders = NULL;
    }

    return hr;
}

HRESULT RunLoop(
    _In_ WEB_SOCKET_HANDLE handle,
    _In_ Transport* transport)
{
    HRESULT hr = S_OK;
    WEB_SOCKET_BUFFER buffers[2] = {0};
    ULONG bufferCount;
    ULONG bytesTransferred;
    WEB_SOCKET_BUFFER_TYPE bufferType;
    WEB_SOCKET_ACTION action;
    PVOID actionContext;

    do
    {
        // Initialize variables that change with every loop revolution.
        bufferCount = ARRAYSIZE(buffers);
        bytesTransferred = 0;

        // Get an action to process.
        hr = WebSocketGetAction(
            handle,
            WEB_SOCKET_ALL_ACTION_QUEUE,
            buffers,
            &bufferCount,
            &action,
            &bufferType,
            NULL,
            &actionContext);
        if (FAILED(hr))
        {
            // If we cannot get an action, abort the handle but continue processing until all operations are completed.
            WebSocketAbortHandle(handle);
        }

        switch (action)
        {
            case WEB_SOCKET_NO_ACTION:

                // No action to perform - just exit the loop.
                break;

            case WEB_SOCKET_RECEIVE_FROM_NETWORK_ACTION:

                wprintf(L"Receiving data from a network:\n");

                assert(bufferCount >= 1);
                for (ULONG i = 0; i < bufferCount; i++)
                {
                    // Read data from a transport (in production application this may be a socket).
                    hr = transport->ReadData(buffers[i].Data.ulBufferLength, &bytesTransferred, buffers[i].Data.pbBuffer);
                    if (FAILED(hr))
                    {
                        break;
                    }

                    DumpData(buffers[i].Data.pbBuffer, bytesTransferred);

                    // Exit the loop if there were not enough data to fill this buffer.
                    if (buffers[i].Data.ulBufferLength > bytesTransferred)
                    {
                        break;
                    }
                }

                break;

            case WEB_SOCKET_INDICATE_RECEIVE_COMPLETE_ACTION:

                wprintf(L"Receive operation completed with a buffer:\n");

                if (bufferCount != 1)
                {
                    assert(!"This should never happen.");
                    hr = E_FAIL;
                    goto quit;
                }

                DumpData(buffers[0].Data.pbBuffer, buffers[0].Data.ulBufferLength);

                break;

            case WEB_SOCKET_SEND_TO_NETWORK_ACTION:

                wprintf(L"Sending data to a network:\n");

                for (ULONG i = 0; i < bufferCount; i++)
                {
                    DumpData(buffers[i].Data.pbBuffer, buffers[i].Data.ulBufferLength);

                    // Write data to a transport (in production application this may be a socket).
                    hr = transport->WriteData(buffers[i].Data.pbBuffer, buffers[i].Data.ulBufferLength);
                    if (FAILED(hr))
                    {
                        break;
                    }

                    bytesTransferred += buffers[i].Data.ulBufferLength;
                }
                break;


            case WEB_SOCKET_INDICATE_SEND_COMPLETE_ACTION:

                wprintf(L"Send operation completed\n");
                break;

            default:

                // This should never happen.
                assert(!"Invalid switch");
                hr = E_FAIL;
                goto quit;
        }

        if (FAILED(hr))
        {
            // If we failed at some point processing actions, abort the handle but continue processing
            // until all operations are completed.
            WebSocketAbortHandle(handle);
        }

        // Complete the action. If application performs asynchronous operation, the action has to be
        // completed after the async operation has finished. The 'actionContext' then has to be preserved
        // so the operation can complete properly.
        WebSocketCompleteAction(handle, actionContext, bytesTransferred);
    }
    while (action != WEB_SOCKET_NO_ACTION);

quit:
    return hr;
}

HRESULT PerformDataExchange(
    _In_ WEB_SOCKET_HANDLE clientHandle,
    _In_ WEB_SOCKET_HANDLE serverHandle,
    _In_ Transport* transport)
{
    HRESULT hr = S_OK;
    BYTE dataToSend[] = {'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd'};
    WEB_SOCKET_BUFFER buffer;

    buffer.Data.pbBuffer = dataToSend;
    buffer.Data.ulBufferLength = ARRAYSIZE(dataToSend);

    wprintf(L"\n-- Queueing a send with a buffer --\n");
    DumpData(buffer.Data.pbBuffer, buffer.Data.ulBufferLength);

    hr = WebSocketSend(clientHandle, WEB_SOCKET_UTF8_MESSAGE_BUFFER_TYPE, &buffer, NULL);
    if (FAILED(hr))
    {
        goto quit;
    }

    hr = RunLoop(clientHandle, transport);
    if (FAILED(hr))
    {
        goto quit;
    }

    wprintf(L"\n-- Queueing a receive --\n");

    hr = WebSocketReceive(serverHandle, NULL, NULL);
    if (FAILED(hr))
    {
        goto quit;
    }

    hr = RunLoop(serverHandle, transport);
    if (FAILED(hr))
    {
        goto quit;
    }

quit:

    return hr;
}

int __cdecl wmain()
{
    HRESULT hr = S_OK;
    WEB_SOCKET_HANDLE clientHandle = NULL;
    WEB_SOCKET_HANDLE serverHandle = NULL;
    Transport transport;

    hr = Initialize(&clientHandle, &serverHandle);
    if (FAILED(hr))
    {
        goto quit;
    }

    hr = PerformHandshake(clientHandle, serverHandle);
    if (FAILED(hr))
    {
        goto quit;
    }

    hr = PerformDataExchange(clientHandle, serverHandle, &transport);
    if (FAILED(hr))
    {
        goto quit;
    }

quit:

    if (clientHandle != NULL)
    {
        WebSocketDeleteHandle(clientHandle);
        clientHandle = NULL;
    }

    if (serverHandle != NULL)
    {
        WebSocketDeleteHandle(serverHandle);
        serverHandle = NULL;
    }

    if (FAILED(hr))
    {
        wprintf(L"Websocket failed with error 0x%x\n", hr);
        return 0;
    }

    return 1;
}
