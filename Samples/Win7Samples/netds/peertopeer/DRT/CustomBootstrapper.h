// CustomBootstrapper.h - Interface for CustomBootstrapper

// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include <tchar.h>
#include <stdio.h>
#include <drt.h>
#include <shlwapi.h>
#include <mstcpip.h>

#define DNS_ADDRESS_QUERY 20


HRESULT DrtCreateCustomBootstrapResolver(USHORT port, PCWSTR pwszAddress, DRT_BOOTSTRAP_PROVIDER** ppModule);
void DrtDeleteCustomBootstrapResolver(DRT_BOOTSTRAP_PROVIDER* pResolver);

class CBootStrapResolveContext
{
private:
    LONG                m_RefCount;
    BOOL                m_fSplitDetect;
    DWORD               m_dwTimeout;
    DWORD               m_dwMaxResults;
    BOOL                m_LockCreated;
    CRITICAL_SECTION    m_Lock;
    HANDLE              m_hCallbackComplete;
    BOOL                m_fEndResolve;
    BOOL                m_fResolveInProgress;
    ULONG               m_CallbackThreadId;

public:
    void AddRef();
    void Release();
    CBootStrapResolveContext();
    ~CBootStrapResolveContext();

    HRESULT Init(
             BOOL   fSplitDetect,
             DWORD  dwTimeout,
             DWORD  dwMaxResults
             );

    HRESULT IssueResolve(
            __in_opt const PVOID pvCallbackContext,
            __in DRT_BOOTSTRAP_RESOLVE_CALLBACK callback,
            __in_ecount(32)  PCWSTR szPortString,
            __in  PWSTR address
            );

    void EndResolve();
};

class CustomDnsBootStrapper
{
private:
    volatile LONG m_lAttachCount;
    volatile LONG m_lRefCount;
    DRT_BOOTSTRAP_PROVIDER m_BootStrapModule;

    USHORT              m_Port;
    WCHAR               m_szPortString[32];
    PWSTR               m_Address;

public:
    CustomDnsBootStrapper();
    ~CustomDnsBootStrapper();

    void Release();
    void AddRef();

    HRESULT Init(
        __in USHORT port,
        __in PCWSTR pwszAddress,
        __out DRT_BOOTSTRAP_PROVIDER** ppModule
        );

    static HRESULT Attach(
        __in const PVOID pvContext
        );

    static VOID Detach(
        __in const PVOID pvContext
        );

    static HRESULT InitResolve(
        __in_opt const PVOID pvContext,
        __in BOOL       fSplitDetect,
        __in DWORD      dwTimeout,
        __in DWORD      dwMaxResults,
        __out DRT_BOOTSTRAP_RESOLVE_CONTEXT* ResolveContext,
        __out BOOL* fFatalError
        );
        
    static HRESULT IssueResolve(
        __in const      PVOID pvContext,
        __in_opt const PVOID pvCallbackContext,
        __in DRT_BOOTSTRAP_RESOLVE_CALLBACK callback,
        __in DRT_BOOTSTRAP_RESOLVE_CONTEXT ResolveContext,
        __out BOOL* fFatalError
        );        

    static VOID EndResolve(
        __in const PVOID pContext,
        __in DRT_BOOTSTRAP_RESOLVE_CONTEXT ResolveContext
        );

    static HRESULT Register(
        __in const PVOID pvContext,
        __in const SOCKET_ADDRESS_LIST* pAddressList
        );

    static VOID Unregister(
        __in const PVOID pvContext
        );
};