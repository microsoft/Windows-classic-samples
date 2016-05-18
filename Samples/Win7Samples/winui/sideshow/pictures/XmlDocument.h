// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#pragma once

#include <msxml2.h>

#include "XmlNode.h"
#include "XmlNodeList.h"
#include "XmlElement.h"

class CXmlDocument
{
private:
    IXMLDOMDocument2    *m_pXMLDoc;
    CXmlNodeList        *m_pChildNodes;

public:
    CXmlDocument();
    virtual ~CXmlDocument();

    BOOL Load(LPWSTR pwszFileName);
    CXmlNodeList* GetChildNodes();

    CXmlElement* AddElement(LPWSTR pwszName);

    void GetXml(BSTR* bstrXml);
};
