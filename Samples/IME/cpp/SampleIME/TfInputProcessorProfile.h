// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

class CTfInputProcessorProfile
{
public:
    CTfInputProcessorProfile();
    ~CTfInputProcessorProfile();

    HRESULT CreateInstance();
    HRESULT GetCurrentLanguage(_Out_ LANGID *plangid);
    HRESULT GetDefaultLanguageProfile(LANGID langid, REFGUID catid, _Out_ CLSID *pclsid, _Out_ GUID *pguidProfile);

private:
    ITfInputProcessorProfiles* _pInputProcessorProfile;
};
