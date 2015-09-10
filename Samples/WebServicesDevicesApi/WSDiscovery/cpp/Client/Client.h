//////////////////////////////////////////////////////////////////////////////
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include <windows.h>
#include <wsdapi.h>

void DisplayUsages();

// Parses the given command line arguments.
// The caller should delete[] epr and matchByRule.
// The caller should also deallocate scopesList by calling
// WSDFreeLinkedMemory.
_Success_( return == S_OK )
HRESULT ParseArguments
(   _In_ int argc
,   _In_reads_( argc ) LPWSTR* argv
,   _Outptr_result_maybenull_ LPCWSTR *epr
,   _Outptr_result_maybenull_ WSD_URI_LIST **scopesList
,   _Outptr_result_maybenull_ LPCWSTR *matchByRule
);

// Generates a tag for use when sending
// out search queries (Probe or Resolve)
//
// The tag carries a format of "Tag0000"
// where 0000 is a decimal number ranging
// from 0000 to 9999.
//
// The caller should delete[] the tag string
// returned by this function.
_Success_( return == S_OK )
HRESULT GenerateTag
(   _Outptr_ LPCWSTR *generatedTag
);

// Determines whether the given EndpointReference
// is valid or not.  This is purely for cosmetic
// purposes on the client such that the client
// will cease to send a Resolve (SearchById) message
// if the EnpointReference is invalid.  Without this
// check, the client will proceed to sending the message
// on the network, but it will fail to receive any responses
// since no target services are being associated to the
// given (invalid) EndpointReference.
//
// An EndpointReference is valid if it starts with
// the following schemes:
// - http://
// - https://
// - uri:
// - uuid:      (must be 41 characters long)
// - urn:uuid:  (must be 45 characters long)
//
// Returns S_OK if the EndpointReference is valid,
// or E_INVALIDARG if the EndpointReference is invalid
// or if epr is NULL.
HRESULT ValidateEndpointReference
(   _In_ LPCWSTR epr
);
