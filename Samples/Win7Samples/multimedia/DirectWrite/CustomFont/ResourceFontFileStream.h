// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
//----------------------------------------------------------------------------

#pragma once

// ResourceFontFileStream
//
//      Implements the IDWriteFontFileStream interface in terms of a font
//      embedded as a resource in the application. The font file key is
//      a UINT resource identifier.
//
class ResourceFontFileStream : public IDWriteFontFileStream
{
public:
    explicit ResourceFontFileStream(UINT resourceID);

    // IUnknown methods
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void** ppvObject);
    virtual ULONG STDMETHODCALLTYPE AddRef();
    virtual ULONG STDMETHODCALLTYPE Release();

    // IDWriteFontFileStream methods
    virtual HRESULT STDMETHODCALLTYPE ReadFileFragment(
        void const** fragmentStart, // [fragmentSize] in bytes
        UINT64 fileOffset,
        UINT64 fragmentSize,
        OUT void** fragmentContext
        );

    virtual void STDMETHODCALLTYPE ReleaseFileFragment(
        void* fragmentContext
        );

    virtual HRESULT STDMETHODCALLTYPE GetFileSize(
        OUT UINT64* fileSize
        );

    virtual HRESULT STDMETHODCALLTYPE GetLastWriteTime(
        OUT UINT64* lastWriteTime
        );

    bool IsInitialized()
    {
        return resourcePtr_ != NULL;
    }

private:
    ULONG refCount_;
    void const* resourcePtr_;       // [resourceSize_] in bytes
    DWORD resourceSize_;

    static HMODULE const moduleHandle_;
    static HMODULE GetCurrentModule();
};
