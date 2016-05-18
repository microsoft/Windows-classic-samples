//////////////////////////////////////////////////////////////////////
// 
// dxvahd_utils.h
//
// Helper functions for DXVA-HD
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

/*-------------------------------------------------------------------
 
  The functions declared in this header are simple wrappers around 
  two DXVA-HD methods:

  * IDXVAHD_VideoProcessor::SetVideoProcessStreamState
  * IDXVAHD_VideoProcessor::SetVideoProcessBltState 

  The SetVideoProcessStreamState method applies per-stream settings 
  for the video processing

  The SetVideoProcessBltState method applies settings for the 
  entire blit operation.

  Each method takes a state parameter (a enumeration value that
  specifies which state to set) and a data structure for that 
  state. 

-------------------------------------------------------------------*/

// Stream settings
HRESULT DXVAHD_SetStreamFormat(IDXVAHD_VideoProcessor *pVP, UINT stream, D3DFORMAT format);
HRESULT DXVAHD_SetFrameFormat(IDXVAHD_VideoProcessor *pVP, UINT stream, DXVAHD_FRAME_FORMAT format);
HRESULT DXVAHD_SetDestinationRect(IDXVAHD_VideoProcessor *pVP, UINT stream, BOOL bEnable, const RECT &rect);
HRESULT DXVAHD_SetSourceRect(IDXVAHD_VideoProcessor *pVP, UINT stream, BOOL bEnable, const RECT& rect);
HRESULT DXVAHD_SetLumaKey(IDXVAHD_VideoProcessor *pVP, UINT stream, BOOL bEnable, float fLower, float fUpper);
HRESULT DXVAHD_SetPlanarAlpha(IDXVAHD_VideoProcessor *pVP, UINT stream, BOOL bEnable, float fAlpha);
HRESULT DXVAHD_SetFilterValue(IDXVAHD_VideoProcessor *pVP, UINT stream, DXVAHD_FILTER filter, BOOL bEnable, INT value);
HRESULT DXVAHD_SetInputColorSpace(IDXVAHD_VideoProcessor *pVP, UINT stream, BOOL bPlayback, UINT RGB_Range, UINT YCbCr_Matrix, UINT YCbCr_xvYCC);

// Blit settings
HRESULT DXVAHD_SetOutputColorSpace(IDXVAHD_VideoProcessor *pVP, BOOL bPlayback, UINT RGB_Range, UINT YCbCr_Matrix, UINT YCbCr_xvYCC);
HRESULT DXVAHD_SetBackgroundColor(IDXVAHD_VideoProcessor *pVP, BOOL bYCbCr, const DXVAHD_COLOR& color);
HRESULT DXVAHD_SetTargetRect(IDXVAHD_VideoProcessor *pVP, BOOL bEnable, const RECT &rect);

