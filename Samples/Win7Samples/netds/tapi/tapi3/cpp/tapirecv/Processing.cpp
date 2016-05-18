/*


Copyright (c) 1999 - 2000  Microsoft Corporation


Module Name:

    Processing.cpp

Abstract:

    Implementation of the ITTAPIEventNotification interface. An application 
    must implement and register this interface in order to receive calls and 
    events related to calls. See TAPI documentation for more information on 
    this interface.
 
    This file also contains a collection of functions related to event 
    processing

*/


#include "common.h"

#include "Processing.h"

#include "WorkerThread.h"

#include "AVIFileWriter.h"


//
// the name of the file we will save the incoming audio to
//

#define SZ_OUTPUTFILENAME "recording.wav"


//
// the worker thread for asycnhronous message processing
//

CWorkerThread g_WorkerThread;


///////////////////////////////////////////////////////////////////////////////
//
// ITTAPIEventNotification::Event
//
// the method on the tapi callback object that will be called 
// when tapi notifies the application of an event 
//
// this method should return as soon as possible, so we are not 
// going to actually process the events here. Instead, we will post
// events to a worker thread for asynchronous processing.
//
// in a real life application that could be another thread, the
// application's main thread, or we could post events to a window.
//
///////////////////////////////////////////////////////////////////////////////

HRESULT STDMETHODCALLTYPE CTAPIEventNotification::Event(IN TAPI_EVENT TapiEvent,
                                                        IN IDispatch *pEvent)
{
    
    LogMessage("CTAPIEventNotification::Event "
               "posting message for asynchronous processing");

    
    //
    // AddRef the event so it doesn't go away after we return
    //

    pEvent->AddRef();


    //
    // Post a message to our own worker thread to be processed asynchronously
    //

    g_WorkerThread.PostMessage(WM_PRIVATETAPIEVENT,
                              (WPARAM) TapiEvent,
                              (LPARAM) pEvent);

    return S_OK;
}


///////////////////////////////////////////////////////////////////////////////
//
// GetTerminalFromStreamEvent
//
//
// given pCallMediaEvent, find the one and only terminal selected on its stream
//
// return the terminal and S_OK if success, error otherwise
//
///////////////////////////////////////////////////////////////////////////////

HRESULT GetTerminalFromStreamEvent(IN ITCallMediaEvent *pCallMediaEvent,
                                   OUT ITTerminal **ppTerminal)
{

    HRESULT hr = E_FAIL;

    
    //
    // don't return garbage
    //

    *ppTerminal = NULL;


    //
    // Get the stream for this event.
    //

    ITStream *pStream = NULL;
    
    hr = pCallMediaEvent->get_Stream(&pStream);

    if ( FAILED(hr) )
    {
        
        LogMessage("GetTerminalFromStreamEvent: "
                   "Failed to get stream from pCallMediaEvent hr = 0x%lx", hr);

        return hr;
    }


    //
    // Enumerate terminals on this stream.
    //

    IEnumTerminal *pEnumTerminal = NULL;

    hr = pStream->EnumerateTerminals(&pEnumTerminal);

    pStream->Release();
    pStream = NULL;

    if ( FAILED(hr) ) 
    {
        LogMessage("GetTerminalFromStreamEvent: "
                   "Failed to enumerate terminals hr = 0x%lx", hr);
        return hr;
    }

    
    //
    // we should have at most one terminal selected on the stream, so 
    // get the first terminal
    //

    ITTerminal *pTerminal = NULL;

    hr = pEnumTerminal->Next(1, &pTerminal, NULL);

    if ( hr != S_OK )
    {
        LogMessage("GetTerminalFromStreamEvent: "
                   "Failed to get a terminal from enumeration hr = 0x%lx", hr);

        pEnumTerminal->Release();
        pEnumTerminal = NULL;

        return E_FAIL;
    }

    *ppTerminal = pTerminal;

    pTerminal = NULL;

    
    //
    // we should not have any more terminals on this stream, 
    // double-check this.
    //

    hr = pEnumTerminal->Next(1, &pTerminal, NULL);

    if (hr == S_OK)
    {
        LogError("GetTerminalFromStreamEvent: "
                 "more than one terminal on the stream!");

        _ASSERTE(FALSE);

        pTerminal->Release();
        pTerminal = NULL;

    }

    pEnumTerminal->Release();
    pEnumTerminal = NULL;


    return S_OK;
}





///////////////////////////////////////////////////////////////////////////////
//
// IsMessageForActiveCall
//
// return TRUE if the event received is for the currently active call
//
///////////////////////////////////////////////////////////////////////////////

BOOL IsMessageForActiveCall(IN ITCallStateEvent *pCallStateEvent)
{
    
    EnterCriticalSection(&g_CurrentCallCritSection);
    
    //
    // if we don't have an active call we have not received call notification
    // for a call that we own, so return FALSE
    //

    if (NULL == g_pCurrentCall)
    {
        LogMessage("IsMessageForActiveCall: no active call. return FALSE");
    
        LeaveCriticalSection(&g_CurrentCallCritSection);


        return FALSE;
    }


    //
    // get the call corresponding to the event
    //
    
    ITCallInfo *pCallInfo = NULL;
    
    HRESULT hr = pCallStateEvent->get_Call(&pCallInfo);

    if (FAILED(hr))
    {
        LogError("IsMessageForActiveCall: failed to get call. "
                 "returning FALSE");
        
        LeaveCriticalSection(&g_CurrentCallCritSection);

        return FALSE;
    }


    //
    // get IUnknown of the call from the event
    //

    IUnknown *pIncomingCallUnk = NULL;

    hr = pCallInfo->QueryInterface(IID_IUnknown, (void**)&pIncomingCallUnk);

    pCallInfo->Release();
    pCallInfo = NULL;

    if (FAILED(hr))
    {
        LogError("IsMessageForActiveCall: "
                 "failed to qi incoming call for IUnknown. returning FALSE");

        LeaveCriticalSection(&g_CurrentCallCritSection);
        
        return FALSE;
    }


    //
    // get IUnknown of the call from the event
    //

    IUnknown *pCurrentCallUnk = NULL;

    hr = g_pCurrentCall->QueryInterface(IID_IUnknown, (void**)&pCurrentCallUnk);

    LeaveCriticalSection(&g_CurrentCallCritSection);

    if (FAILED(hr))
    {
        LogError("IsMessageForActiveCall: "
                 "Failed to QI current call for IUnknown. returning FALSE.");

        pIncomingCallUnk->Release();
        pIncomingCallUnk = NULL;


        return FALSE;
    }


    //
    // compare IUnknowns of the current call and the event call
    // if they are the same, this is the same call
    //

    BOOL bSameCall = FALSE;

    if (pCurrentCallUnk == pIncomingCallUnk)
    {
        bSameCall = TRUE;
    }
    else
    {
        LogMessage("IsMessageForActiveCall: "
                    "current and event calls are different. returning FALSE.");

        bSameCall = FALSE;
    }

    pCurrentCallUnk->Release();
    pCurrentCallUnk = NULL;
        
    pIncomingCallUnk->Release();
    pIncomingCallUnk = NULL;

    return bSameCall;
}


///////////////////////////////////////////////////////////////////////////////
//
// GetAddressFromCall
//
//
// return ITAddress of the address corresponding to the supplied 
// ITBasicCallControl
//
///////////////////////////////////////////////////////////////////////////////

HRESULT GetAddressFromCall(IN ITBasicCallControl *pCallControl, 
                           OUT ITAddress **ppAddress)
{

    HRESULT hr = E_FAIL;


    //
    // don't return garbage
    //

    *ppAddress = NULL;

    
    //
    // get ITCallInfo so we can get the call's address
    //
    
    ITCallInfo *pCallInfo = NULL;

    hr = pCallControl->QueryInterface(IID_ITCallInfo, (void**)&pCallInfo);

    if (FAILED(hr))
    {
        LogError("GetAddressFromCall: "
                 "Failed to QI call for ITCallInfo");

        return hr;
    }


    //
    // get the call's address
    //

    ITAddress *pAddress = NULL;

    hr = pCallInfo->get_Address(&pAddress);
    
    
    pCallInfo->Release();
    pCallInfo = NULL;


    if (FAILED(hr))
    {
        LogError("GetAddressFromCall: failed to get address");

    }
    else 
    {
        *ppAddress = pAddress;
    }

    return hr;
}


///////////////////////////////////////////////////////////////////////////////
//
// CreateRenderMediaStreamingTerminal
//
// create rendering media streaming terminal. 
//
// if success, return the pointer to the terminal
// if failed, return NULL
//
///////////////////////////////////////////////////////////////////////////////

ITTerminal *CreateRenderMediaStreamingTerminal(IN ITBasicCallControl *pCallControl)
{
    
    HRESULT hr = E_FAIL;


    //
    // get address from call
    //

    ITAddress *pAddress = NULL;

    hr = GetAddressFromCall(pCallControl, &pAddress);

    if (FAILED(hr))
    {
        LogError("CreateRenderMediaStreamingTerminal: failed to get address from call");

        return NULL;
    }

    
    //
    // get the terminal support interface from the address
    //

    ITTerminalSupport *pTerminalSupport = NULL;

    hr = pAddress->QueryInterface( IID_ITTerminalSupport,
                                   (void **)&pTerminalSupport );

    pAddress->Release();
    pAddress = NULL;

    if (FAILED(hr))
    {
        LogError("CreateRenderMediaStreamingTerminal: "
                 "failed to QI pAddress for ITTerminalSupport");

        return NULL;
    }

    
    //
    // get string for the terminal's class id
    //

    WCHAR *pszTerminalClass = NULL;

    hr = StringFromIID(CLSID_MediaStreamTerminal, &pszTerminalClass);

    if (FAILED(hr))
    {
        LogError("CreateRenderMediaStreamingTerminal: "
                 "Failed to generate string from terminal's class id");

        pTerminalSupport->Release();
        pTerminalSupport = NULL;

        return NULL;
    }


    //
    // make bstr out of the class id
    //

    BSTR bstrTerminalClass = SysAllocString (pszTerminalClass);

    
    //
    // free the string returned by StringFromIID
    //

    CoTaskMemFree(pszTerminalClass);
    pszTerminalClass = NULL;


    //
    // create media streaming terminal for rendering
    //
    
    ITTerminal *pTerminal = NULL;

    hr = pTerminalSupport->CreateTerminal(bstrTerminalClass,
                                          TAPIMEDIATYPE_AUDIO,
                                          TD_RENDER,
                                          &pTerminal);

    
    //
    // release resources no longer needed
    //

    SysFreeString(bstrTerminalClass);
    bstrTerminalClass = NULL;

    pTerminalSupport->Release();
    pTerminalSupport = NULL;


    if (FAILED(hr))
    {
        LogError("CreateRenderMediaStreamingTerminal: "
                 "failed to create media streaming terminal hr = 0x%lx", hr);

        return NULL;
    }


    //
    // successfully created media streaming terminal. return.
    //

    LogMessage("CreateRenderMediaStreamingTerminal: "
               "Terminal created successfully");

    return pTerminal;

}


///////////////////////////////////////////////////////////////////////////////
//
// SetAudioFormat
// 
// tell media streaming terminal the audio format we would like
// to receive
//
///////////////////////////////////////////////////////////////////////////////

HRESULT SetAudioFormat(IN ITTerminal *pTerminal)
{

    HRESULT hr = E_FAIL;
    
    
    //
    // get ITAMMediaFormat interface on the terminal
    //

    ITAMMediaFormat *pITMediaFormat = NULL;

    hr = pTerminal->QueryInterface(IID_ITAMMediaFormat, 
                                   (void **)&pITMediaFormat);

    if (FAILED(hr))
    {
        LogError("SetAudioFormat: failed to QI terminal for ITAMMediaFormat");
     
        return hr;
    }

    //
    // will ask media streaming terminal for audio in this format
    //

    WAVEFORMATEX WaveFormat;

    ZeroMemory(&WaveFormat, sizeof(WAVEFORMATEX));
    
    WaveFormat.wFormatTag = WAVE_FORMAT_PCM; // pcm
    WaveFormat.nChannels = 1;                // mono
    WaveFormat.nSamplesPerSec = 8000;        // 8 khz
    WaveFormat.nAvgBytesPerSec = 16000;      // 16000 bytes per sec
    WaveFormat.nBlockAlign = 2;              // 2 bytes per block 
    WaveFormat.wBitsPerSample = 16;          // 16 bits per sample
    WaveFormat.cbSize = 0;                   // no extra format-specific data

    //
    // configure the wave format we want
    //

    AM_MEDIA_TYPE MediaType;

    MediaType.majortype            = MEDIATYPE_Audio;
    MediaType.subtype              = MEDIASUBTYPE_PCM;
    MediaType.bFixedSizeSamples    = TRUE;
    MediaType.bTemporalCompression = FALSE;
    MediaType.lSampleSize          = 0;
    MediaType.formattype           = FORMAT_WaveFormatEx;
    MediaType.pUnk                 = NULL;
    MediaType.cbFormat             = sizeof(WAVEFORMATEX);
    MediaType.pbFormat             = (BYTE*)&WaveFormat;


    //
    // log the wave format we are setting on the terminal
    //

    LogMessage("SetAudioFormat: setting wave format on terminal. ");
    LogFormat(&WaveFormat);


    //
    // set the format
    //

    hr = pITMediaFormat->put_MediaFormat(&MediaType);


    if (FAILED(hr))
    {

        //
        // try to see what format the terminal wanted
        //

        LogError("SetAudioFormat: failed to set format");

        AM_MEDIA_TYPE *pMediaFormat = NULL;

        HRESULT hr2 = pITMediaFormat->get_MediaFormat(&pMediaFormat);


        if (SUCCEEDED(hr2))
        {

            if (pMediaFormat->formattype == FORMAT_WaveFormatEx)
            {

                //
                // log the terminal's format
                //

                LogError("SetAudioFormat: terminal's format is");
                LogFormat((WAVEFORMATEX*) pMediaFormat->pbFormat);

            }
            else 
            {

                LogError("SetAudioFormat: "
                         "terminal's format is not WAVEFORMATEX");
            }


            //
            // note: we are responsible for deallocating the format returned by 
            // get_MediaFormat
            //

            DeleteMediaType(pMediaFormat);

        } //  succeeded getting terminal's format
        else
        {

            LogError("SetAudioFormat: failed to get terminal's format");

        }

    }

    pITMediaFormat->Release();
    pITMediaFormat = NULL;


    LogError("SetAudioFormat: completed");

 
    return hr;

}


///////////////////////////////////////////////////////////////////////////////
//
// SetAllocatorProperties
//
// suggest allocator properties to the terminal
//
///////////////////////////////////////////////////////////////////////////////

HRESULT SetAllocatorProperties(IN ITTerminal *pTerminal)
{


    //
    // different buffer sizes may produce different sound quality, depending
    // on the underlying transport that is being used.
    // 
    // this function illustrates how an app can control the number and size of
    // buffers. A multiple of 30 ms (480 bytes at 16-bit 8 KHz PCM) is the most
    // appropriate sample size for IP (especailly G.723.1).
    //
    // However, small buffers can cause poor audio quality on some voice boards.
    //
    // If this method is not called, the allocator properties suggested by the 
    // connecting filter will be used.
    //


    HRESULT hr = E_FAIL;

    
    //
    // get ITAllocator properties interface on the terminal
    //

    ITAllocatorProperties *pITAllocatorProperties = NULL;


    hr = pTerminal->QueryInterface(IID_ITAllocatorProperties,
                                   (void **)&pITAllocatorProperties);


    if (FAILED(hr))
    {
        LogError("SetAllocatorProperties: "
                 "failed to QI terminal for ITAllocatorProperties");

        return hr;
    }

    
    //
    // configure allocator properties
    //
    // suggest the size and number of the samples for MST to pre-allocate. 
    //
    
    ALLOCATOR_PROPERTIES AllocProps;
    
    AllocProps.cBuffers   = 5;    // ask MST to allocate 5 buffers
    AllocProps.cbBuffer   = 4800; // 4800 bytes each
    AllocProps.cbAlign    = 1;    // no need to align buffers
    AllocProps.cbPrefix   = 0;    // no extra memory preceeding the actual data
    
    
    hr = pITAllocatorProperties->SetAllocatorProperties(&AllocProps);

    if (FAILED(hr))
    {
        LogError("SetAllocatorProperties: "
                 "failed to set allocator properties. hr = 0x%lx", hr);

        pITAllocatorProperties->Release();
        pITAllocatorProperties = NULL;

        return hr;
    }

    
    //
    // ask media streaming terminal to allocate buffers for us. 
    // TRUE is the default, so strictly speaking, we didn't have to call 
    // this method.
    //

    hr = pITAllocatorProperties->SetAllocateBuffers(TRUE);


    pITAllocatorProperties->Release();
    pITAllocatorProperties = NULL;

    
    if (FAILED(hr))
    {
        LogError("SetAllocatorProperties: "
                 "failed to SetAllocateBuffers, hr = 0x%lx", hr);

        return hr;
    }


    //
    // succeeded setting allocator properties
    //

    LogMessage("SetAllocatorProperties: succeeded.");


    return S_OK;

}


///////////////////////////////////////////////////////////////////////////////
//
// SelectAndInitializeTerminal
//
//
// Set audio format and allocator properties on the terminal and
// select the terminal on the stream
//
///////////////////////////////////////////////////////////////////////////////

HRESULT SelectAndInitializeTerminal(IN ITTerminal *pRecordTerminal,
                                    IN ITStream *pStream)
{
    
    HRESULT hr = E_FAIL;


    //
    // set audio format on the created terminal
    //

    hr = SetAudioFormat(pRecordTerminal);

    if (FAILED(hr))
    {
     
        //
        // the terminal does not support the wave format we wanted.
        // no big deal for the receiver, we'll just have to create the 
        // file in the format requested by MST.
        //

        LogMessage("CreateAndSelectMST: "
                   "Failed to set audio format on recording terminal. "
                   "Continuing");

    }


    //
    // set allocator properties for this terminal
    //

    hr = SetAllocatorProperties(pRecordTerminal);

    if (FAILED(hr))
    {
     
        //
        // not a fatal error. our allocator props were rejected, 
        // but this does not necessarily mean streaming will fail.
        //

        LogError("CreateAndSelectMST: Failed to set "
                 "allocator properties on recording terminal");
    }


    //
    // select terminal on the stream
    //

    hr = pStream->SelectTerminal(pRecordTerminal);

    if (FAILED(hr))
    {

        LogError("CreateAndSelectMST: Failed to select "
                 "terminal on the stream");
    }

    return hr;

}


///////////////////////////////////////////////////////////////////////////////
//
// IsRenderingStream
//
// returns TRUE if the stream's direction is TD_RENDER
//
///////////////////////////////////////////////////////////////////////////////

BOOL IsRenderingStream(ITStream *pStream)
{

    //
    // check the stream's direction
    //

    TERMINAL_DIRECTION TerminalDirection;

    HRESULT hr = pStream->get_Direction(&TerminalDirection);

    if (FAILED(hr))
    {
        LogError("IsRenderingStream: Failed to get stream direction");
        
        return FALSE;
    }


    if (TD_RENDER == TerminalDirection)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


///////////////////////////////////////////////////////////////////////////////
//
// IsAudioStream
//
// returns TRUE if the stream's type is TAPIMEDIATYPE_AUDIO 
//
///////////////////////////////////////////////////////////////////////////////

BOOL IsAudioStream(ITStream *pStream)
{
    
    //
    // check the stream's media type 
    //
    
    long nMediaType = 0;

    HRESULT hr = pStream->get_MediaType(&nMediaType);

    if (FAILED(hr))
    {
        LogError("IsAudioStream: Failed to get media type");

        return FALSE;
    }


    //
    // return true if the stream is audio
    //

    if (TAPIMEDIATYPE_AUDIO == nMediaType)
    {
        
        return TRUE;
    }
    else
    {
    
        return FALSE;
    }

}


///////////////////////////////////////////////////////////////////////////////
// 
// CreateAndSelectMST
// 
// check the call's streams. create media streaming terminal on the first
// incoming audio stream
//
// returns:
//
// S_OK if a terminal was created and selected
// S_FALSE if no appropriate stream was found
// error if failed
//
///////////////////////////////////////////////////////////////////////////////

HRESULT CreateAndSelectMST()
{

    LogMessage("CreateAndSelectMST: started");


    //
    // we should already have the call
    //

    if (NULL == g_pCurrentCall) 
    {
        LogError("CreateAndSelectMST: g_pCurrentCall is NULL");

        return E_UNEXPECTED;
    }


    HRESULT hr = E_FAIL;


    //
    // get the ITStreamControl interface for this call
    //

    ITStreamControl *pStreamControl = NULL;

    hr = g_pCurrentCall->QueryInterface(IID_ITStreamControl,
                                        (void**)&pStreamControl);

    if (FAILED(hr))
    {
        LogError("CreateAndSelectMST: failed to QI call for ITStreamControl");

        return hr;
    }


    //
    // enumerate the streams on the call
    //

    IEnumStream *pEnumStreams = NULL;
    
    hr = pStreamControl->EnumerateStreams(&pEnumStreams);
    
    pStreamControl->Release();
    pStreamControl = NULL;

    if (FAILED(hr))
    {
        LogError("CreateAndSelectMST: failed to enumerate streams on call");
        
        return hr;
    }

    
    //
    // walk through the list of streams on the call
    // for the first incoming audio stream, create a media streaming terminal
    // and select it on the stream
    //
  
    BOOL bTerminalCreatedAndSelected = FALSE;

    while (!bTerminalCreatedAndSelected)
    {
        
        ITStream *pStream = NULL;

        hr = pEnumStreams->Next(1, &pStream, NULL);

        if (S_OK != hr)
        {
            //
            // no more streams or error
            //

            LogError("CreateAndSelectMST: didn't find an incoming audio stream."
                     " terminal not selected.");

            break;
        }


        //
        // create and select media streaming terminal on the first incoming
        // audio stream
        //

        if ( IsRenderingStream(pStream) && IsAudioStream(pStream))
        {

            LogMessage("CreateAndSelectMST: creating mst");

            //
            // create media streaming terminal and select it on the stream
            //
            
            ITTerminal *pRecordTerminal = NULL;

            pRecordTerminal = CreateRenderMediaStreamingTerminal(g_pCurrentCall);

            if (NULL != pRecordTerminal)
            {

                hr = SelectAndInitializeTerminal(pRecordTerminal, pStream);

                pRecordTerminal->Release();
                pRecordTerminal = NULL;


                if (SUCCEEDED(hr))
                {
                    //
                    // set the flag, so we can break out of the loop
                    //

                    bTerminalCreatedAndSelected = TRUE;
                }

            } // media streaming terminal created successfully

        } // stream is rendering and audio
        else
        {
            // 
            // the stream is of wrong direction, or type
            //

        }

        pStream->Release();
        pStream = NULL;

    } // while (enumerating streams on the call)


    //
    // done with the stream enumeration. release
    //

    pEnumStreams->Release();
    pEnumStreams = NULL;

    
    if (bTerminalCreatedAndSelected)
    {
        LogMessage("CreateAndSelectMST: terminal selected");

        return S_OK;
    }
    else
    {
        LogMessage("CreateAndSelectMST: no terminal selected");

        return S_FALSE;
    }

}


///////////////////////////////////////////////////////////////////////////////
//
// GetTerminalFromMediaEvent
//
//
// get the terminal selected on the event's stream
//
///////////////////////////////////////////////////////////////////////////////

HRESULT GetTerminalFromMediaEvent(IN ITCallMediaEvent *pCallMediaEvent,
                                  OUT ITTerminal **ppTerminal)
{

    HRESULT hr = E_FAIL;


    //
    // don't return garbage if we fail
    //

    *ppTerminal = NULL;


    //
    // get the stream corresponding to this event
    //

    ITStream *pStream = NULL;

    hr = pCallMediaEvent->get_Stream(&pStream);

    if ( FAILED(hr) )
    {
        LogError("GetTerminalFromMediaEvent: "
                 "failed to get stream hr = 0x%lx", hr);

        return hr;
    }


    //
    // find the terminal on this stream 
    //

    //
    // get terminal enumeration on the stream
    //
    
    IEnumTerminal *pEnumTerminal = NULL;

    hr = pStream->EnumerateTerminals(&pEnumTerminal);

    pStream->Release();
    pStream = NULL;


    if ( FAILED(hr) )
    {
        LogError("GetTerminalFromMediaEvent: failed to enumerate terminals, "
            "hr = 0x%lx", hr);
        
        return hr;

    }

    
    //
    // walk through terminal enumeration
    //

    ULONG nTerminalsFetched = 0;
    
    ITTerminal *pStreamTerminal = NULL;


    //
    // assuming there is at most one terminal selected on the stream
    //

    hr = pEnumTerminal->Next(1, &pStreamTerminal, &nTerminalsFetched);

    pEnumTerminal->Release();
    pEnumTerminal = NULL;

    if (S_OK != hr)
    {
        LogError("GetTerminalFromMediaEvent: enumeration returned no "
                 "terminals, hr = 0x%lx", hr);
        
        return hr;

    }

    *ppTerminal = pStreamTerminal;

    
    LogMessage("GetTerminalFromMediaEvent: succeeded");
    
    return S_OK;
}


///////////////////////////////////////////////////////////////////////////////
//
// GetNumberOfSamplesOnStream
//
// read the terminal's allocator properties to return the number of samples
// the terminal provides
//
///////////////////////////////////////////////////////////////////////////////

HRESULT GetNumberOfSamplesOnStream(IN IMediaStream *pTerminalMediaStream,
                                   IN OUT DWORD *pnNumberOfSamples)
{

    HRESULT hr = S_OK;


    //
    // don't return garbage
    //
       
    *pnNumberOfSamples = 0;


    //
    // get allocator properties
    //

    ITAllocatorProperties *pAllocProperites = NULL;

    hr = pTerminalMediaStream->QueryInterface(IID_ITAllocatorProperties,
                                        (void **)&pAllocProperites);

    if (FAILED(hr))
    {
        LogError("GetNumberOfSamplesOnStream: "
                 "Failed to QI terminal for ITAllocatorProperties");

        return hr;
    }

    
    //
    // we want to know the number of samples we will be getting
    //

    ALLOCATOR_PROPERTIES AllocProperties;

    hr = pAllocProperites->GetAllocatorProperties(&AllocProperties);

    pAllocProperites->Release();
    pAllocProperites = NULL;


    if (FAILED(hr))
    {
        LogError("GetNumberOfSamplesOnStream: "
                 "Failed to get terminal's allocator properties");
        
        return hr;
    }


    *pnNumberOfSamples = AllocProperties.cBuffers;


    //
    // log the number of buffers and their sizes
    //

    LogMessage("GetNumberOfSamplesOnStream: [%ld] samples, [%ld] bytes each",
               *pnNumberOfSamples, AllocProperties.cbBuffer);


    return S_OK;
}


///////////////////////////////////////////////////////////////////////////////
//
// ReleaseEvents
//
//
// close the handles passed into the function and release the array of handles
//
///////////////////////////////////////////////////////////////////////////////

void ReleaseEvents(IN OUT HANDLE *pEvents,   // array of events to be freed
                   IN DWORD nNumberOfEvents  // number of events in the array
                   )
{

    //
    // close all the handles in the array
    //

    for (DWORD i = 0; i < nNumberOfEvents; i++)
    {
        CloseHandle(pEvents[i]);
        pEvents[i] = NULL;
    }

    
    //
    // free the array itself
    //

    FreeMemory(pEvents);
    pEvents = NULL;
}


///////////////////////////////////////////////////////////////////////////////
//
// AllocateEvents
//
// allocate the array of events of size nNumberOfSamples
//
// return pointer to the allocated and initialized array if success
// or NULL if failed
//
///////////////////////////////////////////////////////////////////////////////

HANDLE *AllocateEvents(IN DWORD nNumberOfEvents)
{

    //
    // pointer to an array of event handles
    //

    HANDLE *pSampleReadyEvents = NULL;

    pSampleReadyEvents = 
        (HANDLE*)AllocateMemory(sizeof(HANDLE) * nNumberOfEvents);

    if (NULL == pSampleReadyEvents)
    {
        LogError("AllocateEvents: Failed to allocate sample ready events.");
        
        return NULL;
    }


    //
    // create an event for every allocated handle
    //

    
    for (DWORD i = 0; i < nNumberOfEvents; i++)
    {

        pSampleReadyEvents[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
        
        if (NULL == pSampleReadyEvents[i])
        {

            LogError("AllocateEvents: "
                     "Failed to create event for sample %d", i);


            //
            // close handles we have created already
            //

            for (DWORD j = 0; j< i; j++)
            {
                CloseHandle(pSampleReadyEvents[j]);

                pSampleReadyEvents[j] = NULL;
            }


            FreeMemory(pSampleReadyEvents);
            pSampleReadyEvents = NULL;

            return NULL;

        }

    } // creating events for each sample

    
    //
    // succeded creating events. return the pointer to the array
    //

    return pSampleReadyEvents;

}


///////////////////////////////////////////////////////////////////////////////
//
// ReleaseSamples
//
// aborts and releases every sample in the array of samples of size 
// nNumberOfSamples and deallocates the array itself
//
// ppStreamSamples becomes invalid when the function returns
//
///////////////////////////////////////////////////////////////////////////////

void ReleaseSamples(IN OUT IStreamSample **ppStreamSamples,
                    IN DWORD nNumberOfSamples)
{

    for (DWORD i = 0; i < nNumberOfSamples; i++)
    {
        ppStreamSamples[i]->CompletionStatus(COMPSTAT_WAIT |
                                             COMPSTAT_ABORT,
                                             INFINITE);

        
        //
        // regardless of the error code, release the sample
        //

        ppStreamSamples[i]->Release();
        ppStreamSamples[i] = NULL;
    }

    FreeMemory(ppStreamSamples);
    ppStreamSamples = NULL;
}


///////////////////////////////////////////////////////////////////////////////
//
// AllocateStreamSamples
//
// allocate the array of nNumberOfSamples samples, and initialize each sample 
// pointer in the array with samples from the supplied stream.
//
// return pointer to the allocated and initialized array if success
// or NULL if failed
//
///////////////////////////////////////////////////////////////////////////////

IStreamSample **AllocateStreamSamples(IN IMediaStream *pMediaStream,
                                      IN DWORD nNumberOfSamples)
{

    //
    // allocate stream sample array
    //

    IStreamSample **ppStreamSamples = (IStreamSample **)
        AllocateMemory( sizeof(IStreamSample*) * nNumberOfSamples );


    if (NULL == ppStreamSamples)
    {
        LogError("AllocateStreamSamples: Failed to allocate stream sample array");

        return NULL;
    }


    //
    // allocate samples from the stream and put them into the array
    // 

    for (DWORD i = 0; i < nNumberOfSamples; i++)
    {

        HRESULT hr = pMediaStream->AllocateSample(0, &ppStreamSamples[i]);

        if (FAILED(hr))
        {

            LogError("AllocateStreamSamples: Failed to allocateSample. "
                     "Sample #%d", i);

            for (DWORD j = 0; j < i; j++)
            {
                ppStreamSamples[j]->Release();
                ppStreamSamples[j] = NULL;
            }

            FreeMemory(ppStreamSamples);
            ppStreamSamples = NULL;

            return NULL;
            
        } // failed AllocateSample()

    } // allocating samples on the stream



    //
    // succeeded allocating samples
    //

    return ppStreamSamples;

}


///////////////////////////////////////////////////////////////////////////////
//
// AssociateEventsWithSamples
//
// call Update() on every sample of the array of stream samples to associate it
// with an event from the array of events. The events will be signaled when the
// corresponding sample has data and is ready to be written to a file
//
///////////////////////////////////////////////////////////////////////////////

HRESULT AssociateEventsWithSamples(IN HANDLE *pSampleReadyEvents,
                                   IN IStreamSample **ppStreamSamples,
                                   IN DWORD nNumberOfSamples)
{


    for (DWORD i = 0; i < nNumberOfSamples; i++)
    {

        //
        // the event passed to Update will be signaled when the sample is 
        // filled with data
        //

        HRESULT hr = 
            ppStreamSamples[i]->Update(0, pSampleReadyEvents[i], NULL, 0);


        if (FAILED(hr))
        {
            
            LogError("AssociateEventsWithSamples: "
                     "Failed to call update on sample #%d", i);


            //
            // abort the samples we have Update()'d
            //

            for (DWORD j = 0; j < i; j++)
            {
      
                //
                // no need to check the return code here -- best effort attempt
                // if failed -- too bad
                //

                ppStreamSamples[j]->CompletionStatus(COMPSTAT_WAIT |
                                                     COMPSTAT_ABORT,
                                                     INFINITE);
            }

            return hr;

        } // Update() failed

    } // Update()'ing all samples


    return S_OK;
}


///////////////////////////////////////////////////////////////////////////////
//
// GetAudioFormat
//
// return a pointer to wave format structure for the audio data produced by 
// the stream. the caller is responsible for deallocating returned stucture
//
// returns NULL if failed
// 
//////////////////////////////////////////////////////////////////////////////

WAVEFORMATEX *GetAudioFormat(IMediaStream *pTerminalMediaStream)
{


    //
    // get ITAMMediaFormat interface on the terminal, so we can query for
    // audio format
    //

    ITAMMediaFormat *pITMediaFormat = NULL;

    HRESULT hr = pTerminalMediaStream->QueryInterface(IID_ITAMMediaFormat,
                                                     (void **)&pITMediaFormat);

    if (FAILED(hr))
    {
        LogError("GetAudioFormat: "
                 "failed to QI terminal for ITAMMediaFormat");
 
        return NULL;
    }


    //
    // use ITAMMediaFormat to get terminal's media format
    //

    AM_MEDIA_TYPE *pMediaType = NULL;

    hr = pITMediaFormat->get_MediaFormat(&pMediaType);

    pITMediaFormat->Release();
    pITMediaFormat = NULL;


    if (FAILED(hr))
    {
        LogError("GetAudioFormat: failed to get_MediaFormat hr = 0x%lx", hr);

        return NULL;
    }


    //
    // did we get back a format that we can use?
    //

    if ((pMediaType->pbFormat == NULL) ||
        pMediaType->formattype != FORMAT_WaveFormatEx)
    {

        LogError("GetAudioFormat: invalid format");

        DeleteMediaType(pMediaType);
        pMediaType = NULL;

        return NULL;
    }

    // 
    // allocate and return wave format
    //

    WAVEFORMATEX *pFormat = 
        (WAVEFORMATEX *)AllocateMemory(pMediaType->cbFormat);

    if (NULL != pFormat)
    {
        CopyMemory(pFormat, pMediaType->pbFormat, pMediaType->cbFormat);
    }
    else
    {
        LogError("GetAudioFormat: Failed to allocate memory for audio format");
    }


    //
    // remember to release AM_MEDIA_TYPE that we no longer need
    //

    DeleteMediaType(pMediaType);
    pMediaType = NULL;

    return pFormat;

}


///////////////////////////////////////////////////////////////////////////////
//
// WriteSampleToFile
//
// This function writes the data portion of the sample into the file
//
///////////////////////////////////////////////////////////////////////////////

HRESULT WriteSampleToFile(IN IStreamSample *pStreamSample, // sample to record
                          IN CAVIFileWriter *pFileWriter)  // file to record to
{

    //
    // get the sample's IMemoryData interface so we can get to the 
    // sample's data
    //

    IMemoryData *pSampleMemoryData = NULL;

    HRESULT hr = pStreamSample->QueryInterface(IID_IMemoryData,
                                               (void **)&pSampleMemoryData);

    if (FAILED(hr))
    {

        LogError("WriteSampleToFile: "
                 "Failed to qi sample for IMemoryData");

        return hr;
    }


    //
    // get to the sample's data buffer
    //

    DWORD nBufferSize = 0;

    BYTE *pnDataBuffer = NULL;

    DWORD nActualDataSize = 0;


    hr = pSampleMemoryData->GetInfo(&nBufferSize,
                                    &pnDataBuffer,
                                    &nActualDataSize);


    pSampleMemoryData->Release();
    pSampleMemoryData = NULL;


    if (FAILED(hr))
    {
        LogError("WriteSampleToFile: "
                 "Failed to get to the sample's data");

        return hr;
    }


    //
    // write the data buffer to the avi file
    //

    LogMessage("WriteSampleToFile: received a sample of size %ld bytes",
                nActualDataSize);


    ULONG nBytesWritten = 0;

    hr = pFileWriter->Write(pnDataBuffer,
                            nActualDataSize,
                            &nBytesWritten);

    if (FAILED(hr) || (0 == nBytesWritten))
    {
        LogError("WriteSampleToFile: FileWriter.Write() wrote no data.");

        return E_FAIL;
    }


    return hr;
}


///////////////////////////////////////////////////////////////////////////////
//
// GetSampleID
//
// given the return code fom WaitForMultipleObjects, this function determines
// which sample was signal and returns S_OK and the id of the signaled sample
// or E_FAIL if WaitForMultipleEvents returned an error
//
///////////////////////////////////////////////////////////////////////////////

HRESULT GetSampleID(IN DWORD nWaitCode,         // code from WaitForMultiple...
                    IN DWORD nNumberOfSamples,  // the total number of samples
                    IN OUT DWORD *pnSampleID)   // the calculated id of the 
                                                //          signaled sample
{


    //
    // event abandoned?
    //

    if ( (nWaitCode >= WAIT_ABANDONED_0) && 
         (nWaitCode < WAIT_ABANDONED_0 + nNumberOfSamples) )
    {

        LogError("GetSampleID: event for sample #%lu abandoned.", 
                 nWaitCode - WAIT_ABANDONED_0);

        return E_FAIL;
    }


    //
    // any other error?
    //

    if ( (WAIT_OBJECT_0 > nWaitCode) || 
         (WAIT_OBJECT_0 + nNumberOfSamples <= nWaitCode) )
    {
        LogMessage("GetSampleID: "
                   "waiting for samples failed or timed out. "
                   "WaitForMultipleObjects returned %lu", nWaitCode);

        return E_FAIL;
    }


    //
    // which sample was signaled?
    //

    *pnSampleID = nWaitCode - WAIT_OBJECT_0;

    return S_OK;
}


///////////////////////////////////////////////////////////////////////////////
//
// WriteStreamToFile
//
// extract samples from the terminal's media stream and write them into a file
//
// returns when the call is disconnected (call disconnect causes media streaming
// terminal to abort the samples
//
///////////////////////////////////////////////////////////////////////////////

HRESULT WriteStreamToFile(IN IMediaStream *pTerminalMediaStream)
{

    LogMessage("WriteStreamToFile: started");


    HRESULT hr = E_FAIL;


    //
    // get the number of stream samples we will be using
    //

    DWORD nNumberOfSamples = 0;

    hr = GetNumberOfSamplesOnStream(pTerminalMediaStream, &nNumberOfSamples);

    if (FAILED(hr))
    {
        LogError("WriteStreamToFile: failed to get the number of samples");

        return hr;
    }


    //
    // the number of samples directly corresponds the number of events we will 
    // be waiting on later. WaitForMultipleObjects has a limit of 
    // MAXIMUM_WAIT_OBJECTS events.
    //

    if (nNumberOfSamples > MAXIMUM_WAIT_OBJECTS)
    {

        LogError("WriteStreamToFile: the number of samples [%ld] "
                 "exceeds the number allowed by the design of this "
                 "application [%ld]", 
                 nNumberOfSamples, MAXIMUM_WAIT_OBJECTS);

        return E_FAIL;

    }

   
    //
    // allocate events that will be signaled when each sample is ready to be 
    // saved to a file
    //

    HANDLE *pSampleReadyEvents = NULL;

    pSampleReadyEvents = AllocateEvents(nNumberOfSamples);

    if (NULL == pSampleReadyEvents)
    {
        LogError("WriteStreamToFile: Failed to allocate sample ready events.");
        
        return E_OUTOFMEMORY;
    }


    //
    // allocate array of stream samples
    //
   
    IStreamSample **ppStreamSamples = NULL;

    ppStreamSamples = AllocateStreamSamples(pTerminalMediaStream, 
                                            nNumberOfSamples);

    if (NULL == ppStreamSamples)
    {

        LogError("WriteStreamToFile: Failed to allocate stream sample array");


        //
        // release events we have allocated
        //

        ReleaseEvents(pSampleReadyEvents, nNumberOfSamples);
        pSampleReadyEvents = NULL;

        return E_FAIL;
    }


    //
    // we have the samples, we have the events. 
    // associate events with samples so events get signaled when the 
    // corresponding samples are ready to be written to a file
    // 

    hr = AssociateEventsWithSamples(pSampleReadyEvents,
                                    ppStreamSamples,
                                    nNumberOfSamples);

    if (FAILED(hr))
    {
        LogError("WriteStreamToFile: Failed to associate events with samples");


        //
        // release events and samples we have allocated
        //

        ReleaseEvents(pSampleReadyEvents, nNumberOfSamples);
        pSampleReadyEvents = NULL;

        ReleaseSamples(ppStreamSamples, nNumberOfSamples);
        ppStreamSamples = NULL;

        return E_FAIL;
    }

    
    //
    // get the format of the data delivered by media streaming terminal
    //

    WAVEFORMATEX *pAudioFormat = NULL;

    pAudioFormat = GetAudioFormat(pTerminalMediaStream);

    if (NULL == pAudioFormat)
    {
        LogError("WriteStreamToFile: Failed to get audio format");


        //
        // release events and samples we have allocated
        //

        ReleaseEvents(pSampleReadyEvents, nNumberOfSamples);
        pSampleReadyEvents = NULL;

        ReleaseSamples(ppStreamSamples, nNumberOfSamples);
        ppStreamSamples = NULL;

        return E_FAIL;

    }


    //
    // create a file with the required name and format.
    //
   
    CAVIFileWriter FileWriter;

    hr = FileWriter.Initialize(SZ_OUTPUTFILENAME, *pAudioFormat);


    //
    // no longer need audio format
    //

    FreeMemory(pAudioFormat);
    pAudioFormat = NULL;


    if (FAILED(hr))
    {
        LogError("WriteStreamToFile: open file");


        //
        // release events and samples we have allocated
        //

        ReleaseEvents(pSampleReadyEvents, nNumberOfSamples);
        pSampleReadyEvents = NULL;

        ReleaseSamples(ppStreamSamples, nNumberOfSamples);
        ppStreamSamples = NULL;

        return hr;

    }


    //
    // just for logging, count the number of samples we have recorded
    //

    ULONG nStreamSamplesRecorded = 0;


    while(TRUE)
    {

        //
        // wait for the events associated with the samples
        // when a samples has data, the corresponding event will be 
        // signaled
        //
    
        DWORD nWaitCode = WaitForMultipleObjects(nNumberOfSamples,
                                                 pSampleReadyEvents,
                                                 FALSE,
                                                 INFINITE);

        
        //
        // get the id of the sample that was signaled. fail if Wait returned
        // error
        // 

        DWORD nSampleID = 0;

        hr = GetSampleID(nWaitCode, nNumberOfSamples, &nSampleID);

        if (FAILED(hr))
        {
            LogError("WriteStreamToFile: wait failed");

            break;
        }


        //
        // we filtered out all invalid error codes. so nSampleID has no 
        // choice but be a valid sample index.
        //

        _ASSERTE(nSampleID < nNumberOfSamples);


        //
        // make sure the sample is ready to be read
        //

        hr = ppStreamSamples[nSampleID]->CompletionStatus(COMPSTAT_WAIT, 0);

    
        //
        // check against S_OK explicitly -- not all success codes mean the
        // sample is ready to be used (MS_S_ENDOFSTREAM, etc)
        //

        if (S_OK != hr)
        {

            if (E_ABORT == hr)
            {
        
                //
                // recording was aborted, probably because 
                // the call was disconnected
                //

                LogMessage("WriteStreamToFile: recording aborted");
            }
            else
            {

                LogMessage("WriteStreamToFile: sample is not completed. "
                            "hr = 0x%lx", hr);
            }

            break;
        }

        
        //
        // we have the sample that was signaled and which is now ready to be
        // saved to a file. Record the sample.
        //

        hr = WriteSampleToFile(ppStreamSamples[nSampleID], &FileWriter);

        if (FAILED(hr))
        {
            LogError("WriteStreamToFile: failed to write sample to file");

            break;
        }


        //
        // one more sample was recorded. update the count.
        //

        nStreamSamplesRecorded++;


        //
        // we are done with this sample. return it to the source stream
        // to be refilled with data
        //

        hr = ppStreamSamples[nSampleID]->Update(0,
                                               pSampleReadyEvents[nSampleID],
                                               NULL,
                                               0);

        if (FAILED(hr))
        {
            LogError("WriteStreamToFile: "
                     "Failed to Update the sample recorded. "
                     "hr = 0x%lx", hr);

            break;
        }

    } // sample-writing loop
 

    LogMessage("WriteStreamToFile: wrote the total of %lu samples", 
                nStreamSamplesRecorded);


    //
    // release samples and events
    //

    ReleaseSamples(ppStreamSamples, nNumberOfSamples);
    ppStreamSamples = NULL;

    ReleaseEvents(pSampleReadyEvents, nNumberOfSamples);
    pSampleReadyEvents = NULL;


    //
    // deallocated samples and events. safe to exit.
    //

    LogMessage("WriteStreamToFile: completed, hr = 0x%lx", hr);

 
    return hr;
}


///////////////////////////////////////////////////////////////////////////////
//
// RecordMessage
//
// record the terminal's stream into a file
//
///////////////////////////////////////////////////////////////////////////////

HRESULT RecordMessage(IN ITTerminal *pRecordTerm)
{

    LogMessage("RecordMessage: started");

    HRESULT hr = E_FAIL;


    //
    // get IMediaStream interface on the terminal
    //
    
    IMediaStream *pTerminalMediaStream = NULL;

    hr = pRecordTerm->QueryInterface(IID_IMediaStream, 
                                     (void**)&pTerminalMediaStream);

    if (FAILED(hr))
    {
        LogError("RecordMessage: Failed to qi terminal for IMediaStream.");

        return hr;
    }

    
    //
    // write terminal stream data to a file
    //
    
    hr = WriteStreamToFile(pTerminalMediaStream);

    
    //
    // done with the terminal stream, release.
    //

    pTerminalMediaStream->Release();
    pTerminalMediaStream = NULL;


    LogMessage("RecordMessage: finished");

    return hr;

}


///////////////////////////////////////////////////////////////////////////////
//
// ProcessCallNotificationEvent
//
// processing for TE_CALLNOTIFICATION event
//
///////////////////////////////////////////////////////////////////////////////

HRESULT ProcessCallNotificationEvent(IDispatch *pEvent)
{

    HRESULT hr = E_FAIL;


    //
    // we are being notified of a new call
    //
    // if we own the call and there is not other active call
    // consider this to be the active call
    //
    // wait for CS_OFFERING message before answering the call
    //

    ITCallNotificationEvent *pCallNotificationEvent = NULL;

    hr = pEvent->QueryInterface( IID_ITCallNotificationEvent, 
                                (void **)&pCallNotificationEvent);

    if (FAILED(hr))
    {
        LogError("ProcessCallNotificationEvent: "
                 "Failed to QI event for ITCallNotificationEvent");

        return hr;
    }
   

    //
    // get the call from notification event
    //

    ITCallInfo *pCall = NULL;

    hr = pCallNotificationEvent->get_Call(&pCall);

    //
    // release the ITCallNotificationEvent interface
    //

    pCallNotificationEvent->Release();
    pCallNotificationEvent = NULL;


    if (FAILED(hr))
    {
        LogError("ProcessCallNotificationEvent: "
                 "Failed to get call from Call notification event");


        return hr;
    }

    
    //
    // if we already have an active call, reject the new incoming call
    //

    EnterCriticalSection(&g_CurrentCallCritSection);

    if (NULL != g_pCurrentCall)
    {

        LeaveCriticalSection(&g_CurrentCallCritSection);

        LogMessage("ProcessCallNotificationEvent: "
                   "incoming call while another call in progress");

        
        ITBasicCallControl *pSecondCall = NULL;

        hr = pCall->QueryInterface(IID_ITBasicCallControl, 
                                   (void**)&pSecondCall);

        pCall->Release();
        pCall = NULL;


        if (FAILED(hr))
        {
            LogError("ProcessCallNotificationEvent: failed to qi incoming call "
                     "for ITBasicCallConrtrol");

            return hr;
        }


        //
        // reject the incoming call
        //

        LogMessage("ProcessCallNotificationEvent: rejecting the incoming call");
        

        hr = pSecondCall->Disconnect(DC_REJECTED);

        pSecondCall->Release();
        pSecondCall = NULL;

        return E_FAIL;
    }


    //
    // check to see if we own the call
    //

    CALL_PRIVILEGE cp;
   
    hr = pCall->get_Privilege( &cp );


    if ( FAILED(hr) )
    {
        LogError("ProcessCallNotificationEvent: Failed to get call owner info");

        pCall->Release();
        pCall = NULL;

        LeaveCriticalSection(&g_CurrentCallCritSection);

        return hr;
    }

    if ( CP_OWNER != cp )
    {

        //
        // we don't own the call. ignore it.
        //

        LogMessage("ProcessCallNotificationEvent: We don't own the call.");

        pCall->Release();
        pCall = NULL;

        LeaveCriticalSection(&g_CurrentCallCritSection);

        return E_FAIL;
    }
    else
    {
        LogMessage("ProcessCallNotificationEvent: Incoming call.");
    }


    //
    // keep the call for future use
    // 

    hr = pCall->QueryInterface(IID_ITBasicCallControl, 
                               (void**)&g_pCurrentCall);


    if (FAILED(hr))
    {
        LogError("ProcessCallNotificationEvent: failed to qi incoming call for"
                  "ITBasicCallControl.");
        
        g_pCurrentCall = NULL;
    }
    
    LeaveCriticalSection(&g_CurrentCallCritSection);

    pCall->Release();
    pCall = NULL;


    return hr;

} // ProcessCallNotificationEvent


///////////////////////////////////////////////////////////////////////////////
//
// ProcessCallMediaEvent
//
// processing for TE_CALLMEDIA event. if stream is active, record the 
// incoming data
//
///////////////////////////////////////////////////////////////////////////////

HRESULT ProcessCallMediaEvent(IDispatch *pEvent)
{

    //
    // Get ITCallMediaEvent interface from the event
    //

    ITCallMediaEvent *pCallMediaEvent = NULL;

    HRESULT hr = pEvent->QueryInterface( IID_ITCallMediaEvent, 
                                 (void **)&pCallMediaEvent );

    if (FAILED(hr))
    {
    
        //
        // the event does not have the interface we want
        //

        LogError("ProcessCallMediaEvent: TE_CALLMEDIA. "
                 "Failed to QI event for ITCallMediaEvent");

        return hr;
    }


    //
    // get the CALL_MEDIA_EVENT that we are being notified of.
    //

    CALL_MEDIA_EVENT CallMediaEvent;

    hr = pCallMediaEvent->get_Event(&CallMediaEvent);

    if ( FAILED(hr) )
    {

        //
        // failed to get call media event 
        //
        
        LogError("ProcessCallMediaEvent: TE_CALLMEDIA. "
                 "Failed to get call media event hr = 0x%lx", hr);
        
        pCallMediaEvent->Release();
        pCallMediaEvent = NULL;

        return hr;
    }


    LogMessage("ProcessCallMediaEvent: processing call media event");

    switch (CallMediaEvent) 
    {

        case CME_STREAM_INACTIVE:
        {

            LogMessage("ProcessCallMediaEvent: CME_STREAM_INACTIVE");

            break;
        }

        case CME_STREAM_NOT_USED:
            
            LogMessage("ProcessCallMediaEvent: CME_STREAM_NOT_USED");
            
            break;

        case CME_NEW_STREAM:

            LogMessage("ProcessCallMediaEvent: CME_NEW_STREAM received");

            break;

        case CME_STREAM_FAIL:

            LogError("ProcessCallMediaEvent: CME_STREAM_FAIL received");
            
            break;

        case CME_TERMINAL_FAIL:
            
            LogError("ProcessCallMediaEvent: CME_STREAM_FAIL received");

            break;

        case CME_STREAM_ACTIVE:
        {

            LogError("ProcessCallMediaEvent: CME_STREAM_ACTIVE received");


            //
            // Get the terminal on the active stream
            //    

            ITTerminal *pRecordStreamTerminal = NULL;

            hr = GetTerminalFromMediaEvent(pCallMediaEvent, 
                                           &pRecordStreamTerminal);

            if ( FAILED(hr) )
            {
                // 
                // the stream has no terminals associated with it
                //

                LogError("ProcessCallMediaEvent: "
                         "failed to get terminal on the active stream");

                break; 
            }

          
            //
            // make sure the direction is right -- we are recording, so the 
            // terminal should be TD_RENDER
            //

            TERMINAL_DIRECTION td;

            hr = pRecordStreamTerminal->get_Direction( &td);

            if ( FAILED(hr) ) 
            {

                LogError("ProcessCallMediaEvent: "
                         "failed to get record terminal's direction.");

                pRecordStreamTerminal->Release();
                pRecordStreamTerminal = NULL;

                break; 
            }
            
            
            //
            // double check that the terminal is rendering terminal
            // since we are the recording side
            //

            if ( TD_RENDER != td ) 
            {

                //
                // this should never ever happen
                //

                LogError("ProcessCallMediaEvent: bad terminal direction");

                pRecordStreamTerminal->Release();
                pRecordStreamTerminal = NULL;

                hr = E_FAIL;

                break;

            }

            //
            // Now do the actual streaming.
            //

            //
            // this will block until:
            //
            // the call is disconnected, or
            // the user chooses to close the app, or
            // there is an error
            //
            // Since we are in the message processing thread,
            // the application will not be able to process messages
            // (the messages will still be queued) until this call
            // returns.
            //
            // If it is important that messages are processed while
            // file is being recorded, recording should be done on 
            // a different thread.
            // 

            RecordMessage(pRecordStreamTerminal);
        
            pRecordStreamTerminal->Release();
            pRecordStreamTerminal = NULL;
    
            break;
        }
    
        default:

            break;
    
    } // switch (call media event)


    //
    // We no longer need the event interface.
    //

    pCallMediaEvent->Release();
    pCallMediaEvent = NULL;

    
    return S_OK;    

}


///////////////////////////////////////////////////////////////////////////////
//
// ProcessCallStateEvent
//
// processing for TE_CALLSTATE. if CS_OFFERING, creates and selects MST, 
// answers the call . Release call if disconnected. 
//
// also verifies that the event is for the current call
//
///////////////////////////////////////////////////////////////////////////////

HRESULT ProcessCallStateEvent(IDispatch *pEvent)
{

    HRESULT hr = S_OK;


    //
    // TE_CALLSTATE is a call state event.  
    // pEvent is an ITCallStateEvent
    // 

    ITCallStateEvent *pCallStateEvent = NULL;


    //
    // Get the interface
    //

    hr = pEvent->QueryInterface(IID_ITCallStateEvent, 
                                (void **)&pCallStateEvent);

    if ( FAILED(hr) )
    {

        LogError("ProcessCallStateEvent: "
                 "Failed to QI event for ITCallStateEvent");

        return hr;
    }


    //
    // make sure the message is for the active call!
    //

    EnterCriticalSection(&g_CurrentCallCritSection);

    if (!IsMessageForActiveCall(pCallStateEvent))
    {
        LogMessage("ProcessCallStateEvent: received TE_CALLSTATE message "
                   "for a different call. ignoring.");

        pCallStateEvent->Release();
        pCallStateEvent = NULL;

        LeaveCriticalSection(&g_CurrentCallCritSection);

        return E_FAIL;
    }


    //
    // get the CallState that we are being notified of.
    //

    CALL_STATE CallState;

    hr = pCallStateEvent->get_State(&CallState);


    //
    // no longer need the event
    //

    pCallStateEvent->Release();
    pCallStateEvent = NULL;


    if (FAILED(hr))
    {
         LogError("ProcessCallStateEvent: "
                  "failed to get state from call state event.");

         LeaveCriticalSection(&g_CurrentCallCritSection);

         return hr;
    }



    //
    // log the new call state
    //

    if (CS_OFFERING == CallState)
    {
        
        LogMessage("ProcessCallStateEvent: call state is CS_OFFERING");


        //
        // try to create and select media streaming terminal on one of
        // the call's incoming audio streams
        //

        hr = CreateAndSelectMST();

        if (S_OK == hr)
        {
        
            //
            // we have selected a terminal on one of the streams
            // answer the call
            //

            LogMessage("ProcessCallStateEvent: answering the call");

            hr = g_pCurrentCall->Answer();

            if (FAILED(hr))
            {
                LogError("ProcessCallStateEvent: Failed to answer the call");
            }

        }
        else
        {
            
            //
            // we could not create mst on any of the streams of 
            // the incoming call. reject the call.
            //

            LogMessage("ProcessCallStateEvent: rejecting the call");

            HRESULT hr2 = g_pCurrentCall->Disconnect(DC_REJECTED);

            if (FAILED(hr2))
            {
                LogError("ProcessCallStateEvent: Failed to reject the call");
            }

        }


        
    } // CS_OFFERING
    else if (CS_DISCONNECTED == CallState)
    {

        LogMessage("ProcessCallStateEvent: call state is CS_DISCONNECTED. "
                   "Releasing the call.");


        //
        // release the call -- no longer need it!
        //

        g_pCurrentCall->Release();
        g_pCurrentCall = NULL;


        //
        // signal the main thread that it can exit if it is time to exit
        //

        LogMessage("ProcessCallStateEvent: signaling main thread.");

        SetEvent(g_hExitEvent);

    } // CS_DISCONNECTED
    else if (CS_CONNECTED == CallState)
    {
        
        LogMessage("ProcessCallStateEvent: call state is CS_CONNECTED");

    } // CS_CONNECTED


    //
    // no longer need to protect current call
    //

    LeaveCriticalSection(&g_CurrentCallCritSection);

    return hr;

}



/////////////////////////////////////////////////////////////////////////////
//
// OnTapiEvent
//
// handler for tapi events. called on a thread that provides asychronous
// processing of tapi messages
//
/////////////////////////////////////////////////////////////////////////////

HRESULT OnTapiEvent(TAPI_EVENT TapiEvent, IDispatch *pEvent)
{
    
    LogMessage("OnTapiEvent: message received");


    HRESULT hr = S_OK;


    switch ( TapiEvent )
    {
    
        case TE_CALLNOTIFICATION:
        {

            LogMessage("OnTapiEvent: received TE_CALLNOTIFICATION");

            hr = ProcessCallNotificationEvent(pEvent);

            break;

        } // TE_CALLNOTIFICATION
        

        case TE_CALLSTATE:
        {
            LogMessage("OnTapiEvent: received TE_CALLSTATE");

            hr = ProcessCallStateEvent(pEvent);

            break;

        } // TE_CALLSTATE


        case TE_CALLMEDIA:
        {

            LogMessage("OnTapiEvent: received TE_CALLMEDIA");

            hr = ProcessCallMediaEvent(pEvent);

            break;    
        
        } // case TE_CALLMEDIA


    default:

        LogMessage("OnTapiEvent: received message %d. not handled.", TapiEvent);

        break;

    }


    //
    // the event was AddRef()'ed when it was posted for asynchronous processing
    // no longer need the event, release it.
    //

    pEvent->Release(); 
    pEvent = NULL;

    LogMessage("OnTapiEvent: exiting.");

    return hr;
}