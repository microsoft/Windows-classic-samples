//////////////////////////////////////////////////////////////////////////////
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//////////////////////////////////////////////////////////////////////////////

// Common include file and utility functions for both
// WSDiscovery Client and Target Service.

#pragma once

#include <windows.h>
#include <stdlib.h>
#include <wsdapi.h>

// RFC2396 scope matching rule as defined in the WS-Discovery specifications.
#define MATCHBY_RFC2396  L"http://schemas.xmlsoap.org/ws/2005/04/discovery/rfc2396"

// Custom scope matching rule used in this SDK sample.
#define MATCHBY_CUSTOM   L"http://www.example.org/ws/2009/07/discovery/odd"

// Deep copies the string.
// The caller should delete[] the destination string.
HRESULT DeepCopyString
(   _In_ LPCWSTR source
,   _Outptr_ LPWSTR *dest
);

// Deep copies the string from source to dest.
// This method is almost identical to DeepCopyString,
// except that this uses WSDAllocateLinkedMemory to
// create the string memory and link it to the
// specified parent.  If parent is NULL, then the
// returned string through dest is the top level
// parent.  The caller should call WSDFreeLinkedMemory
// on the top level parent (such as this string if
// parent is NULL, or may be a WSD_URI_LIST) to
// deallocate memory allocated through this method.
HRESULT DeepCopyStringLinked
(   _In_ LPCWSTR source
,   _In_opt_ void *parent
,   _Outptr_ LPWSTR *dest
);

// Gets the host name of the machine
// in a unicode string.  The caller should
// delete[] the string.
_Success_( return == S_OK )
HRESULT GetWideStringHostName
(   _Outptr_ LPWSTR *hostName
);

// Deep copies a WSD_URI_LIST.
// The caller should call WSDFreeLinkedMemory to delete
// all nodes and elements of the linked list.
_Success_( return == S_OK )
HRESULT DeepCopyWsdUriList
(   _In_ const WSD_URI_LIST *srcList
,   _Outptr_ WSD_URI_LIST **destList
);

// Parses the scopes from the command line.
// argc = # elements in argv (from the main() arguments)
// argv = array of wide strings (from the main() arguments)
// uiStartFromArgv = the index on argv that contains the first scope
// scopesList = the returned WSD_URI_LIST of scopes
//
// The caller should call WSDFreeLinkedMemory to delete all
// nodes and elements of the linked list.
_Success_( return == S_OK )
HRESULT ParseScopes
(   _In_ int argc
,   _In_reads_( argc ) LPWSTR *argv
,   _In_ int startIndex
,   _Outptr_result_maybenull_ WSD_URI_LIST **scopesList
);

// Converts the GUID into a string of the following format:
// 01234567-89AB-CDEF-0123-456789ABCDEF
// The caller should delete[] the string when it is
// no longer needed.
_Success_( return == S_OK )
HRESULT GetGuidString
(   _In_ GUID guidToConvert
,   _Outptr_ LPWSTR *guidString
);

// Prints the given GUID in the format of
// 01234567-89AB-CDEF-0123-456789ABCDEF
void PrintGuid
(   _In_ GUID guidToPrint
);

// Prints the error message in the format
// Message: [HRESULT] {newline}
//
// Example:
// message = L"An error has occured"
// hr = E_FAIL
//
// This will print
// An Error has occured: [0x80004005]
//
void PrintErrorMessage
(   _In_opt_ LPCWSTR message
,   _In_ HRESULT hr
);

// Prints out the contents in the discovered service.
// If this is a Bye message, only the EndpointReference is
// printed as other information on the discovered service
// will not be available.
HRESULT PrintDiscoveredService
(   _In_ IWSDiscoveredService *service
,   _In_ BOOL isByeMessage
);

// Prints out the types list.
void PrintNameList
(   _In_opt_ WSD_NAME_LIST *typesList
);

// Prints out the scopes list.
void PrintUriList
(   _In_opt_ WSD_URI_LIST *scopesList
);

// Prints out the XML element.
void PrintXmlElement
(   _In_opt_ WSDXML_ELEMENT *element
,   UINT indentLevel
);

// Prints out the entire tree of an XML Node.
void PrintXmlNode
(   _In_opt_ WSDXML_NODE *pNode
,   UINT indentLevel
);

// Prints out tabs for a given indentation level.
void PrintIndentTabs
(   _In_ UINT indentLevel
);
