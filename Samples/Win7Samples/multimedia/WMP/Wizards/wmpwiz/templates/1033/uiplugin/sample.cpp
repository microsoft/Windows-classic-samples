/////////////////////////////////////////////////////////////////////////////
//
// [!output root].cpp : Implementation of C[!output Safe_root]
// Copyright (c) Microsoft Corporation. All rights reserved.
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "[!output root].h"
[!if HASWINDOW]
#include "CPluginWindow.h"
[!endif]
[!if HASPROPERTYPAGE]
#include "CPropertyDialog.h"
[!endif]

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::C[!output Safe_root]
// Constructor

C[!output Safe_root]::C[!output Safe_root]()
{
[!if HASWINDOW]
    m_pPluginWindow = NULL;
[!endif]
[!if HASPROPERTYPAGE]
    wcsncpy_s(m_wszPluginText, sizeof(m_wszPluginText) / sizeof(m_wszPluginText[0]), L"[!output root] Plugin", sizeof(m_wszPluginText) / sizeof(m_wszPluginText[0]));
[!endif]
[!if LISTENTOEVENTS]
    m_dwAdviseCookie = 0;
[!endif]
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::~C[!output Safe_root]
// Destructor

C[!output Safe_root]::~C[!output Safe_root]()
{
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]:::FinalConstruct
// Called when an plugin is first loaded. Use this function to do one-time
// intializations that could fail instead of doing this in the constructor,
// which cannot return an error.

HRESULT C[!output Safe_root]::FinalConstruct()
{
    return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]:::FinalRelease
// Called when a plugin is unloaded. Use this function to free any
// resources allocated in FinalConstruct.

void C[!output Safe_root]::FinalRelease()
{
    ReleaseCore();
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::SetCore
// Set WMP core interface

HRESULT C[!output Safe_root]::SetCore(IWMPCore *pCore)
{
    HRESULT hr = S_OK;

    // release any existing WMP core interfaces
    ReleaseCore();

    // If we get passed a NULL core, this  means
    // that the plugin is being shutdown.

    if (pCore == NULL)
    {
        return S_OK;
    }

    m_spCore = pCore;

[!if LISTENTOEVENTS]
    // connect up the event interface
    CComPtr<IConnectionPointContainer>  spConnectionContainer;

    hr = m_spCore->QueryInterface( &spConnectionContainer );

    if (SUCCEEDED(hr))
    {
        hr = spConnectionContainer->FindConnectionPoint( __uuidof(IWMPEvents), &m_spConnectionPoint );
    }

    if (SUCCEEDED(hr))
    {
        hr = m_spConnectionPoint->Advise( GetUnknown(), &m_dwAdviseCookie );

        if ((FAILED(hr)) || (0 == m_dwAdviseCookie))
        {
            m_spConnectionPoint = NULL;
        }
    }

[!endif]
    return hr;
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::ReleaseCore
// Release WMP core interfaces

void C[!output Safe_root]::ReleaseCore()
{
[!if LISTENTOEVENTS]
    if (m_spConnectionPoint)
    {
        if (0 != m_dwAdviseCookie)
        {
            m_spConnectionPoint->Unadvise(m_dwAdviseCookie);
            m_dwAdviseCookie = 0;
        }
        m_spConnectionPoint = NULL;
    }

[!endif]
    if (m_spCore)
    {
        m_spCore = NULL;
    }
}

[!if HASWINDOW]
/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::Create
// Create window for plugin

HRESULT C[!output Safe_root]::Create(HWND hwndParent, HWND *phwndWindow)
{
    if (NULL == phwndWindow)
    {
        return E_UNEXPECTED;
    }

    m_pPluginWindow = new CPluginWindow(this);
    if (NULL == m_pPluginWindow)
    {
        return E_OUTOFMEMORY;
    }

    // create the plugin window
    RECT rcPos = { 0, 0, 100, 100};
    HWND hWnd = m_pPluginWindow->Create(hwndParent, rcPos, L"[!output root] Plugin");
    if (NULL == hWnd)
    {
        delete m_pPluginWindow;
        m_pPluginWindow = NULL;
        return E_FAIL;
    }

    *phwndWindow = hWnd;

    return S_OK;
}
[!endif]

[!if HASWINDOW]
/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::Destroy
// Destroy window for plugin

HRESULT C[!output Safe_root]::Destroy()
{
    if (NULL == m_pPluginWindow)
    {
        return E_UNEXPECTED;
    }

    m_pPluginWindow->DestroyWindow();
    delete m_pPluginWindow;
    m_pPluginWindow = NULL;

    return S_OK;
}
[!endif]

[!if HASWINDOW]
/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::TranslateAccelerator
// Handle TranslateAccelerator for plugin

HRESULT C[!output Safe_root]::TranslateAccelerator(LPMSG lpmsg)
{
    if (NULL == m_pPluginWindow)
    {
        return E_UNEXPECTED;
    }

    if (NULL == lpmsg)
    {
        return E_POINTER;
    }

    return S_OK;
}
[!endif]

[!if HASPROPERTYPAGE]
/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::DisplayPropertyPage
// Display property page for plugin

HRESULT C[!output Safe_root]::DisplayPropertyPage(HWND hwndParent)
{
    CPropertyDialog dialog(this);

    dialog.DoModal(hwndParent);

    return S_OK;
}
[!endif]

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::GetProperty
// Get plugin property

HRESULT C[!output Safe_root]::GetProperty(const WCHAR *pwszName, VARIANT *pvarProperty)
{
    if (NULL == pvarProperty)
    {
        return E_POINTER;
    }

[!if HASWINDOW]
    HRESULT hr = S_OK;

    if (0 == lstrcmpiW(pwszName, PLUGIN_SEPARATEWINDOW_DEFAULTWIDTH ))
    {
        pvarProperty->vt = VT_I4;
        pvarProperty->lVal = 100;
    }
    else if (0 == lstrcmpiW(pwszName, PLUGIN_SEPARATEWINDOW_DEFAULTHEIGHT))
    {
        pvarProperty->vt = VT_I4;
        pvarProperty->lVal = 100;
    }
    else
    {
        hr = E_NOTIMPL;
    }

    return hr;
[!else]
    return E_NOTIMPL;
[!endif]
}

/////////////////////////////////////////////////////////////////////////////
// C[!output Safe_root]::SetProperty
// Set plugin property

HRESULT C[!output Safe_root]::SetProperty(const WCHAR *pwszName, const VARIANT *pvarProperty)
{
    return E_NOTIMPL;
}
