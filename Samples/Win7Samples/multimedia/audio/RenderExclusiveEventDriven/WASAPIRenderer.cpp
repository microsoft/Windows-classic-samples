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

CWASAPIRenderer::CWASAPIRenderer(IMMDevice *Endpoint) : 
    _RefCount(1),
    _Endpoint(Endpoint),
    _AudioClient(NULL),
    _RenderClient(NULL),
    _RenderThread(NULL),
    _ShutdownEvent(NULL),
    _MixFormat(NULL),
    _RenderBufferQueue(NULL),
    _AudioSamplesReadyEvent(NULL)
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
//  Initialize WASAPI in event driven mode, associate the audio client with our samples ready event handle, and retrieve 
//  a render client for the transport.
//
bool CWASAPIRenderer::InitializeAudioEngine()
{
    REFERENCE_TIME bufferDuration = _EngineLatencyInMS*10000;

    HRESULT hr = _AudioClient->Initialize(AUDCLNT_SHAREMODE_EXCLUSIVE, 
                                          AUDCLNT_STREAMFLAGS_EVENTCALLBACK | AUDCLNT_STREAMFLAGS_NOPERSIST, 
                                          bufferDuration, 
                                          bufferDuration ,
                                          _MixFormat, 
                                          NULL);

    //
    //  When rendering in exclusive mode event driven, the HDAudio specification requires that the buffers handed to the device must 
    //  be aligned on a 128 byte boundary.  When the buffer is initialized and the resulting buffer size would not be 128 byte aligned,
    //  we need to "swizzle" the periodicity of the engine to ensure that the buffers are properly aligned.
    //
    if (hr == AUDCLNT_E_BUFFER_SIZE_NOT_ALIGNED) 
    {
        UINT32 bufferSize;
        printf("Buffers not aligned. Aligning the buffers... \n");
        //
        //  Retrieve the buffer size for the audio client.  The buffer size returned is aligned to the nearest 128 byte
        //  boundary given the input buffer duration.
        //
        hr = _AudioClient->GetBufferSize(&bufferSize);
        if(FAILED(hr))
        {
            printf("Unable to get audio client buffer: %x. \n", hr);
            return false;
        }

        //
        //  Release old AudioClient
        //
        SafeRelease(&_AudioClient);

        //
        //  Calculate the new aligned periodicity.  We do that by taking the buffer size returned (which is in frames),
        //  multiplying it by the frames/second in the render format (which gets us seconds per buffer), then converting the 
        //  seconds/buffer calculation into a REFERENCE_TIME.
        //
        bufferDuration = (REFERENCE_TIME)(10000.0 *                         // (REFERENCE_TIME / ms) *
                                          1000 *                            // (ms / s) *
                                          bufferSize /                      // frames /
                                          _MixFormat->nSamplesPerSec +      // (frames / s)
                                          0.5);                             // rounding

        //
        //  Now reactivate an IAudioClient object on our preferred endpoint and reinitialize AudioClient
        //
        hr = _Endpoint->Activate(__uuidof(IAudioClient), CLSCTX_INPROC_SERVER, NULL, reinterpret_cast<void **>(&_AudioClient));
        if (FAILED(hr))
        {
            printf("Unable to activate audio client: %x.\n", hr);
            return false;
        }

        hr = _AudioClient->Initialize(AUDCLNT_SHAREMODE_EXCLUSIVE, 
                                      AUDCLNT_STREAMFLAGS_EVENTCALLBACK | AUDCLNT_STREAMFLAGS_NOPERSIST, 
                                      bufferDuration, 
                                      bufferDuration, 
                                      _MixFormat, 
                                      NULL);
        if (FAILED(hr))
        {
            printf("Unable to reinitialize audio client: %x \n", hr);
            return false;
        }

    }
    else if (FAILED(hr))
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
        printf("Unable to get set event handle: %x.\n", hr);
        return false;
    }
    //
    //  When rendering in event driven mode, we'll always have exactly a buffer's size worth of data
    //  available every time we wake up.
    //
    _BufferSizePerPeriod = _BufferSize;

    hr = _AudioClient->GetService(IID_PPV_ARGS(&_RenderClient));
    if (FAILED(hr))
    {
        printf("Unable to get new render client: %x.\n", hr);
        return false;
    }

    return true;
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

    _AudioSamplesReadyEvent = CreateEventEx(NULL, NULL, 0, EVENT_MODIFY_STATE | SYNCHRONIZE);
    if (_AudioSamplesReadyEvent == NULL)
    {
        printf("Unable to create samples ready event: %d.\n", GetLastError());
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
    if (_AudioSamplesReadyEvent)
    {
        CloseHandle(_AudioSamplesReadyEvent);
        _AudioSamplesReadyEvent = NULL;
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
    //  Now create the thread which is going to drive the renderer.
    //
    _RenderThread = CreateThread(NULL, 0, WASAPIRenderThread, this, 0, NULL);
    if (_RenderThread == NULL)
    {
        printf("Unable to create transport thread: %x.", GetLastError());
        return false;
    }

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
    HANDLE waitArray[2] = {_ShutdownEvent, _AudioSamplesReadyEvent};
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
        DWORD waitResult = WaitForMultipleObjects(2, waitArray, FALSE, INFINITE);
        switch (waitResult)
        {
        case WAIT_OBJECT_0 + 0:     // _ShutdownEvent
            stillPlaying = false;       // We're done, exit the loop.
            break;
        case WAIT_OBJECT_0 + 1:     // _AudioSamplesReadyEvent
            //
            //  We need to provide the next buffer of samples to the audio renderer.
            //
            BYTE *pData;

            //
            //  When rendering in event driven mode, every time we wake up, we'll have a buffer's worth of data available, so if we have
            //  data in our queue, render it.
            //
            if (_RenderBufferQueue == NULL)
            {
                stillPlaying = false;
            }
            else
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
                    printf("Unable to get buffer: %x\n", hr);
                    stillPlaying = false;
                }
                //
                //  We're done with this set of samples, free it.
                //
                delete renderBuffer;
            }
            break;
        }
    }
    if (!DisableMMCSS)
    {
        AvRevertMmThreadCharacteristics(mmcssHandle);
    }

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
