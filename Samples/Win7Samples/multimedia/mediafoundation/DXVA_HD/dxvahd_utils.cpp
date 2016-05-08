//////////////////////////////////////////////////////////////////////
//
// DXVA-HD Helpers
// 
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//////////////////////////////////////////////////////////////////////

#include "DXVAHD_Sample.h"


//-------------------------------------------------------------------
// DXVAHD_SetStreamFormat
//
// Sets the video format for a specified video stream.
//-------------------------------------------------------------------

HRESULT DXVAHD_SetStreamFormat(IDXVAHD_VideoProcessor *pVP, UINT stream, D3DFORMAT format)
{
    DXVAHD_STREAM_STATE_D3DFORMAT_DATA d3dformat = { format };

    HRESULT hr = pVP->SetVideoProcessStreamState(
        stream,
        DXVAHD_STREAM_STATE_D3DFORMAT,
        sizeof(d3dformat),
        &d3dformat
        );

    return hr;
}


//-------------------------------------------------------------------
// DXVAHD_SetFrameFormat
//
// Sets the interlace mode for a specified video stream.
//-------------------------------------------------------------------

HRESULT DXVAHD_SetFrameFormat(IDXVAHD_VideoProcessor *pVP, UINT stream, DXVAHD_FRAME_FORMAT format)
{
    DXVAHD_STREAM_STATE_FRAME_FORMAT_DATA frame_format = { format };
    
    HRESULT hr = pVP->SetVideoProcessStreamState(
        stream,
        DXVAHD_STREAM_STATE_FRAME_FORMAT,
        sizeof(frame_format),
        &frame_format
        );

    return hr;
}


//-------------------------------------------------------------------
// DXVAHD_SetDestinationRect
//
// Sets the destination rectangle for a specified video stream.
//-------------------------------------------------------------------

HRESULT DXVAHD_SetDestinationRect(IDXVAHD_VideoProcessor *pVP, UINT stream, BOOL bEnable, const RECT &rect)
{
    DXVAHD_STREAM_STATE_DESTINATION_RECT_DATA DstRect = { bEnable, rect };

    HRESULT hr = pVP->SetVideoProcessStreamState(
        stream, 
        DXVAHD_STREAM_STATE_DESTINATION_RECT,
        sizeof(DstRect),
        &DstRect
        );
    
    return hr;
}


//-------------------------------------------------------------------
// DXVAHD_SetSourceRect
//
// Sets the source rectangle for a specified video stream.
//-------------------------------------------------------------------

HRESULT DXVAHD_SetSourceRect(IDXVAHD_VideoProcessor *pVP, UINT stream, BOOL bEnable, const RECT& rect)
{
    DXVAHD_STREAM_STATE_SOURCE_RECT_DATA src = { bEnable, rect };

    HRESULT hr = pVP->SetVideoProcessStreamState(
        stream,
        DXVAHD_STREAM_STATE_SOURCE_RECT,
        sizeof(src),
        &src
        );

    return hr;
}
                 

//-------------------------------------------------------------------
// DXVAHD_SetLumaKey
//
// Sets the luma key for a specified video stream.
//-------------------------------------------------------------------

HRESULT DXVAHD_SetLumaKey(IDXVAHD_VideoProcessor *pVP, UINT stream, BOOL bEnable, float fLower, float fUpper)
{
    DXVAHD_STREAM_STATE_LUMA_KEY_DATA luma = { bEnable, fLower, fUpper };

    HRESULT hr = pVP->SetVideoProcessStreamState(
        stream,
        DXVAHD_STREAM_STATE_LUMA_KEY,
        sizeof(luma),
        &luma
        );

    return hr;
}


//-------------------------------------------------------------------
// DXVAHD_SetPlanarAlpha
//
// Sets the planar alpha for a specified video stream.
//-------------------------------------------------------------------

HRESULT DXVAHD_SetPlanarAlpha(IDXVAHD_VideoProcessor *pVP, UINT stream, BOOL bEnable, float fAlpha)
{
    DXVAHD_STREAM_STATE_ALPHA_DATA alpha = { bEnable, fAlpha };

    HRESULT hr = pVP->SetVideoProcessStreamState(
        stream,
        DXVAHD_STREAM_STATE_ALPHA,
        sizeof(alpha),
        &alpha
        );

    return hr;
}


//-------------------------------------------------------------------
// DXVAHD_SetFilterValue
//
// Sets a video processing filter for a specified video stream.
//-------------------------------------------------------------------

HRESULT DXVAHD_SetFilterValue(IDXVAHD_VideoProcessor *pVP, UINT stream, DXVAHD_FILTER filter, BOOL bEnable, INT value)
{
    DXVAHD_STREAM_STATE_FILTER_DATA data = { bEnable, value };

    DXVAHD_STREAM_STATE state = static_cast<DXVAHD_STREAM_STATE>(DXVAHD_STREAM_STATE_FILTER_BRIGHTNESS + filter);

    HRESULT hr = pVP->SetVideoProcessStreamState(
        stream, 
        state,
        sizeof(data),
        &data
        );

    return hr;
}


//-------------------------------------------------------------------
// DXVAHD_SetInputColorSpace
//
// Sets the input color space for a specified video stream.
//-------------------------------------------------------------------

HRESULT DXVAHD_SetInputColorSpace(
    IDXVAHD_VideoProcessor *pVP, 
    UINT stream,
    BOOL bPlayback,     // TRUE = playback, FALSE = video processing 
    UINT RGB_Range,     // 0 = 0-255, 1 = 16-235
    UINT YCbCr_Matrix,  // 0 = BT.601, 1 = BT.709
    UINT YCbCr_xvYCC    // 0 = Conventional YCbCr, 1 = xvYCC
    )
{
    DXVAHD_STREAM_STATE_INPUT_COLOR_SPACE_DATA data = 
    {
        bPlayback ? 0 : 1,
        RGB_Range ? 1 : 0,
        YCbCr_Matrix ? 1 : 0,
        YCbCr_xvYCC ? 1 : 0
    };

    HRESULT hr = pVP->SetVideoProcessStreamState(
        stream,
        DXVAHD_STREAM_STATE_INPUT_COLOR_SPACE,
        sizeof(data),
        &data
        );

    return hr;
}


//-------------------------------------------------------------------
// DXVAHD_SetOutputColorSpace
//
// Sets the output color space for a specified video stream.
//-------------------------------------------------------------------

HRESULT DXVAHD_SetOutputColorSpace(
    IDXVAHD_VideoProcessor *pVP, 
    BOOL bPlayback,     // TRUE = playback, FALSE = video processing 
    UINT RGB_Range,     // 0 = 0-255, 1 = 16-235
    UINT YCbCr_Matrix,  // 0 = BT.601, 1 = BT.709
    UINT YCbCr_xvYCC    // 0 = Conventional YCbCr, 1 = xvYCC
    )
{
    DXVAHD_BLT_STATE_OUTPUT_COLOR_SPACE_DATA data = 
    {
        bPlayback ? 0 : 1,
        RGB_Range ? 1 : 0,
        YCbCr_Matrix ? 1 : 0,
        YCbCr_xvYCC ? 1 : 0
    };

    HRESULT hr = pVP->SetVideoProcessBltState(
        DXVAHD_BLT_STATE_OUTPUT_COLOR_SPACE,
        sizeof(data),
        &data
        );

    return hr;
}


//-------------------------------------------------------------------
// DXVAHD_SetBackgroundColor
//
// Sets the background color. 
//-------------------------------------------------------------------

HRESULT DXVAHD_SetBackgroundColor(
    IDXVAHD_VideoProcessor *pVP, 
    BOOL bYCbCr, 
    const DXVAHD_COLOR& color
    )
{
    DXVAHD_BLT_STATE_BACKGROUND_COLOR_DATA data = { bYCbCr, color };

    HRESULT hr = pVP->SetVideoProcessBltState(
        DXVAHD_BLT_STATE_BACKGROUND_COLOR,
        sizeof (data),
        &data
        );

    return hr;
}


//-------------------------------------------------------------------
// DXVAHD_SetTargetRect
//
// Sets the target rectangle.
//-------------------------------------------------------------------

HRESULT DXVAHD_SetTargetRect(
    IDXVAHD_VideoProcessor *pVP, 
    BOOL bEnable,
    const RECT &rect
    )
{
    DXVAHD_BLT_STATE_TARGET_RECT_DATA tr = { bEnable, rect };

    HRESULT hr = pVP->SetVideoProcessBltState(
        DXVAHD_BLT_STATE_TARGET_RECT,
        sizeof(tr),
        &tr
        );

    return hr;
}


