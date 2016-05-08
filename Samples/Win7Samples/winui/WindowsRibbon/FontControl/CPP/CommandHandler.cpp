// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include <UIRibbon.h>

#include "CommandHandler.h"
#include "RibbonFramework.h"
#include <UIRibbonPropertyHelpers.h>

// Static method to create an instance of the object.
__checkReturn HRESULT CCommandHandler::CreateInstance(__deref_out IUICommandHandler **ppCommandHandler)
{
    if (!ppCommandHandler)
    {
        return E_POINTER;
    }

    *ppCommandHandler = NULL;

    HRESULT hr = S_OK;
   
    CCommandHandler* pCommandHandler = new CCommandHandler();

    if (pCommandHandler != NULL)
    {
        *ppCommandHandler = static_cast<IUICommandHandler *>(pCommandHandler);
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}

// IUnknown method implementations.
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
//
STDMETHODIMP CCommandHandler::UpdateProperty(
    UINT nCmdID,
    __in REFPROPERTYKEY key,
    __in_opt const PROPVARIANT* ppropvarCurrentValue,
    __out PROPVARIANT* ppropvarNewValue)
{
    UNREFERENCED_PARAMETER(nCmdID);

    HRESULT hr = E_NOTIMPL;
    if (key == UI_PKEY_FontProperties)
    {
        hr = E_POINTER;
        if (ppropvarCurrentValue != NULL)
        {
            // Get the font values for the selected text in the font control.
            IPropertyStore *pValues;
            hr = UIPropertyToInterface(UI_PKEY_FontProperties, *ppropvarCurrentValue, &pValues);
            if (SUCCEEDED(hr))
            {
                g_pFCSampleAppManager->GetValues(pValues);

                // Provide the new values to the font control.
                hr = UIInitPropertyFromInterface(UI_PKEY_FontProperties, pValues, ppropvarNewValue);
                pValues->Release();
            }
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
STDMETHODIMP CCommandHandler::Execute(
    UINT nCmdID,
    UI_EXECUTIONVERB verb,
    __in_opt const PROPERTYKEY* key,
    __in_opt const PROPVARIANT* ppropvarValue,
    __in_opt IUISimplePropertySet* pCommandExecutionProperties)
{
    UNREFERENCED_PARAMETER(nCmdID);

    HRESULT hr = E_NOTIMPL;
    if ((key) && (*key == UI_PKEY_FontProperties))
    {
        // Font properties have changed.
        switch (verb)
        {
            case UI_EXECUTIONVERB_EXECUTE:
            {
                hr = E_POINTER;
                if (pCommandExecutionProperties != NULL)
                {
                    // Get the changed properties.
                    PROPVARIANT varChanges;
                    hr = pCommandExecutionProperties->GetValue(UI_PKEY_FontProperties_ChangedProperties, &varChanges);
                    if (SUCCEEDED(hr))
                    {
                        IPropertyStore *pChanges;
                        hr = UIPropertyToInterface(UI_PKEY_FontProperties, varChanges, &pChanges);
                        if (SUCCEEDED(hr))
                        {
                            // Using the changed properties, set the new font on the selection on RichEdit control.
                            g_pFCSampleAppManager->SetValues(pChanges);
                            pChanges->Release();
                        }
                        PropVariantClear(&varChanges);
                    }
                }
                break;
            }
            case UI_EXECUTIONVERB_PREVIEW:
            {
                hr = E_POINTER;
                if (pCommandExecutionProperties != NULL)
                {
                    // Get the changed properties for the preview event.
                    PROPVARIANT varChanges;
                    hr = pCommandExecutionProperties->GetValue(UI_PKEY_FontProperties_ChangedProperties, &varChanges);
                    if (SUCCEEDED(hr))
                    {
                        IPropertyStore *pChanges;
                        hr = UIPropertyToInterface(UI_PKEY_FontProperties, varChanges, &pChanges);
                        if (SUCCEEDED(hr))
                        {
                            // Set the previewed values on the RichEdit control.
                            g_pFCSampleAppManager->SetPreviewValues(pChanges);
                            pChanges->Release();
                        }
                        PropVariantClear(&varChanges);
                    }
                }
                break;
            }
            case UI_EXECUTIONVERB_CANCELPREVIEW:
            {
                hr = E_POINTER;
                if (ppropvarValue != NULL)
                {
                    // Cancel the preview.
                    IPropertyStore *pValues;
                    hr = UIPropertyToInterface(UI_PKEY_FontProperties, *ppropvarValue, &pValues);
                    if (SUCCEEDED(hr))
                    {   
                        g_pFCSampleAppManager->CancelPreview(pValues);
                        pValues->Release();
                    }
                }
                break;
            }
        }
    }

    return hr;
}
