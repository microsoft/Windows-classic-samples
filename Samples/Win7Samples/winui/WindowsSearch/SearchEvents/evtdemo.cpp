// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
// This sample requires ATL (installed as part of Visual Studio)

#include <crtdbg.h>
#include <msdasc.h>
#include <atlbase.h>
#include <atlcom.h>
#include <searchapi.h>
#include <propkey.h>
#include <ntquery.h>
#include <strsafe.h>
#include <wininet.h>

CComModule Module;
CComModule &_Module = Module;

template <class T>  inline HRESULT CreateComObject(T ** ppOutPtr)
{
    *ppOutPtr = NULL;
    CComObject<T> * pObj = NULL;
    HRESULT hr = CComObject<T>::CreateInstance( &pObj );
    if (SUCCEEDED(hr))
    {
        *ppOutPtr = pObj;
        pObj->GetUnknown()->AddRef();
    }
    return hr;
}


//*****************************************************************************
// Open a database session...

HRESULT OpenSession(IDBCreateCommand **ppCreateCommand)
{
    *ppCreateCommand = NULL;

    CComPtr<IDataInitialize> spDataInit;
    CComPtr<IUnknown> spUnknownDBInitialize;
    CComPtr<IDBInitialize> spDBInitialize;
    CComPtr<IDBCreateSession> spDBCreateSession;

    HRESULT hr = CoCreateInstance(CLSID_MSDAINITIALIZE, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&spDataInit));
    if (SUCCEEDED(hr))
    {
        hr = spDataInit->GetDataSource( NULL, CLSCTX_INPROC_SERVER, L"provider=Search.CollatorDSO.1", __uuidof(IDBInitialize), &spUnknownDBInitialize);
    }
    if (SUCCEEDED(hr))
    {
        hr = spUnknownDBInitialize->QueryInterface(&spDBInitialize);
    }
    if (SUCCEEDED(hr))
    {
        hr = spDBInitialize->Initialize();
    }
    if (SUCCEEDED(hr))
    {
        hr = spDBInitialize->QueryInterface(&spDBCreateSession);
    }
    if (SUCCEEDED(hr))
    {
        CComPtr<IUnknown> spUnknownCreateCommand;
        hr = spDBCreateSession->CreateSession(0, __uuidof(IDBCreateCommand), &spUnknownCreateCommand);
        if (SUCCEEDED(hr))
        {
            hr = spUnknownCreateCommand->QueryInterface(ppCreateCommand);
        }
    }

    return hr;
}

//*****************************************************************************
// Run a query against the database, optionally enabling eventing...

HRESULT ExecuteQuery(IDBCreateCommand * pDBCreateCommand, PCWSTR pwszQuerySQL, bool fEnableEventing, REFIID riid, void **ppv)
{
    CComPtr<IUnknown> spUnknownCommand;
    CComPtr<ICommandProperties> spCommandProperties;
    CComPtr<ICommandText> spCommandText;

    HRESULT hr = pDBCreateCommand->CreateCommand(0, __uuidof(ICommand), &spUnknownCommand );
    if (SUCCEEDED(hr))
    {
        hr = spUnknownCommand->QueryInterface(&spCommandProperties);
    }
    if (SUCCEEDED(hr))
    {
        DBPROP    rgProps[2]        = {};
        DBPROPSET propSet           = {};

        rgProps[propSet.cProperties].dwPropertyID       = DBPROP_USEEXTENDEDDBTYPES;
        rgProps[propSet.cProperties].dwOptions          = DBPROPOPTIONS_OPTIONAL;
        rgProps[propSet.cProperties].vValue.vt          = VT_BOOL;
        rgProps[propSet.cProperties].vValue.boolVal     = VARIANT_TRUE;
        propSet.cProperties++;

        if (fEnableEventing)
        {
            rgProps[propSet.cProperties].dwPropertyID   = DBPROP_ENABLEROWSETEVENTS;
            rgProps[propSet.cProperties].dwOptions      = DBPROPOPTIONS_OPTIONAL;
            rgProps[propSet.cProperties].vValue.vt      = VT_BOOL;
            rgProps[propSet.cProperties].vValue.boolVal = VARIANT_TRUE;
            propSet.cProperties++;
        }

        propSet.rgProperties            = rgProps;
        static const GUID guidQueryExt  = DBPROPSET_QUERYEXT;
        propSet.guidPropertySet         = guidQueryExt;

        hr = spCommandProperties->SetProperties(1, &propSet);
    }
    if (SUCCEEDED(hr))
    {
        hr = spUnknownCommand->QueryInterface(&spCommandText);
    }
    if (SUCCEEDED(hr))
    {
        hr = spCommandText->SetCommandText(DBGUID_DEFAULT, pwszQuerySQL);
    }
    if (SUCCEEDED(hr))
    {
        DBROWCOUNT cRows;
        hr = spCommandText->Execute(NULL, riid, NULL, &cRows, reinterpret_cast<IUnknown **>(ppv));
    }

    return hr;
}


//*****************************************************************************
// Retrieves the URL from a given workid

HRESULT RetrieveURL(IDBCreateCommand *pDBCreateCommand, REFPROPVARIANT itemID, PWSTR pwszURL, int cchURL)
{
    WCHAR wszQuery[512];
    CComPtr<IRowset> spRowset;

    HRESULT hr = (itemID.vt == VT_UI4) ? S_OK : E_INVALIDARG;
    if (SUCCEEDED(hr))
    {
        hr = StringCchPrintf( wszQuery, ARRAYSIZE(wszQuery), L"SELECT TOP 1 System.ItemUrl FROM SystemIndex WHERE workid=%u", itemID.ulVal );
    }
    if (SUCCEEDED(hr))
    {
        hr = ExecuteQuery( pDBCreateCommand, wszQuery, false, IID_PPV_ARGS(&spRowset) );
    }
    if (SUCCEEDED(hr))
    {
        CComPtr<IGetRow> spGetRow;
        DBCOUNTITEM ciRowsRetrieved = 0;
        HROW hRow                   = NULL;
        HROW * phRow                = &hRow;
        CComPtr<IPropertyStore> spPropertyStore;

        hr = spRowset->GetNextRows( DB_NULL_HCHAPTER, 0, 1, &ciRowsRetrieved, &phRow );
        if (SUCCEEDED(hr))
        {
            hr = spRowset->QueryInterface(&spGetRow);
            if (SUCCEEDED(hr))
            {
                CComPtr<IUnknown> spUnknownPropertyStore;
                hr = spGetRow->GetRowFromHROW( NULL, hRow, __uuidof(IPropertyStore), &spUnknownPropertyStore );
                if (SUCCEEDED(hr))
                {
                    hr = spUnknownPropertyStore->QueryInterface(&spPropertyStore);
                }
            }
            if (SUCCEEDED(hr))
            {
                PROPVARIANT var = {};
                hr = spPropertyStore->GetValue( PKEY_ItemUrl, &var );
                if (SUCCEEDED(hr))
                {
                    if (var.vt == VT_LPWSTR)
                    {
                        hr = StringCchCopy( pwszURL, cchURL, var.pwszVal );
                    }
                    else
                    {
                        hr = E_INVALIDARG;
                    }
                    ::PropVariantClear( &var );
                }
            }

            spRowset->ReleaseRows( ciRowsRetrieved, phRow, NULL, NULL, NULL );
        }
    }

    return hr;
}

//*****************************************************************************

PCWSTR ItemStateToString( ROWSETEVENT_ITEMSTATE itemState )
{
    switch (itemState)
    {
    case ROWSETEVENT_ITEMSTATE_NOTINROWSET:
        return L"NotInRowset";
    case ROWSETEVENT_ITEMSTATE_INROWSET:
        return L"InRowset";
    case ROWSETEVENT_ITEMSTATE_UNKNOWN:
        return L"Unknown";
    }
    return L"";
}

//*****************************************************************************

PCWSTR PriorityLevelToString( PRIORITY_LEVEL priority )
{
    switch (priority)
    {
    case PRIORITY_LEVEL_FOREGROUND:
        return L"Foreground";
    case PRIORITY_LEVEL_HIGH:
        return L"High";
    case PRIORITY_LEVEL_LOW:
        return L"Low";
    case PRIORITY_LEVEL_DEFAULT:
        return L"Default";
    }
    return L"";
}



//*****************************************************************************
// An event listener class that listens to indexer events and logs them
// as they arrive...

class ATL_NO_VTABLE CRowsetEventListener :
    public CComObjectRootEx<CComMultiThreadModelNoCS>,
    public IRowsetEvents
{
public:

    BEGIN_COM_MAP(CRowsetEventListener)
        COM_INTERFACE_ENTRY(IRowsetEvents)
    END_COM_MAP()

    // IRowsetEvents

    STDMETHOD(OnNewItem)(REFPROPVARIANT itemID, ROWSETEVENT_ITEMSTATE newItemState);
    STDMETHOD(OnChangedItem)(REFPROPVARIANT itemID, ROWSETEVENT_ITEMSTATE rowsetItemState, ROWSETEVENT_ITEMSTATE changedItemState);
    STDMETHOD(OnDeletedItem)(REFPROPVARIANT itemID, ROWSETEVENT_ITEMSTATE deletedItemState);
    STDMETHOD(OnRowsetEvent)(ROWSETEVENT_TYPE eventType, REFPROPVARIANT eventData);

    CComPtr<IDBCreateCommand> spDBCreateCommand;

private:

    HRESULT PrintURL( REFPROPVARIANT itemID );
};


HRESULT CRowsetEventListener::PrintURL( REFPROPVARIANT itemID )
{
    WCHAR wszURL[INTERNET_MAX_URL_LENGTH];

    HRESULT hr = (itemID.vt == VT_UI4) ? S_OK : E_INVALIDARG;
    if (SUCCEEDED(hr))
    {
        hr = RetrieveURL( spDBCreateCommand, itemID, wszURL, ARRAYSIZE(wszURL) );
        if (FAILED(hr))
        {
            // It's possible that for some items we won't be able to retrieve the URL.
            // This can happen when our application doesn't have sufficient priveledges to read the URL
            // or if the URL has been deleted from the system.

            hr = StringCchCopy( wszURL, ARRAYSIZE(wszURL), L"URL-Lookup-NotFound" );
        }
    }
    if (SUCCEEDED(hr))
    {
        printf( "workid: %u;  URL: %S\n", itemID.ulVal, wszURL );
    }
    return hr;
}

STDMETHODIMP CRowsetEventListener::OnNewItem(REFPROPVARIANT itemID, ROWSETEVENT_ITEMSTATE newItemState)
{
    // This event is received when the indexer has completed indexing of a NEW item that falls within the
    // scope of your query.  If your query is for C:\users, then only newly indexed items within C:\users
    // will be given.

    printf( "OnNewItem( newItemState: %S )\n\t\t", ItemStateToString(newItemState) );
    return PrintURL( itemID );
}

STDMETHODIMP CRowsetEventListener::OnChangedItem(REFPROPVARIANT itemID, ROWSETEVENT_ITEMSTATE rowsetItemState, ROWSETEVENT_ITEMSTATE changedItemState)
{
    // This event is received when the indexer has completed re-indexing of an item that was already in
    // the index that falls within the scope of your query.  The rowsetItemState parameter indicates the
    // state of the item regarding your query when it was initially executed.  The changedItemState
    // represents the state of the item following reindexing.

    printf( "OnChangedItem( rowsetItemState: %S changedItemState: %S )\n\t\t", ItemStateToString(rowsetItemState), ItemStateToString(changedItemState) );
    return PrintURL( itemID );
}

STDMETHODIMP CRowsetEventListener::OnDeletedItem(REFPROPVARIANT itemID, ROWSETEVENT_ITEMSTATE deletedItemState)
{
    // This event is received when the indexer has completed deletion of an item that was already in
    // the index that falls within the scope of your query.  Note that the item may not have been in your
    // original query even if the original query was solely scope-based if the item was added following
    // your query.

    printf( "OnDeletedItem( deletedItemState: %S )\n\t\t", ItemStateToString(deletedItemState) );
    return PrintURL( itemID );
}

STDMETHODIMP CRowsetEventListener::OnRowsetEvent(ROWSETEVENT_TYPE eventType, REFPROPVARIANT eventData)
{
    switch (eventType)
    {
    case ROWSETEVENT_TYPE_DATAEXPIRED:
        // This event signals that your rowset is no longer valid, so further calls made to the rowset
        // will fail.  This can happen if the client (your application) loses its connection to the
        // indexer.  Indexer restarts or network problems with remote queries could cause this.

        printf( "OnRowsetEvent( ROWSETEVENT_TYPE_DATAEXPIRED )\n\t\tData backing the rowset has expired.  Requerying is needed.\n" );
        break;

    case ROWSETEVENT_TYPE_FOREGROUNDLOST:
        // This event signals that a previous request for PRIORITY_LEVEL_FOREGROUND made on this rowset
        // has been downgraded to PRIORITY_LEVEL_HIGH.  The most likely cause of this is another query
        // having requested foreground prioritization.  The indexer treats prioritization requests as a
        // stack where only the top request on the stack may have foreground priority.

        printf( "OnRowsetEvent( ROWSETEVENT_TYPE_FOREGROUNDLOST )\n\t\tForeground priority has been downgraded to high priority.\n" );
        break;

    case ROWSETEVENT_TYPE_SCOPESTATISTICS:
        // This informational event is sent periodically when there has been a prioritization request for
        // any value other than PRIORITY_LEVEL_DEFAULT.  This event allows tracking indexing progress in
        // response to a prioritization reqeust.

        printf( "OnRowsetEvent( ROWSETEVENT_TYPE_SCOPESTATISTICS )\n\t\tStatistics( indexedDocs:%u docsToAddCount:%u docsToReindexCount: %u )\n", eventData.caul.pElems[0], eventData.caul.pElems[1], eventData.caul.pElems[2] );
        break;
    }
    return S_OK;
}


//*****************************************************************************
// Watches events on the given query with the given priority for a period of
// time.  If dwTimeout == 0, then it will monitor until all items are indexed
// within the query.  Otherwise, it monitors for dwTimeout MS.

void WatchEvents( PCWSTR pwszQuerySQL, PRIORITY_LEVEL priority, DWORD dwTimeout )
{
    CComPtr<IDBCreateCommand> spDBCreateCommand;
    CComPtr<IRowset> spRowset;
    CComPtr<IRowsetPrioritization> spRowsetPrioritization;
    CComPtr<CRowsetEventListener> spListener;

    HRESULT hr = OpenSession( &spDBCreateCommand );
    if (SUCCEEDED(hr))
    {
        hr = ExecuteQuery( spDBCreateCommand, pwszQuerySQL, true, IID_PPV_ARGS(&spRowset) );
    }
    if (SUCCEEDED(hr))
    {
        hr = spRowset->QueryInterface(&spRowsetPrioritization);
    }
    if (SUCCEEDED(hr))
    {
        hr = CreateComObject( &spListener );
    }
    if (SUCCEEDED(hr))
    {
        spListener->spDBCreateCommand = spDBCreateCommand;

        DWORD dwAdviseID;
        hr = ConnectToConnectionPoint( spListener->GetUnknown(), __uuidof(IRowsetEvents), TRUE, spRowset, &dwAdviseID, NULL );
        if (SUCCEEDED(hr))
        {
            DWORD indexedDocumentCount  = 0;
            DWORD oustandingAddCount    = 0;
            DWORD oustandingModifyCount = 0;

            spRowsetPrioritization->GetScopeStatistics( &indexedDocumentCount, &oustandingAddCount, &oustandingModifyCount );

            printf( "Prioritization and Eventing Demo\n\n" );
            printf( "Query:               %S\n\n", pwszQuerySQL );
            printf( "Indexed Docs:        %u\n", indexedDocumentCount );
            printf( "Oustanding Adds:     %u\n", oustandingAddCount );
            printf( "Oustanding Modifies: %u\n\n", oustandingModifyCount );
            printf( "Setting Priority:    %S\n\n", PriorityLevelToString(priority) );
            printf( "Now monitoring events for this query...\n\n" );

            spRowsetPrioritization->SetScopePriority( priority, 1000 );

            if (dwTimeout == 0)
            {
                while (SUCCEEDED(hr) && ((oustandingAddCount > 0) || (oustandingModifyCount > 0)))
                {
                    Sleep( 1000 );
                    hr = spRowsetPrioritization->GetScopeStatistics( &indexedDocumentCount, &oustandingAddCount, &oustandingModifyCount );
                }
            }
            else
            {
                Sleep( dwTimeout );
            }

            ConnectToConnectionPoint( spListener->GetUnknown(), __uuidof(IRowsetEvents), FALSE, spRowset, &dwAdviseID, NULL );
        }
    }

    if (FAILED(hr))
    {
        printf( "Failure: %08X\n", hr );
    }
}


int wmain(int argc, wchar_t *argv[])
{
    if ( (argc <= 1) ||
         (argc == 2 && ((0 == wcscmp(argv[1], L"-?")) || (0 == wcscmp(argv[1], L"/?")) )) )
    {
        printf( "Allows monitoring and prioritization of indexer URLs.\n\n" );
        printf( "Eventing [drive:][path] [/p[:]priority] [/t[:]duration]\n\n" );
        printf( "  [drive:][path]\n" );
        printf( "             Specifies drive and directory of location to watch\n" );
        printf( "  /p         Prioritizes indexing at the given speed\n" );
        printf( "  priority     F  Foreground      H High\n" );
        printf( "               L  Low             D Default\n" );
        printf( "  /t         Specifies how long in MS to monitor query\n" );
        printf( "  duration     0     Until all content is indexed\n" );
        printf( "               NNNN  Monitor for NNNN milliseconds\n\n" );
    }
    else if (SUCCEEDED(CoInitializeEx(0, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE)))
    {
        HRESULT hr              = S_OK;
        bool fHandled           = true;
        DWORD dwTimeout         = (300 * 1000);                 // default 5 mins
        PRIORITY_LEVEL priority = PRIORITY_LEVEL_DEFAULT;
        WCHAR wszURL[INTERNET_MAX_URL_LENGTH];
        *wszURL = L'\0';

        for (int nArg = 1; ((nArg < argc) && SUCCEEDED(hr)); nArg++)
        {
            if (argv[nArg][0] == '/')
            {
                if (towlower(argv[nArg][1]) == 'p')
                {
                    PCWSTR pwsz = argv[nArg] + 2;
                    if (*pwsz == L':')
                    {
                        pwsz++;
                    }
                    switch (towlower(*pwsz))
                    {
                    case 'f':
                        priority = PRIORITY_LEVEL_FOREGROUND;
                        break;
                    case 'h':
                        priority = PRIORITY_LEVEL_HIGH;
                        break;
                    case 'l':
                        priority = PRIORITY_LEVEL_LOW;
                        break;
                    case 'd':
                        priority = PRIORITY_LEVEL_DEFAULT;
                        break;
                    default:
                        hr = E_INVALIDARG;
                    }
                }
                else if (towlower(argv[nArg][1]) == 't')
                {
                    PCWSTR pwsz = argv[nArg] + 2;
                    if (*pwsz == L':')
                    {
                        pwsz++;
                    }
                    dwTimeout   = static_cast<DWORD>(_wtol(pwsz));
                    fHandled    = true;
                }
                else
                {
                    hr = E_INVALIDARG;
                }
            }
            else
            {
                hr = StringCchCopy( wszURL, ARRAYSIZE(wszURL), argv[nArg] );
            }
        }

        if (SUCCEEDED(hr))
        {
            WCHAR wszQuerySQL[INTERNET_MAX_URL_LENGTH];
            if (*wszURL)
            {
                hr = StringCchPrintf( wszQuerySQL, ARRAYSIZE(wszQuerySQL), L"SELECT workid FROM SystemIndex WHERE SCOPE='%s'", wszURL );
            }
            else
            {
                hr = StringCchCopy( wszQuerySQL, ARRAYSIZE(wszQuerySQL), L"SELECT workid FROM SystemIndex" );
            }
            if (SUCCEEDED(hr))
            {
                WatchEvents( wszQuerySQL, priority, dwTimeout );
            }
        }

        CoUninitialize();
    }
    return 0;
}


