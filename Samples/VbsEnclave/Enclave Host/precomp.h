/*++

Copyright (c) Microsoft Corporation

Module Name:

    precomp.h

Abstract:

    Contians imports and function prototypes for enclapp.c

Author:

    Akash Trehan (aktrehan@microsoft.com) 01-30-2024

--*/

#pragma once

#include <windows.h>
#include <stdio.h>
#include <crtdbg.h>

UCHAR TestOwnerID[IMAGE_ENCLAVE_LONG_ID_LENGTH] = { 0x10, 0x20, 0x30, 0x40, 0x41, 0x31, 0x21, 0x11 };

#define FAILIF(expr) \
    do { if (expr) { _ASSERT_EXPR (FALSE, #expr); goto FailIf; } } while (0)

PVOID
CreateTestEnclave(
    _In_opt_ PVOID Base,
    _In_ SIZE_T Size,
    _In_ PUCHAR OwnerID
);

BOOL
InitializeTestEnclave(
    _In_ PVOID Base,
    _In_ ULONG Threadcount
);

PVOID
LocateTestEnclaveExport(
    _In_ PVOID Base,
    _In_ const char* Name
);

BOOL
CallTestEnclave(
    _In_ PVOID Function,
    _In_ BOOL Wait,
    _In_ ULONG_PTR Input,
    _Out_ PULONG_PTR Output
);
