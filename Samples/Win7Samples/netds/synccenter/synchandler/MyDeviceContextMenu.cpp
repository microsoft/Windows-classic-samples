//////////////////////////////////////////////////////////////////////////////
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//  Module Name:
//      MyDeviceContextMenu.cpp
//
//  Abstract:
//      Implementation of the context menu extension for the My Device Sync
//      Center handler collection.
//
//////////////////////////////////////////////////////////////////////////////

#include "Pch.h"
#include "MyDeviceContextMenu.h"
#include "Resources.h"
#include <propidl.h>  // For property system interfaces
#include <propkey.h>  // For property key definitions

//----------------------------------------------------------------------------
// Static Global Variables
//----------------------------------------------------------------------------

static const char  g_aszDisplayInfoVerb[] = "displayinfo";
static const WCHAR g_wszDisplayInfoVerb[] = L"displayinfo";

//////////////////////////////////////////////////////////////////////////////
// class CMyDeviceContextMenu
//////////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------------
//
//  Description:
//      Create an instance of the context menu class.
//
//  Parameters:
//      punkOuter       - Outer IUnknown for aggregation.
//      ppunk           - IUnknown interface pointer returned to caller.
//
//  Return Values:
//      S_OK            - Operation completed successfully.
//      E_OUTOFMEMORY   - Error allocating the object.
//      Other HRESULTs  - Error querying for requested interface.
//
//----------------------------------------------------------------------------
HRESULT CMyDeviceContextMenu_CreateInstance(
    __in_opt    IUnknown  *punkOuter,
    __deref_out IUnknown **ppunk)
{
    *ppunk = NULL;

    UNREFERENCED_PARAMETER(punkOuter);

    HRESULT hr = E_OUTOFMEMORY;
    CMyDeviceContextMenu *pcm = new CMyDeviceContextMenu();
    if (pcm != NULL)
    {
        hr = pcm->QueryInterface(IID_PPV_ARGS(ppunk));
        pcm->Release();
    }

    return hr;

} //*** CMyDeviceContextMenu_CreateInstance

//----------------------------------------------------------------------------
// IUnknown (CMyDeviceContextMenu)
//----------------------------------------------------------------------------

STDMETHODIMP CMyDeviceContextMenu::QueryInterface(__in REFIID riid, __deref_out void **ppv)
{
    static const QITAB qit[] =
    {
        QITABENT(CMyDeviceContextMenu, IShellExtInit),
        QITABENT(CMyDeviceContextMenu, IContextMenu),
        { 0 },
    };

    return QISearch(this, qit, riid, ppv);

} //*** CMyDeviceContextMenu::QueryInterface

//----------------------------------------------------------------------------

STDMETHODIMP_(ULONG) CMyDeviceContextMenu::Release()
{
    ULONG cRef = InterlockedDecrement(&_cRef);
    if (cRef == 0)
    {
        delete this;
    }
    return cRef;

} //*** CMyDeviceContextMenu::Release

//----------------------------------------------------------------------------
// IShellExtInit (CMyDeviceContextMenu)
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
//  Description:
//      Initialize the object for the Shell extension operation.
//
//  Parameters:
//      pidlFolder  - Structure that identifies the folder being extended.
//      pdtobj      - IDataObject providing info about selected items.
//      hkeyProgID  - Registry key for the folder.
//
//  Return Values:
//      S_OK        - Operation completed successfully.
//
//----------------------------------------------------------------------------
STDMETHODIMP CMyDeviceContextMenu::Initialize(
    __in PCIDLIST_ABSOLUTE   pidlFolder,
    __in IDataObject        *pdtobj,
    __in HKEY                hkeyProgID)
{
    UNREFERENCED_PARAMETER(pidlFolder);
    UNREFERENCED_PARAMETER(hkeyProgID);

    HRESULT hr = S_OK;
    // Hold our IDataObject.  We can use this to determine
    // which items are currently selected in the view.
    _pDataObject = pdtobj;
    _pDataObject->AddRef();

    return hr;

} //*** CMyDeviceContextMenu::Initialize

//----------------------------------------------------------------------------
// IContextMenu (CMyDeviceContextMenu)
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
//  Description:
//      Called by the Shell to allow the context menu handler to add items
//      to the context menu.
//
//  Parameters:
//      hMenu       - Handler to the menu to extend.
//      indexMenu   - Zero-based position at which to insert the first menu item.
//      idCmdFirst  - Minimum value that the handler can specify for a menu item identifier.
//      idCmdLast   - Maximum value that the handler can specify for a menu item identifier.
//      uFlags      - Flags specifying how the menu can be changed.
//
//  Return Values:
//      HRESULT value that has its severity value set to SEVERITY_SUCCESS and
//          its code value set to the largest command identifier that was
//          assigned, plus one.
//      HRESULT value - An error occurred.
//
//----------------------------------------------------------------------------
STDMETHODIMP CMyDeviceContextMenu::QueryContextMenu(
    __in HMENU  hMenu,
    __in UINT   indexMenu,
    __in UINT   idCmdFirst,
    __in UINT   idCmdLast,
    __in UINT   uFlags)
{
    HRESULT hr = S_OK;

    UNREFERENCED_PARAMETER(idCmdLast);
    // CMF_DEFAULTONLY is only relevant if you are trying
    // to change the default (double-click) action for the
    // selected item.  When the user double-clicks your item,
    // the shell will call QueryContextMenu with this flag
    // specified.
    if (!(uFlags & CMF_DEFAULTONLY))
    {
        WCHAR szMenuString[128];
        LoadStringW(g_hmodThisDll, IDS_MENU_DISPLAY_INFO, szMenuString, ARRAYSIZE(szMenuString));

        if (!InsertMenuW(hMenu, indexMenu, MF_STRING | MF_BYPOSITION, idCmdFirst + IDM_DISPLAY_INFO, szMenuString))
        {
            DWORD sc = GetLastError();
            hr = HRESULT_FROM_WIN32(sc);
        }
        else
        {
            hr = MAKE_HRESULT(SEVERITY_SUCCESS, 0, USHORT(IDM_DISPLAY_INFO + 1));
        }
    } // if: not just executing default action

    return hr;

} //*** CMyDeviceContextMenu::QueryContextMenu

//----------------------------------------------------------------------------
//
//  Description:
//      Return information about the items this handler adds to the menu.
//
//  Parameters:
//      idCommand   - Menu command identifier offset.
//      uFlags      - Flags specifying the information to return.
//      lpReserved  - Reserved.
//      pszName     - Address of the buffer to receive the string being requested.
//      uMaxNameLen - Size of the pszName buffer in characters.
//
//  Return Values:
//      S_OK            - Operation completed successfully.
//      E_INVALIDARG    - Command ID was unknown.
//      Other HRESULTS  - Operation failed.
//
//----------------------------------------------------------------------------
STDMETHODIMP CMyDeviceContextMenu::GetCommandString(
    __in                                                UINT_PTR idCommand,
    __in                                                UINT     uFlags,
    __reserved                                          LPUINT   lpReserved,
    __out_awcount(!(uFlags & GCS_UNICODE), uMaxNameLen) LPSTR    pszName,
    __in                                                UINT     uMaxNameLen)
{
    HRESULT hr = E_INVALIDARG;

    UNREFERENCED_PARAMETER(lpReserved);

    if (idCommand != IDM_DISPLAY_INFO)
    {
        hr = E_INVALIDARG;
    }
    else
    {
        switch (uFlags)
        {
            case GCS_HELPTEXTA:
                {
                    int cchLoaded = LoadStringA(g_hmodThisDll, IDS_MENU_DESC_DISPLAY_INFO, pszName, uMaxNameLen);
                    DWORD sc = GetLastError();
                    hr = (cchLoaded == 0) ? HRESULT_FROM_WIN32(sc) : S_OK;
                    break;
                }

            case GCS_HELPTEXTW:
                {
                    int cchLoaded = LoadStringW(g_hmodThisDll, IDS_MENU_DESC_DISPLAY_INFO, (LPWSTR) pszName, uMaxNameLen);
                    DWORD sc = GetLastError();
                    hr = (cchLoaded == 0) ? HRESULT_FROM_WIN32(sc) : S_OK;
                    break;
                }

            case GCS_VERBA:
                hr = StringCchCopyNA(pszName, uMaxNameLen, g_aszDisplayInfoVerb, ARRAYSIZE(g_aszDisplayInfoVerb));
                break;

            case GCS_VERBW:
                hr = StringCchCopyNW((LPWSTR) pszName, uMaxNameLen, g_wszDisplayInfoVerb, ARRAYSIZE(g_wszDisplayInfoVerb));
                break;

            case GCS_VALIDATEA:
            case GCS_VALIDATEW:
            default:
                hr = S_OK;
                break;
        } // switch: info requested
    } // else: command ID was recognized

    return hr;

} //*** CMyDeviceContextMenu::GetCommandString

//----------------------------------------------------------------------------
//
//  Description:
//      Carries out the command associated with a menu item added by this
//      context menu handler.
//
//  Parameters:
//      pici    - Structure containing information about the command.
//
//  Return Values:
//      S_OK            - Operation completed successfully.
//      Other HRESULTS  - Operation failed.
//
//----------------------------------------------------------------------------
STDMETHODIMP CMyDeviceContextMenu::InvokeCommand(__in LPCMINVOKECOMMANDINFO pici)
{
    HRESULT                  hr = S_OK;
    BOOL                     fUnicode  = FALSE;
    CMINVOKECOMMANDINFOEX   *picie = (CMINVOKECOMMANDINFOEX *) pici;

    // Determine if unicode was specified.
    if (pici->cbSize == sizeof(CMINVOKECOMMANDINFOEX))
    {
        if ((pici->fMask & CMIC_MASK_UNICODE) != 0)
        {
            fUnicode = TRUE;
        }
    }

    // Handle a verb.
    if (!fUnicode && (HIWORD(pici->lpVerb) != 0))
    {
        if (StrCmpIA(pici->lpVerb, g_aszDisplayInfoVerb) != 0)
        {
            hr = E_FAIL;
        }
    }
    else if (fUnicode && (HIWORD(picie->lpVerbW) != 0))
    {
        if (StrCmpIW(picie->lpVerbW, g_wszDisplayInfoVerb) != 0)
        {
            hr = E_FAIL;
        }
    }
    else if (LOWORD(pici->lpVerb) != IDM_DISPLAY_INFO)
    {
        hr = E_FAIL;
    }

    // Handle the command.  For demonstration purposes,
    // we'll just display a message box.
    if (SUCCEEDED(hr))
    {
        // We can retrieve properties for the selected handler/item
        // using the PKEYs specified in propkey.h.
        // Other keys that may be useful:
        // PKEY_Sync_HandlerCollectionID -- The Handler Collection ID
        // PKEY_Sync_HandlerID           -- The Handler ID
        // PKEY_Sync_ItemID              -- The Item ID
        PROPVARIANT propvar = {0};
        hr = _GetSelectedItemProperty(PKEY_ItemNameDisplay, &propvar);
        if (SUCCEEDED(hr))
        {
            WCHAR szMessage[128];
            hr = StringCchPrintfW(szMessage, ARRAYSIZE(szMessage), L"Extended Details For %s", propvar.pwszVal);
            if (SUCCEEDED(hr))
            {
                MessageBoxW(pici->hwnd, L"Partnership Info", szMessage, MB_OK | MB_ICONINFORMATION);
            }
            PropVariantClear(&propvar);
        }
    }

    return hr;

} //*** CMyDeviceContextMenu::InvokeCommand

//----------------------------------------------------------------------------
//
//  Description:
//      Gets the specified property for the items that the context menu
//      is currently being invoked on.
//
//  Parameters:
//      pkey     - Key for the property we are looking for.
//      pPropVar - Address of the propvariant that recieves the property.
//
//  Return Values:
//      S_OK            - Operation completed successfully.
//      Other HRESULTS  - Operation failed.
//
//----------------------------------------------------------------------------
HRESULT CMyDeviceContextMenu::_GetSelectedItemProperty(__in REFPROPERTYKEY pkey, __out PROPVARIANT *pPropVar)
{
    // Determine exactly what object the context menu is being
    // displayed for.
    IShellItemArray *pItemArray = NULL;
    HRESULT hr = SHCreateShellItemArrayFromDataObject(_pDataObject, IID_PPV_ARGS(&pItemArray));
    if (SUCCEEDED(hr))
    {
        // Sync Center will only ask for the context menu
        // for a single item.  We can just get item 0 in this case.
        IShellItem *psi = NULL;
        hr = pItemArray->GetItemAt(0, &psi);
        if (SUCCEEDED(hr))
        {
            // IShellItem2 allows us to retrieve the property
            // store for the item.  From here, we can query
            // for properties that appear in the view.
            IShellItem2 *psi2 = NULL;
            hr = psi->QueryInterface(IID_PPV_ARGS(&psi2));
            if (SUCCEEDED(hr))
            {
                IPropertyStore *pps = NULL;
                hr = psi2->GetPropertyStore(GPS_DEFAULT, IID_PPV_ARGS(&pps));
                if (SUCCEEDED(hr))
                {
                    hr = pps->GetValue(pkey, pPropVar);
                    pps->Release();
                }
                psi2->Release();
            }
           psi->Release();
        }
        pItemArray->Release();
    }

    return hr;

} //*** CMyDeviceContextMenu::_GetSelectedItemProperty
