// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
//----------------------------------------------------------------------------

#pragma once

// ResourceFontFileEnumerator
//
//      Implements the IDWriteFontFileEnumerator interface for a collection
//      of fonts embedded in the application as resources. The font collection
//      key is an array of resource IDs.
//
class ResourceFontFileEnumerator : public IDWriteFontFileEnumerator
{
public:
    ResourceFontFileEnumerator(
        IDWriteFactory* factory
        );

    HRESULT Initialize(
        UINT const* resourceIDs,    // [resourceCount]
        UINT32 resourceCount
        );

    ~ResourceFontFileEnumerator()
    {
        SafeRelease(&currentFile_);
        SafeRelease(&factory_);
    }

    // IUnknown methods
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void** ppvObject);
    virtual ULONG STDMETHODCALLTYPE AddRef();
    virtual ULONG STDMETHODCALLTYPE Release();

    // IDWriteFontFileEnumerator methods
    virtual HRESULT STDMETHODCALLTYPE MoveNext(OUT BOOL* hasCurrentFile);
    virtual HRESULT STDMETHODCALLTYPE GetCurrentFontFile(OUT IDWriteFontFile** fontFile);

private:
    ULONG refCount_;

    IDWriteFactory* factory_;
    IDWriteFontFile* currentFile_;
    std::vector<UINT> resourceIDs_;
    size_t nextIndex_;
};
