// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//  Abstract:
//
//      Define and Implement the TThreadpool template class.
//      This class is implemented as a singleton to allow different objects
//      to share the same thread pool.  The underlying Threadpool is 
//      destroyed if there are no references to it.  
//
//      The class ThreadpoolId template parameter is used to allow creation
//      of multiple Threadpools.
//
//---------------------------------------------------------------------------

#pragma once

// Declare TThreadpool
template<ULONG ThreadpoolId>
class TThreadpool
{
public:
    static TThreadpool<ThreadpoolId>* GetThreadpool();
    VOID AddRef();
    VOID Release();
    PTP_POOL GetPTP_POOL();

protected:
    TThreadpool();
    ~TThreadpool();
    BOOL Init();

protected:
    static TLock sm_SingletonLock;
    static TThreadpool<ThreadpoolId>* sm_pThreadpool;
    LONG m_cRef;
    PTP_POOL m_pThreadpool;
};

// Create TThreadpool instances
typedef TThreadpool<0> TClientNotificationThreadpool;
typedef TThreadpool<1> TNetworkIOThreadpool;
typedef TThreadpool<2> TNetworkTimersThreadpool;

// Static memeber instantiations
template<ULONG ThreadpoolId>
TThreadpool<ThreadpoolId>* TThreadpool<ThreadpoolId>::sm_pThreadpool = NULL;

template<ULONG ThreadpoolId>
TLock TThreadpool<ThreadpoolId>::sm_SingletonLock;


//---------------------------------------------------------------------------
// Begin TThreadpool implementation
//---------------------------------------------------------------------------

template<ULONG ThreadpoolId>
TThreadpool<ThreadpoolId>* TThreadpool<ThreadpoolId>::GetThreadpool()
{
    TThreadpool<ThreadpoolId>* pThreadpool = NULL;

    sm_SingletonLock.AcquireExclusive();

    if (sm_pThreadpool)
    {
        pThreadpool = sm_pThreadpool;
        ++pThreadpool->m_cRef;
    }
    else
    {
        pThreadpool = new(std::nothrow) TThreadpool<ThreadpoolId>();
        if (pThreadpool)
        {
            if (pThreadpool->Init())
            {
                sm_pThreadpool = pThreadpool;
            }
            else
            {
                delete pThreadpool;
                pThreadpool = NULL;
            }
        }
    }

    sm_SingletonLock.ReleaseExclusive();

    return pThreadpool;
}

template<ULONG ThreadpoolId>
TThreadpool<ThreadpoolId>::TThreadpool(): 
    m_cRef(1)
{
}  // TThreadpool:TThreadpool

template<ULONG ThreadpoolId>
TThreadpool<ThreadpoolId>::~TThreadpool()
{
    if (m_pThreadpool)
    {
        CloseThreadpool(m_pThreadpool);
    }
}  // TThreadpool::~TThreadpool

template<ULONG ThreadpoolId>
VOID TThreadpool<ThreadpoolId>::AddRef()
{
    sm_SingletonLock.AcquireExclusive();
    
    ++m_cRef;

    sm_SingletonLock.ReleaseExclusive();

}  // TThreadpool::AddRef

template<ULONG ThreadpoolId>
VOID TThreadpool<ThreadpoolId>::Release()
{
    sm_SingletonLock.AcquireExclusive();

    --m_cRef;
    if (0 == m_cRef)
    {
        delete this;
        sm_pThreadpool = NULL;
    }

    sm_SingletonLock.ReleaseExclusive();
}  // TThreadpool::Release

template<ULONG ThreadpoolId>
BOOL TThreadpool<ThreadpoolId>::Init()
{
    SYSTEM_INFO SystemInfo = {0};
    BOOL fRetVal = TRUE;

    GetSystemInfo(&SystemInfo);

    m_pThreadpool = CreateThreadpool(NULL);
    fRetVal = (NULL != m_pThreadpool); 

    if (fRetVal)
    {
        fRetVal = SetThreadpoolThreadMinimum(
            m_pThreadpool, 
            0);
    }

    if (fRetVal)
    {
        SetThreadpoolThreadMaximum(
            m_pThreadpool, 
            SystemInfo.dwNumberOfProcessors * 2);
    }
    
    return fRetVal;
}  // TThreadpool::Init()

template<ULONG ThreadpoolId>
PTP_POOL TThreadpool<ThreadpoolId>::GetPTP_POOL()
{
    return m_pThreadpool;
} // TThreadpool::GetPTP_POOL

//---------------------------------------------------------------------------
// End TThreadpool implementation
//---------------------------------------------------------------------------
