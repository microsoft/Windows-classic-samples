// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "csmcmd.h"

// Forward declarations
int EnumRoots();
int EnumRules();
int AddRoots(PCWSTR pszURL);
int RemoveRoots(PCWSTR pszURL);
int AddRule(BOOL fDefault, BOOL fInclude, BOOL fOverride, PCWSTR pszURL);
int RemoveRule(BOOL fDefault, PCWSTR pszURL);
int Revert();
int Reset();
int Reindex();

// Global objects used for command line parsing.
CExclFlagParam g_IncludeParam(L"include", L"exclude");
CExclFlagParam g_DefaultParam(L"default", L"user");
CFlagParam g_OverrideParam(L"override_children");
CFlagParam g_EnumRootsParam(L"enumerate_roots");
CFlagParam g_EnumRulesParam(L"enumerate_rules");
CSetValueParam g_AddRootParam(L"add_root");
CSetValueParam g_RemoveRootParam(L"remove_root");
CSetValueParam g_AddRuleParam(L"add_rule");
CSetValueParam g_RemoveRuleParam(L"remove_rule");
CFlagParam g_RevertParam(L"revert");
CFlagParam g_Reset(L"reset");
CFlagParam g_Reindex(L"reindex");
CFlagParam g_HelpParam(L"help");
CFlagParam g_AltHelpParam(L"?");

// List of all supported command line options.
CParamBase* Params[] =
{
    &g_IncludeParam,
    &g_DefaultParam,
    &g_EnumRootsParam,
    &g_EnumRulesParam,
    &g_AddRootParam,
    &g_RemoveRootParam,
    &g_AddRuleParam,
    &g_RemoveRuleParam,
    &g_RevertParam,
    &g_Reset,
    &g_Reindex,
    &g_HelpParam,
    &g_AltHelpParam
};

// List of alternative options corresponding to CSM operations.
CParamBase* ExclusiveParams[] =
{
    &g_EnumRootsParam,
    &g_EnumRulesParam,
    &g_AddRootParam,
    &g_RemoveRootParam,
    &g_AddRuleParam,
    &g_RemoveRuleParam,
    &g_RevertParam,
    &g_Reset,
    &g_Reindex,
    &g_HelpParam,
    &g_AltHelpParam,
};

// Command line options help text
const PCWSTR rgParamsHelp[] =
{
    L"/enumerate_roots",
    L"/enumerate_rules",
    L"/add_root <new root path>",
    L"/remove_root <root path to remove>",
    L"/add_rule <rule URL> /[DEFAULT|USER] /[INCLUDE|EXCLUDE]",
    L"/remove_rule <rule URL> /[DEFAULT|USER]",
    L"/revert",
    L"/reset",
    L"/reindex",
    L"/help or /? "
};

int PrintHelp()
{
    wcout << L"NOTE: you must run this tool as an admin to perform functions" << endl;
    wcout << L"      that change the state of the index" << endl;

    wcout << L"List of availible options:" << endl;

    for (DWORD i = 0; i < ARRAYSIZE(rgParamsHelp); i++)
    {
        wcout << L"\t" << rgParamsHelp[i] << endl;
    }
    return ERROR_SUCCESS;
}

// CheckExcusiveParameters function validates that only one option
// from ExclusiveParams list is present in argument list
int CheckExcusiveParameters()
{
    int iRes = ERROR_SUCCESS;
    DWORD dwCount = 0;
    for (int i = 0; i < ARRAYSIZE(ExclusiveParams); i++)
    {
        if (ExclusiveParams[i]->Exists())
        {
            dwCount++;
        }
    }

    if (0 == dwCount)
    {
        wcout << L"Error: CSM operation parameter is expected!" << endl;
        iRes = ERROR_INVALID_PARAMETER;
    }
    else if (1 < dwCount)
    {
        wcout << L"Error: Duplicated CSM operation parameters!" << endl;
        iRes = ERROR_INVALID_PARAMETER;
    }

    return iRes;
}

int wmain(int argc, wchar_t *argv[])
{
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (SUCCEEDED(hr))
    {
        // Parsing command line
        int iRes = ParseParams(Params, ARRAYSIZE(Params), argc - 1, argv + 1);
        if (ERROR_SUCCESS == iRes)
        {
            // Check that only one CSM operation parameter was referred
            iRes = CheckExcusiveParameters();
            if (ERROR_SUCCESS == iRes)
            {
                // Default catalog name will be used if /CATALOG option doesn't specify otherwise
                if (g_HelpParam.Exists() || g_AltHelpParam.Exists())
                {
                    iRes = PrintHelp();
                }
                else if (g_EnumRootsParam.Exists())
                {
                    iRes = EnumRoots();
                }
                else if (g_EnumRulesParam.Exists())
                {
                    iRes = EnumRules();
                }
                else if (g_AddRootParam.Exists())
                {
                    iRes = AddRoots(g_AddRootParam.Get());
                }
                else if (g_RemoveRootParam.Exists())
                {
                    iRes = RemoveRoots(g_RemoveRootParam.Get());
                }
                else if (g_AddRuleParam.Exists())
                {
                    iRes = AddRule(g_DefaultParam.Exists() && g_DefaultParam.Get(),
                                           g_IncludeParam.Exists() && g_IncludeParam.Get(),
                                           g_OverrideParam.Exists(),
                                           g_AddRuleParam.Get());
                }
                else if (g_RemoveRuleParam.Exists())
                {
                    iRes = RemoveRule(g_DefaultParam.Exists() && g_DefaultParam.Get(),
                                              g_RemoveRuleParam.Get());
                }
                else if (g_RevertParam.Exists())
                {
                    iRes = Revert();
                }
                else if (g_Reset.Exists())
                {
                    iRes = Reset();
                }
                else if (g_Reindex.Exists())
                {
                    iRes = Reindex();
                }
                else
                {
                    assert(!"Required parameter is missing!");
                }
            }
            else
            {
                iRes = PrintHelp();
            }
        }

        CoUninitialize();
    }

    return 0;
}


int ReportHRESULTError(PCWSTR pszOpName, HRESULT hr)
{
    int iErr = 0;

    if (FAILED(hr))
    {
        if (HRESULT_FACILITY(hr) == FACILITY_WIN32)
        {
            iErr = HRESULT_CODE(hr);
            void *pMsgBuf;

            ::FormatMessageW(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                NULL,
                iErr,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (PWSTR) &pMsgBuf,
                0, NULL );

            wcout << endl << L"Error: " << pszOpName << L" failed with error " << iErr << L": " << (PWSTR)pMsgBuf << endl;

            LocalFree(pMsgBuf);
        }
        else
        {
            wcout << endl << L"Error: " << pszOpName << L" failed with error 0x" << setbase(16) << hr << endl;
            iErr = -1;
        }
    }

    return iErr;
}

HRESULT CreateCatalogManager(ISearchCatalogManager **ppSearchCatalogManager)
{
    *ppSearchCatalogManager = NULL;

    ISearchManager *pSearchManager;
    HRESULT hr = CoCreateInstance(CLSID_CSearchManager, NULL, CLSCTX_SERVER, IID_PPV_ARGS(&pSearchManager));
    if (SUCCEEDED(hr))
    {
        hr = pSearchManager->GetCatalog(L"SystemIndex", ppSearchCatalogManager);
        pSearchManager->Release();
    }
    return hr;
}

HRESULT CreateCrawlScopManager(ISearchCrawlScopeManager **ppSearchCrawlScopeManager)
{
    *ppSearchCrawlScopeManager = NULL;

    ISearchCatalogManager *pCatalogManager;
    HRESULT hr = CreateCatalogManager(&pCatalogManager);
    if (SUCCEEDED(hr))
    {
        // Crawl scope manager for that catalog
        hr = pCatalogManager->GetCrawlScopeManager(ppSearchCrawlScopeManager);
        pCatalogManager->Release();
    }
    return hr;
}

HRESULT DisplayRootInfo(ISearchRoot* pSearchRoot)
{
    PWSTR pszUrl = NULL;
    HRESULT hr = pSearchRoot->get_RootURL(&pszUrl);
    if (SUCCEEDED(hr))
    {
        wcout << L"\t" << pszUrl;
        BOOL fProvidesNotifications = FALSE;
        hr = pSearchRoot->get_ProvidesNotifications(&fProvidesNotifications);
        if (SUCCEEDED(hr) && fProvidesNotifications)
        {
            wcout << L" - Provides Notifications";
        }
        CoTaskMemFree(pszUrl);
    }

    wcout << endl;
    return hr;
}

int EnumRoots()
{
    // Crawl scope manager for that catalog
    ISearchCrawlScopeManager *pSearchCrawlScopeManager;
    HRESULT hr = CreateCrawlScopManager(&pSearchCrawlScopeManager);
    if (SUCCEEDED(hr))
    {
        // Search roots on that crawl scope
        IEnumSearchRoots *pSearchRoots;
        hr = pSearchCrawlScopeManager->EnumerateRoots(&pSearchRoots);
        ISearchRoot *pSearchRoot;
        if (SUCCEEDED(hr))
        {
            while (SUCCEEDED(hr) &&
                   S_OK == (hr = pSearchRoots->Next(1, &pSearchRoot, NULL)))
            {
                hr = DisplayRootInfo(pSearchRoot);
                pSearchRoot->Release();
            }
            pSearchRoots->Release();
        }
        pSearchCrawlScopeManager->Release();
    }

    return ReportHRESULTError(L"EnumRoots()", hr);
}

int AddRoots(PCWSTR pszURL)
{
    wcout << L"Adding new root " << pszURL << endl;

    // Crawl scope manager for that catalog
    ISearchCatalogManager *pCatalogManager;
    HRESULT hr = CreateCatalogManager(&pCatalogManager);
    if (SUCCEEDED(hr))
    {
        // Crawl scope manager for that catalog
        ISearchCrawlScopeManager *pSearchCrawlScopeManager;
        hr = pCatalogManager->GetCrawlScopeManager(&pSearchCrawlScopeManager);
        if (SUCCEEDED(hr))
        {
            ISearchRoot *pISearchRoot;
            hr = CoCreateInstance(CLSID_CSearchRoot, NULL, CLSCTX_ALL, IID_PPV_ARGS(&pISearchRoot));
            if (SUCCEEDED(hr))
            {
                hr = pISearchRoot->put_RootURL(pszURL);
                if (SUCCEEDED(hr))
                {
                    hr = pSearchCrawlScopeManager->AddRoot(pISearchRoot);
                    if (SUCCEEDED(hr))
                    {
                        hr = pSearchCrawlScopeManager->SaveAll();
                        if (SUCCEEDED(hr))
                        {
                            hr = pCatalogManager->ReindexSearchRoot(pszURL);
                            if (SUCCEEDED(hr))
                            {
                                wcout << "Reindexing was started for root " << pszURL << endl;
                            }
                        }
                    }
                }
                pISearchRoot->Release();
            }
            pSearchCrawlScopeManager->Release();
        }
        pCatalogManager->Release();
    }

    return ReportHRESULTError(L"AddRoots()", hr);
}

int RemoveRoots(PCWSTR pszURL)
{
    wcout << L"Removing root " << pszURL << endl;

    // Crawl scope manager for that catalog
    ISearchCrawlScopeManager *pSearchCrawlScopeManager;
    HRESULT hr = CreateCrawlScopManager(&pSearchCrawlScopeManager);
    if (SUCCEEDED(hr))
    {
        hr = pSearchCrawlScopeManager->RemoveRoot(pszURL);
        if (SUCCEEDED(hr))
        {
            hr = pSearchCrawlScopeManager->SaveAll();
        }
        pSearchCrawlScopeManager->Release();
    }

    return ReportHRESULTError(L"AddRoots()", hr);
}

HRESULT DisplayRule(ISearchScopeRule* pSearchScopeRule)
{
    PWSTR pszUrl;
    HRESULT hr = pSearchScopeRule->get_PatternOrURL(&pszUrl);
    if (SUCCEEDED(hr))
    {
        wcout << L"\t" << pszUrl;
        BOOL fDefault = FALSE;
        hr = pSearchScopeRule->get_IsDefault(&fDefault);
        if (SUCCEEDED(hr))
        {
            wcout << (fDefault ? L" " : L" NOT ") << L"DEFAULT";
            BOOL fIncluded = FALSE;
            hr = pSearchScopeRule->get_IsIncluded(&fIncluded);
            if (SUCCEEDED(hr))
            {
                wcout << (fIncluded ? L" INCLUDED" : L" EXCLUDED ");
            }
        }
        CoTaskMemFree(pszUrl);
    }
    wcout << endl;

    return hr;
}

int EnumRules()
{
    // Crawl scope manager for that catalog
    ISearchCrawlScopeManager *pSearchCrawlScopeManager;
    HRESULT hr = CreateCrawlScopManager(&pSearchCrawlScopeManager);
    if (SUCCEEDED(hr))
    {
        // Search roots on that crawl scope
        IEnumSearchScopeRules *pScopeRules;
        hr = pSearchCrawlScopeManager->EnumerateScopeRules(&pScopeRules);
        if (SUCCEEDED(hr))
        {
            ISearchScopeRule *pSearchScopeRule;
            while (SUCCEEDED(hr) &&
                   S_OK == (hr = pScopeRules->Next(1, &pSearchScopeRule, NULL)))
            {
                hr = DisplayRule(pSearchScopeRule);
                pSearchScopeRule->Release();
            }
            pScopeRules->Release();
        }
        pSearchCrawlScopeManager->Release();
    }

    return ReportHRESULTError(L"EnumRules()", hr);
}

int AddRule(BOOL fDefault, BOOL fInclude, BOOL fOverride, PCWSTR pszURL)
{
    wcout << L"Adding new " <<
         (fDefault ? L"default " : L"user ") <<
         (fInclude ? L"inclusion " : L"exclusion ") <<
         "rule " << pszURL <<
         ((!fDefault && fOverride) ? L"overriding cildren rules" : L"" ) << endl;

    // Crawl scope manager for that catalog
    ISearchCrawlScopeManager *pSearchCrawlScopeManager;
    HRESULT hr = CreateCrawlScopManager(&pSearchCrawlScopeManager);
    if (SUCCEEDED(hr))
    {
        if (fDefault)
        {
            hr = pSearchCrawlScopeManager->AddDefaultScopeRule(pszURL, fInclude, FF_INDEXCOMPLEXURLS);
        }
        else
        {
            hr = pSearchCrawlScopeManager->AddUserScopeRule(pszURL,
                                                             fInclude,
                                                             fOverride,
                                                             FF_INDEXCOMPLEXURLS);
        }
        if (SUCCEEDED(hr))
        {
            hr = pSearchCrawlScopeManager->SaveAll();
        }
        pSearchCrawlScopeManager->Release();
    }
    return ReportHRESULTError(L"AddRules()", hr);
}

int RemoveRule(BOOL fDefault, PCWSTR pszURL)
{
    wcout << L"Removing " << (fDefault ? L"default" : L"user") << " rule " << pszURL << endl;

    // Crawl scope manager for that catalog
    ISearchCrawlScopeManager *pSearchCrawlScopeManager;
    HRESULT hr = CreateCrawlScopManager(&pSearchCrawlScopeManager);
    if (SUCCEEDED(hr))
    {
        if (fDefault)
        {
            hr = pSearchCrawlScopeManager->RemoveDefaultScopeRule(pszURL);
        }
        else
        {
            hr = pSearchCrawlScopeManager->RemoveScopeRule(pszURL);
        }
        if (SUCCEEDED(hr))
        {
            hr = pSearchCrawlScopeManager->SaveAll();
        }
        pSearchCrawlScopeManager->Release();
    }
    return ReportHRESULTError(L"AddRules()", hr);
}

int Revert()
{
    wcout << L"Reverting catalog to its default state." << endl;

    // Crawl scope manager for that catalog
    ISearchCrawlScopeManager *pSearchCrawlScopeManager;
    HRESULT hr = CreateCrawlScopManager(&pSearchCrawlScopeManager);
    if (SUCCEEDED(hr))
    {
        hr = pSearchCrawlScopeManager->RevertToDefaultScopes();
        if (SUCCEEDED(hr))
        {
            hr = pSearchCrawlScopeManager->SaveAll();
        }
        pSearchCrawlScopeManager->Release();
    }

    return ReportHRESULTError(L"Revert()", hr);
}

int Reset()
{
    wcout << L"Resetting catalog." << endl;

    ISearchCatalogManager *pCatalogManager;
    HRESULT hr = CreateCatalogManager(&pCatalogManager);
    if (SUCCEEDED(hr))
    {
        hr = pCatalogManager->Reset();
        pCatalogManager->Release();
    }
    return ReportHRESULTError(L"Reset()", hr);
}

int Reindex()
{
    wcout << L"Reindexing catalog." << endl;

    ISearchCatalogManager *pCatalogManager;
    HRESULT hr = CreateCatalogManager(&pCatalogManager);
    if (SUCCEEDED(hr))
    {
        hr = pCatalogManager->Reindex();
        pCatalogManager->Release();
    }
    return ReportHRESULTError(L"Reindex()", hr);
}