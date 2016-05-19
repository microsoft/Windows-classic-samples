#pragma once

#ifndef _NOTIF_H_
#define _NOTIF_H_

//
// Customized windows message
//
#define WM_NEW_HN_NOTIFICATION    (WM_APP + 1)

// notification object
class CHostedNetworkNotification : public CRefObject
{
public:
    CHostedNetworkNotification(UINT Type, CRefObject * Data = NULL)
    {
        m_Type = Type;

        m_Data = Data;

        if (m_Data != NULL)
        {
            m_Data->AddRef();
        }
    };

    ~CHostedNetworkNotification()
    {
        if (m_Data != NULL)
        {
            m_Data->Release();
            m_Data = NULL;
        }
    };

    // notification types
    static const UINT HostedNetworkStarted        = 1;
    static const UINT HostedNetworkStopped        = 2;
    static const UINT DeviceAdd                   = 3;
    static const UINT DeviceRemove                = 4;
    static const UINT DeviceUpdate                = 5;
    static const UINT HostedNetworkNotAvailable   = 6;
    static const UINT HostedNetworkAvailable      = 7;

    UINT GetNotificationType() {return m_Type;};

    //
    // Get notification data.
    // The caller is responsible for releasing the data after using it.
    //
    CRefObject * GetNotificationData() {
        if (m_Data != NULL)
        {
            m_Data->AddRef();
            return m_Data;
        }
        return NULL;
    }

private:
    CHostedNetworkNotification() : m_Type (0), m_Data(NULL) {};
    // notification type
    UINT m_Type;
    // notification data
    CRefObject * m_Data;
};

class CNotificationSink : public CHostedNetworkNotificationSink
{
private:
    // List of notifications
    CRefObjList<CHostedNetworkNotification *> m_NotificationList;

    // List of devices to be discovered
    CRefObjList<CWlanDevice *> m_UndiscoveredDeviceList;

    // Parent window
    HWND m_hParent;

    CRITICAL_SECTION m_CriticalSection;            // critical section to protect the notification list

    VOID Lock() {EnterCriticalSection(const_cast<LPCRITICAL_SECTION>(&m_CriticalSection));};
    VOID Unlock() {LeaveCriticalSection(const_cast<LPCRITICAL_SECTION>(&m_CriticalSection));};

    // Don't allow to create an empty CNotificationSink object
    CNotificationSink() {InitializeCriticalSection(const_cast<LPCRITICAL_SECTION>(&m_CriticalSection));};

    VOID OnNewNotifcationAvailable(CHostedNetworkNotification *);

    VOID DiscoverDevice(CWlanDevice *);

    volatile LONG m_OutstandingThread;

public:

    CNotificationSink(HWND hParent) 
    {
        m_hParent = hParent;
        InitializeCriticalSection(const_cast<LPCRITICAL_SECTION>(&m_CriticalSection));
    };

    ~CNotificationSink()
    {
        Lock();
        m_UndiscoveredDeviceList.RemoveAllEntries();
        Unlock();

        while (m_OutstandingThread > 0)
        {
            Sleep(100);
        }

        DeleteCriticalSection(const_cast<LPCRITICAL_SECTION>(&m_CriticalSection));
    };

    virtual VOID OnHostedNetworkStarted()
    {
        CHostedNetworkNotification * pNotification = new(std::nothrow) CHostedNetworkNotification(
                                                        CHostedNetworkNotification::HostedNetworkStarted
                                                        );

        OnNewNotifcationAvailable(pNotification);
    };

    virtual VOID OnHostedNetworkStopped()
    {
        CHostedNetworkNotification * pNotification = new(std::nothrow) CHostedNetworkNotification(
                                                        CHostedNetworkNotification::HostedNetworkStopped
                                                        );

        OnNewNotifcationAvailable(pNotification);
    };

    virtual VOID OnHostedNetworkNotAvailable()
    {
        CHostedNetworkNotification * pNotification = new(std::nothrow) CHostedNetworkNotification(
                                                        CHostedNetworkNotification::HostedNetworkNotAvailable
                                                        );

        OnNewNotifcationAvailable(pNotification);
    };

    virtual VOID OnHostedNetworkAvailable()
    {
        CHostedNetworkNotification * pNotification = new(std::nothrow) CHostedNetworkNotification(
                                                        CHostedNetworkNotification::HostedNetworkAvailable
                                                        );

        OnNewNotifcationAvailable(pNotification);
    };
    virtual VOID OnStationJoin(CWlanStation * pStation)
    {
        DOT11_MAC_ADDRESS macAddress;
        pStation->GetMacAddress(macAddress);

        // Create a Wlan device
        CWlanDevice * pDevice = new(std::nothrow) CWlanDevice(macAddress);

        if (pDevice != NULL)
        {
            CHostedNetworkNotification * pNotification = new(std::nothrow) CHostedNetworkNotification(
                                                            CHostedNetworkNotification::DeviceAdd,
                                                            pDevice
                                                            );

            OnNewNotifcationAvailable(pNotification);

            // discover the device
            DiscoverDevice(pDevice);

            pDevice->Release();
            pDevice = NULL;
        }
    };

    virtual VOID OnStationLeave(CWlanStation * pStation)
    {
        DOT11_MAC_ADDRESS macAddress;
        pStation->GetMacAddress(macAddress);

        // Create a Wlan device
        CWlanDevice * pDevice = new(std::nothrow) CWlanDevice(macAddress);

        if (pDevice != NULL)
        {
            CHostedNetworkNotification * pNotification = new(std::nothrow) CHostedNetworkNotification(
                                                            CHostedNetworkNotification::DeviceRemove,
                                                            pDevice
                                                            );

            OnNewNotifcationAvailable(pNotification);

            pDevice->Release();
            pDevice = NULL;
        }
    };

    // 
    // Get next hosted network notification
    // Caller is responsible for releasing the notification after using it.
    // The caller is required to process all notifications till the queue is empty
    // upon receiving one WM_NEW_HN_NOTIFICATION.
    //
    CHostedNetworkNotification * GetNextNotification()
    {
        CHostedNetworkNotification * pNotification = NULL;

        Lock();

        if ( m_NotificationList.GetCount() != 0 )
        {
            pNotification = m_NotificationList.RemoveHead();
        }

        Unlock();

        return pNotification;
    };

};

#endif  // _NOTIF_H_
