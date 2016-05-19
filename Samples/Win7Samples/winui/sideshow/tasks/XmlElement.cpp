// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "StdAfx.h"
#include "XmlElement.h"

CXmlElement::CXmlElement(IXMLDOMDocument* pIDoc, IXMLDOMElement* pIElement)
{
    //
    // Store and refcount the pointers
    //
    m_pIElement = pIElement;
    m_pIElement->AddRef();

    m_pIDoc = pIDoc;
    m_pIDoc->AddRef();
}

CXmlElement::~CXmlElement()
{
    if (NULL != m_pIElement)
    {
        m_pIElement->Release();
        m_pIElement = NULL;
    }

    if (NULL != m_pIDoc)
    {
        m_pIDoc->Release();
        m_pIDoc = NULL;
    }
}

CXmlElement* CXmlElement::AddElement(LPWSTR pwszName)
{
    HRESULT         hr = S_OK;
    CXmlElement     *pElement = NULL;
    IXMLDOMElement  *pIElement = NULL;
    BSTR            bstrName = ::SysAllocString(pwszName);

    if (NULL != m_pIDoc)
    {       
        hr = m_pIDoc->createElement(bstrName, &pIElement);
        if (SUCCEEDED(hr))
        {
            IXMLDOMNode *pNewNode = NULL;

            hr = m_pIElement->appendChild(pIElement, &pNewNode);
            if (SUCCEEDED(hr))
            {
                pElement = new CXmlElement(m_pIDoc, pIElement);

                pNewNode->Release();
                pNewNode = NULL;
            }

            pIElement->Release();
            pIElement = NULL;
        }
    }

    ::SysFreeString(bstrName);

    return pElement;
}

HRESULT CXmlElement::AddAttributeInternal(LPWSTR pwszName, VARIANT& varValue)
{
    HRESULT             hr = S_OK;
    IXMLDOMAttribute    *pAttribute = NULL;
    BSTR                bstrName; 

    bstrName = ::SysAllocString(pwszName);
    if (NULL != bstrName)
    {
        hr = m_pIDoc->createAttribute(bstrName, &pAttribute);
        ::SysFreeString(bstrName);       
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    if (NULL != pAttribute)
    {
        hr = pAttribute->put_value(varValue);
        if (SUCCEEDED(hr))
        {
            hr = m_pIElement->setAttributeNode(pAttribute, NULL);           
        }

        pAttribute->Release();
        pAttribute = NULL;
    }

    return hr;
}

void CXmlElement::AddAttribute(LPWSTR pwszName, LPWSTR pwszValue)
{
    HRESULT             hr = S_OK;
    VARIANT             varValue;

    ::VariantInit(&varValue);

    varValue.vt = VT_BSTR;
    varValue.bstrVal = ::SysAllocString(pwszValue);

    hr = AddAttributeInternal(pwszName, varValue);

    ::VariantClear(&varValue);
}

void CXmlElement::AddAttribute(LPWSTR pwszName, ULONG ulValue)
{
    HRESULT             hr = S_OK;
    VARIANT             varValue;

    ::VariantInit(&varValue);

    varValue.vt = VT_UI4;
    V_UI4(&varValue) = ulValue;

    hr = AddAttributeInternal(pwszName, varValue);

    ::VariantClear(&varValue);
}

void CXmlElement::AddText(LPWSTR pwszText)
{
    BSTR bstrText = ::SysAllocString(pwszText);
    m_pIElement->put_text(bstrText);
    ::SysFreeString(bstrText);
}
