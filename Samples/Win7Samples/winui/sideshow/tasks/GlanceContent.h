// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#pragma once

class CTasksContent;

class CGlanceContent :
    public CBaseContent
{
private:
    CTasksContent   *m_pContent;

protected:
    virtual void LoadContent(DWORD* pdwSize, BYTE** ppbData);
    virtual void FreeContent(BYTE** ppbData);

public:
    CGlanceContent(CTasksContent* pContent);
    ~CGlanceContent();
};
