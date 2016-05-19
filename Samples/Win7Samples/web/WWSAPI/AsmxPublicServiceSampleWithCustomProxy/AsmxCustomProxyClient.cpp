//------------------------------------------------------------
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------

#include "windows.h"
#include "winhttp.h"
#ifndef UNICODE
#define UNICODE
#endif
#include "WebServices.h"
#include "process.h"
#include "stdio.h"
#include "string.h"
#include "terraservice-usa.com.wsdl.h"

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
    WS_SERVICE_PROXY* serviceProxy = NULL;
    WS_HEAP* heap = NULL;
    WS_ENDPOINT_ADDRESS address = {};
    static const WS_STRING serviceUrl = WS_STRING_VALUE(L"http://terraservice.net/TerraService2.asmx");
    WS_CHANNEL_PROPERTY channelPropertyArray[4];
    WS_ADDRESSING_VERSION addressingVersion = WS_ADDRESSING_VERSION_TRANSPORT;
    WS_ENVELOPE_VERSION envelopeVersion = WS_ENVELOPE_VERSION_SOAP_1_1;
    WCHAR* place = NULL;
    WS_HTTP_PROXY_SETTING_MODE proxySettingMode = WS_HTTP_PROXY_SETTING_MODE_CUSTOM;
    WS_CUSTOM_HTTP_PROXY customProxy = {};
    address.url = serviceUrl;
    WINHTTP_AUTOPROXY_OPTIONS autoProxyOptions = {};
    WINHTTP_PROXY_INFO proxyInfo = {};
    HINTERNET session = NULL;
    
    channelPropertyArray[0].id = WS_CHANNEL_PROPERTY_ADDRESSING_VERSION;
    channelPropertyArray[0].value = &addressingVersion;
    channelPropertyArray[0].valueSize = sizeof(addressingVersion);
    
    channelPropertyArray[1].id = WS_CHANNEL_PROPERTY_ENVELOPE_VERSION;
    channelPropertyArray[1].value = &envelopeVersion;
    channelPropertyArray[1].valueSize = sizeof(envelopeVersion);
    
    channelPropertyArray[2].id = WS_CHANNEL_PROPERTY_HTTP_PROXY_SETTING_MODE;
    channelPropertyArray[2].value = &proxySettingMode;
    channelPropertyArray[2].valueSize = sizeof(proxySettingMode);
    
    channelPropertyArray[3].id = WS_CHANNEL_PROPERTY_CUSTOM_HTTP_PROXY;
    channelPropertyArray[3].value = &customProxy;
    channelPropertyArray[3].valueSize = sizeof(customProxy);
    
    
    // This part illustrates how to setup a HTTP header authentication security binding
    // against the HTTP proxy server in case it requires authentication.
    // declare and initialize a default windows credential
    WS_STRING_WINDOWS_INTEGRATED_AUTH_CREDENTIAL windowsCredential = {}; // zero out the struct
    windowsCredential.credential.credentialType = WS_STRING_WINDOWS_INTEGRATED_AUTH_CREDENTIAL_TYPE; // set the credential type
    // for illustration only; usernames and passwords should never be included in source files
    windowsCredential.username.chars = L"domain\\user";
    windowsCredential.username.length = (ULONG)wcslen(windowsCredential.username.chars);
    windowsCredential.password.chars = L"password";
    windowsCredential.password.length = (ULONG)wcslen(windowsCredential.password.chars);
    
    // declare and initialize properties to set the authentication scheme to Basic
    ULONG scheme = WS_HTTP_HEADER_AUTH_SCHEME_NEGOTIATE;
    ULONG target = WS_HTTP_HEADER_AUTH_TARGET_PROXY;
    WS_SECURITY_BINDING_PROPERTY httpProxyAuthBindingProperties[2] =
    {
        { WS_SECURITY_BINDING_PROPERTY_HTTP_HEADER_AUTH_SCHEME, &scheme, sizeof(scheme) },
        { WS_SECURITY_BINDING_PROPERTY_HTTP_HEADER_AUTH_TARGET, &target, sizeof(target) }
    };
    
    // declare and initialize an HTTP header authentication security binding for the HTTP proxy server
    WS_HTTP_HEADER_AUTH_SECURITY_BINDING httpProxyAuthBinding = {}; // zero out the struct
    httpProxyAuthBinding.binding.bindingType = WS_HTTP_HEADER_AUTH_SECURITY_BINDING_TYPE; // set the binding type
    httpProxyAuthBinding.binding.properties = httpProxyAuthBindingProperties;
    httpProxyAuthBinding.binding.propertyCount = WsCountOf(httpProxyAuthBindingProperties);
    httpProxyAuthBinding.clientCredential = &windowsCredential.credential;
    
    // declare and initialize the array of all security bindings
    WS_SECURITY_BINDING* securityBindings[1] = { &httpProxyAuthBinding.binding };
    
    // declare and initialize the security description
    WS_SECURITY_DESCRIPTION securityDescription = {}; // zero out the struct
    securityDescription.securityBindings = securityBindings;
    securityDescription.securityBindingCount = WsCountOf(securityBindings);
    
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
    
    
    session = WinHttpOpen(L"NWS Example",
        WINHTTP_ACCESS_TYPE_NO_PROXY, 
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS,
        WINHTTP_FLAG_ASYNC);
    if (!session)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }
    autoProxyOptions.dwFlags = WINHTTP_AUTOPROXY_RUN_INPROCESS | WINHTTP_AUTOPROXY_AUTO_DETECT;
    autoProxyOptions.dwAutoDetectFlags = WINHTTP_AUTO_DETECT_TYPE_DHCP | WINHTTP_AUTO_DETECT_TYPE_DNS_A;
    autoProxyOptions.fAutoLogonIfChallenged = FALSE;
    
    WinHttpGetProxyForUrl(
        session,
        serviceUrl.chars,
        &autoProxyOptions,
        &proxyInfo);
    
    if (proxyInfo.dwAccessType == WINHTTP_ACCESS_TYPE_NAMED_PROXY)
    {
        if (proxyInfo.lpszProxy)
        {
            customProxy.servers.chars = proxyInfo.lpszProxy;
            customProxy.servers.length = (ULONG)wcslen(proxyInfo.lpszProxy);
        }
        if (proxyInfo.lpszProxyBypass)
        {
            customProxy.bypass.chars = proxyInfo.lpszProxyBypass;
            customProxy.bypass.length = (ULONG)wcslen(proxyInfo.lpszProxyBypass);
        }
    }
    
    hr = WsCreateServiceProxy(
        WS_CHANNEL_TYPE_REQUEST, 
        WS_HTTP_CHANNEL_BINDING, 
        &securityDescription, 
        NULL, 
        0,
        channelPropertyArray,
        WsCountOf(channelPropertyArray),
        &serviceProxy, 
        error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    
    // Open channel to address
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
        LonLatPt point = {10.0, 10.0};
        hr = TerraServiceSoap_ConvertLonLatPtToNearestPlace(
            serviceProxy,
            &point,
            &place,
            heap,
            NULL,
            0,
            NULL,
            error);
        if (FAILED(hr))
        {
            goto Exit;
        }
        
        if (place != NULL)
        {
            wprintf(L"Place @ Lattitude=%f, Longitutde=%f is %s\n", 
                point.Lon,
                point.Lat,
                place);
        }
        else
        {
            wprintf(L"Could not find any place for Lattitude=%f, Longitutde=%f\n",
                point.Lon,
                point.Lat);
        }
        fflush(stdout);
        
        hr = WsResetHeap(
            heap,
            error);
        if (FAILED(hr))
        {
            goto Exit;
        }
    }
Exit:
    if (FAILED(hr))
    {
        // Print out the error
        PrintError(hr, error);
    }
    if (proxyInfo.lpszProxy)
    {
        ::GlobalFree(proxyInfo.lpszProxy);
    }
    if (proxyInfo.lpszProxyBypass)
    {
        ::GlobalFree(proxyInfo.lpszProxyBypass);
    }
    if (serviceProxy != NULL)
    {
        WsCloseServiceProxy(serviceProxy, NULL, NULL);
        WsFreeServiceProxy(serviceProxy);
    }
    if (!session)
    {
        WinHttpCloseHandle(session);
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
