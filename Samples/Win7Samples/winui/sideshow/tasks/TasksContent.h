// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#pragma once

#include "Task.h"

#define MAX_TASKS   10

class CXmlNode;

class CTasksContent : public CBaseContent
{
private:
    CTask*  m_tasks[MAX_TASKS];
    int     m_cTasks;

    CTask* BuildTask(CXmlNode* pNode, int id);

protected:
    virtual void LoadContent(DWORD* pdwSize, BYTE** ppbData);
    virtual void FreeContent(BYTE** ppbData);

    char* BuildMenu();
    void LoadTasks();

public:
    CTasksContent();
    virtual ~CTasksContent();

    ISideShowContent* GetTask(CONTENT_ID id);

    int Count();
};
