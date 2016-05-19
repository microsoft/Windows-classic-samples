// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.


#include "stdafx.h"


/*++
Routine Name:
    CreatePrintTicketJobSimple

Routine Description:

    A simple job for the printer "Microsoft XPS Document Writer" is created.
    A print ticket is sent as part of this job using ExtEscape. The driver should understand
    this escape, so we test whether driver is XPS capable before sending the Print Ticket. 
    A simple line of text is also sent.
    
Arguments:

    <None>

Return Value:

    S_OK if successful, 
    E_* if there is an error 

--*/

HRESULT 
CreatePrintTicketJobSimple(
        VOID
    )
{
    HDC               hdcMXDW        = NULL;
    HRESULT           hr             = S_OK;
    PBYTE             pEscData       = NULL;
    DWORD             cbEscData      = 0;
    IStream          *pPTStream      = NULL;
    PBYTE             pbPTBuf        = NULL;
    DWORD             cbPTBuf        = 0;
    PTPC_STATE_INFO   si             = {0};

    BOOL              bStartPageSent = FALSE;
    BOOL              bStartDocSent  = FALSE;


    // Copy printer name on the state info structure.
    hr = StringCchCopy(si.szPrinterName, CCHOF(si.szPrinterName), gszPrinterName);

    // Prepare Print Ticket Informaion that is to be sent.
    // Get the Job Level User Print Ticket
    if ( SUCCEEDED(hr) )
    {
        hr = GetUserPrintTicketStream(&si, &pPTStream);
    }

    // The MXDW driver escape expects print ticket as a collection of bytes.
    // The .xps package specification expects the PrintTicket to be encoded in the proper
    // utf format. The best way to generate such a collection of utf encoded characters
    // is to convert the PT stream directly into a buffer. The alternative of converting
    // the stream to DOM and then converting DOM to BSTR(by calling DOMDocument->get_xml()
    // is not only expensive, but also it may not generate the proper format for .xps package.

    if ( SUCCEEDED(hr) )
    {
        hr = ConvertPTStreamToBuffer(pPTStream,
                                     &pbPTBuf,
                                     &cbPTBuf
                                    );
    }

    // Create Print Ticket Escape for sending to Printer
    if ( SUCCEEDED(hr) )
    {
        hr = PutTogetherEscapeStructureForPrintTicket(
                                        MXDCOP_PRINTTICKET_FIXED_DOC,
                                        pbPTBuf,
                                        cbPTBuf, 
                                        &pEscData,
                                        &cbEscData);
    }

    //
    // Make sure the printer is installed and we can create its DC.
    //
    if (SUCCEEDED (hr) )
    {
        hdcMXDW = CreateDC( NULL, si.szPrinterName, NULL, NULL);
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
        DOCINFO DocInfo = { 
                            sizeof(DOCINFO), 
                            L"Document with Single PrintTicket", // Title of the print job
                            NULL,                                // Not specifying output file. Driver will throw UI.
                            NULL,                                // Not specifying data type. 
                            0
                          };

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

    // Send Print Ticket for the above page
    if ( SUCCEEDED (hr) )
    {
        if( ExtEscape(hdcMXDW, MXDC_ESCAPE, cbEscData, (LPCSTR) pEscData, 0, NULL) <= 0 )
        { 
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }

    // Send some text 
    if ( SUCCEEDED (hr) )
    {
        WCHAR szText[] = L"This page is associated with default Print Ticket";
        if ( FALSE == TextOut(hdcMXDW, 200, 200, szText, (int)wcslen(szText)) )
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }

    if (bStartPageSent)
    {
        EndPage(hdcMXDW);
    }

    if ( bStartDocSent)
    {
        EndDoc(hdcMXDW);
    }

    if(hdcMXDW != NULL)
    {
        DeleteDC(hdcMXDW);
        hdcMXDW = NULL;
    }

    // Clean up

    CoTaskMemFree(pbPTBuf);
    pbPTBuf = NULL;

    if(pEscData != NULL)
    {
        MemFree (pEscData); 
        pEscData = NULL;
    }

    if ( NULL != pPTStream )
    {
        pPTStream->Release();
        pPTStream = NULL;
    }


    RelaseStateInfoContents(&si);
    return hr;
}
