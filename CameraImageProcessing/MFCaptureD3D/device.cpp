//////////////////////////////////////////////////////////////////////////
//
// device.cpp: Manages the Direct3D device
// 
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//////////////////////////////////////////////////////////////////////////
#include "ImageProcessing\\MainImageProcessing.h"
#include "BufferLock.h"


const DWORD NUM_BACK_BUFFERS = 2;

void TransformImage_RGB24(
    BYTE*       pDest,
    LONG        lDestStride,
    const BYTE* pSrc,
    LONG        lSrcStride,
    DWORD       dwWidthInPixels,
    DWORD       dwHeightInPixels
    );

void TransformImage_RGB32(
    BYTE*       pDest,
    LONG        lDestStride,
    const BYTE* pSrc,
    LONG        lSrcStride,
    DWORD       dwWidthInPixels,
    DWORD       dwHeightInPixels
    );

void TransformImage_YUY2(
    BYTE*       pDest,
    LONG        lDestStride,
    const BYTE* pSrc,
    LONG        lSrcStride,
    DWORD       dwWidthInPixels,
    DWORD       dwHeightInPixels
    );

void TransformImage_NV12(
    BYTE* pDst, 
    LONG dstStride, 
    const BYTE* pSrc, 
    LONG srcStride,
    DWORD dwWidthInPixels,
    DWORD dwHeightInPixels
    );


RECT    LetterBoxRect(const RECT& rcSrc, const RECT& rcDst);
RECT    CorrectAspectRatio(const RECT& src, const MFRatio& srcPAR);
HRESULT GetDefaultStride(IMFMediaType *pType, LONG *plStride);




// Static table of output formats and conversion functions.
struct ConversionFunction
{
    GUID               subtype;
    IMAGE_TRANSFORM_FN xform;
};


ConversionFunction   g_FormatConversions[] =
{
    { MFVideoFormat_RGB32, TransformImage_RGB32 },
    { MFVideoFormat_RGB24, TransformImage_RGB24 },
    { MFVideoFormat_YUY2,  TransformImage_YUY2  },      
    { MFVideoFormat_NV12,  TransformImage_NV12  }
};

const DWORD   g_cFormats = ARRAYSIZE(g_FormatConversions);


//-------------------------------------------------------------------
// Constructor
//-------------------------------------------------------------------

DrawDevice::DrawDevice() : 
    m_hwnd(NULL),
    m_pD3D(NULL),
    m_pDevice(NULL),
    m_pSwapChain(NULL),
    m_format(D3DFMT_UNKNOWN),
    m_width(0),
    m_height(0),
    m_lDefaultStride(0),
    m_interlace(MFVideoInterlace_Unknown),
    m_convertFn(NULL)
{
    m_PixelAR.Denominator = m_PixelAR.Numerator = 1; 

    ZeroMemory(&m_d3dpp, sizeof(m_d3dpp));
}


//-------------------------------------------------------------------
// Destructor
//-------------------------------------------------------------------

DrawDevice::~DrawDevice()
{
    DestroyDevice();
}


//-------------------------------------------------------------------
// GetFormat
//
// Get a supported output format by index.
//-------------------------------------------------------------------

HRESULT DrawDevice::GetFormat(DWORD index, GUID *pSubtype) const
{
    if (index < g_cFormats)
    {
        *pSubtype = g_FormatConversions[index].subtype;
        return S_OK;
    }
    return MF_E_NO_MORE_TYPES;
}


//-------------------------------------------------------------------
//  IsFormatSupported
//
//  Query if a format is supported.
//-------------------------------------------------------------------

BOOL DrawDevice::IsFormatSupported(REFGUID subtype) const
{
    for (DWORD i = 0; i < g_cFormats; i++)
    {
        if (subtype == g_FormatConversions[i].subtype)
        {
            return TRUE;
        }
    }
    return FALSE;
}




//-------------------------------------------------------------------
// CreateDevice
//
// Create the Direct3D device.
//-------------------------------------------------------------------

HRESULT DrawDevice::CreateDevice(HWND hwnd)
{
    if (m_pDevice)
    {
        return S_OK;
    }

    // Create the Direct3D object.
    if (m_pD3D == NULL)
    {
        m_pD3D = Direct3DCreate9(D3D_SDK_VERSION);

        if (m_pD3D == NULL)
        {
            return E_FAIL;
        }
    }


    HRESULT hr = S_OK;
    D3DPRESENT_PARAMETERS pp = { 0 };
    D3DDISPLAYMODE mode = { 0 };

    hr = m_pD3D->GetAdapterDisplayMode(
        D3DADAPTER_DEFAULT, 
        &mode
        );

    if (FAILED(hr)) { goto done; }

    hr = m_pD3D->CheckDeviceType(
        D3DADAPTER_DEFAULT,
        D3DDEVTYPE_HAL,
        mode.Format,
        D3DFMT_X8R8G8B8,
        TRUE    // windowed
        );

    if (FAILED(hr)) { goto done; }

    pp.BackBufferFormat = D3DFMT_X8R8G8B8;
    pp.SwapEffect = D3DSWAPEFFECT_COPY;
    pp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;  
    pp.Windowed = TRUE;
    pp.hDeviceWindow = hwnd;

    hr = m_pD3D->CreateDevice(
        D3DADAPTER_DEFAULT,
        D3DDEVTYPE_HAL,
        hwnd,
        D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_FPU_PRESERVE,
        &pp,
        &m_pDevice
        );

    if (FAILED(hr)) { goto done; }

    m_hwnd = hwnd;
    m_d3dpp = pp;

done:
    return hr;
}

//-------------------------------------------------------------------
// SetConversionFunction
//
// Set the conversion function for the specified video format.
//-------------------------------------------------------------------

HRESULT DrawDevice::SetConversionFunction(REFGUID subtype)
{
    m_convertFn = NULL;

    for (DWORD i = 0; i < g_cFormats; i++)
    {
        if (g_FormatConversions[i].subtype == subtype)
        {
            m_convertFn = g_FormatConversions[i].xform;
            return S_OK;
        }
    }

    return MF_E_INVALIDMEDIATYPE;
}


//-------------------------------------------------------------------
// SetVideoType
//
// Set the video format.  
//-------------------------------------------------------------------

HRESULT DrawDevice::SetVideoType(IMFMediaType *pType)
{
    HRESULT hr = S_OK;
    GUID subtype = { 0 };
    MFRatio PAR = { 0 };

    // Find the video subtype.
    hr = pType->GetGUID(MF_MT_SUBTYPE, &subtype);

    if (FAILED(hr)) { goto done; }

    // Choose a conversion function.
    // (This also validates the format type.)

    hr = SetConversionFunction(subtype); 
    
    if (FAILED(hr)) { goto done; }

    //
    // Get some video attributes.
    //

    // Get the frame size.
    hr = MFGetAttributeSize(pType, MF_MT_FRAME_SIZE, &m_width, &m_height);
    
    if (FAILED(hr)) { goto done; }

    
    // Get the interlace mode. Default: assume progressive.
    m_interlace = (MFVideoInterlaceMode)MFGetAttributeUINT32(
        pType,
        MF_MT_INTERLACE_MODE, 
        MFVideoInterlace_Progressive
        );

    // Get the image stride.
    hr = GetDefaultStride(pType, &m_lDefaultStride);

    if (FAILED(hr)) { goto done; }

    // Get the pixel aspect ratio. Default: Assume square pixels (1:1)
    hr = MFGetAttributeRatio(
        pType, 
        MF_MT_PIXEL_ASPECT_RATIO, 
        (UINT32*)&PAR.Numerator, 
        (UINT32*)&PAR.Denominator
        );

    if (SUCCEEDED(hr))
    {
        m_PixelAR = PAR;
    }
    else
    {
        m_PixelAR.Numerator = m_PixelAR.Denominator = 1;
    }

    m_format = (D3DFORMAT)subtype.Data1;

    // Create Direct3D swap chains.

    hr = CreateSwapChains();

    if (FAILED(hr)) { goto done; }


    // Update the destination rectangle for the correct
    // aspect ratio.

    UpdateDestinationRect();

done:
    if (FAILED(hr))
    {
        m_format = D3DFMT_UNKNOWN;
        m_convertFn = NULL;
    }
    return hr;
}

//-------------------------------------------------------------------
//  UpdateDestinationRect
//
//  Update the destination rectangle for the current window size.
//  The destination rectangle is letterboxed to preserve the 
//  aspect ratio of the video image.
//-------------------------------------------------------------------

void DrawDevice::UpdateDestinationRect()
{
    RECT rcClient;
    RECT rcSrc = { 0, 0, m_width, m_height };

    GetClientRect(m_hwnd, &rcClient);

    rcSrc = CorrectAspectRatio(rcSrc, m_PixelAR);

    m_rcDest = LetterBoxRect(rcSrc, rcClient);
}


//-------------------------------------------------------------------
// CreateSwapChains
//
// Create Direct3D swap chains.
//-------------------------------------------------------------------

HRESULT DrawDevice::CreateSwapChains()
{
    HRESULT hr = S_OK;

    D3DPRESENT_PARAMETERS pp = { 0 };

    SafeRelease(&m_pSwapChain);

    pp.BackBufferWidth  = m_width;
    pp.BackBufferHeight = m_height;
    pp.Windowed = TRUE;
    pp.SwapEffect = D3DSWAPEFFECT_FLIP;
    pp.hDeviceWindow = m_hwnd;
    pp.BackBufferFormat = D3DFMT_X8R8G8B8;
    pp.Flags = 
        D3DPRESENTFLAG_VIDEO | D3DPRESENTFLAG_DEVICECLIP |
        D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
    pp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
    pp.BackBufferCount = NUM_BACK_BUFFERS;

    hr = m_pDevice->CreateAdditionalSwapChain(&pp, &m_pSwapChain);

    return hr;
}


//-------------------------------------------------------------------
// DrawFrame
//
// Draw the video frame.
//-------------------------------------------------------------------

HRESULT DrawDevice::DrawFrame(IMFMediaBuffer *pBuffer)
{
    if (m_convertFn == NULL)
    {
        return MF_E_INVALIDREQUEST;
    }

    HRESULT hr = S_OK;
    BYTE *pbScanline0 = NULL;
    LONG lStride = 0;
    D3DLOCKED_RECT lr;

    IDirect3DSurface9 *pSurf = NULL;
    IDirect3DSurface9 *pBB = NULL;

    if (m_pDevice == NULL || m_pSwapChain == NULL)
    {
        return S_OK;
    }

    VideoBufferLock buffer(pBuffer);    // Helper object to lock the video buffer.

    hr = TestCooperativeLevel();

    if (FAILED(hr)) { goto done; }

    // Lock the video buffer. This method returns a pointer to the first scan
    // line in the image, and the stride in bytes.

    hr = buffer.LockBuffer(m_lDefaultStride, m_height, &pbScanline0, &lStride);

    if (FAILED(hr)) { goto done; }

    // Get the swap-chain surface.
    hr = m_pSwapChain->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &pSurf);

    if (FAILED(hr)) { goto done; }

    // Lock the swap-chain surface.
    hr = pSurf->LockRect(&lr, NULL, D3DLOCK_NOSYSLOCK );

    if (FAILED(hr)) { goto done; }


    // Convert the frame. This also copies it to the Direct3D surface.
    
    m_convertFn(
        (BYTE*)lr.pBits,
        lr.Pitch,
        pbScanline0,
        lStride,
        m_width,
        m_height
        );
	
	MainImageProcessing((unsigned char *)lr.pBits, m_width, m_height);

    hr = pSurf->UnlockRect();

    if (FAILED(hr)) { goto done; }


    // Color fill the back buffer.
    hr = m_pDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pBB);

    if (FAILED(hr)) { goto done; }

    hr = m_pDevice->ColorFill(pBB, NULL, D3DCOLOR_XRGB(0, 0, 0));

    if (FAILED(hr)) { goto done; }


    // Blit the frame.

    hr = m_pDevice->StretchRect(pSurf, NULL, pBB, &m_rcDest, D3DTEXF_LINEAR);
    
    if (FAILED(hr)) { goto done; }


    // Present the frame.
    
    hr = m_pDevice->Present(NULL, NULL, NULL, NULL);
    

done:
    SafeRelease(&pBB);
    SafeRelease(&pSurf);
    return hr;
}

//-------------------------------------------------------------------
// TestCooperativeLevel
//
// Test the cooperative-level status of the Direct3D device.
//-------------------------------------------------------------------

HRESULT DrawDevice::TestCooperativeLevel()
{
    if (m_pDevice == NULL)
    {
        return E_FAIL;
    }

    HRESULT hr = S_OK;

    // Check the current status of D3D9 device.
    hr = m_pDevice->TestCooperativeLevel();

    switch (hr)
    {
    case D3D_OK:
        break;

    case D3DERR_DEVICELOST:
        hr = S_OK;

    case D3DERR_DEVICENOTRESET:
        hr = ResetDevice();
        break;

    default:
        // Some other failure.
        break;
    }

    return hr;
}


//-------------------------------------------------------------------
// ResetDevice
//
// Resets the Direct3D device.
//-------------------------------------------------------------------

HRESULT DrawDevice::ResetDevice()
{
    HRESULT hr = S_OK;

    if (m_pDevice)
    {
        D3DPRESENT_PARAMETERS d3dpp = m_d3dpp;

        hr = m_pDevice->Reset(&d3dpp);

        if (FAILED(hr))
        {
            DestroyDevice();
        }
    }

    if (m_pDevice == NULL)
    {
        hr = CreateDevice(m_hwnd);

        if (FAILED(hr)) { goto done; }
    }

    if ((m_pSwapChain == NULL) && (m_format != D3DFMT_UNKNOWN))
    {
        hr = CreateSwapChains();
        
        if (FAILED(hr)) { goto done; }

        UpdateDestinationRect();
    }

done:

   return hr;
}


//-------------------------------------------------------------------
// DestroyDevice 
//
// Release all Direct3D resources.
//-------------------------------------------------------------------

void DrawDevice::DestroyDevice()
{
    SafeRelease(&m_pSwapChain);
    SafeRelease(&m_pDevice);
    SafeRelease(&m_pD3D);
}



