// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#pragma once

class CXmlElement
{
private:
    IXMLDOMElement  *m_pIElement;
    IXMLDOMDocument *m_pIDoc;

    HRESULT AddAttributeInternal(LPWSTR pwszName, VARIANT& varValue);

public:
    CXmlElement(IXMLDOMDocument* pIDoc, IXMLDOMElement* pIElement);
    virtual ~CXmlElement();

    CXmlElement* AddElement(LPWSTR pwszName);
    void AddAttribute(LPWSTR pwszName, LPWSTR pwszValue);
    void AddAttribute(LPWSTR pwszName, ULONG ulValue);

    void AddText(LPWSTR pwszText);
};
