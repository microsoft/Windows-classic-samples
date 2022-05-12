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

#include "pch.h"
#include <assert.h>
#include <avrt.h>
#include "WASAPIRenderer.h"

//
//  A simple WASAPI Render client.
//

void CWASAPIRenderer::SetUp(IMMDevice* Endpoint, bool EnableStreamSwitch, ERole EndpointRole, bool EnableAudioViewManagerService)
{
    _endpoint = Endpoint;
    _enableStreamSwitch = EnableStreamSwitch;
    _endpointRole = EndpointRole;
    _enableAudioViewManagerService = EnableAudioViewManagerService;
}

//
//  Empty destructor - everything should be released in the Shutdown() call.
//
CWASAPIRenderer::~CWASAPIRenderer(void)
{
}

//
//  Initialize WASAPI in event driven mode.
//
HRESULT CWASAPIRenderer::InitializeAudioEngine()
{
    RETURN_IF_FAILED(_audioClient->Initialize(AUDCLNT_SHAREMODE_SHARED,
        AUDCLNT_STREAMFLAGS_EVENTCALLBACK | AUDCLNT_STREAMFLAGS_NOPERSIST,
        _engineLatencyInMS * (long long)10000,
        0,
        _mixFormat.get(),
        NULL));

    //
    //  Retrieve the buffer size for the audio client.
    //
    RETURN_IF_FAILED(_audioClient->GetBufferSize(&_bufferSize));

    RETURN_IF_FAILED(_audioClient->SetEventHandle(_audioSamplesReadyEvent.get()));

    RETURN_IF_FAILED(_audioClient->GetService(IID_PPV_ARGS(&_renderClient)));

    if (_enableAudioViewManagerService)
    {
        auto const hr = [&]
        {
            wil::com_ptr_nothrow<IAudioViewManagerService> audioViewManagerService;
            RETURN_IF_FAILED(_audioClient->GetService(IID_PPV_ARGS(&audioViewManagerService)));
            // Pass the window that this audio stream is associated with.
            // This is used by the system for purposes such as rendering spatial audio
            // in Mixed Reality scenarios.
            RETURN_IF_FAILED(audioViewManagerService->SetAudioStreamWindow(GetConsoleWindow()));

            return S_OK;
        }();
        if (SUCCEEDED(hr))
        {
            printf("Audio stream has been associated with the console window\n");
        }
        else
        {
            printf("Unable to associate the audio stream with a window\n");
        }
    }

    return S_OK;
}

//
//  The Event Driven renderer will be woken up every defaultDevicePeriod hundred-nano-seconds.
//  Convert that time into a number of frames.
//
UINT32 CWASAPIRenderer::BufferSizePerPeriod()
{
    REFERENCE_TIME defaultDevicePeriod, minimumDevicePeriod;
    if (FAILED(_audioClient->GetDevicePeriod(&defaultDevicePeriod, &minimumDevicePeriod)))
    {
        printf("Unable to retrieve device period\n");
        return 0;
    }
    double devicePeriodInSeconds = defaultDevicePeriod / (10000.0 * 1000.0);
    return static_cast<UINT32>(_mixFormat->nSamplesPerSec * devicePeriodInSeconds + 0.5);
}

//
//  Retrieve the format we'll use to render samples.
//
//  We use the Mix format since we're rendering in shared mode.
//
HRESULT CWASAPIRenderer::LoadFormat()
{
    RETURN_IF_FAILED(_audioClient->GetMixFormat(wil::out_param(_mixFormat)));

    _frameSize = _mixFormat->nBlockAlign;
    RETURN_IF_FAILED(CalculateMixFormatType());

    return S_OK;
}

//
//  Crack open the mix format and determine what kind of samples are being rendered.
//
HRESULT CWASAPIRenderer::CalculateMixFormatType()
{
    if (_mixFormat->wFormatTag == WAVE_FORMAT_PCM ||
        _mixFormat->wFormatTag == WAVE_FORMAT_EXTENSIBLE &&
        reinterpret_cast<WAVEFORMATEXTENSIBLE*>(_mixFormat.get())->SubFormat == KSDATAFORMAT_SUBTYPE_PCM)
    {
        if (_mixFormat->wBitsPerSample == 16)
        {
            _renderSampleType = RenderSampleType::Pcm16Bit;
        }
        else
        {
            printf("Unknown PCM integer sample type\n");
            return E_UNEXPECTED;
        }
    }
    else if (_mixFormat->wFormatTag == WAVE_FORMAT_IEEE_FLOAT ||
        (_mixFormat->wFormatTag == WAVE_FORMAT_EXTENSIBLE &&
            reinterpret_cast<WAVEFORMATEXTENSIBLE*>(_mixFormat.get())->SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT))
    {
        _renderSampleType = RenderSampleType::Float;
    }
    else
    {
        printf("unrecognized device format.\n");
        return E_UNEXPECTED;
    }
    return S_OK;
}
//
//  Initialize the renderer.
//
HRESULT CWASAPIRenderer::Initialize(UINT32 EngineLatency)
{
    if (EngineLatency < 30)
    {
        printf("Engine latency in shared mode event driven cannot be less than 30ms\n");
        return E_UNEXPECTED;
    }

    //
    //  Create our shutdown and samples ready events- we want auto reset events that start in the not-signaled state.
    //
    RETURN_IF_FAILED(_shutdownEvent.create());

    RETURN_IF_FAILED(_audioSamplesReadyEvent.create());

    //
    //  Create our stream switch event- we want auto reset events that start in the not-signaled state.
    //  Note that we create this event even if we're not going to stream switch - that's because the event is used
    //  in the main loop of the renderer and thus it has to be set.
    //
    RETURN_IF_FAILED(_streamSwitchEvent.create());

    //
    //  Now activate an IAudioClient object on our preferred endpoint and retrieve the mix format for that endpoint.
    //
    RETURN_IF_FAILED(_endpoint->Activate(__uuidof(IAudioClient), CLSCTX_INPROC_SERVER, NULL, reinterpret_cast<void**>(&_audioClient)));

    RETURN_IF_FAILED(CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&_deviceEnumerator)));

    //
    // Load the MixFormat.  This may differ depending on the shared mode used
    //
    RETURN_IF_FAILED(LoadFormat());

    //
    //  Remember our configured latency in case we'll need it for a stream switch later.
    //
    _engineLatencyInMS = EngineLatency;
    RETURN_IF_FAILED(InitializeAudioEngine());

    if (_enableStreamSwitch)
    {
        RETURN_IF_FAILED(InitializeStreamSwitch());
    }

    return S_OK;
}

//
//  Shut down the render code and free all the resources.
//
void CWASAPIRenderer::Shutdown()
{
    if (_renderThread)
    {
        SetEvent(_shutdownEvent.get());
        WaitForSingleObject(_renderThread.get(), INFINITE);
        _renderThread.reset();
    }

    if (_enableStreamSwitch)
    {
        TerminateStreamSwitch();
    }
}


//
//  Start rendering - Create the render thread and start rendering the buffer.
//
HRESULT CWASAPIRenderer::Start(std::forward_list<RenderBuffer>&& RenderBufferQueue)
{
    _renderBufferQueue = std::move(RenderBufferQueue);

    //
    //  We want to pre-roll the first buffer's worth of data into the pipeline.  That way the audio engine won't glitch on startup.  
    //
    {
        BYTE* pData;

        if (_renderBufferQueue.empty())
        {
            RETURN_IF_FAILED(_renderClient->GetBuffer(_bufferSize, &pData));
            RETURN_IF_FAILED(_renderClient->ReleaseBuffer(_bufferSize, AUDCLNT_BUFFERFLAGS_SILENT));
        }
        else
        {
            //
            //  Remove the buffer from the queue.
            //
            std::forward_list<RenderBuffer> head;
            head.splice_after(head.before_begin(), _renderBufferQueue, _renderBufferQueue.before_begin());
            RenderBuffer& renderBuffer = head.front();
            DWORD bufferLengthInFrames = renderBuffer._bufferLength / _frameSize;

            RETURN_IF_FAILED(_renderClient->GetBuffer(bufferLengthInFrames, &pData));

            CopyMemory(pData, renderBuffer._buffer.get(), renderBuffer._bufferLength);
            RETURN_IF_FAILED(_renderClient->ReleaseBuffer(bufferLengthInFrames, 0));
        }
    }

    //
    //  Now create the thread which is going to drive the renderer.
    //
    _renderThread.reset(CreateThread(NULL, 0, WASAPIRenderThread, this, 0, NULL));
    RETURN_LAST_ERROR_IF_NULL(_renderThread);

    //
    //  We're ready to go, start rendering!
    //
    RETURN_IF_FAILED(_audioClient->Start());

    return S_OK;
}

//
//  Stop the renderer.
//
void CWASAPIRenderer::Stop()
{
    //
    //  Tell the render thread to shut down, wait for the thread to complete then clean up all the stuff we 
    //  allocated in Start().
    //
    if (_shutdownEvent)
    {
        SetEvent(_shutdownEvent.get());
    }

    if (FAILED(_audioClient->Stop()))
    {
        printf("Unable to stop audio client\n");
    }

    if (_renderThread)
    {
        WaitForSingleObject(_renderThread.get(), INFINITE);
        _renderThread.reset();
    }

    //
    //  Drain the buffers in the render buffer queue.
    //
    _renderBufferQueue.clear();
}


//
//  Render thread - processes samples from the audio engine
//
DWORD CWASAPIRenderer::WASAPIRenderThread(LPVOID Context)
{
    CWASAPIRenderer* renderer = static_cast<CWASAPIRenderer*>(Context);
    return renderer->DoRenderThread();
}

DWORD CWASAPIRenderer::DoRenderThread()
{
    bool stillPlaying = true;
    wil::unique_couninitialize_call uninitialize(false);

    HANDLE waitArray[3] = { _shutdownEvent.get(), _streamSwitchEvent.get(), _audioSamplesReadyEvent.get() };

    if (SUCCEEDED(CoInitializeEx(NULL, COINIT_MULTITHREADED)))
    {
        uninitialize.activate();
    }
    else
    {
        printf("Unable to initialize COM in render thread\n");
    }
    
    while (stillPlaying)
    {
        DWORD waitResult = WaitForMultipleObjects(3, waitArray, FALSE, INFINITE);
        switch (waitResult)
        {
        case WAIT_OBJECT_0 + 0:     // _shutdownEvent
            stillPlaying = false;       // We're done, exit the loop.
            break;
        case WAIT_OBJECT_0 + 1:     // _streamSwitchEvent
            //
            //  We've received a stream switch request.
            //
            //  We need to stop the renderer, tear down the _audioClient and _renderClient objects and re-create them on the new.
            //  endpoint if possible.  If this fails, abort the thread.
            //
            stillPlaying = SUCCEEDED(HandleStreamSwitchEvent());
            break;
        case WAIT_OBJECT_0 + 2:     // _audioSamplesReadyEvent
            stillPlaying = SUCCEEDED(ProduceAudioFrames());
            break;
        }
    }

    return 0;
}

HRESULT CWASAPIRenderer::ProduceAudioFrames()
{
    //
    //  We need to provide the next buffer of samples to the audio renderer.
    //

    UINT32 padding;
    //
    //  We want to find out how much of the buffer *isn't* available (is padding).
    //
    RETURN_IF_FAILED(_audioClient->GetCurrentPadding(&padding));

    //
    //  Calculate the number of frames available.  We'll render
    //  that many frames or the number of frames left in the buffer, whichever is smaller.
    //
    UINT32 framesAvailable = _bufferSize - padding;

    //
    // Stop if we have nothing more to render.
    //
    if (_renderBufferQueue.empty())
    {
        return HRESULT_FROM_WIN32(ERROR_HANDLE_EOF);
    }

    //
    //  If the buffer at the head of the render buffer queue does not fit in the frames available,
    //  then skip this pass. We will have more room on the next pass.
    //
    if (_renderBufferQueue.front()._bufferLength > (framesAvailable * _frameSize))
    {
        return S_OK;
    }

    //
    //  Remove the first buffer from the head of the queue.
    //
    std::forward_list<RenderBuffer> head;
    head.splice_after(head.before_begin(), _renderBufferQueue, _renderBufferQueue.before_begin());
    RenderBuffer& renderBuffer = head.front();

    //
    //  Copy data from the render buffer to the output buffer and bump our render pointer.
    //
    UINT32 framesToWrite = renderBuffer._bufferLength / _frameSize;
    BYTE* pData;
    RETURN_IF_FAILED(_renderClient->GetBuffer(framesToWrite, &pData));
    CopyMemory(pData, renderBuffer._buffer.get(), framesToWrite * _frameSize);
    RETURN_IF_FAILED(_renderClient->ReleaseBuffer(framesToWrite, 0));

    return S_OK;
}

HRESULT CWASAPIRenderer::InitializeStreamSwitch()
{
    RETURN_IF_FAILED(_audioClient->GetService(IID_PPV_ARGS(&_audioSessionControl)));

    //
    //  Create the stream switch complete event- we want a manual reset event that starts in the not-signaled state.
    //
    RETURN_IF_FAILED(_streamSwitchCompleteEvent.create());

    //
    //  Register for session and endpoint change notifications.  
    //
    //  A stream switch is initiated when we receive a session disconnect notification or we receive a default device changed notification.
    //
    RETURN_IF_FAILED(_audioSessionControl->RegisterAudioSessionNotification(this));

    RETURN_IF_FAILED(_deviceEnumerator->RegisterEndpointNotificationCallback(this));

    return S_OK;
}

void CWASAPIRenderer::TerminateStreamSwitch()
{
    if (_audioSessionControl)
    {
        // Unregistration can fail if InitializeStreamSwitch failed to register.
        _audioSessionControl->UnregisterAudioSessionNotification(this);
    }

    if (_deviceEnumerator)
    {
        // Unregistration can fail if InitializeStreamSwitch failed to register.
        _deviceEnumerator->UnregisterEndpointNotificationCallback(this);
    }

    _streamSwitchCompleteEvent.reset();
}

//
//  Handle the stream switch.
//
//  When a stream switch happens, we want to do several things in turn:
//
//  1) Stop the current renderer.
//  2) Release any resources we have allocated (the _audioClient, _audioSessionControl (after unregistering for notifications) and
//        _renderClient).
//  3) Wait until the default device has changed (or 500ms has elapsed).  If we time out, we need to abort because the stream switch can't happen.
//  4) Retrieve the new default endpoint for our role.
//  5) Re-instantiate the audio client on that new endpoint.  
//  6) Retrieve the mix format for the new endpoint.  If the mix format doesn't match the old endpoint's mix format, we need to abort because the stream
//      switch can't happen.
//  7) Re-initialize the _audioClient.
//  8) Re-register for session disconnect notifications and reset the stream switch complete event.
//
HRESULT CWASAPIRenderer::HandleStreamSwitchEvent()
{
    DWORD waitResult;

    assert(_inStreamSwitch);
    _inStreamSwitch = false;
    //
    //  Step 1.  Stop rendering.
    //
    RETURN_IF_FAILED(_audioClient->Stop());

    //
    //  Step 2.  Release our resources.  Note that we don't release the mix format, we need it for step 6.
    //
    RETURN_IF_FAILED(_audioSessionControl->UnregisterAudioSessionNotification(this));

    _audioSessionControl.reset();
    _renderClient.reset();
    _audioClient.reset();
    _endpoint.reset();

    //
    //  Step 3.  Wait for the default device to change.
    //
    //  There is a race between the session disconnect arriving and the new default device 
    //  arriving (if applicable).  Wait the shorter of 500 milliseconds or the arrival of the 
    //  new default device, then attempt to switch to the default device.  In the case of a 
    //  format change (i.e. the default device does not change), we artificially generate  a
    //  new default device notification so the code will not needlessly wait 500ms before 
    //  re-opening on the new format.  (However, note below in step 6 that in this SDK 
    //  sample, we are unlikely to actually successfully absorb a format change, but a 
    //  real audio application implementing stream switching would re-format their 
    //  pipeline to deliver the new format).  
    //
    waitResult = WaitForSingleObject(_streamSwitchCompleteEvent.get(), 500);
    if (waitResult == WAIT_TIMEOUT)
    {
        printf("Stream switch timeout - aborting...\n");
        return E_UNEXPECTED;
    }

    //
    //  Step 4.  If we can't get the new endpoint, we need to abort the stream switch.  If there IS a new device,
    //          we should be able to retrieve it.
    //
    RETURN_IF_FAILED(_deviceEnumerator->GetDefaultAudioEndpoint(eRender, _endpointRole, &_endpoint));

    //
    //  Step 5 - Re-instantiate the audio client on the new endpoint.
    //
    RETURN_IF_FAILED(_endpoint->Activate(__uuidof(IAudioClient), CLSCTX_INPROC_SERVER, NULL, reinterpret_cast<void**>(&_audioClient)));

    //
    //  Step 6 - Retrieve the new mix format.
    //
    wil::unique_cotaskmem_ptr<WAVEFORMATEX> wfxNew;
    RETURN_IF_FAILED(_audioClient->GetMixFormat(wil::out_param(wfxNew)));

    //
    //  Note that this is an intentionally naive comparison.  A more sophisticated comparison would
    //  compare the sample rate, channel count and format and apply the appropriate conversions into the render pipeline.
    //
    if (memcmp(_mixFormat.get(), wfxNew.get(), sizeof(WAVEFORMATEX) + wfxNew->cbSize) != 0)
    {
        printf("New mix format doesn't match old mix format.  Aborting.\n");
        return E_UNEXPECTED;
    }

    //
    //  Step 7:  Re-initialize the audio client.
    //
    RETURN_IF_FAILED(InitializeAudioEngine());

    //
    //  Step 8: Re-register for session disconnect notifications.
    //
    RETURN_IF_FAILED(_audioClient->GetService(IID_PPV_ARGS(&_audioSessionControl)));
    RETURN_IF_FAILED(_audioSessionControl->RegisterAudioSessionNotification(this));

    //
    //  Reset the stream switch complete event because it's a manual reset event.
    //
    ResetEvent(_streamSwitchCompleteEvent.get());
    //
    //  And we're done.  Start rendering again.
    //
    RETURN_IF_FAILED(_audioClient->Start());

    return S_OK;
}

//
//  Called when an audio session is disconnected.  
//
//  When a session is disconnected because of a device removal or format change event, we just want 
//  to let the render thread know that the session's gone away
//
HRESULT CWASAPIRenderer::OnSessionDisconnected(AudioSessionDisconnectReason DisconnectReason)
{
    if (DisconnectReason == DisconnectReasonDeviceRemoval)
    {
        //
        //  The stream was disconnected because the device we're rendering to was removed.
        //
        //  We want to reset the stream switch complete event (so we'll block when the HandleStreamSwitchEvent function
        //  waits until the default device changed event occurs).
        //
        //  Note that we _don't_ set the _streamSwitchCompleteEvent - that will be set when the OnDefaultDeviceChanged event occurs.
        //
        _inStreamSwitch = true;
        SetEvent(_streamSwitchEvent.get());
    }
    if (DisconnectReason == DisconnectReasonFormatChanged)
    {
        //
        //  The stream was disconnected because the format changed on our render device.
        //
        //  We want to flag that we're in a stream switch and then set the stream switch event (which breaks out of the renderer).  We also
        //  want to set the _streamSwitchCompleteEvent because we're not going to see a default device changed event after this.
        //
        _inStreamSwitch = true;
        SetEvent(_streamSwitchEvent.get());
        SetEvent(_streamSwitchCompleteEvent.get());
    }
    return S_OK;
}
//
//  Called when the default render device changed.  We just want to set an event which lets the stream switch logic know that it's ok to 
//  continue with the stream switch.
//
HRESULT CWASAPIRenderer::OnDefaultDeviceChanged(EDataFlow Flow, ERole Role, LPCWSTR /*NewDefaultDeviceId*/)
{
    if (Flow == eRender && Role == _endpointRole)
    {
        //
        //  The default render device for our configured role was changed.
        //
        //  If we're not in a stream switch already, we want to initiate a stream switch event.  
        //  We also we want to set the stream switch complete event.  That will signal the render thread that it's ok to re-initialize the
        //  audio renderer.
        //
        if (!_inStreamSwitch)
        {
            _inStreamSwitch = true;
            SetEvent(_streamSwitchEvent.get());
        }
        SetEvent(_streamSwitchCompleteEvent.get());
    }
    return S_OK;
}
