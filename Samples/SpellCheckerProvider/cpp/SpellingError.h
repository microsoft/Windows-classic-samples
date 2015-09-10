// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

// SpellingError.h : Declaration of the spelling error class

#pragma once
#include <SpellCheck.h>
#include "SampleSpellChecker.h"
#include <atlbase.h>
#include <atlcom.h>
#include "resource.h"

class ATL_NO_VTABLE CSpellingError
    : public ISpellingError
    , public ATL::CComCoClass<CSpellingError, &CLSID_SpellingError> // ATL implementation for CreateInstance, etc...
    , public ATL::CComObjectRootEx<ATL::CComMultiThreadModelNoCS> // ATL implementation for IUnknown
{
public:

    virtual ~CSpellingError();

    IFACEMETHOD(get_StartIndex)(_Out_ ULONG* value);
    IFACEMETHOD(get_Length)(_Out_ ULONG* value);
    IFACEMETHOD(get_CorrectiveAction)(_Out_ CORRECTIVE_ACTION* value);
    IFACEMETHOD(get_Replacement)(_Out_ PWSTR* value);

    static HRESULT CreateInstance(_In_ ULONG startIndex, _In_ ULONG errorLength, _In_ CORRECTIVE_ACTION correctiveAction, _In_ PCWSTR replacement, _COM_Outptr_ CSpellingError** spellingError);

public:
    DECLARE_REGISTRY_RESOURCEID(IDR_SPELLINGERROR)
    
    BEGIN_COM_MAP(CSpellingError)
        COM_INTERFACE_ENTRY(ISpellingError)
    END_COM_MAP()

    DECLARE_NOT_AGGREGATABLE(CSpellingError)

private:
    HRESULT Init(_In_ ULONG startIndex, _In_ ULONG errorLength, _In_ CORRECTIVE_ACTION correctiveAction, _In_ PCWSTR replacement);

private:
    ULONG _startIndex;
    ULONG _errorLength;
    CORRECTIVE_ACTION _correctiveAction;
    PWSTR _replacement;
};

OBJECT_ENTRY_NON_CREATEABLE_EX_AUTO (__uuidof(SpellingError), CSpellingError)
