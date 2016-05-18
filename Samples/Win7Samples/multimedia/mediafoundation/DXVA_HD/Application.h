//////////////////////////////////////////////////////////////////////
//
// Application.h
// 
// Contains most of the application logic. 
// 
// WinMain creates an instance of this class. This class runs the
// message loop for the application window.
// 
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//////////////////////////////////////////////////////////////////////

#pragma once

class Application
{
    enum ApplicationMode
    {
        Mode1 = VK_F1,
        Mode2 = VK_F2,
        Mode3 = VK_F3,
        Mode4 = VK_F4,
        Mode5 = VK_F5,
        Mode6 = VK_F6,
        Mode7 = VK_F7,
        Mode8 = VK_F8,
        Mode9 = VK_F9
    };

    HWND                m_Hwnd;     // Application window
    DwmHelper           m_DWM;      // DWM functions
    D3DHelper           m_D3D;      // Manages the Direct3D device
    Timer               m_timer;    // Timer for running the render loop.

    ApplicationMode     m_mode;     // Current mode for user interaction.

    DXVAHD_DEVICE_USAGE m_usage;    // DXVA-HD device usage (command-line parameter)

    // DXVA-HD interfaces
    IDXVAHD_Device          *m_pDXVAHD;
    IDXVAHD_VideoProcessor  *m_pDXVAVP;
    IDirect3DSurface9       *m_pMainStream;
    IDirect3DSurface9*      m_ppSubStream[SUB_STREAM_COUNT];

    // Current settings
    WORD    m_PlanarAlphaValue;
    BYTE    m_PixelAlphaValue;

    RECT    m_rcMainVideoSourceRect;    // Source rectangle for the main video image.
    RECT    m_rcMainVideoDestRect;      // Destination rectangle for the main video image.
    RECT    m_rcVideoSubRect;           // Source rectangle for the substream image.
    
    // Note: The Substream destination rectangle is calculated per frame, so we do not
    // store it in a member variable.

    // Values for calculating the target rectangle, as a percentage of the 
    // destination surface.
    INT     m_TargetWidthPercent;       
    INT     m_TargetHeightPercent;      

    // Color information.
    INT     m_iExtendedColor;       // Index into the EX_COLOR_INFO array.
    INT     m_iBackgroundColor;     // Index into the BACKGROUND_COLORS array.

    // Proc amp filtering information

    struct ProcAmpInfo
    {
        BOOL                        bSupported;
        DXVAHD_FILTER_RANGE_DATA    Range;
        INT                         CurrentValue;
        INT                         Step;
    };

    ProcAmpInfo m_Filters[ NUM_FILTERS ];

public:
    Application() :
        m_pDXVAHD(NULL),
        m_pDXVAVP(NULL),
        m_pMainStream(NULL),
        m_PlanarAlphaValue(DEFAULT_PLANAR_ALPHA_VALUE),
        m_PixelAlphaValue(DEFAULT_PIXEL_ALPHA_VALUE),
        m_mode(Mode1),
        m_usage(DXVAHD_DEVICE_USAGE_PLAYBACK_NORMAL)
    {
        ZeroMemory(m_ppSubStream, sizeof( m_ppSubStream) );

        ZeroMemory(&m_Filters, sizeof(m_Filters));

        ResetSettings();
    }

    ~Application()
    {
        DestroyDXVAHD();
    }

    BOOL    Initialize();
    INT     MessageLoop();
    LRESULT HandleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    BOOL    InitializeDXVAHD();
    void    DestroyDXVAHD();

    BOOL    ProcessVideoFrame();
    BOOL    ResetDevice(BOOL bChangeWindowMode = FALSE);

    BOOL    ParseCommandLine();
    BOOL    PreTranslateMessage(const MSG& msg);
    void    OnKeyDown(UINT vk, BOOL fDown, int cRepeat, UINT flags);
    void    OnSize();

    HRESULT UpdateVideoSubRect();
    HRESULT ApplySettings();
    HRESULT ResetSettings();
    HRESULT AdjustSetting(int dx, int dy);
    HRESULT AdjustAlphaSetting(int dx, int dy);
    HRESULT AdjustExtendedColor(int dy);
    HRESULT AdjustBackgroundColor(int dx);
    HRESULT AdjustFilter(DXVAHD_FILTER filter, int dy);
    HRESULT AdjustTargetRect(int dx, int dy);
        
    void    CalculateSubstreamRect(INT frame, RECT *prect);
};