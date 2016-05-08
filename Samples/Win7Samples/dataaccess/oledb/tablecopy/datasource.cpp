//-----------------------------------------------------------------------------
// Microsoft OLE DB TABLECOPY Sample
// Copyright (C) 1995-2000 Microsoft Corporation
//
// @doc
//
// @module DATASOURCE.CPP
//
//-----------------------------------------------------------------------------

/////////////////////////////////////////////////////////////////
// Includes
//
/////////////////////////////////////////////////////////////////
#include "WinMain.h"
#include "Common.h"
#include "DataSource.h"
#include "msdaguid.h"	//CLSID_OLEDB_ENUMERATOR


/////////////////////////////////////////////////////////////////
// CDataSource::CDataSource
//
/////////////////////////////////////////////////////////////////
CDataSource::CDataSource()
{
    //OLEDB Interfaces
    m_pIDBInitialize		= NULL;	//DataSource
    m_pIOpenRowset			= NULL;	//Session
    m_pIDBSchemaRowset		= NULL;	//Session
    m_pITableDefinition		= NULL;	//Session
    m_pIIndexDefinition		= NULL;	//Session
    m_pICommandText			= NULL;	//Command

    //ProviderInfo
    m_pwszProviderName		= NULL;	//Pointer into m_rgProviderInfo.wszName
    m_pwszProviderParseName	= NULL;	//Pointer into m_rgProviderInfo.wszParseName
    m_pwszProviderFileName	= NULL; //DBPROP_PROVIDERNAME
    m_pwszProviderVer		= NULL; //DBPROP_PROVIDERVER
    m_pwszProviderOLEDBVer	= NULL; //DBPROP_PROVIDEROLEDBVER

    //Enumerator ProvierInfo
    m_cProviderInfo			= 0;	
    m_rgProviderInfo		= NULL;
    m_pIParseDisplayName	= NULL; //Enumerator

    //Properties
    m_fReadOnly				= TRUE;	//DBPROP_DATASOURCEREADONLY
    m_fPrimaryKeysSupported = FALSE;//DBPROP_SQLSUPPORT
    m_fMultipleParamSets    = FALSE;//DBPROP_MULTIPLEPARAMSETS
    m_fIRowsetChange		= FALSE;//DBPROP_IRowsetChange
    m_fIRowsetUpdate		= FALSE;//DBPROP_IRowsetUpdate
    m_dwStorageObjects		= 0;	//DBPROP_STRUCTUREDSTORAGE

    m_pwszCatalog			= NULL; //DBPROP_CURRENTCATALOG
    m_pwszCatalogTerm		= NULL; //DBPROP_CATALOGTERM
    m_pwszCatalogLocation	= NULL; //DBPROP_CATALOGLOCATION
    m_pwszSchemaTerm		= NULL; //DBPROP_SCHEMATERM
    m_pwszTableTerm			= NULL; //DBPROP_TABLETERM

    //DataSource Info
    m_ulActiveSessions		= 0;	//DBPROP_ACTIVESESSIONS
    m_pwszDataSource		= NULL;	//DBPROP_DATASOURCENAME
    m_pwszDBMS				= NULL; //DBPROP_DBMSNAME
    m_pwszDBMSVer			= NULL; //DBPROP_DBMSVER

    m_fConnected = FALSE;
}

/////////////////////////////////////////////////////////////////
// CDataSource::~CDataSource
//
/////////////////////////////////////////////////////////////////
CDataSource::~CDataSource()
{
    Disconnect();

    //Enumerator ProviderInfo
    m_cProviderInfo			= 0;	
    SAFE_FREE(m_rgProviderInfo);
    SAFE_RELEASE(m_pIParseDisplayName);
}


/////////////////////////////////////////////////////////////////
// BOOL CDataSource::IsConnected
//
/////////////////////////////////////////////////////////////////
BOOL CDataSource::IsConnected()
{
    return m_fConnected;
}


/////////////////////////////////////////////////////////////////
// BOOL CDataSource::IsSimilar
//
/////////////////////////////////////////////////////////////////
BOOL CDataSource::IsSimilar(CDataSource* pCDataSource)
{
    ASSERT(pCDataSource);
    
    //Must be Connected to compare
    if(!IsConnected() || !pCDataSource->IsConnected())
        return FALSE;

    ASSERT(m_pwszDBMS && pCDataSource->m_pwszDBMS);
    ASSERT(m_pwszDBMSVer && pCDataSource->m_pwszDBMSVer);
    
    //Similiar: Must be the same DBMS and Version
    if(wcscmp(m_pwszDBMS, pCDataSource->m_pwszDBMS)==0 &&
        wcscmp(m_pwszDBMSVer, pCDataSource->m_pwszDBMSVer)==0)
        return TRUE;

    return FALSE;
}


/////////////////////////////////////////////////////////////////
// BOOL CDataSource::IsEqual
//
/////////////////////////////////////////////////////////////////
BOOL CDataSource::IsEqual(CDataSource* pCDataSource)
{
    ASSERT(pCDataSource);
    
    //Must be Connected to compare
    if(!IsConnected() || !pCDataSource->IsConnected())
        return FALSE;

    ASSERT(m_pwszCatalog && pCDataSource->m_pwszCatalog);
    ASSERT(m_pwszDBMS && pCDataSource->m_pwszDBMS);
    ASSERT(m_pwszDBMSVer && pCDataSource->m_pwszDBMSVer);
    
    //Equal: 
    //Must have same Catalog, DBMS, Version
    //Note, Don't have to have the same DataSource Name (an access
    //database can have multiple names for the same file)
    if(wcscmp(m_pwszCatalog, pCDataSource->m_pwszCatalog)==0 &&
        wcscmp(m_pwszDBMS, pCDataSource->m_pwszDBMS)==0 &&
        wcscmp(m_pwszDBMSVer, pCDataSource->m_pwszDBMSVer)==0)
        return TRUE;

    return FALSE;
}


/////////////////////////////////////////////////////////////////
// BOOL CDataSource::Disconnect
//
/////////////////////////////////////////////////////////////////
BOOL CDataSource::Disconnect()
{
    //Interfaces
    SAFE_RELEASE(m_pIDBInitialize);
    SAFE_RELEASE(m_pIOpenRowset);
    SAFE_RELEASE(m_pIDBSchemaRowset);
    SAFE_RELEASE(m_pITableDefinition);
    SAFE_RELEASE(m_pIIndexDefinition);
    SAFE_RELEASE(m_pICommandText);

    //Properties
    SAFE_FREE(m_pwszCatalog);
    SAFE_FREE(m_pwszCatalogTerm);
    SAFE_FREE(m_pwszCatalogLocation);
    SAFE_FREE(m_pwszSchemaTerm);
    SAFE_FREE(m_pwszTableTerm);
    SAFE_FREE(m_pwszDataSource);
    SAFE_FREE(m_pwszDBMS);
    SAFE_FREE(m_pwszDBMSVer);

    SAFE_FREE(m_pwszProviderFileName);
    SAFE_FREE(m_pwszProviderVer);
    SAFE_FREE(m_pwszProviderOLEDBVer);

    m_fConnected = FALSE;
    return TRUE;
}


/////////////////////////////////////////////////////////////////
// BOOL CDataSource::Connect
//
/////////////////////////////////////////////////////////////////
HRESULT CDataSource::Connect(HWND hWnd, CDataSource* pCDataSource)
{
    ASSERT(hWnd);

    HRESULT				hr;
    ULONG               cPropSets = 0;
    DBPROPSET*			rgPropSets = NULL;
    
    IDBProperties*		pIDBProperties = NULL;
    IDBCreateSession*   pIDBCreateSession = NULL;

    //Local interface pointers, until we have a connection
    IDBInitialize*		pIDBInitialize = NULL;
    IOpenRowset*		pIOpenRowset = NULL;
    IDBCreateCommand*   pIDBCreateCommand = NULL;
    ICommandText*		pICommandText = NULL;

    ULONG chEaten = 0;
    IMoniker* pIMoniker = NULL;

    
    //if there is no provider chosen pick something from the enumerator
    if(pCDataSource == NULL)
    {
        //if there is an enumerator
        if (m_pIParseDisplayName)
        {
            //Get the IDBInitalize interface
            //Could just do a CoCreateInstance on the Provider CLSID, but since were using
            //The enumerator, we can be a more general app by using IParseDisplayName
            ASSERT(m_pwszProviderParseName);
            XTESTC(hr = m_pIParseDisplayName->ParseDisplayName(NULL, m_pwszProviderParseName, &chEaten, &pIMoniker));
            XTESTC(hr = BindMoniker(pIMoniker, 0, IID_IDBInitialize, (void**)&pIDBInitialize));
        }
        else
        {
            //if there is no ParseDisplay name there was no enumerator
            //so let try whatever the user typed in if something was typed in
            if (m_pwszProviderName)
            {
                CLSID	clsid;
                hr = CLSIDFromProgID(m_pwszProviderName, &clsid);

                if(SUCCEEDED(hr))
                {
                    XTESTC(hr = CoCreateInstance(clsid, NULL, CLSCTX_INPROC_SERVER, IID_IDBInitialize, (void**)&pIDBInitialize));
                }
            }
        }
    }
    else
    {
        //get what was chosen
        CLSID	clsid;
        XTESTC(hr = CLSIDFromProgID(m_pwszProviderName, &clsid));
        if (S_OK	== hr)
        {
            XTESTC(hr = CoCreateInstance(clsid, NULL, CLSCTX_INPROC_SERVER, IID_IDBInitialize, (void**)&pIDBInitialize));
        }
        else
        {
            goto CLEANUP;
        }
    }

    //DBPROP_INIT_HWND
    if(IsSettableProperty(pIDBInitialize, DBPROP_INIT_HWND, DBPROPSET_DBINIT))
	{
#ifdef _WIN64
        SetProperty(DBPROP_INIT_HWND, DBPROPSET_DBINIT, &cPropSets, &rgPropSets, DBTYPE_I8, (LONG_PTR)hWnd);
#else
        SetProperty(DBPROP_INIT_HWND, DBPROPSET_DBINIT, &cPropSets, &rgPropSets, DBTYPE_I4, (LONG_PTR)hWnd);
#endif
	}

    //DBPROP_INIT_PROMPT
    if(IsSettableProperty(pIDBInitialize, DBPROP_INIT_PROMPT, DBPROPSET_DBINIT))
        SetProperty(DBPROP_INIT_PROMPT, DBPROPSET_DBINIT, &cPropSets, &rgPropSets, DBTYPE_I2, DBPROMPT_COMPLETE);
    //DBPROP_INIT_MODE
    if(IsSettableProperty(pIDBInitialize, DBPROP_INIT_MODE, DBPROPSET_DBINIT))
        SetProperty(DBPROP_INIT_MODE, DBPROPSET_DBINIT, &cPropSets, &rgPropSets, DBTYPE_I4, DB_MODE_READWRITE | DB_MODE_SHARE_DENY_WRITE);

    //Set the DataSource Properties
    //IDBProperties is a MANDATORY interface, if error try and Initialize anyway
    XTEST(hr = pIDBInitialize->QueryInterface(IID_IDBProperties, (void **)&pIDBProperties));
    XTEST(hr = pIDBProperties->SetProperties(cPropSets, rgPropSets));
        
    //Initailize
    XTESTC(hr = pIDBInitialize->Initialize());

    //Get the Session Object
    XTESTC(hr = pIDBInitialize->QueryInterface(IID_IDBCreateSession, (void **)&pIDBCreateSession));

    //Create the SessionObject
    XTESTC(hr = pIDBCreateSession->CreateSession(NULL, IID_IOpenRowset, (IUnknown**)&pIOpenRowset));

    //Get the CommandText object - If supported
    if(SUCCEEDED(pIOpenRowset->QueryInterface(IID_IDBCreateCommand, (void**)&pIDBCreateCommand)))
        XTESTC(hr = pIDBCreateCommand->CreateCommand(NULL, IID_ICommandText, (IUnknown**)&pICommandText));

    //Do away with any previous connections
    Disconnect();
    
    //If we have made it this far, we are successfully connected, 
    m_fConnected = TRUE;

    //Save the new connection interfaces
    SAFE_ADDREF(pIDBInitialize);
    m_pIDBInitialize = pIDBInitialize;

    SAFE_ADDREF(pIOpenRowset);
    m_pIOpenRowset	= pIOpenRowset;

    SAFE_ADDREF(pICommandText);
    m_pICommandText = pICommandText;

    //Obtain ITableDefinition/IIndexDefintion if supported
    pIOpenRowset->QueryInterface(IID_ITableDefinition, (void**)&m_pITableDefinition);
    pIOpenRowset->QueryInterface(IID_IIndexDefinition, (void**)&m_pIIndexDefinition);
    pIOpenRowset->QueryInterface(IID_IDBSchemaRowset,  (void**)&m_pIDBSchemaRowset);

    //Now get the connection properties
    QTESTC(hr = GetConnectionProps());

CLEANUP:
    FreeProperties(cPropSets, rgPropSets);

    //Release Interfaces
    SAFE_RELEASE(pIDBInitialize);
    SAFE_RELEASE(pIDBProperties);
    SAFE_RELEASE(pIDBCreateSession);
    SAFE_RELEASE(pIOpenRowset);
    SAFE_RELEASE(pIDBCreateCommand);
    SAFE_RELEASE(pICommandText);
    SAFE_RELEASE(pIMoniker);
    return hr;
}



/////////////////////////////////////////////////////////////////////////////
// HRESULT CDataSource::GetConnectionProps
//
/////////////////////////////////////////////////////////////////////////////
HRESULT CDataSource::GetConnectionProps()
{
    HRESULT hr = S_OK;
    DWORD dwSqlSupport = 0;
    
    //DBPROP_CURRENTCATALOG
    hr = GetProperty(m_pIDBInitialize, DBPROP_CURRENTCATALOG, 
                            DBPROPSET_DATASOURCE, &m_pwszCatalog); 

    //DBPROP_CATALOGTERM
    hr = GetProperty(m_pIDBInitialize, DBPROP_CATALOGTERM, 
                            DBPROPSET_DATASOURCEINFO, &m_pwszCatalogTerm); 
    if(!m_pwszCatalogTerm[0])
    {
        SAFE_FREE(m_pwszCatalogTerm);
        m_pwszCatalogTerm = wcsDuplicate(L"Catalog");
    }


    //DBPROP_CATALOGLOCATION
    hr = GetProperty(m_pIDBInitialize, DBPROP_CATALOGLOCATION, 
                            DBPROPSET_DATASOURCEINFO, &m_pwszCatalogLocation); 

    //DBPROP_SCHEMATERM
    hr = GetProperty(m_pIDBInitialize, DBPROP_SCHEMATERM, 
                            DBPROPSET_DATASOURCEINFO, &m_pwszSchemaTerm); 
    if(!m_pwszSchemaTerm[0])
    {
        SAFE_FREE(m_pwszSchemaTerm);
        m_pwszSchemaTerm = wcsDuplicate(L"Schema");
    }

    //DBPROP_TABLETERM
    hr = GetProperty(m_pIDBInitialize, DBPROP_TABLETERM, 
                            DBPROPSET_DATASOURCEINFO, &m_pwszTableTerm); 
    if(!m_pwszTableTerm[0])
    {
        SAFE_FREE(m_pwszTableTerm);
        m_pwszTableTerm = wcsDuplicate(L"Table");
    }

    //DBPROP_ACTIVESESSIONS
    hr = GetProperty(m_pIDBInitialize, DBPROP_ACTIVESESSIONS, 
                            DBPROPSET_DATASOURCEINFO, &m_ulActiveSessions);

    //DBPROP_DATASOURCENAME
    hr = GetProperty(m_pIDBInitialize, DBPROP_DATASOURCENAME, 
                            DBPROPSET_DATASOURCEINFO, &m_pwszDataSource);

    //DBPROP_DBMSNAME
    hr = GetProperty(m_pIDBInitialize, DBPROP_DBMSNAME, 
                            DBPROPSET_DATASOURCEINFO, &m_pwszDBMS);

    //DBPROP_DBMSVER
    hr = GetProperty(m_pIDBInitialize, DBPROP_DBMSVER, 
                            DBPROPSET_DATASOURCEINFO, &m_pwszDBMSVer);

    //DBPROP_DATASOURCEREADONLY 
    hr = GetProperty(m_pIDBInitialize, DBPROP_DATASOURCEREADONLY, 
                            DBPROPSET_DATASOURCEINFO, &m_fReadOnly);

    //DBPROP_PROVIDERNAME
    hr = GetProperty(m_pIDBInitialize, DBPROP_PROVIDERNAME, 
                            DBPROPSET_DATASOURCEINFO, &m_pwszProviderFileName);

    //DBPROP_PROVIDERVER 
    hr = GetProperty(m_pIDBInitialize, DBPROP_PROVIDERVER, 
                            DBPROPSET_DATASOURCEINFO, &m_pwszProviderVer);

    //DBPROP_PROVIDEROLEDBVER 
    hr = GetProperty(m_pIDBInitialize, DBPROP_PROVIDEROLEDBVER, 
                            DBPROPSET_DATASOURCEINFO, &m_pwszProviderOLEDBVer);

    //DBPROP_MULTIPLEPARAMSETS 
    hr = GetProperty(m_pIDBInitialize, DBPROP_MULTIPLEPARAMSETS, 
                            DBPROPSET_DATASOURCEINFO, &m_fMultipleParamSets);

    //DBPROP_STRUCTUREDSTORAGE
    hr = GetProperty(m_pIDBInitialize, DBPROP_STRUCTUREDSTORAGE, 
                            DBPROPSET_DATASOURCEINFO, &m_dwStorageObjects);
    
    //DBPROP_SQLSUPPORT 
    hr = GetProperty(m_pIDBInitialize, DBPROP_SQLSUPPORT, 
                            DBPROPSET_DATASOURCEINFO, &dwSqlSupport);

    //Are PrimaryKeys supported
    m_fPrimaryKeysSupported = dwSqlSupport & DBPROPVAL_SQL_ANSI89_IEF;

    //DBPROP_IRowsetChange
    m_fIRowsetChange = FALSE;
    if(IsSupportedProperty(m_pIDBInitialize, DBPROP_IRowsetChange, DBPROPSET_ROWSET))
        m_fIRowsetChange = TRUE;

    //DBPROP_IRowsetUpdate
    m_fIRowsetUpdate = FALSE;
    if(IsSupportedProperty(m_pIDBInitialize, DBPROP_IRowsetUpdate, DBPROPSET_ROWSET))
        m_fIRowsetUpdate = TRUE;

    return S_OK;
}


/////////////////////////////////////////////////////////////////////////////
// HRESULT CDataSource::GetProviders
//
/////////////////////////////////////////////////////////////////////////////
HRESULT CDataSource::GetProviders()
{
    HRESULT hr;
    
    HROW*		rghRows = NULL;
    DBCOUNTITEM cRowsObtained = 0;
    IRowset*	pIRowset = NULL;

    IAccessor* pIAccessor = NULL;
    HACCESSOR hAccessor = DB_NULL_HACCESSOR;

    //Release Previous ProviderInfo
    m_cProviderInfo = 0;
    SAFE_FREE(m_rgProviderInfo);
    ISourcesRowset* pISourcesRowset = NULL;
    
    // Bind the user and table name for the list
    const static ULONG cBindings = 4;
    const static DBBINDING rgBindings[cBindings] = 
        {
            1,	 			
            offsetof(PROVIDERINFO, wszName),
            0,
            0,	
            NULL,			
            NULL, 		
            NULL,		
            DBPART_VALUE,
            DBMEMOWNER_CLIENTOWNED,		
            DBPARAMIO_NOTPARAM, 
            MAX_NAME_LEN, 		
            0, 				
            DBTYPE_WSTR, 	
            0,	
            0, 				

            2,	 			
            offsetof(PROVIDERINFO, wszParseName),
            0,
            0,	
            NULL,			
            NULL, 		
            NULL,		
            DBPART_VALUE,
            DBMEMOWNER_CLIENTOWNED,		
            DBPARAMIO_NOTPARAM, 
            MAX_NAME_LEN, 		
            0, 				
            DBTYPE_WSTR, 	
            0,	
            0, 				

            3,	 			
            offsetof(PROVIDERINFO, wszDescription),
            0,
            0,	
            NULL,			
            NULL, 		
            NULL,		
            DBPART_VALUE,
            DBMEMOWNER_CLIENTOWNED,		
            DBPARAMIO_NOTPARAM, 
            MAX_NAME_LEN, 		
            0, 				
            DBTYPE_WSTR, 	
            0,	
            0, 				
    
            4,	 			
            offsetof(PROVIDERINFO, wType),
            0,
            0,	
            NULL,			
            NULL, 		
            NULL,		
            DBPART_VALUE,
            DBMEMOWNER_CLIENTOWNED,		
            DBPARAMIO_NOTPARAM, 
            sizeof(DBTYPE), 		
            0, 				
            DBTYPE_UI2, 	
            0,	
            0, 				
    };


    //Initialize the OLE DB Enumerator and Obtain rowset
    hr = CoCreateInstance(CLSID_OLEDB_ENUMERATOR, NULL, CLSCTX_INPROC_SERVER, IID_ISourcesRowset, (void**)&pISourcesRowset);
    if (REGDB_E_CLASSNOTREG	== hr)
    {
        //if there is no enumerator, horse'em

        //Alloc room for ProviderInfo
        m_cProviderInfo++;
        SAFE_REALLOC(m_rgProviderInfo, PROVIDERINFO, m_cProviderInfo);
        memset(&m_rgProviderInfo[0], 0, sizeof(PROVIDERINFO));

        StringCchCopyW(m_rgProviderInfo[0].wszName,
                       sizeof(m_rgProviderInfo[0].wszName)/sizeof(WCHAR),
                       L"MSDASQL");
        StringCchCopyW(m_rgProviderInfo[0].wszParseName,
                       sizeof(m_rgProviderInfo[0].wszParseName)/sizeof(WCHAR),
                       L"{c8b522cb-5cf3-11ce-ade5-00aa0044773d}");
        StringCchCopyW(m_rgProviderInfo[0].wszDescription,
                       sizeof(m_rgProviderInfo[0].wszDescription)/sizeof(WCHAR),
                       L"Microsoft OLE DB Provider for ODBC Drivers");
        m_rgProviderInfo[0].wType = DBSOURCETYPE_DATASOURCE;

        return hr;
    }

    XTESTC(hr);
    XTESTC(hr = pISourcesRowset->GetSourcesRowset(NULL, IID_IRowset, 0, NULL, (IUnknown**)&pIRowset));

    //Create Accessor
    XTESTC(hr = pIRowset->QueryInterface(IID_IAccessor, (void **)&pIAccessor));
    XTESTC(hr = pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA, cBindings, rgBindings, 0, &hAccessor, NULL));

    //Obtain IParseDisplayName interface
    XTESTC(hr = pISourcesRowset->QueryInterface(IID_IParseDisplayName, (void**)&m_pIParseDisplayName));

    //Loop through the entire returned rowet
    while(TRUE)
    {
        XTESTC(hr = pIRowset->GetNextRows(NULL, 0, MAX_BLOCK_SIZE, &cRowsObtained, &rghRows));
        
        //ENDOFROWSET
        if(cRowsObtained==0) 
            break;
        
        //Alloc room for ProviderInfo (in chunks)
        SAFE_REALLOC(m_rgProviderInfo, PROVIDERINFO, m_cProviderInfo + cRowsObtained);
        memset(&m_rgProviderInfo[m_cProviderInfo], 0, sizeof(PROVIDERINFO)*cRowsObtained);

        //Loop over rows obtained and get ProviderInfo
        for(ULONG i=0; i<cRowsObtained; i++) 
        {	
            //Get the Data
            XTESTC(hr = pIRowset->GetData(rghRows[i], hAccessor, (void*)&m_rgProviderInfo[m_cProviderInfo]));

            //Only interested in DATASOURCEs, not other ENUMERATORs
            if(m_rgProviderInfo[m_cProviderInfo].wType == DBSOURCETYPE_DATASOURCE)
                m_cProviderInfo++;
        }
            
        //Release all the rows
        XTESTC(hr = pIRowset->ReleaseRows(cRowsObtained, rghRows, NULL, NULL, NULL));
        SAFE_FREE(rghRows);
    }

CLEANUP:
    if(hAccessor && pIAccessor)
        XTEST(pIAccessor->ReleaseAccessor(hAccessor,NULL));

    SAFE_RELEASE(pISourcesRowset);
    SAFE_RELEASE(pIRowset);
    SAFE_RELEASE(pIAccessor);
    SAFE_FREE(rghRows);
    return hr;
}


