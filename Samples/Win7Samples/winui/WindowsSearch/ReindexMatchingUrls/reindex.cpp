// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
// The application provides three ways to specify the files to reindex.
//
// Supported commands:
//   filetype    Reindexes all URLs that match the given file extensions
//   mimetype    Reindexes all URLs that match the given mimetypes
//   where       Reindexes all URLs that match a given WHERE clause
//
// The filetype test shows the usage of the ReindexMatchingUrls function. The other two options queries
// the indexer directly for the matching urls, then sends a message to add and delete the files. The boiler-plate
// code in CmdLineBase.h/.cpp is used to simplify the tasks of processing options, printing usage information etc.
// but this is not required to use the IShellLibrary API.
//
// Usage information for each command can be obtained via the '-?' option (-h and -help are also supported).

#include <stdio.h>
#include <windows.h>
#include <searchapi.h>
#include <iostream>
#include <stdlib.h>
#include <atlbase.h>
#include <cmdlinebase.h>
#include <atldbcli.h>
#include <strsafe.h>
#include <WinInet.h>

class CReindexWhereClauseCommand;
class CReindexMatchingMimeTypeCommand;
class CReindexMatchingWhereClauseCommand;
class CReindexMatchingFileTypeCommand;

#define CONSUME_NEXT_ARG(ppszArgs, cArgs) ((cArgs)--, ((ppszArgs)++)[0])

int wmain(int argc,  wchar_t *argv[])
{
    // initialize COM before doing anything else, since the IShellLibrary API depends on COM
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (SUCCEEDED(hr))
    {
        CMetaCommand::PFNCREATECOMMAND rgMainCmds[] =
        {
            CMetaCommand::Create<CReindexMatchingFileTypeCommand>,
            CMetaCommand::Create<CReindexMatchingMimeTypeCommand>,
            CMetaCommand::Create<CReindexMatchingWhereClauseCommand>,
        };
        {
            PCWSTR pszExeName = PathFindFileNameW(CONSUME_NEXT_ARG(argv, argc));
            CMetaCommand main(pszExeName, L"Reindexes the files that match the requirements specified. \nreindex <command> -? will give you the usage for each command", rgMainCmds, ARRAYSIZE(rgMainCmds));
            main.Execute(const_cast<PCWSTR *>(argv), argc);
        }
        CoUninitialize();
    }

    return 0;
}

// CReindexMatchingFileTypeCommand - Reindexes the URLs matching the given filetypes
//
// This class handles the filetype command. It uses the ReindexMatchingURLs command to reindex the
// filetypes specified.
class CReindexMatchingFileTypeCommand :
    public CCmdBase
{
public:
    CReindexMatchingFileTypeCommand(PCWSTR pszName = L"filetype", PCWSTR pszDescription = L"Reindexes all URLs that match the given file extensions") :
      CCmdBase(pszName, pszDescription, L"<.FILETYPE1> [<.FILETYPE2> ...] (EX. reindex filetype .foo .bar)")
    {
    }

    // Processes the filetypes that we are reindexing.
    HRESULT v_ProcessArguments(PCWSTR* ppszArgs, int cArgs)
    {
        HRESULT hr = cArgs ? S_OK : E_FAIL;
        if (FAILED(hr))
        {
            ParseError(L"Invalid number of arguments.");
        }
        else
        {
            // We are going to create a copy of ppszArgs to make an array of filetypes.
            m_ppszFileTypes = (PWSTR*) CoTaskMemAlloc(sizeof(PWSTR*) * cArgs);
            hr = m_ppszFileTypes ? S_OK : E_FAIL;
            if (SUCCEEDED(hr))
            {
                m_cArgs = cArgs;
                for (int i = 0; (i < m_cArgs) && SUCCEEDED(hr); i++)
                {
                    PCWSTR pszArg = CONSUME_NEXT_ARG(ppszArgs, cArgs);
                    hr = pszArg[0] == L'.' ? S_OK : E_FAIL;
                    if (FAILED(hr))
                    {
                        ParseError(L"'%s' is not a valid filetype.", pszArg);
                    }
                    else
                    {
                        hr = SHStrDup(pszArg, &(m_ppszFileTypes[i]));
                    }
                }
                if (FAILED(hr))
                {
                    FreeAll();
                }
            }
        }
        return hr;
    }

    HRESULT v_ExecuteCommand()
    {
        ISearchManager *pSearchManager;
        HRESULT hr = CoCreateInstance(__uuidof(CSearchManager), NULL, CLSCTX_ALL, IID_PPV_ARGS(&pSearchManager));
        if (SUCCEEDED(hr))
        {
            ISearchCatalogManager *pCatalogManager;
            hr = pSearchManager->GetCatalog(L"SystemIndex", &pCatalogManager);
            if (SUCCEEDED(hr))
            {
                for (int i = 0; (i < m_cArgs) && SUCCEEDED(hr); i++)
                {
                    WCHAR szPattern[MAX_PATH];
                    // We are creating a pattern that looks like *.FILETYPE and reindexing urls that match this pattern.
                    hr = StringCchPrintf(szPattern, ARRAYSIZE(szPattern), L"*%s", szPattern);
                    if (SUCCEEDED(hr))
                    {
                        hr = pCatalogManager->ReindexMatchingURLs(szPattern);
                    }
                }
                pCatalogManager->Release();
            }
            pSearchManager->Release();
        }
        if (SUCCEEDED(hr))
        {
            Output(L"Succeeded!");
        }
        FreeAll();
        return S_OK;
    }

private:
    PWSTR *m_ppszFileTypes;
    int m_cArgs;

    // This function frees the saved arguments in m_ppszFileTypes
    void FreeAll()
    {
        for (int i = 0; i < m_cArgs; i++)
        {
            CoTaskMemFree(m_ppszFileTypes[i]);
        }
        CoTaskMemFree(m_ppszFileTypes);
    }
};

// CReindexWhereClauseCommand - The base class that uses a WHERE clause to find the URLs that need
// reindexing.
//
// This is a base class for the mimetype and where subcommands. The argument processing is done in
// the subclasses during which a WHERE clause is created.
class CReindexWhereClauseCommand :
    public CCmdBase
{
public:
    CReindexWhereClauseCommand(PCWSTR pszName, PCWSTR pszDescription, PCWSTR pszUsage) :
      CCmdBase(pszName, pszDescription, pszUsage)
    {
    }

    virtual HRESULT v_ProcessArguments(PCWSTR* ppszArgs, int cArgs) = 0;

    // The m_szWhereClause is already set in v_ProcessArguments
    HRESULT v_ExecuteCommand()
    {
        UINT uiItemsReindexed;
        HRESULT hr = ReindexMatchingUrls(m_szWhereClause, &uiItemsReindexed);
        if (SUCCEEDED(hr))
        {
            Output(L"%d items reindexed", uiItemsReindexed);
        }
        return hr;
    }

protected:
    // v_ProcessArguments will fill this
    WCHAR m_szWhereClause[1024];

private:
    // class which is returned for each result in the query
    class CItem
    {
    private:
        WCHAR _szUrl[INTERNET_MAX_URL_LENGTH];

    public:
        PWSTR GetUrl() { return _szUrl; };

        BEGIN_COLUMN_MAP(CItem)
            COLUMN_ENTRY(1, _szUrl)
        END_COLUMN_MAP()
    };

    HRESULT GetItemsChangedSink(ISearchPersistentItemsChangedSink **ppspics)
    {
        *ppspics = NULL;

        ISearchManager *pSearchManager;
        HRESULT hr = CoCreateInstance(__uuidof(CSearchManager), NULL, CLSCTX_ALL, IID_PPV_ARGS(&pSearchManager));
        if (SUCCEEDED(hr))
        {
            // Get the catalog manager from the search manager
            ISearchCatalogManager *pCatalogManager;
            hr = pSearchManager->GetCatalog(L"SystemIndex", &pCatalogManager);
            if (SUCCEEDED(hr))
            {
                // get the notification change sink so we can send notifications
                hr = pCatalogManager->GetPersistentItemsChangedSink(ppspics);
                pCatalogManager->Release();
            }
            pSearchManager->Release();
        }
        return hr;
    }

    // This function will find the name of the original email URL if the given url is an argument
    // returns S_FALSE if the URL isn't an argument
    // returns S_OK if the URL is an attacment, pszOrigEmailUrl is filled with the original email URL
    HRESULT IfAtachmentmentGetOrigEmailURL(PCWSTR pszUrl, int cchUrl, PWSTR pszOrigEmailUrl, int cchOrigEmailUrl)
    {
        HRESULT hr = S_FALSE;
        // we are looking for '/at='
        int cchSearchString = 4;

        // start cchSearchString away from the end
        WCHAR *ptrString = ((WCHAR*)pszUrl) + cchUrl - cchSearchString;

        for (int i = cchUrl - cchSearchString; i >= 0; i--)
        {
            // email attachments end with /at=filename
            if (CompareString(LOCALE_SYSTEM_DEFAULT, NORM_IGNORECASE, ptrString, cchSearchString, L"/at=", cchSearchString) == CSTR_EQUAL)
            {
                // we found it, we should copy everything until the '/' to pszOrigEmailUrl
                hr = StringCchCopyN(pszOrigEmailUrl, cchOrigEmailUrl, pszUrl, i);
                if (SUCCEEDED(hr))
                {
                    hr = S_OK;
                }
                break;
            }
            ptrString--;
        }
        return hr;
    }

    HRESULT ReindexUrl(PCWSTR pszUrl, ISearchPersistentItemsChangedSink *pItemsNotify)
    {
        // generate a DELETE notification causing the URL to go away
        SEARCH_ITEM_PERSISTENT_CHANGE changes[2];
        changes[0].Change = SEARCH_CHANGE_DELETE;
        changes[0].Priority = SEARCH_NORMAL_PRIORITY;
        changes[0].URL = NULL;
        changes[0].OldURL = (PWSTR)pszUrl;

        changes[1].Change = SEARCH_CHANGE_ADD;
        changes[1].Priority = SEARCH_NORMAL_PRIORITY;
        changes[1].URL = (PWSTR)pszUrl;
        changes[1].OldURL = NULL;

        // tell the index that the items need to be updated with add/delete
        // change events (OnItemsChanged is responsible for freeing URLs)
        HRESULT hrChanges[ARRAYSIZE(changes)];
        HRESULT hr = pItemsNotify->OnItemsChanged(ARRAYSIZE(changes), changes, hrChanges);
        if (FAILED(hr))
        {
            Output(L"OnItemsChanged(%s) failed with 0x%x\n", pszUrl, hr);
            // dump the individual hr's
            Output(L"\tDELETE hr = 0x%x\n", hrChanges[0]);
            Output(L"\tNEW hr = 0x%x\n\n", hrChanges[1]);
        }
        return hr;
    }

    HRESULT ReindexMatchingUrls(PCWSTR pszWhere, UINT *puiItemsReindexed)
    {
        *puiItemsReindexed = 0;
        WCHAR szQuery[2048];
        HRESULT hr = StringCchPrintf(szQuery, ARRAYSIZE(szQuery), L"select System.ItemUrl from SystemIndex where %s", pszWhere);
        if (SUCCEEDED(hr))
        {
            ISearchPersistentItemsChangedSink *pItemsNotify;
            hr = GetItemsChangedSink(&pItemsNotify);
            if (SUCCEEDED(hr))
            {
                CDataSource cDataSource;
                hr = cDataSource.OpenFromInitializationString(L"provider=Search.CollatorDSO.1;EXTENDED PROPERTIES=\"Application=Windows\"");
                if (SUCCEEDED(hr))
                {
                    CSession cSession;
                    hr = cSession.Open(cDataSource);
                    if (SUCCEEDED(hr))
                    {
                        CCommand<CAccessor<CItem>, CRowset> cItems;
                        // execute the SQL, get back the recordset
                        hr = cItems.Open(cSession, szQuery);
                        if (SUCCEEDED(hr))
                        {
                            for (hr = cItems.MoveFirst(); hr == S_OK; hr = cItems.MoveNext())
                            {
                                // get the url from the current item
                                PWSTR pszUrl = cItems.GetUrl();
                                int cchUrl = lstrlen(pszUrl);
                                if (0 < cchUrl)
                                {
                                    // we only need to compare the first five characters
                                    int cchUrlCmp = cchUrl <= 5 ? cchUrl : 5;

                                    // mapi urls need to be massaged to deal with attachments
                                    if (CompareString(LOCALE_SYSTEM_DEFAULT, NORM_IGNORECASE, pszUrl, cchUrlCmp, L"mapi:", 5) == CSTR_EQUAL)
                                    {
                                        WCHAR pszOrigEmailUrl[INTERNET_MAX_URL_LENGTH];
                                        // If it is an attachment URL then it needs to be truncated to get the content of the email reindexed
                                        HRESULT hrReindex = IfAtachmentmentGetOrigEmailURL(pszUrl, cchUrl, pszOrigEmailUrl, ARRAYSIZE(pszOrigEmailUrl));

                                        // if the attachement was a url, reindex the content as well
                                        if (S_OK == hrReindex)
                                        {
                                            if (SUCCEEDED(ReindexUrl(pszOrigEmailUrl, pItemsNotify)))
                                            {
                                                Output(L"Reindexed - %s\n\n", pszOrigEmailUrl);
                                                (*puiItemsReindexed)++;
                                            }
                                            else
                                            {
                                                Output(L"Failed to reindex - %s\n\n", pszUrl);
                                            }
                                        }
                                    }
                                }
                                if (SUCCEEDED(ReindexUrl(pszUrl, pItemsNotify)))
                                {
                                    Output(L"Reindexing - %s\n\n", pszUrl);
                                    (*puiItemsReindexed)++;
                                }
                                else
                                {
                                    Output(L"Failed to reindex - %s\n\n", pszUrl);
                                }
                            }
                            // if we hit the end of the result set it's a normal S_OK
                            if (hr == DB_S_ENDOFROWSET)
                                hr = S_OK;
                        }
                        cSession.Close();
                    }
                    cDataSource.Close();
                }
                pItemsNotify->Release();
            }
        }
        return hr;
    }
};

// CReindexMatchingMimeTypeCommand - This is the class the processes the mimetype command.
//
// The v_processArguments builds up the where clause using the mimetypes specified. The base class (CReindexWhereClauseCommand)
// processes the where clause and reindexes the matching URLs.
class CReindexMatchingMimeTypeCommand :
    public CReindexWhereClauseCommand
{
public:
    CReindexMatchingMimeTypeCommand(PCWSTR pszName = L"mimetype", PCWSTR pszDescription = L"Reindexes all URLs that match the given mimetypes") :
      CReindexWhereClauseCommand(pszName, pszDescription, L"<MIMETYPE1> [<MIMETYPE2> ...] (EX. reindex mimetype image/jpeg)")
    {
    }

    HRESULT v_ProcessArguments(PCWSTR* ppszArgs, int cArgs)
    {
        // query against System.MimeType field
        HRESULT hr = cArgs ? S_OK : E_FAIL;
        if (FAILED(hr))
        {
            ParseError(L"Invalid number of arguments");
        }
        else
        {
            hr = StringCchCopy(m_szWhereClause, ARRAYSIZE(m_szWhereClause), L"contains(System.MimeType,'");
            if (SUCCEEDED(hr))
            {
                // add on clauses to the contains statement
                hr = StringCchCat(m_szWhereClause, ARRAYSIZE(m_szWhereClause), CONSUME_NEXT_ARG(ppszArgs, cArgs));
                if (SUCCEEDED(hr))
                {
                    while (cArgs)
                    {
                        hr = StringCchCat(m_szWhereClause, ARRAYSIZE(m_szWhereClause), L" OR ");
                        if (SUCCEEDED(hr))
                        {
                            hr = StringCchCat(m_szWhereClause, ARRAYSIZE(m_szWhereClause), CONSUME_NEXT_ARG(ppszArgs, cArgs));
                        }
                    }
                }
                if (SUCCEEDED(hr))
                {
                    hr = StringCchCat(m_szWhereClause, ARRAYSIZE(m_szWhereClause), L"')");
                }
            }
        }
        return hr;
    }
};

// CReindexMatchingWhereClauseCommand - This is the class the processes the where command.
//
// The v_processArguments builds up the where clause
class CReindexMatchingWhereClauseCommand :
    public CReindexWhereClauseCommand
{
public:
    CReindexMatchingWhereClauseCommand(PCWSTR pszName = L"where", PCWSTR pszDescription = L"Reindexes all URLs that match a given WHERE clause") :
      CReindexWhereClauseCommand(pszName, pszDescription, L"<WHERE_CLAUSE> (EX. reindex where System.ItemNameDisplay = 'test.txt')")
    {
    }

    HRESULT v_ProcessArguments(PCWSTR* ppszArgs, int cArgs)
    {
        // just copy the rest of ppszArgs into one string.
        HRESULT hr = cArgs ? S_OK : E_FAIL;
        if (FAILED(hr))
        {
            ParseError(L"Invalid number of arguments.");
        }
        else
        {
            StringCchCopy(m_szWhereClause, ARRAYSIZE(m_szWhereClause), CONSUME_NEXT_ARG(ppszArgs, cArgs));
            while (cArgs && SUCCEEDED(hr))
            {
                hr = StringCchCat(m_szWhereClause, ARRAYSIZE(m_szWhereClause), L" ");
                if (SUCCEEDED(hr))
                {
                    hr = StringCchCat(m_szWhereClause, ARRAYSIZE(m_szWhereClause), CONSUME_NEXT_ARG(ppszArgs, cArgs));
                }
            }
        }
        return hr;
    }

};
