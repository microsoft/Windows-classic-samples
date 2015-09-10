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
#include <time.h>
#include <wsdapi.h>
#include "Common.h"
#include "ClientNotificationSink.h"
#include "Client.h"

//Usages: WSDiscoveryClient.exe [/a EndpointReference] | [/sr | /sc] scope [...]
//You can specify exactly one of /a, /sr or /sc - not a combination of them.
//
//An EndpointReference is the ID of the target service, which must be a valid
//URI as per Section 2.6 of the WS-Discovery Specifications.  Specifically, it must
//begin with one of http://, https://, uri:, urn:uuid: or uuid:.
//
//Examples:
//
//To find all WS-Discovery enabled target services: (Probe)
//WSDiscoveryClient.exe
//
//To find a device with a given EndpointReference: (Resolve)
//WSDiscoveryClient.exe /a urn:uuid:91022ed0-7d3d-11de-8a39-0800200c9a66
//
//To find a device with two scopes using the RFC2396 scope matching rule: (Probe)
//WSDiscoveryClient.exe /sr http://www.contoso.com/wsd/scope1 http://www.contoso.com/wsd/scope2
//
//To find a device with a given scope using the custom scope matching rule: (Probe)
//WSDiscoveryClient.exe /sc http://www.contoso.com/wsd/special/123
int _cdecl wmain
(   int argc
,   _In_reads_( argc ) LPWSTR *argv
)
{
    IWSDiscoveryProvider *provider = NULL;
    IWSDiscoveryProviderNotify *tempNotify = NULL;
    CClientNotificationSink *sink = NULL;
    LPCWSTR tag = NULL;
    LPCWSTR epr = NULL;
    LPCWSTR matchByRule = NULL;
    WSD_URI_LIST *scopesList = NULL;
    BOOL isSinkAttached = FALSE;
    HRESULT hr = S_OK;
    HRESULT hr2 = S_OK;

    wprintf( L"----------------------------------------\r\n" );
    wprintf( L"WS-Discovery SDK Sample Client\r\n" );
    wprintf( L"Copyright (c) Microsoft Corporation.  All rights reserved.\r\n" );
    wprintf( L"----------------------------------------\r\n" );

    hr = ParseArguments( 
        argc, argv, &epr, &scopesList, &matchByRule );

    if ( S_OK != hr )
    {
        PrintErrorMessage( L"Failed to parse arguments", hr );
    }
    
    if ( S_OK == hr && NULL != epr )
    {
        // If an EndpointReference is specified, make sure that the
        // EndpointReference is a valid one before proceeding.
        // (This is purely for cosmetic purposes.)
        hr = ValidateEndpointReference( epr );
    }

    if ( S_OK == hr )
    {
        // Create the discovery provider.
        // Note: This sample does not include searching with Types.  If you would like
        // to search using Types, you will need to create an IWSDXMLContext and pass
        // the parameter in WSDCreateDiscoveryProvider.  IWSDXMLContext is later used
        // to build the WSD_NAME_LIST structures.
        wprintf( L"Creating the discovery provider...\r\n" );
        hr = WSDCreateDiscoveryProvider( NULL, &provider );

        if ( S_OK != hr )
        {
            PrintErrorMessage( L"Failed to create discovery provider", hr );
        }
    }

    if ( S_OK == hr )
    {
        // Create the notification sink for callbacks
        // in the provider
        wprintf( L"Creating the notification sink...\r\n" );
        hr = CreateClientNotificationSink(&sink);

        if ( S_OK != hr )
        {
            PrintErrorMessage( L"Failed to create notification sink", hr );
        }
    }

    if ( S_OK == hr )
    {
        // Obtain the discovery provider notify (callback object)
        // needed to be attached to the provider
        wprintf( L"Obtaining the IWSDiscoveryProviderNotify interface " );
        wprintf( L"from the notification sink...\r\n" );

        hr = sink->QueryInterface( 
            __uuidof( IWSDiscoveryProviderNotify ), (void**)&tempNotify );

        if ( S_OK != hr )
        {
            PrintErrorMessage( L"Failed to query interface for IWSDiscoveryProviderNotify", hr );
        }
    }

    if ( S_OK == hr )
    {
        // Attach the notification sink to the provider to receive
        // callbacks of Hello, ProbeMatches and Bye messages.  Once the
        // sink is attached successfully, the notification sink begins
        // to listen to callbacks.  If you have custom routines in the
        // notification sink that requires resources such as internal
        // variables, you should ensure that those variables are properly
        // initialized before attaching the sink to the provider.
        wprintf( L"Attaching the notification sink...\r\n" );
        hr = provider->Attach( tempNotify );

        if ( S_OK != hr )
        {
            PrintErrorMessage(
                L"Failed to attach notification sink to the provider", hr );
        }
    }

    if ( S_OK == hr )
    {
        // Set flag to true so that the sink will be detached on exit.
        isSinkAttached = TRUE;

        // Generate tag for use with the search query.
        // Note that the tag generation is done for demo purposes only.
        // In reality, any string may be used as a tag.  It may even be
        // a hardcoded string.  The tag is sent as part of the Probe
        // or Resolve message, and will be returned in the ProbeMatches
        // and ResolveMatches messages, making it easier for you to
        // distinguish between what messages are responses to your search
        // requests.  The use of a tag, however, is optional.
        wprintf( L"Generating a tag for use in this client: " );
        hr = GenerateTag( &tag );

        if ( S_OK != hr )
        {
            PrintErrorMessage( L"Failed to generate tag", hr );
        }
        else
        {
            wprintf( L"%s\r\n", tag );
        }
    }

    if ( S_OK == hr )
    {
        wprintf( L"\r\n" );
        wprintf( L"----------------------------------------\r\n" );
        wprintf( L"Search begins - press any key to terminate the client...\r\n" );
        wprintf( L"----------------------------------------\r\n" );

        // A general client message pattern looks like the following.
        //
        // Client sends a Probe message (IWSDiscoveryProvider::SearchByType)
        // to search for the types of target services (or all services) within
        // a given number of scopes (or all scopes available).
        //
        // (IWSDiscoveryProvider::SearchByType searches both type and scope.
        // In this sample, only the scope is used.)
        //
        // Client receives a ProbeMatches message from those target services
        // that matches (IWSDiscoveryProviderNotify::Add).
        //
        // If the received ProbeMatches message does not contain an XAddrs list,
        // then the client, if it wishes, shall send a Resolve message
        // (IWSDiscoveryProvider::SearchById) to request the given target service
        // to provide an XAddrs list of transport addresses.
        //
        // Client receives a ResolveMatches message from the given target service
        // (IWSDiscoveryProviderNotify::Add).
        //
        // Note that this sample does not implement the full message pattern listed above.
        // It only executes either IWSDiscoveryProvider::SearchById or
        // IWSDiscoveryProvider::SearchByType one at a time depending on what
        // the command line arguments are given to this application.

        // epr != NULL - /a - SearchById - Resolve
        if ( NULL != epr )
        {
            // Calls SearchById to send a Resolve message.  The notification
            // sink will listen to ResolveMatches message, and shall call
            // IWSDiscoveryProviderNotify::Add (implemented by
            // CClientNotificationSink::Add in this case) when a ResolveMatches
            // message arrives.
            //
            // Note that SearchById is an asynchronous call.
            // The return code for this is solely based on whether that
            // async call has been started successfully.  The results of the
            // search are done through callback methods implemented in the
            // notification sink.
            hr = provider->SearchById( epr, tag );

            if ( S_OK != hr )
            {
                PrintErrorMessage( L"SearchById failed...", hr );
            }
        }
        // case 1 - matchByRule != NULL && scopesList != NULL (they both will be non-null)
        // - /sc or /sr - SearchByType - Probe
        // case 2 - they are all NULL
        // - no arguments - SearchByType - Probe
        else
        {
            // Calls SearchByType to send a Probe message.  Types, Scopes and
            // MatchBy rule are included in the Probe message if they are provided.
            // If none of them are provided, the Probe message searches for all
            // WS-Discovery enabled target services.  The notification sink will
            // listen to ProbeMatches messages, and shall call IWSDiscoveryProviderNotify::Add
            // (implemented by CClientNotificationSink::Add in this case) when
            // a ProbeMatches message arrives.
            //
            // Note that Types, Scopes and MatchBy rule are all optional.
            // If scopes are specified but MatchBy rule is not, then RFC2396
            // scope matching rule is used as defined by WS-Discovery specifications.
            //
            // Also note that this sample does not cover the usage of Types.
            // If you need to send a Probe message with Types, you should
            // include the IWSDXMLContext when calling WSDCreateDiscoveryProvider
            // above, and build your own WSD_NAME_LIST structure here.  Building
            // this structure requires you to call IWSDXMLContext::AddNameToNamespace
            // method in order to extract WSDXML_NAME objects from the XML context.
            //
            // Please see MSDN documentation for details.
            //
            // SearchByType is also an async call.  See above for comments.
            hr = provider->SearchByType( NULL, scopesList , matchByRule, tag );

            if ( S_OK != hr )
            {
                PrintErrorMessage( L"SearchByType", hr );
            }
        }

        (void)_getch(); // Wait for key to be pressed for client termination
    }

    // Key has been pressed to terminate client

    if ( NULL != provider && isSinkAttached )
    {
        // This detaches the notification sink.  We do this regardless whether
        // S_OK == hr or not, and it is important to do this upon exit if a
        // notification sink has previously been attached to the provider.  Not
        // doing so may result in an access violation when the provider is being
        // destroyed.  When this method returns, all callback methods would have 
        // been completed, and will cease to listen to any more messages.
        //
        // Use hr2 so that the original value of hr is not lost.
        hr2 = provider->Detach();

        if ( S_OK != hr2 )
        {
            PrintErrorMessage( L"Failed to detach", hr2 );
        }
    }

    // clean up and exit routines below

    if ( NULL != tag )
    {
        delete[] tag;
        tag = NULL;
    }

    if ( NULL != sink )
    {
        sink->Release();
        sink = NULL;
    }

    if ( NULL != tempNotify )
    {
        tempNotify->Release();
        tempNotify = NULL;
    }

    if ( NULL != provider )
    {
        provider->Release();
        provider = NULL;
    }

    if ( NULL != epr )
    {
        delete[] epr;
        epr = NULL;
    }

    if ( NULL != matchByRule )
    {
        delete[] matchByRule;
        matchByRule = NULL;
    }

    if ( NULL != scopesList )
    {
        WSDFreeLinkedMemory( scopesList );
        scopesList = NULL;
    }

    if ( S_OK != hr )
    {
        // display usage if it fails for any reason
        wprintf( L"\r\n" );
        DisplayUsages();
        wprintf( L"\r\n" );
    }

    wprintf( L"WS-Discovery SDK Sample Client Terminated\r\n" );
    wprintf( L"Press any key to exit..." );
    (void)_getch();

    return 0;
}

void DisplayUsages()
{
    wprintf( L"Usages: WSDiscoveryClient.exe [/a EndpointReference] | [/sr | /sc] scope [...]\r\n" );
    wprintf( L"You can specify exactly one of /a, /sr or /sc - not a combination of them.\r\n" );
    wprintf( L"\r\n" );
    wprintf( L"An EndpointReference is the ID of the target service, which must be a valid\r\n" );
    wprintf( L"URI as per Section 2.6 of the WS-Discovery Specifications.  Specifically, it must\r\n" );
    wprintf( L"begin with one of http://, https://, uri:, urn:uuid: or uuid:.\r\n" );
    wprintf( L"\r\n" );
    wprintf( L"Examples:\r\n" );
    wprintf( L"\r\n" );
    wprintf( L"To find all WS-Discovery enabled target services: (Probe)\r\n" );
    wprintf( L"WSDiscoveryClient.exe\r\n" );
    wprintf( L"\r\n" );
    wprintf( L"To find a device with a given EndpointReference: (Resolve)\r\n" );
    wprintf( L"WSDiscoveryClient.exe /a urn:uuid:91022ed0-7d3d-11de-8a39-0800200c9a66\r\n" );
    wprintf( L"\r\n" );
    wprintf( L"To find a device with two scopes using the RFC2396 scope matching rule: (Probe)\r\n" );
    wprintf( L"WSDiscoveryClient.exe /sr http://www.contoso.com/wsd/scope1 " );
    wprintf( L"http://www.contoso.com/wsd/scope2 \r\n" );
    wprintf( L"\r\n" );
    wprintf( L"To find a device with a given scope using the custom scope matching rule: (Probe)\r\n" );
    wprintf( L"WSDiscoveryClient.exe /sc http://www.contoso.com/wsd/special/123 \r\n" );
}

// Generates a tag name of the following format:
// Tag0000
// where 0000 is a random number between 0000 to 9999.
// The caller should call delete[] on the string when
// it is no longer needed.
_Success_( return == S_OK )
HRESULT GenerateTag
(   _Outptr_ LPCWSTR *generatedTag
)
{
    HRESULT hr = S_OK;
    LPWSTR tempTag = NULL;
    int randomNum = 0;

    if ( NULL == generatedTag )
    {
        hr = E_POINTER;
    }
    else
    {
        *generatedTag = NULL;
    }

    if ( S_OK == hr )
    {
        // 12345678
        // Tag0000
        tempTag = new WCHAR[8];
        
        if ( NULL == tempTag )
        {
            hr = E_OUTOFMEMORY;
        }
    }

    if ( S_OK == hr )
    {
        // generate a random number between 0000 to 9999
        srand( (unsigned int)time( NULL ) );
        randomNum = rand() % 10000;

        // create the tag string
        // (StringCchPrintfW always NULL terminates the string)
        hr = StringCchPrintfW( tempTag, 8, L"Tag%04d", randomNum );
    }

    if ( S_OK == hr )
    {
        // outside pointer now owns temp string
        *generatedTag = tempTag;
        tempTag = NULL;
    }

    if ( NULL != tempTag )
    {
        delete[] tempTag;
        tempTag = NULL;
    }

    return hr;
}

// Parses the command line arguments according to rules
// specified in the DisplayUsages() function.
// The caller should pass in argc and argv as they were
// being passed into the wmain() function.
// The caller should call WSDFreeLinkedMemory on the
// returned scopesList, and delete[] on epr and
// matchByRule when they are no longer needed.
_Success_( return == S_OK )
HRESULT ParseArguments
(   _In_ int argc
,   _In_reads_( argc ) LPWSTR* argv
,   _Outptr_result_maybenull_ LPCWSTR *epr
,   _Outptr_result_maybenull_ WSD_URI_LIST **scopesList
,   _Outptr_result_maybenull_ LPCWSTR *matchByRule
)
{
    HRESULT hr = S_OK;
    LPWSTR tempEpr = NULL;
    LPWSTR tempMatchByRule = NULL;
    WSD_URI_LIST *tempScopesList = NULL;

    // should be at least 1 - 1st arg = exe name
    if ( 1 > argc || NULL == argv || NULL == *argv )
    {
        hr = E_INVALIDARG;
    }
    else if ( NULL == epr ||
              NULL == scopesList || 
              NULL == matchByRule )
    {
        hr = E_POINTER;
    }
    else
    {
        *epr = NULL;
        *scopesList = NULL;
        *matchByRule = NULL;
    }

    // expecting arguments to be in this format:
    // argv[0] = executable name - ignore
    // argv[1] = if present, must be one of /a, /sr or /sc
    //           if not present, then argc = 1

    // if argv[1] = /a,
    // then argv[2] = EndpointReference
    // argc must be = 3

    // if argv[1] = /sr or /sc,
    // then argv[2] and above will be the scopes
    //              to search for
    // argc must be >= 3

    if ( 1 >= argc )
    {
        // do nothing - no parsing required
        // will discover all WS-Discovery enabled services
        // Probe
    }
    else if ( 0 == wcscmp( L"/a", argv[1] ) )
    {
        // search using endpoint reference address
        // Resolve
        
        // argc must be = 3
        if ( 3 != argc )
        {
            hr = E_INVALIDARG; // fail to parse
        }

        if ( S_OK == hr )
        {
            hr = DeepCopyString(argv[2], &tempEpr); // deep copy epr
        }

        if ( S_OK == hr )
        {
            // outside pointer now owns temp epr
            *epr = tempEpr;
            tempEpr = NULL;
        }
    }
    else if ( 0 == wcscmp( L"/sr", argv[1] ) ||
              0 == wcscmp( L"/sc", argv[1] ) )
    {
        // search using scopes
        // Probe

        // argc must be >= 3
        if ( 3 > argc )
        {
            hr = E_INVALIDARG; // fail to parse
        }

        if ( S_OK == hr )
        {
            // /sr - use RFC2396
            // /sc - use Customs Rule

            if ( L'r' == argv[1][2] ) // /sr
            {
                hr = DeepCopyString( MATCHBY_RFC2396, &tempMatchByRule );
            }
            else // /sc
            {
                hr = DeepCopyString( MATCHBY_CUSTOM, &tempMatchByRule );
            }
        }

        if ( S_OK == hr )
        {
            // parse the scopes beginning with argv[2] and above
            hr = ParseScopes( argc, argv, 2, &tempScopesList );
        }

        if ( S_OK == hr )
        {
            // outside pointer now owns the temp list
            // and the match by rule
            *scopesList = tempScopesList;
            tempScopesList = NULL;

            *matchByRule = tempMatchByRule;
            tempMatchByRule = NULL;
        }
    }
    else
    {
        hr = E_INVALIDARG;
    }

    if ( NULL != tempScopesList )
    {
        WSDFreeLinkedMemory( tempScopesList );
        tempScopesList = NULL;
    }

    if ( NULL != tempEpr )
    {
        delete[] tempEpr;
        tempEpr = NULL;
    }

    if ( NULL != tempMatchByRule )
    {
        delete[] tempMatchByRule;
        tempMatchByRule = NULL;
    }

    return hr;
}

HRESULT ValidateEndpointReference
(   _In_ LPCWSTR epr
)
{
    HRESULT hr = S_OK;
    LPCWSTR tempEpr = NULL; // (soft copy) used to determine if this is a logical or physical address
    
    if ( NULL == epr )
    {
        hr = E_INVALIDARG;
    }
    
    if ( S_OK == hr )
    {
        // Determine whether this EndpointReference begins with one of
        // http://, https:// or uri:
        if ( NULL != wcsstr( epr, L"http://" ) ||
             NULL != wcsstr( epr, L"https://" ) ||
             NULL != wcsstr( epr, L"uri:" ) )
        {
            // this is a valid EndpointReference beginning with
            // http://, https:// or uri:
            
            tempEpr = epr;
        }
    }
    
    if ( S_OK == hr && NULL == tempEpr )
    {
        // Determine whether this EndpointReference begins with urn:uuid:
        tempEpr = wcsstr( epr, L"urn:uuid:" );
        
        if ( NULL != tempEpr )
        {
            // The EndpointReference begins with urn:uuid:.
            
            //          1         2        3          4
            // 123456789012345678901234567890123456789012345
            // urn:uuid:f452f1ae-fbb4-11de-a6bb-00cc30bfc300
            
            // It must therefore be exactly 45 characters long
            if ( wcslen( tempEpr ) != 45 )
            {
                hr = E_INVALIDARG;
            }
        }
    }
    
    if ( S_OK == hr && NULL == tempEpr )
    {
        // Determine whether this EndpointReference begins with urn:uuid:
        tempEpr = wcsstr( epr, L"uuid:" );
        
        if ( NULL != tempEpr )
        {
            // The EndpointReference begins with uuid:.
            
            //          1         2        3          4
            // 12345678901234567890123456789012345678901
            // uuid:f452f1ae-fbb4-11de-a6bb-00cc30bfc300
            
            // It must therefore be exactly 41 characters long
            if ( wcslen( tempEpr ) != 41 )
            {
                hr = E_INVALIDARG;
            }
        }
    }
    
    if ( S_OK == hr && NULL == tempEpr )
    {
        // The EndpointReference does not begin with one of the
        // appropriate schemes.
        
        hr = E_INVALIDARG;
    }
    
    tempEpr = NULL;
    return hr;
}
