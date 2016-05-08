// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved


// Sample application that demonstrates the use of the ATL OLEDB access to Windows Search
//
// Here are some interesting command lines to try
//
// wdsoledb /Select "System.ItemName, System.Size" /Sorting "System.Size ASC" /MaxResults 5 txt
//
// wdsoledb /Select "System.Size, System.ItemName" /Sorting "System.Size DESC" jpg
//
// wdsoledb /Select "System.ItemPathDisplay" wma
//
// wdsoledb /Select "System.ItemUrl" Content
//
// Also this sample demonstrates two other ways to retrieve results from Windows Search:
// binding to C++ class (via ATL) and via IPropertyStore. They are invoked with /2 and /3 command line switches.
//
// NOTE: this sample requires ATL that is installed as part of Visual Studio 2008

#include "WSOleDB.h"
#include "cmdline.h"

using namespace std;

// Helper functions
void Usage(PCWSTR pszProgName)
{
    wcout << L"Usage: " << endl << endl << pszProgName << endl;
    wcout << L"    [/ContentLocale <LCID value>]" << endl;
    wcout << L"    [/ContentProperties \"<prop1, prop2, ...>\"]" << endl;
    wcout << L"    [/KeywordLocale <LCID value>]" << endl;
    wcout << L"    [/MaxResults <number of results>]" << endl;
    wcout << L"    [/Select \"<col1, col2, ...>\"]" << endl;
    wcout << L"    [/Sorting <order value>]" << endl;
    wcout << L"    [/Syntax <None | AQS | NQS>]" << endl;
    wcout << L"    [/TermExpansion <None | PrefixAll | StemAll>]" << endl;
    wcout << L"    [/Where \"<where clause1, where clause2, ...>\"]" << endl;
    wcout << L"    [query expression]" << endl;
    wcout << L"OR  /2 for compile-time binding" << endl;
    wcout << L"OR  /3 for binding to property store. This sample will only work on Windows 7." << endl;
    wcout << L"     Note, /2 and /3 use hardcoded SQL strings" << endl;
}

// This helper function creates SQL string using query helper out of parameters
HRESULT GetSQLStringFromParams(LCID lcidContentLocaleParam,
                               PCWSTR pszContentPropertiesParam,
                               LCID lcidKeywordLocaleParam,
                               LONG nMaxResultsParam,
                               PCWSTR pszSelectColumnsParam,
                               PCWSTR pszSortingParam,
                               SEARCH_QUERY_SYNTAX sqsSyntaxParam,
                               SEARCH_TERM_EXPANSION steTermExpansionParam,
                               PCWSTR pszWhereRestrictionsParam,
                               PCWSTR pszExprParam,
                               PWSTR *ppszSQL)
{
    ISearchQueryHelper *pQueryHelper;

    // Create an instance of the search manager
    ISearchManager *pSearchManager;
    HRESULT hr = CoCreateInstance(__uuidof(CSearchManager), NULL, CLSCTX_LOCAL_SERVER, IID_PPV_ARGS(&pSearchManager));
    if (SUCCEEDED(hr))
    {
        // Get the catalog manager from the search manager
        ISearchCatalogManager *pSearchCatalogManager;
        hr = pSearchManager->GetCatalog(L"SystemIndex", &pSearchCatalogManager);
        if (SUCCEEDED(hr))
        {
            // Get the query helper from the catalog manager
            hr = pSearchCatalogManager->GetQueryHelper(&pQueryHelper);
            if (SUCCEEDED(hr))
            {
                hr = pQueryHelper->put_QueryContentLocale(lcidContentLocaleParam);
                if (SUCCEEDED(hr))
                {
                    hr = pQueryHelper->put_QueryContentProperties(pszContentPropertiesParam);
                }
                if (SUCCEEDED(hr))
                {
                    hr = pQueryHelper->put_QueryKeywordLocale(lcidKeywordLocaleParam);
                }
                if (SUCCEEDED(hr))
                {
                    hr = pQueryHelper->put_QueryMaxResults(nMaxResultsParam);
                }
                if (SUCCEEDED(hr))
                {
                    hr = pQueryHelper->put_QuerySelectColumns(pszSelectColumnsParam);
                }
                if (SUCCEEDED(hr))
                {
                    hr = pQueryHelper->put_QuerySorting(pszSortingParam);
                }
                if (SUCCEEDED(hr))
                {
                    hr = pQueryHelper->put_QuerySyntax(sqsSyntaxParam);
                }
                if (SUCCEEDED(hr))
                {
                    hr = pQueryHelper->put_QueryTermExpansion(steTermExpansionParam);
                }
                if (SUCCEEDED(hr))
                {
                    hr = pQueryHelper->put_QueryWhereRestrictions(pszWhereRestrictionsParam);
                }
                if (SUCCEEDED(hr))
                {
                    hr = pQueryHelper->GenerateSQLFromUserQuery(pszExprParam, ppszSQL);
                }
                pQueryHelper->Release();
            }
            pSearchCatalogManager->Release();
        }
        pSearchManager->Release();
    }
    return hr;
}

// This function retrieves parameters from command line arguments and converts them into query string
HRESULT GetSQLStringFromCommandLine(int argc, WCHAR *argv[], PWSTR *ppszSQL)
{
    *ppszSQL = NULL;

    HRESULT hr;
    if (argc < 2)
    {
        Usage(PathFindFileName(argv[0]));
        hr = E_INVALIDARG;
    }
    else
    {
        // these classes are used for parsing the command line parameters into
        // values used to initalize the query
        CContentLocaleParam ContentLocaleParam(GetUserDefaultLCID());
        CContentPropertiesParam ContentPropertiesParam(L"");
        CKeywordLocaleParam KeywordLocaleParam(GetUserDefaultLCID());
        CMaxResultsParam MaxResultsParam(-1);
        CSelectColumnsParam SelectColumnsParam(L"System.ItemName");
        CSortingParam SortingParam(L"");
        CSyntaxParam SyntaxParam(L"AQS");
        CTermExpansionParam TermExpansionParam(L"None");
        CWhereRestrictionsParam WhereRestrictionsParam(L"");
        CExprParam ExprParam(L"");

        CParamBase* QueryHelperCallbackParams[] =
        {
            &ContentLocaleParam,
            &ContentPropertiesParam,
            &KeywordLocaleParam,
            &MaxResultsParam,
            &SelectColumnsParam,
            &SortingParam,
            &SyntaxParam,
            &TermExpansionParam,
            &WhereRestrictionsParam,
            &ExprParam
        };

        hr = ParseParams(QueryHelperCallbackParams, ARRAYSIZE(QueryHelperCallbackParams), argc - 1, argv + 1);
        if (SUCCEEDED(hr))
        {
            // Initialize the query helper with the information from the cmd line
            hr = GetSQLStringFromParams(
                    ContentLocaleParam.Get(),
                    ContentPropertiesParam.Get(),
                    KeywordLocaleParam.Get(),
                    MaxResultsParam.Get(),
                    SelectColumnsParam.Get(),
                    SortingParam.Get(),
                    SyntaxParam.Get(),
                    TermExpansionParam.Get(),
                    WhereRestrictionsParam.Get(),
                    ExprParam.Get(),
                    ppszSQL);
        }
    }
    return hr;
}

void PrintDate(double date)
{
    SYSTEMTIME sysTime;
    BOOL ok = VariantTimeToSystemTime(date, &sysTime);
    if (ok)
    {
        SYSTEMTIME localTime;
        ok = SystemTimeToTzSpecificLocalTime(NULL, &sysTime, &localTime);
        if (ok)
        {
            WCHAR szBuffer[100];
            ok = GetDateFormat(LOCALE_USER_DEFAULT, 0, &localTime, NULL, szBuffer, ARRAYSIZE(szBuffer) / sizeof(WCHAR));
            if (ok)
            {
                wcout << szBuffer;
                ok = GetTimeFormat(LOCALE_USER_DEFAULT, 0, &localTime, NULL, szBuffer, ARRAYSIZE(szBuffer) / sizeof(WCHAR));
                if (ok)
                {
                    wcout << L" " << szBuffer;
                }
            }
        }
    }
    if (!ok)
    {
        wcout << L"Could not print date " << date;
    }
}

// This helper function can print some propvariants and handles BSTR vectors
void PrintPropVariant(REFPROPVARIANT variant)
{
    if (variant.vt == (VT_ARRAY | VT_BSTR) && variant.parray->cDims == 1)
    {
        BSTR *pBStr;
        HRESULT hr = SafeArrayAccessData(variant.parray, reinterpret_cast<void **>(&pBStr));
        if (SUCCEEDED(hr))
        {
            for (unsigned int i = 0; i < variant.parray->rgsabound[0].cElements; i++)
            {
                if (i == 0)
                {
                    wcout << L"[";
                }
                else
                {
                    wcout << L"; ";
                }
                wcout << pBStr[i];
            }
            wcout << L"]";
            SafeArrayUnaccessData(variant.parray);
        }
        else
        {
            wcout << L"Could not print vector";
        }
    }
    else
    {
        switch (variant.vt)
        {
        case VT_LPWSTR: wcout << variant.pwszVal; break;
        case VT_BSTR: wcout << variant.bstrVal; break;
        case VT_I1: wcout << variant.cVal; break;
        case VT_UI2: wcout << variant.uiVal; break;
        case VT_I2: wcout << variant.iVal; break;
        case VT_UI4: wcout << variant.ulVal; break;
        case VT_I4: wcout << variant.lVal; break;
        case VT_UI8: wcout << variant.uhVal.HighPart << variant.uhVal.LowPart; break;
        case VT_I8: wcout << variant.hVal.HighPart << variant.hVal.LowPart; break;
        case VT_DATE: PrintDate(variant.date); break;
        default:
            wcout << L"Unhandled variant type " << variant.vt;
            break;
        }
    }
}

// Start of the first sample which illustrates generic bindings.
// Column type and values are resolved dynamically after next result is fetched.
void BindToDynamicSample(int argc, WCHAR *argv[])
{
    PWSTR pszSQL;
    HRESULT hr = GetSQLStringFromCommandLine(argc, argv, &pszSQL);
    if (SUCCEEDED(hr))
    {
        wcout << L"Generated query: " << pszSQL << endl;
        CDataSource cDataSource;
        hr = cDataSource.OpenFromInitializationString(L"provider=Search.CollatorDSO.1;EXTENDED PROPERTIES=\"Application=Windows\"");
        if (SUCCEEDED(hr))
        {
            CSession cSession;
            hr = cSession.Open(cDataSource);
            if (SUCCEEDED(hr))
            {
                CCommand<CDynamicAccessor, CRowset> cCommand;
                hr = cCommand.Open(cSession, pszSQL);
                if (SUCCEEDED(hr))
                {
                    for (hr = cCommand.MoveFirst(); S_OK == hr; hr = cCommand.MoveNext())
                    {
                        for (DBORDINAL i = 1; i <= cCommand.GetColumnCount(); i++)
                        {
                            PCWSTR pszName = cCommand.GetColumnName(i);
                            wcout << pszName << L" : ";

                            DBSTATUS status;
                            cCommand.GetStatus(i, &status);
                            if (status == DBSTATUS_S_ISNULL)
                            {
                                wcout << L"NULL";
                            }
                            else if (status == DBSTATUS_S_OK || status == DBSTATUS_S_TRUNCATED)
                            {
                                DBTYPE type;
                                cCommand.GetColumnType(i, &type);
                                switch (type)
                                {
                                case DBTYPE_VARIANT:
                                    PrintPropVariant(*(static_cast<PROPVARIANT *>(cCommand.GetValue(i))));
                                    break;
                                case DBTYPE_WSTR:
                                    {
                                        DBLENGTH cbLen;
                                        cCommand.GetLength(i, &cbLen);
                                        WCHAR szBuffer[2048];
                                        StringCchCopyN(szBuffer, ARRAYSIZE(szBuffer), static_cast<WCHAR *>(cCommand.GetValue(i)), cbLen / sizeof(WCHAR));
                                        wcout << szBuffer;
                                    }
                                    break;
                                case DBTYPE_I1:
                                    wcout << *static_cast<UCHAR *>(cCommand.GetValue(i));
                                    break;
                                case DBTYPE_UI2:
                                    wcout << *static_cast<USHORT *>(cCommand.GetValue(i));
                                    break;
                                case DBTYPE_I2:
                                    wcout << *static_cast<SHORT *>(cCommand.GetValue(i));
                                    break;
                                case DBTYPE_UI4:
                                    wcout << *static_cast<DWORD *>(cCommand.GetValue(i));
                                    break;
                                case DBTYPE_I4:
                                    wcout << *static_cast<INT *>(cCommand.GetValue(i));
                                    break;
                                case DBTYPE_UI8:
                                    wcout << *static_cast<ULONGLONG *>(cCommand.GetValue(i));
                                    break;
                                case DBTYPE_I8:
                                    wcout << *static_cast<LONGLONG *>(cCommand.GetValue(i));
                                    break;
                                case DBTYPE_DATE:
                                    PrintDate(*static_cast<DATE *>(cCommand.GetValue(i)));
                                    break;
                                default:
                                    wcout << L"Unhandled database type " << type;
                                    break;
                                }
                            }
                            else
                            {
                                wcout << L"Error reading column " << i << L" (" << hr << L")";
                            }
                            wcout << endl;
                        }
                        wcout << endl;
                    }

                    if (DB_S_ENDOFROWSET == hr)
                    {
                        hr = S_FALSE;  // no rows
                    }
                    else
                    {
                        wcout << L"Query failed with error" << hr << endl;
                    }
                    cCommand.Close();
                }
                else
                {
                    wcout << L"Command (" << pszSQL << L") failed with error " << hr << endl;
                }
                cCommand.ReleaseCommand();
            }
        }
        CoTaskMemFree(pszSQL);
    }
}
// End of generic bindings sample

// Direct binding to C++ class. This sample uses hardcoded parameters for SQL string creation and binds to CMyAccessor class layout.
class CMyAccessor
{
public:
    WCHAR _szItemUrl[2048];
    __int64 _size;
    BEGIN_COLUMN_MAP(CMyAccessor)
        COLUMN_ENTRY(1, _szItemUrl)
        COLUMN_ENTRY(2, _size)
    END_COLUMN_MAP()
};

void BindToAccessorSample()
{
    PWSTR pszSQL;
    // This AQS will match all files named desktop.ini and return System.ItemPathDisplay and System.Size
    // which will be mapped to _szItemUrl and _size respectively
    HRESULT hr = GetSQLStringFromParams(1033, L"", 1033, -1, L"System.ItemPathDisplay, System.Size",
                                        L"", SEARCH_ADVANCED_QUERY_SYNTAX, SEARCH_TERM_NO_EXPANSION, L"",
                                        L"filename:desktop.ini", &pszSQL);
    if (SUCCEEDED(hr))
    {
        wcout << L"Generated query: " << pszSQL << endl;

        CDataSource cDataSource;
        hr = cDataSource.OpenFromInitializationString(L"provider=Search.CollatorDSO.1;EXTENDED PROPERTIES=\"Application=Windows\"");
        if (SUCCEEDED(hr))
        {
            CSession cSession;
            hr = cSession.Open(cDataSource);
            if (SUCCEEDED(hr))
            {
                // cCommand is derived from CMyAccessor which has binding information in column map
                // This allows ATL to put data directly into apropriate class members.
                CCommand<CAccessor<CMyAccessor>, CRowset> cCommand;
                hr = cCommand.Open(cSession, pszSQL);
                if (SUCCEEDED(hr))
                {
                    __int64 maxValue = 0;
                    __int64 minValue = ULONG_MAX;

                    for (hr = cCommand.MoveFirst(); S_OK == hr; hr = cCommand.MoveNext())
                    {
                        wcout << cCommand._szItemUrl << L": " << cCommand._size << L" bytes" << endl;

                        maxValue = max(maxValue, cCommand._size);
                        minValue = min(minValue, cCommand._size);
                    }

                    wcout << L"Max:" << maxValue << L"Min:" << minValue << endl;

                    cCommand.Close();
                }
                cCommand.ReleaseCommand();
            }
        }
        CoTaskMemFree(pszSQL);
    }
}
// End of direct binding sample.

// Last sample doesn't use ATL. It demonstates how to retrieve and access IPropertyStore objects from IRowset
// This sample requires Windows 7 to run successfully.

// Resolve provider class id, create a provider, initialize it, create session and then return a command
HRESULT GetWindowsSearchCommandObj(REFIID riid, void **ppv)
{
    CLSID clsidWindowsSearch;
    *ppv = NULL;
    HRESULT hr = CLSIDFromProgID(L"Search.CollatorDSO.1", &clsidWindowsSearch);
    if (SUCCEEDED(hr))
    {
        IDBInitialize *pdbi;
        hr = CoCreateInstance(clsidWindowsSearch, 0, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pdbi));
        if (SUCCEEDED(hr))
        {
            hr = pdbi->Initialize();
            if (SUCCEEDED(hr))
            {
                IDBCreateSession *pdbcs;
                hr = pdbi->QueryInterface(IID_PPV_ARGS(&pdbcs));
                if (SUCCEEDED(hr))
                {
                    IDBCreateCommand *pdbcc;
                    hr = pdbcs->CreateSession(0, IID_IDBCreateCommand, reinterpret_cast<IUnknown **>(&pdbcc));
                    if (SUCCEEDED(hr))
                    {
                        ICommand *pcmd;
                        hr = pdbcc->CreateCommand(0, IID_ICommand, reinterpret_cast<IUnknown **>(&pcmd));
                        if (SUCCEEDED(hr))
                        {
                            hr = pcmd->QueryInterface(riid, ppv);
                            pcmd->Release();
                        }
                        pdbcc->Release();
                    }
                    pdbcs->Release();
                }
            }
            pdbi->Release();
        }
    }
    return hr;
}

// Create ICommandText object and execute provided SQL text resulting in IRowset object
HRESULT ExecuteQuery(PCWSTR pwszSql, REFIID riid, void **ppv)
{
    ICommandText *pcmdText;
    *ppv = NULL;
    HRESULT hr = GetWindowsSearchCommandObj(IID_PPV_ARGS(&pcmdText));
    if (SUCCEEDED(hr))
    {
        hr = pcmdText->SetCommandText(DBGUID_DEFAULT, pwszSql);
        if (SUCCEEDED(hr))
        {
            DBROWCOUNT cRows;
            hr= pcmdText->Execute(NULL, riid, NULL, &cRows, reinterpret_cast<IUnknown **>(ppv));
        }
        pcmdText->Release();
    }
    return hr;
}

// Helper function used to strip off non-printable characters
void _StripCharacters(PWSTR pszText, PCWSTR pszRemove)
{
    PWSTR pszSource = pszText;
    PWSTR pszDest = pszSource;
    while (*pszSource)
    {
        // Skip copying characters found in pszRemove
        if (!StrChr(pszRemove, *pszSource))
        {
            *pszDest = *pszSource;
            pszDest++;
        }
        pszSource++;
    }
    *pszDest = 0;   // NULL terminate
}

// Helper function to retrieve the value from property store and print it
HRESULT PrintProperty(IPropertyStore *pps, REFPROPERTYKEY pkey)
{
    IPropertyDescription *ppd;
    HRESULT hr = PSGetPropertyDescription(pkey, IID_PPV_ARGS(&ppd));
    if (SUCCEEDED(hr))
    {
        WCHAR *pszName;
        hr = ppd->GetCanonicalName(&pszName);
        if (SUCCEEDED(hr))
        {
            wcout << L"Aggregated " << pszName << L" : ";
            PROPVARIANT propvar;
            hr = pps->GetValue(pkey, &propvar);
            if (SUCCEEDED(hr))
            {
                WCHAR *pszValue;
                hr = ppd->FormatForDisplay(propvar, PDFF_NOAUTOREADINGORDER, &pszValue);
                if (SUCCEEDED(hr))
                {
                    // Strip remaining order reading marks from the string:
                    //                            LRM   RLM   LRE   RLE   PDF   LRO   RLO
                    _StripCharacters(pszValue, L"\x200e\x200f\x202a\x202b\x202c\x202d\x202e");

                    wcout << pszValue << endl;
                    CoTaskMemFree(pszValue);
                }
                PropVariantClear(&propvar);
            }
            CoTaskMemFree(pszName);
        }
        ppd->Release();
    }
    return hr;
}

// Following query aggregates sizes and last modified dates of all indexed items under c:\users
// Grouping on System.Null will result in a single group containing all SELECT-ed results.
// Note that SELECT only fetches System.Search.EntryID which is the fastest property to retrieve.
// DATERANGE is a new aggregate, introduced in Windows 7.
WCHAR c_szAggregateSizeInUsers[] =
    L"GROUP ON System.Null AGGREGATE SUM(System.Size), DATERANGE(System.DateModified) OVER ("
        L"SELECT System.Search.EntryID from SystemIndex WHERE Scope='file:c:/users'"
    L")";

// Execute the query, fetch first row and get a property store out of it, then print relevant properties
void BindToPropertyStoreSample()
{
    wcout << L"Query text: " << c_szAggregateSizeInUsers << endl;
    IRowset *prs;
    HRESULT hr = ExecuteQuery(c_szAggregateSizeInUsers, IID_PPV_ARGS(&prs));
    if (SUCCEEDED(hr))
    {
        // The query above should only return a single result, thus only one HROW is enough
        HROW hRow;
        HROW *pgrHRows = &hRow;
        DBCOUNTITEM cRowsReturned;
        hr = prs->GetNextRows(0, 0, 1, &cRowsReturned, &pgrHRows);
        if (SUCCEEDED(hr))
        {
            if (cRowsReturned == 0)
            {
                wcout << L"Query returned zero results, check that the scope is indexed" << endl;
            }
            else
            {
                // Retrieve IPropertyStore and dump information from it
                IGetRow *pGetRow;
                hr = prs->QueryInterface(IID_PPV_ARGS(&pGetRow));
                if (SUCCEEDED(hr))
                {
                    IPropertyStore *pps;
                    hr = pGetRow->GetRowFromHROW(NULL, hRow, IID_IPropertyStore, reinterpret_cast<IUnknown **>(&pps));
                    if (SUCCEEDED(hr))
                    {
                        hr = PrintProperty(pps, PKEY_Size);
                        if (SUCCEEDED(hr))
                        {
                            hr = PrintProperty(pps, PKEY_DateModified);
                        }
                        pps->Release();
                    }
                    pGetRow->Release();
                }
            }
        }
        prs->Release();
    }
}
// End of IPropertyStore sample

void wmain(int argc, wchar_t *argv[])
{
    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE);
    if (SUCCEEDED(hr))
    {
        try
        {
            // Handle /2 and /3 as special cases, otherwise do command line parsing
            if (argc > 1 && argv[1][0] == L'/' && argv[1][1] == L'2')
            {
                BindToAccessorSample();
            }
            else if (argc > 1 && argv[1][0] == L'/' && argv[1][1] == L'3')
            {
                BindToPropertyStoreSample();
            }
            else
            {
                BindToDynamicSample(argc, argv);
            }
        }
        catch (CAtlException &e)
        {
            wcout << L"Caught ATL exception " << e.m_hr << endl;
            hr = e;
        }
        CoUninitialize();
    }
}
