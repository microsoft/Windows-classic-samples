// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#ifndef __TEDTRANSCODE_H
#define __TEDTRANSCODE_H

#include "tedutil.h"

class CTedTranscodeTopologyBuilder
{
public:
    CTedTranscodeTopologyBuilder(LPCWSTR szSource, HRESULT* phr);
    ~CTedTranscodeTopologyBuilder();

    size_t GetProfileCount();
    CAtlStringW GetProfileName(size_t iElement);

    HRESULT BuildTranscodeTopology(const CAtlStringW& strProfileName, LPCWSTR szOutputFilePath, IMFTopology** ppTopology);

protected:
    struct StringGuidMap
    {
        LPCWSTR szName;
        GUID gidValue;
    };
    
    enum EAttributeType
    {
        AttributeType_UINT32,
        AttributeType_String,
        AttributeType_Ratio,
        AttributeType_AudioSubtype,
        AttributeType_VideoSubtype,
        AttributeType_ContainerType,
    };
    struct StringAttributeMap
    {
        LPCWSTR szName;
        LPCWSTR szSecondName;
        GUID gidAttribute;
        EAttributeType eAttributeType;
    };

    HRESULT LoadTranscodeProfiles();
    HRESULT LoadTranscodeProfile(ITedDataLoader* pLoader);
    
    HRESULT MakeCompleteAudioAttributes(IMFAttributes* pAudioAttributes);

    // String -> Guid conversion functions
    HRESULT StringToContainerType(LPCWSTR szName, GUID* pgidContainerType);
    HRESULT StringToAudioSubtype(LPCWSTR szSubtype, GUID* pgidSubtype);
    HRESULT StringToVideoSubtype(LPCWSTR szSubtype, GUID* pgidSubtype);
    HRESULT StringToAMFormatType(LPCWSTR szSubtype, GUID* pgidSubtype);
    HRESULT FindGuidInMap(LPCWSTR szName, GUID* pgidValue, const StringGuidMap* pMap, DWORD cMapElements);

    // Attribute loading functions
    HRESULT LoadContainerAttributes(ITedDataLoader* pLoader, IMFTranscodeProfile* pProfile);
    HRESULT LoadAttributes(ITedDataLoader* pLoader, IMFAttributes* pAttributes, const StringAttributeMap* pMap, size_t cMapElements);
    HRESULT TryLoadUINT32Attribute(ITedDataLoader* pLoader, LPCWSTR szName, IMFAttributes* pAttributes, GUID gidAttributeName);
    HRESULT TryLoadRatioAttribute(ITedDataLoader* pLoader, LPCWSTR szNameFirst, LPCWSTR szNameSecond, IMFAttributes* pAttributes, GUID gidAttributeName);
    HRESULT TryLoadStringAttribute(ITedDataLoader* pLoader, LPCWSTR szName, IMFAttributes* pAttributes, GUID gidAttributeName);
    HRESULT TryLoadAudioSubtypeAttribute(ITedDataLoader* pLoader, LPCWSTR szName, IMFAttributes* pAttributes, GUID gidAttributeName);
    HRESULT TryLoadVideoSubtypeAttribute(ITedDataLoader* pLoader, LPCWSTR szName, IMFAttributes* pAttributes, GUID gidAttributeName);

    // Source attribute copying
    HRESULT FillProfileWithSourceType(IMFTranscodeProfile* pProfile);
    HRESULT GetSourceMediaType(REFGUID gidMajorType, IMFMediaType** ppMediaType);
    HRESULT CopyDesiredAttributes(IMFMediaType* pSourceType, IMFAttributes* pTargetAttributes, const StringAttributeMap* pMap, size_t cMapElements);

private:
    const static LPCWSTR m_kszTranscodeProfileFile;


    const static StringGuidMap m_kaAudioSubtypeMap[];
    const static StringGuidMap m_kaVideoSubtypeMap[];
    const static StringGuidMap m_kaContainerMap[];

    const static StringAttributeMap m_kaContainerAttributeMap[];
    const static StringAttributeMap m_kaAudioAttributeMap[];
    const static StringAttributeMap m_kaVideoAttributeMap[];

    CAtlArray<CAtlStringW> m_arrProfileNames;
    CInterfaceArray<IMFTranscodeProfile> m_arrProfiles;

    CComPtr<IMFMediaSource> m_spSource;
};

#endif