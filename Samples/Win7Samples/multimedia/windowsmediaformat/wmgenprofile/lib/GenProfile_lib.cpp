//*****************************************************************************
//
// Microsoft Windows Media
// Copyright (C) Microsoft Corporation. All rights reserved.
//
// FileName:            GenProfile_lib.cpp
//
// Abstract:            The implementation for the GenProfile static library.
//
//*****************************************************************************

#include "stdafx.h"
#include "GenProfile_lib.h"
#include <intsafe.h>

struct PIXEL_FORMAT
{
    const GUID* guidFormat;
    DWORD dwFourCC;
    WORD wBitsPerPixel;
};

PIXEL_FORMAT PixelFormats[] = 
{
    { &WMMEDIASUBTYPE_RGB555,   BI_RGB,         16 },
    { &WMMEDIASUBTYPE_RGB24,    BI_RGB,         24 },
    { &WMMEDIASUBTYPE_RGB32,    BI_RGB,         32 },
    { &WMMEDIASUBTYPE_I420,     0x30323449,     12 },
    { &WMMEDIASUBTYPE_IYUV,     0x56555949,     12 },
    { &WMMEDIASUBTYPE_YV12,     0x32315659,     12 },
    { &WMMEDIASUBTYPE_YUY2,     0x32595559,     16 },
    { &WMMEDIASUBTYPE_UYVY,     0x59565955,     16 },
    { &WMMEDIASUBTYPE_YVYU,     0x55595659,     16 }
};


DWORD WaveFrequency[] =
{
    8000, 11025, 12000, 16000, 22050, 24000, 32000, 44100, 48000
};


//------------------------------------------------------------------------------
// Name: CopyMediaType()
// Desc: Allocates memory for a WM_MEDIA_TYPE and its format data and 
//       copies an existing media type into it.
//------------------------------------------------------------------------------
STDMETHODIMP CopyMediaType( WM_MEDIA_TYPE** ppmtDestination, 
                            WM_MEDIA_TYPE* pmtSource )
{
    if ( !ppmtDestination )
    {
        return E_POINTER;
    }
    if ( !pmtSource )
    {
        return E_NOTIMPL;
    }

    ULONG SizeToAlloc;

    if (FAILED(ULongAdd(sizeof( WM_MEDIA_TYPE ), pmtSource->cbFormat, &SizeToAlloc)))
    {
        return HRESULT_FROM_WIN32(ERROR_ARITHMETIC_OVERFLOW);
    }

    //
    // Create enough space for the media type and its format data
    //
    *ppmtDestination = (WM_MEDIA_TYPE*) new BYTE[ SizeToAlloc ];
    if ( !*ppmtDestination)
    {
        return E_OUTOFMEMORY;
    }

    //
    // Copy the media type and the format data
    //
    memcpy( *ppmtDestination, pmtSource, sizeof( WM_MEDIA_TYPE ) );
    (*ppmtDestination)->pbFormat = ( ((BYTE*) *ppmtDestination) + sizeof( WM_MEDIA_TYPE ) ); // Format data is immediately after media type
    memcpy( (*ppmtDestination)->pbFormat, pmtSource->pbFormat, pmtSource->cbFormat );

    return S_OK;
}


//------------------------------------------------------------------------------
// Name: EnsureIWMCodecInfo3()
// Desc: Creates an IWMCodecInfo3 interface if none exists, and ensures
//       an outstanding reference either way.  This way the IWMCodecInfo3
//       object is guaranteed to exist and isn't released too many times.
//------------------------------------------------------------------------------
STDMETHODIMP EnsureIWMCodecInfo3( IWMCodecInfo3** ppCodecInfo3 )
{
    HRESULT hr = S_OK;

    if ( !ppCodecInfo3 )
    {
        return E_POINTER;
    }

    do
    {
        if ( !*ppCodecInfo3 )
        {
            //
            // Create a new IWMCodecInfo3 object
            //
            IWMProfileManager* pProfileManager;
            hr = WMCreateProfileManager( &pProfileManager );
            if ( FAILED( hr ) )
            {
                break;
            }
            assert( pProfileManager );

            hr = pProfileManager->QueryInterface( IID_IWMCodecInfo3, (void**) ppCodecInfo3 );
            SAFE_RELEASE( pProfileManager );
            if ( FAILED( hr ) )
            {
                break;
            }
        }
        else
        {
            //
            // Add a reference to the existing object, so that it won't be destroyed during cleanup
            //
            SAFE_ADDREF( (*ppCodecInfo3) );
        }
        assert( *ppCodecInfo3 );

        //
        // It should now not matter if the IWMCodecInfo3 was just created or was passed in
        //
    }
    while ( FALSE );

    return hr;
}


//------------------------------------------------------------------------------
// Name: SetCodecVBRSettings()
// Desc: Enables VBR with the specified number of passes, or disables it.
//------------------------------------------------------------------------------
STDMETHODIMP SetCodecVBRSettings( IWMCodecInfo3* pCodecInfo3, 
                                  GUID guidCodecType, 
                                  DWORD dwCodecIndex, 
                                  BOOL fIsVBR, 
                                  DWORD dwVBRPasses )
{
    HRESULT hr;

    if ( !pCodecInfo3 )
    {
        return E_INVALIDARG;
    }

    do
    {
        //
        // Configure the codec to use or not use VBR as requested
        //
        hr = pCodecInfo3->SetCodecEnumerationSetting( guidCodecType, dwCodecIndex, g_wszVBREnabled, WMT_TYPE_BOOL, (BYTE*) &fIsVBR, sizeof( BOOL ) );
        if ( FAILED( hr ) )
        {
            //
            // If VBR is requested, then it's a problem, but otherwise the codec may just not support VBR
            //
            if ( ( !fIsVBR ) && ( NS_E_UNSUPPORTED_PROPERTY == hr ) )
            {
                hr = S_OK;
            }
            else
            {
                break;
            }
        }

        if ( fIsVBR )
        {
            hr = pCodecInfo3->SetCodecEnumerationSetting( guidCodecType, dwCodecIndex, g_wszNumPasses, WMT_TYPE_DWORD, (BYTE*) &dwVBRPasses, sizeof( DWORD ) );
            if ( FAILED( hr ) )
            {
                break;
            }
        }

    }
    while ( FALSE );

    return hr;
}


//------------------------------------------------------------------------------
// Name: SetStreamLanguage()
// Desc: Sets the language in the stream configuration.
//------------------------------------------------------------------------------
STDMETHODIMP SetStreamLanguage( IWMStreamConfig * pStreamConfig, LCID dwLanguage )
{
    HRESULT hr = S_OK;
    IWMStreamConfig3 * pStreamConfig3 = NULL;
    IMultiLanguage * pMLang = NULL;
    BSTR bstrLanguage = NULL;

    do
    {
        hr = CoCreateInstance( CLSID_CMultiLanguage,
                               NULL,
                               CLSCTX_ALL,
                               IID_IMultiLanguage,
                               (VOID **) &pMLang );

        if( FAILED( hr ) )
        {
            break;
        }

        hr = pMLang->GetRfc1766FromLcid( dwLanguage, &bstrLanguage );
        if( FAILED( hr ) )
        {
            break;
        }

        hr = pStreamConfig->QueryInterface(	IID_IWMStreamConfig3, (void**)&pStreamConfig3 );
    	if(	FAILED(	hr ) )
    	{
    		break;
    	}
    
    	hr = pStreamConfig3->SetLanguage( bstrLanguage );
    	if(	FAILED(	hr ) )
    	{
    		break;
    	}
    }
    while (FALSE);

    SAFE_RELEASE( pMLang );
    SAFE_RELEASE( pStreamConfig3 );

    if ( !bstrLanguage )
    {
        SysFreeString( bstrLanguage );
    }

    return hr;
}


/*
** Functions that create media types for the various stream types
*/

//------------------------------------------------------------------------------
// Name: CreateUncompressedAudioMediaType()
// Desc: Initializes a WM_MEDIA_TYPE for uncompressed audio.
//------------------------------------------------------------------------------
STDMETHODIMP CreateUncompressedAudioMediaType(  WM_MEDIA_TYPE** ppmtMediaType,
                                    DWORD dwSamplesPerSecond, 
                                    WORD wNumChannels, 
                                    WORD wBitsPerSample )
{
    HRESULT hr = S_OK;

    WM_MEDIA_TYPE mtUncompressedAudio;
    WAVEFORMATEX wfxUncompressedAudio;

    if ( !ppmtMediaType )
    {
        return E_POINTER;
    }
    //
    // pCodecInfo3 is allowed to be NULL, since CreateMediatypeForFormat calls EnsureIWMCodecInfo3
    //
    
    do
    {
        //
        // Setup the local copy of the uncompressed media type
        //
        ZeroMemory( &mtUncompressedAudio, sizeof( mtUncompressedAudio ) );
        
        mtUncompressedAudio.majortype = WMMEDIATYPE_Audio;
        mtUncompressedAudio.subtype = WMMEDIASUBTYPE_PCM;
        mtUncompressedAudio.bFixedSizeSamples = TRUE;
        mtUncompressedAudio.bTemporalCompression = FALSE;
        mtUncompressedAudio.lSampleSize = wNumChannels * wBitsPerSample / 8;
        mtUncompressedAudio.formattype = WMFORMAT_WaveFormatEx;
        mtUncompressedAudio.pUnk = NULL;
        mtUncompressedAudio.cbFormat = sizeof( WAVEFORMATEX );
        mtUncompressedAudio.pbFormat = (BYTE*) &wfxUncompressedAudio;
        
        //
        // Configure the WAVEFORMATEX structure for the uncompressed audio
        //
        ZeroMemory( &wfxUncompressedAudio, sizeof( wfxUncompressedAudio ) );

        wfxUncompressedAudio.wFormatTag = 1;
        wfxUncompressedAudio.nChannels = wNumChannels;
        wfxUncompressedAudio.nSamplesPerSec = dwSamplesPerSecond;
        wfxUncompressedAudio.nAvgBytesPerSec = dwSamplesPerSecond * ( wNumChannels * wBitsPerSample / 8 );
        wfxUncompressedAudio.nBlockAlign = wNumChannels * wBitsPerSample / 8;
        wfxUncompressedAudio.wBitsPerSample = wBitsPerSample;
        wfxUncompressedAudio.cbSize = sizeof( WAVEFORMATEX );
        
        //
        // Return a copy of the media type to the caller, since the mediatype is on the stack
        //
        hr = CopyMediaType( ppmtMediaType, &mtUncompressedAudio );
        if ( FAILED( hr ) )
        {
            break;
        }
    }
    while( FALSE );

    return( hr );
}


//------------------------------------------------------------------------------
// Name: CreateVideoMediaType()
// Desc: Initializes a WM_MEDIA_TYPE for video.
//------------------------------------------------------------------------------
STDMETHODIMP CreateVideoMediaType(  WM_MEDIA_TYPE** ppmtMediaType,
                                    IWMCodecInfo3* pCodecInfo3, 
                                    DWORD dwCodecIndex,
                                    DWORD dwFPS, 
                                    DWORD dwWidth, 
                                    DWORD dwHeight, 
                                    DWORD dwBitrate,
                                    BOOL fIsVBR,
                                    DWORD dwNumberOfPasses )
{
    HRESULT hr = S_OK;

    WM_MEDIA_TYPE *pMediaType = NULL;
    WMVIDEOINFOHEADER *pVIH;

    if ( !ppmtMediaType )
    {
        return E_POINTER;
    }
    //
    // pCodecInfo3 is allowed to be NULL, since CreateMediatypeForFormat calls EnsureIWMCodecInfo3
    //

    do
    {
        //
        // Get the mediatype for the codec
        //
        hr = CreateMediatypeForFormat( &pMediaType,
                               pCodecInfo3,
                               NULL,
                               WMMEDIATYPE_Video, 
                               dwCodecIndex, 
                               0,
                               fIsVBR,
                               dwNumberOfPasses );
        if( FAILED( hr ) )
        {
            break;
        }
        assert( pMediaType );

        //
        // Configure the WMVIDEOINFOHEADER structure of the media type
        //
        pVIH = (WMVIDEOINFOHEADER*) pMediaType->pbFormat;
        pVIH->dwBitRate = dwBitrate;

        pVIH->rcSource.right = dwWidth;
        pVIH->rcSource.bottom = dwHeight;
        pVIH->rcTarget.right = dwWidth;
        pVIH->rcTarget.bottom = dwHeight;
        pVIH->bmiHeader.biWidth = dwWidth;
        pVIH->bmiHeader.biHeight = dwHeight;

        pVIH->AvgTimePerFrame = ( (LONGLONG) 10000000 ) / ( (LONGLONG) dwFPS );

        //
        // Return a copy of the media type to the caller
        //
        hr = CopyMediaType( ppmtMediaType, pMediaType );
        if ( FAILED( hr ) )
        {
            break;
        }
    }
    while( FALSE );

    SAFE_ARRAYDELETE( pMediaType );

    return( hr );
}


//------------------------------------------------------------------------------
// Name: CreateUncompressedVideoMediaType()
// Desc: Initializes a WM_MEDIA_TYPE for uncompressed video.
//------------------------------------------------------------------------------
STDMETHODIMP CreateUncompressedVideoMediaType(  WM_MEDIA_TYPE** ppmtMediaType,
                                                GUID guidFormat,
                                                DWORD dwFourCC,
                                                WORD wBitsPerPixel,
                                                BYTE* pbPaletteData,
                                                DWORD cbPaletteDataSize,
                                                DWORD dwFPS, 
                                                DWORD dwWidth, 
                                                DWORD dwHeight )
{
    const DWORD BITFIELD_DATA_SIZE = sizeof( RGBQUAD ) * 3;

    HRESULT hr;
    WM_MEDIA_TYPE mtUncompressedVideo;
    WMVIDEOINFOHEADER* pvihUncompressedVideo;
    BYTE* pbFormatData = NULL;
    DWORD dwFormatDataSize;
    DWORD dwMaxColors;
    BOOL fPalettePresent;
    BYTE* pbPostVIHData;
    DWORD cbExpectedPostVIHDataSize;
    BOOL fBitfieldsPresent;

    if ( !ppmtMediaType )
    {
        return E_POINTER;
    }
    //
    // pCodecInfo3 is allowed to be NULL, since CreateMediatypeForFormat calls EnsureIWMCodecInfo3
    //
    
    //
    // The width must be on a byte boundry
    //
    if ( dwWidth * wBitsPerPixel % 8 != 0 ) 
    {
        return E_INVALIDARG;
    }
    
    //
    // The width, height, and frames per second must all be non-zero
    //
    if ( 0 == dwWidth || 0 == dwHeight  || 0 == dwFPS )
    {
        return E_INVALIDARG;
    }

    do
    {
        //
        // Allocate space for the format data ( WMVIDEOINFOHEADER + pallete data )
        //
        dwFormatDataSize = sizeof( WMVIDEOINFOHEADER );
        cbExpectedPostVIHDataSize = 0;

        //
        // If there are <= 8 bits / pixel, then there needs to be palette data following the WMVIDEOINFOHEADER
        //
        fPalettePresent = ( wBitsPerPixel <= 8 );
        if ( fPalettePresent ) 
        {
            dwMaxColors = 1 << wBitsPerPixel;
            cbExpectedPostVIHDataSize = sizeof( RGBQUAD ) * dwMaxColors;
        }

        //
        // If the format uses bitfields, then make sure the data is following
        //
        fBitfieldsPresent = ( BI_BITFIELDS == dwFourCC );
        if ( fBitfieldsPresent )
        {
            cbExpectedPostVIHDataSize = BITFIELD_DATA_SIZE;
        }

        if ( fPalettePresent || fBitfieldsPresent )
        {
            dwFormatDataSize += cbExpectedPostVIHDataSize;
            if ( !pbPaletteData || ( cbPaletteDataSize != cbExpectedPostVIHDataSize ) )
            {
                hr = E_INVALIDARG;
                break;
            }
        }

        pbFormatData = new BYTE[ dwFormatDataSize ];
        if ( !pbFormatData )
        {
            hr = E_OUTOFMEMORY;
            break;
        }
        ZeroMemory( pbFormatData, dwFormatDataSize );

        pbPostVIHData = pbFormatData + sizeof( WMVIDEOINFOHEADER ); 
        pvihUncompressedVideo = (WMVIDEOINFOHEADER*) pbFormatData;

        //
        // Set up the local copy of the uncompressed media type
        //
        ZeroMemory( &mtUncompressedVideo, sizeof( mtUncompressedVideo ) );
        
        mtUncompressedVideo.majortype = WMMEDIATYPE_Video;
        mtUncompressedVideo.subtype = guidFormat;
        mtUncompressedVideo.bFixedSizeSamples = TRUE;
        mtUncompressedVideo.bTemporalCompression = FALSE;
        mtUncompressedVideo.lSampleSize = wBitsPerPixel * dwWidth * dwHeight / 8;
        mtUncompressedVideo.formattype = WMFORMAT_VideoInfo;
        mtUncompressedVideo.pUnk = NULL;
        mtUncompressedVideo.cbFormat = dwFormatDataSize;
        mtUncompressedVideo.pbFormat = (BYTE*) pbFormatData;
        
        //
        // Configure the WMVIDEOINFOHEADER structure for uncompressed video
        //
        pvihUncompressedVideo->dwBitRate = mtUncompressedVideo.lSampleSize * dwFPS * 8;

        pvihUncompressedVideo->rcSource.right = dwWidth;
        pvihUncompressedVideo->rcSource.bottom = dwHeight;
        pvihUncompressedVideo->rcTarget.right = dwWidth;
        pvihUncompressedVideo->rcTarget.bottom = dwHeight;


        pvihUncompressedVideo->bmiHeader.biSizeImage = mtUncompressedVideo.lSampleSize;
        pvihUncompressedVideo->bmiHeader.biPlanes = 1;
        pvihUncompressedVideo->bmiHeader.biSize = sizeof( BITMAPINFOHEADER );
        pvihUncompressedVideo->bmiHeader.biWidth = dwWidth;
        pvihUncompressedVideo->bmiHeader.biHeight = dwHeight;
        pvihUncompressedVideo->bmiHeader.biCompression = dwFourCC;
        pvihUncompressedVideo->bmiHeader.biBitCount = wBitsPerPixel;

        pvihUncompressedVideo->AvgTimePerFrame = ( (LONGLONG) 10000000 ) / ( (LONGLONG) dwFPS );

        //
        // Copy the palette information, if present
        //
        if ( ( fPalettePresent || fBitfieldsPresent ) && pbPaletteData )
        {
            memcpy( pbPostVIHData, pbPaletteData, cbExpectedPostVIHDataSize );
        }

        //
        // Return a copy of the media type to the caller, since the media type is on the stack
        //
        hr = CopyMediaType( ppmtMediaType, &mtUncompressedVideo );
        if ( FAILED( hr ) )
        {
            break;
        }
    }
    while( FALSE );

    SAFE_ARRAYDELETE( pbFormatData );

    return( hr );
}


//------------------------------------------------------------------------------
// Name: CreateScriptMediaType()
// Desc: Initializes a WM_MEDIA_TYPE for script.
//------------------------------------------------------------------------------
STDMETHODIMP CreateScriptMediaType(  WM_MEDIA_TYPE** ppmtMediaType )
{
    HRESULT hr = S_OK;
    WM_MEDIA_TYPE wmtMediaType;
    WMSCRIPTFORMAT wsfScriptFormat;

    assert( ppmtMediaType );

    do
    {
        ZeroMemory( &wmtMediaType, sizeof( wmtMediaType ) );

        //
        // Configure media type
        //
		wmtMediaType.majortype              = WMMEDIATYPE_Script;
		wmtMediaType.subtype                = GUID_NULL;
		wmtMediaType.bFixedSizeSamples      = FALSE;
		wmtMediaType.bTemporalCompression   = TRUE;
		wmtMediaType.lSampleSize            = 0;
        wmtMediaType.formattype             = WMFORMAT_Script;
		wmtMediaType.cbFormat               = sizeof( WMSCRIPTFORMAT );
		wmtMediaType.pbFormat               = (BYTE*) &wsfScriptFormat;

        wsfScriptFormat.scriptType          = WMSCRIPTTYPE_TwoStrings;

        //
        // Return a copy of the media type to the caller
        //
        hr = CopyMediaType( ppmtMediaType, &wmtMediaType );
        if ( FAILED( hr ) )
        {
            break;
        }
    }
    while ( FALSE );

    return hr;
}


//------------------------------------------------------------------------------
// Name: CreateImageMediaType()
// Desc: Initializes a WM_MEDIA_TYPE for image.
//------------------------------------------------------------------------------
STDMETHODIMP CreateImageMediaType(  WM_MEDIA_TYPE** ppmtMediaType,
                                    DWORD dwWidth, 
                                    DWORD dwHeight, 
                                    DWORD dwBitrate )
{
    static const WORD BIT_COUNT = 24;

    HRESULT hr = S_OK;
    WM_MEDIA_TYPE wmtMediaType;
    WMVIDEOINFOHEADER vihVideoInfo;

    do
    {
        ZeroMemory( &wmtMediaType, sizeof( wmtMediaType ) );

        //
        // Set up the WM_MEDIA_TYPE structure
        //
		wmtMediaType.majortype              = WMMEDIATYPE_Image;
		wmtMediaType.subtype                = WMMEDIASUBTYPE_RGB24;
		wmtMediaType.bFixedSizeSamples      = FALSE;
		wmtMediaType.bTemporalCompression   = FALSE;
		wmtMediaType.lSampleSize			= 0;
		wmtMediaType.bFixedSizeSamples      = FALSE;
		wmtMediaType.bTemporalCompression   = FALSE;
		wmtMediaType.lSampleSize            = 0;

		wmtMediaType.formattype             = WMFORMAT_VideoInfo;
		wmtMediaType.pUnk                   = NULL;
		wmtMediaType.cbFormat               = sizeof( WMVIDEOINFOHEADER );
		wmtMediaType.pbFormat               = (BYTE*) &vihVideoInfo;

        //
        // Set up the WMVIDEOINFOHEADER structure
        //
		ZeroMemory( &vihVideoInfo, sizeof( vihVideoInfo ) );

		vihVideoInfo.rcSource.left = 0;
		vihVideoInfo.rcSource.top = 0;
		vihVideoInfo.rcSource.bottom = dwHeight;
		vihVideoInfo.rcSource.right = dwWidth;
		vihVideoInfo.rcTarget = vihVideoInfo.rcSource;

        vihVideoInfo.dwBitRate = dwBitrate;
		vihVideoInfo.dwBitErrorRate = 0;
		vihVideoInfo.AvgTimePerFrame = 0;

		vihVideoInfo.bmiHeader.biSize = sizeof( BITMAPINFOHEADER );
		vihVideoInfo.bmiHeader.biWidth = dwWidth;
		vihVideoInfo.bmiHeader.biHeight = dwHeight;
		vihVideoInfo.bmiHeader.biPlanes = 1;
		vihVideoInfo.bmiHeader.biBitCount = BIT_COUNT;
		vihVideoInfo.bmiHeader.biCompression = BI_RGB;
		vihVideoInfo.bmiHeader.biSizeImage = ( dwHeight * dwWidth * vihVideoInfo.bmiHeader.biBitCount ) / 8;
		vihVideoInfo.bmiHeader.biXPelsPerMeter = 0;
		vihVideoInfo.bmiHeader.biYPelsPerMeter = 0;
		vihVideoInfo.bmiHeader.biClrUsed = 0;
		vihVideoInfo.bmiHeader.biClrImportant = 0;

        //
        // Return a copy of the media type to the caller
        //
        hr = CopyMediaType( ppmtMediaType, &wmtMediaType );
        if ( FAILED( hr ) )
        {
            break;
        }
    }
    while ( FALSE );

    return hr;
}


//------------------------------------------------------------------------------
// Name: CreateWebMediaType()
// Desc: Initializes a WM_MEDIA_TYPE for Web media.
//------------------------------------------------------------------------------
STDMETHODIMP CreateWebMediaType(  WM_MEDIA_TYPE** ppmtMediaType )
{
    HRESULT hr = S_OK;
    WM_MEDIA_TYPE wmtMediaType;
    WMT_WEBSTREAM_FORMAT wwfWebFormat;

    do
    {
        ZeroMemory( &wmtMediaType, sizeof( wmtMediaType ) );

        //
        // Configure media type
        //
		wmtMediaType.majortype              = WMMEDIATYPE_FileTransfer;
		wmtMediaType.subtype                = WMMEDIASUBTYPE_WebStream;
		wmtMediaType.bFixedSizeSamples      = FALSE;
		wmtMediaType.bTemporalCompression   = TRUE;
		wmtMediaType.lSampleSize            = 0;
		wmtMediaType.formattype             = WMFORMAT_WebStream;
		wmtMediaType.pUnk                   = NULL;
		wmtMediaType.cbFormat               = sizeof( WMT_WEBSTREAM_FORMAT );
		wmtMediaType.pbFormat               = (BYTE*) &wwfWebFormat;

		ZeroMemory( &wwfWebFormat, sizeof( wwfWebFormat ) );
        wwfWebFormat.cbSize = sizeof( WMT_WEBSTREAM_FORMAT );
        wwfWebFormat.cbSampleHeaderFixedData = sizeof( WMT_WEBSTREAM_SAMPLE_HEADER );
        wwfWebFormat.wVersion = 1;
        wwfWebFormat.wReserved = 0;

        //
        // Return a copy of the media type to the caller
        //
        hr = CopyMediaType( ppmtMediaType, &wmtMediaType );
        if ( FAILED( hr ) )
        {
            break;
        }
    }
    while ( FALSE );

    return hr;
}


//------------------------------------------------------------------------------
// Name: CreateFileMediaType()
// Desc: Initializes a WM_MEDIA_TYPE for file transfer.
//------------------------------------------------------------------------------
STDMETHODIMP CreateFileMediaType(  WM_MEDIA_TYPE** ppmtMediaType )
{
    HRESULT hr = S_OK;
    WM_MEDIA_TYPE wmtMediaType;

    do
    {
        ZeroMemory( &wmtMediaType, sizeof( wmtMediaType ) );

        //
        // Configure media type
        //
		wmtMediaType.majortype              = WMMEDIATYPE_FileTransfer;
		wmtMediaType.subtype                = GUID_NULL;
		wmtMediaType.bFixedSizeSamples      = FALSE;
		wmtMediaType.bTemporalCompression   = FALSE;
		wmtMediaType.lSampleSize            = 0;

        //
        // Return a copy of the media type to the caller
        //
        hr = CopyMediaType( ppmtMediaType, &wmtMediaType );
        if ( FAILED( hr ) )
        {
            break;
        }
    }
    while ( FALSE );

    return hr;
}


/*
** Functions that create various stream types
*/

//------------------------------------------------------------------------------
// Name: CreateAudioStream()
// Desc: Creates an audio stream and returns its configuration object.
//------------------------------------------------------------------------------
STDMETHODIMP CreateAudioStream( IWMStreamConfig** ppStreamConfig, 
                                IWMCodecInfo3* pCodecInfo3, 
                                IWMProfile *pProfile,
                                DWORD dwBufferWindow,
                                DWORD dwCodecIndex,
                                DWORD dwFormatIndex,
                                BOOL fIsVBR,
                                DWORD dwNumberOfPasses,
                                LCID dwLanguage  )
{
    HRESULT hr = S_OK;
    IWMStreamConfig* pStreamConfig = NULL;
    IWMStreamConfig* pFormatConfig = NULL;
    IWMPropertyVault* pPropertyVault = NULL;
    WM_MEDIA_TYPE* pMediaType = NULL;
    IWMMediaProps* pMediaProps = NULL;
    DWORD dwBitrate;

    if ( !ppStreamConfig )
    {
        return E_POINTER;
    }
    if ( !pProfile )
    {
        return E_INVALIDARG;
    }
    
    //
    // pCodecInfo3 is allowed to be NULL, since CreateMediatypeForFormat calls EnsureIWMCodecInfo3
    //

    do
    {
        //
        // Create the audio stream
        //
        hr = pProfile->CreateNewStream( WMMEDIATYPE_Audio, &pStreamConfig );
        if ( FAILED( hr ) )
        {
            break;
        }
        assert( pStreamConfig );

        //
        // Create the media type and get the format's stream configuration
        //
        hr = CreateMediatypeForFormat( &pMediaType,
                                       pCodecInfo3, 
                                       &pFormatConfig,
                                       WMMEDIATYPE_Audio, 
                                       dwCodecIndex, 
                                       dwFormatIndex, 
                                       fIsVBR, 
                                       dwNumberOfPasses );
        if ( FAILED( hr ) )
        {
            break;
        }
        assert( pMediaType );

        //
        // Configure the new audio stream
        //
        hr = pFormatConfig->GetBitrate( &dwBitrate );
        if ( FAILED( hr ) )
        {
            break;
        }

        hr = pStreamConfig->SetBitrate( dwBitrate );
        if ( FAILED( hr ) )
        {
            break;
        }

        hr = pStreamConfig->SetBufferWindow( dwBufferWindow );
        if ( FAILED( hr ) )
        {
            break;
        }

        //
        // Set the media type on the stream
        //
        hr = pStreamConfig->QueryInterface( IID_IWMMediaProps, (void**) &pMediaProps );
        if ( FAILED( hr ) )
        {
            break;
        }
        assert( pMediaProps );

        hr = pMediaProps->SetMediaType( pMediaType );
        if ( FAILED( hr ) )
        {
            break;
        }

        //
        // If this profile will be used by the writer, we should set
        // the VBREnabled attribute properly
        //
        if ( SUCCEEDED( pStreamConfig->QueryInterface( IID_IWMPropertyVault, (void**)&pPropertyVault ) ) )
        {
            assert( pPropertyVault );
            pPropertyVault->SetProperty( g_wszVBREnabled, WMT_TYPE_BOOL, (BYTE*)&fIsVBR, sizeof( fIsVBR ) );

            SAFE_RELEASE( pPropertyVault );
        }

        hr = SetStreamLanguage( pStreamConfig, dwLanguage );
        if ( FAILED( hr ) )
        {
            break;
        }

        //
        // Return the pStreamConfig to the caller
        //
        SAFE_ADDREF( pStreamConfig );
        *ppStreamConfig = pStreamConfig;
    }
    while ( FALSE );

    SAFE_RELEASE( pStreamConfig );
    SAFE_RELEASE( pFormatConfig );
    SAFE_RELEASE( pPropertyVault );
    SAFE_ARRAYDELETE( pMediaType );
    SAFE_RELEASE( pMediaProps );

    return hr;
}


//------------------------------------------------------------------------------
// Name: CreateAudioStream()
// Desc: Creates an audio stream and returns its configuration object.
//------------------------------------------------------------------------------
STDMETHODIMP CreateUncompressedAudioStream( IWMStreamConfig** ppStreamConfig, 
                                IWMProfile *pProfile,
                                DWORD dwSamplesPerSecond, 
                                WORD wNumChannels, 
                                WORD wBitsPerSample,
                                LCID dwLanguage  )
{
    HRESULT hr = S_OK;
    IWMStreamConfig* pStreamConfig = NULL;
    IWMPropertyVault* pPropertyVault = NULL;
    WM_MEDIA_TYPE* pMediaType = NULL;
    IWMMediaProps* pMediaProps = NULL;
    BOOL fFalse = FALSE;

    if ( !ppStreamConfig )
    {
        return E_POINTER;
    }
    if ( !pProfile )
    {
        return E_INVALIDARG;
    }
    
    //
    // pCodecInfo3 is allowed to be NULL, since CreateMediatypeForFormat calls EnsureIWMCodecInfo3
    //

    do
    {
        //
        // Create the audio stream
        //
        hr = pProfile->CreateNewStream( WMMEDIATYPE_Audio, &pStreamConfig );
        if ( FAILED( hr ) )
        {
            break;
        }
        assert( pStreamConfig );

        //
        // Create the media type for the uncompressed audio
        //
        hr = CreateUncompressedAudioMediaType( &pMediaType,
                                                dwSamplesPerSecond, 
                                                wNumChannels, 
                                                wBitsPerSample );
        if ( FAILED( hr ) )
        {
            break;
        }
        assert( pMediaType );

        //
        // Configure the new audio stream
        //
        hr = pStreamConfig->SetBitrate( dwSamplesPerSecond * wNumChannels * wBitsPerSample );
        if ( FAILED( hr ) )
        {
            break;
        }

        hr = pStreamConfig->SetBufferWindow( 0 );
        if ( FAILED( hr ) )
        {
            break;
        }

        //
        // Set the media type on the stream
        //
        hr = pStreamConfig->QueryInterface( IID_IWMMediaProps, (void**) &pMediaProps );
        if ( FAILED( hr ) )
        {
            break;
        }
        assert( pMediaProps );

        hr = pMediaProps->SetMediaType( pMediaType );
        if ( FAILED( hr ) )
        {
            break;
        }

        //
        // If this profile will be used by the writer, we should set
        // VBREnabled attribute properly
        //
        if ( SUCCEEDED( pStreamConfig->QueryInterface( IID_IWMPropertyVault, (void**)&pPropertyVault ) ) )
        {
            assert( pPropertyVault );
            pPropertyVault->SetProperty( g_wszVBREnabled, WMT_TYPE_BOOL, (BYTE*)&fFalse, sizeof( fFalse ) );

            SAFE_RELEASE( pPropertyVault );
        }

        hr = SetStreamLanguage( pStreamConfig, dwLanguage );
        if ( FAILED( hr ) )
        {
            break;
        }

        //
        // Return the pStreamConfig to the caller
        //
        SAFE_ADDREF( pStreamConfig );
        *ppStreamConfig = pStreamConfig;
    }
    while ( FALSE );

    SAFE_RELEASE( pStreamConfig );
    SAFE_RELEASE( pPropertyVault );
    SAFE_ARRAYDELETE( pMediaType );
    SAFE_RELEASE( pMediaProps );

    return hr;
}


//------------------------------------------------------------------------------
// Name: CreateVideoStream()
// Desc: Creates a video stream and returns its configuration object.
//------------------------------------------------------------------------------
STDMETHODIMP CreateVideoStream( IWMStreamConfig** ppStreamConfig,
                                IWMCodecInfo3* pCodecInfo3,
                                IWMProfile *pProfile,
                                DWORD dwCodecIndex,
                                DWORD dwBitrate,
                                DWORD dwBufferWindow,
                                DWORD dwWidth,
                                DWORD dwHeight,
                                DWORD dwFPS,
                                DWORD dwQuality,
                                DWORD dwSecPerKey,
                                BOOL fIsVBR,
                                VIDEO_VBR_MODE vbrMode,
                                DWORD dwVBRQuality,
                                DWORD dwMaxBitrate,
                                DWORD dwMaxBufferWindow,
                                LCID dwLanguage  )
{
    HRESULT hr = S_OK;
    IWMStreamConfig* pStreamConfig = NULL;
    IWMPropertyVault* pPropertyVault = NULL;
    WM_MEDIA_TYPE* pMediaType = NULL;
    IWMVideoMediaProps* pVideoMediaProps = NULL;
    DWORD dwNumberOfPasses;

    if ( !ppStreamConfig )
    {
        return E_POINTER;
    }
    if ( !pProfile )
    {
        return E_INVALIDARG;
    }
    //
    // pCodecInfo3 is allowed to be NULL, since CreateVideoMediatype calls EnsureIWMCodecInfo3
    //

    do
    {
        switch( vbrMode )
        {
		case VBR_OFF:
            dwNumberOfPasses = 0;
			break;

        case VBR_QUALITYBASED:
            dwNumberOfPasses = 1;
            break;
   
        case VBR_CONSTRAINED:
            dwNumberOfPasses = 2;
            break;
    
        case VBR_UNCONSTRAINED:
            dwNumberOfPasses = 2;
            break;

        default:
            hr = E_FAIL;
            break;
        }
        if ( FAILED( hr ) )
        {
            break;
        }

        //
        // Create the video stream
        //
        hr = pProfile->CreateNewStream( WMMEDIATYPE_Video, &pStreamConfig );
        if ( FAILED( hr ) )
        {
            break;
        }
        assert( pStreamConfig );

        //
        // Configure the new video stream
        //
        hr = pStreamConfig->SetBitrate( dwBitrate );
        if ( FAILED( hr ) )
        {
            break;
        }

        hr = pStreamConfig->SetBufferWindow( dwBufferWindow );
        if ( FAILED( hr ) )
        {
            break;
        }

        //
        // Set the media type for the stream
        //
        hr = CreateVideoMediaType(  &pMediaType,
                                    pCodecInfo3, 
                                    dwCodecIndex, 
                                    dwFPS, 
                                    dwWidth, 
                                    dwHeight, 
									dwBitrate,
                                    fIsVBR,
                                    dwNumberOfPasses );
        if ( FAILED( hr ) )
        {
            break;
        }
        assert( pMediaType );

        hr = pStreamConfig->QueryInterface( IID_IWMVideoMediaProps, (void**) &pVideoMediaProps );
        if ( FAILED( hr ) )
        {
            break;
        }
        assert( pVideoMediaProps );

        hr = pVideoMediaProps->SetMediaType( pMediaType );
        if ( FAILED( hr ) )
        {
            break;
        }

        //
        // Set quality and MaxKeyFrameSpacing on the IWMVideoMediaProps object
        //
        hr = pVideoMediaProps->SetQuality( dwQuality );
        if ( FAILED( hr ) )
        {
            break;
        }

        hr = pVideoMediaProps->SetMaxKeyFrameSpacing( (LONGLONG) dwSecPerKey * 10000000 );
        if ( FAILED( hr ) )
        {
            break;
        }

        //
        // If this profile will be used by writer, we should set the properties
        // on the stream configuration
        //
        if ( SUCCEEDED( pStreamConfig->QueryInterface( IID_IWMPropertyVault, (void**)&pPropertyVault ) ) )
        {
            assert( pPropertyVault );
            hr = pPropertyVault->SetProperty( g_wszVBREnabled, WMT_TYPE_BOOL, (BYTE*) &fIsVBR, sizeof( fIsVBR ) );
            if ( FAILED( hr ) )
            {
                break;
            }
            
            switch( vbrMode )
            {
            case VBR_QUALITYBASED:
                //
                // Only the quality needs to be set
                //
                hr = pPropertyVault->SetProperty( g_wszVBRQuality, WMT_TYPE_DWORD, (BYTE*) &dwVBRQuality, sizeof( DWORD ) );
                break;
       
            case VBR_CONSTRAINED:
                //
                // The peak bitrate and, optionally, max buffer window need to be set
                // 
                hr = pPropertyVault->SetProperty( g_wszVBRBitrateMax, WMT_TYPE_DWORD, (BYTE*) &dwMaxBitrate, sizeof( DWORD ) );
                if ( FAILED( hr ) )
                {
                    break;
                }
                
                if( dwMaxBufferWindow != 0 )
	            {
                    hr = pPropertyVault->SetProperty( g_wszVBRBufferWindowMax, WMT_TYPE_DWORD, (BYTE*) &dwMaxBufferWindow, sizeof( DWORD ) );
                }
                break;
        
            case VBR_UNCONSTRAINED:
                break;

			case VBR_OFF:
				break;

            default:
                hr = E_FAIL;
                break;
            }
            if ( FAILED( hr ) )
            {
                break;
            }

            SAFE_RELEASE( pPropertyVault );
        }

        hr = SetStreamLanguage( pStreamConfig, dwLanguage );
        if ( FAILED( hr ) )
        {
            break;
        }

        //
        // Return the pStreamConfig to the caller
        //
        SAFE_ADDREF( pStreamConfig );
        *ppStreamConfig = pStreamConfig;
    }
    while ( FALSE );

    SAFE_RELEASE( pStreamConfig );
    SAFE_RELEASE( pPropertyVault );
    SAFE_ARRAYDELETE( pMediaType );
    SAFE_RELEASE( pVideoMediaProps );

    return hr;
}


//------------------------------------------------------------------------------
// Name: CreateUncompressedVideoStream()
// Desc: Creates an uncompressed video stream and returns its configuration object.
//------------------------------------------------------------------------------
STDMETHODIMP CreateUncompressedVideoStream( IWMStreamConfig** ppStreamConfig,
                                IWMProfile *pProfile,
                                GUID guidFormat,
                                DWORD dwFourCC,
                                WORD wBitsPerPixel,
                                BYTE* pbPaletteData,
                                DWORD cbPaletteDataSize,
                                DWORD dwWidth,
                                DWORD dwHeight,
                                DWORD dwFPS,
                                LCID dwLanguage )
{
    HRESULT hr = S_OK;
    IWMStreamConfig* pStreamConfig = NULL;
    IWMPropertyVault* pPropertyVault = NULL;
    WM_MEDIA_TYPE* pMediaType = NULL;
    IWMVideoMediaProps* pVideoMediaProps = NULL;
    BOOL fFalse = FALSE;

    if ( !ppStreamConfig )
    {
        return E_POINTER;
    }
    if ( !pProfile )
    {
        return E_INVALIDARG;
    }
    //
    // pCodecInfo3 is allowed to be NULL, since CreateVideoMediatype calls EnsureIWMCodecInfo3
    //

    do
    {
        //
        // Create the video stream
        //
        hr = pProfile->CreateNewStream( WMMEDIATYPE_Video, &pStreamConfig );
        if ( FAILED( hr ) )
        {
            break;
        }
        assert( pStreamConfig );

        //
        // Configure the new video stream
        //
        hr = pStreamConfig->SetBitrate( wBitsPerPixel * dwWidth * dwHeight * dwFPS );
        if ( FAILED( hr ) )
        {
            break;
        }

        hr = pStreamConfig->SetBufferWindow( 0 );
        if ( FAILED( hr ) )
        {
            break;
        }

        //
        // Set the media type for the stream
        //
        hr = CreateUncompressedVideoMediaType( &pMediaType,
                                    guidFormat,
                                    dwFourCC,
                                    wBitsPerPixel,
                                    pbPaletteData,
                                    cbPaletteDataSize,
                                    dwFPS, 
                                    dwWidth, 
                                    dwHeight );
        if ( FAILED( hr ) )
        {
            break;
        }
        assert( pMediaType );

        hr = pStreamConfig->QueryInterface( IID_IWMVideoMediaProps, (void**) &pVideoMediaProps );
        if ( FAILED( hr ) )
        {
            break;
        }
        assert( pVideoMediaProps );

        hr = pVideoMediaProps->SetMediaType( pMediaType );
        if ( FAILED( hr ) )
        {
            break;
        }

        //
        // If this profile will be used by writer, we should set the properties
        // on the stream configuration
        //
        if ( SUCCEEDED( pStreamConfig->QueryInterface( IID_IWMPropertyVault, (void**)&pPropertyVault ) ) )
        {
            assert( pPropertyVault );
            hr = pPropertyVault->SetProperty( g_wszVBREnabled, WMT_TYPE_BOOL, (BYTE*) &fFalse, sizeof( fFalse ) );
            if ( FAILED( hr ) )
            {
                break;
            }
            
            SAFE_RELEASE( pPropertyVault );
        }

        hr = SetStreamLanguage( pStreamConfig, dwLanguage );
        if ( FAILED( hr ) )
        {
            break;
        }

        //
        // Return the pStreamConfig to the caller
        //
        SAFE_ADDREF( pStreamConfig );
        *ppStreamConfig = pStreamConfig;
    }
    while ( FALSE );

    SAFE_RELEASE( pStreamConfig );
    SAFE_RELEASE( pPropertyVault );
    SAFE_ARRAYDELETE( pMediaType );
    SAFE_RELEASE( pVideoMediaProps );

    return hr;
}


//------------------------------------------------------------------------------
// Name: CreateScriptStream()
// Desc: Creates a scripat stream and returns its configuration object.
//------------------------------------------------------------------------------
STDMETHODIMP CreateScriptStream( IWMStreamConfig** ppStreamConfig, 
                                 IWMProfile *pProfile, 
                                 DWORD dwBitrate, 
                                 DWORD dwBufferWindow,
                                 LCID dwLanguage  )
{
    HRESULT hr = S_OK;
    IWMStreamConfig* pStreamConfig = NULL;
    WM_MEDIA_TYPE* pMediaType = NULL;
    IWMMediaProps* pMediaProps = NULL;

    if ( !ppStreamConfig )
    {
        return E_POINTER;
    }
    if ( !pProfile )
    {
        return E_INVALIDARG;
    }
    

    do
    {
        //
        // Create the script stream
        //
        hr = pProfile->CreateNewStream( WMMEDIATYPE_Script, &pStreamConfig );
        if ( FAILED( hr ) )
        {
            break;
        }
        assert( pStreamConfig );

        //
        // Configure the new stream
        //
        hr = pStreamConfig->SetBitrate( dwBitrate );
        if ( FAILED( hr ) )
        {
            break;
        }

        hr = pStreamConfig->SetBufferWindow( dwBufferWindow );
        if ( FAILED( hr ) )
        {
            break;
        }

        //
        // Set the media type for the script stream
        //
        hr = CreateScriptMediaType( &pMediaType );
        if ( FAILED( hr ) )
        {
            break;
        }

        hr = pStreamConfig->QueryInterface( IID_IWMMediaProps, (void**) &pMediaProps );
        if ( FAILED( hr ) )
        {
            break;
        }
        assert( pMediaProps );

        hr = pMediaProps->SetMediaType( pMediaType );
        if ( FAILED( hr ) )
        {
            break;
        }

        hr = SetStreamLanguage( pStreamConfig, dwLanguage );
        if ( FAILED( hr ) )
        {
            break;
        }

        //
        // Return the pStreamConfig to the caller
        //
        SAFE_ADDREF( pStreamConfig );
        *ppStreamConfig = pStreamConfig;
    }
    while ( FALSE );

    SAFE_RELEASE( pStreamConfig );
    SAFE_ARRAYDELETE( pMediaType );
    SAFE_RELEASE( pMediaProps );

    return hr;
}


//------------------------------------------------------------------------------
// Name: CreateImageStream()
// Desc: Creates an image stream and returns its configuration object.
//------------------------------------------------------------------------------
STDMETHODIMP CreateImageStream( IWMStreamConfig** ppStreamConfig,
                                IWMProfile *pProfile,
                                DWORD dwBitrate, 
                                DWORD dwBufferWindow, 
                                DWORD dwWidth,
                                DWORD dwHeight,
                                LCID dwLanguage  )
{
    HRESULT hr = S_OK;
    IWMStreamConfig* pStreamConfig = NULL;
    WM_MEDIA_TYPE* pMediaType = NULL;
    IWMMediaProps* pMediaProps = NULL;

    if ( !ppStreamConfig )
    {
        return E_POINTER;
    }
    if ( !pProfile )
    {
        return E_INVALIDARG;
    }
    
    do
    {
        //
        // Create the image stream
        //
        hr = pProfile->CreateNewStream( WMMEDIATYPE_Image, &pStreamConfig );
        if ( FAILED( hr ) )
        {
            break;
        }
        assert( pStreamConfig );

        //
        // Configure the new stream
        //
        hr = pStreamConfig->SetBitrate( dwBitrate );
        if ( FAILED( hr ) )
        {
            break;
        }

        hr = pStreamConfig->SetBufferWindow( dwBufferWindow );
        if ( FAILED( hr ) )
        {
            break;
        }

        //
        // Set the media type for the image stream
        //
        hr = CreateImageMediaType(  &pMediaType,
                                    dwBitrate, 
                                    dwWidth, 
                                    dwHeight );
        if ( FAILED( hr ) )
        {
            break;
        }

        hr = pStreamConfig->QueryInterface( IID_IWMMediaProps, (void**) &pMediaProps );
        if ( FAILED( hr ) )
        {
            break;
        }
        assert( pMediaProps );

        hr = pMediaProps->SetMediaType( pMediaType );
        if ( FAILED( hr ) )
        {
            break;
        }

        hr = SetStreamLanguage( pStreamConfig, dwLanguage );
        if ( FAILED( hr ) )
        {
            break;
        }

        //
        // Return the pStreamConfig to the caller
        //
        SAFE_ADDREF( pStreamConfig );
        *ppStreamConfig = pStreamConfig;
    }
    while ( FALSE );

    SAFE_RELEASE( pStreamConfig );
    SAFE_ARRAYDELETE( pMediaType );
    SAFE_RELEASE( pMediaProps );

    return hr;
}


//------------------------------------------------------------------------------
// Name: CreateWebStream()
// Desc: Creates a Web stream and returns its configuration object.
//------------------------------------------------------------------------------
STDMETHODIMP CreateWebStream( IWMStreamConfig** ppStreamConfig, 
                              IWMProfile *pProfile, 
                              DWORD dwBitrate, 
                              DWORD dwBufferWindow,
                              LCID dwLanguage  )
{
    HRESULT hr = S_OK;
    IWMStreamConfig* pStreamConfig = NULL;
    WM_MEDIA_TYPE* pMediaType = NULL;
    IWMMediaProps* pMediaProps = NULL;

    if ( !ppStreamConfig )
    {
        return E_POINTER;
    }
    if ( !pProfile )
    {
        return E_INVALIDARG;
    }
    
    do
    {
        //
        // Create the Web stream
        //
        hr = pProfile->CreateNewStream( WMMEDIATYPE_FileTransfer, &pStreamConfig );
        if ( FAILED( hr ) )
        {
            break;
        }
        assert( pStreamConfig );

        //
        // Configure the new stream
        //
        hr = pStreamConfig->SetBitrate( dwBitrate );
        if ( FAILED( hr ) )
        {
            break;
        }

        hr = pStreamConfig->SetBufferWindow( dwBufferWindow );
        if ( FAILED( hr ) )
        {
            break;
        }

        //
        // Set the media type for the stream
        //
        hr = CreateWebMediaType( &pMediaType );
        if ( FAILED( hr ) )
        {
            break;
        }

        hr = pStreamConfig->QueryInterface( IID_IWMMediaProps, (void**) &pMediaProps );
        if ( FAILED( hr ) )
        {
            break;
        }
        assert( pMediaProps );

        hr = pMediaProps->SetMediaType( pMediaType );
        if ( FAILED( hr ) )
        {
            break;
        }

        hr = SetStreamLanguage( pStreamConfig, dwLanguage );
        if ( FAILED( hr ) )
        {
            break;
        }

        //
        // Return the pStreamConfig to the caller
        //
        SAFE_ADDREF( pStreamConfig );
        *ppStreamConfig = pStreamConfig;
    }
    while ( FALSE );

    SAFE_RELEASE( pStreamConfig );
    SAFE_ARRAYDELETE( pMediaType );
    SAFE_RELEASE( pMediaProps );

    return hr;
}


//------------------------------------------------------------------------------
// Name: CreateFileStream()
// Desc: Creates a file stream and returns its configuration object.
//------------------------------------------------------------------------------
STDMETHODIMP CreateFileStream( IWMStreamConfig** ppStreamConfig, 
                               IWMProfile *pProfile, 
                               DWORD dwBufferWindow, 
                               DWORD dwBitrate, 
                               WORD wMaxFilenameLength,
                               LCID dwLanguage  )
{
    HRESULT hr = S_OK;
    IWMStreamConfig* pStreamConfig = NULL;
    IWMStreamConfig2* pStreamConfig2 = NULL;
    WM_MEDIA_TYPE* pMediaType = NULL;
    IWMMediaProps* pMediaProps = NULL;

    if ( !ppStreamConfig )
    {
        return E_POINTER;
    }
    if ( !pProfile )
    {
        return E_INVALIDARG;
    }
    

    do
    {
        //
        // Create the file stream
        //
        hr = pProfile->CreateNewStream( WMMEDIATYPE_FileTransfer, &pStreamConfig );
        if ( FAILED( hr ) )
        {
            break;
        }
        assert( pStreamConfig );

        //
        // Configure the new stream
        //
        hr = pStreamConfig->SetBitrate( dwBitrate );
        if ( FAILED( hr ) )
        {
            break;
        }

        hr = pStreamConfig->SetBufferWindow( dwBufferWindow );
        if ( FAILED( hr ) )
        {
            break;
        }

        //
        // Set the media type for the stream
        //
        hr = CreateFileMediaType( &pMediaType );
        if ( FAILED( hr ) )
        {
            break;
        }

        hr = pStreamConfig->QueryInterface( IID_IWMMediaProps, (void**) &pMediaProps );
        if ( FAILED( hr ) )
        {
            break;
        }
        assert( pMediaProps );

        hr = pMediaProps->SetMediaType( pMediaType );
        if ( FAILED( hr ) )
        {
            break;
        }

        //
        // Add the filename data unit extension to the stream
        //
        hr = pStreamConfig->QueryInterface( IID_IWMStreamConfig2, (void**) &pStreamConfig2 );
        if ( FAILED( hr ) )
        {
            break;
        }
        assert( pStreamConfig2 );

        hr = pStreamConfig2->AddDataUnitExtension( WM_SampleExtensionGUID_FileName,
                                                   wMaxFilenameLength, 
                                                   NULL,
                                                   0 );
        if ( FAILED( hr ) )
        {
            break;
        }

        hr = SetStreamLanguage( pStreamConfig, dwLanguage );
        if ( FAILED( hr ) )
        {
            break;
        }

        //
        // Return the pStreamConfig to the caller
        //
        SAFE_ADDREF( pStreamConfig );
        *ppStreamConfig = pStreamConfig;
    }
    while ( FALSE );

    SAFE_RELEASE( pStreamConfig );
    SAFE_ARRAYDELETE( pMediaType );
    SAFE_RELEASE( pMediaProps );
    SAFE_RELEASE( pStreamConfig2 );

    return hr;
}


//------------------------------------------------------------------------------
// Name: CreateMediatypeForFormat()
// Desc: Initializes a WM_MEDIA_TYPE for a codec format, setting VBR attributes.
//------------------------------------------------------------------------------
STDMETHODIMP CreateMediatypeForFormat( WM_MEDIA_TYPE** ppmtDestination,
                                       IWMCodecInfo3* pCodecInfo3, 
                                       IWMStreamConfig** ppFormatConfig,
                                       GUID guidCodecType, 
                                       DWORD dwCodecIndex, 
                                       DWORD dwFormatIndex, 
                                       BOOL fIsVBR, 
                                       DWORD dwVBRPasses )
{
    HRESULT hr = S_OK;
    IWMProfileManager* pProfileManager = NULL;
    IWMStreamConfig* pStreamConfig = NULL;
    IWMMediaProps* pMediaProps = NULL;

    if ( !ppmtDestination )
    {
        return E_INVALIDARG;
    }

    do
    {
        *ppmtDestination = NULL;

        //
        // Make sure the pCodecInfo3 exists, and there's an outstanding reference
        //
        hr = EnsureIWMCodecInfo3( &pCodecInfo3 );
        if ( FAILED( hr ) )
        {
            break;
        }
        assert( pCodecInfo3 );

        //
        // Set the VBR settings appropriately
        //
        hr = SetCodecVBRSettings( pCodecInfo3, 
                            guidCodecType, 
                            dwCodecIndex, 
                            fIsVBR, 
                            dwVBRPasses );
        if ( FAILED( hr ) )
        {
            break;
        }

        //
        // Call the version that doesn't set the VBR attributes
        //
        hr = CreateMediatypeForFormat( ppmtDestination,
                                       pCodecInfo3,
                                       ppFormatConfig,
                                       guidCodecType,
                                       dwCodecIndex,
                                       dwFormatIndex );
        if ( FAILED( hr ) )
        {
            break;
        }
        assert( *ppmtDestination );
    }
    while ( FALSE );

    if ( FAILED( hr ) )
    {
        SAFE_ARRAYDELETE( (*ppmtDestination) );
    }

    SAFE_RELEASE( pCodecInfo3 );
    SAFE_RELEASE( pProfileManager );
    SAFE_RELEASE( pStreamConfig );
    SAFE_RELEASE( pMediaProps );

    return hr;
}


//------------------------------------------------------------------------------
// Name: CreateMediatypeForFormat() (Overloaded)
// Desc: Initializes a WM_MEDIA_TYPE for a codec format, without setting 
//       VBR attributes.
//------------------------------------------------------------------------------
STDMETHODIMP CreateMediatypeForFormat( WM_MEDIA_TYPE** ppmtDestination,
                                       IWMCodecInfo3* pCodecInfo3,
                                       IWMStreamConfig** ppFormatConfig,
                                       GUID guidCodecType, 
                                       DWORD dwCodecIndex, 
                                       DWORD dwFormatIndex )
{
    HRESULT hr = S_OK;
    IWMProfileManager* pProfileManager = NULL;
    IWMStreamConfig* pFormatConfig = NULL;
    IWMMediaProps* pMediaProps = NULL;
    DWORD dwMediaTypeLength;

    if ( !ppmtDestination )
    {
        return E_INVALIDARG;
    }

    do
    {
        *ppmtDestination = NULL;

        //
        // Make sure the pCodecInfo3 exists, and there's an outstanding reference
        //
        hr = EnsureIWMCodecInfo3( &pCodecInfo3 );
        if ( FAILED( hr ) )
        {
            break;
        }
        assert( pCodecInfo3 );

        //
        // Get the stream configuration for the given format
        //
        hr = pCodecInfo3->GetCodecFormat( guidCodecType, dwCodecIndex, dwFormatIndex, &pFormatConfig );
        if ( FAILED( hr ) )
        {
            break;
        }
        assert( pFormatConfig );

        //
        // Get the media type for the requested format
        //
        hr = pFormatConfig->QueryInterface( IID_IWMMediaProps, (void**) &pMediaProps );
        if ( FAILED( hr ) )
        {
            break;
        }
        assert( pMediaProps );

        dwMediaTypeLength = 0;
        hr = pMediaProps->GetMediaType( NULL, &dwMediaTypeLength );
        if( FAILED( hr ) )
        {
            break;
        }

        *ppmtDestination = (WM_MEDIA_TYPE *) new BYTE[ dwMediaTypeLength ];
        if( !*ppmtDestination )
        {
            hr = E_OUTOFMEMORY;
            break;
        }

        hr = pMediaProps->GetMediaType( *ppmtDestination, &dwMediaTypeLength );
        if( FAILED( hr ) )
        {
            break;
        }
        assert( *ppmtDestination );

        if ( ppFormatConfig )
        {
            SAFE_ADDREF( pFormatConfig );
            *ppFormatConfig = pFormatConfig;
        }
    }
    while ( FALSE );

    if ( FAILED( hr ) )
    {
        SAFE_ARRAYDELETE( *ppmtDestination );
    }

    SAFE_RELEASE( pCodecInfo3 );
    SAFE_RELEASE( pProfileManager );
    SAFE_RELEASE( pFormatConfig );
    SAFE_RELEASE( pMediaProps );

    return hr;
}


//------------------------------------------------------------------------------
// Name: WriteProfileAsPRX()
// Desc: Writes a profile to a PRX file.
//------------------------------------------------------------------------------
STDMETHODIMP WriteProfileAsPRX( LPCTSTR tszFilename, LPCWSTR wszProfileData, DWORD dwProfileDataLength )
{
    HANDLE hFile = NULL;
    HRESULT hr = S_OK;
    DWORD dwBytesWritten;
    BOOL bResult;

    assert( tszFilename );
    assert( wszProfileData );
    assert( 0 != dwProfileDataLength );

    do
    {
        //
        // Create the file, overwriting any existing file
        //
        hFile = CreateFile( tszFilename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL );
        if ( INVALID_HANDLE_VALUE == hFile )
        {
            hr = HRESULT_FROM_WIN32( GetLastError() );
            break;
        }

        //
        // Write the profile data to the file
        //
        bResult = WriteFile( hFile, wszProfileData, dwProfileDataLength, &dwBytesWritten, NULL );
        if ( !bResult )
        {
            hr = HRESULT_FROM_WIN32( GetLastError() );
            break;
        }
    }
    while ( FALSE );

    //
    // Close the file, if it was opened successfully
    //
    SAFE_CLOSEHANDLE( hFile );

    return hr;
}


//------------------------------------------------------------------------------
// Name: AddSMPTEExtensionToStream()
// Desc: Add a data unit extension for SMPTE time code.
//------------------------------------------------------------------------------
STDMETHODIMP AddSMPTEExtensionToStream( IWMStreamConfig* pStream )
{
    HRESULT hr = S_OK;
    IWMStreamConfig2* pStreamConfig2 = NULL;

    if ( !pStream )
    {
        return E_INVALIDARG;
    }

    do
    {
        //
        // Get the IWMStreamConfig2 interface
        //
        hr = pStream->QueryInterface( IID_IWMStreamConfig2, (void**) &pStreamConfig2 );
        if ( FAILED( hr ) )
        {
            break;
        }
        assert( pStreamConfig2 );

        //
        // Add SMPTE extension
        //
        hr = pStreamConfig2->AddDataUnitExtension( WM_SampleExtensionGUID_Timecode,
                                                   sizeof( WMT_TIMECODE_EXTENSION_DATA ),
                                                   NULL,
                                                   0 );
        if ( FAILED( hr ) )
        {
            break;
        }
    }
    while ( FALSE );

    SAFE_RELEASE( pStreamConfig2 );

    return hr;
}



//------------------------------------------------------------------------------
// Name: GetUncompressedWaveFormatCount()
// Desc: Returns the number of uncompressed wave formats, four for each
//       frequency in the WaveFrequency array.
//------------------------------------------------------------------------------
STDMETHODIMP GetUncompressedWaveFormatCount( DWORD * pdwCount )
{
    *pdwCount = 4 * sizeof( WaveFrequency ) / sizeof( WaveFrequency[0] );

    return S_OK;
}


//------------------------------------------------------------------------------
// Name: GetUncompressedWaveFormat()
// Desc: Retrieves wave format parameters by index.
//------------------------------------------------------------------------------
STDMETHODIMP GetUncompressedWaveFormat( DWORD dwIndex,                                 
                                         DWORD * pdwSamplesPerSecond, 
                                         WORD * pwNumChannels, 
                                         WORD * pwBitsPerSample )
{
    if ( NULL == pdwSamplesPerSecond 
        || NULL == pwNumChannels
        || NULL == pwBitsPerSample )
    {
        return E_POINTER;
    }

    DWORD dwCount;
    HRESULT hr;
    
    hr = GetUncompressedWaveFormatCount( &dwCount );
    if ( FAILED( hr ) )
    {
        return hr;
    }

    if ( dwIndex >= dwCount )
    {
        return E_INVALIDARG;
    }

    *pdwSamplesPerSecond = WaveFrequency[ dwIndex / 4 ];

    switch ( dwIndex % 4 )
    {
    case 0:
        *pwBitsPerSample = 8;
        *pwNumChannels = 1;
        break;
    case 1:
        *pwBitsPerSample = 8;
        *pwNumChannels = 2;
        break;
    case 2:
        *pwBitsPerSample = 16;
        *pwNumChannels = 1;
        break;
    case 3:
        *pwBitsPerSample = 16;
        *pwNumChannels = 2;
        break;
    }

    return S_OK;
}


//------------------------------------------------------------------------------
// Name: GetUncompressedPixelFormatCount()
// Desc: Returns the number of formats in the PixelFormats array.
//------------------------------------------------------------------------------
STDMETHODIMP GetUncompressedPixelFormatCount( DWORD * pdwCount )
{
    *pdwCount = sizeof( PixelFormats ) / sizeof( PixelFormats[0] );

    return S_OK;
}


//------------------------------------------------------------------------------
// Name: GetUncompressedPixelFormat()
// Desc: Retrieves pixel format parameters by index.
//------------------------------------------------------------------------------
STDMETHODIMP GetUncompressedPixelFormat( DWORD dwIndex,
                                         GUID * pguidFormat,
                                         DWORD * pdwFourCC,
                                         WORD * pwBitsPerPixel )
{
    if ( NULL == pguidFormat 
        || NULL == pdwFourCC
        || NULL == pwBitsPerPixel )
    {
        return E_POINTER;
    }

    DWORD dwCount = sizeof( PixelFormats ) / sizeof( PixelFormats[0] );

    if ( dwIndex >= dwCount )
    {
        return E_INVALIDARG;
    }

    *pguidFormat = *PixelFormats[ dwIndex ].guidFormat;
    *pdwFourCC = PixelFormats[ dwIndex ].dwFourCC;
    *pwBitsPerPixel = PixelFormats[ dwIndex ].wBitsPerPixel;

    return S_OK;
}
