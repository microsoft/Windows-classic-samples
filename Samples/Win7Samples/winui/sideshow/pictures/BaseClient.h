// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#pragma once

struct ContentItem
{
    ContentItem()
    {
        m_pContent = NULL;
        m_pNext = NULL;
    };

    ISideShowContent    *m_pContent;
    ContentItem         *m_pNext;
};

class CBaseClient
{
protected:
    ISideShowSession                *m_pSession;
    ISideShowContentManager         *m_pContentMgr;
    ISideShowNotificationManager    *m_pNotificationMgr;

    ContentItem                     *m_pContentList;
    APPLICATION_ID                   m_applicationID;

protected:
    //
    // Use this method to add content to the device;
    // the base class will keep track of it in m_pContentList
    //
    void AddContentItem(ISideShowContent* pContent);

public:
    CBaseClient();
    virtual ~CBaseClient();

    void Register();
    void Unregister();

    //
    // Override this method in your subclass to
    // create content objects, and use the
    // AddContentItem method to add them to the
    // platform.
    //
    virtual void AddContent() = 0;
    virtual void ClearContent();

    void RemoveAllContent();
};
