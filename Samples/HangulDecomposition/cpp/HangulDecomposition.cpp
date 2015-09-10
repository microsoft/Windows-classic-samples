// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include <windows.h>
#include <stdio.h>
#include <ElsCore.h>
#include <ElsSrvc.h>

#pragma comment(lib, "elscore.lib")

// This function returns true if transliterating is succeeded and 
// transliterated result string is same as expected.
bool TestRecognizeMappingText(_In_ PMAPPING_SERVICE_INFO mappingServiceInfo, _In_z_ const wchar_t* queryValue, _In_z_ const wchar_t* expectedValue);

int __cdecl main()
{
    // create Hangul Decomposition Transliteration service.
    PMAPPING_SERVICE_INFO mappingServiceInfo = NULL;
    DWORD servicesCount = 0;
    MAPPING_ENUM_OPTIONS enumOptions;
    ZeroMemory(&enumOptions, sizeof(enumOptions));
    enumOptions.Size = sizeof(enumOptions);
    enumOptions.pGuid = const_cast<GUID*>(&ELS_GUID_TRANSLITERATION_HANGUL_DECOMPOSITION);
    int testCount = 1;

    if (SUCCEEDED(MappingGetServices(&enumOptions, &mappingServiceInfo, &servicesCount)) && mappingServiceInfo != NULL)
    {
        bool succeeded;

        // Hangul syllable is decomposed into Korean 2beolsik keyboard keystrokes. 
        // Decomposed syllable string is represented by Compatibility Jamo.
        succeeded = TestRecognizeMappingText(mappingServiceInfo, L"\xAC00\xAC01", L"\x3131\x314F\x3131\x314F\x3131");
        wprintf(L"test %d: %s\n", testCount++, succeeded ? L"succeeded" : L"failed");

        // A twin consonant is treated as a basic consonants. Because 2beolsik Keyboard
        // defines keys for twin consonants.
        succeeded = TestRecognizeMappingText(mappingServiceInfo, L"\xAE4C\xC600", L"\x3132\x314F\x3147\x3155\x3146");
        wprintf(L"test %d: %s\n", testCount++, succeeded ? L"succeeded" : L"failed");

        // A single syllable can be decomposed in 2 to 5 jamos.
        succeeded = TestRecognizeMappingText(mappingServiceInfo, L"\xAC00\xB220\xB400\xB923", L"\x3131\x314F\x3134\x315C\x3153\x3137\x3157\x3150\x3134\x3139\x315C\x3154\x3131\x3145");
        wprintf(L"test %d: %s\n", testCount++, succeeded ? L"succeeded" : L"failed");

        // Modern compatibility jamos are also decomposed, but not for old jamos
        succeeded = TestRecognizeMappingText(mappingServiceInfo, L"\x313A\x3165", L"\x3139\x3131\x3165");
        wprintf(L"test %d: %s\n", testCount++, succeeded ? L"succeeded" : L"failed");

        // Decomposing is not applied to other characters.
        succeeded = TestRecognizeMappingText(mappingServiceInfo, L"1A@\xAC00*", L"1A@\x3131\x314F*");
        wprintf(L"test %d: %s\n", testCount++, succeeded ? L"succeeded" : L"failed");

        // free services.
        MappingFreeServices(mappingServiceInfo);
    }
    else
    {
        wprintf(L"Failed to create a transliteration service\n");
    }

    return 0;
}

bool TestRecognizeMappingText(_In_ PMAPPING_SERVICE_INFO mappingServiceInfo, _In_z_ const wchar_t* queryValue, _In_z_ const wchar_t* expectedValue)
{
    MAPPING_PROPERTY_BAG mappingPropertyBag;
    ZeroMemory(&mappingPropertyBag, sizeof(mappingPropertyBag));
    mappingPropertyBag.Size = sizeof(mappingPropertyBag);
    bool succeeded = false;

    size_t queryValueLength = wcslen(queryValue);
    if (SUCCEEDED(MappingRecognizeText(mappingServiceInfo, queryValue, static_cast<DWORD>(queryValueLength + 1), 0, NULL, &mappingPropertyBag)))
    {
        if (mappingPropertyBag.dwRangesCount > 0)
        {
            const wchar_t* actualValue = static_cast<const wchar_t*>(mappingPropertyBag.prgResultRanges[0].pData);
            succeeded = 0 == wcscmp(expectedValue, actualValue);
        }
        MappingFreePropertyBag(&mappingPropertyBag);
    }

    return succeeded;
}
