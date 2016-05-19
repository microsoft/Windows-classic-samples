// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "Application.h"
#include "BorderSizeHandler.h"
#include "BorderStyleHandler.h"
#include "ButtonHandler.h"
#include "LayoutHandler.h"
#include "ShapeHandler.h"
#include "SizeAndColorHandler.h"
#include "ids.h"

//
//  FUNCTION: OnViewChanged(UINT, UI_VIEWTYPE, IUnknown*, UI_VIEWVERB, INT)
//
//  PURPOSE: Called when the state of a View (Ribbon is a view) changes, for example, created, destroyed, or resized.
//
//
STDMETHODIMP CApplication::OnViewChanged(UINT32 nViewID, __in UI_VIEWTYPE typeID,
__in IUnknown* pView, UI_VIEWVERB verb, INT32 uReasonCode)
{
    UNREFERENCED_PARAMETER(nViewID);
    UNREFERENCED_PARAMETER(typeID);
    UNREFERENCED_PARAMETER(pView);
    UNREFERENCED_PARAMETER(uReasonCode);


    HRESULT hr = E_FAIL;

    switch (verb)
    {
    case UI_VIEWVERB_SIZE:
        // Redraw the shapes since the space available has now changed.
        InvalidateRect(m_hwnd, NULL, TRUE);
        hr = S_OK;
        break;
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
//    In this Gallery sample, there is one handler for each gallery, and one for all of
//    the buttons in the Size and Color gallery.
//
//
STDMETHODIMP CApplication::OnCreateUICommand(UINT32 nCmdID, __in UI_COMMANDTYPE typeID,
__deref_out IUICommandHandler** ppCommandHandler)
{
    UNREFERENCED_PARAMETER(typeID);

    HRESULT hr = E_FAIL;
    switch (nCmdID)
    {
    case IDR_CMD_SHAPES:
    {
        CShapeHandler *pShapeHandler = NULL;
        hr = CShapeHandler::CreateInstance(&pShapeHandler);
        if (SUCCEEDED(hr))
        {
            hr = pShapeHandler->QueryInterface(IID_PPV_ARGS(ppCommandHandler));
            pShapeHandler->Release();
        }
        break;
    }
    case IDR_CMD_SIZEANDCOLOR:
    {
        CSizeAndColorHandler *pSizeAndColorHandler = NULL;
        hr = CSizeAndColorHandler::CreateInstance(&pSizeAndColorHandler);
        if (SUCCEEDED(hr))
        {
            hr = pSizeAndColorHandler->QueryInterface(IID_PPV_ARGS(ppCommandHandler));
            pSizeAndColorHandler->Release();
        }
        break;
    }
    case IDR_CMD_BORDERSTYLES:
    {
        CBorderStyleHandler *pBorderStyleHandler = NULL;
        hr = CBorderStyleHandler::CreateInstance(&pBorderStyleHandler);
        if (SUCCEEDED(hr))
        {
            hr = pBorderStyleHandler->QueryInterface(IID_PPV_ARGS(ppCommandHandler));
            pBorderStyleHandler->Release();
        }
        break;
    }
    case IDR_CMD_BORDERSIZES:
    {
        CBorderSizeHandler *pBorderSizeHandler = NULL;
        hr = CBorderSizeHandler::CreateInstance(&pBorderSizeHandler);
        if (SUCCEEDED(hr))
        {
            hr = pBorderSizeHandler->QueryInterface(IID_PPV_ARGS(ppCommandHandler));
            pBorderSizeHandler->Release();
        }
        break;
    }
    case IDR_CMD_LAYOUTS:
    {
        CLayoutHandler *pLayoutHandler = NULL;
        hr = CLayoutHandler::CreateInstance(&pLayoutHandler);
        if (SUCCEEDED(hr))
        {
            hr = pLayoutHandler->QueryInterface(IID_PPV_ARGS(ppCommandHandler));
            pLayoutHandler->Release();
        }
        break;
    }
    case IDR_CMD_RED:
    case IDR_CMD_GREEN:
    case IDR_CMD_BLUE:
    case IDR_CMD_SMALL:
    case IDR_CMD_MEDIUM:
    case IDR_CMD_LARGE:
    {
        CButtonHandler *pButtonHandler = NULL;
        hr = CButtonHandler::CreateInstance(&pButtonHandler);
        if (SUCCEEDED(hr))
        {
            hr = pButtonHandler->QueryInterface(IID_PPV_ARGS(ppCommandHandler));
            pButtonHandler->Release();
        }
        break;
    }
    }
    return hr;
}

//
//  FUNCTION: OnDestroyUICommand(UINT, UI_COMMANDTYPE, IUICommandHandler*)
//
//  PURPOSE: Called by the Ribbon framework for each command at the time of ribbon destruction.
//
//
STDMETHODIMP CApplication::OnDestroyUICommand(UINT32 commandId, __in UI_COMMANDTYPE typeID,
__in_opt IUICommandHandler* pCommandHandler)
{
    UNREFERENCED_PARAMETER(commandId);
    UNREFERENCED_PARAMETER(typeID);
    UNREFERENCED_PARAMETER(pCommandHandler);
    return E_NOTIMPL;
}

HRESULT CApplication::CreateInstance(__deref_out CApplication **ppApplication, HWND hwnd)
{
    if (!ppApplication)
    {
        return E_POINTER;
    }
    if (!hwnd)
    {
        return E_INVALIDARG;
    }

    *ppApplication = NULL;

    HRESULT hr = S_OK;

    CApplication* pApplication = new CApplication();

    if (pApplication != NULL)
    {
        pApplication->m_hwnd = hwnd;
        *ppApplication = pApplication;
        
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
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