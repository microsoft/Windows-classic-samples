//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//

#pragma once

#include <msxml6.h>
#include <wrl.h>

using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Details;

#define MAX_BUFFER_LENGTH 4096

class CXMLHttpRequest2Callback :
    public Microsoft::WRL::RuntimeClass<RuntimeClassFlags<ClassicCom>, IXMLHTTPRequest2Callback>
{
private:

    //
    // Event object handle.
    //

    HANDLE m_hComplete;

    //
    // Return value from final callbacks, including OnResponseReceived or
    // OnError event handler. Once a final callback is called, no more callback
    // will be executed.
    //

    HRESULT m_hr;

    //
    // HTTP status code from OnHeadersAvailable event handler.
    //

    DWORD m_dwStatus;

    CXMLHttpRequest2Callback();

    ~CXMLHttpRequest2Callback();

    STDMETHODIMP
    RuntimeClassInitialize();

    friend HRESULT Microsoft::WRL::Details::MakeAndInitialize<CXMLHttpRequest2Callback,CXMLHttpRequest2Callback>(CXMLHttpRequest2Callback **);

public:

    STDMETHODIMP
    OnRedirect(
        __RPC__in_opt IXMLHTTPRequest2 *pXHR,
        __RPC__in_string const WCHAR *pwszRedirectUrl
    );

    STDMETHODIMP
    OnHeadersAvailable(
        __RPC__in_opt IXMLHTTPRequest2 *pXHR,
        DWORD dwStatus,
        __RPC__in_string const WCHAR *pwszStatus
    );

    STDMETHODIMP
    OnDataAvailable(
        __RPC__in_opt IXMLHTTPRequest2 *pXHR,
        __RPC__in_opt ISequentialStream *pResponseStream
    );

    STDMETHODIMP
    OnResponseReceived(
        __RPC__in_opt IXMLHTTPRequest2 *pXHR,
        __RPC__in_opt ISequentialStream *pResponseStream
    );

    STDMETHODIMP
    OnError(
        __RPC__in_opt IXMLHTTPRequest2 *pXHR,
        HRESULT hrError
    );

    STDMETHODIMP
    ReadFromStream(
        _In_opt_ ISequentialStream *pStream
    );

    STDMETHODIMP
    WaitForComplete(
        _Out_ PDWORD pdwStatus
    );
};
