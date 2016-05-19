// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "stdafx.h"

VOID 
CNotificationSink::OnNewNotifcationAvailable(
    CHostedNetworkNotification * pNotification
    )
{
    bool fNotifyParent = false;

    _ASSERT(pNotification != NULL);
    if (pNotification != NULL)
    {
        Lock();
        
        m_NotificationList.AddTail(pNotification);

        // 
        // Check whether need to send a message to the parent window
        // Only post a message when the notification list becomes non-empty from empty.
        //
        fNotifyParent = (m_NotificationList.GetCount() == 1);

        Unlock();

        if (fNotifyParent)
        {
            // Post a message to the parent window for new notifications
            ::PostMessage(m_hParent, WM_NEW_HN_NOTIFICATION, 0, 0);
        }
    }
}

VOID 
CNotificationSink::DiscoverDevice(
    CWlanDevice * pDevice
    )
{
    // some higher level methods can be used for device discovery.
}