//////////////////////////////////////////////////////////////////////////
// ContentEnabler.cpp: Manages content enabler action.
// 
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//////////////////////////////////////////////////////////////////////////

#include "ProtectedPlayback.h"
#include "strsafe.h"


void LogEnableType(const GUID& guidEnableType);
void LogTrustStatus(MF_URL_TRUST_STATUS status);


///////////////////////////////////////////////////////////////////////
//  Name: CreateInstance
//  Description:  Static class method to create the object.
//  
//  hNotify:   Handle to the application window to receive notifications.
//  ppManager: Receives an AddRef's pointer to the ContentProtectionManager
//             object. The caller must release the pointer.
/////////////////////////////////////////////////////////////////////////

HRESULT ContentProtectionManager::CreateInstance(HWND hNotify, ContentProtectionManager **ppManager)
{
    if (hNotify == NULL || ppManager == NULL)
    {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;

    ContentProtectionManager *pManager = new ContentProtectionManager(hNotify);
    if (pManager == NULL)
    {
        hr = E_OUTOFMEMORY;
    }

    if (SUCCEEDED(hr))
    {
        *ppManager = pManager;
        (*ppManager)->AddRef();
    }

    return hr;
}

/////////////////////////////////////////////////////////////////////////
//  ContentProtectionManager constructor
/////////////////////////////////////////////////////////////////////////

ContentProtectionManager::ContentProtectionManager(HWND hNotify)
    : m_nRefCount(0), m_pMEG(NULL), m_pResult(NULL), m_pEnabler(NULL), m_hwnd(hNotify),
      m_state(Enabler_Ready), m_hrStatus(S_OK)
{
}

/////////////////////////////////////////////////////////////////////////
//  ContentProtectionManager destructor
/////////////////////////////////////////////////////////////////////////


ContentProtectionManager::~ContentProtectionManager()
{
    TRACE((L"~ContentEnabler\n"));
    SAFE_RELEASE(m_pMEG);
    SAFE_RELEASE(m_pResult);
    SAFE_RELEASE(m_pEnabler);
}


///////////////////////////////////////////////////////////////////////
//  AddRef
/////////////////////////////////////////////////////////////////////////

ULONG ContentProtectionManager::AddRef()
{
    return InterlockedIncrement(&m_nRefCount);
}


///////////////////////////////////////////////////////////////////////
//  Release
/////////////////////////////////////////////////////////////////////////

ULONG ContentProtectionManager::Release()
{
    ULONG uCount = InterlockedDecrement(&m_nRefCount);
    if (uCount == 0)
    {
        delete this;
    }
    // For thread safety, return a temporary variable.
    return uCount;
}



///////////////////////////////////////////////////////////////////////
//  QueryInterface
/////////////////////////////////////////////////////////////////////////

HRESULT ContentProtectionManager::QueryInterface(REFIID iid, void** ppv)
{
    if (!ppv)
    {
        return E_POINTER;
    }
    if (iid == IID_IUnknown)
    {
        *ppv = static_cast<IUnknown*>(static_cast<IMFAsyncCallback*>(this));
    }
    else if (iid == IID_IMFAsyncCallback)
    {
        *ppv = static_cast<IMFAsyncCallback*>(this);
    }
    else if (iid == IID_IMFContentProtectionManager)
    {
        *ppv = static_cast<IMFContentProtectionManager*>(this);
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }
    AddRef();
    return S_OK;
}


// IMFContentProtectionManager methods

///////////////////////////////////////////////////////////////////////
//  Name: BeginEnableContent
//  Description:  Called by the PMP session to start the enable action.
/////////////////////////////////////////////////////////////////////////

HRESULT ContentProtectionManager::BeginEnableContent(
    IMFActivate *pEnablerActivate,
    IMFTopology *pTopo,
    IMFAsyncCallback *pCallback,
    IUnknown *punkState
    )
{
    TRACE((L"ContentProtectionManager::BeginEnableContent"));

    HRESULT hr = S_OK;

    if (m_pEnabler != NULL)
    {
        TRACE((L"A previous call is still pending."));
        return E_FAIL;
    }

    // Create an async result for later use.
    hr = MFCreateAsyncResult(NULL, pCallback, punkState, &m_pResult);
    LOG_MSG_IF_FAILED(L"MFCreateAsyncResult", hr);

    // Create the enabler from the IMFActivate pointer.
    if (SUCCEEDED(hr))
    {
        hr = pEnablerActivate->ActivateObject(IID_IMFContentEnabler, (void**)&m_pEnabler);
        LOG_MSG_IF_FAILED(L"ActivateObject", hr);
    }

    // Notify the application. The application will call DoEnable from the app thread.
    if (SUCCEEDED(hr))
    {
        m_state = Enabler_Ready; // Reset the state.
        PostMessage(m_hwnd, WM_APP_CONTENT_ENABLER, 0, 0);
    }

    return hr;
}

///////////////////////////////////////////////////////////////////////
//  Name: EndEnableContent
//  Description:  Completes the enable action.
/////////////////////////////////////////////////////////////////////////

HRESULT ContentProtectionManager::EndEnableContent(IMFAsyncResult *pResult)
{
    TRACE((L"ContentProtectionManager::EndEnableContent"));


    if (pResult == NULL)
    {
        return E_POINTER;
    }

    // Release interfaces, so that we're ready to accept another call
    // to BeginEnableContent.
    SAFE_RELEASE(m_pResult);
    SAFE_RELEASE(m_pEnabler);
    SAFE_RELEASE(m_pMEG);

    return m_hrStatus;
}



///////////////////////////////////////////////////////////////////////
//  Name: Invoke
//  Description:  Callback for asynchronous BeginGetEvent method.
//  
//  pAsyncResult: Pointer to the result.
/////////////////////////////////////////////////////////////////////////

HRESULT ContentProtectionManager::Invoke(IMFAsyncResult *pAsyncResult)
{
    HRESULT hr = S_OK;
    IMFMediaEvent* pEvent = NULL;
    MediaEventType meType = MEUnknown;  // Event type
    PROPVARIANT varEventData;           // Event data

    PropVariantInit(&varEventData);

    // Get the event from the event queue.
    hr = m_pMEG->EndGetEvent(pAsyncResult, &pEvent);
    LOG_MSG_IF_FAILED(L"IMediaEventGenerator::EndGetEvent", hr);

    // Get the event type.
    if (SUCCEEDED(hr))
    {
        hr = pEvent->GetType(&meType);
    }

    // Get the event status. If the operation that triggered the event did
    // not succeed, the status is a failure code.
    if (SUCCEEDED(hr))
    {
        hr = pEvent->GetStatus(&m_hrStatus);
    }

    // Get the event data.
    if (SUCCEEDED(hr))
    {
        hr = pEvent->GetValue(&varEventData);
    }

    if (SUCCEEDED(hr))
    {
        // For the MEEnablerCompleted action, notify the application.
        // Otehrwise, request another event.
        TRACE((L"Content enabler event: %s", EventName(meType)));

        if (meType == MEEnablerCompleted)
        {
            PostMessage(m_hwnd, WM_APP_CONTENT_ENABLER, 0, 0);
        }
        else 
        {
            if (meType == MEEnablerProgress)
            {
                if (varEventData.vt == VT_LPWSTR)
                {
                    TRACE((L"Progress: %s", varEventData.pwszVal)); 
                }
            }
            m_pMEG->BeginGetEvent(this, NULL);
        }
    }
    

    // Clean up.
    PropVariantClear(&varEventData);
    SAFE_RELEASE(pEvent);

    return S_OK;
}


///////////////////////////////////////////////////////////////////////
//  Name: DoEnable
//  Description:  Does the enabler action.
//
//  flags: If ForceNonSilent, then always use non-silent enable.
//         Otherwise, use silent enable if possible.
////////////////////////////////////////////////////////////////////////

HRESULT ContentProtectionManager::DoEnable(EnablerFlags flags)
{
    TRACE((L"ContentProtectionManager::DoEnable (flags =%d)", flags));

    HRESULT             hr = S_OK;
    BOOL                bAutomatic = FALSE;
    GUID                guidEnableType;

    // Get the enable type. (Just for logging. We don't use it.)
    hr = m_pEnabler->GetEnableType(&guidEnableType);
    LOG_MSG_IF_FAILED(L"GetEnableType", hr);

    if (SUCCEEDED(hr))
    {
        LogEnableType(guidEnableType);
    }


    // Query for the IMFMediaEventGenerator interface so that we can get the
    // enabler events.
    if (SUCCEEDED(hr))
    {
        hr = m_pEnabler->QueryInterface(IID_IMFMediaEventGenerator, (void**)&m_pMEG);
    }

    // Ask for the first event.
    if (SUCCEEDED(hr))
    {
        hr = m_pMEG->BeginGetEvent(this, NULL);
    }

    // Decide whether to use silent or non-silent enabling. If flags is ForceNonSilent,
    // then we use non-silent. Otherwise, we query whether the enabler object supports 
    // silent enabling (also called "automatic" enabling).
    if (SUCCEEDED(hr))
    {
        if (flags == ForceNonSilent)
        {
            TRACE((L"Forcing non-silent enable."));
            bAutomatic = FALSE;
        }
        else
        {
            hr = m_pEnabler->IsAutomaticSupported(&bAutomatic);
            TRACE((L"IsAutomatic: auto = %d", bAutomatic));
        }
    }
    
    // Start automatic or non-silent, depending.
    if (SUCCEEDED(hr))
    {
        if (bAutomatic)
        {
            m_state = Enabler_SilentInProgress;
            TRACE((L"Content enabler: Automatic is supported"));
            hr = m_pEnabler->AutomaticEnable();
        }
        else
        {
            m_state = Enabler_NonSilentInProgress;
            TRACE((L"Content enabler: Using non-silent enabling"));
            hr = DoNonSilentEnable();
        }
    }

    if (FAILED(hr))
    {
        m_hrStatus = hr;
    }

    return hr;
}


///////////////////////////////////////////////////////////////////////
//  Name: CancelEnable
//  Description:  Cancels the current action.
//  
//  During silent enable, this cancels the enable action in progress.
//  During non-silent enable, this cancels the MonitorEnable thread.
/////////////////////////////////////////////////////////////////////////

HRESULT ContentProtectionManager::CancelEnable()
{
    HRESULT hr = S_OK;
    if (m_state != Enabler_Complete)
    {
        hr = m_pEnabler->Cancel();
        LOG_MSG_IF_FAILED(L"IMFContentEnabler::Cancel", hr);

        if (FAILED(hr))
        {
            // If Cancel fails for some reason, queue the MEEnablerCompleted
            // event ourselves. This will cause the current action to fail.
            m_pMEG->QueueEvent(MEEnablerCompleted, GUID_NULL, hr, NULL);
        }
    }
    return hr;
}

///////////////////////////////////////////////////////////////////////
//  Name: CompleteEnable
//  Description:  Completes the current action.
//  
//  This method invokes the PMP session's callback. 
/////////////////////////////////////////////////////////////////////////

HRESULT ContentProtectionManager::CompleteEnable()
{
    m_state = Enabler_Complete;

    // m_pResult can be NULL if the BeginEnableContent was not called.
    // This is the case when the application initiates the enable action, eg 
    // when MFCreatePMPMediaSession fails and returns an IMFActivate pointer.
    if (m_pResult)
    {
        TRACE((L"ContentProtectionManager: Invoking the pipeline's callback. (status = 0x%X)", m_hrStatus)); 
        m_pResult->SetStatus(m_hrStatus);
        MFInvokeCallback(m_pResult);
    }
    return S_OK;
}

///////////////////////////////////////////////////////////////////////
//  Name: DoNonSilentEnable
//  Description:  Performs non-silent enable.
/////////////////////////////////////////////////////////////////////////

HRESULT ContentProtectionManager::DoNonSilentEnable()
{

    // Trust status for the URL.
    MF_URL_TRUST_STATUS trustStatus = MF_LICENSE_URL_UNTRUSTED;

    WCHAR   *sURL = NULL;       // Enable URL
    DWORD   cchURL = 0;         // Size of enable URL in characters.

    BYTE    *pPostData = NULL;  // Buffer to hold HTTP POST data.
    DWORD   cbPostDataSize = 0; // Size of buffer, in bytes.

    HRESULT hr = S_OK;

    // Get the enable URL. This is where we get the enable data for non-silent enabling.
    hr = m_pEnabler->GetEnableURL(&sURL, &cchURL, &trustStatus);
    LOG_MSG_IF_FAILED(L"GetEnableURL", hr);

    if (SUCCEEDED(hr))
    {
        TRACE((L"Content enabler: URL = %s", sURL));
        LogTrustStatus(trustStatus);
    }

    if (trustStatus != MF_LICENSE_URL_TRUSTED)
    {
        TRACE((L"The enabler URL is not trusted. Failing.")); 
        hr = E_FAIL;
    }

    // Start the thread that monitors the non-silent enable action. 
    if (SUCCEEDED(hr))
    {
        hr = m_pEnabler->MonitorEnable();
    }

    // Get the HTTP POST data
    if (SUCCEEDED(hr))
    {
        hr = m_pEnabler->GetEnableData(&pPostData, &cbPostDataSize);
        LOG_MSG_IF_FAILED(L"GetEnableData", hr);
    }

    // Initialize the browser control.
    if (SUCCEEDED(hr))
    {
        hr = m_webHelper.Init((DispatchCallback*)this);
    }

    // Open the URL and send the HTTP POST data.
    if (SUCCEEDED(hr))
    {
        hr = m_webHelper.OpenURLWithData(sURL, pPostData, cbPostDataSize);
    }

    CoTaskMemFree(pPostData);
    CoTaskMemFree(sURL);

    return hr;
}


///////////////////////////////////////////////////////////////////////
//  Name: OnDispatchInvoke
//  Description:  Called when browser control sends an event.
//
//  dispIdMember: Dispatch ID from the DWebBrowserEvents2 interface.
/////////////////////////////////////////////////////////////////////////

void ContentProtectionManager::OnDispatchInvoke(DISPID  dispIdMember)
{
    if (dispIdMember == DISPID_ONQUIT)
    {
        // The user closed the browser window. Notify the application.
        TRACE((TEXT("DISPID_ONQUIT")));
        PostMessage(m_hwnd, WM_APP_BROWSER_DONE, 0, 0);
        m_webHelper.Exit();
    }
}





void LogEnableType(const GUID& guidEnableType)
{
    if (guidEnableType == MFENABLETYPE_WMDRMV1_LicenseAcquisition)
    {
        TRACE((L"MFENABLETYPE_WMDRMV1_LicenseAcquisition"));
    }
    else if (guidEnableType == MFENABLETYPE_WMDRMV7_LicenseAcquisition)
    {
        TRACE((L"MFENABLETYPE_WMDRMV7_LicenseAcquisition"));
    }
    else if (guidEnableType == MFENABLETYPE_WMDRMV7_Individualization)
    {
        TRACE((L"MFENABLETYPE_WMDRMV7_Individualization"));
    }
    else if (guidEnableType == MFENABLETYPE_MF_UpdateRevocationInformation)
    {
        TRACE((L"MFENABLETYPE_MF_UpdateRevocationInformation"));
    }
    else if (guidEnableType == MFENABLETYPE_MF_UpdateUntrustedComponent)
    {
        TRACE((L"MFENABLETYPE_MF_UpdateUntrustedComponent"));
    }
    else
    {
        TRACE((L"Unknown content enabler type."));
    }
}



void LogTrustStatus(MF_URL_TRUST_STATUS status)
{
    switch (status)
    {
    case MF_LICENSE_URL_UNTRUSTED:
        TRACE((L"MF_LICENSE_URL_UNTRUSTED"));
        break;

    case MF_LICENSE_URL_TRUSTED:
        TRACE((L"MF_LICENSE_URL_TRUSTED"));
        break;

    case MF_LICENSE_URL_TAMPERED:
        TRACE((L"MF_LICENSE_URL_TAMPERED"));
        break;
    }
}
