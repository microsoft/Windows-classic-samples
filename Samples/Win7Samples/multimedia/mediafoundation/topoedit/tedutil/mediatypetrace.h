// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

#include "atlbase.h"
#include "atlconv.h"

#include <assert.h>

#define CHECKHR_GOTO(x, y) if(FAILED(x)) goto y

#define INTERNAL_GUID_TO_STRING( _Attribute, _skip ) \
    if( Attr == _Attribute) \
    { \
        pAttrStr =  #_Attribute; \
        C_ASSERT( (sizeof(#_Attribute) / sizeof(#_Attribute[0])) > _skip ); \
        pAttrStr += _skip; \
        goto done; \
    } \
    
LPCSTR STRING_FROM_GUID( GUID Attr )
{
    LPCSTR pAttrStr = NULL;

    // Generics
    INTERNAL_GUID_TO_STRING( MF_MT_MAJOR_TYPE, 6 );                     // MAJOR_TYPE
    INTERNAL_GUID_TO_STRING( MF_MT_SUBTYPE, 6 );                        // SUBTYPE
    INTERNAL_GUID_TO_STRING( MF_MT_ALL_SAMPLES_INDEPENDENT, 6 );        // ALL_SAMPLES_INDEPENDENT   
    INTERNAL_GUID_TO_STRING( MF_MT_FIXED_SIZE_SAMPLES, 6 );             // FIXED_SIZE_SAMPLES
    INTERNAL_GUID_TO_STRING( MF_MT_COMPRESSED, 6 );                     // COMPRESSED
    INTERNAL_GUID_TO_STRING( MF_MT_SAMPLE_SIZE, 6 );                    // SAMPLE_SIZE
    INTERNAL_GUID_TO_STRING( MF_MT_USER_DATA, 6 );                      // MF_MT_USER_DATA

    // Audio
    INTERNAL_GUID_TO_STRING( MF_MT_AUDIO_NUM_CHANNELS, 12 );            // NUM_CHANNELS
    INTERNAL_GUID_TO_STRING( MF_MT_AUDIO_SAMPLES_PER_SECOND, 12 );      // SAMPLES_PER_SECOND
    INTERNAL_GUID_TO_STRING( MF_MT_AUDIO_AVG_BYTES_PER_SECOND, 12 );    // AVG_BYTES_PER_SECOND
    INTERNAL_GUID_TO_STRING( MF_MT_AUDIO_BLOCK_ALIGNMENT, 12 );         // BLOCK_ALIGNMENT
    INTERNAL_GUID_TO_STRING( MF_MT_AUDIO_BITS_PER_SAMPLE, 12 );         // BITS_PER_SAMPLE
    INTERNAL_GUID_TO_STRING( MF_MT_AUDIO_VALID_BITS_PER_SAMPLE, 12 );   // VALID_BITS_PER_SAMPLE
    INTERNAL_GUID_TO_STRING( MF_MT_AUDIO_SAMPLES_PER_BLOCK, 12 );       // SAMPLES_PER_BLOCK
    INTERNAL_GUID_TO_STRING( MF_MT_AUDIO_CHANNEL_MASK, 12 );            // CHANNEL_MASK
    INTERNAL_GUID_TO_STRING( MF_MT_AUDIO_PREFER_WAVEFORMATEX, 12 );     // PREFER_WAVEFORMATEX

    // Video
    INTERNAL_GUID_TO_STRING( MF_MT_FRAME_SIZE, 6 );                     // FRAME_SIZE
    INTERNAL_GUID_TO_STRING( MF_MT_FRAME_RATE, 6 );                     // FRAME_RATE
    INTERNAL_GUID_TO_STRING( MF_MT_PIXEL_ASPECT_RATIO, 6 );             // PIXEL_ASPECT_RATIO
    INTERNAL_GUID_TO_STRING( MF_MT_INTERLACE_MODE, 6 );                 // INTERLACE_MODE
    INTERNAL_GUID_TO_STRING( MF_MT_AVG_BITRATE, 6 );                    // AVG_BITRATE

    // Major type values
    INTERNAL_GUID_TO_STRING( MFMediaType_Default, 12 );                 // Default
    INTERNAL_GUID_TO_STRING( MFMediaType_Audio, 12 );                   // Audio
    INTERNAL_GUID_TO_STRING( MFMediaType_Video, 12 );                   // Video
    INTERNAL_GUID_TO_STRING( MFMediaType_Script, 12 );                  // Script
    INTERNAL_GUID_TO_STRING( MFMediaType_Image, 12 );                   // Image
    INTERNAL_GUID_TO_STRING( MFMediaType_HTML, 12 );                    // HTML
    INTERNAL_GUID_TO_STRING( MFMediaType_Binary, 12 );                  // Binary
    INTERNAL_GUID_TO_STRING( MFMediaType_SAMI, 12 );                    // SAMI
    INTERNAL_GUID_TO_STRING( MFMediaType_Protected, 12 );               // Protected

    // Minor video type values
    INTERNAL_GUID_TO_STRING( MFVideoFormat_Base, 14 );                  // Base
    INTERNAL_GUID_TO_STRING( MFVideoFormat_MP43, 14 );                  // MP43
    INTERNAL_GUID_TO_STRING( MFVideoFormat_WMV1, 14 );                  // WMV1
    INTERNAL_GUID_TO_STRING( MFVideoFormat_WMV2, 14 );                  // WMV2
    INTERNAL_GUID_TO_STRING( MFVideoFormat_WMV3, 14 );                  // WMV3
    INTERNAL_GUID_TO_STRING( MFVideoFormat_MPG1, 14 );                  // MPG1
    INTERNAL_GUID_TO_STRING( MFVideoFormat_MPG2, 14 );                  // MPG2

    // Minor audio type values
    INTERNAL_GUID_TO_STRING( MFAudioFormat_Base, 14 );                  // Base
    INTERNAL_GUID_TO_STRING( MFAudioFormat_PCM, 14 );                   // PCM
    INTERNAL_GUID_TO_STRING( MFAudioFormat_DTS, 14 );                   // DTS
    INTERNAL_GUID_TO_STRING( MFAudioFormat_Dolby_AC3_SPDIF, 14 );       // Dolby_AC3_SPDIF
    INTERNAL_GUID_TO_STRING( MFAudioFormat_Float, 14 );                 // IEEEFloat
    INTERNAL_GUID_TO_STRING( MFAudioFormat_WMAudioV8, 14 );             // WMAudioV8
    INTERNAL_GUID_TO_STRING( MFAudioFormat_WMAudioV9, 14 );             // WMAudioV9
    INTERNAL_GUID_TO_STRING( MFAudioFormat_WMAudio_Lossless, 14 );      // WMAudio_Lossless
    INTERNAL_GUID_TO_STRING( MFAudioFormat_WMASPDIF, 14 );              // WMASPDIF
    INTERNAL_GUID_TO_STRING( MFAudioFormat_MP3, 14 );                   // MP3
    INTERNAL_GUID_TO_STRING( MFAudioFormat_MPEG, 14 );                  // MPEG

done:    
    return pAttrStr;
}

class CMediaTypeTrace
{
public:
    CMediaTypeTrace(IMFMediaType * pMT)
        : m_spMT(pMT)
    {
    }
    
    LPCSTR GetString()
    {
        HRESULT hr = S_OK;
        GUID MajorType;
        UINT32 cAttrCount;
        LPCSTR pszGuidStr;
        CAtlStringA tempStr;
        WCHAR TempBuf[200];

        if( m_szResp.IsEmpty() == FALSE )
        {
        goto done;
        }

        if( m_spMT == NULL )
        {
        m_szResp.Append( "<NULL>" );

        goto done;
        }

        hr = m_spMT->GetMajorType( &MajorType );
        CHECKHR_GOTO( hr, done );    

        pszGuidStr = STRING_FROM_GUID(MajorType);
        if (pszGuidStr != NULL)
        {
            m_szResp.Append( pszGuidStr );
            m_szResp.Append( ": " );
        }
        else
        {
        m_szResp.Append( "Other: " );
        }

        hr = m_spMT->GetCount(&cAttrCount);
        CHECKHR_GOTO( hr, done );

        for( UINT32 i = 0; i < cAttrCount; i++ )
        {
        GUID guidId;
        MF_ATTRIBUTE_TYPE attrType;

        hr = m_spMT->GetItemByIndex(i, &guidId, NULL);
        CHECKHR_GOTO( hr, done );

        hr = m_spMT->GetItemType(guidId, &attrType);
        CHECKHR_GOTO( hr, done );

        pszGuidStr = STRING_FROM_GUID(guidId);
        if (pszGuidStr != NULL)
        {
            m_szResp.Append( pszGuidStr );
        }
        else
        {
            LPOLESTR guidStr = NULL;
            StringFromCLSID(guidId, &guidStr);
            
            m_szResp.Append(CW2A(guidStr));

            CoTaskMemFree(guidStr);
        }

        m_szResp.Append( "=" );

        switch( attrType )
        {
            case MF_ATTRIBUTE_UINT32:
            {
                UINT32 Val;
                hr = m_spMT->GetUINT32( guidId, &Val );
                CHECKHR_GOTO( hr, done );

                tempStr.Format("%d", Val);
                m_szResp.Append(tempStr) ;
                break;
            }                
            case MF_ATTRIBUTE_UINT64:
            {
                UINT64 Val;
                hr = m_spMT->GetUINT64(guidId, &Val);
                CHECKHR_GOTO( hr, done );

                if( guidId == MF_MT_FRAME_SIZE )
                {
                    tempStr.Format("W %u, H: %u", HI32( Val ), LO32( Val ) );
                }
                else if( (guidId == MF_MT_FRAME_RATE) || (guidId == MF_MT_PIXEL_ASPECT_RATIO) )
                {
                    tempStr.Format("W %u, H: %u", HI32( Val ), LO32( Val ) );
                }
                else
                {
                    tempStr.Format("%ld", Val);
                }

                m_szResp.Append( tempStr );

                break;
            }                
            case MF_ATTRIBUTE_DOUBLE:
            {
                DOUBLE Val;
                hr = m_spMT->GetDouble(guidId, &Val);
                CHECKHR_GOTO( hr, done );

                tempStr.Format("%f", Val);
                m_szResp.Append(tempStr);

                break;
            }                
            case MF_ATTRIBUTE_GUID:
            {
                GUID Val;
                const char * pValStr;

                hr = m_spMT->GetGUID(guidId, &Val);
                CHECKHR_GOTO( hr, done );

                pValStr = STRING_FROM_GUID(Val);
                if( pValStr != NULL)
                {
                    m_szResp.Append( pValStr );
                }
                else
                {
                    LPOLESTR guidStr = NULL;
                    StringFromCLSID(Val, &guidStr);
                    
                    m_szResp.Append(CW2A(guidStr));

                    CoTaskMemFree(guidStr);
                }
                
                break;
            }                
            case MF_ATTRIBUTE_STRING:
            {
                hr = m_spMT->GetString( guidId, TempBuf, sizeof(TempBuf) / sizeof(TempBuf[0]), NULL );
                if( hr == HRESULT_FROM_WIN32( ERROR_INSUFFICIENT_BUFFER ) )
                {
                    m_szResp.Append( "<Too Long>" );
                    break;
                }
                CHECKHR_GOTO( hr, done );

                m_szResp.Append( CW2A(TempBuf) );
                break;
            }                
            case MF_ATTRIBUTE_BLOB:
            {
                m_szResp.Append( "<BLOB>" );
                break;
            }                
            case MF_ATTRIBUTE_IUNKNOWN:
            {
                m_szResp.Append( "<UNK>" );
                break;
            }
            default:
                assert(0);
        }

        m_szResp.Append( ", " );
        }

        assert( m_szResp.GetLength() >= 2);
        m_szResp.Left( m_szResp.GetLength() - 2);

        done:

        if (FAILED(hr))
        {
        m_szResp.Empty();
        return NULL;
        }
        else
        {
        return (LPCSTR)m_szResp;
        }
    }
    
    
private:
    CAtlStringA m_szResp;
    CComPtr<IMFMediaType> m_spMT;    
};
