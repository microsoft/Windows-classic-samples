// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "NetCostEventSink.h"
#include <new>

//********************************************************************************************
// Function: StartListeningForEvents
//
// Description: Creates the CNetCostEventSink object, and start a thread to perform an advise on it
//
//********************************************************************************************
HRESULT CNetCostEventSink::StartListeningForEvents(_In_ REFIID riid, _In_opt_ DESTINATION_INFO *pDestAddress, _Outptr_ CNetCostEventSink** ppSinkCostMgr)
{
    HRESULT hr = S_OK;
    if (ppSinkCostMgr == NULL)
    {
        hr = E_POINTER;
    }
    else
    {
        *ppSinkCostMgr = NULL;

        // Create our CNetCostEventSink object that will be used to advise to the Connection point
        CNetCostEventSink *pMgrEventSink = new (std::nothrow) CNetCostEventSink(pDestAddress, riid);   
        
        if (pMgrEventSink)
        {
            pMgrEventSink->m_hThread = CreateThread(NULL, 
                                                0, 
                                                &StartListeningForEventsThread, 
                                                pMgrEventSink, 
                                                0, 
                                                &(pMgrEventSink->m_dwThreadId));

            if (pMgrEventSink->m_hThread == INVALID_HANDLE_VALUE)
            {
                DWORD dwError = GetLastError();
                hr = HRESULT_FROM_WIN32(dwError);
            }

            if (SUCCEEDED(hr))
            {
                *ppSinkCostMgr = pMgrEventSink;
                (*ppSinkCostMgr)->AddRef();
            }

            pMgrEventSink->Release();
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    return hr;
}


//********************************************************************************************
// Function: StartListeningForEventsThread
//
// Description: This is our thread entry proc for the thread that will listen on events.
//
//********************************************************************************************
DWORD WINAPI CNetCostEventSink::StartListeningForEventsThread(_In_ LPVOID pArg)
{
    HRESULT hr = S_OK;

    HRESULT hrCoinit = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (SUCCEEDED(hrCoinit) || (RPC_E_CHANGED_MODE == hrCoinit))
    {
        CNetCostEventSink* pThis = reinterpret_cast<CNetCostEventSink*>(pArg);
        hr = pThis->ListenForEvents();

        if (RPC_E_CHANGED_MODE != hrCoinit)
        {
            CoUninitialize();
        }
    }
    else
    {
        hr = hrCoinit;
    }
    return hr;
}

//********************************************************************************************
// Function: ListenForEvents
//
// Description: The main listener function. Listens, and waits on a Message Loop. 
//
//********************************************************************************************
HRESULT CNetCostEventSink::ListenForEvents()
{
    HRESULT hr = S_OK;
    CComPtr<IConnectionPointContainer> pCpc;
    CComPtr<IUnknown> pSink;
    hr = CoCreateInstance(CLSID_NetworkListManager, NULL, 
        CLSCTX_ALL, __uuidof(INetworkCostManager), (LPVOID*)&m_pCostManager);

    if (SUCCEEDED(hr))
    {
        //If register for destination cost notifications, call SetDestinationAddresses to register the requested Destination IP addresses
        if ((m_riid == IID_INetworkCostManagerEvents) && (wcslen(m_destSockAddr.addrString) > 0))
        {
            hr = m_pCostManager->SetDestinationAddresses(1, &(m_destSockAddr.ipAddr), VARIANT_TRUE);
        }
    }
    if (SUCCEEDED(hr))
    {
        hr = m_pCostManager->QueryInterface(IID_IConnectionPointContainer, (void**)&pCpc);
    }
    if (SUCCEEDED(hr))
    {
        hr = pCpc->FindConnectionPoint(m_riid, &m_pConnectionPoint);
    }
    if (SUCCEEDED(hr))
    {
        hr = this->QueryInterface(IID_IUnknown, (void**)&pSink);
    }
    if (SUCCEEDED(hr))
    {
        hr = m_pConnectionPoint->Advise(pSink, &m_dwCookie);
    }
    if (SUCCEEDED(hr))
    {
        BOOL bRet;
        MSG msg;
        while((bRet = GetMessage(&msg, NULL, 0, 0 )) != 0)
        { 
            if (bRet == -1)
            {
                break;
            }
            TranslateMessage(&msg); 
            DispatchMessage(&msg); 
        }
    }
    return hr;
}

//********************************************************************************************
// Function: StopListeningForEvents
//
// Description: Stops the event listener thread by posting WM_QUIT to it, then returns the exit code.
//
//********************************************************************************************
HRESULT CNetCostEventSink::StopListeningForEvents()
{
    HRESULT hr = S_OK;
    if (m_pConnectionPoint != NULL)
    {
        hr = m_pConnectionPoint->Unadvise(m_dwCookie);
    }
    
    if (m_hThread != INVALID_HANDLE_VALUE)
    {
        PostThreadMessage(m_dwThreadId, WM_QUIT, 0, 0);
        WaitForSingleObject(m_hThread, INFINITE);

        DWORD dwExitCode = 0;
        if (!GetExitCodeThread(m_hThread, &dwExitCode))
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
        else
        {
            hr = HRESULT_FROM_WIN32(dwExitCode);
        }

        CloseHandle(m_hThread);
    }
    this->Release();
    return hr;
}

//********************************************************************************************
// Function:  CNetCostEventSink() & ~CNetCostEventSink()
//
// Description: Constructor and destructor for CNetCostEventSink class
//
//********************************************************************************************
CNetCostEventSink::CNetCostEventSink(_In_opt_ DESTINATION_INFO *pDestAddress, _In_ REFIID riid):m_lRef(1), m_riid(riid)
{
    ZeroMemory(m_destSockAddr.addrString, IP_ADDRESS_SIZE*sizeof(WCHAR));
    ZeroMemory(&m_defaultInterfaceGuid, sizeof(GUID));
    
    if (pDestAddress)
    {
        wmemcpy(m_destSockAddr.addrString, pDestAddress->addrString,wcslen(pDestAddress->addrString));
        m_destSockAddr.ipAddr = pDestAddress->ipAddr;
    }
}

CNetCostEventSink::~CNetCostEventSink()
{
    
}

//********************************************************************************************
// Function:  IUnknown members
//
// Description: Implementation of QueryInterface, AddRef and Release, methods of IUnknown interface
//
//********************************************************************************************
STDMETHODIMP CNetCostEventSink::QueryInterface (_In_ REFIID riid, _Out_ LPVOID* ppv)
{
    static const QITAB rgqit[] =
    {
        QITABENT(CNetCostEventSink, INetworkCostManagerEvents),
        QITABENT(CNetCostEventSink, INetworkConnectionCostEvents),
        { 0 }
    };    
    return QISearch(this, rgqit, riid, ppv);    
}

STDMETHODIMP_(ULONG) CNetCostEventSink::AddRef(VOID)
{
    return InterlockedIncrement( (LONG *)&m_lRef );
}

STDMETHODIMP_(ULONG) CNetCostEventSink::Release(VOID)
{
    ULONG ulNewRef = (ULONG)InterlockedDecrement( (LONG *)&m_lRef );
    if (ulNewRef == 0)
    {
        delete this;
    }
    return ulNewRef;
}

//********************************************************************************************
// Function: CostChanged
//
// Description: Callback function to display new machine cost
//
//********************************************************************************************


STDMETHODIMP CNetCostEventSink::CostChanged (_In_ DWORD cost, _In_opt_ NLM_SOCKADDR *pSockAddr)
{
    SYSTEMTIME LocalTime;
    
    GetLocalTime( &LocalTime );
    wprintf(L"\n***********************************\n");
    if (pSockAddr)
    {
        wprintf(L"Cost Change for Destination address : %s\n",g_pSinkDestCostMgr->m_destSockAddr.addrString);
    }
    
    else
    {
        wprintf(L"Machine Cost changed\n");
    }
    
    DisplayCostDescription(cost);    
    return S_OK;
}

//********************************************************************************************
// Function: DataPlanStatusChanged
//
// Description: Callback function to display new machine data plan status.
//
//********************************************************************************************

STDMETHODIMP CNetCostEventSink::DataPlanStatusChanged (_In_opt_ NLM_SOCKADDR *pSockAddr)
{
    HRESULT hr = S_OK;
    SYSTEMTIME LocalTime;
    NLM_DATAPLAN_STATUS dataPlanStatus;

    GetLocalTime(&LocalTime);
    wprintf(L"\n***********************************\n");

    if (pSockAddr)
    {
        wprintf(L"New Data Plan Status for Destination address : %s\n",m_destSockAddr.addrString);
    }
    else
    {
        wprintf(L"Machine Data Plan Status Changed\n");
    }
    
    hr = m_pCostManager->GetDataPlanStatus(&dataPlanStatus, pSockAddr);            
    if (hr == S_OK)
    {
        //If there is an interface change, applications should disconnect and reconnect to the new interface
        if (!IsEqualGUID (dataPlanStatus.InterfaceGuid, m_defaultInterfaceGuid))
        {
            wprintf(L"There is an interface change. Please disconnect and reconnect to the new interface \n");
            m_defaultInterfaceGuid = dataPlanStatus.InterfaceGuid;
        }
        DisplayDataPlanStatus(&dataPlanStatus);
    }
    DisplayError(hr);
    return hr;
}

//********************************************************************************************
// Function: ConnectionCostChanged
//
// Description: Callback function to display new connection cost
//
//********************************************************************************************


STDMETHODIMP CNetCostEventSink::ConnectionCostChanged (_In_ GUID connectionId, _In_ DWORD cost)
{
    SYSTEMTIME LocalTime;
    GetLocalTime( &LocalTime );
    wprintf(L"\n***********************************\n");
    wprintf(L"Connection Cost Changed\n");
    
    //get connection ID
    WCHAR szGuid[39]={0};
    StringFromGUID2( connectionId, szGuid, 39 );
    wprintf(L"Connection ID    :    %s\n", szGuid);
    DisplayCostDescription(cost);
    return S_OK;
}

//********************************************************************************************
// Function: ConnectionDataPlanStatusChanged
//
// Description: Callback function to display new connection data plan status.
//
//********************************************************************************************

STDMETHODIMP CNetCostEventSink::ConnectionDataPlanStatusChanged (_In_ GUID connectionId)
{
    HRESULT hr = S_OK;
    SYSTEMTIME LocalTime;
    CComPtr<INetworkListManager> pLocalNLM;
    CComPtr<INetworkConnection> pConnection;
    CComPtr<INetworkConnectionCost> pConnectionCost;
    NLM_DATAPLAN_STATUS dataPlanStatus;

    GetLocalTime( &LocalTime );
    wprintf(L"\n***********************************\n");
    wprintf(L"Connection data plan status changed\n");
    //get connection ID
    WCHAR szGuid[39]={0};
    StringFromGUID2( connectionId, szGuid, 39 );
    wprintf(L"Connection ID    :    %s\n", szGuid);        
    hr = CoCreateInstance(CLSID_NetworkListManager, NULL, CLSCTX_ALL, IID_PPV_ARGS(&pLocalNLM));
    if (SUCCEEDED(hr))
    {
        hr = GetConnectionFromGUID(pLocalNLM, connectionId, &pConnection);
    }
    
    if (SUCCEEDED(hr))
    {
        hr = pConnection->QueryInterface(IID_PPV_ARGS(&pConnectionCost));    
    }
    
    if (SUCCEEDED(hr))
    {
        hr = pConnectionCost->GetDataPlanStatus(&dataPlanStatus);       
    }
    
    if (SUCCEEDED(hr))
    {
        DisplayDataPlanStatus(&dataPlanStatus);
    }
    DisplayError(hr);
    return hr;
}
