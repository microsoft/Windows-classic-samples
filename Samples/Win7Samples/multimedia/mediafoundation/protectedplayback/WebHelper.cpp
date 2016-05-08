//-----------------------------------------------------------------------------
// File: WebHelper.h
// Desc: Class for opening a browser window and sending data via HTTP PUSH.
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
//  Copyright (C) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------

#include "ProtectedPlayback.h"
#include "WebHelper.h"


//-----------------------------------------------------------------------------
// Name: Init
// Desc: Initializes the InternetExplorer object.
//-----------------------------------------------------------------------------

HRESULT WebHelper::Init(DispatchCallback *pCallback)
{
    m_pDispatchCB = pCallback;

    IConnectionPointContainer *pCPContainer = NULL;
    HWND hwndBrowser = NULL;

    // Create the InternetExplorer object.
    HRESULT hr = CoCreateInstance(CLSID_InternetExplorer, NULL,
        CLSCTX_ALL, IID_IWebBrowser2, (void**)&m_pBrowser);
    LOG_MSG_IF_FAILED(L"CoCreateInstance(CLSID_InternetExplorer)", hr);

    // Set up the connection point so that we receive events.
    if (SUCCEEDED(hr))
    {
        hr = m_pBrowser->QueryInterface(IID_IConnectionPointContainer, (void**)&pCPContainer);
        LOG_MSG_IF_FAILED(L"QueryInterface for IConnectionPointContainer)", hr);
    }

    if (SUCCEEDED(hr))
    {
        hr = pCPContainer->FindConnectionPoint(DIID_DWebBrowserEvents2, &m_pCP);
        LOG_MSG_IF_FAILED(L"FindConnectionPoint)", hr);
    }

    if (SUCCEEDED(hr))
    {
        hr = m_pCP->Advise(this, &m_dwCookie);
        LOG_MSG_IF_FAILED(L"Advise)", hr);
    }

    if (SUCCEEDED(hr))
    {
        hr = m_pBrowser->get_HWND((SHANDLE_PTR*)&hwndBrowser);
    }

    // Move the browser window to the front.
    if (SUCCEEDED(hr))
    {
        SetWindowPos(hwndBrowser, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    }

    SAFE_RELEASE(pCPContainer);

    return hr;
}

//-----------------------------------------------------------------------------
// Name: Exit
// Desc: Cleans up.
//-----------------------------------------------------------------------------

void WebHelper::Exit()
{
    // Release the connection point. 
    if (m_pCP)
    {
        m_pCP->Unadvise(m_dwCookie);
    }

    SAFE_RELEASE(m_pBrowser);
    SAFE_RELEASE(m_pCP);

    m_pDispatchCB = NULL;
}


//-----------------------------------------------------------------------------
// Name: OpenURLWithData
// Desc: Navigates to a URL and POSTs the license acquisition data.
//
// wszURL: The license acquisition URL
// pbPostData: The license acquisition data.
// cbData: Size of the data, in bytes.
//
// (Get the values of these parameters from WM_GET_LICENSE_DATA structure.)
//-----------------------------------------------------------------------------

HRESULT WebHelper::OpenURLWithData(const WCHAR *wszURL, const BYTE *pbPostData, DWORD cbData)
{
    // This string is the header needed for HTTP POST actions.
    const LPWSTR POST_HEADER_DATA = L"Content-Type: application/x-www-form-urlencoded\r\n";

    if (!wszURL)
    {
        return E_INVALIDARG;
    }

    if (!m_pBrowser)
    {
        return E_UNEXPECTED;
    }
    
    HRESULT hr = S_OK;
    BSTR    bstrURL = NULL;
    VARIANT vtEmpty;
    VARIANT vtHeader;
    VARIANT vtPostData;

    VariantInit(&vtEmpty);
    VariantInit(&vtHeader);
    VariantInit(&vtPostData);

    // Allocate a BSTR for the URL.
    bstrURL = SysAllocString(wszURL);
    if (bstrURL == NULL)
    {
        hr = E_OUTOFMEMORY;
    }

    // Allocate a BSTR for the header.
    if (SUCCEEDED(hr))
    {
        vtHeader.bstrVal = SysAllocString(POST_HEADER_DATA);
        if (vtHeader.bstrVal == NULL)
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
            vtHeader.vt = VT_BSTR;
        }
    }


    if (SUCCEEDED(hr))
    {
        if ( pbPostData )
        {
            // Convert the POST data to a safe array in a variant. The safe array type is VT_UI1.

            void *pvData = NULL;
            SAFEARRAY *saPostData = SafeArrayCreateVector(VT_UI1, 0, cbData);
            if (saPostData == NULL)
            {
                hr = E_OUTOFMEMORY;
            }

            if (SUCCEEDED(hr))
            {
                hr = SafeArrayAccessData(saPostData, &pvData);
            }

            if (SUCCEEDED(hr))
            {
                CopyMemory((BYTE*)pvData, pbPostData, cbData);
                hr = SafeArrayUnaccessData(saPostData);
            }

            if (SUCCEEDED(hr))
            {
                vtPostData.vt = VT_ARRAY | VT_UI1;
                vtPostData.parray = saPostData;
            }
        }
    }



    // Make the IE window visible.
    if (SUCCEEDED(hr))
    {
        hr = m_pBrowser->put_Visible(VARIANT_TRUE);
        LOG_MSG_IF_FAILED(L"put_Visible)", hr);
    }

    // Navigate to the URL.
    if (SUCCEEDED(hr))
    {
        hr = m_pBrowser->Navigate(bstrURL, &vtEmpty, &vtEmpty, &vtPostData, &vtHeader);
        LOG_MSG_IF_FAILED(L"Navigate", hr);
    }

    SysFreeString(bstrURL);

    VariantClear(&vtEmpty);
    VariantClear(&vtHeader);
    VariantClear(&vtPostData);

    return hr;
}

