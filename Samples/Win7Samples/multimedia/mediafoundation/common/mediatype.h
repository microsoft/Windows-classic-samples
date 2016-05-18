#pragma once

#include <mferror.h>

namespace MediaFoundationSamples
{   
    class MediaType;
    class AudioType;
    class VideoType;
    class MPEGVideoType;

    MFOffset    MakeOffset(float v);
    MFVideoArea MakeArea(float x, float y, DWORD width, DWORD height);

    // Flat functions
    HRESULT     GetFrameRate(IMFMediaType *pType, MFRatio *pRatio);
    HRESULT     GetVideoDisplayArea(IMFMediaType *pType, MFVideoArea *pArea);
    HRESULT     GetDefaultStride(IMFMediaType *pType, LONG *plStride);


    //////////////////////////////////////////////////////////////////////////
    // MediaType is a wrapper around an IMFMediaType. It can be used 
    // to read (and modify) an existing type or create a new type. This class
    // does not implement the IMFMediaType interface.
    //
    // The AudioType, VideoType, and MPEGVideoType
    // classes are specialized for those formats.
    //////////////////////////////////////////////////////////////////////////

    class MediaType 
    {
    protected:
        IMFMediaType* m_pType;

    protected:
        BOOL IsValid()
        {
            return m_pType != NULL;
        }

        IMFMediaType* GetMediaType()
        {
            assert(IsValid());
            return m_pType;
        }

    public:

        // Construct from an existing media type.
        MediaType(IMFMediaType* pType = NULL) : m_pType(NULL)
        {
            if (pType)
            {
                m_pType = pType;
                m_pType->AddRef();  
            }
        }

        // Copy ctor
        MediaType(const MediaType&  mt)
        {
            m_pType = mt.m_pType;
            if (m_pType)
            {
                m_pType->AddRef();
            }
        }

        virtual ~MediaType()
        {
            SAFE_RELEASE(m_pType);
        }


        // Assignment
        MediaType& operator=(const MediaType& mt)
        {
            // If we are assigned to ourselves, do nothing.
            if (!AreComObjectsEqual(m_pType, mt.m_pType))
            {
                if (m_pType)
                {
                    m_pType->Release();
                }

                m_pType = mt.m_pType;
                if (m_pType)
                {
                    m_pType->AddRef();
                }
            }
            return *this;
        }

        // address-of operator
        IMFMediaType** operator&()
        {
            return &m_pType;
        }

        // coerce to underlying pointer type.
        operator IMFMediaType*()
        {
            return m_pType;
        }

        virtual HRESULT CreateEmptyType()
        {
            SAFE_RELEASE(m_pType);

            HRESULT hr = S_OK;

            hr = MFCreateMediaType(&m_pType);

            return hr;
        }

        // Detach the interface (does not Release)
        IMFMediaType* Detach()
        {
            IMFMediaType* p = m_pType;
            m_pType = NULL;
            return p;
        }

        // Direct wrappers of IMFMediaType methods.
        // (For these methods, we leave parameter validation to the IMFMediaType implementation.)

        // Retrieves the major type GUID.
        HRESULT GetMajorType(GUID *pGuid)
        {
            return GetMediaType()->GetMajorType(pGuid);
        }

        // Specifies whether the media data is compressed
        HRESULT IsCompressedFormat(BOOL *pbCompressed)
        {
            return GetMediaType()->IsCompressedFormat(pbCompressed);
        }

        // Compares two media types and determines whether they are identical.
        HRESULT IsEqual(IMFMediaType *pType, DWORD *pdwFlags)
        {
            return GetMediaType()->IsEqual(pType, pdwFlags);
        }

        // Retrieves an alternative representation of the media type.
        HRESULT GetRepresentation( GUID guidRepresentation, LPVOID *ppvRepresentation)
        {
            return GetMediaType()->GetRepresentation(guidRepresentation, ppvRepresentation);
        }

        // Frees memory that was allocated by the GetRepresentation method.
        HRESULT FreeRepresentation( GUID guidRepresentation, LPVOID pvRepresentation) 
        {
            return GetMediaType()->FreeRepresentation(guidRepresentation, pvRepresentation);
        }


        // Helper methods

        // CopyFrom: Copy all of the attributes from another media type into this type.
        HRESULT CopyFrom(MediaType *pType)
        {
            if (!pType->IsValid())
            {
                return E_UNEXPECTED;
            }
            if (pType == NULL)
            {
                return E_POINTER;
            }
            return CopyFrom(pType->m_pType);
        }

        HRESULT CopyFrom(IMFMediaType *pType)
        {
            if (pType == NULL)
            {
                return E_POINTER;
            }
            return pType->CopyAllItems(m_pType);
        }

        // Returns the underlying IMFMediaType pointer. 
        HRESULT GetMediaType(IMFMediaType** ppType)
        {
            assert(IsValid());
            CheckPointer(ppType, E_POINTER);
            *ppType = m_pType;
            (*ppType)->AddRef();
            return S_OK;
        }

        // Sets the major type GUID.
        HRESULT SetMajorType(GUID guid)
        {
            return GetMediaType()->SetGUID(MF_MT_MAJOR_TYPE, guid);
        }

        // Retrieves the subtype GUID.
        HRESULT GetSubType(GUID* pGuid)
        {
            CheckPointer(pGuid, E_POINTER);
            return GetMediaType()->GetGUID(MF_MT_SUBTYPE, pGuid);
        }

        // Sets the subtype GUID.
        HRESULT SetSubType(GUID guid)
        {
            return GetMediaType()->SetGUID(MF_MT_SUBTYPE, guid);
        }

        // Extracts the FOURCC code from the subtype.
        // Not all subtypes follow this pattern.
        HRESULT GetFourCC(DWORD *pFourCC)
        {
            assert(IsValid());
            CheckPointer(pFourCC, E_POINTER);
            HRESULT hr = S_OK;
            GUID guidSubType = GUID_NULL;

            if (SUCCEEDED(hr))
            {
                hr = GetSubType(&guidSubType);
            }

            if (SUCCEEDED(hr))
            {
                *pFourCC = guidSubType.Data1;
            }
            return hr;
        }       

        //  Queries whether each sample is independent of the other samples in the stream.
        HRESULT GetAllSamplesIndependent(BOOL* pbIndependent)
        {
            CheckPointer(pbIndependent, E_POINTER);
            return GetMediaType()->GetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, (UINT32*)pbIndependent);
        }

        //  Specifies whether each sample is independent of the other samples in the stream.
        HRESULT SetAllSamplesIndependent(BOOL bIndependent)
        {
            return GetMediaType()->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, (UINT32)bIndependent);
        }

        // Queries whether the samples have a fixed size.
        HRESULT GetFixedSizeSamples(BOOL *pbFixed)
        {
            CheckPointer(pbFixed, E_POINTER);
            return GetMediaType()->GetUINT32(MF_MT_FIXED_SIZE_SAMPLES, (UINT32*)pbFixed);
        }

        // Specifies whether the samples have a fixed size.
        HRESULT SetFixedSizeSamples(BOOL bFixed)
        {
            return GetMediaType()->SetUINT32(MF_MT_FIXED_SIZE_SAMPLES, (UINT32)bFixed);
        }

        // Retrieves the size of each sample, in bytes. 
        HRESULT GetSampleSize(UINT32 *pnSize)
        {   
            CheckPointer(pnSize, E_POINTER);
            return GetMediaType()->GetUINT32(MF_MT_SAMPLE_SIZE, pnSize);
        }

        // Sets the size of each sample, in bytes. 
        HRESULT SetSampleSize(UINT32 nSize)
        {   
            return GetMediaType()->SetUINT32(MF_MT_SAMPLE_SIZE, nSize);
        }

        // Retrieves a media type that was wrapped by the MFWrapMediaType function.
        HRESULT Unwrap(IMFMediaType **ppOriginal)
        {
            CheckPointer(ppOriginal, E_POINTER);
            return ::MFUnwrapMediaType(GetMediaType(), ppOriginal);
        }

        // The following versions return reasonable defaults if the relevant attribute is not present (zero/FALSE).
        // This is useful for making quick comparisons betweeen media types. 

        BOOL AllSamplesIndependent()
        {
            return (BOOL)MFGetAttributeUINT32(GetMediaType(), MF_MT_ALL_SAMPLES_INDEPENDENT, FALSE);
        }

        BOOL FixedSizeSamples()
        {
            return (BOOL)MFGetAttributeUINT32(GetMediaType(), MF_MT_FIXED_SIZE_SAMPLES, FALSE);
        }

        UINT32 SampleSize()
        {   
            return MFGetAttributeUINT32(GetMediaType(), MF_MT_SAMPLE_SIZE, 0);
        }
    };

    
    //////////////////////////////////////////////////////////////////////////
    //
    // VideoType provides access to video-specific attributes.
    //
    // Note: MPEG-specific attributes are accessible from MPEGVideoType.
    //
    //////////////////////////////////////////////////////////////////////////

    class VideoType : public MediaType
    {
        friend class MediaType; 

    public:

        VideoType(IMFMediaType* pType = NULL) : MediaType(pType)
        {
        }

        virtual HRESULT CreateEmptyType()
        {
            SAFE_RELEASE(m_pType);

            HRESULT hr = S_OK;

            hr = MediaType::CreateEmptyType();
            if (SUCCEEDED(hr))
            {
                hr = SetMajorType(MFMediaType_Video);
            }
            return hr;
        }


        // Retrieves a description of how the frames are interlaced.
        HRESULT GetInterlaceMode(MFVideoInterlaceMode *pmode)
        {
            CheckPointer(pmode, E_POINTER);
            return GetMediaType()->GetUINT32(MF_MT_INTERLACE_MODE, (UINT32*)pmode);
        }

        // Sets a description of how the frames are interlaced.
        HRESULT SetInterlaceMode(MFVideoInterlaceMode mode)
        {
            return GetMediaType()->SetUINT32(MF_MT_INTERLACE_MODE, (UINT32)mode);
        }

        // This returns the default or attempts to compute it, in its absence.
        HRESULT GetDefaultStride(LONG *plStride)
        {
            return MediaFoundationSamples::GetDefaultStride( GetMediaType(), plStride );
        }

        // Sets the default stride. Only appropriate for uncompressed data formats.
        HRESULT SetDefaultStride(LONG nStride)
        {
            return GetMediaType()->SetUINT32(MF_MT_DEFAULT_STRIDE, (UINT32)nStride);
        }

        // Retrieves the width and height of the video frame.
        HRESULT GetFrameDimensions(UINT32 *pdwWidthInPixels, UINT32 *pdwHeightInPixels)
        {
            return MFGetAttributeSize(GetMediaType(), MF_MT_FRAME_SIZE, pdwWidthInPixels, pdwHeightInPixels);
        }

        // Sets the width and height of the video frame.
        HRESULT SetFrameDimensions(UINT32 dwWidthInPixels, UINT32 dwHeightInPixels)
        {
            return MFSetAttributeSize(GetMediaType(), MF_MT_FRAME_SIZE, dwWidthInPixels, dwHeightInPixels);
        }

        // Retrieves the data error rate in bit errors per second
        HRESULT GetDataBitErrorRate(UINT32 *pRate)
        {
            CheckPointer(pRate, E_POINTER);
            return GetMediaType()->GetUINT32(MF_MT_AVG_BIT_ERROR_RATE, pRate);
        }

        // Sets the data error rate in bit errors per second
        HRESULT SetDataBitErrorRate(UINT32 rate)
        {
            return GetMediaType()->SetUINT32(MF_MT_AVG_BIT_ERROR_RATE, rate);
        }

        // Retrieves the approximate data rate of the video stream.
        HRESULT GetAverageBitRate(UINT32 *pRate)
        {
            CheckPointer(pRate, E_POINTER);
            return GetMediaType()->GetUINT32(MF_MT_AVG_BITRATE, pRate);
        }

        // Sets the approximate data rate of the video stream.
        HRESULT SetAvgerageBitRate(UINT32 rate)
        {
            return GetMediaType()->SetUINT32(MF_MT_AVG_BITRATE, rate);
        }

        // Retrieves custom color primaries.
        HRESULT GetCustomVideoPrimaries(MT_CUSTOM_VIDEO_PRIMARIES *pPrimaries)
        {
            CheckPointer(pPrimaries, E_POINTER);
            return GetMediaType()->GetBlob(MF_MT_CUSTOM_VIDEO_PRIMARIES, (UINT8*)pPrimaries, sizeof(MT_CUSTOM_VIDEO_PRIMARIES), NULL);
        }

        // Sets custom color primaries.
        HRESULT SetCustomVideoPrimaries(const MT_CUSTOM_VIDEO_PRIMARIES& primary)
        {
            return GetMediaType()->SetBlob(MF_MT_CUSTOM_VIDEO_PRIMARIES, (const UINT8*)&primary, sizeof(MT_CUSTOM_VIDEO_PRIMARIES));
        }

        // Gets the number of frames per second.
        HRESULT GetFrameRate(UINT32 *pnNumerator, UINT32 *pnDenominator)
        {
            CheckPointer(pnNumerator, E_POINTER);
            CheckPointer(pnDenominator, E_POINTER);
            return MFGetAttributeRatio(GetMediaType(), MF_MT_FRAME_RATE, pnNumerator, pnDenominator);
        }

        // Gets the frames per second as a ratio.
        HRESULT GetFrameRate(MFRatio *pRatio)
        {
            CheckPointer(pRatio, E_POINTER);
            return GetFrameRate((UINT32*)&pRatio->Numerator, (UINT32*)&pRatio->Denominator);
        }
        
        // Sets the number of frames per second.
        HRESULT SetFrameRate(UINT32 nNumerator, UINT32 nDenominator)
        {
            return MFSetAttributeRatio(GetMediaType(), MF_MT_FRAME_RATE, nNumerator, nDenominator);
        }

        // Sets the number of frames per second, as a ratio.
        HRESULT SetFrameRate(const MFRatio& ratio)
        {
            return MFSetAttributeRatio(GetMediaType(), MF_MT_FRAME_RATE, ratio.Numerator, ratio.Denominator);
        }

        // Queries the geometric aperture.
        HRESULT GetGeometricAperture(MFVideoArea *pArea)
        {
            CheckPointer(pArea, E_POINTER);
            return GetMediaType()->GetBlob(MF_MT_GEOMETRIC_APERTURE, (UINT8*)pArea, sizeof(MFVideoArea), NULL);
        }
        
        // Sets the geometric aperture.
        HRESULT SetGeometricAperture(const MFVideoArea& area)
        {
            return GetMediaType()->SetBlob(MF_MT_GEOMETRIC_APERTURE, (UINT8*)&area, sizeof(MFVideoArea));
        }
        
        // Retrieves the maximum number of frames from one key frame to the next.
        HRESULT GetMaxKeyframeSpacing(UINT32 *pnSpacing)
        {
            CheckPointer(pnSpacing, E_POINTER);
            return GetMediaType()->GetUINT32(MF_MT_MAX_KEYFRAME_SPACING, pnSpacing);
        }

        // Sets the maximum number of frames from one key frame to the next.
        HRESULT SetMaxKeyframeSpacing(UINT32 nSpacing)
        {
            return GetMediaType()->SetUINT32(MF_MT_MAX_KEYFRAME_SPACING, nSpacing);
        }

        // Retrieves the region that contains the valid portion of the signal.
        HRESULT GetMinDisplayAperture(MFVideoArea *pArea)
        {
            CheckPointer(pArea, E_POINTER);
            return GetMediaType()->GetBlob(MF_MT_MINIMUM_DISPLAY_APERTURE, (UINT8*)pArea, sizeof(MFVideoArea), NULL);
        }

        // Sets the the region that contains the valid portion of the signal.
        HRESULT SetMinDisplayAperture(const MFVideoArea& area)
        {
            return GetMediaType()->SetBlob(MF_MT_MINIMUM_DISPLAY_APERTURE, (UINT8*)&area, sizeof(MFVideoArea));
        }

 
        // Retrieves the aspect ratio of the output rectangle for a video media type. 
        HRESULT GetPadControlFlags(MFVideoPadFlags *pFlags)
        {
            CheckPointer(pFlags, E_POINTER);
            return GetMediaType()->GetUINT32(MF_MT_PAD_CONTROL_FLAGS, (UINT32*)pFlags);
        }

        // Sets the aspect ratio of the output rectangle for a video media type. 
        HRESULT SetPadControlFlags(MFVideoPadFlags flags)
        {
            return GetMediaType()->SetUINT32(MF_MT_PAD_CONTROL_FLAGS, flags);
        }

        // Retrieves an array of palette entries for a video media type. 
        HRESULT GetPaletteEntries(MFPaletteEntry *paEntries, UINT32 nEntries)
        {
            CheckPointer(paEntries, E_POINTER);
            return GetMediaType()->GetBlob(MF_MT_PALETTE, (UINT8*)paEntries, sizeof(MFPaletteEntry) * nEntries, NULL);
        }

        // Sets an array of palette entries for a video media type. 
        HRESULT SetPaletteEntries(MFPaletteEntry *paEntries, UINT32 nEntries)
        {
            CheckPointer(paEntries, E_POINTER);
            return GetMediaType()->SetBlob(MF_MT_PALETTE, (UINT8*)paEntries, sizeof(MFPaletteEntry) * nEntries);
        }

        // Retrieves the number of palette entries.
        HRESULT GetNumPaletteEntries(UINT32 *pnEntries)
        {
            CheckPointer(pnEntries, E_POINTER);
            UINT32 nBytes = 0;
            HRESULT hr = S_OK;
            hr = GetMediaType()->GetBlobSize(MF_MT_PALETTE, &nBytes);
            if (SUCCEEDED(hr))
            {
                if (nBytes % sizeof(MFPaletteEntry) != 0)
                {
                    hr = E_UNEXPECTED;
                }
            }
            if (SUCCEEDED(hr))
            {
                *pnEntries = nBytes / sizeof(MFPaletteEntry);
            }
            return hr;
        }

        // Queries the 4×3 region of video that should be displayed in pan/scan mode.
        HRESULT GetPanScanAperture(MFVideoArea *pArea)
        {
            CheckPointer(pArea, E_POINTER);
            return GetMediaType()->GetBlob(MF_MT_PAN_SCAN_APERTURE, (UINT8*)pArea, sizeof(MFVideoArea), NULL);
        }
        
        // Sets the 4×3 region of video that should be displayed in pan/scan mode.
        HRESULT SetPanScanAperture(const MFVideoArea& area)
        {
            return GetMediaType()->SetBlob(MF_MT_PAN_SCAN_APERTURE, (UINT8*)&area, sizeof(MFVideoArea));
        }
        
        // Queries whether pan/scan mode is enabled.
        HRESULT IsPanScanEnabled(BOOL *pBool)
        {
            CheckPointer(pBool, E_POINTER);
            return GetMediaType()->GetUINT32(MF_MT_PAN_SCAN_ENABLED, (UINT32*)pBool);
        }

        // Sets whether pan/scan mode is enabled.
        HRESULT SetPanScanEnabled(BOOL bEnabled)
        {
            return GetMediaType()->SetUINT32(MF_MT_PAN_SCAN_ENABLED, (UINT32)bEnabled);
        }

        // Queries the pixel aspect ratio
        HRESULT GetPixelAspectRatio(UINT32 *pnNumerator, UINT32 *pnDenominator)
        {
            CheckPointer(pnNumerator, E_POINTER);
            CheckPointer(pnDenominator, E_POINTER);
            return MFGetAttributeRatio(GetMediaType(), MF_MT_PIXEL_ASPECT_RATIO, pnNumerator, pnDenominator);
        }       

        // Sets the pixel aspect ratio
        HRESULT SetPixelAspectRatio(UINT32 nNumerator, UINT32 nDenominator)
        {
            return MFSetAttributeRatio(GetMediaType(), MF_MT_PIXEL_ASPECT_RATIO, nNumerator, nDenominator);
        }

        HRESULT SetPixelAspectRatio(const MFRatio& ratio)
        {
            return MFSetAttributeRatio(GetMediaType(), MF_MT_PIXEL_ASPECT_RATIO, ratio.Numerator, ratio.Denominator);
        }

        // Queries the intended aspect ratio.
        HRESULT GetSourceContentHint(MFVideoSrcContentHintFlags *pFlags)
        {
            CheckPointer(pFlags, E_POINTER);
            return GetMediaType()->GetUINT32(MF_MT_SOURCE_CONTENT_HINT, (UINT32*)pFlags);
        }

        // Sets the intended aspect ratio.
        HRESULT SetSourceContentHint(MFVideoSrcContentHintFlags nFlags)
        {
            return GetMediaType()->SetUINT32(MF_MT_SOURCE_CONTENT_HINT, (UINT32)nFlags);
        }

        // Queries an enumeration which represents the conversion function from RGB to R'G'B'.
        HRESULT GetTransferFunction(MFVideoTransferFunction *pnFxn)
        {
            CheckPointer(pnFxn, E_POINTER);
            return GetMediaType()->GetUINT32(MF_MT_TRANSFER_FUNCTION, (UINT32*)pnFxn);
        }

        // Set an enumeration which represents the conversion function from RGB to R'G'B'.
        HRESULT SetTransferFunction(MFVideoTransferFunction nFxn)
        {
            return GetMediaType()->SetUINT32(MF_MT_TRANSFER_FUNCTION, (UINT32)nFxn);
        }

        // Queries how chroma was sampled for a Y'Cb'Cr' video media type.
        HRESULT GetChromaSiting(MFVideoChromaSubsampling *pSampling)
        {
            CheckPointer(pSampling, E_POINTER);
            return GetMediaType()->GetUINT32(MF_MT_VIDEO_CHROMA_SITING, (UINT32*)pSampling);
        }
        
        // Sets how chroma was sampled for a Y'Cb'Cr' video media type.
        HRESULT SetChromaSiting(MFVideoChromaSubsampling nSampling)
        {
            return GetMediaType()->SetUINT32(MF_MT_VIDEO_CHROMA_SITING, (UINT32)nSampling);
        }
        
        // Queries the optimal lighting conditions for viewing.
        HRESULT GetVideoLighting(MFVideoLighting *pLighting)
        {
            CheckPointer(pLighting, E_POINTER);
            return GetMediaType()->GetUINT32(MF_MT_VIDEO_LIGHTING, (UINT32*)pLighting);
        }
        
        // Sets the optimal lighting conditions for viewing.
        HRESULT SetVideoLighting(MFVideoLighting nLighting)
        {
            return GetMediaType()->SetUINT32(MF_MT_VIDEO_LIGHTING, (UINT32)nLighting);
        }
        
        // Queries the nominal range of the color information in a video media type. 
        HRESULT GetVideoNominalRange(MFNominalRange *pRange)
        {
            CheckPointer(pRange, E_POINTER);
            return GetMediaType()->GetUINT32(MF_MT_VIDEO_NOMINAL_RANGE, (UINT32*)pRange);
        }

        // Sets the nominal range of the color information in a video media type. 
        HRESULT SetVideoNominalRange(MFNominalRange nRange)
        {
            return GetMediaType()->SetUINT32(MF_MT_VIDEO_NOMINAL_RANGE, (UINT32)nRange);
        }

        // Queries the color primaries for a video media type.
        HRESULT GetVideoPrimaries(MFVideoPrimaries *pPrimaries)
        {
            CheckPointer(pPrimaries, E_POINTER);
            return GetMediaType()->GetUINT32(MF_MT_VIDEO_PRIMARIES, (UINT32*)pPrimaries);
        }

        // Sets the color primaries for a video media type.
        HRESULT SetVideoPrimaries(MFVideoPrimaries nPrimaries)
        {
            return GetMediaType()->SetUINT32(MF_MT_VIDEO_PRIMARIES, (UINT32)nPrimaries);
        }

        // Gets a enumeration representing the conversion matrix from the 
        // Y'Cb'Cr' color space to the R'G'B' color space.
        HRESULT GetYUVMatrix(MFVideoTransferMatrix *pMatrix)
        {
            CheckPointer(pMatrix, E_POINTER);
            return GetMediaType()->GetUINT32(MF_MT_YUV_MATRIX, (UINT32*)pMatrix);
        } 

        // Sets an enumeration representing the conversion matrix from the 
        // Y'Cb'Cr' color space to the R'G'B' color space.
        HRESULT SetYUVMatrix(MFVideoTransferMatrix nMatrix)
        {
            return GetMediaType()->SetUINT32(MF_MT_YUV_MATRIX, (UINT32)nMatrix);
        } 

        // 
        // The following versions return reasonable defaults if the relevant attribute is not present (zero/FALSE).
        // This is useful for making quick comparisons betweeen media types. 
        //

        MFRatio GetPixelAspectRatio() // Defaults to 1:1 (square pixels)
        {
            MFRatio PAR = { 0, 0 };
            HRESULT hr = S_OK;

            hr = MFGetAttributeRatio(GetMediaType(), MF_MT_PIXEL_ASPECT_RATIO, (UINT32*)&PAR.Numerator, (UINT32*)&PAR.Denominator);
            if (FAILED(hr))
            {
                PAR.Numerator = 1;
                PAR.Denominator = 1;
            }
            return PAR;
        }       

        BOOL IsPanScanEnabled() // Defaults to FALSE
        {
            return (BOOL)MFGetAttributeUINT32(GetMediaType(), MF_MT_PAN_SCAN_ENABLED, FALSE);
        }

        // Returns (in this order) 
        // 1. The pan/scan region, only if pan/scan mode is enabled.
        // 2. The geometric aperture.
        // 3. The entire video area.
        HRESULT GetVideoDisplayArea(MFVideoArea *pArea)
        {
            CheckPointer(pArea, E_POINTER);

            return MediaFoundationSamples::GetVideoDisplayArea(GetMediaType(), pArea);
        }
    };  


    //////////////////////////////////////////////////////////////////////////
    //
    // AudioTypeWrapper provides access to audio-specific attributes.
    //
    //////////////////////////////////////////////////////////////////////////

    class AudioType : public MediaType
    {
        friend class MediaType; 

    public:


        AudioType(IMFMediaType* pType = NULL) : MediaType(pType)
        {
        }
        
        virtual HRESULT CreateEmptyType()
        {
            SAFE_RELEASE(m_pType);

            HRESULT hr = S_OK;

            hr = MediaType::CreateEmptyType();

            if (SUCCEEDED(hr))
            {
                hr = SetMajorType(MFMediaType_Audio);
            }
            return hr;
        }
        
        // Query the average number of bytes per second.
        HRESULT GetAvgerageBytesPerSecond(UINT32 *pBytes)
        {
            CheckPointer(pBytes, E_POINTER);
            return GetMediaType()->GetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, pBytes);
        }

        // Sets the average number of bytes per second.
        HRESULT SetAvgerageBytesPerSecond(UINT32 nBytes)
        {
            return GetMediaType()->SetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, nBytes);
        }

        // Query the bits per audio sample
        HRESULT GetBitsPerSample(UINT32 *pBits)
        {
            CheckPointer(pBits, E_POINTER);
            return GetMediaType()->GetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, pBits);
        }
        
        // Sets the bits per audio sample
        HRESULT SetBitsPerSample(UINT32 nBits)
        {
            return GetMediaType()->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, nBits);
        }
        
        // Query the block aignment in bytes
        HRESULT GetBlockAlignment(UINT32 *pBytes)
        {
            CheckPointer(pBytes, E_POINTER);
            return GetMediaType()->GetUINT32(MF_MT_AUDIO_BLOCK_ALIGNMENT, pBytes);
        }
        
        // Sets the block aignment in bytes
        HRESULT SetBlockAlignment(UINT32 nBytes)
        {
            return GetMediaType()->SetUINT32(MF_MT_AUDIO_BLOCK_ALIGNMENT, nBytes);
        }
        
        // Query the assignment of audio channels to speaker positions.
        HRESULT GetChannelMask(UINT32 *pMask)
        {
            CheckPointer(pMask, E_POINTER);
            return GetMediaType()->GetUINT32(MF_MT_AUDIO_CHANNEL_MASK, pMask);
        }
        
        // Sets the assignment of audio channels to speaker positions.
        HRESULT SetChannelMask(UINT32 nMask)
        {
            return GetMediaType()->SetUINT32(MF_MT_AUDIO_CHANNEL_MASK, nMask);
        }
        
        // Query the number of audio samples per second (floating-point value).
        HRESULT GetFloatSamplesPerSecond(double *pfSampleRate)
        {
            CheckPointer(pfSampleRate, E_POINTER);
            return GetMediaType()->GetDouble(MF_MT_AUDIO_FLOAT_SAMPLES_PER_SECOND, pfSampleRate);
        }

        // Sets the number of audio samples per second (floating-point value).
        HRESULT SetFloatSamplesPerSecond(double fSampleRate)
        {
            return GetMediaType()->SetDouble(MF_MT_AUDIO_FLOAT_SAMPLES_PER_SECOND, fSampleRate);
        }

        // Queries the number of audio channels
        HRESULT GetNumChannels(UINT32 *pnChannels)
        {
            CheckPointer(pnChannels, E_POINTER);
            return GetMediaType()->GetUINT32(MF_MT_AUDIO_NUM_CHANNELS, pnChannels);
        }

        // Sets the number of audio channels
        HRESULT SetNumChannels(UINT32 nChannels)
        {
            return GetMediaType()->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, nChannels);
        }

        // Query the number of audio samples contained in one compressed block of audio data.
        HRESULT GetSamplesPerBlock(UINT32 *pnSamples)
        {
            CheckPointer(pnSamples, E_POINTER);
            return GetMediaType()->GetUINT32(MF_MT_AUDIO_SAMPLES_PER_BLOCK, pnSamples);
        }

        // Sets the number of audio samples contained in one compressed block of audio data.
        HRESULT SetSamplesPerBlock(UINT32 nSamples)
        {
            return GetMediaType()->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_BLOCK, nSamples);
        }

        // Query the number of audio samples per second as an integer value.
        HRESULT GetSamplesPerSecond(UINT32 *pnSampleRate)
        {
            CheckPointer(pnSampleRate, E_POINTER);
            return GetMediaType()->GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, pnSampleRate);
        }
 
        // Set the number of audio samples per second as an integer value.
        HRESULT SetSamplesPerSecond(UINT32 nSampleRate)
        {
            return GetMediaType()->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, nSampleRate);
        }
 
        // Query number of valid bits of audio data in each sample.
        HRESULT GetValidBitsPerSample(UINT32 *pnBits)
        {
            CheckPointer(pnBits, E_POINTER);
            HRESULT hr = GetMediaType()->GetUINT32(MF_MT_AUDIO_VALID_BITS_PER_SAMPLE, pnBits);
            if (!SUCCEEDED(hr))
            {
                hr = GetBitsPerSample(pnBits);
            }
            return hr;
        } 
        
        // Set the number of valid bits of audio data in each sample.
        HRESULT SetValidBitsPerSample(UINT32 nBits)
        {
            return GetMediaType()->SetUINT32(MF_MT_AUDIO_VALID_BITS_PER_SAMPLE, nBits);
        } 


        // The following versions return zero if the relevant attribute is not present. 
        // This is useful for making quick comparisons betweeen media types. 

        UINT32 AvgerageBytesPerSecond()
        {
            return MFGetAttributeUINT32(GetMediaType(), MF_MT_AUDIO_AVG_BYTES_PER_SECOND, 0);
        }

        UINT32 BitsPerSample()
        {
            return MFGetAttributeUINT32(GetMediaType(), MF_MT_AUDIO_BITS_PER_SAMPLE, 0);
        }
        
        UINT32 GetBlockAlignment()
        {
            return MFGetAttributeUINT32(GetMediaType(), MF_MT_AUDIO_BLOCK_ALIGNMENT, 0);
        }
        
        double FloatSamplesPerSecond()
        {
            return MFGetAttributeDouble(GetMediaType(), MF_MT_AUDIO_FLOAT_SAMPLES_PER_SECOND, 0.0);
        }

        UINT32 NumChannels()
        {
            return MFGetAttributeUINT32(GetMediaType(), MF_MT_AUDIO_NUM_CHANNELS, 0);
        }

        UINT32 SamplesPerSecond()
        {
            return MFGetAttributeUINT32(GetMediaType(), MF_MT_AUDIO_SAMPLES_PER_SECOND, 0);
        }

    };  


    //////////////////////////////////////////////////////////////////////////
    //
    // MPEGVideoType provides access to MPEG-1 and MPEG-2 video 
    // attributes, plus the general video attributes.
    //
    //////////////////////////////////////////////////////////////////////////

    class MPEGVideoType : public VideoType
    {
        friend class MediaType; 

    public:

        MPEGVideoType(IMFMediaType* pType = NULL) : VideoType(pType)
        {
        }

        // Retrieves the MPEG sequence header for a video media type.
        HRESULT GetMpegSeqHeader(BYTE *pData, UINT32 cbSize)
        {
            CheckPointer(pData, E_POINTER);
            return GetMediaType()->GetBlob(MF_MT_MPEG_SEQUENCE_HEADER, pData, cbSize, NULL);
        }

        // Sets the MPEG sequence header for a video media type.
        HRESULT SetMpegSeqHeader(const BYTE *pData, UINT32 cbSize)
        {
            CheckPointer(pData, E_POINTER);
            return GetMediaType()->SetBlob(MF_MT_MPEG_SEQUENCE_HEADER, pData, cbSize);
        }

        // Retrieves the size of the MPEG sequence header.
        HRESULT GetMpegSeqHeaderSize(UINT32 *pcbSize)
        {
            CheckPointer(pcbSize, E_POINTER);

            *pcbSize = 0;

            HRESULT hr = GetMediaType()->GetBlobSize(MF_MT_MPEG_SEQUENCE_HEADER, pcbSize);

            if (hr == MF_E_ATTRIBUTENOTFOUND)
            {
                hr = S_OK;
            }
            return hr;
        }
        
        // Retrieve the group-of-pictures (GOP) start time code, for an MPEG-1 or MPEG-2 video media type.
        HRESULT GetStartTimeCode(UINT32 *pnTime)
        {
            CheckPointer(pnTime, E_POINTER);
            return GetMediaType()->GetUINT32(MF_MT_MPEG_START_TIME_CODE, pnTime);
        }

        // Sets the group-of-pictures (GOP) start time code, for an MPEG-1 or MPEG-2 video media type.
        HRESULT SetStartTimeCode(UINT32 nTime)
        {
            return GetMediaType()->SetUINT32(MF_MT_MPEG_START_TIME_CODE, nTime);
        }

        // Retrieves assorted flags for MPEG-2 video media type
        HRESULT GetMPEG2Flags(UINT32 *pnFlags)
        {
            CheckPointer(pnFlags, E_POINTER);
            return GetMediaType()->GetUINT32(MF_MT_MPEG2_FLAGS, pnFlags);
        }

        // Sets assorted flags for MPEG-2 video media type
        HRESULT SetMPEG2Flags(UINT32 nFlags)
        {
            return GetMediaType()->SetUINT32(MF_MT_MPEG2_FLAGS, nFlags);
        }

        // Retrieves the MPEG-2 level in a video media type.
        HRESULT GetMPEG2Level(UINT32 *pLevel)
        {
            CheckPointer(pLevel, E_POINTER);
            return GetMediaType()->GetUINT32(MF_MT_MPEG2_LEVEL, pLevel);
        }
 
        // Sets the MPEG-2 level in a video media type.
        HRESULT SetMPEG2Level(UINT32 nLevel)
        {
            return GetMediaType()->SetUINT32(MF_MT_MPEG2_LEVEL, nLevel);
        }
 
        // Retrieves the MPEG-2 profile in a video media type
        HRESULT GetMPEG2Profile(UINT32 *pProfile)
        {
            CheckPointer(pProfile, E_POINTER);
            return GetMediaType()->GetUINT32(MF_MT_MPEG2_PROFILE, pProfile);
        }
 
        // Sets the MPEG-2 profile in a video media type
        HRESULT SetMPEG2Profile(UINT32 nProfile)
        {
            return GetMediaType()->SetUINT32(MF_MT_MPEG2_PROFILE, nProfile);
        }

    };


    //////////////////////////////////////////////////////////////////////////

    // Helper functions for manipulating media types.

    inline MFOffset MakeOffset(float v)
    {
        MFOffset offset;
        offset.value = short(v);
        offset.fract = WORD(65536 * (v-offset.value));
        return offset;
    }

    inline MFVideoArea MakeArea(float x, float y, DWORD width, DWORD height)
    {
        MFVideoArea area;
        area.OffsetX = MakeOffset(x);
        area.OffsetY = MakeOffset(y);
        area.Area.cx = width;
        area.Area.cy = height;
        return area;
    }

    // Get the frame rate from a video media type.
    inline HRESULT GetFrameRate(IMFMediaType *pType, MFRatio *pRatio)
    {
        return MFGetAttributeRatio(pType, MF_MT_FRAME_RATE, (UINT32*)&pRatio->Numerator, (UINT32*)&pRatio->Denominator);
    }

    // Get the correct display area from a video media type.
    inline HRESULT GetVideoDisplayArea(IMFMediaType *pType, MFVideoArea *pArea)
    {
        HRESULT hr = S_OK;
        BOOL bPanScan = FALSE;
        UINT32 width = 0, height = 0;

        bPanScan = MFGetAttributeUINT32(pType, MF_MT_PAN_SCAN_ENABLED, FALSE);

        // In pan/scan mode, try to get the pan/scan region.
        if (bPanScan)
        {
            hr = pType->GetBlob(MF_MT_PAN_SCAN_APERTURE, (UINT8*)pArea, sizeof(MFVideoArea), NULL);
        }

        // If not in pan/scan mode, or the pan/scan region is not set, get the minimimum display aperture.

        if (!bPanScan || hr == MF_E_ATTRIBUTENOTFOUND)
        {
            hr = pType->GetBlob(MF_MT_MINIMUM_DISPLAY_APERTURE, (UINT8*)pArea, sizeof(MFVideoArea), NULL);

            if (hr == MF_E_ATTRIBUTENOTFOUND)
            {
                // Minimum display aperture is not set.

                // For backward compatibility with some components, check for a geometric aperture. 
                hr = pType->GetBlob(MF_MT_GEOMETRIC_APERTURE, (UINT8*)pArea, sizeof(MFVideoArea), NULL);
            }

            // Default: Use the entire video area.

            if (hr == MF_E_ATTRIBUTENOTFOUND)
            {
                hr = MFGetAttributeSize(pType, MF_MT_FRAME_SIZE, &width, &height);
                if (SUCCEEDED(hr))
                {
                    *pArea = MakeArea(0.0, 0.0, width, height);
                }
            }
        }

        return hr;
    }


    inline HRESULT GetDefaultStride(IMFMediaType *pType, LONG *plStride)
    {
        LONG lStride = 0;

        // Try to get the default stride from the media type.
        HRESULT hr = pType->GetUINT32(MF_MT_DEFAULT_STRIDE, (UINT32*)&lStride);
        if (FAILED(hr))
        {
            // Attribute not set. Try to calculate the default stride.
            GUID subtype = GUID_NULL;

            UINT32 width = 0;
            UINT32 height = 0;

            // Get the subtype and the image size.
            hr = pType->GetGUID(MF_MT_SUBTYPE, &subtype);
            if (SUCCEEDED(hr))
            {
                hr = MFGetAttributeSize(pType, MF_MT_FRAME_SIZE, &width, &height);
            }
            if (SUCCEEDED(hr))
            {
                hr = MFGetStrideForBitmapInfoHeader(subtype.Data1, width, &lStride);
            }

            // Set the attribute for later reference.
            if (SUCCEEDED(hr))
            {
                (void)pType->SetUINT32(MF_MT_DEFAULT_STRIDE, UINT32(lStride));
            }
        }

        if (SUCCEEDED(hr))
        {
            *plStride = lStride;
        }
        return hr;
    }


}



