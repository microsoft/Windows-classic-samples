/* Copyright (c) Microsoft Corporation. All rights reserved. */

#include "windows.h"
#include "radutil.h"

extern LPCWSTR pwszDllType;

static HMODULE hModule;

BOOL
WINAPI
DllMain(
   HINSTANCE hInstance,
   DWORD dwReason,
   LPVOID lpReserved
   )
{
   if (dwReason == DLL_PROCESS_ATTACH)
   {
      hModule = hInstance;
      DisableThreadLibraryCalls(hInstance);
   }

   return TRUE;
}

STDAPI
DllRegisterServer( VOID )
{
   return RadiusExtensionInstall(hModule, pwszDllType, TRUE);
}

STDAPI
DllUnregisterServer( VOID )
{
   return RadiusExtensionInstall(hModule, pwszDllType, FALSE);
}
