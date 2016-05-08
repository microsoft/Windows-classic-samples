//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1995-2000 Microsoft Corporation.
//
// @doc 
//
// @module DATALITE.CPP | DATALITE source file for all test modules.
//

#include "modstandard.hpp"
#include "DATALITE.h"
#include "ExtraLib.h"

#define MAXFINDBUFFERSIZE (MAXDATALEN * sizeof(WCHAR))


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Module Values
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// {{ TCW_MODULE_GLOBALS
DECLARE_MODULE_CLSID = { 0x4872af80, 0x37a4, 0x11d1, { 0xa8, 0x8b, 0x00, 0xc0, 0x4f, 0xd7, 0xa0, 0xf5} };
DECLARE_MODULE_NAME("DataLite");
DECLARE_MODULE_OWNER("Microsoft");
DECLARE_MODULE_DESCRIP("Test GetData coercions for ReadOnly Providers");
DECLARE_MODULE_VERSION(795921705);
// }}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Globals
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
CTable	*		g_pCTable = NULL;
IDataConvert *	g_pIDataConvert = NULL;
BOOL			g_fCommandSupport = FALSE;

enum eCoerceType	
{
	eNORMAL=0, 
	eBYREF, 
	eTRUNCATE,
	eGETCOLUMNS,
	eGETCOLUMNS_BYREF,
	eGETCOLUMNS_TRUNCATE
};

static BOOL IsVariableLengthType(DBTYPE wdbType)
{
	return (wdbType == DBTYPE_STR || wdbType == DBTYPE_WSTR || wdbType == DBTYPE_BYTES );
};

static DBLENGTH GetDataLength(void *pMakeData, DBTYPE wColType, DBLENGTH cbBytesLen)
{
	DBLENGTH cbLength;

	switch ( wColType )
	{
	case DBTYPE_WSTR:
		cbLength = (wcslen((WCHAR *)pMakeData))*sizeof(WCHAR);
		break;
	case DBTYPE_STR:
		cbLength = strlen((char *)pMakeData);
		break;
	case DBTYPE_VARNUMERIC:
	case DBTYPE_BYTES:
		cbLength = cbBytesLen;
		break;
	default:
		cbLength = GetDBTypeSize(wColType);
		break;
	}

	return cbLength;
}

static void FindVariantTypes(IUnknown *pIUnknown, CTable *pTable)
{
	IAccessor *pIAccessor = NULL;
	IRowset *pIRowset = NULL;
	HACCESSOR hAccessor = DB_INVALID_HACCESSOR;
	DBPART	dwPart=DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH;
	ULONG_PTR cBinding = 0, cRowSize = 0, cCount = 0, i = 0;
	DBBINDING *rgBinding = NULL;
	HROW *pHRow = NULL;
	BYTE *pData = NULL;
	VARIANT *pVar = NULL;
	
	if(!VerifyInterface(pIUnknown, IID_IRowset, ROWSET_INTERFACE, (IUnknown**)&pIRowset))
		goto CLEANUP;

	if(!VerifyInterface(pIRowset, IID_IAccessor, ROWSET_INTERFACE, (IUnknown**)&pIAccessor))
		goto CLEANUP;

	//create an accessor on the rowset
	if( GetAccessorAndBindings(pIRowset,DBACCESSOR_ROWDATA,&hAccessor,
		&rgBinding,&cBinding,&cRowSize,dwPart,ALL_COLS_BOUND,FORWARD,
		NO_COLS_BY_REF,NULL,NULL,NULL,DBTYPE_EMPTY,0,NULL,NULL,
		NO_COLS_OWNED_BY_PROV,DBPARAMIO_NOTPARAM,TRUE) != S_OK )
			goto CLEANUP;

	pData = (BYTE *)PROVIDER_ALLOC(cRowSize);

	if( pIRowset->GetNextRows(NULL,0,1,&cCount,&pHRow) != S_OK )
		goto CLEANUP;

	//get the data
	if( pIRowset->GetData(*pHRow, hAccessor, pData) != S_OK )
		goto CLEANUP;

	for (i = 0; i<cBinding; i++)
	{
		// Check for value binding
		if ((rgBinding[i].dwPart) & DBPART_VALUE)
		{	
			// Skip checking the value binding for BOOKMARKS
			if (rgBinding[i].iOrdinal!=0)
			{
				if (  rgBinding[i].wType == DBTYPE_VARIANT )
				{
					// Get the data in the consumer's buffer
					pVar=(VARIANT *)(pData + rgBinding[i].obValue);
				
					CCol &NewCol = pTable->GetColInfoForUpdate(rgBinding[i].iOrdinal);
					NewCol.SetSubType(pVar->vt);
				}
			}
		}
	}


CLEANUP:
	pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL);
	
	PROVIDER_FREE(rgBinding);
	PROVIDER_FREE(pData);
	PROVIDER_FREE(pHRow);

	if (hAccessor != DB_INVALID_HACCESSOR && pIAccessor)
		pIAccessor->ReleaseAccessor(hAccessor,NULL);

	SAFE_RELEASE(pIAccessor);
	SAFE_RELEASE(pIRowset);
	return;
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
	IUnknown			*pIUnknown=NULL;
	IDBCreateCommand	*pIDBCreateCommand = NULL;

	if(!CommonModuleInit(pThisTestModule, IID_IRowset))
		return FALSE;

	// IDBCreateCommand
	if(!VerifyInterface(pThisTestModule->m_pIUnknown2, IID_IDBCreateCommand, SESSION_INTERFACE, (IUnknown**)&pIDBCreateCommand))
	{
		// Just note it.
		g_fCommandSupport = FALSE;		
		odtLog << L"IDBCreateCommand is not supported by Provider." << ENDL;
	}
	else
	{
		pIDBCreateCommand->Release();
		g_fCommandSupport = TRUE;
	}

	
	if (!SUCCEEDED(CoCreateInstance(CLSID_OLEDB_CONVERSIONLIBRARY,
						  NULL,
						  CLSCTX_INPROC_SERVER,
						  IID_IDataConvert,
						  (void **)&g_pIDataConvert)) )
		return TEST_SKIPPED;

	if(!SetDCLibraryVersion((IUnknown *)g_pIDataConvert, 0x200))
	{
		odtLog << L"Unable to set Data Conversion Library's behavior version!" << ENDL;
		odtLog << L"Need to upgrade to latest MSDADC.DLL." << ENDL;
		return TEST_FAIL;
	}

	//create the table
	g_pCTable = new CTable(pThisTestModule->m_pIUnknown2, (WCHAR *)gwszModuleName, USENULLS);

	if( !g_pCTable || 
		!SUCCEEDED(g_pCTable->CreateTable(20,1,NULL,PRIMARY,TRUE)) )
	{
		if(g_pCTable)
			delete g_pCTable;
		g_pCTable = NULL;

		odtLog<<L"Table creation failed\n";
		return TEST_FAIL;
	}

	if(FAILED(g_pCTable->CreateRowset( SELECT_ALLFROMTBL, IID_IRowset,	0,	NULL, &pIUnknown,				
										NULL, NULL)))
		return TEST_FAIL;

	FindVariantTypes(pIUnknown, g_pCTable);
	SAFE_RELEASE(pIUnknown);

	return TRUE;
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
	//drop table
	if(g_pCTable)
	{
		g_pCTable->DropTable();
		delete g_pCTable;
		g_pCTable = NULL;
	}

	//release OLE memory allocator and IDataConvert interface
	SAFE_RELEASE(g_pIDataConvert);

	//Release IDBCreateCommand interface
	return 	CommonModuleTerminate(pThisTestModule);
}	

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Base Test Case Section
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//
//	TCDATALITE:	the base class for the rest of test cases in this
//						test module. 
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class TCDATALITE : public CRowset
{
	private:

	protected:

		//@cmember: interface pointer for IConvertType
		IConvertType *	m_pIConvertType;

		//@cmember: Binding elements for the current coercion
		DBBINDING		m_Binding;

		//@cmember:	the pointer to the row buffer
		BYTE *			m_pData;

		//@cmember:	the pointer to the row buffer of Expected return values.
		BYTE *			m_pExpectedData;

		//@cmember: hresult
		HRESULT    		m_hr;

		//@cmember: hresult
		HRESULT    		m_hrExpected;

		//@cmember: hresult
		HRESULT    		m_hrActual;

		//@mfunc: initialialize interface pointers
		BOOL	Init();

		//@mfunc: Terminate 
		BOOL	Terminate();

		// Open a rowset 
		BOOL	OpenRowset(IID	riid);

		// Release the rowset
		BOOL	ReleaseRowset();

		// Create Accessor for the coercion
		HRESULT CreateAccessor
				(
					DBTYPE		wType, 
					eCoerceType eCrcType,
					DBCOUNTITEM ulRowIndex,
					DBCOUNTITEM ulColIndex
				);

		// Get the coerced data
		HRESULT FetchData(HROW hrow);

		// Confirm coerced result
		BOOL	VerifyResult(DBTYPE wDstType);

		// loop through table and coerce each cell.
		int		GetDataCoerce
				(
					DBTYPE wType,
					eCoerceType eCrType
				);

		void	FindValidCoercions();

		BOOL	CompatibleStatus
				(
					DBSTATUS dbsActual,
					DBSTATUS dbsExpected
				);

		ULONG	GetPadSize(DBTYPE wType);

		BOOL	IsGetColumnsType(eCoerceType eCrType);

		virtual HRESULT CreateRowset(EQUERY eQuery = SELECT_ORDERBYNUMERIC, REFIID riid = IID_IRowset, CTable* pCTable = NULL, DBACCESSORFLAGS dwAccessorFlags = DBACCESSOR_ROWDATA, DBPART dwPart = DBPART_ALL, DWORD dwColsBound = UPDATEABLE_COLS_BOUND, ECOLUMNORDER eBindingOrder = FORWARD, ECOLS_BY_REF	eColsByRef = NO_COLS_BY_REF, DBTYPE	wTypeModifier = DBTYPE_EMPTY, BLOBTYPE dwBlobType = NO_BLOB_COLS);
		
		virtual HRESULT	DropRowset();

	public:
		//constructor
		TCDATALITE(WCHAR *wstrTestCaseName);

		//destructor
		~TCDATALITE();
};


//--------------------------------------------------------------------
// @mfunc base class TCDATALITE constructor, must take testcase name
//			as parameter.
//
TCDATALITE::TCDATALITE(WCHAR * wstrTestCaseName)	//Takes TestCase Class name as parameter
						: CRowset (wstrTestCaseName) 
{
	//initialize member data
	m_pData			= NULL;
	m_pExpectedData = NULL;
	m_hr			= S_OK;		
	m_hrExpected	= S_OK;
	m_hrActual		= S_OK;
}


//--------------------------------------------------------------------
// @mfunc base class TCDATALITE destructor
//
TCDATALITE::~TCDATALITE()
{

}

//--------------------------------------------------------------------
//@mfunc: Init creates a Data Source object, a DB Session object, 
//and a command object and initialize corresponding interface pointers.
//
//--------------------------------------------------------------------
BOOL TCDATALITE::Init()
{
	m_pData = (BYTE *)PROVIDER_ALLOC(MAXFINDBUFFERSIZE);
	m_pExpectedData = (BYTE *)PROVIDER_ALLOC(MAXFINDBUFFERSIZE);
	
	return (CRowset::Init());
}


//--------------------------------------------------------------------
//@mfunc: Terminate release the data source object, DB Session object, Command object
//
//--------------------------------------------------------------------
BOOL TCDATALITE::Terminate()
{
	PROVIDER_FREE(m_pData);
	PROVIDER_FREE(m_pExpectedData);

	return (CRowset::Terminate());
}

//--------------------------------------------------------------------
//@mfunc: Open Rowset, get IRowset and IAccessor interfaces
//--------------------------------------------------------------------
BOOL TCDATALITE::OpenRowset(IID	riid)
{
	HRESULT		hr;
	IUnknown	*pIUnknown=NULL;
	EQUERY		eQuery = SELECT_ORDERBYNUMERIC;
	
	if (!g_fCommandSupport)
		eQuery = SELECT_ALLFROMTBL;

	hr = g_pCTable->CreateRowset( eQuery, riid,	0,	NULL, &pIUnknown,				
							NULL, NULL);
	
	if ( FAILED(hr) || !pIUnknown) 
		return FALSE;

	// Get Accessor Interface
	if(!VerifyInterface(pIUnknown, IID_IRowset, ROWSET_INTERFACE, (IUnknown **)&m_pIRowset))
		return FALSE;

	// Get Accessor Interface
	if(!VerifyInterface(pIUnknown, IID_IAccessor, ROWSET_INTERFACE, (IUnknown **)&m_pIAccessor))
		return FALSE;

	// Get IConvertType Interface
	if(!VerifyInterface(pIUnknown, IID_IConvertType, ROWSET_INTERFACE, (IUnknown **)&m_pIConvertType))
		return FALSE;

	SAFE_RELEASE(pIUnknown)

	return SUCCEEDED(hr);
}

//--------------------------------------------------------------------
//@mfunc: Release the interfaces obtained through OpenRowset
//--------------------------------------------------------------------
BOOL TCDATALITE::ReleaseRowset()
{	
	SAFE_RELEASE(m_pIAccessor);
	SAFE_RELEASE(m_pIRowset);
	SAFE_RELEASE(m_pIConvertType);
	return TRUE;
}

//--------------------------------------------------------------------
//@mfunc: Setup the accessor
//--------------------------------------------------------------------
HRESULT TCDATALITE::CreateAccessor
(
	DBTYPE		wType,
	eCoerceType	eCrcType,
	ULONG_PTR	ulRowIndex, 
	ULONG_PTR	ulColIndex
)
{
	CCol			TempCol;
	HRESULT			hr = S_OK;
	WCHAR			wszData[MAXFINDBUFFERSIZE];
	BOOL			fIsVariant = FALSE;
	DBTYPE			wVariantType = 0xFFFF;
	DBTYPE			wColType = DBTYPE_EMPTY;
	void *			pvData = NULL;
	void *			pMakeData = NULL;
	DBLENGTH		cbDstLength = 0;
	DBLENGTH		cbDataLength = 0;
	DBLENGTH		cbOutLength = 0;
	USHORT			ulsize = 0;
	DBSTATUS		dbsStatus;
	DBBINDSTATUS	BindStatus = DBSTATUS_S_OK;
	DBTYPE			dbByRefMask = ( eCrcType == eBYREF || eCrcType == eGETCOLUMNS_BYREF ? DBTYPE_BYREF : 0x0 );

	g_pCTable->GetColInfo(ulColIndex, TempCol);
	wColType = TempCol.GetProviderType();

	if (wColType == DBTYPE_UDT)
		return S_FALSE;        // skip all UDTs



	if( eCrcType == eBYREF || eCrcType == eTRUNCATE || 
		eCrcType == eGETCOLUMNS_BYREF || eCrcType == eGETCOLUMNS_TRUNCATE )
	{
		if( !IsVariableLengthType(wType) )
			return S_FALSE;
	}

	if ( TempCol.GetIsLong() )  // skip all BLOBs 
		return S_FALSE;

	m_hr = g_pCTable->MakeData
						(
							wszData, 
							ulRowIndex,
							ulColIndex, 
							PRIMARY, 
							( wColType == DBTYPE_VARIANT ? TempCol.GetSubType() : wColType ), 
							FALSE, 
							&wVariantType
						);

	if( m_hr == DB_E_BADTYPE )
		return S_FALSE;

	if( wVariantType == 0xFFFF )
		wVariantType = TempCol.GetSubType();

	if( wColType == DBTYPE_VARIANT )
	{
		wColType = wVariantType;
		fIsVariant = TRUE;
	}

	if ( m_hr == S_OK )
	{
		pMakeData = (BYTE *)WSTR2DBTYPE(wszData, wColType, &ulsize );

		cbDataLength = GetDataLength(pMakeData, wColType, ulsize);

		if( fIsVariant )
		{
			if ( wVariantType > DBTYPE_UI1 )
			{
				PROVIDER_FREE(pMakeData);
				return S_FALSE;
			}
			else
			{
				pvData = (void *)DBTYPE2VARIANT(pMakeData, wVariantType);
				if( wColType == DBTYPE_BSTR )
					SysFreeString(*(BSTR*)pMakeData);
			}
		}
		else
			pvData = pMakeData;
	
		
		size_t cchMakeData = wcslen(wszData);

		// Fixup mapping for BOOL
		if( DBTYPE_BOOL == wColType )
		{
			if( 0 == wcscmp(wszData, L"0") )
				cchMakeData = wcslen(L"False");
			else
				cchMakeData = wcslen(L"True");
		}

		switch( wType )
		{
		case DBTYPE_STR:
			{
			cbOutLength = cchMakeData+1;		
			cbOutLength += GetPadSize(wColType); // allow latitude in string representations
			break;
			}
		case DBTYPE_WSTR:
			{
			cbOutLength = (cchMakeData+1)*sizeof(WCHAR);
			cbOutLength += GetPadSize(wColType) * sizeof(WCHAR); // allow latitude in string representations
			break;
			}
		case DBTYPE_VARNUMERIC:
			cbOutLength = MAX_VARNUM_BYTE_SIZE;
			break;
		case DBTYPE_BYTES:
			cbOutLength = cbDataLength;
			break;
		default:
			cbOutLength = GetDBTypeSize(wType);
			break;
		}
		
		switch ( eCrcType )
		{
		case eNORMAL:
		case eGETCOLUMNS:
			break;
		case eBYREF:
		case eGETCOLUMNS_BYREF:
			cbOutLength = sizeof(void *);
			break;
		case eTRUNCATE:
		case eGETCOLUMNS_TRUNCATE:
			if( wColType == DBTYPE_CY || wColType == DBTYPE_NUMERIC || 
				wColType == DBTYPE_R4 || wColType == DBTYPE_R8 )
				cbOutLength = 1;
			else if ( cbOutLength <= 4 )
				cbOutLength = 1;
			else
				cbOutLength = cbOutLength - 4;

			if ( wType == DBTYPE_WSTR )
				cbOutLength *= sizeof(WCHAR);

			break;
		}
	}
	else
		cbOutLength = MAXFINDBUFFERSIZE - offsetof(DATA, bValue);


	m_hrExpected = g_pIDataConvert->DataConvert 
					(
						(fIsVariant ? DBTYPE_VARIANT : wColType),
						wType | dbByRefMask,
						cbDataLength,
						&cbDstLength,
						pvData,
						m_pExpectedData+offsetof(DATA, bValue),
						cbOutLength,
						(m_hr == S_FALSE ? DBSTATUS_S_ISNULL : DBSTATUS_S_OK),
						&dbsStatus,
						(wType == DBTYPE_NUMERIC ? 39 : 0),
						0,
						DBDATACONVERT_DEFAULT
					);

	*(DBLENGTH *)(m_pExpectedData+offsetof(DATA, ulLength)) = cbDstLength;
	*(DBSTATUS *)(m_pExpectedData+offsetof(DATA, sStatus)) = dbsStatus;

	m_Binding.iOrdinal		= ulColIndex;
	m_Binding.dwPart		= DBPART_VALUE | DBPART_LENGTH | DBPART_STATUS;
	m_Binding.eParamIO		= DBPARAMIO_NOTPARAM;	
	m_Binding.pTypeInfo		= NULL;
	m_Binding.cbMaxLen		= cbOutLength;
	m_Binding.obValue		= offsetof(DATA, bValue);
	m_Binding.obLength		= offsetof(DATA, ulLength);
	m_Binding.obStatus		= offsetof(DATA, sStatus);
	m_Binding.dwMemOwner	= DBMEMOWNER_CLIENTOWNED;
	m_Binding.wType			= wType | dbByRefMask;
	m_Binding.pBindExt		= NULL;
	m_Binding.bPrecision	= (wType == DBTYPE_NUMERIC ? 39 : 0);
	m_Binding.bScale		= 0;
	m_Binding.pObject		= NULL;
	m_Binding.dwFlags		= 0;

	// Create the accessor
	m_hr = m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA,1, &m_Binding,NULL,&m_hAccessor,&BindStatus);

	if ( FAILED(m_hr))
	{
		if( !COMPARE(BindStatus, DBBINDSTATUS_UNSUPPORTEDCONVERSION) )
		{
			hr = E_FAIL;
			goto CLEANUP;
		}
		
		if(!(fIsVariant && ((wType|dbByRefMask) == DBTYPE_ERROR)) && !COMPARE(m_pIConvertType->CanConvert
								(
									(fIsVariant ? DBTYPE_VARIANT : wColType), 
									wType | dbByRefMask,
									DBCONVERTFLAGS_COLUMN
								), S_FALSE) )
		{
			hr = E_FAIL;
			goto CLEANUP;
		}

		hr = S_FALSE; // Accessor validation failed, but pass because it should have failed.
	}

CLEANUP:
	PROVIDER_FREE(pvData);
	return hr;
}


//--------------------------------------------------------------------
//@mfunc: Call GetData
//--------------------------------------------------------------------
HRESULT TCDATALITE::FetchData(HROW hrow)
{
	return (m_hrActual = m_pIRowset->GetData(hrow,m_hAccessor,m_pData));
}

//--------------------------------------------------------------------
//@mfunc: Check results
//--------------------------------------------------------------------
BOOL TCDATALITE::VerifyResult(DBTYPE wDstType)
{
	BOOL		fDataOK = TRUE;
	DBLENGTH	cbActual, cbExpected;
	DBSTATUS	dbsActual, dbsExpected;
	CCol		TempCol;
	BOOL		fChangedFromStr = FALSE;
	USHORT		ulExpectedSize, ulActualSize;
	DBLENGTH	cbBytesReceived = 0;
	HRESULT		hr;
	DBTYPE		wSrcType;
	double		dblTolerance = 0;

	g_pCTable->GetColInfo(m_Binding.iOrdinal, TempCol);

	wSrcType = TempCol.GetProviderType();

	// status
	dbsActual = *(DBSTATUS *)((BYTE *)m_pData+offsetof(DATA, sStatus));
	dbsExpected = *(DBSTATUS *)((BYTE *)m_pExpectedData+offsetof(DATA, sStatus));

	// length
	cbActual = *(ULONG *)((BYTE *)m_pData+offsetof(DATA, ulLength));
	cbExpected = *(ULONG *)((BYTE *)m_pExpectedData+offsetof(DATA, ulLength));

	// value
	void *pActual, *pExpected;

	if ( m_Binding.wType & DBTYPE_BYREF )
	{
		pActual = *(void **)(m_pData + offsetof(DATA, bValue));
		pExpected = *(void **)(m_pExpectedData + offsetof(DATA, bValue));
	}
	else
	{
		pActual = m_pData + offsetof(DATA, bValue);
		pExpected = m_pExpectedData + offsetof(DATA, bValue);
	}

	// Check HRESULT
	if ( m_hrActual == DB_E_UNSUPPORTEDCONVERSION )
	{
		if ( !COMPARE(m_pIConvertType->CanConvert(TempCol.GetProviderType(), m_Binding.wType, DBCONVERTFLAGS_COLUMN), S_FALSE) )
		{
			odtLog << "Conversion was supported, but GetData failed\n";		
			fDataOK = FALSE;							
		}
		goto CLEANUP; // we're done if we receive this HRESULT
	}
	else if ( m_hrExpected == DB_E_UNSUPPORTEDCONVERSION )
	{
		if (SUCCEEDED(m_hrActual)) //|| dbsActual != DBSTATUS_E_BADACCESSOR)

		{
			odtLog << "Warning, unexpected conversion is supported\n";		
		}
		goto CLEANUP; // always leave
	}		

	if ( SUCCEEDED(m_hrActual) != SUCCEEDED(m_hrExpected) )
	{
		if ( m_hrActual == DB_E_ERRORSOCCURRED && m_hrExpected == S_OK )
		{
			if ( !COMPARE(dbsActual,DBSTATUS_E_BADACCESSOR) )
			{
				// The only way if this is valid is if the provider doesn't support the conversion
				// in which case DBSTATUS_E_BADACCESSOR is the only valid status.
				odtLog << "HRESULT indicated unsupported conversion, but bad status received" ;		
				fDataOK = FALSE;
				goto CLEANUP;
			}
		}	
		else
		{
			CHECK(m_hrActual, m_hrExpected);
			fDataOK = FALSE;
			goto CLEANUP;				
		}
	}

	// Check status
	if ( dbsActual != dbsExpected )
	{
		// check for deferred validation
		if ( dbsActual == DBSTATUS_E_BADACCESSOR )
		{
			if ( !COMPARE(m_pIConvertType->CanConvert(TempCol.GetProviderType(), m_Binding.wType, DBCONVERTFLAGS_COLUMN), S_FALSE) )
			{
				odtLog << "Conversion was supported, but GetData failed\n";		
				fDataOK = FALSE;
				goto CLEANUP;				
			}

			fDataOK = TRUE;
			odtLog << "Conversion from type " << TempCol.GetProviderType() << " to type " << wDstType << " is not supported\n";
			goto CLEANUP;
		}

		// Special case because Variant(vt=VT_NULL) is equivalent to a DBSTATUS_S_ISNULL status
		if ( dbsExpected == DBSTATUS_S_ISNULL && dbsActual == DBSTATUS_S_OK )
		{
			VARIANT *pVariant = NULL;

			if ( m_Binding.wType == DBTYPE_VARIANT )				
			{
				pVariant = (VARIANT *)pActual;
				if ( pVariant->vt == VT_NULL )
				{
					fDataOK = TRUE;
					goto CLEANUP;
				}
				else
				{
					odtLog << "Unexpected status\n";		
					fDataOK = FALSE;
					goto CLEANUP;
				}					
			}				
		}

		if (!CompatibleStatus(dbsActual, dbsExpected))
		{
			odtLog << "Unexpected status" << " Expected:" << dbsExpected << " Received:" << dbsActual << ENDL;		
			fDataOK = FALSE;
			goto CLEANUP;
		}
	}

	// Check Length
	if ( dbsActual != DBSTATUS_S_OK && dbsActual != DBSTATUS_S_TRUNCATED ) 
		return TRUE;

	if (dbsExpected != DBSTATUS_S_ISNULL && dbsActual != DBSTATUS_S_ISNULL)
	{
		// defer checking cbLength of variable length type to the actual data comparison
		if(	wDstType != DBTYPE_STR && wDstType != DBTYPE_WSTR &&
			wDstType != DBTYPE_BSTR && wDstType != DBTYPE_VARNUMERIC)
		{
			if(!COMPARE( cbActual, cbExpected))
			{		
				odtLog << "Unexpected length\n" ;
				fDataOK = FALSE;
				goto CLEANUP;
			}
		}
	}

	// ulActualSize and ulExpectedSize are only relevant to VARNUMERIC
	ulActualSize = USHORT(cbActual);
	ulExpectedSize = USHORT(cbExpected);



	wDstType &= (~DBTYPE_BYREF);

	if ( (wDstType == DBTYPE_STR || wDstType == DBTYPE_WSTR || wDstType == DBTYPE_BSTR) )
	{
		WCHAR *	wszActual = NULL;
		WCHAR *	wszExpected = NULL;
		void *	pMakeDataExpected = NULL;
		void *	pMakeDataActual = NULL;

		if( dbsActual == DBSTATUS_S_TRUNCATED )
		{
			fDataOK = TRUE;
			goto CLEANUP;
		}

		wSrcType = TempCol.GetProviderType();
		if( wSrcType == DBTYPE_VARIANT )
		{
			wSrcType =  TempCol.GetSubType();
		}

		if ( wSrcType == DBTYPE_DECIMAL || wSrcType == DBTYPE_CY || wSrcType == DBTYPE_R4 ||
			 wSrcType == DBTYPE_R8 || wSrcType == DBTYPE_VARNUMERIC || wSrcType == DBTYPE_NUMERIC || 
			 wSrcType == DBTYPE_DATE )
		{
			fChangedFromStr = TRUE;

			// Fixup to WSTR
			ConvertToWSTR(pActual, wDstType, &wszActual);
			ConvertToWSTR(pExpected, wDstType, &wszExpected);

			pMakeDataExpected = (BYTE *)WSTR2DBTYPE(wszExpected, wSrcType, &ulExpectedSize );
			pMakeDataActual = (BYTE *)WSTR2DBTYPE(wszActual, wSrcType, &ulActualSize );

			if ( wDstType == DBTYPE_BSTR )
			{
				SysFreeString(*(BSTR *)pActual);
				SysFreeString(*(BSTR *)pExpected);
			}
			else if ( m_Binding.wType & DBTYPE_BYREF )
			{
				PROVIDER_FREE(pActual);
				PROVIDER_FREE(pExpected);
			}

			pActual = pMakeDataActual;
			pExpected = pMakeDataExpected;
			wDstType = wSrcType;

			PROVIDER_FREE(wszActual);
			PROVIDER_FREE(wszExpected);
		}
	}

	cbBytesReceived = (dbsActual == DBSTATUS_S_TRUNCATED) ? m_Binding.cbMaxLen : cbActual;

	switch ( wDstType )
	{
	case DBTYPE_R8:
		{
		double dblExpected;
		double dblActual;

		dblActual = *(double *)pActual;
		dblExpected = *(double *)pExpected;

		dblTolerance = 16.0/pow((double)2,(int)53); // large tolerance in case we didn't create the data


		if ( fabs(dblActual - dblExpected) > fabs(dblTolerance * dblActual) )
			fDataOK = FALSE;
		
		break;
		}
	case DBTYPE_R4:
		{
		float fltExpected, fltActual;
		double dblTolerance;
		fltActual = *(float *)pActual;
		fltExpected = *(float *)pExpected;

		dblTolerance = 1.0/pow((double)2,(int)24);


		if ( fabs(fltActual - fltExpected) > fabs(dblTolerance * fltActual) )
			fDataOK = FALSE;
				
		break;
		}
	case DBTYPE_DATE:
		{
		DATE dtExpected, dtActual;
		double dblTolerance;
		dtActual = *(DATE *)pActual;
		dtExpected = *(DATE *)pExpected;

		// 86400 seconds is a day. We want to be accurate to 1 second.
		// OLE Aut's Date was designed to hold a value to nearest second.		
		// A conversion from timestamp will lost precision.
		dblTolerance = 1.0/86400.0;

		if ( fabs(dtActual - dtExpected) > dblTolerance )
			fDataOK = FALSE;
		break;
		}
	case DBTYPE_NUMERIC:
		{		
		fDataOK = CompareVarNumeric((DB_VARNUMERIC *)pActual, sizeof(DB_NUMERIC),
					(DB_VARNUMERIC *)pExpected, sizeof(DB_NUMERIC));
		break;
		}
	case DBTYPE_VARNUMERIC:
		{
		fDataOK = CompareVarNumeric((DB_VARNUMERIC *)pActual, ulActualSize,
						(DB_VARNUMERIC *)pExpected, ulExpectedSize);
		break;
		}
	case DBTYPE_DECIMAL:
		{
		fDataOK = CompareDecimal((DECIMAL *)pActual, (DECIMAL *)pExpected);
		break;
		}
	case DBTYPE_CY:
	{
		__int64 cyExpected, cyActual;
		double dblTolerance;
		cyActual = *(__int64 *)pActual;
		cyExpected = *(__int64 *)pExpected;

		if( wSrcType == DBTYPE_R4 )
			dblTolerance = 1.0/pow((double)10,(int)3);  // 3 = 7(real precision) - 4 (CY scale by 10,000)
		else if( wSrcType == DBTYPE_R8 )
			dblTolerance = 1.0/pow((double)10,(int)9);
		else 
			dblTolerance = 0.0;
		
		if( fabs(double(cyActual - cyExpected)) > fabs(dblTolerance * cyActual) )		
			fDataOK = FALSE;

		break;
	}
	case DBTYPE_I8:
	{
		__int64 i8Expected, i8Actual;
		
		i8Actual = *(__int64 *)pActual;
		i8Expected = *(__int64 *)pExpected;

		fDataOK = (i8Actual == i8Expected);

		if( !fDataOK && i8Expected < 0 )
		{
			// it's provider specific whether negative fractional elements
			// are rounded up or down
			fDataOK = (i8Actual - i8Expected <= 1) && (i8Actual - i8Expected >= -1);
		}

		break;
	}
	case DBTYPE_VARIANT:
	{
		VARIANT *pVarActual = (VARIANT *)pActual;
		VARIANT *pVarExpected = (VARIANT *)pExpected;

		fDataOK = CompareVariant((VARIANT *)pActual,(VARIANT *)pExpected, FALSE);
		if(!fDataOK)
		{
			// be lenient on what VARIANT type to expect.
			// See if the Returned VARIANT type is compatible with the expected VARIANT type.
			// Use OLE AUT's VariantChangeTypeEx for this compatibility verification
			if( V_VT(pVarActual) != V_VT(pVarExpected) )
			{
				//The only problem with trying to convert to the same type are the
				// NULL and EMPTY cases.  If we are expecting NULL or EMPTY
				// we should never receive a different type
				if( V_VT(pVarExpected) != VT_NULL && V_VT(pVarExpected) != VT_EMPTY )
				{
				//If there not the same type, we can try and convert them to the same type
				//if the conversion fails, we are not able to compare these two 
				//disjoint types...
					hr = VariantChangeTypeEx(
							pVarActual,				// Destination (convert in place)
							pVarActual,				// Source
							MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT),	// LCID
							0,						// dwFlags
							V_VT(pVarExpected));
		 				
					if( hr == DISP_E_TYPEMISMATCH )
					{
						hr = VariantChangeTypeEx(
								pVarActual,				// Destination (convert in place)
								pVarActual,				// Source
								GetUserDefaultLCID(),	// LCID
								0,						// dwFlags
								V_VT(pVarExpected));
					}

					if( SUCCEEDED(hr) )
						fDataOK = CompareVariant(pVarActual, pVarExpected, FALSE);					
				}
			}
		}

		VariantClear(pVarActual);
		VariantClear(pVarExpected);
		break;
	}
	case DBTYPE_STR:
		{
		if( dbsActual == DBSTATUS_S_TRUNCATED && m_Binding.cbMaxLen >= sizeof(char) )
			cbBytesReceived -= sizeof(char);
		fDataOK = (0 == strncmp((char *)pActual, (char *)pExpected, cbBytesReceived));
		break;
		}
	case DBTYPE_WSTR:
		{
		if( dbsActual == DBSTATUS_S_TRUNCATED && m_Binding.cbMaxLen >= sizeof(WCHAR) )
			cbBytesReceived -= sizeof(WCHAR);
		fDataOK = (0 == wcsncmp((WCHAR *)pActual, (WCHAR *)pExpected, cbBytesReceived/sizeof(WCHAR)));
		break;
		}
	case DBTYPE_BYTES:
		{
		DBLENGTH cb = 0;
		
		if( dbsActual == DBSTATUS_S_TRUNCATED ) 
		{
			if( m_Binding.cbMaxLen < cbActual )
				cb = m_Binding.cbMaxLen;				
			else
				cb = cbActual;
		}
		else
			cb = cbActual;

		fDataOK = (0 == memcmp(pActual, pExpected, cb));
		break;
		}
	default:
		{
		fDataOK = CompareDBTypeData(pActual, pExpected,	wDstType, cbBytesReceived,
									BYTE(m_Binding.bPrecision),BYTE(m_Binding.bScale),	
									NULL, FALSE, DBTYPE_EMPTY, cbBytesReceived);

		break;
		}
	}

	if( fDataOK )
	{
		if( !COMPARE(m_pIConvertType->CanConvert(TempCol.GetProviderType(), m_Binding.wType, DBCONVERTFLAGS_COLUMN), S_OK ) )
			{
				odtLog << "Conversion was successful, but CanConvert reports that conversion is not supported\n";		
				fDataOK = FALSE;
				goto CLEANUP;				
			}
	}
CLEANUP:

	if( m_Binding.wType & DBTYPE_BYREF )
	{
		if( dbsActual == DBSTATUS_S_OK  )
			PROVIDER_FREE(pActual);
		if( dbsExpected == DBSTATUS_S_OK )
			PROVIDER_FREE(pExpected);
	}
	else if( fChangedFromStr )
	{
		PROVIDER_FREE(pActual);
		PROVIDER_FREE(pExpected);
	}
	else if( wDstType == DBTYPE_BSTR )
	{
		if( dbsActual == DBSTATUS_S_OK )
			SysFreeString(*(BSTR *)pActual);
		if( dbsExpected == DBSTATUS_S_OK )
			SysFreeString(*(BSTR *)pExpected);
	}

	return fDataOK;
}

//--------------------------------------------------------------------
//@mfunc: Loop through the rows and columns to perform the datacoercion
//
//--------------------------------------------------------------------
int TCDATALITE::GetDataCoerce(DBTYPE wType, eCoerceType eCrcType)
{
	BOOL		fTestPass = TEST_PASS;
	DBCOUNTITEM	cCols = g_pCTable->CountColumnsOnTable();
	DBCOUNTITEM	cRows = g_pCTable->GetRowsOnCTable();
	DBCOUNTITEM	ulColIndex = 0;
	DBCOUNTITEM	ulRowIndex = 0;
	DBCOUNTITEM	cRowsObtained = 0;
	HROW		rghrow[1];
	HROW*		prghRows = rghrow;
	CRowObject	RowObject;

	//TESTC(OpenRowset(IID_IRowset));
	TESTC_(CreateRowset(SELECT_ORDERBYNUMERIC, IID_IRowset, g_pCTable), S_OK);

	for ( ulRowIndex = 1; ulRowIndex <= cRows; ulRowIndex++ )
	{	
		if (IsGetColumnsType(eCrcType))
		{
			if( m_ulpOleObjects & (DBPROPVAL_OO_ROWOBJECT | DBPROPVAL_OO_SINGLETON) )
			{
				TEST2C_(m_hr = GetRowObject(ulRowIndex, &RowObject),S_OK,E_NOINTERFACE);
			}
			else
				m_hr = E_NOINTERFACE;

			if (m_hr == E_NOINTERFACE)
			{
				fTestPass = TEST_SKIPPED;
				odtLog << L"No Row Object support." << ENDL;
				goto CLEANUP;
			}
		}
		else
		{
			TESTC_(m_pIRowset->GetNextRows(NULL, 0, 1, &cRowsObtained, &prghRows), S_OK)
			TESTC(cRowsObtained == 1);
		}

		for ( ulColIndex = 1; ulColIndex <= cCols; ulColIndex++ )
		{
			m_hr = CreateAccessor(wType, eCrcType, ulRowIndex, ulColIndex);

			if (m_hr == S_FALSE)
				continue;
			else if (FAILED(m_hr))
			{
				odtLog << "Bad Accessor Validation at column: "<<ulColIndex<<" and row: "<<ulRowIndex<<".\n";
				fTestPass = TEST_FAIL;
				continue;
			}

			if (IsGetColumnsType(eCrcType))
			{
				m_hrActual = RowObject.GetColumns(1, &m_Binding, m_pData);
			}
			else
				FetchData(prghRows[0]);

			if (!COMPARE(VerifyResult(wType), TRUE))
			{
				odtLog << "GetData coerce error at column: "<<ulColIndex<<" and row: "<<ulRowIndex<<".\n\n";
				fTestPass = TEST_FAIL;
			}

			TESTC_(m_pIAccessor->ReleaseAccessor(m_hAccessor, NULL), S_OK);
		}

		if (IsGetColumnsType(eCrcType))
			RowObject.ReleaseRowObject();
		
		TESTC_(m_pIRowset->ReleaseRows(cRowsObtained, prghRows, NULL, NULL, NULL), S_OK);
	}

CLEANUP:
	DropRowset();
	return fTestPass;
}


void TCDATALITE::FindValidCoercions()
{
	DBTYPE wTypes[] = 
	 {	
	DBTYPE_I2,
	DBTYPE_I4,
	DBTYPE_R4,
	DBTYPE_R8,
	DBTYPE_CY,
	DBTYPE_DATE,
	DBTYPE_BSTR,
	DBTYPE_IDISPATCH,
	DBTYPE_ERROR,
	DBTYPE_BOOL,
	DBTYPE_VARIANT,
	DBTYPE_IUNKNOWN,
	DBTYPE_DECIMAL,
	DBTYPE_UI1,
	DBTYPE_I1,
	DBTYPE_UI2,
	DBTYPE_UI4,
	DBTYPE_I8,
	DBTYPE_UI8,
	DBTYPE_GUID,
	DBTYPE_VECTOR,
	DBTYPE_RESERVED,
	DBTYPE_BYTES,
	DBTYPE_STR,
	DBTYPE_WSTR,
	DBTYPE_NUMERIC,
	DBTYPE_UDT,
	DBTYPE_DBDATE,
	DBTYPE_DBTIME,
	DBTYPE_DBTIMESTAMP,
	DBTYPE_WSTR | DBTYPE_BYREF, 
	DBTYPE_STR | DBTYPE_BYREF,
	DBTYPE_PROPVARIANT,
	DBTYPE_FILETIME,
	DBTYPE_VARNUMERIC
    };

	WCHAR *wTypeNames[] = 
	 {	
	L"DBTYPE_I2",
	L"DBTYPE_I4",
	L"DBTYPE_R4",
	L"DBTYPE_R8",
	L"DBTYPE_CY",
	L"DBTYPE_DATE",
	L"DBTYPE_BSTR",
	L"DBTYPE_IDISPATCH",
	L"DBTYPE_ERROR",
	L"DBTYPE_BOOL",
	L"DBTYPE_VARIANT",
	L"DBTYPE_IUNKNOWN",
	L"DBTYPE_DECIMAL",
	L"DBTYPE_UI1",
	L"DBTYPE_I1",
	L"DBTYPE_UI2",
	L"DBTYPE_UI4",
	L"DBTYPE_I8",
	L"DBTYPE_UI8",
	L"DBTYPE_GUID",
	L"DBTYPE_VECTOR",
	L"DBTYPE_RESERVED",
	L"DBTYPE_BYTES",
	L"DBTYPE_STR",
	L"DBTYPE_WSTR",
	L"DBTYPE_NUMERIC",
	L"DBTYPE_UDT",
	L"DBTYPE_DBDATE",
	L"DBTYPE_DBTIME",
	L"DBTYPE_DBTIMESTAMP",
	L"DBTYPE_WSTR | DBTYPE_BYREF", 
	L"DBTYPE_STR | DBTYPE_BYREF",
	L"DBTYPE_PROPVARIANT",
	L"DBTYPE_FILETIME",
	L"DBTYPE_VARNUMERIC"
    };

	int i = 0, j = 0;
	HRESULT hr;

	for ( i = 0; i < sizeof(wTypes)/sizeof(DBTYPE); i++ )
	{
		for ( j = 0; j < sizeof(wTypes)/sizeof(DBTYPE); j++ )
		{
			odtLog << "Conversion from type: " << wTypeNames[i] << " to type: " << wTypeNames[j] << " is ";

			hr = m_pIConvertType->CanConvert(wTypes[i], wTypes[j], DBCONVERTFLAGS_COLUMN);

			if ( hr == S_OK )
				odtLog << "Supported.\n";
			else if ( hr == S_FALSE )
				odtLog << " NOT supported.\n";
			else
				odtLog << " Unknown. Bad types to the function\n";
		
		}
		odtLog << "\n";
	}

}


BOOL TCDATALITE::CompatibleStatus(DBSTATUS dbsActual, DBSTATUS dbsExpected)
{
	switch ( dbsActual )
	{
	case DBSTATUS_E_DATAOVERFLOW:
	case DBSTATUS_E_CANTCONVERTVALUE:
		return	(dbsExpected == DBSTATUS_E_CANTCONVERTVALUE ||
				dbsExpected == DBSTATUS_E_SIGNMISMATCH	||
				dbsExpected == DBSTATUS_E_DATAOVERFLOW );
	default:
		return (dbsActual == dbsExpected);
	}
}


ULONG TCDATALITE::GetPadSize(DBTYPE wType)
{
	// The OLE DB spec doesn't always define canonical string formats for 
	// conversion from DBTYPES to strings.
	// For example, individual providers may choose to convert certain
	// numeric types with trailing or leading zeros.

	switch(wType)
	{
	// provide a margin to account for varying precision, sign, leading/trailing zeroes
	case DBTYPE_R8:
		return 17;

	case DBTYPE_NUMERIC:
	case DBTYPE_R4:	
	case DBTYPE_DECIMAL:
	case DBTYPE_VARNUMERIC:
	case DBTYPE_CY:
		return 10;	

	case DBTYPE_DBTIMESTAMP:
		return 10;  // Some providers may choose to display the fractional seconds field differently

	case DBTYPE_DATE:
		return 20;

	default:
		return 0;
	}
}


BOOL TCDATALITE::IsGetColumnsType(eCoerceType eCoerceType)
{
	return (eCoerceType == eGETCOLUMNS || eCoerceType == eGETCOLUMNS_BYREF);
}


HRESULT TCDATALITE::CreateRowset
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
	HRESULT hr = CRowset::CreateRowset(eQuery, riid, pCTable, dwAccessorFlags, dwPart,
					dwColsToBind, eBindingOrder, eColsByRef, wTypeModifier, dwBlobType);

	if( SUCCEEDED(hr) )
		COMPARE(VerifyInterface(m_pIRowset, IID_IConvertType, ROWSET_INTERFACE, (IUnknown**)&m_pIConvertType), TRUE);

	return hr;
}

HRESULT TCDATALITE::DropRowset()
{
	SAFE_RELEASE(m_pIConvertType);

	return CRowset::DropRowset();	
}

// {{ TCW_TEST_CASE_MAP(TCDBTYPE_I2)
//--------------------------------------------------------------------
// @class Test DBTYPE_I2
//
class TCDBTYPE_I2 : public TCDATALITE {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCDBTYPE_I2,TCDATALITE);
	// }}
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember Normal
	int Variation_1();
	// @cmember Use BYREF
	int Variation_2();
	// @cmember GetColumns
	int Variation_3();
	// @cmember GetColumns BYREF
	int Variation_4();
	// }}
} ;
// {{ TCW_TESTCASE(TCDBTYPE_I2)
#define THE_CLASS TCDBTYPE_I2
BEG_TEST_CASE(TCDBTYPE_I2, TCDATALITE, L"Test DBTYPE_I2")
	TEST_VARIATION(1,		L"Normal")
	TEST_VARIATION(2,		L"Use BYREF")
	TEST_VARIATION(3,		L"GetColumns")
	TEST_VARIATION(4,		L"GetColumns BYREF")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(TCDBTYPE_I4)
//--------------------------------------------------------------------
// @class Test DBTYPE_I4
//
class TCDBTYPE_I4 : public TCDATALITE {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCDBTYPE_I4,TCDATALITE);
	// }}
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember Normal
	int Variation_1();
	// @cmember Use BYREF
	int Variation_2();
	// @cmember GetColumns
	int Variation_3();
	// @cmember GetColumns BYREF
	int Variation_4();
	// }}
} ;
// {{ TCW_TESTCASE(TCDBTYPE_I4)
#define THE_CLASS TCDBTYPE_I4
BEG_TEST_CASE(TCDBTYPE_I4, TCDATALITE, L"Test DBTYPE_I4")
	TEST_VARIATION(1,		L"Normal")
	TEST_VARIATION(2,		L"Use BYREF")
	TEST_VARIATION(3,		L"GetColumns")
	TEST_VARIATION(4,		L"GetColumns BYREF")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(TCDBTYPE_R4)
//--------------------------------------------------------------------
// @class Test DBTYPE_R4
//
class TCDBTYPE_R4 : public TCDATALITE {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCDBTYPE_R4,TCDATALITE);
	// }}
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember Normal
	int Variation_1();
	// @cmember Use BYREF
	int Variation_2();
	// @cmember GetColumns]
	int Variation_3();
	// @cmember GetColumns BYREF
	int Variation_4();
	// }}
} ;
// {{ TCW_TESTCASE(TCDBTYPE_R4)
#define THE_CLASS TCDBTYPE_R4
BEG_TEST_CASE(TCDBTYPE_R4, TCDATALITE, L"Test DBTYPE_R4")
	TEST_VARIATION(1,		L"Normal")
	TEST_VARIATION(2,		L"Use BYREF")
	TEST_VARIATION(3,		L"GetColumns]")
	TEST_VARIATION(4,		L"GetColumns BYREF")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(TCDBTYPE_R8)
//--------------------------------------------------------------------
// @class Test DBTYPE_R8
//
class TCDBTYPE_R8 : public TCDATALITE {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCDBTYPE_R8,TCDATALITE);
	// }}
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember Normal
	int Variation_1();
	// @cmember Use BYREF
	int Variation_2();
	// @cmember GetColumns
	int Variation_3();
	// @cmember GetColumns BYREF
	int Variation_4();
	// }}
} ;
// {{ TCW_TESTCASE(TCDBTYPE_R8)
#define THE_CLASS TCDBTYPE_R8
BEG_TEST_CASE(TCDBTYPE_R8, TCDATALITE, L"Test DBTYPE_R8")
	TEST_VARIATION(1,		L"Normal")
	TEST_VARIATION(2,		L"Use BYREF")
	TEST_VARIATION(3,		L"GetColumns")
	TEST_VARIATION(4,		L"GetColumns BYREF")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(TCDBTYPE_CY)
//--------------------------------------------------------------------
// @class Test DBTYPE_CY
//
class TCDBTYPE_CY : public TCDATALITE {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCDBTYPE_CY,TCDATALITE);
	// }}
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember Normal
	int Variation_1();
	// @cmember Use BYREF
	int Variation_2();
	// @cmember GetColumns
	int Variation_3();
	// @cmember GetColumns BYREF
	int Variation_4();
	// }}
} ;
// {{ TCW_TESTCASE(TCDBTYPE_CY)
#define THE_CLASS TCDBTYPE_CY
BEG_TEST_CASE(TCDBTYPE_CY, TCDATALITE, L"Test DBTYPE_CY")
	TEST_VARIATION(1,		L"Normal")
	TEST_VARIATION(2,		L"Use BYREF")
	TEST_VARIATION(3,		L"GetColumns")
	TEST_VARIATION(4,		L"GetColumns BYREF")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(TCDBTYPE_DATE)
//--------------------------------------------------------------------
// @class Test DBTYPE_DATE
//
class TCDBTYPE_DATE : public TCDATALITE {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCDBTYPE_DATE,TCDATALITE);
	// }}
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember Normal
	int Variation_1();
	// @cmember Use BYREF
	int Variation_2();
	// @cmember GetColumns
	int Variation_3();
	// @cmember GetColumns BYREF
	int Variation_4();
	// }}
} ;
// {{ TCW_TESTCASE(TCDBTYPE_DATE)
#define THE_CLASS TCDBTYPE_DATE
BEG_TEST_CASE(TCDBTYPE_DATE, TCDATALITE, L"Test DBTYPE_DATE")
	TEST_VARIATION(1,		L"Normal")
	TEST_VARIATION(2,		L"Use BYREF")
	TEST_VARIATION(3,		L"GetColumns")
	TEST_VARIATION(4,		L"GetColumns BYREF")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(TCDBTYPE_BSTR)
//--------------------------------------------------------------------
// @class Test DBTYPE_BSTR
//
class TCDBTYPE_BSTR : public TCDATALITE {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCDBTYPE_BSTR,TCDATALITE);
	// }}
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember Normal
	int Variation_1();
	// @cmember Use BYREF
	int Variation_2();
	// @cmember GetColumns
	int Variation_3();
	// @cmember GetColumns BYREF
	int Variation_4();
	// }}
} ;
// {{ TCW_TESTCASE(TCDBTYPE_BSTR)
#define THE_CLASS TCDBTYPE_BSTR
BEG_TEST_CASE(TCDBTYPE_BSTR, TCDATALITE, L"Test DBTYPE_BSTR")
	TEST_VARIATION(1,		L"Normal")
	TEST_VARIATION(2,		L"Use BYREF")
	TEST_VARIATION(3,		L"GetColumns")
	TEST_VARIATION(4,		L"GetColumns BYREF")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCDBTYPE_ERROR)
//--------------------------------------------------------------------
// @class Test DBTYPE_ERROR
//
class TCDBTYPE_ERROR : public TCDATALITE {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCDBTYPE_ERROR,TCDATALITE);
	// }}
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember Normal
	int Variation_1();
	// @cmember Use BYREF
	int Variation_2();
	// @cmember GetColumns
	int Variation_3();
	// @cmember GetColumns BYREF
	int Variation_4();
	// }}
} ;
// {{ TCW_TESTCASE(TCDBTYPE_ERROR)
#define THE_CLASS TCDBTYPE_ERROR
BEG_TEST_CASE(TCDBTYPE_ERROR, TCDATALITE, L"Test DBTYPE_ERROR")
	TEST_VARIATION(1,		L"Normal")
	TEST_VARIATION(2,		L"Use BYREF")
	TEST_VARIATION(3,		L"GetColumns")
	TEST_VARIATION(4,		L"GetColumns BYREF")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(TCDBTYPE_BOOL)
//--------------------------------------------------------------------
// @class Test DBTYPE_BOOL
//
class TCDBTYPE_BOOL : public TCDATALITE {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCDBTYPE_BOOL,TCDATALITE);
	// }}
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember Normal
	int Variation_1();
	// @cmember Use BYREF
	int Variation_2();
	// @cmember Truncation Case
	int Variation_3();
	// @cmember GetColumns
	int Variation_4();
	// @cmember GetColumns BYREF
	int Variation_5();
	// }}
} ;
// {{ TCW_TESTCASE(TCDBTYPE_BOOL)
#define THE_CLASS TCDBTYPE_BOOL
BEG_TEST_CASE(TCDBTYPE_BOOL, TCDATALITE, L"Test DBTYPE_BOOL")
	TEST_VARIATION(1,		L"Normal")
	TEST_VARIATION(2,		L"Use BYREF")
	TEST_VARIATION(3,		L"Truncation Case")
	TEST_VARIATION(4,		L"GetColumns")
	TEST_VARIATION(5,		L"GetColumns BYREF")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(TCDBTYPE_VARIANT)
//--------------------------------------------------------------------
// @class Test DBTYPE_VARIANT
//
class TCDBTYPE_VARIANT : public TCDATALITE {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCDBTYPE_VARIANT,TCDATALITE);
	// }}
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember Normal
	int Variation_1();
	// @cmember Use BYREF
	int Variation_2();
	// @cmember GetColumns
	int Variation_3();
	// @cmember GetColumns BYREF
	int Variation_4();
	// }}
} ;
// {{ TCW_TESTCASE(TCDBTYPE_VARIANT)
#define THE_CLASS TCDBTYPE_VARIANT
BEG_TEST_CASE(TCDBTYPE_VARIANT, TCDATALITE, L"Test DBTYPE_VARIANT")
	TEST_VARIATION(1,		L"Normal")
	TEST_VARIATION(2,		L"Use BYREF")
	TEST_VARIATION(3,		L"GetColumns")
	TEST_VARIATION(4,		L"GetColumns BYREF")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCDBTYPE_DECIMAL)
//--------------------------------------------------------------------
// @class Test DBTYPE_DECIMAL
//
class TCDBTYPE_DECIMAL : public TCDATALITE {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCDBTYPE_DECIMAL,TCDATALITE);
	// }}
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember Normal
	int Variation_1();
	// @cmember Use BYREF
	int Variation_2();
	// @cmember GetColumns
	int Variation_3();
	// @cmember GetColumns BYREF
	int Variation_4();
	// }}
} ;
// {{ TCW_TESTCASE(TCDBTYPE_DECIMAL)
#define THE_CLASS TCDBTYPE_DECIMAL
BEG_TEST_CASE(TCDBTYPE_DECIMAL, TCDATALITE, L"Test DBTYPE_DECIMAL")
	TEST_VARIATION(1,		L"Normal")
	TEST_VARIATION(2,		L"Use BYREF")
	TEST_VARIATION(3,		L"GetColumns")
	TEST_VARIATION(4,		L"GetColumns BYREF")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(TCDBTYPE_UI1)
//--------------------------------------------------------------------
// @class Test DBTYPE_UI1
//
class TCDBTYPE_UI1 : public TCDATALITE {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCDBTYPE_UI1,TCDATALITE);
	// }}
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember Normal
	int Variation_1();
	// @cmember Use BYREF
	int Variation_2();
	// @cmember GetColumns
	int Variation_3();
	// @cmember GetColumns BYREF
	int Variation_4();
	// }}
} ;
// {{ TCW_TESTCASE(TCDBTYPE_UI1)
#define THE_CLASS TCDBTYPE_UI1
BEG_TEST_CASE(TCDBTYPE_UI1, TCDATALITE, L"Test DBTYPE_UI1")
	TEST_VARIATION(1,		L"Normal")
	TEST_VARIATION(2,		L"Use BYREF")
	TEST_VARIATION(3,		L"GetColumns")
	TEST_VARIATION(4,		L"GetColumns BYREF")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(TCDBTYPE_I1)
//--------------------------------------------------------------------
// @class Test DBTYPE_I1
//
class TCDBTYPE_I1 : public TCDATALITE {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCDBTYPE_I1,TCDATALITE);
	// }}
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember Normal
	int Variation_1();
	// @cmember Use BYREF
	int Variation_2();
	// @cmember GetColumns
	int Variation_3();
	// @cmember GetColumns BYREF
	int Variation_4();
	// }}
} ;
// {{ TCW_TESTCASE(TCDBTYPE_I1)
#define THE_CLASS TCDBTYPE_I1
BEG_TEST_CASE(TCDBTYPE_I1, TCDATALITE, L"Test DBTYPE_I1")
	TEST_VARIATION(1,		L"Normal")
	TEST_VARIATION(2,		L"Use BYREF")
	TEST_VARIATION(3,		L"GetColumns")
	TEST_VARIATION(4,		L"GetColumns BYREF")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(TCDBTYPE_UI2)
//--------------------------------------------------------------------
// @class Test DBTYPE_UI2
//
class TCDBTYPE_UI2 : public TCDATALITE {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCDBTYPE_UI2,TCDATALITE);
	// }}
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember Normal
	int Variation_1();
	// @cmember Use BYREF
	int Variation_2();
	// @cmember GetColumns
	int Variation_3();
	// @cmember GetColumns BYREF
	int Variation_4();
	// }}
} ;
// {{ TCW_TESTCASE(TCDBTYPE_UI2)
#define THE_CLASS TCDBTYPE_UI2
BEG_TEST_CASE(TCDBTYPE_UI2, TCDATALITE, L"Test DBTYPE_UI2")
	TEST_VARIATION(1,		L"Normal")
	TEST_VARIATION(2,		L"Use BYREF")
	TEST_VARIATION(3,		L"GetColumns")
	TEST_VARIATION(4,		L"GetColumns BYREF")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(TCDBTYPE_UI4)
//--------------------------------------------------------------------
// @class Test DBTYPE_UI4
//
class TCDBTYPE_UI4 : public TCDATALITE {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCDBTYPE_UI4,TCDATALITE);
	// }}
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember Normal
	int Variation_1();
	// @cmember Use BYREF
	int Variation_2();
	// @cmember GetColumns
	int Variation_3();
	// @cmember GetColumns BYREF
	int Variation_4();
	// }}
} ;
// {{ TCW_TESTCASE(TCDBTYPE_UI4)
#define THE_CLASS TCDBTYPE_UI4
BEG_TEST_CASE(TCDBTYPE_UI4, TCDATALITE, L"Test DBTYPE_UI4")
	TEST_VARIATION(1,		L"Normal")
	TEST_VARIATION(2,		L"Use BYREF")
	TEST_VARIATION(3,		L"GetColumns")
	TEST_VARIATION(4,		L"GetColumns BYREF")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(TCDBTYPE_I8)
//--------------------------------------------------------------------
// @class Test DBTYPE_I8
//
class TCDBTYPE_I8 : public TCDATALITE {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCDBTYPE_I8,TCDATALITE);
	// }}
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember Normal
	int Variation_1();
	// @cmember Use BYREF
	int Variation_2();
	// @cmember GetColumns
	int Variation_3();
	// @cmember GetColumns BYREF
	int Variation_4();
	// }}
} ;
// {{ TCW_TESTCASE(TCDBTYPE_I8)
#define THE_CLASS TCDBTYPE_I8
BEG_TEST_CASE(TCDBTYPE_I8, TCDATALITE, L"Test DBTYPE_I8")
	TEST_VARIATION(1,		L"Normal")
	TEST_VARIATION(2,		L"Use BYREF")
	TEST_VARIATION(3,		L"GetColumns")
	TEST_VARIATION(4,		L"GetColumns BYREF")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(TCDBTYPE_UI8)
//--------------------------------------------------------------------
// @class Test DBTYPE_UI8
//
class TCDBTYPE_UI8 : public TCDATALITE {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCDBTYPE_UI8,TCDATALITE);
	// }}
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember Normal
	int Variation_1();
	// @cmember Use BYREF
	int Variation_2();
	// @cmember GetColumns
	int Variation_3();
	// @cmember GetColumns BYREF
	int Variation_4();
	// }}
} ;
// {{ TCW_TESTCASE(TCDBTYPE_UI8)
#define THE_CLASS TCDBTYPE_UI8
BEG_TEST_CASE(TCDBTYPE_UI8, TCDATALITE, L"Test DBTYPE_UI8")
	TEST_VARIATION(1,		L"Normal")
	TEST_VARIATION(2,		L"Use BYREF")
	TEST_VARIATION(3,		L"GetColumns")
	TEST_VARIATION(4,		L"GetColumns BYREF")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(TCDBTYPE_GUID)
//--------------------------------------------------------------------
// @class Test DBTYPE_GUID
//
class TCDBTYPE_GUID : public TCDATALITE {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCDBTYPE_GUID,TCDATALITE);
	// }}
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember Normal
	int Variation_1();
	// @cmember Use BYREF
	int Variation_2();
	// @cmember GetColumns
	int Variation_3();
	// @cmember GetColumns BYREF
	int Variation_4();
	// }}
} ;
// {{ TCW_TESTCASE(TCDBTYPE_GUID)
#define THE_CLASS TCDBTYPE_GUID
BEG_TEST_CASE(TCDBTYPE_GUID, TCDATALITE, L"Test DBTYPE_GUID")
	TEST_VARIATION(1,		L"Normal")
	TEST_VARIATION(2,		L"Use BYREF")
	TEST_VARIATION(3,		L"GetColumns")
	TEST_VARIATION(4,		L"GetColumns BYREF")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(TCDBTYPE_BYTES)
//--------------------------------------------------------------------
// @class Test DBTYPE_BYTES
//
class TCDBTYPE_BYTES : public TCDATALITE {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCDBTYPE_BYTES,TCDATALITE);
	// }}
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember Normal
	int Variation_1();
	// @cmember Use BYREF
	int Variation_2();
	// @cmember Truncation Case
	int Variation_3();
	// @cmember GetColumns
	int Variation_4();
	// @cmember GetColumns BYREF
	int Variation_5();
	// @cmember GetColumns Truncation
	int Variation_6();
	// }}
} ;
// {{ TCW_TESTCASE(TCDBTYPE_BYTES)
#define THE_CLASS TCDBTYPE_BYTES
BEG_TEST_CASE(TCDBTYPE_BYTES, TCDATALITE, L"Test DBTYPE_BYTES")
	TEST_VARIATION(1,		L"Normal")
	TEST_VARIATION(2,		L"Use BYREF")
	TEST_VARIATION(3,		L"Truncation Case")
	TEST_VARIATION(4,		L"GetColumns")
	TEST_VARIATION(5,		L"GetColumns BYREF")
	TEST_VARIATION(6,		L"GetColumns Truncation")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(TCDBTYPE_STR)
//--------------------------------------------------------------------
// @class Test DBTYPE_STR
//
class TCDBTYPE_STR : public TCDATALITE {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCDBTYPE_STR,TCDATALITE);
	// }}
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember Normal
	int Variation_1();
	// @cmember Use BYREF
	int Variation_2();
	// @cmember Truncation Case
	int Variation_3();
	// @cmember GetColumns
	int Variation_4();
	// @cmember GetColumns BYREF
	int Variation_5();
	// @cmember GetColumns BYREF Truncation
	int Variation_6();
	// }}
} ;
// {{ TCW_TESTCASE(TCDBTYPE_STR)
#define THE_CLASS TCDBTYPE_STR
BEG_TEST_CASE(TCDBTYPE_STR, TCDATALITE, L"Test DBTYPE_STR")
	TEST_VARIATION(1,		L"Normal")
	TEST_VARIATION(2,		L"Use BYREF")
	TEST_VARIATION(3,		L"Truncation Case")
	TEST_VARIATION(4,		L"GetColumns")
	TEST_VARIATION(5,		L"GetColumns BYREF")
	TEST_VARIATION(6,		L"GetColumns BYREF Truncation")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(TCDBTYPE_WSTR)
//--------------------------------------------------------------------
// @class Test DBTYPE_WSTR
//
class TCDBTYPE_WSTR : public TCDATALITE {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCDBTYPE_WSTR,TCDATALITE);
	// }}
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember Normal
	int Variation_1();
	// @cmember Use BYREF
	int Variation_2();
	// @cmember Truncation Case
	int Variation_3();
	// @cmember GetColumns
	int Variation_4();
	// @cmember GetColumns BYREF
	int Variation_5();
	// @cmember GetColumns BYREF Truncation
	int Variation_6();
	// }}
} ;
// {{ TCW_TESTCASE(TCDBTYPE_WSTR)
#define THE_CLASS TCDBTYPE_WSTR
BEG_TEST_CASE(TCDBTYPE_WSTR, TCDATALITE, L"Test DBTYPE_WSTR")
	TEST_VARIATION(1,		L"Normal")
	TEST_VARIATION(2,		L"Use BYREF")
	TEST_VARIATION(3,		L"Truncation Case")
	TEST_VARIATION(4,		L"GetColumns")
	TEST_VARIATION(5,		L"GetColumns BYREF")
	TEST_VARIATION(6,		L"GetColumns BYREF Truncation")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(TCDBTYPE_NUMERIC)
//--------------------------------------------------------------------
// @class Test DBTYPE_NUMERIC
//
class TCDBTYPE_NUMERIC : public TCDATALITE {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCDBTYPE_NUMERIC,TCDATALITE);
	// }}
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember Normal
	int Variation_1();
	// @cmember Use BYREF
	int Variation_2();
	// @cmember GetColumns
	int Variation_3();
	// @cmember GetColumns BYREF
	int Variation_4();
	// }}
} ;
// {{ TCW_TESTCASE(TCDBTYPE_NUMERIC)
#define THE_CLASS TCDBTYPE_NUMERIC
BEG_TEST_CASE(TCDBTYPE_NUMERIC, TCDATALITE, L"Test DBTYPE_NUMERIC")
	TEST_VARIATION(1,		L"Normal")
	TEST_VARIATION(2,		L"Use BYREF")
	TEST_VARIATION(3,		L"GetColumns")
	TEST_VARIATION(4,		L"GetColumns BYREF")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCDBTYPE_DBDATE)
//--------------------------------------------------------------------
// @class Test DBTYPE_DBDATE
//
class TCDBTYPE_DBDATE : public TCDATALITE {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCDBTYPE_DBDATE,TCDATALITE);
	// }}
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember Normal
	int Variation_1();
	// @cmember Use BYREF
	int Variation_2();
	// @cmember GetColumns
	int Variation_3();
	// @cmember GetColumns BYREF
	int Variation_4();
	// }}
} ;
// {{ TCW_TESTCASE(TCDBTYPE_DBDATE)
#define THE_CLASS TCDBTYPE_DBDATE
BEG_TEST_CASE(TCDBTYPE_DBDATE, TCDATALITE, L"Test DBTYPE_DBDATE")
	TEST_VARIATION(1,		L"Normal")
	TEST_VARIATION(2,		L"Use BYREF")
	TEST_VARIATION(3,		L"GetColumns")
	TEST_VARIATION(4,		L"GetColumns BYREF")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(TCDBTYPE_DBTIME)
//--------------------------------------------------------------------
// @class Test DBTYPE_DBTIME
//
class TCDBTYPE_DBTIME : public TCDATALITE {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCDBTYPE_DBTIME,TCDATALITE);
	// }}
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember Normal
	int Variation_1();
	// @cmember Use BYREF
	int Variation_2();
	// @cmember GetColumns
	int Variation_3();
	// @cmember GetColumns BYREF
	int Variation_4();
	// }}
} ;
// {{ TCW_TESTCASE(TCDBTYPE_DBTIME)
#define THE_CLASS TCDBTYPE_DBTIME
BEG_TEST_CASE(TCDBTYPE_DBTIME, TCDATALITE, L"Test DBTYPE_DBTIME")
	TEST_VARIATION(1,		L"Normal")
	TEST_VARIATION(2,		L"Use BYREF")
	TEST_VARIATION(3,		L"GetColumns")
	TEST_VARIATION(4,		L"GetColumns BYREF")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(TCDBTYPE_DBTIMESTAMP)
//--------------------------------------------------------------------
// @class Test DBTYPE_DBTIMESTAMP
//
class TCDBTYPE_DBTIMESTAMP : public TCDATALITE {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCDBTYPE_DBTIMESTAMP,TCDATALITE);
	// }}
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember Normal
	int Variation_1();
	// @cmember Use BYREF
	int Variation_2();
	// @cmember GetColumns
	int Variation_3();
	// @cmember GetColumns BYREF
	int Variation_4();
	// }}
} ;
// {{ TCW_TESTCASE(TCDBTYPE_DBTIMESTAMP)
#define THE_CLASS TCDBTYPE_DBTIMESTAMP
BEG_TEST_CASE(TCDBTYPE_DBTIMESTAMP, TCDATALITE, L"Test DBTYPE_DBTIMESTAMP")
	TEST_VARIATION(1,		L"Normal")
	TEST_VARIATION(2,		L"Use BYREF")
	TEST_VARIATION(3,		L"GetColumns")
	TEST_VARIATION(4,		L"GetColumns BYREF")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }} TCW_TEST_CASE_MAP_END


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Test Case Section
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


// {{ TCW_TEST_CASE_MAP(TCIConvetType)
//--------------------------------------------------------------------
// @class Display supported conversion
//
class TCIConvetType : public TCDATALITE {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCIConvetType,TCDATALITE);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Display all conversion
	int Variation_1();
	// }}
};
// {{ TCW_TESTCASE(TCIConvetType)
#define THE_CLASS TCIConvetType
BEG_TEST_CASE(TCIConvetType, TCDATALITE, L"Display supported conversion")
	TEST_VARIATION(1,		L"Display all conversion")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}


// {{ TCW_TEST_CASE_MAP(TCDBTYPE_VARNUMERIC)
//--------------------------------------------------------------------
// @class Test VARNUMERIC conversions
//
class TCDBTYPE_VARNUMERIC : public TCDATALITE {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCDBTYPE_VARNUMERIC,TCDATALITE);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Normal
	int Variation_1();
	// @cmember Truncation
	int Variation_2();
	// @cmember GetColumns
	int Variation_3();
	// @cmember GetColumns BYREF
	int Variation_4();
	// @cmember GetColumns Truncation
	int Variation_5();
	// @cmember Normal BYREF
	int Variation_6();
	// }}
};
// {{ TCW_TESTCASE(TCDBTYPE_VARNUMERIC)
#define THE_CLASS TCDBTYPE_VARNUMERIC
BEG_TEST_CASE(TCDBTYPE_VARNUMERIC, TCDATALITE, L"Test VARNUMERIC conversions")
	TEST_VARIATION(1,		L"Normal")
	TEST_VARIATION(2,		L"Truncation")
	TEST_VARIATION(3,		L"GetColumns")
	TEST_VARIATION(4,		L"GetColumns BYREF")
	TEST_VARIATION(5,		L"GetColumns Truncation")
	TEST_VARIATION(6,		L"Normal BYREF")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}


// }} END_DECLARE_TEST_CASES()

// {{ TCW_TESTMODULE(ThisModule)
TEST_MODULE(27, ThisModule, gwszModuleDescrip)
	TEST_CASE(1, TCDBTYPE_I2)
	TEST_CASE(2, TCDBTYPE_I4)
	TEST_CASE(3, TCDBTYPE_R4)
	TEST_CASE(4, TCDBTYPE_R8)
	TEST_CASE(5, TCDBTYPE_CY)
	TEST_CASE(6, TCDBTYPE_DATE)
	TEST_CASE(7, TCDBTYPE_BSTR)
	TEST_CASE(8, TCDBTYPE_ERROR)
	TEST_CASE(9, TCDBTYPE_BOOL)
	TEST_CASE(10, TCDBTYPE_VARIANT)
	TEST_CASE(11, TCDBTYPE_DECIMAL)
	TEST_CASE(12, TCDBTYPE_UI1)
	TEST_CASE(13, TCDBTYPE_I1)
	TEST_CASE(14, TCDBTYPE_UI2)
	TEST_CASE(15, TCDBTYPE_UI4)
	TEST_CASE(16, TCDBTYPE_I8)
	TEST_CASE(17, TCDBTYPE_UI8)
	TEST_CASE(18, TCDBTYPE_GUID)
	TEST_CASE(19, TCDBTYPE_BYTES)
	TEST_CASE(20, TCDBTYPE_STR)
	TEST_CASE(21, TCDBTYPE_WSTR)
	TEST_CASE(22, TCDBTYPE_NUMERIC)
	TEST_CASE(23, TCDBTYPE_DBDATE)
	TEST_CASE(24, TCDBTYPE_DBTIME)
	TEST_CASE(25, TCDBTYPE_DBTIMESTAMP)
	TEST_CASE(26, TCIConvetType)
	TEST_CASE(27, TCDBTYPE_VARNUMERIC)
END_TEST_MODULE()
// }}


// {{ TCW_TC_PROTOTYPE(TCDBTYPE_I2)
//*-----------------------------------------------------------------------
//|	Test Case:		TCDBTYPE_I2 - Test DBTYPE_I2
//| Created:  	9/27/97
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCDBTYPE_I2::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCDATALITE::Init())
	// }}
	{ 
		// TO DO:  Add your own code here 
		return TRUE;
	} 
	return FALSE;
} 


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Normal
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_I2::Variation_1()
{ 
	return GetDataCoerce(DBTYPE_I2, eNORMAL);
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Use BYREF
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_I2::Variation_2()
{ 
	return GetDataCoerce(DBTYPE_I2, eBYREF);
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc GetColumns
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_I2::Variation_3()
{
	return GetDataCoerce(DBTYPE_I2, eGETCOLUMNS);
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc GetColumns BYREF
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_I2::Variation_4()
{
	return GetDataCoerce(DBTYPE_I2, eGETCOLUMNS_BYREF);
}
// }}
// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCDBTYPE_I2::Terminate()
{ 
	// TO DO:  Add your own code here 

// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCDATALITE::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END


// {{ TCW_TC_PROTOTYPE(TCDBTYPE_I4)
//*-----------------------------------------------------------------------
//|	Test Case:		TCDBTYPE_I4 - Test DBTYPE_I4
//| Created:  	9/27/97
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCDBTYPE_I4::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCDATALITE::Init())
	// }}
	{ 
		// TO DO:  Add your own code here 
		return TRUE;
	} 
	return FALSE;
} 


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Normal
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_I4::Variation_1()
{ 
	return GetDataCoerce(DBTYPE_I4, eNORMAL);
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Use BYREF
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_I4::Variation_2()
{ 
	return GetDataCoerce(DBTYPE_I4, eBYREF);
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc GetColumns
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_I4::Variation_3()
{
	return GetDataCoerce(DBTYPE_I4, eGETCOLUMNS);
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc GetColumns BYREF
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_I4::Variation_4()
{
	return GetDataCoerce(DBTYPE_I4, eGETCOLUMNS_BYREF);
}
// }}
// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCDBTYPE_I4::Terminate()
{ 
	// TO DO:  Add your own code here 

// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCDATALITE::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END


// {{ TCW_TC_PROTOTYPE(TCDBTYPE_R4)
//*-----------------------------------------------------------------------
//|	Test Case:		TCDBTYPE_R4 - Test DBTYPE_R4
//| Created:  	9/27/97
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCDBTYPE_R4::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCDATALITE::Init())
	// }}
	{ 
		// TO DO:  Add your own code here 
		return TRUE;
	} 
	return FALSE;
} 


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Normal
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_R4::Variation_1()
{ 
	return GetDataCoerce(DBTYPE_R4, eNORMAL);
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Use BYREF
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_R4::Variation_2()
{ 
	return GetDataCoerce(DBTYPE_R4, eBYREF);
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc GetColumns]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_R4::Variation_3()
{
	return GetDataCoerce(DBTYPE_R4, eGETCOLUMNS);
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc GetColumns BYREF
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_R4::Variation_4()
{
	return GetDataCoerce(DBTYPE_R4, eGETCOLUMNS_BYREF);
}
// }}
// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCDBTYPE_R4::Terminate()
{ 
	// TO DO:  Add your own code here 

// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCDATALITE::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END


// {{ TCW_TC_PROTOTYPE(TCDBTYPE_R8)
//*-----------------------------------------------------------------------
//|	Test Case:		TCDBTYPE_R8 - Test DBTYPE_R8
//| Created:  	9/27/97
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCDBTYPE_R8::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCDATALITE::Init())
	// }}
	{ 
		// TO DO:  Add your own code here 
		return TRUE;
	} 
	return FALSE;
} 


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Normal
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_R8::Variation_1()
{ 
	return GetDataCoerce(DBTYPE_R8, eNORMAL);
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Use BYREF
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_R8::Variation_2()
{ 
	return GetDataCoerce(DBTYPE_R8, eBYREF);
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc GetColumns
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_R8::Variation_3()
{
	return GetDataCoerce(DBTYPE_R8, eGETCOLUMNS);
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc GetColumns BYREF
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_R8::Variation_4()
{
	return GetDataCoerce(DBTYPE_R8, eGETCOLUMNS_BYREF);
}
// }}
// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCDBTYPE_R8::Terminate()
{ 
	// TO DO:  Add your own code here 

// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCDATALITE::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END


// {{ TCW_TC_PROTOTYPE(TCDBTYPE_CY)
//*-----------------------------------------------------------------------
//|	Test Case:		TCDBTYPE_CY - Test DBTYPE_CY
//| Created:  	9/27/97
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCDBTYPE_CY::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCDATALITE::Init())
	// }}
	{ 
		// TO DO:  Add your own code here 
		return TRUE;
	} 
	return FALSE;
} 


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Normal
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_CY::Variation_1()
{ 
	return GetDataCoerce(DBTYPE_CY, eNORMAL);
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Use BYREF
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_CY::Variation_2()
{ 
	return GetDataCoerce(DBTYPE_CY, eBYREF);
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc GetColumns
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_CY::Variation_3()
{
	return GetDataCoerce(DBTYPE_CY, eGETCOLUMNS);
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc GetColumns BYREF
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_CY::Variation_4()
{
	return GetDataCoerce(DBTYPE_CY, eGETCOLUMNS_BYREF);
}
// }}
// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCDBTYPE_CY::Terminate()
{ 
	// TO DO:  Add your own code here 

// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCDATALITE::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END


// {{ TCW_TC_PROTOTYPE(TCDBTYPE_DATE)
//*-----------------------------------------------------------------------
//|	Test Case:		TCDBTYPE_DATE - Test DBTYPE_DATE
//| Created:  	9/27/97
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCDBTYPE_DATE::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCDATALITE::Init())
	// }}
	{ 
		// TO DO:  Add your own code here 
		return TRUE;
	} 
	return FALSE;
} 


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Normal
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_DATE::Variation_1()
{ 
	return GetDataCoerce(DBTYPE_DATE, eNORMAL);
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Use BYREF
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_DATE::Variation_2()
{ 
	return GetDataCoerce(DBTYPE_DATE, eBYREF);
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc GetColumns
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_DATE::Variation_3()
{
	return GetDataCoerce(DBTYPE_DATE, eGETCOLUMNS);
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc GetColumns BYREF
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_DATE::Variation_4()
{
	return GetDataCoerce(DBTYPE_DATE, eGETCOLUMNS);
}
// }}
// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCDBTYPE_DATE::Terminate()
{ 
	// TO DO:  Add your own code here 

// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCDATALITE::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END


// {{ TCW_TC_PROTOTYPE(TCDBTYPE_BSTR)
//*-----------------------------------------------------------------------
//|	Test Case:		TCDBTYPE_BSTR - Test DBTYPE_BSTR
//| Created:  	9/27/97
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCDBTYPE_BSTR::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCDATALITE::Init())
	// }}
	{ 
		// TO DO:  Add your own code here 
		return TRUE;
	} 
	return FALSE;
} 


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Normal
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_BSTR::Variation_1()
{ 
	return GetDataCoerce(DBTYPE_BSTR, eNORMAL);
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Use BYREF
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_BSTR::Variation_2()
{ 
	return GetDataCoerce(DBTYPE_BSTR, eBYREF);
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc GetColumns
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_BSTR::Variation_3()
{
	return GetDataCoerce(DBTYPE_BSTR, eGETCOLUMNS);
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc GetColumns BYREF
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_BSTR::Variation_4()
{
	return GetDataCoerce(DBTYPE_BSTR, eGETCOLUMNS_BYREF);
}
// }}
// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCDBTYPE_BSTR::Terminate()
{ 
	// TO DO:  Add your own code here 

// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCDATALITE::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END


// {{ TCW_TC_PROTOTYPE(TCDBTYPE_ERROR)
//*-----------------------------------------------------------------------
//|	Test Case:		TCDBTYPE_ERROR - Test DBTYPE_ERROR
//| Created:  	9/27/97
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCDBTYPE_ERROR::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCDATALITE::Init())
	// }}
	{ 
		// TO DO:  Add your own code here 
		return TRUE;
	} 
	return FALSE;
} 


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Normal
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_ERROR::Variation_1()
{ 
	return GetDataCoerce(DBTYPE_ERROR, eNORMAL);
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Use BYREF
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_ERROR::Variation_2()
{ 
	return GetDataCoerce(DBTYPE_ERROR, eBYREF);
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc GetColumns
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_ERROR::Variation_3()
{
	return GetDataCoerce(DBTYPE_ERROR, eGETCOLUMNS);
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc GetColumns BYREF
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_ERROR::Variation_4()
{
	return GetDataCoerce(DBTYPE_ERROR, eGETCOLUMNS_BYREF);
}
// }}
// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCDBTYPE_ERROR::Terminate()
{ 
	// TO DO:  Add your own code here 

// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCDATALITE::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END


// {{ TCW_TC_PROTOTYPE(TCDBTYPE_BOOL)
//*-----------------------------------------------------------------------
//|	Test Case:		TCDBTYPE_BOOL - Test DBTYPE_BOOL
//| Created:  	9/27/97
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCDBTYPE_BOOL::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCDATALITE::Init())
	// }}
	{ 
		// TO DO:  Add your own code here 
		return TRUE;
	} 
	return FALSE;
} 


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Normal
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_BOOL::Variation_1()
{ 
	return GetDataCoerce(DBTYPE_BOOL, eNORMAL);
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Use BYREF
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_BOOL::Variation_2()
{ 
	return GetDataCoerce(DBTYPE_BOOL, eBYREF);
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Truncation Case
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_BOOL::Variation_3()
{ 
	return GetDataCoerce(DBTYPE_BOOL, eTRUNCATE);
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc GetColumns
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_BOOL::Variation_4()
{
	return GetDataCoerce(DBTYPE_BOOL, eGETCOLUMNS);
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc GetColumns BYREF
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_BOOL::Variation_5()
{
	return GetDataCoerce(DBTYPE_BOOL, eGETCOLUMNS_BYREF);
}
// }}
// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCDBTYPE_BOOL::Terminate()
{ 
	// TO DO:  Add your own code here 

// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCDATALITE::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END


// {{ TCW_TC_PROTOTYPE(TCDBTYPE_VARIANT)
//*-----------------------------------------------------------------------
//|	Test Case:		TCDBTYPE_VARIANT - Test DBTYPE_VARIANT
//| Created:  	9/27/97
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCDBTYPE_VARIANT::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCDATALITE::Init())
	// }}
	{ 
		// TO DO:  Add your own code here 
		return TRUE;
	} 
	return FALSE;
} 


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Normal
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_VARIANT::Variation_1()
{ 
	return GetDataCoerce(DBTYPE_VARIANT, eNORMAL);
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Use BYREF
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_VARIANT::Variation_2()
{ 
	return GetDataCoerce(DBTYPE_VARIANT, eBYREF);
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc GetColumns
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_VARIANT::Variation_3()
{
	return GetDataCoerce(DBTYPE_VARIANT, eGETCOLUMNS);
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc GetColumns BYREF
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_VARIANT::Variation_4()
{
	return GetDataCoerce(DBTYPE_VARIANT, eGETCOLUMNS_BYREF);
}
// }}
// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCDBTYPE_VARIANT::Terminate()
{ 
	// TO DO:  Add your own code here 

// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCDATALITE::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END


// {{ TCW_TC_PROTOTYPE(TCDBTYPE_DECIMAL)
//*-----------------------------------------------------------------------
//|	Test Case:		TCDBTYPE_DECIMAL - Test DBTYPE_DECIMAL
//| Created:  	9/27/97
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCDBTYPE_DECIMAL::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCDATALITE::Init())
	// }}
	{ 
		// TO DO:  Add your own code here 
		return TRUE;
	} 
	return FALSE;
} 


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Normal
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_DECIMAL::Variation_1()
{ 
	return GetDataCoerce(DBTYPE_DECIMAL, eNORMAL);
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Use BYREF
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_DECIMAL::Variation_2()
{ 
	return GetDataCoerce(DBTYPE_DECIMAL, eBYREF);
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc GetColumns
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_DECIMAL::Variation_3()
{
	return GetDataCoerce(DBTYPE_DECIMAL, eGETCOLUMNS);
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc GetColumns BYREF
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_DECIMAL::Variation_4()
{
	return GetDataCoerce(DBTYPE_DECIMAL, eGETCOLUMNS_BYREF);
}
// }}
// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCDBTYPE_DECIMAL::Terminate()
{ 
	// TO DO:  Add your own code here 

// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCDATALITE::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END


// {{ TCW_TC_PROTOTYPE(TCDBTYPE_UI1)
//*-----------------------------------------------------------------------
//|	Test Case:		TCDBTYPE_UI1 - Test DBTYPE_UI1
//| Created:  	9/27/97
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCDBTYPE_UI1::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCDATALITE::Init())
	// }}
	{ 
		// TO DO:  Add your own code here 
		return TRUE;
	} 
	return FALSE;
} 


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Normal
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_UI1::Variation_1()
{ 
	return GetDataCoerce(DBTYPE_UI1, eNORMAL);
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Use BYREF
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_UI1::Variation_2()
{ 
	return GetDataCoerce(DBTYPE_UI1, eBYREF);
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc GetColumns
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_UI1::Variation_3()
{
	return GetDataCoerce(DBTYPE_UI1, eGETCOLUMNS);
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc GetColumns BYREF
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_UI1::Variation_4()
{
	return GetDataCoerce(DBTYPE_UI1, eGETCOLUMNS_BYREF);
}
// }}
// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCDBTYPE_UI1::Terminate()
{ 
	// TO DO:  Add your own code here 

// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCDATALITE::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END


// {{ TCW_TC_PROTOTYPE(TCDBTYPE_I1)
//*-----------------------------------------------------------------------
//|	Test Case:		TCDBTYPE_I1 - Test DBTYPE_I1
//| Created:  	9/27/97
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCDBTYPE_I1::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCDATALITE::Init())
	// }}
	{ 
		// TO DO:  Add your own code here 
		return TRUE;
	} 
	return FALSE;
} 


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Normal
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_I1::Variation_1()
{ 
	return GetDataCoerce(DBTYPE_I1, eNORMAL);
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Use BYREF
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_I1::Variation_2()
{ 
	return GetDataCoerce(DBTYPE_I1, eBYREF);
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc GetColumns
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_I1::Variation_3()
{
	return GetDataCoerce(DBTYPE_I1, eGETCOLUMNS);
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc GetColumns BYREF
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_I1::Variation_4()
{
	return GetDataCoerce(DBTYPE_I1, eGETCOLUMNS_BYREF);
}
// }}
// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCDBTYPE_I1::Terminate()
{ 
	// TO DO:  Add your own code here 

// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCDATALITE::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END


// {{ TCW_TC_PROTOTYPE(TCDBTYPE_UI2)
//*-----------------------------------------------------------------------
//|	Test Case:		TCDBTYPE_UI2 - Test DBTYPE_UI2
//| Created:  	9/27/97
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCDBTYPE_UI2::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCDATALITE::Init())
	// }}
	{ 
		// TO DO:  Add your own code here 
		return TRUE;
	} 
	return FALSE;
} 


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Normal
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_UI2::Variation_1()
{ 
	return GetDataCoerce(DBTYPE_UI2, eNORMAL);
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Use BYREF
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_UI2::Variation_2()
{ 
	return GetDataCoerce(DBTYPE_UI2, eBYREF);
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc GetColumns
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_UI2::Variation_3()
{
	return GetDataCoerce(DBTYPE_UI2, eGETCOLUMNS);
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc GetColumns BYREF
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_UI2::Variation_4()
{
	return GetDataCoerce(DBTYPE_UI2, eGETCOLUMNS_BYREF);
}
// }}
// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCDBTYPE_UI2::Terminate()
{ 
	// TO DO:  Add your own code here 

// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCDATALITE::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END


// {{ TCW_TC_PROTOTYPE(TCDBTYPE_UI4)
//*-----------------------------------------------------------------------
//|	Test Case:		TCDBTYPE_UI4 - Test DBTYPE_UI4
//| Created:  	9/27/97
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCDBTYPE_UI4::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCDATALITE::Init())
	// }}
	{ 
		// TO DO:  Add your own code here 
		return TRUE;
	} 
	return FALSE;
} 


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Normal
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_UI4::Variation_1()
{ 
	return GetDataCoerce(DBTYPE_UI4, eNORMAL);
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Use BYREF
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_UI4::Variation_2()
{ 
	return GetDataCoerce(DBTYPE_UI4, eBYREF);
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc GetColumns
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_UI4::Variation_3()
{
	return GetDataCoerce(DBTYPE_UI4, eGETCOLUMNS);
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc GetColumns BYREF
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_UI4::Variation_4()
{
	return GetDataCoerce(DBTYPE_UI4, eGETCOLUMNS_BYREF);
}
// }}
// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCDBTYPE_UI4::Terminate()
{ 
	// TO DO:  Add your own code here 

// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCDATALITE::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END


// {{ TCW_TC_PROTOTYPE(TCDBTYPE_I8)
//*-----------------------------------------------------------------------
//|	Test Case:		TCDBTYPE_I8 - Test DBTYPE_I8
//| Created:  	9/27/97
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCDBTYPE_I8::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCDATALITE::Init())
	// }}
	{ 
		// TO DO:  Add your own code here 
		return TRUE;
	} 
	return FALSE;
} 


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Normal
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_I8::Variation_1()
{ 
	return GetDataCoerce(DBTYPE_I8, eNORMAL);
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Use BYREF
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_I8::Variation_2()
{ 
	return GetDataCoerce(DBTYPE_I8, eBYREF);
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc GetColumns
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_I8::Variation_3()
{
	return GetDataCoerce(DBTYPE_I8, eGETCOLUMNS);
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc GetColumns BYREF
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_I8::Variation_4()
{
	return GetDataCoerce(DBTYPE_I8, eGETCOLUMNS_BYREF);
}
// }}
// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCDBTYPE_I8::Terminate()
{ 
	// TO DO:  Add your own code here 

// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCDATALITE::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END


// {{ TCW_TC_PROTOTYPE(TCDBTYPE_UI8)
//*-----------------------------------------------------------------------
//|	Test Case:		TCDBTYPE_UI8 - Test DBTYPE_UI8
//| Created:  	9/27/97
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCDBTYPE_UI8::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCDATALITE::Init())
	// }}
	{ 
		// TO DO:  Add your own code here 
		return TRUE;
	} 
	return FALSE;
} 


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Normal
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_UI8::Variation_1()
{ 
	return GetDataCoerce(DBTYPE_UI8, eNORMAL);
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Use BYREF
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_UI8::Variation_2()
{ 
	return GetDataCoerce(DBTYPE_UI8, eBYREF);
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc GetColumns
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_UI8::Variation_3()
{
	return GetDataCoerce(DBTYPE_UI8, eGETCOLUMNS);
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc GetColumns BYREF
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_UI8::Variation_4()
{
return GetDataCoerce(DBTYPE_UI8, eGETCOLUMNS_BYREF);
}
// }}
// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCDBTYPE_UI8::Terminate()
{ 
	// TO DO:  Add your own code here 

// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCDATALITE::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END


// {{ TCW_TC_PROTOTYPE(TCDBTYPE_GUID)
//*-----------------------------------------------------------------------
//|	Test Case:		TCDBTYPE_GUID - Test DBTYPE_GUID
//| Created:  	9/27/97
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCDBTYPE_GUID::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCDATALITE::Init())
	// }}
	{ 
		// TO DO:  Add your own code here 
		return TRUE;
	} 
	return FALSE;
} 


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Normal
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_GUID::Variation_1()
{ 
	return GetDataCoerce(DBTYPE_GUID, eNORMAL);
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Use BYREF
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_GUID::Variation_2()
{ 
	return GetDataCoerce(DBTYPE_GUID, eBYREF);
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc GetColumns
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_GUID::Variation_3()
{
	return GetDataCoerce(DBTYPE_GUID, eGETCOLUMNS);
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc GetColumns BYREF
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_GUID::Variation_4()
{
	return GetDataCoerce(DBTYPE_GUID, eGETCOLUMNS_BYREF);
}
// }}
// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCDBTYPE_GUID::Terminate()
{ 
	// TO DO:  Add your own code here 

// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCDATALITE::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END


// {{ TCW_TC_PROTOTYPE(TCDBTYPE_BYTES)
//*-----------------------------------------------------------------------
//|	Test Case:		TCDBTYPE_BYTES - Test DBTYPE_BYTES
//| Created:  	9/27/97
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCDBTYPE_BYTES::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCDATALITE::Init())
	// }}
	{ 
		// TO DO:  Add your own code here 
		return TRUE;
	} 
	return FALSE;
} 


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Normal
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_BYTES::Variation_1()
{ 
	return GetDataCoerce(DBTYPE_BYTES, eNORMAL);
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Use BYREF
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_BYTES::Variation_2()
{ 
	return GetDataCoerce(DBTYPE_BYTES, eBYREF);
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Truncation Case
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_BYTES::Variation_3()
{ 
	return GetDataCoerce(DBTYPE_BYTES, eTRUNCATE);
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc GetColumns
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_BYTES::Variation_4()
{
	return GetDataCoerce(DBTYPE_BYTES, eGETCOLUMNS);
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc GetColumns BYREF
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_BYTES::Variation_5()
{
	return GetDataCoerce(DBTYPE_BYTES, eGETCOLUMNS_BYREF);
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//--------------------------------------------------------------------
// @mfunc GetColumns Truncation
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_BYTES::Variation_6()
{
	return GetDataCoerce(DBTYPE_BYTES, eGETCOLUMNS_TRUNCATE);
}
// }}
// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCDBTYPE_BYTES::Terminate()
{ 
	// TO DO:  Add your own code here 

// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCDATALITE::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END


// {{ TCW_TC_PROTOTYPE(TCDBTYPE_STR)
//*-----------------------------------------------------------------------
//|	Test Case:		TCDBTYPE_STR - Test DBTYPE_STR
//| Created:  	9/27/97
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCDBTYPE_STR::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCDATALITE::Init())
	// }}
	{ 
		// TO DO:  Add your own code here 
		return TRUE;
	} 
	return FALSE;
} 


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Normal
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_STR::Variation_1()
{ 
	return GetDataCoerce(DBTYPE_STR, eNORMAL);
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Use BYREF
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_STR::Variation_2()
{ 
	return GetDataCoerce(DBTYPE_STR, eBYREF);
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Truncation Case
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_STR::Variation_3()
{ 
	return GetDataCoerce(DBTYPE_STR, eTRUNCATE);
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc GetColumns
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_STR::Variation_4()
{
	return GetDataCoerce(DBTYPE_STR, eGETCOLUMNS);
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc GetColumns BYREF
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_STR::Variation_5()
{
	return GetDataCoerce(DBTYPE_STR, eGETCOLUMNS_BYREF);
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc GetColumns BYREF Truncation
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_STR::Variation_6()
{
	return GetDataCoerce(DBTYPE_STR, eGETCOLUMNS_TRUNCATE);
}
// }}
// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCDBTYPE_STR::Terminate()
{ 
	// TO DO:  Add your own code here 

// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCDATALITE::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END


// {{ TCW_TC_PROTOTYPE(TCDBTYPE_WSTR)
//*-----------------------------------------------------------------------
//|	Test Case:		TCDBTYPE_WSTR - Test DBTYPE_WSTR
//| Created:  	9/27/97
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCDBTYPE_WSTR::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCDATALITE::Init())
	// }}
	{ 
		// TO DO:  Add your own code here 
		return TRUE;
	} 
	return FALSE;
} 


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Normal
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_WSTR::Variation_1()
{ 
	return GetDataCoerce(DBTYPE_WSTR, eNORMAL);
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Use BYREF
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_WSTR::Variation_2()
{ 
	return GetDataCoerce(DBTYPE_WSTR, eBYREF);
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Truncation Case
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_WSTR::Variation_3()
{ 
	return GetDataCoerce(DBTYPE_WSTR, eTRUNCATE);
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc GetColumns
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_WSTR::Variation_4()
{
	return GetDataCoerce(DBTYPE_WSTR, eGETCOLUMNS);
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc GetColumns BYREF
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_WSTR::Variation_5()
{
	return GetDataCoerce(DBTYPE_WSTR, eGETCOLUMNS_BYREF);
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc GetColumns BYREF Truncation
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_WSTR::Variation_6()
{
	return GetDataCoerce(DBTYPE_WSTR, eGETCOLUMNS_TRUNCATE);
}
// }}
// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCDBTYPE_WSTR::Terminate()
{ 
	// TO DO:  Add your own code here 

// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCDATALITE::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END


// {{ TCW_TC_PROTOTYPE(TCDBTYPE_NUMERIC)
//*-----------------------------------------------------------------------
//|	Test Case:		TCDBTYPE_NUMERIC - Test DBTYPE_NUMERIC
//| Created:  	9/27/97
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCDBTYPE_NUMERIC::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCDATALITE::Init())
	// }}
	{ 
		// TO DO:  Add your own code here 
		return TRUE;
	} 
	return FALSE;
} 


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Normal
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_NUMERIC::Variation_1()
{ 
	return GetDataCoerce(DBTYPE_NUMERIC, eNORMAL);
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Use BYREF
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_NUMERIC::Variation_2()
{ 
	return GetDataCoerce(DBTYPE_NUMERIC, eBYREF);
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc GetColumns
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_NUMERIC::Variation_3()
{
	return GetDataCoerce(DBTYPE_NUMERIC, eGETCOLUMNS);
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc GetColumns BYREF
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_NUMERIC::Variation_4()
{
	return GetDataCoerce(DBTYPE_NUMERIC, eGETCOLUMNS_BYREF);
}
// }}
// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCDBTYPE_NUMERIC::Terminate()
{ 
	// TO DO:  Add your own code here 

// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCDATALITE::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END


// {{ TCW_TC_PROTOTYPE(TCDBTYPE_DBDATE)
//*-----------------------------------------------------------------------
//|	Test Case:		TCDBTYPE_DBDATE - Test DBTYPE_DBDATE
//| Created:  	9/27/97
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCDBTYPE_DBDATE::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCDATALITE::Init())
	// }}
	{ 
		// TO DO:  Add your own code here 
		return TRUE;
	} 
	return FALSE;
} 


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Normal
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_DBDATE::Variation_1()
{ 
	return GetDataCoerce(DBTYPE_DBDATE, eNORMAL);
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Use BYREF
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_DBDATE::Variation_2()
{ 
	return GetDataCoerce(DBTYPE_DBDATE, eBYREF);
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc GetColumns
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_DBDATE::Variation_3()
{
	return GetDataCoerce(DBTYPE_DBDATE, eGETCOLUMNS);
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc GetColumns BYREF
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_DBDATE::Variation_4()
{
	return GetDataCoerce(DBTYPE_DBDATE, eGETCOLUMNS_BYREF);
}
// }}
// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCDBTYPE_DBDATE::Terminate()
{ 
	// TO DO:  Add your own code here 

// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCDATALITE::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END


// {{ TCW_TC_PROTOTYPE(TCDBTYPE_DBTIME)
//*-----------------------------------------------------------------------
//|	Test Case:		TCDBTYPE_DBTIME - Test DBTYPE_DBTIME
//| Created:  	9/27/97
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCDBTYPE_DBTIME::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCDATALITE::Init())
	// }}
	{ 
		// TO DO:  Add your own code here 
		return TRUE;
	} 
	return FALSE;
} 


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Normal
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_DBTIME::Variation_1()
{ 
	return GetDataCoerce(DBTYPE_DBTIME, eNORMAL);
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Use BYREF
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_DBTIME::Variation_2()
{ 
	return GetDataCoerce(DBTYPE_DBTIME, eBYREF);
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc GetColumns
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_DBTIME::Variation_3()
{
	return GetDataCoerce(DBTYPE_DBTIME, eGETCOLUMNS);
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc GetColumns BYREF
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_DBTIME::Variation_4()
{
	return GetDataCoerce(DBTYPE_DBTIME, eGETCOLUMNS_BYREF);
}
// }}
// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCDBTYPE_DBTIME::Terminate()
{ 
	// TO DO:  Add your own code here 

// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCDATALITE::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END


// {{ TCW_TC_PROTOTYPE(TCDBTYPE_DBTIMESTAMP)
//*-----------------------------------------------------------------------
//|	Test Case:		TCDBTYPE_DBTIMESTAMP - Test DBTYPE_DBTIMESTAMP
//| Created:  	9/27/97
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCDBTYPE_DBTIMESTAMP::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCDATALITE::Init())
	// }}
	{ 
		// TO DO:  Add your own code here 
		return TRUE;
	} 
	return FALSE;
} 


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Normal
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_DBTIMESTAMP::Variation_1()
{ 
	return GetDataCoerce(DBTYPE_DBTIMESTAMP, eNORMAL);
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Use BYREF
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_DBTIMESTAMP::Variation_2()
{ 
	return GetDataCoerce(DBTYPE_DBTIMESTAMP, eBYREF);
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc GetColumns
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_DBTIMESTAMP::Variation_3()
{
	return GetDataCoerce(DBTYPE_DBTIMESTAMP, eGETCOLUMNS);
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc GetColumns BYREF
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_DBTIMESTAMP::Variation_4()
{
	return GetDataCoerce(DBTYPE_DBTIMESTAMP, eGETCOLUMNS_BYREF);
}
// }}
// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCDBTYPE_DBTIMESTAMP::Terminate()
{ 
	// TO DO:  Add your own code here 

// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCDATALITE::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END


// {{ TCW_TC_PROTOTYPE(TCIConvetType)
//*-----------------------------------------------------------------------
//|	Test Case:		TCIConvetType - Display supported conversion
//|	Created:			10/01/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIConvetType::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCDATALITE::Init())
	// }}
	{
		// TO DO:  Add your own code here
		return TRUE;
	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Display all conversion
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIConvetType::Variation_1()
{
	OpenRowset(IID_IRowset);
	FindValidCoercions();
	ReleaseRowset();

	return TEST_PASS;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIConvetType::Terminate()
{
	// TO DO:  Add your own code here

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCDATALITE::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCDBTYPE_VARNUMERIC)
//*-----------------------------------------------------------------------
//|	Test Case:		TCDBTYPE_VARNUMERIC - Test VARNUMERIC conversions
//|	Created:			03/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCDBTYPE_VARNUMERIC::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCDATALITE::Init())
	// }}
	{
		// TO DO:  Add your own code here
		return TRUE;
	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Normal
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_VARNUMERIC::Variation_1()
{
	return GetDataCoerce(DBTYPE_VARNUMERIC, eNORMAL);
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Truncation
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_VARNUMERIC::Variation_2()
{
	return GetDataCoerce(DBTYPE_VARNUMERIC, eTRUNCATE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc GetColumns
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_VARNUMERIC::Variation_3()
{
	return GetDataCoerce(DBTYPE_VARNUMERIC, eGETCOLUMNS);
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc GetColumns BYREF
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_VARNUMERIC::Variation_4()
{
	return GetDataCoerce(DBTYPE_VARNUMERIC, eGETCOLUMNS_BYREF);
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//--------------------------------------------------------------------
// @mfunc GetColumns Truncation
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_VARNUMERIC::Variation_5()
{
	return GetDataCoerce(DBTYPE_VARNUMERIC, eGETCOLUMNS_TRUNCATE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//--------------------------------------------------------------------
// @mfunc Normal BYREF
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDBTYPE_VARNUMERIC::Variation_6()
{
	return GetDataCoerce(DBTYPE_VARNUMERIC, eBYREF);
}
// }}
// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCDBTYPE_VARNUMERIC::Terminate()
{
	// TO DO:  Add your own code here

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCDATALITE::Terminate());
}	// }}
// }}
// }}
