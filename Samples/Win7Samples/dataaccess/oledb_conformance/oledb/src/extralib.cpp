//--------------------------------------------------------------------
// Microsoft OLE DB Test
//	
// Copyright (C) 1995-2000 Microsoft Corporation
//
// @doc  
//
// @module ExtraLib.cpp | This module is a general library for interface testing 
//

#include "MODStandard.hpp"	// Standard headers	

#define  DBINITCONSTANTS	// Must be defined to initialize constants in OLEDB.H
#define  INITGUID

#include "oledb.h"			// OLE DB Header Files
#include "oledberr.h"
#include "transact.h"

#include "privlib.h"		// Private Library
#include "msdasql.h"		// MS (OLE DB) Extra GUIDs (KAGPROP_QUERYBASEDUPDATES)
#include "ExtraLib.h"		// ExtraLib header file



////////////////////////////////////////////////////////////////////////////
// Globals
//
/////////////////////////////////////////////////////////////////////////////
CTable*		g_pTable      = NULL;                      // Global table
CTable*		g_pEmptyTable = NULL;                      // Empty table
CTable*		g_p1RowTable  = NULL;                      // 1 Row Table
DBCOUNTITEM	g_ulNextRow   = 0;							// NextRow

IDBInitialize*    g_pIDBInitialize    = NULL;       //Global DSO ptr
IDBCreateSession* g_pIDBCreateSession = NULL;       //Global DSO ptr
IOpenRowset*      g_pIOpenRowset	  = NULL;       //Global Session ptr

CRITICAL_SECTION  GlobalMutex;                      //Global Mutex

DBID INVALID_DBID;
DBID VALID_INDEXID = DB_NULLID;
GUID NULLGUID = DB_NULLGUID;
DBID NULLDBID = DB_NULLID;


////////////////////////////////////////////////////////////////////////////
//  ModuleInit
//
////////////////////////////////////////////////////////////////////////////
BOOL CommonModuleInit(CThisTestModule* pThisTestModule, REFIID riid, DBCOUNTITEM cRows, EINTERFACE eInterface) 
{
	TBEGIN
    if(pThisTestModule == NULL)
		return FALSE;

	//Initlize the Global Critical section
	InitializeCriticalSection(&GlobalMutex);
	
	//Create the session
	if(!ModuleCreateDBSession(pThisTestModule))
		return FALSE;

	//IDBInitialize interface
	TESTC_(QI(pThisTestModule->m_pIUnknown, IID_IDBInitialize, (void**)&g_pIDBInitialize),S_OK)
	//IDBCreateSession interface
	TESTC_(QI(pThisTestModule->m_pIUnknown, IID_IDBCreateSession, (void**)&g_pIDBCreateSession),S_OK)
	
	//Session IUnknown
	TESTC_(QI(pThisTestModule->m_pIUnknown2, IID_IOpenRowset, (void**)&g_pIOpenRowset),S_OK)
	
	//Create all Global Tables    
	TESTC(CreateTable(&g_pTable, cRows))
	TESTC(CreateTable(&g_pEmptyTable, NO_ROWS))
	TESTC(CreateTable(&g_p1RowTable, ONE_ROW))

	//Indicate NextRow in table
	g_ulNextRow = g_pTable->GetRowsOnCTable() + 10;
	
	//Make sure interface is supported, can't do any testing without it
	if(riid != IID_IUnknown)
		TESTC_PROVIDER(SupportedInterface(riid, eInterface));

CLEANUP:
    TRETURN
}


////////////////////////////////////////////////////////////////////////////
//  ModuleTerminate
//
////////////////////////////////////////////////////////////////////////////

BOOL CommonModuleTerminate(CThisTestModule * pThisTestModule) 
{
    if(pThisTestModule == NULL)
		return FALSE;
    
	SAFE_RELEASE(g_pIDBInitialize);
	SAFE_RELEASE(g_pIDBCreateSession);
	SAFE_RELEASE(g_pIOpenRowset);
	
	//Drop all the Global tables
	DropTable(&g_pTable);
	DropTable(&g_pEmptyTable);
	DropTable(&g_p1RowTable);

	//Delete the Global Critical section
	DeleteCriticalSection(&GlobalMutex);
	return ModuleReleaseDBSession(pThisTestModule);
}


//////////////////////////////////////////////////////////////////////////////
//  Memory Alloc/Free Routines
//
//  A "safe" wrapper arround CoTaskMemAlloc to protect boundaries
//////////////////////////////////////////////////////////////////////////////  

void* BoundarySafeAlloc(ULONG cb) 
{
	//no-op case
	if(cb==0) return NULL;
    
	//A "safe" wrapper arround CoTaskMemAlloc to protect boundaries
	// [head | size | buffer | tail]   - Current memory layout
    TBEGIN

    BYTE*  pHead   = NULL;
    ULONG* pSize   = NULL;
    BYTE*  pBuffer = NULL;
    BYTE*  pTail   = NULL;

	//Alloc buffer with padding at head and tail
	// [head | size | buffer | tail]
	pHead = (BYTE*)PROVIDER_ALLOC(1 + 3 + sizeof(ULONG) + cb + 1);
	TESTC(pHead!=NULL)

	pSize   = (ULONG*)(pHead + sizeof(ULONG));  //pointer to the size
	pBuffer = (BYTE*)pSize + sizeof(ULONG);     //pointer to the actual buffer
	pTail   = pBuffer + cb;			            //pointer to the tail signature
	
	//insert signature into padding
	*pHead = '{'; //head signature
	*pSize = cb;  //size
	*pTail = '}'; //tail signature
	
	//insert alloc-signature into buffer
    //So a PROVIDER_ALLOC(5) => "{aaaaa}"
    memset(pBuffer, 'a', cb);				
        
CLEANUP:
    //return the non-padded buffer
	return pBuffer;
}

////////////////////////////////////////////////////////////////////////////
//  SafeFree
//
////////////////////////////////////////////////////////////////////////////

void BoundarySafeFree(void* pv)
{
    //no-op case
	if(!pv) return;
	
	// [head | size | buffer | tail]

	BYTE*  pBuffer = (BYTE*)pv;	                        //pointer to the actual buffer
	ULONG* pSize   = (ULONG*)(pBuffer - sizeof(ULONG)); //pointer to the size
	BYTE*  pHead   = (BYTE*)pSize - sizeof(ULONG);      //pointer to the head signature
	BYTE*  pTail   = pBuffer + *pSize;                  //pointer to the tail signature

	//Verify head and tail signature
	ASSERT(*pHead=='{' && *pTail=='}');
		
	//insert free-signature into buffer
    //So a PROVIDER_FREE(pv) => "{zzzzz}"
    memset(pBuffer,'z',*pSize);				

	//Free the entire buffer + padding
	PROVIDER_FREE(pHead);
}
 
////////////////////////////////////////////////////////////////////////////
//  SafeFree
//
////////////////////////////////////////////////////////////////////////////

void BoundarySafeFree(ULONG cBuffers, void** ppv)
{
	//no-op case
	if(cBuffers==0 || ppv==NULL)
		return;

	//delagate to other fuction
	for(ULONG i=0; i<cBuffers; i++)
		BoundarySafeFree(ppv[i]);
}



////////////////////////////////////////////////////////////////////
// General Misc Helper Functions
//
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
//  FreeAccessorBuffer
//
////////////////////////////////////////////////////////////////////////////
BOOL FreeAccessorBufferAndBindings(DBCOUNTITEM *pcBindings, DBBINDING **prgBindings, void **ppData, bool bFreeBuffer) {
	TBEGIN
	DBCOUNTITEM				i,
							cBindings		=	*pcBindings;
	DBBINDING				*rgBindings		=	*prgBindings;
	void					*pData			=	*ppData;

	TESTC(pData != NULL);

	for(i=0; i<cBindings; i++) {
		if (VALUE_IS_BOUND(rgBindings[i]) && rgBindings[i].wType == DBTYPE_VARIANT)
			VariantClear((VARIANTARG*) &((char*)pData)[rgBindings[i].obValue]);
		PROVIDER_FREE(rgBindings[i].pObject);
	}

	PROVIDER_FREE(*prgBindings);
	if (bFreeBuffer)
		PROVIDER_FREE(*ppData);
	*pcBindings		=	0;

CLEANUP:
	TRETURN
}



////////////////////////////////////////////////////////////////////////////
//  CreateTable
//
////////////////////////////////////////////////////////////////////////////
BOOL CreateTable(CTable** ppTable, DBCOUNTITEM cRows)
{
	//Function to new/create a table of size, cRows
	ASSERT(ppTable);
	TBEGIN

	//Instaniate the Table
	CTable* pCTable = new CTable(g_pIOpenRowset,L"EXTRA");
	QTESTC(pCTable != NULL)

	//Create the table
	//With the index column updateable, (TRUE)
	TESTC_(pCTable->CreateTable(cRows,COL_ONE,NULL,PRIMARY,TRUE),S_OK);

	//Output the TableName...
	PRVTRACE(L"TableName = %s, Columns = %d, Rows = %d\n", pCTable->GetTableName(), pCTable->CountColumnsOnTable(), pCTable->GetRowsOnCTable());

CLEANUP:
	*ppTable = pCTable;
	TRETURN
}


////////////////////////////////////////////////////////////////////////////
//  TableInsert
//
////////////////////////////////////////////////////////////////////////////
HRESULT TableInsert(DBCOUNTITEM cRowsToInsert, CTable* pCTable)
{
	TBEGIN
	HRESULT hr = S_OK;

	if(pCTable == NULL)
		return E_FAIL;
	
	//Insert the specified number of rows into the table...
	QTESTC_(hr = pCTable->Insert(g_ulNextRow, PRIMARY, TRUE, NULL, FALSE, cRowsToInsert), S_OK);
	
CLEANUP:
	g_ulNextRow += cRowsToInsert;
	return hr;
}


////////////////////////////////////////////////////////////////////////////
//  DropTable
//
////////////////////////////////////////////////////////////////////////////

BOOL DropTable(CTable** ppTable)
{
	TBEGIN
	ASSERT(ppTable);

	//no-op case, no table to drop
	if(*ppTable == NULL)
		TRETURN

	//Otherwise drop it
	TESTC_((*ppTable)->DropTable(),S_OK)
	
CLEANUP:
	SAFE_DELETE(*ppTable)
	TRETURN
}

////////////////////////////////////////////////////////////////////////////
//  QI
//
////////////////////////////////////////////////////////////////////////////
HRESULT QI(IUnknown* pExistingIUnknown, REFIID riid, void** ppRequestedIUnknown) 
{             
    IUnknown* pIUnknown = NULL;

	//null output args
	if(ppRequestedIUnknown)
		*ppRequestedIUnknown = NULL;

    //Invalidarg check
	if(pExistingIUnknown==NULL)
		return E_INVALIDARG;
    
    //Query for the desired interface
    HRESULT hr = pExistingIUnknown->QueryInterface(riid, (void**)&pIUnknown);

	//Quickly verify the results.
	//NOTE: We don't call VerifyOutputInterface since it calls this method!
	if(SUCCEEDED(hr))
	{
		TESTC(pIUnknown != NULL);

		//if user wants interface then return it, otherwise release it.
		if(ppRequestedIUnknown)
			*ppRequestedIUnknown = pIUnknown;
		else
	        SAFE_RELEASE(pIUnknown);
	}
	else
	{
		TESTC(pIUnknown == NULL);
	}

CLEANUP:
    return hr;
}


////////////////////////////////////////////////////////////////////////////
//  VerifyArray
//
////////////////////////////////////////////////////////////////////////////
BOOL VerifyArray(DBCOUNTITEM cEle, ULONG* rgEle, ULONG scode)
{   
    //loop through the array of elements, and compare to scode
    for(ULONG i=0; i<cEle; i++)
	{
		if(rgEle[i] != scode)
			return FALSE;
	}

	return TRUE;
}


////////////////////////////////////////////////////////////////////////////
//  Verify that it is a valid interface
//
////////////////////////////////////////////////////////////////////////////

BOOL ValidInterface(IID riid, IUnknown* pIUnknown)
{
    TBEGIN
	if(pIUnknown==NULL)
		return FALSE;

	//Try to actaully use it!

    //IID_IRowset
    //IID_IRowsetLocate
	if(riid == IID_IRowset || riid == IID_IRowsetLocate)
		TEST2C_(((IRowset*)pIUnknown)->RestartPosition(NULL), S_OK, DB_S_COMMANDREEXECUTED)
    
    //IID_IRowsetInfo
	else if(riid == IID_IRowsetInfo)
		TESTC_(((IRowsetInfo*)pIUnknown)->GetProperties(0, NULL, 0, NULL), E_INVALIDARG)

	//IID_IRowsetUpdate
	else if(riid == IID_IRowsetUpdate)
		TESTC_(((IRowsetUpdate*)pIUnknown)->Update(NULL,FIVE_ROWS,NULL,NULL,NULL,NULL),E_INVALIDARG)
    
    //IID_IRowsetChange
	else if(riid == IID_IRowsetChange)
	{
		HRESULT hr = ((IRowsetChange*)pIUnknown)->DeleteRows(NULL,FIVE_ROWS,NULL,NULL);
		TESTC(hr==E_INVALIDARG || hr==DB_E_NOTSUPPORTED)
	}
	//IID_IRowsetOpenRowset
  else if(riid == IID_IOpenRowset)
	{
        TESTC_(((IOpenRowset*)pIUnknown)->OpenRowset(NULL,NULL,NULL,riid,0,NULL,NULL),E_INVALIDARG)
	}
	//IID_IGetDataSource
  else if(riid == IID_IGetDataSource)
	{
        TESTC_(((IGetDataSource*)pIUnknown)->GetDataSource(riid,NULL),E_INVALIDARG)
	}
	//IConnectionPoints
  else if(riid == IID_IRowsetNotify || riid == IID_IDBAsynchNotify)
	{
        TESTC_(((IConnectionPoint*)pIUnknown)->Advise(NULL, NULL),E_POINTER)
	}
	//IID_IUnknown
	else
	{
		//All interfaces are IUnknown
		TESTC_(QI(pIUnknown, riid),S_OK)
	}

CLEANUP:
    TRETURN
}

////////////////////////////////////////////////////////////////////////////
//  Test to see if the interface is supported
//
////////////////////////////////////////////////////////////////////////////

BOOL SupportedInterface(IID riid, EINTERFACE eInterface)
{
    TBEGIN
	HRESULT						hr = S_OK;
	ULONG						cPropSets = 0;
	DBPROPSET*					rgPropSets = NULL;
	IConnectionPointContainer*	pIConnectionPointContainer = NULL;
	IOpenRowset*				pIOpenRowset = NULL;
	IRowset*					pIRowset = NULL;
	IGetRow*					pIGetRow = NULL;
    IUnknown*					pIUnknown = NULL;

	DBCOUNTITEM	cRowsObtained	= 0;
	HROW		*rghRows		= NULL;

	//Try to obtain the desired interface
	//TODO instead of hard coding all of this, should just pass in
	//wither the interface is DATASOURCE/SESSION/COMMAND/ROWSET
	//and see if we can get the required interface.  But this will require
	//all tests to change since they all call this...

	if( riid==IID_IRowset || 
		riid==IID_IRowsetLocate ||
		riid==IID_IRowsetUpdate ||
		riid==IID_IRowsetChange ||
		riid==IID_IGetRow)
    {
		ASSERT(g_pTable);
		QTESTC_(hr = g_pTable->CreateRowset(USE_OPENROWSET, riid, 0, NULL, (IUnknown**)&pIUnknown),S_OK);
    }
	else if( riid==IID_IRow ||
			 riid==IID_IRowChange ||
			 riid==IID_IRowSchemaChange)
    {
		ASSERT(g_pTable);
		hr = E_FAIL;

		switch(eInterface)
		{
			case ROW_INTERFACE:
				//To see if row objects exists, we can't do a singleton select, since thats not 
				//a requirement if row objects are supported.  
				//(see: DBPROP_OLEOBJECTS with DBPROPVAL_OO_ROWOBJECT and DBPROPVAL_OO_SINGLETON)
				if(FAILED(hr = g_pTable->CreateRowset(USE_OPENROWSET, riid, 0, NULL, (IUnknown**)&pIUnknown)))
				{
					//Create the rowset first...
					TESTC_(hr = g_pTable->CreateRowset(USE_OPENROWSET, IID_IRowset, 0, NULL, (IUnknown**)&pIRowset),S_OK);
				
					//Currently we know if row objects are supported, but not this interface.
					TESTC_(hr = pIRowset->GetNextRows(0, 0, 1, &cRowsObtained, &rghRows),S_OK);
					QTESTC(VerifyInterface(pIRowset, IID_IGetRow, ROWSET_INTERFACE, (IUnknown**)&pIGetRow));
					
					//Determine if this row object interface is supported...
					TEST3C_(hr = pIGetRow->GetRowFromHROW(NULL, rghRows[0], riid, (IUnknown**)&pIUnknown),S_OK,DB_S_NOROWSPECIFICCOLUMNS,E_NOINTERFACE);
					QTESTC(hr==S_OK || hr==DB_S_NOROWSPECIFICCOLUMNS);
					
					//Release the row handle
					TESTC_(hr = pIRowset->ReleaseRows(cRowsObtained, rghRows, NULL, NULL, NULL),S_OK);
				}
				break;

			default:
				ASSERT(!L"Unhandled Case!");
				QTEST(FALSE);
				break;
		}
    }
    else if( riid==IID_IRowsetNotify || riid==IID_IDBAsynchNotify || riid==IID_IConnectionPointContainer)
    {
		//Obtain the correct connection point container
		switch(eInterface)
		{
			case ROWSET_INTERFACE:
				ASSERT(g_pTable);
				QTESTC_(hr = g_pTable->CreateRowset(USE_OPENROWSET, IID_IConnectionPointContainer,0,NULL,(IUnknown**)&pIConnectionPointContainer),S_OK);
				break;

			case ROW_INTERFACE:
				ASSERT(g_pTable);
				QTESTC(SetSettableProperty(DBPROP_IRow, DBPROPSET_ROWSET, &cPropSets, &rgPropSets));
				hr = g_pTable->CreateRowset(USE_OPENROWSET, IID_IConnectionPointContainer, cPropSets, rgPropSets, (IUnknown**)&pIConnectionPointContainer);
				QTESTC(hr==S_OK || hr==DB_S_NOTSINGLETON);
				break;

			case DATASOURCE_INTERFACE:
				ASSERT(g_pIDBInitialize);
				QTESTC_(hr = QI(g_pIDBInitialize, IID_IConnectionPointContainer, (void**)&pIConnectionPointContainer),S_OK);
				break;

			case STREAM_INTERFACE:
				//TODO
				hr = E_NOINTERFACE;
				break;
			default:
				ASSERT(!L"Unhandled Case!");
				QTEST(FALSE);
				break;
		}
		
		if (riid!=IID_IConnectionPointContainer)		
			//Try to find the requested connection point
    	QTESTC_(hr = pIConnectionPointContainer->FindConnectionPoint(riid, (IConnectionPoint**)&pIUnknown),S_OK)
		else
		{
			pIUnknown = pIConnectionPointContainer;
			pIConnectionPointContainer = NULL;
		}
	}
    else if(riid==IID_IOpenRowset ||
			riid==IID_IGetDataSource ||
			riid==IID_IAlterTable)
    {
		QTESTC(VerifyInterface(g_pIOpenRowset, riid, SESSION_INTERFACE, (IUnknown**)&pIUnknown));
    }
    else 
	{
		//Needs to have another interface added...
		QTESTC(FALSE);
	}
    
    //See if it really can be used
    if(SUCCEEDED(hr) && pIUnknown) 
        QTESTC(ValidInterface(riid,pIUnknown))

CLEANUP:
	FreeProperties(&cPropSets, &rgPropSets);
	SAFE_RELEASE(pIConnectionPointContainer);
    SAFE_RELEASE(pIOpenRowset);
	SAFE_RELEASE(pIRowset);
    SAFE_RELEASE(pIUnknown);
	SAFE_RELEASE(pIGetRow);
	SAFE_FREE(rghRows);
	TRETURN;
}


////////////////////////////////////////////////////////////////////////////
//  VerifyOutputInterface
//
////////////////////////////////////////////////////////////////////////////
BOOL VerifyOutputInterface(HRESULT hr, REFIID riid, IUnknown** ppIUnknown)
{
	TBEGIN

	//The following code is always needed whenever calling a method that
	//returns an interface pointer and an HRESULT.  This is fairly trival code
	//but keeps all this common checking in one location
	
	//Verify Results	
	if(SUCCEEDED(hr))
	{
		//Verify Valid...
		if(ppIUnknown)
			TESTC(*ppIUnknown != NULL);

		//Verify returned pointer supports the interface...
		TESTC_(QI(*ppIUnknown, riid),S_OK);
	}
	else
	{
		//Verify NULL
		if(ppIUnknown)
			TESTC(*ppIUnknown == NULL);
	}

CLEANUP:
	TRETURN
}


//////////////////////////////////////////////////////////////////
// WaitForThread
//
//////////////////////////////////////////////////////////////////
DWORD WaitForThreads(ULONG cThreads, LPHANDLE rghThread)
{ 
	// Added 12/7/98 by akimball.  We need a message loop here to comply with apartment model
	// rules.  If a provider is marked "Both" in the registry, it should send notifications to
	// listeners on the same thread that was used to Advise.  In order to receive these
	// marshalled notifications, the listener threads(s) need to poll their message loop(s).
	MSG msg;
	DWORD dwRet = WAIT_OBJECT_0;
	ULONG  cEvents = cThreads;
	HANDLE rghEvents[MAXIMUM_WAIT_OBJECTS];
	
	//The maximum number of object handles is MAXIMUM_WAIT_OBJECTS minus one. 
	//MsgWaitForMultipleObjects has this limitiation, so we can make use of a static array as well...
	ASSERT(cThreads < MAXIMUM_WAIT_OBJECTS);
	memcpy(rghEvents, rghThread, sizeof(HANDLE)*cThreads);

	while(cEvents)
	{
		ULONG iEvent = cEvents;
				
		//Wait for an event or message
		dwRet = MsgWaitForMultipleObjects(
					cEvents,				// Number of events to wait for
					rghEvents,				// The array of events
					FALSE,					// wait for all or wait for one
					INFINITE,				// Timeout value
					QS_ALLINPUT);			// Any message wakes up
		
		// There is a window message available. Dispatch it.
		if (dwRet == WAIT_OBJECT_0 + cEvents)	
		{
			while (PeekMessage(&msg,NULL,NULL,NULL,PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else if(dwRet < WAIT_OBJECT_0 + cEvents)
		{
			//One of the events were signled.
			iEvent = dwRet - WAIT_OBJECT_0;
		}
		else
		{
			//Something else (bad)...
			DWORD dwLastError = GetLastError();
			break;
		}

		//One of the events were signled.
		if(iEvent < cEvents)
		{
			//NOTE: MsgWaitForMultipleObjects works great if you only have to wait for one thread.
			//But if you need to wait for multiple threads, you can't just use "TRUE - wait for all events"
			//since it never actually returns until all events are completed, which doesn't help
			//if you need to actually pump COM messges for apartment model threads.  And passing "FALSE"
			//stops as soon as one thread is complete, not similar to WaitForMultipleObjects symmantics.

			//So basically what we do is wait until one of the threads is complete (or singled).  
			//If complete (we get here), then we just remove it from the possible events to wait for
			//and continue this process until all events are done and all messaages are pumped...
			
			//We need to compact the events (except for the last one)
			if((iEvent+1) < cEvents)
				memmove(&rghEvents[iEvent], &rghEvents[iEvent+1], (cEvents-(iEvent+1))*sizeof(HANDLE));
			cEvents--;
		}
	}

	//Sometimes MsgWait seems to exit prematurly, for some reason
	//Truely make sure all events have completed before returning control...
	WaitForMultipleObjects(
					cThreads,				// Number of events to wait for
					rghThread,				// The array of events
					TRUE,					// wait for all or wait for one
					INFINITE				// Timeout value
					);
	return dwRet;
}


//////////////////////////////////////////////////////////////////
// VerifyThreadingModel
//
// Looks up in the registry a particular value and verifys it
//////////////////////////////////////////////////////////////////
BOOL VerifyThreadingModel(CLSID clsidProv, WCHAR* pwszThreadingModel, CLSCTX clsctx)
{
	TBEGIN
	WCHAR* pwszCLSID = NULL;
	WCHAR  wszBuffer[MAX_QUERY_LEN];
	HRESULT hr = S_OK;

	/*
		However, an in-proc (DLL-based) COM server does not call CoInitialize/CoInitializeEx 
		because those APIs will have been called by the time the DLL server is loaded. 
		Therefore, a DLL server must use the registry to inform COM of the threading model it 
		supports so that COM can ensure that the system operates in a way that is compatible with it. 
		A named value of the component's CLSID\InprocServer32 key called ThreadingModel is used for 
		this purpose as follows: 

		ThreadingModel value not present: Supports single-threading model. 
		ThreadingModel=Apartment : Supports STA model. 
		ThreadingModel=Both : Supports STA and MTA model. 
		ThreadingModel=Free : Supports only MTA. 

		NOTE: ThreadingModel is a named value, not a subkey of InprocServer32 as incorrectly documented in some earlier versions of the Win32 documentation. 
	*/

	//From the Above we only need to check to make sure that InprocServer32 objects have the 
	//correct ThreadingModel for the testing.  For other out-of-proc the DLL internally calls 
	//CoInitialize(Ex) with the threading model, so their is no key to check, and currently
	//without the key I know no way to querying this info.


	// CLSID\{clsid}\InprocServer32
	if(clsctx & CLSCTX_INPROC_SERVER)
	{
		QTESTC_(hr = StringFromCLSID(clsidProv, &pwszCLSID), S_OK);
		swprintf(wszBuffer, L"CLSID\\%s\\InprocServer32", pwszCLSID);
	
		//Obtain the Value for ThreadModel	
		QTESTC_(hr = GetRegEntry(HKEY_CLASSES_ROOT, wszBuffer, L"ThreadingModel", wszBuffer, MAX_QUERY_LEN),S_OK)
		
		//Verify Correct threading model
		//This is case-insensitive
		QTESTC(_wcsicmp(wszBuffer, pwszThreadingModel)==EQUAL_STR)
	}
	else
	{
		//Either LocalServer32, LocalService, LocalHandler32, or something else...
		//These do not have a ThreadingModel key and do not need it due to the above explaination.
		//So for these providers and objects, about all that we can do is indicate a warning
		//if we are trying to use them in a free threaded environment and we can't find the key...
		TOUTPUT("Provider is not INPROC_SERVER, so there is no way to verify ThreadingModel");
		TOUTPUT("This module requires an object that supports a ThreadingModel=" << pwszThreadingModel);
		TOUTPUT(" for this module to operate correctly.");
	}

CLEANUP:
	PROVIDER_FREE(pwszCLSID);
	TRETURN;
}


/////////////////////////////////////////////////////////////////////////////
// IsUnicodeOS
//
/////////////////////////////////////////////////////////////////////////////
BOOL IsUnicodeOS()
{
	static BOOL fInitialized = FALSE;
	static BOOL fUnicode = TRUE;

	//NOTE:  Don't call another other helper functions from within this function
	//that might also use the IsUnicodeOS flag, otherwise this will be an infinite loop...
	
	if(!fInitialized)
	{
		HKEY hkJunk = HKEY_CURRENT_USER;

		// Check to see if we have win95's broken registry, thus we do not have Unicode support
		if((RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE", 0, KEY_READ, &hkJunk) == S_OK) && hkJunk == HKEY_CURRENT_USER)
		{
			// Try the ANSI version
			if((RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE", 0, KEY_READ, &hkJunk) == S_OK) && (hkJunk != HKEY_CURRENT_USER))
			{
				fUnicode = FALSE;
			}
		}

		if(hkJunk != HKEY_CURRENT_USER)
			RegCloseKey(hkJunk);
		fInitialized = TRUE;
	}

	return fUnicode;
}


////////////////////////////////////////////////////////
// HRESULT CreateRegKey
//
////////////////////////////////////////////////////////
HRESULT CreateRegKey(HKEY hRootKey, WCHAR* pwszKeyName, HKEY* phKey, REGSAM samDesired)
{
	HRESULT hr = E_FAIL;
	ULONG dwDisposition = 0;

	//Need the name of the key to open
	if(!pwszKeyName)
		return E_FAIL;
	
	if(IsUnicodeOS())
	{
		hr = RegCreateKeyExW(hRootKey, pwszKeyName, 0, NULL, REG_OPTION_NON_VOLATILE, samDesired, NULL, phKey, &dwDisposition);
	}
	else
	{
		CHAR szBuffer[MAX_NAME_LEN];
		ConvertToMBCS(pwszKeyName, szBuffer, MAX_NAME_LEN);

		hr = RegCreateKeyExA(hRootKey, szBuffer, 0, NULL, REG_OPTION_NON_VOLATILE, samDesired, NULL, phKey, &dwDisposition);
	}

	if(hr != S_OK)
		hr = E_FAIL;
	return hr;
}


////////////////////////////////////////////////////////
// HRESULT OpenRegKey
//
////////////////////////////////////////////////////////
HRESULT OpenRegKey(HKEY hRootKey, WCHAR* pwszKeyName, DWORD ulOptions, REGSAM samDesired, HKEY* phKey)
{
	HRESULT hr = E_FAIL;

	if(IsUnicodeOS())
	{
		//Obtain the Key for HKEY_CLASSES_ROOT\"SubKey"
		hr = RegOpenKeyExW(hRootKey, pwszKeyName, ulOptions, samDesired, phKey);
	}
	else
	{
		CHAR szBuffer[MAX_NAME_LEN];
		ConvertToMBCS(pwszKeyName, szBuffer, MAX_NAME_LEN);

		//Obtain the Key for HKEY_CLASSES_ROOT\"SubKey"
		hr = RegOpenKeyExA(hRootKey, szBuffer, ulOptions, samDesired, phKey);
	}

	if(hr != S_OK)
		return E_FAIL;
	return hr;
}


////////////////////////////////////////////////////////
// WCHAR* GetProgID
//
////////////////////////////////////////////////////////
WCHAR* GetProgID(REFCLSID clsid)
{
	WCHAR* pwszProgID = NULL;
	WCHAR wszBuffer[MAX_NAME_LEN] = {0};

	//ProgID From the Sprecified CLSID
	if(FAILED(ProgIDFromCLSID(clsid, &pwszProgID)))
	{
		//If that does work, we will just return the 
		//String representation of the GUID
		StringFromGUID2(clsid, wszBuffer, MAX_NAME_LEN);
		pwszProgID = wcsDuplicate(wszBuffer);
	}

	return pwszProgID;
}


////////////////////////////////////////////////////////
// HRESULT GetRegEnumKey
//
////////////////////////////////////////////////////////
HRESULT GetRegEnumKey(HKEY hRootKey, WCHAR* pwszKeyName, DWORD dwIndex, WCHAR* pwszSubKeyName, ULONG cBytes)
{
	HRESULT hr = E_FAIL;
	HKEY hKey = NULL;

	//Need some place to put the key name returned!
	if(!pwszSubKeyName)
		return E_FAIL;
	
	//Obtain the Key for HKEY_CLASSES_ROOT\"KeyName"
	if(pwszKeyName)
	{
		if(FAILED(OpenRegKey(hRootKey, pwszKeyName, 0, KEY_READ, &hKey)))
			goto CLEANUP;
	}
	
	if(IsUnicodeOS())
	{
		//Obtain the specified RegItem at the index specified
		hr = RegEnumKeyW(hKey ? hKey : hRootKey, dwIndex, pwszSubKeyName, cBytes);
	}
	else
	{
		CHAR szBuffer[MAX_NAME_LEN] = {0};
		
		//Obtain the specified RegItem at the index specified
		hr = RegEnumKeyA(hKey ? hKey : hRootKey, dwIndex, szBuffer, MAX_NAME_LEN);
		ConvertToWCHAR(szBuffer, pwszSubKeyName, cBytes);
	}
	

CLEANUP:
	if(hr != S_OK && hr != ERROR_NO_MORE_ITEMS)
		hr = E_FAIL;
	CloseRegKey(hKey);
	return hr;
}


////////////////////////////////////////////////////////
// HRESULT GetRegEnumValue
//
////////////////////////////////////////////////////////
HRESULT GetRegEnumValue(HKEY hRootKey, WCHAR* pwszKeyName, DWORD dwIndex, WCHAR** ppwszValueName)
{
	HRESULT hr = S_OK;
	ULONG cBytes = 0;
	HKEY hKey = NULL;
	ULONG cMaxValueChars = 0;

	//Obtain the Key for HKEY_CLASSES_ROOT\"KeyName"
	if(pwszKeyName)
	{
		if(FAILED(OpenRegKey(hRootKey, pwszKeyName, 0, KEY_READ, &hKey)))
			goto CLEANUP;
	}

	//First obtain the length of the Value...
	if(S_OK == RegQueryInfoKey(hKey ? hKey : hRootKey, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &cMaxValueChars, NULL, NULL, NULL))
	{
		//Alloc a buffer large enough...
		SAFE_ALLOC(*ppwszValueName, WCHAR, cMaxValueChars+1);
		(*ppwszValueName)[0] = L'\0';

		//Now obtain the data...
		cBytes = (cMaxValueChars+1)*sizeof(WCHAR);
		hr = GetRegEnumValue(hRootKey, pwszKeyName, dwIndex, *ppwszValueName, &cBytes);
	}
	
CLEANUP:
	CloseRegKey(hKey);
	return hr;
}


////////////////////////////////////////////////////////
// HRESULT GetRegEnumValue
//
////////////////////////////////////////////////////////
HRESULT GetRegEnumValue(HKEY hRootKey, WCHAR* pwszKeyName, DWORD dwIndex, WCHAR* pwszValueName, ULONG* pcBytes)
{
	HRESULT hr = E_FAIL;
	HKEY hKey = NULL;
	ASSERT(pcBytes);

	//Obtain the Key for HKEY_CLASSES_ROOT\"KeyName"
	if(pwszKeyName)
	{
		if(FAILED(OpenRegKey(hRootKey, pwszKeyName, 0, KEY_READ, &hKey)))
			goto CLEANUP;
	}

	if(IsUnicodeOS())
	{
		//Obtain the specified RegItem at the index specified
		hr = RegEnumValueW(hKey ? hKey : hRootKey, dwIndex, pwszValueName, pcBytes, 0, NULL, NULL, 0);
	}
	else
	{
		CHAR szBuffer[MAX_NAME_LEN] = {0};
		ULONG cTotal = sizeof(szBuffer);
		
		//Obtain the specified RegItem at the index specified
		hr = RegEnumValueA(hKey ? hKey : hRootKey, dwIndex, pwszValueName ? szBuffer : NULL, &cTotal, 0, NULL, NULL, 0);
		
		if(pwszValueName)
			ConvertToWCHAR(szBuffer, pwszValueName, *pcBytes);
		*pcBytes = cTotal;
	}
	
CLEANUP:
	if(hr != S_OK && hr != ERROR_NO_MORE_ITEMS)
		hr = E_FAIL;
	CloseRegKey(hKey);
	return hr;
}


////////////////////////////////////////////////////////
// HRESULT GetRegEntry
//
////////////////////////////////////////////////////////
HRESULT GetRegEntry(HKEY hRootKey, WCHAR* pwszKeyName, WCHAR* pwszValueName, WCHAR* pwszValue, ULONG cBytes)
{
	if(IsUnicodeOS())
	{
		return GetRegEntry(hRootKey, pwszKeyName, pwszValueName, pwszValue, cBytes, REG_SZ);
	}
	else
	{
		CHAR szBuffer[MAX_NAME_LEN];
		HRESULT hr = GetRegEntry(hRootKey, pwszKeyName, pwszValueName, szBuffer, MAX_NAME_LEN, REG_SZ);
		if(SUCCEEDED(hr))
			ConvertToWCHAR(szBuffer, pwszValue, cBytes);

		return hr;
	}
}


////////////////////////////////////////////////////////
// HRESULT GetRegEntry
//
////////////////////////////////////////////////////////
HRESULT GetRegEntry(HKEY hRootKey, WCHAR* pwszKeyName, WCHAR* pwszValueName, ULONG* pulValue)
{
	return GetRegEntry(hRootKey, pwszKeyName, pwszValueName, pulValue, sizeof(ULONG), REG_DWORD);
}


////////////////////////////////////////////////////////
// HRESULT GetRegEntry
//
////////////////////////////////////////////////////////
HRESULT GetRegEntry(HKEY hRootKey, WCHAR* pwszKeyName, WCHAR* pwszValueName, void* pStruct, ULONG cBytes, ULONG dwType)
{
	HRESULT hr = E_FAIL;
	HKEY hKey = NULL;
	ASSERT(pStruct);
	
	//Obtain the Data for the above key
	if(pwszKeyName)
	{
		if(FAILED(OpenRegKey(hRootKey, pwszKeyName, 0, KEY_READ, &hKey)))
			goto CLEANUP;
	}
		
	if(IsUnicodeOS())
	{
		hr = RegQueryValueExW(hKey ? hKey : hRootKey, pwszValueName, NULL, &dwType, (BYTE*)pStruct, &cBytes);
	}
	else
	{
		//Obtain the Data for the above key
		CHAR szBuffer[MAX_NAME_LEN];
		ConvertToMBCS(pwszValueName, szBuffer, MAX_NAME_LEN);
		
		hr = RegQueryValueExA(hKey ? hKey : hRootKey, szBuffer, NULL, &dwType, (BYTE*)pStruct, &cBytes);
	}
	
	if(hr != S_OK)
		hr = E_FAIL;

CLEANUP:
	CloseRegKey(hKey);
	return hr;
}


////////////////////////////////////////////////////////
// HRESULT SetRegEntry
//
////////////////////////////////////////////////////////
HRESULT SetRegEntry(HKEY hRootKey, WCHAR* pwszKeyName, WCHAR* pwszValueName, WCHAR* pwszValue)
{
	if(IsUnicodeOS())
	{
		return SetRegEntry(hRootKey, pwszKeyName, pwszValueName, pwszValue ? pwszValue : L"", pwszValue ? ((DWORD)wcslen(pwszValue)+1)*sizeof(WCHAR) : sizeof(WCHAR), REG_SZ);
	}
	else
	{
		CHAR szBuffer[MAX_NAME_LEN];
		ConvertToMBCS(pwszValue, szBuffer, MAX_NAME_LEN);
		return SetRegEntry(hRootKey, pwszKeyName, pwszValueName, pwszValue ? szBuffer : "", pwszValue ? ((DWORD)strlen(szBuffer)+1)*sizeof(CHAR) : sizeof(CHAR), REG_SZ);
	}
}


////////////////////////////////////////////////////////
// HRESULT SetRegEntry
//
////////////////////////////////////////////////////////
HRESULT SetRegEntry(HKEY hRootKey, WCHAR* pwszKeyName, WCHAR* pwszValueName, ULONG ulValue)
{
	return SetRegEntry(hRootKey, pwszKeyName, pwszValueName, &ulValue, sizeof(ULONG), REG_DWORD);
}


////////////////////////////////////////////////////////
// HRESULT SetRegEntry
//
////////////////////////////////////////////////////////
HRESULT SetRegEntry(HKEY hRootKey, WCHAR* pwszKeyName, WCHAR* pwszValueName, void* pStruct, ULONG cBytes, DWORD dwType)
{
	HRESULT hr = E_FAIL;
	HKEY hKey = NULL;

	//Obtain the Data for the above key
	if(pwszKeyName)
	{
		if(FAILED(hr = CreateRegKey(hRootKey, pwszKeyName, &hKey)))
			goto CLEANUP;
	}
	
	if(IsUnicodeOS())
	{
		//Set the data for the above key (or the root key...)
		hr = RegSetValueExW(hKey ? hKey : hRootKey, pwszValueName, 0, dwType, (BYTE*)pStruct, cBytes);
	}
	else
	{
		//Set the data for the above key (or the root key...)
		CHAR szBuffer[MAX_NAME_LEN];
		ConvertToMBCS(pwszValueName, szBuffer, MAX_NAME_LEN);
		hr = RegSetValueExA(hKey ? hKey : hRootKey, szBuffer, 0, dwType, (BYTE*)pStruct, cBytes);
	}

	if(hr != S_OK)
		hr = E_FAIL;

CLEANUP:
	CloseRegKey(hKey);
	return hr;
}



////////////////////////////////////////////////////////
// HRESULT DelRegEntry
//
////////////////////////////////////////////////////////
HRESULT DelRegEntry(HKEY hRootKey, WCHAR* pwszKeyName)
{
	HRESULT hr;

	//DelRegEntry
	if(IsUnicodeOS())
	{
		hr = RegDeleteKeyW(hRootKey, pwszKeyName);
	}
	else
	{
		CHAR szBuffer[MAX_NAME_LEN];
		ConvertToMBCS(pwszKeyName, szBuffer, MAX_NAME_LEN);
		hr = RegDeleteKeyA(hRootKey, szBuffer);
	}

	//Entry successfully deleted - return S_OK
	if(hr==S_OK) 
		return S_OK;

	//Entry not found - return S_FALSE
	if(hr==ERROR_FILE_NOT_FOUND)
		return S_FALSE;

	return E_FAIL;
}


////////////////////////////////////////////////////////
// HRESULT DelAllRegEntry
//
////////////////////////////////////////////////////////
HRESULT DelAllRegEntry(HKEY hRootKey, WCHAR* pwszKeyName)
{
	HKEY hKey = NULL;
	WCHAR wszBuffer[MAX_NAME_LEN];
	HRESULT hr = S_OK;
	
	//RegDeleteKey only deletes the key if there are no subkeys
	//This is a pain to always have to delete the subkeys to remove the key...
	if(SUCCEEDED(hr = OpenRegKey(hRootKey, pwszKeyName, 0, KEY_READ | KEY_WRITE, &hKey)))
	{
		//Loop over all subkeys...
		//NOTE: GetRegEnum requires KEY_ENUMERATE_SUB_KEYS which is found in KEY_READ.
		while((hr = GetRegEnumKey(hKey, NULL, 0, wszBuffer, MAX_NAME_LEN))==S_OK)
		{
			//Recurse and delete the sub key...
			if(FAILED(hr = DelAllRegEntry(hKey, wszBuffer)))
				break;
		}
		   
		//Now we can delete the root key
		hr = DelRegEntry(hRootKey, pwszKeyName);
		CloseRegKey(hKey);
	}

	return hr;
}

	
////////////////////////////////////////////////////////
// HRESULT CloseRegKey
//
////////////////////////////////////////////////////////
HRESULT CloseRegKey(HKEY hKey)
{
	HRESULT hr = S_OK;
	
	//RegCloseKey
	if(hKey)
		hr = RegCloseKey(hKey);
	
	if(hr != S_OK)
		hr = E_FAIL;
	return hr;
}


////////////////////////////////////////////////////////////////////////////
//  PersistFile::Save the current DSO
//
////////////////////////////////////////////////////////////////////////////

HRESULT SaveDSO(WCHAR* pwszFileName)
{
	TBEGIN
	ASSERT(g_pIDBInitialize);
	HRESULT hr = S_OK;
	IPersistFile* pIPersistFile = NULL;

	//Obtain the Persist interface
	QTESTC_(hr = QI(g_pIDBInitialize,IID_IPersistFile,(void**)&pIPersistFile),S_OK);

	//Save the current DSO
	QTESTC_(hr = pIPersistFile->Save(pwszFileName,FALSE),S_OK);

CLEANUP:
	SAFE_RELEASE(pIPersistFile);
	return hr;
}	


HRESULT CreateNewDSO(IUnknown* pIUnkOuter, REFIID riid, IUnknown** ppIUnknown, DWORD dwOptions)
{
	ASSERT(ppIUnknown);
	HRESULT			hr = S_OK;
	IUnknown*		pIUnknown = NULL;

	//CoCreate another instance
	//Some interfaces may not be available until after initialization.
	//And we need to allow aggregation to work, so IID_IUnknown is the only choice...
	QTESTC_(hr = GetModInfo()->CreateProvider(pIUnkOuter, IID_IUnknown, (IUnknown**)&pIUnknown),S_OK)

	//SetProperties, Initialize (if requested)
	QTESTC_(hr = InitializeDataSource(pIUnknown, dwOptions),S_OK);

	//Now obtain requested interface
	QTESTC(VerifyInterface(pIUnknown, riid, DATASOURCE_INTERFACE, ppIUnknown));

CLEANUP:
	SAFE_RELEASE(pIUnknown);
	return hr;
}


////////////////////////////////////////////////////////////////////////////
//  InitializeDataSource
//
////////////////////////////////////////////////////////////////////////////
HRESULT InitializeDataSource(IUnknown* pDataSource, DWORD dwOptions)
{
	ASSERT(pDataSource);

	ULONG cPropSets = 0;
	DBPROPSET* rgPropSets = NULL;
	IDBInitialize* pIDBInitialize = NULL; 
	IDBProperties* pIDBProperties = NULL; 
	HRESULT hr = S_OK;

	//SetProperties (if requested)
	if(dwOptions & CREATEDSO_SETPROPERTIES)
	{
		//Build our init options from string passed to us from the LTM InitString
		GetInitProps(&cPropSets,&rgPropSets);

		QTESTC_(hr = QI(pDataSource, IID_IDBProperties, (void**)&pIDBProperties),S_OK)
		QTESTC_(hr = pIDBProperties->SetProperties(cPropSets,rgPropSets),S_OK)
	}
	
	//Initialize (if requested)
	if(dwOptions & CREATEDSO_INITIALIZE)
	{
		QTESTC_(hr = QI(pDataSource, IID_IDBInitialize, (void**)&pIDBInitialize),S_OK)
		QTESTC_(hr = pIDBInitialize->Initialize(),S_OK)

		//Is the DataSource from the Pool?
		if(SUCCEEDED(hr))
		{
			//Display the Pooling Status...
			DisplayPooling(pDataSource/*, TRUE*/);
		}
	}

CLEANUP:
	SAFE_RELEASE(pIDBInitialize);
	SAFE_RELEASE(pIDBProperties);
	FreeProperties(&cPropSets,&rgPropSets);
	return hr;
}


////////////////////////////////////////////////////////////////////////////
//  ShouldInitialize
//
////////////////////////////////////////////////////////////////////////////
BOOL ShouldInitialize(IUnknown* pDataSource)
{
	//If we are running the test in debug mode and we don't mind seeing potential dialogs
	//then allow this DSO to be initialized.
	if(GetModInfo()->GetDebugMode() & DEBUGMODE_DIALOGS)
		return TRUE;

	//NOTE: Make sure that the interface supports properties, before asking for properties.
	//Since our internal method fails if a property interface is not supported.
	if(SUCCEEDED(QI(pDataSource, IID_IDBProperties)))
	{
		ULONG_PTR ulValue = 0;
	
		//Don't call Initialize with PROMPT = something that prompts, since we running
		//in automation, unless of course thats what were interested in...
		if(!GetProperty(DBPROP_INIT_PROMPT, DBPROPSET_DBINIT, pDataSource, &ulValue) || ulValue==0 || ulValue==DBPROMPT_NOPROMPT)
			return TRUE;
	}

	return FALSE;
}
			

////////////////////////////////////////////////////////////////////////////
//  ReleaseDataSource
//
////////////////////////////////////////////////////////////////////////////
HRESULT ReleaseDataSource(IUnknown** ppDataSource)
{
	TBEGIN
	ASSERT(ppDataSource);
	HRESULT hr = S_OK;
	CPoolManager cPoolManager;

	if(*ppDataSource)
	{
		//This function will be called from nunmerous locations, when its determined that the 
		//datasoure is no longer needed, and the state of the datasource is no longer important.
		//The main additional testing that we need to do is to try and get this datasource into the 
		//pool.
		
		//NOTE: We don't always Initialize, since the caller may have prompting properties or 
		//other properties which would prevent automation from running...
		if(ShouldInitialize(*ppDataSource))
		{
			//See if the datasource can be directly initialized to enter into the pool...
			QTESTC_(InitializeDataSource(*ppDataSource, CREATEDSO_SETPROPERTIES | CREATEDSO_INITIALIZE),S_OK);
		}
	}

CLEANUP:
	//Now release the object...
	cPoolManager.ReleaseObject(ppDataSource);
	return hr;
}


HRESULT CreateNewSession(IUnknown* pIDSO, REFIID riid, IUnknown** ppIUnknown, IUnknown* pIUnkOuter)
{
	ASSERT(ppIUnknown);
	IUnknown* pDataSource = NULL;
	IDBCreateSession* pIDBCreateSession = NULL;
	HRESULT hr = S_OK;

	//Create a new DSO if not passed in one
	if(!pIDSO)
	{
		//Can't just ask for IDBCreateSession, since its not initalized yet...
		QTESTC_(hr = CreateNewDSO(NULL, IID_IUnknown, &pDataSource),S_OK);
		QTESTC_(hr = QI(pDataSource, IID_IDBCreateSession, (void**)&pIDBCreateSession),S_OK);
	}
	else
	{
		QTESTC_(hr = QI(pIDSO, IID_IDBCreateSession, (void**)&pIDBCreateSession),S_OK)
	}

	//Create the Session Object
	QTESTC_(hr = pIDBCreateSession->CreateSession(pIUnkOuter, riid, ppIUnknown),S_OK)
	
CLEANUP:
	SAFE_RELEASE(pDataSource);
	SAFE_RELEASE(pIDBCreateSession);
	return hr;
}

HRESULT CreateNewCommand(IUnknown* pISession, REFIID riid, IUnknown** ppIUnknown, IUnknown* pIUnkOuter)
{
	ASSERT(ppIUnknown);
	IDBCreateCommand* pIDBCreateCommand = NULL;
	HRESULT hr = S_OK;

	//Create a new DSO if not passed in one
	if(!pISession)
	{
		QTESTC_(hr = CreateNewSession(NULL, IID_IDBCreateCommand, (IUnknown**)&pIDBCreateCommand),S_OK);
	}
	else
	{
		if(!VerifyInterface(pISession, IID_IDBCreateCommand, SESSION_INTERFACE, (IUnknown**)&pIDBCreateCommand))
		{
			hr = E_NOINTERFACE;
			goto CLEANUP;
		}
	}

	//Create the Command Object
	QTESTC_(hr = pIDBCreateCommand->CreateCommand(pIUnkOuter, riid, ppIUnknown),S_OK)
	
CLEANUP:
	SAFE_RELEASE(pIDBCreateCommand);
	return hr;
}


////////////////////////////////////////////////////////////////////////////
// CanConvert
//
////////////////////////////////////////////////////////////////////////////
BOOL CanConvert(IUnknown* pIUnknown, DBTYPE wFromType, DBTYPE wToType)
{
	TBEGIN
	ASSERT(pIUnknown);
	HRESULT hr = E_FAIL;
	IConvertType* pIConvertType = NULL;

	//Obtain the IConverType interface
	TESTC_(hr = QI(pIUnknown, IID_IConvertType, (void**)&pIConvertType),S_OK);
	ASSERT(pIConvertType);

	//See if this type conversion is supported
	hr = pIConvertType->CanConvert(wFromType, wToType, DBCONVERTFLAGS_COLUMN);

CLEANUP:
	SAFE_RELEASE(pIConvertType);
	return hr == S_OK;
}


////////////////////////////////////////////////////////////////////////////
//  AppendString
//
////////////////////////////////////////////////////////////////////////////
HRESULT AppendString(WCHAR** ppwszOldString, WCHAR* pwszAppend)
{
	ASSERT(ppwszOldString);

	//NOTE: this function takes an existing string pwszOldString
	//And appends to it, returning a new pointer, and releasing the previous string...
	
	//Trival Cases
	if(pwszAppend == NULL)
		return S_OK;
		
	//Trival Cases
	if(*ppwszOldString == NULL)
	{
		*ppwszOldString = wcsDuplicate(pwszAppend);
		return S_OK;
	}

	size_t ulNewLen = wcslen(*ppwszOldString) + wcslen(pwszAppend);

	//Reallocate the string
	SAFE_REALLOC(*ppwszOldString, WCHAR, (ulNewLen+1)*sizeof(WCHAR));
	wcscat(*ppwszOldString, pwszAppend);
		
CLEANUP:
	return S_OK;
}


////////////////////////////////////////////////////////////////////////////
//  GetTableSchemaInfo
//
////////////////////////////////////////////////////////////////////////////
BOOL GetTableSchemaInfo
(
	CTable *	pTable,
	WCHAR *		pwszTargetTable,
	BOOL *		pfFound,
	WCHAR **	ppwszCatalogName,
	WCHAR **	ppwszSchemaName
)
{
	TBEGIN
	BOOL	fFound = FALSE;
	HRESULT hr;
	HROW	hRow;
	WCHAR * pwszCatalogName = NULL;
	WCHAR * pwszSchemaName = NULL;
	WCHAR *	pwszTableName = NULL;

	ASSERT(pTable && pwszTargetTable);

	if( pfFound )
		*pfFound = FALSE;
	if( ppwszCatalogName )
		*ppwszCatalogName = NULL;
	if( ppwszSchemaName )
		*ppwszSchemaName = NULL;

	//Obtain Schema TABLES Rowset
	//We don't want to put any restrictions, since its not required to support 
	//restrictions...
	CRowset Rowset;
	Rowset.SetProperty(DBPROP_CANHOLDROWS);
	TESTC_PROVIDER(Rowset.CreateRowset(SELECT_DBSCHEMA_TABLE, IID_IRowset, pTable, DBACCESSOR_ROWDATA, DBPART_ALL, ALL_COLS_BOUND)==S_OK);

	//Try to find the specified row with this table...
	while(hr = Rowset.GetNextRows(&hRow)==S_OK)
	{
		//GetData for this row
		TESTC_(hr = Rowset.GetRowData(hRow, &Rowset.m_pData),S_OK);

		//TABLE_NAME column
		DBBINDING* pCatalogBinding	= Rowset.m_rgBinding[0].iOrdinal == 0 ? &Rowset.m_rgBinding[1] : &Rowset.m_rgBinding[0];
		DBBINDING* pSchemaBinding	= Rowset.m_rgBinding[0].iOrdinal == 0 ? &Rowset.m_rgBinding[2] : &Rowset.m_rgBinding[1];
		DBBINDING* pTableBinding	= Rowset.m_rgBinding[0].iOrdinal == 0 ? &Rowset.m_rgBinding[3] : &Rowset.m_rgBinding[2];
		DBBINDING* pTypeBinding		= Rowset.m_rgBinding[0].iOrdinal == 0 ? &Rowset.m_rgBinding[4] : &Rowset.m_rgBinding[3];

		//See if this is a TABLE type...
		if(STATUS_BINDING(*pTypeBinding, Rowset.m_pData)==DBSTATUS_S_OK &&
			wcscmp((WCHAR*)&VALUE_BINDING(*pTypeBinding, Rowset.m_pData),L"TABLE")==0)
		{
			//See if this contains a tablename...
			if(STATUS_BINDING(*pTableBinding, Rowset.m_pData)==DBSTATUS_S_OK)
			{
				//TableName
				pwszTableName = (WCHAR*)&VALUE_BINDING(*pTableBinding, Rowset.m_pData);

				//Catalog Name
				if(STATUS_BINDING(*pCatalogBinding, Rowset.m_pData)==DBSTATUS_S_OK)
					pwszCatalogName = (WCHAR*)&VALUE_BINDING(*pCatalogBinding, Rowset.m_pData);
				//Schema Name
				if(STATUS_BINDING(*pSchemaBinding, Rowset.m_pData)==DBSTATUS_S_OK)
					pwszSchemaName = (WCHAR*)&VALUE_BINDING(*pSchemaBinding, Rowset.m_pData);

				if( 0 == wcscmp(pwszTableName, pwszTargetTable) )
				{
					fFound = TRUE;
					break;					
				}
			}
		}

		//Release this row...	
		Rowset.ReleaseRows(hRow);
	}

CLEANUP:

	if( fFound )
	{
		if( ppwszCatalogName )
			*ppwszCatalogName = wcsDuplicate(pwszCatalogName);	
		if( ppwszSchemaName )
			*ppwszSchemaName = wcsDuplicate(pwszSchemaName);
	}

	if( pfFound )
		*pfFound = fFound;

	TRETURN
}


////////////////////////////////////////////////////////////////////////////
// ConvertString
//
////////////////////////////////////////////////////////////////////////////
HRESULT	ConvertString(WCHAR* pwszBuffer, BOOL fUpperCase)
{
	//no-op
	if(pwszBuffer == NULL)
		return E_FAIL;

	//Convert String to Upper to Lower
	for(ULONG i=0; pwszBuffer[i]; i++)
	{
		if(fUpperCase)
			pwszBuffer[i] = towupper(pwszBuffer[i]);
		else
			pwszBuffer[i] = towlower(pwszBuffer[i]);
	}
	
	return S_OK;
}


////////////////////////////////////////////////////////////////////////////
//  CreateString
//
////////////////////////////////////////////////////////////////////////////
WCHAR* CreateString(WCHAR* pwszFmt, ...)
{
	//Process all varible input args...
	va_list		marker;
	WCHAR		wszBuffer[MAX_QUERY_LEN];
	wszBuffer[0] = L'\0';

	// Use format and arguements as input
	//This version will not overwrite the stack, since it only copies
	//upto the max size of the array
	va_start(marker, pwszFmt);
	_vsnwprintf(wszBuffer, MAX_QUERY_LEN, pwszFmt, marker);
	va_end(marker);

	//Make sure there is a NULL Terminator, vsnwprintf will not copy
	//the terminator if length==MAX_QUERY_LEN
	wszBuffer[MAX_QUERY_LEN-1] = L'\0';
	return wcsDuplicate(wszBuffer);
}

	
////////////////////////////////////////////////////////////////////////////
// Storage Object helpers
//
////////////////////////////////////////////////////////////////////////////
DBBINDING* FindBinding(DBCOUNTITEM cBindings, DBBINDING* rgBindings, DBTYPE wType, DBCOUNTITEM* pulIndex)
{
	//Loop through bindings and the binding of the indicated type
	for(DBCOUNTITEM i=0; i<cBindings; i++)
	{
		//Remove Type Modifiers
		DBTYPE wBindingType = rgBindings[i].wType & ~(DBTYPE_BYREF | DBTYPE_ARRAY | DBTYPE_VECTOR);
		
		//Find the Storage binding
		if(wBindingType == wType)
		{
			if(pulIndex)
				*pulIndex = i;
			return &rgBindings[i];
		}
	}

	return NULL;
}

BOOL GetStorageObject(DBCOUNTITEM cBindings, DBBINDING* rgBindings, void* pData, REFIID riid, IUnknown** ppIUnknown)
{	
	//Verify input params
	TBEGIN
	ASSERT(cBindings && rgBindings && pData);
	BOOL bFound = FALSE;

	//Make sure pointer is not currently pointing to something
	if(ppIUnknown)
	{
		ASSERT(*ppIUnknown == NULL);
	}

	//Find the IUNKNOWN binding
	DBBINDING* pBinding = FindBinding(cBindings, rgBindings, DBTYPE_IUNKNOWN);
	if(pBinding) 
	{
		//Verify pObject.
		//NOTE:  pObject == NULL, indicates a default iid of IID_IUnknown.
		if(pBinding->pObject)
		{
			TESTC(pBinding->pObject->iid == riid)
		}

		//DBSTATUS_S_ISNULL case
		if(STATUS_IS_BOUND(*pBinding) && STATUS_BINDING(*pBinding,pData) == DBSTATUS_S_ISNULL)
		{
			//Verify Length == 0;
			if(LENGTH_IS_BOUND(*pBinding))
				TESTC(LENGTH_BINDING(*pBinding,pData) == 0);

			bFound = TRUE;
		}
		else
		{
			//Verify Status
			if(STATUS_IS_BOUND(*pBinding))
				TESTC( STATUS_BINDING(*pBinding,pData) == DBSTATUS_S_OK);

			//Verify Length
//			if(LENGTH_IS_BOUND(*pBinding))
//				TESTC( LENGTH_BINDING(*pBinding,pData) == sizeof(IUnknown*))
				
			//Verify Value (interface)
			if(VALUE_IS_BOUND(*pBinding) && ppIUnknown)
				*ppIUnknown = (IUnknown*)VALUE_BINDING(*pBinding,pData);

			//Found the interface and everything was correct
			bFound = TRUE;
		}
	}
	
CLEANUP:
	return bFound;
}				   


HRESULT GetStorageData(DBCOUNTITEM cBindings, DBBINDING* rgBindings, void* pData, void* pBuffer, ULONG* pcBytes, REFIID riid, IUnknown** ppIUnknown)
{	
	//Verify input params
	TBEGIN
	ASSERT(cBindings && rgBindings && pData);
	HRESULT hr = E_INVALIDARG;
	IUnknown* pIUnknown = NULL;
	
	//Get our storage object first
	QTESTC(GetStorageObject(cBindings, rgBindings, pData, riid, &pIUnknown));
	
	//If the above call to GetStorageObject succeeded, set hr to S_OK.
	hr = S_OK;

	//Obtain the data from the storage object
	if(pBuffer && pcBytes)
		QTESTC_(hr = GetStorageData(riid, pIUnknown, pBuffer, pcBytes),S_OK);
	
CLEANUP:
	//Does the user want the storage interface
	if(ppIUnknown)
		*ppIUnknown = pIUnknown;
	else
		SAFE_RELEASE(pIUnknown);
	return hr;
}				   


HRESULT GetStorageData(REFIID riid, IUnknown* pIUnknown, void* pBuffer, ULONG* pcBytes)
{	
	//Verify input params
	TBEGIN
	HRESULT hr = E_INVALIDARG;

	if(pIUnknown && pBuffer && pcBytes)
	{
		//cBytes on input respresents the maximum number of bytes to read (size of the buffer).  
		//on ouput it represents the actual number of bytes copied
		if(*pcBytes == 0)
			return E_INVALIDARG;

		//Which method to call to read the data...
		TEST2C_(hr = StorageRead(riid, pIUnknown, pBuffer, *pcBytes, pcBytes), S_OK, S_FALSE);
		hr = S_OK;
	}	
	
CLEANUP:
	return hr;
}				   

BOOL SetStorageObject(DBCOUNTITEM cBindings, DBBINDING* rgBindings, void* pData, DBCOUNTITEM iTableRow, REFIID riid, CStorage* pCStorage, DBSTATUS dbStatus, DBLENGTH cBytes)
{	
	//Verify input params
	ASSERT(cBindings && rgBindings);
	ASSERT(pData);

	BOOL bFound = FALSE;
	void* pBuffer = NULL;
	IUnknown* pIUnknown = NULL;

	//Find the IUNKNOWN binding
	DBBINDING* pBinding = FindBinding(cBindings, rgBindings, DBTYPE_IUNKNOWN);
	if(pBinding)
	{
		//Remove any existing storage object (setup by FillInputBindings)
		if(VALUE_IS_BOUND(*pBinding) && STATUS_IS_BOUND(*pBinding) && STATUS_BINDING(*pBinding, pData)==DBSTATUS_S_OK)
		{
			pIUnknown = (IUnknown*)VALUE_BINDING(*pBinding, pData);
			if(pIUnknown)
			{
				//NOTE: FillInputBindings actually puts 2 refcounts on the stream, since the 
				//provider is responsible for releasing one.  So just make sure of this...
				ULONG ulRefCount = pIUnknown->Release();
				SAFE_RELEASE(pIUnknown);
				TESTC(ulRefCount == 1);
			}
		}
		
		//Set Status
		STATUS_BINDING(*pBinding,pData) = dbStatus;

		//Set Value (Storage interface)
		if(VALUE_IS_BOUND(*pBinding))
		{
			if(dbStatus == DBSTATUS_S_OK)
			{
				//Create the data for the storage object
				DBLENGTH cBufferSize = 0;
				if(pCStorage && iTableRow)
				{
					DBLENGTH cBytesLeft;
					
					//Create the data for the storage object
					TESTC(CreateData(&pBuffer, iTableRow, pBinding, &cBufferSize))
					
					//If cBytes == ULONG_MAX it just means write all the bytes 
					//otherwise it means write the indicated number of bytes
					if(cBytes == MAX_PTR)
						cBytes = cBufferSize;
					cBytesLeft = cBytes;

					//Dump it into the storage object
					while(cBytesLeft)
					{
						//Which method to call to write the data...
						TESTC_(StorageWrite(riid, (IStream*)pCStorage, pBuffer, (ULONG)min(cBytesLeft, cBufferSize), NULL),S_OK);
						cBytesLeft -= min(cBytesLeft, cBufferSize);
					}

					//Now that we have successfully wrote the data into the stream object
					//reset the stream current poisition so it can be used by SetData...
					TESTC_(pCStorage->Seek(0),S_OK);
				}
					
				//Passing the object pointer should be suffiencent enough, as the provider
				//will have to QI of the object anyway to determine if it even supports
				//the interface, but since many incorrectly do not we will explictly
				//QI here so the majority of the cases don't crash, we already have
				//quite a few that cover this boundary senario
				if(pCStorage)
					TESTC_(pCStorage->QueryInterface(riid, (void**)&pIUnknown),S_OK);
				VALUE_BINDING(*pBinding,pData) = pIUnknown;
			}
			else
			{
				//Value should not be looked at by the provider, make sure of this...
				VALUE_BINDING(*pBinding,pData) = INVALID(IUnknown*);
			}
		}
			
		//Set Length
		//NOTE: This has to be done after the value is setup to correctly specify the number 
		//of bytes to write, (some providers need to know in the length binding the number of bytes
		//in the stream ahead of time before acctually reading the stream).
		if(LENGTH_IS_BOUND(*pBinding))
			LENGTH_BINDING(*pBinding,pData) = (cBytes==MAX_PTR /* || cBytes==0 */) ? sizeof(IUnknown*) : cBytes;
				
		//Found the interface and everything was correct
		bFound = TRUE;
	}

CLEANUP:
	PROVIDER_FREE(pBuffer);
	SAFE_RELEASE(pIUnknown);
	return bFound;
}				   

BOOL VerifyAccessorStatus(DBCOUNTITEM cBindings, DBBINDING* rgBindings, DBBINDSTATUS* rgBindStatus, DBTYPE wType, DBBINDSTATUS dwBindStatus)
{
	TBEGIN
	DBCOUNTITEM ulBinding = 0;

	//Find the IUNKNOWN binding
	TESTC(FindBinding(cBindings, rgBindings, wType, &ulBinding) != NULL);

	//See if the AccessorStatus is what we expect
	TESTC(rgBindStatus != NULL);
	QTESTC(rgBindStatus[ulBinding] == dwBindStatus)

CLEANUP:
	TRETURN
}
	 
BOOL VerifyBindings(HRESULT hrReturned, IAccessor* pIAccessor, HACCESSOR hAccessor, void* pData)
{
	TBEGIN
	ASSERT(pIAccessor);
	ULONG dwAccessorFlags;
	DBCOUNTITEM cBindings = 0;
	DBBINDING* rgBindings = NULL;
	
	//Obtain the bindings
	if(SUCCEEDED(pIAccessor->GetBindings(hAccessor, &dwAccessorFlags, &cBindings, &rgBindings)))
	{
		//Delegate
		QTESTC(VerifyBindings(hrReturned, cBindings, rgBindings, pData));
	}

CLEANUP:
	FreeAccessorBindings(cBindings, rgBindings);
	TRETURN
}

BOOL VerifyBindingStatus(DBCOUNTITEM cBindings, DBBINDING* rgBindings, void* pData, DBTYPE wType, DBSTATUS dwStatus)
{
	TBEGIN
	ASSERT(pData);

	//Find the IUNKNOWN binding
	DBBINDING* pBinding = FindBinding(cBindings, rgBindings, wType);
	TESTC(pBinding != NULL);

	//See if the binding status is what we expect
	if(STATUS_IS_BOUND(*pBinding))
	{
		DBSTATUS dwActualStatus = STATUS_BINDING(*pBinding, pData);
		if(dwActualStatus != dwStatus)
			TERROR("Expected: " << GetStatusName(dwStatus) << " Received: " << GetStatusName(dwActualStatus));
	}

CLEANUP:
	TRETURN
}

BOOL CreateData(void** ppBuffer, DBCOUNTITEM iTableRow, DBBINDING* pBindings, DBLENGTH* pcSize)
{
	ASSERT(ppBuffer && *ppBuffer==NULL);
	ASSERT(g_pTable && iTableRow);
	ASSERT(pBindings && pcSize);
	TBEGIN

	//Nice wrapper arround MakeData
	WCHAR* pwszData = NULL;
	CCol Col;

	//Obtain the same data as used on the backend
	pwszData = (WCHAR*)PROVIDER_ALLOC(sizeof(WCHAR)*DATA_SIZE);
	TESTC_(g_pTable->MakeData(pwszData, iTableRow, pBindings->iOrdinal, PRIMARY, DBTYPE_EMPTY, TRUE /* NoNulls */),S_OK);

	//Get ColInfo
	TESTC_(g_pTable->GetColInfo(pBindings->iOrdinal, Col),S_OK);

	//cSize is a count of Bytes (not chars)
	*pcSize = Col.GetMaxSize();
	if(Col.GetProviderType() == DBTYPE_WSTR || Col.GetProviderType() == DBTYPE_BSTR)
		*pcSize = Col.GetMaxSize() * sizeof(WCHAR);

	//Now convert it to the specified type
	*ppBuffer = WSTR2DBTYPE(pwszData, Col.GetProviderType(), (USHORT*)pcSize);

CLEANUP:
	PROVIDER_FREE(pwszData);
	TRETURN
}


////////////////////////////////////////////////////////////////////////////
//  FindValue
//
////////////////////////////////////////////////////////////////////////////
BOOL FindValue(DBCOUNTITEM ulValue, DBCOUNTITEM cElements, DBCOUNTITEM* rgElements)
{
	for(DBCOUNTITEM i=0; i<cElements; i++)
	{
		ASSERT(rgElements);
		if(ulValue == rgElements[i])
			return TRUE;
	}

	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
// Property Routines
//
//////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
//  Find the Property Set within the PropSets and return a pointer to it
//
////////////////////////////////////////////////////////////////////////////
BOOL FindPropSet(GUID guidPropertySet, ULONG cPropSets, DBPROPSET* rgPropSets, DBPROPSET** ppPropSet)
{
	//Will find the property with in the PropSets and return an index to it

	//Loop over the number of property sets
	for(ULONG iSet=0; iSet<cPropSets; iSet++)
	{
		//Make sure where looking in the right property set
		if(guidPropertySet == rgPropSets[iSet].guidPropertySet)
		{
			if(ppPropSet)
				*ppPropSet =  &rgPropSets[iSet];
			return TRUE;
		}
	}

	if(ppPropSet)
		*ppPropSet =  NULL;
	return FALSE;
}


////////////////////////////////////////////////////////////////////////////
//  Find the Property within the PropSets and return a pointer to it
//
////////////////////////////////////////////////////////////////////////////
BOOL FindProperty(DBPROPID dwPropertyID, GUID guidPropertySet, ULONG cPropSets, DBPROPSET* rgPropSets, DBPROP** ppProp)
{
	//Will find the property with in the PropSets and return an index to it

	//Try to Find the correct Property Set
	for(ULONG iPropSet=0; iPropSet < cPropSets; iPropSet++)
	{
		DBPROPSET* pPropSet = &rgPropSets[iPropSet];
		if(guidPropertySet != pPropSet->guidPropertySet)
			continue;

		//Loop over the array of properties in this set
		for(ULONG iProp=0; iProp<pPropSet->cProperties; iProp++)
		{
			DBPROP* pProp = &pPropSet->rgProperties[iProp];
			if(pProp->dwPropertyID == dwPropertyID)
			{	
				if(ppProp)
					*ppProp = pProp;
				return TRUE;
			}
		}

	}

	if(ppProp)
		*ppProp = NULL;
	return FALSE;
}


////////////////////////////////////////////////////////////////
// EqualProperty
//
////////////////////////////////////////////////////////////////
BOOL EqualProperty(DBPROP* pProp, DBPROP* pProp2, enum PROPSETCOMPAREOPTIONS_ENUM eCompareOptions)
{
	TBEGIN
	ASSERT(pProp);
	ASSERT(pProp2);
	WCHAR* pwszPropertyName = GetPropertyName(pProp->dwPropertyID, DBPROPSET_DBINIT);
	
	//Verify the Same dwPropertyID
	if(pProp->dwPropertyID != pProp2->dwPropertyID)
		TERROR("Property \"" << pwszPropertyName << "\" IDs don't match");

	//Verify the Same dwOptions
	//TODO make this an error when bug is fixed...
	if(pProp->dwOptions != pProp2->dwOptions)
	{
		if(eCompareOptions==IGNORE_OPTION_FOR_VTEMPTY && V_VT(&pProp->vValue)==VT_EMPTY && V_VT(&pProp2->vValue)==VT_EMPTY)
			odtLog << L"WARNING!!! " << pwszPropertyName << L": Options don't match for properties with VT_EMPTY values: ignoring because of IGNORE_OPTION_FOR_VTEMPTY is passed in\n";
		else
		{

			TERROR("Property \"" << pwszPropertyName << "\" Options don't match");
		}
	}


	//Verify the Same dwStatus
	if(pProp->dwStatus != pProp2->dwStatus)
		TERROR("Property \"" << pwszPropertyName << "\" Status' don't match");

	//Verify the Same colid
//	QTESTC(VerifyColID());

	//Verify the Same Value
	if(!CompareVariant(&pProp->vValue, &pProp2->vValue))
	{
		WCHAR wszBuffer1[MAX_QUERY_LEN];
		WCHAR wszBuffer2[MAX_QUERY_LEN];
		
		//Display Differences...
		VariantToString(&pProp->vValue, wszBuffer1, MAX_QUERY_LEN, FALSE);
		VariantToString(&pProp2->vValue, wszBuffer2, MAX_QUERY_LEN, FALSE);

		TERROR(L"Property values for \"" << pwszPropertyName << "\" don't match:  Expected: \"" << wszBuffer1 << "\" Actual: \"" << wszBuffer2 << "\"");
		QTESTC(FALSE);
	}
	
CLEANUP:
	TRETURN
}


////////////////////////////////////////////////////////////////
// EqualPropSets
//
////////////////////////////////////////////////////////////////
BOOL EqualPropSets(ULONG cPropSets, DBPROPSET* rgPropSets, ULONG cPropSets2, DBPROPSET* rgPropSets2, enum PROPSETCOMPAREOPTIONS_ENUM eCompareOptions)
{
	//This function makes sure that both property sets are identical, 
	//(except order can be different)
	TBEGIN
	ULONG i,j=0;
	DBPROPSET* pPropSet = NULL;
	DBPROPSET* pPropSet2 = NULL;
	DBPROP* pProp = NULL;
	DBPROP* pProp2 = NULL;

	//Verify the Same number of sets
	QTESTC(cPropSets == cPropSets2);
	
	for(i=0; i<cPropSets; i++)
	{
		//Current PropSet
		pPropSet = &rgPropSets[i];
		
		//Find the corresponding PropSet
		QTESTC(FindPropSet(pPropSet->guidPropertySet, cPropSets2, rgPropSets2, &pPropSet2));

		//Now make sure that each set is equal
		QTESTC(pPropSet->cProperties == pPropSet2->cProperties);
		
		for(j=0; j<pPropSet->cProperties; j++)
		{
			pProp = &pPropSet->rgProperties[j];
			QTESTC(FindProperty(pProp->dwPropertyID, pPropSet->guidPropertySet, 1, pPropSet2, &pProp2));
			
			//Make sure the Property is equal
			QTESTC(EqualProperty(pProp, pProp2, eCompareOptions));
		}
	}

CLEANUP:
	TRETURN
}

	

////////////////////////////////////////////////////////////////////////////
//  Verify that all of the properties of the set have a certain status
//
////////////////////////////////////////////////////////////////////////////

BOOL VerifyPropSetStatus(ULONG cPropSets, DBPROPSET* rgPropSets, DBPROPSTATUS dwPropStatus)
{
	TBEGIN

	//no-op case, no properties
	if(cPropSets==0) 
		return TRUE;

	//Loop over all the property sets
	for(ULONG i=0; i<cPropSets; i++)
	{
		//Loop over the array of properties
		for(ULONG j=0; j<rgPropSets[i].cProperties; j++)
		{
			QTESTC(rgPropSets[i].rgProperties[j].dwStatus == dwPropStatus)
		}
	}

CLEANUP:
	TRETURN
}

BOOL FreePropInfos(ULONG cPropertyInfos, DBPROPINFO* rgPropInfos)
{
	//Loop through all PropInfos and free the variants
	for(ULONG i=0; i<cPropertyInfos; i++)
		VariantClear(&rgPropInfos[i].vValues);

	//Free outer alloced array
	PROVIDER_FREE(rgPropInfos);
	return TRUE;
}


BOOL GetPropStatus(ULONG cPropSets, DBPROPSET* rgPropSets, DBPROPID dwPropertyID, GUID guidPropertySet, DBPROPSTATUS* pdwPropStatus)
{
	ASSERT(cPropSets && rgPropSets);
	ASSERT(pdwPropStatus);
	TBEGIN

	DBPROP* pProp = NULL;		
	
	//Need to find the property within the PropSets
	TESTC(FindProperty(dwPropertyID, guidPropertySet, cPropSets, rgPropSets, &pProp))
	
	//Grab the PropStatus field
	*pdwPropStatus = pProp->dwStatus;
	
CLEANUP:	
	TRETURN
}


BOOL VerifyPropStatus(ULONG cPropSets, DBPROPSET* rgPropSets, DBPROPID dwPropertyID, GUID guidPropertySet, DBPROPSTATUS dwInputStatus, enum VALUE_ENUM eValue)
{
	ASSERT(cPropSets && rgPropSets);
	TBEGIN
	DBPROP* pProp = NULL;
		
	//Need to find the property within the PropSets
	DBPROPFLAGS dwFlags = GetPropInfoFlags(dwPropertyID, guidPropertySet);
	TESTC(FindProperty(dwPropertyID, guidPropertySet, cPropSets, rgPropSets, &pProp))

	//See if the provider supports it
	if(dwFlags == DBPROPFLAGS_NOTSUPPORTED)
	{
		if(pProp->dwOptions == DBPROPOPTIONS_REQUIRED)
		{
			if(!(pProp->dwStatus == DBPROPSTATUS_NOTSUPPORTED))
				TERROR("Status returned: " << GetPropStatusName(pProp->dwStatus) << " Status expected: " << GetPropStatusName(DBPROPSTATUS_NOTSUPPORTED));
		}
		else
		{
			//Both of the following errors apply for optional - as the spec doesn't mandate order
			if(!(pProp->dwStatus == DBPROPSTATUS_NOTSUPPORTED || pProp->dwStatus == DBPROPSTATUS_NOTSET))
				TERROR("Status returned: " << GetPropStatusName(pProp->dwStatus) << " Status expected: " << GetPropStatusName(DBPROPSTATUS_NOTSET));
		}
	}
	//See if this is a Read-Only property
	else if(BITCLEAR(dwFlags, DBPROPFLAGS_WRITE))
	{
		//The OLE DB Spec allows setting a Read-Only property
		//as long as its to the default value...
		//Can't really check here, since it would require me to call
		//ICommandProperties::GetProperties on the command object to find 
		//the default value.  This would assume CommandObjects are supported!
		//Can't Check the RowsetInfo::GetProperties since defaults can change
		//from the Command to the Rowset.  Spec is incomplete, unable to
		//get default values without assumming commands are supported...
		if(pProp->dwOptions == DBPROPOPTIONS_REQUIRED)
		{
			if(!(pProp->dwStatus == DBPROPSTATUS_NOTSETTABLE || pProp->dwStatus == dwInputStatus))
				TERROR("Status returned: " << GetPropStatusName(pProp->dwStatus) << " Status expected: " << GetPropStatusName(dwInputStatus != DBPROPSTATUS_OK ? dwInputStatus : DBPROPSTATUS_NOTSETTABLE));
		}
		else
		{
			//Both of the following errors apply for optional - as the spec doesn't mandate order
			if(!(pProp->dwStatus == DBPROPSTATUS_NOTSETTABLE || pProp->dwStatus == DBPROPSTATUS_NOTSET || pProp->dwStatus == dwInputStatus))
				TERROR("Status returned: " << GetPropStatusName(pProp->dwStatus) << " Status expected: " << GetPropStatusName(dwInputStatus != DBPROPSTATUS_OK ? dwInputStatus : DBPROPSTATUS_NOTSETTABLE));
		}
	}
	else
	{
		//DBPROPOPTIONS_REQUIRED
		if(pProp->dwOptions == DBPROPOPTIONS_REQUIRED)
		{
			if(dwFlags & DBPROPFLAGS_COLUMNOK)
			{
				if(!(pProp->dwStatus == dwInputStatus || pProp->dwStatus == DBPROPSTATUS_NOTALLSETTABLE))
				{
					// The provider doesn't necessarily support all valid values, so warn
					if (eValue == INDETERMINATE_VALUE && pProp->dwStatus == DBPROPSTATUS_BADVALUE)
					{
						TWARNING("Status returned: " << GetPropStatusName(pProp->dwStatus) << " Status expected: " << GetPropStatusName(dwInputStatus != DBPROPSTATUS_OK ? dwInputStatus : DBPROPSTATUS_NOTALLSETTABLE));
					}
					// Otherwise error
					else
						TERROR("Status returned: " << GetPropStatusName(pProp->dwStatus) << " Status expected: " << GetPropStatusName(dwInputStatus != DBPROPSTATUS_OK ? dwInputStatus : DBPROPSTATUS_NOTALLSETTABLE));
				}
			}
			else
			{
				if(!(pProp->dwStatus == dwInputStatus))
				{
					// The provider doesn't necessarily support all valid values, so warn
					if (eValue == INDETERMINATE_VALUE && pProp->dwStatus == DBPROPSTATUS_BADVALUE)
					{
						TWARNING("Status returned: " << GetPropStatusName(pProp->dwStatus) << " Status expected: " << GetPropStatusName(dwInputStatus));
					}
					// Otherwise error
					else
						TERROR("Status returned: " << GetPropStatusName(pProp->dwStatus) << " Status expected: " << GetPropStatusName(dwInputStatus));
				}
			}
		}
		else
		{
			if(dwFlags & DBPROPFLAGS_COLUMNOK)
			{
				//Both of the following errors apply for optional - as the spec doesn't mandate order
				if(!(pProp->dwStatus == dwInputStatus || pProp->dwStatus == DBPROPSTATUS_NOTSET || pProp->dwStatus == DBPROPSTATUS_NOTALLSETTABLE))
				{
					// The provider doesn't necessarily support all valid values, so warn
					if (eValue == INDETERMINATE_VALUE && pProp->dwStatus == DBPROPSTATUS_BADVALUE)
					{
						TWARNING("Status returned: " << GetPropStatusName(pProp->dwStatus) << " Status expected: " << GetPropStatusName(dwInputStatus != DBPROPSTATUS_OK ? dwInputStatus : DBPROPSTATUS_NOTSET));
					}
					// Otherwise error
					else
						TERROR("Status returned: " << GetPropStatusName(pProp->dwStatus) << " Status expected: " << GetPropStatusName(dwInputStatus != DBPROPSTATUS_OK ? dwInputStatus : DBPROPSTATUS_NOTSET));
				}
			}
			else
			{
				if(!(pProp->dwStatus == dwInputStatus || pProp->dwStatus == DBPROPSTATUS_NOTSET))
				{
					// The provider doesn't necessarily support all valid values, so warn
					if (eValue == INDETERMINATE_VALUE && pProp->dwStatus == DBPROPSTATUS_BADVALUE)
					{
						TWARNING("Status returned: " << GetPropStatusName(pProp->dwStatus) << " Status expected: " << GetPropStatusName(dwInputStatus != DBPROPSTATUS_OK ? dwInputStatus : DBPROPSTATUS_NOTSET));
					}
					// Otherwise error
					else
						TERROR("Status returned: " << GetPropStatusName(pProp->dwStatus) << " Status expected: " << GetPropStatusName(dwInputStatus != DBPROPSTATUS_OK ? dwInputStatus : DBPROPSTATUS_NOTSET));
				}
			}
		}
	}
	
CLEANUP:	
	TRETURN
}



////////////////////////////////////////////////////////////////////////////
//  GetPropInfoType
//
////////////////////////////////////////////////////////////////////////////
VARTYPE GetPropInfoType(DBPROPID dwPropertyID, GUID guidPropertySet)
{
	TBEGIN
	DBPROPINFO* pPropInfo = NULL;
	VARTYPE vtType = 0;

	pPropInfo = GetPropInfo(dwPropertyID, guidPropertySet);
	TESTC(pPropInfo != NULL);
	
	//Get the value		
	vtType = pPropInfo->vtType;

CLEANUP:
	FreePropInfos(1, pPropInfo);
	return vtType;
}

////////////////////////////////////////////////////////////////////////////
//  GetPropInfo
//
////////////////////////////////////////////////////////////////////////////
DBPROPINFO* GetPropInfo(DBPROPID dwPropertyID, GUID guidPropertySet)
{
	return GetPropInfo(dwPropertyID, guidPropertySet, g_pIDBInitialize);
}

////////////////////////////////////////////////////////////////////////////
//  GetPropInfoFlags
//
////////////////////////////////////////////////////////////////////////////
DBPROPFLAGS GetPropInfoFlags(DBPROPID dwPropertyID, GUID guidPropertySet)
{
	return GetPropInfoFlags(dwPropertyID, guidPropertySet, g_pIDBInitialize);
}


////////////////////////////////////////////////////////////////////////////
//  GetPropDesc
//
////////////////////////////////////////////////////////////////////////////
WCHAR* GetPropDesc(DBPROPID dwPropertyID, GUID guidPropertySet, IUnknown* pDataSource)
{
	TBEGIN
	WCHAR* pwszDesc = NULL;

	//Default to global DataSource...
	if(pDataSource == NULL)
		pDataSource = g_pIDBInitialize;
	
	//Obtain Property Info for this Property
	DBPROPINFO* pPropInfo = GetPropInfo(dwPropertyID, guidPropertySet, pDataSource);
	TESTC(pPropInfo != NULL);

	//The Description is already allocated by GetPropInfo method...
	pwszDesc = pPropInfo->pwszDescription;

CLEANUP:
	FreePropInfos(1, pPropInfo);
	return pwszDesc;
}


////////////////////////////////////////////////////////////////////////////
//  SupportedProperty
//
////////////////////////////////////////////////////////////////////////////
BOOL SupportedProperty(DBPROPID dwPropertyID, GUID guidPropertySet)
{
	return GetPropInfoFlags(dwPropertyID, guidPropertySet) != DBPROPFLAGS_NOTSUPPORTED; 
}

////////////////////////////////////////////////////////////////////////////
//  SettableProperty
//
////////////////////////////////////////////////////////////////////////////
BOOL SettableProperty(DBPROPID dwPropertyID, GUID guidPropertySet)
{
	return GetPropInfoFlags(dwPropertyID, guidPropertySet) & DBPROPFLAGS_WRITE ? TRUE : FALSE;
}



////////////////////////////////////////////////////////////////////////////
//  SetSupportedProperty
//
////////////////////////////////////////////////////////////////////////////
BOOL SetSupportedProperty(DBPROPID dwPropertyID, GUID guidPropertySet, ULONG* pcPropSets, DBPROPSET** prgPropSets, void* pValue, DBTYPE wType, DBPROPOPTIONS dwOptions, DBID colid)
{
	//Set only if supported and settable, otherwise return FALSE
	if(SupportedProperty(dwPropertyID, guidPropertySet)) 
		return SetProperty(dwPropertyID, guidPropertySet, pcPropSets, prgPropSets, pValue, wType, dwOptions, colid);

	return FALSE;
}


////////////////////////////////////////////////////////////////////////////
//  SetSettableProperty
//
////////////////////////////////////////////////////////////////////////////
BOOL SetSettableProperty(DBPROPID dwPropertyID, GUID guidPropertySet, ULONG* pcPropSets, DBPROPSET** prgPropSets, void* pValue, DBTYPE wType, DBPROPOPTIONS dwOptions, DBID colid)
{
	//Set only if supported and settable, otherwise return FALSE
	if(SettableProperty(dwPropertyID, guidPropertySet)) 
		return SetProperty(dwPropertyID, guidPropertySet, pcPropSets, prgPropSets, pValue, wType, dwOptions, colid);

	return FALSE;
}

////////////////////////////////////////////////////////////////////////////
//  Set one property of the passed in Set
//
////////////////////////////////////////////////////////////////////////////
BOOL SetProperty(DBPROPID dwPropertyID, GUID guidPropertySet, ULONG* pcPropIDSets, DBPROPIDSET** prgPropIDSets)
{
	TBEGIN
	ASSERT(dwPropertyID && pcPropIDSets && prgPropIDSets);
	
	//Make our lives a little easier
	DBPROPIDSET* rgPropIDSets = *prgPropIDSets;
	ULONG cPropIDSets = *pcPropIDSets;

	ULONG iPropIDSet = ULONG_MAX;
	
	//Find the correct PropSet structure to add the property to
	for(ULONG i=0; i<cPropIDSets; i++)
		if(guidPropertySet == rgPropIDSets[i].guidPropertySet)
			iPropIDSet = i;

	//Do we need to create another PropSets structure for this property
	if(iPropIDSet==ULONG_MAX)
	{
		iPropIDSet = cPropIDSets;
		rgPropIDSets = PROVIDER_REALLOC_(rgPropIDSets,cPropIDSets+1,DBPROPIDSET);
		rgPropIDSets[iPropIDSet].cPropertyIDs = 0;
		rgPropIDSets[iPropIDSet].rgPropertyIDs = NULL;
		rgPropIDSets[iPropIDSet].guidPropertySet = guidPropertySet;
		cPropIDSets++;
	}

	//Now make our lives really easy
	DBPROPID* rgPropertyIDs = rgPropIDSets[iPropIDSet].rgPropertyIDs;
	ULONG cPropertyIDs = rgPropIDSets[iPropIDSet].cPropertyIDs;

	//do we need to enlarge the list
	rgPropertyIDs = PROVIDER_REALLOC_(rgPropertyIDs,cPropertyIDs+1,DBPROPID);
	
	//Add the new property to the list
	rgPropertyIDs[cPropertyIDs] = dwPropertyID;
	//Increment the number of properties
	cPropertyIDs++;

	//Now go back to the rediculous property structures
	rgPropIDSets[iPropIDSet].rgPropertyIDs = rgPropertyIDs;
	rgPropIDSets[iPropIDSet].cPropertyIDs  = cPropertyIDs;
	*prgPropIDSets = rgPropIDSets;
	*pcPropIDSets = cPropIDSets;

	TRETURN
}


////////////////////////////////////////////////////////////////////////////
//  Set one property of the passed in PropIDSet
//
////////////////////////////////////////////////////////////////////////////
BOOL SetProperty(DBPROPID dwPropertyID, GUID guidPropertySet, ULONG* pcPropSets, DBPROPSET** prgPropSets, void* pValue, DBTYPE wType, DBPROPOPTIONS dwOptions, DBID colid)
{
	TBEGIN
	ASSERT(prgPropSets && pcPropSets);
	
	//Make our lives a little easier
	DBPROPSET* rgPropSets = *prgPropSets;
	ULONG cPropSets = *pcPropSets;
	
	ULONG iPropSet = ULONG_MAX;
	
	//Find the correct PropSet structure to add the property to
	for(ULONG i=0; i<cPropSets; i++)
		if(guidPropertySet == rgPropSets[i].guidPropertySet)
			iPropSet = i;

	//Do we need to create another PropSets structure for this property
	if(iPropSet==ULONG_MAX)
	{
		iPropSet = cPropSets;
		rgPropSets = PROVIDER_REALLOC_(rgPropSets,cPropSets+1,DBPROPSET);
		rgPropSets[iPropSet].cProperties = 0;
		rgPropSets[iPropSet].rgProperties = NULL;
		rgPropSets[iPropSet].guidPropertySet = guidPropertySet;
		cPropSets++;
	}

	//Now make our lives really easy
	DBPROP* rgProperties = rgPropSets[iPropSet].rgProperties;
	ULONG cProperties = rgPropSets[iPropSet].cProperties;

	//do we need to enlarge the list
	rgProperties = PROVIDER_REALLOC_(rgProperties,cProperties+1,DBPROP);
	
	//Add the new property to the list
	rgProperties[cProperties].dwPropertyID = dwPropertyID;
	rgProperties[cProperties].dwOptions    = dwOptions;
	//rgProperties[cProperties].colid        = colid;
	CHECK(DuplicateDBID(colid, &rgProperties[cProperties].colid), S_OK);

	//Status is supposed to be ignored on input
	rgProperties[cProperties].dwStatus     = INVALID(DBPROPSTATUS);

	//Variant property value
	CreateVariant(&rgProperties[cProperties].vValue, wType, pValue);
	
	//Increment the number of properties
	cProperties++;

	//Now go back to the rediculous property structures
	rgPropSets[iPropSet].rgProperties = rgProperties;
	rgPropSets[iPropSet].cProperties  = cProperties;
	*prgPropSets = rgPropSets;
	*pcPropSets = cPropSets;

	TRETURN
}


/////////////////////////////////////////////////////////////////////////////
// HRESULT CreateFromDisplayName
//
/////////////////////////////////////////////////////////////////////////////
HRESULT CreateFromDisplayName(IParseDisplayName* pIParseDisplayName, WCHAR* pwszDisplayName, REFIID riid, IUnknown** ppIUnknown)
{
	HRESULT hr = S_OK;
	IMoniker* pIMoniker = NULL;
	ULONG chEaten = 0;

	//Obtain RootEnumerator if no enumerator was passed in...
	SAFE_ADDREF(pIParseDisplayName);
	if(pIParseDisplayName == NULL)
		TESTC_(hr = CoCreateInstance(CLSID_OLEDB_ENUMERATOR, NULL, CLSCTX_INPROC_SERVER, IID_IParseDisplayName, (void**)&pIParseDisplayName),S_OK);

	//ParseDisplayName
	TESTC_(hr = pIParseDisplayName->ParseDisplayName(NULL, pwszDisplayName, &chEaten, &pIMoniker),S_OK);
	TESTC_(hr = BindMoniker(pIMoniker, 0, riid, (void**)ppIUnknown),S_OK);

CLEANUP:
	//NOTE:  We AddRef'd this pointer at the begining of this function
	//so we need to release it...
	SAFE_RELEASE(pIParseDisplayName);
	SAFE_RELEASE(pIMoniker);
	return hr;
}


/////////////////////////////////////////////////////////////////////////////
// HRESULT GetEnumInfo
//
/////////////////////////////////////////////////////////////////////////////
HRESULT GetEnumInfo(IParseDisplayName* pIParseDisplayName, ULONG* pcEnumInfo, ENUMINFO** prgEnumInfo)
{
	HRESULT hr;
	ASSERT(pcEnumInfo);
	ASSERT(prgEnumInfo);

	HROW rghRows[MAX_NAME_LEN];
	HROW* phRows = rghRows;
	DBCOUNTITEM cRowsObtained = 0;
	IRowset* pIRowset = NULL;

	IAccessor* pIAccessor = NULL;
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	ULONG ulRefCount = 0;

	//Make our lives easier
	ULONG cEnumInfo = 0;
	ENUMINFO* rgEnumInfo = NULL;
	ISourcesRowset* pISourcesRowset = NULL;
	
	// Bind the user and table name for the list
	const static DBBINDING rgBindings[] = 
		{
			1,	 			
			offsetof(ENUMINFO, wszName),
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
			offsetof(ENUMINFO, wszParseName),
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
			offsetof(ENUMINFO, wszDescription),
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
			offsetof(ENUMINFO, wType),
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

			5,	 			
			offsetof(ENUMINFO, fIsParent),
			0,
			0,	
			NULL,			
			NULL, 		
			NULL,		
			DBPART_VALUE,
			DBMEMOWNER_CLIENTOWNED,		
			DBPARAMIO_NOTPARAM, 
			sizeof(VARIANT_BOOL), 
			0, 				
			DBTYPE_BOOL, 	
			0,	
			0, 				
	};
	const static DBCOUNTITEM cBindings = NUMELEM(rgBindings);

	//Obtain RootEnumerator if no enumerator was passed in...
	SAFE_ADDREF(pIParseDisplayName);
	if(pIParseDisplayName == NULL)
		TESTC_(hr = CoCreateInstance(CLSID_OLEDB_ENUMERATOR, NULL, CLSCTX_INPROC_SERVER, IID_IParseDisplayName, (void**)&pIParseDisplayName),S_OK);

	//Obtain ISourcesRowset interface
	TESTC_(hr = pIParseDisplayName->QueryInterface(IID_ISourcesRowset, (void**)&pISourcesRowset),S_OK);
	TESTC_(hr = pISourcesRowset->GetSourcesRowset(NULL, IID_IRowset, 0, NULL, (IUnknown**)&pIRowset),S_OK);

	//Create Accessor
	TESTC_(hr = pIRowset->QueryInterface(IID_IAccessor, (void**)&pIAccessor),S_OK);
	TESTC_(hr = pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA, cBindings, rgBindings, 0, &hAccessor, NULL),S_OK);

	//Loop through the entire returned rowet
	while(TRUE)
	{
		hr = pIRowset->GetNextRows(NULL, 0, MAX_NAME_LEN, &cRowsObtained, &phRows);
		TESTC(hr==S_OK || hr==DB_S_ENDOFROWSET || hr==DB_S_ROWLIMITEXCEEDED);
		hr = S_OK;

		//ENDOFROWSET
		if(FAILED(hr) || cRowsObtained==0) 
			break;
		
		//Alloc room for ProviderInfo (in chunks)
		SAFE_REALLOC(rgEnumInfo, ENUMINFO, cEnumInfo + cRowsObtained);
		memset(&rgEnumInfo[cEnumInfo], 0, (size_t)(sizeof(ENUMINFO)*cRowsObtained));

		//Loop over rows obtained and get ProviderInfo
		for(DBCOUNTITEM i=0; i<cRowsObtained; i++) 
		{	
			//Get the Data
			TESTC_(hr = pIRowset->GetData(rghRows[i], hAccessor, (void*)&rgEnumInfo[cEnumInfo]), S_OK);
			cEnumInfo++;
		}
			
		//Release all the rows
		TESTC_(hr = pIRowset->ReleaseRows(cRowsObtained, rghRows, NULL, NULL, NULL),S_OK);
	}

	//NOTE:  The Root Enumerator doesn't include itself in the List of Enumerators
	//This is good for the case of tree controls would be never ending, but bad
	//for apps like this one, where you might want to see the "rowset" of the Root
	//Enum.  So to get arround this I will just add it to the list manually

	//Alloc room for extra MSDAENUM additon
	SAFE_REALLOC(rgEnumInfo, ENUMINFO, cEnumInfo + 1);
	wcscpy(rgEnumInfo[cEnumInfo].wszName,		L"MSDAENUM");
	wcscpy(rgEnumInfo[cEnumInfo].wszParseName,	L"{c8b522d0-5cf3-11ce-ade5-00aa0044773d}");
	wcscpy(rgEnumInfo[cEnumInfo].wszDescription,L"Microsoft OLE DB Root Enumerator");
	rgEnumInfo[cEnumInfo].wType					= DBSOURCETYPE_ENUMERATOR;
	rgEnumInfo[cEnumInfo].fIsParent				= VARIANT_TRUE;
	cEnumInfo++;

CLEANUP:
	//Output Params
	*pcEnumInfo = cEnumInfo;
	*prgEnumInfo = rgEnumInfo;
	
	if(hAccessor && pIAccessor)
		pIAccessor->ReleaseAccessor(hAccessor, &ulRefCount);

	SAFE_RELEASE(pISourcesRowset);
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pIAccessor);
	
	//NOTE:  We AddRef'd this pointer at the begining of this function
	//so we need to release it...
	SAFE_RELEASE(pIParseDisplayName);
	return hr;
}


//////////////////////////////////////////////////////////////////////////
// Class CDataSource
//
// Basically a nice wrapper arround a DataSource object.  
//////////////////////////////////////////////////////////////////////////
CDataSource::CDataSource(WCHAR* pwszTestCaseName) : CDataSourceObject(pwszTestCaseName)
{
	m_cPropSets      = 0;
	m_rgPropSets     = NULL;				

	if(!m_fInsideTestCase)
		CDataSource::Init();
}

CDataSource::~CDataSource()
{
	CDataSource::Terminate();
}

BOOL CDataSource::Init()
{
	return CDataSourceObject::Init();
}

BOOL CDataSource::Terminate()
{
    FreeProperties();
	ReleaseDataSourceObject();
	return CDataSourceObject::Terminate();
}

IDBInitialize* const CDataSource::pIDBInit()
{
	return m_pIDBInitialize;
}


IUnknown* const CDataSource::pDataSource()
{
	return pIDBInit();
}

IDBInitialize* const CDataSource::operator()()
{
	return pIDBInit();
}

HRESULT CDataSource::CreateInstance()
{
	//Delegate
	return CDataSourceObject::CreateDataSourceObject();
}

HRESULT CDataSource::Initialize(IUnknown* pIUnknowon, EREINITIALIZE eReinitialize)
{
	//Use the DataSource pointer passed in...
	SetDataSourceObject(pIUnknowon); 
	
	//Delegate
	return CDataSourceObject::InitializeDSO(eReinitialize, m_cPropSets, m_rgPropSets);
}

HRESULT CDataSource::Initialize(EREINITIALIZE eReinitialize)
{
	//Create Instance (if we haven't already)
	CreateInstance();
	
	//Delegate
	return CDataSourceObject::InitializeDSO(eReinitialize, m_cPropSets, m_rgPropSets);
}

HRESULT CDataSource::Uninitialize()
{
	//Delegate
	return CDataSourceObject::UninitializeDSO();
}

BOOL CDataSource::SetSupportedProperty(DBPROPID dwPropertyID, GUID guidPropertySet, void* pValue, DBTYPE wType, DBPROPOPTIONS dwOptions, DBID colid)
{
	if(guidPropertySet == DBPROPSET_ROWSET && (dwPropertyID == DBPROP_IRowsetChange || dwPropertyID == DBPROP_IRowsetUpdate) && (VARIANT_BOOL)pValue==VARIANT_TRUE)
		::SetSupportedProperty(DBPROP_UPDATABILITY,guidPropertySet,&m_cPropSets,&m_rgPropSets,(void*)DBPROPVAL_UP_ALL,DBTYPE_I4,dwOptions,colid);
	
	return ::SetSupportedProperty(dwPropertyID, guidPropertySet, &m_cPropSets, &m_rgPropSets, pValue, wType, dwOptions, colid);
}
BOOL CDataSource::SetSettableProperty(DBPROPID dwPropertyID, GUID guidPropertySet, void* pValue, DBTYPE wType, DBPROPOPTIONS dwOptions, DBID colid)
{
	if(guidPropertySet == DBPROPSET_ROWSET && (dwPropertyID == DBPROP_IRowsetChange || dwPropertyID == DBPROP_IRowsetUpdate) && (VARIANT_BOOL)pValue==VARIANT_TRUE)
		::SetSettableProperty(DBPROP_UPDATABILITY,guidPropertySet,&m_cPropSets,&m_rgPropSets,(void*)DBPROPVAL_UP_ALL,DBTYPE_I4,dwOptions,colid);
	
	return ::SetSettableProperty(dwPropertyID, guidPropertySet, &m_cPropSets, &m_rgPropSets, pValue, wType, dwOptions, colid);
}
BOOL CDataSource::SetProperty(DBPROPID dwPropertyID, GUID guidPropertySet, void* pValue, DBTYPE wType, DBPROPOPTIONS dwOptions, DBID colid)
{
	if(guidPropertySet == DBPROPSET_ROWSET && (dwPropertyID == DBPROP_IRowsetChange || dwPropertyID == DBPROP_IRowsetUpdate) && (VARIANT_BOOL)pValue==VARIANT_TRUE)
		::SetProperty(DBPROP_UPDATABILITY, guidPropertySet, &m_cPropSets, &m_rgPropSets, (void*)DBPROPVAL_UP_ALL, DBTYPE_I4, dwOptions, colid);
	
	return ::SetProperty(dwPropertyID, guidPropertySet, &m_cPropSets,&m_rgPropSets, pValue, wType, dwOptions, colid);
}
BOOL CDataSource::FreeProperties()
{
	return ::FreeProperties(&m_cPropSets,&m_rgPropSets);
}


HRESULT CDataSource::GetPropertyInfo(GUID guidPropertySet, ULONG* pcPropInfoSets, DBPROPINFOSET** prgPropInfoSets, WCHAR** ppwszStringBuffer)
{
	TBEGIN
	HRESULT hr = S_OK;
	IDBProperties* pIDBProperties = NULL;
	ULONG cPropInfoSets = 0;
	DBPROPINFOSET* rgPropInfoSets = NULL;
	WCHAR* pwszStringBuffer = NULL;

	const ULONG cPropertyIDSets = 1;
	DBPROPIDSET rgPropertyIDSets[cPropertyIDSets];
	rgPropertyIDSets[0].cPropertyIDs = 0;
	rgPropertyIDSets[0].rgPropertyIDs = NULL;
	rgPropertyIDSets[0].guidPropertySet = guidPropertySet;

	//Obtain IDBProperties
	TESTC_(GetDataSourceObject(IID_IDBProperties, (IUnknown**)&pIDBProperties),S_OK);

	//GetPropertyInfo
	TESTC_(pIDBProperties->GetPropertyInfo(cPropertyIDSets, rgPropertyIDSets, &cPropInfoSets, &rgPropInfoSets, &pwszStringBuffer),S_OK);

CLEANUP:
	if(pcPropInfoSets)
		*pcPropInfoSets = cPropInfoSets;
	if(prgPropInfoSets)
		*prgPropInfoSets = rgPropInfoSets;
	else
		::FreeProperties(&cPropInfoSets, &rgPropInfoSets);
	if(ppwszStringBuffer)
		*ppwszStringBuffer = pwszStringBuffer;
	else
		SAFE_FREE(pwszStringBuffer);
	SAFE_RELEASE(pIDBProperties);
	return hr;
}


////////////////////////////////////////////////////////////////////////////
//  CDataSource::VerifyDataSource
//
////////////////////////////////////////////////////////////////////////////
BOOL CDataSource::VerifyDataSource(IUnknown* pDataSource, REFCLSID rclsid)
{
	TBEGIN
	IPersist* pIPersist = NULL;
	CLSID clsidProv;

	//No-op
	QTESTC(pDataSource != NULL);

	//Get the CLSID for the passed in DataSource
	QTESTC(VerifyInterface(pDataSource, IID_IPersist, DATASOURCE_INTERFACE, (IUnknown**)&pIPersist));

	//Verify the CLSID
	QTESTC_(pIPersist->GetClassID(&clsidProv),S_OK);
	QTESTC(memcmp(&clsidProv, &rclsid, sizeof(CLSID))==0);

CLEANUP:
	SAFE_RELEASE(pIPersist);
	TRETURN
}

	


////////////////////////////////////////////////////////////////////////////
//  CDataSource::VerifyDataSource
//
////////////////////////////////////////////////////////////////////////////
BOOL CDataSource::VerifyDataSource(IUnknown* pDataSource, DBPROPID dwPropertyID, GUID guidPropertySet, DBTYPE wType, void* pValue)
{
	TBEGIN
	VARIANT vProvider;
	VARIANT vConsumer;
	VariantInit(&vProvider);
	VariantInit(&vConsumer);

	//no-op
	QTESTC(pDataSource != NULL);
		
	//We need to make sure that the specified DataSource contains the property
	QTESTC(GetProperty(dwPropertyID, guidPropertySet, pDataSource, &vProvider));

	//Create a Variant from the Passed in Values...
	QTESTC_(CreateVariant(&vConsumer, wType, pValue),S_OK);

	//Now Verify they are Equal
	if(!CompareVariant(&vProvider, &vConsumer))
	{
		WCHAR wszProvider[MAX_QUERY_LEN];
		WCHAR wszConsumer[MAX_QUERY_LEN];
		
		//Display Differences...
		VariantToString(&vProvider, wszProvider, MAX_QUERY_LEN, FALSE);
		VariantToString(&vConsumer, wszConsumer, MAX_QUERY_LEN, FALSE);

		//We allow default property values, meaning even though VT_EMPTY was specified
		//the provider can use that to mean to set a default value...
		if((wType==VT_EMPTY || (wType==VT_VARIANT && V_VT(&vConsumer)==VT_EMPTY)) 
			&& V_VT(&vProvider)!=VT_EMPTY)
		{
			//We should display the default value since
			//the is no real programmatic way to verify this...
			if(GetModInfo()->GetDebugMode() & DEBUGMODE_DIALOGS)
			{
				TWARNING("Default Value \"" << wszProvider << "\" for Property \"" << GetStaticPropDesc(dwPropertyID, guidPropertySet) << "\" returned when VT_EMPTY specified");
			}
		}
		else
		{
			TERROR(L"Property values for \"" << GetStaticPropDesc(dwPropertyID, guidPropertySet) << "\" don't match:  Expected: \"" << wszConsumer << "\" Actual: \"" << wszProvider << "\"");
			QTESTC(FALSE);
		}
	}

CLEANUP:
	VariantClear(&vProvider);
	VariantClear(&vConsumer);
	TRETURN
}




//////////////////////////////////////////////////////////////////
// Class COpenRowset
//
// Used to create multiple instantes of CTestCases even though our
// privlib is really not designed for it, quite a useful hack...
//////////////////////////////////////////////////////////////////

COpenRowset::COpenRowset(WCHAR* pwszTestCaseName) :	CSessionObject(pwszTestCaseName) 
{
	m_cPropSets      = 0;
	m_rgPropSets     = NULL;				

	if(!m_fInsideTestCase)
		COpenRowset::Init();
}

COpenRowset::~COpenRowset() 
{
	COpenRowset::Terminate();
}

BOOL COpenRowset::Init()
{
	TBEGIN

    //Init baseclass
	TESTC(CSessionObject::Init())
	
	//Use the global DSO created in Module init
	SetDataSourceObject(g_pIDBInitialize); 

	//Use the global DB Rowset created in Module init
	SetDBSession(g_pIOpenRowset); 

	//Use the global CTable created in Module init, by default
	SetTable(g_pTable, DELETETABLE_NO);		

    //Create the DB Session, and use IOpenRowset not the Command Object
    TESTC_(CreateDBSession(OPENROWSET_GENERATED),S_OK)

CLEANUP:
	TRETURN
}
                                                
BOOL COpenRowset::Terminate()
{
    FreeProperties();
    ReleaseDBSession();
	ReleaseDataSourceObject();
	return CSessionObject::Terminate();
}

IOpenRowset* const COpenRowset::pIOpenRowset()
{
	ASSERT(m_pIOpenRowset);
	return m_pIOpenRowset;
}
IOpenRowset* const COpenRowset::operator()()
{
	return pIOpenRowset();
}

IUnknown*	const COpenRowset::pISession()
{
	ASSERT(m_pIOpenRowset);
	return m_pIOpenRowset;
}

HRESULT COpenRowset::CreateOpenRowset(IID riid, IUnknown** ppRowset, IUnknown* pIUnkOuter, EINTERFACE eInterface)
{
    return CreateOpenRowset(m_pTable, riid, ppRowset, pIUnkOuter, eInterface); 
}


HRESULT COpenRowset::CreateOpenRowset(CTable* pCTable, IID riid, IUnknown** ppRowset, IUnknown* pIUnkOuter, EINTERFACE eInterface)
{
    ASSERT(pCTable);
	TBEGIN
	HRESULT hr = S_OK;

	//Use CursorEngine ( if requested )
	if(GetModInfo()->UseServiceComponents() & SERVICECOMP_CURSORENGINE)
		SetProperty(DBPROP_CLIENTCURSOR, DBPROPSET_ROWSET);

	//This method implicity uses m_cProperties/m_rgProperties.
	//So if you want particular properties set, just simply set them before hand
	//then call this method.

    //Call OpenRowset  
	hr = pIOpenRowset()->OpenRowset(pIUnkOuter, &pCTable->GetTableID(), NULL, riid, m_cPropSets, m_rgPropSets, ppRowset);
    
	//Verify the property results...
	TESTC(VerifyProperties(hr, m_cPropSets, m_rgPropSets));

	//Verify Results
    if(SUCCEEDED(hr))
    {
        //Verify Usable Rowset
        if(ppRowset && riid!=IID_NULL)  
        {  
            TESTC(*ppRowset!=NULL);
            TESTC(ValidInterface(riid, *ppRowset));

			//Do some default testing of the object returned
			TESTC(DefaultObjectTesting(*ppRowset, eInterface));
		} 
        
        //Verify no errors
        if(hr==S_OK) 
			TESTC(VerifyPropSetStatus(m_cPropSets, m_rgPropSets))
    }
    
	if(FAILED(hr) || riid==IID_NULL)
    {
        //Verify no rowset returned
        if(ppRowset) 
            TESTC(*ppRowset==NULL)
    }

CLEANUP:
	return hr;
}

BOOL COpenRowset::SetSupportedProperty(DBPROPID dwPropertyID, GUID guidPropertySet, void* pValue, DBTYPE wType, DBPROPOPTIONS dwOptions, DBID colid)
{
	if(guidPropertySet == DBPROPSET_ROWSET && (dwPropertyID == DBPROP_IRowsetChange || dwPropertyID == DBPROP_IRowsetUpdate) && (VARIANT_BOOL)pValue==VARIANT_TRUE)
		::SetSupportedProperty(DBPROP_UPDATABILITY,guidPropertySet,&m_cPropSets,&m_rgPropSets,(void*)DBPROPVAL_UP_ALL,DBTYPE_I4,dwOptions,colid);
	
	return ::SetSupportedProperty(dwPropertyID, guidPropertySet, &m_cPropSets, &m_rgPropSets, pValue, wType, dwOptions, colid);
}
BOOL COpenRowset::SetSettableProperty(DBPROPID dwPropertyID, GUID guidPropertySet, void* pValue, DBTYPE wType, DBPROPOPTIONS dwOptions, DBID colid)
{
	if(guidPropertySet == DBPROPSET_ROWSET && (dwPropertyID == DBPROP_IRowsetChange || dwPropertyID == DBPROP_IRowsetUpdate) && (VARIANT_BOOL)pValue==VARIANT_TRUE)
		::SetSettableProperty(DBPROP_UPDATABILITY,guidPropertySet,&m_cPropSets,&m_rgPropSets,(void*)DBPROPVAL_UP_ALL,DBTYPE_I4,dwOptions,colid);
	
	return ::SetSettableProperty(dwPropertyID, guidPropertySet, &m_cPropSets, &m_rgPropSets, pValue, wType, dwOptions, colid);
}
BOOL COpenRowset::SetProperty(DBPROPID dwPropertyID, GUID guidPropertySet, void* pValue, DBTYPE wType, DBPROPOPTIONS dwOptions, DBID colid)
{
	if(guidPropertySet == DBPROPSET_ROWSET && (dwPropertyID == DBPROP_IRowsetChange || dwPropertyID == DBPROP_IRowsetUpdate) && (VARIANT_BOOL)pValue==VARIANT_TRUE)
		::SetProperty(DBPROP_UPDATABILITY, guidPropertySet, &m_cPropSets, &m_rgPropSets, (void*)DBPROPVAL_UP_ALL, DBTYPE_I4, dwOptions, colid);
	
	return ::SetProperty(dwPropertyID, guidPropertySet, &m_cPropSets,&m_rgPropSets, pValue, wType, dwOptions, colid);
}
BOOL COpenRowset::FreeProperties()
{
	return ::FreeProperties(&m_cPropSets,&m_rgPropSets);
}




//////////////////////////////////////////////////////////////////////////
// Class CCommand
//
// Basically a nice wrapper arround a Command object.  
//////////////////////////////////////////////////////////////////////////
CCommand::CCommand(WCHAR* pwszTestCaseName) : CCommandObject(pwszTestCaseName)
{
	m_pICommandText			= NULL;
	m_pICommandProperties	= NULL;
	
	//Properties
	m_cPropSets      = 0;
	m_rgPropSets     = NULL;

	if(!m_fInsideTestCase)
		CCommand::Init();
}

CCommand::~CCommand()
{
	CCommand::Terminate();
}

BOOL CCommand::Init()
{
	TBEGIN

	//Init baseclass
	TESTC(CCommandObject::Init())
	
	//Use the global DSO created in Module init
	SetDataSourceObject(g_pIDBInitialize); 

	//Use the global DB Rowset created in Module init
	SetDBSession(g_pIOpenRowset); 

	//Use the global CTable created in Module init, by default
	SetTable(g_pTable, DELETETABLE_NO);		

	//Use the global C1RowTable for the second table, if ever needed
	SetTable2(g_p1RowTable, DELETETABLE_NO);

CLEANUP:
	TRETURN
}


BOOL CCommand::Terminate()
{
	SAFE_RELEASE(m_pICommandText);
	SAFE_RELEASE(m_pICommandProperties);
	FreeProperties();

	ReleaseCommandObject(); //releases m_pICommand
	ReleaseDBSession();
	ReleaseDataSourceObject();
	return CCommandObject::Terminate();
}


HRESULT CCommand::CreateCommand(IUnknown* pIUnkOuter)
{
	TBEGIN
	HRESULT hr = S_OK;
	
	//Delegate
	QTESTC_(hr = CCommandObject::CreateCommandObject(pIUnkOuter),S_OK);

	//Obtiain other mandatory interfaces
	TESTC_(hr = QI(m_pICommand, IID_ICommandText,		(void**)&m_pICommandText),S_OK);
	TESTC_(hr = QI(m_pICommand, IID_ICommandProperties, (void**)&m_pICommandProperties),S_OK);

CLEANUP:
	return hr;
}

HRESULT CCommand::SetCommandText(EQUERY eQuery, REFGUID rguidDialect)
{
	TBEGIN
	HRESULT hr = S_OK;
	WCHAR* pwszText = NULL;
	
	//Obtain the SQL Statement to set...
	TESTC_(hr = m_pTable->CreateSQLStmt(eQuery, NULL, &pwszText, NULL, NULL),S_OK);

	//Set the Command Text...
	QTESTC_(hr = m_pICommandText->SetCommandText(rguidDialect, pwszText),S_OK);

CLEANUP:
	SAFE_FREE(pwszText);
	return hr;
}

HRESULT CCommand::SetProperties()
{
	TBEGIN
	HRESULT hr = S_OK;
	
	//Set the Properties (from the cache)
	QTESTC_(hr = m_pICommandProperties->SetProperties(m_cPropSets, m_rgPropSets),S_OK);

CLEANUP:
	return hr;
}

HRESULT CCommand::Execute(IUnknown* pUnkOuter, REFIID riid, IUnknown** ppRowset, DBROWCOUNT* pcRowsAffected, DBPARAMS* pParams)
{
	TBEGIN
	HRESULT hr = S_OK;

	//Delegate
	QTESTC_(hr = m_pICommandText->Execute(pUnkOuter, riid, pParams, pcRowsAffected, ppRowset),S_OK);

CLEANUP:
	return hr;
}
	
BOOL CCommand::SetProperty(DBPROPID dwPropertyID, GUID guidPropertySet, void* pValue, DBTYPE wType, DBPROPOPTIONS dwOptions, DBID colid)
{
	if(guidPropertySet == DBPROPSET_ROWSET && (dwPropertyID == DBPROP_IRowsetChange || dwPropertyID == DBPROP_IRowsetUpdate) && (VARIANT_BOOL)pValue==VARIANT_TRUE)
		::SetProperty(DBPROP_UPDATABILITY, guidPropertySet, &m_cPropSets, &m_rgPropSets, (void*)DBPROPVAL_UP_ALL, DBTYPE_I4, dwOptions, colid);
	
	return ::SetProperty(dwPropertyID, guidPropertySet, &m_cPropSets,&m_rgPropSets, pValue, wType, dwOptions, colid);
}
BOOL CCommand::FreeProperties()
{
	return ::FreeProperties(&m_cPropSets,&m_rgPropSets);
}




//////////////////////////////////////////////////////////////////////////
// Class CRowset
//
// Basically a nice wrapper arround a rowset object.  This test will use
// multiple rowsets, and its easier to have all the general functionality
// together
//////////////////////////////////////////////////////////////////////////


CRowset::CRowset(WCHAR* pwszTestCaseName) : CRowsetObject(pwszTestCaseName)
{
    //Protected
	m_hAccessor         = DB_NULL_HACCESSOR;
	m_cRowSize          = 0;
	m_cBindings			= 0;					   
	m_rgBinding 		= NULL;
	m_pData             = NULL;

	//CRowsetObject members, but we make use of them here
	m_cPropSets          = 0;
	m_rgPropSets         = NULL;

    //Private
    m_pIUnknown			= NULL;
    m_pIRowset			= NULL;

	m_ulMaxOpenRows			= 0;
	m_bCanHoldRows			= FALSE;
	m_bCanScrollBackwards	= FALSE;
	m_bCanFetchBackwards	= FALSE;
	m_ulpOleObjects			= 0;

	if(!m_fInsideTestCase)
		CRowset::Init();
}


CRowset::~CRowset()
{
	CRowset::Terminate();
}

BOOL CRowset::Init()
{
	TBEGIN

	//Init baseclass
	TESTC(CRowsetObject::Init())
	
	//Use the global DSO created in Module init
	SetDataSourceObject(g_pIDBInitialize); 

	//Use the global DB Rowset created in Module init
	SetDBSession(g_pIOpenRowset); 

	//Use the global CTable created in Module init, by default
	SetTable(g_pTable, DELETETABLE_NO);		

	//Use the global C1RowTable for the second table, if ever needed
	SetTable2(g_p1RowTable, DELETETABLE_NO);

CLEANUP:
	TRETURN
}
                                                
BOOL CRowset::Terminate()
{
	DropRowset();				//Rowset
	ReleaseDBSession();			//Session
	ReleaseDataSourceObject();	//DataSource
	return CRowsetObject::Terminate();
}


////////////////////////////////////////////////////////////////////////////
//  CRowset::DropRowset
//
////////////////////////////////////////////////////////////////////////////
HRESULT CRowset::DropRowset()
{
	FreeProperties();

	//Release
	ReleaseAccessor(m_hAccessor);
	SAFE_RELEASE(m_pIAccessor);  
	SAFE_RELEASE(m_pIUnknown);
	SAFE_RELEASE(m_pIRowset);    //Shouldn't be any references left

	//Delete m_pData
	PROVIDER_FREE(m_pData);
	PROVIDER_FREE(m_rgBinding);
	
	ReleaseRowsetObject();  //releases m_pIAccessor
	ReleaseCommandObject(); //releases m_pICommand
	return S_OK;
}


IUnknown* const CRowset::pIUnknown()
{
	return m_pIUnknown;
}


IRowset* const CRowset::pIRowset()
{
	if(!m_pIRowset)
		QI(m_pIAccessor,IID_IRowset,(void**)&m_pIRowset);

	return m_pIRowset;
}

ICommand* const CRowset::pICommand()
{
	if(!m_pICommand)
		CreateCommandObject();
	
	return m_pICommand;
}

IUnknown*	const CRowset::pISession()
{
	return m_pIOpenRowset;
}

CTable* const CRowset::pTable()
{
	return m_pTable;
}

IRowset* const CRowset::operator()()
{
	return pIRowset();
}

IAccessor* const CRowset::pIAccessor()
{
	if(!m_pIAccessor)
	{
		//Try to grab the Accessor from the rowset object
		if(m_pIRowset)
			QI(m_pIRowset,IID_IAccessor,(void**)&m_pIAccessor);
		//Otherwise just get it from the command object
		else
			QI(pICommand(),IID_IAccessor,(void**)&m_pIAccessor);
	}

	return m_pIAccessor;
}


HRESULT CRowset::CreateCommand(EQUERY eQuery)
{
	//Call execute to create the command and set the command text, but don't execute
	return pTable()->ExecuteCommand(eQuery,IID_NULL,NULL,NULL,NULL,NULL,EXECUTE_NEVER,0,NULL,NULL,NULL,&m_pICommand);
}


HRESULT CRowset::GetColInfo(DBORDINAL* pcColumns, DBCOLUMNINFO** prgInfo, WCHAR** ppStringsBuffer)
{
    //Can pass NULL for other args, if just want # of columns
	TBEGIN
	ASSERT(pcColumns && prgInfo);
		
	HRESULT hr = E_FAIL;
    WCHAR* pLocalStringsBuffer = NULL;
    IColumnsInfo* pIColumnsInfo = NULL; 
	DBORDINAL iCol = 0;

	//Obtain the ColumnsInfo interface
	TESTC_(hr = QI(pIRowset(),IID_IColumnsInfo,(void**)&pIColumnsInfo),S_OK)
	
	//GetColumnInfo	
	QTESTC_(hr = pIColumnsInfo->GetColumnInfo(pcColumns, prgInfo, &pLocalStringsBuffer),S_OK)
	TESTC(*pcColumns!=0 && *prgInfo!=NULL && pLocalStringsBuffer!=NULL)

	//May times we just want the flags or something and don't need the overhead
	//of the string buffer.  So if not requesting the string buffer, make sure
	//we null the column names, so they aren't incorrectly referenced...
	if(ppStringsBuffer == NULL)
	{
		for(iCol=0; iCol<*pcColumns; iCol++)
			(*prgInfo)[iCol].pwszName = NULL;
	}

CLEANUP:
    if(ppStringsBuffer)
        *ppStringsBuffer = pLocalStringsBuffer;
    else
    	PROVIDER_FREE(pLocalStringsBuffer);
        
	SAFE_RELEASE(pIColumnsInfo);
    return hr;
}


HRESULT CRowset::FindColInfo(DBCOLUMNFLAGS dwFlags, DBCOLUMNINFO* pColInfo)
{
    TBEGIN
	ASSERT(pColInfo);

	DBORDINAL i,cColumns = 0;
	DBCOLUMNINFO* rgColInfo = NULL;
	HRESULT hr = E_FAIL;

    //Obtain the Columns Info
    TESTC_(hr = GetColInfo(&cColumns, &rgColInfo),S_OK);

	//Find the Column Requested
	hr = E_FAIL;
	for(i=0; i<cColumns; i++)
	{
		if(dwFlags & rgColInfo[i].dwFlags)
		{
			*pColInfo = rgColInfo[i]; 
			hr = S_OK;
			break;
		}
	}

CLEANUP:
	PROVIDER_FREE(rgColInfo);
	return hr;
}


DBORDINAL CRowset::GetTotalColumns(BOOL fIncludeBookmark)
{
    TBEGIN	
    DBORDINAL cColumns =  0;
	DBCOLUMNINFO* rgColInfo = NULL;
	
	//Obtain the Columns Info
	TESTC_(GetColInfo(&cColumns, &rgColInfo),S_OK);

	//Bookmark column
	if(cColumns && fIncludeBookmark == FALSE)
	{
		if(rgColInfo[0].iOrdinal==0)
			cColumns--;
	}

CLEANUP:
	PROVIDER_FREE(rgColInfo);
	return cColumns;
}


BOOL CRowset::VerifyAllRows()
{
	TBEGIN
	DBCOUNTITEM cRowsObtained = 0;
	HROW* rghRows = NULL;

	//get the number of rows in the table
	DBCOUNTITEM ulRowCount = pTable()->CountRowsOnTable();

	//loop through the rowset, retrieve one row at a time
	for(ULONG i=1; i<=ulRowCount; i++)
	{
		//GetNextRow 
		TESTC_(GetNextRows(0, 1, &cRowsObtained, &rghRows),S_OK);
		
		//VerifyRowHandles
		TESTC(VerifyRowHandles(cRowsObtained, rghRows, i));

		//release the row handle
		TESTC_(ReleaseRows(cRowsObtained, rghRows),S_OK);
		PROVIDER_FREE(rghRows);
	}

 	//Verify the cursor is at the end of the rowset
	TESTC_(GetNextRows(0, 1, &cRowsObtained, &rghRows), DB_S_ENDOFROWSET);
	PROVIDER_FREE(rghRows);

	//Call again at end of rowset...
	TESTC_(GetNextRows(0, 1, &cRowsObtained, &rghRows), DB_S_ENDOFROWSET);
	TESTC(cRowsObtained==0 && rghRows == NULL);

CLEANUP:
	ReleaseRows(cRowsObtained, rghRows);
	PROVIDER_FREE(rghRows);
	TRETURN
}

BOOL CRowset::CompareRowset(IUnknown* pIRowset)
{
	//Invalidarg	
	if(pIRowset==NULL)
		return FALSE;
	
    TBEGIN
	CRowset RowsetA;

    //Create a rowset object from the pointer
    TESTC_(RowsetA.CreateRowset(pIRowset),S_OK)

    //make sure both have the same number of rows
    QCOMPC(GetTotalRows(), RowsetA.GetTotalRows())

    //make sure both have the same number of columns
    QCOMPC(GetTotalColumns(), RowsetA.GetTotalColumns())

CLEANUP:
    TRETURN
}

BOOL CRowset::SetSupportedProperty(DBPROPID dwPropertyID, GUID guidPropertySet, void* pValue, DBTYPE wType, DBPROPOPTIONS dwOptions, DBID colid)
{
	if(guidPropertySet == DBPROPSET_ROWSET && (dwPropertyID == DBPROP_IRowsetChange || dwPropertyID == DBPROP_IRowsetUpdate) && (VARIANT_BOOL)pValue==VARIANT_TRUE)
		::SetSupportedProperty(DBPROP_UPDATABILITY,guidPropertySet,&m_cPropSets, &m_rgPropSets, (void*)DBPROPVAL_UP_ALL, DBTYPE_I4, dwOptions, colid);

	return ::SetSupportedProperty(dwPropertyID, guidPropertySet, &m_cPropSets, &m_rgPropSets, pValue, wType, dwOptions, colid);
}
BOOL CRowset::SetSettableProperty(DBPROPID dwPropertyID, GUID guidPropertySet, void* pValue, DBTYPE wType, DBPROPOPTIONS dwOptions, DBID colid)
{
	if(guidPropertySet == DBPROPSET_ROWSET && (dwPropertyID == DBPROP_IRowsetChange || dwPropertyID == DBPROP_IRowsetUpdate) && (VARIANT_BOOL)pValue==VARIANT_TRUE)
		::SetSettableProperty(DBPROP_UPDATABILITY,guidPropertySet,&m_cPropSets, &m_rgPropSets, (void*)DBPROPVAL_UP_ALL, DBTYPE_I4, dwOptions, colid);

	return ::SetSettableProperty(dwPropertyID, guidPropertySet, &m_cPropSets, &m_rgPropSets, pValue, wType, dwOptions, colid);
}
BOOL CRowset::SetProperty(DBPROPID dwPropertyID, GUID guidPropertySet, void* pValue, DBTYPE wType, DBPROPOPTIONS dwOptions, DBID colid)
{
	if(guidPropertySet == DBPROPSET_ROWSET && (dwPropertyID == DBPROP_IRowsetChange || dwPropertyID == DBPROP_IRowsetUpdate) && (VARIANT_BOOL)pValue==VARIANT_TRUE)
		::SetProperty(DBPROP_UPDATABILITY, guidPropertySet, &m_cPropSets,&m_rgPropSets, (void*)DBPROPVAL_UP_ALL, DBTYPE_I4, dwOptions, colid);

	return ::SetProperty(dwPropertyID, guidPropertySet, &m_cPropSets,&m_rgPropSets, pValue, wType, dwOptions, colid);
}
BOOL CRowset::GetProperty(DBPROPID dwPropertyID, GUID guidPropertySet, VARIANT_BOOL bValue)
{
	return ::GetProperty(dwPropertyID,guidPropertySet,pIUnknown(),bValue);
}
BOOL CRowset::GetProperty(DBPROPID dwPropertyID, GUID guidPropertySet, VARIANT_BOOL* pbValue)
{
	return ::GetProperty(dwPropertyID, guidPropertySet, pIUnknown(), pbValue);
}
BOOL CRowset::GetProperty(DBPROPID dwPropertyID, GUID guidPropertySet, ULONG_PTR* pulValue)
{
	return ::GetProperty(dwPropertyID, guidPropertySet, pIUnknown(), pulValue);
}
BOOL CRowset::GetProperty(DBPROPID dwPropertyID, GUID guidPropertySet, WCHAR** ppwszValue)
{
	return ::GetProperty(dwPropertyID, guidPropertySet, pIUnknown(), ppwszValue);
}
BOOL CRowset::FreeProperties()
{
	return ::FreeProperties(&m_cPropSets,&m_rgPropSets);
}
HRESULT CRowset::RestartPosition(HCHAPTER hChapter)
{
	HRESULT hr = S_OK;
	hr = pIRowset()->RestartPosition(hChapter);
	if(hr==S_OK || hr==DB_S_COMMANDREEXECUTED)
		return S_OK;

	return hr;
}

DBCOUNTITEM CRowset::GetTotalRows()
{
	TBEGIN
	DBCOUNTITEM cRows=NO_ROWS;

	DBCOUNTITEM cRowsObtained = 1;
	HROW* rghRows=NULL;
	HRESULT hr = E_FAIL;

	TESTC_(hr = RestartPosition(),S_OK);
	while(cRowsObtained)
	{
		rghRows=NULL;

		//Get the next row
		hr = pIRowset()->GetNextRows(NULL,0,ONE_ROW,&cRowsObtained,&rghRows);
		TEST2C_(hr, S_OK, DB_S_ENDOFROWSET);
			
		//If able to obtain a row, increment row count	
		if(hr==S_OK)  
		{
			cRows++;
			ReleaseRows(rghRows[0]);
			PROVIDER_FREE(rghRows);
		}
		else
		{
			//make sure the rghRows is NULL when cRowsObtained is 0
			TCOMPARE(cRowsObtained, 0);
			TCOMPARE(rghRows, NULL);
		}
	}

	//Restore Position to the begining
	TESTC_(hr = RestartPosition(),S_OK);

CLEANUP:
	PROVIDER_FREE(rghRows);
	return cRows;
}


HRESULT CRowset::GetNextRows(HROW* phRow)
{
	return GetNextRows(0, ONE_ROW, phRow);
}
HRESULT CRowset::GetNextRows(DBROWCOUNT cRows, HROW* rghRow)
{
	return GetNextRows(0, cRows, rghRow);
}
HRESULT CRowset::GetNextRows(DBROWOFFSET lOffset, DBROWCOUNT cRows, HROW* rghRow)
{
	return GetNextRows(lOffset, cRows, NULL, rghRow);
}
HRESULT CRowset::GetNextRows(DBROWOFFSET lOffset, DBROWCOUNT cRows, DBCOUNTITEM* pcRowsObtained, HROW* rghRows)
{
	//Consumer Allocated version
	ASSERT(rghRows);

	//Delegate
	return GetNextRows(lOffset, cRows, pcRowsObtained, &rghRows);
}
HRESULT CRowset::GetNextRows(DBROWOFFSET lOffset, DBROWCOUNT cRows, DBCOUNTITEM* pcRowsObtained, HROW** prghRows)
{
	ASSERT(prghRows);
	TBEGIN
	
	DBCOUNTITEM cRowsObtained = NO_ROWS;
	DBCOUNTITEM i=0;
	
	//Record if we passed in consumer allocated array...
	HROW* rghRowsInput = *prghRows;

	//GetNextRows
	HRESULT hr = pIRowset()->GetNextRows(NULL, lOffset, cRows, &cRowsObtained, prghRows);
	
	//Verify Correct values returned
	if(SUCCEEDED(hr))
	{
		if(hr == S_OK)
		{
			TESTC(cRowsObtained==(DBCOUNTITEM)ABS(cRows));
		}
		else
		{
			TESTC(cRowsObtained < (DBCOUNTITEM)ABS(cRows));
		}

		//Verify row array
		for(i=0; i<cRowsObtained; i++)
		{
			TESTC(*prghRows != NULL);
			TESTC((*prghRows)[i]!=DB_NULL_HROW)
		}
	}
	else
	{
		TESTC(cRowsObtained == 0);
	}

	//Verify output array, depending upon consumer or provider allocated...
	if(rghRowsInput)
	{
		//This is a users allocated static array,
		//This had better not be nulled out by the provider, if non-null on input
		TESTC(*prghRows == rghRowsInput);
	}
	else
	{
		TESTC(cRowsObtained ? *prghRows != NULL : *prghRows == NULL);
	}

CLEANUP:
	if(pcRowsObtained)
		*pcRowsObtained = cRowsObtained;
	return hr;
}


HRESULT CRowset::GetRow(DBCOUNTITEM iRow, HROW* phRow)
{
	return GetRow(iRow,ONE_ROW,phRow);
}
HRESULT CRowset::GetRow(DBCOUNTITEM iRow, DBCOUNTITEM cRows, HROW* rghRow)
{
	return GetRow(iRow,cRows, NULL, rghRow);
}
HRESULT CRowset::GetRow(DBCOUNTITEM iRow, DBCOUNTITEM cRows, DBCOUNTITEM* pcRowsObtained, HROW* rghRow)
{
	TBEGIN
	ASSERT(cRows && rghRow);
	HRESULT hr = E_FAIL;

	//No-op, can't get the 0th row!
	if(iRow == 0)
		return E_FAIL;

	//Obtain cRows starting at iRow
	TESTC_(hr = RestartPosition(),S_OK) 
	hr = GetNextRows(iRow-ONE_ROW, cRows, pcRowsObtained, rghRow);

CLEANUP:
	return hr;
}


HRESULT CRowset::ResynchRows(DBCOUNTITEM cRows,HROW* rghRow,DBCOUNTITEM* pcResynchedRows, HROW** prgResynchedRows, DBROWSTATUS** prgRowStatus)
{
	TBEGIN
	HRESULT hr = E_FAIL;
	IRowsetResynch* pIRowsetResynch = NULL;

	DBCOUNTITEM cResynchedRows = 0;
	HROW* rgResynchedRows = NULL;
	DBROWSTATUS* rgRowStatus = NULL;

	//Resynch all the rows
	TESTC_(hr = QI(pIUnknown(),IID_IRowsetResynch,(void**)&pIRowsetResynch),S_OK)
	hr = pIRowsetResynch->ResynchRows(cRows,rghRow,&cResynchedRows,&rgResynchedRows,&rgRowStatus);
	
	//Verify status Array
	if(cResynchedRows)
	{
		TESTC(rgResynchedRows!=NULL && rgRowStatus!=NULL)

		//Verify RowStatus array
		if(hr==S_OK)
 			VerifyArray(cResynchedRows,rgRowStatus,DBROWSTATUS_S_OK);
	}
	else
		TESTC(rgResynchedRows==NULL && rgRowStatus==NULL)

		
	//Verify rgResynchedRows
	if(cRows && SUCCEEDED(hr))
	{
		TESTC(cResynchedRows==cRows && rgResynchedRows!=NULL && rgRowStatus!=NULL);

		//Verify correct row handles returned
		if(rghRow)
			for(DBCOUNTITEM i=0; i<cResynchedRows; i++)
				TESTC(rghRow[i] == rgResynchedRows[i]) 
	}
	else
	{
		//Need to release the row handles
		//Since Update AddRef's only if you don't already have the row
		ReleaseRows(cResynchedRows,rgResynchedRows);
	}

CLEANUP:  
	SAFE_RELEASE(pIRowsetResynch);
	
	if(pcResynchedRows)
		*pcResynchedRows = cResynchedRows;

	if(prgResynchedRows)
		*prgResynchedRows = rgResynchedRows;
	else
		PROVIDER_FREE(rgResynchedRows);

	if(prgRowStatus)
		*prgRowStatus = rgRowStatus;
	else
		PROVIDER_FREE(rgRowStatus);
	
	return hr;
}



HRESULT CRowset::ReleaseRows(HROW hRow)
{
	return ReleaseRows(ONE_ROW,&hRow);
}

HRESULT CRowset::ReleaseRows(DBCOUNTITEM cRows, HROW* rghRow, DBREFCOUNT** prgRefCounts, DBROWSTATUS** prgRowStatus)
{
	DBREFCOUNT* rgRefCounts = PROVIDER_ALLOC_(cRows, DBREFCOUNT);
	DBROWSTATUS* rgRowStatus = PROVIDER_ALLOC_(cRows, DBROWSTATUS);

	HRESULT hr = ReleaseRows(cRows, rghRow, rgRefCounts, rgRowStatus);

//CLEANUP:
	//RefCounts
	if(prgRefCounts)
		*prgRefCounts = rgRefCounts;
	else
		PROVIDER_FREE(rgRefCounts);  
	//RowStatus
	if(prgRowStatus)
		*prgRowStatus = rgRowStatus;
	else
		PROVIDER_FREE(rgRowStatus);  

	return hr;	
}

HRESULT CRowset::ReleaseRows(DBCOUNTITEM cRows, HROW* rghRow, DBREFCOUNT* rgRefCounts, DBROWSTATUS* rgRowStatus)
{
	IRowset * pIRS = pIRowset();

	TBEGIN 
	//No-op cases
	if(cRows==NO_ROWS || rghRow==NULL || m_pIRowset==NULL)
		return S_OK;

	// Make sure we were able to get the rowset pointer from pIRowset().  It can fail.
	if (!pIRS)
		return E_FAIL;

	//ReleaseRows
	HRESULT hr = pIRS->ReleaseRows(cRows, rghRow, NULL, rgRefCounts, rgRowStatus);
	
	//Verify Status Array
	if(hr == S_OK && rgRowStatus)
	{
		//Since S_OK for ReleaseRows can return either DBROWSTATUS_S_OK or 
		//DBROWSTATUS_S_PENDINGCHANGES, we have to check for both
		//Instead of just calling VerifyArray()
		for(DBCOUNTITEM i=0; i<cRows; i++)
			TESTC(rgRowStatus[i]==DBROWSTATUS_S_OK || rgRowStatus[i]==DBROWSTATUS_S_PENDINGCHANGES);
	}
	
CLEANUP:
	return hr;	
}

HRESULT CRowset::ReleaseAccessor(HACCESSOR hAccessor, DBCOUNTITEM cBindings, DBBINDING* rgBindings, void* pData)
{
	TBEGIN 
	HRESULT hr = E_FAIL;

	//Free the pData
	if(pData)
		QTESTC_(ReleaseInputBindingsMemory(cBindings, rgBindings, (BYTE*)pData, TRUE),S_OK)

	//Now Release the Bindings
	if(cBindings)
		QTESTC_(hr = FreeAccessorBindings(cBindings, rgBindings),S_OK)
	
	//ReleaseAccessor
	if(hAccessor && (m_pIRowset || m_pIAccessor))
		QTESTC_(hr = pIAccessor()->ReleaseAccessor(hAccessor,NULL), S_OK)

CLEANUP:
	return hr;	
}

BOOL CRowset::ValidRow(HROW hRow)
{
	return ValidRow(ONE_ROW, &hRow);
}
BOOL CRowset::ValidRow(DBCOUNTITEM cRows, HROW* rghRow)
{
	TBEGIN
	ASSERT(cRows && rghRow);

	void* pData = PROVIDER_ALLOC(SIZEOF_ONEROW);

	for(DBCOUNTITEM i=0; i<cRows; i++)
		QTESTC_(GetData(rghRow[i],m_hAccessor,pData),S_OK)

CLEANUP:
	PROVIDER_FREE(pData);
	TRETURN
}


BOOL CRowset::IsSameRow(HROW hRow1, HROW hRow2)
{
	TBEGIN
	BOOL bEqual = FALSE;
	IRowsetIdentity* pIRowsetIdentity = NULL;
	HRESULT hr = S_OK;

	//Can we compare these row handles literally?
	if(GetProperty(DBPROP_LITERALIDENTITY, DBPROPSET_ROWSET))
	{
		//Equal?
		bEqual = (hRow1 == hRow2);
	}
	else
	{
		//We could just call IsSameRow always since its a required level-0 interface
		//But its only required if you have set the property (DBPROP_IRowsetIdentity)
		//ahead of time before rowset creation, and we want to get additional
		//testing of the DBPROP_LITERALIDENTITY property anway...
		QTESTC_(QI(pIRowset(), IID_IRowsetIdentity, (void**)&pIRowsetIdentity),S_OK);
		TEST2C_(hr = pIRowsetIdentity->IsSameRow(hRow1, hRow2),S_OK,S_FALSE);

		//Equal?
		bEqual = (hr == S_OK); //S_OK = Equal, S_FALSE = NotEqual;
	}

CLEANUP:
	SAFE_RELEASE(pIRowsetIdentity);
	return bEqual;
}

HRESULT CRowset::GetRowData(HROW hRow, void** ppData)
{
	return GetRowData(ONE_ROW,&hRow,ppData);
}

HRESULT CRowset::GetRowData(DBCOUNTITEM cRows, HROW* rghRow, void** rgpData)
{
	TBEGIN
	ASSERT(cRows && rghRow && rgpData);
	HRESULT hr = E_FAIL;

	for(DBCOUNTITEM i=0; i<cRows; i++)
	{
		if(rgpData[i]==NULL)  
			rgpData[i] = PROVIDER_ALLOC(SIZEOF_ONEROW);
		TESTC(rgpData[i]!=NULL)

		//Get the Data for row hRow
		QTESTC_(hr = GetData(rghRow[i], m_hAccessor, rgpData[i]),S_OK);
	}

CLEANUP:
	return hr;
}

HRESULT CRowset::GetData(HROW hRow, HACCESSOR hAccessor, void* pData)
{
	TBEGIN
	DBACCESSORFLAGS dwAccessorFlags;
	DBCOUNTITEM cBindings = 0;
	DBBINDING* rgBindings = NULL;
	CRowObject RowObject;
	HRESULT hr = S_OK;

	//Does the provider support Row Objects?
	hr = RowObject.CreateRowObject(pIRowset(), hRow);

	//Verify Results...
	if(SUCCEEDED(hr) && hAccessor && hRow && pData)
	{
		//Obtain the accessor bindings
		QTESTC_(hr = pIAccessor()->GetBindings(hAccessor, &dwAccessorFlags, &cBindings, &rgBindings),S_OK)

		//Get the Data for row object
		hr = RowObject.GetColumns(cBindings, rgBindings, pData);
	}
	else
	{
		//Get the Data for row hRow
		hr = pIRowset()->GetData(hRow, hAccessor, pData);
 	}
	
	//Display any binding errors and status'
	TESTC(VerifyBindings(hr, pIAccessor(), hAccessor, pData));

CLEANUP:
	FreeAccessorBindings(cBindings, rgBindings);
	return hr;
}


BOOL CRowset::CompareRowData(void* pOrgData, void* pData, HACCESSOR hAccessor, BOOL fSetData)
{
	ASSERT(pOrgData && pData);
	TBEGIN

	if(!hAccessor)
		hAccessor = m_hAccessor;
	ASSERT(hAccessor);

	ULONG dwAccessorFlags;
	DBCOUNTITEM cBindings = 0;
	DBBINDING* rgBindings = NULL;
	
	//Obtain the bindings
	TESTC_(pIAccessor()->GetBindings(hAccessor, &dwAccessorFlags, &cBindings, &rgBindings),S_OK)

	//Compare pOrgData to pData
	QTESTC(CompareBuffer(pOrgData, pData, cBindings, rgBindings, NULL, fSetData, FALSE, COMPARE_ONLY))

CLEANUP:
	FreeAccessorBindings(cBindings, rgBindings);
	TRETURN
}


HRESULT CRowset::GetStorageData(HROW hRow, HACCESSOR hAccessor, void* pBuffer, ULONG* pcBytes, REFIID riid, IUnknown** ppIUnknown)
{
	TBEGIN
	ASSERT(hRow && hAccessor);
	HRESULT hr = S_OK;
	IUnknown* pIUnknown = NULL;
	void* pData = NULL;

	DBACCESSORFLAGS dwAccessorFlags;
	DBCOUNTITEM cBindings = 0;
	DBBINDING* rgBindings = NULL;
	CRowObject RowObject;
	WCHAR* pwszStringBuffer = NULL;

	//Obtain the accessor bindings
	QTESTC_(hr = pIAccessor()->GetBindings(hAccessor, &dwAccessorFlags, &cBindings, &rgBindings),S_OK)

	//Does the provider support Row Objects?
	hr = RowObject.CreateRowObject(pIRowset(), hRow);

	//Verify Results...
	if(SUCCEEDED(hr))
	{
		DBCOLUMNINFO dbColInfo;

		//Find the IUNKNOWN binding
		DBBINDING* pBinding = FindBinding(cBindings, rgBindings, DBTYPE_IUNKNOWN);
		TESTC(pBinding != NULL);
		
		//Obtain the ColumnID (sure which this was ordinal based)
		TESTC(::FindColInfo(RowObject.pIRow(), NULL, pBinding->iOrdinal, &dbColInfo, &pwszStringBuffer));

		//Obtain the storage object through IRow::Open
		//NOTE: DB_E_NOTFOUND is basically for ISNULL data.  We map this to S_OK so its
		//similar to the GetData case so our callers don't have to special case for the same senario
		hr = RowObject.Open(NULL, &dbColInfo.columnid, DBGUID_STREAM, riid, &pIUnknown);
		QTESTC(hr==S_OK || hr==DB_E_NOTFOUND);
		hr = S_OK;

		//Obtain the stoage data if requested
		if(pBuffer && pcBytes)
			QTESTC_(hr = ::GetStorageData(riid, pIUnknown, pBuffer, pcBytes),S_OK);
	}
	else
	{
		//Alloc pData, large enough
		SAFE_ALLOC(pData, void*, DATA_SIZE);

		//GetData to obtain the storage pointer
		QTESTC_(hr = GetData(hRow, hAccessor, pData),S_OK)

		//Get Storage Data
		QTESTC_(hr = ::GetStorageData(cBindings, rgBindings, pData, pBuffer, pcBytes, riid, &pIUnknown),S_OK);
 	}
	
CLEANUP:
	if(ppIUnknown)
		*ppIUnknown = pIUnknown;
	else
		SAFE_RELEASE(pIUnknown);

	// Release buffer
	FreeAccessorBufferAndBindings(&cBindings, &rgBindings, &pData, true);
	SAFE_FREE(pwszStringBuffer);
	return hr;
}


HRESULT CRowset::GetBookmark(HROW hRow, DBBKMARK* pcbBookmark, BYTE **ppBookmark)
{
	TBEGIN
	ASSERT(hRow && pcbBookmark && ppBookmark);
	HRESULT hr = E_FAIL;

	*pcbBookmark = 0;
	*ppBookmark = NULL;
	void* pData =  NULL;

	DBLENGTH cRowSize = 0;
	DBCOUNTITEM cBindings = 0;
	DBBINDING* rgBindings = NULL;
	
	HACCESSOR hAccessor = NULL;

	//Create the Accessor to bind to the Bookmark column
	TESTC_(GetAccessorAndBindings(pIUnknown(), DBACCESSOR_ROWDATA, &hAccessor,
		&rgBindings, &cBindings, &cRowSize, DBPART_ALL, ALL_COLS_BOUND),S_OK);
		
	//Alloc memory for the GetData
	pData = PROVIDER_ALLOC(sizeof(void*)*cRowSize);

	//Get the data for the row
	TESTC_(hr = GetData(hRow, hAccessor, pData),S_OK)
	
	//make sure the 0 column is for bookmark
	TESTC(rgBindings[0].iOrdinal == 0)
	
	//get the length of the bookmark
	if(LENGTH_IS_BOUND(rgBindings[0]))
		*pcbBookmark = LENGTH_BINDING(rgBindings[0],pData);

	//allocate memory for bookmark
	*ppBookmark= (BYTE*)PROVIDER_ALLOC(*pcbBookmark);
	
	//copy the value of the bookmark into the consumer's buffer
	if(VALUE_IS_BOUND(rgBindings[0]))
		memcpy(*ppBookmark, &VALUE_BINDING(rgBindings[0],pData), (size_t)*pcbBookmark);

CLEANUP:
	ReleaseAccessor(hAccessor, cBindings, rgBindings, pData);
	return hr;
}


BOOL CRowset::CompareRowData(HROW hRow, void* pData)
{
	return CompareRowData(ONE_ROW,&hRow,&pData);
}
BOOL CRowset::CompareRowData(DBCOUNTITEM cRows, HROW* rghRow, void** rgpData)
{
	ASSERT(cRows && rghRow && rgpData);

	TBEGIN
	DBCOUNTITEM i;
    void* pRowData = NULL;
		
	for(i=0; i<cRows; i++)
	{
		//Get Row[i] Data
		TESTC_(GetRowData(rghRow[i],&pRowData),S_OK)
		//Compare Row[i] to Input[i]
		QTESTC(CompareRowData(pRowData, rgpData[i])) 
	}

CLEANUP:
	//free RowData buffer
	PROVIDER_FREE(pRowData);
	TRETURN
}


//////////////////////////////////////////////////////////////////////////////
// CRowset::VerifyRowHandles
//
//////////////////////////////////////////////////////////////////////////////
BOOL CRowset::VerifyRowHandles(HROW hRow, DBCOUNTITEM iRow)
{
	TBEGIN
	CRowObject RowObjectA;	
	void* pData = NULL;
	
	//Does the provider support Row Objects?
	HRESULT hr = RowObjectA.CreateRowObject(pIRowset(), hRow);
	TEST3C_(hr, S_OK, DB_S_NOROWSPECIFICCOLUMNS, E_NOINTERFACE); 

	//Allocate the Data
	SAFE_ALLOC(pData, void*, SIZEOF_ONEROW);

	if(SUCCEEDED(hr))
	{
		//Fetch the data assicoated with the row object
		TESTC_(RowObjectA.GetColumns(m_cBindings, m_rgBinding, pData),S_OK);
	}
	else
	{
		//Fetch the data assicoated with the row
		QTESTC_(hr = GetData(hRow, m_hAccessor, pData),S_OK);
	}
	
	//Need to use misfunc CompareData to compare rowset data, with BackEnd
	if(!CompareData(m_cRowsetCols, m_rgTableColOrds, iRow, pData, m_cBindings, m_rgBinding, m_pTable, NULL, PRIMARY, COMPARE_ONLY))
	{
		//Data incorrect for this row!
		TERROR("Data was incorrect for row " << iRow);
		QTESTC(FALSE);
	}
	
	ReleaseRowData(pData);
	
CLEANUP:
	TRETURN
}


//////////////////////////////////////////////////////////////////////////////
// CRowset::VerifyRowHandles
//
//////////////////////////////////////////////////////////////////////////////
BOOL CRowset::VerifyRowHandles						
	(	
		DBCOUNTITEM   cRows,
		HROW*	rghRows,				//row handles
		DBCOUNTITEM	iRowStart,				//Starting row Position
		ECOLUMNORDER eOrder			 	//Direction
	)	
{
	TBEGIN
	HRESULT hr = S_OK;
	BOOL bReturn = TRUE;

	//Loop through all the rows, note: we want to know the results for all
	//rows, so indicate errors, but continue processing, so its not an iterative
	//processing for fixing bugs and finding problems...
	for(DBCOUNTITEM i=0; i<cRows; i++)
	{
		DBCOUNTITEM iRow = eOrder == FORWARD ? iRowStart+i : iRowStart-i;

		if(!rghRows[i])
		{
			//Data incorrect for this row!
			TERROR("NULL hRow for row " << iRow);
			bReturn = FALSE;
		}
		else
		{
			//Verify the data for this row...
			if(!VerifyRowHandles(rghRows[i], iRow))
				bReturn = FALSE;
		}
	}

	return bReturn;
}


BOOL CRowset::CompareTableData(DBCOUNTITEM iTableRow, void* pData, DBCOUNTITEM cBindings, DBBINDING* rgBindings)
{
	ASSERT(iTableRow && pData);
	ASSERT(cBindings && rgBindings);
	TBEGIN

	//Need to use misfunc CompareData to compare rowset data, with BackEnd
	QTESTC(CompareData(m_cRowsetCols,m_rgTableColOrds,iTableRow,pData,cBindings,rgBindings,pTable(),NULL,PRIMARY,COMPARE_ONLY))

CLEANUP:
	TRETURN
}


BOOL CRowset::MakeRowData(void** ppData, HACCESSOR hAccessor, DBCOUNTITEM* piRow)
{
	ASSERT(ppData);
	ASSERT(m_cRowsetCols && m_rgTableColOrds);
	TBEGIN

	//Set to the next row
	//Note:  We make a copy here for the rest of the function, in MultiThreaded
	//senarios by the time execution gets to FillInputBindings the global index
	//may have changed, thus inserting two rows with the same index.
	DBCOUNTITEM ulNextRow = ++g_ulNextRow;

	if(!hAccessor)
		hAccessor = m_hAccessor;
	ASSERT(hAccessor);

	ULONG dwAccessorFlags;
	DBCOUNTITEM cBindings = 0;
	DBBINDING* rgBindings = NULL;
	
	//Let the user know with which data row we modified with
	if(piRow)
		*piRow = ulNextRow;

	//Obtain the bindings
	TESTC_(pIAccessor()->GetBindings(hAccessor, &dwAccessorFlags, &cBindings, &rgBindings),S_OK)

	//Fill Bindings, similar to the first row
	TESTC_(FillInputBindings(pTable(), dwAccessorFlags, cBindings, rgBindings,
		(BYTE**)ppData, ulNextRow, m_cRowsetCols, m_rgTableColOrds, PRIMARY),S_OK)
		
	//Verify we got something back
	TESTC(ppData && *ppData!=NULL)

CLEANUP:	
	FreeAccessorBindings(cBindings, rgBindings);
	TRETURN
}


BOOL CRowset::ReleaseRowData(void* pData, HACCESSOR hAccessor, BOOL fFreeBuffer)
{
	TBEGIN
	ULONG dwAccessorFlags;
	DBCOUNTITEM cBindings = 0;
	DBBINDING* rgBindings = NULL;
	
	//No-op case
	if(!pData)
		return TRUE;

	//Otherwise we better have an Accessor
	if(!hAccessor)
		hAccessor = m_hAccessor;
	QTESTC(hAccessor != NULL);

	//Obtain the bindings
	TESTC_(pIAccessor()->GetBindings(hAccessor, &dwAccessorFlags, &cBindings, &rgBindings),S_OK)

	//Release outofline memory, created from FillInputBindings
	TESTC_(ReleaseInputBindingsMemory(cBindings,rgBindings,(BYTE*)pData, fFreeBuffer),S_OK)

CLEANUP:
	FreeAccessorBindings(cBindings, rgBindings);
	TRETURN
}


HRESULT CRowset::CreateAccessor(HACCESSOR* phAccessor, DBACCESSORFLAGS dwAccessorFlags, DBPART dwPart, DBCOUNTITEM* pcBindings, DBBINDING** prgBindings, DBLENGTH* pcRowSize, BLOBTYPE dwBlobType, DBBINDSTATUS** prgBindStatus, DWORD dwColsBound, ECOLS_BY_REF eColsByRef, DBTYPE dwModifier)
{
	ASSERT(phAccessor); //everything else is optional

 	//Create Accessor 
	HRESULT hr = GetAccessorAndBindings(pIUnknown(), dwAccessorFlags, phAccessor,
		prgBindings,pcBindings,pcRowSize,dwPart, dwColsBound, FORWARD,
		eColsByRef, NULL,NULL,NULL, dwModifier, 0,NULL,NULL,
		NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, dwBlobType, prgBindStatus);

	return hr;
}


HRESULT CRowset::CreateRowsetFromTbl(DBCOUNTITEM cRows)
{
	//We don't allow successive x calls
    ASSERT(!m_pIAccessor);	//This used to be ASSERT(cRows && !m_pIAccessor).
							//cRows was removed in order to create zero length Rowsets.
	HRESULT hr = E_FAIL;
	TBEGIN

	//Querying for X number of rows, would require SQL statments diff from
	//the privlib, so an easy way to do this is to just create a table of X
	//rows, set the RowsetObject to use that table, and have CreateRowset take over...
	
	//Create a new table
	CTable* pNewTable = NULL;

	//Create a table with cRows	
	TESTC(CreateTable(&pNewTable,cRows))

	//Set the RowsetObject to use that table, m_pTable = pNewTable
	//Delete when done, because this it not a global table, 
	//and will not be needed by other testcases...
	SetTable(pNewTable, DELETETABLE_YES); 

	//Now create the rowset, using this table
	hr = CreateRowset(USE_SUPPORTED_SELECT_ALLFROMTBL);

CLEANUP:
	return hr;
}

HRESULT CRowset::CreateRowset
(
	DBPROPID			dwPropertyID,
	EQUERY				eQuery,					//the type of rowset to create
	REFIID				riid,					//riid to ask for
	CTable*				pCTable,
	DBACCESSORFLAGS		dwAccessorFlags,		//the accessor flags
	DBPART				dwPart,					//the type of binding
	DWORD				dwColsToBind,			//the columns in accessor
	ECOLUMNORDER		eBindingOrder,			//the order to bind columns
	ECOLS_BY_REF		eColsByRef,				//which columns to bind by reference
	DBTYPE				wTypeModifier,			//the type modifier used for accessor
	BLOBTYPE			dwBlobType				//BLOB option
)
{
	TBEGIN
	HRESULT hr = E_FAIL;

	//We don't allow successful calls to CreateRowset
	ASSERT(!m_pIAccessor);

	//Set the desired property
	TESTC(SetProperty(dwPropertyID, DBPROPSET_ROWSET))

	//Delegate out to CreateRowset
	hr = CRowset::CreateRowset(eQuery, riid, pCTable, dwAccessorFlags, dwPart, dwColsToBind, eBindingOrder, eColsByRef, wTypeModifier, dwBlobType);

CLEANUP:
	return hr;
}

HRESULT CRowset::CreateRowset
(	
	EQUERY				eQuery,					//the type of rowset to create
	REFIID				riid,					//riid to ask for
	CTable*				pCTable,
	DBACCESSORFLAGS		dwAccessorFlags,		//the accessor flags
	DBPART				dwPart,					//the type of binding
	DWORD				dwColsToBind,			//the columns in accessor
	ECOLUMNORDER		eBindingOrder,			//the order to bind columns
	ECOLS_BY_REF		eColsByRef,				//which columns to bind by reference
	DBTYPE				wTypeModifier,			//the type modifier used for accessor
	BLOBTYPE			dwBlobType				//BLOB option
)
{
	//We don't allow successful calls to CreateRowset
	ASSERT(!m_pIAccessor);

	TBEGIN
	HRESULT hr = E_FAIL;

	//We do have a problem with a provider that doesn't support commands
	//Most of the variations in this test require "Order By" so we can be
	//guareenteed the cursor and retreive are returning the correct row

	//For providers that don't support Commands, OpenRowset is not guareenteed
	//to return "Order by" anything.  So instead of just failing those variations,
	//I have choosen to run them "hoping" the provider that doesn't support commands
	//returns the data in the order of either the "ini" file or as MakeData is expecting...
	if(m_pIDBCreateCommand==NULL && 
		(eQuery == SELECT_ALLFROMTBL || eQuery == SELECT_COLLISTFROMTBL || eQuery == SELECT_ORDERBYNUMERIC))
		eQuery = USE_SUPPORTED_SELECT_ALLFROMTBL;

	//Conformance limiting for STRICT
	if(GetModInfo()->IsStrictLeveling() && !IsReqInterface(ROWSET_INTERFACE, riid))
		return E_NOINTERFACE;

	//Use the global CTable created in Module init, by default
	SetTable(pCTable ? pCTable : g_pTable, DELETETABLE_NO);

	//Delegate out to privlib
	//Only create the rowset if there was no errors in setting properties
	QTESTC_(hr = CreateRowsetObject(eQuery, riid, EXECUTE_IFNOERROR, &m_pIUnknown),S_OK);

	//Obtain the Rowset pointer, from the accessor
	QTESTC_(hr = QI(m_pIUnknown, IID_IRowset, (void**)&m_pIRowset),S_OK);
		
	//Create Accessor on the Rowset
	hr = GetAccessorAndBindings(m_pIUnknown, dwAccessorFlags, &m_hAccessor,
		&m_rgBinding, &m_cBindings, &m_cRowSize, dwPart, dwColsToBind, 
		eBindingOrder, 	eColsByRef, NULL, NULL, NULL, wTypeModifier, 0, NULL, NULL, 
		NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, dwBlobType);
	QTESTC(hr==S_OK || hr==DB_E_NULLACCESSORNOTSUPPORTED)

	if(m_cBindings == 0)
	{
		ReleaseAccessor(m_hAccessor);
		QTESTC_(hr = GetAccessorAndBindings(m_pIUnknown, dwAccessorFlags, &m_hAccessor,
		&m_rgBinding, &m_cBindings, &m_cRowSize, dwPart, ALL_COLS_EXCEPTBOOKMARK, 
		eBindingOrder, 	eColsByRef, NULL, NULL, NULL, wTypeModifier, 0, NULL, NULL, 
		NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, dwBlobType),S_OK);
	}

	//Total TableRows
	m_ulTableRows = pTable()->GetRowsOnCTable();

	//Alloc scratch buffer
	m_pData = PROVIDER_ALLOC(SIZEOF_ONEROW);
	if(m_cRowSize) 
		TESTC(m_pData!=NULL)

	//MaxOpenRows
	m_ulMaxOpenRows = 0;
	GetProperty(DBPROP_MAXOPENROWS, DBPROPSET_ROWSET, &m_ulMaxOpenRows);

	//CanHoldRows
	m_bCanHoldRows = GetProperty(DBPROP_CANHOLDROWS, DBPROPSET_ROWSET, VARIANT_TRUE);
	
	//CanScrollBackwards
	m_bCanScrollBackwards = GetProperty(DBPROP_CANSCROLLBACKWARDS, DBPROPSET_ROWSET, VARIANT_TRUE);

	//CanFetchBackwards
	m_bCanFetchBackwards = GetProperty(DBPROP_CANFETCHBACKWARDS, DBPROPSET_ROWSET, VARIANT_TRUE);

	//DBPROP_OLEOBJECTS (DataSourceInfo)
	::GetProperty(DBPROP_OLEOBJECTS, DBPROPSET_DATASOURCEINFO, g_pIDBInitialize, &m_ulpOleObjects);

CLEANUP:
	return hr;
}

HRESULT CRowset::CreateRowset
(
	IUnknown*			pIRowset,
	EQUERY				eQuery,					//the type of rowset to create
	REFIID				riid,					//riid to ask for
	CTable*				pCTable,
	DBACCESSORFLAGS		dwAccessorFlags,		//the accessor flags
	DBPART				dwPart,					//the type of binding
	DWORD				dwColsToBind,			//the columns in accessor
	ECOLUMNORDER		eBindingOrder,			//the order to bind columns
	ECOLS_BY_REF		eColsByRef,				//which columns to bind by reference
	DBTYPE				wTypeModifier,			//the type modifier used for accessor
	BLOBTYPE			dwBlobType				//BLOB option
)
{
	TBEGIN
	HRESULT hr = E_FAIL;
	IAccessor* pIAccessor = NULL;

	//InvalidArg case
	if(!pIRowset)
		return E_INVALIDARG;

	//Obtain IRowset pointer, and keep a copy of it
	SAFE_RELEASE(m_pIUnknown);
	SAFE_RELEASE(m_pIRowset);
	TESTC_(hr = QI(pIRowset, IID_IUnknown, (void**)&m_pIUnknown),S_OK);
	TESTC_(hr = QI(pIRowset, IID_IRowset, (void**)&m_pIRowset),S_OK);
    
	//Setup the RowsetObject from an existing rowset
	//Need to obtain the IAccessor interface
	TESTC_(hr = QI(m_pIUnknown, IID_IAccessor, (void**)&pIAccessor),S_OK);
	
	//SetRowsetObject, it creates a new Sesison so release this one...
	TESTC(SetRowsetObject(pIAccessor));

	//Obtain the hAccessor bindings
	TEST2C_(hr = GetAccessorAndBindings(m_pIUnknown, dwAccessorFlags, &m_hAccessor,
		&m_rgBinding, &m_cBindings, &m_cRowSize, dwPart, dwColsToBind, 
		eBindingOrder, 	eColsByRef, NULL, NULL, NULL, wTypeModifier, 0, NULL, NULL, 
		NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, dwBlobType), S_OK, DB_E_NULLACCESSORNOTSUPPORTED);

	if(m_cBindings == 0)
	{
		ReleaseAccessor(m_hAccessor);
		QTESTC_(hr = GetAccessorAndBindings(m_pIUnknown, dwAccessorFlags, &m_hAccessor,
			&m_rgBinding, &m_cBindings, &m_cRowSize, dwPart, ALL_COLS_BOUND, 
			eBindingOrder, 	eColsByRef, NULL, NULL, NULL, wTypeModifier, 0, NULL, NULL, 
			NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, dwBlobType),S_OK);
	}

	//Need to setup m_cRowsetCols and m_rgTableColOrds
	TESTC(pTable()->GetQueryInfo(SELECT_ALLFROMTBL, &m_cRowsetCols, &m_rgTableColOrds, NULL, NULL, NULL, NULL));

	//Alloc scratch buffer
	m_pData = PROVIDER_ALLOC(SIZEOF_ONEROW);
	if(m_cRowSize) 
		TESTC(m_pData!=NULL)

	//ulMaxOpenRows
	m_ulMaxOpenRows = 0;
	GetProperty(DBPROP_MAXOPENROWS, DBPROPSET_ROWSET, &m_ulMaxOpenRows);

CLEANUP:	 
	SAFE_RELEASE(pIAccessor);
    return hr;
}


////////////////////////////////////////////////////////////////////////////
//  CRowset::GetRowObject
//
////////////////////////////////////////////////////////////////////////////
HRESULT CRowset::GetRowObject(DBCOUNTITEM iRow, CRowObject* pCRowObject, ULONG_PTR ulpOleObjects, HROW hRowSpecified)
{
	TBEGIN
	ASSERT(pCRowObject);
	HRESULT hr = S_OK;
	IUnknown* pIUnknown = NULL;

	//Delegate
	hr = GetRowObject(NULL, iRow, IID_IRow, &pIUnknown, ulpOleObjects, hRowSpecified);
	if(SUCCEEDED(hr))
	{
		//We just mask of the warning, otherwise the caller has to expect both depending upon
		//whichever way we end up creating the row object.  This is the "common" case function
		//where thats not important.  If your really testing for the warning, then just call the
		//other delegate function (GetRowObject) above directly...
		if(hr == DB_S_NOROWSPECIFICCOLUMNS || hr == DB_S_NOTSINGLETON)
			hr = S_OK;
		
		//Now create the row object
		if(hr == S_OK)
		{
			TESTC_(hr = pCRowObject->SetRowObject(pIUnknown),S_OK); 
		}
	}

CLEANUP:
	SAFE_RELEASE(pIUnknown);
	return hr;
}

////////////////////////////////////////////////////////////////////////////
//  CRowset::GetRowObject
//
////////////////////////////////////////////////////////////////////////////
HRESULT CRowset::GetRowObject(IUnknown* pIUnkOuter, DBCOUNTITEM iRow, REFIID riid, IUnknown** ppIUnknown, ULONG_PTR ulpOleObjects, HROW hRowSpecified)
{
	TBEGIN
	ASSERT(ppIUnknown);
	HROW		hRow		= NULL;
	HRESULT		hr			= S_OK;
	ULONG		cPropSets	= 0;
	DBPROPSET	*rgPropSets = NULL;

	//Output Params
	*ppIUnknown = NULL;

	//The specified row has to be valid...
	if(!iRow)
		return E_FAIL;

	//Default is to use the property of this rowset...
	if(!ulpOleObjects)
		ulpOleObjects = m_ulpOleObjects;

	//Otherwise
	//Note: we do this first, since singltons are expensive
	if(ulpOleObjects & DBPROPVAL_OO_ROWOBJECT)
	{
		//Obtain the specified row from the rowset...
		if(!hRowSpecified)
		{
			//NOTE: If the user passes in a row handle (hRowSpecifed) then we don't have to
			//obtain the row, which basiclaly means RestartPosition and a offset fetch.  Allowing
			//the user to specify (if they want) it allows traversals of forward only rowsets...
			TESTC_(hr = GetRow(iRow, ONE_ROW, &hRow),S_OK);
		}
		
		//Obtain the row object from the row handle...
		TEST3C_(hr = GetRowFromHROW(pIUnkOuter, hRowSpecified ? hRowSpecified : hRow, riid, ppIUnknown), S_OK, DB_S_NOROWSPECIFICCOLUMNS, E_NOINTERFACE);
	}
	//If the provider supports singleton selects
 	else if(ulpOleObjects & DBPROPVAL_OO_SINGLETON)
	{
		//If the user is not requesting one of the interfaces that automatically is spec'd
		//to return a row object, we need to set the DBPROP_IRow property to indicate a 
		//row object instead of a rowset, - (ie: aggregation case asking for IID_IUnknown...)
		if(!(riid == IID_IRow || riid == IID_IRowChange || riid == IID_IRowSchemaChange))
		{
			::SetProperty(DBPROP_IRow, DBPROPSET_ROWSET, &cPropSets, &rgPropSets);
		}

		//AccessOrder.
		//The Row object also looks at the accessorder property.
		DBPROP dbProp;
		if(::GetProperty(DBPROP_ACCESSORDER, DBPROPSET_ROWSET, pIUnknown(), &dbProp))
		{
			//The default of the provider may be random/optional, in which case we don't always want to 
			//set AccessOrder=Random for the row.  So if the property was set as required (meaning more
			//than likely set by the consumer), then ask for it on the row.
			if((dbProp.dwOptions == DBPROPOPTIONS_REQUIRED) && V_VT(&dbProp.vValue) == VT_I4 && V_I4(&dbProp.vValue)==DBPROPVAL_AO_RANDOM)
				::SetProperty(DBPROP_ACCESSORDER, DBPROPSET_ROWSET, &cPropSets, &rgPropSets, (void*)&dbProp.vValue, VT_VARIANT);
		}

		//Create from singleton select (to find this particular row...)
		//NOTE: We allow DB_E_NOTSUPPORTED since if were running from an INI file and they don't
		//have support the particular query thats was CreateSQLStmt returns.  But we do return it
		//to the caller so they can validate if needed.  DB_E_NOAGGREGATION can be returned in the
		//case of running with remoting, so let the caller verify...
		TEST5C_(hr = pTable()->Select(pIUnkOuter, iRow, riid, cPropSets, rgPropSets, NULL, ppIUnknown), S_OK, DB_S_NOTSINGLETON, E_NOINTERFACE, DB_E_NOTSUPPORTED, DB_E_NOAGGREGATION);
	}
	else
	{
		//We don't have a way to handle whatever flags were passed in...
		hr = E_INVALIDARG;
	}
	
CLEANUP:
	ReleaseRows(hRow);
	::FreeProperties(&cPropSets, &rgPropSets);
	return hr;
}


////////////////////////////////////////////////////////////////////////////
//  CRowset::GetRowFromHROW
//
////////////////////////////////////////////////////////////////////////////
HRESULT CRowset::GetRowFromHROW(IUnknown* pIUnkOuter, HROW hRow, REFIID riid, IUnknown** ppIUnknown)
{
	TBEGIN
	HRESULT		hr			= S_OK;
	IUnknown*	pIUnknown	= NULL;
	IGetRow*	pIGetRow	= NULL;

	//Obtain IGetRow - Optional interface
	QTESTC_(hr = QI(pIRowset(), IID_IGetRow, (void**)&pIGetRow),S_OK);
	
	//IGetRow::GetRowFromHROW
	hr = pIGetRow->GetRowFromHROW(pIUnkOuter, hRow, riid, &pIUnknown);
	TEST4C_(hr, S_OK, DB_S_NOROWSPECIFICCOLUMNS, E_NOINTERFACE, DB_E_NOAGGREGATION); 

	//Verify results
	if(SUCCEEDED(hr))
	{
		if(hr == DB_S_NOROWSPECIFICCOLUMNS)
		{
			//Change to S_OK so all calling functions don't have to explcitly check
			//this allowed warning code...
			hr = S_OK;

			//TODO: 
			//Make sure there are no extra columns, since the provider can determine this
			//quickly and easily.  Note: We can't verify the opposite since this warning is not 
			//"required" if its expensive to determine or not possible...
			
			//To quickly determine this just dump this into our Row Object class
			//and veriy the same number of columns as the rowset object...
			DBORDINAL cExColumns = 0;
			CRowObject RowObjectA;
			RowObjectA.SetRowObject(pIUnknown);
			
			//Make sure their are truely no extra columns...
			TESTC_(RowObjectA.GetExtraColumnInfo(&cExColumns, NULL, NULL),S_OK);
			TESTC(cExColumns == 0);
		}

		TESTC(pIUnknown != NULL);
		TESTC(DefaultObjectTesting(pIUnknown, ROW_INTERFACE));
	}
	else
	{
		TESTC(pIUnknown == NULL);
	}

CLEANUP:
	if(ppIUnknown)
		*ppIUnknown = pIUnknown;
	else
		SAFE_RELEASE(pIUnknown);
	SAFE_RELEASE(pIGetRow);
	return hr;
}


////////////////////////////////////////////////////////////////////////////
//  CRowset::GetURLFromHROW
//
////////////////////////////////////////////////////////////////////////////
HRESULT CRowset::GetURLFromHROW(HROW hRow, WCHAR** ppwszURL)
{
	TBEGIN
	HRESULT hr = S_OK;
	WCHAR* pwszURL = NULL;
	IRow*		pIRow		= NULL;
	IGetRow*	pIGetRow	= NULL;

	//Obtain IGetRow - Optional interface
	QTESTC_(hr = QI(pIRowset(), IID_IGetRow, (void**)&pIGetRow),S_OK);

	//IGetRow::GetURLFromHROW
	hr = pIGetRow->GetURLFromHROW(hRow, &pwszURL);

	//Verify results
	if(SUCCEEDED(hr))
	{
		TESTC(pwszURL != NULL);

		//Obtain the Root Binder
		IBindResource* pIBindResource = GetModInfo()->GetRootBinder();
		if(pIBindResource)
		{
			//Make sure this is a valid URL...
			TESTC_(pIBindResource->Bind
				(
					NULL,				//pUnkOuter
					pwszURL,			//pwszURL
					DBBINDURLFLAG_READ,	//dwBindURLFlags
					DBGUID_ROW,			//rguid
					IID_IRow,			//riid
					NULL,				//pAuthenticate
					NULL,				//pImplSession
					NULL,				//dwBindStatus
					(IUnknown**)&pIRow	//ppUnk
				),S_OK);

			//Verify the row object returned...
			TESTC(DefaultObjectTesting(pIRow, ROW_INTERFACE));

			//NOTE: This is just a sanity check that the URL is correct.  We have agreed
			//that the BindResource test will use GetURLFromHROW and do the exhaustive testing
			//since it has the frameowrk setup for the URL mappings and data comparisons...
		}
		else
		{
			//Root Binder not available, unable to verify URL
			//NOTE: This is mainly here for the case when running under remoting
			//and the Root Binder doesn't have the surogate correctly added.  Instead of bombing
			//all testing just output a warning so the lab-automation machines are updated...
			TWARNING("RootBinder not available - unable to verify URL");
		}
	}
	else
	{
		TESTC(pwszURL == NULL);
	}

CLEANUP:
	if(ppwszURL)
		*ppwszURL = pwszURL;
	else
		SAFE_FREE(pwszURL);
	SAFE_RELEASE(pIRow);
	SAFE_RELEASE(pIGetRow);
	return hr;
}



////////////////////////////////////////////////////////////////////
// Class CRowsetChange
//
// Conatins all the functionality of IRowsetChange as well as all the
// IRowset functionality
////////////////////////////////////////////////////////////////////

CRowsetChange::CRowsetChange(WCHAR* pwszTestCaseName) : CRowset(pwszTestCaseName)
{
	//private
	m_pIRowsetChange    = NULL;
}

CRowsetChange::~CRowsetChange()
{
	CRowsetChange::Terminate();
}

BOOL CRowsetChange::Terminate()
{
	SAFE_RELEASE(m_pIRowsetChange);
	return CRowset::Terminate();
}

IRowsetChange* const CRowsetChange::pIRowsetChange()
{
	if(!m_pIRowsetChange)
		TESTC_(QI(pIUnknown(),IID_IRowsetChange, (void**)&m_pIRowsetChange),S_OK);

CLEANUP:
	return m_pIRowsetChange;
}

HRESULT CRowsetChange::ModifyRow(HROW hRow)
{
	return ModifyRow(ONE_ROW,&hRow);
}

HRESULT CRowsetChange::ModifyRow(DBCOUNTITEM cRows, HROW* rghRow)
{
	TBEGIN
	ASSERT(cRows && rghRow);

	HRESULT hr = E_FAIL;
	void* pData = NULL;

	for(DBCOUNTITEM i=0; i<cRows; i++)
	{
		//Create new row data
		TESTC(MakeRowData(&pData))
		
		//SetData using the new data
		hr = SetData(rghRow[i], m_hAccessor, pData);

		//Release the data
		TESTC(ReleaseRowData(pData))
		pData = NULL;
	}

CLEANUP:
	ReleaseRowData(pData);
	return hr;
}


HRESULT CRowsetChange::SetData(HROW hRow, HACCESSOR hAccessor, void* pData)
{
	TBEGIN
	ASSERT(hRow && hAccessor);
	HRESULT hr = S_OK;

	DBACCESSORFLAGS dwAccessorFlags;
	DBCOUNTITEM cBindings = 0;
	DBBINDING* rgBindings = NULL;
	CRowObject RowObject;

	//Does the provider support Row Objects?
	hr = RowObject.CreateRowObject(pIRowset(), hRow);

	//Verify Results...
	if(SUCCEEDED(hr) && hAccessor && hRow && pData)
	{
		//Obtain the accessor bindings
		QTESTC_(hr = pIAccessor()->GetBindings(hAccessor, &dwAccessorFlags, &cBindings, &rgBindings),S_OK)

		//Obtain the data
		hr = RowObject.SetColumns(cBindings, rgBindings, pData);
	}
	else
	{
		//SetData
		hr = pIRowsetChange()->SetData(hRow, hAccessor, pData);
 	}
	
	//Display any binding errors and status'
	TESTC(VerifyBindings(hr, pIAccessor(), hAccessor, pData));

	if(hr==S_OK)
	{
	}
	else
	{
	}

CLEANUP:
	FreeAccessorBindings(cBindings, rgBindings);
	return hr;
}



HRESULT CRowsetChange::DeleteRow(HROW hRow)
{
	return DeleteRow(ONE_ROW,&hRow);
}

HRESULT CRowsetChange::DeleteRow(DBCOUNTITEM cRows, HROW* rghRow, DBROWSTATUS* rgRowStatus)
{
	TBEGIN
	ASSERT(cRows && rghRow);
	
	//Delete the rows
	HRESULT hr = pIRowsetChange()->DeleteRows(NULL,cRows,rghRow,rgRowStatus);
	
	//Verify no errors
	if(SUCCEEDED(hr) && rgRowStatus)
	{
		TESTC(VerifyArray(cRows,rgRowStatus,DBROWSTATUS_S_OK))
	}
	else
	{
	}

CLEANUP:
	return hr;
}


HRESULT CRowsetChange::InsertRow(HROW* phRow)
{
	return InsertRow(ONE_ROW,phRow);
}

HRESULT CRowsetChange::InsertRow(DBCOUNTITEM cRows, HROW* rghRow)
{
	TBEGIN
	ASSERT(cRows && rghRow);

	HRESULT hr = E_FAIL;
	void* pData = NULL;

	//Insert new row(s), using pData (row 1 data)
	for(DBCOUNTITEM i=0; i<cRows; i++)
	{
		//This has to be done every time, to guarentee a unique value for the index column
		TESTC(MakeRowData(&pData))

		//IRowsetChange->InsertRow
		hr = InsertRow(m_hAccessor, pData, &rghRow[i]);
		
		//Release the bindings
		TESTC(ReleaseRowData(pData))
		pData = NULL;
	}

CLEANUP:
	ReleaseRowData(pData);
	return hr;
}


HRESULT CRowsetChange::InsertRow(HACCESSOR hAccessor, void* pData, HROW* phRow)
{
	TBEGIN

	//IRowsetChange::InsertRow
	HRESULT hr = pIRowsetChange()->InsertRow(NULL, hAccessor, pData, phRow);
		
	//Display any binding errors and status'
	TESTC(VerifyBindings(hr, pIAccessor(), hAccessor, pData));

	//Verify valid returned row handle
	if(SUCCEEDED(hr))
	{
		if(phRow)
			TESTC(*phRow !=DB_NULL_HROW);
	}
	else
	{
		if(phRow)
			TESTC(*phRow == DB_NULL_HROW);
	}

CLEANUP:
	return hr;
}


HRESULT CRowsetChange::CreateRowset
(
	DBPROPID			dwPropertyID,
	EQUERY				eQuery,					//the type of rowset to create
	REFIID				riid,					//riid to ask for
	CTable*				pCTable,
	DBACCESSORFLAGS		dwAccessorFlags,		//the accessor flags
	DBPART				dwPart,					//the type of binding
	DWORD				dwColsToBind,			//the columns in accessor
	ECOLUMNORDER		eBindingOrder,			//the order to bind columns
	ECOLS_BY_REF		eColsByRef,				//which columns to bind by reference
	DBTYPE				wTypeModifier,			//the type modifier used for accessor
	BLOBTYPE			dwBlobType				//BLOB option
)
{
	TBEGIN
	HRESULT hr = E_FAIL;
	
	//Set the desired property
	TESTC(SetProperty(dwPropertyID, DBPROPSET_ROWSET))

	//Delegate out to CreateRowset
	hr = CRowsetChange::CreateRowset(eQuery, riid, pCTable, dwAccessorFlags, dwPart, dwColsToBind, eBindingOrder, eColsByRef, wTypeModifier, dwBlobType);

CLEANUP:
	return hr;
}

HRESULT CRowsetChange::CreateRowset
(
	EQUERY				eQuery,					//the type of rowset to create
	REFIID				riid,					//riid to ask for
	CTable*				pCTable,
	DBACCESSORFLAGS		dwAccessorFlags,		//the accessor flags
	DBPART				dwPart,					//the type of binding
	DWORD				dwColsToBind,			//the columns in accessor
	ECOLUMNORDER		eBindingOrder,			//the order to bind columns
	ECOLS_BY_REF		eColsByRef,				//which columns to bind by reference
	DBTYPE				wTypeModifier,			//the type modifier used for accessor
	BLOBTYPE			dwBlobType				//BLOB option
)
{
	TBEGIN
	HRESULT hr = E_FAIL;

	//Set the apporpiate properties
	TESTC(SetProperty(DBPROP_IRowsetChange))
	
	//Deletgate to the rowset object
	QTESTC_(hr = CRowset::CreateRowset(eQuery, riid, pCTable, dwAccessorFlags, dwPart, dwColsToBind, eBindingOrder, eColsByRef, wTypeModifier, dwBlobType),S_OK)
	
	//Obtain the Approatiate interface pointers
	m_pIRowsetChange = pIRowsetChange();	
	TESTC(m_pIRowsetChange != NULL);

CLEANUP:
	return hr;
}

HRESULT CRowsetChange::CreateRowset
(
	IUnknown*			pIRowset,
	EQUERY				eQuery,					//the type of rowset to create
	REFIID				riid,					//riid to ask for
	CTable*				pCTable,
	DBACCESSORFLAGS		dwAccessorFlags,		//the accessor flags
	DBPART				dwPart,					//the type of binding
	DWORD				dwColsToBind,			//the columns in accessor
	ECOLUMNORDER		eBindingOrder,			//the order to bind columns
	ECOLS_BY_REF		eColsByRef,				//which columns to bind by reference
	DBTYPE				wTypeModifier,			//the type modifier used for accessor
	BLOBTYPE			dwBlobType				//BLOB option
)
{
	TBEGIN
	HRESULT hr = E_FAIL;

	//InvalidArg case
	if(pIRowset==NULL)
		return E_FAIL;
	
	//Set the apporpiate properties
	TESTC(SetProperty(DBPROP_IRowsetChange))
	
	//Deletgate to the rowset copy constructor
	QTESTC_(hr = CRowset::CreateRowset(pIRowset, eQuery, riid, pCTable, dwAccessorFlags, dwPart, dwColsToBind, eBindingOrder, eColsByRef, wTypeModifier, dwBlobType),S_OK)

	//Obtain the Approatiate interface pointers
	m_pIRowsetChange = pIRowsetChange();	
	if(m_pIRowsetChange == NULL)
	{
		hr = E_FAIL;
		goto CLEANUP;
	}	
	
CLEANUP:
    return hr;
}


//////////////////////////////////////////////////////////////////
// Class CRowsetUpdate
//
// Conatins all the functionality of IRowsetUpdate as well as the
// ability of CRowsetChange
//////////////////////////////////////////////////////////////////

CRowsetUpdate::CRowsetUpdate(WCHAR* pwszTestCaseName) : CRowsetChange(pwszTestCaseName)
{
	//private
	m_pIRowsetUpdate    = NULL;
	m_ulMaxPendingRows  = 0;
}

CRowsetUpdate::~CRowsetUpdate()
{
	CRowsetUpdate::Terminate();
}

BOOL CRowsetUpdate::Terminate()
{
	SAFE_RELEASE(m_pIRowsetUpdate);
	return CRowsetChange::Terminate();
}

IRowsetUpdate* const CRowsetUpdate::pIRowsetUpdate()
{
	if(!m_pIRowsetUpdate)
		TESTC_(QI(pIUnknown(),IID_IRowsetUpdate, (void**)&m_pIRowsetUpdate),S_OK);

CLEANUP:
	return m_pIRowsetUpdate;
}


HRESULT CRowsetUpdate::GetPendingRows(DBCOUNTITEM PendingRows)
{
	return GetPendingRows(DBPENDINGSTATUS_ALL, PendingRows);
}
HRESULT CRowsetUpdate::GetPendingRows(DBROWSTATUS RowStatus, DBCOUNTITEM PendingRows)
{
    return GetPendingRows(RowStatus, PendingRows, DBPENDINGSTATUS_ALL);
}
HRESULT CRowsetUpdate::GetPendingRows(DBPENDINGSTATUS dwRowStatus, DBCOUNTITEM PendingRows, DBPENDINGSTATUS PendingStatus)
{
	TBEGIN
	DBCOUNTITEM cPendingRows = 0;
	HROW* rgPendingRows = NULL;
	DBPENDINGSTATUS* rgPendingStatus = NULL;
	DBCOUNTITEM i;

	HRESULT hr = E_FAIL;
	
	if(PendingRows) //Should be pending changes
	{
		//GetPendingRows
		TESTC_(hr = pIRowsetUpdate()->GetPendingRows(NULL,dwRowStatus,&cPendingRows,&rgPendingRows,&rgPendingStatus),S_OK)
		TESTC(cPendingRows==PendingRows && rgPendingRows!=NULL && rgPendingStatus!=NULL)

		//If want the row status, verify array 
        if(dwRowStatus != DBPENDINGSTATUS_ALL) 
		    for(i=0; i<cPendingRows; i++)
			    TESTC(rgPendingRows[i]!=DB_NULL_HROW && (rgPendingStatus[i] & PendingStatus))
	}
	else // Should be no pending changes
	{	
		//GetPendingRows
		TESTC_(hr = pIRowsetUpdate()->GetPendingRows(NULL,dwRowStatus,&cPendingRows,&rgPendingRows,&rgPendingStatus),S_FALSE)
		COMPC(cPendingRows,NO_ROWS)
		TESTC(rgPendingRows==NULL && rgPendingStatus==NULL)  
	}


CLEANUP:
	ReleaseRows(cPendingRows,rgPendingRows);
	PROVIDER_FREE(rgPendingRows);
	PROVIDER_FREE(rgPendingStatus);
	return hr;
}

HRESULT CRowsetUpdate::GetOriginalData(HROW hRow, void** ppData)
{
	return GetOriginalData(ONE_ROW,&hRow,ppData);
}
HRESULT CRowsetUpdate::GetOriginalData(DBCOUNTITEM cRows, HROW* rghRow, void** rgpData)
{
	TBEGIN
	ASSERT(cRows && rghRow && *rghRow && rgpData);

	HRESULT hr = E_FAIL;
	for(DBCOUNTITEM i=0; i<cRows; i++)
	{
		if(rgpData[i]==NULL) rgpData[i] = PROVIDER_ALLOC(SIZEOF_ONEROW);
		TESTC(rgpData[i]!=NULL)
		
		hr = pIRowsetUpdate()->GetOriginalData(rghRow[i],m_hAccessor,rgpData[i]);

		//Display any binding errors and status'
		TESTC(VerifyBindings(hr, pIAccessor(), m_hAccessor, rgpData[i]));
		QTESTC_(hr, S_OK);
	}

CLEANUP:
	return hr;
}


BOOL CRowsetUpdate::CompareWithDefaults(void* pOriginalData)
{
	TBEGIN
	//This method should be called to see if GetOriginalData does indend
	//return defaults for pending inserted rows.  
	//The problem is that I have no clue what the defaults are supposed to be? 
	
	//The best that I can really do is to verify that all the status is 
	//DBSTATUS_S_OK or the status is DBSTATUS_S_ISNULL...
	DBSTATUS dwStatus = 0;
	for(DBCOUNTITEM i=0; i<m_cBindings; i++)
	{
		//It would be an error to call this method an not have bound the status
		ASSERT(STATUS_IS_BOUND(m_rgBinding[i]));

		//Verify the status
		dwStatus = STATUS_BINDING(m_rgBinding[i], pOriginalData);
		QTESTC(dwStatus == DBSTATUS_S_OK || dwStatus == DBSTATUS_S_ISNULL || dwStatus == DBSTATUS_E_UNAVAILABLE);
	}
	
CLEANUP:
	TRETURN;
}

BOOL CRowsetUpdate::CompareOrgRowData(HROW hRow)
{
	TBEGIN
	ASSERT(hRow != DB_NULL_HROW);

	void* pOriginalData = NULL;

	//Get the OriginalData
    TESTC_(GetOriginalData(hRow,&pOriginalData),S_OK)

	//Compare with the current row data
	QTESTC(CompareRowData(hRow, pOriginalData))

CLEANUP:
	PROVIDER_FREE(pOriginalData);
	TRETURN
}

HRESULT CRowsetUpdate::GetOrgStorageData(HROW hRow, HACCESSOR hAccessor, void* pBuffer, ULONG* pcBytes, REFIID riid, IUnknown** ppIUnknown)
{
	TBEGIN
	ASSERT(hRow && hAccessor);
	HRESULT hr = S_OK;

	//Alloc pData, large enough
	void* pData = PROVIDER_ALLOC(sizeof(void*)*DATA_SIZE);
	
	DBACCESSORFLAGS dwAccessorFlags;
	DBCOUNTITEM cBindings = 0;
	DBBINDING* rgBindings = NULL;

	//Obtain the accessor bindings
	QTESTC_(hr = pIAccessor()->GetBindings(hAccessor, &dwAccessorFlags, &cBindings, &rgBindings),S_OK)

	//GetData to obtain the storage pointer
	QTESTC_(hr = pIRowsetUpdate()->GetOriginalData(hRow, hAccessor, pData),S_OK)
	
	//Get Storage Data...
	hr = ::GetStorageData(cBindings, rgBindings, pData, pBuffer, pcBytes, riid, ppIUnknown);

CLEANUP:
	PROVIDER_FREE(pData);
	FreeAccessorBindings(cBindings, rgBindings);
	return hr;
}

BOOL CRowsetUpdate::CompareOrgStorageData(HROW hRow, HACCESSOR hAccessor, void* pBuffer)
{
	ASSERT(hRow && hAccessor && pBuffer);
	TBEGIN

	ULONG cBytes = 0;
	void* pStorageBuffer = PROVIDER_ALLOC(sizeof(void*)*DATA_SIZE);

	//Get the storage data
	TESTC(GetOrgStorageData(hRow, hAccessor, pStorageBuffer, &cBytes))

	//Compare storage data with passed in buffer
	QTESTC(memcmp(pBuffer, pStorageBuffer, (size_t)cBytes))

CLEANUP:
	PROVIDER_FREE(pStorageBuffer);
	TRETURN
}

HRESULT CRowsetUpdate::HardDeleteRow(HROW hRow)
{
	return HardDeleteRow(ONE_ROW,&hRow);
}
HRESULT CRowsetUpdate::HardDeleteRow(DBCOUNTITEM cRows, HROW* rghRow)
{
	TBEGIN
	ASSERT(cRows && rghRow);

	HRESULT hr = E_FAIL;

    //First soft-delete the row
	QTESTC_(hr = DeleteRow(cRows, rghRow),S_OK)

	//Now make the delete perminate, 
	QTESTC_(hr = UpdateRow(cRows, rghRow),S_OK)

CLEANUP:
	return hr;
}

HRESULT CRowsetUpdate::UpdateAll()
{
	return UpdateRow(0,NULL);
}
HRESULT CRowsetUpdate::UpdateRow(HROW hRow)
{
	return UpdateRow(ONE_ROW,&hRow);
}
HRESULT CRowsetUpdate::UpdateRow(DBCOUNTITEM cRows, HROW* rghRow, DBCOUNTITEM* pcUpdatedRows, HROW** prgUpdatedRows, DBROWSTATUS** prgRowStatus)
{
	TBEGIN

	DBCOUNTITEM cUpdatedRows = 0;
	HROW* rgUpdatedRows = NULL;
	DBROWSTATUS* rgRowStatus = NULL;

    //Call Update
	HRESULT hr = pIRowsetUpdate()->Update(NULL,cRows,rghRow,&cUpdatedRows,&rgUpdatedRows,&rgRowStatus);

	//Verify status Array
	if(cUpdatedRows)
	{
		TESTC(rgUpdatedRows!=NULL && rgRowStatus!=NULL)

		//Verify RowStatus array
		if(hr==S_OK)
 			VerifyArray(cUpdatedRows,rgRowStatus,DBROWSTATUS_S_OK);
	}
	else
		TESTC(rgUpdatedRows==NULL && rgRowStatus==NULL)

		
	//Verify correct rows are updated
	if(cRows && rghRow)
	{
		TESTC(cUpdatedRows == cRows);
		for(DBCOUNTITEM i=0; i<cUpdatedRows; i++)
			TESTC(rghRow[i] == rgUpdatedRows[i]) 
	}
	
	//Need to release the row handles only if you don't already have the rows, 
	//and you don't want them returned
	if(cRows==0 && rghRow==NULL && pcUpdatedRows==NULL && prgUpdatedRows==NULL)
		ReleaseRows(cUpdatedRows, rgUpdatedRows);
	

CLEANUP:  
	if(pcUpdatedRows)
		*pcUpdatedRows = cUpdatedRows;

	if(prgUpdatedRows)
		*prgUpdatedRows = rgUpdatedRows;
	else
		PROVIDER_FREE(rgUpdatedRows);

	if(prgRowStatus)
		*prgRowStatus = rgRowStatus;
	else
		PROVIDER_FREE(rgRowStatus);
	
	return hr;
}

HRESULT CRowsetUpdate::UndoAll()
{
	return UndoRow(0,NULL);
}											
HRESULT CRowsetUpdate::UndoRow(HROW hRow)
{
	return UndoRow(ONE_ROW,&hRow);
}
HRESULT CRowsetUpdate::UndoRow(DBCOUNTITEM cRows,HROW* rghRow,DBCOUNTITEM* pcRowsUndone,HROW** prgRowsUndone,DBROWSTATUS** prgRowStatus)
{
	TBEGIN
	
	DBCOUNTITEM cRowsUndone = 0;
	HROW* rgRowsUndone = NULL;
	DBROWSTATUS* rgRowStatus = NULL;
	
	DBCOUNTITEM i;

    //Call Undo
	HRESULT hr = pIRowsetUpdate()->Undo(NULL,cRows,rghRow,&cRowsUndone,&rgRowsUndone,&rgRowStatus);
	
	//Verify status Array
	if(cRowsUndone)
	{
		TESTC(rgRowsUndone!=NULL && rgRowStatus!=NULL)

		//Verify RowStatus array
		if(hr==S_OK)
 			VerifyArray(cRowsUndone,rgRowStatus,DBROWSTATUS_S_OK);
	}
	else
		TESTC(rgRowsUndone==NULL && rgRowStatus==NULL)

		
	//Verify rgRowsUndone
	if(cRows && SUCCEEDED(hr))
	{
		TESTC(cRowsUndone==cRows && rgRowsUndone!=NULL && rgRowStatus!=NULL);

		//Verify correct row handles returned
		if(rghRow)
			for(i=0; i<cRowsUndone; i++)
				TESTC(rghRow[i] == rgRowsUndone[i])
	}
	else
	{
		//Need to release the row handles
		//Since Undo AddRef's only if you don't already have the row
		ReleaseRows(cRowsUndone,rgRowsUndone);
	}

CLEANUP:
	if(pcRowsUndone)
		*pcRowsUndone = cRowsUndone;

	if(prgRowsUndone)
		*prgRowsUndone = rgRowsUndone;
	else
		PROVIDER_FREE(rgRowsUndone);

	if(prgRowStatus)
		*prgRowStatus = rgRowStatus;
	else
		PROVIDER_FREE(rgRowStatus);
	
	return hr;
}

HRESULT CRowsetUpdate::GetRowStatus(HROW hRow, DBROWSTATUS RowStatus)
{
	return GetRowStatus(ONE_ROW,&hRow,RowStatus);
}
HRESULT CRowsetUpdate::GetRowStatus(DBCOUNTITEM cRows, HROW* rghRow, DBPENDINGSTATUS dwRowStatus)
{
	TBEGIN
	ASSERT(cRows && rghRow && *rghRow!=DB_NULL_HROW);

	HRESULT hr = E_FAIL;
	//Need to create the DBROWSTATUS array
	DBPENDINGSTATUS* rgPendingStatus = PROVIDER_ALLOC_(cRows,DBPENDINGSTATUS);
	TESTC(rgPendingStatus!=NULL)

	TESTC_(hr = pIRowsetUpdate()->GetRowStatus(NULL,cRows,rghRow,rgPendingStatus),S_OK)

	//Need to verify that all the status returned matches RowStatus
	QTESTC(VerifyArray(cRows,rgPendingStatus,dwRowStatus))

CLEANUP:
	//Dealloc the DBROWSTATUS array
	PROVIDER_FREE(rgPendingStatus);
	return hr;
}


HRESULT CRowsetUpdate::CreateRowset
(
	DBPROPID			dwPropertyID,
	EQUERY				eQuery,					//the type of rowset to create
	REFIID				riid,					//riid to ask for
	CTable*				pCTable,
	DBACCESSORFLAGS		dwAccessorFlags,		//the accessor flags
	DBPART				dwPart,					//the type of binding
	DWORD				dwColsToBind,			//the columns in accessor
	ECOLUMNORDER		eBindingOrder,			//the order to bind columns
	ECOLS_BY_REF		eColsByRef,				//which columns to bind by reference
	DBTYPE				wTypeModifier,			//the type modifier used for accessor
	BLOBTYPE			dwBlobType				//BLOB option
)
{
	TBEGIN
	HRESULT hr = E_FAIL;
	
	//Set the desired property
	TESTC(SetProperty(dwPropertyID, DBPROPSET_ROWSET))

	//Delegate out to CreateRowset
	hr = CRowsetUpdate::CreateRowset(eQuery, riid, pCTable, dwAccessorFlags, dwPart, dwColsToBind, eBindingOrder, eColsByRef, wTypeModifier, dwBlobType);

CLEANUP:
	return hr;
}

HRESULT CRowsetUpdate::CreateRowset
(
	EQUERY				eQuery,					//the type of rowset to create
	REFIID				riid,					//riid to ask for
	CTable*				pCTable,
	DBACCESSORFLAGS		dwAccessorFlags,		//the accessor flags
	DBPART				dwPart,					//the type of binding
	DWORD				dwColsToBind,			//the columns in accessor
	ECOLUMNORDER		eBindingOrder,			//the order to bind columns
	ECOLS_BY_REF		eColsByRef,				//which columns to bind by reference
	DBTYPE				wTypeModifier,			//the type modifier used for accessor
	BLOBTYPE			dwBlobType				//BLOB option
)
{
	TBEGIN
	HRESULT hr = E_FAIL;

	//Set the apporpiate properties
	TESTC(SetProperty(DBPROP_IRowsetUpdate))
	
	//Deletgate to the rowset object
	QTESTC_(hr = CRowsetChange::CreateRowset(eQuery, riid, pCTable, dwAccessorFlags, dwPart, dwColsToBind, eBindingOrder, eColsByRef, wTypeModifier, dwBlobType),S_OK)

	//Obtain the Approatiate interface pointers
	m_pIRowsetUpdate = pIRowsetUpdate();	
	if(m_pIRowsetUpdate == NULL)
	{
		hr = E_FAIL;
		goto CLEANUP;
	}	

	//MaxPendingRows
	m_ulMaxPendingRows = 0;
	GetProperty(DBPROP_MAXPENDINGROWS, DBPROPSET_ROWSET, &m_ulMaxPendingRows);

CLEANUP:
	return hr;
}

HRESULT CRowsetUpdate::CreateRowset
(
	IUnknown*			pIRowset,
	EQUERY				eQuery,					//the type of rowset to create
	REFIID				riid,					//riid to ask for
	CTable*				pCTable,
	DBACCESSORFLAGS		dwAccessorFlags,		//the accessor flags
	DBPART				dwPart,					//the type of binding
	DWORD				dwColsToBind,			//the columns in accessor
	ECOLUMNORDER		eBindingOrder,			//the order to bind columns
	ECOLS_BY_REF		eColsByRef,				//which columns to bind by reference
	DBTYPE				wTypeModifier,			//the type modifier used for accessor
	BLOBTYPE			dwBlobType				//BLOB option
)
{
	TBEGIN
	HRESULT hr = E_FAIL;

	//InvalidArg case
	if(pIRowset==NULL)
		return E_FAIL;
	
    //Set the apporpiate properties
	TESTC(SetProperty(DBPROP_IRowsetUpdate))
	
	//Deletgate to the rowset change copy constructor
	QTESTC_(hr = CRowsetChange::CreateRowset(pIRowset, eQuery, riid, pCTable, dwAccessorFlags, dwPart, dwColsToBind, eBindingOrder, eColsByRef, wTypeModifier, dwBlobType),S_OK)

	//Obtain the Approatiate interface pointers
	m_pIRowsetUpdate = pIRowsetUpdate();	
	if(m_pIRowsetUpdate == NULL)
	{
		hr = E_FAIL;
		goto CLEANUP;
	}	

	//MaxPendingRows
	m_ulMaxPendingRows = 0;
	GetProperty(DBPROP_MAXPENDINGROWS, DBPROPSET_ROWSET, &m_ulMaxPendingRows);

CLEANUP:  
    return hr;
}



///////////////////////////////////////////////////////////////////////////////
// Class CListener
// 
// Listens on the connections for notifications, (ie: a sink)
///////////////////////////////////////////////////////////////////////////////
CListener::CListener(REFIID riid, IUnknown* pIUnknown)
	: CBase(NULL, NULL)
{
	m_pCImpIRowsetNotify		= NULL;
	m_pCImpIDBAsynchNotify		= NULL;
	m_pCImpIRowPositionChange	= NULL;

	m_pIUnknown				= pIUnknown;
	m_iid					= riid;

	Restart();
}

CListener::~CListener() 
{
	SAFE_DELETE(m_pCImpIRowsetNotify);
	SAFE_DELETE(m_pCImpIDBAsynchNotify);
	SAFE_DELETE(m_pCImpIRowPositionChange);
}

HRESULT CListener::QueryInterface
    (
    REFIID riid,        //@parm IN | Interface ID of the interface being queried for.
    LPVOID * ppv        //@parm OUT | Pointer to interface that was instantiated
    )
{
	if(!ppv)
		return E_INVALIDARG;
	*ppv = NULL;

	//IUNKNOWN
	if(riid == IID_IUnknown)
	{
		*ppv = this;
	}
	//MANADATORY
	//OPTIONAL
	else if(riid == IID_IRowsetNotify && (m_iid == IID_IUnknown || m_iid == IID_IRowsetNotify))
	{
		if(!m_pCImpIRowsetNotify)
			m_pCImpIRowsetNotify = new CImpIRowsetNotify(this, NULL);
		*ppv = m_pCImpIRowsetNotify;
	}
	else if(riid == IID_IDBAsynchNotify && (m_iid == IID_IUnknown || m_iid == IID_IDBAsynchNotify))
	{
		if(!m_pCImpIDBAsynchNotify)
			m_pCImpIDBAsynchNotify = new CImpIDBAsynchNotify(this, NULL);
		*ppv = m_pCImpIDBAsynchNotify;
	}
	else if(riid == IID_IRowPositionChange && (m_iid == IID_IUnknown || m_iid == IID_IRowPositionChange))
	{
		if(!m_pCImpIRowPositionChange)
			m_pCImpIRowPositionChange = new CImpIRowPositionChange(this, NULL);
		*ppv = m_pCImpIRowPositionChange;
	}
	//NOTSUPPORTED
	else
	{
		*ppv = NULL;
		return E_NOINTERFACE;
	}

	if(!*ppv)
		return E_OUTOFMEMORY;

	((IUnknown*)*ppv)->AddRef();
	return S_OK;
}


BOOL CListener::Restart()
{
	m_eCancelReason= -1;
	m_eCancelPhase = -1;

	m_eAdviseReason		= -1;
	m_eAdvisePhase		= -1;
	m_eUnadviseReason	= -1;
	m_eUnadvisePhase	= -1;

	m_hrReturn			= S_OK;
	m_bFire				= TRUE;
	m_dwCookie			= 0;
	
	m_pReEntrantFunc	= NULL;
	m_pReEntrantData	= NULL;

	//Init which reasons/phases to fire notifcations for
	SetEvent(DBREASON_ALL, DBEVENTPHASE_ALL, TRUE);

	//Init times notified
	ResetTimesNotified(); 
	return TRUE;
}

HRESULT CListener::AcceptOrVeto(
		IUnknown*       pIUnknown,
		DBREASON		eReason,		// @parm IN	| Reason for notification
		DBEVENTPHASE	ePhase, 		// @parm IN	| Phase of event
		BOOL            fCantDeny)
{
	//Should this Listener Accept for veto the change.
	ASSERT(eReason <= DBREASON_ALL);	// DBREASON is unsigned and reasons start at zero so we don't need to check lower bound here
	ASSERT(ePhase  <= DBEVENTPHASE_ALL);	// DBEVENTPHASE is unsigned and phases start at zero so we don't need to check lower bound here

	//record that the source notified the Listener
	m_rgNotified[eReason][ePhase]++;

	//Do we want any notifications
	if(!m_bFire)
	    return S_OK;

	//Do we want this reason at all
	if(!WantedEvent(eReason, DBEVENTPHASE_ALL))
	    return DB_S_UNWANTEDREASON;
	
	//Do we want this phase of the reason
	if(!WantedEvent(eReason, ePhase))
		return DB_S_UNWANTEDPHASE;

	//Do we continue to Accept for Veto this change
	if(m_eCancelReason == eReason && m_eCancelPhase == ePhase)
		return S_FALSE;
	
	//We may want to advise or unadvise ourseleves during this notification...
	if(m_eAdviseReason == eReason && m_eAdvisePhase == ePhase)
	{
		Advise(&m_dwCookie, pIUnknown);
		m_eAdviseReason = -1;
		m_eAdvisePhase  = -1;
	}
	if(m_eUnadviseReason == eReason && m_eUnadvisePhase == ePhase)
	{
		Unadvise(m_dwCookie, pIUnknown);
		m_eUnadviseReason = -1;
		m_eUnadvisePhase  = -1;
	}

	return m_hrReturn;
}


BOOL CListener::SetEvent(DBREASON eReason, DBEVENTPHASE ePhase, BOOL bValue)
{
	ASSERT(eReason <= DBREASON_ALL);	// DBREASON is unsigned and reasons start at zero so we don't need to check lower bound here
	ASSERT(ePhase  <= DBEVENTPHASE_ALL);	// DBEVENTPHASE is unsigned and phases start at zero so we don't need to check lower bound here

	//Can return the number of times notified for either all reasons / phases
	//or just a particular reason / phase

	for(ULONG iReason=0; iReason<DBREASON_ALL; iReason++)
	{
		if(iReason == eReason || eReason == DBREASON_ALL)
			for(ULONG iPhase=0; iPhase<DBEVENTPHASE_ALL; iPhase++)
			{
				if(iPhase == ePhase || ePhase == DBEVENTPHASE_ALL)
					m_rgEvent[iReason][iPhase] = bValue;
			}
	}
	
	return TRUE;
}

BOOL CListener::WantedEvent(DBREASON eReason, DBEVENTPHASE ePhase)
{
	ASSERT(eReason <= DBREASON_ALL);	// DBREASON is unsigned and reasons start at zero so we don't need to check lower bound here
	ASSERT(ePhase  <= DBEVENTPHASE_ALL);	// DBEVENTPHASE is unsigned and phases start at zero so we don't need to check lower bound here

	for(ULONG iReason=0; iReason<DBREASON_ALL; iReason++)
	{
		if(iReason == eReason || eReason == DBREASON_ALL)
			for(ULONG iPhase=0; iPhase<DBEVENTPHASE_ALL; iPhase++)
			{
				if(iPhase == ePhase || ePhase == DBEVENTPHASE_ALL)
					if(m_rgEvent[iReason][iPhase])
						return TRUE;
			}
	}

	return FALSE;
}


ULONG CListener::GetTimesNotified(DBREASON eReason, DBEVENTPHASE ePhase)
{
	ASSERT(eReason <= DBREASON_ALL);	// DBREASON is unsigned and reasons start at zero so we don't need to check lower bound here
	ASSERT(ePhase  <= DBEVENTPHASE_ALL);	// DBEVENTPHASE is unsigned and phases start at zero so we don't need to check lower bound here

	//Can return the number of times notified for either all reasons / phases
	//or just a particular reason / phase

	ULONG cTimesNotified = 0;

	for(ULONG iReason=0; iReason<DBREASON_ALL; iReason++)
	{
		if(iReason == eReason || eReason == DBREASON_ALL)
			for(ULONG iPhase=0; iPhase<DBEVENTPHASE_ALL; iPhase++)
			{
				if(iPhase == ePhase || ePhase == DBEVENTPHASE_ALL)
					cTimesNotified += m_rgNotified[iReason][iPhase];
			}
	}
	
	return cTimesNotified;
}
	
BOOL CListener::ResetTimesNotified(DBREASON eReason, DBEVENTPHASE ePhase)
{
	ASSERT(eReason <= DBREASON_ALL);	// DBREASON is unsigned and reasons start at zero so we don't need to check lower bound here
	ASSERT(ePhase  <= DBEVENTPHASE_ALL);	// DBEVENTPHASE is unsigned and phases start at zero so we don't need to check lower bound here

	//Can Reaset all times notified for either all reasons / phases
	//or just a particular reason / phase

	for(ULONG iReason=0; iReason<DBREASON_ALL; iReason++)
	{
		if(iReason == eReason || eReason == DBREASON_ALL)
			for(ULONG iPhase=0; iPhase<DBEVENTPHASE_ALL; iPhase++)
			{
				if(iPhase == ePhase || ePhase == DBEVENTPHASE_ALL)
					m_rgNotified[iReason][iPhase] = 0;
			}
	}
	
	return TRUE;
}

HRESULT CListener::Advise(DWORD* pdwCookie, IUnknown* pIUnknown, REFIID riid)
{
	ASSERT(pdwCookie);
	TBEGIN

	HRESULT hr = S_OK;
	IConnectionPoint* pICP = NULL;
	IConnectionPointContainer* pICPC = NULL;

	//Obtain the connection point container
	TESTC_(hr = QI(pIUnknown ? pIUnknown : m_pIUnknown, IID_IConnectionPointContainer, (void **)&pICPC),S_OK)
	TESTC(pICPC != NULL);

	//Obtain the IRowsetNotify connection point 
	TESTC_(hr = pICPC->FindConnectionPoint(riid != IID_IUnknown ? riid : m_iid, &pICP),S_OK)
	TESTC(pICP != NULL);

	//Now we can advise the connection
	TESTC_(hr = pICP->Advise(this, pdwCookie),S_OK)

CLEANUP:
	SAFE_RELEASE(pICP);
	SAFE_RELEASE(pICPC);
	return hr;
}

HRESULT CListener::Unadvise(DWORD dwCookie, IUnknown* pIUnknown, REFIID riid)
{
	TBEGIN

	HRESULT hr = S_OK;
	IConnectionPoint* pICP = NULL;
	IConnectionPointContainer* pICPC = NULL;

	//Cookie can't be 0
	TESTC(dwCookie != 0);

	//Obtain the connection point container
	TESTC_(hr = QI(pIUnknown ? pIUnknown : m_pIUnknown,IID_IConnectionPointContainer,(void **)&pICPC),S_OK)
	TESTC(pICPC != NULL);

	//Obtain the IRowsetNotify connection point 
	TESTC_(hr = pICPC->FindConnectionPoint(riid != IID_IUnknown ? riid : m_iid, &pICP),S_OK)
	TESTC(pICP != NULL);

	//Now we can advise the connection
	TESTC_(hr = pICP->Unadvise(dwCookie),S_OK)

CLEANUP:
	SAFE_RELEASE(pICP);
	SAFE_RELEASE(pICPC);
	return hr;
}


BOOL CListener::SetReEntrantcy(DBREASON eReason, DBEVENTPHASE ePhase, void* pReEntrantFunc, void* pReEntrantData)
{
	m_pReEntrantFunc	= pReEntrantFunc;
	m_pReEntrantData	= pReEntrantData;
	m_eReEntrantReason	= eReason;
	m_eReEntrantPhase	= ePhase;
	return TRUE;
}

BOOL CListener::GetReEntrantcy(DBREASON eReason, DBEVENTPHASE ePhase, void** ppReEntrantFunc, void** ppReEntrantData)
{
	if(eReason == m_eReEntrantReason && ePhase == m_eReEntrantPhase && m_pReEntrantFunc)
	{
		ASSERT(ppReEntrantFunc);
		ASSERT(ppReEntrantData);
		*ppReEntrantFunc = m_pReEntrantFunc;
		*ppReEntrantData = m_pReEntrantData;
		return TRUE;
	}
	
	return FALSE;
}


HRESULT CListener::CauseNotification(DBREASON eReason, IUnknown* pIUnknown)
{
	TBEGIN
	HRESULT hr = S_OK;
	IRowset* pIRowset = NULL;

	if(pIUnknown == NULL)
		pIUnknown = m_pIUnknown;

	//Obtain the rowset interface...
	TESTC_(QI(pIUnknown, IID_IRowset, (void**)&pIRowset),S_OK);

	//Need to cause a notification to occur, of type eReason
	switch(eReason)
	{
		case DBREASON_ROW_ACTIVATE:
		{
			DBCOUNTITEM cRowsObtained = 0;
			HROW* rghRows = NULL;

			//cause notification DBREASON_ROW_ACTIVATE to be sent 
			QTESTC_(hr = pIRowset->GetNextRows(NULL,0,ONE_ROW,&cRowsObtained,&rghRows),S_OK)
			
			//Need to grab a release a row once grabbed, 
			//but don't want to send this notification
			SetNotifications(OFF);
				QTESTC_(hr = pIRowset->ReleaseRows(cRowsObtained,rghRows,NULL,NULL,NULL),S_OK)
				PROVIDER_FREE(rghRows);
			SetNotifications(ON);
			break;
		}

		case DBREASON_ROW_RELEASE:	
		{
			DBCOUNTITEM cRowsObtained = 0;
			HROW* rghRows = NULL;

			//Need to grab a row to release it, 
			//but don't want to send this notification
			SetNotifications(OFF);
				QTESTC_(hr = pIRowset->GetNextRows(NULL,0,ONE_ROW,&cRowsObtained,&rghRows),S_OK)
			SetNotifications(ON);
			
			//cause notification DBREASON_ROW_RELEASE to be sent 
			QTESTC_(hr = pIRowset->ReleaseRows(cRowsObtained,rghRows,NULL,NULL,NULL),S_OK)
			PROVIDER_FREE(rghRows);
			break;
		}

		//Other values are not valid reasons, yet...
		case DBREASON_ROWSET_FETCHPOSITIONCHANGE:
		case DBREASON_ROWSET_RELEASE:
		case DBREASON_COLUMN_SET:	
		case DBREASON_COLUMN_RECALCULATED:
		case DBREASON_ROW_DELETE:
		case DBREASON_ROW_FIRSTCHANGE:
		case DBREASON_ROW_INSERT:
		case DBREASON_ROW_RESYNCH:
		case DBREASON_ROW_UNDOCHANGE:
		case DBREASON_ROW_UNDOINSERT:	
		case DBREASON_ROW_UNDODELETE:
		case DBREASON_ROW_UPDATE:

		default:
			ASSERT(FALSE); //Unsupported Tyoes, yet...
	}
	
CLEANUP:
	SAFE_RELEASE(pIRowset);
	return hr;
}


BOOL CListener::IsCancelable(DBREASON eReason, DBEVENTPHASE ePhase, IUnknown* pIUnknown)
{
	ULONG_PTR ulCancelable = 0;
	if(pIUnknown == NULL)
		pIUnknown = m_pIUnknown;

	switch(eReason)
	{
		case DBREASON_ROWSET_FETCHPOSITIONCHANGE:
			GetProperty(DBPROP_NOTIFYROWSETFETCHPOSITIONCHANGE, DBPROPSET_ROWSET, pIUnknown, &ulCancelable);
			break;

		case DBREASON_ROWSET_RELEASE:
			GetProperty(DBPROP_NOTIFYROWSETRELEASE, DBPROPSET_ROWSET, pIUnknown, &ulCancelable);
			break;

		case DBREASON_COLUMN_SET:
			GetProperty(DBPROP_NOTIFYCOLUMNSET, DBPROPSET_ROWSET, pIUnknown, &ulCancelable);
			break;

		case DBREASON_ROW_DELETE:
			GetProperty(DBPROP_NOTIFYROWDELETE, DBPROPSET_ROWSET, pIUnknown, &ulCancelable);
			break;

		case DBREASON_ROW_FIRSTCHANGE:
			GetProperty(DBPROP_NOTIFYROWFIRSTCHANGE, DBPROPSET_ROWSET, pIUnknown, &ulCancelable);
			break;

		case DBREASON_ROW_INSERT:
			GetProperty(DBPROP_NOTIFYROWINSERT, DBPROPSET_ROWSET, pIUnknown, &ulCancelable);
			break;

		case DBREASON_ROW_RESYNCH:
			GetProperty(DBPROP_NOTIFYROWRESYNCH, DBPROPSET_ROWSET, pIUnknown, &ulCancelable);
			break;

		case DBREASON_ROW_UNDOCHANGE:
			GetProperty(DBPROP_NOTIFYROWUNDOCHANGE, DBPROPSET_ROWSET, pIUnknown, &ulCancelable);
			break;

		case DBREASON_ROW_UNDOINSERT:
			GetProperty(DBPROP_NOTIFYROWUNDOINSERT, DBPROPSET_ROWSET, pIUnknown, &ulCancelable);
			break;

		case DBREASON_ROW_UNDODELETE:
			GetProperty(DBPROP_NOTIFYROWUNDODELETE, DBPROPSET_ROWSET, pIUnknown, &ulCancelable);
			break;

		case DBREASON_ROW_UPDATE:
			GetProperty(DBPROP_NOTIFYROWUPDATE, DBPROPSET_ROWSET, pIUnknown, &ulCancelable);
			break;

		case DBREASON_ROWSET_CHANGED:
			GetProperty(DBPROP_NOTIFYROWSETCHANGED, DBPROPSET_ROWSET, pIUnknown, &ulCancelable);
			break;

		//Only DIDEVENT is fired for the following reasons, so they are not cancelable
		case DBREASON_COLUMN_RECALCULATED:
		case DBREASON_ROW_ACTIVATE:
		case DBREASON_ROW_RELEASE:
			break;

		default:
			ASSERT(!L"Unhandled case...");
	};

	switch(ePhase)
	{
		case DBEVENTPHASE_OKTODO:
			return (BOOL)(ulCancelable & DBPROPVAL_NP_OKTODO);
		
		case DBEVENTPHASE_ABOUTTODO:
			return (BOOL)(ulCancelable & DBPROPVAL_NP_ABOUTTODO);

		case DBEVENTPHASE_SYNCHAFTER:
			return (BOOL)(ulCancelable & DBPROPVAL_NP_SYNCHAFTER);

		case DBEVENTPHASE_FAILEDTODO:
		case DBEVENTPHASE_DIDEVENT:
			break;

		default:
			ASSERT(!L"Unhandled case...");
	}

	return FALSE;
}



///////////////////////////////////////////////////////////////////////////////
// Class CImpIRowsetNotify
// 
// Implementation of IRowsetNotify
///////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CImpIRowsetNotify::OnFieldChange( 
            /* [in] */ IRowset __RPC_FAR *pRowset,
            /* [in] */ HROW hRow,
            /* [in] */ DBORDINAL cColumns,
            /* [size_is][in] */ DBORDINAL __RPC_FAR rgColumns[  ],
            /* [in] */ DBREASON eReason,
            /* [in] */ DBEVENTPHASE ePhase,
            /* [in] */ BOOL fCantDeny)
{
	//Accept / Veto the change, and do processing
	HRESULT hr = AcceptOrVeto(pRowset, eReason, ePhase, fCantDeny, 0, NULL, cColumns, rgColumns);

	//Display Notification is user is interested...
	PRVTRACE(L"    0x%08x::OnFieldChange(0x%08x, 0x%08x, %d, 0x%08x, %s, %s, %s) - %s\n", this, pRowset, hRow, cColumns, rgColumns, GetReasonDesc(eReason), GetPhaseDesc(ePhase), fCantDeny==TRUE ? L"TRUE" : L"FALSE", GetErrorName(hr));
	return hr;
}
        
STDMETHODIMP CImpIRowsetNotify::OnRowChange( 
            /* [in] */ IRowset __RPC_FAR *pRowset,
            /* [in] */ DBCOUNTITEM cRows,
            /* [size_is][in] */ const HROW __RPC_FAR rghRows[  ],
            /* [in] */ DBREASON eReason,
            /* [in] */ DBEVENTPHASE ePhase,
            /* [in] */ BOOL fCantDeny)
{
	//Accept / Veto the change, and do processing
	HRESULT hr = AcceptOrVeto(pRowset, eReason, ePhase, fCantDeny, cRows,	rghRows );

	//Display Notification is user is interested...
	PRVTRACE(L"    0x%08x::OnRowChange(0x%08x, %d, 0x%08x, %s, %s, %s) - %s\n", this, pRowset, cRows, rghRows, GetReasonDesc(eReason), GetPhaseDesc(ePhase), fCantDeny==TRUE ? L"TRUE" : L"FALSE", GetErrorName(hr));
	return hr;
}
        
STDMETHODIMP CImpIRowsetNotify::OnRowsetChange( 
            /* [in] */ IRowset __RPC_FAR *pRowset,
            /* [in] */ DBREASON eReason,
            /* [in] */ DBEVENTPHASE ePhase,
            /* [in] */ BOOL fCantDeny)
{
	//Accept / Veto the change, and do processing
	HRESULT hr = AcceptOrVeto(pRowset, eReason, ePhase, fCantDeny);

	//Display Notification is user is interested...
	PRVTRACE(L"    0x%08x::OnRowsetChange(0x%08x, %s, %s, %s) - %s\n", this, pRowset, GetReasonDesc(eReason), GetPhaseDesc(ePhase), fCantDeny==TRUE ? L"TRUE" : L"FALSE", GetErrorName(hr));
	return hr;
}
    

HRESULT CImpIRowsetNotify::AcceptOrVeto(
		IRowset*		pIRowset,
		DBREASON		eReason,		// @parm IN	| Reason for notification
		DBEVENTPHASE	ePhase, 		// @parm IN	| Phase of event
		BOOL            fCantDeny,
		DBCOUNTITEM     cRows,
		const HROW		rghRows[],
		DBORDINAL       cColumns,
		const DBORDINAL	rgColumns[] )
{
	CListener* pCListener = (CListener*)m_pCBase;
	
	//Delegate to handle all cases, except reentrantcy...
	HRESULT hr = pCListener->AcceptOrVeto(pIRowset, eReason, ePhase, fCantDeny);
	if(hr == S_OK)
	{
		//If testing reentrantcy, call the specificed method while inside a Listener
		REENTRANT_ROWSETFUNC pReEntrantFunc = NULL;
		CRowset* pCRowset = NULL;

		if(pCListener->GetReEntrantcy(eReason, ePhase, (void**)&pReEntrantFunc, (void**)&pCRowset))
		{
			//Turn off reentrantcy, so we don't have a infinte loop
			pCListener->SetReEntrantcy(eReason, ePhase, NULL, NULL);

			//Call the desired function, within the listener
			ASSERT(pReEntrantFunc);
			(pReEntrantFunc)(pCRowset, pIRowset, cRows, rghRows, cColumns, rgColumns);
		}
	}

	return hr;
}


///////////////////////////////////////////////////////////////////////////////
// Class CImpIDBAsynchNotify
// 
// Implementation of IDBAsynchNotify
///////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CImpIDBAsynchNotify::OnLowResource( 
            /* [in] */ DB_DWRESERVE dwReserved) 
{
	//Delegate
	HRESULT hr = AcceptOrVeto(DB_NULL_HCHAPTER, DBREASON_ONLOWRESOURCE, DBASYNCHOP_OPEN, 0, 0, DBASYNCHPHASE_COMPLETE, NULL);

	//Display Notification is user is interested...
	PRVTRACE(L"    0x%08x::OnLowResource(%d) - %s\n", this, dwReserved, GetErrorName(hr));
	return hr;
}
        
STDMETHODIMP CImpIDBAsynchNotify::OnProgress( 
            /* [in] */ HCHAPTER hChapter,
            /* [in] */ DBASYNCHOP ulOperation,
            /* [in] */ DBCOUNTITEM ulProgress,
            /* [in] */ DBCOUNTITEM ulProgressMax,
            /* [in] */ DBASYNCHPHASE ulAsynchPhase,
            /* [in] */ LPOLESTR pwszStatusText)
{
	//Delegate
	HRESULT hr = AcceptOrVeto(hChapter, DBREASON_ONPROGRESS, ulOperation, ulProgress, ulProgressMax, ulAsynchPhase, NULL);

	//Display Notification is user is interested...
	PRVTRACE(L"    0x%08x::OnProgress(0x%08x, %s, %d, %d, %s, %s) - %s\n", this, hChapter, ulOperation==DBASYNCHOP_OPEN ? L"DBASYNCHOP_OPEN" : L"(Unknown)", ulProgress, ulProgressMax, GetAsynchPhase(ulAsynchPhase), pwszStatusText, GetErrorName(hr));
	return hr;
}
        
STDMETHODIMP CImpIDBAsynchNotify::OnStop( 
            /* [in] */ HCHAPTER hChapter,
            /* [in] */ DBASYNCHOP ulOperation,
            /* [in] */ HRESULT hrStatus,
            /* [in] */ LPOLESTR pwszStatusText)
{
	HRESULT hr = AcceptOrVeto(hChapter, DBREASON_ONSTOP, ulOperation, 0, 0, DBASYNCHPHASE_COMPLETE, NULL);

	//Display Notification is user is interested...
	PRVTRACE(L"    0x%08x::OnStop(0x%08x, %s, %s, %s) - %s\n", this, hChapter, ulOperation==DBASYNCHOP_OPEN ? L"DBASYNCHOP_OPEN" : L"(Unknown)", GetErrorName(hrStatus), pwszStatusText, GetErrorName(hr));
	return hr;
}


HRESULT CImpIDBAsynchNotify::AcceptOrVeto
(
			HCHAPTER		hChapter,
			DBASYNCHREASON	eReason,
			DBASYNCHOP		ulOperation,
			DBCOUNTITEM		ulProgress,            
			DBCOUNTITEM		ulProgressMax,            
			DBASYNCHPHASE	ePhase,
            LPOLESTR		pwszStatusText
)
{
	CListener* pCListener = (CListener*)m_pCBase;
	
	//Delegate to handle all cases, except reentrantcy...
	HRESULT hr = pCListener->AcceptOrVeto(NULL, eReason, ePhase, TRUE/*fCantDeny*/);
	if(hr == S_OK)
	{
	}

	return hr;
}


///////////////////////////////////////////////////////////////////////////////
// Class CImpIRowPositionChange
// 
// Implementation of IRowPositionChange
///////////////////////////////////////////////////////////////////////////////
HRESULT CImpIRowPositionChange::OnRowPositionChange(
	DBREASON		eReason,
	DBEVENTPHASE	ePhase,
	BOOL			fCantDeny)
{
	CListener* pCListener = (CListener*)m_pCBase;

	//Delegate to handle all cases, except reentrantcy...
	HRESULT hr = pCListener->AcceptOrVeto(NULL, eReason, ePhase, fCantDeny);
	if(hr == S_OK)
	{
	}
	
	//Display Notification is user is interested...
	PRVTRACE(L"    0x%08x::OnRowPositionChange(%s, %s, %s) - %s\n", this, GetReasonDesc(eReason), GetPhaseDesc(ePhase), fCantDeny==TRUE ? L"TRUE" : L"FALSE", GetErrorName(hr));
	return hr;
}



WCHAR* GetReasonDesc(DBREASON eReason)
{
	static const WIDENAMEMAP rgReasons[] = 
	{
		VALUE_WCHAR(DBREASON_ROWSET_FETCHPOSITIONCHANGE),
		VALUE_WCHAR(DBREASON_ROWSET_CHANGED),
		VALUE_WCHAR(DBREASON_COLUMN_SET),
		VALUE_WCHAR(DBREASON_ROW_DELETE),
		VALUE_WCHAR(DBREASON_ROW_FIRSTCHANGE),
		VALUE_WCHAR(DBREASON_ROW_INSERT),
		VALUE_WCHAR(DBREASON_ROW_RESYNCH),
		VALUE_WCHAR(DBREASON_ROW_UNDOCHANGE),
		VALUE_WCHAR(DBREASON_ROW_UNDOINSERT),
		VALUE_WCHAR(DBREASON_ROW_UNDODELETE),
		VALUE_WCHAR(DBREASON_ROW_UPDATE),
		VALUE_WCHAR(DBREASON_ROWSET_RELEASE),
		VALUE_WCHAR(DBREASON_COLUMN_RECALCULATED),
		VALUE_WCHAR(DBREASON_ROW_ACTIVATE),
		VALUE_WCHAR(DBREASON_ROW_RELEASE),
    	
		VALUE_WCHAR(DBREASON_ROWPOSITION_CHANGED),
		VALUE_WCHAR(DBREASON_ROWPOSITION_CHAPTERCHANGED),
		VALUE_WCHAR(DBREASON_ROWPOSITION_CLEARED),
		VALUE_WCHAR(DBREASON_ROW_ASYNCHINSERT),
	};
	
	return GetMapName(eReason, NUMELEM(rgReasons), rgReasons);
}

WCHAR* GetPhaseDesc(DBEVENTPHASE ePhase)
{
	static const WIDENAMEMAP rgPhases[] = 
	{
		VALUE_WCHAR(DBEVENTPHASE_OKTODO),
		VALUE_WCHAR(DBEVENTPHASE_ABOUTTODO),
		VALUE_WCHAR(DBEVENTPHASE_SYNCHAFTER),
		VALUE_WCHAR(DBEVENTPHASE_DIDEVENT),
		VALUE_WCHAR(DBEVENTPHASE_FAILEDTODO),
	};
	
	return GetMapName(ePhase, NUMELEM(rgPhases), rgPhases);
}


WCHAR* GetAsynchPhase(DBASYNCHPHASE ePhase)
{
	const static WIDENAMEMAP rgAsynchPhases[] = 
	{
		VALUE_WCHAR(DBASYNCHPHASE_INITIALIZATION),
		VALUE_WCHAR(DBASYNCHPHASE_POPULATION),
		VALUE_WCHAR(DBASYNCHPHASE_COMPLETE),
		VALUE_WCHAR(DBASYNCHPHASE_CANCELED),
	};
	
	return GetMapName(ePhase, NUMELEM(rgAsynchPhases), rgAsynchPhases);
}
