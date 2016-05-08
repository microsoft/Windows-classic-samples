//***************************************************************************
//    THIS CODE AND INFORMATION IS PROVIDED 'AS IS' WITHOUT WARRANTY OF
//    ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//    THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//    PARTICULAR PURPOSE.
//
//    Copyright Microsoft Corporation. All Rights Reserved.
//***************************************************************************

//***************************************************************************
//
//    File:          PropSheetHost.cpp
//
//    Description:   
//
//***************************************************************************

//***************************************************************************
//    #include statements
//***************************************************************************

#include "stdafx.h"

/***************************************************************************

    CPropSheetHost::CPropSheetHost()

***************************************************************************/

CPropSheetHost::CPropSheetHost(HINSTANCE hInstance, 
                               HWND hwndParent)
{
    m_hInst = hInstance;
    m_hwndParent = hwndParent;
    m_hwndHidden = NULL;
    m_ObjRefCount = 0;

    m_cfDSPropSheetConfig = RegisterClipboardFormat(CFSTR_DS_PROPSHEETCONFIG);
    m_cfDSObjectNames = RegisterClipboardFormat(CFSTR_DSOBJECTNAMES);
    m_cfDSDispSpecOptions = RegisterClipboardFormat(CFSTR_DSDISPLAYSPECOPTIONS);

    m_szHiddenWindowClass = TEXT("CPropSheetHostHiddenWindowClass");

#if 1
    m_pwszPrefix = PROP_SHEET_PREFIX_ADMIN;
#else
    m_pwszPrefix = PROP_SHEET_PREFIX_SHELL;
#endif
}

/***************************************************************************

    CPropSheetHost::~CPropSheetHost()

***************************************************************************/

CPropSheetHost::~CPropSheetHost()
{
}

/***************************************************************************

    CPropSheetHost::SetObject()

***************************************************************************/

HRESULT CPropSheetHost::SetObject(LPCWSTR pwszADsPath)
{
    CComPtr<IADs> spADsTemp;
    HRESULT hr = ADsGetObject(pwszADsPath, IID_IADs, (LPVOID*)&spADsTemp);
    if(FAILED(hr))
    {
        return hr;
    }

    return SetObject(spADsTemp);
}

/***************************************************************************

    CPropSheetHost::SetObject()

***************************************************************************/

HRESULT CPropSheetHost::SetObject(IADs *pads)
{
    HRESULT hr = pads->QueryInterface(IID_IADs, (LPVOID*)&m_spADObject);

    return hr;
}

/***************************************************************************

    CPropSheetHost::Run()

***************************************************************************/

void CPropSheetHost::Run()
{
    if(!m_spADObject.p)
    {
        return;
    }

    // Create the hidden window.
    m_hwndHidden = _CreateHiddenWindow();
    if(!m_hwndHidden)
    {
        return;
    }

    /*
    Display the proeprty sheet. This is a modal call and will not return 
    until the property sheet is dimissed.
    */
    _CreatePropertySheet();

    // Destroy the hidden window.
    DestroyWindow(m_hwndHidden);
}

/***************************************************************************

    CPropSheetHost::_CreateHiddenWindow()

***************************************************************************/

HWND CPropSheetHost::_CreateHiddenWindow()
{
    WNDCLASS  wc;

    if(!GetClassInfo(m_hInst, m_szHiddenWindowClass, &wc))
    {
        ZeroMemory(&wc, sizeof(wc));
           
        wc.style          = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc    = (WNDPROC)_HiddenWindowProc;
        wc.cbClsExtra     = 0;
        wc.cbWndExtra     = sizeof(CPropSheetHost*);
        wc.hInstance      = m_hInst;
        wc.hCursor        = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground  = (HBRUSH)(COLOR_WINDOW + 1);
        wc.lpszClassName  = m_szHiddenWindowClass;

        if(!RegisterClass(&wc))
        {
            return NULL;
        }
    }

    m_hwndHidden = CreateWindowEx(  0,
                                    m_szHiddenWindowClass,
                                    NULL,
                                    WS_OVERLAPPED |
                                        0,
                                    CW_USEDEFAULT,
                                    CW_USEDEFAULT,
                                    CW_USEDEFAULT,
                                    CW_USEDEFAULT,
                                    NULL,
                                    NULL,
                                    m_hInst,
                                    (LPVOID)this);

    return m_hwndHidden;
}

/***************************************************************************

    CPropSheetHost::_HiddenWindowProc()

***************************************************************************/

LRESULT CALLBACK CPropSheetHost::_HiddenWindowProc( HWND hWnd,
                                                    UINT uMessage,
                                                    WPARAM wParam,
                                                    LPARAM lParam)
{
    CPropSheetHost *pThis = (CPropSheetHost*)((LONG_PTR)GetWindowLongPtr(hWnd, VIEW_POINTER_OFFSET));

    switch (uMessage)
    {
    case WM_NCCREATE:
        {
            LPCREATESTRUCT lpcs = (LPCREATESTRUCT)lParam;
            pThis = (CPropSheetHost*)(lpcs->lpCreateParams);
            ::SetWindowLongPtr(hWnd, VIEW_POINTER_OFFSET, (LONG)(LONG_PTR)pThis);
        }
        break;

    case WM_CREATE:
        break;

    case WM_DESTROY:
        break;

    case WM_ADSPROP_NOTIFY_CHANGE:
        OutputDebugString(TEXT("WM_ADSPROP_NOTIFY_CHANGE\n"));
        break;

    case WM_DSA_SHEET_CREATE_NOTIFY:
        {
            PDSA_SEC_PAGE_INFO pSecPageInfo;
            
            // Extract the secondary sheet information from the wParam.
            if(S_OK == pThis->_ExtractSecPageInfo(wParam, &pSecPageInfo))
            {
                // Create a secondary property sheet.
                pThis->_CreateSecondaryPropertySheet(pSecPageInfo);
            }
            else
            {
                // Even if the extraction failed, the wParam needs to be freed.
                pSecPageInfo = (PDSA_SEC_PAGE_INFO)wParam;
            }

            /*
            The receiver of the  message must free the DSA_SEC_PAGE_INFO 
            structure when it is no longer needed.
            */
            LocalFree(pSecPageInfo);
        }
        return 0;

    case WM_DSA_SHEET_CLOSE_NOTIFY:
        if(PROP_SHEET_HOST_ID == wParam)
        {
            OutputDebugString(TEXT("PROP_SHEET_HOST_ID\n"));
        }
        return 0;

    default:
        break;
    }

    return DefWindowProc(hWnd, uMessage, wParam, lParam);
}

/***************************************************************************

    CPropSheetHost::_AddPagesForObject()

    This method will add all of the property pages for the specified object 
    by performing the following steps:
    
    1. Create an instance of the CLSID_DsPropertyPages object.
    
    2. Initialize the CLSID_DsPropertyPages object.
    
    3. Call the IShellPropSheetExt::AddPages method of the 
    CLSID_DsPropertyPages object. The CLSID_DsPropertyPages will enumerate 
    the admin property sheet extensions, create and initialize each 
    extension object and call the extension's IShellPropSheetExt::AddPages 
    method to cause the extension to add its pages to the property sheet.

***************************************************************************/

HRESULT CPropSheetHost::_AddPagesForObject(IADs *padsObject)
{
    HRESULT hr;
    
    // Get a copy of our IDataObject.
    CComPtr<IDataObject> spDataObject;
    hr = this->QueryInterface(IID_IDataObject, (LPVOID*)&spDataObject);
    if(FAILED(hr))
    {
        return hr;
    }

    // Create the DS property pages object.
    CComPtr<IShellExtInit> spExtInit;
    hr = spExtInit.CoCreateInstance(CLSID_DsPropertyPages);
    if(FAILED(hr))
    {
        return hr;
    }

    // Initialize the  DS property pages object.
    hr = spExtInit->Initialize(NULL, spDataObject, NULL);
    if(FAILED(hr))
    {
        return hr;
    }

    CComPtr<IShellPropSheetExt> spPropSheet;
    hr = spExtInit->QueryInterface(IID_IShellPropSheetExt, (LPVOID*)&spPropSheet);
    if(FAILED(hr))
    {
        return hr;
    }

    hr = spPropSheet->AddPages(_AddPagesCallback, (LPARAM)this);
    if(FAILED(hr))
    {
        return hr;
    }

    return hr;
}

/***************************************************************************

    CPropSheetHost::_CreatePropertySheet()

***************************************************************************/

void CPropSheetHost::_CreatePropertySheet()
{
    if(!m_spADObject.p)
    {
        return;
    }
    
    HRESULT hr;

    hr = _AddPagesForObject(m_spADObject);
    if(FAILED(hr))
    {
        return;
    }

    /*
    The property page handle array should have been filled in _AddPagesForObject 
    when AddPages was called.
    */
    if(m_rgPageHandles.GetSize() == 0)
    {
        return;
    }

    // Create the caption for the property sheet.
    CComBSTR sbstrTemp;
    hr = m_spADObject->get_Name(&sbstrTemp);
    if(S_OK == hr)
    {
        sbstrTemp += " ";
    }
    else
    {
        sbstrTemp = "";
    }
    sbstrTemp += m_pwszPrefix;
    sbstrTemp += " Properties";

    // Initialize the property sheet.
    PROPSHEETHEADER psh;

    ZeroMemory(&psh, sizeof(psh));

    USES_CONVERSION;

    // Fill out the PROPSHEETHEADER structure.
    psh.dwSize           = sizeof(PROPSHEETHEADER);
    psh.dwFlags          = PSH_DEFAULT;
    psh.hwndParent       = m_hwndParent;
    psh.hInstance        = NULL;
    psh.pszIcon          = NULL;
    psh.pszCaption       = W2T(sbstrTemp);
    psh.nPages           = (UINT)m_rgPageHandles.GetSize();
    psh.phpage           = m_rgPageHandles.GetData();
    psh.pfnCallback      = NULL;

    // Display the property sheet.
    PropertySheet(&psh);

    // Remove all page handles from the array.
    m_rgPageHandles.RemoveAll();
}

/***************************************************************************

    CPropSheetHost::_AddPagesCallback()

***************************************************************************/

BOOL CALLBACK CPropSheetHost::_AddPagesCallback(HPROPSHEETPAGE hPage, LPARAM lParam)
{
    CPropSheetHost *pThis = (CPropSheetHost*)lParam;
    
    pThis->m_rgPageHandles.Add(hPage);

    return TRUE;
}

/***************************************************************************

    CPropSheetHost::_ExtractSecPageInfo()

***************************************************************************/

HRESULT CPropSheetHost::_ExtractSecPageInfo(WPARAM wParam, PDSA_SEC_PAGE_INFO *ppSecPageInfo)
{
    if(!wParam || !ppSecPageInfo)
    {
        return E_INVALIDARG;
    }

    OSVERSIONINFO osvi;

    osvi.dwOSVersionInfoSize = sizeof(osvi);
    if(GetVersionEx(&osvi))
    {
        // This functionality isn't supported prior to Windows 2000.
        if((osvi.dwPlatformId != VER_PLATFORM_WIN32_NT) || (osvi.dwMajorVersion < 5))
        {
            return E_UNEXPECTED;
        }
        
        /*
        Special case for Windows 2000. Need to insert an HWND member at the front 
        of the DSA_SEC_PAGE_INFO structure.
        */
        if((5 == osvi.dwMajorVersion) && (0 == osvi.dwMinorVersion))
        {
            SIZE_T sizeOldSize = LocalSize((HLOCAL)wParam);
            DWORD dwOffset = sizeof(HWND);

            *ppSecPageInfo = (PDSA_SEC_PAGE_INFO)LocalAlloc(GPTR, sizeOldSize + sizeof(HWND));
            if(!*ppSecPageInfo)
            {
                return E_OUTOFMEMORY;
            }

            LPBYTE pDest = (LPBYTE)*ppSecPageInfo;
            LPBYTE pSource = (LPBYTE)wParam;

            // Copy the original memory to the new block.
            CopyMemory(pDest + dwOffset, pSource, sizeOldSize);

            // Need to update the title offset.
            (*ppSecPageInfo)->offsetTitle += dwOffset;

            // Free the old memory.
            LocalFree((HLOCAL)wParam);
        }
        else
        {
            // Just return the original memory.
            *ppSecPageInfo = (PDSA_SEC_PAGE_INFO)wParam;
        }

        return S_OK;
    }

    return E_FAIL;
}

/***************************************************************************

    CPropSheetHost::_CreateSecondaryPropertySheet()

***************************************************************************/

void CPropSheetHost::_CreateSecondaryPropertySheet(DSA_SEC_PAGE_INFO *pDSASecPageInfo)
{
    LPDSOBJECT pdsObject = &pDSASecPageInfo->dsObjectNames.aObjects[0];
    
    LPWSTR pwszTitle = (LPWSTR)((LPBYTE)pDSASecPageInfo + pDSASecPageInfo->offsetTitle);
    LPWSTR pwszClass = (LPWSTR)((LPBYTE)pdsObject + pdsObject->offsetClass);

    // Create a new property sheet host object.
    CPropSheetHost *pSecondarySheet = new CPropSheetHost(m_hInst, pDSASecPageInfo->hwndParentSheet);
    if(!pSecondarySheet)
    {
        return;
    }

    // Hold a reference count for the CPropSheetHost object.
    pSecondarySheet->AddRef();

    // Build the ADsPath of the object to create the property sheet for.
    LPWSTR pwszName = (LPWSTR)((LPBYTE)pdsObject + pdsObject->offsetName);
    CComBSTR sbstrTemp = "LDAP://";
    sbstrTemp += pwszName;
    pSecondarySheet->SetObject(sbstrTemp);

    pSecondarySheet->Run();

    /*
    Release the CPropSheetHost object. Other components may still hold a 
    reference to the object, so this cannot just be deleted here. Let the object 
    delete itself when all references are released.
    */
    pSecondarySheet->Release();
}

