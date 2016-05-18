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
#include <propkey.h>
#include <functiondiscoverykeys.h>
#include <setupapi.h>
#include <shlwapi.h>
#include <new>

// Needed before including devpkey.h as there's no lib with the DEVPKEY objects
#include <initguid.h> 
#include <devpkey.h>

// Sample Headers
#include "CDeviceContextMenu.h"
#include "common.h"
#include "resource.h"


DWORD WINAPI _ExecuteThreadProc( __in LPVOID pVoid );


//------------------------------------------------------------------------------
// CDeviceContextMenu::CDeviceContextMenu (Constructor)
//------------------------------------------------------------------------------
CDeviceContextMenu::CDeviceContextMenu():
    m_cRef(1),
    m_szCommandName(NULL)
{
    DllIncLockCount();
}


//------------------------------------------------------------------------------
// CDeviceContextMenu::~CDeviceContextMenu (Destructor)
//------------------------------------------------------------------------------
CDeviceContextMenu::~CDeviceContextMenu()
{
    if( NULL != m_szCommandName )
    {
        CoTaskMemFree( m_szCommandName );
        m_szCommandName = NULL;
    }
    DllDecLockCount();
}

//------------------------------------------------------------------------------
// CDeviceContextMenu::GetTitle
//
//      Get the context menu item's display name
//------------------------------------------------------------------------------
IFACEMETHODIMP CDeviceContextMenu::GetTitle(
    __in IShellItemArray* psiItemArray, 
    __out LPWSTR* ppszName
    ) 
{
    UNREFERENCED_PARAMETER( psiItemArray );

    HRESULT hr = E_FAIL;
    WCHAR szTemp[MAX_PATH] = {0};

    if( LoadString( g_hInstance, IDS_ContextMenu_Title, szTemp, ARRAYSIZE(szTemp)))
    {
        hr = SHStrDup( szTemp, ppszName );
    }
    else
    {
        hr = HRESULT_FROM_WIN32( GetLastError() );
    }

    return hr;
}// CDeviceContextMenu::GetTitle


//------------------------------------------------------------------------------
// CDeviceContextMenu::GetIcon
//
//      Get the context menu item's icon resource string
//------------------------------------------------------------------------------
IFACEMETHODIMP CDeviceContextMenu::GetIcon(
    __in IShellItemArray* psiItemArray, 
    __out LPWSTR* ppszIcon
    ) 
{ 
    UNREFERENCED_PARAMETER( psiItemArray );

    // this sample uses the mouse icon
    return SHStrDup( L"main.cpl,-100", ppszIcon );

}// CDeviceContextMenu::GetIcon


//------------------------------------------------------------------------------
// CDeviceContextMenu::GetToolTip
//
//      Get the context menu item's tooltip string
//------------------------------------------------------------------------------
IFACEMETHODIMP CDeviceContextMenu::GetToolTip(
    __in IShellItemArray* psiItemArray, 
    __out LPWSTR* ppszInfotip
    ) 
{ 
    UNREFERENCED_PARAMETER( psiItemArray );

    HRESULT hr = E_FAIL;
    WCHAR szTemp[MAX_PATH] = {0};

    if( LoadString( g_hInstance, IDS_ContextMenu_Tooltip, szTemp, ARRAYSIZE(szTemp)))
    {
        hr = SHStrDup( szTemp, ppszInfotip );
    }
    else
    {
        hr = HRESULT_FROM_WIN32( GetLastError() );
    }

    return hr;
}// CDeviceContextMenu::GetToolTip


//------------------------------------------------------------------------------
// CDeviceContextMenu::GetCanonicalName
//
//      Get the context menu item's canonical name
//------------------------------------------------------------------------------
IFACEMETHODIMP CDeviceContextMenu::GetCanonicalName( __out GUID* pguidCommandName ) 
{ 
    *pguidCommandName = __uuidof(DeviceContextMenu);
    return S_OK;
} // CDeviceContextMenu::GetCanonicalName


//------------------------------------------------------------------------------
// CDeviceContextMenu::GetState
//
//      Get the context menu item's state 
//------------------------------------------------------------------------------
IFACEMETHODIMP CDeviceContextMenu::GetState(
    __in_opt IShellItemArray* psiItemArray, 
    BOOL fOkToBeSlow, 
    __out EXPCMDSTATE* pCmdState
    ) 
{
    UNREFERENCED_PARAMETER( psiItemArray );
    UNREFERENCED_PARAMETER( fOkToBeSlow );

    // The second parameter fOkToBeSlow lets you know if performing slow
    // operations (such as I/O, network calls) is OK. If this value is FALSE and
    // your implementation performs slow operations, GetState should return
    // E_PENDING.  This will cause explorer to call GetState again on a
    // background thread with fOkToBeSlow set to TRUE.  This prevents slow
    // operations in your implementation from running on the UI thread, thus
    // preventing performance issues (hanging UI).

    //
    // The state of the context menu item can be set based on information such
    // as the device's properties. In this sample, it's set to be enabled. See
    // the Invoke method below for how to get properties from the shell item
    // and or the device's devnodes.
    //
    *pCmdState = ECS_ENABLED;
    return S_OK;
}// CDeviceContextMenu::GetState


//------------------------------------------------------------------------------
// CDeviceContextMenu::Invoke
//
//      Invoke the context menu item.
//------------------------------------------------------------------------------
IFACEMETHODIMP CDeviceContextMenu::Invoke(
    __in IShellItemArray* psiItemArray,
    __in IBindCtx* pbc
    )
{
    UNREFERENCED_PARAMETER( pbc );

    HRESULT         hr              = S_OK;
    IShellItem*     pShellItem      = NULL;
    IShellItem2*    pShellItem2     = NULL;
    LPWSTR          pszHardwareID   = NULL;
    PROPVARIANT     pv              = {0};
    PWSTR           pszName         = NULL;

    if( NULL == psiItemArray )
    {
        return E_INVALIDARG;
    }

    PropVariantInit( &pv );

    // 
    // The selected device is the only shell item in the shell item array.
    // Use GetItemAt with index 0 to get the shell item corresponding to the
    // selected device.
    //
    if( S_OK == hr )
    {
        hr = psiItemArray->GetItemAt( 0, &pShellItem );
    }

    //
    // Get the IShellItem2 interface of the shell item, so we can
    // access its properties from the shell item directly or from
    // the devnode(s) of the device. 
    //
    if( S_OK == hr )
    {
        hr = pShellItem->QueryInterface( 
            __uuidof(pShellItem2), 
            reinterpret_cast<void**>(&pShellItem2)
            );
    }

    //
    // Here you might take a look at the saved m_pszCommandName to
    // decide what actions to take and what application to end up
    // launching.
    //

    //
    // Get device's properties directly from the shell item object. In
    // this case we'll get the name. 
    //
    if( S_OK == hr )
    {
        hr = pShellItem2->GetProperty( PKEY_ItemNameDisplay, &pv );
    }
    if( S_OK == hr &&
        VT_LPWSTR == pv.vt )
    {
        hr = SHStrDup( pv.pwszVal, &pszName );
    }

    //
    // Get device's properties from the devnode(s) of the device.
    // We get one of the device's hardware ids in this sample.
    //
    if( S_OK == hr )
    {
        // ignore the failure because we don't use this information
        // in this sample.
        GetDeviceHardwareID( pShellItem2, &pszHardwareID );
    }

    //
    // Launch a thread to show a message box with one of the properties
    // we grabbed. This section of code would be replaced with code to
    // launch an relevant device related application based on
    // contextual information from the properties (if needed).
    //
    // A separate thread is used here to avoid blocking the main UI 
    // thread while the message box exists.
    //
    if( S_OK == hr )
    {
        CreateThread( NULL, 0, _ExecuteThreadProc, pszName, 0, NULL );
    }

    //
    // Clean up
    //
    if( NULL != pShellItem )
    {
        pShellItem->Release();
    }

    if( NULL != pShellItem2 )
    {
        pShellItem2->Release();
    }

    if( NULL != pszHardwareID )
    {
        CoTaskMemFree( pszHardwareID );
    }
        
    PropVariantClear( &pv );
   
    return hr;
} // CDeviceContextMenu::Invoke


//------------------------------------------------------------------------------
// CDeviceContextMenu::GetFlags
//
//      Get the context menu item's flags
//------------------------------------------------------------------------------
IFACEMETHODIMP CDeviceContextMenu::GetFlags( __out EXPCMDFLAGS* pFlags ) 
{ 
    *pFlags = ECF_DEFAULT;
    return S_OK; 
}// CDeviceContextMenu::GetFlags


//------------------------------------------------------------------------------
// CDeviceContextMenu::EnumSubCommands
//
//     Returns an IEnumExplorerCommand instance used to enumerate sub commands 
//     of the current command when the context menu item has submenus. 
//     It's not implemented in this sample.
//------------------------------------------------------------------------------
IFACEMETHODIMP CDeviceContextMenu::EnumSubCommands(
    __deref_out IEnumExplorerCommand** ppEnum
    ) 
{ 
    UNREFERENCED_PARAMETER( ppEnum );

    return E_NOTIMPL; 
}// CDeviceContextMenu::EnumSubCommands


//------------------------------------------------------------------------------
// CDeviceContextMenu::Initialize
//
//      Initializes the context menu handler with the application-specified 
//      context menu items name and its properties from the property bag. 
//------------------------------------------------------------------------------
IFACEMETHODIMP CDeviceContextMenu::Initialize(
    __in PCWSTR pszCommandName, 
    __in IPropertyBag* ppb
    )
{
    UNREFERENCED_PARAMETER( ppb );

    // Shell will create one object for each context menu item if the context
    // menu handler supports multiple items. Initialize will be called for each
    // of the objects. The behavior of the Invoke implementation can be changed
    // based on which item was clicked on by the user. To do that, the command
    // name is saved to a class member for later use in Invoke.
    //
    // Data stored under this item in the registry can be read via the property
    // bag if needed.

    // In this sample, we save the item name for later use.
    return SHStrDup( pszCommandName, &m_szCommandName );
}// CDeviceContextMenu::Initialize


//
// IUnknown
//

//------------------------------------------------------------------------------
// CDeviceContextMenu::QueryInterface
//------------------------------------------------------------------------------
IFACEMETHODIMP CDeviceContextMenu::QueryInterface(
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

    if( __uuidof(IExplorerCommand) == riid )
    {
        *ppvObject = static_cast<IExplorerCommand*>(this);
        AddRef();
    }
    else if( __uuidof(IInitializeCommand) == riid )
    {
        *ppvObject = static_cast<IInitializeCommand*>(this);
        AddRef();
    }
    else if( __uuidof(IUnknown) == riid )
    {
        *ppvObject = static_cast<IUnknown*>(static_cast<IExplorerCommand*>(this));
        AddRef();
    }
    else
    {
        hr = E_NOINTERFACE;
    }

    return hr;
}// CDeviceContextMenu::QueryInterface


//------------------------------------------------------------------------------
// CDeviceContextMenu::AddRef
//------------------------------------------------------------------------------
IFACEMETHODIMP_(ULONG) CDeviceContextMenu::AddRef()
{
    return InterlockedIncrement( &m_cRef );
}// CDeviceContextMenu::AddRef


//------------------------------------------------------------------------------
// CDeviceContextMenu::Release
//------------------------------------------------------------------------------
IFACEMETHODIMP_(ULONG) CDeviceContextMenu::Release()
{
    LONG cRef = InterlockedDecrement( &m_cRef );

    if( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}// CDeviceContextMenu::Release


//------------------------------------------------------------------------------
// CDeviceContextMenu::GetDeviceHardwareID
//
//      Get device hardware id from the devnode(s) of the device. You can get 
//      device properties that cannot be obtained directly from the shell item object
//      throught the device devnode(s).
//
//      Note there are two pieces of contextual information from the shell
//      object you can use to find the devnodes for your device.
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
HRESULT CDeviceContextMenu::GetDeviceHardwareID(
    __in IShellItem2* pShellItem2,
    __out PWSTR* ppszHardwareID
    )
{
    DWORD           cbBuffer            = 0;
    HDEVINFO        devInfo             = {0}; 
    SP_DEVINFO_DATA devInfoData         = {0};
    DEVPROPTYPE     devPropType         = 0;
    HRESULT         hr                  = S_OK;
    PBYTE           pBuffer             = NULL;
    PROPVARIANT     pv                  = {0};
    PCWSTR          pszHwid             = NULL;

    PropVariantInit( &pv );

    //
    // First get the Function Paths (Device Instance Paths) needed to grab the
    // devnode info directly from PnP.
    //
    if( S_OK == hr )
    {
        hr = pShellItem2->GetProperty( PKEY_Devices_FunctionPaths, &pv );
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
    // we're just attempting to get one device hardware id from the devnode who's
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
        // Get the first Hardware ID from the list
        //
        pszHwid = reinterpret_cast<PWSTR>(pBuffer);
        hr = SHStrDup( pszHwid, ppszHardwareID );
    }

    //
    // Cleanup
    //
    if( NULL != pBuffer )
    {
        delete[] pBuffer;
        pBuffer = NULL;
    }

    if( INVALID_HANDLE_VALUE != devInfo )
    {
        (void) SetupDiDestroyDeviceInfoList( devInfo );
    }

    PropVariantClear( &pv );

    return hr;
}// CDeviceContextMenu::GetDeviceHardwareID


//------------------------------------------------------------------------------
// _ExecuteThreadProc
//
//      Create a new thread to show a message box
//------------------------------------------------------------------------------
DWORD WINAPI _ExecuteThreadProc( __in LPVOID pVoid )
{
    HRESULT hr                  = S_OK;
    WCHAR   szTitle[MAX_PATH]   = {0};
    
    DllIncLockCount();

    if( NULL == LoadString(
         g_hInstance, IDS_ContextMenu_Title, szTitle, ARRAYSIZE(szTitle)) )
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

    //
    // Show a message box
    //
    if( S_OK == hr )
    {
        MessageBox( NULL, reinterpret_cast<PWSTR>(pVoid), szTitle, MB_OK );
    }

    CoTaskMemFree( pVoid );

    DllDecLockCount();

    return 0;
}// _ExecuteThreadProc
