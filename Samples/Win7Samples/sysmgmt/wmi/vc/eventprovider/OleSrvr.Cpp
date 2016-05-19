// **************************************************************************

//

// Copyright (c)  Microsoft Corporation, All Rights Reserved
//
// File:  OLESRVR.CPP
//
// Description:
//		  Event provider sample - COM server implementations
//
// History:
//
// **************************************************************************

#include <windows.h>
#include <stdio.h>
#include <time.h>
#include <locale.h>

#include <wbemidl.h>
#include <initguid.h>

#include "evprov.h"

#include <strsafe.h>

static HINSTANCE g_hInstance;

//***************************************************************************
//
//  DllMain
//
//***************************************************************************

BOOL WINAPI DllMain(
    HINSTANCE hinstDLL,
    DWORD dwReason,
    LPVOID lpReserved
    )
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        g_hInstance = hinstDLL;
        setlocale(LC_ALL, "");    
    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
    }

    return TRUE;
}



//***************************************************************************
//
//  class CFactory
//
//***************************************************************************

class CFactory : public IClassFactory
{
    ULONG           m_cRef;
    CLSID           m_ClsId;

public:
    CFactory(const CLSID & ClsId);
    ~CFactory();

    //
    // IUnknown members
    //
    STDMETHODIMP         QueryInterface(REFIID, LPVOID *);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

    //
    // IClassFactory members
    //
    STDMETHODIMP     CreateInstance(LPUNKNOWN, REFIID, LPVOID *);
    STDMETHODIMP     LockServer(BOOL);
};




//***************************************************************************
//
// Count number of objects and number of locks on this DLL.
//
//***************************************************************************

static ULONG g_cObj = 0;
static ULONG g_cLock = 0;



//***************************************************************************
//
//  DllGetClassObject
//
//***************************************************************************

extern "C"
HRESULT APIENTRY DllGetClassObject(
    REFCLSID rclsid,
    REFIID riid,
    LPVOID * ppv
    )
{
    HRESULT         hr;


    CFactory *pFactory;

    //
    //  Verify the caller is asking for our type of object.
    //
    if (CLSID_MyEventProvider != rclsid) 
            return E_FAIL;

    //
    // Check that we can provide the interface.
    //
    if (IID_IUnknown != riid && IID_IClassFactory != riid)
        return E_NOINTERFACE;

    //
    // Get a new class factory.
    //
    pFactory = new CFactory(rclsid);

    if (!pFactory)
        return E_OUTOFMEMORY;

    //
    // Verify we can get an instance.
    //
    hr = pFactory->QueryInterface(riid, ppv);

    if (FAILED(hr))
        delete pFactory;

    return hr;
}

//***************************************************************************
//
//  DllCanUnloadNow
//
//***************************************************************************
extern "C"
HRESULT APIENTRY DllCanUnloadNow(void)
{
    SCODE sc = TRUE;

    if (g_cObj || g_cLock)
        sc = S_FALSE;

    return sc;
}

//***************************************************************************
//
//***************************************************************************

void ObjectCreated(DWORD dwType)
{
    InterlockedIncrement((LONG *) &g_cObj);
}

void ObjectDestroyed(DWORD dwType)
{
    InterlockedDecrement((LONG *) &g_cObj);
}


//***************************************************************************
//
//  CFactory::CFactory
//
//***************************************************************************

CFactory::CFactory(const CLSID & ClsId)
{
    m_cRef = 0;
    m_ClsId = ClsId;
    ObjectCreated(0);
}

//***************************************************************************
//
//  CFactory::~CFactory
//
//***************************************************************************

CFactory::~CFactory()
{
    // nothing
    ObjectDestroyed(0);
}

//***************************************************************************
//
//  CFactory::QueryInterface, AddRef and Release
//
//***************************************************************************

STDMETHODIMP CFactory::QueryInterface(REFIID riid, LPVOID * ppv)
{
    *ppv = 0;

    if (IID_IUnknown==riid || IID_IClassFactory==riid)
    {
        *ppv = this;
        AddRef();
        return NOERROR;
    }

    return E_NOINTERFACE;
}


ULONG CFactory::AddRef()
{
    return ++m_cRef;
}


ULONG CFactory::Release()
{
    if (0 != --m_cRef)
        return m_cRef;
    delete this;
    return 0;
}

//***************************************************************************
//
//  CFactory::CreateInstance
//
//***************************************************************************

STDMETHODIMP CFactory::CreateInstance(
    LPUNKNOWN pUnkOuter,
    REFIID riid,
    LPVOID * ppvObj)
{
    IUnknown* pObj;
    HRESULT  hr;

    //
    //  Defaults
    //
    *ppvObj=NULL;
    hr = E_OUTOFMEMORY;

    //
    // We aren't supporting aggregation.
    //
    if (pUnkOuter)
        return CLASS_E_NOAGGREGATION;

    if (m_ClsId == CLSID_MyEventProvider)
    {
        pObj = (IWbemProviderInit *) new CMyEventProvider;
    }

    if (!pObj)
        return hr;

    //
    //  Initialize the object and verify that it can return the
    //  interface in question.
    //                                         
    hr = pObj->QueryInterface(riid, ppvObj);

    //
    // Kill the object if initial creation or Init failed.
    //
    if (FAILED(hr))
        delete pObj;

    return hr;
}

//***************************************************************************
//
//***************************************************************************

STDMETHODIMP CFactory::LockServer(BOOL fLock)
{
    if (fLock)
        InterlockedIncrement((LONG *) &g_cLock);
    else
        InterlockedDecrement((LONG *) &g_cLock);

    return NOERROR;
}




//***************************************************************************
//
//  DllRegisterServer
//
//  Standard OLE entry point for registering the server.
//
//  RETURN VALUES:
//
//      S_OK        Registration was successful
//      E_FAIL      Registration failed.
//
//***************************************************************************

extern "C"
HRESULT APIENTRY DllRegisterServer(void)
{
    wchar_t Path[MAX_PATH + 1];
    wchar_t *pGuidStr = 0;
    wchar_t KeyPath[1024];

    // Where are we?
    // =============
	memset (Path, NULL, sizeof(Path));
    GetModuleFileNameW(g_hInstance, Path, sizeof(Path)/sizeof(wchar_t) - 1);

    // Convert CLSID to string.
    // ========================

    StringFromCLSID(CLSID_MyEventProvider, &pGuidStr);
	StringCbPrintfW(KeyPath, sizeof(KeyPath), L"SOFTWARE\\Classes\\CLSID\\\\%s", pGuidStr);


    HKEY hKey;
	LONG lRes = RegCreateKeyExW( HKEY_LOCAL_MACHINE, KeyPath, 0, NULL,
                    REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, 
                    &hKey, NULL );
    if (lRes)
        return E_FAIL;

    wchar_t *pName = L"WMI Sample Event Provider";
    RegSetValueExW(hKey, 0, 0, REG_SZ, (const BYTE *) pName, 
		(DWORD)(wcslen(pName) * sizeof(wchar_t) + sizeof(wchar_t)));

    HKEY hSubkey;
	lRes = RegCreateKeyExW( hKey, L"InprocServer32", 0, NULL,
                    REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, 
                    &hSubkey, NULL );
	if (lRes)
        return E_FAIL;

    RegSetValueExW(hSubkey, 0, 0, REG_SZ, (const BYTE *) Path, 		
		(DWORD)(wcslen(Path) * sizeof(wchar_t) + sizeof(wchar_t)));
    RegSetValueExW(hSubkey, L"ThreadingModel", 0, REG_SZ, (const BYTE *) L"Both", 
		(DWORD)(wcslen(L"Both") * sizeof(wchar_t) + sizeof(wchar_t)));


    RegCloseKey(hSubkey);
    RegCloseKey(hKey);

    CoTaskMemFree(pGuidStr);

    return S_OK;
}

//***************************************************************************
//
//  DllUnregisterServer
//
//  Standard OLE entry point for unregistering the server.
//
//  RETURN VALUES:
//
//      S_OK        Unregistration was successful
//      E_FAIL      Unregistration failed.
//
//***************************************************************************

extern "C"
HRESULT APIENTRY DllUnregisterServer(void)
{
    wchar_t *pGuidStr = 0;
    HKEY hKey;
    wchar_t KeyPath[256];

    StringFromCLSID(CLSID_MyEventProvider, &pGuidStr);
	StringCbPrintfW(KeyPath, sizeof(KeyPath), L"SOFTWARE\\Classes\\CLSID\\%s", pGuidStr);

    // Delete InProcServer32 subkey.
    // =============================
    LONG lRes = RegOpenKeyExW(HKEY_LOCAL_MACHINE, KeyPath, 0, KEY_WRITE, &hKey);
    if (lRes)
        return E_FAIL;

    RegDeleteKeyW(hKey, L"InprocServer32");
    RegCloseKey(hKey);

    // Delete CLSID GUID key.
    // ======================

    lRes = RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Classes\\CLSID", 0, KEY_WRITE, &hKey);
    if (lRes)
        return E_FAIL;

    RegDeleteKeyW(hKey, pGuidStr);
    RegCloseKey(hKey);

    CoTaskMemFree(pGuidStr);

    return S_OK;
}

