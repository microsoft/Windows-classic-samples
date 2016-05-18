//+-------------------------------------------------------------------------
//
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//
//  File:       setupui.h
//
//--------------------------------------------------------------------------

#ifndef _SETUPUI_H_3E24CC91_BC41_4182_BEBA_785BBB28B677_
#define _SETUPUI_H_3E24CC91_BC41_4182_BEBA_785BBB28B677_

#include <windows.h>
#include <urlmon.h>

/*---------------------------------------------------------------------------
 *
 * Constants
 *
 ---------------------------------------------------------------------------*/
#define MAX_STR_CAPTION 256

/*---------------------------------------------------------------------------
 *
 * Enums
 *
 ---------------------------------------------------------------------------*/
enum irmProgress // progress dialog return messages
{
    irmNotInitialized = -1, // dialog was not initialized
    irmOK             =  0, // ok
    irmCancel         =  1, // user depressed cancel button
};

/*---------------------------------------------------------------------------
 *
 * CDownloadUI class
 *
 ---------------------------------------------------------------------------*/
class CDownloadUI
{
public:
     CDownloadUI();
     ~CDownloadUI();

    bool Initialize(HINSTANCE hInst, HWND hwndParent, LPCSTR szCaption);
    bool Terminate();
    HWND GetCurrentWindow();
    bool HasUserCanceled();
    void SetUserCancel();
    void InitProgressBar(ULONG ulProgressMax);
    void IncrementProgressBar(ULONG ulProgress);

    irmProgress SetBannerText(LPCSTR szBanner);
    irmProgress SetActionText(LPCSTR szAction);

private:
    HINSTANCE m_hInst;  // handle to instance containing resources

    HWND  m_hwndProgress;    // handle to progress dialog
    HWND  m_hwndParent;      // handle to parent window
    char  m_szCaption[MAX_STR_CAPTION]; // caption
    bool  m_fInitialized;    // whether dialog has been initialized
    bool  m_fUserCancel;     // whether user has chosen to cancel
    ULONG m_ulProgressMax;   // maximum number of ticks on progress bar
    ULONG m_ulProgressSoFar; // current progress
};

/*---------------------------------------------------------------------------
 *
 * CDownloadBindStatusCallback class
 *
 ---------------------------------------------------------------------------*/

class CDownloadBindStatusCallback : public IBindStatusCallback
{
 public: // IUnknown implemented virtual functions
     HRESULT         __stdcall QueryInterface(const IID& riid, void** ppvObj);
     unsigned long   __stdcall AddRef();
     unsigned long   __stdcall Release();
 public: // IBindStatusCallback implemented virtual functions
     CDownloadBindStatusCallback(CDownloadUI* piDownloadUI);
    ~CDownloadBindStatusCallback();

    HRESULT __stdcall OnStartBinding(DWORD, IBinding*) {return S_OK;}
    HRESULT __stdcall GetPriority(LONG*) {return S_OK;}
    HRESULT __stdcall OnLowResource(DWORD ) {return S_OK;}
    HRESULT __stdcall OnProgress(ULONG ulProgress, ULONG ulProgressMax, ULONG ulStatusCode, LPCWSTR szStatusText);
    HRESULT __stdcall OnStopBinding(HRESULT, LPCWSTR ) {return S_OK;}
    HRESULT __stdcall GetBindInfo(DWORD*, BINDINFO*) {return S_OK;}
    HRESULT __stdcall OnDataAvailable(DWORD, DWORD, FORMATETC*, STGMEDIUM*) {return S_OK;}
    HRESULT __stdcall OnObjectAvailable(REFIID, IUnknown*) {return S_OK;}
 private:
    CDownloadUI* m_pDownloadUI; // pointer to actual UI
    int          m_iRefCnt;
    ULONG        m_ulProgressSoFar;
};

#endif //_SETUPUI_H_3E24CC91_BC41_4182_BEBA_785BBB28B677_