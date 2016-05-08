// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
//----------------------------------------------------------------------------

#include "Common.h"
#include "ResourceFontCollectionLoader.h"
#include "ResourceFontFileEnumerator.h"

IDWriteFontCollectionLoader* ResourceFontCollectionLoader::instance_(
    new(std::nothrow) ResourceFontCollectionLoader()
    );

HRESULT STDMETHODCALLTYPE ResourceFontCollectionLoader::QueryInterface(REFIID iid, void** ppvObject)
{
    if (iid == IID_IUnknown || iid == __uuidof(IDWriteFontCollectionLoader))
    {
        *ppvObject = this;
        AddRef();
        return S_OK;
    }
    else
    {
        *ppvObject = NULL;
        return E_NOINTERFACE;
    }
}

ULONG STDMETHODCALLTYPE ResourceFontCollectionLoader::AddRef()
{
    return InterlockedIncrement(&refCount_);
}

ULONG STDMETHODCALLTYPE ResourceFontCollectionLoader::Release()
{
    ULONG newCount = InterlockedDecrement(&refCount_);
    if (newCount == 0)
        delete this;

    return newCount;
}

HRESULT STDMETHODCALLTYPE ResourceFontCollectionLoader::CreateEnumeratorFromKey(
    IDWriteFactory* factory,
    void const* collectionKey,                      // [collectionKeySize] in bytes
    UINT32 collectionKeySize,
    OUT IDWriteFontFileEnumerator** fontFileEnumerator
    )
{
    *fontFileEnumerator = NULL;

    HRESULT hr = S_OK;

    if (collectionKeySize % sizeof(UINT) != 0)
        return E_INVALIDARG;

    ResourceFontFileEnumerator* enumerator = new(std::nothrow) ResourceFontFileEnumerator(
        factory
        );
    if (enumerator == NULL)
        return E_OUTOFMEMORY;

    UINT const* resourceIDs    = static_cast<UINT const*>(collectionKey);
    UINT32 const resourceCount = collectionKeySize / sizeof(UINT);

    hr = enumerator->Initialize(
            resourceIDs, 
            resourceCount
            );

    if (FAILED(hr))
    {
        delete enumerator;
        return hr;
    }

    *fontFileEnumerator = SafeAcquire(enumerator);

    return hr;
}

