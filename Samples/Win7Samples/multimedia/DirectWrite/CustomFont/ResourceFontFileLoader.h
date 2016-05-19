// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
//----------------------------------------------------------------------------

#pragma once

// ResourceFontFileLoader
//
//      Implements the IDWriteFontFileLoader interface for fonts
//      embedded as a resources in the application. The font file key is
//      a UINT resource identifier.
//
class ResourceFontFileLoader : public IDWriteFontFileLoader
{
public:
    ResourceFontFileLoader() : refCount_(0)
    {
    }

    // IUnknown methods
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void** ppvObject);
    virtual ULONG STDMETHODCALLTYPE AddRef();
    virtual ULONG STDMETHODCALLTYPE Release();

    // IDWriteFontFileLoader methods
    virtual HRESULT STDMETHODCALLTYPE CreateStreamFromKey(
        void const* fontFileReferenceKey,       // [fontFileReferenceKeySize] in bytes
        UINT32 fontFileReferenceKeySize,
        OUT IDWriteFontFileStream** fontFileStream
        );

    // Gets the singleton loader instance.
    static IDWriteFontFileLoader* GetLoader()
    {
        return instance_;
    }

    static bool IsLoaderInitialized()
    {
        return instance_ != NULL;
    }

private:
    ULONG refCount_;

    static IDWriteFontFileLoader* instance_;
};
