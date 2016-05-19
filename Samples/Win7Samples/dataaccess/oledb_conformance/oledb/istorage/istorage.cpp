//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright (C) 1995-2000 Microsoft Corporation
//
// @doc  
//
// @module IStorage.cpp | This module tests the OLE DB IStorage support 
//

//////////////////////////////////////////////////////////////////////////
// Includes
//
//////////////////////////////////////////////////////////////////////////
#define  DBINITCONSTANTS	// Must be defined to initialize constants in OLEDB.H
#define  INITGUID
#include "MODStandard.hpp"		// Standard headers			
#include "IStorage.h"			// IStorage header
#include "ExtraLib.h"


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Module Values
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// {{ TCW_MODULE_GLOBALS
DECLARE_MODULE_CLSID = { 0xf633fc10, 0xf649, 0x11cf, { 0xb0, 0x37, 0x00, 0xa0, 0xc9, 0x0d, 0x80, 0x7a }};
DECLARE_MODULE_NAME("IStorage");
DECLARE_MODULE_OWNER("Microsoft");
DECLARE_MODULE_DESCRIP("IStorage interface test");
DECLARE_MODULE_VERSION(838070366);
// TCW_WizardVersion(2)
// TCW_Automation(False)
// }} TCW_MODULE_GLOBALS_END


//--------------------------------------------------------------------
// @func Module level initialization routine
//
// @rdesc Success or Failure
// 		@flag  TRUE  | Successful initialization
//		@flag  FALSE | Initialization problems
//
BOOL ModuleInit(CThisTestModule * pThisTestModule)
{	
	TBEGIN
	
	if(CommonModuleInit(pThisTestModule, IID_IRowset))
	{
		//Determine which Storage interfaces are supported...
		//Must have at least of the storage interfaces tested by this test to continue
		ULONG_PTR dwValue = 0;
		TEST_PROVIDER(GetProperty(DBPROP_STRUCTUREDSTORAGE, DBPROPSET_DATASOURCEINFO, g_pIDBInitialize, &dwValue));
		TEST_PROVIDER(dwValue != 0);
		
		//If storage objects are supported, 
		//then ISequentialStream is required by all providers.
		TEST(dwValue && (dwValue & DBPROPVAL_SS_ISEQUENTIALSTREAM));
		return TEST_PASS;	
	}

	return TEST_FAIL;
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


////////////////////////////////////////////////////////////////////////////
//  rgInvalidStatus
//
////////////////////////////////////////////////////////////////////////////
//Invalid Status' to try...
//These are the only valid status' list in the spec for input on SetData/InsertRow
static const DBSTATUS rgInvalidStatus[] = 
{
//		DBSTATUS_S_OK	,
	DBSTATUS_E_BADACCESSOR	,
	DBSTATUS_E_CANTCONVERTVALUE	,
//		DBSTATUS_S_ISNULL	,
	DBSTATUS_S_TRUNCATED	,
	DBSTATUS_E_SIGNMISMATCH	,
	DBSTATUS_E_DATAOVERFLOW	,
	DBSTATUS_E_CANTCREATE	,
	DBSTATUS_E_UNAVAILABLE	,
	DBSTATUS_E_PERMISSIONDENIED	,
	DBSTATUS_E_INTEGRITYVIOLATION	,
	DBSTATUS_E_SCHEMAVIOLATION	,
	DBSTATUS_E_BADSTATUS	,
//		DBSTATUS_S_DEFAULT,
    MDSTATUS_S_CELLEMPTY	,
//		DBSTATUS_S_IGNORE,	
    DBSTATUS_E_DOESNOTEXIST	,
	DBSTATUS_E_INVALIDURL	,
	DBSTATUS_E_RESOURCELOCKED	,
	DBSTATUS_E_RESOURCEEXISTS	,
	DBSTATUS_E_CANNOTCOMPLETE	,
	DBSTATUS_E_VOLUMENOTFOUND	,
	DBSTATUS_E_OUTOFSPACE	,
	DBSTATUS_S_CANNOTDELETESOURCE	,
	DBSTATUS_E_READONLY	,
	DBSTATUS_E_RESOURCEOUTOFSCOPE	,
	DBSTATUS_S_ALREADYEXISTS,
	DBSTATUS_E_CANCELED	,
	DBSTATUS_E_NOTCOLLECTION,
};


////////////////////////////////////////////////////////////////////////////
//  TCBase
//
////////////////////////////////////////////////////////////////////////////
class TCBase
{
public:
	//constructor
	TCBase() { SetTestCaseParam(DBPROP_ISequentialStream); }

	//methods
	virtual void SetTestCaseParam(DBPROPID dwStorageID = DBPROP_ISequentialStream)
	{
		m_dwStorageID = dwStorageID;
		switch(dwStorageID)
		{
			case DBPROP_ISequentialStream:
				m_dwFlags		= BLOB_IID_ISEQSTREAM;
				m_dwPropVal		= DBPROPVAL_SS_ISEQUENTIALSTREAM;
				m_riidStorage	= IID_ISequentialStream; 
				break;
			
			case DBPROP_IStream:
				m_dwFlags		= BLOB_IID_ISTREAM;
				m_dwPropVal		= DBPROPVAL_SS_ISTREAM;
				m_riidStorage	= IID_IStream; 
				break;

			case DBPROP_ILockBytes:
				m_dwFlags		= BLOB_IID_ILOCKBYTES;
				m_dwPropVal		= DBPROPVAL_SS_ILOCKBYTES;
				m_riidStorage	= IID_ILockBytes; 
				break;

			default:
				ASSERT(!L"Unhandled Type...");
				break;
		};
	}

	//Supported Storage
	virtual BOOL SupportedStorageInterface(DBPROPID dwPropertyID = 0)
	{
		TBEGIN
		ULONG_PTR dwValue = 0;
		HACCESSOR hAccessor = DB_NULL_HACCESSOR;
		HROW  hRow = NULL;
		IUnknown* pIUnknown = NULL;
		IUnknown* pIUnkRetry = NULL;
		BOOL fRetry = FALSE;
		HRESULT hr = S_OK;
		CRowset RowsetA;

		//DBPROP_STRUCTUREDSTORAGE
		//This property should be supported if streams are supported to let the consumer
		//and application know this info ahead of time without having to obtain stream object.
		TESTC(GetProperty(DBPROP_STRUCTUREDSTORAGE, DBPROPSET_DATASOURCEINFO, g_pIDBInitialize, &dwValue));

		//NOTE: Some providers, actually support the storage interface on the actual object
		//but incorrectly support the property advertising it.  So instead of determining
		//which variations to run based upon the property, we run based upon whats exposed to
		//the consumer, since thats what they will use, if QI succeeds functions use interfaces.

		//Properties are really only useful for upfront info if the object has not been created 
		//yet and dynamic logic depends upon knowing it before the object is ever obtained,
		//ie: mainly used for dynamic UI senario, not functionality.

		//So first try to obtain the interface, then veriy the property support matches

		RowsetA.SetSettableProperty(DBPROP_ACCESSORDER, DBPROPSET_ROWSET, (void*)DBPROPVAL_AO_RANDOM, DBTYPE_I4);
		if(dwPropertyID)
			RowsetA.SetProperty(dwPropertyID, DBPROPSET_ROWSET);
		TEST2C_(hr = RowsetA.CreateRowset(m_dwStorageID, USE_OPENROWSET),S_OK,DB_E_ERRORSOCCURRED);
		if(FAILED(hr))
		{
			fRetry = TRUE;
			if(!dwPropertyID)
				TESTC(m_dwStorageID != DBPROP_ISequentialStream);
			
			//Provider might even fail rowset creation with this property, and still support
			//the storage object interface, so retry...
			RowsetA.FreeProperties();
			RowsetA.SetSettableProperty(DBPROP_ACCESSORDER, DBPROPSET_ROWSET, (void*)DBPROPVAL_AO_RANDOM, DBTYPE_I4);
			if(dwPropertyID)
				RowsetA.SetProperty(dwPropertyID, DBPROPSET_ROWSET);
			TEST2C_(hr = RowsetA.CreateRowset(DBPROP_ISequentialStream, USE_OPENROWSET), S_OK, dwPropertyID ? DB_E_ERRORSOCCURRED : S_OK);
			QTESTC(hr == S_OK);
		}
		
		//Grab a row handle
		TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK)

		//Create Accessor binding BLOB/Storage Object data
		TEST2C_(hr = RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, NULL, NULL, NULL, m_dwFlags), S_OK, DB_E_ERRORSOCCURRED);
		if(SUCCEEDED(hr))
		{
			//The provider may pass Accessor validation and fail GetData (deferred validation)
			TEST3C_(hr = RowsetA.GetStorageData(hRow, hAccessor, NULL, NULL, m_riidStorage, (IUnknown**)&pIUnknown),S_OK,DB_S_ERRORSOCCURRED,DB_E_ERRORSOCCURRED);
		}

		if(SUCCEEDED(hr))
		{
			//Should have supported the rowset property
			//NOTE: Log error and continue (so we can test the extra exposed interface)
			if(fRetry)
			{
				TERROR("Rowset storage property needs to be supported, since interface is available: " << GetInterfaceName(m_riidStorage));
				m_dwStorageID = DBPROP_ISequentialStream;
			}

			//Make sure the property Matches it
			//NOTE: Log error and continue (so we can test the extra exposed interface)
			if(!BITSET(dwValue, m_dwPropVal))
				TERROR("DBPROP_STRUCTUREDSTORAGE needs updating to indicate support for this storage interface: " << GetInterfaceName(m_riidStorage));

			//Make sure this interface is available...
			//Data might be NULL
			if(pIUnknown)
			{
				TESTC_(pIUnknown->QueryInterface(m_riidStorage, (void**)&pIUnkRetry),S_OK);
				TESTC(DefaultObjectTesting(pIUnkRetry, STREAM_INTERFACE));
			}
		}
		else
		{
			//Should not have supported the rowset property
			TESTC(fRetry == TRUE);

			//Obtain a SequentialStream interface (instead)
			TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, NULL, NULL, NULL, BLOB_IID_ISEQSTREAM), S_OK);

			//Obtain the Storage interface
			TESTC_(RowsetA.GetStorageData(hRow, hAccessor, NULL, NULL, IID_ISequentialStream, (IUnknown**)&pIUnknown), S_OK);
		
			//Make sure this interface is not available...
			//Data might be NULL
			if(pIUnknown)
			{
				TESTC_(pIUnknown->QueryInterface(m_riidStorage, (void**)&pIUnkRetry),E_NOINTERFACE);
			}
		
			//Make sure the property Matches it
			TESTC(!BITSET(dwValue, m_dwPropVal));

			//Truely unsupported interface, (skip variations)
			QTESTC(FALSE);
		}

	CLEANUP:
		RowsetA.ReleaseRows(hRow);
		RowsetA.ReleaseAccessor(hAccessor);
		SAFE_RELEASE(pIUnknown);
		SAFE_RELEASE(pIUnkRetry);
		TRETURN;
	}

	//data
	DBPROPID m_dwStorageID;
	DWORD m_dwFlags;
	DWORD m_dwPropVal;
	IID m_riidStorage;
};



////////////////////////////////////////////////////////////////////////////
//  TCTransactions
//
////////////////////////////////////////////////////////////////////////////
class TCTransactions : public CTransaction, public TCBase
{
public:
	//constructors
	TCTransactions(WCHAR* pwszTestCaseName = INVALID(WCHAR*)) : CTransaction(pwszTestCaseName) {}
};


////////////////////////////////////////////////////////////////////////////
//  TCStorage
//
////////////////////////////////////////////////////////////////////////////
class TCStorage : public COLEDB, public TCBase
{
public:
	//constructors
	TCStorage(WCHAR* pwszTestCaseName = INVALID(WCHAR*));
	virtual ~TCStorage();

	//methods
	virtual BOOL	Init();
	virtual BOOL	Terminate();

	virtual BOOL	VerifyAccessorValidation(CRowset* pCRowset, DBACCESSORFLAGS dwAccessorFlags, DWORD dwBlobType, HRESULT hrDefferred, DBBINDSTATUS dwBindStatus, BOOL fAllowSuccess = FALSE, ECOLS_BOUND eColsBound = UPDATEABLE_COLS_BOUND, DBBINDSTATUS** prgBindStatus = NULL, BOOL* pfDefferred = NULL, ECOLS_BY_REF eColsByRef = NO_COLS_BY_REF);
	virtual BOOL	CompareStorageBuffer(CRowset* pCRowset, HROW hRow, HACCESSOR hAccessor, CStorage* pCStorage);

	virtual HRESULT StorageRead(IUnknown* pIUnknown, void* pBuffer, DBLENGTH cBytes, DBLENGTH* pcBytesRead = NULL, DBLENGTH ulOffset = 0);
	static  HRESULT StorageRead(REFIID iid, IUnknown* pIUnknown, void* pBuffer, DBLENGTH cBytes, DBLENGTH* pcBytesRead = NULL, DBLENGTH ulOffset = 0);
	
	static  HRESULT StorageWrite(REFIID iid, IUnknown* pIUnknown, void* pBuffer, DBLENGTH cBytes, DBLENGTH* pcBytesWrote = NULL, DBLENGTH ulOffset = 0);
	virtual HRESULT StorageWrite(IUnknown* pIUnknown, void* pBuffer, DBLENGTH cBytes, DBLENGTH* pcBytesWrote = NULL, DBLENGTH ulOffset = 0);

	virtual HRESULT StorageSeek(IUnknown* pIUnknown, LONG lOffset, DWORD dwOrigin = STREAM_SEEK_SET, ULONG* pulOffset = NULL);
	virtual HRESULT StorageSetSize(IUnknown* pIUnknown, DBLENGTH cbSize);
	virtual HRESULT StorageCopyTo(IUnknown* pIUnknown, IStream* pIDestStream, ULONG cbBytes, ULONG* pcbRead = NULL, ULONG* pcbWritten = NULL);
	virtual HRESULT StorageStat(IUnknown* pIUnknown, STATSTG* pstatstg, DWORD grfStatFlag = STATFLAG_DEFAULT);
	virtual HRESULT StorageLockRegion(IUnknown* pIUnknown, ULONG ulOffset, ULONG cbBytes, DWORD dwLockType);
	virtual HRESULT StorageUnlockRegion(IUnknown* pIUnknown, ULONG ulOffset, ULONG cbBytes, DWORD dwLockType);

	//Thread routines
	static  ULONG WINAPI Thread_StorageRead(LPVOID pv);
	static  ULONG WINAPI Thread_StorageWrite(LPVOID pv);

	//data
	void*	m_pBuffer;
	void*	m_pBuffer2;
};


////////////////////////////////////////////////////////////////////////////
//  TCStorage::TCStorage
//
////////////////////////////////////////////////////////////////////////////
TCStorage::TCStorage(WCHAR* wstrTestCaseName) : COLEDB(wstrTestCaseName) 
{
	m_pBuffer	= NULL;
	m_pBuffer2	= NULL;
}


////////////////////////////////////////////////////////////////////////////
//  TCStorage::~TCStorage
//
////////////////////////////////////////////////////////////////////////////
TCStorage::~TCStorage()
{
	Terminate();
}


////////////////////////////////////////////////////////////////////////////
//  TCStorage::Init
//
////////////////////////////////////////////////////////////////////////////
BOOL TCStorage::Init()
{

 	TBEGIN
	
	//Allocate data buffers to use for variations...
	SAFE_ALLOC(m_pBuffer, void*, DATA_SIZE);
	SAFE_ALLOC(m_pBuffer2, void*, DATA_SIZE);

	//Initialize the CRowset object...
	return COLEDB::Init();

CLEANUP:
	TRETURN
}

////////////////////////////////////////////////////////////////////////////
//  TCStorage::Terminate
//
////////////////////////////////////////////////////////////////////////////
BOOL TCStorage::Terminate()
{
	SAFE_FREE(m_pBuffer);
	SAFE_FREE(m_pBuffer2);
	return COLEDB::Terminate();
}

	
///////////////////////////////////////////////////////////
// TCStorage::VerifyAccessorValidation
//
///////////////////////////////////////////////////////////
BOOL TCStorage::VerifyAccessorValidation(CRowset* pCRowset, DBACCESSORFLAGS dwAccessorFlags, DWORD dwBlobType, HRESULT hrDefferred, DBBINDSTATUS dwBindStatus, BOOL fAllowSuccess, ECOLS_BOUND eColsBound, DBBINDSTATUS** prgBindStatus, BOOL* pfDefferred, ECOLS_BY_REF eColsByRef)
{
	TBEGIN
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	HROW  hRow = NULL;
	DBLENGTH cBytes = 0;
	HRESULT hr = S_OK;
	
	DBORDINAL cBindings = 0;
	DBBINDING* rgBindings = NULL;
	DBBINDSTATUS* rgBindStatus = NULL;
	IUnknown* pIUnknown = NULL;

	//Create Accessor
	hr = pCRowset->CreateAccessor(&hAccessor, dwAccessorFlags, DBPART_ALL, &cBindings, &rgBindings, &cBytes, dwBlobType, &rgBindStatus, eColsBound, eColsByRef);
	TEST2C_(hr, S_OK, DB_E_ERRORSOCCURRED);

	//May have Deferred Accessor Validation
	if(hr == S_OK)
	{
		//Indicate Deferred Validation
		if(pfDefferred)
			*pfDefferred = TRUE;
		
		//Determine the expected interface
		IID iid = IID_NULL;
		if(dwBlobType & BLOB_IID_ISEQSTREAM)
			iid = IID_ISequentialStream;
		else if(dwBlobType & BLOB_IID_ISTORAGE)
			iid = IID_IStorage;
		else if(dwBlobType & BLOB_IID_ILOCKBYTES)
			iid = IID_ILockBytes;
		else if(dwBlobType & BLOB_IID_ISTREAM)
			iid = IID_IStream;
		else if(dwBlobType & BLOB_IID_IUNKNOWN)
			iid = IID_IUnknown;

		//Grab a row handle
		TESTC_(pCRowset->GetRow(FIRST_ROW, &hRow),S_OK);
		
		//Get the row data...
		hr = pCRowset->GetStorageData(hRow, hAccessor, NULL, NULL, iid, &pIUnknown);
		
		//There are "special" cases where even though there should be an error,
		//A success code can be returned.  ie:  DBPROP_MULTIPLESTORAGEOBJECTS=FALSE
		//We should not be able to obtain more than 1 storage object, but the spec
		//has been relaxed to indicate FALSE - "may" not be able to obtain more than one.
		//So we need to allow success in special circumstances 
		//(by default - fAllowSuccess = FALSE)
		if(fAllowSuccess)
		{
			TEST3C_(hr, S_OK, DB_S_ERRORSOCCURRED, hrDefferred);
		}
		else
		{
			QTESTC_(hr, hrDefferred);
		}
	}
	else
	{
		//Indicate Non-Deferred Validation
		if(pfDefferred)
			*pfDefferred = FALSE;

		//Verify Accessor Status
		//Only verify the Accessor Status if the user doesn't want the Status returned.
		//So for specical cases, the variation knows better what the status of each should be,
		//not neccessarly all the same...
		if(prgBindStatus == NULL)
			QTESTC(VerifyAccessorStatus(cBindings, rgBindings, rgBindStatus, DBTYPE_IUNKNOWN, dwBindStatus));
	}

CLEANUP:
	if(prgBindStatus)
		*prgBindStatus = rgBindStatus;
	else
		PROVIDER_FREE(rgBindStatus);
	
	//Can only release out-of-line memory when there is actual data
	//If this fails, we can't release this memory...
	pCRowset->ReleaseAccessor(hAccessor, cBindings, rgBindings);

	pCRowset->ReleaseRows(hRow);
	SAFE_RELEASE(pIUnknown);
	TRETURN;
}

/*
///////////////////////////////////////////////////////////
// TCStorage::CompareStorageData
//
///////////////////////////////////////////////////////////
BOOL TCStorage::CompareStorageData(CRowset* pCRowset, HROW hRow, HACCESSOR hAccessor, IUnknown* pIUnknown, REFIID riid)
{
	ASSERT(pCRowset && hRow && hAccessor && pCStorage);
	TBEGIN

	//This function would be called from GetData, similar to "CompareData".
	//It verifies that the returned stream, matches the backend data (privlib's data)...
	DBLENGTH cBytes = DATA_SIZE;
	void* pBuffer = PROVIDER_ALLOC(sizeof(void*)*cBytes);

	//Obtain the Data from the stream
	TESTC_(StorageRead(pIUnknown, pBuffer, DATA_SIZE, &cBytes), S_OK)

	//Compare the streams
	TESTC(pCStorage->Compare(cBytes, pBuffer));

CLEANUP:
	PROVIDER_FREE(pBuffer);
	TRETURN
}
*/

///////////////////////////////////////////////////////////
// TCStorage::CompareStorageBuffer
//
///////////////////////////////////////////////////////////
BOOL TCStorage::CompareStorageBuffer(CRowset* pCRowset, HROW hRow, HACCESSOR hAccessor, CStorage* pCStorage)
{
	ASSERT(pCRowset && hRow && hAccessor && pCStorage);
	TBEGIN

	//This function would be called from SetData, similar to "CompareBuffer".
	//It verifies that the passed in stream buffer, matches the rowset data, which verifies
	//that the data was actually set into the rowset and can be retrived...
	ULONG cBytes = DATA_SIZE;
	void* pBuffer = PROVIDER_ALLOC(sizeof(void*)*cBytes);

	//Get the rowset data (bound as a stream)
	TESTC_(pCRowset->GetStorageData(hRow, hAccessor, pBuffer, &cBytes, m_riidStorage),S_OK)

	//Compare the passed in stream, to the backend data
	TESTC(pCStorage->Compare(cBytes, pBuffer));

CLEANUP:
	PROVIDER_FREE(pBuffer);
	TRETURN
}


////////////////////////////////////////////////////////////////////////////
// StorageRead
//
////////////////////////////////////////////////////////////////////////////
HRESULT TCStorage::StorageRead(IUnknown* pIUnknown, void* pBuffer, DBLENGTH cBytes, DBLENGTH* pcBytesRead, DBLENGTH ulOffset)
{
	//Delegate
	return StorageRead(m_riidStorage, pIUnknown, pBuffer, cBytes, pcBytesRead, ulOffset);
}


////////////////////////////////////////////////////////////////////////////
// StorageRead
//
////////////////////////////////////////////////////////////////////////////
HRESULT TCStorage::StorageRead(REFIID iid, IUnknown* pIUnknown, void* pBuffer, DBLENGTH cBytes, DBLENGTH* pcBytesRead, DBLENGTH ulOffset)
{
	ULONG cbRead = 0;

	//Delegate
	HRESULT hr = ::StorageRead(iid, pIUnknown, pBuffer, (ULONG)cBytes, &cbRead, (ULONG)ulOffset);
	if(pcBytesRead)
		*pcBytesRead = cbRead;
	return hr;
}


////////////////////////////////////////////////////////////////////////////
// StorageWrite
//
////////////////////////////////////////////////////////////////////////////
HRESULT TCStorage::StorageWrite(IUnknown* pIUnknown, void* pBuffer, DBLENGTH cBytes, DBLENGTH* pcBytesWrote, DBLENGTH ulOffset)
{
	//Delegate
	return StorageWrite(m_riidStorage, pIUnknown, pBuffer, cBytes, pcBytesWrote, ulOffset);
}


////////////////////////////////////////////////////////////////////////////
// StorageWrite
//
////////////////////////////////////////////////////////////////////////////
HRESULT TCStorage::StorageWrite(REFIID iid, IUnknown* pIUnknown, void* pBuffer, DBLENGTH cBytes, DBLENGTH* pcBytesWrote, DBLENGTH ulOffset)
{
	ULONG cbWrote = 0;

	//Delegate
	HRESULT hr = ::StorageWrite(iid, pIUnknown, pBuffer, (ULONG)cBytes, &cbWrote, (ULONG)ulOffset);
	if(pcBytesWrote)
		*pcBytesWrote = cbWrote;
	return hr;
}


////////////////////////////////////////////////////////////////////////////
// StorageSeek
//
////////////////////////////////////////////////////////////////////////////
HRESULT TCStorage::StorageSeek(IUnknown* pIUnknown, LONG lOffset, DWORD dwOrigin, ULONG* pulOffset)
{
	HRESULT hr = S_OK;
	ULARGE_INTEGER ulNewPosition = { 0 };
	LARGE_INTEGER largeOffset;
	largeOffset.QuadPart = lOffset;
	
	//Invalid arg checks
	if(!pIUnknown)
		return E_INVALIDARG;

	if(m_riidStorage == IID_IStream)
	{
		//Seek to the new position
		IStream* pIStream = (IStream*)pIUnknown;
		hr = pIStream->Seek(largeOffset, dwOrigin, pulOffset ? &ulNewPosition : NULL);
	}
	else
	{
		//I have no clue what type of object this is.
		ASSERT(!L"Unhandled Storage interface...");
		hr = E_FAIL;
	}

	if(pulOffset)
		*pulOffset = (ULONG)ulNewPosition.QuadPart;
	return hr;
}


////////////////////////////////////////////////////////////////////////////
// StorageSetSize
//
////////////////////////////////////////////////////////////////////////////
HRESULT TCStorage::StorageSetSize(IUnknown* pIUnknown, DBLENGTH cbSize)
{
	HRESULT hr = S_OK;
	ULARGE_INTEGER largeSize = { (ULONG)cbSize };

	if(!pIUnknown)
		return E_INVALIDARG;

	if(m_riidStorage == IID_IStream)
	{
		//Seek to the new position
		IStream* pIStream = (IStream*)pIUnknown;
		hr = pIStream->SetSize(largeSize);
	}
	else if(m_riidStorage == IID_ILockBytes)
	{
		//Seek to the new position
		ILockBytes* pILockBytes = (ILockBytes*)pIUnknown;
		hr = pILockBytes->SetSize(largeSize);
	}
	else
	{
		//I have no clue what type of object this is.
		ASSERT(!L"Unhandled Storage interface...");
		hr = E_FAIL;
	}

	return hr;
}


////////////////////////////////////////////////////////////////////////////
// StorageCopyTo
//
////////////////////////////////////////////////////////////////////////////
HRESULT TCStorage::StorageCopyTo(IUnknown* pIUnknown, IStream* pIDestStream, ULONG cbBytes, ULONG* pcbRead, ULONG* pcbWritten)
{
	HRESULT hr = S_OK;
	ULARGE_INTEGER largeSize = { cbBytes };

	if(!pIUnknown)
		return E_INVALIDARG;

	if(m_riidStorage == IID_IStream)
	{
		ULARGE_INTEGER largecb = { cbBytes };
		ULARGE_INTEGER largeRead = { 0 };
		ULARGE_INTEGER largeWritten = { 0 };
		
		//CopyTo to the destination stream....
		IStream* pIStream = (IStream*)pIUnknown;
		hr = pIStream->CopyTo(pIDestStream, largecb, &largeRead, &largeWritten);

		if(pcbRead)
			*pcbRead = (ULONG)largeRead.QuadPart;
		if(pcbWritten)
			*pcbWritten = (ULONG)largeWritten.QuadPart;
	}
	else
	{
		//I have no clue what type of object this is.
		ASSERT(!L"Unhandled Storage interface...");
		hr = E_FAIL;
	}

	return hr;
}


////////////////////////////////////////////////////////////////////////////
// StorageStat
//
////////////////////////////////////////////////////////////////////////////
HRESULT TCStorage::StorageStat(IUnknown* pIUnknown, STATSTG* pstatstg, DWORD grfStatFlag)
{
	HRESULT hr = S_OK;

	if(!pIUnknown)
		return E_INVALIDARG;

	if(m_riidStorage == IID_IStream)
	{
		//Stat....
		IStream* pIStream = (IStream*)pIUnknown;
		hr = pIStream->Stat(pstatstg, grfStatFlag);
	}
	else if(m_riidStorage == IID_ILockBytes)
	{
		//Stat....
		ILockBytes* pILockBytes = (ILockBytes*)pIUnknown;
		hr = pILockBytes->Stat(pstatstg, grfStatFlag);
	}
	else
	{
		//I have no clue what type of object this is.
		ASSERT(!L"Unhandled Storage interface...");
		hr = E_FAIL;
	}

	return hr;
}


////////////////////////////////////////////////////////////////////////////
// StorageLockRegion
//
////////////////////////////////////////////////////////////////////////////
HRESULT TCStorage::StorageLockRegion(IUnknown* pIUnknown, ULONG ulOffset, ULONG cbBytes, DWORD dwLockType)
{
	HRESULT hr = S_OK;
	ULARGE_INTEGER largeOffset = { ulOffset };
	ULARGE_INTEGER largecBytes = { cbBytes };

	if(!pIUnknown)
		return E_INVALIDARG;
	
	if(m_riidStorage == IID_IStream)
	{
		//Lock....
		IStream* pIStream = (IStream*)pIUnknown;
		hr = pIStream->LockRegion(largeOffset, largecBytes, dwLockType);
	}
	else if(m_riidStorage == IID_ILockBytes)
	{
		//Lock....
		ILockBytes* pILockBytes = (ILockBytes*)pIUnknown;
		hr = pILockBytes->LockRegion(largeOffset, largecBytes, dwLockType);
	}
	else
	{
		//I have no clue what type of object this is.
		ASSERT(!L"Unhandled Storage interface...");
		hr = E_FAIL;
	}

	return hr;
}


	
////////////////////////////////////////////////////////////////////////////
// StorageUnlockRegion
//
////////////////////////////////////////////////////////////////////////////
HRESULT TCStorage::StorageUnlockRegion(IUnknown* pIUnknown, ULONG ulOffset, ULONG cbBytes, DWORD dwLockType)
{
	HRESULT hr = S_OK;
	ULARGE_INTEGER largeOffset = { ulOffset };
	ULARGE_INTEGER largecBytes = { cbBytes };

	if(!pIUnknown)
		return E_INVALIDARG;

	if(m_riidStorage == IID_IStream)
	{
		//Unlock....
		IStream* pIStream = (IStream*)pIUnknown;
		hr = pIStream->UnlockRegion(largeOffset, largecBytes, dwLockType);
	}
	else if(m_riidStorage == IID_ILockBytes)
	{
		//Unlock....
		ILockBytes* pILockBytes = (ILockBytes*)pIUnknown;
		hr = pILockBytes->UnlockRegion(largeOffset, largecBytes, dwLockType);
	}
	else
	{
		//I have no clue what type of object this is.
		ASSERT(!L"Unhandled Storage interface...");
		hr = E_FAIL;
	}

	return hr;
}


////////////////////////////////////////////////////////////////////////////
//  TCStorage::Thread_StorageRead
//
////////////////////////////////////////////////////////////////////////////
ULONG TCStorage::Thread_StorageRead(LPVOID pv)
{
	THREAD_BEGIN
	ASSERT(THREAD_FUNC && THREAD_ARG1 && THREAD_ARG3);

	//Thread Stack Variables
	IUnknown*	pIUnknown	= (IUnknown*)THREAD_FUNC;	//[in]
	IID*		pIID		= (IID*)THREAD_ARG1;		//[in]
	DBLENGTH	cBytes		= (DBLENGTH)THREAD_ARG2;	//[in]
	DBLENGTH*	pcBytesRead	= (DBLENGTH*)THREAD_ARG3;	//[out]

	cBytes  = min(cBytes, DATA_SIZE);
	void* pBuffer = NULL;
	SAFE_ALLOC(pBuffer, BYTE, cBytes);

	ThreadSwitch(); //Let the other threads Catch up

	//Call Read
	TEST2C_(TCStorage::StorageRead(*pIID, pIUnknown, pBuffer, cBytes, pcBytesRead),S_OK,S_FALSE);

	ThreadSwitch(); //Let the other thread(s) catch up

CLEANUP:
	PROVIDER_FREE(pBuffer);
	THREAD_RETURN
}

////////////////////////////////////////////////////////////////////////////
//  TCStorage::Thread_StorageWrite
//
////////////////////////////////////////////////////////////////////////////
ULONG TCStorage::Thread_StorageWrite(LPVOID pv)
{
	THREAD_BEGIN

	ASSERT(THREAD_FUNC && THREAD_ARG1 && THREAD_ARG3);

	//Thread Stack Variables
	IUnknown*	pIUnknown	= (IUnknown*)THREAD_FUNC;	//[in]
	IID*		pIID		= (IID*)THREAD_ARG1;		//[in]
	DBLENGTH	cBytes		= (DBLENGTH)THREAD_ARG2;	//[in]
	DBLENGTH*	pcBytesWrote= (DBLENGTH*)THREAD_ARG3;	//[out]

	cBytes  = min(cBytes, DATA_SIZE);
	void*   pBuffer = NULL;
	SAFE_ALLOC(pBuffer, BYTE, cBytes);

	ThreadSwitch(); //Let the other threads Catch up

	//Call Write
	//Should not be able to call Write, from any thread, or single threaded
	TESTC_(TCStorage::StorageWrite(*pIID, pIUnknown, pBuffer, cBytes, pcBytesWrote),STG_E_ACCESSDENIED);
	
	ThreadSwitch(); //Let the other thread(s) catch up

CLEANUP:
	PROVIDER_FREE(pBuffer);
	THREAD_RETURN
}





// {{ TCW_TEST_CASE_MAP(TCSeqStream_General)
//--------------------------------------------------------------------
// @class Test the ISequentialStream supported
//
class TCSeqStream_General : public TCStorage { 
public:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCSeqStream_General,TCStorage);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Empty
	int Variation_1();
	// @cmember Properties - DBPROP_DELAYSTORAGEOBJECTS
	int Variation_2();
	// @cmember Properties - DBPROP_TRANSACTEDOBJECT
	int Variation_3();
	// @cmember Properties - DBPROP_MULTIPLESTORAGEOBJECTS
	int Variation_4();
	// @cmember Properties - DBPROP_BLOCKINGSTORAGEOBJECTS
	int Variation_5();
	// @cmember Properties - DBPROP_STRUCTUREDSTORAGE
	int Variation_6();
	// @cmember Properties - DBPROP_OLEOBJECTS
	int Variation_7();
	// @cmember Empty
	int Variation_8();
	// @cmember Properties - Storage Property ie: DBPROP_ISequentialStream
	int Variation_9();
	// @cmember Empty
	int Variation_10();
	// @cmember Boundary - Seek
	int Variation_11();
	// @cmember Boundary - SetSize
	int Variation_12();
	// @cmember Boundary - CopyTo
	int Variation_13();
	// @cmember Boundary - Commit
	int Variation_14();
	// @cmember Boundary - Revert
	int Variation_15();
	// @cmember Boundary - LockRegion
	int Variation_16();
	// @cmember Boundary - UnlockRegion
	int Variation_17();
	// @cmember Boundary - Stat
	int Variation_18();
	// @cmember Boundary - Clone
	int Variation_19();
	// @cmember Boundary - ILockBytes::Flush
	int Variation_20();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCSeqStream_General)
#define THE_CLASS TCSeqStream_General
BEG_TEST_CASE(TCSeqStream_General, TCStorage, L"Test the ISequentialStream supported")
	TEST_VARIATION(1, 		L"Empty")
	TEST_VARIATION(2, 		L"Properties - DBPROP_DELAYSTORAGEOBJECTS")
	TEST_VARIATION(3, 		L"Properties - DBPROP_TRANSACTEDOBJECT")
	TEST_VARIATION(4, 		L"Properties - DBPROP_MULTIPLESTORAGEOBJECTS")
	TEST_VARIATION(5, 		L"Properties - DBPROP_BLOCKINGSTORAGEOBJECTS")
	TEST_VARIATION(6, 		L"Properties - DBPROP_STRUCTUREDSTORAGE")
	TEST_VARIATION(7, 		L"Properties - DBPROP_OLEOBJECTS")
	TEST_VARIATION(8, 		L"Empty")
	TEST_VARIATION(9, 		L"Properties - Storage Property ie: DBPROP_ISequentialStream")
	TEST_VARIATION(10, 		L"Empty")
	TEST_VARIATION(11, 		L"Boundary - Seek")
	TEST_VARIATION(12, 		L"Boundary - SetSize")
	TEST_VARIATION(13, 		L"Boundary - CopyTo")
	TEST_VARIATION(14, 		L"Boundary - Commit")
	TEST_VARIATION(15, 		L"Boundary - Revert")
	TEST_VARIATION(16, 		L"Boundary - LockRegion")
	TEST_VARIATION(17, 		L"Boundary - UnlockRegion")
	TEST_VARIATION(18, 		L"Boundary - Stat")
	TEST_VARIATION(19, 		L"Boundary - Clone")
	TEST_VARIATION(20, 		L"Boundary - ILockBytes::Flush")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END



// {{ TCW_TEST_CASE_MAP(TCSeqStream_Read)
//--------------------------------------------------------------------
// @class Test Storage Objects with GetData functionality [Read]
//
class TCSeqStream_Read : public TCStorage { 
public:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCSeqStream_Read,TCStorage);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember General - Verify RefCount of retrived Storage Objects interface
	int Variation_1();
	// @cmember Empty
	int Variation_2();
	// @cmember General - 1 - Specify IRowsetLocate
	int Variation_3();
	// @cmember General - 2 - Specify Exetened Fetch / no CANHOLDROWS / AnyOrder / AnyColumns, OWNINSERT
	int Variation_4();
	// @cmember General - 3 - DBPROP_ACCESSORDER - DBPROPVAL_AO_RANDOM
	int Variation_5();
	// @cmember General - 4 - DBPROP_ACCESSORDER - DBPROPVAL_AO_SEQUENTIALSTORAGEOBJECTS
	int Variation_6();
	// @cmember General - 4 - DBPROP_ACCESSORDER - DBPROPVAL_AO_SEQUENTIALSTORAGEOBJECTS - retrieve as if was random
	int Variation_7();
	// @cmember General - 4 - DBPROP_ACCESSORDER - DBPROPVAL_AO_SEQUENTIALSTORAGEOBJECTS - retrieve columns beyond stream
	int Variation_8();
	// @cmember Boundary - Open and Close stream, without reading
	int Variation_9();
	// @cmember Boundary - ::Read [valid, 0, NULL] - with outstanding row handles, accessors, and storage object
	int Variation_10();
	// @cmember Boundary - ::Read [valid, 0, NULL] - with outstanding row handles, accessors, and storage object without releasing
	int Variation_11();
	// @cmember Boundary - ::Read [valid, 0, valid]
	int Variation_12();
	// @cmember Boundary - ::Read [valid, N, NULL]
	int Variation_13();
	// @cmember Boundary - ::Read [valid, N, valid]
	int Variation_14();
	// @cmember Boundary - ::Read more than 1 call to Read to read the buffer
	int Variation_15();
	// @cmember Boundary - ::Read Even / Odd number of bytes
	int Variation_16();
	// @cmember Boundary - ::Write after retrieved Storage Objects pointer from GetData
	int Variation_17();
	// @cmember Empty
	int Variation_18();
	// @cmember Error - S_FALSE
	int Variation_19();
	// @cmember Error - STG_E_ACCESSDENIED
	int Variation_20();
	// @cmember Error - STG_E_REVERTED
	int Variation_21();
	// @cmember Error - STG_E_INVALIDPOINTER
	int Variation_22();
	// @cmember Parameters - pObject NULL
	int Variation_23();
	// @cmember Parameters - pObject->dwFlags STGM_READ
	int Variation_24();
	// @cmember Parameters - pObject->dwFlags STGM_READ | STGM_DIRECT
	int Variation_25();
	// @cmember Parameters - pObject->dwFlags STGM_WRITE
	int Variation_26();
	// @cmember Parameters - pObject->dwFlags STGM_READWRITE
	int Variation_27();
	// @cmember Parameters - pObject->dwFlags STGM_TRANSACTED
	int Variation_28();
	// @cmember Parameters - pObject->dwFlags ULONG_MAX
	int Variation_29();
	// @cmember Empty
	int Variation_30();
	// @cmember Parameters - pObject->iid IID_ISequentialStream
	int Variation_31();
	// @cmember Parameters - pObject->iid IID_IStorage
	int Variation_32();
	// @cmember Parameters - pObject->iid IID_IStream
	int Variation_33();
	// @cmember Parameters - pObject->iid IID_ILockBytes
	int Variation_34();
	// @cmember Parameters - pObject->iid IID_IUnknown
	int Variation_35();
	// @cmember Parameters - pObject->iid IID_NULL
	int Variation_36();
	// @cmember Empty
	int Variation_37();
	// @cmember Parameters - Multiple Storage columns
	int Variation_38();
	// @cmember Parameters - 1 Storage column, 1 long data
	int Variation_39();
	// @cmember Parameters - 1 Storage column, Multiple long data
	int Variation_40();
	// @cmember Parameters - NULL Storage column, GetData - GetNextRows - GetData, without CANHOLDROWS
	int Variation_41();
	// @cmember Parameters - NULL Storage column, GetData - GetNextRows - GetData, with CANHOLDROWS
	int Variation_42();
	// @cmember Parameters - NULL Storage column, GetData - GetData - GetData
	int Variation_43();
	// @cmember Sequence - GetData twice [same row]
	int Variation_44();
	// @cmember Sequence - GetData twice [same row] with releasing
	int Variation_45();
	// @cmember Sequence - GetData twice [diff row]
	int Variation_46();
	// @cmember Sequence - GetData twice [diff row] - with releasing
	int Variation_47();
	// @cmember Empty
	int Variation_48();
	// @cmember Sequence - GetData on Storage, with previous object open, no data read
	int Variation_49();
	// @cmember Sequence - GetData on Storage, with previous object open, but all data read
	int Variation_50();
	// @cmember Sequence - GetData on Storage, with previous object open, but partial data read
	int Variation_51();
	// @cmember Sequence - Obtain Storage object and release without ever reading
	int Variation_52();
	// @cmember IAccessor - BYREF storage column should fail
	int Variation_53();
	// @cmember IAccessor - VALUE only binding
	int Variation_54();
	// @cmember IAccessor - VALUE / LENGTH only binding
	int Variation_55();
	// @cmember IAccessor - VALUE /STATUS only binding
	int Variation_56();
	// @cmember IAccessor - LENGTH only binding
	int Variation_57();
	// @cmember IAccessor - STATUS only binding
	int Variation_58();
	// @cmember IAccessor - LENGTH / STATUS only binding
	int Variation_59();
	// @cmember IAccessor - DBACCESSOR_PASSBYREF
	int Variation_60();
	// @cmember Related - IRowsetResynch - GetVisibleData twice [same row]
	int Variation_61();
	// @cmember Related - IRowsetResynch - GetVisibleData twice [same row] with releasing
	int Variation_62();
	// @cmember Related - IRowsetResynch - GetVisibleData twice [diff row]
	int Variation_63();
	// @cmember Related - IRowsetResynch - GetVisibleData twice [diff row] with releasing
	int Variation_64();
	// @cmember Empty
	int Variation_65();
	// @cmember Related - IRowsetUpdate - GetOriginalData twice [same row]
	int Variation_66();
	// @cmember Related - IRowsetUpdate - GetOriginalData twice [same row] with releasing
	int Variation_67();
	// @cmember Related - IRowsetUpdate - GetOriginalData twice [diff row]
	int Variation_68();
	// @cmember Related - IRowsetUpdate - GetOriginalData twice [diff row] with releasing
	int Variation_69();
	// @cmember Empty
	int Variation_70();
	// @cmember ICommandWithParameters -
	int Variation_71();
	// @cmember Empty
	int Variation_72();
	// @cmember Usage - ReleaseRows
	int Variation_73();
	// @cmember Usage - ReleaseRows - ::Read E_UNEXPECTED
	int Variation_74();
	// @cmember Empty
	int Variation_75();
	// @cmember Stress - BLOBs beyond 64k
	int Variation_76();
	// @cmember Empty
	int Variation_77();
	// @cmember MultiUser - 2 Rowsets with GetData
	int Variation_78();
	// @cmember Empty
	int Variation_79();
	// @cmember MultiThreads - ::Read from the stream from seperate threads
	int Variation_80();
	// @cmember MultiThreads - ::Write to the stream from seperate threads
	int Variation_81();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCSeqStream_Read)
#define THE_CLASS TCSeqStream_Read
BEG_TEST_CASE(TCSeqStream_Read, TCStorage, L"Test Storage Objects with GetData functionality [Read]")
	TEST_VARIATION(1, 		L"General - Verify RefCount of retrived Storage Objects interface")
	TEST_VARIATION(2, 		L"Empty")
	TEST_VARIATION(3, 		L"General - 1 - Specify IRowsetLocate")
	TEST_VARIATION(4, 		L"General - 2 - Specify Exetened Fetch / no CANHOLDROWS / AnyOrder / AnyColumns, OWNINSERT")
	TEST_VARIATION(5, 		L"General - 3 - DBPROP_ACCESSORDER - DBPROPVAL_AO_RANDOM")
	TEST_VARIATION(6, 		L"General - 4 - DBPROP_ACCESSORDER - DBPROPVAL_AO_SEQUENTIALSTORAGEOBJECTS")
	TEST_VARIATION(7, 		L"General - 4 - DBPROP_ACCESSORDER - DBPROPVAL_AO_SEQUENTIALSTORAGEOBJECTS - retrieve as if was random")
	TEST_VARIATION(8, 		L"General - 4 - DBPROP_ACCESSORDER - DBPROPVAL_AO_SEQUENTIALSTORAGEOBJECTS - retrieve columns beyond stream")
	TEST_VARIATION(9, 		L"Boundary - Open and Close stream, without reading")
	TEST_VARIATION(10, 		L"Boundary - ::Read [valid, 0, NULL] - with outstanding row handles, accessors, and storage object")
	TEST_VARIATION(11, 		L"Boundary - ::Read [valid, 0, NULL] - with outstanding row handles, accessors, and storage object without releasing")
	TEST_VARIATION(12, 		L"Boundary - ::Read [valid, 0, valid]")
	TEST_VARIATION(13, 		L"Boundary - ::Read [valid, N, NULL]")
	TEST_VARIATION(14, 		L"Boundary - ::Read [valid, N, valid]")
	TEST_VARIATION(15, 		L"Boundary - ::Read more than 1 call to Read to read the buffer")
	TEST_VARIATION(16, 		L"Boundary - ::Read Even / Odd number of bytes")
	TEST_VARIATION(17, 		L"Boundary - ::Write after retrieved Storage Objects pointer from GetData")
	TEST_VARIATION(18, 		L"Empty")
	TEST_VARIATION(19, 		L"Error - S_FALSE")
	TEST_VARIATION(20, 		L"Error - STG_E_ACCESSDENIED")
	TEST_VARIATION(21, 		L"Error - STG_E_REVERTED")
	TEST_VARIATION(22, 		L"Error - STG_E_INVALIDPOINTER")
	TEST_VARIATION(23, 		L"Parameters - pObject NULL")
	TEST_VARIATION(24, 		L"Parameters - pObject->dwFlags STGM_READ")
	TEST_VARIATION(25, 		L"Parameters - pObject->dwFlags STGM_READ | STGM_DIRECT")
	TEST_VARIATION(26, 		L"Parameters - pObject->dwFlags STGM_WRITE")
	TEST_VARIATION(27, 		L"Parameters - pObject->dwFlags STGM_READWRITE")
	TEST_VARIATION(28, 		L"Parameters - pObject->dwFlags STGM_TRANSACTED")
	TEST_VARIATION(29, 		L"Parameters - pObject->dwFlags ULONG_MAX")
	TEST_VARIATION(30, 		L"Empty")
	TEST_VARIATION(31, 		L"Parameters - pObject->iid IID_ISequentialStream")
	TEST_VARIATION(32, 		L"Parameters - pObject->iid IID_IStorage")
	TEST_VARIATION(33, 		L"Parameters - pObject->iid IID_IStream")
	TEST_VARIATION(34, 		L"Parameters - pObject->iid IID_ILockBytes")
	TEST_VARIATION(35, 		L"Parameters - pObject->iid IID_IUnknown")
	TEST_VARIATION(36, 		L"Parameters - pObject->iid IID_NULL")
	TEST_VARIATION(37, 		L"Empty")
	TEST_VARIATION(38, 		L"Parameters - Multiple Storage columns")
	TEST_VARIATION(39, 		L"Parameters - 1 Storage column, 1 long data")
	TEST_VARIATION(40, 		L"Parameters - 1 Storage column, Multiple long data")
	TEST_VARIATION(41, 		L"Parameters - NULL Storage column, GetData - GetNextRows - GetData, without CANHOLDROWS")
	TEST_VARIATION(42, 		L"Parameters - NULL Storage column, GetData - GetNextRows - GetData, with CANHOLDROWS")
	TEST_VARIATION(43, 		L"Parameters - NULL Storage column, GetData - GetData - GetData")
	TEST_VARIATION(44, 		L"Sequence - GetData twice [same row]")
	TEST_VARIATION(45, 		L"Sequence - GetData twice [same row] with releasing")
	TEST_VARIATION(46, 		L"Sequence - GetData twice [diff row]")
	TEST_VARIATION(47, 		L"Sequence - GetData twice [diff row] - with releasing")
	TEST_VARIATION(48, 		L"Empty")
	TEST_VARIATION(49, 		L"Sequence - GetData on Storage, with previous object open, no data read")
	TEST_VARIATION(50, 		L"Sequence - GetData on Storage, with previous object open, but all data read")
	TEST_VARIATION(51, 		L"Sequence - GetData on Storage, with previous object open, but partial data read")
	TEST_VARIATION(52, 		L"Sequence - Obtain Storage object and release without ever reading")
	TEST_VARIATION(53, 		L"IAccessor - BYREF storage column should fail")
	TEST_VARIATION(54, 		L"IAccessor - VALUE only binding")
	TEST_VARIATION(55, 		L"IAccessor - VALUE / LENGTH only binding")
	TEST_VARIATION(56, 		L"IAccessor - VALUE /STATUS only binding")
	TEST_VARIATION(57, 		L"IAccessor - LENGTH only binding")
	TEST_VARIATION(58, 		L"IAccessor - STATUS only binding")
	TEST_VARIATION(59, 		L"IAccessor - LENGTH / STATUS only binding")
	TEST_VARIATION(60, 		L"IAccessor - DBACCESSOR_PASSBYREF")
	TEST_VARIATION(61, 		L"Related - IRowsetResynch - GetVisibleData twice [same row]")
	TEST_VARIATION(62, 		L"Related - IRowsetResynch - GetVisibleData twice [same row] with releasing")
	TEST_VARIATION(63, 		L"Related - IRowsetResynch - GetVisibleData twice [diff row]")
	TEST_VARIATION(64, 		L"Related - IRowsetResynch - GetVisibleData twice [diff row] with releasing")
	TEST_VARIATION(65, 		L"Empty")
	TEST_VARIATION(66, 		L"Related - IRowsetUpdate - GetOriginalData twice [same row]")
	TEST_VARIATION(67, 		L"Related - IRowsetUpdate - GetOriginalData twice [same row] with releasing")
	TEST_VARIATION(68, 		L"Related - IRowsetUpdate - GetOriginalData twice [diff row]")
	TEST_VARIATION(69, 		L"Related - IRowsetUpdate - GetOriginalData twice [diff row] with releasing")
	TEST_VARIATION(70, 		L"Empty")
	TEST_VARIATION(71, 		L"ICommandWithParameters -")
	TEST_VARIATION(72, 		L"Empty")
	TEST_VARIATION(73, 		L"Usage - ReleaseRows")
	TEST_VARIATION(74, 		L"Usage - ReleaseRows - ::Read E_UNEXPECTED")
	TEST_VARIATION(75, 		L"Empty")
	TEST_VARIATION(76, 		L"Stress - BLOBs beyond 64k")
	TEST_VARIATION(77, 		L"Empty")
	TEST_VARIATION(78, 		L"MultiUser - 2 Rowsets with GetData")
	TEST_VARIATION(79, 		L"Empty")
	TEST_VARIATION(80, 		L"MultiThreads - ::Read from the stream from seperate threads")
	TEST_VARIATION(81, 		L"MultiThreads - ::Write to the stream from seperate threads")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCSeqStream_Write)
//--------------------------------------------------------------------
// @class Test Storage Objects with SetData functionality [Read]
//
class TCSeqStream_Write : public TCStorage { 
public:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCSeqStream_Write,TCStorage);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember General - Verfiy RefCount, that provider releases storage object
	int Variation_1();
	// @cmember Empty
	int Variation_2();
	// @cmember Boundary - SetData with 0 buffer size
	int Variation_3();
	// @cmember Boundary - SetData with small buffer size
	int Variation_4();
	// @cmember Boundary - SetData with large buffer size
	int Variation_5();
	// @cmember Boundary - SetData with equal buffer size
	int Variation_6();
	// @cmember Boundary - SetData with even/odd number of bytes
	int Variation_7();
	// @cmember Empty
	int Variation_8();
	// @cmember Boundary - InsertRow with 0 buffer size
	int Variation_9();
	// @cmember Boundary - InsertRow with small buffer size
	int Variation_10();
	// @cmember Boundary - InsertRow with large buffer size
	int Variation_11();
	// @cmember Boundary - InsertRow with equal buffer size
	int Variation_12();
	// @cmember Boundary - InsertRow with even/odd number of bytes
	int Variation_13();
	// @cmember Empty
	int Variation_14();
	// @cmember Parameters - SetData with NULL pObject
	int Variation_15();
	// @cmember Parameters - SetData with NULL interface pointer
	int Variation_16();
	// @cmember Parameters - SetData with DBSTATUS_S_ISNULL storage column
	int Variation_17();
	// @cmember Parameters - SetData with DBSTATUS_S_IGNORE storage column
	int Variation_18();
	// @cmember Parameters - SetData with DBSTATUS_S_DEFAULT storage column
	int Variation_19();
	// @cmember Parameters - SetData with Invalid DBSTATUS
	int Variation_20();
	// @cmember Empty
	int Variation_21();
	// @cmember Parameters - SetData twice [same row]
	int Variation_22();
	// @cmember Parameters - SetData twice [with 2 open storage objects]
	int Variation_23();
	// @cmember Parameters - SetData twice [diff rows]
	int Variation_24();
	// @cmember Parameters - SetData twice [with 2 open storage objects] diff rows
	int Variation_25();
	// @cmember Parameters - SetData -> GetData
	int Variation_26();
	// @cmember Parameters - SetData -> GetData [diff rows]
	int Variation_27();
	// @cmember Empty
	int Variation_28();
	// @cmember Parameters - InsertRow with NULL pObject
	int Variation_29();
	// @cmember Parameters - InsertRow with NULL interface pointer
	int Variation_30();
	// @cmember Parameters - InsertRow with DBSTATUS_S_ISNULL storage column
	int Variation_31();
	// @cmember Parameters - InsertRow with DBSTATUS_S_IGNORE storage column
	int Variation_32();
	// @cmember Parameters - InsertRow with DBSTATUS_S_DEFAULT storage column
	int Variation_33();
	// @cmember Parameters - InsertRow with Invalid DBSTATUS
	int Variation_34();
	// @cmember Empty
	int Variation_35();
	// @cmember Parameters - InsertRow 2 rows
	int Variation_36();
	// @cmember Parameters - InsertRow 2 rows [with 2 open storage objects]
	int Variation_37();
	// @cmember Parameters - InsertRow -> GetData
	int Variation_38();
	// @cmember Parameters - InsertRow -> SetData
	int Variation_39();
	// @cmember Empty
	int Variation_40();
	// @cmember Related - IAccessor - Bind LENGTH with more bytes than in buffer
	int Variation_41();
	// @cmember Related - IAccessor - Bind LENGTH with less bytes than in buffer
	int Variation_42();
	// @cmember Related - IAccessor - SetData for Storage column, GetData binding as long
	int Variation_43();
	// @cmember Related - IAccessor - SetData NULL Storage column, GetData binding as long DBSTATUS_S_ISNULL
	int Variation_44();
	// @cmember Empty
	int Variation_45();
	// @cmember Related - IRowsetResynch - Modify / Resynch
	int Variation_46();
	// @cmember Related - IRowsetResynch - Insert / Resynch
	int Variation_47();
	// @cmember Empty
	int Variation_48();
	// @cmember Related - DeleteRows - Delete a row with an open storage object
	int Variation_49();
	// @cmember Related - DeleteRows - Modify / DeleteRows
	int Variation_50();
	// @cmember Related - DeleteRows - Insert / DeleteRows
	int Variation_51();
	// @cmember Empty
	int Variation_52();
	// @cmember Sequence - GetData - SetData - GetData - Verify
	int Variation_53();
	// @cmember Sequence - GetData - DeleteRow - GetData - Verify
	int Variation_54();
	// @cmember Sequence - GetData - InsertRow - GetData - Verify
	int Variation_55();
	// @cmember Empty
	int Variation_56();
	// @cmember MultiUser - 2 rowsets, SetData on 1, InsertRow on the other
	int Variation_57();
	// @cmember Boundary - SetData with not direct interface
	int Variation_58();
	// @cmember Boundary - SetData with non-Storage interface
	int Variation_59();
	// @cmember Boundary - SetData verify objects released on error
	int Variation_60();
	// @cmember Empty
	int Variation_61();
	// @cmember Boundary -InsertRow with not direct interface
	int Variation_62();
	// @cmember Boundary - InsertRow with non-Storage interface
	int Variation_63();
	// @cmember Boundary - InsertRow verify objects released on error
	int Variation_64();
	// @cmember Empty
	int Variation_65();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCSeqStream_Write)
#define THE_CLASS TCSeqStream_Write
BEG_TEST_CASE(TCSeqStream_Write, TCStorage, L"Test Storage Objects with SetData functionality [Read]")
	TEST_VARIATION(1, 		L"General - Verfiy RefCount, that provider releases storage object")
	TEST_VARIATION(2, 		L"Empty")
	TEST_VARIATION(3, 		L"Boundary - SetData with 0 buffer size")
	TEST_VARIATION(4, 		L"Boundary - SetData with small buffer size")
	TEST_VARIATION(5, 		L"Boundary - SetData with large buffer size")
	TEST_VARIATION(6, 		L"Boundary - SetData with equal buffer size")
	TEST_VARIATION(7, 		L"Boundary - SetData with even/odd number of bytes")
	TEST_VARIATION(8, 		L"Empty")
	TEST_VARIATION(9, 		L"Boundary - InsertRow with 0 buffer size")
	TEST_VARIATION(10, 		L"Boundary - InsertRow with small buffer size")
	TEST_VARIATION(11, 		L"Boundary - InsertRow with large buffer size")
	TEST_VARIATION(12, 		L"Boundary - InsertRow with equal buffer size")
	TEST_VARIATION(13, 		L"Boundary - InsertRow with even/odd number of bytes")
	TEST_VARIATION(14, 		L"Empty")
	TEST_VARIATION(15, 		L"Parameters - SetData with NULL pObject")
	TEST_VARIATION(16, 		L"Parameters - SetData with NULL interface pointer")
	TEST_VARIATION(17, 		L"Parameters - SetData with DBSTATUS_S_ISNULL storage column")
	TEST_VARIATION(18, 		L"Parameters - SetData with DBSTATUS_S_IGNORE storage column")
	TEST_VARIATION(19, 		L"Parameters - SetData with DBSTATUS_S_DEFAULT storage column")
	TEST_VARIATION(20, 		L"Parameters - SetData with Invalid DBSTATUS")
	TEST_VARIATION(21, 		L"Empty")
	TEST_VARIATION(22, 		L"Parameters - SetData twice [same row]")
	TEST_VARIATION(23, 		L"Parameters - SetData twice [with 2 open storage objects]")
	TEST_VARIATION(24, 		L"Parameters - SetData twice [diff rows]")
	TEST_VARIATION(25, 		L"Parameters - SetData twice [with 2 open storage objects] diff rows")
	TEST_VARIATION(26, 		L"Parameters - SetData -> GetData")
	TEST_VARIATION(27, 		L"Parameters - SetData -> GetData [diff rows]")
	TEST_VARIATION(28, 		L"Empty")
	TEST_VARIATION(29, 		L"Parameters - InsertRow with NULL pObject")
	TEST_VARIATION(30, 		L"Parameters - InsertRow with NULL interface pointer")
	TEST_VARIATION(31, 		L"Parameters - InsertRow with DBSTATUS_S_ISNULL storage column")
	TEST_VARIATION(32, 		L"Parameters - InsertRow with DBSTATUS_S_IGNORE storage column")
	TEST_VARIATION(33, 		L"Parameters - InsertRow with DBSTATUS_S_DEFAULT storage column")
	TEST_VARIATION(34, 		L"Parameters - InsertRow with Invalid DBSTATUS")
	TEST_VARIATION(35, 		L"Empty")
	TEST_VARIATION(36, 		L"Parameters - InsertRow 2 rows")
	TEST_VARIATION(37, 		L"Parameters - InsertRow 2 rows [with 2 open storage objects]")
	TEST_VARIATION(38, 		L"Parameters - InsertRow -> GetData")
	TEST_VARIATION(39, 		L"Parameters - InsertRow -> SetData")
	TEST_VARIATION(40, 		L"Empty")
	TEST_VARIATION(41, 		L"Related - IAccessor - Bind LENGTH with more bytes than in buffer")
	TEST_VARIATION(42, 		L"Related - IAccessor - Bind LENGTH with less bytes than in buffer")
	TEST_VARIATION(43, 		L"Related - IAccessor - SetData for Storage column, GetData binding as long")
	TEST_VARIATION(44, 		L"Related - IAccessor - SetData NULL Storage column, GetData binding as long DBSTATUS_S_ISNULL")
	TEST_VARIATION(45, 		L"Empty")
	TEST_VARIATION(46, 		L"Related - IRowsetResynch - Modify / Resynch")
	TEST_VARIATION(47, 		L"Related - IRowsetResynch - Insert / Resynch")
	TEST_VARIATION(48, 		L"Empty")
	TEST_VARIATION(49, 		L"Related - DeleteRows - Delete a row with an open storage object")
	TEST_VARIATION(50, 		L"Related - DeleteRows - Modify / DeleteRows")
	TEST_VARIATION(51, 		L"Related - DeleteRows - Insert / DeleteRows")
	TEST_VARIATION(52, 		L"Empty")
	TEST_VARIATION(53, 		L"Sequence - GetData - SetData - GetData - Verify")
	TEST_VARIATION(54, 		L"Sequence - GetData - DeleteRow - GetData - Verify")
	TEST_VARIATION(55, 		L"Sequence - GetData - InsertRow - GetData - Verify")
	TEST_VARIATION(56, 		L"Empty")
	TEST_VARIATION(57, 		L"MultiUser - 2 rowsets, SetData on 1, InsertRow on the other")
	TEST_VARIATION(58, 		L"Boundary - SetData with not direct interface")
	TEST_VARIATION(59, 		L"Boundary - SetData with non-Storage interface")
	TEST_VARIATION(60, 		L"Boundary - SetData verify objects released on error")
	TEST_VARIATION(61, 		L"Empty")
	TEST_VARIATION(62, 		L"Boundary -InsertRow with not direct interface")
	TEST_VARIATION(63, 		L"Boundary - InsertRow with non-Storage interface")
	TEST_VARIATION(64, 		L"Boundary - InsertRow verify objects released on error")
	TEST_VARIATION(65, 		L"Empty")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCSeqStream_Buffered)
//--------------------------------------------------------------------
// @class Test ISequentialStream support in Buffered Update mode
//
class TCSeqStream_Buffered : public TCStorage { 
public:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCSeqStream_Buffered,TCStorage);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember SetData -> GetData -> GetData
	int Variation_1();
	// @cmember SetData -> GetData -> Update
	int Variation_2();
	// @cmember SetData -> GetOriginalData -> GetOriginalData
	int Variation_3();
	// @cmember SetData -> GetOriginalData -> Update
	int Variation_4();
	// @cmember Empty
	int Variation_5();
	// @cmember SetData -> Update -> GetData -> GetData == GetOriginalData
	int Variation_6();
	// @cmember SetData -> Update -> GetOriginalData -> GetOriginalData == GetData
	int Variation_7();
	// @cmember Empty
	int Variation_8();
	// @cmember SetData -> Undo -> GetData -> GetData == GetOriginalData
	int Variation_9();
	// @cmember SetData -> Undo -> GetOriginalData -> GetOriginalData == GetData
	int Variation_10();
	// @cmember Empty
	int Variation_11();
	// @cmember SetData [row1] -> SetData [row1] -> Update
	int Variation_12();
	// @cmember SetData [row1] -> SetData [row2] -> Update
	int Variation_13();
	// @cmember SetData [row1] -> SetData [row2] -> Update [row1]
	int Variation_14();
	// @cmember Empty
	int Variation_15();
	// @cmember SetData [row1] -> SetData [row1] -> Undo [row1]
	int Variation_16();
	// @cmember SetData [row1] -> SetData [row2] -> Undo [row1]
	int Variation_17();
	// @cmember SetData [row1] -> SetData [row2] -> Undo [row2]
	int Variation_18();
	// @cmember Empty
	int Variation_19();
	// @cmember SetData -> SetData [NULL Storage Objects interface] -> Update
	int Variation_20();
	// @cmember SetData -> SetData [NULL Storage Objects interface] -> Undo
	int Variation_21();
	// @cmember Empty
	int Variation_22();
	// @cmember SetData (row1
	int Variation_23();
	// @cmember Empty
	int Variation_24();
	// @cmember SetData (with ::Read that returns an error
	int Variation_25();
	// @cmember SetData (with ::Read that returns an error
	int Variation_26();
	// @cmember Empty
	int Variation_27();
	// @cmember SetData -> GetData (binding as long
	int Variation_28();
	// @cmember SetData -> GetOriginalData (binding as long
	int Variation_29();
	// @cmember SetData -> Update -> GetData (binding as long
	int Variation_30();
	// @cmember SetData -> Update -> GetOriginalData (binding as long
	int Variation_31();
	// @cmember Empty
	int Variation_32();
	// @cmember Insert -> GetData -> GetData
	int Variation_33();
	// @cmember Insert -> GetData -> Update
	int Variation_34();
	// @cmember Insert -> GetOriginalData -> GetOriginalData
	int Variation_35();
	// @cmember Insert -> GetOriginalData -> Update
	int Variation_36();
	// @cmember Empty
	int Variation_37();
	// @cmember Insert -> Update -> GetData -> GetData
	int Variation_38();
	// @cmember Insert -> Update -> GetOriginalData -> GetOrignialData
	int Variation_39();
	// @cmember Insert -> Undo -> GetOriginalData -> GetOriginalData
	int Variation_40();
	// @cmember Empty
	int Variation_41();
	// @cmember Insert [rowX] -> Insert [rowY] -> Update [rowY]
	int Variation_42();
	// @cmember Insert [rowX] -> Insert [rowY] -> Update [rowX]
	int Variation_43();
	// @cmember Insert [rowX] -> Insert [rowY] -> Update all
	int Variation_44();
	// @cmember Empty
	int Variation_45();
	// @cmember Insert [rowX] -> Insert [rowY] -> Undo [rowY]
	int Variation_46();
	// @cmember Insert [rowX] -> Insert [rowY] -> Undo [rowX]
	int Variation_47();
	// @cmember Insert [rowX] -> Insert [rowY] -> Undo all
	int Variation_48();
	// @cmember Empty
	int Variation_49();
	// @cmember Delete -> GetOriginalData -> GetOriginalData
	int Variation_50();
	// @cmember Delete -> GetOriginalData -> GetOriginalData -> Update
	int Variation_51();
	// @cmember Delete -> GetOriginalData -> GetOriginalData -> Undo
	int Variation_52();
	// @cmember Empty
	int Variation_53();
	// @cmember Delete [row1] -> Delete [row2] -> Update
	int Variation_54();
	// @cmember Delete [row1] -> Delete [row2] -> Undo
	int Variation_55();
	// @cmember Delete [row1] -> Delete [row2] -> Update [row1]
	int Variation_56();
	// @cmember Delete [row1] -> Delete [row2] -> Update [row2]
	int Variation_57();
	// @cmember Delete [row1] -> Delete [row2] -> Undo [row1]
	int Variation_58();
	// @cmember Delete [row1] -> Delete [row2] -> Undo [row2]
	int Variation_59();
	// @cmember Empty
	int Variation_60();
	// @cmember Sequence - SetData -> Delete -> Update
	int Variation_61();
	// @cmember Sequence - SetData -> Delete -> Undo
	int Variation_62();
	// @cmember Empty
	int Variation_63();
	// @cmember Sequence - SetData [row1] -> Delete [row2] -> Update
	int Variation_64();
	// @cmember Empty
	int Variation_65();
	// @cmember Sequence - Insert -> SetData -> Update
	int Variation_66();
	// @cmember Sequence - Insert -> SetData -> Undo
	int Variation_67();
	// @cmember Empty
	int Variation_68();
	// @cmember Sequence - Insert -> SetData -> Delete -> Update
	int Variation_69();
	// @cmember Sequence - Insert -> SetData -> Delete -> Undo
	int Variation_70();
	// @cmember Sequence - Insert [rowX] -> SetData [rowY] -> Delete [rowZ] -> Update all
	int Variation_71();
	// @cmember Empty
	int Variation_72();
	// @cmember Release - Get [row1] -> SetData -> ReleaseRows -> Update
	int Variation_73();
	// @cmember Release - Get [row1] -> SetData -> ReleaseRows -> Undo
	int Variation_74();
	// @cmember Empty
	int Variation_75();
	// @cmember Release - Insert -> ReleaseRows -> Update
	int Variation_76();
	// @cmember Release - Insert -> ReleaseRows -> Undo
	int Variation_77();
	// @cmember Empty
	int Variation_78();
	// @cmember Release - Get [row1] -> Delete -> ReleaseRows -> Update
	int Variation_79();
	// @cmember Release - Get [row1] -> Delete -> ReleaseRows -> Undo
	int Variation_80();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCSeqStream_Buffered)
#define THE_CLASS TCSeqStream_Buffered
BEG_TEST_CASE(TCSeqStream_Buffered, TCStorage, L"Test ISequentialStream support in Buffered Update mode")
	TEST_VARIATION(1, 		L"SetData -> GetData -> GetData")
	TEST_VARIATION(2, 		L"SetData -> GetData -> Update")
	TEST_VARIATION(3, 		L"SetData -> GetOriginalData -> GetOriginalData")
	TEST_VARIATION(4, 		L"SetData -> GetOriginalData -> Update")
	TEST_VARIATION(5, 		L"Empty")
	TEST_VARIATION(6, 		L"SetData -> Update -> GetData -> GetData == GetOriginalData")
	TEST_VARIATION(7, 		L"SetData -> Update -> GetOriginalData -> GetOriginalData == GetData")
	TEST_VARIATION(8, 		L"Empty")
	TEST_VARIATION(9, 		L"SetData -> Undo -> GetData -> GetData == GetOriginalData")
	TEST_VARIATION(10, 		L"SetData -> Undo -> GetOriginalData -> GetOriginalData == GetData")
	TEST_VARIATION(11, 		L"Empty")
	TEST_VARIATION(12, 		L"SetData [row1] -> SetData [row1] -> Update")
	TEST_VARIATION(13, 		L"SetData [row1] -> SetData [row2] -> Update")
	TEST_VARIATION(14, 		L"SetData [row1] -> SetData [row2] -> Update [row1]")
	TEST_VARIATION(15, 		L"Empty")
	TEST_VARIATION(16, 		L"SetData [row1] -> SetData [row1] -> Undo [row1]")
	TEST_VARIATION(17, 		L"SetData [row1] -> SetData [row2] -> Undo [row1]")
	TEST_VARIATION(18, 		L"SetData [row1] -> SetData [row2] -> Undo [row2]")
	TEST_VARIATION(19, 		L"Empty")
	TEST_VARIATION(20, 		L"SetData -> SetData [NULL Storage Objects interface] -> Update")
	TEST_VARIATION(21, 		L"SetData -> SetData [NULL Storage Objects interface] -> Undo")
	TEST_VARIATION(22, 		L"Empty")
	TEST_VARIATION(23, 		L"SetData (row1")
	TEST_VARIATION(24, 		L"Empty")
	TEST_VARIATION(25, 		L"SetData (with ::Read that returns an error")
	TEST_VARIATION(26, 		L"SetData (with ::Read that returns an error")
	TEST_VARIATION(27, 		L"Empty")
	TEST_VARIATION(28, 		L"SetData -> GetData (binding as long")
	TEST_VARIATION(29, 		L"SetData -> GetOriginalData (binding as long")
	TEST_VARIATION(30, 		L"SetData -> Update -> GetData (binding as long")
	TEST_VARIATION(31, 		L"SetData -> Update -> GetOriginalData (binding as long")
	TEST_VARIATION(32, 		L"Empty")
	TEST_VARIATION(33, 		L"Insert -> GetData -> GetData")
	TEST_VARIATION(34, 		L"Insert -> GetData -> Update")
	TEST_VARIATION(35, 		L"Insert -> GetOriginalData -> GetOriginalData")
	TEST_VARIATION(36, 		L"Insert -> GetOriginalData -> Update")
	TEST_VARIATION(37, 		L"Empty")
	TEST_VARIATION(38, 		L"Insert -> Update -> GetData -> GetData")
	TEST_VARIATION(39, 		L"Insert -> Update -> GetOriginalData -> GetOrignialData")
	TEST_VARIATION(40, 		L"Insert -> Undo -> GetOriginalData -> GetOriginalData")
	TEST_VARIATION(41, 		L"Empty")
	TEST_VARIATION(42, 		L"Insert [rowX] -> Insert [rowY] -> Update [rowY]")
	TEST_VARIATION(43, 		L"Insert [rowX] -> Insert [rowY] -> Update [rowX]")
	TEST_VARIATION(44, 		L"Insert [rowX] -> Insert [rowY] -> Update all")
	TEST_VARIATION(45, 		L"Empty")
	TEST_VARIATION(46, 		L"Insert [rowX] -> Insert [rowY] -> Undo [rowY]")
	TEST_VARIATION(47, 		L"Insert [rowX] -> Insert [rowY] -> Undo [rowX]")
	TEST_VARIATION(48, 		L"Insert [rowX] -> Insert [rowY] -> Undo all")
	TEST_VARIATION(49, 		L"Empty")
	TEST_VARIATION(50, 		L"Delete -> GetOriginalData -> GetOriginalData")
	TEST_VARIATION(51, 		L"Delete -> GetOriginalData -> GetOriginalData -> Update")
	TEST_VARIATION(52, 		L"Delete -> GetOriginalData -> GetOriginalData -> Undo")
	TEST_VARIATION(53, 		L"Empty")
	TEST_VARIATION(54, 		L"Delete [row1] -> Delete [row2] -> Update")
	TEST_VARIATION(55, 		L"Delete [row1] -> Delete [row2] -> Undo")
	TEST_VARIATION(56, 		L"Delete [row1] -> Delete [row2] -> Update [row1]")
	TEST_VARIATION(57, 		L"Delete [row1] -> Delete [row2] -> Update [row2]")
	TEST_VARIATION(58, 		L"Delete [row1] -> Delete [row2] -> Undo [row1]")
	TEST_VARIATION(59, 		L"Delete [row1] -> Delete [row2] -> Undo [row2]")
	TEST_VARIATION(60, 		L"Empty")
	TEST_VARIATION(61, 		L"Sequence - SetData -> Delete -> Update")
	TEST_VARIATION(62, 		L"Sequence - SetData -> Delete -> Undo")
	TEST_VARIATION(63, 		L"Empty")
	TEST_VARIATION(64, 		L"Sequence - SetData [row1] -> Delete [row2] -> Update")
	TEST_VARIATION(65, 		L"Empty")
	TEST_VARIATION(66, 		L"Sequence - Insert -> SetData -> Update")
	TEST_VARIATION(67, 		L"Sequence - Insert -> SetData -> Undo")
	TEST_VARIATION(68, 		L"Empty")
	TEST_VARIATION(69, 		L"Sequence - Insert -> SetData -> Delete -> Update")
	TEST_VARIATION(70, 		L"Sequence - Insert -> SetData -> Delete -> Undo")
	TEST_VARIATION(71, 		L"Sequence - Insert [rowX] -> SetData [rowY] -> Delete [rowZ] -> Update all")
	TEST_VARIATION(72, 		L"Empty")
	TEST_VARIATION(73, 		L"Release - Get [row1] -> SetData -> ReleaseRows -> Update")
	TEST_VARIATION(74, 		L"Release - Get [row1] -> SetData -> ReleaseRows -> Undo")
	TEST_VARIATION(75, 		L"Empty")
	TEST_VARIATION(76, 		L"Release - Insert -> ReleaseRows -> Update")
	TEST_VARIATION(77, 		L"Release - Insert -> ReleaseRows -> Undo")
	TEST_VARIATION(78, 		L"Empty")
	TEST_VARIATION(79, 		L"Release - Get [row1] -> Delete -> ReleaseRows -> Update")
	TEST_VARIATION(80, 		L"Release - Get [row1] -> Delete -> ReleaseRows -> Undo")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCSeqStream_Transactions)
//--------------------------------------------------------------------
// @class Test the zombie states of Storage objects
//
class TCSeqStream_Transactions : public TCTransactions { 
public:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCSeqStream_Transactions,TCTransactions);
	// }} TCW_DECLARE_FUNCS_END
 
	ULONG m_cPropSets;
	DBPROPSET* m_rgPropSets;

	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Storage Objects::Read - Zombie - ABORT  with fRetaining == TRUE
	int Variation_1();
	// @cmember Storage Objects::Read - Zombie - ABORT  with fRetaining == FALSE
	int Variation_2();
	// @cmember Storage Objects::Read - Zombie - COMMIT  with fRetaining == TRUE
	int Variation_3();
	// @cmember Storage Objects::Read - Zombie - COMMIT  with fRetaining == FALSE
	int Variation_4();
	// @cmember Empty
	int Variation_5();
	// @cmember Storage Objects::Write - Zombie - ABORT  with fRetaining == TRUE
	int Variation_6();
	// @cmember Storage Objects::Write - Zombie - ABORT  with fRetaining == FALSE
	int Variation_7();
	// @cmember Storage Objects::Write - Zombie - COMMIT  with fRetaining == TRUE
	int Variation_8();
	// @cmember Storage Objects::Write - Zombie - COMMIT  with fRetaining == FALSE
	int Variation_9();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCSeqStream_Transactions)
#define THE_CLASS TCSeqStream_Transactions
BEG_TEST_CASE(TCSeqStream_Transactions, TCTransactions, L"Test the zombie states of Storage objects")
	TEST_VARIATION(1, 		L"Storage Objects::Read - Zombie - ABORT  with fRetaining == TRUE")
	TEST_VARIATION(2, 		L"Storage Objects::Read - Zombie - ABORT  with fRetaining == FALSE")
	TEST_VARIATION(3, 		L"Storage Objects::Read - Zombie - COMMIT  with fRetaining == TRUE")
	TEST_VARIATION(4, 		L"Storage Objects::Read - Zombie - COMMIT  with fRetaining == FALSE")
	TEST_VARIATION(5, 		L"Empty")
	TEST_VARIATION(6, 		L"Storage Objects::Write - Zombie - ABORT  with fRetaining == TRUE")
	TEST_VARIATION(7, 		L"Storage Objects::Write - Zombie - ABORT  with fRetaining == FALSE")
	TEST_VARIATION(8, 		L"Storage Objects::Write - Zombie - COMMIT  with fRetaining == TRUE")
	TEST_VARIATION(9, 		L"Storage Objects::Write - Zombie - COMMIT  with fRetaining == FALSE")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// }} END_DECLARE_TEST_CASES()


////////////////////////////////////////////////////////////////////////
// TCStream_General
//
////////////////////////////////////////////////////////////////////////
#define COPY_TEST_CASE(theClass, baseClass)						\
	class theClass : public baseClass							\
	{															\
	public:														\
		static const WCHAR		m_wszTestCaseName[];			\
		DECLARE_TEST_CASE_FUNCS(theClass, baseClass);			\
	};															\
const WCHAR		theClass::m_wszTestCaseName[] = { L#theClass };	\


#define TEST_CASE_WITH_PARAM(iCase, theClass, param)			\
    case iCase:													\
		pCTestCase = new theClass(NULL);						\
		((theClass*)pCTestCase)->SetTestCaseParam(param);		\
		pCTestCase->SetOwningMod(iCase-1, pCThisTestModule);	\
		return pCTestCase;


//TCStream
COPY_TEST_CASE(TCStream_General, TCSeqStream_General)
COPY_TEST_CASE(TCStream_Read, TCSeqStream_Read)
COPY_TEST_CASE(TCStream_Write, TCSeqStream_Write)
COPY_TEST_CASE(TCStream_Buffered, TCSeqStream_Buffered)
COPY_TEST_CASE(TCStream_Transactions, TCSeqStream_Transactions)

//TCLockBytes
COPY_TEST_CASE(TCLockBytes_General, TCSeqStream_General)
COPY_TEST_CASE(TCLockBytes_Read, TCSeqStream_Read)
COPY_TEST_CASE(TCLockBytes_Write, TCSeqStream_Write)
COPY_TEST_CASE(TCLockBytes_Buffered, TCSeqStream_Buffered)
COPY_TEST_CASE(TCLockBytes_Transactions, TCSeqStream_Transactions)


//NOTE: The #ifdef block below is only for test wizard.  TestWizard has too many 
//strict rules in the parsing code and requires a 1:1 correspondence between
//testcases and the map.  What the #else section is doing is basically "reusing"
//existing testcases by just passing in a paraemter which changes the behvior.
//So we make LTM think there are 15 cases in here with different names, but in
//reality we only have to maintain code for the unique cases.  This way we can
//easily get testing of other storage interfaces, without maintaining 4 different
//tests with almost identical code...

#if 0 
// {{ TCW_TESTMODULE(ThisModule)
TEST_MODULE(5, ThisModule, gwszModuleDescrip)
	TEST_CASE(1, TCSeqStream_General)
	TEST_CASE(2, TCSeqStream_Read)
	TEST_CASE(3, TCSeqStream_Write)
	TEST_CASE(4, TCSeqStream_Buffered)
	TEST_CASE(5, TCSeqStream_Transactions)
END_TEST_MODULE()
// }} TCW_TESTMODULE_END
#else
TEST_MODULE(15, ThisModule, gwszModuleDescrip)
	//IID_ISequentialStream
	TEST_CASE(1, TCSeqStream_General)
	TEST_CASE(2, TCSeqStream_Read)
	TEST_CASE(3, TCSeqStream_Write)
	TEST_CASE(4, TCSeqStream_Buffered)
	TEST_CASE(5, TCSeqStream_Transactions)

	//IID_IStream
	TEST_CASE_WITH_PARAM(6, TCStream_General, DBPROP_IStream)
	TEST_CASE_WITH_PARAM(7, TCStream_Read, DBPROP_IStream)
	TEST_CASE_WITH_PARAM(8, TCStream_Write, DBPROP_IStream)
	TEST_CASE_WITH_PARAM(9, TCStream_Buffered, DBPROP_IStream)
	TEST_CASE_WITH_PARAM(10, TCStream_Transactions, DBPROP_IStream)

	//IID_ILockBytes
	TEST_CASE_WITH_PARAM(11, TCLockBytes_General, DBPROP_ILockBytes)
	TEST_CASE_WITH_PARAM(12, TCLockBytes_Read, DBPROP_ILockBytes)
	TEST_CASE_WITH_PARAM(13, TCLockBytes_Write, DBPROP_ILockBytes)
	TEST_CASE_WITH_PARAM(14, TCLockBytes_Buffered, DBPROP_ILockBytes)
	TEST_CASE_WITH_PARAM(15, TCLockBytes_Transactions, DBPROP_ILockBytes)
END_TEST_MODULE()
#endif


// {{ TCW_TC_PROTOTYPE(TCSeqStream_General)
//*-----------------------------------------------------------------------
//| Test Case:		TCSeqStream_General - Test the ISequentialStream supported
//|	Created:			08/14/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCSeqStream_General::Init()
{
	TBEGIN

	// {{ TCW_INIT_BASECLASS_CHECK
	TESTC(TCStorage::Init());
	// }}
 	
	//Determine if this Storage interface is supported...
	TESTC_PROVIDER(SupportedStorageInterface());

CLEANUP:
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_General::Variation_1()
{
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Properties - DBPROP_DELAYSTORAGEOBJECTS
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_General::Variation_2()
{
	ULONG dwFlags;
	
	//DBPROP_DELAYSTORAGEOBJECTS
	//Really should support this property for consumers if your support storage 
	//objects, this way they can determine one way or the other the functionality
	TESTC_PROVIDER(SupportedProperty(DBPROP_DELAYSTORAGEOBJECTS, DBPROPSET_ROWSET));

	//If supported must be Readable
	dwFlags = GetPropInfoFlags(DBPROP_DELAYSTORAGEOBJECTS, DBPROPSET_ROWSET);
	TESTC(BITSET(dwFlags, DBPROPFLAGS_READ))
	
	//Settable is allowed, but really quite odd for a provider to allow the consumer
	//to change this major functionaltiy at runtime...
	TESTW(BITCLEAR(dwFlags, DBPROPFLAGS_WRITE));

CLEANUP:
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Properties - DBPROP_TRANSACTEDOBJECT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_General::Variation_3()
{
	ULONG dwFlags;
	
	//DBPROP_TRANSACTEDOBJECT
	//Really should support this property for consumers if your support storage 
	//objects, this way they can determine one way or the other the functionality
	TESTC_PROVIDER(SupportedProperty(DBPROP_TRANSACTEDOBJECT, DBPROPSET_ROWSET));

	//If supported must be Readable
	dwFlags = GetPropInfoFlags(DBPROP_TRANSACTEDOBJECT, DBPROPSET_ROWSET);
	TESTC(BITSET(dwFlags, DBPROPFLAGS_READ))
	
	//Settable is allowed, but really quite odd for a provider to allow the consumer
	//to change this major functionaltiy at runtime...
	TESTW(BITCLEAR(dwFlags, DBPROPFLAGS_WRITE));

CLEANUP:
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Properties - DBPROP_MULTIPLESTORAGEOBJECTS
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_General::Variation_4()
{
	ULONG dwFlags;
	
	//DBPROP_MULTIPLESTORAGEOBJECTS
	//Really should support this property for consumers if your support storage 
	//objects, this way they can determine one way or the other the functionality
	TESTC_PROVIDER(SupportedProperty(DBPROP_MULTIPLESTORAGEOBJECTS, DBPROPSET_DATASOURCEINFO));

	//If supported must be Readable
	dwFlags = GetPropInfoFlags(DBPROP_MULTIPLESTORAGEOBJECTS, DBPROPSET_DATASOURCEINFO);
	TESTC(BITSET(dwFlags, DBPROPFLAGS_READ))
	
	//Settable is allowed, but really quite odd for a provider to allow the consumer
	//to change this major functionaltiy at runtime...
	TESTW(BITCLEAR(dwFlags, DBPROPFLAGS_WRITE));

CLEANUP:
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Properties - DBPROP_BLOCKINGSTORAGEOBJECTS
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_General::Variation_5()
{											  
	//Verify Read-Only
	ULONG dwFlags;
	
	//DBPROP_BLOCKINGSTORAGEOBJECTS
	//Really should support this property for consumers if your support storage 
	//objects, this way they can determine one way or the other the functionality
	TESTC_PROVIDER(SupportedProperty(DBPROP_BLOCKINGSTORAGEOBJECTS, DBPROPSET_ROWSET));

	//If supported must be Readable
	dwFlags = GetPropInfoFlags(DBPROP_BLOCKINGSTORAGEOBJECTS, DBPROPSET_ROWSET);
	TESTC(BITSET(dwFlags, DBPROPFLAGS_READ))
	
	//Settable is allowed, but really quite odd for a provider to allow the consumer
	//to change this major functionaltiy at runtime...
	TESTW(BITCLEAR(dwFlags, DBPROPFLAGS_WRITE));

CLEANUP:
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Properties - DBPROP_STRUCTUREDSTORAGE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_General::Variation_6()
{
	//Verify Read-Only
	ULONG dwFlags;
	ULONG_PTR ulValue = 0;
	
	//DBPROP_STRUCTUREDSTORAGE
	//Really should support this property for consumers if your support storage 
	//objects, this way they can determine one way or the other the functionality
	TESTC(::GetProperty(DBPROP_STRUCTUREDSTORAGE, DBPROPSET_DATASOURCEINFO, g_pIDBInitialize, &ulValue))

	//If supported must be Readable
	dwFlags = GetPropInfoFlags(DBPROP_STRUCTUREDSTORAGE, DBPROPSET_DATASOURCEINFO);
	TESTC(BITSET(dwFlags, DBPROPFLAGS_READ))
	
	//Settable is allowed, but really quite odd for a provider to allow the consumer
	//to change this major functionaltiy at runtime...
	TESTW(BITCLEAR(dwFlags, DBPROPFLAGS_WRITE));

	//Should be at least one of the valid interfaces (DBPROPVAL_SS_*)
	//As ISequentialStream is required by all providers if storage objects are supported.
	TESTC(ulValue && (ulValue & DBPROPVAL_SS_ISEQUENTIALSTREAM));

CLEANUP:
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Properties - DBPROP_OLEOBJECTS
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_General::Variation_7()
{
	ULONG dwFlags;
	ULONG_PTR ulValue = 0;

	//Provider Should support OLEOBJECTS, if we are this far
	//Really should support this property for consumers if your support storage 
	//objects, this way they can determine one way or the other the functionality
	TESTC(::GetProperty(DBPROP_OLEOBJECTS, DBPROPSET_DATASOURCEINFO, g_pIDBInitialize, &ulValue))
		
	//If supported must be Readable
	dwFlags = GetPropInfoFlags(DBPROP_OLEOBJECTS, DBPROPSET_DATASOURCEINFO);
	TESTC(BITSET(dwFlags, DBPROPFLAGS_READ))
	
	//Settable is allowed, but really quite odd for a provider to allow the consumer
	//to change this major functionaltiy at runtime...
	TESTW(BITCLEAR(dwFlags, DBPROPFLAGS_WRITE));

	//Supports access to BLOBS as structured storage objects
	TESTC(ulValue && ulValue & DBPROPVAL_OO_BLOB);

CLEANUP:
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_General::Variation_8()
{
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc Properties - Storage Property ie: DBPROP_ISequentialStream
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_General::Variation_9()
{
	HRESULT hr = S_OK;
	CRowset RowsetA;
	ULONG dwFlags;

	void* pData = NULL;
	HROW hRow = NULL;
	HACCESSOR hAccessor = NULL;

	//DBPROP_ISequentialStream must be supported if we in here
	TESTC(SupportedProperty(m_dwStorageID, DBPROPSET_ROWSET))
	
	//Get dwFlags
	dwFlags = GetPropInfoFlags(m_dwStorageID, DBPROPSET_ROWSET);

	//Try and set property for all columns (DBID_NULL), shouldn't be able to
	hr = RowsetA.CreateRowset(m_dwStorageID);
	TESTC(hr == S_OK || hr == DB_E_ERRORSOCCURRED);

	if(!(dwFlags & DBPROPFLAGS_COLUMNOK))
	{
		//DBPROPFLAGS_COLUMNOK indicates that this property is allowed on all columns
		//By not having this bit set, the provider ignores the colid value of the 
		//property and is only able to retrive BLOB columns whose conversion is 
		//supported (which is what the rest of the test does...
		TESTC_(hr, S_OK);
		VerifyPropStatus(RowsetA.m_cPropSets, RowsetA.m_rgPropSets, m_dwStorageID, DBPROPSET_ROWSET, DBPROPSTATUS_OK);
	}
	else if(hr == DB_E_ERRORSOCCURRED)
	{
		//NOTSETTABLE
		if(!(dwFlags & DBPROPFLAGS_WRITE))
			VerifyPropStatus(RowsetA.m_cPropSets, RowsetA.m_rgPropSets, m_dwStorageID, DBPROPSET_ROWSET, DBPROPSTATUS_NOTSETTABLE);
		//BADCOLUMN
		else
			VerifyPropStatus(RowsetA.m_cPropSets, RowsetA.m_rgPropSets, m_dwStorageID, DBPROPSET_ROWSET, DBPROPSTATUS_BADCOLUMN);
	}
	else
	{
		//Grab a row handle
		TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK)
		
		//Alloc buffers
		SAFE_ALLOC(pData, BYTE, RowsetA.m_cRowSize*sizeof(void*));

		//If we get here, this means the Property DBPROP_ISequentialStream
		//Could be set on all columns!  This means the provider must be able 
		//To GetData for all columns bound as ISequentialStream...

		//Create An Accessor Binding all columns as Storage Objects
		//BLOB_BIND_ALL_COLS accoplishes this.  Otheriwse BLOB_BIND_ALL_BLOBS
		//Only binds all BLOB (ISLONG) columns as BLOBS...
		for(ULONG i=0; i<RowsetA.m_cRowsetCols; i++)
		{
			//Actually since most providers only allow 1 blob column bound
			//per accessor, DBPROP_MULITPLSTORAGEOBJECTS, we will only create a seperate
			//Accessor per BLOB column
			TESTC_(GetAccessorAndBindings(RowsetA.pIAccessor(), DBACCESSOR_ROWDATA, &hAccessor,
				NULL, 0, NULL, DBPART_ALL, USE_COLS_TO_BIND_ARRAY, FORWARD,
				NO_COLS_BY_REF,NULL,NULL,NULL,DBTYPE_EMPTY,1,&RowsetA.m_rgTableColOrds[i],NULL,
				NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, m_dwFlags | BLOB_BIND_ALL_COLS),S_OK)

			//Get the Data, ISequentialStream - should actually succeed
			TESTC_(RowsetA.GetStorageData(hRow, hAccessor),S_OK);

			//ReleaseAccessor
			RowsetA.ReleaseAccessor(hAccessor);
		}
	}

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor);
	PROVIDER_FREE(pData);
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSeqStream_General::Variation_10()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc Boundary - Seek
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSeqStream_General::Variation_11()
{ 
	TBEGIN
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	HROW  hRow = NULL;

	ULONG cbOriginal = 0;
	ULONG cBytes = 0;
	ULONG ulSeekPos = 0;
	IStream* pIStream = NULL;
	CRowset RowsetA;

	//This variation requires IStream::Seek, skip if not supported...
	TESTC_PROVIDER(m_riidStorage==IID_IStream);

	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);
		
	//Grab a row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK)
	
	//Create Accessor binding BLOB/Storage Object data
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, NULL, NULL, NULL, m_dwFlags), S_OK)

	//Obtain the Storage interface
	TESTC_(RowsetA.GetStorageData(hRow, hAccessor, NULL, NULL, m_riidStorage, (IUnknown**)&pIStream),S_OK)

	//NOTE:  In the following code we call IStream::Read directly since we are
	//testing IStream::Seek.  Our helper function StorageRead will automatically
	//do a seek to the indicated offset, which works great, but not in the senario
	//where we are already testing the seek position...

	//Now read all of the data...
	TESTC_(StorageSeek(pIStream, 0, STREAM_SEEK_SET, &ulSeekPos),S_OK);
	TEST2C_(pIStream->Read(m_pBuffer, ULONG_MAX, &cbOriginal),S_OK,S_FALSE);
	TESTC(cbOriginal > 0 && cbOriginal < ULONG_MAX && ulSeekPos == 0);

	//Seek to the begining
	TESTC_(StorageSeek(pIStream, 10, STREAM_SEEK_SET, &ulSeekPos), S_OK);
	TESTC_(pIStream->Read(m_pBuffer, 20, &cBytes),S_OK);
	TESTC(cBytes == 20 && ulSeekPos == 10);

	//Seek to the end
	TESTC_(StorageSeek(pIStream, 0, STREAM_SEEK_END, &ulSeekPos),S_OK);
	TEST2C_(pIStream->Read(m_pBuffer, ULONG_MAX, &cBytes),S_OK,S_FALSE);
	TESTC(cBytes == 0 && ulSeekPos == cbOriginal);

	//Seek to the middle
	TESTC_(StorageSeek(pIStream, cbOriginal/2, STREAM_SEEK_SET, &ulSeekPos),S_OK);
	TESTC_(pIStream->Read(m_pBuffer, cbOriginal-cbOriginal/2, &cBytes),S_OK);
	TESTC(cBytes == cbOriginal-cbOriginal/2 && ulSeekPos == cbOriginal/2);

	//Seek before the stream - Error
	//It is an error to seek before the stream - (what error?)
	TESTC_(StorageSeek(pIStream, NEGATIVE(cbOriginal+1), STREAM_SEEK_CUR, &ulSeekPos), STG_E_INVALIDFUNCTION);

	//Make sure the seek pointer is unchanged...
	//To obtain the current seek position, just use the CurrentPos+0...
	TESTC_(StorageSeek(pIStream, 0, STREAM_SEEK_CUR, &ulSeekPos), S_OK);
	TESTC(ulSeekPos == cbOriginal)

	//Seek past the end of the stream - Allowed
	TESTC_(StorageSeek(pIStream, cbOriginal+10, STREAM_SEEK_SET, &ulSeekPos), S_OK);
	TESTC(ulSeekPos == cbOriginal+10);

	//Seek from current position
	TESTC_(StorageSeek(pIStream, 11, STREAM_SEEK_SET, &ulSeekPos),S_OK);
	TESTC_(StorageSeek(pIStream, 31, STREAM_SEEK_CUR, &ulSeekPos),S_OK);
	TEST2C_(pIStream->Read(m_pBuffer, cbOriginal, &cBytes),S_OK,S_FALSE);
	TESTC_(StorageSeek(pIStream, 0, STREAM_SEEK_CUR, &ulSeekPos), S_OK);
	TESTC(cBytes == cbOriginal-11-31 && ulSeekPos == cbOriginal);

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor);
	SAFE_RELEASE_(pIStream);
	TRETURN;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc Boundary - SetSize
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSeqStream_General::Variation_12()
{ 
	TBEGIN
	HACCESSOR hReadAccessor = DB_NULL_HACCESSOR;
	HACCESSOR hRWAccessor = DB_NULL_HACCESSOR;
	HROW  hRow = NULL;
	HRESULT hr = S_OK;

	DBLENGTH cbOriginal = 0;
	DBLENGTH cBytes = 0;
	IUnknown* pIUnknown = NULL;
	CRowset RowsetA;

	//This variation requires IStream::Seek or ILockBytes::Seek, skip if not supported...
	TESTC_PROVIDER(m_riidStorage==IID_IStream || m_riidStorage==IID_ILockBytes);

	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);
		
	//Grab a row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK)
	
	//Create Accessor binding BLOB/Storage Object data
	TESTC_(RowsetA.CreateAccessor(&hReadAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, NULL, NULL, NULL, m_dwFlags | BLOB_STGM_READ), S_OK)
	
	//May require STGM_WRITE cababilities since the stream shouldn't 
	//really need extra overhead to grow unless ::Write can be called...
	TEST2C_(hr = RowsetA.CreateAccessor(&hRWAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, NULL, NULL, NULL, m_dwFlags | BLOB_STGM_READWRITE), S_OK, DB_E_ERRORSOCCURRED)
	if(FAILED(hr))
		TESTC_(RowsetA.CreateAccessor(&hRWAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, NULL, NULL, NULL, m_dwFlags | BLOB_STGM_READ), S_OK)

	//Obtain the Storage interface
	TESTC_(RowsetA.GetStorageData(hRow, hRWAccessor, NULL, NULL, m_riidStorage, (IUnknown**)&pIUnknown),S_OK)
	TEST2C_(StorageRead(pIUnknown, m_pBuffer, ULONG_MAX, &cbOriginal),S_OK,S_FALSE);
	TESTC(cbOriginal > 0 && cbOriginal < ULONG_MAX);

	//SetSize, (a little larger)
	//Some streams may not be extendable
	TEST2C_(hr = StorageSetSize(pIUnknown, cbOriginal+100), S_OK, STG_E_MEDIUMFULL);
	TESTW_(hr, S_OK);

	if(SUCCEEDED(hr))
	{
		//Make sure we can read the new buffer length, the data as defined by COM
		//is undefined bytes, but at least there is space avialble...
		TEST2C_(StorageRead(pIUnknown, m_pBuffer, 101, &cBytes, cbOriginal),S_OK,S_FALSE);
		TESTC(cBytes == 100);

		//Read from the begining again
		//NOTE:  this testcase is for Stream and ILockBytes both with have the ability
		//to reposition in the stream...
		TEST2C_(StorageRead(pIUnknown, m_pBuffer, cbOriginal+100, &cBytes, 0),S_OK,S_FALSE);
		TESTC(cBytes == cbOriginal+100);
	}

	//Reduce the size of the stream...
	TEST2C_(hr = StorageSetSize(pIUnknown, cbOriginal-10), S_OK, STG_E_MEDIUMFULL);
	TESTW_(hr, S_OK);

	if(SUCCEEDED(hr))
	{
		//Read from the begining again (smaller stream now...)
		TEST2C_(StorageRead(pIUnknown, m_pBuffer, cbOriginal, &cBytes, 0),S_OK,S_FALSE);
		TESTC(cBytes == cbOriginal-10);

		//Read from the end of the stream
		TEST2C_(StorageRead(pIUnknown, m_pBuffer, 10, &cBytes, cbOriginal-10),S_OK,S_FALSE);
		TESTC(cBytes == 0);
	}

	//See if we can change the size with only STGM_READ specified...
	SAFE_RELEASE_(pIUnknown);
	RowsetA.ReleaseRows(hRow);
	TESTC_(RowsetA.GetNextRows(&hRow),S_OK);

	//Obtain the Storage interface
	TESTC_(RowsetA.GetStorageData(hRow, hReadAccessor, NULL, NULL, m_riidStorage, (IUnknown**)&pIUnknown),S_OK)

	//SetSize, (a little larger)
	TEST2C_(StorageSetSize(pIUnknown, cbOriginal+100), S_OK, STG_E_MEDIUMFULL);

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hReadAccessor);
	RowsetA.ReleaseAccessor(hRWAccessor);
	SAFE_RELEASE_(pIUnknown);
	TRETURN;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc Boundary - CopyTo
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSeqStream_General::Variation_13()
{ 
	TBEGIN
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	HROW  hRow = NULL;

	ULONG cbRead = 0;
	ULONG cbWritten = 0;

	IStream* pIStream = NULL;
	CStorage* pCStorage = new CStorage;
	CRowset RowsetA;

	//This variation requires IStream::CopyTo
	TESTC_PROVIDER(m_riidStorage==IID_IStream);

	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);
		
	//Grab a row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK)
	
	//Create Accessor binding BLOB/Storage Object data
	//May require STGM_WRITE cababilities since the stream shouldn't 
	//really need extra overhead to grow unless ::Write can be called...
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, NULL, NULL, NULL, m_dwFlags), S_OK)

	//Obtain the Storage interface
	TESTC_(RowsetA.GetStorageData(hRow, hAccessor, NULL, NULL, m_riidStorage, (IUnknown**)&pIStream),S_OK)

	//CopyTo our own stream...
	TESTC_(StorageCopyTo(pIStream, (IStream*)pCStorage, ULONG_MAX, &cbRead, &cbWritten),S_OK);
	TESTC(cbRead == cbWritten);

	//Read from both streams...
	TESTC_(StorageRead(pIStream, m_pBuffer, cbRead),S_OK);
	TESTC_(StorageRead(pCStorage->pUnknown(), m_pBuffer2, cbRead),S_OK);
	TESTC(memcmp(m_pBuffer, m_pBuffer2, cbRead)==0);

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor);
	SAFE_RELEASE_(pCStorage);
	TRETURN;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc Boundary - Commit
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSeqStream_General::Variation_14()
{ 
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	HROW  hRow = NULL;

	ULONG cbOriginal = DATA_SIZE;
	IStream* pIStream = NULL;
	CRowset RowsetA;

	//This variation requires IStream::Commit
	TESTC_PROVIDER(m_riidStorage==IID_IStream);

	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);
		
	//Grab a row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK)
	
	//Create Accessor binding BLOB/Storage Object data
	//May require STGM_WRITE cababilities since the stream shouldn't 
	//really need extra overhead to grow unless ::Write can be called...
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, NULL, NULL, NULL, m_dwFlags), S_OK)

	//Obtain the Storage interface
	TESTC_(RowsetA.GetStorageData(hRow, hAccessor, m_pBuffer, &cbOriginal, m_riidStorage, (IUnknown**)&pIStream),S_OK)

	//Commit the stream
	TESTC_(pIStream->Commit(STGC_DEFAULT),S_OK);

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor);
	SAFE_RELEASE(pIStream);
	TRETURN;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc Boundary - Revert
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSeqStream_General::Variation_15()
{ 
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	HROW  hRow = NULL;

	ULONG cbOriginal = DATA_SIZE;
	IStream* pIStream = NULL;
	CRowset RowsetA;

	//This variation requires IStream::Commit
	TESTC_PROVIDER(m_riidStorage==IID_IStream);

	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);
		
	//Grab a row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK)
	
	//Create Accessor binding BLOB/Storage Object data
	//May require STGM_WRITE cababilities since the stream shouldn't 
	//really need extra overhead to grow unless ::Write can be called...
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, NULL, NULL, NULL, m_dwFlags), S_OK)

	//Obtain the Storage interface
	TESTC_(RowsetA.GetStorageData(hRow, hAccessor, m_pBuffer, &cbOriginal, m_riidStorage, (IUnknown**)&pIStream),S_OK)

	//Commit the stream
	TESTC_(pIStream->Revert(),S_OK);

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor);
	SAFE_RELEASE(pIStream);
	TRETURN;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc Boundary - LockRegion
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSeqStream_General::Variation_16()
{ 
	TBEGIN
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	HROW  hRow = NULL;
	ULONG cbOriginal = DATA_SIZE;
	IUnknown* pIUnknown = NULL;
	CRowset RowsetA;

	//This variation requires IStream or ILockBytes, skip if not supported...
	TESTC_PROVIDER(m_riidStorage==IID_IStream || m_riidStorage==IID_ILockBytes);

	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);
		
	//Grab a row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK)
	
	//Create Accessor binding BLOB/Storage Object data
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, NULL, NULL, NULL, m_dwFlags),S_OK);

	//Obtain the Storage interface
	TESTC_(RowsetA.GetStorageData(hRow, hAccessor, m_pBuffer, &cbOriginal, m_riidStorage, &pIUnknown),S_OK);

	//LockRegion...
	//Doesn't have to be supported according to OLE
	TEST2C_(StorageLockRegion(pIUnknown, 0, cbOriginal, LOCK_WRITE),S_OK,STG_E_INVALIDFUNCTION);

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor);
	SAFE_RELEASE_(pIUnknown);
	TRETURN;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc Boundary - UnlockRegion
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSeqStream_General::Variation_17()
{ 
	TBEGIN
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	HROW  hRow = NULL;
	ULONG cbOriginal = DATA_SIZE;
	IUnknown* pIUnknown = NULL;
	CRowset RowsetA;

	//This variation requires IStream or ILockBytes, skip if not supported...
	TESTC_PROVIDER(m_riidStorage==IID_IStream || m_riidStorage==IID_ILockBytes);

	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);
		
	//Grab a row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK)
	
	//Create Accessor binding BLOB/Storage Object data
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, NULL, NULL, NULL, m_dwFlags),S_OK);

	//Obtain the Storage interface
	TESTC_(RowsetA.GetStorageData(hRow, hAccessor, m_pBuffer, &cbOriginal, m_riidStorage, &pIUnknown),S_OK);

	//UnlockRegion...
	//Doesn't have to be supported according to OLE
	TEST2C_(StorageUnlockRegion(pIUnknown, 0, cbOriginal, LOCK_WRITE),S_OK,STG_E_INVALIDFUNCTION);

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor);
	SAFE_RELEASE_(pIUnknown);
	TRETURN;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc Boundary - Stat
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSeqStream_General::Variation_18()
{ 
	TBEGIN
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	HROW  hRow = NULL;
	ULONG cbOriginal = DATA_SIZE;
	IUnknown* pIUnknown = NULL;
	STATSTG stagstg = { 0 };
	CRowset RowsetA;

	//This variation requires IStream or ILockBytes, skip if not supported...
	TESTC_PROVIDER(m_riidStorage==IID_IStream || m_riidStorage==IID_ILockBytes);

	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);
		
	//Grab a row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK)
	
	//Create Accessor binding BLOB/Storage Object data
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, NULL, NULL, NULL, m_dwFlags),S_OK);

	//Obtain the Storage interface
	TESTC_(RowsetA.GetStorageData(hRow, hAccessor, m_pBuffer, &cbOriginal, m_riidStorage, &pIUnknown),S_OK)

	//Obtain the status for this stream...
	TESTC_(StorageStat(pIUnknown, &stagstg, STATFLAG_NONAME),S_OK);
	TESTC(stagstg.pwcsName == NULL);

	//Verify some of the fields...
	TESTC_(StorageStat(pIUnknown, &stagstg, STATFLAG_DEFAULT),S_OK);
	//Name	
	TOUTPUT(L"Storage pwcsName = \"" << stagstg.pwcsName << "\"");
	SAFE_FREE(stagstg.pwcsName);

	//Type
	if(m_riidStorage == IID_ILockBytes)
	{
		TESTC(stagstg.type == STGTY_LOCKBYTES);
	}
	else
	{
		TESTC(stagstg.type == STGTY_STREAM);
	}

	//cbSize
	TESTC((ULONG)stagstg.cbSize.QuadPart == cbOriginal);
	
	//grfMode
	//But STGM_READ == 0, so every other bit includes this then...
	// TESTC(stagstg.grfMode >= 0 /*& STGM_READ*/);  - always true
	
	//grfLocksSupported
	TESTC(stagstg.grfLocksSupported == 0 || stagstg.grfLocksSupported & (LOCK_WRITE | LOCK_EXCLUSIVE | LOCK_ONLYONCE));

	//Invalid Argument Testing
	TESTC_(StorageStat(pIUnknown, &stagstg, 2),STG_E_INVALIDFLAG);
	TESTC_(StorageStat(pIUnknown, &stagstg, ULONG_MAX),STG_E_INVALIDFLAG);


CLEANUP:
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor);
	SAFE_RELEASE_(pIUnknown);
	TRETURN;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc Boundary - Clone
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSeqStream_General::Variation_19()
{ 
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	HROW  hRow = NULL;

	ULONG cBytes = 0;
	ULONG cbOriginal = DATA_SIZE;

	IStream* pIStream = NULL;
	IStream* pIStreamClone = NULL;
	ULONG ulSeekPos, ulSeekPos2 = 0;
	CRowset RowsetA;

	//This variation requires IStream::CopyTo
	TESTC_PROVIDER(m_riidStorage==IID_IStream);

	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);
		
	//Grab a row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK)
	
	//Create Accessor binding BLOB/Storage Object data
	//May require STGM_WRITE cababilities since the stream shouldn't 
	//really need extra overhead to grow unless ::Write can be called...
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, NULL, NULL, NULL, m_dwFlags), S_OK)

	//Obtain the Storage interface
	TESTC_(RowsetA.GetStorageData(hRow, hAccessor, m_pBuffer, &cbOriginal, m_riidStorage, (IUnknown**)&pIStream),S_OK)

	//Clone the stream
	//It returns a new stream object, but with its own unique seek index...
	TESTC_(pIStream->Clone(&pIStreamClone),S_OK);

	//Read from both streams...
	TESTC_(StorageRead(pIStream, m_pBuffer, cbOriginal),S_OK);
	TESTC_(StorageRead(pIStreamClone, m_pBuffer2, cbOriginal),S_OK);
	TESTC(memcmp(m_pBuffer, m_pBuffer2, cbOriginal)==0);

	//Make sure they have their own seek pointers...
	TESTC_(StorageSeek(pIStream, 10, STREAM_SEEK_SET, &ulSeekPos),S_OK);
	TESTC_(StorageSeek(pIStreamClone, 20, STREAM_SEEK_SET, &ulSeekPos2),S_OK);
	TESTC(ulSeekPos == 10 && ulSeekPos2 == 20);
	
	//Continue to make sure...
	TEST2C_(pIStream->Read(m_pBuffer, cbOriginal, &cBytes),S_OK,S_FALSE);
	TESTC(cBytes == cbOriginal-10);
	TEST2C_(pIStreamClone->Read(m_pBuffer2, cbOriginal, &cBytes),S_OK,S_FALSE);
	TESTC(cBytes == cbOriginal-20);

	//TODO:  Need to test Write is seen by the other stream, since they
	//are over the same dataset...

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor);
	SAFE_RELEASE(pIStream);
	SAFE_RELEASE(pIStreamClone);
	TRETURN;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc Boundary - ILockBytes::Flush
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSeqStream_General::Variation_20()
{ 
	TBEGIN
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	HROW  hRow = NULL;
	ULONG cbOriginal = DATA_SIZE;
	ILockBytes* pILockBytes = NULL;
	CRowset RowsetA;

	//This variation requires IStream, skip if not supported...
	TESTC_PROVIDER(m_riidStorage==IID_ILockBytes);

	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);
		
	//Grab a row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK)
	
	//Create Accessor binding BLOB/Storage Object data
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, NULL, NULL, NULL, m_dwFlags),S_OK);

	//Obtain the Storage interface
	TESTC_(RowsetA.GetStorageData(hRow, hAccessor, m_pBuffer, &cbOriginal, m_riidStorage, (IUnknown**)&pILockBytes),S_OK)

	//Flush...
	TESTC_(pILockBytes->Flush(),S_OK);

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor);
	SAFE_RELEASE_(pILockBytes);
	TRETURN;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCSeqStream_General::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCStorage::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCSeqStream_Read)
//*-----------------------------------------------------------------------
//| Test Case:		TCSeqStream_Read - Test Storage Objects with GetData functionality [Read]
//|	Created:			08/21/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCSeqStream_Read::Init()
{
	TBEGIN

	// {{ TCW_INIT_BASECLASS_CHECK
	TESTC(TCStorage::Init());
	// }}
 	
	//Determine if this Storage interface is supported...
	TESTC_PROVIDER(SupportedStorageInterface());

CLEANUP:
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc General - Verify RefCount of retrived Storage Objects interface
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Read::Variation_1()
{
	TBEGIN
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	HROW  hRow = NULL;
	IUnknown* pIUnknown = NULL;
	
	CRowset RowsetA;
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);
	
	//Grab a row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK)

	//Create Accessor binding BLOB/Storage Object data
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, NULL, NULL, NULL, m_dwFlags), S_OK)
	
	//Obtain the Storage interface
	TESTC_(RowsetA.GetStorageData(hRow, hAccessor, NULL, NULL, m_riidStorage, (IUnknown**)&pIUnknown),S_OK)

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor);
	SAFE_RELEASE_(pIUnknown);
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Read::Variation_2()
{
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc General - 1 - Specify IRowsetLocate
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Read::Variation_3()
{
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc General - 2 - Specify Exetened Fetch / no CANHOLDROWS / AnyOrder / AnyColumns, OWNINSERT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Read::Variation_4()
{
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc General - 3 - DBPROP_ACCESSORDER - DBPROPVAL_AO_RANDOM
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Read::Variation_5()
{
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	DBCOUNTITEM i,cRowsObtained = 0;
	HROW* rghRows = NULL;
	DBORDINAL cBindings = 0;
	DBBINDING* rgBindings = NULL;
	void* pData = NULL;
	DBLENGTH cBytes = 0;
	IUnknown* pIUnknown = NULL;
	
	CRowset RowsetA;
	RowsetA.SetSettableProperty(DBPROP_ACCESSORDER, DBPROPSET_ROWSET, (void*)DBPROPVAL_AO_RANDOM, DBTYPE_I4);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);
	
	//Create Accessor binding BLOB/Storage Object data
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, &cBindings, &rgBindings, &cBytes, m_dwFlags), S_OK)
	SAFE_ALLOC(pData, void*, cBytes);
	
	//Obtain all the rows at once, and verify
	//This tests the senario of being able to "see" BLOBs are the beyond 
	//the current fetch position, meaning the provider may have to "cache" the entire BLOB
	//in order to obtain th value (ie: meaning of DBPROP_ACCESSORDER)
	TESTC_(RowsetA.GetNextRows(0, RowsetA.m_ulTableRows, &cRowsObtained, &rghRows),S_OK)

	//Obtain the Storage interface
	//Make sure we can obtain all storag eobjects for all rows, even those already fetched...
	for(i=0; i<cRowsObtained; i++)
	{
		//Obtain the Storage Object
		TESTC_(RowsetA.GetData(rghRows[i], hAccessor, pData),S_OK)
		TESTC(GetStorageObject(cBindings, rgBindings, pData, m_riidStorage, (IUnknown**)&pIUnknown))

		//Verify Storage Object matches backend
		TESTC(RowsetA.CompareTableData(FIRST_ROW+i/*iRow*/, pData, cBindings, rgBindings));
		SAFE_RELEASE(pIUnknown);
	}

CLEANUP:
	RowsetA.ReleaseRows(cRowsObtained, rghRows);
	RowsetA.ReleaseAccessor(hAccessor, cBindings, rgBindings, pData);
	SAFE_RELEASE(pIUnknown);
	SAFE_FREE(rghRows);
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc General - 4 - DBPROP_ACCESSORDER - DBPROPVAL_AO_SEQUENTIALSTORAGEOBJECTS
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Read::Variation_6()
{
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	HROW  hRow = NULL;
	DBORDINAL i,cBindings = 0;
	DBBINDING* rgBindings = NULL;
	void* pData = NULL;
	DBLENGTH cBytes = 0;
	IUnknown* pIUnknown = NULL;
	
	//NOTE: DBPROPVAL_AO_SEQUENTIAL is covered in the IRowset test...
	//In this test we are interested only in storage objects...
	CRowset RowsetA;
	RowsetA.SetSettableProperty(DBPROP_ACCESSORDER, DBPROPSET_ROWSET, (void*)DBPROPVAL_AO_SEQUENTIALSTORAGEOBJECTS, DBTYPE_I4);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);
	
	//Create Accessor binding BLOB/Storage Object data
	//NOTE: With SEQUENTIALSTORAGEOBJECTS (accoridng to the spec) you cannot bind other columns
	//after the storage object (otherwise UNAVAILABLE) may be returned for all columns after including
	//the storage object column iteself!  So we pass in BLOB_BIND_FORWARDONLY to indciate
	//to privlib to not bind other columns after the storage object...
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, &cBindings, &rgBindings, &cBytes, m_dwFlags | BLOB_BIND_FORWARDONLY), S_OK)
	SAFE_ALLOC(pData, void*, cBytes);
	
	//Obtain all the rows (sequentially)
	for(i=0; i<RowsetA.m_ulTableRows; i++)
	{
		TESTC_(RowsetA.GetNextRows(&hRow),S_OK)

		//Obtain the Storage Object
		TESTC_(RowsetA.GetData(hRow, hAccessor, pData),S_OK)
		TESTC(GetStorageObject(cBindings, rgBindings, pData, m_riidStorage, (IUnknown**)&pIUnknown))

		//Verify Storage Object matches backend
		TESTC(RowsetA.CompareTableData(FIRST_ROW+i/*iRow*/, pData, cBindings, rgBindings));
	
		SAFE_RELEASE(pIUnknown);
		RowsetA.ReleaseRows(hRow);
	}

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor, cBindings, rgBindings, pData);
	SAFE_RELEASE(pIUnknown);
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc General - 4 - DBPROP_ACCESSORDER - DBPROPVAL_AO_SEQUENTIALSTORAGEOBJECTS - retrieve as if was random
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Read::Variation_7()
{
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	DBCOUNTITEM iRow,cRowsObtained = 0;
	HROW* rghRows = NULL;
	DBORDINAL cBindings = 0;
	DBBINDING* rgBindings = NULL;
	void* pData = NULL;
	DBLENGTH cBytes = 0;
	IUnknown* pIUnknown = NULL;
	HRESULT hr = S_OK;
	
	CRowset RowsetA;
	RowsetA.SetSettableProperty(DBPROP_ACCESSORDER, DBPROPSET_ROWSET, (void*)DBPROPVAL_AO_SEQUENTIALSTORAGEOBJECTS, DBTYPE_I4);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);
	
	//Create Accessor binding BLOB/Storage Object data
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, &cBindings, &rgBindings, &cBytes, m_dwFlags), S_OK)
	SAFE_ALLOC(pData, void*, cBytes);
	
	//Obtain all the rows at once, and verify
	//This tests the senario of being able to "see" BLOBs are the beyond 
	//the current fetch position, meaning the provider may have to "cache" the entire BLOB
	//in order to obtain th value (ie: meaning of DBPROP_ACCESSORDER)
	TESTC_(RowsetA.GetNextRows(0, RowsetA.m_ulTableRows, &cRowsObtained, &rghRows),S_OK)

	//Obtain the Storage interface
	//Make sure we can obtain all storag eobjects for all rows, even those already fetched...
	for(iRow=0; iRow<cRowsObtained; iRow++)
	{
		//Obtain the Storage Object
		TEST3C_(hr = RowsetA.GetData(rghRows[iRow], hAccessor, pData),S_OK,DB_S_ERRORSOCCURRED,DB_E_ERRORSOCCURRED);

		if(hr == S_OK)
		{
			//Verify Storage Object matches backend
			TESTC(GetStorageObject(cBindings, rgBindings, pData, m_riidStorage, (IUnknown**)&pIUnknown));
			if(!RowsetA.CompareTableData(FIRST_ROW+iRow/*iRow*/, pData, cBindings, rgBindings))
			{
				//Data incorrect for this row!
				TERROR("Data was incorrect for row " << FIRST_ROW+iRow);
				QTESTC(FALSE);
			}
		}
		else
		{
			//Verify the errors...
			for(ULONG iBinding=0; iBinding<cBindings; iBinding++)
			{
				DBBINDING* pBinding = &rgBindings[iBinding];
				DBSTATUS dbStatus = STATUS_BINDING(*pBinding, pData);
				
				switch(dbStatus)
				{
					case DBSTATUS_S_OK:
					case DBSTATUS_S_ISNULL:
						//Obtain the storage object if it succeeded 
						//(so we can release it after the comparision)
						if(pBinding == FindBinding(cBindings, rgBindings, DBTYPE_IUNKNOWN))
							TESTC(GetStorageObject(cBindings, rgBindings, pData, m_riidStorage, (IUnknown**)&pIUnknown));
												
						//Verify the data so this successfully obtained column
						if(!RowsetA.CompareTableData(FIRST_ROW+iRow, pData, 1/*cBindings*/, pBinding))
						{
							//Data incorrect for this row!
							TERROR("Data was incorrect for row " << FIRST_ROW+iRow);
							QTESTC(FALSE);
						}

						SAFE_RELEASE(pIUnknown);
						break;
				
					default:
						//Spec specifcally indicates for this senario DBSTATUS_E_UNAVAILABLE
						TESTC(dbStatus == DBSTATUS_E_UNAVAILABLE);
						break;
				};
			}
		}

		SAFE_RELEASE(pIUnknown);
	}

CLEANUP:
	RowsetA.ReleaseRows(cRowsObtained, rghRows);
	RowsetA.ReleaseAccessor(hAccessor, cBindings, rgBindings, pData);
	SAFE_RELEASE(pIUnknown);
	SAFE_FREE(rghRows);
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc General - 4 - DBPROP_ACCESSORDER - DBPROPVAL_AO_SEQUENTIALSTORAGEOBJECTS - retrieve columns beyond stream
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Read::Variation_8()
{
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	HROW  hRow = NULL;
	DBCOUNTITEM iRow;
	DBORDINAL cBindings = 0;
	DBBINDING* rgBindings = NULL;
	void* pData = NULL;
	DBLENGTH cBytes = 0;
	IUnknown* pIUnknown = NULL;
	HRESULT hr = S_OK;
	
	//NOTE: DBPROPVAL_AO_SEQUENTIAL is covered in the IRowset test...
	//In this test we are interested only in storage objects...
	CRowset RowsetA;
	RowsetA.SetSettableProperty(DBPROP_ACCESSORDER, DBPROPSET_ROWSET, (void*)DBPROPVAL_AO_SEQUENTIALSTORAGEOBJECTS, DBTYPE_I4);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);
	
	//Create Accessor binding BLOB/Storage Object data
	//NOTE: This binds all columns (potentially even columns beyond the stream).  This may fail with
	//SequentialStorageObjects according to spec (DBSTATUS_E_UNAVAILABLE)
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, &cBindings, &rgBindings, &cBytes, m_dwFlags), S_OK)
	SAFE_ALLOC(pData, void*, cBytes);
	
	//Obtain all the rows (sequentially)
	for(iRow=0; iRow<RowsetA.m_ulTableRows; iRow++)
	{
		TESTC_(RowsetA.GetNextRows(&hRow),S_OK)

		//Obtain the Storage Object
		TEST3C_(hr = RowsetA.GetData(hRow, hAccessor, pData),S_OK,DB_S_ERRORSOCCURRED,DB_E_ERRORSOCCURRED)
		
		if(hr == S_OK)
		{
			//Verify Storage Object matches backend
			TESTC(GetStorageObject(cBindings, rgBindings, pData, m_riidStorage, (IUnknown**)&pIUnknown));
			if(!RowsetA.CompareTableData(FIRST_ROW+iRow/*iRow*/, pData, cBindings, rgBindings))
			{
				//Data incorrect for this row!
				TERROR("Data was incorrect for row " << FIRST_ROW+iRow);
				QTESTC(FALSE);
			}
		}
		else
		{
			//Verify the errors...
			for(ULONG iBinding=0; iBinding<cBindings; iBinding++)
			{
				DBBINDING* pBinding = &rgBindings[iBinding];
				DBSTATUS dbStatus = STATUS_BINDING(*pBinding, pData);
				
				switch(dbStatus)
				{
					case DBSTATUS_S_OK:
					case DBSTATUS_S_ISNULL:
						//Obtain the storage object if it succeeded 
						//(so we can release it after the comparision)
						if(pBinding == FindBinding(cBindings, rgBindings, DBTYPE_IUNKNOWN))
							TESTC(GetStorageObject(cBindings, rgBindings, pData, m_riidStorage, (IUnknown**)&pIUnknown));
										
						//Verify the data so this successfully obtained column
						if(!RowsetA.CompareTableData(FIRST_ROW+iRow, pData, 1/*cBindings*/, pBinding))
						{
							//Data incorrect for this row!
							TERROR("Data was incorrect for row " << FIRST_ROW+iRow);
							QTESTC(FALSE);
						}

						SAFE_RELEASE(pIUnknown);
						break;
				
					default:
					{
						//Spec specifcally indicates for this senario DBSTATUS_E_UNAVAILABLE
						//But this is only allowed for the Stream and column beyond, all columns
						//prior to the stream should be returned successfully...
						DBBINDING* pStreamBinding = FindBinding(cBindings, rgBindings, DBTYPE_IUNKNOWN);
						TESTC(pBinding->iOrdinal >= pStreamBinding->iOrdinal);						
						TESTC(dbStatus == DBSTATUS_E_UNAVAILABLE);
						break;
					}
				};
			}
		}
	
		SAFE_RELEASE(pIUnknown);
		RowsetA.ReleaseRows(hRow);
	}

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor, cBindings, rgBindings, pData);
	SAFE_RELEASE(pIUnknown);
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc Boundary - Open and Close stream, without reading
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Read::Variation_9()
{
	TBEGIN
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	HROW  hRow = NULL;

	DBLENGTH cBytes = 0;
	IUnknown* pIUnknown = NULL;

	CRowset RowsetA;
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);
	
	//Grab a row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK)
	
	//Create Accessor binding BLOB/Storage Object data
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, NULL, NULL, NULL, m_dwFlags | BLOB_BIND_STR), S_OK)

	//Obtain the Storage interface
	TESTC_(RowsetA.GetStorageData(hRow, hAccessor, NULL, NULL, m_riidStorage, (IUnknown**)&pIUnknown),S_OK)
	
	//Don't read the stream.  In the 2.5 OLE DB Interfaces there is an IRow::Open method which
	//returns a stream object.  It would be VERY common for a user to obtain an object an either
	//release it for some reason (another error in a method), or not actually read all the stream, 
	//which would be the same senario as not reading any.  Sense many providers probably cache
	//the stream on any read which may end up reading all of it, we don't read any...
//	TEST2C_(StorageRead(pIUnknown, m_pBuffer, 0, &cBytes),S_OK,S_FALSE)

	//Release the Stream	
	RowsetA.ReleaseAccessor(hAccessor);
	RowsetA.ReleaseRows(hRow);
	SAFE_RELEASE_(pIUnknown);
	RowsetA.DropRowset();

	//Now actually try to do something else...
	TESTC_(RowsetA.CreateRowset(),S_OK);

	//Grab a row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK)
	TESTC(RowsetA.VerifyRowHandles(ONE_ROW, &hRow, FIRST_ROW));

CLEANUP:
	SAFE_RELEASE_(pIUnknown);
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor);
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc Boundary - ::Read [valid, 0, NULL] - with outstanding row handles, accessors, and storage object
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Read::Variation_10()
{
	TBEGIN
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	HROW  hRow = NULL;
	IUnknown* pIUnknown = NULL;
	DBLENGTH cBytes = INVALID(DBLENGTH);

	CRowset RowsetA;
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);
	
	//Grab a row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK)
	
	//Create Accessor binding BLOB/Storage Object data
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, NULL, NULL, NULL, m_dwFlags), S_OK)

	//Obtain the Storage interface
	TESTC_(RowsetA.GetStorageData(hRow, hAccessor, NULL, NULL, m_riidStorage, (IUnknown**)&pIUnknown),S_OK)
	
	//Now call IStorage::Read(NULL, valid, valid)
	TESTC_(StorageRead(pIUnknown, m_pBuffer, 0, NULL),S_OK)
	
CLEANUP:
	RowsetA.Terminate();

	//According to the spec the Storage lifetime is bound by the active row.
	//"... When the rowset is released, it forces the release of any remaining rows
	//	or accessors the consumer may hold. Such handle objects are subordinate to 
	//	the rowset. That is, they do not take reference counts upon the rowset and 
	//	cannot cause the rowset to linger beyond the point where all the interfaces 
	//	for the rowset have been released. The rowset must clean up all such 
	//	subordinate objects."

//	RowsetA.ReleaseRows(hRow);
//	RowsetA.ReleaseAccessor(hAccessor);

	//Make sure the Storage Object is still usable, but Zombied after the rowset is released.
	//The rowset must cleanup all outstanding row handles and accessor handles.  And doing so will
	//cause the storage objects lifetime to end, making it a zombie.
	if(pIUnknown)
	{
		TCHECK(StorageRead(pIUnknown, m_pBuffer, 1, &cBytes),E_UNEXPECTED);
		TCOMPARE_(cBytes == 0);
	}
	SAFE_RELEASE_(pIUnknown);
	TRETURN;
}
// }}




// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc Boundary - ::Read [valid, 0, NULL] - with outstanding row handles, accessors, and storage object without releasing
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSeqStream_Read::Variation_11()
{ 
	TBEGIN
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	HROW  hRow = NULL;
	IUnknown* pIUnknown = NULL;
	DBLENGTH cBytes = INVALID(DBLENGTH);

	CRowset RowsetA;
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);
	
	//Grab a row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK)
	
	//Create Accessor binding BLOB/Storage Object data
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, NULL, NULL, NULL, m_dwFlags), S_OK)

	//Obtain the Storage interface
	TESTC_(RowsetA.GetStorageData(hRow, hAccessor, NULL, NULL, m_riidStorage, (IUnknown**)&pIUnknown),S_OK)
	
	//Now call IStorage::Read(NULL, valid, valid)
	TESTC_(StorageRead(pIUnknown, m_pBuffer, 0, NULL),S_OK)
	
CLEANUP:
	RowsetA.Terminate();

	//According to the spec the Storage lifetime is bound by the active row.
	//"... When the rowset is released, it forces the release of any remaining rows
	//	or accessors the consumer may hold. Such handle objects are subordinate to 
	//	the rowset. That is, they do not take reference counts upon the rowset and 
	//	cannot cause the rowset to linger beyond the point where all the interfaces 
	//	for the rowset have been released. The rowset must clean up all such 
	//	subordinate objects."

//	RowsetA.ReleaseRows(hRow);
//	RowsetA.ReleaseAccessor(hAccessor);

	if(pIUnknown)
	{
		//Proposely don't release this storage object, this will cause a leak, but will
		//more importantly make sure that the rowset can be released with outstanding
		//row handles, outstanding accessors, and outstanding storage objects 
		//(since their lifetime is controlled by the rows lifetime...)

		if(!TCHECK(StorageRead(pIUnknown, m_pBuffer, 1, &cBytes),E_UNEXPECTED))
		{
			//NOTE: Some providers have bugs where they do not correctly Zombie the stream,
			//as it incorrestly has a refcount on the rowset.  Due to this bug in the provider,
			//(releasing the rowset does not zombie the stream), to some providers this creates a "lock"
			//on the table and all future operations hang.  To prevent "no testing" after this point,
			//if we are on a provider that incorrectly doesn't zombie, then just count it as an error
			//and move on, otherwise we will leave the strem and future operations had better succeed...
			SAFE_RELEASE_(pIUnknown);
		}
		
		TCOMPARE_(cBytes == 0);
	}
	
	TRETURN;
} 
// }} TCW_VAR_PROTOTYPE_END



// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc Boundary - ::Read [valid, 0, valid]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Read::Variation_12()
{
	TBEGIN
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	HROW  hRow = NULL;

	DBLENGTH cBytes = 0;
	IUnknown* pIUnknown = NULL;

	CRowset RowsetA;
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);
	
	//Grab a row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK)
	
	//Create Accessor binding BLOB/Storage Object data
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, NULL, NULL, NULL, m_dwFlags | BLOB_BIND_STR), S_OK)

	//Obtain the Storage interface
	TESTC_(RowsetA.GetStorageData(hRow, hAccessor, NULL, NULL, m_riidStorage, (IUnknown**)&pIUnknown),S_OK)
	
	//Now call IStorage::Read(NULL, valid, valid)
	TEST2C_(StorageRead(pIUnknown, m_pBuffer, 0, &cBytes),S_OK,S_FALSE)
	TESTC(cBytes == 0)

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor);
	SAFE_RELEASE_(pIUnknown);
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc Boundary - ::Read [valid, N, NULL]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Read::Variation_13()
{
	TBEGIN
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	HROW  hRow = NULL;
	IUnknown* pIUnknown = NULL;

	CRowset RowsetA;
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);
	
	//Grab a row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK)
	
	//Create Accessor binding BLOB/Storage Object data
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, NULL, NULL, NULL, m_dwFlags), S_OK)

	//Obtain the Storage interface
	TESTC_(RowsetA.GetStorageData(hRow, hAccessor, NULL, NULL, m_riidStorage, (IUnknown**)&pIUnknown),S_OK)
	
	//Now call IStorage::Read(NULL, valid, valid)
	TESTC_(StorageRead(pIUnknown, m_pBuffer, 10, NULL),S_OK)

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor);
	SAFE_RELEASE_(pIUnknown);
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc Boundary - ::Read [valid, N, valid]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Read::Variation_14()
{
	TBEGIN
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	HROW  hRow = NULL;

	DBLENGTH cBytes = 0;
	IUnknown* pIUnknown = NULL;

	CRowset RowsetA;
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);
	
	//Grab a row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK)
	
	//Create Accessor binding BLOB/Storage Object data
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, NULL, NULL, NULL, m_dwFlags | BLOB_BIND_STR), S_OK)

	//Obtain the Storage interface
	TESTC_(RowsetA.GetStorageData(hRow, hAccessor, NULL, NULL, m_riidStorage, (IUnknown**)&pIUnknown),S_OK)
	
	//Now call IStorage::Read(NULL, valid, valid)
	TESTC_(StorageRead(pIUnknown, m_pBuffer, 10, &cBytes),S_OK)
	TESTC(cBytes == 10)

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor);
	SAFE_RELEASE_(pIUnknown);
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc Boundary - ::Read more than 1 call to Read to read the buffer
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Read::Variation_15()
{		
	TBEGIN
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	HROW  hRow = NULL;
	
	DBLENGTH cbRead		= 0;
	DBLENGTH cbTotal	= 0;
	ULONG cbOriginal	= DATA_SIZE;
	IUnknown* pIUnknown = NULL;
	HRESULT hr = S_OK;

	CRowset RowsetA;
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);
	
	//Grab a row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK)
	
	//Create Accessor binding BLOB/Storage Object data
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, NULL, NULL, NULL, m_dwFlags), S_OK)

	//Get the Data, ISequentialStream - should actually succeed
	TESTC_(RowsetA.GetStorageData(hRow, hAccessor, m_pBuffer2, &cbOriginal, m_riidStorage, NULL),S_OK)
	TESTC_(RowsetA.GetStorageData(hRow, hAccessor, NULL, NULL, m_riidStorage, (IUnknown**)&pIUnknown),S_OK)
	
	//Now call IStorage::Read - 0 bytes
	TEST2C_(StorageRead(pIUnknown, m_pBuffer, 0, &cbRead),S_OK,S_FALSE)
	TESTC(cbRead == 0)

	//Now call IStorage::Read - reading in chunks
	while(SUCCEEDED(hr))
	{
		TEST2C_(hr = StorageRead(pIUnknown, (BYTE*)m_pBuffer + cbTotal, 1, &cbRead, cbTotal),S_OK,S_FALSE);
		cbTotal += cbRead;

		//Note: We exit so we don't overflow the buffer, (even if the provider has a bug)
		//we will catch that later
		if(cbRead == 0 || cbTotal == cbOriginal)
			break;
	}
	
	TESTC(cbTotal == cbOriginal)
	TESTC(memcmp(m_pBuffer, m_pBuffer2, cbTotal)==0)
	
	//Now call IStorage::Read - make sure were at the end
	TEST2C_(StorageRead(pIUnknown, m_pBuffer, 1, &cbRead, cbTotal),S_OK,S_FALSE)
	TESTC(cbRead == 0)

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor);
	SAFE_RELEASE_(pIUnknown);
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc Boundary - ::Read Even / Odd number of bytes
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Read::Variation_16()
{
	TBEGIN
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	HROW  hRow = NULL;

	DBLENGTH cBytes = 0;
	IUnknown* pIUnknown = NULL;

	CRowset RowsetA;
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);
	
	//Grab a row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK)
	
	//Create Accessor binding BLOB/Storage Object data
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, NULL, NULL, NULL, m_dwFlags | BLOB_BIND_STR), S_OK)

	//Obtain the Storage interface
	TESTC_(RowsetA.GetStorageData(hRow, hAccessor, NULL, NULL, m_riidStorage, (IUnknown**)&pIUnknown),S_OK)
	
	//Now call IStorage::Read - even numer of bytes
	TESTC_(StorageRead(pIUnknown, m_pBuffer, 10, &cBytes),S_OK)
	TESTC(cBytes == 10)

	//Now call IStorage::Read - odd number of bytes
	TESTC_(StorageRead(pIUnknown, m_pBuffer, 11, &cBytes),S_OK)
	TESTC(cBytes == 11)

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor);
	SAFE_RELEASE_(pIUnknown);
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc Boundary - ::Write after retrieved Storage Objects pointer from GetData
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Read::Variation_17()
{
	TBEGIN
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	HROW  hRow = NULL;

	DBLENGTH cBytes = 0;
	IUnknown* pIUnknown = NULL;

	CRowset RowsetA;
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);
	
	//Grab a row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK)
	
	//Create Accessor binding BLOB/Storage Object data
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, NULL, NULL, NULL, m_dwFlags), S_OK)

	//Obtain the Storage interface
	TESTC_(RowsetA.GetStorageData(hRow, hAccessor, NULL, NULL, m_riidStorage, (IUnknown**)&pIUnknown),S_OK)
	
	//Now call ISequentialStream->Write on the retrieved pointer
	TESTC_(StorageWrite(pIUnknown, m_pBuffer, 10, &cBytes),STG_E_ACCESSDENIED)
	TESTC(cBytes == 0)

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor);
	SAFE_RELEASE_(pIUnknown);
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Read::Variation_18()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc Error - S_FALSE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Read::Variation_19()
{
	TBEGIN
	HRESULT hr;
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	HROW  hRow = NULL;
	
	DBLENGTH cBytes = 0;
	IUnknown* pIUnknown = NULL;

	CRowset RowsetA;
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);
	
	//Grab a row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK)
	
	//Create Accessor binding BLOB/Storage Object data
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, NULL, NULL, NULL, m_dwFlags | BLOB_BIND_STR), S_OK)

	//Obtain the Storage interface
	TESTC_(RowsetA.GetStorageData(hRow, hAccessor, NULL, NULL, m_riidStorage, (IUnknown**)&pIUnknown),S_OK)
	
	//Now call IStorage::Read, more than the max bytes
	//According to OLE2 this senario can return either S_OK or S_FALSE
	hr = StorageRead(pIUnknown, m_pBuffer, DATA_SIZE*sizeof(void*), &cBytes);
	TESTC(hr == S_OK || hr == S_FALSE)

	//But make sure that it returns something less than the max and greater than 0
	TESTC(cBytes != 0 && cBytes != DATA_SIZE*sizeof(void*))

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor);
	SAFE_RELEASE_(pIUnknown);
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc Error - STG_E_ACCESSDENIED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Read::Variation_20()
{
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc Error - STG_E_REVERTED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Read::Variation_21()
{
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(22)
//*-----------------------------------------------------------------------
// @mfunc Error - STG_E_INVALIDPOINTER
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Read::Variation_22()
{
	TBEGIN
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	HROW  hRow = NULL;

	DBLENGTH cBytes = INVALID(DBLENGTH);
	IUnknown* pIUnknown = NULL;

	CRowset RowsetA;
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);
	
	//Grab a row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK)
	
	//Create Accessor binding BLOB/Storage Object data
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, NULL, NULL, NULL, m_dwFlags | BLOB_BIND_STR), S_OK)

	//Obtain the Storage interface
	TESTC_(RowsetA.GetStorageData(hRow, hAccessor, NULL, NULL, m_riidStorage, (IUnknown**)&pIUnknown),S_OK)
	
	//Now call IStorage::Read(NULL, valid, valid)
	TESTC_(StorageRead(pIUnknown, NULL, 10, &cBytes),STG_E_INVALIDPOINTER);
	TESTC(cBytes == 0)

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor);
	SAFE_RELEASE_(pIUnknown);
	TRETURN;
}
// }}



// {{ TCW_VAR_PROTOTYPE(23)
//*-----------------------------------------------------------------------
// @mfunc Parameters - pObject NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Read::Variation_23()
{
	TBEGIN
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	HROW  hRow = NULL;
	HRESULT hr = S_OK;

	IUnknown* pIUnknown = NULL;
	DBLENGTH cBytes = 0;
	void* pBuffer = NULL;

	CRowset RowsetA;
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);
	
	//Grab a row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK)
	
	//Create Accessor binding BLOB/Storage Object data
	//According to the 2.1 OLE DB Spec pObject == NULL is equivalent to setting pObject->iid == IID_IUnknown
	//Retriving IUnknown binding is useful since it doesn't require knowing ahead of time what
	//type of objects the provider supports ahead of time, and is useful for services to just
	//pass object instances to other services which actually to the reading, etc
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, NULL, NULL, &cBytes, BLOB_NULL_POBJECT), S_OK)

	//Alloc buffers
	SAFE_ALLOC(pBuffer, BYTE, cBytes);
	
	//Get the Data, ISequentialStream - should actually succeed
	TESTC_(RowsetA.GetStorageData(hRow, hAccessor, pBuffer, (ULONG*)&cBytes, IID_IUnknown, (IUnknown**)&pIUnknown),S_OK)
	TESTC(cBytes != 0);

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor);
	SAFE_RELEASE_(pIUnknown);		 
	SAFE_FREE(pBuffer);
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(24)
//*-----------------------------------------------------------------------
// @mfunc Parameters - pObject->dwFlags STGM_READ
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Read::Variation_24()
{
	TBEGIN
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	HROW  hRow = NULL;

	IUnknown* pIUnknown = NULL;

	CRowset RowsetA;
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);
	
	//Grab a row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK)
	
	//Create Accessor binding BLOB/Storage Object data
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, NULL, NULL, NULL, m_dwFlags | BLOB_STGM_READ), S_OK)

	//Obtain the Storage interface
	TESTC_(RowsetA.GetStorageData(hRow, hAccessor, NULL, NULL, m_riidStorage, (IUnknown**)&pIUnknown),S_OK)
	
	//Now call IStorage::Read(NULL, valid, valid)
	TESTC_(StorageRead(pIUnknown, m_pBuffer, 0, NULL),S_OK)
	
CLEANUP:
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor);
	SAFE_RELEASE_(pIUnknown);
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(25)
//*-----------------------------------------------------------------------
// @mfunc Parameters - pObject->dwFlags STGM_READ | STGM_DIRECT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Read::Variation_25()
{
	TBEGIN
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	HROW  hRow = NULL;
	IUnknown* pIUnknown = NULL;

	CRowset RowsetA;
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);
	
	//Grab a row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK)
	
	//Create Accessor binding BLOB/Storage Object data
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, NULL, NULL, NULL, m_dwFlags | BLOB_STGM_READ | BLOB_STGM_DIRECT), S_OK)

	//Obtain the Storage interface
	TESTC_(RowsetA.GetStorageData(hRow, hAccessor, NULL, NULL, m_riidStorage, (IUnknown**)&pIUnknown),S_OK)
	
	//Now call IStorage::Read(NULL, valid, valid)
	TESTC_(StorageRead(pIUnknown, m_pBuffer, 0, NULL),S_OK)
	
CLEANUP:
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor);
	SAFE_RELEASE_(pIUnknown);
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(26)
//*-----------------------------------------------------------------------
// @mfunc Parameters - pObject->dwFlags STGM_WRITE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Read::Variation_26()
{
	CRowset RowsetA;
	ULONG_PTR ulValue = 0;
	HRESULT hrExpected = DB_E_BADSTORAGEFLAGS;
	DBBINDSTATUS dwBindStatus = DBBINDSTATUS_BADSTORAGEFLAGS;

	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);
		
	//Determine Storage STGM Support
	TESTW(RowsetA.GetProperty(DBPROP_STORAGEFLAGS, DBPROPSET_ROWSET, &ulValue)); 

	hrExpected = (ulValue & DBPROPVAL_STGM_WRITE) ? S_OK : DB_E_BADSTORAGEFLAGS;
	dwBindStatus = (ulValue & DBPROPVAL_STGM_WRITE) ? DBBINDSTATUS_OK : DBBINDSTATUS_BADSTORAGEFLAGS;

	//Create Accessor binding BLOB/Storage Object data
	TESTC(VerifyAccessorValidation(&RowsetA, DBACCESSOR_ROWDATA, 
		m_dwFlags | BLOB_STGM_WRITE, hrExpected, dwBindStatus));

CLEANUP:
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(27)
//*-----------------------------------------------------------------------
// @mfunc Parameters - pObject->dwFlags STGM_READWRITE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Read::Variation_27()
{
	CRowset RowsetA;
	ULONG_PTR ulValue = 0;
	HRESULT hrExpected = DB_E_BADSTORAGEFLAGS;
	DBBINDSTATUS dwBindStatus = DBBINDSTATUS_BADSTORAGEFLAGS;

	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);
	
	//Determine Storage STGM Support
	TESTW(RowsetA.GetProperty(DBPROP_STORAGEFLAGS, DBPROPSET_ROWSET, &ulValue)); 

	hrExpected = ulValue & DBPROPVAL_STGM_READWRITE ? S_OK : DB_E_BADSTORAGEFLAGS;
	dwBindStatus = ulValue & DBPROPVAL_STGM_READWRITE ? DBBINDSTATUS_OK : DBBINDSTATUS_BADSTORAGEFLAGS;

	//Create Accessor binding BLOB/Storage Object data
	TESTC(VerifyAccessorValidation(&RowsetA, DBACCESSOR_ROWDATA, 
		m_dwFlags | BLOB_STGM_READWRITE, hrExpected, dwBindStatus));

CLEANUP:
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(28)
//*-----------------------------------------------------------------------
// @mfunc Parameters - pObject->dwFlags STGM_TRANSACTED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Read::Variation_28()
{
	CRowset RowsetA;
	ULONG_PTR ulValue = 0;
	HRESULT hrExpected = DB_E_BADSTORAGEFLAGS;
	DBBINDSTATUS dwBindStatus = DBBINDSTATUS_BADSTORAGEFLAGS;

	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);
	
	//Determine Storage STGM Support
	//DBPROP_STORAGEFLAGSS has been removed from the spec...
//	TESTW(RowsetA.GetProperty(DBPROP_STORAGEFLAGS, DBPROPSET_ROWSET, &ulValue)); 
	hrExpected = ulValue & DBPROPVAL_STGM_TRANSACTED ? S_OK : DB_E_BADSTORAGEFLAGS;
	dwBindStatus = ulValue & DBPROPVAL_STGM_TRANSACTED ? DBBINDSTATUS_OK : DBBINDSTATUS_BADSTORAGEFLAGS;

	//Create Accessor binding BLOB/Storage Object data
	TESTC(VerifyAccessorValidation(&RowsetA, DBACCESSOR_ROWDATA, 
		m_dwFlags | BLOB_STGM_TRANSACTED, hrExpected, dwBindStatus));

CLEANUP:
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(29)
//*-----------------------------------------------------------------------
// @mfunc Parameters - pObject->dwFlags ULONG_MAX
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Read::Variation_29()
{
	CRowset RowsetA;

	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);
	
	//Create Accessor binding BLOB/Storage Object data
	TESTC(VerifyAccessorValidation(&RowsetA, DBACCESSOR_ROWDATA, 
		m_dwFlags | BLOB_STGM_INVALID, DB_E_BADSTORAGEFLAGS, DBBINDSTATUS_BADSTORAGEFLAGS));
	
CLEANUP:
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(30)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Read::Variation_30()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(31)
//*-----------------------------------------------------------------------
// @mfunc Parameters - pObject->iid IID_ISequentialStream
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Read::Variation_31()
{
	TBEGIN
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	HROW  hRow = NULL;

	DBLENGTH cBytes = 0;
	IUnknown* pIUnknown = NULL;

	CRowset RowsetA;
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);
	
	//Grab a row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK)
	
	//Create Accessor binding BLOB/Storage Object data
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, NULL, NULL, NULL, m_dwFlags), S_OK)

	//Get the Data, ISequentialStream - should actually succeed
	TESTC_(RowsetA.GetStorageData(hRow, hAccessor, NULL, NULL, m_riidStorage, (IUnknown**)&pIUnknown),S_OK)
	
	//Now call IStorage::Read(NULL, valid, valid)
	TESTC_(StorageRead(pIUnknown, m_pBuffer, 10, &cBytes),S_OK)
	TESTC(cBytes == 10)

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor);
	SAFE_RELEASE_(pIUnknown);
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(32)
//*-----------------------------------------------------------------------
// @mfunc Parameters - pObject->iid IID_IStorage
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Read::Variation_32()
{
	ULONG_PTR ulValue = 0;
	CRowset RowsetA;
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	RowsetA.SetSettableProperty(DBPROP_IStorage);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);

	//Obtain supported Storage Interfaces
	TESTC(::GetProperty(DBPROP_STRUCTUREDSTORAGE, DBPROPSET_DATASOURCEINFO, g_pIDBInitialize, &ulValue));
	
	//Verify IID_IStorage
	if(ulValue & DBPROPVAL_SS_ISTORAGE)
	{
		//Create Accessor binding BLOB/IID_IStorage data
		TESTC(VerifyAccessorValidation(&RowsetA, DBACCESSOR_ROWDATA, 
			BLOB_IID_ISTORAGE, S_OK, DBBINDSTATUS_OK));
	}
	else
	{
		//Create Accessor binding BLOB/IID_IStorage data
		TESTC(VerifyAccessorValidation(&RowsetA, DBACCESSOR_ROWDATA, 
			BLOB_IID_ISTORAGE, E_NOINTERFACE, DBBINDSTATUS_NOINTERFACE));
	}

CLEANUP:
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(33)
//*-----------------------------------------------------------------------
// @mfunc Parameters - pObject->iid IID_IStream
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Read::Variation_33()
{
	ULONG_PTR ulValue = 0;
	CRowset RowsetA;
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	RowsetA.SetSettableProperty(DBPROP_IStream);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);
	
	//Obtain supported Storage Interfaces
	TESTC(::GetProperty(DBPROP_STRUCTUREDSTORAGE, DBPROPSET_DATASOURCEINFO, g_pIDBInitialize, &ulValue));
	
	//Verify IID_IStorage
	if(ulValue & DBPROPVAL_SS_ISTREAM)
	{
		//Create Accessor binding BLOB/IID_IStorage data
		TESTC(VerifyAccessorValidation(&RowsetA, DBACCESSOR_ROWDATA, 
			BLOB_IID_ISTREAM, S_OK, DBBINDSTATUS_OK));
	}
	else
	{
		//Create Accessor binding BLOB/IID_IStorage data
		TESTC(VerifyAccessorValidation(&RowsetA, DBACCESSOR_ROWDATA, 
			BLOB_IID_ISTREAM, E_NOINTERFACE, DBBINDSTATUS_NOINTERFACE));
	}

CLEANUP:
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(34)
//*-----------------------------------------------------------------------
// @mfunc Parameters - pObject->iid IID_ILockBytes
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Read::Variation_34()
{
	ULONG_PTR ulValue = 0;
	CRowset RowsetA;
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	RowsetA.SetSettableProperty(DBPROP_ILockBytes);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);

	//Obtain supported Storage Interfaces
	TESTC(::GetProperty(DBPROP_STRUCTUREDSTORAGE, DBPROPSET_DATASOURCEINFO, g_pIDBInitialize, &ulValue));
	
	//Verify IID_IStorage
	if(ulValue & DBPROPVAL_SS_ILOCKBYTES)
	{
		//Create Accessor binding BLOB/IID_IStorage data
		TESTC(VerifyAccessorValidation(&RowsetA, DBACCESSOR_ROWDATA, 
			BLOB_IID_ILOCKBYTES, S_OK, DBBINDSTATUS_OK));
	}
	else
	{
		//Create Accessor binding BLOB/IID_ILockBytes data
		//Should fail, since Property was not supported...
		TESTC(VerifyAccessorValidation(&RowsetA, DBACCESSOR_ROWDATA, 
			BLOB_IID_ILOCKBYTES, E_NOINTERFACE, DBBINDSTATUS_NOINTERFACE));
	}

CLEANUP:
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(35)
//*-----------------------------------------------------------------------
// @mfunc Parameters - pObject->iid IID_IUnknown
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Read::Variation_35()
{
	TBEGIN
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	HROW  hRow = NULL;
	HRESULT hr = S_OK;

	DBLENGTH cBytes = 0;
	IUnknown* pIUnknown = NULL;
	void* pBuffer = NULL;

	CRowset RowsetA;
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);
	
	//Grab a row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK)
	
	//Create Accessor binding BLOB/Storage Object data
	//Retriving IUnknown binding is useful since it doesn't require knowing ahead of time what
	//type of objects the provider supports ahead of time, and is useful for services to just
	//pass object instances to other services which actually to the reading, etc
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, NULL, NULL, &cBytes, BLOB_IID_IUNKNOWN), S_OK)

	//Alloc buffers
	SAFE_ALLOC(pBuffer, BYTE, cBytes);
	
	//Get the Data, ISequentialStream - should actually succeed
	TESTC_(RowsetA.GetStorageData(hRow, hAccessor, pBuffer, (ULONG*)&cBytes, IID_IUnknown, (IUnknown**)&pIUnknown),S_OK)
	TESTC(cBytes != 0);

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor);
	SAFE_RELEASE_(pIUnknown);		 
	SAFE_FREE(pBuffer);
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(36)
//*-----------------------------------------------------------------------
// @mfunc Parameters - pObject->iid IID_NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Read::Variation_36()
{
	CRowset RowsetA;
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);
	
	//Create Accessor binding BLOB/IID_IStorage data
	TESTC(VerifyAccessorValidation(&RowsetA, DBACCESSOR_ROWDATA, 
		BLOB_IID_NULL, E_NOINTERFACE, DBBINDSTATUS_NOINTERFACE));

CLEANUP:
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(37)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Read::Variation_37()
{
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(38)
//*-----------------------------------------------------------------------
// @mfunc Parameters - Multiple Storage columns
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Read::Variation_38()
{
	HRESULT hrExpected = S_OK;
	DBBINDSTATUS dwBindStatus = DBBINDSTATUS_OK;

	ULONG cBlobs = 0;
	DBORDINAL i;
	DBORDINAL cColumns = 0;
	DBCOLUMNINFO* rgColInfo = NULL;
	BOOL fDefferred = TRUE;
	DBBINDSTATUS* rgBindStatus = NULL;
	BOOL fMultipleObjects = FALSE;

	CRowset RowsetA;
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);

	//Find the Total Number of BLOB Columns
	TESTC_(RowsetA.GetColInfo(&cColumns, &rgColInfo),S_OK);
	for(i=0; i<cColumns; i++)
		if(rgColInfo[i].dwFlags & DBCOLUMNFLAGS_ISLONG)
			cBlobs++;

	//Expected Results
	fMultipleObjects = ::GetProperty(DBPROP_MULTIPLESTORAGEOBJECTS, DBPROPSET_DATASOURCEINFO, g_pIDBInitialize, VARIANT_TRUE);
	hrExpected = cBlobs>1 && !fMultipleObjects ? E_NOINTERFACE : S_OK;
	dwBindStatus = hrExpected==E_NOINTERFACE ? DBBINDSTATUS_NOINTERFACE : DBBINDSTATUS_OK;

	//Only Bind the BLOB Column, so we know how many BLOB columns there are
	TESTC(VerifyAccessorValidation(&RowsetA, DBACCESSOR_ROWDATA, 
		m_dwFlags | BLOB_BIND_ALL_BLOBS, hrExpected, dwBindStatus, fMultipleObjects==FALSE, BLOB_COLS_BOUND, &rgBindStatus, &fDefferred));
	
	//If !DeferredValidation and we are not allowed to have multiple blobs
	//bound, we need to verify that the first blob binding succeeds, and 
	//all following ones fail, (can only have 1 bound...)
	if(!fDefferred && rgBindStatus)
	{
		TESTC(cBlobs >= 2);
		TESTC(rgBindStatus[0]==DBBINDSTATUS_OK);
		TESTC(VerifyArray(cBlobs-1, &rgBindStatus[1], dwBindStatus));
	}

CLEANUP:
	PROVIDER_FREE(rgColInfo);
	PROVIDER_FREE(rgBindStatus);
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(39)
//*-----------------------------------------------------------------------
// @mfunc Parameters - 1 Storage column, 1 long data
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Read::Variation_39()
{
	HRESULT hrExpected = S_OK;
	DBBINDSTATUS dwBindStatus = DBBINDSTATUS_OK;
	DBBINDSTATUS* rgBindStatus = NULL;
	BOOL fDefferred = TRUE;

	ULONG cBlobs = 0;
	DBORDINAL i;
	DBORDINAL cColumns = 0;
	DBCOLUMNINFO* rgColInfo = NULL;
	BOOL fMultipleObjects = FALSE;

	CRowset RowsetA;
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);

	//Find the Total Number of BLOB Columns
	TESTC_(RowsetA.GetColInfo(&cColumns, &rgColInfo),S_OK);
	for(i=0; i<cColumns; i++)
		if(rgColInfo[i].dwFlags & DBCOLUMNFLAGS_ISLONG)
			cBlobs++;

	//Expected Results
	fMultipleObjects = ::GetProperty(DBPROP_MULTIPLESTORAGEOBJECTS, DBPROPSET_DATASOURCEINFO, g_pIDBInitialize, VARIANT_TRUE);
	hrExpected = cBlobs>1 && !fMultipleObjects ? E_NOINTERFACE : S_OK;
	dwBindStatus = hrExpected==E_NOINTERFACE ? DBBINDSTATUS_NOINTERFACE : DBBINDSTATUS_OK;

	//Only Bind the BLOB Column, so we know how many BLOB columns there are
	TESTC(VerifyAccessorValidation(&RowsetA, DBACCESSOR_ROWDATA, 
		m_dwFlags | BLOB_BIND_ALL_BLOBS, hrExpected, dwBindStatus, fMultipleObjects==FALSE, BLOB_COLS_BOUND, &rgBindStatus, &fDefferred));

	//If !DeferredValidation and we are not allowed to have multiple blobs
	//bound, we need to verify that the first blob binding succeeds, and 
	//all following ones fail, (can only have 1 bound...)
	if(!fDefferred && rgBindStatus)
	{
		TESTC(cBlobs >= 2);
		TESTC(rgBindStatus[0]==DBBINDSTATUS_OK);
		TESTC(VerifyArray(cBlobs-1, &rgBindStatus[1], dwBindStatus));
	}

CLEANUP:
	PROVIDER_FREE(rgColInfo);
	PROVIDER_FREE(rgBindStatus);
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(40)
//*-----------------------------------------------------------------------
// @mfunc Parameters - 1 Storage column, Multiple long data
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Read::Variation_40()
{
	HRESULT hrExpected = S_OK;
	DBBINDSTATUS dwBindStatus = DBBINDSTATUS_OK;
	DBBINDSTATUS* rgBindStatus = NULL;
	BOOL fDefferred = TRUE;
	BOOL fMultipleObjects = FALSE;

	ULONG cBlobs = 0;
	DBORDINAL i;
	DBORDINAL cColumns = 0;
	DBCOLUMNINFO* rgColInfo = NULL;

	CRowset RowsetA;
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);

	//Find the Total Number of BLOB Columns
	TESTC_(RowsetA.GetColInfo(&cColumns, &rgColInfo),S_OK);
	for(i=0; i<cColumns; i++)
		if(rgColInfo[i].dwFlags & DBCOLUMNFLAGS_ISLONG)
			cBlobs++;

	//Expected Results
	fMultipleObjects = ::GetProperty(DBPROP_MULTIPLESTORAGEOBJECTS, DBPROPSET_DATASOURCEINFO, g_pIDBInitialize, VARIANT_TRUE);
	hrExpected = cBlobs>1 && !fMultipleObjects ? E_NOINTERFACE : S_OK;
	dwBindStatus = hrExpected==E_NOINTERFACE ? DBBINDSTATUS_NOINTERFACE : DBBINDSTATUS_OK;

	//Only Bind the BLOB Column, so we know how many BLOB columns there are
	TESTC(VerifyAccessorValidation(&RowsetA, DBACCESSOR_ROWDATA, 
		m_dwFlags | BLOB_BIND_ALL_BLOBS, hrExpected, dwBindStatus, fMultipleObjects==FALSE, BLOB_COLS_BOUND, &rgBindStatus, &fDefferred));

	//If !DeferredValidation and we are not allowed to have multiple blobs
	//bound, we need to verify that the first blob binding succeeds, and 
	//all following ones fail, (can only have 1 bound...)
	if(!fDefferred && rgBindStatus)
	{
		TESTC(cBlobs >= 2);
		TESTC(rgBindStatus[0]==DBBINDSTATUS_OK);
		TESTC(VerifyArray(cBlobs-1, &rgBindStatus[1], dwBindStatus));
	}

CLEANUP:
	PROVIDER_FREE(rgColInfo);
	PROVIDER_FREE(rgBindStatus);
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(41)
//*-----------------------------------------------------------------------
// @mfunc Parameters - NULL Storage column, GetData - GetNextRows - GetData, without CANHOLDROWS
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Read::Variation_41()
{
	TBEGIN
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	
	HRESULT hr = S_OK;
	HROW  hRow = NULL;
	DBCOUNTITEM cRowsObtained = 0;
	DBLENGTH cBytes = 0;
	
	DBORDINAL cBindings = 0;
	DBBINDING* rgBindings = NULL;
	IUnknown* pIUnknown = NULL;

	CRowset RowsetA;
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);
	
	//Create Accessor binding BLOB/Storage Object data
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, &cBindings, &rgBindings, &cBytes, m_dwFlags), S_OK)

	//We have a choice to find the NULL Storage column, either we can
	//Loop over all CCol of the table to find the row number with NULL,
	//Or we can simple assume their is at least 1 NULL column in the table
	//and loop over all rows in the rowset.  This is a much better solution
	//since it forces thier to be other calls after the NULL GetData call...
	
	//Loop over all rows in the row set..
	while(TRUE)
	{
		//Grab the next row
		//Exit if end of rowset
		hr = RowsetA.GetNextRows(&hRow);
		TEST2C_(hr, S_OK, DB_S_ENDOFROWSET);
		if(hr == DB_S_ENDOFROWSET)
			break;
		
		//Get the Data, ISequentialStream - should actually succeed
		//Row Objects (IRow::Open) can return DB_E_NOTFOUND for ISNULL data...
		hr = RowsetA.GetStorageData(hRow, hAccessor, NULL, NULL, m_riidStorage, (IUnknown**)&pIUnknown);
		TEST2C_(hr, S_OK, DB_E_NOTFOUND);

		//Release the storage object
		SAFE_RELEASE(pIUnknown);

		//Release the row handle, since didn't ask for CANHOLDROWS...
		TESTC_(RowsetA.ReleaseRows(hRow),S_OK);
	}

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	PROVIDER_FREE(rgBindings);
	SAFE_RELEASE_(pIUnknown);
	TRETURN;
}
// }}

// {{ TCW_VAR_PROTOTYPE(42)
//*-----------------------------------------------------------------------
// @mfunc Parameters - NULL Storage column, GetData - GetNextRows - GetData, with CANHOLDROWS
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Read::Variation_42()
{
	TBEGIN
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	
	HRESULT hr = S_OK;
	HROW  hRow = NULL;
	DBCOUNTITEM cRowsObtained = 0;
	DBLENGTH cBytes = 0;
	
	DBORDINAL cBindings = 0;
	DBBINDING* rgBindings = NULL;
	IUnknown* pIUnknown = NULL;

	CRowset RowsetA;
	RowsetA.SetProperty(DBPROP_CANHOLDROWS);
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);
	
	//Create Accessor binding BLOB/Storage Object data
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, &cBindings, &rgBindings, &cBytes, m_dwFlags), S_OK)

	//We have a choice to find the NULL Storage column, either we can
	//Loop over all CCol of the table to find the row number with NULL,
	//Or we can simple assume their is at least 1 NULL column in the table
	//and loop over all rows in the rowset.  This is a much better solution
	//since it forces thier to be other calls after the NULL GetData call...
	
	//Loop over all rows in the row set..
	while(TRUE)
	{
		//Grab the next row
		//Exit if end of rowset
		hr = RowsetA.GetNextRows(&hRow);
		TESTC(hr == S_OK || hr == DB_S_ENDOFROWSET);
		if(hr == DB_S_ENDOFROWSET)
			break;
		
		//Get the Data, ISequentialStream - should actually succeed
		//Row Objects (IRow::Open) can return DB_E_NOTFOUND for ISNULL data...
		hr = RowsetA.GetStorageData(hRow, hAccessor, NULL, NULL, m_riidStorage, (IUnknown**)&pIUnknown);
		TEST2C_(hr, S_OK, DB_E_NOTFOUND);

		//Need to release the object before calling it again...
		SAFE_RELEASE(pIUnknown);
	}

CLEANUP:
	SAFE_RELEASE_(pIUnknown);
	RowsetA.ReleaseRows(hRow);
	PROVIDER_FREE(rgBindings);
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(43)
//*-----------------------------------------------------------------------
// @mfunc Parameters - NULL Storage column, GetData - GetData - GetData
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Read::Variation_43()
{
	TBEGIN
	HRESULT hr = S_OK;
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	
	HROW* rghRows = NULL;
	DBCOUNTITEM cRowsObtained = 0;
	DBLENGTH cBytes = 0;
	
	ULONG_PTR ulMaxOpenRows = 0;
	DBCOUNTITEM cTotalRows = 0;

	DBCOUNTITEM i=0;
	DBORDINAL cBindings = 0;
	DBBINDING* rgBindings = NULL;
	IUnknown* pIUnknown = NULL;

	CRowset RowsetA;
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);
	cTotalRows = RowsetA.GetTotalRows();
	
	RowsetA.GetProperty(DBPROP_MAXOPENROWS, DBPROPSET_ROWSET, &ulMaxOpenRows);
	TESTC_PROVIDER(ulMaxOpenRows==0 || ulMaxOpenRows >= cTotalRows);

	//Create Accessor binding BLOB/Storage Object data
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, &cBindings, &rgBindings, &cBytes, m_dwFlags), S_OK)

	//We have a choice to find the NULL Storage column, either we can
	//Loop over all CCol of the table to find the row number with NULL,
	//Or we can simple assume their is at least 1 NULL column in the table
	//and loop over all rows in the rowset.  This is a much better solution
	//since it forces thier to be other calls after the NULL GetData call...
	
	//Grab all the row handles ahead of time
	TESTC_(RowsetA()->GetNextRows(NULL, 0, cTotalRows+1, &cRowsObtained, &rghRows),DB_S_ENDOFROWSET);
	

	//Loop over all rows in the row set..
	for(i=0; i<cRowsObtained; i++)
	{
		//Get the Data, ISequentialStream - should actually succeed
		//Row Object (IRow::Open) can also return DB_E_NOTFOUND
		hr = RowsetA.GetStorageData(rghRows[i], hAccessor, NULL, NULL, m_riidStorage, (IUnknown**)&pIUnknown);
		TEST2C_(hr, S_OK, DB_E_NOTFOUND);

		//Need to release the object before calling it again...
		SAFE_RELEASE(pIUnknown);
	}


CLEANUP:
	RowsetA.ReleaseRows(cRowsObtained, rghRows);
	RowsetA.ReleaseAccessor(hAccessor, cBindings, rgBindings);

	SAFE_RELEASE_(pIUnknown);
	PROVIDER_FREE(rghRows);
	
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(44)
//*-----------------------------------------------------------------------
// @mfunc Sequence - GetData twice [same row]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Read::Variation_44()
{
	TBEGIN
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	HROW  hRow = NULL;
	HRESULT hr = S_OK;
	DBLENGTH cBytes = 0;
	
	void* pData     = NULL;

	DBORDINAL cBindings = 0;
	DBBINDING* rgBindings = NULL;
	IUnknown* pIUnknown = NULL;
	IUnknown* pIUnknown2 = NULL;

	CRowset RowsetA;
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);
	
	//Grab a row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK)
	
	//Create Accessor binding BLOB/Storage Object data
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, &cBindings, &rgBindings, &cBytes, m_dwFlags), S_OK)

	//Alloc buffers
	SAFE_ALLOC(pData, void*, cBytes);

	//Get the Data, ISequentialStream - should actually succeed
	TESTC_(RowsetA.GetStorageData(hRow, hAccessor, NULL, NULL, m_riidStorage, (IUnknown**)&pIUnknown),S_OK)

	//Get Data again (only if the provider can have multiple objects open)
	if(::GetProperty(DBPROP_MULTIPLESTORAGEOBJECTS, DBPROPSET_DATASOURCEINFO, g_pIDBInitialize))
	{
		//Get the Data, ISequentialStream - should actually succeed
		TESTC_(RowsetA.GetStorageData(hRow, hAccessor, NULL, NULL, m_riidStorage, (IUnknown**)&pIUnknown2),S_OK)
	}
	else
	{
		//The spec allow MULTIPLESTORAGEOBJECTS=FALSE to mean "may" not allow more
		//than one, so this should allow either the expected failure or S_OK
		hr = RowsetA.GetData(hRow, hAccessor, pData);
		TEST2C_(hr, S_OK, DB_S_ERRORSOCCURRED);
		TESTC(VerifyBindingStatus(cBindings, rgBindings, pData, DBTYPE_IUNKNOWN, hr==S_OK ? DBSTATUS_S_OK : DBSTATUS_E_CANTCREATE))
	}

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor, cBindings, rgBindings, pData);
	SAFE_RELEASE_(pIUnknown);		 
	SAFE_RELEASE_(pIUnknown2);		 
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(45)
//*-----------------------------------------------------------------------
// @mfunc Sequence - GetData twice [same row] with releasing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Read::Variation_45()
{
	
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	HROW  hRow = NULL;
	
	DBLENGTH cBytes = 0;
	CRowset RowsetA;
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);
	
	//Grab a row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK)
	
	//Create Accessor binding BLOB/Storage Object data
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, NULL, NULL, &cBytes, m_dwFlags | BLOB_BIND_STR), S_OK)

	//Get the Data, ISequentialStream - should actually succeed
	TESTC_(RowsetA.GetStorageData(hRow, hAccessor, m_pBuffer, (ULONG*)&cBytes, m_riidStorage, NULL),S_OK)

	//Once ISeqStream is released we can read again
	TESTC_(RowsetA.GetStorageData(hRow, hAccessor, m_pBuffer2, (ULONG*)&cBytes, m_riidStorage, NULL),S_OK)

	//Now make sure both calls produce the same data
	TESTC(memcmp(m_pBuffer, m_pBuffer2, cBytes)==0)

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor);
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(46)
//*-----------------------------------------------------------------------
// @mfunc Sequence - GetData twice [diff row]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Read::Variation_46()
{
	TBEGIN
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	HRESULT hr = S_OK;
	
	HROW  rghRows[TWO_ROWS] = {NULL,NULL};
	DBLENGTH cBytes = 0;
	
	void* pData     = NULL;

	DBORDINAL cBindings = 0;
	DBBINDING* rgBindings = NULL;
	IUnknown* pIUnknown = NULL;
	IUnknown* pIUnknown2 = NULL;

	CRowset RowsetA;
	RowsetA.SetProperty(DBPROP_CANHOLDROWS);
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);
	
	//Grab a row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &rghRows[ROW_ONE]),S_OK)
	
	//Create Accessor binding BLOB/Storage Object data
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, &cBindings, &rgBindings, &cBytes, m_dwFlags), S_OK)

	//Alloc buffers
	SAFE_ALLOC(pData, void*, cBytes);

	//Get the Data, ISequentialStream - should actually succeed
	TESTC_(RowsetA.GetData(rghRows[ROW_ONE], hAccessor, pData),S_OK)
	TESTC(GetStorageObject(cBindings, rgBindings, pData, m_riidStorage, (IUnknown**)&pIUnknown))

	//Verify ISeqStream::Read matches backend
	TESTC(RowsetA.CompareTableData(FIRST_ROW,pData,cBindings,rgBindings))

	//Now see if we can grab another row handle, before ISeqStream is released
	hr = RowsetA.GetNextRows(&rghRows[ROW_TWO]);
	TEST2C_(hr, S_OK, E_UNEXPECTED);

	if(hr == S_OK)
	{
		//Get the Data, ISequentialStream - should actually succeed
		hr = RowsetA.GetData(rghRows[ROW_TWO], hAccessor, pData);
		TEST2C_(hr, S_OK, DB_S_ERRORSOCCURRED)
		
		//Determine which one should have been returned...	
		if(hr==S_OK)
		{
			//We can't verify the property MULTIPLESTORAGEOBJECTS == TRUE in this case
			//Since FALSE means that the provider "may not" be able to return another
			//object, but could in some circumstances.  Many providers report 
			//FALSE for this property, but are capable of returning other objects
			//under normal circumstances...
//			TESTC(::GetProperty(DBPROP_MULTIPLESTORAGEOBJECTS, DBPROPSET_DATASOURCEINFO, g_pIDBInitialize));
			TESTC(GetStorageObject(cBindings, rgBindings, pData, m_riidStorage, (IUnknown**)&pIUnknown2))
		}
		else
		{
			TESTC(::GetProperty(DBPROP_MULTIPLESTORAGEOBJECTS, DBPROPSET_DATASOURCEINFO, g_pIDBInitialize, VARIANT_FALSE));
		}
	}
	else
	{
		//Should only return an error if BLOCKINGSTORAGEOBJECTS
		TESTC(RowsetA.GetProperty(DBPROP_BLOCKINGSTORAGEOBJECTS));
	}

CLEANUP:
	RowsetA.ReleaseRows(TWO_ROWS,rghRows);
	RowsetA.ReleaseAccessor(hAccessor, cBindings, rgBindings, pData);
	SAFE_RELEASE_(pIUnknown);
	SAFE_RELEASE_(pIUnknown2);
	TRETURN;
}
// }}

// {{ TCW_VAR_PROTOTYPE(47)
//*-----------------------------------------------------------------------
// @mfunc Sequence - GetData twice [diff row] - with releasing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Read::Variation_47()
{
	TBEGIN
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	
	HROW  rghRows[TWO_ROWS] = {NULL,NULL};
	DBLENGTH cBytes = 0;
	
	void* pData     = NULL;
	void* pData2    = NULL;

	DBORDINAL cBindings = 0;
	DBBINDING* rgBindings = NULL;

	IUnknown* pIUnknown = NULL;

	CRowset RowsetA;
	RowsetA.SetProperty(DBPROP_CANHOLDROWS);
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);
	
	//Grab a row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &rghRows[ROW_ONE]),S_OK)
	
	//Create Accessor binding BLOB/Storage Object data
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, &cBindings, &rgBindings, &cBytes, m_dwFlags | BLOB_BIND_STR), S_OK)

	//Alloc buffers
	SAFE_ALLOC(pData, void*, cBytes);
	SAFE_ALLOC(pData2, void*, cBytes);

	//Get the Data, ISequentialStream - should actually succeed
	TESTC_(RowsetA.GetData(rghRows[ROW_ONE], hAccessor, pData),S_OK)
	TESTC(GetStorageObject(cBindings, rgBindings, pData, m_riidStorage, (IUnknown**)&pIUnknown))

	//Verify ISeqStream::Read matches backend
	TESTC(RowsetA.CompareTableData(FIRST_ROW,pData,cBindings,rgBindings))
	//Need to release ISeqStream before calling other methods
	SAFE_RELEASE_(pIUnknown);

	//Now see if we can grab another row handle, once ISeqStream is released
	TESTC_(RowsetA.GetNextRows(&rghRows[ROW_TWO]),S_OK)

	//Get the Data, ISequentialStream - should actually succeed
	TESTC_(RowsetA.GetData(rghRows[ROW_TWO], hAccessor, pData2),S_OK)
	TESTC(GetStorageObject(cBindings, rgBindings, pData2, m_riidStorage, (IUnknown**)&pIUnknown))

CLEANUP:
	RowsetA.ReleaseRows(TWO_ROWS,rghRows);
	RowsetA.ReleaseAccessor(hAccessor, cBindings, rgBindings, pData);
	PROVIDER_FREE(pData2);
	SAFE_RELEASE_(pIUnknown);
	TRETURN;
}
// }}



// {{ TCW_VAR_PROTOTYPE(48)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSeqStream_Read::Variation_48()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(49)
//*-----------------------------------------------------------------------
// @mfunc Sequence - GetData on Storage, with previous object open, no data read
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSeqStream_Read::Variation_49()
{ 
	TBEGIN
	HRESULT hr = S_OK;	
	HACCESSOR hAccessor1;
	HACCESSOR hAccessor2;
	
	HROW  hRow = NULL;
	DBLENGTH cBytes = 0;
	
	void* pData1    = NULL;
	void* pData2    = NULL;

	DBORDINAL cBindings1 = 0;
	DBBINDING* rgBindings1 = NULL;
	DBORDINAL cBindings2 = 0;
	DBBINDING* rgBindings2 = NULL;

	IUnknown* pIUnknown = NULL;
//	CRowset RowsetB;

	CRowset RowsetA;
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
//	RowsetA.SetSettableProperty(DBPROP_SERVERCURSOR);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);
	
	//Grab a row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK)
	
	//Create Accessor binding BLOB/Storage Object data
	TESTC_(RowsetA.CreateAccessor(&hAccessor1, DBACCESSOR_ROWDATA, DBPART_ALL, &cBindings1, &rgBindings1, &cBytes, m_dwFlags), S_OK)
	//Create Accessor binding BLOB as long data data
	TESTC_(RowsetA.CreateAccessor(&hAccessor2, DBACCESSOR_ROWDATA, DBPART_ALL, &cBindings2, &rgBindings2, &cBytes, BLOB_LONG), S_OK)

	//Alloc buffers
	SAFE_ALLOC(pData1, void*, cBytes);
	SAFE_ALLOC(pData2, void*, cBytes);

	//Get the Data, ISequentialStream - should actually succeed
	TESTC_(RowsetA.GetData(hRow, hAccessor1, pData1),S_OK)
	TESTC(GetStorageObject(cBindings1, rgBindings1, pData1, m_riidStorage, (IUnknown**)&pIUnknown))

	//Make sure we can perform other operations on the connection, while their is an active
	//stream open, and no data has been read...
//	RowsetB.SetSettableProperty(DBPROP_SERVERCURSOR);
//	TESTC_(RowsetB.CreateRowset(m_dwStorageID),S_OK);

	//NOTE: We have not read any of the above stresm.
	//So even though a provider may indicate they have blocking storage objects they may very
	//well still be able to do other operations now, verify this case. (ie: Kagera)
	TEST3C_(hr = RowsetA.GetData(hRow, hAccessor2, pData2),S_OK,DB_S_ERRORSOCCURRED, DB_E_ERRORSOCCURRED);
	
	//We might have to release the storage object before doing anything else...
	if(RowsetA.GetProperty(DBPROP_BLOCKINGSTORAGEOBJECTS))
	{
		//Provider may have blocking storage objects
		if(hr == S_OK)
		{
			//GetData succeeded although blocking objects, just indicate a warning to let the consumer know
			TWARNING("Provider was able to open another storage object (without reading any data), although BlockingStorageObjects=TRUE");
			TESTW_(hr, DB_S_ERRORSOCCURRED);
		}
		else
		{
			//Does reading all the stream help?
			TESTC(RowsetA.CompareTableData(FIRST_ROW,pData1,cBindings1,rgBindings1))
			TEST3C_(hr = RowsetA.GetData(hRow, hAccessor2, pData2),S_OK, DB_S_ERRORSOCCURRED, DB_E_ERRORSOCCURRED);
			
			if(hr == S_OK)
			{
				//GetData succeeded although blocking objects, just indicate a warning to let the consumer know
				TWARNING("Provider was able to open another storage object (by reading all the data), although BlockingStorageObjects=TRUE");
				TESTW_(hr, DB_S_ERRORSOCCURRED);
			}
			else
			{			
				//Last resort, the provider is correct - they truely have blocking storage
				//objects.  release the stream before doing anything else...
				SAFE_RELEASE_(pIUnknown);
				TESTC_(RowsetA.GetData(hRow, hAccessor2, pData2),S_OK)
			}
		}
	}
	else
	{
		//Provider does not have blocking-storage objects, then S_OK should have been returned
		TESTC_(hr, S_OK)
	}

	//Verify the Data
	TESTC(RowsetA.CompareTableData(FIRST_ROW,pData2,cBindings2,rgBindings2))

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor1, cBindings1, rgBindings1, pData1);
	RowsetA.ReleaseAccessor(hAccessor2, cBindings2, rgBindings2, pData2);
	SAFE_RELEASE_(pIUnknown);
	TRETURN;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(50)
//*-----------------------------------------------------------------------
// @mfunc Sequence - GetData on Storage, with previous object open, but all data read
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Read::Variation_50()
{
	TBEGIN
	HRESULT hr = S_OK;	
	HACCESSOR hAccessor1;
	HACCESSOR hAccessor2;
	
	HROW  hRow = NULL;
	DBLENGTH cBytes = 0;
	
	void* pData1    = NULL;
	void* pData2    = NULL;

	DBORDINAL cBindings1 = 0;
	DBBINDING* rgBindings1 = NULL;
	DBORDINAL cBindings2 = 0;
	DBBINDING* rgBindings2 = NULL;

	IUnknown* pIUnknown = NULL;

	CRowset RowsetA;
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);
	
	//Grab a row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK)
	
	//Create Accessor binding BLOB/Storage Object data
	TESTC_(RowsetA.CreateAccessor(&hAccessor1, DBACCESSOR_ROWDATA, DBPART_ALL, &cBindings1, &rgBindings1, &cBytes,m_dwFlags), S_OK)
	//Create Accessor binding BLOB as long data data
	TESTC_(RowsetA.CreateAccessor(&hAccessor2, DBACCESSOR_ROWDATA, DBPART_ALL, &cBindings2, &rgBindings2, &cBytes, BLOB_LONG), S_OK)

	//Alloc buffers
	SAFE_ALLOC(pData1, void*, cBytes);
	SAFE_ALLOC(pData2, void*, cBytes);

	//Get the Data, ISequentialStream - should actually succeed
	TESTC_(RowsetA.GetData(hRow, hAccessor1, pData1),S_OK)
	TESTC(GetStorageObject(cBindings1, rgBindings1, pData1, m_riidStorage, (IUnknown**)&pIUnknown))
	TESTC(RowsetA.CompareTableData(FIRST_ROW,pData1,cBindings1,rgBindings1))

	//NOTE: We have actually read all of the stream, (above).
	//So even though a provider may indicate they have blocking storage objects they may very
	//well still be able to do other operations now, verify this case. (ie: Kagera)
	TEST3C_(hr = RowsetA.GetData(hRow, hAccessor2, pData2),S_OK,DB_S_ERRORSOCCURRED, DB_E_ERRORSOCCURRED);
	
	//We might have to release the storage object before doing anything else...
	if(RowsetA.GetProperty(DBPROP_BLOCKINGSTORAGEOBJECTS))
	{
		//Provider may have blocking storage objects
		if(hr == S_OK)
		{
			//GetData succeeded although blocking objects, just indicate a warning to let the consumer know
			TWARNING("Provider was able to open another storage object (by reading all the data), although BlockingStorageObjects=TRUE");
			TESTW_(hr, DB_S_ERRORSOCCURRED);
		}
		else
		{
			//Have to release the stream before doing anything else...
			SAFE_RELEASE_(pIUnknown);
			TESTC_(RowsetA.GetData(hRow, hAccessor2, pData2),S_OK)
		}
	}
	else
	{
		//Provider does not have blocking-storage objects, then S_OK should have been returned
		TESTC_(hr, S_OK)
	}

	//Verify the Data
	TESTC(RowsetA.CompareTableData(FIRST_ROW,pData2,cBindings2,rgBindings2))

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor1, cBindings1, rgBindings1, pData1);
	RowsetA.ReleaseAccessor(hAccessor2, cBindings2, rgBindings2, pData2);
	SAFE_RELEASE_(pIUnknown);
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(51)
//*-----------------------------------------------------------------------
// @mfunc Sequence - GetData on Storage, with previous object open, but partial data read
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Read::Variation_51()
{
	TBEGIN
	HRESULT hr = S_OK;	
	HACCESSOR hAccessor1;
	HACCESSOR hAccessor2;
	
	HROW  hRow = NULL;
	DBLENGTH cBytes = 0;
	
	void* pData1    = NULL;
	void* pData2    = NULL;

	DBLENGTH cbRead = 0;
	BYTE* pBuffer = NULL;

	DBORDINAL cBindings1 = 0;
	DBBINDING* rgBindings1 = NULL;
	DBORDINAL cBindings2 = 0;
	DBBINDING* rgBindings2 = NULL;

	IUnknown* pIUnknown = NULL;

	CRowset RowsetA;
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);
	
	//Grab a row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK)
	
	//Create Accessor binding BLOB/Storage Object data
	TESTC_(RowsetA.CreateAccessor(&hAccessor1, DBACCESSOR_ROWDATA, DBPART_ALL, &cBindings1, &rgBindings1, &cBytes, m_dwFlags), S_OK)
	//Create Accessor binding BLOB as long data data
	TESTC_(RowsetA.CreateAccessor(&hAccessor2, DBACCESSOR_ROWDATA, DBPART_ALL, &cBindings2, &rgBindings2, &cBytes, BLOB_LONG), S_OK)

	//Alloc buffers
	SAFE_ALLOC(pData1, void*, cBytes);
	SAFE_ALLOC(pData2, void*, cBytes);
	SAFE_ALLOC(pBuffer, BYTE, cBytes + 10);

	//Get the Data, ISequentialStream - should actually succeed
	TESTC_(RowsetA.GetData(hRow, hAccessor1, pData1),S_OK)
	TESTC(GetStorageObject(cBindings1, rgBindings1, pData1, m_riidStorage, (IUnknown**)&pIUnknown))

	//Read some of the data first... (2 bytes)
	TESTC_(hr = StorageRead(pIUnknown, pBuffer, 2, &cbRead),S_OK);
	TESTC(cbRead == 2);

	//NOTE: We have not read any of the above stresm.
	//So even though a provider may indicate they have blocking storage objects they may very
	//well still be able to do other operations now, verify this case. (ie: Kagera)
	TEST3C_(hr = RowsetA.GetData(hRow, hAccessor2, pData2),S_OK,DB_S_ERRORSOCCURRED, DB_E_ERRORSOCCURRED);
	
	//We might have to release the storage object before doing anything else...
	if(RowsetA.GetProperty(DBPROP_BLOCKINGSTORAGEOBJECTS))
	{
		//Provider may have blocking storage objects
		if(hr == S_OK)
		{
			//GetData succeeded although blocking objects, just indicate a warning to let the consumer know
			TWARNING("Provider was able to open another storage object (by reading some data), although BlockingStorageObjects=TRUE");
			TESTW_(hr, DB_S_ERRORSOCCURRED);
		}
		else
		{
			//Does reading the rest of the stream help?
			TEST2C_(hr = StorageRead(pIUnknown, pBuffer, cBytes+10, &cbRead),S_OK,S_FALSE);
			//NOTE: At this point we don't know how large the data should be (without extra work), 
			//since cBytes indicates the row size for all columns (since all are bound).  But we
			//at least simply know that the stream has to be less than the all columns, and we
			//have already read 2 bytes as well...
			TESTC(cbRead <= cBytes-2);
			
			//Now try and GetData again for the stream
			TEST3C_(hr = RowsetA.GetData(hRow, hAccessor2, pData2),S_OK,DB_S_ERRORSOCCURRED, DB_E_ERRORSOCCURRED);
			
			if(hr == S_OK)
			{
				//GetData succeeded although blocking objects, just indicate a warning to let the consumer know
				TWARNING("Provider was able to open another storage object (by reading all the data), although BlockingStorageObjects=TRUE");
				TESTW_(hr, DB_S_ERRORSOCCURRED);
			}
			else
			{			
				//Last resort, the provider is correct - they truely have blocking storage
				//objects.  release the stream before doing anything else...
				SAFE_RELEASE_(pIUnknown);
				TESTC_(RowsetA.GetData(hRow, hAccessor2, pData2),S_OK)
			}
		}
	}
	else
	{
		//Provider does not have blocking-storage objects, then S_OK should have been returned
		TESTC_(hr, S_OK)
	}

	//Verify the Data
	TESTC(RowsetA.CompareTableData(FIRST_ROW,pData2,cBindings2,rgBindings2))

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor1, cBindings1, rgBindings1, pData1);
	RowsetA.ReleaseAccessor(hAccessor2, cBindings2, rgBindings2, pData2);
	SAFE_RELEASE_(pIUnknown);
	SAFE_FREE(pBuffer);
	TRETURN;
}
// }}




// {{ TCW_VAR_PROTOTYPE(52)
//*-----------------------------------------------------------------------
// @mfunc Sequence - Obtain Storage object and release without ever reading
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSeqStream_Read::Variation_52()
{ 
	TBEGIN
	HROW		hRow	  = NULL;
	HACCESSOR	hAccessor = NULL;
	IUnknown*	pIUnknown = NULL;

	DBPROP rgProp[] = 
	{
		{ DBPROP_IRowset,		DBPROPOPTIONS_REQUIRED,	DBPROPSTATUS_OK, {DB_NULLGUID, 0, (LPOLESTR)0}, {VT_BOOL, 0,0,0, VARIANT_TRUE}},
		{ DBPROP_SERVERCURSOR,	DBPROPOPTIONS_REQUIRED,	DBPROPSTATUS_OK, {DB_NULLGUID, 0, (LPOLESTR)0}, {VT_BOOL, 0,0,0, VARIANT_TRUE}}
	};

	//Loop through interesting property (cursor) combinations
	for(ULONG i=0; i<NUMELEM(rgProp); i++)
	{
		DBPROP* pProp = &rgProp[i];

		CRowset RowsetA;
		RowsetA.SetSettableProperty(pProp->dwPropertyID, DBPROPSET_ROWSET, &pProp->vValue, DBTYPE_VARIANT);

		CRowset RowsetB;
		RowsetB.SetSettableProperty(pProp->dwPropertyID, DBPROPSET_ROWSET, &pProp->vValue, DBTYPE_VARIANT);
		TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);
	
		//Obtain the storage object
		TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK)
		TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, NULL, NULL, NULL, m_dwFlags), S_OK)
		TESTC_(RowsetA.GetStorageData(hRow, hAccessor, NULL, NULL, m_riidStorage, (IUnknown**)&pIUnknown),S_OK)

		//Release the storage object - without reading any data...
		//(ie: Consumer either didn't need it, or there was an error somewhere else and is just
		//cleaning up memory allocated)
		
		//Note: Some providers will incorrectly hang, since no data has been read, so this is 
		//a good senario.  The interesting aspect is that even if only 1 byte is read everything 
		//may works fine.
		SAFE_RELEASE(pIUnknown);
	
		//Make sure it doesn't hold any references on the rowset or backend...
		TESTC_(RowsetB.CreateRowset(m_dwStorageID),S_OK);
	}

CLEANUP:
	SAFE_RELEASE(pIUnknown);
	TRETURN;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(53)
//*-----------------------------------------------------------------------
// @mfunc IAccessor - BYREF storage column should fail
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Read::Variation_53()
{
	HRESULT hrExpected = S_OK;
	DBBINDSTATUS dwBindStatus = DBBINDSTATUS_OK;
	
	CRowset RowsetA;
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);
	
	//Expected Results
	hrExpected = CanConvert(RowsetA(), DBTYPE_BYTES, DBTYPE_IUNKNOWN | DBTYPE_BYREF) ? S_OK : DB_E_UNSUPPORTEDCONVERSION;
	dwBindStatus = FAILED(hrExpected) ? DBBINDSTATUS_UNSUPPORTEDCONVERSION : DBBINDSTATUS_OK;

	//Only Bind the BLOB Column, so we know how many BLOB columns there are
	TESTC(VerifyAccessorValidation(&RowsetA, DBACCESSOR_ROWDATA, 
		m_dwFlags, hrExpected, dwBindStatus, FALSE, BLOB_COLS_BOUND, NULL, NULL, ALL_COLS_BY_REF));

CLEANUP:
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(54)
//*-----------------------------------------------------------------------
// @mfunc IAccessor - VALUE only binding
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Read::Variation_54()
{
	TBEGIN
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	
	HROW  hRow = NULL;
	DBLENGTH cBytes = 0;
	
	void* pData     = NULL;

	DBORDINAL cBindings = 0;
	DBBINDING* rgBindings = NULL;
	IUnknown* pIUnknown = NULL;

	CRowset RowsetA;
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);
	
	//Grab a row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK)
	
	//Create Accessor binding BLOB/Storage Object data, VALUE only binding
	TESTC_(GetAccessorAndBindings(RowsetA.pIAccessor(), DBACCESSOR_ROWDATA, &hAccessor,
		&rgBindings, &cBindings, &cBytes, DBPART_VALUE, BLOB_COLS_BOUND, FORWARD,
		NO_COLS_BY_REF,NULL,NULL,NULL,DBTYPE_EMPTY,0,NULL,NULL,
		NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, m_dwFlags | BLOB_BIND_STR),S_OK)

	//Alloc buffers
	SAFE_ALLOC(pData, void*, cBytes);

	//Get the Data, ISequentialStream - should actually succeed
	TESTC_(RowsetA.GetData(hRow, hAccessor, pData),S_OK)
	TESTC(GetStorageObject(cBindings, rgBindings, pData, m_riidStorage, (IUnknown**)&pIUnknown))

	//Verify ISeqStream::Read matches backend
	TESTC(RowsetA.CompareTableData(FIRST_ROW,pData,cBindings,rgBindings))

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor, cBindings, rgBindings, pData);
	SAFE_RELEASE_(pIUnknown);
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(55)
//*-----------------------------------------------------------------------
// @mfunc IAccessor - VALUE / LENGTH only binding
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Read::Variation_55()
{
	TBEGIN
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	
	HROW  hRow = NULL;
	DBLENGTH cBytes = 0;
	
	void* pData     = NULL;

	DBORDINAL cBindings = 0;
	DBBINDING* rgBindings = NULL;
	IUnknown* pIUnknown = NULL;

	CRowset RowsetA;
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);
	
	//Grab a row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK)
	
	//Create Accessor binding BLOB/Storage Object data, VALUE only binding
	TESTC_(GetAccessorAndBindings(RowsetA.pIAccessor(), DBACCESSOR_ROWDATA, &hAccessor,
		&rgBindings, &cBindings, &cBytes, DBPART_VALUE | DBPART_LENGTH,BLOB_COLS_BOUND,FORWARD,
		NO_COLS_BY_REF,NULL,NULL,NULL,DBTYPE_EMPTY,0,NULL,NULL,
		NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, m_dwFlags),S_OK)

	//Alloc buffers
	SAFE_ALLOC(pData, void*, cBytes);

	//Get the Data, ISequentialStream - should actually succeed
	TESTC_(RowsetA.GetData(hRow, hAccessor, pData),S_OK)
	TESTC(GetStorageObject(cBindings, rgBindings, pData, m_riidStorage, (IUnknown**)&pIUnknown))

	//Verify ISeqStream::Read matches backend
	TESTC(RowsetA.CompareTableData(FIRST_ROW,pData,cBindings,rgBindings))

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor, cBindings, rgBindings, pData);
	SAFE_RELEASE_(pIUnknown);
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(56)
//*-----------------------------------------------------------------------
// @mfunc IAccessor - VALUE /STATUS only binding
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Read::Variation_56()
{
	TBEGIN
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	
	HROW  hRow = NULL;
	DBLENGTH cBytes = 0;
	
	void* pData     = NULL;

	DBORDINAL cBindings = 0;
	DBBINDING* rgBindings = NULL;
	IUnknown* pIUnknown = NULL;

	CRowset RowsetA;
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);
	
	//Grab a row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK)
	
	//Create Accessor binding BLOB/Storage Object data, STATUS|VALUE only binding
	TESTC_(GetAccessorAndBindings(RowsetA.pIAccessor(), DBACCESSOR_ROWDATA, &hAccessor,
		&rgBindings, &cBindings, &cBytes, DBPART_VALUE | DBPART_STATUS,BLOB_COLS_BOUND,FORWARD,
		NO_COLS_BY_REF,NULL,NULL,NULL,DBTYPE_EMPTY,0,NULL,NULL,
		NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, m_dwFlags | BLOB_BIND_STR),S_OK)

	//Alloc buffers
	SAFE_ALLOC(pData, void*, cBytes);

	//Get the Data, ISequentialStream - should actually succeed
	TESTC_(RowsetA.GetData(hRow, hAccessor, pData),S_OK)
	TESTC(GetStorageObject(cBindings, rgBindings, pData, m_riidStorage, (IUnknown**)&pIUnknown))

	//Verify ISeqStream::Read matches backend
	TESTC(RowsetA.CompareTableData(FIRST_ROW,pData,cBindings,rgBindings))

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor, cBindings, rgBindings, pData);
	SAFE_RELEASE_(pIUnknown);
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(57)
//*-----------------------------------------------------------------------
// @mfunc IAccessor - LENGTH only binding
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Read::Variation_57()
{
	
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	
	HROW  hRow = NULL;
	DBLENGTH cBytes = 0;
	
	void* pData     = NULL;

	DBORDINAL cBindings = 0;
	DBBINDING* rgBindings = NULL;
	
	CRowset RowsetA;
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);
	
	//Grab a row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK)
	
	//Create Accessor binding BLOB/Storage Object data, VALUE only binding
	TESTC_(GetAccessorAndBindings(RowsetA.pIAccessor(), DBACCESSOR_ROWDATA, &hAccessor,
		&rgBindings, &cBindings, &cBytes, DBPART_LENGTH,BLOB_COLS_BOUND,FORWARD,
		NO_COLS_BY_REF,NULL,NULL,NULL,DBTYPE_EMPTY,0,NULL,NULL,
		NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, m_dwFlags),S_OK)

	//Alloc buffers
	SAFE_ALLOC(pData, void*, cBytes);

	//Get the Data, ISequentialStream - should actually succeed
	TESTC_(RowsetA.GetData(hRow, hAccessor, pData),S_OK)
	
	//Verify LENGTH == sizeof(IUnknown*)	
	//GetStorageObject verifies whatever is bound, in this case only LENGTH
	TESTC(GetStorageObject(cBindings, rgBindings, pData, m_riidStorage, NULL))

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor, cBindings, rgBindings, pData);
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(58)
//*-----------------------------------------------------------------------
// @mfunc IAccessor - STATUS only binding
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Read::Variation_58()
{
	
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	
	HROW  hRow = NULL;
	DBLENGTH cBytes = 0;
	
	void* pData     = NULL;

	DBORDINAL cBindings = 0;
	DBBINDING* rgBindings = NULL;

	CRowset RowsetA;
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);
	
	//Grab a row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK)
	
	//Create Accessor binding BLOB/Storage Object data, STATUS only binding
	TESTC_(GetAccessorAndBindings(RowsetA.pIAccessor(), DBACCESSOR_ROWDATA, &hAccessor,
		&rgBindings, &cBindings, &cBytes, DBPART_STATUS,BLOB_COLS_BOUND,FORWARD,
		NO_COLS_BY_REF,NULL,NULL,NULL,DBTYPE_EMPTY,0,NULL,NULL,
		NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, m_dwFlags),S_OK)

	//Alloc buffers
	SAFE_ALLOC(pData, void*, cBytes);

	//Get the Data, ISequentialStream - should actually succeed
	TESTC_(RowsetA.GetData(hRow, hAccessor, pData),S_OK)

	//Verify STATUS == DBSTATUS_S_OK	
	//GetStorageObject verifies whatever is bound, in this case only STATUS
	TESTC(GetStorageObject(cBindings, rgBindings, pData, m_riidStorage, NULL))

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor, cBindings, rgBindings, pData);
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(59)
//*-----------------------------------------------------------------------
// @mfunc IAccessor - LENGTH / STATUS only binding
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Read::Variation_59()
{
	
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	
	HROW  hRow = NULL;
	DBLENGTH cBytes = 0;
	
	void* pData     = NULL;

	DBORDINAL cBindings = 0;
	DBBINDING* rgBindings = NULL;

	CRowset RowsetA;
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);
	
	//Grab a row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK)
	
	//Create Accessor binding BLOB/Storage Object data, STATUS | LENGTH only binding
	TESTC_(GetAccessorAndBindings(RowsetA.pIAccessor(), DBACCESSOR_ROWDATA, &hAccessor,
		&rgBindings, &cBindings, &cBytes, DBPART_LENGTH | DBPART_STATUS,BLOB_COLS_BOUND,FORWARD,
		NO_COLS_BY_REF,NULL,NULL,NULL,DBTYPE_EMPTY,0,NULL,NULL,
		NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, m_dwFlags),S_OK)

	//Alloc buffers
	SAFE_ALLOC(pData, void*, cBytes);

	//Get the Data, ISequentialStream - should actually succeed
	TESTC_(RowsetA.GetData(hRow, hAccessor, pData),S_OK)

	//Verify LENGTH == sizeof(IUnknown*) && STATUS == DBSTATUS_S_OK	
	//GetStorageObject verifies whatever is bound, in this case STATUS / LENGTH
	TESTC(GetStorageObject(cBindings, rgBindings, pData, m_riidStorage, NULL))

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor, cBindings, rgBindings, pData);
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(60)
//*-----------------------------------------------------------------------
// @mfunc IAccessor - DBACCESSOR_PASSBYREF
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Read::Variation_60()
{
	HRESULT hrExpected = S_OK;
	BOOL fDeferredBlob = FALSE;
	DBBINDSTATUS dwBindStatus = DBBINDSTATUS_OK;
	DBCOLUMNINFO dbColInfo;

	CRowset RowsetA;
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);
	
	//PASSBYREF Accessors may not be supported by the provider
	TESTC_PROVIDER(::GetProperty(DBPROP_BYREFACCESSORS, DBPROPSET_DATASOURCEINFO, g_pIDBInitialize));
	
	//Determine if the BLOB column is deferred.
	TESTC_(RowsetA.FindColInfo(DBCOLUMNFLAGS_ISLONG, &dbColInfo),S_OK);
	
	//Expected Results
	fDeferredBlob = dbColInfo.dwFlags & DBCOLUMNFLAGS_MAYDEFER;
	hrExpected = fDeferredBlob ? DB_E_BADBINDINFO : S_OK;
	dwBindStatus = hrExpected==DB_E_BADBINDINFO ? DBBINDSTATUS_BADBINDINFO : DBBINDSTATUS_OK;

	//Provider may also have deferred accessor validation
	TESTC(VerifyAccessorValidation(&RowsetA, DBACCESSOR_ROWDATA | DBACCESSOR_PASSBYREF, 
		m_dwFlags, hrExpected, dwBindStatus, FALSE, BLOB_COLS_BOUND, NULL, NULL, NO_COLS_BY_REF));

CLEANUP:
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(61)
//*-----------------------------------------------------------------------
// @mfunc Related - IRowsetResynch - GetVisibleData twice [same row]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Read::Variation_61()
{
	TBEGIN
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	HRESULT hr = S_OK;
	
	HROW  hRow = NULL;
	DBLENGTH cBytes = 0;
	
	void* pData     = NULL;
	void* pData2    = NULL;

	IRowsetResynch* pIRowsetResynch = NULL;

	DBORDINAL cBindings = 0;
	DBBINDING* rgBindings = NULL;
	IUnknown* pIUnknown = NULL;

	CRowset RowsetA;
	RowsetA.SetProperty(DBPROP_IRowsetResynch);
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_PROVIDER(RowsetA.CreateRowset(m_dwStorageID)==S_OK);
	
	//Grab a row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK)
	
	//Create Accessor binding BLOB/Storage Object data
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, &cBindings, &rgBindings, &cBytes, m_dwFlags), S_OK)

	//Alloc buffers
	SAFE_ALLOC(pData, void*, cBytes);
	SAFE_ALLOC(pData2, void*, cBytes);

	//GetVisibleData, ISequentialStream - should actually succeed
	TESTC_(QI(RowsetA(),IID_IRowsetResynch, (void**)&pIRowsetResynch),S_OK)
	TESTC_(pIRowsetResynch->GetVisibleData(hRow, hAccessor, pData),S_OK)
	TESTC(GetStorageObject(cBindings, rgBindings, pData, m_riidStorage, (IUnknown**)&pIUnknown))

	//GetVisibileData again, already an open storage object
	hr = pIRowsetResynch->GetVisibleData(hRow, hAccessor, pData2);
	TEST2C_(hr, S_OK, E_UNEXPECTED);

	if(hr == S_OK)
	{
		//Verify Data is the same
		TESTC(RowsetA.CompareRowData(pData, pData2, hAccessor))
	}
	else
	{
		//Should only return an error if BLOCKINGSTORAGEOBJECTS
		TESTC(RowsetA.GetProperty(DBPROP_BLOCKINGSTORAGEOBJECTS));
	}

CLEANUP:
	//Need to release the storage object
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor, cBindings, rgBindings, pData);
	PROVIDER_FREE(pData2);

	SAFE_RELEASE_(pIUnknown);
	SAFE_RELEASE(pIRowsetResynch);
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(62)
//*-----------------------------------------------------------------------
// @mfunc Related - IRowsetResynch - GetVisibleData twice [same row] with releasing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Read::Variation_62()
{
	
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	
	HROW  hRow = NULL;
	DBLENGTH cBytes = 0;
	void* pData     = NULL;
	void* pData2    = NULL;

	IRowsetResynch* pIRowsetResynch = NULL;

	DBORDINAL cBindings = 0;
	DBBINDING* rgBindings = NULL;

	CRowset RowsetA;
	RowsetA.SetProperty(DBPROP_IRowsetResynch);
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_PROVIDER(RowsetA.CreateRowset(m_dwStorageID)==S_OK);
	
	//Grab a row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK)
	
	//Create Accessor binding BLOB/Storage Object data
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, &cBindings, &rgBindings, &cBytes, m_dwFlags | BLOB_BIND_STR), S_OK)

	//Alloc buffers
	SAFE_ALLOC(pData, void*, cBytes);
	SAFE_ALLOC(pData2, void*, cBytes);

	//GetVisibleData, ISequentialStream - should actually succeed
	TESTC_(QI(RowsetA(),IID_IRowsetResynch, (void**)&pIRowsetResynch),S_OK)
	TESTC_(pIRowsetResynch->GetVisibleData(hRow, hAccessor, pData),S_OK)

	//Call ISequentialStream::Read to read the entire buffer
	TESTC_(::GetStorageData(cBindings, rgBindings, pData, m_pBuffer, (ULONG*)&cBytes, m_riidStorage, NULL),S_OK)

	//GetVisibileData again
	TESTC_(pIRowsetResynch->GetVisibleData(hRow, hAccessor, pData2),S_OK)

	//Make sure both calls on the same row contain equal data
	TESTC_(::GetStorageData(cBindings, rgBindings, pData2, m_pBuffer2, (ULONG*)&cBytes, m_riidStorage, NULL),S_OK)
	TESTC(memcmp(m_pBuffer, m_pBuffer2, cBytes)==0)

CLEANUP:
	//Need to release the storage object
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor, cBindings, rgBindings, pData);
	PROVIDER_FREE(pData2);
	SAFE_RELEASE(pIRowsetResynch);
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(63)
//*-----------------------------------------------------------------------
// @mfunc Related - IRowsetResynch - GetVisibleData twice [diff row]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Read::Variation_63()
{
	TBEGIN
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	HRESULT hr = S_OK;
	
	HROW  rghRows[TWO_ROWS] = {NULL,NULL};
	DBLENGTH cBytes = 0;
	
	void* pData     = NULL;

	IRowsetResynch* pIRowsetResynch = NULL;

	DBORDINAL cBindings = 0;
	DBBINDING* rgBindings = NULL;
	IUnknown* pIUnknown = NULL;
	IUnknown* pIUnknown2 = NULL;

	CRowset RowsetA;
	RowsetA.SetProperty(DBPROP_CANHOLDROWS);
	RowsetA.SetProperty(DBPROP_IRowsetResynch);
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_PROVIDER(RowsetA.CreateRowset(m_dwStorageID)==S_OK);
	
	//Grab a row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &rghRows[ROW_ONE]),S_OK)
	
	//Create Accessor binding BLOB/Storage Object data
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, &cBindings, &rgBindings, &cBytes, m_dwFlags), S_OK)

	//Alloc buffers
	SAFE_ALLOC(pData, void*, cBytes);

	//GetVisibleData, ISequentialStream - should actually succeed
	TESTC_(QI(RowsetA(),IID_IRowsetResynch, (void**)&pIRowsetResynch),S_OK)
	TESTC_(pIRowsetResynch->GetVisibleData(rghRows[ROW_ONE], hAccessor, pData),S_OK)
	TESTC(::GetStorageObject(cBindings, rgBindings, pData, m_riidStorage, (IUnknown**)&pIUnknown))

	//GetNextRow, while a storage object is open
	hr = RowsetA.GetNextRows(&rghRows[ROW_TWO]);
	TEST2C_(hr, S_OK, E_UNEXPECTED);

	if(hr == S_OK)
	{
		//Get Data for the new row
		TESTC_(pIRowsetResynch->GetVisibleData(rghRows[ROW_TWO], hAccessor, pData),S_OK);
		TESTC(::GetStorageObject(cBindings, rgBindings, pData, m_riidStorage, (IUnknown**)&pIUnknown2));
	}
	else
	{
		//Should only return an error if BLOCKINGSTORAGEOBJECTS
		TESTC(RowsetA.GetProperty(DBPROP_BLOCKINGSTORAGEOBJECTS));
	}

CLEANUP:
	RowsetA.ReleaseRows(TWO_ROWS,rghRows);
	RowsetA.ReleaseAccessor(hAccessor, cBindings, rgBindings, pData);

	SAFE_RELEASE(pIRowsetResynch);
	SAFE_RELEASE_(pIUnknown);
	SAFE_RELEASE_(pIUnknown2);
	TRETURN;
}
// }}

// {{ TCW_VAR_PROTOTYPE(64)
//*-----------------------------------------------------------------------
// @mfunc Related - IRowsetResynch - GetVisibleData twice [diff row] with releasing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Read::Variation_64()
{
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	
	HROW  rghRows[TWO_ROWS] = {NULL,NULL};
	DBLENGTH cBytes = 0;
	void* pData     = NULL;
	void* pData2    = NULL;

	IRowsetResynch* pIRowsetResynch = NULL;

	DBORDINAL cBindings = 0;
	DBBINDING* rgBindings = NULL;

	CRowset RowsetA;
	RowsetA.SetProperty(DBPROP_CANHOLDROWS);
	RowsetA.SetProperty(DBPROP_IRowsetResynch);
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_PROVIDER(RowsetA.CreateRowset(m_dwStorageID)==S_OK);
	
	//Grab a row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &rghRows[ROW_ONE]),S_OK)
	
	//Create Accessor binding BLOB/Storage Object data
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, &cBindings, &rgBindings, &cBytes, m_dwFlags | BLOB_BIND_STR), S_OK)

	//Alloc buffers
	SAFE_ALLOC(pData, void*, cBytes);
	SAFE_ALLOC(pData2, void*, cBytes);

	//GetVisibleData, ISequentialStream - should actually succeed
	TESTC_(QI(RowsetA(),IID_IRowsetResynch, (void**)&pIRowsetResynch),S_OK)

	//Call ISequentialStream::Read to read the entire buffer
	TESTC_(pIRowsetResynch->GetVisibleData(rghRows[ROW_ONE], hAccessor, pData),S_OK)
	TESTC_(::GetStorageData(cBindings, rgBindings, pData, m_pBuffer, (ULONG*)&cBytes, m_riidStorage, NULL),S_OK)

	//GetNextRow
	TESTC_(RowsetA.GetNextRows(&rghRows[ROW_TWO]),S_OK)

	//GetVisibileData again
	TESTC_(pIRowsetResynch->GetVisibleData(rghRows[ROW_TWO], hAccessor, pData2),S_OK)
	TESTC_(::GetStorageData(cBindings, rgBindings, pData2, m_pBuffer2, (ULONG*)&cBytes, m_riidStorage, NULL),S_OK)

	//Make sure diff rows have diff data
	TESTC(memcmp(m_pBuffer, m_pBuffer2, cBytes)!=0)

CLEANUP:
	//Need to release the storage object
	RowsetA.ReleaseRows(TWO_ROWS,rghRows);
	RowsetA.ReleaseAccessor(hAccessor, cBindings, rgBindings, pData);
	PROVIDER_FREE(pData2);
	SAFE_RELEASE(pIRowsetResynch);
	TRETURN;
}
// }}

// {{ TCW_VAR_PROTOTYPE(65)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Read::Variation_65()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(66)
//*-----------------------------------------------------------------------
// @mfunc Related - IRowsetUpdate - GetOriginalData twice [same row]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Read::Variation_66()
{
	TBEGIN
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	HROW  hRow = NULL;
	HRESULT hr = S_OK;

	void* pData  = NULL;
	IUnknown* pIUnknown = NULL;
	IUnknown* pIUnknown2 = NULL;

	DBORDINAL cBindings = 0;
	DBBINDING* rgBindings = NULL;
	DBLENGTH cBytes = INVALID(DBLENGTH);

	CRowsetUpdate RowsetA;
	RowsetA.SetProperty(DBPROP_IRowsetUpdate);
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_PROVIDER(RowsetA.CreateRowset(m_dwStorageID)==S_OK);
	
	//Grab a row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK)
	
	//Create Accessor binding BLOB/Storage Object data
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, &cBindings, &rgBindings, &cBytes, m_dwFlags), S_OK)

	//Alloc buffers
	SAFE_ALLOC(pData, void*, cBytes);

	//Get the Data, ISequentialStream - should actually succeed
	TESTC_(RowsetA.pIRowsetUpdate()->GetOriginalData(hRow, hAccessor, pData),S_OK)
	TESTC_(::GetStorageData(cBindings, rgBindings, pData, NULL, NULL, m_riidStorage, (IUnknown**)&pIUnknown),S_OK)
	
	//GetOriginalData again - (already an open storage object)
	hr = RowsetA.pIRowsetUpdate()->GetOriginalData(hRow, hAccessor, pData);

	//Get Data again (only if the provider can have multiple objects open)
	if(::GetProperty(DBPROP_MULTIPLESTORAGEOBJECTS, DBPROPSET_DATASOURCEINFO, g_pIDBInitialize))
	{
		//Provider allows multiple open storage objects
		TESTC_(hr, S_OK);
		TESTC_(::GetStorageData(cBindings, rgBindings, pData, NULL, NULL, m_riidStorage, (IUnknown**)&pIUnknown2),S_OK)
	}
	else
	{
		//GetOriginalData again, already an open storage object
		//The spec allow MULTIPLESTORAGEOBJECTS=FALSE to mean "may" not allow more
		//than one, so this should allow either the expected failure or S_OK
		TEST2C_(hr, S_OK, DB_S_ERRORSOCCURRED);
		TESTC(VerifyBindingStatus(cBindings, rgBindings, pData, DBTYPE_IUNKNOWN, hr==S_OK ? DBSTATUS_S_OK : DBSTATUS_E_CANTCREATE))
		if(hr==S_OK)
		{
			TESTC_(::GetStorageData(cBindings, rgBindings, pData, NULL, NULL, m_riidStorage, (IUnknown**)&pIUnknown2),S_OK)
		}
	}

CLEANUP:
	RowsetA.Terminate();

	//According to the spec the Storage lifetime is bound by the active row.
	//"... When the rowset is released, it forces the release of any remaining rows
	//	or accessors the consumer may hold. Such handle objects are subordinate to 
	//	the rowset. That is, they do not take reference counts upon the rowset and 
	//	cannot cause the rowset to linger beyond the point where all the interfaces 
	//	for the rowset have been released. The rowset must clean up all such 
	//	subordinate objects."

	//Proposely don't release this storage object, this will cause a leak, but will
	//more importantly make sure that the rowset can be released with outstanding
	//row handles, outstanding accessors, and outstanding storage objects 
	//(since their lifetime is controlled by the rows lifetime...)

//	RowsetA.ReleaseRows(hRow);
//	RowsetA.ReleaseAccessor(hAccessor, cBindings, rgBindings, pData);

	//Make sure the Storage Object is still usable, but Zombied after the rowset is released.
	//The rowset must cleanup all outstanding row handles and accessor handles.  And doing so will
	//cause the storage objects lifetime to end, making it a zombie.
	if(pIUnknown)
	{
		TEST2C_(StorageRead(pIUnknown, m_pBuffer, 1, &cBytes),E_UNEXPECTED, E_FAIL);		
		TCOMPARE_(cBytes == 0);
	}

	if (pData && rgBindings)
		FreeAccessorBufferAndBindings(&cBindings, &rgBindings, &pData, true);

	SAFE_RELEASE_(pIUnknown);
	SAFE_RELEASE_(pIUnknown2);
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(67)
//*-----------------------------------------------------------------------
// @mfunc Related - IRowsetUpdate - GetOriginalData twice [same row] with releasing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Read::Variation_67()
{
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	HROW  hRow = NULL;
	DBLENGTH cBytes = 0;

	CRowsetUpdate RowsetA;
	RowsetA.SetProperty(DBPROP_IRowsetUpdate);
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_PROVIDER(RowsetA.CreateRowset(m_dwStorageID)==S_OK);	

	//Grab a row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK)
	
	//Create Accessor binding BLOB/Storage Object data
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, NULL, NULL, &cBytes, m_dwFlags | BLOB_BIND_STR), S_OK)

	//GetOriginalData, ISequentialStream - should actually succeed
	TESTC_(RowsetA.GetOrgStorageData(hRow, hAccessor, m_pBuffer, (ULONG*)&cBytes, m_riidStorage, NULL),S_OK)

	//GetOriginalData again
	TESTC_(RowsetA.GetOrgStorageData(hRow, hAccessor, m_pBuffer2, (ULONG*)&cBytes, m_riidStorage, NULL),S_OK)

	//Make sure both calls on the same row contain equal data
	TESTC(memcmp(m_pBuffer, m_pBuffer2, cBytes)==0)

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor);
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(68)
//*-----------------------------------------------------------------------
// @mfunc Related - IRowsetUpdate - GetOriginalData twice [diff row]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Read::Variation_68()
{
	TBEGIN
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	HRESULT hr = S_OK;
	
	HROW  rghRows[TWO_ROWS] = {NULL,NULL};
	
	DBLENGTH cBytes = 0;
	IUnknown* pIUnknown = NULL;
	IUnknown* pIUnknown2 = NULL;

	CRowsetUpdate RowsetA;
	RowsetA.SetProperty(DBPROP_CANHOLDROWS);
	RowsetA.SetProperty(DBPROP_IRowsetUpdate);
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_PROVIDER(RowsetA.CreateRowset(m_dwStorageID)==S_OK);
	
	//Grab a row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &rghRows[ROW_ONE]),S_OK)
	
	//Create Accessor binding BLOB/Storage Object data
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, NULL, NULL, &cBytes, m_dwFlags), S_OK)

	//GetOriginalData, ISequentialStream - should actually succeed
	TESTC_(RowsetA.GetOrgStorageData(rghRows[ROW_ONE], hAccessor, m_pBuffer, (ULONG*)&cBytes, m_riidStorage, (IUnknown**)&pIUnknown),S_OK)

	//GetNextRow, while a storage object is open
	hr = RowsetA.GetNextRows(&rghRows[ROW_TWO]);
	TEST2C_(hr, S_OK, E_UNEXPECTED);

	if(hr == S_OK)
	{
		//Get Data for the new row (Could be NULL data - DB_E_NOTFOUND)
		hr = RowsetA.GetOrgStorageData(rghRows[ROW_TWO], hAccessor, m_pBuffer2, (ULONG*)&cBytes, m_riidStorage, (IUnknown**)&pIUnknown2);
		TEST3C_(hr, S_OK, DB_E_NOTFOUND, DB_S_ERRORSOCCURRED)
		
		//Determine which one should have been returned...	
		//Since the spec allow MULTIPLESTORAGEOBJECTS=FALSE to indicate may not allow
		//Even the success case could really be a FALSE case, so all we can really
		//check is that if unable to get another object, that it is FALSE...
		if(hr==DB_S_ERRORSOCCURRED)
		{
			TESTC(::GetProperty(DBPROP_MULTIPLESTORAGEOBJECTS, DBPROPSET_DATASOURCEINFO, g_pIDBInitialize, VARIANT_FALSE));
		}
	}
	else
	{
		//Should only return an error if BLOCKINGSTORAGEOBJECTS
		TESTC(RowsetA.GetProperty(DBPROP_BLOCKINGSTORAGEOBJECTS));
	}

CLEANUP:
	RowsetA.ReleaseRows(TWO_ROWS,rghRows);
	RowsetA.ReleaseAccessor(hAccessor);
	SAFE_RELEASE_(pIUnknown);
	SAFE_RELEASE_(pIUnknown2);
	TRETURN;
}
// }}

// {{ TCW_VAR_PROTOTYPE(69)
//*-----------------------------------------------------------------------
// @mfunc Related - IRowsetUpdate - GetOriginalData twice [diff row] with releasing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Read::Variation_69()
{
	HRESULT hr = S_OK;
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	HROW  rghRows[TWO_ROWS] = {NULL,NULL};
	DBLENGTH cBytes = 0;
	
	CRowsetUpdate RowsetA;
	RowsetA.SetProperty(DBPROP_CANHOLDROWS);
	RowsetA.SetProperty(DBPROP_IRowsetUpdate);
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_PROVIDER(RowsetA.CreateRowset(m_dwStorageID)==S_OK);
	
	//Grab a row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &rghRows[ROW_ONE]),S_OK)
	
	//Create Accessor binding BLOB/Storage Object data
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, NULL, NULL, &cBytes, m_dwFlags | BLOB_BIND_STR), S_OK)

	//GetOriginalData, ISequentialStream - should actually succeed
	TESTC_(RowsetA.GetOrgStorageData(rghRows[ROW_ONE], hAccessor, m_pBuffer, (ULONG*)&cBytes, m_riidStorage, NULL),S_OK)

	//GetNextRow, while a storage object is open
	TESTC_(RowsetA.GetNextRows(&rghRows[ROW_TWO]),S_OK)

	//GetOrginalData again, already an open storage object 
	hr = RowsetA.GetOrgStorageData(rghRows[ROW_TWO], hAccessor, m_pBuffer2, (ULONG*)&cBytes, m_riidStorage, NULL);
	TEST2C_(hr, S_OK, DB_E_NOTFOUND)

	//Make sure diff rows have diff data
	if(hr == S_OK)
	{
		TESTC(memcmp(m_pBuffer, m_pBuffer2, cBytes)!=0)
	}
	else
	{
		//Could contain NULL data - DB_E_NOTFOUND
		TESTC(cBytes == 0);
	}

CLEANUP:
	RowsetA.ReleaseRows(TWO_ROWS,rghRows);
	RowsetA.ReleaseAccessor(hAccessor);
	TRETURN;
}
// }}

// {{ TCW_VAR_PROTOTYPE(70)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Read::Variation_70()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(71)
//*-----------------------------------------------------------------------
// @mfunc ICommandWithParameters -
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Read::Variation_71()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(72)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Read::Variation_72()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(73)
//*-----------------------------------------------------------------------
// @mfunc Usage - ReleaseRows
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Read::Variation_73()
{
	TBEGIN
	HRESULT hr = S_OK;	
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	HROW  rghRows[TWO_ROWS] = {NULL,NULL};
	DBLENGTH cBytes = 0;
	ULONG ulRefCount = 0;
	
	void* pData     = NULL;
	void* pData2    = NULL;

	DBORDINAL cBindings = 0;
	DBBINDING* rgBindings = NULL;

	IUnknown* pIUnknown  = NULL;
	IUnknown* pIUnknown2 = NULL;

	CRowset RowsetA;
	RowsetA.SetProperty(DBPROP_CANHOLDROWS);
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);
	
	//Grab a row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &rghRows[ROW_ONE]),S_OK)
	
	//Create Accessor binding BLOB/Storage Object data
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, &cBindings, &rgBindings, &cBytes, m_dwFlags), S_OK)

	//Alloc buffers
	SAFE_ALLOC(pData, void*, cBytes);
	SAFE_ALLOC(pData2, void*, cBytes);

	//Get the Data, ISequentialStream - should actually succeed
	TESTC_(RowsetA.GetData(rghRows[ROW_ONE], hAccessor, pData),S_OK)
	TESTC(GetStorageObject(cBindings, rgBindings, pData, m_riidStorage, (IUnknown**)&pIUnknown))

	//Verify ISeqStream::Read matches backend
	TESTC(RowsetA.CompareTableData(FIRST_ROW,pData,cBindings,rgBindings))

	//Need to call ReleaseRows which will allow another storage object open
	TESTC_(RowsetA.ReleaseRows(1, &rghRows[ROW_ONE], &ulRefCount),S_OK)
	
	//Try releasing again
	if(ulRefCount != 0)
		TESTC_(RowsetA.ReleaseRows(1, &rghRows[ROW_ONE], &ulRefCount),S_OK)

	//Now see if we can grab another row handle, once ISeqStream is released
	if(ulRefCount == 0 || ::GetProperty(DBPROP_MULTIPLESTORAGEOBJECTS, DBPROPSET_DATASOURCEINFO, g_pIDBInitialize))
	{
		TESTC_(RowsetA.GetNextRows(&rghRows[ROW_TWO]),S_OK)

		//Get the Data, ISequentialStream - should actually succeed
		TESTC_(RowsetA.GetData(rghRows[ROW_TWO], hAccessor, pData2),S_OK)
		TESTC(GetStorageObject(cBindings, rgBindings, pData2, m_riidStorage, (IUnknown**)&pIUnknown2))
	}
	else
	{
		hr = RowsetA.GetNextRows(&rghRows[ROW_TWO]);
		TEST2C_(hr, S_OK, E_UNEXPECTED);
		if(hr == S_OK)
		{
			//Since the row is still active and we are olny allowed 1 storage object
			//open GetData should fail to retrieve the other storage object
			//The spec allows for MULTIPLESTORAGEOBJECTS=FALSE to indicate "may" not
			//allow more than 1, we need to allow the successful case...
			hr = RowsetA.GetData(rghRows[ROW_TWO], hAccessor, pData2);
			TEST2C_(hr, S_OK, DB_S_ERRORSOCCURRED);
			TESTC(VerifyBindingStatus(cBindings, rgBindings, pData2, DBTYPE_IUNKNOWN, hr==S_OK ? DBSTATUS_S_OK : DBSTATUS_E_CANTCREATE));
		}
		else
		{
			//Should only return an error if BLOCKINGSTORAGEOBJECTS
			TESTC(RowsetA.GetProperty(DBPROP_BLOCKINGSTORAGEOBJECTS));
		}
	}


CLEANUP:
	RowsetA.ReleaseRows(TWO_ROWS,rghRows);
	RowsetA.ReleaseAccessor(hAccessor, cBindings, rgBindings, pData);
	PROVIDER_FREE(pData2);
	SAFE_RELEASE_(pIUnknown);
	SAFE_RELEASE_(pIUnknown2);
	TRETURN;
}
// }}
	
// {{ TCW_VAR_PROTOTYPE(74)
//*-----------------------------------------------------------------------
// @mfunc Usage - ReleaseRows - ::Read E_UNEXPECTED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Read::Variation_74()
{
	TBEGIN
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	HROW  hRow = NULL;
	
	DBLENGTH cBytes = MAX_PTR;
	IUnknown* pIUnknown = NULL;
	ULONG ulRefCount = 0;

	CRowset RowsetA;
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);
	
	//Grab a row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK)
	
	//Create Accessor binding BLOB/Storage Object data
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, NULL, NULL, NULL, m_dwFlags | BLOB_BIND_STR), S_OK)

	//Get the Data, ISequentialStream - should actually succeed
	TESTC_(RowsetA.GetStorageData(hRow, hAccessor, NULL, NULL, m_riidStorage, (IUnknown**)&pIUnknown),S_OK)

	//Need to call ReleaseRows which will invalidate the storage object
	TESTC_(RowsetA.ReleaseRows(1, &hRow, &ulRefCount),S_OK)

	//Make sure the storage object is no longer valid
	//Only if the Row was actually released...
	if(ulRefCount == 0)
	{
		TESTC_(StorageRead(pIUnknown, m_pBuffer, 1000, &cBytes),E_UNEXPECTED);
		TESTC(cBytes == 0);
	}
	else
	{
		TESTC_(StorageRead(pIUnknown, m_pBuffer, 1000, &cBytes),S_OK);
		TESTC(cBytes>0 && cBytes<1000);
	}

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor);
	SAFE_RELEASE_(pIUnknown);
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(75)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Read::Variation_75()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(76)
//*-----------------------------------------------------------------------
// @mfunc Stress - BLOBs beyond 64k
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Read::Variation_76()
{
	
	return TEST_PASS;
}
// }}




// {{ TCW_VAR_PROTOTYPE(77)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSeqStream_Read::Variation_77()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(78)
//*-----------------------------------------------------------------------
// @mfunc MultiUser - 2 Rowsets with GetData
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Read::Variation_78()
{
	TBEGIN
	HACCESSOR hAccessorA;
	HACCESSOR hAccessorB;
	
	HROW  hRowA = NULL;
	HROW  hRowB = NULL;

	DBLENGTH cBytes = 0;

	IUnknown* pIUnknownA = NULL;
	IUnknown* pIUnknownB = NULL;

	CRowset RowsetA;
	CRowset RowsetB;
	
	//RowsetA / RowsetB
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);
	RowsetB.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetB.CreateRowset(m_dwStorageID),S_OK);
	
	//Grab a row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hRowA),S_OK)
	TESTC_(RowsetB.GetRow(FIRST_ROW, &hRowB),S_OK)
	
	//Create Accessor binding BLOB/Storage Object data
	TESTC_(RowsetA.CreateAccessor(&hAccessorA,DBACCESSOR_ROWDATA,DBPART_ALL, NULL, NULL, &cBytes, m_dwFlags), S_OK)
	TESTC_(RowsetB.CreateAccessor(&hAccessorB,DBACCESSOR_ROWDATA,DBPART_ALL, NULL, NULL, &cBytes, m_dwFlags), S_OK)

	//Get the Data, ISequentialStream - should actually succeed
	TESTC_(RowsetA.GetStorageData(hRowA, hAccessorA, m_pBuffer, (ULONG*)&cBytes, m_riidStorage, (IUnknown**)&pIUnknownA),S_OK)
	TESTC_(RowsetB.GetStorageData(hRowB, hAccessorB, m_pBuffer2, (ULONG*)&cBytes, m_riidStorage, (IUnknown**)&pIUnknownB),S_OK)

	//Data should be the same, since from the same table
	TESTC(memcmp(m_pBuffer, m_pBuffer2, cBytes)==0)

CLEANUP:
	RowsetA.ReleaseRows(hRowA);
	RowsetB.ReleaseRows(hRowB);

	RowsetA.ReleaseAccessor(hAccessorA);
	RowsetB.ReleaseAccessor(hAccessorB);

	SAFE_RELEASE_(pIUnknownA);
	SAFE_RELEASE_(pIUnknownB);
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(79)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Read::Variation_79()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(80)
//*-----------------------------------------------------------------------
// @mfunc MultiThreads - ::Read from the stream from seperate threads
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Read::Variation_80()
{
	TBEGIN
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	HROW  hRow = NULL;
	
	INIT_THREADS(THREE_THREADS);
	THREADARG T1Arg, T2Arg, T3Arg;

	DBLENGTH cBytes = MAX_PTR;
	DBLENGTH rgcBytesRead[THREE_THREADS] = { 0, 0, 0 };
	IUnknown* pIUnknown = NULL;

	CRowset RowsetA;
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);
	
	//Grab a row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK)
	
	//Create Accessor binding BLOB/Storage Object data
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, NULL, NULL, NULL, m_dwFlags | BLOB_BIND_STR), S_OK)

	//Obtain the Storage interface
	TESTC_(RowsetA.GetStorageData(hRow, hAccessor, NULL, NULL, m_riidStorage, (IUnknown**)&pIUnknown),S_OK)
	
	//Steup threading Arguments
	T1Arg.pFunc = pIUnknown;
	T1Arg.pArg1 = (void*)&m_riidStorage;
	T1Arg.pArg2 = (void*)cBytes;
	T1Arg.pArg3 = &rgcBytesRead[THREAD_ONE];
		
	T2Arg.pFunc = pIUnknown;
	T2Arg.pArg1 = (void*)&m_riidStorage;
	T2Arg.pArg2 = (void*)cBytes;
	T2Arg.pArg3 = &rgcBytesRead[THREAD_TWO];

	T3Arg.pFunc = pIUnknown;
	T3Arg.pArg1 = (void*)&m_riidStorage;
	T3Arg.pArg2 = (void*)cBytes;
	T3Arg.pArg3 = &rgcBytesRead[THREAD_THREE];
	
	//Now call IStorage::Read from Seperate Threads
	CREATE_THREAD(THREAD_ONE, Thread_StorageRead, &T1Arg);
	CREATE_THREAD(THREAD_TWO, Thread_StorageRead, &T2Arg);
	CREATE_THREAD(THREAD_THREE, Thread_StorageRead, &T3Arg);
	
	START_THREADS();
	END_THREADS();	

	//Now verify the results
	if(m_riidStorage == IID_IStream || m_riidStorage == IID_ILockBytes)
	{
		//All should have been able to Read from the begining of the stream...
		TESTC(rgcBytesRead[THREAD_ONE]==rgcBytesRead[THREAD_TWO]);
		TESTC(rgcBytesRead[THREAD_TWO]==rgcBytesRead[THREAD_THREE]);
	}
	else
	{
		//Since the stream should be serialized, 1 thread should have read the
		//entire stream, and the other 2 should have read nothing...
		//This is the case for stream interfaces that are read "sequentially", not 
		if(rgcBytesRead[THREAD_ONE])
			TESTC(rgcBytesRead[THREAD_TWO]==0 && rgcBytesRead[THREAD_THREE]==0);
		if(rgcBytesRead[THREAD_TWO])
			TESTC(rgcBytesRead[THREAD_ONE]==0 && rgcBytesRead[THREAD_THREE]==0);
		if(rgcBytesRead[THREAD_THREE])
			TESTC(rgcBytesRead[THREAD_ONE]==0 && rgcBytesRead[THREAD_TWO]==0);
	}

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor);
	SAFE_RELEASE_(pIUnknown);
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(81)
//*-----------------------------------------------------------------------
// @mfunc MultiThreads - ::Write to the stream from seperate threads
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Read::Variation_81()
{
	TBEGIN
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	HROW  hRow = NULL;
	
	DBLENGTH cBytes = 100;
	DBLENGTH cBytesWrite = 0;
	IUnknown* pIUnknown = NULL;

	INIT_THREADS(THREE_THREADS);
	THREADARG T1Arg;

	CRowset RowsetA;
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);
	
	//Grab a row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK)
	
	//Create Accessor binding BLOB/Storage Object data
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, NULL, NULL, NULL, m_dwFlags | BLOB_BIND_STR), S_OK)

	//Obtain the Storage interface
	TESTC_(RowsetA.GetStorageData(hRow, hAccessor, NULL, NULL, m_riidStorage, (IUnknown**)&pIUnknown),S_OK)
	
	//Steup threading Arguments
	T1Arg.pFunc = pIUnknown;
	T1Arg.pArg1 = (void*)&m_riidStorage;
	T1Arg.pArg2 = (void*)cBytes;
	T1Arg.pArg3 = &cBytesWrite;
	
	//Now call IStorage::Read from Seperate Threads
	CREATE_THREAD(THREAD_ONE, Thread_StorageWrite, &T1Arg);
	CREATE_THREAD(THREAD_TWO, Thread_StorageWrite, &T1Arg);
	CREATE_THREAD(THREAD_THREE, Thread_StorageWrite, &T1Arg);
	
	START_THREADS();
	END_THREADS();	

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor);
	SAFE_RELEASE_(pIUnknown);
	TRETURN;
}
// }}
// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCSeqStream_Read::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCStorage::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCSeqStream_Write)
//*-----------------------------------------------------------------------
//| Test Case:		TCSeqStream_Write - Test Storage Objects with SetData functionality [Read]
//|	Created:			08/21/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCSeqStream_Write::Init()
{
	TBEGIN

	// {{ TCW_INIT_BASECLASS_CHECK
	TESTC(TCStorage::Init());
	// }}
 	
	//Determine if this Storage interface is supported...
	TESTC_PROVIDER(SupportedStorageInterface(DBPROP_IRowsetChange));

CLEANUP:
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc General - Verfiy RefCount, that provider releases storage object
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Write::Variation_1()
{
	TBEGIN
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	HRESULT hr = S_OK;

	HROW  hRow = NULL;
	DBLENGTH cBytes = 0;
	DBCOUNTITEM iTableRow = 0;
	
	DBORDINAL cBindings = 0;
	DBBINDING* rgBindings = NULL;
	CStorage* pCStorage = new CStorage;
	void* pData     = NULL;

	CRowsetChange RowsetA;
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);

	//Grab a row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK)
	
	//Create Accessor binding BLOB/Storage Object data
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, &cBindings, &rgBindings, &cBytes, m_dwFlags), S_OK)

	//Alloc buffers
	TESTC(RowsetA.MakeRowData(&pData,hAccessor,&iTableRow))
	TESTC(SetStorageObject(cBindings, rgBindings, pData, iTableRow, m_riidStorage, pCStorage))

	//SetData, Should call my own ISequentialStream::Read
	//The provider should also release the ISeqStream pointer
	SAFE_ADDREF(pCStorage);
	TESTC_(hr = RowsetA.SetData(hRow, hAccessor, pData),S_OK);

	//The RefCount of pIUnknown was 2 before the call to SetData, so since the 
	//provider should call release the refcount should be 1 after the call
	TESTC(GetRefCount(pCStorage->pUnknown())==1)
	
	//Verify the set storage object data...
	TESTC(CompareStorageBuffer(&RowsetA, hRow, hAccessor, pCStorage));

CLEANUP:
	SAFE_RELEASE(pCStorage);
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor, cBindings, rgBindings, pData);
	SAFE_RELEASE_(pCStorage);
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Write::Variation_2()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Boundary - SetData with 0 buffer size
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Write::Variation_3()
{
	TBEGIN
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	
	HROW  hRow = NULL;
	DBLENGTH cBytes = 0;
	DBCOUNTITEM iTableRow = 0;
	
	void* pData     = NULL;
	CStorage* pCStorage = new CStorage;
	
	IUnknown* pIUnknown = NULL;
	DBLENGTH cbRead = MAX_PTR;
	DBORDINAL cBindings = 0;
	DBBINDING* rgBindings = NULL;

	CRowsetChange RowsetA;
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);

	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);

	//Grab a row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK)

	//Create Accessor binding BLOB/Storage Object data
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, &cBindings, &rgBindings, &cBytes, m_dwFlags | BLOB_BIND_STR), S_OK)

	//Alloc buffers
	TESTC(RowsetA.MakeRowData(&pData,hAccessor,&iTableRow))
	TESTC(SetStorageObject(cBindings, rgBindings, pData, iTableRow, m_riidStorage, pCStorage, DBSTATUS_S_OK, 0))

	//SetData, Should call my own ISequentialStream::Read
	//The provider should also release the ISeqStream pointer
	//Since the data is bound as DBSTATUS_S_OK, but there are 0 bytes it should
	//delete the current columns and empty the BLOB column
	TESTC_(RowsetA.SetData(hRow, hAccessor, pData),S_OK)
    
	//Now doing a GetData should return a ISeqStream pointer of 0 bytes.
	TESTC_(RowsetA.GetStorageData(hRow, hAccessor, NULL, NULL, m_riidStorage, (IUnknown**)&pIUnknown),S_OK)
	TEST2C_(StorageRead(pIUnknown, m_pBuffer, 100, &cbRead),S_OK,S_FALSE)
	TESTC(cbRead == 0)

CLEANUP:
	RowsetA.ReleaseRows(hRow);

	//	RowsetA.ReleaseAccessor(hAccessor, cBindings, rgBindings, pData);
	FreeAccessorBufferAndBindings(&cBindings, &rgBindings, &pData, true);
	RowsetA.pIAccessor()->ReleaseAccessor(hAccessor, NULL);
	SAFE_RELEASE_(pIUnknown);
	TRETURN;
}
// }}

// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Boundary - SetData with small buffer size
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Write::Variation_4()
{
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	
	HROW  hRow = NULL;
	DBLENGTH cBytes = 0;
	DBCOUNTITEM iTableRow = 0;
	DBCOUNTITEM ulIndex = 0;
	DBBINDING *pUnkBinding = NULL;
	
	DBORDINAL cBindings = 0;
	DBBINDING* rgBindings = NULL;

	void* pData     = NULL;
	CStorage* pCStorage = new CStorage;

	CRowsetChange RowsetA;
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);

	//Grab a row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK)
	
	//Create Accessor binding BLOB/Storage Object data
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, &cBindings, &rgBindings, &cBytes, m_dwFlags), S_OK)

	//Alloc buffers
	TESTC(RowsetA.MakeRowData(&pData,hAccessor,&iTableRow))
	pUnkBinding	=	FindBinding(cBindings, rgBindings, DBTYPE_IUNKNOWN, &ulIndex);
	TESTC(pUnkBinding != NULL);

	TESTC(SetStorageObject(cBindings, rgBindings, pData, iTableRow, m_riidStorage, pCStorage, 
		DBSTATUS_S_OK, LENGTH_BINDING(*pUnkBinding, pData)))

	//SetData, Should call my own ISequentialStream::Read
	//The provider should also release the ISeqStream pointer
	TESTC_(RowsetA.SetData(hRow, hAccessor, pData),S_OK)

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor, cBindings, rgBindings, pData);
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Boundary - SetData with large buffer size
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Write::Variation_5()
{
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	
	HROW  hRow = NULL;
	DBLENGTH cBytes = 0;
	DBCOUNTITEM iTableRow = 0;
	
	DBORDINAL cBindings = 0;
	DBBINDING* rgBindings = NULL;

	void* pData     = NULL;
	CStorage* pCStorage = new CStorage;
	
	CRowsetChange RowsetA;
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);

	//Grab a row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK)

	//Create Accessor binding BLOB/Storage Object data
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, &cBindings, &rgBindings, &cBytes, m_dwFlags | BLOB_BIND_STR), S_OK)
	
	//Alloc buffers
	TESTC(RowsetA.MakeRowData(&pData,hAccessor,&iTableRow))
	TESTC(SetStorageObject(cBindings, rgBindings, pData, iTableRow, m_riidStorage, pCStorage, DBSTATUS_S_OK, 0x800))

	//SetData, Should call my own ISequentialStream::Read
	//The provider should also release the ISeqStream pointer
	TESTC_(RowsetA.SetData(hRow, hAccessor, pData),S_OK)

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor, cBindings, rgBindings, pData);
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Boundary - SetData with equal buffer size
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Write::Variation_6()
{
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	
	HROW  hRow = NULL;
	DBLENGTH cBytes = 0;
	DBCOUNTITEM iTableRow = 0;
	
	DBORDINAL cBindings = 0;
	DBBINDING* rgBindings = NULL;
	DBCOUNTITEM ulIndex = 0;

	void* pData     = NULL;
	CStorage* pCStorage = new CStorage;
	
	CRowsetChange RowsetA;
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);

	//Grab a row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK)
	
	//Create Accessor binding BLOB/Storage Object data
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, &cBindings, &rgBindings, &cBytes, m_dwFlags), S_OK)

	//Alloc buffers
	TESTC(RowsetA.MakeRowData(&pData,hAccessor,&iTableRow))
	TESTC(FindBinding(cBindings, rgBindings, DBTYPE_IUNKNOWN, &ulIndex) != NULL);
	TESTC(SetStorageObject(cBindings, rgBindings, pData, iTableRow, m_riidStorage, pCStorage, 
		DBSTATUS_S_OK, LENGTH_BINDING(rgBindings[ulIndex], pData)))

	//SetData, Should call my own ISequentialStream::Read
	//The provider should also release the ISeqStream pointer
	TESTC_(RowsetA.SetData(hRow, hAccessor, pData),S_OK)

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor, cBindings, rgBindings, pData);
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Boundary - SetData with even/odd number of bytes
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Write::Variation_7()
{
	
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	
	HROW  hRow = NULL;
	DBLENGTH cBytes = 0;
	DBCOUNTITEM iTableRow = 0;
	
	DBORDINAL cBindings = 0;
	DBBINDING* rgBindings = NULL;

	void* pData     = NULL;
	CStorage* pCStorage = new CStorage;
	
	CRowsetChange RowsetA;
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);

	//Grab a row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK)
	
	//Create Accessor binding BLOB/Storage Object data
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, &cBindings, &rgBindings, &cBytes, m_dwFlags | BLOB_BIND_STR), S_OK)

	//Alloc buffers
	TESTC(RowsetA.MakeRowData(&pData,hAccessor,&iTableRow))
	TESTC(SetStorageObject(cBindings, rgBindings, pData, iTableRow, m_riidStorage, pCStorage, DBSTATUS_S_OK, 0x402))

	//SetData, Should call my own ISequentialStream::Read
	//The provider should also release the ISeqStream pointer
	TESTC_(RowsetA.SetData(hRow, hAccessor, pData),S_OK)

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor, cBindings, rgBindings, pData);
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Write::Variation_8()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc Boundary - InsertRow with 0 buffer size
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Write::Variation_9()
{
	TBEGIN
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	
	HROW  hRow = NULL;
	DBLENGTH cBytes = 0;
	DBCOUNTITEM iTableRow = 0;
	
	DBORDINAL cBindings = 0;
	DBBINDING* rgBindings = NULL;
	HRESULT hr = S_OK;

	void* pData     = NULL;
	CStorage* pCStorage = new CStorage;
	
	IUnknown* pIUnknown = NULL;
	DBLENGTH cbRead = MAX_PTR;
	
	CRowsetChange RowsetA;
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);

	//Create Accessor binding BLOB/Storage Object data
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, &cBindings, &rgBindings, &cBytes, m_dwFlags), S_OK)

	//Alloc buffers
	TESTC(RowsetA.MakeRowData(&pData,hAccessor,&iTableRow))
	TESTC(SetStorageObject(cBindings, rgBindings, pData, iTableRow, m_riidStorage, pCStorage, DBSTATUS_S_OK, 0))

	//InsertRow, Should call my own ISequentialStream::Read
	//The provider should also release the ISeqStream pointer
	//Since the data is bound as DBSTATUS_S_OK, but there are 0 bytes it should
	//delete the current BLOB and place 0 bytes
	TESTC_(hr = RowsetA.InsertRow(hAccessor,pData,&hRow),S_OK);

	//Now doing a GetData should return a ISeqStream pointer of 0 bytes.
	TESTC_(RowsetA.GetStorageData(hRow, hAccessor, NULL, NULL, m_riidStorage, (IUnknown**)&pIUnknown),S_OK)
	TEST2C_(StorageRead(pIUnknown, m_pBuffer, 100, &cbRead),S_OK,S_FALSE)
	TESTC(cbRead == 0)

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor, cBindings, rgBindings, pData);
	SAFE_RELEASE_(pIUnknown);
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc Boundary - InsertRow with small buffer size
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Write::Variation_10()
{
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	
	HROW  hRow = NULL;
	DBLENGTH cBytes = 0;
	DBCOUNTITEM iTableRow = 0;
	
	DBORDINAL cBindings = 0;
	DBBINDING* rgBindings = NULL;

	void* pData     = NULL;
	CStorage* pCStorage = new CStorage;
	
	CRowsetChange RowsetA;
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);

	//Create Accessor binding BLOB/Storage Object data
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, &cBindings, &rgBindings, &cBytes, m_dwFlags | BLOB_BIND_STR), S_OK)

	//Alloc buffers
	TESTC(RowsetA.MakeRowData(&pData,hAccessor,&iTableRow))
	TESTC(SetStorageObject(cBindings, rgBindings, pData, iTableRow, m_riidStorage, pCStorage, DBSTATUS_S_OK, 10))

	//InsertRow, Should call my own ISequentialStream::Read
	//The provider should also release the ISeqStream pointer
	TESTC_(RowsetA.InsertRow(hAccessor,pData,&hRow),S_OK)

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor, cBindings, rgBindings, pData);
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc Boundary - InsertRow with large buffer size
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Write::Variation_11()
{
	
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	
	HROW  hRow = NULL;
	DBLENGTH cBytes = 0;
	DBCOUNTITEM iTableRow = 0;
	
	DBORDINAL cBindings = 0;
	DBBINDING* rgBindings = NULL;

	void* pData     = NULL;
	CStorage* pCStorage = new CStorage;
	
	CRowsetChange RowsetA;
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);

	//Create Accessor binding BLOB/Storage Object data
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, &cBindings, &rgBindings, &cBytes, m_dwFlags), S_OK)

	//Alloc buffers
	TESTC(RowsetA.MakeRowData(&pData,hAccessor,&iTableRow))
	TESTC(SetStorageObject(cBindings, rgBindings, pData, iTableRow, m_riidStorage, pCStorage, DBSTATUS_S_OK, 0x800))

	//InsertRow, Should call my own ISequentialStream::Read
	//The provider should also release the ISeqStream pointer
	TESTC_(RowsetA.InsertRow(hAccessor,pData,&hRow),S_OK)

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor, cBindings, rgBindings, pData);
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc Boundary - InsertRow with equal buffer size
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Write::Variation_12()
{
	
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	
	HROW  hRow = NULL;
	DBLENGTH cBytes = 0;
	DBCOUNTITEM iTableRow = 0;
	
	DBORDINAL cBindings = 0;
	DBBINDING* rgBindings = NULL;

	void* pData     = NULL;
	CStorage* pCStorage = new CStorage;
	
	CRowsetChange RowsetA;
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);

	//Create Accessor binding BLOB/Storage Object data
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, &cBindings, &rgBindings, &cBytes, m_dwFlags | BLOB_BIND_STR), S_OK)

	//Alloc buffers
	TESTC(RowsetA.MakeRowData(&pData,hAccessor,&iTableRow))
	TESTC(SetStorageObject(cBindings, rgBindings, pData, iTableRow, m_riidStorage, pCStorage, DBSTATUS_S_OK, 0x400))

	//InsertRow, Should call my own ISequentialStream::Read
	//The provider should also release the ISeqStream pointer
	TESTC_(RowsetA.InsertRow(hAccessor,pData,&hRow),S_OK)

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor, cBindings, rgBindings, pData);
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc Boundary - InsertRow with even/odd number of bytes
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Write::Variation_13()
{
	
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	
	HROW  hRow = NULL;
	DBLENGTH cBytes = 0;
	DBCOUNTITEM iTableRow = 0;
	
	DBORDINAL cBindings = 0;
	DBBINDING* rgBindings = NULL;

	void* pData     = NULL;
	CStorage* pCStorage = new CStorage;
	
	CRowsetChange RowsetA;
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);

	//Create Accessor binding BLOB/Storage Object data
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, &cBindings, &rgBindings, &cBytes, m_dwFlags), S_OK)

	//Alloc buffers
	TESTC(RowsetA.MakeRowData(&pData,hAccessor,&iTableRow))
	TESTC(SetStorageObject(cBindings, rgBindings, pData, iTableRow, m_riidStorage, pCStorage, DBSTATUS_S_OK, 0x402))

	//InsertRow, Should call my own ISequentialStream::Read
	//The provider should also release the ISeqStream pointer
	TESTC_(RowsetA.InsertRow(hAccessor,pData,&hRow),S_OK)

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor, cBindings, rgBindings, pData);
	TRETURN;
}
// }}




// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSeqStream_Write::Variation_14()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc Parameters - SetData with NULL pObject
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Write::Variation_15()
{
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	HROW  hRow = NULL;
	void* pData     = NULL;
	
	CRowsetChange RowsetA;
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);

	//Grab a row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK)
	
	//Create Accessor binding BLOB/Storage Object data
	//According to the 2.1 OLE DB Spec pObject == NULL is equivalent to setting pObject->iid == IID_IUnknown
	//Retriving IUnknown binding is useful since it doesn't require knowing ahead of time what
	//type of objects the provider supports ahead of time, and is useful for services to just
	//pass object instances to other services which actually to the reading, etc
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, NULL, NULL, NULL, BLOB_NULL_POBJECT), S_OK)

	//Alloc buffers
	TESTC(RowsetA.MakeRowData(&pData, hAccessor));

	//SetData, Should call my own ISequentialStream::Read
	//The provider should also release the ISeqStream pointer
	TESTC_(RowsetA.SetData(hRow, hAccessor, pData),S_OK)

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseRowData(pData, hAccessor);
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc Parameters - SetData with NULL interface pointer
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Write::Variation_16()
{
	//Passing NULL for SetData ISeqStrem object
	TBEGIN
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	HROW  hRow = NULL;
	DBLENGTH cBytes = 0;
	DBCOUNTITEM iTableRow = 0;

	DBORDINAL cBindings = 0;
	DBBINDING* rgBindings = NULL;

	void* pData     = NULL;
	DBLENGTH cbRead = 1000;
	IUnknown* pIUnknown = NULL;

	CRowsetChange RowsetA;
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);
	
	//Grab a row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK)
	
	//Create Accessor binding BLOB/Storage Object data
	//Only Bind the BLOB Column, since if we bind the index column and do a
	//SetData on it, GetData will fail on SQLServer since it is unable to position
	//on the row now that it may have been moved...
	TESTC_(GetAccessorAndBindings(RowsetA.pIAccessor(), DBACCESSOR_ROWDATA, &hAccessor,
		&rgBindings, &cBindings, &cBytes, DBPART_ALL,BLOB_COLS_BOUND,FORWARD,
		NO_COLS_BY_REF,NULL,NULL,NULL,DBTYPE_EMPTY,0,NULL,NULL,
		NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, m_dwFlags),S_OK)

	//Alloc buffers
	TESTC(RowsetA.MakeRowData(&pData,hAccessor,&iTableRow))
	TESTC(SetStorageObject(cBindings, rgBindings, pData, iTableRow, IID_IUnknown, NULL))
	
	//SetData, Should call ISequentialStream::Read
	//Should delete the current BLOB column
	TESTC_(RowsetA.SetData(hRow, hAccessor, pData),S_OK)

	//Calling GetData should return a valid storage object, with no data
	TESTC_(RowsetA.GetStorageData(hRow, hAccessor, NULL, NULL, m_riidStorage, (IUnknown**)&pIUnknown),S_OK)
    
	TESTC(pIUnknown != NULL)

	//Now make sure the buffer is empty
	TEST2C_(StorageRead(pIUnknown, m_pBuffer, cbRead, &cbRead),S_OK,S_FALSE)
	TESTC(cbRead == 0)

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor, cBindings, rgBindings, pData);
	SAFE_RELEASE_(pIUnknown);
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc Parameters - SetData with DBSTATUS_S_ISNULL storage column
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Write::Variation_17()
{
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	HROW  hRow = NULL;
	
	DBORDINAL cBindings = 0;
	DBBINDING* rgBindings = NULL;
	void* pData  = NULL;

	CRowsetChange RowsetA;
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);

	//Grab a row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK)
	
	//Create Accessor binding BLOB/Storage Object data
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, &cBindings, &rgBindings, NULL, m_dwFlags), S_OK)

	//Alloc buffers
	TESTC(RowsetA.MakeRowData(&pData,hAccessor))

	//Set the StorageObject to ISNULL...
	TESTC(SetStorageObject(cBindings, rgBindings, pData, 0, m_riidStorage, NULL, DBSTATUS_S_ISNULL));

	//SetData, should not look at the stream object (ISNULL)
	TESTC_(RowsetA.SetData(hRow, hAccessor, pData),S_OK)

	//GetData on the NULL storage column, should return DBSTATUS_S_ISNULL
	TESTC_(RowsetA.GetData(hRow, hAccessor, pData),S_OK)
	TESTC(VerifyBindingStatus(cBindings, rgBindings, pData, DBTYPE_IUNKNOWN, DBSTATUS_S_ISNULL))

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor, cBindings, rgBindings, pData);
	TRETURN;
}
// }}




// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc Parameters - SetData with DBSTATUS_S_IGNORE storage column
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSeqStream_Write::Variation_18()
{ 
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	HROW  hRow = NULL;
	
	DBORDINAL cBindings = 0;
	DBBINDING* rgBindings = NULL;
	void* pData  = NULL;

	CRowsetChange RowsetA;
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);

	//Grab a row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK)
	
	//Create Accessor binding BLOB/Storage Object data
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, &cBindings, &rgBindings, NULL, m_dwFlags), S_OK)

	//Alloc buffers
	TESTC(RowsetA.MakeRowData(&pData,hAccessor))

	//Set the StorageObject to IGNORE...
	TESTC(SetStorageObject(cBindings, rgBindings, pData, 0, m_riidStorage, NULL, DBSTATUS_S_IGNORE));

	//SetData, Should ignore the stream, and just set the other columns
	TESTC_(RowsetA.SetData(hRow, hAccessor, pData),S_OK)

	//GetData on the storage column
	TEST2C_(RowsetA.GetData(hRow, hAccessor, pData),S_OK,DB_S_ERRORSOCCURRED)
	//TODO: Verify the data of all other columns were changed, except Stream should not have changed

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor, cBindings, rgBindings, pData);
	TRETURN;
} 
// }} TCW_VAR_PROTOTYPE_END






// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc Parameters - SetData with DBSTATUS_S_DEFAULT storage column
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSeqStream_Write::Variation_19()
{ 
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	HROW  hRow = NULL;
	
	DBORDINAL cBindings = 0;
	DBBINDING* rgBindings = NULL;
	void* pData  = NULL;

	CRowsetChange RowsetA;
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);

	//Grab a row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK)
	
	//Create Accessor binding BLOB/Storage Object data
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, &cBindings, &rgBindings, NULL, m_dwFlags), S_OK)

	//Alloc buffers
	TESTC(RowsetA.MakeRowData(&pData,hAccessor))

	//Set the StorageObject to DEFAULT...
	TESTC(SetStorageObject(cBindings, rgBindings, pData, 0, m_riidStorage, NULL, DBSTATUS_S_DEFAULT));

	//SetData, Should ignore the stream, and just set it to the default
	TESTC_(RowsetA.SetData(hRow, hAccessor, pData),S_OK)

	//GetData on the storage column
	TESTC_(RowsetA.GetData(hRow, hAccessor, pData),S_OK)
	//TODO: Verify the data of all other columns were changed, except Stream should not have changed

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor, cBindings, rgBindings, pData);
	TRETURN;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc Parameters - SetData with Invalid DBSTATUS
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSeqStream_Write::Variation_20()
{ 
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	HROW  hRow = NULL;
	
	DBORDINAL i,cBindings = 0;
	DBBINDING* rgBindings = NULL;
	void* pData  = NULL;

	CRowsetChange RowsetA;
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);

	//Grab a row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK)
	
	//Create Accessor binding BLOB/Storage Object data
	//NOTE: Only bind BLOB columns, so we dont have to refill all the column status and data back
	//to DBSTATUS_S_OK inside the loop...
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, &cBindings, &rgBindings, NULL, m_dwFlags, NULL, BLOB_COLS_BOUND), S_OK)

	//Alloc buffers
	TESTC(RowsetA.MakeRowData(&pData,hAccessor))

	//Loop over the possible invalid status'...
	for(i=0; i<NUMELEM(rgInvalidStatus); i++)
	{
		DBSTATUS dwStatus = rgInvalidStatus[i];
		
		//Store the invalid status in the status binding...
		TESTC(SetStorageObject(cBindings, rgBindings, pData, 0, m_riidStorage, NULL, dwStatus));

		//SetData, should fail with BADSTATUS for the Stream column
		TESTC_(RowsetA.SetData(hRow, hAccessor, pData), DB_E_ERRORSOCCURRED);
		VerifyBindingStatus(cBindings, rgBindings, pData, DBTYPE_IUNKNOWN, DBSTATUS_E_BADSTATUS);

		//TODO:  Verify SetData/GetData on the other columns
	}

	//TODO: Do interative status's as well, so we don't have providers hard coding
	//to the ones in our list.

	//TODO: Since some provider may just not release on these errors, we need to make sure
	//that when successfully passed a stream and an error occurs by the provider,
	//that they release it, since it will end up with the same status as the above for loop
	//but for a completely different reason of which they need to release the object
	//and not just error on it...

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor, cBindings, rgBindings, pData);
	TRETURN;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSeqStream_Write::Variation_21()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(22)
//*-----------------------------------------------------------------------
// @mfunc Parameters - SetData twice [same row]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Write::Variation_22()
{
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	
	HROW  hRow = NULL;
	DBLENGTH cBytes = 0;
	DBCOUNTITEM iTableRow = 0;
	
	DBORDINAL cBindings = 0;
	DBBINDING* rgBindings = NULL;

	void* pData     = NULL;
	CStorage* pCStorage = new CStorage;
	
	CRowsetChange RowsetA;
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);

	//Grab a row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK)
	
	//Create Accessor binding BLOB/Storage Object data
	//Only Bind the BLOB Column, since if we bind the index column and do a
	//SetData on it, the second SetData GetData will fail on SQLServer 
	//since it is unable to position on the row now that it may have been moved...
	TESTC_(GetAccessorAndBindings(RowsetA.pIAccessor(), DBACCESSOR_ROWDATA, &hAccessor,
		&rgBindings, &cBindings, &cBytes, DBPART_ALL,BLOB_COLS_BOUND,FORWARD,
		NO_COLS_BY_REF,NULL,NULL,NULL,DBTYPE_EMPTY,0,NULL,NULL,
		NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, m_dwFlags),S_OK)

	//Alloc buffers
	TESTC(RowsetA.MakeRowData(&pData,hAccessor,&iTableRow))
	TESTC(SetStorageObject(cBindings, rgBindings, pData, iTableRow, m_riidStorage, pCStorage))

	//SetData, Should call my own ISequentialStream::Read
	//The provider should also release the ISeqStream pointer
	SAFE_ADDREF(pCStorage);
	TESTC_(RowsetA.SetData(hRow, hAccessor, pData),S_OK)

	//The provider should also release the ISeqStream pointer
	//Need to reset the Stream
	TESTC_(pCStorage->Seek(0),S_OK);
	TESTC_(RowsetA.SetData(hRow, hAccessor, pData),S_OK)

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor, cBindings, rgBindings, pData);
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(23)
//*-----------------------------------------------------------------------
// @mfunc Parameters - SetData twice [with 2 open storage objects]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Write::Variation_23()
{
	
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	
	HROW  hRow = NULL;
	DBLENGTH cBytes = 0;
	DBCOUNTITEM iTableRow = 0;
	
	DBORDINAL cBindings = 0;
	DBBINDING* rgBindings = NULL;

	void* pData     = NULL;
	CStorage* pCStorage = new CStorage;
	
	CRowsetChange RowsetA;
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);

	//Grab a row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK)
	
	//Create Accessor binding BLOB/Storage Object data
	//Only Bind the BLOB Column, since if we bind the index column and do a
	//SetData on it, the second SetData GetData will fail on SQLServer 
	//since it is unable to position on the row now that it may have been moved...
	TESTC_(GetAccessorAndBindings(RowsetA.pIAccessor(), DBACCESSOR_ROWDATA, &hAccessor,
		&rgBindings, &cBindings, &cBytes, DBPART_ALL,BLOB_COLS_BOUND,FORWARD,
		NO_COLS_BY_REF,NULL,NULL,NULL,DBTYPE_EMPTY,0,NULL,NULL,
		NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, m_dwFlags),S_OK)

	//Alloc buffers
	TESTC(RowsetA.MakeRowData(&pData,hAccessor,&iTableRow))
	TESTC(SetStorageObject(cBindings, rgBindings, pData, iTableRow, m_riidStorage, pCStorage))

	//SetData, Should call my own ISequentialStream::Read
	//The provider should also release the ISeqStream pointer
	SAFE_ADDREF(pCStorage);
	TESTC_(RowsetA.SetData(hRow, hAccessor, pData),S_OK)

	//The provider should also release the ISeqStream pointer
	//Need to reset the Stream
	TESTC_(pCStorage->Seek(0),S_OK);
	TESTC_(RowsetA.SetData(hRow, hAccessor, pData),S_OK)

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor, cBindings, rgBindings, pData);
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(24)
//*-----------------------------------------------------------------------
// @mfunc Parameters - SetData twice [diff rows]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Write::Variation_24()
{
	
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	
	HROW  rghRows[TWO_ROWS] = {NULL,NULL};
	DBLENGTH cBytes = 0;
	DBCOUNTITEM iTableRow = 0;
	
	void* pData     = NULL;
	void* pData2    = NULL;
	CStorage* pCStorage1 = new CStorage;
	CStorage* pCStorage2 = new CStorage;
	
	DBORDINAL cBindings = 0;
	DBBINDING* rgBindings = NULL;

	CRowsetChange RowsetA;
	RowsetA.SetProperty(DBPROP_CANHOLDROWS);
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);

	//Grab a row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &rghRows[ROW_ONE]),S_OK)
	
	//Create Accessor binding BLOB/Storage Object data
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, &cBindings, &rgBindings, &cBytes, m_dwFlags), S_OK)

	//Alloc buffers
	TESTC(RowsetA.MakeRowData(&pData,hAccessor,&iTableRow))
	TESTC(SetStorageObject(cBindings, rgBindings, pData, iTableRow, m_riidStorage, pCStorage1))
	
	TESTC(RowsetA.MakeRowData(&pData2,hAccessor,&iTableRow))
	TESTC(SetStorageObject(cBindings, rgBindings, pData2, iTableRow, m_riidStorage, pCStorage2))

	//SetData, Should call my own ISequentialStream::Read
	//The provider should also release the ISeqStream pointer
	TESTC_(RowsetA.SetData(rghRows[ROW_ONE], hAccessor, pData),S_OK)

	//Grab the next row
	TESTC_(RowsetA.GetNextRows(&rghRows[ROW_TWO]),S_OK)
	
	//The provider should also release the ISeqStream pointer
	TESTC_(RowsetA.SetData(rghRows[ROW_TWO], hAccessor, pData2),S_OK)

CLEANUP:
	RowsetA.ReleaseRows(TWO_ROWS,rghRows);
	RowsetA.ReleaseRowData(pData2, hAccessor);
	RowsetA.ReleaseAccessor(hAccessor, cBindings, rgBindings, pData);
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(25)
//*-----------------------------------------------------------------------
// @mfunc Parameters - SetData twice [with 2 open storage objects] diff rows
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Write::Variation_25()
{
	
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	
	HROW  rghRows[TWO_ROWS] = {NULL,NULL};
	DBLENGTH cBytes = 0;
	DBCOUNTITEM iTableRow = 0;
	
	void* pData     = NULL;
	void* pData2    = NULL;
	CStorage* pCStorage = new CStorage;

	DBORDINAL cBindings = 0;
	DBBINDING* rgBindings = NULL;

	CRowsetChange RowsetA;
	RowsetA.SetProperty(DBPROP_CANHOLDROWS);
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);

	//Grab a row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &rghRows[ROW_ONE]),S_OK)
	
	//Create Accessor binding BLOB/Storage Object data
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, &cBindings, &rgBindings, &cBytes, m_dwFlags | BLOB_BIND_STR), S_OK)

	//Alloc buffers
	TESTC(RowsetA.MakeRowData(&pData,hAccessor,&iTableRow))
	TESTC(SetStorageObject(cBindings, rgBindings, pData, iTableRow, m_riidStorage, pCStorage))
	pCStorage->Clear();
	
	TESTC(RowsetA.MakeRowData(&pData2,hAccessor,&iTableRow))
	TESTC(SetStorageObject(cBindings, rgBindings, pData2, iTableRow, m_riidStorage, pCStorage))

	//SetData, Should call my own ISequentialStream::Read
	//The provider should also release the ISeqStream pointer
	SAFE_ADDREF(pCStorage);
	TESTC_(RowsetA.SetData(rghRows[ROW_ONE], hAccessor, pData),S_OK)

	//Grab the next row
	TESTC_(RowsetA.GetNextRows(&rghRows[ROW_TWO]),S_OK)
	
	//The provider should also release the ISeqStream pointer
	//Need to reset the Stream
	TESTC_(pCStorage->Seek(0),S_OK);
	TESTC_(RowsetA.SetData(rghRows[ROW_TWO], hAccessor, pData2),S_OK)

CLEANUP:
	RowsetA.ReleaseRows(TWO_ROWS,rghRows);
	RowsetA.ReleaseRowData(pData2, hAccessor);
	RowsetA.ReleaseAccessor(hAccessor, cBindings, rgBindings, pData);
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(26)
//*-----------------------------------------------------------------------
// @mfunc Parameters - SetData -> GetData
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Write::Variation_26()
{
	TBEGIN

	//Passing Valid ISeqStrem object to SetData
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	
	HROW  hRow = NULL;
	DBLENGTH cBytes = 0;
	DBCOUNTITEM iTableRow = 0;
	
	DBORDINAL cBindings = 0;
	DBBINDING* rgBindings = NULL;

	void* pData     = NULL;
	CStorage* pCStorage = new CStorage;
	IUnknown* pIUnknown2 = NULL;
	
	CRowsetChange RowsetA;
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);

	//Grab a row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK)
	
	//Create Accessor binding BLOB/Storage Object data
	//Only Bind the BLOB Column, since if we bind the index column and do a
	//SetData on it, the second SetData GetData will fail on SQLServer 
	//since it is unable to position on the row now that it may have been moved...
	TESTC_(GetAccessorAndBindings(RowsetA.pIAccessor(), DBACCESSOR_ROWDATA, &hAccessor,
		&rgBindings, &cBindings, &cBytes, DBPART_ALL,BLOB_COLS_BOUND,FORWARD,
		NO_COLS_BY_REF,NULL,NULL,NULL,DBTYPE_EMPTY,0,NULL,NULL,
		NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, m_dwFlags),S_OK)

	//Alloc buffers
	TESTC(RowsetA.MakeRowData(&pData,hAccessor,&iTableRow))
	TESTC(SetStorageObject(cBindings, rgBindings, pData, iTableRow, m_riidStorage, pCStorage))

	//SetData, Should call my own ISequentialStream::Read
	//The provider should also release the ISeqStream pointer
	TESTC_(RowsetA.SetData(hRow, hAccessor, pData),S_OK);

	//Now make sure the data matches the backend
	TESTC_(RowsetA.GetStorageData(hRow, hAccessor, NULL, NULL, m_riidStorage, (IUnknown**)&pIUnknown2),S_OK)

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor, cBindings, rgBindings, pData);
	SAFE_RELEASE_(pIUnknown2);
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(27)
//*-----------------------------------------------------------------------
// @mfunc Parameters - SetData -> GetData [diff rows]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Write::Variation_27()
{
	TBEGIN
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	HRESULT hr = S_OK;

	HROW  rghRows[TWO_ROWS] = {NULL,NULL};
	DBLENGTH cBytes = 0;
	DBCOUNTITEM iTableRow = 0;
	
	DBORDINAL cBindings = 0;
	DBBINDING* rgBindings = NULL;
	IUnknown* pIUnknown2 = NULL;

	void* pData     = NULL;
	CStorage* pCStorage = new CStorage;
	
	CRowsetChange RowsetA;
	RowsetA.SetProperty(DBPROP_CANHOLDROWS);
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);

	//Grab a row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &rghRows[ROW_ONE]),S_OK)
	
	//Create Accessor binding BLOB/Storage Object data
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, &cBindings, &rgBindings, &cBytes, m_dwFlags | BLOB_BIND_STR), S_OK)

	//Alloc buffers
	TESTC(RowsetA.MakeRowData(&pData,hAccessor,&iTableRow))
	TESTC(SetStorageObject(cBindings, rgBindings, pData, iTableRow, m_riidStorage, pCStorage))

	//SetData, Should call my own ISequentialStream::Read
	//The provider should also release the ISeqStream pointer
	TESTC_(RowsetA.SetData(rghRows[ROW_ONE], hAccessor, pData),S_OK)

	//Grab the next row handle
	TESTC_(RowsetA.GetNextRows(&rghRows[ROW_TWO]),S_OK)

	//Now call GetData on a different row handle
	//A different row may have data or maybe NULL depending 
	//upon previous variations, so expect either S_OK (valid) or DB_E_NOTFOUND (NULL)
	hr = RowsetA.GetStorageData(rghRows[ROW_TWO], hAccessor, NULL, NULL, m_riidStorage, (IUnknown**)&pIUnknown2);
	TEST2C_(hr, S_OK, DB_E_NOTFOUND);

CLEANUP:
	SAFE_RELEASE_(pIUnknown2);
	RowsetA.ReleaseRows(TWO_ROWS,rghRows);
	RowsetA.ReleaseAccessor(hAccessor, cBindings, rgBindings, pData);
	TRETURN;
}
// }}




// {{ TCW_VAR_PROTOTYPE(28)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSeqStream_Write::Variation_28()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(29)
//*-----------------------------------------------------------------------
// @mfunc Parameters - InsertRow with NULL pObject
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Write::Variation_29()
{
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	HROW  rghRows[TWO_ROWS] = {NULL,NULL};
	void* pData     = NULL;

	CRowsetChange RowsetA;
	RowsetA.SetProperty(DBPROP_CANHOLDROWS);
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);

	//Create Accessor binding BLOB/Storage Object data
	//According to the 2.1 OLE DB Spec pObject == NULL is equivalent to setting pObject->iid == IID_IUnknown
	//Retriving IUnknown binding is useful since it doesn't require knowing ahead of time what
	//type of objects the provider supports ahead of time, and is useful for services to just
	//pass object instances to other services which actually to the reading, etc
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, NULL, NULL, NULL, BLOB_NULL_POBJECT), S_OK)

	//Alloc buffers
	TESTC(RowsetA.MakeRowData(&pData, hAccessor));

	//InsertRow, Should call my own ISequentialStream::Read
	//The provider should also release the ISeqStream pointer
	TESTC_(RowsetA.InsertRow(hAccessor, pData, &rghRows[ROW_ONE]),S_OK)

CLEANUP:
	RowsetA.ReleaseRows(TWO_ROWS, rghRows);
	RowsetA.ReleaseRowData(pData, hAccessor);
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(30)
//*-----------------------------------------------------------------------
// @mfunc Parameters - InsertRow with NULL interface pointer
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Write::Variation_30()
{
	TBEGIN
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	
	HROW  hRow = NULL;
	DBLENGTH cBytes = 0;
	DBCOUNTITEM iTableRow = 0;

	DBORDINAL cBindings = 0;
	DBBINDING* rgBindings = NULL;
	HRESULT hr = S_OK;

	void* pData     = NULL;
	IUnknown* pIUnknown = NULL;

	CRowsetChange RowsetA;
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);
	
	//Create Accessor binding BLOB/Storage Object data
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, &cBindings, &rgBindings, &cBytes, m_dwFlags), S_OK)

	//Alloc buffers
	TESTC(RowsetA.MakeRowData(&pData,hAccessor,&iTableRow))
	TESTC(SetStorageObject(cBindings, rgBindings, pData, iTableRow, IID_IUnknown, NULL))
	
	//InsertRow, will NULL object, Should delete the current BLOB column
	TESTC_(hr = RowsetA.InsertRow( hAccessor, pData, &hRow),S_OK)

	//Calling GetData should return a valid storage object, with no data
	TESTC_(RowsetA.GetStorageData(hRow, hAccessor, NULL, NULL, m_riidStorage, (IUnknown**)&pIUnknown),S_OK)

	TESTC(pIUnknown != NULL)

	//Now make sure the storage object contains no data...
	TEST2C_(StorageRead(pIUnknown, m_pBuffer, 10, &cBytes),S_OK,S_FALSE)
	TESTC(cBytes == 0)

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor, cBindings, rgBindings, pData);
	SAFE_RELEASE_(pIUnknown);
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(31)
//*-----------------------------------------------------------------------
// @mfunc Parameters - InsertRow with DBSTATUS_S_ISNULL storage column
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Write::Variation_31()
{
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	HROW  hRow = NULL;

	DBORDINAL cBindings = 0;
	DBBINDING* rgBindings = NULL;
	void* pData     = NULL;

	CRowsetChange RowsetA;
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);

	//Create Accessor binding BLOB/Storage Object data
	//Need to bind the Index Column...
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, &cBindings, &rgBindings, NULL, m_dwFlags), S_OK)

	//Alloc buffers
	TESTC(RowsetA.MakeRowData(&pData,hAccessor))

	//Set the StorageObject to ISNULL...
	TESTC(SetStorageObject(cBindings, rgBindings, pData, 0, m_riidStorage, NULL, DBSTATUS_S_ISNULL));

	//InsertRow, Should call set NULL data for the BLOB
	TESTC_(RowsetA.InsertRow( hAccessor, pData, &hRow),S_OK);

	//GetData on the NULL storage column, should return DBSTATUS_S_ISNULL
	TESTC_(RowsetA.GetData(hRow, hAccessor, pData),S_OK)
	TESTC(VerifyBindingStatus(cBindings, rgBindings, pData, DBTYPE_IUNKNOWN, DBSTATUS_S_ISNULL))

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor, cBindings, rgBindings, pData);
	TRETURN;
}
// }}




// {{ TCW_VAR_PROTOTYPE(32)
//*-----------------------------------------------------------------------
// @mfunc Parameters - InsertRow with DBSTATUS_S_IGNORE storage column
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSeqStream_Write::Variation_32()
{ 
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	HROW  hRow = NULL;

	DBORDINAL cBindings = 0;
	DBBINDING* rgBindings = NULL;
	void* pData     = NULL;

	CRowsetChange RowsetA;
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);

	//Create Accessor binding BLOB/Storage Object data
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, &cBindings, &rgBindings, NULL, m_dwFlags), S_OK)

	//Alloc buffers
	TESTC(RowsetA.MakeRowData(&pData,hAccessor))

	//Set the StorageObject to IGNORE...
	TESTC(SetStorageObject(cBindings, rgBindings, pData, 0, m_riidStorage, NULL, DBSTATUS_S_IGNORE));

	//InsertRow, Should just ignore stream
	TESTC_(RowsetA.InsertRow( hAccessor, pData, &hRow),S_OK);

	//GetData
	TESTC_(RowsetA.GetData(hRow, hAccessor, pData),S_OK)
	//TODO: Verify all other columns are inserted, and default or NULL for stream

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor, cBindings, rgBindings, pData);
	TRETURN;
} 
// }} TCW_VAR_PROTOTYPE_END






// {{ TCW_VAR_PROTOTYPE(33)
//*-----------------------------------------------------------------------
// @mfunc Parameters - InsertRow with DBSTATUS_S_DEFAULT storage column
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSeqStream_Write::Variation_33()
{ 
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	HROW  hRow = NULL;

	DBORDINAL cBindings = 0;
	DBBINDING* rgBindings = NULL;
	void* pData     = NULL;

	CRowsetChange RowsetA;
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);

	//Create Accessor binding BLOB/Storage Object data
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, &cBindings, &rgBindings, NULL, m_dwFlags), S_OK)

	//Alloc buffers
	TESTC(RowsetA.MakeRowData(&pData,hAccessor))

	//Set the StorageObject to DEFAULT...
	TESTC(SetStorageObject(cBindings, rgBindings, pData, 0, m_riidStorage, NULL, DBSTATUS_S_DEFAULT));

	//InsertRow, Should just ignore stream
	TESTC_(RowsetA.InsertRow( hAccessor, pData, &hRow),S_OK);

	//GetData
	TESTC_(RowsetA.GetData(hRow, hAccessor, pData),S_OK)
	//TODO: Verify all other columns are inserted, and default for stream

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor, cBindings, rgBindings, pData);
	TRETURN;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(34)
//*-----------------------------------------------------------------------
// @mfunc Parameters - InsertRow with Invalid DBSTATUS
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSeqStream_Write::Variation_34()
{ 
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	HROW  hRow = NULL;
	
	DBORDINAL i,cBindings = 0;
	DBBINDING* rgBindings = NULL;
	void* pData  = NULL;

	CRowsetChange RowsetA;
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);

	//Grab a row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK)
	
	//Create Accessor binding BLOB/Storage Object data
	//NOTE: have to bind all columns, since some might be non-null or not have a default...
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, &cBindings, &rgBindings, NULL, m_dwFlags), S_OK)

	//Loop over the possible invalid status'...
	for(i=0; i<NUMELEM(rgInvalidStatus); i++)
	{
		DBSTATUS dwStatus = rgInvalidStatus[i];
		
		//Alloc buffers
		TESTC(RowsetA.MakeRowData(&pData,hAccessor))

		//Store the invalid status in the status binding...
		TESTC(SetStorageObject(cBindings, rgBindings, pData, 0, m_riidStorage, NULL, dwStatus));

		//InsertRow, should fail with BADSTATUS for the Stream column
		//Provider might be "all-or-nothing" so also allow DB_E_ERRORSOCCURRED
		TEST2C_(RowsetA.InsertRow( hAccessor, pData, NULL), DB_S_ERRORSOCCURRED, DB_E_ERRORSOCCURRED);
		VerifyBindingStatus(cBindings, rgBindings, pData, DBTYPE_IUNKNOWN, DBSTATUS_E_BADSTATUS);

		//TODO:  Verify SetData/GetData on the other columns
	}

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor, cBindings, rgBindings, pData);
	TRETURN;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(35)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSeqStream_Write::Variation_35()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(36)
//*-----------------------------------------------------------------------
// @mfunc Parameters - InsertRow 2 rows
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Write::Variation_36()
{
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	
	HROW  rghRows[TWO_ROWS] = {NULL,NULL};
	DBLENGTH cBytes = 0;
	DBCOUNTITEM iTableRow = 0;
	
	void* pData     = NULL;
	void* pData2    = NULL;
	CStorage* pCStorage1 = new CStorage;
	CStorage* pCStorage2 = new CStorage;
	
	DBORDINAL cBindings = 0;
	DBBINDING* rgBindings = NULL;

	CRowsetChange RowsetA;
	RowsetA.SetProperty(DBPROP_CANHOLDROWS);
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);

	//Create Accessor binding BLOB/Storage Object data
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, &cBindings, &rgBindings, &cBytes, m_dwFlags), S_OK)

	//Alloc buffers
	TESTC(RowsetA.MakeRowData(&pData,hAccessor,&iTableRow))
	TESTC(SetStorageObject(cBindings, rgBindings, pData, iTableRow, m_riidStorage, pCStorage1))

	TESTC(RowsetA.MakeRowData(&pData2,hAccessor,&iTableRow))
	TESTC(SetStorageObject(cBindings, rgBindings, pData2, iTableRow, m_riidStorage, pCStorage2))

	//InsertRow, Should call my own ISequentialStream::Read
	//The provider should also release the ISeqStream pointer
	TESTC_(RowsetA.InsertRow( hAccessor, pData, &rghRows[ROW_ONE]),S_OK)

	//The provider should also release the ISeqStream pointer
	TESTC_(RowsetA.InsertRow( hAccessor, pData2, &rghRows[ROW_TWO]),S_OK)

CLEANUP:
	RowsetA.ReleaseRows(TWO_ROWS,rghRows);
	RowsetA.ReleaseRowData(pData,hAccessor);
	RowsetA.ReleaseAccessor(hAccessor, cBindings, rgBindings, pData2);
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(37)
//*-----------------------------------------------------------------------
// @mfunc Parameters - InsertRow 2 rows [with 2 open storage objects]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Write::Variation_37()
{
	
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	
	HROW  rghRows[TWO_ROWS] = {NULL,NULL};
	DBLENGTH cBytes = 0;
	DBCOUNTITEM iTableRow = 0;
	
	void* pData     = NULL;
	void* pData2    = NULL;
	CStorage* pCStorage = new CStorage;
	
	DBORDINAL cBindings = 0;
	DBBINDING* rgBindings = NULL;

	CRowsetChange RowsetA;
	RowsetA.SetProperty(DBPROP_CANHOLDROWS);
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);

	//Create Accessor binding BLOB/Storage Object data
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, &cBindings, &rgBindings, &cBytes, m_dwFlags | BLOB_BIND_STR), S_OK)

	//Alloc buffers
	TESTC(RowsetA.MakeRowData(&pData,hAccessor,&iTableRow))
	TESTC(SetStorageObject(cBindings, rgBindings, pData, iTableRow, m_riidStorage, pCStorage))
	pCStorage->Clear();

	TESTC(RowsetA.MakeRowData(&pData2,hAccessor,&iTableRow))
	TESTC(SetStorageObject(cBindings, rgBindings, pData2, iTableRow, m_riidStorage, pCStorage))

	//InsertRow, Should call my own ISequentialStream::Read
	//The provider should also release the ISeqStream pointer
	SAFE_ADDREF(pCStorage);
	TESTC_(RowsetA.InsertRow( hAccessor, pData, &rghRows[ROW_ONE]),S_OK)

	//The provider should also release the ISeqStream pointer
	//Need to reset the Stream
	TESTC_(pCStorage->Seek(0),S_OK);
	TESTC_(RowsetA.InsertRow( hAccessor, pData2, &rghRows[ROW_TWO]),S_OK)

CLEANUP:
	RowsetA.ReleaseRows(TWO_ROWS,rghRows);
	RowsetA.ReleaseRowData(pData2,hAccessor);
	RowsetA.ReleaseAccessor(hAccessor, cBindings, rgBindings, pData);
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(38)
//*-----------------------------------------------------------------------
// @mfunc Parameters - InsertRow -> GetData
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Write::Variation_38()
{
	TBEGIN
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	
	HROW  hRow = NULL;
	DBLENGTH cBytes = 0;
	DBCOUNTITEM iTableRow = 0;
	
	void* pData     = NULL;
	CStorage* pCStorage = new CStorage;
	
	DBORDINAL cBindings = 0;
	DBBINDING* rgBindings = NULL;
	IUnknown* pIUnknown2 = NULL;

	CRowsetChange RowsetA;
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);

	//Create Accessor binding BLOB/Storage Object data
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, &cBindings, &rgBindings, &cBytes, m_dwFlags), S_OK)

	//Alloc buffers
	TESTC(RowsetA.MakeRowData(&pData,hAccessor,&iTableRow))
	TESTC(SetStorageObject(cBindings, rgBindings, pData, iTableRow, m_riidStorage, pCStorage))

	//InsertRow, Should call my own ISequentialStream::Read
	//The provider should also release the ISeqStream pointer
	TESTC_(RowsetA.InsertRow( hAccessor, pData, &hRow),S_OK);

	//Calling GetData should return a valid storage object, with no data
	TESTC(hRow != DB_NULL_HROW)
	//Now make sure InsertRow matches GetData
	TESTC_(RowsetA.GetStorageData(hRow, hAccessor, NULL, NULL, m_riidStorage, (IUnknown**)&pIUnknown2),S_OK)

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor, cBindings, rgBindings, pData);
	SAFE_RELEASE_(pIUnknown2);
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(39)
//*-----------------------------------------------------------------------
// @mfunc Parameters - InsertRow -> SetData
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Write::Variation_39()
{
	TBEGIN
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	
	HROW  hRow = NULL;
	DBLENGTH cBytes = 0;
	DBCOUNTITEM iTableRow = 0;
	
	void* pData     = NULL;
	void* pData2    = NULL;

	DBORDINAL cBindings = 0;
	DBBINDING* rgBindings = NULL;
	CStorage* pCStorage = new CStorage;
	HRESULT hr = S_OK;

	CRowsetChange RowsetA;
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);

	//Create Accessor binding BLOB/Storage Object data
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, &cBindings, &rgBindings, &cBytes, m_dwFlags | BLOB_BIND_STR), S_OK)

	//Alloc buffers
	TESTC(RowsetA.MakeRowData(&pData,hAccessor,&iTableRow))
	TESTC(SetStorageObject(cBindings, rgBindings, pData, iTableRow, m_riidStorage, pCStorage))
	pCStorage->Clear();

	TESTC(RowsetA.MakeRowData(&pData2,hAccessor,&iTableRow))
	TESTC(SetStorageObject(cBindings, rgBindings, pData2, iTableRow, m_riidStorage, pCStorage))

	//InsertRow, Should call my own ISequentialStream::Read
	//The provider should also release the ISeqStream pointer
	SAFE_ADDREF(pCStorage);
	TESTC_(RowsetA.InsertRow( hAccessor, pData, &hRow),S_OK)
	TESTC(hRow != DB_NULL_HROW)

	//Now make sure SetData can be called on a newly inserted row
	//Need to reset the Stream, and addref so we can verify the refcount on return
	SAFE_ADDREF(pCStorage);
	TESTC_(pCStorage->Seek(0),S_OK);
	hr = RowsetA.SetData(hRow, hAccessor, pData2);
	TEST2C_(hr, S_OK, DB_E_NEWLYINSERTED);
	
	//Some providers may not be able to position on newly inserted rows
	if(hr==DB_E_NEWLYINSERTED)
	{
		//Make sure they indicate they cannot position on newly inserted rows...
		TESTC(RowsetA.GetProperty(DBPROP_CHANGEINSERTEDROWS, DBPROPSET_ROWSET, VARIANT_FALSE));
	}

CLEANUP:
	//NOTE: We AddRef'd an extra time to verify refcount...
	SAFE_RELEASE_(pCStorage);

	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseRowData(pData2,hAccessor);
	RowsetA.ReleaseAccessor(hAccessor, cBindings, rgBindings, pData);
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(40)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Write::Variation_40()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(41)
//*-----------------------------------------------------------------------
// @mfunc Related - IAccessor - Bind LENGTH with more bytes than in buffer
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Write::Variation_41()
{
	
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	
	HROW  hRow = NULL;
	DBLENGTH cBytes = 0;
	DBCOUNTITEM iTableRow = 0;
	DBORDINAL ulBinding = 0;

	DBORDINAL cBindings = 0;
	DBBINDING* rgBindings = NULL;

	void* pData     = NULL;
	CStorage* pCStorage = new CStorage;
	
	CRowsetChange RowsetA;
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);

	//Grab a row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK)
	
	//Create Accessor binding BLOB/Storage Object data
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, &cBindings, &rgBindings, &cBytes, m_dwFlags), S_OK)

	//Alloc buffers
	TESTC(RowsetA.MakeRowData(&pData,hAccessor,&iTableRow))
	
	//SetStorage object to have 1/2 bytes
	TESTC(SetStorageObject(cBindings, rgBindings, pData, iTableRow, m_riidStorage, pCStorage, DBSTATUS_S_OK, 0x400/2))
	//Now make the LENGTH binding have all bytes
	TESTC(FindBinding(cBindings, rgBindings, DBTYPE_IUNKNOWN, &ulBinding) != NULL)
	LENGTH_BINDING(rgBindings[ulBinding],pData) = 0x400;

	//SetData, Should call my own ISequentialStream::Read
	//The provider should also release the ISeqStream pointer
	//Provider/Driver shouldn't pay any attention to the LENGTH binding
	//Should read the Stream to the end
	
	//Some providers according to the spec, may require LENGTH bound.
	//So we either need a way to determine if they need length bound,
	//Or we have to have enough data in the Storage Object as is indicated
	//by the LENGTH binding, to be a general tests...
	//Until we have a way of determining if LENGTH is required we will bind
	//LENGTH to be no larger than the actual length of the stream...
	LENGTH_BINDING(rgBindings[ulBinding],pData) = 0x400/2;
	TESTC_(RowsetA.SetData(hRow, hAccessor, pData),S_OK)

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor, cBindings, rgBindings, pData);
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(42)
//*-----------------------------------------------------------------------
// @mfunc Related - IAccessor - Bind LENGTH with less bytes than in buffer
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Write::Variation_42()
{
	
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	
	HROW  hRow = NULL;
	DBLENGTH cBytes = 0;
	DBCOUNTITEM iTableRow = 0;
	DBORDINAL ulBinding = 0;

	DBORDINAL cBindings = 0;
	DBBINDING* rgBindings = NULL;

	void* pData     = NULL;
	CStorage* pCStorage = new CStorage;
	
	CRowsetChange RowsetA;
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);

	//Grab a row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK)
	
	//Create Accessor binding BLOB/Storage Object data
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, &cBindings, &rgBindings, &cBytes, m_dwFlags | BLOB_BIND_STR), S_OK)

	//Alloc buffers
	TESTC(RowsetA.MakeRowData(&pData,hAccessor,&iTableRow))
	
	//SetStorage object to have all bytes bytes
	TESTC(SetStorageObject(cBindings, rgBindings, pData, iTableRow, m_riidStorage, pCStorage, DBSTATUS_S_OK, 0x400))

	//Now make the LENGTH binding have 1/2 bytes
	//TODO: BUG We need to come up with a better way to "truncate" the length, since for MBCS
	//characters this could be incorrectly split arcross lead bytes, causing an error.  Will depend
	//upon what the seed is for the row generation, but could easily happen...
	TESTC(FindBinding(cBindings, rgBindings, DBTYPE_IUNKNOWN, &ulBinding) != NULL)
	LENGTH_BINDING(rgBindings[ulBinding],pData) = 0x400/2;

	//SetData, Should call my own ISequentialStream::Read
	//The provider should also release the ISeqStream pointer
	//Should be able to put less in the table than is in the Storage Object
	TESTC_(RowsetA.SetData(hRow, hAccessor, pData),S_OK)

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor, cBindings, rgBindings, pData);
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(43)
//*-----------------------------------------------------------------------
// @mfunc Related - IAccessor - SetData for Storage column, GetData binding as long
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Write::Variation_43()
{
	TBEGIN
	HACCESSOR hAccessor1;
	HACCESSOR hAccessor2;
	
	HROW  hRow = NULL;
	DBLENGTH cBytes = 0;
	DBCOUNTITEM iTableRow = 0;
	
	void* pData1     = NULL;
	void* pData2     = NULL;

	DBORDINAL cBindings1 = 0;
	DBBINDING* rgBindings1 = NULL;
	DBORDINAL cBindings2 = 0;
	DBBINDING* rgBindings2 = NULL;
	
	CStorage* pCStorage = new CStorage;
	SAFE_ADDREF(pCStorage); //AddRef so we have it arround after SetData
	CRowsetChange RowsetA;
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);

	//Grab a row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK)
	
	//Create Accessor binding BLOB/Storage Object data
	//Only Bind the BLOB Column, since if we bind the index column and do a
	//SetData on it, the GetData will fail on SQLServer since it is unable 
	//to position on the row now that it may have been moved...
	TESTC_(GetAccessorAndBindings(RowsetA.pIAccessor(), DBACCESSOR_ROWDATA, &hAccessor1,
		&rgBindings1, &cBindings1, &cBytes, DBPART_ALL,BLOB_COLS_BOUND,FORWARD,
		NO_COLS_BY_REF,NULL,NULL,NULL,DBTYPE_EMPTY,0,NULL,NULL,
		NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, m_dwFlags),S_OK)
	
	//Create Accessor binding BLOB as long data
	TESTC_(GetAccessorAndBindings(RowsetA.pIAccessor(), DBACCESSOR_ROWDATA, &hAccessor2,
		&rgBindings2, &cBindings2, &cBytes, DBPART_ALL,BLOB_COLS_BOUND,FORWARD,
		NO_COLS_BY_REF,NULL,NULL,NULL,DBTYPE_EMPTY,0,NULL,NULL,
		NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, BLOB_LONG),S_OK)

	//Alloc buffers
	TESTC(RowsetA.MakeRowData(&pData1,hAccessor1,&iTableRow))
	TESTC(SetStorageObject(cBindings1, rgBindings1, pData1, iTableRow, m_riidStorage, pCStorage))
	SAFE_ALLOC(pData2, void*, cBytes);

	//SetData, Should call my own ISequentialStream::Read
	//The provider should also release the ISeqStream pointer
	TESTC_(RowsetA.SetData(hRow, hAccessor1, pData1),S_OK)

	//Now try to GetData on the storage column, but binding as long data
	TESTC_(RowsetA.GetData(hRow, hAccessor2, pData2),S_OK)
	
	//Now we know exactly what data was set in the stream, and we have the 
	//data returned from GetData bound as LONG, so now compare the two buffers
	TESTC_(pCStorage->Seek(0),S_OK);
	TESTC_(pCStorage->Read(m_pBuffer, sizeof(void*)*DATA_SIZE, (ULONG*)&cBytes),S_FALSE);
	TESTC(cBytes != 0 && cBytes<sizeof(void*)*DATA_SIZE);
	TESTC(memcmp(m_pBuffer, (BYTE*)pData2+rgBindings2[0].obValue, cBytes)==0);

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor1,cBindings1, rgBindings1, pData1);
	RowsetA.ReleaseAccessor(hAccessor2,cBindings2, rgBindings2, pData2);
	SAFE_RELEASE_(pCStorage);
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(44)
//*-----------------------------------------------------------------------
// @mfunc Related - IAccessor - SetData NULL Storage column, GetData binding as long DBSTATUS_S_ISNULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Write::Variation_44()
{
	TBEGIN
	HACCESSOR hAccessor1;
	HACCESSOR hAccessor2;
	
	HROW  hRow = NULL;
	DBLENGTH cBytes = 0;
	
	void* pData1  = NULL;
	void* pData2  = NULL;
	
	DBORDINAL cBindings1 = 0;
	DBBINDING* rgBindings1 = NULL;
	DBORDINAL cBindings2 = 0;
	DBBINDING* rgBindings2 = NULL;
	IUnknown* pIUnknown = NULL;

	CRowsetChange RowsetA;
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);

	//Grab a row handle
	TESTC_(RowsetA.GetRow(FIFTH_ROW, &hRow),S_OK)
	
	//Create Accessor binding BLOB/Storage Object data
	TESTC_(GetAccessorAndBindings(RowsetA.pIAccessor(), DBACCESSOR_ROWDATA, &hAccessor1,
		&rgBindings1, &cBindings1, &cBytes, DBPART_STATUS,BLOB_COLS_BOUND,FORWARD,
		NO_COLS_BY_REF,NULL,NULL,NULL,DBTYPE_EMPTY,0,NULL,NULL,
		NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, m_dwFlags),S_OK)

	//Create Accessor binding BLOB as long data
	TESTC_(GetAccessorAndBindings(RowsetA.pIAccessor(), DBACCESSOR_ROWDATA, &hAccessor2,
		&rgBindings2, &cBindings2, &cBytes, DBPART_STATUS,BLOB_COLS_BOUND,FORWARD,
		NO_COLS_BY_REF,NULL,NULL,NULL,DBTYPE_EMPTY,0,NULL,NULL,
		NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, BLOB_LONG),S_OK)
	
	//Alloc buffers
	TESTC(RowsetA.MakeRowData(&pData1,hAccessor1))
	SAFE_ALLOC(pData2, void*, cBytes);

	//Bind only status as DBSTATUS_S_ISNULL
	TESTC(SetStorageObject(cBindings1, rgBindings1, pData1, 0, m_riidStorage, NULL, DBSTATUS_S_ISNULL));

	//SetData should use NULL for stream
	TESTC_(RowsetA.SetData(hRow, hAccessor1, pData1),S_OK)

	//GetData on the BLOB Column binding as LONG
	TESTC_(RowsetA.GetData(hRow, hAccessor2, pData2),S_OK)
	TESTC(STATUS_BINDING(rgBindings2[0], pData2) == DBSTATUS_S_ISNULL)

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor1, cBindings1, rgBindings1, pData1);
	RowsetA.ReleaseAccessor(hAccessor2, cBindings2, rgBindings2, pData2);
	SAFE_RELEASE_(pIUnknown);
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(45)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Write::Variation_45()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(46)
//*-----------------------------------------------------------------------
// @mfunc Related - IRowsetResynch - Modify / Resynch
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Write::Variation_46()
{
	
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	
	HROW  hRow = NULL;
	DBLENGTH cBytes = 0;
	DBCOUNTITEM iTableRow = 0;
	
	DBORDINAL cBindings = 0;
	DBBINDING* rgBindings = NULL;

	void* pData     = NULL;
	CStorage* pCStorage = new CStorage;
	
	CRowsetChange RowsetA;
	RowsetA.SetProperty(DBPROP_IRowsetResynch);
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_PROVIDER(RowsetA.CreateRowset(m_dwStorageID)==S_OK);

	//Grab a row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK)
	
	//Create Accessor binding BLOB/Storage Object data
	//Only Bind the BLOB Column, since if we bind the index column and do a
	//SetData on it, the GetData will fail on SQLServer since it is unable 
	//to position on the row now that it may have been moved...
	TESTC_(GetAccessorAndBindings(RowsetA.pIAccessor(), DBACCESSOR_ROWDATA, &hAccessor,
		&rgBindings, &cBindings, &cBytes, DBPART_ALL,BLOB_COLS_BOUND,FORWARD,
		NO_COLS_BY_REF,NULL,NULL,NULL,DBTYPE_EMPTY,0,NULL,NULL,
		NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, m_dwFlags),S_OK)

	//Alloc buffers
	TESTC(RowsetA.MakeRowData(&pData,hAccessor,&iTableRow))
	TESTC(SetStorageObject(cBindings, rgBindings, pData, iTableRow, m_riidStorage, pCStorage))

	//SetData, Should call my own ISequentialStream::Read
	//The provider should also release the ISeqStream pointer
	TESTC_(RowsetA.SetData(hRow, hAccessor, pData),S_OK)

	//Resynch all the rows
	TESTC_(RowsetA.ResynchRows(0,NULL),S_OK)

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor, cBindings, rgBindings, pData);
	TRETURN;
}
// }}

// {{ TCW_VAR_PROTOTYPE(47)
//*-----------------------------------------------------------------------
// @mfunc Related - IRowsetResynch - Insert / Resynch
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Write::Variation_47()
{
	TBEGIN
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	HRESULT hr;
	DBROWSTATUS* rgRowStatus = NULL;

	HROW  hRow = NULL;
	DBLENGTH cBytes = 0;
	DBCOUNTITEM iTableRow = 0;
	
	DBORDINAL cBindings = 0;
	DBBINDING* rgBindings = NULL;

	void* pData     = NULL;
	CStorage* pCStorage = new CStorage;
	
	CRowsetChange RowsetA;
	RowsetA.SetProperty(DBPROP_IRowsetResynch);
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_PROVIDER(RowsetA.CreateRowset(m_dwStorageID)==S_OK);

	//Create Accessor binding BLOB/Storage Object data
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, &cBindings, &rgBindings, &cBytes, m_dwFlags), S_OK)

	//Alloc buffers
	TESTC(RowsetA.MakeRowData(&pData,hAccessor,&iTableRow))
	TESTC(SetStorageObject(cBindings, rgBindings, pData, iTableRow, m_riidStorage, pCStorage))

	//InsertRow, Should call my own ISequentialStream::Read
	//The provider should also release the ISeqStream pointer
	SAFE_ADDREF(pCStorage);
	TESTC_(RowsetA.InsertRow( hAccessor, pData, &hRow),S_OK)

	//Resynch all the rows
	hr = RowsetA.ResynchRows(ONE_ROW,&hRow,NULL,NULL,&rgRowStatus);

	//For NewlyInserted Rows, Resynch can return DBROWSTATUS_E_NEWLYINSERTED
	//if DBPROP_STRONGIDENTITY is VARIANT_FALSE
	if(RowsetA.GetProperty(DBPROP_STRONGIDENTITY, DBPROPSET_ROWSET)==VARIANT_FALSE)
	{
		TESTC_(hr, DB_E_ERRORSOCCURRED);
		TESTC(rgRowStatus && rgRowStatus[0]==DBROWSTATUS_E_NEWLYINSERTED);
	}
	else
	{
		TESTC_(hr, S_OK);
	}


CLEANUP:
	//NOTE: We AddRef'd an extra time to verify refcount...
	SAFE_RELEASE_(pCStorage);

	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor, cBindings, rgBindings, pData);
	PROVIDER_FREE(rgRowStatus);
	TRETURN;
}
// }}

// {{ TCW_VAR_PROTOTYPE(48)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Write::Variation_48()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(49)
//*-----------------------------------------------------------------------
// @mfunc Related - DeleteRows - Delete a row with an open storage object
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Write::Variation_49()
{
	TBEGIN
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	HRESULT hr = S_OK;
	
	HROW  hRow = NULL;
	DBLENGTH cBytes = 0;
	DBCOUNTITEM iTableRow = 0;
	
	DBORDINAL cBindings = 0;
	DBBINDING* rgBindings = NULL;

	void* pData     = NULL;
	IUnknown* pIUnknown = NULL;
	
	CRowsetChange RowsetA;
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);

	//Grab a row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK)

	//Create Accessor binding BLOB/Storage Object data
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, &cBindings, &rgBindings, &cBytes, m_dwFlags), S_OK)

	//GetData for this row
	SAFE_ALLOC(pData, void*, cBytes);
	TESTC_(RowsetA.GetData(hRow, hAccessor, pData),S_OK)
	TESTC(GetStorageObject(cBindings,rgBindings,pData,m_riidStorage, (IUnknown**)&pIUnknown))

	//Try and delete the row, that has an open storage object
	hr = RowsetA.DeleteRow(ONE_ROW, &hRow);
	TEST2C_(hr, S_OK, E_UNEXPECTED);
	if(hr == E_UNEXPECTED)
	{
		//Should only return an error if BLOCKINGSTORAGEOBJECTS
		TESTC(RowsetA.GetProperty(DBPROP_BLOCKINGSTORAGEOBJECTS));
	}

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor, cBindings, rgBindings, pData);
	SAFE_RELEASE_(pIUnknown);
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(50)
//*-----------------------------------------------------------------------
// @mfunc Related - DeleteRows - Modify / DeleteRows
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Write::Variation_50()
{
	
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	
	HROW  hRow = NULL;
	DBLENGTH cBytes = 0;
	DBCOUNTITEM iTableRow = 0;
	
	DBORDINAL cBindings = 0;
	DBBINDING* rgBindings = NULL;

	void* pData     = NULL;
	CStorage* pCStorage = new CStorage;
	
	CRowsetChange RowsetA;
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);

	//Grab a row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK)
	
	//Create Accessor binding BLOB/Storage Object data
	//Only Bind the BLOB Column, since if we bind the index column and do a
	//SetData on it, the GetData will fail on SQLServer since it is unable 
	//to position on the row now that it may have been moved...
	TESTC_(GetAccessorAndBindings(RowsetA.pIAccessor(), DBACCESSOR_ROWDATA, &hAccessor,
		&rgBindings, &cBindings, &cBytes, DBPART_ALL,BLOB_COLS_BOUND,FORWARD,
		NO_COLS_BY_REF,NULL,NULL,NULL,DBTYPE_EMPTY,0,NULL,NULL,
		NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, m_dwFlags),S_OK)

	//Alloc buffers
	TESTC(RowsetA.MakeRowData(&pData,hAccessor,&iTableRow))
	TESTC(SetStorageObject(cBindings, rgBindings, pData, iTableRow, m_riidStorage, pCStorage))

	//SetData, Should call my own ISequentialStream::Read
	//The provider should also release the ISeqStream pointer
	TESTC_(RowsetA.SetData(hRow, hAccessor, pData),S_OK)

	//Try and delete the Modified row
	TESTC_(RowsetA.DeleteRow(ONE_ROW, &hRow),S_OK)

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor, cBindings, rgBindings, pData);
	TableInsert(ONE_ROW);	//Adjust the table
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(51)
//*-----------------------------------------------------------------------
// @mfunc Related - DeleteRows - Insert / DeleteRows
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Write::Variation_51()
{
	TBEGIN
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	
	HROW  hRow = NULL;
	DBLENGTH cBytes = 0;
	DBCOUNTITEM iTableRow = 0;
	
	DBORDINAL cBindings = 0;
	DBBINDING* rgBindings = NULL;
	DBROWSTATUS dwRowStatus = 0;
	HRESULT hr = S_OK;

	void* pData     = NULL;
	CStorage* pCStorage = new CStorage;
	
	CRowsetChange RowsetA;
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);

	//Create Accessor binding BLOB/Storage Object data
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, &cBindings, &rgBindings, &cBytes, m_dwFlags), S_OK)

	//Alloc buffers
	TESTC(RowsetA.MakeRowData(&pData,hAccessor,&iTableRow))
	TESTC(SetStorageObject(cBindings, rgBindings, pData, iTableRow, m_riidStorage, pCStorage))

	//InsertRow, Should call my own ISequentialStream::Read
	//The provider should also release the ISeqStream pointer
	SAFE_ADDREF(pCStorage);
	TESTC_(RowsetA.InsertRow( hAccessor, pData, &hRow),S_OK)

	//Try and delete the Inserted row
	hr = RowsetA.DeleteRow(ONE_ROW, &hRow, &dwRowStatus);
	TEST2C_(hr, S_OK, DB_E_ERRORSOCCURRED);

	//Provider may not be able to position on newly inserted rows...
	if(hr==DB_E_ERRORSOCCURRED)
	{
		//Status must indicate the correct problem...
		TESTC(dwRowStatus == DBROWSTATUS_E_NEWLYINSERTED);

		//Must indicate they cannot position on newly inserted rows...
		TESTC(RowsetA.GetProperty(DBPROP_CHANGEINSERTEDROWS, DBPROPSET_ROWSET, VARIANT_FALSE));
	}

CLEANUP:
	//NOTE: We AddRef'd an extra time to verify refcount...
	SAFE_RELEASE_(pCStorage);

	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor, cBindings, rgBindings, pData);
	TableInsert(ONE_ROW);	//Adjust the table
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(52)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Write::Variation_52()
{
	return TEST_PASS;
}
// }}




// {{ TCW_VAR_PROTOTYPE(53)
//*-----------------------------------------------------------------------
// @mfunc Sequence - GetData - SetData - GetData - Verify
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSeqStream_Write::Variation_53()
{ 
	TBEGIN
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	HRESULT hr = S_OK;

	HROW  hRow = NULL;
	DBLENGTH cBytes = 0;
	DBCOUNTITEM iTableRow = 0;
	
	DBORDINAL cBindings = 0;
	DBBINDING* rgBindings = NULL;
	CStorage* pCStorage = new CStorage;
	IUnknown* pIUnknown = NULL;
	void* pData     = NULL;

	CRowsetChange RowsetA;
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);

	//Grab a row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK)
	
	//Create Accessor binding BLOB/Storage Object data
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, 
		&cBindings, &rgBindings, &cBytes, m_dwFlags, NULL, UPDATEABLE_NONINDEX_COLS_BOUND), S_OK)

	//Obtain the storage object first...
	//Some providers have shared memory, so just the fact of getting a stream, reading, and releasing
	//causes them to setup state.  So the next time its retrived they incorrectly fail...
	//Test this sequence of Get - Release - Set - Get - S_OK
	TESTC_(RowsetA.GetStorageData(hRow, hAccessor, NULL, NULL, m_riidStorage, (IUnknown**)&pIUnknown),S_OK)
	SAFE_RELEASE(pIUnknown);
	
	//Alloc buffers
	TESTC(RowsetA.MakeRowData(&pData,hAccessor,&iTableRow))
	TESTC(SetStorageObject(cBindings, rgBindings, pData, iTableRow, m_riidStorage, pCStorage))

	//SetData, Should call my own ISequentialStream::Read
	//The provider should also release the ISeqStream pointer
	SAFE_ADDREF(pCStorage);
	TESTC_(hr = RowsetA.SetData(hRow, hAccessor, pData),S_OK);

	//The RefCount of pIUnknown was 2 before the call to SetData, so since the 
	//provider should call release the refcount should be 1 after the call
	TESTC(GetRefCount(pCStorage->pUnknown())==1)
	
	//Verify the set storage object data...
	TESTC(CompareStorageBuffer(&RowsetA, hRow, hAccessor, pCStorage));

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor, cBindings, rgBindings, pData);
	SAFE_RELEASE_(pCStorage);
	SAFE_RELEASE(pIUnknown);
	TRETURN;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(54)
//*-----------------------------------------------------------------------
// @mfunc Sequence - GetData - DeleteRow - GetData - Verify
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSeqStream_Write::Variation_54()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(55)
//*-----------------------------------------------------------------------
// @mfunc Sequence - GetData - InsertRow - GetData - Verify
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSeqStream_Write::Variation_55()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(56)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Write::Variation_56()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(57)
//*-----------------------------------------------------------------------
// @mfunc MultiUser - 2 rowsets, SetData on 1, InsertRow on the other
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Write::Variation_57()
{
	
	HACCESSOR hAccessorA;
	HACCESSOR hAccessorB;
	
	HROW  hRowA = NULL;
	HROW  hRowB = NULL;
	DBLENGTH cBytes = 0;
	DBCOUNTITEM iTableRow = 0;

	void* pDataA    = NULL;
	void* pDataB    = NULL;

	DBORDINAL cBindingsA = 0;
	DBBINDING* rgBindingsA = NULL;
	DBORDINAL cBindingsB = 0;
	DBBINDING* rgBindingsB = NULL;

	CRowsetChange RowsetA;
	CRowsetChange RowsetB;
	
	CStorage* pCStorage = new CStorage;

	//Rowsets
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);
	RowsetB.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetB.CreateRowset(m_dwStorageID),S_OK);
		
	//Grab a row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hRowA),S_OK)
	
	//Create Accessor binding BLOB/Storage Object data
	TESTC_(RowsetA.CreateAccessor(&hAccessorA,DBACCESSOR_ROWDATA,DBPART_ALL,&cBindingsA, &rgBindingsA, &cBytes, m_dwFlags), S_OK)
	TESTC_(RowsetB.CreateAccessor(&hAccessorB,DBACCESSOR_ROWDATA,DBPART_ALL,&cBindingsB, &rgBindingsB, &cBytes, m_dwFlags | BLOB_BIND_STR), S_OK)

	//Alloc buffers
	TESTC(RowsetA.MakeRowData(&pDataA,hAccessorA,&iTableRow))
	TESTC(SetStorageObject(cBindingsA, rgBindingsA, pDataA, iTableRow, m_riidStorage, pCStorage))
	pCStorage->Clear();
	
	TESTC(RowsetB.MakeRowData(&pDataB,hAccessorB,&iTableRow))
	TESTC(SetStorageObject(cBindingsB, rgBindingsB, pDataB, iTableRow, m_riidStorage, pCStorage))

	//SetData on one rowset
	SAFE_ADDREF(pCStorage);
	TESTC_(RowsetA.SetData(hRowA, hAccessorA, pDataA),S_OK)

	//InsertRow on the other
	//Need to reset the stream
	TESTC_(pCStorage->Seek(0),S_OK);
	TESTC_(RowsetB.InsertRow( hAccessorB, pDataB, &hRowB),S_OK)

CLEANUP:
	RowsetA.ReleaseRows(hRowA);
	RowsetB.ReleaseRows(hRowB);

	RowsetA.ReleaseAccessor(hAccessorA, cBindingsA, rgBindingsA, pDataA);
	RowsetB.ReleaseAccessor(hAccessorB, cBindingsB, rgBindingsB, pDataB);
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(58)
//*-----------------------------------------------------------------------
// @mfunc Boundary - SetData with not direct interface
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSeqStream_Write::Variation_58()
{ 
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	
	HROW  hRow = NULL;
	DBLENGTH cBytes = 0;
	DBCOUNTITEM iTableRow = 0;
	
	void* pData     = NULL;
	CStorage* pCStorage = new CStorage;
	DBORDINAL cBindings = 0;
	DBBINDING* rgBindings = NULL;
	DBORDINAL ulBinding = 0;
	IUnknown* pIUnknown = (ILockBytes*)pCStorage;
	if(m_riidStorage==IID_ILockBytes)
		pIUnknown = (IStream*)pCStorage;

	CRowsetChange RowsetA;
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);

	//Grab a row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK)

	//Create Accessor binding BLOB/Storage Object data
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, &cBindings, &rgBindings, &cBytes, m_dwFlags | BLOB_BIND_STR), S_OK)

	//Alloc buffers
	TESTC(RowsetA.MakeRowData(&pData,hAccessor,&iTableRow))
	TESTC(SetStorageObject(cBindings, rgBindings, pData, iTableRow, m_riidStorage, pCStorage))

	//Set a non-Storage object, to make sure the provider is not referencing this
	//pointer directly, as with any object a QI should occur.  This is also for
	//consumers that implement their object with multiple interface iheritence
	//you will be passed an object implmenting the interface, but not neccesarrly
	//the exact interface to directly dereference.
	TESTC(FindBinding(cBindings, rgBindings, DBTYPE_IUNKNOWN, &ulBinding) != NULL)
	VALUE_BINDING(rgBindings[ulBinding], pData) = pIUnknown;

	//Passing a storage object, and it implement the interface in pObject->iid
	//but this is the object and must have QI called to obtain the correct interface
	TESTC_(RowsetA.SetData(hRow, hAccessor, pData),S_OK)

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor, cBindings, rgBindings, pData);
	TRETURN;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(59)
//*-----------------------------------------------------------------------
// @mfunc Boundary - SetData with non-Storage interface
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSeqStream_Write::Variation_59()
{ 
	TBEGIN
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	
	HROW  hRow = NULL;
	DBLENGTH cBytes = 0;
	DBCOUNTITEM iTableRow = 0;
	
	void* pData     = NULL;
	DBORDINAL cBindings = 0;
	DBBINDING* rgBindings = NULL;
	DBORDINAL ulBinding = 0;
	CDispatch* pCDispatch = new CDispatch;

	CRowsetChange RowsetA;
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);

	//Grab a row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK)

	//Create Accessor binding BLOB/Storage Object data
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, &cBindings, &rgBindings, &cBytes, m_dwFlags | BLOB_BIND_STR), S_OK)

	//Alloc buffers
	TESTC(RowsetA.MakeRowData(&pData,hAccessor,&iTableRow))
	TESTC(SetStorageObject(cBindings, rgBindings, pData, iTableRow, m_riidStorage, NULL))

	//Set a non-Storage object, to make sure the provider is not referencing this
	//pointer directly, as with any object a QI should occur.  This is also for
	//consumers that implement their object with multiple interface iheritence
	//you will be passed an object implmenting the interface, but not neccesarrly
	//the exact interface to directly dereference.
	SAFE_ADDREF(pCDispatch);
	TESTC(FindBinding(cBindings, rgBindings, DBTYPE_IUNKNOWN, &ulBinding) != NULL)
	VALUE_BINDING(rgBindings[ulBinding], pData) = pCDispatch;

	//NOTE:  We are using pObject->iid = Storage interface, and passing
	//a pointer to an interface, but it doesn't implement the storage interface
	TEST2C_(RowsetA.SetData(hRow, hAccessor, pData),DB_S_ERRORSOCCURRED,DB_E_ERRORSOCCURRED)
	TESTC(VerifyBindingStatus(cBindings, rgBindings, pData, DBTYPE_IUNKNOWN, DBSTATUS_E_CANTCREATE));

CLEANUP:
	//NOTE: We AddRef'd an extra time to verify refcount...
	SAFE_RELEASE_(pCDispatch);

	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor, cBindings, rgBindings, pData);
	TRETURN;
} 
// }} TCW_VAR_PROTOTYPE_END






// {{ TCW_VAR_PROTOTYPE(60)
//*-----------------------------------------------------------------------
// @mfunc Boundary - SetData verify objects released on error
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSeqStream_Write::Variation_60()
{ 
	TBEGIN
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	
	HROW  hRow = NULL;
	DBLENGTH cBytes = 0;
	DBCOUNTITEM iTableRow = 0;
	
	void* pData     = NULL;
	DBORDINAL i,cBindings = 0;
	DBBINDING* rgBindings = NULL;
	CStorage* pCStorage = new CStorage;

	CRowsetChange RowsetA;
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);

	//Grab a row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK)

	//Create Accessor binding BLOB/Storage Object data
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, &cBindings, &rgBindings, &cBytes, m_dwFlags | BLOB_BIND_STR), S_OK)

	//Loop over all columns creating errors.
	//This was we get testing of errors before as well as after the stream object
	for(i=0; i<cBindings; i++)
	{
		DBBINDING* pBinding = &rgBindings[i];
		if(pBinding->wType != DBTYPE_IUNKNOWN)
		{
			//Alloc buffers
			TESTC(RowsetA.MakeRowData(&pData,hAccessor,&iTableRow))
			TESTC(SetStorageObject(cBindings, rgBindings, pData, iTableRow, m_riidStorage, pCStorage))

			//Try and cause an error with one of the other columns (not the stream column) to verify
			//the provider releases the stream on error...
			//The simplest way to do this (has to be a runtime issue, otherwise createaccess would fail)
			//is to make one of the status' on input not be valid.
			DBSTATUS dbStatus = STATUS_BINDING(*pBinding, pData);
			STATUS_BINDING(*pBinding, pData) = DBSTATUS_E_SCHEMAVIOLATION;

			//NOTE:  We are using pObject->iid = Storage interface, and passing
			//a pointer to an interface, but it doesn't implement the storage interface
			SAFE_ADDREF(pCStorage);
			TESTC_(pCStorage->Seek(0),S_OK);
			TEST2C_(RowsetA.SetData(hRow, hAccessor, pData),DB_S_ERRORSOCCURRED,DB_E_ERRORSOCCURRED)

			//Verify the provider flagged the column and status as incorrect
			TESTC(STATUS_BINDING(*pBinding, pData) == DBSTATUS_E_BADSTATUS);
		
			//Verify the storage object was released
			TESTC(GetRefCount(pCStorage->pUnknown()) == 1);
			
			//Restore the status binding so we can continue testing other columns in error....
			STATUS_BINDING(*pBinding, pData) = dbStatus;
			QTESTC_(ReleaseInputBindingsMemory(cBindings, rgBindings, (BYTE*)pData, TRUE),S_OK)
			pData = NULL;
		}
	}

CLEANUP:
	//NOTE: We AddRef'd an extra time to verify refcount...
	SAFE_RELEASE_(pCStorage);

	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor, cBindings, rgBindings, pData);
	TRETURN;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(61)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSeqStream_Write::Variation_61()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(62)
//*-----------------------------------------------------------------------
// @mfunc Boundary -InsertRow with not direct interface
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSeqStream_Write::Variation_62()
{ 
	TBEGIN
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	
	HROW  hRow = NULL;
	DBLENGTH cBytes = 0;
	DBCOUNTITEM iTableRow = 0;
	
	void* pData     = NULL;
	CStorage* pCStorage = new CStorage;
	DBORDINAL cBindings = 0;
	DBBINDING* rgBindings = NULL;
	DBORDINAL ulBinding = 0;
	IUnknown* pIUnknown = (ILockBytes*)pCStorage;
	if(m_riidStorage==IID_ILockBytes)
		pIUnknown = (IStream*)pCStorage;

	CRowsetChange RowsetA;
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);

	//Create Accessor binding BLOB/Storage Object data
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, &cBindings, &rgBindings, &cBytes, m_dwFlags | BLOB_BIND_STR), S_OK)

	//Alloc buffers
	TESTC(RowsetA.MakeRowData(&pData,hAccessor,&iTableRow))
	TESTC(SetStorageObject(cBindings, rgBindings, pData, iTableRow, m_riidStorage, pCStorage))
	
	//Set a non-Storage object, to make sure the provider is not referencing this
	//pointer directly, as with any object a QI should occur.  This is also for
	//consumers that implement their object with multiple interface iheritence
	//you will be passed an object implmenting the interface, but not neccesarrly
	//the exact interface to directly dereference.
	TESTC(FindBinding(cBindings, rgBindings, DBTYPE_IUNKNOWN, &ulBinding) != NULL)
	VALUE_BINDING(rgBindings[ulBinding], pData) = pIUnknown;

	//Passing a storage object, and it implement the interface in pObject->iid
	//but this is the object and must have QI called to obtain the correct interface
	SAFE_ADDREF(pCStorage);
	TESTC_(RowsetA.InsertRow(hAccessor,pData,&hRow), S_OK);

CLEANUP:
	SAFE_RELEASE_(pCStorage);
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor, cBindings, rgBindings, pData);
	TRETURN;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(63)
//*-----------------------------------------------------------------------
// @mfunc Boundary - InsertRow with non-Storage interface
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSeqStream_Write::Variation_63()
{ 
	TBEGIN
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	
	HROW  hRow = NULL;
	DBLENGTH cBytes = 0;
	DBCOUNTITEM iTableRow = 0;
	
	void* pData     = NULL;
	DBORDINAL cBindings = 0;
	DBBINDING* rgBindings = NULL;
	DBORDINAL ulBinding = 0;
	CDispatch* pCDispatch = new CDispatch;

	CRowsetChange RowsetA;
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);

	//Create Accessor binding BLOB/Storage Object data
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, &cBindings, &rgBindings, &cBytes, m_dwFlags | BLOB_BIND_STR), S_OK)

	//Alloc buffers
	TESTC(RowsetA.MakeRowData(&pData,hAccessor,&iTableRow))
	TESTC(SetStorageObject(cBindings, rgBindings, pData, iTableRow, m_riidStorage, NULL))
	
	//Set a non-Storage object, to make sure the provider is not referencing this
	//pointer directly, as with any object a QI should occur.  This is also for
	//consumers that implement their object with multiple interface iheritence
	//you will be passed an object implmenting the interface, but not neccesarrly
	//the exact interface to directly dereference.
	SAFE_ADDREF(pCDispatch);
	TESTC(FindBinding(cBindings, rgBindings, DBTYPE_IUNKNOWN, &ulBinding) != NULL)
	VALUE_BINDING(rgBindings[ulBinding], pData) = pCDispatch;

	//NOTE:  We are using pObject->iid = Storage interface, and passing
	//a pointer to an interface, but it doesn't implement the storage interface
	TEST2C_(RowsetA.InsertRow(hAccessor,pData,&hRow),DB_S_ERRORSOCCURRED,DB_E_ERRORSOCCURRED);
	TESTC(VerifyBindingStatus(cBindings, rgBindings, pData, DBTYPE_IUNKNOWN, DBSTATUS_E_CANTCREATE));

CLEANUP:
	//NOTE: We AddRef'd an extra time to verify refcount...
	SAFE_RELEASE(pCDispatch);

	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor, cBindings, rgBindings, pData);
	TRETURN;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(64)
//*-----------------------------------------------------------------------
// @mfunc Boundary - InsertRow verify objects released on error
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSeqStream_Write::Variation_64()
{ 
	TBEGIN
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	HROW  hRow = NULL;
	DBLENGTH cBytes = 0;
	DBCOUNTITEM iTableRow = 0;
	
	void* pData     = NULL;
	DBORDINAL i,cBindings = 0;
	DBBINDING* rgBindings = NULL;
	CStorage* pCStorage = new CStorage;
	HRESULT hr = S_OK;

	CRowsetChange RowsetA;
	RowsetA.SetSettableProperty(DBPROP_IRowsetLocate);
	TESTC_(RowsetA.CreateRowset(m_dwStorageID),S_OK);

	//Grab a row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK)

	//Create Accessor binding BLOB/Storage Object data
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, &cBindings, &rgBindings, &cBytes, m_dwFlags | BLOB_BIND_STR), S_OK)

	//Loop over all columns creating errors.
	//This was we get testing of errors before as well as after the stream object
	for(i=0; i<cBindings; i++)
	{
		DBBINDING* pBinding = &rgBindings[i];
		if(pBinding->wType != DBTYPE_IUNKNOWN)
		{
			//Alloc buffers
			TESTC(RowsetA.MakeRowData(&pData,hAccessor,&iTableRow))
			TESTC(SetStorageObject(cBindings, rgBindings, pData, iTableRow, m_riidStorage, pCStorage))

			//Try and cause an error with one of the other columns (not the stream column) to verify
			//the provider releases the stream on error...
			//The simplest way to do this (has to be a runtime issue, otherwise createaccess would fail)
			//is to make one of the status' on input not be valid.
			DBSTATUS dbStatus = STATUS_BINDING(*pBinding, pData);
			STATUS_BINDING(*pBinding, pData) = DBSTATUS_E_SCHEMAVIOLATION;

			//NOTE:  We are using pObject->iid = Storage interface, and passing
			//a pointer to an interface, but it doesn't implement the storage interface
			SAFE_ADDREF(pCStorage);
			TESTC_(pCStorage->Seek(0),S_OK);
			TEST2C_(hr = RowsetA.InsertRow(hAccessor,pData,NULL),DB_S_ERRORSOCCURRED,DB_E_ERRORSOCCURRED);

			//Verify the provider flagged the column and status as incorrect
			TESTC(STATUS_BINDING(*pBinding, pData) == DBSTATUS_E_BADSTATUS);
		
			//Verify the storage object was released
			TESTC(GetRefCount(pCStorage->pUnknown()) == 1);
			
			//Restore the status binding so we can continue testing other columns in error....
			STATUS_BINDING(*pBinding, pData) = dbStatus;
			QTESTC_(ReleaseInputBindingsMemory(cBindings, rgBindings, (BYTE*)pData, TRUE),S_OK)
			pData = NULL;
		}
	}

CLEANUP:
	//NOTE: We AddRef'd an extra time to verify refcount...
	SAFE_RELEASE_(pCStorage);

	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor, cBindings, rgBindings, pData);
	TRETURN;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(65)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSeqStream_Write::Variation_65()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCSeqStream_Write::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCStorage::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCSeqStream_Buffered)
//*-----------------------------------------------------------------------
//| Test Case:		TCSeqStream_Buffered - Test ISequentialStream support in Buffered Update mode
//|	Created:			09/13/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCSeqStream_Buffered::Init()
{
	TBEGIN

	// {{ TCW_INIT_BASECLASS_CHECK
	TESTC(TCStorage::Init());
	// }}
 	
	//Determine if this Storage interface is supported...
	TESTC_PROVIDER(SupportedStorageInterface(DBPROP_IRowsetUpdate));

	//Currently these TestCases for Bufferred Mode Storage Objects are not written
	//or complete.  Currently they will be skipped until code complete.
	return TEST_SKIPPED;

CLEANUP:
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc SetData -> GetData -> GetData
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Buffered::Variation_1()
{
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc SetData -> GetData -> Update
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Buffered::Variation_2()
{
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc SetData -> GetOriginalData -> GetOriginalData
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Buffered::Variation_3()
{
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc SetData -> GetOriginalData -> Update
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Buffered::Variation_4()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Buffered::Variation_5()
{
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc SetData -> Update -> GetData -> GetData == GetOriginalData
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Buffered::Variation_6()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc SetData -> Update -> GetOriginalData -> GetOriginalData == GetData
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Buffered::Variation_7()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Buffered::Variation_8()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc SetData -> Undo -> GetData -> GetData == GetOriginalData
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Buffered::Variation_9()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc SetData -> Undo -> GetOriginalData -> GetOriginalData == GetData
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Buffered::Variation_10()
{
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Buffered::Variation_11()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc SetData [row1] -> SetData [row1] -> Update
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Buffered::Variation_12()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc SetData [row1] -> SetData [row2] -> Update
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Buffered::Variation_13()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc SetData [row1] -> SetData [row2] -> Update [row1]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Buffered::Variation_14()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Buffered::Variation_15()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc SetData [row1] -> SetData [row1] -> Undo [row1]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Buffered::Variation_16()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc SetData [row1] -> SetData [row2] -> Undo [row1]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Buffered::Variation_17()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc SetData [row1] -> SetData [row2] -> Undo [row2]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Buffered::Variation_18()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Buffered::Variation_19()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc SetData -> SetData [NULL Storage Objects interface] -> Update
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Buffered::Variation_20()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc SetData -> SetData [NULL Storage Objects interface] -> Undo
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Buffered::Variation_21()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(22)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Buffered::Variation_22()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(23)
//*-----------------------------------------------------------------------
// @mfunc SetData (row1
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Buffered::Variation_23()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(24)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Buffered::Variation_24()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(25)
//*-----------------------------------------------------------------------
// @mfunc SetData (with ::Read that returns an error
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Buffered::Variation_25()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(26)
//*-----------------------------------------------------------------------
// @mfunc SetData (with ::Read that returns an error
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Buffered::Variation_26()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(27)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Buffered::Variation_27()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(28)
//*-----------------------------------------------------------------------
// @mfunc SetData -> GetData (binding as long
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Buffered::Variation_28()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(29)
//*-----------------------------------------------------------------------
// @mfunc SetData -> GetOriginalData (binding as long
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Buffered::Variation_29()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(30)
//*-----------------------------------------------------------------------
// @mfunc SetData -> Update -> GetData (binding as long
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Buffered::Variation_30()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(31)
//*-----------------------------------------------------------------------
// @mfunc SetData -> Update -> GetOriginalData (binding as long
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Buffered::Variation_31()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(32)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Buffered::Variation_32()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(33)
//*-----------------------------------------------------------------------
// @mfunc Insert -> GetData -> GetData
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Buffered::Variation_33()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(34)
//*-----------------------------------------------------------------------
// @mfunc Insert -> GetData -> Update
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Buffered::Variation_34()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(35)
//*-----------------------------------------------------------------------
// @mfunc Insert -> GetOriginalData -> GetOriginalData
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Buffered::Variation_35()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(36)
//*-----------------------------------------------------------------------
// @mfunc Insert -> GetOriginalData -> Update
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Buffered::Variation_36()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(37)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Buffered::Variation_37()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(38)
//*-----------------------------------------------------------------------
// @mfunc Insert -> Update -> GetData -> GetData
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Buffered::Variation_38()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(39)
//*-----------------------------------------------------------------------
// @mfunc Insert -> Update -> GetOriginalData -> GetOrignialData
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Buffered::Variation_39()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(40)
//*-----------------------------------------------------------------------
// @mfunc Insert -> Undo -> GetOriginalData -> GetOriginalData
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Buffered::Variation_40()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(41)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Buffered::Variation_41()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(42)
//*-----------------------------------------------------------------------
// @mfunc Insert [rowX] -> Insert [rowY] -> Update [rowY]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Buffered::Variation_42()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(43)
//*-----------------------------------------------------------------------
// @mfunc Insert [rowX] -> Insert [rowY] -> Update [rowX]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Buffered::Variation_43()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(44)
//*-----------------------------------------------------------------------
// @mfunc Insert [rowX] -> Insert [rowY] -> Update all
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Buffered::Variation_44()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(45)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Buffered::Variation_45()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(46)
//*-----------------------------------------------------------------------
// @mfunc Insert [rowX] -> Insert [rowY] -> Undo [rowY]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Buffered::Variation_46()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(47)
//*-----------------------------------------------------------------------
// @mfunc Insert [rowX] -> Insert [rowY] -> Undo [rowX]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Buffered::Variation_47()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(48)
//*-----------------------------------------------------------------------
// @mfunc Insert [rowX] -> Insert [rowY] -> Undo all
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Buffered::Variation_48()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(49)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Buffered::Variation_49()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(50)
//*-----------------------------------------------------------------------
// @mfunc Delete -> GetOriginalData -> GetOriginalData
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Buffered::Variation_50()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(51)
//*-----------------------------------------------------------------------
// @mfunc Delete -> GetOriginalData -> GetOriginalData -> Update
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Buffered::Variation_51()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(52)
//*-----------------------------------------------------------------------
// @mfunc Delete -> GetOriginalData -> GetOriginalData -> Undo
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Buffered::Variation_52()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(53)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Buffered::Variation_53()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(54)
//*-----------------------------------------------------------------------
// @mfunc Delete [row1] -> Delete [row2] -> Update
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Buffered::Variation_54()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(55)
//*-----------------------------------------------------------------------
// @mfunc Delete [row1] -> Delete [row2] -> Undo
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Buffered::Variation_55()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(56)
//*-----------------------------------------------------------------------
// @mfunc Delete [row1] -> Delete [row2] -> Update [row1]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Buffered::Variation_56()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(57)
//*-----------------------------------------------------------------------
// @mfunc Delete [row1] -> Delete [row2] -> Update [row2]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Buffered::Variation_57()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(58)
//*-----------------------------------------------------------------------
// @mfunc Delete [row1] -> Delete [row2] -> Undo [row1]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Buffered::Variation_58()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(59)
//*-----------------------------------------------------------------------
// @mfunc Delete [row1] -> Delete [row2] -> Undo [row2]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Buffered::Variation_59()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(60)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Buffered::Variation_60()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(61)
//*-----------------------------------------------------------------------
// @mfunc Sequence - SetData -> Delete -> Update
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Buffered::Variation_61()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(62)
//*-----------------------------------------------------------------------
// @mfunc Sequence - SetData -> Delete -> Undo
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Buffered::Variation_62()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(63)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Buffered::Variation_63()
{
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(64)
//*-----------------------------------------------------------------------
// @mfunc Sequence - SetData [row1] -> Delete [row2] -> Update
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Buffered::Variation_64()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(65)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Buffered::Variation_65()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(66)
//*-----------------------------------------------------------------------
// @mfunc Sequence - Insert -> SetData -> Update
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Buffered::Variation_66()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(67)
//*-----------------------------------------------------------------------
// @mfunc Sequence - Insert -> SetData -> Undo
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Buffered::Variation_67()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(68)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Buffered::Variation_68()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(69)
//*-----------------------------------------------------------------------
// @mfunc Sequence - Insert -> SetData -> Delete -> Update
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Buffered::Variation_69()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(70)
//*-----------------------------------------------------------------------
// @mfunc Sequence - Insert -> SetData -> Delete -> Undo
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Buffered::Variation_70()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(71)
//*-----------------------------------------------------------------------
// @mfunc Sequence - Insert [rowX] -> SetData [rowY] -> Delete [rowZ] -> Update all
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Buffered::Variation_71()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(72)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Buffered::Variation_72()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(73)
//*-----------------------------------------------------------------------
// @mfunc Release - Get [row1] -> SetData -> ReleaseRows -> Update
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Buffered::Variation_73()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(74)
//*-----------------------------------------------------------------------
// @mfunc Release - Get [row1] -> SetData -> ReleaseRows -> Undo
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Buffered::Variation_74()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(75)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Buffered::Variation_75()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(76)
//*-----------------------------------------------------------------------
// @mfunc Release - Insert -> ReleaseRows -> Update
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Buffered::Variation_76()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(77)
//*-----------------------------------------------------------------------
// @mfunc Release - Insert -> ReleaseRows -> Undo
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Buffered::Variation_77()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(78)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Buffered::Variation_78()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(79)
//*-----------------------------------------------------------------------
// @mfunc Release - Get [row1] -> Delete -> ReleaseRows -> Update
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Buffered::Variation_79()
{
	
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(80)
//*-----------------------------------------------------------------------
// @mfunc Release - Get [row1] -> Delete -> ReleaseRows -> Undo
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Buffered::Variation_80()
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
BOOL TCSeqStream_Buffered::Terminate()
{
 	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCStorage::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCSeqStream_Transactions)
//*-----------------------------------------------------------------------
//| Test Case:		TCSeqStream_Transactions - Test the zombie states of Storage objects
//|	Created:			09/09/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCSeqStream_Transactions::Init()
{
	m_cPropSets = 0;
	m_rgPropSets = NULL;

	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCTransactions::Init())
	// }}
	{
		//Determine if this Storage interface is supported...
		TEST_PROVIDER(SupportedStorageInterface());

		//Set Properties, were done testing if not supported
		SetSettableProperty(DBPROP_IRowsetLocate, DBPROPSET_ROWSET, &m_cPropSets, &m_rgPropSets);

		//register interface to be tested                     
		if(RegisterInterface(ROWSET_INTERFACE, IID_IRowset))
			return TRUE;
	}

	//Not all providers have to support transactions
	//If a required interface, an error would ahve been posted by VerifyInterface
	TEST_PROVIDER(m_pITransactionLocal != NULL);
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Storage Objects::Read - Zombie - ABORT  with fRetaining == TRUE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Transactions::Variation_1()
{
	TBEGIN
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	HROW  hRow = NULL;
	
	DBLENGTH cBytes = MAX_PTR;
	void* pData     = NULL;
	void* pBuffer   = NULL;

	DBORDINAL cBindings = 0;
	DBBINDING* rgBindings = NULL;
	IUnknown* pIUnknown = NULL;

	CRowset RowsetA;
	RowsetA.SetSupportedProperty(DBPROP_IRowsetLocate);
	
	//Start the Transaction
	//And obtain the IOpenRowset interface
	TESTC(StartTransaction(USE_SUPPORTED_SELECT_ALLFROMTBL, NULL, m_cPropSets,m_rgPropSets));
	TESTC(m_pIRowset!=NULL) //Rowset interface from CTransAction

	//Obtain the first row
	TESTC_PROVIDER(RowsetA.CreateRowset(m_pIRowset)==S_OK);

	//Grab a row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK)
	
	//Create Accessor binding BLOB/Storage Object data
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, &cBindings, &rgBindings, &cBytes, m_dwFlags), S_OK)

	//Alloc buffers
	SAFE_ALLOC(pData, void*, cBytes);
	SAFE_ALLOC(pBuffer, void*, cBytes);

	//Get the Data, ISequentialStream - should actually succeed
	TESTC_(RowsetA.GetData(hRow, hAccessor, pData),S_OK)
	TESTC(GetStorageObject(cBindings, rgBindings, pData, m_riidStorage, (IUnknown**)&pIUnknown))

	//Abort the Transaction with fRetaining==TRUE
	TESTC(GetAbort(TRUE))
	cBytes = MAX_PTR;

	//Obtain the ABORTPRESERVE flag and adjust ExpectedHr 
	if(m_fAbortPreserve ) 
	{
		//Make sure the storage object is valid
		TESTC_(TCStorage::StorageRead(m_riidStorage, pIUnknown, pBuffer, 10, &cBytes),S_OK)
		TESTC(cBytes == 10)
	}
	else
	{
		//Make sure the storage object is invalid
		TESTC_(TCStorage::StorageRead(m_riidStorage, pIUnknown, pBuffer, 10, &cBytes),E_UNEXPECTED)
		TESTC(cBytes == 0)
	}

CLEANUP:
	//Need to release the storage object
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor, cBindings, rgBindings, pData);
	PROVIDER_FREE(pBuffer);
	SAFE_RELEASE_(pIUnknown);

	CleanUpTransaction(S_OK);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Storage Objects::Read - Zombie - ABORT  with fRetaining == FALSE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Transactions::Variation_2()
{
	TBEGIN
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	HROW  hRow = NULL;
	DBLENGTH cBytes = 0;
	
	void* pData     = NULL;
	void* pBuffer   = NULL;
	
	DBORDINAL cBindings = 0;
	DBBINDING* rgBindings = NULL;
	IUnknown* pIUnknown = NULL;

	CRowset RowsetA;
	RowsetA.SetSupportedProperty(DBPROP_IRowsetLocate);
	
	//Start the Transaction
	//And obtain the IOpenRowset interface
	TESTC(StartTransaction(USE_SUPPORTED_SELECT_ALLFROMTBL, NULL, m_cPropSets,m_rgPropSets));
	TESTC(m_pIRowset!=NULL) //Rowset interface from CTransAction

	//Obtain the first row
	TESTC_PROVIDER(RowsetA.CreateRowset(m_pIRowset)==S_OK);

	//Grab a row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK)
	
	//Create Accessor binding BLOB/Storage Object data
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, &cBindings, &rgBindings, &cBytes, m_dwFlags | BLOB_BIND_STR), S_OK)

	//Alloc buffers
	SAFE_ALLOC(pData, void*, cBytes);
	SAFE_ALLOC(pBuffer, void*, cBytes);

	//Get the Data, ISequentialStream - should actually succeed
	TESTC_(RowsetA.GetData(hRow, hAccessor, pData),S_OK)
	TESTC(GetStorageObject(cBindings, rgBindings, pData, m_riidStorage, (IUnknown**)&pIUnknown))

	//Abort the Transaction with fRetaining==FALSE
	TESTC(GetAbort(FALSE))
	cBytes = MAX_PTR;
	
	//Obtain the ABORTPRESERVE flag and adjust ExpectedHr 
	if(m_fAbortPreserve ) 
	{
		//Make sure the storage object is valid
		TESTC_(TCStorage::StorageRead(m_riidStorage, pIUnknown, pBuffer, 10, &cBytes),S_OK)
		TESTC(cBytes == 10)
	}
	else
	{
		//Make sure the storage object is invalid
		TESTC_(TCStorage::StorageRead(m_riidStorage, pIUnknown, pBuffer, 10, &cBytes),E_UNEXPECTED)
		TESTC(cBytes == 0)
	}

CLEANUP:
	//Need to release the storage object
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor, cBindings, rgBindings, pData);
	PROVIDER_FREE(pBuffer);
	SAFE_RELEASE_(pIUnknown);

	CleanUpTransaction(XACT_E_NOTRANSACTION); //No longer in a transaction
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Storage Objects::Read - Zombie - COMMIT  with fRetaining == TRUE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Transactions::Variation_3()
{
	TBEGIN
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	HROW  hRow = NULL;
	DBLENGTH cBytes = 0;
	
	void* pData     = NULL;
	void* pBuffer   = NULL;

	DBORDINAL cBindings = 0;
	DBBINDING* rgBindings = NULL;
	IUnknown* pIUnknown = NULL;

	CRowset RowsetA;
	RowsetA.SetSupportedProperty(DBPROP_IRowsetLocate);
	
	//Start the Transaction
	//And obtain the IOpenRowset interface
	TESTC(StartTransaction(USE_SUPPORTED_SELECT_ALLFROMTBL, NULL, m_cPropSets,m_rgPropSets));
	TESTC(m_pIRowset!=NULL) //Rowset interface from CTransAction

	//Obtain the first row
	TESTC_PROVIDER(RowsetA.CreateRowset(m_pIRowset)==S_OK);

	//Grab a row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK)
	
	//Create Accessor binding BLOB/Storage Object data
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, &cBindings, &rgBindings, &cBytes, m_dwFlags), S_OK)

	//Alloc buffers
	SAFE_ALLOC(pData, void*, cBytes);
	SAFE_ALLOC(pBuffer, void*, cBytes);

	//Get the Data, ISequentialStream - should actually succeed
	TESTC_(RowsetA.GetData(hRow, hAccessor, pData),S_OK)
	TESTC(GetStorageObject(cBindings, rgBindings, pData, m_riidStorage, (IUnknown**)&pIUnknown))

	//Commit the Transaction with fRetaining==TRUE
	TESTC(GetCommit(TRUE))
	cBytes = MAX_PTR;
	
	//Obtain the COMMITPRESERVE flag and adjust ExpectedHr 
	if(m_fCommitPreserve ) 
	{
		//Make sure the storage object is valid
		TESTC_(TCStorage::StorageRead(m_riidStorage, pIUnknown, pBuffer, 10, &cBytes),S_OK)
		TESTC(cBytes == 10)
	}
	else
	{
		//Make sure the storage object is invalid
		TESTC_(TCStorage::StorageRead(m_riidStorage, pIUnknown, pBuffer, 10, &cBytes),E_UNEXPECTED)
		TESTC(cBytes == 0)
	}

CLEANUP:
	//Need to release the storage object
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor, cBindings, rgBindings, pData);
	PROVIDER_FREE(pBuffer);
	SAFE_RELEASE_(pIUnknown);

	CleanUpTransaction(S_OK);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Storage Objects::Read - Zombie - COMMIT  with fRetaining == FALSE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Transactions::Variation_4()
{
	TBEGIN
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	HROW  hRow = NULL;
	DBLENGTH cBytes = 0;
	
	void* pData     = NULL;
	void* pBuffer   = NULL;

	DBORDINAL cBindings = 0;
	DBBINDING* rgBindings = NULL;
	IUnknown* pIUnknown = NULL;

	CRowset RowsetA;
	RowsetA.SetSupportedProperty(DBPROP_IRowsetLocate);
	
	//Start the Transaction
	//And obtain the IOpenRowset interface
	TESTC(StartTransaction(USE_SUPPORTED_SELECT_ALLFROMTBL, NULL, m_cPropSets,m_rgPropSets));
	TESTC(m_pIRowset!=NULL) //Rowset interface from CTransAction

	//Obtain the first row
	TESTC_PROVIDER(RowsetA.CreateRowset(m_pIRowset)==S_OK);

	//Grab a row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK)
	
	//Create Accessor binding BLOB/Storage Object data
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, &cBindings, &rgBindings, &cBytes, m_dwFlags | BLOB_BIND_STR), S_OK)

	//Alloc buffers
	SAFE_ALLOC(pData, void*, cBytes);
	SAFE_ALLOC(pBuffer, void*, cBytes);

	//Get the Data, ISequentialStream - should actually succeed
	TESTC_(RowsetA.GetData(hRow, hAccessor, pData),S_OK)
	TESTC(GetStorageObject(cBindings, rgBindings, pData, m_riidStorage, (IUnknown**)&pIUnknown))

	//Commit the Transaction with fRetaining==FALSE
	TESTC(GetCommit(FALSE))
	cBytes = MAX_PTR;

	//Obtain the COMMITPRESERVE flag and adjust ExpectedHr 
	if(m_fCommitPreserve ) 
	{
		//Make sure the storage object is valid
		TESTC_(TCStorage::StorageRead(m_riidStorage, pIUnknown, pBuffer, 10, &cBytes),S_OK)
		TESTC(cBytes == 10)
	}
	else
	{
		//Make sure the storage object is invalid
		TESTC_(TCStorage::StorageRead(m_riidStorage, pIUnknown, pBuffer, 10, &cBytes),E_UNEXPECTED)
		TESTC(cBytes == 0)
	}

CLEANUP:
	//Need to release the storage object
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor, cBindings, rgBindings, pData);
	PROVIDER_FREE(pBuffer);
	SAFE_RELEASE_(pIUnknown);

	CleanUpTransaction(XACT_E_NOTRANSACTION); //No longer in a transaction
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Transactions::Variation_5()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Storage Objects::Write - Zombie - ABORT  with fRetaining == TRUE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Transactions::Variation_6()
{
	TBEGIN
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	HROW  hRow = NULL;
	DBLENGTH cBytes = 0;
	
	void* pData     = NULL;
	void* pBuffer   = NULL;

	DBORDINAL cBindings = 0;
	DBBINDING* rgBindings = NULL;
	IUnknown* pIUnknown = NULL;

	CRowset RowsetA;
	RowsetA.SetSupportedProperty(DBPROP_IRowsetLocate);
	
	//Start the Transaction
	//And obtain the IOpenRowset interface
	TESTC(StartTransaction(USE_SUPPORTED_SELECT_ALLFROMTBL, NULL, m_cPropSets,m_rgPropSets));
	TESTC(m_pIRowset!=NULL) //Rowset interface from CTransAction

	//Obtain the first row
	TESTC_PROVIDER(RowsetA.CreateRowset(m_pIRowset)==S_OK);

	//Grab a row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK)
	
	//Create Accessor binding BLOB/Storage Object data
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, &cBindings, &rgBindings, &cBytes, m_dwFlags), S_OK)

	//Alloc buffers
	SAFE_ALLOC(pData, void*, cBytes);
	SAFE_ALLOC(pBuffer, void*, cBytes);

	//Get the Data, ISequentialStream - should actually succeed
	TESTC_(RowsetA.GetData(hRow, hAccessor, pData),S_OK)
	TESTC(GetStorageObject(cBindings, rgBindings, pData, m_riidStorage, (IUnknown**)&pIUnknown))

	//Abort the Transaction with fRetaining==TRUE
	TESTC(GetAbort(TRUE))
	cBytes = MAX_PTR;

	//Obtain the ABORTPRESERVE flag and adjust ExpectedHr 
	if(m_fAbortPreserve ) 
	{
		//Make sure the storage object is valid
		TESTC_(TCStorage::StorageWrite(m_riidStorage, pIUnknown, pBuffer, 10, &cBytes),STG_E_ACCESSDENIED)
		TESTC(cBytes == 0)
	}
	else
	{
		//Make sure the storage object is invalid
		TESTC_(TCStorage::StorageWrite(m_riidStorage, pIUnknown, pBuffer, 10, &cBytes),E_UNEXPECTED)
		TESTC(cBytes == 0)
	}

CLEANUP:
	//Need to release the storage object
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor, cBindings, rgBindings, pData);
	PROVIDER_FREE(pBuffer);
	SAFE_RELEASE_(pIUnknown);

	CleanUpTransaction(S_OK);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Storage Objects::Write - Zombie - ABORT  with fRetaining == FALSE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Transactions::Variation_7()
{
	TBEGIN
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	HROW  hRow = NULL;
	DBLENGTH cBytes = 0;
	
	void* pData     = NULL;
	void* pBuffer   = NULL;

	DBORDINAL cBindings = 0;
	DBBINDING* rgBindings = NULL;
	IUnknown* pIUnknown = NULL;

	CRowset RowsetA;
	RowsetA.SetSupportedProperty(DBPROP_IRowsetLocate);
	
	//Start the Transaction
	//And obtain the IOpenRowset interface
	TESTC(StartTransaction(USE_SUPPORTED_SELECT_ALLFROMTBL, NULL, m_cPropSets,m_rgPropSets));
	TESTC(m_pIRowset!=NULL) //Rowset interface from CTransAction

	//Obtain the first row
	TESTC_PROVIDER(RowsetA.CreateRowset(m_pIRowset)==S_OK);

	//Grab a row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK)
	
	//Create Accessor binding BLOB/Storage Object data
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, &cBindings, &rgBindings, &cBytes, m_dwFlags | BLOB_BIND_STR), S_OK)

	//Alloc buffers
	SAFE_ALLOC(pData, void*, cBytes);
	SAFE_ALLOC(pBuffer, void*, cBytes);

	//Get the Data, ISequentialStream - should actually succeed
	TESTC_(RowsetA.GetData(hRow, hAccessor, pData),S_OK)
	TESTC(GetStorageObject(cBindings, rgBindings, pData, m_riidStorage, (IUnknown**)&pIUnknown))

	//Abort the Transaction with fRetaining==FALSE
	TESTC(GetAbort(FALSE))
	cBytes = MAX_PTR;

	//Obtain the ABORTPRESERVE flag and adjust ExpectedHr 
	if(m_fAbortPreserve ) 
	{
		//Make sure the storage object is valid
		TESTC_(TCStorage::StorageWrite(m_riidStorage, pIUnknown, pBuffer, 10, &cBytes),STG_E_ACCESSDENIED)
		TESTC(cBytes == 0)
	}
	else
	{
		//Make sure the storage object is invalid
		TESTC_(TCStorage::StorageWrite(m_riidStorage, pIUnknown, pBuffer, 10, &cBytes),E_UNEXPECTED)
		TESTC(cBytes == 0)
	}

CLEANUP:
	//Need to release the storage object
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor, cBindings, rgBindings, pData);
	PROVIDER_FREE(pBuffer);
	SAFE_RELEASE_(pIUnknown);

	CleanUpTransaction(XACT_E_NOTRANSACTION); //No longer in a transaction
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc Storage Objects::Write - Zombie - COMMIT  with fRetaining == TRUE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Transactions::Variation_8()
{
	TBEGIN
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	HROW  hRow = NULL;
	DBLENGTH cBytes = 0;
	
	void* pData     = NULL;
	void* pBuffer   = NULL;

	DBORDINAL cBindings = 0;
	DBBINDING* rgBindings = NULL;
	IUnknown* pIUnknown = NULL;

	CRowset RowsetA;
	RowsetA.SetSupportedProperty(DBPROP_IRowsetLocate);
	
	//Start the Transaction
	//And obtain the IOpenRowset interface
	TESTC(StartTransaction(USE_SUPPORTED_SELECT_ALLFROMTBL, NULL, m_cPropSets,m_rgPropSets));
	TESTC(m_pIRowset!=NULL) //Rowset interface from CTransAction

	//Obtain the first row
	TESTC_PROVIDER(RowsetA.CreateRowset(m_pIRowset)==S_OK);

	//Grab a row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK)
	
	//Create Accessor binding BLOB/Storage Object data
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, &cBindings, &rgBindings, &cBytes, m_dwFlags), S_OK)

	//Alloc buffers
	SAFE_ALLOC(pData, void*, cBytes);
	SAFE_ALLOC(pBuffer, void*, cBytes);

	//Get the Data, ISequentialStream - should actually succeed
	TESTC_(RowsetA.GetData(hRow, hAccessor, pData),S_OK)
	TESTC(GetStorageObject(cBindings, rgBindings, pData, m_riidStorage, (IUnknown**)&pIUnknown))

	//Commit the Transaction with fRetaining==TRUE
	TESTC(GetCommit(TRUE))
	cBytes = MAX_PTR;

	//Obtain the COMMITPRESERVE flag and adjust ExpectedHr 
	if(m_fCommitPreserve ) 
	{
		//Make sure the storage object is valid
		TESTC_(TCStorage::StorageWrite(m_riidStorage, pIUnknown, pBuffer, 10, &cBytes),STG_E_ACCESSDENIED)
		TESTC(cBytes == 0)
	}
	else
	{
		//Make sure the storage object is invalid
		TESTC_(TCStorage::StorageWrite(m_riidStorage, pIUnknown, pBuffer, 10, &cBytes),E_UNEXPECTED)
		TESTC(cBytes == 0)
	}

CLEANUP:
	//Need to release the storage object
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor, cBindings, rgBindings, pData);
	PROVIDER_FREE(pBuffer);
	SAFE_RELEASE_(pIUnknown);

	CleanUpTransaction(S_OK);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc Storage Objects::Write - Zombie - COMMIT  with fRetaining == FALSE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSeqStream_Transactions::Variation_9()
{
	TBEGIN
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	HROW  hRow = NULL;
	DBLENGTH cBytes = 0;
	
	void* pData     = NULL;
	void* pBuffer   = NULL;

	DBORDINAL cBindings = 0;
	DBBINDING* rgBindings = NULL;
	IUnknown* pIUnknown = NULL;

	CRowset RowsetA;
	RowsetA.SetSupportedProperty(DBPROP_IRowsetLocate);
	
	//Start the Transaction
	//And obtain the IOpenRowset interface
	TESTC(StartTransaction(USE_SUPPORTED_SELECT_ALLFROMTBL, NULL, m_cPropSets,m_rgPropSets));
	TESTC(m_pIRowset!=NULL) //Rowset interface from CTransAction

	//Obtain the first row
	TESTC_PROVIDER(RowsetA.CreateRowset(m_pIRowset)==S_OK);

	//Grab a row handle
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK)
	
	//Create Accessor binding BLOB/Storage Object data
	TESTC_(RowsetA.CreateAccessor(&hAccessor, DBACCESSOR_ROWDATA, DBPART_ALL, &cBindings, &rgBindings, &cBytes, m_dwFlags | BLOB_BIND_STR), S_OK)

	//Alloc buffers
	SAFE_ALLOC(pData, void*, cBytes);
	SAFE_ALLOC(pBuffer, void*, cBytes);

	//Get the Data, ISequentialStream - should actually succeed
	TESTC_(RowsetA.GetData(hRow, hAccessor, pData),S_OK)
	TESTC(GetStorageObject(cBindings, rgBindings, pData, m_riidStorage, (IUnknown**)&pIUnknown))

	//Commit the Transaction with fRetaining==FALSE
	TESTC(GetCommit(FALSE))
	cBytes = MAX_PTR;

	//Obtain the COMMITPRESERVE flag and adjust ExpectedHr 
	if(m_fCommitPreserve ) 
	{
		//Make sure the storage object is valid
		TESTC_(TCStorage::StorageWrite(m_riidStorage, pIUnknown, pBuffer, 10, &cBytes),STG_E_ACCESSDENIED)
		TESTC(cBytes == 0)
	}
	else
	{
		//Make sure the storage object is invalid
		TESTC_(TCStorage::StorageWrite(m_riidStorage, pIUnknown, pBuffer, 10, &cBytes),E_UNEXPECTED)
		TESTC(cBytes == 0)
	}

CLEANUP:
	//Need to release the storage object
	RowsetA.ReleaseRows(hRow);
	RowsetA.ReleaseAccessor(hAccessor, cBindings, rgBindings, pData);
	PROVIDER_FREE(pBuffer);
	SAFE_RELEASE_(pIUnknown);

	CleanUpTransaction(XACT_E_NOTRANSACTION); //No longer in a transaction
	TRETURN
}
// }}
// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCSeqStream_Transactions::Terminate()
{
	FreeProperties(&m_cPropSets,&m_rgPropSets);

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCTransactions::Terminate());
}	// }}
// }}
// }}


