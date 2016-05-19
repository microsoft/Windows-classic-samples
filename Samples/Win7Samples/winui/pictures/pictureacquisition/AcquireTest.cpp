/*
 * Copyright (c) Microsoft Corporation. All rights reserved.
 *
 * Sample application that demonstrates photo acquisitions 
 * using methods of IPhotoAcquire
 *
 */
#define UNICODE
#include <windows.h>
#include <windowsx.h>
#include <PhotoAcquire.h>
#include <shlobj.h>
#include <shellapi.h>
#include <StrSafe.h>
#include "resource.h"


HINSTANCE g_hInstance = NULL;
bool      g_fSuppressErrorMessageDialog = false;
bool      g_fInfiniteLoop               = false;
bool      g_fDeleteFilesWhenDone        = false;

void OutputFormattedDebugString(PCWSTR pszFormat, ...)
{
    TCHAR szBuffer[1024];
    va_list pArgPtr;
    va_start(pArgPtr, pszFormat);
    StringCchVPrintf(szBuffer, ARRAYSIZE(szBuffer), pszFormat, pArgPtr);
    va_end(pArgPtr);

    OutputDebugString(szBuffer);
}

class PhotoAcquireProgressCB : public IPhotoAcquireProgressCB
{
private:
    LONG m_cRef;

private:
    PhotoAcquireProgressCB()
        : m_cRef(1)
    {
    }
    virtual ~PhotoAcquireProgressCB()
    {
    }

public:
    // IUnknown methods
    STDMETHODIMP_(ULONG) AddRef()
    {
        return InterlockedIncrement(&m_cRef);
    }


    STDMETHODIMP_(ULONG) Release()
    {
        LONG nRefCount = InterlockedDecrement(&m_cRef);
        if (!nRefCount)
        {
            delete this;
        }
        return nRefCount;
    }

    STDMETHODIMP QueryInterface(REFIID riid, LPVOID *ppvObject)
    {
        if (IsEqualIID(riid, IID_IUnknown))
        {
            *ppvObject = static_cast<IPhotoAcquireProgressCB*>(this);
        }
        else if (IsEqualIID(riid, IID_IPhotoAcquireProgressCB))
        {
            *ppvObject = static_cast<IPhotoAcquireProgressCB*>(this);
        }
        else
        {
            *ppvObject = NULL;
            return E_NOINTERFACE;
        }
        reinterpret_cast<IUnknown*>(*ppvObject)->AddRef();
        return S_OK;
    }


    // IPhotoAcquireProgressCB methods
    STDMETHODIMP Cancelled(__out BOOL *pfCancelled)
    {
        OutputFormattedDebugString(L"Cancelled(pfCancelled: 0x%p)\n", pfCancelled);
        return E_NOTIMPL;
    }

    STDMETHODIMP StartEnumeration(IPhotoAcquireSource *pPhotoAcquireSource)
    {
        OutputFormattedDebugString(L"StartEnumeration(pPhotoAcquireSource: 0x%p)\n", pPhotoAcquireSource);
        return E_NOTIMPL;
    }
    STDMETHODIMP FoundItem(IPhotoAcquireItem *pPhotoAcquireItem)
    {
        OutputFormattedDebugString(L"FoundItem(pPhotoAcquireItem: 0x%p)\n", pPhotoAcquireItem);
        return E_NOTIMPL;
    }
    STDMETHODIMP EndEnumeration(HRESULT hrEnumerate)
    {
        OutputFormattedDebugString(L"EndEnumeration(hr: 0x%08X)\n", hrEnumerate);
        return E_NOTIMPL;
    }

    STDMETHODIMP StartTransfer(IPhotoAcquireSource *pPhotoAcquireSource)
    {
        OutputFormattedDebugString(L"StartTransfer(pPhotoAcquireSource: 0x%p)\n", pPhotoAcquireSource);
        return E_NOTIMPL;
    }
    STDMETHODIMP StartItemTransfer(UINT nItemIndex, IPhotoAcquireItem *pPhotoAcquireItem)
    {
        OutputFormattedDebugString(L"StartItemTransfer(nItemIndex: %u, pPhotoAcquireItem: 0x%p)\n", nItemIndex, pPhotoAcquireItem);
        return E_NOTIMPL;
    }
    STDMETHODIMP DirectoryCreated(LPCWSTR pszDirectory)
    {
        OutputFormattedDebugString(L"DirectoryCreated(pszDirectory: %ws)\n", pszDirectory);
        return E_NOTIMPL;
    }
    
    STDMETHODIMP UpdateTransferPercent(BOOL fOverall, UINT nPercent)
    {
        OutputFormattedDebugString(L"UpdateTransferPercent(fOverall: %d, nPercent: %u)\n", fOverall, nPercent);
        return E_NOTIMPL;
    }
    STDMETHODIMP EndItemTransfer(UINT nItemIndex, IPhotoAcquireItem *pPhotoAcquireItem, HRESULT hr)
    {
        OutputFormattedDebugString(L"EndItemTransfer(nItemIndex: %d, pPhotoAcquireItem: %p, hr: 0x%08X)\n", nItemIndex, pPhotoAcquireItem, hr);
        return E_NOTIMPL;
    }
    STDMETHODIMP EndTransfer(HRESULT hrTransfer)
    {
        OutputFormattedDebugString(L"EndTransfer(hr: 0x%08X)\n", hrTransfer);
        return E_NOTIMPL;
    }
    STDMETHODIMP StartDelete(IPhotoAcquireSource* pPhotoAcquireSource)
    {
        OutputFormattedDebugString(L"StartDelete(pPhotoAcquireSource: 0x%p)\n", pPhotoAcquireSource);
        return E_NOTIMPL;
    }
    STDMETHODIMP StartItemDelete(UINT nItemIndex, IPhotoAcquireItem *pPhotoAcquireItem)
    {
        OutputFormattedDebugString(L"StartItemDelete(nItemIndex: %u, pPhotoAcquireItem: 0x%p)\n", nItemIndex, pPhotoAcquireItem);
        return E_NOTIMPL;
    }
    STDMETHODIMP UpdateDeletePercent(UINT nPercent)
    {
        OutputFormattedDebugString(L"UpdateDeletePercent(nPercent: %u)\n", nPercent);
        return E_NOTIMPL;
    }
    STDMETHODIMP EndItemDelete(UINT nItemIndex, IPhotoAcquireItem *pPhotoAcquireItem, HRESULT hr)
    {
        OutputFormattedDebugString(L"EndItemDelete(nItemIndex: %u, pPhotoAcquireItem: 0x%p, hr: 0x%08X)\n", nItemIndex, pPhotoAcquireItem, hr);
        return E_NOTIMPL;
    }
    STDMETHODIMP EndDelete(HRESULT hrDelete)
    {
        OutputFormattedDebugString(L"EndDelete(hr: 0x%08X)\n", hrDelete);
        return E_NOTIMPL;
    }
    STDMETHODIMP EndSession(HRESULT hrSession)
    {
        OutputFormattedDebugString(L"EndSession(hr: 0x%08X)\n", hrSession);
        return E_NOTIMPL;
    }
    STDMETHODIMP GetDeleteAfterAcquire(__out BOOL* pfDeleteAfterAcquire)
    {
        OutputFormattedDebugString(L"GetDeleteAfterAcquire(pfDeleteAfterAcquire: 0x%p)\n", pfDeleteAfterAcquire);
        return E_NOTIMPL;
    }
    STDMETHODIMP ErrorAdvise(HRESULT hrError, PCWSTR pszErrorMessage, ERROR_ADVISE_MESSAGE_TYPE nMessageType, __out ERROR_ADVISE_RESULT *pnErrorAdviseResult)
    {
        OutputFormattedDebugString(L"ErrorAdvise(hr: 0x%08X, pszErrorMessage: %ws, nMessageType: %d, pnErrorAdviseResult: %p)\n", hrError, pszErrorMessage, nMessageType, pnErrorAdviseResult);
        return E_NOTIMPL;
    }
    STDMETHODIMP GetUserInput(REFIID, IUnknown*, PROPVARIANT* pPropVar, const PROPVARIANT* pPropVarDefault)
    {
        if (g_fInfiniteLoop)
        {
            return PropVariantCopy(pPropVar, pPropVarDefault);
        }
        else
        {
            return E_NOTIMPL;
        }
    }

public:
    static HRESULT Create(REFIID riid, __out void** ppv)
    {
        *ppv = NULL;

        HRESULT hr;
        PhotoAcquireProgressCB* pPhotoAcquireProgressCB = new PhotoAcquireProgressCB;
        if (pPhotoAcquireProgressCB != NULL)
        {
             hr = pPhotoAcquireProgressCB->QueryInterface(riid, ppv);
             pPhotoAcquireProgressCB->Release();
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
        return hr;
    }
};


HRESULT MsgWaitForSingleHandle(HANDLE hHandle, DWORD dwMilliseconds)
{
    HRESULT hr = S_OK;

    BOOL fDone = FALSE;
    while (!fDone)
    {
        // pull any messages out of the queue and process them
        MSG msg;
        while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        // Wait for the handle to be signalled OR an input event
        DWORD dwResult = MsgWaitForMultipleObjects(1, &hHandle, FALSE, dwMilliseconds, QS_ALLINPUT);
        switch (dwResult)
        {
        case WAIT_OBJECT_0:
            fDone = TRUE;
            break;

        case WAIT_OBJECT_0+1:
            // Process messages
            break;

        case WAIT_ABANDONED:
            // This handle is an abandoned mutex.  Exit with a failure.
            hr = HRESULT_FROM_WIN32(ERROR_ABANDONED_WAIT_0);
            fDone = TRUE;
            break;

        case WAIT_TIMEOUT:
            // The wait time expired
            hr = HRESULT_FROM_WIN32(ERROR_TIMEOUT);
            fDone = TRUE;
            break;

        case WAIT_FAILED:
        default:
            // Something bad happened
            hr = E_UNEXPECTED;
            fDone = TRUE;
            break;
        }
    }
    return hr;
}

class AcquireThreadData
{
private:
    HWND   m_hWndParent;
    BOOL   m_fShowProgressUi;
    PWSTR  m_pszCustomTemplate;

private:
    // No implementation
    AcquireThreadData(const AcquireThreadData&);
    AcquireThreadData& operator=(const AcquireThreadData&);

public:
    AcquireThreadData()
        : m_hWndParent(NULL)
        , m_fShowProgressUi(TRUE)
        , m_pszCustomTemplate(NULL)
    {
    }
    ~AcquireThreadData()
    {
        if (m_pszCustomTemplate != NULL)
        {
            delete[] m_pszCustomTemplate;
            m_pszCustomTemplate = NULL;
        }
    }

    void SetParentWindow(HWND hWndParent)
    {
        m_hWndParent = hWndParent;
    }
    HWND GetParentWindow() const
    {
        return m_hWndParent;
    }

    void SetShowProgressUi(BOOL fShowProgressUi)
    {
        m_fShowProgressUi = fShowProgressUi;
    }
    BOOL GetShowProgressUi() const
    {
        return m_fShowProgressUi;
    }

    HRESULT SetCustomTemplate(PCWSTR pszCustomTemplate)
    {
        HRESULT hr = S_OK;
        if (m_pszCustomTemplate != NULL)
        {
            delete[] m_pszCustomTemplate;
            m_pszCustomTemplate = NULL;
        }
        if (pszCustomTemplate != NULL)
        {
            size_t cchLength = lstrlen(pszCustomTemplate);
            m_pszCustomTemplate = new WCHAR[cchLength + 1];
            if (m_pszCustomTemplate != NULL)
            {
                hr = StringCchCopy(m_pszCustomTemplate, cchLength + 1, pszCustomTemplate);
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
        return hr;
    }
    PCWSTR GetCustomTemplate() const
    {
        return m_pszCustomTemplate;
    }
};

HRESULT SelectDevice(HWND hWndParent, __out BSTR* pstrSelectedDeviceId)
{
    *pstrSelectedDeviceId = L"";

    // Create the devuce selection dialog
    IPhotoAcquireDeviceSelectionDialog* pPhotoAcquireDeviceSelectionDialog = NULL;
    HRESULT hr = CoCreateInstance(CLSID_PhotoAcquireDeviceSelectionDialog, NULL, CLSCTX_INPROC_SERVER, IID_IPhotoAcquireDeviceSelectionDialog, (void**)&pPhotoAcquireDeviceSelectionDialog);
    if (SUCCEEDED(hr))
    {
        // Load the prompt string
        WCHAR szSubmitButtonText[MAX_PATH] = {0};
        LoadString(g_hInstance, IDS_SELECTION_SUBMIT_TEXT, szSubmitButtonText, ARRAYSIZE(szSubmitButtonText));

        // Load the caption string
        WCHAR szCaption[MAX_PATH] = {0};
        LoadString(g_hInstance, IDS_SELECTION_CAPTION, szCaption, ARRAYSIZE(szCaption));

        // Set the prompt
        hr = pPhotoAcquireDeviceSelectionDialog->SetSubmitButtonText(szSubmitButtonText);
        if (SUCCEEDED(hr))
        {
            // Set the caption
            hr = pPhotoAcquireDeviceSelectionDialog->SetTitle(szCaption);
            if (SUCCEEDED(hr))
            {
                // Select a device
                DEVICE_SELECTION_DEVICE_TYPE nType;
                hr = pPhotoAcquireDeviceSelectionDialog->DoModal(hWndParent, DSF_WPD_DEVICES|DSF_FS_DEVICES, pstrSelectedDeviceId, &nType);
            }
        }
        pPhotoAcquireDeviceSelectionDialog->Release();
    }
    return hr;
}

HRESULT EnumerateTransferredFilesFromEnum(IPhotoAcquire* pPhotoAcquire)
{
    OutputFormattedDebugString(L"***********************************************************\n"
                               L"Begin enumeration of files using IPhotoAcquire::EnumResults\n");
    IEnumString* pEnumString = NULL;
    HRESULT hr = pPhotoAcquire->EnumResults(&pEnumString);
    if (SUCCEEDED(hr))
    {
        for (int nCurrentFile = 1; ; ++nCurrentFile)
        {
            PWSTR pszCurrent = NULL;
            if (S_OK == pEnumString->Next(1, &pszCurrent, NULL))
            {
                OutputFormattedDebugString(L"%8d. %s\n", nCurrentFile, pszCurrent);
                CoTaskMemFree(pszCurrent);
            }
            else
            {
                break;
            }
        }
        pEnumString->Release();
    }
    OutputFormattedDebugString(L"End enumeration of files using IPhotoAcquire::EnumResults\n"
                               L"***********************************************************\n\n");
    return hr;
}

HRESULT EnumerateTransferredFilesFromItems(IPhotoAcquireSource* pPhotoAcquireSource)
{
    OutputFormattedDebugString(L"***********************************************************\n"
                               L"Begin enumeration of files using PKEY_PhotoAcquire_FinalFilename\n");
    UINT nItemCount = 0;
    HRESULT hr = pPhotoAcquireSource->GetItemCount(&nItemCount);
    for (UINT nCurr = 0; nCurr < nItemCount && SUCCEEDED(hr); ++nCurr)
    {
        IPhotoAcquireItem* pPhotoAcquireItem = NULL;
        hr = pPhotoAcquireSource->GetItemAt(nCurr, &pPhotoAcquireItem);
        if (SUCCEEDED(hr))
        {
            PROPVARIANT pv = {0};
            hr = pPhotoAcquireItem->GetProperty(PKEY_PhotoAcquire_FinalFilename, &pv);
            if (SUCCEEDED(hr))
            {
                if (g_fDeleteFilesWhenDone)
                {
                    DeleteFileW(pv.pwszVal);
                }
                OutputFormattedDebugString(L"%8d. %s\n", nCurr + 1, pv.pwszVal);
                PropVariantClear(&pv);
            }
            UINT nSubItemCount = 0;
            hr = pPhotoAcquireItem->GetSubItemCount(&nSubItemCount);
            if (SUCCEEDED(hr))
            {
                for (UINT nCurrentSubItem = 0; nCurrentSubItem < nSubItemCount && SUCCEEDED(hr); ++nCurrentSubItem)
                {
                    IPhotoAcquireItem* pPhotoAcquireSubItem = NULL;
                    hr = pPhotoAcquireItem->GetSubItemAt(nCurrentSubItem, &pPhotoAcquireSubItem);
                    if (SUCCEEDED(hr))
                    {
                        hr = pPhotoAcquireSubItem->GetProperty(PKEY_PhotoAcquire_FinalFilename, &pv);
                        if (SUCCEEDED(hr))
                        {
                            if (g_fDeleteFilesWhenDone)
                            {
                                DeleteFileW(pv.pwszVal);
                            }
                            OutputFormattedDebugString(L"        %8d. %s\n", nCurrentSubItem + 1, pv.pwszVal);
                            PropVariantClear(&pv);
                        }
                        pPhotoAcquireSubItem->Release();
                    }
                }
            }
            pPhotoAcquireItem->Release();
        }
    }
    OutputFormattedDebugString(L"End enumeration of files using PKEY_PhotoAcquire_FinalFilename\n"
                               L"***********************************************************\n\n");
    return hr;
}


HRESULT TransferPhotosAndVideos(HWND hWndParent, BOOL fShowProgressUi, BOOL fUseCustomTemplate, PCWSTR pszCustomTemplate, PCWSTR pszDeviceId)
{
    IPhotoAcquireProgressCB* pPhotoAcquireProgressCB = NULL;
    HRESULT hr = PhotoAcquireProgressCB::Create(IID_PPV_ARGS(&pPhotoAcquireProgressCB));
    if (SUCCEEDED(hr))
    {
        IPhotoAcquire* pPhotoAcquire = NULL;
        hr = CoCreateInstance(CLSID_PhotoAcquire, NULL, CLSCTX_INPROC, IID_PPV_ARGS(&pPhotoAcquire));
        if (SUCCEEDED(hr))
        {
            IPhotoAcquireSource* pPhotoAcquireSource = NULL;
            hr = pPhotoAcquire->CreatePhotoSource(pszDeviceId, &pPhotoAcquireSource);
            if (SUCCEEDED(hr))
            {
                if (fUseCustomTemplate)
                {
                    IPhotoAcquireSettings* pPhotoAcquireSettings = NULL;
                    hr = pPhotoAcquireSource->GetPhotoAcquireSettings(&pPhotoAcquireSettings);
                    if (SUCCEEDED(hr))
                    {
                        hr = pPhotoAcquireSettings->SetOutputFilenameTemplate(pszCustomTemplate);
                        pPhotoAcquireSettings->Release();
                    }
                }
                if (SUCCEEDED(hr))
                {
                    hr = pPhotoAcquire->Acquire(pPhotoAcquireSource, fShowProgressUi, hWndParent, L"Test Application", pPhotoAcquireProgressCB);
                    if (SUCCEEDED(hr))
                    {
                        hr = EnumerateTransferredFilesFromEnum(pPhotoAcquire);
                        if (SUCCEEDED(hr))
                        {
                            hr = EnumerateTransferredFilesFromItems(pPhotoAcquireSource);
                        }
                    }
                }
                pPhotoAcquireSource->Release();
            }
            pPhotoAcquire->Release();
        }
        pPhotoAcquireProgressCB->Release();
    }

    return hr;
}


DWORD CALLBACK AcquireThread(__in PVOID pVoid)
{
    AcquireThreadData* pAcquireThreadData = reinterpret_cast<AcquireThreadData*>(pVoid);
    if (pAcquireThreadData != NULL)
    {
        HRESULT hr = CoInitialize(NULL);
        if (SUCCEEDED(hr))
        {
            BSTR bstrSelectedDeviceId = NULL;
            hr = SelectDevice(pAcquireThreadData->GetParentWindow(), &bstrSelectedDeviceId);
            if (SUCCEEDED(hr))
            {
                // Launch acquisition
                hr = TransferPhotosAndVideos(pAcquireThreadData->GetParentWindow(), pAcquireThreadData->GetShowProgressUi(), pAcquireThreadData->GetCustomTemplate() != NULL, pAcquireThreadData->GetCustomTemplate(), bstrSelectedDeviceId);

                SysFreeString(bstrSelectedDeviceId);
            }
            CoUninitialize();
        }

        delete pAcquireThreadData;
    }
    return 0;
}

int PhotoAcquireTest_GetCheckedRadioButton(HWND hWnd)
{
    int nResult = -1;

    static const int s_Controls[] =
    {
        IDC_ACQUISITION_RADIO,
        IDC_DEVSEL_RADIO,
        IDC_SETTINGS_RADIO
    };
    for (int i = 0; i < _ARRAYSIZE(s_Controls); ++i)
    {
        if (IsDlgButtonChecked(hWnd, s_Controls[i]) == BST_CHECKED)
        {
            nResult = s_Controls[i];
            break;
        }
    }
    return nResult;
}

void PhotoAcquireTest_LaunchDeviceSelectionDialog(HWND hWnd)
{
    DWORD dwDeviceTypes = 0;
    static const struct
    {
        int nControlId;
        DWORD dwDeviceType;
    } s_DeviceTypes[] =
    {
        {IDC_DEVSEL_FS_DEVICES,           DSF_FS_DEVICES},
        {IDC_DEVSEL_WPD_DEVICES,          DSF_WPD_DEVICES},
        {IDC_DEVSEL_WIA_CAMERAS,          DSF_WIA_CAMERAS},
        {IDC_DEVSEL_WIA_SCANNERS,         DSF_WIA_SCANNERS},
        {IDC_DEVSEL_STI_DEVICES,          DSF_STI_DEVICES},
        {IDC_DEVSEL_DV_DEVICES,           DSF_DV_DEVICES},
        {IDC_DEVSEL_SHOW_OFFLINE,         DSF_SHOW_OFFLINE},
        {IDC_DEVSEL_CPANEL_MODE,          DSF_CPL_MODE}
    };
    for (int i = 0; i < _ARRAYSIZE(s_DeviceTypes); ++i)
    {
        if (IsDlgButtonChecked(hWnd, s_DeviceTypes[i].nControlId) == BST_CHECKED)
        {
            dwDeviceTypes |= s_DeviceTypes[i].dwDeviceType;
        }
    }

    // Create the device selection dialog
    IPhotoAcquireDeviceSelectionDialog* pPhotoAcquireDeviceSelectionDialog = NULL;
    HRESULT hr = CoCreateInstance(CLSID_PhotoAcquireDeviceSelectionDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pPhotoAcquireDeviceSelectionDialog));
    if (SUCCEEDED(hr))
    {
        // Load the caption string
        WCHAR szCaption[MAX_PATH] = {0};
        LoadString(g_hInstance, IDS_SELECTION_CAPTION, szCaption, ARRAYSIZE(szCaption));

        // Load the submit button text string
        WCHAR szSubmitButtonText[MAX_PATH] = {0};
        LoadString(g_hInstance, IDS_SELECTION_SUBMIT_TEXT, szSubmitButtonText, ARRAYSIZE(szSubmitButtonText));

        // Set the title
        hr = pPhotoAcquireDeviceSelectionDialog->SetTitle(szCaption);
        if (SUCCEEDED(hr))
        {
            // Set the title
            hr = pPhotoAcquireDeviceSelectionDialog->SetSubmitButtonText(szSubmitButtonText);
            if (SUCCEEDED(hr))
            {
                // Select a device
                if ((dwDeviceTypes & DSF_CPL_MODE) == 0)
                {
                    BSTR bstrDeviceId = NULL;
                    DEVICE_SELECTION_DEVICE_TYPE nDeviceSelectionType;
                    hr = pPhotoAcquireDeviceSelectionDialog->DoModal(hWnd, dwDeviceTypes, &bstrDeviceId, &nDeviceSelectionType);
                    if (S_OK == hr)
                    {
                        TCHAR szMessage[MAX_PATH] = TEXT("");
                        StringCchPrintf(szMessage, ARRAYSIZE(szMessage), TEXT("Device ID: %ws, Device Type: %d"), bstrDeviceId, nDeviceSelectionType);
                        MessageBox(hWnd, szMessage, TEXT("Device Selection Result"), IDOK|MB_ICONINFORMATION);
                        SysFreeString(bstrDeviceId);
                    }
                }
                else
                {
                    pPhotoAcquireDeviceSelectionDialog->DoModal(hWnd, dwDeviceTypes, NULL, NULL);
                }
            }
        }
        pPhotoAcquireDeviceSelectionDialog->Release();
    }
}


void PhotoAcquireTest_LaunchAcquisition(HWND hWnd)
{
    // Create the thread data object
    AcquireThreadData* pAcquireThreadData = new AcquireThreadData;
    if (pAcquireThreadData != NULL)
    {
        // Set the parent window unless it was suppressed
        if (IsDlgButtonChecked(hWnd, IDC_ACQUIRE_USE_NULL_HWND) != BST_CHECKED)
        {
            pAcquireThreadData->SetParentWindow(hWnd);
        }

        // Suppress the progress UI if requested
        if (IsDlgButtonChecked(hWnd, IDC_ACQUIRE_SUPPRESS_PROGRESS) == BST_CHECKED)
        {
            pAcquireThreadData->SetShowProgressUi(FALSE);
        }

        // Set the custom naming template if it was supplied
        if (IsDlgButtonChecked(hWnd, IDC_ACQUIRE_USE_CUSTOM_TEMPLATE) == BST_CHECKED)
        {
            int nTextLength = GetWindowTextLength(GetDlgItem(hWnd, IDC_ACQUIRE_CUSTOM_TEMPLATE));
            if (nTextLength != 0)
            {
                PWSTR pszText = new WCHAR[nTextLength + 1];
                if (pszText != NULL)
                {
                    GetDlgItemText(hWnd, IDC_ACQUIRE_CUSTOM_TEMPLATE, pszText, nTextLength + 1);
                    pAcquireThreadData->SetCustomTemplate(pszText);
                    delete[] pszText;
                }
            }
        }

        // Create the worker thread
        DWORD dwThreadId;
        HANDLE hThread = CreateThread(NULL, 0, AcquireThread, pAcquireThreadData, 0, &dwThreadId);
        if (hThread != NULL)
        {
            // Wait for it to finish
            MsgWaitForSingleHandle(hThread, INFINITE);
            CloseHandle(hThread);
        }
        else
        {
            // If we couldn't create the thread, delete the thread data
            delete pAcquireThreadData;
        }
    }
}

void PhotoAcquireTest_LaunchSettings(HWND hWnd)
{
    // Create the settings dialog object
    IPhotoAcquireOptionsDialog* pDialog = NULL;
    HRESULT hr = CoCreateInstance(CLSID_PhotoAcquireOptionsDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pDialog));
    if (SUCCEEDED(hr))
    {
        // Initialize it
        hr = pDialog->Initialize(NULL);
        if (SUCCEEDED(hr))
        {
            // Run the dialog modally
            INT_PTR nResult = 0;
            hr = pDialog->DoModal(hWnd, &nResult);
            if (SUCCEEDED(hr))
            {
                // If the user clicked OK, then save the updated settings
                if (nResult == IDOK)
                {
                    pDialog->SaveData();
                }
            }
        }
        pDialog->Release();
    }
}


void PhotoAcquireTest_UpdateControlState(HWND hWnd)
{
    static const struct
    {
        int nControlId;
        int nRadioButton;
    } s_SubControls[] =
    {
        {IDC_ACQUIRE_USE_NULL_HWND,       IDC_ACQUISITION_RADIO},
        {IDC_ACQUIRE_SUPPRESS_PROGRESS,   IDC_ACQUISITION_RADIO},
        {IDC_ACQUIRE_USE_CUSTOM_TEMPLATE, IDC_ACQUISITION_RADIO},
        {IDC_DEVSEL_FS_DEVICES,           IDC_DEVSEL_RADIO},
        {IDC_DEVSEL_WPD_DEVICES,          IDC_DEVSEL_RADIO},
        {IDC_DEVSEL_WIA_CAMERAS,          IDC_DEVSEL_RADIO},
        {IDC_DEVSEL_WIA_SCANNERS,         IDC_DEVSEL_RADIO},
        {IDC_DEVSEL_STI_DEVICES,          IDC_DEVSEL_RADIO},
        {IDC_DEVSEL_DV_DEVICES,           IDC_DEVSEL_RADIO},
        {IDC_DEVSEL_SHOW_OFFLINE,         IDC_DEVSEL_RADIO},
        {IDC_DEVSEL_CPANEL_MODE,          IDC_DEVSEL_RADIO}
    };

    int nCurrentRadioButton = PhotoAcquireTest_GetCheckedRadioButton(hWnd);
    for (int i = 0; i < _ARRAYSIZE(s_SubControls); ++i)
    {
        EnableWindow(GetDlgItem(hWnd, s_SubControls[i].nControlId), s_SubControls[i].nRadioButton == nCurrentRadioButton);
    }

    // Enable the custom template edit control if the currently selected radio button is IDC_ACQUISITION_RADIO, and IDC_ACQUIRE_USE_CUSTOM_TEMPLATE is checked
    EnableWindow(GetDlgItem(hWnd, IDC_ACQUIRE_CUSTOM_TEMPLATE), (nCurrentRadioButton == IDC_ACQUISITION_RADIO) && (IsDlgButtonChecked(hWnd, IDC_ACQUIRE_USE_CUSTOM_TEMPLATE) == BST_CHECKED));
}


LRESULT PhotoAcquireTest_InitDialog(HWND hWnd)
{
    Button_SetCheck(GetDlgItem(hWnd, IDC_ACQUISITION_RADIO), BST_CHECKED);

    // Set the device types in the device selection dialog
    Button_SetCheck(GetDlgItem(hWnd, IDC_DEVSEL_FS_DEVICES), BST_CHECKED);
    Button_SetCheck(GetDlgItem(hWnd, IDC_DEVSEL_WPD_DEVICES), BST_CHECKED);
    Button_SetCheck(GetDlgItem(hWnd, IDC_DEVSEL_WIA_CAMERAS), BST_CHECKED);
    Button_SetCheck(GetDlgItem(hWnd, IDC_DEVSEL_WIA_SCANNERS), BST_CHECKED);
    Button_SetCheck(GetDlgItem(hWnd, IDC_DEVSEL_SHOW_OFFLINE), BST_CHECKED);
    Button_SetCheck(GetDlgItem(hWnd, IDC_DEVSEL_STI_DEVICES), BST_CHECKED);
    Button_SetCheck(GetDlgItem(hWnd, IDC_DEVSEL_DV_DEVICES), BST_CHECKED);
    Button_SetCheck(GetDlgItem(hWnd, IDC_DEVSEL_CPANEL_MODE), BST_UNCHECKED);

    PhotoAcquireTest_UpdateControlState(hWnd);

    return TRUE;
}

//**************************************************************
// WM_COMMAND handlers
//**************************************************************

void PhotoAcquireTest_OnOK(HWND hWnd)
{
    if (Button_GetCheck(GetDlgItem(hWnd, IDC_ACQUISITION_RADIO)) == BST_CHECKED)
    {
        PhotoAcquireTest_LaunchAcquisition(hWnd);
    }
    else if (Button_GetCheck(GetDlgItem(hWnd, IDC_DEVSEL_RADIO)) == BST_CHECKED)
    {
        PhotoAcquireTest_LaunchDeviceSelectionDialog(hWnd);
    }
    else if (Button_GetCheck(GetDlgItem(hWnd, IDC_SETTINGS_RADIO)) == BST_CHECKED)
    {
        PhotoAcquireTest_LaunchSettings(hWnd);
    }
}

void PhotoAcquireTest_OnCancel(HWND hWnd)
{
    EndDialog(hWnd, 0);
}

void PhotoAcquireTest_OnRadioButton(HWND hWnd)
{
    PhotoAcquireTest_UpdateControlState(hWnd);
}

void PhotoAcquireTest_OnCustomTemplate(HWND hWnd)
{
    PhotoAcquireTest_UpdateControlState(hWnd);
}

//**************************************************************
// Internal helpers
//**************************************************************


INT_PTR CALLBACK PhotoAcquireTestDialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    INT_PTR fResult = FALSE;
    switch (uMsg)
    {
    case WM_INITDIALOG:
        {
            fResult = PhotoAcquireTest_InitDialog(hWnd);
        }
        break;

    case WM_COMMAND:
        {
            switch (LOWORD(wParam))
            {
            case IDOK:
                PhotoAcquireTest_OnOK(hWnd);
                break;

            case IDCANCEL:
                PhotoAcquireTest_OnCancel(hWnd);
                break;

            case IDC_ACQUISITION_RADIO:
                PhotoAcquireTest_OnRadioButton(hWnd);
                break;

            case IDC_DEVSEL_RADIO:
                PhotoAcquireTest_OnRadioButton(hWnd);
                break;

            case IDC_SETTINGS_RADIO:
                PhotoAcquireTest_OnRadioButton(hWnd);
                break;

            case IDC_ACQUIRE_USE_CUSTOM_TEMPLATE:
                PhotoAcquireTest_OnCustomTemplate(hWnd);
                break;
            }
        }
        break;
    }
    return fResult;
}


int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE, __in __nullterminated wchar_t*, int)
{
    g_hInstance = hInstance;

    HRESULT hr = CoInitialize(NULL);
    if (SUCCEEDED(hr))
    {
        bool fAcquireWithoutTestDialog = false;
        WCHAR szDeviceId[MAX_PATH] = {0};

        // Convert to an array of arguments
        int nArgCount;
        PWSTR *ppszArgs = CommandLineToArgvW(GetCommandLineW(), &nArgCount);
        if (ppszArgs != NULL)
        {
            // Loop through all of the arguments
            int nCurrentArg = 1;
            while (nCurrentArg<nArgCount)
            {
                // Is this a switch?
                if (lstrlen(ppszArgs[nCurrentArg]) >= 2 && (L'/'==ppszArgs[nCurrentArg][0] || L'-'==ppszArgs[nCurrentArg][0]))
                {
                    switch (towupper(ppszArgs[nCurrentArg][1]))
                    {
                    case L'A':
                        fAcquireWithoutTestDialog = true;
                        break;

                    case L'S':
                        g_fSuppressErrorMessageDialog = true;
                        break;

                    case L'I':
                        g_fInfiniteLoop = true;
                        g_fDeleteFilesWhenDone = true;
                        fAcquireWithoutTestDialog = true;
                        break;

                    case L'D':
                        g_fDeleteFilesWhenDone = true;
                        break;
                    }
                }

                // Don't we have a device yet?  Save this as a device ID.
                else if (lstrlen(szDeviceId) == 0)
                {
                    StringCchCopy(szDeviceId, ARRAYSIZE(szDeviceId), ppszArgs[nCurrentArg]);
                }

                ++nCurrentArg;
            }

            LocalFree(ppszArgs);
        }

        if (fAcquireWithoutTestDialog)
        {
            // If we don't have a device, get one
            if (lstrlen(szDeviceId) == 0)
            {
                // Select the device
                BSTR bstrSelectedDeviceId = NULL;
                hr = SelectDevice(NULL, &bstrSelectedDeviceId);
                if (SUCCEEDED(hr))
                {
                    // Save the device ID
                    hr = StringCchCopy(szDeviceId, ARRAYSIZE(szDeviceId), bstrSelectedDeviceId);

                    SysFreeString(bstrSelectedDeviceId);
                }
            }

            if (SUCCEEDED(hr))
            {
                do
                {
                    // Launch acquisition
                    hr = TransferPhotosAndVideos(g_fInfiniteLoop ? GetDesktopWindow() : NULL, TRUE, FALSE, NULL, szDeviceId);
                }
                while (g_fInfiniteLoop && hr != E_ABORT);
            }
        }
        else
        {
            // Initialize the common controls library
            INITCOMMONCONTROLSEX icex = {0};
            icex.dwSize = sizeof(icex);
            icex.dwICC = ICC_LINK_CLASS|ICC_STANDARD_CLASSES;
            InitCommonControlsEx(&icex);

            DialogBox(hInstance, MAKEINTRESOURCE(IDD_TEST_DIALOG), NULL, PhotoAcquireTestDialogProc);
        }
        CoUninitialize();
    }
    return 0;
}

