//
// dllmain.cpp
//
// DllMain module entry point.
//

#include "globals.h"

//+---------------------------------------------------------------------------
//
// DllMain
//
//----------------------------------------------------------------------------

BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID pvReserved)
{
    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:

            g_hInst = hInstance;

            if (!InitializeCriticalSectionAndSpinCount(&g_cs, 0))
                return FALSE;

            break;

        case DLL_PROCESS_DETACH:

            DeleteCriticalSection(&g_cs);

            break;
    }

    return TRUE;
}
