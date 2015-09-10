// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

// SampleSpellCheckProviderFactory.h : Declaration of the sample provider factory

#pragma once
#include <SpellCheckProvider.h>
#include "SampleSpellChecker.h"
#include <atlbase.h>
#include <atlcom.h>
#include "resource.h"

class ATL_NO_VTABLE CSampleSpellCheckProviderFactory
    : public ISpellCheckProviderFactory
    , public ATL::CComCoClass<CSampleSpellCheckProviderFactory, &CLSID_SampleSpellCheckProviderFactory> // ATL implementation for CreateInstance, etc...
    , public ATL::CComObjectRootEx<ATL::CComMultiThreadModelNoCS> // ATL implementation for IUnknown
{
// ISpellCheckProviderFactory
public:
    IFACEMETHOD(get_SupportedLanguages)(_COM_Outptr_ IEnumString** value);
    IFACEMETHOD(IsSupported)(_In_ PCWSTR languageTag, _Out_ BOOL* value);
    IFACEMETHOD(CreateSpellCheckProvider)(_In_ PCWSTR languageTag, _COM_Outptr_ ISpellCheckProvider** value);

public:
    DECLARE_REGISTRY_RESOURCEID(IDR_SPELLCHECKPROVIDERFACTORY)

    BEGIN_COM_MAP(CSampleSpellCheckProviderFactory)
        COM_INTERFACE_ENTRY(ISpellCheckProviderFactory)
    END_COM_MAP()

    DECLARE_NOT_AGGREGATABLE(CSampleSpellCheckProviderFactory)
};

OBJECT_ENTRY_AUTO(__uuidof(SampleSpellCheckProviderFactory), CSampleSpellCheckProviderFactory)
