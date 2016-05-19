// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
#pragma once
#include "ChatTransport.h"
#include <MMDeviceAPI.h>
#include <AudioClient.h>

class CWasapiChat :
    public CChatTransport
{
    IMMDevice * _ChatEndpoint;
    IAudioClient *_AudioClient;
    IAudioRenderClient *_RenderClient;
    IAudioCaptureClient *_CaptureClient;
    EDataFlow _Flow;

    HANDLE _ChatThread;
    HANDLE _ShutdownEvent;
    HANDLE _AudioSamplesReadyEvent;

    static DWORD __stdcall WasapiChatThread(LPVOID Context);
public:
    CWasapiChat(HWND AppWindow);
    bool Initialize(bool UseInputDevice);
    void Shutdown();
    virtual ~CWasapiChat(void);
    bool StartChat(bool HideFromVolumeMixer);
    void StopChat();
    bool HandlesMessage(HWND, UINT);
    bool CanStartChat();
    ChatTransportType TransportType() { return ChatTransportWasapi; }
    INT_PTR CALLBACK MessageHandler(HWND, UINT, WPARAM, LPARAM);
};
