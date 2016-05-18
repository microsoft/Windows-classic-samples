// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

const   DWORD gdwVersion       = 1;

HRESULT
RelaseStateInfoContents(
    __inout PPTPC_STATE_INFO psi);

HRESULT
GetUserPrintTicketStream(
    __in        PPTPC_STATE_INFO   psi,
    __deref_out IStream           **ppPrintTicketStream
   );

HRESULT
ConvertPTPCStreamToDOM(
    __in        IStream           *pPTPCStream,
    __deref_out IXMLDOMDocument2 **ppXMLDOM
    );

HRESULT
ConvertPTStreamToBuffer(
    __in                            IStream *pStream,
    __deref_out_bcount(*pcbPTBuf)   PBYTE   *ppbPTBuf,
    __out                           PDWORD  pcbPTBuf
    );

HRESULT
GetPrintCapabilitiesBasedOnPrintTicket(
    __inout     PPTPC_STATE_INFO   psi,
    __in        IStream           *pPrintTicketStream,
    __deref_out IStream          **ppPrintCapsStream
    );

HRESULT
CreatePTFeatureOptionNodeFromPrintCapOptionNode(
    __in        PPTPC_STATE_INFO   psi,
    __in        IXMLDOMNode       *pPrintCapsOptionNode,
    __deref_out IXMLDOMNode      **ppPrintTicketFeatureNode
    );

HRESULT 
ConvertFullPrintTicketToMinimalPrintTicket(
    __inout  IXMLDOMDocument2  *pPrintTicketDOM
    );

HRESULT
MergeNodeIntoMinimalPrintTicket(
    __in  IXMLDOMDocument2  *pPrintTicketMinimal, 
    __in  IXMLDOMNode       *pNode);

