// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
//----------------------------------------------------------------------------

#include "Common.h"
#include "ResourceFontFileLoader.h"
#include "ResourceFontFileStream.h"

// Smart pointer to singleton instance of the font file loader.
IDWriteFontFileLoader* ResourceFontFileLoader::instance_(
    new(std::nothrow) ResourceFontFileLoader()
    );

// QueryInterface
HRESULT STDMETHODCALLTYPE ResourceFontFileLoader::QueryInterface(REFIID iid, void** ppvObject)
{
    if (iid == IID_IUnknown || iid == __uuidof(IDWriteFontFileLoader))
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

// AddRef
ULONG STDMETHODCALLTYPE ResourceFontFileLoader::AddRef()
{
    return InterlockedIncrement(&refCount_);
}

// Release
ULONG STDMETHODCALLTYPE ResourceFontFileLoader::Release()
{
    ULONG newCount = InterlockedDecrement(&refCount_);
    if (newCount == 0)
        delete this;

    return newCount;
}

// CreateStreamFromKey
//
//      Creates an IDWriteFontFileStream from a key that identifies the file. The
//      format and meaning of the key is entirely up to the loader implementation.
//      The only requirements imposed by DWrite are that a key must remain valid
//      for as long as the loader is registered. The same key must also uniquely
//      identify the same file, so for example you must not recycle keys so that
//      a key might represent different files at different times.
//
//      In this case the key is a UINT which identifies a font resources.
//
HRESULT STDMETHODCALLTYPE ResourceFontFileLoader::CreateStreamFromKey(
    void const* fontFileReferenceKey,       // [fontFileReferenceKeySize] in bytes
    UINT32 fontFileReferenceKeySize,
    OUT IDWriteFontFileStream** fontFileStream
    )
{
    *fontFileStream = NULL;

    // Make sure the key is the right size.
    if (fontFileReferenceKeySize != sizeof(UINT))
        return E_INVALIDARG;

    UINT resourceID = *static_cast<UINT const*>(fontFileReferenceKey);

    // Create the stream object.
    ResourceFontFileStream* stream = new(std::nothrow) ResourceFontFileStream(resourceID);
    if (stream == NULL)
        return E_OUTOFMEMORY;

    if (!stream->IsInitialized())
    {
        delete stream;
        return E_FAIL;
    }

    *fontFileStream = SafeAcquire(stream);

    return S_OK;
}

