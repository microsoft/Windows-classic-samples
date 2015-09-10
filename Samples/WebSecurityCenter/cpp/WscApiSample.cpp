// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.


/**
 * This sample demonstrates how to query Security Center for the product name, 
 * product state, and product signature status for each security product
 * registered.
 */

#include <stdio.h>
#include <atlbase.h>
#include <atlstr.h>
#include <wscapi.h>
#include <iwscapi.h>


HRESULT 
GetSecurityProducts(
    _In_ WSC_SECURITY_PROVIDER provider
    )
{
    HRESULT                         hr                  = S_OK;
    IWscProduct*                    PtrProduct          = nullptr;
    IWSCProductList*                PtrProductList      = nullptr;
    BSTR                            PtrVal              = nullptr;
    LONG                            ProductCount        = 0;
    WSC_SECURITY_PRODUCT_STATE      ProductState;
    WSC_SECURITY_SIGNATURE_STATUS   ProductStatus;

    if (provider != WSC_SECURITY_PROVIDER_FIREWALL && 
        provider != WSC_SECURITY_PROVIDER_ANTIVIRUS && 
        provider != WSC_SECURITY_PROVIDER_ANTISPYWARE)
    {
        hr = E_INVALIDARG;
        goto exit;
    }

    //
    // Initialize can only be called once per instance, so you need to
    // CoCreateInstance for each security product type you want to query.
    //
    hr = CoCreateInstance(
            __uuidof(WSCProductList), 
            NULL, 
            CLSCTX_INPROC_SERVER, 
            __uuidof(IWSCProductList), 
            reinterpret_cast<LPVOID*> (&PtrProductList));
    if(FAILED(hr))
    {
        wprintf(L"CoCreateInstance returned error = 0x%d \n", hr);
        goto exit;
    }

    //
    // Initialize the product list with the type of security product you're 
    // interested in.
    //
    hr = PtrProductList->Initialize(provider);
    if(FAILED(hr))
    {
        wprintf(L"Initialize failed with error: 0x%d\n", hr);
        goto exit;
    }

    //
    // Get the number of security products of that type.
    //
    hr = PtrProductList->get_Count(&ProductCount);
    if (FAILED(hr))
    {
        wprintf(L"get_Count failed with error: 0x%d\n", hr);
        goto exit;
    }

    if (provider == WSC_SECURITY_PROVIDER_FIREWALL)
    {
        wprintf(L"\n\nFirewall Products:\n");
    }
    else if (provider == WSC_SECURITY_PROVIDER_ANTIVIRUS)
    {
        wprintf(L"\n\nAntivirus Products:\n");
    }
    else
    {
        wprintf(L"\n\nAntispyware Products:\n");
    }

    //
    // Loop over each product, querying the specific attributes.
    //
    for (LONG i = 0; i < ProductCount; i++)
    {
        //
        // Get the next security product
        //
        hr = PtrProductList->get_Item(i, &PtrProduct);
        if(FAILED(hr))
        {
            wprintf(L"get_Item failed with error: 0x%d\n", hr);
            goto exit;
        }

        //
        // Get the product name
        //
        hr = PtrProduct->get_ProductName(&PtrVal);
        if (FAILED(hr))
        {
            wprintf(L"get_ProductName failed with error: 0x%d\n", hr);
            goto exit;
        }
        wprintf(L"\nProduct name: %s\n", PtrVal);
        // Caller is responsible for freeing the string
        SysFreeString(PtrVal);
        PtrVal = nullptr;

        //
        // Get the product state
        //
        hr = PtrProduct->get_ProductState(&ProductState);
        if (FAILED(hr))
        {
            wprintf(L"get_ProductState failed with error: 0x%d\n", hr);
            goto exit;
        }

        LPWSTR pszState;
        if (ProductState == WSC_SECURITY_PRODUCT_STATE_ON)
        {
            pszState = L"On";
        }
        else if (ProductState == WSC_SECURITY_PRODUCT_STATE_OFF)
        {
            pszState = L"Off";
        }
        else if (ProductState == WSC_SECURITY_PRODUCT_STATE_SNOOZED)
        {
            pszState = L"Snoozed";
        }
        else
        {
            pszState = L"Expired";
        }
        wprintf(L"Product state: %s\n", pszState);

        //
        // Get the signature status (not applicable to firewall products)
        //
        if (provider != WSC_SECURITY_PROVIDER_FIREWALL)
        {
            hr = PtrProduct->get_SignatureStatus(&ProductStatus);
            if (FAILED(hr))
            {
                wprintf(L"get_SignatureStatus failed with error: 0x%d\n", hr);
                goto exit;
            }
            LPWSTR pszStatus = (ProductStatus == WSC_SECURITY_PRODUCT_UP_TO_DATE) ? 
                                    L"Up-to-date" : L"Out-of-date";
            wprintf(L"Product status: %s\n", pszStatus);
        }

        //
        // Get the remediation path for the security product
        //
        hr = PtrProduct->get_RemediationPath(&PtrVal);
        if (FAILED(hr))
        {
            wprintf(L"get_RemediationPath failed with error: 0x%d\n", hr);
            goto exit;
        }
        wprintf(L"Product remediation path: %s\n", PtrVal);
        // Caller is responsible for freeing the string
        SysFreeString(PtrVal);
        PtrVal = nullptr;

        //
        // Get the product state timestamp (updated when product changes its 
        // state), and only applicable for AV products (NULL is returned for
        // AS and FW products)
        //
        if (provider == WSC_SECURITY_PROVIDER_ANTIVIRUS)
        {
            hr = PtrProduct->get_ProductStateTimestamp(&PtrVal);
            if (FAILED(hr))
            {
                wprintf(L"get_ProductStateTimestamp failed with error: 0x%d\n", hr);
                goto exit;
            }
            wprintf(L"Product state timestamp: %s\n", PtrVal);
            // Caller is responsible for freeing the string
            SysFreeString(PtrVal);
            PtrVal = nullptr;
        }

        PtrProduct->Release();
        PtrProduct = nullptr;
    }

exit:

    if (nullptr != PtrVal)
    {
        SysFreeString(PtrVal);
    }
    if (nullptr != PtrProductList)
    {
        PtrProductList->Release();
    }
    if (nullptr != PtrProduct)
    {
        PtrProduct->Release();
    }
    return hr;
}

void PrintUsage()
{
    wprintf(L"Usage: WscApiSample.exe [-av | -as | -fw]\n");
    wprintf(L"   av: Query Antivirus programs\n");
    wprintf(L"   as: Query Antispyware programs\n");
    wprintf(L"   fw: Query Firewall programs\n\n");
}

int 
__cdecl 
wmain(
    _In_              int     argc, 
    _In_reads_(argc)  LPCWSTR argv[]
    )
{
    int     ret             = 0;
    HRESULT hr              = S_OK;
    int     iProviderCount  = 0;
    WSC_SECURITY_PROVIDER providers[3];

    if (argc < 2 || argc > 4)
    {
        PrintUsage();
        return -1;
    }

    //
    // Parse command line arguments
    //
    for (int i = 1; i < argc; i++)
    {
        if (_wcsnicmp(argv[i], L"-av", MAX_PATH) == 0)
        {
            providers[iProviderCount] = WSC_SECURITY_PROVIDER_ANTIVIRUS;
            iProviderCount ++;
        }
        else if (_wcsnicmp(argv[i], L"-as", MAX_PATH) == 0)
        {
            providers[iProviderCount] = WSC_SECURITY_PROVIDER_ANTISPYWARE;
            iProviderCount ++;
        }
        else if (_wcsnicmp(argv[i], L"-fw", MAX_PATH) == 0)
        {
            providers[iProviderCount] = WSC_SECURITY_PROVIDER_FIREWALL;
            iProviderCount ++;
        }
        else
        {
            PrintUsage();
            return -1;
        }
    }

    CoInitializeEx(0, COINIT_APARTMENTTHREADED );

    for (int i = 0; i < iProviderCount; i++)
    {
        //
        // Query security products of the specified type (AV, AS, or FW)
        //
        hr = GetSecurityProducts(providers[i]);
        if (FAILED(hr))
        {
            ret = -1;
            break;
        }
    }

    CoUninitialize();
    return ret;
}
