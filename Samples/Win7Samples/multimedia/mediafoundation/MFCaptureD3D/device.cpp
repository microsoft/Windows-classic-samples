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

#include "MFCaptureD3D.h"
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


inline LONG Width(const RECT& r)
{
    return r.right - r.left;
}

inline LONG Height(const RECT& r)
{
    return r.bottom - r.top;
}


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

    hr = pSurf->UnlockRect();

    if (FAILED(hr)) { goto done; }


    // Color fill the back buffer.
    hr = m_pDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pBB);

    if (FAILED(hr)) { goto done; }

    hr = m_pDevice->ColorFill(pBB, NULL, D3DCOLOR_XRGB(0, 0, 0x80));

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



//-------------------------------------------------------------------
//
// Conversion functions
//
//-------------------------------------------------------------------

__forceinline BYTE Clip(int clr)
{
    return (BYTE)(clr < 0 ? 0 : ( clr > 255 ? 255 : clr ));
}

__forceinline RGBQUAD ConvertYCrCbToRGB(
    int y,
    int cr,
    int cb
    )
{
    RGBQUAD rgbq;

    int c = y - 16;
    int d = cb - 128;
    int e = cr - 128;

    rgbq.rgbRed =   Clip(( 298 * c           + 409 * e + 128) >> 8);
    rgbq.rgbGreen = Clip(( 298 * c - 100 * d - 208 * e + 128) >> 8);
    rgbq.rgbBlue =  Clip(( 298 * c + 516 * d           + 128) >> 8);

    return rgbq;
}


//-------------------------------------------------------------------
// TransformImage_RGB24 
//
// RGB-24 to RGB-32
//-------------------------------------------------------------------

void TransformImage_RGB24(
    BYTE*       pDest,
    LONG        lDestStride,
    const BYTE* pSrc,
    LONG        lSrcStride,
    DWORD       dwWidthInPixels,
    DWORD       dwHeightInPixels
    )
{
    for (DWORD y = 0; y < dwHeightInPixels; y++)
    {
        RGBTRIPLE *pSrcPel = (RGBTRIPLE*)pSrc;
        DWORD *pDestPel = (DWORD*)pDest;

        for (DWORD x = 0; x < dwWidthInPixels; x++)
        {
            pDestPel[x] = D3DCOLOR_XRGB(
                pSrcPel[x].rgbtRed,
                pSrcPel[x].rgbtGreen,
                pSrcPel[x].rgbtBlue
                );
        }

        pSrc += lSrcStride;
        pDest += lDestStride;
    }
}

//-------------------------------------------------------------------
// TransformImage_RGB32
//
// RGB-32 to RGB-32 
//
// Note: This function is needed to copy the image from system
// memory to the Direct3D surface.
//-------------------------------------------------------------------

void TransformImage_RGB32(
    BYTE*       pDest,
    LONG        lDestStride,
    const BYTE* pSrc,
    LONG        lSrcStride,
    DWORD       dwWidthInPixels,
    DWORD       dwHeightInPixels
    )
{
    MFCopyImage(pDest, lDestStride, pSrc, lSrcStride, dwWidthInPixels * 4, dwHeightInPixels);
}

//-------------------------------------------------------------------
// TransformImage_YUY2 
//
// YUY2 to RGB-32
//-------------------------------------------------------------------

void TransformImage_YUY2(
    BYTE*       pDest,
    LONG        lDestStride,
    const BYTE* pSrc,
    LONG        lSrcStride,
    DWORD       dwWidthInPixels,
    DWORD       dwHeightInPixels
    )
{
    for (DWORD y = 0; y < dwHeightInPixels; y++)
    {
        RGBQUAD *pDestPel = (RGBQUAD*)pDest;
        WORD    *pSrcPel = (WORD*)pSrc;

        for (DWORD x = 0; x < dwWidthInPixels; x += 2)
        {
            // Byte order is U0 Y0 V0 Y1

            int y0 = (int)LOBYTE(pSrcPel[x]); 
            int u0 = (int)HIBYTE(pSrcPel[x]);
            int y1 = (int)LOBYTE(pSrcPel[x + 1]);
            int v0 = (int)HIBYTE(pSrcPel[x + 1]);

            pDestPel[x] = ConvertYCrCbToRGB(y0, v0, u0);
            pDestPel[x + 1] = ConvertYCrCbToRGB(y1, v0, u0);
        }

        pSrc += lSrcStride;
        pDest += lDestStride;
    }

}


//-------------------------------------------------------------------
// TransformImage_NV12
//
// NV12 to RGB-32
//-------------------------------------------------------------------

void TransformImage_NV12(
    BYTE* pDst, 
    LONG dstStride, 
    const BYTE* pSrc, 
    LONG srcStride,
    DWORD dwWidthInPixels,
    DWORD dwHeightInPixels
    )
{
    const BYTE* lpBitsY = pSrc;
    const BYTE* lpBitsCb = lpBitsY  + (dwHeightInPixels * srcStride);;
    const BYTE* lpBitsCr = lpBitsCb + 1;

    for (UINT y = 0; y < dwHeightInPixels; y += 2)
    {
        const BYTE* lpLineY1 = lpBitsY;
        const BYTE* lpLineY2 = lpBitsY + srcStride;
        const BYTE* lpLineCr = lpBitsCr;
        const BYTE* lpLineCb = lpBitsCb;

        LPBYTE lpDibLine1 = pDst;
        LPBYTE lpDibLine2 = pDst + dstStride;

        for (UINT x = 0; x < dwWidthInPixels; x += 2)
        {
            int  y0 = (int)lpLineY1[0];
            int  y1 = (int)lpLineY1[1];
            int  y2 = (int)lpLineY2[0];
            int  y3 = (int)lpLineY2[1];
            int  cb = (int)lpLineCb[0];
            int  cr = (int)lpLineCr[0];

            RGBQUAD r = ConvertYCrCbToRGB(y0, cr, cb);
            lpDibLine1[0] = r.rgbBlue;
            lpDibLine1[1] = r.rgbGreen;
            lpDibLine1[2] = r.rgbRed;
            lpDibLine1[3] = 0; // Alpha

            r = ConvertYCrCbToRGB(y1, cr, cb);
            lpDibLine1[4] = r.rgbBlue;
            lpDibLine1[5] = r.rgbGreen;
            lpDibLine1[6] = r.rgbRed;
            lpDibLine1[7] = 0; // Alpha

            r = ConvertYCrCbToRGB(y2, cr, cb);
            lpDibLine2[0] = r.rgbBlue;
            lpDibLine2[1] = r.rgbGreen;
            lpDibLine2[2] = r.rgbRed;
            lpDibLine2[3] = 0; // Alpha

            r = ConvertYCrCbToRGB(y3, cr, cb);
            lpDibLine2[4] = r.rgbBlue;
            lpDibLine2[5] = r.rgbGreen;
            lpDibLine2[6] = r.rgbRed;
            lpDibLine2[7] = 0; // Alpha

            lpLineY1 += 2;
            lpLineY2 += 2;
            lpLineCr += 2;
            lpLineCb += 2;

            lpDibLine1 += 8;
            lpDibLine2 += 8;
        }

        pDst += (2 * dstStride);
        lpBitsY   += (2 * srcStride);
        lpBitsCr  += srcStride;
        lpBitsCb  += srcStride;
    }
}


//-------------------------------------------------------------------
// LetterBoxDstRect
//
// Takes a src rectangle and constructs the largest possible 
// destination rectangle within the specifed destination rectangle 
// such thatthe video maintains its current shape.
//
// This function assumes that pels are the same shape within both the 
// source and destination rectangles.
//
//-------------------------------------------------------------------

RECT    LetterBoxRect(const RECT& rcSrc, const RECT& rcDst)
{
    // figure out src/dest scale ratios
    int iSrcWidth  = Width(rcSrc);
    int iSrcHeight = Height(rcSrc);

    int iDstWidth  = Width(rcDst);
    int iDstHeight = Height(rcDst);

    int iDstLBWidth;
    int iDstLBHeight;

    if (MulDiv(iSrcWidth, iDstHeight, iSrcHeight) <= iDstWidth) {

        // Column letter boxing ("pillar box")

        iDstLBWidth  = MulDiv(iDstHeight, iSrcWidth, iSrcHeight);
        iDstLBHeight = iDstHeight;
    }
    else {

        // Row letter boxing.

        iDstLBWidth  = iDstWidth;
        iDstLBHeight = MulDiv(iDstWidth, iSrcHeight, iSrcWidth);
    }


    // Create a centered rectangle within the current destination rect

    RECT rc;

    LONG left = rcDst.left + ((iDstWidth - iDstLBWidth) / 2);
    LONG top = rcDst.top + ((iDstHeight - iDstLBHeight) / 2);

    SetRect(&rc, left, top, left + iDstLBWidth, top + iDstLBHeight);

    return rc;
}


//-----------------------------------------------------------------------------
// CorrectAspectRatio
//
// Converts a rectangle from the source's pixel aspect ratio (PAR) to 1:1 PAR.
// Returns the corrected rectangle.
//
// For example, a 720 x 486 rect with a PAR of 9:10, when converted to 1x1 PAR,  
// is stretched to 720 x 540. 
//-----------------------------------------------------------------------------

RECT CorrectAspectRatio(const RECT& src, const MFRatio& srcPAR)
{
    // Start with a rectangle the same size as src, but offset to the origin (0,0).
    RECT rc = {0, 0, src.right - src.left, src.bottom - src.top};

    if ((srcPAR.Numerator != 1) || (srcPAR.Denominator != 1))
    {
        // Correct for the source's PAR.

        if (srcPAR.Numerator > srcPAR.Denominator)
        {
            // The source has "wide" pixels, so stretch the width.
            rc.right = MulDiv(rc.right, srcPAR.Numerator, srcPAR.Denominator);
        }
        else if (srcPAR.Numerator < srcPAR.Denominator)
        {
            // The source has "tall" pixels, so stretch the height.
            rc.bottom = MulDiv(rc.bottom, srcPAR.Denominator, srcPAR.Numerator);
        }
        // else: PAR is 1:1, which is a no-op.
    }
    return rc;
}


//-----------------------------------------------------------------------------
// GetDefaultStride
//
// Gets the default stride for a video frame, assuming no extra padding bytes.
//
//-----------------------------------------------------------------------------

HRESULT GetDefaultStride(IMFMediaType *pType, LONG *plStride)
{
    LONG lStride = 0;

    // Try to get the default stride from the media type.
    HRESULT hr = pType->GetUINT32(MF_MT_DEFAULT_STRIDE, (UINT32*)&lStride);
    if (FAILED(hr))
    {
        // Attribute not set. Try to calculate the default stride.
        GUID subtype = GUID_NULL;

        UINT32 width = 0;
        UINT32 height = 0;

        // Get the subtype and the image size.
        hr = pType->GetGUID(MF_MT_SUBTYPE, &subtype);
        if (SUCCEEDED(hr))
        {
            hr = MFGetAttributeSize(pType, MF_MT_FRAME_SIZE, &width, &height);
        }
        if (SUCCEEDED(hr))
        {
            hr = MFGetStrideForBitmapInfoHeader(subtype.Data1, width, &lStride);
        }

        // Set the attribute for later reference.
        if (SUCCEEDED(hr))
        {
            (void)pType->SetUINT32(MF_MT_DEFAULT_STRIDE, UINT32(lStride));
        }
    }

    if (SUCCEEDED(hr))
    {
        *plStride = lStride;
    }
    return hr;
}