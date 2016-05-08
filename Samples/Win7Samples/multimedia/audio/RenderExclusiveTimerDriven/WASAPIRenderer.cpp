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

#include <mmsystem.h>

//
//  A simple WASAPI Render client.
//

CWASAPIRenderer::CWASAPIRenderer(IMMDevice *Endpoint) : 
    _RefCount(1),
    _Endpoint(Endpoint),
    _AudioClient(NULL),
    _RenderClient(NULL),
    _RenderThread(NULL),
    _ShutdownEvent(NULL),
    _MixFormat(NULL),
    _RenderBufferQueue(0)
{
    _Endpoint->AddRef();    // Since we're holding a copy of the endpoint, take a reference to it.  It'll be released in Shutdown();
}

//
//  Empty destructor - everything should be released in the Shutdown() call.
//
CWASAPIRenderer::~CWASAPIRenderer(void) 
{
}
#define PERIODS_PER_BUFFER 4
//
//  Initialize WASAPI in event driven mode, associate the audio client with our samples ready event handle, and retrieve 
//  a render client for the transport.
//
bool CWASAPIRenderer::InitializeAudioEngine()
{
    REFERENCE_TIME bufferDuration = _EngineLatencyInMS*10000*PERIODS_PER_BUFFER;
    REFERENCE_TIME periodicity = _EngineLatencyInMS*10000;

    //
    //  We initialize the engine with a periodicity of _EngineLatencyInMS and a buffer size of PERIODS_PER_BUFFER times the latency - this ensures 
    //  that we will always have space available for rendering audio.  We only need to do this for exclusive mode timer driven rendering.
    //
    HRESULT hr = _AudioClient->Initialize(AUDCLNT_SHAREMODE_EXCLUSIVE, 
        AUDCLNT_STREAMFLAGS_NOPERSIST, 
        bufferDuration, 
        periodicity,
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

    hr = _AudioClient->GetService(IID_PPV_ARGS(&_RenderClient));
    if (FAILED(hr))
    {
        printf("Unable to get new render client: %x.\n", hr);
        return false;
    }

    return true;
}
//
//  That buffer duration is calculated as being PERIODS_PER_BUFFER x the
//  periodicity, so each period we're going to see 1/PERIODS_PER_BUFFERth 
//  the size of the buffer.
//
UINT32 CWASAPIRenderer::BufferSizePerPeriod()
{
    return _BufferSize / PERIODS_PER_BUFFER;
}

//
//  Retrieve the format we'll use to rendersamples.
//
//  Start with the mix format and see if the endpoint can render that.  If not, try
//  the mix format converted to an integer form (most audio solutions don't support floating 
//  point rendering and the mix format is usually a floating point format).
//
bool CWASAPIRenderer::LoadFormat()
{
    HRESULT hr = _AudioClient->GetMixFormat(&_MixFormat);
    if (FAILED(hr))
    {
        printf("Unable to get mix format on audio client: %x.\n", hr);
        return false;
    }
    assert(_MixFormat != NULL);

    hr = _AudioClient->IsFormatSupported(AUDCLNT_SHAREMODE_EXCLUSIVE,_MixFormat, NULL);
    if (hr == AUDCLNT_E_UNSUPPORTED_FORMAT)
    {
        printf("Device does not natively support the mix format, converting to PCM.\n");

        //
        //  If the mix format is a float format, just try to convert the format to PCM.
        //
        if (_MixFormat->wFormatTag == WAVE_FORMAT_IEEE_FLOAT)
        {
            _MixFormat->wFormatTag = WAVE_FORMAT_PCM;
            _MixFormat->wBitsPerSample = 16;
            _MixFormat->nBlockAlign = (_MixFormat->wBitsPerSample / 8) * _MixFormat->nChannels;
            _MixFormat->nAvgBytesPerSec = _MixFormat->nSamplesPerSec*_MixFormat->nBlockAlign;
        }
        else if (_MixFormat->wFormatTag == WAVE_FORMAT_EXTENSIBLE && 
            reinterpret_cast<WAVEFORMATEXTENSIBLE *>(_MixFormat)->SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT)
        {
            WAVEFORMATEXTENSIBLE *waveFormatExtensible = reinterpret_cast<WAVEFORMATEXTENSIBLE *>(_MixFormat);
            waveFormatExtensible->SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
            waveFormatExtensible->Format.wBitsPerSample = 16;
            waveFormatExtensible->Format.nBlockAlign = (_MixFormat->wBitsPerSample / 8) * _MixFormat->nChannels;
            waveFormatExtensible->Format.nAvgBytesPerSec = waveFormatExtensible->Format.nSamplesPerSec*waveFormatExtensible->Format.nBlockAlign;
            waveFormatExtensible->Samples.wValidBitsPerSample = 16;
        }
        else
        {
            printf("Mix format is not a floating point format.\n");
            return false;
        }

        hr = _AudioClient->IsFormatSupported(AUDCLNT_SHAREMODE_EXCLUSIVE,_MixFormat,NULL);
        if (FAILED(hr))
        {
            printf("Format is not supported \n");
            return false;
        }
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
    //
    //  Create our shutdown and samples ready events- we want auto reset events that start in the not-signaled state.
    //
    _ShutdownEvent = CreateEventEx(NULL, NULL, 0, EVENT_MODIFY_STATE | SYNCHRONIZE);
    if (_ShutdownEvent == NULL)
    {
        printf("Unable to create shutdown event: %d.\n", GetLastError());
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

    SafeRelease(&_Endpoint);
    SafeRelease(&_AudioClient);
    SafeRelease(&_RenderClient);

    if (_MixFormat)
    {
        CoTaskMemFree(_MixFormat);
        _MixFormat = NULL;
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
    HANDLE waitArray[1] = {_ShutdownEvent};
    HANDLE mmcssHandle = NULL;
    DWORD mmcssTaskIndex = 0;

    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (FAILED(hr))
    {
        printf("Unable to initialize COM in render thread: %x\n", hr);
        return hr;
    }

    //
    //  We want to make sure that our timer resolution is a multiple of the latency, otherwise the system timer cadence will
    //  cause us to starve the renderer.
    //
    //  Set the system timer to 1ms as a worst case value.
    //
    timeBeginPeriod(1);

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
        //
        //  When running in timer mode, wait for half the configured latency.
        //
        DWORD waitResult = WaitForMultipleObjects(1, waitArray, FALSE, _EngineLatencyInMS/2);
        switch (waitResult)
        {
        case WAIT_OBJECT_0 + 0:     // _ShutdownEvent
            stillPlaying = false;       // We're done, exit the loop.
            break;
        case WAIT_TIMEOUT:          // Timeout
            //
            //  We need to provide the next buffer of samples to the audio renderer.  If we're done with our samples, we're done.
            //
            if (_RenderBufferQueue == NULL)
            {
                stillPlaying = false;
            }
            else
            {
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
                    while (_RenderBufferQueue != NULL && (_RenderBufferQueue->_BufferLength <= (framesAvailable *_FrameSize)))
                    {
                        //
                        //  We know that the buffer at the head of the queue will fit, so remove it and write it into 
                        //  the engine buffer.  Continue doing this until we no longer can fit
                        //  the recent buffer into the engine buffer.
                        //
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

                        //
                        //  Now recalculate the padding and frames available because we've consumed
                        //  some of the buffer.
                        //
                        hr = _AudioClient->GetCurrentPadding(&padding);
                        if (SUCCEEDED(hr))
                        {
                            //
                            //  Calculate the number of frames available.  We'll render
                            //  that many frames or the number of frames left in the buffer, 
                            //  whichever is smaller.
                            //
                            framesAvailable = _BufferSize - padding;
                        }
                        else
                        {
                            printf("Unable to get current padding: %x\n", hr);
                            stillPlaying = false;
                        }
                    }
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

    //
    //  Revert the system timer to the previous value.
    //
    timeEndPeriod(1);

    CoUninitialize();
    return 0;
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
        *Object = static_cast<IUnknown *>(this);
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
