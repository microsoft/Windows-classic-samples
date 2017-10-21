//-----include header files, 引入標頭檔-----
#include <stdio.h>														//	include "stdio.h" header file, 引入標頭檔stdio.h
#include "..\\MFCaptureD3D.h"											//	include "MFCaptureD3D.h" header file, 引入標頭檔MFCaptureD3D.h
#include "..\\BufferLock.h"												//	include "BufferLock.h" header file, 引入標頭檔BufferLock.h

//-----funtion declaration, 副程式宣告區-----
__forceinline BYTE Clip(int clr);
__forceinline RGBQUAD ConvertYCrCbToRGB(
	int y,
	int cr,
	int cb
);
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
);

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
);

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
);

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
);

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

RECT    LetterBoxRect(const RECT& rcSrc, const RECT& rcDst);

//-----------------------------------------------------------------------------
// CorrectAspectRatio
//
// Converts a rectangle from the source's pixel aspect ratio (PAR) to 1:1 PAR.
// Returns the corrected rectangle.
//
// For example, a 720 x 486 rect with a PAR of 9:10, when converted to 1x1 PAR,  
// is stretched to 720 x 540. 
//-----------------------------------------------------------------------------

RECT CorrectAspectRatio(const RECT& src, const MFRatio& srcPAR);

//-----------------------------------------------------------------------------
// GetDefaultStride
//
// Gets the default stride for a video frame, assuming no extra padding bytes.
//
//-----------------------------------------------------------------------------

HRESULT GetDefaultStride(IMFMediaType *pType, LONG *plStride);

inline LONG Width(const RECT& r);

inline LONG Height(const RECT& r);