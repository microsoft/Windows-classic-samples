// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "Application.h"
#include "CommandHandler.h"
#include "RibbonFramework.h"

#include <UIRibbonPropertyHelpers.h>

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

__checkReturn HRESULT CApplication::CreateInstance(__deref_out_opt CApplication** ppApplication)
{
    if (!ppApplication)
    {
        return E_POINTER;
    }

    *ppApplication = NULL;
    HRESULT hr = E_FAIL;

    CApplication* pApplication = new CApplication();

    if (pApplication != NULL)
    {
        *ppApplication = pApplication;
        hr = S_OK;
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}

//
//  FUNCTION: OnCreateUICommand(UINT, UI_COMMANDTYPE, IUICommandHandler)
//
//  PURPOSE: Called by the Ribbon framework for each command specified in markup, to allow
//           the host application to bind a command handler to that command.
//
//  COMMENTS:
//
//    In this sample, the same command handler is returned for all commands
//    specified in the ContextPopup.xml file.
//
//
STDMETHODIMP CApplication::OnCreateUICommand(
    UINT nCmdID,
    __in UI_COMMANDTYPE typeID,
    __deref_out IUICommandHandler** ppCommandHandler)
{
    UNREFERENCED_PARAMETER(typeID);
    UNREFERENCED_PARAMETER(nCmdID);

    HRESULT hr = S_OK;

    if (NULL == m_pCommandHandler)
    {
        hr = CCommandHandler::CreateInstance(&m_pCommandHandler);
        if (FAILED(hr))
        {
            return hr;
        }
    }

    return m_pCommandHandler->QueryInterface(IID_PPV_ARGS(ppCommandHandler));
}

//
//  FUNCTION: OnViewChanged(UINT, UI_VIEWTYPE, IUnknown*, UI_VIEWVERB, INT)
//
//  PURPOSE: Called when the state of a View (Ribbon is a view) changes, for example, created, destroyed, or resized.
//
//
STDMETHODIMP CApplication::OnViewChanged(
    UINT viewId,
    __in UI_VIEWTYPE typeId,
    __in IUnknown* pView,
    UI_VIEWVERB verb,
    INT uReasonCode)
{
    UNREFERENCED_PARAMETER(viewId);
    UNREFERENCED_PARAMETER(uReasonCode);

    HRESULT hr = E_NOTIMPL;

    // Checks to see if the view that was changed was a Ribbon view.
    if (UI_VIEWTYPE_RIBBON == typeId)
    {
        switch (verb)
        {
            // The view was newly created.
            case UI_VIEWVERB_CREATE:
            {
                hr = S_OK;
                break;
            }
            // The view has been resized.  For the Ribbon view, the application should
            // call GetHeight to determine the height of the ribbon.
            case UI_VIEWVERB_SIZE:
            {
                IUIRibbon* pRibbon = NULL;
                UINT uRibbonHeight;
                // Call to the framework to determine the desired height of the Ribbon.
                hr = pView->QueryInterface(IID_PPV_ARGS(&pRibbon));
                if (SUCCEEDED(hr))
                {
                    hr = pRibbon->GetHeight(&uRibbonHeight);
                    pRibbon->Release();
                }
                break;
            }
            // The view was destroyed.
            case UI_VIEWVERB_DESTROY:
            {
                hr = S_OK;
                break;
            }
        }
    }
    return hr;
}


//
//  FUNCTION: OnDestroyUICommand(UINT, UI_COMMANDTYPE, IUICommandHandler*)
//
//  PURPOSE: Called by the Ribbon framework for each command at the time of ribbon destruction.
//
STDMETHODIMP CApplication::OnDestroyUICommand(
    UINT32 nCmdID,
    __in UI_COMMANDTYPE typeID,
    __in_opt IUICommandHandler* commandHandler)
{
    UNREFERENCED_PARAMETER(nCmdID);
    UNREFERENCED_PARAMETER(typeID);
    UNREFERENCED_PARAMETER(commandHandler);

    return E_NOTIMPL;
}

__checkReturn int CApplication::GetCurrentContext()
{
    return m_iCurrentContext;
}

void CApplication::SetCurrentContext(int newContext)
{
    m_iCurrentContext = newContext;
}