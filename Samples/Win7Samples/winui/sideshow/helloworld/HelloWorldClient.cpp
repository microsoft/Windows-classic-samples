// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "StdAfx.h"
#include "HelloWorldClient.h"
#include "HelloWorldContent.h"

//
// This APPLICATION_ID is the unique ID for this
// Windows SideShow application. It is a GUID.
//

// {625B4120-AD9D-4109-B4FB-D3CC0863BAA3}
static const APPLICATION_ID HELLOWORLD_APPLICATION_ID = 
{ 0x625b4120, 0xad9d, 0x4109, { 0xb4, 0xfb, 0xd3, 0xcc, 0x8, 0x63, 0xba, 0xa3 } };

using namespace std;

CHelloWorldClient::CHelloWorldClient() :
    m_pSession(NULL),
    m_pContentMgr(NULL),
    m_pContent(NULL)
{
}

CHelloWorldClient::~CHelloWorldClient()
{
}

void CHelloWorldClient::Register()
{
    //
    // Create the SideShowSession object that
    // enables us to talk to the platform.
    //
    cout << "Registering Windows SideShow Client..." << endl;
    HRESULT hr = S_OK;

    if (NULL == m_pSession && 
        NULL == m_pContentMgr)
    {
        ::CoCreateInstance(CLSID_SideShowSession,
                           NULL,
                           CLSCTX_INPROC_SERVER,
                           IID_PPV_ARGS(&m_pSession));
        if (FAILED(hr))
        {
            cout << "Failed to CoCreate SideShowSession, hr = " << hr << endl;
        }
        else
        {
            //
            // Register this client application with the platform
            // so that it can send content to devices
            //
            hr = m_pSession->RegisterContent(HELLOWORLD_APPLICATION_ID,
                                             SIDESHOW_ENDPOINT_SIMPLE_CONTENT_FORMAT,
                                             &m_pContentMgr);
            if (S_OK == hr)
            {
                cout << "Successfully registered with Windows SideShow platform." << endl;
            }
            else if (S_FALSE == hr)
            {
                cout << "Successfully registered with the Windows SideShow platform, " <<
                        "but no devices available or enabled." << endl;
            }
            else if (E_INVALIDARG == hr)
            {
                cout << "ISideShowSession::RegisterContent failed with Invalid Argument: " <<
                        "Have you added the proper information to the registry for this application?" << endl;
            }
            else 
            {
                cout << "ISideShowSession::RegisterContent failed, hr = " << hr << endl;
            }
        }       
    }
    else
    {
        cout << "This instance has already registered with the Windows SideShow platform" << endl;
    }
}

void CHelloWorldClient::Unregister()
{
    if (NULL != m_pContent)
    {
        m_pContent->Release();
        m_pContent = NULL;
    }

    if (NULL != m_pContentMgr)
    {
        m_pContentMgr->Release();
        m_pContentMgr = NULL;
    }

    if (NULL != m_pSession)
    {       
        m_pSession->Release();
        m_pSession = NULL;
    }
}


void CHelloWorldClient::AddContent()
{
    if (NULL != m_pContentMgr)
    {
        //
        // Create an instance of the content object if it doesn't exist
        //
        if (NULL == m_pContent)
        {
            m_pContent = new CHelloWorldContent();
        }

        HRESULT hr = m_pContentMgr->Add(m_pContent);

        if (SUCCEEDED(hr))
        {
            cout << "Successfully added content" << endl;
        }
        else
        {
            cout << "Failed to add content, HRESULT = " << hr << endl;
        }
    }
}

void CHelloWorldClient::RemoveAllContent()
{
    if (NULL != m_pContentMgr)
    {
        HRESULT hr = m_pContentMgr->RemoveAll();
        if (SUCCEEDED(hr))
        {
            cout << "Removed all content" << endl;
        }
        else
        {
            cout << "Failed to remove content, HRESULT = " << hr << endl;
        }
    }
}

