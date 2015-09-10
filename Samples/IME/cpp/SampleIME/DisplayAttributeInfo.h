// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved


#pragma once

//+---------------------------------------------------------------------------
//
// CDisplayAttributeInfo class
//
//----------------------------------------------------------------------------

class CDisplayAttributeInfo : public ITfDisplayAttributeInfo
{
public:
    CDisplayAttributeInfo();
    ~CDisplayAttributeInfo();

    // IUnknown
    STDMETHODIMP QueryInterface(REFIID riid, _Outptr_ void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

    // ITfDisplayAttributeInfo
    STDMETHODIMP GetGUID(_Out_ GUID *pguid);
    STDMETHODIMP GetDescription(_Out_ BSTR *pbstrDesc);
    STDMETHODIMP GetAttributeInfo(_Out_ TF_DISPLAYATTRIBUTE *pTSFDisplayAttr);
    STDMETHODIMP SetAttributeInfo(_In_ const TF_DISPLAYATTRIBUTE *ptfDisplayAttr);
    STDMETHODIMP Reset();

protected:
    const GUID* _pguid;
    const TF_DISPLAYATTRIBUTE* _pDisplayAttribute;
    const WCHAR* _pDescription;
    const WCHAR* _pValueName;

private:
    LONG _refCount; // COM ref count
};

//+---------------------------------------------------------------------------
//
// CDisplayAttributeInfoInput class
//
//----------------------------------------------------------------------------

class CDisplayAttributeInfoInput : public CDisplayAttributeInfo
{
public:
    CDisplayAttributeInfoInput()
    {
        _pguid = &Global::SampleIMEGuidDisplayAttributeInput;
        _pDisplayAttribute = &_s_DisplayAttribute;
        _pDescription = _s_szDescription;
        _pValueName = _s_szValueName;
    }

    static const TF_DISPLAYATTRIBUTE _s_DisplayAttribute;
    static const WCHAR _s_szDescription[];
    static const WCHAR _s_szValueName[];
};

//+---------------------------------------------------------------------------
//
// CDisplayAttributeInfoConverted class
//
//----------------------------------------------------------------------------

class CDisplayAttributeInfoConverted : public CDisplayAttributeInfo
{
public:
    CDisplayAttributeInfoConverted()
    {
        _pguid = &Global::SampleIMEGuidDisplayAttributeConverted;
        _pDisplayAttribute = &_s_DisplayAttribute;
        _pDescription = _s_szDescription;
        _pValueName = _s_szValueName;
    }

    static const TF_DISPLAYATTRIBUTE _s_DisplayAttribute;
    static const WCHAR _s_szDescription[];
    static const WCHAR _s_szValueName[];
};
