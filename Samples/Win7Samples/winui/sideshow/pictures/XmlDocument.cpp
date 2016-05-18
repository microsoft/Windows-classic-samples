// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "StdAfx.h"
#include "XmlDocument.h"

using namespace std;

CXmlDocument::CXmlDocument() :
    m_pChildNodes(NULL)
{
    HRESULT hr = S_OK;

    hr = CoCreateInstance(__uuidof(DOMDocument30),
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          IID_PPV_ARGS(&m_pXMLDoc));

    if (FAILED(hr))
    {
        cout << "Failed to create an instance of IXMLDOMDocument" << endl;
    }
}

CXmlDocument::~CXmlDocument()
{
    if (NULL != m_pXMLDoc)
    {
        m_pXMLDoc->Release();
        m_pXMLDoc = NULL;
    }
}

BOOL CXmlDocument::Load(LPWSTR pwszFileName)
{
    VARIANT_BOOL fLoaded = VARIANT_FALSE;

    if (NULL != pwszFileName && NULL != m_pXMLDoc)
    {
        cout << "created xml doc" << endl;

        HRESULT         hr = S_OK;
        VARIANT         varFile;

        //
        // Initialize the VARIANT with the filename
        //
        ::VariantInit(&varFile);
        varFile.bstrVal = ::SysAllocString(pwszFileName);
        varFile.vt = VT_BSTR;

        hr = m_pXMLDoc->load(varFile, &fLoaded);
        if (SUCCEEDED(hr) && fLoaded == VARIANT_TRUE)
        {
            cout <<  "Loaded xml document." << endl;
        }
        else
        {
            cout << "Failed to load xml document, hr = " << hr << endl;
        }

        //
        // Cleanup the VARIANT; this will free the allocated BSTR
        //
        ::VariantClear(&varFile);
    }

    return VARIANT_TRUE == fLoaded;
}

CXmlNodeList* CXmlDocument::GetChildNodes()
{
    //
    // If there is a list of child nodes, return it.
    // Otherwise, generate it from the IXMLDOMDocument
    //
    if (NULL == m_pChildNodes && NULL != m_pXMLDoc)
    {
        HRESULT         hr = S_OK;
        IXMLDOMNodeList *pChildNodes = NULL;
        IXMLDOMElement  *pDocElement = NULL;

        hr = m_pXMLDoc->get_documentElement(&pDocElement);
        if (SUCCEEDED(hr) && NULL != pDocElement)
        {
            hr = pDocElement->get_childNodes(&pChildNodes);
            if (SUCCEEDED(hr) && NULL != pChildNodes)
            {
                m_pChildNodes = new CXmlNodeList(pChildNodes);

                //
                // Release the child nodes; CXmlNodeList will
                // retain a reference.
                //
                pChildNodes->Release();
                pChildNodes = NULL;
            }

            //
            // Release the element
            //
            pDocElement->Release();
            pDocElement = NULL;
        }
    }

    return m_pChildNodes;
}


CXmlElement* CXmlDocument::AddElement(LPWSTR pwszName)
{
    HRESULT         hr = S_OK;
    CXmlElement     *pElement = NULL;
    IXMLDOMElement  *pIElement = NULL;
    IXMLDOMNode     *pNewNode = NULL;
    BSTR            bstrName = ::SysAllocString(pwszName);

    if (NULL != m_pXMLDoc)
    {      
        hr = m_pXMLDoc->createElement(bstrName, &pIElement);
        if (SUCCEEDED(hr))
        {
            hr = m_pXMLDoc->appendChild(pIElement, &pNewNode);
            if (SUCCEEDED(hr))
            {
                pElement = new CXmlElement(m_pXMLDoc, pIElement);

                //
                // Release the new node
                //
                pNewNode->Release();
                pNewNode = NULL;
            }

            //
            // Release the Element
            //
            pIElement->Release();
            pIElement = NULL;
        }
    }

    ::SysFreeString(bstrName);

    return pElement;
}

void CXmlDocument::GetXml(BSTR* pbstrXml)
{
    HRESULT hr = m_pXMLDoc->get_xml(pbstrXml);
    if (FAILED(hr))
    {
        ::SysFreeString(*pbstrXml);
    }
}
