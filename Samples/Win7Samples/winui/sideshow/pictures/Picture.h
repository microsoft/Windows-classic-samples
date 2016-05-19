// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#pragma once

class CPicture :
    public CBaseContent
{
private:
    LPWSTR      m_pwszFile;
    CONTENT_ID  m_prevId;
    CONTENT_ID  m_nextId;

protected:
    virtual void LoadContent(DWORD* pdwSize, BYTE** ppbData, ISideShowCapabilities *pICapabilities);
    virtual void FreeContent(BYTE** ppbData);

    LPSTR GetContentXML();

public:
    CPicture();
    CPicture(CONTENT_ID id, LPWSTR pwszFile);
    virtual ~CPicture();

    void SetNextID(CONTENT_ID id);
    void SetPrevID(CONTENT_ID id);
};
