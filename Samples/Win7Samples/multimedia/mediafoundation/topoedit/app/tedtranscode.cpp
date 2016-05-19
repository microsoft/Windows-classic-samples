// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "stdafx.h"

#include "tedtranscode.h"
#include "tedobj.h"
#include "tedutil.h"

#include <wmsdkidl.h>
#include <mferror.h>
#include <assert.h>
#include <initguid.h>

#define SAFE_COTASKMEMFREE(p) CoTaskMemFree(p); p = NULL;

const LPCWSTR CTedTranscodeTopologyBuilder::m_kszTranscodeProfileFile = L"TranscodeProfiles.xml";

const CTedTranscodeTopologyBuilder::StringGuidMap CTedTranscodeTopologyBuilder::m_kaContainerMap[] =
{
    { L"MFTranscodeContainerType_ASF", MFTranscodeContainerType_ASF },
    { L"MFTranscodeContainerType_MPEG4", MFTranscodeContainerType_MPEG4 },
};
#define ContainerMapSize (sizeof(m_kaContainerMap) / sizeof(StringGuidMap))

const CTedTranscodeTopologyBuilder::StringGuidMap CTedTranscodeTopologyBuilder::m_kaAudioSubtypeMap[] =
{
    { L"MFAudioFormat_WMAudioV9", MFAudioFormat_WMAudioV9 },
    { L"MFAudioFormat_WMAudioV8", MFAudioFormat_WMAudioV8 },
    { L"MFAudioFormat_WMAudio_Lossless", MFAudioFormat_WMAudio_Lossless },
    { L"MFAudioFormat_MSP1", MFAudioFormat_MSP1 },
    { L"MFAudioFormat_MP3", MFAudioFormat_MP3 },
    { L"MFAudioFormat_PCM", MFAudioFormat_PCM },
    { L"MFAudioFormat_Float", MFAudioFormat_Float },
    { L"MFAudioFormat_AAC", MFAudioFormat_AAC },
};
#define AudioSubtypeMapSize (sizeof(m_kaAudioSubtypeMap) / sizeof(StringGuidMap))

const CTedTranscodeTopologyBuilder::StringGuidMap CTedTranscodeTopologyBuilder::m_kaVideoSubtypeMap[] =
{
    { L"MFVideoFormat_WMV3", MFVideoFormat_WMV3 },
    { L"MFVideoFormat_WMV2", MFVideoFormat_WMV2 },
    { L"MFVideoFormat_WMV1", MFVideoFormat_WMV1 },
    { L"MFVideoFormat_MSS2", MFVideoFormat_MSS2 },
    { L"MFVideoFormat_MSS1", MFVideoFormat_MSS1 },
    { L"MFVideoFormat_MPEG2", MFVideoFormat_MPEG2 },
    { L"MFVideoFormat_M4S2", WMMEDIASUBTYPE_M4S2 },
    { L"MFVideoFormat_H264", MFVideoFormat_H264 },
};
#define VideoSubtypeMapSize (sizeof(m_kaVideoSubtypeMap) / sizeof(StringGuidMap))

const CTedTranscodeTopologyBuilder::StringAttributeMap CTedTranscodeTopologyBuilder::m_kaAudioAttributeMap[] =
{
    { L"AudioFormat", NULL, MF_MT_SUBTYPE, AttributeType_AudioSubtype },
    { L"AudioBitsPerSample", NULL, MF_MT_AUDIO_BITS_PER_SAMPLE, AttributeType_UINT32 },
    { L"AudioSamplesPerSecond", NULL, MF_MT_AUDIO_SAMPLES_PER_SECOND, AttributeType_UINT32 },
    { L"AudioNumChannels", NULL, MF_MT_AUDIO_NUM_CHANNELS, AttributeType_UINT32 },
    { L"AudioBlockAlignment", NULL, MF_MT_AUDIO_BLOCK_ALIGNMENT, AttributeType_UINT32 },
    { L"AudioAvgBytesPerSecond", NULL, MF_MT_AUDIO_AVG_BYTES_PER_SECOND, AttributeType_UINT32 },
    { L"AudioEncodingProfile", NULL, MF_TRANSCODE_ENCODINGPROFILE, AttributeType_String },
};
#define AudioAttributeMapSize (sizeof(m_kaAudioAttributeMap) / sizeof(StringAttributeMap))

const CTedTranscodeTopologyBuilder::StringAttributeMap CTedTranscodeTopologyBuilder::m_kaVideoAttributeMap[] =
{ 
    { L"VideoFormat", NULL, MF_MT_SUBTYPE, AttributeType_VideoSubtype },
    { L"VideoBitrate", NULL, MF_MT_AVG_BITRATE, AttributeType_UINT32 },
    { L"VideoEncodeComplexity", NULL, MF_TRANSCODE_QUALITYVSSPEED, AttributeType_UINT32 },
    { L"VideoFrameWidth", L"VideoFrameHeight", MF_MT_FRAME_SIZE, AttributeType_Ratio },
    { L"VideoFrameRateNumerator", L"VideoFrameRateDenominator", MF_MT_FRAME_RATE, AttributeType_Ratio },
    { L"VideoPixelAspectRatioNumerator", L"VideoPixelAspectRatioDenominator", MF_MT_PIXEL_ASPECT_RATIO, AttributeType_Ratio },
    { L"VideoEncodingProfile", NULL, MF_TRANSCODE_ENCODINGPROFILE, AttributeType_String },
};
#define VideoAttributeMapSize (sizeof(m_kaVideoAttributeMap) / sizeof(StringAttributeMap))

const CTedTranscodeTopologyBuilder::StringAttributeMap CTedTranscodeTopologyBuilder::m_kaContainerAttributeMap[] =
{
    { L"ConatinerType", NULL, MF_TRANSCODE_CONTAINERTYPE, AttributeType_ContainerType },
    { L"SkipMetadataTransfer", NULL, MF_TRANSCODE_SKIP_METADATA_TRANSFER, AttributeType_UINT32 },
};

CTedTranscodeTopologyBuilder::CTedTranscodeTopologyBuilder(LPCWSTR szSource, HRESULT* phr)
{
    HRESULT hr;
    CComPtr<IMFSourceResolver> spResolver;
    CComPtr<IUnknown> spSourceUnk;
    MF_OBJECT_TYPE ObjectType;
    
    IFC( MFCreateSourceResolver(&spResolver) );
    IFC( spResolver->CreateObjectFromURL(szSource, MF_RESOLUTION_MEDIASOURCE,
                                        NULL, &ObjectType, &spSourceUnk) );
    hr = spSourceUnk->QueryInterface(IID_IMFMediaSource, (void**) &m_spSource);
    if(E_NOINTERFACE == hr)
    {
        hr = MF_E_UNSUPPORTED_BYTESTREAM_TYPE;
    }
    IFC( hr );

    IFC( LoadTranscodeProfiles() );

Cleanup:
    *phr = hr;
}

CTedTranscodeTopologyBuilder::~CTedTranscodeTopologyBuilder()
{
}

size_t CTedTranscodeTopologyBuilder::GetProfileCount()
{
    return m_arrProfileNames.GetCount();
}

CAtlStringW CTedTranscodeTopologyBuilder::GetProfileName(size_t iElement)
{
    return m_arrProfileNames[iElement];
}

HRESULT CTedTranscodeTopologyBuilder::BuildTranscodeTopology(const CAtlStringW& strProfileName, LPCWSTR szOutputFilePath, IMFTopology** ppTopology)
{
    size_t iProfile = m_arrProfiles.GetCount();

    for(size_t i = 0; i < m_arrProfileNames.GetCount(); i++)
    {
        if(strProfileName == m_arrProfileNames[i])
        {
            iProfile = i;
            break;
        }
    }

    assert(iProfile < m_arrProfiles.GetCount());
    if(iProfile >= m_arrProfiles.GetCount())
    {
        return MF_E_NOT_FOUND;
    }

    return MFCreateTranscodeTopology(m_spSource, szOutputFilePath, m_arrProfiles[iProfile], ppTopology);
}

HRESULT CTedTranscodeTopologyBuilder::LoadTranscodeProfiles()
{
    HRESULT hr;
    CComPtr<ITedDataLoader> spLoader;
    LPWSTR szNextObject;

    IFC( CoCreateInstance(CLSID_CXMLDataLoader, NULL, CLSCTX_INPROC_SERVER, IID_ITedDataLoader, (void**) &spLoader) );

    IFC( spLoader->LoadFromFile(m_kszTranscodeProfileFile, L"TedTranscodeProfiles") );

    BOOL fHasNext;
    IFC( spLoader->HasNextObject(&fHasNext) );
    while(fHasNext)
    {
        hr = spLoader->GetNextObject(&szNextObject);
        if(S_FALSE == hr)
        {
            hr = S_OK;
            break;
        }
        else
        {
            IFC( hr );
        }

        if(wcscmp(szNextObject, L"TedTranscodeProfile") != 0)
        {
            IFC( E_INVALIDARG );
        }

        IFC( LoadTranscodeProfile(spLoader) );

        IFC( spLoader->HasNextObject(&fHasNext) );
    }
    
Cleanup:
    if(FAILED(hr))
    {
        hr = TED_E_TRANSCODE_PROFILES_FILE_INVALID;
    }

    return hr;
}

HRESULT CTedTranscodeTopologyBuilder::LoadTranscodeProfile(ITedDataLoader* pLoader)
{
    HRESULT hr;
    LPWSTR szData = NULL;
    CComPtr<IMFTranscodeProfile> spTranscodeProfile;
    CComPtr<IMFAttributes> spAudioAttributes;
    UINT32 cAudioAttributes;
    CComPtr<IMFAttributes> spVideoAttributes;
    UINT32 cVideoAttributes;

    IFC( MFCreateTranscodeProfile(&spTranscodeProfile) );

    IFC( pLoader->LoadData(L"ProfileName", &szData, 0) );
    m_arrProfileNames.Add(szData);
    SAFE_COTASKMEMFREE( szData );

    IFC( LoadContainerAttributes(pLoader, spTranscodeProfile) );

    IFC( MFCreateAttributes(&spAudioAttributes, 6) );
    IFC( LoadAttributes(pLoader, spAudioAttributes, m_kaAudioAttributeMap, AudioAttributeMapSize) );
    IFC( spAudioAttributes->GetCount(&cAudioAttributes) );
    if(cAudioAttributes > 0)
    {
        IFC( spAudioAttributes->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio) );
        IFC( MakeCompleteAudioAttributes(spAudioAttributes) );
        IFC( spTranscodeProfile->SetAudioAttributes(spAudioAttributes) );
    }

    IFC( MFCreateAttributes(&spVideoAttributes, 6) );
    IFC( LoadAttributes(pLoader, spVideoAttributes, m_kaVideoAttributeMap, VideoAttributeMapSize) );
    IFC( spVideoAttributes->GetCount(&cVideoAttributes) );
    if(cVideoAttributes > 0)
    {
        IFC( spVideoAttributes->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video) );
        IFC( spTranscodeProfile->SetVideoAttributes(spVideoAttributes) );
    }

    IFC( FillProfileWithSourceType(spTranscodeProfile) );

    m_arrProfiles.Add(spTranscodeProfile);

Cleanup:
    SAFE_COTASKMEMFREE( szData );

    return hr;
}

HRESULT CTedTranscodeTopologyBuilder::MakeCompleteAudioAttributes(IMFAttributes* pAudioAttributes)
{
    HRESULT hr;
    GUID guidSubtype;
    CComPtr<IMFCollection> spTypeCollection;
    DWORD cTypes;
    BOOL fFound = FALSE;
    UINT8* pbExtra = NULL;
    UINT32 cbExtra;

    IFC( pAudioAttributes->GetGUID(MF_MT_SUBTYPE, &guidSubtype) );
    IFC( MFTranscodeGetAudioOutputAvailableTypes(guidSubtype, MFT_ENUM_FLAG_ALL, NULL, &spTypeCollection) );

    IFC( spTypeCollection->GetElementCount(&cTypes) );
    for(DWORD i = 0; i < cTypes; i++)
    {
        CComPtr<IUnknown> spUnkType;
        CComPtr<IMFMediaType> spType;
        BOOL fMatch = FALSE;

        IFC( spTypeCollection->GetElement(i, &spUnkType) );
        IFC( spUnkType->QueryInterface(IID_IMFMediaType, (LPVOID*)&spType) );

        IFC( pAudioAttributes->Compare(spType, MF_ATTRIBUTES_MATCH_OUR_ITEMS, &fMatch) );

        if(fMatch)
        {
            IFC( spType->GetAllocatedBlob(MF_MT_USER_DATA, &pbExtra, &cbExtra) );
            IFC( pAudioAttributes->SetBlob(MF_MT_USER_DATA, pbExtra, cbExtra) );

            fFound = TRUE;
            break;
        }
    }

    if(!fFound)
    {
        IFC( TED_E_INVALID_TRANSCODE_PROFILE )
    }

Cleanup:
    CoTaskMemFree( pbExtra );

    return hr;
}

HRESULT CTedTranscodeTopologyBuilder::LoadContainerAttributes(ITedDataLoader* pLoader, IMFTranscodeProfile* pProfile)
{
    HRESULT hr;
    CComPtr<IMFAttributes> spAttributes;
    LPWSTR szData = NULL;
    GUID gidContainerType;

    IFC( MFCreateAttributes(&spAttributes, 1) );
    
    IFC( pLoader->LoadData(L"ContainerType", &szData, 0) );
    IFC( StringToContainerType(szData, &gidContainerType) );
    IFC( spAttributes->SetGUID(MF_TRANSCODE_CONTAINERTYPE, gidContainerType) );
    SAFE_COTASKMEMFREE( szData );

    hr = pLoader->LoadData(L"SkipMetadataTransfer", &szData, 0);
    if(SUCCEEDED(hr))
    {
        BOOL fSkipMetadata = (0 != _wtoi(szData) );
        IFC( spAttributes->SetUINT32(MF_TRANSCODE_SKIP_METADATA_TRANSFER, fSkipMetadata) );
        SAFE_COTASKMEMFREE( szData );
    }

    IFC( spAttributes->SetUINT32(MF_TRANSCODE_TOPOLOGYMODE, MF_TRANSCODE_TOPOLOGYMODE_HARDWARE_ALLOWED) );
    IFC( pProfile->SetContainerAttributes(spAttributes) );

Cleanup:
    SAFE_COTASKMEMFREE( szData );

    return hr;
}

HRESULT CTedTranscodeTopologyBuilder::LoadAttributes(ITedDataLoader* pLoader, IMFAttributes* pAttributes, const StringAttributeMap* pMap, size_t cMapElements)
{
    HRESULT hr = S_OK;

    for(size_t i = 0; i < cMapElements; i++)
    {
        switch(pMap[i].eAttributeType)
        {
        case AttributeType_AudioSubtype:
            IFC( TryLoadAudioSubtypeAttribute(pLoader, pMap[i].szName, pAttributes, pMap[i].gidAttribute) );
            break;
        case AttributeType_VideoSubtype:
            IFC( TryLoadVideoSubtypeAttribute(pLoader, pMap[i].szName, pAttributes, pMap[i].gidAttribute) );
            break;
        case AttributeType_UINT32:
            IFC( TryLoadUINT32Attribute(pLoader, pMap[i].szName, pAttributes, pMap[i].gidAttribute) );
            break;
        case AttributeType_String:
            IFC( TryLoadStringAttribute(pLoader, pMap[i].szName, pAttributes, pMap[i].gidAttribute) );
            break;
        case AttributeType_Ratio:
            IFC( TryLoadRatioAttribute(pLoader, pMap[i].szName, pMap[i].szSecondName, pAttributes, pMap[i].gidAttribute) );
            break;
        }
    }

Cleanup:
    return hr;
}

HRESULT CTedTranscodeTopologyBuilder::StringToContainerType(LPCWSTR szName, GUID* pgidContainerType)
{
    return FindGuidInMap(szName, pgidContainerType, m_kaContainerMap, ContainerMapSize);
}

HRESULT CTedTranscodeTopologyBuilder::StringToAudioSubtype(LPCWSTR szSubtype, GUID* pgidSubtype)
{
    return FindGuidInMap(szSubtype, pgidSubtype, m_kaAudioSubtypeMap, AudioSubtypeMapSize);
}

HRESULT CTedTranscodeTopologyBuilder::StringToVideoSubtype(LPCWSTR szSubtype, GUID* pgidSubtype)
{
    return FindGuidInMap(szSubtype, pgidSubtype, m_kaVideoSubtypeMap, VideoSubtypeMapSize);
}

HRESULT CTedTranscodeTopologyBuilder::FindGuidInMap(LPCWSTR szName, GUID* pgidValue, const StringGuidMap* pMap, DWORD cMapElements)
{
    HRESULT hr = MF_E_NOT_FOUND;

    for(DWORD i = 0; i < cMapElements; i++)
    {
        if(wcscmp(szName, pMap[i].szName) == 0)
        {
            *pgidValue = pMap[i].gidValue;
            hr = S_OK;
        }
    }

    return hr;
}

HRESULT CTedTranscodeTopologyBuilder::TryLoadUINT32Attribute(ITedDataLoader* pLoader, LPCWSTR szName, IMFAttributes* pAttributes, GUID gidAttributeName)
{
    HRESULT hr;
    LPWSTR szData = NULL;

    hr = pLoader->LoadData(szName, &szData, 0);
    if(SUCCEEDED(hr))
    {
        UINT32 unValue = wcstoul(szData, NULL, 10);
        IFC( pAttributes->SetUINT32(gidAttributeName, unValue) );
    }
    hr = S_OK;

Cleanup:
    SAFE_COTASKMEMFREE( szData );

    return hr;
}

HRESULT CTedTranscodeTopologyBuilder::TryLoadRatioAttribute(ITedDataLoader* pLoader, LPCWSTR szNameFirst, LPCWSTR szNameSecond, IMFAttributes* pAttributes, GUID gidAttributeName)
{
    HRESULT hr;
    LPWSTR szData = NULL;

    hr = pLoader->LoadData(szNameFirst, &szData, 0);
    if(SUCCEEDED(hr))
    {
        UINT32 unFirstValue = wcstoul(szData, NULL, 10);

        SAFE_COTASKMEMFREE( szData );
        IFC( pLoader->LoadData(szNameSecond, &szData, 0) );
        UINT32 unSecondValue = wcstoul(szData, NULL, 10);

        IFC( MFSetAttributeRatio(pAttributes, gidAttributeName, unFirstValue, unSecondValue) );
    }
    hr = S_OK;

Cleanup:
    SAFE_COTASKMEMFREE( szData );

    return hr;
}

HRESULT CTedTranscodeTopologyBuilder::TryLoadStringAttribute(ITedDataLoader* pLoader, LPCWSTR szName, IMFAttributes* pAttributes, GUID gidAttributeName)
{
    HRESULT hr;
    LPWSTR szData = NULL;

    hr = pLoader->LoadData(szName, &szData, 0);
    if(SUCCEEDED(hr))
    {
        IFC( pAttributes->SetString(gidAttributeName, szData) );
    }
    hr = S_OK;

Cleanup:
    SAFE_COTASKMEMFREE( szData );

    return hr;
}

HRESULT CTedTranscodeTopologyBuilder::TryLoadAudioSubtypeAttribute(ITedDataLoader* pLoader, LPCWSTR szName, IMFAttributes* pAttributes, GUID gidAttributeName)
{
    HRESULT hr;
    LPWSTR szData = NULL;

    hr = pLoader->LoadData(szName, &szData, 0);
    if(SUCCEEDED(hr))
    {
        GUID gidSubtype;
        IFC( StringToAudioSubtype(szData, &gidSubtype) );
        IFC( pAttributes->SetGUID(gidAttributeName, gidSubtype) );
    }
    hr = S_OK;

Cleanup:
    SAFE_COTASKMEMFREE( szData );

    return hr;
}

HRESULT CTedTranscodeTopologyBuilder::TryLoadVideoSubtypeAttribute(ITedDataLoader* pLoader, LPCWSTR szName, IMFAttributes* pAttributes, GUID gidAttributeName)
{
    HRESULT hr;
    LPWSTR szData = NULL;

    hr = pLoader->LoadData(szName, &szData, 0);
    if(SUCCEEDED(hr))
    {
        GUID gidSubtype;
        IFC( StringToVideoSubtype(szData, &gidSubtype) );
        IFC( pAttributes->SetGUID(gidAttributeName, gidSubtype) );
    }
    hr = S_OK;

Cleanup:
    SAFE_COTASKMEMFREE( szData );

    return hr;
}

HRESULT CTedTranscodeTopologyBuilder::FillProfileWithSourceType(IMFTranscodeProfile* pProfile)
{
    HRESULT hr;
    CComPtr<IMFAttributes> spAudioAttributes;
    CComPtr<IMFAttributes> spVideoAttributes;

    IFC( pProfile->GetAudioAttributes(&spAudioAttributes) );
    if(spAudioAttributes)
    {
        CComPtr<IMFMediaType> spmtAudio;

        IFC( GetSourceMediaType(MFMediaType_Audio, &spmtAudio) );

        if(spmtAudio)
        {
            IFC( CopyDesiredAttributes(spmtAudio, spAudioAttributes, m_kaAudioAttributeMap, AudioAttributeMapSize) );
        }
    }

    IFC( pProfile->GetVideoAttributes(&spVideoAttributes) );
    if(spVideoAttributes)
    {
        CComPtr<IMFMediaType> spmtVideo;

        IFC( GetSourceMediaType(MFMediaType_Video, &spmtVideo) );

        if(spmtVideo)
        {
            IFC( CopyDesiredAttributes(spmtVideo, spVideoAttributes, m_kaVideoAttributeMap, VideoAttributeMapSize) );
        }
    }

Cleanup:
    return hr;
}

HRESULT CTedTranscodeTopologyBuilder::GetSourceMediaType(REFGUID gidMajorType, IMFMediaType** ppMediaType)
{
    HRESULT hr;
    CComPtr<IMFPresentationDescriptor> spPD;
    DWORD cSDs;

    IFC( m_spSource->CreatePresentationDescriptor(&spPD) );
    IFC( spPD->GetStreamDescriptorCount(&cSDs) );
    
    for(DWORD i = 0; i < cSDs; i++)
    {
        CComPtr<IMFStreamDescriptor> spSD;
        CComPtr<IMFMediaTypeHandler> spMTH;
        GUID gidMTHMajorType;
        BOOL fSelected;

        IFC( spPD->GetStreamDescriptorByIndex(i, &fSelected, &spSD) );
        IFC( spSD->GetMediaTypeHandler(&spMTH) );

        IFC( spMTH->GetMajorType(&gidMTHMajorType) );
        if(gidMTHMajorType == gidMajorType)
        {
            IFC( spMTH->GetCurrentMediaType(ppMediaType) );
        }
    }

Cleanup:
    return hr;
}

HRESULT CTedTranscodeTopologyBuilder::CopyDesiredAttributes(IMFMediaType* pSourceType, IMFAttributes* pTargetAttributes, const StringAttributeMap* pMap, size_t cMapElements)
{
    HRESULT hr = S_OK;

    for(size_t i = 0; i < cMapElements; i++)
    {
        PROPVARIANT varValue;
        PropVariantInit(&varValue);

        hr = pSourceType->GetItem(pMap[i].gidAttribute, &varValue);
        if(SUCCEEDED(hr))
        {
            if(FAILED(pTargetAttributes->GetItem(pMap[i].gidAttribute, NULL)))
            {
                pTargetAttributes->SetItem(pMap[i].gidAttribute, varValue);
            }
        }

        IFC( PropVariantClear(&varValue) );
    }

Cleanup:
    return hr;
}