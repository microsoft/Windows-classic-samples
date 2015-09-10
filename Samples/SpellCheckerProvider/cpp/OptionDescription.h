// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

// OptionDescription.h : Declaration of the option description class

#pragma once
#include <SpellCheck.h>
#include "SampleSpellChecker.h"
#include <atlbase.h>
#include <atlcom.h>
#include "resource.h"

class ATL_NO_VTABLE COptionDescription
    : public IOptionDescription
    , public ATL::CComCoClass<COptionDescription, &CLSID_OptionDescription> // ATL implementation for CreateInstance, etc...
    , public ATL::CComObjectRootEx<ATL::CComMultiThreadModelNoCS> // ATL implementation for IUnknown
{
public:
    IFACEMETHOD(get_Id)(_Out_ PWSTR* value);
    IFACEMETHOD(get_Heading)(_Out_ PWSTR* value);
    IFACEMETHOD(get_Description)(_Out_ PWSTR* value);
    IFACEMETHOD(get_Labels)(_COM_Outptr_ IEnumString** value);

    static HRESULT CreateInstance(_In_ PCWSTR optionId, _COM_Outptr_ COptionDescription** option);

public:
    DECLARE_REGISTRY_RESOURCEID(IDR_OPTIONDESCRIPTION)

    BEGIN_COM_MAP(COptionDescription)
        COM_INTERFACE_ENTRY(IOptionDescription)
    END_COM_MAP()
    
    DECLARE_NOT_AGGREGATABLE(COptionDescription)

private:
    HRESULT Init(_In_ PCWSTR optionId);

private:
    wchar_t _optionId[MAX_PATH];
};

OBJECT_ENTRY_NON_CREATEABLE_EX_AUTO (__uuidof(OptionDescription), COptionDescription)
