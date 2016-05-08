// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include <windows.h>
#include <objidl.h>  
#include <rpcsal.h>

#include <msxml6.h>

#define MemAlloc(size)      ((PVOID) LocalAlloc(LMEM_FIXED, (size)))
#define MemAllocZ(size)     ((PVOID) LocalAlloc(LPTR, (size)))
#define MemFree(p)          { if (p) LocalFree((HLOCAL) (p)); }

VOID DisplayUsage(
    VOID
    );

VOID vFormatAndPrint(
        IN  DWORD dwMsgId
    );

HRESULT
IStreamToDOMDocument(
    __inout     IStream           *pStream,
    __deref_out IXMLDOMDocument2 **ppXmlDocument
    );

HRESULT
DOMDocumentToIStream(
    __in     IXMLDOMDocument2 *pXmlDocument,
    __inout  IStream          *pStream
    );