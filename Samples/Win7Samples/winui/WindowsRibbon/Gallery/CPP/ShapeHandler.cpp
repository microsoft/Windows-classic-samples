// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "ShapeHandler.h"
#include "PropertySet.h"
#include "Renderer.h"
#include "Resource.h"
#include "RibbonFramework.h"
#include <uiribbonpropertyhelpers.h>

extern CRenderer   g_renderer;

//
//  FUNCTION: Execute()
//
//  PURPOSE: Called by the Ribbon framework when the user selects or hovers over a new shape.
//
//  COMMENTS:
//    This will update the type of shape being displayed.
//
//
STDMETHODIMP CShapeHandler::Execute(UINT nCmdID,
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

    UINT selected;
    hr = UIPropertyToUInt32(*key, *ppropvarValue, &selected);
    switch (verb)
    {
    case UI_EXECUTIONVERB_PREVIEW:
        // Show a preview of a new shape.    
        param.eShapeType = (SHAPE_TYPE)selected;
        g_renderer.UpdateRenderParam(param);
        hr = S_OK;
        break;
    case UI_EXECUTIONVERB_CANCELPREVIEW:
        // Show the shape that was selected before the preview- ppropvarValue contains the previous selected item.
        // Note that the renderer did not have to store the value from before preview was called.
        param.eShapeType = (SHAPE_TYPE)selected;
        g_renderer.UpdateRenderParam(param);
        hr = S_OK;
        break;
    case UI_EXECUTIONVERB_EXECUTE:
        if ( key && *key == UI_PKEY_SelectedItem)
        {      
            // Update the renderer with the newly-selected shape.
            param.eShapeType = (SHAPE_TYPE)selected;
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
STDMETHODIMP CShapeHandler::UpdateProperty(UINT nCmdID,
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

        int imageIds[4];
        int labelIds[] = {IDS_RECTANGLE, IDS_ELLIPSE, IDS_ROUNDED_RECTANGLE, IDS_DIAMOND};

        int dpi = GetDeviceCaps(GetDC(NULL), LOGPIXELSX);
        if (dpi > 144)
        {
            imageIds[0] = IDB_RECTANGLE_192;
            imageIds[1] = IDB_ELLIPSE_192;
            imageIds[2] = IDB_ROUNDED_RECTANGLE_192;
            imageIds[3] = IDB_DIAMOND_192;
        }
        else if (dpi > 120)
        {
            imageIds[0] = IDB_RECTANGLE_144;
            imageIds[1] = IDB_ELLIPSE_144;
            imageIds[2] = IDB_ROUNDED_RECTANGLE_144;
            imageIds[3] = IDB_DIAMOND_144;
        }
        else if (dpi > 96)
        {
            imageIds[0] = IDB_RECTANGLE_120;
            imageIds[1] = IDB_ELLIPSE_120;
            imageIds[2] = IDB_ROUNDED_RECTANGLE_120;
            imageIds[3] = IDB_DIAMOND_120;
        }
        else
        {
            imageIds[0] = IDB_RECTANGLE_96;
            imageIds[1] = IDB_ELLIPSE_96;
            imageIds[2] = IDB_ROUNDED_RECTANGLE_96;
            imageIds[3] = IDB_DIAMOND_96;
        }

        // Populate the gallery with the four available shape types.
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

            pImg->Release();
            pItem->Release();
        }
        pCollection->Release();
        hr = S_OK;
    }
    else if (key == UI_PKEY_SelectedItem)
    {
        // Use the current shape as the selection.
        RenderParam param;
        g_renderer.GetRenderParam(&param);
        hr = UIInitPropertyFromUInt32(UI_PKEY_SelectedItem, param.eShapeType, ppropvarNewValue);
    }
    return hr;
}

// Factory method to create IUIImages from resource identifiers.
HRESULT CShapeHandler::CreateUIImageFromBitmapResource(LPCTSTR pszResource, __out IUIImage **ppimg)
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

HRESULT CShapeHandler::CreateInstance(__deref_out CShapeHandler **ppHandler)
{
    if (!ppHandler)
    {
        return E_POINTER;
    }
    
    *ppHandler = NULL;

    HRESULT hr = S_OK;

    CShapeHandler* pHandler = new CShapeHandler();

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
STDMETHODIMP_(ULONG) CShapeHandler::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) CShapeHandler::Release()
{
    LONG cRef = InterlockedDecrement(&m_cRef);
    if (cRef == 0)
    {
        delete this;
    }

    return cRef;
}

STDMETHODIMP CShapeHandler::QueryInterface(REFIID iid, void** ppv)
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