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

class CXMLHttpRequest3Callback :
    public Microsoft::WRL::RuntimeClass<RuntimeClassFlags<ClassicCom>, IXMLHTTPRequest3Callback>
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

    //
    // Retry with a new connection after hitting certificate issues using
    // certificate ignore flags and a copy of the issuer list.
    //

    BOOL m_fRetry;
    DWORD m_dwCertIgnoreFlags;
    DWORD m_cIssuerList;
    const WCHAR **m_rgpwszIssuerList;

    CXMLHttpRequest3Callback();

    ~CXMLHttpRequest3Callback();

    STDMETHODIMP
    RuntimeClassInitialize();

    friend HRESULT Microsoft::WRL::Details::MakeAndInitialize<CXMLHttpRequest3Callback,CXMLHttpRequest3Callback>(CXMLHttpRequest3Callback **);

    //
    // Certificates handling utilities.
    //

    STDMETHODIMP
    DuplicateIssuerList( 
        _In_ DWORD cIssuerList,
        _In_reads_(cIssuerList) const WCHAR **rgpwszIssuerList,
        _Out_ const WCHAR ***prgpwszDuplicateIssuerList
    );

    STDMETHODIMP
    FreeIssuerList(
        _In_ DWORD cIssuerList,
        _Frees_ptr_ const WCHAR **rgpwszIssuerList
    );

    STDMETHODIMP
    CompareIssuer(
        _In_ VOID *pvCertContext,
        _In_ DWORD cIssuerList,
        _In_reads_(cIssuerList) const WCHAR **rgpwszIssuerList,
        _Out_ BOOL *pfMatch
    );

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
    OnServerCertificateReceived( 
        __RPC__in_opt IXMLHTTPRequest3 *pXHR,
        DWORD dwCertErrors,
        DWORD cServerCertChain,
        __RPC__in_ecount_full_opt(cServerCertChain) const XHR_CERT *rgServerCertChain
    );

    STDMETHODIMP
    OnClientCertificateRequested( 
        __RPC__in_opt IXMLHTTPRequest3 *pXHR,
        DWORD cIssuerList,
        __RPC__in_ecount_full_opt(cIssuerList) const WCHAR **rgpwszIssuerList
    );

    STDMETHODIMP
    SelectCert(
        _In_ DWORD cIssuerList,
        _Frees_ptr_ const WCHAR **rgpwszIssuerList,
        _Inout_ DWORD *pcbCertHash,
        _Inout_updates_(*pcbCertHash) BYTE *pbCertHash
    );

    STDMETHODIMP
    GetCertResult(
        _Out_ BOOL *pfRetry,
        _Out_ DWORD *pdwCertIgnoreFlags,
        _Out_ DWORD *pcIssuerList,
        _Out_ const WCHAR ***prgpwszIssuerList
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
