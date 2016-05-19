// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include <windows.h>
#include <stdio.h>
#include <shlwapi.h>
#include <propsys.h>
#include <strsafe.h>

void DumpProperties(PROPDESC_ENUMFILTER pdefFilter);
void DumpPropertyDescription(PCWSTR pszPropertyName);
void DumpPropertyKey(IPropertyDescription* ppdDump);
void DumpCanonicalName(IPropertyDescription* ppdDump);
void DumpDisplayName(IPropertyDescription* ppdDump);
void DumpEditInvitation(IPropertyDescription* ppdDump);
void DumpDefaultColumnWidth(IPropertyDescription* ppdDump);
void DumpSortDescriptionLabel(IPropertyDescription* ppdDump);
void DisplayUsage();
void RegisterSchema(PCWSTR pszFileName);
void UnregisterSchema(PCWSTR pszFileName);
void DisplayDumpInformation(int nNumArgs, PWSTR *pszArgList);
void RefreshSchema();
void ParsePropFlag(PCWSTR pszFlagToUse);

int wmain(int argc, wchar_t *argv[])
{
    //Must specify at least a flag, and at least one other argument depending on the flag.
    //See DisplayUsage for more information
    if (argc > 1)
    {
        // Initialize COM
        HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE);
        if (SUCCEEDED(hr))
        {
            //Check the first argument (-register, -unregister, -refresh, -dump, or -showprops)
            PCWSTR pszAction    = argv[1];
            PCWSTR pszArgument  = argv[2];

            if (!StrCmpICW(pszAction, L"-register") && argc > 2)
            {
                //attempt to register the schema
                RegisterSchema(pszArgument);
            }
            else if (!StrCmpICW(pszAction, L"-unregister") && argc > 2)
            {
                // attempt to unregister the schema
                UnregisterSchema(pszArgument);
            }
            else if (!StrCmpICW(pszAction, L"-dump") && argc > 2)
            {
                // iterate through each property name listed and dump property description info
                DisplayDumpInformation(argc,argv);
            }
            else if (!StrCmpICW(pszAction, L"-refresh"))
            {
                //refresh the schema, should be called after registering or unregistering
                RefreshSchema();
            }
            else if (!StrCmpICW(pszAction, L"-showprops") && argc > 2)
            {
                //check to see if the next argument is one we can accept
                ParsePropFlag(pszArgument);
            }
            else if (argc <= 2)
            {
                DisplayUsage();
            }
            else
            {
                // only -register, -unregister, -dump, refresh, and -showprops are supported
                hr = E_INVALIDARG;
                wprintf(L"Unrecognized flag: %s\n", pszAction);
            }

            CoUninitialize();
        }
        else
        {
            wprintf(L"CoInitialize failed with error: 0x%08x\n", hr);
        }
    }
    else
    {
        DisplayUsage();
    }

    return 0;
}

void DumpProperties(PROPDESC_ENUMFILTER pdefFilter)
{
    //get the list of properties that fall into the filter
    IPropertyDescriptionList *ppdl;
    HRESULT hr = PSEnumeratePropertyDescriptions(pdefFilter, IID_PPV_ARGS(&ppdl));
    if (SUCCEEDED(hr))
    {
        UINT cuProps = 0;
        //GetCount returns the number of properties that fall under the filter
        //specified in call to PSEnumeratePropertyDescriptions
        hr = ppdl->GetCount(&cuProps);
        if (SUCCEEDED(hr))
        {
            switch (pdefFilter)
            {
                //These enums are part of the PROPDESC_ENUMFILTER enum type
            case PDEF_ALL:
                wprintf(L"\nThere are %d properties in the system.\n", cuProps);
                break;
            case PDEF_SYSTEM:
                wprintf(L"\nThere are %d System properties.\n", cuProps);
                break;
            case PDEF_NONSYSTEM:
                wprintf(L"\nThere are %d NonSystem properties.\n", cuProps);
                break;
            case PDEF_VIEWABLE:
                wprintf(L"\nThere are %d Viewable properties.\n", cuProps);
                break;
            case PDEF_QUERYABLE:
                wprintf(L"\nThere are %d Queryable properties.\n", cuProps);
                break;
            case PDEF_INFULLTEXTQUERY:
                wprintf(L"\nThere are %d InFullTextQuery properties.\n", cuProps);
                break;
            }

            if (cuProps <= 0)
            {
                wprintf(L"No Properties to display\n");
            }
            else
            {
                wprintf(L"Printing the Canonical Name of the properties:\n");
                IPropertyDescription *ppd = NULL;;
                PWSTR pszCanonicalName;
                UINT iIndex;

                for (iIndex=0; iIndex < cuProps; iIndex++)
                {
                    //iterates through each item in the description list and
                    //displays the canonical name. Information in addition to Canonical Name
                    //is displayed via the -dump flag
                    hr = ppdl->GetAt(iIndex, IID_PPV_ARGS(&ppd));
                    if (SUCCEEDED(hr))
                    {
                        hr = ppd->GetCanonicalName(&pszCanonicalName);
                        if (SUCCEEDED(hr))
                        {
                            ppd->Release();
                            wprintf(L"%s\n", pszCanonicalName);
                        }
                        else
                        {
                            wprintf(L"IPropertyDescription::GetCanonicalName failed with error: 0x%08x\n", hr);
                        }
                    }
                    else
                    {
                        wprintf(L"IPropertyDescriptionList::GetAt failed with error: 0x%08x\n", hr);
                    }
                }
            }
        }
        else
        {
            wprintf(L"IPropertyDescriptionList::GetCount failed with error: 0x%08x\n", hr);
        }
    }
    else
    {
        wprintf(L"PSEnumeratePropertyDescriptions failed with error: 0x%08x\n", hr);
    }
}

void DumpPropertyDescription(PCWSTR pszPropertyName)
{
    IPropertyDescription* ppdDump;
    HRESULT hr = PSGetPropertyDescriptionByName(pszPropertyName, IID_PPV_ARGS(&ppdDump));
    if (SUCCEEDED(hr))
    {
        wprintf(L"%s\n", pszPropertyName);
        wprintf(L"----------------------------\n");

        DumpPropertyKey(ppdDump);
        DumpCanonicalName(ppdDump);
        DumpDisplayName(ppdDump);
        DumpEditInvitation(ppdDump);
        DumpDefaultColumnWidth(ppdDump);
        DumpSortDescriptionLabel(ppdDump);

        ppdDump->Release();
    }
    else
    {
        //This Error condition is a common one and should be handled specifically
        if (TYPE_E_ELEMENTNOTFOUND == hr)
        {
            wprintf(L"%s - Property Not found\n", pszPropertyName, hr);
        }
        else
        {
            wprintf(L"%s - Error 0x%08x obtaining Property Description\n", pszPropertyName, hr);
        }
    }
}

void DumpPropertyKey(IPropertyDescription* ppdDump)
{
    PROPERTYKEY key;
    //the unique property identifier
    if (SUCCEEDED(ppdDump->GetPropertyKey(&key)))
    {
        WCHAR wszKey[MAX_PATH] = {0};
        if (SUCCEEDED(PSStringFromPropertyKey(key, wszKey, ARRAYSIZE(wszKey))))
        {
            wprintf(L"Property Key:\t\t%s\n", wszKey);
        }
    }
}

void DumpCanonicalName(IPropertyDescription* ppdDump)
{
    PWSTR pszName = NULL;
    if (SUCCEEDED(ppdDump->GetCanonicalName(&pszName)))
    {
        wprintf(L"Canonical Name:\t\t%s\n", pszName);
        CoTaskMemFree(pszName);
    }
}

void DumpDisplayName(IPropertyDescription* ppdDump)
{
    PWSTR pszName = NULL;
    //the name as displayed in, for example, the Search Pane
    if (SUCCEEDED(ppdDump->GetDisplayName(&pszName)))
    {
        wprintf(L"Display Name:\t\t%s\n", pszName);
        CoTaskMemFree(pszName);
    }
}

void DumpEditInvitation(IPropertyDescription* ppdDump)
{
    PWSTR pszInvite = NULL;
    //the way in which the property is edited
    if (SUCCEEDED(ppdDump->GetEditInvitation(&pszInvite)))
    {
        wprintf(L"Edit Invitation:\t%s\n", pszInvite);
        CoTaskMemFree(pszInvite);
    }
}

void DumpDefaultColumnWidth(IPropertyDescription* ppdDump)
{
    UINT cchWidth;
    //the column width as displayed in the listview
    if (SUCCEEDED(ppdDump->GetDefaultColumnWidth(&cchWidth)))
    {
        wprintf(L"Default Column Width:\t%d\n", cchWidth);
    }
}

void DumpSortDescriptionLabel(IPropertyDescription* ppdDump)
{
    PWSTR pszAscending = NULL;
    //the manner in which this property is sorted
    if (SUCCEEDED(ppdDump->GetSortDescriptionLabel(FALSE, &pszAscending)))
    {
        PWSTR pszDescending = NULL;
        if (SUCCEEDED(ppdDump->GetSortDescriptionLabel(TRUE, &pszDescending)))
        {
            wprintf(L"Sort Description Label:\t%s/%s\n", pszAscending, pszDescending);
            CoTaskMemFree(pszDescending);
        }

        CoTaskMemFree(pszAscending);
    }
}

void RegisterSchema(PCWSTR pszFileName)
{
    HRESULT hr = PSRegisterPropertySchema(pszFileName);
    if (SUCCEEDED(hr))
    {
        wprintf(L"PSRegisterPropertySchema succeeded.\n");
    }
    else
    {
        wprintf(L"PSRegisterPropertySchema failed for schema file %s with error: 0x%08x\n", pszFileName, hr);
    }
}

void UnregisterSchema(PCWSTR pszFileName)
{
    HRESULT hr = PSUnregisterPropertySchema(pszFileName);
    if (SUCCEEDED(hr))
    {
        wprintf(L"PSUnregisterPropertySchema succeeded.\n");
    }
    else
    {
        wprintf(L"PSUnregisterPropertySchema failed for schema file %s with error: 0x%08x\n", pszFileName, hr);
    }
}

void DisplayDumpInformation(int nNumArgs, PWSTR *pszArgList)
{
    for (int argi = 2; argi < nNumArgs; argi++)
    {
        wprintf(L"\n");
        DumpPropertyDescription(pszArgList[argi]);
    }
}

void RefreshSchema()
{
    // NOTE: Currently this API is not supported, although it may be in the future.  In order to refresh an already registered schema, please use PSRegisterPropertySchema instead and pass in the same path which was used to register the schema initially.
    HRESULT hr = PSRefreshPropertySchema();
    if (SUCCEEDED(hr))
    {
        wprintf(L"PSRefreshPropertySchema succeeded.\n");
    }
    else
    {
        wprintf(L"PSRefreshPropertySchema failed with error 0x%08x\n", hr);
    }
}

void ParsePropFlag(PCWSTR pszFlagToUse)
{
    if (!StrCmpICW(pszFlagToUse, L"All"))
    {
        DumpProperties(PDEF_ALL);
    }
    else if (!StrCmpICW(pszFlagToUse, L"System"))
    {
        DumpProperties(PDEF_SYSTEM);
    }
    else if (!StrCmpICW(pszFlagToUse, L"NonSystem"))
    {
        DumpProperties(PDEF_NONSYSTEM);
    }
    else if (!StrCmpICW(pszFlagToUse, L"Viewable"))
    {
        DumpProperties(PDEF_VIEWABLE);
    }
    else if (!StrCmpICW(pszFlagToUse, L"Queryable"))
    {
        DumpProperties(PDEF_QUERYABLE);
    }
    else if (!StrCmpICW(pszFlagToUse, L"InFullTextQuery"))
    {
        DumpProperties(PDEF_INFULLTEXTQUERY);
    }
    else
    {
        DisplayUsage();
    }
}

void DisplayUsage()
{
    wprintf(L"Usage: [OPTIONS] [ARGUMENTS]\n\n");
    wprintf(L"Options:\n");
    wprintf(L" -register <filename>\t\t\t\tRegisters the provided property schema\n");
    wprintf(L" -unregister <filename>\t\t\t\tUnregisters the provided property schema\n");
    wprintf(L" -refresh\t\t\t\t\tRefreshes the property schema\n");
    wprintf(L" -dump <PropertyName1> <PropertyName2> ...\tShows information about each provided PropertyName\n");
    wprintf(L" -showprops <PropertyType>\t\t\tShows properties for the given property type\n");
    wprintf(L"Arguments:\n");
    wprintf(L"  filename\t\t\t\t\tProperty Schema to be registered\unregistered\n");
    wprintf(L"  PropertyName:\t\t\t\t\tThe Canonical Name of the property\n");
    wprintf(L"  Property Type:\t\t\t\tThe type of filter to be applied with what properties are shown\n");
    wprintf(L"\t\t\t\t\t\tAll\n\t\t\t\t\t\tSystem\n\t\t\t\t\t\tNonSytem\n\t\t\t\t\t\tViewable\n\t\t\t\t\t\tQueryable\n\t\t\t\t\t\tInFullTextQuery\n");
}
