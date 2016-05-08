// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "StdAfx.h"
#include "Picture.h"
#include "XmlDocument.h"

CPicture::CPicture(CONTENT_ID id, LPWSTR pwszFile)
{
    m_contentID = id;
    m_prevId = id - 1;
    m_nextId = id + 1;

    //
    // Copy the filename locally for the alternate text
    // 
    if (NULL != pwszFile)
    {
        size_t length = wcslen(pwszFile) + 1;
        m_pwszFile = new WCHAR[length];
        if (NULL != m_pwszFile)
        {           
            StringCchCopyW(m_pwszFile, length, pwszFile);
        }
    }
}

CPicture::CPicture()
{
    m_contentID = CONTENT_ID_GLANCE;
    m_prevId = 0;
    m_nextId = 0;
    m_pwszFile = NULL;
}

CPicture::~CPicture()
{
    if (NULL != m_pwszFile)
    {
        delete [] m_pwszFile;
        m_pwszFile = NULL;
    }
}


void CPicture::LoadContent(DWORD* pdwSize, BYTE** ppbData, ISideShowCapabilities * /*pICapabilities*/)
{
    if (NULL == pdwSize ||
        NULL == ppbData)
    {
        return;
    }

    //
    // Return size for the string, including the terminating NULL
    //
    LPSTR pszContent = GetContentXML();
    *ppbData = (BYTE*)pszContent;
    *pdwSize = (DWORD)strlen(pszContent) + 1;
}

void CPicture::FreeContent(BYTE** ppbData)
{
    //
    // Free the memory allocated in LoadContent
    //
    if (NULL != ppbData)
    {
        ::CoTaskMemFree(*ppbData);
    }
}

LPSTR CPicture::GetContentXML()
{
    LPSTR pszXML = NULL;

    CXmlDocument xmlDoc;

    CXmlElement* pBody = xmlDoc.AddElement(L"body");
    CXmlElement* pContent = pBody->AddElement(L"content");
        pContent->AddAttribute(L"id", m_contentID);
        pContent->AddAttribute(L"title", m_pwszFile);
        CXmlElement* pImage = pContent->AddElement(L"img");
            pImage->AddAttribute(L"align", L"c");
            pImage->AddAttribute(L"fit", L"screen");
            pImage->AddAttribute(L"alt", m_pwszFile);
            //
            // The Image ID is the CONTENT_ID of the raw image bytes;
            // that is the same as this content ID, offset by
            // (CID_RAWIMAGE_FIRST - CID_XMLIMAGE_FIRST)
            //
            pImage->AddAttribute(L"id", m_contentID + (CID_RAWIMAGE_FIRST - CID_XMLIMAGE_FIRST));

        CXmlElement* pLeftBtn = pContent->AddElement(L"btn");
            pLeftBtn->AddAttribute(L"key", L"left");
            pLeftBtn->AddAttribute(L"target", m_prevId);

        CXmlElement* pRightBtn = pContent->AddElement(L"btn");
            pRightBtn->AddAttribute(L"key", L"right");
            pRightBtn->AddAttribute(L"target", m_nextId);

    BSTR bstrXml;
    xmlDoc.GetXml(&bstrXml);

    //
    // The device handles UTF8 encoded strings, so the XML
    // needs to be converted from unicode to UTF8.
    //
    pszXML = AllocTaskUtf8String(bstrXml);

    ::SysFreeString(bstrXml);
    delete pBody;
    delete pContent;
    delete pImage;
    delete pLeftBtn;
    delete pRightBtn;

    return pszXML;
}

void CPicture::SetNextID(CONTENT_ID id)
{
    m_nextId = id;
}

void CPicture::SetPrevID(CONTENT_ID id)
{
    m_prevId = id;
}