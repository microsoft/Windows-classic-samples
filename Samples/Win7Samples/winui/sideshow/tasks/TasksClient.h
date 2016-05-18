// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#pragma once

class CTasksContent;
class CTasksEvents;
class CGlanceContent;

class CTasksClient :
    public CBaseClient
{
private:
    CTasksContent   *m_pContent;
    CGlanceContent  *m_pGlanceContent;
    CTasksEvents    *m_pEvents;

public:
    CTasksClient();
    virtual ~CTasksClient();

    virtual void AddContent();
};
