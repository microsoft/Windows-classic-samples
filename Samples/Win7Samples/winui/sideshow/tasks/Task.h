// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#pragma once

class CTask :
    public CBaseContent
{
private:
    WCHAR       m_wszName[32];
    WCHAR       m_wszDetails[64];
    WCHAR       m_wszCategory[32];
    WCHAR       m_wszTimeDue[32];

    CONTENT_ID  m_prevId;
    CONTENT_ID  m_nextId;

protected:
    virtual void LoadContent(DWORD* pdwSize, BYTE** ppbData);
    virtual void FreeContent(BYTE** ppbData);

    LPSTR GetContentXML();

public:
    CTask();
    CTask(CONTENT_ID id, LPWSTR name, LPWSTR details, LPWSTR category, LPWSTR dueTime);
    virtual ~CTask();

    CONTENT_ID GetID();
    LPWSTR GetName();
    LPWSTR GetCategory();

    void SetNextID(CONTENT_ID id);
};
