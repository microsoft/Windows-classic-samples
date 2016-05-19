//*****************************************************************************
//
// Microsoft Windows Media
// Copyright ( C) Microsoft Corporation. All rights reserved.
//
// FileName:            UncompAVIToWMV.cpp
//
// Abstract:            Implementation of CUncompAVIToWMV class.
//
//*****************************************************************************

#include <tchar.h>
#include <stdio.h>
#include <windows.h>
#include <assert.h>
#include <mmsystem.h>
#include <strsafe.h>

#include "UncompAVIToWMV.h"

//
// Maximum number of files specified in file
//
#define MAX_INPUT_FILE      63

//
// {AA1A7D50-0690-4c22-9578-4A129E5ECD63}
//
static const GUID WMMEDIATYPE_MyArbitrary =
{ 0xaa1a7d50, 0x690, 0x4c22, { 0x95, 0x78, 0x4a, 0x12, 0x9e, 0x5e, 0xcd, 0x63 } };

const LPWSTR wszDefaultConnectionName = L"UncompAVIToWMV";

//------------------------------------------------------------------------------
// Name: ConvertTCharToWChar()
// Desc: Converts TCHAR string to wide characters.
//------------------------------------------------------------------------------
HRESULT ConvertTCharToWChar( LPCTSTR ptszInput, __out LPWSTR * pwszOutput )
{
    int cchOutput = 0;
    
    if( NULL == ptszInput || NULL == pwszOutput )
    {
        return( E_INVALIDARG );
    }

    //
    // Get output buffer size
    //
#ifdef UNICODE
    cchOutput = wcslen( ptszInput ) + 1;
#else //UNICODE
    cchOutput = MultiByteToWideChar( CP_ACP, 0, ptszInput, -1, NULL, 0 );
    if( 0 == cchOutput )
    {
        return( HRESULT_FROM_WIN32( GetLastError() ) );
    }
#endif // UNICODE

    *pwszOutput = new WCHAR[ cchOutput ];
    if( NULL == *pwszOutput)
    {
        return( E_OUTOFMEMORY );
    }

#ifdef UNICODE
    wcsncpy_s( *pwszOutput, cchOutput, ptszInput, cchOutput - 1);
#else //UNICODE
    if( 0 == MultiByteToWideChar( CP_ACP, 0, ptszInput, -1, *pwszOutput, cchOutput ) )
    {
        SAFE_ARRAYDELETE( *pwszOutput );
        return( HRESULT_FROM_WIN32( GetLastError() ) );
    }        
#endif // UNICODE

    return( S_OK );
}

//------------------------------------------------------------------------------
// Name: ConvertCharToTChar()
// Desc: Converts CHAR string to TCHAR.
//------------------------------------------------------------------------------
HRESULT ConvertCharToTChar( LPCSTR pszInput, __out LPTSTR * ptszOutput )
{
    int cchOutput = 0;
    
    if( NULL == pszInput || NULL == ptszOutput )
    {
        return( E_INVALIDARG );
    }

    //
    // Get output buffer size
    //
#ifdef UNICODE
    cchOutput = MultiByteToWideChar( CP_ACP, 0, pszInput, -1, NULL, 0 );
    if( 0 == cchOutput )
    {
        return( HRESULT_FROM_WIN32( GetLastError() ) );
    }
#else //UNICODE
    cchOutput = strlen( pszInput ) + 1;
#endif // UNICODE

    *ptszOutput = new TCHAR[ cchOutput ];
    if( NULL == *ptszOutput)
    {
        return( E_OUTOFMEMORY );
    }

#ifdef UNICODE
    if( 0 == MultiByteToWideChar( CP_ACP, 0, pszInput, -1, *ptszOutput, cchOutput ) )
    {
        SAFE_ARRAYDELETE( *ptszOutput );
        return( HRESULT_FROM_WIN32( GetLastError() ) );
    }        
#else //UNICODE
    (void)StringCchCopy( *ptszOutput, cchOutput, pszInput );
#endif // UNICODE

    return( S_OK );
}

//------------------------------------------------------------------------------
// Name: ConvertTCharToChar()
// Desc: Converts TCHAR string to CHAR.
//------------------------------------------------------------------------------
HRESULT ConvertTCharToChar( LPCTSTR ptszInput, __out LPSTR * pszOutput )
{
    int cchOutput = 0;
    
    if( NULL == ptszInput || NULL == pszOutput )
    {
        return( E_INVALIDARG );
    }

    //
    // Get output buffer size
    //
#ifdef UNICODE
    cchOutput = WideCharToMultiByte ( CP_ACP, 0, ptszInput, -1, NULL, 0, NULL, NULL );
    if( 0 == cchOutput )
    {
        return( HRESULT_FROM_WIN32( GetLastError() ) );
    }
#else //UNICODE
    cchOutput = strlen( ptszInput ) + 1;
#endif // UNICODE

    *pszOutput = new CHAR[ cchOutput ];
    if( NULL == *pszOutput)
    {
        return( E_OUTOFMEMORY );
    }

#ifdef UNICODE
    if( 0 == WideCharToMultiByte( CP_ACP, 0, ptszInput, -1, *pszOutput, cchOutput, NULL, NULL ) )
    {
        SAFE_ARRAYDELETE( *pszOutput );
        return( HRESULT_FROM_WIN32( GetLastError() ) );
    }        
#else //UNICODE
    (void)StringCchCopy( *pszOutput, cchOutput, ptszInput );
#endif // UNICODE

    return( S_OK );
}

//------------------------------------------------------------------------------
// Name: CWMInput::CWMInput()
// Desc: Constructor.
//------------------------------------------------------------------------------
CWMInput::CWMInput()  : m_qwPresentTime( 0 ),
                        m_dwInput( 0 ),
                        m_dwSamples( 0 ),
                        m_dwCurrentSample( 0 ),
                        m_pAVIFile( NULL ),
                        m_pwszConnectionName( NULL ),
                        m_pAVIStream( NULL ),
                        m_fAddSMPTE( FALSE )
{
    ZeroMemory( &m_Mt, sizeof( m_Mt ) );
    ZeroMemory( &m_StreamInfo, sizeof( m_StreamInfo ) );
}

//------------------------------------------------------------------------------
// Name: CWMInput::Cleanup()
// Desc: Releases global allocations.
//------------------------------------------------------------------------------
void CWMInput::Cleanup()
{
    if( NULL != m_pAVIStream )
    {
        AVIStreamRelease( m_pAVIStream );
        m_pAVIStream = NULL;
    }

    SAFE_ARRAYDELETE( m_pwszConnectionName );
    SAFE_ARRAYDELETE( m_Mt.pbFormat );
}

//------------------------------------------------------------------------------
// Name: CUncompAVIToWMV::CUncompAVIToWMV()
// Desc: Constructor.
//------------------------------------------------------------------------------
CUncompAVIToWMV::CUncompAVIToWMV()
{
    m_pWMWriter         = NULL;
    m_pIWMWriterPreprocess = NULL;
    m_dwArbitraryInput = 0;
    m_fPreprocessing = FALSE;
    m_fArbitrary = FALSE;
    m_fPreserveProfile = TRUE;

    AVIFileInit();
}

//------------------------------------------------------------------------------
// Name: CUncompAVIToWMV::~CUncompAVIToWMV()
// Desc: Constructor.
//------------------------------------------------------------------------------
CUncompAVIToWMV::~CUncompAVIToWMV()
{
    SAFE_RELEASE( m_pIWMWriterPreprocess );
    SAFE_RELEASE( m_pWMWriter );
    AVIFileExit();
}


//------------------------------------------------------------------------------
// Name: CUncompAVIToWMV::Initial()
// Desc: Initializes the writer according to WAV/AVI input files and
// parameter settings.
//------------------------------------------------------------------------------
HRESULT CUncompAVIToWMV::Initial( __in LPTSTR    ptszInFile,
                                  BOOL           fInFileListFile,
                                  __in LPTSTR    ptszOutFile,
                                  IWMProfile*    pProfile,
                                  BOOL           fArbitrary,
                                  BOOL           fPreserveProfile,
                                  BOOL           fAddSMPTE,
                                  int            nMaxDuration )
{
    HRESULT     hr = S_OK;
    LPTSTR      tszFileList[ MAX_INPUT_FILE ];
    int         nFileListSize = 0;
    
    if( NULL == ptszInFile || NULL == ptszOutFile || NULL == pProfile )
    {
        return( E_INVALIDARG );
    }

    m_fArbitrary = fArbitrary;
    m_fPreserveProfile = fPreserveProfile;

    if( nMaxDuration > 0 )
    {
        m_qwMaxDuration = 10000000 * (QWORD)nMaxDuration;
    }
    else
    {
        m_qwMaxDuration = 0x7FFFFFFFFFFFFFFF;
    }

    if( fInFileListFile )
    {
        ZeroMemory( tszFileList, sizeof( tszFileList) );
        nFileListSize = MAX_INPUT_FILE;

        //
        // Create list of input files and initialize AVI input list
        //
        hr = GetTokensFromFile( ptszInFile, tszFileList, &nFileListSize );
        if( FAILED( hr ) || 0 == nFileListSize )
        {
            _tprintf( _T( "Failed to read AVI file list from %s (hr=0x%08x)\n" ), ptszInFile, hr );
            return( E_INVALIDARG );
        }

        int i;
        for( i = 0; i < nFileListSize; i++ )
        {
            hr = InitAVISource( tszFileList[ i ] );
            if( FAILED( hr ) )
            {
                _tprintf( _T( "Failed to initial AVI source %s (hr=0x%08x)\n" ), tszFileList[ i ], hr );
                break;
            }
        }

        //
        // Deallocate input file list
        //
        for( i = 0; i < nFileListSize; i++ )
        {
            SAFE_ARRAYDELETE( tszFileList[ i ] );
        }

        if( FAILED( hr ) )
        {
            return( hr );
        }
    }
    else
    {
        //
        // There is only one WAV/AVI file
        //
        hr = InitAVISource( ptszInFile );
        if( FAILED( hr ) )
        {
            _tprintf( _T( "Failed to initial AVI source %s (hr=0x%08x)\n" ), ptszInFile, hr );
            return( hr );
        }
    }

    //
    // Create a WMWriter for the output file and set the profile
    //
    hr = WMCreateWriter( NULL, &m_pWMWriter );
    if( FAILED( hr ) )
    {
        _tprintf( _T( "Failed to Create WMWriter (hr=0x%08x)\n" ), hr );
        return( hr );
    }

    if( m_fPreprocessing )
    {
        //
        // Get a pointer to the preprocessing interface for multipass encoding
        //
        hr = m_pWMWriter->QueryInterface( IID_IWMWriterPreprocess, 
                                          (void **)&m_pIWMWriterPreprocess );
        if( FAILED( hr ) )
        {
            _tprintf( _T( "Failed to query IWMWriterPreprocess interface %x\n" ), hr );
            return( hr );
        }
    }

    //
    // Synchronize information in a current profile with AVI inputs.
    // As a result, all inputs in the profile should match AVI inputs.
    //
    hr = UpdateProfile( pProfile );
    if( FAILED( hr ) )
    {
        _tprintf( _T( "Failed to update profile : (hr=0x%08x)\n" ), hr );
        return( hr );
    }

    if( m_fArbitrary )
    {
        //
        // Add arbitrary stream to current profile
        //
        hr = AddArbitraryStream( pProfile );
        if( FAILED( hr ) )
        {
            _tprintf( _T( "Failed to add arbitrary stream (hr=0x%08x)\n" ), hr );
            return( hr );
        }
    }

    if( fAddSMPTE )
    {
        //
        // If fAddSMPTE is TRUE, we need to set up SMPTE.
        //
        hr = SetupSMPTE( pProfile );
        if( FAILED( hr ) )
        {
            _tprintf( _T( "Failed to get the video frame rate for SMPTE (hr=0x%08x)\n" ), hr );
            return( hr );
        }
    }

    //
    //  Save profile to writer
    //
    hr = m_pWMWriter->SetProfile( pProfile );
    if( FAILED( hr ) )
    {
        _tprintf( _T( "Failed to set profile (hr=0x%08x)\n" ), hr );
        return( hr );
    }

    //
    //  Update writer's inputs according to AVI inputs
    //
    hr = UpdateWriterInputs();
    if( FAILED( hr ) )
    {
        _tprintf( _T( "Failed to update writer inputs (hr=0x%08x)\n" ), hr );
        return( hr );
    }

    WCHAR * pwszOutFile = NULL;

    //
    // Convert the output filename to a wide character string and give it 
    // to the writer. 
    //
    hr = ConvertTCharToWChar( ptszOutFile, &pwszOutFile );
    if( FAILED( hr ) )
    {
        _tprintf( _T( "Failed to convert output file name to wchar string (hr=0x%08x)\n" ), hr );
        return( hr );
    }

    //
    // Note: Indexing is automatically enabled when writing the uncompressed 
    // samples. We don't need to set up indexing manually. You can call 
    // IWMWriterFileSink3::SetAutoIndexing( FALSE ) to disable indexing. 
    //
    hr = m_pWMWriter->SetOutputFilename( pwszOutFile );
    SAFE_ARRAYDELETE( pwszOutFile );

    if( FAILED( hr ) )
    {
        _tprintf( _T( "Failed to set output filename (hr=0x%08x)\n" ), hr );
    }

    return( hr );
}

//------------------------------------------------------------------------------
// Name: CUncompAVIToWMV::InitAVISource()
// Desc: Creates a list of all AVI inputs from AVI/WAV files.
//------------------------------------------------------------------------------
HRESULT CUncompAVIToWMV::InitAVISource( __in LPTSTR ptszInputFile )
{
    HRESULT         hr = S_OK;
    BITMAPINFO*     pBMI = NULL;
    CWMInput        input;

    if( NULL == ptszInputFile )
    {
        return( E_INVALIDARG );
    }

    //
    // Open the AVI file
    //
    hr = AVIFileOpen( &input.m_pAVIFile, 
                      ptszInputFile, 
                      OF_SHARE_DENY_NONE, 
                      NULL );
    if( FAILED( hr ) )
    {
        return( hr );
    }

    //
    // First, get the audio stream information from the AVI file
    //
    hr = AVIFileGetStream( input.m_pAVIFile, 
                           &input.m_pAVIStream, 
                           streamtypeAUDIO, 
                           0 );
    if( SUCCEEDED( hr ) )
    {
        LONG    cbWFX;

        input.m_dwSamples = AVIStreamEnd( input.m_pAVIStream );
        input.m_dwCurrentSample = AVIStreamStart( input.m_pAVIStream );

        //
        // If the stream contains any samples to play, put its parameters into the list
        //
        if( input.m_dwSamples > input.m_dwCurrentSample )
        {
            hr = AVIStreamFormatSize( input.m_pAVIStream, 0, &cbWFX );
            if( FAILED( hr ) )
            {
                return( hr );
            }

            if( cbWFX < sizeof( WAVEFORMATEX ) )
            {
                cbWFX = sizeof( WAVEFORMATEX );
            }

            input.m_pWFX = (WAVEFORMATEX*)new BYTE[ cbWFX ];
            if( NULL == input.m_pWFX )
            {
                return( E_OUTOFMEMORY );
            }

            hr = AVIStreamReadFormat( input.m_pAVIStream, 
                                      0, 
                                      input.m_pWFX, 
                                      &cbWFX );
            if( FAILED( hr ) )
            {
                SAFE_ARRAYDELETE( input.m_pWFX );
                return( hr );
            }

            if( WAVE_FORMAT_PCM == ( input.m_pWFX )->wFormatTag )
            {
                ( input.m_pWFX )->cbSize = 0;
            }

            hr = AVIStreamInfo( input.m_pAVIStream, 
                                &input.m_StreamInfo, 
                                sizeof( AVISTREAMINFO ) );
            if( FAILED( hr ) )
            {
                SAFE_ARRAYDELETE( input.m_pWFX );
                return( hr );
            }

            //
            // Set up the WM_MEDIA_TYPE structure and give it to the IWMInputMediaProps interface
            //
            input.m_Mt.majortype            = WMMEDIATYPE_Audio;
            input.m_Mt.subtype              = WMMEDIASUBTYPE_PCM;
            input.m_Mt.bFixedSizeSamples    = TRUE;
            input.m_Mt.bTemporalCompression = FALSE;
            input.m_Mt.lSampleSize          = input.m_StreamInfo.dwSampleSize;
            input.m_Mt.formattype           = WMFORMAT_WaveFormatEx;
            input.m_Mt.pUnk                 = NULL;
            input.m_Mt.cbFormat             = sizeof( WAVEFORMATEX ) + ( input.m_pWFX )->cbSize;
            input.m_Mt.pbFormat             = (BYTE *) input.m_pWFX;
            input.m_Type                    = WMMEDIATYPE_Audio;

            //
            // add to the list of AVI inputs
            //
            m_Inputs.Append( &input );
        }
    }

    //
    // Configure the video input format =========================================================
    //

    //
    // First, get the video stream information from the AVI file
    //
    hr = AVIFileGetStream( input.m_pAVIFile, &input.m_pAVIStream, streamtypeVIDEO, 0 );
    if( SUCCEEDED( hr ) )
    {
        LONG        cbBMI;

        input.m_pWFX = NULL;
        input.m_dwSamples = AVIStreamEnd( input.m_pAVIStream );
        input.m_dwCurrentSample = AVIStreamStart( input.m_pAVIStream );

        //
        // if input contains any samples to play, put its parameters into the list
        //
        if( input.m_dwSamples > input.m_dwCurrentSample )
        {
            hr = AVIStreamFormatSize( input.m_pAVIStream, 0, &cbBMI );
            if( FAILED( hr ) )
            {
                return( hr );
            }

            pBMI = (BITMAPINFO *)new BYTE[ cbBMI ];
            if( NULL == pBMI )
            {
                return( E_OUTOFMEMORY );
            }

            hr = AVIStreamReadFormat( input.m_pAVIStream, 0, pBMI, &cbBMI );
            if( FAILED( hr ) )
            {
                SAFE_ARRAYDELETE( pBMI );
                return( hr );
            }

            hr = AVIStreamInfo( input.m_pAVIStream, &input.m_StreamInfo, sizeof( AVISTREAMINFO ) );
            if( FAILED( hr ) )
            {
                SAFE_ARRAYDELETE( pBMI );
                return( hr );
            }

            //
            // Set up the WM_MEDIA_TYPE structure and give it to the IWMInputMediaProps interface
            //
            DWORD cbVideoInfo = sizeof( WMVIDEOINFOHEADER ) - sizeof( BITMAPINFOHEADER ) + cbBMI;
            
            WMVIDEOINFOHEADER*  pVideoInfo = (WMVIDEOINFOHEADER *)new BYTE[ cbVideoInfo ];
            if( NULL == pVideoInfo )
            {
                SAFE_ARRAYDELETE( pBMI );
                return( E_OUTOFMEMORY );
            }

            pVideoInfo->rcSource.left   = 0;
            pVideoInfo->rcSource.top    = 0;
            pVideoInfo->rcSource.bottom = pBMI->bmiHeader.biHeight;
            pVideoInfo->rcSource.right  = pBMI->bmiHeader.biWidth;
            pVideoInfo->rcTarget        = pVideoInfo->rcSource;
            pVideoInfo->dwBitRate       = MulDiv( input.m_StreamInfo.dwSuggestedBufferSize * 8, 
                                                  input.m_StreamInfo.dwRate, 
                                                  input.m_StreamInfo.dwScale );
            pVideoInfo->dwBitErrorRate  = 0;
            pVideoInfo->AvgTimePerFrame = 10000000 * (QWORD)input.m_StreamInfo.dwScale
                                          / input.m_StreamInfo.dwRate;

            CopyMemory( &(pVideoInfo->bmiHeader), pBMI, cbBMI );

            input.m_Mt.majortype = WMMEDIATYPE_Video;

            //
            // Map to the correct subtype GUID. If we don't map, just set it
            // to GUID_NULL; this probably means the SDK doesn't support the
            // input, but we'll still try to set the properties, just in case.
            //
            if( pBMI->bmiHeader.biCompression == BI_RGB )
            {
                if( pBMI->bmiHeader.biBitCount == 32 )
                {
                    input.m_Mt.subtype = WMMEDIASUBTYPE_RGB32;
                }
                else if( pBMI->bmiHeader.biBitCount == 24 )
                {
                    input.m_Mt.subtype = WMMEDIASUBTYPE_RGB24;
                }
                else if( pBMI->bmiHeader.biBitCount == 16 )
                {
                    input.m_Mt.subtype = WMMEDIASUBTYPE_RGB555;
                }
                else if( pBMI->bmiHeader.biBitCount == 8 )
                {
                    input.m_Mt.subtype = WMMEDIASUBTYPE_RGB8;
                }
                else if( pBMI->bmiHeader.biBitCount == 4 )
                {
                    input.m_Mt.subtype = WMMEDIASUBTYPE_RGB4;
                }
                else if( pBMI->bmiHeader.biBitCount == 1 )
                {
                    input.m_Mt.subtype = WMMEDIASUBTYPE_RGB1;
                }
                else
                {
                    input.m_Mt.subtype = GUID_NULL;
                }
            }
            else if( pBMI->bmiHeader.biCompression == 
                        MAKEFOURCC( _T('I'), _T('4'), _T('2'), _T('0') ) )
            {
                input.m_Mt.subtype = WMMEDIASUBTYPE_I420;
            }
            else if( pBMI->bmiHeader.biCompression == 
                        MAKEFOURCC( _T('I'), _T('Y'), _T('U'), _T('V') ) )
            {
                input.m_Mt.subtype = WMMEDIASUBTYPE_IYUV;
            }
            else if( pBMI->bmiHeader.biCompression == 
                        MAKEFOURCC( _T('Y'), _T('V'), _T('1'), _T('2') ) )
            {
                input.m_Mt.subtype = WMMEDIASUBTYPE_YV12;
            }
            else if( pBMI->bmiHeader.biCompression == 
                        MAKEFOURCC( _T('Y'), _T('U'), _T('Y'), _T('2') ) )
            {
                input.m_Mt.subtype = WMMEDIASUBTYPE_YUY2;
            }
            else if( pBMI->bmiHeader.biCompression == 
                        MAKEFOURCC( _T('U'), _T('Y'), _T('V'), _T('Y') ) )
            {
                input.m_Mt.subtype = WMMEDIASUBTYPE_UYVY;
            }
            else if( pBMI->bmiHeader.biCompression == 
                        MAKEFOURCC( _T('Y'), _T('V'), _T('Y'), _T('U') ) )
            {
                input.m_Mt.subtype = WMMEDIASUBTYPE_YVYU;
            }
            else if( pBMI->bmiHeader.biCompression == 
                        MAKEFOURCC( _T('Y'), _T('V'), _T('U'), _T('9') ) )
            {
                input.m_Mt.subtype = WMMEDIASUBTYPE_YVU9;
            }
            else
            {
                input.m_Mt.subtype = GUID_NULL;
            }

            input.m_Mt.bFixedSizeSamples = TRUE;
            input.m_Mt.bTemporalCompression = FALSE;
            input.m_Mt.lSampleSize = input.m_StreamInfo.dwSampleSize;
            input.m_Mt.formattype = WMFORMAT_VideoInfo;
            input.m_Mt.pUnk = NULL;
            input.m_Mt.cbFormat = cbVideoInfo;
            input.m_Mt.pbFormat = (BYTE *)pVideoInfo;

            SAFE_ARRAYDELETE( pBMI );

            input.m_Type = WMMEDIATYPE_Video;

            //
            // add to the list of AVI inputs
            //
            m_Inputs.Append( &input );
        }
    }
    else
    {
        hr = S_OK;
    }

    return( hr );
}

//------------------------------------------------------------------------------
// Name: CUncompAVIToWMV::Start()
// Desc: Creates output ASF file.
//------------------------------------------------------------------------------
HRESULT CUncompAVIToWMV::Start()
{
    HRESULT hr = S_OK;

    if( NULL == m_pWMWriter )
    {
        return( E_INVALIDARG );
    }

    //
    // Start writing ===========================================================
    //
    hr = m_pWMWriter->BeginWriting();
    if( FAILED( hr ) )
    {
        if( NS_E_VIDEO_CODEC_NOT_INSTALLED == hr )
        {
            _tprintf( _T( "BeginWriting failed: Video Codec not installed\n" ) );
        }
        if( NS_E_AUDIO_CODEC_NOT_INSTALLED == hr )
        {
            _tprintf( _T( "BeginWriting failed: Audio Codec not installed\n" ) );
        }
        else if( NS_E_INVALID_OUTPUT_FORMAT == hr )
        {
            _tprintf( _T( "BeginWriting failed: Invalid Output Format \n" ) );
        }
        else if( NS_E_VIDEO_CODEC_ERROR == hr )
        {
            _tprintf( _T( "BeginWriting failed: An unexpected error occurred with the video codec \n" ) );
        }
        else if( NS_E_AUDIO_CODEC_ERROR == hr )
        {
            _tprintf( _T( "BeginWriting failed: An unexpected error occurred with the audio codec \n" ) );
        }
        else
        {
            _tprintf( _T( "BeginWriting failed: Error (hr=0x%08x)\n" ), hr );
        }

        return( hr );
    }

    if( m_fPreprocessing )
    {
        //
        //  PREPROCESSING : preprocess all video streams
        //
        hr = DoPreprocessing( );
        if( FAILED( hr ) )
        {
            if( NS_E_INVALID_NUM_PASSES == hr )
            {
                _tprintf( _T( "Preprocessing failed: Invalid preprocessing passes. Don't use -m option\n" ) );
            }
            else
            {
                _tprintf( _T( "Preprocessing failed: Error (hr=0x%08x)\n" ), hr );
            }

            m_pWMWriter->EndWriting();
            return( hr );
        }
    }

    DWORD dwCurrentProgress = 0;
    QWORD qwAllSamples = 0;
    DWORD dwCurrentSample = 0;

    //
    //  Calculate total number of samples - used for showing progress
    //
    CWMInput * pInput = m_Inputs.Iterate( ITER_FIRST );

    while( NULL != pInput )
    {
        qwAllSamples += pInput->m_dwSamples;
        pInput = m_Inputs.Iterate( ITER_NEXT );
    }

    _tprintf( _T( "            0%%--------20%%-------40%%-------60%%-------80%%-------100%%\n" ) );
    _tprintf( _T( "convert:    " ) );

    //
    // Get an AVI input with the lowest presentation time value and write it 
    // to the output
    //
    pInput = NULL;

    El< CWMInput > * pElem = NULL;

    pInput = m_Inputs.GetMinElement( &pElem );

    while( NULL != pInput )
    {
        //
        // Don't write sample again if the maximum duration time is reached.
        //
        if( pInput->m_qwPresentTime >= m_qwMaxDuration )
        {
            _tprintf( _T( "\nMax duration is reached" ) );
            break;
        }

        //
        // If current AVI input is finished, remove it from the list
        //
        if( pInput->m_dwCurrentSample >= pInput->m_dwSamples )
        {
            m_Inputs.Erase( pElem );
        }
        else
        {
            //
            // Write samples from the chosen input
            //
            DWORD dwSample = pInput->m_dwCurrentSample;

            hr = WriteSample( pInput );
            if( FAILED( hr ) )
            {
                if( NS_E_INVALID_NUM_PASSES == hr )
                {
                    _tprintf( _T( "WriteSample failed: Invalid preprocessing passes. Use -m option\n" ) );
                }
                else 
                {
                    _tprintf( _T( "WriteSample failed: Error (hr=0x%08x)\n" ), hr );
                }

                m_pWMWriter->EndWriting();
                return( hr );
            }

            //
            // Calculate current sample number to show the progress
            //
            dwCurrentSample += pInput->m_dwCurrentSample - dwSample;

            //
            // Show progress
            //
            while( dwCurrentProgress <= dwCurrentSample * 100 / qwAllSamples )
            {
                _tprintf( _T( "*" ) );
                dwCurrentProgress += 2;
            }
        }

        //
        //  Get next AVI input to be played
        //
        pInput = m_Inputs.GetMinElement( &pElem );
    }

    _tprintf( _T( "\n" ) );

    //
    // Tell the writer we're done. 
    //
    hr = m_pWMWriter->EndWriting();
    if( FAILED( hr ) )
    {
        _tprintf( _T( "EndWriting failed: Error (hr=0x%08x)\n" ), hr );
        return( hr );
    }

    return( hr );
}

//------------------------------------------------------------------------------
// Name: CUncompAVIToWMV::UpdateProfile()
// Desc: Synchronizes information in a current profile with AVI inputs.
//       As a result, all inputs in the profile should match AVI inputs.
//
//       If there are more AVI inputs than profile inputs, additional
//       streams are created and stored in the profile.
//
//       If there are more profile inputs than AVI inputs, outstanding
//       streams are removed from the profile.
//------------------------------------------------------------------------------
HRESULT CUncompAVIToWMV::UpdateProfile( IWMProfile * pProfile )
{
    HRESULT hr = S_OK;

    //
    // List of streams defined in the current profile
    //
    CTSimpleList< CProfileStreams > ProfStreamList;

    if( NULL == pProfile )
    {
        return( E_INVALIDARG );
    }

    //
    // Create stream list from the current profile
    //
    hr = CreateProfileStreamList( pProfile, &ProfStreamList );

    CProfileStreams         * pProfStream = NULL;
    CWMInput                * pInput = NULL;
    El< CProfileStreams >   * pProfStreamElem = NULL;
    El< CWMInput >          * pInputElem = NULL;
    El< CWMInput >          * pInputElemRemove = NULL;

    //
    // Iterate through the list of AVI inputs and compare their types with inputs 
    // already defined in the current profile. If there is a match, corresponding 
    // connection names are stored in the AVI input, creating a link which is then 
    // used to initialize writer inputs and deliver samples to the writer.
    //

    pInput = m_Inputs.Iterate( ITER_FIRST, &pInputElem );

    while( NULL != pInput )
    {
        //
        // For a given AVI input, find first input in the current profile with matching media type
        //
        pProfStream = Find( &ProfStreamList, pInput, &pProfStreamElem );
        if( NULL != pProfStream &&
            NULL != pProfStream->m_pwszConnectionName )
        {
            //
            // There is a matching input in the current profile. Save its number and connection
            // name and remove it from the list of outstanding profile inputs.
            //
            pInput->m_pwszConnectionName = new WCHAR[ ( wcslen( pProfStream->m_pwszConnectionName ) + 1 ) ];
            if( NULL == pInput->m_pwszConnectionName )
            {
                hr = E_OUTOFMEMORY;
                break;
            }

            (void)StringCchCopyW( pInput->m_pwszConnectionName,  wcslen( pProfStream->m_pwszConnectionName ) + 1, pProfStream->m_pwszConnectionName );

            ProfStreamList.Erase( pProfStreamElem );
        }
        else
        {
            if( !m_fPreserveProfile )
            {
                //
                // There is no matching input in the profile - create one
                //
                do
                {
                    WORD    wStreamNum = 0;
                    LPWSTR  pwszConnectionName = NULL;

                    if( WMMEDIATYPE_Video == pInput->m_Type )
                    {
                        hr = AddVideoStream( pProfile,
                                             (WMVIDEOINFOHEADER *)pInput->m_Mt.pbFormat,
                                             &wStreamNum,
                                             50,
                                             5,
                                             &pwszConnectionName );
                    }
                    else if( WMMEDIATYPE_Audio == pInput->m_Type )
                    {
                        hr = AddAudioStream( pProfile,
                                             pInput->m_pWFX->nSamplesPerSec,
                                             pInput->m_pWFX->nChannels,
                                             pInput->m_pWFX->wBitsPerSample,
                                             &wStreamNum,
                                             &pwszConnectionName );
                    }

                    if( SUCCEEDED( hr ) )
                    {
                        //
                        // A new input in the profile has been created.
                        // Store the connection name with a corresponding AVI input.
                        //
                        pInput->m_pwszConnectionName = pwszConnectionName;
                    }
                    else
                    {
                        //
                        // Input creation failed, so mark current AVI input
                        // to remove from the list
                        //
                        pInputElemRemove = pInputElem;
                    }

                } while( FALSE );
            }
            else
            {
                //
                //  There is no matching input in the profile. 
                //  Since the profile cannot be extended, mark current AVI input
                //  to remove from the list.
                //
                pInputElemRemove = pInputElem;
            }
        }

        pInput = m_Inputs.Iterate( ITER_NEXT, &pInputElem );

        //
        // Remove marked AVI input from the list
        //
        if( NULL != pInputElemRemove )
        {
            m_Inputs.Erase( pInputElemRemove );
            pInputElemRemove = NULL;
        }
    }

    //
    // Remove all unused inputs from the profile.
    //
    pProfStream = ProfStreamList.Iterate( ITER_FIRST );

    while( NULL != pProfStream )
    {
        hr = pProfile->RemoveStreamByNumber( pProfStream->m_wStreamNum );
        pProfStream = ProfStreamList.Iterate( ITER_NEXT );
    }

    return( hr );
}

//------------------------------------------------------------------------------
// Name: CUncompAVIToWMV::CreateProfileStreamList()
// Desc: Creates a list of inputs for a given profile.
//------------------------------------------------------------------------------
HRESULT CUncompAVIToWMV::CreateProfileStreamList( IWMProfile* pProfile, 
                                                  CTSimpleList< CProfileStreams >* pProfStreamList )
{
    HRESULT                 hr = S_OK;
    IWMStreamConfig         * pIWMStreamConfig = NULL;
    IWMMediaProps           * pMediaProps = NULL;
    WM_MEDIA_TYPE           * pMediaType = NULL;
    GUID                    guidInputType;
    WORD                    wStreamNum = 0;
    DWORD                   cbMediaType = 0;
    DWORD                   cStreams = 0;
    CProfileStreams         * pProfStream = NULL;
    WCHAR                   * pwszConnectionName = NULL;
    WORD                    cchConnectionName = 0;

    if( NULL == pProfile  || NULL == pProfStreamList )
    {
        return( E_INVALIDARG );
    }

    hr = pProfile->GetStreamCount( &cStreams );
    if( FAILED( hr ) )
    {
        return( hr );
    }

    //
    // Create a list of inputs defined in the given profile
    //

    for( DWORD i = 0; i < cStreams; i++ )
    {
        hr = pProfile->GetStream( i, &pIWMStreamConfig );
        if( FAILED( hr ) )
        {
            break;
        }

        hr = pIWMStreamConfig->GetStreamType( &guidInputType );
        if( FAILED( hr ) )
        {
            break;
        }

        hr = pIWMStreamConfig->QueryInterface( IID_IWMMediaProps, 
                                               (void **)&pMediaProps );
        if( FAILED( hr ) )
        {
            break;
        }

        hr = pMediaProps->GetMediaType( NULL, &cbMediaType );
        if( FAILED( hr ) )
        {
            break;
        }

        pMediaType = (WM_MEDIA_TYPE *)new BYTE[ cbMediaType ];
        if( NULL == pMediaType )
        {
            hr = E_OUTOFMEMORY;
            break;
        }

        hr = pMediaProps->GetMediaType( pMediaType, &cbMediaType );
        if( FAILED( hr ) )
        {
            break;
        }

        hr = pIWMStreamConfig->GetConnectionName( NULL, &cchConnectionName );
        if( FAILED( hr ) )
        {
            break;
        }

        pwszConnectionName = new WCHAR[ cchConnectionName ];
        if( NULL == pwszConnectionName )
        {
            hr = E_OUTOFMEMORY;
            break;
        }

        hr = pIWMStreamConfig->GetConnectionName( pwszConnectionName, 
                                                  &cchConnectionName );
        if( FAILED( hr ) )
        {
            break;
        }

        hr = pIWMStreamConfig->GetStreamNumber( &wStreamNum );
        if( FAILED( hr ) )
        {
            break;
        }

        //
        // Only one stream for each connection is needed on this list
        //
        pProfStream = pProfStreamList->Iterate( ITER_FIRST );

        while( NULL != pProfStream )
        {
            if( 0 == wcscmp( pwszConnectionName, pProfStream->m_pwszConnectionName ) )
            {
                //
                // There is already a stream for this connection on the list; do not append
                // this one. This could happen if the profile is MBR. 
                //
                break;
            }

            pProfStream = pProfStreamList->Iterate( ITER_NEXT );
        }

        if( NULL == pProfStream )
        {
            //
            // There is no stream for this connection on the list; append this one.
            // pProfStreamList will not allocate memory to save pwszConnectionName 
            // and pMediaType, so the memory of pwszConnectionName and pMediaType
            // should not be released now.
            //
            CProfileStreams ProfileStream( guidInputType, 
                                           wStreamNum, 
                                           pMediaType, 
                                           pwszConnectionName );

            if ( pProfStreamList->Append( &ProfileStream ) )
            {
                //
                // Set the pointers to NULL, so their memory will not be released 
                // by this function.
                //
                pMediaType = NULL;
                pwszConnectionName = NULL;
            }
        }

        SAFE_ARRAYDELETE( pwszConnectionName );
        SAFE_ARRAYDELETE( pMediaType );
        SAFE_RELEASE( pMediaProps );
        SAFE_RELEASE( pIWMStreamConfig );
    }

    SAFE_ARRAYDELETE( pwszConnectionName );
    SAFE_ARRAYDELETE( pMediaType );
    SAFE_RELEASE( pMediaProps );
    SAFE_RELEASE( pIWMStreamConfig );

    return( hr );
}

//------------------------------------------------------------------------------
// Name: CUncompAVIToWMV::UpdateWriterInputs()
// Desc: Sets up properties of writer inputs using information 
//       stored in AVI inputs.
//------------------------------------------------------------------------------
HRESULT CUncompAVIToWMV::UpdateWriterInputs()
{
    HRESULT             hr = S_OK;
    DWORD               cInputs = 0;
    GUID                guidInputType;
    IWMInputMediaProps  * pInputProps = NULL;
    IWMStreamConfig     * pIWMStreamConfig = NULL;
    WCHAR               * pwszConnectionName = NULL;
    WORD                cchConnectionName = 0;

    if( NULL == m_pWMWriter )
    {
        return( E_INVALIDARG );
    }

    hr = m_pWMWriter->GetInputCount( &cInputs );
    if( FAILED( hr ) )
    {
        return( hr );
    }

    //
    // Browse through all writer inputs and set properties according to
    // corresponding AVI inputs
    //
    for( DWORD i = 0; i < cInputs; i++ )
    {
        hr = m_pWMWriter->GetInputProps( i, &pInputProps );
        if( FAILED( hr ) )
        {
            break;
        }

        hr = pInputProps->QueryInterface( IID_IWMStreamConfig, 
                                          (void **) &pIWMStreamConfig );
        if( FAILED( hr ) )
        {
            break;
        }

        hr = pIWMStreamConfig->GetStreamType( &guidInputType );
        if( FAILED( hr ) )
        {
            return( hr );
        }

        //
        // If this is an arbitrary stream, save its number
        //
        if( WMMEDIATYPE_MyArbitrary == guidInputType )
        {
            m_dwArbitraryInput = i;
        }
        else
        {
            hr = pIWMStreamConfig->GetConnectionName( NULL, &cchConnectionName );
            if( FAILED( hr ) )
            {
                break;
            }

            pwszConnectionName = new WCHAR[ cchConnectionName ];
            if( NULL == pwszConnectionName )
            {
                hr = E_OUTOFMEMORY;
                break;
            }

            hr = pIWMStreamConfig->GetConnectionName( pwszConnectionName, 
                                                      &cchConnectionName );
            if( FAILED( hr ) )
            {
                break;
            }

            //
            // Look for AVI input with the matching connection name and 
            // set writer input properties according to it
            //
            CWMInput * pInput = m_Inputs.Iterate( ITER_FIRST );

            while( NULL != pInput )
            {
                if( 0 == wcscmp( pInput->m_pwszConnectionName, pwszConnectionName ) )
                {
                    pInput->m_dwInput = i;

                    hr = pInputProps->SetMediaType( &pInput->m_Mt );
                    if( FAILED( hr ) )
                    {
                        break;
                    }

                    hr = m_pWMWriter->SetInputProps( i, pInputProps );
                    break;
                }

                pInput = m_Inputs.Iterate( ITER_NEXT );
            }

            if( NULL == pInput )
            {
                hr = E_INVALIDARG;
            }

            if ( FAILED( hr ) )
            {
                break;
            }
        }

        SAFE_ARRAYDELETE( pwszConnectionName );
        SAFE_RELEASE( pInputProps );
        SAFE_RELEASE( pIWMStreamConfig );
    }

    SAFE_ARRAYDELETE( pwszConnectionName );
    SAFE_RELEASE( pInputProps );
    SAFE_RELEASE( pIWMStreamConfig );

    return( hr );
}

//------------------------------------------------------------------------------
// Name: CUncompAVIToWMV::AddArbitraryStream()
// Desc: Adds an arbitrary stream of DWORDs to the current profile.
//------------------------------------------------------------------------------
HRESULT CUncompAVIToWMV::AddArbitraryStream( IWMProfile * pProfile )
{
    HRESULT             hr = S_OK;
    IWMStreamConfig     * pIWMStreamConfig = NULL;
    IWMMediaProps       * pIWMMediaProps = NULL;
    WM_MEDIA_TYPE       mt;

    if( NULL == pProfile )
    {
        return( E_INVALIDARG );
    }

    hr = pProfile->CreateNewStream( WMMEDIATYPE_MyArbitrary, &pIWMStreamConfig );
    if( FAILED( hr ) || NULL == pIWMStreamConfig )
    {
        return( hr );
    }

    ZeroMemory( &mt, sizeof( mt ) );

    mt.majortype = WMMEDIATYPE_MyArbitrary;
    mt.subtype = GUID_NULL;
    mt.bFixedSizeSamples = FALSE;
    mt.bTemporalCompression = FALSE;
    mt.lSampleSize = 0;
    mt.formattype = GUID_NULL;
    mt.pUnk = NULL;
    mt.cbFormat = 0;
    mt.pbFormat = 0;

    do
    {
        hr = pIWMStreamConfig->SetStreamName( L"Arbitrary Stream" );
        if( FAILED( hr ) )
        {
            break;
        }

        hr = pIWMStreamConfig->SetConnectionName( L"Arbitrary Stream Connection" );
        if( FAILED( hr ) )
        {
            break;
        }

        hr = pIWMStreamConfig->SetBitrate( 1024 );
        if( FAILED( hr ) )
        {
            break;
        }

        hr = pIWMStreamConfig->QueryInterface( IID_IWMMediaProps, 
                                               (void **)&pIWMMediaProps );
        if( FAILED( hr ) )
        {
            break;
        }

        hr = pIWMMediaProps->SetMediaType( &mt );
        if( FAILED( hr ) )
        {
            break;
        }

        hr = pProfile->AddStream( pIWMStreamConfig );
        if( FAILED( hr ) )
        {
            break;
        }

    } while( FALSE );

    SAFE_RELEASE( pIWMMediaProps );
    SAFE_RELEASE( pIWMStreamConfig );

    return( hr );
}

//------------------------------------------------------------------------------
// Name: CUncompAVIToWMV::ListSystemProfile()
// Desc: Lists all system profiles (version 8.0), and displays their indexes and names.
//------------------------------------------------------------------------------
HRESULT CUncompAVIToWMV::ListSystemProfile()
{
    HRESULT             hr = S_OK;
    DWORD               dwIndex = 0;
    DWORD               cProfiles = 0;
    IWMProfileManager   * pIWMProfileManager = NULL;
    IWMProfileManager2  * pIWMProfileManager2 = NULL;
    IWMProfile          * pIWMProfile = NULL;
    WCHAR               * pwszName = NULL;
    DWORD               cchName = 0;

    do
    {
        //
        // Create profile manager
        //
        hr = WMCreateProfileManager( &pIWMProfileManager );
        if( FAILED( hr ) )
        {
            break;
        }

        hr = pIWMProfileManager->QueryInterface( IID_IWMProfileManager2, 
                                                 ( void ** )&pIWMProfileManager2 );
        if( FAILED( hr ) )
        {
            break;
        }

        //
        // Set system profile version to 8.0
        //
        hr = pIWMProfileManager2->SetSystemProfileVersion( WMT_VER_8_0 );
        if( FAILED( hr ) )
        {
            break;
        }

        hr = pIWMProfileManager->GetSystemProfileCount( &cProfiles );
        if( FAILED( hr ) )
        {
            break;
        }
    
        _tprintf( _T( "Profile Indexes are as follows:\n" ) );

        //
        // Iterate all system profiles
        //
        for( dwIndex = 0; dwIndex < cProfiles; dwIndex++ )
        {
            hr = pIWMProfileManager->LoadSystemProfile( dwIndex, &pIWMProfile );
            if ( FAILED( hr ) )
            {
                break;
            }

            hr = pIWMProfile->GetName( NULL, &cchName );
            if ( FAILED( hr ) )
            {
                break;
            }

            pwszName = new WCHAR[ cchName ];
            if( NULL == pwszName )
            {
                hr = E_OUTOFMEMORY;
                break;
            }

            hr = pIWMProfile->GetName( pwszName, &cchName );
            if ( FAILED( hr ) )
            {
                break;
            }
    
            //
            // Display the system profile index and name
            //
            _tprintf( _T( "   %d - %ws \n" ), dwIndex + 1, pwszName );

            SAFE_ARRAYDELETE( pwszName );
            SAFE_RELEASE( pIWMProfile );
        }

        if( FAILED( hr ) )
        {
            break;
        }
    }
    while( FALSE );

    //
    // Release all resources
    //
    SAFE_ARRAYDELETE( pwszName );
    SAFE_RELEASE( pIWMProfile );
    SAFE_RELEASE( pIWMProfileManager2 );
    SAFE_RELEASE( pIWMProfileManager );

    return( hr );
}

//------------------------------------------------------------------------------
// Name: CUncompAVIToWMV::LoadSystemProfile()
// Desc: Loads a system profile (version 8.0) by the index.
//------------------------------------------------------------------------------
HRESULT CUncompAVIToWMV::LoadSystemProfile( DWORD dwProfileIndex, 
                                            IWMProfile ** ppIWMProfile )
{
    HRESULT             hr = S_OK;
    IWMProfileManager   * pIWMProfileManager = NULL;
    IWMProfileManager2  * pIWMProfileManager2 = NULL;

    if( NULL == ppIWMProfile )
    {
        return( E_POINTER );
    }

    do
    {
        //
        // Index starts from 0 but the user sees it as starting from 1
        //
        dwProfileIndex--;

        //
        // Create profile manager
        //
        hr = WMCreateProfileManager( &pIWMProfileManager );
        if( FAILED( hr ) )
        {
            break;
        }

        hr = pIWMProfileManager->QueryInterface( IID_IWMProfileManager2, 
                                                 ( void ** )&pIWMProfileManager2 );
        if( FAILED( hr ) )
        {
            break;
        }

        //
        // Set system profile version to 8.0
        //
        hr = pIWMProfileManager2->SetSystemProfileVersion( WMT_VER_8_0 );
        if( FAILED( hr ) )
        {
            break;
        }

        //
        // Load the system profile by index
        //
        hr = pIWMProfileManager->LoadSystemProfile( dwProfileIndex, 
                                                    ppIWMProfile );
        if( FAILED( hr ) )
        {
            break;
        }
    }
    while( FALSE );

    //
    // Release all resources
    //
    SAFE_RELEASE( pIWMProfileManager2 );
    SAFE_RELEASE( pIWMProfileManager );

    return( hr );
}

//------------------------------------------------------------------------------
// Name: CUncompAVIToWMV::LoadCustomProfile()
// Desc: Loads a custom profile from file.
//------------------------------------------------------------------------------
HRESULT CUncompAVIToWMV::LoadCustomProfile( LPCTSTR ptszProfileFile, 
                                            IWMProfile ** ppIWMProfile )
{
    HRESULT             hr = S_OK;
    DWORD               dwLength = 0;
    DWORD               dwBytesRead = 0;
    IWMProfileManager   * pProfileManager = NULL;
    HANDLE              hFile = INVALID_HANDLE_VALUE;
    LPWSTR              pProfile = NULL;

    if( NULL == ptszProfileFile || NULL == ppIWMProfile )
    {
        return( E_POINTER );
    }

    do
    {
        //
        // Create profile manager
        //
        hr = WMCreateProfileManager( &pProfileManager );
        if( FAILED( hr ) )
        {
            break;
        }

        //
        // Open the profile file
        //
        hFile = CreateFile( ptszProfileFile, 
                            GENERIC_READ, 
                            FILE_SHARE_READ, 
                            NULL, 
                            OPEN_EXISTING, 
                            FILE_ATTRIBUTE_NORMAL, 
                            NULL );
        if( INVALID_HANDLE_VALUE == hFile )
        {
            hr = HRESULT_FROM_WIN32( GetLastError() );
            break;
        }

        if( FILE_TYPE_DISK != GetFileType( hFile ) )
        {
            hr = NS_E_INVALID_NAME;
            break;
        }

        dwLength = GetFileSize( hFile, NULL );
        if( -1 == dwLength )
        {
            hr = HRESULT_FROM_WIN32( GetLastError() );
            break;
        }

        //
        // Allocate memory for profile buffer
        //
        pProfile = (WCHAR *)new BYTE[ dwLength + sizeof(WCHAR) ];
        if( NULL == pProfile )
        {
            hr = E_OUTOFMEMORY;
            break;
        }

        // The buffer must be null-terminated.
        memset( pProfile, 0, dwLength + sizeof(WCHAR) );

        //
        // Read the profile to a buffer
        //
        if( !ReadFile( hFile, pProfile, dwLength, &dwBytesRead, NULL ) )
        {
            hr = HRESULT_FROM_WIN32( GetLastError() );
            break;
        }

        //
        // Load the profile from the buffer
        //
        hr = pProfileManager->LoadProfileByData( pProfile, 
                                                 ppIWMProfile );
        if( FAILED(hr) )
        {
            break;
        }
    }
    while( FALSE );

    //
    // Release all resources
    //
    SAFE_ARRAYDELETE( pProfile );
    SAFE_CLOSEFILEHANDLE( hFile );
    SAFE_RELEASE( pProfileManager );

    return( hr );
}

//------------------------------------------------------------------------------
// Name: CUncompAVIToWMV::CreateEmptyProfile()
// Desc: Creates an empty profile.
//------------------------------------------------------------------------------
HRESULT CUncompAVIToWMV::CreateEmptyProfile( IWMProfile ** ppIWMProfile )
{
    HRESULT             hr = S_OK;
    IWMProfileManager*  pIWMProfileManager = NULL;

    if( NULL == ppIWMProfile )
    {
        return( E_POINTER );
    }

    do
    {
        //
        // Create profile manager
        //
        hr = WMCreateProfileManager( &pIWMProfileManager );
        if( FAILED( hr ) )
        {
            break;
        }

        hr = pIWMProfileManager->CreateEmptyProfile( WMT_VER_8_0, ppIWMProfile );
        if( FAILED( hr ) )
        {
            break;
        }
    }
    while( FALSE );

    //
    // Release all resources
    //
    SAFE_RELEASE( pIWMProfileManager );

    return( hr );
}

//------------------------------------------------------------------------------
// Name: CUncompAVIToWMV::SaveProfile()
// Desc: Save the profile to a file.
//------------------------------------------------------------------------------
HRESULT CUncompAVIToWMV::SaveProfile( LPCTSTR ptszFileName, 
                                      IWMProfile * pIWMProfile )
{
    HRESULT             hr = S_OK;
    IWMProfileManager   * pIWMProfileManager = NULL;
    DWORD               dwLength = 0;
    LPWSTR              pBuffer = NULL;
    HANDLE              hFile = INVALID_HANDLE_VALUE;
    DWORD               dwBytesWritten = 0;


    if( ( NULL == ptszFileName ) || ( NULL == pIWMProfile ) )
    {
        return( E_INVALIDARG );
    }

    do
    {
        //
        // Create profile manager
        //
        hr = WMCreateProfileManager( &pIWMProfileManager );
        if( FAILED( hr ) )
        {
            break;
        }

        //
        // Save profile to a buffer
        //
        hr = pIWMProfileManager->SaveProfile( pIWMProfile, NULL, &dwLength );
        if( FAILED( hr ) )
        {
            break;
        }

        pBuffer = new WCHAR[ dwLength ];
        if( NULL == pBuffer )
        {
            hr = E_OUTOFMEMORY;
            break;
        }

        hr = pIWMProfileManager->SaveProfile( pIWMProfile, pBuffer, &dwLength );
        if( FAILED( hr ) )
        {
            break;
        }

        hFile = CreateFile( ptszFileName, 
                            GENERIC_WRITE, 
                            0, 
                            NULL, 
                            CREATE_ALWAYS, 
                            FILE_ATTRIBUTE_NORMAL, 
                            NULL );
        if( INVALID_HANDLE_VALUE == hFile )
        {
            hr = HRESULT_FROM_WIN32( GetLastError() );
            break;
        }

        if( FILE_TYPE_DISK != GetFileType( hFile ) )
        {
            hr = NS_E_INVALID_NAME;
            break;
        }

        //
        // Write profile buffer to file
        //
        if( !WriteFile( hFile, pBuffer, dwLength * sizeof(WCHAR), &dwBytesWritten, NULL) ||
             dwLength*sizeof(WCHAR) != dwBytesWritten )
        {
            hr = HRESULT_FROM_WIN32( GetLastError() );
            break;
        }

    } while( FALSE );

    SAFE_CLOSEFILEHANDLE( hFile );
    SAFE_ARRAYDELETE( pBuffer );
    SAFE_RELEASE( pIWMProfileManager );

    return( hr );
}

//------------------------------------------------------------------------------
// Name: CUncompAVIToWMV::DoPreprocessing()
// Desc: Perform preprocessing on all input video and audio streams.
//       This method does not update any stream info on the AVI input list.
//------------------------------------------------------------------------------
HRESULT CUncompAVIToWMV::DoPreprocessing()
{
    HRESULT         hr = S_OK;
    DWORD           cCurrentSamples = 0;
    DWORD           dwCurrentSample = 0;
    LONG            cSamplesToRead = 0;
    INSSBuffer      * pSample  = NULL;
    BYTE            * pbBuffer = NULL;
    DWORD           cbBuffer = 0;
    QWORD           qwPresentTime = 0;
    DWORD           dwNumPasses = 0;
    DWORD           dwCurrentProgress = 0;
    PAVISTREAM      pCurrentStream = NULL;
    DWORD           dwCurrentInput = 0;
    AVISTREAMINFO   CurrentStreamInfo;
    El< CWMInput >  * pStreamElem = NULL;
    CWMInput        * pStream = NULL;

    if( !m_fPreprocessing )
    {
        return( E_FAIL );
    }

    if( NULL == m_pWMWriter || NULL == m_pIWMWriterPreprocess )
    {
        return( E_INVALIDARG );
    }

    pStream = m_Inputs.Iterate( ITER_FIRST, &pStreamElem );

    while( SUCCEEDED( hr ) && NULL != pStream )
    {
        if( WMMEDIATYPE_Video == pStream->m_Type || 
            WMMEDIATYPE_Audio == pStream->m_Type )
        {
            dwCurrentInput = pStream->m_dwInput;
            pCurrentStream = pStream->m_pAVIStream;
            CurrentStreamInfo = pStream->m_StreamInfo;
            cCurrentSamples = pStream->m_dwSamples;

            hr = m_pIWMWriterPreprocess->GetMaxPreprocessingPasses( dwCurrentInput, 
                                                                    0, 
                                                                    &dwNumPasses );
            if( FAILED( hr ) )
            {
                return( hr );
            }

            if( 0 != dwNumPasses )
            {
                //
                // Use the recommended number of passes
                //
                hr = m_pIWMWriterPreprocess->SetNumPreprocessingPasses( dwCurrentInput, 
                                                                        0, 
                                                                        dwNumPasses );
                if( FAILED( hr ) )
                {
                    return( hr );
                }

                _tprintf( _T( "            0%%--------20%%-------40%%-------60%%-------80%%-------100%%\n" ) );
                _tprintf( _T( "preprocess: " ) );

                while( 0 != dwNumPasses )
                {
                    if( WMMEDIATYPE_Audio == pStream->m_Type )
                    {
                        //
                        // We want to read half second of audio at a time
                        //
                        cSamplesToRead = ( pStream->m_pWFX->nAvgBytesPerSec * 4 ) 
                            / ( pStream->m_pWFX->nChannels * pStream->m_pWFX->wBitsPerSample );
                    }
                    else if( WMMEDIATYPE_Video == pStream->m_Type )
                    {
                        cSamplesToRead = AVISTREAMREAD_CONVENIENT;
                    }

                    hr = m_pIWMWriterPreprocess->BeginPreprocessingPass( dwCurrentInput, 0 );
                    if( FAILED( hr ) )
                    {
                        return( hr );
                    }

                    dwCurrentSample = 0;
                    dwCurrentProgress = 0;

                    while( dwCurrentSample < cCurrentSamples )
                    {
                        LONG    cbSample = 0;
                        LONG    cSamples = 0;

                        hr = AVIStreamRead( pCurrentStream, 
                                            dwCurrentSample, 
                                            cSamplesToRead, 
                                            0, 
                                            0, 
                                            &cbSample, 
                                            &cSamples );
                        assert( SUCCEEDED( hr ) );
                        if( FAILED( hr ) )
                        {
                            return( hr );
                        }

                        if( 0 < cbSample )
                        {
                            hr = m_pWMWriter->AllocateSample( cbSample, &pSample );
                            assert( SUCCEEDED( hr ) );

                            if( SUCCEEDED( hr ) )
                            {
                                hr = pSample->GetBufferAndLength( &pbBuffer, &cbBuffer );
                                assert( SUCCEEDED( hr ) && (long)cbBuffer >= cbSample );
                            }

                            if( SUCCEEDED( hr ) )
                            {
                                hr = AVIStreamRead( pCurrentStream, 
                                                    dwCurrentSample, 
                                                    cSamples, 
                                                    pbBuffer, 
                                                    cbBuffer, 
                                                    &cbSample, 
                                                    &cSamples );
                                assert( SUCCEEDED( hr ) );
                            }

                            if( SUCCEEDED( hr ) )
                            {
                                hr = pSample->SetLength( cbSample );
                                assert( SUCCEEDED( hr ) );
                            }

                            if( SUCCEEDED( hr ) )
                            {
                                hr = m_pIWMWriterPreprocess->PreprocessSample( dwCurrentInput, // input number
                                                                               qwPresentTime,  // presentation time
                                                                               0,              // flags
                                                                               pSample );      // the data
                                assert( SUCCEEDED( hr ) );
                            }

                            SAFE_RELEASE( pSample );

                            if( FAILED( hr ) )
                            {
                                return( hr );
                            }
                        }
                        else
                        {
                            //
                            // 0-sized sample; that's OK, just skip the frame.
                            //
                            cbBuffer = 0;
                            cSamples = 1;
                        }

                        dwCurrentSample += cSamples;

                        //
                        // Update presentation time for this AVI input.
                        // Note: The writer expects presentation times to be in 100-nanosecond units.
                        //
                        if( WMMEDIATYPE_Audio == pStream->m_Type )
                        {
                            qwPresentTime += 10000000 * (QWORD)cbBuffer 
                                             / pStream->m_pWFX->nAvgBytesPerSec;
                        }
                        else if( WMMEDIATYPE_Video == pStream->m_Type )
                        {
                            qwPresentTime = 10000000 * (QWORD)dwCurrentSample 
                                            * CurrentStreamInfo.dwScale
                                            / CurrentStreamInfo.dwRate;
                        }

                        while( dwCurrentProgress <= dwCurrentSample * 100 / cCurrentSamples )
                        {
                            _tprintf( _T( "*" ) );
                            dwCurrentProgress += 2;
                        }

                        //
                        // Don't do preprocessing again if the maximum duration time is reached.
                        //
                        if( qwPresentTime >= m_qwMaxDuration )
                        {
                            _tprintf( _T( "\nMax duration is reached" ) );
                            break;
                        }
                    }

                    hr = m_pIWMWriterPreprocess->EndPreprocessingPass( dwCurrentInput, 0 );
                    assert( SUCCEEDED( hr ) );
                    if( FAILED( hr ) )
                    {
                        return( hr );
                    }

                    dwNumPasses--;
                }

                _tprintf( _T( "\n" ) );
            }
        }

        pStream = m_Inputs.Iterate( ITER_NEXT, &pStreamElem );
    }

    return( hr );
}

//------------------------------------------------------------------------------
// Name: CUncompAVIToWMV::WriteSample()
// Desc: Writes next sample from AVI input to WMWriter. Updates presentation 
//       time for AVI input
//------------------------------------------------------------------------------
HRESULT CUncompAVIToWMV::WriteSample( CWMInput * pInput )
{
    HRESULT         hr = S_OK;
    LONG            cbSample = 0;
    LONG            cSamples = 0;
    INSSBuffer      * pSample  = NULL;
    BYTE            * pbBuffer = NULL;
    DWORD           cbBuffer = 0;
    LONG            cSamplesToRead = 0;

    if( NULL == pInput )
    {
        return( E_INVALIDARG );
    }

    if( WMMEDIATYPE_Audio == pInput->m_Type )
    {
        //
        // We want to read one half-second of audio at a time
        //
        cSamplesToRead = ( pInput->m_pWFX->nAvgBytesPerSec * 4 ) 
            / ( pInput->m_pWFX->nChannels * pInput->m_pWFX->wBitsPerSample );
    }
    else if( WMMEDIATYPE_Video == pInput->m_Type )
    {
        cSamplesToRead = AVISTREAMREAD_CONVENIENT;
    }

    hr = AVIStreamRead( pInput->m_pAVIStream, 
                        pInput->m_dwCurrentSample, 
                        cSamplesToRead, 
                        0, 
                        0, 
                        &cbSample, 
                        &cSamples );
    if( FAILED( hr ) )
    {
        return( hr );
    }

    if( 0 < cbSample )
    {
        hr = m_pWMWriter->AllocateSample( cbSample, &pSample );
        assert( SUCCEEDED( hr ) );

        if( SUCCEEDED( hr ) )
        {
            hr = pSample->GetBufferAndLength( &pbBuffer, &cbBuffer );
            assert( SUCCEEDED( hr ) );
        }

        if( SUCCEEDED( hr ) )
        {
            hr = AVIStreamRead( pInput->m_pAVIStream, 
                                pInput->m_dwCurrentSample, 
                                cSamples, 
                                pbBuffer, 
                                cbBuffer, 
                                &cbSample, 
                                &cSamples );
            assert( SUCCEEDED( hr ) );
        }

        if( SUCCEEDED( hr ) )
        {
            hr = pSample->SetLength( cbSample );
            assert( SUCCEEDED( hr ) );
        }

        if( SUCCEEDED( hr ) )
        {
            //
            // Add SMPTE time code to the first video stream.
            //
            if( pInput->m_fAddSMPTE )
            {
                hr = AddSMPTETimeCode( pSample, pInput->m_qwPresentTime );
                assert( SUCCEEDED( hr ) );
            }
        }

        if( SUCCEEDED( hr ) )
        {
            hr = m_pWMWriter->WriteSample( pInput->m_dwInput,           // input number
                                           pInput->m_qwPresentTime,     // presentation time
                                           0,                           // flags
                                           pSample );                   // the data
        }

        SAFE_RELEASE( pSample );

        if( FAILED( hr ) )
        {
            return( hr );
        }
    }
    else    // 0 == cbSample
    {
        if( WMMEDIATYPE_Video == pInput->m_Type )
        {
            //
            // 0-sized sample; that's OK, just skip the frame.
            //
            cSamples = 1;
        }
        else if( WMMEDIATYPE_Audio == pInput->m_Type )
        {
            return( hr );
        }
    }

    if( m_fArbitrary && WMMEDIATYPE_Audio == pInput->m_Type )
    {
        //
        // Arbitrary stream. This is for example purposes; there is no need 
        // to send an arbitrary stream for an audio stream.
        //
        hr = m_pWMWriter->AllocateSample( sizeof(DWORD), &pSample );
        assert( SUCCEEDED( hr ) );

        if( SUCCEEDED( hr ) )
        {
            hr = pSample->GetBuffer( &pbBuffer );
            assert( SUCCEEDED( hr ) );
        }

        if( SUCCEEDED( hr ) )
        {
            memcpy( pbBuffer, &pInput->m_dwCurrentSample, sizeof(DWORD) );

            hr = m_pWMWriter->WriteSample( m_dwArbitraryInput,      // input number
                                           pInput->m_qwPresentTime, // presentation time
                                           0,                       // flags
                                           pSample );               // the data
        }

        SAFE_RELEASE( pSample );
    }

    pInput->m_dwCurrentSample += cSamples;

    //
    // Update presentation time for this AVI input.
    // Note: The writer expects presentation times to be in 100-nanosecond units.
    //
    if( WMMEDIATYPE_Audio == pInput->m_Type )
    {
        pInput->m_qwPresentTime += 10000000 * (QWORD)cbBuffer 
                                   / pInput->m_pWFX->nAvgBytesPerSec;
    }
    else if( WMMEDIATYPE_Video == pInput->m_Type )
    {
        pInput->m_qwPresentTime = 10000000 * (QWORD)pInput->m_dwCurrentSample
                                  * pInput->m_StreamInfo.dwScale
                                  / pInput->m_StreamInfo.dwRate;
    }

    return( hr );
}

//------------------------------------------------------------------------------
// Name: CUncompAVIToWMV::SetupSMPTE()
// Desc: 1. Finds the video stream to which SMPTE code will be written.
//       2. Adds a data unit extension to this video stream to store the SMPTE code.
//       3. Saves the frame rate and sets the m_fAddSMPTE flag of this video stream.
//------------------------------------------------------------------------------
HRESULT CUncompAVIToWMV::SetupSMPTE( IWMProfile * pProfile )
{
    HRESULT                 hr = S_OK;
    IWMStreamConfig         * pIWMStreamConfig = NULL;
    IWMStreamConfig2        * pIWMStreamConfig2 = NULL;
    IWMMediaProps           * pMediaProps = NULL;
    WM_MEDIA_TYPE           * pMediaType = NULL;
    DWORD                   cbMediaType = 0;
    DWORD                   cStreams = 0;
    LPWSTR                  pwszConnectionName = NULL;
    WORD                    cchConnectionName = 0;
    GUID                    guidStreamType;

    if( NULL == pProfile )
    {
        return( E_INVALIDARG );
    }

    hr = pProfile->GetStreamCount( &cStreams );
    if( FAILED( hr ) )
    {
        return( hr );
    }

    //
    // Find the first video stream.
    // Currently, SMPTE only supports one video stream.
    //
    for( DWORD i = 0; i < cStreams; i++ )
    {
        hr = pProfile->GetStream( i, &pIWMStreamConfig );
        if( FAILED( hr ) )
        {
            break;
        }

        hr = pIWMStreamConfig->GetStreamType( &guidStreamType );
        if( FAILED( hr ) )
        {
            break;
        }

        if( WMMEDIATYPE_Video == guidStreamType )
        {
            break;
        }

        SAFE_RELEASE( pIWMStreamConfig );
    }

    if( FAILED( hr ) )
    {
        SAFE_RELEASE( pIWMStreamConfig );
        return( hr );
    }

    if( NULL == pIWMStreamConfig )
    {
        return( E_INVALIDARG );
    }

    do
    {
        //
        // We need to call IWMStreamConfig2::AddDataUnitExtension to add 
        // a data unit extension to store SMPTE code. 
        // 
        hr = pIWMStreamConfig->QueryInterface( IID_IWMStreamConfig2, 
                                               (void **)&pIWMStreamConfig2 );
        if( FAILED( hr ) )
        {
            break;
        }

        hr = pIWMStreamConfig2->AddDataUnitExtension( WM_SampleExtensionGUID_Timecode, 
                                                      sizeof(WMT_TIMECODE_EXTENSION_DATA), 
                                                      NULL, 
                                                      0 );
        if( FAILED( hr ) )
        {
            break;
        }

        //
        // Update the profile.
        //
        hr = hr = pProfile->ReconfigStream( pIWMStreamConfig );
        if( FAILED( hr ) )
        {
            break;
        }

        //
        // Get the frame rate and input number of this video stream
        //

        hr = pIWMStreamConfig->QueryInterface( IID_IWMMediaProps, 
                                               (void **)&pMediaProps );
        if( FAILED( hr ) )
        {
            break;
        }

        hr = pMediaProps->GetMediaType( NULL, &cbMediaType );
        if( FAILED( hr ) )
        {
            break;
        }

        pMediaType = (WM_MEDIA_TYPE *) new BYTE[ cbMediaType ];
        if( !pMediaType )
        {
            hr = E_OUTOFMEMORY;
            break;
        }

        hr = pMediaProps->GetMediaType( pMediaType, &cbMediaType );
        if( FAILED( hr ) )
        {
            break;
        }

        //
        // Save the AvgTimePerFrame of this stream
        //
        WMVIDEOINFOHEADER * pVIH = (WMVIDEOINFOHEADER *) pMediaType->pbFormat;
        m_qwSMPTEAvgTimePerFrame = pVIH->AvgTimePerFrame;

        hr = pIWMStreamConfig->GetConnectionName( NULL, &cchConnectionName );
        if( FAILED( hr ) )
        {
            break;
        }

        pwszConnectionName = new WCHAR[ cchConnectionName + 1 ];
        if( NULL == pwszConnectionName )
        {
            hr = E_OUTOFMEMORY;
            break;
        }

        hr = pIWMStreamConfig->GetConnectionName( pwszConnectionName, 
                                                  &cchConnectionName );
        if( FAILED( hr ) )
        {
            break;
        }

        //
        // Look for an AVI input with the matching connection name and 
        // set the m_fAddSMPTE flag
        //
        CWMInput * pInput = m_Inputs.Iterate( ITER_FIRST );
        while( NULL != pInput )
        {
            if( 0 == wcscmp( pInput->m_pwszConnectionName, pwszConnectionName ) )
            {
                break;
            }

            pInput = m_Inputs.Iterate( ITER_NEXT );
        }

        if( NULL == pInput )
        {
            hr = E_INVALIDARG;
            break;
        }
        else
        {
            pInput->m_fAddSMPTE = TRUE;
        }

    } while( FALSE );

    SAFE_ARRAYDELETE( pMediaType );
    SAFE_ARRAYDELETE( pwszConnectionName );
    SAFE_RELEASE( pMediaProps );
    SAFE_RELEASE( pIWMStreamConfig2 );
    SAFE_RELEASE( pIWMStreamConfig );

    return( hr );
}

//------------------------------------------------------------------------------
// Name: CUncompAVIToWMV::AddSMPTETimeCode()
// Desc: Adds SMPTE time code.
//------------------------------------------------------------------------------
HRESULT CUncompAVIToWMV::AddSMPTETimeCode( INSSBuffer * pSample, 
                                           QWORD qwPresTime )
{
    HRESULT         hr = S_OK;
    INSSBuffer3     * pNSSBuffer3 = NULL;

    if( NULL == pSample || 0 == m_qwSMPTEAvgTimePerFrame )
    {
        return( E_INVALIDARG );
    }

    hr = pSample->QueryInterface( IID_INSSBuffer3, (void**)&pNSSBuffer3 );
    if( SUCCEEDED( hr ) )
    {
        WMT_TIMECODE_EXTENSION_DATA SMPTEExtData;
        DWORD dwTimeCode;
        DWORD dwFrameNumber;

        ZeroMemory( &SMPTEExtData, sizeof( SMPTEExtData ) );

        //
        // wRange specifies the range to which the time code belongs.
        // You can change 86400 to other small values if you like.
        //
        SMPTEExtData.wRange = (WORD)( qwPresTime / 10000000 / 86400 );
        
        dwTimeCode = (DWORD)( ( qwPresTime / 10000000 ) % 86400 );  // time in seconds
        dwFrameNumber = (DWORD)( ( qwPresTime % 10000000 )  / m_qwSMPTEAvgTimePerFrame );

        // 
        // Time code is stored so that the hexadecimal value is read as if 
        // it were a decimal value. That is, the time code value 0x01133512 
        // does not represent decimal 18035986, rather it specifies 1 hour,
        // 13 minutes, 35 seconds, and 12 frames.
        //
        SMPTEExtData.dwTimecode = ( ( dwTimeCode / 3600 ) << 24 ) |         // Hours
                                  ( ( dwTimeCode / 60 % 60 ) << 16 ) |      // Minutes
                                  ( ( dwTimeCode % 60 ) << 8 ) |            // Seconds
                                  dwFrameNumber;                            // Frames

        hr = pNSSBuffer3->SetProperty( WM_SampleExtensionGUID_Timecode,
                                       (BYTE *) &SMPTEExtData, 
                                       sizeof( SMPTEExtData ) );
    }

    SAFE_RELEASE( pNSSBuffer3 );

    return( hr );
}

//------------------------------------------------------------------------------
// Name: CUncompAVIToWMV::SetAttribute()
// Desc: Add attribute to header info.
//------------------------------------------------------------------------------
HRESULT CUncompAVIToWMV::SetAttribute( CONTENT_DESC* pCntDesc )
{
    HRESULT             hr = S_OK;
    WMT_ATTR_DATATYPE   enumDataType;
    DWORD               dwValueLength = 0;
    BYTE                * pbValue = NULL;
    IWMHeaderInfo       * pHeaderInfo = NULL;
    WCHAR               * pwszName = NULL;

    do
    {
        //
        // Determine the attribute type and set the length and value properly
        //
        if( 0 == _tcsicmp( _T( "string" ), pCntDesc->ptszType ) )
        {
            enumDataType = WMT_TYPE_STRING;

            //
            // Convert TCHAR string value to WCHAR string value
            //
            hr = ConvertTCharToWChar( pCntDesc->ptszValue, (WCHAR **)&pbValue );
            if( FAILED( hr ) )
            {
                _tprintf( _T( "Can not convert attribute value to wchar string (hr=0x%08x)\n" ), hr );
                break;
            }

            dwValueLength = ( wcslen( (WCHAR *)pbValue ) + 1 ) * sizeof(WCHAR);
        }
        else if( 0 == _tcsicmp( _T( "qword" ), pCntDesc->ptszType ) )
        {
            enumDataType = WMT_TYPE_QWORD;
            dwValueLength = sizeof(QWORD);

            pbValue = new BYTE[ dwValueLength ];
            if( NULL == pbValue )
            {
                hr = E_OUTOFMEMORY;
                _tprintf( _T( "Insufficient Memory\n") );
                break;
            }

            *(QWORD *)pbValue = _ttoi64( pCntDesc->ptszValue );
        }
        else if( 0 == _tcsicmp( _T( "dword" ), pCntDesc->ptszType ) )
        {
            enumDataType = WMT_TYPE_DWORD;
            dwValueLength = sizeof(DWORD);

            pbValue = new BYTE[ dwValueLength ];
            if( NULL == pbValue )
            {
                hr = E_OUTOFMEMORY;
                _tprintf( _T( "Insufficient Memory\n") );
                break;
            }

            *(DWORD *)pbValue = _ttoi( pCntDesc->ptszValue );
        }
        else if( 0 == _tcsicmp( _T( "word" ), pCntDesc->ptszType ) )
        {
            enumDataType = WMT_TYPE_WORD;
            dwValueLength = sizeof( WORD );

            pbValue = new BYTE[ dwValueLength ];
            if( NULL == pbValue )
            {
                hr = E_OUTOFMEMORY;
                _tprintf( _T( "Insufficient Memory\n") );
                break;
            }

            *(WORD *)pbValue = (WORD)_ttoi( pCntDesc->ptszValue );
        }
        else if( 0 == _tcsicmp( _T( "binary" ), pCntDesc->ptszType ) )
        {
            //
            // If thehe binary data is read as Unicode, the string will have unwanted 
            // characters in it. For example, the binary data "abcd" will be read 
            // as "a\0b\0c\0d\0". Its essential to convert the data back to 
            // binary by removing all the unwanted characters.
            //

            enumDataType = WMT_TYPE_BINARY;

            hr = ConvertTCharToChar( pCntDesc->ptszValue, (CHAR **)&pbValue );
            if( FAILED( hr ) )
            {
                _tprintf( _T( "Can not convert attribute value to char string (hr=0x%08x)\n" ), hr );
                break;
            }

            dwValueLength = strlen( (CHAR *)pbValue );
        }
        else if( 0 == _tcsicmp( _T( "bool" ), pCntDesc->ptszType ) )
        {
            enumDataType = WMT_TYPE_BOOL;
            dwValueLength = sizeof( BOOL );

            pbValue = new BYTE[ dwValueLength ];
            if( NULL == pbValue )
            {
                hr = E_OUTOFMEMORY;
                _tprintf( _T( "Insufficient Memory\n") );
                break;
            }

            if( 0 == _tcsicmp( _T( "true" ), pCntDesc->ptszValue ) )
            {
                *(BOOL *)pbValue = TRUE;
            }
            else if( 0 == _tcsicmp( _T( "false" ), pCntDesc->ptszValue ) )
            {
                *(BOOL *)pbValue = TRUE;
            }
            else
            {
                hr = E_INVALIDARG;
                _tprintf( _T( "Invalid boolean attribute\n") );
                break;
            }
        }
        else
        {
            hr = E_INVALIDARG;
            _tprintf( _T( "Invalid attribute type\n") );
            break;
        }

        //
        // Get the header info interface and give it this new attribute
        //
        hr = m_pWMWriter->QueryInterface( IID_IWMHeaderInfo, (void **)&pHeaderInfo );
        if( FAILED( hr ) )
        {
            _tprintf( _T( "Failed to query IWMHeaderInfo interface (hr=0x%08x)\n" ), hr );
            break;
        }

        //
        // Convert TCHAR string value to WCHAR string value
        //
        hr = ConvertTCharToWChar( pCntDesc->ptszName, &pwszName );
        if( FAILED( hr ) )
        {
            _tprintf( _T( "Failed to convert attribute name to wchar string (hr=0x%08x)\n" ), hr );
            break;
        }

        hr = pHeaderInfo->SetAttribute( pCntDesc->wStreamNum, 
                                        pwszName, 
                                        enumDataType, 
                                        pbValue, 
                                        (WORD)dwValueLength );
        if( FAILED( hr ) )
        {
            _tprintf( _T( "Failed to set attribute %s (hr=0x%08x)\n" ), pCntDesc->ptszName, hr );
            break;
        }
    }
    while( FALSE );

    SAFE_ARRAYDELETE( pwszName );
    SAFE_ARRAYDELETE( pbValue );
    SAFE_RELEASE( pHeaderInfo );

    return( hr );
}

//------------------------------------------------------------------------------
// Name: CUncompAVIToWMV::SetDRM()
// Desc: Adds DRM attributes.
//       When SUPPORT_DRM is not defined, link to wmvcore.lib.
//       When SUPPORT_DRM is defined, link to wmstubdrm.lib, which 
//       contains the certificate that you must first acquire from 
//       Microsoft before working with DRM. 
//------------------------------------------------------------------------------

HRESULT CUncompAVIToWMV::SetDRM()
{
#ifdef SUPPORT_DRM

    //
    // Tell the writer to turn on digital rights management by setting the use_drm attribute to 1
    //
    HRESULT         hr = S_OK;
    IWMHeaderInfo   * pHeaderInfo = NULL;

    do
    {
        hr = m_pWMWriter->QueryInterface( IID_IWMHeaderInfo, (void **)&pHeaderInfo );
        if( FAILED( hr ) )
        {
            _tprintf( _T( "Failed to query IWMHeaderInfo interface (hr=0x%08x)\n" ), hr );
            break;
        }

        BOOL fUseDRM = TRUE;
        hr = pHeaderInfo->SetAttribute( 0, 
                                        g_wszWMUse_DRM, 
                                        WMT_TYPE_BOOL, 
                                        (BYTE *) &fUseDRM, 
                                        sizeof( BOOL ) );
        if( FAILED( hr ) )
        {
            _tprintf( _T( "Failed to set Use_DRM attribute (hr=0x%08x)\n" ), hr );
            break;
        }

        DWORD dwDRMFlags = WMT_RIGHT_PLAYBACK | WMT_RIGHT_COPY_TO_NON_SDMI_DEVICE |  WMT_RIGHT_COPY_TO_CD;
        hr = pHeaderInfo->SetAttribute( 0, 
                                        g_wszWMDRM_Flags, 
                                        WMT_TYPE_DWORD, 
                                        (BYTE *) &dwDRMFlags, 
                                        sizeof(DWORD) );
        if( FAILED( hr ) )
        {
            _tprintf( _T( "Failed to set DRM_Flags attribute (hr=0x%08x)\n" ), hr );
            break;
        }

        DWORD dwDRMLevel = 150;
        hr = pHeaderInfo->SetAttribute( 0, 
                                        g_wszWMDRM_Level, 
                                        WMT_TYPE_DWORD, 
                                        (BYTE *) &dwDRMLevel, 
                                        sizeof(DWORD) );
        if( FAILED( hr ) )
        {
            _tprintf( _T( "Failed to set DRM_Level attribute (hr=0x%08x)\n" ), hr );
            break;
        }
    }
    while( FALSE );

    SAFE_RELEASE( pHeaderInfo );

    return( hr );

#else

    _tprintf( _T( "DRM is not implemented\n") );
    return( E_NOTIMPL );

#endif
}

//------------------------------------------------------------------------------
// Name: CUncompAVIToWMV::GetTokensFromFile()
// Desc: Creates a list of tokens from an ASCII input file. 
//       Tokens should be separated by CR/LF..
//------------------------------------------------------------------------------
HRESULT CUncompAVIToWMV::GetTokensFromFile( LPCTSTR ptszFileName, 
                                            __out_ecount(*TokenNum) LPTSTR pptszTokens[], 
                                            int * TokenNum )
{
    HRESULT hr = S_OK;
    HANDLE hFile = NULL;
    LPSTR pszBuffer = NULL;
    DWORD dwFileSize = 0;
    DWORD dwRead = 0;
    int nTokenCount = 0;


    if( NULL == ptszFileName || NULL == pptszTokens || NULL == TokenNum )
    {
        return( E_INVALIDARG );
    }

    hFile = CreateFile( ptszFileName, 
                        GENERIC_READ, 
                        FILE_SHARE_READ,
                        NULL, 
                        OPEN_EXISTING, 
                        FILE_ATTRIBUTE_NORMAL, 
                        NULL );
    if( INVALID_HANDLE_VALUE == hFile )
    {
        return( HRESULT_FROM_WIN32( GetLastError() ) );
    }

    do
    {
        if( FILE_TYPE_DISK != GetFileType( hFile ) )
        {
            hr = NS_E_INVALID_NAME;
            break;
        }

        dwFileSize = GetFileSize( hFile, NULL );
        if( INVALID_FILE_SIZE == dwFileSize )
        {
            hr = HRESULT_FROM_WIN32( GetLastError() );
            break;
        }

        pszBuffer = ( LPSTR ) VirtualAlloc( 0, 
                                            dwFileSize + 1, 
                                            MEM_COMMIT, 
                                            PAGE_READWRITE);
        if( NULL == pszBuffer )
        {
            hr = E_OUTOFMEMORY;
            break;
        }

        if( !ReadFile( hFile, pszBuffer, dwFileSize, &dwRead, NULL ) ||
            dwFileSize != dwRead )
        {
            hr = HRESULT_FROM_WIN32( GetLastError() );
            break;
        }

        pszBuffer[ dwFileSize ] = 0x0d;
        nTokenCount = 0;
		
		char* context = NULL;
        LPSTR pToken = strtok_s( pszBuffer, "\r\n", &context);

        while( NULL != pToken )
        {
            hr = ConvertCharToTChar( pToken, &pptszTokens[ nTokenCount ] );
            if( FAILED( hr ) )
            {
                _tprintf( _T( "Failed to convert input file name to tchar string (hr=0x%08x)\n" ), hr );
                break;
            }

            nTokenCount ++;
            if( nTokenCount >= *TokenNum )
            {
                break;
            }

			char* context = NULL;
            pToken = strtok_s( NULL, "\r\n", &context);
        }

        *TokenNum = nTokenCount;

    } while( FALSE );

    if( NULL != hFile )
    {
        CloseHandle( hFile );
    }

    if( NULL != pszBuffer )
    {
        VirtualFree( (LPVOID)pszBuffer, 0, MEM_RELEASE );
    }

    return( hr );
}

////////////////////////////////////////////////////////////////////////////////
// The following functions work in a very similar manner to their counterparts 
// in the WmGenProfile sample. They are used to add a video or audio stream 
// to a profile.
////////////////////////////////////////////////////////////////////////////////

//------------------------------------------------------------------------------
// Name: CUncompAVIToWMV::SetStreamBasics()
// Desc: Creates and configures a stream.
//------------------------------------------------------------------------------
HRESULT CUncompAVIToWMV::SetStreamBasics( IWMStreamConfig * pIWMStreamConfig,
                                          IWMProfile * pIWMProfile,
                                          __in LPWSTR pwszStreamName,
                                          __in LPWSTR pwszConnectionName,
                                          DWORD dwBitrate,
                                          WM_MEDIA_TYPE * pmt )
{
    HRESULT hr = S_OK;
    IWMMediaProps * pIWMMediaProps = NULL;
    IWMStreamConfig * pIWMStreamConfig2 = NULL;
    WORD wStreamNum = 0;

    if( NULL == pIWMStreamConfig || NULL == pIWMProfile || NULL == pmt )
    {
        return( E_INVALIDARG );
    }

    do
    {
        hr = pIWMProfile->CreateNewStream( pmt->majortype, &pIWMStreamConfig2 );
        if( FAILED( hr ) )
        {
            break;
        }

        hr = pIWMStreamConfig2->GetStreamNumber( &wStreamNum );
        SAFE_RELEASE( pIWMStreamConfig2 );
        if( FAILED( hr ) )
        {
            break;
        }

        hr = pIWMStreamConfig->SetStreamNumber( wStreamNum );
        if( FAILED( hr ) )
        {
            break;
        }

        hr = pIWMStreamConfig->SetStreamName( pwszStreamName );
        if( FAILED( hr ) )
        {
            break;
        }

        hr = pIWMStreamConfig->SetConnectionName( pwszConnectionName );
        if( FAILED( hr ) )
        {
            break;
        }

        hr = pIWMStreamConfig->SetBitrate( dwBitrate );
        if( FAILED( hr ) )
        {
            break;
        }

        hr = pIWMStreamConfig->QueryInterface( IID_IWMMediaProps, 
                                               (void **) &pIWMMediaProps );
        if( FAILED( hr ) )
        {
            break;
        }

        hr = pIWMMediaProps->SetMediaType( pmt );
        if( FAILED( hr ) )
        {
            break;
        }
    }
    while( FALSE );

    SAFE_RELEASE( pIWMMediaProps );

    return( hr );
}

//------------------------------------------------------------------------------
// Name: CUncompAVIToWMV::AddVideoStream()
// Desc: Configures and adds a video stream.
//------------------------------------------------------------------------------
HRESULT CUncompAVIToWMV::AddVideoStream( IWMProfile * pIWMProfile,
                                         WMVIDEOINFOHEADER * pInputVIH,
                                         WORD  * pwStreamNum,
                                         DWORD dwQuality,
                                         DWORD dwSecPerKey,
                                         __out LPWSTR * pwszConnectionName )
{
    HRESULT hr = S_OK;

    IWMProfileManager   * pManager = NULL;
    IWMCodecInfo        * pCodecInfo = NULL;
    IWMStreamConfig     * pStreamConfig = NULL;
    IWMVideoMediaProps  * pMediaProps = NULL;
    WM_MEDIA_TYPE       * pMediaType = NULL;

    if( NULL == pIWMProfile || 
        NULL == pInputVIH || 
        NULL == pwStreamNum || 
        NULL == pwszConnectionName )
    {
        return( E_INVALIDARG );
    }

    do
    {
        hr = WMCreateProfileManager( &pManager );
        if( FAILED( hr ) )
        {
            break;
        }

        hr = pManager->QueryInterface( IID_IWMCodecInfo, 
                                       (void **) &pCodecInfo );
        if( FAILED( hr ) )
        {
            break;
        }

        DWORD cCodecs;

        hr = pCodecInfo->GetCodecInfoCount( WMMEDIATYPE_Video, &cCodecs );
        if( FAILED( hr ) )
        {
            break;
        }

        //
        // Search from the last codec because the last codec usually 
        // is the newest codec. 
        //
        for( int i = cCodecs-1; i >= 0; i-- )
        {
            DWORD cFormats;
            hr = pCodecInfo->GetCodecFormatCount( WMMEDIATYPE_Video, 
                                                  i, 
                                                  &cFormats );
            if( FAILED( hr ) )
            {
                break;
            }

            DWORD j;
            for( j = 0; j < cFormats; j++ )
            {
                SAFE_RELEASE( pStreamConfig );

                hr = pCodecInfo->GetCodecFormat( WMMEDIATYPE_Video, 
                                                 i, 
                                                 j, 
                                                 &pStreamConfig );
                if( FAILED( hr ) )
                {
                    break;
                }

                SAFE_RELEASE( pMediaProps );

                hr = pStreamConfig->QueryInterface( IID_IWMVideoMediaProps, 
                                                    (void **) &pMediaProps );
                if( FAILED( hr ) )
                {
                    break;
                }

                DWORD cbMT;

                hr = pMediaProps->GetMediaType( NULL, &cbMT );

                if( FAILED( hr ) )
                {
                    break;
                }

                SAFE_ARRAYDELETE( pMediaType );

                pMediaType = (WM_MEDIA_TYPE *) new BYTE[ cbMT ];
                if( !pMediaType )
                {
                    hr = E_OUTOFMEMORY;
                    break;
                }

                hr = pMediaProps->GetMediaType( pMediaType, &cbMT );
                if( FAILED( hr ) )
                {
                    break;
                }

                if( pMediaType->formattype != WMFORMAT_VideoInfo )
                {
                    SAFE_RELEASE( pStreamConfig );
                    continue;
                }

                WMVIDEOINFOHEADER * pVIH = (WMVIDEOINFOHEADER *) pMediaType->pbFormat;

                if( pVIH->bmiHeader.biBitCount >= pInputVIH->bmiHeader.biBitCount )
                {
                    break;
                }

                SAFE_RELEASE( pStreamConfig );
            }

            if( FAILED( hr ) || NULL != pStreamConfig )
            {
                break;
            }
        }

        if( FAILED( hr ) )
        {
            break;
        }

        if( NULL == pStreamConfig )
        {
            hr = NS_E_VIDEO_CODEC_NOT_INSTALLED;
            break;
        }

        WMVIDEOINFOHEADER * pVIH = (WMVIDEOINFOHEADER *) pMediaType->pbFormat;

        //
        // Set the target bitrate to 1/30 of the source bitrate 
        // since it's compressing. 
        //
        pVIH->dwBitRate = pInputVIH->dwBitRate / 30;

        pVIH->rcSource.right = pInputVIH->rcSource.right;
        pVIH->rcSource.bottom = pInputVIH->rcSource.bottom;
        pVIH->rcTarget.right = pInputVIH->rcTarget.right;
        pVIH->rcTarget.bottom = pInputVIH->rcTarget.bottom;
        pVIH->bmiHeader.biWidth = pInputVIH->bmiHeader.biWidth;
        pVIH->bmiHeader.biHeight = pInputVIH->bmiHeader.biHeight;

        pVIH->AvgTimePerFrame = pInputVIH->AvgTimePerFrame;

        hr = pMediaProps->SetQuality( dwQuality );
        if( FAILED( hr ) )
        {
            break;
        }

        hr = pMediaProps->SetMaxKeyFrameSpacing( 10000000 * (QWORD)dwSecPerKey );
        if( FAILED( hr ) )
        {
            break;
        }

        hr = SetStreamBasics( pStreamConfig, 
                              pIWMProfile, 
                              L"Video Stream", 
                              L"Video", 
                              pVIH->dwBitRate, pMediaType );
        if( FAILED( hr ) )
        {
            break;
        }

        *pwszConnectionName = new WCHAR[ wcslen( wszDefaultConnectionName ) + 4 ];
        if( NULL == *pwszConnectionName )
        {
            hr = E_OUTOFMEMORY;
            break;
        }

        hr = pIWMProfile->AddStream( pStreamConfig );
        if( FAILED( hr ) )
        {
            SAFE_ARRAYDELETE( *pwszConnectionName );
            break;
        }

        hr = pStreamConfig->GetStreamNumber( pwStreamNum );
        if( FAILED( hr ) )
        {
            SAFE_ARRAYDELETE( *pwszConnectionName );
            break;
        }

        //
        // Each stream in the profile has to have a unique connection name.
        // Let's use the stream number to create it.
        //
        if( *pwStreamNum > 127 )
        {
            hr = E_FAIL;
            break;
        }

        (void)StringCchPrintfW( *pwszConnectionName, wcslen( wszDefaultConnectionName) + 4, L"%s%d", wszDefaultConnectionName, (DWORD)*pwStreamNum );

        hr = pStreamConfig->SetConnectionName( *pwszConnectionName );
        if( FAILED( hr ) )
        {
            SAFE_ARRAYDELETE( *pwszConnectionName );
            break;
        }

        hr = pIWMProfile->ReconfigStream( pStreamConfig );
        if( FAILED( hr ) )
        {
            SAFE_ARRAYDELETE( *pwszConnectionName );
            break;
        }
    }
    while( FALSE );

    SAFE_RELEASE( pCodecInfo );
    SAFE_RELEASE( pStreamConfig );
    SAFE_RELEASE( pMediaProps );
    SAFE_RELEASE( pManager );
    SAFE_ARRAYDELETE( pMediaType );

    return( hr );
}

//------------------------------------------------------------------------------
// Name: CUncompAVIToWMV::AddAudioStream()
// Desc: Configures and adds an audio stream.
//------------------------------------------------------------------------------
HRESULT CUncompAVIToWMV::AddAudioStream( IWMProfile * pIWMProfile,
                                         DWORD dwSampleRate,
                                         DWORD dwChannels,
                                         WORD wBitsPerSample,
                                         WORD  * pwStreamNum,
                                         __out LPWSTR * pwszConnectionName )
{
    HRESULT             hr = S_OK;
    IWMProfileManager   * pIWMProfileManager = NULL;
    IWMStreamConfig     * pIWMStreamConfig = NULL;
    IWMMediaProps       * pIMP  = NULL;
    IWMCodecInfo        * pIWMInfo = NULL;
    WAVEFORMATEX        * pWfx = NULL;
    WM_MEDIA_TYPE       * pType = NULL;

    if( NULL == pIWMProfile || NULL == pwStreamNum || NULL == pwszConnectionName )
    {
        return( E_INVALIDARG );
    }

    do
    {
        hr = WMCreateProfileManager( &pIWMProfileManager );
        if( FAILED( hr ) )
        {
            break;
        }

        hr = pIWMProfileManager->QueryInterface( IID_IWMCodecInfo, (void **) &pIWMInfo );
        if( FAILED( hr ) )
        {
            break;
        }

        DWORD i, j;
        DWORD cCodecs;

        hr = pIWMInfo->GetCodecInfoCount( WMMEDIATYPE_Audio, &cCodecs );
        if( FAILED( hr ) )
        {
            break;
        }

        for( i = 0; i < cCodecs; i++ )
        {
            DWORD cFormats;
            hr = pIWMInfo->GetCodecFormatCount( WMMEDIATYPE_Audio, 
                                                i, 
                                                &cFormats );
            if( FAILED( hr ) )
            {
                break;
            }

            //
            // Find a proper format in this codec 
            //
            for( j = 0; j < cFormats; j++ )
            {
                if( NULL != pType )
                {
                    SAFE_ARRAYDELETE( pType );
                }

                DWORD cbType = 0;
                hr = pIWMInfo->GetCodecFormat( WMMEDIATYPE_Audio, 
                                               i, 
                                               j, 
                                               &pIWMStreamConfig );
                if( FAILED( hr ) )
                {
                    break;
                }

                SAFE_RELEASE( pIMP );

                hr = pIWMStreamConfig->QueryInterface( IID_IWMMediaProps, 
                                                       (void **)&pIMP );
                if( FAILED( hr ) )
                {
                    break;
                }

                hr = pIMP->GetMediaType( NULL, &cbType );
                if( FAILED( hr ) )
                {
                    break;
                }

                pType = (WM_MEDIA_TYPE *) new BYTE[ cbType ];
                if( NULL == pType )
                {
                    hr = E_OUTOFMEMORY;
                    break;
                }

                hr = pIMP->GetMediaType( pType, &cbType );
                if( FAILED( hr ) )
                {
                    break;
                }

                if( pType->formattype != WMFORMAT_WaveFormatEx )
                {
                    hr = E_FAIL;
                    break;
                }

                pWfx = (WAVEFORMATEX *) pType->pbFormat;

                //
                // This sample will use this format only if it has the same 
                // sample rate, channels and more bits per sample. 
                // This is not necessary, because normally the codec can convert 
                // the sample rate and bits per sample for you. 
                // 
                if( pWfx->nSamplesPerSec == dwSampleRate &&
                    pWfx->nChannels == dwChannels &&
                    pWfx->wBitsPerSample >= wBitsPerSample )
                {
                    break;
                }

                SAFE_RELEASE( pIWMStreamConfig );
            }

            if( FAILED( hr ) || NULL != pIWMStreamConfig )
            {
                break;
            }
        }

        if( FAILED( hr ) )
        {
            break;
        }

        if( NULL == pIWMStreamConfig )
        {
            hr = NS_E_AUDIO_CODEC_NOT_INSTALLED;
            break;
        }

        //
        // We found a valid WAVEFORMATEX; go ahead and set up the stream.
        //
        hr = SetStreamBasics( pIWMStreamConfig, 
                              pIWMProfile, 
                              L"Audio Stream", 
                              L"Audio", 
                              pWfx->nAvgBytesPerSec * 8, 
                              pType );
        if( FAILED( hr ) )
        {
            break;
        }

        *pwszConnectionName = new WCHAR[ wcslen( wszDefaultConnectionName ) + 4 ];
        if( NULL == *pwszConnectionName )
        {
            hr = E_OUTOFMEMORY;
            break;
        }

        hr = pIWMProfile->AddStream( pIWMStreamConfig );
        if( FAILED( hr ) )
        {
            SAFE_ARRAYDELETE( *pwszConnectionName );
            break;
        }

        hr = pIWMStreamConfig->GetStreamNumber( pwStreamNum );
        if( FAILED( hr ) )
        {
            SAFE_ARRAYDELETE( *pwszConnectionName );
            break;
        }

        //
        // Each stream in the profile has to have a unique connection name.
        // Let's use the stream number to create it.
        //

        if( *pwStreamNum > 127 )
        {
            hr = E_FAIL;
            break;
        }

        (void)StringCchPrintfW( *pwszConnectionName, wcslen( wszDefaultConnectionName ) + 4, L"%s%d", wszDefaultConnectionName, (DWORD)*pwStreamNum );

        hr = pIWMStreamConfig->SetConnectionName( *pwszConnectionName );
        if( FAILED( hr ) )
        {
            SAFE_ARRAYDELETE( *pwszConnectionName );
            break;
        }

        hr = pIWMProfile->ReconfigStream( pIWMStreamConfig );
        if( FAILED( hr ) )
        {
            SAFE_ARRAYDELETE( *pwszConnectionName );
            break;
        }
    }
    while( FALSE );

    SAFE_ARRAYDELETE( pType );
    SAFE_RELEASE( pIWMInfo );
    SAFE_RELEASE( pIWMStreamConfig );
    SAFE_RELEASE( pIMP );
    SAFE_RELEASE( pIWMProfileManager );

    return( hr );
}

//------------------------------------------------------------------------------
// Name: CompareMediaTypes()
// Desc: Used by CTSimpleList<CProfileStreams> to compare media types.
//------------------------------------------------------------------------------
BOOL CompareMediaTypes( const WM_MEDIA_TYPE * pMedia1, const WM_MEDIA_TYPE * pMedia2)
{
    if( pMedia1->majortype != pMedia2->majortype )
    {
        return( FALSE );
    }

    return( TRUE );
}

