//--------------------------------------------------------------------
// Microsoft OLE DB Test Table dump
// Copyright 1995-1999 Microsoft Corporation.  
//
// File name: TABLEDUMP.CPP
//
//      TableDump routines for the Test TableDump.
//
//


////////////////////////////////////////////////////////////////////////
// Defines
//
////////////////////////////////////////////////////////////////////////
#define INITGUID
#define DBINITCONSTANTS


////////////////////////////////////////////////////////////////////////
// Includes
//
////////////////////////////////////////////////////////////////////////
#include "Common.h"
#include "TableDump.h"


////////////////////////////////////////////////////////////////////////
// Globals
//
////////////////////////////////////////////////////////////////////////
extern FILE*  g_fpLogFile = NULL;    // our log file

CTable*		g_pCTable = NULL;

WCHAR*		g_pwszTableName = NULL;

WCHAR*		g_pwszDefaultQuery = NULL;

WCHAR*		g_pwszRootURL = NULL;



//**********************************************************************
// 
// main
// 
// Purpose:
//
//     Entry point for this program.
// 
// Parameters:
//
//     none
//     
// Return Value:
//
//     none
// 
// Comments:      
//
//     Main does some initializing, then calls DumpTable, then cleans up.
//     All of the interesting action in this program happens in DumpTable.
// 
//**********************************************************************
void __cdecl main(int argc, char* argv[])
{
	HRESULT hr = E_FAIL;
	CHAR	szFileName[MAX_QUERY_LEN];
	CHAR*	pszCmdLine = NULL;
	WCHAR*  pwszCmdLine = NULL;
	
	SHORT 	i,cArgs;
	WCHAR*	rgArgs[MAX_ARGS];
	CHAR*   pszOutput = NULL;
	INT		iExit = 0;

	ULONG cPropSets =0;
	DBPROPSET* rgPropSets = NULL;
	TESTC(hr = OleInitialize( NULL ));

	/*
	Set the locale.  LTM has options to set the locale, and TableDump should match these.
	However, setting to .ACP will cause failure of wcstod function in Privlib when run on German.
	The correct thing to do at the minimum is add a command line option for TableDump to pass the
	Locale. We should also add a locale entry to the ini file, and then when the test reads the ini
	file set the locale for the	test to match the locale used at ini file creation time.  LTM is
	normally run with the locale setting as "<Not Set>", but TableDump needs to set the locale here
	anyway.

	Note: If we don't call setlocale here, then TableDump makes incorrect strings for statements.
	*/
	setlocale(LC_ALL, ".ACP");
	
	//Initialize array...
	memset(rgArgs, 0, sizeof(rgArgs));

	//This is for privlib CModInfo class, (NULL - since we don't have CThisTestModule)
	QTESTC_(hr = CreateModInfo(NULL), S_OK); 

	//Don't include the Executable name in the Arguments, just takes up space
	//and causes problems with the parser since the first keyword isn't following
	//semicolon or the first item of the string...
	pszCmdLine = GetCommandLine();
	if(argc>=2 && argv && argv[0] && argv[1])
	{
		//Skip over the Execution name and following quote...
		CHAR* pszTemp = strstr(pszCmdLine, argv[0]);
		if(pszTemp)
			pszCmdLine = pszTemp + strlen(argv[0]) + 1;
	}

	//Now that we have something decent for the InitString, set it...
	pwszCmdLine = ConvertToWCHAR(pszCmdLine);
	GetModInfo()->SetInitString(pwszCmdLine);
	GetModInfo()->ParseInitString();

	//Now use the Privlib to parse this Command Line!
	ParseCmdLine(&cArgs, rgArgs, &cPropSets, &rgPropSets);
	
    OutputText( "\n\n");
    OutputText( "-------------------------\n");

    //Required Arguments were not specified
	if(rgArgs[ARG_PROVIDER]==NULL || 
		(GetModInfo()->GetTableName()==NULL && rgArgs[ARG_CREATETABLE]==NULL))
	{
		//Display usage...
		OutputText( "\n Options:\n");
		OutputText( "	PROVIDER=;                  - required\n");
		OutputText( "	TABLENAME=;                 - required\n");
		OutputText( "	CONNECTIONSTRING=;          - required\n");
		OutputText( "\n");
		OutputText( "\n");
		OutputText( "	DEFAULTQUERY=;              - optional\n");
		OutputText( "	ROOT_URL=;		            - optional\n");
		OutputText( "	BINDINGTYPE=;               - optional [DBTYPE_STR, DBTYPE_WSTR, DBTYPE_VARIANT]\n");
		OutputText( "	CREATETABLE=;               - optional\n");
		OutputText( "	CREATEINDEX=;               - optional\n");
		OutputText( "	OUTPUT=;                    - optional\n");
		OutputText("\n\n Results:\n");
		TESTC(hr = E_FAIL);
	}

	//Current Info
	OutputText( "\n Options:\n");
	OutputText( "  PROVIDER=%S;\n",				rgArgs[ARG_PROVIDER]);
    OutputText( "  TABLENAME=%S;\n",			GetModInfo()->GetTableName());
    OutputText( "  DEFAULTQUERY=%S;\n",			GetModInfo()->GetDefaultQuery());
    OutputText( "  BINDINGTYPE=%S;\n",			rgArgs[ARG_BINDINGTYPE]);
    OutputText( "  CREATETABLE=%S;\n",			rgArgs[ARG_CREATETABLE]);
    OutputText( "  CREATEINDEX=%S;\n",			rgArgs[ARG_CREATEINDEX]);
    OutputText( "  OUTPUT=%S;\n",				rgArgs[ARG_OUTPUT]);
	
	//Display Properties
	OutputText( "\n Properties:\n");
	DisplayAllProps(NULL, cPropSets, rgPropSets);
	
	//Results
	OutputText("\n\n Results:\n");
		
	// Open output file
	if(rgArgs[ARG_OUTPUT] == NULL)
	{
		//Default Output File - TableDump.ini
		strcpy(szFileName, "TableDump.ini");
	}
	else
	{
		WideCharToMultiByte(CP_ACP, 0, rgArgs[ARG_OUTPUT], -1, szFileName, MAX_QUERY_LEN, NULL, NULL);
	}

   	//Open the File in Binary mode, so there is no translation of special characters
	//Expecially line-feeds and carriage returns...
	if(szFileName[0])
		g_fpLogFile = fopen( szFileName, "wt");
	if (!g_fpLogFile)
    {
		OutputText("ERROR: cannot open log File: %s\n", szFileName);
		TESTC(hr = STG_E_FILENOTFOUND);
	}
  
    TESTC(hr = DumpTable(cArgs, rgArgs, cPropSets, rgPropSets, pszCmdLine));
    
CLEANUP:
	//Close file
	if (g_fpLogFile)
    	fclose(g_fpLogFile);

	//Results
	if(SUCCEEDED(hr))
	{
		OutputText("  Succeeded! Produced: %s\n", szFileName);
	}
	else
	{
		//So batch files know this operation failed...
		iExit = 1; 

		//Indicate any errors...
		if(rgArgs[ARG_PROVIDER]==NULL)
			OutputText( "  ERROR:  PROVIDER=; was not specified\n");
		if(GetModInfo()->GetTableName()==NULL && rgArgs[ARG_CREATETABLE]==NULL)
			OutputText( "  ERROR:  TABLENAME=; was not specified\n");
		OutputText( "  Failed! 0x%08x = %S\n", hr, GetErrorName(hr));

		//On failure make sure we remove the file.
		//This way we don't have a partially completed (or empty) file depending upon where
		//the error occurred.  This way we prevent someone using a corrupted file...
		DeleteFileA(szFileName);
	}
	OutputText( "-------------------------\n");

	//FreeMemory
	for(i=0; i<MAX_ARGS; i++)
		SAFE_FREE(rgArgs[i]);

	//FreeProperties...
	FreeProperties(&cPropSets, &rgPropSets);
	SAFE_FREE(pwszCmdLine);
   	OleUninitialize();
	exit(iExit);
	return;    
}    



//**********************************************************************
//  
//  DumpTable
//  
//  Comments:      
//  
//     At a high level, an OLE DB data consumer obtains data by 
//     
//     1. Getting hooked up to a data provider's Data Source object,
//        and initializing that object
//     2. Getting a DBSession object from the Data Source object
//     3. Getting the data from the Rowset object.
//     
//     DumpTable follows these steps by making calls to GetDataSource,
//     GetDBSessionDataSource, and DumpRowset    
//  
//**********************************************************************


HRESULT DumpTable(ULONG cArgs, WCHAR* rgArgs[], ULONG cPropSets, DBPROPSET* rgPropSets, CHAR* pszCmdLine)
{
	ASSERT(pszCmdLine);

	IDBInitialize*	    pIDBInitialize 	= NULL;
    IOpenRowset*        pIOpenRowset    = NULL;
    IRowset*		    pIRowset		= NULL;
	CTable*				pCTable			= NULL;

	WCHAR*		pwszProvider	= rgArgs[ARG_PROVIDER];
	WCHAR*		pwszDefaultQuery= GetModInfo()->GetDefaultQuery();
	WCHAR*		pwszTableName	= GetModInfo()->GetTableName();
	WCHAR*		pwszRootURL		= GetModInfo()->GetRootURL();
	WCHAR*		pwszRowURL		= NULL;
	WCHAR*		pwszOrderByNumeric = NULL;
	DBCOUNTITEM cRows			= 0;
	DBORDINAL	ulIndex			= 0;
	HRESULT	    hr = E_FAIL;

	//Now try to obtain the BindingType
	DBTYPE wBindingType = DBTYPE_WSTR;
	if(rgArgs[ARG_BINDINGTYPE])
	{
		wBindingType = GetDBType(rgArgs[ARG_BINDINGTYPE]);
		if(wBindingType == 0)
			wBindingType = DBTYPE_WSTR;
	}

	//Get DataSource and Session
	TESTC(hr = GetDataSource(pwszProvider, cPropSets, rgPropSets, &pIDBInitialize));
    TESTC(hr = GetSession( pIDBInitialize, &pIOpenRowset ));

	//Create the CTable object (using the Privlib)
	pCTable = new CTable(pIOpenRowset, pwszTableName);

	//Obtain cRows from argument
	if(rgArgs[ARG_CREATETABLE])
	{
		cRows = (DBCOUNTITEM)wcstoul(rgArgs[ARG_CREATETABLE], NULL, 10);
		if(cRows == -1)
			cRows = 100;
	}
	
	//Obtain ulIndex from argument
	if(rgArgs[ARG_CREATEINDEX])
	{
		ulIndex = (DBORDINAL)wcstoul(rgArgs[ARG_CREATEINDEX], NULL, 10);
		if(ulIndex == -1)
			ulIndex = 1;
	}

	//Construct the table.
	//NOTE: If a table is already set (GetModInfo) then CTable uses that table instead
	//of actually creating the table, but also allows use to obtain col info, columnsrowset, etc...
	TESTC(hr = pCTable->CreateTable(cRows, ulIndex, NULL, PRIMARY, TRUE));
	if(!pwszTableName)
	{
		pwszTableName = pCTable->GetTableName();
		GetModInfo()->SetTableName(pwszTableName);
	}

	//The DefaultQuery may have a %s to indicate the TableName format specifier.
	if(pwszDefaultQuery)
	{
		WCHAR* pwszFormat = wcsstr(pwszDefaultQuery, L"%s");
		if(pwszFormat)
		{
			pwszFormat = (WCHAR*)PROVIDER_ALLOC((wcslen(pwszDefaultQuery)+wcslen(pwszTableName)+1)*sizeof(WCHAR));
			swprintf(pwszFormat, pwszDefaultQuery, pwszTableName);
			pwszDefaultQuery = pwszFormat;
			GetModInfo()->SetDefaultQuery(pwszDefaultQuery);
		}
	}

	if(pwszDefaultQuery == NULL && rgArgs[ARG_CREATETABLE])
	{
		// User requested table creation but did not supply a default query.
		// By default, use a select statement with an order by clause on
		// a numeric column.
		if(SUCCEEDED(pCTable->CreateSQLStmt(SELECT_ORDERBYNUMERIC,NULL,&pwszOrderByNumeric,NULL,NULL)))
		{
			pwszDefaultQuery = pwszOrderByNumeric;
		}
	}
	
	//Display ProviderInfo
	DumpProviderInfo(pwszProvider, pwszTableName, pszCmdLine);

	//Get pointer to the rowset to dump data from.
	TESTC(hr = GetRowset( pIDBInitialize, pIOpenRowset, pwszTableName, pwszDefaultQuery, IID_IRowset, (IUnknown**)&pIRowset ));

	//Save pointers to these variables. These will be required
	//in DumpRowset, since DumpURLInfo and DumpQueryInfo will
	//be called in DumpAllRows.
	g_pCTable = pCTable;
	g_pwszTableName = pwszTableName;
	g_pwszDefaultQuery = pwszDefaultQuery;
	g_pwszRootURL = pwszRootURL;

	//Display Data section
	TESTC(hr = DumpRowset( pIRowset, wBindingType, pCTable ));

CLEANUP:
	SAFE_FREE(pwszRowURL);
	SAFE_FREE(pwszOrderByNumeric);
	SAFE_RELEASE(pIRowset);
    SAFE_RELEASE(pIOpenRowset);
    SAFE_RELEASE(pIDBInitialize);
	SAFE_DELETE(pCTable);
	return hr;
}						    





//**********************************************************************
//  
// GetDataSource
// 
// Purpose:
//
//     Returns an IDBInitialize interface pointer on Data Source object.
//
// Parameters:
// 	
//   IDBInitialize** ppIDBInitialize  - out pointer through which to return
// 									        IDBInitialize pointer on data 
//								            provider's Data Source object 	
// 
// Return Value:
// 
// 	S_OK		- Success
//  E_*	    	- Failure
//     
//
// Comments:      
// 
//     The pointer returned through ppIDBInitialize has been AddRef'ed,
//     it must be Release'd later by the caller.
//  
//**********************************************************************

HRESULT GetDataSource
	(
	const LPWSTR lpProgID,
	ULONG cPropSets,
	DBPROPSET* rgPropSets,
	IDBInitialize**	ppIDBInitialize
	)
{
	IDBInitialize*	pIDBInitialize = NULL;
	IDBProperties*	pIDBProperties = NULL;
	CLSID			clsid;
	HRESULT	hr	=	S_OK;

	ASSERT(ppIDBInitialize);
	ASSERT(lpProgID);

	//Find the CLSID from the ProgID
	//The user may have entered a ProgID or a CLSID 
	//Since both methds return the same errors, just do a retry...
	if(FAILED(hr = CLSIDFromProgID(lpProgID, &clsid)))
		XTESTC(hr = CLSIDFromString(lpProgID, &clsid));

	//Create an instance of the provider
	//Use CLSCTX_SERVER incase a provider has support for LOCAL_SERVER otherwise
	//will fall back to INPROC_SERVER if not...
	XTESTC(hr = CoCreateInstance(clsid, NULL, CLSCTX_SERVER, IID_IDBInitialize, (void **)&pIDBInitialize )); 
	
    XTESTC(hr = pIDBInitialize->QueryInterface( IID_IDBProperties, (void**)&pIDBProperties));

	XTEST(hr = pIDBProperties->SetProperties(cPropSets, rgPropSets));
	if(hr == DB_S_ERRORSOCCURRED || hr == DB_E_ERRORSOCCURRED)
		DisplayPropErrors(NULL, cPropSets, rgPropSets);
	TESTC(hr);

	hr = pIDBInitialize->Initialize();
	if(hr == DB_S_ERRORSOCCURRED || hr == DB_E_ERRORSOCCURRED)
		DisplayPropErrors(NULL, IID_IDBProperties, pIDBInitialize);
	XTESTC(hr);

	*ppIDBInitialize = pIDBInitialize;
	SAFE_ADDREF(pIDBInitialize);

CLEANUP:    
	SAFE_RELEASE( pIDBProperties );
	SAFE_RELEASE( pIDBInitialize );
	return hr;    
}




//  **********************************************************************
//
// GetSession
//
// Purpose:
//      Calls the provider's Data Source object to get an IOpenRowset interface
//      pointer on a DBSession object.  
//      
// Parameters:
//      pIDBInitialize      - pointer to Data Source object
//      ppIOpenRowset   - out pointer through which to return 
//                            IOpenRowset pointer on DBSession object
//
// Return Value: 
//
// 	S_OK		- Success
//  E_*	    	- Failure
//
//
// Comments:
//
//      The interface pointer returned through ppIOpenRowset has been 
//      AddRef'ed, the caller must Release it later.
//
//**********************************************************************

HRESULT GetSession
    (
    IDBInitialize*      pIDBInitialize,      // [in]
    IOpenRowset**       ppIOpenRowset    // [out]
    )
{
    IDBCreateSession*   pIDBCreateSession;
    IOpenRowset*        pIOpenRowset;
    HRESULT             hr;


    //OutputText( "Getting a DBSession object from the data source object...\n" );
    
    ASSERT(pIDBInitialize);
    ASSERT(ppIOpenRowset);

    XTESTC(hr = pIDBInitialize->QueryInterface( IID_IDBCreateSession, (void**)&pIDBCreateSession));
    XTESTC(hr = pIDBCreateSession->CreateSession( NULL, IID_IOpenRowset, (IUnknown**)&pIOpenRowset ));    
     
     // all went well
     *ppIOpenRowset = pIOpenRowset;
     
CLEANUP:
	SAFE_RELEASE(pIDBCreateSession);
    return hr;
}


//**********************************************************************
//
// GetRowset
//
// Purpose:
//      Calls the provider's DBSession object to get an IRowset interface
//      pointer on a Rowset object.  
//      
// Parameters:
//      pIOpenRowset        - interface pointer on DBSession object
//      pwszTableName       - name of "table" (in this case text file)
//      ppIUnknown	       - out pointer through which to return 
//                            IRowset pointer on Rowset object
//
// Return Value: 
//
// 	S_OK		- Success
//  E_*	    	- Failure
//
// Comments:
//
//      The interface pointer returned through ppIUnknown has been 
//      AddRef'ed, the caller must Release it later.
//
///**********************************************************************

HRESULT GetRowset
    (
	IDBInitialize* pIDBInitialize,
    IOpenRowset*   pIOpenRowset,		// [in]
    WCHAR*         pwszTableName,		// [in] 
    WCHAR*         pwszDefaultQuery,    // [in] 
	REFIID		   riid,
    IUnknown**     ppIUnknown			// [out]
    )
{
    ASSERT(pIDBInitialize);
	ASSERT(pIOpenRowset != NULL);
    ASSERT(ppIUnknown  != NULL );
	HRESULT         hr = E_FAIL;
	
	IDBCreateCommand* pIDBCreateCommand = NULL;
	ICommandText* pICommandText = NULL;
	ICommandProperties* pICommandProperties = NULL;
	IDBSchemaRowset* pIDBSchemaRowset = NULL;
	ISourcesRowset* pISourcesRowset = NULL;
	IColumnsRowset* pIColumnsRowset = NULL;
	CLSID clsidEnumerator;

	ULONG		cPropSets = 0;
	DBPROPSET*	rgPropSets = NULL;
	GUID		guidSchema;

	// tell the provider which table to open
	DBID TableID;
	TableID.eKind           = DBKIND_NAME;
	TableID.uName.pwszName  = pwszTableName;

	//The user may have reuqested through the InitString exactly how to create the
	//rowset, so override any parameter passed in if this is the case...
	EQUERY eQuery = USE_OPENROWSET;
	if(pwszDefaultQuery)
		eQuery = SELECT_ALLFROMTBL;
	if(GetModInfo()->GetRowsetQuery() != NO_QUERY)
		eQuery = GetModInfo()->GetRowsetQuery();

	//If using a Default Query we might as well use that instead of OpenRowset
	switch(eQuery)
	{
		case USE_OPENROWSET:
		case USE_SUPPORTED_SELECT_ALLFROMTBL:
		{
			//Some providers may require IRowsetLocate to position on BLOBs
			if(SettableProperty(DBPROP_IRowsetLocate, DBPROPSET_ROWSET, pIDBInitialize))
				SetProperty(DBPROP_IRowsetLocate, DBPROPSET_ROWSET, &cPropSets, &rgPropSets, DBTYPE_BOOL, VARIANT_TRUE);

			XTESTC(hr = pIOpenRowset->OpenRowset
								(
								NULL,                   // pUnkOuter - we are not aggregating
								&TableID,               // pTableID -  the table we want
								NULL,					// pIndexID - the index we want
								riid,		            // riid - interface we want on the rowset object
								cPropSets,              // cProperties - we are niave about props for now
								rgPropSets,             // prgProperties[]
								ppIUnknown				// ppRowset
								));
			break;
		}

		case USE_ENUMERATOR:
		{
			//Obtain an Instance to the Enumerator
			XTESTC(hr = CLSIDFromProgID(GetModInfo()->GetEnumerator(), &clsidEnumerator));
			XTESTC(hr = GetModInfo()->CreateProvider(clsidEnumerator, NULL, IID_ISourcesRowset, (IUnknown**)&pISourcesRowset));

			//ISourcesRowset::GetSourcesRowset
			XTESTC(hr = pISourcesRowset->GetSourcesRowset(NULL, riid, cPropSets, rgPropSets, ppIUnknown));
			break;
		}

		case USE_COLUMNSROWSET:
		{
			//Obtain the rowset, from which columns rowset will be called
			XTESTC(hr = pIOpenRowset->OpenRowset
								(
								NULL,							// pUnkOuter - we are not aggregating
								&TableID,						// pTableID -  the table we want
								NULL,							// pIndexID - the index we want
								IID_IColumnsRowset,				// riid - interface we want on the rowset object
								0,								// cProperties - we are native about props for now
								NULL,							// prgProperties[]
								(IUnknown**)&pIColumnsRowset	// ppRowset
								));
		
			//Now obtain the ColumnsRowset
			XTESTC(hr = pIColumnsRowset->GetColumnsRowset(NULL, 0, NULL, riid, cPropSets, rgPropSets, ppIUnknown));
			break;
		}

		case SELECT_DBSCHEMA_ASSERTIONS:
		case SELECT_DBSCHEMA_CATALOGS:
		case SELECT_DBSCHEMA_CHARACTER_SETS:
		case SELECT_DBSCHEMA_CHECK_CONSTRAINTS:
		case SELECT_DBSCHEMA_COLLATIONS:
		case SELECT_DBSCHEMA_COLUMN_DOMAIN_USAGE:
		case SELECT_DBSCHEMA_COLUMN_PRIVILEGES:
		case SELECT_DBSCHEMA_COLUMNS:
		case SELECT_DBSCHEMA_CONSTRAINT_COLUMN_USAGE:
		case SELECT_DBSCHEMA_CONSTRAINT_TABLE_USAGE:
		case SELECT_DBSCHEMA_FOREIGN_KEYS:
		case SELECT_DBSCHEMA_INDEXES:
		case SELECT_DBSCHEMA_KEY_COLUMN_USAGE:
		case SELECT_DBSCHEMA_PRIMARY_KEYS:
		case SELECT_DBSCHEMA_PROCEDURE_PARAMETERS:
		case SELECT_DBSCHEMA_PROCEDURES:
		case SELECT_DBSCHEMA_PROVIDER_TYPES:
		case SELECT_DBSCHEMA_REFERENTIAL_CONSTRAINTS:
		case SELECT_DBSCHEMA_SCHEMATA:
		case SELECT_DBSCHEMA_SQL_LANGUAGES:
		case SELECT_DBSCHEMA_STATISTICS:
		case SELECT_DBSCHEMA_TABLE_CONSTRAINTS:
		case SELECT_DBSCHEMA_TABLE_PRIVILEGES:
		case SELECT_DBSCHEMA_TABLE:
		case SELECT_DBSCHEMA_TRANSLATIONS:
		case SELECT_DBSCHEMA_USAGE_PRIVILEGES:
		case SELECT_DBSCHEMA_VIEW_COLUMN_USAGE:
		case SELECT_DBSCHEMA_VIEW_TABLE_USAGE:
		case SELECT_DBSCHEMA_VIEWS:
		{
			if(GetSchemaGUID(eQuery, &guidSchema))
			{
				//Obtain SchemaRowset interface
				XTESTC(hr = pIOpenRowset->QueryInterface(IID_IDBSchemaRowset, (void**)&pIDBSchemaRowset));

				//IDBSchemaRowset::GetRowset
				XTESTC(hr = pIDBSchemaRowset->GetRowset(NULL, guidSchema, 
					0, NULL, riid, cPropSets, rgPropSets, ppIUnknown));
			}
			break;
		}

		default:
		{
			//Some providers may require IRowsetLocate to position on BLOBs
			if(SettableProperty(DBPROP_IRowsetLocate, DBPROPSET_ROWSET, pIDBInitialize))
				SetProperty(DBPROP_IRowsetLocate, DBPROPSET_ROWSET, &cPropSets, &rgPropSets, DBTYPE_BOOL, VARIANT_TRUE);

			//Obtain Command Object
			XTESTC(hr = pIOpenRowset->QueryInterface(IID_IDBCreateCommand, (void**)&pIDBCreateCommand));
			XTESTC(hr = pIDBCreateCommand->CreateCommand(NULL, IID_ICommandText, (IUnknown**)&pICommandText));

			//SetProperties
			XTESTC(hr = pICommandText->QueryInterface(IID_ICommandProperties, (void**)&pICommandProperties));
			XTESTC(hr = pICommandProperties->SetProperties(cPropSets, rgPropSets));

			//Execute Command
			XTESTC(hr = pICommandText->SetCommandText(DBGUID_DBSQL, pwszDefaultQuery));
			XTESTC(hr = pICommandText->Execute(NULL, riid, 0, NULL, ppIUnknown));
			break;
		}
	};
	
CLEANUP:
	//Display Property Errors (if occurred)
	if(hr == DB_S_ERRORSOCCURRED || hr == DB_E_ERRORSOCCURRED)
	{
		if(pwszDefaultQuery)
			DisplayPropErrors(NULL, IID_ICommandProperties, pICommandText);
		else
			DisplayPropErrors(NULL, cPropSets, rgPropSets);
	}

	FreeProperties(&cPropSets, &rgPropSets);
	SAFE_RELEASE(pIDBCreateCommand);
	SAFE_RELEASE(pICommandText);
	SAFE_RELEASE(pICommandProperties);
	SAFE_RELEASE(pIDBSchemaRowset);
	SAFE_RELEASE(pISourcesRowset);
	SAFE_RELEASE(pIColumnsRowset);
	return hr;
}    


//**********************************************************************
// 
// DumpRowset
// 
// Purpose:
// 
//     Pulls the data from a Rowset object.
//     
// Parameters:
// 
// 	IRowset*	pIRowset    -  interface pointer on data provider's
//                             Rowset object
// 
// Return Value:
// 
//     S_OK        - Success
//     E_*         - Failure
// 
// Comments:      
// 
//     At a high level, a consumer pulls the data from a Rowset object by:
//     
//     1. getting metadata for the Rowset's columns
//     2. using that metadata, along with the consumer's own knowledge of
//        how it wants to recieve the data, to create bindings. Bindings
//        represent how the actual data in the Rowset's columns is
//        actually transferred to the consumer's buffer.
//     3. pass the bindings to the Rowset, and get in return an accessor
//        handle that represents that particulr set of bindings   
//     4. get the actual data
//     5. clean up the rowset (at a minumum, release the accessor) 
//     
//     DumpRowset performs these steps by calling GetColumnsInfo,
//     SetupBindings, CreateAccessor, GetData, and CleanupRowset
//       
//**********************************************************************

HRESULT DumpRowset
	(
	IRowset*	pIRowset,
	DBTYPE		wBindingType,
	CTable*		pCTable
	)
{
	DBORDINAL		cColumns		= 0;
	DBCOLUMNINFO*	rgColumnInfo 	= NULL;

	DBCOUNTITEM		cBindings		= 0;
	DBBINDING*		rgBindings		= NULL;
	DBLENGTH		ulRowSize		= 0;
	HACCESSOR		hAccessor		= NULL;
	WCHAR*			pwszStringBuffer= NULL;
	HRESULT 		hr				= S_OK;
    
	if(pIRowset == NULL)
		return E_FAIL;
	
	TESTC(hr = GetColumnsInfo( pIRowset, &cColumns, &rgColumnInfo, &pwszStringBuffer ));

	//Setup bindings and create the Accessor
	TESTC(hr = SetupBindings( cColumns, rgColumnInfo, wBindingType, &cBindings, &rgBindings, &ulRowSize ));
	TESTC(hr = CreateAccessor( pIRowset, cBindings, rgBindings, &hAccessor ));
	
	//Display the Rowset...
	TESTC(hr = DumpAllRows( pIRowset, cBindings, rgBindings, hAccessor, wBindingType, ulRowSize, pCTable));

CLEANUP:
	if(pIRowset && hAccessor)
		CleanupRowset( pIRowset, hAccessor );
	SAFE_FREE(rgColumnInfo);
	SAFE_FREE(pwszStringBuffer);
	return hr;		
}






////////////////////////////////////////////////////////////////////////
// GetColumnsInfo
//
////////////////////////////////////////////////////////////////////////
HRESULT GetColumnsInfo
	(
	IUnknown*		pIUnkRowset,
	DBORDINAL*		pcColumns,
	DBCOLUMNINFO**	prgColumnInfo,
	WCHAR**			ppStringsBuffer,
	BOOL			fWantBookmark
	)
{
	IColumnsInfo* 	pIColumnsInfo = NULL;
    DBORDINAL		cColumns = 0;
    DBCOLUMNINFO*	rgColumnInfo = NULL;
    WCHAR*			pStringsBuffer = NULL;
	HRESULT 		hr = S_OK;
    

	ASSERT(pIUnkRowset);
    ASSERT(pcColumns && prgColumnInfo && ppStringsBuffer);

	//GetColumnsInfo 
	XTESTC(hr = pIUnkRowset->QueryInterface( IID_IColumnsInfo, (void **) &pIColumnsInfo ));
	XTESTC(hr = pIColumnsInfo->GetColumnInfo(&cColumns, &rgColumnInfo, &pStringsBuffer ));

	//if we don't want the Bookmark Column
	if(!fWantBookmark && cColumns && rgColumnInfo && rgColumnInfo[0].iOrdinal==0)
	{
		//Shift rest of ColumnInfo back over Bookmark column
		//If we don't want the bookmark column
		cColumns--;
		memmove(rgColumnInfo, &rgColumnInfo[1], (size_t)(sizeof(DBCOLUMNINFO)*cColumns));
	}

    // fill out-params
    *pcColumns = cColumns;
    *prgColumnInfo = rgColumnInfo;
    *ppStringsBuffer = pStringsBuffer;
    
CLEANUP:
    SAFE_RELEASE(pIColumnsInfo);
	return hr;
}


////////////////////////////////////////////////////////////////////////
// SetupBindings
//
////////////////////////////////////////////////////////////////////////
HRESULT SetupBindings
	(
	DBORDINAL		cColumns,
	DBCOLUMNINFO*	rgColumnInfo,
	DBTYPE			wBindingType,
	DBCOUNTITEM*	pcBindings,
	DBBINDING**		prgBindings,
	DBLENGTH*		pulRowSize
	)
{
	DBORDINAL i=0;
	DWORD_PTR dwOffset = 0;

    DBCOUNTITEM cBindings = 0;
	DBBINDING* rgBindings = NULL;
	HRESULT hr = S_OK;

    ASSERT(pcBindings && prgBindings);
    ASSERT(pulRowSize);

	//Allocate bindings
	SAFE_ALLOC(rgBindings, DBBINDING, cColumns);
	
	// Create bindings.
	// Bind everything as a WSTR just to keep things simple.
	for(i=0; i<cColumns; i++)
	{
		//Don't bind the bookmark column
		if(rgColumnInfo[i].iOrdinal == 0)
			continue;

		rgBindings[cBindings].obStatus  = dwOffset + offsetof(DATA, sStatus);
 		rgBindings[cBindings].obLength  = dwOffset + offsetof(DATA, ulLength);
		rgBindings[cBindings].obValue   = dwOffset + offsetof(DATA, bValue);
		
		rgBindings[cBindings].dwPart	= DBPART_VALUE | DBPART_LENGTH | DBPART_STATUS;
        rgBindings[cBindings].eParamIO  = DBPARAMIO_NOTPARAM;                              
		rgBindings[cBindings].iOrdinal  = rgColumnInfo[i].iOrdinal;
		rgBindings[cBindings].wType		= wBindingType; 
		rgBindings[cBindings].pTypeInfo = NULL;
		rgBindings[cBindings].pObject	= NULL;
		rgBindings[cBindings].dwMemOwner= DBMEMOWNER_CLIENTOWNED;
		rgBindings[cBindings].pBindExt	= NULL;
		rgBindings[cBindings].dwFlags	= 0;

		rgBindings[cBindings].bPrecision= rgColumnInfo[i].bPrecision;
		rgBindings[cBindings].bScale	= rgColumnInfo[i].bScale;
		rgBindings[cBindings].cbMaxLen  = 0;
						   	
		//May need to adjust the MaxLen, depending upon what the BindingType is
		switch(wBindingType)
		{
			case DBTYPE_VARIANT:
				rgBindings[cBindings].cbMaxLen = sizeof(VARIANT);
				break;

			case DBTYPE_BYTES:
				rgBindings[cBindings].cbMaxLen	= min(rgColumnInfo[i].ulColumnSize, MAX_BUFFER_SIZE);
				break;

			//Strings are kind of a pain.  Although we get the luxury of 
			//Having the provider coerce the type, we need to allocate a buffer 
			//large enough for the provider to store the type in string format
			case DBTYPE_STR:
			case DBTYPE_WSTR:
				//Account for TYPE -> String
				//(make sure enough room in buffer)
				switch(rgColumnInfo[i].wType)
				{
					case DBTYPE_NULL:
					case DBTYPE_EMPTY:
						//Don't need much room for these...
						break;

					case DBTYPE_I1:
					case DBTYPE_I2:
					case DBTYPE_I4:
					case DBTYPE_UI1:
					case DBTYPE_UI2:
					case DBTYPE_UI4:
					case DBTYPE_R4:
						//All of the above fit well into 15 characters of display size
						rgBindings[cBindings].cbMaxLen = 15;
						break;

					case DBTYPE_I8:
					case DBTYPE_UI8:
					case DBTYPE_R8:
					case DBTYPE_CY:
					case DBTYPE_ERROR:
					case DBTYPE_BOOL:
						//All of the above fit well into 25 characters of display size
						rgBindings[cBindings].cbMaxLen = 25;
						break;

					case DBTYPE_DECIMAL:
					case DBTYPE_NUMERIC:
					case DBTYPE_DATE:
					case DBTYPE_DBDATE:
					case DBTYPE_DBTIMESTAMP:
					case DBTYPE_GUID:
						//All of the above fit well into 50 characters of display size
						rgBindings[cBindings].cbMaxLen = 50;
						break;
					
					case DBTYPE_BYTES:
						//Bytes -> String, 1 byte = 2 Ascii chars. (0xFF == "FF")
						rgBindings[cBindings].cbMaxLen	= min(rgColumnInfo[i].ulColumnSize, MAX_BUFFER_SIZE) * 2;
						break;

					case DBTYPE_STR:
					case DBTYPE_WSTR:
						//String -> String
						//cbMaxLen already contains the length, and below we will 
						//account for the NULL terminator as well...
						rgBindings[cBindings].cbMaxLen	= min(rgColumnInfo[i].ulColumnSize, MAX_BUFFER_SIZE);
						break;

					case DBTYPE_VARNUMERIC:
						// Varnumeric columnsize/precision has little bearing on display size
						rgBindings[cBindings].cbMaxLen = 500;
						break;

					default:
 						//For everything else, BSTR, VARIANT, IUNKNOWN, UDT, etc
						//Just default to our largest buffer size
						rgBindings[cBindings].cbMaxLen	= MAX_BUFFER_SIZE;
						break;
				};
				break;

			
			//Currently we only handle binding to STR, WSTR, VARIANT
			default:
				ASSERT(!"Unhandled Binding Type!");
				break;
		};
					
		//Special Handling for other non-oledb defined convertable types to WSTR
		//NOTE: The spec requires all supported types to be converted to 
		//WSTR, but this only applies where the OLE DB conversion is defined.
		//Some are not defined so we need to bind nativly.
		switch(rgColumnInfo[i].wType)
		{
			case DBTYPE_HCHAPTER:
				rgBindings[cBindings].wType		= rgColumnInfo[i].wType;
				rgBindings[cBindings].cbMaxLen  = sizeof(HCHAPTER);
				break;	
		
			case DBTYPE_IUNKNOWN:
			case DBTYPE_IUNKNOWN | DBTYPE_BYREF:
				rgBindings[cBindings].wType		= rgColumnInfo[i].wType;
				rgBindings[cBindings].cbMaxLen  = sizeof(IUnknown*);
				break;

			default:
				//DBTYPE_VECTOR
				//NOTE: Bind Vectors natively so since their is not standard on the 
				//string format.  This way were can persist it in our own "comma-deliminated"
				//format and read it back in as well...
				if(rgColumnInfo[i].wType & DBTYPE_VECTOR)
				{
					rgBindings[cBindings].wType		= rgColumnInfo[i].wType;
					rgBindings[cBindings].cbMaxLen	= sizeof(DBVECTOR);
				}

				//DBTYPE_ARRAY
				if(rgColumnInfo[i].wType & DBTYPE_ARRAY)
				{
					rgBindings[cBindings].wType		= rgColumnInfo[i].wType;
					rgBindings[cBindings].cbMaxLen	= sizeof(SAFEARRAY*);
				}
				break;
		};
					
		//ulColumnSize - is count of Chars for Strings		
		if(rgBindings[cBindings].wType == DBTYPE_WSTR)
			rgBindings[cBindings].cbMaxLen *= 2;

		//Account for the NULL terminator
		if(rgBindings[cBindings].wType == DBTYPE_STR)
			rgBindings[cBindings].cbMaxLen	+= sizeof(CHAR);
		if(rgBindings[cBindings].wType == DBTYPE_WSTR)
			rgBindings[cBindings].cbMaxLen	+= sizeof(WCHAR);

		//MAX_LENGTH
		rgBindings[cBindings].cbMaxLen	= min(rgBindings[cBindings].cbMaxLen, MAX_BUFFER_SIZE);
		dwOffset = rgBindings[cBindings].cbMaxLen + rgBindings[cBindings].obValue;
		dwOffset = ROUND_UP( dwOffset, COLUMN_ALIGNVAL );
		cBindings++;
	}  

CLEANUP:
    //Output args
	*pcBindings		= cBindings;
	*prgBindings	= rgBindings;
	*pulRowSize		= dwOffset;
	return hr;
}



////////////////////////////////////////////////////////////////////////
// CreateAccessor
//
////////////////////////////////////////////////////////////////////////
HRESULT CreateAccessor
	(
	IUnknown*	pIUnkRowset,
	DBCOUNTITEM cBindings,
	DBBINDING*	rgBindings,
	HACCESSOR*	phAccessor 
	)
{
	IAccessor*	pIAccessor = NULL;
	HACCESSOR   hAccessor;
	HRESULT 	hr;


	ASSERT(pIUnkRowset && phAccessor);
	DBBINDSTATUS* rgBindStatus = NULL;
	SAFE_ALLOC(rgBindStatus, DBBINDSTATUS, cBindings);

  	// Get an accessor for our bindings from the rowset, via IAccessor 
	XTESTC(hr = pIUnkRowset->QueryInterface( IID_IAccessor, (void**)&pIAccessor ));
	XTEST(hr = pIAccessor->CreateAccessor( DBACCESSOR_ROWDATA, cBindings, rgBindings, 0, &hAccessor, rgBindStatus));
	if(hr == DB_S_ERRORSOCCURRED || hr == DB_E_ERRORSOCCURRED)
		DisplayAccessorErrors(NULL, cBindings, rgBindings, rgBindStatus);
	TESTC(hr);

	*phAccessor = hAccessor;

CLEANUP:
	SAFE_FREE(rgBindStatus);
	SAFE_RELEASE(pIAccessor);
	return hr;
}


////////////////////////////////////////////////////////////////////////
// DumpAllRows
//
////////////////////////////////////////////////////////////////////////
HRESULT DumpAllRows
	(
	IRowset*		pIRowset,
    DBCOUNTITEM		cBindings,	  	  
    DBBINDING*		rgBindings,
    HACCESSOR		hAccessor,
	DBTYPE   		wBindingType,
	DBLENGTH   		cMaxRowSize,	
	CTable* pCTable
	)
{
	DBCOUNTITEM	cRowsObtained = 0;
	DBCOUNTITEM	iRow;
	DBCOUNTITEM	iLoop=0;
	void*	pData = NULL;
	HROW 	rghRows[MAX_ARGS];
	HROW*	pRows = &rghRows[0];
	HRESULT hr = S_OK;

	WCHAR*			pwszRowURL=NULL;
	IGetRow*		pIGR=NULL;

	DBORDINAL	cColumns = 0;
	DBCOLUMNINFO* rgColumnInfo = NULL;
	WCHAR* pwszStringBuffer = NULL;
	CRowObject RowObject;
	
	ASSERT(pIRowset != NULL);
    ASSERT(rgBindings != NULL);
	
	//Determine if RowObjects are supported...
	BOOL bRowObjects = GetProperty(DBPROP_IRow, DBPROPSET_ROWSET, pIRowset, VARIANT_TRUE);

	// create a buffer for row data, big enough to hold the biggest row
	SAFE_ALLOC(pData, BYTE, cMaxRowSize);

	//Obtain the ColumnInfo
	TESTC(hr = GetColumnsInfo( pIRowset, &cColumns, &rgColumnInfo, &pwszStringBuffer ));

	// process all the rows, 1 row at a time
	while(TRUE)
	{
		XTESTC(hr = pIRowset->GetNextRows(
			NULL,						// hChapter
			0,							// cRowsToSkip
			1,							// cRowsDesired
			&cRowsObtained,             // pcRowsObtained
			&pRows ));					// filled in w/ row handles

		//For the very first row we need to obtain the URL of
		//that row if IGetRow is supported on the rowset. Then
		//dump the URL info, dump query info and dump columns 
		//info, in that order. After doing all this continue 
		//dumping the data of each row one by one.
		if(!iLoop)
		{
			if(VerifyInterface(pIRowset,IID_IGetRow, ROWSET_INTERFACE, (IUnknown**)&pIGR))
				CHECKW(hr = pIGR->GetURLFromHROW(rghRows[0], &pwszRowURL), S_OK);

			TESTC(g_pCTable != NULL)
			TESTC(g_pwszTableName != NULL)

			//Display URLInfo.
			DumpURLInfo(g_pwszRootURL, (pwszRowURL ? pwszRowURL : g_pwszRootURL), g_pwszTableName);
			
			//Display QueryInfo
			TESTC(hr = DumpQueryInfo(g_pCTable, g_pwszDefaultQuery, g_pwszTableName));

			//Display Column Info
			DumpColumnsInfo( cColumns, rgColumnInfo, pCTable);

			SAFE_FREE(rgColumnInfo);
			SAFE_FREE(pwszStringBuffer);
			SAFE_FREE(pwszRowURL);
			SAFE_RELEASE(pIGR);
		}

		if ( cRowsObtained == 0 )			// all done, no more rows left to get
			break;

		// loop over rows obtained, getting data for each
		for ( iRow=0; iRow < cRowsObtained; iRow++ )
		{
			DBCOUNTITEM	cBindingsEx = 0;
			DBBINDING* rgBindingsEx = NULL;
			void*	pDataEx = NULL;

			//Try creating the RowObject for this row...
			if(/*bRowObjects && */SUCCEEDED(hr = RowObject.CreateRowObject(pIRowset, rghRows[iRow])))
			{
				DBLENGTH cRowSizeEx = 0;

				//Obtain the ColumnInfo
				TESTC(hr = GetColumnsInfo( RowObject.pIRow(), &cColumns, &rgColumnInfo, &pwszStringBuffer ));

				//Bind all columns, including any extra columns on the row object
				XTESTC(hr = SetupBindings( cColumns, rgColumnInfo, wBindingType, &cBindingsEx, &rgBindingsEx, &cRowSizeEx));
				
				//Allocate room for data, we can't just use pData since that size is just for the 
				//common rowset columns.  The RowObject could have numerous extra columns and it could
				//be different for every row...
				SAFE_ALLOC(pDataEx, BYTE, cRowSizeEx);

				//IRow::GetColumns
				XTESTC(hr = RowObject.GetColumns(cBindingsEx, rgBindingsEx, pDataEx));

				//Display any errors...
				if(hr == DB_S_ERRORSOCCURRED || hr == DB_E_ERRORSOCCURRED)
					DisplayBindingErrors(NULL, cBindingsEx, rgBindingsEx, pDataEx);
				TESTC(hr);

				//Dump this row of data into the file
				DumpRow(cBindingsEx, rgBindingsEx, pDataEx);
			}
			else
			{
				//IRowset::GetData
				XTEST(hr = pIRowset->GetData(rghRows[iRow], hAccessor, pData ));
			
				//Display any errors...
				if(hr == DB_S_ERRORSOCCURRED || hr == DB_E_ERRORSOCCURRED)
					DisplayBindingErrors(NULL, cBindings, rgBindings, pData);
				TESTC(hr);

				//Dump this row of data into the file
				DumpRow(cBindings, rgBindings, pData);
			}

			//Row Object cleanup...
			ReleaseInputBindingsMemory(cBindingsEx, rgBindingsEx, (BYTE*)pDataEx, TRUE);
			FreeAccessorBindings(cBindingsEx, rgBindingsEx);
			SAFE_FREE(rgColumnInfo);
			SAFE_FREE(pwszStringBuffer);
			RowObject.ReleaseRowObject();
		}
		
		// release row handles
		XTESTC(hr = pIRowset->ReleaseRows( cRowsObtained, rghRows, NULL, NULL, NULL ));
		iLoop++;
	}

CLEANUP:
	// free row data buffer
	SAFE_FREE(rgColumnInfo);
	SAFE_FREE(pwszStringBuffer);
	SAFE_FREE(pwszRowURL);
	SAFE_RELEASE(pIGR);
	SAFE_FREE( pData );
    return hr;
}



//**********************************************************************
// 
// CleanupRowset
// 
// Purpose:
//
//     Allows the rowset to perform any necessary cleanup.
//  
// Parameters:
//
// 	IRowset*	pIRowset    - interface pointer on data provider's Rowset
//                            object
// 	HACCESSOR 	hAccessor   - accessor handle to release
// 
// Return Value:
//
//     S_OK        - Success
//     E_*         - Failure
//     
//     
// Comments:      
//
//     In this app, the only cleanup that the rowset needs to do is
//     release the accessor handle. 
// 
//**********************************************************************

HRESULT CleanupRowset
	(
	IRowset*	pIRowset,
	HACCESSOR 	hAccessor
	)
{
	IAccessor*	pIAccessor = NULL;
	HRESULT		hr;
    
    ASSERT(pIRowset != NULL);
    
	// tell the rowset object it can release the accessor, via IAccessor
	XTESTC(hr = pIRowset->QueryInterface( IID_IAccessor, (void**)&pIAccessor ));
	XTESTC(hr = pIAccessor->ReleaseAccessor( hAccessor, NULL ));
    
CLEANUP:    
	SAFE_RELEASE(pIAccessor);
    return hr;    
}












