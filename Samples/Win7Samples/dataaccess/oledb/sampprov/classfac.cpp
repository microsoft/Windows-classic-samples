//--------------------------------------------------------------------
// Microsoft OLE DB Sample Provider
// (C) Copyright 1991 - 1999 Microsoft Corporation. All Rights Reserved.
//
// @doc
//
// @module CLASSFAC.CPP | The module contains the DLL Entry and Exit
// points, plus the OLE ClassFactory class.
//
//

// Includes ------------------------------------------------------------------

#include "headers.h"
#include "classfac.h"
#include "msdasc.h"			//CLSID_RootBinder

// Static vars ---------------------------------------------------------------

static const char * s_strDllName = "SampProv";  // used with GetModuleHandle
static const struct
{
	char * strRegKey;
	char * strValueName;
	DWORD  dwType;
	char * strValue;
} s_rgRegInfo[] =
{
	//CLSID_SampProv
    { "SampProv",																NULL,				REG_SZ,		"Microsoft OLE DB Sample Provider"					},
    { "SampProv\\Clsid",														NULL,				REG_SZ,		"{E8CCCB79-7C36-101B-AC3A-00AA0044773D}"			},
    { "CLSID\\{E8CCCB79-7C36-101B-AC3A-00AA0044773D}",							NULL,				REG_SZ,		"SampProv"											},
	{ "CLSID\\{E8CCCB79-7C36-101B-AC3A-00AA0044773D}",							"OLEDB_SERVICES",	REG_DWORD,	"-1"												},
    { "CLSID\\{E8CCCB79-7C36-101B-AC3A-00AA0044773D}\\ProgID",					NULL,				REG_SZ,		"SampProv"											},
    { "CLSID\\{E8CCCB79-7C36-101B-AC3A-00AA0044773D}\\VersionIndependentProgID",NULL,				REG_SZ,		"SampProv"											},
    { "CLSID\\{E8CCCB79-7C36-101B-AC3A-00AA0044773D}\\InprocServer32",			NULL,				REG_SZ,		"%s"												},
	{ "CLSID\\{E8CCCB79-7C36-101B-AC3A-00AA0044773D}\\InprocServer32",			"ThreadingModel",	REG_SZ,		"Both"												},
	{ "CLSID\\{E8CCCB79-7C36-101B-AC3A-00AA0044773D}\\OLE DB Provider",			NULL,				REG_SZ,		"Microsoft OLE DB Sample Provider"					},

	//CLSID_SampProvConnectionPage
    { "SampProvConnectionPage",													NULL,				REG_SZ,		"Microsoft OLE DB Sample Provider Connection Page"	},
    { "SampProvConnectionPage\\Clsid",											NULL,				REG_SZ,		"{119C8711-905B-11d2-AF65-00C04F6F8697}"			},
    { "CLSID\\{119C8711-905B-11d2-AF65-00C04F6F8697}",							NULL,				REG_SZ,		"SampProvConnectionPage"							},
    { "CLSID\\{119C8711-905B-11d2-AF65-00C04F6F8697}\\ProgID",					NULL,				REG_SZ,		"SampProvConnectionPage"							},
    { "CLSID\\{119C8711-905B-11d2-AF65-00C04F6F8697}\\VersionIndependentProgID",NULL,				REG_SZ,		"SampProvConnectionPage"							},
    { "CLSID\\{119C8711-905B-11d2-AF65-00C04F6F8697}\\InprocServer32",			NULL,				REG_SZ,		"%s"												},
	{ "CLSID\\{119C8711-905B-11d2-AF65-00C04F6F8697}\\InprocServer32",			"ThreadingModel",	REG_SZ,		"Both"												},

	//CLSID_SampProvAdvancedPage
    { "SampProvAdvancedPage",													NULL,				REG_SZ,		"Microsoft OLE DB Sample Provider Advanced Page"	},
    { "SampProvAdvancedPage\\Clsid",											NULL,				REG_SZ,		"{119C8712-905B-11d2-AF65-00C04F6F8697}"			},
    { "CLSID\\{119C8712-905B-11d2-AF65-00C04F6F8697}",							NULL,				REG_SZ,		"SampProvAdvancedPage"								},
    { "CLSID\\{119C8712-905B-11d2-AF65-00C04F6F8697}\\ProgID",					NULL,				REG_SZ,		"SampProvAdvancedPage"								},
    { "CLSID\\{119C8712-905B-11d2-AF65-00C04F6F8697}\\VersionIndependentProgID",NULL,				REG_SZ,		"SampProvAdvancedPage"								},
    { "CLSID\\{119C8712-905B-11d2-AF65-00C04F6F8697}\\InprocServer32",			NULL,				REG_SZ,		"%s"												},
	{ "CLSID\\{119C8712-905B-11d2-AF65-00C04F6F8697}\\InprocServer32",			"ThreadingModel",	REG_SZ,		"Both"												},


	//CLSID_SampProvBinder
	{ "SampProvBinder",															NULL,				REG_SZ,		"Microsoft OLE DB Sample Provider Binder"			},
	{ "SampProvBinder\\Clsid",													NULL,				REG_SZ,		"{245E7460-B577-11D2-AF53-00C04F782926}"			},
	{ "CLSID\\{245E7460-B577-11D2-AF53-00C04F782926}",							NULL,				REG_SZ,		"SampProvBinder"									},
	{ "CLSID\\{245E7460-B577-11D2-AF53-00C04F782926}\\ProgID",					NULL,				REG_SZ,		"SampProvBinder"									},
	{ "CLSID\\{245E7460-B577-11D2-AF53-00C04F782926}\\VersionIndependentProgID",NULL,				REG_SZ,		"SampProvBinder"									},
	{ "CLSID\\{245E7460-B577-11D2-AF53-00C04F782926}\\InprocServer32",			NULL,				REG_SZ,		"%s"												},
	{ "CLSID\\{245E7460-B577-11D2-AF53-00C04F782926}\\InprocServer32",			"ThreadingModel",	REG_SZ,		"Both"												},
	{ "CLSID\\{245E7460-B577-11D2-AF53-00C04F782926}\\OLE DB Binder",			NULL,				REG_SZ,		"Microsoft OLE DB Sample Provider Binder"			},
};

// Code ----------------------------------------------------------------------

// DllMain -------------------------------------------------------------------
//
// @func DLL Entry point where Instance and Thread attach/detach notifications
// takes place.  OLE is initialized and the IMalloc Interface pointer is obtained.
//
// @rdesc Boolean Flag
//      @flag TRUE | Successful initialization
//      @flag FALSE | Failure to intialize
//
BOOL WINAPI DllMain
    (
    HINSTANCE   hInstDLL,   //@parm IN | Application Instance Handle
    DWORD       fdwReason,  //@parm IN | Indicated Process or Thread activity
    LPVOID      lpvReserved //@parm IN | Reserved...
    )
{
    BOOL        fRetVal = FALSE;
    SYSTEM_INFO SystemInformation;

    switch (fdwReason)
        {
    case DLL_PROCESS_ATTACH:

        // Assume successfully initialized
        fRetVal = TRUE;

        // Do one-time initialization when first process attaches
        if (!g_cAttachedProcesses)
            {
            g_hInstance = hInstDLL;

            // Get the system page size
            if (!g_dwPageSize)
                {
                GetSystemInfo( &SystemInformation );    // can't fail
                g_dwPageSize = SystemInformation.dwPageSize;
                }
            }

        // Do per-process initialization here...
        
        // Remember that another process successfully attached
        g_cAttachedProcesses++;
        break;

    case DLL_PROCESS_DETACH:
        // Clean up when the last process is going away
        if (g_cAttachedProcesses == 1)
            {
            }

        // Do per-process clean up here...

        // Remember that a process has detached
        g_cAttachedProcesses--;
        break;

    case DLL_THREAD_ATTACH:
        break;

    case DLL_THREAD_DETACH:
        break;
        }

    return fRetVal;
}


// DllGetClassObject ---------------------------------------------------------
//
// @func This function is exposed to OLE so that the classfactory can
// be obtained.
//
// @rdesc HRESULT indicating status of routine
//      @flag S_OK                      | The object was retrieved successfully.
//      @flag CLASS_E_CLASSNOTAVAILABLE | DLL does not support class.
//      @flag E_OUTOFMEMORY             | Out of memory.
//      @flag E_INVALIDARG              | One or more arguments are invalid.
//      @flag E_UNEXPECTED              | An unexpected error occurred.
//      @flag OTHER | Other HRESULTs returned by called functions
//
HRESULT CALLBACK DllGetClassObject
    (
    REFCLSID    rclsid, //@parm IN | CLSID of the object class to be loaded
    REFIID      riid,   //@parm IN | Interface on object to be instantiated
    LPVOID *    ppvObj  //@parm OUT | Pointer to interface that was instantiated
    )
{
    CClassFactory * pClassFactory;
    HRESULT         hr;

    // Check for valid ppvObj pointer
    if (!ppvObj)
        return ResultFromScode( E_INVALIDARG );

    // In case we fail, we need to zero output arguments
    *ppvObj = NULL;

    // We only service CLSID_SampProv
    if (rclsid != CLSID_SampProv && 
		rclsid != CLSID_SampProvConnectionPage &&
		rclsid != CLSID_SampProvAdvancedPage &&
		rclsid != CLSID_SampProvBinder)
        return ResultFromScode( CLASS_E_CLASSNOTAVAILABLE );

    // We only support the IUnknown and IClassFactory interfaces
    if (riid != IID_IUnknown &&
        riid != IID_IClassFactory)
        return ResultFromScode( E_NOINTERFACE );

    // Create our ClassFactory object
    pClassFactory = new CClassFactory(rclsid);
    if (pClassFactory == NULL)
        return ResultFromScode( E_OUTOFMEMORY );

    // Get the desired interface on this object
    hr = pClassFactory->QueryInterface( riid, ppvObj );
    if (!SUCCEEDED( hr ))
        SAFE_DELETE( pClassFactory );

    return hr;
}


// DllCanUnloadNow -----------------------------------------------------------
//
// @func Indicates whether the DLL is no longer in use and
// can be unloaded.
//
// @rdesc HRESULT indicating status of routine
//      @flag S_OK | DLL can be unloaded now.
//      @flag S_FALSE | DLL cannot be unloaded now.
//
STDAPI DllCanUnloadNow( void )
{
    if (!g_cObj && !g_cLock)
        return ResultFromScode( S_OK );
    else
        return ResultFromScode( S_FALSE );
}


//-----------------------------------------------------------------------------
// CClassFactory
//-----------------------------------------------------------------------------

// CClassFactory -------------------------------------------------------------
//
// @mfunc Constructor for this class
//
// @rdesc NONE
//
CClassFactory::CClassFactory(REFCLSID clsid)
{
    m_cRef	= 0;
	m_clsid	= clsid;

    // Increment global object count
    OBJECT_CONSTRUCTED();
}

// ~CClassFactory ------------------------------------------------------------
//
// @mfunc Destructor for this class
//
// @rdesc NONE
//
CClassFactory:: ~CClassFactory( void )
{
    // Decrement global object count
    OBJECT_DESTRUCTED();
}

// QueryInterface ------------------------------------------------------------
//
// @mfunc Returns a pointer to a specified interface. Callers use
// QueryInterface to determine which interfaces the called object
// supports.
//
// @rdesc HRESULT indicating the status of the method
//      @flag S_OK          | Interface is supported and ppvObject is set.
//      @flag E_NOINTERFACE | Interface is not supported by the object
//      @flag E_INVALIDARG  | One or more arguments are invalid.
//
STDMETHODIMP CClassFactory::QueryInterface
    (
    REFIID      riid,   //@parm IN | Interface ID of the interface being queried for.
    LPVOID *    ppvObj  //@parm OUT | Pointer to interface that was instantiated
    )
{
    // Check for valid ppvObj pointer
    if (!ppvObj)
        return ResultFromScode( E_INVALIDARG );

    // In case we fail, we need to zero output arguments
    *ppvObj = NULL;

    // Do we support this interface?
    if (riid == IID_IUnknown ||
        riid == IID_IClassFactory)
        *ppvObj = (LPVOID) this;

    // If we're going to return an interface, AddRef it first
    if (*ppvObj)
        {
        ((LPUNKNOWN) *ppvObj)->AddRef();
        return ResultFromScode( S_OK );
        }
    else
        return ResultFromScode( E_NOINTERFACE );
}

// AddRef --------------------------------------------------------------------
//
// @mfunc Increments a persistence count for the object
//
// @rdesc Current reference count
//
STDMETHODIMP_( DBREFCOUNT ) CClassFactory::AddRef( void )
{
    return ++m_cRef;
}

// Release -------------------------------------------------------------------
//
// @mfunc Decrements a persistence count for the object and if
// persistence count is 0, the object destroys itself.
//
// @rdesc Current reference count
//
STDMETHODIMP_( DBREFCOUNT ) CClassFactory::Release( void )
{
    if (!--m_cRef)
        {
        delete this;
        return 0;
        }

    return m_cRef;
}

// CreateInstance ------------------------------------------------------------
//
// @mfunc Creates an uninitialized instance of an object class.
// Initialization is subsequently performed using another
// interface-specific method
//
// @rdesc HRESULT indicating the status of the method
//      @flag S_OK          | Interface is supported and ppvObject is set.
//      @flag E_NOINTERFACE | Interface is not supported by the object
//      @flag E_INVALIDARG  | One or more arguments are invalid.
//      @flag E_OUTOFMEMORY | Memory could not be allocated
//      @flag OTHER         | Other HRESULTs returned by called functions
//
STDMETHODIMP CClassFactory::CreateInstance
    (
    LPUNKNOWN   pUnkOuter,  //@parm IN | Points to the controlling IUnknown interface
    REFIID      riid,       //@parm IN | Interface ID of the interface being queried for.
    LPVOID *    ppvObj      //@parm OUT | Pointer to interface that was instantiated
    )
{
    HRESULT	hr						= S_OK;
    CDataSource*	pCDataSource	= NULL;
	CPropertyPage*  pCPropertyPage	= NULL;
	CBinder *		pBinder = NULL;

    // Check for valid ppvObj pointer
    if (!ppvObj)
        return E_INVALIDARG;

    // In case we fail, we need to zero output arguments
    *ppvObj = NULL;

    // If we're given a controlling IUnknown, it must ask for IUnknown.
    // Otherwise, the caller will end up getting a pointer to their pUnkOuter
    // instead of to the new object we create and will have no way of getting
    // back to this new object, so they won't be able to free it.  Bad!
    if (pUnkOuter && riid != IID_IUnknown)
        return DB_E_NOAGGREGATION;

    //CLSID_SampProv
    if(m_clsid == CLSID_SampProv)
	{
		// Create a CDataSource object
		pCDataSource = new CDataSource( pUnkOuter );
	}
	else if(m_clsid == CLSID_SampProvConnectionPage)
	{
		// Create a CDSLConnectionPage object
		pCPropertyPage = new CDSLConnectionPage();
	}
	else if(m_clsid == CLSID_SampProvAdvancedPage)
	{
		// Create a CDSLAdvancedPage object
		pCPropertyPage = new CDSLAdvancedPage();
	}
	else if(m_clsid == CLSID_SampProvBinder)
	{
		// Create a CBinder object
		pBinder = new CBinder(pUnkOuter);
	}
	else
	{
        return CLASS_E_CLASSNOTAVAILABLE;
	}

	//E_OUTOFMEMORY
	if(!pCDataSource && !pCPropertyPage && !pBinder)
	{
		return E_OUTOFMEMORY;
	}

    //Obtain the correct interface...
    if(pCDataSource)
	{
		hr = E_FAIL;
		if(pCDataSource->FInit())
			hr = pCDataSource->QueryInterface( riid, ppvObj );
	}
	else if(pBinder)
	{
		hr = E_OUTOFMEMORY;
		if(pBinder->FInit())
			hr = pBinder->QueryInterface( riid, ppvObj );
	}
	else
	{
		hr = pCPropertyPage->QueryInterface( riid, ppvObj );
	}

	if(FAILED(hr))
	{
		SAFE_DELETE(pCDataSource);
		SAFE_DELETE(pCPropertyPage);
	}

    return hr;
}

// LockServer ----------------------------------------------------------------
//
// @mfunc Controls whether an object application is kept in memory.
// Keeping the application alive in memory allows instances of this
// class to be created more quickly.
//
// @rdesc HRESULT indicating the status of the method
//      @flag S_OK | Interface is supported and ppvObject is set.
//
STDMETHODIMP CClassFactory::LockServer
    (
    BOOL fLock                  //@parm IN | TRUE or FALSE to lock or unlock
    )
{
    if (fLock)
        InterlockedIncrement( &g_cLock );
    else
        InterlockedDecrement( &g_cLock );

    return NOERROR;
}


//---------------------------------------------------------------------------
// @func    Removes keys to the registry.
//
// @rdesc Returns NOERROR
//
// @comm
// Special Notes:   This allows us to avoid using a .reg file.
//
// Note that a more robust method would be to trace existing
// CLSID and ProgID, and trace each from the other.  This would handle the
// case where either changed.  Then should probably enumerate all keys under
// the ProgID and CLSID, then delete them.
//
// Also note the problem with our exposing a different CLSID for the debug
// and ndebug versions, yet the ProgID remains the same.  Should we have
// different ProgID's also?
//
//---------------------------------------------------------------------------
STDAPI DllUnregisterServer
    (
    void
    )
{
    int     i;
    int     iNumErrors = 0;
	LONG	stat;

    // Delete all table entries.  Loop in reverse order, since they
    // are entered in a basic-to-complex order.
    // We cannot delete a key that has subkeys.
    // Ignore errors.
    for (i=NUMELEM( s_rgRegInfo ) - 1; i >= 0; i--)
        {
		stat = RegDeleteKey( HKEY_CLASSES_ROOT, s_rgRegInfo[i].strRegKey );
        if ((stat != ERROR_SUCCESS) && 
        	(stat != ERROR_FILE_NOT_FOUND) )
            iNumErrors++;
        }

	IRegisterProvider*	pIRegisterProvider = NULL;
	//Obtain the Root Binder
	if(SUCCEEDED(CoCreateInstance(CLSID_RootBinder, NULL, CLSCTX_ALL, IID_IRegisterProvider, (void**)&pIRegisterProvider)))
	{
		//Unregister SampProv's URL prefix with the RootBinder
		//NOTE:  Don't fail, since we may have never registered ourselves with the root binder
		pIRegisterProvider->UnregisterProvider(SAMPPROV_URL_PREFIX, 0, CLSID_SampProvBinder);
		pIRegisterProvider->Release();
	}

    return ResultFromScode( iNumErrors ? E_FAIL : S_OK );
}


//---------------------------------------------------------------------------
// @func    Adds necessary keys to the registry.
//
// @rdesc Returns one of the following
// @flag NOERROR    | Registration succeeded
// @flag E_FAIL     | Something didn't work
//
// @comm
// Special Notes:   This allows us to avoid using a .reg file.
// Here is what was in the SampProv.REG file of yore.
// Note that now we have two CLSID's, one for DEBUG.  Then each one
// can point to a different .DLL.
//
//---------------------------------------------------------------------------
STDAPI DllRegisterServer
    (
    void
    )
{
    HKEY        hk;
    HMODULE     hModule;
    DWORD       dwDisposition;
    LONG        stat;
	CHAR		strFileName[MAX_PATH+1];
    CHAR        strOutBuff[300+1];
	HRESULT		hr = S_OK;

    // Get the full path name for this DLL.
    if (NULL == (hModule = GetModuleHandle( s_strDllName )))
        return ResultFromScode( E_FAIL );
    if (0 == GetModuleFileName( hModule, strFileName, sizeof( strFileName ) / sizeof( char )))
        return ResultFromScode( E_FAIL );

    // Make a clean start
    DllUnregisterServer();

    // Loop through s_rgRegInfo, and put everything in it.
    // Every entry is based on HKEY_CLASSES_ROOT.
    for (ULONG i=0; i < NUMELEM( s_rgRegInfo ); i++)
        {

		// Fill in any "%s" arguments with the name of this DLL.
		if (s_rgRegInfo[i].dwType == REG_DWORD)
			*(DWORD*)strOutBuff = atol( s_rgRegInfo[i].strValue );
		else
			StringCchPrintfA( strOutBuff, sizeof(strOutBuff), s_rgRegInfo[i].strValue, strFileName );

        // Create the Key.  If it exists, we open it.
        // Thus we can still change the value below.
        stat = RegCreateKeyEx(
                HKEY_CLASSES_ROOT,
                s_rgRegInfo[i].strRegKey,
                0,  // dwReserved
                NULL,   // lpszClass
                REG_OPTION_NON_VOLATILE,
                KEY_ALL_ACCESS, // security access mask
                NULL,   // lpSecurityAttributes
                &hk,    // phkResult
                &dwDisposition );
        if (stat != ERROR_SUCCESS)
            return ResultFromScode( E_FAIL );

        stat = RegSetValueEx(
                hk,								// created above
                s_rgRegInfo[i].strValueName,	// lpszValueName
                0,								// dwReserved
                s_rgRegInfo[i].dwType,			// fdwType
				(BYTE *) strOutBuff,			// value
                s_rgRegInfo[i].dwType == REG_SZ ?
                (DWORD)strlen( strOutBuff ) + 1 :		// cbData, including null terminator
				sizeof(DWORD));					
        RegCloseKey( hk );
        if (stat != ERROR_SUCCESS)
            return ResultFromScode( E_FAIL );
        }

	IRegisterProvider*	pIRegisterProvider = NULL;
	//Register with the RootBinder
	if(SUCCEEDED(CoCreateInstance(CLSID_RootBinder, NULL, CLSCTX_ALL, IID_IRegisterProvider, (void**)&pIRegisterProvider)))
	{
		//Register Sampprov's URL prefix with the RootBinder
		hr = pIRegisterProvider->SetURLMapping(SAMPPROV_URL_PREFIX, 0, CLSID_SampProvBinder);
		pIRegisterProvider->Release();
	}

    return ResultFromScode( hr );
}

