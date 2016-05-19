// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
#pragma once
#include "ChatTransport.h"
#include <mmsystem.h>

class CWaveChat :
    public CChatTransport
{
    HWAVEIN _waveHandle;
    WAVEHDR _waveHeader1;
    WORD *  _waveBuffer1;
    WAVEHDR _waveHeader2;
    WORD *  _waveBuffer2;
public:
    CWaveChat(HWND AppWindow);
    bool Initialize(bool UseInputDevice);
    void Shutdown();
    virtual ~CWaveChat(void);
    bool StartChat(bool HideFromVolumeMixer);
    void StopChat();
    bool HandlesMessage(HWND, UINT);
    bool CanStartChat();
    ChatTransportType TransportType() { return ChatTransportWave; }
    INT_PTR CALLBACK MessageHandler(HWND, UINT, WPARAM, LPARAM);
};
