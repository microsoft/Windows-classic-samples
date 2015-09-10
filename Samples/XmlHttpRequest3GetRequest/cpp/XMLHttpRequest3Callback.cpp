//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//

#include <windows.h>
#include <wincrypt.h>
#include <cryptuiapi.h>
#include "XMLHttpRequest3Callback.h"

CXMLHttpRequest3Callback::CXMLHttpRequest3Callback()
{
    m_hr = S_OK;
    m_dwStatus = 0;
    m_hComplete = NULL;

    m_fRetry = FALSE;
    m_dwCertIgnoreFlags = 0;
    m_cIssuerList = 0;
    m_rgpwszIssuerList = NULL;
}

CXMLHttpRequest3Callback::~CXMLHttpRequest3Callback()
{
    if (m_hComplete)
    {
        CloseHandle(m_hComplete);
        m_hComplete = NULL;
    }

    if (m_rgpwszIssuerList)
    {
        FreeIssuerList(m_cIssuerList, m_rgpwszIssuerList);
        m_cIssuerList = 0;
        m_rgpwszIssuerList = NULL;
    }
}

STDMETHODIMP
CXMLHttpRequest3Callback::RuntimeClassInitialize()
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
CXMLHttpRequest3Callback::OnRedirect(
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
CXMLHttpRequest3Callback::OnHeadersAvailable(
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
CXMLHttpRequest3Callback::ReadFromStream(
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
CXMLHttpRequest3Callback::OnDataAvailable(
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
CXMLHttpRequest3Callback::OnResponseReceived(
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
CXMLHttpRequest3Callback::OnError(
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
CXMLHttpRequest3Callback::OnServerCertificateReceived( 
    __RPC__in_opt IXMLHTTPRequest3 *pXHR,
    DWORD dwCertErrors,
    DWORD cServerCertChain,
    __RPC__in_ecount_full_opt(cServerCertChain) const XHR_CERT *rgServerCertChain
)
/*++

Routine Description:

   Called when server certificate errors or the server certificate chain is avaiable.

   This callback function must not throw any exceptions.

Arguments:

    pXHR - The interface pointer of IXMLHTTPRequest3 object.

    dwCertErrors - A bitmask of server certificate errors (XHR_CERT_ERROR_*).

    cServerCertChain - The number of certificates in the server certificate chain.

    rgServerCertChain - The server certificate chain as an array of encoded certificates.

Return Value:

    HRESULT.

--*/
{
    UNREFERENCED_PARAMETER(pXHR);

    PCCERT_CONTEXT pCertContext = NULL;
    BYTE *pbCertEncoded = NULL;
    DWORD cbCertEncoded = 0;
    DWORD i = 0;

    if (cServerCertChain == 0 || rgServerCertChain == NULL)
    {
        goto Exit;
    }

    //
    // Each element of the chain is an encoded certificate.
    //

    for (i = 0; i < cServerCertChain; i++)
    {
        pbCertEncoded = rgServerCertChain[i].pbCert;
        cbCertEncoded = rgServerCertChain[i].cbCert;

        pCertContext = CertCreateCertificateContext(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                                                    pbCertEncoded,
                                                    cbCertEncoded);

        //
        // Use the certificate context here.
        //

        if (pCertContext)
        {
            CertFreeCertificateContext(pCertContext);
            pCertContext = NULL;
        }
    }

    //
    // On the retry connection, ignore each specific cert error.
    //

    if ((dwCertErrors & XHR_CERT_ERROR_REVOCATION_FAILED) != 0)
    {
        m_dwCertIgnoreFlags |= XHR_CERT_IGNORE_REVOCATION_FAILED;
        m_fRetry = TRUE;
    }

    if ((dwCertErrors & XHR_CERT_ERROR_UNKNOWN_CA) != 0)
    {
        m_dwCertIgnoreFlags |= XHR_CERT_IGNORE_UNKNOWN_CA;
        m_fRetry = TRUE;
    }

    if ((dwCertErrors & XHR_CERT_ERROR_CERT_CN_INVALID) != 0)
    {
        m_dwCertIgnoreFlags |= XHR_CERT_IGNORE_CERT_CN_INVALID;
        m_fRetry = TRUE;
    }

    if ((dwCertErrors & XHR_CERT_ERROR_CERT_DATE_INVALID) != 0)
    {
        m_dwCertIgnoreFlags |= XHR_CERT_IGNORE_CERT_DATE_INVALID;
        m_fRetry = TRUE;
    }

Exit:

    return S_OK;
}

STDMETHODIMP
CXMLHttpRequest3Callback::OnClientCertificateRequested( 
    __RPC__in_opt IXMLHTTPRequest3 *pXHR,
    DWORD cIssuerList,
    __RPC__in_ecount_full_opt(cIssuerList) const WCHAR **rgpwszIssuerList
)
/*++

Routine Description:

   Called when the server requests client certificates.

   This callback function must not throw any exceptions.

Arguments:

    pXHR - The interface pointer of IXMLHTTPRequest3 object.

    cIssuerList - The number of issuer strings in the issuer list.

    rgpwszIssuerList - The array of issuer strings for filtering client certificates.

Return Value:

    HRESULT.

--*/
{
    UNREFERENCED_PARAMETER(pXHR);

    HRESULT hr = S_OK;

    if (cIssuerList == 0 || rgpwszIssuerList == NULL)
    {
        goto Exit;
    }

    //
    // On the retry connection, use issuer list to filter client certificates.
    //

    hr = DuplicateIssuerList(cIssuerList, rgpwszIssuerList, &m_rgpwszIssuerList);

    if (FAILED(hr))
    {
        goto Exit;
    }

    m_cIssuerList = cIssuerList;
    m_fRetry = TRUE;

Exit:

    return hr;
}

STDMETHODIMP
CXMLHttpRequest3Callback::DuplicateIssuerList( 
    _In_ DWORD cIssuerList,
    _In_reads_(cIssuerList) const WCHAR **rgpwszIssuerList,
    _Out_ const WCHAR ***prgpwszDuplicateIssuerList
)
/*++

Routine Description:

   Create a duplicate of the issuer list string array.

Arguments:

    cIssuerList - The number of issuers in the issuer list.

    rgpwszIssuerList - The source issuer list.

    prgpwszDuplicateIssuerList - The newly created duplicate issuer list.

Return Value:

    HRESULT.

--*/
{
    HRESULT hr = S_OK;
    DWORD i = 0;
    DWORD cchIssuer = 0;
    WCHAR *pwszIssuer = NULL;
    const WCHAR **rgpwszDuplicateIssuerList = NULL;

    if (cIssuerList == 0 || rgpwszIssuerList == NULL)
    {
        hr = E_INVALIDARG;
        goto Exit;
    }

    rgpwszDuplicateIssuerList = (const WCHAR**) new WCHAR*[cIssuerList];

    if (rgpwszDuplicateIssuerList == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    memset(rgpwszDuplicateIssuerList, 0, sizeof(WCHAR*) * cIssuerList);

    for (i = 0; i < cIssuerList && rgpwszIssuerList[i] != NULL; i++)
    {
        cchIssuer = (DWORD) wcslen(rgpwszIssuerList[i]);
        pwszIssuer = new WCHAR[cchIssuer + 1];

        if (pwszIssuer == NULL)
        {
            hr = E_OUTOFMEMORY;
            goto Exit;
        }

        wcscpy_s(pwszIssuer, cchIssuer + 1, rgpwszIssuerList[i]);
        (rgpwszDuplicateIssuerList)[i] = pwszIssuer;
    }

Exit:

    if (FAILED(hr))
    {
        FreeIssuerList(i, rgpwszDuplicateIssuerList);
        rgpwszDuplicateIssuerList = NULL;
    }

    *prgpwszDuplicateIssuerList = rgpwszDuplicateIssuerList;

    return hr;
}

STDMETHODIMP
CXMLHttpRequest3Callback::FreeIssuerList(
    _In_ DWORD cIssuerList,
    _Frees_ptr_ const WCHAR **rgpwszIssuerList
)
/*++

Routine Description:

    Free a duplicated issuer list.

Arguments:

    cIssuerList - The count of the issuers in the duplicate issuer list.

    rgpwszIssuerList - The duplicated issuer list to be freed.

Return Value:

    HRESULT.

--*/
{
    DWORD i = 0;

    if (cIssuerList == 0 || rgpwszIssuerList == NULL)
    {
        goto Exit;
    }

    for (i = 0; i < cIssuerList; i++)
    {
        delete[] rgpwszIssuerList[i];
        rgpwszIssuerList[i] = NULL;
    }

    delete[] rgpwszIssuerList;

Exit:

    return S_OK;
}

STDMETHODIMP
CXMLHttpRequest3Callback::CompareIssuer(
    _In_ VOID *pvCertContext,
    _In_ DWORD cIssuerList,
    _In_reads_(cIssuerList) const WCHAR **rgpwszIssuerList,
    _Out_ BOOL *pfMatch
)
/*++

Routine Description:

    Check to see if the issuer of the certificate context is among any of the
    issuers in the issuer list.

Arguments:

    pvCertContext - A pointer to a certificate context.

    cIssuerList - The count of issuers in the issuer list.

    rgpwszIssuerList - The issuer list.

    pfMatch - A pointer to whether a match was found.

Return Value:

    HRESULT.

--*/
{
    DWORD i = 0;
    WCHAR *pwszName = NULL;
    DWORD cchName = 0;
    PCCERT_CONTEXT pCertContext = (PCCERT_CONTEXT) pvCertContext;

    *pfMatch = FALSE;

    cchName = CertNameToStr(X509_ASN_ENCODING,
                            &pCertContext->pCertInfo->Issuer,
                            CERT_SIMPLE_NAME_STR | CERT_NAME_STR_CRLF_FLAG | CERT_NAME_STR_NO_PLUS_FLAG,
                            NULL,
                            0);

    pwszName = new WCHAR[cchName];

    if (pwszName == NULL)
    {
        goto Exit;
    }

    CertNameToStr(X509_ASN_ENCODING,
                  &pCertContext->pCertInfo->Issuer,
                  CERT_SIMPLE_NAME_STR | CERT_NAME_STR_CRLF_FLAG | CERT_NAME_STR_NO_PLUS_FLAG,
                  pwszName,
                  cchName);

    for (i = 0; i < cIssuerList; i++)
    {
        if (wcsncmp(pwszName, rgpwszIssuerList[i], cchName) == 0)
        {
            *pfMatch = TRUE;
            break;
        }
    }

Exit:

    if (pwszName != NULL)
    {
        delete[] pwszName;
    }

    return S_OK;
}

STDMETHODIMP
CXMLHttpRequest3Callback::SelectCert(
    _In_ DWORD cIssuerList,
    _Frees_ptr_ const WCHAR **rgpwszIssuerList,
    _Inout_ DWORD *pcbCertHash,
    _Inout_updates_(*pcbCertHash) BYTE *pbCertHash
)
/*++

Routine Description:

    Display a certificate picker UI for the user to select a client certificate
    to use for the current retry connection using the issuer list as a display
    filter, and return the thumbprint of the selected certificate.

Arguments:

    pcIssuerList - The count of issuers in the issuer list.

    rgpwszIssuerList - The issuer list used to filter the certificates displayed.


Return Value:

    HRESULT.

--*/
{
    HRESULT hr = S_OK;
    PCCERT_CONTEXT pCertContext = NULL;
    PCCERT_CONTEXT pSelectedContext = NULL;
    HCERTSTORE hMyCertStore = NULL;
    HCERTSTORE hMemStore = NULL;
    BOOL fMatch = FALSE;

    hMyCertStore = CertOpenStore(CERT_STORE_PROV_SYSTEM_W,
                                 X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                                 NULL,
                                 CERT_SYSTEM_STORE_CURRENT_USER,
                                 L"MY");

    if (hMyCertStore == NULL)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }

    hMemStore = CertOpenStore(CERT_STORE_PROV_MEMORY,
                              X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                              NULL,
                              CERT_STORE_CREATE_NEW_FLAG,
                              NULL);

    if (hMemStore == NULL)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }

    for (pCertContext = CertEnumCertificatesInStore(hMyCertStore, pCertContext);
         pCertContext != NULL;
         pCertContext = CertEnumCertificatesInStore(hMyCertStore, pCertContext))
    {
        hr = CompareIssuer((VOID*) pCertContext, cIssuerList, rgpwszIssuerList, &fMatch);

        if (FAILED(hr) || !fMatch)
        {
            continue;
        }

        if (!CertAddCertificateContextToStore(hMemStore,
                                              pCertContext,
                                              CERT_STORE_ADD_ALWAYS,
                                              NULL))
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto Exit;
        }
    }

    pSelectedContext = CryptUIDlgSelectCertificateFromStore(hMemStore,
                                                            NULL,
                                                            NULL,
                                                            NULL,
                                                            0,
                                                            0,
                                                            NULL);

    if (pSelectedContext == NULL)
    {
        hr = E_POINTER;
        goto Exit;
    }

    if (!CertGetCertificateContextProperty(pSelectedContext,
                                           CERT_HASH_PROP_ID,
                                           pbCertHash,
                                           pcbCertHash))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }

Exit:

    if (pSelectedContext != NULL)
    {
        CertFreeCertificateContext(pSelectedContext);
    }

    if (pCertContext != NULL)
    {
        CertFreeCertificateContext(pCertContext);
    }

    if (hMemStore != NULL)
    {
        CertCloseStore(hMemStore, 0);
    }

    if (hMyCertStore != NULL)
    {
        CertCloseStore(hMyCertStore, 0);
    }

    FreeIssuerList(cIssuerList, rgpwszIssuerList);

    return hr;
}

STDMETHODIMP
CXMLHttpRequest3Callback::GetCertResult(
    _Out_ BOOL *pfRetry,
    _Out_ DWORD *pdwCertIgnoreFlags,
    _Out_ DWORD *pcIssuerList,
    _Out_ const WCHAR ***prgpwszIssuerList
)
/*++

Routine Description:

   Retrieve certificate state for the retry connection.

Arguments:

    pfRetry - A pointer to the retry connection state.

    pdwCertIgnoreFlags - A pointer to the certificate ignore flags.

    pcIssuerList - A pointer to the count of issuers in the issuer list.

    prgpwszIssuerList - A pointer to the array of issuer strings.

Return Value:

    HRESULT.

--*/
{
    HRESULT hr = S_OK;

    *pfRetry = m_fRetry;
    *pdwCertIgnoreFlags = m_dwCertIgnoreFlags;

    if (m_cIssuerList != 0 && m_rgpwszIssuerList != NULL)
    {
        *pcIssuerList = m_cIssuerList;
        hr = DuplicateIssuerList(m_cIssuerList, m_rgpwszIssuerList, prgpwszIssuerList);
    }
    else
    {
        *pcIssuerList = 0;
        *prgpwszIssuerList = NULL;
    }

    return hr;
}

STDMETHODIMP
CXMLHttpRequest3Callback::WaitForComplete(
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
