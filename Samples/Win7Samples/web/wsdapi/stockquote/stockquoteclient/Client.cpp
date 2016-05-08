/////////////////////////////////////////////////////////////////////
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include <stdio.h>
#include <conio.h>
#include "Client.h"
#include <strsafe.h>

//////////////////////////////////////////////////////////////////////////////
// print_result - Display HRESULTs
//////////////////////////////////////////////////////////////////////////////
void print_result(HRESULT hr)
{
    if( S_OK == hr )
    {
        _cwprintf(L"[S_OK]\r\n");
    }
    else 
    {
        _cwprintf(L"[ERROR: %x]\r\n", hr);
    }
}

//////////////////////////////////////////////////////////////////////////////
// Usage
//////////////////////////////////////////////////////////////////////////////
void Usage()
{
    _cwprintf(L"StockQuoteClient.exe <device-address>\r\n");
    _cwprintf(L"Ex: StockQuoteClient.exe urn:uuid:41F8BE9C-CF75-48f7-9294-1B2DC8698214\r\n");
}

//////////////////////////////////////////////////////////////////////////////
// Main Entry Point
//      argv[0] = executable name
//      argv[1] = device address
//////////////////////////////////////////////////////////////////////////////
int _cdecl wmain(int argc, __in_ecount(argc) LPWSTR* argv)
{
    LPCWSTR pszDeviceAddress = NULL;
    WCHAR szLocalAddress[MAX_PATH];
    HRESULT hr = S_OK;
    UUID uuid = { 0 };
    CStockQuoteProxy* pStockQuoteProxy = NULL;
    TRADE_PRICE_REQUEST tradePriceRequest = { 0 };
    TRADE_PRICE* pTradePrice = NULL;

    //////////////////////////////////////////////////////////////////////////
    // Process command-line parameters
    //////////////////////////////////////////////////////////////////////////

    if( argc <= 1 )
    {
        Usage();
        return -1;
    }

    pszDeviceAddress = argv[1];

    //////////////////////////////////////////////////////////////////////////
    // Build the proxy
    //////////////////////////////////////////////////////////////////////////

    // Generate local ID for our proxy
    if( S_OK == hr )
    {
        RPC_STATUS st = UuidCreate( &uuid );
        if( st != RPC_S_OK )
        {
            hr = E_FAIL;
        }
    }

    if( S_OK == hr )
    {
        hr = StringCbPrintfW(
            szLocalAddress, sizeof(szLocalAddress),
            L"urn:uuid:%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
            uuid.Data1, uuid.Data2, uuid.Data3,
            uuid.Data4[0], uuid.Data4[1], uuid.Data4[2], uuid.Data4[3], 
            uuid.Data4[4], uuid.Data4[5], uuid.Data4[6], uuid.Data4[7]); 
    }

    // Build the proxy
    if( S_OK == hr )
    {
        _cwprintf(L"Creating a proxy for device %s...", pszDeviceAddress);
        hr = CreateCStockQuoteProxy( pszDeviceAddress, szLocalAddress,
                &pStockQuoteProxy, NULL );
        print_result( hr );
    }

    //////////////////////////////////////////////////////////////////////////
    // Call service methods on the proxy
    //////////////////////////////////////////////////////////////////////////

    if( S_OK == hr )
    {
        _cwprintf(L"Calling GetLastTradePrice... ");
        // Request price for our fictional "QQQQ" stock
        tradePriceRequest.tickerSymbol = L"QQQQ";

        hr = pStockQuoteProxy->GetLastTradePrice( &tradePriceRequest,
                &pTradePrice );
        // pTradePrice will be destroyed in "cleanup" area at end of function
        print_result( hr );
    }


    if( S_OK == hr )
    {
        _cwprintf(L"Trade Price for %s is %f\r\n", 
            tradePriceRequest.tickerSymbol, 
            pTradePrice->price);
    }

    _cwprintf(L"Cleaning up resources... ");

    // cleanup
    if( NULL != pTradePrice )
    {
        WSDFreeLinkedMemory( pTradePrice );
        pTradePrice = NULL;
    }

    if( NULL != pStockQuoteProxy )
    {
        pStockQuoteProxy->Release();
        pStockQuoteProxy = NULL;
    }

    _cwprintf(L"finished.\r\n");

    if( SUCCEEDED(hr) )
    {
        return 0;
    }

    return -1;
} // main
