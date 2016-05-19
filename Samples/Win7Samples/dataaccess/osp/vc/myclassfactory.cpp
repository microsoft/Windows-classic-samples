//--------------------------------------------------------------------
// Microsoft OLE DB Sample OLEDB Simple Provider
// (C) Copyright 1991 - 1999 Microsoft Corporation.  All Rights Reserved.
//
// module MyClassFactory.cpp | The module contains the DLL Entry and Exit
// points, plus the OLE ClassFactory class.
//
//


////////////////////////////////////////////////////////
// Includes
//
////////////////////////////////////////////////////////
#include "Common.h"
#include "MyClassFactory.h"


// Added GUID constant for the sample object:
const GUID CLSID_ospsampc = {0x1e79b2c1,0x77b,0x11d1,{0xb3,0xae,0x0,0xaa,0x0,0xc1,0xa9,0x24}};

////////////////////////////////////////////////////////
// Globals
//
////////////////////////////////////////////////////////
LONG g_cObj	= 0L;						// # of outstanding objects
LONG g_cLock = 0L;						// # of explicit locks set
DWORD g_cAttachedProcesses = 0L;		// # of attached processes
HINSTANCE g_hInstance = NULL;


REGENTRY rgRegInfo[] =
{
	//SampleOSProvider
    { HKEY_CLASSES_ROOT, "ospsampc",			NULL,	"Microsoft Sample OLE DB Simple Provider DLL (C++)"							},

	//CLSID
    { HKEY_CLASSES_ROOT, "ospsampc\\Clsid",		NULL,	"{1E79B2C1-077B-11d1-B3AE-00AA00C1A924}"									},
    { HKEY_CLASSES_ROOT, "CLSID\\{1E79B2C1-077B-11d1-B3AE-00AA00C1A924}",							NULL,				"ospsampc"	},
    { HKEY_CLASSES_ROOT, "CLSID\\{1E79B2C1-077B-11d1-B3AE-00AA00C1A924}\\ProgID",					NULL,				"ospsampc"	},
    { HKEY_CLASSES_ROOT, "CLSID\\{1E79B2C1-077B-11d1-B3AE-00AA00C1A924}\\VersionIndependentProgID", NULL,				"ospsampc"	},
    { HKEY_CLASSES_ROOT, "CLSID\\{1E79B2C1-077B-11d1-B3AE-00AA00C1A924}\\InprocServer32",			"ThreadingModel",	"Both"		},
    { HKEY_CLASSES_ROOT, "CLSID\\{1E79B2C1-077B-11d1-B3AE-00AA00C1A924}\\InprocServer32",			NULL,				"%s"		},
};


////////////////////////////////////////////////////////
// MyClassFactory
//
////////////////////////////////////////////////////////
MyClassFactory::MyClassFactory()
{
    m_cRef = 0;
	InterlockedIncrement(&g_cObj);
}

MyClassFactory::~MyClassFactory()
{
	InterlockedDecrement(&g_cObj);
}


STDMETHODIMP MyClassFactory::QueryInterface(REFIID	riid, void** ppv)
{
    if(ppv == NULL)
        return E_INVALIDARG;

    // Do we support this interface?
    if (riid == IID_IUnknown ||
        riid == IID_IClassFactory)
    {
		*ppv = this;
	}
    else
	{
		*ppv = NULL;
        return E_NOINTERFACE;
	}

    if ((IUnknown*)*ppv)
	{
		((IUnknown*)*ppv)->AddRef();
	} 
    return S_OK;
}


STDMETHODIMP MyClassFactory::CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppv)
{
	HRESULT hr = S_OK;
	MyDataSource* pMyDataSource = NULL;

    if(ppv == NULL)
        return E_INVALIDARG;

    // In case we fail, we need to zero output arguments
    *ppv = NULL;

    // If we're given a controlling IUnknown, it must ask for IUnknown.
    // Otherwise, the caller will end up getting a pointer to their pUnkOuter
    // instead of to the new object we create and will have no way of getting
    // back to this new object, so they won't be able to free it.  Bad!
    if (pUnkOuter && riid != IID_IUnknown)
        return E_NOINTERFACE;

    //Create a MyDataSource object
   if(!(pMyDataSource = new MyDataSource()))
    {
		hr = E_OUTOFMEMORY;
		goto CLEANUP;
	}

    // Initialize it
	pMyDataSource->AddRef();
	if(FAILED(hr = pMyDataSource->Init()))
		goto CLEANUP;
		
	//Obtain correct riid
	hr = pMyDataSource->QueryInterface(riid, ppv);

CLEANUP:
	if((pMyDataSource)) { (pMyDataSource)->Release(); (pMyDataSource) = NULL; }
	return hr;
}


STDMETHODIMP MyClassFactory::LockServer(BOOL fLock)
{
    if (fLock)
        InterlockedIncrement( &g_cLock );
    else
        InterlockedDecrement( &g_cLock );

    return NOERROR;
}



////////////////////////////////////////////////////////
// DllMain
//
////////////////////////////////////////////////////////
BOOL WINAPI DllMain
    (
    HINSTANCE   hInstDLL,   //@parm IN | Application Instance Handle
    DWORD       fdwReason,  //@parm IN | Indicated Process or Thread activity
    LPVOID      lpvReserved //@parm IN | Reserved...
    )
{
    BOOL	fRetVal = FALSE;

    switch(fdwReason)
    {
		case DLL_PROCESS_ATTACH:

			// Assume successfully initialized
			fRetVal = TRUE;

			// Do one-time initialization when first process attaches
			if (!g_cAttachedProcesses)
			{
				g_hInstance = hInstDLL;
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



////////////////////////////////////////////////////////
// DllGetClassObject
//
////////////////////////////////////////////////////////
HRESULT CALLBACK DllGetClassObject
    (
    REFCLSID    rclsid, //@parm IN | CLSID of the object class to be loaded
    REFIID      riid,   //@parm IN | Interface on object to be instantiated
    LPVOID *    ppvObj  //@parm OUT | Pointer to interface that was instantiated
    )
{
    MyClassFactory * pClassFactory;
	HRESULT         hr;

    // Check for valid ppvObj pointer
    if (!ppvObj)
        return E_INVALIDARG;

    // In case we fail, we need to zero output arguments
    *ppvObj = NULL;

    // We only service CLSID_SampleOSProvider
    if (rclsid != CLSID_ospsampc)
        return CLASS_E_CLASSNOTAVAILABLE;

    // We only support the IUnknown and IClassFactory interfaces
    if (riid != IID_IUnknown &&
        riid != IID_IClassFactory)
        return E_NOINTERFACE;

    // Create our ClassFactory object
    if(!(pClassFactory = new MyClassFactory()))
        return E_OUTOFMEMORY;

    // Get the desired interface on this object
    hr = pClassFactory->QueryInterface( riid, ppvObj );
    if(FAILED(hr))
        delete pClassFactory;

	return hr;
}


////////////////////////////////////////////////////////
// DllCanUnloadNow
//
////////////////////////////////////////////////////////
STDAPI DllCanUnloadNow( void )
{
    if (!g_cObj && !g_cLock)
        return S_OK;
    else
        return S_FALSE;
}



////////////////////////////////////////////////////////
// DllUnregisterServer
//
////////////////////////////////////////////////////////
STDAPI DllUnregisterServer()
{
    int     iNumErrors = 0;

    // Delete all table entries.  Loop in reverse order, since they
    // are entered in a basic-to-complex order.
    // We cannot delete a key that has subkeys.
    // Ignore errors.
    for (int i=NUMELEM(rgRegInfo)-1; i>=0; i--)
    {
		if(FAILED(DelRegEntry(&rgRegInfo[i])))
			iNumErrors++;
	}

    return iNumErrors ? E_FAIL : S_OK;
}


////////////////////////////////////////////////////////
// DllRegisterServer
//
////////////////////////////////////////////////////////
STDAPI DllRegisterServer()
{
	HRESULT		hr;
    HMODULE     hModule;
    CHAR		szBuffer[MAX_NAME_LEN];
	CHAR		szFullFileName[MAX_PATH+1];

    // Get the full path name for this DLL.
    if (NULL == (hModule = GetModuleHandle( "ospsampc" )))
        return E_FAIL;
    if (0 == GetModuleFileName( hModule, szFullFileName, MAX_PATH+1))
        return E_FAIL;

    // Make a clean start
    DllUnregisterServer();

    // Loop through s_rgRegInfo, and put everything in it.
    // Every entry is based on HKEY_CLASSES_ROOT.
    for (int i=0; i<NUMELEM(rgRegInfo); i++)
    {
        // Fill in any "%s" arguments with the name of this DLL.
        wsprintf(szBuffer, rgRegInfo[i].szValue, szFullFileName );
		rgRegInfo[i].szValue = szBuffer;

		//Set the Registry Entry for this Key
        if(FAILED(hr = SetRegEntry(&rgRegInfo[i])))
			goto CLEANUP;
	}

CLEANUP:
    return hr;
}




