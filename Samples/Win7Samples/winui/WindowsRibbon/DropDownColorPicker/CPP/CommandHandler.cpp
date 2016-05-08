// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "Application.h"
#include "CommandHandler.h"

static COLORREF g_colors[] = 
{
    RGB(192, 0, 0),     // Dark red.
    RGB(255, 0, 0),     // Red.
    RGB(255, 192, 0),   // Gold.
    RGB(255, 255, 0),   // Yellow.
    RGB(146, 208, 80),  // Lime.
    RGB(0, 176, 80),    // Dark green.
    RGB(0, 176, 240),   // Turquoise.
    RGB(0, 112, 192),   // Dark blue.
    RGB(0, 32, 96),     // Dark blue.
    RGB(112, 48, 160)   // Purple.
};

// Static method to create an instance of this object.
__checkReturn HRESULT CColorPickerHandler::CreateInstance(__deref_out CColorPickerHandler **ppCommandHandler)
{
    if (!ppCommandHandler)
    {
        return E_POINTER;
    }

    *ppCommandHandler = NULL;

    HRESULT hr = E_FAIL;

    CColorPickerHandler* pHandler = new CColorPickerHandler();

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
}

STDMETHODIMP CColorPickerHandler::Execute(UINT nCmdID,
                       UI_EXECUTIONVERB verb, 
                       __in_opt const PROPERTYKEY* key,
                       __in_opt const PROPVARIANT* ppropvarValue,
                       __in_opt IUISimplePropertySet* pCommandExecutionProperties)
{
    UNREFERENCED_PARAMETER(nCmdID);
    UNREFERENCED_PARAMETER(key);

    HRESULT hr = S_OK;
    ColorProperty colorProp;
    UINT color = 0;
    UINT type = UI_SWATCHCOLORTYPE_NOCOLOR;
    
    switch(verb)
    {
    case UI_EXECUTIONVERB_EXECUTE:
    case UI_EXECUTIONVERB_PREVIEW:
    case UI_EXECUTIONVERB_CANCELPREVIEW:
        // The Ribbon framework passes color type as the primary property.
        if (ppropvarValue != NULL)
        {
            // Retrieve color type.
            hr = UIPropertyToUInt32(UI_PKEY_ColorType, *ppropvarValue, &type);
            if (FAILED(hr))
            {
                return hr;
            }
        }

        // The Ribbon framework passes color as additional property if the color type is RGB.
        if (type == UI_SWATCHCOLORTYPE_RGB && pCommandExecutionProperties != NULL)
        {
            // Retrieve color.
            PROPVARIANT var;
            hr = pCommandExecutionProperties->GetValue(UI_PKEY_Color, &var);
            if (FAILED(hr))
            {
                return hr;
            }
            UIPropertyToUInt32(UI_PKEY_Color, var, &color);
        }

        colorProp.color = (COLORREF)color;

        colorProp.type = (UI_SWATCHCOLORTYPE)type;

        // Handle the execution event.
        g_renderer.Execute(verb, colorProp);

        break;            
    }

    return hr;
}

STDMETHODIMP CColorPickerHandler::UpdateProperty(UINT nCmdID,
                              __in REFPROPERTYKEY key,
                              __in_opt const PROPVARIANT* ppropvarCurrentValue,
                              __out PROPVARIANT* ppropvarNewValue)
{
    UNREFERENCED_PARAMETER(ppropvarCurrentValue);

    HRESULT hr = S_OK;

    // Initialize the DDCP color type.
    if (key == UI_PKEY_ColorType)
    {
        hr = UIInitPropertyFromUInt32(key, UI_SWATCHCOLORTYPE_RGB, ppropvarNewValue);
        if (FAILED(hr))
        {
            return hr;
        }
    }
    // Initialize the default selected color for each DDCP.
    else if (key == UI_PKEY_Color)
    {
        switch (nCmdID)
        {
        case IDR_CMD_THEMEDDCP:
            hr = UIInitPropertyFromUInt32(key, RGB(0,255,0), ppropvarNewValue);
            break;
        case IDR_CMD_STANDARDDDCP:
            hr = UIInitPropertyFromUInt32(key, RGB(255,0,0), ppropvarNewValue);
            break;
        case IDR_CMD_HIGHLIGHTDDCP:
            hr = UIInitPropertyFromUInt32(key, RGB(0,0,255), ppropvarNewValue);
            break;
        }
        if (FAILED(hr))
        {
            return hr;
        }
    }

    // Customize standard DDCP.
    if (nCmdID == IDR_CMD_STANDARDDDCP)
    {
        if (key == UI_PKEY_StandardColors)
        {
            // Customize color chips in standard DDCP.
            ULONG rStandardColors[DDCP_WIDTH * DDCP_HEIGHT];

            for (LONG i = 0; i < DDCP_WIDTH * DDCP_HEIGHT; i++)
            {
                if (!m_fInitialized)
                {
                    // Default customized colors.
                    rStandardColors[i] = g_colors[i % 10];
                }
                else
                {
                    // Update color chips with the ones in client color grid.
                    rStandardColors[i] = g_renderer.GetColor(i);
                }
            }

            hr = InitPropVariantFromUInt32Vector(&rStandardColors[0], DDCP_WIDTH * DDCP_HEIGHT, ppropvarNewValue);
            if (FAILED(hr))
            {
                return hr;
            }

            m_fInitialized = TRUE;

            return hr;
        }
        // Customize color tooltips in standard DDCP.
        else if (key == UI_PKEY_StandardColorsTooltips)
        {
            // Return S_OK to let the API determine tooltips automatically.
            return S_OK;
        }
    }
    return hr;
}

// IUnknown methods.
STDMETHODIMP_(ULONG) CColorPickerHandler::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) CColorPickerHandler::Release()
{
    LONG cRef = InterlockedDecrement(&m_cRef);

    if (cRef == 0)
    {
        delete this;
    }

    return cRef;
}

STDMETHODIMP CColorPickerHandler::QueryInterface(REFIID iid, void** ppv)
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

// Static method to create an instance of this object.
__checkReturn HRESULT CButtonHandler::CreateInstance(__deref_out CButtonHandler **ppCommandHandler)
{
    if (!ppCommandHandler)
    {
        return E_POINTER;
    }

    *ppCommandHandler = NULL;

    HRESULT hr = E_FAIL;

    CButtonHandler* pHandler = new CButtonHandler();

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
}

STDMETHODIMP CButtonHandler::Execute(UINT nCmdID,
                       UI_EXECUTIONVERB verb, 
                       __in_opt const PROPERTYKEY* key,
                       __in_opt const PROPVARIANT* ppropvarValue,
                       __in_opt IUISimplePropertySet* pCommandExecutionProperties)
{
    UNREFERENCED_PARAMETER(key);
    UNREFERENCED_PARAMETER(ppropvarValue);
    UNREFERENCED_PARAMETER(pCommandExecutionProperties);

    HRESULT hr = S_OK;

    if (nCmdID == IDR_CMD_UPDATE && verb == UI_EXECUTIONVERB_EXECUTE)
    {
        // Invalidate customized standard DDCP to update the color chips.
        hr = g_pFramework->InvalidateUICommand(IDR_CMD_STANDARDDDCP, UI_INVALIDATIONS_PROPERTY, &UI_PKEY_StandardColors);
        if (SUCCEEDED(hr))
        {
            // Invalidate customized standard DDCP to update color tooltips.
            hr = g_pFramework->InvalidateUICommand(IDR_CMD_STANDARDDDCP, UI_INVALIDATIONS_PROPERTY, &UI_PKEY_StandardColorsTooltips);
        }
    }

    if (nCmdID == IDR_CMD_CLEAR && verb == UI_EXECUTIONVERB_EXECUTE)
    {
        // Clear color grid.
        g_renderer.ClearColorGrid();
    }

    return hr;
}

STDMETHODIMP CButtonHandler::UpdateProperty(UINT nCmdID,
                              __in REFPROPERTYKEY key,
                              __in_opt const PROPVARIANT* ppropvarCurrentValue,
                              __out PROPVARIANT* ppropvarNewValue)
{
    UNREFERENCED_PARAMETER(nCmdID);
    UNREFERENCED_PARAMETER(key);
    UNREFERENCED_PARAMETER(ppropvarCurrentValue);
    UNREFERENCED_PARAMETER(ppropvarNewValue);

    return S_OK;
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