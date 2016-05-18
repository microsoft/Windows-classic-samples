//*****************************************************************************
//
// Microsoft Windows Media
// Copyright ( C) Microsoft Corporation. All rights reserved.
//
// FileName:            UncompAVIToWMV.h
//
// Abstract:            Interface of CUncompAVIToWMV class.
//
//*****************************************************************************

#if !defined(AFX_UncompAVIToWMV_H__21A57EE3_8C94_4E10_8092_4C171BCBA68E__INCLUDED_)
#define AFX_UncompAVIToWMV_H__21A57EE3_8C94_4E10_8092_4C171BCBA68E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <wmsdk.h>
#include <vfw.h>
#include "TSimpleList.h"

#ifndef SAFE_RELEASE
    #define SAFE_RELEASE( x )           \
        if ( NULL != x )                \
        {                               \
            x->Release( );              \
            x = NULL;                   \
        }
#endif // SAFE_RELEASE

#ifndef SAFE_ARRAYDELETE
    #define SAFE_ARRAYDELETE( x )       \
       if( x )                          \
       {                                \
           delete [] x;                 \
           x = NULL;                    \
       }
#endif //SAFE_ARRAYDELETE

#ifndef SAFE_CLOSEFILEHANDLE
    #define SAFE_CLOSEFILEHANDLE( h )   \
        if( INVALID_HANDLE_VALUE != h ) \
        {                               \
            CloseHandle( h );           \
            h = INVALID_HANDLE_VALUE;   \
        }
#endif //SAFE_CLOSEFILEHANDLE

////////////////////////////////////////////////////////////////////////////////
struct CONTENT_DESC
{
    CONTENT_DESC ()
    {
        wStreamNum = 0;
        ptszName = NULL;
        ptszValue = NULL;
    }

    WORD                wStreamNum;
    LPTSTR              ptszName;
    LPTSTR              ptszType;
    LPTSTR              ptszValue;
};

//
//  AVI input info
//
class CWMInput
{
public:
    CWMInput();
    ~CWMInput() {}; 

    //
    // Used by GetMinElement method of CTSimpleList< CWMInput >
    //
    bool operator<( const CWMInput & right )
    {
        return( m_qwPresentTime < right.m_qwPresentTime );
    }

    void    Cleanup();

    QWORD                   m_qwPresentTime;        // in 100 ns
    GUID                    m_Type;
    PAVIFILE                m_pAVIFile;
    PAVISTREAM              m_pAVIStream;
    DWORD                   m_dwInput;
    AVISTREAMINFO           m_StreamInfo;
    WM_MEDIA_TYPE           m_Mt;
    WAVEFORMATEX           *m_pWFX;
    DWORD                   m_dwSamples;
    DWORD                   m_dwCurrentSample;
    WCHAR                  *m_pwszConnectionName;
    BOOL                    m_fAddSMPTE;
};

// 
// AVI input list
//
typedef CTSimpleList< CWMInput > INPUTLIST;

//
// Input file list
//
typedef CTSimpleList< PAVIFILE > PAVILIST;

class CProfileStreams;

////////////////////////////////////////////////////////////////////////////////
class CUncompAVIToWMV
{
public:

    CUncompAVIToWMV();
    ~CUncompAVIToWMV();

    HRESULT Initial( __in LPTSTR pszInFile, 
                     BOOL        fInFileListFile,
                     __in LPTSTR pszOutFile, 
                     IWMProfile  * pProfile,
                     BOOL        fArbitrary,
                     BOOL        fPreserveProfile,
                     BOOL        fAddSMPTE,
                     int         nMaxDuration );
    
    static HRESULT ListSystemProfile();

    static HRESULT LoadSystemProfile( DWORD dwProfileIndex, 
                                      IWMProfile ** ppIWMProfile );

    static HRESULT LoadCustomProfile( LPCTSTR ptszProfileFile, 
                                      IWMProfile ** ppIWMProfile );

    static HRESULT CreateEmptyProfile( IWMProfile ** ppIWMProfile );

    static HRESULT SaveProfile( LPCTSTR ptszFileName, 
                                IWMProfile * pIWMProfile );

    HRESULT SetAttribute( CONTENT_DESC * pCntDesc );

    HRESULT SetDRM();

    HRESULT Start();

    void    SetPreprocessing( BOOL fPreprocessing) { m_fPreprocessing = fPreprocessing; }

protected:

    HRESULT SetStreamBasics( IWMStreamConfig * pIWMStreamConfig,
                             IWMProfile * pIWMProfile,
                             __in LPWSTR pwszStreamName,
                             __in LPWSTR pwszConnectionName,
                             DWORD dwBitrate,
                             WM_MEDIA_TYPE * pmt );

    HRESULT AddVideoStream( IWMProfile * pIWMProfile,
                            WMVIDEOINFOHEADER * pInputVIH,
                            WORD  * wStreamNum,
                            DWORD dwQuality,
                            DWORD dwSecPerKey,
                            __out LPWSTR * pwszConnectionName );

    HRESULT AddAudioStream( IWMProfile * pIWMProfile,
                            DWORD dwSampleRate,
                            DWORD dwChannels,
                            WORD wBitsPerSample,
                            WORD  * wStreamNum,
                            __out LPWSTR * pwszConnectionName );

    HRESULT GetTokensFromFile( LPCTSTR ptszFileName, 
                               __out_ecount(*TokenNum) LPTSTR pptszTokens[], 
                               int * TokenNum );

    HRESULT CreateProfileStreamList( IWMProfile * pProfile, 
                                     CTSimpleList< CProfileStreams > * pProfStreamList );

    HRESULT AddArbitraryStream( IWMProfile * pProfile );

    HRESULT UpdateProfile( IWMProfile * pProfile );

    HRESULT UpdateWriterInputs( );

    HRESULT InitAVISource( __in LPTSTR ptszInputFile );

    HRESULT DoPreprocessing( );

    HRESULT WriteSample( CWMInput * pCurStream );

    HRESULT SetupSMPTE( IWMProfile * pProfile );

    HRESULT AddSMPTETimeCode( INSSBuffer * pSample, QWORD qwPresTime );

    IWMWriter               * m_pWMWriter;
    IWMWriterPreprocess     * m_pIWMWriterPreprocess;

    DWORD                   m_dwArbitraryInput;
    QWORD                   m_qwSMPTEAvgTimePerFrame;
    BOOL                    m_fPreprocessing;
    BOOL                    m_fArbitrary;
    BOOL                    m_fPreserveProfile;
    QWORD                   m_qwMaxDuration;

    //
    //  list of inputs
    //
    INPUTLIST               m_Inputs;
};

///////////////////////////////////////////////////////////////
// This is a helper class used only be UpdateProfile function.
// It compares stream and input using their types.
///////////////////////////////////////////////////////////////
BOOL CompareMediaTypes( const WM_MEDIA_TYPE * pMedia1, const WM_MEDIA_TYPE * pMedia2);

class CProfileStreams
{
public :
    CProfileStreams() : m_wStreamNum( 0 ), 
                        m_pMt( NULL ), 
                        m_pwszConnectionName( NULL )
    {
        ZeroMemory( &m_Type, sizeof( GUID ) );
    }

    CProfileStreams( GUID Type, WORD dwStreamNum, WM_MEDIA_TYPE * pMt, __in LPWSTR wszConnectionName ) :
                m_Type( Type ), m_wStreamNum( dwStreamNum ), m_pwszConnectionName( wszConnectionName ), m_pMt( pMt )
    {
    }

    ~CProfileStreams()
    {
    }

    void Cleanup()
    {
        SAFE_ARRAYDELETE( m_pMt );
        SAFE_ARRAYDELETE( m_pwszConnectionName );
    }

    //
    // Used by TSimpleList Find() to compare streams by stream type
    //
    BOOL operator == ( const CWMInput & right )
    {
       return( m_Type == right.m_Type && CompareMediaTypes( m_pMt, &right.m_Mt ) ); 
    }

    GUID                m_Type;
    WORD                m_wStreamNum;
    WM_MEDIA_TYPE       * m_pMt;
    WCHAR               * m_pwszConnectionName;
};

#endif // !defined(AFX_UncompAVIToWMV_H__21A57EE3_8C94_4E10_8092_4C171BCBA68E__INCLUDED_)
