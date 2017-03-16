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


#pragma once

#include "stdafx.h"



namespace DWriteCustomFontSets
{
    class FontDownloadListener : public IDWriteFontDownloadListener
    {
    public:
        FontDownloadListener();
        ~FontDownloadListener();

        HANDLE GetDownloadCompletedEventHandle() { return m_downloadCompletedEventHandle; };
        HRESULT GetDownloadResult() { return m_downloadResult; }

        // IDWriteFontDownloadListener members

        // Callback method -- will be called by DirectWrite after a call to IDWriteFontDownloadQueue::BeginDownload has completed.
        virtual void STDMETHODCALLTYPE DownloadCompleted(
            _In_ IDWriteFontDownloadQueue* downloadQueue,
            _In_opt_ IUnknown* context,
            HRESULT downloadResult
        ) override;


        // IDWriteFontDownloadListener inherits from IUnknown:

        HRESULT STDMETHODCALLTYPE QueryInterface(
            REFIID riid,
            _COM_Outptr_ void **obj
        ) override;
        ULONG STDMETHODCALLTYPE AddRef() override;
        ULONG STDMETHODCALLTYPE Release() override;

    private:
        HANDLE m_downloadCompletedEventHandle;
        HRESULT m_downloadResult = S_OK;
        uint32_t m_refCount = 0;
    };

} // DWriteCustomFontSets namespace
