// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
//
//  A very simple "Chat" client - reads samples from the default console device and discards the output.
//

#include "StdAfx.h"
#include "WasapiChat.h"

CWasapiChat::CWasapiChat(HWND AppWindow) : CChatTransport(AppWindow),
    _ChatEndpoint(NULL),
    _AudioClient(NULL),
    _RenderClient(NULL),
    _CaptureClient(NULL),
    _Flow(eRender),
    _ChatThread(NULL),
    _ShutdownEvent(NULL),
    _AudioSamplesReadyEvent(NULL)
{
}

CWasapiChat::~CWasapiChat(void) 
{
    SafeRelease(&_ChatEndpoint);
    SafeRelease(&_AudioClient);
    SafeRelease(&_RenderClient);
    SafeRelease(&_CaptureClient);

    if (_ChatThread)
    {
        CloseHandle(_ChatThread);
    }
    if (_ShutdownEvent)
    {
        CloseHandle(_ShutdownEvent);
    }
    if (_AudioSamplesReadyEvent)
    {
        CloseHandle(_AudioSamplesReadyEvent);
    }
}
//
//  We can "Chat" if there's more than one capture device.
//
bool CWasapiChat::Initialize(bool UseInputDevice)
{
    IMMDeviceEnumerator *deviceEnumerator;
    HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&deviceEnumerator));
    if (FAILED(hr))
    {
        MessageBox(_AppWindow, L"Unable to instantiate device enumerator", L"WASAPI Transport Initialize Failure", MB_OK);
        return false;
    }

    if (UseInputDevice)
    {
        _Flow = eCapture;
    }
    else
    {
        _Flow = eRender;
    }

    hr = deviceEnumerator->GetDefaultAudioEndpoint(_Flow, eCommunications, &_ChatEndpoint);
    deviceEnumerator->Release();
    if (FAILED(hr))
    {
        MessageBox(_AppWindow, L"Unable to retrieve default endpoint", L"WASAPI Transport Initialize Failure", MB_OK);
        return false;
    }
    //
    //  Create our shutdown event - we want an auto reset event that starts in the not-signaled state.
    //
    _ShutdownEvent = CreateEventEx(NULL, NULL, 0, EVENT_MODIFY_STATE | SYNCHRONIZE);
    if (_ShutdownEvent == NULL)
    {
        MessageBox(_AppWindow, L"Unable to create shutdown event.", L"WASAPI Transport Initialize Failure", MB_OK);
        return false;
    }

    _AudioSamplesReadyEvent = CreateEventEx(NULL, NULL, 0, EVENT_MODIFY_STATE | SYNCHRONIZE);
    if (_ShutdownEvent == NULL)
    {
        MessageBox(_AppWindow, L"Unable to create samples ready event.", L"WASAPI Transport Initialize Failure", MB_OK);
        return false;
    }

    return true;
}

//
//  Shut down the chat code and free all the resources.
//
void CWasapiChat::Shutdown()
{
    if (_ChatThread)
    {
        SetEvent(_ShutdownEvent);
        WaitForSingleObject(_ChatThread, INFINITE);
        CloseHandle(_ChatThread);
        _ChatThread = NULL;
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
    SafeRelease(&_ChatEndpoint);
    SafeRelease(&_AudioClient);
    SafeRelease(&_RenderClient);
    SafeRelease(&_CaptureClient);
}


//
//  Start the "Chat" - open the capture device, start capturing.
//
bool CWasapiChat::StartChat(bool HideFromVolumeMixer)
{
    WAVEFORMATEX *mixFormat = NULL;
    HRESULT hr = _ChatEndpoint->Activate(__uuidof(IAudioClient), CLSCTX_INPROC_SERVER, NULL, reinterpret_cast<void **>(&_AudioClient));
    if (FAILED(hr))
    {
        MessageBox(_AppWindow, L"Unable to activate audio client.", L"WASAPI Transport Start Failure", MB_OK);
        return false;
    }
    hr = _AudioClient->GetMixFormat(&mixFormat);
    if (FAILED(hr))
    {
        MessageBox(_AppWindow, L"Unable to get mix format on audio client.", L"WASAPI Transport Start Failure", MB_OK);
        return false;
    }

    //
    //  Initialize the chat transport - Initialize WASAPI in event driven mode, associate the audio client with
    //  our samples ready event handle, retrieve a capture/render client for the transport, create the chat thread
    //  and start the audio engine.
    //
    GUID chatGuid;
    hr = CoCreateGuid(&chatGuid);
    if (FAILED(hr))
    {
        MessageBox(_AppWindow, L"Unable to create GUID.", L"WASAPI Transport Start Failure", MB_OK);
        return false;
    }

    hr = _AudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, (HideFromVolumeMixer ? AUDCLNT_SESSIONFLAGS_DISPLAY_HIDE : 0) | AUDCLNT_STREAMFLAGS_EVENTCALLBACK | AUDCLNT_STREAMFLAGS_NOPERSIST, 500000, 0, mixFormat, &chatGuid);
    CoTaskMemFree(mixFormat);
    mixFormat = NULL;

    if (FAILED(hr))
    {
        MessageBox(_AppWindow, L"Unable to initialize audio client.", L"WASAPI Transport Start Failure", MB_OK);
        return false;
    }

    hr = _AudioClient->SetEventHandle(_AudioSamplesReadyEvent);
    if (FAILED(hr))
    {
        MessageBox(_AppWindow, L"Unable to set ready event.", L"WASAPI Transport Start Failure", MB_OK);
        return false;
    }

    if (_Flow == eRender)
    {
        hr = _AudioClient->GetService(IID_PPV_ARGS(&_RenderClient));
    }
    else
    {
        hr = _AudioClient->GetService(IID_PPV_ARGS(&_CaptureClient));
    }
    if (FAILED(hr))
    {
        MessageBox(_AppWindow, L"Unable to get Capture/Render client.", L"WASAPI Transport Start Failure", MB_OK);
        return false;
    }

    //
    //  Now create the thread which is going to drive the "Chat".
    //
    _ChatThread = CreateThread(NULL, 0, WasapiChatThread, this, 0, NULL);
    if (_ChatThread == NULL)
    {
        MessageBox(_AppWindow, L"Unable to create transport thread.", L"WASAPI Transport Start Failure", MB_OK);
        return false;
    }

    //
    //  For render, we want to pre-roll a frames worth of silence into the pipeline.  That way the audio engine won't glitch on startup.
    //
    if (_Flow == eRender)
    {
        BYTE *pData;
        UINT32 framesAvailable;
        hr = _AudioClient->GetBufferSize(&framesAvailable);
        if (FAILED(hr))
        {
            MessageBox(_AppWindow, L"Failed to get client buffer size.", L"WASAPI Transport Start Failure", MB_OK);
            return false;
        }
        hr = _RenderClient->GetBuffer(framesAvailable, &pData);
        if (FAILED(hr))
        {
            MessageBox(_AppWindow, L"Failed to get buffer.", L"WASAPI Transport Start Failure", MB_OK);
            return false;
        }
        hr = _RenderClient->ReleaseBuffer(framesAvailable, AUDCLNT_BUFFERFLAGS_SILENT);
        if (FAILED(hr))
        {
            MessageBox(_AppWindow, L"Failed to release buffer.", L"WASAPI Transport Start Failure", MB_OK);
            return false;
        }
    }

    //
    //  We're ready to go, start the chat!
    //
    hr = _AudioClient->Start();
    if (FAILED(hr))
    {
        MessageBox(_AppWindow, L"Unable to start chat client.", L"WASAPI Transport Start Failure", MB_OK);
        return false;
    }

    return true;
}

//
//  Stop the "Chat" - Stop the capture thread and release the buffers.
//
void CWasapiChat::StopChat()
{
    //
    //  Tell the chat thread to shut down, wait for the thread to complete then clean up all the stuff we 
    //  allocated in StartChat().
    //
    if (_ShutdownEvent)
    {
        SetEvent(_ShutdownEvent);
    }
    if (_ChatThread)
    {
        WaitForSingleObject(_ChatThread, INFINITE);

        CloseHandle(_ChatThread);
        _ChatThread = NULL;
    }

    SafeRelease(&_RenderClient);
    SafeRelease(&_CaptureClient);
    SafeRelease(&_AudioClient);
}


DWORD CWasapiChat::WasapiChatThread(LPVOID Context)
{
    bool stillPlaying = true;
    CWasapiChat *chat = static_cast<CWasapiChat *>(Context);
    HANDLE waitArray[2] = {chat->_ShutdownEvent, chat->_AudioSamplesReadyEvent};

    while (stillPlaying)
    {
        HRESULT hr;
        DWORD waitResult = WaitForMultipleObjects(2, waitArray, FALSE, INFINITE);
        switch (waitResult)
        {
        case WAIT_OBJECT_0 + 0:
            stillPlaying = false;       // We're done, exit the loop.
            break;
        case WAIT_OBJECT_0 + 1:
            //
            //  Either stream silence to the audio client or ignore the audio samples.
            //
            //  Note that we don't check for errors here.  This is because 
            //      (a) there's no way of reporting the failure
            //      (b) once the streaming engine has started there's really no way for it to fail.
            //
            if (chat->_Flow == eRender)
            {
                BYTE *pData;
                UINT32 framesAvailable;
                hr = chat->_AudioClient->GetCurrentPadding(&framesAvailable);
                hr = chat->_RenderClient->GetBuffer(framesAvailable, &pData);
                hr = chat->_RenderClient->ReleaseBuffer(framesAvailable, AUDCLNT_BUFFERFLAGS_SILENT);
            }
            else
            {
                BYTE *pData;
                UINT32 framesAvailable;
                DWORD flags;
                hr = chat->_AudioClient->GetCurrentPadding(&framesAvailable);
                hr = chat->_CaptureClient->GetBuffer(&pData, &framesAvailable, &flags, NULL, NULL);
                hr = chat->_CaptureClient->ReleaseBuffer(framesAvailable);
            }
        }
    }
    return 0;
}

//
//  Returns "true" for the window messages we handle in our transport.
//
bool CWasapiChat::HandlesMessage(HWND /*hWnd*/, UINT /*message*/)
{
    return false;
}


//
//  Don't process Wave messages.  
//
INT_PTR CWasapiChat::MessageHandler(HWND /*hWnd*/, UINT /*message*/, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
    return FALSE;
}