// RGBtoYUV.cpp: implementation of the RGBtoYUV class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "RGBtoYUV.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

double dconversionmatrix[3][3] = { 0.299, 0.587, 0.114, /* Y */
                                   -0.14317, -0.28886, 0.436, /* Cr */
                                   0.615, -0.51499, -0.10001 }; /* Cb */

double dDecodematrix[3][3] = { 1, 0, 1.13983, /* R */
                                   1, -0.39465, -0.58060, /* G */
                                   1, 2.03211, 0 }; /* B */


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

RGBtoYUV::RGBtoYUV():
m_pbyYdata( 0 ),
m_pbyCbData( 0 ),
m_pbyCrData( 0 ),
m_nWidth( 0 ),
m_nHeight( 0 ),
m_nWidthCbCr( 0 ),
m_nHeightCbCr( 0 ),
m_pbyCrDataOut( 0 ),
m_pbyCbDataOut( NULL ),
m_pbyRGBOut( NULL )
{

}

RGBtoYUV::~RGBtoYUV()
{
    delete[] m_pbyYdata;
    delete[] m_pbyCbData;
    delete[] m_pbyCrData;
    delete[] m_pbyCbDataOut;
    delete[] m_pbyCrDataOut;
    delete[] m_pbyRGBOut;
}

bool RGBtoYUV::CovertRGBtoYUV( byte* pbyData_i, int nWidth_i, int nHeight_i,
                               CString csChromaSampling_i )
{
    if( 0 == pbyData_i )
    {
        return false;
    }

    m_nWidth        = nWidth_i;
    m_nHeight       = nHeight_i;


    if( F_444 == csChromaSampling_i )
    {
        m_nChromaV    = 4;
        m_nChromaH    = 4;
    }
    else if( F_440 == csChromaSampling_i )
    {
        m_nChromaV    = 2;
        m_nChromaH    = 4;
    }
    if( F_422 == csChromaSampling_i )
    {
        m_nChromaV    = 4;
        m_nChromaH    = 2;
    }
    else if( F_420 == csChromaSampling_i )
    {
        m_nChromaV    = 2;
        m_nChromaH    = 2;
    }
    if( F_411 == csChromaSampling_i )
    {
        m_nChromaV    = 4;
        m_nChromaH    = 1;
    }
    else if( F_410 == csChromaSampling_i )
    {
        m_nChromaV    = 2;
        m_nChromaH    = 1;
    }

    m_nWidthCbCr  = m_nWidth * ( m_nChromaH / 4.0 );
    m_nHeightCbCr = m_nHeight * ( m_nChromaV / 4.0 );

    delete[] m_pbyYdata;
    delete[] m_pbyCbData;
    delete[] m_pbyCrData;

    m_pbyYdata  = new byte[m_nWidth*m_nWidth*3];
    m_pbyCbData = new byte[m_nWidth*m_nWidth*3];
    m_pbyCrData = new byte[m_nWidth*m_nWidth*3];

    int nYoutPos    = 0;
    for( int nCol = 0; nCol < m_nHeight; nCol++ )
    {
        for( int nWid = 0; nWid < ( m_nWidth * 3 ); nWid += 3 )
        {
            int nR = pbyData_i[( nCol * m_nWidth * 3 ) + nWid + 2];
            int nG = pbyData_i[( nCol * m_nWidth *3 ) + nWid + 1];
            int nB = pbyData_i[( nCol * m_nWidth *3 ) + nWid ];

            // ITU-R version formula
            m_pbyYdata[nYoutPos] = dconversionmatrix[0][0] * nR
                                   + dconversionmatrix[0][1] * nG
                                   + dconversionmatrix[0][2] * nB; // B

            m_pbyCbData[nYoutPos] = dconversionmatrix[1][0] * nR
                   + dconversionmatrix[1][1] * nG
                   + dconversionmatrix[1][2] * nB + 128;

            m_pbyCrData[nYoutPos] = dconversionmatrix[2][0] * nR
                   + dconversionmatrix[2][1] * nG
                   + dconversionmatrix[2][2] * nB + 128;


               nYoutPos++;

               m_pbyYdata[nYoutPos]  = m_pbyYdata[nYoutPos-1] ; //G
               m_pbyCbData[nYoutPos]  = m_pbyCbData[nYoutPos-1] ;
               m_pbyCrData[nYoutPos]  = m_pbyCrData[nYoutPos-1] ;

               nYoutPos++;

               m_pbyYdata[nYoutPos]  = m_pbyYdata[nYoutPos-1] ; // R
              m_pbyCbData[nYoutPos]  = m_pbyCbData[nYoutPos-1] ;
              m_pbyCrData[nYoutPos]  = m_pbyCrData[nYoutPos-1] ;

               nYoutPos++;

        }
    }
    return true;
}

byte* RGBtoYUV::GetCbData( int& nWidth_o, int& nHeight_o )
{
    int nInPos    = 0;
    int nOutPos  = 0;
    int nFactor = 4/ m_nChromaH;
    int nFactorV = 4/ m_nChromaV;

   delete[] m_pbyCbDataOut;
   nWidth_o = m_nWidthCbCr;
   nHeight_o = m_nHeightCbCr;
   m_pbyCbDataOut  = new byte[m_nWidthCbCr*m_nHeightCbCr*3];
   ZeroMemory( m_pbyCbDataOut,  m_nWidthCbCr*m_nHeightCbCr*3  );

    for( int nCol = 0; nCol < m_nHeight; nCol++ )
    {
        if( 0 != ( nCol % nFactorV ))
        {
            continue;
        }
        for( int nWid = 0; nWid < ( m_nWidth * 3 );  )
        {
                int nCbValue = 0;
                for( int nCount = 0; nCount < (nFactor ) ; nCount++ )
                {
                        nCbValue += m_pbyCbData[( nCol * m_nWidth * 3 ) + nWid ];
                        nWid +=  3;
                }

                for( int nCount = 0; nCount < 3  ; nCount++ )
                {
                        m_pbyCbDataOut[nOutPos+nCount] = nCbValue / nFactor;
                }
                nOutPos +=  3;
        }
    }

    nOutPos = 0;
    if( 2 == nFactorV )
    {
        nOutPos = 0;
        for( int nCol = 0; nCol < ( m_nHeight - 1 ); nCol += 2 )
        {
            for( int nWid = 0; nWid < ( m_nWidth * 3 );  )
            {
                    int nCbValue = 0;
                    nCbValue += m_pbyCbData[( nCol * m_nWidth * 3 ) + nWid ];
                    nCbValue += m_pbyCbData[(( nCol + 1 ) * m_nWidth * 3 ) + nWid ];

                    for( int nCount = 0; nCount < 3  ; nCount++ )
                    {
                            m_pbyCbDataOut[nOutPos+nCount] = nCbValue / 2;
                    }
                    nWid += (nFactor * 3 );
                    nOutPos   += 3;
            }
        }
    }

    return m_pbyCbDataOut;
}

byte* RGBtoYUV::GetCrData( int& nWidth_o, int& nHeight_o )
{
    int nInPos    = 0;
    int nOutPos  = 0;
    int nFactor = 4/ m_nChromaH;
    int nFactorV = 4/ m_nChromaV;

   delete[] m_pbyCrDataOut;
   nWidth_o = m_nWidthCbCr;
   nHeight_o = m_nHeightCbCr;
   m_pbyCrDataOut  = new byte[m_nWidthCbCr*m_nHeightCbCr*3];
   ZeroMemory( m_pbyCrDataOut,  m_nWidthCbCr*m_nHeightCbCr*3  );

    for( int nCol = 0; nCol < m_nHeight; nCol++ )
    {
        if( 0 != ( nCol % nFactorV ))
        {
            continue;
        }
        for( int nWid = 0; nWid < ( m_nWidth * 3 );  )
        {
                int nCbValue = 0;
                for( int nCount = 0; nCount < (nFactor ) ; nCount++ )
                {
                        nCbValue += m_pbyCrData[( nCol * m_nWidth * 3 ) + nWid ];
                        nWid +=  3;
                }

                for( int nCount = 0; nCount < 3  ; nCount++ )
                {
                        m_pbyCrDataOut[nOutPos+nCount] = nCbValue / nFactor;
                }
                nOutPos +=  3;
        }
    }

    nOutPos = 0;
    if( 2 == nFactorV )
    {
        nOutPos = 0;
        for( int nCol = 0; nCol < ( m_nHeight - 1 ); nCol += 2 )
        {
            for( int nWid = 0; nWid < ( m_nWidth * 3 );  )
            {
                    int nCbValue = 0;
                    nCbValue += m_pbyCrData[( nCol * m_nWidth * 3 ) + nWid ];
                    nCbValue += m_pbyCrData[(( nCol + 1 ) * m_nWidth * 3 ) + nWid ];

                    for( int nCount = 0; nCount < 3  ; nCount++ )
                    {
                            m_pbyCrDataOut[nOutPos+nCount] = nCbValue / 2;
                    }
                    nWid += (nFactor * 3 );
                    nOutPos   += 3;
            }
        }
    }

    return m_pbyCrDataOut;
}

void RGBtoYUV::YUVtoRGB()
{

    int nWidth  = 0;
    int nHeight = 0;
    GetCbData( nWidth, nHeight );
    GetCrData(  nWidth, nHeight );

    int nFactorW = 4/ m_nChromaH;
    int nFactorV = 4/ m_nChromaV;

   delete[] m_pbyRGBOut;
   m_pbyRGBOut  = new byte[m_nWidth*m_nHeight*3];
   ZeroMemory( m_pbyRGBOut, m_nWidth*m_nHeight*3 );

    int nYoutPos    = 0;
    int nCrHeigth = 0;
    for( int nCol = 0; nCol < m_nHeight; nCol++ )
    {

        int nCrWidth = 0;
        for( int nWid = 0; nWid < ( m_nWidth * 3 ); nWid += 3 )
        {

            int nY       = m_pbyYdata[( nCol * m_nWidth * 3 ) + nWid ];
            int nCb    = m_pbyCbDataOut[( nCrHeigth * m_nWidthCbCr *3 ) + nCrWidth] - 128;
            int nCr     = m_pbyCrDataOut[( nCrHeigth * m_nWidthCbCr *3 ) + nCrWidth ] - 128;

            if(0 == ( nWid % nFactorW ))
            {
                nCrWidth += 3;
            }

            // ITU-R version formula
            m_pbyRGBOut[nYoutPos+2] = dDecodematrix[0][0] * nY
                                   + dDecodematrix[0][1] * nCb
                                   + dDecodematrix[0][2] * nCr; // R


            m_pbyRGBOut[nYoutPos+1] = dDecodematrix[1][0] * nY
                                   + dDecodematrix[1][1] * nCb
                                   + dDecodematrix[1][2] * nCr; // G


            m_pbyRGBOut[nYoutPos] = dDecodematrix[2][0] * nY
                                   + dDecodematrix[2][1] * nCb
                                   + dDecodematrix[2][2] * nCr; // G

               nYoutPos += 3;
        }
        if( 0 == ( nCol % nFactorV ))
        {
            nCrHeigth++;
        }
    }
}

