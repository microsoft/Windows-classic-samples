// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
//----------------------------------------------------------------------------

#include "Common.h"
#include "ResourceFontFileEnumerator.h"
#include "ResourceFontFileLoader.h"

ResourceFontFileEnumerator::ResourceFontFileEnumerator(
    IDWriteFactory* factory
    ) : 
    refCount_(0), 
    factory_(SafeAcquire(factory)),
    currentFile_(),
    nextIndex_(0)
{
}

HRESULT ResourceFontFileEnumerator::Initialize(
    UINT const* resourceIDs,    // [resourceCount]
    UINT32 resourceCount
    )
{
    try
    {
        resourceIDs_.assign(resourceIDs, resourceIDs + resourceCount);
    }
    catch (...)
    {
        return ExceptionToHResult();
    }
    return S_OK;
}

HRESULT STDMETHODCALLTYPE ResourceFontFileEnumerator::QueryInterface(REFIID iid, OUT void** ppvObject)
{
    if (iid == IID_IUnknown || iid == __uuidof(IDWriteFontFileEnumerator))
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

ULONG STDMETHODCALLTYPE ResourceFontFileEnumerator::AddRef()
{
    return InterlockedIncrement(&refCount_);
}

ULONG STDMETHODCALLTYPE ResourceFontFileEnumerator::Release()
{
    ULONG newCount = InterlockedDecrement(&refCount_);
    if (newCount == 0)
        delete this;

    return newCount;
}

HRESULT STDMETHODCALLTYPE ResourceFontFileEnumerator::MoveNext(OUT BOOL* hasCurrentFile)
{
    HRESULT hr = S_OK;

    *hasCurrentFile = FALSE;
    SafeRelease(&currentFile_);

    if (nextIndex_ < resourceIDs_.size())
    {
        hr = factory_->CreateCustomFontFileReference(
                &resourceIDs_[nextIndex_],
                sizeof(UINT),
                ResourceFontFileLoader::GetLoader(),
                &currentFile_
                );

        if (SUCCEEDED(hr))
        {
            *hasCurrentFile = TRUE;

            ++nextIndex_;
        }
    }

    return hr;
}

HRESULT STDMETHODCALLTYPE ResourceFontFileEnumerator::GetCurrentFontFile(OUT IDWriteFontFile** fontFile)
{
    *fontFile = SafeAcquire(currentFile_);

    return (currentFile_ != NULL) ? S_OK : E_FAIL;
}
