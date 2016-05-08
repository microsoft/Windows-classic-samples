//////////////////////////////////////////////////////////////////////////
//
// ReadOnlyPlugin.cpp : 
//
//  This file contains the implementation of a sample plugin for the 
//  photo acquisition process.  The sample plugin sets the read-only
//	attribute on acquired files.  
//
//  To enable the plugin, run regsvr32.exe with the built DLL as the 
//	argument.
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//////////////////////////////////////////////////////////////////////////


#define UNICODE
#include <windows.h>
#include <strsafe.h>
#include <windowsx.h>
#include <PhotoAcquire.h>
#include <PropKey.h>
#include "resource.h"
//////////////////////////////////////////////////////////////////////////
// Forward declarations for helper functions
HRESULT SetRegistryString(HKEY hKeyRoot, 
                          PCWSTR pszValueName, 
                          PCWSTR pszString, 
                          PCWSTR pszSubKeyFormat, 
                          ...);
HRESULT SetRegistryValueFormatV(HKEY hKeyRoot, 
                                PCWSTR pszValueName, 
                                DWORD dwType, 
                                const void* pValue, 
                                DWORD dwSize, 
                                PCWSTR pszSubKeyFormat, 
                                va_list pArgs);
HRESULT SetRegistryDWord(HKEY hKeyRoot, 
                         PCWSTR pszValueName, 
                         DWORD dwValue, 
                         PCWSTR pszSubKeyFormat,
                         ...);

//////////////////////////////////////////////////////////////////////////
//  Global static variables
//////////////////////////////////////////////////////////////////////////
// TODO: Change this CLSID for each new plugin you create.
// {08e2efd9-51ba-4d93-8d2f-2f0911d7ab9c}
static const CLSID CLSID_ThisPlugin = 
    {0x08e2efd9, 0x51ba, 0x4d93, {0x8d, 0x2f, 0x2f, 0x09, 0x11, 0xd7, 0xab, 0x9c}};

// TODO: Change this name for your own plugin. 
// It appears only in the registry, but serves to help identify the plugin.
static PCWSTR g_pszPluginRegistryName = L"Read-Only Photo Acquire Plugin";
static LONG g_nComponents = 0;
static LONG g_nServerLocks = 0;
static HINSTANCE g_hInstance = NULL;

void DllAddRef()
{
    InterlockedIncrement(&g_nComponents);
}

void DllRelease()
{
    InterlockedDecrement(&g_nComponents);
}

//////////////////////////////////////////////////////////////////////////
//  ThisPhotoAcquirePluginClassFactory
//  Description: A class that provides an implementation of IUnknown and
//  IClassFactory methods.  
//////////////////////////////////////////////////////////////////////////
class ThisPhotoAcquirePluginClassFactory : public IClassFactory
{
public:
    // Constructor and destructor
    ThisPhotoAcquirePluginClassFactory();
    ~ThisPhotoAcquirePluginClassFactory();

    // IUnknown
    STDMETHODIMP QueryInterface(const IID &iid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

    // IClassFactory
    STDMETHODIMP CreateInstance(IUnknown *pUnknownOuter, const IID &iid, void **ppvObject);
    STDMETHODIMP LockServer(BOOL bLock);

private:
    LONG m_cRef;
};
//////////////////////////////////////////////////////////////////////////
//  ThisPhotoAcquirePlugin
//  Description: A class that provides an implementation of 
//		IPhotoAcquirePlugin.
//////////////////////////////////////////////////////////////////////////
class ThisPhotoAcquirePlugin : public IPhotoAcquirePlugin
{
public:
    // Constructor and destructor
    ThisPhotoAcquirePlugin();
    ~ThisPhotoAcquirePlugin();

    // IUnknown
    STDMETHODIMP QueryInterface(const IID &iid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

    // IPhotoAcquirePlugin
    STDMETHODIMP Initialize(IPhotoAcquireSource* pPhotoAcquireSource, IPhotoAcquireProgressCB* pPhotoAcquireProgressCB);
    STDMETHODIMP ProcessItem(DWORD dwAcquireStage, IPhotoAcquireItem* pPhotoAcquireItem, IStream* pStream, LPCWSTR pszFinalFilename, IPropertyStore* pPropertyStore);
    STDMETHODIMP TransferComplete(HRESULT hr);
    STDMETHODIMP DisplayConfigureDialog(HWND hWndParent);

    static HRESULT CreateInstance(REFIID riid, void** ppv);

private:
    LONG m_cRef;
};

///////////////////////////////////////////////////////////////////////
//
//  Description:  Entry point for the DLL.
//  Arguments:  hinst - Handle to the DLL module. 
//              dwReason - Indicates why the DLL entry-point function 
//					is being called.
//				lpReserved - Not used in this implementation.
//
/////////////////////////////////////////////////////////////////////////
extern "C" BOOL WINAPI DllMain(HINSTANCE hinst, DWORD dwReason, LPVOID lpReserved)
{
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
        g_hInstance = hinst;
        break;
    }
    return TRUE;
}

///////////////////////////////////////////////////////////////////////
//
//  Description:  Enters required information into the registry and
//	registers the plugin.
//	The module description is entered at 
//	[HKEY_CLASSES_ROOT\CLSID\{CLSID}]
//	(where {CLSID} is the GUID for the plugin).
//	""="Module description"
//
//	The threading model and the module location are set under
//	[HKEY_CLASSES_ROOT\CLSID\{CLSID}\InProcServer32]
//	""=[path of this DLL]
//	ThreadingModel=Apartment
//
//  The display name for the plugin is set under
//  [HKLM\Software\Microsoft\Windows\CurrentVersion\Photo Acquisition\
//  Plugins\{CLSID}]
//	DisplayName=[path of this DLL]
//
//  The DWORD value that indicates whether or not the plugin is enabled
//  is set under
//  [HKCU\Software\Microsoft\Windows\CurrentVersion\Photo Acquisition\
//  Plugins\{CLSID}]
//  Enabled=1
///////////////////////////////////////////////////////////////////////

extern "C" STDMETHODIMP DllRegisterServer()
{
    HRESULT hr;
    // Convert our clsid to a string
    WCHAR szClsid[40];
    if (StringFromGUID2(CLSID_ThisPlugin, szClsid, ARRAYSIZE(szClsid)) != 0)
    {
        // Get the path and filename for this plugin
        WCHAR szModulePath[MAX_PATH + 1] = {0};
        DWORD dwResult = GetModuleFileName(g_hInstance, szModulePath, ARRAYSIZE(szModulePath) - 1);
        if (dwResult != 0)
        {
            // Make sure it isn't truncated
            if (dwResult < ARRAYSIZE(szModulePath) && szModulePath[ARRAYSIZE(szModulePath) - 2] == 0)
            {
                // Write the module description
                hr = SetRegistryString(HKEY_CLASSES_ROOT, L"", g_pszPluginRegistryName, L"CLSID\\%ws", szClsid);

                // Create the InprocServer32 regkey path
                if (SUCCEEDED(hr))
                {
                    hr = SetRegistryString(HKEY_CLASSES_ROOT, L"", szModulePath, L"CLSID\\%ws\\InprocServer32", szClsid);
                }

                // Create the threading registry key
                if (SUCCEEDED(hr))
                {
                    hr = SetRegistryString(HKEY_CLASSES_ROOT, L"ThreadingModel", L"Apartment", L"CLSID\\%ws\\InprocServer32", szClsid);
                }

                // Register the plugin
                if (SUCCEEDED(hr))
                {
                    // Create the displayname name
                    WCHAR szDisplayName[MAX_PATH + 3];
                    hr = StringCchPrintfW(szDisplayName, ARRAYSIZE(szDisplayName), L"@%ws,-1", szModulePath);
                    if (SUCCEEDED(hr))
                    {
                        hr = SetRegistryString(HKEY_LOCAL_MACHINE, L"DisplayName", szDisplayName, L"Software\\Microsoft\\Windows\\CurrentVersion\\Photo Acquisition\\Plugins\\%ws", szClsid);
                    }
                }

                // Enable the plugin for the current user
                if (SUCCEEDED(hr))
                {
                    hr = SetRegistryDWord(HKEY_CURRENT_USER, L"Enabled", 1, L"Software\\Microsoft\\Windows\\CurrentVersion\\Photo Acquisition\\Plugins\\%ws", szClsid);
                }
            }
            else
            {
                hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
            }
        }
        else
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }
    else
    {
        hr = E_UNEXPECTED;
    }
    return hr;
}
///////////////////////////////////////////////////////////////////////
//
//  Description:  Default implementation simply returns, since in this 
//  sample we don't need to remove information from the registry or
//  unregister the plugin.  
///////////////////////////////////////////////////////////////////////
extern "C" STDMETHODIMP DllUnregisterServer()
{
    return S_OK;
}
///////////////////////////////////////////////////////////////////////
//
//  Description:  Indicates whether the DLL may be unloaded.
///////////////////////////////////////////////////////////////////////
extern "C" STDMETHODIMP DllCanUnloadNow()
{
    if (g_nServerLocks == 0 && g_nComponents == 0)
    {
        return S_OK;
    }
    else
    {
        return S_FALSE;
    }
}
///////////////////////////////////////////////////////////////////////
//
//  Description:  Retrieves the class object.
///////////////////////////////////////////////////////////////////////
extern "C" STDAPI DllGetClassObject(const CLSID &clsid, 
                                    const IID &iid, 
                                    void **ppvObject)
{
    // Make sure we've got a valid ppvObject
    if (!ppvObject)
    {
        return E_INVALIDARG;
    }

    // Initialize out parameter
    *ppvObject = NULL;

    HRESULT hr;

    // Make sure this component is supplied by this server
    if (CLSID_ThisPlugin == clsid)
    {
        // Create class factory
        ThisPhotoAcquirePluginClassFactory* pThisPhotoAcquirePluginClassFactory = 
            new ThisPhotoAcquirePluginClassFactory;
        if (pThisPhotoAcquirePluginClassFactory != NULL)
        {
            // Get the requested interface
            hr = pThisPhotoAcquirePluginClassFactory->QueryInterface(iid, ppvObject);

            // Release
            pThisPhotoAcquirePluginClassFactory->Release();
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    else
    {
        hr = CLASS_E_CLASSNOTAVAILABLE;
    }
    return hr;
}


//----------------------------------------------------------------------
// ThisPhotoAcquirePluginClassFactory implementation
//----------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////
//
//  Description:  Constructor.
///////////////////////////////////////////////////////////////////////
ThisPhotoAcquirePluginClassFactory::ThisPhotoAcquirePluginClassFactory()
    : m_cRef(1)
{
}
///////////////////////////////////////////////////////////////////////
//
//  Description:  Destructor.
///////////////////////////////////////////////////////////////////////
ThisPhotoAcquirePluginClassFactory::~ThisPhotoAcquirePluginClassFactory()
{
}
///////////////////////////////////////////////////////////////////////
//
//  Description:  Implementation of IUnknown::QueryInterface.
///////////////////////////////////////////////////////////////////////
HRESULT ThisPhotoAcquirePluginClassFactory::QueryInterface(const IID &iid, void **ppvObject)
{
    if ((iid==IID_IUnknown) || (iid==IID_IClassFactory))
    {
        *ppvObject = static_cast<LPVOID>(this);
    }
    else
    {
        *ppvObject = NULL;
        return E_NOINTERFACE;
    }
    reinterpret_cast<IUnknown*>(*ppvObject)->AddRef();
    return S_OK;
}

///////////////////////////////////////////////////////////////////////
//
//  Description:  Implementation of IUnknown::AddRef.
///////////////////////////////////////////////////////////////////////
ULONG ThisPhotoAcquirePluginClassFactory::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

///////////////////////////////////////////////////////////////////////
//
//  Description:  Implementation of IUnknown::Release.
///////////////////////////////////////////////////////////////////////
ULONG ThisPhotoAcquirePluginClassFactory::Release()
{
    if (InterlockedDecrement(&m_cRef)==0)
    {
        delete this;
        return 0;
    }
    return m_cRef;
}

///////////////////////////////////////////////////////////////////////
//
//  Description:  Implementation of IClassFactory::CreateInstance.
///////////////////////////////////////////////////////////////////////
HRESULT ThisPhotoAcquirePluginClassFactory::CreateInstance(
    IUnknown *pUnknownOuter, 
    REFIID iid, 
    void **ppvObject)
{
    HRESULT hr;

    // Validate and initialize out parameter
    if (ppvObject == NULL)
    {
        return E_INVALIDARG;
    }
    *ppvObject = NULL;

    // No aggregation supported
    if (pUnknownOuter == NULL)
    {
        hr = ThisPhotoAcquirePlugin::CreateInstance(iid, ppvObject);
    }
    else
    {
        hr = CLASS_E_NOAGGREGATION;
    }

    return hr;
}
///////////////////////////////////////////////////////////////////////
//
//  Description:  Implementation of IClassFactory::LockServer.
///////////////////////////////////////////////////////////////////////
HRESULT ThisPhotoAcquirePluginClassFactory::LockServer(BOOL bLock)
{
    if (bLock)
    {
        InterlockedIncrement(&g_nServerLocks);
    }
    else
    {
        InterlockedDecrement(&g_nServerLocks);
    }
    return S_OK;
}

//----------------------------------------------------------------------
// ThisPhotoAcquirePlugin implementation
//----------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////
//
//  Description:  Constructor.
///////////////////////////////////////////////////////////////////////
ThisPhotoAcquirePlugin::ThisPhotoAcquirePlugin()
    : m_cRef(1)
{
    DllAddRef();
}
///////////////////////////////////////////////////////////////////////
//
//  Description:  Destructor.
///////////////////////////////////////////////////////////////////////
ThisPhotoAcquirePlugin::~ThisPhotoAcquirePlugin()
{
    DllRelease();
}

///////////////////////////////////////////////////////////////////////
//
//  Description:  Retrieve a new instance of the plugin.
///////////////////////////////////////////////////////////////////////
HRESULT ThisPhotoAcquirePlugin::CreateInstance(REFIID riid, void** ppv)
{
    HRESULT hr;
    ThisPhotoAcquirePlugin* pThisPhotoAcquirePlugin = 
        new ThisPhotoAcquirePlugin();
    if (pThisPhotoAcquirePlugin != NULL)
    {
        hr = pThisPhotoAcquirePlugin->QueryInterface(riid, ppv);
        pThisPhotoAcquirePlugin->Release();
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }
    return hr;
}
///////////////////////////////////////////////////////////////////////
//
//  Description:  Implementation of IUnknown::QueryInterface.
///////////////////////////////////////////////////////////////////////
HRESULT ThisPhotoAcquirePlugin::QueryInterface(const IID &iid, 
                                               void **ppvObject)
{
    if ((iid == IID_IUnknown) || (iid == IID_IPhotoAcquirePlugin))
    {
        *ppvObject = static_cast<LPVOID>(this);
    }
    else
    {
        *ppvObject = NULL;
        return E_NOINTERFACE;
    }
    reinterpret_cast<IUnknown*>(*ppvObject)->AddRef();
    return S_OK;
}

///////////////////////////////////////////////////////////////////////
//
//  Description:  Implementation of IUnknown::AddRef.
///////////////////////////////////////////////////////////////////////
ULONG ThisPhotoAcquirePlugin::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

///////////////////////////////////////////////////////////////////////
//
//  Description:  Implementation of IUnknown::Release.
///////////////////////////////////////////////////////////////////////
ULONG ThisPhotoAcquirePlugin::Release()
{
    if (InterlockedDecrement(&m_cRef)==0)
    {
        delete this;
        return 0;
    }
    return m_cRef;
}
///////////////////////////////////////////////////////////////////////
//
//  Description:  Implementation of IPhotoAcquirePlugin::Initialize.
//		This callback is invoked when the acquisition session begins.
//  Arguments:  pPhotoAcquireSource - Specifies the source from 
//					which photos are being acquired.
//				pPhotoAcquireProgressCB - Specifies the callback used 
//					to provide additional processing during acquisition.
///////////////////////////////////////////////////////////////////////
HRESULT ThisPhotoAcquirePlugin::Initialize(IPhotoAcquireSource*, 
                                           IPhotoAcquireProgressCB*)
{
    return S_OK;
}

///////////////////////////////////////////////////////////////////////
//
//  Description:  Implementation of IPhotoAcquirePlugin::ProcessItem.
//		This callback is invoked each time an item is acquired,
//		both before and after the item is saved.
//  Arguments:  dwAcquireStage - PAPS_PRESAVE or PAPS_POSTSAVE.
//					Specifies whether the item has been saved already. 
//				pPhotoAcquireItem - The item being acquired. 
//					Not used in this sample
//				pOriginalItemStream - Pointer to an IStream object 
//					for the original item.  NULL if dwAcquireStage is 
//					PAPS_POSTSAVE.  Not used in this sample.
//				pszFinalFilename - The file name of the destination of 
//					the item.  NULL if dwAcquireStage is PAPS_PRESAVE.
//				pPropertyStore - The item's property store.  NULL if 
//					dwAcquireStage is PAPS_POSTSAVE. 
//					Not used in this sample.
//				
///////////////////////////////////////////////////////////////////////
HRESULT ThisPhotoAcquirePlugin::ProcessItem(DWORD dwAcquireStage, 
                                            IPhotoAcquireItem*, 
                                            IStream*, 
                                            LPCWSTR pszFinalFilename, 
                                            IPropertyStore*)
{
    if (dwAcquireStage == PAPS_POSTSAVE)
    {
        SetFileAttributesW(pszFinalFilename, FILE_ATTRIBUTE_READONLY);
    }
    return S_OK;
}

///////////////////////////////////////////////////////////////////////
//
//  Description:  
//		Implementation of IPhotoAcquirePlugin::TransferComplete.
//		This callback is invoked when the acquisition session ends.
//		This implementation simply returns since it is not providing
//		any additional logic on transfer completion.
//  Arguments:  hrTransfer - HRESULT indicated the result of the
//					transfer session.  Not used in this example.
///////////////////////////////////////////////////////////////////////
HRESULT ThisPhotoAcquirePlugin::TransferComplete(HRESULT hrTransfer)
{
    return S_OK;
}

///////////////////////////////////////////////////////////////////////
//
//  Description:  
//		Implementation of IPhotoAcquirePlugin::DisplayConfigureDialog.
//		This callback is invoked when the acquisition configuration 
//		dialog is displayed.
// 		This implementation simply returns since it is not providing
//		any additional logic when the dialog displays.
//
//	Arguments: hWndParent - Handle to the configuration dialog window.
//					Not used in this example.
//		
///////////////////////////////////////////////////////////////////////
HRESULT ThisPhotoAcquirePlugin::DisplayConfigureDialog(HWND hWndParent)
{
    return E_NOTIMPL;
}

///////////////////////////////////////////////////////////////////////
//
//	Helper functions
//
///////////////////////////////////////////////////////////////////////
//
//  Description:  Helper function that creates a registry key and writes 
//					a formatted value to the key.
//					Called by the functions SetRegistryDWORD and 
//					SetRegistryString, below.
//  Arguments:  hKeyRoot - Root of the registry key. 
//					Typically one of the following predefined values:
//					HKEY_CLASSES_ROOT
//					HKEY_CURRENT_CONFIG
//					HKEY_CURRENT_USER
//					HKEY_LOCAL_MACHINE
//					HKEY_PERFORMANCE_DATA
//					HKEY_USERS
//              pszValueName - Pointer to a string containing the 
//					name of the value to set. 
//				dwType - Type of registry key value.
//				pValue - Pointer to a buffer containing the data 
//					to be stored with the specified value name. 
//				dwSize - Size of value.
//				pszSubKeyFormat - Pointer to a buffer containing a 
//					printf-style format string that indicates the 
//					format for the registry subkey to create. 
//					This string must be null-terminated.
//				pArgs - A va_list containing the arguments to be 
//					inserted into pszSubKeyFormat.
//
/////////////////////////////////////////////////////////////////////////
HRESULT SetRegistryValueFormatV(HKEY hKeyRoot, 
                                PCWSTR pszValueName, 
                                DWORD dwType, 
                                const void* pValue, 
                                DWORD dwSize, 
                                PCWSTR pszSubKeyFormat, 
                                va_list pArgs)
{
    WCHAR szSubKey[MAX_PATH];
    HRESULT hr = StringCchVPrintfW(szSubKey, ARRAYSIZE(szSubKey), pszSubKeyFormat, pArgs);
    if (SUCCEEDED(hr))
    {
        // Open the regkey
        HKEY hRegKey;
        LONG lResult = RegCreateKeyEx(hKeyRoot, szSubKey, 0, NULL, 0, KEY_WRITE, NULL, &hRegKey, NULL);
        hr = HRESULT_FROM_WIN32(lResult);
        if (SUCCEEDED(hr))
        {
            // Write the value
            lResult = RegSetValueEx(hRegKey, pszValueName, 0, dwType, (BYTE*)pValue, dwSize);
            hr = HRESULT_FROM_WIN32(lResult);
            RegCloseKey(hRegKey);
        }
    }
    return hr;
}
///////////////////////////////////////////////////////////////////////
//
//  Description:  Helper function that creates a string type registry 
//					key and writes a formatted string value to the key.
//  Arguments:  hKeyRoot - Root of the registry key. 
//					Typically one of the following predefined values:
//					HKEY_CLASSES_ROOT
//					HKEY_CURRENT_CONFIG
//					HKEY_CURRENT_USER
//					HKEY_LOCAL_MACHINE
//					HKEY_PERFORMANCE_DATA
//					HKEY_USERS
//              pszValueName - Pointer to a string containing the 
//					name of the value to set. 
//				pszString - Pointer to a string containing the 
//					string value.
//				pszSubKeyFormat - Pointer to a buffer containing a 
//					printf-style format string that indicates the 
//					format for the registry subkey to create. 
//					This string must be null-terminated.
//				... - A va_list containing the arguments to be 
//					inserted into pszSubKeyFormat.
//
/////////////////////////////////////////////////////////////////////////
HRESULT SetRegistryString(HKEY hKeyRoot, PCWSTR pszValueName, PCWSTR pszString, PCWSTR pszSubKeyFormat, ...)
{
    va_list pArgs;
    va_start(pArgs, pszSubKeyFormat);
    HRESULT hr = SetRegistryValueFormatV(hKeyRoot, 
                                            pszValueName, 
                                            REG_SZ, 
                                            pszString, 
                                            (lstrlenW(pszString) + 1) * sizeof(WCHAR), 
                                            pszSubKeyFormat, 
                                            pArgs);
    va_end(pArgs);
    return hr;
}
///////////////////////////////////////////////////////////////////////
//
//  Description:  Helper function that creates a DWORD type registry 
//					key and writes a formatted DWORD value to the key.
//  Arguments:  hKeyRoot - Root of the registry key to create.
//					Typically one of the following predefined values:
//					HKEY_CLASSES_ROOT
//					HKEY_CURRENT_CONFIG
//					HKEY_CURRENT_USER
//					HKEY_LOCAL_MACHINE
//					HKEY_PERFORMANCE_DATA
//					HKEY_USERS
//              pszValueName - Pointer to a string containing the 
//					name of the value to set. 
//				pszString - Pointer to a string containing the 
//					string value.
//				pszSubKeyFormat - Pointer to a buffer containing a 
//					printf-style format string that indicates the 
//					format for the registry subkey to create. 
//					This string must be null-terminated.
//				... - A va_list containing the arguments to be 
//					inserted into pszSubKeyFormat.
//
/////////////////////////////////////////////////////////////////////////
HRESULT SetRegistryDWord(HKEY hKeyRoot, PCWSTR pszValueName, DWORD dwValue, PCWSTR pszSubKeyFormat, ...)
{
    va_list pArgs;
    va_start(pArgs, pszSubKeyFormat);
    HRESULT hr = SetRegistryValueFormatV(hKeyRoot, 
                                            pszValueName, 
                                            REG_DWORD, 
                                            &dwValue, 
                                            sizeof(DWORD), 
                                            pszSubKeyFormat, 
                                            pArgs);
    va_end(pArgs);
    return hr;
}

