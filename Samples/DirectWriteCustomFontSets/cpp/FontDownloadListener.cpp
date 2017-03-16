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
#include "DXHelper.h"
#include "FontDownloadListener.h"


namespace DWriteCustomFontSets
{

    FontDownloadListener::FontDownloadListener()
    {
        m_downloadCompletedEventHandle = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (m_downloadCompletedEventHandle == nullptr)
        {
            HRESULT hr = HRESULT_FROM_WIN32(GetLastError());
            throw DX::CreateDXException(hr);
        }
    }

    FontDownloadListener::~FontDownloadListener()
    {
        CloseHandle(m_downloadCompletedEventHandle);
    }


    void FontDownloadListener::DownloadCompleted(
        _In_ IDWriteFontDownloadQueue* downloadQueue,
        _In_opt_ IUnknown* context,
        HRESULT downloadResult
    )
    {
        m_downloadResult = downloadResult;
        SetEvent(m_downloadCompletedEventHandle);
    }


    HRESULT FontDownloadListener::QueryInterface(REFIID riid, _COM_Outptr_ void** obj)
    {
        *obj = nullptr;
        if (riid == __uuidof(IDWriteFontDownloadListener) || riid == __uuidof(IUnknown))
        {
            AddRef();
            *obj = this;
            return S_OK;
        }
        return E_NOINTERFACE;
    }


    ULONG FontDownloadListener::AddRef()
    {
        return InterlockedIncrement(&m_refCount);
    }


    ULONG FontDownloadListener::Release()
    {
        uint32_t newCount = InterlockedDecrement(&m_refCount);
        if (newCount == 0)
            delete this;
        return newCount;
    }

} // DWriteCustomFontSets namespace
