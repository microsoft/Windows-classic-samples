// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "StdAfx.h"
#include "TasksClient.h"
#include "TasksContent.h"
#include "GlanceContent.h"
#include "TasksEvents.h"

//
// This APPLICATION_ID is the unique ID for this
// Windows SideShow application. It is a GUID.
//

// {FB35ABAF-3C45-4198-9BC2-C79C065D33AC}
static const APPLICATION_ID TASKS_APPLICATION_ID =
{ 0xfb35abaf, 0x3c45, 0x4198, { 0x9b, 0xc2, 0xc7, 0x9c, 0x6, 0x5d, 0x33, 0xac } };

CTasksClient::CTasksClient() :
    m_pContent(NULL),
    m_pEvents(NULL),
    m_pGlanceContent(NULL)
{
    m_applicationID = TASKS_APPLICATION_ID;
}

CTasksClient::~CTasksClient()
{
    if (NULL != m_pGlanceContent)
    {
        m_pGlanceContent->Release();
        m_pGlanceContent = NULL;
    }

    if (NULL != m_pEvents)
    {
        m_pEvents->Release();
        m_pEvents = NULL;
    }

    if(NULL != m_pContent)
    {
        m_pContent->Release();
        m_pContent = NULL;
    }
}

void CTasksClient::AddContent()
{
    if (NULL != m_pContentMgr)
    {
        //
        // Create the objects that implement the COM interfaces;
        // remember we have to Release (rather than delete) them
        // later.
        //
        m_pContent = new CTasksContent();
        m_pEvents = new CTasksEvents(m_pContent);
        m_pGlanceContent = new CGlanceContent(m_pContent);

        //
        // Add the content and Glance data to the platform
        //
        AddContentItem(m_pContent);
        AddContentItem(m_pGlanceContent);

        //
        // Set the event sink for platform events
        //
        (void)m_pContentMgr->SetEventSink(m_pEvents);
    }
}
