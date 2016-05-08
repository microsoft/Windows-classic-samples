// ----------------------------------------------------------------------------
// 
// This file is part of the Microsoft COM+ Samples.
// 
// Copyright (C) 1995-2000 Microsoft Corporation. All rights reserved.
// 
// This source code is intended only as a supplement to Microsoft
// Development Tools and/or on-line documentation. See these other
// materials for detailed information regarding Microsoft code samples.
// 
// THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
// KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
// 
// ----------------------------------------------------------------------------

// ===========================================================================
// Description:
// 
//  This is the server-portion of the SIMPLE Network OLE sample. This
// application implements the CLSID_SimpleObject class as a LocalServer.
// Instances of this class support a limited form of the IStream interface --
// calls to IStream::Read and IStream::Write will "succeed" (they do nothing),
// and calls on any other methods fail with E_NOTIMPL.
// 
//  The purpose of this sample is to demonstrate what is minimally required
// to implement an object that can be used by clients (both those on the same
// machine using OLE and those using Network OLE across the network).
// 
// Instructions:
// 
//  To use this sample:
//   * build it using the NMAKE command. NMAKE will create SSERVER.EXE and
//     SCLIENT.EXE.
//   * edit the SSERVER.REG file to make the LocalServer32 key point to the
//     location of SSERVER.EXE, and run the INSTALL.BAT command (it simply
//     performs REGEDIT SSERVER.REG)
//   * run SSERVER.EXE. it should display the message "Waiting..."
//   * run SCLIENT.EXE on the same machine using no command-line arguments,
//     or from another machine using the machine-name (UNC or DNS) as the sole
//     command-line argument. it will connect to the server, perform some read
//     and write calls, and disconnect. both SSERVER.EXE and SCLIENT.EXE will
//     automatically terminate. both applications will display some status text.
//   * you can also run SCLIENT.EXE from a different machine without having first
//     run SSERVER.EXE on the machine. in this case, SSERVER.EXE will be launched
//     by OLE in the background and you will be able to watch the output of
//     SCLIENT.EXE but the output of SSERVER.EXE will be hidden.
//   * to examine the automatic launch-security features of Network OLE, try
//     using the '...\CLSID\{...}\LaunchPermission = Y' key commented out in 
//     the SSERVER.REG file and reinstalling it. by setting different read-access
//     privileges on this key (using the Security/Permissions... dialog in the
//     REGEDT32 registry tool built into the system) you can allow other
//     users to run the SCLIENT.EXE program from their accounts.
// 
// Copyright 1996 Microsoft Corporation.  All Rights Reserved.
// ===========================================================================

// %%Includes: ---------------------------------------------------------------
#define INC_OLE2
#define STRICT
#include <stdio.h>
#include <windows.h>
#include <initguid.h>

// %%GUIDs: ------------------------------------------------------------------
DEFINE_GUID(CLSID_SimpleObject, 0x5e9ddec7, 0x5767, 0x11cf, 0xbe, 0xab, 0x0, 0xaa, 0x0, 0x6c, 0x36, 0x6);

// %%Globals: ----------------------------------------------------------------
HANDLE          hevtDone;

// %%Classes: ----------------------------------------------------------------
// simple class-factory: only knows how to create CSimpleObject instances
class CClassFactory : public IClassFactory {
  public:
    // IUnknown
    STDMETHODIMP    QueryInterface (REFIID riid, void** ppv);
    STDMETHODIMP_(ULONG) AddRef(void)  { return 1; };
    STDMETHODIMP_(ULONG) Release(void) { return 1; }

    // IClassFactory
    STDMETHODIMP    CreateInstance (LPUNKNOWN punkOuter, REFIID iid, void **ppv);
    STDMETHODIMP    LockServer (BOOL fLock) { return E_FAIL; };
    };

// simple object supporting a dummy IStream
class CSimpleObject : public IStream {
  public:
    // IUnknown
    STDMETHODIMP    QueryInterface (REFIID iid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef(void)  { return InterlockedIncrement(&m_cRef); };
    STDMETHODIMP_(ULONG) Release(void) { if (InterlockedDecrement(&m_cRef) == 0) { delete this; return 0; } return 1; }

    // IStream
    STDMETHODIMP    Read(void *pv, ULONG cb, ULONG *pcbRead);
    STDMETHODIMP    Write(VOID const *pv, ULONG cb, ULONG *pcbWritten);
    STDMETHODIMP    Seek(LARGE_INTEGER dbMove, DWORD dwOrigin, ULARGE_INTEGER *pbNewPosition)
        { return E_FAIL; }
    STDMETHODIMP    SetSize(ULARGE_INTEGER cbNewSize)
        { return E_FAIL; }
    STDMETHODIMP    CopyTo(IStream *pstm, ULARGE_INTEGER cb, ULARGE_INTEGER *pcbRead, ULARGE_INTEGER *pcbWritten)
        { return E_FAIL; }
    STDMETHODIMP    Commit(DWORD grfCommitFlags)
        { return E_FAIL; }
    STDMETHODIMP    Revert(void)
        { return E_FAIL; }
    STDMETHODIMP    LockRegion(ULARGE_INTEGER bOffset, ULARGE_INTEGER cb, DWORD dwLockType)
        { return E_FAIL; }
    STDMETHODIMP    UnlockRegion(ULARGE_INTEGER bOffset, ULARGE_INTEGER cb, DWORD dwLockType)
        { return E_FAIL; }
    STDMETHODIMP    Stat(STATSTG *pstatstg, DWORD grfStatFlag)
        { return E_FAIL; }
    STDMETHODIMP    Clone(IStream **ppstm)
        { return E_FAIL; }

    // constructors/destructors
    CSimpleObject()     { m_cRef = 1; }
    ~CSimpleObject()    { SetEvent(hevtDone); }

  private:
    LONG        m_cRef;
    };

// %%Globals: ----------------------------------------------------------------
CClassFactory   g_ClassFactory;

// ---------------------------------------------------------------------------
// %%Function: Message
// 
//  Formats and displays a message to the console.
// ---------------------------------------------------------------------------
 void
Message(LPTSTR szPrefix, HRESULT hr)
{
    LPTSTR   szMessage;

    if (hr == S_OK)
        {
        wprintf(szPrefix);
        wprintf(TEXT("\n"));
        return;
        }
 
    if (HRESULT_FACILITY(hr) == FACILITY_WINDOWS)
        hr = HRESULT_CODE(hr);

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        hr,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), //The user default language
        (LPTSTR)&szMessage,
        0,
        NULL );

    wprintf(TEXT("%s: %s(%lx)\n"), szPrefix, szMessage, hr);
    
    LocalFree(szMessage);
}  // Message

// ---------------------------------------------------------------------------
// %%Function: CSimpleObject::QueryInterface
// ---------------------------------------------------------------------------
 STDMETHODIMP
CSimpleObject::QueryInterface(REFIID riid, void** ppv)
{
    if (ppv == NULL)
        return E_INVALIDARG;
    if (riid == IID_IUnknown || riid == IID_IStream)
        {
        *ppv = (IUnknown *) this;
        AddRef();
        return S_OK;
        }
    *ppv = NULL;
    return E_NOINTERFACE;
}  // CSimpleObject::QueryInterface

// ---------------------------------------------------------------------------
// %%Function: CSimpleObject::Read
// ---------------------------------------------------------------------------
 STDMETHODIMP
CSimpleObject::Read(void *pv, ULONG cb, ULONG *pcbRead)
{
    Message(TEXT("Server: IStream:Read"), S_OK);
    if (!pv && cb != 0)
        return E_INVALIDARG;

    // fill the buffer with FF's. we could read it from somewhere.
    if (cb != 0)
        memset(pv, 0xFF, cb);

    if (pcbRead)
        *pcbRead = cb;
    return S_OK;
}  // CSimpleObject::Read

// ---------------------------------------------------------------------------
// %%Function: CSimpleObject::Write
// ---------------------------------------------------------------------------
 STDMETHODIMP
CSimpleObject::Write(VOID const *pv, ULONG cb, ULONG *pcbWritten)
{
    Message(TEXT("Server: IStream:Write"), S_OK);
    if (!pv && cb != 0)
        return E_INVALIDARG;
    // ignore the data, but we could examine it or put it somewhere
    if (pcbWritten)
        *pcbWritten = cb;
    return S_OK;
}  // CSimpleObject::Write

// ---------------------------------------------------------------------------
// %%Function: CClassFactory::QueryInterface
// ---------------------------------------------------------------------------
 STDMETHODIMP
CClassFactory::QueryInterface(REFIID riid, void** ppv)
{
    if (ppv == NULL)
        return E_INVALIDARG;
    if (riid == IID_IClassFactory || riid == IID_IUnknown)
        {
        *ppv = (IClassFactory *) this;
        AddRef();
        return S_OK;
        }
    *ppv = NULL;
    return E_NOINTERFACE;
}  // CClassFactory::QueryInterface

// ---------------------------------------------------------------------------
// %%Function: CClassFactory::CreateInstance
// ---------------------------------------------------------------------------
 STDMETHODIMP
CClassFactory::CreateInstance(LPUNKNOWN punkOuter, REFIID riid, void** ppv)
{
    LPUNKNOWN   punk;
    HRESULT     hr;

    *ppv = NULL;

    if (punkOuter != NULL)
        return CLASS_E_NOAGGREGATION;

    Message(TEXT("Server: IClassFactory:CreateInstance"), S_OK);

    punk = new CSimpleObject;

    if (punk == NULL)
        return E_OUTOFMEMORY;

    hr = punk->QueryInterface(riid, ppv);
    punk->Release();
    return hr;
}  // CClassFactory::CreateInstance

// ---------------------------------------------------------------------------
// %%Function: main
// ---------------------------------------------------------------------------
 void __cdecl
main()
{
    HRESULT hr;
    DWORD   dwRegister;

    // create the thread which is signaled when the instance is deleted
    hevtDone = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (hevtDone == NULL)
        {
        hr = HRESULT_FROM_WIN32(GetLastError());
        Message(TEXT("Server: CreateEvent"), hr);
        exit(hr);
        }

    // initialize COM for free-threading
    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (FAILED(hr))
        {
        Message(TEXT("Server: CoInitializeEx"), hr);
        exit(hr);
        }

    // register the class-object with OLE
    hr = CoRegisterClassObject(CLSID_SimpleObject, &g_ClassFactory,
        CLSCTX_SERVER, REGCLS_SINGLEUSE, &dwRegister);
    if (FAILED(hr))
        {
        Message(TEXT("Server: CoRegisterClassObject"), hr);
        exit(hr);
        }

    Message(TEXT("Server: Waiting"), S_OK);

    // wait until an object is created and deleted.
    WaitForSingleObject(hevtDone, INFINITE);

    CloseHandle(hevtDone);

    CoUninitialize();
    Message(TEXT("Server: Done"), S_OK);
}  // main

// EOF =======================================================================

