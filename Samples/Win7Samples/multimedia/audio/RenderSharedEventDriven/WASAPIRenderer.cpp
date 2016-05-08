// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
#include "StdAfx.h"
#include <assert.h>
#include <avrt.h>
#include "WASAPIRenderer.h"

//
//  A simple WASAPI Render client.
//

CWASAPIRenderer::CWASAPIRenderer(IMMDevice *Endpoint, bool EnableStreamSwitch, ERole EndpointRole) : 
    _RefCount(1),
    _Endpoint(Endpoint),
    _AudioClient(NULL),
    _RenderClient(NULL),
    _RenderThread(NULL),
    _ShutdownEvent(NULL),
    _MixFormat(NULL),
    _RenderBufferQueue(0),
    _AudioSamplesReadyEvent(NULL),
    _EnableStreamSwitch(EnableStreamSwitch),
    _EndpointRole(EndpointRole),
    _StreamSwitchEvent(NULL),
    _StreamSwitchCompleteEvent(NULL),
    _AudioSessionControl(NULL),
    _DeviceEnumerator(NULL),
    _InStreamSwitch(false)
{
    _Endpoint->AddRef();    // Since we're holding a copy of the endpoint, take a reference to it.  It'll be released in Shutdown();
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
bool CWASAPIRenderer::InitializeAudioEngine()
{
    HRESULT hr = _AudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, 
        AUDCLNT_STREAMFLAGS_EVENTCALLBACK | AUDCLNT_STREAMFLAGS_NOPERSIST, 
        _EngineLatencyInMS*10000, 
        0, 
        _MixFormat, 
        NULL);

    if (FAILED(hr))
    {
        printf("Unable to initialize audio client: %x.\n", hr);
        return false;
    }

    //
    //  Retrieve the buffer size for the audio client.
    //
    hr = _AudioClient->GetBufferSize(&_BufferSize);
    if(FAILED(hr))
    {
        printf("Unable to get audio client buffer: %x. \n", hr);
        return false;
    }

    hr = _AudioClient->SetEventHandle(_AudioSamplesReadyEvent);
    if (FAILED(hr))
    {
        printf("Unable to set ready event: %x.\n", hr);
        return false;
    }

    hr = _AudioClient->GetService(IID_PPV_ARGS(&_RenderClient));
    if (FAILED(hr))
    {
        printf("Unable to get new render client: %x.\n", hr);
        return false;
    }

    return true;
}

//
//  The Event Driven renderer will be woken up every defaultDevicePeriod hundred-nano-seconds.
//  Convert that time into a number of frames.
//
UINT32 CWASAPIRenderer::BufferSizePerPeriod()
{
    REFERENCE_TIME defaultDevicePeriod, minimumDevicePeriod;
    HRESULT hr = _AudioClient->GetDevicePeriod(&defaultDevicePeriod, &minimumDevicePeriod);
    if (FAILED(hr))
    {
        printf("Unable to retrieve device period: %x\n", hr);
        return 0;
    }
    double devicePeriodInSeconds = defaultDevicePeriod / (10000.0*1000.0);
    return static_cast<UINT32>(_MixFormat->nSamplesPerSec * devicePeriodInSeconds + 0.5);
}

//
//  Retrieve the format we'll use to render samples.
//
//  We use the Mix format since we're rendering in shared mode.
//
bool CWASAPIRenderer::LoadFormat()
{
    HRESULT hr = _AudioClient->GetMixFormat(&_MixFormat);
    if (FAILED(hr))
    {
        printf("Unable to get mix format on audio client: %x.\n", hr);
        return false;
    }

    _FrameSize = _MixFormat->nBlockAlign;
    if (!CalculateMixFormatType())
    {
        return false;
    }
    return true;
}

//
//  Crack open the mix format and determine what kind of samples are being rendered.
//
bool CWASAPIRenderer::CalculateMixFormatType()
{
    if (_MixFormat->wFormatTag == WAVE_FORMAT_PCM || 
        _MixFormat->wFormatTag == WAVE_FORMAT_EXTENSIBLE &&
            reinterpret_cast<WAVEFORMATEXTENSIBLE *>(_MixFormat)->SubFormat == KSDATAFORMAT_SUBTYPE_PCM)
    {
        if (_MixFormat->wBitsPerSample == 16)
        {
            _RenderSampleType = SampleType16BitPCM;
        }
        else
        {
            printf("Unknown PCM integer sample type\n");
            return false;
        }
    }
    else if (_MixFormat->wFormatTag == WAVE_FORMAT_IEEE_FLOAT ||
             (_MixFormat->wFormatTag == WAVE_FORMAT_EXTENSIBLE &&
               reinterpret_cast<WAVEFORMATEXTENSIBLE *>(_MixFormat)->SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT))
    {
        _RenderSampleType = SampleTypeFloat;
    }
    else 
    {
        printf("unrecognized device format.\n");
        return false;
    }
    return true;
}
//
//  Initialize the renderer.
//
bool CWASAPIRenderer::Initialize(UINT32 EngineLatency)
{
    if (EngineLatency < 30)
    {
        printf("Engine latency in shared mode event driven cannot be less than 30ms\n");
        return false;
    }

    //
    //  Create our shutdown and samples ready events- we want auto reset events that start in the not-signaled state.
    //
    _ShutdownEvent = CreateEventEx(NULL, NULL, 0, EVENT_MODIFY_STATE | SYNCHRONIZE);
    if (_ShutdownEvent == NULL)
    {
        printf("Unable to create shutdown event: %d.\n", GetLastError());
        return false;
    }

    _AudioSamplesReadyEvent = CreateEventEx(NULL, NULL, 0, EVENT_MODIFY_STATE | SYNCHRONIZE);
    if (_AudioSamplesReadyEvent == NULL)
    {
        printf("Unable to create samples ready event: %d.\n", GetLastError());
        return false;
    }

    //
    //  Create our stream switch event- we want auto reset events that start in the not-signaled state.
    //  Note that we create this event even if we're not going to stream switch - that's because the event is used
    //  in the main loop of the renderer and thus it has to be set.
    //
    _StreamSwitchEvent = CreateEventEx(NULL, NULL, 0, EVENT_MODIFY_STATE | SYNCHRONIZE);
    if (_StreamSwitchEvent == NULL)
    {
        printf("Unable to create stream switch event: %d.\n", GetLastError());
        return false;
    }

    //
    //  Now activate an IAudioClient object on our preferred endpoint and retrieve the mix format for that endpoint.
    //
    HRESULT hr = _Endpoint->Activate(__uuidof(IAudioClient), CLSCTX_INPROC_SERVER, NULL, reinterpret_cast<void **>(&_AudioClient));
    if (FAILED(hr))
    {
        printf("Unable to activate audio client: %x.\n", hr);
        return false;
    }

    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&_DeviceEnumerator));
    if (FAILED(hr))
    {
        printf("Unable to instantiate device enumerator: %x\n", hr);
        return false;
    }

    //
    // Load the MixFormat.  This may differ depending on the shared mode used
    //
    if (!LoadFormat())
    {
        printf("Failed to load the mix format \n");
        return false;
    }

    //
    //  Remember our configured latency in case we'll need it for a stream switch later.
    //
    _EngineLatencyInMS = EngineLatency;

    if (!InitializeAudioEngine())
    {
        return false;
    }

    if (_EnableStreamSwitch)
    {
        if (!InitializeStreamSwitch())
        {
            return false;
        }
    }

    return true;
}

//
//  Shut down the render code and free all the resources.
//
void CWASAPIRenderer::Shutdown()
{
    if (_RenderThread)
    {
        SetEvent(_ShutdownEvent);
        WaitForSingleObject(_RenderThread, INFINITE);
        CloseHandle(_RenderThread);
        _RenderThread = NULL;
    }

    if (_ShutdownEvent)
    {
        CloseHandle(_ShutdownEvent);
        _ShutdownEvent = NULL;
    }
    if (_AudioSamplesReadyEvent)
    {
        CloseHandle(_AudioSamplesReadyEvent);
        _AudioSamplesReadyEvent = NULL;
    }
    if (_StreamSwitchEvent)
    {
        CloseHandle(_StreamSwitchEvent);
        _StreamSwitchEvent = NULL;
    }

    SafeRelease(&_Endpoint);
    SafeRelease(&_AudioClient);
    SafeRelease(&_RenderClient);

    if (_MixFormat)
    {
        CoTaskMemFree(_MixFormat);
        _MixFormat = NULL;
    }

    if (_EnableStreamSwitch)
    {
        TerminateStreamSwitch();
    }
}


//
//  Start rendering - Create the render thread and start rendering the buffer.
//
bool CWASAPIRenderer::Start(RenderBuffer *RenderBufferQueue)
{
    HRESULT hr;

    _RenderBufferQueue = RenderBufferQueue;

    //
    //  We want to pre-roll the first buffer's worth of data into the pipeline.  That way the audio engine won't glitch on startup.  
    //
    {
        BYTE *pData;

        if (_RenderBufferQueue != NULL)
        {
            //
            //  Remove the buffer from the queue.
            //
            RenderBuffer *renderBuffer = _RenderBufferQueue;
            _RenderBufferQueue = renderBuffer->_Next;
            DWORD bufferLengthInFrames = renderBuffer->_BufferLength / _FrameSize;

            hr = _RenderClient->GetBuffer(bufferLengthInFrames, &pData);
            if (FAILED(hr))
            {
                printf("Failed to get buffer: %x.\n", hr);
                return false;
            }

            CopyMemory(pData, renderBuffer->_Buffer, renderBuffer->_BufferLength);
            hr = _RenderClient->ReleaseBuffer(bufferLengthInFrames, 0);

            delete renderBuffer;
        }
        else
        {
            hr = _RenderClient->GetBuffer(_BufferSize, &pData);
            if (FAILED(hr))
            {
                printf("Failed to get buffer: %x.\n", hr);
                return false;
            }
            hr = _RenderClient->ReleaseBuffer(_BufferSize, AUDCLNT_BUFFERFLAGS_SILENT);
        }
        if (FAILED(hr))
        {
            printf("Failed to release buffer: %x.\n", hr);
            return false;
        }
    }

    //
    //  Now create the thread which is going to drive the renderer.
    //
    _RenderThread = CreateThread(NULL, 0, WASAPIRenderThread, this, 0, NULL);
    if (_RenderThread == NULL)
    {
        printf("Unable to create transport thread: %x.", GetLastError());
        return false;
    }

    //
    //  We're ready to go, start rendering!
    //
    hr = _AudioClient->Start();
    if (FAILED(hr))
    {
        printf("Unable to start render client: %x.\n", hr);
        return false;
    }

    return true;
}

//
//  Stop the renderer.
//
void CWASAPIRenderer::Stop()
{
    HRESULT hr;

    //
    //  Tell the render thread to shut down, wait for the thread to complete then clean up all the stuff we 
    //  allocated in Start().
    //
    if (_ShutdownEvent)
    {
        SetEvent(_ShutdownEvent);
    }

    hr = _AudioClient->Stop();
    if (FAILED(hr))
    {
        printf("Unable to stop audio client: %x\n", hr);
    }

    if (_RenderThread)
    {
        WaitForSingleObject(_RenderThread, INFINITE);

        CloseHandle(_RenderThread);
        _RenderThread = NULL;
    }

    //
    //  Drain the buffers in the render buffer queue.
    //
    while (_RenderBufferQueue != NULL)
    {
        RenderBuffer *renderBuffer = _RenderBufferQueue;
        _RenderBufferQueue = renderBuffer->_Next;
        delete renderBuffer;
    }

}


//
//  Render thread - processes samples from the audio engine
//
DWORD CWASAPIRenderer::WASAPIRenderThread(LPVOID Context)
{
    CWASAPIRenderer *renderer = static_cast<CWASAPIRenderer *>(Context);
    return renderer->DoRenderThread();
}

DWORD CWASAPIRenderer::DoRenderThread()
{
    bool stillPlaying = true;
    HANDLE waitArray[3] = {_ShutdownEvent, _StreamSwitchEvent, _AudioSamplesReadyEvent};
    HANDLE mmcssHandle = NULL;
    DWORD mmcssTaskIndex = 0;

    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (FAILED(hr))
    {
        printf("Unable to initialize COM in render thread: %x\n", hr);
        return hr;
    }

    if (!DisableMMCSS)
    {
        mmcssHandle = AvSetMmThreadCharacteristics(L"Audio", &mmcssTaskIndex);
        if (mmcssHandle == NULL)
        {
            printf("Unable to enable MMCSS on render thread: %d\n", GetLastError());
        }
    }

    while (stillPlaying)
    {
        HRESULT hr;
        DWORD waitResult = WaitForMultipleObjects(3, waitArray, FALSE, INFINITE);
        switch (waitResult)
        {
        case WAIT_OBJECT_0 + 0:     // _ShutdownEvent
            stillPlaying = false;       // We're done, exit the loop.
            break;
        case WAIT_OBJECT_0 + 1:     // _StreamSwitchEvent
            //
            //  We've received a stream switch request.
            //
            //  We need to stop the renderer, tear down the _AudioClient and _RenderClient objects and re-create them on the new.
            //  endpoint if possible.  If this fails, abort the thread.
            //
            if (!HandleStreamSwitchEvent())
            {
                stillPlaying = false;
            }
            break;
        case WAIT_OBJECT_0 + 2:     // _AudioSamplesReadyEvent
            //
            //  We need to provide the next buffer of samples to the audio renderer.
            //
            BYTE *pData;
            UINT32 padding;
            UINT32 framesAvailable;
            //
            //  We want to find out how much of the buffer *isn't* available (is padding).
            //
            hr = _AudioClient->GetCurrentPadding(&padding);
            if (SUCCEEDED(hr))
            {
                //
                //  Calculate the number of frames available.  We'll render
                //  that many frames or the number of frames left in the buffer, whichever is smaller.
                //
                framesAvailable = _BufferSize - padding;

                //
                //  If the buffer at the head of the render buffer queue fits in the frames available, render it.  If we don't
                //  have enough room to fit the buffer, skip this pass - we will have enough room on the next pass.
                //
                if (_RenderBufferQueue == NULL)
                {
                    stillPlaying = false;
                }
                else if (_RenderBufferQueue->_BufferLength <= (framesAvailable *_FrameSize))
                {
                    RenderBuffer *renderBuffer = _RenderBufferQueue;
                    _RenderBufferQueue = renderBuffer->_Next;

                    UINT32 framesToWrite = renderBuffer->_BufferLength / _FrameSize;
                    hr = _RenderClient->GetBuffer(framesToWrite, &pData);
                    if (SUCCEEDED(hr))
                    {
                        //
                        //  Copy data from the render buffer to the output buffer and bump our render pointer.
                        //
                        CopyMemory(pData, renderBuffer->_Buffer, framesToWrite*_FrameSize);
                        hr = _RenderClient->ReleaseBuffer(framesToWrite, 0);
                        if (!SUCCEEDED(hr))
                        {
                            printf("Unable to release buffer: %x\n", hr);
                            stillPlaying = false;
                        }
                    }
                    else
                    {
                        printf("Unable to release buffer: %x\n", hr);
                        stillPlaying = false;
                    }
                    //
                    //  We're done with this set of samples, free it.
                    //
                    delete renderBuffer;
                }
            }
            break;
        }
    }

    //
    //  Unhook from MMCSS.
    //
    if (!DisableMMCSS)
    {
        AvRevertMmThreadCharacteristics(mmcssHandle);
    }

    CoUninitialize();
    return 0;
}


bool CWASAPIRenderer::InitializeStreamSwitch()
{
    HRESULT hr = _AudioClient->GetService(IID_PPV_ARGS(&_AudioSessionControl));
    if (FAILED(hr))
    {
        printf("Unable to retrieve session control: %x\n", hr);
        return false;
    }

    //
    //  Create the stream switch complete event- we want a manual reset event that starts in the not-signaled state.
    //
    _StreamSwitchCompleteEvent = CreateEventEx(NULL, NULL, CREATE_EVENT_INITIAL_SET | CREATE_EVENT_MANUAL_RESET, EVENT_MODIFY_STATE | SYNCHRONIZE);
    if (_StreamSwitchCompleteEvent == NULL)
    {
        printf("Unable to create stream switch event: %d.\n", GetLastError());
        return false;
    }
    //
    //  Register for session and endpoint change notifications.  
    //
    //  A stream switch is initiated when we receive a session disconnect notification or we receive a default device changed notification.
    //
    hr = _AudioSessionControl->RegisterAudioSessionNotification(this);
    if (FAILED(hr))
    {
        printf("Unable to register for stream switch notifications: %x\n", hr);
        return false;
    }

    hr = _DeviceEnumerator->RegisterEndpointNotificationCallback(this);
    if (FAILED(hr))
    {
        printf("Unable to register for stream switch notifications: %x\n", hr);
        return false;
    }

    return true;
}

void CWASAPIRenderer::TerminateStreamSwitch()
{
    HRESULT hr;
    if (_AudioSessionControl != NULL)
    {
        hr = _AudioSessionControl->UnregisterAudioSessionNotification(this);
        if (FAILED(hr))
        {
            printf("Unable to unregister for session notifications: %x\n", hr);
        }
    }

    if (_DeviceEnumerator)
    {
        hr = _DeviceEnumerator->UnregisterEndpointNotificationCallback(this);
        if (FAILED(hr))
        {
            printf("Unable to unregister for endpoint notifications: %x\n", hr);
        }
    }

    if (_StreamSwitchCompleteEvent)
    {
        CloseHandle(_StreamSwitchCompleteEvent);
        _StreamSwitchCompleteEvent = NULL;
    }

    SafeRelease(&_AudioSessionControl);
    SafeRelease(&_DeviceEnumerator);
}

//
//  Handle the stream switch.
//
//  When a stream switch happens, we want to do several things in turn:
//
//  1) Stop the current renderer.
//  2) Release any resources we have allocated (the _AudioClient, _AudioSessionControl (after unregistering for notifications) and 
//        _RenderClient).
//  3) Wait until the default device has changed (or 500ms has elapsed).  If we time out, we need to abort because the stream switch can't happen.
//  4) Retrieve the new default endpoint for our role.
//  5) Re-instantiate the audio client on that new endpoint.  
//  6) Retrieve the mix format for the new endpoint.  If the mix format doesn't match the old endpoint's mix format, we need to abort because the stream
//      switch can't happen.
//  7) Re-initialize the _AudioClient.
//  8) Re-register for session disconnect notifications and reset the stream switch complete event.
//
bool CWASAPIRenderer::HandleStreamSwitchEvent()
{
    HRESULT hr;

    assert(_InStreamSwitch);
    //
    //  Step 1.  Stop rendering.
    //
    hr = _AudioClient->Stop();
    if (FAILED(hr))
    {
        printf("Unable to stop audio client during stream switch: %x\n", hr);
        goto ErrorExit;
    }

    //
    //  Step 2.  Release our resources.  Note that we don't release the mix format, we need it for step 6.
    //
    hr = _AudioSessionControl->UnregisterAudioSessionNotification(this);
    if (FAILED(hr))
    {
        printf("Unable to stop audio client during stream switch: %x\n", hr);
        goto ErrorExit;
    }

    SafeRelease(&_AudioSessionControl);
    SafeRelease(&_RenderClient);
    SafeRelease(&_AudioClient);
    SafeRelease(&_Endpoint);

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
    DWORD waitResult = WaitForSingleObject(_StreamSwitchCompleteEvent, 500);
    if (waitResult == WAIT_TIMEOUT)
    {
        printf("Stream switch timeout - aborting...\n");
        goto ErrorExit;
    }

    //
    //  Step 4.  If we can't get the new endpoint, we need to abort the stream switch.  If there IS a new device,
    //          we should be able to retrieve it.
    //
    hr = _DeviceEnumerator->GetDefaultAudioEndpoint(eRender, _EndpointRole, &_Endpoint);
    if (FAILED(hr))
    {
        printf("Unable to retrieve new default device during stream switch: %x\n", hr);
        goto ErrorExit;
    }
    //
    //  Step 5 - Re-instantiate the audio client on the new endpoint.
    //
    hr = _Endpoint->Activate(__uuidof(IAudioClient), CLSCTX_INPROC_SERVER, NULL, reinterpret_cast<void **>(&_AudioClient));
    if (FAILED(hr))
    {
        printf("Unable to activate audio client on the new endpoint: %x.\n", hr);
        goto ErrorExit;
    }
    //
    //  Step 6 - Retrieve the new mix format.
    //
    WAVEFORMATEX *wfxNew;
    hr = _AudioClient->GetMixFormat(&wfxNew);
    if (FAILED(hr))
    {
        printf("Unable to retrieve mix format for new audio client: %x.\n", hr);
        goto ErrorExit;
    }

    //
    //  Note that this is an intentionally naive comparison.  A more sophisticated comparison would
    //  compare the sample rate, channel count and format and apply the appropriate conversions into the render pipeline.
    //
    if (memcmp(_MixFormat, wfxNew, sizeof(WAVEFORMATEX) + wfxNew->cbSize) != 0)
    {
        printf("New mix format doesn't match old mix format.  Aborting.\n");
        CoTaskMemFree(wfxNew);
        goto ErrorExit;
    }
    CoTaskMemFree(wfxNew);

    //
    //  Step 7:  Re-initialize the audio client.
    //
    if (!InitializeAudioEngine())
    {
        goto ErrorExit;
    }

    //
    //  Step 8: Re-register for session disconnect notifications.
    //
    hr = _AudioClient->GetService(IID_PPV_ARGS(&_AudioSessionControl));
    if (FAILED(hr))
    {
        printf("Unable to retrieve session control on new audio client: %x\n", hr);
        goto ErrorExit;
    }
    hr = _AudioSessionControl->RegisterAudioSessionNotification(this);
    if (FAILED(hr))
    {
        printf("Unable to retrieve session control on new audio client: %x\n", hr);
        goto ErrorExit;
    }

    //
    //  Reset the stream switch complete event because it's a manual reset event.
    //
    ResetEvent(_StreamSwitchCompleteEvent);
    //
    //  And we're done.  Start rendering again.
    //
    hr = _AudioClient->Start();
    if (FAILED(hr))
    {
        printf("Unable to start the new audio client: %x\n", hr);
        goto ErrorExit;
    }

    _InStreamSwitch = false;
    return true;

ErrorExit:
    _InStreamSwitch = false;
    return false;
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
        //  Note that we _don't_ set the _StreamSwitchCompleteEvent - that will be set when the OnDefaultDeviceChanged event occurs.
        //
        _InStreamSwitch = true;
        SetEvent(_StreamSwitchEvent);
    }
    if (DisconnectReason == DisconnectReasonFormatChanged)
    {
        //
        //  The stream was disconnected because the format changed on our render device.
        //
        //  We want to flag that we're in a stream switch and then set the stream switch event (which breaks out of the renderer).  We also
        //  want to set the _StreamSwitchCompleteEvent because we're not going to see a default device changed event after this.
        //
        _InStreamSwitch = true;
        SetEvent(_StreamSwitchEvent);
        SetEvent(_StreamSwitchCompleteEvent);
    }
    return S_OK;
}
//
//  Called when the default render device changed.  We just want to set an event which lets the stream switch logic know that it's ok to 
//  continue with the stream switch.
//
HRESULT CWASAPIRenderer::OnDefaultDeviceChanged(EDataFlow Flow, ERole Role, LPCWSTR /*NewDefaultDeviceId*/)
{
    if (Flow == eRender && Role == _EndpointRole)
    {
        //
        //  The default render device for our configuredf role was changed.  
        //
        //  If we're not in a stream switch already, we want to initiate a stream switch event.  
        //  We also we want to set the stream switch complete event.  That will signal the render thread that it's ok to re-initialize the
        //  audio renderer.
        //
        if (!_InStreamSwitch)
        {
            _InStreamSwitch = true;
            SetEvent(_StreamSwitchEvent);
        }
        SetEvent(_StreamSwitchCompleteEvent);
    }
    return S_OK;
}

//
//  IUnknown
//
HRESULT CWASAPIRenderer::QueryInterface(REFIID Iid, void **Object)
{
    if (Object == NULL)
    {
        return E_POINTER;
    }
    *Object = NULL;

    if (Iid == IID_IUnknown)
    {
        *Object = static_cast<IUnknown *>(static_cast<IAudioSessionEvents *>(this));
        AddRef();
    }
    else if (Iid == __uuidof(IMMNotificationClient))
    {
        *Object = static_cast<IMMNotificationClient *>(this);
        AddRef();
    }
    else if (Iid == __uuidof(IAudioSessionEvents))
    {
        *Object = static_cast<IAudioSessionEvents *>(this);
        AddRef();
    }
    else
    {
        return E_NOINTERFACE;
    }
    return S_OK;
}
ULONG CWASAPIRenderer::AddRef()
{
    return InterlockedIncrement(&_RefCount);
}
ULONG CWASAPIRenderer::Release()
{
    ULONG returnValue = InterlockedDecrement(&_RefCount);
    if (returnValue == 0)
    {
        delete this;
    }
    return returnValue;
}