/**************************************************************************
   THIS CODE AND INFORMATION IS PROVIDED 'AS IS' WITHOUT WARRANTY OF
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
   PARTICULAR PURPOSE.

   Copyright 2002 Microsoft Corporation.  All Rights Reserved.
**************************************************************************/

/**************************************************************************

   File:          ADEvent.cpp

   Description:   

**************************************************************************/

/**************************************************************************
   #include statements
**************************************************************************/

#include <olectl.h>
#include <shlobj.h>
#include <dsclient.h>
#include "ADEvent.h"

#include <initguid.h>
#include <dsadmin.h>

#include <tchar.h>
#include <stdio.h>

/**************************************************************************
   private function prototypes
**************************************************************************/

int WideCharToLocal(LPTSTR pLocal, size_t buffSize, LPWSTR pWide, DWORD dwChars);
int LocalToWideChar(LPWSTR pWide, size_t buffSize, LPTSTR pLocal, DWORD dwChars);
BOOL DeleteEntireKey(HKEY hKey, LPTSTR pszSubKey);

/**************************************************************************
   global variables and definitions
**************************************************************************/

// {08698521-653C-4386-B206-DFA3C0F904E5}
DEFINE_GUID(CLSID_AdminEventHandler, 
0x8698521, 0x653c, 0x4386, 0xb2, 0x6, 0xdf, 0xa3, 0xc0, 0xf9, 0x4, 0xe5);

HINSTANCE   g_hInst;
UINT        g_DllRefCount;
TCHAR       g_szMainTitle[] = TEXT("Sample AD Admin Notification Handler");


/**************************************************************************

    DllMain

**************************************************************************/

extern "C" BOOL WINAPI DllMain(  HINSTANCE hInstance, 
                                 DWORD dwReason, 
                                 LPVOID lpReserved)
{
    switch(dwReason)
    {
    case DLL_PROCESS_ATTACH:
        g_hInst = hInstance;
        break;
    }
   
    return TRUE;
}                                 

/**************************************************************************

    DllCanUnloadNow

**************************************************************************/

STDAPI DllCanUnloadNow(void)
{
    return (g_DllRefCount == 0) ? S_OK : S_FALSE;
}

/**************************************************************************

    DllGetClassObject

**************************************************************************/

STDAPI DllGetClassObject(   REFCLSID rclsid, 
                            REFIID riid, 
                            LPVOID *ppReturn)
{
    *ppReturn = NULL;

    //if we don't support this classid, return the proper error code
    if(!IsEqualCLSID(rclsid, CLSID_AdminEventHandler))
    {
        return CLASS_E_CLASSNOTAVAILABLE;
    }
       
    //create a CClassFactory object and check it for validity
    CClassFactory *pClassFactory = new CClassFactory();
    if(NULL == pClassFactory)
    {
        return E_OUTOFMEMORY;
    }
       
    //get the QueryInterface return for our return value
    HRESULT hr = pClassFactory->QueryInterface(riid, ppReturn);

    //call Release to decement the ref count - creating the object set it to one 
    //and QueryInterface incremented it - since its being used externally (not by 
    //us), we only want the ref count to be 1
    pClassFactory->Release();

    //return the result from QueryInterface
    return hr;
}

/**************************************************************************

    DllRegisterServer

**************************************************************************/

typedef struct _REGSTRUCT
{
    HKEY  hRootKey;
    LPTSTR lpszSubKey;
    LPTSTR lpszValueName;
    LPTSTR lpszData;
}REGSTRUCT, *LPREGSTRUCT;

STDAPI DllRegisterServer(void)
{
    int      i;
    HKEY     hKey;
    LRESULT  lResult;
    DWORD    dwDisp;
    TCHAR    szSubKey[MAX_PATH];
    TCHAR    szCLSID[MAX_PATH];
    TCHAR    szModule[MAX_PATH];
    LPWSTR   pwsz;

    //get the CLSID in string form
    StringFromIID(CLSID_AdminEventHandler, &pwsz);

    if(pwsz)
    {
        WideCharToLocal(szCLSID, ARRAYSIZE(szCLSID), pwsz, ARRAYSIZE(szCLSID));

        //free the string
        LPMALLOC pMalloc;
        CoGetMalloc(1, &pMalloc);
        if(pMalloc)
        {
            pMalloc->Free(pwsz);
            pMalloc->Release();
        }
    }

    //get this DLL's path and file name
    GetModuleFileName(g_hInst, szModule, ARRAYSIZE(szModule));

    //register the CLSID entries
    REGSTRUCT ClsidEntries[] = {HKEY_CLASSES_ROOT,   TEXT("CLSID\\%s"),                                   NULL,                   g_szMainTitle,
                                HKEY_CLASSES_ROOT,   TEXT("CLSID\\%s\\InprocServer32"),                   NULL,                   TEXT("%s"),
                                HKEY_CLASSES_ROOT,   TEXT("CLSID\\%s\\InprocServer32"),                   TEXT("ThreadingModel"), TEXT("Apartment"),
                                NULL,                NULL,                                                NULL,                   NULL};

    for(i = 0; ClsidEntries[i].hRootKey; i++)
    {
        //Create the sub key string.
        _stprintf_s(szSubKey, ARRAYSIZE(szSubKey), ClsidEntries[i].lpszSubKey, szCLSID);

        lResult = RegCreateKeyEx(   ClsidEntries[i].hRootKey,
                                    szSubKey,
                                    0,
                                    NULL,
                                    REG_OPTION_NON_VOLATILE,
                                    KEY_WRITE,
                                    NULL,
                                    &hKey,
                                    &dwDisp);
           
        if(NOERROR == lResult)
        {
            TCHAR szData[MAX_PATH] = TEXT("");

            //if necessary, create the value string
            _stprintf_s(szData, ARRAYSIZE(szData), ClsidEntries[i].lpszData, szModule);

            lResult = RegSetValueEx(    hKey,
                                        ClsidEntries[i].lpszValueName,
                                        0,
                                        REG_SZ,
                                        (LPBYTE)szData,
                                        lstrlen(szData) + 1);

            RegCloseKey(hKey);
        }
        else
        {
            return SELFREG_E_CLASS;
        }
    }

    return S_OK;
}

/**************************************************************************

   DllUnregisterServer()

**************************************************************************/

STDAPI DllUnregisterServer(VOID)
{
    LPWSTR   pwsz;
    TCHAR    szCLSID[MAX_PATH];
    TCHAR    szSubKey[MAX_PATH];

    //get the CLSID in string form
    StringFromIID(CLSID_AdminEventHandler, &pwsz);

    if(pwsz)
    {
        WideCharToLocal(szCLSID, ARRAYSIZE(szCLSID), pwsz, ARRAYSIZE(szCLSID));

        //free the string
        CoTaskMemFree(pwsz);
    }
    else
    {
        return E_FAIL;
    }

    //delete the object's registry entries
    _stprintf_s(szSubKey, ARRAYSIZE(szSubKey), TEXT("CLSID\\%s"), szCLSID);
    DeleteEntireKey(HKEY_CLASSES_ROOT, szSubKey);

    return S_OK;
}

///////////////////////////////////////////////////////////////////////////
//
// IClassFactory implementation
//

/**************************************************************************

    CClassFactory::CClassFactory

**************************************************************************/

CClassFactory::CClassFactory()
{
    m_ObjRefCount = 1;
    g_DllRefCount++;
}

/**************************************************************************

    CClassFactory::~CClassFactory

**************************************************************************/

CClassFactory::~CClassFactory()
{
    g_DllRefCount--;
}

/**************************************************************************

    CClassFactory::QueryInterface

**************************************************************************/

STDMETHODIMP CClassFactory::QueryInterface(  REFIID riid, 
                                             LPVOID FAR * ppReturn)
{
    *ppReturn = NULL;

    if(IsEqualIID(riid, IID_IUnknown))
    {
        *ppReturn = (LPUNKNOWN)(LPCLASSFACTORY)this;
    }
       
    if(IsEqualIID(riid, IID_IClassFactory))
    {
        *ppReturn = (LPCLASSFACTORY)this;
    }   

    if(*ppReturn)
    {
        (*(LPUNKNOWN*)ppReturn)->AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}                                             

/**************************************************************************

    CClassFactory::AddRef

**************************************************************************/

STDMETHODIMP_(DWORD) CClassFactory::AddRef()
{
    return ++m_ObjRefCount;
}


/**************************************************************************

    CClassFactory::Release

**************************************************************************/

STDMETHODIMP_(DWORD) CClassFactory::Release()
{
    if(--m_ObjRefCount == 0)
    {
        delete this;
        return 0;
    }
       
    return m_ObjRefCount;
}

/**************************************************************************

    CClassFactory::CreateInstance

**************************************************************************/

STDMETHODIMP CClassFactory::CreateInstance(  LPUNKNOWN pUnknown, 
                                             REFIID riid, 
                                             LPVOID FAR * ppObject)
{
    *ppObject = NULL;

    if(pUnknown != NULL)
    {
        return CLASS_E_NOAGGREGATION;
    }

    //add implementation specific code here

    CAdminNotifyHandler *pAdminNotifyHandler = new CAdminNotifyHandler;
    if(NULL == pAdminNotifyHandler)
    {
        return E_OUTOFMEMORY;
    }
      
    //get the QueryInterface return for our return value
    HRESULT hr = pAdminNotifyHandler->QueryInterface(riid, ppObject);

    //call Release to decement the ref count
    pAdminNotifyHandler->Release();

    //return the result from QueryInterface
    return hr;
}

/**************************************************************************

    CClassFactory::LockServer

**************************************************************************/

STDMETHODIMP CClassFactory::LockServer(BOOL)
{
    return E_NOTIMPL;
}


///////////////////////////////////////////////////////////////////////////
//
// CAdminNotifyHandler implementation
//

/**************************************************************************

    CAdminNotifyHandler::CAdminNotifyHandler()

**************************************************************************/

CAdminNotifyHandler::CAdminNotifyHandler()
{
    m_ObjRefCount = 1;
    g_DllRefCount++;
    m_pdoFrom = NULL;
    m_pdoTo = NULL;
}

/**************************************************************************

    CAdminNotifyHandler::~CAdminNotifyHandler()

**************************************************************************/

CAdminNotifyHandler::~CAdminNotifyHandler()
{
    g_DllRefCount--;

    End();
}

///////////////////////////////////////////////////////////////////////////
//
// IUnknown Implementation
//

/**************************************************************************

    CAdminNotifyHandler::QueryInterface

**************************************************************************/

STDMETHODIMP CAdminNotifyHandler::QueryInterface( REFIID riid, 
                                            LPVOID FAR * ppReturn)
{
    *ppReturn = NULL;

    //IUnknown
    if(IsEqualIID(riid, IID_IUnknown))
    {
        *ppReturn = (LPVOID)this;
    }

    //IDsAdminNotifyHandler
    if(IsEqualIID(riid, IID_IDsAdminNotifyHandler))
    {
        *ppReturn = (IDsAdminNotifyHandler*)this;
    }   

    if(*ppReturn)
    {
        (*(LPUNKNOWN*)ppReturn)->AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}                                             

/**************************************************************************

    CAdminNotifyHandler::AddRef

**************************************************************************/

STDMETHODIMP_(DWORD) CAdminNotifyHandler::AddRef()
{
    return ++m_ObjRefCount;
}


/**************************************************************************

    CAdminNotifyHandler::Release

**************************************************************************/

STDMETHODIMP_(DWORD) CAdminNotifyHandler::Release()
{
    if(--m_ObjRefCount == 0)
    {
        delete this;
        return 0;
    }
       
    return m_ObjRefCount;
}

///////////////////////////////////////////////////////////////////////////
//
// IDsAdminNotifyHandler Implementation
//

/**************************************************************************

    CAdminNotifyHandler::Initialize()

**************************************************************************/

STDMETHODIMP CAdminNotifyHandler::Initialize(IDataObject* pExtraInfo, 
                                             ULONG* puEventFlags)
{
    OutputDebugString(TEXT("CAdminNotifyHandler::Initialize\n"));
    
    if(NULL == puEventFlags)
    {
        return E_INVALIDARG;
    }
    
    //Tell the host which events the handler wants to receive.
    *puEventFlags = DSA_NOTIFY_ALL;
    
    return S_OK;
}

/**************************************************************************

    CAdminNotifyHandler::Begin()

**************************************************************************/

STDMETHODIMP CAdminNotifyHandler::Begin(ULONG uEvent, 
                                        IDataObject* pArg1, 
                                        IDataObject* pArg2, 
                                        ULONG* puFlags, 
                                        BSTR* pBstr)
{
    OutputDebugString(TEXT("CAdminNotifyHandler::Begin\n"));

    if(NULL == pArg1)
    {
        return E_INVALIDARG;
    }
    if(NULL == puFlags)
    {
        return E_INVALIDARG;
    }
    if(NULL == pBstr)
    {
        return E_INVALIDARG;
    }
    
    //release any data objects this handler may already have
    End();

    //save the data objects
    pArg1->QueryInterface(IID_IDataObject, (LPVOID*)&m_pdoFrom);

    if(NULL != pArg2)
    {
        pArg2->QueryInterface(IID_IDataObject, (LPVOID*)&m_pdoTo);
    }

    STGMEDIUM   stm;
    FORMATETC   fe;
    HRESULT     hr;

    fe.cfFormat = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_DSOBJECTNAMES);
    fe.ptd = NULL;
    fe.dwAspect = DVASPECT_CONTENT;
    fe.lindex = -1;
    fe.tymed = TYMED_HGLOBAL;
    hr = pArg1->GetData(&fe, &stm);
    if(SUCCEEDED(hr))
    {
        LPDSOBJECTNAMES pdson = (LPDSOBJECTNAMES)GlobalLock(stm.hGlobal);

        if(pdson)
        {
            LPWSTR  pwszName = (LPWSTR)((LPBYTE)pdson + pdson->aObjects[0].offsetName);
            LPWSTR  pwszClass = (LPWSTR)((LPBYTE)pdson + pdson->aObjects[0].offsetClass);

            OutputDebugString(TEXT("\t"));
            OutputDebugStringW(pwszClass);
            OutputDebugString(TEXT(" - "));
            OutputDebugStringW(pwszName);
            OutputDebugString(TEXT("\n"));

            GlobalUnlock(stm.hGlobal);
        }
    
        ReleaseStgMedium(&stm);
    }

    if(pArg2)
    {
        fe.cfFormat = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_DSOBJECTNAMES);
        fe.ptd = NULL;
        fe.dwAspect = DVASPECT_CONTENT;
        fe.lindex = -1;
        fe.tymed = TYMED_HGLOBAL;
        hr = pArg2->GetData(&fe, &stm);
        if(SUCCEEDED(hr))
        {
            LPDSOBJECTNAMES pdson = (LPDSOBJECTNAMES)GlobalLock(stm.hGlobal);

            if(pdson)
            {
                LPWSTR  pwszName = (LPWSTR)((LPBYTE)pdson + pdson->aObjects[0].offsetName);
                LPWSTR  pwszClass = (LPWSTR)((LPBYTE)pdson + pdson->aObjects[0].offsetClass);

                OutputDebugString(TEXT("\t"));
                OutputDebugStringW(pwszClass);
                OutputDebugString(TEXT(" - "));
                OutputDebugStringW(pwszName);
                OutputDebugString(TEXT("\n"));

                GlobalUnlock(stm.hGlobal);
            }
    
            ReleaseStgMedium(&stm);
        }
    }

    *puFlags = DSA_NOTIFY_FLAG_ADDITIONAL_DATA |
                DSA_NOTIFY_FLAG_FORCE_ADDITIONAL_DATA;

    WCHAR   wszTitle[MAX_PATH];
    LocalToWideChar(wszTitle, ARRAYSIZE(wszTitle), g_szMainTitle, MAX_PATH);
    *pBstr = SysAllocString(wszTitle);

    return *pBstr ? S_OK : E_OUTOFMEMORY;
}

/**************************************************************************

    CAdminNotifyHandler::Notify()

**************************************************************************/

STDMETHODIMP CAdminNotifyHandler::Notify(ULONG nItem, ULONG uFlags)
{
    OutputDebugString(TEXT("CAdminNotifyHandler::Notify\n"));
    
    return S_OK;
}

/**************************************************************************

    CAdminNotifyHandler::End()

**************************************************************************/

STDMETHODIMP CAdminNotifyHandler::End(void)
{
    OutputDebugString(TEXT("CAdminNotifyHandler::End\n"));

    if(m_pdoFrom)
    {
        m_pdoFrom->Release();
        m_pdoFrom = NULL;
    }
    if(m_pdoTo)
    {
        m_pdoTo->Release();
        m_pdoTo = NULL;
    }

    return S_OK;
}

    
/**************************************************************************

    WideCharToLocal()
   
**************************************************************************/

int WideCharToLocal(LPTSTR pLocal, size_t buffSize, LPWSTR pWide, DWORD dwChars)
{
    *pLocal = 0;

    #ifdef UNICODE
    wcsncpy_s(pLocal, buffSize, pWide, dwChars);
    #else
    WideCharToMultiByte(    CP_ACP, 
                            0, 
                            pWide, 
                            -1, 
                            pLocal, 
                            dwChars, 
                            NULL, 
                            NULL);
    #endif

    return lstrlen(pLocal);
}

/**************************************************************************

    LocalToWideChar()
   
**************************************************************************/

int LocalToWideChar(LPWSTR pWide, size_t buffSize, LPTSTR pLocal, DWORD dwChars)
{
    *pWide = 0;

    #ifdef UNICODE
    wcsncpy_s(pWide, buffSize, pLocal, dwChars);
    #else
    MultiByteToWideChar(    CP_ACP, 
                            0, 
                            pLocal, 
                            -1, 
                            pWide, 
                            dwChars); 
    #endif

    return lstrlenW(pWide);
}

/**************************************************************************

   DeleteEntireKey()

**************************************************************************/

BOOL DeleteEntireKey(HKEY hKey, LPTSTR pszSubKey)
{
    LRESULT  lResult;
    HKEY     hEnumKey;

    lResult = RegOpenKeyEx( hKey,
                            pszSubKey,
                            0,
                            KEY_ENUMERATE_SUB_KEYS,
                            &hEnumKey);

    if(NOERROR == lResult)
    {
        TCHAR szKey[MAX_PATH];
        DWORD dwSize = MAX_PATH;

        while(ERROR_SUCCESS == RegEnumKeyEx(hEnumKey, 0, szKey, &dwSize, NULL, NULL, NULL, NULL))
        {
            DeleteEntireKey(hEnumKey, szKey);

            dwSize = MAX_PATH;
        }
   
        RegCloseKey(hEnumKey);
    }
    else
    {
        return FALSE;
    }

    RegDeleteKey(hKey, pszSubKey);

    return TRUE;
}

