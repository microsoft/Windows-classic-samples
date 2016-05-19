// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "BorderSizeHandler.h"
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
//  PURPOSE: Called by the Ribbon framework when the selects or types in a new border size.
//
//  COMMENTS:
//    This sets the size of the border around the shapes being drawn or displays an error
//    message if the user entered an invalid size.
//
//
STDMETHODIMP CBorderSizeHandler::Execute(UINT nCmdID,
                   UI_EXECUTIONVERB verb, 
                   __in_opt const PROPERTYKEY* key,
                   __in_opt const PROPVARIANT* ppropvarValue,
                   __in_opt IUISimplePropertySet* pCommandExecutionProperties)
{
    UNREFERENCED_PARAMETER(nCmdID);

    HRESULT hr = E_FAIL;
    if (verb == UI_EXECUTIONVERB_EXECUTE)
    {
        if (key && *key == UI_PKEY_SelectedItem)
        {
            RenderParam param;
            g_renderer.GetRenderParam(&param);

            UINT selected = ppropvarValue->uintVal;
            switch (selected)
            {
            case 0:
                param.uBorderSize = 1;
                break;
            case 1:
                param.uBorderSize = 3;
                break;
            case 2:
                param.uBorderSize = 5;
                break;
            case UI_COLLECTION_INVALIDINDEX: // The new selection is a value that the user typed.
                if (pCommandExecutionProperties != NULL)
                {
                    PROPVARIANT var;
                    pCommandExecutionProperties->GetValue(UI_PKEY_Label, &var); // The text entered by the user is contained in the property set with the pkey UI_PKEY_Label.
                    
                    BSTR bstr = var.bstrVal;
                    ULONG newSize;

                    hr = VarUI4FromStr(bstr,GetUserDefaultLCID(),0,&newSize);
                    
                    if (FAILED(hr) || newSize < 1 || newSize > 15)
                    {
                        WCHAR wszErrorMessage[MAX_RESOURCE_LENGTH];
                        LoadString(GetModuleHandle(NULL), IDS_INVALID_SIZE_MESSAGE, wszErrorMessage, MAX_RESOURCE_LENGTH);

                        WCHAR wszErrorTitle[MAX_RESOURCE_LENGTH];
                        LoadString(GetModuleHandle(NULL), IDS_INVALID_SIZE_TITLE, wszErrorTitle, MAX_RESOURCE_LENGTH);

                        MessageBox(NULL, wszErrorMessage, wszErrorTitle, MB_OK);
                        // Manually changing the text requires invalidating the StringValue property.
                        g_pFramework->InvalidateUICommand(IDR_CMD_BORDERSIZES, UI_INVALIDATIONS_PROPERTY, &UI_PKEY_StringValue);
                        break;
                    }
                    param.uBorderSize = newSize;
                    PropVariantClear(&var);
                }
                break;
            }
            g_renderer.UpdateRenderParam(param);
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
//    Depending on the value of key, this will populate the gallery, update the selected item or 
//    text, or enable/disable the gallery.
//
//
STDMETHODIMP CBorderSizeHandler::UpdateProperty(UINT nCmdID,
                              __in REFPROPERTYKEY key,
                              __in_opt const PROPVARIANT* ppropvarCurrentValue,
                              __out PROPVARIANT* ppropvarNewValue)
{
    UNREFERENCED_PARAMETER(nCmdID);

    HRESULT hr = E_FAIL;

    if (key == UI_PKEY_ItemsSource)
    {
        IUICollection* pCollection;
        hr = ppropvarCurrentValue->punkVal->QueryInterface(IID_PPV_ARGS(&pCollection));
        if (FAILED(hr))
        {
            return hr;
        }

        int labelIds[] = {IDS_BORDERSIZE_1, IDS_BORDERSIZE_3, IDS_BORDERSIZE_5};

        // Populate the combobox with the three default border sizes
        for (int i=0; i<_countof(labelIds); i++)
        {
            // Create a new property set for each item.
            CPropertySet* pItem;
            hr = CPropertySet::CreateInstance(&pItem);
            if (FAILED(hr))
            {
                pCollection->Release();
                return hr;
            }
            
            // Load the label for each size from the resource file.
            WCHAR wszLabel[MAX_RESOURCE_LENGTH];
            LoadString(GetModuleHandle(NULL), labelIds[i], wszLabel, MAX_RESOURCE_LENGTH);

            // Initialize the property set with no image, the label that was just loaded, and no category.
            pItem->InitializeItemProperties(NULL, wszLabel, UI_COLLECTION_INVALIDINDEX);

            // Add the newly-created property set to the collection supplied by the framework.
            pCollection->Add(pItem);

            pItem->Release();
        }
        pCollection->Release();
        hr = S_OK;
    }
    else if (key == UI_PKEY_Categories)
    {
        // A return value of S_FALSE or E_NOTIMPL will result in a gallery with no categories.
        // If you return any error other than E_NOTIMPL, the contents of the gallery will not display.
        hr = S_FALSE;
    }
    else if (key == UI_PKEY_SelectedItem)
    {
        // Use the first size as the default selection.
        hr = UIInitPropertyFromUInt32(UI_PKEY_SelectedItem, 0, ppropvarNewValue);
    }
    else if (key == UI_PKEY_StringValue)
    {
        // Set the text of the size combobox to the current border size.
        RenderParam param;
        g_renderer.GetRenderParam(&param);
        BSTR bstr;

        hr = VarBstrFromUI4(param.uBorderSize, GetUserDefaultLCID(), 0, &bstr);
        if(FAILED(hr))
        {
            return hr;
        }
        
        hr = UIInitPropertyFromString(UI_PKEY_StringValue, bstr, ppropvarNewValue);
        SysFreeString(bstr);
    }
    else if (key == UI_PKEY_Enabled)
    {
        // The border size combobox gets disabled when Dash Line is selected for the border style.
        RenderParam param;
        g_renderer.GetRenderParam(&param);
        BOOL active = param.eBorderStyle != DASH;
        hr = UIInitPropertyFromBoolean(UI_PKEY_Enabled, active, ppropvarNewValue);
    }
    return hr;
}

HRESULT CBorderSizeHandler::CreateInstance(__deref_out CBorderSizeHandler **ppHandler)
{
    if (!ppHandler)
    {
        return E_POINTER;
    }

    *ppHandler = NULL;

    HRESULT hr = S_OK;

    CBorderSizeHandler* pHandler = new CBorderSizeHandler();

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
STDMETHODIMP_(ULONG) CBorderSizeHandler::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) CBorderSizeHandler::Release()
{
    LONG cRef = InterlockedDecrement(&m_cRef);
    if (cRef == 0)
    {
        delete this;
    }

    return cRef;
}

STDMETHODIMP CBorderSizeHandler::QueryInterface(REFIID iid, void** ppv)
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