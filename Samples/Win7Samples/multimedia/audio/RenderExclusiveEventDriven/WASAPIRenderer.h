// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
#pragma once
#include <MMDeviceAPI.h>
#include <AudioClient.h>
#include <AudioPolicy.h>

struct RenderBuffer
{
    RenderBuffer *  _Next;
    UINT32          _BufferLength;
    BYTE *          _Buffer;

    RenderBuffer() :
        _Next(NULL),
        _BufferLength(0),
        _Buffer(NULL)
    {
    }

    ~RenderBuffer()
    {
        delete [] _Buffer;
    }
};

class CWASAPIRenderer : public IUnknown
{
public:
    //  Public interface to CWASAPIRenderer.
    enum RenderSampleType
    {
        SampleTypeFloat,
        SampleType16BitPCM,
    };

    CWASAPIRenderer(IMMDevice *Endpoint);
    bool Initialize(UINT32 EngineLatency);
    void Shutdown();
    bool Start(RenderBuffer *RenderBufferQueue);
    void Stop();
    WORD ChannelCount() { return _MixFormat->nChannels; }
    UINT32 SamplesPerSecond() { return _MixFormat->nSamplesPerSec; }
    UINT32 BytesPerSample() { return _MixFormat->wBitsPerSample / 8; }
    RenderSampleType SampleType() { return _RenderSampleType; }
    UINT32 FrameSize() { return _FrameSize; }
    UINT32 BufferSize() { return _BufferSize; }
    UINT32 BufferSizePerPeriod() { return _BufferSizePerPeriod; }
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

private:
    ~CWASAPIRenderer(void);
    LONG    _RefCount;
    //
    //  Core Audio Rendering member variables.
    //
    IMMDevice * _Endpoint;
    IAudioClient *_AudioClient;
    IAudioRenderClient *_RenderClient;

    HANDLE      _RenderThread;
    HANDLE      _ShutdownEvent;
    HANDLE      _AudioSamplesReadyEvent;
    WAVEFORMATEX *_MixFormat;
    UINT32      _FrameSize;
    RenderSampleType _RenderSampleType;
    UINT32      _BufferSize;
    UINT32      _BufferSizePerPeriod;
    LONG        _EngineLatencyInMS;

    //
    //  Render buffer management.
    //
    RenderBuffer *_RenderBufferQueue;

    static DWORD __stdcall WASAPIRenderThread(LPVOID Context);
    DWORD CWASAPIRenderer::DoRenderThread();

    //
    //  IUnknown
    //
    STDMETHOD(QueryInterface)(REFIID iid, void **pvObject);

    //
    //  Utility functions.
    //
    bool CalculateMixFormatType();
    bool InitializeAudioEngine();
    bool LoadFormat();
};
