// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "SizeAndColorHandler.h"
#include "PropertySet.h"
#include "Renderer.h"
#include "Resource.h"
#include "RibbonFramework.h"
#include "ids.h"

extern CRenderer   g_renderer;

//
//  FUNCTION: Execute()
//
//  PURPOSE: Called by the Ribbon framework when the user executes an action on the size and color gallery.
//
//  COMMENTS:
//    When a button in a Commands gallery is clicked, the Execute events are sent to the handler for the 
//    button, not the handler for the gallery. This function will never actually be called.
//
//
STDMETHODIMP CSizeAndColorHandler::Execute(UINT nCmdID,
                   UI_EXECUTIONVERB verb, 
                   __in_opt const PROPERTYKEY* key,
                   __in_opt const PROPVARIANT* ppropvarValue,
                   __in_opt IUISimplePropertySet* pCommandExecutionProperties)
{
    UNREFERENCED_PARAMETER(nCmdID);
    UNREFERENCED_PARAMETER(verb);
    UNREFERENCED_PARAMETER(key);
    UNREFERENCED_PARAMETER(ppropvarValue);
    UNREFERENCED_PARAMETER(pCommandExecutionProperties);

    return E_FAIL;
}

//
//  FUNCTION: UpdateProperty()
//
//  PURPOSE: Called by the Ribbon framework when a command property (PKEY) needs to be updated.
//
//  COMMENTS:
//
//    This function is used to populate the gallery.
//
//
STDMETHODIMP CSizeAndColorHandler::UpdateProperty(UINT nCmdID,
                              __in REFPROPERTYKEY key,
                              __in_opt const PROPVARIANT* ppropvarCurrentValue,
                              __out PROPVARIANT* ppropvarNewValue)
{
    UNREFERENCED_PARAMETER(nCmdID);
    UNREFERENCED_PARAMETER(ppropvarNewValue);

    HRESULT hr = E_FAIL;

    if(key == UI_PKEY_Categories)
    {
        IUICollection* pCollection;
        hr = ppropvarCurrentValue->punkVal->QueryInterface(IID_PPV_ARGS(&pCollection));
        if (FAILED(hr))
        {
            return hr;
        }

        // Create a property set for the Size category.
        CPropertySet *pSize;
        hr = CPropertySet::CreateInstance(&pSize);
        if (FAILED(hr))
        {
            pCollection->Release();
            return hr;
        }

        // Load the label for the Size category from the resource file.
        WCHAR wszSizeLabel[MAX_RESOURCE_LENGTH];
        LoadString(GetModuleHandle(NULL), IDS_SIZE_CATEGORY, wszSizeLabel, MAX_RESOURCE_LENGTH);

        // Initialize the property set with the label that was just loaded and a category id of 0.
        pSize->InitializeCategoryProperties(wszSizeLabel, 0);

        // Add the newly-created property set to the collection supplied by the framework.
        pCollection->Add(pSize);

        pSize->Release();


        // Create a property set for the Color category.
        CPropertySet *pColor;
        hr = CPropertySet::CreateInstance(&pColor);
        if (FAILED(hr))
        {
            pCollection->Release();
            return hr;
        }

        // Load the label for the Color category from the resource file.
        WCHAR wszColorLabel[MAX_RESOURCE_LENGTH];
        LoadString(GetModuleHandle(NULL), IDS_COLOR_CATEGORY, wszColorLabel, MAX_RESOURCE_LENGTH);

        // Initialize the property set with the label that was just loaded and a category id of 1.
        pColor->InitializeCategoryProperties(wszColorLabel, 1);
        
        // Add the newly-created property set to the collection supplied by the framework.
        pCollection->Add(pColor);

        pColor->Release();
        pCollection->Release();

        hr = S_OK;
    }
    else if(key == UI_PKEY_ItemsSource)
    {
        IUICollection* pCollection;
        hr = ppropvarCurrentValue->punkVal->QueryInterface(IID_PPV_ARGS(&pCollection));
        if (FAILED(hr))
        {
            return hr;
        }

        int commandIds[] = {IDR_CMD_SMALL, IDR_CMD_MEDIUM, IDR_CMD_LARGE, IDR_CMD_RED, IDR_CMD_GREEN, IDR_CMD_BLUE};
        int categoryIds[] = {0, 0, 0, 1, 1, 1};

        // Populate the gallery with the three size and three colors in two separate categories.
        for (int i=0; i<_countof(commandIds); i++)
        {
            // Create a new property set for each item.
            CPropertySet* pCommand;
            hr = CPropertySet::CreateInstance(&pCommand);
            if (FAILED(hr))
            {
                pCollection->Release();
                return hr;
            }

            // Initialize the property set with the appropriate command id and category id and type Boolean (which makes these appear as ToggleButtons).
            pCommand->InitializeCommandProperties(categoryIds[i], commandIds[i], UI_COMMANDTYPE_BOOLEAN);

            // Add the newly-created property set to the collection supplied by the framework.
            pCollection->Add(pCommand);

            pCommand->Release();
        }
        pCollection->Release();
        hr = S_OK;
    }
    return hr;
}

HRESULT CSizeAndColorHandler::CreateInstance(__deref_out CSizeAndColorHandler **ppHandler)
{
    if (!ppHandler)
    {
        return E_POINTER;
    }

    *ppHandler = NULL;

    HRESULT hr = S_OK;

    CSizeAndColorHandler* pHandler = new CSizeAndColorHandler();

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
STDMETHODIMP_(ULONG) CSizeAndColorHandler::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) CSizeAndColorHandler::Release()
{
    LONG cRef = InterlockedDecrement(&m_cRef);
    if (cRef == 0)
    {
        delete this;
    }

    return cRef;
}

STDMETHODIMP CSizeAndColorHandler::QueryInterface(REFIID iid, void** ppv)
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