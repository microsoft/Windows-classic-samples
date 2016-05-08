// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.


#include "stdafx.h"


HRESULT
XPathQueryOnPrintCaps(
    __in PPTPC_STATE_INFO   psi,
    __in IXMLDOMDocument2  *pPrintCapsDOM,
    __in BSTR               bstrXPathQuery,
    __out IXMLDOMNodeList **ppIXMLDOMNodeList

    )
{
    HRESULT           hr                       = S_OK;
    BSTR              bstrSelectionLanguage    = SysAllocString(L"SelectionLanguage");
    BSTR              bstrXPath                = SysAllocString(L"XPath");
    BSTR              bstrSelectionNamespaces  = SysAllocString(L"SelectionNamespaces");

    BSTR bstrNS = SysAllocString(L" \
                        xmlns:psf=\"http://schemas.microsoft.com/windows/2003/08/printing/printschemaframework\" \
                        xmlns:psk=\"http://schemas.microsoft.com/windows/2003/08/printing/printschemakeywords\" \
                        ");

    if ( NULL == pPrintCapsDOM  ||
         NULL == bstrXPathQuery ||
         NULL == ppIXMLDOMNodeList)
    {
        hr = E_INVALIDARG;
    }

    *ppIXMLDOMNodeList = NULL;

    if ( NULL == bstrSelectionLanguage  ||
         NULL == bstrXPath              ||
         NULL == bstrSelectionNamespaces||
         NULL == bstrNS
        )
    {
        hr = E_OUTOFMEMORY;
    }

    if ( SUCCEEDED(hr) )
    {
       VARIANT var;
       VariantInit (&var);
       var.vt      = VT_BSTR;
       var.bstrVal = bstrXPath;

       hr = pPrintCapsDOM->setProperty(bstrSelectionLanguage, var);
    }

    if ( SUCCEEDED(hr) )
    {
        VARIANT var;
        VariantInit (&var);
        var.vt      = VT_BSTR;
        var.bstrVal = bstrNS;

       hr = pPrintCapsDOM->setProperty(bstrSelectionNamespaces, var);
    }

    //
    // Select all the options for PageMediaSize
    //
    if ( SUCCEEDED(hr) )
    {
        hr = pPrintCapsDOM->selectNodes(bstrXPathQuery, ppIXMLDOMNodeList);
    }

    if ( NULL != bstrSelectionLanguage )
    {
        SysFreeString(bstrSelectionLanguage);
    }

    if ( NULL != bstrXPath )
    {
        SysFreeString(bstrXPath);
    }

    if ( NULL != bstrSelectionNamespaces)
    {
        SysFreeString(bstrSelectionNamespaces);
    }

    if ( NULL != bstrNS)
    {
        SysFreeString(bstrNS);
    }
    return hr;
}

#define XPATHQUERYFORPRINTCAPS(FeatureName, OptionName) \
                                            L"psf:PrintCapabilities/psf:Feature[substring-after(@name,':')='" FeatureName L"']" \
                                            L"[name(namespace::*[.='http://schemas.microsoft.com/windows/2003/08/printing/printschemakeywords'])=substring-before(@name,':')]" \
                                            L"/psf:Option" \
                                            L"[substring-after(@name,':')='"  OptionName  L"']" \
                                            L"[name(namespace::*[.='http://schemas.microsoft.com/windows/2003/08/printing/printschemakeywords'])=substring-before(@name,':')]"

HRESULT
GetNodeWithA4PaperSize(
    __in PPTPC_STATE_INFO   psi,
    __in IXMLDOMDocument2  *pPrintCapsDOM,
    __out IXMLDOMNode     **ppPaperSizeNode
    )
{
    HRESULT             hr                  = S_OK;
    IXMLDOMNodeList     *pPaperSizeNodeList = NULL;

    BSTR    bstrXPathQuery  = SysAllocString(XPATHQUERYFORPRINTCAPS(L"PageMediaSize", L"ISOA4") );

    if ( NULL == ppPaperSizeNode )
    {
        *ppPaperSizeNode = NULL;
    }
    
    if ( NULL == bstrXPathQuery 
       )
    {
        hr = E_OUTOFMEMORY;
    }

    if ( SUCCEEDED(hr) )
    {
        hr = XPathQueryOnPrintCaps(psi, pPrintCapsDOM, bstrXPathQuery, &pPaperSizeNodeList);
    } 

    if ( SUCCEEDED(hr) )
    {
        hr = pPaperSizeNodeList->nextNode(ppPaperSizeNode);
    }

    if ( NULL != bstrXPathQuery)
    {
        SysFreeString(bstrXPathQuery);
    } 

    if ( NULL != pPaperSizeNodeList )
    {
        pPaperSizeNodeList->Release();
    }


    return hr;
}

HRESULT
CreateNewPrintTicketWithA4PaperSize(
    __in PPTPC_STATE_INFO     psi,
    __in IXMLDOMDocument2    *pPrintCapsDOM,
    __in IStream             *pBasePrintTicketStream, 
    __in IXMLDOMDocument2    *pBasePrintTicketDOM, 
    __out IStream           **ppPrintTicketA4Stream)
{
    IStream          *pDeltaPrintTicketStream   = NULL;
    IXMLDOMDocument2 *pDeltaPrintTicket         = NULL;
    IXMLDOMNode      *pPrintCapsA4Node          = NULL;
    IXMLDOMNode      *pPrintTicketA4Node        = NULL;

    HRESULT hr = S_OK;

    if ( NULL != ppPrintTicketA4Stream )
    {
        *ppPrintTicketA4Stream = NULL;
    }
    else
    {
        hr = E_INVALIDARG;
    }

    if ( SUCCEEDED(hr) )
    {
        hr = GetNodeWithA4PaperSize(psi, pPrintCapsDOM, &pPrintCapsA4Node);
    }

    if ( SUCCEEDED(hr) )
    {
        hr = CreatePTFeatureOptionNodeFromPrintCapOptionNode(psi, pPrintCapsA4Node, &pPrintTicketA4Node);
             
    }

    // Create a duplicate print ticket from the same print ticket stream as the base print ticket.
    if ( SUCCEEDED(hr) )
    {
        hr = ConvertPTPCStreamToDOM(pBasePrintTicketStream, &pDeltaPrintTicket);
    }

    // From the Cloned Print Ticket, take away all off feature-option pairs
    // so that only a minimal (or you may call it sparse) print ticket remains.
    if ( SUCCEEDED(hr) )
    {
        hr = ConvertFullPrintTicketToMinimalPrintTicket(pDeltaPrintTicket);
    }

    if ( SUCCEEDED(hr) )
    {
        hr = MergeNodeIntoMinimalPrintTicket(pDeltaPrintTicket, pPrintTicketA4Node);
    }

    // The API PTMergeAndValidatePrintTicket() takes in a base PrintTicket, a Delta or a New PrintTicket
    // and returns the merged print ticket. We already have a stream for Base PT. We create a 
    // stream for Delta PT and also prepare an empty stream where the merged PT can be written.
    if ( SUCCEEDED(hr) )
    {
        hr = CreateStreamOnHGlobal(NULL, TRUE, &pDeltaPrintTicketStream);
    }

    if ( SUCCEEDED(hr) )
    {
        hr = DOMDocumentToIStream(pDeltaPrintTicket, pDeltaPrintTicketStream);
    }

    if ( SUCCEEDED(hr) )
    {
        hr = CreateStreamOnHGlobal(NULL, TRUE, ppPrintTicketA4Stream);
    }

    //
    // Make sure printticket stream points to the begining
    //
    if ( SUCCEEDED(hr) )
    {
        LARGE_INTEGER li = {0,0};
        hr = pBasePrintTicketStream->Seek(li, STREAM_SEEK_SET, NULL);
        if ( SUCCEEDED(hr) )
        {
            hr = pDeltaPrintTicketStream->Seek(li, STREAM_SEEK_SET, NULL);
        }
    }

    if ( SUCCEEDED(hr) )
    {
        hr = PTMergeAndValidatePrintTicket(psi->hProvider, pBasePrintTicketStream, pDeltaPrintTicketStream, kPTPageScope, *ppPrintTicketA4Stream, NULL);
    }

    // If operation succeeded, the returned hr can be either 
    //      S_PT_NO_CONFLICT                 (0x00040001) OR
    //      S_PT_CONFLICT_RESOLVED           (0x00040002)
    // as defined in prntvpt.h.
    // These codes are ALSO defined as "Success but static" or "Macintosh clipboard format" in winerror.h where 
    // they have a different meaning. So do not get confused.

    if ( pPrintCapsA4Node )
    {
        pPrintCapsA4Node->Release();
        pPrintCapsA4Node = NULL;
    }

    if ( pPrintTicketA4Node )
    {
        pPrintTicketA4Node->Release();
        pPrintTicketA4Node = NULL;
    }

    if ( pDeltaPrintTicket )
    {
        pDeltaPrintTicket->Release();
        pDeltaPrintTicket = NULL;
    }

    if ( pDeltaPrintTicketStream )
    {
        pDeltaPrintTicketStream->Release();
        pDeltaPrintTicketStream = NULL;
    }
    return hr;
}

HRESULT
CreateJobWith2PrintTickets(
    __in                    PPTPC_STATE_INFO    psi,
    __in_bcount(cbPTBuf1)   PBYTE               pbPTBuf1,
    __in                    DWORD               cbPTBuf1,
    __in_bcount(cbPTBuf2)   PBYTE               pbPTBuf2,
    __in                    DWORD               cbPTBuf2
    )
{
    HRESULT          hr         = S_OK;
    HDC              hdcMXDW    = NULL;
    PBYTE            pEscData   = NULL;
    DWORD            cbEscData  = 0;

    BOOL bStartDocSent = FALSE;
    BOOL bStartPageSent = FALSE;

    // Make sure the printer is installed and we can create its DC.
    if (SUCCEEDED (hr) )
    {
        hdcMXDW = CreateDC( NULL, psi->szPrinterName, NULL, NULL);
        if(NULL == hdcMXDW)
        {
            DWORD dwLastError = GetLastError();
            vFormatAndPrint(IDS_APP_CREATEDC_FAILED);
            if ( ERROR_INVALID_PRINTER_NAME == dwLastError )
            {
                vFormatAndPrint(IDS_APP_MXDWPRINTER_NOTINSTALLED);
            }
            hr = HRESULT_FROM_WIN32(dwLastError);
        }
    }

    // Make sure driver is a XPSDrv driver. i.e. it can accept XPS content.
    if (SUCCEEDED (hr) )
    {
        hr = IsXPSCapableDriver(hdcMXDW);
     
        // Only a return value of S_OK from IsXPSCapableDriver() means driver is  XPS capable
        // while we were querying the driver.
        // A return value of S_FALSE means driver is not XPS capable
        if ( S_FALSE == hr )
        {
            vFormatAndPrint(IDS_APP_XPS_CAPABLE_PRINTER_NOTINSTALLED);
            hr = HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
        }
    }

    if ( SUCCEEDED(hr) )
    {
        DOCINFO	DocInfo = { 
                            sizeof(DOCINFO), 
                            L"Multiple PrintTickets Sample",  // Title of the print job
                            NULL,                             // Not specifying output file. Determined by Printer
                            NULL,                             // Not specifying data type. 
                            0
                          };


        //Send the Escape
        if( StartDoc(hdcMXDW, &DocInfo) > 0)
        { 
            bStartDocSent = TRUE;
        }
        else
        {
            // Sometimes StartDoc fails if the specified output file cannot be opened.
            // Sometimes that happens when a viewer is viewing the file, while the driver
            // is trying to write to it.
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }

    if ( SUCCEEDED (hr) )
    {
        if( StartPage(hdcMXDW) > 0)
        { 
            bStartPageSent = TRUE;
        }
        else
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }

    // Create Escape for first Print Ticket
    if ( SUCCEEDED(hr) )
    {
        hr = PutTogetherEscapeStructureForPrintTicket(
                                        MXDCOP_PRINTTICKET_FIXED_DOC,
                                        pbPTBuf1,
                                        cbPTBuf1,
                                        &pEscData,
                                        &cbEscData);
    }

    // Send Print Ticket for the first page
    if ( SUCCEEDED (hr) )
    {
        if( ExtEscape(hdcMXDW, MXDC_ESCAPE, cbEscData, (LPCSTR) pEscData, 0, NULL) <= 0 )
        { 
            vFormatAndPrint(IDS_APP_EXTESCAPE_FAILED);
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }

    if ( SUCCEEDED (hr) )
    {   
        WCHAR szText[] = L"First Page with User Default PrintTicket";
        if ( FALSE == TextOut(hdcMXDW, 200, 200, szText, (int) wcslen(szText) ) )
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }

    if ( bStartPageSent )
    {   
        if( EndPage(hdcMXDW) > 0)
        { 
            bStartPageSent = FALSE;
        }
        else
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }

    // Free the pEscData allocated in PutTogetherEscapeStructureForPrintTicket 
    if(pEscData != NULL)
    { 
        MemFree (pEscData); 
        pEscData = NULL;
    }

    // Start new page and use a different print ticket.
    if ( SUCCEEDED(hr) )
    {
        if( StartPage(hdcMXDW) > 0)
        { 
            bStartPageSent = TRUE;
        }
        else
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }


    // Create Escape for second Print Ticket
    if ( SUCCEEDED(hr) )
    {
        hr = PutTogetherEscapeStructureForPrintTicket(
                                        MXDCOP_PRINTTICKET_FIXED_PAGE,
                                        pbPTBuf2,
                                        cbPTBuf2, 
                                        &pEscData,
                                        &cbEscData);
    }

    // Send Print Ticket for the above page
    if ( SUCCEEDED (hr) )
    {
        if( ExtEscape(hdcMXDW, MXDC_ESCAPE, cbEscData, (LPCSTR) pEscData, 0, NULL) <= 0 )
        { 
            vFormatAndPrint(IDS_APP_EXTESCAPE_FAILED);
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }

    if ( SUCCEEDED (hr) )
    {   
        WCHAR szText[] = L"Second Page with Modified Print Ticket";
        if ( FALSE == TextOut(hdcMXDW, 200, 200, szText, (int)wcslen(szText)) )
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }

    
    if( bStartPageSent )
    {
        EndPage(hdcMXDW);
    }

    if( bStartDocSent )
    {
        EndDoc(hdcMXDW);
    }
     
    if(hdcMXDW != NULL)
    {
        DeleteDC(hdcMXDW);
        hdcMXDW = NULL;
    }
    
    if(pEscData != NULL)
    { 
        MemFree (pEscData); 
        pEscData = NULL;
    }
    return hr;
}


/*++
Routine Name:
    CreateJobWith2PrintTickets

Routine Description:
--*/

HRESULT
CreateJobWith2PrintTickets(
    __in PPTPC_STATE_INFO   psi,
    __in IStream            *pPTStream1,
    __in IStream            *pPTStream2)
{
    HRESULT     hr          = S_OK;
    PBYTE       pbPTBuf1      = NULL;
    DWORD       cbPTBuf1      = 0;

    PBYTE       pbPTBuf2       = NULL;
    DWORD       cbPTBuf2      = 0;
    
    if ( SUCCEEDED(hr) )
    {
        hr = ConvertPTStreamToBuffer(pPTStream1,
                                     &pbPTBuf1,
                                     &cbPTBuf1
                                    );
    }

    if ( SUCCEEDED(hr) )
    {
        hr = ConvertPTStreamToBuffer(pPTStream2,
                                     &pbPTBuf2,
                                     &cbPTBuf2
                                    );
    }


    if (SUCCEEDED(hr) )
    {
        hr = CreateJobWith2PrintTickets(psi, pbPTBuf1, cbPTBuf1, pbPTBuf2, cbPTBuf2);
    }

    CoTaskMemFree(pbPTBuf1);
    CoTaskMemFree(pbPTBuf2);

    return hr;
}



/*++
Routine Name:
    CreateMultiplePrintTicketJob

Routine Description:

    This routine creates a print job for the "Microsoft XPS Document Writer" printer.
    The job has multiple print tickets. A different print ticket for each page.
    One print ticket is the full user default print ticket. Second print ticket is 
    a full print ticket with a media size A4. 
    First page has a simple line of text and an embedded image.
    Second page has a simple line of text.
    
Arguments:

    <None>

Return Value:

    S_OK if successful, 
    E_* if there is an error 

--*/

HRESULT
CreatePrintJobMultiplePrintTicket(
    VOID
    )
{
    HRESULT          hr                       = S_OK;
    PTPC_STATE_INFO  si                       = {0};

    IStream          *pPrintCapsStream        = NULL;
    IXMLDOMDocument2 *pPrintCapsDOM           = NULL;

    IStream          *pBasePrintTicketStream  = NULL;
    IXMLDOMDocument2 *pBasePrintTicketDOM     = NULL;

    IStream          *pPrintTicketA4Stream    = NULL;

    hr = StringCchCopy(si.szPrinterName, CCHOF(si.szPrinterName), gszPrinterName);

    if ( SUCCEEDED(hr) )
    {
        hr = GetUserPrintTicketStream(&si, &pBasePrintTicketStream);
    }

    if ( SUCCEEDED(hr) )
    {
         hr = ConvertPTPCStreamToDOM(pBasePrintTicketStream, &pBasePrintTicketDOM);
    }

    // Use the PrintTicket to get the print capabilities. If you want to get
    // the print ticket based on the default print ticket, you don't need to
    // pass the default print ticket stream to the API. But we are doing it
    // here just to show how it can be done.
    // ConvertPTPCStreamToDOM may have moved the stream pointer. 
    // So make sure printticket stream points to the begining
    if ( SUCCEEDED(hr) )
    {
        LARGE_INTEGER li = {0,0};
        hr = pBasePrintTicketStream->Seek(li, STREAM_SEEK_SET, NULL);
    }
    if ( SUCCEEDED(hr) )
    {
        hr = GetPrintCapabilitiesBasedOnPrintTicket(&si, pBasePrintTicketStream, &pPrintCapsStream);
    }

    if ( SUCCEEDED(hr) )
    {
        hr = ConvertPTPCStreamToDOM(pPrintCapsStream, &pPrintCapsDOM);
    }

    // Now we have the default PT and the Print Capabilities. Use them to create a new PrintTicket with A4 paper size
    // instead of the default "Letter" ("Letter" is default on U.S. systems. It may or may not be default on your system).
    if ( SUCCEEDED(hr) )
    {
        hr = CreateNewPrintTicketWithA4PaperSize(&si, pPrintCapsDOM, pBasePrintTicketStream, pBasePrintTicketDOM, &pPrintTicketA4Stream);
    }

    if ( SUCCEEDED(hr) )
    {
        hr = CreateJobWith2PrintTickets(&si, pBasePrintTicketStream, pPrintTicketA4Stream);
    }


    // Cleanup
    
    if ( NULL != pPrintCapsDOM )
    {
        pPrintCapsDOM->Release();
        pPrintCapsDOM = NULL;
    }

    if ( NULL != pPrintCapsStream )
    {
        pPrintCapsStream->Release();
        pPrintCapsStream = NULL;
    }

    if ( NULL != pBasePrintTicketDOM )
    {
        pBasePrintTicketDOM->Release();
        pBasePrintTicketDOM = NULL;
    }

    if ( NULL != pBasePrintTicketStream )
    {
        pBasePrintTicketStream->Release();
        pBasePrintTicketStream = NULL;
    }

    if ( NULL != pPrintTicketA4Stream )
    {
        pPrintTicketA4Stream->Release();
        pPrintTicketA4Stream = NULL;
    }

    RelaseStateInfoContents(&si);
    return hr;
}

