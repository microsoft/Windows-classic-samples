
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

/****************************************************************************

    FILE: Windows Parental Controls (WPC) User Interface Extensibility sample

    PURPOSE: Demonstrates managing ISV-registered links in the Parental
             Controls User Interface central page.  Instances of the 
             WpcExtension WMI class in the WPC namespace are manipulated
             for this purpose.


    FUNCTIONS:

        wmain() - implements overall command line application
        WpcsUIExtensEnum() - returns an iterator for enumerating 
          instances of the WpcExtension class 
        WpcsUIExtensQuery() - returns an iterator for obtaining
          matching results from a WMI query
        WpcsUIExtensAdd() - adds a new WpcExtension class instance
          entry
        WpcsUIExtensModify() - modifies one or more properties in 
          an existing WpcExtension instance
        WpcsUIExtensDelete() - removes a WpcExtension instance
        GetObjectPath() - helper function to build a BSTR needed
          to specify a given WpcExtension instance
        PrintEntries() - example function showing use of an 
          iterator returned from WpcsUIExtensEnum() or
          WpcsUIExtensQuery() to print results
        CmdLineParse() - handle command line input
        GetParameter() - helper for CmdLineParse()


    COMMENTS:
        A UI Extensibility link is specified by a GUID and a Subsystem number.
        The only Subsystem currently used is 0, but future changes may add more
        restrictions categories (subsystems) differentiated in the UI.  Information
        for the extensibility link consists of:
            Name resource DLL path and ID string
            SubTitle resource DLL path and ID string
            Image (icon resource) resource DLL path and ID string
			Disabled state image resource DLL path and ID string
            Executable path string
        The specified exe path is invoked when the user clicks on the link as
        shown in the Parental Controls control panel.

****************************************************************************/

#include "UIExtensibility.h"


HRESULT WpcsUIExtensEnum(IWbemServices* piWmiServices, 
                         IEnumWbemClassObject** ppiEnumerator);

HRESULT WpcsUIExtensQuery(IWbemServices* piWmiServices, PCWSTR pcszQuery, 
                          IEnumWbemClassObject** ppiEnumerator);

HRESULT WpcsUIExtensAdd(IWbemServices* piWmiServices, UXENTRY* pEntry);

HRESULT WpcsUIExtensModify(IWbemServices* piWmiServices, DWORD dwMask, 
                           UXENTRY* pEntry);

HRESULT WpcsUIExtensDelete(IWbemServices* piWmiServices, UXENTRY* pEntry);

HRESULT GetObjectPath(PCWSTR pcszGuid, UINT nSubsystem, 
                                      BSTR* pbstrObjectPath);

HRESULT CmdLineParse(int argc, WCHAR* argv[], PARSERESULT* pParseResult);

HRESULT GetParameter(PCWSTR pcszArg, PARSERESULT* pParseResult);

void Usage (PCWSTR pcszProgramName);

HRESULT PrintEntries(IEnumWbemClassObject* piEnumerator);


// call as Usage(argv[0])
void Usage (PCWSTR pcszProgramName)
{
    wprintf(L"Usage:  %s <operation> [argument1..n]\n", pcszProgramName);
    wprintf(L" Where operations and associated arguments are as follows:\n\n");
    wprintf(L"list \t\t\t\tList registered extensions\n\n");
    wprintf(L"query <query string> \t\tPerform query and list matches\n\n");
	wprintf(L"add /g:<GUID> /c:<subsystem> /n:<name_path> /s:<subtitle_path> \n\t /i:<image_path> /d:<disabled_image_path> /e:<exe_path>\n");
    wprintf(L"\t\t\t\tAdd new extension keyed by GUID string\n\t\t\t\tand subsystem (currently 0)\n\n");
	wprintf(L"mod /g:<GUID> /c:<subsystem> [/n:<name_path> | /s:<subtitle_path> | \n\t /i:<image_path> | /d:<disabled_image_path> | /e:<exe_path>]\n");
    wprintf(L"\t\t\t\tModify one or more elements \n\t\t\t\tin existing extension\n\n");
    wprintf(L"del /g:<GUID> /c:<subsystem> \tDelete extension\n\n");
}


// Application entry point
int __cdecl wmain(int argc, __in_ecount(argc) WCHAR* argv[])
{

    // Declare command line parsing result structure
    PARSERESULT stParseResult;

    // Parse operational mode and arguments from command line.
    //  Function is responsible for printing its own errors
    HRESULT hr = CmdLineParse(argc, argv, &stParseResult);
    if (hr == E_INVALIDARG)
    {
        // Print usage and bypass further initialization
        Usage(argv[0]);
    }
    else if (SUCCEEDED(hr))
    {
        hr = WpcuCOMInit();
        if (FAILED(hr))
        {
            wprintf(L"Error:  Failed to initialize COM, hr is %8x.\n", hr);
        }
    }
    if (SUCCEEDED(hr))
    {
        // Declare and initialize WPC API interface pointers
        IWbemServices* piWmiServices = NULL;
        // Connect to WPC namespace in WMI on local machine
        hr = WpcuWmiConnect(WPCS_WMI_NAMESPACE, &piWmiServices);
        if (FAILED(hr))
        {
            wprintf(L"Error:  WpcuWmiConnect() failed, hr is %8x.\n", hr);
        }
        else
        {
            // Perform mode-specific operations
            IEnumWbemClassObject* piEnum = NULL;
            switch (stParseResult.eOperation)
            {
                case OPERATION_LIST:
                    hr = WpcsUIExtensEnum(piWmiServices, &piEnum);
                    if (FAILED(hr))
                    {
                        wprintf(L"Error:  WpcsUIExtensEnum() failed, hr is %8x.\n", hr);
                    }
                    else
                    {
                        hr = PrintEntries(piEnum);
                    }
                    break;

                case OPERATION_QUERY:
                    hr = WpcsUIExtensQuery(piWmiServices, stParseResult.pszQuery, 
						&piEnum);
                    if (FAILED(hr))
                    {
                        wprintf(L"Error:  WpcsUIExtensQuery() failed, hr is %8x.\n", hr);
                    }
                    else
                    {
                        hr = PrintEntries(piEnum);
                    }
                    break;

                case OPERATION_ADD:
                    hr = WpcsUIExtensAdd(piWmiServices, &(stParseResult.stEntry));
                    if (FAILED(hr))
                    {
                        wprintf(L"Error:  WpcsUIExtensAdd() failed, hr is %8x.\n", hr);
                    }
                    else
                    {
                        wprintf(L"Info:  Extension entry added.\n");
                    }
                    break;

                case OPERATION_MOD:
                    hr = WpcsUIExtensModify(piWmiServices, stParseResult.dwMask, 
                        &(stParseResult.stEntry));
                    if (FAILED(hr))
                    {
                        wprintf(L"Error:  WpcsUIExtensModify() failed, hr is %8x.\n", hr);
                    }
                    else
                    {
                        wprintf(L"Info:  Extension entry modified.\n");
                    }
                    break;

                case OPERATION_DEL:
                    hr = WpcsUIExtensDelete(piWmiServices, &(stParseResult.stEntry));
                    if (FAILED(hr))
                    {
                        wprintf(L"Error:  WpcsUIExtensDelete() failed, hr is %8x.\n", hr);
                    }
                    else
                    {
                        wprintf(L"Info:  Extension entry deleted.\n");
                    }
                    break;
            }

            // Cleanup
            if (piEnum != NULL)
            {
                piEnum->Release();
            }
            if (piWmiServices != NULL)
            {
                piWmiServices->Release();
            }
        }

    }

    WpcuCOMCleanup();

    return (SUCCEEDED(hr)) ? 0 : 1;
}

HRESULT WpcsUIExtensEnum(IWbemServices* piWmiServices, 
                        IEnumWbemClassObject** ppiEnumerator)
{
    if (!piWmiServices || !ppiEnumerator)
    {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    BSTR bstrClass = SysAllocString(WPCS_WMI_UI_EXTENSION);
    if (bstrClass == NULL)
    {
        hr = E_OUTOFMEMORY;
    }
    else
    {
        hr = piWmiServices->CreateInstanceEnum(bstrClass, WBEM_FLAG_SHALLOW, NULL, 
            ppiEnumerator);
        SysFreeString(bstrClass);
    }

    return (hr);
}

HRESULT WpcsUIExtensQuery(IWbemServices* piWmiServices, PCWSTR pcszQuery, 
                         IEnumWbemClassObject** ppiEnumerator) 
{
    if (!piWmiServices || !pcszQuery || !ppiEnumerator) 
    {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    BSTR bstrWQL = SysAllocString(WPCS_WMI_STRING_WQL);
    if (bstrWQL == NULL)
    {
        hr = E_OUTOFMEMORY;
    }
    else
    {
        // Build query string
        BSTR bstrQuery = SysAllocString(pcszQuery);
        if (bstrQuery == NULL)
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
            // Obtain enumerator for query
            hr = piWmiServices->ExecQuery(bstrWQL, bstrQuery, 
                WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, 
                                         NULL, ppiEnumerator);

            SysFreeString(bstrQuery);
        }

        SysFreeString(bstrWQL);
    }

    return (hr);
}

HRESULT WpcsUIExtensAdd(IWbemServices* piWmiServices, UXENTRY* pEntry)
{
    if (!piWmiServices || !pEntry || !(pEntry->pszGuid))
    {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    BSTR bstrObjectPath = SysAllocString(WPCS_WMI_UI_EXTENSION);
    if (bstrObjectPath == NULL)
    {
        hr = E_OUTOFMEMORY;
    }
    else
    {
        // Get the class object    
        IWbemClassObject* piClass;
        hr = piWmiServices->GetObject(bstrObjectPath, WBEM_FLAG_RETURN_WBEM_COMPLETE, 
            NULL, &piClass, NULL);
        if (SUCCEEDED(hr))
        {
            // Spawn a new instance
            IWbemClassObject* piInstance;
            hr = piClass->SpawnInstance(0, &piInstance);
            if (SUCCEEDED(hr))
            {
                // Fill in the properties
                hr = WpcuWmiInstancePutString(piInstance, L"ID", pEntry->pszGuid);
                if (SUCCEEDED(hr))
                {
                    hr = WpcuWmiInstancePutDWORD(piInstance, L"Subsystem", 
						pEntry->nSubsystem);
                    if (SUCCEEDED(hr) && pEntry->pszNamePath)
                    {
                        hr = WpcuWmiInstancePutString(piInstance, L"Name", 
                            pEntry->pszNamePath);
                    }
                    if (SUCCEEDED(hr) && pEntry->pszSubTitlePath)
                    {
                        hr = WpcuWmiInstancePutString(piInstance, L"SubTitle", 
                            pEntry->pszSubTitlePath);
                    }
                    if (SUCCEEDED(hr) && pEntry->pszImagePath)
                    {
                        hr = WpcuWmiInstancePutString(piInstance, L"ImagePath", 
                            pEntry->pszImagePath);
                    }
					if (SUCCEEDED(hr) && pEntry->pszDisabledImagePath)
                    {
                        hr = WpcuWmiInstancePutString(piInstance, L"DisabledImagePath", 
                            pEntry->pszDisabledImagePath);
                    }
                    if (SUCCEEDED(hr) && pEntry->pszExePath)
                    {
                        hr = WpcuWmiInstancePutString(piInstance, L"Path", 
                            pEntry->pszExePath);
                    }

                    if (SUCCEEDED(hr))
                    {
                        // Put instance
                        hr = piWmiServices->PutInstance(piInstance, 
                            WBEM_FLAG_CREATE_ONLY, NULL, NULL);
                    }
                }
                
                piInstance->Release();
            }
            piClass->Release();
        }
        SysFreeString(bstrObjectPath);
    }

    return (hr);
}

HRESULT WpcsUIExtensModify(IWbemServices* piWmiServices, DWORD dwMask,  UXENTRY* pEntry)
{
    if (!piWmiServices || !pEntry || !pEntry->pszGuid || (pEntry->nSubsystem > 9))
    {
        return E_INVALIDARG;
    }

    BSTR bstrObjectPath;
    HRESULT hr = GetObjectPath(pEntry->pszGuid, pEntry->nSubsystem, &bstrObjectPath);
    if (SUCCEEDED(hr))
    {
        // Get existing object
        IWbemClassObject* piInstance;
        hr = piWmiServices->GetObject(bstrObjectPath, WBEM_FLAG_RETURN_WBEM_COMPLETE, 
			NULL, &piInstance, NULL);
        if (SUCCEEDED(hr))
        {
            // Put data as needed
			if (dwMask & UMASK_NAMEPATH)
            {
                hr = WpcuWmiInstancePutString(piInstance, L"Name", pEntry->pszNamePath);
            }

            if (SUCCEEDED(hr) && (dwMask & UMASK_SUBTITLEPATH))
            {
				hr = WpcuWmiInstancePutString(piInstance, L"SubTitlePath", 
					pEntry->pszSubTitlePath);
            }

            if (SUCCEEDED(hr) && (dwMask & UMASK_IMAGEPATH))
            {
                hr = WpcuWmiInstancePutString(piInstance, L"ImagePath", 
					pEntry->pszImagePath);
            }

			if (SUCCEEDED(hr) && (dwMask & UMASK_DISABLEDIMAGEPATH))
            {
                hr = WpcuWmiInstancePutString(piInstance, L"DisabledImagePath", 
					pEntry->pszImagePath);
            }

            if (SUCCEEDED(hr) && (dwMask & UMASK_EXEPATH))
            {
                hr = WpcuWmiInstancePutString(piInstance, L"Path", pEntry->pszExePath);
            }

            if (SUCCEEDED(hr))
            {
                // Put instance - use UPDATE_ONLY for modify
                hr = piWmiServices->PutInstance(piInstance, WBEM_FLAG_UPDATE_ONLY, NULL, 
					NULL);
            }
            piInstance->Release();
        }
        
        SysFreeString(bstrObjectPath);
    }

    return (hr);
}

HRESULT WpcsUIExtensDelete(IWbemServices* piWmiServices, UXENTRY* pEntry)
{
    if (!piWmiServices || !pEntry || !pEntry->pszGuid || (pEntry->nSubsystem > 9))
    {
        return E_INVALIDARG;
    }

    BSTR bstrObjectPath;
    HRESULT hr = GetObjectPath(pEntry->pszGuid, pEntry->nSubsystem, &bstrObjectPath);
    if (SUCCEEDED(hr))
    {
        hr = piWmiServices->DeleteInstance(bstrObjectPath, 0, NULL, NULL);
        SysFreeString(bstrObjectPath);
    }

    return (hr);
}


//
// Helper functions
//

HRESULT GetObjectPath(PCWSTR pcszGuid, UINT nSubsystem, BSTR* pbstrObjectPath)
{
    if (!pcszGuid || !pbstrObjectPath)
    {
        return E_INVALIDARG;
    }
    
    // Build the instance string. Max size is classname + format string (-wildcards) + 
    // guid length + 1 subsystem digit + NULL, but it is fine to slightly overestimate.
    size_t cch = wcslen(WPCS_WMI_UI_EXTENSION) + wcslen(WPCS_WMI_UI_EXTENSION_FORMAT_KEYS) 
		+ wcslen(pcszGuid) + 2;
    HRESULT hr = S_OK;
    PWSTR pszObjectPath = new WCHAR[cch];
    if (!pszObjectPath)
    {
        hr = E_OUTOFMEMORY;
    }
    else
    {
        hr = StringCchPrintfW(pszObjectPath, cch, WPCS_WMI_UI_EXTENSION_FORMAT_KEYS, 
			WPCS_WMI_UI_EXTENSION, pcszGuid, nSubsystem);
        if (SUCCEEDED(hr))
        {
            // Convert object path to BSTR
            *pbstrObjectPath = SysAllocString(pszObjectPath);
            if (*pbstrObjectPath == NULL)
            {
                hr = E_OUTOFMEMORY;
            }
            else
            {
                hr = S_OK;
            }
        }
        delete [] pszObjectPath;
    }
    return hr;
    
}


HRESULT PrintEntries(IEnumWbemClassObject* piEnumerator)
{
    if (!piEnumerator)
    {
        return E_INVALIDARG;
    }
    
    // Print out list
    wprintf(L"Info:  UI extension list:\n");

    //
    // Loop until we fail or receive S_FALSE (no more to come)
    //
    HRESULT hr = S_OK;
    IWbemClassObject* piInstance;
    ULONG cReturned = 0;
    ULONG i=0;
    do 
    {
        hr = piEnumerator->Next(WBEM_INFINITE, 1, &piInstance, &cReturned);
        if (FAILED(hr))
        {
            wprintf(L"Error:  IEnumWbemClassObject::Next() failed, hr is %08x.\n", hr);
            if (hr == WBEM_E_INVALID_QUERY)
            {
                wprintf(L"Error:\t\tInvalid query.\n");
            }
        }
        else if (cReturned)
        {
            PWSTR psz;
            DWORD dw;
            ++i; //increment count

            // Print it ignoring any failed entries
            wprintf(L"\tEntry %d:\n", i);

            HRESULT hrProp = WpcuWmiStringFromInstance(piInstance, L"ID", &psz);
            if (SUCCEEDED(hrProp))
            {
                if (psz)
                {
                    wprintf(L"\t\tGUID:\t\t%s\n", psz);
                    delete [] psz;
                }
                else
                {
                    wprintf(L"\t\tGUID: Undefined\n");
                }
            }
            else
            {
                wprintf(L"Error:  Failed to retrieve GUID, hrProp is %8x.\n", hrProp);
            }

            hrProp = WpcuWmiDWORDFromInstance(piInstance, L"Subsystem", &dw);
            if (SUCCEEDED(hrProp))
            {
                wprintf(L"\t\tSubsystem:\t\t%u\n", dw);
            }
            else
            {
                wprintf(L"Error:  Failed to retrieve Subsystem, hrProp is %8x.\n", 
					hrProp);
            }
            
            hrProp = WpcuWmiStringFromInstance(piInstance, L"Name", &psz);
            if (SUCCEEDED(hrProp))
            {
                if (psz)
                {
                    wprintf(L"\t\tName:\t\t%s\n", psz);
                    delete [] psz;
                }
                else
                {
                    wprintf(L"\t\tName: Undefined\n");
                }
            }
            else
            {
                wprintf(L"Error:  Failed to retrieve Name, hrProp is %8x.\n", hrProp);
            }
            
            hrProp = WpcuWmiStringFromInstance(piInstance, L"SubTitle", &psz);
            if (SUCCEEDED(hrProp))
            {
                if (psz)
                {
                    wprintf(L"\t\tSubTitle:\t%s\n", psz);
                    delete [] psz;
                }
                else
                {
                    wprintf(L"\t\tSubTitle: Undefined\n");
                }
            }
            else
            {
                wprintf(L"Error:  Failed to retrieve SubTitle, hrProp is %8x.\n", hrProp);
            }
            
            hrProp = WpcuWmiStringFromInstance(piInstance, L"Path", &psz);
            if (SUCCEEDED(hrProp))
            {
                if (psz)
                {
                    wprintf(L"\t\tExePath:\t\t%s\n", psz);
                    delete [] psz;
                }
                else
                {
                    wprintf(L"\t\tExePath: Undefined\n");
                }
            }
            else
            {
                wprintf(L"Error:  Failed to retrieve ExePath, hrProp is %8x.\n", hrProp);
            }
            
            hrProp = WpcuWmiStringFromInstance(piInstance, L"ImagePath", &psz);
            if (SUCCEEDED(hrProp))
            {
                if (psz)
                {
                    wprintf(L"\t\tImagePath:\t%s\n", psz);
                    delete [] psz;
                }
                else
                {
                    wprintf(L"\t\tImagePath: Undefined\n");
                }
            }
            else
            {
                wprintf(L"Error:  Failed to retrieve ImagePath, hrProp is %8x.\n", 
					hrProp);
            }
			
			hrProp = WpcuWmiStringFromInstance(piInstance, L"DisabledImagePath", &psz);
            if (SUCCEEDED(hrProp))
            {
                if (psz)
                {
                    wprintf(L"\t\tDisabledImagePath:\t%s\n", psz);
                    delete [] psz;
                }
                else
                {
                    wprintf(L"\t\tDisabledImagePath: Undefined\n");
                }
            }
            else
            {
                wprintf(L"Error:  Failed to retrieve DisabledImagePath, hrProp is %8x.\n", 
					hrProp);
            }

            // Free the interface
            piInstance->Release();
        }
    } while (hr == WBEM_S_NO_ERROR);

    wprintf(L"Info:  End of list.\n");
    return hr;
}

HRESULT GetParameter(PWSTR pszArg, PARSERESULT* pParseResult)
{
    // Default to invalid
    HRESULT hr = S_OK;

    if (wcslen(pszArg) < 3)
    {
        hr = E_INVALIDARG;
    }
    else 
    {
        if ((_wcsnicmp(pszArg, L"/g:", 3) == 0) || (_wcsnicmp(pszArg, L"-g:", 3) == 0))
        {
            pParseResult->stEntry.pszGuid = &(pszArg[3]);
            pParseResult->dwMask |= UMASK_GUID;
        }
        else if ((_wcsnicmp(pszArg, L"/c:", 3) == 0) || (_wcsnicmp(pszArg, L"-c:", 3) 
			== 0))
        {
            pParseResult->stEntry.nSubsystem = _wtoi(&(pszArg[3]));
            pParseResult->dwMask |= UMASK_SUBSYSTEM;
        } 
        else if ((_wcsnicmp(pszArg, L"/n:", 3) == 0) || (_wcsnicmp(pszArg, L"-n:", 3) 
			== 0))
        {
            pParseResult->stEntry.pszNamePath = &(pszArg[3]);
            pParseResult->dwMask |= UMASK_NAMEPATH;
        }
        else if ((_wcsnicmp(pszArg, L"/s:", 3) == 0) || (_wcsnicmp(pszArg, L"-s:", 3) 
			== 0))
        {
            pParseResult->stEntry.pszSubTitlePath = &(pszArg[3]);
            pParseResult->dwMask |= UMASK_SUBTITLEPATH;
        }
        else if ((_wcsnicmp(pszArg, L"/i:", 3) == 0) || (_wcsnicmp(pszArg, L"-i:", 3) 
			== 0))
        {
            pParseResult->stEntry.pszImagePath= &(pszArg[3]);
            pParseResult->dwMask |= UMASK_IMAGEPATH;
        }
		else if ((_wcsnicmp(pszArg, L"/d:", 3) == 0) || (_wcsnicmp(pszArg, L"-d:", 3) 
			== 0))
        {
            pParseResult->stEntry.pszDisabledImagePath= &(pszArg[3]);
            pParseResult->dwMask |= UMASK_DISABLEDIMAGEPATH;
        }
        else if ((_wcsnicmp(pszArg, L"/e:", 3) == 0) || (_wcsnicmp(pszArg, L"-e:", 3) 
			== 0))
        {
            pParseResult->stEntry.pszExePath = &(pszArg[3]);
            pParseResult->dwMask |= UMASK_EXEPATH;
        }
        else
        {
            // Unkown Parameter
            hr = E_INVALIDARG;
        }
    }
    return hr;
}


// Parse the command line
HRESULT CmdLineParse(int argc, WCHAR* argv[], PARSERESULT* pParseResult)
{
    if (!pParseResult)
    {
        return E_FAIL;
    }

    HRESULT hr = E_INVALIDARG;
    ZeroMemory(pParseResult, sizeof(PARSERESULT));
    // Determine operational mode and check prerequisites
    if (argc >= ARGS_MIN)
    {
        if (_wcsicmp(argv[1], L"list") == 0)
        {
            if (argc == 2)
            {
                pParseResult->eOperation = OPERATION_LIST;
                hr = S_OK;
            }
        }
        else if (_wcsicmp(argv[1], L"query") == 0)
        {
            if (argc == 3)
            {
                pParseResult->eOperation = OPERATION_QUERY;
                pParseResult->pszQuery = argv[2];
                hr = S_OK;
            }
        }
        else if ((_wcsicmp(argv[1], L"add") == 0) || (_wcsicmp(argv[1], L"mod") == 0) || 
			(_wcsicmp(argv[1], L"del") == 0))
        {
            if (argc >= 4 && argc <= 9)
            {
                if (_wcsicmp(argv[1], L"add") == 0)
                {
                    pParseResult->eOperation = OPERATION_ADD;
                }
                else if (_wcsicmp(argv[1], L"mod") == 0)
                {
                    pParseResult->eOperation = OPERATION_MOD;
                }
                else
                {
                    pParseResult->eOperation = OPERATION_DEL;
                }
                
                hr = S_OK;
                for (int i=2;SUCCEEDED(hr) && i<argc; ++i)
                {
                    hr = GetParameter(argv[i], pParseResult);
                }
                // Verify we at least have the keys set, any unneeded ones will be ignored
                if (SUCCEEDED(hr) && (pParseResult->stEntry.pszGuid))
                {
                    hr = S_OK;
                }
            }
        }
    }

    return (hr);
}



