// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "resource.h"
#include "shvui.h"
#include "shvuicf.h"
#include "regutil.h"
#include "registration.h"
#include "DebugHelper.h"
#include <new>

SHVUIClassFactoryData* g_FactoryData = NULL;
HWND g_hwndDlg = NULL;
HINSTANCE g_hInst = NULL;

// Cache of the remote configuration 
// or non-default configuration (multi-config)
BYTE g_config = 0;

//
// g_fConfigModeMemoryOnly indicates that we are on remote configuration 
// or non-default configuration (multi-config).
// Hence instead of reading the configuration settings from registry, read it 
// from g_config, which a cache of the configuration result, 
// and the caller (NPS UI) will call SetConfig() or SetConfigToID() to persist it. 

// When configuring the remote machine, the local machine invokes 
// INapComponentConfig::GetConfig() on remote machine to obtain the remote 
// machine configuration, it then invokes local UI by calling 
// INapComponentConfig2::InvokeUIFromConfigBlob() and obtains user's changes. 
// It sets the new changes to remote machine using 
// INapComponentConfig::SetConfig().
//

// When configuring a multi-config configuration, the scenario is similar to 
// the remote configuration. The only difference is that
// INapComponentConfig::GetConfig() is replaced with INapComponentConfig3::GetConfigFromID() 
// and INapComponentConfig::SetConfig() is replaced with INapComponentConfig3::SetConfigToID()
//
BOOL g_fConfigModeMemoryOnly = FALSE;

INT_PTR CALLBACK DlgProcSHVPage(
             _In_ HWND hDlg, 
             _In_ UINT message, 
             _In_ WPARAM wParam, 
             _In_ LPARAM //lParam
             );

// Caller needs to pass in an array of size of MAX_PATH or bigger to use this function safely.
void AssembleRegkeyPathWithID (_Out_writes_(MAX_PATH) LPTSTR pConfigRegkeyWithID, _In_ UINT32 uConfigID);

int __stdcall wWinMain(
    _In_ HINSTANCE hInst,
    _In_opt_ HINSTANCE, // unreferenced hPrevInstance
    _In_ LPWSTR lpCmdLine,
    _In_ int // unreferenced nShowCmd
    ) 
{
    BOOL bExit = FALSE ;
    BOOL bShow = TRUE;
    BOOL bCOMInvoked = FALSE;

    int retCode = 0;
    BOOL bIsCOMInitialized = FALSE;

    WCHAR command[64];
    int nMax = sizeof(command)/sizeof(WCHAR);
    
    g_hInst = hInst;

    g_FactoryData = new (std::nothrow) SHVUIClassFactoryData();
    if (g_FactoryData == NULL)
    {
        DebugPrintfW(L" --- wWinMain - Out of memory");
        return FALSE;
    }

    HRESULT hr = CoInitialize(NULL) ;
    if (FAILED(hr))
    {
        DebugPrintfW(L" --- wWinMain - CoInitialize failed %#x!",hr);
        goto Cleanup;
    }

    bIsCOMInitialized = TRUE;

    // Check for "/RegServer" or "/UnRegServer" parameters.
    LPWSTR tokens = L"-/" ;
	LPWSTR nextToken = NULL;
    LPWSTR oneToken = wcstok_s(lpCmdLine, tokens, &nextToken) ; 
    while (oneToken != NULL)
    {
        int ret = LoadString(hInst, IDS_CMD_EMBEDDED, command, nMax);
        if (ret == 0)
        {
            DebugPrintfW(L" --- wWinMain - LoadString failed for IDS_CMD_EMBEDDED");
            goto Cleanup;
        }
        if (_wcsicmp(oneToken, command) == 0)
        {
            //Called through INapComponentConfig.  Hide until InvokeUI() call.
            bCOMInvoked = TRUE;
            bShow = FALSE;
        }
        ret = LoadString(hInst, IDS_CMD_UNREGSERVER, command, nMax);
        if (ret == 0)
        {
            DebugPrintfW(L" --- wWinMain - LoadString failed for IDS_CMD_UNREGSERVER");
            goto Cleanup;
        }
        if (_wcsicmp(oneToken, command) == 0)
        {
            ShvUIUnRegisterServer();
            // We are done, so exit.
            bExit = TRUE ;
        }
        ret = LoadString(hInst, IDS_CMD_REGSERVER, command, nMax);
        if (ret == 0)
        {
            DebugPrintfW(L" --- wWinMain - LoadString failed for IDS_CMD_REGSERVER");
            goto Cleanup;
        }
        if (_wcsicmp(oneToken, command) == 0)
        {
            ShvUIRegisterServer();
            // We are done, so exit.
            bExit = TRUE ;
        }

        oneToken = wcstok_s(NULL, tokens, &nextToken) ;
    }

    if (bExit)
    {
        goto Cleanup;
    }

    ShvUIClassFactory::StartFactory();

    if (bCOMInvoked)
    {
        //Process / dispatch messages, allow system to wait for COM method calls
        BOOL bContinue = TRUE;
        DWORD result;
        while (bContinue)
        {
            result = MsgWaitForMultipleObjects (
                                1,
                                NULL,
                                FALSE,
                                INFINITE,
                                QS_ALLINPUT);
            switch (result)
            {
                case WAIT_OBJECT_0 + 1:
                    MSG msg;
                    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
                    {
                        if (msg.message == WM_QUIT)
                        {
                            bContinue = FALSE;
                            PostQuitMessage(0);
                            break;
                        }
                        else
                        {
                            TranslateMessage(&msg);
                            DispatchMessage(&msg);
                        }
                    }
                    break;

                default:
                    break;
            }
        }

    }

Cleanup:

    ShvUIClassFactory::StopFactory();

    if (bIsCOMInitialized)
    {
        CoUninitialize();
    }

    if (g_FactoryData != NULL)
    {
        delete (g_FactoryData);
        g_FactoryData = NULL;
    }

    return retCode ;
}

ShvUI::ShvUI() : 
    m_cRef(0)
{
    InterlockedIncrement(&g_cObjRefCount);
}

ShvUI::~ShvUI()
{
    InterlockedDecrement(&g_cObjRefCount);

    //The only supported interface is destroyed...quit the application
    PostQuitMessage(0);
}


//Implementation of IUnknown

STDMETHODIMP ShvUI::QueryInterface(
    __RPC__in const IID& iid, 
    __RPC__out void** ppv)
{
    if (iid == IID_IUnknown)
    {
        *ppv = static_cast<INapComponentConfig*>(this);
    }
    else if (iid == IID_INapComponentConfig)
    {
        *ppv = static_cast<INapComponentConfig*>(this);
    }
    else if (iid == IID_INapComponentConfig2)
    {
        *ppv = static_cast<INapComponentConfig2*>(this);
    }
    else if (iid == IID_INapComponentConfig3)
    {
        *ppv = static_cast<INapComponentConfig3*>(this);
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }
    reinterpret_cast<IUnknown*>(*ppv)->AddRef();
    return S_OK;
}

ULONG ShvUI::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

ULONG ShvUI::Release()
{
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if (cRef == 0)
    {
        delete this;
    }
    return cRef;
}


// Implementation of INapComponentConfig

STDMETHODIMP ShvUI::IsUISupported(
    __RPC__out BOOL *isSupported)
{
    *isSupported = TRUE;
    return S_OK;
}

STDMETHODIMP ShvUI::InvokeUI(
    __RPC__in_opt HWND hwndParent)
{
    DialogBox(
      g_hInst, 
      MAKEINTRESOURCE(IDD_SDKSHV_DIALOG),
      hwndParent,
      DlgProcSHVPage
      );

    return S_OK;
}


STDMETHODIMP ShvUI::GetConfig(
    _Out_ UINT16 *bCount, 
    _Outptr_result_buffer_all_(*bCount) BYTE** data)
{
    if (!bCount || !data)
    {
        return E_INVALIDARG;
    }

    DWORD result = ERROR_SUCCESS;
    HRESULT hr = S_OK;
    PVOID isChecked = NULL;

    result = ShvuiGetRegistryValue(
                     HKEY_LOCAL_MACHINE,
                     SDKSHV_DEFAULT_CONFIG_KEY,
                     SHV_CONFIG_BLOB,
                     REG_DWORD,
                     &isChecked);

    *bCount = static_cast<UINT16>(sizeof(BYTE));
    *data = static_cast<BYTE*>(CoTaskMemAlloc(sizeof(BYTE)));
    if (*data)
    {
        if (result == ERROR_SUCCESS)
        {
            **data = reinterpret_cast<BYTE>(isChecked);
        }
        else
        {
            **data = TRUE;
            hr = __HRESULT_FROM_WIN32(result);
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}


STDMETHODIMP ShvUI::SetConfig(
    UINT16 bCount, 
    __RPC__in_ecount_full(bCount) BYTE* data)
{
    if (!data)
    {
        return E_INVALIDARG;
    }

    if (bCount == sizeof(g_config))
    {
        // set the cached data
        g_config = *data; 

        // commit the cache
        BOOL isChecked = g_config;
        ShvuiSetRegistryValue(
                        SDKSHV_DEFAULT_CONFIG_KEY,
                        NULL,
                        SHV_CONFIG_BLOB,
                        REG_DWORD,
                        reinterpret_cast<PBYTE>(&isChecked),
                        sizeof(DWORD));
    }
    return S_OK;
}

// Implementation of INapComponentConfig2

STDMETHODIMP ShvUI::IsRemoteConfigSupported(
    __RPC__out BOOL* isSupported, 
    __RPC__out UINT8* remoteConfigType)
{
    *isSupported = TRUE;
    *remoteConfigType = remoteConfigTypeConfigBlob;
    return S_OK;
}

STDMETHODIMP ShvUI::InvokeUIForMachine(
    __RPC__in_opt HWND hwndParent, 
    __RPC__in_opt CountedString* machineName)
{
    WCHAR command[64];
    int nMax = sizeof(command)/sizeof(WCHAR);
    LoadString(GetModuleHandle(NULL), IDS_INVOKE_UI, command, nMax);

    MessageBox(hwndParent, command, machineName->string, MB_OK);
    return S_OK;
}

STDMETHODIMP ShvUI::InvokeUIFromConfigBlob(
    __RPC__in_opt HWND hwndParent,
    __RPC__in UINT16 inbCount,
    __RPC__in_ecount_full(inbCount) BYTE *inData,
    __RPC__out UINT16 *outbCount,
    _Outptr_result_buffer_all_(*outbCount) BYTE **outdata,
    __RPC__out BOOL *fConfigChanged)
{
    HRESULT result = S_OK;

    if (inbCount != sizeof(g_config))
    {
         return E_INVALIDARG;
    }

    g_fConfigModeMemoryOnly = TRUE;

    // 
    // We should have set it TRUE only if the results before and after are different.
    // To keep it simple, set it TRUE without comparison.
    //
    *fConfigChanged = TRUE;

    g_config = *inData;

    DialogBox(
      g_hInst, 
      MAKEINTRESOURCE(IDD_SDKSHV_DIALOG),
      hwndParent,
      DlgProcSHVPage
      );

    *outbCount = static_cast<UINT16>(inbCount);
    *outdata = static_cast<BYTE*>(CoTaskMemAlloc(inbCount));
    if (*outdata)
    {
        **outdata = g_config;
    }
    else
    {
        result = E_OUTOFMEMORY;
    }

    g_fConfigModeMemoryOnly = FALSE;
    return result;
}


// Implementation of INapComponentConfig3

STDMETHODIMP ShvUI::NewConfig(
    UINT32 configID)
{
    // We have no action to take, and we act when SetConfigToID is called.
    UNREFERENCED_PARAMETER(configID);
    return S_OK;
}

STDMETHODIMP ShvUI::DeleteConfig(
    UINT32 configID)
{
    WCHAR configRegkeyWithID[MAX_PATH] = {0};
    AssembleRegkeyPathWithID(configRegkeyWithID, configID);
    ShvuiDeleteRegistryKey(configRegkeyWithID);
    return S_OK;
}

STDMETHODIMP ShvUI::DeleteAllConfig()
{
    ShvuiDeleteRegistryKey(SDKSHV_MULTI_CONFIG_KEY);
    return S_OK;
}

_Success_(return == 0)
STDMETHODIMP ShvUI::GetConfigFromID(
    _In_ UINT32 configID,
    _Out_ UINT16 *bCount,
    _Outptr_result_buffer_all_(*bCount) BYTE **outdata)
{
    if (!bCount || !outdata)
    {
        return E_INVALIDARG;
    }

    if (configID == DEFAULT_CONFIGURATION_ID)
    {
        return GetConfig(bCount, outdata);
    }

    HRESULT result = ERROR_SUCCESS;
    PVOID isChecked = NULL;

    WCHAR configRegkeyWithID[MAX_PATH] = {0};
    AssembleRegkeyPathWithID(configRegkeyWithID, configID);

    result = ShvuiGetRegistryValue(
                     HKEY_LOCAL_MACHINE,
                     configRegkeyWithID,
                     SHV_CONFIG_BLOB,
                     REG_DWORD,
                     &isChecked);

    *bCount = static_cast<UINT16>(sizeof(BYTE));
    *outdata = static_cast<BYTE*>(CoTaskMemAlloc(sizeof(BYTE)));
    if (*outdata)
    {
        **outdata = (result == ERROR_SUCCESS) ? reinterpret_cast<BYTE>(isChecked) : TRUE;
    }
    else
    {
        result = E_OUTOFMEMORY;
    }

    return result;
}

STDMETHODIMP ShvUI::SetConfigToID(
    UINT32 configID,
    UINT16 bCount,
    __RPC__in_ecount_full(bCount) BYTE *outdata)
{
    if (!outdata)
    {
        return E_INVALIDARG;
    }

    if (configID == DEFAULT_CONFIGURATION_ID)
    {
        return SetConfig(bCount, outdata);
    }

    WCHAR configRegkeyWithID[MAX_PATH] = {0};
    AssembleRegkeyPathWithID(configRegkeyWithID, configID);

    if (bCount == sizeof(g_config))
    {
        // commit the cache
        BOOL isChecked = *outdata;
        ShvuiSetRegistryValue(
                        configRegkeyWithID,
                        NULL,
                        SHV_CONFIG_BLOB,
                        REG_DWORD,
                        reinterpret_cast<PBYTE>(&isChecked),
                        sizeof(DWORD)
                        );
    }
    
    return S_OK;
}

DWORD ProcessCheckBox(
    _In_ HWND hDlg,
    _In_ int nIDCheckBox)
{
    DWORD result = ERROR_SUCCESS;
    BOOL isChecked = FALSE;

    isChecked = (BST_CHECKED == (BOOL)IsDlgButtonChecked(
                                                 hDlg,
                                                 nIDCheckBox
                                                 ));
    if(nIDCheckBox == IDC_SDKSHV_FW)
    {
        if (!g_fConfigModeMemoryOnly)
        {
            result = ShvuiSetRegistryValue(
                SDKSHV_DEFAULT_CONFIG_KEY,
                NULL,
                SHV_CONFIG_BLOB,
                REG_DWORD,
                reinterpret_cast<PBYTE>(&isChecked),
                sizeof(DWORD));
        } 
        else
        {
            if(isChecked)
            {
                g_config = 1;
            }
            else
            {
                g_config = 0;
            }
        }
    }

    return result;
}

DWORD OnClickOk(
    _In_ HWND hDlg)
{
    DWORD result = ERROR_SUCCESS;
    
    result = ProcessCheckBox(
            hDlg,
            IDC_SDKSHV_FW);
    DEBUGLOGRESULT(L"OnClickOk-ProcessCheckBox(IDC_SHV_FW)", result);

cleanup:    
    return result;
}


DWORD InitializeCheckBoxes(
    _In_ HWND hDlg)
{
    DWORD result = ERROR_SUCCESS;
    DWORD isChecked = FALSE;
    DWORD data = 0;

    if (g_fConfigModeMemoryOnly)
    {
         isChecked = g_config;
    }
    else
    {
        result = ShvuiGetRegistryValue(
                         HKEY_LOCAL_MACHINE,
                         SDKSHV_DEFAULT_CONFIG_KEY,
                         SHV_CONFIG_BLOB,
                         REG_DWORD,
                         (PVOID *)(&data));
        if (result == ERROR_SUCCESS)
        {
            isChecked = data;
        }
    }

    // if the value is not present, we will try and set a default value of 1    
    if (result == ERROR_FILE_NOT_FOUND)
    {
        isChecked = TRUE;
        result = ShvuiSetRegistryValue(
                        SDKSHV_DEFAULT_CONFIG_KEY,
                        NULL,
                        SHV_CONFIG_BLOB,
                        REG_DWORD,
                        reinterpret_cast<PBYTE>(&isChecked),
                        sizeof(DWORD));

        DEBUGLOGRESULT(L"SetCheckBox-ShvuiSetRegistryValue", result);        
    }
    else
    {
        DEBUGLOGRESULT(L"SetCheckBox-ShvuiGetRegistryValue", result);        
    }

    if (isChecked == TRUE)
    {
        if (FALSE == CheckDlgButton(
                hDlg,
                IDC_SDKSHV_FW,
                (isChecked == TRUE) ? BST_CHECKED : BST_UNCHECKED
                ))
            {
                result = GetLastError();
            }
    }

cleanup:
    return result;

}

static BOOLEAN bInitializing = TRUE;

// DlgProc
INT_PTR CALLBACK DlgProcSHVPage(
    _In_ HWND hDlg, 
    _In_ UINT message, 
    _In_ WPARAM wParam, 
    _In_ LPARAM /*lParam*/)
{    
    DWORD result = ERROR_SUCCESS;
    
    //
    // process message
    //    
    switch(message)
    {

        case WM_INITDIALOG:
            // read from the registries and set the check boxes as appropriate
            InitializeCheckBoxes(hDlg);
            bInitializing = FALSE;
            return TRUE;

        case WM_COMMAND:

            switch(LOWORD(wParam))
            {
                //Ignore check-boxes, these are processed in OnClickOK()
                case IDC_SDKSHV_FW:
                    return TRUE;

                case IDOK:
                    result = OnClickOk(hDlg);
                    if(ERROR_SUCCESS == result)
                    {
                        EndDialog(hDlg, 0);
                    }
                    return TRUE;
                    
                case IDAPPLY:
                    OnClickOk(hDlg);
                    break;
                    
                case IDCANCEL:
                    // end the dialog
                    EndDialog(hDlg, 0);
                    return TRUE;
                    
                default:
                    break;
            }
            break;

        case WM_QUIT:
            // end the dialog
              EndDialog(hDlg, 0);
		return TRUE;
        default:
            break;                
    }
    return FALSE;
}

void AssembleRegkeyPathWithID (
    _Out_writes_(MAX_PATH) LPTSTR pconfigRegkeyWithID, 
    _In_ UINT32 uConfigID)
{    
    ZeroMemory(pconfigRegkeyWithID, MAX_PATH);
    WCHAR configID[20] = {0};
    StringCchPrintf (configID, 20, L"\\%u", uConfigID);
    StringCchCopy (pconfigRegkeyWithID, MAX_PATH, SDKSHV_MULTI_CONFIG_KEY);
    StringCchCat  (pconfigRegkeyWithID, MAX_PATH, configID);
}

