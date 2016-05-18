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
#include "WaveChat.h"

CWaveChat::CWaveChat(HWND AppWindow) : CChatTransport(AppWindow),
    _waveHandle(NULL),
    _waveBuffer1(NULL),
    _waveBuffer2(NULL)
{
    ZeroMemory(&_waveHeader1, sizeof(_waveHeader1));
    ZeroMemory(&_waveHeader2, sizeof(_waveHeader2));
}

CWaveChat::~CWaveChat(void)
{
}
//
//  We can "Chat" if there's more than one wave input device.
//
bool CWaveChat::Initialize(bool UseInputDevice)
{
    if (!UseInputDevice)
    {
        MessageBox(_AppWindow, L"Wave Chat can only run on the input device", L"Failed to initialize chat", MB_OK);
        return false;
    }
    if (waveInGetNumDevs() == 0)
    {
        MessageBox(NULL, L"No Capture Devices found", L"Failed to initialize chat", MB_OK);
        return false;
    }
    return true;
}

//
//  Shut down the chat code and free all the resources.
//
void CWaveChat::Shutdown()
{
    if (_waveHandle)
    {
        MMRESULT mmr = waveInStop(_waveHandle);
        if (mmr != MMSYSERR_NOERROR)
        {
            MessageBox(NULL, L"Failed to start", L"Error", MB_OK);
        }

        mmr = waveInReset(_waveHandle);
        if (mmr != MMSYSERR_NOERROR)
        {
            MessageBox(NULL, L"Failed to reset", L"Error", MB_OK);
        }

        mmr = waveInUnprepareHeader(_waveHandle, &_waveHeader1, sizeof(_waveHeader1));
        if (mmr != MMSYSERR_NOERROR)
        {
            MessageBox(NULL, L"Failed to unprepare wave header 1", L"Error", MB_OK);
        }
        delete []_waveBuffer1;

        mmr = waveInUnprepareHeader(_waveHandle, &_waveHeader2, sizeof(_waveHeader2));
        if (mmr != MMSYSERR_NOERROR)
        {
            MessageBox(NULL, L"Failed to unprepare wave header 2", L"Error", MB_OK);
        }
        delete []_waveBuffer2;

        mmr = waveInClose(_waveHandle);
        if (mmr != MMSYSERR_NOERROR)
        {
            MessageBox(NULL, L"Failed to close wave handle", L"Error", MB_OK);
        }
        _waveHandle = NULL;
    }
}
#ifndef WAVE_MAPPED_DEFAULT_COMMUNICATION_DEVICE
#define  WAVE_MAPPED_DEFAULT_COMMUNICATION_DEVICE   0x0010
#endif


//
//  Start the "Chat" - open the wave in device, prepare two capture buffers for reading.
//
bool CWaveChat::StartChat(bool HideFromVolumeMixer)
{
    WAVEFORMATEX waveFormat = {0};
    waveFormat.cbSize = 0;
    waveFormat.nSamplesPerSec = 44100;
    waveFormat.nChannels = 2;
    waveFormat.wBitsPerSample = 16;
    waveFormat.nBlockAlign = (waveFormat.wBitsPerSample / 8)*waveFormat.nChannels;
    waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
    waveFormat.wFormatTag = WAVE_FORMAT_PCM;

    MMRESULT mmr = waveInOpen(&_waveHandle, WAVE_MAPPER, &waveFormat,
                      reinterpret_cast<DWORD_PTR>(_AppWindow), NULL,
                      CALLBACK_WINDOW | WAVE_MAPPED_DEFAULT_COMMUNICATION_DEVICE);
    if (mmr != MMSYSERR_NOERROR)
    {
        MessageBox(_AppWindow, L"Failed to open wave in", L"Error", MB_OK);
        return false;
    }

    ZeroMemory(&_waveHeader1, sizeof(_waveHeader1));
    _waveBuffer1 = new (std::nothrow) WORD[waveFormat.nAvgBytesPerSec];
    if (_waveBuffer1 == NULL)
    {
        MessageBox(_AppWindow, L"Failed to allocate buffer for header 1", L"Error", MB_OK);
        return false;
    }
    _waveHeader1.dwBufferLength = waveFormat.nAvgBytesPerSec;
    _waveHeader1.lpData = reinterpret_cast<LPSTR>(_waveBuffer1);


    mmr = waveInPrepareHeader(_waveHandle, &_waveHeader1, sizeof(_waveHeader1));
    if (mmr != MMSYSERR_NOERROR)
    {
        MessageBox(_AppWindow, L"Failed to prepare header 1", L"Error", MB_OK);
        return false;
    }

    mmr = waveInAddBuffer(_waveHandle, &_waveHeader1, sizeof(_waveHeader1));
    if (mmr != MMSYSERR_NOERROR)
    {
        MessageBox(_AppWindow, L"Failed to add buffer 1", L"Error", MB_OK);
        return false;
    }

    ZeroMemory(&_waveHeader2, sizeof(_waveHeader2));
    _waveBuffer2 = new (std::nothrow) WORD[waveFormat.nAvgBytesPerSec];
    if (_waveBuffer2 == NULL)
    {
        MessageBox(_AppWindow, L"Failed to allocate buffer for header 2", L"Error", MB_OK);
        return false;
    }
    _waveHeader2.dwBufferLength = waveFormat.nAvgBytesPerSec;
    _waveHeader2.lpData = reinterpret_cast<LPSTR>(_waveBuffer2);

    mmr = waveInPrepareHeader(_waveHandle, &_waveHeader2, sizeof(_waveHeader2));
    if (mmr != MMSYSERR_NOERROR)
    {
        MessageBox(NULL, L"Failed to prepare header 2", L"Error", MB_OK);
        return false;
    }

    mmr = waveInAddBuffer(_waveHandle, &_waveHeader2, sizeof(_waveHeader2));
    if (mmr != MMSYSERR_NOERROR)
    {
        MessageBox(_AppWindow, L"Failed to add buffer 2", L"Error", MB_OK);
        return false;
    }

    mmr = waveInStart(_waveHandle);
    if (mmr != MMSYSERR_NOERROR)
    {
        MessageBox(_AppWindow, L"Failed to start", L"Error", MB_OK);
        return false;
    }
    return true;
}

//
//  Stop the "Chat" - Stop the wave in device, reset it (which removes the pending capture buffers), unprepare the headers, and free the buffers.
//
void CWaveChat::StopChat()
{
    if (_waveHandle)
    {
        MMRESULT mmr = waveInStop(_waveHandle);
        if (mmr != MMSYSERR_NOERROR)
        {
            MessageBox(NULL, L"Failed to start", L"Error", MB_OK);
        }

        mmr = waveInReset(_waveHandle);
        if (mmr != MMSYSERR_NOERROR)
        {
            MessageBox(NULL, L"Failed to reset", L"Error", MB_OK);
        }
        mmr = waveInUnprepareHeader(_waveHandle, &_waveHeader1, sizeof(_waveHeader1));
        if (mmr != MMSYSERR_NOERROR)
        {
            MessageBox(NULL, L"Failed to unprepare wave header 1", L"Error", MB_OK);
        }
        delete []_waveBuffer1;

        mmr = waveInUnprepareHeader(_waveHandle, &_waveHeader2, sizeof(_waveHeader2));
        if (mmr != MMSYSERR_NOERROR)
        {
            MessageBox(NULL, L"Failed to unprepare wave header 2", L"Error", MB_OK);
        }
        delete []_waveBuffer2;

        mmr = waveInClose(_waveHandle);
        if (mmr != MMSYSERR_NOERROR)
        {
            MessageBox(NULL, L"Failed to close wave handle", L"Error", MB_OK);
        }
        _waveHandle = NULL;
    }
}

//
//  Returns "true" for the window messages we handle in our transport.
//
bool CWaveChat::HandlesMessage(HWND /*hWnd*/, UINT message)
{
    switch (message)
    {
    case MM_WIM_OPEN:
    case MM_WIM_CLOSE:
    case MM_WIM_DATA:
        return true;
    }
    return false;
}


//
//  Process Wave messages.  
//
//  We ignore all the wave messages except the "Data" message.
//
INT_PTR CWaveChat::MessageHandler(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case MM_WIM_OPEN:
        return TRUE;
    case MM_WIM_CLOSE:
        return TRUE;
    case MM_WIM_DATA:
        {
            //
            //  Process the capture data since we've received a buffers worth of data.
            //
            //  In real life, we'd copy the capture data out of the waveHeader that just completed and process it, but since
            //  this is a sample, we discard the data and simply re-submit the buffer.
            //
            MMRESULT mmr;
            HWAVEIN waveHandle = reinterpret_cast<HWAVEIN>(wParam);
            LPWAVEHDR waveHeader = reinterpret_cast<LPWAVEHDR>(lParam);
            if (_waveHandle)
            {
                mmr = waveInAddBuffer(waveHandle, waveHeader, sizeof(WAVEHDR));
                if (mmr != MMSYSERR_NOERROR)
                {
                    MessageBox(hWnd, L"Failed to add buffer", L"Error", MB_OK);
                }
            }
            return TRUE;
        }
    }
    return FALSE;
}