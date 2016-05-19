// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "StdAfx.h"
#include "XmlDocument.h"
#include <strsafe.h>

CXmlNode::CXmlNode(IXMLDOMNode* pNode) :
    m_pChildNodes(NULL)
{
    //
    // Store the Node and take a reference to it
    //
    m_pNode = pNode;
    m_pNode->AddRef();
}

CXmlNode::~CXmlNode()
{
    if (NULL != m_pNode)
    {
        m_pNode->Release();
        m_pNode = NULL;
    }
}

CXmlNodeList* CXmlNode::GetChildNodes()
{    
    if (NULL == m_pChildNodes && NULL != m_pNode)
    {
        HRESULT hr = S_OK;
        IXMLDOMNodeList* pChildNodes = NULL;

        hr = m_pNode->get_childNodes(&pChildNodes);
        if (SUCCEEDED(hr) && NULL != pChildNodes)
        {
            m_pChildNodes = new CXmlNodeList(pChildNodes);

            pChildNodes->Release();
            pChildNodes = NULL;
        }
    }

    return m_pChildNodes;
}

BSTR CXmlNode::GetName()
{
    BSTR bstrName = NULL;
    if (NULL != m_pNode)
    {
        (void)m_pNode->get_nodeName(&bstrName);
    }

    return bstrName;
}

BSTR CXmlNode::GetInnerText()
{
    BSTR bstrText = NULL;
    if (NULL != m_pNode)
    {
        (void)m_pNode->get_text(&bstrText);
    }

    return bstrText;
}
