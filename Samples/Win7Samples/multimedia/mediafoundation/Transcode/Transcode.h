//////////////////////////////////////////////////////////////////////////
//
// Transcode.h
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//
// This sample demonstrates how to perform simple transcoding
// to WMA or WMV.
//
////////////////////////////////////////////////////////////////////////// 

#pragma once

#define WINVER _WIN32_WINNT_WIN7

#include <stdio.h>
#include <assert.h>
#include <mfapi.h>
#include <mfidl.h>

template <class T> void SafeRelease(T **ppT)
{
    if (*ppT)
    {
        (*ppT)->Release();
        *ppT = NULL;
    }
}


class CTranscoder
{
public:
    CTranscoder();
    virtual ~CTranscoder();

    HRESULT OpenFile(const WCHAR *sURL);
    HRESULT ConfigureAudioOutput();
    HRESULT ConfigureVideoOutput();
    HRESULT ConfigureContainer();
    HRESULT EncodeToFile(const WCHAR *sURL);

private:

    HRESULT Shutdown();
    HRESULT Transcode();
    HRESULT Start();

    IMFMediaSession*        m_pSession;
    IMFMediaSource*         m_pSource;
    IMFTopology*            m_pTopology;
    IMFTranscodeProfile*    m_pProfile;
};