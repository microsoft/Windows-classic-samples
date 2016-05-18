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

// Main entry point
int __cdecl wmain()
{
    
    HRESULT hr = S_OK;
    WS_ERROR* error = NULL;
    WS_HEAP* heap = NULL;
    WS_SERVICE_PROXY* serviceProxy = NULL;
    static const WS_STRING serviceUrl = WS_STRING_VALUE(L"https://localhost:8443/example");
    
    
    // declare and initialize a username credential
    WS_STRING_USERNAME_CREDENTIAL usernameCredential = {}; // zero out the struct
    static const WS_STRING userName = WS_STRING_VALUE(L"usr1");
    static const WS_STRING passWord = WS_STRING_VALUE(L"pwd1");
    usernameCredential.credential.credentialType = WS_STRING_USERNAME_CREDENTIAL_TYPE; // set the credential type
    usernameCredential.username = userName;
    usernameCredential.password = passWord;
    
    // declare and initialize a username message security binding
    WS_USERNAME_MESSAGE_SECURITY_BINDING usernameBinding = {}; // zero out the struct
    usernameBinding.binding.bindingType = WS_USERNAME_MESSAGE_SECURITY_BINDING_TYPE; // set the binding type
    usernameBinding.bindingUsage = WS_SUPPORTING_MESSAGE_SECURITY_USAGE; // set the binding usage
    usernameBinding.clientCredential = &usernameCredential.credential;
    
    // declare and initialize an SSL transport security binding
    WS_SSL_TRANSPORT_SECURITY_BINDING sslBinding = {}; // zero out the struct
    sslBinding.binding.bindingType = WS_SSL_TRANSPORT_SECURITY_BINDING_TYPE; // set the binding type
    
    // declare and initialize the array of all security bindings
    WS_SECURITY_BINDING* securityBindings[2] = { &sslBinding.binding, &usernameBinding.binding };
    
    // declare and initialize the security description
    WS_SECURITY_DESCRIPTION securityDescription = {}; // zero out the struct
    securityDescription.securityBindings = securityBindings;
    securityDescription.securityBindingCount = WsCountOf(securityBindings);
    
    __nullterminated static const WCHAR* productName = L"Pencil";
    __nullterminated WCHAR* expectedShipDate = NULL;
    __nullterminated WCHAR* orderStatus  = NULL;
    WS_ENDPOINT_ADDRESS address = {};
    address.url = serviceUrl;
    
    
    // Create an error object for storing rich error information
    hr = WsCreateError(
        NULL, 
        0, 
        &error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    // Create a heap to store deserialized data
    hr = WsCreateHeap(
        /*maxSize*/ 2048, 
        /*trimSize*/ 512, 
        NULL, 
        0, 
        &heap, 
        error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    hr = WsCreateServiceProxy(
        WS_CHANNEL_TYPE_REQUEST, 
        WS_HTTP_CHANNEL_BINDING, 
        &securityDescription, 
        NULL, 
        0, 
        NULL, 
        0, 
        &serviceProxy, 
        error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    
    hr = WsOpenServiceProxy(
        serviceProxy, 
        &address, 
        NULL, 
        error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    for (int i = 0; i < 100; i++)
    {
        unsigned int orderID;
    
        // Submit an order, and get expected ship date
        hr = PurchaseOrderBinding_Order(
            serviceProxy, 
            100, 
            (WCHAR*)productName, 
            &orderID, 
            &expectedShipDate, 
            heap, 
            NULL, 
            0, 
            NULL, 
            error);
    
        if (FAILED(hr))
        {
            goto Exit;
        }
    
        // Print out confirmation contents
        wprintf(L"Expected ship date for order %lu is %s\n",
            orderID,
            expectedShipDate);
        
        WsResetHeap(heap, NULL);
        
        // Get the current status of the order
        hr = PurchaseOrderBinding_OrderStatus(
            serviceProxy, 
            &orderID, 
            &orderStatus, 
            heap, 
            NULL, 
            0, 
            NULL, 
            error);
    
        if (FAILED(hr))
        {
            goto Exit;
        }
    
        // Print out order status
        wprintf(L"Order status for order %lu is: %s\n",
            orderID,
            orderStatus);
        
        WsResetHeap(
            heap, 
            NULL);
    
        // Get the current status of the order using an invalid order ID
        orderID = 321;
        hr = PurchaseOrderBinding_OrderStatus(
            serviceProxy, 
            &orderID, 
            &orderStatus, 
            heap, 
            NULL, 
            0, 
            NULL, 
            error);
    
        // Check to see if we got a fault
        if (hr == WS_E_ENDPOINT_FAULT_RECEIVED)
        {
            // Print the strings in the error object
            PrintError(hr, error);
        
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
        
            // Try to get the fault detail from the error object
            _OrderNotFoundFaultType* orderNotFound;
            hr = WsGetFaultErrorDetail(
                error,
                &orderNotFoundFaultTypeDescription,
                WS_READ_OPTIONAL_POINTER,
                heap,
                &orderNotFound,
                sizeof(orderNotFound));
                
            if (FAILED(hr))
            {
                goto Exit;
            }
        
            if (orderNotFound != NULL)
            {
                // Print out the fault detail
                wprintf(L"Order %lu was not found\n", orderNotFound->orderID);
            }
        
            // Reset error so it can be used again
            hr = WsResetError(error);
            if (FAILED(hr))
            {
                goto Exit;
            }
        }
    
        if (FAILED(hr))
        {
            goto Exit;
        }
    
        WsResetHeap(heap, NULL);
    
        wprintf(L"\n");
    }
                   
Exit:
    if (FAILED(hr))
    {
        // Print out the error
        PrintError(hr, error);
    }
    if (serviceProxy != NULL)
    {
        WsCloseServiceProxy(serviceProxy, NULL, NULL);
        WsFreeServiceProxy(serviceProxy);
    }
    
    
    if (heap != NULL)
    {
        WsFreeHeap(heap);
    }
    if (error != NULL)
    {
        WsFreeError(error);
    }
    fflush(stdout);
    return SUCCEEDED(hr) ? 0 : -1;
}
