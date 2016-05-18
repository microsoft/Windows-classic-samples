// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "CommandHandler.h"
#include "RibbonFramework.h"

#include <UIRibbonPropertyHelpers.h>

//  FUNCTION: GetContextMapIDFromCommandID()
//
//  PURPOSE: Given the command id, return the Context Map it corresponds to so that we can set it.
//
//  RETURNS: The ID of the Context Map that corresponds to iCmdID, 0 if the Context Map ID is not found.
//
//  NOTE:    No command in the ribbon markup is allowed to use 0 as its ID.
//
//
__checkReturn int GetContextMapIDFromCommandID(__in int iCmdID)
{
    int iRet = 0;

    switch (iCmdID)
    {
    case IDC_CMD_CONTEXT1:
        iRet = IDC_CMD_CONTEXTMAP1;
        break;
    case IDC_CMD_CONTEXT2:
        iRet = IDC_CMD_CONTEXTMAP2;
        break;
    case IDC_CMD_CONTEXT3:
        iRet = IDC_CMD_CONTEXTMAP3;
        break;
    case IDC_CMD_CONTEXT4:
        iRet = IDC_CMD_CONTEXTMAP4;
        break;
    default:
        ;
    }

    return iRet;
}

// IUnknown methods.
STDMETHODIMP_(ULONG) CCommandHandler::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) CCommandHandler::Release()
{
    LONG cRef = InterlockedDecrement(&m_cRef);
    if (cRef == 0)
    {
        delete this;
    }

    return cRef;
}

STDMETHODIMP CCommandHandler::QueryInterface(REFIID iid, void** ppv)
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

__checkReturn HRESULT CCommandHandler::CreateInstance(__deref_out CCommandHandler **ppCommandHandler)
{
    if (!ppCommandHandler)
    {
        return E_POINTER;
    }

    *ppCommandHandler = NULL;
    HRESULT hr = E_FAIL;

    CCommandHandler* pHandler = new CCommandHandler();

    if (pHandler != NULL)
    {
        *ppCommandHandler = pHandler;
        hr = S_OK;
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
};

//
//  FUNCTION: UpdateProperty()
//
//  PURPOSE: Called by the Ribbon framework when a command property (PKEY) needs to be updated.
//
//  COMMENTS:
//
//    This function is used to provide new command property values, such as labels, icons, or
//    tooltip information, when requested by the Ribbon framework.  
//    
//    In this sample, this updates the toggle state of the toggle buttons in the ribbon.  
//
//
STDMETHODIMP CCommandHandler::UpdateProperty(
    UINT nCmdID,
    __in REFPROPERTYKEY key,
    __in_opt const PROPVARIANT* ppropvarCurrentValue,
    __out PROPVARIANT* ppropvarNewValue)
{
    UNREFERENCED_PARAMETER(ppropvarCurrentValue);

    HRESULT hr = E_NOTIMPL;

    if (key == UI_PKEY_BooleanValue)
    {
        if (nCmdID != cmdToggleButton)
        {
            hr = UIInitPropertyFromBoolean(UI_PKEY_BooleanValue, g_pApplication->GetCurrentContext() == GetContextMapIDFromCommandID(nCmdID), ppropvarNewValue);
        }
    }

    return hr;
}

//
//  FUNCTION: Execute()
//
//  PURPOSE: Called by the Ribbon framework when a command is executed by the user.  For example, when
//           a button is pressed.
//
//
STDMETHODIMP CCommandHandler::Execute(
                                      UINT nCmdID,
                                      UI_EXECUTIONVERB verb,
                                      __in_opt const PROPERTYKEY* key,
                                      __in_opt const PROPVARIANT* ppropvarValue,
                                      __in_opt IUISimplePropertySet* pCommandExecutionProperties)
{
    UNREFERENCED_PARAMETER(verb);
    UNREFERENCED_PARAMETER(ppropvarValue);
    UNREFERENCED_PARAMETER(pCommandExecutionProperties);

    HRESULT hr = S_OK;

    if (key != NULL && *key == UI_PKEY_BooleanValue)
    {
        if (nCmdID != cmdToggleButton)
        {
            g_pApplication->SetCurrentContext(GetContextMapIDFromCommandID(nCmdID));

            // We need to update the toggle state (boolean value) of the toggle buttons,
            // But we just invalidate all things to keep it simple.
            hr = g_pFramework->InvalidateUICommand(UI_ALL_COMMANDS, UI_INVALIDATIONS_VALUE, NULL);
        }
    }

    return hr;
}
