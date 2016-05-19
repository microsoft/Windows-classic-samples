// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

#include "tedutil.h"

class CPropertyController;
class CPropertyListView;

struct KeyStringPair
{
    GUID m_key;
    CAtlStringW m_str;
};

struct KeyStringTypeTriplet
{
    GUID m_key;
    CAtlStringW m_str;
    VARTYPE m_vt;
    TED_ATTRIBUTE_CATEGORY m_category;
};

class CPropertyInfo : public ITedPropertyInfo
{
public:
    CPropertyInfo();
    virtual ~CPropertyInfo();
    
    virtual HRESULT STDMETHODCALLTYPE GetPropertyInfoName(__out LPWSTR* szName, __out TED_ATTRIBUTE_CATEGORY* pCategory) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetPropertyCount(DWORD* pdwCount) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetProperty(DWORD dwIndex, __out LPWSTR* strName, __out LPWSTR* strValue) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetPropertyType(DWORD dwIndex, __out VARTYPE* vt)  = 0;
    virtual HRESULT STDMETHODCALLTYPE IsWriteable() { return S_OK; }
    
    virtual HRESULT STDMETHODCALLTYPE SetProperty(DWORD dwIndex, __in LPCWSTR strName, VARTYPE vt, __in LPCWSTR strValue) = 0;

    static KeyStringTypeTriplet ms_AttributeKeyStrings[];
    static KeyStringPair ms_AttributeValueStrings[];
    
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppInterface);
    ULONG STDMETHODCALLTYPE AddRef();
    ULONG STDMETHODCALLTYPE Release();

protected:
    void ConvertKeyToString(GUID key, /* out */ CAtlStringW& strName);
    void ConvertPropertyValueToString(GUID key, PROPVARIANT propVal, /* out */ CAtlStringW& strValue);

    void ConvertStringToKey(CAtlStringW strName, /* out */ GUID& key);
    void ConvertStringToPropertyValue(GUID key, CAtlStringW strvalue, VARTYPE vt, /* out */ PROPVARIANT& propVal);

private:
    LONG m_cRef;
};

class CNodePropertyInfo : public CPropertyInfo
{
public:
    CNodePropertyInfo(CComPtr<IMFTopologyNode> spNode);
    virtual ~CNodePropertyInfo();

    virtual HRESULT STDMETHODCALLTYPE GetPropertyInfoName(__out LPWSTR* szName, __out TED_ATTRIBUTE_CATEGORY* pCategory);
    virtual HRESULT STDMETHODCALLTYPE GetPropertyCount(DWORD* pdwCount);
    virtual HRESULT STDMETHODCALLTYPE GetProperty(DWORD dwIndex, __out LPWSTR* strName, __out LPWSTR* strValue);
    virtual HRESULT STDMETHODCALLTYPE GetPropertyType(DWORD dwIndex, __out VARTYPE* vt);
    
    virtual HRESULT STDMETHODCALLTYPE SetProperty(DWORD dwIndex, __in LPCWSTR strName, VARTYPE vt, __in LPCWSTR strValue);
    
private:
    CComPtr<IMFTopologyNode> m_spNode;
    CComPtr<IPropertyStore> m_spNodePropertyStore;
    CComPtr<IMFAttributes> m_spNodeAttributes;
};

class CConnectionPropertyInfo : public CPropertyInfo
{
public:
    CConnectionPropertyInfo(CComPtr<IMFMediaType> spUpstreamType, CComPtr<IMFMediaType> spDownstreamType);
    virtual ~CConnectionPropertyInfo();

    virtual HRESULT STDMETHODCALLTYPE GetPropertyInfoName(__out LPWSTR* szName, __out TED_ATTRIBUTE_CATEGORY* pCategory);
    virtual HRESULT STDMETHODCALLTYPE GetPropertyCount(DWORD* pdwCount);
    virtual HRESULT STDMETHODCALLTYPE GetProperty(DWORD dwIndex, __out LPWSTR* strName, __out LPWSTR* strValue);
    virtual HRESULT STDMETHODCALLTYPE GetPropertyType(DWORD dwIndex, __out VARTYPE* vt);
    
    virtual HRESULT STDMETHODCALLTYPE SetProperty(DWORD dwIndex, __in LPCWSTR strName, VARTYPE vt, __in LPCWSTR strValue);
    
private:
    CComPtr<IMFMediaType> m_spUpstreamType;
    CComPtr<IMFMediaType> m_spDownstreamType;
};

class CAttributesPropertyInfo : public CPropertyInfo
{
public:
    CAttributesPropertyInfo(CComPtr<IMFAttributes> spAttributes, CAtlString strName, TED_ATTRIBUTE_CATEGORY Category);
    virtual ~CAttributesPropertyInfo();
    
    virtual HRESULT STDMETHODCALLTYPE GetPropertyInfoName(__out LPWSTR* szName, __out TED_ATTRIBUTE_CATEGORY* pCategory);
    virtual HRESULT STDMETHODCALLTYPE GetPropertyCount(DWORD* pdwCount);
    virtual HRESULT STDMETHODCALLTYPE GetProperty(DWORD dwIndex, __out LPWSTR* strName, __out LPWSTR* strValue);
    virtual HRESULT STDMETHODCALLTYPE GetPropertyType(DWORD dwIndex, __out VARTYPE* vt);
    
    virtual HRESULT STDMETHODCALLTYPE SetProperty(DWORD dwIndex, __in LPCWSTR strName, VARTYPE vt, __in LPCWSTR strValue);
    
private:
    CComPtr<IMFAttributes> m_spAttributes;
    CAtlString m_strName;
    TED_ATTRIBUTE_CATEGORY m_Category;
};

class COTAPropertyInfo : public CPropertyInfo
{
public:
    COTAPropertyInfo(CComPtr<IMFOutputTrustAuthority>* arrOTA, DWORD cOTACount);
    virtual ~COTAPropertyInfo();
    
    virtual HRESULT STDMETHODCALLTYPE GetPropertyInfoName(__out LPWSTR* szName, __out TED_ATTRIBUTE_CATEGORY* pCategory);
    virtual HRESULT STDMETHODCALLTYPE GetPropertyCount(DWORD* pdwCount);
    virtual HRESULT STDMETHODCALLTYPE GetProperty(DWORD dwIndex, __out LPWSTR* strName, __out LPWSTR* strValue);
    virtual HRESULT STDMETHODCALLTYPE GetPropertyType(DWORD dwIndex, __out VARTYPE* vt);
    virtual HRESULT STDMETHODCALLTYPE IsWriteable() { return S_FALSE; }
    
    virtual HRESULT STDMETHODCALLTYPE SetProperty(DWORD dwIndex, __in LPCWSTR strName, VARTYPE vt, __in LPCWSTR strValue);

private:
    CComPtr<IMFOutputTrustAuthority>* m_arrOTA;
    DWORD m_cOTACount;
};