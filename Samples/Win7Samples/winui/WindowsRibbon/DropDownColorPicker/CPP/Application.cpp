// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "Application.h"

// Static method to create an instance of this object.
__checkReturn HRESULT CApplication::CreateInstance(__deref_out_opt IUIApplication** ppApplication, HWND hwnd)
{
    if (!ppApplication)
    {
        return E_POINTER;
    }

    if (!hwnd)
    {
        return E_POINTER;
    }    

    *ppApplication = NULL;

    CApplication* pApplication = new CApplication();

    if (pApplication != NULL)
    {
        pApplication->m_hWnd = hwnd;
        *ppApplication = pApplication;        
    }

    else
    {
        return E_OUTOFMEMORY;
    }

    return S_OK;
}

// IUnknown methods.
STDMETHODIMP_(ULONG) CApplication::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) CApplication::Release()
{
    LONG cRef = InterlockedDecrement(&m_cRef);

    if (cRef == 0)
    {
        delete this;
    }

    return cRef;
}

STDMETHODIMP CApplication::QueryInterface(REFIID iid, void** ppv)
{
    if (!ppv)
    {
        return E_POINTER;
    }

    if (iid == __uuidof(IUnknown))
    {
        *ppv = static_cast<IUnknown*>(this);
    }
    else if (iid == __uuidof(IUIApplication))
    {
        *ppv = static_cast<IUIApplication*>(this);
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }

    AddRef();

    return S_OK;
}

//
//  FUNCTION: OnViewChanged(UINT, UI_VIEWTYPE, IUnknown*, UI_VIEWVERB, INT)
//
//  PURPOSE: Called when the state of a View (Ribbon is a view) changes, for example, created, destroyed, or resized.
//
//
STDMETHODIMP CApplication::OnViewChanged(UINT32 nViewID, __in UI_VIEWTYPE typeID, __in IUnknown* pView, UI_VIEWVERB verb, INT32 uReasonCode)  
{
    UNREFERENCED_PARAMETER(nViewID);
    UNREFERENCED_PARAMETER(typeID);
    UNREFERENCED_PARAMETER(pView);
    UNREFERENCED_PARAMETER(verb);
    UNREFERENCED_PARAMETER(uReasonCode);
    InvalidateRect(m_hWnd, NULL, TRUE);

    return S_OK;
}

//
//  FUNCTION: OnCreateUICommand(UINT, UI_COMMANDTYPE, IUICommandHandler)
//
//  PURPOSE: Called by the Ribbon framework for each command specified in markup, to allow
//           the host application to bind a command handler to that command.
//
//  COMMENTS:
//
//    In this SimpleRibbon sample, the same command handler is returned for all commands
//    specified in the ribbonmarkup.xml file.
//
//
STDMETHODIMP CApplication::OnCreateUICommand(UINT32 nCmdID, __in UI_COMMANDTYPE typeID, __deref_out IUICommandHandler** ppCommandHandler) 
{ 
    UNREFERENCED_PARAMETER(typeID);

    HRESULT hr = S_OK;

    switch(nCmdID)
    {
        case IDR_CMD_UPDATE:
        case IDR_CMD_CLEAR:
            // Create CommandHandler object.
            if (NULL == m_pButtonHandler)
            {
                hr = CButtonHandler::CreateInstance(&m_pButtonHandler);
                if (FAILED(hr))
                {
                    return hr;
                }
            }
            return m_pButtonHandler->QueryInterface(IID_PPV_ARGS(ppCommandHandler));
        case IDR_CMD_THEMEDDCP:
        case IDR_CMD_STANDARDDDCP:
        case IDR_CMD_HIGHLIGHTDDCP:
            // Create CommandHandler object.
            if (NULL == m_pColorPickerHandler)
            {
                hr = CColorPickerHandler::CreateInstance(&m_pColorPickerHandler);
                if (FAILED(hr))
                {
                    return hr;
                }
            }
            return m_pColorPickerHandler->QueryInterface(IID_PPV_ARGS(ppCommandHandler));
    }

    return hr;  
}

//
//  FUNCTION: OnDestroyUICommand(UINT, UI_COMMANDTYPE, IUICommandHandler*)
//
//  PURPOSE: Called by the Ribbon framework for each command at the time of ribbon destruction.
//
//
STDMETHODIMP CApplication::OnDestroyUICommand(UINT32 commandId, __in UI_COMMANDTYPE typeID, __in_opt IUICommandHandler* pCommandHandler) 
{
    UNREFERENCED_PARAMETER(commandId);
    UNREFERENCED_PARAMETER(typeID);
    UNREFERENCED_PARAMETER(pCommandHandler);

    return E_NOTIMPL; 
}