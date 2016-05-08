// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.


#include "stdafx.h"

/*++
Routine Name:
    CreatePrintJobWithImagePassThrough

Routine Description:

    This routine creates a print job for the "Microsoft XPS Document Writer" printer.
    A .jpg image is sent using an image escape. A fixed page XAML is sent that references the image.
    
Arguments:

    <None>

Return Value:

    S_OK if successful, 
    E_* if there is an error 

--*/

HRESULT
CreatePrintJobWithImagePassThrough(
    VOID
    )
{
    HDC               hdcMXDW        = NULL;
    HRESULT           hr             = S_OK;
    PTPC_STATE_INFO   si             = {0};
    IStream          *pPTStream      = NULL;
    PBYTE             pbPTBuf        = NULL;
    DWORD             cbPTBuf        = 0;
    PBYTE             pEscData       = NULL;
    DWORD             cbEscData      = 0;
    BOOL              bStartPageSent = FALSE;
    BOOL              bStartDocSent  = FALSE;
    PBYTE             pImgEscData    = NULL;
    DWORD             cbImgEscData = 0;
    CHAR              szURI[MAX_PATH] = "/Documents/1/Resources/Images/image1.jpg";
    LPSTR             pFixedPage      = "<FixedPage Width=\"816\" Height=\"1056\" "
                                        "xmlns=\"http://schemas.microsoft.com/xps/2005/06\" xml:lang=\"und\">"
                                        "<Path Data=\"M 100.00,100.00 L 440.48,100.00 440.48,613.44 100.00,613.44 z\">"
                                        "<Path.Fill><ImageBrush ImageSource=\"/Documents/1/Resources/Images/image1.jpg\" "
                                        "Viewbox=\"0,0,256,384\" TileMode=\"None\" ViewboxUnits=\"Absolute\" ViewportUnits=\"Absolute\" "
                                        "Viewport=\"100.00,100.00,340.48,513.44\" /></Path.Fill></Path></FixedPage>";

    
    hr = StringCchCopy(si.szPrinterName, CCHOF(si.szPrinterName), gszPrinterName);

    if ( SUCCEEDED(hr) )
    {
        GetUserPrintTicketStream(&si, &pPTStream);
    }

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

    //
    // Make sure driver is a XPSDrv driver. i.e. it can accept XPS content.

    // 
    if (SUCCEEDED (hr) )
    {
        hr = IsXPSCapableDriver(hdcMXDW);
    }

       
    //
    // Send the Print Ticket
    // If return value is S_OK, it means driver is  XPS capable
    // If return value is S_FALSE, it means driver is not XPS capable
    // If return value is E_FAIL, it means something bad happened
    // while we were querying the driver.
    //
    if (S_OK != hr)
    {
        vFormatAndPrint(IDS_APP_XPS_CAPABLE_PRINTER_NOTINSTALLED);
        hr = HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
    }
    else
    {
    
        DOCINFO	DocInfo = { 
                            sizeof(DOCINFO), 
                            L"Fixed Page PrintTicket Sample", // Title of the print job
                            NULL,                             // Not specifying output file. Determined by Printer
                            NULL,                             // Not specifying data type. 
                            0
                          };



        if( StartDoc(hdcMXDW, &DocInfo) > 0)
        { 
            bStartDocSent = TRUE;
        }
        else
        {
            // Sometimes StartDoc fails if the specified output file cannot be opened.
            // Sometimes it happens when a viewer is viewing the file, while the driver
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

    // Send Image as an escape 
    // Note: Here the image has been including within the .exe as a resource. This makes it
    // easier to distribute the .exe as a single package without the need to bundle an extra .jpg file.
    // But in real world, you'll likely need to load the .jpg file separately.
    if ( SUCCEEDED(hr) )
    {
        DWORD  cbImage = 0;
        LPVOID pImage  = NULL;
        HRSRC hrsrc = FindResource(NULL, L"#201", L"JPGIMAGE");

        if ( NULL != hrsrc )
        {
            cbImage = SizeofResource(NULL, hrsrc);
            HGLOBAL hResData = LoadResource(NULL, hrsrc);

            if ( NULL != hResData)
            {
                pImage = LockResource(hResData);
            }
        }
        else
        {
            hr = HRESULT_FROM_WIN32(GetLastError() );   
        }

        if ( SUCCEEDED(hr) )
        {
            hr = PutTogetherEscapeStructureForImage(
                                            MXDCOP_SET_S0PAGE_RESOURCE,
                                            (PBYTE)pImage,
                                            cbImage,
                                            szURI,
                                            &pImgEscData,
                                            &cbImgEscData);
        }
    }

    //
    // Send the image using escape created above
    //
    if ( SUCCEEDED (hr) )
    {
        if( ExtEscape(hdcMXDW, MXDC_ESCAPE, cbImgEscData, (LPCSTR) pImgEscData, 0, NULL) <= 0 )
        { 
            vFormatAndPrint(IDS_APP_EXTESCAPE_FAILED);
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }

    //
    // Send XAML to reference the image just sent
    //

    if ( SUCCEEDED(hr) )
    {
        size_t cbFixedPage       = 0;
        PBYTE pFixedPageEscData  = NULL;
        DWORD cbFixedPageEscData = 0;
        
        cbFixedPage = strlen(pFixedPage);

        if ( SUCCEEDED(hr) )
        {
            hr = PutTogetherEscapeStructureForFixedPage(
                                            MXDCOP_SET_S0PAGE,
                                            (PBYTE)pFixedPage,
                                            (DWORD)cbFixedPage,
                                            &pFixedPageEscData,
                                            &cbFixedPageEscData);  
        }

        if ( SUCCEEDED (hr) )
        {
            if( ExtEscape(hdcMXDW, MXDC_ESCAPE, cbFixedPageEscData, (LPCSTR) pFixedPageEscData, 0, NULL) <= 0 )
            { 
                vFormatAndPrint(IDS_APP_EXTESCAPE_FAILED);
                hr = HRESULT_FROM_WIN32(GetLastError());
            }
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
	
    // Clean up
    if(pEscData != NULL)
    { 
        MemFree (pEscData); 
        pEscData = NULL;
    }

    if ( NULL != pbPTBuf )
    {
        CoTaskMemFree(pbPTBuf);
        pbPTBuf = NULL;
    }

    if ( NULL != pPTStream )
    {
        pPTStream->Release();
        pPTStream = NULL;
    }
		
    if(hdcMXDW != NULL)
    { 
        DeleteDC(hdcMXDW);
        hdcMXDW = NULL;
    }

    RelaseStateInfoContents(&si);
    return hr;
}

