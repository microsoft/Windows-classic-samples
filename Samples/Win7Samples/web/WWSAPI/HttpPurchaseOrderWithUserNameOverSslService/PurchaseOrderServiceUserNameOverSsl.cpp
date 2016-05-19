//------------------------------------------------------------
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------

#ifndef UNICODE
#define UNICODE
#endif
#include "WebServices.h"
#include "process.h"
#include "stdio.h"
#include "string.h"
#include "wincrypt.h"

BOOL CompareWsString(const WS_STRING* string1, const WS_STRING* string2)
{
    ULONG length = string1->length;

    if (length == string2->length)
    {
        if (0 == length)
        {
            return TRUE;
        }

// The strings are not null-terminated because we pass also their lenghts to the CompareStringW        
#pragma warning(suppress:26035)
        if (CompareStringW(LOCALE_INVARIANT, 0, string1->chars,
            length, string2->chars, length) == CSTR_EQUAL)
        {
            return TRUE;
        }
    }

    return FALSE;
}
#include <strsafe.h>
#include "PurchaseOrder.wsdl.h"

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

HANDLE closeServer = NULL;  



HRESULT CALLBACK AuthorizationCallback(
    __in const WS_OPERATION_CONTEXT* context, 
    __out BOOL* authorized, 
    __in_opt WS_ERROR* error)
{
    HRESULT hr = S_OK;
    const WS_STRING fixedUsername = WS_STRING_VALUE(L"usr1");
    WS_MESSAGE* message = NULL;
    WS_STRING usernameIdentity = {};
    *authorized = FALSE;
    
    hr = WsGetOperationContextProperty(context, WS_OPERATION_CONTEXT_PROPERTY_INPUT_MESSAGE, &message, sizeof(message), error);
    if (FAILED(hr))
    {
        return hr;
    }
    
    hr = WsGetMessageProperty(message, WS_MESSAGE_PROPERTY_USERNAME, &usernameIdentity, sizeof(usernameIdentity), error);
    if (FAILED(hr))
    {
        return hr;
    }
        
    *authorized = CompareWsString(&usernameIdentity, &fixedUsername);
    return S_OK;
}

// define a custom validator for received username/password pairs
static HRESULT CALLBACK MyPasswordValidator(
    __in void* callbackState,
    __in const WS_STRING* username, 
    __in const WS_STRING* password,
    __in const WS_ASYNC_CONTEXT* asyncContext, 
    __in_opt WS_ERROR* error)
{
    UNREFERENCED_PARAMETER(callbackState);
    UNREFERENCED_PARAMETER(asyncContext);
    UNREFERENCED_PARAMETER(error);

    const WS_STRING fixedUsername = WS_STRING_VALUE(L"usr1");
    const WS_STRING fixedPassword = WS_STRING_VALUE(L"pwd1");

    if (CompareWsString(
            username, 
            &fixedUsername) 
        && 
        CompareWsString(
            password, 
            &fixedPassword))
    {
        return S_OK;
    }
    return S_FALSE;
}

static const WCHAR ExpectedShipDate [] = L"1/1/2006";
static const WCHAR OrderStatusString [] = L"Pending";

volatile long numberOfOrders = 0;


HRESULT CALLBACK PurchaseOrderImpl(
    __in const WS_OPERATION_CONTEXT* context,
    __in int quantity,
    __in_opt __nullterminated WCHAR* productName,
    __out unsigned int* orderID,
    __out_opt __deref __nullterminated WCHAR** expectedShipDate,
    __in_opt const WS_ASYNC_CONTEXT* asyncContext,
    __in_opt WS_ERROR* error)
{
    UNREFERENCED_PARAMETER(asyncContext);

    WS_HEAP* heap = NULL;
    HRESULT hr = S_OK;
    
    wprintf(L"%ld, %s\n", quantity, productName);
    fflush(stdout);
    
    hr = WsGetOperationContextProperty(
        context, 
        WS_OPERATION_CONTEXT_PROPERTY_HEAP, 
        &heap, 
        sizeof(heap), 
        error);
if (FAILED(hr))
{
    goto Exit;
}
    
    hr = WsAlloc(
        heap, 
        sizeof(ExpectedShipDate), 
        (void**)expectedShipDate, 
        error);
if (FAILED(hr))
{
    goto Exit;
}
    
    hr = StringCbCopyW(
        *expectedShipDate, 
        sizeof(ExpectedShipDate), 
        ExpectedShipDate);
if (FAILED(hr))
{
    goto Exit;
}
    
    *orderID = 123;
    
Exit:
    return hr;
}


HRESULT CALLBACK GetOrderStatusImpl(
    __in const WS_OPERATION_CONTEXT* context,
    __inout unsigned int* orderID,
    __out_opt __deref __nullterminated WCHAR** status,
    __in_opt const WS_ASYNC_CONTEXT* asyncContext,
    __in_opt WS_ERROR* error)
{
    UNREFERENCED_PARAMETER(asyncContext);

    WS_HEAP* heap = NULL;
    HRESULT hr = S_OK;

    // Generate a fault if we don't recognize the order ID
    if (*orderID != 123)
    {
        // Fill out details about the fault
        _OrderNotFoundFaultType orderNotFound;
        orderNotFound.orderID = *orderID;
        
        static const WS_XML_STRING _faultDetailName = WS_XML_STRING_VALUE("OrderNotFound");
        static const WS_XML_STRING _faultDetailNs = WS_XML_STRING_VALUE("http://example.com");
        static const WS_XML_STRING _faultAction = WS_XML_STRING_VALUE("http://example.com/fault");
        static const WS_ELEMENT_DESCRIPTION _faultElementDescription = 
        { 
            (WS_XML_STRING*)&_faultDetailName, 
            (WS_XML_STRING*)&_faultDetailNs, 
            WS_UINT32_TYPE, 
            NULL 
        };
        static const WS_FAULT_DETAIL_DESCRIPTION orderNotFoundFaultTypeDescription = 
        { 
            (WS_XML_STRING*)&_faultAction, 
            (WS_ELEMENT_DESCRIPTION*)&_faultElementDescription 
        };
        
        // Set fault detail information in the error object
        hr = WsSetFaultErrorDetail(
            error,
            &orderNotFoundFaultTypeDescription,
            WS_WRITE_REQUIRED_VALUE,
            &orderNotFound,
            sizeof(orderNotFound));
        
        if (FAILED(hr))
        {
            goto Exit;
        }
        
        // Add an error string to the error object.  This string will
        // be included in the fault that is sent.
        static const WS_STRING errorMessage = WS_STRING_VALUE(L"Invalid order ID");
        hr = WsAddErrorString(error, &errorMessage);
        
        if (FAILED(hr))
        {
            goto Exit;
        }

        hr = E_FAIL;
        goto Exit;
    }
    
    *orderID = *orderID;
    
    hr = WsGetOperationContextProperty(
        context, 
        WS_OPERATION_CONTEXT_PROPERTY_HEAP, 
        &heap, 
        sizeof(heap), 
        error);
if (FAILED(hr))
{
    goto Exit;
}
    
    hr = WsAlloc(
        heap, 
        sizeof(OrderStatusString), 
        (void**)status, 
        error);
if (FAILED(hr))
{
    goto Exit;
}
    
    hr = StringCbCopyW(
        *status, 
        sizeof(OrderStatusString), 
        OrderStatusString);
if (FAILED(hr))
{
    goto Exit;
}
    
Exit:
    return hr;
}

HRESULT CALLBACK CloseChannelCallback(
    __in const WS_OPERATION_CONTEXT* context, 
    __in_opt const WS_ASYNC_CONTEXT* asyncContext)
{
    UNREFERENCED_PARAMETER(context);
    UNREFERENCED_PARAMETER(asyncContext);

    ULONG orderCount = InterlockedIncrement(&numberOfOrders);
    if (orderCount == 300)
    {
        SetEvent(closeServer);
    }
    return S_OK;
}

static const PurchaseOrderBindingFunctionTable purchaseOrderFunctions = {PurchaseOrderImpl, GetOrderStatusImpl};

// Method contract for the service
static const WS_SERVICE_CONTRACT purchaseOrderContract = 
{
    &PurchaseOrder_wsdl.contracts.PurchaseOrderBinding, // comes from the generated header.
    NULL, // for not specifying the default contract
    &purchaseOrderFunctions // specified by the user
};


// Main entry point
int __cdecl wmain()
{
    
    HRESULT hr = S_OK;
    WS_SERVICE_HOST* host = NULL;
    WS_SERVICE_ENDPOINT serviceEndpoint = {};
    const WS_SERVICE_ENDPOINT* serviceEndpoints[1];
    serviceEndpoints[0] = &serviceEndpoint;
    WS_ERROR* error = NULL;
    
    // declare and initialize a username message security binding
    WS_USERNAME_MESSAGE_SECURITY_BINDING usernameBinding = {}; // zero out the struct
    usernameBinding.binding.bindingType = WS_USERNAME_MESSAGE_SECURITY_BINDING_TYPE; // set the binding type
    usernameBinding.bindingUsage = WS_SUPPORTING_MESSAGE_SECURITY_USAGE; // set the binding usage
    usernameBinding.passwordValidator = MyPasswordValidator;
    
    // declare and initialize an SSL transport security binding
    WS_SSL_TRANSPORT_SECURITY_BINDING sslBinding = {}; // zero out the struct
    sslBinding.binding.bindingType = WS_SSL_TRANSPORT_SECURITY_BINDING_TYPE; // set the binding type
    // NOTE: At the server, the SSL certificate for the listen URI must be
    // registered with http.sys using a tool such as httpcfg.exe.
    
    // declare and initialize the array of all security bindings
    WS_SECURITY_BINDING* securityBindings[2] = { &sslBinding.binding, &usernameBinding.binding };
    
    // declare and initialize the security description
    WS_SECURITY_DESCRIPTION securityDescription = {}; // zero out the struct
    securityDescription.securityBindings = securityBindings;
    securityDescription.securityBindingCount = WsCountOf(securityBindings);
    WS_SERVICE_ENDPOINT_PROPERTY serviceProperties[1];
    WS_SERVICE_PROPERTY_CLOSE_CALLBACK closeCallbackProperty = {CloseChannelCallback};
    serviceProperties[0].id = WS_SERVICE_ENDPOINT_PROPERTY_CLOSE_CHANNEL_CALLBACK;
    serviceProperties[0].value = &closeCallbackProperty;
    serviceProperties[0].valueSize = sizeof(closeCallbackProperty);
    
    
    // Initialize service endpoint
    serviceEndpoint.address.url.chars = L"https://localhost:8443/example"; // address given as uri
    serviceEndpoint.address.url.length = (ULONG)wcslen(serviceEndpoint.address.url.chars);
    serviceEndpoint.channelBinding = WS_HTTP_CHANNEL_BINDING; // channel binding for the endpoint
    serviceEndpoint.channelType = WS_CHANNEL_TYPE_REPLY; // the channel type
    serviceEndpoint.securityDescription = &securityDescription; // security description
    serviceEndpoint.contract = &purchaseOrderContract;  // the contract
    serviceEndpoint.properties = serviceProperties;
    serviceEndpoint.propertyCount = WsCountOf(serviceProperties);
    serviceEndpoint.authorizationCallback = AuthorizationCallback;
    
    // Create an error object for storing rich error information
    hr = WsCreateError(
        NULL, 
        0, 
        &error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    // Create Event object for closing the server
    closeServer = CreateEvent(
        NULL, 
        TRUE, 
        FALSE, 
        NULL);
    if (closeServer == NULL)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }   
    // Creating a service host
    hr = WsCreateServiceHost(
        serviceEndpoints, 
        1, 
        NULL, 
        0, 
        &host, 
        error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    // WsOpenServiceHost to start the listeners in the service host 
    hr = WsOpenServiceHost(
        host, 
        NULL, 
        error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    WaitForSingleObject(closeServer, INFINITE);
    // Close the service host
    hr = WsCloseServiceHost(host, NULL, error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
Exit:
    if (FAILED(hr))
    {
        // Print out the error
        PrintError(hr, error);
    }
    if (host != NULL)
    {
        WsFreeServiceHost(host);
    }
    
    
    if (error != NULL)
    {
        WsFreeError(error);
    }
    if (closeServer != NULL)
    {
        CloseHandle(closeServer);
    }
    fflush(stdout);
    return SUCCEEDED(hr) ? 0 : -1;
}

