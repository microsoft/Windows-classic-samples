//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//

#include "XMLHttpRequest2Callback.h"

CXMLHttpRequest2Callback::CXMLHttpRequest2Callback()
{
    m_hr = S_OK;
    m_dwStatus = 0;
    m_hComplete = NULL;
}

CXMLHttpRequest2Callback::~CXMLHttpRequest2Callback()
{
    if (m_hComplete)
    {
        CloseHandle(m_hComplete);
        m_hComplete = NULL;
    }
}

STDMETHODIMP
CXMLHttpRequest2Callback::RuntimeClassInitialize()
/*++

Routine Description:

    Initalize the call back instance.

Arguments:

    None.

Return Value:

    HRESULT.

--*/
{
    HRESULT hr = S_OK;
    HANDLE hEvent = NULL;

    //
    // Callers needing to receive completion or status events on a STA or UI
    // thread must use a mechanism that will not block the threads window message
    // pump. One example is by posting a window message to the STA or UI thread
    // window handle.
    //

    hEvent = CreateEventEx(NULL, NULL, CREATE_EVENT_MANUAL_RESET, EVENT_ALL_ACCESS);

    if (hEvent == NULL)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }

    m_hComplete = hEvent;
    hEvent = NULL;

Exit:

    if (hEvent)
    {
        CloseHandle(hEvent);
        hEvent = NULL;
    }

    return hr;
}

STDMETHODIMP
CXMLHttpRequest2Callback::OnRedirect(
    __RPC__in_opt IXMLHTTPRequest2 *pXHR,
    __RPC__in_string const WCHAR *pwszRedirectUrl
)
/*++

Routine Description:

    This funciton is called when the HTTP request is being redirected to a new URL.

    This callback function must not throw any exceptions.

Arguments:

    pXHR - The interface pointer of IXMLHTTPRequest2 object.

    pwszRedirectUrl - The new URL for the HTTP request.

Return Value:

    HRESULT.

--*/
{
    UNREFERENCED_PARAMETER(pXHR);
    UNREFERENCED_PARAMETER(pwszRedirectUrl);

    return S_OK;
}

STDMETHODIMP
CXMLHttpRequest2Callback::OnHeadersAvailable(
    __RPC__in_opt IXMLHTTPRequest2 *pXHR,
    DWORD dwStatus,
    __RPC__in_string const WCHAR *pwszStatus
)
/*++

Routine Description:

    Sends a request using the Request Handle specified and implements
    proxy failover if supported.

    This callback function must not throw any exceptions.

Arguments:

    pXHR - The interface pointer of IXMLHTTPRequest2 object.

    dwStatus - The value of HTTP status code.

    pwszStatus - The description text of HTTP status code.

Return Value:

    HRESULT.

--*/
{
    UNREFERENCED_PARAMETER(pwszStatus);

    HRESULT hr = S_OK;

    PWSTR pwszHeaders = NULL;
    PWSTR pwszSingleHeader = NULL;

    if (pXHR == NULL)
    {
        hr = E_INVALIDARG;
        goto Exit;
    }

    //
    // Demonstrate how to get all response headers.
    //

    hr = pXHR->GetAllResponseHeaders(&pwszHeaders);
    if (FAILED(hr))
    {
        goto Exit;
    }

    //
    // Demonstrate how to get a specific response header.
    //

    hr = pXHR->GetResponseHeader(L"server", &pwszSingleHeader);
    if (FAILED(hr) &&
        hr != HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
    {
        goto Exit;
    }

    hr = S_OK;

Exit:

    if (pwszHeaders != NULL)
    {
        CoTaskMemFree(pwszHeaders);
        pwszHeaders = NULL;
    }

    if (pwszSingleHeader != NULL)
    {
        CoTaskMemFree(pwszSingleHeader);
        pwszSingleHeader = NULL;
    }

    m_dwStatus = dwStatus;
    return hr;
}

STDMETHODIMP
CXMLHttpRequest2Callback::ReadFromStream(
    _In_opt_ ISequentialStream *pStream
)
/*++

Routine Description:

    Demonstrate how to read from the HTTP response stream.

Arguments:

    pStream - the data stream read form the http response.

Return Value:

    HRESULT.

--*/
{
    HRESULT hr = S_OK;
    BYTE buffer[MAX_BUFFER_LENGTH];
    DWORD cbRead = 0;

    if (pStream == NULL)
    {
        hr = E_INVALIDARG;
        goto Exit;
    }

    for(;;)
    {
        hr = pStream->Read(buffer, MAX_BUFFER_LENGTH - 1, &cbRead);

        if (FAILED(hr) ||
            cbRead == 0)
        {
            goto Exit;
        }

        buffer[cbRead] = 0;
    }

Exit:

    return hr;
}

STDMETHODIMP
CXMLHttpRequest2Callback::OnDataAvailable(
    __RPC__in_opt IXMLHTTPRequest2 *pXHR,
    __RPC__in_opt ISequentialStream *pResponseStream
)
/*++

Routine Description:

    This function is called when a portion of the entity body has been received.
    The application can begin processing the data at this point,
    or wait until the whole response is complete.

    This callback function must not throw any exceptions.

Arguments:

    pXHR - The interface pointer of IXMLHTTPRequest2 object.

    pResponseStream - a pointer to the input stream.

Return Value:

    HRESULT.

--*/
{
    UNREFERENCED_PARAMETER(pXHR);

    //
    // This sample function is processing data as it is received by reading from
    // the response stream. If real-time chunk-by-chunk processing (such as for
    // streaming applications) is not needed, then the entire response is available
    // from the OnResponseReceived callback.  Receiving will be suspended until
    // this callback function returns and this callback will be invoked multiple
    // times during a request.  This callback function must not block and
    // should not perform costly operations such as UI updates.
    //

    return ReadFromStream(pResponseStream);
}

STDMETHODIMP
CXMLHttpRequest2Callback::OnResponseReceived(
    __RPC__in_opt IXMLHTTPRequest2 *pXHR,
    __RPC__in_opt ISequentialStream *pResponseStream
)
/*++

Routine Description:

    Called when the entire entity body has been received.
    At this point the application can begin processing the data by calling Read
    on the response ISequentialStream or store a reference to the ISequentialStream
    for later processing.

    This callback function must not throw any exceptions.

Arguments:

    pXHR - The interface pointer of IXMLHTTPRequest2 object.

    pResponseStream - a pointer to the input stream.

Return Value:

    HRESULT.

--*/
{
    UNREFERENCED_PARAMETER(pXHR);
    UNREFERENCED_PARAMETER(pResponseStream);

    m_hr = S_OK;
    SetEvent(m_hComplete);
    return m_hr;
}

STDMETHODIMP
CXMLHttpRequest2Callback::OnError(
    __RPC__in_opt IXMLHTTPRequest2 *pXHR,
    HRESULT hrError
)
/*++

Routine Description:

   Called when an error occurs during the HTTP request.  The error is indicated in hrError.

   This callback function must not throw any exceptions.

Arguments:

    pXHR - The interface pointer of IXMLHTTPRequest2 object.

    hrError - The errocode for the httprequest.

Return Value:

    HRESULT.

--*/
{
    UNREFERENCED_PARAMETER(pXHR);

    m_hr = hrError;
    SetEvent(m_hComplete);
    return S_OK;
}

STDMETHODIMP
CXMLHttpRequest2Callback::WaitForComplete(
    _Out_ PDWORD pdwStatus
)
/*++

Routine Description:

    Waiting for completion of the request. Once it's done, get the execution
    result of final call backs, and http status code if it's available.

    N.B. Callers needing to receive completion or status events on a STA or UI
    thread must use a mechanism that will not block the threads window message
    pump. One example is by posting a window message to the STA or UI thread
    window handle.

Arguments:

    pdwStatus - Supplies a pointer to access the status code.

Return Value:

    HRESULT.

--*/
{
    DWORD dwError = ERROR_SUCCESS;
    HRESULT hr = S_OK;

    if (pdwStatus == NULL)
    {
        hr = E_INVALIDARG;
        goto Exit;
    }

    dwError = WaitForSingleObject(m_hComplete, INFINITE);

    if (dwError == WAIT_FAILED)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }
    else if (dwError != WAIT_OBJECT_0)
    {
        hr = E_ABORT;
        goto Exit;
    }

    if (FAILED(m_hr))
    {
        hr = m_hr;
        goto Exit;
    }

    hr = S_OK;
    *pdwStatus = m_dwStatus;

Exit:

    return hr;
}
