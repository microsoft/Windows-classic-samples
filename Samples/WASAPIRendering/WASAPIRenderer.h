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
#include <MMDeviceAPI.h>
#include <AudioClient.h>
#include <AudioPolicy.h>
#include <wil/result.h>
#include <wil/com.h>
#include <wrl\implements.h>

using namespace Microsoft::WRL;

struct RenderBuffer
{
    UINT32 _bufferLength;
    std::unique_ptr<BYTE[]> _buffer;

    RenderBuffer(UINT32 length) :
        _bufferLength(length),
        _buffer(new BYTE[length])
    {
    }
};

class CWASAPIRenderer :
    public RuntimeClass< RuntimeClassFlags< ClassicCom >, IMMNotificationClient, IAudioSessionEvents >
{
public:
    //  Public interface to CWASAPIRenderer.
    enum RenderSampleType
    {
        Float,
        Pcm16Bit,
    };

    CWASAPIRenderer() = default;
    ~CWASAPIRenderer(void);
    void SetUp(IMMDevice* Endpoint, bool EnableStreamSwitch, ERole EndpointRole, bool EnableAudioViewManagerService);
    HRESULT Initialize(UINT32 EngineLatency);
    void Shutdown();
    HRESULT Start(std::forward_list<RenderBuffer>&& RenderBufferQueue);
    void Stop();
    WORD ChannelCount() { return _mixFormat->nChannels; }
    UINT32 SamplesPerSecond() { return _mixFormat->nSamplesPerSec; }
    UINT32 BytesPerSample() { return _mixFormat->wBitsPerSample / 8; }
    RenderSampleType SampleType() { return _renderSampleType; }
    UINT32 FrameSize() { return _frameSize; }
    UINT32 BufferSize() { return _bufferSize; }
    UINT32 BufferSizePerPeriod();

private:
    //
    //  Core Audio Rendering member variables.
    //
    wil::com_ptr_nothrow<IMMDevice>                _endpoint;
    wil::com_ptr_nothrow<IAudioClient>             _audioClient;
    wil::com_ptr_nothrow<IAudioRenderClient>       _renderClient;

    wil::unique_handle                      _renderThread;
    wil::unique_event_nothrow               _shutdownEvent;
    wil::unique_event_nothrow               _audioSamplesReadyEvent;
    wil::unique_cotaskmem_ptr<WAVEFORMATEX> _mixFormat;
    UINT32           _frameSize                     = 0;
    UINT32           _bufferSize                    = 0;
    LONG             _engineLatencyInMS             = 0;
    bool             _enableAudioViewManagerService = false;
    RenderSampleType _renderSampleType              = RenderSampleType::Pcm16Bit;

    //
    //  Render buffer management.
    //
    std::forward_list<RenderBuffer> _renderBufferQueue;

    static DWORD __stdcall WASAPIRenderThread(LPVOID Context);
    DWORD DoRenderThread();
    //
    //  Stream switch related members and methods.
    //
    bool                    _enableStreamSwitch = false;
    ERole                   _endpointRole       = eConsole;
    wil::unique_event_nothrow                  _streamSwitchEvent;          // Set when the current session is disconnected or the default device changes.
    wil::unique_event_nothrow                  _streamSwitchCompleteEvent;  // Set when the default device changed.
    wil::com_ptr_nothrow<IAudioSessionControl> _audioSessionControl;
    wil::com_ptr_nothrow<IMMDeviceEnumerator>  _deviceEnumerator;
    bool                    _inStreamSwitch = false;

    HRESULT InitializeStreamSwitch();
    void TerminateStreamSwitch();
    HRESULT HandleStreamSwitchEvent();
    HRESULT ProduceAudioFrames();

    STDMETHOD(OnDisplayNameChanged) (LPCWSTR /*NewDisplayName*/, LPCGUID /*EventContext*/) { return S_OK; };
    STDMETHOD(OnIconPathChanged) (LPCWSTR /*NewIconPath*/, LPCGUID /*EventContext*/) { return S_OK; };
    STDMETHOD(OnSimpleVolumeChanged) (float /*NewSimpleVolume*/, BOOL /*NewMute*/, LPCGUID /*EventContext*/) { return S_OK; }
    STDMETHOD(OnChannelVolumeChanged) (DWORD /*ChannelCount*/, float /*NewChannelVolumes*/[], DWORD /*ChangedChannel*/, LPCGUID /*EventContext*/) { return S_OK; };
    STDMETHOD(OnGroupingParamChanged) (LPCGUID /*NewGroupingParam*/, LPCGUID /*EventContext*/) { return S_OK; };
    STDMETHOD(OnStateChanged) (AudioSessionState /*NewState*/) { return S_OK; };
    STDMETHOD(OnSessionDisconnected) (AudioSessionDisconnectReason DisconnectReason);
    STDMETHOD(OnDeviceStateChanged) (LPCWSTR /*DeviceId*/, DWORD /*NewState*/) { return S_OK; }
    STDMETHOD(OnDeviceAdded) (LPCWSTR /*DeviceId*/) { return S_OK; };
    STDMETHOD(OnDeviceRemoved) (LPCWSTR /*DeviceId(*/) { return S_OK; };
    STDMETHOD(OnDefaultDeviceChanged) (EDataFlow Flow, ERole Role, LPCWSTR NewDefaultDeviceId);
    STDMETHOD(OnPropertyValueChanged) (LPCWSTR /*DeviceId*/, const PROPERTYKEY /*Key*/) { return S_OK; };

    //
    //  Utility functions.
    //
    HRESULT CalculateMixFormatType();
    HRESULT InitializeAudioEngine();
    HRESULT LoadFormat();
};
