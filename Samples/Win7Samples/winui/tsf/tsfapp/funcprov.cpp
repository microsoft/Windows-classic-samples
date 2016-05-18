/**************************************************************************
   THIS CODE AND INFORMATION IS PROVIDED 'AS IS' WITHOUT WARRANTY OF
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
   PARTICULAR PURPOSE.

   Copyright 2001 Microsoft Corporation. All Rights Reserved.
**************************************************************************/

/**************************************************************************

   File:          FuncProv.cpp

   Description:   ITfFunctionProvider implementation.

**************************************************************************/

/**************************************************************************
	#include statements
**************************************************************************/

#include "TSFEdit.h"
#include "globals.h"

/**************************************************************************

	CTSFEditWnd::GetType()

**************************************************************************/

STDMETHODIMP CTSFEditWnd::GetType(GUID *pguid)
{
    *pguid = GUID_NULL;
    
    return S_OK;
}
    
/**************************************************************************

	CTSFEditWnd::GetDescription()

**************************************************************************/

STDMETHODIMP CTSFEditWnd::GetDescription(BSTR *pbstrDesc)
{
    *pbstrDesc = SysAllocString(L"TSFApp Function Provider");
    
    return *pbstrDesc ? S_OK : E_OUTOFMEMORY;
}
    
/**************************************************************************

	CTSFEditWnd::GetFunction()

**************************************************************************/

STDMETHODIMP CTSFEditWnd::GetFunction(REFGUID rguid, REFIID riid, IUnknown **ppunk)
{
    HRESULT hr = E_NOINTERFACE;
    
    *ppunk = NULL;

    if(IsEqualGUID(rguid, GUID_NULL))
    {
    }
    
    return hr;
}

/**************************************************************************

	CTSFEditWnd::_InitFunctionProvider()

**************************************************************************/

BOOL CTSFEditWnd::_InitFunctionProvider()
{
    HRESULT hr = E_FAIL;
    ITfSourceSingle *pSourceSingle;

    hr = m_pThreadMgr->QueryInterface(&pSourceSingle);
    if(SUCCEEDED(hr))
    {
        hr = pSourceSingle->AdviseSingleSink(m_tfClientID, IID_ITfFunctionProvider, (ITfFunctionProvider*)this);
        
        pSourceSingle->Release();
    }

    return SUCCEEDED(hr);
}

/**************************************************************************

	CTSFEditWnd::_UninitFunctionProvider()

**************************************************************************/

void CTSFEditWnd::_UninitFunctionProvider()
{
    HRESULT hr;
    ITfSourceSingle *pSourceSingle;

    hr = m_pThreadMgr->QueryInterface(&pSourceSingle);
    if(SUCCEEDED(hr))
    {
        hr = pSourceSingle->UnadviseSingleSink(m_tfClientID, IID_ITfFunctionProvider);

        pSourceSingle->Release();
    }
}

