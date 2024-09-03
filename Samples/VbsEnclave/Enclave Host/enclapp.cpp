//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

/*++

    This app demonstrates the life cycle of a VBS enclave including
    making function calls into the enclave.

--*/

#include "precomp.h"

HRESULT Run()
{
    if (!IsEnclaveTypeSupported(ENCLAVE_TYPE_VBS))
    {
        printf("VBS Enclave not supported\n");
        return E_NOTIMPL;
    }

    // Create the enclave
    constexpr ENCLAVE_CREATE_INFO_VBS CreateInfo
    {
        ENCLAVE_VBS_FLAG_DEBUG, // Flags
        { 0x10, 0x20, 0x30, 0x40, 0x41, 0x31, 0x21, 0x11 }, // OwnerID
    };

    PVOID Enclave = CreateEnclave(GetCurrentProcess(),
        nullptr, // Preferred base address
        0x10000000, // size
        0,
        ENCLAVE_TYPE_VBS,
        &CreateInfo,
        sizeof(ENCLAVE_CREATE_INFO_VBS),
        nullptr);
    RETURN_LAST_ERROR_IF_NULL(Enclave);

    // Ensure we terminate and delete the enclave even if something goes wrong.
    auto cleanup = wil::scope_exit([&]
        {
            // fWait = TRUE means that we wait for all threads in the enclave to terminate.
            // This is necessary because you cannot delete an enclave if it still has
            // running threads.
            LOG_IF_WIN32_BOOL_FALSE(TerminateEnclave(Enclave, TRUE));

            // Delete the enclave.
            LOG_IF_WIN32_BOOL_FALSE(DeleteEnclave(Enclave));
        });

    // Load enclave module with SEM_FAILCRITICALERRORS enabled to suppress
    // the error message dialog.
    {
        DWORD previousMode = GetThreadErrorMode();
        SetThreadErrorMode(previousMode | SEM_FAILCRITICALERRORS, nullptr);
        auto restoreErrorMode = wil::scope_exit([&]
            {
                SetThreadErrorMode(previousMode, nullptr);
            });
        RETURN_IF_WIN32_BOOL_FALSE(LoadEnclaveImageW(Enclave, L"vbsenclave.dll"));
    }

    // Initialize the enclave with one thread.
    // Once initialized, no more DLLs can be loaded into the enclave.
    ENCLAVE_INIT_INFO_VBS InitInfo{};

    InitInfo.Length = sizeof(ENCLAVE_INIT_INFO_VBS);
    InitInfo.ThreadCount = 1;

    RETURN_IF_WIN32_BOOL_FALSE(InitializeEnclave(GetCurrentProcess(),
        Enclave,
        &InitInfo,
        InitInfo.Length,
        nullptr));

    // Locate the function in the enclave.
    PENCLAVE_ROUTINE Routine = reinterpret_cast<PENCLAVE_ROUTINE>(GetProcAddress(reinterpret_cast<HMODULE>(Enclave), "CallEnclaveTest"));
    RETURN_LAST_ERROR_IF_NULL(Routine);

    // Call the function. Our test function XOR's its input with a magic number.
    ULONG_PTR Input = 0x1234;
    void* Output;

    RETURN_IF_WIN32_BOOL_FALSE(CallEnclave(Routine, reinterpret_cast<void*>(Input), TRUE /* fWaitForThread */, &Output));

    // Verify that it performed the expected calculation.
    if ((reinterpret_cast<ULONG_PTR>(Output) ^ Input) != 0xDADAF00D)
    {
        printf("Unexpected result from enclave\n");
    }        

    // Destructor of "cleanup" variable will terminate and delete the enclave.

    return S_OK;
}

int
main(
    [[maybe_unused]] _In_ int argc,
    [[maybe_unused]] _In_reads_(argc) char** argv
)
{
    // Print diagnostic messages to the console for developer convenience.
    wil::SetResultLoggingCallback([](wil::FailureInfo const& failure) noexcept
        {
            wchar_t message[1024];
            wil::GetFailureLogString(message, ARRAYSIZE(message), failure);
            wprintf(L"Diagnostic message: %ls\n", message);
        });

    HRESULT hr = Run();
    if (hr == HRESULT_FROM_WIN32(ERROR_INVALID_IMAGE_HASH))
    {
        wprintf(L"If you developer-signed the DLL, make sure that you have enabled test signing.\n");
    }

    return SUCCEEDED(hr) ? EXIT_SUCCESS : EXIT_FAILURE;
}
