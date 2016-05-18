// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "stdafx.h"

#include "serialization.h"

#include <assert.h>
#include <string.h>
#include <mferror.h>
#include <strsafe.h>
#include <intsafe.h>


//////////////////////////////////////////////////////////////////////////////
//

CXMLDataSaver::CXMLDataSaver()
{
}

CXMLDataSaver::~CXMLDataSaver()
{
}

HRESULT CXMLDataSaver::Init(LPCWSTR docName) 
{
    HRESULT hr = S_OK;
    CComPtr<IXMLDOMNode> pXDN;
    CComPtr<IXMLDOMNode> tempNode;

    m_spXMLDoc.Release();
    IFC( CoCreateInstance(__uuidof(DOMDocument60), NULL, CLSCTX_INPROC_SERVER, __uuidof(IXMLDOMDocument2), (void**)&m_spXMLDoc) );
    IFC( m_spXMLDoc->QueryInterface(IID_IXMLDOMNode, (void **)&pXDN) );

    // Create root node
    m_spRootNode.Release();
    VARIANT var;
    var.intVal = NODE_ELEMENT;
    var.vt = VT_INT;
    IFC( m_spXMLDoc->createNode(var, CComBSTR(docName), NULL, &tempNode) );
    IFC( pXDN->appendChild(tempNode, &m_spRootNode) );

Cleanup:
    assert(!FAILED(hr));
    return hr;
}

HRESULT CXMLDataSaver::BeginSaveObject(LPCWSTR name)
{
    HRESULT hr = S_OK;

    m_spCurrObjNode.Release();
    CComVariant var;
    var.intVal = NODE_ELEMENT;
    var.vt = VT_INT;
    CComPtr<IXMLDOMNode> tempNode;
    IFC( m_spXMLDoc->createNode(var, CComBSTR(name), NULL, &tempNode) );
    IFC( m_spRootNode->appendChild(tempNode, &m_spCurrObjNode) );

Cleanup:
    assert(!FAILED(hr));
    return hr;
}

HRESULT CXMLDataSaver::BeginSaveChildObjects()
{
    SaveContext SCtx;

    SCtx.m_spRootNode = m_spRootNode;
    SCtx.m_spCurrObjNode = m_spCurrObjNode;
    m_arrSaveContexts.Add(SCtx);

    m_spRootNode = m_spCurrObjNode;

    return S_OK;
}

HRESULT CXMLDataSaver::EndSaveChildObjects()
{
    if(!m_arrSaveContexts.IsEmpty())
    {
        size_t nLastNode = m_arrSaveContexts.GetCount() - 1;

        SaveContext SCtx = m_arrSaveContexts.GetAt(nLastNode);
        m_arrSaveContexts.RemoveAt(nLastNode);

        m_spRootNode = SCtx.m_spRootNode;
        m_spCurrObjNode = SCtx.m_spCurrObjNode;
    }

    return S_OK;
}

HRESULT CXMLDataSaver::SaveData(LPCWSTR strName, LPCWSTR strValue)
{
    HRESULT hr = S_OK;

    CComPtr<IXMLDOMNode> tempNode;
    CComPtr<IXMLDOMNode> propertyNode;
    CComPtr<IXMLDOMElement> propertyElement;

    IFC( m_spXMLDoc->createNode(CComVariant(NODE_ELEMENT), CComBSTR(strName), NULL, &tempNode) );
    IFC( m_spCurrObjNode->appendChild(tempNode, &propertyNode) );

    IFC( propertyNode->QueryInterface(IID_IXMLDOMElement, (void**) &propertyElement) );

    IFC( propertyElement->setAttribute(CComBSTR(L"Value"), CComVariant(strValue)) );

Cleanup:
     assert(!FAILED(hr));
    return hr;
}

HRESULT CXMLDataSaver::SaveToFile(LPCWSTR fileName)
{
    HRESULT hr = S_OK;
    CComVariant varFileName(fileName);

    IFC( m_spXMLDoc->save(varFileName) );

Cleanup:
    assert(!FAILED(hr));
    return hr;
}

//////////////////////////////////////////////////////////////////////////
//

// We will not be saving any files with 1000+ character strings, so we will treat any files that have such strings as corrupt
const UINT CXMLDataLoader::MAX_STRING_LENGTH = 1000; 

CXMLDataLoader::CXMLDataLoader()
{
}

CXMLDataLoader::~CXMLDataLoader()
{
}

HRESULT CXMLDataLoader::HasNextObject(BOOL* pfHasNextObject)
{
    if(NULL == pfHasNextObject)
    {
        return E_POINTER;
    }
    
    *pfHasNextObject =  (m_spNextObjNode.p != NULL);

    return S_OK;
}

HRESULT CXMLDataLoader::GetNextObject(__out LPWSTR* strName)
{
    USES_CONVERSION;
    
    HRESULT hr = S_OK;
    CComBSTR nodeName;

    if(NULL == strName)
    {
        IFC( E_POINTER );
    }

    m_spCurrObjNode = m_spNextObjNode;
    IFC( LoadNextObject() );

    assert(m_spCurrObjNode != NULL);
    IFC( m_spCurrObjNode->get_nodeName(&nodeName) );

    if(nodeName.Length() > MAX_STRING_LENGTH)
    {
        IFC( STRSAFE_E_INVALID_PARAMETER );
    }
    
    if(nodeName.Length() + 1 < nodeName.Length())
    {
        return E_INVALIDARG;
    }
    
    DWORD cbAlloc = 0;
    IFC( DWordMult(nodeName.Length() + 1, sizeof(WCHAR), &cbAlloc) );
    
    *strName = (LPWSTR) CoTaskMemAlloc(cbAlloc);
    if(!*strName) IFC( E_OUTOFMEMORY );

    IFC( StringCchCopyN(*strName, nodeName.Length() + 1, nodeName, nodeName.Length()) );
    (*strName)[nodeName.Length()] = 0;
  
    
Cleanup:
    return hr;
}

HRESULT CXMLDataLoader::HasChildObjects(BOOL* pfHasChildObjects)
{
    HRESULT hr = S_OK;
    CComPtr<IXMLDOMNodeList> spChildList;
    
    if(NULL == pfHasChildObjects)
    {
        IFC(hr = E_POINTER);
    }
    
    IFC( m_spCurrObjNode->get_childNodes(&spChildList) );

    long length;
    IFC( spChildList->get_length(&length) );

    BOOL fHasChildObject = FALSE;
    for(long i = 0; i < length; i++)
    {
        CComPtr<IXMLDOMNode> spChildNode;

        IFC( spChildList->nextNode(&spChildNode) );

        DOMNodeType nodeType;
        IFC( spChildNode->get_nodeType(&nodeType) );

        if(nodeType == NODE_ELEMENT)
        {
            fHasChildObject = TRUE;
            break;
        }
    }

    *pfHasChildObjects = fHasChildObject;

Cleanup:
    return hr;
}

HRESULT CXMLDataLoader::BeginLoadChildObjects()
{
    HRESULT hr = S_OK;
    LoadContext LCtx;
    
    BOOL fHasChildObjects;
    IFC( HasChildObjects(&fHasChildObjects) );

    if(!fHasChildObjects) IFC( hr = E_FAIL );

    LCtx.m_spObjectList = m_spObjectList;
    LCtx.m_spCurrObjNode = m_spCurrObjNode;
    LCtx.m_spNextObjNode = m_spNextObjNode;
    m_arrLoadContexts.Add(LCtx);

    m_spObjectList.Release();
    IFC( m_spCurrObjNode->get_childNodes(&m_spObjectList) );

    IFC( LoadNextObject() );
    
Cleanup:
    return hr;
}

HRESULT CXMLDataLoader::EndLoadChildObjects()
{
    HRESULT hr = S_OK;

    size_t nLastObj = m_arrLoadContexts.GetCount() - 1;

    LoadContext LCtx = m_arrLoadContexts.GetAt(nLastObj);
    m_arrLoadContexts.RemoveAt(nLastObj);

    m_spObjectList = LCtx.m_spObjectList;
    m_spCurrObjNode = LCtx.m_spCurrObjNode;
    m_spNextObjNode = LCtx.m_spNextObjNode;
    
    return hr;
}

HRESULT CXMLDataLoader::LoadData(LPCWSTR strName, __out LPWSTR* strValue, long nIndex)
{
    HRESULT hr = S_OK;
    CComPtr<IXMLDOMNodeList> spChildList;
    CComPtr<IXMLDOMNamedNodeMap> spAttributes;
    CComPtr<IXMLDOMNode> spFoundNode;
    CComPtr<IXMLDOMNode> spAttributeNode;
    CComVariant nodeValue;
    CComBSTR bstrValue;
    long nCurrentIndex = 0;

    IFC( m_spCurrObjNode->get_childNodes(&spChildList) );
    
    long length;
    IFC(spChildList->get_length(&length));

    for(long i = 0; i < length; i++) 
    {
        CComPtr<IXMLDOMNode> childNode;
        CComBSTR nodeName;

        IFC( spChildList->nextNode(&childNode) );

        IFC( childNode->get_nodeName(&nodeName) );

        if(nodeName == CAtlStringW::PCXSTR(strName)) 
        {
            if(nIndex == nCurrentIndex)
            {
                spFoundNode = childNode;
                break;
            }
            else
            {
                nCurrentIndex++;
            }
        }
    }

    if(spFoundNode == NULL) 
    {
        hr = MF_E_NOT_FOUND;
        goto Cleanup;
    }

    IFC( spFoundNode->get_attributes(&spAttributes) );

    IFC( spAttributes->getNamedItem(CComBSTR(L"Value"), &spAttributeNode) );

    IFC( spAttributeNode->get_nodeValue(&nodeValue) );

    bstrValue = nodeValue.bstrVal;
    
    if(bstrValue.Length() > MAX_STRING_LENGTH)
    {
        IFC( STRSAFE_E_INVALID_PARAMETER );
    }
    
    if(bstrValue.Length() + 1 < bstrValue.Length())
    {
        IFC( E_INVALIDARG );
    }
    
    DWORD cbAlloc = 0;
    IFC( DWordMult(bstrValue.Length() + 1, sizeof(WCHAR), &cbAlloc) );
    
    *strValue = (LPWSTR) CoTaskMemAlloc(cbAlloc);
    if(!*strValue) IFC( E_OUTOFMEMORY );

    StringCchCopyN(*strValue, bstrValue.Length() + 1, bstrValue,  bstrValue.Length());
    (*strValue)[bstrValue.Length()] = 0;
    
Cleanup:
    return hr;
}

HRESULT CXMLDataLoader::LoadFromFile(LPCWSTR fileName, LPCWSTR docName) 
{
    HRESULT hr = S_OK;
    CComPtr<IXMLDOMNodeList> spChildList;
    CComBSTR nodeName;

    m_spXMLDoc.Release();
    IFC( CoCreateInstance(__uuidof(DOMDocument60), NULL, CLSCTX_INPROC_SERVER, __uuidof(IXMLDOMDocument2), (void**)&m_spXMLDoc) );

    VARIANT_BOOL varSucceeded;
    IFC( m_spXMLDoc->load(CComVariant(fileName), &varSucceeded) );

    IFC( m_spXMLDoc->get_childNodes(&spChildList) );

    long length;
    IFC(spChildList->get_length(&length));
    if(length != 1) 
    {
        hr = MF_E_INVALID_FILE_FORMAT;
        goto Cleanup;
    }

    m_spRootNode.Release();
    IFC( spChildList->nextNode(&m_spRootNode) );

    IFC( m_spRootNode->get_nodeName(&nodeName) );

    if(nodeName != CAtlStringW::PCXSTR(docName)) 
    {
        hr = MF_E_INVALID_FILE_FORMAT;
        goto Cleanup;
    }

    m_spObjectList.Release();
    IFC( m_spRootNode->get_childNodes(&m_spObjectList) );

    IFC( LoadNextObject() );

Cleanup:
    return hr;
}

HRESULT CXMLDataLoader::LoadNextObject()
{
    HRESULT hr = S_OK;

    HRESULT hrAttribute = S_OK;
    do
    {
        CComPtr<IXMLDOMElement> spElement;

        m_spNextObjNode.Release();
        hr = m_spObjectList->nextNode(&m_spNextObjNode);
        if(hr == S_FALSE)
        {
            break;
        }

        IFC( m_spNextObjNode->QueryInterface(IID_IXMLDOMElement, (void**) &spElement) );
        VARIANT var;
        hrAttribute = spElement->getAttribute(CComBSTR(L"Value"), &var);
    } while(hrAttribute != S_FALSE);

Cleanup:
    return hr;
}
