//////////////////////////////////////////////////////////////////////
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
//  TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//  Copyright (C) 2003  Microsoft Corporation.  All rights reserved.
//
//  Compartment.cpp
//
//          The status of GUID_COMPARTMENT_KEYBOARD_OPENCLOSE and
//          GUID_COMPARTMENT_KEYBOARD_DISABLED.
//
//////////////////////////////////////////////////////////////////////

#include "Globals.h"
#include "TextService.h"

//+---------------------------------------------------------------------------
//
// _IsKeyboardDisabled
//
// GUID_COMPARTMENT_KEYBOARD_DISABLED is the compartment in the context
// object.
//
//----------------------------------------------------------------------------

BOOL CTextService::_IsKeyboardDisabled()
{
    ITfCompartmentMgr *pCompMgr = NULL;
    ITfDocumentMgr *pDocMgrFocus = NULL;
    ITfContext *pContext = NULL;
    BOOL fDisabled = FALSE;

    if ((_pThreadMgr->GetFocus(&pDocMgrFocus) != S_OK) ||
        (pDocMgrFocus == NULL))
    {
        // if there is no focus document manager object, the keyboard 
        // is disabled.
        fDisabled = TRUE;
        goto Exit;
    }

    if ((pDocMgrFocus->GetTop(&pContext) != S_OK) ||
        (pContext == NULL))
    {
        // if there is no context object, the keyboard is disabled.
        fDisabled = TRUE;
        goto Exit;
    }

    if (pContext->QueryInterface(IID_ITfCompartmentMgr, (void **)&pCompMgr) == S_OK)
    {
        ITfCompartment *pCompartmentDisabled;
        ITfCompartment *pCompartmentEmptyContext;

        // Check GUID_COMPARTMENT_KEYBOARD_DISABLED.
        if (pCompMgr->GetCompartment(GUID_COMPARTMENT_KEYBOARD_DISABLED, &pCompartmentDisabled) == S_OK)
        {
            VARIANT var;
            if (S_OK == pCompartmentDisabled->GetValue(&var))
            {
                if (var.vt == VT_I4) // Even VT_EMPTY, GetValue() can succeed
                {
                    fDisabled = (BOOL)var.lVal;
                }
            }
            pCompartmentDisabled->Release();
        }

        // Check GUID_COMPARTMENT_EMPTYCONTEXT.
        if (pCompMgr->GetCompartment(GUID_COMPARTMENT_EMPTYCONTEXT, &pCompartmentEmptyContext) == S_OK)
        {
            VARIANT var;
            if (S_OK == pCompartmentEmptyContext->GetValue(&var))
            {
                if (var.vt == VT_I4) // Even VT_EMPTY, GetValue() can succeed
                {
                    fDisabled = (BOOL)var.lVal;
                }
            }
            pCompartmentEmptyContext->Release();
        }

        pCompMgr->Release();
    }

Exit:
    if (pContext)
        pContext->Release();

    if (pDocMgrFocus)
        pDocMgrFocus->Release();

    return fDisabled;
}

//+---------------------------------------------------------------------------
//
// _IsKeyboardOpen
//
// GUID_COMPARTMENT_KEYBOARD_OPENCLOSE is the compartment in the thread manager
// object.
//
//----------------------------------------------------------------------------

BOOL CTextService::_IsKeyboardOpen()
{
    ITfCompartmentMgr *pCompMgr = NULL;
    BOOL fOpen = FALSE;

    if (_pThreadMgr->QueryInterface(IID_ITfCompartmentMgr, (void **)&pCompMgr) == S_OK)
    {
        ITfCompartment *pCompartment;
        if (pCompMgr->GetCompartment(GUID_COMPARTMENT_KEYBOARD_OPENCLOSE, &pCompartment) == S_OK)
        {
            VARIANT var;
            if (S_OK == pCompartment->GetValue(&var))
            {
                if (var.vt == VT_I4) // Even VT_EMPTY, GetValue() can succeed
                {
                    fOpen = (BOOL)var.lVal;
                }
            }
        }
        pCompMgr->Release();
    }

    return fOpen;
}

//+---------------------------------------------------------------------------
//
// _SetKeyboardOpen
//
// GUID_COMPARTMENT_KEYBOARD_OPENCLOSE is the compartment in the thread manager
// object.
//
//----------------------------------------------------------------------------

HRESULT CTextService::_SetKeyboardOpen(BOOL fOpen)
{
    HRESULT hr = E_FAIL;
    ITfCompartmentMgr *pCompMgr = NULL;

    if (_pThreadMgr->QueryInterface(IID_ITfCompartmentMgr, (void **)&pCompMgr) == S_OK)
    {
        ITfCompartment *pCompartment;
        if (pCompMgr->GetCompartment(GUID_COMPARTMENT_KEYBOARD_OPENCLOSE, &pCompartment) == S_OK)
        {
            VARIANT var;
            var.vt = VT_I4;
            var.lVal = fOpen;
            hr = pCompartment->SetValue(_tfClientId, &var);
        }
        pCompMgr->Release();
    }

    return hr;
}


