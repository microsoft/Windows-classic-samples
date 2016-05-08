// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include <shobjidl.h>
#include <propsys.h>
#include <propvarutil.h>
#include <propkey.h>
#include <strsafe.h>

HRESULT GetPropertyStore(PCWSTR pszFilename, GETPROPERTYSTOREFLAGS gpsFlags, IPropertyStore** ppps)
{
    WCHAR szExpanded[MAX_PATH];
    HRESULT hr = ExpandEnvironmentStrings(pszFilename, szExpanded, ARRAYSIZE(szExpanded)) ? S_OK : HRESULT_FROM_WIN32(GetLastError());
    if (SUCCEEDED(hr))
    {
        WCHAR szAbsPath[MAX_PATH];
        hr = _wfullpath(szAbsPath, szExpanded, ARRAYSIZE(szAbsPath)) ? S_OK : E_FAIL;
        if (SUCCEEDED(hr))
        {
            hr = SHGetPropertyStoreFromParsingName(szAbsPath, NULL, gpsFlags, IID_PPV_ARGS(ppps));
        }
    }
    return hr;
}

HRESULT PrintProperty(IPropertyStore *pps, REFPROPERTYKEY key, PCWSTR pszCanonicalName)
{
    PROPVARIANT propvarValue = {0};
    HRESULT hr = pps->GetValue(key, &propvarValue);
    if (SUCCEEDED(hr))
    {
        PWSTR pszDisplayValue = NULL;
        hr = PSFormatForDisplayAlloc(key, propvarValue, PDFF_DEFAULT, &pszDisplayValue);
        if (SUCCEEDED(hr))
        {
            wprintf(L"%s = %s\n", pszCanonicalName, pszDisplayValue);
            CoTaskMemFree(pszDisplayValue);
        }
        PropVariantClear(&propvarValue);
    }
    return hr;
}

HRESULT EnumerateProperties(PCWSTR pszFilename)
{
    IPropertyStore* pps = NULL;

    // Call the helper to get the property store for the initialized item
    // Note that as long as you have the property store, you are keeping the file open
    // So always release it once you are done.

    HRESULT hr = GetPropertyStore(pszFilename, GPS_DEFAULT, &pps);
    if (SUCCEEDED(hr))
    {
        // Retrieve the number of properties stored in the item.
        DWORD cProperties = 0;
        hr = pps->GetCount(&cProperties);
        if (SUCCEEDED(hr))
        {
            for (DWORD i = 0; i < cProperties; i++)
            {
                // Get the property key at a given index.
                PROPERTYKEY key;
                hr = pps->GetAt(i, &key);
                if (SUCCEEDED(hr))
                {
                    // Get the canonical name of the property
                    PWSTR pszCanonicalName = NULL;
                    hr = PSGetNameFromPropertyKey(key, &pszCanonicalName);
                    if (SUCCEEDED(hr))
                    {
                        hr = PrintProperty(pps, key, pszCanonicalName);
                        CoTaskMemFree(pszCanonicalName);
                    }
                }
            }
        }
        pps->Release();
    }
    else
    {
        wprintf(L"Error %x: getting the propertystore for the item.\n", hr);
    }
    return hr;
}

HRESULT GetPropertyValue(PCWSTR pszFilename, PCWSTR pszCanonicalName)
{
    // Convert the Canonical name of the property to PROPERTYKEY
    PROPERTYKEY key;
    HRESULT hr = PSGetPropertyKeyFromName(pszCanonicalName, &key);
    if (SUCCEEDED(hr))
    {
        IPropertyStore* pps = NULL;

        // Call the helper to get the property store for the initialized item
        hr = GetPropertyStore(pszFilename, GPS_DEFAULT, &pps);
        if (SUCCEEDED(hr))
        {
            hr = PrintProperty(pps, key, pszCanonicalName);
            pps->Release();
        }
        else
        {
            wprintf(L"Error %x: getting the propertystore for the item.\n", hr);
        }
    }
    else
    {
        wprintf(L"Invalid property specified: %s\n", pszCanonicalName);
    }
    return hr;
}

HRESULT SetPropertyValue(PCWSTR pszFilename, PCWSTR pszCanonicalName, PCWSTR pszValue)
{
    // Convert the Canonical name of the property to PROPERTYKEY
    PROPERTYKEY key;
    HRESULT hr = PSGetPropertyKeyFromName(pszCanonicalName, &key);
    if (SUCCEEDED(hr))
    {
        IPropertyStore* pps = NULL;

        // Call the helper to get the property store for the
        // initialized item
        hr = GetPropertyStore(pszFilename, GPS_READWRITE, &pps);
        if (SUCCEEDED(hr))
        {
            PROPVARIANT propvarValue = {0};
            hr = InitPropVariantFromString(pszValue, &propvarValue);
            if (SUCCEEDED(hr))
            {
                hr = PSCoerceToCanonicalValue(key, &propvarValue);
                if (SUCCEEDED(hr))
                {
                    // Set the value to the property store of the item.
                    hr = pps->SetValue(key, propvarValue);
                    if (SUCCEEDED(hr))
                    {
                        // Commit does the actual writing back to the file stream.
                        hr = pps->Commit();
                        if (SUCCEEDED(hr))
                        {
                            wprintf(L"Property %s value %s written successfully \n", pszCanonicalName, pszValue);
                        }
                        else
                        {
                            wprintf(L"Error %x: Commit to the propertystore failed.\n", hr);
                        }
                    }
                    else
                    {
                        wprintf(L"Error %x: Set value to the propertystore failed.\n", hr);
                    }
                }
                PropVariantClear(&propvarValue);
            }
            pps->Release();
        }
        else
        {
            wprintf(L"Error %x: getting the propertystore for the item.\n", hr);
        }
    }
     else
    {
        wprintf(L"Invalid property specified: %s\n", pszCanonicalName);
    }
    return hr;
}

HRESULT GetPropertyDescription(PCWSTR pszCanonicalName)
{
    // Get the property description for the given property.
    // Property description contains meta information on the property itself.
    IPropertyDescription* ppd = NULL;
    HRESULT hr = PSGetPropertyDescriptionByName(pszCanonicalName, IID_PPV_ARGS(&ppd));
    if (SUCCEEDED(hr))
    {
        PWSTR pszPropertyLabel = NULL;
        hr = ppd->GetDisplayName(&pszPropertyLabel);
        if (SUCCEEDED(hr))
        {
            wprintf(L"Property %s has label : %s\n", pszCanonicalName, pszPropertyLabel);
            CoTaskMemFree(pszPropertyLabel);
        }
        ppd->Release();
    }
     else
    {
        wprintf(L"Invalid property specified: %s\n", pszCanonicalName);
    }
    return hr;
}

void Usage(PCWSTR pszAppName)
{
    wprintf(L"Usage: %s [OPTIONS] [Filename] \n", pszAppName);
    wprintf(L"\n");
    wprintf(L"Options:\n");
    wprintf(L" -get <PropertyName>   Get the value for the property defined\n");
    wprintf(L"                       by its Canonical Name in <propertyName>\n");
    wprintf(L" -set <PropertyName>   Set the value for the property defined\n");
    wprintf(L"      <PropertyValue>  by <PropertyName> with value <PropertyValue>\n");
    wprintf(L" -enum                 Enumerate all the properties.\n");
    wprintf(L" -info <PropertyName>  Get schema information on property.\n");
    wprintf(L"\n");
    wprintf(L"Examples:\n");
    wprintf(L"PropertyEdit -get System.Author foo.jpg\n");
    wprintf(L"PropertyEdit -set System.Author \"John Doe\" foo.jpg\n");
    wprintf(L"PropertyEdit -enum foo.jpg\n");
    wprintf(L"PropertyEdit -info System.Author \n");
}

// returns the next argument, and advances ppszArgs and cArgs past it
#define CONSUME_NEXT_ARG(ppszArgs, cArgs) ((cArgs)--, ((ppszArgs)++)[0])

int wmain(int argc, wchar_t* argv[])
{
    if (SUCCEEDED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE)))
    {
        PCWSTR pszAppName = CONSUME_NEXT_ARG(argv, argc);
        PCWSTR pszOp = CONSUME_NEXT_ARG(argv, argc);
        if (pszOp && ((pszOp[0] == L'-') || (pszOp[0] == L'/')))
        {
            /* skip - or / */
            pszOp++;
            if (!_wcsicmp(pszOp, L"?"))
            {
                Usage(pszAppName);
            }
            else if (!_wcsicmp(pszOp, L"get"))
            {
                PCWSTR pszPropertyName = CONSUME_NEXT_ARG(argv, argc);
                if (pszPropertyName)
                {
                    PCWSTR pszFileName = CONSUME_NEXT_ARG(argv, argc);
                    if (pszFileName)
                    {
                        GetPropertyValue(pszFileName, pszPropertyName);
                    }
                    else
                    {
                        wprintf(L"No file name specified.\n");
                    }
                }
                else
                {
                    wprintf(L"No property canonical name specified.\n");
                }
            }
            else if (!_wcsicmp(pszOp, L"enum"))
            {
                PCWSTR pszFileName = CONSUME_NEXT_ARG(argv, argc);
                if (pszFileName)
                {
                    EnumerateProperties(pszFileName);
                }
                else
                {
                    wprintf(L"No file name specified.\n");
                }
            }
            else if (!_wcsicmp(pszOp, L"set"))
            {
                PCWSTR pszPropertyName = CONSUME_NEXT_ARG(argv, argc);
                if (pszPropertyName)
                {
                    PCWSTR pszPropertyValue = CONSUME_NEXT_ARG(argv, argc);
                    if (pszPropertyValue)
                    {
                        PCWSTR pszFileName = CONSUME_NEXT_ARG(argv, argc);
                        if (pszFileName)
                        {
                            SetPropertyValue(pszFileName, pszPropertyName, pszPropertyValue);
                        }
                        else
                        {
                            wprintf(L"No file name specified.\n");
                        }
                    }
                    else
                    {
                        wprintf(L"No property value specified.\n");
                    }
                }
                else
                {
                    wprintf(L"No property canonical name specified.\n");
                }
            }
            else if (!_wcsicmp(pszOp, L"info"))
            {
                PCWSTR pszPropertyName = CONSUME_NEXT_ARG(argv, argc);
                if (pszPropertyName)
                {
                    GetPropertyDescription(pszPropertyName);
                }
                else
                {
                    wprintf(L"No property canonical name specified.\n");
                }
            }
            else
            {
                wprintf(L"Unrecognized operation specified: -%s\n", pszOp);
                Usage(pszAppName);
            }
        }
        else
        {
            wprintf(L"No operation specified.\n");
            Usage(pszAppName);
        }
        CoUninitialize();
    }
    return 0;
}