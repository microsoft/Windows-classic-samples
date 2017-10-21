// BitmapClass.cpp: implementation of the BitmapClass class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BitmapClass.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

BitmapClass::BitmapClass():
m_pbyPixelData( NULL ),
m_pby32PixelData( NULL )
{
    ZeroMemory( &m_BMPFileHeader, sizeof( BITMAPFILEHEADER ));
    ZeroMemory( &m_BMPInfoHeader, sizeof( BITMAPINFOHEADER ));
}

BitmapClass::~BitmapClass()
{
    delete[] m_pbyPixelData;
}

bool BitmapClass::Open( CString cFilePath_i)
{
    FILE* fp = NULL;
    fp = _wfopen( cFilePath_i, L"rb" );
    if( NULL == fp )
    {
        AfxMessageBox( L"Invalid File ");
        return false;
    }

    if( !ReadBMPFileHeaderInformations( fp ))
    {
        AfxMessageBox( L"Failed to read BMPHeader Informations ");
        return false;
    }

    fseek( fp, 14, 0 ); // size of BITMAPFILEHEADER

    if( !ReadBMPInfoHeaderInformations( fp ))
    {
        AfxMessageBox( L"Failed to read BMPInfoHeader Informations ");
        return false;
    }

    fseek( fp, 0, 0 ); // Seek to origin;
    ReadPixelData( fp );
    //Create32BitData();
    return true;
}

bool BitmapClass::ReadBMPFileHeaderInformations( const void* ptr_i )
{
    FILE* pData = (FILE*)ptr_i;

    int nSeek  = 0;
    // Signature 2Byte Long
    fread( &m_BMPFileHeader.bfType, 2, 1, pData );

    // Total File size 4Byte Long
    fread( &m_BMPFileHeader.bfSize, 4, 1, pData );

    // 2 Reserved field of 2Bytes[ total 4Byte] Long
    fread( &m_BMPFileHeader.bfReserved1, 2, 1, pData );
    fread( &m_BMPFileHeader.bfReserved2, 2, 1, pData );

    // Read the offset to pixel data 4 Bytes Long
    fread( &m_BMPFileHeader.bfOffBits, 4, 1, pData );

    return true;
}

bool BitmapClass::ReadBMPInfoHeaderInformations( const void* ptr_i )
{
    FILE* pData = (FILE*)ptr_i;

    int nSeek  = 0;

    // BMPINFOHEADER structure size 4Byte Long
    fread( &m_BMPInfoHeader.biSize, 4, 1, pData );

    // Image Width 4Byte Long
    fread( &m_BMPInfoHeader.biWidth, 4, 1, pData );

    // Image Height 4Byte Long
    fread( &m_BMPInfoHeader.biHeight, 4, 1, pData );

    // Number of clolr planes 2Byte Long :- Must be set to 1
    fread( &m_BMPInfoHeader.biPlanes, 2, 1, pData );

    // Number of bits per pixel 2Byte Long
    fread( &m_BMPInfoHeader.biBitCount, 2, 1, pData );

    // Compression method 4Byte Long : 0  for BI_RGB is common
    fread( &m_BMPInfoHeader.biCompression, 4, 1, pData );

    // Image Size 4Byte Long
    fread( &m_BMPInfoHeader.biSizeImage, 4, 1, pData );

    // Horizontal resolution 4Byte Long
    fread( &m_BMPInfoHeader.biXPelsPerMeter, 4, 1, pData );

    // Vertical resolution 4Byte Long
    fread( &m_BMPInfoHeader.biYPelsPerMeter, 4, 1, pData );

    // the number of colors in the color palette, or 0 to default to 2 toth power of n,  4Byte Long
    fread( &m_BMPInfoHeader.biClrUsed, 4, 1, pData );

    // the number of important colors used, or 0 when every color is important; generally ignored. 4Byte Long
    fread( &m_BMPInfoHeader.biClrImportant, 4, 1, pData );
    return true;

}

void BitmapClass::ReadPixelData( const void* ptr_i )
{
    FILE* pData = (FILE*)ptr_i;
    fseek( pData, m_BMPFileHeader.bfOffBits, 0 );

    delete[] m_pbyPixelData;
    int nSize = m_BMPInfoHeader.biSizeImage;

    if( 0 == nSize )
    {
        nSize = m_BMPInfoHeader.biWidth * m_BMPInfoHeader.biHeight * m_BMPInfoHeader.biBitCount /8;
    }
    m_pbyPixelData = new byte[nSize];
    ZeroMemory( m_pbyPixelData, nSize );

    fread( m_pbyPixelData, nSize, 1, pData );

    /*FILE* Fp = NULL;
    Fp = fopen( "c:\\test.bin", "wb" );
    fwrite( m_pbyPixelData, nSize, 1, Fp );
    fclose( Fp );*/
}

void BitmapClass::Create32BitData()
{
    delete[] m_pby32PixelData;
    int nMultiple = 8;

    switch( m_BMPInfoHeader.biBitCount )
    {
        case 32:
            nMultiple = 0;
            break;

        case 24:
            nMultiple = 8;
            break;

        case 16:
            nMultiple = 16;
            break;

        case  8:
            nMultiple = 24;
            break;
    }

    int nSize = m_BMPInfoHeader.biSizeImage * nMultiple;
    if( 0 == nSize )
    {
        nSize = m_BMPInfoHeader.biWidth * m_BMPInfoHeader.biHeight * nMultiple;
    }

    m_pby32PixelData = new byte[nSize];
    ZeroMemory( m_pby32PixelData, nSize );

    int nOutCount = 0;
    for( int nHeight = m_BMPInfoHeader.biHeight; nHeight > 0; nHeight-- )
    {
        nOutCount= (m_BMPInfoHeader.biHeight - nHeight ) * m_BMPInfoHeader.biWidth * 4;
        int nCount = ( nHeight * m_BMPInfoHeader.biWidth * 3 );
        for( int nWidth = 0 ; nWidth <  m_BMPInfoHeader.biWidth; nWidth += 3 )
        {
            m_pby32PixelData[nOutCount++] = m_pbyPixelData[nCount++];
            m_pby32PixelData[nOutCount++] = m_pbyPixelData[nCount++];
            m_pby32PixelData[nOutCount++] = m_pbyPixelData[nCount++];
            m_pby32PixelData[nOutCount++] = 0;
        }
    }
}

void BitmapClass::Draw( HWND hwnd, int nXStart_i, int nYStart_i, int nWidth_i, int nHeight_i )
{

    RECT Rect; // just a simple rect to hold the size of our window

    HDC hdc = ::GetDC( hwnd ); 

    if( 0 == m_BMPInfoHeader.biHeight )
    {
        m_BMPInfoHeader.biPlanes = 1;
        m_BMPInfoHeader.biSize = sizeof(m_BMPInfoHeader); // size of this struct
        m_BMPInfoHeader.biHeight = nHeight_i - 3;
        m_BMPInfoHeader.biWidth  = nWidth_i - 3;
        m_BMPInfoHeader.biBitCount = 24;
        m_BMPInfoHeader.biCompression = BI_RGB;

        delete[] m_pbyPixelData;
        int nSize = m_BMPInfoHeader.biWidth * m_BMPInfoHeader.biHeight * m_BMPInfoHeader.biBitCount/8;
        m_pbyPixelData = new byte[nSize];
        memset( m_pbyPixelData, 100, nSize );
    }
    BITMAPINFO biBitMapInfo; // We set this up to grab what we want
    biBitMapInfo.bmiHeader = m_BMPInfoHeader;

    HDC iDC = CreateCompatibleDC(0); // create dc to store the image

    HBITMAP iBitmap = CreateDIBSection( iDC, &biBitMapInfo, DIB_RGB_COLORS, 0, 0, 0); // create a dib section for the dc
    SelectObject(iDC, iBitmap); // assign the dib section to the dc

    SetDIBitsToDevice( iDC, 0, 0, biBitMapInfo.bmiHeader.biWidth, biBitMapInfo.bmiHeader.biHeight,
                       0, 0, 0, biBitMapInfo.bmiHeader.biHeight, &m_pbyPixelData[0], &biBitMapInfo,
                       DIB_RGB_COLORS); // set the new dibs to the dc

  //  if((( nWidth_i - 3)  < biBitMapInfo.bmiHeader.biWidth ) ||
    //   (( nHeight_i - 3 ) < biBitMapInfo.bmiHeader.biHeight ))
    {
       // StretchBlt( hdc, 0, 0, nWidth_i - 3, nHeight_i - 3, iDC, 0, 0,
              //  biBitMapInfo.bmiHeader.biWidth, biBitMapInfo.bmiHeader.biHeight, SRCCOPY );
    }
   // else
    {
        BitBlt( hdc, nXStart_i, nYStart_i, nWidth_i - 3, nHeight_i + 3, iDC, 0, 0, SRCCOPY); // copy hdc to their hdc
    }

    DeleteDC( iDC ); // delete dc
    DeleteObject( iBitmap ); // delete object
    ::ReleaseDC( hwnd, hdc );
}

void BitmapClass::ResetDisplay( HWND hwnd, int nXStart_i, int nYStart_i, int nWidth_i, int nHeight_i )
{
    m_BMPInfoHeader.biHeight = 0;
    Draw( hwnd, nXStart_i, nYStart_i, nWidth_i, nHeight_i );
}

void BitmapClass::CreateBitmap( const byte* ptr_i, int& nWidth_i, int& nHeight_i, IMAGE_TYPE eImageType_i )
{
    delete[] m_pbyPixelData;
    int nBitCount = 0;

    if( BMP_24 == eImageType_i )
    {
        m_BMPInfoHeader.biCompression = BI_RGB;
        nBitCount = 3;
    }
    else if( BMP_32 == eImageType_i )
    {
       // m_BMPInfoHeader.biCompression = BI_RGBA;
        nBitCount = 4;
    }

    int nSize = nWidth_i * nHeight_i * nBitCount;

    m_pbyPixelData = new byte[nSize];
    ZeroMemory( m_pbyPixelData, nSize );

    memcpy( m_pbyPixelData, ptr_i, nSize );

    m_BMPInfoHeader.biPlanes = 1;
    m_BMPInfoHeader.biSize = sizeof(m_BMPInfoHeader); // size of this struct
    m_BMPInfoHeader.biHeight = nHeight_i;
    m_BMPInfoHeader.biWidth  = nWidth_i;
    m_BMPInfoHeader.biBitCount = nBitCount * 8;
}