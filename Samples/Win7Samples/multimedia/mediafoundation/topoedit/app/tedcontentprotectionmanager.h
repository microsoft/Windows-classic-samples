// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

#include "tedobj.h"
#include "mfidl.h"
#include "mediaobj.h"
#include "shellapi.h"
#include "wmsdk.h"

// {E2102B14-0B1B-413b-94FB-47C73E2EFC18}
DEFINE_GUID(&CLSID_CTedContentProtectionManager, 0xe2102b14, 0xb1b, 0x413b, 0x94, 0xfb, 0x47, 0xc7, 0x3e, 0x2e, 0xfc, 0x18);

class CTedContentProtectionManager 
    : public IMFContentProtectionManager
    , public IWMReaderCallback
{
public:
    CTedContentProtectionManager::CTedContentProtectionManager(CTedApp* pApp) 
        : m_pApp(pApp)
        , m_cRef(0)
    {
    }

    METHODASYNCCALLBACK(OnEnableEvent, CTedContentProtectionManager);

    HRESULT STDMETHODCALLTYPE BeginEnableContent(IMFActivate* pEnablerActivate, IMFTopology* pTopo, IMFAsyncCallback* pCallback, IUnknown* punkState)
    {
        HRESULT hr = S_OK;
        CComPtr<IMFMediaEventGenerator> spMEG;
        
        IFC( MFCreateAsyncResult(NULL, pCallback, punkState, &m_spResult) );
        
        IFC( pEnablerActivate->ActivateObject(IID_IMFContentEnabler, (void**) &m_spContentEnabler) );

        GUID gidEnableType;
        IFC( m_spContentEnabler->GetEnableType(&gidEnableType) );

        if(MFENABLETYPE_WMDRMV1_LicenseAcquisition == gidEnableType || MFENABLETYPE_WMDRMV7_LicenseAcquisition == gidEnableType)
        {
            BOOL isAutomaticSupported = FALSE;
            IFC( m_spContentEnabler->IsAutomaticSupported(&isAutomaticSupported) );

            if(!isAutomaticSupported)
            {
                m_pApp->PostMessage(WM_MF_HANDLE_PROTECTED_CONTENT, 0, 0);
            }
            else
            {
        	    IFC( m_spContentEnabler->AutomaticEnable() );
            }
        }
        else if(MFENABLETYPE_WMDRMV7_Individualization == gidEnableType)
        {
            m_pApp->PostMessage(WM_MF_HANDLE_INDIVIDUALIZATION, 0, 0);
        }
        else if(MFENABLETYPE_MF_UpdateUntrustedComponent == gidEnableType || MFENABLETYPE_MF_UpdateRevocationInformation == gidEnableType)
        {
            m_pApp->PostMessage(WM_MF_HANDLE_UNTRUSTED_COMPONENT, 0, 0);
        }

        IFC( m_spContentEnabler->QueryInterface(IID_IMFMediaEventGenerator, (void**) &spMEG) );
        IFC( spMEG->BeginGetEvent( &m_xOnEnableEvent, NULL) );
        
    Cleanup:
        return hr;
    }

    HRESULT STDMETHODCALLTYPE EndEnableContent(IMFAsyncResult* pResult)
    {
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE ManualEnableContent()
    {
        HRESULT hr = S_OK;
        DWORD dwURLLength;
        LPWSTR wszURL;
        MF_URL_TRUST_STATUS TrustStatus;

        IFC( m_spContentEnabler->GetEnableURL(&wszURL, &dwURLLength, &TrustStatus) );

        int iResult = IDYES;
        if(MF_LICENSE_URL_UNTRUSTED == TrustStatus)
        {
            iResult = m_pApp->MessageBox(LoadAtlString(IDS_LICENSE_URL_UNTRUSTED), NULL, MB_YESNO);    
        }
        else if(MF_LICENSE_URL_TAMPERED == TrustStatus)
        {
            iResult = m_pApp->MessageBox(LoadAtlString(IDS_LICENSE_URL_TAMPERED), NULL, MB_YESNO);
        }

        if(iResult == IDYES)
        {
            HINSTANCE hResult = ShellExecute(NULL, L"open", wszURL, NULL, NULL, SW_NORMAL);
            
            if(hResult < (HINSTANCE) 32)
            {
                m_pApp->MessageBox(LoadAtlString(IDS_E_LICENSE_URL_BROWSER), NULL, MB_OK);
            }
        }

    
        Cleanup:
            return hr;
    }

    HRESULT STDMETHODCALLTYPE Individualize()
    {
        HRESULT hr = S_OK;

        CComPtr<IWMReader> spReader;
        CComPtr<IWMDRMReader> spDRMReader;
            
        IFC( WMCreateReader(NULL, 0, &spReader) );

        IFC( spReader->Open(L"C:\\non_existent_file.just_for_indiv", (IWMReaderCallback *)this, NULL) );
        IFC( spReader->QueryInterface(&spDRMReader) );
        IFC( spDRMReader->Individualize(0) );

    Cleanup:
        return hr;
    }

    HRESULT STDMETHODCALLTYPE OnSample(DWORD  dwOutputNum, QWORD  cnsSampleTime, QWORD  cnsSampleDuration, DWORD  dwFlags, INSSBuffer*  pSample, void*  pvContext)
    {
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE OnStatus(WMT_STATUS,HRESULT,WMT_ATTR_DATATYPE,BYTE *,void *)
    {
        return S_OK;
    }
    
    virtual STDMETHODIMP_(ULONG) AddRef()
    {
        LONG cRef = InterlockedIncrement(&m_cRef);

        return cRef;
    }
    
    virtual STDMETHODIMP_(ULONG) Release()
    {
        LONG cRef = InterlockedDecrement(&m_cRef);

        if(0 == cRef)
        {
            delete this;
        }

        return cRef;
    }
    
    virtual STDMETHODIMP QueryInterface(REFIID riid, void** ppvObject)
    {
        if(!ppvObject)
        {
        	return E_POINTER;
        }

        if(IID_IMFContentProtectionManager == riid || IID_IUnknown == riid)
        {
        	*ppvObject = this;
        	AddRef();

        	return S_OK;
        }

        *ppvObject = NULL;
        return E_NOINTERFACE;
    }

    void OnEnableEvent(IMFAsyncResult* pResult)
    {
        HRESULT hr = S_OK;
        
        CComPtr<IMFMediaEventGenerator> spMEG;
        CComPtr<IMFMediaEvent> spEvent;
                
        IFC( m_spContentEnabler->QueryInterface(IID_IMFMediaEventGenerator, (void**) &spMEG) );
        IFC( spMEG->EndGetEvent(pResult, &spEvent) );

        MediaEventType met;
        IFC( spEvent->GetType(&met) );

        HRESULT hrEvent;
        IFC( spEvent->GetStatus(&hrEvent) );

        if(MEEnablerCompleted == met)
        {
            if(SUCCEEDED(hrEvent))
            {
                m_pApp->MessageBox(LoadAtlString(IDS_LICENSE_ACQUIRED), NULL, MB_OK);
            }
            else
            {
                m_pApp->HandleMMError(LoadAtlString(IDS_LICENSE_ACQUIRED_FAILED), hr);
            }

           IFC( MFInvokeCallback(m_spResult) );
        }
        else
        {
            IFC( spMEG->BeginGetEvent(&m_xOnEnableEvent, NULL) );
        }

    Cleanup:
        ;
    }
    
private:
    CTedApp* m_pApp;
    LONG m_cRef;
    CComPtr<IMFContentEnabler> m_spContentEnabler;
    CComPtr<IMFAsyncResult> m_spResult;
};
