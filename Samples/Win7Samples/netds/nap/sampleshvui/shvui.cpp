// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF

// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO

// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A

// PARTICULAR PURPOSE.

//

// Copyright (c) Microsoft Corporation. All rights reserved.

#include "precomp.h"
#include "resource.h"
#include "shvui.h"
#include "shvuicf.h"
#include "regutil.h"

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
             HWND hDlg, 
             UINT message, 
             WPARAM wParam, 
             LPARAM //lParam
             );

// Caller needs to pass in an array of size of MAX_PATH or bigger to use this function safely.
void AssembleRegkeyPathWithID (__out_ecount(MAX_PATH) LPTSTR pwszConfigRegkeyWithID, __in UINT32 uConfigID);

int __stdcall wWinMain(
    __in HINSTANCE hInst, 
    __in_opt HINSTANCE, // unreferenced hPrevInst
    __in_opt LPWSTR lpCmdLine, 
    __in int // unreferenced nCmdShow
    ) 
{
    BOOL bExit = FALSE ;
    BOOL bShow = TRUE;
    BOOL bCOMInvoked = FALSE;

    int bRetCode = TRUE;
    BOOL bIsCOMInitialized = FALSE;

    WCHAR wszCmd[64];
    int nMax = sizeof(wszCmd)/sizeof(WCHAR);
    

    g_hInst = hInst;

    g_FactoryData = new SHVUIClassFactoryData();
    if (g_FactoryData == NULL)
    {
        return FALSE;
    }


    HRESULT hr = CoInitialize(NULL) ;
    if (FAILED(hr))
    {
        bRetCode = FALSE;
        goto Cleanup;
    }

    bIsCOMInitialized = TRUE;

    // Check for "/RegServer" or "/UnRegServer" parameters.
    LPWSTR szTokens = L"-/" ;
	LPWSTR szNextToken = NULL;
    LPWSTR szToken = wcstok_s(lpCmdLine, szTokens, &szNextToken) ; 
    while (szToken != NULL)
    {
        LoadString(hInst, IDS_CMD_EMBEDDED, wszCmd, nMax);
        if (_wcsicmp(szToken, wszCmd) == 0)
        {
            //Called through INapComponentConfig.  Hide until InvokeUI() call.
            bCOMInvoked = TRUE;
            bShow = FALSE;
        }
        LoadString(hInst, IDS_CMD_UNREGSERVER, wszCmd, nMax);
        if (_wcsicmp(szToken, wszCmd) == 0)
        {
            ShvUIClassFactory::Unregister() ;
            // We are done, so exit.
            bExit = TRUE ;
        }
        LoadString(hInst, IDS_CMD_REGSERVER, wszCmd, nMax);
        if (_wcsicmp(szToken, wszCmd) == 0)
        {
            ShvUIClassFactory::Register() ;
            // We are done, so exit.
            bExit = TRUE ;
        }

        szToken = wcstok_s(NULL, szTokens, &szNextToken) ;
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
        DWORD dwResult;
        while (bContinue)
        {
            dwResult = MsgWaitForMultipleObjects (
                                1,
                                NULL,
                                FALSE,
                                INFINITE,
                                QS_ALLINPUT);
            switch (dwResult)
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

    return bRetCode ;
}

ShvUI::~ShvUI()
{
    //The only supported interface is destroyed...quit the application
    PostQuitMessage(0);
}


//Implementation of INapComponentConfig
STDMETHODIMP ShvUI::QueryInterface(const IID& iid, void** ppv)
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


STDMETHODIMP ShvUI::IsUISupported(BOOL* isSupported)
{
    *isSupported = TRUE;
    return S_OK;
}

STDMETHODIMP ShvUI::InvokeUI(HWND hwndParent)
{
    DialogBox(
      g_hInst, 
      MAKEINTRESOURCE(IDD_SDKSHV_DIALOG),
      hwndParent,
      DlgProcSHVPage
      );

    return S_OK;
}


STDMETHODIMP
ShvUI::GetConfig(UINT16 *bCount, BYTE** data)
{
    if (!bCount || !data)
    {
        return E_INVALIDARG;
    }

    DWORD result = ERROR_SUCCESS;
    DWORD dwIsChecked = TRUE;

    result = ShvuiGetRegistryValue(
                     HKEY_LOCAL_MACHINE,
                     SDKSHV_DEFAULT_CONFIG_KEY,
                     SHV_CONFIG_BLOB,
                     REG_DWORD,
                     (PVOID*)&dwIsChecked
                     );

    *bCount = (UINT16) sizeof(BYTE);
    *data = (BYTE*) CoTaskMemAlloc(sizeof(BYTE));
    if (*data)
    {
        **data = (BYTE) dwIsChecked;
    }

    return result;
}



STDMETHODIMP
ShvUI::SetConfig(UINT16 bCount, BYTE* data)
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
                        (PBYTE)&isChecked,
                        sizeof(DWORD)
                        );
    }
    return S_OK;
}

STDMETHODIMP 
ShvUI::IsRemoteConfigSupported(BOOL* isSupported, UINT8* remoteConfigType)
{
    *isSupported = TRUE;
    *remoteConfigType = remoteConfigTypeConfigBlob;
    return S_OK;
}

STDMETHODIMP 
ShvUI::InvokeUIForMachine(HWND hwndParent, CountedString* machineName)
{
    MessageBox(hwndParent, L"InvokeUIForMachine", machineName->string, MB_OK);
    return S_OK;
}

STDMETHODIMP 
ShvUI::InvokeUIFromConfigBlob(
                                HWND hwndParent,
                                UINT16 inbCount,
                                BYTE* inData, 
                                UINT16* outbCount, 
                                BYTE** outdata, 
                                BOOL *fConfigChanged
                                      )
{
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

    *outbCount = (UINT16) inbCount;
    *outdata = (BYTE*)CoTaskMemAlloc(*outbCount);
    **outdata = g_config;

    g_fConfigModeMemoryOnly = FALSE;
    return S_OK;
}


// *********************************************************************
// INapComponentConfig3 methods:
// *********************************************************************

STDMETHODIMP ShvUI::NewConfig(UINT32 configID)
{
    // We have no action to take, and we act when SetConfigToID is called.
    UNREFERENCED_PARAMETER(configID);
    return S_OK;
}

STDMETHODIMP ShvUI::DeleteConfig(UINT32 configID)
{
    WCHAR wszConfigRegkeyWithID[MAX_PATH] = {0};
    AssembleRegkeyPathWithID(wszConfigRegkeyWithID, configID);
    ShvuiDeleteRegistryKey(wszConfigRegkeyWithID);
    return S_OK;
}

STDMETHODIMP ShvUI::DeleteAllConfig()
{
    ShvuiDeleteRegistryKey(SDKSHV_MULTI_CONFIG_KEY);
    return S_OK;
}

STDMETHODIMP ShvUI::GetConfigFromID(UINT32 configID, UINT16* bCount, BYTE** outdata)
{
    if (!bCount || !outdata)
    {
        return E_INVALIDARG;
    }

    if (configID == DEFAULT_CONFIGURATION_ID)
    {
        return GetConfig(bCount, outdata);
    }

    DWORD result = ERROR_SUCCESS;
    DWORD dwIsChecked = TRUE;

    WCHAR wszConfigRegkeyWithID[MAX_PATH] = {0};
    AssembleRegkeyPathWithID(wszConfigRegkeyWithID, configID);

    result = ShvuiGetRegistryValue(
                     HKEY_LOCAL_MACHINE,
                     wszConfigRegkeyWithID,
                     SHV_CONFIG_BLOB,
                     REG_DWORD,
                     (PVOID*)&dwIsChecked
                     );

    *bCount = (UINT16) sizeof(BYTE);
    *outdata = (BYTE*) CoTaskMemAlloc(sizeof(BYTE));
    if (*outdata)
    {
        **outdata = (BYTE)dwIsChecked;
    }

    return result;
}

STDMETHODIMP ShvUI::SetConfigToID(UINT32 configID, UINT16 bCount, BYTE* outdata)
{
    if (!outdata)
    {
        return E_INVALIDARG;
    }

    if (configID == DEFAULT_CONFIGURATION_ID)
    {
        return SetConfig(bCount, outdata);
    }

    WCHAR wszConfigRegkeyWithID[MAX_PATH] = {0};
    AssembleRegkeyPathWithID(wszConfigRegkeyWithID, configID);

    if (bCount == sizeof(g_config))
    {
        // commit the cache
        BOOL isChecked = *outdata;
        ShvuiSetRegistryValue(
                        wszConfigRegkeyWithID,
                        NULL,
                        SHV_CONFIG_BLOB,
                        REG_DWORD,
                        (PBYTE)&isChecked,
                        sizeof(DWORD)
                        );
    }
    
    return S_OK;
}

DWORD
ProcessCheckBox(
    HWND hDlg,
    int nIDCheckBox)
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
                (PBYTE)&isChecked,
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

DWORD
OnClickOk(HWND hDlg)
{
    DWORD result = ERROR_SUCCESS;
    
    result = ProcessCheckBox(
            hDlg,
            IDC_SDKSHV_FW);
    DEBUGLOGRESULT(L"OnClickOk-ProcessCheckBox(IDC_SHV_FW)", result);

cleanup:    
    return result;
}


DWORD
InitializeCheckBoxes(HWND hDlg)
{
    DWORD result = ERROR_SUCCESS;
    DWORD dwIsChecked = FALSE;

    if (g_fConfigModeMemoryOnly)
    {
         dwIsChecked = g_config;
    }
    else
    {
         result = ShvuiGetRegistryValue(
                         HKEY_LOCAL_MACHINE,
                         SDKSHV_DEFAULT_CONFIG_KEY,
                         SHV_CONFIG_BLOB,
                         REG_DWORD,
                         (PVOID*)&dwIsChecked
                         );
    }

    // if the value is not present, we will try and set a defautl value of 1    
    if (result == ERROR_FILE_NOT_FOUND)
    {
        dwIsChecked = TRUE;
        result = ShvuiSetRegistryValue(
                        SDKSHV_DEFAULT_CONFIG_KEY,
                        NULL,
                        SHV_CONFIG_BLOB,
                        REG_DWORD,
                        (PBYTE)&dwIsChecked,
                        sizeof(DWORD));

        DEBUGLOGRESULT(L"SetCheckBox-ShvuiSetRegistryValue", result);        
    }
    else
    {
        DEBUGLOGRESULT(L"SetCheckBox-ShvuiGetRegistryValue", result);        
    }

    if (dwIsChecked == TRUE)
    {
        if (FALSE == CheckDlgButton(
                hDlg,
                IDC_SDKSHV_FW,
                (dwIsChecked == TRUE) ? BST_CHECKED : BST_UNCHECKED
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
             HWND hDlg, 
             UINT message, 
             WPARAM wParam, 
             LPARAM //lParam
             )
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
                    result = OnClickOk(hDlg);
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

void AssembleRegkeyPathWithID (__out_ecount(MAX_PATH) LPTSTR pwszConfigRegkeyWithID, __in UINT32 uConfigID)
{    
    ZeroMemory(pwszConfigRegkeyWithID, MAX_PATH);
    WCHAR wszConfigID[20] = {0};
    StringCchPrintf (wszConfigID, 20, L"\\%u", uConfigID);
    StringCchCopy (pwszConfigRegkeyWithID, MAX_PATH, SDKSHV_MULTI_CONFIG_KEY);
    StringCchCat  (pwszConfigRegkeyWithID, MAX_PATH, wszConfigID);
}