//////////////////////////////////////////////////////////////////////////////
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//////////////////////////////////////////////////////////////////////////////
#pragma once

#include <wsdapi.h>

// StockQuote.h is automatically generated from StockQuote.idl
#include "StockQuote.h"

//////////////////////////////////////////////////////////////////////////////
// CStockQuoteService Class
//       Implements the service functionality
//////////////////////////////////////////////////////////////////////////////
class CStockQuoteService : public IStockQuote
{
private:
    ULONG m_cRef;

public:
    CStockQuoteService();

    HRESULT STDMETHODCALLTYPE GetLastTradePrice(TRADE_PRICE_REQUEST* body, TRADE_PRICE** bodyOut);

public:
    //////////////////////////////////////////////////////////////////////////
    // Methods to make this class act like a COM object
    //////////////////////////////////////////////////////////////////////////
    HRESULT STDMETHODCALLTYPE QueryInterface(
        REFIID riid, 
        void** ppvObject)
	{
		if (NULL == ppvObject)
        {
			return E_POINTER;
        }

		HRESULT hr = S_OK;
		*ppvObject = NULL;

		if (( __uuidof(IStockQuote) == riid) ||
			( __uuidof(IUnknown) == riid) )
		{
			*ppvObject = (IStockQuote *)this;
		}
		else
		{
			hr = E_NOINTERFACE;
		}

		if (SUCCEEDED(hr))
		{
			((LPUNKNOWN) *ppvObject)->AddRef( );
		}

		return hr;
	} 

    ULONG STDMETHODCALLTYPE AddRef()
	{
		ULONG ulNewRefCount = (ULONG)InterlockedIncrement((LONG *)&m_cRef);
		return ulNewRefCount;
	}

    ULONG STDMETHODCALLTYPE Release()
	{
		ULONG ulNewRefCount = (ULONG)InterlockedDecrement((LONG *)&m_cRef);

		if( 0 == ulNewRefCount )
		{
			delete this;
		}
		return ulNewRefCount;
	} 
};
