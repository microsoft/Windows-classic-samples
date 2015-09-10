// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

// SampleSpellCheckProvider.h : Declaration of the sample provider

#pragma once
#include <SpellCheckProvider.h>
#include "SampleSpellChecker.h"
#include <atlbase.h>
#include <atlcom.h>
#include "resource.h"
#include "sampleengine.h"

class ATL_NO_VTABLE CSampleSpellCheckProvider 
    : public ISpellCheckProvider
    , public ATL::CComCoClass<CSampleSpellCheckProvider, &CLSID_SampleSpellCheckProvider> // ATL implementation for CreateInstance, etc...
    , public ATL::CComObjectRootEx<ATL::CComMultiThreadModelNoCS> // ATL implementation for IUnknown
{
// ISpellCheckProvider
public:
    IFACEMETHOD(get_LanguageTag)(_Out_ PWSTR* value);
    IFACEMETHOD(get_Id)(_Out_ PWSTR* value);
    IFACEMETHOD(get_LocalizedName)(_Out_ PWSTR* value);
    IFACEMETHOD(Check)(_In_ PCWSTR text, _COM_Outptr_ IEnumSpellingError** value);
    IFACEMETHOD(Suggest)(_In_ PCWSTR word, _COM_Outptr_ IEnumString** value);
    IFACEMETHOD(GetOptionValue)(_In_ PCWSTR optionId, _Out_ BYTE* value);
    IFACEMETHOD(SetOptionValue)(_In_ PCWSTR optionId, BYTE value);
    IFACEMETHOD(get_OptionIds)(_COM_Outptr_ IEnumString** value);
    IFACEMETHOD(InitializeWordlist)(WORDLIST_TYPE wordlistType, _In_ IEnumString* words);
    IFACEMETHOD(GetOptionDescription)(_In_ PCWSTR optionId, _COM_Outptr_ IOptionDescription** value);

public:
    static HRESULT CreateInstance(_In_ PCWSTR languageTag, _COM_Outptr_ CSampleSpellCheckProvider** spellProvider);
    HRESULT EngineCheck(_In_ PCWSTR text, _Out_ SampleEngine::SpellingError* spellingError);

public:
    DECLARE_REGISTRY_RESOURCEID(IDR_SPELLCHECKPROVIDER)

    BEGIN_COM_MAP(CSampleSpellCheckProvider)
        COM_INTERFACE_ENTRY(ISpellCheckProvider)
    END_COM_MAP()

    DECLARE_NOT_AGGREGATABLE(CSampleSpellCheckProvider)

private:
    HRESULT Init(_In_ PCWSTR languageTag);

private:
    wchar_t _languageTag[MAX_PATH];
    SampleEngine engine;
};

OBJECT_ENTRY_NON_CREATEABLE_EX_AUTO(__uuidof(SampleSpellCheckProvider), CSampleSpellCheckProvider)
