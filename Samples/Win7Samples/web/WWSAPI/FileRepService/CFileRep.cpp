//------------------------------------------------------------
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------

#include "Service.h"
#include "strsafe.h"
#include "stdlib.h"
#include "intsafe.h"
#include "assert.h"

// Currently we only support 3 modes: TCP, HTTP and HTTP with SSL. Thus, this simple check here is
// enough to figure out the proper configuration. This will not be enough once we have more advanced
// security settings. The wire protocol makes these settings explicit so that it does not have
// to be changed in that case. This is only used for the command line.
// For more advanced parsing, WsDecodeUrl should be used.
HRESULT ParseTransport(
    __in_z const LPWSTR url, 
    __out TRANSPORT_MODE* transport, 
    __out SECURITY_MODE* securityMode)
{
    if (wcsstr(url, L"http:") == url)
    {
        *transport = HTTP_TRANSPORT;    
        *securityMode = NO_SECURITY;
        return S_OK;
    }
    else if (wcsstr(url, L"https:") == url)
    {
        *transport = HTTP_TRANSPORT;
        *securityMode = SSL_SECURITY;
        return S_OK;
    }
    else if (wcsstr(url, L"net.tcp:") == url)
    {
        *transport = TCP_TRANSPORT;
        *securityMode = NO_SECURITY;
        return S_OK;
    }

    return E_FAIL;
}

// Print out rich error info
void PrintError(
    __in HRESULT errorCode, 
    __in_opt WS_ERROR* error)
{
    wprintf(L"Failure: errorCode=0x%lx\n", errorCode);

    if (errorCode == E_INVALIDARG || errorCode == WS_E_INVALID_OPERATION)
    {
        // Correct use of the APIs should never generate these errors
        wprintf(L"The error was due to an invalid use of an API.  This is likely due to a bug in the program.\n");
        DebugBreak();
    }

    HRESULT hr = S_OK;
    if (error != NULL)
    {
        ULONG errorCount;
        hr = WsGetErrorProperty(error, WS_ERROR_PROPERTY_STRING_COUNT, &errorCount, sizeof(errorCount));
        if (FAILED(hr))
        {
            goto Exit;
        }
        for (ULONG i = 0; i < errorCount; i++)
        {
            WS_STRING string;
            hr = WsGetErrorString(error, i, &string);
            if (FAILED(hr))
            {
                goto Exit;
            }
            wprintf(L"%.*s\n", string.length, string.chars);
        }
    }
Exit:
    if (FAILED(hr))
    {
        wprintf(L"Could not get error string (errorCode=0x%lx)\n", hr);
    }
}

CFileRep::CFileRep(
    __in REPORTING_LEVEL errorReporting, 
    __in long maxChannels, 
    __in TRANSPORT_MODE transport, 
    __in SECURITY_MODE security, 
    __in MESSAGE_ENCODING encoding)
{
    assert(maxChannels >=1);
    this->errorReporting = errorReporting;
    this->maxChannels = maxChannels;
    
    this->started = false;
    this->transportMode = transport;
    this->encoding = encoding;
    this->securityMode = security;

    this->listener = NULL;
    this->channelManager = NULL;
}

CFileRep::~CFileRep()
{
    Stop();    
}

HRESULT CFileRep::Start(
    __in_ecount(uriLength) const LPWSTR uri, 
    __in DWORD uriLength)
{    
    PrintVerbose(L"Entering CFileRep::Start");
    assert(!started); // Should not be called twice without calling stop first.

    if (started)
    {
        PrintVerbose(L"Leaving CFileRep::Start");
        return S_FALSE;
    }

    HRESULT hr = S_OK;

    DWORD newLength = 0;
    DWORD bytes = 0;

    IfFailedExit(DWordAdd(uriLength, 1, &newLength));
    IfFailedExit(DWordMult(newLength, sizeof (WCHAR), &bytes));        

    // Make local copy of the URI.
    this->uri.chars = (LPWSTR)HeapAlloc(GetProcessHeap(), 0, bytes);
    IfNullExit(this->uri.chars);

    IfFailedExit(StringCchCopyNW(this->uri.chars, newLength, uri, uriLength));
    
    this->uri.length = uriLength;

    // We are ready to start listening.
    started = true;
    IfFailedExit(InitializeListener());

    PrintInfo(L"Service startup succeeded.\n");
    PrintVerbose(L"Leaving CFileRep::Start");    

    return S_OK;

    
    ERROR_EXIT

    PrintError(L"Service startup failed.\n", true);        
        
    Stop();  

    if (NULL != this->uri.chars)
    {
        HeapFree(GetProcessHeap(), 0, this->uri.chars);
        this->uri.chars = NULL;
    } 
    
    PrintVerbose(L"Leaving CFileRep::Start");    
    return hr;
}

// We are told to stop the service. Clean up all state.
HRESULT CFileRep::Stop()
{
    PrintVerbose(L"Entering CFileRep::Stop");

    if (!started)
    {
        PrintVerbose(L"Leaving CFileRep::Stop");
        return S_FALSE;
    }

    started = false;
    if (NULL != channelManager)
    {
        channelManager->Stop();
    }

    PrintInfo(L"Shutdown signaled.");

    if (NULL != listener)
    {
        // This aborts the main service loop that waits for incoming requests.        
        // WsCloseListener can fail but will still close the listener in that case. 
        // So we can ignore the error.
        WsCloseListener(listener, NULL, NULL);            
    }

    if (NULL != channelManager)
    {
        channelManager->WaitForCleanup();
        delete channelManager;
        channelManager = NULL;        
    }

    if (NULL != listener)
    {
        WsFreeListener(listener);
        listener = NULL;
    }

    if (NULL != uri.chars)
    {
        HeapFree(GetProcessHeap(), 0, uri.chars);
        uri.chars = NULL;
    }    

    PrintVerbose(L"Leaving CFileRep::Stop");

    return S_OK;
}

// The following are the error reporting and tracing functions.
void CFileRep::PrintVerbose(
    __in_z const WCHAR message[])
{
    if (REPORT_VERBOSE <= errorReporting) 
    {
        wprintf(L"FileRep: ");    
        
        wprintf(message);
        wprintf(L"\n");
    }
}

void CFileRep::PrintInfo(
    __in_z const WCHAR message[])
{
    if (REPORT_INFO <= errorReporting) 
    {
        wprintf(L"FileRep: ");    
        
        wprintf(message);
        wprintf(L"\n");
    }
}

void CFileRep::PrintError(
    __in HRESULT hr, 
    __in_opt WS_ERROR* error, 
    __in bool displayAlways)
{
    // We don't alyways display errors since in during shutdown certain
    // failures are expected.
    if (!(displayAlways || started))
    {
        return;
    }

    if (REPORT_ERROR <= errorReporting) 
    {
        ::PrintError(hr, error); 
    }
}

void CFileRep::PrintError(
    __in_z const WCHAR message[], 
    __in bool displayAlways)
{
    if (!(displayAlways || started))
    {
        return;
    }

    if (REPORT_ERROR <= errorReporting) 
    {
        wprintf(L"FileRep ERROR: ");                    
        wprintf(message);
        wprintf(L"\n");
    }
}

void CFileRep::GetEncoding(
    __out WS_ENCODING* encodingProperty, 
    __out ULONG* propertyCount)
{
    *propertyCount = 1;
    
    if (TEXT_ENCODING == encoding)
    {        
        *encodingProperty = WS_ENCODING_XML_UTF8;
    }
    else if (BINARY_ENCODING == encoding)
    {
        *encodingProperty = WS_ENCODING_XML_BINARY_SESSION_1;
    }
    else if (MTOM_ENCODING == encoding)
    {
        *encodingProperty = WS_ENCODING_XML_MTOM_UTF8;
    }
    else // default encoding
    {
        *propertyCount = 0;
    }
}

// Set up the listener. Each service has exactly one. Accept channels and start the processing.
HRESULT CFileRep::InitializeListener()
{    
    PrintVerbose(L"Entering CFileRep::InitializeListener");

    HRESULT hr = S_OK;
    WS_ERROR* error = NULL;
    WS_SSL_TRANSPORT_SECURITY_BINDING transportSecurityBinding = {};
    WS_SECURITY_DESCRIPTION securityDescription = {};
    WS_SECURITY_DESCRIPTION* pSecurityDescription = NULL;
    WS_SECURITY_BINDING* securityBindings[1];

    WS_LISTENER_PROPERTY listenerProperties[1];
    WS_CALLBACK_MODEL callbackModel = WS_LONG_CALLBACK;
    listenerProperties[0].id = WS_LISTENER_PROPERTY_ASYNC_CALLBACK_MODEL;
    listenerProperties[0].value = &callbackModel;
    listenerProperties[0].valueSize = sizeof(callbackModel);


    assert(NULL == channelManager);

    IfFailedExit(WsCreateError(NULL, 0, &error));  
   
    if (SSL_SECURITY == securityMode)
    {
        // Initialize a security description for SSL.
        transportSecurityBinding.binding.bindingType = WS_SSL_TRANSPORT_SECURITY_BINDING_TYPE;
        securityBindings[0] = &transportSecurityBinding.binding;
        securityDescription.securityBindings = securityBindings;
        securityDescription.securityBindingCount = WsCountOf(securityBindings);
        pSecurityDescription = &securityDescription;
    }

    if (TCP_TRANSPORT == transportMode) // Create a TCP listener
    {   
        IfFailedExit(WsCreateListener(WS_CHANNEL_TYPE_DUPLEX_SESSION, WS_TCP_CHANNEL_BINDING, 
            listenerProperties, WsCountOf(listenerProperties), pSecurityDescription, &listener, error));
    }
    else // Create an HTTP listener
    {        
        IfFailedExit(WsCreateListener(WS_CHANNEL_TYPE_REPLY, WS_HTTP_CHANNEL_BINDING, 
            listenerProperties, WsCountOf(listenerProperties), pSecurityDescription, &listener, error));
    }        
    
    IfFailedExit(WsOpenListener(listener, &uri, NULL, error));    

    // We put fixed values here to not overly complicate the command line. 
    long maxIdleChannels = 20;
    long minIdleChannels = 10;
    if (maxChannels < maxIdleChannels)
    {
        maxIdleChannels = maxChannels;
        minIdleChannels = maxIdleChannels/2 + 1;
    }

    channelManager = new CChannelManager(this, minIdleChannels, maxIdleChannels, maxChannels);
    IfNullExit(channelManager);
    IfFailedExit(channelManager->Initialize());

    // Spins up a bunch of channels so that we are prepared to handle multipe requests in a timely fashion.
    IfFailedExit(channelManager->CreateChannels());

    if (NULL != error)
    {
        WsFreeError(error);
    }

    PrintVerbose(L"Leaving CFileRep::InitializeListener");

    return S_OK;


    ERROR_EXIT

    PrintError(L"CFileRep::InitializeListener", true);
    PrintError(hr, error, true); 
        
    // Class state is cleaned up in Stop so only clean up locals even in case of failure.
    if (NULL != error)
    {
        WsFreeError(error);
    }

    PrintVerbose(L"Leaving CFileRep::InitializeListener");
    
    return hr;
}


