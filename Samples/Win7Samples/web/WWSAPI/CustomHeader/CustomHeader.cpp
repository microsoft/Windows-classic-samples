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
    WS_MESSAGE* message = NULL;
    
    // Create an error object for storing rich error information
    hr = WsCreateError(
        NULL, 
        0, 
        &error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    // Create a message
    hr = WsCreateMessage(
        WS_ENVELOPE_VERSION_SOAP_1_2, 
        WS_ADDRESSING_VERSION_1_0, 
        NULL, 
        0, 
        &message, 
        error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    // Initialize the message
    hr = WsInitializeMessage(message, WS_BLANK_MESSAGE, NULL, error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    // Initialize purchase order
    _PurchaseOrderType purchaseOrderToAdd;
    purchaseOrderToAdd.quantity = 100;
    purchaseOrderToAdd.productName = L"Pencil";
    
    // Add purchase order data as a header
    hr = WsAddCustomHeader(
        message, 
        &PurchaseOrder_wsdl.globalElements.PurchaseOrderType, 
        WS_WRITE_REQUIRED_VALUE,
        &purchaseOrderToAdd, 
        sizeof(purchaseOrderToAdd), 
        0, 
        error);
    
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    // Get the purchase order header from the message
    _PurchaseOrderType* purchaseOrder;
    hr = WsGetCustomHeader(
        message, 
        &PurchaseOrder_wsdl.globalElements.PurchaseOrderType, 
        WS_SINGLETON_HEADER,
        0,
        WS_READ_REQUIRED_POINTER, 
        NULL, 
        &purchaseOrder, 
        sizeof(purchaseOrder), 
        NULL, 
        error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    // Print out header contents
    wprintf(L"%ld, %s\n", 
        purchaseOrder->quantity, 
        purchaseOrder->productName);
    
Exit:
    if (FAILED(hr))
    {
        // Print out the error
        PrintError(hr, error);
    }
    
    if (message != NULL)
    {
        WsFreeMessage(message);
    }
    if (error != NULL)
    {
        WsFreeError(error);
    }
    fflush(stdout);
    return SUCCEEDED(hr) ? 0 : -1;
}
