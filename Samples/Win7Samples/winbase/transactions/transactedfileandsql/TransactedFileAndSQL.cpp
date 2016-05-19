//---------------------------------------------------------------------
//  This file is part of the Microsoft .NET Framework SDK Code Samples.
// 
//  Copyright (C) Microsoft Corporation.  All rights reserved.
// 
//This source code is intended only as a supplement to Microsoft
//Development Tools and/or on-line documentation.  See these other
//materials for detailed information regarding Microsoft code samples.
// 
//THIS CODE AND INFORMATION ARE PROVIDED AS IS WITHOUT WARRANTY OF ANY
//KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
//IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//PARTICULAR PURPOSE.
//---------------------------------------------------------------------

#define DBINITCONSTANTS

#include "stdio.h"
#include "string.h"
#include "sqloledb.h"
#include "xolehlp.h"
#include "time.h"
#include "transact.h"

// Process 5 rows at a time.
#define NUMROWS_CHUNK				5

IMalloc* g_pIMalloc;

BOOL 		Abort = FALSE;  //if FALSE transaction will be comitted. If TRUE transaction will be aborted.

DWORD		serverNameLen = MAX_COMPUTERNAME_LENGTH+1;
WCHAR		serverName[MAX_COMPUTERNAME_LENGTH+1];
WCHAR*		dbName = L"pubs";
WCHAR*		sqlCommand	 = L"INSERT jobs VALUES ('Test', 10, 100) SELECT COUNT(*) FROM jobs";

void ParseCommandLine(int argc, WCHAR** argv)
{

	BOOL UseLocalServer = TRUE; 
	
	//Parse command line arguments
	for ( int i = 1; i < argc; i++ )
    	{
    		if ( 0 == _wcsnicmp( argv[i], L"-server",7))
		{
			wcsncpy_s(serverName, argv[i+1], MAX_COMPUTERNAME_LENGTH);
			serverName[MAX_COMPUTERNAME_LENGTH] = '\0';
			UseLocalServer = FALSE;
			i++;
		}
		else if ( 0 == _wcsnicmp( argv[i], L"-abort",6))
		{
			Abort = TRUE;
		}
		else if ( 0 == _wcsnicmp( argv[i], L"-help",5))
		{
			wprintf(L"\n\nUsage: %s [-server <servername>] [-abort] \n\n",argv[0]);
			wprintf(L"     -server <servername>\n");
			wprintf(L"     SQL server to connect to. Default is the local machine.\n\n");
			wprintf(L"     -abort\n");
			wprintf(L"     Aborts the transaction at the end. Otherwise by default the\n");
			wprintf(L"     transaction will be comitted.\n\n");
			wprintf(L"     NOTE: All command line parameters are case sensitive.\n\n");
			exit (1);
		}
		else
		{
			wprintf(L"\n\nInvalid parameters.\nType '%s -help' for help.\n\n", argv[0]);
			exit(1);
		}
		
        }

	if (UseLocalServer && !GetComputerNameW(serverName, &serverNameLen))
	{
		wprintf(L"GetComputerNameW() returned 0x%x\n", GetLastError());
		exit(1);
	}

	//Display the configuration information
	wprintf(L"\n---------------------------------\n");
	wprintf(L"(Type '%s -help' for usage)\n\n", argv[0]);
	wprintf(L"CONFIGURATION\n");
	if (Abort == TRUE) 
		wprintf(L"Transaction will be aborted.\n");
	else 
		wprintf(L"Transaction will be comitted.\n");
	wprintf(L"serverName  = %s\n", serverName);
	wprintf(L"dbName      = %s\n", dbName);
	wprintf(L"sqlCommand  = %s\n", sqlCommand);
	wprintf(L"---------------------------------\n\n");

	return;
	
}


HRESULT InitializeDatabase(const WCHAR* srvName, const WCHAR* databaseName, IDBInitialize** ppIDBInitialize)
{
	HRESULT				hr					= E_FAIL;
	IDBInitialize*			pIDBInitialize		= NULL;
	IDBProperties*		pIDBProperties		= NULL;
	DBPROP				InitProperties[4];
	DBPROPSET			rgInitPropSet[1];

	// Create an instance of the data source object.
	hr = CoCreateInstance(CLSID_SQLOLEDB, NULL, CLSCTX_INPROC_SERVER, IID_IDBInitialize, (void**)&pIDBInitialize);
	if (SUCCEEDED(hr))
	{
		//Initialize the property values needed to establish the connection.
		for (int i = 0; i < 4; i++) 
		{
			VariantInit(&InitProperties[i].vValue);
		}

		//Server name.
		InitProperties[0].dwPropertyID		= DBPROP_INIT_DATASOURCE;
		InitProperties[0].vValue.vt			= VT_BSTR;
		InitProperties[0].vValue.bstrVal	= SysAllocString(srvName);
		InitProperties[0].dwOptions			= DBPROPOPTIONS_REQUIRED;
		InitProperties[0].colid				= DB_NULLID;

		//Database.
		InitProperties[1].dwPropertyID		= DBPROP_INIT_CATALOG;
		InitProperties[1].vValue.vt			= VT_BSTR;
		InitProperties[1].vValue.bstrVal	= SysAllocString(databaseName);
		InitProperties[1].dwOptions			= DBPROPOPTIONS_REQUIRED;
		InitProperties[1].colid				= DB_NULLID;
		
		//Using Windows Authentication.
		InitProperties[2].dwPropertyID		= DBPROP_AUTH_INTEGRATED; 
		InitProperties[2].vValue.vt			= VT_BSTR;
		InitProperties[2].vValue.bstrVal	= SysAllocString(L"SSPI");
		InitProperties[2].dwOptions			= DBPROPOPTIONS_REQUIRED;
		InitProperties[2].colid				= DB_NULLID;

		// Construct the DBPROPSET structure(rgInitPropSet). The DBPROPSET structure is used to pass an array of DBPROP 
		// structures (InitProperties) to the SetProperties method.
		rgInitPropSet[0].guidPropertySet	= DBPROPSET_DBINIT;
		rgInitPropSet[0].cProperties		= 4;
		rgInitPropSet[0].rgProperties		= InitProperties;

		//Set initialization properties.
		hr = pIDBInitialize->QueryInterface(IID_IDBProperties, (void **)&pIDBProperties);
		if (SUCCEEDED(hr))
		{
			hr = pIDBProperties->SetProperties(1, rgInitPropSet); 
			if (SUCCEEDED(hr))
			{
				//Now establish the connection to the data source.
				hr = pIDBInitialize->Initialize();
				if (SUCCEEDED(hr))
				{
					*ppIDBInitialize = pIDBInitialize;
				}
				else
				{
					wprintf(L"pIDBProperties->Initialize() returned hr=0x%x\n", hr);
				}
			}
			else
			{
				wprintf(L"pIDBProperties->SetProperties() returned hr=0x%x\n", hr);
			}
			pIDBProperties->Release();
		}
		else
		{
			wprintf(L"QueryInterface(IID_IDBProperties) returned hr=0x%x\n", hr);
		}
	}
	else
	{
		wprintf(L"CoCreateInstance(CLSID_SQLOLEDB, IID_IDBInitialize) returned hr=0x%x\n", hr);
	}
	return hr;
}


//********************************************************************
HRESULT GetDBCreateSession(IDBInitialize* pIDBInitialize, IDBCreateSession** ppIDBCreateSession)
{
	HRESULT				hr					= E_FAIL;
	IDBCreateSession*	pIDBCreateSession	= NULL;

	hr = pIDBInitialize->QueryInterface(IID_IDBCreateSession, (void**) &pIDBCreateSession);
	if (SUCCEEDED(hr))
	{
		*ppIDBCreateSession = pIDBCreateSession;
	}
	else
	{
		wprintf(L"QueryInterface(IID_IDBCreateSession) returned hr=0x%x\n", hr);
	}
	
	return hr;
}

HRESULT CreateTransaction(ITransaction** ppITransaction)
{
	HRESULT hr = E_FAIL;
	ITransactionDispenser* pITransactionDispenser = NULL;
	ITransaction* pITransaction = NULL;
		
	wprintf(L"Creating a transaction...\n");
	//Create a transaction
	hr = DtcGetTransactionManagerEx(NULL, NULL, IID_ITransactionDispenser, OLE_TM_FLAG_NONE, NULL, (void**) &pITransactionDispenser);
	if (FAILED(hr))
	{
		wprintf(L"ERROR: Getting a transaction dispenser object failed. HR=0x%x\n", hr);
		goto cleanup;
	}
	hr = pITransactionDispenser->BeginTransaction(NULL, ISOLATIONLEVEL_READCOMMITTED, ISOFLAG_RETAIN_BOTH, NULL, &pITransaction);
	if (FAILED(hr))
	{
		wprintf(L"ERROR: Begin transaction call failed. HR=0x%x\n", hr);
		goto cleanup;
	}

	*ppITransaction = pITransaction;
	
cleanup:
	
	if(NULL != pITransactionDispenser)
		pITransactionDispenser->Release();
	
	return(hr);
	
}

HRESULT GetKernelTransactionHandle(ITransaction* pITransaction, HANDLE* ppTransactionHandle)
{
	HRESULT hr = E_FAIL;
	IKernelTransaction*	pKernelTransaction	= NULL;
	HANDLE hTransactionHandle = INVALID_HANDLE_VALUE;
	
	wprintf(L"Getting a transaction handle to use with transacted file operation...\n");
	// query for IKernelTransaction interface for a handle to use with transacted file operation
	hr = pITransaction->QueryInterface(IID_IKernelTransaction, (void**) &pKernelTransaction);
	if (FAILED(hr))
	{
		wprintf(L"ERROR: QueryInterface for IKernelTransaction failed with hr=0x%x\n", hr);
		goto cleanup;
	}
	hr = pKernelTransaction->GetHandle(&hTransactionHandle);
	if (FAILED(hr))
	{
		wprintf(L"ERROR: GetHandle call on IKernelTransaction failed with hr=0x%x\n", hr);
		goto cleanup;
	}

	*ppTransactionHandle = hTransactionHandle;

cleanup:

	if(NULL != pKernelTransaction)
		pKernelTransaction->Release();

	return(hr);
	
}

HRESULT GetCoordinatedUniversalTime (char* uniTime)
{
	HRESULT hr = E_FAIL;
	
	struct tm newtime;
	 __int64 ltime;
	char buf[26];
	errno_t err;
	
	//get time
	 _time64( &ltime );
	err = _gmtime64_s( &newtime, &ltime );
	
	if (err)
	{
	   wprintf(L"ERROR: Get time function failed.");
	}
	   
	// Convert to an ASCII representation 
	err = asctime_s(buf, _countof(buf), &newtime);
	
	if (err)
	{
		wprintf(L"ERROR: Converting time to an ASCII representation failed.");
	}
	else
	{
		strcpy_s(uniTime, _countof(buf), buf);
		wprintf(L"Universal time is: %S\n", buf );	
		hr = S_OK;
	}
		
	return(hr);

}

HRESULT TransactedFileOperation(HANDLE hTransactionHandle)
{
	HRESULT hr = E_FAIL;
	HANDLE hAppend = INVALID_HANDLE_VALUE;
	DWORD  dwBytesWritten, dwPos;
	char buf[25];

	wprintf(L"Open file...\n");
	// Open the existing file, or if the file does not exist, create it
	hAppend = CreateFileTransacted(TEXT("test.txt"), // open test.txt
	            FILE_APPEND_DATA,         	// open for writing
	            FILE_SHARE_READ,          	// allow multiple readers
	            NULL,                     	// no security
	            OPEN_ALWAYS,              	// open or create
	            FILE_ATTRIBUTE_NORMAL,    	// normal file
	            NULL,                   	// no attr. template
		     hTransactionHandle,		// pass the transaction handle
		     NULL,			// no mini version
		     NULL);			// reserved

	if (hAppend == INVALID_HANDLE_VALUE)
	{
	   wprintf(L"ERROR: Could not open test.txt.");
	   goto cleanup;
	}

	//any operation done using hAppend handle will now be transactional

	//append the current universal time to the file
	hr = GetCoordinatedUniversalTime(buf);
	if (FAILED(hr)) 
		goto cleanup;
	wprintf(L"Appending the time to the file...\n");
	dwPos = SetFilePointer(hAppend, 0, NULL, FILE_END);
	WriteFile(hAppend, buf, _countof(buf), &dwBytesWritten, NULL);

cleanup:

	if(NULL != hAppend)
		CloseHandle(hAppend);

	return(hr);
	
}

HRESULT CreateTransactedCommand(IDBCreateSession* pIDBCreateSession, ITransaction* pITransaction, const WCHAR* commandText, ICommandText** ppICommandText)
{
	HRESULT				hr						= E_FAIL;
	ITransactionDispenser*	pITransactionDispenser	= NULL;
	ITransactionJoin*		pITransactionJoin		= NULL;
	IDBCreateCommand*	pIDBCreateCommand		= NULL;
	ICommandText*		pICommandText			= NULL;


	hr = pIDBCreateSession->CreateSession(NULL, IID_IDBCreateCommand, (IUnknown**) &pIDBCreateCommand);
	if (SUCCEEDED(hr))
	{
		// Join the transaction.
		hr = pIDBCreateCommand->QueryInterface(IID_ITransactionJoin, (void**) &pITransactionJoin);
		if (SUCCEEDED(hr))
		{
			hr = pITransactionJoin->JoinTransaction((IUnknown*) pITransaction, ISOLATIONLEVEL_READCOMMITTED, 0, NULL);
			if (SUCCEEDED(hr))
			{
				hr = pIDBCreateCommand->CreateCommand(NULL, IID_ICommandText, (IUnknown**) &pICommandText);
				if (SUCCEEDED(hr))
				{
					hr = pICommandText->SetCommandText(DBGUID_DBSQL, commandText);
					if (SUCCEEDED(hr))
					{
						*ppICommandText = pICommandText;
					}
					else
					{
						wprintf(L"pICommandText->SetCommandText(%s) returned hr=0x%x\n", commandText, hr);
					}
				}
				else
				{
					wprintf(L"pIDBCreateCommand->CreateCommand(IID_ICommandText) returned hr=0x%x\n", hr);
				}
			}
			else
			{
				wprintf(L"pITransactionJoin->JoinTransaction(pITransaction) returned hr=0x%x\n", hr);
			}
			pITransactionJoin->Release();
		}
		else
		{
			wprintf(L"QueryInterface(IID_ITransactionJoin) returned hr=0x%x\n", hr);
		}
		pIDBCreateCommand->Release();
	}
	else
	{
		wprintf(L"pIDBCreateSession->CreateSession(IID_IDBCreateCommand) returned hr=0x%x\n", hr);
	}
	return hr;
}

//********************************************************************
HRESULT ExecuteTransactedCommand(IDBCreateSession* pIDBCreateSession, const WCHAR* commandText, ITransaction* pITransaction, IMultipleResults** ppIMultipleResults)
{
	HRESULT				hr						= E_FAIL;
	DBROWCOUNT		dbRowCount				= 0;
	IMultipleResults*		pIMultipleResults		= NULL;
	ICommandText*		pICommandText			= NULL;


	hr = CreateTransactedCommand(pIDBCreateSession, pITransaction, commandText, &pICommandText);
	if (FAILED(hr))
	{
		wprintf(L"CreateTransactedCommand() returned hr=0x%x\n", hr);
		goto cleanup;
	}

	//Execute the SQL command
	wprintf(L"Executing the database query...\n");
	hr = pICommandText->Execute(NULL, IID_IMultipleResults, NULL, &dbRowCount, (IUnknown**) &pIMultipleResults);
	if (FAILED(hr))
	{
		wprintf(L"pICommandText->Execute() returned hr=0x%x\n", hr);
		goto cleanup;
	}

	*ppIMultipleResults = pIMultipleResults;
	
cleanup:
	
	if(NULL != pICommandText)
		pICommandText->Release();

	return hr;

}

HRESULT GetDBBindings(ULONG nCols, DBCOLUMNINFO* pColumnsInfo, DBBINDING** ppDBBindings, char** ppRowValues)
{
	HRESULT			hr			= E_FAIL;
	ULONG			nCol;
	ULONG			cbRow		= 0;
	DBBINDING*		pDBBindings = NULL;
	char*			pRowValues = NULL;

	try {
		
	pDBBindings = new DBBINDING[nCols];
	if (pDBBindings != NULL)
	{
		for (nCol = 0; nCol < nCols; nCol++)
		{
			pDBBindings[nCol].iOrdinal		= nCol+1;
			pDBBindings[nCol].obValue		= cbRow;
			pDBBindings[nCol].obLength		= 0;
			pDBBindings[nCol].obStatus		= 0;
			pDBBindings[nCol].pTypeInfo		= NULL;
			pDBBindings[nCol].pObject		= NULL;
			pDBBindings[nCol].pBindExt		= NULL;
			pDBBindings[nCol].dwPart		= DBPART_VALUE;
			pDBBindings[nCol].dwMemOwner	= DBMEMOWNER_CLIENTOWNED;
			pDBBindings[nCol].eParamIO		= DBPARAMIO_NOTPARAM;
			pDBBindings[nCol].cbMaxLen		= pColumnsInfo[nCol].ulColumnSize;
			pDBBindings[nCol].dwFlags		= 0;
			pDBBindings[nCol].wType			= pColumnsInfo[nCol].wType;
			pDBBindings[nCol].bPrecision	= pColumnsInfo[nCol].bPrecision;
			pDBBindings[nCol].bScale		= pColumnsInfo[nCol].bScale;
			cbRow += pDBBindings[nCol].cbMaxLen;
		}

		if (cbRow > 0)
		{
			pRowValues = new char[cbRow];
			if (pRowValues != NULL)
			{
				*ppDBBindings	= pDBBindings;
				*ppRowValues	= pRowValues;
				hr				= S_OK;
			}
			else
			{
				wprintf(L"pRowValues == NULL.\n");
			}
		}
		else
		{
			wprintf(L"cbRow <= 0.\n");
		}

		if (hr != S_OK)
		{
			delete [] pDBBindings;
		}
	}
	else
	{
		wprintf(L"pDBBindings == NULL.\n");
	}

	}

	catch (...)
	{
		wprintf(L"Exception occured while getting DB bindings.");

		if (NULL != pDBBindings)
			delete [] pDBBindings;
		
		throw;
	}
	
	return hr;
}

void ProcessRowset(IRowset* pIRowset) 
{
	HRESULT			hr;
	ULONG			nCols;
	IColumnsInfo*	pIColumnsInfo	= NULL;
	DBCOLUMNINFO*	pColumnsInfo	= NULL;
	OLECHAR*		pColumnStrings	= NULL;
	ULONG			nCol;
	ULONG			cRowsObtained;
	ULONG			iRow;
	HROW			rghRows[NUMROWS_CHUNK];
	HROW*			pRows			= &rghRows[0];
	IAccessor*		pIAccessor;
	HACCESSOR		hAccessor;
	DBBINDSTATUS*	pDBBindStatus	= NULL;
	DBBINDING*		pDBBindings		= NULL;
	char*			pRowValues;

	hr = pIRowset->QueryInterface(IID_IColumnsInfo, (void**) &pIColumnsInfo);
	if (SUCCEEDED(hr))
	{
		hr = pIColumnsInfo->GetColumnInfo(&nCols, &pColumnsInfo, &pColumnStrings);
		if (SUCCEEDED(hr))
		{
			hr = GetDBBindings(nCols, pColumnsInfo, &pDBBindings, &pRowValues);
			if (SUCCEEDED(hr))
			{
				pDBBindStatus = new DBBINDSTATUS[nCols];
				if (pDBBindStatus != NULL)
				{
					hr = pIRowset->QueryInterface(IID_IAccessor, (void**) &pIAccessor);
					if (SUCCEEDED(hr))
					{
						hr = pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA, nCols, pDBBindings, 0, &hAccessor, pDBBindStatus);
						if (SUCCEEDED(hr))
						{
							// Process all the rows, NUMROWS_CHUNK rows at a time.
							do
							{
								hr = pIRowset->GetNextRows(0, 0, NUMROWS_CHUNK, &cRowsObtained, &pRows);
								if (SUCCEEDED(hr))
								{
									// Loop over rows obtained, getting data for each.
									for (iRow=0; iRow < cRowsObtained; iRow++)
									{
										pIRowset->GetData(rghRows[iRow], hAccessor, pRowValues);
										for (nCol = 0; nCol < nCols; nCol++)
										{
											wprintf(L"COUNT(jobs)=%i\n", pRowValues[pDBBindings[nCol].obValue]);
										}
									}
									pIRowset->ReleaseRows(cRowsObtained, rghRows, NULL, NULL, NULL);
								}
								else
								{
									wprintf(L"pIRowset->GetNextRows() returned hr=0x%x\n", hr);
								}
							}
							while (cRowsObtained > 0);
							pIAccessor->ReleaseAccessor(hAccessor, NULL);
						}
						else
						{
							wprintf(L"pIAccessor->CreateAccessor() returned hr=0x%x\n", hr);
						}
						pIAccessor->Release();
					}
					else
					{
						wprintf(L"pIRowset->QueryInterface(IID_IAccessor) returned hr=0x%x\n", hr);
					}
					delete [] pDBBindStatus;
				}
				else
				{
					wprintf(L"pDBBindStatus == NULL.\n");
					hr = E_FAIL;
				}
				delete [] pDBBindings;
				delete [] pRowValues;
			}
			else
			{
				wprintf(L"GetDBBindings() returned hr=0x%x\n", hr);
			}
			g_pIMalloc->Free(pColumnsInfo);
			g_pIMalloc->Free(pColumnStrings);
		}
		else
		{
			wprintf(L"GetColumnInfo() returned hr=0x%x\n", hr);
		}
		pIColumnsInfo->Release();
	}
	else
	{
		wprintf(L"pIRowset->QueryInterface(IID_IColumnsInfo) returned hr=0x%x\n", hr);
	}
	return;
}

HRESULT DisplaySQLRowSetCount (IMultipleResults* pIMultipleResults)
{
	HRESULT hr = E_FAIL;
	DBROWCOUNT		dbRowCount				= 0;
	IRowset*				pIRowset				= NULL;
	
	
	//Display the SQL data before the transaction commits
	hr = pIMultipleResults->GetResult(NULL, DBRESULTFLAG_DEFAULT, IID_IRowset, &dbRowCount, (IUnknown**) &pIRowset);
	if (hr == S_OK)
	{
		// Because the SQL statement alters the data it is reading and the isolation level is read committed,
		// the data is not available yet.  
		wprintf(L"SQL rowset: ");
		if (pIRowset != NULL)
		{
			ProcessRowset(pIRowset);
			pIRowset->Release();
		}
		else
		{
			wprintf(L"No IRowset yet to process.\n");
		}
	}
	else
	{
		wprintf(L"pIMultipleResults->GetResult failed. HR=0x%x\n", hr);
	}
	
	return(hr);
}

HRESULT CommitOrAbortTransaction(ITransaction* pITransaction, BOOL AbortTransaction)
{
	HRESULT hr = E_FAIL;
	
	if (AbortTransaction == FALSE)
		{
		// Commit the transaction
		wprintf(L"Comitting transaction...\n");
		hr = pITransaction->Commit(FALSE, XACTTC_SYNC_PHASEONE, 0);
		if (FAILED(hr))
			{	
				wprintf(L"ERROR: Commit operation failed. HR=0x%x\n", hr);
			}
		else
			{
				wprintf(L"\nCommit operation succeeded.\n\n");
			}
		}
	else
		{
		//abort the transaction
		wprintf(L"Aborting transaction...\n");
		hr = pITransaction->Abort(NULL, //don't provide a reason for abort
				FALSE,  //must be FALSE
				FALSE); //abort synchronous
		if (FAILED(hr))
			{	
				wprintf(L"ERROR: Abort operation failed. HR=0x%x\n", hr);
			}
		else
			{
				wprintf(L"\nAbort operation succeeded.\n\n");
			}
		}

	return(hr);
	
}

//********************************************************************
int __cdecl wmain(int argc, WCHAR* argv[])
{
	HRESULT				hr					= S_OK;
	IDBCreateSession*	pIDBCreateSession	= NULL;
	IDBInitialize* 			pIDBInitialize 		= NULL;
	IMultipleResults*		pIMultipleResults	= NULL;
	ITransaction*			pITransaction		= NULL;
	HANDLE 				hTransactionHandle 	= INVALID_HANDLE_VALUE;
	

	ParseCommandLine(argc, argv);

	//Initialize COM
	hr = CoInitialize(NULL);
	if (FAILED(hr))
	{
		wprintf(L"CoInitialize(NULL) returned hr=0x%x\n", hr);
		goto cleanup;
	}
	hr = CoGetMalloc(MEMCTX_TASK, &g_pIMalloc);
	if (FAILED(hr))
	{
		wprintf(L"CoGetMalloc(MEMCTX_TASK) returned hr=0x%x\n", hr);
		goto cleanup;
	}

	//Initialize the database
	wprintf(L"Initializing the database...\n");
	hr = InitializeDatabase(serverName, dbName, &pIDBInitialize);
	if (FAILED(hr)) goto cleanup;

	//create a database session
	hr = GetDBCreateSession(pIDBInitialize, &pIDBCreateSession);
	if (FAILED(hr)) goto cleanup;
	
	// Get a pointer to a new transaction
	hr = CreateTransaction(&pITransaction);
	if (FAILED(hr)) goto cleanup;
	
	// Get a transaction handle to use with transacted file operation
	hr = GetKernelTransactionHandle(pITransaction, &hTransactionHandle);

	// Do transacted file operations
	if (SUCCEEDED(hr))
		hr = TransactedFileOperation(hTransactionHandle);
	
	// Do transacted database operations 
	if (SUCCEEDED(hr))
		hr = ExecuteTransactedCommand(pIDBCreateSession, sqlCommand, pITransaction, &pIMultipleResults);

	// If anything after the creation of ITransaction failed, abort the transaction
	if (FAILED(hr))
	{	wprintf(L"Due to errors, transaction will be aborted...\n");
		//force transaction to abort
		hr = CommitOrAbortTransaction(pITransaction, TRUE);
		if (FAILED(hr)) goto cleanup;
	}

	//Display the row count of the SQL table before commit or abort
	DisplaySQLRowSetCount(pIMultipleResults);

	//commit or abort the transaction depending on the '-abort' command line parameter
	hr = CommitOrAbortTransaction(pITransaction, Abort);
	if (FAILED(hr)) goto cleanup;

	//Display the row count of the SQL table after the commit or abort
	DisplaySQLRowSetCount(pIMultipleResults);
			
					
cleanup:
		
		if (NULL != g_pIMalloc) 
			g_pIMalloc->Release();

		if(NULL != hTransactionHandle)
			CloseHandle(hTransactionHandle);

		if(NULL != pIMultipleResults)
			pIMultipleResults->Release();
		if(NULL != pITransaction)
			pITransaction->Release();
		if (NULL != pIDBCreateSession) 
			pIDBCreateSession->Release();
		if (NULL != pIDBInitialize) 
			pIDBInitialize->Release();
	
		CoUninitialize();

	return (hr);
}
