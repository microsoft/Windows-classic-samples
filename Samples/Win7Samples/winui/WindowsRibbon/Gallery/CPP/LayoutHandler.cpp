// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "LayoutHandler.h"
#include "PropertySet.h"
#include "Renderer.h"
#include "Resource.h"
#include "RibbonFramework.h"
#include <uiribbonpropertyhelpers.h>

extern CRenderer   g_renderer;

//
//  FUNCTION: Execute()
//
//  PURPOSE: Called by the Ribbon framework when the user chooses a new layout.
//
//  COMMENTS:
//    This will update the layout of the shapes being drawn.
//
//
STDMETHODIMP CLayoutHandler::Execute(UINT nCmdID,
                   UI_EXECUTIONVERB verb, 
                   __in_opt const PROPERTYKEY* key,
                   __in_opt const PROPVARIANT* ppropvarValue,
                   __in_opt IUISimplePropertySet* pCommandExecutionProperties)
{
    UNREFERENCED_PARAMETER(nCmdID);
    UNREFERENCED_PARAMETER(pCommandExecutionProperties);

    HRESULT hr = E_FAIL;
    if (verb == UI_EXECUTIONVERB_EXECUTE)
    {
        if (key && *key == UI_PKEY_SelectedItem)
        {
            // Get the newly-selected layout and update the render parameters with it.

            RenderParam param;
            g_renderer.GetRenderParam(&param);

            UINT selected = ppropvarValue->uintVal;
            
            param.eViewLayout = (VIEW_LAYOUT)selected;
            g_renderer.UpdateRenderParam(param);

            hr = S_OK;
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
//    This function is used to initialize the contents and selection of the gallery.
//
//
STDMETHODIMP CLayoutHandler::UpdateProperty(UINT nCmdID,
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
            pCollection->Release();
            return hr;
        }

        int labelIds[] = {IDS_LAYOUT_1, IDS_LAYOUT_2, IDS_LAYOUT_3};

        // Populate the combobox with the three layout options.
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
  
            // Load the label from the resource file.
            WCHAR wszLabel[MAX_RESOURCE_LENGTH];
            LoadString(GetModuleHandle(NULL), labelIds[i], wszLabel, MAX_RESOURCE_LENGTH);

            // Initialize the property set with no image, the label that was just loaded, and no category.
            pItem->InitializeItemProperties(NULL, wszLabel, UI_COLLECTION_INVALIDINDEX);

            pCollection->Add(pItem);
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
        // Use the first layout as the default selection.
        hr = UIInitPropertyFromUInt32(UI_PKEY_SelectedItem, 0, ppropvarNewValue);
    }
    return hr;
}

HRESULT CLayoutHandler::CreateInstance(__deref_out CLayoutHandler **ppHandler)
{
    if (!ppHandler)
    {
        return E_POINTER;
    }

    *ppHandler = NULL;

    HRESULT hr = S_OK;

    CLayoutHandler* pHandler = new CLayoutHandler();

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
STDMETHODIMP_(ULONG) CLayoutHandler::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) CLayoutHandler::Release()
{
    LONG cRef = InterlockedDecrement(&m_cRef);
    if (cRef == 0)
    {
        delete this;
    }

    return cRef;
}

STDMETHODIMP CLayoutHandler::QueryInterface(REFIID iid, void** ppv)
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