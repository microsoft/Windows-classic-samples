/*++

Copyright (c) Microsoft Corporation. All rights reserved.

--*/

#ifndef __BITMAPUTIL__
#define __BITMAPUTIL__

namespace BitmapUtil 
{

//////////////////////////////////////////////////////////////////////////
//
// GetBitmapHeaderSize
//

/*++

    The GetBitmapHeaderSize function returns the size of the DIB header


    ULONG 
    GetBitmapHeaderSize(
        LPCVOID pDib
    );

Parameters

    pDib
        [in] Pointer to the in-memory DIB that can be represented by
        BITMAPCOREHEADER, BITMAPINFOHEADER, BITMAPV4HEADER or BITMAPV5HEADER.


Return Values
    
    Returns the size of the DIB header in bytes or 0 if the header is
    not recognized.

--*/

ULONG 
GetBitmapHeaderSize(
    LPCVOID pDib
);


//////////////////////////////////////////////////////////////////////////
//
// GetBitmapLineWidthInBytes
//

/*++

    The GetBitmapLineWidthInBytes function returns the number of bytes
    in one scan line of the image


    ULONG 
    GetBitmapLineWidthInBytes(
        ULONG nWidthInPixels, 
        ULONG nBitCount
    );

Parameters

    nWidthInPixels
        [in] Width of a scan line in pixels

    nBitCount
        [in] Number of bits per pixel


Return Values
    
    Returns the size one scan line of the image in bytes.

--*/

ULONG 
GetBitmapLineWidthInBytes(
    ULONG nWidthInPixels, 
    ULONG nBitCount
);


//////////////////////////////////////////////////////////////////////////
//
// GetBitmapDimensions
//

/*++

    The GetBitmapDimensions function returns the width and height of a DIB


    BOOL  
    GetBitmapDimensions(
        LPCVOID  pDib, 
        UINT    *pWidth, 
        UINT    *pHeight
    );


Parameters

    pDib
        [in] Pointer to the in-memory DIB that can be represented by
        BITMAPCOREHEADER, BITMAPINFOHEADER, BITMAPV4HEADER or BITMAPV5HEADER.

    pWidth
        [out] Receives the width of the image

    pHeight
        [out] Receives the height of the image


Return Values
    
    Returns TRUE if the header is recognized, FALSE otherwise.

--*/

BOOL  
GetBitmapDimensions(
    LPCVOID  pDib, 
    UINT    *pWidth, 
    UINT    *pHeight
);


//////////////////////////////////////////////////////////////////////////
//
// GetBitmapSize
//

/*++

    The GetBitmapSize function returns total size of the DIB. The size is
    the sum of the bitmap header, the color palette (if present), the color 
    profile data (if present) and the pixel data.


    ULONG 
    GetBitmapSize(
        LPCVOID pDib
    );


Parameters

    pDib
        [in] Pointer to the in-memory DIB that can be represented by
        BITMAPCOREHEADER, BITMAPINFOHEADER, BITMAPV4HEADER or BITMAPV5HEADER.


Return Values
    
    Returns the size of the image in bytes or 0 if the header is not 
    recognized.

--*/

ULONG 
GetBitmapSize(
    LPCVOID pDib
);


//////////////////////////////////////////////////////////////////////////
//
// GetBitmapOffsetBits
//

/*++

    The GetBitmapOffsetBits function returns the offset, in bytes, from the 
    beginning of the DIB data block to the bitmap bits.


    ULONG 
    GetBitmapOffsetBits(
        LPCVOID pDib
    );


Parameters

    pDib
        [in] Pointer to the in-memory DIB that can be represented by
        BITMAPCOREHEADER, BITMAPINFOHEADER, BITMAPV4HEADER or BITMAPV5HEADER.


Return Values
    
    Returns the offset from the beginning of the DIB data block to the bitmap 
    pixels in bytes or 0 if the header is not recognized.

--*/

ULONG 
GetBitmapOffsetBits(
    LPCVOID pDib
);


//////////////////////////////////////////////////////////////////////////
//
// FixBitmapHeight
//

/*++

    The FixBitmapHeight function calculates the height of the DIB if the 
    height is not specified in the header and fills in the biSizeImage and 
    biHeight fields of the header.


    BOOL  
    FixBitmapHeight(
        PVOID pDib, 
        ULONG nSize, 
        BOOL  bTopDown
    );


Parameters

    pDib
        [in] Pointer to the in-memory DIB that can be represented by
        BITMAPCOREHEADER, BITMAPINFOHEADER, BITMAPV4HEADER or BITMAPV5HEADER.

    nSize
        [in] The total size of the image in bytes.

    bTopDown
        [in] TRUE if the first scan line in the memory corresponds to the top 
        line of the image, FALSE if it corresponds to the bottom line.


Return Values
    
    Returns TRUE if the header is recognized, FALSE otherwise.

--*/

BOOL  
FixBitmapHeight(
    PVOID pDib, 
    ULONG nSize, 
    BOOL  bTopDown
);


//////////////////////////////////////////////////////////////////////////
//
// FillBitmapFileHeader
//

/*++

    The FillBitmapFileHeader function fills in a BITMAPFILEHEADER structure 
    according to the values specified in the DIB.


    BOOL  
    FillBitmapFileHeader(
        LPCVOID           pDib,
        PBITMAPFILEHEADER pbmfh 
    );


Parameters

    pDib
        [in] Pointer to the in-memory DIB that can be represented by
        BITMAPCOREHEADER, BITMAPINFOHEADER, BITMAPV4HEADER or BITMAPV5HEADER.

    pbmfh
        [out] Receives the BITMAPFILEHEADER structure filled with the
        values specified in the DIB


Return Values
    
    Returns TRUE if the header is recognized, FALSE otherwise.

--*/

BOOL  
FillBitmapFileHeader(
    LPCVOID           pDib,
    PBITMAPFILEHEADER pbmfh 
);

}; // namespace BitmapUtil

#endif //__BITMAPUTIL__

