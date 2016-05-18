// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

#ifndef _WLANMGR_H_
#define _WLANMGR_H_

class CWlanStation : public CRefObject
{
private:
    DOT11_MAC_ADDRESS m_MacAddress;

    // Don't allow to create an empty CWlanStation object
    CWlanStation() {};

public:

    CWlanStation(const CWlanStation&);
    CWlanStation(const WLAN_HOSTED_NETWORK_PEER_STATE&);
    ~CWlanStation();

    BOOL operator==(const CWlanStation&);
    BOOL operator==(const DOT11_MAC_ADDRESS);
    
    VOID GetMacAddress(DOT11_MAC_ADDRESS&) const;
};


//
// Hosted network notification sink
//
class CHostedNetworkNotificationSink
{
public:

    virtual VOID OnHostedNetworkStarted() = 0;
    virtual VOID OnHostedNetworkStopped() = 0;
    virtual VOID OnHostedNetworkNotAvailable() = 0;
    virtual VOID OnHostedNetworkAvailable() = 0;
    virtual VOID OnStationJoin(CWlanStation *) = 0;
    virtual VOID OnStationLeave(CWlanStation *) = 0;
};

class CWlanManager
{
private:
    CRITICAL_SECTION m_CriticalSection;             // critical section to protect the object

    HANDLE m_CallbackComplete;                      // event to signal that a callback to the client completes

    HANDLE m_WlanHandle;                            // handle for WLANAPI
    DWORD m_ServerVersion;                          // version of wlansvc

    BOOL m_HostedNetworkAllowed;                    // is hosted network allowed?

    // connection settings for hosted network
    WLAN_HOSTED_NETWORK_CONNECTION_SETTINGS m_HostedNetworkConnSettings;

    // security settings for hosted network
    WLAN_HOSTED_NETWORK_SECURITY_SETTINGS m_HostedNetworkSecSettings;

    // state of hosted network
    WLAN_HOSTED_NETWORK_STATE m_HostedNetworkState;

    // list of stations that are connected to the hosted network
    CRefObjList<CWlanStation *> m_StationList;

    // a notification sink provided by the client to receivd notifications
    CHostedNetworkNotificationSink * m_NotificationSink;

    bool m_Initialized;                            // is wlan manager initialized?

    VOID Notify(const PWLAN_NOTIFICATION_DATA pNotifData);

    VOID OnHostedNetworkStarted();
    VOID OnHostedNetworkStopped();
    VOID OnHostedNetworkNotAvailable();
    VOID OnHostedNetworkAvailable();
    VOID OnStationJoin(const WLAN_HOSTED_NETWORK_PEER_STATE&);
    VOID OnStationLeave(DOT11_MAC_ADDRESS);
    VOID OnStationStateChange(const WLAN_HOSTED_NETWORK_PEER_STATE&);

    static VOID WINAPI WlanNotificationCallback(PWLAN_NOTIFICATION_DATA, PVOID);

    VOID Lock() {EnterCriticalSection(const_cast<LPCRITICAL_SECTION>(&m_CriticalSection));};
    VOID Unlock() {LeaveCriticalSection(const_cast<LPCRITICAL_SECTION>(&m_CriticalSection));};

    //
    // Send notification to client?
    // If yes, reset callback complete event and return a pointer to notification sink.
    // Otherwise return NULL.
    // Calls to this methos are serialized.
    // Caller is responsible for holding the critical section
    //
    CHostedNetworkNotificationSink * GetNotificationSink()
    {
        CHostedNetworkNotificationSink * sink = NULL;

        if (m_NotificationSink != NULL)
        {
            _ASSERT(m_CallbackComplete != NULL);

            ResetEvent(m_CallbackComplete);
            sink = m_NotificationSink;
        }

        return sink;
    };
public:
    CWlanManager();
    ~CWlanManager();

    HRESULT Init();

    HRESULT SetHostedNetworkName(CAtlString&);

    HRESULT GetHostedNetworkName(CAtlString&);

    HRESULT SetHostedNetworkKey(CAtlString&);

    HRESULT GetHostedNetworkKey(CAtlString&);

    HRESULT StartHostedNetwork();

    HRESULT StopHostedNetwork();

    HRESULT ForceStopHostedNetwork();

    HRESULT GetStaionList(CRefObjList<CWlanStation*>&);

    HRESULT GetHostedNetworkInterfaceGuid(GUID&);

    // register for notifications
    // an error is returned if notifications have been registered already
    HRESULT AdviseHostedNetworkNotification(CHostedNetworkNotificationSink *);

    // unregister for notifications
    HRESULT UnadviseHostedNetworkNotification();

    HRESULT IsHostedNetworkStarted(bool &);

};

#endif  // _WLANMGR_H_
