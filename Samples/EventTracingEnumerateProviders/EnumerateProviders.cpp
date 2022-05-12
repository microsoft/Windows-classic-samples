//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved

#include <windows.h>
#include <memory>
#include <new>
#include <stdio.h>
#include <tdh.h>

#pragma comment(lib, "tdh.lib")

#define MAX_GUID_SIZE 39

static PCWSTR GetSchemaSourceName(ULONG schemaSource)
{
    switch (schemaSource)
    {
    case 0: return L"XML manifest";
    case 1: return L"WMI MOF class";
    default: return L"unknown";
    }
}

static void ShowProviderInfo(PROVIDER_ENUMERATION_INFO* pEnum)
{
    for (DWORD i = 0; i < pEnum->NumberOfProviders; i++)
    {
        WCHAR stringGuid[MAX_GUID_SIZE];
        (void)StringFromGUID2(pEnum->TraceProviderInfoArray[i].ProviderGuid, stringGuid, ARRAYSIZE(stringGuid));

        // Provider information is in pEnum->TraceProviderInfoArray[i].
        wprintf(L"Provider name: %ls\nProvider GUID: %ls\nSource: %u (%ls)\n\n",
            (LPWSTR)((PBYTE)(pEnum)+pEnum->TraceProviderInfoArray[i].ProviderNameOffset),
            stringGuid,
            pEnum->TraceProviderInfoArray[i].SchemaSource,
            GetSchemaSourceName(pEnum->TraceProviderInfoArray[i].SchemaSource));
    }
}

#if defined(NTDDI_WIN10_MN) && (NTDDI_VERSION >= NTDDI_WIN10_MN)
void TdhEnumerateProvidersForDecodingSourceSample()
{
    DWORD status;
    std::unique_ptr<BYTE[]> manifestProvidersBuffer;
    DWORD manifestProvidersBufferSize = 0;

    // Available in Windows 10 build 20348 or later.
    // Retrieve providers registered via manifest files with TdhEnumerateProvidersForDecodingSource.
    // Allocate the required buffer and call TdhEnumerateProvidersForDecodingSource.
    // The list of providers can change between the time you retrieved the required
    // buffer size and the time you enumerated the providers, so call
    // TdhEnumerateProvidersForDecodingSource in a loop until the function does
    // not return ERROR_INSUFFICIENT_BUFFER.
    for (;;)
    {
        // Note that the only supported decoding sources are DecodingSourceXMLFile
        // and DecodingSourceWbem. This sample uses DecodingSourceXMLFile.
        status = TdhEnumerateProvidersForDecodingSource(
            DecodingSourceXMLFile,
            reinterpret_cast<PROVIDER_ENUMERATION_INFO*>(manifestProvidersBuffer.get()),
            manifestProvidersBufferSize,
            &manifestProvidersBufferSize);

        if (status != ERROR_INSUFFICIENT_BUFFER)
        {
            break;
        }

        manifestProvidersBuffer.reset(new(std::nothrow) BYTE[manifestProvidersBufferSize]);
        if (!manifestProvidersBuffer)
        {
            status = ERROR_OUTOFMEMORY;
            break;
        }
    }

    if (ERROR_SUCCESS != status)
    {
        wprintf(L"TdhEnumerateProvidersForDecodingSource failed with error %lu.\n", status);
        return;
    }
    else
    {
        ShowProviderInfo(reinterpret_cast<PROVIDER_ENUMERATION_INFO*>(manifestProvidersBuffer.get()));
    }
}
#endif

void TdhEnumerateProvidersSample()
{
    DWORD status;
    std::unique_ptr<BYTE[]> providerBuffer;
    DWORD providerBufferSize = 0;

    // Available in Windows Vista or later.
    // Retrieve providers registered via manifest files and via MOF class with TdhEnumerateProviders.
    // Allocate the required buffer and call TdhEnumerateProviders.
    // The list of providers can change between the time you retrieved the required
    // buffer size and the time you enumerated the providers, so call
    // TdhEnumerateProviders in a loop until the function does not return
    // ERROR_INSUFFICIENT_BUFFER.
    for (;;)
    {
        status = TdhEnumerateProviders(
            reinterpret_cast<PROVIDER_ENUMERATION_INFO*>(providerBuffer.get()),
            &providerBufferSize);

        if (status != ERROR_INSUFFICIENT_BUFFER)
        {
            break;
        }

        providerBuffer.reset(new(std::nothrow) BYTE[providerBufferSize]);
        if (!providerBuffer)
        {
            status = ERROR_OUTOFMEMORY;
            break;
        }

    }

    if (ERROR_SUCCESS != status)
    {
        wprintf(L"TdhEnumerateProviders failed with error %lu.\n", status);
        return;
    }
    else
    {
        ShowProviderInfo(reinterpret_cast<PROVIDER_ENUMERATION_INFO*>(providerBuffer.get()));
    }
}

int wmain(void)
{
    wprintf(L"TdhEnumerateProviders output:\n");
    TdhEnumerateProvidersSample();

#if defined(NTDDI_WIN10_MN) && (NTDDI_VERSION >= NTDDI_WIN10_MN)
    wprintf(L"TdhEnumerateProvidersForDecodingSource output:\n");
    TdhEnumerateProvidersForDecodingSourceSample();
#endif
}
