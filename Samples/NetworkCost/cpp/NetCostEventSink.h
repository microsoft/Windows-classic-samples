// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once
#include "Utils.h"
class CNetCostEventSink : 
    public INetworkCostManagerEvents,
    public INetworkConnectionCostEvents
{
    private:
        DWORD                           m_dwCookie;
        CComPtr<IConnectionPoint>       m_pConnectionPoint;
        GUID                            m_riid;
        HANDLE                          m_hThread;
        DWORD                           m_dwThreadId;
        ULONG                           m_lRef;
        DESTINATION_INFO                m_destSockAddr;
        CComPtr<INetworkCostManager>    m_pCostManager;

        //Default InterfaceGuid, stored to detect change in interface, when there is a dataplan status change.
        GUID                            m_defaultInterfaceGuid;
        
        //
        // The main listener function. Listens, and waits on a Message Loop. 
        //
        HRESULT ListenForEvents();
        
        //
        // This is our thread entry proc for the thread that will listen on events.
        //
        static DWORD WINAPI StartListeningForEventsThread(_In_ LPVOID pArg);

    public:

        CNetCostEventSink(_In_opt_ DESTINATION_INFO *pDestAddress, _In_ REFIID riid);
        ~CNetCostEventSink();

        //
        // Creates the CNetCostEventSink object, and start a thread to perform an advise on it
        //
        static HRESULT StartListeningForEvents(_In_ REFIID riid, _In_opt_ DESTINATION_INFO *pDestAddress, _Outptr_ CNetCostEventSink** ppSinkCostMgr);
        
        //
        // Stops the event listener thread by posting WM_QUIT to it, then returns the exit code.
        //
        HRESULT StopListeningForEvents();

        // INetworkCostManagerEvents members
        STDMETHOD(CostChanged) (_In_ DWORD cost, _In_opt_ NLM_SOCKADDR *pSockAddr);
        STDMETHOD(DataPlanStatusChanged) (_In_opt_ NLM_SOCKADDR *pSockAddr);

         // INetworkConnectionCostEvents members
        STDMETHOD(ConnectionCostChanged) (_In_ GUID connectionId, _In_ DWORD cost);
        STDMETHOD(ConnectionDataPlanStatusChanged) (_In_ GUID connectionId);
        

        // IUnknown members
        STDMETHOD(QueryInterface) (_In_ REFIID riid, _Out_ LPVOID* ppv);
        STDMETHOD_(ULONG, AddRef) (VOID);
        STDMETHOD_(ULONG, Release)(VOID);
};

//Global Instance declaration  for CNetCostEventSink
extern CNetCostEventSink *g_pSinkCostMgr;
extern CNetCostEventSink *g_pSinkConnectionCostMgr;
extern CNetCostEventSink *g_pSinkDestCostMgr;
