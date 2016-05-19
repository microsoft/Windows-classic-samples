// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
// DeviceDll.cpp : Implementation of DLL Exports from DeviceDll.def


#include <stdio.h>
#include <atlbase.h> 

#include "DeviceDll.h"


//object that implements the COM Server
CComModule _Module;

#include <atlcom.h>     //has the Macro BEGIN_OBJECT_MAP etc.
#include <objbase.h>
#include <upnphost.h>
#include "resource.h"
#include "dimmerdevice.h"
#include "DimmerService.h"
#include "DimmerDeviceDCO.h"


BEGIN_OBJECT_MAP(ObjectMap)	
   OBJECT_ENTRY(CLSID_UPNPSampleDimmerDevice, UPNPDimmerDevice)
END_OBJECT_MAP()


/////////////////////////////////////////////////////////////////////////////
// DLL Entry Point
/////////////////////////////////////////////////////////////////////////////
extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID /*lpReserved*/)
{
   if (dwReason == DLL_PROCESS_ATTACH)
   {
      _Module.Init(ObjectMap, hInstance);
      DisableThreadLibraryCalls(hInstance);
   }
   else 
      if (dwReason == DLL_PROCESS_DETACH)
         _Module.Term();

   return TRUE;    // ok
}



/////////////////////////////////////////////////////////////////////////////
// Used to determine whether the DLL can be unloaded by OLE
/////////////////////////////////////////////////////////////////////////////
STDAPI DllCanUnloadNow(void)
{
   return (_Module.GetLockCount()==0) ? S_OK : S_FALSE;
}




/////////////////////////////////////////////////////////////////////////////
// Returns a class factory to create an object of the requested type
/////////////////////////////////////////////////////////////////////////////
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
   return _Module.GetClassObject(rclsid, riid, ppv);
}



/////////////////////////////////////////////////////////////////////////////
// DllRegisterServer - Adds entries to the system registry
/////////////////////////////////////////////////////////////////////////////
STDAPI DllRegisterServer(void)
{
   // registers object, typelib and all interfaces in typelib
   return _Module.RegisterServer(TRUE);
}



/////////////////////////////////////////////////////////////////////////////
// DllUnregisterServer - Removes entries from the system registry
////////////////////////////////////////////////////////////////////////////
STDAPI DllUnregisterServer(void)
{
   return _Module.UnregisterServer(FALSE);
}
