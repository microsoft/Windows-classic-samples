//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright (C) 1995-2000 Microsoft Corporation
//
// @doc
//
// @module ExtraLib.h    | This module contains header information for 
//	    					ExtraLib.cpp general testing library
//
// @rev 01 | 02-22-96 | Microsoft | Created
// @rev 02 | 12-01-96 | Microsoft | Updated
//

#ifndef _EXTRALIB_H_
#define _EXTRALIB_H_

#include "oledb.h" 			// MS (OLE DB) Header Files
#include "oledberr.h"		// MS (OLE DB) Errors 
#include <msdasql.h>		// MS (OLE DB) Extra GUIDs (KAGPROP_QUERYBASEDUPDATES)
#include <msdaguid.h>		// MS (OLE DB) IRowPosition

#include "privlib.h"		// Private Library
#include <process.h>        // _beginthreadex
#include <olectl.h>			//CONNECT_E_NOCONNECT


////////////////////////////////////////////////////////////////////////////
// Globals
//
/////////////////////////////////////////////////////////////////////////////
extern CTable*  g_pTable;						// Global table
extern CTable*  g_pEmptyTable;					// Empty table
extern CTable*  g_p1RowTable;					// 1 Row Table
extern DBCOUNTITEM	g_ulNextRow;				// NextRow

extern IDBInitialize*    g_pIDBInitialize;     //Global DSO ptr
extern IDBCreateSession* g_pIDBCreateSession;  //Global DSO ptr
extern IOpenRowset*      g_pIOpenRowset;	   //Global Session ptr

extern CRITICAL_SECTION  GlobalMutex;          //Global Mutex


////////////////////////////////////////////////////////////////////////////
// Forwards
//
/////////////////////////////////////////////////////////////////////////////
class CStorage;
class CImpIRowsetNotify;
class CImpIDBAsynchNotify;
class CImpIRowPositionChange;


///////////////////////////////////////////////////////////////////////////
// Provider Specific
//
///////////////////////////////////////////////////////////////////////////
#define PROVIDER_CLSID GetModInfo()->GetThisTestModule()->m_ProviderClsid
#define MSDASQL  (CLSID_MSDASQL==PROVIDER_CLSID)


///////////////////////////////////////////////////////////////////////////
// Strings
//
///////////////////////////////////////////////////////////////////////////
#define EQUAL_STR 0
#define ON  TRUE
#define OFF FALSE
 							  

///////////////////////////////////////////////////////////////////////////
// Macros
// 
///////////////////////////////////////////////////////////////////////////
#define SAFE_RELEASE_DSO(pv)			{ if(pv) ReleaseDataSource((IUnknown**)&(pv)); pv = NULL;	}

#define TCHECK(exp,hr)					CHECK(exp, hr)
#define TCOMPARE(x,y)					COMPARE(x, y)
#define TCOMPARE_(x)					COMPARE(x, TRUE)

#define TEST(exp)						{ if(!COMPARE(exp, TRUE)) TRETURN;			}
#define TEST_(exp,hr)					{ if(!CHECK(exp,hr)) TRETURN;				}
#define QTEST(exp)						{ if(!QCOMPARE(exp, TRUE)) TRETURN;			}
#define QTEST_(exp,hr)					{ if(!QCHECK((exp),hr)) TRETURN;			} 

//COMP Macros
#define COMP(exp1,exp2)					{ if(!COMPARE(exp1,exp2)) TRETURN;			} 
#define COMPC(exp1,exp2)				{ if(!COMPARE(exp1,exp2)) goto CLEANUP;	} 

//QCOMP Macros
#define QCOMP(exp1,exp2)				{ if(!QCOMPARE(exp1,exp2)) TRETURN;			} 
#define QCOMPC(exp1,exp2)				{ if(!QCOMPARE(exp1,exp2)) goto CLEANUP;	} 

#define TEST_PROVIDER(exp)				{ TESTB = TEST_PASS; if(!(exp)) { TOUTPUT_LINE(L"NotSupported by Provider"); TESTB = TEST_SKIPPED; TRETURN; } }

#define MODULENAME						(LPWSTR)gwszModuleName

#define SIZEOF_ONEROW					(sizeof(void*)*m_cRowSize)
#define SIZEOF_TABLE (5)
#define MAX_NAME_LEN					 256
#define MAX_QUERY_LEN					4096
#define NEGATIVE(ul)					(-(LONG)(ul))



//////////////////////////////////////////////////////////////////////
//  Enums
//
//////////////////////////////////////////////////////////////////////

//Const int names
enum {ONE_ROW=1,TWO_ROWS,THREE_ROWS,FOUR_ROWS,FIVE_ROWS,SIX_ROWS,SEVEN_ROWS,EIGHT_ROWS,NINE_ROWS,TEN_ROWS,ELEVEN_ROWS,TWELVE_ROWS,THIRTEEN_ROWS,FOURTEEN_ROWS};
//Array index names
enum {ROW_ONE=0,ROW_TWO,ROW_THREE,ROW_FOUR,ROW_FIVE,ROW_SIX,ROW_SEVEN,ROW_EIGHT,ROW_NINE,ROW_TEN,ROW_ELEVEN,ROW_TWELVE,ROW_THIRTEEN,ROW_FOURTEEN};
//Table index name
enum {NO_ROWS=0,FIRST_ROW,SECOND_ROW,THIRD_ROW,FOURTH_ROW,FIFTH_ROW,SIXTH_ROW,SEVENTH_ROW,EIGHTH_ROW,NINTH_ROW,TENTH_ROW,TWELTH_ROW,THIRTEENTH_ROW,FOURTEENTH_ROW};

enum {COL_ONE=1};

// Prop status comparison 
enum VALUE_ENUM
{
	NOT_USED=0,			// Ignored
	VALID_VALUE,		// Value is supported
	INVALID_VALUE,		// Value is unsupported
	INDETERMINATE_VALUE	// We don't know whether the value passed is supported
};

// PropSets comparison 
enum PROPSETCOMPAREOPTIONS_ENUM
{
	EXACT_MATCH=0,				// properties in the properties' sets should exactly match
	IGNORE_OPTION_FOR_VTEMPTY	// if both properties values are VT_EMPTY then ignore dwOptions mismatch
};

#define DBPENDINGSTATUS_ALL      ( DBPENDINGSTATUS_NEW |DBPENDINGSTATUS_CHANGED |DBPENDINGSTATUS_DELETED )
#define DBPART_ALL               ( DBPART_VALUE | DBPART_LENGTH | DBPART_STATUS )

#define DBPROPVAL_NP_ALL         ( DBPROPVAL_NP_OKTODO | DBPROPVAL_NP_ABOUTTODO	| DBPROPVAL_NP_SYNCHAFTER | DBPROPVAL_NP_FAILEDTODO	| DBPROPVAL_NP_DIDEVENT	)
#define DBPROPVAL_NP_CANCELABLE  ( DBPROPVAL_NP_OKTODO | DBPROPVAL_NP_ABOUTTODO	| DBPROPVAL_NP_SYNCHAFTER )

#define DBPROPVAL_UP_ALL         ( DBPROPVAL_UP_CHANGE | DBPROPVAL_UP_DELETE | DBPROPVAL_UP_INSERT )

#define DBPROPVAL_RT_ALL         ( DBPROPVAL_RT_FREETHREAD | DBPROPVAL_RT_APTMTTHREAD | DBPROPVAL_RT_SINGLETHREAD )
#define DBPROPVAL_RT_BOTH        ( DBPROPVAL_RT_FREETHREAD | DBPROPVAL_RT_APTMTTHREAD )

enum DBASYNCHREASONENUM
{
	DBREASON_ONLOWRESOURCE	=	DBREASON_ROW_ASYNCHINSERT	+ 1,
	DBREASON_ONPROGRESS		=	DBREASON_ONLOWRESOURCE		+ 1,
	DBREASON_ONSTOP			=	DBREASON_ONPROGRESS			+ 1,
};
typedef DWORD DBASYNCHREASON;


#define DBREASON_ALL             ( DBREASON_ONSTOP + 1			)
#define DBEVENTPHASE_ALL         ( DBEVENTPHASE_DIDEVENT + 1	)


////////////////////////////////////////////////////////////////////////
//  Multi-Threads
//
////////////////////////////////////////////////////////////////////////
enum {THREAD_ONE=0, THREAD_TWO, THREAD_THREE, THREAD_FOUR, THREAD_FIVE, THREAD_SIX}; 
enum {ONE_THREAD=1,TWO_THREADS,THREE_THREADS, FOUR_THREADS, FIVE_THREADS, SIX_THREADS, MAX_THREADS=10 };


enum {LISTENER_ONE=0, LISTENER_TWO, LISTENER_THREE, LISTENER_FOUR, LISTENER_FIVE}; 
enum {SOURCE_ONE=0, SOURCE_TWO, SOURCE_THREE, SOURCE_FOUR, SOURCE_FIVE}; 

//This simply the usage of threads down to 3 lines of code, CREATE/START/END
inline BOOL CreateThreads(LPTHREAD_START_ROUTINE pFunc,void* pArgs,ULONG cThreads,HANDLE* rghThread,DWORD* rgThreadID)
{
	ASSERT(pFunc && pArgs && rghThread && rgThreadID);

	for(ULONG i=0; i<cThreads; i++)
		rghThread[i] = (void*)_beginthreadex(NULL,0,(UINT(WINAPI*)(void*))pFunc,pArgs,CREATE_SUSPENDED,(UINT*)&rgThreadID[i]);

// NOTE: CreateThread (WinAPI) is bad, it doesn't work well with the CrtLib,
//       Supposed to use, _beginthreadex...  CreateThread is much nicer, and doesn't
//       require all the "casting", but _begin thread actually delegates out to eventually...
//		rghThread[i] = CreateThread(NULL,0,pFunc,pArgs,CREATE_SUSPENDED,&rgThreadID[i]);

	return TRUE;
}
inline BOOL CreateThread(LPTHREAD_START_ROUTINE pFunc,void* pArg,HANDLE* phThread,DWORD* pThreadID)
{
	return CreateThreads(pFunc,pArg,ONE_THREAD,phThread,pThreadID);
}
inline BOOL StartThreads(ULONG cThreads, LPHANDLE rghThread)
{
	ASSERT(cThreads && rghThread);

	for(ULONG i=0; i<cThreads; i++)
		ResumeThread(rghThread[i]);
	return TRUE;
}
inline BOOL StartThread(LPHANDLE phThread)
{
	return StartThreads(ONE_THREAD,phThread);
}

extern DWORD WaitForThreads(ULONG cThreads, LPHANDLE rghThread);

inline BOOL EndThreads(ULONG cThreads, LPHANDLE rghThread)
{
	WaitForThreads(cThreads, rghThread);
	for(ULONG i=0; i<cThreads; i++) 
	   CloseHandle(rghThread[i]);
	return TRUE;
}
inline BOOL EndThread(LPHANDLE phThread)
{
   return EndThreads(ONE_THREAD,phThread);
}

inline HRESULT GetThreadCode(HANDLE hThread)
{
	HRESULT hr = S_OK;

	//Get the thread return code
	if(GetExitCodeThread(hThread, (LPDWORD)&hr))
		return hr;

	DWORD dwLastError = GetLastError();
	return E_FAIL;
}

inline BOOL GetThreadResults(ULONG cThreads, HANDLE* rghThreads, HRESULT hrExpected)
{
	ULONG cMatched = 0;
	for(ULONG i=0; i<cThreads; i++)
	{
		HRESULT hrActual = GetThreadCode(rghThreads[i]);
		if(hrActual == hrExpected)
			cMatched++;
	}

	return cMatched;
}

//This simply the usage of threads down to 3 lines of code, CREATE/START/END
#define INIT_THREADS(cTh)  const ULONG cThreads = cTh; HANDLE rghThread[cThreads]; DWORD rgThreadID[cThreads]

#define CREATE_THREADS(pFunc,pArgs)         CreateThreads(pFunc,pArgs,cThreads,rghThread,rgThreadID)
#define CREATE_THREAD(iThread,pFunc,pArg)   CreateThreads(pFunc,pArg,ONE_THREAD,&rghThread[iThread],&rgThreadID[iThread])

#define CREATE_FIRST_THREADS(pFunc,pArgs)   CreateThreads(pFunc,pArgs, cThreads/2, rghThread, rgThreadID)
#define CREATE_SECOND_THREADS(pFunc,pArgs)  CreateThreads(pFunc,pArgs, cThreads-cThreads/2, &rghThread[cThreads/2], &rgThreadID[cThreads/2])
										   
#define START_THREADS()                     StartThreads(cThreads,rghThread)     
#define START_THREAD(iThread)               StartThread(&rghThread[iThread])

#define WAITFOR_THREADS()                   WaitForThreads(cThreads,rghThread)
#define WAITFOR_THREAD(iThread)             WaitForThreads(1, &rghThread[iThread])

#define END_THREADS()                       EndThreads(cThreads,rghThread)
#define END_THREAD(iThread)                 EndThread(&rghThread[iThread])

#define GET_THREADRESULTS(hrExpected)			GetThreadResults(cThreads, rghThread, hrExpected)
#define GET_THREADRESULT(iThread, hrExpected)	GetThreadResults(1, &rghThread[iThread], hrExpected)


#define THREAD_FUNC (((THREADARG*)pv)->pFunc)
#define THREAD_ARG1 ((ULONG_PTR)(((THREADARG*)pv)->pArg1))
#define THREAD_ARG2 ((ULONG_PTR)(((THREADARG*)pv)->pArg2))
#define THREAD_ARG4 ((ULONG_PTR)(((THREADARG*)pv)->pArg4))
#define THREAD_ARG3 ((ULONG_PTR)(((THREADARG*)pv)->pArg3))
#define THREAD_ARG5 ((ULONG_PTR)(((THREADARG*)pv)->pArg5))

struct THREADARG
{
	LPVOID	pFunc;
	LPVOID	pArg1;
	LPVOID	pArg2;
	LPVOID	pArg3;
	LPVOID	pArg4;
	LPVOID	pArg5;
};

inline THREADARG InitThreadArg(void* pFunc, void* pArg1 = NULL, void* pArg2 = NULL, void* pArg3 = NULL, void* pArg4 = NULL, void* pArg5 = NULL)
{
	THREADARG ThreadArg = { pFunc, pArg1, pArg2, pArg3, pArg4, pArg5};
	return ThreadArg;
}


WINOLEAPI  CoInitializeEx(LPVOID pvReserved, DWORD dwCoInit);
#define THREAD_BEGIN						CoInitializeEx(NULL, 0/*COINIT_MULTITHREADED*/ ); TBEGIN 
#define	THREAD_BEGIN_(fFreeThreaded)		CoInitializeEx(NULL, fFreeThreaded ? 0/*COINIT_MULTITHREADED*/ : 2/*COINIT_APARTMENTTHREADED*/); TBEGIN 
#define THREAD_END(hr)						CoUninitialize();		  _endthreadex(hr); return 0;
#define THREAD_RETURN						CoUninitialize();		  TRETURN
#define ThreadSwitch()						Sleep(0)


////////////////////////////////////////////////////////////////////////
// Extern
//
////////////////////////////////////////////////////////////////////////
extern GUID NULLGUID;
extern DBID NULLDBID;
extern DBID INVALID_DBID; 
extern DBID VALID_INDEXID;
#define INVALID_TABLEID INVALID_DBID
#define INVALID_INDEXID INVALID_DBID


//////////////////////////////////////////////////////////////////////////////
//  Memory Alloc/Free Routines
//
//  A "safe" wrapper arround CoTaskMemAlloc to protect boundaries
//////////////////////////////////////////////////////////////////////////////  

#define HEAD_SIGN 0xedededed
#define TAIL_SIGN 0xdededede
#define ALLOC_SIGN 0xaa
#define FREE_SIGN 0xbb

void* BoundarySafeAlloc(ULONG cb); 
void BoundarySafeFree(void* pv);
void BoundarySafeFree(ULONG cBuffers, void** ppv);

#define PROVIDER_FREE2(cEle,ppv)	{ if(ppv) for(ULONG i=0; i<cEle; i++) PROVIDER_FREE(ppv[i]); /*ppv = NULL;*/ }


////////////////////////////////////////////////////////////////////
// General Misc Helper Functions
//
////////////////////////////////////////////////////////////////////

BOOL CreateTable(CTable** ppTable, DBCOUNTITEM cRows);
BOOL DropTable(CTable** ppTable);
HRESULT TableInsert(DBCOUNTITEM cRows, CTable* pCTable = g_pTable);

BOOL VerifyOutputInterface(HRESULT hr, REFIID riid, IUnknown** ppIUnknown);
BOOL ValidInterface(IID riid, IUnknown* pIUnknown);
BOOL SupportedInterface(IID riid, EINTERFACE eInterface = UNKNOWN_INTERFACE);
BOOL VerifyThreadingModel(CLSID clsidProv, WCHAR* pwszThreadingModel, CLSCTX clsctx = CLSCTX_INPROC_SERVER);

BOOL CommonModuleInit(CThisTestModule * pThisTestModule, REFIID riid = IID_IUnknown, DBCOUNTITEM cRows = SIZEOF_TABLE, EINTERFACE eInterface = UNKNOWN_INTERFACE);
BOOL CommonModuleTerminate(CThisTestModule * pThisTestModule); 

#define PERSIST_FILE L"Persist.sav"
HRESULT SaveDSO(WCHAR* pwszFileName);

HRESULT CreateNewDSO(IUnknown* pIUnkOuter, REFIID riid, IUnknown** ppIUnknown, DWORD dwOptions = CREATEDSO_SETPROPERTIES | CREATEDSO_INITIALIZE);

BOOL	ShouldInitialize(IUnknown* pDataSource);
HRESULT InitializeDataSource(IUnknown* pDataSource, DWORD dwOptions = CREATEDSO_SETPROPERTIES | CREATEDSO_INITIALIZE);
HRESULT ReleaseDataSource(IUnknown** ppDataSource);
HRESULT CreateNewSession(IUnknown* pIDataSource, REFIID riid, IUnknown** ppIUnknown, IUnknown* pIUnkOuter = NULL);
HRESULT CreateNewCommand(IUnknown* pISession, REFIID riid, IUnknown** ppIUnknown, IUnknown* pIUnkOuter = NULL);

BOOL CanConvert(IUnknown* pIUnknown, DBTYPE wFromType, DBTYPE wToType);
HRESULT	ConvertString(WCHAR* pwszBuffer, BOOL fUpperCase = TRUE);
WCHAR*	CreateString(WCHAR* pwszFmt, ...);
HRESULT	AppendString(WCHAR** ppwszOldString, WCHAR* pwszAppend);
BOOL GetTableSchemaInfo(CTable* pTable,	WCHAR* pwszTargetTable,	BOOL* pfFound, WCHAR** ppwszCatalogName = NULL, WCHAR** ppwszSchemaName = NULL);
BOOL FreeAccessorBufferAndBindings(DBCOUNTITEM *pcBindings, DBBINDING **prgBindings, void **ppData, bool bFreeBuffer = false);

////////////////////////////////////////////////////////////////////////////
// Window functions
//
////////////////////////////////////////////////////////////////////////////
BOOL		IsUnicodeOS();


///////////////////////////////////////////////////////////////
// Registry
//
///////////////////////////////////////////////////////////////
HRESULT		CreateRegKey(HKEY hRootKey, WCHAR* pwszKeyName, HKEY* phKey, REGSAM samDesired = KEY_READ | KEY_WRITE);
HRESULT		OpenRegKey(HKEY hRootKey, WCHAR* pwszKeyName, DWORD ulOptions, REGSAM samDesired, HKEY* phKey);

HRESULT		GetRegEnumKey(HKEY hRootKey, WCHAR* pwszKeyName, DWORD dwIndex, WCHAR* pwszSubKeyName, ULONG cBytes);
HRESULT		GetRegEnumValue(HKEY hRootKey, WCHAR* pwszKeyName, DWORD dwIndex, WCHAR** ppwszValueName);
HRESULT		GetRegEnumValue(HKEY hRootKey, WCHAR* pwszKeyName, DWORD dwIndex, WCHAR* pwszValueName, ULONG* pcBytes);

HRESULT		GetRegEntry(HKEY hRootKey, WCHAR* pwszKeyName, WCHAR* pwszValueName, WCHAR* pwszValue, ULONG cBytes);
HRESULT		GetRegEntry(HKEY hRootKey, WCHAR* pwszKeyName, WCHAR* pwszValueName, ULONG* pulValue);
HRESULT		GetRegEntry(HKEY hRootKey, WCHAR* pwszKeyName, WCHAR* pwszValueName, void* pStruct, ULONG cbSize, ULONG dwType = REG_BINARY);

HRESULT		SetRegEntry(HKEY hRootKey, WCHAR* pwszKeyName, WCHAR* pwszValueName, WCHAR* pwszValue);
HRESULT		SetRegEntry(HKEY hRootKey, WCHAR* pwszKeyName, WCHAR* pwszValueName, ULONG ulValue);
HRESULT		SetRegEntry(HKEY hRootKey, WCHAR* pwszKeyName, WCHAR* pwszValueName, void* pStruct, ULONG cbSize, ULONG dwType = REG_BINARY);

HRESULT		DelRegEntry(HKEY hRootKey, WCHAR* pwszKeyName);
HRESULT		DelAllRegEntry(HKEY hRootKey, WCHAR* pwszKeyName);

HRESULT		CloseRegKey(HKEY hKey);



///////////////////////////////////////////////////////////////////
// Storage Object helpers
//
///////////////////////////////////////////////////////////////////

DBBINDING* FindBinding(DBCOUNTITEM cBindings, DBBINDING* rgBindings, DBTYPE wType, DBCOUNTITEM* pulIndex = NULL);
BOOL GetStorageObject(    DBCOUNTITEM cBindings, DBBINDING* rgBindings, void* pData, REFIID riid, IUnknown** ppIUnknown);
BOOL SetStorageObject(    DBCOUNTITEM cBindings, DBBINDING* rgBindings, void* pData, DBCOUNTITEM iTableRow = 0, REFIID riid = IID_IUnknown, CStorage* pCStorage = NULL, DBSTATUS dbStatus = DBSTATUS_S_OK, DBLENGTH cBytes = MAX_PTR);

HRESULT GetStorageData(DBCOUNTITEM cBindings, DBBINDING* rgBindings, void* pData, void* pBuffer, ULONG* pcBytes = NULL, REFIID riid = IID_IUnknown, IUnknown** ppIUnknown = NULL);
HRESULT GetStorageData(REFIID riid, IUnknown* pIUnknown, void* pBuffer, ULONG* pcBytes = NULL);
BOOL CreateData(void** ppBuffer, DBCOUNTITEM iTableRow, DBBINDING* pBindings, DBLENGTH* pcSize);

BOOL VerifyAccessorStatus(DBCOUNTITEM cBindings, DBBINDING* rgBindings, DBBINDSTATUS* rgBindStatus, DBTYPE wType, DBBINDSTATUS dwBindStatus);
BOOL VerifyBindingStatus(DBCOUNTITEM cBindings, DBBINDING* rgBindings, void* pData, DBTYPE wType, DBSTATUS dwStatus);
BOOL VerifyBindings(HRESULT hrReturned, IAccessor* pIAccessor, HACCESSOR hAccessor, void* pData);


////////////////////////////////////////////////////////////////////
// Property Routines
//
////////////////////////////////////////////////////////////////////
VARTYPE GetPropInfoType(DBPROPID dwPropertyID, GUID guidPropertySet);
DBPROPFLAGS GetPropInfoFlags(DBPROPID dwPropertyID, GUID guidPropertySet);
DBPROPINFO* GetPropInfo(DBPROPID dwPropertyID, GUID guidPropertySet);
WCHAR* GetPropDesc(DBPROPID dwPropertyID, GUID guidPropertySet, IUnknown* pDataSource = NULL);

BOOL FindPropSet(GUID guidPropertySet, ULONG cPropSets, DBPROPSET* rgPropSets, DBPROPSET** ppPropSet = NULL);
BOOL FindProperty(DBPROPID dwPropertyID, GUID guidPropertySet, ULONG cPropSets, DBPROPSET* rgPropSets, DBPROP** ppProp = NULL);
BOOL EqualProperty(DBPROP* pProp, DBPROP* pProp2, enum PROPSETCOMPAREOPTIONS_ENUM eCompareOptions = EXACT_MATCH);
BOOL EqualPropSets(ULONG cPropSets, DBPROPSET* rgPropSets, ULONG cPropSets2, DBPROPSET* rgPropSets2, enum PROPSETCOMPAREOPTIONS_ENUM eCompareOptions = EXACT_MATCH);

BOOL SupportedProperty(DBPROPID dwPropertyID, GUID guidPropertySet);
BOOL SettableProperty(DBPROPID dwPropertyID, GUID guidPropertySet);

BOOL SetSupportedProperty(DBPROPID dwPropertyID, GUID guidPropertySet, ULONG* pcPropSets, DBPROPSET** prgPropSets, void* pValue = (void*)VARIANT_TRUE, DBTYPE wType = DBTYPE_BOOL, DBPROPOPTIONS dwOptions = DBPROPOPTIONS_REQUIRED, DBID colid = DB_NULLID);
BOOL SetSettableProperty(DBPROPID dwPropertyID, GUID guidPropertySet, ULONG* pcPropSets, DBPROPSET** prgPropSets, void* pValue = (void*)VARIANT_TRUE, DBTYPE wType = DBTYPE_BOOL, DBPROPOPTIONS dwOptions = DBPROPOPTIONS_REQUIRED, DBID colid = DB_NULLID);
BOOL SetProperty(DBPROPID dwPropertyID, GUID guidPropertySet, ULONG* pcPropIDSets, DBPROPIDSET** prgPropIDSets);
BOOL SetProperty(DBPROPID dwPropertyID, GUID guidPropertySet, ULONG* pcPropSets, DBPROPSET** prgPropSets, void* pValue = (void*)VARIANT_TRUE, DBTYPE wType = DBTYPE_BOOL, DBPROPOPTIONS dwOptions = DBPROPOPTIONS_REQUIRED, DBID colid = DB_NULLID);

BOOL GetPropStatus(ULONG cPropSets, DBPROPSET* rgPropSets, DBPROPID dwPropertyID, GUID guidPropertySet, DBPROPSTATUS* pdwInputStatus);
BOOL VerifyPropSetStatus(ULONG cPropSets, DBPROPSET* rgPropSets, DBPROPSTATUS dwPropStatus = DBPROPSTATUS_OK);
BOOL VerifyPropStatus(ULONG cPropSets, DBPROPSET* rgPropSets, ULONG dwProperty, GUID guidPropertySet, DBPROPSTATUS dwPropStatus = DBPROPSTATUS_OK, enum VALUE_ENUM eValue = NOT_USED);

BOOL FreePropInfos(ULONG cPropertyInfos, DBPROPINFO* rgPropInfos);
BOOL FindValue(DBCOUNTITEM ulValue, DBCOUNTITEM cElements, DBCOUNTITEM* rgElements);



////////////////////////////////////////////////////////////////////
// Notifications
//
////////////////////////////////////////////////////////////////////
WCHAR* GetPhaseDesc(DBEVENTPHASE ePhase);
WCHAR* GetReasonDesc(DBREASON eReason);
WCHAR* GetAsynchPhase(DBASYNCHPHASE ePhase);


////////////////////////////////////////////////////////////////////
// inline Misc Helpfer Functions
//
////////////////////////////////////////////////////////////////////
HRESULT QI(IUnknown* pExistingIUnknown,REFIID riid,void** ppRequestedIUnknown = NULL);
BOOL	VerifyArray(DBCOUNTITEM cEle, ULONG* rgEle, ULONG scode);

inline BOOL Prepare(ICommand* pICommand)
{
	ASSERT(pICommand);
	
	TBEGIN
    ICommandPrepare* pICommandPrepare = NULL;

    //Obtain the prepare interface
	TESTC_(QI(pICommand, IID_ICommandPrepare, (void**)&pICommandPrepare),S_OK)

	//prepare the command
	TESTC_(pICommandPrepare->Prepare(0),S_OK)

CLEANUP:
	SAFE_RELEASE(pICommandPrepare);
	TRETURN
}

//////////////////////////////////////////////////////////////////////////////
//  BitWise manipulations
//
//////////////////////////////////////////////////////////////////////////////
inline ULONG EnumToBitmask(ULONG eValue)  
{
	ULONG ulMask = 0x1;

	//Has to be less then the numer of "bits" in the return value
	ASSERT(eValue < (sizeof(ULONG)*8)); 
	return ulMask << (eValue);
}
#define BITSET(val, mask)   (((val) & (mask)) == (mask))
#define BITCLEAR(val, mask) (!((val) & (mask)))

#define SETBIT(val, mask)   ((val) |= (mask))
#define CLEARBIT(val, mask) ((val) &= ~(mask))



/////////////////////////////////////////////////////////////////////////////
// Enumerator Operations
//
/////////////////////////////////////////////////////////////////////////////
struct ENUMINFO
{
	WCHAR			wszName[MAX_NAME_LEN];
	WCHAR			wszParseName[MAX_NAME_LEN];
	WCHAR			wszDescription[MAX_NAME_LEN];
	DBTYPE			wType;
	VARIANT_BOOL	fIsParent;
};

HRESULT GetEnumInfo(IParseDisplayName* pIParseDisplayName, ULONG* pcEnumInfo, ENUMINFO** prgEnumInfo);
HRESULT CreateFromDisplayName(IParseDisplayName* pIParseDisplayName, WCHAR* pwszDisplayName, REFIID riid, IUnknown** ppIUnknown);


//////////////////////////////////////////////////////////////////////////
// Class CDataSource
//
// Basically a nice wrapper arround a DataSource object.  
//////////////////////////////////////////////////////////////////////////
class CDataSource : public CDataSourceObject
{
public:
	//Constructors
	CDataSource(WCHAR* pwszTestCaseName = INVALID(WCHAR*));
    virtual ~CDataSource();

    //members
	virtual BOOL Init();
	virtual BOOL Terminate();

    //cast operator
	virtual IDBInitialize*  const pIDBInit();
	virtual IUnknown*		const pDataSource();
	virtual IDBInitialize*  const operator ()();
		
    virtual HRESULT CreateInstance();
    virtual HRESULT Initialize(IUnknown* pDataSource, EREINITIALIZE eReinitialize = REINITIALIZE_YES);
    virtual HRESULT Initialize(EREINITIALIZE eReinitialize = REINITIALIZE_YES);
    virtual HRESULT Uninitialize();

	//GetProperties
	virtual HRESULT GetPropertyInfo(GUID guidPropertySet, ULONG* pcPropInfoSets = NULL, DBPROPINFOSET** prgPropInfoSets = NULL, WCHAR** ppwszStringBuffer = NULL);

	//SetProperties
    virtual BOOL SetProperty(DBPROPID dwPropertyID, GUID guidPropertySet = DBPROPSET_ROWSET, void* pValue = (void*)VARIANT_TRUE, DBTYPE wType = DBTYPE_BOOL, DBPROPOPTIONS dwOptions = DBPROPOPTIONS_REQUIRED, DBID colid = DB_NULLID);
    virtual BOOL SetSupportedProperty(DBPROPID dwPropertyID, GUID guidPropertySet = DBPROPSET_ROWSET, void* pValue = (void*)VARIANT_TRUE, DBTYPE wType = DBTYPE_BOOL, DBPROPOPTIONS dwOptions = DBPROPOPTIONS_REQUIRED, DBID colid = DB_NULLID);
    virtual BOOL SetSettableProperty(DBPROPID dwPropertyID, GUID guidPropertySet = DBPROPSET_ROWSET, void* pValue = (void*)VARIANT_TRUE, DBTYPE wType = DBTYPE_BOOL, DBPROPOPTIONS dwOptions = DBPROPOPTIONS_REQUIRED, DBID colid = DB_NULLID);
	virtual BOOL FreeProperties();

	//Verification
	virtual BOOL	VerifyDataSource(IUnknown* pDataSource, REFCLSID rclsid);
	virtual BOOL	VerifyDataSource(IUnknown* pDataSource, DBPROPID dwPropertyID, GUID guidPropertySet, DBTYPE wType, void* pValue);
	

    //data 
	ULONG	    m_cPropSets;
	DBPROPSET*  m_rgPropSets;
};


//////////////////////////////////////////////////////////////////
// Class COpenRowset
//
// Used to create multiple instantes of CTestCases even though our
// privlib is really not designed for it, quite a useful hack...
//////////////////////////////////////////////////////////////////

class COpenRowset : public CSessionObject
{
public:
	//Constructors
	COpenRowset(WCHAR* pwszTestCaseName = INVALID(WCHAR*));
	virtual ~COpenRowset();

    //methods
    virtual BOOL Init();
    virtual BOOL Terminate();

    //helper
    virtual HRESULT CreateOpenRowset(IID riid = IID_IRowset, IUnknown** ppIRowset = NULL, IUnknown* pIUnkOuter = NULL, EINTERFACE eInterface = ROWSET_INTERFACE);
    virtual HRESULT CreateOpenRowset(CTable* pCTable,IID riid = IID_IRowset, IUnknown** ppIRowset = NULL, IUnknown* pIUnkOuter = NULL, EINTERFACE eInterface = ROWSET_INTERFACE);

    virtual BOOL SetProperty(DBPROPID dwPropertyID, GUID guidPropertySet = DBPROPSET_ROWSET, void* pValue = (void*)VARIANT_TRUE, DBTYPE wType = DBTYPE_BOOL, DBPROPOPTIONS dwOptions = DBPROPOPTIONS_REQUIRED, DBID colid = DB_NULLID);
    virtual BOOL SetSupportedProperty(DBPROPID dwPropertyID, GUID guidPropertySet = DBPROPSET_ROWSET, void* pValue = (void*)VARIANT_TRUE, DBTYPE wType = DBTYPE_BOOL, DBPROPOPTIONS dwOptions = DBPROPOPTIONS_REQUIRED, DBID colid = DB_NULLID);
    virtual BOOL SetSettableProperty(DBPROPID dwPropertyID, GUID guidPropertySet = DBPROPSET_ROWSET, void* pValue = (void*)VARIANT_TRUE, DBTYPE wType = DBTYPE_BOOL, DBPROPOPTIONS dwOptions = DBPROPOPTIONS_REQUIRED, DBID colid = DB_NULLID);
	virtual BOOL FreeProperties();
	
    //interface
	virtual IUnknown*	const pISession();
    virtual IOpenRowset* const operator()();
    virtual IOpenRowset* const pIOpenRowset();

//protected:
    //data 
	ULONG	    m_cPropSets;
	DBPROPSET*  m_rgPropSets;
    
protected:
    //data pointers
};


//////////////////////////////////////////////////////////////////////////
// Class CCommand
//
// Basically a nice wrapper arround a command object.  
//////////////////////////////////////////////////////////////////////////
class CCommand : public CCommandObject
{
public:
	//Constructors
	CCommand(WCHAR* pwszTestCaseName = INVALID(WCHAR*));
    virtual ~CCommand();

    //members
	virtual BOOL		Init();
	virtual BOOL		Terminate();

	//Helpers
	virtual HRESULT		CreateCommand(IUnknown* pIUnkOuter = NULL);
	virtual HRESULT		SetCommandText(EQUERY eQuery, REFGUID rguidDialect = DBGUID_DEFAULT);
	virtual HRESULT		SetProperties();
	virtual HRESULT		Execute(IUnknown* pUnkOuter, REFIID riid, IUnknown** ppRowset, DBROWCOUNT* pcRowsAffected = NULL, DBPARAMS* pParams = NULL);

	virtual BOOL		SetProperty(DBPROPID dwPropertyID, GUID guidPropertySet = DBPROPSET_ROWSET, void* pValue = (void*)VARIANT_TRUE, DBTYPE wType = DBTYPE_BOOL, DBPROPOPTIONS dwOptions = DBPROPOPTIONS_REQUIRED, DBID colid = DB_NULLID);
	virtual BOOL		FreeProperties();

//protected:
	ICommandText*		m_pICommandText;
	ICommandProperties*	m_pICommandProperties;

	ULONG				m_cPropSets;
	DBPROPSET*			m_rgPropSets;
};


//////////////////////////////////////////////////////////////////////////
// Class CRowset
//
// Basically a nice wrapper arround a rowset object.  This test will use
// multiple rowsets, and its easier to have all the general functionality
// together
//////////////////////////////////////////////////////////////////////////

class CRowset : public CRowsetObject
{
public:
	//Constructors
	CRowset(WCHAR* pwszTestCaseName = INVALID(WCHAR*));
    virtual ~CRowset();

    //members
	virtual BOOL Init();
	virtual BOOL Terminate();

	virtual HRESULT CreateCommand(EQUERY eQuery = USE_SUPPORTED_SELECT_ALLFROMTBL);
	virtual HRESULT CreateAccessor(HACCESSOR* phAccessor, DBACCESSORFLAGS dwAccessorFlags, DBPART dwPart = DBPART_ALL, DBCOUNTITEM* pcBindings=NULL, DBBINDING** prgDBBINDING=NULL, DBLENGTH* pcRowSize=NULL, BLOBTYPE dwBlobType = NO_BLOB_COLS, DBBINDSTATUS** prgBindStauts = NULL, DWORD dwColsBound = UPDATEABLE_COLS_BOUND, ECOLS_BY_REF eColsByRef = NO_COLS_BY_REF, DBTYPE dwModifier = DBTYPE_EMPTY);

    virtual HRESULT CreateRowset(EQUERY eQuery = SELECT_ORDERBYNUMERIC, REFIID riid = IID_IRowset, CTable* pCTable = NULL, DBACCESSORFLAGS dwAccessorFlags = DBACCESSOR_ROWDATA, DBPART dwPart = DBPART_ALL, DWORD dwColsBound = UPDATEABLE_COLS_BOUND, ECOLUMNORDER eBindingOrder = FORWARD, ECOLS_BY_REF	eColsByRef = NO_COLS_BY_REF, DBTYPE	wTypeModifier = DBTYPE_EMPTY, BLOBTYPE dwBlobType = NO_BLOB_COLS);
    virtual HRESULT CreateRowset(DBPROPID dwProperty, EQUERY eQuery = SELECT_ORDERBYNUMERIC, REFIID riid = IID_IRowset, CTable* pCTable = NULL, DBACCESSORFLAGS dwAccessorFlags = DBACCESSOR_ROWDATA, DBPART dwPart = DBPART_ALL, DWORD dwColsBound = UPDATEABLE_COLS_BOUND, ECOLUMNORDER eBindingOrder = FORWARD, ECOLS_BY_REF	eColsByRef = NO_COLS_BY_REF, DBTYPE	wTypeModifier = DBTYPE_EMPTY, BLOBTYPE dwBlobType = NO_BLOB_COLS);
	virtual HRESULT CreateRowset(IUnknown* pIRowset, EQUERY eQuery = SELECT_ORDERBYNUMERIC, REFIID riid = IID_IRowset, CTable* pCTable = NULL, DBACCESSORFLAGS dwAccessorFlags = DBACCESSOR_ROWDATA, DBPART dwPart = DBPART_ALL, DWORD dwColsBound = UPDATEABLE_COLS_BOUND, ECOLUMNORDER eBindingOrder = FORWARD, ECOLS_BY_REF	eColsByRef = NO_COLS_BY_REF, DBTYPE	wTypeModifier = DBTYPE_EMPTY, BLOBTYPE dwBlobType = NO_BLOB_COLS);

	virtual HRESULT CreateRowsetFromTbl(DBCOUNTITEM cTableSize);
	virtual HRESULT	DropRowset();

	//helpers
	virtual BOOL SetProperty(DBPROPID dwPropertyID, GUID guidPropertySet = DBPROPSET_ROWSET, void* pValue = (void*)VARIANT_TRUE, DBTYPE wType = DBTYPE_BOOL, DBPROPOPTIONS dwOptions = DBPROPOPTIONS_REQUIRED, DBID colid = DB_NULLID);
    virtual BOOL SetSupportedProperty(DBPROPID dwPropertyID, GUID guidPropertySet = DBPROPSET_ROWSET, void* pValue = (void*)VARIANT_TRUE, DBTYPE wType = DBTYPE_BOOL, DBPROPOPTIONS dwOptions = DBPROPOPTIONS_REQUIRED, DBID colid = DB_NULLID);
    virtual BOOL SetSettableProperty(DBPROPID dwPropertyID, GUID guidPropertySet = DBPROPSET_ROWSET, void* pValue = (void*)VARIANT_TRUE, DBTYPE wType = DBTYPE_BOOL, DBPROPOPTIONS dwOptions = DBPROPOPTIONS_REQUIRED, DBID colid = DB_NULLID);

	virtual BOOL GetProperty(DBPROPID dwPropertyID, GUID guidPropertySet = DBPROPSET_ROWSET, VARIANT_BOOL bValue = VARIANT_TRUE);
	virtual BOOL GetProperty(DBPROPID dwPropertyID, GUID guidPropertySet, VARIANT_BOOL* pbValue);		 
	virtual BOOL GetProperty(DBPROPID dwPropertyID, GUID guidPropertySet, ULONG_PTR* pulValue);		 
	virtual BOOL GetProperty(DBPROPID dwPropertyID, GUID guidPropertySet, WCHAR** ppwszValue);		 
	virtual BOOL FreeProperties();

	//Wrappers
	virtual HRESULT GetRow(DBCOUNTITEM iRow, HROW* phRow);
	virtual HRESULT GetRow(DBCOUNTITEM iRow, DBCOUNTITEM cRows, HROW* rghRow);
	virtual HRESULT GetRow(DBCOUNTITEM iRow, DBCOUNTITEM cRows, DBCOUNTITEM* pcRowsObtained, HROW* rghRow);

	virtual HRESULT GetNextRows(HROW* phRow);
	virtual HRESULT GetNextRows(DBROWCOUNT cRows, HROW* rghRow);
	virtual HRESULT GetNextRows(DBROWOFFSET lOffset, DBROWCOUNT cRows, HROW* rghRow);
	virtual HRESULT GetNextRows(DBROWOFFSET lOffset, DBROWCOUNT cRows, DBCOUNTITEM* pcRowsObtained, HROW* rghRow);
	virtual HRESULT GetNextRows(DBROWOFFSET lOffset, DBROWCOUNT cRows, DBCOUNTITEM* pcRowsObtained, HROW** rghRow);

	virtual HRESULT ReleaseRows(HROW hRow);
	virtual HRESULT ReleaseRows(DBCOUNTITEM cRows, HROW* rghRow, DBREFCOUNT* rgRefCounts, DBROWSTATUS* rgRowStatus = NULL);
	virtual HRESULT ReleaseRows(DBCOUNTITEM cRows, HROW* rghRow, DBREFCOUNT** prgRefCounts = NULL, DBROWSTATUS** prgRowStatus = NULL);

	virtual HRESULT ReleaseAccessor(HACCESSOR hAccessor, DBCOUNTITEM cBindings = 0, DBBINDING* rgBindings = NULL, void* pData = NULL);
	virtual HRESULT RestartPosition(HCHAPTER hChapter = DB_INVALID_HCHAPTER);

	virtual HRESULT GetRowData(HROW hRow, void** ppData);
	virtual HRESULT GetRowData(DBCOUNTITEM cRows, HROW* rghRow, void** ppData);
	virtual HRESULT GetData(HROW hRow, HACCESSOR hAccessor, void* pData);

	virtual DBCOUNTITEM   GetTotalRows();
    virtual DBORDINAL   GetTotalColumns(BOOL fIncludeBookmark = FALSE);
	
	virtual HRESULT FindColInfo(DBCOLUMNFLAGS dwFlags, DBCOLUMNINFO* pColInfo);
	virtual HRESULT GetColInfo(DBORDINAL* pcColumns, DBCOLUMNINFO** prgInfo, WCHAR** ppStringsBuffer=NULL);
	virtual HRESULT GetBookmark(HROW hRow, DBBKMARK *pcbBookmark, BYTE **ppBookmark);

	virtual HRESULT ResynchRows(DBCOUNTITEM cRows,HROW* rghRow,DBCOUNTITEM* pcResynchedRows = NULL, HROW** prgResynchedRows = NULL, DBROWSTATUS** prgRowStatus = NULL);

	//IGetRow
	virtual HRESULT GetRowObject(DBCOUNTITEM iRow, CRowObject* pCRowObject, ULONG_PTR ulpOleObjects = 0, HROW hRowSpecified = NULL);
	virtual HRESULT GetRowObject(IUnknown* pIUnkOuter, DBCOUNTITEM iRow, REFIID riid, IUnknown** ppIUnknown, ULONG_PTR ulpOleObjects = 0, HROW hRowSpecified = NULL);
	virtual HRESULT GetRowFromHROW(IUnknown* pIUnkOuter, HROW hRow, REFIID riid, IUnknown** ppIUnknown);
	virtual HRESULT GetURLFromHROW(HROW hRow, WCHAR** ppwszURL);

	//helpers
	virtual BOOL CompareRowData(void* pOrgData, void* pData, HACCESSOR hAccessor = DB_NULL_HACCESSOR, BOOL fSetData = FALSE);
	virtual BOOL CompareRowData(HROW hRow, void* pData);
	virtual BOOL CompareRowData(DBCOUNTITEM cRows, HROW* rghRow, void** rgpData);

	virtual HRESULT GetStorageData(HROW hRow, HACCESSOR hAccessor, void* pBuffer = NULL, ULONG* pcBytes = NULL, REFIID riid = IID_IUnknown, IUnknown** ppIUnknown = NULL);

    virtual BOOL VerifyRowHandles(DBCOUNTITEM iRow, HROW hRow);
	virtual BOOL VerifyRowHandles(DBCOUNTITEM cRows, HROW* rghRows, DBCOUNTITEM	iRowStart, ECOLUMNORDER eOrder = FORWARD);
	virtual BOOL CompareTableData(DBCOUNTITEM iTableRow, void* pData, DBCOUNTITEM cBindings = 0, DBBINDING* rgBindings = NULL);

    virtual BOOL VerifyAllRows();
    virtual BOOL CompareRowset(IUnknown* pIRowset);

	virtual BOOL ValidRow(HROW hRow);
	virtual BOOL ValidRow(DBCOUNTITEM cRows, HROW* rghRow);
	virtual BOOL IsSameRow(HROW hRow1, HROW hRow2);

	//Create Data
	virtual BOOL MakeRowData(void** ppData, HACCESSOR hAccessor = DB_NULL_HACCESSOR, DBCOUNTITEM* piRow = NULL);
	virtual BOOL ReleaseRowData(void* pData, HACCESSOR hAccessor = DB_NULL_HACCESSOR, BOOL fFreeBuffer = TRUE);

    //cast operator
	virtual IUnknown*	const pISession();
	virtual IAccessor*	const pIAccessor();
	virtual ICommand*	const pICommand();
	virtual IUnknown*	const pIUnknown();
	virtual IRowset*	const pIRowset();
	virtual IRowset*	const operator ()();

	virtual CTable* const pTable();
	virtual DBCOUNTITEM   GetMaxOpenRows() { return m_ulMaxOpenRows; }
	virtual BOOL    AllowOpenRows(DBCOUNTITEM cOpenRows) { return m_ulMaxOpenRows==0 || m_ulMaxOpenRows>=cOpenRows; }
		
	//data
	HACCESSOR			m_hAccessor;
	DBLENGTH			m_cRowSize;
	DBCOUNTITEM			m_cBindings;
	DBBINDING*			m_rgBinding;
	void*				m_pData;

	DBCOUNTITEM			m_ulTableRows;
	DBCOUNTITEM			m_ulMaxOpenRows;
	BOOL				m_bCanHoldRows;
	BOOL				m_bCanFetchBackwards;
	BOOL				m_bCanScrollBackwards;
	ULONG_PTR			m_ulpOleObjects;

protected:
	//data pointers
	IUnknown*		   m_pIUnknown;
	IRowset*		   m_pIRowset;
};


////////////////////////////////////////////////////////////////////
// Class CRowsetChange
//
// Conatins all the functionality of IRowsetChange as well as all the
// IRowset functionality
////////////////////////////////////////////////////////////////////

class CRowsetChange : public CRowset
{
public:
	//contsructor
	CRowsetChange(WCHAR* pwszTestCaseName = INVALID(WCHAR*));
    virtual ~CRowsetChange();
	virtual BOOL Terminate();

	// SetData methods
	virtual HRESULT ModifyRow(HROW hRow);
	virtual HRESULT ModifyRow(DBCOUNTITEM cRows, HROW* rghRow);
	virtual HRESULT SetData(HROW hRow, HACCESSOR hAccessor, void* pData);

	// Delete mothods
	virtual HRESULT DeleteRow(HROW hRow);
	virtual HRESULT DeleteRow(DBCOUNTITEM cRows, HROW* rghRow, DBROWSTATUS* rgRowStatus = NULL);

	// InsertRow methods
	virtual HRESULT InsertRow(HROW* phRow);
    virtual HRESULT InsertRow(DBCOUNTITEM cRows, HROW* rghRow);
	virtual HRESULT InsertRow(HACCESSOR hAccessor, void* pData, HROW* phRow);

    virtual HRESULT CreateRowset(EQUERY eQuery = SELECT_ORDERBYNUMERIC, REFIID riid = IID_IRowset, CTable* pCTable = NULL, DBACCESSORFLAGS dwAccessorFlags = DBACCESSOR_ROWDATA, DBPART dwPart = DBPART_ALL, DWORD dwColsBound = UPDATEABLE_COLS_BOUND, ECOLUMNORDER eBindingOrder = FORWARD, ECOLS_BY_REF	eColsByRef = NO_COLS_BY_REF, DBTYPE	wTypeModifier = DBTYPE_EMPTY, BLOBTYPE dwBlobType = NO_BLOB_COLS);
    virtual HRESULT CreateRowset(DBPROPID dwProperty, EQUERY eQuery = SELECT_ORDERBYNUMERIC, REFIID riid = IID_IRowset, CTable* pCTable = NULL, DBACCESSORFLAGS dwAccessorFlags = DBACCESSOR_ROWDATA, DBPART dwPart = DBPART_ALL, DWORD dwColsBound = UPDATEABLE_COLS_BOUND, ECOLUMNORDER eBindingOrder = FORWARD, ECOLS_BY_REF	eColsByRef = NO_COLS_BY_REF, DBTYPE	wTypeModifier = DBTYPE_EMPTY, BLOBTYPE dwBlobType = NO_BLOB_COLS);
    virtual HRESULT CreateRowset(IUnknown* pIRowset, EQUERY eQuery = SELECT_ORDERBYNUMERIC, REFIID riid = IID_IRowset, CTable* pCTable = NULL, DBACCESSORFLAGS dwAccessorFlags = DBACCESSOR_ROWDATA, DBPART dwPart = DBPART_ALL, DWORD dwColsBound = UPDATEABLE_COLS_BOUND, ECOLUMNORDER eBindingOrder = FORWARD, ECOLS_BY_REF	eColsByRef = NO_COLS_BY_REF, DBTYPE	wTypeModifier = DBTYPE_EMPTY, BLOBTYPE dwBlobType = NO_BLOB_COLS);

	//Interface to private members
	virtual IRowsetChange* const pIRowsetChange();
	
protected:
	//Data
	IRowsetChange*				m_pIRowsetChange;  
};
//////////////////////////////////////////////////////////////////
// Class CRowsetUpdate
//
// Conatins all the functionality of IRowsetUpdate as well as the
// ability of CRowsetChange
//////////////////////////////////////////////////////////////////


class CRowsetUpdate : public CRowsetChange
{
public:
	//contsructor
	CRowsetUpdate(WCHAR* pwszTestCaseName = INVALID(WCHAR*));
    virtual ~CRowsetUpdate();
	virtual BOOL Terminate();

	//Wrappers
	virtual HRESULT HardDeleteRow(HROW hRow);
	virtual HRESULT HardDeleteRow(DBCOUNTITEM cRows, HROW* rghRow);

	//IRowsetUpdate::Update
	virtual HRESULT UpdateRow(HROW hRow);
	virtual HRESULT UpdateRow(DBCOUNTITEM cRows, HROW* rghRow, DBCOUNTITEM* pcUpdatedRows = NULL, HROW** prgUpdatedRows = NULL, DBROWSTATUS** prgRowStatus = NULL);
	virtual HRESULT UpdateAll();
	
	//IRowsetUpdate::Undo
	virtual HRESULT UndoRow(HROW hRow);
	virtual HRESULT UndoRow(DBCOUNTITEM cRows,HROW* rghRow,DBCOUNTITEM* pcRowsUndone = NULL, HROW** prgRowsUndone = NULL, DBROWSTATUS** prgRowStatus = NULL);
	virtual HRESULT UndoAll();
	
	//IRowsetUpdate::GetPendingRows
	virtual HRESULT GetPendingRows(DBCOUNTITEM PendingRows);
	virtual HRESULT GetPendingRows(DBPENDINGSTATUS RowStatus, DBCOUNTITEM PendingRows);
	virtual HRESULT GetPendingRows(DBPENDINGSTATUS RowStatus, DBCOUNTITEM PendingRows, DBPENDINGSTATUS PendingStatus);
	
	//IRowsetUpdate::GetOriginalData
	virtual HRESULT GetOriginalData(HROW hRow, void** ppData);
	virtual HRESULT GetOriginalData(DBCOUNTITEM cRows, HROW* rghRow, void** ppData);

	//IRowsetUpdate::GetRowStatus
	virtual HRESULT GetRowStatus(HROW hRow, DBROWSTATUS RowStatus);
	virtual HRESULT GetRowStatus(DBCOUNTITEM cRows , HROW* rghRow, DBROWSTATUS RowStatus);

	//helpers
	virtual BOOL CompareOrgRowData(HROW hRow);
	
	virtual BOOL CompareOrgStorageData(HROW hRow, HACCESSOR hAccessor, void* pBuffer);
	virtual HRESULT GetOrgStorageData(HROW hRow, HACCESSOR hAccessor, void* pBuffer = NULL, ULONG* pcBytes = NULL, REFIID riid = IID_IUnknown, IUnknown** ppIUnknown = NULL);	
	
	virtual BOOL CompareWithDefaults(void* pData);

    virtual HRESULT CreateRowset(EQUERY eQuery = SELECT_ORDERBYNUMERIC, REFIID riid = IID_IRowset, CTable* pCTable = NULL, DBACCESSORFLAGS dwAccessorFlags = DBACCESSOR_ROWDATA, DBPART dwPart = DBPART_ALL, DWORD dwColsBound = UPDATEABLE_COLS_BOUND, ECOLUMNORDER eBindingOrder = FORWARD, ECOLS_BY_REF	eColsByRef = NO_COLS_BY_REF, DBTYPE	wTypeModifier = DBTYPE_EMPTY, BLOBTYPE dwBlobType = NO_BLOB_COLS);
    virtual HRESULT CreateRowset(DBPROPID dwProperty, EQUERY eQuery = SELECT_ORDERBYNUMERIC, REFIID riid = IID_IRowset, CTable* pCTable = NULL, DBACCESSORFLAGS dwAccessorFlags = DBACCESSOR_ROWDATA, DBPART dwPart = DBPART_ALL, DWORD dwColsBound = UPDATEABLE_COLS_BOUND, ECOLUMNORDER eBindingOrder = FORWARD, ECOLS_BY_REF	eColsByRef = NO_COLS_BY_REF, DBTYPE	wTypeModifier = DBTYPE_EMPTY, BLOBTYPE dwBlobType = NO_BLOB_COLS);
    virtual HRESULT CreateRowset(IUnknown* pIRowset, EQUERY eQuery = SELECT_ORDERBYNUMERIC, REFIID riid = IID_IRowset, CTable* pCTable = NULL, DBACCESSORFLAGS dwAccessorFlags = DBACCESSOR_ROWDATA, DBPART dwPart = DBPART_ALL, DWORD dwColsBound = UPDATEABLE_COLS_BOUND, ECOLUMNORDER eBindingOrder = FORWARD, ECOLS_BY_REF	eColsByRef = NO_COLS_BY_REF, DBTYPE	wTypeModifier = DBTYPE_EMPTY, BLOBTYPE dwBlobType = NO_BLOB_COLS);
	
	//Interface to private methods, can't modify
	virtual IRowsetUpdate* const pIRowsetUpdate();
	virtual DBCOUNTITEM   GetMaxPendingRows() { return m_ulMaxPendingRows; }
	virtual BOOL    AllowPendingRows(DBCOUNTITEM cPendingRows) { return m_ulMaxPendingRows==0 || m_ulMaxPendingRows>=cPendingRows; }

protected:
	//Data
	IRowsetUpdate*				m_pIRowsetUpdate;
	DBCOUNTITEM						m_ulMaxPendingRows;
};


///////////////////////////////////////////////////////////////////////////////
// Class CListener
// 
// Listens on the connections for notifications, (ie: a sink)
//////////////////////////////////////////////////////////////////////////////
class CListener : public CBase
{
public:
	CListener(REFIID riid = IID_IUnknown, IUnknown* pIUnknown = NULL);
	virtual ~CListener();

	//IUnknown
	DEFINE_QI_ADDREF_RELEASE;

	//Helpers
	virtual HRESULT Advise(DWORD* pdwCookie, IUnknown* pIUnknown = NULL, REFIID riid = IID_IUnknown);
	virtual HRESULT Unadvise(DWORD dwCookie, IUnknown* pIUnknown = NULL, REFIID riid = IID_IUnknown);

	virtual BOOL	SetNotifications(BOOL bValue = TRUE)				{	m_bFire = bValue;	return TRUE; }
	virtual BOOL	SetCancel(DBREASON eReason, DBEVENTPHASE ePhase)	{ 	m_eCancelReason = eReason;	m_eCancelPhase = ePhase; return TRUE; }
	virtual BOOL	SetError(HRESULT hr)								{	m_hrReturn = hr; return TRUE; }
	virtual BOOL	SetType(REFIID riid = IID_IUnknown)					{	m_iid = riid; return TRUE; }

	virtual BOOL	WantedEvent(DBREASON eReason, DBEVENTPHASE ePhase);
	virtual BOOL	SetEvent(DBREASON eReason, DBEVENTPHASE ePhase, BOOL bValue = TRUE);

	virtual BOOL	SetAdvise(DBREASON eReason, DBEVENTPHASE ePhase)	{	m_eAdviseReason	= eReason;	m_eAdvisePhase	= ePhase;	return TRUE; }	
	virtual BOOL	SetUnadvise(DBREASON eReason, DBEVENTPHASE ePhase)	{	m_eUnadviseReason = eReason; m_eUnadvisePhase = ePhase;	 return TRUE; }

	virtual BOOL	Restart();
	virtual BOOL	ResetTimesNotified(DBREASON eReason = DBREASON_ALL, DBEVENTPHASE ePhase = DBEVENTPHASE_ALL);
	virtual ULONG	GetTimesNotified(DBREASON eReason = DBREASON_ALL, DBEVENTPHASE ePhase = DBEVENTPHASE_ALL);
	virtual BOOL	IsCancelable(DBREASON eReason, DBEVENTPHASE ePhase, IUnknown* pIUnknown = NULL);
	virtual HRESULT	CauseNotification(DBREASON eReason, IUnknown* pIUnknown = NULL);

	virtual BOOL	SetReEntrantcy(DBREASON eReason, DBEVENTPHASE ePhase, void* pReEntrantFunc, void* pReEntrantData);
	virtual BOOL	GetReEntrantcy(DBREASON eReason, DBEVENTPHASE ePhase, void** ppReEntrantFunc, void** ppReEntrantData);

	//Helpers
	virtual HRESULT AcceptOrVeto(
						IUnknown*		pIUnknown,
						DBREASON		eReason,		// @parm IN	| Reason for notification
						DBEVENTPHASE	ePhase, 		// @parm IN	| Phase of event
						BOOL            fCantDeny);

protected:
	//Data
	//OPTIONAL
	CImpIRowsetNotify*				m_pCImpIRowsetNotify;
	CImpIDBAsynchNotify*			m_pCImpIDBAsynchNotify;
	CImpIRowPositionChange*			m_pCImpIRowPositionChange;

	DBREASON		m_eCancelReason;  // Weither to Cancel/Veto the notifcation
	DBEVENTPHASE	m_eCancelPhase;   // Weither to Cancel/Veto the notifcation

	DBREASON		m_eAdviseReason;	//Reason to advise another listener on
	DBEVENTPHASE	m_eAdvisePhase;		//Phase to advise another listener on

	DBREASON		m_eUnadviseReason;	//Reason to Unadvise another listener on
	DBEVENTPHASE	m_eUnadvisePhase;	//Phase to Unadvise another listener on

	HRESULT			m_hrReturn;     // Which HRESULT to return
	BOOL			m_bFire;        // Indicates weither to fire notifcations at all

	//2D array, of numer of times REASON / PHASE was notified
	ULONG			m_rgNotified[DBREASON_ALL][DBEVENTPHASE_ALL];

	//2D array, of which REASON / PHASE to fire
	ULONG			m_rgEvent[DBREASON_ALL][DBEVENTPHASE_ALL];
	DWORD			m_dwCookie;

	//Reentrant Data
	DBREASON		m_eReEntrantReason;		
	DBEVENTPHASE	m_eReEntrantPhase;		
	void*			m_pReEntrantFunc;		// func to call within the listener
	void*			m_pReEntrantData;

	//Source information (optional)
	IID				m_iid;
	IUnknown*		m_pIUnknown;
};


//REENTRANT_ROWSETFUNC - function to be called from within a listener...
typedef HRESULT (*REENTRANT_ROWSETFUNC)  (CRowset* pCRowset, IRowset* pIRowset, DBCOUNTITEM cRows, const HROW rghRows[], DBORDINAL cColumns, const DBORDINAL rgColumns[]);

////////////////////////////////////////////////////////
// CImpIRowsetNotify
//
////////////////////////////////////////////////////////
class CImpIRowsetNotify : public IRowsetNotify		//@base public | IRowsetNotify
{
	private: //@access private
		DEFINE_IUNKNOWN_MEMBER_DATA(CImpIRowsetNotify, IRowsetNotify)

	public: //@access public
 		DEFINE_IUNKNOWN_CTOR_DTOR(CImpIRowsetNotify, IRowsetNotify);
		DEFINE_DELEGATING_QI_ADDREF_RELEASE;

		//IRowsetNotify
		virtual HRESULT STDMETHODCALLTYPE OnFieldChange( 
				/* [in] */ IRowset __RPC_FAR *pRowset,
				/* [in] */ HROW hRow,
				/* [in] */ DBORDINAL cColumns,
				/* [size_is][in] */ DBORDINAL __RPC_FAR rgColumns[  ],
				/* [in] */ DBREASON eReason,
				/* [in] */ DBEVENTPHASE ePhase,
				/* [in] */ BOOL fCantDeny);
        
	    virtual HRESULT STDMETHODCALLTYPE OnRowChange( 
				/* [in] */ IRowset __RPC_FAR *pRowset,
				/* [in] */ DBCOUNTITEM cRows,
				/* [size_is][in] */ const HROW __RPC_FAR rghRows[  ],
				/* [in] */ DBREASON eReason,
				/* [in] */ DBEVENTPHASE ePhase,
				/* [in] */ BOOL fCantDeny);
        
		virtual HRESULT STDMETHODCALLTYPE OnRowsetChange( 
				/* [in] */ IRowset __RPC_FAR *pRowset,
				/* [in] */ DBREASON eReason,
				/* [in] */ DBEVENTPHASE ePhase,
				/* [in] */ BOOL fCantDeny);

		//Helpers
		virtual HRESULT AcceptOrVeto(
			IRowset*		pIRowset,
			DBREASON		eReason,		// @parm IN	| Reason for notification
			DBEVENTPHASE	ePhase, 		// @parm IN	| Phase of event
			BOOL            fCantDeny,
			DBCOUNTITEM     cRows       = 0,
			const HROW		rghRows[]   = NULL,
			DBORDINAL       cColumns    = 0,
			const DBORDINAL	rgColumns[] = NULL );

	protected:
		//data
};


////////////////////////////////////////////////////////
// CImpIDBAsynchNotify
//
////////////////////////////////////////////////////////
class CImpIDBAsynchNotify : public IDBAsynchNotify		//@base public | IDBAsynchNotify
{
	private: //@access private
		DEFINE_IUNKNOWN_MEMBER_DATA(CImpIDBAsynchNotify, IDBAsynchNotify)

	public: //@access public
 		DEFINE_IUNKNOWN_CTOR_DTOR(CImpIDBAsynchNotify, IDBAsynchNotify);
		DEFINE_DELEGATING_QI_ADDREF_RELEASE;

		//IDBAsynchNotify
		STDMETHOD(OnLowResource)( 
			/* [in] */ DB_DWRESERVE dwReserved) ;

		STDMETHOD(OnProgress)( 
			/* [in] */ HCHAPTER hChapter,
			/* [in] */ DBASYNCHOP ulOperation,
			/* [in] */ DBCOUNTITEM ulProgress,
			/* [in] */ DBCOUNTITEM ulProgressMax,
			/* [in] */ DBASYNCHPHASE ulAsynchPhase,
			/* [in] */ LPOLESTR pwszStatusText);

		STDMETHOD(OnStop)( 
			/* [in] */ HCHAPTER hChapter,
			/* [in] */ DBASYNCHOP ulOperation,
			/* [in] */ HRESULT hrStatus,
			/* [in] */ LPOLESTR pwszStatusText);
        
	
		//Helpers
		virtual HRESULT AcceptOrVeto(
				HCHAPTER		hChapter,
				DBASYNCHREASON	eReason,
				DBASYNCHOP		ulOperation,
				DBCOUNTITEM		ulProgress,            
				DBCOUNTITEM		ulProgressMax,            
				DBASYNCHPHASE	ePhase,
				LPOLESTR		pwszStatusText);

protected:
	//Data
};



////////////////////////////////////////////////////////
// CImpIRowPositionChange
//
////////////////////////////////////////////////////////
class CImpIRowPositionChange : public IRowPositionChange		//@base public | IRowPositionChange
{
	private: //@access private
		DEFINE_IUNKNOWN_MEMBER_DATA(CImpIRowPositionChange, IRowPositionChange)

	public: //@access public
 		DEFINE_IUNKNOWN_CTOR_DTOR(CImpIRowPositionChange, IRowPositionChange);
		DEFINE_DELEGATING_QI_ADDREF_RELEASE;

		//IRowPositionChange
		STDMETHOD(OnRowPositionChange)
				(
					DBREASON		eReason, 
					DBEVENTPHASE	ePhase, 
					BOOL			fCantDeny
				);
        
protected:
	//Data
};


#endif  // _EXTRALIB_H_