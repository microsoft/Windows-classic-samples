// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "stdafx.h"

HRESULT
IsXPSCapableDriver(
    HDC hdc
    )
{
    CHAR    lpszOutData[ARRAYSIZE(gszTechnologyIdentifier)] = {0};
    INT     cbOutput                                        = (INT) ( strlen(gszTechnologyIdentifier) + 1 );
    HRESULT hr                                              = E_FAIL;

    if ( 0 > ExtEscape(
                hdc,            // handle to DC
                GETTECHNOLOGY,  // escape function
                0,              // size of input structure
                NULL,           // input structure
                cbOutput,       // size of output structure
                lpszOutData     // output structure
                )
        )
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }
    else
    {
        //
        // ExtEscape succeeded. Compare the technology returned with
        // the technology we want it to return.
        //
        if (0 == strcmp(lpszOutData, gszTechnologyIdentifier) )
        {
            hr = S_OK;
        }
        else
        {
            hr = S_FALSE;
        }
    }

    return hr;
}


HRESULT
PutTogetherEscapeStructureForImage(
    __in                              ULONG                       OpCode,
    __in_bcount(cbInBuffer)           PBYTE                       pbInBuffer,
    __in                              DWORD                       cbInBuffer,
    __in                              LPCSTR                      pszURI,
    __deref_out_bcount(*pcbOutBuffer) PBYTE                      *ppbOutBuffer,
    __out                             PDWORD                      pcbOutBuffer
    )
{
    P_MXDC_ESCAPE_HEADER_T          pEscHeader      = NULL;
    P_MXDC_XPS_S0PAGE_RESOURCE_T    pEscData        = NULL;
    P_MXDC_S0PAGE_RESOURCE_ESCAPE_T pEscBuffer      = NULL;
    DWORD                           cbEscBuffer     = 0;
    HRESULT                         hr              = S_OK;

    if ( NULL == pbInBuffer   ||
         0    == cbInBuffer   ||
         NULL == ppbOutBuffer ||
         NULL == pcbOutBuffer )
    {
        hr = E_INVALIDARG;
    }

    if ( SUCCEEDED(hr) )
    {
        *ppbOutBuffer = NULL;
        *pcbOutBuffer = 0;

        //
        // bData[1] is the beginig of the data buffer. The actual
        // size of the buffer is cbBuffer (which includes bData[1]
        // So we subtract 1 
        //
        cbEscBuffer =  sizeof(MXDC_S0PAGE_RESOURCE_ESCAPE_T) - sizeof(pEscData->bData); 
        cbEscBuffer += cbInBuffer;
    
        // Allocate contiguous data
        pEscBuffer = (P_MXDC_S0PAGE_RESOURCE_ESCAPE_T) MemAlloc(cbEscBuffer);

        if ( NULL == pEscBuffer)
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
            // Zero out allocated memory
            memset((PBYTE)pEscBuffer, 0, cbEscBuffer);

            
            pEscHeader      = &(pEscBuffer->mxdcEscape);
            pEscData        = &(pEscBuffer->xpsS0PageResourcePassthrough);

            // Fill out the Escape Header data
            pEscHeader->cbInput    = cbEscBuffer;
            pEscHeader->cbOutput   = 0;
            pEscHeader->opCode     = OpCode; 

            // Fill out the Escape data
            pEscData->dwSize         = sizeof(MXDC_XPS_S0PAGE_RESOURCE_T) + cbInBuffer - sizeof(pEscData->bData);
            pEscData->dwResourceType = MXDC_RESOURCE_JPEG;
            hr = StringCchCopyA((LPSTR)pEscData->szUri, CCHOF(pEscData->szUri), pszURI);
            if ( SUCCEEDED(hr) )
            {
                pEscData->dwDataSize = cbInBuffer; 
                memcpy(&pEscData->bData, pbInBuffer, cbInBuffer);
            }
        }
    }

    if ( SUCCEEDED(hr ) )
    {
        *ppbOutBuffer  = (PBYTE)pEscBuffer;
        *pcbOutBuffer  = cbEscBuffer;
    }

    return hr;
}

HRESULT
PutTogetherEscapeStructureForPrintTicket(
    __in                               ULONG           OpCode,
    __in_bcount(cbInBuffer)            PBYTE           pbInBuffer,
    __in                               DWORD           cbInBuffer,
    __deref_out_bcount(*pcbOutBuffer)  PBYTE          *ppbOutBuffer,
    __out                              PDWORD          pcbOutBuffer
    )
{

    P_MXDC_ESCAPE_HEADER_T      pEscHeader  = NULL;
    P_MXDC_PRINTTICKET_DATA_T   pEscData    = NULL;
    P_MXDC_PRINTTICKET_ESCAPE_T pEscBuffer  = NULL;
    DWORD                       cbEscBuffer = 0;
    HRESULT                     hr          = S_OK;

    if ( NULL == pbInBuffer   ||
         0    == cbInBuffer   ||
         NULL == ppbOutBuffer ||
         NULL == pcbOutBuffer )
    {
        hr = E_INVALIDARG;
    }

    if ( SUCCEEDED(hr) )
    {
        *ppbOutBuffer = NULL;
        *pcbOutBuffer = 0;

        //
        // bData[1] is the beginig of the data buffer. The actual
        // size of the buffer is cbBuffer (which includes bData[1]
        // So we subtract 1 
        //
        cbEscBuffer = sizeof(MXDC_PRINTTICKET_ESCAPE_T) - sizeof(pEscData->bData); 
        cbEscBuffer += cbInBuffer;
    
        // Allocate contiguous data
        pEscBuffer = (P_MXDC_PRINTTICKET_ESCAPE_T) MemAlloc(cbEscBuffer);
        
        if ( NULL == pEscBuffer )
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
            // Zero out allocated memory
            memset((PBYTE)pEscBuffer, 0, cbEscBuffer);

            pEscHeader = &(pEscBuffer->mxdcEscape);
            pEscData   = &(pEscBuffer->printTicketData);


            // Fill out the Escape Header data
            pEscHeader->cbInput    = cbEscBuffer;
            pEscHeader->cbOutput   = 0;
            pEscHeader->opCode     = OpCode; 

            // Fill out the actual escape data
            pEscData->dwDataSize   = cbInBuffer + sizeof(pEscData->dwDataSize);
            memcpy(&pEscData->bData, pbInBuffer, cbInBuffer);
        }
    }

    if ( SUCCEEDED(hr ) )
    {
        *ppbOutBuffer = (PBYTE)pEscBuffer;
        *pcbOutBuffer = cbEscBuffer;
    }

    return hr;

}

HRESULT
PutTogetherEscapeStructureForFixedPage(
    __in                              ULONG                       OpCode,
    __in_bcount(cbInBuffer)           PBYTE                       pbInBuffer,
    __in                              DWORD                       cbInBuffer,
    __deref_out_bcount(*pcbOutBuffer) PBYTE                      *ppbOutBuffer,
    __out                             PDWORD                      pcbOutBuffer
    )
{
    
    P_MXDC_S0PAGE_PASSTHROUGH_ESCAPE_T  pEscBuffer  = NULL;
    P_MXDC_ESCAPE_HEADER_T              pEscHeader  = NULL;
    P_MXDC_S0PAGE_DATA_T                pEscData    = NULL;

    DWORD                               cbEscBuffer = 0;
    HRESULT                             hr          = S_OK;

    if ( NULL == pbInBuffer   ||
         0    == cbInBuffer   ||
         NULL == ppbOutBuffer ||
         NULL == pcbOutBuffer )
    {
        hr = E_INVALIDARG;
    }

    if ( SUCCEEDED(hr) )
    {
        *ppbOutBuffer = NULL;
        *pcbOutBuffer = 0;

        //
        // bData[1] is the beginig of the data buffer. The actual
        // size of the buffer is cbBuffer (which includes bData[1]
        // So we subtract 1 
        //
        cbEscBuffer = sizeof(MXDC_S0PAGE_PASSTHROUGH_ESCAPE_T) - sizeof(pEscData->bData); 
        cbEscBuffer += cbInBuffer;
    
        // Allocate contiguous data
        pEscBuffer = (P_MXDC_S0PAGE_PASSTHROUGH_ESCAPE_T)MemAlloc(cbEscBuffer);
        
        if ( NULL == pEscBuffer )
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
            // Zero out allocated memory
            // Note that memory allocated is not just sizeof(P_MXDC_S0PAGE_PASSTHROUGH_ESCAPE_T)
            // There are extra bytes at the end of the structure.
            memset((PBYTE)pEscBuffer, 0, cbEscBuffer);

            pEscHeader = &(pEscBuffer->mxdcEscape);
            pEscData   = &(pEscBuffer->xpsS0PageData);


            // Fill out the Escape Header data
            pEscHeader->cbInput    = cbEscBuffer;
            pEscHeader->cbOutput   = 0;
            pEscHeader->opCode     = OpCode; 

            // Fill out the actual escape data
            pEscData->dwSize       = cbInBuffer + sizeof(pEscData->dwSize);
            memcpy(&pEscData->bData, pbInBuffer, cbInBuffer);
        }
    }

    if ( SUCCEEDED(hr ) )
    {
        *ppbOutBuffer = (PBYTE)pEscBuffer;
        *pcbOutBuffer = cbEscBuffer;
    }

    return hr;
}
