/*++

Copyright (c) Microsoft Corporation

Module Name:

    enclave.c

Abstract:

    Defines the code that will be loaded into the VSM enclave

Author:

    Akash Trehan (aktrehan@microsoft.com) 01-30-2024

--*/

#include "precomp.h"

BOOL
DllMain(
    _In_ HINSTANCE hinstDLL,
    _In_ DWORD dwReason,
    _In_ LPVOID lpvReserved
)
{
    UNREFERENCED_PARAMETER(hinstDLL);
    UNREFERENCED_PARAMETER(lpvReserved);

    if (dwReason == DLL_PROCESS_ATTACH) {
        InitialCookie = 0xDADAF00D;
    }

    return TRUE;
}

__declspec(dllexport)
ULONG_PTR
CallEnclaveTest(
    _In_ ULONG_PTR Context
)
{
    WCHAR String[32];
    swprintf_s(String, ARRAYSIZE(String), L"%s\n", L"CallEnclaveTest started");
    OutputDebugStringW(String);

    return Context ^ InitialCookie;
}