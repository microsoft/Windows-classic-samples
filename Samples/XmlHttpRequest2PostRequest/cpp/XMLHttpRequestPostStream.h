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

class CXMLHttpRequestPostStream : 
    public Microsoft::WRL::RuntimeClass<RuntimeClassFlags<ClassicCom>, ISequentialStream>
{
private:

    //
    // File handle.
    //

    HANDLE m_hFile;

    CXMLHttpRequestPostStream();

    ~CXMLHttpRequestPostStream();
    
    friend Microsoft::WRL::ComPtr<CXMLHttpRequestPostStream> Microsoft::WRL::Details::Make<CXMLHttpRequestPostStream>();

public:

    STDMETHODIMP
    Open(
        _In_opt_ PCWSTR pcwszFileName
    );

    STDMETHODIMP
    Read(
        _Out_writes_bytes_to_(cb, *pcbRead)  void *pv,
        ULONG cb,
        _Out_opt_  ULONG *pcbRead
    );

    STDMETHODIMP
    Write(
        _In_reads_bytes_(cb)  const void *pv,
        ULONG cb,
        _Out_opt_  ULONG *pcbWritten
    );

    STDMETHODIMP
    GetSize(
        _Out_ ULONGLONG *pullSize
    );
};
