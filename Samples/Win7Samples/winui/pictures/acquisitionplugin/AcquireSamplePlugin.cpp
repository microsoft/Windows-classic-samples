////////////////////////////////////////////////////////////////////////
//
// Sample acquisition plugin
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
// 
// Copyright (c) Microsoft Corporation. All rights reserved.
////////////////////////////////////////////////////////////////////////
//
//  This file contains the implementation of a sample plugin for the 
//  photo acquisition process.  This sample plugin demonstrates methods
//  of IPhotoAcquirePlugin and IUserInputString.
//  To enable the plugin, either run regsvr32.exe with the built DLL as the 
//	argument or run AcquireSamplePlugin.reg
//
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
// {9e683e3f-9a8e-4109-b067-0cd924db653e}
CLSID CLSID_AcquireSamplePlugin = {0x9e683e3f, 0x9a8e, 0x4109, {0xb0, 0x67, 0x0c, 0xd9, 0x24, 0xdb, 0x65, 0x3e}};

static LONG g_nComponents = 0;
static LONG g_nServerLocks = 0;
// TODO: Change this name for your own plugin. 
// It appears only in the registry, but serves to help identify the plugin.
static PCWSTR g_pszPluginRegistryName = L"Read-Only Photo Acquire Plugin";
static HINSTANCE g_hInstance = NULL;


//////////////////////////////////////////////////////////////////////////
//  ThisPhotoAcquirePluginClassFactory
//  Description: A class that provides an implementation of IUnknown and
//  IClassFactory methods.  
//////////////////////////////////////////////////////////////////////////
class AcquireSamplePluginClassFactory : public IClassFactory
{
public:
    // Constructor and destructor
    AcquireSamplePluginClassFactory();
    ~AcquireSamplePluginClassFactory();

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

////////////////////////////////////////////////////////////////////////
//
//  AcquireSamplePlugin
//	Description: Implements IPhotoAcquirePlugin and IUserInputString.
//	The IUserInputString methods specify properties of the dialog box
//	to display when IPhotoAcquireProgressCB::GetUserInput is called to
//	request string input from the user.
//
////////////////////////////////////////////////////////////////////////
class AcquireSamplePlugin : public IPhotoAcquirePlugin, IUserInputString
{
public:
    // Constructor and destructor
    AcquireSamplePlugin();
    ~AcquireSamplePlugin();

    // IUnknown
    STDMETHODIMP QueryInterface(const IID &iid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

    // IPhotoAcquirePlugin
    STDMETHODIMP Initialize(IPhotoAcquireSource* pPhotoAcquireSource, IPhotoAcquireProgressCB* pPhotoAcquireProgressCB);
    STDMETHODIMP ProcessItem(DWORD dwAcquireStage, IPhotoAcquireItem* pPhotoAcquireItem, IStream* pStream, LPCWSTR pszFinalFilename, IPropertyStore* pPropertyStore);
    STDMETHODIMP TransferComplete(HRESULT hr);
    STDMETHODIMP DisplayConfigureDialog(HWND hWndParent);

    // IUserInputString
    IFACEMETHODIMP GetSubmitButtonText(BSTR* pbstrSubmitButtonText);
    IFACEMETHODIMP GetPrompt(BSTR* pbstrPromptTitle);
    IFACEMETHODIMP GetStringId(BSTR* pbstrStringId);
    IFACEMETHODIMP GetStringType(USER_INPUT_STRING_TYPE* pnStringType);
    IFACEMETHODIMP GetTooltipText(__out BSTR* pbstrTooltipText);
    IFACEMETHODIMP GetMaxLength(UINT* pcchMaxLength);
    IFACEMETHODIMP GetDefault(BSTR* pbstrDefault);
    IFACEMETHODIMP GetMruCount(UINT* pnMruCount);
    IFACEMETHODIMP GetMruEntryAt(UINT nIndex, BSTR *pbstrMruEntry);
    IFACEMETHODIMP GetImage(UINT nSize, HBITMAP* phBitmap, HICON* phIcon);

    HRESULT SysAllocStringHelper(PCWSTR pszString, BSTR* pbstrString)
    {
        *pbstrString = SysAllocString(pszString);
        return *pbstrString ? S_OK : E_OUTOFMEMORY;
    }

    static HRESULT CreateInstance(REFIID riid, void** ppv);

private:
    LONG m_cRef;
    PROPVARIANT m_propVarSample;
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
void DllAddRef()
{
    InterlockedIncrement(&g_nComponents);
}
///////////////////////////////////////////////////////////////////////
//
//  Description:  Entry point for the DLL.
//  Arguments:  hinst - Handle to the DLL module. 
//              dwReason - Indicates why the DLL entry-point function 
//					is being called.
//				lpReserved - Not used in this implementation.
//
/////////////////////////////////////////////////////////////////////////
void DllRelease()
{
    InterlockedDecrement(&g_nComponents);
}
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
//	""=[Module description]
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
    if (StringFromGUID2(CLSID_AcquireSamplePlugin, szClsid, ARRAYSIZE(szClsid)) != 0)
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
extern "C" STDAPI DllGetClassObject(const CLSID &clsid, const IID &iid, void **ppvObject)
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
    if (CLSID_AcquireSamplePlugin == clsid)
    {
        // Create class factory
        AcquireSamplePluginClassFactory* pAcquireSamplePluginClassFactory = new AcquireSamplePluginClassFactory;
        if (pAcquireSamplePluginClassFactory != NULL)
        {
            // Get the requested interface
            hr = pAcquireSamplePluginClassFactory->QueryInterface(iid, ppvObject);

            // Release
            pAcquireSamplePluginClassFactory->Release();
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
// AcquireSamplePluginClassFactory implementation
//----------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////
//  Description:  Constructor.
///////////////////////////////////////////////////////////////////////
AcquireSamplePluginClassFactory::AcquireSamplePluginClassFactory()
    : m_cRef(1)
{
}
///////////////////////////////////////////////////////////////////////
//  Description:  Destructor.
///////////////////////////////////////////////////////////////////////
AcquireSamplePluginClassFactory::~AcquireSamplePluginClassFactory()
{
}
///////////////////////////////////////////////////////////////////////
//  Description:  Implementation of IUnknown::QueryInterface.
///////////////////////////////////////////////////////////////////////
HRESULT AcquireSamplePluginClassFactory::QueryInterface(const IID &iid, void **ppvObject)
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
//  Description:  Implementation of IUnknown::AddRef.
///////////////////////////////////////////////////////////////////////
ULONG AcquireSamplePluginClassFactory::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

///////////////////////////////////////////////////////////////////////
//  Description:  Implementation of IUnknown::Release.
///////////////////////////////////////////////////////////////////////
ULONG AcquireSamplePluginClassFactory::Release()
{
    if (InterlockedDecrement(&m_cRef)==0)
    {
        delete this;
        return 0;
    }
    return m_cRef;
}

///////////////////////////////////////////////////////////////////////
//  Description:  Implementation of IClassFactory::CreateInstance.
///////////////////////////////////////////////////////////////////////
HRESULT AcquireSamplePluginClassFactory::CreateInstance(IUnknown *pUnknownOuter, REFIID iid, void **ppvObject)
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
        hr = AcquireSamplePlugin::CreateInstance(iid, ppvObject);
    }
    else
    {
        hr = CLASS_E_NOAGGREGATION;
    }

    return hr;
}
///////////////////////////////////////////////////////////////////////
//  Description:  Implementation of IClassFactory::LockServer.
///////////////////////////////////////////////////////////////////////
HRESULT AcquireSamplePluginClassFactory::LockServer(BOOL bLock)
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
// AcquireSamplePlugin implementation
//----------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////
//  Description:  Constructor.
///////////////////////////////////////////////////////////////////////
AcquireSamplePlugin::AcquireSamplePlugin()
    : m_cRef(1)
{
    PropVariantInit(&m_propVarSample);
    DllAddRef();
}
///////////////////////////////////////////////////////////////////////
//  Description:  Destructor.
///////////////////////////////////////////////////////////////////////
AcquireSamplePlugin::~AcquireSamplePlugin()
{
    DllRelease();
}

///////////////////////////////////////////////////////////////////////
//
//  Description:  Retrieve a new instance of the plugin.
///////////////////////////////////////////////////////////////////////
HRESULT AcquireSamplePlugin::CreateInstance(REFIID riid, void** ppv)
{
    HRESULT hr;
    AcquireSamplePlugin* pAcquireSamplePlugin = new AcquireSamplePlugin;
    if (pAcquireSamplePlugin != NULL)
    {
        hr = pAcquireSamplePlugin->QueryInterface(riid, ppv);
        pAcquireSamplePlugin->Release();
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
HRESULT AcquireSamplePlugin::QueryInterface(const IID &iid, void **ppvObject)
{
    if ((iid == IID_IUnknown) || (iid == IID_IPhotoAcquirePlugin))
    {
        *ppvObject = static_cast<LPVOID>(this);
    }
    else if (iid == IID_IUserInputString)
    {
        *ppvObject = static_cast<IUserInputString*>(this);
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
ULONG AcquireSamplePlugin::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

///////////////////////////////////////////////////////////////////////
//
//  Description:  Implementation of IUnknown::Release.
///////////////////////////////////////////////////////////////////////
ULONG AcquireSamplePlugin::Release()
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
//
//		In this implementation, IPhotoAcquireProgressCB::GetUserInput
//		is called to prompt the user for a property value when import
//		begins.
//  Arguments:  pPhotoAcquireSource - Specifies the source from 
//					which photos are being acquired. Not used in this sample.
//				pPhotoAcquireProgressCB - Specifies the callback used 
//					to provide additional processing during acquisition.
///////////////////////////////////////////////////////////////////////
HRESULT AcquireSamplePlugin::Initialize(IPhotoAcquireSource* pPhotoAcquireSource, 
										IPhotoAcquireProgressCB* pPhotoAcquireProgressCB)
{
    UNREFERENCED_PARAMETER(pPhotoAcquireSource);
    IUserInputString* pUserInputString;
    HRESULT hr = QueryInterface(IID_IUserInputString, (void**)&pUserInputString);
    if (SUCCEEDED(hr))
    {
        PROPVARIANT propVarDefault = {0};
        propVarDefault.vt = VT_BSTR;
        hr = SysAllocStringHelper(L"Default Sample Property", &propVarDefault.bstrVal);
        if (SUCCEEDED(hr))
        {
            hr = pPhotoAcquireProgressCB->GetUserInput(IID_IUserInputString, pUserInputString, &m_propVarSample, &propVarDefault);
            PropVariantClear(&propVarDefault);
        }
        pUserInputString->Release();
    }
    return hr;
}
///////////////////////////////////////////////////////////////////////
//
//  Description:  Implementation of IPhotoAcquirePlugin::ProcessItem.
//		This callback is invoked each time an item is acquired,
//		both before and after the item is saved.  In this implementation
//		the item's Comment property is set before saving, and
//		a backup copy of the file is made after saving.
//  Arguments:  dwAcquireStage - Specifies whether the item has been 
//					saved yet.
//				pPhotoAcquireItem - The item being acquired. 
//					Not used in this sample
//				pStream - Pointer to an IStream object 
//					for the original item.  NULL if dwAcquireStage is 
//					PAPS_POSTSAVE.  Not used in this sample.
//				pszFinalFilename - The file name of the destination of 
//					the item.  NULL if dwAcquireStage is PAPS_PRESAVE.
//				pPropertyStore - The item's property store.  NULL if 
//					dwAcquireStage is PAPS_POSTSAVE. 
//				
///////////////////////////////////////////////////////////////////////
HRESULT AcquireSamplePlugin::ProcessItem(DWORD dwAcquireStage, 
										 IPhotoAcquireItem* pPhotoAcquireItem, 
										 IStream* pStream, 
										 LPCWSTR pszFinalFilename, 
										 IPropertyStore* pPropertyStore)
{
    UNREFERENCED_PARAMETER(pPhotoAcquireItem);
    UNREFERENCED_PARAMETER(pStream);
    HRESULT hr = S_OK;
    if (dwAcquireStage == PAPS_PRESAVE)
    {
        if (m_propVarSample.vt != VT_EMPTY)
        {
            hr = pPropertyStore->SetValue(PKEY_Comment, m_propVarSample);
        }
    }
    else if (dwAcquireStage == PAPS_POSTSAVE)
    {
        WCHAR szSampleBackupFile[MAX_PATH];
        if (SUCCEEDED(StringCchCopy(szSampleBackupFile, sizeof(szSampleBackupFile)/sizeof(szSampleBackupFile[0]), pszFinalFilename)))
        {
            if (SUCCEEDED(StringCchCat(szSampleBackupFile, sizeof(szSampleBackupFile)/sizeof(szSampleBackupFile[0]), L".original")))
            {
                CopyFile(pszFinalFilename, szSampleBackupFile, TRUE);
            }
        }
    }
    return hr;
}

///////////////////////////////////////////////////////////////////////
//
//  Description:  
//		Implementation of IPhotoAcquirePlugin::TransferComplete.
//		This callback is invoked when the acquisition session ends.
//		This implementation displays a message box on successful transfer.
//  Arguments:  hrTransfer - HRESULT indicated the result of the
//					transfer session.  
///////////////////////////////////////////////////////////////////////
HRESULT AcquireSamplePlugin::TransferComplete(HRESULT hrTransfer)
{
    if (SUCCEEDED(hrTransfer))
    {
        MessageBox(NULL, 
					L"Transfer Completed Successfully!", 
					L"Sample Acquire Plugin", 
					MB_ICONINFORMATION|MB_TASKMODAL);
    }
    return S_OK;
}

///////////////////////////////////////////////////////////////////////
//
//  Description:  
//		Implementation of IPhotoAcquirePlugin::DisplayConfigureDialog,
//		invoked when the acquisition configuration dialog is displayed.
//
//	Arguments: hWndParent - Handle to the configuration dialog window.
//					
///////////////////////////////////////////////////////////////////////
HRESULT AcquireSamplePlugin::DisplayConfigureDialog(HWND hWndParent)
{
    MessageBox(hWndParent, 
		L"AcquireSamplePlugin::DisplayConfigureDialog", 
		L"Sample Acquire Plugin", 
		MB_ICONINFORMATION|MB_TASKMODAL);
    return S_OK;
}


///////////////////////////////////////////////////////////////////////
//
//  Description:  
//		Implementation of IUserInputString::GetSubmitButtonText.
//		This function specifies the string to display in the submit button
//		when IPhotoAcquireProgressCB::GetUserInput is called to prompt the 
//		user for string input.
//  Arguments:  pbstrSubmitButtonText -- pointer to a BSTR containing the
//		submit button text.  
///////////////////////////////////////////////////////////////////////
HRESULT AcquireSamplePlugin::GetSubmitButtonText(BSTR* pbstrSubmitButtonText)
{
    return SysAllocStringHelper(L"Continue", pbstrSubmitButtonText);
}

///////////////////////////////////////////////////////////////////////
//
//  Description:  
//		Implementation of IUserInputString::GetPrompt.
//		This function specifies the string to display in the prompt
//		when IPhotoAcquireProgressCB::GetUserInput is called to prompt the 
//		user for string input.
//  Arguments:  pbstrPromptTitle - pointer to a BSTR to contain the prompt.  
///////////////////////////////////////////////////////////////////////
HRESULT AcquireSamplePlugin::GetPrompt(BSTR* pbstrPromptTitle)
{
    return SysAllocStringHelper(L"Enter sample text", pbstrPromptTitle);
}

///////////////////////////////////////////////////////////////////////
//
//  Description:  
//		Implementation of IUserInputString::GetStringID.
//		This function specifies the canonical name for the string requested
//		when IPhotoAcquireProgressCB::GetUserInput is called to prompt the 
//		user for string input.
//  Arguments:  pbstrStringId - pointer to a BSTR to contain the canonical name.    
///////////////////////////////////////////////////////////////////////
HRESULT AcquireSamplePlugin::GetStringId(BSTR* pbstrStringId)
{
    return SysAllocStringHelper(L"Sample plugin", pbstrStringId);
}

///////////////////////////////////////////////////////////////////////
//
//  Description:  
//		Implementation of IUserInputString::GetStringType.
//		This function specifies the format string requested
//		when IPhotoAcquireProgressCB::GetUserInput is called to prompt the 
//		user for string input.
//  Arguments:  pnStringType -- may be USER_INPUT_DEFAULT or 
//	USER_INPUT_PATH_ELEMENT
///////////////////////////////////////////////////////////////////////
HRESULT AcquireSamplePlugin::GetStringType(USER_INPUT_STRING_TYPE* pnStringType)
{
    *pnStringType = USER_INPUT_DEFAULT;
    return S_OK;
}
///////////////////////////////////////////////////////////////////////
//
//  Description:  
//		Implementation of IUserInputString::GetTooltipText.
//		This function specifies the tooltip text to display for the input box 
//		when IPhotoAcquireProgressCB::GetUserInput is called to prompt the 
//		user for string input.
//  Arguments:  pbstrTooltipText - pointer to a BSTR to contain the tooltip.    
///////////////////////////////////////////////////////////////////////
HRESULT AcquireSamplePlugin::GetTooltipText(__out BSTR* pbstrTooltipText)
{
    return SysAllocStringHelper(L"Sample plugin tooltip text", pbstrTooltipText);
}
///////////////////////////////////////////////////////////////////////
//
//  Description:  
//		Implementation of IUserInputString::GetMaxLength.
//		Specifies the value of the maximum length allowed for the input string.
//  Arguments:  pcchMaxLength - UINT indicating the length.  
///////////////////////////////////////////////////////////////////////
HRESULT AcquireSamplePlugin::GetMaxLength(UINT* pcchMaxLength)
{
    *pcchMaxLength = 30;
    return S_OK;
}

///////////////////////////////////////////////////////////////////////
//
//  Description:  
//		Implementation of IUserInputString::GetDefault.
//		Specifies the default text to display in the input box when
//		IPhotoAcquireProgressCB::GetUserInput is called to prompt the 
//		user for string input.
//  Arguments:  pbstrDefault - pointer to a BSTR to contain the default string.  
///////////////////////////////////////////////////////////////////////
HRESULT AcquireSamplePlugin::GetDefault(BSTR* pbstrDefault)
{
    return SysAllocStringHelper(L"Default Plugin Text", pbstrDefault);
}

///////////////////////////////////////////////////////////////////////
//
//  Description:  
//		Implementation of IUserInputString::GetMruCount.
//		Specifies the number of items in the list of most recently used items to
//		display in the input box when IPhotoAcquireProgressCB::GetUserInput 
//		is called to prompt the user for string input.
//  Arguments:  pnMruCount - pointer to UINT to receive the number of items.  
///////////////////////////////////////////////////////////////////////
HRESULT AcquireSamplePlugin::GetMruCount(UINT* pnMruCount)
{
    *pnMruCount = 0;
    return S_OK;
}

///////////////////////////////////////////////////////////////////////
//
//  Description:  
//		Implementation of IUserInputString::GetMruEntryAt.
//		Specifies the entry at the given index in the list of most recently 
//		used items to display in the input box when 
//		IPhotoAcquireProgressCB::GetUserInput is called to prompt the 
//		user for string input.
//  Arguments:  nIndex - Integer containing the index of the entry
//				pbstrMruEntry - Pointer to the string at the given index.  
///////////////////////////////////////////////////////////////////////
HRESULT AcquireSamplePlugin::GetMruEntryAt(UINT nIndex, BSTR *pbstrMruEntry)
{
    *pbstrMruEntry = NULL;
    return E_NOTIMPL;
}
///////////////////////////////////////////////////////////////////////
//
//  Description:  
//		Implementation of IUserInputString::GetImage.
//		Specifies the image to display in the dialog box when 
//		IPhotoAcquireProgressCB::GetUserInput is called to prompt the 
//		user for string input.  The image may be either a bitmap or an icon
//  Arguments:  nSize - UINT indicating the size of the image.  
//				phBitmap - pointer to the handle to a bitmap image
//				phIcon - pointer to a handle to an icon
///////////////////////////////////////////////////////////////////////
HRESULT AcquireSamplePlugin::GetImage(UINT /* nSize */, HBITMAP* phBitmap, HICON* phIcon)
{
	HRESULT hr = S_OK;

    if (NULL != phBitmap)
    {
        *phBitmap = NULL;
    }
	else
	{
		hr = E_INVALIDARG;
	}


    if (NULL != phIcon)
    {
        *phIcon = 
			reinterpret_cast<HICON>(LoadImage(
			g_hInstance, 
			MAKEINTRESOURCE(IDI_SAMPLEPLUGIN), //string indicating icon to load
			IMAGE_ICON,
			0, 0, // cx and cy = 0, so the actual size of the icon will be used.
			0));
        if (*phIcon == NULL)
        {
            hr = E_FAIL;
        }
    }
	else
	{
		hr = E_INVALIDARG;
	}
    return hr;
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