// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "StdAfx.h"
#include "AlarmClient.h"
#include "AlarmContent.h"

//
// This APPLICATION_ID is the unique ID for this
// Windows SideShow application. It is a GUID.
//

// {0F11B914-F461-4785-877C-482AB481CF7A}
static const GUID ALARM_CLIENT_APPLICATION_ID = 
{ 0xf11b914, 0xf461, 0x4785, { 0x87, 0x7c, 0x48, 0x2a, 0xb4, 0x81, 0xcf, 0x7a } };


CAlarmClient::CAlarmClient() :
    m_pSession(NULL),
    m_pContentMgr(NULL),
    m_pContent(NULL),
    m_pNotificationMgr(NULL),
    m_pNotification(NULL),
    m_pAlarms(NULL),
    m_lastNotificationId(0)
{
    m_pAlarms = new Alarm();
}

CAlarmClient::~CAlarmClient()
{
    //
    // Release the memory used for all of the alarms
    //
    while (NULL != m_pAlarms)
    {
        Alarm* tmp = m_pAlarms->m_pNext;
        delete m_pAlarms;
        m_pAlarms = tmp;
    }
}

void CAlarmClient::SetAlarm(SYSTEMTIME* pAlarmTime)
{
    if (NULL != pAlarmTime)
    {
        Alarm* pLastAlarm = m_pAlarms;

        //
        // Find the last Alarm in the list
        //
        while (pLastAlarm->m_pNext != NULL)
        {
            pLastAlarm = pLastAlarm->m_pNext;
        }

        //
        // Increment the last-used NOTIFICATION_ID so
        // we have a new one to assign to this alarm
        //
        m_lastNotificationId++;

        //
        // Initialize the Alarm struct; this is empty
        // because the last item in the list is always
        // an empty structure.
        //
        pLastAlarm->m_bSet          = TRUE;
        pLastAlarm->m_time          = *pAlarmTime;
        pLastAlarm->m_notificationId= m_lastNotificationId;
        pLastAlarm->m_pClient       = this;

        pLastAlarm->m_pNext         = new Alarm();

        //
        // Start a thread that will fire the alarm when it's time
        //
        StartAlarmThread(pLastAlarm);

        ::MessageBox(NULL, L"You have set the alarm.", L"Alarm Set", MB_OK);
    }
}

void CAlarmClient::ShowNotification(Alarm* pAlarm)
{
    if (NULL != m_pNotification)
    {
        HRESULT hr = S_OK;
        WCHAR   wszMsg[32];
        SYSTEMTIME expTime;

        //
        // Set the NotificationId property
        //
        hr = m_pNotification->put_NotificationId(pAlarm->m_notificationId);
        if (FAILED(hr))
        {
            //
            // handle failure case
            //
        }

        //
        // Set the Title property
        //
        hr = m_pNotification->put_Title(L"Alarm!!!");
        if (FAILED(hr))
        {
            //
            // handle failure case
            //
        }

        //
        // Set the Message property
        // Use the safe string functions so the buffer doesn't
        // get overrun.
        //
        hr = StringCchPrintf(wszMsg,
                             ARRAYSIZE(wszMsg),
                             L"Alarm %d set for %02d:%02d",
                             pAlarm->m_notificationId,
                             pAlarm->m_time.wHour,
                             pAlarm->m_time.wMinute);
        if (SUCCEEDED(hr))
        {           
            hr = m_pNotification->put_Message(wszMsg);
            if (FAILED(hr))
            {
                //
                // handle failure case
                //
            }
        }
        else
        {
            //
            // If StringCchPrintf failed above, just use a default message for
            // the alarm so that it doesn't appear blank.
            //
            hr = m_pNotification->put_Message(L"The Alarm just went off!");
            if (FAILED(hr))
            {
                //
                // handle failure case
                //
            }
        }

        //
        // Set the Notification to expire after 90 days
        //
        ::GetLocalTime(&expTime);

        FILETIME ft = { 0 };
        SystemTimeToFileTime(&expTime, &ft);

        ULARGE_INTEGER u = { 0 };
        memcpy(&u, &ft, sizeof(u));
        u.QuadPart += 90 * 24 * 60 * 60 * 10000000LLU;  // 90 days
        memcpy(&ft, &u, sizeof(ft));

        FileTimeToSystemTime(&ft, &expTime);

        hr = m_pNotification->put_ExpirationTime(&expTime);
        if (FAILED(hr))
        {
            //
            // handle failure case
            //
        }

        //
        // Show the notification on the device, and
        // revoke it when the user dismisses the alarm
        //
        if (NULL != m_pNotificationMgr)
        {
            m_pNotificationMgr->Show(m_pNotification);

            ::MessageBox(NULL, wszMsg, L"Alarm went off!", MB_OK);
            
            m_pNotificationMgr->Revoke(pAlarm->m_notificationId);
        }
    }
}

DWORD WINAPI CAlarmClient::TimeMonitor(LPVOID lpParam)
{    
    Alarm* pAlarm = (Alarm*)lpParam;

    if (NULL == pAlarm)
    {
        return 0;
    }

    SYSTEMTIME  sysTime;
    FILETIME    ftNow;
    FILETIME    ftAlarm;
    bool fNotificationShown = false;

    ::SystemTimeToFileTime(&(pAlarm->m_time), &ftAlarm);

    while (!fNotificationShown)
    {
        //
        // Get the local system time
        //
        ::GetLocalTime(&sysTime);
        ::SystemTimeToFileTime(&sysTime, &ftNow);

        //
        // Compare the times; if it is currently past the
        // time for the Alarm to fire, then show it.
        //
        if (::CompareFileTime(&ftNow, &ftAlarm) >= 0)
        {
            pAlarm->m_pClient->ShowNotification(pAlarm);
            fNotificationShown = true;
        }
        else
        {
            Sleep(1000);        
        }
    }
    
    return 0;
}

void CAlarmClient::StartAlarmThread(Alarm* pAlarm)
{
    HANDLE hThread = ::CreateThread(NULL, 
                                    0, 
                                    this->TimeMonitor, 
                                    pAlarm,
                                    0,
                                    NULL);

    if (NULL == hThread)
    {
        //
        // Failed to start alarm thread
        //
    }
    
}

void CAlarmClient::CreateNotification()
{
    if (NULL != m_pSession &&
        NULL == m_pNotificationMgr)
    {
        HRESULT hr = ::CoCreateInstance(CLSID_SideShowNotification,
                                        NULL,
                                        CLSCTX_INPROC_SERVER, 
                                        IID_PPV_ARGS(&m_pNotification));

        if (FAILED(hr))
        {
            //
            // Failed to CoCreate SideShowNotification
            //
        }
        else
        {
            //
            // Register with the platform so that we can provide Notifications
            // to the devices
            //
            hr = m_pSession->RegisterNotifications(ALARM_CLIENT_APPLICATION_ID,
                                                   &m_pNotificationMgr);

            if (S_OK == hr)
            {
                //
                // Successfully registered notifications with Windows SideShow platform.
                //
            }
            else if (S_FALSE == hr)
            {
                //
                // Successfully registered notifications with Windows SideShow platform,
                // but no devices available or enabled.
                //
            }
            else if (E_INVALIDARG == hr)
            {
                //
                // ISideShowSession::RegisterNotifications failed with Invalid Argument.
                // Registry data is missing.
                //
            }
            else 
            {
                //
                // ISideShowSession::RegisterNotifications failed.
                //
            }
        }       
    }
}

void CAlarmClient::Register()
{
    //
    // Create the SideShowSession object that
    // enables us to talk to the platform.
    //
    HRESULT hr = S_OK;

    if (NULL == m_pSession && 
        NULL == m_pContentMgr)
    {
        hr = ::CoCreateInstance(CLSID_SideShowSession,
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
            hr = m_pSession->RegisterContent(ALARM_CLIENT_APPLICATION_ID,
                                             SIDESHOW_ENDPOINT_SIMPLE_CONTENT_FORMAT,
                                             &m_pContentMgr);
            if (SUCCEEDED(hr))
            {
                //
                // Successfully registered with Windows SideShow platform,
                // now go register for Notifications
                //
                CreateNotification();
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

void CAlarmClient::Unregister()
{
    if (NULL != m_pNotification)
    {
        m_pNotification->Release();
        m_pNotification = NULL;
    }

    if (NULL != m_pNotificationMgr)
    {
        m_pNotificationMgr->Release();
        m_pNotificationMgr = NULL;
    }

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


void CAlarmClient::AddContent()
{
    if (NULL != m_pContentMgr)
    {
        //
        // Create an instance of the content object if it doesn't exist
        //
        if (NULL == m_pContent)
        {
            m_pContent = new CAlarmContent();
        }
        
        HRESULT hr = m_pContentMgr->Add(m_pContent);

        if (SUCCEEDED(hr))
        {
            //
            // Successfully added content
            //
        }
        else
        {
            //
            // Failed to add content
            //
        }
    }
}

void CAlarmClient::RemoveAllContent()
{
    if (NULL != m_pContentMgr)
    {
        HRESULT hr = m_pContentMgr->RemoveAll();
        if (SUCCEEDED(hr))
        {
            //
            // Removed all content.
            //
        }
        else
        {
            //
            // Failed to remove content.
            //
        }
    }  
}
