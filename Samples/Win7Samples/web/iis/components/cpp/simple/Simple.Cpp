// Simple.cpp : Implementation of CSimple

#include "stdafx.h"
#include "CATLSmpl.h"
#include "Simple.h"

STDMETHODIMP CSimple::InterfaceSupportsErrorInfo(REFIID riid)
{
    static const IID* arr[] = 
    {
        &IID_ISimple,
    };

    for (int i = 0; i < sizeof(arr) / sizeof(arr[0]); i++)
    {
        if (InlineIsEqualGUID(*arr[i],riid))
            return S_OK;
    }
    return S_FALSE;
}

CSimple::CSimple()
    : m_bstrMyProperty(OLESTR("C++ ATL Simple Component"))
{}

//Put function for myProperty

STDMETHODIMP CSimple::put_myProperty(BSTR bstrInValue)
{
    if (bstrInValue == NULL)
        return E_POINTER;

    m_bstrMyProperty = bstrInValue;

    return S_OK;
}

//Get function for myProperty

STDMETHODIMP CSimple::get_myProperty(BSTR *pbstrOutValue)
{
    if (pbstrOutValue == NULL)
        return E_POINTER;

    // Get Value from Property

    *pbstrOutValue = m_bstrMyProperty.Copy();
    
    return S_OK;
}

//Basic Method to convert a string to uppercase

STDMETHODIMP CSimple::myMethod(BSTR bstrIn, BSTR *pbstrOut)  
{
    if (bstrIn == NULL || pbstrOut == NULL)
        return E_POINTER;

    // Create a temporary CComBSTR

    CComBSTR bstrTemp(bstrIn);

    if (!bstrTemp)
        return E_OUTOFMEMORY;

    // Make string uppercase   

    wcsupr(bstrTemp);  
    
    // Return m_str member of bstrTemp

    *pbstrOut = bstrTemp.Detach();

    return S_OK;
}
