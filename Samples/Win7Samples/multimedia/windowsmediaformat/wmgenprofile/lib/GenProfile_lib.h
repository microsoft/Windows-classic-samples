//*****************************************************************************
//
// Microsoft Windows Media
// Copyright (C) Microsoft Corporation. All rights reserved.
//
// FileName:            GenProfile_lib.h
//
// Abstract:            The header file which contains the structures and
//                      functions used by the library to create a profile
//
//*****************************************************************************


#ifndef __GENPROFILE_H__
#define __GENPROFILE_H__

#include <wmsdk.h>

#define MAX_DATA_UNIT_EXTENSIONS 5 // An arbitrarily chosen maximum for the number of data unit extensions


//
// Enumeration of the different kinds of streams
//
enum StreamType
{
	ST_Unknown,		            // Unknown streamtype
    ST_Audio,                   // Audio stream
    ST_Video,                   // Video stream
    ST_Script,                  // Script stream
    ST_Image,                   // Image stream
    ST_Web,                     // Web stream
    ST_File,                    // File stream
    ST_Arbitrary                // Arbitrary data stream
};

enum VIDEO_VBR_MODE
{
    VBR_OFF,                    // VBR is not used (CBR)
    VBR_QUALITYBASED,           // Quality based VBR, generally one pass encoding
    VBR_CONSTRAINED,            // Constrain the Peak Bitrate and also ( optionally ) the peak bufferwindow, 2 Pass
    VBR_UNCONSTRAINED           // No constraint on the Peak Bitrate, generally 2 pass
};

enum MUTEX_TYPE
{
    MT_Bitrate,                 // Exclude the streams based on bitrate
    MT_Language,                // Exclude the streams based on language
    MT_Presentation             // Exclude the streams based on the presentation format (for example, different aspect ratios)
};


/*
** Methods helpful in creation of a profile
*/
///////////////////////////////////////////////////////////////////////////////
STDMETHODIMP EnsureIWMCodecInfo3( IWMCodecInfo3** ppCodecInfo3 );

///////////////////////////////////////////////////////////////////////////////
STDMETHODIMP SetCodecVBRSettings( IWMCodecInfo3* pCodecInfo3, GUID guidCodecType, DWORD dwCodecIndex, BOOL bIsVBR, DWORD dwVBRPasses );

///////////////////////////////////////////////////////////////////////////////
STDMETHODIMP SetStreamLanguage( IWMStreamConfig * pStreamConfig, LCID dwLanguage );

///////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CreateUncompressedAudioMediaType(  WM_MEDIA_TYPE** ppmtMediaType,
                                    DWORD dwSamplesPerSecond, 
                                    WORD wNumChannels, 
                                    WORD wBitsPerSample );

///////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CreateVideoMediaType(  WM_MEDIA_TYPE** ppmtMediaType,
                                    IWMCodecInfo3* pCodecInfo3, 
                                    DWORD dwCodecIndex,
                                    DWORD dwFPS, 
                                    DWORD dwWidth, 
                                    DWORD dwHeight, 
                                    DWORD dwBitrate,
                                    BOOL bIsVBR,
                                    DWORD dwNumberOfPasses );

///////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CreateUncompressedVideoMediaType(  WM_MEDIA_TYPE** ppmtMediaType,
                                    GUID guidFormat,
                                    DWORD dwFourCC,
                                    WORD wBitsPerPixel,
                                    BYTE* pbPaletteData,
                                    DWORD cbPaletteDataSize,
                                    DWORD dwFPS, 
                                    DWORD dwWidth, 
                                    DWORD dwHeight );

///////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CreateScriptMediaType( WM_MEDIA_TYPE** ppmtMediaType );

///////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CreateImageMediaType( WM_MEDIA_TYPE** ppmtMediaType, DWORD dwBitrate, DWORD dwWidth, DWORD dwHeight );

///////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CreateWebMediaType( WM_MEDIA_TYPE** ppmtMediaType );

///////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CreateFileMediaType( WM_MEDIA_TYPE** ppmtMediaType );

///////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CreateMediatypeForFormat( WM_MEDIA_TYPE** ppmtDestination,
                                       IWMCodecInfo3* pCodecInfo, 
                                       IWMStreamConfig** ppFormatConfig,
                                       GUID guidStreamType, 
                                       DWORD dwCodecIndex, 
                                       DWORD dwFormatIndex );

///////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CreateMediatypeForFormat( WM_MEDIA_TYPE** ppmtDestination,
                                       IWMCodecInfo3* pCodecInfo3, 
                                       IWMStreamConfig** ppFormatConfig,
                                       GUID guidCodecType, 
                                       DWORD dwCodecIndex, 
                                       DWORD dwFormatIndex, 
                                       BOOL bIsVBR, 
                                       DWORD dwVBRPasses );

///////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CreateFileStream( IWMStreamConfig** ppStreamConfig, 
                               IWMProfile *pProfile, 
                               DWORD dwBufferWindow, 
                               DWORD dwBitrate, 
                               WORD wMaxFilenameLength,
                               LCID dwLanguage );

///////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CreateWebStream( IWMStreamConfig** ppStreamConfig, 
                              IWMProfile *pProfile, 
                              DWORD dwBitrate, 
                              DWORD dwBufferWindow,
                              LCID dwLanguage );

///////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CreateImageStream( IWMStreamConfig** ppStreamConfig,
                                IWMProfile *pProfile,
                                DWORD dwBitrate, 
                                DWORD dwBufferWindow, 
                                DWORD dwWidth,
                                DWORD dwHeight,
                                LCID dwLanguage );

///////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CreateScriptStream( IWMStreamConfig** ppStreamConfig, 
                                 IWMProfile *pProfile, 
                                 DWORD dwBitrate, 
                                 DWORD dwBufferWindow,
                                 LCID dwLanguage  );

///////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CreateAudioStream( IWMStreamConfig** ppStreamConfig, 
                                IWMCodecInfo3* pCodecInfo3, 
                                IWMProfile *pProfile,
                                DWORD dwBufferWindow,
                                DWORD dwCodecIndex,
                                DWORD dwFormatIndex,
                                BOOL bIsVBR,
                                DWORD dwNumberOfPasses,
                                LCID dwLanguage );

///////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CreateUncompressedAudioStream( IWMStreamConfig** ppStreamConfig, 
                                IWMProfile *pProfile,
                                DWORD dwSamplesPerSecond, 
                                WORD wNumChannels, 
                                WORD wBitsPerSample,
                                LCID dwLanguage );

///////////////////////////////////////////////////////////////////////////////
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
                                LCID dwLanguage );

///////////////////////////////////////////////////////////////////////////////
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
                                LCID dwLanguage );
                                
///////////////////////////////////////////////////////////////////////////////
STDMETHODIMP WriteProfileAsPRX( LPCTSTR tszFilename, LPCWSTR wszProfileData, DWORD dwProfileDataLength );

///////////////////////////////////////////////////////////////////////////////
STDMETHODIMP AddSMPTEExtensionToStream( IWMStreamConfig* pStream );

///////////////////////////////////////////////////////////////////////////////
STDMETHODIMP GetUncompressedWaveFormatCount( DWORD * pdwCount );

///////////////////////////////////////////////////////////////////////////////
STDMETHODIMP GetUncompressedWaveFormat( DWORD dwIndex,                                 
                                        DWORD * pdwSamplesPerSecond, 
                                        WORD * pwNumChannels, 
                                        WORD * pwBitsPerSample );


///////////////////////////////////////////////////////////////////////////////
STDMETHODIMP GetUncompressedPixelFormatCount( DWORD * pdwCount );

///////////////////////////////////////////////////////////////////////////////
STDMETHODIMP GetUncompressedPixelFormat( DWORD dwIndex,                                 
                                         GUID * pguidFormat,
                                         DWORD * pdwFourCC,
                                         WORD * pwBitsPerPixel );

#endif __GENPROFILE_H__
