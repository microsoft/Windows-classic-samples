//////////////////////////////////////////////////////////////////////////////
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//////////////////////////////////////////////////////////////////////////////

#include <windows.h>
#include <stdlib.h>
#include <strsafe.h>
#include <string.h>
#include <stdio.h>
#include <conio.h>
#include <wsdapi.h>
#include "Common.h"
#include "TargetService.h"
#include "Service.h"

//Usages: WSDiscoveryService.exe [/a EndpointReference] [/s scope [...]]
//You may use one of /a or /s, or both (in that order).
//
//By default, both the default scope matching rule (RFC2396) and the custom
//scope matching rule (see CTargetService::MatchScopes) are attached.  But unless
//there are scopes specified for use with the target service using the /s flag,
//the attached scope matching rules will have no effect.
//
// If an EndpointReference is provided, it must begin with one of http://, https://,
// uri:, urn:uuid: or uuid:.
//
//To start a default target service with a randomly generated
//EndpointReference in the form of a urn:uuid (as defined in Section 2.6 of the
//WS-Discovery spec):
//WSDiscoveryService.exe
//
//To have 2 scopes attached to the target service:
//WSDiscoveryService.exe /s http://www.contoso.com/wsd/scope1/Printer3
//http://www.contoso.com/wsd/scope1/NoticeBoard
//
//To have an Endpoint Reference attached to the target service:
//WSDiscoveryService.exe /a urn:uuid:91022ed0-7d3d-11de-8a39-0800200c9a66
//
//To do both:
//WSDiscoveryService.exe /a urn:uuid:91022ed0-7d3d-11de-8a39-0800200c9a66 /s
//http://www.contoso.com/wsd/scope1/Printer3
//http://www.contoso.com/wsd/scope1/NoticeBoard
int _cdecl wmain
(   int argc
,   _In_reads_( argc ) LPWSTR *argv
)
{
    LPWSTR epr = NULL;
    LPCWSTR eprAfterInit = NULL; // do not deallocate
    WSD_URI_LIST *scopesList = NULL;
    CTargetService *service = NULL;
    HRESULT hr = S_OK;
    char command = '\0';

    wprintf( L"----------------------------------------\r\n" );
    wprintf( L"WS-Discovery SDK Sample Target Service\r\n" );
    wprintf( L"Copyright (c) Microsoft Corporation.  All rights reserved.\r\n" );
    wprintf( L"----------------------------------------\r\n" );

    hr = ParseArguments( argc, argv, &epr, &scopesList );

    if ( S_OK != hr )
    {
        PrintErrorMessage( L"Failed to parse arguments", hr );
    }

    if ( S_OK == hr )
    {
        wprintf( L"Creating and initializing the target service...\r\n" );
        hr = CreateTargetService( epr, scopesList, &service );

        if ( S_OK != hr )
        {
            PrintErrorMessage( L"Failed to create or initialize the target service", hr );
        }
    }

    if ( S_OK == hr )
    {
        wprintf( L"Obtaining EndpointReference: " );
        hr = service->GetEndpointReference( &eprAfterInit );

        if ( S_OK == hr )
        {
            wprintf( L"%s\r\n", eprAfterInit );
        }
        else
        {
            PrintErrorMessage( L"Failed to obtain EPR", hr );
        }
    }

    if ( S_OK == hr )
    {
        wprintf( L"Sending a Hello Message...\r\n" );
        hr = service->SendHelloMessage();

        if ( S_OK != hr )
        {
            wprintf( L"Failed to Send a Hello message...\r\n" );
        }
    }

    // listen for commands

    while ( 'q' != command && 'Q' != command && S_OK == hr )
    {
        wprintf( L"\r\n" );
        wprintf( L"Press 'h' to send a Hello, 'b' to send a Bye, 'q' to terminate: " );
        command = (char)_getch();
        wprintf( L"\r\n" );

        if ( 'h' == command || 'H' == command )
        {
            wprintf( L"Sending a Hello Message...\r\n" );
            hr = service->SendHelloMessage();

            if ( S_OK != hr )
            {
                wprintf( L"Failed to Send a Hello message...\r\n" );
            }
        }
        else if ( 'b' == command || 'B' == command )
        {
            wprintf( L"Sending a Bye Message...\r\n" );
            hr = service->SendByeMessage();

            if ( S_OK != hr )
            {
                wprintf( L"Failed to Send a Bye message...\r\n" );
            }
        }
        else if ( 'q' != command && 'Q' != command )
        {
            wprintf( L"Command not recognized..." );
        }
    }

    if ( S_OK == hr )
    {
        wprintf( L"Terminating the target service...\r\n" );
        hr = service->Terminate();

        if ( S_OK != hr )
        {
            PrintErrorMessage( L"Failed to terminate the target service", hr );
        }
    }

    eprAfterInit = NULL; // do not deallocate

    if ( NULL != epr )
    {
        delete[] epr;
        epr = NULL;
    }

    if ( NULL != scopesList )
    {
        WSDFreeLinkedMemory( scopesList );
        scopesList = NULL;
    }

    if ( NULL != service )
    {
        service->Release();
        service = NULL;
    }

    if ( S_OK != hr )
    {
        // display usage if it fails for any reason
        wprintf( L"\r\n" );
        DisplayUsages();
        wprintf( L"\r\n" );
    }

    wprintf( L"Target service application has ended.  Press any key to exit..." );
    (void)_getch();

    return 0;
}

// Parses the command line arguments according to rules
// specified in the DisplayUsages() function.
// The caller should pass in argc and argv as they were
// being passed into the wmain() function.
// The caller should call WSDFreeLinkedMemory on the
// returned scopesList, and delete[] on epr
// when they are no longer needed.
_Success_( return == S_OK )
HRESULT ParseArguments
(   _In_ int argc
,   _In_reads_( argc ) LPWSTR *argv
,   _Outptr_result_maybenull_ LPWSTR *epr
,   _Outptr_result_maybenull_ WSD_URI_LIST **scopesList
)
{
    HRESULT hr = S_OK;
    LPWSTR tempEpr = NULL;
    WSD_URI_LIST *tempScopesList = NULL;

    if ( 1 > argc || NULL == argv )
    {
        // argc must be at least 1
        // the minimum it contains is the executable name
        hr = E_INVALIDARG;
    }
    else if ( NULL == epr || NULL == scopesList )
    {
        hr = E_POINTER;
    }
    else
    {
        *epr = NULL;
        *scopesList = NULL;
    }

    if ( S_OK == hr )
    {
        // expecting arguments to be of the following format:
        // argv[0] = executable name - ignored
        //
        // argv[1] = if specified, one of /a or /s
        // if /a, then argv[2] = endpoint reference address
        // if /s, then argv[2] and above are scopes
        //
        // if argv[1] was /a, then
        // argv[3] = if specified, must be /s
        // argv[4] and above are scopes
        //
        // argc must be at least 3

        if ( 1 == argc )
        {
            // Do nothing.
            // A logical address will be generated and
            // no scopes attached.
        }
        else if ( 2 == argc )
        {
            // Not enough parameters.
            hr = E_INVALIDARG;
        }
        else if ( 0 == wcscmp( argv[1], L"/a" ) )
        {
            // argv[0]               [1] [2]     [3] [4]...
            // WSDiscoveryService.exe /a some_epr /s some_scope ...
            // argc must be either 3 or at least 5 - it cannot be 4

            if ( 4 == argc )
            {
                hr = E_INVALIDARG;
            }
            else if ( 5 <= argc && 0 != wcscmp( argv[3], L"/s" ) )
            {
                // if argv[3] is specified, it has to be /s
                hr = E_INVALIDARG;
            }

            if ( S_OK == hr )
            {
                // copy the EndpointReference
                hr = DeepCopyString( argv[2], &tempEpr );
            }

            if ( S_OK == hr && 5 <= argc )
            {
                // extract scopes from argv[4] and above
#pragma prefast( suppress: 6385, "Number of strings are previously checked." )
                hr = ParseScopes( argc, argv, 4, &tempScopesList );
            }
        }
        else if ( 0 == wcscmp( argv[1], L"/s" ) )
        {
            // extract scopes from argv[2]
            hr = ParseScopes( argc, argv, 2, &tempScopesList );
        }
        else
        {
            // unknown arguments specified
            hr = E_INVALIDARG;
        }
    }

    if ( S_OK == hr )
    {
        // outside pointer now owns temp var
        *epr = tempEpr;
        tempEpr = NULL;

        *scopesList = tempScopesList;
        tempScopesList = NULL;
    }

    if ( NULL != tempEpr )
    {
        delete[] tempEpr;
        tempEpr = NULL;
    }

    if ( NULL != tempScopesList )
    {
        WSDFreeLinkedMemory( tempScopesList );
        tempScopesList = NULL;
    }

    return hr;
};

void DisplayUsages()
{
    wprintf( L"Usages: WSDiscoveryService.exe [/a EndpointReference] [/s scope [...]]\r\n" );
    wprintf( L"You may use one of /a or /s, or both (in that order).\r\n" );
    wprintf( L"\r\n" );
    wprintf( L"By default, both the default scope matching rule (RFC2396) and the custom\r\n" );
    wprintf( L"scope matching rule (see CTargetService::MatchScopes) are attached.  But unless\r\n" );
    wprintf( L"there are scopes specified for use with the target service using the /s flag,\r\n" );
    wprintf( L"the attached scope matching rules will have no effect.\r\n" );
    wprintf( L"\r\n" );
    wprintf( L"If an EndpointReference is provided, it must begin with one of http://, https://,\r\n" );
    wprintf( L"uri:, urn:uuid: or uuid:.\r\n" );
    wprintf( L"\r\n" );
    wprintf( L"To start a default target service with a randomly generated\r\n" );
    wprintf( L"EndpointReference in the form of a urn:uuid (as defined in Section 2.6 of the\r\n" );
    wprintf( L"WS-Discovery spec):\r\n" );
    wprintf( L"WSDiscoveryService.exe\r\n" );
    wprintf( L"\r\n" );
    wprintf( L"To have 2 scopes attached to the target service:\r\n" );
    wprintf( L"WSDiscoveryService.exe /s http://www.contoso.com/wsd/scope1/Printer3\r\n" );
    wprintf( L"http://www.contoso.com/wsd/scope1/NoticeBoard\r\n" );
    wprintf( L"\r\n" );
    wprintf( L"To have an Endpoint Reference attached to the target service:\r\n" );
    wprintf( L"WSDiscoveryService.exe /a urn:uuid:91022ed0-7d3d-11de-8a39-0800200c9a66\r\n" );
    wprintf( L"\r\n" );
    wprintf( L"To do both:\r\n" );
    wprintf( L"WSDiscoveryService.exe /a urn:uuid:91022ed0-7d3d-11de-8a39-0800200c9a66 /s\r\n" );
    wprintf( L"http://www.contoso.com/wsd/scope1/Printer3\r\n" );
    wprintf( L"http://www.contoso.com/wsd/scope1/NoticeBoard\r\n" );
}
