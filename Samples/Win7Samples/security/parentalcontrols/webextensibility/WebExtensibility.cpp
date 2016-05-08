
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

/****************************************************************************

    FILE: Windows Parental Controls (WPC) Web Filtering Extensibility sample

    PURPOSE: Demonstrates managing policy settings relevant to web content
             filtering.  Properties in the singleton SystemSettings class
             instance are get/set to specify:
                URL exemptions
                Application exemptions (no filtering on any traffic to/from 
                 application)
                Whether the in-box web content filter or a 3rd-party 
                 replacement filter is exclusive active filter


    FUNCTIONS:

        wmain() - implements overall command line application
        WpcsWebExtensExemptionsGet() - passes back either an HTTP
          application or URL exemption list
        WpcsWebExtensExemptionAdd() - adds either an HTTP application
          or URL exemption to the appropriate list
        WpcsWebExtensExemptionDel() - deletes the specified entry from
          either the HTTP application or URL exemption list
        WpcsWebExtensFilterGet() - passes back the current settings for
          the FilterID and FilterNamePath properties selecting the active
          web content filter
        WpcsWebExtensFilterSet() - sets the FilterID and FilterNamePath 
          properties as specified for ownership of web content filtering
        WpcsWebExtensFilterReset() - resets the FilterID and FilterNamePath
          properties to the defaults for the Windows Vista web content 
          filter selection as active
        CmdLineParse() - handles command line input

    COMMENTS:
        1.  Third-party web content filters should respect the HTTP 
            application and URL exemption lists.  
        2.  Note that registration of a user interface link for a 3rd-
            party web content filter is required.  This is accomplished as 
            shown in the UIExtensibility sample.  Once registered, setting
            the FilterID to the same GUID as for the UI link entry, and 
            setting a FilterNamePath, promotes the link from a generic UI
            extensibility link to the exclusive active web content filter
            position in the control panel.  It also disables the Windows
            Vista web content filter for the user.

****************************************************************************/


#include "WebExtensibility.h"

HRESULT WpcsWebExtensExemptionsGet(IWbemServices* piWmiServices, OPERATION eOperation,
                                   DWORD* pdwNumExemptions, PWSTR** pppszExemptions);

HRESULT WpcsWebExtensExemptionAdd(IWbemServices* piWmiServices, OPERATION eOperation, 
                                 PCWSTR pcszExemption);

HRESULT WpcsWebExtensExemptionDel(IWbemServices* piWmiServices, OPERATION eOperation, 
                                 PCWSTR pcszExemption);

HRESULT WpcsWebExtensFilterGet(IWbemServices* piWmiServices, PWSTR* ppszFilterID,
                              PWSTR* ppszFilterNamePath);

HRESULT WpcsWebExtensFilterSet(IWbemServices* piWmiServices, PCWSTR pcszFilterID,
                              PCWSTR pcszFilterNamePath);

HRESULT WpcsWebExtensFilterReset(IWbemServices* piWmiServices);

HRESULT CmdLineParse(int argc, WCHAR* argv[], OPERATION* peOperation, 
                     SUBOPERATION* peSubOperation, PCWSTR* ppcszExemption,
                     PCWSTR* ppcszFilterID, PCWSTR* ppcszFilterNamePath);

void Usage (PCWSTR pcszProgramName);


// call as Usage(argv[0])
void Usage (PCWSTR pcszProgramName)
{
    wprintf(L"Usage:  %s <operation> [argument1..n]\n", pcszProgramName);
    wprintf(L" Where operations and associated arguments are as follows:\n\n");
    wprintf(L"http list \t\tList HTTP application exemptions\n\n");
    wprintf(L"http add <path> \tAdd HTTP application exemption\n\n");
    wprintf(L"http del <path> \tDelete HTTP application exemption\n\n");
    wprintf(L"url list \t\tList URL allow exemptions\n\n");
    wprintf(L"url add <URL> \t\tAdd URL allow exemption\n\n");
    wprintf(L"url del <URL> \t\tDelete URL allow exemption\n\n");
    wprintf(L"filter get \t\tDisplay current filter ID and name path overrides\n\n");
    wprintf(L"filter set /g:<FilterID> /n:<FilterNamePath>\n \
             \t\tSet overriding filter by ID and name resource path\n\n");
    wprintf(L"filter reset \t\tReset filter override\n\n");
}


// Application entry point
int __cdecl wmain(int argc, __in_ecount(argc) WCHAR* argv[])
{
    OPERATION eOperation;
    SUBOPERATION eSubOperation;
    PCWSTR pcszExemption, pcszFilterID, pcszFilterNamePath;

    HRESULT hr = CmdLineParse(argc, argv, &eOperation, &eSubOperation, &pcszExemption, 
                              &pcszFilterID, &pcszFilterNamePath);
    if (hr == E_INVALIDARG)
    {
        // Printf usage and bypass further initialization
        Usage(argv[0]);
    }
    else if (SUCCEEDED(hr))
    {
        hr = WpcuCOMInit();
        if (FAILED(hr))
        {
            wprintf(L"Error:  Failed to initialize COM, hr is %8x.\n", hr);
        }
        else
        {
            // Declare and initialize WPC API interface pointers
            IWbemServices* piWmiServices;
            // Connect to WPC namespace in WMI on local machine
            hr = WpcuWmiConnect(WPCS_WMI_NAMESPACE, &piWmiServices);
            if (FAILED(hr))
            {
                wprintf(L"Error:  WpcuWmiConnect() failed, hr is %8x.\n", hr);
            }
            else
            {
                // Perform mode-specific operations
                DWORD dwNumExemptions = 0;
                PWSTR* ppszExemptions = NULL;

                switch (eOperation)
                {
                    case OPERATION_HTTP:
                    case OPERATION_URL:

                        switch (eSubOperation)
                        {
                            case EXEMPTION_LIST:
                                hr = WpcsWebExtensExemptionsGet(piWmiServices, eOperation, 
                                                                &dwNumExemptions, &ppszExemptions);
                                if (FAILED(hr))
                                {
                                    wprintf(L"Error:  WpcsWebExtensGet() failed, hr is %8x.\n", hr);
                                }
                                else
                                {
                                    // Print results
                                    wprintf(L"Info:  Exemption List:\n");
                                    for (DWORD i = 0; i < dwNumExemptions; i++)
                                    {
                                        wprintf(L"\t%3d:\t%s\n", i, ppszExemptions[i]);
                                    }
                                    wprintf(L"Info:  end of list.\n");
                                }

                                // Clean up allocations from called function
                                for (DWORD i = 0; i < dwNumExemptions; i++)
                                {
                                    if (ppszExemptions[i] != NULL)
                                    {
                                        delete[] ppszExemptions[i];
                                        ppszExemptions[i] = NULL;
                                    }
                                }
                                dwNumExemptions = 0;
                                if (ppszExemptions)
                                {
                                    delete[] ppszExemptions;
                                    ppszExemptions = NULL;
                                }
                                break;

                            case EXEMPTION_ADD:
                                hr = WpcsWebExtensExemptionAdd(piWmiServices, eOperation, 
                                                               pcszExemption);
                                if (FAILED(hr))
                                {
                                    wprintf(L"Error:  WpcsWebExtensAdd() failed, hr is %8x.\n", hr);
                                }
                                else
                                {
                                    wprintf(L"Info:  Exemption entry added.\n");
                                }
                                break;

                            case EXEMPTION_DEL:
                                hr = WpcsWebExtensExemptionDel(piWmiServices, eOperation, 
                                                               pcszExemption);
                                if (FAILED(hr))
                                {
                                    wprintf(L"Error:  WpcsWebExtensDel() failed, hr is %8x.\n", hr);
                                }
                                else
                                {
                                    wprintf(L"Info:  Exemption entry deleted.\n");
                                }
                                break;
                        }
                        break;
                    case OPERATION_FILTER:
                    
                        switch (eSubOperation)
                        {
                            case ID_NAME_GET:
                                PWSTR pszCurrentFilterID, pszCurrentFilterNamePath;
                                hr = WpcsWebExtensFilterGet(piWmiServices, &pszCurrentFilterID, 
                                                            &pszCurrentFilterNamePath);
                                if (FAILED(hr))
                                {
                                    wprintf(L"Error:  WpcsWebFilterIDNameGet() failed, hr is %8x.\n", hr);
                                }
                                else
                                {
                                    // Print name and ID
                                    wprintf(L"Info:  Web content filter:\n");
                                    wprintf(L"Info:  \tID:  %s.\n", 
                                             (pszCurrentFilterID == NULL) ? L"Invalid ID" : 
                                              pszCurrentFilterID);
                                    wprintf(L"Info:  \tName:  %s.\n",
                                            (pszCurrentFilterNamePath == NULL) ? L"<NULL> (in-box filter)" : 
                                             pszCurrentFilterNamePath);
                                }

                                // Clean up allocations from called function
                                if (pszCurrentFilterID)
                                {
                                    delete[] pszCurrentFilterID;
                                }
                                if (pszCurrentFilterNamePath)
                                {
                                    delete[] pszCurrentFilterNamePath;
                                }
                                break;

                            case ID_NAME_SET:
								hr = WpcsWebExtensFilterSet(piWmiServices, pcszFilterID, pcszFilterNamePath);
                                if (FAILED(hr))
                                {
                                    wprintf(L"Error:  WpcsWebFilterIDNameSet() failed, hr is %8x.\n", hr);
                                }
                                else
                                {
                                    wprintf(L"Info:  Filter ID and Name set.\n");
                                }
                                break;

                            case ID_NAME_RESET:
                                hr = WpcsWebExtensFilterReset(piWmiServices);
                                if (FAILED(hr))
                                {
                                    wprintf(L"Error:  WpcsWebFilterIDNameReset() failed, hr is %8x.\n", hr);
                                }
                                else
                                {
                                    wprintf(L"Info:  Filter ownership reset.\n");
                                }
                                break;
                        }
                }

                // Cleanup
                piWmiServices->Release();
            }
            WpcuCOMCleanup();
        }
    }

    return (SUCCEEDED(hr)) ? 0 : 1;
}

// Calling code is responsible for freeing the array of exemptions passed back
HRESULT WpcsWebExtensExemptionsGet(IWbemServices* piWmiServices, OPERATION eOperation,
                                   DWORD* pdwNumExemptions, PWSTR** pppszExemptions)
{
    HRESULT hr = S_OK;

    // Do basic parameter validation
    if (!piWmiServices || !pdwNumExemptions || !pppszExemptions ||
        (eOperation != OPERATION_HTTP && eOperation != OPERATION_URL))
    {
        hr = E_INVALIDARG;
    }
    else
    {
        IWbemClassObject* piWmiSystemSettings;
        hr = WpcuWmiObjectGet(piWmiServices, WPCS_WMI_SYSTEM_SETTINGS, 
            &piWmiSystemSettings);
        if (SUCCEEDED(hr))
        {
            PWSTR pcszProperty = (eOperation == OPERATION_HTTP) ? L"HTTPExemptionList" :
                L"URLExemptionList";
            hr = WpcuWmiStringArrayFromInstance(piWmiSystemSettings, pcszProperty, 
                pdwNumExemptions, pppszExemptions);
            piWmiSystemSettings->Release();
        }
    }

    return (hr);

}


HRESULT WpcsWebExtensExemptionAdd(IWbemServices* piWmiServices, OPERATION eOperation, 
                                 PCWSTR pcszExemption)
{
    HRESULT hr = E_INVALIDARG;

    // Do basic parameter validation
    if (piWmiServices && pcszExemption &&
        (eOperation == OPERATION_HTTP || eOperation == OPERATION_URL))
    {
        // Get existing array.  If null, just put new.  Else, build new array 
        // from original with added value and put
        DWORD dwNumElements = 0;
        PWSTR* ppszValue = NULL;
        hr = WpcsWebExtensExemptionsGet(piWmiServices, eOperation, &dwNumElements, 
                &ppszValue);
        if (SUCCEEDED(hr))
        {
            // Shallow copy existing array to new that is one larger, or create a 
            //  single entry array if no elements were originally present
            PCWSTR* ppcszNewValue = new PCWSTR[dwNumElements + 1];
            if (!ppcszNewValue)
            {
                hr = E_OUTOFMEMORY;
            }
            else
            {
                for (DWORD i = 0; i < dwNumElements; i++)
                {
                    ppcszNewValue[i] = ppszValue[i];
                }
                // Write requested additional string as last element
                ppcszNewValue[dwNumElements] = pcszExemption;
                PCWSTR pcszProperty = (eOperation == OPERATION_HTTP) ? 
                    L"HTTPExemptionList" : L"URLExemptionList";
                IWbemClassObject* piWmiSystemSettings = NULL;
                hr = WpcuWmiObjectGet(piWmiServices, WPCS_WMI_SYSTEM_SETTINGS, 
                    &piWmiSystemSettings);
                if (SUCCEEDED(hr))
                {
                    hr = WpcuWmiInstancePutStringArray(piWmiSystemSettings, 
                                                       pcszProperty, 
                                                       dwNumElements + 1, 
                                                       ppcszNewValue);
                    if (SUCCEEDED(hr))
                    {
                        hr = piWmiServices->PutInstance(piWmiSystemSettings,
                            WBEM_FLAG_UPDATE_ONLY, NULL, NULL);
                    }
                    piWmiSystemSettings->Release();
                }
                delete[] ppcszNewValue;
            }
            // Clean up allocations from WpcsWebExtensExemptionsGet()
            for (DWORD i = 0; i < dwNumElements; i++)
            {
                if (ppszValue[i] != NULL)
                {
                    delete[] ppszValue[i];
                    ppszValue[i] = NULL;
                }
            }
            if (ppszValue)
            {
                delete[] ppszValue;
                ppszValue = NULL;
            }
        }
    }

    return (hr);
}


HRESULT WpcsWebExtensExemptionDel(IWbemServices* piWmiServices, OPERATION eOperation, 
                                 PCWSTR pcszExemption)
{
    HRESULT hr = E_INVALIDARG;

    if (piWmiServices && pcszExemption &&
        (eOperation == OPERATION_HTTP || eOperation == OPERATION_URL))
    {
        // Get and walk through get array, find string match (case insensitive)
        // Build new put array without that entry, unless down to 0 -> put null variant
        DWORD dwNumElements = 0;
        PWSTR* ppszValue = NULL;
        hr = WpcsWebExtensExemptionsGet(piWmiServices, eOperation, &dwNumElements, 
            &ppszValue);
        if (SUCCEEDED(hr))
        {
            BOOL fMatch = FALSE;
            DWORD dwIndexMatch = 0;
            for (DWORD i = 0; i < dwNumElements; i++)
            {
                if (_wcsicmp(pcszExemption, ppszValue[i]) == 0)
                {
                    fMatch = TRUE;
                    dwIndexMatch = i;
                    break;
                }
            }
            if (!fMatch)
            {
                hr = E_FAIL;
            }
            else
            {
                // Prepare to write new array
                PCWSTR pcszProperty = (eOperation == OPERATION_HTTP) ? 
                    L"HTTPExemptionList" : L"URLExemptionList";
                IWbemClassObject* piWmiSystemSettings = NULL;
                hr = WpcuWmiObjectGet(piWmiServices, WPCS_WMI_SYSTEM_SETTINGS, 
                    &piWmiSystemSettings);
                if (SUCCEEDED(hr))
                {
                    if (dwNumElements > 1)
                    {
                        // If at least 2 elements present, create new array
                        // that is one smaller
                        PCWSTR* ppcszNewValue = new PCWSTR[dwNumElements - 1];
                        if (!ppcszNewValue)
                        {
                            hr = E_OUTOFMEMORY;
                        }
                        else
                        {
                            // Shallow copy all except the matching element
                            for (DWORD i = 0; i < dwNumElements; i++)
                            {
                                if (i < dwIndexMatch)
                                {
                                    ppcszNewValue[i] = ppszValue[i];
                                }
                                else if (i > dwIndexMatch)
                                {
                                    ppcszNewValue[i - 1] = ppszValue[i];
                                }
                            }
                            // Write array to instance
                            hr = WpcuWmiInstancePutStringArray(piWmiSystemSettings, 
                                                               pcszProperty, 
                                                               dwNumElements - 1, 
                                                               ppcszNewValue);
                            if (SUCCEEDED(hr))
                            {
                                hr = piWmiServices->PutInstance(piWmiSystemSettings,
                                                                WBEM_FLAG_UPDATE_ONLY, 
                                                                NULL, 
                                                                NULL);
                            }
                            delete[] ppcszNewValue;
                        }
                    }
                    else
                    {
                        // Only one element was originally present.  Write a NULL variant
                        hr = WpcuWmiInstancePutNULLVariant(piWmiSystemSettings, pcszProperty);
                        if (SUCCEEDED(hr))
                        {
                            hr = piWmiServices->PutInstance(piWmiSystemSettings,
                                                            WBEM_FLAG_UPDATE_ONLY, 
                                                            NULL, 
                                                            NULL);
                        }
                    }
                    piWmiSystemSettings->Release();
                }
            }
            // Clean up allocations from WpcsWebExtensExemptionsGet()
            for (DWORD i = 0; i < dwNumElements; i++)
            {
                if (ppszValue[i] != NULL)
                {
                    delete[] ppszValue[i];
                    ppszValue[i] = NULL;
                }
            }
            if (ppszValue)
            {
                delete[] ppszValue;
                ppszValue = NULL;
            }
        }
    }

    return (hr);
}


// Get FilterID and FilterNamePath properties from SystemSettings instance.  Allows
//  expressing cases where either are stored as type VT_NULL by having the returned
//  pointers be null
// Calling code is responsible for freeing the ID and Name Path strings passed back
HRESULT WpcsWebExtensFilterGet(IWbemServices* piWmiServices, PWSTR* ppszFilterID,
                              PWSTR* ppszFilterNamePath)
{
    HRESULT hr = E_INVALIDARG;

    // Do basic parameter validation
    if (piWmiServices && ppszFilterID && ppszFilterNamePath)
    {
        IWbemClassObject* piWmiSystemSettings;
        hr = WpcuWmiObjectGet(piWmiServices, WPCS_WMI_SYSTEM_SETTINGS, 
                              &piWmiSystemSettings);
        if (SUCCEEDED(hr))
        {
            hr = WpcuWmiStringFromInstance(piWmiSystemSettings, L"FilterID", 
                ppszFilterID);
            if (SUCCEEDED(hr))
            {
                hr = WpcuWmiStringFromInstance(piWmiSystemSettings, L"FilterName", 
                    ppszFilterNamePath);
            }

            piWmiSystemSettings->Release();
        }
    }

    return (hr);
}

HRESULT WpcsWebExtensFilterSet(IWbemServices* piWmiServices, PCWSTR pcszFilterID,
                              PCWSTR pcszFilterNamePath)
{
    HRESULT hr = E_INVALIDARG;

    // Do basic parameter validation
    // Note:  pcszFilterID and pcszFilterNamePath cannot be NULL
    if (piWmiServices && pcszFilterID && pcszFilterNamePath)
    {
        IWbemClassObject* piWmiSystemSettings;
        hr = WpcuWmiObjectGet(piWmiServices, WPCS_WMI_SYSTEM_SETTINGS, 
                              &piWmiSystemSettings);
        if (SUCCEEDED(hr))
        {
            //  This property must always be written as a properly formatted 
            //   GUID for the instance update to be successful.  Note that Windows
            //   Vista's content filter is signified by a zeroed GUID - 
            //   {00000000-0000-0000-0000-000000000000}
            hr = WpcuWmiInstancePutString(piWmiSystemSettings, L"FilterID", 
                pcszFilterID);
            if (SUCCEEDED(hr))
            {
                hr = WpcuWmiInstancePutString(piWmiSystemSettings, 
                    L"FilterName", pcszFilterNamePath);
                if (SUCCEEDED(hr))
                {
                    hr = piWmiServices->PutInstance(piWmiSystemSettings,
                                        WBEM_FLAG_UPDATE_ONLY, NULL, NULL);
                }
            }
            piWmiSystemSettings->Release();
        }
    }

    return (hr);
}


HRESULT WpcsWebExtensFilterReset(IWbemServices* piWmiServices)
{
    HRESULT hr = E_INVALIDARG;

    // Do basic parameter validation
    if (piWmiServices)
    {
        IWbemClassObject* piWmiSystemSettings;
        hr = WpcuWmiObjectGet(piWmiServices, WPCS_WMI_SYSTEM_SETTINGS,
                              &piWmiSystemSettings);
        if (SUCCEEDED(hr))
        {
            hr = WpcuWmiInstancePutNULLVariant(piWmiSystemSettings, L"FilterName");
            if (SUCCEEDED(hr))
            {
                hr = WpcuWmiInstancePutString(piWmiSystemSettings, L"FilterID", 
                                              L"{00000000-0000-0000-0000-000000000000}");
                if (SUCCEEDED(hr))
                {
                    hr = piWmiServices->PutInstance(piWmiSystemSettings,
                                                    WBEM_FLAG_UPDATE_ONLY, 
                                                    NULL, 
                                                    NULL);
                }
            }
            piWmiSystemSettings->Release();
        }
    }

    return (hr);
}




// Parse the command line
HRESULT CmdLineParse(int argc, WCHAR* argv[], OPERATION* peOperation, 
                     SUBOPERATION* peSubOperation, PCWSTR* ppcszExemption,
                     PCWSTR* ppcszFilterID, PCWSTR* ppcszFilterNamePath)
{
    HRESULT hr = E_INVALIDARG;

    // Do basic parameter validation
    if (peOperation && ppcszExemption && ppcszFilterID && ppcszFilterNamePath)
    {
        // Initialize defaults
        *ppcszExemption = NULL;
        *ppcszFilterNamePath = NULL;
        *ppcszFilterID = NULL;

        // Determine operational mode and check prerequisites
        if (argc >= ARGS_MIN)
        {
            if (_wcsicmp(argv[1], L"http") == 0)
            {
                *peOperation = OPERATION_HTTP;
            }
            else if (_wcsicmp(argv[1], L"url") == 0)
            {
                *peOperation = OPERATION_URL;
            }
            else if (_wcsicmp(argv[1], L"filter") == 0)
            {
                *peOperation = OPERATION_FILTER;
            }
            else 
            {
                peOperation = NULL;
            }

            if (peOperation)
            {
                if ((*peOperation == OPERATION_HTTP) ||
                    (*peOperation == OPERATION_URL))
                {
                    if (argc == 3)
                    {
                        if (_wcsicmp(argv[2], L"list") == 0)
                        {
                            *peSubOperation = EXEMPTION_LIST;
                            hr = S_OK;
                        }
                    }
                    else if (argc == 4)
                    {
                        if (_wcsicmp(argv[2], L"add") == 0)
                        {
                            *peSubOperation = EXEMPTION_ADD;
                            *ppcszExemption = argv[3];
                            hr = S_OK;
                        }
                        else if (_wcsicmp(argv[2], L"del") == 0)
                        {
                            // Handle del string, argv[3]
                            *peSubOperation = EXEMPTION_DEL;
                            *ppcszExemption = argv[3];
                            hr = S_OK;
                        }
                    }
                }
                else if (*peOperation == OPERATION_FILTER)
                {
                    if (argc == 3)
                    {
                         if (_wcsicmp(argv[2], L"get") == 0)
                        {
                            *peSubOperation = ID_NAME_GET;
                            hr = S_OK;
                        }
                        else if (_wcsicmp(argv[2], L"reset") == 0)
                        {
                            *peSubOperation = ID_NAME_RESET;
                            hr = S_OK;
                        }
                    }
                    else if (argc == 5)
                    {
                        if (_wcsicmp(argv[2], L"set") == 0)
                        {
                            *peSubOperation = ID_NAME_SET;
                    
                            for (int i = 3; i < argc; i++)
                            {
                                if ((_wcsnicmp(argv[i], L"/g:", 3) == 0) || 
                                    (_wcsnicmp(argv[i], L"-g:", 3) == 0))
                                {
                                    *ppcszFilterID = argv[i] + 3;
                                }
                                else if ((_wcsnicmp(argv[i], L"/n:", 3) == 0) || 
                                         (_wcsnicmp(argv[i], L"-n:", 3) == 0))
                                {
                                    *ppcszFilterNamePath = argv[i] + 3;
                                }
                            }

                            if (*ppcszFilterID && *ppcszFilterNamePath)
                            {
                                hr = S_OK;
                            }
                        }
                    }
                }
            }
        }
    }

    return (hr);
}


