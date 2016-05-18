// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#pragma once

class CXmlNode;

class CXmlNodeList
{
private:
    IXMLDOMNodeList *m_pChildNodes;

public:
    CXmlNodeList(IXMLDOMNodeList* pChildNodes);
    virtual ~CXmlNodeList();

    int Count();

    CXmlNode* GetNode(long index);
};