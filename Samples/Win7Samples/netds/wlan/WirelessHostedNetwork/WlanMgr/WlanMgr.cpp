// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "stdafx.h"

//
// WLAN notification callback
//
VOID WINAPI 
CWlanManager::WlanNotificationCallback(
    PWLAN_NOTIFICATION_DATA pNotifData,
    PVOID pContext
    )
{
    CWlanManager* pWlanManager = (CWlanManager *)pContext;

    pWlanManager->Notify(pNotifData);
}

// CWlanManager
CWlanManager::CWlanManager() : 
    m_WlanHandle(NULL),
    m_NotificationSink(NULL),
    m_CallbackComplete(NULL),
    m_Initialized(false)
{
    // initialize critical section first, throw exception
    InitializeCriticalSection(&m_CriticalSection);
}

CWlanManager::~CWlanManager()
{
    // Unadvise, ignore the error
    UnadviseHostedNetworkNotification();

    if (m_WlanHandle != NULL)
    {
        WlanCloseHandle(m_WlanHandle, NULL);
        m_WlanHandle = NULL;
    }
    
    if (m_CallbackComplete != NULL)
    {
        CloseHandle(m_CallbackComplete);
        m_CallbackComplete = NULL;
    }

    DeleteCriticalSection(&m_CriticalSection);
}

HRESULT 
CWlanManager::Init()
{
    HRESULT hr = S_OK;
    DWORD retCode = ERROR_SUCCESS;
    DWORD dwDataSize = 0;
    BOOL *pbMode = NULL;                                                    // whether hosted network is allowed or not
    PWLAN_HOSTED_NETWORK_CONNECTION_SETTINGS pConnSettings = NULL;          // hosted network connectivity settings
    PWLAN_HOSTED_NETWORK_SECURITY_SETTINGS pSecSettings = NULL;             // hosted network security settings
    PWLAN_HOSTED_NETWORK_STATUS pAPStatus = NULL;                           // hosted network status
    WLAN_OPCODE_VALUE_TYPE valueType;

    Lock();

    if (m_Initialized)
    {
        //
        // no-op because it is already initialized
        //
        BAIL();
    }

    // open a wlan handle first
    retCode = WlanOpenHandle(
                WLAN_API_VERSION,
                NULL,           // reserved
                &m_ServerVersion,
                &m_WlanHandle
                );

    BAIL_ON_WIN32_ERROR(retCode, hr);

    // register notifications
    retCode = WlanRegisterNotification(
                m_WlanHandle,
                WLAN_NOTIFICATION_SOURCE_HNWK,
                TRUE,
                &CWlanManager::WlanNotificationCallback,
                this,
                NULL,       // reserved
                NULL
                );
    BAIL_ON_WIN32_ERROR(retCode, hr);

    //
    // Initialize the hosted network.
    // It is a no-op if the hosted network is already initialized.
    // Bail out if it fails.
    //
    retCode = WlanHostedNetworkInitSettings(
                m_WlanHandle,
                NULL,
                NULL        // reserved
                );
    BAIL_ON_WIN32_ERROR(retCode, hr);

    //
    // Is hosted network enabled?
    //
    retCode = WlanHostedNetworkQueryProperty(
                m_WlanHandle,
                wlan_hosted_network_opcode_enable,
                &dwDataSize,
                (PVOID *)&pbMode,
                &valueType,
                NULL        // reserved
                );
    BAIL_ON_WIN32_ERROR(retCode, hr);

    if(!pbMode || dwDataSize < sizeof(BOOL))
    {
        BAIL_ON_WIN32_ERROR(ERROR_INVALID_DATA, hr);
    }    

    //
    // get the hosted network connectivity settings
    //
    retCode = WlanHostedNetworkQueryProperty(
                m_WlanHandle,
                wlan_hosted_network_opcode_connection_settings,
                &dwDataSize,
                (PVOID *)&pConnSettings,
                &valueType,
                NULL        // reserved
                );
    BAIL_ON_WIN32_ERROR(retCode, hr);

    if( !pConnSettings || dwDataSize < sizeof(WLAN_HOSTED_NETWORK_CONNECTION_SETTINGS))
    {
        BAIL_ON_WIN32_ERROR(ERROR_INVALID_DATA, hr);
    }
    
    //
    // get the hosted network seucrity settings
    //
    retCode = WlanHostedNetworkQueryProperty(
                m_WlanHandle,
                wlan_hosted_network_opcode_security_settings,
                &dwDataSize,
                (PVOID *)&pSecSettings,
                &valueType,
                NULL        // reserved
                );
    BAIL_ON_WIN32_ERROR(retCode, hr);

    if( !pSecSettings || dwDataSize < sizeof(WLAN_HOSTED_NETWORK_SECURITY_SETTINGS))
    {
        BAIL_ON_WIN32_ERROR(ERROR_INVALID_DATA, hr);
    }

    //
    // get the hosted network status
    //
    retCode = WlanHostedNetworkQueryStatus(
                m_WlanHandle,
                &pAPStatus,
                NULL        // reserved
                );
    BAIL_ON_WIN32_ERROR(retCode, hr);

    //
    // save the values
    //
    m_HostedNetworkAllowed = *pbMode;
    m_HostedNetworkConnSettings = *pConnSettings;
    m_HostedNetworkSecSettings = *pSecSettings;
    m_HostedNetworkState = pAPStatus->HostedNetworkState;

    //
    // add existing stations if the hosted network has started already
    //
    if (wlan_hosted_network_active == pAPStatus->HostedNetworkState)
    {
        for (DWORD i = 0; i < pAPStatus->dwNumberOfPeers; i++)
        {
            OnStationJoin(pAPStatus->PeerList[i]);
        }

    }

    m_Initialized = true;

error:

    if (retCode != ERROR_SUCCESS && m_WlanHandle != NULL)
    {
        //
        // Close WLAN handle in failure cases
        //
        WlanCloseHandle(m_WlanHandle, NULL);
        m_WlanHandle = NULL;
    }

    Unlock();

    if (pbMode != NULL)
    {
        WlanFreeMemory(pbMode);
        pbMode = NULL;
    }

    if (pConnSettings != NULL)
    {
        WlanFreeMemory(pConnSettings);
        pConnSettings = NULL;
    }

    if (pSecSettings != NULL)
    {
        WlanFreeMemory(pSecSettings);
        pSecSettings = NULL;
    }

    if (pAPStatus != NULL)
    {
        WlanFreeMemory(pAPStatus);
        pAPStatus = NULL;
    }

    return hr;
}

// process the notifications received from hosted network
VOID 
CWlanManager::Notify(
    const PWLAN_NOTIFICATION_DATA pNotifData
    )
{
    if (pNotifData != NULL && WLAN_NOTIFICATION_SOURCE_HNWK == pNotifData->NotificationSource)
    {
        //
        // Only look at hosted network notifications
        //
        switch (pNotifData->NotificationCode)
        {
        case wlan_hosted_network_state_change:
            if (sizeof(WLAN_HOSTED_NETWORK_STATE_CHANGE) == pNotifData->dwDataSize && pNotifData->pData != NULL)
            {
                PWLAN_HOSTED_NETWORK_STATE_CHANGE pStateChange = (PWLAN_HOSTED_NETWORK_STATE_CHANGE)pNotifData->pData;
                
                switch (pStateChange->NewState)
                {
                case wlan_hosted_network_active:
                    OnHostedNetworkStarted();
                    break;
                case wlan_hosted_network_idle:
                    if (wlan_hosted_network_active == pStateChange->OldState)
                    {
                        OnHostedNetworkStopped();
                    }
                    else
                    {
                        OnHostedNetworkAvailable();
                    }
                    break;
                case wlan_hosted_network_unavailable:
                    if (wlan_hosted_network_active == pStateChange->OldState)
                    {
                        OnHostedNetworkStopped();
                    }
                    OnHostedNetworkNotAvailable();
                    break;
                }
            }
            else
            {
                //
                // Shall NOT happen
                //
                _ASSERT(FALSE);
            }
            break;

        case wlan_hosted_network_peer_state_change:
            if (sizeof(WLAN_HOSTED_NETWORK_DATA_PEER_STATE_CHANGE) == pNotifData->dwDataSize && pNotifData->pData != NULL)
            {
                PWLAN_HOSTED_NETWORK_DATA_PEER_STATE_CHANGE pPeerStateChange = (PWLAN_HOSTED_NETWORK_DATA_PEER_STATE_CHANGE)pNotifData->pData;
                
                if (wlan_hosted_network_peer_state_authenticated == pPeerStateChange->NewState.PeerAuthState)
                {
                    //
                    // A station joined the hosted network
                    //
                    OnStationJoin(pPeerStateChange->NewState);
                }
                else if (wlan_hosted_network_peer_state_invalid == pPeerStateChange->NewState.PeerAuthState)
                {
                    //
                    // A station left the hosted network
                    //
                    OnStationLeave(pPeerStateChange->NewState.PeerMacAddress);
                }
                else
                {
                    //
                    // The authentication state changed
                    //
                    OnStationStateChange(pPeerStateChange->NewState);
                }
            }
            else
            {
                //
                // Shall NOT happen
                //
                _ASSERT(FALSE);
            }
            break;
        
        case wlan_hosted_network_radio_state_change:
            if (sizeof(WLAN_HOSTED_NETWORK_RADIO_STATE) == pNotifData->dwDataSize && pNotifData->pData != NULL)
            {
                // PWLAN_HOSTED_NETWORK_RADIO_STATE pRadioState = (PWLAN_HOSTED_NETWORK_RADIO_STATE)pNotifData->pData;

                //
                // Do nothing for now
                //
            }
            else
            {
                //
                // Shall NOT happen
                //
                _ASSERT(FALSE);
            }
            break;
        }
    }
}

VOID 
CWlanManager::OnHostedNetworkStarted()
{
    CHostedNetworkNotificationSink * pSink = NULL;

    Lock();

    //
    // Change hosted network state
    //
    m_HostedNetworkState = wlan_hosted_network_active;

    pSink = GetNotificationSink();

    Unlock();

    if (pSink != NULL)
    {
        //
        // Notify client
        //
        pSink->OnHostedNetworkStarted();

        _ASSERT(m_CallbackComplete != NULL);

        //
        // Signal callback complete
        //
        SetEvent(m_CallbackComplete);
    }
}
                             
VOID 
CWlanManager::OnHostedNetworkStopped()
{
    CHostedNetworkNotificationSink * pSink = NULL;
    CRefObjList<CWlanStation *> stationList;

    Lock();

    // Save all stations that haven't left the network
    while ( 0 != m_StationList.GetCount() )
    {
        CWlanStation* pStation = m_StationList.RemoveHead();

        stationList.AddTail(pStation);
    }

    m_HostedNetworkState = wlan_hosted_network_idle;
    pSink = GetNotificationSink();

    Unlock();

    if (pSink != NULL)
    {
        //
        // Notify client
        //

        // notify station leave
        while ( 0 != stationList.GetCount() )
        {
            CWlanStation* pStation = stationList.RemoveHead();

            pSink->OnStationLeave(pStation);

            pStation->Release();

            pStation = NULL;
        }

        pSink->OnHostedNetworkStopped();

        _ASSERT(m_CallbackComplete != NULL);

        //
        // Signal callback complete
        //
        SetEvent(m_CallbackComplete);
    }
}

VOID 
CWlanManager::OnHostedNetworkNotAvailable()
{
    CHostedNetworkNotificationSink * pSink = NULL;

    Lock();

    //
    // Change hosted network state
    //
    m_HostedNetworkState = wlan_hosted_network_unavailable;

    pSink = GetNotificationSink();

    Unlock();

    if (pSink != NULL)
    {
        //
        // Notify client
        //
        pSink->OnHostedNetworkNotAvailable();

        _ASSERT(m_CallbackComplete != NULL);

        //
        // Signal callback complete
        //
        SetEvent(m_CallbackComplete);
    }
}

VOID 
CWlanManager::OnHostedNetworkAvailable()
{
    CHostedNetworkNotificationSink * pSink = NULL;

    Lock();

    //
    // Change hosted network state
    //
    m_HostedNetworkState = wlan_hosted_network_idle;

    pSink = GetNotificationSink();

    Unlock();

    if (pSink != NULL)
    {
        //
        // Notify client
        //
        pSink->OnHostedNetworkAvailable();

        _ASSERT(m_CallbackComplete != NULL);

        //
        // Signal callback complete
        //
        SetEvent(m_CallbackComplete);
    }
}

VOID 
CWlanManager::OnStationJoin(
    const WLAN_HOSTED_NETWORK_PEER_STATE& StationState
    )
{
    CHostedNetworkNotificationSink * pSink = NULL;
    CWlanStation * pStation = new(std::nothrow) CWlanStation(StationState);

    if (pStation != NULL)
    {
        Lock();

        //
        // The station should not be in the station list
        //
        _ASSERT(!m_StationList.IsInArray(pStation));

        pStation->AddRef();
        m_StationList.AddTail(pStation);

        pSink = GetNotificationSink();

        Unlock();

        if (pSink != NULL)
        {
            //
            // Notify client
            //
            pSink->OnStationJoin(pStation);

            _ASSERT(m_CallbackComplete != NULL);

            //
            // Signal callback complete
            //
            SetEvent(m_CallbackComplete);
        }

        pStation->Release();
        pStation = NULL;
    }
}

VOID 
CWlanManager::OnStationLeave(
    DOT11_MAC_ADDRESS MacAddress
    )
{
    CHostedNetworkNotificationSink * pSink = NULL;
    CWlanStation * pStation = NULL;

    Lock();

    //
    // Find the station and remove it from the station list
    //
    for (size_t i = 0; i < m_StationList.GetCount(); i++)
    {
        POSITION pos = m_StationList.FindIndex(i);
        CWlanStation* pTmpStation = m_StationList.GetAt(pos);
        if (*pTmpStation == MacAddress)
        {
            //
            // Found the station, remove it from the list
            //
            m_StationList.RemoveAt(pos);
            pStation = pTmpStation;
            break;
        }
    }

    //
    // The station should be in the station list
    //
    _ASSERT(pStation != NULL);

    if (pStation != NULL)
    {
        pSink = GetNotificationSink();
    }

    Unlock();

    if (pSink != NULL)
    {
        //
        // Notify client
        //
        pSink->OnStationLeave(pStation);

        _ASSERT(m_CallbackComplete != NULL);

        //
        // Signal callback complete
        //
        SetEvent(m_CallbackComplete);
    }

    if (pStation != NULL)
    {
        pStation->Release();
        pStation = NULL;
    }
}

VOID 
CWlanManager::OnStationStateChange(
    const WLAN_HOSTED_NETWORK_PEER_STATE&
    )
{
    //
    // It shall NOT happen for now
    //
    _ASSERT(FALSE);
}

HRESULT 
CWlanManager::SetHostedNetworkName(
    CAtlString& strSsid
    )
{
    HRESULT hr = S_OK;
    DWORD dwError = ERROR_SUCCESS;
    DOT11_SSID ssid, oldSsid;

    //
    // Convert string to SSID
    //
    dwError = StringToSsid(strSsid, &ssid);
    BAIL_ON_WIN32_ERROR(dwError, hr);

    Lock();
    if (m_Initialized)
    {
        // save old SSID
        oldSsid = m_HostedNetworkConnSettings.hostedNetworkSSID;
                
        m_HostedNetworkConnSettings.hostedNetworkSSID = ssid;

        // set the new connection setttings
        dwError = WlanHostedNetworkSetProperty(
                    m_WlanHandle,
                    wlan_hosted_network_opcode_connection_settings,
                    sizeof( WLAN_HOSTED_NETWORK_CONNECTION_SETTINGS),
                    (PVOID)&m_HostedNetworkConnSettings,
                    NULL,
                    NULL
                    );
        if (dwError != ERROR_SUCCESS)
        {
            // revert back to old SSID
            m_HostedNetworkConnSettings.hostedNetworkSSID = oldSsid;
            hr = HRESULT_FROM_WIN32(dwError);
        }
    }
    else
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_STATE);
    }
    Unlock();
    
error:

    return hr;
}

HRESULT 
CWlanManager::GetHostedNetworkName(
    CAtlString& strSsid
    )
{
    DWORD dwError = ERROR_SUCCESS;
    WCHAR wszSsid[WLAN_MAX_NAME_LENGTH];
    DWORD dwSsidStrLen = WLAN_MAX_NAME_LENGTH;

    Lock();
    wszSsid[0] = L'\0';
    if (m_Initialized)
    {
        //
        // Convert SSID to string
        //
        dwError = SsidToDisplayName(
                    &m_HostedNetworkConnSettings.hostedNetworkSSID,
                    TRUE,
                    wszSsid,
                    &dwSsidStrLen
                    );

        if (ERROR_SUCCESS == dwError)
        {
            strSsid = wszSsid;
        }
    }
    else
    {
        dwError = ERROR_INVALID_STATE;
    }

    Unlock();


    return HRESULT_FROM_WIN32(dwError);
}

HRESULT 
CWlanManager::SetHostedNetworkKey(
    CAtlString& strKey
    )
{
    HRESULT hr = S_OK;
    DWORD dwError = ERROR_SUCCESS;
    UCHAR strKeyBuf[WLAN_MAX_NAME_LENGTH];
    DWORD dwKeyBufLen = WLAN_MAX_NAME_LENGTH;

    dwError = ConvertPassPhraseKeyStringToBuffer(
                strKey,
                strKey.GetLength(),
                DOT11_AUTH_ALGO_RSNA_PSK,
                strKeyBuf,
                &dwKeyBufLen
                );

    BAIL_ON_WIN32_ERROR(dwError, hr);

    if (0 == dwKeyBufLen)
    {
        BAIL_ON_WIN32_ERROR(ERROR_INVALID_PARAMETER, hr);
    }

    dwError = WlanHostedNetworkSetSecondaryKey(
                m_WlanHandle,
                dwKeyBufLen,
                strKeyBuf,
                TRUE,           // passphrase
                TRUE,           // persistent
                NULL,           // not interested in failure reason
                NULL            // reserved
                );

    BAIL_ON_WIN32_ERROR(dwError, hr);

error:
    return hr;
}

HRESULT 
CWlanManager::GetHostedNetworkKey(
    CAtlString& strKey
    )
{
    HRESULT hr = S_OK;
    DWORD dwError = ERROR_SUCCESS;

    BOOL bIsPassPhrase = FALSE;
    BOOL bPersistent = FALSE;
    PUCHAR pucSecondaryKey = NULL;
    DWORD dwSecondaryKeyLength = 0;
    WCHAR strSecondaryKey[WLAN_MAX_NAME_LENGTH];
    
    // get the user security key
    dwError = WlanHostedNetworkQuerySecondaryKey(
                m_WlanHandle,
                &dwSecondaryKeyLength,
                &pucSecondaryKey,
                &bIsPassPhrase,
                &bPersistent,
                NULL,
                NULL
                );

    BAIL_ON_WIN32_ERROR(dwError, hr);

    int cchKey = 0;
    if (dwSecondaryKeyLength > 0)
    {
        // Must be passphrase
        _ASSERT(bIsPassPhrase);
        // convert the key
        if (bIsPassPhrase)
        {
            #pragma prefast(suppress:26035, "If the key is a pass phrase, it is guaranteed to be null-terminated.")
            cchKey = MultiByteToWideChar(
                        CP_ACP, 
                        MB_ERR_INVALID_CHARS,
                        (LPCSTR)pucSecondaryKey,
                        dwSecondaryKeyLength,
                        strSecondaryKey, 
                        sizeof(strSecondaryKey) / sizeof(strSecondaryKey[0]) -1
                        );
        }
    }

    if(cchKey == 0)
    {
        // secondary key is not set or not passphrase
        // set a temporary one
        CAtlString strTmpKey = L"HostedNetwork12345";

        hr = SetHostedNetworkKey(strTmpKey);
        BAIL_ON_FAILURE(hr);
        strKey = strTmpKey;
    }
    else
    {
        // got the key
        strKey = strSecondaryKey;
    }

error:    
    if (pucSecondaryKey != NULL)
    {
        WlanFreeMemory(pucSecondaryKey);
        pucSecondaryKey = NULL;
    }

    return hr;
}

HRESULT 
CWlanManager::StartHostedNetwork()
{
    HRESULT hr = S_OK;
    DWORD dwError = ERROR_SUCCESS;

    Lock();

    if (!m_Initialized)
    {
        BAIL_ON_WIN32_ERROR(ERROR_INVALID_STATE, hr);
    }

//    if (wlan_hosted_network_active == m_HostedNetworkState)
//    {
//        BAIL_ON_WIN32_ERROR(ERROR_INVALID_STATE, hr);
//    }

    //
    // Start hosted network
    //
    dwError = WlanHostedNetworkStartUsing(
                m_WlanHandle,
                NULL,
                NULL
                );
    BAIL_ON_WIN32_ERROR(dwError, hr);

error:
    Unlock();

    return hr;
}

HRESULT 
CWlanManager::StopHostedNetwork()
{
    HRESULT hr = S_OK;

    DWORD dwError = ERROR_SUCCESS;

    Lock();

    if (!m_Initialized)
    {
        BAIL_ON_WIN32_ERROR(ERROR_INVALID_STATE, hr);
    }

    if (m_HostedNetworkState != wlan_hosted_network_active)
    {
        BAIL_ON_WIN32_ERROR(ERROR_INVALID_STATE, hr);
    }

    //
    // Stop hosted network
    //
    dwError = WlanHostedNetworkStopUsing(
                m_WlanHandle,
                NULL,
                NULL
                );
    BAIL_ON_WIN32_ERROR(dwError, hr);

error:
    Unlock();

    return hr;
}

HRESULT 
CWlanManager::ForceStopHostedNetwork()
{
    HRESULT hr = S_OK;

    DWORD dwError = ERROR_SUCCESS;

    Lock();

    if (!m_Initialized)
    {
        BAIL_ON_WIN32_ERROR(ERROR_INVALID_STATE, hr);
    }

    if (m_HostedNetworkState != wlan_hosted_network_active)
    {
        BAIL_ON_WIN32_ERROR(ERROR_INVALID_STATE, hr);
    }

    //
    // Stop hosted network
    //
    dwError = WlanHostedNetworkForceStop(
                m_WlanHandle,
                NULL,
                NULL
                );
    BAIL_ON_WIN32_ERROR(dwError, hr);

error:
    Unlock();

    return hr;

}

HRESULT 
CWlanManager::GetStaionList(
    CRefObjList<CWlanStation*>& StationList
    )
{
    HRESULT hr = S_OK;

    Lock();

    if (!m_Initialized)
    {
        BAIL_ON_WIN32_ERROR(ERROR_INVALID_STATE, hr);
    }

    if (m_HostedNetworkState != wlan_hosted_network_active)
    {
        BAIL_ON_WIN32_ERROR(ERROR_INVALID_STATE, hr);
    }

    //
    // Remove old entries
    //
    StationList.RemoveAllEntries();

    //
    // Copy all the stations to the list
    //
    for (size_t i = 0; i < m_StationList.GetCount(); i++)
    {
        CWlanStation* pStation = m_StationList.GetAt(m_StationList.FindIndex(i));
        _ASSERT(pStation != NULL);

        pStation->AddRef();
        StationList.AddTail(pStation);
    }

error:
    Unlock();

    return hr;
}

HRESULT 
CWlanManager::GetHostedNetworkInterfaceGuid(
    GUID& InterfaceGuid
    )
{
    HRESULT hr = S_OK;
    DWORD dwError = ERROR_SUCCESS;
    PWLAN_HOSTED_NETWORK_STATUS pAPStatus = NULL;                        // hosted network status

    //
    // get the hosted network status
    //
    dwError = WlanHostedNetworkQueryStatus(
                m_WlanHandle,
                &pAPStatus,
                NULL        // reserved
                );
    BAIL_ON_WIN32_ERROR(dwError, hr);

    InterfaceGuid = pAPStatus->IPDeviceID;

error:
    if (pAPStatus != NULL)
    {
        WlanFreeMemory(pAPStatus);
        pAPStatus = NULL;
    }

    return hr;
}

HRESULT 
CWlanManager::AdviseHostedNetworkNotification(
    CHostedNetworkNotificationSink * pSink
    )
{
    HRESULT hr = S_OK;

    Lock();

    if (NULL == pSink)
    {
        BAIL_ON_HRESULT_ERROR(hr, E_INVALIDARG);
    }

    if (!m_Initialized)
    {
        BAIL_ON_WIN32_ERROR(ERROR_INVALID_STATE, hr);
    }

    if (m_NotificationSink != NULL)
    {
        BAIL_ON_HRESULT_ERROR(hr, E_FAIL);
    }

    //
    // Create callback complete event if needed
    //
    if (NULL == m_CallbackComplete)
    {
        m_CallbackComplete = CreateEvent(
                                    NULL,
                                    FALSE,        // manual reset
                                    TRUE,        // initial state
                                    NULL
                                    );

        if (NULL == m_CallbackComplete)
        {
            BAIL_ON_LAST_ERROR(hr);
        }
    }
    
    //
    // Set notification sink
    //
    m_NotificationSink = pSink;
    
    //
    // check if hosted network is supported
    //
    if (wlan_hosted_network_unavailable == m_HostedNetworkState)
    {
        pSink->OnHostedNetworkNotAvailable();
    }

    //
    // Check if hosted network is enabled
    //
    if (wlan_hosted_network_active == m_HostedNetworkState)
    {
        for (unsigned int i = 0; i < m_StationList.GetCount(); i++)
        {
            CWlanStation * pStation = m_StationList.GetAt(m_StationList.FindIndex(i));

            _ASSERT(pStation != NULL);

            pSink->OnStationJoin(pStation);
        }
    }

error:
    Unlock();

    return hr;
}

HRESULT 
CWlanManager::UnadviseHostedNetworkNotification()
{
    HRESULT hr = S_OK;
    HANDLE eCallbackComplete = NULL;

    Lock();

    if (m_NotificationSink != NULL)
    {
        m_NotificationSink = NULL;

        _ASSERT(m_CallbackComplete != NULL);
        eCallbackComplete = m_CallbackComplete;
    }
    else
    {
        hr = E_FAIL;
    }
    
    Unlock();

    //
    // Wait for callback to complete
    //
    if (eCallbackComplete != NULL)
    {
        WaitForSingleObject(eCallbackComplete, INFINITE);
    }

    return hr;
}

HRESULT 
CWlanManager::IsHostedNetworkStarted(
    bool & fStarted
    )
{
    HRESULT hr = S_OK;

    Lock();
    
    if (m_Initialized)
    {
        fStarted = (wlan_hosted_network_active == m_HostedNetworkState);
    }
    else
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_STATE);
    }

    Unlock();


    return hr;
}
