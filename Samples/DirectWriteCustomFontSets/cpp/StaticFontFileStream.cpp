//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************


#include "stdafx.h"
#include "StaticFontFileStream.h"


namespace DWriteCustomFontSets
{
    // IDWriteFontFileStream implementations

    HRESULT StaticFontFileStream::ReadFileFragment(_Outptr_result_bytebuffer_(fragmentSize) void const** fragmentStart, UINT64 fileOffset, UINT64 fragmentSize, _Out_ void** fragmentContext)
    {
        *fragmentContext = nullptr;

        // We need to do bounds checking.
        if (fileOffset <= m_dataSize && fragmentSize <= (m_dataSize - fileOffset) ) // subtraction in second comparison won't overflow if first comparison is true
        {
            *fragmentStart = m_data + fileOffset;
            return S_OK;
        }
        else
        {
            *fragmentStart = nullptr;
            return E_INVALIDARG;
        }
    }

    HRESULT StaticFontFileStream::GetFileSize(_Out_ UINT64* fileSize)
    {
        *fileSize = m_dataSize;
        return S_OK;
    }

    HRESULT StaticFontFileStream::GetLastWriteTime(_Out_ UINT64* lastWriteTime)
    {
        *lastWriteTime = 0;
        return E_NOTIMPL; // E_NOTIMPL by design -- see method documentation in dwrite.h.
    }



    // IUnknown method implementations

    ULONG StaticFontFileStream::AddRef()
    {
        return InterlockedIncrement(&m_refCount);
    }

    HRESULT StaticFontFileStream::QueryInterface(REFIID riid, void** object)
    {
        *object = nullptr;
        if (riid == __uuidof(IUnknown)
            || riid == __uuidof(IDWriteFontFileStream))
        {
            AddRef();
            *object = this;
            return S_OK;
        }
        return E_NOINTERFACE;
    }

    ULONG StaticFontFileStream::Release()
    {
        ULONG newCount = InterlockedDecrement(&m_refCount);
        if (newCount == 0)
            delete this;
        return newCount;
    }

} // namespace DWriteCustomFontSets