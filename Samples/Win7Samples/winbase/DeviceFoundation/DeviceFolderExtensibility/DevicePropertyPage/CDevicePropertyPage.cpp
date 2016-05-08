////////////////////////////////////////////////////////////////////////////////
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
////////////////////////////////////////////////////////////////////////////////

// Public Headers
#include <windows.h>
#include <shlobj.h>
#include <shellapi.h>
#include <propkey.h>
#include <functiondiscoverykeys.h>
#include <setupapi.h>
#include <CommonControls.h>
#include <new>

// Needed before including devpkey.h as there's no lib with the DEVPKEY objects
#include <initguid.h> 
#include <devpkey.h>

// Sample Headers
#include "CDevicePropertyPage.h"
#include "common.h"
#include "resource.h"

#define ARRAY_SIZE(a)   (sizeof(a) / sizeof((a)[0]))

//
// Struct used to pass data from AddPages -> Window Proc function
//
struct PROPSINPUT
{
    IShellItem2* pShellItem;
    HICON hIcon;
    // Add extra data needed here.
};


//------------------------------------------------------------------------------
// CDevicePropertyPage::CDevicePropertyPage (Constructor)
//------------------------------------------------------------------------------
CDevicePropertyPage::CDevicePropertyPage():
    m_cRef(1),
    m_pShellItem(NULL)
{
    DllIncLockCount();
}


//------------------------------------------------------------------------------
// CDevicePropertyPage::~CDevicePropertyPage (Destructor)
//------------------------------------------------------------------------------
CDevicePropertyPage::~CDevicePropertyPage()
{
    if( NULL != m_pShellItem )
    {
        m_pShellItem->Release();
    }
    DllDecLockCount();
}

//
// IShellPropSheetExt
//

//------------------------------------------------------------------------------
// CDevicePropertyPage::AddPages
//
//      This method is called by the Windows Shell to give oportunity to 
//      register one or more property pages with the property sheet. Minimal
//      work should be done here. Essentially only enough work to figure out
//      whether the property page should be added or not (i.e. no critical data
//      is missing). All other work should be done in the DlgProc function
//      when the WM_INITDIALOG message is sent.
//------------------------------------------------------------------------------
IFACEMETHODIMP CDevicePropertyPage::AddPages(
    __in LPFNADDPROPSHEETPAGE pfnAddPage,
    __in LPARAM lParam
    )
{
    HPROPSHEETPAGE  hPage;
    HRESULT         hr  = S_OK;
    PROPSHEETPAGE   psp = {0};
    PROPSINPUT*     pPropsInput = NULL;

    if( NULL == pfnAddPage )
    {
        return E_INVALIDARG;
    }

    //
    // Allocate the PROPSINPUT struct
    //
    pPropsInput = new (std::nothrow) PROPSINPUT;
    if( NULL == pPropsInput )
    {
        hr = E_OUTOFMEMORY;
    }

    if( S_OK == hr )
    {
        //
        // Fill out the PROPSHEETPAGE structure
        //
        psp.dwSize = sizeof(PROPSHEETPAGE);
        psp.dwFlags = PSP_USECALLBACK;
        psp.hInstance = g_hInstance;
        psp.pszTemplate = MAKEINTRESOURCE(IDD_PROPPAGE_DEVICE);
        psp.pfnDlgProc = PropPageDlgProc;
        psp.lParam = reinterpret_cast<LPARAM>( pPropsInput );
        psp.pfnCallback = PropPageDlgCleanup;

        //
        // Give a reference to the shell item to the callback data
        //
        hr = m_pShellItem->QueryInterface( &pPropsInput->pShellItem );
    }

    //
    // Grab the icon from the shell item
    //
    if( S_OK == hr )
    {
        hr = GetIconFromItem( m_pShellItem, SHIL_LARGE, &pPropsInput->hIcon );
    }

    //
    // Create the page
    //
    hPage = CreatePropertySheetPage( &psp );
    if( NULL == hPage )
    {
        hr = E_OUTOFMEMORY;
    }

    //
    // Give the shell the page
    //
    if( S_OK == hr &&
        !pfnAddPage( hPage, lParam ) )
    {
        hr = E_FAIL;
    }

    //
    // As the documentation for AddPages describes, you can "request" that your
    // page is the one visibile when the property sheet is loaded. You can do
    // this by giving the index of the page you added (1 based). In this sample
    // we just added one page, so we can return 1 instead of the HR (assuming
    // we didn't fail) to request our page to be the one visible on launch.
    // Keep in mind it is a "request" and is not guaranteed. 
    //
    if( S_OK == hr )
    {
        hr = static_cast<HRESULT>(1);
    }
    else
    {
        if( NULL != hPage )
        {
            DestroyPropertySheetPage( hPage );
        }
        if( NULL != pPropsInput->pShellItem )
        {
            pPropsInput->pShellItem->Release();
        }
        if( NULL != pPropsInput->hIcon )
        {
            DestroyIcon( pPropsInput->hIcon );
        }
    }

    return hr;
}// CDevicePropertyPage::AddPages


//------------------------------------------------------------------------------
// CDevicePropertyPage::ReplacePage
//      Not supported.
//------------------------------------------------------------------------------
IFACEMETHODIMP CDevicePropertyPage::ReplacePage(
    __in UINT uPageID,
    __in LPFNADDPROPSHEETPAGE pfnReplacePage,
    __in LPARAM lParam
    )
{
    UNREFERENCED_PARAMETER( uPageID );
    UNREFERENCED_PARAMETER( pfnReplacePage );
    UNREFERENCED_PARAMETER( lParam );

    return E_NOTIMPL;
}// CDevicePropertyPage::ReplacePage


//
// IShellExtInit
//

//------------------------------------------------------------------------------
// CDevicePropertyPage::Initialize
//
//      This is the first method that the Shell calls after it creates an
//      instance of a property sheet extension.
//------------------------------------------------------------------------------
IFACEMETHODIMP CDevicePropertyPage::Initialize(
    __in PCIDLIST_ABSOLUTE pidlFolder,
    __in IDataObject* pdtobj,
    __in HKEY hkeyProgID
    )
{
    UNREFERENCED_PARAMETER( hkeyProgID );
    UNREFERENCED_PARAMETER( pidlFolder );

    if( NULL != m_pShellItem )
    {
        m_pShellItem->Release();
        m_pShellItem = NULL;
    }

    //
    // Grab the interface to the shell item
    //
    return SHGetItemFromObject(
        pdtobj, 
        __uuidof(IShellItem2),
        reinterpret_cast<void**>(&m_pShellItem)
        );
}// CDevicePropertyPage::Initialize


//
// IUnknown
//

//------------------------------------------------------------------------------
// CDevicePropertyPage::QueryInterface
//------------------------------------------------------------------------------
IFACEMETHODIMP CDevicePropertyPage::QueryInterface(
    __in REFIID riid, 
    __deref_out void** ppvObject
    )
{
    HRESULT hr = S_OK;

    if( NULL == ppvObject )
    {
        return E_INVALIDARG;
    }

    *ppvObject = NULL;

    if( __uuidof(IShellExtInit) == riid )
    {
        *ppvObject = static_cast<IShellExtInit*>(this);
        AddRef();
    }
    else if( __uuidof(IShellPropSheetExt) == riid )
    {
        *ppvObject = static_cast<IShellPropSheetExt*>(this);
        AddRef();
    }
    else if( __uuidof(IUnknown) == riid )
    {
        *ppvObject = static_cast<IUnknown*>(static_cast<IShellExtInit*>(this));
        AddRef();
    }
    else
    {
        hr = E_NOINTERFACE;
    }

    return hr;
}// CDevicePropertyPage::QueryInterface


//------------------------------------------------------------------------------
// CDevicePropertyPage::AddRef
//------------------------------------------------------------------------------
IFACEMETHODIMP_(ULONG) CDevicePropertyPage::AddRef()
{
    return InterlockedIncrement( &m_cRef );
}// CDevicePropertyPage::AddRef


//------------------------------------------------------------------------------
// CDevicePropertyPage::Release
//------------------------------------------------------------------------------
IFACEMETHODIMP_(ULONG) CDevicePropertyPage::Release()
{
    LONG cRef = InterlockedDecrement( &m_cRef );

    if( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}// CDevicePropertyPage::Release


//
// Private class methods
//

//------------------------------------------------------------------------------
// CDevicePropertyPage::GetIconFromItem [STATIC FUNC]
//
//      Gets a handle to the icon of the shell item. phIcon needs to be cleaned
//      up with DestroyIcon() when done. 
//------------------------------------------------------------------------------
HRESULT CDevicePropertyPage::GetIconFromItem(
    __in IShellItem* pShellItem, 
    __in int iImageList, 
    __out HICON* phIcon
    )
{
    HRESULT         hr              = S_OK;
    int             iIcon           = 0;
    PITEMID_CHILD   pidl            = NULL;
    IImageList*     pImageList      = NULL;
    IParentAndItem* pParentAndItem  = NULL;
    IShellFolder*   pShellFolder    = NULL;

    *phIcon = NULL;

    hr = pShellItem->QueryInterface( &pParentAndItem );

    if( S_OK == hr )
    {
        hr = pParentAndItem->GetParentAndItem( NULL, &pShellFolder, &pidl );
    }

    if( S_OK == hr )
    {
        hr = SHGetImageList(
            iImageList,
            __uuidof(IImageList),
            reinterpret_cast<void**>(&pImageList)
            );
    }

    if( S_OK == hr )
    {
        iIcon = SHMapPIDLToSystemImageListIndex( pShellFolder, pidl, NULL );
        hr = pImageList->GetIcon( iIcon, 0, phIcon );
    }

    //
    // Cleanup
    //
    if( NULL != pImageList )
    {
        pImageList->Release();
    }
    if( NULL != pidl )
    {
        ILFree( pidl );
    }
    if( NULL != pShellFolder )
    {
        pShellFolder->Release();
    }
    if( NULL != pParentAndItem )
    {
        pParentAndItem->Release();
    }

    return hr;
}// CDevicePropertyPage::GetIconFromItem


//------------------------------------------------------------------------------
// CDevicePropertyPage::PropPageDlgProc [STATIC FUNC]
//
//      This function is used to process windows messages that are sent to the
//      property page.
//------------------------------------------------------------------------------
INT_PTR CALLBACK CDevicePropertyPage::PropPageDlgProc(
    __in HWND hWndDlg,
    __in UINT uMsg,
    __in WPARAM wParam,
    __in LPARAM lParam
    )
{
    UNREFERENCED_PARAMETER( wParam );

    HRESULT             hr              = S_OK;
    PROPSINPUT*         pPropsInput     = NULL;
    LPPROPSHEETPAGE     ppsp            = NULL;

    if( WM_INITDIALOG == uMsg )
    {
        ppsp = reinterpret_cast<LPPROPSHEETPAGE>(lParam);
        pPropsInput = reinterpret_cast<PROPSINPUT*>(ppsp->lParam);

        //
        // Set the Icon if available
        //
        if( NULL != pPropsInput->hIcon )
        {
            SendMessage(
                GetDlgItem( hWndDlg, IDC_DEVICE_ICON ),
                STM_SETICON,
                reinterpret_cast<WPARAM>(pPropsInput->hIcon),
                0
                );
        }

        //
        // Populate the page with properties available on the shell object
        //
        hr = PopulateShellProperties( hWndDlg, pPropsInput->pShellItem );

        //
        // Populate the page with properties available directly from the
        // devnode(s)
        //
        if( S_OK == hr )
        {
            hr = PopulateDevnodeProperties( hWndDlg, pPropsInput->pShellItem );
        }
    }

    return hr;
}// CDevicePropertyPage::PropPageDlgProc


//------------------------------------------------------------------------------
// CDevicePropertyPage::PropPageDlgCleanup [STATIC FUNC]
//
//      This function is called just before the page is created and when it's
//      about to be destroyed. In this sample, it will just be used to free the
//      pidl. 
//------------------------------------------------------------------------------
UINT CALLBACK CDevicePropertyPage::PropPageDlgCleanup(
    __in HWND hwnd,
    __in UINT uMsg,
    __in LPPROPSHEETPAGE ppsp
    )
{
    UNREFERENCED_PARAMETER( hwnd );

    PROPSINPUT* pPropsInput = reinterpret_cast<PROPSINPUT*>(ppsp->lParam);

    if( PSPCB_RELEASE == uMsg &&
        NULL != pPropsInput )
    {
        if( NULL != pPropsInput->pShellItem )
        {
            pPropsInput->pShellItem->Release();
        }
        if( NULL != pPropsInput->hIcon )
        {
            DestroyIcon( pPropsInput->hIcon );
        }

        delete pPropsInput;
    }

    return 1;
}// CDevicePropertyPage::PropPageDlgCleanup


//------------------------------------------------------------------------------
// CDevicePropertyPage::PopulateShellProperties [STATIC FUNC]
//
//      Populates the property page with the properties available directly from
//      the shell item object.  Other properties have to be farmed from the
//      devnodes that make up the device. This process is shown in 
//      PopulateDevnodeProperties.
//------------------------------------------------------------------------------
HRESULT CDevicePropertyPage::PopulateShellProperties(
    __in HWND hWndDlg,
    __in IShellItem2* pShellItem
    )
{
    HRESULT hr              = S_OK;
    PWSTR   pszValue        = {0};
    GUID    guidContainerID = {0};

    //
    // Start filling out the properties on the property page.
    // If any failure occurs, the rest of the properties will be skipped.
    //
    // If this pattern isn't ideal for your code--for instance you don't
    // need all properties available all the time--then be sure to adjust
    // the error checking pattern.
    //

    //
    // Device Name
    //
    if( S_OK == hr &&
        S_OK == pShellItem->GetString( PKEY_ItemNameDisplay, &pszValue ))
    {
        SetDlgItemText( hWndDlg, IDC_NAME_FIELD, pszValue );
        CoTaskMemFree( pszValue );
    }

    //
    // Container Id
    //
    if( S_OK == hr &&
        S_OK == pShellItem->GetCLSID( PKEY_Devices_ContainerId, &guidContainerID ) &&
        S_OK == StringFromCLSID( guidContainerID, &pszValue ))
    {
        SetDlgItemText( hWndDlg, IDC_CONTAINERID_FIELD, pszValue );
        CoTaskMemFree( pszValue );
    }

    //
    // Manufacturer
    //
    if( S_OK == hr &&
        S_OK == pShellItem->GetString( PKEY_Devices_Manufacturer, &pszValue ))
    {
        SetDlgItemText( hWndDlg, IDC_MANUFACTURER_FIELD, pszValue );
        CoTaskMemFree( pszValue );
    }

    //
    // Model Name
    //
    if( S_OK == hr &&
        S_OK == pShellItem->GetString( PKEY_Devices_ModelName, &pszValue ))
    {
        SetDlgItemText( hWndDlg, IDC_MODELNAME_FIELD, pszValue );
        CoTaskMemFree( pszValue );
    }

    //
    // Model Number
    //
    if( S_OK == hr &&
        S_OK == pShellItem->GetString( PKEY_Devices_ModelNumber, &pszValue ))
    {
        SetDlgItemText( hWndDlg, IDC_MODELNUMBER_FIELD, pszValue );
        CoTaskMemFree( pszValue );
    }

    //
    // Device Description 1
    //
    if( S_OK == hr &&
        S_OK == pShellItem->GetString( PKEY_Devices_DeviceDescription1, &pszValue ))
    {
        SetDlgItemText( hWndDlg, IDC_DESCRIPTION1_FIELD, pszValue );
        CoTaskMemFree( pszValue );
    }

    //
    // Device Description 2
    //
    if( S_OK == hr &&
        S_OK == pShellItem->GetString( PKEY_Devices_DeviceDescription2, &pszValue ))
    {
        SetDlgItemText( hWndDlg, IDC_DESCRIPTION2_FIELD, pszValue );
        CoTaskMemFree( pszValue );
    }

    return hr;
}// CDevicePropertyPage::PopulateShellProperties


//------------------------------------------------------------------------------
// CDevicePropertyPage::PopulateDevnodeProperties [STATIC FUNC]
//
//      Populates the property page with some properties that come from the
//      devnode(s) of the device directly. When implementing the property page
//      for your device, this method is needed for all properties that cannot
//      be obtained directly from the shell item object.
//
//      Note there are two pieces of contextual information from the shell
//      object you can use to find the devnodes for your device and farm extra
//      properties and/or communicate with your device.
//
//      1) Function Paths (PKEY_Devices_FunctionPaths)
//          This property is a vector of strings of the Device Instance Paths
//          for your device's devnodes. This function below will show how to
//          use this property to access properties off the devnodes.
//
//      2) Interface Paths (PKEY_Devices_InterfacePaths)
//          This property is a vectory of strings of the Device Interface Paths
//          of all your device's devnodes. Use of this property use not shown in
//          this sample, but if you find the property helpful for your scenario
//          you can use it. 
//------------------------------------------------------------------------------
HRESULT CDevicePropertyPage::PopulateDevnodeProperties(
    __in HWND hWndDlg,
    __in IShellItem2* pShellItem
    )
{
    DWORD           cbBuffer            = 0;
    HDEVINFO        devInfo             = {0}; 
    SP_DEVINFO_DATA devInfoData         = {0};
    DEVPROPTYPE     devPropType         = 0;
    HRESULT         hr                  = S_OK;
    SYSTEMTIME      installTime         = {0};
    PBYTE           pBuffer             = NULL;
    PROPVARIANT     pv                  = {0};
    PCWSTR          pszHwid             = NULL;
    WCHAR           szInstallTime[1024] = {0};

    PropVariantInit( &pv );

    //
    // First get the Function Paths (Device Instance Paths) needed to grab the
    // devnode info directly from PnP.
    //
    if( S_OK == hr )
    {
        hr = pShellItem->GetProperty( PKEY_Devices_FunctionPaths, &pv );
    }
    if( S_OK == hr &&
        ((VT_VECTOR | VT_LPWSTR) != pv.vt ||
        ( 0 == pv.calpwstr.cElems ) ) )
    {
        // Function Paths doesn't exist or is the wrong type or empty. 
        // This should never happen, but its good practice to check anyway.
        hr = HRESULT_FROM_WIN32( ERROR_NOT_FOUND );
    }

    //
    // For simplicity in the sample, we'll just work with the first path in the
    // list. i.e. the first devnode in the list.
    //
    // IMPORTANT: In a real scenario, you need to keep in mind that your device
    // shown in the Devices and Printers folder is likely made up of one or more
    // Device Functions (devnodes). In this case, you may not be able to get all
    // properties from just any devnode. You'll need to look at all devnodes and
    // figure out which one contains the properties you're after. In this sample
    // we're just attempting to get a set of properties from the devnode who's
    // Device Instance Path is in the FunctionPaths list retreived from the shell
    // object. 
    //

    //
    // Create an empty HDEVINFO set to use for the first devnode's info
    //
    if( S_OK == hr )
    {
        devInfo = SetupDiCreateDeviceInfoList( NULL, NULL );
        if( INVALID_HANDLE_VALUE == devInfo )
        {
            hr = HRESULT_FROM_WIN32( GetLastError() );
        }
    }

    //
    // Open the devnode's info
    //
    if( S_OK == hr )
    {
        devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

        if( FALSE == SetupDiOpenDeviceInfo(
                        devInfo,
                        pv.calpwstr.pElems[0],
                        NULL, 
                        0, 
                        &devInfoData 
                        ) )
        {
            hr = HRESULT_FROM_WIN32( GetLastError() );
        }
    }

    //
    // Now populate the property page with some properties from the devnode
    // 
    // You can ask for properties defined in devpkey.h. Some keys in 
    // FunctionDiscoveryKey.h are also available on devnodes; For example,
    // devnodes from PnP-X (Network Devices) will have PKEY_PNPX properties
    // set on them. These can be retreived with SetupDi* calls as well.
    // PROPERTYKEY can be safely cast to DEVPROPKEY. However, keep in mind
    // the types are defined differently. Variant types are essentially a
    // superset of DEVPROPTYPEs. The mapping on many property types is
    // straight forward. DEVPROP_TYPE_STRING and VT_LPWSTR
    // are the same, for example. Below we'll get a PnP-X property so it's
    // clear how this works. 
    //
    // One case where the mapping isn't exact, is VT_VECTOR | VT_LPWSTR
    // (vector of strings), which in the devnode is stored as a 
    // DEVPROP_TYPE_STRING_LIST (REG_MULTI_SZ style string list). Keep this
    // in mind when asking for PKEY types from a devnode vs. DEVPKEY types.
    //

    //
    // Get the hardware ids
    //
    if( S_OK == hr )
    {
        // Get the required buffer size 
        if( FALSE == SetupDiGetDeviceProperty(
                        devInfo,
                        &devInfoData,
                        &DEVPKEY_Device_HardwareIds,
                        &devPropType,
                        NULL,
                        0,
                        &cbBuffer,
                        0 
                        ) &&
            ERROR_INSUFFICIENT_BUFFER != GetLastError() )
        {
            hr = HRESULT_FROM_WIN32( GetLastError() );
        }
    }

    if( S_OK == hr )
    {
        pBuffer = new (std::nothrow) BYTE[cbBuffer];
        if( NULL == pBuffer )
        {
            hr = E_OUTOFMEMORY;
        }
        ZeroMemory( pBuffer, cbBuffer );
    }

    if( S_OK == hr )
    {
        if( FALSE == SetupDiGetDeviceProperty(
                        devInfo,
                        &devInfoData,
                        &DEVPKEY_Device_HardwareIds,
                        &devPropType,
                        pBuffer,
                        cbBuffer,
                        NULL,
                        0 
                        ) )
        {
            hr = HRESULT_FROM_WIN32( GetLastError() );
        }
    }

    if( S_OK == hr &&
        DEVPROP_TYPE_STRING_LIST == devPropType )
    {
        //
        // The UI only has slots for up to three hardware ids, so we'll
        // just set up to the first three.
        //
        pszHwid = reinterpret_cast<PWSTR>(pBuffer);

        if( NULL != *pszHwid )
        {
            SetDlgItemText(
                hWndDlg, 
                IDC_HARDWAREID_FIELD1, 
                pszHwid
                );
            pszHwid = pszHwid + wcslen(pszHwid) + 1;
        }
        if( NULL != *pszHwid )
        {
            SetDlgItemText(
                hWndDlg, 
                IDC_HARDWAREID_FIELD2, 
                pszHwid
                );
            pszHwid = pszHwid + wcslen(pszHwid) + 1;
        }
        if( NULL != *pszHwid )
        {
            SetDlgItemText(
                hWndDlg, 
                IDC_HARDWAREID_FIELD3, 
                pszHwid
                );
        }
    }
    
    if( NULL != pBuffer )
    {
        delete[] pBuffer;
        pBuffer = NULL;
    }

    //
    // Get the install date
    //
    if( S_OK == hr )
    {
        // Get the required buffer size 
        if( FALSE == SetupDiGetDeviceProperty(
                        devInfo,
                        &devInfoData,
                        &DEVPKEY_Device_InstallDate,
                        &devPropType,
                        NULL,
                        0,
                        &cbBuffer,
                        0 
                        ) &&
            ERROR_INSUFFICIENT_BUFFER != GetLastError() )
        {
            hr = HRESULT_FROM_WIN32( GetLastError() );
        }
    }

    if( S_OK == hr )
    {
        pBuffer = new (std::nothrow) BYTE[cbBuffer];
        if( NULL == pBuffer )
        {
            hr = E_OUTOFMEMORY;
        }
    }

    if( S_OK == hr )
    {
        if( FALSE == SetupDiGetDeviceProperty(
                        devInfo,
                        &devInfoData,
                        &DEVPKEY_Device_InstallDate,
                        &devPropType,
                        pBuffer,
                        cbBuffer,
                        NULL,
                        0 
                        ) )
        {
            hr = HRESULT_FROM_WIN32( GetLastError() );
        }
        else if( DEVPROP_TYPE_FILETIME != devPropType )
        {
            hr = E_INVALIDARG;
        }
    }

    if( S_OK == hr )
    {
        if( FALSE == FileTimeToSystemTime(
                reinterpret_cast<FILETIME*>(pBuffer), 
                &installTime ))
        {
            hr = HRESULT_FROM_WIN32( GetLastError() );
        }
    }

    if( S_OK == hr )
    {
        if( 0 == GetDateFormatEx( 
                LOCALE_NAME_USER_DEFAULT,
                DATE_AUTOLAYOUT | DATE_LONGDATE,
                &installTime,
                NULL,
                szInstallTime,
                ARRAY_SIZE(szInstallTime),
                NULL
                ))
        {
            hr = HRESULT_FROM_WIN32( GetLastError() );
        }
    }

    if( S_OK == hr )
    {
        SetDlgItemText( hWndDlg, IDC_INSTALLDATE_FIELD, szInstallTime );
    }
    
    if( NULL != pBuffer )
    {
        delete[] pBuffer;
        pBuffer = NULL;
    }

    //
    // Now get PKEY_** property instead of a DEVPKEY property
    // to show how that's done.  This would typically just be done
    // for PNPX properties, but other properties in FunctionDisocovery.h
    // might be available on the devnode.
    //
    // Let's attempt to get the IP Address, though it won't be available on
    // Mice (which is what this sample property page binds to). It at least
    // shows how the process would work. Also, as stated above, only *some*
    // of the devnodes that make up your PnP-X device may contain the 
    // IP Address, so you might have to look through all the devnodes by
    // iterating the FunctionPaths list. 
    //
    // This property won't be set on the property page itself since a mouse
    // will never have an IP Address.
    //
    if( S_OK == hr )
    {
        // Get the required buffer size 
        if( FALSE == SetupDiGetDeviceProperty(
                        devInfo,
                        &devInfoData,
                        (const DEVPROPKEY*)&PKEY_PNPX_IpAddress,
                        &devPropType,
                        NULL,
                        0,
                        &cbBuffer,
                        0 
                        ) &&
            ERROR_INSUFFICIENT_BUFFER != GetLastError() )
        {
            hr = HRESULT_FROM_WIN32( GetLastError() );
        }
    }

    if( S_OK == hr )
    {
        pBuffer = new (std::nothrow) BYTE[cbBuffer];
        if( NULL == pBuffer )
        {
            hr = E_OUTOFMEMORY;
        }
    }

    if( S_OK == hr )
    {
        if( FALSE == SetupDiGetDeviceProperty(
                        devInfo,
                        &devInfoData,
                        (const DEVPROPKEY*)&PKEY_PNPX_IpAddress,
                        &devPropType,
                        pBuffer,
                        cbBuffer,
                        NULL,
                        0 
                        ) )
        {
            hr = HRESULT_FROM_WIN32( GetLastError() );
        }
    }

    if( S_OK == hr &&
        DEVPROP_TYPE_STRING == devPropType
        )
    {
        // pBuffer now contains the IP Address if this is a PnP-X device and
        // this was one of the devnodes that had the property set.
    }

    //
    // Cleanup
    //
    if( NULL != pBuffer )
    {
        delete[] pBuffer;
    }

    if( INVALID_HANDLE_VALUE != devInfo )
    {
        (void) SetupDiDestroyDeviceInfoList( devInfo );
    }

    PropVariantClear( &pv );

    return hr;
}// CDevicePropertyPage::PopulateDevnodeProperties
