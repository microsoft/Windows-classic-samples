//////////////////////////////////////////////////////////////////////////////
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <conio.h>
#include "Service.h"
#include <strsafe.h>

WSD_LOCALIZED_STRING thisDeviceName = {NULL, L"StockQuote Device"};
WSD_LOCALIZED_STRING_LIST thisDeviceNameList = {NULL, &thisDeviceName};
const WSD_THIS_DEVICE_METADATA thisDeviceMetadata = {
    &thisDeviceNameList,        // FriendlyName;
    L"0.095",                   // FirmwareVersion;
    L"0123456789-9876543210",   // SerialNumber;
}; // thisDeviceMetadata

//////////////////////////////////////////////////////////////////////////////
// print_result - Display an HRESULT
//////////////////////////////////////////////////////////////////////////////
void print_result(HRESULT hr)
{
    if (hr == S_OK)
    {
        _cwprintf(L"[S_OK]\r\n");
    }
    else 
    {
        _cwprintf(L"[ERROR: %x]\r\n", hr);
    }
}

//////////////////////////////////////////////////////////////////////////////
// StockLookup - Dummy code representing looking up a stock quote
//////////////////////////////////////////////////////////////////////////////
float StockLookup(
    LPCWSTR pszSymbol)
{
    // Ignore "pszSymbol" for this sample
    UNREFERENCED_PARAMETER(pszSymbol);

    return 39.42F;
}

//////////////////////////////////////////////////////////////////////////////
// CStockQuoteService constructor
//////////////////////////////////////////////////////////////////////////////
CStockQuoteService::CStockQuoteService()
:   m_cRef(1)
{
}

//////////////////////////////////////////////////////////////////////////////
// CStockQuoteService::GetLastTradePrice
//  Service method which can be called by remote clients.  This method
//  takes the stock quote listed in the parameters structure and returns
//  a structure with a stock price.
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CStockQuoteService::GetLastTradePrice(
    TRADE_PRICE_REQUEST* parameters, 
    TRADE_PRICE** ppTradePrice)
{
    TRADE_PRICE* pTradePrice = NULL;
    HRESULT hr = S_OK;

    _cwprintf(L"Client invoking GetLastTradePrice method\r\n");

    // Validate parameters
    if( NULL == parameters )
    {
        return E_INVALIDARG;
    }

    if( NULL == parameters->tickerSymbol )
    {
        return E_INVALIDARG;
    }

    if( NULL == ppTradePrice )
    {
        return E_POINTER;
    }

    *ppTradePrice = NULL;

    // Allocate response structure with WSDAllocateLinkedMemory.  The
    // Deallocator for this port type's stub functions is WSDFreeLinkedMemory.
    _cwprintf(L"    Allocating response structure... ");
    pTradePrice = (TRADE_PRICE*)WSDAllocateLinkedMemory( NULL,
            sizeof(TRADE_PRICE) );

    if( NULL == pTradePrice )
    {
        hr = E_OUTOFMEMORY;
    }

    print_result( hr );

    // Fill response structure
    if( S_OK == hr )
    {
        _cwprintf(L"    Calling into StockLookup function... ");
        pTradePrice->price = StockLookup(parameters->tickerSymbol);
        print_result( hr );
    }

    if( S_OK == hr )
    {
        *ppTradePrice = pTradePrice;
        pTradePrice = NULL;

        _cwprintf(L"    Response has been prepared.\r\n");
    }

    _cwprintf(L"    Service method exit code: ");
    print_result( hr );

    // cleanup
    if( NULL != pTradePrice )
    {
        WSDFreeLinkedMemory( pTradePrice );
        pTradePrice = NULL;
    }

    return hr;
}

//////////////////////////////////////////////////////////////////////////////
// Usage
//////////////////////////////////////////////////////////////////////////////
void Usage()
{
    _cwprintf(L"StockQuoteService.exe [<device address>]\r\n");
    _cwprintf(L"Ex: StockQuoteService.exe urn:uuid:41F8BE9C-CF75-48f7-9294-1B2DC8698214\r\n");
}

//////////////////////////////////////////////////////////////////////////////
// Main Entry Point
//      argv[0] = executable name
//      argv[1] = device address
//////////////////////////////////////////////////////////////////////////////
int _cdecl wmain(
    int argc, 
    __in_ecount(argc) LPWSTR* argv)
{
    HRESULT hr = S_OK;
    WCHAR szDeviceAddress[MAX_PATH];
    IWSDDeviceHost* pHost = NULL;
    CStockQuoteService* pStockQuoteService = NULL;
    IStockQuote* pIStockQuote = NULL;
    UUID uuid;

    //////////////////////////////////////////////////////////////////////////
    // Process command-line parameters
    //////////////////////////////////////////////////////////////////////////

    if( argc > 1 )
    {
        if( 0 == wcscmp( argv[1], L"/?") || 0 == wcscmp( argv[1], L"-?" ) )
        {
            Usage();
            return -1;
        }
        else
        {
            // Copy device address, if specified
            hr = ::StringCbCopyW( szDeviceAddress,
                    sizeof(szDeviceAddress), argv[1] );
        }
    }
    else
    {
        // If none was specified, generate an ID for our host
        RPC_STATUS st = UuidCreate( &uuid );
        if( st != RPC_S_OK )
        {
            hr = E_FAIL;
        }

        if( S_OK == hr )
        {
            hr = StringCbPrintfW(
                szDeviceAddress, sizeof(szDeviceAddress),
                L"urn:uuid:%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
                uuid.Data1, uuid.Data2, uuid.Data3,
                uuid.Data4[0], uuid.Data4[1], uuid.Data4[2], uuid.Data4[3],
                uuid.Data4[4], uuid.Data4[5], uuid.Data4[6], uuid.Data4[7]);
        }
    }

    //////////////////////////////////////////////////////////////////////////
    // Build our StockQuote service
    //////////////////////////////////////////////////////////////////////////

    // Instaniate our IStockQuote object
    if( S_OK == hr )
    {
        _cwprintf(L"Creating the stock quote service... ");
        pStockQuoteService = new CStockQuoteService();

        if( NULL == pStockQuoteService )
        {
            hr = E_OUTOFMEMORY;
        }
        print_result( hr );
    }

    if( S_OK == hr )
    {
        _cwprintf(L"Querying stock quote for service interface... ");
        hr = pStockQuoteService->QueryInterface( __uuidof(IStockQuote),
                (void**)&pIStockQuote );
        print_result( hr );
    }

    if( S_OK == hr )
    {
        _cwprintf(L"Creating stock quote host with ID %s... ", szDeviceAddress);
        hr = CreateStockQuoteHost( szDeviceAddress, &thisDeviceMetadata,
                pIStockQuote, &pHost, NULL );
        print_result( hr );
    }

    //////////////////////////////////////////////////////////////////////////
    // Start host and wait for a keypress
    //////////////////////////////////////////////////////////////////////////
    if( S_OK == hr )
    {
        _cwprintf(L"Starting stock quote host... ");
        hr = pHost->Start( 0, NULL, NULL );
        print_result( hr );
    }

    if( S_OK == hr )
    {
        _cwprintf(L">>> Service running.  Press <Enter> to stop service. <<<\r\n");
        // Ignore character returned from prompt
        (void)::_getch();
    }

    //////////////////////////////////////////////////////////////////////////
    // Stop and terminate host
    //////////////////////////////////////////////////////////////////////////
    if( S_OK == hr )
    {
        _cwprintf(L"Stopping Host... ");
        hr = pHost->Stop();
        print_result( hr );
    }

    _cwprintf(L"Cleaning up resources... ");

    // cleanup

    // Terminating host is non-optional, so it's done in the cleanup block
    if( NULL != pHost )
    {
        (void)pHost->Terminate();
        pHost->Release();
        pHost = NULL;
    }

    if( NULL != pStockQuoteService )
    {
        pStockQuoteService->Release();
        pStockQuoteService = NULL;
    }

    if( NULL != pIStockQuote )
    {
        pIStockQuote->Release();
        pIStockQuote = NULL;
    }

    _cwprintf(L"finished.\r\n");

    if( SUCCEEDED(hr) )
    {
        return 0;
    }

    return -1;
} // main
