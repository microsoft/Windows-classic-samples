//////////////////////////////////////////////////////////////////////////
//
// device.h: Manages the Direct3D device
// 
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//////////////////////////////////////////////////////////////////////////


#pragma once

// Function pointer for the function that transforms the image.

typedef void (*IMAGE_TRANSFORM_FN)(
    BYTE*       pDest,
    LONG        lDestStride,
    const BYTE* pSrc,
    LONG        lSrcStride,
    DWORD       dwWidthInPixels,
    DWORD       dwHeightInPixels
    );


// DrawDevice class

class DrawDevice
{
private:

    HWND                    m_hwnd;

    IDirect3D9              *m_pD3D;
    IDirect3DDevice9        *m_pDevice;
    IDirect3DSwapChain9     *m_pSwapChain;

    D3DPRESENT_PARAMETERS   m_d3dpp;

    // Format information
    D3DFORMAT               m_format;
    UINT                    m_width;
    UINT                    m_height;
    LONG                    m_lDefaultStride;
    MFRatio                 m_PixelAR;
    MFVideoInterlaceMode    m_interlace;
    RECT                    m_rcDest;       // Destination rectangle

    // Drawing
    IMAGE_TRANSFORM_FN      m_convertFn;    // Function to convert the video to RGB32

private:
    
    HRESULT TestCooperativeLevel();
    HRESULT SetConversionFunction(REFGUID subtype);
    HRESULT CreateSwapChains();
    void    UpdateDestinationRect();
    
public:

    DrawDevice();
    virtual ~DrawDevice();

    HRESULT CreateDevice(HWND hwnd);
    HRESULT ResetDevice();
    void    DestroyDevice();

    HRESULT SetVideoType(IMFMediaType *pType);
    HRESULT DrawFrame(IMFMediaBuffer *pBuffer);

    // What video formats we accept
    BOOL     IsFormatSupported(REFGUID subtype) const;
    HRESULT  GetFormat(DWORD index, GUID *pSubtype)  const;
};
