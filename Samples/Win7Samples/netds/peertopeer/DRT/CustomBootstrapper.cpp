// CustomBootstrapper.cpp - Implementation of CustomBootstrapper

// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved


#include "CustomBootstrapper.h"

#pragma comment(lib,"ws2_32")
#pragma comment(lib,"shlwapi")

void CBootStrapResolveContext::Release()
{
    LONG    lNewCount=InterlockedDecrement(&m_RefCount);
    if (lNewCount == 0)
        delete this;
    return;
}

void CBootStrapResolveContext::AddRef()
{
    InterlockedIncrement(&m_RefCount);
}

HRESULT DrtCreateCustomBootstrapResolver(USHORT port, PCWSTR pwszAddress, DRT_BOOTSTRAP_PROVIDER** ppModule)
{
    HRESULT hr = S_OK;
    int     SocketResult=-1;
    CustomDnsBootStrapper* pBootStrapper=NULL;
    WSADATA WsaData;
    USHORT  Version=MAKEWORD(2,2);

    SocketResult=WSAStartup(Version,&WsaData);

    if (SocketResult != 0)
    {
        hr=HRESULT_FROM_WIN32(SocketResult);
    }

    if (SUCCEEDED(hr))
    {
        pBootStrapper = new CustomDnsBootStrapper;

        if (pBootStrapper == NULL)
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
            hr = pBootStrapper->Init(port,pwszAddress,ppModule);
        }
    }

    if(FAILED(hr))
    {
        if (pBootStrapper != NULL)
        {
            delete pBootStrapper;
            pBootStrapper=NULL;
        }

        if (SocketResult == 0)
        {
            WSACleanup();
        }

    }

    return hr;
}


void DrtDeleteCustomBootstrapResolver(DRT_BOOTSTRAP_PROVIDER* pResolver)
{
    CustomDnsBootStrapper *pBootStrapper = (CustomDnsBootStrapper*)pResolver->pvContext;
    pBootStrapper->Release();

    WSACleanup();
}

CustomDnsBootStrapper::~CustomDnsBootStrapper()
{
    if (m_Address != NULL)
    {
        LocalFree(m_Address);
        m_Address=NULL;
    }
}

CustomDnsBootStrapper::CustomDnsBootStrapper()
{
    m_lAttachCount = 0;
    m_lRefCount = 0;
    m_Port=0;
    m_Address=NULL;
}


void CustomDnsBootStrapper::Release()
{
    LONG refcount = InterlockedDecrement(&m_lRefCount);
    if(refcount == 0)
        delete this;
}

void CustomDnsBootStrapper::AddRef()
{
    InterlockedIncrement(&m_lRefCount);
}

HRESULT CustomDnsBootStrapper::Init(
    USHORT port,
    PCWSTR pwszAddress,
    DRT_BOOTSTRAP_PROVIDER** ppModule)
{
    HRESULT hr = S_OK;
    *ppModule=NULL;

    if (pwszAddress != NULL)
    {
        m_Port=port;
        if(swprintf_s(m_szPortString,_countof(m_szPortString),L"%u",m_Port) == -1)
            hr = E_INVALIDARG;

        if (SUCCEEDED(hr))
        {
            m_Address=StrDup(pwszAddress);

            if (m_Address == NULL)
            {
                hr=E_OUTOFMEMORY;
            }
            else
            {
                m_BootStrapModule.pvContext  = this;
                m_BootStrapModule.Attach     = &CustomDnsBootStrapper::Attach;
                m_BootStrapModule.Detach     = &CustomDnsBootStrapper::Detach;
                m_BootStrapModule.InitResolve = &CustomDnsBootStrapper::InitResolve;
                m_BootStrapModule.IssueResolve = &CustomDnsBootStrapper::IssueResolve;
                m_BootStrapModule.EndResolve = &CustomDnsBootStrapper::EndResolve;
                m_BootStrapModule.Register   = &CustomDnsBootStrapper::Register;
                m_BootStrapModule.Unregister = &CustomDnsBootStrapper::Unregister;

                m_BootStrapModule.pvContext=this;
                *ppModule = &m_BootStrapModule;
                AddRef();
            }
        }
    }
    else
    {
        hr=E_INVALIDARG;
    }

    return hr;
}


HRESULT CustomDnsBootStrapper::Attach(
           const PVOID pvContext
    )
{
    CustomDnsBootStrapper* pBootStrapper = (CustomDnsBootStrapper*) pvContext;
    LONG    lInitialValue;

    lInitialValue = InterlockedCompareExchange(&pBootStrapper->m_lAttachCount, 1, 0);
    if (lInitialValue != 0)
    {
        return DRT_E_BOOTSTRAPPROVIDER_IN_USE;
    }
    pBootStrapper->AddRef();

    return S_OK;
}

VOID CustomDnsBootStrapper::Detach(
           const PVOID pvContext
    )

{
    CustomDnsBootStrapper* pBootStrapper = (CustomDnsBootStrapper*) pvContext;
    LONG    lInitialValue;

    lInitialValue = InterlockedCompareExchange(&pBootStrapper->m_lAttachCount, 0, 1);
    pBootStrapper->Release();

    return;
}


HRESULT CustomDnsBootStrapper::InitResolve(
    __in_opt const PVOID pvContext,
    BOOL fSplitDetect,
    DWORD dwTimeout,
    DWORD dwMaxResults,
    DRT_BOOTSTRAP_RESOLVE_CONTEXT* pResolveContext,
    BOOL* fFatalError
    )
{
    HRESULT hr = S_OK;
    *fFatalError = FALSE;
    CustomDnsBootStrapper* pBootStrapper = (CustomDnsBootStrapper*) pvContext;
    CBootStrapResolveContext* pBSResolveContext = NULL;
    UNREFERENCED_PARAMETER(dwMaxResults);

    if (pResolveContext == NULL)
    {
        hr = E_INVALIDARG;
        goto exit;
    }
        
    *pResolveContext = NULL;

    hr = DRT_E_BOOTSTRAPPROVIDER_NOT_ATTACHED;
    if (pBootStrapper->m_lAttachCount != 0)
    {
        pBSResolveContext = new CBootStrapResolveContext;
        *pResolveContext = pBSResolveContext;

        if (pBSResolveContext == NULL)
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
            // The cache is not scope aware so we ask for
            // a larger number of addresses than the cache wants.
            // In the expectation that one of them may be good for us
            hr = pBSResolveContext->Init(fSplitDetect, dwTimeout, DNS_ADDRESS_QUERY);

            if(FAILED(hr))
            {
            
                *pResolveContext = NULL;
                delete pBSResolveContext;
            }
            else
            {
                pBootStrapper->AddRef ();
            }
        }
    }

exit:
    
    if FAILED(hr)
    {
        //
        // CustomDNSResolver has no retry cases, so any failed HRESULT is fatal
        //
        *fFatalError = TRUE;
    }

    return (hr);
}    

const DWORD dwIsatapPrivateV4Marker = 0x00005efe;
const DWORD dwIsatapPublicV4Marker = 0x02005efe;

HRESULT CustomDnsBootStrapper::IssueResolve(
    const PVOID pvContext,
    __in_opt const PVOID pvCallbackContext,
    DRT_BOOTSTRAP_RESOLVE_CALLBACK callback,
    DRT_BOOTSTRAP_RESOLVE_CONTEXT ResolveContext,
    BOOL* fFatalError
    )
{
    HRESULT hr = S_OK;

    *fFatalError = FALSE;
    CustomDnsBootStrapper* pBootStrapper = (CustomDnsBootStrapper*) pvContext;
    CBootStrapResolveContext* pResolveContext = NULL;

    if (callback == NULL)
    {
        return E_INVALIDARG;
    }
    
    hr = DRT_E_BOOTSTRAPPROVIDER_NOT_ATTACHED;
    if (pBootStrapper->m_lAttachCount != 0)
    {
        pResolveContext = (CBootStrapResolveContext*)ResolveContext;
        pResolveContext->AddRef();
        hr = pResolveContext->IssueResolve(pvCallbackContext, callback, pBootStrapper->m_szPortString, pBootStrapper->m_Address);
        pResolveContext->Release();
    }
    
    if FAILED(hr)
    {
        //
        // DNSResolver has no retry cases, so any failed HRESULT is fatal
        //
        *fFatalError = TRUE;
    }
    return hr;
}

VOID CustomDnsBootStrapper::EndResolve(
    const PVOID pvContext,
          DRT_BOOTSTRAP_RESOLVE_CONTEXT ResolveContext
    )
{
    CBootStrapResolveContext* pResolveContext=(CBootStrapResolveContext*)ResolveContext;
    CustomDnsBootStrapper *ThisPtr = (CustomDnsBootStrapper *)pvContext;
    pResolveContext->EndResolve();
    pResolveContext->Release();
    ThisPtr->Release();

    return ;
}


HRESULT CustomDnsBootStrapper::Register(
    const PVOID pvContext,
    const SOCKET_ADDRESS_LIST* pAddressList
    )
{
    //Custom DNS resolver Register does nothing at this time
    UNREFERENCED_PARAMETER(pvContext);
    UNREFERENCED_PARAMETER(pAddressList);
    return S_OK;
}

VOID CustomDnsBootStrapper::Unregister(
    const PVOID pvContext
    )
{
    //Custom DNS resolver Unregister does nothing at this time
    UNREFERENCED_PARAMETER(pvContext);
}

CBootStrapResolveContext::CBootStrapResolveContext()
{
    m_LockCreated = FALSE;
    m_hCallbackComplete = NULL;
    m_fEndResolve = FALSE;
    m_fResolveInProgress = FALSE;
    m_CallbackThreadId = NULL;
    m_RefCount = 1;
}

CBootStrapResolveContext::~CBootStrapResolveContext()
{
    if (m_hCallbackComplete)
    {
        CloseHandle(m_hCallbackComplete);
        m_hCallbackComplete = NULL;
    }
    if (m_LockCreated)
    {
        DeleteCriticalSection(&m_Lock);
        m_LockCreated=FALSE;
    }
}

HRESULT CBootStrapResolveContext::Init(
         BOOL fSplitDetect,
         DWORD dwTimeout,
         DWORD dwMaxResults)
{
    HRESULT hr = S_OK;

    m_LockCreated = InitializeCriticalSectionAndSpinCount(&m_Lock, 0x80001000);

    if (!m_LockCreated)
    {
        hr = E_OUTOFMEMORY;
    }
    
    m_fSplitDetect = fSplitDetect;
    m_dwTimeout = dwTimeout;
    m_dwMaxResults = dwMaxResults;
 
    return hr;
}

HRESULT CBootStrapResolveContext::IssueResolve(
            __in_opt const PVOID pvCallbackContext,
            DRT_BOOTSTRAP_RESOLVE_CALLBACK callback,
            __in_ecount(32)  PCWSTR szPortString,
             PWSTR address)
{
    HRESULT hr = S_OK;
    SOCKET_ADDRESS_LIST* Addresses = NULL;
    SOCKADDR_IN6* psockAddrs = NULL;
    EnterCriticalSection(&m_Lock);
    m_fResolveInProgress = TRUE;
    m_CallbackThreadId = GetCurrentThreadId();
    LeaveCriticalSection(&m_Lock);

    if(m_dwMaxResults > 0)
    {
        PWCHAR      CurrentAddress = address;
        PWCHAR      EndAddress = NULL;
        WCHAR       SavedCharacter;
        
        for(;;)
        {
            if(m_fEndResolve)
            {
                goto exit;
            }
            //  Trim white space and separators
            while ((*CurrentAddress == L' ') || (*CurrentAddress == L';'))
            {
                CurrentAddress++;
            }
            if (*CurrentAddress == 0)
            {
                break;
            }
            EndAddress = CurrentAddress;
            while ((*EndAddress != 0) && (*EndAddress != L' ') && (*EndAddress != L';'))
            {
                EndAddress++;
            }
            SavedCharacter=*EndAddress;
            *EndAddress=L'\0';

            // Retrieve bootstrap possibilities
            ADDRINFOW addrInf;
            ZeroMemory( &addrInf, sizeof(addrinfo) );
            addrInf.ai_flags =  AI_CANONNAME;
            addrInf.ai_family = AF_UNSPEC;
            addrInf.ai_socktype = SOCK_STREAM;
            addrInf.ai_protocol = 0;

            ADDRINFOW* results = NULL;
            ADDRINFOW* resultsCopy = NULL;
            
            int nStat = GetAddrInfoW( CurrentAddress, szPortString, &addrInf, &results );
            if ( nStat == 0 )
            {
                size_t idx = 0;
                size_t nBytes = SIZEOF_SOCKET_ADDRESS_LIST(m_dwMaxResults);

                Addresses = (SOCKET_ADDRESS_LIST*) malloc(nBytes);
                psockAddrs = (SOCKADDR_IN6*) malloc(sizeof(SOCKADDR_IN6)*m_dwMaxResults);
                if(Addresses == NULL || psockAddrs == NULL)
                {
                    *EndAddress=SavedCharacter;
                    hr = E_OUTOFMEMORY;
                
                    goto exit;
                }

                ZeroMemory( Addresses, nBytes );
                ZeroMemory( psockAddrs, m_dwMaxResults * sizeof(SOCKADDR_IN6) );

                resultsCopy = results;
                while((NULL != results) && (idx < m_dwMaxResults)) 
                {
                    Addresses->iAddressCount++;
                    if(results->ai_family == AF_INET)
                    {
                        // We found an IPV4 address so lets turn it into a link-local isatap address since the DRT doesn't understand
                        // IPv4 natively.
                        Addresses->Address[idx].iSockaddrLength = sizeof(SOCKADDR_IN6);
                        Addresses->Address[idx].lpSockaddr = (LPSOCKADDR)&psockAddrs[idx];

                        // Construct a Link-local isatap address for this IPv4 address
                        DWORD* pTunnel = (DWORD*)&(psockAddrs[idx].sin6_addr);
                        BOOL fIsPrivate = IN4_IS_ADDR_RFC1918((IN_ADDR*)results->ai_addr);
                        sockaddr_in* pAddr  = (sockaddr_in*)results->ai_addr;

                        psockAddrs[idx].sin6_family = AF_INET6;
                        psockAddrs[idx].sin6_port = pAddr->sin_port;
                        
                        pTunnel[0] = htonl(0xfe800000);
                        pTunnel[1] = 0x00000000;
                        pTunnel[2] = htonl(fIsPrivate ? dwIsatapPrivateV4Marker : dwIsatapPublicV4Marker);
                        pTunnel[3] = pAddr->sin_addr.S_un.S_addr;
                        idx++;
                    }
                    else if(results->ai_family == AF_INET6)
                    {
                        // We only deal with IPV6 addresses internally
                        Addresses->Address[idx].iSockaddrLength = (INT)results->ai_addrlen;
                        Addresses->Address[idx].lpSockaddr = (LPSOCKADDR)&psockAddrs[idx];
                        CopyMemory(&psockAddrs[idx], results->ai_addr, results->ai_addrlen);
                        idx++;
                    }
                    results = results->ai_next; //next item in l-list
                }

                //
                //  Call the callback to signal completion
                //
                (*callback)(hr, pvCallbackContext, Addresses, FALSE);
                FreeAddrInfoW(resultsCopy);

                delete[] Addresses;
                Addresses = NULL;
                delete[] psockAddrs;
                psockAddrs = NULL;
            }
            else
            {
                //
                // GetAddrInfoW Failed  
                // but there may be more addresses in the string so keep going
                // otherwise we return E_NO_MORE and retry next cycle
                //
            }

            *EndAddress=SavedCharacter;
            CurrentAddress=EndAddress;
        }
    }

    //
    //  Tell the drt there will be no more results
    //
    (*callback) (DRT_E_NO_MORE, pvCallbackContext,NULL, FALSE);

    if(Addresses!= NULL)
    {
        delete[] Addresses;
        Addresses = NULL;
    }
    if(psockAddrs != NULL)
    {
        delete[] psockAddrs;
        psockAddrs = NULL;
    }

exit:
    EnterCriticalSection(&m_Lock);
    if(m_hCallbackComplete)
    {
        //
        // Notify EndResolve that callbacks have completed
        //
        SetEvent(m_hCallbackComplete);
    }
    m_fResolveInProgress = FALSE;
    LeaveCriticalSection(&m_Lock);

    return hr;
}


VOID
CBootStrapResolveContext::EndResolve()
{
    BOOL fWaitForCallback = FALSE;
    HANDLE CallbackComplete = NULL;

    CallbackComplete = CreateEvent(NULL, TRUE, FALSE, NULL);
    
    EnterCriticalSection(&m_Lock);
    if(m_fResolveInProgress && ( GetCurrentThreadId() != m_CallbackThreadId) )
    {
        if(m_fEndResolve == FALSE)
        {
            //
            // This is the first thread to call EndResolve and we need to wait for a callback
            // to complete so initialize the class member event
            //
            m_fEndResolve = TRUE;
            m_hCallbackComplete = CallbackComplete;
        }
        fWaitForCallback = TRUE;
    }
    LeaveCriticalSection(&m_Lock);

    if(CallbackComplete && (m_hCallbackComplete != CallbackComplete) )
    {
        //
        // This thread was not the first to call EndResolve, so its event is not in use, release it
        // (m_hCallbackComplete is released in the destructor)
        //
        CloseHandle(CallbackComplete);
        CallbackComplete = NULL;
    }
    
    if(fWaitForCallback && m_hCallbackComplete)
    {
        WaitForSingleObject(m_hCallbackComplete, INFINITE);
    }
}
