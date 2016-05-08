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
#include "CalculatorService.wsdl.h"

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

HRESULT CreateXmlSecurityToken(
    __deref_out WS_SECURITY_TOKEN** xmlSecurityToken,
    __in_opt WS_ERROR* error)
{
    HRESULT hr = S_OK;
    WS_HEAP* heap = NULL;
    WS_XML_READER* reader = NULL;
    WS_XML_BUFFER* buffer = NULL;

    // The username/password are included in code for the simplicity
    // of the sample.  This should NOT be done for real applications.
    char* securityTokenWireXmlForm = "<x:UsernameToken xmlns:x='http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd'><x:Username>usr1</x:Username><x:Password>pwd1</x:Password></x:UsernameToken>";

    // create an XML reader
    hr = WsCreateReader(
        NULL, 
        0, 
        &reader, 
        error);
if (FAILED(hr))
{
    goto Exit;
}

    // define the input and encoding for the XML reader
    WS_XML_READER_BUFFER_INPUT readerInput;
    ZeroMemory(&readerInput, sizeof(readerInput));
    readerInput.input.inputType = WS_XML_READER_INPUT_TYPE_BUFFER;
    readerInput.encodedData = securityTokenWireXmlForm;
    readerInput.encodedDataSize = (ULONG)strlen(securityTokenWireXmlForm);

    WS_XML_READER_TEXT_ENCODING readerEncoding;
    ZeroMemory(&readerEncoding, sizeof(readerEncoding));
    readerEncoding.encoding.encodingType = WS_XML_READER_ENCODING_TYPE_TEXT;
    readerEncoding.charSet = WS_CHARSET_UTF8;

    // set the input and encoding for the XML reader
    hr = WsSetInput(
        reader, 
        &readerEncoding.encoding, 
        &readerInput.input, 
        NULL, 
        0, 
        error);
if (FAILED(hr))
{
    goto Exit;
}

    // create a heap to read the security token XML form into an XML buffer allocated on that heap
    hr = WsCreateHeap(
        2048, 
        512, 
        NULL, 0, 
        &heap, 
        error);
if (FAILED(hr))
{
    goto Exit;
}

    // read the security token XML form into an XML buffer
    hr = WsReadType(
        reader, 
        WS_ELEMENT_TYPE_MAPPING, 
        WS_XML_BUFFER_TYPE, NULL, 
        WS_READ_REQUIRED_POINTER, 
        heap, 
        &buffer, 
        sizeof(buffer), 
        error);
if (FAILED(hr))
{
    goto Exit;
}

    // create an XML security token from the token's wire form available in the XML buffer
    hr = WsCreateXmlSecurityToken(
        buffer, 
        NULL, 
        NULL, 
        0, 
        xmlSecurityToken, 
        error);

 Exit:
    // The heap, and the XML buffer allocated on it, need not be kept
    // alive once the token creation call returns.  Note that the XML
    // buffer allocated on the heap is automatically freed along with
    // the heap, and is never freed directly.
    if (heap != NULL)
    {
        WsFreeHeap(
            heap);
    }

    if (reader != NULL)
    {
        WsFreeReader(
            reader);
    }

    return hr;
}


// Main entry point
int __cdecl wmain()
{
    
    HRESULT hr = S_OK;
    WS_ERROR* error = NULL;
    WS_HEAP* heap = NULL;
    WS_SECURITY_TOKEN* securityToken = NULL;
    WS_SERVICE_PROXY* proxy = NULL;
    
    // declare and initialize an XML security token message security binding
    WS_XML_TOKEN_MESSAGE_SECURITY_BINDING xmlTokenBinding = {}; // zero out the struct
    xmlTokenBinding.binding.bindingType = WS_XML_TOKEN_MESSAGE_SECURITY_BINDING_TYPE; // set the binding type
    xmlTokenBinding.bindingUsage = WS_SUPPORTING_MESSAGE_SECURITY_USAGE; // set the binding usage
    
    // declare and initialize an SSL transport security binding
    WS_SSL_TRANSPORT_SECURITY_BINDING sslBinding = {}; // zero out the struct
    sslBinding.binding.bindingType = WS_SSL_TRANSPORT_SECURITY_BINDING_TYPE; // set the binding type
    
    // declare and initialize the array of all security bindings
    WS_SECURITY_BINDING* securityBindings[2] = { &sslBinding.binding, &xmlTokenBinding.binding };
    
    // declare and initialize the security description
    WS_SECURITY_DESCRIPTION securityDescription = {}; // zero out the struct
    securityDescription.securityBindings = securityBindings;
    securityDescription.securityBindingCount = WsCountOf(securityBindings);
    
    int result = 0;
    WS_ENDPOINT_ADDRESS address = {};
    static const WS_STRING url = WS_STRING_VALUE(L"https://localhost:8443/example");
    address.url = url;
    
    // Create an error object for storing rich error information
    hr = WsCreateError(
        NULL, 
        0, 
        &error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    // create an XML security token and set it on the relevant security binding
    hr = CreateXmlSecurityToken(&securityToken, error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    xmlTokenBinding.xmlToken = securityToken;
    
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
    
    // Create the proxy
    hr = WsCreateServiceProxy(
        WS_CHANNEL_TYPE_REQUEST, 
        WS_HTTP_CHANNEL_BINDING, 
        &securityDescription, 
        NULL, 
        0, 
        NULL,
        0,
        &proxy, 
        error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    // after the proxy is created, the security token handle can be freed
    if (securityToken != NULL)
    {
        WsFreeSecurityToken(securityToken);
        securityToken = NULL;
    }
    
    
    
    hr = WsOpenServiceProxy(
        proxy, 
        &address, 
        NULL, 
        error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    hr = DefaultBinding_ICalculator_Add(
        proxy, 
        1, 
        2, 
        &result, 
        heap, 
        NULL, 
        0, 
        NULL, 
        error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    wprintf(L"%d + %d = %d\n", 1, 2, result);
                   
Exit:
    if (FAILED(hr))
    {
        // Print out the error
        PrintError(hr, error);
    }
    if (proxy != NULL)
    {
        WsCloseServiceProxy(
            proxy, 
            NULL, 
            NULL);
    
        WsFreeServiceProxy(
            proxy);
    }
    if (securityToken != NULL)
    {
        WsFreeSecurityToken(
            securityToken);
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
