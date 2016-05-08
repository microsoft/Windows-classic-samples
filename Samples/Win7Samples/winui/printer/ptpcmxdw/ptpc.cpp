// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "stdafx.h"
#include "prntvpt.h"

HRESULT
RelaseStateInfoContents(
    __inout PPTPC_STATE_INFO psi)
{
    HRESULT hr = S_OK;

    if ( psi->hPrinter )
    {
        ClosePrinter(psi->hPrinter);
        psi->hPrinter = NULL;
    }

    if ( psi->hProvider)
    {
        hr = PTCloseProvider(psi->hProvider);
        psi->hProvider = NULL;
    }

    return hr;
}

/*++
Routine Name:
    GetProviderWrapper

Routine Description:

    Before calling into PT APIs, we need to have a PrintTicket Provider. This routine calls
    the PTOpenProvider API to obtain the provider and caches the provider in psi. 
    If the provider is already cached, then that cached provider is returned.

Arguments:
    psi : The state info where the provider is cached.

Return Value:
    S_OK if successful.
    E_* otherwise.

--*/
HRESULT 
GetProviderWrapper(
    __inout PPTPC_STATE_INFO psi
    )
{
    HRESULT hr = S_OK;

    if ( NULL == psi )
    {
        hr = E_INVALIDARG;
    }
    else if ( NULL == psi->hProvider )
    {
        hr = PTOpenProvider( 
                            psi->szPrinterName,
                            gdwVersion,
                            &(psi->hProvider)
                            );        
    }
    
    return hr;
}

/*++
Routine Name:
    OpenPrinterWrapper

Routine Description:

    Opens a Printer handle and caches it. If handle already cached, then it does nothing.

Arguments:
    psi : The state info where the handle is cached.

Return Value:
    S_OK if successful.
    E_* otherwise.

--*/
HRESULT
OpenPrinterWrapper(
    __inout PPTPC_STATE_INFO psi
    )
{
    HRESULT hr = S_OK;

    if ( NULL == psi )
    {
        hr = E_INVALIDARG;
    }
    else if ( NULL == psi->hPrinter )
    {
        if ( FALSE == OpenPrinter(psi->szPrinterName, &(psi->hPrinter), NULL) )
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }
    return hr;
}

/*++
Routine Name:
    DocumentProperties

Routine Description:

    DocumentProperties is the best way to get the User Devmode 

Arguments:

    psi     : The state info where the handle is cached.
              This should also contain the hPrinter which is used for the call
              to GetPrinter
    ppDevMode: Pointer where the pointer to devmode is stored on exit.

Return Value:
    S_OK if successful.
    E_* otherwise.

--*/
HRESULT
GetDevmodeFromDocProperty(
    __inout     PPTPC_STATE_INFO psi,
    __deref_out LPDEVMODE       *ppDevMode
    )
{
    HRESULT  hr      = S_OK;
    PDEVMODE pdm     = NULL;
    LONG     lRetVal = -1;

    if ( NULL == psi ||
         NULL == ppDevMode ||
         NULL == psi->hPrinter )
    {
        hr = E_INVALIDARG;
    }

    if ( SUCCEEDED(hr) )
    {
        *ppDevMode = NULL;
        lRetVal = DocumentProperties(NULL,                 // handle to parent window 
                                      psi->hPrinter,        // handle to printer object
                                      psi->szPrinterName,   // device name
                                      NULL,                 // modified device mode
                                      NULL,                 // original device mode
                                      0                     // mode options
                       );

        if ( lRetVal > 0 )   
        {
            pdm = (PDEVMODE)MemAlloc(lRetVal);

            if ( NULL == pdm )
            {
                hr = E_OUTOFMEMORY;
            }
        }
        else if (0 == lRetVal )
        {
            hr = E_FAIL;
        }
        else
        {
            hr = HRESULT_FROM_WIN32(GetLastError() );
        }
    }

    if ( SUCCEEDED(hr) )
    {
        lRetVal = DocumentProperties(NULL,          
                                     psi->hPrinter, 
                                     psi->szPrinterName,
                                     pdm,
                                     NULL,
                                     DM_OUT_BUFFER
                                     );
                
        if ( IDOK == lRetVal )
        {
            *ppDevMode = pdm;
        }
        else if ( IDCANCEL == lRetVal ) 
        {
            // Shouldn't happen since we are not throwing UI
            hr = E_FAIL;
        }
        else
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }

    // If we allocated the memory but the second call to DocumentProperties
    // failed, we release the memory.
    if ( !SUCCEEDED(hr) && NULL != pdm)
    {
        MemFree(pdm);
    }
    return hr;
}

/*++
Routine Name:
    GetUserDevmode

Routine Description:

    Gets User Default Devmode from PrinterInfo9.

Arguments:
    psi      : state info.
    ppDevMode: Pointer where the pointer to devmode is stored on exit.

Return Value:
    S_OK if successful.
    E_* otherwise.

--*/
HRESULT
GetUserDevmode(
    __inout     PPTPC_STATE_INFO psi,
    __deref_out LPDEVMODE       *ppDevMode)
{
    HRESULT          hr         = S_OK;
    PRINTER_INFO_9 * pPrnInfo9  = NULL;
    LPDEVMODE        pDevMode   = NULL;

    if ( NULL == psi ||
         NULL == ppDevMode )
    {
        hr = E_INVALIDARG;
    }

    if ( SUCCEEDED(hr) )
    {
        hr = OpenPrinterWrapper(psi);
    }

    if ( SUCCEEDED(hr) )
    {
        hr = GetDevmodeFromDocProperty(psi, ppDevMode);
    }

    return hr;
}

/*++
Routine Name:
    ConvertDevmodeToPrintTicketStream

Routine Description:

    Gets User Default Devmode from PrinterInfo9.

Arguments:
    psi                 : state info.
    pDevMode            : Pointer to the incoming devmode
    ppPrintTicketStream : Points to the stream pointer on exit.

Return Value:
    S_OK if successful.
    E_* otherwise.

--*/
HRESULT
ConvertDevmodeToPrintTicketStream(
    __inout     PPTPC_STATE_INFO    psi,
    __in        LPDEVMODE          pDevMode,
    __deref_out IStream          **ppPrintTicketStream
    )
{
    HRESULT     hr         = S_OK;
    DWORD       cbDevmode  = 0;

    if ( NULL == psi                ||
         NULL == pDevMode           ||
         NULL == ppPrintTicketStream)
    {
        hr = E_INVALIDARG;
    }


    if ( SUCCEEDED (hr) )
    {
        *ppPrintTicketStream = NULL;
        hr = GetProviderWrapper(psi);
    }

    if ( SUCCEEDED(hr) )
    {
        hr = CreateStreamOnHGlobal(NULL, TRUE, ppPrintTicketStream);
    }

    if ( SUCCEEDED(hr))
    {    
        cbDevmode = pDevMode->dmSize + pDevMode->dmDriverExtra;
        hr = PTConvertDevModeToPrintTicket( 
                    psi->hProvider,
                    cbDevmode,
                    pDevMode,
                    kPTJobScope,
                    *ppPrintTicketStream
                    );   
    }
    
    return hr;
}

/*++
Routine Name:
    GetUserPrintTicketStream

Routine Description:

    Gets User Default PrintTicket in form of a stream. The routine first obtains default 
    devmode by calling GetPrinter with PRINTER_INFO_9. It then calls the PT/PC APIs
    to convert the devmode into a PrintTicket. The API returns the PrintTicket in 
    the form of a stream.


Arguments:
    psi                 : state info.
    ppPrintTicketStream : Points to the PrintTicket stream pointer on exit.

Return Value:
    S_OK if successful.
    E_* otherwise.

--*/
HRESULT
GetUserPrintTicketStream(
    __in        PPTPC_STATE_INFO psi,
    __deref_out IStream          **ppPrintTicketStream
   )
{
    HRESULT     hr       = S_OK;
    LPDEVMODE   pDevMode = NULL;
  
    if ( SUCCEEDED(hr) )
    {
        hr = GetUserDevmode(psi, &pDevMode);
    }

    if ( SUCCEEDED(hr) )
    {
        hr = ConvertDevmodeToPrintTicketStream(psi, pDevMode, ppPrintTicketStream);
    }

    // Don't need devmode anymore;
    MemFree(pDevMode);
    pDevMode = NULL;

    return hr;
}


/*++
Routine Name:
    GetPrintCapabilitiesBasedOnPrintTicket

Routine Description:

    Obtains the PrintCapablities of the device based on the incoming PrintTicket


Arguments:
    psi                : state info.
    pPrintTicketStream : Incoming PrintTicket.
    ppPrintCapsStream  : On exit, points to the PrintCapabilities stream pointer.

Return Value:
    S_OK if successful.
    E_* otherwise.

--*/
HRESULT
GetPrintCapabilitiesBasedOnPrintTicket(
    __inout     PPTPC_STATE_INFO   psi,
    __in        IStream           *pPrintTicketStream,
    __deref_out IStream          **ppPrintCapsStream
   )
{
    HRESULT         hr                  = S_OK;
    HPTPROVIDER     hProvider           = NULL;


    if ( NULL == psi                ||
         NULL == pPrintTicketStream ||
         NULL == ppPrintCapsStream 
       )
    {
        hr = E_INVALIDARG;
    }

    if ( SUCCEEDED(hr) )
    {
        *ppPrintCapsStream = NULL;
        hr = GetProviderWrapper(psi);
    }


    if ( SUCCEEDED(hr) )
    {
        hr = CreateStreamOnHGlobal(NULL, TRUE, ppPrintCapsStream);
    }

    if ( SUCCEEDED(hr))
    {    
   
        hr = PTGetPrintCapabilities(
                            psi->hProvider,
                            pPrintTicketStream,
                            *ppPrintCapsStream,
                            NULL
            );

    }

    return hr;
}

/*++
Routine Name:
    ConvertPTPCStreamToDOM

Routine Description:

    Extracts the information in the PrintTicket or PrintCapabilities to create an XML DOM object.

Arguments:
    pPTPCStream : Incoming Stream.
    ppXMLDOM    : On exit, points the XMLDOM document pointer.

Return Value:
    S_OK if successful.
    E_* otherwise.

--*/
HRESULT
ConvertPTPCStreamToDOM(
    __in        IStream           *pPTPCStream,
    __deref_out IXMLDOMDocument2 **ppXMLDOM
    )
{
    HRESULT hr = S_OK;

    if ( NULL == pPTPCStream ||
         NULL == ppXMLDOM )
    {
        hr = E_INVALIDARG;
    }

    // Convert IStream to PrintTicket DOM document
    if ( SUCCEEDED(hr) )
    {
        hr = IStreamToDOMDocument(pPTPCStream, ppXMLDOM);
    }

    return hr;
}

/*++
Routine Name:
    ConvertPTStreamToBuffer

Routine Description:

    Extracts the information in the PrintTicket or PrintCapabilities and puts it
    in a buffer.

Arguments:
    pPTPCStream : Incoming Stream.
    ppbPTBuf    : On exit, points the buffer pointer.
    pcbPTBuf    : On exit, it points to a DWORD which specifies the size of the above buffer 

Return Value:
    S_OK if successful.
    E_* otherwise.

--*/
HRESULT
ConvertPTStreamToBuffer(
    __in                             IStream *pStream,
    __deref_out_bcount(*pcbPTBuf)    PBYTE   *ppbPTBuf,
    __out                            PDWORD   pcbPTBuf
    )
{
    HRESULT     hr       = S_OK;
    UINT        cbBuf    = 0;
    PBYTE       pBuffer  = NULL;
    STATSTG     stats;

    if( NULL == pStream  ||
        NULL == ppbPTBuf ||
        NULL == pcbPTBuf
       )
    {
        hr = E_INVALIDARG;
    }

    if ( SUCCEEDED(hr) )
    {

        hr  = pStream->Stat( &stats, STATFLAG_NONAME );
    }

    if( SUCCEEDED(hr) )
    {
        //
        // If the value is ever larger than this, we have a problem anyway... there
        // should never be a > 4GB print ticket.  If there is, it will simply get truncated
        // to 4 GB, and probably fail a client parse, so aside from not working, this won't
        // actually cause any bad stuff.
        //
        cbBuf = stats.cbSize.LowPart;
    
        pBuffer = (PBYTE)CoTaskMemAlloc(cbBuf);

        if( NULL == pBuffer )
        {
            hr = E_OUTOFMEMORY;
        }
    }

    if( SUCCEEDED(hr) )
    {
        LARGE_INTEGER pos = { 0 };
        hr  = pStream->Seek( pos, STREAM_SEEK_SET, NULL );
    }

    if( SUCCEEDED(hr) )
    {
        ULONG bytesRead = 0;
        PBYTE pos       = pBuffer;
        ULONG bytesLeft = cbBuf;
        do
        {
            hr         = pStream->Read( pos, bytesLeft, &bytesRead );
            pos       += bytesRead;
            bytesLeft -= bytesRead;
        } while( SUCCEEDED(hr) && bytesRead > 0 );
    }

    if( SUCCEEDED(hr) )
    {
        *pcbPTBuf         = cbBuf;
        *ppbPTBuf         = pBuffer;
    }

    if( FAILED(hr) && pBuffer)
    {
        CoTaskMemFree( pBuffer );
        pBuffer = NULL;
    }

    return hr;
}

/*++
Routine Name:
    ConvertFullPrintTicketToMinimalPrintTicket

Routine Description:

    The routine converts a full input print ticket into a minimal print ticket.
    PrintTicket consists of a header (namespaces etc) and a collection of feature-option 
    pairs. The minimal (but still valid, well formed) print ticket will not have any of
    the feature-option pairs.

Arguments:
    pPrintTicketDOM : Print Ticket

Return Value:
    S_OK if successful.
    E_* otherwise.

--*/
HRESULT 
ConvertFullPrintTicketToMinimalPrintTicket(
    __inout  IXMLDOMDocument2  *pPrintTicketDOM
    )
{
    HRESULT         hr                  = S_OK;
    IXMLDOMElement  *pXMLDOMElement     = NULL;
    IXMLDOMNodeList *pXMLDOMNodeList    = NULL;

    hr = pPrintTicketDOM->get_documentElement(&pXMLDOMElement);
    
    if ( SUCCEEDED(hr) )
    {
        hr = pXMLDOMElement->get_childNodes(&pXMLDOMNodeList);
    }

    if ( SUCCEEDED(hr) )
    {
        IXMLDOMNode     *pXMLChild      = NULL;
        while ( SUCCEEDED(hr) &&
                (S_OK == (hr = pXMLDOMNodeList->nextNode(&pXMLChild)) )
               )           
        {
            hr = pXMLDOMElement->removeChild(pXMLChild, NULL);
            pXMLChild->Release();
        }
    }

    if ( pXMLDOMNodeList )
    {
        pXMLDOMNodeList->Release();
        pXMLDOMNodeList = NULL;
    }

    if ( pXMLDOMElement )
    {
        pXMLDOMElement->Release();
        pXMLDOMElement = NULL;
    }

    // pXMLDOMNodeList->nextNode returns S_FALSE when no more nodes can be found
    // That is an expected case, so we replace S_FALSE with S_OK.
    if ( S_FALSE == hr )
    {
        hr = S_OK;
    }
    return hr;
}

/*++
Routine Name:
    RemoveAllAttributesExceptNameFromOptionNode

Routine Description:

If this is the incoming Option Node.

    <psf:Option name="psk:ISOA4" constrained="psk:None">   <--- Removes the "constrained" attribute
                                                                Only retains the name attriubte
        <psf:ScoredProperty name="psk:MediaSizeWidth">
            <psf:Value xsi:type="xsd:integer">210000</psf:Value>
        </psf:ScoredProperty>
        <psf:ScoredProperty name="psk:MediaSizeHeight">
            <psf:Value xsi:type="xsd:integer">297000</psf:Value>
        </psf:ScoredProperty>
        <psf:Property name="psk:DisplayName">
            <psf:Value xsi:type="xsd:string">A4</psf:Value>
        </psf:Property>

    </psf:Option>


Arguments:
    pNode     : Print Caps Option Node

Return Value:
    S_OK if successful.
    E_* otherwise.

--*/
HRESULT
RemoveAllAttributesExceptNameFromOptionNode(
    __inout IXMLDOMNode  *pNode)
{
    HRESULT             hr               = S_OK;
    LONG                cAttributes      = 0;
    IXMLDOMNamedNodeMap *pAttrMap        = NULL;
    
    hr = pNode->get_attributes(&pAttrMap);

    if ( SUCCEEDED(hr) )
    {
        hr = pAttrMap->get_length( &cAttributes );
    }

    for( INT i = 0; SUCCEEDED(hr) && i < cAttributes; i++ )
    {    
        BSTR                bstrAttrName     = NULL;
        IXMLDOMNode        *pCurrentAttrNode = NULL;
        IXMLDOMAttribute   *pCurrentAttr     = NULL;

        hr = pAttrMap->get_item( i, &pCurrentAttrNode );
        if (SUCCEEDED(hr))
        {
            hr = pCurrentAttrNode->QueryInterface(
                            IID_IXMLDOMAttribute,
                            (void**)&pCurrentAttr );
        }

        if (SUCCEEDED(hr))
        {
            hr = pCurrentAttr->get_baseName( &bstrAttrName );
        }
        
        if ( SUCCEEDED(hr) )
        {
            // If an attribute is not named "name", remove it
            if ( wcscmp( bstrAttrName, L"name" ) != 0 )
            {
                hr = pAttrMap->removeNamedItem(bstrAttrName, NULL);
            }
        }

        if ( pCurrentAttrNode )
        {
            pCurrentAttrNode->Release();
        }

        if ( pCurrentAttr )
        {
            pCurrentAttr->Release();
        }

        if ( NULL != bstrAttrName )
        {
            SysFreeString(bstrAttrName);
            bstrAttrName = NULL;
        }

    }
    
    if ( NULL != pAttrMap )
    {
        pAttrMap->Release();
        pAttrMap = NULL;
    }
    return hr;
}

/*++
Routine Name:
    RemoveAllChildrenWithProperty

Routine Description:



    <psf:Option name="psk:ISOA4">
        <psf:ScoredProperty name="psk:MediaSizeWidth">
            <psf:Value xsi:type="xsd:integer">210000</psf:Value>
        </psf:ScoredProperty>
        <psf:ScoredProperty name="psk:MediaSizeHeight">
            <psf:Value xsi:type="xsd:integer">297000</psf:Value>
        </psf:ScoredProperty>

        This part is removed...
           <psf:Property name="psk:DisplayName">
                <psf:Value xsi:type="xsd:string">A4</psf:Value>
            </psf:Property>

    </psf:Option>


Arguments:
    pNode     : Print Caps Option Node

Return Value:
    S_OK if successful.
    E_* otherwise.

--*/

HRESULT
RemoveAllChildrenWithProperty(
    __inout        IXMLDOMNode       *pNode)
{
    HRESULT          hr              = S_OK;
    IXMLDOMNodeList *pXMLDOMNodeList = NULL;

    hr = pNode->get_childNodes(&pXMLDOMNodeList);

    if ( SUCCEEDED(hr) )
    {
        IXMLDOMNode     *pXMLChild      = NULL;
        while ( SUCCEEDED(hr) &&
                (S_OK == (hr = pXMLDOMNodeList->nextNode(&pXMLChild)) )
               )           
        {
            BSTR bstrName = NULL;
            pXMLChild->get_baseName(&bstrName);
            if ( 0 == wcscmp(bstrName, L"Property") )
            {
                hr = pNode->removeChild(pXMLChild, NULL);
            }

            pXMLChild->Release();
            SysFreeString(bstrName);
        }

    }

    if ( pXMLDOMNodeList )
    {
        pXMLDOMNodeList->Release();
        pXMLDOMNodeList = NULL;
    }

    // When there is no remaining node,  pXMLDOMNodeList->nextNode will return
    // S_FALSE. This is expected. Lets replace it with S_OK.
    if ( S_FALSE == hr )
    {
        hr = S_OK;
    }
    return hr;
}

/*++
Routine Name:
    ConvertPrintCapOptionNodeToPrintTicketOptionNode

Routine Desription

The PrintCap Option Node looks like 

    <psf:Option name="psk:ISOA4" constrained="psk:None">
        <psf:ScoredProperty name="psk:MediaSizeWidth">
            <psf:Value xsi:type="xsd:integer">210000</psf:Value>
        </psf:ScoredProperty>
        <psf:ScoredProperty name="psk:MediaSizeHeight">
            <psf:Value xsi:type="xsd:integer">297000</psf:Value>
        </psf:ScoredProperty>
        <psf:Property name="psk:DisplayName">
            <psf:Value xsi:type="xsd:string">A4</psf:Value>
        </psf:Property>
    </psf:Option>

We change it to
    <psf:Option name="psk:ISOA4">
        <psf:ScoredProperty name="psk:MediaSizeWidth">
            <psf:Value xsi:type="xsd:integer">210000</psf:Value>
        </psf:ScoredProperty>
        <psf:ScoredProperty name="psk:MediaSizeHeight">
            <psf:Value xsi:type="xsd:integer">297000</psf:Value>
        </psf:ScoredProperty>
    </psf:Option>

    i.e. Remove the "constrained" attribute
         Remove the psf:Property 

--*/
HRESULT
ConvertPrintCapOptionNodeToPrintTicketOptionNode(
    __inout    IXMLDOMNode       *pOptionNode
    )
{
 
    HRESULT hr = S_OK;

    // Remove the "constrained" attribute.
    hr = RemoveAllAttributesExceptNameFromOptionNode(pOptionNode);
    

    if ( SUCCEEDED(hr) )
    {
        // Iterate through the children and remove all
        // those that are property nodes. This is required
        // to convert the PrintCap node into a PrintTicket node.
        hr = RemoveAllChildrenWithProperty(pOptionNode);
    }

    return hr;
}

/*++
Routine Name:
    CreatePTFeatureOptionNodeFromPrintCapOptionNode

Routine Description:
    For each feature in the PrintCapabilities, there are multiple options (i.e. each PrintCap
    feature node has multiple child option nodes). Each option node has multiple Properties.
    Some of these properties are only valid for PrintCaps and not for PrintTickets. This routine
    will first convert the PrintCap node into a print ticket node by paring off the extra information.
    It will then create a Feature-Option node pair that can be used in a PrintTicket.


Arguments:
    psi                         : Pointer to State Info
    pPrintCapsOptionNode        : Print Caps Option Node that needs to be modified.
    ppPrintTicketFeatureNode    : The node that needs to be added to the PrintTicket

Return Value:
    S_OK if successful.
    E_* otherwise.

--*/
HRESULT
CreatePTFeatureOptionNodeFromPrintCapOptionNode(
    __in        PPTPC_STATE_INFO   psi,
    __in        IXMLDOMNode       *pPrintCapsOptionNode,
    __deref_out IXMLDOMNode      **ppPrintTicketFeatureNode
    )
{
    HRESULT     hr                  = S_OK;
    IXMLDOMNode *pParentNode        = NULL;
    IXMLDOMNode *pOptionNode        = NULL;
    VARIANT_BOOL varDeep            = VARIANT_TRUE;

    if ( NULL == pPrintCapsOptionNode ||
         NULL == ppPrintTicketFeatureNode )
    {
        hr = E_INVALIDARG;
    }

    //
    // To Create a new PrintTicket Node from a corresponding print capabilties node
    // the following steps need to be followed.
    // 1. Clone the option node.
    // 2. From the Option node, remove the "constrained attribute
    // 3. From the option node, remove the Property "DisplayName"
    // 4. Since the pPrintCapabilitiesNode only indicates the option, but doesn't indicate which feature
    //    we have to go to the parent to get the feature Node.
    // 5. Clone the feauture node and then add the cloned option node as a child.  
    
    if ( SUCCEEDED(hr) )
    {
        *ppPrintTicketFeatureNode = NULL;
        hr = pPrintCapsOptionNode->cloneNode(varDeep, &pOptionNode);
    }

    if ( SUCCEEDED(hr) )
    {
        hr = ConvertPrintCapOptionNodeToPrintTicketOptionNode(pOptionNode);
    }

    if ( SUCCEEDED(hr) )
    {
        hr = pPrintCapsOptionNode->get_parentNode(&pParentNode);
    }

    if ( SUCCEEDED(hr) )
    {
        varDeep = VARIANT_FALSE;
        hr = pParentNode->cloneNode(varDeep, ppPrintTicketFeatureNode);
    }

    if ( SUCCEEDED(hr) )
    {
        hr = (*ppPrintTicketFeatureNode)->appendChild(pOptionNode, NULL);
    }

    if (pOptionNode)
    {
        pOptionNode->Release();
        pOptionNode = NULL;
    }

    if (pParentNode)
    {
        pParentNode->Release();
        pParentNode = NULL;
    }

    return hr;
}

/*++
Routine Name:
    MergeNodeIntoMinimalPrintTicket

Routine Description:
    Merges the incoming node onto a minimal print ticket. The node is added as a child node.

Arguments:
    pPrintTicketMinimal : Print Ticket
    pNode               : The node that needs to be added to the PrintTicket

Return Value:
    S_OK if merge was successful.
    E_* otherwise.

--*/
HRESULT
MergeNodeIntoMinimalPrintTicket(
    __in  IXMLDOMDocument2  *pPrintTicketMinimal, 
    __in  IXMLDOMNode       *pNode)
{
    HRESULT hr = S_OK;
    IXMLDOMElement *pXMLDOMElement = NULL;

    hr = pPrintTicketMinimal->get_documentElement(&pXMLDOMElement);
    if ( SUCCEEDED(hr) )
    {
        hr = pXMLDOMElement->appendChild(pNode, NULL);
    }

    pXMLDOMElement->Release();

    return hr;
}
