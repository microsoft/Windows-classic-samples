// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.


/*++

Copyright (c) 2006  Microsoft Corporation

Module Name:

    dxva2vp.cpp

Abstract:

    This sample code demonstrates DXVA2 Video Processor API.

Environment:

    Windows XP or later.

Command line options:

    -hh : Force to use hardware D3D9 device and hardware DXVA2 device.
    -hs : Force to use hardware D3D9 device and software DXVA2 device.
    -ss : Force to use software D3D9 device and software DXVA2 device.

Keyboard assignment:

    ESC or Alt + F4 : Exit.

    Alt + Enter : Mode change between Window mode and Fullscreen mode.

    F1 - F8 : Sub stream's color is changed and enters in the following modes.

    HOME : Reset to the default/initial values in each modes.

    END : Enable/Disable the frame drop debug spew.

    F1 : [WHITE] Increment and decrement alpha values.

        UP   : increment main and sub stream's planar alpha
        DOWN : decrement main and sub stream's planar alpha
        RIGHT: increment sub stream's pixel alpha
        LEFT : decrement sub stream's pixel alpha

    F2 : [RED] Increment and decrement main stream's source area.

        UP   : decrement in the vertical direction (zoom in)
        DOWN : increment in the vertical direction (zoom out)
        RIGHT: increment in the horizontal direction (zoom in)
        LEFT : decrement in the horizontal direction (zoom out)

    F3 : [YELLOW] Move main stream's source area.

        UP   : move up
        DOWN : move down
        RIGHT: move right
        LEFT : move left

    F4 : [GREEN] Increment and decrement main stream's destination area.

        UP   : increment in the vertical direction
        DOWN : decrement in the vertical direction
        RIGHT: increment in the horizontal direction
        LEFT : decrement in the horizontal direction

    F5 : [CYAN] Move main stream's destination area.

        UP   : move up
        DOWN : move down
        RIGHT: move right
        LEFT : move left

    F6 : [BLUE] Change background color or extended color information.

        UP   : change to the next extended color in EX_COLOR_INFO
        DOWN : change to the previous extended color in EX_COLOR_INFO
        RIGHT: change to the next background color in BACKGROUND_COLORS
        LEFT : change to the previous background color in BACKGROUND_COLORS

        EX_COLOR_INFO : SDTV ITU-R BT.601 YCbCr to driver's optimal RGB range,
                        SDTV ITU-R BT.601 YCbCr to studio RGB [16...235],
                        SDTV ITU-R BT.601 YCbCr to computer RGB [0...255],
                        HDTV ITU-R BT.709 YCbCr to driver's optimal RGB range,
                        HDTV ITU-R BT.709 YCbCr to studio RGB [16...235],
                        HDTV ITU-R BT.709 YCbCr to computer RGB [0...255]

        BACKGROUND_COLORS : WHITE, RED, YELLOW, GREEN, CYAN, BLUE, MAGENTA, BLACK

    F7 : [MAGENTA] Increment and decrement the brightness or contrast.

        UP   : increment brightness
        DOWN : decrement brightness
        RIGHT: increment contrast
        LEFT : decrement contrast

    F8 : [BLACK] Increment and decrement the hue or saturation.

        UP   : increment hue
        DOWN : decrement hue
        RIGHT: increment saturation
        LEFT : decrement saturation

    F9 : [ORANGE] Increment and decrement target area.

        UP   : decrement in the vertical direction
        DOWN : increment in the vertical direction
        RIGHT: increment in the horizontal direction
        LEFT : decrement in the horizontal direction

--*/

#include <windows.h>
#include <windowsx.h>
#include <dwmapi.h>
#include <mmsystem.h>
#include <tchar.h>
#include <strsafe.h>
#include <initguid.h>
#include <d3d9.h>
#include <dxva2api.h>


//
// Type definitions.
//

typedef HRESULT (WINAPI * PFNDWMISCOMPOSITIONENABLED)(
    __out BOOL* pfEnabled
    );

typedef HRESULT (WINAPI * PFNDWMGETCOMPOSITIONTIMINGINFO)(
    __in HWND hwnd,
    __out DWM_TIMING_INFO* pTimingInfo
    );

typedef HRESULT (WINAPI * PFNDWMSETPRESENTPARAMETERS)(
    __in HWND hwnd,
    __inout DWM_PRESENT_PARAMETERS* pPresentParams
    );


//
// Internal data.
//

const TCHAR CLASS_NAME[]  = TEXT("DXVA2 VP Sample Class");
const TCHAR WINDOW_NAME[] = TEXT("DXVA2 VP Sample Application");

const UINT VIDEO_MAIN_WIDTH  = 640;
const UINT VIDEO_MAIN_HEIGHT = 480;
const RECT VIDEO_MAIN_RECT   = {0, 0, VIDEO_MAIN_WIDTH, VIDEO_MAIN_HEIGHT};
const UINT VIDEO_SUB_WIDTH   = 128;
const UINT VIDEO_SUB_HEIGHT  = 128;
const RECT VIDEO_SUB_RECT    = {0, 0, VIDEO_SUB_WIDTH, VIDEO_SUB_HEIGHT};

const UINT VIDEO_SUB_VX  = 5;
const UINT VIDEO_SUB_VY  = 3;
const UINT VIDEO_FPS     = 60;
const UINT VIDEO_MSPF    = (1000 + VIDEO_FPS / 2) / VIDEO_FPS;
const UINT VIDEO_100NSPF = VIDEO_MSPF * 10000;

const BYTE DEFAULT_PLANAR_ALPHA_VALUE = 0xFF;
const BYTE DEFAULT_PIXEL_ALPHA_VALUE  = 0x80;

const UINT VIDEO_REQUIED_OP = DXVA2_VideoProcess_YUV2RGB |
                              DXVA2_VideoProcess_StretchX |
                              DXVA2_VideoProcess_StretchY |
                              DXVA2_VideoProcess_SubRects |
                              DXVA2_VideoProcess_SubStreams;

const D3DFORMAT VIDEO_RENDER_TARGET_FORMAT = D3DFMT_X8R8G8B8;
const D3DFORMAT VIDEO_MAIN_FORMAT          = D3DFMT_YUY2;
const D3DFORMAT VIDEO_SUB_FORMAT           = D3DFORMAT('VUYA'); // AYUV

const UINT BACK_BUFFER_COUNT = 1;
const UINT SUB_STREAM_COUNT  = 1;
const UINT DWM_BUFFER_COUNT  = 4;

const UINT EX_COLOR_INFO[][2] =
{
    // SDTV ITU-R BT.601 YCbCr to driver's optimal RGB range
    {DXVA2_VideoTransferMatrix_BT601, DXVA2_NominalRange_Unknown},
    // SDTV ITU-R BT.601 YCbCr to studio RGB [16...235]
    {DXVA2_VideoTransferMatrix_BT601, DXVA2_NominalRange_16_235},
    // SDTV ITU-R BT.601 YCbCr to computer RGB [0...255]
    {DXVA2_VideoTransferMatrix_BT601, DXVA2_NominalRange_0_255},
    // HDTV ITU-R BT.709 YCbCr to driver's optimal RGB range
    {DXVA2_VideoTransferMatrix_BT709, DXVA2_NominalRange_Unknown},
    // HDTV ITU-R BT.709 YCbCr to studio RGB [16...235]
    {DXVA2_VideoTransferMatrix_BT709, DXVA2_NominalRange_16_235},
    // HDTV ITU-R BT.709 YCbCr to computer RGB [0...255]
    {DXVA2_VideoTransferMatrix_BT709, DXVA2_NominalRange_0_255}
};


//
// Studio RGB [16...235] colors.
//

// 100%
const D3DCOLOR RGB_WHITE        = D3DCOLOR_XRGB(0xEB, 0xEB, 0xEB);
const D3DCOLOR RGB_RED          = D3DCOLOR_XRGB(0xEB, 0x10, 0x10);
const D3DCOLOR RGB_YELLOW       = D3DCOLOR_XRGB(0xEB, 0xEB, 0x10);
const D3DCOLOR RGB_GREEN        = D3DCOLOR_XRGB(0x10, 0xEB, 0x10);
const D3DCOLOR RGB_CYAN         = D3DCOLOR_XRGB(0x10, 0xEB, 0xEB);
const D3DCOLOR RGB_BLUE         = D3DCOLOR_XRGB(0x10, 0x10, 0xEB);
const D3DCOLOR RGB_MAGENTA      = D3DCOLOR_XRGB(0xEB, 0x10, 0xEB);
const D3DCOLOR RGB_BLACK        = D3DCOLOR_XRGB(0x10, 0x10, 0x10);
const D3DCOLOR RGB_ORANGE       = D3DCOLOR_XRGB(0xEB, 0x7E, 0x10);

// 75%
const D3DCOLOR RGB_WHITE_75pc   = D3DCOLOR_XRGB(0xB4, 0xB4, 0xB4);
const D3DCOLOR RGB_YELLOW_75pc  = D3DCOLOR_XRGB(0xB4, 0xB4, 0x10);
const D3DCOLOR RGB_CYAN_75pc    = D3DCOLOR_XRGB(0x10, 0xB4, 0xB4);
const D3DCOLOR RGB_GREEN_75pc   = D3DCOLOR_XRGB(0x10, 0xB4, 0x10);
const D3DCOLOR RGB_MAGENTA_75pc = D3DCOLOR_XRGB(0xB4, 0x10, 0xB4);
const D3DCOLOR RGB_RED_75pc     = D3DCOLOR_XRGB(0xB4, 0x10, 0x10);
const D3DCOLOR RGB_BLUE_75pc    = D3DCOLOR_XRGB(0x10, 0x10, 0xB4);

// -4% / +4%
const D3DCOLOR RGB_BLACK_n4pc   = D3DCOLOR_XRGB(0x07, 0x07, 0x07);
const D3DCOLOR RGB_BLACK_p4pc   = D3DCOLOR_XRGB(0x18, 0x18, 0x18);

// -Inphase / +Quadrature
const D3DCOLOR RGB_I            = D3DCOLOR_XRGB(0x00, 0x1D, 0x42);
const D3DCOLOR RGB_Q            = D3DCOLOR_XRGB(0x2C, 0x00, 0x5C);

const D3DCOLOR BACKGROUND_COLORS[] =
{
    RGB_WHITE, RGB_RED,  RGB_YELLOW,  RGB_GREEN,
    RGB_CYAN,  RGB_BLUE, RGB_MAGENTA, RGB_BLACK
};


//
// Global variables.
//

BOOL g_bD3D9HW  = TRUE;
BOOL g_bD3D9SW  = TRUE;
BOOL g_bDXVA2HW = TRUE;
BOOL g_bDXVA2SW = TRUE;

BOOL g_bWindowed        = TRUE;
BOOL g_bTimerSet        = FALSE;
BOOL g_bInModeChange    = FALSE;
BOOL g_bUpdateSubStream = FALSE;
BOOL g_bDspFrameDrop    = FALSE;
BOOL g_bDwmQueuing      = FALSE;

HWND    g_Hwnd         = NULL;
HANDLE  g_hTimer       = NULL;
HMODULE g_hRgb9rastDLL = NULL;
HMODULE g_hDwmApiDLL   = NULL;

PVOID g_pfnD3D9GetSWInfo               = NULL;
PVOID g_pfnDwmIsCompositionEnabled     = NULL;
PVOID g_pfnDwmGetCompositionTimingInfo = NULL;
PVOID g_pfnDwmSetPresentParameters     = NULL;

RECT g_RectWindow = {0};

DWORD g_StartSysTime = 0;
DWORD g_PreviousTime = 0;

UINT g_VK_Fx = VK_F1;

IDirect3D9*        g_pD3D9  = NULL;
IDirect3DDevice9*  g_pD3DD9 = NULL;
IDirect3DSurface9* g_pD3DRT = NULL;

D3DPRESENT_PARAMETERS g_D3DPP = {0};

IDirectXVideoProcessorService* g_pDXVAVPS = NULL;
IDirectXVideoProcessor*        g_pDXVAVPD = NULL;

IDirect3DSurface9* g_pMainStream = NULL;
IDirect3DSurface9* g_pSubStream  = NULL;

GUID                     g_GuidVP    = {0};
DXVA2_VideoDesc          g_VideoDesc = {0};
DXVA2_VideoProcessorCaps g_VPCaps    = {0};

INT g_BackgroundColor = 0;
INT g_ExColorInfo     = 0;
INT g_ProcAmpSteps[4] = {0};

WORD g_PlanarAlphaValue = DEFAULT_PLANAR_ALPHA_VALUE;
BYTE g_PixelAlphaValue  = DEFAULT_PIXEL_ALPHA_VALUE;

RECT g_SrcRect = VIDEO_MAIN_RECT;
RECT g_DstRect = VIDEO_MAIN_RECT;

DXVA2_ValueRange g_ProcAmpRanges[4] = {0};
DXVA2_ValueRange g_NFilterRanges[6] = {0};
DXVA2_ValueRange g_DFilterRanges[6] = {0};

DXVA2_Fixed32 g_ProcAmpValues[4] = {0};
DXVA2_Fixed32 g_NFilterValues[6] = {0};
DXVA2_Fixed32 g_DFilterValues[6] = {0};

UINT g_TargetWidthPercent  = 100;
UINT g_TargetHeightPercent = 100;

//
// Helper inline functions.
//
inline BOOL operator != (const DXVA2_ValueRange& x, const DXVA2_ValueRange& y)
{
    return memcmp(&x, &y, sizeof(x)) ? TRUE : FALSE;
}


//
// Debug Macro
//
#if defined(DBG) || defined(DEBUG) || defined(_DEBUG)

#define DBGMSG(x)  {DbgPrint(TEXT("%s(%u) : "), TEXT(__FILE__), __LINE__); DbgPrint x;}

VOID
DbgPrint(PCTSTR format, ...)
{
    va_list args;
    va_start(args, format);

    TCHAR string[MAX_PATH];

    if (SUCCEEDED(StringCbVPrintf(string, sizeof(string), format, args)))
    {
        OutputDebugString(string);
    }
    else
    {
        DebugBreak();
    }
}

#else   // DBG || DEBUG || _DEBUG

#define DBGMSG(x)

#endif  // DBG || DEBUG || _DEBUG


BOOL
RegisterSoftwareRasterizer()
{
    HRESULT hr;

    if (!g_hRgb9rastDLL)
    {
        return FALSE;
    }

    hr = g_pD3D9->RegisterSoftwareDevice(g_pfnD3D9GetSWInfo);

    if (FAILED(hr))
    {
        DBGMSG((TEXT("RegisterSoftwareDevice failed with error 0x%x.\n"), hr));
        return FALSE;
    }

    return TRUE;
}


BOOL
InitializeD3D9()
{
    HRESULT hr;

    g_pD3D9 = Direct3DCreate9(D3D_SDK_VERSION);

    if (!g_pD3D9)
    {
        DBGMSG((TEXT("Direct3DCreate9 failed.\n")));
        return FALSE;
    }

    //
    // Register the inbox software rasterizer if software D3D9 could be used.
    // CreateDevice(D3DDEVTYPE_SW) will fail if software device is not registered.
    //
    if (g_bD3D9SW)
    {
        RegisterSoftwareRasterizer();
    }

    if (g_bWindowed)
    {
        g_D3DPP.BackBufferWidth  = 0;
        g_D3DPP.BackBufferHeight = 0;
    }
    else
    {
        g_D3DPP.BackBufferWidth  = GetSystemMetrics(SM_CXSCREEN);
        g_D3DPP.BackBufferHeight = GetSystemMetrics(SM_CYSCREEN);
    }

    g_D3DPP.BackBufferFormat           = VIDEO_RENDER_TARGET_FORMAT;
    g_D3DPP.BackBufferCount            = BACK_BUFFER_COUNT;
    g_D3DPP.SwapEffect                 = D3DSWAPEFFECT_DISCARD;
    g_D3DPP.hDeviceWindow              = g_Hwnd;
    g_D3DPP.Windowed                   = g_bWindowed;
    g_D3DPP.Flags                      = D3DPRESENTFLAG_VIDEO;
    g_D3DPP.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
    g_D3DPP.PresentationInterval       = D3DPRESENT_INTERVAL_ONE;

    //
    // Mark the back buffer lockable if software DXVA2 could be used.
    // This is because software DXVA2 device requires a lockable render target
    // for the optimal performance.
    //
    if (g_bDXVA2SW)
    {
        g_D3DPP.Flags |= D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
    }

    //
    // First try to create a hardware D3D9 device.
    //
    if (g_bD3D9HW)
    {
        hr = g_pD3D9->CreateDevice(D3DADAPTER_DEFAULT,
                                   D3DDEVTYPE_HAL,
                                   g_Hwnd,
                                   D3DCREATE_FPU_PRESERVE |
                                   D3DCREATE_MULTITHREADED |
                                   D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                                   &g_D3DPP,
                                   &g_pD3DD9);

        if (FAILED(hr))
        {
            DBGMSG((TEXT("CreateDevice(HAL) failed with error 0x%x.\n"), hr));
        }
    }

    //
    // Next try to create a software D3D9 device.
    //
    if (!g_pD3DD9 && g_bD3D9SW)
    {
        hr = g_pD3D9->CreateDevice(D3DADAPTER_DEFAULT,
                                   D3DDEVTYPE_SW,
                                   g_Hwnd,
                                   D3DCREATE_FPU_PRESERVE |
                                   D3DCREATE_MULTITHREADED |
                                   D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                                   &g_D3DPP,
                                   &g_pD3DD9);

        if (FAILED(hr))
        {
            DBGMSG((TEXT("CreateDevice(SW) failed with error 0x%x.\n"), hr));
        }
    }

    if (!g_pD3DD9)
    {
        return FALSE;
    }

    return TRUE;
}


VOID
DestroyD3D9()
{
    if (g_pD3DD9)
    {
        g_pD3DD9->Release();
        g_pD3DD9 = NULL;
    }

    if (g_pD3D9)
    {
        g_pD3D9->Release();
        g_pD3D9 = NULL;
    }
}


INT
ComputeLongSteps(DXVA2_ValueRange &range)
{
    float f_step = DXVA2FixedToFloat(range.StepSize);

    if (f_step == 0.0)
    {
        return 0;
    }

    float f_max = DXVA2FixedToFloat(range.MaxValue);
    float f_min = DXVA2FixedToFloat(range.MinValue);
    INT steps = INT((f_max - f_min) / f_step / 32);

    return max(steps, 1);
}


BOOL
CreateDXVA2VPDevice(REFGUID guid)
{
    HRESULT hr;

    //
    // Query the supported render target format.
    //
    UINT i, count;
    D3DFORMAT* formats = NULL;

    hr = g_pDXVAVPS->GetVideoProcessorRenderTargets(guid,
                                                    &g_VideoDesc,
                                                    &count,
                                                    &formats);

    if (FAILED(hr))
    {
        DBGMSG((TEXT("GetVideoProcessorRenderTargets failed with error 0x%x.\n"), hr));
        return FALSE;
    }

    for (i = 0; i < count; i++)
    {
        if (formats[i] == VIDEO_RENDER_TARGET_FORMAT)
        {
            break;
        }
    }

    CoTaskMemFree(formats);

    if (i >= count)
    {
        DBGMSG((TEXT("GetVideoProcessorRenderTargets doesn't support that format.\n")));
        return FALSE;
    }

    //
    // Query the supported substream format.
    //
    formats = NULL;

    hr = g_pDXVAVPS->GetVideoProcessorSubStreamFormats(guid,
                                                       &g_VideoDesc,
                                                       VIDEO_RENDER_TARGET_FORMAT,
                                                       &count,
                                                       &formats);

    if (FAILED(hr))
    {
        DBGMSG((TEXT("GetVideoProcessorSubStreamFormats failed with error 0x%x.\n"), hr));
        return FALSE;
    }

    for (i = 0; i < count; i++)
    {
        if (formats[i] == VIDEO_SUB_FORMAT)
        {
            break;
        }
    }

    CoTaskMemFree(formats);

    if (i >= count)
    {
        DBGMSG((TEXT("GetVideoProcessorSubStreamFormats doesn't support that format.\n")));
        return FALSE;
    }

    //
    // Query video processor capabilities.
    //
    hr = g_pDXVAVPS->GetVideoProcessorCaps(guid,
                                           &g_VideoDesc,
                                           VIDEO_RENDER_TARGET_FORMAT,
                                           &g_VPCaps);

    if (FAILED(hr))
    {
        DBGMSG((TEXT("GetVideoProcessorCaps failed with error 0x%x.\n"), hr));
        return FALSE;
    }

    //
    // Check to see if the device is software device.
    //
    if (g_VPCaps.DeviceCaps & DXVA2_VPDev_SoftwareDevice)
    {
        if (!g_bDXVA2SW)
        {
            DBGMSG((TEXT("The DXVA2 device isn't a hardware device.\n")));
            return FALSE;
        }
    }
    else
    {
        if (!g_bDXVA2HW)
        {
            DBGMSG((TEXT("The DXVA2 device isn't a software device.\n")));
            return FALSE;
        }
    }

    //
    // This is a progressive device and we cannot provide any reference sample.
    //
    if (g_VPCaps.NumForwardRefSamples > 0 || g_VPCaps.NumBackwardRefSamples > 0)
    {
        DBGMSG((TEXT("NumForwardRefSamples or NumBackwardRefSamples is greater than 0.\n")));
        return FALSE;
    }

    //
    // Check to see if the device supports all the VP operations we want.
    //
    if ((g_VPCaps.VideoProcessorOperations & VIDEO_REQUIED_OP) != VIDEO_REQUIED_OP)
    {
        DBGMSG((TEXT("The DXVA2 device doesn't support the VP operations.\n")));
        return FALSE;
    }

    //
    // Create a main stream surface.
    //
    hr = g_pDXVAVPS->CreateSurface(VIDEO_MAIN_WIDTH,
                                   VIDEO_MAIN_HEIGHT,
                                   0,
                                   VIDEO_MAIN_FORMAT,
                                   g_VPCaps.InputPool,
                                   0,
                                   DXVA2_VideoSoftwareRenderTarget,
                                   &g_pMainStream,
                                   NULL);

    if (FAILED(hr))
    {
        DBGMSG((TEXT("CreateSurface(MainStream) failed with error 0x%x.\n"), hr));
        return FALSE;
    }

    //
    // Create a sub stream surface.
    //
    hr = g_pDXVAVPS->CreateSurface(VIDEO_SUB_WIDTH,
                                   VIDEO_SUB_HEIGHT,
                                   0,
                                   VIDEO_SUB_FORMAT,
                                   g_VPCaps.InputPool,
                                   0,
                                   DXVA2_VideoSoftwareRenderTarget,
                                   &g_pSubStream,
                                   NULL);

    if (FAILED(hr))
    {
        DBGMSG((TEXT("CreateSurface(SubStream, InputPool) failed with error 0x%x.\n"), hr));
    }

    //
    // Fallback to default pool type if the suggested pool type failed.
    //
    // This could happen when software DXVA2 device is used with hardware D3D9 device.
    // This is due to the fact that D3D9 doesn't understand AYUV format and D3D9 needs
    // an information from the driver in order to allocate it from the system memory.
    // Since software DXVA2 device suggests system memory pool type for the optimal
    // performance but the driver may not support it other than default pool type,
    // D3D9 will fail to create it.
    //
    // Note that creating a default pool type surface will significantly reduce the
    // performance when it is used with software DXVA2 device.
    //
    if (!g_pSubStream && g_VPCaps.InputPool != D3DPOOL_DEFAULT)
    {
        hr = g_pDXVAVPS->CreateSurface(VIDEO_SUB_WIDTH,
                                       VIDEO_SUB_HEIGHT,
                                       0,
                                       VIDEO_SUB_FORMAT,
                                       D3DPOOL_DEFAULT,
                                       0,
                                       DXVA2_VideoSoftwareRenderTarget,
                                       &g_pSubStream,
                                       NULL);

        if (FAILED(hr))
        {
            DBGMSG((TEXT("CreateSurface(SubStream, DEFAULT) failed with error 0x%x.\n"), hr));
        }
    }

    if (!g_pSubStream)
    {
        return FALSE;
    }

    //
    // Query ProcAmp ranges.
    //
    DXVA2_ValueRange range;

    for (i = 0; i < ARRAYSIZE(g_ProcAmpRanges); i++)
    {
        if (g_VPCaps.ProcAmpControlCaps & (1 << i))
        {
            hr = g_pDXVAVPS->GetProcAmpRange(guid,
                                             &g_VideoDesc,
                                             VIDEO_RENDER_TARGET_FORMAT,
                                             1 << i,
                                             &range);

            if (FAILED(hr))
            {
                DBGMSG((TEXT("GetProcAmpRange failed with error 0x%x.\n"), hr));
                return FALSE;
            }

            //
            // Reset to default value if the range is changed.
            //
            if (range != g_ProcAmpRanges[i])
            {
                g_ProcAmpRanges[i] = range;
                g_ProcAmpValues[i] = range.DefaultValue;
                g_ProcAmpSteps[i]  = ComputeLongSteps(range);
            }
        }
    }

    //
    // Query Noise Filter ranges.
    //
    if (g_VPCaps.VideoProcessorOperations & DXVA2_VideoProcess_NoiseFilter)
    {
        for (i = 0; i < ARRAYSIZE(g_NFilterRanges); i++)
        {
            hr = g_pDXVAVPS->GetFilterPropertyRange(guid,
                                                    &g_VideoDesc,
                                                    VIDEO_RENDER_TARGET_FORMAT,
                                                    DXVA2_NoiseFilterLumaLevel + i,
                                                    &range);

            if (FAILED(hr))
            {
                DBGMSG((TEXT("GetFilterPropertyRange(Noise) failed with error 0x%x.\n"), hr));
                return FALSE;
            }

            //
            // Reset to default value if the range is changed.
            //
            if (range != g_NFilterRanges[i])
            {
                g_NFilterRanges[i] = range;
                g_NFilterValues[i] = range.DefaultValue;
            }
        }
    }

    //
    // Query Detail Filter ranges.
    //
    if (g_VPCaps.VideoProcessorOperations & DXVA2_VideoProcess_DetailFilter)
    {
        for (i = 0; i < ARRAYSIZE(g_DFilterRanges); i++)
        {
            hr = g_pDXVAVPS->GetFilterPropertyRange(guid,
                                                    &g_VideoDesc,
                                                    VIDEO_RENDER_TARGET_FORMAT,
                                                    DXVA2_DetailFilterLumaLevel + i,
                                                    &range);

            if (FAILED(hr))
            {
                DBGMSG((TEXT("GetFilterPropertyRange(Detail) failed with error 0x%x.\n"), hr));
                return FALSE;
            }

            //
            // Reset to default value if the range is changed.
            //
            if (range != g_DFilterRanges[i])
            {
                g_DFilterRanges[i] = range;
                g_DFilterValues[i] = range.DefaultValue;
            }
        }
    }

    //
    // Finally create a video processor device.
    //
    hr = g_pDXVAVPS->CreateVideoProcessor(guid,
                                          &g_VideoDesc,
                                          VIDEO_RENDER_TARGET_FORMAT,
                                          SUB_STREAM_COUNT,
                                          &g_pDXVAVPD);

    if (FAILED(hr))
    {
        DBGMSG((TEXT("CreateVideoProcessor failed with error 0x%x.\n"), hr));
        return FALSE;
    }

    g_GuidVP = guid;

    return TRUE;
}


VOID
FillRectangle(
    D3DLOCKED_RECT& lr,
    const UINT sx,
    const UINT sy,
    const UINT ex,
    const UINT ey,
    const DWORD color)
{
    BYTE* p = (BYTE*) lr.pBits;

    p += lr.Pitch * sy;

    for (UINT y = sy; y < ey; y++, p += lr.Pitch)
    {
        for (UINT x = sx; x < ex; x++)
        {
            ((DWORD*) p)[x] = color;
        }
    }
}


DWORD
RGBtoYUV(const D3DCOLOR rgb)
{
    const INT A = HIBYTE(HIWORD(rgb));
    const INT R = LOBYTE(HIWORD(rgb)) - 16;
    const INT G = HIBYTE(LOWORD(rgb)) - 16;
    const INT B = LOBYTE(LOWORD(rgb)) - 16;

    //
    // studio RGB [16...235] to SDTV ITU-R BT.601 YCbCr
    //
    INT Y = ( 77 * R + 150 * G +  29 * B + 128) / 256 + 16;
    INT U = (-44 * R -  87 * G + 131 * B + 128) / 256 + 128;
    INT V = (131 * R - 110 * G -  21 * B + 128) / 256 + 128;

    return D3DCOLOR_AYUV(A, Y, U, V);
}


DWORD
RGBtoYUY2(const D3DCOLOR rgb)
{
    const D3DCOLOR yuv = RGBtoYUV(rgb);

    const BYTE Y = LOBYTE(HIWORD(yuv));
    const BYTE U = HIBYTE(LOWORD(yuv));
    const BYTE V = LOBYTE(LOWORD(yuv));

    return MAKELONG(MAKEWORD(Y, U), MAKEWORD(Y, V));
}


BOOL
UpdateSubStream()
{
    HRESULT hr;

    if (!g_bUpdateSubStream)
    {
        return TRUE;
    }

    D3DCOLOR color;

    switch (g_VK_Fx)
    {
    case VK_F1 : color = RGB_WHITE;   break;
    case VK_F2 : color = RGB_RED;     break;
    case VK_F3 : color = RGB_YELLOW;  break;
    case VK_F4 : color = RGB_GREEN;   break;
    case VK_F5 : color = RGB_CYAN;    break;
    case VK_F6 : color = RGB_BLUE;    break;
    case VK_F7 : color = RGB_MAGENTA; break;
    case VK_F8 : color = RGB_BLACK;   break;
    case VK_F9 : color = RGB_ORANGE;  break;
    default    :                      return FALSE;
    }

    D3DLOCKED_RECT lr;

    hr = g_pSubStream->LockRect(&lr, NULL, D3DLOCK_NOSYSLOCK);

    if (FAILED(hr))
    {
        DBGMSG((TEXT("LockRect failed with error 0x%x.\n"), hr));
        return FALSE;
    }

    const BYTE R = LOBYTE(HIWORD(color));
    const BYTE G = HIBYTE(LOWORD(color));
    const BYTE B = LOBYTE(LOWORD(color));

    FillRectangle(lr,
                  0,
                  0,
                  VIDEO_SUB_WIDTH,
                  VIDEO_SUB_HEIGHT,
                  RGBtoYUV(D3DCOLOR_ARGB(g_PixelAlphaValue, R, G, B)));

    hr = g_pSubStream->UnlockRect();

    if (FAILED(hr))
    {
        DBGMSG((TEXT("UnlockRect failed with error 0x%x.\n"), hr));
        return FALSE;
    }

    g_bUpdateSubStream = FALSE;

    return TRUE;
}


BOOL
InitializeVideo()
{
    HRESULT hr;

    //
    // Draw the main stream (SMPTE color bars).
    //
    D3DLOCKED_RECT lr;

    hr = g_pMainStream->LockRect(&lr, NULL, D3DLOCK_NOSYSLOCK);

    if (FAILED(hr))
    {
        DBGMSG((TEXT("LockRect failed with error 0x%x.\n"), hr));
        return FALSE;
    }

    // YUY2 is two pixels per DWORD.
    const UINT dx = VIDEO_MAIN_WIDTH / 2;

    // First row stripes.
    const UINT y1 = VIDEO_MAIN_HEIGHT * 2 / 3;

    FillRectangle(lr, dx * 0 / 7, 0, dx * 1 / 7, y1, RGBtoYUY2(RGB_WHITE_75pc));
    FillRectangle(lr, dx * 1 / 7, 0, dx * 2 / 7, y1, RGBtoYUY2(RGB_YELLOW_75pc));
    FillRectangle(lr, dx * 2 / 7, 0, dx * 3 / 7, y1, RGBtoYUY2(RGB_CYAN_75pc));
    FillRectangle(lr, dx * 3 / 7, 0, dx * 4 / 7, y1, RGBtoYUY2(RGB_GREEN_75pc));
    FillRectangle(lr, dx * 4 / 7, 0, dx * 5 / 7, y1, RGBtoYUY2(RGB_MAGENTA_75pc));
    FillRectangle(lr, dx * 5 / 7, 0, dx * 6 / 7, y1, RGBtoYUY2(RGB_RED_75pc));
    FillRectangle(lr, dx * 6 / 7, 0, dx * 7 / 7, y1, RGBtoYUY2(RGB_BLUE_75pc));

    // Second row stripes.
    const UINT y2 = VIDEO_MAIN_HEIGHT * 3 / 4;

    FillRectangle(lr, dx * 0 / 7, y1, dx * 1 / 7, y2, RGBtoYUY2(RGB_BLUE_75pc));
    FillRectangle(lr, dx * 1 / 7, y1, dx * 2 / 7, y2, RGBtoYUY2(RGB_BLACK));
    FillRectangle(lr, dx * 2 / 7, y1, dx * 3 / 7, y2, RGBtoYUY2(RGB_MAGENTA_75pc));
    FillRectangle(lr, dx * 3 / 7, y1, dx * 4 / 7, y2, RGBtoYUY2(RGB_BLACK));
    FillRectangle(lr, dx * 4 / 7, y1, dx * 5 / 7, y2, RGBtoYUY2(RGB_CYAN_75pc));
    FillRectangle(lr, dx * 5 / 7, y1, dx * 6 / 7, y2, RGBtoYUY2(RGB_BLACK));
    FillRectangle(lr, dx * 6 / 7, y1, dx * 7 / 7, y2, RGBtoYUY2(RGB_WHITE_75pc));

    // Third row stripes.
    const UINT y3  = VIDEO_MAIN_HEIGHT;

    FillRectangle(lr, dx *  0 / 28, y2, dx *  5 / 28, y3, RGBtoYUY2(RGB_I));
    FillRectangle(lr, dx *  5 / 28, y2, dx * 10 / 28, y3, RGBtoYUY2(RGB_WHITE));
    FillRectangle(lr, dx * 10 / 28, y2, dx * 15 / 28, y3, RGBtoYUY2(RGB_Q));
    FillRectangle(lr, dx * 15 / 28, y2, dx * 20 / 28, y3, RGBtoYUY2(RGB_BLACK));
    FillRectangle(lr, dx * 20 / 28, y2, dx * 16 / 21, y3, RGBtoYUY2(RGB_BLACK_n4pc));
    FillRectangle(lr, dx * 16 / 21, y2, dx * 17 / 21, y3, RGBtoYUY2(RGB_BLACK));
    FillRectangle(lr, dx * 17 / 21, y2, dx *  6 /  7, y3, RGBtoYUY2(RGB_BLACK_p4pc));
    FillRectangle(lr, dx *  6 /  7, y2, dx *  7 /  7, y3, RGBtoYUY2(RGB_BLACK));

    hr = g_pMainStream->UnlockRect();

    if (FAILED(hr))
    {
        DBGMSG((TEXT("UnlockRect failed with error 0x%x.\n"), hr));
        return FALSE;
    }

    //
    // Draw the sub stream in the next video process.
    //
    g_bUpdateSubStream = TRUE;

    return TRUE;
}


BOOL
InitializeDXVA2()
{
    HRESULT hr;

    //
    // Retrieve a back buffer as the video render target.
    //
    hr = g_pD3DD9->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &g_pD3DRT);

    if (FAILED(hr))
    {
        DBGMSG((TEXT("GetBackBuffer failed with error 0x%x.\n"), hr));
        return FALSE;
    }

    //
    // Create DXVA2 Video Processor Service.
    //
    hr = DXVA2CreateVideoService(g_pD3DD9,
                                 IID_IDirectXVideoProcessorService,
                                 (VOID**)&g_pDXVAVPS);

    if (FAILED(hr))
    {
        DBGMSG((TEXT("DXVA2CreateVideoService failed with error 0x%x.\n"), hr));
        return FALSE;
    }

    //
    // Initialize the video descriptor.
    //
    g_VideoDesc.SampleWidth                         = VIDEO_MAIN_WIDTH;
    g_VideoDesc.SampleHeight                        = VIDEO_MAIN_HEIGHT;
    g_VideoDesc.SampleFormat.VideoChromaSubsampling = DXVA2_VideoChromaSubsampling_MPEG2;
    g_VideoDesc.SampleFormat.NominalRange           = DXVA2_NominalRange_16_235;
    g_VideoDesc.SampleFormat.VideoTransferMatrix    = EX_COLOR_INFO[g_ExColorInfo][0];
    g_VideoDesc.SampleFormat.VideoLighting          = DXVA2_VideoLighting_dim;
    g_VideoDesc.SampleFormat.VideoPrimaries         = DXVA2_VideoPrimaries_BT709;
    g_VideoDesc.SampleFormat.VideoTransferFunction  = DXVA2_VideoTransFunc_709;
    g_VideoDesc.SampleFormat.SampleFormat           = DXVA2_SampleProgressiveFrame;
    g_VideoDesc.Format                              = VIDEO_MAIN_FORMAT;
    g_VideoDesc.InputSampleFreq.Numerator           = VIDEO_FPS;
    g_VideoDesc.InputSampleFreq.Denominator         = 1;
    g_VideoDesc.OutputFrameFreq.Numerator           = VIDEO_FPS;
    g_VideoDesc.OutputFrameFreq.Denominator         = 1;

    //
    // Query the video processor GUID.
    //
    UINT count;
    GUID* guids = NULL;

    hr = g_pDXVAVPS->GetVideoProcessorDeviceGuids(&g_VideoDesc, &count, &guids);

    if (FAILED(hr))
    {
        DBGMSG((TEXT("GetVideoProcessorDeviceGuids failed with error 0x%x.\n"), hr));
        return FALSE;
    }

    //
    // Create a DXVA2 device.
    //
    for (UINT i = 0; i < count; i++)
    {
        if (CreateDXVA2VPDevice(guids[i]))
        {
            break;
        }
    }

    CoTaskMemFree(guids);

    if (!g_pDXVAVPD)
    {
        DBGMSG((TEXT("Failed to create a DXVA2 device.\n")));
        return FALSE;
    }

    if (!InitializeVideo())
    {
        return FALSE;
    }

    return TRUE;
}


VOID
DestroyDXVA2()
{
    if (g_pSubStream)
    {
        g_pSubStream->Release();
        g_pSubStream = NULL;
    }

    if (g_pMainStream)
    {
        g_pMainStream->Release();
        g_pMainStream = NULL;
    }

    if (g_pDXVAVPD)
    {
        g_pDXVAVPD->Release();
        g_pDXVAVPD = NULL;
    }

    if (g_pDXVAVPS)
    {
        g_pDXVAVPS->Release();
        g_pDXVAVPS = NULL;
    }

    if (g_pD3DRT)
    {
        g_pD3DRT->Release();
        g_pD3DRT = NULL;
    }
}


BOOL
EnableDwmQueuing()
{
    HRESULT hr;

    //
    // DWM is not available.
    //
    if (!g_hDwmApiDLL)
    {
        return TRUE;
    }

    //
    // Check to see if DWM is currently enabled.
    //
    BOOL bDWM = FALSE;

    hr = ((PFNDWMISCOMPOSITIONENABLED) g_pfnDwmIsCompositionEnabled)(&bDWM);

    if (FAILED(hr))
    {
        DBGMSG((TEXT("DwmIsCompositionEnabled failed with error 0x%x.\n"), hr));
        return FALSE;
    }

    //
    // DWM queuing is disabled when DWM is disabled.
    //
    if (!bDWM)
    {
        g_bDwmQueuing = FALSE;
        return TRUE;
    }

    //
    // DWM queuing is enabled already.
    //
    if (g_bDwmQueuing)
    {
        return TRUE;
    }

    //
    // Retrieve DWM refresh count of the last vsync.
    //
    DWM_TIMING_INFO dwmti = {0};

    dwmti.cbSize = sizeof(dwmti);

    hr = ((PFNDWMGETCOMPOSITIONTIMINGINFO) g_pfnDwmGetCompositionTimingInfo)(NULL, &dwmti);

    if (FAILED(hr))
    {
        DBGMSG((TEXT("DwmGetCompositionTimingInfo failed with error 0x%x.\n"), hr));
        return FALSE;
    }

    //
    // Enable DWM queuing from the next refresh.
    //
    DWM_PRESENT_PARAMETERS dwmpp = {0};

    dwmpp.cbSize             = sizeof(dwmpp);
    dwmpp.fQueue             = TRUE;
    dwmpp.cRefreshStart      = dwmti.cRefresh + 1;
    dwmpp.cBuffer            = DWM_BUFFER_COUNT;
    dwmpp.fUseSourceRate     = FALSE;
    dwmpp.cRefreshesPerFrame = 1;
    dwmpp.eSampling          = DWM_SOURCE_FRAME_SAMPLING_POINT;

    hr = ((PFNDWMSETPRESENTPARAMETERS) g_pfnDwmSetPresentParameters)(g_Hwnd, &dwmpp);

    if (FAILED(hr))
    {
        DBGMSG((TEXT("DwmSetPresentParameters failed with error 0x%x.\n"), hr));
        return FALSE;
    }

    //
    // DWM queuing is enabled.
    //
    g_bDwmQueuing = TRUE;

    return TRUE;
}


BOOL
ChangeFullscreenMode(BOOL bFullscreen)
{
    //
    // Mark the mode change in progress to prevent the device is being reset in OnSize.
    // This is because these API calls below will generate WM_SIZE messages.
    //
    g_bInModeChange = TRUE;

    if (bFullscreen)
    {
        //
        // Save the window position.
        //
        if (!GetWindowRect(g_Hwnd, &g_RectWindow))
        {
            DBGMSG((TEXT("GetWindowRect failed with error %d.\n"), GetLastError()));
            return FALSE;
        }

        if (!SetWindowLongPtr(g_Hwnd, GWL_STYLE, WS_POPUP | WS_VISIBLE))
        {
            DBGMSG((TEXT("SetWindowLongPtr failed with error %d.\n"), GetLastError()));
            return FALSE;
        }

        if (!SetWindowPos(g_Hwnd,
                          HWND_NOTOPMOST,
                          0,
                          0,
                          GetSystemMetrics(SM_CXSCREEN),
                          GetSystemMetrics(SM_CYSCREEN),
                          SWP_FRAMECHANGED))
        {
            DBGMSG((TEXT("SetWindowPos failed with error %d.\n"), GetLastError()));
            return FALSE;
        }
    }
    else
    {
        if (!SetWindowLongPtr(g_Hwnd, GWL_STYLE, WS_OVERLAPPEDWINDOW | WS_VISIBLE))
        {
            DBGMSG((TEXT("SetWindowLongPtr failed with error %d.\n"), GetLastError()));
            return FALSE;
        }

        //
        // Restore the window position.
        //
        if (!SetWindowPos(g_Hwnd,
                          HWND_NOTOPMOST,
                          g_RectWindow.left,
                          g_RectWindow.top,
                          g_RectWindow.right - g_RectWindow.left,
                          g_RectWindow.bottom - g_RectWindow.top,
                          SWP_FRAMECHANGED))
        {
            DBGMSG((TEXT("SetWindowPos failed with error %d.\n"), GetLastError()));
            return FALSE;
        }
    }

    g_bInModeChange = FALSE;

    return TRUE;
}


BOOL
ResetDevice(BOOL bChangeWindowMode = FALSE)
{
    HRESULT hr;

    if (bChangeWindowMode)
    {
        g_bWindowed = !g_bWindowed;

        if (!ChangeFullscreenMode(!g_bWindowed))
        {
            return FALSE;
        }
    }

    if (g_pD3DD9)
    {
        //
        // Destroy DXVA2 device because it may be holding any D3D9 resources.
        //
        DestroyDXVA2();

        if (g_bWindowed)
        {
            g_D3DPP.BackBufferWidth  = 0;
            g_D3DPP.BackBufferHeight = 0;
        }
        else
        {
            g_D3DPP.BackBufferWidth  = GetSystemMetrics(SM_CXSCREEN);
            g_D3DPP.BackBufferHeight = GetSystemMetrics(SM_CYSCREEN);
        }

        g_D3DPP.Windowed = g_bWindowed;

        //
        // Reset will change the parameters, so use a copy instead.
        //
        D3DPRESENT_PARAMETERS d3dpp = g_D3DPP;

        hr = g_pD3DD9->Reset(&d3dpp);

        if (FAILED(hr))
        {
            DBGMSG((TEXT("Reset failed with error 0x%x.\n"), hr));
        }

        if (SUCCEEDED(hr) && InitializeDXVA2())
        {
            return TRUE;
        }

        //
        // If either Reset didn't work or failed to initialize DXVA2 device,
        // try to recover by recreating the devices from the scratch.
        //
        DestroyDXVA2();
        DestroyD3D9();
    }

    if (InitializeD3D9() && InitializeDXVA2())
    {
        return TRUE;
    }

    //
    // Fallback to Window mode, if failed to initialize Fullscreen mode.
    //
    if (g_bWindowed)
    {
        return FALSE;
    }

    DestroyDXVA2();
    DestroyD3D9();

    if (!ChangeFullscreenMode(FALSE))
    {
        return FALSE;
    }

    g_bWindowed = TRUE;

    if (InitializeD3D9() && InitializeDXVA2())
    {
        return TRUE;
    }

    return FALSE;
}


DWORD
GetFrameNumber()
{
    DWORD currentTime;
    DWORD currentSysTime = timeGetTime();

    if (g_StartSysTime > currentSysTime)
    {
        currentTime = currentSysTime + (0xFFFFFFFF - g_StartSysTime);
    }
    else
    {
        currentTime = currentSysTime - g_StartSysTime;
    }

    DWORD frame = currentTime / VIDEO_MSPF;
    DWORD delta = (currentTime - g_PreviousTime) / VIDEO_MSPF;

    if (delta > 1)
    {
        if (g_bDspFrameDrop)
        {
            DBGMSG((TEXT("Frame dropped %u frame(s).\n"), delta - 1));
        }
    }

    if (delta > 0)
    {
        g_PreviousTime = currentTime;
    }

    return frame;
}


DXVA2_AYUVSample16
GetBackgroundColor()
{
    const D3DCOLOR yuv = RGBtoYUV(BACKGROUND_COLORS[g_BackgroundColor]);

    const BYTE Y = LOBYTE(HIWORD(yuv));
    const BYTE U = HIBYTE(LOWORD(yuv));
    const BYTE V = LOBYTE(LOWORD(yuv));

    DXVA2_AYUVSample16 color;

    color.Cr    = V * 0x100;
    color.Cb    = U * 0x100;
    color.Y     = Y * 0x100;
    color.Alpha = 0xFFFF;

    return color;
}


RECT
ScaleRectangle(const RECT& input, const RECT& src, const RECT& dst)
{
    RECT rect;

    UINT src_dx = src.right - src.left;
    UINT src_dy = src.bottom - src.top;

    UINT dst_dx = dst.right - dst.left;
    UINT dst_dy = dst.bottom - dst.top;

    //
    // Scale input rectangle within src rectangle to dst rectangle.
    //
    rect.left   = input.left   * dst_dx / src_dx;
    rect.right  = input.right  * dst_dx / src_dx;
    rect.top    = input.top    * dst_dy / src_dy;
    rect.bottom = input.bottom * dst_dy / src_dy;

    return rect;
}


BOOL
ProcessVideo()
{
    HRESULT hr;

    if (!g_pD3DD9)
    {
        return FALSE;
    }

    RECT rect;
    GetClientRect(g_Hwnd, &rect);

    if (IsRectEmpty(&rect))
    {
        return TRUE;
    }

    //
    // Check the current status of D3D9 device.
    //
    hr = g_pD3DD9->TestCooperativeLevel();

    switch (hr)
    {
    case D3D_OK :
        break;

    case D3DERR_DEVICELOST :
        DBGMSG((TEXT("TestCooperativeLevel returned D3DERR_DEVICELOST.\n")));
        return TRUE;

    case D3DERR_DEVICENOTRESET :
        DBGMSG((TEXT("TestCooperativeLevel returned D3DERR_DEVICENOTRESET.\n")));

        if (!ResetDevice())
        {
            return FALSE;
        }

        break;

    default :
        DBGMSG((TEXT("TestCooperativeLevel failed with error 0x%x.\n"), hr));
        return FALSE;
    }

    DXVA2_VideoProcessBltParams blt = {0};
    DXVA2_VideoSample samples[2] = {0};

    DWORD frame = GetFrameNumber();

    LONGLONG start_100ns = frame * LONGLONG(VIDEO_100NSPF);
    LONGLONG end_100ns   = start_100ns + LONGLONG(VIDEO_100NSPF);

    RECT client;
    GetClientRect(g_Hwnd, &client);

    RECT target;

    target.left   = client.left   + (client.right  - client.left) / 2 * (100 - g_TargetWidthPercent)  / 100;
    target.right  = client.right  - (client.right  - client.left) / 2 * (100 - g_TargetWidthPercent)  / 100;
    target.top    = client.top    + (client.bottom - client.top)  / 2 * (100 - g_TargetHeightPercent) / 100;
    target.bottom = client.bottom - (client.bottom - client.top)  / 2 * (100 - g_TargetHeightPercent) / 100;

    //
    // Calculate sub stream destination based on the current frame number.
    //
    RECT ssdest;
    INT x, y, wx, wy;

    x = frame * VIDEO_SUB_VX;
    wx = VIDEO_MAIN_WIDTH - VIDEO_SUB_WIDTH;
    x = (x / wx) & 0x1 ? wx - (x % wx) : x % wx;

    y = frame * VIDEO_SUB_VY;
    wy = VIDEO_MAIN_HEIGHT - VIDEO_SUB_HEIGHT;
    y = (y / wy) & 0x1 ? wy - (y % wy) : y % wy;

    SetRect(&ssdest, x, y, x + VIDEO_SUB_WIDTH, y + VIDEO_SUB_HEIGHT);

    //
    // Initialize VPBlt parameters.
    //
    blt.TargetFrame = start_100ns;
    blt.TargetRect  = target;

    // DXVA2_VideoProcess_Constriction
    blt.ConstrictionSize.cx = target.right - target.left;
    blt.ConstrictionSize.cy = target.bottom - target.top;

    blt.BackgroundColor = GetBackgroundColor();

    // DXVA2_VideoProcess_YUV2RGBExtended
    blt.DestFormat.VideoChromaSubsampling = DXVA2_VideoChromaSubsampling_Unknown;
    blt.DestFormat.NominalRange           = EX_COLOR_INFO[g_ExColorInfo][1];
    blt.DestFormat.VideoTransferMatrix    = DXVA2_VideoTransferMatrix_Unknown;
    blt.DestFormat.VideoLighting          = DXVA2_VideoLighting_dim;
    blt.DestFormat.VideoPrimaries         = DXVA2_VideoPrimaries_BT709;
    blt.DestFormat.VideoTransferFunction  = DXVA2_VideoTransFunc_709;

    blt.DestFormat.SampleFormat = DXVA2_SampleProgressiveFrame;

    // DXVA2_ProcAmp_Brightness
    blt.ProcAmpValues.Brightness = g_ProcAmpValues[0];

    // DXVA2_ProcAmp_Contrast
    blt.ProcAmpValues.Contrast = g_ProcAmpValues[1];

    // DXVA2_ProcAmp_Hue
    blt.ProcAmpValues.Hue = g_ProcAmpValues[2];

    // DXVA2_ProcAmp_Saturation
    blt.ProcAmpValues.Saturation = g_ProcAmpValues[3];

    // DXVA2_VideoProcess_AlphaBlend
    blt.Alpha = DXVA2_Fixed32OpaqueAlpha();

    // DXVA2_VideoProcess_NoiseFilter
    blt.NoiseFilterLuma.Level       = g_NFilterValues[0];
    blt.NoiseFilterLuma.Threshold   = g_NFilterValues[1];
    blt.NoiseFilterLuma.Radius      = g_NFilterValues[2];
    blt.NoiseFilterChroma.Level     = g_NFilterValues[3];
    blt.NoiseFilterChroma.Threshold = g_NFilterValues[4];
    blt.NoiseFilterChroma.Radius    = g_NFilterValues[5];

    // DXVA2_VideoProcess_DetailFilter
    blt.DetailFilterLuma.Level       = g_DFilterValues[0];
    blt.DetailFilterLuma.Threshold   = g_DFilterValues[1];
    blt.DetailFilterLuma.Radius      = g_DFilterValues[2];
    blt.DetailFilterChroma.Level     = g_DFilterValues[3];
    blt.DetailFilterChroma.Threshold = g_DFilterValues[4];
    blt.DetailFilterChroma.Radius    = g_DFilterValues[5];

    //
    // Initialize main stream video sample.
    //
    samples[0].Start = start_100ns;
    samples[0].End   = end_100ns;

    // DXVA2_VideoProcess_YUV2RGBExtended
    samples[0].SampleFormat.VideoChromaSubsampling = DXVA2_VideoChromaSubsampling_MPEG2;
    samples[0].SampleFormat.NominalRange           = DXVA2_NominalRange_16_235;
    samples[0].SampleFormat.VideoTransferMatrix    = EX_COLOR_INFO[g_ExColorInfo][0];
    samples[0].SampleFormat.VideoLighting          = DXVA2_VideoLighting_dim;
    samples[0].SampleFormat.VideoPrimaries         = DXVA2_VideoPrimaries_BT709;
    samples[0].SampleFormat.VideoTransferFunction  = DXVA2_VideoTransFunc_709;

    samples[0].SampleFormat.SampleFormat = DXVA2_SampleProgressiveFrame;

    samples[0].SrcSurface = g_pMainStream;

    // DXVA2_VideoProcess_SubRects
    samples[0].SrcRect = g_SrcRect;

    // DXVA2_VideoProcess_StretchX, Y
    samples[0].DstRect = ScaleRectangle(g_DstRect, VIDEO_MAIN_RECT, client);

    // DXVA2_VideoProcess_PlanarAlpha
    samples[0].PlanarAlpha = DXVA2FloatToFixed(float(g_PlanarAlphaValue) / 0xFF);

    //
    // Initialize sub stream video sample.
    //
    samples[1] = samples[0];

    // DXVA2_VideoProcess_SubStreamsExtended
    samples[1].SampleFormat = samples[0].SampleFormat;

    // DXVA2_VideoProcess_SubStreams
    samples[1].SampleFormat.SampleFormat = DXVA2_SampleSubStream;

    samples[1].SrcSurface = g_pSubStream;

    samples[1].SrcRect = VIDEO_SUB_RECT;

    samples[1].DstRect = ScaleRectangle(ssdest, VIDEO_MAIN_RECT, client);

    if (!UpdateSubStream())
    {
        return FALSE;
    }

    if (g_TargetWidthPercent < 100 || g_TargetHeightPercent < 100)
    {
        hr = g_pD3DD9->ColorFill(g_pD3DRT, NULL, D3DCOLOR_XRGB(0, 0, 0));

        if (FAILED(hr))
        {
            DBGMSG((TEXT("ColorFill failed with error 0x%x.\n"), hr));
        }
    }

    hr = g_pDXVAVPD->VideoProcessBlt(g_pD3DRT,
                                     &blt,
                                     samples,
                                     SUB_STREAM_COUNT + 1,
                                     NULL);

    if (FAILED(hr))
    {
        DBGMSG((TEXT("VideoProcessBlt failed with error 0x%x.\n"), hr));
    }

    //
    // Re-enable DWM queuing if it is not enabled.
    //
    EnableDwmQueuing();

    hr = g_pD3DD9->Present(NULL, NULL, NULL, NULL);

    if (FAILED(hr))
    {
        DBGMSG((TEXT("Present failed with error 0x%x.\n"), hr));
    }

    return TRUE;
}


VOID
OnDestroy(HWND hwnd)
{
    PostQuitMessage(0);
}


VOID
ResetParameter()
{
    switch (g_VK_Fx)
    {
    case VK_F1 :
        g_PlanarAlphaValue = DEFAULT_PLANAR_ALPHA_VALUE;
        g_PixelAlphaValue  = DEFAULT_PIXEL_ALPHA_VALUE;
        g_bUpdateSubStream = TRUE;
        break;

    case VK_F2 :
    case VK_F3 :
        g_SrcRect = VIDEO_MAIN_RECT;
        break;

    case VK_F4 :
    case VK_F5 :
        g_DstRect = VIDEO_MAIN_RECT;
        break;

    case VK_F6 :
        g_BackgroundColor = 0;
        g_ExColorInfo = 0;
        break;

    case VK_F7 :
        g_ProcAmpValues[0] = g_ProcAmpRanges[0].DefaultValue;
        g_ProcAmpValues[1] = g_ProcAmpRanges[1].DefaultValue;
        break;

    case VK_F8 :
        g_ProcAmpValues[2] = g_ProcAmpRanges[2].DefaultValue;
        g_ProcAmpValues[3] = g_ProcAmpRanges[3].DefaultValue;
        break;

    case VK_F9 :
        g_TargetWidthPercent  = 100;
        g_TargetHeightPercent = 100;
        break;

    }
}


DXVA2_Fixed32
ValidateValueRange(
    const DXVA2_Fixed32& value,
    const INT steps,
    const DXVA2_ValueRange& range
    )
{
    float f_value = DXVA2FixedToFloat(value);
    float f_max = DXVA2FixedToFloat(range.MaxValue);
    float f_min = DXVA2FixedToFloat(range.MinValue);

    f_value += DXVA2FixedToFloat(range.StepSize) * steps;
    f_value = min(max(f_value, f_min), f_max);

    return DXVA2FloatToFixed(f_value);
}


VOID
IncrementParameter(UINT vk)
{
    RECT rect, intersect;

    switch (g_VK_Fx)
    {
    case VK_F1 :
        if (vk == VK_UP)
        {
            g_PlanarAlphaValue = min(g_PlanarAlphaValue + 8, 0xFF);
        }
        else
        {
            g_PixelAlphaValue = min(g_PixelAlphaValue + 8, 0xFF);
            g_bUpdateSubStream = TRUE;
        }

        break;

    case VK_F2 :
        rect = g_SrcRect;

        InflateRect(&rect, vk == VK_UP ? 0 : -8, vk == VK_UP ? -8 : 0);
        IntersectRect(&intersect, &rect, &VIDEO_MAIN_RECT);

        if (!IsRectEmpty(&intersect))
        {
            g_SrcRect = intersect;
        }

        break;

    case VK_F3 :
        rect = g_SrcRect;

        OffsetRect(&rect, vk == VK_UP ? 0 : 8, vk == VK_UP ? -8 : 0);
        IntersectRect(&intersect, &rect, &VIDEO_MAIN_RECT);

        if (EqualRect(&rect, &intersect))
        {
            g_SrcRect = rect;
        }

        break;

    case VK_F4 :
        rect = g_DstRect;

        InflateRect(&rect, vk == VK_UP ? 0 : 8, vk == VK_UP ? 8 : 0);
        IntersectRect(&intersect, &rect, &VIDEO_MAIN_RECT);

        if (!IsRectEmpty(&intersect))
        {
            g_DstRect = intersect;
        }

        break;

    case VK_F5 :
        rect = g_DstRect;

        OffsetRect(&rect, vk == VK_UP ? 0 : 8, vk == VK_UP ? -8 : 0);
        IntersectRect(&intersect, &rect, &VIDEO_MAIN_RECT);

        if (EqualRect(&rect, &intersect))
        {
            g_DstRect = rect;
        }

        break;

    case VK_F6 :
        if (vk == VK_UP)
        {
            if (++g_ExColorInfo > ARRAYSIZE(EX_COLOR_INFO) - 1)
            {
                g_ExColorInfo = 0;
            }
        }
        else
        {
            if (++g_BackgroundColor > ARRAYSIZE(BACKGROUND_COLORS) - 1)
            {
                g_BackgroundColor = 0;
            }
        }

        break;

    case VK_F7 :
        if (vk == VK_UP)
        {
            g_ProcAmpValues[0] = ValidateValueRange(g_ProcAmpValues[0],
                                                    g_ProcAmpSteps[0],
                                                    g_ProcAmpRanges[0]);
        }
        else
        {
            g_ProcAmpValues[1] = ValidateValueRange(g_ProcAmpValues[1],
                                                    g_ProcAmpSteps[1],
                                                    g_ProcAmpRanges[1]);
        }

        break;

    case VK_F8 :
        if (vk == VK_UP)
        {
            g_ProcAmpValues[2] = ValidateValueRange(g_ProcAmpValues[2],
                                                    g_ProcAmpSteps[2],
                                                    g_ProcAmpRanges[2]);
        }
        else
        {
            g_ProcAmpValues[3] = ValidateValueRange(g_ProcAmpValues[3],
                                                    g_ProcAmpSteps[3],
                                                    g_ProcAmpRanges[3]);
        }

        break;

    case VK_F9 :
        if (vk == VK_UP)
        {
            g_TargetHeightPercent = min(g_TargetHeightPercent + 4, 100);
        }
        else
        {
            g_TargetWidthPercent = min(g_TargetWidthPercent + 4, 100);
        }

        break;

    }
}


VOID
DecrementParameter(UINT vk)
{
    RECT rect, intersect;

    switch (g_VK_Fx)
    {
    case VK_F1 :
        if (vk == VK_DOWN)
        {
            g_PlanarAlphaValue = max(g_PlanarAlphaValue - 8, 0);
        }
        else
        {
            g_PixelAlphaValue = max(g_PixelAlphaValue - 8, 0);
            g_bUpdateSubStream = TRUE;
        }

        break;

    case VK_F2 :
        rect = g_SrcRect;

        InflateRect(&rect, vk == VK_DOWN ? 0 : 8, vk == VK_DOWN ? 8 : 0);
        IntersectRect(&intersect, &rect, &VIDEO_MAIN_RECT);

        if (!IsRectEmpty(&intersect))
        {
            g_SrcRect = intersect;
        }

        break;

    case VK_F3 :
        rect = g_SrcRect;

        OffsetRect(&rect, vk == VK_DOWN ? 0 : -8, vk == VK_DOWN ? 8 : 0);
        IntersectRect(&intersect, &rect, &VIDEO_MAIN_RECT);

        if (EqualRect(&rect, &intersect))
        {
            g_SrcRect = rect;
        }

        break;

    case VK_F4 :
        rect = g_DstRect;

        InflateRect(&rect, vk == VK_DOWN ? 0 : -8, vk == VK_DOWN ? -8 : 0);
        IntersectRect(&intersect, &rect, &VIDEO_MAIN_RECT);

        if (!IsRectEmpty(&intersect))
        {
            g_DstRect = intersect;
        }

        break;

    case VK_F5 :
        rect = g_DstRect;

        OffsetRect(&rect, vk == VK_DOWN ? 0 : -8, vk == VK_DOWN ? 8 : 0);
        IntersectRect(&intersect, &rect, &VIDEO_MAIN_RECT);

        if (EqualRect(&rect, &intersect))
        {
            g_DstRect = rect;
        }

        break;

    case VK_F6 :
        if (vk == VK_DOWN)
        {
            if (--g_ExColorInfo < 0)
            {
                g_ExColorInfo = ARRAYSIZE(EX_COLOR_INFO) - 1;
            }
        }
        else
        {
            if (--g_BackgroundColor < 0)
            {
                g_BackgroundColor = ARRAYSIZE(BACKGROUND_COLORS) - 1;
            }
        }

        break;

    case VK_F7 :
        if (vk == VK_DOWN)
        {
            g_ProcAmpValues[0] = ValidateValueRange(g_ProcAmpValues[0],
                                                    g_ProcAmpSteps[0] * -1,
                                                    g_ProcAmpRanges[0]);
        }
        else
        {
            g_ProcAmpValues[1] = ValidateValueRange(g_ProcAmpValues[1],
                                                    g_ProcAmpSteps[1] * -1,
                                                    g_ProcAmpRanges[1]);
        }

        break;

    case VK_F8 :
        if (vk == VK_DOWN)
        {
            g_ProcAmpValues[2] = ValidateValueRange(g_ProcAmpValues[2],
                                                    g_ProcAmpSteps[2] * -1,
                                                    g_ProcAmpRanges[2]);
        }
        else
        {
            g_ProcAmpValues[3] = ValidateValueRange(g_ProcAmpValues[3],
                                                    g_ProcAmpSteps[3] * -1,
                                                    g_ProcAmpRanges[3]);
        }

        break;

    case VK_F9 :
        if (vk == VK_DOWN)
        {
            g_TargetHeightPercent = max(g_TargetHeightPercent - 4, 4);
        }
        else
        {
            g_TargetWidthPercent = max(g_TargetWidthPercent - 4, 4);
        }

        break;

    }
}


VOID
OnKey(HWND hwnd, UINT vk, BOOL fDown, int cRepeat, UINT flags)
{
    if (!fDown)
    {
        return;
    }

    if (vk == VK_ESCAPE)
    {
        DestroyWindow(hwnd);
        return;
    }

    if (!g_pD3DD9)
    {
        return;
    }

    switch (vk)
    {
    case VK_F1 :
    case VK_F2 :
    case VK_F3 :
    case VK_F4 :
    case VK_F5 :
    case VK_F6 :
    case VK_F7 :
    case VK_F8 :
    case VK_F9 :
        g_VK_Fx = vk;
        g_bUpdateSubStream = TRUE;
        break;

    case VK_HOME :
        ResetParameter();
        break;

    case VK_END :
        g_bDspFrameDrop = !g_bDspFrameDrop;
        break;

    case VK_UP :
    case VK_RIGHT :
        IncrementParameter(vk);
        break;

    case VK_DOWN :
    case VK_LEFT :
        DecrementParameter(vk);
        break;
    }
}


VOID
OnPaint(HWND hwnd)
{
    ValidateRect(hwnd , NULL);

    ProcessVideo();
}


VOID
OnSize(HWND hwnd, UINT state, int cx, int cy)
{
    if (!g_pD3DD9)
    {
        return;
    }

    RECT rect;
    GetClientRect(hwnd, &rect);

    if (IsRectEmpty(&rect))
    {
        return;
    }

    //
    // Do not reset the device while the mode change is in progress.
    //
    if (g_bInModeChange)
    {
        return;
    }

    if (!ResetDevice())
    {
        DestroyWindow(hwnd);
        return;
    }

    InvalidateRect(hwnd , NULL , FALSE);
}


LRESULT CALLBACK
WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        HANDLE_MSG(hwnd, WM_DESTROY, OnDestroy);
        HANDLE_MSG(hwnd, WM_KEYDOWN, OnKey);
        HANDLE_MSG(hwnd, WM_PAINT,   OnPaint);
        HANDLE_MSG(hwnd, WM_SIZE,    OnSize);
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}


BOOL
InitializeWindow()
{
    WNDCLASS wc = {0};

    wc.lpfnWndProc   = WindowProc;
    wc.hInstance     = GetModuleHandle(NULL);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = CLASS_NAME;

    if (!RegisterClass(&wc))
    {
        DBGMSG((TEXT("RegisterClass failed with error %d.\n"), GetLastError()));
        return FALSE;
    }

    //
    // Start in Window mode regardless of the initial mode.
    //
    g_Hwnd = CreateWindow(CLASS_NAME,
                          WINDOW_NAME,
                          WS_OVERLAPPEDWINDOW,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          NULL,
                          NULL,
                          GetModuleHandle(NULL),
                          NULL);

    if (!g_Hwnd)
    {
        DBGMSG((TEXT("CreateWindow failed with error %d.\n"), GetLastError()));
        return FALSE;
    }

    ShowWindow(g_Hwnd, SW_SHOWDEFAULT);
    UpdateWindow(g_Hwnd);

    //
    // Change the window from Window mode to Fullscreen mode.
    //
    if (!g_bWindowed)
    {
        if (!ChangeFullscreenMode(TRUE))
        {
            //
            // If failed, revert to Window mode.
            //
            if (!ChangeFullscreenMode(FALSE))
            {
                return FALSE;
            }

            g_bWindowed = TRUE;
        }
    }

    return TRUE;
}


BOOL
InitializeTimer()
{
    g_hTimer = CreateWaitableTimer(NULL, FALSE, NULL);

    if (!g_hTimer)
    {
        DBGMSG((TEXT("CreateWaitableTimer failed with error %d.\n"), GetLastError()));
        return FALSE;
    }

    LARGE_INTEGER li = {0};

    if (!SetWaitableTimer(g_hTimer,
                          &li,
                          VIDEO_MSPF,
                          NULL,
                          NULL,
                          FALSE))
    {
        DBGMSG((TEXT("SetWaitableTimer failed with error %d.\n"), GetLastError()));
        return FALSE;
    }

    g_bTimerSet = (timeBeginPeriod(1) == TIMERR_NOERROR);

    g_StartSysTime = timeGetTime();

    return TRUE;
}


VOID
DestroyTimer()
{
    if (g_bTimerSet)
    {
        timeEndPeriod(1);
        g_bTimerSet = FALSE;
    }

    if (g_hTimer)
    {
        CloseHandle(g_hTimer);
        g_hTimer = NULL;
    }
}


BOOL
PreTranslateMessage(const MSG& msg)
{
    //
    // Only interested in Alt + Enter.
    //
    if (msg.message != WM_SYSKEYDOWN || msg.wParam != VK_RETURN)
    {
        return FALSE;
    }

    if (!g_pD3DD9)
    {
        return TRUE;
    }

    RECT rect;
    GetClientRect(msg.hwnd, &rect);

    if (IsRectEmpty(&rect))
    {
        return TRUE;
    }

    if (ResetDevice(TRUE))
    {
        return TRUE;
    }

    DestroyWindow(msg.hwnd);
    return TRUE;
}


INT
MessageLoop()
{
    MSG msg = {0};

    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (PreTranslateMessage(msg))
            {
                continue;
            }

            TranslateMessage(&msg);
            DispatchMessage(&msg);

            continue;
        }

        //
        // Wait until the timer expires or any message is posted.
        //
        if (MsgWaitForMultipleObjects(1,
                                      &g_hTimer,
                                      FALSE,
                                      INFINITE,
                                      QS_ALLINPUT) == WAIT_OBJECT_0)
        {
            if (!ProcessVideo())
            {
                DestroyWindow(g_Hwnd);
            }
        }
    }

    return INT(msg.wParam);
}


BOOL
InitializeParameter(PCTSTR psz)
{
    HRESULT hr;

    size_t cch;
    hr = StringCchLength(psz, STRSAFE_MAX_CCH, &cch);

    if (FAILED(hr))
    {
        DBGMSG((TEXT("StringCchLength failed with error 0x%x.\n"), hr));
        return FALSE;
    }

    //
    // No command line parameter is specified. Use default.
    //
    if (!cch)
    {
        return TRUE;
    }

    //
    // hardware D3D9 + hardware DXVA2 only
    //
    if (CompareString(LOCALE_INVARIANT,
                      NORM_IGNORECASE,
                      psz,
                      -1,
                      TEXT("-hh"),
                      -1) == CSTR_EQUAL)
    {
        g_bD3D9SW = FALSE;
        g_bDXVA2SW = FALSE;

        return TRUE;
    }

    //
    // hardware D3D9 + software DXVA2 only
    //
    if (CompareString(LOCALE_INVARIANT,
                      NORM_IGNORECASE,
                      psz,
                      -1,
                      TEXT("-hs"),
                      -1) == CSTR_EQUAL)
    {
        g_bD3D9SW = FALSE;
        g_bDXVA2HW = FALSE;

        return TRUE;
    }

    //
    // software D3D9 + software DXVA2 only
    //
    if (CompareString(LOCALE_INVARIANT,
                      NORM_IGNORECASE,
                      psz,
                      -1,
                      TEXT("-ss"),
                      -1) == CSTR_EQUAL)
    {
        g_bD3D9HW = FALSE;
        g_bDXVA2HW = FALSE;

        return TRUE;
    }

    //
    // Invalid command line parameter is specified.
    //
    return FALSE;
}


BOOL
InitializeModule()
{
    //
    // Load these DLLs dynamically because these may not be available prior to Vista.
    //
    g_hRgb9rastDLL = LoadLibrary(TEXT("rgb9rast.dll"));

    if (!g_hRgb9rastDLL)
    {
        DBGMSG((TEXT("LoadLibrary(rgb9rast.dll) failed with error %d.\n"), GetLastError()));
        goto SKIP_RGB9RAST;
    }

    g_pfnD3D9GetSWInfo = GetProcAddress(g_hRgb9rastDLL, "D3D9GetSWInfo");

    if (!g_pfnD3D9GetSWInfo)
    {
        DBGMSG((TEXT("GetProcAddress(D3D9GetSWInfo) failed with error %d.\n"), GetLastError()));
        return FALSE;
    }

SKIP_RGB9RAST:

    g_hDwmApiDLL = LoadLibrary(TEXT("dwmapi.dll"));

    if (!g_hDwmApiDLL)
    {
        DBGMSG((TEXT("LoadLibrary(dwmapi.dll) failed with error %d.\n"), GetLastError()));
        goto SKIP_DWMAPI;
    }

    g_pfnDwmIsCompositionEnabled = GetProcAddress(g_hDwmApiDLL, "DwmIsCompositionEnabled");

    if (!g_pfnDwmIsCompositionEnabled)
    {
        DBGMSG((TEXT("GetProcAddress(DwmIsCompositionEnabled) failed with error %d.\n"), GetLastError()));
        return FALSE;
    }

    g_pfnDwmGetCompositionTimingInfo = GetProcAddress(g_hDwmApiDLL, "DwmGetCompositionTimingInfo");

    if (!g_pfnDwmGetCompositionTimingInfo)
    {
        DBGMSG((TEXT("GetProcAddress(DwmGetCompositionTimingInfo) failed with error %d.\n"), GetLastError()));
        return FALSE;
    }

    g_pfnDwmSetPresentParameters = GetProcAddress(g_hDwmApiDLL, "DwmSetPresentParameters");

    if (!g_pfnDwmSetPresentParameters)
    {
        DBGMSG((TEXT("GetProcAddress(DwmSetPresentParameters) failed with error %d.\n"), GetLastError()));
        return FALSE;
    }

SKIP_DWMAPI:

    return TRUE;
}


INT WINAPI
_tWinMain(
    __in     HINSTANCE hInstance,
    __in_opt HINSTANCE hPrevInstance,
    __in     LPTSTR lpCmdLine,
    __in     INT nCmdShow
    )
{
    INT wParam = 0;

    if (InitializeModule() &&
        InitializeParameter(lpCmdLine) &&
        InitializeWindow() &&
        InitializeD3D9() &&
        InitializeDXVA2() &&
        InitializeTimer())
    {
        wParam = MessageLoop();
    }

    DestroyTimer();
    DestroyDXVA2();
    DestroyD3D9();

    return wParam;
}


