// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "StdAfx.h"
#include "TasksContent.h"
#include "TasksEvents.h"

CTasksEvents::CTasksEvents(CTasksContent* pContent)
{
    m_pContent = pContent;
}

CTasksEvents::~CTasksEvents()
{
}

// ISideShowEvents methods
HRESULT CTasksEvents::ContentMissing(
        const CONTENT_ID contentId,
        ISideShowContent** ppIContent)
{
    HRESULT hr = E_FAIL;
    if (NULL != m_pContent)
    {
        ISideShowContent* pIContent = NULL;

        //
        // Get the associated task for the missing content ID
        //
        pIContent = m_pContent->GetTask(contentId);
        if (NULL != pIContent)
        {
            //
            // Use QI to return the AddRef'ed pointer to the content.
            // The platform will call Release for us.
            //
            hr = pIContent->QueryInterface(IID_PPV_ARGS(ppIContent));
        }
    }
    return hr;
}

HRESULT CTasksEvents::ApplicationEvent(
        ISideShowCapabilities* /*pICapabilities*/,
        const DWORD /*dwEventId*/,
        const DWORD /*dwEventSize*/,
        const BYTE* /*pbEventData*/)
{
    return S_OK;
}

HRESULT CTasksEvents::DeviceAdded(
        ISideShowCapabilities* /*pIDevice*/)
{
    return S_OK;
}

HRESULT CTasksEvents::DeviceRemoved(
        ISideShowCapabilities* /*pIDevice*/)
{
    return S_OK;
}
