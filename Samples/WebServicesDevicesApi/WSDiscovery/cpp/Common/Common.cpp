//////////////////////////////////////////////////////////////////////////////
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//////////////////////////////////////////////////////////////////////////////

#include <winsock2.h>
#include <windows.h>
#include <stdlib.h>
#include <strsafe.h>
#include <string.h>
#include <wsdapi.h>
#include "Common.h"

#define TEMP_HOST_NAME_LENGTH 80 // length of temporary host name

// length of temporary guid string
// 0        1         2         3
// 123456789012345678901234567890123456  +1 for NULL char
// 91022ed0-7d3d-11de-8a39-0800200c9a66
#define TEMP_GUID_STRING_LENGTH 37

// helper method for DeepCopyString and DeepCopyStringLinked
// - they both have similar routines, except that DeepCopyString
// allocates memory using the "new" keyword, and DeepCopyStringLinked
// uses WSDAllocateLinkedMemory to allocate memory for the string and links
// it to the specified parent
_Success_( return == S_OK )
static HRESULT _DeepCopyStringHelper
(   _In_ BOOL isLinked
,   _In_ LPCWSTR source
,   _In_opt_ void *parent
,   _Outptr_ LPWSTR *dest
);

HRESULT DeepCopyString
(   _In_ LPCWSTR source
,   _Outptr_ LPWSTR *dest
)
{
    return _DeepCopyStringHelper( FALSE, source, NULL, dest );
}

HRESULT DeepCopyStringLinked
(   _In_ LPCWSTR source
,   _In_opt_ void *parent
,   _Outptr_ LPWSTR *dest
)
{
    return _DeepCopyStringHelper( TRUE, source, parent, dest );
}

_Success_( return == S_OK )
HRESULT _DeepCopyStringHelper
(   _In_ BOOL isLinked
,   _In_ LPCWSTR source
,   _In_opt_ void *parent
,   _Outptr_ LPWSTR *dest
)
{
    HRESULT hr = S_OK;
    size_t stringLength = 0;
    LPWSTR tempDest = NULL;

    if ( NULL == source )
    {
        hr = E_INVALIDARG;
    }
    else if ( NULL == dest )
    {
        hr = E_POINTER;
    }
    else
    {
        *dest = NULL;
    }

    if ( S_OK == hr )
    {
        // determine length of source string
        stringLength = wcslen(source);
    }

    if ( S_OK == hr )
    {
        // num char = stringLength + 1 (for NULL char)
        if ( isLinked )
        {
            tempDest = (WCHAR *)WSDAllocateLinkedMemory( 
                parent, sizeof( WCHAR ) * ( stringLength + 1 ) );
        }
        else
        {
            tempDest = new WCHAR[stringLength + 1];
        }

        if ( NULL == tempDest )
        {
            hr = E_OUTOFMEMORY;
        }
    }

    if ( S_OK == hr )
    {
        // deep copy string
        hr = StringCchCopyW( tempDest, stringLength + 1, source );
    }

    if ( S_OK == hr )
    {
        // outside pointer now owns the temp string
        *dest = tempDest;
        tempDest = NULL;
    }

    if ( NULL != tempDest )
    {
        if ( isLinked )
        {
            WSDFreeLinkedMemory( tempDest );
        }
        else
        {
            delete[] tempDest;
        }

        tempDest = NULL;
    }

    return hr;
}

_Success_( return == S_OK )
HRESULT GetWideStringHostName
(   _Outptr_ LPWSTR *hostName
)
{
    HRESULT hr = S_OK;
    LPWSTR tempHostName = NULL;
    WSADATA wsaData = {0};
    int returnCode = 0;
    size_t stringLength = 0;
    WORD versionRequested = 0;
    char asciiHostName[TEMP_HOST_NAME_LENGTH] = {0};
    BOOL isWinsockStarted = FALSE;

    if ( NULL == hostName )
    {
        hr = E_POINTER;
    }
    else
    {
        *hostName = NULL;
    }

    if ( S_OK == hr )
    {
        // use v2.2 for Windows XP and above
        versionRequested = MAKEWORD( 2, 2 );
        returnCode = WSAStartup( versionRequested, &wsaData );

        if ( 0 != returnCode )
        {
            // no usable winsock dll available
            hr = HRESULT_FROM_WIN32( (DWORD)WSAGetLastError() );
        }
        else
        {
            isWinsockStarted = TRUE;
        }
    }

    if ( S_OK == hr )
    {
        // get the host name in ascii string
#pragma prefast( suppress: 38026, "This sample uses ASCII version of host names only for simplicity." )
        returnCode = gethostname( asciiHostName, sizeof( asciiHostName ) );

        if ( 0 != returnCode )
        {
            // failed to retrieve host name
            hr = HRESULT_FROM_WIN32( (DWORD)WSAGetLastError() );
        }
    }
    if ( S_OK == hr )
    {
        // create enough memory allocation for the wide string host name
        stringLength = strlen( asciiHostName );
        tempHostName = new WCHAR[stringLength + 1]; // + 1 for NULL char

        if ( NULL == tempHostName )
        {
            hr = E_OUTOFMEMORY;
        }
    }

    if ( S_OK == hr )
    {
        // convert the host name from ascii string to unicode string
        hr = StringCchPrintfW( tempHostName, stringLength + 1, L"%S", asciiHostName );
    }

    if ( S_OK == hr )
    {
        // ensure that the last char is NULL
        tempHostName[stringLength] = L'\0';

        // outside pointer now owns the temp host name
        *hostName = tempHostName;
        tempHostName = NULL;
    }

    if ( isWinsockStarted )
    {
        (void)WSACleanup(); // don't care about the return code
    }

    if ( NULL != tempHostName )
    {
        delete[] tempHostName;
        tempHostName = NULL;
    }

    return hr;
}

_Success_( return == S_OK )
HRESULT DeepCopyWsdUriList
(   _In_ const WSD_URI_LIST *srcList
,   _Outptr_ WSD_URI_LIST **destList
)
{
    HRESULT hr = S_OK;
    WSD_URI_LIST *duplicatedList = NULL;
    WSD_URI_LIST *tempList = NULL; // soft copy of current node for iteration - do not delete

    if ( NULL == srcList )
    {
        hr = E_INVALIDARG;
    }
    else if ( NULL == destList )
    {
        hr = E_POINTER;
    }
    else
    {
        *destList = NULL;
    }

    while ( S_OK == hr && NULL != srcList )
    {
        if ( NULL == duplicatedList )
        {
            // first node to be duplicated - create the first node in the list
            duplicatedList = (WSD_URI_LIST *)WSDAllocateLinkedMemory( NULL, sizeof( WSD_URI_LIST ) );
            
            if ( NULL == duplicatedList )
            {
                hr = E_OUTOFMEMORY;
            }
            else
            {
                // use the temp list for subsequent nodes
                tempList = duplicatedList;
            }
        }
        else
        {
            // subsequent nodes - create the next node
            tempList->Next = (WSD_URI_LIST *)WSDAllocateLinkedMemory( 
                tempList, // parent
                sizeof( WSD_URI_LIST ) ); // child

            if ( NULL == tempList->Next )
            {
                hr = E_OUTOFMEMORY;
            }
            else
            {
                tempList = tempList->Next;
            }
        }
        
        if ( S_OK == hr )
        {
            // initialize the node
            tempList->Element = NULL;
            tempList->Next = NULL;
            
            // copy the uri string
            hr = DeepCopyStringLinked( 
                    srcList->Element, 
                    tempList, // parent
                    const_cast<LPWSTR*>( &(tempList->Element) ) ); // child
        }
        
        if ( S_OK == hr )
        {
            // move to next node
            srcList = srcList->Next;
        }
    }

    if ( S_OK == hr )
    {
        // outside pointer now owns the duplicated list
        *destList = duplicatedList;
        duplicatedList = NULL;
    }

    if ( NULL != tempList )
    {
        tempList = NULL; // do not delete
    }

    if ( NULL != duplicatedList )
    {
        WSDFreeLinkedMemory( duplicatedList );
        duplicatedList = NULL;
    }

    return hr;
}

_Success_( return == S_OK )
HRESULT ParseScopes
(   _In_ int argc
,   _In_reads_( argc ) LPWSTR *argv
,   _In_ int startIndex
,   _Outptr_result_maybenull_ WSD_URI_LIST **scopesList
)
{
    // Create linked list of scopes.
    // 
    // Here is a quick overview of how we use linked memory here,
    // and how we useWSDAttachLinkedMemory and WSDFreeLinkedMemory
    // to accomplish this.
    //
    // Example:
    // tempScopeList [WSD_URI_LIST - top level parent (i.e. parent: NULL)]
    // --> Element: string1 (parent: tempScopeList - this node owns the string)
    // --> Next: tempScopeList2 (parent: tempScopeList - this node owns the next node)
    //           --> Element: string2 (parent: tempScopeList2)
    //           --> Next: tempScopeList3 (parent: tempScopeList2)
    //                     --> Element: string3 (parent: tempScopeList3)
    //                     --> Next: NULL
    // 
    // If all of the above elements and nodes are allocated using
    // WSDAttachLinkedMemory, you can use WSDFreeLinkedMemory to
    // deallocate all of the above memory allocations in one function call
    // rather than to have a for loop and delete them one by one.
    
    HRESULT hr = S_OK;
    WSD_URI_LIST *tempScopesList = NULL;
    WSD_URI_LIST *parentNode = NULL; // do not deallocate
    WSD_URI_LIST **nextNode = NULL; // do not deallocate
    int i = 0; // for loop

    if ( 1 > argc || NULL == argv ||
        0 > startIndex || argc <= startIndex )
    {
        // Required conditions to pass:
        // argc >= 1
        // argv != NULL
        // 0 <= startIndex < argc
        hr = E_INVALIDARG;
    }
    else if ( NULL == scopesList )
    {
        hr = E_POINTER;
    }
    else
    {
        *scopesList = NULL;
    }

    nextNode = &tempScopesList;
    parentNode = NULL;

    for ( i = startIndex; i < argc && S_OK == hr; i++ )
    {
        // create the node in the memory of previous_node->next
        *nextNode = (WSD_URI_LIST *)WSDAllocateLinkedMemory(
            parentNode, sizeof( WSD_URI_LIST ) );

        if ( NULL == *nextNode )
        {
            hr = E_OUTOFMEMORY;
        }
        
        if ( S_OK == hr )
        {
            (*nextNode)->Next = NULL;

            // copy string to the node
            _Analysis_assume_(NULL != *nextNode);
            hr = DeepCopyStringLinked( argv[i], *nextNode, 
                const_cast<LPWSTR *>( &((*nextNode)->Element) ) );
        }

        if ( S_OK == hr )
        {
            // move to next node (memory wise)
            parentNode = *nextNode;
            nextNode = &((*nextNode)->Next);
        }
    }

    // Do not deallocate nextNode and parentNode.
    // Those references are held by tempScopesList.
    nextNode = NULL;
    parentNode = NULL;

    if ( S_OK == hr )
    {
        // outside pointer now owns the list
        *scopesList = tempScopesList;
        tempScopesList = NULL;
    }

    if ( NULL != tempScopesList )
    {
        WSDFreeLinkedMemory( tempScopesList );
        tempScopesList = NULL;
    }

    return hr;
}

_Success_( return == S_OK )
HRESULT GetGuidString
(   _In_ GUID guidToConvert
,   _Outptr_ LPWSTR *guidString
)
{
    HRESULT hr = S_OK;
    LPWSTR tempGuidString = NULL;

    if ( NULL == guidString )
    {
        hr = E_POINTER;
    }
    else
    {
        *guidString = NULL;
    }

    if ( S_OK == hr )
    {
        tempGuidString = new WCHAR[TEMP_GUID_STRING_LENGTH];

        if ( NULL == tempGuidString )
        {
            hr = E_OUTOFMEMORY;
        }
    }

    if ( S_OK == hr )
    {
        hr = StringCchPrintfW( tempGuidString, TEMP_GUID_STRING_LENGTH, 
            L"%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
            guidToConvert.Data1, guidToConvert.Data2, guidToConvert.Data3,
            guidToConvert.Data4[0], guidToConvert.Data4[1], guidToConvert.Data4[2],
            guidToConvert.Data4[3], guidToConvert.Data4[4], guidToConvert.Data4[5],
            guidToConvert.Data4[6], guidToConvert.Data4[7] );
    }

    if ( S_OK == hr )
    {
        // outside pointer now owns the temp string
        *guidString = tempGuidString;
        tempGuidString = NULL;
    }

    if ( NULL != tempGuidString )
    {
        delete[] tempGuidString;
        tempGuidString = NULL;
    }

    return hr;
}

void PrintGuid
(   _In_ GUID guidToPrint
)
{
    HRESULT hr = S_OK;
    LPWSTR guidString = NULL;

    // obtain the guid string
    hr = GetGuidString( guidToPrint, &guidString );

    if ( S_OK == hr )
    {
        wprintf( guidString );
    }
    else
    {
        PrintErrorMessage( L"Failed to print GUID string", hr );
    }

    if ( NULL != guidString )
    {
        delete[] guidString;
        guidString = NULL;
    }
}

void PrintErrorMessage
(   _In_opt_ LPCWSTR message
,   _In_ HRESULT hr
)
{
    wprintf( L"%s: [0x%08X]\r\n", message, hr );
}

HRESULT PrintDiscoveredService
(   _In_ IWSDiscoveredService *service
,   _In_ BOOL isByeMessage
)
{
    HRESULT hr = S_OK;
    LPCWSTR probeResolveTag = NULL; // do not deallocate
    WSD_ENDPOINT_REFERENCE *wsdEpr = NULL; // do not deallocate
    ULONGLONG instanceId = 0;
    WSD_NAME_LIST *typesList = NULL; // do not deallocate
    WSD_URI_LIST *scopesList = NULL; // do not deallocate
    WSD_URI_LIST *xAddrsList = NULL; // do not deallocate
    ULONGLONG metadataVersion = 0;
    LPCWSTR localAddress = NULL; // do not deallocate
    LPCWSTR remoteAddress = NULL; // do not deallocate
    GUID localGuid = { 0 };
    WSDXML_ELEMENT *extendedXmlHeader = NULL; // do not deallocate
    WSDXML_ELEMENT *extendedXmlBody = NULL; // do not deallocate

    if ( NULL == service )
    {
        hr = E_INVALIDARG;
    }
    
    if ( S_OK == hr )
    {
        hr = service->GetEndpointReference( &wsdEpr );

        if ( S_OK != hr )
        {
            PrintErrorMessage( L"Failed to obtain EPR", hr );
        }
    }
    
    if ( !isByeMessage )
    {
        // If this is not a Bye message, we shall attempt to retrieve
        // all fields that are available in the discovered service.
        
        if ( S_OK == hr )
        {
            hr = service->GetProbeResolveTag( &probeResolveTag );

            if ( S_OK != hr )
            {
                PrintErrorMessage( L"Failed to obtain tag", hr );
            }
        }

        if ( S_OK == hr )
        {
            hr = service->GetInstanceId( &instanceId );

            if ( S_OK != hr )
            {
                PrintErrorMessage( L"Failed to obtain Instance ID", hr );
            }
        }

        if ( S_OK == hr )
        {
            hr = service->GetTypes( &typesList );

            if ( S_OK != hr )
            {
                PrintErrorMessage( L"Failed to obtain types list", hr );
            }
        }

        if ( S_OK == hr )
        {
            hr = service->GetScopes( &scopesList );

            if ( S_OK != hr )
            {
                PrintErrorMessage( L"Failed to obtain scopes list", hr );
            }
        }

        if ( S_OK == hr )
        {
            hr = service->GetXAddrs( &xAddrsList );

            if ( S_OK != hr )
            {
                PrintErrorMessage( L"Failed to obtain XAddrs list", hr );
            }
        }

        if ( S_OK == hr )
        {
            hr = service->GetMetadataVersion( &metadataVersion );

            if ( S_OK != hr )
            {
                PrintErrorMessage( L"Failed to obtain metadata version", hr );
            }
        }

        if ( S_OK == hr )
        {
            hr = service->GetLocalTransportAddress( &localAddress );

            if ( S_OK != hr )
            {
                PrintErrorMessage( L"Failed to obtain local transport address", hr );
            }
        }

        if ( S_OK == hr )
        {
            hr = service->GetRemoteTransportAddress( &remoteAddress );

            if ( S_OK != hr )
            {
                PrintErrorMessage( L"Failed to obtain remote transport address", hr );
            }
        }

        if ( S_OK == hr )
        {
            hr = service->GetLocalInterfaceGUID( &localGuid );

            if ( S_OK != hr )
            {
                PrintErrorMessage( L"Failed to obtain Local Interface GUID", hr );
            }
        }

        if ( S_OK == hr )
        {
            hr = service->GetExtendedDiscoXML(
                &extendedXmlHeader, &extendedXmlBody );

            if ( S_OK != hr )
            {
                PrintErrorMessage( L"Failed to extended discovery XML", hr );
            }
        }
    
    }
    
    if ( S_OK == hr )
    {
        if ( isByeMessage )
        {
            // only print the EndpointReference if this is a Bye message
            wprintf( L"EndpointReference: %s\r\n", wsdEpr->Address );
        }
        else
        {
            // print everything if this is not a Bye message
            
            wprintf( L"Tag: %s\r\n", probeResolveTag );
        
            wprintf( L"EndpointReference: %s\r\n", wsdEpr->Address );

            wprintf( L"EndpointReference Properties:\r\n" );

            if ( NULL == wsdEpr->ReferenceProperties.Any )
            {
                wprintf( L"  No EndpointReference Properties\r\n" );
            }
            else
            {
                PrintXmlElement( wsdEpr->ReferenceProperties.Any, 1 );
            }

            wprintf( L"EndpointReference Parameters:\r\n" );

            if ( NULL == wsdEpr->ReferenceParameters.Any )
            {
                wprintf( L"  No EndpointReference Parameters\r\n" );
            }
            else
            {
                PrintXmlElement( wsdEpr->ReferenceParameters.Any, 1 );
            }

            wprintf( L"InstanceId: %llu\r\n", instanceId );
            
            wprintf( L"Types:\r\n" );
            PrintNameList( typesList );
            
            wprintf( L"Scopes:\r\n" );
            PrintUriList( scopesList );

            wprintf( L"XAddrs:\r\n" );
            PrintUriList(xAddrsList);

            wprintf( L"Metadata Version: %llu\r\n", metadataVersion );
            wprintf( L"Local Transport Address: %s\r\n", localAddress );
            wprintf( L"Remote Transport Address: %s\r\n", remoteAddress );

            wprintf( L"Local Interface GUID: " );
            PrintGuid( localGuid );
            wprintf( L"\r\n" );

            wprintf( L"Extended XML Header:\r\n" );
            PrintXmlElement( extendedXmlHeader, 1 );
            
            wprintf( L"Extended XML Body:\r\n" );
            PrintXmlElement( extendedXmlBody, 1 );
        }
    }

    return hr;
}

void PrintNameList
(   _In_opt_ WSD_NAME_LIST *typesList
)
{
    if ( NULL == typesList )
    {
        wprintf( L"  None\r\n" );
    }
    else
    {
        while ( NULL != typesList )
        {
            wprintf( L"  " );

            if ( NULL == typesList->Element ||
                 NULL == typesList->Element->Space )
            {
                // Chances of this happening is rare, but we
                // handle it anyway.
                wprintf( L"ERROR: Type or namespace missing" );
            }
            else
            {
                // Since the URI of the namespace is too long, we have opted not to
                // have it printed here.  However, you may access that information
                // through typesList->Element->Space->Uri.
                wprintf( L"%s:%s", typesList->Element->Space->PreferredPrefix,
                    typesList->Element->LocalName );
            }

            wprintf( L"\r\n" );

            typesList = typesList->Next;
        }
    }
}

void PrintUriList
(   _In_opt_ WSD_URI_LIST *scopesList
)
{
    if ( NULL == scopesList )
    {
        wprintf( L"  None\r\n" );
    }
    else
    {
        while ( NULL != scopesList )
        {
            wprintf( L"  %s\r\n", scopesList->Element );
            scopesList = scopesList->Next;
        }
    }
}

void PrintXmlElement
(   _In_opt_ WSDXML_ELEMENT *element
,   UINT indentLevel
)
{
    WSDXML_ATTRIBUTE *tempAttribute = NULL; // iteration - do not delete

    PrintIndentTabs( indentLevel );

    if ( NULL == element )
    {
        wprintf( L"No XML element\r\n" );
    }
    else
    {
        if ( NULL == element->Name ||
             NULL == element->Name->Space )
        {
            // This is rare, but we still handle it.
            wprintf( L"ERROR: Malformed name" );
        }
        else
        {
            wprintf( L"%s:%s", 
                element->Name->Space->PreferredPrefix, 
                element->Name->LocalName );
        }

        wprintf( L" " );

        if ( NULL == element->FirstAttribute )
        {
            wprintf( L"[No Attributes]" );
        }
        else
        {
            tempAttribute = element->FirstAttribute;

            while ( NULL != tempAttribute )
            {
                wprintf( L"[" );

                if ( NULL == tempAttribute->Name || NULL == tempAttribute->Name->Space )
                {
                    // it's odd for an attribute to exist without a name
                    // we will therefore ignore it
                    wprintf( L"ERROR: Malformed attribute" );
                }
                else
                {
                    // Since the URI of the namespace is too long, we have opted not to
                    // have it printed here. However, you may access that information
                    // through typesList->Element->Space->Uri.
                    wprintf( L"%s:%s: %s", tempAttribute->Name->Space->PreferredPrefix,
                        tempAttribute->Name->LocalName, tempAttribute->Value );
                }

                wprintf( L"] " );

                tempAttribute = tempAttribute->Next;
            }
        }

        wprintf( L"\r\n" );
        
        // We limit recursion level to 255 to avoid stack overflow problems
        // which can be a security risk.  In addition, we only print when a
        // child is present.
        if ( NULL != element->FirstChild && 255 > indentLevel )
        {
            PrintXmlNode( element->FirstChild, indentLevel + 1 );
        }
    }

    tempAttribute = NULL;
}

void PrintXmlNode
(   _In_opt_ WSDXML_NODE *node
,   UINT indentLevel
)
{
    PrintIndentTabs( indentLevel );

    if ( NULL == node )
    {
        wprintf( L"No XML Node\r\n" );
    }
    else
    {
        while ( NULL != node )
        {
            if ( WSDXML_NODE::ElementType == node->Type )
            {
                // same indent level
                PrintXmlElement( (WSDXML_ELEMENT*)node, indentLevel );
            }
            else // TextType
            {
                wprintf( L"%s\r\n", ((WSDXML_TEXT*)node)->Text );
            }

            node = node->Next;
        }
    }
}

void PrintIndentTabs
(   _In_ UINT indentLevel
)
{
    UINT i = 0;

    for ( i = 0; i < indentLevel; i++ )
    {
        wprintf( L"  " );
    }
}
