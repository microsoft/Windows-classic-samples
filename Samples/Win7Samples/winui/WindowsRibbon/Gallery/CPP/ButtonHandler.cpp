// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "ButtonHandler.h"
#include "PropertySet.h"
#include "Renderer.h"
#include "Resource.h"
#include "RibbonFramework.h"
#include <uiribbonpropertyhelpers.h>
#include "ids.h"

extern CRenderer   g_renderer;

//
//  FUNCTION: Execute()
//
//  PURPOSE: Called by the Ribbon framework when one of the size or color buttons is pressed by the user.
//
//  COMMENTS:
//    This sets either the size or the color of the shapes being drawn and invalidates the toggled state of each button.
//
//
STDMETHODIMP CButtonHandler::Execute(UINT nCmdID,
                   UI_EXECUTIONVERB verb, 
                   __in_opt const PROPERTYKEY* key,
                   __in_opt const PROPVARIANT* ppropvarValue,
                   __in_opt IUISimplePropertySet* pCommandExecutionProperties)
{
    UNREFERENCED_PARAMETER(ppropvarValue);
    UNREFERENCED_PARAMETER(pCommandExecutionProperties);

    HRESULT hr = E_FAIL;

    if (verb == UI_EXECUTIONVERB_EXECUTE)
    {
        if (key && *key == UI_PKEY_BooleanValue) // The user clicked one of the toggle buttons in the command gallery.
        {
            RenderParam param;
            g_renderer.GetRenderParam(&param);

            switch (nCmdID)
            {
            case IDR_CMD_RED:
            case IDR_CMD_GREEN:
            case IDR_CMD_BLUE:
                param.eShapeColor = (SHAPE_COLOR)(nCmdID - IDR_CMD_RED);
                break;
            case IDR_CMD_SMALL:
            case IDR_CMD_MEDIUM:
            case IDR_CMD_LARGE:
                param.eShapeSize = (SHAPE_SIZE)(nCmdID - IDR_CMD_SMALL);
                break;
            }
            g_renderer.UpdateRenderParam(param);
            // Update the BooleanValue property on all commands to deselect the previous size or color.
            hr = g_pFramework->InvalidateUICommand(UI_ALL_COMMANDS, UI_INVALIDATIONS_PROPERTY, &UI_PKEY_BooleanValue);
        }
    }
    return hr;
}

//
//  FUNCTION: UpdateProperty()
//
//  PURPOSE: Called by the Ribbon framework when a command property (PKEY) needs to be updated.
//
//  COMMENTS:
//
//    This function updates the toggled state of each button.
//
//
STDMETHODIMP CButtonHandler::UpdateProperty(UINT nCmdID,
                              __in REFPROPERTYKEY key,
                              __in_opt const PROPVARIANT* ppropvarCurrentValue,
                              __out PROPVARIANT* ppropvarNewValue)
{
    UNREFERENCED_PARAMETER(ppropvarCurrentValue);

    HRESULT hr = E_FAIL;
    if (key == UI_PKEY_BooleanValue)
    {
        RenderParam param;
        g_renderer.GetRenderParam(&param);

        // The currently active size and color will appear selected; the others will appear deselected.
        if (nCmdID == (UINT)(param.eShapeSize + IDR_CMD_SMALL) || nCmdID == (UINT)(param.eShapeColor + IDR_CMD_RED))
        {
            hr = UIInitPropertyFromBoolean(UI_PKEY_BooleanValue, TRUE, ppropvarNewValue);
        }
        else
        {
            hr = UIInitPropertyFromBoolean(UI_PKEY_BooleanValue, FALSE, ppropvarNewValue);
        }
    }
    return hr;
}

HRESULT CButtonHandler::CreateInstance(__deref_out CButtonHandler **ppHandler)
{
    if (!ppHandler)
    {
        return E_POINTER;
    }

    *ppHandler = NULL;

    HRESULT hr = S_OK;

    CButtonHandler* pHandler = new CButtonHandler();

    if (pHandler != NULL)
    {
        *ppHandler = pHandler;
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}

// IUnknown methods.
STDMETHODIMP_(ULONG) CButtonHandler::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) CButtonHandler::Release()
{
    LONG cRef = InterlockedDecrement(&m_cRef);
    if (cRef == 0)
    {
        delete this;
    }

    return cRef;
}

STDMETHODIMP CButtonHandler::QueryInterface(REFIID iid, void** ppv)
{
    if (!ppv)
    {
        return E_POINTER;
    }

    if (iid == __uuidof(IUnknown))
    {
        *ppv = static_cast<IUnknown*>(this);
    }
    else if (iid == __uuidof(IUICommandHandler))
    {
        *ppv = static_cast<IUICommandHandler*>(this);
    }
    else 
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}