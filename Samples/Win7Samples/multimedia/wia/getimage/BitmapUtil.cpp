/*++

Copyright (c) Microsoft Corporation. All rights reserved.

--*/

#include "stdafx.h"
#include "resource.h"

#include "BitmapUtil.h"

namespace BitmapUtil 
{

//////////////////////////////////////////////////////////////////////////
//
// GetBitmapHeaderSize
//

ULONG GetBitmapHeaderSize(LPCVOID pDib)
{
    ULONG nHeaderSize = *(PDWORD) pDib;

    switch (nHeaderSize)
	{
		case sizeof(BITMAPCOREHEADER):
        case sizeof(BITMAPINFOHEADER):
		case sizeof(BITMAPV4HEADER):
		case sizeof(BITMAPV5HEADER):
		{
			return nHeaderSize;
		}
    }

    return 0;
}


//////////////////////////////////////////////////////////////////////////
//
// GetBitmapLineWidthInBytes
//

ULONG GetBitmapLineWidthInBytes(ULONG nWidthInPixels, ULONG nBitCount)
{
    return (((nWidthInPixels * nBitCount) + 31) & ~31) >> 3;
}


//////////////////////////////////////////////////////////////////////////
//
// GetBitmapDimensions
//

BOOL GetBitmapDimensions(LPCVOID pDib, UINT *pWidth, UINT *pHeight)
{
    ULONG nHeaderSize = GetBitmapHeaderSize(pDib);

    if (nHeaderSize == 0)
    {
        return FALSE;
    }

    if (nHeaderSize == sizeof(BITMAPCOREHEADER))
    {
        PBITMAPCOREHEADER pbmch = (PBITMAPCOREHEADER) pDib;

        if (pWidth != NULL)
        {
            *pWidth  = pbmch->bcWidth;
        }

        if (pHeight != NULL)
        {
            *pHeight = pbmch->bcHeight;
        }
    }
    else
    {
        PBITMAPINFOHEADER pbmih = (PBITMAPINFOHEADER) pDib;

        if (pWidth != NULL)
        {
            *pWidth  = pbmih->biWidth;
        }

        if (pHeight != NULL)
        {
            *pHeight = abs(pbmih->biHeight);
        }
    }

    return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
// GetBitmapSize
//

ULONG GetBitmapSize(LPCVOID pDib)
{
    ULONG nHeaderSize = GetBitmapHeaderSize(pDib);

    if (nHeaderSize == 0)
    {
        return 0;
    }

    // Start the calculation with the header size

    ULONG nDibSize = nHeaderSize;

    // is this an old style BITMAPCOREHEADER?

    if (nHeaderSize == sizeof(BITMAPCOREHEADER))
    {
        PBITMAPCOREHEADER pbmch = (PBITMAPCOREHEADER) pDib;

        // Add the color table size

        if (pbmch->bcBitCount <= 8)
        {
            nDibSize += sizeof(RGBTRIPLE) * (1i64 << pbmch->bcBitCount);
        }

        // Add the bitmap size

        ULONG nWidth = GetBitmapLineWidthInBytes(pbmch->bcWidth, pbmch->bcBitCount);

        nDibSize += nWidth * pbmch->bcHeight;
    }
    else
    {
        // this is at least a BITMAPINFOHEADER

        PBITMAPINFOHEADER pbmih = (PBITMAPINFOHEADER) pDib;

        // Add the color table size

        if (pbmih->biClrUsed != 0)
        {
            nDibSize += sizeof(RGBQUAD) * pbmih->biClrUsed;
        }
        else if (pbmih->biBitCount <= 8)
        {
            nDibSize += sizeof(RGBQUAD) * (1i64 << pbmih->biBitCount);
        }

        // Add the bitmap size

        if (pbmih->biSizeImage != 0)
        {
            nDibSize += pbmih->biSizeImage;
        }
        else
        {
            // biSizeImage must be specified for compressed bitmaps

            if (pbmih->biCompression != BI_RGB &&
                pbmih->biCompression != BI_BITFIELDS)
            {
                return 0;
            }

            ULONG nWidth = GetBitmapLineWidthInBytes(pbmih->biWidth, pbmih->biBitCount);

            nDibSize += nWidth * abs(pbmih->biHeight);
        }

        // Consider special cases

        if (nHeaderSize == sizeof(BITMAPINFOHEADER))
        {     
            // If this is a 16 or 32 bit bitmap and BI_BITFIELDS is used, 
            // bmiColors member contains three DWORD color masks.
            // For V4 or V5 headers, this info is included the header

            if (pbmih->biCompression == BI_BITFIELDS)
            {
                nDibSize += 3 * sizeof(DWORD);
            }
        }
        else if (nHeaderSize >= sizeof(BITMAPV5HEADER))
        {
            // If this is a V5 header and an ICM profile is specified,
            // we need to consider the profile data size
            
            PBITMAPV5HEADER pbV5h = (PBITMAPV5HEADER) pDib;

            // if there is some padding before the profile data, add it

            if (pbV5h->bV5ProfileData > nDibSize)
            {
                nDibSize = pbV5h->bV5ProfileData;
            }

            // add the profile data size

            nDibSize += pbV5h->bV5ProfileSize;
        }
    }

    return nDibSize;
}


//////////////////////////////////////////////////////////////////////////
//
// GetBitmapOffsetBits
//

ULONG GetBitmapOffsetBits(LPCVOID pDib)
{
    ULONG nHeaderSize = GetBitmapHeaderSize(pDib);

    if (nHeaderSize == 0)
    {
        return 0;
    }

    // Start the calculation with the header size

    ULONG nOffsetBits = nHeaderSize;

    // is this an old style BITMAPCOREHEADER?

    if (nHeaderSize == sizeof(BITMAPCOREHEADER))
    {
        PBITMAPCOREHEADER pbmch = (PBITMAPCOREHEADER) pDib;

        // Add the color table size

        if (pbmch->bcBitCount <= 8)
        {
            nOffsetBits += sizeof(RGBTRIPLE) * (1i64 << pbmch->bcBitCount);
        }
    }
    else
    {
        // this is at least a BITMAPINFOHEADER

        PBITMAPINFOHEADER pbmih = (PBITMAPINFOHEADER) pDib;

        // Add the color table size

        if (pbmih->biClrUsed != 0)
        {
            nOffsetBits += sizeof(RGBQUAD) * pbmih->biClrUsed;
        }
        else if (pbmih->biBitCount <= 8)
        {
            nOffsetBits += sizeof(RGBQUAD) * (1i64 << pbmih->biBitCount);
        }

        // Consider special cases

        if (nHeaderSize == sizeof(BITMAPINFOHEADER))
        {     
            // If this is a 16 or 32 bit bitmap and BI_BITFIELDS is used, 
            // bmiColors member contains three DWORD color masks.
            // For V4 or V5 headers, this info is included in the header

            if (pbmih->biCompression == BI_BITFIELDS)
            {
                nOffsetBits += 3 * sizeof(DWORD);
            }
        }
        else if (nHeaderSize >= sizeof(BITMAPV5HEADER))
        {
            // If this is a V5 header and an ICM profile is specified,
            // we need to consider the profile data size
            
            PBITMAPV5HEADER pbV5h = (PBITMAPV5HEADER) pDib;

            // if the profile data comes before the pixel data, add it

            if (pbV5h->bV5ProfileData <= nOffsetBits)
            {
                nOffsetBits += pbV5h->bV5ProfileSize;
            }
        }
    }

    return nOffsetBits;
}


//////////////////////////////////////////////////////////////////////////
//
// FixBitmapHeight
//

BOOL FixBitmapHeight(PVOID pDib, ULONG nSize, BOOL bTopDown)
{
    ULONG nHeaderSize = GetBitmapHeaderSize(pDib);

    if (nHeaderSize == 0)
    {
        return FALSE;
    }

    // is this an old style BITMAPCOREHEADER?

    if (nHeaderSize == sizeof(BITMAPCOREHEADER))
    {
        PBITMAPCOREHEADER pbmch = (PBITMAPCOREHEADER) pDib;

        // fix the height value if necessary

        if (pbmch->bcHeight == 0)
        {
            // start the calculation with the header size

            ULONG nSizeImage = nSize - nHeaderSize;

            // subtract the color table size

            if (pbmch->bcBitCount <= 8)
            {
                nSizeImage -= sizeof(RGBTRIPLE) * (1i64 << pbmch->bcBitCount);
            }

            // calculate the height

            ULONG nWidth = GetBitmapLineWidthInBytes(pbmch->bcWidth, pbmch->bcBitCount);

            if (nWidth == 0)
            {
                return FALSE;
            }

            LONG nHeight = nSizeImage / nWidth;

            pbmch->bcHeight = (WORD) nHeight;
        }
    }
    else
    {
        // this is at least a BITMAPINFOHEADER

        PBITMAPINFOHEADER pbmih = (PBITMAPINFOHEADER) pDib;

        // fix the height value if necessary

        if (pbmih->biHeight == 0)
        {
            // find the size of the image data

            ULONG nSizeImage;

            if (pbmih->biSizeImage != 0)
            {
                // if the size is specified in the header, take it

                nSizeImage = pbmih->biSizeImage;
            }
            else
            {
                // start the calculation with the header size

                nSizeImage = nSize - nHeaderSize;

                // subtract the color table size

                if (pbmih->biClrUsed != 0)
                {
                    nSizeImage -= sizeof(RGBQUAD) * pbmih->biClrUsed;
                }
                else if (pbmih->biBitCount <= 8)
                {
                    nSizeImage -= sizeof(RGBQUAD) * (1i64 << pbmih->biBitCount);
                }

                // Consider special cases

                if (nHeaderSize == sizeof(BITMAPINFOHEADER))
                {     
                    // If this is a 16 or 32 bit bitmap and BI_BITFIELDS is used, 
                    // bmiColors member contains three DWORD color masks.
                    // For V4 or V5 headers, this info is included the header

                    if (pbmih->biCompression == BI_BITFIELDS)
                    {
                        nSizeImage -= 3 * sizeof(DWORD);
                    }
                }
                else if (nHeaderSize >= sizeof(BITMAPV5HEADER))
                {
                    // If this is a V5 header and an ICM profile is specified,
                    // we need to consider the profile data size
            
                    PBITMAPV5HEADER pbV5h = (PBITMAPV5HEADER) pDib;

                    // add the profile data size

                    nSizeImage -= pbV5h->bV5ProfileSize;
                }

                // store the image size

                pbmih->biSizeImage = nSizeImage;
            }

            // finally, calculate the height

            ULONG nWidth = GetBitmapLineWidthInBytes(pbmih->biWidth, pbmih->biBitCount);

            if (nWidth == 0)
            {
                return FALSE;
            }

			LONG nHeight = nSizeImage / nWidth;

            pbmih->biHeight = bTopDown ? -nHeight : nHeight;
        }
    }

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
// FillBitmapFileHeader
//

BOOL FillBitmapFileHeader(LPCVOID pDib, PBITMAPFILEHEADER pbmfh)
{
    ULONG nSize = GetBitmapSize(pDib);

    if (nSize == 0)
    {
        return FALSE;
    }

    ULONG nOffset = GetBitmapOffsetBits(pDib);

    if (nOffset == 0)
    {
        return FALSE;
    }

    pbmfh->bfType      = MAKEWORD('B', 'M');
    pbmfh->bfSize      = sizeof(BITMAPFILEHEADER) + nSize;
    pbmfh->bfReserved1 = 0;
    pbmfh->bfReserved2 = 0;
    pbmfh->bfOffBits   = sizeof(BITMAPFILEHEADER) + nOffset;

    return TRUE;
}

}; // namespace BitmapUtil
