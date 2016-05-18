// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
#pragma once

//
//  Pure virtual class which defines a "Chat Transport"
//
class CChatTransport
{
protected:
    HWND    _AppWindow;
public:
    CChatTransport(HWND hWnd) : _AppWindow(hWnd)
    {
    }
    virtual ~CChatTransport(void) {};
    enum ChatTransportType
    {
        ChatTransportWave,
        ChatTransportWasapi
    };
    virtual bool Initialize(bool UseCaptureDevice) = 0;
    virtual void Shutdown() = 0;
    virtual bool StartChat(bool HideFromVolumeMixer) = 0;
    virtual void StopChat() = 0;
    virtual ChatTransportType TransportType() = 0;
    virtual bool HandlesMessage(HWND hWnd, UINT message) = 0;
    virtual INT_PTR CALLBACK MessageHandler(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) = 0;
};
