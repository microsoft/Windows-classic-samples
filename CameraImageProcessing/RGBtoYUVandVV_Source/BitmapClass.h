// BitmapClass.h: interface for the BitmapClass class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BITMAPCLASS_H__82B78722_38FB_4501_AEA0_8256E93FF534__INCLUDED_)
#define AFX_BITMAPCLASS_H__82B78722_38FB_4501_AEA0_8256E93FF534__INCLUDED_



#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

typedef enum
{
    BMP_24,
    BMP_32,
    BMP_8
}IMAGE_TYPE;

class BitmapClass
{

public:

    BitmapClass();
    ~BitmapClass();
    bool Open( CString cFilePath_i );
    void Draw( HWND hwnd, int nXStart_i, int nYStart_i, int nWidth_i, int nHeight_i );
    void ResetDisplay( HWND hwnd, int nXStart_i, int nYStart_i, int nWidth_i, int nHeight_i );
    void CreateBitmap( const byte* ptr_i, int& nWidth_i, int& nHeight_i, IMAGE_TYPE eImageType_i );

    int  Width()
    {
        return m_BMPInfoHeader.biWidth;
    };
    int  Heigth()
    {
        return m_BMPInfoHeader.biHeight;
    };
    unsigned char*  Data()
    {
        return m_pbyPixelData;
    }

private:

    bool ReadBMPFileHeaderInformations( const void* ptr_i );
    bool ReadBMPInfoHeaderInformations( const void* ptr_i );
    void ReadPixelData( const void* ptr_i );
    void Create32BitData( );

    BITMAPFILEHEADER    m_BMPFileHeader;
    BITMAPINFOHEADER    m_BMPInfoHeader;
    byte*               m_pbyPixelData;
    byte*               m_pby32PixelData;
};

#endif // !defined(AFX_BITMAPCLASS_H__82B78722_38FB_4501_AEA0_8256E93FF534__INCLUDED_)
