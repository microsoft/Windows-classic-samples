// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

// EnumSpellingError.h : Declaration of the spelling error enumerator class

#pragma once
#include <SpellCheck.h>
#include "SampleSpellChecker.h"
#include <atlbase.h>
#include <atlcom.h>
#include "resource.h"
#include "SampleSpellCheckProvider.h"

class ATL_NO_VTABLE CEnumSpellingError
    : public IEnumSpellingError
    , public ATL::CComCoClass<CEnumSpellingError, &CLSID_EnumSpellingError> // ATL implementation for CreateInstance, etc...
    , public ATL::CComObjectRootEx<ATL::CComMultiThreadModelNoCS> // ATL implementation for IUnknown
{
// IEnumSpellingError
public:
    _Success_(return == S_OK)
    IFACEMETHOD(Next)(_COM_Outptr_ ISpellingError** value);

public:
    static HRESULT CreateInstance(_In_ PCWSTR text, _In_ CSampleSpellCheckProvider* spellcheckProvider, _COM_Outptr_ CEnumSpellingError** enumSpellingError);

    virtual ~CEnumSpellingError();
public:
    DECLARE_REGISTRY_RESOURCEID(IDR_ENUMSPELLINGERROR)
    
    BEGIN_COM_MAP(CEnumSpellingError)
        COM_INTERFACE_ENTRY(IEnumSpellingError)
    END_COM_MAP()

    DECLARE_NOT_AGGREGATABLE(CEnumSpellingError)

private:
    HRESULT Init(_In_ PCWSTR text, _In_ CSampleSpellCheckProvider* spellcheckProvider);

private:
    CSampleSpellCheckProvider* _spellcheckProvider;
    PWSTR _text;
    PWSTR _currentTextPosition;
};

OBJECT_ENTRY_NON_CREATEABLE_EX_AUTO(__uuidof(EnumSpellingError), CEnumSpellingError)
