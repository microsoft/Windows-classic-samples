// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "stdafx.h"

// TODO - TODO - TODO -TODO - TODO - TODO - TODO - TODO - TODO - TODO
// 
// CLSID_FDProvider must match the value defined in FDProvider.cpp
//
// TODO - TODO - TODO -TODO - TODO - TODO - TODO - TODO - TODO - TODO
const GUID CLSID_FDProvider = { 0x8c19066a, 0x643a, 0x4586, { 0x92, 0xb2, 0xa7, 0x85, 0xb9, 0xd, 0x76, 0x6f }};

class TClassFactory: 
    public IClassFactory
{
public:
    // IUnknown
    STDMETHODIMP_(ULONG) AddRef();
    
    STDMETHODIMP_(ULONG) Release();
    
    STDMETHODIMP QueryInterface(
        REFIID riid, 
        __deref_out void** ppv);

    // IClassFactory
    STDMETHODIMP CreateInstance(
        __in_opt IUnknown* punkOuter, 
        REFIID iid, 
        __deref_out void** ppv);

    STDMETHODIMP LockServer(
        BOOL fLock);

    // Constructor / Destuctor
    TClassFactory();
    HRESULT Init();

    // Public functionality
    HRESULT ProcessLogoffNotification(
        DWORD SessionId);


protected:
    ~TClassFactory();

    LONG m_cRef;
    IClassFactory* m_pFDProviderClassFactory;
    IEXEHostControl* m_pIEXEHostControl;
}; // TClassFactory

// Forward defines
VOID __stdcall ProviderLifetimeNotificationCallback(
    bool fDestructed);

// Global variables
HANDLE g_hQuitEvent = NULL;
LONG g_cHostProcessRefCount = 0;

//---------------------------------------------------------------------------
// Begin TClassFactory implemetation
//---------------------------------------------------------------------------

TClassFactory::TClassFactory():
    m_cRef(1),
    m_pFDProviderClassFactory(NULL),
    m_pIEXEHostControl(NULL)
{ 
}  // TClassFactory::TClassFactory

TClassFactory::~TClassFactory() 
{ 
    if (m_pFDProviderClassFactory)
    {
        m_pFDProviderClassFactory->Release();
        m_pFDProviderClassFactory = NULL;
    }
    if (m_pIEXEHostControl)
    {
        m_pIEXEHostControl->Release();
        m_pIEXEHostControl = NULL;
    }
}  // TClassFactory::~TClassFactory

HRESULT TClassFactory::Init()
{
    HRESULT hr = S_OK;

    // Load the profider that will be hosted
    // and get a pointer to it's class factory
    hr = CoGetClassObject(
        CLSID_FDProvider,
        CLSCTX_INPROC_SERVER,
        NULL,
        IID_IClassFactory,
        (void**) &m_pFDProviderClassFactory);

    // Get a pointer to IEXEHostControl exposed by 
    // the provider's class factory
    if (S_OK == hr)
    {
        hr = m_pFDProviderClassFactory->QueryInterface(
            __uuidof(IEXEHostControl),
            (void**) &m_pIEXEHostControl);
    }

    // Register our liftime notification callback with the provider
    if (S_OK == hr)
    {
        hr = m_pIEXEHostControl->RegisterProviderLifetimeNotificationCallback(
            &ProviderLifetimeNotificationCallback);
    }

    return hr;
}  // TClassFactory::Init

STDMETHODIMP_(ULONG) TClassFactory::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}  // TClassFactory::AddRef

STDMETHODIMP_(ULONG) TClassFactory::Release()
{
    LONG cRef = InterlockedDecrement(&m_cRef);
    if (0 == cRef)
    {
        delete this;
    }

    return cRef;
}  // TClassFactory::Release

STDMETHODIMP TClassFactory::QueryInterface(
    REFIID riid, 
    __deref_out_opt void** ppv)
{
    HRESULT hr = S_OK;

    if (ppv)
    {
        *ppv = NULL;
    }
    else
    {
        hr = E_INVALIDARG;
    }

    if (S_OK == hr)
    {
        if (IID_IUnknown == riid)
        {
            AddRef();
            *ppv = (IUnknown*) this;
        }
        else if (IID_IClassFactory == riid)
        {
            AddRef();
            *ppv = (IClassFactory*) this;
        }
        else
        {
            hr = E_NOINTERFACE;
        }
    }

    return hr;
}  // TClassFactory::QueryInterface

STDMETHODIMP TClassFactory::CreateInstance(
    __in_opt IUnknown* pUnkownOuter, 
    REFIID riid, 
    __deref_out_opt void** ppv)
{
    return m_pFDProviderClassFactory->CreateInstance(
        pUnkownOuter,
        riid,
        ppv);
}  // TClassFactory::CreateInstance

STDMETHODIMP TClassFactory::LockServer(
    BOOL fLock)
{
    // External calls to LockServer are passed through to
    // the ProviderLifetime manager callback
    if (fLock)
    {
        ProviderLifetimeNotificationCallback(false);
    }
    else
    {
        ProviderLifetimeNotificationCallback(true);
    }

    return S_OK;
}  // TClassFactory::LockServer

HRESULT TClassFactory::ProcessLogoffNotification(
    DWORD SessionId)
{
    return m_pIEXEHostControl->LogoffNotification(SessionId);
}  // TClassFactory::ProcessLogoffNotification

//---------------------------------------------------------------------------
// End TClassFactory implemetation
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Begin global function implementation
//---------------------------------------------------------------------------

VOID __stdcall ProviderLifetimeNotificationCallback(
    bool fDestructed)
{
    if (!fDestructed)
    {
        InterlockedIncrement(&g_cHostProcessRefCount);
        CoAddRefServerProcess();
    }
    else
    {
        InterlockedDecrement(&g_cHostProcessRefCount);
        if (CoReleaseServerProcess() == 0)
        {
            // If there are no provider objects 
            // and no Lock on the class object, let the exe exit
            SetEvent(g_hQuitEvent);
        }
    }
}  // ProviderLifetimeNotificationCallback

int __stdcall WinMain(
    __in HINSTANCE hInstance,
    __in_opt HINSTANCE hPrevInstance,
    __in_opt LPSTR lpCmdLine,
    __in int nShowCmd)
{
    HRESULT hr = S_OK;
    TClassFactory* pClassFactory = NULL;
    ULONG hClassRegistration = NULL;
    HWND hMessageWindow = NULL;
    bool fWTSRegistered = false;
    bool fQuit = false;
    DWORD WaitResult = 0;
    MSG msg = {0};

    hr = CoInitializeEx(
        NULL,
        COINIT_MULTITHREADED);

    // Create and event that will be signaled 
    // when all COM objects have been released
    // and the process should end.
    if (S_OK == hr)
    {
        // Create a event that will be signaled when the Host should exit
        if (S_OK == hr)
        {
            g_hQuitEvent = CreateEvent(
                NULL, 
                TRUE, 
                FALSE, 
                NULL);
            if (!g_hQuitEvent)
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
            }
        }

        // Create our class factory
        if (S_OK == hr)
        {
            pClassFactory = new(std::nothrow) TClassFactory();
            if (!pClassFactory)
            {
                hr = E_OUTOFMEMORY;
            }
        }
        if (S_OK == hr)
        {
            hr = pClassFactory->Init();
        }

        // TODO - TODO - TODO -TODO - TODO - TODO - TODO - TODO - TODO - TODO
        //
        // If this EXE host is converted into a Windows Service, 
        // the message pump and WTSRegisterSessionNotification should be
        // removed and HandlerEx with SERVICE_CONTROL_SESSIONCHANGE
        // should be used instead.
        // 
        // TODO - TODO - TODO -TODO - TODO - TODO - TODO - TODO - TODO - TODO

        // Create a message only window to process
        // session change events that will notify us when a logoff has occured.
        if (S_OK == hr)
        {
            hMessageWindow = CreateWindow(
                L"Message",
                L"",
                0,
                CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                HWND_MESSAGE,  // Message only window
                NULL,
                hInstance,
                NULL);
            if (!hMessageWindow)
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
            }
        }

        // Register the window for session change events
        if (S_OK == hr)
        {
            if (WTSRegisterSessionNotification(
                hMessageWindow,
                NOTIFY_FOR_ALL_SESSIONS))
            {
                fWTSRegistered = true;
            }
            else
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
            }
        }
    
        // Register our class factory with COM remoting
        if (S_OK == hr)
        {
            hr = CoRegisterClassObject(
                CLSID_FDProvider,
                (IClassFactory*) pClassFactory,
                CLSCTX_LOCAL_SERVER,
                REGCLS_MULTIPLEUSE,
                &hClassRegistration);
        }

        // Start a message pump and wait for g_hQuitEvent to be signaled
        if (S_OK == hr)
        {
            while (!fQuit)
            {
                WaitResult = MsgWaitForMultipleObjectsEx(
                    1,
                    &g_hQuitEvent,
                    INFINITE,
                    QS_ALLINPUT,
                    MWMO_INPUTAVAILABLE);

                if (WAIT_OBJECT_0 == WaitResult)
                {
                    // g_hQuitEvent has been signaled 
                    fQuit = true;
                }
                else if (WAIT_OBJECT_0 + 1 == WaitResult)
                {
                    // Window messages are avaialble, process them
                    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
                    {
                        if (WM_QUIT == msg.message)
                        {
                            // WM_QUIT message has been posted.
                            fQuit = true;
                        }
                        if (WM_WTSSESSION_CHANGE == msg.message)
                        {
                            if (WTS_SESSION_LOGOFF == msg.wParam)
                            {
                                // A logoff has occured.  

                                hr = pClassFactory->ProcessLogoffNotification((DWORD) msg.lParam);
                                if (S_OK != hr)
                                {
                                    // if the logoff can not be processed, 
                                    // exit the process.
                                    fQuit = true;
                                }
                            }
                        }
                        else
                        {
                            TranslateMessage(&msg);
                            DispatchMessage(&msg);
                        }
                    }
                }
                else
                {
                    // This is unexpected, exit the process.
                    fQuit = true;
                }
            }
        }

// Cleanup
        if (hClassRegistration)
        {
            CoRevokeClassObject(hClassRegistration);
        }
        if (pClassFactory)
        {
            pClassFactory->Release();
        }
        if (fWTSRegistered)
        {
            WTSUnRegisterSessionNotification(hMessageWindow);
        }
        if (hMessageWindow)
        {
            DestroyWindow(hMessageWindow);
        }
        if (g_hQuitEvent)
        {
            CloseHandle(g_hQuitEvent);
        }

        CoUninitialize();
    }

	return hr;
}  // WinMain

//---------------------------------------------------------------------------
// End global function implementation
//---------------------------------------------------------------------------
