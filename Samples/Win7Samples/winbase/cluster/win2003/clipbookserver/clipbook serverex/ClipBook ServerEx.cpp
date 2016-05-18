/////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2003 <company name>
//
//  Module Name:
//      ClipBook ServerEx.cpp
//
//  Description:
//      Implementation of the CClipBookServerApp class and DLL initialization
//      routines.
//
//  Author:
//      <name> (<e-mail name>) Mmmm DD, 2003
//
//  Revision History:
//
//  Notes:
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#pragma warning( push )
#include <initguid.h>
#include <CluAdmEx.h>
#pragma warning( pop )

#include "ClipBook ServerEx.h"
#include "ExtObj.h"
#include "BasePage.h"
#include "RegExt.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CComModule _Module;

BEGIN_OBJECT_MAP( ObjectMap )
    OBJECT_ENTRY( CLSID_CoClipBookServerEx, CExtObject )
END_OBJECT_MAP()

/////////////////////////////////////////////////////////////////////////////
// Global Function Prototypes
/////////////////////////////////////////////////////////////////////////////

STDAPI
DllCanUnloadNow( void );

STDAPI
DllGetClassObject(
      REFCLSID  rclsidIn
    , REFIID    riidIn
    , LPVOID *  ppvOut
    );

STDAPI
DllRegisterServer( void );

STDAPI
DllUnregisterServer( void );

STDAPI
DllRegisterCluAdminExtension(
    HCLUSTER hClusterIn
    );

STDAPI
DllUnregisterCluAdminExtension(
    HCLUSTER hClusterIn
    );


/////////////////////////////////////////////////////////////////////////////
// class CClipBookServerApp
/////////////////////////////////////////////////////////////////////////////

class CClipBookServerApp : public CWinApp
{
public:
    virtual BOOL InitInstance();
    virtual int ExitInstance();

}; //*** class CClipBookServerApp

/////////////////////////////////////////////////////////////////////////////
// The one and only CClipBookServerApp object

CClipBookServerApp theApp;

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CClipBookServerApp::InitInstance
//
//  Description:
//      Initialize this instance of the application.
//
//  Arguments:
//      None.
//
//  Return Value:
//      Any return codes from CWinApp::InitInstance().
//
//--
/////////////////////////////////////////////////////////////////////////////
BOOL
CClipBookServerApp::InitInstance( void )
{
    _Module.Init( ObjectMap, m_hInstance );

    return CWinApp::InitInstance();

}  //*** CClipBookServerApp::InitInstance

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CClipBookServerApp::ExitInstance
//
//  Description:
//      Deinitialize this instance of the application.
//
//  Arguments:
//      None.
//
//  Return Value:
//      Any return codes from CWinApp::ExitInstance().
//
//--
/////////////////////////////////////////////////////////////////////////////
int
CClipBookServerApp::ExitInstance( void )
{
    _Module.Term();

    return CWinApp::ExitInstance();

}  //*** CClipBookServerApp::ExitInstance

/////////////////////////////////////////////////////////////////////////////
//++
//
//  FormatError
//
//  Description:
//      Format an error.
//
//  Arguments:
//      rstrError   String in which to return the error message.
//      dwError     Error code to format.
//
//  Return Value:
//      None.
//
//--
/////////////////////////////////////////////////////////////////////////////
void
FormatError(
      CString & rstrErrorInout
    , DWORD     dwErrorIn
    )
{
    DWORD   cch;
    TCHAR   szError[ 512 ];

    //
    // Format the NT status code from CLUSAPI.  This is necessary
    // for the cases where cluster messages haven't been added to
    // the system message file yet.
    //

    cch = FormatMessage(
                      FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS
                    , ::GetModuleHandle( _T( "CLUSAPI.DLL" ) )
                    , dwErrorIn
                    , MAKELANGID( LANG_NEUTRAL, SUBLANG_NEUTRAL )
                    , szError
                    , RTL_NUMBER_OF( szError )
                    , 0
                    );
    if ( cch == 0 )
    {
        cch = FormatMessage(
                          FORMAT_MESSAGE_FROM_SYSTEM
                        , NULL
                        , dwErrorIn
                        , MAKELANGID( LANG_NEUTRAL, SUBLANG_NEUTRAL )
                        , szError
                        , RTL_NUMBER_OF( szError )
                        , 0
                        );
        if ( cch == 0 )
        {
            //
            // Format the NT status code from NTDLL since this hasn't been
            // integrated into the system yet.
            //

            cch = FormatMessage(
                              FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS
                            , ::GetModuleHandle( _T( "NTDLL.DLL" ) )
                            , dwErrorIn
                            , MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL)
                            , szError
                            , RTL_NUMBER_OF( szError )
                            , 0
                            );
        }  // if:  error formatting status code from system
    }  // if:  error formatting status code from ClusApi

    if ( cch > 0 )
    {
        rstrErrorInout = szError;
    }  // if:  no error
    else
    {
        TRACE( _T( "FormatError() - Error 0x%08.8x formatting string for error code 0x%08.8x\n" ), GetLastError(), dwErrorIn );
        rstrErrorInout.Format( _T( "Error 0x%08.8x" ), dwErrorIn );
    }  // else:  error formatting the message

}  //*** FormatError

/////////////////////////////////////////////////////////////////////////////
//++
//
//  DllCanUnloadNow
//
//  Description:
//      Used to determine whether the DLL can be unloaded by COM.
//
//  Arguments:
//      None.
//
//  Return Value:
//      S_OK
//          Dll can be unloaded now.
//
//      S_FALSE
//          Dll cannot be unloaded now...
//
//--
/////////////////////////////////////////////////////////////////////////////
STDAPI
DllCanUnloadNow( void )
{
    AFX_MANAGE_STATE( AfxGetStaticModuleState() );

    return ( AfxDllCanUnloadNow() && _Module.GetLockCount() == 0 ) ? S_OK : S_FALSE;

}  //*** DllCanUnloadNow

/////////////////////////////////////////////////////////////////////////////
//++
//
//  DllGetClassObject
//
//  Description:
//      Returns a class factory to create an object of the requested type
//
//  Arguments:
//      rclsidIn    - CLSID of the class to create.
//      riidIn      - Requested interface of the class.
//      ppvOut      - Pointer to send the class back to the caller.
//
//  Return Value:
//      S_OK
//          Success.
//
//      Other errors as HRESULTs...
//
//--
/////////////////////////////////////////////////////////////////////////////
STDAPI
DllGetClassObject(
      REFCLSID  rclsidIn
    , REFIID    riidIn
    , LPVOID *  ppvOut
    )
{
    return _Module.GetClassObject( rclsidIn, riidIn, ppvOut );

}  //*** DllGetClassObject

/////////////////////////////////////////////////////////////////////////////
//++
//
//  DllRegisterServer
//
//  Description:
//      Perform all relevant COM registration for this component.
//
//  Arguments:
//      None.
//
//  Return Value:
//      S_OK
//          Success.
//
//      Other errors as HRESULTs...
//
//--
/////////////////////////////////////////////////////////////////////////////
STDAPI
DllRegisterServer( void )
{
    HRESULT hr = S_OK;

    //
    //  Registers object, typelib and all interfaces in typelib
    //

    hr = _Module.RegisterServer( FALSE /*bRegTypeLib*/ );

    return hr;

}  //*** DllRegisterServer

/////////////////////////////////////////////////////////////////////////////
//++
//
//  DllRegisterServer
//
//  Description:
//      Perform all relevant COM un-registration for this component.
//
//  Arguments:
//      None.
//
//  Return Value:
//      S_OK
//          Success.
//
//      Other errors as HRESULTs...
//
//--
/////////////////////////////////////////////////////////////////////////////
STDAPI
DllUnregisterServer( void )
{
    HRESULT hr = S_OK;

    hr = _Module.UnregisterServer();

    return hr;

}  //*** DllUnregisterServer

/////////////////////////////////////////////////////////////////////////////
//++
//
//  DllRegisterCluAdminExtension
//
//  Description:
//      Register the extension with the cluster database.
//
//  Arguments:
//      hClusterIn      Handle to the cluster to modify.
//
//  Return Value:
//      S_OK            Extension registered successfully.
//      Win32 error code if another failure occurred.
//
//--
/////////////////////////////////////////////////////////////////////////////
STDAPI
DllRegisterCluAdminExtension(
    HCLUSTER hClusterIn
    )
{
    DWORD       sc = ERROR_SUCCESS;
    DWORD       scTemp = ERROR_SUCCESS;
    LPCWSTR     pwszResTypes = g_wszResourceTypeNames;

    AFX_MANAGE_STATE( AfxGetStaticModuleState() );

    //
    // Register to extend our resource type(s).
    //

    while ( *pwszResTypes != L'\0' )
    {
        wprintf( L"  %s\n", pwszResTypes );
        scTemp = RegisterCluAdminResourceTypeExtension(
                      hClusterIn
                    , pwszResTypes
                    , &CLSID_CoClipBookServerEx
                    );
        if ( scTemp != ERROR_SUCCESS )
        {
            sc = scTemp;
        } // if:  error registering the extension

        pwszResTypes += lstrlenW( pwszResTypes ) + 1;
    }  // while:  more resource types

    if ( sc != ERROR_SUCCESS )
    {
        goto Cleanup;
    } // if:

Cleanup:

    return HRESULT_FROM_WIN32( sc );

}  //*** DllRegisterCluAdminExtension

/////////////////////////////////////////////////////////////////////////////
//++
//
//  DllUnregisterCluAdminExtension
//
//  Description:
//      Unregister the extension with the cluster database.
//
//  Arguments:
//      hClusterIn      Handle to the cluster to modify.
//
//  Return Value:
//      S_OK            Extension unregistered successfully.
//      Win32 error code if another failure occurred.
//
//--
/////////////////////////////////////////////////////////////////////////////
STDAPI
DllUnregisterCluAdminExtension(
    HCLUSTER hClusterIn
    )
{
    DWORD       sc = ERROR_SUCCESS;
    DWORD       scTemp = ERROR_SUCCESS;
    LPCWSTR     pwszResTypes = g_wszResourceTypeNames;

    AFX_MANAGE_STATE( AfxGetStaticModuleState() );


    //
    // Unregister to extend our resource type(s).
    //

    while ( *pwszResTypes != L'\0' )
    {
        wprintf( L"  %s\n", pwszResTypes );
        scTemp = UnregisterCluAdminResourceTypeExtension(
                      hClusterIn
                    , pwszResTypes
                    , &CLSID_CoClipBookServerEx
                    );
        if ( scTemp != ERROR_SUCCESS )
        {
            sc = scTemp;
        } // if:  error unregistering the extension

        pwszResTypes += lstrlenW( pwszResTypes ) + 1;
    }  // while:  more resource types

    if ( sc != ERROR_SUCCESS )
    {
        goto Cleanup;
    } // if:

Cleanup:

    return HRESULT_FROM_WIN32( sc );

}  //*** DllUnregisterCluAdminExtension
