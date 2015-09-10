//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once
#include <array>

class FontFileStream : public IDWriteFontFileStream
{
public:
    FontFileStream(_In_ std::vector<BYTE> *data);

    virtual HRESULT STDMETHODCALLTYPE QueryInterface(
        REFIID uuid,
        _Outptr_ void** object
        ) override;

    virtual ULONG STDMETHODCALLTYPE AddRef() override;

    virtual ULONG STDMETHODCALLTYPE Release() override;

    virtual HRESULT STDMETHODCALLTYPE ReadFileFragment(
        _Outptr_result_bytebuffer_(fragmentSize) void const** fragmentStart,
        UINT64 fileOffset,
        UINT64 fragmentSize,
        _Out_ void** fragmentContext
        ) override;

    virtual void STDMETHODCALLTYPE ReleaseFileFragment(
        _In_ void* fragmentContext
        ) override;

    virtual HRESULT STDMETHODCALLTYPE GetFileSize(
        _Out_ UINT64* fileSize
        ) override;

    virtual HRESULT STDMETHODCALLTYPE GetLastWriteTime(
        _Out_ UINT64* lastWriteTime
        ) override;

private:
    // Reference counter
    ULONG m_refCount;

    // Raw data of the file
    std::vector<BYTE> m_data;
};
