//-------------------------------------------------------------------
//
// Conversion functions
//
//-------------------------------------------------------------------

//-----include header files, 引入標頭檔-----
#include "Conversion.h"

//-----funtion implementation, 副程式實作-----
__forceinline BYTE Clip(int clr)
{
	return (BYTE)(clr < 0 ? 0 : (clr > 255 ? 255 : clr));
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

	rgbq.rgbRed = Clip((298 * c + 409 * e + 128) >> 8);
	rgbq.rgbGreen = Clip((298 * c - 100 * d - 208 * e + 128) >> 8);
	rgbq.rgbBlue = Clip((298 * c + 516 * d + 128) >> 8);

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
	const BYTE* lpBitsCb = lpBitsY + (dwHeightInPixels * srcStride);;
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
		lpBitsY += (2 * srcStride);
		lpBitsCr += srcStride;
		lpBitsCb += srcStride;
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
	int iSrcWidth = Width(rcSrc);
	int iSrcHeight = Height(rcSrc);

	int iDstWidth = Width(rcDst);
	int iDstHeight = Height(rcDst);

	int iDstLBWidth;
	int iDstLBHeight;

	if (MulDiv(iSrcWidth, iDstHeight, iSrcHeight) <= iDstWidth) {

		// Column letter boxing ("pillar box")

		iDstLBWidth = MulDiv(iDstHeight, iSrcWidth, iSrcHeight);
		iDstLBHeight = iDstHeight;
	}
	else {

		// Row letter boxing.

		iDstLBWidth = iDstWidth;
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
	RECT rc = { 0, 0, src.right - src.left, src.bottom - src.top };

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

inline LONG Width(const RECT& r)
{
	return r.right - r.left;
}

inline LONG Height(const RECT& r)
{
	return r.bottom - r.top;
}

/*	YUVtoRGB24 function is proformed to convert the YUV image to RGB image
	input parameter is (const YUV image, const YUV encoding type)
	return ()
	YUVtoRGB24函數用於將YUV格式圖像轉換為RGB格式圖像
	輸入參數為(YUV圖像常數資料, YUV圖像編碼方式)

*/
//unsigned char* YUVtoRGB24()
