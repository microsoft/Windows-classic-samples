// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

// This sample program reads lines from the console, parses them using the system
// schema, and displays the resulting condition trees. A real application might
// feed the condition trees to a graphical UI and/or translate them into a query
// language to be executed.

// To compile this sample to run also on Windows Vista, uncomment the following line.
// #define _WIN32_WINNT _WIN32_WINNT_VISTA

#include <stdio.h>
#include <propsys.h>
#include <propvarutil.h>
#include <shlwapi.h>
#include <structuredquery.h>

#pragma comment(lib, "propsys.lib")
#pragma comment(lib, "shlwapi.lib")

// Returns a label for a condition operation. Do not free the result.

#define MAP_ENTRY(x) {L#x, x}

PCWSTR GetOperationLabel(CONDITION_OPERATION op)
{
    PCWSTR pszLabel = NULL;

    static struct { PCWSTR pszLabel; CONDITION_OPERATION op; } const c_rgOperationLabels[] =
    {
        MAP_ENTRY(COP_IMPLICIT),
        MAP_ENTRY(COP_EQUAL),
        MAP_ENTRY(COP_NOTEQUAL),
        MAP_ENTRY(COP_LESSTHAN),
        MAP_ENTRY(COP_GREATERTHAN),
        MAP_ENTRY(COP_LESSTHANOREQUAL),
        MAP_ENTRY(COP_GREATERTHANOREQUAL),
        MAP_ENTRY(COP_VALUE_STARTSWITH),
        MAP_ENTRY(COP_VALUE_ENDSWITH),
        MAP_ENTRY(COP_VALUE_CONTAINS),
        MAP_ENTRY(COP_VALUE_NOTCONTAINS),
        MAP_ENTRY(COP_DOSWILDCARDS),
        MAP_ENTRY(COP_WORD_EQUAL),
        MAP_ENTRY(COP_WORD_STARTSWITH),
        MAP_ENTRY(COP_APPLICATION_SPECIFIC),
    };

    for (ULONG i = 0 ; !pszLabel && i < ARRAYSIZE(c_rgOperationLabels) ; ++i)
    {
        if (c_rgOperationLabels[i].op == op)
        {
            pszLabel = c_rgOperationLabels[i].pszLabel;
        }
    }

    if (!pszLabel)
    {
        pszLabel = L"???";
    }

    return pszLabel;
}

// Display to stdout one condition tree, with indentation.
// Code that targets only Windows 7 and later can take advantage of ICondition2 and IObjectArray.
// Code that targets also earlier versions of Windows and Windows Search should use ICondition and IEnumUnknown.
// Note that the two functions display leaves for search terms with no explicit property somewhat differently.

#if _WIN32_WINNT >= _WIN32_WINNT_WIN7
HRESULT DisplayQuery(ICondition2* pc, int cIndentation)
{
    CONDITION_TYPE ct;
    HRESULT hr = pc->GetConditionType(&ct);
    if (SUCCEEDED(hr))
    {
        switch (ct)
        {
        case CT_AND_CONDITION:
        case CT_OR_CONDITION:
            {
                wprintf(L"%*s%s\n", 2 * cIndentation, L"", (ct == CT_AND_CONDITION ? L"AND" : L"OR"));
                IObjectArray* poaSubs;
                hr = pc->GetSubConditions(IID_PPV_ARGS(&poaSubs));
                if (SUCCEEDED(hr))
                {
                    UINT cSubs;
                    hr = poaSubs->GetCount(&cSubs);
                    for (UINT i = 0 ; SUCCEEDED(hr) && i < cSubs ; ++i)
                    {
                        ICondition2* pcSub;
                        hr = poaSubs->GetAt(i, IID_PPV_ARGS(&pcSub));
                        if (SUCCEEDED(hr))
                        {
                            DisplayQuery(pcSub, cIndentation + 1);
                            pcSub->Release();
                        }
                    }
                    poaSubs->Release();
                }
            }
            break;
        case CT_NOT_CONDITION:
            {
                wprintf(L"%*s%s\n", 2 * cIndentation, L"", L"NOT");
                // ICondition::GetSubConditions can return the single subcondition of a negation node directly.
                ICondition2* pcSub;
                hr = pc->GetSubConditions(IID_PPV_ARGS(&pcSub));
                if (SUCCEEDED(hr))
                {
                    DisplayQuery(pcSub, cIndentation + 1);
                    pcSub->Release();
                }
            }
            break;
        case CT_LEAF_CONDITION:
            {
                PROPERTYKEY propkey;
                CONDITION_OPERATION op;
                PROPVARIANT propvar;
                hr = pc->GetLeafConditionInfo(&propkey, &op, &propvar);
                if (SUCCEEDED(hr))
                {
                    IPropertyDescription* ppd;
                    hr = PSGetPropertyDescription(propkey, IID_PPV_ARGS(&ppd));
                    if (SUCCEEDED(hr))
                    {
                        PWSTR pszPropertyName;
                        hr = ppd->GetCanonicalName(&pszPropertyName);
                        if (SUCCEEDED(hr))
                        {
                            PROPVARIANT propvarString;
                            hr = PropVariantChangeType(&propvarString, propvar, PVCHF_ALPHABOOL, VT_LPWSTR); // Real applications should prefer PSFormatForDisplay but we want more "raw" values.
                            if (SUCCEEDED(hr))
                            {
                                PWSTR pszSemanticType;
                                hr = pc->GetValueType(&pszSemanticType);
                                if (SUCCEEDED(hr))
                                {
                                    // The semantic type may be NULL; if so, do not display it at all.
                                    if (!pszSemanticType)
                                    {
                                        hr = SHStrDup(L"", &pszSemanticType);
                                    }

                                    if (SUCCEEDED(hr))
                                    {
                                        wprintf(L"%*sLEAF %s %s %s %s\n", 2 * cIndentation, L"", pszPropertyName, GetOperationLabel(op), propvarString.pwszVal, pszSemanticType);
                                    }
                                    CoTaskMemFree(pszSemanticType);
                                }
                                PropVariantClear(&propvarString);
                            }
                            CoTaskMemFree(pszPropertyName);
                        }
                        ppd->Release();
                    }
                    PropVariantClear(&propvar);
                }
            }
            break;
        }
    }
    return hr;
}
#else // _WIN32_WINNT >= _WIN32_WINNT_WIN7
HRESULT DisplayQuery(ICondition* pc, int cIndentation)
{
    CONDITION_TYPE ct;
    HRESULT hr = pc->GetConditionType(&ct);
    if (SUCCEEDED(hr))
    {
        switch (ct)
        {
        case CT_AND_CONDITION:
        case CT_OR_CONDITION:
            {
                wprintf(L"%*s%s\n", 2 * cIndentation, L"", (ct == CT_AND_CONDITION ? L"AND" : L"OR"));
                IEnumUnknown* peuSubs;
                hr = pc->GetSubConditions(IID_PPV_ARGS(&peuSubs));
                if (SUCCEEDED(hr))
                {
                    IUnknown* punk;
                    while ((hr = peuSubs->Next(1, &punk, NULL)) == S_OK)
                    {
                        ICondition* pcSub;
                        hr = punk->QueryInterface(IID_PPV_ARGS(&pcSub));
                        if (SUCCEEDED(hr))
                        {
                            DisplayQuery(pcSub, cIndentation + 1);
                            pcSub->Release();
                        }
                        punk->Release();
                    }
                    peuSubs->Release();
                }
            }
            break;
        case CT_NOT_CONDITION:
            {
                wprintf(L"%*s%s\n", 2 * cIndentation, L"", L"NOT");
                // ICondition::GetSubConditions can return the single subcondition of a negation node directly.
                ICondition* pcSub;
                hr = pc->GetSubConditions(IID_PPV_ARGS(&pcSub));
                if (SUCCEEDED(hr))
                {
                    DisplayQuery(pcSub, cIndentation + 1);
                    pcSub->Release();
                }
            }
            break;
        case CT_LEAF_CONDITION:
            {
                PWSTR pszPropertyName;
                CONDITION_OPERATION op;
                PROPVARIANT propvar;
                hr = pc->GetComparisonInfo(&pszPropertyName, &op, &propvar);
                if (SUCCEEDED(hr))
                {
                    // The property name may be NULL or "*"; if NULL, display it as such.
                    if (!pszPropertyName)
                    {
                        hr = SHStrDup(L"(NULL)", &pszPropertyName);
                    }

                    PROPVARIANT propvarString;
                    hr = PropVariantChangeType(&propvarString, propvar, PVCHF_ALPHABOOL, VT_LPWSTR); // Real applications should prefer PSFormatForDisplay but we want more "raw" values.
                    if (SUCCEEDED(hr))
                    {
                        PWSTR pszSemanticType;
                        hr = pc->GetValueType(&pszSemanticType);
                        if (SUCCEEDED(hr))
                        {
                            // The semantic type may be NULL; if so, do not display it at all.
                            if (!pszSemanticType)
                            {
                                hr = SHStrDup(L"", &pszSemanticType);
                            }

                            if (SUCCEEDED(hr))
                            {
                                wprintf(L"%*sLEAF %s %s %s %s\n", 2 * cIndentation, L"", pszPropertyName, GetOperationLabel(op), propvarString.pwszVal, pszSemanticType);
                            }
                            CoTaskMemFree(pszSemanticType);
                        }
                        PropVariantClear(&propvarString);
                    }
                    CoTaskMemFree(pszPropertyName);
                    PropVariantClear(&propvar);
                }
            }
            break;
        }
    }
    return hr;
}
#endif // _WIN32_WINNT >= _WIN32_WINNT_WIN7

int wmain(int /* argc */, wchar_t* /* argv */ [])
{
    wprintf(L"StructuredQuerySample\n");
    wprintf(L"Please enter a query in Advanced Query Syntax (AQS), or an empty line to exit\n");
    wprintf(L"the program. The program will parse the query, resolve the resulting condition\n");
    wprintf(L"tree and display it in tree form. Some sample inputs to try:\n");
    wprintf(L"from:bill               modified:last week      subject:(cats OR dogs)\n");
    wprintf(L"name:(bubble NOT bath)  author:~~george         taken:<October 2007\n");
    wprintf(L"kind:=folder            has:attachment          rating:****\n");
    wprintf(L"flower                  readstatus:read         size:42KB\n");
    wprintf(L"System.IsShared:=FALSE  exposuretime:>=0.125    received:5/25/2006 .. 7/17/2007\n");
    wprintf(L"Note that these queries are for English as UI language. If your UI language is\n");
    wprintf(L"different, you should use keywords from that language. Note though that\n");
    wprintf(L"System.IsShared:=FALSE is an example of a query in \"canonical syntax\", which\n");
    wprintf(L"will work for any UI language and should be used in programmatic use of AQS.\n");
    wprintf(L"MSDN on AQS: http://msdn.microsoft.com/en-us/library/aa965711(VS.85).aspx)\n");
    wprintf(L"Note also that any times in the condition trees are in UTC, not local time.\n");
    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (SUCCEEDED(hr))
    {
        // It is possible to CoCreateInstance a query parser directly using __uuidof(QueryParser) but by
        // using a QueryParserManager we can get a query parser prepared for a certain UI language and catalog.
        IQueryParserManager* pqpm;
        hr = CoCreateInstance(__uuidof(QueryParserManager), NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pqpm));
        if (SUCCEEDED(hr))
        {
            // The language given to IQueryParserManager::CreateLoadedParser should be the user interface language of the
            // application, which is often the same as the operating system's default UI language for this user. It is used for
            // deciding what language to be used when interpreting keywords.
            IQueryParser* pqp;
            hr = pqpm->CreateLoadedParser(L"SystemIndex", GetUserDefaultUILanguage(), IID_PPV_ARGS(&pqp));
            if (SUCCEEDED(hr))
            {
                // This sample turns off "natural query syntax", which is the more relaxed and human language like
                // search notation, and also makes searches automatically look for words beginning with what has been
                // specified (so an input of "from:bill" will search for anything from someone whose name begins with
                // "bill".
                hr = pqpm->InitializeOptions(FALSE, TRUE, pqp);
                WCHAR szLine[1024];
                while (SUCCEEDED(hr) && fgetws(szLine, ARRAYSIZE(szLine), stdin)[0] != L'\n')
                {
                    // The actual work of parsing a query string is done here.
                    IQuerySolution* pqs;
                    hr = pqp->Parse(szLine, NULL, &pqs);
                    if (SUCCEEDED(hr))
                    {
                        // In this sample we do not bother distinguishing between various parse errors though we could.
                        // Note that there will be a solution even if there were parse errors; it just may not be what
                        // the user intended.
                        IEnumUnknown* peu;
                        hr = pqs->GetErrors(IID_PPV_ARGS(&peu));
                        if (SUCCEEDED(hr))
                        {
                            if (peu->Skip(1) == S_OK)
                            {
                                wprintf(L"Some part of the query string could not be parsed.\n");
                            }
                            ICondition* pc;
                            hr = pqs->GetQuery(&pc, NULL);
                            if (SUCCEEDED(hr))
                            {
                                // IQueryCondition::Resolve and IConditionFactory2::ResolveCondition turn any date/time references
                                // (relative, such as "today", and absolute, such as "5/7/2009") into absolute date/time references
                                // (in the UTC time zone), and also simplifies the result in various ways.
                                // Note that pc is unchanged and could be resolved against additional dates/times.
                                // Code that targets only Windows 7 and later can take advantage of IConditionFactory2::ResolveCondition.
                                // Code that targets also earlier versions of Windows and Windows Search should use IQueryCondition::Resolve.
                                SYSTEMTIME st;
                                GetLocalTime(&st);
#if (_WIN32_WINNT >= _WIN32_WINNT_WIN7)
                                IConditionFactory2* pcf;
                                hr = pqs->QueryInterface(IID_PPV_ARGS(&pcf));
                                if (SUCCEEDED(hr))
                                {
                                    ICondition2* pcResolved;
                                    hr = pcf->ResolveCondition(pc, SQRO_DEFAULT, &st, IID_PPV_ARGS(&pcResolved));
                                    if (SUCCEEDED(hr))
                                    {
                                        hr = DisplayQuery(pcResolved, 0);
                                        pcResolved->Release();
                                    }
                                    pcf->Release();
                                }
#else // _WIN32_WINNT >= _WIN32_WINNT_WIN7
                                ICondition* pcResolved;
                                // With IConditionFactory::Resolve we must use SQRO_DONT_SPLIT_WORDS to behave like
                                // IConditionFactory2::ResolveCondition, we want to keep natural phrases unbroken.
                                hr = pqs->Resolve(pc, SQRO_DONT_SPLIT_WORDS, &st, &pcResolved);
                                if (SUCCEEDED(hr))
                                {
                                    hr = DisplayQuery(pcResolved, 0);
                                    pcResolved->Release();
                                }
#endif // _WIN32_WINNT >= _WIN32_WINNT_WIN7
                                pc->Release();
                            }
                            peu->Release();
                        }
                        pqs->Release();
                    }
                }
                pqp->Release();
            }
            pqpm->Release();
        }
        CoUninitialize();
    }
    return SUCCEEDED(hr) ? EXIT_SUCCESS : EXIT_FAILURE;
}
