/*++

Copyright (c) 1999-2001 Microsoft Corporation

Module Name:

    pluggable.cpp

Abstract:

    Registration and DLL entry points.

--*/

#include "stdafx.h"
#include <termmgr.h>

#include "PlgTerm.h"

//GUID for our terminal
#include <initguid.h>
#include "GUIDs.h"


#ifdef _MERGE_PROXYSTUB
extern "C" HINSTANCE hProxyDll;
#endif

#ifdef DEBUG_HEAPS
// for heap debugging
#include <crtdbg.h>
#endif // DEBUG_HEAPS

CComModule _Module;

// Must have an entry here for each cocreatable object.

BEGIN_OBJECT_MAP(ObjectMap)
    OBJECT_ENTRY(CLSID_PlgTermSample, CPlgTermSample)
END_OBJECT_MAP()

/*++
PTRegisterPluggable

Description:
    Register the terminal class
--*/

HRESULT PTRegisterPluggable( BSTR bstrSuperclassCLSID)
{
    LOG((MSP_TRACE, "PTRegisterPluggable enter"));

    HRESULT hr=E_FAIL;
    CComPtr<ITPluggableTerminalClassRegistration> pITPTClassReg=NULL;
    hr=CoCreateInstance(CLSID_PluggableTerminalRegistration,
							NULL,
							CLSCTX_INPROC_SERVER,
							IID_ITPluggableTerminalClassRegistration,
							reinterpret_cast<void**>(&pITPTClassReg));
    if(FAILED(hr))
    {
        LOG((MSP_ERROR, "CoCreateInstance - ITPluggableTerminalClassRegistration 0x%08x", hr));
        return hr;
    }

    //
    // Get the TerminalClass id
    //

    LPOLESTR pszTerminalCLSID = NULL;
    hr = StringFromCLSID(CLSID_PlgTermSampleClass, &pszTerminalCLSID);
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "StringFromCLSID 0x%08x", hr));
        return hr;
    }

    hr=pITPTClassReg->put_TerminalClass(pszTerminalCLSID);
    CoTaskMemFree(pszTerminalCLSID);
    if(FAILED(hr))
    {
        LOG((MSP_ERROR, "ITPluggableTerminalClassRegistration::put_TerminalClass 0x%08x", hr));
        return hr;
    }

    //
    // Get terminal's com class id
    //

    LPOLESTR pszCOMClassID = NULL;
    hr = StringFromCLSID(CLSID_PlgTermSample, &pszCOMClassID);
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "StringFromCLSID 0x%08x", hr));
        return hr;
    }

    hr=pITPTClassReg->put_CLSID(pszCOMClassID);
    CoTaskMemFree(pszCOMClassID);
    if(FAILED(hr))
    {
        LOG((MSP_ERROR, "ITPluggableTerminalClassRegistration::put_CLSID 0x%08x", hr));
        return hr;
    }


    //
    // Set other terminal attributes - don't care if FAILs or bstrTemp == NULL
    //

    //
    //name
    //
    BSTR bstrTemp = SysAllocString(SZTERMNAME);
    pITPTClassReg->put_Name(bstrTemp);
    ::SysFreeString(bstrTemp);

    //
    //company
    //
    bstrTemp = SysAllocString( _T("Sample SDK") );
    pITPTClassReg->put_Company(bstrTemp);
    ::SysFreeString(bstrTemp);

    //
    //version
    //
    bstrTemp = SysAllocString( _T("1.2") );
    pITPTClassReg->put_Version(bstrTemp);
    ::SysFreeString(bstrTemp);

    //
    //direction
    //
    TMGR_DIRECTION   td = TMGR_TD_RENDER;
    pITPTClassReg->put_Direction(td);

    //
    //media type
    //
    long lMediaType=TAPIMEDIATYPE_AUDIO;
    pITPTClassReg->put_MediaTypes(lMediaType);

    //
    // Register terminal
    //
    hr = pITPTClassReg->Add( bstrSuperclassCLSID );
    if(FAILED(hr))
    {
        LOG((MSP_ERROR, "ITPluggableTerminalClassRegistration::Add 0x%08x", hr));
    }
    LOG((MSP_TRACE, "PTRegisterPluggable exit - 0x%08x", hr));
    return hr;
}


/*++
PTRegister

Description:
    Register the terminal superclass
	then call the terminal class registration
--*/
HRESULT PTRegister()
{
    
    LOG((MSP_TRACE, "PTRegister enter"));

    HRESULT hr=E_FAIL;

    //
    // need to register superclass terminals
    //
    CComPtr<ITPluggableTerminalSuperclassRegistration> pITPTSuperReg=NULL;
    hr=CoCreateInstance(CLSID_PluggableSuperclassRegistration,
							NULL,
							CLSCTX_INPROC_SERVER,
							IID_ITPluggableTerminalSuperclassRegistration,
							reinterpret_cast<void**>(&pITPTSuperReg));
    if(FAILED(hr))
    {
        LOG((MSP_ERROR, "CoCreateInstance - ITPluggableTerminalSuperclassRegistration 0x%08x", hr));
        return hr;
    }

    //
    //we need to put_CLSID - first operation - need for GetTerminalSuperclassInfo
    //
    LPOLESTR bstrSuperCLSID = NULL;
    hr = StringFromCLSID(CLSID_PlgSuperSampleClass, &bstrSuperCLSID);
	
    if(FAILED(hr))
    {
        LOG((MSP_ERROR, "StringFromCLSID 0x%08x", hr));
        return hr;
    }

    hr=pITPTSuperReg->put_CLSID(bstrSuperCLSID);

    //
    //clean up
    //
    CoTaskMemFree(bstrSuperCLSID);
    bstrSuperCLSID=NULL;

    if(FAILED(hr))
    {
        LOG((MSP_ERROR, "ITPluggableTerminalSuperclassRegistration::put_CLSID 0x%08x", hr));
        return hr;
    }
    if(FAILED(pITPTSuperReg->GetTerminalSuperclassInfo()))
    {
	//
	//it's ok just add one
	//
	BSTR bstrName = SysAllocString( _T("PluggableTerminalsSamples") );
        if(bstrName==NULL)
        {
            LOG((MSP_ERROR, "SysAllocString failed"));
            return E_OUTOFMEMORY;
        }
        //
        //try to put name
        //
        pITPTSuperReg->put_Name(bstrName);
        //
        //clean up
        //
        ::SysFreeString(bstrName);
        bstrName=NULL;

        hr=pITPTSuperReg->Add();
        if(FAILED(hr))
        {
            LOG((MSP_ERROR, "ITPluggableTerminalSuperclassRegistration::Add 0x%08x", hr));
            return hr;
        }
    }

    hr = StringFromCLSID(CLSID_PlgSuperSampleClass, &bstrSuperCLSID);
	
    if(FAILED(hr))
    {
        LOG((MSP_ERROR, "StringFromCLSID 0x%08x", hr));
        return hr;
    }

    //
    //register the class
    //
    hr=PTRegisterPluggable(bstrSuperCLSID);

    //
    //clean up
    //
    CoTaskMemFree(bstrSuperCLSID);
    bstrSuperCLSID=NULL;
	
    if(FAILED(hr))
    {
        LOG((MSP_ERROR, "PTRegisterPluggable 0x%08x", hr));
    }

    LOG((MSP_TRACE, "PTRegisterPluggable exit - 0x%08x", hr));
    return hr;
}

/*++
PTUnregister

Description:
    UnRegister the terminal class, terminal superclass
--*/
HRESULT PTUnregister()
{
    LOG((MSP_TRACE, "PTUnregister enter"));

    HRESULT hr=E_FAIL;
    CComPtr<ITPluggableTerminalClassRegistration> pITPTClassReg=NULL;
    hr=CoCreateInstance(CLSID_PluggableTerminalRegistration,
							NULL,
							CLSCTX_INPROC_SERVER,
							IID_ITPluggableTerminalClassRegistration,
							reinterpret_cast<void**>(&pITPTClassReg));
    if(FAILED(hr))
    {
        LOG((MSP_ERROR, "CoCreateInstance - ITPluggableTerminalClassRegistration 0x%08x", hr));
        return hr;
    }

    //
    // Get the TerminalClass id
    //

    LPOLESTR pszTerminalCLSID = NULL;
    hr = StringFromCLSID(CLSID_PlgTermSampleClass, &pszTerminalCLSID);
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "StringFromCLSID 0x%08x", hr));
        return hr;
    }

    hr=pITPTClassReg->put_TerminalClass(pszTerminalCLSID);
    CoTaskMemFree(pszTerminalCLSID);
    if(FAILED(hr))
    {
        LOG((MSP_ERROR, "ITPluggableTerminalClassRegistration::put_TerminalClass 0x%08x", hr));
        return hr;
    }

    //
    //CLSID for superclass
    //

    LPOLESTR bstrSuperCLSID = NULL;
    hr = StringFromCLSID(CLSID_PlgSuperSampleClass, &bstrSuperCLSID);
	
    if(FAILED(hr))
    {
        LOG((MSP_ERROR, "StringFromCLSID 0x%08x", hr));
        return hr;
    }

    hr=pITPTClassReg->Delete(bstrSuperCLSID);

    //
    //clean up
    //
    CoTaskMemFree(bstrSuperCLSID);
    bstrSuperCLSID=NULL;

    LOG((MSP_TRACE, "PTUnregister exit - 0x%08x", hr));
    return hr;

}


/////////////////////////////////////////////////////////////////////////////
// DLL Entry Point

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
    lpReserved;

#ifdef _MERGE_PROXYSTUB
    if (!PrxDllMain(hInstance, dwReason, lpReserved))
    {
        return FALSE;
    }
#endif

    if (dwReason == DLL_PROCESS_ATTACH)
    {

#ifdef DEBUG_HEAPS
        // turn on leak detection on process exit
        _CrtSetDbgFlag( _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF );
#endif // DEBUG_HEAPS
        
        _Module.Init(ObjectMap, hInstance);
        DisableThreadLibraryCalls(hInstance);
        MSPLOGREGISTER(_T("sampleterm"));

    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
        MSPLOGDEREGISTER();
        _Module.Term();
    }
    return TRUE;    // ok
}

/////////////////////////////////////////////////////////////////////////////
// Used to determine whether the DLL can be unloaded by OLE

STDAPI DllCanUnloadNow(void)
{
#ifdef _MERGE_PROXYSTUB
    if ( PrxDllCanUnloadNow() != S_OK )
    {
        return S_FALSE;
    }
#endif

    if ( _Module.GetLockCount() == 0 )
    {
        //
        // All references to COM objects in this DLL have been released, so
        // the DLL can now be safely unloaded. After this returns, DllMain
        // will be called with dwReason == DLL_PROCESS_DETACH.
        //
        return S_OK;
    }
    else
    {
        return S_FALSE;
    }
}

/////////////////////////////////////////////////////////////////////////////
// Returns a class factory to create an object of the requested type

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
#ifdef _MERGE_PROXYSTUB
    if ( PrxDllGetClassObject(rclsid, riid, ppv) == S_OK )
    {
        return S_OK;
    }
#endif
    return _Module.GetClassObject(rclsid, riid, ppv);
}

/////////////////////////////////////////////////////////////////////////////
// DllRegisterServer - Adds entries to the system registry

STDAPI DllRegisterServer(void)
{
    //
    // Register terminals
    //
    PTRegister();

#ifdef _MERGE_PROXYSTUB
    HRESULT hRes = PrxDllRegisterServer();

    if ( FAILED(hRes) )
    {
        return hRes;
    }
#endif

    // registers object, typelib and all interfaces in typelib
    return _Module.RegisterServer(TRUE);
}

/////////////////////////////////////////////////////////////////////////////
// DllUnregisterServer - Removes entries from the system registry

STDAPI DllUnregisterServer(void)
{
    //
    // Unregister terminals
    //
    PTUnregister();

#ifdef _MERGE_PROXYSTUB
    PrxDllUnregisterServer();
#endif

    _Module.UnregisterServer();
    
    return S_OK;
}


