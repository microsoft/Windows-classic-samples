// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "StdAfx.h"
#include "Task.h"
#include "XmlDocument.h"

CTask::CTask() :
    m_prevId(0),
    m_nextId(0)
{
}


CTask::CTask(CONTENT_ID id, LPWSTR name, LPWSTR details, LPWSTR category, LPWSTR dueTime) :
    CBaseContent()
{
    HRESULT hr = E_FAIL;
    m_contentID = id;

    hr = StringCbCopy(m_wszName, sizeof(m_wszName), name);
    if (FAILED(hr))
    {
        //handle the error
    }

    hr = StringCbCopy(m_wszDetails, sizeof(m_wszDetails), details);
    if (FAILED(hr))
    {
        //handle the error
    }

    hr = StringCbCopy(m_wszCategory, sizeof(m_wszCategory), category);
    if (FAILED(hr))
    {
        //handle the error
    }

    hr = StringCbCopy(m_wszTimeDue, sizeof(m_wszTimeDue), dueTime);
    if (FAILED(hr))
    {
        //handle the error
    }

    m_prevId = id-1;
    m_nextId = id+1;
}

CTask::~CTask()
{
}


void CTask::LoadContent(DWORD* pdwSize, BYTE** ppbData)
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

void CTask::FreeContent(BYTE** ppbData)
{
    //
    // Free the memory allocated in LoadContent (actually GetContentXML())
    //
    if (NULL != ppbData)
    {
        ::CoTaskMemFree(*ppbData);
    }
}

CONTENT_ID CTask::GetID()
{
    return m_contentID;
}

LPWSTR CTask::GetName()
{
    return m_wszName;
}

LPWSTR CTask::GetCategory()
{
    return m_wszCategory;
}

LPSTR CTask::GetContentXML()
{
    LPSTR pszXML = NULL;

    CXmlDocument xmlDoc;

    CXmlElement* pBody = xmlDoc.AddElement(L"body");
    CXmlElement* pContent = pBody->AddElement(L"content");
        pContent->AddAttribute(L"id", GetID());
        pContent->AddAttribute(L"title", GetName());
        CXmlElement* pCategory = pContent->AddElement(L"txt");
            pCategory->AddText(m_wszCategory);
        CXmlElement* pDetails = pContent->AddElement(L"txt");
            pDetails->AddText(m_wszDetails);
        CXmlElement* pDueTime = pContent->AddElement(L"txt");
            pDueTime->AddText(m_wszTimeDue);

        CXmlElement* pLeftBtn = pContent->AddElement(L"btn");
            pLeftBtn->AddAttribute(L"key", L"left");
            pLeftBtn->AddAttribute(L"target", m_prevId);

        CXmlElement* pRightBtn = pContent->AddElement(L"btn");
            pRightBtn->AddAttribute(L"key", L"right");
            pRightBtn->AddAttribute(L"target", m_nextId);

    BSTR bstrXml;
    xmlDoc.GetXml(&bstrXml);

    printf("Sending Content: \n%ws\n\n", bstrXml);

    //
    // The device handles UTF8 encoded strings, so the XML
    // needs to be converted from unicode to UTF8.
    //
    pszXML = AllocTaskUtf8String(bstrXml);

    ::SysFreeString(bstrXml);

    return pszXML;
}

void CTask::SetNextID(CONTENT_ID id)
{
    m_nextId = id;
}
