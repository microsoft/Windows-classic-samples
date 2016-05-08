// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
//----------------------------------------------------------------------------

#pragma once

// ResourceFontCollectionLoader
//
//      Implements the IDWriteFontCollectionLoader interface for collections
//      of fonts embedded in the application as resources. The font collection
//      key is an array of resource IDs.
//
class ResourceFontCollectionLoader : public IDWriteFontCollectionLoader
{
public:
    ResourceFontCollectionLoader() : refCount_(0)
    {
    }

    // IUnknown methods
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void** ppvObject);
    virtual ULONG STDMETHODCALLTYPE AddRef();
    virtual ULONG STDMETHODCALLTYPE Release();

    // IDWriteFontCollectionLoader methods
    virtual HRESULT STDMETHODCALLTYPE CreateEnumeratorFromKey(
        IDWriteFactory* factory,
        void const* collectionKey,                      // [collectionKeySize] in bytes
        UINT32 collectionKeySize,
        OUT IDWriteFontFileEnumerator** fontFileEnumerator
        );

    // Gets the singleton loader instance.
    static IDWriteFontCollectionLoader* GetLoader()
    {
        return instance_;
    }

    static bool IsLoaderInitialized()
    {
        return instance_ != NULL;
    }

private:
    ULONG refCount_;

    static IDWriteFontCollectionLoader* instance_;
};
