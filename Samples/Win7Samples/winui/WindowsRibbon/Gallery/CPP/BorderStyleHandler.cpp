// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "BorderStyleHandler.h"
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
//  PURPOSE: Called by the Ribbon framework when the user selects a new border style.
//
//  COMMENTS:
//    This sets a new style for the border around the shapes being drawn.
//
//
STDMETHODIMP CBorderStyleHandler::Execute(UINT nCmdID,
                   UI_EXECUTIONVERB verb, 
                   __in_opt const PROPERTYKEY* key,
                   __in_opt const PROPVARIANT* ppropvarValue,
                   __in_opt IUISimplePropertySet* pCommandExecutionProperties)
{
    UNREFERENCED_PARAMETER(nCmdID);
    UNREFERENCED_PARAMETER(pCommandExecutionProperties);

    HRESULT hr = E_FAIL;

    RenderParam param;
    g_renderer.GetRenderParam(&param);

    if (verb == UI_EXECUTIONVERB_EXECUTE)
    {
        if (ppropvarValue == NULL) // The Button part of the Border Style SplitButtonGallery was clicked.
        {
            if (param.eBorderStyle == NONE)
            {
                param.eBorderStyle = SOLID;
            }
            else
            {
                param.eBorderStyle = NONE;
            }
            g_renderer.UpdateRenderParam(param);
            // Update the visual state of the button- toggled on for border selected, off for no border.
            g_pFramework->InvalidateUICommand(IDR_CMD_BORDERSTYLES, UI_INVALIDATIONS_PROPERTY, &UI_PKEY_BooleanValue);
            // The Border Sizes combobox needs to be re-enabled if the dash border was just de-selected.
            g_pFramework->InvalidateUICommand(IDR_CMD_BORDERSIZES, UI_INVALIDATIONS_PROPERTY, &UI_PKEY_Enabled);
            hr = S_OK;
        }
        else if ( key && *key == UI_PKEY_SelectedItem)
        {      
            UINT selected;
            hr = UIPropertyToUInt32(*key, *ppropvarValue, &selected);

            param.eBorderStyle = (BORDER_STYLE)selected;
            g_renderer.UpdateRenderParam(param);
            // The Border Sizes combobox needs to be disabled if dash border was just selected, or enabled otherwise.
            g_pFramework->InvalidateUICommand(IDR_CMD_BORDERSIZES, UI_INVALIDATIONS_PROPERTY, &UI_PKEY_Enabled);
            g_pFramework->InvalidateUICommand(IDR_CMD_BORDERSTYLES, UI_INVALIDATIONS_PROPERTY, &UI_PKEY_BooleanValue);
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
//    Depending on the value of key, this will populate the gallery, update the selection, or 
//    toggle the state of the button.
//
//
STDMETHODIMP CBorderStyleHandler::UpdateProperty(UINT nCmdID,
                              __in REFPROPERTYKEY key,
                              __in_opt const PROPVARIANT* ppropvarCurrentValue,
                              __out PROPVARIANT* ppropvarNewValue)
{
    UNREFERENCED_PARAMETER(nCmdID);

    HRESULT hr = E_FAIL;

    if(key == UI_PKEY_Categories)
    {
        // A return value of S_FALSE or E_NOTIMPL will result in a gallery with no categories.
        // If you return any error other than E_NOTIMPL, the contents of the gallery will not display.
        hr = S_FALSE;
    }
    else if (key == UI_PKEY_ItemsSource)
    {
        IUICollection* pCollection;
        hr = ppropvarCurrentValue->punkVal->QueryInterface(IID_PPV_ARGS(&pCollection));
        if (FAILED(hr))
        {
            return hr;
        }

        int imageIds[3];
        int labelIds[] = {IDS_BORDER_NONE, IDS_BORDER_SOLID, IDS_BORDER_DASH};

        int dpi = GetDeviceCaps(GetDC(NULL), LOGPIXELSX);
        if (dpi > 144)
        {
            imageIds[0] = IDB_NONE_192;
            imageIds[1] = IDB_SOLID_192;
            imageIds[2] = IDB_DASH_192;
        }
        else if (dpi > 120)
        {
            imageIds[0] = IDB_NONE_144;
            imageIds[1] = IDB_SOLID_144;
            imageIds[2] = IDB_DASH_144;
        }
        else if (dpi > 96)
        {
            imageIds[0] = IDB_NONE_120;
            imageIds[1] = IDB_SOLID_120;
            imageIds[2] = IDB_DASH_120;
        }
        else
        {
            imageIds[0] = IDB_NONE_96;
            imageIds[1] = IDB_SOLID_96;
            imageIds[2] = IDB_DASH_96;
        }

        // Populate the dropdown with the three border styles.
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

            // Create an IUIImage from a resource id.
            IUIImage* pImg;
            hr = CreateUIImageFromBitmapResource(MAKEINTRESOURCE(imageIds[i]), &pImg);
            if (FAILED(hr))
            {
                pCollection->Release();
                pItem->Release();
                return hr;
            }

            // Load the label from the resource file.
            WCHAR wszLabel[MAX_RESOURCE_LENGTH];
            LoadString(GetModuleHandle(NULL), labelIds[i], wszLabel, MAX_RESOURCE_LENGTH);

            // Initialize the property set with the image and label that were just loaded and no category.
            pItem->InitializeItemProperties(pImg, wszLabel, UI_COLLECTION_INVALIDINDEX);

            // Add the newly-created property set to the collection supplied by the framework.
            pCollection->Add(pItem);

            pItem->Release();
            pImg->Release();
        }
        pCollection->Release();
        hr = S_OK;
    }
    else if (key == UI_PKEY_SelectedItem)
    {
        // Use the current border style as the selection.
        RenderParam param;
        g_renderer.GetRenderParam(&param);
     
        hr = UIInitPropertyFromUInt32(UI_PKEY_SelectedItem, param.eBorderStyle, ppropvarNewValue);
    }
    else if (key == UI_PKEY_BooleanValue)
    {
        // The button will appear selected if there is a border, or deselected if there is no border.
        RenderParam param;
        g_renderer.GetRenderParam(&param);

        BOOL active = param.eBorderStyle != NONE;

        g_pFramework->InvalidateUICommand(IDR_CMD_BORDERSTYLES, UI_INVALIDATIONS_PROPERTY, &UI_PKEY_SelectedItem);
        hr = UIInitPropertyFromBoolean(UI_PKEY_BooleanValue, active, ppropvarNewValue);
    }
    return hr;
}


// Factory method to create IUIImages from resource identifiers.
HRESULT CBorderStyleHandler::CreateUIImageFromBitmapResource(LPCTSTR pszResource, __out IUIImage **ppimg)
{
    HRESULT hr = E_FAIL;

    *ppimg = NULL;

    if (NULL == m_pifbFactory)
    {
        hr = CoCreateInstance(CLSID_UIRibbonImageFromBitmapFactory, NULL, CLSCTX_ALL, IID_PPV_ARGS(&m_pifbFactory));
        if (FAILED(hr))
        {
            return hr;
        }
    }

    // Load the bitmap from the resource file.
    HBITMAP hbm = (HBITMAP) LoadImage(GetModuleHandle(NULL), pszResource, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
    if (hbm)
    {
        // Use the factory implemented by the framework to produce an IUIImage.
        hr = m_pifbFactory->CreateImage(hbm, UI_OWNERSHIP_TRANSFER, ppimg);
        if (FAILED(hr))
        {
            DeleteObject(hbm);
        }
    }
    return hr;
}

HRESULT CBorderStyleHandler::CreateInstance(__deref_out CBorderStyleHandler **ppHandler)
{
    if (!ppHandler)
    {
        return E_POINTER;
    }

    *ppHandler = NULL;

    HRESULT hr = S_OK;

    CBorderStyleHandler* pHandler = new CBorderStyleHandler();

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
STDMETHODIMP_(ULONG) CBorderStyleHandler::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) CBorderStyleHandler::Release()
{
    LONG cRef = InterlockedDecrement(&m_cRef);
    if (cRef == 0)
    {
        delete this;
    }

    return cRef;
}

STDMETHODIMP CBorderStyleHandler::QueryInterface(REFIID iid, void** ppv)
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