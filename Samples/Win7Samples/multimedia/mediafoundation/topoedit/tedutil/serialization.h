// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

#include <msxml6.h>

#include "tedutil.h"
#include "resource.h"

class CXMLDataSaver
    : public IDispatchImpl<ITedDataSaver, &IID_ITedDataSaver, &LIBID_TedUtil>
    , public CComObjectRoot
    , public CComCoClass<CXMLDataSaver, &CLSID_CXMLDataSaver>
{
public:
    CXMLDataSaver();
    virtual ~CXMLDataSaver();

    BEGIN_COM_MAP(CXMLDataSaver)
        COM_INTERFACE_ENTRY(ITedDataSaver)
        COM_INTERFACE_ENTRY(IDispatch)
    END_COM_MAP()

    
    DECLARE_REGISTRY_RESOURCEID(IDR_XMLDATASAVER);
    DECLARE_CLASSFACTORY();
    
    HRESULT STDMETHODCALLTYPE Init(LPCWSTR docName);
    HRESULT STDMETHODCALLTYPE BeginSaveObject(LPCWSTR strName);
    HRESULT STDMETHODCALLTYPE BeginSaveChildObjects();
    HRESULT STDMETHODCALLTYPE EndSaveChildObjects();
    HRESULT STDMETHODCALLTYPE SaveData(LPCWSTR strName, LPCWSTR strValue);
    HRESULT STDMETHODCALLTYPE SaveToFile(LPCWSTR fileName);
    
protected:
    typedef struct _SaveContext
    {
        CComPtr<IXMLDOMNode> m_spRootNode;
        CComPtr<IXMLDOMNode> m_spCurrObjNode;
    } SaveContext;
    
private:
    CComPtr<IXMLDOMDocument2> m_spXMLDoc;
    CComPtr<IXMLDOMNode> m_spRootNode;
    CComPtr<IXMLDOMNode> m_spCurrObjNode;

    CAtlArray<SaveContext> m_arrSaveContexts;
};

class CXMLDataLoader 
    : public IDispatchImpl<ITedDataLoader, &IID_ITedDataLoader, &LIBID_TedUtil>
    , public CComObjectRoot
    , public CComCoClass<CXMLDataLoader, &CLSID_CXMLDataLoader>
{
public:
    CXMLDataLoader();
    ~CXMLDataLoader();

    BEGIN_COM_MAP(CXMLDataLoader)
        COM_INTERFACE_ENTRY(ITedDataLoader)
        COM_INTERFACE_ENTRY(IDispatch)
    END_COM_MAP()

    DECLARE_REGISTRY_RESOURCEID(IDR_XMLDATALOADER);
    DECLARE_CLASSFACTORY();
    
    HRESULT STDMETHODCALLTYPE HasNextObject(BOOL* pfHasNextObject);
    HRESULT STDMETHODCALLTYPE GetNextObject(__out LPWSTR* strName);
    HRESULT STDMETHODCALLTYPE HasChildObjects(BOOL* pfHasChildObjects);
    HRESULT STDMETHODCALLTYPE BeginLoadChildObjects();
    HRESULT STDMETHODCALLTYPE EndLoadChildObjects();
    HRESULT STDMETHODCALLTYPE LoadData(LPCWSTR strName, __out LPWSTR* strValue, long nIndex = 0);
    HRESULT STDMETHODCALLTYPE LoadFromFile(LPCWSTR fileName, LPCWSTR docName);
    
    const static HRESULT E_NODE_NOT_FOUND;
    const static HRESULT E_STRING_TOO_LONG;

protected:
    HRESULT LoadNextObject();

    typedef struct _LoadContext
    {
        CComPtr<IXMLDOMNodeList> m_spObjectList;
        CComPtr<IXMLDOMNode> m_spCurrObjNode;
        CComPtr<IXMLDOMNode> m_spNextObjNode;
    } LoadContext;
    
private:
    CComPtr<IXMLDOMDocument2> m_spXMLDoc;
    CComPtr<IXMLDOMNode> m_spRootNode;
    CComPtr<IXMLDOMNodeList> m_spObjectList;
    CComPtr<IXMLDOMNode> m_spCurrObjNode;
    CComPtr<IXMLDOMNode> m_spNextObjNode;
    CAtlArray<LoadContext> m_arrLoadContexts;
    const static UINT MAX_STRING_LENGTH;
};
