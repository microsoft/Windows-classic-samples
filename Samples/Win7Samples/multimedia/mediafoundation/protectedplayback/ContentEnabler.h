//////////////////////////////////////////////////////////////////////////
// ContentEnabler.h: Manages content enabler action.
// 
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//////////////////////////////////////////////////////////////////////////

#pragma once

// WM_APP_CONTENT_ENABLER: Signals that the application must perform a
// content enabler action.
const UINT WM_APP_CONTENT_ENABLER = WM_APP + 3; // no message parameters

// WM_APP_BROWSER_DONE: Signals that the user closed the browser window.
const UINT WM_APP_BROWSER_DONE = WM_APP + 4; // no message parameters


enum EnablerState
{
    Enabler_Ready,
    Enabler_SilentInProgress,
    Enabler_NonSilentInProgress,
    Enabler_Complete
};

enum EnablerFlags
{
    SilentOrNonSilent = 0,  // Use silent if supported, otherwise use non-silent.
    ForceNonSilent = 1      // Use non-silent.
};
    

//////////////////////////////////////////////////////////////////////////
//  ContentProtectionManager
//  Description: Manages content-enabler actions.
// 
//  This object implements IMFContentProtectionManager. The PMP media
//  session uses this interface to pass a content enabler object back
//  to the application. A content enabler in an object that performs some 
//  action needed to play a protected file, such as license acquistion.
//
//  For more information about content enablers, see IMFContentEnabler in
//  the Media Foundation SDK documentation.
//////////////////////////////////////////////////////////////////////////

class ContentProtectionManager : 
    public IMFAsyncCallback, 
    public IMFContentProtectionManager,
    public DispatchCallback // To get callbacks from the browser control.
{
public:

    static HRESULT CreateInstance(HWND hNotify, ContentProtectionManager **ppManager);

    // IUnknown
    STDMETHODIMP QueryInterface(REFIID iid, void** ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

    // IMFAsyncCallback methods
    STDMETHODIMP GetParameters(DWORD*, DWORD*)
    {
        // Implementation of this method is optional.
        return E_NOTIMPL;
    }

    STDMETHODIMP Invoke(IMFAsyncResult* pAsyncResult);

    // IMFContentProtectionManager methods
    STDMETHODIMP BeginEnableContent(
        IMFActivate *pEnablerActivate,
        IMFTopology *pTopo,
        IMFAsyncCallback *pCallback,
        IUnknown *punkState
        );

    STDMETHODIMP EndEnableContent(IMFAsyncResult *pResult);

    // DispatchCallback
    void OnDispatchInvoke(DISPID  dispIdMember);

    // Public methods for the application.

    HRESULT         DoEnable(EnablerFlags flags = SilentOrNonSilent);
    HRESULT         CancelEnable();
    HRESULT         CompleteEnable();

    EnablerState    GetState() const { return m_state; }
    HRESULT         GetStatus() const { return m_hrStatus; }

private:


    ContentProtectionManager(HWND hwndNotify);
    virtual ~ContentProtectionManager();

    HRESULT DoNonSilentEnable();


    long                    m_nRefCount;        // Reference count.

    EnablerState            m_state;
    HRESULT                 m_hrStatus;         // Status code from the most recent event.

    HWND                    m_hwnd;

    IMFContentEnabler       *m_pEnabler;        // Content enabler.
    IMFMediaEventGenerator  *m_pMEG;            // The content enabler's event generator interface.
    IMFAsyncResult          *m_pResult;         // Asynchronus result object.

    WebHelper               m_webHelper;        // For non-silent enable

};