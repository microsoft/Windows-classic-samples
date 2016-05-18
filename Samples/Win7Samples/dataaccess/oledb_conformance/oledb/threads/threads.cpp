//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright (C) 1995-2000 Microsoft Corporation
//
// @doc  
//
// @module Threads.cpp | This module tests the OLEDB threading support 
//

// disable warning: C4312 Conversion to bigger-size warning. 
// For example, "type cast": conversion from "int" to "int*_ptr64" of greater size. 
#pragma warning(disable: 4312)

//////////////////////////////////////////////////////////////////
// Includes
//
//////////////////////////////////////////////////////////////////
#include "MODStandard.hpp"		// Standard headers
#include "Threads.h"			// Threads header
#include "ExtraLib.h"


//////////////////////////////////////////////////////////////////
// Defines
//
//////////////////////////////////////////////////////////////////
#define MAX_ITERATIONS	7


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Module Values
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// {{ TCW_MODULE_GLOBALS
DECLARE_MODULE_CLSID = { 0x1e9af4d0, 0xca41, 0x11cf, { 0xbc, 0x5d, 0x00, 0xa0, 0xc9, 0x0d, 0x80, 0x7a }};
DECLARE_MODULE_NAME("Threads");
DECLARE_MODULE_OWNER("Microsoft");
DECLARE_MODULE_DESCRIP("Threads Testing for OLEDB");
DECLARE_MODULE_VERSION(835236726);
// TCW_WizardVersion(2)
// TCW_Automation(True)
// }} TCW_MODULE_GLOBALS_END


//////////////////////////////////////////////////////////////////
// VerifyThreadingModel
//
//////////////////////////////////////////////////////////////////
BOOL VerifyThreadingModel()
{
	return VerifyThreadingModel(PROVIDER_CLSID, L"Free", GetModInfo()->GetClassContext()) || VerifyThreadingModel(PROVIDER_CLSID, L"Both", GetModInfo()->GetClassContext());
}


//--------------------------------------------------------------------
// @func Module level initialization routine
//
// @rdesc Success or Failure
// 		@flag  TRUE  | Successful initialization
//		@flag  FALSE | Initialization problems
//
BOOL ModuleInit(CThisTestModule * pThisTestModule)
{	
	//CommonInit
	//Need to actually call this first, since it sets up error objects...
    BOOL bReturn = CommonModuleInit(pThisTestModule, IID_IRowset, TEN_ROWS);

	//This Threading Test assumes FreeThreaded or Both
	//It is designed to pass pointers freely between threads, it will NOT
	//work in apartment model only environments...
	if(bReturn && !VerifyThreadingModel())
	{
		TOUTPUT("This test is designed for FreeThreaded providers, ThreadingModel=\"Free\" or \"Both\"");
		return TEST_SKIPPED;
	}

	return bReturn;
}	
  

//--------------------------------------------------------------------
// @func Module level termination routine
//
// @rdesc Success or Failure
// 		@flag  TRUE  | Successful initialization
//		@flag  FALSE | Initialization problems
//
BOOL ModuleTerminate(CThisTestModule * pThisTestModule)
{
	return CommonModuleTerminate(pThisTestModule);
}	


//////////////////////////////////////////////////////////////////
// Class CThreads
//
// Used to create multiple instantes of CTestCases even though our
// privlib is really not designed for it, quite a useful hack...
//////////////////////////////////////////////////////////////////

class CThreads : public CRowset
{
public:
	//Constructors
	CThreads(WCHAR* pwszTestCaseName = INVALID(WCHAR*));
	virtual ~CThreads();

    //methods

	//IUnknown
	static ULONG WINAPI Thread_QI(LPVOID pv);
	static ULONG WINAPI Thread_AddRef(LPVOID pv);
	static ULONG WINAPI Thread_Release(LPVOID pv);

	//DataSource (IDBProperties / IDBInitialize)
	static ULONG WINAPI Thread_IDBInitialize(LPVOID pv);
	static ULONG WINAPI Thread_IDBUninitialize(LPVOID pv);
	static ULONG WINAPI Thread_NewDSOInitialize(LPVOID pv);

	static ULONG WINAPI Thread_IDBGetPropertyInfo(LPVOID pv);
	static ULONG WINAPI Thread_IDBGetProperties(LPVOID pv);
	static ULONG WINAPI Thread_IDBSetProperties(LPVOID pv);


	//ICommand
	static ULONG WINAPI Thread_CommandCancel(LPVOID pv);
	static ULONG WINAPI Thread_CommandExecute(LPVOID pv);

	//ICommandPrepare
	static ULONG WINAPI Thread_CommandPrepare(LPVOID pv);
	static ULONG WINAPI Thread_CommandUnPrepare(LPVOID pv);

	//ICommandProperties
	static ULONG WINAPI Thread_GetCommandProperties(LPVOID pv);
	static ULONG WINAPI Thread_SetCommandProperties(LPVOID pv);

	//ICommandText
	static ULONG WINAPI Thread_SetCommandText(LPVOID pv);
	static ULONG WINAPI Thread_GetCommandText(LPVOID pv);

	//ICommandWithParameters
	static ULONG WINAPI Thread_SetDefaultValues(LPVOID pv);
	static ULONG WINAPI Thread_GetDefaultValues(LPVOID pv);

	//IAccessor
	static ULONG WINAPI Thread_CreateAccessor(LPVOID pv);
	static ULONG WINAPI Thread_GetBindings(LPVOID pv);
	static ULONG WINAPI Thread_ReleaseAccessor(LPVOID pv);

	//IColumnsInfo
	static ULONG WINAPI Thread_GetColumnsInfo(LPVOID pv);

	//IErrorRecords
	static ULONG WINAPI Thread_AddErrorRecord(LPVOID pv);
	static ULONG WINAPI Thread_GetErrorInfo(LPVOID pv);

    //Session (IDBCreateCommand / IDBCreateSession / ITableDefinition)
	static ULONG WINAPI Thread_IDBCreateCommand(LPVOID pv);
	static ULONG WINAPI Thread_IDBCreateSession(LPVOID pv);

	static ULONG WINAPI Thread_IDBCreateTable(LPVOID pv);
	static ULONG WINAPI Thread_AddColumn(LPVOID pv);
	static ULONG WINAPI Thread_DropColumn(LPVOID pv);

	//Schema
	static ULONG WINAPI Thread_GetSchemaRowset(LPVOID pv);
	static ULONG WINAPI Thread_GetSchemaInfo(LPVOID pv);
	
	//Class Factory
	static ULONG WINAPI Thread_GetClassFactory(LPVOID pv);
	static ULONG WINAPI Thread_CreateInstance(LPVOID pv);

	//IPersist
	static ULONG WINAPI Thread_PersistFileSave(LPVOID pv);
	static ULONG WINAPI Thread_PersistFileLoad(LPVOID pv);

	//IConnectionPoint
	static ULONG WINAPI Thread_CreateListener(LPVOID pv);

	//ITransaction
	static ULONG WINAPI Thread_StartTransaction(LPVOID pv);
	static ULONG WINAPI Thread_Commit(LPVOID pv);
	static ULONG WINAPI Thread_Abort(LPVOID pv);

	//IRowset
	static ULONG WINAPI Thread_AddRefRows(LPVOID pv);
	static ULONG WINAPI Thread_ReleaseRows(LPVOID pv);
	static ULONG WINAPI Thread_GetNextRows(LPVOID pv);
	static ULONG WINAPI Thread_GetData(LPVOID pv);
	static ULONG WINAPI Thread_RestartPosition(LPVOID pv);

	//IOpenRowset
	static ULONG WINAPI Thread_OpenRowset(LPVOID pv);

	//IRowsetChange
	static ULONG WINAPI Thread_SetData(LPVOID pv);
	static ULONG WINAPI Thread_DeleteRows(LPVOID pv);
	static ULONG WINAPI Thread_InsertRow(LPVOID pv);

	//IRowsetInfo
	static ULONG WINAPI Thread_GetRowsetProperties(LPVOID pv);
	static ULONG WINAPI Thread_GetSpecification(LPVOID pv);

	//IRowsetWithparameters
	static ULONG WINAPI Thread_Requery(LPVOID pv);

	//IRowsetResynch
	static ULONG WINAPI Thread_ResynchRows(LPVOID pv);
	static ULONG WINAPI Thread_GetVisibleData(LPVOID pv);

	//IRowsetLocate
	static ULONG WINAPI Thread_GetRowsAt(LPVOID pv);
	static ULONG WINAPI Thread_GetRowsByBookmark(LPVOID pv);

	//IRowsetUpdate
	static ULONG WINAPI Thread_GetOriginalData(LPVOID pv);
	static ULONG WINAPI Thread_GetPendingRows(LPVOID pv);
	static ULONG WINAPI Thread_GetRowStatus(LPVOID pv);
	static ULONG WINAPI Thread_Undo(LPVOID pv);
	static ULONG WINAPI Thread_Update(LPVOID pv);

	//IMultipleResults
	static ULONG WINAPI Thread_GetResult(LPVOID pv);

protected:
    //data 
    
private:
    //data pointers
};


CThreads::CThreads(WCHAR* pwszTestCaseName) : CRowset(pwszTestCaseName) 
{
	//private
}

CThreads::~CThreads() 
{
}


///////////////////////////////////////////////////////////
// Thread routines - IUnknown
//
///////////////////////////////////////////////////////////

ULONG CThreads::Thread_QI(LPVOID pv)
{
	THREAD_BEGIN

	//Thread Stack Variables
	IUnknown* pIUnknown = (IUnknown*)THREAD_FUNC;
	IID*      priid     = (IID*)THREAD_ARG1;
	ASSERT(pIUnknown && priid);
	HRESULT hr = S_OK;

	ThreadSwitch(); //Let the other threads Catch up

	for(ULONG i=0; i<MAX_ITERATIONS; i++)
	{
		TESTC_(hr = QI(pIUnknown,*priid),S_OK)
	}
	
	ThreadSwitch(); //Let the other thread(s) catch up

CLEANUP:
	THREAD_END(hr);
}

ULONG CThreads::Thread_AddRef(LPVOID pv)
{
	THREAD_BEGIN
	ULONG ulRefCount = 0;

	//Thread Stack Variables
	IUnknown* pThis     = (IUnknown*)THREAD_FUNC;
	ASSERT(pThis);

	ThreadSwitch(); //Let the other thread(s) catch up
	ulRefCount = pThis->AddRef();
	ThreadSwitch(); //Let the other thread(s) catch up

	THREAD_END(ulRefCount);
}

ULONG CThreads::Thread_Release(LPVOID pv)
{
	THREAD_BEGIN
	ULONG ulRefCount = 0;

	//Thread Stack Variables
	IUnknown* pThis     = (IUnknown*)THREAD_FUNC;
	ASSERT(pThis);
	
	ThreadSwitch(); //Let the other threads Catch up
	ulRefCount = pThis->Release();
	ThreadSwitch(); //Let the other thread(s) catch up

	THREAD_END(ulRefCount);
}

///////////////////////////////////////////////////////////
// Thread routines - IDBProperties
//
///////////////////////////////////////////////////////////
ULONG CThreads::Thread_IDBGetPropertyInfo(LPVOID pv)
{
	THREAD_BEGIN
	HRESULT hr = S_OK;

	//Thread Stack Variables
	IDBProperties*  pIDBProperties	= (IDBProperties*)THREAD_FUNC;
	ULONG			cPropIDSets		= (ULONG)THREAD_ARG1;
	DBPROPIDSET*	rgPropIDSets	= (DBPROPIDSET*)THREAD_ARG2;
	ASSERT(pIDBProperties);

	//Local Variables
	ULONG cPropInfoSets = 0;
	DBPROPINFOSET* rgPropInfoSets = NULL;
	WCHAR* pwszStringBuffer = NULL;
	 
	ThreadSwitch(); //Let the other thread(s) catch up
		
	for(ULONG i=0; i<MAX_ITERATIONS; i++)
	{
		//GetPropertyInfo
		TESTC_(hr = pIDBProperties->GetPropertyInfo(cPropIDSets, rgPropIDSets, &cPropInfoSets, &rgPropInfoSets, &pwszStringBuffer),S_OK)
		TESTC(cPropInfoSets!=0 && rgPropInfoSets!=NULL && pwszStringBuffer!=NULL);
		::FreeProperties(&cPropInfoSets, &rgPropInfoSets, &pwszStringBuffer);
	}

	ThreadSwitch(); //Let the other thread(s) catch up

CLEANUP:
	::FreeProperties(&cPropInfoSets, &rgPropInfoSets, &pwszStringBuffer);
	THREAD_END(hr);
}


///////////////////////////////////////////////////////////
// Thread routines - IDBProperties
//
///////////////////////////////////////////////////////////
ULONG CThreads::Thread_IDBGetProperties(LPVOID pv)
{
	THREAD_BEGIN
	HRESULT hr = S_OK;

	//Thread Stack Variables
	IDBProperties*  pIDBProperties	= (IDBProperties*)THREAD_FUNC;
	ULONG			cPropIDSets		= (ULONG)THREAD_ARG1;
	DBPROPIDSET*	rgPropIDSets	= (DBPROPIDSET*)THREAD_ARG2;
	ASSERT(pIDBProperties);

	//Local Variables
	ULONG cPropSets = 0;
	DBPROPSET* rgPropSets = NULL;
	 
	ThreadSwitch(); //Let the other thread(s) catch up
		
	for(ULONG i=0; i<MAX_ITERATIONS; i++)
	{
		//GetPropertyInfo
		TESTC_(hr = pIDBProperties->GetProperties(cPropIDSets, rgPropIDSets, &cPropSets, &rgPropSets),S_OK)
		TESTC(rgPropSets!=NULL)
		::FreeProperties(&cPropSets,&rgPropSets);
	}

	ThreadSwitch(); //Let the other thread(s) catch up

CLEANUP:
	::FreeProperties(&cPropSets,&rgPropSets);
	THREAD_END(hr);
}



///////////////////////////////////////////////////////////
// Thread routines - IDBProperties
//
///////////////////////////////////////////////////////////
ULONG CThreads::Thread_IDBSetProperties(LPVOID pv)
{
	THREAD_BEGIN
	HRESULT hr = S_OK;

	//Thread Stack Variables
	IDBProperties*  pIDBProperties	= (IDBProperties*)THREAD_FUNC;
	ULONG			cPropSets		= (ULONG)THREAD_ARG1;
	DBPROPSET*		rgPropSets		= (DBPROPSET*)THREAD_ARG2;
	ASSERT(pIDBProperties);

	ThreadSwitch(); //Let the other thread(s) catch up
		
	for(ULONG i=0; i<MAX_ITERATIONS; i++)
	{
		//GetPropertyInfo
		TESTC_(hr = pIDBProperties->SetProperties(cPropSets, rgPropSets),S_OK)
	}

	ThreadSwitch(); //Let the other thread(s) catch up

CLEANUP:
	THREAD_END(hr);
}


///////////////////////////////////////////////////////////
// Thread routines - IDBInitialize
//
///////////////////////////////////////////////////////////
ULONG CThreads::Thread_IDBInitialize(LPVOID pv)
{
	THREAD_BEGIN
	HRESULT hr = S_OK;

	//Thread Stack Variables
	IDBInitialize*  pIDBInitialize	= (IDBInitialize*)THREAD_FUNC;
	HRESULT			hrExpected		= (HRESULT)THREAD_ARG1;
	ASSERT(pIDBInitialize);
	
	ThreadSwitch(); //Let the other thread(s) catch up

	for(ULONG i=0; i<MAX_ITERATIONS; i++)
	{
		//IDBInitialize
		//May already be initialized from another thread
		TEST2C_(hr = pIDBInitialize->Initialize(), S_OK, hrExpected);
	}
	
	ThreadSwitch(); //Let the other thread(s) catch up

CLEANUP:
	THREAD_END(hr);
}

ULONG CThreads::Thread_IDBUninitialize(LPVOID pv)
{
	THREAD_BEGIN
	HRESULT hr = S_OK;

	//Thread Stack Variables
	IDBInitialize*  pIDBInitialize	= (IDBInitialize*)THREAD_FUNC;
	HRESULT			hrExpected		= (HRESULT)THREAD_ARG1;
	ASSERT(pIDBInitialize);

	ThreadSwitch(); //Let the other thread(s) catch up
		
	for(ULONG i=0; i<MAX_ITERATIONS; i++)
	{
		//IDBInitialize::Unitialize
		TEST2C_(hr = pIDBInitialize->Uninitialize(), S_OK, hrExpected);
	}
	
	ThreadSwitch(); //Let the other thread(s) catch up
	
CLEANUP:
	THREAD_END(hr);
}


///////////////////////////////////////////////////////////
// Thread routines - IDBInitialize
//
///////////////////////////////////////////////////////////
ULONG CThreads::Thread_NewDSOInitialize(LPVOID pv)
{
	THREAD_BEGIN
	HRESULT hr = S_OK;

	//Thread Stack Variables
	IDBInitialize*  pIDBInitialize = NULL;
	IDBProperties*  pIDBProperties = NULL;
	CThreads*		pThis		 = (CThreads*)THREAD_FUNC;
	ULONG			cPropSets	 = (ULONG)THREAD_ARG1;
	DBPROPSET*		rgPropSets	 = (DBPROPSET*)THREAD_ARG2;
	HRESULT			hrExpected	 = (HRESULT)THREAD_ARG3;
	HRESULT			hrExpected2	 = (HRESULT)THREAD_ARG4;
	ASSERT(pThis == NULL);
	
	ThreadSwitch(); //Let the other thread(s) catch up

	//Create the new DSO
	TESTC_(CreateNewDSO(NULL, IID_IDBInitialize,(IUnknown**)&pIDBInitialize, CREATEDSO_NONE),S_OK);

	//SetProperties
	TESTC_(QI(pIDBInitialize, IID_IDBProperties, (void**)&pIDBProperties),S_OK);
	TESTC_(pIDBProperties->SetProperties(cPropSets, rgPropSets),S_OK);
	
	//IDBInitialize
	TEST3C_(hr = pIDBInitialize->Initialize(), S_OK, hrExpected, hrExpected2);
	ThreadSwitch(); //Let the other thread(s) catch up

CLEANUP:
	SAFE_RELEASE(pIDBInitialize);
	SAFE_RELEASE(pIDBProperties);
	THREAD_END(hr);
}


///////////////////////////////////////////////////////////
// Thread routines - ICommand
//
///////////////////////////////////////////////////////////
ULONG CThreads::Thread_CommandCancel(LPVOID pv)
{
	THREAD_BEGIN
	HRESULT hr;

	//Thread Stack Variables
	CRowset*	pThis		= (CRowset*)THREAD_FUNC;
	HRESULT		hrExpected	= (HRESULT)THREAD_ARG1;
	ASSERT(pThis);

	//Local Variables

	ThreadSwitch(); //Let the other thread(s) catch up
	if(pThis->GetCommandSupport())
	{
		for(ULONG i=0; i<MAX_ITERATIONS; i++)
		{
			TEST2C_(hr = pThis->pICommand()->Cancel(), S_OK, hrExpected);
		}
	}

	ThreadSwitch(); //Let the other thread(s) catch up

CLEANUP:
	THREAD_END(hr);
}

ULONG CThreads::Thread_CommandExecute(LPVOID pv)
{
	THREAD_BEGIN
	HRESULT hr;

	//Thread Stack Variables
	CRowset*	pThis		= (CRowset*)THREAD_FUNC;
	HRESULT		hrExpected	= (HRESULT)THREAD_ARG1;
	ASSERT(pThis);

	//Local Variables
	IRowset* pIRowset = NULL;

	ThreadSwitch(); //Let the other thread(s) catch up
	
	if(pThis->GetCommandSupport())
	{
		for(ULONG i=0; i<MAX_ITERATIONS; i++)
		{
			hr = pThis->pICommand()->Execute(NULL,IID_IRowset,NULL,NULL,(IUnknown**)&pIRowset);
			TEST2C_(hr, S_OK, hrExpected);
	
			//Verify Result...
			TESTC(VerifyOutputInterface(hr, IID_IRowset, (IUnknown**)&pIRowset));
			SAFE_RELEASE(pIRowset)
		}
	}

	ThreadSwitch(); //Let the other thread(s) catch up

CLEANUP:
	SAFE_RELEASE(pIRowset)
	THREAD_END(hr);
}

///////////////////////////////////////////////////////////
// Thread routines - ICommandPrepare
//
///////////////////////////////////////////////////////////

ULONG CThreads::Thread_CommandPrepare(LPVOID pv)
{
	THREAD_BEGIN
	HRESULT hr = S_OK;

	//Thread Stack Variables
	CRowset*  pThis  = (CRowset*)THREAD_FUNC;
	ASSERT(pThis);

	//Local Variables
	ICommandPrepare* pICommandPrepare = NULL;
	ThreadSwitch(); //Let the other thread(s) catch up

	if(pThis->GetCommandSupport())
	{
		if(hr = QI(pThis->m_pICommand, IID_ICommandPrepare, (void**)&pICommandPrepare)==S_OK)
		{
			for(ULONG i=0; i<MAX_ITERATIONS; i++)
			{
				TESTC_(hr = pICommandPrepare->Prepare(1),S_OK)
			}
		}
	}
	ThreadSwitch(); //Let the other thread(s) catch up

CLEANUP:
	SAFE_RELEASE(pICommandPrepare);
	THREAD_END(hr);
}

ULONG CThreads::Thread_CommandUnPrepare(LPVOID pv)
{
	THREAD_BEGIN					   
	HRESULT hr;

	//Thread Stack Variables
	CRowset*	pThis		= (CRowset*)THREAD_FUNC;
	HRESULT		hrExpected	= (HRESULT)THREAD_ARG1;
	ASSERT(pThis);

	//Local Variables
	ICommandPrepare* pICommandPrepare = NULL;
	ThreadSwitch(); //Let the other thread(s) catch up

	if(pThis->GetCommandSupport())
	{
		if(QI(pThis->m_pICommand,IID_ICommandPrepare,(void**)&pICommandPrepare)==S_OK)
		{
			for(ULONG i=0; i<MAX_ITERATIONS; i++)
			{
				//Another thread may have already executed a rowset
				hr = pICommandPrepare->Unprepare();
				TEST2C_(hr, S_OK, hrExpected);
			}
		}
	}	

	ThreadSwitch(); //Let the other thread(s) catch up

CLEANUP:
	SAFE_RELEASE(pICommandPrepare);
	THREAD_END(hr);
}


///////////////////////////////////////////////////////////
// Thread routines - ICommandProperties
//
///////////////////////////////////////////////////////////

ULONG CThreads::Thread_GetCommandProperties(LPVOID pv)
{
	THREAD_BEGIN
	HRESULT hr = S_OK;

	//Thread Stack Variables
	CRowset*		pThis			= (CRowset*)THREAD_FUNC;
	ULONG			cPropIDSets		= (ULONG)THREAD_ARG1;
	DBPROPIDSET*	rgPropIDSets	= (DBPROPIDSET*)THREAD_ARG2;
	ASSERT(pThis);

	//Local Variables
	ICommandProperties* pICommandProperties = NULL;
	ULONG cPropSets = 0;
	DBPROPSET* rgPropSets = NULL;

	ThreadSwitch(); //Let the other thread(s) catch up

	if(pThis->GetCommandSupport())
	{
		TESTC_(hr = QI(pThis->m_pICommand,IID_ICommandProperties,(void**)&pICommandProperties),S_OK)

		for(ULONG i=0; i<MAX_ITERATIONS; i++)
		{
			TESTC_(hr = pICommandProperties->GetProperties(cPropIDSets, rgPropIDSets, &cPropSets, &rgPropSets),S_OK)
			if(cPropIDSets)
				TESTC(rgPropSets!=NULL && rgPropSets[0].cProperties==cPropIDSets);
			::FreeProperties(&cPropSets,&rgPropSets);
		}
	}
	
	ThreadSwitch(); //Let the other thread(s) catch up
	
CLEANUP:
	SAFE_RELEASE(pICommandProperties);
	::FreeProperties(&cPropSets,&rgPropSets);
	THREAD_END(hr);
}

ULONG CThreads::Thread_SetCommandProperties(LPVOID pv)
{
	THREAD_BEGIN
	HRESULT hr = S_OK;

	//Thread Stack Variables
	CRowset*    pThis      = (CRowset*)THREAD_FUNC;
	ULONG       cPropSets  = (ULONG)THREAD_ARG1;
	DBPROPSET*  rgPropSets  = (DBPROPSET*)THREAD_ARG2;
	ASSERT(pThis);

	//Local Variables
	ICommandProperties* pICommandProperties = NULL;
	ThreadSwitch(); //Let the other thread(s) catch up

	if(pThis->GetCommandSupport())
	{
		TESTC_(hr = QI(pThis->m_pICommand,IID_ICommandProperties,(void**)&pICommandProperties),S_OK)
		for(ULONG i=0; i<MAX_ITERATIONS; i++)
		{
			TESTC_(hr = pICommandProperties->SetProperties(cPropSets, rgPropSets),S_OK)
		}
	}
	ThreadSwitch(); //Let the other thread(s) catch up

CLEANUP:
	SAFE_RELEASE(pICommandProperties);
	THREAD_END(hr);
}

///////////////////////////////////////////////////////////
// Thread routines - ICommandText
//
///////////////////////////////////////////////////////////

ULONG CThreads::Thread_SetCommandText(LPVOID pv)
{
	THREAD_BEGIN
	HRESULT hr = S_OK;

	//Thread Stack Variables
	CRowset* pThis = (CRowset*)THREAD_FUNC;
	WCHAR* pwszCommandText = (WCHAR*)THREAD_ARG1;
	ASSERT(pThis);

	//Local Variables
	ICommandText* pICommandText = NULL;
	ThreadSwitch(); //Let the other thread(s) catch up

	if(pThis->GetCommandSupport())
	{
		TESTC_(hr = QI(pThis->pICommand(),IID_ICommandText,(void**)&pICommandText),S_OK)
		for(ULONG i=0; i<MAX_ITERATIONS; i++)
		{
			TESTC_(hr = pICommandText->SetCommandText(DBGUID_DEFAULT, pwszCommandText),S_OK)
		}
	}
	
	ThreadSwitch(); //Let the other thread(s) catch up

CLEANUP:
	SAFE_RELEASE(pICommandText);
	THREAD_END(hr);
}

ULONG CThreads::Thread_GetCommandText(LPVOID pv)
{
	THREAD_BEGIN
	HRESULT hr = S_OK;

	//Thread Stack Variables
	CRowset* pThis = (CRowset*)THREAD_FUNC;
	ASSERT(pThis);

	//Local Variables
	WCHAR* pwszCommandText = NULL;
	ICommandText* pICommandText = NULL;
	ThreadSwitch(); //Let the other thread(s) catch up

	if(pThis->GetCommandSupport())
	{
		TESTC_(hr = QI(pThis->pICommand(),IID_ICommandText,(void**)&pICommandText),S_OK)
		for(ULONG i=0; i<MAX_ITERATIONS; i++)
		{
			TESTC_(hr = pICommandText->GetCommandText(NULL,&pwszCommandText),S_OK)
			PROVIDER_FREE(pwszCommandText);
		}
	}
	ThreadSwitch(); //Let the other thread(s) catch up

CLEANUP:
	SAFE_RELEASE(pICommandText);
	THREAD_END(hr);
}


///////////////////////////////////////////////////////////
// Thread routines - ICommandWithParameters
//
///////////////////////////////////////////////////////////

ULONG CThreads::Thread_SetDefaultValues(LPVOID pv)
{
	THREAD_BEGIN
	HRESULT hr = S_OK;

	//Thread Stack Variables
	CRowset* pThis = (CRowset*)THREAD_FUNC;
	ASSERT(pThis);

	//Local Variables

	ThreadSwitch(); //Let the other thread(s) catch up
	//TODO V2
	ThreadSwitch(); //Let the other thread(s) catch up

	THREAD_END(hr);
}

ULONG CThreads::Thread_GetDefaultValues(LPVOID pv)
{
	THREAD_BEGIN
	HRESULT hr = S_OK;

	//Thread Stack Variables
	CRowset* pThis = (CRowset*)THREAD_FUNC;
	ASSERT(pThis);

	//Local Variables

	ThreadSwitch(); //Let the other thread(s) catch up
	//TODO V2
	ThreadSwitch(); //Let the other thread(s) catch up

	THREAD_END(hr);
}


///////////////////////////////////////////////////////////
// Thread routines - IAccessor
//
///////////////////////////////////////////////////////////

ULONG CThreads::Thread_CreateAccessor(LPVOID pv)
{
	THREAD_BEGIN
	HRESULT hr = S_OK;

	//Thread Stack Variables
	CRowset* pThis = (CRowset*)THREAD_FUNC;
	ASSERT(pThis);

	DBORDINAL cBindings = pThis->m_cBindings;
	DBBINDING* rgBinding = pThis->m_rgBinding;
	DBLENGTH cbRowSize = pThis->m_cRowSize;
	HACCESSOR rghAccessors[MAX_ITERATIONS];

	ThreadSwitch(); //Let the other thread(s) catch up
	ULONG i;
	for(ULONG i=0; i<MAX_ITERATIONS; i++)
	{
		TESTC_(hr = pThis->pIAccessor()->CreateAccessor(DBACCESSOR_ROWDATA,cBindings,rgBinding,cbRowSize,&rghAccessors[i],NULL),S_OK)
	}
	ThreadSwitch(); //Let the other thread(s) catch up

CLEANUP:
	for(i=0; i<MAX_ITERATIONS; i++)
	{
		pThis->pIAccessor()->ReleaseAccessor(rghAccessors[i], NULL);
	}
	THREAD_END(hr);
}

ULONG CThreads::Thread_GetBindings(LPVOID pv)
{
	THREAD_BEGIN

	//Thread Stack Variables
	CRowset* pThis			= (CRowset*)THREAD_FUNC;
	HACCESSOR* phAccessor	= (HACCESSOR*)THREAD_ARG1;
	HRESULT hrExpected		= (HRESULT)THREAD_ARG2;
	ASSERT(pThis && phAccessor);

	//Local Variables
	DBACCESSORFLAGS dwAccessorFlags = 0;
	DBORDINAL cBindings = 0;
	DBBINDING* rgBinding = NULL;
	HRESULT hr = S_OK;

	ThreadSwitch(); //Let the other thread(s) catch up
	for(ULONG i=0; i<MAX_ITERATIONS; i++)
	{
		hr = pThis->pIAccessor()->GetBindings(*phAccessor,&dwAccessorFlags,&cBindings,&rgBinding);
		TEST2C_(hr, S_OK, hrExpected);
		PROVIDER_FREE(rgBinding);
	}
	
	ThreadSwitch(); //Let the other thread(s) catch up

CLEANUP:
	PROVIDER_FREE(rgBinding);
	THREAD_END(hr);
}

ULONG CThreads::Thread_ReleaseAccessor(LPVOID pv)
{
	THREAD_BEGIN
	HRESULT hr = S_OK;

	//Thread Stack Variables
	CRowset* pThis = (CRowset*)THREAD_FUNC;
	HACCESSOR* phAccessor = (HACCESSOR*)THREAD_ARG1;
	ASSERT(pThis && phAccessor);

	//Local Variables

	ThreadSwitch(); //Let the other thread(s) catch up
	TESTC_(hr = pThis->pIAccessor()->ReleaseAccessor(*phAccessor,NULL),S_OK)
	ThreadSwitch(); //Let the other thread(s) catch up

CLEANUP:
	THREAD_END(hr);
}


///////////////////////////////////////////////////////////
// Thread routines - IColumnsInfo
//
///////////////////////////////////////////////////////////

ULONG CThreads::Thread_GetColumnsInfo(LPVOID pv)
{
	THREAD_BEGIN
	HRESULT hr = S_OK;

	//Thread Stack Variables
	CRowset* pThis = (CRowset*)THREAD_FUNC;
	ASSERT(pThis);

	//Local Variables
	DBORDINAL i,cColumns = 0;
	DBCOLUMNINFO* rgInfo = NULL;
	WCHAR* pwszStringsBuffer = NULL;
	
	IColumnsInfo* pIColInfo = NULL;
	TESTC_(hr = QI(pThis->pIRowset(),IID_IColumnsInfo,(void**)&pIColInfo),S_OK)
	
	ThreadSwitch(); //Let the other threads Catch up
	for(i=0; i<MAX_ITERATIONS; i++)
	{
		TESTC_(hr = pIColInfo->GetColumnInfo(&cColumns,&rgInfo,&pwszStringsBuffer),S_OK)
		PROVIDER_FREE(rgInfo);  
		PROVIDER_FREE(pwszStringsBuffer);
	}
	ThreadSwitch(); //Let the other threads Catch up

CLEANUP:
	PROVIDER_FREE(rgInfo);  
	PROVIDER_FREE(pwszStringsBuffer);
	SAFE_RELEASE(pIColInfo);
	THREAD_END(hr);
}


///////////////////////////////////////////////////////////
// Thread routines - IErrorRecords
//
///////////////////////////////////////////////////////////

ULONG CThreads::Thread_AddErrorRecord(LPVOID pv)
{
	THREAD_BEGIN
	HRESULT hr = S_OK;

	//Thread Stack Variables
	CRowset* pThis = (CRowset*)THREAD_FUNC;
	ASSERT(pThis);

	IErrorInfo* pIErrorInfo = NULL;
	IErrorRecords* pIErrorRecords = NULL;
	ISupportErrorInfo* pISupportErrorInfo = NULL;

	ERRORINFO ErrorInfo[1];
	ErrorInfo[0].hrError = DB_E_DELETEDROW;
	ErrorInfo[0].dwMinor = 0;
	ErrorInfo[0].clsid   = CLSID_TESTMODULE;
	ErrorInfo[0].iid     = IID_IRowset;
	ErrorInfo[0].dispid  = DISPID_VALUE;

	//Cause an error to occur
	hr = QI(pThis->pIRowset(), IID_NULL, NULL);
	
	//Obtain the error object, from OLE Automation
	TESTC_PROVIDER(hr = QI(pThis->pIRowset(),IID_ISupportErrorInfo,(void**)&pISupportErrorInfo)==S_OK);
	TESTC_PROVIDER(hr = pISupportErrorInfo->InterfaceSupportsErrorInfo(IID_IRowset)==S_OK)
	
	QTESTC_(hr = GetErrorInfo(NULL,&pIErrorInfo),S_OK)
	TESTC_(hr = QI(pIErrorInfo,IID_IErrorRecords,(void**)&pIErrorRecords),S_OK)	

	//Local Variables
	ThreadSwitch(); //Let the other thread(s) catch up
		
	//Now after all the Junk, we can actually add an error record...
	hr = pIErrorRecords->AddErrorRecord(&ErrorInfo[0],IDENTIFIER_SDK_ERROR,NULL,NULL,0);
	ThreadSwitch(); //Let the other thread(s) catch up

CLEANUP:
	SAFE_RELEASE(pIErrorInfo);
	SAFE_RELEASE(pIErrorRecords);
	SAFE_RELEASE(pISupportErrorInfo);
	THREAD_END(hr);
}

ULONG CThreads::Thread_GetErrorInfo(LPVOID pv)
{
	THREAD_BEGIN
	HRESULT hr = S_OK;

	//Thread Stack Variables
	CRowset* pThis = (CRowset*)THREAD_FUNC;
	ASSERT(pThis);

	IErrorInfo* pIErrorInfo = NULL;
	IErrorInfo* pIOutErrorInfo = NULL;
	IErrorRecords* pIErrorRecords = NULL;
	ISupportErrorInfo* pISupportErrorInfo = NULL;

	//Cause an error to occur
	hr = QI(pThis->pIRowset(), IID_NULL, NULL);

	//Obtain the error object, from OLE Automation
	TESTC_PROVIDER(hr = QI(pThis->pIRowset(),IID_ISupportErrorInfo,(void**)&pISupportErrorInfo)==S_OK);
	TESTC_PROVIDER(hr = pISupportErrorInfo->InterfaceSupportsErrorInfo(IID_IRowset)==S_OK)
	
	QTESTC_(hr = GetErrorInfo(NULL,&pIErrorInfo),S_OK)
	TESTC_(hr = QI(pIErrorInfo,IID_IErrorRecords,(void**)&pIErrorRecords),S_OK)	

	//Local Variables
	ThreadSwitch(); //Let the other thread(s) catch up
		
	//Now after all the Junk, we can actually get the error info...
	hr = pIErrorRecords->GetErrorInfo(0,GetSystemDefaultLCID(),&pIOutErrorInfo);
	ThreadSwitch(); //Let the other thread(s) catch up

CLEANUP:
	SAFE_RELEASE(pIErrorInfo);
	SAFE_RELEASE(pIOutErrorInfo);
	SAFE_RELEASE(pIErrorRecords);
	SAFE_RELEASE(pISupportErrorInfo);
	THREAD_END(hr);
}

///////////////////////////////////////////////////////////
// Thread routines - IDBCreateCommand
//
///////////////////////////////////////////////////////////

ULONG CThreads::Thread_IDBCreateCommand(LPVOID pv)
{
	THREAD_BEGIN
	HRESULT hr = S_OK;

	//Thread Stack Variables
	CRowset* pThis = (CRowset*)THREAD_FUNC;
	ASSERT(pThis);

	//Local Variables
	IUnknown* pIUnknown = NULL;
	
	ThreadSwitch(); //Let the other thread(s) catch up

	//Provider might not support CommandObjects
	if(pThis->GetCommandSupport())
	{
		for(ULONG i=0; i<MAX_ITERATIONS; i++)
		{
			TESTC_(hr = pThis->m_pIDBCreateCommand->CreateCommand(NULL,IID_ICommand,&pIUnknown),S_OK)
			SAFE_RELEASE(pIUnknown);
		}
	}
	
	ThreadSwitch(); //Let the other thread(s) catch up

CLEANUP:
	SAFE_RELEASE(pIUnknown);
	THREAD_END(hr);
}

///////////////////////////////////////////////////////////
// Thread routines - IDBCreateSession
//
///////////////////////////////////////////////////////////

ULONG CThreads::Thread_IDBCreateSession(LPVOID pv)
{
	THREAD_BEGIN
	HRESULT hr = S_OK;

	//Thread Stack Variables
	CRowset* pThis		= (CRowset*)THREAD_FUNC;
	HRESULT	 hrExpected = (HRESULT)THREAD_ARG1;
	ASSERT(pThis);

	//Local Variables
	IUnknown* pIUnknown = NULL;
	IDBCreateSession* pIDBCreateSession = NULL;
	ULONG i=0;
	
	TESTC_(hr = QI(pThis->m_pIDBInitialize,IID_IDBCreateSession,(void**)&pIDBCreateSession),S_OK)

	ThreadSwitch(); //Let the other thread(s) catch up
		
	for(i=0; i<MAX_ITERATIONS; i++)
	{
		TEST2C_(hr = pIDBCreateSession->CreateSession(NULL,IID_IOpenRowset,&pIUnknown),S_OK,hrExpected)
		SAFE_RELEASE(pIUnknown);
	}
	ThreadSwitch(); //Let the other thread(s) catch up

CLEANUP:
	SAFE_RELEASE(pIDBCreateSession);
	SAFE_RELEASE(pIUnknown);
	THREAD_END(hr);
}

///////////////////////////////////////////////////////////
// Thread routines - Session (IDBTableDefinition)
//
///////////////////////////////////////////////////////////
ULONG CThreads::Thread_IDBCreateTable(LPVOID pv)
{
	THREAD_BEGIN
	HRESULT hr = S_OK;

	//Thread Stack Variables
	CRowset* pThis = (CRowset*)THREAD_FUNC;
	ASSERT(pThis);

	//Local Variables
	ITableDefinition* pITableDefinition = NULL;
	CTable* pTable = pThis->m_pTable;

	//TODO V2, QI fails with E_NOINTERFACE
	TESTC_(hr = QI(pThis->pISession(),IID_ITableDefinition,(void**)&pITableDefinition),S_OK)
	 
	ThreadSwitch(); //Let the other thread(s) catch up
	//TESTC_(pITableDefinition->CreateTable(NULL, &pTable->GetTableID(), pTable->CountColumnsOnTable(), m_rgColumnDescs, IID_IRowset, NULL, NULL),S_OK)
	ThreadSwitch(); //Let the other thread(s) catch up

CLEANUP:
	SAFE_RELEASE(pITableDefinition);
	THREAD_END(hr);
}

ULONG CThreads::Thread_AddColumn(LPVOID pv)
{
	THREAD_BEGIN
	HRESULT hr = S_OK;

	//Thread Stack Variables
	CRowset* pThis = (CRowset*)THREAD_FUNC;
	ASSERT(pThis);

	//Local Variables
	ITableDefinition* pITableDefinition = NULL;
	CTable* pTable = pThis->m_pTable;

	//TODO V2, QI Above fails with E_NOINTERFACE
	TESTC_(hr = QI(pThis->pISession(),IID_ITableDefinition,(void**)&pITableDefinition),S_OK)

	ThreadSwitch(); //Let the other thread(s) catch up
	TESTC_(hr = pITableDefinition->AddColumn(&pTable->GetTableID(), NULL, NULL),S_OK)
	ThreadSwitch(); //Let the other thread(s) catch up
	
CLEANUP:
	SAFE_RELEASE(pITableDefinition);
	THREAD_END(hr);
}

ULONG CThreads::Thread_DropColumn(LPVOID pv)
{
	THREAD_BEGIN
	HRESULT hr = S_OK;

	//Thread Stack Variables
	CRowset* pThis = (CRowset*)THREAD_FUNC;
	ASSERT(pThis);

	//Local Variables
	ITableDefinition* pITableDefinition = NULL;
	CTable* pTable = pThis->m_pTable;
	
	//TODO V2, QI fails with E_NOINTERFACE
	TESTC_(hr = QI(pThis->pISession(),IID_ITableDefinition,(void**)&pITableDefinition),S_OK)

	ThreadSwitch(); //Let the other thread(s) catch up
	TESTC_(hr = pITableDefinition->AddColumn(&pTable->GetTableID(), NULL, NULL),S_OK)
	ThreadSwitch(); //Let the other thread(s) catch up
	
CLEANUP:
	SAFE_RELEASE(pITableDefinition);
	THREAD_END(hr);
}

///////////////////////////////////////////////////////////
// Thread routines - Schema
//
///////////////////////////////////////////////////////////

ULONG CThreads::Thread_GetSchemaRowset(LPVOID pv)
{
	THREAD_BEGIN
	HRESULT hr = S_OK;

	//Thread Stack Variables
	CRowset* pThis = (CRowset*)THREAD_FUNC;
	ASSERT(pThis);

	//Local Variables
	IDBSchemaRowset* pIDBSchemaRowset = NULL;
	IUnknown* pIUnknown = NULL;
	ULONG i=0;

	ThreadSwitch(); //Let the other thread(s) catch up
	
	//IDBSchemaRowset is optional
	TESTC_PROVIDER(hr = QI(pThis->pISession(),IID_IDBSchemaRowset,(void**)&pIDBSchemaRowset)==S_OK);
	for(i=0; i<MAX_ITERATIONS; i++)
	{
		//Manadatory Schema Rowset...
		TESTC_(hr = pIDBSchemaRowset->GetRowset(NULL,DBSCHEMA_PROVIDER_TYPES,0,NULL,IID_IRowset,0,NULL,&pIUnknown),S_OK);
		SAFE_RELEASE(pIUnknown);
	}
	ThreadSwitch(); //Let the other thread(s) catch up
	
CLEANUP:
	SAFE_RELEASE(pIDBSchemaRowset);
	SAFE_RELEASE(pIUnknown);
	THREAD_END(hr);
}

ULONG CThreads::Thread_GetSchemaInfo(LPVOID pv)
{
	THREAD_BEGIN
	HRESULT hr = S_OK;

	//Thread Stack Variables
	CRowset* pThis = (CRowset*)THREAD_FUNC;
	ASSERT(pThis);

	//Local Variables
	IDBSchemaRowset* pIDBSchemaRowset = NULL;
	ULONG i,cSchemas = 0;
	GUID* rgSchemas = NULL;
	ULONG* rgRestrictions = NULL;

	ThreadSwitch(); //Let the other thread(s) catch up
		
	//IDBSchemaRowset is optional
	TESTC_PROVIDER(hr = QI(pThis->pISession(),IID_IDBSchemaRowset,(void**)&pIDBSchemaRowset)==S_OK);
	for(i=0; i<MAX_ITERATIONS; i++)
	{
		TESTC_(hr = pIDBSchemaRowset->GetSchemas(&cSchemas,&rgSchemas,&rgRestrictions),S_OK)
		PROVIDER_FREE(rgSchemas);
		PROVIDER_FREE(rgRestrictions);
	}
	ThreadSwitch(); //Let the other thread(s) catch up

CLEANUP:
	SAFE_RELEASE(pIDBSchemaRowset);
	PROVIDER_FREE(rgSchemas);
	PROVIDER_FREE(rgRestrictions);
	THREAD_END(hr);
}
	
///////////////////////////////////////////////////////////
// Thread routines - ClassFactory
//
///////////////////////////////////////////////////////////
ULONG CThreads::Thread_GetClassFactory(LPVOID pv)
{
	THREAD_BEGIN
	HRESULT hr = S_OK;

	//Thread Stack Variables
	CRowset* pThis = (CRowset*)THREAD_FUNC;
	ASSERT(pThis);

	//Local Variables
	IClassFactory* pIClassFactory = NULL;
	
	ThreadSwitch(); //Let the other thread(s) catch up
	for(ULONG i=0; i<MAX_ITERATIONS; i++)
	{
		TESTC_(hr = CoGetClassObject(PROVIDER_CLSID,CLSCTX_INPROC_SERVER,NULL,IID_IClassFactory,(void**)&pIClassFactory), S_OK); 
		SAFE_RELEASE(pIClassFactory);
	}
	ThreadSwitch(); //Let the other thread(s) catch up

CLEANUP:
	SAFE_RELEASE(pIClassFactory);
	THREAD_END(hr);
}

ULONG CThreads::Thread_CreateInstance(LPVOID pv)
{
	THREAD_BEGIN
	HRESULT hr = S_OK;

	//Thread Stack Variables
	IClassFactory* pIClassFactory = (IClassFactory*)THREAD_FUNC;
	ASSERT(pIClassFactory);

	//Local Variables
	IDBInitialize* pIDBInitialize = NULL;

	ThreadSwitch(); //Let the other thread(s) catch up
	for(ULONG i=0; i<MAX_ITERATIONS; i++)
	{
		TESTC_(hr = pIClassFactory->CreateInstance(NULL,IID_IDBInitialize,(void**)&pIDBInitialize), S_OK); 
		SAFE_RELEASE(pIDBInitialize);
	}
	
	ThreadSwitch(); //Let the other thread(s) catch up

CLEANUP:
	SAFE_RELEASE(pIDBInitialize);
	THREAD_END(hr);
}


ULONG CThreads::Thread_PersistFileSave(LPVOID pv)
{
	THREAD_BEGIN
	HRESULT hr = NOERROR;

	//Thread Stack Variables
	IDBInitialize* pThis = (IDBInitialize*)THREAD_FUNC;
	ASSERT(pThis);

	//Local Variables
	IPersistFile* pIPersistFile = NULL; 

	hr=QI(pThis,IID_IPersistFile,(void**)&pIPersistFile);
	TEST2C_(hr, S_OK, E_NOINTERFACE);
	
	ThreadSwitch(); //Let the other thread(s) catch up
	
	if(pIPersistFile)
	{
		for(ULONG i=0; i<MAX_ITERATIONS; i++)
		{
			TESTC_(hr = pIPersistFile->Save(PERSIST_FILE, FALSE),S_OK)
		}
	}

	ThreadSwitch(); //Let the other thread(s) catch up
	
CLEANUP:
	SAFE_RELEASE(pIPersistFile);
	THREAD_END(hr);
}

ULONG CThreads::Thread_PersistFileLoad(LPVOID pv)
{
	THREAD_BEGIN
	HRESULT hr = NOERROR;

	//Thread Stack Variables
	IDBInitialize* pThis = (IDBInitialize*)THREAD_FUNC;
	ASSERT(pThis);

	//Local Variables
	IPersistFile* pIPersistFile = NULL; 
	TEST2C_(hr=QI(pThis,IID_IPersistFile,(void**)&pIPersistFile), S_OK, E_NOINTERFACE);
	
	ThreadSwitch(); //Let the other thread(s) catch up
	
	if(pIPersistFile)
	{
		for(ULONG i=0; i<MAX_ITERATIONS; i++)
		{
			TESTC_(hr = pIPersistFile->Load(PERSIST_FILE, 0),S_OK)
		}
	}

	ThreadSwitch(); //Let the other thread(s) catch up
	
CLEANUP:
	SAFE_RELEASE(pIPersistFile);
	THREAD_END(hr);
}

///////////////////////////////////////////////////////////
// Thread routines - IConnectionPoint
//
///////////////////////////////////////////////////////////

ULONG CThreads::Thread_CreateListener(LPVOID pv)
{
	THREAD_BEGIN					 
	HRESULT hr = S_OK;

	//Thread Stack Variables
	CRowset* pThis = (CRowset*)THREAD_FUNC;
	ASSERT(pThis);

	//Local Variables
	IConnectionPoint* pICP = NULL;
	IConnectionPointContainer* pICPC = NULL;
	DWORD dwCookie = 0;
	ULONG i=0;

	//Instantiate a listener object, within this thread
	CListener* pCListener = new CListener(IID_IRowsetNotify, pThis->pIRowset());
	TESTC(pCListener!=NULL);
	pCListener->AddRef();

	//Obtain the connection point container
	TESTC_(hr = pThis->pIRowset()->QueryInterface(IID_IConnectionPointContainer,(void **)&pICPC),S_OK)
	//Obtain the IRowsetNotify connection point 
	TESTC_(hr = pICPC->FindConnectionPoint(IID_IRowsetNotify,&pICP),S_OK)
	
	//Now we can advise the connection from the rowset->pICP to the listener in this thread
	TESTC_(hr = pICP->Advise(pCListener,&dwCookie),S_OK)

	ThreadSwitch(); //Let the other thread(s) catch up
	
	//We need to stall, so that we give enough time for the rowset in the main 
	//thread to generate a notification, so that we pick it up before 
	//closing/unadvising this connection
	
	//Wait until where notified from the Rowset
	//But don't get caught in a infinite loop
	for(i=0; i<10; i++)
	{
		if(pCListener->GetTimesNotified() > 0)
			break;
		SleepEx(1000,FALSE);	
	}


	//Make sure that we are notified once and only once
	TESTC(pCListener->GetTimesNotified() > 0)

CLEANUP:
	//Unadvise the connection to the listener
	if(pICP)
		pICP->Unadvise(dwCookie);

	SAFE_RELEASE(pCListener);
	SAFE_RELEASE(pICP);
	SAFE_RELEASE(pICPC);
	THREAD_END(hr);
}

///////////////////////////////////////////////////////////
// Thread routines - ITransaction
//
///////////////////////////////////////////////////////////
ULONG CThreads::Thread_StartTransaction(LPVOID pv)
{
	THREAD_BEGIN
	HRESULT hr = S_OK;

	//Thread Stack Variables
	CRowset* pThis = (CRowset*)THREAD_FUNC;
	ASSERT(pThis);

	//Local Variables

	ThreadSwitch(); //Let the other thread(s) catch up
	//TODO
	ThreadSwitch(); //Let the other thread(s) catch up
	
	THREAD_END(hr);
}

ULONG CThreads::Thread_Commit(LPVOID pv)
{
	THREAD_BEGIN
	HRESULT hr = S_OK;

	//Thread Stack Variables
	CRowset* pThis = (CRowset*)THREAD_FUNC;
	ASSERT(pThis);

	//Local Variables

	ThreadSwitch(); //Let the other thread(s) catch up
	//TODO
	ThreadSwitch(); //Let the other thread(s) catch up
	
	THREAD_END(hr);
}

ULONG CThreads::Thread_Abort(LPVOID pv)
{
	THREAD_BEGIN
	HRESULT hr = S_OK;

	//Thread Stack Variables
	CRowset* pThis = (CRowset*)THREAD_FUNC;
	ASSERT(pThis);

	//Local Variables

	ThreadSwitch(); //Let the other thread(s) catch up
	//TODO
	ThreadSwitch(); //Let the other thread(s) catch up
	
	THREAD_END(hr);
}


///////////////////////////////////////////////////////////
// Thread routines - IRowset
//
///////////////////////////////////////////////////////////

ULONG CThreads::Thread_AddRefRows(LPVOID pv)
{
	THREAD_BEGIN
	HRESULT hr = S_OK;

	//Thread Stack Variables
	CRowset* pThis     = (CRowset*)THREAD_FUNC;
	HROW*	  pHROW     = (HROW*)THREAD_ARG1;
	ASSERT(pThis && pHROW);

	ThreadSwitch(); //Let the other thread(s) catch up
	TESTC_(hr = pThis->pIRowset()->AddRefRows(ONE_ROW,pHROW,NULL,NULL),S_OK)
	ThreadSwitch(); //Let the other thread(s) catch up

CLEANUP:
	THREAD_END(hr);
}

ULONG CThreads::Thread_ReleaseRows(LPVOID pv)
{
	THREAD_BEGIN
	HRESULT hr = S_OK;

	//Thread Stack Variables
	CRowset*	pThis     = (CRowset*)THREAD_FUNC;
	HROW*		pHROW     = (HROW*)THREAD_ARG1;
	HRESULT		hrExpected= (HRESULT)THREAD_ARG2;
	ASSERT(pThis && pHROW);

	ThreadSwitch(); //Let the other thread(s) catch up
	TEST2C_(hr = pThis->pIRowset()->ReleaseRows(ONE_ROW,pHROW,NULL,NULL,NULL),S_OK,hrExpected);
	ThreadSwitch(); //Let the other thread(s) catch up

CLEANUP:
	THREAD_END(hr);
}

ULONG CThreads::Thread_GetNextRows(LPVOID pv)
{
	THREAD_BEGIN

	//Thread Stack Variables
	CRowset*	pThis     = (CRowset*)THREAD_FUNC;
	DBROWCOUNT		lOffset   = (LONG)THREAD_ARG1;
	DBROWCOUNT		cRows	  = (LONG)THREAD_ARG2;
	HRESULT		hrExpected= (HRESULT)THREAD_ARG3;
	HRESULT hr = S_OK;
	ASSERT(pThis);

	//Local Variables
	DBCOUNTITEM cRowsObtained = 0;
	HROW* rghRow = NULL;

	ThreadSwitch(); //Let the other thread(s) catch up
	
	//MAXOPENROWS
	if(pThis->m_ulMaxOpenRows!=0 && (ULONG)cRows>pThis->m_ulMaxOpenRows)
		cRows = pThis->m_ulMaxOpenRows;

	//GetNextRows may return DB_E_BADSTARTPOSITION if the row
	//that currently marks the next fecth position is deleted.  Therefore
	//the Arg passed into this func will let us know if there might be a possibility
	//of a DeleteRows while this thread is executing...
	TEST2C_(hr = pThis->pIRowset()->GetNextRows(NULL, lOffset, cRows, &cRowsObtained, &rghRow), S_OK, hrExpected);
	if(hr==S_OK)
	{
		//Verify results
		TESTC(cRowsObtained==(ULONG)cRows && rghRow!=NULL)
	}

	ThreadSwitch(); //Let the other thread(s) catch up

CLEANUP:
	pThis->pIRowset()->ReleaseRows(cRowsObtained,rghRow,NULL,NULL,NULL);
	PROVIDER_FREE(rghRow);
	THREAD_END(hr);
}


ULONG CThreads::Thread_GetData(LPVOID pv)
{
	THREAD_BEGIN
	HRESULT hr = S_OK;
	ASSERT(THREAD_FUNC);

	//Thread Stack Variables
	CRowset*	pThis		= (CRowset*)THREAD_FUNC;
	ULONG		cRows		= (ULONG)THREAD_ARG1;
	HROW*		rghRows		= (HROW*)THREAD_ARG2;
	HACCESSOR	hAccessor	= *(HACCESSOR*)THREAD_ARG3;
	HRESULT		hrExpected  = (HRESULT)THREAD_ARG4;

	//Allocate Data
	void* pData = PROVIDER_ALLOC(pThis->m_cRowSize * sizeof(void*));
	
	ThreadSwitch(); //Let the other thread(s) catch up
	
	for(ULONG i=0; i<MAX_ITERATIONS; i++)
	{
		for(ULONG iRow=0; iRow<cRows; iRow++)
		{
			//GetData
			hr = pThis->pIRowset()->GetData(rghRows[iRow], hAccessor, pData);

			if(FAILED(hr) && hrExpected == DB_E_DELETEDROW)
			{
				//According to the spec its provider specific if GetData is passed
				//a deleted row, but it cannot terminate abormally.  Anyother problem
				//such as an invalid row it can crash, but not for deleted.  The reason
				//is that many provider will can REMOVEDELETED = FALSE, then when
				//looping through the rowset there will be deleted rows...
				
				//We will issue a warning if something other than the obvious DB_E_DELETEDROW
				CHECKW(hr, DB_E_DELETEDROW);
			}
			else
			{
				TEST2C_(hr, S_OK, hrExpected);
			}
			pThis->ReleaseRowData(pData, hAccessor, FALSE); //Only free outofline, don't free the actual buffer...
		}
	}
	
	ThreadSwitch(); //Let the other thread(s) catch up

CLEANUP:
	pThis->ReleaseRowData(pData, hAccessor);
	THREAD_END(hr);
}


ULONG CThreads::Thread_RestartPosition(LPVOID pv)
{
	THREAD_BEGIN

	//Thread Stack Variables
	CRowset*	pThis			= (CRowset*)THREAD_FUNC;
	ULONG		fRowsReleased	= (ULONG)THREAD_ARG1;
	ASSERT(pThis && (fRowsReleased == TRUE || fRowsReleased == FALSE));
	HRESULT hr = S_OK;
	
	ThreadSwitch(); //Let the other thread(s) catch up
	
	for(ULONG i=0; i<MAX_ITERATIONS; i++)
	{
		//RestartPosition
		hr = pThis->pIRowset()->RestartPosition(NULL);
	
		//Verify Results
		if(fRowsReleased || SUCCEEDED(hr))
		{
			TEST2C_(hr, S_OK, DB_S_COMMANDREEXECUTED);
		}
		else
		{
			//According to the 2.0 OLE DB Spec, some providers may not be able to 
			//RestartPosition when there are Rows currently held, (even with CANHOLDROWS)
			TESTC_(hr,DB_E_ROWSNOTRELEASED);

			//But there are some restrictions. (and you thought it was going to be easy)
			//DBPROP_QUICKRESTART must also be FALSE
			TESTC(pThis->GetProperty(DBPROP_QUICKRESTART, DBPROPSET_ROWSET, VARIANT_FALSE));
		}
	}
	
	ThreadSwitch(); //Let the other thread(s) catch up

CLEANUP:
	THREAD_END(hr);
}


///////////////////////////////////////////////////////////
// Thread routines - IOpenRowset
//
///////////////////////////////////////////////////////////

ULONG CThreads::Thread_OpenRowset(LPVOID pv)
{
	THREAD_BEGIN
	HRESULT hr = S_OK;

	//Obtain thread stack variables	
	COpenRowset*  pThis     = (COpenRowset*)THREAD_FUNC;
	ULONG		  cPropSets = (ULONG)THREAD_ARG1;
	DBPROPSET*    rgPropSets = (DBPROPSET*)THREAD_ARG2;
	ASSERT(pThis);

	//Local Variables
	IRowset* pIRowset = NULL;

	ThreadSwitch(); //Let the other thread(s) catch up
	for(ULONG i=0; i<MAX_ITERATIONS; i++)
	{
		TESTC_(hr = pThis->pIOpenRowset()->OpenRowset(NULL,&pThis->m_pTable->GetTableID(),NULL,IID_IRowset,cPropSets,rgPropSets,(IUnknown**)&pIRowset),S_OK)
		SAFE_RELEASE(pIRowset);
	}
	ThreadSwitch(); //Let the other thread(s) catch up

CLEANUP:
	SAFE_RELEASE(pIRowset);
	THREAD_END(hr);
}

///////////////////////////////////////////////////////////
// Thread routines - IRowsetChange
//
///////////////////////////////////////////////////////////

ULONG CThreads::Thread_SetData(LPVOID pv)
{
	THREAD_BEGIN
	HRESULT hr = S_OK;

	//Thread Stack Variables
	CRowsetChange*  pThis  = (CRowsetChange*)THREAD_FUNC;
	HROW*	   phRow       = (HROW*)THREAD_ARG1;
	HACCESSOR* phAccessor  = (HACCESSOR*)THREAD_ARG2;
	HRESULT	   hrExpected  = (HRESULT)THREAD_ARG3;
	ASSERT(pThis && phRow && phAccessor);
	void* pData = NULL;

	ThreadSwitch(); //Let the other thread(s) catch up

	//Could be called while a ReleaseRows is called
	for(ULONG i=0; i<MAX_ITERATIONS; i++)
	{
		//Make data for new row(s)
		pThis->MakeRowData(&pData, *phAccessor);

		//Set data
		TEST2C_(hr = pThis->SetData(*phRow, *phAccessor, pData), S_OK, hrExpected);
	
		//Only Release out-of-line data, not the buffer
		pThis->ReleaseRowData(pData, *phAccessor, FALSE);  
	}

	ThreadSwitch(); //Let the other thread(s) catch up

CLEANUP:
	pThis->ReleaseRowData(pData, *phAccessor);
	THREAD_END(hr);
}


ULONG CThreads::Thread_DeleteRows(LPVOID pv)
{
	THREAD_BEGIN
	HRESULT hr;

	//Thread Stack Variables
	CRowsetChange*  pThis		= (CRowsetChange*)THREAD_FUNC;
	ULONG			cRows		= (ULONG)THREAD_ARG1;
	HROW*           rghRow		= (HROW*)THREAD_ARG2;
	HRESULT         hrExpected	= (HRESULT)THREAD_ARG3;
	HRESULT         hrExpected2	= (HRESULT)THREAD_ARG4;
	ASSERT(pThis && cRows && rghRow);
	
	//Local Variables
	DBROWSTATUS* rgRowStatus = (DBROWSTATUS*)PROVIDER_ALLOC(cRows*sizeof(DBROWSTATUS));

	ThreadSwitch(); //Let the other thread(s) catch up
	
	for(ULONG i=0; i<MAX_ITERATIONS; i++)
	{
		//Other threads may be deleting these same rows
		hr = pThis->pIRowsetChange()->DeleteRows(NULL,cRows,rghRow,rgRowStatus);
		TEST3C_(hr, S_OK, hrExpected, hrExpected2)

		if(hr==S_OK)
		{
			TESTC(VerifyArray(cRows, rgRowStatus, DBROWSTATUS_S_OK));
		}
		else
		{
			//make sure that any error was due to other threads interactions
			//it may have already been deleted or released by another thread
			for(ULONG iStatus=0; iStatus<cRows; iStatus++)
			{
				TESTC(	rgRowStatus[iStatus]==DBROWSTATUS_S_OK || 
						rgRowStatus[iStatus]==DBROWSTATUS_E_DELETED ||
						rgRowStatus[iStatus]==DBROWSTATUS_E_NEWLYINSERTED ||
						rgRowStatus[iStatus]==DBROWSTATUS_E_INVALID ||
						rgRowStatus[iStatus]==DBROWSTATUS_E_MAXPENDCHANGESEXCEEDED);
			}
		}
	}

	ThreadSwitch(); //Let the other thread(s) catch up

CLEANUP:
	PROVIDER_FREE(rgRowStatus);
	THREAD_END(hr);
}


ULONG CThreads::Thread_InsertRow(LPVOID pv)
{
	THREAD_BEGIN
	HRESULT hr = S_OK;

	//Thread Stack Variables
	CRowsetChange*  pThis		= (CRowsetChange*)THREAD_FUNC;
	HACCESSOR*		phAccessor	= (HACCESSOR*)THREAD_ARG1;
	HRESULT			hrExpected	= (HRESULT)THREAD_ARG2;

	HROW			rghRows[MAX_ITERATIONS];
	ASSERT(pThis && phAccessor);
	void* pData = NULL;

	ThreadSwitch(); //Let the other thread(s) catch up
	for(ULONG i=0; i<MAX_ITERATIONS; i++)
	{
		//Make data for new row(s)
		//This needs to be done inside the loop, as a new kay value needs to be generated
		pThis->MakeRowData(&pData, *phAccessor);

		TEST2C_(hr = pThis->pIRowsetChange()->InsertRow(NULL, *phAccessor, pData, &rghRows[i]),S_OK,hrExpected)
		pThis->ReleaseRowData(pData, *phAccessor);
		pData = NULL;
	}
	ThreadSwitch(); //Let the other thread(s) catch up

CLEANUP:
	pThis->pIRowset()->ReleaseRows(MAX_ITERATIONS, rghRows, NULL, NULL, NULL);
	pThis->ReleaseRowData(pData, *phAccessor);
	THREAD_END(hr);
}


///////////////////////////////////////////////////////////
// Thread routines - IRowsetInfo
//
///////////////////////////////////////////////////////////
ULONG CThreads::Thread_GetRowsetProperties(LPVOID pv)
{
	THREAD_BEGIN
	HRESULT hr = S_OK;

	//Thread Stack Variables
	CRowset*		pThis			= (CRowset*)THREAD_FUNC;
	ULONG			cPropIDSets		= (ULONG)THREAD_ARG1;
	DBPROPIDSET*	rgPropIDSets	= (DBPROPIDSET*)THREAD_ARG2;
	ASSERT(pThis);

	//Can be (NULL) for all supported properties
	
	//Local Variables
	ULONG i,cPropSets = 0;
	DBPROPSET* rgPropSets = NULL;

	IRowsetInfo* pIRowsetInfo = NULL;
	
	TESTC_(hr = QI(pThis->pIRowset(),IID_IRowsetInfo,(void**)&pIRowsetInfo),S_OK)

	ThreadSwitch(); //Let the other thread(s) catch up
	for(i=0; i<MAX_ITERATIONS; i++)
	{
		TESTC_(hr = pIRowsetInfo->GetProperties(cPropIDSets, rgPropIDSets, &cPropSets, &rgPropSets),S_OK);
		TESTC(rgPropSets!=NULL);
		::FreeProperties(&cPropSets,&rgPropSets);
	}
	ThreadSwitch(); //Let the other thread(s) catch up

CLEANUP:
	SAFE_RELEASE(pIRowsetInfo);
	::FreeProperties(&cPropSets,&rgPropSets);
	THREAD_END(hr);
}

ULONG CThreads::Thread_GetSpecification(LPVOID pv)
{
	THREAD_BEGIN
	HRESULT hr = S_OK;

	//Thread Stack Variables
	CRowset*  pThis  = (CRowset*)THREAD_FUNC;
	IID*      priid  = (IID*)THREAD_ARG1;
	ASSERT(pThis && priid);
	ULONG i=0;

	//Local Variables
	IUnknown* pIUnknown = NULL;
	IRowsetInfo* pIRowsetInfo = NULL;

	TESTC_(QI(pThis->pIRowset(),IID_IRowsetInfo,(void**)&pIRowsetInfo),S_OK)

	ThreadSwitch(); //Let the other thread(s) catch up
	for(i=0; i<MAX_ITERATIONS; i++)
	{
		hr = pIRowsetInfo->GetSpecification(*priid,&pIUnknown);
		TEST2C_(hr,S_OK,S_FALSE);
		
		if(hr==S_OK)
		{
			TESTC(pIUnknown!=NULL);
		}
		else
		{
			TWARNING("GetSpecification returned S_FALSE?");
			TESTC(pIUnknown==NULL);
		}
		SAFE_RELEASE(pIUnknown);
	}

	ThreadSwitch(); //Let the other thread(s) catch up

CLEANUP:
	SAFE_RELEASE(pIUnknown);
	SAFE_RELEASE(pIRowsetInfo);
	THREAD_END(hr);
}


///////////////////////////////////////////////////////////
// Thread routines - IRowsetResynch
//
///////////////////////////////////////////////////////////

ULONG CThreads::Thread_GetVisibleData(LPVOID pv)
{
	THREAD_BEGIN
	HRESULT hr = S_OK;

	//Thread Stack Variables
	CRowset*   pThis       = (CRowset*)THREAD_FUNC;
	HROW*	   phRow       = (HROW*)THREAD_ARG1;
	HACCESSOR* phAccessor  = (HACCESSOR*)THREAD_ARG2;
	ASSERT(pThis && phRow && phAccessor);

	//Allocate Data
	void* pData = PROVIDER_ALLOC((sizeof(void*)*pThis->m_cRowSize));
	ULONG i=0;

	//Local Variables
	IRowsetResynch* pIRowsetResynch = NULL;
	TESTC_(hr = QI(pThis->pIRowset(),IID_IRowsetResynch,(void**)&pIRowsetResynch),S_OK)

	ThreadSwitch(); //Let the other thread(s) catch up
	for(i=0; i<MAX_ITERATIONS; i++)
	{
		TESTC_(hr = pIRowsetResynch->GetVisibleData(*phRow,*phAccessor, pData),S_OK)
		pThis->ReleaseRowData(pData, *phAccessor,FALSE);	//Only release the out-of-line memory
	}
	ThreadSwitch(); //Let the other thread(s) catch up

CLEANUP:
	SAFE_RELEASE(pIRowsetResynch);
	pThis->ReleaseRowData(pData, *phAccessor);
	THREAD_END(hr);
}

ULONG CThreads::Thread_ResynchRows(LPVOID pv)
{
	THREAD_BEGIN
	HRESULT hr = S_OK;

	//Thread Stack Variables
	CRowset*   pThis       = (CRowset*)THREAD_FUNC;
	ASSERT(pThis);

	//Local Variables
	DBCOUNTITEM i,cRowsResynched = 0;
	HROW* rghRowsResynched = NULL;
	DBROWSTATUS* rgRowStatus = NULL;

	//Local Variables
	IRowsetResynch* pIRowsetResynch = NULL;
	TESTC_(QI(pThis->pIRowset(),IID_IRowsetResynch,(void**)&pIRowsetResynch),S_OK)

	ThreadSwitch(); //Let the other thread(s) catch up
		
	//Resynch all rows
	//Can return DB_S_ for rows not resycned (unchanged)
	for(i=0; i<MAX_ITERATIONS; i++)
	{
		hr = pIRowsetResynch->ResynchRows(0,NULL,&cRowsResynched,&rghRowsResynched,&rgRowStatus);
		TEST2C_(hr, S_OK, DB_S_ERRORSOCCURRED)
		pThis->pIRowset()->ReleaseRows(cRowsResynched,rghRowsResynched,NULL,NULL,NULL); 
		PROVIDER_FREE(rghRowsResynched);
		PROVIDER_FREE(rgRowStatus);
	}

	ThreadSwitch(); //Let the other thread(s) catch up

CLEANUP:
	pThis->pIRowset()->ReleaseRows(cRowsResynched,rghRowsResynched,NULL,NULL,NULL); 
	PROVIDER_FREE(rghRowsResynched);
	PROVIDER_FREE(rgRowStatus);
	SAFE_RELEASE(pIRowsetResynch);
	THREAD_END(hr);
}

///////////////////////////////////////////////////////////
// Thread routines - IRowsetWithParameters
//
///////////////////////////////////////////////////////////

ULONG CThreads::Thread_Requery(LPVOID pv)
{
	THREAD_BEGIN
	HRESULT hr = S_OK;

	//Thread Stack Variables
	CRowset*  pThis  = (CRowset*)THREAD_FUNC;
	ASSERT(pThis);

	ThreadSwitch(); //Let the other thread(s) catch up
	//TODO V2
	ThreadSwitch(); //Let the other thread(s) catch up

	THREAD_END(hr);
}


///////////////////////////////////////////////////////////
// Thread routines - IRowsetLocate
//
///////////////////////////////////////////////////////////

ULONG CThreads::Thread_GetRowsAt(LPVOID pv)
{
	THREAD_BEGIN
	HRESULT hr = S_OK;

	//Thread Stack Variables
	CRowset*  pThis  = (CRowset*)THREAD_FUNC;
	ASSERT(pThis);

	//Local Variables
	IRowsetLocate* pIRowsetLocate = NULL;

	DBCOUNTITEM i,cRowsObtained = 0;
	HROW* rghRow = NULL;
	BYTE  Bookmark = DBBMK_FIRST;

	TESTC_(hr = QI(pThis->pIRowset(),IID_IRowsetLocate,(void**)&pIRowsetLocate),S_OK)
	
	ThreadSwitch(); //Let the other thread(s) catch up
	for(i=0; i<MAX_ITERATIONS; i++)
	{
		TESTC_(hr = pIRowsetLocate->GetRowsAt(NULL,NULL,1,&Bookmark,0, ONE_ROW, &cRowsObtained, &rghRow),S_OK)
		pThis->pIRowset()->ReleaseRows(cRowsObtained,rghRow,NULL,NULL,NULL);
		PROVIDER_FREE(rghRow);
	}
	ThreadSwitch(); //Let the other thread(s) catch up

CLEANUP:
	pThis->pIRowset()->ReleaseRows(cRowsObtained,rghRow,NULL,NULL,NULL);
	PROVIDER_FREE(rghRow);
	SAFE_RELEASE(pIRowsetLocate);
	THREAD_END(hr);
}


ULONG CThreads::Thread_GetRowsByBookmark(LPVOID pv)
{
	THREAD_BEGIN
	HRESULT hr = S_OK;

	//Thread Stack Variables
	CRowset*  pThis		= (CRowset*)THREAD_FUNC;
	DBBKMARK* pcbBookmark	= (DBBKMARK*)THREAD_ARG1;
	BYTE** ppBookmark	= (BYTE**)THREAD_ARG2;
	ASSERT(pThis && pcbBookmark && ppBookmark);

	//Local Variables
	IRowsetLocate* pIRowsetLocate = NULL;
	HROW rghRows[MAX_ITERATIONS];
	DBROWSTATUS rgRowStatus[ONE_ROW];
	const BYTE* rgpBookmark[] = {*ppBookmark};
	ULONG i=0;

	//Obtain the Locate interface
	TESTC_(hr = QI(pThis->pIRowset(),IID_IRowsetLocate,(void**)&pIRowsetLocate),S_OK)
	ThreadSwitch(); //Let the other thread(s) catch up

	//GetRowsByBookmark
	for(i=0; i<MAX_ITERATIONS; i++)
	{
		TESTC_(hr = pIRowsetLocate->GetRowsByBookmark(NULL, ONE_ROW, pcbBookmark, rgpBookmark, &rghRows[i], rgRowStatus),S_OK)
	}
	ThreadSwitch(); //Let the other thread(s) catch up

CLEANUP:
	pThis->pIRowset()->ReleaseRows(MAX_ITERATIONS, rghRows, NULL, NULL, NULL);
	SAFE_RELEASE(pIRowsetLocate);
	THREAD_END(hr);
}


///////////////////////////////////////////////////////////
// Thread routines - IRowsetUpdate
//
///////////////////////////////////////////////////////////

ULONG CThreads::Thread_GetOriginalData(LPVOID pv)
{
	THREAD_BEGIN
	HRESULT hr = S_OK;

	//Thread Stack Variables
	CRowsetUpdate*  pThis  = (CRowsetUpdate*)THREAD_FUNC;
	HROW*	   phRow       = (HROW*)THREAD_ARG1;
	HACCESSOR* phAccessor  = (HACCESSOR*)THREAD_ARG2;
	ASSERT(pThis && phRow && phAccessor);

	//Allocate Data
	void* pData = PROVIDER_ALLOC((sizeof(void*)*pThis->m_cRowSize));

	ThreadSwitch(); //Let the other thread(s) catch up
	for(ULONG i=0; i<MAX_ITERATIONS; i++)
	{
		TESTC_(hr = pThis->pIRowsetUpdate()->GetOriginalData(*phRow,*phAccessor, pData),S_OK)
		pThis->ReleaseRowData(pData, *phAccessor,FALSE);//Only free the out-of-line memory, not the buffer
	}
	ThreadSwitch(); //Let the other thread(s) catch up

CLEANUP:
	pThis->ReleaseRowData(pData, *phAccessor);
	THREAD_END(hr);
}

ULONG CThreads::Thread_GetPendingRows(LPVOID pv)
{
	THREAD_BEGIN
	HRESULT hr = S_OK;

	//Thread Stack Variables
	CRowsetUpdate*  pThis = (CRowsetUpdate*)THREAD_FUNC;
	ASSERT(pThis);

	//Local Variables
	DBCOUNTITEM cPendingRows = 0;
	HROW* rgPendingRows = NULL;
	DBROWSTATUS* rgPendingStatus = NULL;

	ThreadSwitch(); //Let the other thread(s) catch up
	
	//GetPendingRows for all rows
	//Can be either S_OK (pending rows) or S_FALSE (no pending rows)
	for(ULONG i=0; i<MAX_ITERATIONS; i++)
	{
		hr = pThis->pIRowsetUpdate()->GetPendingRows(NULL,DBPENDINGSTATUS_ALL,&cPendingRows,&rgPendingRows,&rgPendingStatus);
		TEST2C_(hr, S_OK, S_FALSE);
		pThis->pIRowset()->ReleaseRows(cPendingRows,rgPendingRows,NULL,NULL,NULL);
		PROVIDER_FREE(rgPendingRows);
		PROVIDER_FREE(rgPendingStatus);
	}

	ThreadSwitch(); //Let the other thread(s) catch up

CLEANUP:  
	pThis->pIRowset()->ReleaseRows(cPendingRows,rgPendingRows,NULL,NULL,NULL);
	PROVIDER_FREE(rgPendingRows);
	PROVIDER_FREE(rgPendingStatus);
	THREAD_END(hr);
}

ULONG CThreads::Thread_GetRowStatus(LPVOID pv)
{
	THREAD_BEGIN
	HRESULT hr = S_OK;

	//Thread Stack Variables
	CRowsetUpdate*  pThis  = (CRowsetUpdate*)THREAD_FUNC;
	ULONG*           pcRows = (ULONG*)THREAD_ARG1;
	HROW*            rghRow = (HROW*)THREAD_ARG2;
	HRESULT          hrExpected = (HRESULT)THREAD_ARG3;
	HRESULT          hrExpected2 = (HRESULT)THREAD_ARG3;
	ASSERT(pThis && pcRows && rghRow);

	//Local Variables
	DBROWSTATUS* rgPendingStatus = (DBROWSTATUS*)PROVIDER_ALLOC(sizeof(DBROWSTATUS)*(*pcRows));

	ThreadSwitch(); //Let the other thread(s) catch up
	
	for(ULONG i=0; i<MAX_ITERATIONS; i++)
	{
		hr = pThis->pIRowsetUpdate()->GetRowStatus(NULL,*pcRows,rghRow,rgPendingStatus);
		TEST3C_(hr, S_OK, hrExpected, hrExpected2);
	}

	ThreadSwitch(); //Let the other thread(s) catch up

CLEANUP:
	PROVIDER_FREE(rgPendingStatus);
	THREAD_END(hr);
}


ULONG CThreads::Thread_Undo(LPVOID pv)
{
	THREAD_BEGIN
	HRESULT hr = S_OK;

	//Thread Stack Variables
	CRowsetUpdate*  pThis  = (CRowsetUpdate*)THREAD_FUNC;
	ASSERT(pThis);

	//Local Variables
	DBCOUNTITEM cRowsUndone = 0;
	HROW* rgRowsUndone = NULL;
	DBROWSTATUS* rgRowStatus = NULL;

	ThreadSwitch(); //Let the other thread(s) catch up

	for(ULONG i=0; i<MAX_ITERATIONS; i++)
	{
		//Undo all rows that have pending changes
		TESTC_(hr = pThis->pIRowsetUpdate()->Undo(NULL,0,NULL,&cRowsUndone,&rgRowsUndone,&rgRowStatus),S_OK)
		pThis->pIRowset()->ReleaseRows(cRowsUndone,rgRowsUndone,NULL,NULL,NULL);
		PROVIDER_FREE(rgRowsUndone);
		PROVIDER_FREE(rgRowStatus);
	}
	
	ThreadSwitch(); //Let the other thread(s) catch up

CLEANUP:  
	pThis->pIRowset()->ReleaseRows(cRowsUndone,rgRowsUndone,NULL,NULL,NULL);
	PROVIDER_FREE(rgRowsUndone);
	PROVIDER_FREE(rgRowStatus);
	THREAD_END(hr);
}


ULONG CThreads::Thread_Update(LPVOID pv)
{
	THREAD_BEGIN
	HRESULT hr = S_OK;

	//Thread Stack Variables
	CRowsetUpdate*  pThis  = (CRowsetUpdate*)THREAD_FUNC;
	ASSERT(pThis);

	//Local Variables
	DBCOUNTITEM cUpdatedRows = 0;
	HROW* rgUpdatedRows = NULL;
	DBROWSTATUS* rgRowStatus = NULL;

	ThreadSwitch(); //Let the other thread(s) catch up
		
	for(ULONG i=0; i<MAX_ITERATIONS; i++)
	{
		//Update all rows that have pending changes
		TESTC_(hr = pThis->pIRowsetUpdate()->Update(NULL,0,NULL,&cUpdatedRows,&rgUpdatedRows,&rgRowStatus),S_OK)

		pThis->pIRowset()->ReleaseRows(cUpdatedRows,rgUpdatedRows,NULL,NULL,NULL); 
		PROVIDER_FREE(rgUpdatedRows);
		PROVIDER_FREE(rgRowStatus);
	}

	ThreadSwitch(); //Let the other thread(s) catch up

CLEANUP:  
	pThis->pIRowset()->ReleaseRows(cUpdatedRows,rgUpdatedRows,NULL,NULL,NULL); 
	PROVIDER_FREE(rgUpdatedRows);
	PROVIDER_FREE(rgRowStatus);
	THREAD_END(hr);
}


///////////////////////////////////////////////////////////
// Thread routines - IMultipleResults
//
///////////////////////////////////////////////////////////

ULONG CThreads::Thread_GetResult(LPVOID pv)
{
	THREAD_BEGIN
	HRESULT hr = S_OK;

	//Thread Stack Variables
	CRowset*			pThis	= (CRowset*)THREAD_FUNC;					 //[in]
	IMultipleResults*   pIMultipleResults  = (IMultipleResults*)THREAD_ARG1; //[in]
	IRowset**			ppIRowset = (IRowset**)THREAD_ARG2;					 //[out]
	ASSERT(pThis && pIMultipleResults && ppIRowset);

	ThreadSwitch(); //Let the other thread(s) catch up
	
	//IMultipleResults::GetResults can return S_OK or DB_E_NORESULT if already
	//processed all the result sets
	hr = pIMultipleResults->GetResult(NULL, 0, IID_IRowset, NULL, (IUnknown**)ppIRowset);
	TEST3C_(hr, S_OK, DB_E_OBJECTOPEN, DB_S_NORESULT);

	//Verify Results
	if(hr == S_OK)
	{
		TESTC(*ppIRowset != NULL);
	}
	else
	{
		TESTC(*ppIRowset == NULL);
	}

	ThreadSwitch(); //Let the other thread(s) catch up

CLEANUP:
	THREAD_END(hr);
}


// {{ TCW_TEST_CASE_MAP(TCDataSource)
//--------------------------------------------------------------------
// @class Multi-Threaded testing of the DataSourceObject
//
class TCDataSource : public CThreads { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCDataSource,CThreads);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember GetDataSource threading model CLSID
	int Variation_1();
	// @cmember GetProperties and SetProperites on the same DSO
	int Variation_2();
	// @cmember GetProperties on the same set of properties
	int Variation_3();
	// @cmember GetPropertyInfo on all properties supported
	int Variation_4();
	// @cmember SetProperties on the same DSO
	int Variation_5();
	// @cmember Initialize the DSO from sep threads
	int Variation_6();
	// @cmember Unitilize the DSO from sep threads
	int Variation_7();
	// @cmember Initialize / Unitialize same DSO from sep threads
	int Variation_8();
	// @cmember Initialize seperate DSOs in threads
	int Variation_9();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCDataSource)
#define THE_CLASS TCDataSource
BEG_TEST_CASE(TCDataSource, CThreads, L"Multi-Threaded testing of the DataSourceObject")
	TEST_VARIATION(1, 		L"GetDataSource threading model CLSID")
	TEST_VARIATION(2, 		L"GetProperties and SetProperites on the same DSO")
	TEST_VARIATION(3, 		L"GetProperties on the same set of properties")
	TEST_VARIATION(4, 		L"GetPropertyInfo on all properties supported")
	TEST_VARIATION(5, 		L"SetProperties on the same DSO")
	TEST_VARIATION(6, 		L"Initialize the DSO from sep threads")
	TEST_VARIATION(7, 		L"Unitilize the DSO from sep threads")
	TEST_VARIATION(8, 		L"Initialize / Unitialize same DSO from sep threads")
	TEST_VARIATION(9, 		L"Initialize seperate DSOs in threads")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCSession)
//--------------------------------------------------------------------
// @class Multi-Threaded testing of the SessionObject
//
class TCSession : public CThreads { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCSession,CThreads);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Get the Session Threading model CLSID
	int Variation_1();
	// @cmember Get the Session threading model Property
	int Variation_2();
	// @cmember CreateSession from sep threads
	int Variation_3();
	// @cmember CreateCommand from seperate threads
	int Variation_4();
	// @cmember Empty
	int Variation_5();
	// @cmember Add a column / drop a column from sep threads
	int Variation_6();
	// @cmember Create a table from seperate threads
	int Variation_7();
	// @cmember Empty
	int Variation_8();
	// @cmember Get schema rowset from seperate threads
	int Variation_9();
	// @cmember Get schema info from sep threads
	int Variation_10();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCSession)
#define THE_CLASS TCSession
BEG_TEST_CASE(TCSession, CThreads, L"Multi-Threaded testing of the SessionObject")
	TEST_VARIATION(1, 		L"Get the Session Threading model CLSID")
	TEST_VARIATION(2, 		L"Get the Session threading model Property")
	TEST_VARIATION(3, 		L"CreateSession from sep threads")
	TEST_VARIATION(4, 		L"CreateCommand from seperate threads")
	TEST_VARIATION(5, 		L"Empty")
	TEST_VARIATION(6, 		L"Add a column / drop a column from sep threads")
	TEST_VARIATION(7, 		L"Create a table from seperate threads")
	TEST_VARIATION(8, 		L"Empty")
	TEST_VARIATION(9, 		L"Get schema rowset from seperate threads")
	TEST_VARIATION(10, 		L"Get schema info from sep threads")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCCommand)
//--------------------------------------------------------------------
// @class Multi-Threaded testing of the CommandObject
//
class TCCommand : public CThreads { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCCommand,CThreads);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Get the command threading model CLSID
	int Variation_1();
	// @cmember Get the command threading model Property
	int Variation_2();
	// @cmember Create command objects on seperate threads
	int Variation_3();
	// @cmember Call execute from sep threads, (same command object
	int Variation_4();
	// @cmember Call cancel while two executes are occurring
	int Variation_5();
	// @cmember Empty
	int Variation_6();
	// @cmember Set/Unset diff prop
	int Variation_7();
	// @cmember Set/Unset same prop
	int Variation_8();
	// @cmember Get/Set prop from sep threads
	int Variation_9();
	// @cmember Set the command text / Get text from another thread
	int Variation_10();
	// @cmember Empty
	int Variation_11();
	// @cmember Uprepare / Prepare
	int Variation_12();
	// @cmember Unprepare / Execute
	int Variation_13();
	// @cmember Empty
	int Variation_14();
	// @cmember SetDefaultValues from sep threads, GetDefaultValues from a third
	int Variation_15();
	// @cmember Empty
	int Variation_16();
	// @cmember Execute / Release the command object
	int Variation_17();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCCommand)
#define THE_CLASS TCCommand
BEG_TEST_CASE(TCCommand, CThreads, L"Multi-Threaded testing of the CommandObject")
	TEST_VARIATION(1, 		L"Get the command threading model CLSID")
	TEST_VARIATION(2, 		L"Get the command threading model Property")
	TEST_VARIATION(3, 		L"Create command objects on seperate threads")
	TEST_VARIATION(4, 		L"Call execute from sep threads, (same command object")
	TEST_VARIATION(5, 		L"Call cancel while two executes are occurring")
	TEST_VARIATION(6, 		L"Empty")
	TEST_VARIATION(7, 		L"Set/Unset diff prop")
	TEST_VARIATION(8, 		L"Set/Unset same prop")
	TEST_VARIATION(9, 		L"Get/Set prop from sep threads")
	TEST_VARIATION(10, 		L"Set the command text / Get text from another thread")
	TEST_VARIATION(11, 		L"Empty")
	TEST_VARIATION(12, 		L"Uprepare / Prepare")
	TEST_VARIATION(13, 		L"Unprepare / Execute")
	TEST_VARIATION(14, 		L"Empty")
	TEST_VARIATION(15, 		L"SetDefaultValues from sep threads, GetDefaultValues from a third")
	TEST_VARIATION(16, 		L"Empty")
	TEST_VARIATION(17, 		L"Execute / Release the command object")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCAccessor)
//--------------------------------------------------------------------
// @class Multi-Threaded testing of the Accessor object
//
class TCAccessor : public CThreads { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCAccessor,CThreads);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Create multiple accessors from diff threads
	int Variation_1();
	// @cmember Use the same accessor on diff threads
	int Variation_2();
	// @cmember Empty
	int Variation_3();
	// @cmember GetBindiings / ReleaseAcessor
	int Variation_4();
	// @cmember GetColumnsInfo / IRowsetInfo
	int Variation_5();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCAccessor)
#define THE_CLASS TCAccessor
BEG_TEST_CASE(TCAccessor, CThreads, L"Multi-Threaded testing of the Accessor object")
	TEST_VARIATION(1, 		L"Create multiple accessors from diff threads")
	TEST_VARIATION(2, 		L"Use the same accessor on diff threads")
	TEST_VARIATION(3, 		L"Empty")
	TEST_VARIATION(4, 		L"GetBindiings / ReleaseAcessor")
	TEST_VARIATION(5, 		L"GetColumnsInfo / IRowsetInfo")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCError)
//--------------------------------------------------------------------
// @class Multi-Threaded testing of the ErrorObject
//
class TCError : public CThreads { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCError,CThreads);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Get the Error object threading model CLSID
	int Variation_1();
	// @cmember Get the Error object threading model Property
	int Variation_2();
	// @cmember AddErrorRecord from sep threads, GetErrorInfo from a third
	int Variation_3();
	// @cmember Transfer error record to antoher thread.
	int Variation_4();
	// @cmember Apartment model - make sure apartment model error object is returned
	int Variation_5();
	// @cmember Empty
	int Variation_6();
	// @cmember Generate Errors from sep threads
	int Variation_7();
	// @cmember Use diff threads to make IErrorInfo calls
	int Variation_8();
	// @cmember Create Error object on 1 thread, set error info from many other threads
	int Variation_9();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCError)
#define THE_CLASS TCError
BEG_TEST_CASE(TCError, CThreads, L"Multi-Threaded testing of the ErrorObject")
	TEST_VARIATION(1, 		L"Get the Error object threading model CLSID")
	TEST_VARIATION(2, 		L"Get the Error object threading model Property")
	TEST_VARIATION(3, 		L"AddErrorRecord from sep threads, GetErrorInfo from a third")
	TEST_VARIATION(4, 		L"Transfer error record to antoher thread.")
	TEST_VARIATION(5, 		L"Apartment model - make sure apartment model error object is returned")
	TEST_VARIATION(6, 		L"Empty")
	TEST_VARIATION(7, 		L"Generate Errors from sep threads")
	TEST_VARIATION(8, 		L"Use diff threads to make IErrorInfo calls")
	TEST_VARIATION(9, 		L"Create Error object on 1 thread, set error info from many other threads")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCTransaction)
//--------------------------------------------------------------------
// @class Multi-Threaded testing of the TransactionObject
//
class TCTransaction : public CThreads { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCTransaction,CThreads);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Get the transaction threading model CLSID
	int Variation_1();
	// @cmember Get the transaction threading model Property
	int Variation_2();
	// @cmember Two Commits, from two transactions
	int Variation_3();
	// @cmember Multiple thread running under one transaction, commit
	int Variation_4();
	// @cmember Multiple thread running under one transaction, abort
	int Variation_5();
	// @cmember Empty
	int Variation_6();
	// @cmember One thread aborts, while the other is doing real work
	int Variation_7();
	// @cmember One thread commits, while the other thread is doing real work
	int Variation_8();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCTransaction)
#define THE_CLASS TCTransaction
BEG_TEST_CASE(TCTransaction, CThreads, L"Multi-Threaded testing of the TransactionObject")
	TEST_VARIATION(1, 		L"Get the transaction threading model CLSID")
	TEST_VARIATION(2, 		L"Get the transaction threading model Property")
	TEST_VARIATION(3, 		L"Two Commits, from two transactions")
	TEST_VARIATION(4, 		L"Multiple thread running under one transaction, commit")
	TEST_VARIATION(5, 		L"Multiple thread running under one transaction, abort")
	TEST_VARIATION(6, 		L"Empty")
	TEST_VARIATION(7, 		L"One thread aborts, while the other is doing real work")
	TEST_VARIATION(8, 		L"One thread commits, while the other thread is doing real work")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCOpenRowset)
//--------------------------------------------------------------------
// @class Multi-Threaded testing of the OpenRowsetObject
//
class TCOpenRowset : public CThreads { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCOpenRowset,CThreads);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember QI for IID_IOpenRowset from sep threads
	int Variation_1();
	// @cmember Call OpenRowset from two threads, diff prop, from the same table
	int Variation_2();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCOpenRowset)
#define THE_CLASS TCOpenRowset
BEG_TEST_CASE(TCOpenRowset, CThreads, L"Multi-Threaded testing of the OpenRowsetObject")
	TEST_VARIATION(1, 		L"QI for IID_IOpenRowset from sep threads")
	TEST_VARIATION(2, 		L"Call OpenRowset from two threads, diff prop, from the same table")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCRowset)
//--------------------------------------------------------------------
// @class Multi-Threaded testing of the RowsetObject
//
class TCRowset : public CThreads { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCRowset,CThreads);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Empty
	int Variation_1();
	// @cmember Verify threading model - Free Threaded
	int Variation_2();
	// @cmember Verify read-only threading property
	int Variation_3();
	// @cmember Empty
	int Variation_4();
	// @cmember QI from sep threads for diff riid's
	int Variation_5();
	// @cmember AddRef / Release testing
	int Variation_6();
	// @cmember AddRefRows / ReleaseRows testing
	int Variation_7();
	// @cmember Empty
	int Variation_8();
	// @cmember GetNextRows from multiple threads
	int Variation_9();
	// @cmember GetNextRows from multiple threads, with overlapping set of rows
	int Variation_10();
	// @cmember GetNextRows and GetData from seperate threads
	int Variation_11();
	// @cmember Empty
	int Variation_12();
	// @cmember RestartPosition with GetNextRows
	int Variation_13();
	// @cmember RestartPosition with GetData
	int Variation_14();
	// @cmember Empty
	int Variation_15();
	// @cmember Release a row being accessed by GetNextRows
	int Variation_16();
	// @cmember Delete a row, being accessed by GetData
	int Variation_17();
	// @cmember Release the rowset while being used by another
	int Variation_18();
	// @cmember Empty
	int Variation_19();
	// @cmember GetData, same row, same accessor, diff threads
	int Variation_20();
	// @cmember GetData, same row, diff accessor, diff threads
	int Variation_21();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCRowset)
#define THE_CLASS TCRowset
BEG_TEST_CASE(TCRowset, CThreads, L"Multi-Threaded testing of the RowsetObject")
	TEST_VARIATION(1, 		L"Empty")
	TEST_VARIATION(2, 		L"Verify threading model - Free Threaded")
	TEST_VARIATION(3, 		L"Verify read-only threading property")
	TEST_VARIATION(4, 		L"Empty")
	TEST_VARIATION(5, 		L"QI from sep threads for diff riid's")
	TEST_VARIATION(6, 		L"AddRef / Release testing")
	TEST_VARIATION(7, 		L"AddRefRows / ReleaseRows testing")
	TEST_VARIATION(8, 		L"Empty")
	TEST_VARIATION(9, 		L"GetNextRows from multiple threads")
	TEST_VARIATION(10, 		L"GetNextRows from multiple threads, with overlapping set of rows")
	TEST_VARIATION(11, 		L"GetNextRows and GetData from seperate threads")
	TEST_VARIATION(12, 		L"Empty")
	TEST_VARIATION(13, 		L"RestartPosition with GetNextRows")
	TEST_VARIATION(14, 		L"RestartPosition with GetData")
	TEST_VARIATION(15, 		L"Empty")
	TEST_VARIATION(16, 		L"Release a row being accessed by GetNextRows")
	TEST_VARIATION(17, 		L"Delete a row, being accessed by GetData")
	TEST_VARIATION(18, 		L"Release the rowset while being used by another")
	TEST_VARIATION(19, 		L"Empty")
	TEST_VARIATION(20, 		L"GetData, same row, same accessor, diff threads")
	TEST_VARIATION(21, 		L"GetData, same row, diff accessor, diff threads")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCRowsetChange)
//--------------------------------------------------------------------
// @class Multi-Threaded testing of the RowsetChange Object
//
class TCRowsetChange : public CThreads { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCRowsetChange,CThreads);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember SetData, diff columns, from seperate threads
	int Variation_1();
	// @cmember SetData, same columns, from seperate threads
	int Variation_2();
	// @cmember Empty
	int Variation_3();
	// @cmember DeleteRows from seperate threads, with overlapping set of rows
	int Variation_4();
	// @cmember Empty
	int Variation_5();
	// @cmember Insert a row from seperate threads
	int Variation_6();
	// @cmember Empty
	int Variation_7();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCRowsetChange)
#define THE_CLASS TCRowsetChange
BEG_TEST_CASE(TCRowsetChange, CThreads, L"Multi-Threaded testing of the RowsetChange Object")
	TEST_VARIATION(1, 		L"SetData, diff columns, from seperate threads")
	TEST_VARIATION(2, 		L"SetData, same columns, from seperate threads")
	TEST_VARIATION(3, 		L"Empty")
	TEST_VARIATION(4, 		L"DeleteRows from seperate threads, with overlapping set of rows")
	TEST_VARIATION(5, 		L"Empty")
	TEST_VARIATION(6, 		L"Insert a row from seperate threads")
	TEST_VARIATION(7, 		L"Empty")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCRowsetOther)
//--------------------------------------------------------------------
// @class Multi-Threaded testing of the other Rowset variants
//
class TCRowsetOther : public CThreads { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCRowsetOther,CThreads);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember GetProperties from seperate threads
	int Variation_1();
	// @cmember GetProperties while Releasing the Rowset
	int Variation_2();
	// @cmember GetSpecification from seperate threads
	int Variation_3();
	// @cmember Empty
	int Variation_4();
	// @cmember Requery from seperate threads, V2
	int Variation_5();
	// @cmember Empty
	int Variation_6();
	// @cmember GetRowsAt with a set of overlapping rows from seperate threads
	int Variation_7();
	// @cmember GetRowsByBookmark with a set of overlapping rows from different threads
	int Variation_8();
	// @cmember Empty
	int Variation_9();
	// @cmember GetVisibleData while resyncing the rows
	int Variation_10();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCRowsetOther)
#define THE_CLASS TCRowsetOther
BEG_TEST_CASE(TCRowsetOther, CThreads, L"Multi-Threaded testing of the other Rowset variants")
	TEST_VARIATION(1, 		L"GetProperties from seperate threads")
	TEST_VARIATION(2, 		L"GetProperties while Releasing the Rowset")
	TEST_VARIATION(3, 		L"GetSpecification from seperate threads")
	TEST_VARIATION(4, 		L"Empty")
	TEST_VARIATION(5, 		L"Requery from seperate threads, V2")
	TEST_VARIATION(6, 		L"Empty")
	TEST_VARIATION(7, 		L"GetRowsAt with a set of overlapping rows from seperate threads")
	TEST_VARIATION(8, 		L"GetRowsByBookmark with a set of overlapping rows from different threads")
	TEST_VARIATION(9, 		L"Empty")
	TEST_VARIATION(10, 		L"GetVisibleData while resyncing the rows")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCRowsetUpdate)
//--------------------------------------------------------------------
// @class Multi-Threaded testing of the RowsetUpdate Object
//
class TCRowsetUpdate : public CThreads { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCRowsetUpdate,CThreads);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Empty
	int Variation_1();
	// @cmember Empty
	int Variation_2();
	// @cmember Empty
	int Variation_3();
	// @cmember GetPendingRows while GetRowStatus
	int Variation_4();
	// @cmember GetPendingrows while Update
	int Variation_5();
	// @cmember GetPendingRows while Undo
	int Variation_6();
	// @cmember Empty
	int Variation_7();
	// @cmember GetOriginalData while Update
	int Variation_8();
	// @cmember GetOriginalData while GetVisibleData
	int Variation_9();
	// @cmember Empty
	int Variation_10();
	// @cmember Undo while Update
	int Variation_11();
	// @cmember Empty
	int Variation_12();
	// @cmember Insert while Update
	int Variation_13();
	// @cmember Modify while Update
	int Variation_14();
	// @cmember Delete while Update
	int Variation_15();
	// @cmember Empty
	int Variation_16();
	// @cmember Insert while Undo
	int Variation_17();
	// @cmember Modify while Undo
	int Variation_18();
	// @cmember Delete while Undo
	int Variation_19();
	// @cmember Empty
	int Variation_20();
	// @cmember Update while Resynch
	int Variation_21();
	// @cmember Undo while Resynch
	int Variation_22();
	// @cmember Empty
	int Variation_23();
	// @cmember Requery while Updating
	int Variation_24();
	// @cmember Requerying while Undo
	int Variation_25();
	// @cmember Requery while GetOriginalData
	int Variation_26();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCRowsetUpdate)
#define THE_CLASS TCRowsetUpdate
BEG_TEST_CASE(TCRowsetUpdate, CThreads, L"Multi-Threaded testing of the RowsetUpdate Object")
	TEST_VARIATION(1, 		L"Empty")
	TEST_VARIATION(2, 		L"Empty")
	TEST_VARIATION(3, 		L"Empty")
	TEST_VARIATION(4, 		L"GetPendingRows while GetRowStatus")
	TEST_VARIATION(5, 		L"GetPendingrows while Update")
	TEST_VARIATION(6, 		L"GetPendingRows while Undo")
	TEST_VARIATION(7, 		L"Empty")
	TEST_VARIATION(8, 		L"GetOriginalData while Update")
	TEST_VARIATION(9, 		L"GetOriginalData while GetVisibleData")
	TEST_VARIATION(10, 		L"Empty")
	TEST_VARIATION(11, 		L"Undo while Update")
	TEST_VARIATION(12, 		L"Empty")
	TEST_VARIATION(13, 		L"Insert while Update")
	TEST_VARIATION(14, 		L"Modify while Update")
	TEST_VARIATION(15, 		L"Delete while Update")
	TEST_VARIATION(16, 		L"Empty")
	TEST_VARIATION(17, 		L"Insert while Undo")
	TEST_VARIATION(18, 		L"Modify while Undo")
	TEST_VARIATION(19, 		L"Delete while Undo")
	TEST_VARIATION(20, 		L"Empty")
	TEST_VARIATION(21, 		L"Update while Resynch")
	TEST_VARIATION(22, 		L"Undo while Resynch")
	TEST_VARIATION(23, 		L"Empty")
	TEST_VARIATION(24, 		L"Requery while Updating")
	TEST_VARIATION(25, 		L"Requerying while Undo")
	TEST_VARIATION(26, 		L"Requery while GetOriginalData")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCConnections)
//--------------------------------------------------------------------
// @class Multi-Threaded testing of the RowsetNotify  and related Objects
//
class TCConnections : public CThreads { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCConnections,CThreads);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Multiple Listeners on sep threads, ROW_ACTIVE sent to all threads on their own thread
	int Variation_1();
	// @cmember Multiple Listeners on sep threads, GetNextRows / Modify - all notifcations go to all threads on their own thread
	int Variation_2();
	// @cmember Empty
	int Variation_3();
	// @cmember Multiple Listeners on sep threads, as well as the main thread being a listener
	int Variation_4();
	// @cmember Empty
	int Variation_5();
	// @cmember Two listeners on the same thread, as well as other single threaded listeners
	int Variation_6();
	// @cmember Empty
	int Variation_7();
	// @cmember All apartment model listeners
	int Variation_8();
	// @cmember Some Apartment, Some Free-Threaded lsiteners
	int Variation_9();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCConnections)
#define THE_CLASS TCConnections
BEG_TEST_CASE(TCConnections, CThreads, L"Multi-Threaded testing of the RowsetNotify  and related Objects")
	TEST_VARIATION(1, 		L"Multiple Listeners on sep threads, ROW_ACTIVE sent to all threads on their own thread")
	TEST_VARIATION(2, 		L"Multiple Listeners on sep threads, GetNextRows / Modify - all notifcations go to all threads on their own thread")
	TEST_VARIATION(3, 		L"Empty")
	TEST_VARIATION(4, 		L"Multiple Listeners on sep threads, as well as the main thread being a listener")
	TEST_VARIATION(5, 		L"Empty")
	TEST_VARIATION(6, 		L"Two listeners on the same thread, as well as other single threaded listeners")
	TEST_VARIATION(7, 		L"Empty")
	TEST_VARIATION(8, 		L"All apartment model listeners")
	TEST_VARIATION(9, 		L"Some Apartment, Some Free-Threaded lsiteners")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCSequence)
//--------------------------------------------------------------------
// @class Multi-Threaded testing of the various Rowset Objects together
//
class TCSequence : public CThreads { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCSequence,CThreads);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember SetData multiple times
	int Variation_1();
	// @cmember SetData for different columns within the same row
	int Variation_2();
	// @cmember SetData and GetData for the same column/row
	int Variation_3();
	// @cmember Empty
	int Variation_4();
	// @cmember SetData and Resynch
	int Variation_5();
	// @cmember GetData and Resynch
	int Variation_6();
	// @cmember Empty
	int Variation_7();
	// @cmember Insert a row while GetNextRows
	int Variation_8();
	// @cmember Insert a row while GetData
	int Variation_9();
	// @cmember Insert a row while DeleteRows
	int Variation_10();
	// @cmember Empty
	int Variation_11();
	// @cmember Modify a row while GetNextRows
	int Variation_12();
	// @cmember Modify a row while GetData
	int Variation_13();
	// @cmember Modify a row while ReleaseRows
	int Variation_14();
	// @cmember Empty
	int Variation_15();
	// @cmember Delete a row, while GetNextRows
	int Variation_16();
	// @cmember Delete a row, while GetData
	int Variation_17();
	// @cmember Delete a row, while SetData
	int Variation_18();
	// @cmember Delete a row, while ReleaseRows
	int Variation_19();
	// @cmember Empty
	int Variation_20();
	// @cmember Requery while DeleteRows
	int Variation_21();
	// @cmember Requery while Modifying and row
	int Variation_22();
	// @cmember Requery while Inserting a row
	int Variation_23();
	// @cmember Requery while GetNextRows
	int Variation_24();
	// @cmember Requery while GetData
	int Variation_25();
	// @cmember Requery while RestartPosition (causing a requery
	int Variation_26();
	// @cmember Empty
	int Variation_27();
	// @cmember GetNextRows while Locate
	int Variation_28();
	// @cmember GetData while Locate
	int Variation_29();
	// @cmember RestartPosition while Locate
	int Variation_30();
	// @cmember Empty
	int Variation_31();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCSequence)
#define THE_CLASS TCSequence
BEG_TEST_CASE(TCSequence, CThreads, L"Multi-Threaded testing of the various Rowset Objects together")
	TEST_VARIATION(1, 		L"SetData multiple times")
	TEST_VARIATION(2, 		L"SetData for different columns within the same row")
	TEST_VARIATION(3, 		L"SetData and GetData for the same column/row")
	TEST_VARIATION(4, 		L"Empty")
	TEST_VARIATION(5, 		L"SetData and Resynch")
	TEST_VARIATION(6, 		L"GetData and Resynch")
	TEST_VARIATION(7, 		L"Empty")
	TEST_VARIATION(8, 		L"Insert a row while GetNextRows")
	TEST_VARIATION(9, 		L"Insert a row while GetData")
	TEST_VARIATION(10, 		L"Insert a row while DeleteRows")
	TEST_VARIATION(11, 		L"Empty")
	TEST_VARIATION(12, 		L"Modify a row while GetNextRows")
	TEST_VARIATION(13, 		L"Modify a row while GetData")
	TEST_VARIATION(14, 		L"Modify a row while ReleaseRows")
	TEST_VARIATION(15, 		L"Empty")
	TEST_VARIATION(16, 		L"Delete a row, while GetNextRows")
	TEST_VARIATION(17, 		L"Delete a row, while GetData")
	TEST_VARIATION(18, 		L"Delete a row, while SetData")
	TEST_VARIATION(19, 		L"Delete a row, while ReleaseRows")
	TEST_VARIATION(20, 		L"Empty")
	TEST_VARIATION(21, 		L"Requery while DeleteRows")
	TEST_VARIATION(22, 		L"Requery while Modifying and row")
	TEST_VARIATION(23, 		L"Requery while Inserting a row")
	TEST_VARIATION(24, 		L"Requery while GetNextRows")
	TEST_VARIATION(25, 		L"Requery while GetData")
	TEST_VARIATION(26, 		L"Requery while RestartPosition (causing a requery")
	TEST_VARIATION(27, 		L"Empty")
	TEST_VARIATION(28, 		L"GetNextRows while Locate")
	TEST_VARIATION(29, 		L"GetData while Locate")
	TEST_VARIATION(30, 		L"RestartPosition while Locate")
	TEST_VARIATION(31, 		L"Empty")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCClassFactory)
//--------------------------------------------------------------------
// @class Multi-Threaded testing of the ClassFactory object
//
class TCClassFactory : public CThreads { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCClassFactory,CThreads);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Call CoCreateobject instead of CoCreateInstance
	int Variation_1();
	// @cmember Empty
	int Variation_2();
	// @cmember PersistFile save, load from sep threads
	int Variation_3();
	// @cmember Extended Error object
	int Variation_4();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCClassFactory)
#define THE_CLASS TCClassFactory
BEG_TEST_CASE(TCClassFactory, CThreads, L"Multi-Threaded testing of the ClassFactory object")
	TEST_VARIATION(1, 		L"Call CoCreateobject instead of CoCreateInstance")
	TEST_VARIATION(2, 		L"Empty")
	TEST_VARIATION(3, 		L"PersistFile save, load from sep threads")
	TEST_VARIATION(4, 		L"Extended Error object")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCRealWorld)
//--------------------------------------------------------------------
// @class Real-World MultiThreaded senarios for OLEDB
//
class TCRealWorld : public CThreads { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCRealWorld,CThreads);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Browser app, reading multiple columns concurrently
	int Variation_1();
	// @cmember On-Line database
	int Variation_2();
	// @cmember Multiple Querys on the same Connection Object
	int Variation_3();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCRealWorld)
#define THE_CLASS TCRealWorld
BEG_TEST_CASE(TCRealWorld, CThreads, L"Real-World MultiThreaded senarios for OLEDB")
	TEST_VARIATION(1, 		L"Browser app, reading multiple columns concurrently")
	TEST_VARIATION(2, 		L"On-Line database")
	TEST_VARIATION(3, 		L"Multiple Querys on the same Connection Object")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCMultipleResult)
//--------------------------------------------------------------------
// @class Multi-Threaded testing for IMultipleResults
//
class TCMultipleResult : public CThreads { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCMultipleResult,CThreads);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember GetResults from seperate threads
	int Variation_1();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCMultipleResult)
#define THE_CLASS TCMultipleResult
BEG_TEST_CASE(TCMultipleResult, CThreads, L"Multi-Threaded testing for IMultipleResults")
	TEST_VARIATION(1, 		L"GetResults from seperate threads")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// }} END_DECLARE_TEST_CASES()

// {{ TCW_TESTMODULE(ThisModule)
TEST_MODULE(16, ThisModule, gwszModuleDescrip)
	TEST_CASE(1, TCDataSource)
	TEST_CASE(2, TCSession)
	TEST_CASE(3, TCCommand)
	TEST_CASE(4, TCAccessor)
	TEST_CASE(5, TCError)
	TEST_CASE(6, TCTransaction)
	TEST_CASE(7, TCOpenRowset)
	TEST_CASE(8, TCRowset)
	TEST_CASE(9, TCRowsetChange)
	TEST_CASE(10, TCRowsetOther)
	TEST_CASE(11, TCRowsetUpdate)
	TEST_CASE(12, TCConnections)
	TEST_CASE(13, TCSequence)
	TEST_CASE(14, TCClassFactory)
	TEST_CASE(15, TCRealWorld)
	TEST_CASE(16, TCMultipleResult)
END_TEST_MODULE()
// }} TCW_TESTMODULE_END


// {{ TCW_TC_PROTOTYPE(TCDataSource)
//*-----------------------------------------------------------------------
//| Test Case:		TCDataSource - Multi-Threaded testing of the DataSourceObject
//|	Created:			05/21/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCDataSource::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CThreads::Init())
	// }}
	{
		if(CreateRowset(DBPROP_CANHOLDROWS, SELECT_VALIDATIONORDER)==S_OK)
			return TRUE;
	}

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc GetDataSource threading model CLSID
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDataSource::Variation_1()
{
	TBEGIN
	ASSERT(m_pIDBInitialize);
	ULONG_PTR ulValue = 0;

	//Get ThreadingModel
	TESTC(VerifyThreadingModel())
		
	//Get DataSource ThreadingModel
	TESTC_PROVIDER(::GetProperty(DBPROP_DSOTHREADMODEL, DBPROPSET_DATASOURCEINFO, m_pIDBInitialize, &ulValue));
	//Threading Property is one of the following, not Anded!
	TESTC(ulValue == DBPROPVAL_RT_FREETHREAD || ulValue == DBPROPVAL_RT_APTMTTHREAD || ulValue == DBPROPVAL_RT_SINGLETHREAD);

CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc GetProperties and SetProperites on the same DSO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDataSource::Variation_2()
{
	TBEGIN
	INIT_THREADS(MAX_THREADS);
	ULONG cPropSets = 0;
	DBPROPSET* rgPropSets = NULL;
	IDBProperties* pIDBProperties = NULL;

	//Build our init options from string passed to us from the InitString
	GetInitProps(&cPropSets, &rgPropSets);

	//Create another instance, with properties set, (not initialized)
	CreateNewDSO(NULL, IID_IDBProperties,(IUnknown**)&pIDBProperties, CREATEDSO_NONE);

	//Setup thread arguments
	THREADARG T1Arg = { pIDBProperties, 0, NULL }; //All Properties
	THREADARG T2Arg = { pIDBProperties, (void*)cPropSets, rgPropSets };

	//Half of the threads GetProperties, the other half SetProperties
	CREATE_FIRST_THREADS(Thread_IDBGetProperties, &T1Arg);
	CREATE_SECOND_THREADS(Thread_IDBSetProperties, &T2Arg);

	START_THREADS();
	END_THREADS();	   

//CLEANUP:
	::FreeProperties(&cPropSets,&rgPropSets);
	SAFE_RELEASE(pIDBProperties);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc GetProperties on the same set of properties
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDataSource::Variation_3()
{
	TBEGIN
	INIT_THREADS(MAX_THREADS);
	ULONG cPropIDSets = 0;
	DBPROPIDSET* rgPropIDSets = NULL;
	IDBProperties* pIDBProperties = NULL;

	//Create another instance, with properties set, (not initialized)
	CreateNewDSO(NULL, IID_IDBProperties, (IUnknown**)&pIDBProperties, CREATEDSO_SETPROPERTIES);

	//DBPROP_INIT_PROMPT is a required level 0 property
	if(SupportedProperty(DBPROP_INIT_PROMPT,DBPROPSET_DBINIT))
		::SetProperty(DBPROP_INIT_PROMPT, DBPROPSET_DBINIT, &cPropIDSets, &rgPropIDSets);

	//Add DataSoruce is supported
	if(SupportedProperty(DBPROP_INIT_DATASOURCE,DBPROPSET_DBINIT))
		::SetProperty(DBPROP_INIT_DATASOURCE,DBPROPSET_DBINIT, &cPropIDSets, &rgPropIDSets);

	//Setup thread arguments
	THREADARG T1Arg = { pIDBProperties, (void*)cPropIDSets, rgPropIDSets };
	THREADARG T2Arg = { pIDBProperties, 0, NULL }; //All Properties

	//Half of the threads are before Initialization, the other hald is after Initialization
	CREATE_FIRST_THREADS(Thread_IDBGetProperties, &T1Arg);
	CREATE_SECOND_THREADS(Thread_IDBGetProperties, &T2Arg);

	START_THREADS();
	END_THREADS();	   

//CLEANUP:
	::FreeProperties(&cPropIDSets,&rgPropIDSets);
	SAFE_RELEASE(pIDBProperties);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc GetPropertyInfo on all properties supported
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDataSource::Variation_4()
{
	TBEGIN
	INIT_THREADS(MAX_THREADS);
	IDBProperties* pIDBProperties1 = NULL;
	IDBProperties* pIDBProperties2 = NULL;

	//Create another instance, with properties set, (not initialized)
	CreateNewDSO(NULL, IID_IDBProperties, (IUnknown**)&pIDBProperties1, CREATEDSO_SETPROPERTIES);
	CreateNewDSO(NULL, IID_IDBProperties, (IUnknown**)&pIDBProperties2, CREATEDSO_SETPROPERTIES | CREATEDSO_INITIALIZE);

	//Setup thread arguments
	THREADARG T1Arg = { pIDBProperties1, 0, NULL }; //Before Initialization
	THREADARG T2Arg = { pIDBProperties2, 0, NULL }; //After Initialization
													   
	//Half of the threads are before Initialization, the other hald is after Initialization
	CREATE_FIRST_THREADS(Thread_IDBGetPropertyInfo, &T1Arg);
	CREATE_SECOND_THREADS(Thread_IDBGetPropertyInfo, &T2Arg);

	START_THREADS();
	END_THREADS();	

//CLEANUP:
	SAFE_RELEASE(pIDBProperties1);
	SAFE_RELEASE(pIDBProperties2);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc SetProperties on the same DSO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDataSource::Variation_5()
{
	TBEGIN
	INIT_THREADS(MAX_THREADS);
	ULONG cPropSets = 0;
	DBPROPSET* rgPropSets = NULL;
	IDBProperties* pIDBProperties = NULL;

	//Build our init options from string passed to us from the InitString
	GetInitProps(&cPropSets, &rgPropSets);

	//Create another instance, with properties set, (not initialized)
	CreateNewDSO(NULL, IID_IDBProperties,(IUnknown**)&pIDBProperties, CREATEDSO_NONE);

	//Setup thread arguments
	THREADARG T1Arg = { pIDBProperties, (void*)cPropSets, rgPropSets };

	CREATE_THREADS(Thread_IDBSetProperties, &T1Arg);
	START_THREADS();
	END_THREADS();	   

//CLEANUP:
	::FreeProperties(&cPropSets,&rgPropSets);
	SAFE_RELEASE(pIDBProperties);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Initialize the DSO from sep threads
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDataSource::Variation_6()
{
	TBEGIN
	INIT_THREADS(MAX_THREADS); //number of threads...
	IDBInitialize* pIDBInitialize = NULL; 

	//Create another instance, with properties set, (not initialized)
	CreateNewDSO(NULL, IID_IDBInitialize,(IUnknown**)&pIDBInitialize, CREATEDSO_SETPROPERTIES);

	//Setup thread arguments
	THREADARG T1Arg = { pIDBInitialize, (void*)DB_E_ALREADYINITIALIZED };
	TESTC(pIDBInitialize!=NULL);
	
	//Create Thread
	CREATE_THREADS(Thread_IDBInitialize, &T1Arg);
	START_THREADS();
	END_THREADS();	

CLEANUP:
	SAFE_RELEASE(pIDBInitialize);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Unitilize the DSO from sep threads
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDataSource::Variation_7()
{
	TBEGIN
	INIT_THREADS(MAX_THREADS);
	IDBInitialize* pIDBInitialize = NULL; 

	//Create another instance, since our current one from the privlib has too many
	//references to uninitilize
	CreateNewDSO(NULL, IID_IDBInitialize,(IUnknown**)&pIDBInitialize, CREATEDSO_SETPROPERTIES | CREATEDSO_INITIALIZE);

	//Setup thread arguments
	THREADARG T1Arg = { pIDBInitialize, (void*)S_OK };
	TESTC(pIDBInitialize!=NULL);

	CREATE_THREADS(Thread_IDBUninitialize, &T1Arg);
	START_THREADS();
	END_THREADS();	

CLEANUP:
	SAFE_RELEASE(pIDBInitialize);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc Initialize / Unitialize same DSO from sep threads
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCDataSource::Variation_8()
{ 
	TBEGIN
	INIT_THREADS(MAX_THREADS);	//number of threads...
	IDBInitialize* pIDBInitialize = NULL; 

	//Create another instance, with properties set, (not initialized)
	CreateNewDSO(NULL, IID_IDBInitialize,(IUnknown**)&pIDBInitialize, CREATEDSO_SETPROPERTIES);

	//Setup thread arguments
	THREADARG T1Arg = { pIDBInitialize, (void*)DB_E_ALREADYINITIALIZED };
	THREADARG T2Arg = { pIDBInitialize, (void*)S_OK };
	TESTC(pIDBInitialize!=NULL);

	//Half of the threads are before Initialization, the other hald is after Initialization
	CREATE_FIRST_THREADS(Thread_IDBInitialize, &T1Arg);
	CREATE_SECOND_THREADS(Thread_IDBUninitialize, &T2Arg);

	START_THREADS();
	END_THREADS();	

CLEANUP:
	SAFE_RELEASE(pIDBInitialize);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc Initialize seperate DSOs in threads
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCDataSource::Variation_9()
{ 
	TBEGIN
	INIT_THREADS(MAX_THREADS); //number of threads...
	ULONG cPropSets = 0;
	DBPROPSET* rgPropSets = NULL;

	//Build our init options from string passed to us from the InitString
	GetInitProps(&cPropSets, &rgPropSets);

	//Setup thread arguments
	//We allow E_FAIL, since this number of threads, may be over the number of connections
	//allowed by the DataSource and currently there is no other return code to indicate this
	THREADARG T1Arg = { NULL, (void*)cPropSets, rgPropSets, (void*)DB_SEC_E_AUTH_FAILED, (void*)E_FAIL };
	
	//Create Thread
	CREATE_THREADS(Thread_NewDSOInitialize, &T1Arg);
	START_THREADS();
	END_THREADS();	

	::FreeProperties(&cPropSets, &rgPropSets);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCDataSource::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CThreads::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCSession)
//*-----------------------------------------------------------------------
//| Test Case:		TCSession - Multi-Threaded testing of the SessionObject
//|	Created:			05/21/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCSession::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CThreads::Init())
	// }}
	{
		if(CreateRowset(DBPROP_CANHOLDROWS, SELECT_VALIDATIONORDER)==S_OK)
			return TRUE;
	}

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Get the Session Threading model CLSID
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSession::Variation_1()
{
	TBEGIN

	//Get ThreadingModel
	TESTC(VerifyThreadingModel())
	
CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Get the Session threading model Property
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSession::Variation_2()
{
	TBEGIN
	ASSERT(m_pIDBInitialize);
	ULONG_PTR ulValue = 0;

	//Get ThreadingModel
	TESTC_PROVIDER(::GetProperty(DBPROP_DSOTHREADMODEL, DBPROPSET_DATASOURCEINFO, m_pIDBInitialize, &ulValue));
	//Threading Property is one of the following, not Anded!
	TESTC(ulValue == DBPROPVAL_RT_FREETHREAD || ulValue == DBPROPVAL_RT_APTMTTHREAD || ulValue == DBPROPVAL_RT_SINGLETHREAD);
	
CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc CreateSession from sep threads
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSession::Variation_3()
{
	TBEGIN
	INIT_THREADS(THREE_THREADS);
	ULONG_PTR ulMaxSessions = 0;
	HRESULT hrExpected = S_OK;

	//Determine the Maximum Number of Actiave Sessions
	::GetProperty(DBPROP_ACTIVESESSIONS, DBPROPSET_DATASOURCEINFO, g_pIDBCreateSession, &ulMaxSessions);
	if(ulMaxSessions!=0 && ulMaxSessions < THREE_THREADS+1)
		hrExpected = DB_E_OBJECTCREATIONLIMITREACHED;

	//Setup thread arguments
	THREADARG T1Arg = { this, (void*)hrExpected };

	CREATE_THREADS(Thread_IDBCreateSession, &T1Arg);
	
	START_THREADS();
	END_THREADS();	
	
//CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc CreateCommand from seperate threads
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSession::Variation_4()
{
	
	TBEGIN
	INIT_THREADS(MAX_THREADS);

	//Setup thread arguments
	THREADARG T1Arg = { this };

	CREATE_THREADS(Thread_IDBCreateCommand,&T1Arg);
	
	START_THREADS();
	END_THREADS();	
	
//CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSession::Variation_5()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Add a column / drop a column from sep threads
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSession::Variation_6()
{
	
	TBEGIN
//	INIT_THREADS(MAX_THREADS);

	//Setup thread arguments
	THREADARG T1Arg = { this };

	//TODO V2, AddColumn/DropColumn not supported yet
/*
	CREATE_FIRST_THREADS(Thread_AddColumn,&T1Arg);
	CREATE_SECOND_THREADS(Thread_DropColumn,&T1Arg);
	START_THREADS();
	END_THREADS();	
*/	
//CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Create a table from seperate threads
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSession::Variation_7()
{
	
	TBEGIN
//	INIT_THREADS(MAX_THREADS);

	//Setup thread arguments
	THREADARG T1Arg = { this };

	//TODO V2, CreateTable not supported yet
/*
	CREATE_THREADS(Thread_IDBCreateTable,&T1Arg);
	START_THREADS();
	END_THREADS();	
*/
//CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSession::Variation_8()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc Get schema rowset from seperate threads
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSession::Variation_9()
{
	TBEGIN
	INIT_THREADS(MAX_THREADS);

	//Setup thread arguments
	THREADARG T1Arg = { this };

	//IDBSchemaRowset is optional
	TESTC_PROVIDER(QI(pISession(),IID_IDBSchemaRowset)==S_OK);

	CREATE_THREADS(Thread_GetSchemaRowset,&T1Arg);
	
	START_THREADS();
	END_THREADS();	
	
CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc Get schema info from sep threads
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSession::Variation_10()
{
	TBEGIN
	INIT_THREADS(MAX_THREADS);

	//Setup thread arguments
	THREADARG T1Arg = { this };

	//IDBSchemaRowset is optional
	TESTC_PROVIDER(QI(pISession(),IID_IDBSchemaRowset)==S_OK);

	CREATE_THREADS(Thread_GetSchemaInfo,&T1Arg);
	
	START_THREADS();
	END_THREADS();	
	
CLEANUP:
	TRETURN
}
// }}
// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCSession::Terminate()
{
	

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CThreads::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCCommand)
//*-----------------------------------------------------------------------
//| Test Case:		TCCommand - Multi-Threaded testing of the CommandObject
//|	Created:			05/21/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCCommand::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CThreads::Init())
	// }}
	{
		//Skip if Commands are not uspported..
		TEST_PROVIDER(GetCommandSupport());

		//Create the CommandObject and Set the CommandText
		//For the rest of the Variations in this testcase...
		if(CreateCommand(SELECT_ALLFROMTBL)==S_OK)
			return TRUE;
	}

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Get the command threading model CLSID
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommand::Variation_1()
{
	
	TBEGIN

	//Get ThreadingModel
	TESTC(VerifyThreadingModel())
	
CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Get the command threading model Property
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommand::Variation_2()
{
	
	TBEGIN
	ASSERT(m_pIDBInitialize);
	ULONG_PTR ulValue =  0;

	//Get ThreadingModel
	TESTC_PROVIDER(::GetProperty(DBPROP_DSOTHREADMODEL, DBPROPSET_DATASOURCEINFO, m_pIDBInitialize, &ulValue));
	//Threading Property is one of the following, not Anded!
	TESTC(ulValue == DBPROPVAL_RT_FREETHREAD || ulValue == DBPROPVAL_RT_APTMTTHREAD || ulValue == DBPROPVAL_RT_SINGLETHREAD);
	
CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Create command objects on seperate threads
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommand::Variation_3()
{
	
	TBEGIN
	INIT_THREADS(MAX_THREADS);

	//Setup thread arguments
	THREADARG T1Arg = { this };

	CREATE_THREADS(Thread_IDBCreateCommand,&T1Arg);
	
	START_THREADS();
	END_THREADS();	
	
//CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Call execute from sep threads, (same command object
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommand::Variation_4()
{
	
	TBEGIN
	INIT_THREADS(MAX_THREADS);

	//Setup thread arguments
	THREADARG T1Arg = { this, (void*)S_OK };

	CREATE_THREADS(Thread_CommandExecute,&T1Arg);
	
	START_THREADS();
	END_THREADS();	
	
//CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Call cancel while two executes are occurring
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommand::Variation_5()
{
	TBEGIN
	INIT_THREADS(THREE_THREADS);

	//Setup thread arguments
	THREADARG T1Arg = { this, (void*)DB_E_CANCELED };
	THREADARG T2Arg = { this, (void*)DB_E_CANTCANCEL };

	CREATE_THREAD(THREAD_ONE,   Thread_CommandExecute,	&T1Arg);
	CREATE_THREAD(THREAD_TWO,   Thread_CommandExecute,	&T1Arg);
	CREATE_THREAD(THREAD_THREE, Thread_CommandCancel,	&T2Arg);
	
	START_THREADS();
	END_THREADS();	
	
//CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommand::Variation_6()
{
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Set/Unset diff prop
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommand::Variation_7()
{
	TBEGIN
	INIT_THREADS(MAX_THREADS);
	CRowset RowsetA;

	//Set properties
	SetSettableProperty(DBPROP_CANHOLDROWS);
	SetSettableProperty(DBPROP_IRowsetUpdate, DBPROPSET_ROWSET,(void*)VARIANT_FALSE,DBTYPE_BOOL);

	RowsetA.SetSettableProperty(DBPROP_BOOKMARKS);
	RowsetA.SetSettableProperty(DBPROP_IRowsetChange,DBPROPSET_ROWSET,(void*)VARIANT_FALSE,DBTYPE_BOOL);

	//Setup thread arguments
	THREADARG T1Arg = { &RowsetA, (void*)m_cPropSets, m_rgPropSets };
	THREADARG T2Arg = { &RowsetA, (void*)RowsetA.m_cPropSets, RowsetA.m_rgPropSets };
	TESTC_(RowsetA.CreateCommand(SELECT_ALLFROMTBL),S_OK);

	CREATE_FIRST_THREADS(Thread_SetCommandProperties,&T1Arg);
	CREATE_SECOND_THREADS(Thread_SetCommandProperties,&T2Arg);

	START_THREADS();
	END_THREADS();	

CLEANUP:
	FreeProperties();
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc Set/Unset same prop
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommand::Variation_8()
{
	TBEGIN
	INIT_THREADS(MAX_THREADS);
		
	SetSettableProperty(DBPROP_CANHOLDROWS);
	SetSettableProperty(DBPROP_IRowsetUpdate, DBPROPSET_ROWSET,(void*)VARIANT_FALSE,DBTYPE_BOOL);

	CRowset RowsetA;

	RowsetA.SetSettableProperty(DBPROP_CANHOLDROWS);
	RowsetA.SetSettableProperty(DBPROP_IRowsetUpdate,DBPROPSET_ROWSET,(void*)VARIANT_FALSE,DBTYPE_BOOL);

	//Setup thread arguments
	THREADARG T1Arg = { &RowsetA, (void*)m_cPropSets, m_rgPropSets };
	THREADARG T2Arg = { &RowsetA, (void*)RowsetA.m_cPropSets, RowsetA.m_rgPropSets };
	TESTC_(RowsetA.CreateCommand(SELECT_ALLFROMTBL),S_OK);

	CREATE_FIRST_THREADS(Thread_SetCommandProperties,&T1Arg);
	CREATE_SECOND_THREADS(Thread_SetCommandProperties,&T2Arg);

	START_THREADS();
	END_THREADS();	

CLEANUP:
	FreeProperties();
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc Get/Set prop from sep threads
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommand::Variation_9()
{
	TBEGIN
	INIT_THREADS(MAX_THREADS);

	CRowset RowsetA;

	RowsetA.SetSettableProperty(DBPROP_CANHOLDROWS);
	RowsetA.SetSettableProperty(DBPROP_IRowsetUpdate, DBPROPSET_ROWSET, (void*)VARIANT_TRUE, DBTYPE_BOOL);

	ULONG cPropIDSets = 0;
	DBPROPIDSET* rgPropIDSets = NULL;
	if(SettableProperty(DBPROP_IRowsetUpdate,DBPROPSET_ROWSET))
		::SetProperty(DBPROP_IRowsetUpdate,DBPROPSET_ROWSET,&cPropIDSets,&rgPropIDSets);

	//Setup thread arguments
	THREADARG T1Arg = { &RowsetA, (void*)RowsetA.m_cPropSets, RowsetA.m_rgPropSets };
	THREADARG T2Arg = { &RowsetA, (void*)cPropIDSets, rgPropIDSets };
	TESTC_(RowsetA.CreateCommand(SELECT_ALLFROMTBL),S_OK);

	CREATE_FIRST_THREADS(Thread_SetCommandProperties,&T1Arg);
	CREATE_SECOND_THREADS(Thread_GetCommandProperties,&T2Arg);

	START_THREADS();
	END_THREADS();	

CLEANUP:
	RowsetA.FreeProperties();
	::FreeProperties(&cPropIDSets,&rgPropIDSets);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc Set the command text / Get text from another thread
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommand::Variation_10()
{
	TBEGIN
	INIT_THREADS(MAX_THREADS);
	
	WCHAR* pwszCommandText = NULL;

	//Obtain the CommandText for "SELECT * FROM <table>"
	pTable()->CreateSQLStmt(USE_SUPPORTED_SELECT_ALLFROMTBL,NULL,&pwszCommandText);

	//Setup thread arguments
	THREADARG T1Arg = { this, pwszCommandText };
	THREADARG T2Arg = { this };

	TESTC(pwszCommandText!=NULL)

	CREATE_FIRST_THREADS(Thread_SetCommandText,&T1Arg);
	CREATE_SECOND_THREADS(Thread_GetCommandText,&T2Arg);

	START_THREADS();
	END_THREADS();	

CLEANUP:
	PROVIDER_FREE(pwszCommandText);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommand::Variation_11()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc Uprepare / Prepare
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommand::Variation_12()
{
	
	TBEGIN
	INIT_THREADS(THREE_THREADS);
	
	//Setup thread arguments
	THREADARG T1Arg = { this };
	THREADARG T2Arg = { this, (void*)S_OK };

	CREATE_THREAD(THREAD_ONE,   Thread_CommandPrepare,  &T1Arg);
	CREATE_THREAD(THREAD_TWO,   Thread_CommandPrepare,  &T1Arg);
	CREATE_THREAD(THREAD_THREE, Thread_CommandUnPrepare,&T2Arg);

	START_THREADS();
	END_THREADS();	

//CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc Unprepare / Execute
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommand::Variation_13()
{
	
	TBEGIN
	INIT_THREADS(THREE_THREADS);

	//Setup thread arguments
	THREADARG T1Arg = { this, (void*)S_OK };
	THREADARG T2Arg = { this, (void*)DB_E_OBJECTOPEN };

	//Now try and Execute the Text and Unprepare
	CREATE_THREAD(THREAD_ONE,   Thread_CommandExecute,	&T1Arg);
	CREATE_THREAD(THREAD_TWO,   Thread_CommandUnPrepare,&T2Arg);
	CREATE_THREAD(THREAD_THREE, Thread_CommandUnPrepare,&T2Arg);

	START_THREADS();
	END_THREADS();	

//CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommand::Variation_14()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc SetDefaultValues from sep threads, GetDefaultValues from a third
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommand::Variation_15()
{
	
	TBEGIN
	INIT_THREADS(THREE_THREADS);
		
	//Setup thread arguments
	THREADARG T1Arg = { this };

	CREATE_THREAD(THREAD_ONE,   Thread_SetDefaultValues,&T1Arg);
	CREATE_THREAD(THREAD_TWO,   Thread_SetDefaultValues,&T1Arg);
	CREATE_THREAD(THREAD_THREE, Thread_GetDefaultValues,&T1Arg);

	START_THREADS();
	END_THREADS();	

//CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommand::Variation_16()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc Execute / Release the command object
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommand::Variation_17()
{
	TBEGIN
	INIT_THREADS(FOUR_THREADS);
		
	//Setup thread arguments
	THREADARG T1Arg = { this, (void*)S_OK };
	THREADARG T2Arg = { pICommand() };
	THREADARG T3Arg = { pICommand() };
	THREADARG T4Arg = { this, (void*)DB_E_OBJECTOPEN };

	//AddRef an extra time, just in case Release runs faster
	pICommand()->AddRef();

	CREATE_THREAD(THREAD_ONE,   Thread_CommandExecute,&T1Arg);
	CREATE_THREAD(THREAD_TWO,   Thread_Release,&T2Arg);
	CREATE_THREAD(THREAD_THREE, Thread_AddRef,&T3Arg);
	CREATE_THREAD(THREAD_FOUR,  Thread_CommandUnPrepare,&T4Arg);

	START_THREADS();
	END_THREADS();	

//CLEANUP:
	//Release the extra time
	pICommand()->Release();
	TRETURN
}
// }}
// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCCommand::Terminate()
{
	

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CThreads::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCAccessor)
//*-----------------------------------------------------------------------
//| Test Case:		TCAccessor - Multi-Threaded testing of the Accessor object
//|	Created:			05/21/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCAccessor::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CThreads::Init())
	// }}
	{
		if(CreateRowset(DBPROP_CANHOLDROWS, SELECT_VALIDATIONORDER)==S_OK)
			return TRUE;
	}

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Create multiple accessors from diff threads
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCAccessor::Variation_1()
{
	
	TBEGIN
	INIT_THREADS(MAX_THREADS);
	
	//Setup thread arguments
	THREADARG T1Arg = { this };

	CREATE_THREADS(Thread_CreateAccessor,&T1Arg);

	START_THREADS();
	END_THREADS();	

//CLEANUP:
	TRETURN
}
// }}



// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Use the same accessor on diff threads
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCAccessor::Variation_2()
{
	TBEGIN
	INIT_THREADS(MAX_THREADS);	
	
	CRowset RowsetA;
	DBCOUNTITEM cRowsObtained = 0;
	HROW* rghRows = NULL;
	HRESULT hr = S_OK;
	THREADARG T1Arg;
		
	//CreateRowset
	TESTC_PROVIDER(RowsetA.CreateRowset(SELECT_VALIDATIONORDER)==S_OK);

	//Obtain all row handles (as many row handles as possible)
	TEST3C_(hr = RowsetA.GetNextRows(0, 1000, &cRowsObtained, &rghRows), DB_S_ENDOFROWSET, DB_S_ROWLIMITEXCEEDED, E_OUTOFMEMORY);
	TESTC_PROVIDER(SUCCEEDED(hr));
	
	//Setup Thread Arguments
	T1Arg = InitThreadArg(&RowsetA, (void*)cRowsObtained, rghRows, &RowsetA.m_hAccessor, (void*)S_OK);

	//Create Threads
	CREATE_THREADS(Thread_GetData, &T1Arg);

	START_THREADS();
	END_THREADS();	

CLEANUP:
	RowsetA.ReleaseRows(cRowsObtained, rghRows);
	PROVIDER_FREE(rghRows);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCAccessor::Variation_3()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc GetBindiings / ReleaseAcessor
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCAccessor::Variation_4()
{
	TBEGIN
	INIT_THREADS(TWO_THREADS);
	
	HACCESSOR hAccessor = NULL;
	CRowset RowsetA;

	//Setup Thread Arguments
	THREADARG T1Arg = { &RowsetA, &hAccessor, (void*)DB_E_BADACCESSORHANDLE };
	THREADARG T2Arg = { &RowsetA, &hAccessor };
	
	//CreateRowset
	TESTC_PROVIDER(RowsetA.CreateRowset(SELECT_VALIDATIONORDER)==S_OK);
	
	//Create another accessor
	TESTC_(GetAccessorAndBindings(RowsetA.pIRowset(),DBACCESSOR_ROWDATA,&hAccessor,
		NULL,NULL,NULL,	DBPART_ALL, ALL_COLS_BOUND),S_OK);

	//Create Threads
	CREATE_FIRST_THREADS(Thread_GetBindings,&T1Arg);
	CREATE_SECOND_THREADS(Thread_ReleaseAccessor,&T2Arg);

	START_THREADS();
	END_THREADS();	

CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc GetColumnsInfo / IRowsetInfo
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCAccessor::Variation_5()
{
	TBEGIN
	INIT_THREADS(MAX_THREADS);	
	
	CRowset RowsetA;

	//Setup Thread Arguments
	THREADARG T1Arg = { &RowsetA };
	THREADARG T2Arg = { &RowsetA, 0, NULL };

	//CreateRowset
	TESTC_PROVIDER(RowsetA.CreateRowset(SELECT_VALIDATIONORDER)==S_OK);

	//Create Threads
	CREATE_FIRST_THREADS(Thread_GetColumnsInfo,&T1Arg);
	CREATE_SECOND_THREADS(Thread_GetRowsetProperties,&T2Arg);

	START_THREADS();
	END_THREADS();	

CLEANUP:
	TRETURN
}
// }}
// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCAccessor::Terminate()
{
	

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CThreads::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCError)
//*-----------------------------------------------------------------------
//| Test Case:		TCError - Multi-Threaded testing of the ErrorObject
//|	Created:			05/21/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCError::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CThreads::Init())
	// }}
	{
		if(CreateRowset(DBPROP_CANHOLDROWS, SELECT_VALIDATIONORDER)==S_OK)
			return TRUE;
	}

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Get the Error object threading model CLSID
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCError::Variation_1()
{
	
	TBEGIN

	//Get ThreadingModel
	TESTC(VerifyThreadingModel())
	
CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Get the Error object threading model Property
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCError::Variation_2()
{
	
	TBEGIN
	ASSERT(m_pIDBInitialize);
	ULONG_PTR ulValue = 0;

	//Get ThreadingModel
	TESTC_PROVIDER(::GetProperty(DBPROP_DSOTHREADMODEL, DBPROPSET_DATASOURCEINFO, m_pIDBInitialize, &ulValue));
	//Threading Property is one of the following, not Anded!
	TESTC(ulValue == DBPROPVAL_RT_FREETHREAD || ulValue == DBPROPVAL_RT_APTMTTHREAD || ulValue == DBPROPVAL_RT_SINGLETHREAD);
	
CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc AddErrorRecord from sep threads, GetErrorInfo from a third
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCError::Variation_3()
{
	
	TBEGIN
	INIT_THREADS(THREE_THREADS);	
	
	CRowset RowsetA;

	//Setup Thread Arguments
	THREADARG T1Arg = { &RowsetA };
	THREADARG T2Arg = { &RowsetA };
	THREADARG T3Arg = { &RowsetA };

	//CreateRowset
	TESTC_PROVIDER(RowsetA.CreateRowset(SELECT_VALIDATIONORDER)==S_OK);

	//Create Threads
	CREATE_THREAD(THREAD_ONE,   Thread_AddErrorRecord,&T1Arg);
	CREATE_THREAD(THREAD_TWO,   Thread_AddErrorRecord,&T2Arg);
	CREATE_THREAD(THREAD_THREE, Thread_GetErrorInfo,&T3Arg);

	START_THREADS();
	END_THREADS();	

CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Transfer error record to antoher thread.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCError::Variation_4()
{
	

	//TODO

	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Apartment model - make sure apartment model error object is returned
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCError::Variation_5()
{
	

	//TODO V2

	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCError::Variation_6()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Generate Errors from sep threads
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCError::Variation_7()
{
	

	//TODO

	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc Use diff threads to make IErrorInfo calls
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCError::Variation_8()
{
	
	TBEGIN
	INIT_THREADS(THREE_THREADS);	
	
	CRowset RowsetA;

	//Setup Thread Arguments
	THREADARG T1Arg = { &RowsetA };
	THREADARG T2Arg = { &RowsetA };
	THREADARG T3Arg = { &RowsetA };

	//CreateRowset
	TESTC_PROVIDER(RowsetA.CreateRowset(SELECT_VALIDATIONORDER)==S_OK);

	//Create Threads
	CREATE_THREAD(THREAD_ONE,   Thread_AddErrorRecord,&T1Arg);
	CREATE_THREAD(THREAD_TWO,   Thread_AddErrorRecord,&T2Arg);
	CREATE_THREAD(THREAD_THREE, Thread_GetErrorInfo,&T3Arg);

	START_THREADS();
	END_THREADS();	

CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc Create Error object on 1 thread, set error info from many other threads
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCError::Variation_9()
{
	
	
	//TODO

	return TEST_PASS;
}
// }}
// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCError::Terminate()
{
	

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CThreads::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCTransaction)
//*-----------------------------------------------------------------------
//| Test Case:		TCTransaction - Multi-Threaded testing of the TransactionObject
//|	Created:			05/21/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCTransaction::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CThreads::Init())
	// }}
	{
		if(CreateRowset(DBPROP_CANHOLDROWS, SELECT_VALIDATIONORDER)==S_OK)
			return TRUE;
	}

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Get the transaction threading model CLSID
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCTransaction::Variation_1()
{
	
	TBEGIN

	//Get ThreadingModel
	TESTC(VerifyThreadingModel())
	
CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Get the transaction threading model Property
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCTransaction::Variation_2()
{
	
	TBEGIN
	ASSERT(m_pIDBInitialize);
	ULONG_PTR ulValue = 0;

	//Get ThreadingModel
	TESTC_PROVIDER(::GetProperty(DBPROP_DSOTHREADMODEL, DBPROPSET_DATASOURCEINFO, m_pIDBInitialize, &ulValue));
	//Threading Property is one of the following, not Anded!
	TESTC(ulValue == DBPROPVAL_RT_FREETHREAD || ulValue == DBPROPVAL_RT_APTMTTHREAD || ulValue == DBPROPVAL_RT_SINGLETHREAD);
	
CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Two Commits, from two transactions
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCTransaction::Variation_3()
{
	
	
	//TODO

	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Multiple thread running under one transaction, commit
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCTransaction::Variation_4()
{
	
	
	//TODO
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Multiple thread running under one transaction, abort
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCTransaction::Variation_5()
{
	
	
	//TODO
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCTransaction::Variation_6()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc One thread aborts, while the other is doing real work
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCTransaction::Variation_7()
{
	
	
	//TODO
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc One thread commits, while the other thread is doing real work
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCTransaction::Variation_8()
{
	

	//TODO

	return TEST_PASS;
}
// }}
// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCTransaction::Terminate()
{
	

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CThreads::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCOpenRowset)
//*-----------------------------------------------------------------------
//| Test Case:		TCOpenRowset - Multi-Threaded testing of the OpenRowsetObject
//|	Created:			05/21/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCOpenRowset::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CThreads::Init())
	// }}
	{
		if(CreateRowset(DBPROP_CANHOLDROWS, SELECT_VALIDATIONORDER)==S_OK)
			return TRUE;
	}

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc QI for IID_IOpenRowset from sep threads
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowset::Variation_1()
{
	
	TBEGIN
	INIT_THREADS(MAX_THREADS);
	THREADARG T1Arg;
	
	//CreateOpenRowset
	COpenRowset OpenRowset;
	TESTC_(OpenRowset.CreateOpenRowset(),S_OK);
	
	//Setup threads args
	T1Arg.pFunc = OpenRowset();
	T1Arg.pArg1 = (void*)&IID_IOpenRowset;

	CREATE_THREADS(Thread_QI,&T1Arg);
	START_THREADS();
	END_THREADS();	

CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Call OpenRowset from two threads, diff prop, from the same table
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowset::Variation_2()
{
	TBEGIN
	INIT_THREADS(MAX_THREADS);

	COpenRowset OpenRowset;

	//Setup property structs
	ULONG cPropSets1 = 0;
	DBPROPSET* rgPropSets1 = NULL;
	::SetSettableProperty(DBPROP_CANHOLDROWS,DBPROPSET_ROWSET,&cPropSets1,&rgPropSets1);

	ULONG cPropSets2 = 0;
	DBPROPSET* rgPropSets2 = NULL;
	::SetSettableProperty(DBPROP_IRowset,DBPROPSET_ROWSET,&cPropSets2,&rgPropSets2);

	//Setup thread arguments
	THREADARG T1Arg = { &OpenRowset, (void*)cPropSets1, rgPropSets1 };
	THREADARG T2Arg = { &OpenRowset, (void*)cPropSets2, rgPropSets2 };
	
	//Create OpenRowset
	TESTC_(OpenRowset.CreateOpenRowset(),S_OK);
	
	CREATE_FIRST_THREADS(Thread_OpenRowset,&T1Arg);
	CREATE_SECOND_THREADS(Thread_OpenRowset,&T2Arg);

	START_THREADS();
	END_THREADS();	

CLEANUP:
	::FreeProperties(&cPropSets1,&rgPropSets1);
	::FreeProperties(&cPropSets2,&rgPropSets2);
	TRETURN
}
// }}
// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCOpenRowset::Terminate()
{
	

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CThreads::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCRowset)
//*-----------------------------------------------------------------------
//| Test Case:		TCRowset - Multi-Threaded testing of the RowsetObject
//|	Created:			05/21/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCRowset::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CThreads::Init())
	// }}
	{
		if(CreateRowset(DBPROP_CANHOLDROWS, SELECT_VALIDATIONORDER)==S_OK)
			return TRUE;
	}

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCRowset::Variation_1()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Verify threading model - Free Threaded
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCRowset::Variation_2()
{
	
 	TBEGIN
	ULONG_PTR ulValue = SHRT_MAX;	 
	DBPROPFLAGS dwPropFlags;

	//Create a rowset
	CRowset RowsetA;
	TESTC_PROVIDER(RowsetA.CreateRowset(SELECT_VALIDATIONORDER)==S_OK);

	//Verify Threading model
	TESTC_PROVIDER(RowsetA.GetProperty(DBPROP_ROWTHREADMODEL,DBPROPSET_ROWSET,&ulValue))
	//Threading Property is one of the following, not Anded!
	TESTC(ulValue == DBPROPVAL_RT_FREETHREAD || ulValue == DBPROPVAL_RT_APTMTTHREAD || ulValue == DBPROPVAL_RT_SINGLETHREAD);

	
	//Verify Threading Model is Read/Write
	dwPropFlags = GetPropInfoFlags(DBPROP_ROWTHREADMODEL, DBPROPSET_ROWSET);
	
	//If Provider supported both threading models then the property
	//must be Read/Write to specify how to create the objects
	if(ulValue == DBPROPVAL_RT_BOTH)
	{
		TESTC(BITSET(dwPropFlags, DBPROPFLAGS_READ))
		TESTC(BITSET(dwPropFlags, DBPROPFLAGS_WRITE))
	}
	//Otherwise should be Read-Only since only creates certain type of objects
	else
	{
		TESTC(BITSET(dwPropFlags, DBPROPFLAGS_READ))
		TESTC(BITCLEAR(dwPropFlags, DBPROPFLAGS_WRITE))
	}
	
CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Verify read-only threading property
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCRowset::Variation_3()
{		
	TBEGIN
	DBPROP* pProp = NULL;
	HRESULT hr = S_OK;

	//Create a rowset
	CRowset RowsetA;
	TESTC_PROVIDER(RowsetA.SetSupportedProperty(DBPROP_ROWTHREADMODEL,DBPROPSET_ROWSET,(void*)ULONG_MAX,DBTYPE_I4));

	//ROWTHREADMODEL is read-only, should produce an error trying to set it...
	TESTC_(hr = RowsetA.CreateRowset(SELECT_VALIDATIONORDER), DB_E_ERRORSOCCURRED);

	//Verify Property Status...
	TESTC(FindProperty(DBPROP_ROWTHREADMODEL,DBPROPSET_ROWSET, RowsetA.m_cPropSets, RowsetA.m_rgPropSets, &pProp));
	TESTC(pProp->dwStatus == DBPROPSTATUS_NOTSETTABLE || pProp->dwStatus == DBPROPSTATUS_BADVALUE);

CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCRowset::Variation_4()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc QI from sep threads for diff riid's
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCRowset::Variation_5()
{
	TBEGIN
	INIT_THREADS(MAX_THREADS);
 	THREADARG T1Arg,T2Arg;
	CRowset RowsetA;

	//CreateRowset
	TESTC_PROVIDER(RowsetA.CreateRowset(SELECT_VALIDATIONORDER)==S_OK);

	//Setup threading args	
	T1Arg.pFunc = RowsetA();
	T1Arg.pArg1 = (void*)&IID_IRowset;
	
	T2Arg.pFunc = RowsetA();
	T2Arg.pArg1 = (void*)&IID_IRowsetInfo;

	CREATE_FIRST_THREADS(Thread_QI,&T1Arg);
	CREATE_SECOND_THREADS(Thread_QI,&T2Arg);

	START_THREADS();
	END_THREADS();	

CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc AddRef / Release testing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCRowset::Variation_6()
{
	
	TBEGIN
	INIT_THREADS(FOUR_THREADS);

	THREADARG T1Arg;

	//CreateRowset
	CRowset RowsetA;
	TESTC_PROVIDER(RowsetA.CreateRowset(SELECT_VALIDATIONORDER)==S_OK);

	//Now  setup the thread args
	T1Arg.pFunc = RowsetA(); 
	 
	//As a precation, so we don't acidently release the object
	//if the release threads get there first
	RowsetA()->AddRef();
	RowsetA()->AddRef();
	RowsetA()->AddRef();
	RowsetA()->AddRef();

	//T1 - AddRef, T2 - Release, T3 - AddRef, T4 - Release
	CREATE_THREAD(THREAD_ONE,  Thread_AddRef , &T1Arg);
	CREATE_THREAD(THREAD_TWO,  Thread_Release, &T1Arg);
	CREATE_THREAD(THREAD_THREE,Thread_AddRef,  &T1Arg);
	CREATE_THREAD(THREAD_FOUR, Thread_Release, &T1Arg);

	START_THREADS();
	END_THREADS();	

	//Restore original ref-count
	SetRefCount(RowsetA(), -4);

CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc AddRefRows / ReleaseRows testing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCRowset::Variation_7()
{
	
	TBEGIN
	INIT_THREADS(FOUR_THREADS);

	HROW hRow = DB_NULL_HROW;

	CRowset RowsetA;
	
	//We want to pass the HROW to all threads...
	THREADARG T1Arg = { &RowsetA, (void*)&hRow };
	
	//CreateRowset
	TESTC_PROVIDER(RowsetA.CreateRowset(SELECT_VALIDATIONORDER)==S_OK);
	
	//Obtain a row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK)

	//AddRef the row a few times, incase of race conditions
	TESTC_(RowsetA()->AddRefRows(ONE_ROW,&hRow,NULL,NULL),S_OK)
	TESTC_(RowsetA()->AddRefRows(ONE_ROW,&hRow,NULL,NULL),S_OK)
	TESTC_(RowsetA()->AddRefRows(ONE_ROW,&hRow,NULL,NULL),S_OK)
	TESTC_(RowsetA()->AddRefRows(ONE_ROW,&hRow,NULL,NULL),S_OK)
	TESTC_(RowsetA()->AddRefRows(ONE_ROW,&hRow,NULL,NULL),S_OK)

	//T1 - AddRef, T2 - Release, T3 - AddRef, T4 - Release
	CREATE_THREAD(THREAD_ONE,  Thread_AddRefRows, &T1Arg);
	CREATE_THREAD(THREAD_TWO,  Thread_ReleaseRows,&T1Arg);
	CREATE_THREAD(THREAD_THREE,Thread_AddRefRows, &T1Arg);
	CREATE_THREAD(THREAD_FOUR, Thread_ReleaseRows,&T1Arg);

	START_THREADS();
	END_THREADS();	

CLEANUP:
	//Restore original row ref-count
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseRows(hRow);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCRowset::Variation_8()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc GetNextRows from multiple threads
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCRowset::Variation_9()
{
	
	TBEGIN
	INIT_THREADS(MAX_THREADS);	
	CRowset RowsetA;

	//Setup threading args
	THREADARG T1Arg = { &RowsetA, (void*)0, (void*)ONE_ROW, (void*)DB_S_ENDOFROWSET };  
	
	//CreateRowset
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS, SELECT_VALIDATIONORDER)==S_OK);
	
	//Create Threads
	CREATE_THREADS(Thread_GetNextRows, &T1Arg);

	START_THREADS();
	END_THREADS();

CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc GetNextRows from multiple threads, with overlapping set of rows
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCRowset::Variation_10()
{
	
	TBEGIN
	INIT_THREADS(MAX_THREADS);	
	CRowset RowsetA;

	//We want to pass the HROW to all threads...
	THREADARG T1Arg = { &RowsetA, (void*)0, (void*)FOUR_ROWS, (void*)DB_S_ENDOFROWSET };

	//CreateRowset
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS, SELECT_VALIDATIONORDER)==S_OK); 

	//Create Threads
	CREATE_THREADS(Thread_GetNextRows,&T1Arg);

	START_THREADS();
	END_THREADS();	

CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc GetNextRows and GetData from seperate threads
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCRowset::Variation_11()
{
	TBEGIN
	INIT_THREADS(MAX_THREADS);	
	CRowset RowsetA;
	HROW hRow = NULL;

	//Setup Thread Arguments
	THREADARG T1Arg = { &RowsetA, (void*)ONE_ROW, &hRow, &RowsetA.m_hAccessor, (void*)S_OK };
	THREADARG T2Arg = { &RowsetA, (void*)0, (void*)FOUR_ROWS, (void*)DB_S_ENDOFROWSET };

	//CreateRowset
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS, SELECT_VALIDATIONORDER)==S_OK); 

	//Obtain row handle(s)
	TESTC_(RowsetA.GetRow(FIRST_ROW,&hRow),S_OK)	

	//Create Threads
	CREATE_FIRST_THREADS(Thread_GetData,&T1Arg);
	CREATE_SECOND_THREADS(Thread_GetNextRows,&T2Arg);

	START_THREADS();
	END_THREADS();	

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCRowset::Variation_12()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc RestartPosition with GetNextRows
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCRowset::Variation_13()
{
	TBEGIN
	INIT_THREADS(MAX_THREADS);	
	CRowset RowsetA;

	//Setup Thread Arguments
	THREADARG T1Arg  = { &RowsetA, (void*)0, (void*)FOUR_ROWS, (void*)DB_S_ENDOFROWSET };
	THREADARG T2Arg  = { &RowsetA, (void*)FALSE };  //FALSE - rows not released

	//CreateRowset
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS, SELECT_VALIDATIONORDER)==S_OK); 

	//Create Threads
	CREATE_FIRST_THREADS(Thread_GetNextRows,&T1Arg);
	CREATE_SECOND_THREADS(Thread_RestartPosition,&T2Arg);

	START_THREADS();
	END_THREADS();	

CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc RestartPosition with GetData
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCRowset::Variation_14()
{
	TBEGIN
	INIT_THREADS(MAX_THREADS);	
	CRowset RowsetA;
	HROW hRow = NULL;

	//Setup Thread Arguments
	THREADARG T1Arg = { &RowsetA, (void*)ONE_ROW, &hRow, &RowsetA.m_hAccessor, (void*)S_OK };
	THREADARG T2Arg = { &RowsetA, (void*)FALSE };//FALSE - rows not released

	//CreateRowset
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS, SELECT_VALIDATIONORDER)==S_OK); 

	//Obtain row handle(s)
	TESTC_(RowsetA.GetRow(FIRST_ROW,&hRow),S_OK)	

	//Create Threads
	CREATE_SECOND_THREADS(Thread_GetData,&T1Arg);
	CREATE_FIRST_THREADS(Thread_RestartPosition,&T2Arg);

	START_THREADS();
	END_THREADS();	

CLEANUP:
	RowsetA.ReleaseRows(hRow); 
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCRowset::Variation_15()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc Release a row being accessed by GetNextRows
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCRowset::Variation_16()
{
	TBEGIN
	INIT_THREADS(MAX_THREADS);	
	CRowset RowsetA;
	HROW hRow = NULL;

	//We want to pass the HROW to all threads...
	THREADARG T1Arg = { &RowsetA, (void*)0, (void*)FOUR_ROWS, (void*)DB_S_ENDOFROWSET };
	THREADARG T2Arg = { &RowsetA, &hRow, (void*)DB_E_ERRORSOCCURRED };

	//CreateRowset
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS, SELECT_VALIDATIONORDER)==S_OK); 

	//Grab the first row
	TESTC_(RowsetA.GetRow(FIRST_ROW,&hRow),S_OK)

	//Create Threads
	CREATE_FIRST_THREADS(Thread_GetNextRows,&T1Arg);
	CREATE_SECOND_THREADS(Thread_ReleaseRows,&T2Arg);

	START_THREADS();
	END_THREADS();	

CLEANUP:
	RowsetA.ReleaseRows(hRow); 
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc Delete a row, being accessed by GetData
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCRowset::Variation_17()
{
	TBEGIN
	INIT_THREADS(TWO_THREADS);	
	CRowsetChange RowsetA;
	HROW hRow = NULL;

	//Setup Thread Arguments
	THREADARG T1Arg     = { &RowsetA, (void*)ONE_ROW, &hRow, &RowsetA.m_hAccessor, (void*)DB_E_DELETEDROW };
	THREADARG T2Arg		= { &RowsetA, (void*)ONE_ROW, &hRow, (void*)DB_E_ERRORSOCCURRED };

	//CreateRowset
	TESTC_PROVIDER(RowsetA.CreateRowset(SELECT_VALIDATIONORDER)==S_OK);

	//Obtain row handle(s)
	TESTC_(RowsetA.GetRow(FIRST_ROW,&hRow),S_OK)	

	//Create Threads
	CREATE_FIRST_THREADS(Thread_GetData,	&T1Arg);
	CREATE_SECOND_THREADS(Thread_DeleteRows,&T2Arg);

	START_THREADS();
	END_THREADS();	

CLEANUP:
	RowsetA.ReleaseRows(hRow); 
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc Release the rowset while being used by another
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCRowset::Variation_18()
{
	
	TBEGIN
	INIT_THREADS(THREE_THREADS);	
	CRowset RowsetA;

	HROW hRow = NULL;

	//Setup Thread Arguments
	THREADARG T1Arg = { &RowsetA, (void*)ONE_ROW, &hRow, &RowsetA.m_hAccessor, (void*)S_OK };
	THREADARG T2Arg, T3Arg;

	//CreateRowset
	TESTC_PROVIDER(RowsetA.CreateRowset(SELECT_VALIDATIONORDER)==S_OK);

	//Finsih setting up thread args
	T2Arg.pFunc = RowsetA();
	T3Arg.pFunc = RowsetA();

	//Obtain row handle(s)
	TESTC_(RowsetA.GetRow(FIRST_ROW,&hRow),S_OK)	

	//Create Threads
	CREATE_THREAD(THREAD_ONE, Thread_GetData,&T1Arg);
	CREATE_THREAD(THREAD_TWO, Thread_Release,&T2Arg);
	CREATE_THREAD(THREAD_THREE, Thread_AddRef,&T3Arg);

	START_THREADS();
	END_THREADS();	

CLEANUP:
	RowsetA.ReleaseRows(hRow); 
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCRowset::Variation_19()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc GetData, same row, same accessor, diff threads
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCRowset::Variation_20()
{
	
	TBEGIN
	INIT_THREADS(MAX_THREADS);	
	CRowset RowsetA;
	HROW hRow = NULL;

	//Setup Thread Arguments
	THREADARG TArg1 = { &RowsetA, (void*)ONE_ROW, &hRow, &RowsetA.m_hAccessor, (void*)S_OK };

	//CreateRowset
	TESTC_PROVIDER(RowsetA.CreateRowset(SELECT_VALIDATIONORDER)==S_OK);

	//Obtain row handle(s)
	TESTC_(RowsetA.GetRow(FIRST_ROW,&hRow),S_OK)	

	//Create Threads
	CREATE_THREADS(Thread_GetData,&TArg1);

	START_THREADS();
	END_THREADS();	

CLEANUP:
	RowsetA.ReleaseRows(hRow); 
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc GetData, same row, diff accessor, diff threads
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCRowset::Variation_21()
{
	TBEGIN
	INIT_THREADS(MAX_THREADS);	
	CRowset RowsetA;

	HROW hRow = NULL;
	HACCESSOR hAccessor2 = NULL;

	//Setup Thread Arguments
	THREADARG TArg1 = { &RowsetA, (void*)ONE_ROW, &hRow, &RowsetA.m_hAccessor, (void*)S_OK };
	THREADARG TArg2 = { &RowsetA, (void*)ONE_ROW, &hRow, &hAccessor2, (void*)S_OK };

	//CreateRowset
	TESTC_PROVIDER(RowsetA.CreateRowset(SELECT_VALIDATIONORDER)==S_OK);

	//Create another accessor
	TESTC_(GetAccessorAndBindings(RowsetA(),DBACCESSOR_ROWDATA,&hAccessor2,
		NULL,NULL,NULL, DBPART_ALL, ALL_COLS_BOUND),S_OK)

	//Obtain row handle(s)
	TESTC_(RowsetA.GetRow(FIRST_ROW,&hRow),S_OK)	

	//Create Threads
	CREATE_FIRST_THREADS(Thread_GetData,&TArg1);
	CREATE_SECOND_THREADS(Thread_GetData,&TArg2);

	START_THREADS();
	END_THREADS();	

CLEANUP:
	RowsetA.ReleaseRows(hRow); 
	RowsetA.ReleaseAccessor(hAccessor2);
	TRETURN
}
// }}
// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCRowset::Terminate()
{
	

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CThreads::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCRowsetChange)
//*-----------------------------------------------------------------------
//| Test Case:		TCRowsetChange - Multi-Threaded testing of the RowsetChange Object
//|	Created:			05/28/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCRowsetChange::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CThreads::Init())
	// }}
	{
		TEST_PROVIDER(CreateRowset(DBPROP_IRowsetChange)==S_OK);
		return TRUE;
	}

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc SetData, diff columns, from seperate threads
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCRowsetChange::Variation_1()
{
	TBEGIN
	INIT_THREADS(MAX_THREADS);
	CRowsetChange RowsetA;
	HROW hRow = NULL;
	HACCESSOR hAccessor = NULL;
	
	//Setup Thread Arguments
	THREADARG TArg1 = { &RowsetA, &hRow, &hAccessor, (void*)S_OK };
	THREADARG TArg2 = { &RowsetA, &hRow, &hAccessor, (void*)S_OK };

	//CreateRowset
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);

	//Since SQLServer has a problem trying to reposition on a row if the index 
	//column has changed, don't change the index column, bind all columns except that one
	TESTC_(GetAccessorAndBindings(RowsetA.pIAccessor(), DBACCESSOR_ROWDATA, 
		&hAccessor,	NULL, NULL, NULL, DBPART_ALL, UPDATEABLE_NONINDEX_COLS_BOUND),S_OK); 
	
	//Obtain row handle(s)
	TESTC_(RowsetA.GetRow(FIRST_ROW,&hRow),S_OK)	

	//Create Threads
	CREATE_FIRST_THREADS(Thread_SetData,&TArg1);
	CREATE_SECOND_THREADS(Thread_SetData,&TArg2);

	START_THREADS();
	END_THREADS();	

CLEANUP:
	RowsetA.ReleaseAccessor(hAccessor);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc SetData, same columns, from seperate threads
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCRowsetChange::Variation_2()
{
	TBEGIN
	INIT_THREADS(MAX_THREADS);	
	CRowsetChange RowsetA;
	HROW hRow = NULL;
	HACCESSOR hAccessor = NULL;
	
	//Setup Thread Arguments
	THREADARG TArg1 = { &RowsetA, &hRow, &hAccessor, (void*)S_OK };
	THREADARG TArg2 = { &RowsetA, &hRow, &hAccessor, (void*)S_OK };

	//CreateRowset
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);

	//Since SQLServer has a problem trying to reposition on a row if the index 
	//column has changed, don't change the index column, bind all columns except that one
	TESTC_(GetAccessorAndBindings(RowsetA.pIAccessor(), DBACCESSOR_ROWDATA, 
		&hAccessor,	NULL, NULL, NULL, DBPART_ALL, UPDATEABLE_NONINDEX_COLS_BOUND),S_OK); 

	//Obtain row handle(s)
	TESTC_(RowsetA.GetRow(FIRST_ROW,&hRow),S_OK)	

	//Create Threads
	CREATE_FIRST_THREADS(Thread_SetData, &TArg1);
	CREATE_SECOND_THREADS(Thread_SetData, &TArg2);

	START_THREADS();
	END_THREADS();	

CLEANUP:
	RowsetA.ReleaseAccessor(hAccessor);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCRowsetChange::Variation_3()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc DeleteRows from seperate threads, with overlapping set of rows
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCRowsetChange::Variation_4()
{
	TBEGIN
	INIT_THREADS(MAX_THREADS);	
	CRowsetChange RowsetA;

	ULONG cRows = FOUR_ROWS;
	HROW rghRow[FOUR_ROWS] = { NULL,NULL,NULL,NULL };

	//Setup Thread Arguments
	THREADARG T1Arg = { &RowsetA, (void*)cRows, rghRow, (void*)DB_E_ERRORSOCCURRED, (void*)S_OK };

	//CreateRowset
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);

	//Obtain row handle(s)
	TESTC_(RowsetA.GetRow(FIRST_ROW,FOUR_ROWS,rghRow),S_OK)	

	//Create Threads
	CREATE_THREADS(Thread_DeleteRows,&T1Arg);
	START_THREADS();
	END_THREADS();	

CLEANUP:
	RowsetA.ReleaseRows(FOUR_ROWS,rghRow);
	TableInsert(TWO_ROWS);	//Adjust the table
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCRowsetChange::Variation_5()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Insert a row from seperate threads
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCRowsetChange::Variation_6()
{
	TBEGIN
	INIT_THREADS(MAX_THREADS);	
	CRowsetChange RowsetA;

	HROW hRow = NULL;
	HACCESSOR hAccessor = NULL;
	
	//Setup Thread Arguments
	THREADARG T1Arg = { &RowsetA, &hRow, &hAccessor, (void*)S_OK };
	THREADARG T2Arg = { &RowsetA, &hRow, &hAccessor, (void*)S_OK };

	//CreateRowset
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);

	//Since SQLServer has a problem trying to reposition on a row if the index 
	//column has changed, don't change the index column, bind all columns except that one
	TESTC_(GetAccessorAndBindings(RowsetA.pIAccessor(), DBACCESSOR_ROWDATA, 
		&hAccessor,	NULL, NULL, NULL, DBPART_ALL, UPDATEABLE_NONINDEX_COLS_BOUND),S_OK); 

	//Obtain row handle(s)
	TESTC_(RowsetA.GetRow(FIRST_ROW,&hRow),S_OK)	

	//Create Threads
	CREATE_FIRST_THREADS(Thread_SetData, &T1Arg);
	CREATE_SECOND_THREADS(Thread_SetData, &T2Arg);

	START_THREADS();
	END_THREADS();	

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCRowsetChange::Variation_7()
{
	
	return TEST_PASS;
}
// }}
// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCRowsetChange::Terminate()
{
	

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CThreads::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCRowsetOther)
//*-----------------------------------------------------------------------
//| Test Case:		TCRowsetOther - Multi-Threaded testing of the other Rowset variants
//|	Created:			05/28/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCRowsetOther::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CThreads::Init())
	// }}
	{
		if(CreateRowset(DBPROP_CANHOLDROWS, USE_OPENROWSET)==S_OK)
			return TRUE;
	}

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc GetProperties from seperate threads
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCRowsetOther::Variation_1()
{
	TBEGIN
	INIT_THREADS(MAX_THREADS);	

	//Setup Thread Arguments
	ULONG cPropIDSets1 = 0;
	DBPROPIDSET* rgPropIDSets1 = NULL;
	if(SettableProperty(DBPROP_CANHOLDROWS,DBPROPSET_ROWSET))
		::SetProperty(DBPROP_CANHOLDROWS,DBPROPSET_ROWSET,&cPropIDSets1,&rgPropIDSets1);
	
	ULONG cPropIDSets2 = 0;
	DBPROPIDSET* rgPropIDSets2 = NULL;
	if(SettableProperty(DBPROP_CANSCROLLBACKWARDS,DBPROPSET_ROWSET))
		::SetProperty(DBPROP_CANSCROLLBACKWARDS,DBPROPSET_ROWSET,&cPropIDSets2,&rgPropIDSets2);
	
	THREADARG GetPropArg1 = { this, (void*)cPropIDSets1, rgPropIDSets1 };
	THREADARG GetPropArg2 = { this, (void*)cPropIDSets2, rgPropIDSets2 };

	//Create Threads
	CREATE_FIRST_THREADS(Thread_GetRowsetProperties,&GetPropArg1);
	CREATE_SECOND_THREADS(Thread_GetRowsetProperties,&GetPropArg2);

	START_THREADS();
	END_THREADS();	

//CLEANUP:
	::FreeProperties(&cPropIDSets1,&rgPropIDSets1);
	::FreeProperties(&cPropIDSets2,&rgPropIDSets2);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc GetProperties while Releasing the Rowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCRowsetOther::Variation_2()
{
	
	TBEGIN
	INIT_THREADS(THREE_THREADS);	

	ULONG cPropIDSets = 0;
	DBPROPIDSET* rgPropIDSets = NULL;
	if(SettableProperty(DBPROP_CANHOLDROWS,DBPROPSET_ROWSET))
		::SetProperty(DBPROP_CANHOLDROWS,DBPROPSET_ROWSET,&cPropIDSets,&rgPropIDSets);

	//Setup Thread Arguments
	THREADARG T1Arg = { this, (void*)cPropIDSets, rgPropIDSets };
	THREADARG T2Arg = { pIRowset() };
	THREADARG T3Arg = { pIRowset() };

	//Create Threads
	CREATE_THREAD(THREAD_ONE, Thread_GetRowsetProperties,&T1Arg);
	CREATE_THREAD(THREAD_TWO, Thread_Release,&T2Arg);
	CREATE_THREAD(THREAD_THREE, Thread_AddRef,&T3Arg);

	START_THREADS();
	END_THREADS();	

//CLEANUP:
	::FreeProperties(&cPropIDSets,&rgPropIDSets);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc GetSpecification from seperate threads
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCRowsetOther::Variation_3()
{
	
	TBEGIN
	INIT_THREADS(MAX_THREADS);	

	//Setup Thread Arguments
	THREADARG GetSpecArg = { this, (void*)&IID_IGetDataSource };

	//Create Threads
	CREATE_THREADS(Thread_GetSpecification,&GetSpecArg);

	START_THREADS();
	END_THREADS();	

//CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCRowsetOther::Variation_4()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Requery from seperate threads, V2
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCRowsetOther::Variation_5()
{
	
	TBEGIN
	INIT_THREADS(MAX_THREADS);	

	//Setup Thread Arguments
	THREADARG T1Arg = { this };

	//Create Threads
	CREATE_THREADS(Thread_Requery,&T1Arg);

	START_THREADS();
	END_THREADS();	

//CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCRowsetOther::Variation_6()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc GetRowsAt with a set of overlapping rows from seperate threads
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCRowsetOther::Variation_7()
{
	
	TBEGIN
	INIT_THREADS(MAX_THREADS);	
	CRowset RowsetA; 

	//Setup Thread Arguments
	THREADARG T1Arg = { &RowsetA };

	//CreateRowset
	RowsetA.SetSettableProperty(DBPROP_CANHOLDROWS);
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_IRowsetLocate, SELECT_VALIDATIONORDER)==S_OK);

	//Create Threads
	CREATE_THREADS(Thread_GetRowsAt,&T1Arg);

	START_THREADS();
	END_THREADS();	

CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc GetRowsByBookmark with a set of overlapping rows from different threads
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCRowsetOther::Variation_8()
{
	
	TBEGIN
	INIT_THREADS(MAX_THREADS);	
	CRowset RowsetA;

	DBBKMARK cbBookmark = 0;
	BYTE* pBookmark = NULL;
	HROW hRow = NULL;

	//Setup Thread Arguments
	THREADARG T1Arg = { &RowsetA, &cbBookmark, &pBookmark };

	//CreateRowset
	RowsetA.SetProperty(DBPROP_CANHOLDROWS);
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_IRowsetLocate, SELECT_VALIDATIONORDER)==S_OK);

	//Get the first row
	RowsetA.GetRow(FIRST_ROW, &hRow);
	
	//Get the Bookmark info, this alloc the space for the bookmark
	RowsetA.GetBookmark(hRow, &cbBookmark, &pBookmark);

	//Create Threads
	CREATE_THREADS(Thread_GetRowsByBookmark,&T1Arg);

	START_THREADS();
	END_THREADS();	
	
CLEANUP:
	RowsetA.ReleaseRows(hRow);
	PROVIDER_FREE(pBookmark);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCRowsetOther::Variation_9()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc GetVisibleData while resyncing the rows
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCRowsetOther::Variation_10()
{
	TBEGIN
	INIT_THREADS(MAX_THREADS);	
	CRowset RowsetA;
	HROW hRow = NULL;
	
	//Setup Thread Arguments
	THREADARG T1Arg         = { &RowsetA, &hRow, &RowsetA.m_hAccessor };
	THREADARG T2Arg         = { &RowsetA };

	//CreateRowset
	RowsetA.SetProperty(DBPROP_IRowsetResynch);
	RowsetA.SetProperty(DBPROP_IRowsetLocate);
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS, SELECT_VALIDATIONORDER)==S_OK);

	//Obtain row handle(s)
	TESTC_(RowsetA.GetRow(FIRST_ROW,&hRow),S_OK)	

	//Create Threads
	CREATE_FIRST_THREADS(Thread_GetVisibleData,&T1Arg);
	CREATE_SECOND_THREADS(Thread_ResynchRows,&T2Arg);

	START_THREADS();
	END_THREADS();	

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	TRETURN
}
// }}
// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCRowsetOther::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CThreads::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCRowsetUpdate)
//*-----------------------------------------------------------------------
//| Test Case:		TCRowsetUpdate - Multi-Threaded testing of the RowsetUpdate Object
//|	Created:			05/28/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCRowsetUpdate::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CThreads::Init())
	// }}
	{
		TEST_PROVIDER(CreateRowset(DBPROP_IRowsetUpdate)==S_OK);
		return TRUE;
	}

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCRowsetUpdate::Variation_1()
{
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCRowsetUpdate::Variation_2()
{
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCRowsetUpdate::Variation_3()
{
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc GetPendingRows while GetRowStatus
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCRowsetUpdate::Variation_4()
{
	TBEGIN
	INIT_THREADS(MAX_THREADS);
	CRowsetUpdate RowsetA;  

	ULONG cRows = FOUR_ROWS;
	HROW rghRow[FOUR_ROWS] = {NULL,NULL,NULL,NULL};

	//Setup Thread Arguments
	THREADARG T1Arg         = { &RowsetA };
	THREADARG T2Arg         = { &RowsetA, &cRows, rghRow, (void*)DB_S_ERRORSOCCURRED, (void*)S_OK };

	//CreateRowset
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);

	//Obtain row handle(s)
	TESTC_(RowsetA.GetRow(FIRST_ROW,THREE_ROWS,rghRow),S_OK)	

	//Make some change(s)
	TESTC_(RowsetA.HardDeleteRow(rghRow[ROW_THREE]),S_OK)
	TESTC_(RowsetA.ModifyRow(rghRow[ROW_ONE]),S_OK)
	if(RowsetA.AllowPendingRows(2))
		TESTC_(RowsetA.InsertRow(&rghRow[ROW_FOUR]),S_OK)

	//Create Threads
	CREATE_FIRST_THREADS(Thread_GetPendingRows,&T1Arg);
	CREATE_SECOND_THREADS(Thread_GetRowStatus,&T2Arg);

	START_THREADS();
	END_THREADS();	

CLEANUP:
	//Release handle(s)
	RowsetA.ReleaseRows(FOUR_ROWS,rghRow);
	TableInsert(ONE_ROW);	//Adjust the table
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc GetPendingrows while Update
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCRowsetUpdate::Variation_5()
{
	TBEGIN
	INIT_THREADS(MAX_THREADS);	
	CRowsetUpdate RowsetA;

	const int cRows = FOUR_ROWS;
	HROW rghRow[FOUR_ROWS] = {NULL,NULL,NULL,NULL};

	//Setup Thread Arguments
	THREADARG T1Arg         = { &RowsetA };
	THREADARG T2Arg         = { &RowsetA };

	//CreateRowset
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);

	//Obtain row handle(s)
	TESTC_(RowsetA.GetRow(FIRST_ROW,THREE_ROWS,rghRow),S_OK)	

	//Make some change(s)
	TESTC_(RowsetA.HardDeleteRow(rghRow[ROW_THREE]),S_OK)
	TESTC_(RowsetA.InsertRow(&rghRow[ROW_FOUR]),S_OK)
	if(RowsetA.AllowPendingRows(2))
		TESTC_(RowsetA.ModifyRow(rghRow[ROW_ONE]),S_OK)

	//Create Threads
	CREATE_FIRST_THREADS(Thread_GetPendingRows,&T1Arg);
	CREATE_SECOND_THREADS(Thread_Update,&T2Arg);

	START_THREADS();
	END_THREADS();	

CLEANUP:
	//Release handle(s)
	RowsetA.ReleaseRows(FOUR_ROWS,rghRow);
	TableInsert(ONE_ROW);	//Adjust the table
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc GetPendingRows while Undo
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCRowsetUpdate::Variation_6()
{
	TBEGIN
	INIT_THREADS(MAX_THREADS);	
	CRowsetUpdate RowsetA;

	const int cRows = FOUR_ROWS;
	HROW rghRow[FOUR_ROWS] = {NULL,NULL,NULL,NULL};

	//Setup Thread Arguments
	THREADARG T1Arg         = { &RowsetA };
	THREADARG T2Arg         = { &RowsetA };

	//CreateRowset
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);

	//Obtain row handle(s)
	TESTC_(RowsetA.GetRow(FIRST_ROW,THREE_ROWS,rghRow),S_OK)	

	//Make some change(s)
	TESTC_(RowsetA.InsertRow(&rghRow[ROW_FOUR]),S_OK)
	if(RowsetA.AllowPendingRows(2))
		TESTC_(RowsetA.ModifyRow(rghRow[ROW_ONE]),S_OK)

	//Create Threads
	CREATE_FIRST_THREADS(Thread_GetPendingRows,&T1Arg);
	CREATE_SECOND_THREADS(Thread_Undo,&T2Arg);

	START_THREADS();
	END_THREADS();	

CLEANUP:
	//Release handle(s)
	RowsetA.ReleaseRows(FOUR_ROWS,rghRow);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCRowsetUpdate::Variation_7()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc GetOriginalData while Update
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCRowsetUpdate::Variation_8()
{
	TBEGIN
	INIT_THREADS(MAX_THREADS);	
	CRowsetUpdate RowsetA;
	HROW rghRow[FOUR_ROWS] = {NULL,NULL,NULL,NULL};

	//Setup Thread Arguments
	THREADARG T1Arg         = { &RowsetA, &rghRow[ROW_ONE], &RowsetA.m_hAccessor };
	THREADARG T2Arg         = { &RowsetA };
	
	//CreateRowset
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);

	//Obtain row handle(s)
	TESTC_(RowsetA.GetRow(FIRST_ROW,THREE_ROWS,rghRow),S_OK)	

	//Make some change(s)
	TESTC_(RowsetA.ModifyRow(rghRow[ROW_ONE]),S_OK)
	if(RowsetA.AllowPendingRows(2))
		TESTC_(RowsetA.InsertRow(&rghRow[ROW_FOUR]),S_OK)

	//Create Threads
	CREATE_FIRST_THREADS(Thread_GetOriginalData,&T1Arg);
	CREATE_SECOND_THREADS(Thread_Update,&T2Arg);

	START_THREADS();
	END_THREADS();	

CLEANUP:
	RowsetA.ReleaseRows(FOUR_ROWS,rghRow);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc GetOriginalData while GetVisibleData
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCRowsetUpdate::Variation_9()
{
	TBEGIN
	INIT_THREADS(MAX_THREADS);	
	CRowsetUpdate RowsetA;
	HROW rghRow[FOUR_ROWS] = {NULL,NULL,NULL,NULL};

	//Setup Thread Arguments
	THREADARG T1Arg         = { &RowsetA, &rghRow[ROW_ONE], &RowsetA.m_hAccessor };
	THREADARG T2Arg         = { &RowsetA, &rghRow[ROW_ONE], &RowsetA.m_hAccessor };
	
	//CreateRowset
	RowsetA.SetProperty(DBPROP_IRowsetResynch);
	RowsetA.SetProperty(DBPROP_IRowsetLocate);
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);

	//Obtain row handle(s)
	TESTC_(RowsetA.GetRow(FIRST_ROW,THREE_ROWS,rghRow),S_OK)	

	//Make some change(s)
	TESTC_(RowsetA.ModifyRow(rghRow[ROW_ONE]),S_OK)

	//Create Threads
	CREATE_FIRST_THREADS(Thread_GetOriginalData,&T1Arg);
	CREATE_SECOND_THREADS(Thread_GetVisibleData,&T2Arg);

	START_THREADS();
	END_THREADS();	

CLEANUP:
	RowsetA.ReleaseRows(FOUR_ROWS,rghRow);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCRowsetUpdate::Variation_10()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc Undo while Update
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCRowsetUpdate::Variation_11()
{
	TBEGIN
	INIT_THREADS(MAX_THREADS);
	CRowsetUpdate RowsetA;

	const int cRows = FOUR_ROWS;
	HROW rghRow[FOUR_ROWS] = {NULL,NULL,NULL,NULL};

	//Setup Thread Arguments
	THREADARG T1Arg         = { &RowsetA };
	THREADARG T2Arg         = { &RowsetA };

	//CreateRowset
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);

	//Obtain row handle(s)
	TESTC_(RowsetA.GetRow(FIRST_ROW,THREE_ROWS,rghRow),S_OK)	

	//Make some change(s)
	TESTC_(RowsetA.ModifyRow(rghRow[ROW_ONE]),S_OK)
	if(RowsetA.AllowPendingRows(2))
		TESTC_(RowsetA.InsertRow(&rghRow[ROW_FOUR]),S_OK)

	//Create Threads
	CREATE_FIRST_THREADS(Thread_Update,&T1Arg);
	CREATE_SECOND_THREADS(Thread_Undo,&T2Arg);

	START_THREADS();
	END_THREADS();	

CLEANUP:
	//Release handle(s)
	RowsetA.ReleaseRows(FOUR_ROWS,rghRow);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCRowsetUpdate::Variation_12()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc Insert while Update
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCRowsetUpdate::Variation_13()
{
	TBEGIN
	INIT_THREADS(MAX_THREADS);	
	CRowsetUpdate RowsetA;
	HROW rghRow[FOUR_ROWS] = {NULL,NULL,NULL,NULL};

	//Setup Thread Arguments
	THREADARG T1Arg         = { &RowsetA, &RowsetA.m_hAccessor, (void*)DB_E_MAXPENDCHANGESEXCEEDED };
	THREADARG T2Arg         = { &RowsetA };

	//CreateRowset
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);

	//Obtain row handle(s)
	TESTC_(RowsetA.GetRow(FIRST_ROW,THREE_ROWS,rghRow),S_OK)	

	//Make some change(s)
	if(RowsetA.AllowPendingRows(2))
		TESTC_(RowsetA.ModifyRow(rghRow[ROW_ONE]),S_OK)
	if(RowsetA.AllowPendingRows(3))
		TESTC_(RowsetA.InsertRow(&rghRow[ROW_FOUR]),S_OK)

	//Create Threads
	CREATE_FIRST_THREADS(Thread_InsertRow,&T1Arg);
	CREATE_SECOND_THREADS(Thread_Update,&T2Arg);

	START_THREADS();
	END_THREADS();	

CLEANUP:
	RowsetA.ReleaseRows(FOUR_ROWS,rghRow);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc Modify while Update
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCRowsetUpdate::Variation_14()
{
	TBEGIN
	INIT_THREADS(MAX_THREADS);	
	CRowsetUpdate RowsetA;
	HROW rghRow[FIVE_ROWS] = {NULL,NULL,NULL,NULL,NULL};

	//Setup Thread Arguments
	THREADARG T1Arg         = { &RowsetA, &rghRow[ROW_ONE], &RowsetA.m_hAccessor, (void*)S_OK };
	THREADARG T2Arg         = { &RowsetA };

	//CreateRowset
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS, 
		SELECT_ORDERBYNUMERIC, IID_IRowset, 
		NULL, DBACCESSOR_ROWDATA, DBPART_ALL, 
		UPDATEABLE_COLS_BOUND | NONINDEX_COLS_BOUND)==S_OK);

	//Obtain row handle(s)
	TESTC_(RowsetA.GetRow(FIRST_ROW,FOUR_ROWS,rghRow),S_OK)	
 
	//Make some change(s)
	TESTC_(RowsetA.ModifyRow(rghRow[ROW_ONE]),S_OK)
	if(RowsetA.AllowPendingRows(2))
		TESTC_(RowsetA.DeleteRow(rghRow[ROW_FOUR]) ,S_OK)
 
	//Create Threads
	CREATE_FIRST_THREADS(Thread_SetData,&T1Arg);
	CREATE_SECOND_THREADS(Thread_Update,&T2Arg);

	START_THREADS();
	END_THREADS();	

CLEANUP:
	//Release handle(s)
	RowsetA.ReleaseRows(FOUR_ROWS,rghRow);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc Delete while Update
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCRowsetUpdate::Variation_15()
{
	TBEGIN
	INIT_THREADS(TWO_THREADS);	
	CRowsetUpdate RowsetA;

	ULONG cRows = 0;
	HROW rghRow[FOUR_ROWS] = {NULL,NULL,NULL,NULL};

	//Setup Thread Arguments
	THREADARG T1Arg         = { &RowsetA, (void*)FOUR_ROWS, rghRow, (void*)DB_S_ERRORSOCCURRED, (void*)DB_E_ERRORSOCCURRED };
	THREADARG T2Arg         = { &RowsetA };

	//CreateRowset
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);

	//Obtain row handle(s)
	TESTC_(RowsetA.GetRow(FIRST_ROW,THREE_ROWS,rghRow),S_OK)	
	cRows++;

	//Make some change(s)
	if(RowsetA.AllowPendingRows(2))
	{
		TESTC_(RowsetA.ModifyRow(rghRow[ROW_TWO]),S_OK)
		cRows++;
	}
	if(RowsetA.AllowPendingRows(3))
	{
		TESTC_(RowsetA.HardDeleteRow(rghRow[ROW_THREE]),S_OK)
		cRows++;
	}
	if(RowsetA.AllowPendingRows(4))
	{
		TESTC_(RowsetA.InsertRow(&rghRow[ROW_FOUR]),S_OK)
		cRows++;
	}

	//Create Threads
	CREATE_FIRST_THREADS(Thread_DeleteRows,&T1Arg);
	CREATE_SECOND_THREADS(Thread_Update,&T2Arg);

	START_THREADS();
	END_THREADS();	

CLEANUP:
	//Release handle(s)
	RowsetA.ReleaseRows(FOUR_ROWS,rghRow);
	TableInsert(FIVE_ROWS);	//Adjust the table
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCRowsetUpdate::Variation_16()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc Insert while Undo
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCRowsetUpdate::Variation_17()
{
	TBEGIN
	INIT_THREADS(MAX_THREADS);	
	CRowsetUpdate RowsetA;
	HROW rghRow[FOUR_ROWS] = {NULL,NULL,NULL,NULL};

	//Setup Thread Arguments
	THREADARG T1Arg         = { &RowsetA, &RowsetA.m_hAccessor, (void*)DB_E_MAXPENDCHANGESEXCEEDED };
	THREADARG T2Arg         = { &RowsetA };

	//CreateRowset
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);

	//Obtain row handle(s)
	TESTC_(RowsetA.GetRow(FIRST_ROW,THREE_ROWS,rghRow),S_OK)	

	//Make some change(s)
	if(RowsetA.AllowPendingRows(2))
		TESTC_(RowsetA.InsertRow(&rghRow[ROW_FOUR]),S_OK)
	if(RowsetA.AllowPendingRows(3))
		TESTC_(RowsetA.ModifyRow(rghRow[ROW_ONE]),S_OK)

	//Create Threads
	CREATE_FIRST_THREADS(Thread_InsertRow,&T1Arg);
	CREATE_SECOND_THREADS(Thread_Undo,&T2Arg);

	START_THREADS();
	END_THREADS();	

CLEANUP:
	RowsetA.ReleaseRows(FOUR_ROWS,rghRow);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc Modify while Undo
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCRowsetUpdate::Variation_18()
{
	TBEGIN
	INIT_THREADS(MAX_THREADS);	
	CRowsetUpdate RowsetA;
	HROW rghRow[FOUR_ROWS] = {NULL,NULL,NULL,NULL};

	//Setup Thread Arguments
	THREADARG T1Arg         = { &RowsetA, &rghRow[ROW_ONE], &RowsetA.m_hAccessor, (void*)S_OK };
	THREADARG T2Arg         = { &RowsetA };

	//CreateRowset
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);

	//Obtain row handle(s)
	TESTC_(RowsetA.GetRow(FIRST_ROW,THREE_ROWS,rghRow),S_OK)	

	//Make some change(s)
	TESTC_(RowsetA.HardDeleteRow(rghRow[ROW_THREE]),S_OK)
	if(RowsetA.AllowPendingRows(2))
		TESTC_(RowsetA.InsertRow(&rghRow[ROW_FOUR]),S_OK)
	if(RowsetA.AllowPendingRows(3))
		TESTC_(RowsetA.ModifyRow(rghRow[ROW_ONE]),S_OK)

	//Create Threads
	CREATE_FIRST_THREADS(Thread_SetData,&T1Arg);
	CREATE_SECOND_THREADS(Thread_Undo,&T2Arg);

	START_THREADS();
	END_THREADS();	

CLEANUP:
	//Release handle(s)
	RowsetA.ReleaseRows(FOUR_ROWS,rghRow);
	TableInsert(ONE_ROW);	//Adjust the table
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc Delete while Undo
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCRowsetUpdate::Variation_19()
{
	TBEGIN
	INIT_THREADS(TWO_THREADS);	
	CRowsetUpdate RowsetA;

	ULONG cRows = 0;
	HROW rghRow[FOUR_ROWS] = {NULL,NULL,NULL,NULL};

	//Setup Thread Arguments
	THREADARG T1Arg         = { &RowsetA, (void*)cRows, rghRow, (void*)DB_E_ERRORSOCCURRED, (void*)S_OK };
	THREADARG T2Arg         = { &RowsetA };

	//CreateRowset
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);

	//Obtain row handle(s)
	TESTC_(RowsetA.GetRow(FIRST_ROW,THREE_ROWS,rghRow),S_OK)	
	cRows++;

	//Make some change(s)
	if(RowsetA.AllowPendingRows(2))
	{
		TESTC_(RowsetA.ModifyRow(rghRow[ROW_TWO]),S_OK)
		cRows++;
	}

	if(RowsetA.AllowPendingRows(3))
	{
		TESTC_(RowsetA.InsertRow(&rghRow[ROW_FOUR]),S_OK)
		cRows++;
	}

	//Create Threads
	T1Arg.pArg1 = (void*)cRows;
	CREATE_FIRST_THREADS(Thread_DeleteRows,&T1Arg);
	CREATE_SECOND_THREADS(Thread_Update,&T2Arg);

	START_THREADS();
	END_THREADS();	

CLEANUP:
	//Release handle(s)
	RowsetA.ReleaseRows(FOUR_ROWS,rghRow);
	TableInsert(FOUR_ROWS);	//Adjust the table
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCRowsetUpdate::Variation_20()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc Update while Resynch
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCRowsetUpdate::Variation_21()
{
	TBEGIN
	INIT_THREADS(MAX_THREADS);	
	CRowsetUpdate RowsetA;  

	const int cRows = FOUR_ROWS;
	HROW rghRow[FOUR_ROWS] = {NULL,NULL,NULL,NULL};

	//Setup Thread Arguments
	THREADARG T1Arg         = { &RowsetA };
	THREADARG T2Arg         = { &RowsetA };

	//CreateRowset
	RowsetA.SetProperty(DBPROP_IRowsetResynch);
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);

	//Obtain row handle(s)
	TESTC_(RowsetA.GetRow(FIRST_ROW,THREE_ROWS,rghRow),S_OK)	

	//Make some change(s)
	TESTC_(RowsetA.HardDeleteRow(rghRow[ROW_THREE]),S_OK)
	TESTC_(RowsetA.InsertRow(&rghRow[ROW_FOUR]),S_OK)
	if(RowsetA.AllowPendingRows(2))
		TESTC_(RowsetA.ModifyRow(rghRow[ROW_ONE]),S_OK)

	//Create Threads
	CREATE_FIRST_THREADS(Thread_Update,&T1Arg);
	CREATE_SECOND_THREADS(Thread_ResynchRows,&T2Arg);

	START_THREADS();
	END_THREADS();	

CLEANUP:
	//Release handle(s)
	RowsetA.ReleaseRows(FOUR_ROWS,rghRow);
	TableInsert(ONE_ROW);	//Adjust the table
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(22)
//*-----------------------------------------------------------------------
// @mfunc Undo while Resynch
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCRowsetUpdate::Variation_22()
{
	TBEGIN
	INIT_THREADS(MAX_THREADS);	
	CRowsetUpdate RowsetA;	 

	const int cRows = FOUR_ROWS;
	HROW rghRow[FOUR_ROWS] = {NULL,NULL,NULL,NULL};

	//Setup Thread Arguments
	THREADARG T1Arg         = { &RowsetA };
	THREADARG T2Arg         = { &RowsetA };

	//CreateRowset
	RowsetA.SetProperty(DBPROP_IRowsetResynch);
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);

	//Obtain row handle(s)
	TESTC_(RowsetA.GetRow(FIRST_ROW,THREE_ROWS,rghRow),S_OK)	

	//Make some change(s)
	TESTC_(RowsetA.ModifyRow(rghRow[ROW_ONE]),S_OK)
	if(RowsetA.AllowPendingRows(2))
		TESTC_(RowsetA.InsertRow(&rghRow[ROW_FOUR]),S_OK)

	//Create Threads
	CREATE_FIRST_THREADS(Thread_ResynchRows,&T1Arg);
	CREATE_SECOND_THREADS(Thread_Undo,&T2Arg);

	START_THREADS();
	END_THREADS();	

CLEANUP:
	//Release handle(s)
	RowsetA.ReleaseRows(FOUR_ROWS,rghRow);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(23)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCRowsetUpdate::Variation_23()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(24)
//*-----------------------------------------------------------------------
// @mfunc Requery while Updating
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCRowsetUpdate::Variation_24()
{
	TBEGIN
	INIT_THREADS(MAX_THREADS);	
	CRowsetUpdate RowsetA;

	const int cRows = FOUR_ROWS;
	HROW rghRow[FOUR_ROWS] = {NULL,NULL,NULL,NULL};

	//Setup Thread Arguments
	THREADARG T1Arg         = { &RowsetA };
	THREADARG T2Arg         = { &RowsetA };

	//CreateRowset
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);

	//Obtain row handle(s)
	TESTC_(RowsetA.GetRow(FIRST_ROW,THREE_ROWS,rghRow),S_OK)	

	//Make some change(s)
	TESTC_(RowsetA.ModifyRow(rghRow[ROW_ONE]),S_OK)
	if(RowsetA.AllowPendingRows(2))
		TESTC_(RowsetA.InsertRow(&rghRow[ROW_FOUR]),S_OK)

	//Create Threads
	CREATE_FIRST_THREADS(Thread_Update,&T1Arg);
	CREATE_SECOND_THREADS(Thread_Requery,&T2Arg);

	START_THREADS();
	END_THREADS();	

CLEANUP:
	//Release handle(s)
	RowsetA.ReleaseRows(FOUR_ROWS,rghRow);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(25)
//*-----------------------------------------------------------------------
// @mfunc Requerying while Undo
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCRowsetUpdate::Variation_25()
{
	TBEGIN
	INIT_THREADS(MAX_THREADS);	
	CRowsetUpdate RowsetA;

	const int cRows = FOUR_ROWS;
	HROW rghRow[FOUR_ROWS] = {NULL,NULL,NULL,NULL};

	//Setup Thread Arguments
	THREADARG T1Arg         = { &RowsetA };
	THREADARG T2Arg         = { &RowsetA };

	//CreateRowset
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);

	//Obtain row handle(s)
	TESTC_(RowsetA.GetRow(FIRST_ROW,THREE_ROWS,rghRow),S_OK)	

	//Make some change(s)
	TESTC_(RowsetA.HardDeleteRow(rghRow[ROW_THREE]),S_OK)
	TESTC_(RowsetA.ModifyRow(rghRow[ROW_ONE]),S_OK)
	if(RowsetA.AllowPendingRows(2))
		TESTC_(RowsetA.InsertRow(&rghRow[ROW_FOUR]),S_OK)

	//Create Threads
	CREATE_FIRST_THREADS(Thread_Requery,&T1Arg);
	CREATE_SECOND_THREADS(Thread_Undo,&T2Arg);

	START_THREADS();
	END_THREADS();	

CLEANUP:
	//Release handle(s)
	RowsetA.ReleaseRows(FOUR_ROWS,rghRow);
	TableInsert(ONE_ROW);	//Adjust the table
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(26)
//*-----------------------------------------------------------------------
// @mfunc Requery while GetOriginalData
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCRowsetUpdate::Variation_26()
{
	TBEGIN
	INIT_THREADS(MAX_THREADS);	
	CRowsetUpdate RowsetA;
	HROW rghRow[FOUR_ROWS] = {NULL,NULL,NULL,NULL};

	//Setup Thread Arguments
	THREADARG T1Arg         = { &RowsetA, &rghRow[ROW_ONE], &RowsetA.m_hAccessor };
	THREADARG T2Arg         = { &RowsetA };

	//CreateRowset
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS)==S_OK);

	//Obtain row handle(s)
	TESTC_(RowsetA.GetRow(FIRST_ROW,THREE_ROWS,rghRow),S_OK)	

	//Make some change(s)
	TESTC_(RowsetA.ModifyRow(rghRow[ROW_ONE]),S_OK)
	if(RowsetA.AllowPendingRows(2))
		TESTC_(RowsetA.InsertRow(&rghRow[ROW_FOUR]),S_OK)

	//Create Threads
	CREATE_FIRST_THREADS(Thread_GetOriginalData,&T1Arg);
	CREATE_SECOND_THREADS(Thread_Requery,&T2Arg);

	START_THREADS();
	END_THREADS();	

CLEANUP:
	RowsetA.ReleaseRows(FOUR_ROWS,rghRow);
	TRETURN
}
// }}
// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCRowsetUpdate::Terminate()
{
	

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CThreads::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCConnections)
//*-----------------------------------------------------------------------
//| Test Case:		TCConnections - Multi-Threaded testing of the RowsetNotify  and related Objects
//|	Created:			05/28/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCConnections::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CThreads::Init())
	// }}
	{
		TEST_PROVIDER(SupportedInterface(IID_IRowsetNotify, ROWSET_INTERFACE));
			return TRUE;
	}

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Multiple Listeners on sep threads, ROW_ACTIVE sent to all threads on their own thread
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCConnections::Variation_1()
{
	
	TBEGIN
	INIT_THREADS(THREE_THREADS);	
	CRowset RowsetA;

	DBCOUNTITEM cRowsObtained = 0;
	HROW* rghRow = NULL;

	//Setup Thread Arguments
	THREADARG T1Arg         = { &RowsetA };

	//CreateRowset
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_IConnectionPointContainer)==S_OK);

	//Create Threads
	CREATE_THREAD(THREAD_ONE,   Thread_CreateListener,&T1Arg);
	CREATE_THREAD(THREAD_TWO,   Thread_CreateListener,&T1Arg);
	CREATE_THREAD(THREAD_THREE, Thread_CreateListener,&T1Arg);

	START_THREADS();
	
	SleepEx(3000, FALSE);  //Let the other threads actually get hookup and Listening

	//Now cause the ROW_ACTIVE notifciation to occur
	//Our methods might not be thread-safe, so make a direct call to GetNextRows
	QTESTC_(RowsetA()->GetNextRows(NULL,0,ONE_ROW,&cRowsObtained,&rghRow),S_OK)
	
	END_THREADS();	

CLEANUP:
	//Release handle(s)
	RowsetA.ReleaseRows(ONE_ROW,rghRow);
	PROVIDER_FREE(rghRow);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Multiple Listeners on sep threads, GetNextRows / Modify - all notifcations go to all threads on their own thread
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCConnections::Variation_2()
{
	
	TBEGIN
	INIT_THREADS(THREE_THREADS);	
	CRowset RowsetA;

	DBCOUNTITEM cRowsObtained = 0;
	HROW* rghRow = NULL;

	//Setup Thread Arguments
	THREADARG T1Arg         = { &RowsetA };

	//CreateRowset
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_IConnectionPointContainer)==S_OK);

	//Create Threads
	CREATE_THREAD(THREAD_ONE,   Thread_CreateListener,&T1Arg);
	CREATE_THREAD(THREAD_TWO,   Thread_CreateListener,&T1Arg);
	CREATE_THREAD(THREAD_THREE, Thread_CreateListener,&T1Arg);

	START_THREADS();
	
	SleepEx(3000, FALSE);  //Let the other threads actually get hookup and Listening

	//Now cause the ROW_ACTIVE notifciation to occur
	//Our methods might not be thread-safe, so make a direct call to GetNextRows
	QTESTC_(RowsetA()->GetNextRows(NULL,0,ONE_ROW,&cRowsObtained,&rghRow),S_OK)
	
	END_THREADS();	

CLEANUP:
	//Release handle(s)
	RowsetA.ReleaseRows(ONE_ROW,rghRow);
	PROVIDER_FREE(rghRow);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCConnections::Variation_3()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Multiple Listeners on sep threads, as well as the main thread being a listener
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCConnections::Variation_4()
{
	
	TBEGIN
	INIT_THREADS(THREE_THREADS);	
 	
	CRowset RowsetA;

	DBCOUNTITEM cRowsObtained = 0;
	HROW* rghRow = NULL;

	DWORD dwCookie = 0;
	CListener* pCListener = NULL;

	//Setup Thread Arguments
	THREADARG T1Arg         = { &RowsetA };

	//CreateRowset
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_IConnectionPointContainer)==S_OK);

	//Main thread Listener, off the Rowset
	pCListener = new CListener(IID_IRowsetNotify, RowsetA());
	TESTC(pCListener!=NULL);
	pCListener->AddRef();
	pCListener->Advise(&dwCookie);
	
	//Create Threads
	CREATE_THREAD(THREAD_ONE,   Thread_CreateListener,&T1Arg);
	CREATE_THREAD(THREAD_TWO,   Thread_CreateListener,&T1Arg);
	CREATE_THREAD(THREAD_THREE, Thread_CreateListener,&T1Arg);

	START_THREADS();
	
	SleepEx(3000, FALSE);  //Let the other threads actually get hookup and Listening

	//Now cause the ROW_ACTIVE notifciation to occur
	//Our methods might not be thread-safe, so make a direct call to GetNextRows
	QTESTC_(RowsetA()->GetNextRows(NULL,0,ONE_ROW,&cRowsObtained,&rghRow),S_OK)
	END_THREADS();	

	//Verify the main thread was also notified
	TESTC(pCListener->GetTimesNotified() > 0) 

CLEANUP:
	//Release handle(s)
	if(pCListener)
		pCListener->Unadvise(dwCookie);
	SAFE_RELEASE(pCListener);
	
	RowsetA.ReleaseRows(ONE_ROW,rghRow);
	PROVIDER_FREE(rghRow);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCConnections::Variation_5()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Two listeners on the same thread, as well as other single threaded listeners
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCConnections::Variation_6()
{
	
	TBEGIN
	INIT_THREADS(THREE_THREADS);	
	CRowset RowsetA;

	DBCOUNTITEM cRowsObtained = 0;
	HROW* rghRow = NULL;

	//Setup Thread Arguments
	THREADARG T1Arg         = { &RowsetA };

	//CreateRowset
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_IConnectionPointContainer)==S_OK);

	//Create Threads
	CREATE_THREAD(THREAD_ONE,   Thread_CreateListener,&T1Arg);
	CREATE_THREAD(THREAD_TWO,   Thread_CreateListener,&T1Arg);
	CREATE_THREAD(THREAD_THREE, Thread_CreateListener,&T1Arg);

	START_THREADS();
	
	SleepEx(3000, FALSE);  //Let the other threads actually get hookup and Listening

	//Now cause the ROW_ACTIVE notifciation to occur
	//Our methods might not be thread-safe, so make a direct call to GetNextRows
	QTESTC_(RowsetA()->GetNextRows(NULL,0,ONE_ROW,&cRowsObtained,&rghRow),S_OK)
	
	END_THREADS();	

CLEANUP:
	//Release handle(s)
	RowsetA.ReleaseRows(ONE_ROW,rghRow);
	PROVIDER_FREE(rghRow);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCConnections::Variation_7()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc All apartment model listeners
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCConnections::Variation_8()
{
	

	//TODO V2

	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc Some Apartment, Some Free-Threaded lsiteners
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCConnections::Variation_9()
{
	
	TBEGIN
	INIT_THREADS(THREE_THREADS);	
	CRowset RowsetA;

	DBCOUNTITEM cRowsObtained = 0;
	HROW* rghRow = NULL;

	//Setup Thread Arguments
	THREADARG T1Arg         = { &RowsetA };

	//CreateRowset
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_IConnectionPointContainer)==S_OK);

	//Create Threads
	CREATE_THREAD(THREAD_ONE,   Thread_CreateListener,&T1Arg);
	CREATE_THREAD(THREAD_TWO,   Thread_CreateListener,&T1Arg);
	CREATE_THREAD(THREAD_THREE, Thread_CreateListener,&T1Arg);

	//TODO V2 ApartmentModel Listeners

	START_THREADS();
	
	SleepEx(3000, FALSE);  //Let the other threads actually get hookup and Listening

	//Now cause the ROW_ACTIVE notifciation to occur
	//Our methods might not be thread-safe, so make a direct call to GetNextRows
	QTESTC_(RowsetA()->GetNextRows(NULL,0,ONE_ROW,&cRowsObtained,&rghRow),S_OK)
	
	END_THREADS();	

CLEANUP:
	//Release handle(s)
	RowsetA.ReleaseRows(ONE_ROW,rghRow);
	PROVIDER_FREE(rghRow);
	TRETURN
}
// }}
// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCConnections::Terminate()
{
	

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CThreads::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCSequence)
//*-----------------------------------------------------------------------
//| Test Case:		TCSequence - Multi-Threaded testing of the various Rowset Objects together
//|	Created:			05/28/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCSequence::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CThreads::Init())
	// }}
	{
		if(CreateRowset(DBPROP_CANHOLDROWS, SELECT_VALIDATIONORDER)==S_OK)
			return TRUE;
	}

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc SetData multiple times
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSequence::Variation_1()
{
	TBEGIN
	INIT_THREADS(MAX_THREADS);	
	CRowsetChange RowsetA;

	HROW hRow = NULL;
	HACCESSOR hAccessor = NULL;

	//Setup Thread Arguments
	THREADARG T1Arg         = { &RowsetA, &hRow, &hAccessor, (void*)S_OK };
	THREADARG T2Arg         = { &RowsetA, &hRow, &hAccessor, (void*)S_OK }; 

	//CreateRowset
	TESTC_PROVIDER(RowsetA.CreateRowset(SELECT_VALIDATIONORDER)==S_OK);

	//Since SQLServer has a problem trying to reposition on a row if the index 
	//column has changed, don't change the index column, bind all columns except that one
	TESTC_(GetAccessorAndBindings(RowsetA.pIAccessor(), DBACCESSOR_ROWDATA, 
		&hAccessor,	NULL, NULL, NULL, DBPART_ALL, UPDATEABLE_NONINDEX_COLS_BOUND),S_OK); 

	//Obtain row handle(s)
	TESTC_(RowsetA.GetRow(FIRST_ROW,&hRow),S_OK)	

	//Create Threads
	CREATE_FIRST_THREADS(Thread_SetData,&T1Arg);
	CREATE_SECOND_THREADS(Thread_SetData,&T2Arg);

	START_THREADS();
	END_THREADS();	

CLEANUP:
	//Release handle(s)
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc SetData for different columns within the same row
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSequence::Variation_2()
{
	TBEGIN
	INIT_THREADS(MAX_THREADS);	
	CRowsetChange RowsetA;

	HROW hRow = NULL;
	HACCESSOR hAccessor = NULL;

	//Setup Thread Arguments
	THREADARG T1Arg         = { &RowsetA, &hRow, &hAccessor, (void*)S_OK };
	THREADARG T2Arg         = { &RowsetA, &hRow, &hAccessor, (void*)S_OK }; 

	//CreateRowset
	TESTC_PROVIDER(RowsetA.CreateRowset(SELECT_VALIDATIONORDER)==S_OK);

	//Since SQLServer has a problem trying to reposition on a row if the index 
	//column has changed, don't change the index column, bind all columns except that one
	TESTC_(GetAccessorAndBindings(RowsetA.pIAccessor(), DBACCESSOR_ROWDATA, 
		&hAccessor,	NULL, NULL, NULL, DBPART_ALL, UPDATEABLE_NONINDEX_COLS_BOUND),S_OK); 

	//Obtain row handle(s)
	TESTC_(RowsetA.GetRow(FIRST_ROW,&hRow),S_OK)	

	//Create Threads
	CREATE_FIRST_THREADS(Thread_SetData,&T1Arg);
	CREATE_SECOND_THREADS(Thread_SetData,&T2Arg);

	START_THREADS();
	END_THREADS();	

CLEANUP:
	//Release handle(s)
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc SetData and GetData for the same column/row
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSequence::Variation_3()
{
	TBEGIN
	INIT_THREADS(MAX_THREADS);	
	CRowsetChange RowsetA;
	HROW hRow = NULL;
	HACCESSOR hAccessor = NULL;

	//Setup Thread Arguments
	THREADARG T1Arg         = { &RowsetA, &hRow, &hAccessor, (void*)S_OK };
	THREADARG T2Arg         = { &RowsetA, (void*)ONE_ROW, &hRow, &hAccessor, (void*)S_OK };

	//CreateRowset
	TESTC_PROVIDER(RowsetA.CreateRowset()==S_OK);

	//Since SQLServer has a problem trying to reposition on a row if the index 
	//column has changed, don't change the index column, bind all columns except that one
	TESTC_(GetAccessorAndBindings(RowsetA.pIAccessor(), DBACCESSOR_ROWDATA, 
		&hAccessor,	NULL, NULL, NULL, DBPART_ALL, UPDATEABLE_NONINDEX_COLS_BOUND),S_OK); 

	//Obtain row handle(s)
	TESTC_(RowsetA.GetRow(FIRST_ROW,&hRow),S_OK)	

	//Create Threads
	CREATE_FIRST_THREADS(Thread_SetData,&T1Arg);
	CREATE_SECOND_THREADS(Thread_GetData,&T2Arg);

	START_THREADS();
	END_THREADS();	

CLEANUP:
	//Release handle(s)
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSequence::Variation_4()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc SetData and Resynch
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSequence::Variation_5()
{
	TBEGIN
	INIT_THREADS(MAX_THREADS);	
	CRowsetChange RowsetA;	 

	HROW hRow = NULL;
	HACCESSOR hAccessor = NULL;

	//Setup Thread Arguments
	THREADARG T1Arg         = { &RowsetA, &hRow, &hAccessor, (void*)S_OK };
	THREADARG T2Arg         = { &RowsetA }; 

	//CreateRowset
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_IRowsetResynch, SELECT_VALIDATIONORDER)==S_OK);

	//Since SQLServer has a problem trying to reposition on a row if the index 
	//column has changed, don't change the index column, bind all columns except that one
	TESTC_(GetAccessorAndBindings(RowsetA.pIAccessor(), DBACCESSOR_ROWDATA, 
		&hAccessor,	NULL, NULL, NULL, DBPART_ALL, UPDATEABLE_NONINDEX_COLS_BOUND),S_OK); 

	//Obtain row handle(s)
	TESTC_(RowsetA.GetRow(FIRST_ROW,&hRow),S_OK)	

	//Create Threads
	CREATE_FIRST_THREADS(Thread_SetData,&T1Arg);
	CREATE_SECOND_THREADS(Thread_ResynchRows,&T2Arg);

	START_THREADS();
	END_THREADS();	

CLEANUP:
	//Release handle(s)
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc GetData and Resynch
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSequence::Variation_6()
{
	TBEGIN
	INIT_THREADS(MAX_THREADS);	
	CRowsetChange RowsetA;	
	HROW hRow = NULL;

	//Setup Thread Arguments
	THREADARG T1Arg         = { &RowsetA, (void*)ONE_ROW, &hRow, &RowsetA.m_hAccessor, (void*)S_OK };
	THREADARG T2Arg         = { &RowsetA }; 

	//CreateRowset
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_IRowsetResynch, SELECT_VALIDATIONORDER)==S_OK);

	//Obtain row handle(s)
	TESTC_(RowsetA.GetRow(FIRST_ROW,&hRow),S_OK)	

	//Create Threads
	CREATE_FIRST_THREADS(Thread_GetData,&T1Arg);
	CREATE_SECOND_THREADS(Thread_ResynchRows,&T2Arg);

	START_THREADS();
	END_THREADS();	

CLEANUP:
	//Release handle(s)
	RowsetA.ReleaseRows(hRow);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSequence::Variation_7()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc Insert a row while GetNextRows
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSequence::Variation_8()
{
	TBEGIN
	INIT_THREADS(MAX_THREADS);	
	CRowsetChange RowsetA;

	//Setup Thread Arguments
	THREADARG T1Arg         = { &RowsetA, &RowsetA.m_hAccessor, (void*)S_OK };
	THREADARG T2Arg         = { &RowsetA, (void*)0, (void*)ONE_ROW, (void*)S_OK };

	//CreateRowset
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS, SELECT_VALIDATIONORDER)==S_OK);

	//Create Threads
	CREATE_FIRST_THREADS(Thread_InsertRow,&T1Arg);
	CREATE_SECOND_THREADS(Thread_GetNextRows,&T2Arg);

	START_THREADS();
	END_THREADS();	

CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc Insert a row while GetData
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSequence::Variation_9()
{
	TBEGIN
	INIT_THREADS(MAX_THREADS);	
	CRowsetChange RowsetA;
	HROW hRow = NULL;

	//Setup Thread Arguments
	THREADARG T1Arg         = { &RowsetA, &RowsetA.m_hAccessor, (void*)S_OK };
	THREADARG T2Arg         = { &RowsetA, (void*)ONE_ROW, &hRow, &RowsetA.m_hAccessor, (void*)S_OK };

	//CreateRowset
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS, SELECT_VALIDATIONORDER)==S_OK);

	//Obtain row handle(s)
	TESTC_(RowsetA.GetRow(FIRST_ROW,&hRow),S_OK)	

	//Create Threads
	CREATE_FIRST_THREADS(Thread_InsertRow,&T1Arg);
	CREATE_SECOND_THREADS(Thread_GetData,&T2Arg);

	START_THREADS();
	END_THREADS();	

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc Insert a row while DeleteRows
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSequence::Variation_10()
{
	TBEGIN
	INIT_THREADS(MAX_THREADS);	
	CRowsetChange RowsetA;
	HROW rghRow[FOUR_ROWS] = {NULL,NULL,NULL,NULL};

	//Setup Thread Arguments
	THREADARG T1Arg         = { &RowsetA, &RowsetA.m_hAccessor, (void*)S_OK };
	THREADARG T2Arg         = { &RowsetA, (void*)FOUR_ROWS, rghRow, (void*)DB_E_ERRORSOCCURRED, (void*)S_OK };

	//CreateRowset
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS, SELECT_VALIDATIONORDER)==S_OK);

	//Obtain row handle(s)
	TESTC_(RowsetA.GetRow(FIRST_ROW, FOUR_ROWS, rghRow),S_OK)	

	//Create Threads
	CREATE_FIRST_THREADS(Thread_InsertRow,&T1Arg);
	CREATE_SECOND_THREADS(Thread_DeleteRows,&T2Arg);

	START_THREADS();
	END_THREADS();	

CLEANUP:
	RowsetA.ReleaseRows(FOUR_ROWS, rghRow);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSequence::Variation_11()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc Modify a row while GetNextRows
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSequence::Variation_12()
{
	TBEGIN
	INIT_THREADS(MAX_THREADS);	
	CRowsetChange RowsetA;
	HROW hRow = NULL;
	HACCESSOR hAccessor = NULL;

	//Setup Thread Arguments
	THREADARG T1Arg         = { &RowsetA, &hRow, &hAccessor, (void*)S_OK };
	THREADARG T2Arg         = { &RowsetA, (void*)0, (void*)ONE_ROW, (void*)DB_S_ENDOFROWSET };
	
	//CreateRowset
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS, SELECT_VALIDATIONORDER)==S_OK);

	//Since SQLServer has a problem trying to reposition on a row if the index 
	//column has changed, don't change the index column, bind all columns except that one
	TESTC_(GetAccessorAndBindings(RowsetA.pIAccessor(), DBACCESSOR_ROWDATA, 
		&hAccessor,	NULL, NULL, NULL, DBPART_ALL, UPDATEABLE_NONINDEX_COLS_BOUND),S_OK); 

	//Obtain row handle(s)
	TESTC_(RowsetA.GetRow(FIRST_ROW,&hRow),S_OK)	

	//Create Threads
	CREATE_FIRST_THREADS(Thread_SetData,&T1Arg);
	CREATE_SECOND_THREADS(Thread_GetNextRows,&T2Arg);

	START_THREADS();
	END_THREADS();	

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc Modify a row while GetData
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSequence::Variation_13()
{
	TBEGIN
	INIT_THREADS(MAX_THREADS);	
	CRowsetChange RowsetA;
	HROW hRow = NULL;
	HACCESSOR hAccessor = NULL;

	//Setup Thread Arguments
	THREADARG T1Arg         = { &RowsetA, &hRow, &hAccessor, (void*)S_OK };
	THREADARG T2Arg         = { &RowsetA, (void*)ONE_ROW, &hRow, &hAccessor, (void*)S_OK };

	//CreateRowset
	TESTC_PROVIDER(RowsetA.CreateRowset(SELECT_VALIDATIONORDER)==S_OK);

	//Since SQLServer has a problem trying to reposition on a row if the index 
	//column has changed, don't change the index column, bind all columns except that one
	TESTC_(GetAccessorAndBindings(RowsetA.pIAccessor(), DBACCESSOR_ROWDATA, 
		&hAccessor,	NULL, NULL, NULL, DBPART_ALL, UPDATEABLE_NONINDEX_COLS_BOUND),S_OK); 

	//Obtain row handle(s)
	TESTC_(RowsetA.GetRow(FIRST_ROW,&hRow),S_OK)	

	//Create Threads
	CREATE_FIRST_THREADS(Thread_SetData,&T1Arg);
	CREATE_SECOND_THREADS(Thread_GetData,&T2Arg);

	START_THREADS();
	END_THREADS();	

CLEANUP:
	//Release handle(s)
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc Modify a row while ReleaseRows
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSequence::Variation_14()
{
	TBEGIN
	INIT_THREADS(MAX_THREADS);	
	CRowsetChange RowsetA;
	HROW hRow = NULL;
	HACCESSOR hAccessor = NULL;

	//Setup Thread Arguments
	THREADARG T1Arg         = { &RowsetA, &hRow, &hAccessor, (void*)DB_E_BADROWHANDLE };
	THREADARG T2Arg         = { &RowsetA, &hRow, (void*)DB_E_ERRORSOCCURRED }; 

	//CreateRowset
	TESTC_PROVIDER(RowsetA.CreateRowset(SELECT_VALIDATIONORDER)==S_OK);

	//Since SQLServer has a problem trying to reposition on a row if the index 
	//column has changed, don't change the index column, bind all columns except that one
	TESTC_(GetAccessorAndBindings(RowsetA.pIAccessor(), DBACCESSOR_ROWDATA, 
		&hAccessor,	NULL, NULL, NULL, DBPART_ALL, UPDATEABLE_NONINDEX_COLS_BOUND),S_OK); 

	//Obtain row handle(s)
	TESTC_(RowsetA.GetRow(FIRST_ROW,&hRow),S_OK)	

	//Create Threads
	CREATE_FIRST_THREADS(Thread_SetData,&T1Arg);
	CREATE_SECOND_THREADS(Thread_ReleaseRows,&T2Arg);

	START_THREADS();
	END_THREADS();	

CLEANUP:
	RowsetA.ReleaseAccessor(hAccessor);
	RowsetA.ReleaseRows(hRow);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSequence::Variation_15()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc Delete a row, while GetNextRows
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSequence::Variation_16()
{
	TBEGIN
	INIT_THREADS(TWO_THREADS);	
	
	CRowsetChange RowsetA;
	HROW hRow = NULL;
	
	//Setup Thread Arguments
	THREADARG T1Arg         = { &RowsetA, (void*)ONE_ROW, &hRow, (void*)DB_E_ERRORSOCCURRED, (void*)S_OK };
	THREADARG T2Arg         = { &RowsetA, (void*)0, (void*)ONE_ROW, (void*)DB_E_BADSTARTPOSITION };

	//CreateRowset
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS, SELECT_VALIDATIONORDER)==S_OK);

	//Obtain row handle(s)
	TESTC_(RowsetA.GetRow(FIRST_ROW,&hRow),S_OK)	

	//Create Threads
	CREATE_FIRST_THREADS(Thread_DeleteRows,&T1Arg);
	CREATE_SECOND_THREADS(Thread_GetNextRows,&T2Arg);

	START_THREADS();
	END_THREADS();	

CLEANUP:
	//Release handle(s)
	RowsetA.ReleaseRows(hRow);
	TableInsert(ONE_ROW);	//Adjust the table
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc Delete a row, while GetData
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSequence::Variation_17()
{
	TBEGIN
	INIT_THREADS(TWO_THREADS);	
	CRowsetChange RowsetA;
	HROW rghRows[TWO_ROWS] = {NULL,NULL};

	//NOTE: Its invalid to call GetData with a deleterow handle.  Its totally
	//provider specific.  So the best we can do is to call GetData and DeleteRows
	//from seperate threads on different row handles, becuase the DeleteRows might
	//get the before the GetData, possibly causing it to crash...
	
	//Setup Thread Arguments
	THREADARG T1Arg         = { &RowsetA, (void*)ONE_ROW, &rghRows[ROW_ONE], &RowsetA.m_hAccessor, (void*)S_OK };
	THREADARG T2Arg         = { &RowsetA, (void*)ONE_ROW, &rghRows[ROW_TWO], (void*)DB_E_ERRORSOCCURRED, (void*)S_OK };

	//CreateRowset
	TESTC_PROVIDER(RowsetA.CreateRowset(SELECT_VALIDATIONORDER)==S_OK);

	//Obtain row handle(s)
	TESTC_(RowsetA.GetRow(FIRST_ROW, TWO_ROWS, rghRows),S_OK)	

	//Create Threads
	CREATE_FIRST_THREADS(Thread_GetData,&T1Arg);
	CREATE_SECOND_THREADS(Thread_DeleteRows,&T2Arg);

	START_THREADS();
	END_THREADS();	

CLEANUP:
	//Release handle(s)
	RowsetA.ReleaseRows(TWO_ROWS,rghRows);
	TableInsert(ONE_ROW);	//Adjust the table
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc Delete a row, while SetData
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSequence::Variation_18()
{
	TBEGIN
	INIT_THREADS(MAX_THREADS);	
	CRowsetChange RowsetA;
	HROW hRow = NULL;
	HACCESSOR hAccessor = NULL;

	//Setup Thread Arguments
	THREADARG T1Arg         = { &RowsetA, &hRow, &hAccessor, (void*)DB_E_DELETEDROW };
	THREADARG T2Arg         = { &RowsetA, (void*)ONE_ROW, &hRow, (void*)DB_E_ERRORSOCCURRED, (void*)S_OK };

	//CreateRowset
	TESTC_PROVIDER(RowsetA.CreateRowset(SELECT_VALIDATIONORDER)==S_OK);

	//Since SQLServer has a problem trying to reposition on a row if the index 
	//column has changed, don't change the index column, bind all columns except that one
	TESTC_(GetAccessorAndBindings(RowsetA.pIAccessor(), DBACCESSOR_ROWDATA, 
		&hAccessor,	NULL, NULL, NULL, DBPART_ALL, UPDATEABLE_NONINDEX_COLS_BOUND),S_OK); 

	//Obtain row handle(s)
	TESTC_(RowsetA.GetRow(FIRST_ROW,&hRow),S_OK)

	//Create Threads
	CREATE_FIRST_THREADS(Thread_SetData,&T1Arg);
	CREATE_SECOND_THREADS(Thread_DeleteRows,&T2Arg);

	START_THREADS();
	END_THREADS();	

CLEANUP:
	//Release handle(s)
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor);
	TableInsert(ONE_ROW);	//Adjust the table
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc Delete a row, while ReleaseRows
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSequence::Variation_19()
{
	TBEGIN
	INIT_THREADS(TWO_THREADS);	
	CRowsetChange RowsetA;
	HROW hRow = NULL;

	//Setup Thread Arguments
	THREADARG T1Arg         = { &RowsetA, (void*)ONE_ROW, &hRow, (void*)DB_E_ERRORSOCCURRED, (void*)S_OK };
	THREADARG T2Arg         = { &RowsetA, &hRow, (void*)S_OK };

	//CreateRowset
	TESTC_PROVIDER(RowsetA.CreateRowset(SELECT_VALIDATIONORDER)==S_OK);

	//Obtain row handle(s)
	TESTC_(RowsetA.GetRow(FIRST_ROW,&hRow),S_OK)	

	//Create Threads
	CREATE_FIRST_THREADS(Thread_DeleteRows,&T1Arg);
	CREATE_SECOND_THREADS(Thread_ReleaseRows,&T2Arg);

	START_THREADS();
	WAITFOR_THREADS();
	
	END_THREADS();	

CLEANUP:
	//Release handle(s)
	RowsetA.ReleaseRows(hRow);
	TableInsert(ONE_ROW);	//Adjust the table
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSequence::Variation_20()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc Requery while DeleteRows
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSequence::Variation_21()
{
	TBEGIN
	INIT_THREADS(TWO_THREADS);	
	CRowsetChange RowsetA;
	HROW hRow = NULL;

	//Setup Thread Arguments
	THREADARG T1Arg         = { &RowsetA, (void*)ONE_ROW, &hRow, (void*)DB_E_ERRORSOCCURRED, (void*)S_OK };
	THREADARG T2Arg         = { &RowsetA };

	//CreateRowset
	TESTC_PROVIDER(RowsetA.CreateRowset(SELECT_VALIDATIONORDER)==S_OK);

	//Obtain row handle(s)
	TESTC_(RowsetA.GetRow(FIRST_ROW,&hRow),S_OK)	

	//Create Threads
	CREATE_FIRST_THREADS(Thread_DeleteRows,&T1Arg);
	CREATE_SECOND_THREADS(Thread_Requery,&T2Arg);

	START_THREADS();
	END_THREADS();	

CLEANUP:
	//Release handle(s)
	RowsetA.ReleaseRows(hRow);
	TableInsert(ONE_ROW);	//Adjust the table
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(22)
//*-----------------------------------------------------------------------
// @mfunc Requery while Modifying and row
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSequence::Variation_22()
{
	TBEGIN
	INIT_THREADS(MAX_THREADS);	
	CRowsetChange RowsetA;
	HROW hRow = NULL;
	HACCESSOR hAccessor = NULL;

	//Setup Thread Arguments
	THREADARG T1Arg         = { &RowsetA, &hRow, &hAccessor, (void*)S_OK };
	THREADARG T2Arg         = { &RowsetA }; 

	//CreateRowset
	TESTC_PROVIDER(RowsetA.CreateRowset(SELECT_VALIDATIONORDER)==S_OK);

	//Since SQLServer has a problem trying to reposition on a row if the index 
	//column has changed, don't change the index column, bind all columns except that one
	TESTC_(GetAccessorAndBindings(RowsetA.pIAccessor(), DBACCESSOR_ROWDATA, 
		&hAccessor,	NULL, NULL, NULL, DBPART_ALL, UPDATEABLE_NONINDEX_COLS_BOUND),S_OK); 

	//Obtain row handle(s)
	TESTC_(RowsetA.GetRow(FIRST_ROW,&hRow),S_OK)	

	//Create Threads
	CREATE_FIRST_THREADS(Thread_SetData,&T1Arg);
	CREATE_SECOND_THREADS(Thread_Requery,&T2Arg);

	START_THREADS();
	END_THREADS();	

CLEANUP:
	RowsetA.ReleaseRows(hAccessor);
	RowsetA.ReleaseRows(hRow);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(23)
//*-----------------------------------------------------------------------
// @mfunc Requery while Inserting a row
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSequence::Variation_23()
{
	TBEGIN
	INIT_THREADS(MAX_THREADS);	
	CRowsetChange RowsetA;

	//Setup Thread Arguments
	THREADARG T1Arg         = { &RowsetA, &RowsetA.m_hAccessor, (void*)S_OK };
	THREADARG T2Arg         = { &RowsetA }; 

	//CreateRowset
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS, SELECT_VALIDATIONORDER)==S_OK);

	//Create Threads
	CREATE_FIRST_THREADS(Thread_InsertRow,&T1Arg);
	CREATE_SECOND_THREADS(Thread_Requery,&T2Arg);

	START_THREADS();
	END_THREADS();	

CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(24)
//*-----------------------------------------------------------------------
// @mfunc Requery while GetNextRows
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSequence::Variation_24()
{
	TBEGIN
	INIT_THREADS(TWO_THREADS);	
	//CRowsetChange RowsetA;
	CRowset RowsetA;

	//Setup Thread Arguments
	THREADARG T1Arg         = { &RowsetA, (void*)0, (void*)ONE_ROW, (void*)S_OK };

	//CreateRowset
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS, SELECT_VALIDATIONORDER)==S_OK);

	//Create Threads
	CREATE_FIRST_THREADS(Thread_Requery,&T1Arg);
	CREATE_SECOND_THREADS(Thread_GetNextRows,&T1Arg);

	START_THREADS();
	END_THREADS();	

CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(25)
//*-----------------------------------------------------------------------
// @mfunc Requery while GetData
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSequence::Variation_25()
{
	TBEGIN
	INIT_THREADS(TWO_THREADS);	
	CRowset RowsetA;
	HROW hRow = NULL;

	//Setup Thread Arguments
	THREADARG T1Arg         = { &RowsetA, (void*)ONE_ROW, &hRow, &RowsetA.m_hAccessor, (void*)S_OK };
	THREADARG T2Arg         = { &RowsetA }; 

	//CreateRowset
	TESTC_PROVIDER(RowsetA.CreateRowset(SELECT_VALIDATIONORDER)==S_OK);

	//Obtain row handle(s)
	TESTC_(RowsetA.GetRow(FIRST_ROW,&hRow),S_OK)	

	//Create Threads
	CREATE_FIRST_THREADS(Thread_GetData,&T1Arg);
	CREATE_SECOND_THREADS(Thread_Requery,&T2Arg);

	START_THREADS();
	END_THREADS();	

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(26)
//*-----------------------------------------------------------------------
// @mfunc Requery while RestartPosition (causing a requery
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSequence::Variation_26()
{
	TBEGIN
	INIT_THREADS(MAX_THREADS);	
	//CRowsetChange RowsetA;
	CRowset RowsetA;

	//Setup Thread Arguments
	THREADARG T1Arg         = { &RowsetA, (void*)TRUE };
	THREADARG T2Arg         = { &RowsetA }; 

	//CreateRowset
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS, SELECT_VALIDATIONORDER)==S_OK);

	//Create Threads
	CREATE_FIRST_THREADS(Thread_RestartPosition,&T1Arg);
	CREATE_SECOND_THREADS(Thread_Requery,&T2Arg);

	START_THREADS();
	END_THREADS();	

CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(27)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSequence::Variation_27()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(28)
//*-----------------------------------------------------------------------
// @mfunc GetNextRows while Locate
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSequence::Variation_28()
{
	TBEGIN
	INIT_THREADS(MAX_THREADS);	
	CRowset RowsetA;	 

	//Setup Thread Arguments
	THREADARG T1Arg         = { &RowsetA, (void*)0, (void*)ONE_ROW, (void*)DB_S_ENDOFROWSET };

	//CreateRowset
	RowsetA.SetProperty(DBPROP_IRowsetLocate);
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS, SELECT_VALIDATIONORDER)==S_OK);

	//Create Threads
	CREATE_FIRST_THREADS(Thread_GetNextRows,&T1Arg);
	CREATE_SECOND_THREADS(Thread_GetRowsAt,&T1Arg);

	START_THREADS();
	END_THREADS();	

CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(29)
//*-----------------------------------------------------------------------
// @mfunc GetData while Locate
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSequence::Variation_29()
{
	TBEGIN
	INIT_THREADS(MAX_THREADS);	
	CRowset RowsetA;		  
	HROW hRow = NULL;

	//Setup Thread Arguments
	THREADARG T1Arg = { &RowsetA, (void*)ONE_ROW, &hRow, &RowsetA.m_hAccessor, (void*)S_OK };
	THREADARG T2Arg = { &RowsetA };

	//CreateRowset
	RowsetA.SetProperty(DBPROP_IRowsetLocate);
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS, SELECT_VALIDATIONORDER)==S_OK);

	//Obtain row handle(s)
	TESTC_(RowsetA.GetRow(FIRST_ROW,&hRow),S_OK)	

	//Create Threads
	CREATE_FIRST_THREADS(Thread_GetData,  &T1Arg);
	CREATE_SECOND_THREADS(Thread_GetRowsAt,&T2Arg);

	START_THREADS();
	END_THREADS();	

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(30)
//*-----------------------------------------------------------------------
// @mfunc RestartPosition while Locate
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSequence::Variation_30()
{
	TBEGIN
	INIT_THREADS(MAX_THREADS);	
	CRowset RowsetA;

	HROW hRow = NULL;

	//Setup Thread Arguments
	THREADARG T1Arg         = { &RowsetA, (void*)FALSE };//FALSE - rows not released
	THREADARG T2Arg         = { &RowsetA };

	//CreateRowset
	RowsetA.SetProperty(DBPROP_IRowsetLocate);
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_CANHOLDROWS, SELECT_VALIDATIONORDER)==S_OK);
	
	//Actually fetch a row to force requery
	TESTC_(RowsetA.GetRow(FIRST_ROW,&hRow),S_OK)	

	//Create Threads
	CREATE_FIRST_THREADS(Thread_RestartPosition, &T1Arg);
	CREATE_SECOND_THREADS(Thread_GetRowsAt, &T2Arg);

	START_THREADS();
	END_THREADS();	

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(31)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSequence::Variation_31()
{
	
	return TEST_PASS;
}
// }}
// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCSequence::Terminate()
{
	

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CThreads::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCClassFactory)
//*-----------------------------------------------------------------------
//| Test Case:		TCClassFactory - Multi-Threaded testing of the ClassFactory object
//|	Created:			05/30/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCClassFactory::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CThreads::Init())
	// }}
	{
		if(CreateRowset(DBPROP_CANHOLDROWS, SELECT_VALIDATIONORDER)==S_OK)
			return TRUE;
	}

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Call CoCreateobject instead of CoCreateInstance
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCClassFactory::Variation_1()
{
	
	TBEGIN
	INIT_THREADS(FOUR_THREADS);	
	
	IClassFactory* pIClassFactory = NULL;

	//Get the ClassFactory
	CoGetClassObject(PROVIDER_CLSID,CLSCTX_INPROC_SERVER,NULL,IID_IClassFactory,(void**)&pIClassFactory); 
	ASSERT(pIClassFactory);

	//Setup Thread Arguments
	THREADARG T1Arg         = { this };
	THREADARG T2Arg         = { pIClassFactory };
	
	//Create Threads
	CREATE_THREAD(THREAD_ONE,   Thread_GetClassFactory,&T1Arg);
	CREATE_THREAD(THREAD_TWO,   Thread_GetClassFactory,&T1Arg);

	CREATE_THREAD(THREAD_THREE, Thread_CreateInstance,&T2Arg);
	CREATE_THREAD(THREAD_FOUR,  Thread_CreateInstance,&T2Arg);

	START_THREADS();
	END_THREADS();	

//CLEANUP:
	SAFE_RELEASE(pIClassFactory);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCClassFactory::Variation_2()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc PersistFile save, load from sep threads
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCClassFactory::Variation_3()
{
	
	TBEGIN
	INIT_THREADS(MAX_THREADS);	
	IDBInitialize* pIDBInitialize = NULL;

	//Create another instance, since our current one from the privlib has too many
	//references to uninitilize
	GetModInfo()->CreateProvider(NULL, IID_IDBInitialize, (IUnknown**)&pIDBInitialize);

	//Setup thread arguments
	THREADARG T1Arg = { pIDBInitialize };
	TESTC(pIDBInitialize!=NULL);

	//Main Thread Saves
	TESTC_PROVIDER(SaveDSO(PERSIST_FILE)==S_OK);

	//Create Threads
	//Load from seperate threads
	CREATE_THREADS(Thread_PersistFileLoad, &T1Arg);

	START_THREADS();
	END_THREADS();	

CLEANUP:
	SAFE_RELEASE(pIDBInitialize);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Extended Error object
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCClassFactory::Variation_4()
{
	

	//TODO
	
	return TEST_PASS;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCClassFactory::Terminate()
{
	

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CThreads::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCRealWorld)
//*-----------------------------------------------------------------------
//| Test Case:		TCRealWorld - Real-World MultiThreaded senarios for OLEDB
//|	Created:			05/30/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCRealWorld::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CThreads::Init())
	// }}
	{
		TEST_PROVIDER(CreateRowset(DBPROP_CANHOLDROWS, SELECT_ALLFROMTBL)==S_OK);
		return TRUE;
	}

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Browser app, reading multiple columns concurrently
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCRealWorld::Variation_1()
{
	TBEGIN
	INIT_THREADS(MAX_THREADS);
	CRowset RowsetA;
	HROW hRow = NULL;

	//Setup Thread Arguments
	THREADARG T1Arg = { &RowsetA, (void*)ONE_ROW, &hRow, &RowsetA.m_hAccessor, (void*)S_OK };

	//CreateRowset
	TESTC_PROVIDER(RowsetA.CreateRowset(SELECT_ALLFROMTBL)==S_OK);

	//Obtain row handle(s)
	TESTC_(RowsetA.GetRow(FIRST_ROW,&hRow),S_OK)	

	//Create Threads
	CREATE_THREADS(Thread_GetData, &T1Arg);

	START_THREADS();
	END_THREADS();	

CLEANUP:
	TRETURN

}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc On-Line database
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCRealWorld::Variation_2()
{
	TBEGIN
	INIT_THREADS(MAX_THREADS);	
	CRowsetChange RowsetA;

	HROW hRow = NULL;
	HACCESSOR hAccessor = NULL;

	//Setup Thread Arguments
	THREADARG T1Arg         = { &RowsetA, &hRow, &hAccessor, (void*)S_OK };
	THREADARG T2Arg         = { &RowsetA, (void*)ONE_ROW, &hRow, &hAccessor, (void*)S_OK };

	//CreateRowset
	TESTC_PROVIDER(RowsetA.CreateRowset(SELECT_ALLFROMTBL)==S_OK);

	//Since SQLServer has a problem trying to reposition on a row if the index 
	//column has changed, don't change the index column, bind all columns except that one
	TESTC_(GetAccessorAndBindings(RowsetA.pIAccessor(), DBACCESSOR_ROWDATA, 
		&hAccessor,	NULL, NULL, NULL, DBPART_ALL, UPDATEABLE_NONINDEX_COLS_BOUND),S_OK); 

	//Obtain row handle(s)
	TESTC_(RowsetA.GetRow(FIRST_ROW,&hRow),S_OK)	

	//Create Threads
	CREATE_FIRST_THREADS(Thread_SetData,&T1Arg);
	CREATE_SECOND_THREADS(Thread_GetData,&T2Arg);

	//TODO These threads really should be long senarios...

	START_THREADS();
	END_THREADS();	

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Multiple Querys on the same Connection Object
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCRealWorld::Variation_3()
{
	TBEGIN
	INIT_THREADS(MAX_THREADS);

	//Setup thread arguments
	THREADARG T1Arg = { this, (void*)S_OK };

	//CreateCommandObject before entering threads
	CreateCommandObject();

	CREATE_THREADS(Thread_CommandExecute,&T1Arg);
	
	START_THREADS();
	END_THREADS();	

//CLEANUP:
	TRETURN
}
// }}
// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCRealWorld::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CThreads::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCMultipleResult)
//*-----------------------------------------------------------------------
//| Test Case:		TCMultipleResult - Multi-Threaded testing for IMultipleResults
//|	Created:			11/21/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCMultipleResult::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CThreads::Init())
	// }}
	{
		if(CreateRowset(DBPROP_CANHOLDROWS, SELECT_VALIDATIONORDER)==S_OK)
			return TRUE;
	}

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//--------------------------------------------------------------------
// @mfunc GetResults from seperate threads
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCMultipleResult::Variation_1()
{
	TBEGIN
	INIT_THREADS(THREE_THREADS);
	THREADARG T1Arg, T2Arg, T3Arg;

	IMultipleResults* pIMultipleResults = NULL;
	IRowset* rgpIRowset[THREE_THREADS] = { NULL, NULL, NULL };
	
	CRowset RowsetA;
	//CreateRowset
	TESTC_PROVIDER(RowsetA.CreateRowset(SELECT_VALIDATIONORDER)==S_OK);

	//Obtain the MultipleResults interface
	//This is an optional interface.
	TESTC_PROVIDER(pTable()->ExecuteCommand(SELECT_ALLFROMTBL, IID_IMultipleResults, NULL, NULL, NULL, NULL, EXECUTE_IFNOERROR, 0, NULL, NULL, (IUnknown**)&pIMultipleResults, NULL)==S_OK);
	
	
	//Setup thread arguments
	T1Arg.pFunc = this;
	T1Arg.pArg1 = pIMultipleResults;
	T1Arg.pArg2 = &rgpIRowset[THREAD_ONE];

	T2Arg.pFunc = this;
	T2Arg.pArg1 = pIMultipleResults;
	T2Arg.pArg2 = &rgpIRowset[THREAD_TWO];
	
	T3Arg.pFunc = this;
	T3Arg.pArg1 = pIMultipleResults;
	T3Arg.pArg2 = &rgpIRowset[THREAD_THREE];

	CREATE_THREAD(THREAD_ONE,   Thread_GetResult, &T1Arg);
	CREATE_THREAD(THREAD_TWO,   Thread_GetResult, &T2Arg);
	CREATE_THREAD(THREAD_THREE, Thread_GetResult, &T3Arg);
	
	//Execute threads	
	START_THREADS();
	END_THREADS();	
	
	//Verify results
	//1 thread should have executed the statment, the other 2 should have no results
	if(rgpIRowset[THREAD_ONE])
	{
		TESTC(rgpIRowset[THREAD_TWO]==NULL && rgpIRowset[THREAD_THREE]==NULL);
	}
	else if(rgpIRowset[THREAD_TWO])
	{
		TESTC(rgpIRowset[THREAD_ONE]==NULL && rgpIRowset[THREAD_THREE]==NULL);
	}
	else if(rgpIRowset[THREAD_THREE])
	{
		TESTC(rgpIRowset[THREAD_ONE]==NULL && rgpIRowset[THREAD_TWO]==NULL);
	}
	else
	{
		TESTC(0); //Error, there was no rowset
	}
		
CLEANUP:
	SAFE_RELEASE(pIMultipleResults);
	SAFE_RELEASE(rgpIRowset[THREAD_ONE]);
	SAFE_RELEASE(rgpIRowset[THREAD_TWO]);
	SAFE_RELEASE(rgpIRowset[THREAD_THREE]);
	TRETURN
}


// }}
// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCMultipleResult::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CThreads::Terminate());
}	// }}
// }}
// }}
