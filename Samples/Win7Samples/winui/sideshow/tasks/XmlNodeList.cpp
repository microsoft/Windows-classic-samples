// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "StdAfx.h"
#include "XMLDocument.h"

using namespace std;

CXmlNodeList::CXmlNodeList(IXMLDOMNodeList* pChildNodes)
{
    //
    // Store the pointer and take a reference
    //
    m_pChildNodes = pChildNodes;
    m_pChildNodes->AddRef();
}

CXmlNodeList::~CXmlNodeList()
{
    if (NULL != m_pChildNodes)
    {
        m_pChildNodes->Release();
        m_pChildNodes = NULL;
    }
}

int CXmlNodeList::Count()
{
    long nNodes = 0;
    (void)m_pChildNodes->get_length(&nNodes);

    return nNodes;
}

CXmlNode* CXmlNodeList::GetNode(long index)
{
    HRESULT     hr = S_OK;
    CXmlNode    *pXmlNode = NULL;
    IXMLDOMNode *pNode = NULL;

    hr = m_pChildNodes->get_item(index, &pNode);
    if (SUCCEEDED(hr))
    {
        pXmlNode = new CXmlNode(pNode);

        //
        // Release the DOM Node
        //
        pNode->Release();
        pNode = NULL;
    }

    return pXmlNode;
}

