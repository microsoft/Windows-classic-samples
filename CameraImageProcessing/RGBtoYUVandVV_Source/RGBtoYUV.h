// RGBtoYUV.h: interface for the RGBtoYUV class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_RGBTOYUV_H__999B80FB_D5EF_4955_895A_8189521E2D93__INCLUDED_)
#define AFX_RGBTOYUV_H__999B80FB_D5EF_4955_895A_8189521E2D93__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


const CString LUMA_ONLY = L"Y";
const CString CB_CR     = L"CbCr";
const CString CB_ONLY   = L"Cb";
const CString CR_ONLY   = L"Cr";
const CString Y_CB_CR   = L"YCbCr";

const CString F_444     = L"4:4:4";
const CString F_440     = L"4:4:0";
const CString F_422     = L"4:2:2";
const CString F_420     = L"4:2:0";
const CString F_411     = L"4:1:1";
const CString F_410     = L"4:1:0";

class RGBtoYUV
{

public:

    RGBtoYUV();
    ~RGBtoYUV();
    bool CovertRGBtoYUV( byte* pbyData_i, int nWidth_i, int nHeight_i, CString csChromaSampling_i );
    byte* ConvertYUVtoRGB( )
    {
        YUVtoRGB();
        return m_pbyRGBOut;
    }

    byte* GetYData()
    {
        return m_pbyYdata;
    }
    byte* GetCbData( int& nWidth_o, int& nHeight_o );
    byte* GetCrData( int& nWidth_o, int& nHeight_o );

private:

    void YUVtoRGB();


private:

    byte* m_pbyYdata;
    byte* m_pbyCbData;
    byte* m_pbyCrData;
    byte* m_pbyCrDataOut;
    byte* m_pbyCbDataOut;
    byte* m_pbyRGBOut;

    int m_nWidth;
    int m_nHeight;
    int m_nWidthCbCr;
    int m_nHeightCbCr;
    int m_nChromaV; // Veritcal Pixel count out of 4
    int m_nChromaH; // Horizontal Pixel count out of 4
};

#endif // !defined(AFX_RGBTOYUV_H__999B80FB_D5EF_4955_895A_8189521E2D93__INCLUDED_)
