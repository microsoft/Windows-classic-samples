// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "PropertySet.h"

#include <windows.h>
#include <strsafe.h>
#include <uiribbonpropertyhelpers.h>

//
//  FUNCTION: InitializeCommandProperties()
//
//  PURPOSE: Initializes a property set for use with the ItemsSource property of a gallery of type "Commands."
//
//  COMMENTS:
//    This takes values that will be used when queried for the CategoryId, CommandId, and CommandType properties.
//
//
void CPropertySet::InitializeCommandProperties(int categoryId, int commandId, UI_COMMANDTYPE commandType)
{
    m_categoryId = categoryId;
    m_commandId = commandId;
    m_commandType = commandType;
}

//
//  FUNCTION: InitializeItemProperties()
//
//  PURPOSE: Initializes a property set for use with the ItemsSource property of any type of gallery other than "Commands."
//
//  COMMENTS:
//    This takes values that will be used when queried for the ItemImage, Label, and CategoryId properties.
//
//
void CPropertySet::InitializeItemProperties(IUIImage *image, __in PCWSTR label, int categoryId)
{
    m_pimgItem = image;
    if (m_pimgItem)
    {
        m_pimgItem->AddRef();
    }
    StringCchCopyW(m_wszLabel, MAX_RESOURCE_LENGTH, label);
    m_categoryId = categoryId;
}

//
//  FUNCTION: InitializeCategoryProperties()
//
//  PURPOSE: Initializes a property set for use with the Categories property of any gallery.
//
//  COMMENTS:
//    This takes values that will be used when queried for the Label and CategoryId properties.
//
//
void CPropertySet::InitializeCategoryProperties(__in PCWSTR label, int categoryId)
{
    StringCchCopyW(m_wszLabel, MAX_RESOURCE_LENGTH, label);
    m_categoryId = categoryId;
}

//
//  FUNCTION: GetValue(REFPROPERTYKEY, PROPVARIANT*)
//
//  PURPOSE: Retrieves the value of one of the properties used when adding something to a gallery.
//
//  COMMENTS:
//    This will be called by the framework on the property sets returned for the ItemsSource and 
//    Categories properties in order to create and display the contents of the gallery.
//
//
STDMETHODIMP CPropertySet::GetValue(__in REFPROPERTYKEY key, __out PROPVARIANT *ppropvar)
{
    if (key == UI_PKEY_ItemImage)
    {
        if (m_pimgItem)
        {
            return UIInitPropertyFromImage(UI_PKEY_ItemImage, m_pimgItem, ppropvar);
        }
        return S_FALSE;
    }
    else if (key == UI_PKEY_Label)
    {
        return UIInitPropertyFromString(UI_PKEY_Label, m_wszLabel, ppropvar);
    }
    else if (key == UI_PKEY_CategoryId)
    {
        return UIInitPropertyFromUInt32(UI_PKEY_CategoryId, m_categoryId, ppropvar);
    }
    else if (key == UI_PKEY_CommandId)
    {
        if(m_commandId != -1)
        {
            return UIInitPropertyFromUInt32(UI_PKEY_CommandId, m_commandId, ppropvar);
        }
        return S_FALSE;
    }
    else if (key == UI_PKEY_CommandType)
    {
        return UIInitPropertyFromUInt32(UI_PKEY_CommandType, m_commandType, ppropvar);
    }
    return E_FAIL;
}

HRESULT CPropertySet::CreateInstance(__deref_out CPropertySet **ppPropertySet)
{
    if (!ppPropertySet)
    {
        return E_POINTER;
    }

    *ppPropertySet = NULL;

    HRESULT hr = S_OK;

    CPropertySet* pPropertySet = new CPropertySet();

    if (pPropertySet == NULL)
    {
        hr = E_OUTOFMEMORY;
    }

    if (SUCCEEDED(hr))
    {
        *ppPropertySet = pPropertySet;
    }

    return hr;
}

// IUnknown methods.
STDMETHODIMP_(ULONG) CPropertySet::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) CPropertySet::Release()
{
    LONG cRef = InterlockedDecrement(&m_cRef);
    if (cRef == 0)
    {
        delete this;
    }

    return cRef;
}

STDMETHODIMP CPropertySet::QueryInterface(REFIID iid, void** ppv)
{
    if (!ppv)
    {
        return E_POINTER;
    }

    if (iid == __uuidof(IUnknown))
    {
        *ppv = static_cast<IUnknown*>(this);
    }
    else if (iid == __uuidof(IUISimplePropertySet))
    {
        *ppv = static_cast<IUISimplePropertySet*>(this);
    }
    else 
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}
