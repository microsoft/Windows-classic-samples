// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef CAPTURE_H
#define CAPTURE_H

#ifndef UNICODE
#define UNICODE
#endif 

#if !defined( NTDDI_VERSION )
#define NTDDI_VERSION NTDDI_WIN8
#endif

#if !defined( _WIN32_WINNT )
#define _WIN32_WINNT _WIN32_WINNT_WIN8
#endif

#include <new>
#include <windows.h>
#include <windowsx.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mferror.h>
#include <mfcaptureengine.h>
#include <shlwapi.h>
#include <strsafe.h>
#include <commctrl.h>
#include <d3d11.h>

const UINT WM_APP_CAPTURE_EVENT = WM_APP + 1;

HWND    CreatePreviewWindow(HINSTANCE hInstance, HWND hParent);
HWND    CreateMainWindow(HINSTANCE hInstance);
void    SetMenuItemText(HMENU hMenu, UINT uItem, _In_ PWSTR pszText);
void    ShowError(HWND hwnd, PCWSTR szMessage, HRESULT hr);
void    ShowError(HWND hwnd, UINT id, HRESULT hr);
HRESULT CloneVideoMediaType(IMFMediaType *pSrcMediaType, REFGUID guidSubType, IMFMediaType **ppNewMediaType);
HRESULT CreatePhotoMediaType(IMFMediaType *pSrcMediaType, IMFMediaType **ppPhotoMediaType);

// DXGI DevManager support
extern IMFDXGIDeviceManager* g_pDXGIMan;
extern ID3D11Device*         g_pDX11Device;
extern UINT                  g_ResetToken;

#ifdef _DEBUG
#define DBGMSG(x)  { DbgPrint x;}
#else
#define DBGMSG(x)
#endif

VOID DbgPrint(PCTSTR format, ...);


template <class T> void SafeRelease(T **ppT)
{
    if (*ppT)
    {
        (*ppT)->Release();
        *ppT = NULL;
    }
}

// Gets an interface pointer from a Media Foundation collection.
template <class IFACE>
HRESULT GetCollectionObject(IMFCollection *pCollection, DWORD index, IFACE **ppObject)
{
    IUnknown *pUnk;
    HRESULT hr = pCollection->GetElement(index, &pUnk);
    if (SUCCEEDED(hr))
    {
        hr = pUnk->QueryInterface(IID_PPV_ARGS(ppObject));
        pUnk->Release();
    }
    return hr;
}


struct ChooseDeviceParam
{
    ChooseDeviceParam() : ppDevices(NULL), count(0)
    {
    }
    ~ChooseDeviceParam()
    {
        for (DWORD i = 0; i < count; i++)
        {
            SafeRelease(&ppDevices[i]);
        }
        CoTaskMemFree(ppDevices);
    }

    IMFActivate **ppDevices;
    UINT32      count;
    UINT32      selection;
};



// CaptureManager class
// Wraps the capture engine and implements the event callback.

class CaptureManager
{
    // The event callback object.
    class CaptureEngineCB : public IMFCaptureEngineOnEventCallback
    {
        long m_cRef;
        HWND m_hwnd;

    public:
        CaptureEngineCB(HWND hwnd) : m_cRef(1), m_hwnd(hwnd), m_fSleeping(false), m_pManager(NULL) {}

        // IUnknown
        STDMETHODIMP QueryInterface(REFIID riid, void** ppv);
        STDMETHODIMP_(ULONG) AddRef();
        STDMETHODIMP_(ULONG) Release();
    
        // IMFCaptureEngineOnEventCallback
        STDMETHODIMP OnEvent( _In_ IMFMediaEvent* pEvent);

        bool m_fSleeping;
        CaptureManager* m_pManager;
    };

    HWND                    m_hwndEvent;
    HWND                    m_hwndPreview;

    IMFCaptureEngine        *m_pEngine;
    IMFCapturePreviewSink   *m_pPreview;

    CaptureEngineCB         *m_pCallback;

    bool                    m_bPreviewing;
    bool                    m_bRecording;
    bool                    m_bPhotoPending;

    UINT                    m_errorID;
    HANDLE                  m_hEvent;
    HANDLE                  m_hpwrRequest;
    bool                    m_fPowerRequestSet;

    CaptureManager(HWND hwnd) : 
        m_hwndEvent(hwnd), m_hwndPreview(NULL), m_pEngine(NULL), m_pPreview(NULL), 
        m_pCallback(NULL), m_bRecording(false), m_bPreviewing(false), m_bPhotoPending(false), m_errorID(0),m_hEvent(NULL)
        ,m_hpwrRequest(INVALID_HANDLE_VALUE)
        ,m_fPowerRequestSet(false)
    {
        REASON_CONTEXT  pwrCtxt;

        pwrCtxt.Version = POWER_REQUEST_CONTEXT_VERSION;
        pwrCtxt.Flags   = POWER_REQUEST_CONTEXT_SIMPLE_STRING;
        pwrCtxt.Reason.SimpleReasonString = L"CaptureEngine is recording!";

        m_hpwrRequest = PowerCreateRequest(&pwrCtxt);
    }

    void SetErrorID(HRESULT hr, UINT id)
    {
        m_errorID = SUCCEEDED(hr) ? 0 : id;
    }

    // Capture Engine Event Handlers
    void OnCaptureEngineInitialized(HRESULT& hrStatus);
    void OnPreviewStarted(HRESULT& hrStatus);
    void OnPreviewStopped(HRESULT& hrStatus);
    void OnRecordStarted(HRESULT& hrStatus);
    void OnRecordStopped(HRESULT& hrStatus);
    void WaitForResult()
    {
        WaitForSingleObject(m_hEvent, INFINITE);
    }
public:
    ~CaptureManager()
    {
        DestroyCaptureEngine();
    }

    static HRESULT CreateInstance(HWND hwndEvent, CaptureManager **ppEngine)
    {
        HRESULT hr = S_OK;
        *ppEngine = NULL;

        CaptureManager *pEngine = new (std::nothrow) CaptureManager(hwndEvent);
        if (pEngine == NULL)
        {
            hr = E_OUTOFMEMORY;
            goto Exit;
        }
        *ppEngine = pEngine;
        pEngine = NULL;

    Exit:
        if (NULL != pEngine)
        {
            delete pEngine;
        }
        return hr;
    }

    HRESULT InitializeCaptureManager(HWND hwndPreview, IUnknown* pUnk);
    void DestroyCaptureEngine()
    {
        if (NULL != m_hEvent)
        {
            CloseHandle(m_hEvent);
            m_hEvent = NULL;
        }
        SafeRelease(&m_pPreview);
        SafeRelease(&m_pEngine);
        SafeRelease(&m_pCallback);

        if(g_pDXGIMan)
        {
            g_pDXGIMan->ResetDevice(g_pDX11Device, g_ResetToken);
        }
        SafeRelease(&g_pDX11Device);
        SafeRelease(&g_pDXGIMan);

        m_bPreviewing = false;
        m_bRecording = false;
        m_bPhotoPending = false;
        m_errorID = 0;  
    }



    bool    IsPreviewing() const { return m_bPreviewing; }
    bool    IsRecording() const { return m_bRecording; }
    bool    IsPhotoPending() const { return m_bPhotoPending; }
    UINT    ErrorID() const { return m_errorID; }

    HRESULT OnCaptureEvent(WPARAM wParam, LPARAM lParam); 
    HRESULT SetVideoDevice(IUnknown *pUnk);
    HRESULT StartPreview();
    HRESULT StopPreview();
    HRESULT StartRecord(PCWSTR pszDestinationFile);
    HRESULT StopRecord();
    HRESULT TakePhoto(PCWSTR pszFileName);

    void    SleepState(bool fSleeping)
    {
        if (NULL != m_pCallback)
        {
            m_pCallback->m_fSleeping = fSleeping;
        }
    }

    HRESULT UpdateVideo()
    {
        if (m_pPreview)
        {
            return m_pPreview->UpdateVideo(NULL, NULL, NULL);
        }
        else
        {
            return S_OK;
        }
    }
};

#endif CAPTURE_H
