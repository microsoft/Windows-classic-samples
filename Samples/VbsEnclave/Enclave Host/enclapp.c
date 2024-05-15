/*++

Copyright (c) Microsoft Corporation

Module Name:

    enclapp.c

Abstract:

    This app demonstrates the life cycle of a VBS enclave including
    making function calls into the enclave.

Author:

    Akash Trehan (aktrehan@microsoft.com) 01-30-2024

--*/

#include "precomp.h"

int
main(
    _In_ int argc,
    _In_reads_(argc) char** argv
)
{
    PVOID Base;
    BOOL Success;
    PVOID Function;
    ULONG_PTR Input;
    ULONG_PTR Output;

    FAILIF(IsEnclaveTypeSupported(ENCLAVE_TYPE_VBS) == FALSE);

    // Create enclave
    Base = CreateTestEnclave(NULL, 0x10000000, TestOwnerID);

    FAILIF(Base == NULL);

    // Load enclave module
    Success = LoadEnclaveImage(Base, L"vbsenclave.dll");

    FAILIF(Success == FALSE);

    // Initialize enclave
    Success = InitializeTestEnclave(Base, 1);

    FAILIF(Success == FALSE);

    // Call enclave
    Function = LocateTestEnclaveExport(Base, "CallEnclaveTest");

    FAILIF(Function == NULL);

    Input = 0x1234;
    Success = CallTestEnclave(Function, TRUE, Input, &Output);

    FAILIF(Success == FALSE);
    FAILIF((Output ^ Input) != 0xDADAF00D);

    // Terminate enclave
    Success = TerminateEnclave(Base, TRUE);

    FAILIF(Success == FALSE);

    // Delete enclave
    Success = DeleteEnclave(Base);

    FAILIF(Success == FALSE);

    return 0;

FailIf:
    return 1;
}

PVOID
CreateTestEnclave(
    _In_opt_ PVOID PreferredBase,
    _In_ SIZE_T Size,
    _In_ PUCHAR OwnerID
)
{
    ENCLAVE_CREATE_INFO_VBS CreateInfo;

    RtlZeroMemory(&CreateInfo, sizeof(ENCLAVE_CREATE_INFO_VBS));
    RtlCopyMemory(&CreateInfo.OwnerID, OwnerID, IMAGE_ENCLAVE_LONG_ID_LENGTH);
    CreateInfo.Flags = ENCLAVE_VBS_FLAG_DEBUG;

    return CreateEnclave(GetCurrentProcess(),
        PreferredBase,
        Size,
        0,
        ENCLAVE_TYPE_VBS,
        &CreateInfo,
        sizeof(ENCLAVE_CREATE_INFO_VBS),
        NULL);
}

BOOL
InitializeTestEnclave(
    _In_ PVOID Base,
    _In_ ULONG ThreadCount
)
{
    ENCLAVE_INIT_INFO_VBS InitInfo;

    InitInfo.Length = sizeof(ENCLAVE_INIT_INFO_VBS);
    InitInfo.ThreadCount = ThreadCount;

    return InitializeEnclave(GetCurrentProcess(),
        Base,
        &InitInfo,
        InitInfo.Length,
        NULL);
}

PVOID
LocateTestEnclaveExport(
    _In_ PVOID Base,
    _In_ const char* Name
)
{
    return (PVOID)(ULONG_PTR)GetProcAddress(Base, Name);
}

BOOL
CallTestEnclave(
    _In_ PVOID Function,
    _In_ BOOL Wait,
    _In_ ULONG_PTR Input,
    _Out_ PULONG_PTR Output
)
{
    return CallEnclave((PENCLAVE_ROUTINE)(ULONG_PTR)Function,
        (PVOID)Input,
        Wait,
        (PVOID*)Output);
}
