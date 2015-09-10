// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "common.h"

// This header file contains all definitions used by both client and server services.

// Size of each file access when serializing the file into messages.
// A bigger number here results in fewer file reads while a smaller number reduces
// memory consumption. This is a sub-chunking of the chunk size set by the user
// when starting the services. If the chunk size set then is bigger than FILE_CHUNK
// a chunk is read in more than one file access.
#define FILE_CHUNK 131072

#define DISCOVERY_REQUEST -1

// Error Uris used to transmit errors from server service to client service.
namespace GlobalStrings
{
    static const WCHAR noError[] = L"http://tempuri.org/FileRep/NoError";
    static const WCHAR serializationFailed[] = L"http://tempuri.org/FileRep/SerializationFailed";
    static const WCHAR invalidFileName[] = L"http://tempuri.org/FileRep/InvalidFileName";
    static const WCHAR unableToDetermineFileLength[] = L"http://tempuri.org/FileRep/UnableToDetermineFileLength";
    static const WCHAR invalidRequest[] = L"http://tempuri.org/FileRep/InvalidRequest";
    static const WCHAR outOfRange[] = L"http://tempuri.org/FileRep/OutOfRange";
    static const WCHAR unableToSetFilePointer[] = L"http://tempuri.org/FileRep/UnableToSetFilePointer";
}

class CChannelManager;
class CRequest;

typedef enum
{
    REPORT_ERROR = 1,
    REPORT_INFO = 2,
    REPORT_VERBOSE = 3,
} REPORTING_LEVEL;

typedef enum
{
    INVALID_REQUEST = 1,
    FILE_DOES_NOT_EXIST = 2,
    FAILED_TO_CREATE_FILE,

} FAULT_TYPE;

// This class is the base class for both the client and server service.
class CFileRep
{
public:
    CFileRep(
        _In_ REPORTING_LEVEL errorReporting,
        _In_ long maxChannels,
        _In_ TRANSPORT_MODE transport,
        _In_ SECURITY_MODE security,
        _In_ MESSAGE_ENCODING encoding);

    ~CFileRep();

    HRESULT Start(
        _In_reads_(uriLength) const LPWSTR uri,
        _In_ DWORD uriLength);

    // A stop resets all custom state and releases all resources associated with a running instance of the service.
    HRESULT Stop();

    inline bool IsRunning() { return started; }

    WS_LISTENER* GetListener() { return listener; }

    CChannelManager* GetChannelManager() { return channelManager; }

    virtual HRESULT ProcessMessage(
        _In_ CRequest* request,
        _In_ const WS_XML_STRING* receivedAction) = 0;

    // These have to be public since things like thread creation can fail outside of the class.
    void PrintVerbose(
        _In_z_ const WCHAR message[]);

    void PrintInfo(
        _In_z_ const WCHAR message[]);

    void PrintError(
        _In_ HRESULT hr,
        _In_opt_ WS_ERROR* error,
        _In_ bool displayAlways);

    void PrintError(
        _In_z_ const WCHAR message[],
        _In_ bool displayAlways);

    // The default maximum heap size is set very conservatively to protect against headers of excessive size.
    // Since we are sending large chunks of data round that is not sufficient for us.
    static WS_MESSAGE_PROPERTY CreateHeapProperty()
    {
        static SIZE_T heapSize = MAXMESSAGESIZE * 2;

        static WS_HEAP_PROPERTY heapPropertyArray[] =
        {
            { WS_HEAP_PROPERTY_MAX_SIZE, &heapSize, sizeof(heapSize) }
        };

        static WS_HEAP_PROPERTIES heapProperties =
        {
            heapPropertyArray, WsCountOf(heapPropertyArray)
        };

        WS_MESSAGE_PROPERTY ret;
        ret.id = WS_MESSAGE_PROPERTY_HEAP_PROPERTIES;
        ret.value = &heapProperties;
        ret.valueSize = sizeof(heapProperties);
        return ret;
    }

    void GetEncoding(
        _Out_writes_to_(1, *propertyCount) WS_ENCODING* encodingProperty,
        _Out_ ULONG* propertyCount);

protected:
    HRESULT InitializeListener();

    WS_STRING uri;
    volatile bool started;
    REPORTING_LEVEL errorReporting;
    long maxChannels;
    TRANSPORT_MODE transportMode;
    MESSAGE_ENCODING encoding;
    SECURITY_MODE securityMode;
    WS_LISTENER* listener;
    CChannelManager* channelManager;
};

// Manages the channels and related state to maximize reuse of structures and performance.
// If there are less channels that are ready to accept a request (called idle channels)
// than minIdleChannels then we create a new channel and make it listen for incoming requests.
// If there are more idle channels than maxIdleChannels then we destroy the next channel that
// becomes idle. There are never more than maxTotalChannels channels overall.
// The reason for having a minimum number of idle channels is that otherwise there would be a
// bottleneck when multiple requests come in simultaniously as channel creation takes some time.
// The reason for having a maximum number of idle channels is to limit resource consumption.
// Resource reuse is an important performance booster. Resetting a data structure is a lot cheaper
// than destroying and recreating it later, which is why there are so many reset APIs.
// However, never destroying resources can also be problematic as you can potentially hold on
// to significant resources much longer than neccessary. This class, using the algorithm described
// above, tries to find a middle ground.
// If the last issue is not a concern, a simpler and most likely superior implementation is to
// simply create as many channels and associated resources as needed and to reuse them perpetually
// in their own loop.
class CChannelManager
{
public:
    CChannelManager(
        _In_ CFileRep* server,
        _In_ long minIdleChannels,
        _In_ long maxIdleChannels,
        _In_ long maxTotalChannels);

    ~CChannelManager();

    HRESULT Initialize();

    HRESULT CreateChannels();

    static ULONG WINAPI CreateChannelWorkItem(
        _In_ void* state);

    void CreateChannel(
        _In_ CRequest* request);

    void ChannelCreated();

    void ChannelInUse();

    void ChannelFreed();

    void ChannelIdle();

    static void CALLBACK CleanupCallback(
        _In_ HRESULT hr,
        _In_ WS_CALLBACK_MODEL callbackModel,
        _In_ void* state);

    void Stop();

    void WaitForCleanup();

    inline long GetChannelCount() { return totalChannels; }

    inline bool IsRunning() { return running; }

    inline bool ShouldDestroyChannel() { return (!IsRunning() || idleChannels > maxIdleChannels); }

private:
    inline void PrintVerbose(
        _In_z_ const WCHAR message[])
    {
        server->PrintVerbose(message);
    }

    long minIdleChannels;
    long maxIdleChannels;
    long maxTotalChannels;

    long idleChannels;
    long activeChannels;
    long totalChannels;

    CFileRep* server;
    HANDLE stopEvent;
    volatile bool running;

#if (DBG || _DEBUG)
    bool initialized;
#endif
};

// This class implements the part of the message processing loop that is common to the client and server service.
// It contains the per-channel state as well as the common processing methods.
class CRequest
{
public:
    CRequest(
        _In_ CFileRep* server);

    ~CRequest();

    HRESULT Initialize();

    // Static callback methods. They delegate to their non-static counterparts.
    static HRESULT CALLBACK ResetChannelCallback(
        _In_ HRESULT hr,
        _In_ WS_CALLBACK_MODEL callbackModel,
        _In_ void* callbackState,
        _Inout_ WS_ASYNC_OPERATION* next,
        _In_opt_ const WS_ASYNC_CONTEXT* asyncContext,
        _In_opt_ WS_ERROR* error);

    static HRESULT CALLBACK AcceptChannelCallback(
        _In_ HRESULT hr,
        _In_ WS_CALLBACK_MODEL callbackModel,
        _In_ void* callbackState,
        _Inout_ WS_ASYNC_OPERATION* next,
        _In_opt_ const WS_ASYNC_CONTEXT* asyncContext,
        _In_opt_ WS_ERROR* error);

    static HRESULT CALLBACK ReceiveFirstMessageCallback(
        _In_ HRESULT hr,
        _In_ WS_CALLBACK_MODEL callbackModel,
        _In_ void* callbackState,
        _Inout_ WS_ASYNC_OPERATION* next,
        _In_opt_ const WS_ASYNC_CONTEXT* asyncContext,
        _In_opt_ WS_ERROR* error);

    static HRESULT CALLBACK ReceiveMessageCallback(
        _In_ HRESULT hr,
        _In_ WS_CALLBACK_MODEL callbackModel,
        _In_ void* callbackState,
        _Inout_ WS_ASYNC_OPERATION* next,
        _In_opt_ const WS_ASYNC_CONTEXT* asyncContext,
        _In_opt_ WS_ERROR* error);

    static HRESULT CALLBACK ReadHeaderCallback(
        _In_ HRESULT hr,
        _In_ WS_CALLBACK_MODEL callbackModel,
        _In_ void* callbackState,
        _Inout_ WS_ASYNC_OPERATION* next,
        _In_opt_ const WS_ASYNC_CONTEXT* asyncContext,
        _In_opt_ WS_ERROR* error);

    static HRESULT CALLBACK CloseChannelCallback(
        _In_ HRESULT hr,
        _In_ WS_CALLBACK_MODEL callbackModel,
        _In_ void* callbackState,
        _Inout_ WS_ASYNC_OPERATION* next,
        _In_opt_ const WS_ASYNC_CONTEXT* asyncContext,
        _In_opt_ WS_ERROR* error);

    static HRESULT CALLBACK RequestCompleteCallback(
        _In_ HRESULT hr,
        _In_ WS_CALLBACK_MODEL callbackModel,
        _In_ void* callbackState,
        _Inout_ WS_ASYNC_OPERATION* next,
        _In_opt_ const WS_ASYNC_CONTEXT* asyncContext,
        _In_opt_ WS_ERROR* error);

    static HRESULT CALLBACK HandleFailureCallback(
        _In_ HRESULT hr,
        _In_ WS_CALLBACK_MODEL callbackModel,
        _In_ void* callbackState,
        _Inout_ WS_ASYNC_OPERATION* next,
        _In_opt_ const WS_ASYNC_CONTEXT* asyncContext,
        _In_opt_ WS_ERROR* error);

    // The non-static counterparts that actually do the work.
    HRESULT ResetChannel(
        _In_ HRESULT hr,
        _Inout_ WS_ASYNC_OPERATION* next,
        _In_ WS_CALLBACK_MODEL callbackModel,
        _In_opt_ WS_ERROR* error);

    HRESULT AcceptChannel(
        _In_ HRESULT hr,
        _Inout_ WS_ASYNC_OPERATION* next,
        _In_ WS_CALLBACK_MODEL callbackModel,
        _In_opt_ const WS_ASYNC_CONTEXT* asyncContext,
        _In_opt_ WS_ERROR* error);

    HRESULT ReceiveFirstMessage(
        _In_ HRESULT hr,
        _Inout_ WS_ASYNC_OPERATION* next,
        _In_ WS_CALLBACK_MODEL callbackModel);

    HRESULT ReceiveMessage(
        _In_ HRESULT hr,
        _Inout_ WS_ASYNC_OPERATION* next,
        _In_ WS_CALLBACK_MODEL callbackModel,
        _In_opt_ const WS_ASYNC_CONTEXT* asyncContext,
        _In_opt_ WS_ERROR* error);

    HRESULT ReadHeader(
        _In_ HRESULT hr,
        _Inout_ WS_ASYNC_OPERATION* next,
        _In_ WS_CALLBACK_MODEL callbackModel,
        _In_opt_ WS_ERROR* error);

    HRESULT CloseChannel(
        _In_ HRESULT hr,
        _Inout_ WS_ASYNC_OPERATION* next,
        _In_ WS_CALLBACK_MODEL callbackModel,
        _In_opt_ const WS_ASYNC_CONTEXT* asyncContext,
        _In_opt_ WS_ERROR* error);

    HRESULT RequestComplete(
        _In_ HRESULT hr,
        _Inout_ WS_ASYNC_OPERATION* next);

    HRESULT HandleFailure(
        _In_ HRESULT hr,
        _Inout_ WS_ASYNC_OPERATION* next,
        _In_opt_ WS_ERROR* error);


    WS_CHANNEL* GetChannel() { return channel; }

    WS_MESSAGE* GetRequestMessage() { return requestMessage; }

    WS_MESSAGE* GetReplyMessage() { return replyMessage; }

    WS_ERROR* GetError() { return error; }

    CFileRep* GetServer() { return server; }

    // We use SOAP faults to communicate to the tool errors that might be expected
    // during execution, such as file not found. For critical errors we abort the channel.
    // For internal communication between the services we use status messages that are part
    // of the regular message exchange instead of faults. We could use faults there as well,
    // but we do not want to fault internal communication because for example the user
    // requested an invalid file.
    HRESULT SendFault(
        _In_ FAULT_TYPE faultType);

    static inline CRequest* GetRequest(
        _In_ void* callbackState);

    WS_ASYNC_STATE asyncState;

private:
    inline void PrintVerbose(
        _In_z_ const WCHAR message[])
    {
        server->PrintVerbose(message);
    }

    // State associated with a request. Except the CFileRep pointer, this is not application specific.
    WS_CHANNEL* channel;
    WS_MESSAGE* requestMessage;
    WS_MESSAGE* replyMessage;
    WS_ERROR* error;
    CFileRep* server;
    bool channelInUse;
};


// Server service.
class CFileRepServer : public CFileRep
{
public:
    CFileRepServer(
        _In_ REPORTING_LEVEL errorReporting,
        _In_ DWORD maxChannels,
        _In_ TRANSPORT_MODE transport,
        _In_ SECURITY_MODE security,
        _In_ MESSAGE_ENCODING encoding,
        _In_ DWORD chunkSize) : CFileRep(
            errorReporting,
            maxChannels,
            transport,
            security,
            encoding)
    {
        this->chunkSize = chunkSize;
    }

    HRESULT ProcessMessage(
        _In_ CRequest* request,
        _In_ const WS_XML_STRING* receivedAction);

protected:

    HRESULT SendError(
        _In_ CRequest* request,
        _In_z_ const WCHAR errorMessage[]);

    HRESULT ReadAndSendFile(
        _In_ CRequest* request,
        _In_ const LPWSTR fileName,
        _In_ LONGLONG chunkPosition,
        _In_opt_ WS_ERROR* error);

    HRESULT SendFileInfo(
        _In_ CRequest* requeste,
        _In_z_ const LPWSTR fileName,
        _In_ LONGLONG llFileLength,
        _In_ DWORD chunkSize);

    HRESULT ReadAndSendChunk(
        _In_ CRequest* request,
        _In_ long chunkSize,
        _In_ LONGLONG chunkPosition,
        _In_ HANDLE file);

    long chunkSize;
};

// Client service.
class CFileRepClient : public CFileRep
{
public:
    CFileRepClient(
        _In_ REPORTING_LEVEL errorReporting,
        _In_ DWORD maxChannels,
        _In_ TRANSPORT_MODE transport,
        _In_ SECURITY_MODE security,
        _In_ MESSAGE_ENCODING encoding) : CFileRep(
            errorReporting,
            maxChannels,
            transport,
            security,
            encoding)
    {
    }

    HRESULT ProcessMessage(
        _In_ CRequest* request,
        _In_ const WS_XML_STRING* receivedAction);

protected:
    HRESULT SendUserResponse(
        _In_ CRequest* request,
        _In_ TRANSFER_RESULTS result);

    HRESULT ProcessUserRequest(
        _In_ CRequest* request,
        _In_z_ const LPWSTR sourcePath,
        _In_z_ const LPWSTR destinationPath,
        _In_z_ const LPWSTR serverUri,
        _In_ TRANSPORT_MODE transportMode,
        _In_ SECURITY_MODE securityMode,
        _In_ MESSAGE_ENCODING encoding,
        _In_ REQUEST_TYPE requestType);

    HRESULT CreateServerChannel(
        _In_ MESSAGE_ENCODING serverEncoding,
        _In_ TRANSPORT_MODE serverTransportMode,
        _In_ SECURITY_MODE serverSecurityMode,
        _In_opt_ WS_ERROR* error,
        _Outptr_ WS_CHANNEL** channel);

    HRESULT ExtendFile(
        _In_ HANDLE file,
        _In_ LONGLONG length);

    HRESULT ProcessChunk(
        _In_ long chunkSize,
        _In_ HANDLE file,
        _In_ LONGLONG fileLength,
        _In_ WS_MESSAGE* requestMessage,
        _In_ WS_MESSAGE* replyMessage,
        _In_ WS_CHANNEL* channel,
        _In_opt_ WS_ERROR* error,
        _In_ FileRequest* request);

    HRESULT DeserializeAndWriteMessage(
        _In_ WS_MESSAGE* message,
        _In_ long chunkSize,
        _Out_ LONGLONG* chunkPosition,
        _Out_ long* contentLength,
        _In_ HANDLE file);
};

// Helper functions.
void PrintError(
    _In_ HRESULT errorCode,
    _In_opt_ WS_ERROR* error);

HRESULT ParseTransport(
    _In_z_ const LPWSTR url,
    _Out_ TRANSPORT_MODE* transport,
    _Out_ SECURITY_MODE* securityMode);

void CleanupChannel(
    _In_opt_ WS_CHANNEL* channel);


