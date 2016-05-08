// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "StdAfx.h"
#include "BaseClient.h"

using namespace std;

CBaseClient::CBaseClient() :
    m_pSession(NULL),
    m_pContentMgr(NULL),
    m_pNotificationMgr(NULL),
    m_pContentList(NULL),
    m_applicationID(GUID_NULL)
{
}

CBaseClient::~CBaseClient()
{
    //
    // Release the memory used for all of the content
    //
    while (NULL != m_pContentList)
    {
        ContentItem* item = m_pContentList->m_pNext;

        //
        // Be sure to Release the reference to the COM interface!
        //
        if (NULL != m_pContentList->m_pContent)
        {          
            m_pContentList->m_pContent->Release();
        }

        delete m_pContentList;
        m_pContentList = item;
    }
}

void CBaseClient::Register()
{
    //
    // Create the SideShowSession object that
    // enables us to talk to the platform.
    //
    HRESULT hr = S_OK;

    if (NULL == m_pSession && 
        NULL == m_pContentMgr &&
        NULL == m_pNotificationMgr)
    {
        ::CoCreateInstance(CLSID_SideShowSession,
                           NULL,
                           CLSCTX_INPROC_SERVER,
                           IID_PPV_ARGS(&m_pSession));
        if (FAILED(hr))
        {
            //
            // Failed to CoCreate SideShowSession.
            //
        }
        else
        {
            //
            // Register this client application with the platform
            // so that it can send content to devices
            //
            hr = m_pSession->RegisterContent(m_applicationID,
                                             SIDESHOW_ENDPOINT_SIMPLE_CONTENT_FORMAT,
                                             &m_pContentMgr);
            if (SUCCEEDED(hr))
            {
                if (S_FALSE == hr)
                {
                    //
                    // Successfully registered with the Windows SideShow platform,
                    // but no devices available or enabled.
                    //
                }

                //
                // Successfully registered with Windows SideShow platform,
                // now go register for Notifications
                //
                hr = m_pSession->RegisterNotifications(m_applicationID,
                                                       &m_pNotificationMgr);
                if (S_OK == hr)
                {
                    //
                    // Successfully registered with the Windows SideShow platform for content and notifications
                    //
                }
                else if (S_FALSE == hr)
                {
                    //
                    // Successfully registered with the Windows SideShow platform,
                    // but no devices available or enabled.
                    //
                }
                else if (E_INVALIDARG == hr)
                {
                    //
                    // ISideShowSession::RegisterContent failed with Invalid Argument:
                    // Have you added the proper information to the registry for this application?
                    //
                }
                else
                {
                    //
                    // ISideShowSession::RegisterNotifications failed
                    //
                }
            }
            else if (E_INVALIDARG == hr)
            {
                //
                // ISideShowSession::RegisterContent failed with Invalid Argument:
                // Have you added the proper information to the registry for this application?
                //
            }
            else 
            {
                //
                // ISideShowSession::RegisterContent failed
                //
            }
        }       
    }
    else
    {
        //
        // This instance has already registered with the Windows SideShow platform
        //
    }
}

void CBaseClient::Unregister()
{
    if (NULL != m_pNotificationMgr)
    {
        m_pNotificationMgr->Release();
        m_pNotificationMgr = NULL;
    }

    if (NULL != m_pContentMgr)
    {
        //
        // Remove the event sink, if any, registered with the platform
        //
        (void)m_pContentMgr->SetEventSink(NULL);

        m_pContentMgr->Release();
        m_pContentMgr = NULL;
    }

    if (NULL != m_pSession)
    {       
        m_pSession->Release();
        m_pSession = NULL;
    }
}

void CBaseClient::AddContentItem(ISideShowContent* pContent)
{
    //
    // Create a new item if the list is empty
    //
    if (NULL == m_pContentList)
    {
        m_pContentList = new ContentItem();
    }

    //
    // Find the last item in the list
    //
    ContentItem* lastItem = m_pContentList;
    while (NULL != lastItem->m_pNext)
    {
        lastItem = lastItem->m_pNext;
    }

    //
    // Be sure to AddRef the COM interface!
    //
    lastItem->m_pContent = pContent;
    if (NULL != lastItem->m_pContent)
    {
        lastItem->m_pContent->AddRef();
    }

    lastItem->m_pNext = new ContentItem();

    //
    // Add the content to the device
    //
    if (NULL != m_pContentMgr)
    {
        HRESULT hr = m_pContentMgr->Add(pContent);

        if (FAILED(hr))
        {
            //
            // Failed to add content
            //
        }
    }
}

void CBaseClient::RemoveAllContent()
{
    if (NULL != m_pContentMgr)
    {
        HRESULT hr = m_pContentMgr->RemoveAll();
        if (FAILED(hr))
        {
            //
            // Failed to remove content
            //
        }
    }
}

