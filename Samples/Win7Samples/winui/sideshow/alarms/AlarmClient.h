// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#pragma once

class CAlarmClient;

struct Alarm
{
    Alarm()
    {
        m_bSet = FALSE;
        m_notificationId = 0;
        m_pClient = NULL;
        m_pNext = NULL;
    }

    SYSTEMTIME      m_time;
    BOOL            m_bSet;
    NOTIFICATION_ID m_notificationId;
    CAlarmClient    *m_pClient;
    Alarm           *m_pNext;
};

class CAlarmClient
{
private:
    ISideShowSession                *m_pSession;
    ISideShowContentManager         *m_pContentMgr;
    ISideShowContent                *m_pContent;
    ISideShowNotificationManager    *m_pNotificationMgr;
    ISideShowNotification           *m_pNotification;

    Alarm                           *m_pAlarms;
    NOTIFICATION_ID                  m_lastNotificationId;

protected:
    void StartAlarmThread(Alarm* pAlarm);
    void ShowNotification(Alarm* pAlarm);
    static DWORD WINAPI TimeMonitor(LPVOID lpParam);
    void CreateNotification();

public:
    CAlarmClient();
    virtual ~CAlarmClient();

    void Register();
    void Unregister();

    void AddContent();
    void RemoveAllContent();

    void SetAlarm(SYSTEMTIME* pAlarmTime);
};
