//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1995-2000 Microsoft Corporation.
//
// @doc 
//
// @module IROWFIND.CPP | Template source file for all test modules.
//

#include "modstandard.hpp"
#define  DBINITCONSTANTS	// Must be defined to initialize constants in OLEDB.H
#define  INITGUID
#include "irowfind.h"
#include <float.h>
#include <msdaguid.h>		// CLSID_OLEDB_CONVERSIONLIBRARY

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Module Values
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// {{ TCW_MODULE_GLOBALS
DECLARE_MODULE_CLSID = { 0x9eb41234, 0x15cc, 0x11d1, { 0xa8, 0x7f, 0x00, 0xc0, 0x4f, 0xd7, 0xa0, 0xf5 }};
DECLARE_MODULE_NAME("IRowsetFind");
DECLARE_MODULE_OWNER("Microsoft");
DECLARE_MODULE_DESCRIP("Tests the IRowsetFind interface");
DECLARE_MODULE_VERSION(795921705);
// TCW_WizardVersion(2)
// TCW_Automation(True)
// }} TCW_MODULE_GLOBALS_END

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Globals
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
CTable *			g_pCTable=NULL;				//Global table
CTable *			g_pEmptyTable = NULL;		//Empty table
CTable *			g_p1RowTable = NULL;		//One row table
IDBCreateSession *	g_pIDBCreateSession = NULL;	//pointer to IDBCreateSession interface
IDBCreateCommand *	g_pIDBCreateCommand = NULL;	//pointer to IDBCreateComand interface

DBORDINAL			g_ulColNum;					// Column number to test FindValue match on.
DBTYPE				g_wColType;					// DBTYPE of that column
DBROWCOUNT			g_lRowLast;					// the index of last row in g_pCTable.
LONG_PTR			g_lNullCollation;			// contains the value of the NULL_COLLATION property

ULONG g_ulDCVer = 0x200;
//	Interface for IDataConvert, gotten by CoCreateInstance
IDataConvert *	g_pIDataConvert=NULL;


enum ePrptIdx	
{
	IDX_Bookmarks=0, 
	IDX_OrderedBookmarks, 
	IDX_LiteralBookmarks,
	IDX_FetchBackwards, 
	IDX_ScrollBackwards, 
	IDX_CanHoldRows,
	IDX_RemoveDeleted, 
	IDX_BookmarkSkipped,
	IDX_OtherUpdateDelete,	
	IDX_OtherInsert, 
	IDX_BookmarkType,
	IDX_IRowsetDeleteBookmarks, 
	IDX_IRowsetChange,
	IDX_IRowsetLocate
};

enum eSUBCOMPAREOP
{
	SUBOP_EMPTY,
	SUBOP_ALWAYS_EQ,
	SUBOP_CONTAINS_BEGIN, 
	SUBOP_CONTAINS_MIDDLE, 
	SUBOP_CONTAINS_END,
	SUBOP_CASESENSITIVE,
	SUBOP_CASEINSENSITIVE,
	SUBOP_ALWAYS_NULL
};
		
#define PROPERTY_COUNT	IDX_IRowsetLocate+1

//record the properties default values
struct	DBPrptRecord
{
	BOOL	fSupported;
	BOOL	fDefault;
} g_rgDBPrpt[PROPERTY_COUNT];

struct FindValueInfo
{
	DBSTATUS		dbsStatus;
	DBLENGTH		cbLength;
	void			*pValue;
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Defines
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#define RESTART					TRUE
#define USE_STREAM				TRUE
#define USE_DBTYPE_EMPTY		TRUE

#define TESTC_DRIVER(exp)		{ if((exp)==FALSE) { odtLog << L"NotSupported by Driver, skipping Variation\n"; fTestPass=TEST_SKIPPED; goto CLEANUP;			} }

inline ULONG MYABS(short s)
{
	return ( s >= 0 ? s : -s );
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// prototypes
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static BOOL IsDBTYPEFindable(DBTYPE wType);
static void FindVariantTypes(IRowset *pIRowset, CTable *pTable);

// Return TRUE for only DBTYPEs that match an OLE VARIANT TYPE.
static BOOL IsOleVarEnumType( DBTYPE wdbType )
{
	switch ( wdbType )
	{
	case DBTYPE_STR:
	case DBTYPE_WSTR:
	case DBTYPE_BYTES:
	case DBTYPE_NUMERIC:
	case DBTYPE_DBTIME:
	case DBTYPE_DBDATE:
	case DBTYPE_DBTIMESTAMP:
	case DBTYPE_GUID:
	case DBTYPE_VARNUMERIC:
	case DBTYPE_FILETIME:
	case DBTYPE_PROPVARIANT:
		return FALSE;
	default:
		return TRUE;
	}
};

// Used for creating tables where test variations have different results for different compare operators, 
// on different data types, on different providers, etc.
struct VariationResults
{
	WCHAR    szProviderName[256];
	HRESULT	 expHR1;
	HRESULT  expHR2;
	int      expRows;
	BOOL	 bVerifyData;
	BOOL	 bSQLOLEDB;
	BOOL     bMSDASQL;
	ULONG    minCompareOp;
	ULONG    maxCompareOp;
};

static DBLENGTH GetDataLength(void *pMakeData, DBTYPE wColType, DBLENGTH cbBytesLen)
{
	DBLENGTH cbLength;

	switch ( wColType )
	{
	case DBTYPE_WSTR:
		cbLength = (wcslen((WCHAR *)pMakeData)+1)*sizeof(WCHAR);
		break;
	case DBTYPE_STR:
		cbLength = strlen((char *)pMakeData)+sizeof(char);
		break;
	case DBTYPE_BYTES:
	case DBTYPE_VARNUMERIC:
	case DBTYPE_UDT:
		cbLength = cbBytesLen;
		break;
	default:
		cbLength = GetDBTypeSize(wColType);
		break;
	}

	return cbLength;
}

static BOOL ValidateCompareOp ( DWORD dwFindCompareOps, DBCOMPAREOP CompareOp )
{
	if ((CompareOp & DBCOMPAREOPS_CASESENSITIVE) &&
		!(dwFindCompareOps & DBPROPVAL_CO_CASESENSITIVE))
		return FALSE;

	if ((CompareOp & DBCOMPAREOPS_CASEINSENSITIVE) &&
		!(dwFindCompareOps & DBPROPVAL_CO_CASEINSENSITIVE))
		return FALSE;

	switch ( CompareOp & ~(DBCOMPAREOPS_CASESENSITIVE | DBCOMPAREOPS_CASEINSENSITIVE) )
	{
	case DBCOMPAREOPS_EQ:
	case DBCOMPAREOPS_NE:
	case DBCOMPAREOPS_LE:
	case DBCOMPAREOPS_LT:
	case DBCOMPAREOPS_GE:
	case DBCOMPAREOPS_GT:			
			return ( dwFindCompareOps & DBPROPVAL_CO_EQUALITY );
	case DBCOMPAREOPS_BEGINSWITH:	
			return ( dwFindCompareOps & DBPROPVAL_CO_STRING || dwFindCompareOps & DBPROPVAL_CO_BEGINSWITH );
	case DBCOMPAREOPS_IGNORE:
		return TRUE;
	case DBCOMPAREOPS_NOTBEGINSWITH:
			return ( dwFindCompareOps & DBPROPVAL_CO_BEGINSWITH );
	case DBCOMPAREOPS_NOTCONTAINS:
	case DBCOMPAREOPS_CONTAINS:
			return (dwFindCompareOps & DBPROPVAL_CO_CONTAINS);
	default:
		return FALSE;
	}
}

static DWORD GetFindCompareOps(IUnknown *pIUnknown, DBID *pColDBID)
{
	IRowsetInfo *	pIRowsetInfo = NULL;
	DBPROPIDSET		DBPropIDSet;
	ULONG			cPropertySets;
	ULONG			cProperties;
	ULONG			i;
	DBPROPSET *		prgProperties = NULL;
	DWORD			iFindOps = 0;
	
	if(!VerifyInterface(pIUnknown, IID_IRowsetInfo, ROWSET_INTERFACE, (IUnknown**)&pIRowsetInfo))
		return 0;

	DBPropIDSet.rgPropertyIDs = NULL;
	DBPropIDSet.cPropertyIDs = 1;
	DBPropIDSet.guidPropertySet = DBPROPSET_ROWSET;

	DBPropIDSet.rgPropertyIDs=(DBPROPID *)PROVIDER_ALLOC(sizeof(DBPROPID));
	DBPropIDSet.rgPropertyIDs[0]= DBPROP_FINDCOMPAREOPS;

	//get property	
	TESTC(SUCCEEDED(pIRowsetInfo->GetProperties(1,&DBPropIDSet, &cPropertySets, &prgProperties)));
	TESTC(cPropertySets == 1);

	cProperties = prgProperties[0].cProperties;
	
	for ( i = 0; i < cProperties; i++ )
	{
		if ( CompareDBID(prgProperties[0].rgProperties[i].colid, DB_NULLID) )
		{
			iFindOps = V_I4(&prgProperties[0].rgProperties[i].vValue);
			break;
		}
	
		if ( CompareDBID(*pColDBID, prgProperties[0].rgProperties[i].colid) )
			iFindOps = V_I4(&prgProperties[0].rgProperties[i].vValue);
	}
	
CLEANUP:
	if(prgProperties)
	{
		if(prgProperties->rgProperties)
			PROVIDER_FREE(prgProperties->rgProperties);
		PROVIDER_FREE(prgProperties);
	}

	if(DBPropIDSet.rgPropertyIDs)
		PROVIDER_FREE(DBPropIDSet.rgPropertyIDs);

	SAFE_RELEASE(pIRowsetInfo);
	return iFindOps;
}

// Some DBTYPE we can't use with this test because
// we require the values in the column to be unique.
static BOOL IsDBTYPEFindable(DBTYPE wType)
{
	switch ( wType )
	{
	case DBTYPE_EMPTY:
	case DBTYPE_NULL:
	case DBTYPE_BOOL:
	case DBTYPE_IDISPATCH:
	case DBTYPE_IUNKNOWN:
		return FALSE;
	default:
		return TRUE;
	}
}

static BOOL AddShortToNumeric(DB_NUMERIC * pNumeric, short sNum)
{
	ASSERT(pNumeric);
	
	// Check parameters
	if (!pNumeric)
		return FALSE;

	if (pNumeric->sign == 0)
	{
		sNum = -sNum;  // if the numeric is negative, negate sNum
	}

	// If operation won't overflow
	if (pNumeric->precision <= sizeof(DWORDLONG) - 1)
	{
		BYTE PrevSign = pNumeric->sign;
		// check for sign change.
		if (*(UNALIGNED DWORDLONG *)pNumeric->val < sNum)
		{
			if ( (sNum > 0 && PrevSign == 0) ||
				 (sNum < 0 && PrevSign == 1) )
			{
				pNumeric->sign = ( pNumeric->sign == 1 ? 0 : 1 );  // flip-flop the sign
				*(UNALIGNED DWORDLONG *)pNumeric->val = MYABS(sNum) - *(UNALIGNED DWORDLONG *)pNumeric->val ;
			}
		}
		else
			*(UNALIGNED DWORDLONG *)pNumeric->val += sNum;
	}
	else
	{
		DWORDLONG dwlAccum = sNum;

		// Assume 64 bit math.
		for(ULONG ul = 0; ul < sizeof(pNumeric->val) / sizeof(ULONG); ul++ )
		{
			dwlAccum +=(DWORDLONG)(*(((UNALIGNED ULONG *)pNumeric->val) + ul));
			*(((UNALIGNED ULONG *)pNumeric->val) + ul) = (ULONG)dwlAccum;
			dwlAccum >>= sizeof(ULONG) * 8;
		}
	}

	//	Adjust length if overflow into next byte
	if (pNumeric->precision < sizeof(pNumeric->val) &&
		*(pNumeric->val + pNumeric->precision) != 0)
		pNumeric->precision++;

	return TRUE;
}

static BOOL AddShortToDecimal(DECIMAL * pDecimal, short sNum)
{
	ASSERT(pDecimal);
	
	// Check parameters
	if (!pDecimal)
		return FALSE;

	// If operation won't overflow
	if (pDecimal->Hi32 == 0 && pDecimal->Mid32 == 0)
	{
		// check for sign change.

		if ( (pDecimal->sign == 0 && sNum >= 0) || (pDecimal->sign == 0x080 && sNum <= 0) )
		{
			pDecimal->Lo32 += MYABS(sNum);
		}
		else if (pDecimal->Lo32 < MYABS(sNum))
		{	// possible sign change
			if ( (sNum > 0 && pDecimal->sign == 0x80) ||
				 (sNum < 0 && pDecimal->sign == 0) )
			{
				pDecimal->sign = ( pDecimal->sign = 0 ? 0x80 : 0 );  // flip-flop the sign
				pDecimal->Lo32 = MYABS(sNum) - pDecimal->Lo32;
			}
			else
				pDecimal->Lo32 += MYABS(sNum);
		} 
		else if ( (pDecimal->sign == 0 && sNum < 0) || (pDecimal->sign == 0x80 && sNum > 0) )
		{
			pDecimal->Lo32 -= MYABS(sNum);
		}
	}
	else
	{
		DWORDLONG dwlAccum = ( pDecimal->sign == 0x80 ? -sNum : sNum );

		dwlAccum += pDecimal->Lo32;
		pDecimal->Lo32 = (ULONG)dwlAccum;

		dwlAccum >>= sizeof(ULONG) * 8;

		dwlAccum += pDecimal->Mid32;
		pDecimal->Mid32 = (ULONG)dwlAccum;

		dwlAccum >>= sizeof(ULONG) * 8;

		dwlAccum += pDecimal->Hi32;
		pDecimal->Hi32 = (ULONG)dwlAccum;
	}
	return TRUE;
}

void FindNullCollationProperty(IDBCreateSession *pIDBCreateSession)
{
	HRESULT				hr;
	ULONG				cProperties;
	DBPROPSET *			prgProperties=NULL;
	DBPROPIDSET			DBPropIDSet;
	DBPROPID			PropID = DBPROP_NULLCOLLATION;
	IDBProperties *		pIDBProperties = NULL;

	//Initialize
	DBPropIDSet.rgPropertyIDs = NULL;
	DBPropIDSet.cPropertyIDs = 1;
	DBPropIDSet.guidPropertySet = DBPROPSET_DATASOURCE;

	DBPropIDSet.rgPropertyIDs=&PropID;

	//get properties
	if(!VerifyInterface(pIDBCreateSession, IID_IDBProperties, DATASOURCE_INTERFACE, (IUnknown**)&pIDBProperties))
		return;

	hr = pIDBProperties->GetProperties(1,&DBPropIDSet,&cProperties,&prgProperties);

	if(prgProperties[0].rgProperties[cProperties-1].dwStatus==DBPROPSTATUS_NOTSUPPORTED)
	{
		// check for conformance violation
		//ASSERT(FALSE==IsReqProperty(DBPROPSET_DATASOURCE, DBPROP_NULLCOLLATION));
		g_lNullCollation = 0;
	}
	else
	{	
		g_lNullCollation = V_I4(&prgProperties[0].rgProperties[0].vValue);
	} 

	FreeProperties(&cProperties,&prgProperties);
	SAFE_RELEASE(pIDBProperties);
}


static void FindVariantTypes(IUnknown *pIUnknown, CTable *pTable)
{
	IAccessor *	pIAccessor;
	IRowset *	pIRowset;
	HACCESSOR	hAccessor;
	DBPART		dwPart=DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH;
	DBCOUNTITEM	cBinding = 0;
	DBCOUNTITEM cCount = 0;
	DBCOUNTITEM i = 0;
	DBLENGTH	cRowSize = 0;
	DBBINDING *	rgBinding = NULL;
	HROW *		pHRow = NULL;
	BYTE *		pData = NULL;
	VARIANT *	pVar = NULL;
	
	TESTC(VerifyInterface(pIUnknown, IID_IRowset, ROWSET_INTERFACE, (IUnknown**)&pIRowset));
	TESTC(VerifyInterface(pIRowset, IID_IAccessor, ROWSET_INTERFACE, (IUnknown**)&pIAccessor));

	//create an accessor on the rowset
	TESTC_(GetAccessorAndBindings(pIRowset,DBACCESSOR_ROWDATA,&hAccessor,
		&rgBinding,&cBinding,&cRowSize,dwPart,ALL_COLS_BOUND,FORWARD,
		NO_COLS_BY_REF,NULL,NULL,NULL,DBTYPE_EMPTY,0,NULL,NULL,
		NO_COLS_OWNED_BY_PROV,DBPARAMIO_NOTPARAM,TRUE), S_OK);

	pData = (BYTE *)PROVIDER_ALLOC(cRowSize);
	TESTC(pData != NULL);

	//get the data
	TESTC_(pIRowset->GetNextRows(NULL,0,1,&cCount,&pHRow), S_OK);
	TESTC_(pIRowset->GetData(*pHRow, hAccessor, pData), S_OK);

	for ( i = 0; i<cBinding; i++)
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

	if(pIRowset)
		pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL);

	if(pIAccessor)
		pIAccessor->ReleaseAccessor(hAccessor,NULL);
	
	PROVIDER_FREE(rgBinding);
	PROVIDER_FREE(pData);
	PROVIDER_FREE(pHRow);

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
	HRESULT				hr;
	IUnknown *			pIRowset=NULL;
	IRowsetInfo	*		pIRowsetInfo=NULL;
	DBCOLUMNINFO *		rgInfo=NULL;
	ULONG				cProperties;
	DBPROPSET *			prgProperties=NULL;
	DBPROPIDSET			DBPropIDSet;
	ULONG				cPropertyCount=PROPERTY_COUNT;
	CCol				TempCol;
	SYSTEMTIME			SystemTime;
	BOOL				fFound = FALSE, fInit = FALSE;
	DBORDINAL			ulStart = 0;
	DATE				dOleDate = 0;
	WCHAR *				pwszSeedValue =NULL;

	if(!ModuleCreateDBSession(pThisTestModule))
		return FALSE;

	// When used with an .ini file, this test performs destructive variations that alter the data
	// of non-updateable columns.  Hence the test notifies PrivLib that read only column data
	// should not be validated
	if (GetModInfo())
		GetModInfo()->SetCompReadOnlyCols(FALSE);

	//store IDBCreateSession and IDBCreateCommand pointer
	//IDBCreateSession
	if(!VerifyInterface(pThisTestModule->m_pIUnknown, IID_IDBCreateSession, DATASOURCE_INTERFACE, (IUnknown**)&g_pIDBCreateSession))
		return FALSE;

	//IDBCreateCommand
	if(!VerifyInterface(pThisTestModule->m_pIUnknown2, IID_IDBCreateCommand, SESSION_INTERFACE, (IUnknown**)&g_pIDBCreateCommand))
	{
		// Note the limitation and continue.
		odtLog << L"IDBCreateCommand is not supported by Provider." << ENDL;
	}

	// Find NULL COLLATION property
	FindNullCollationProperty(g_pIDBCreateSession);

	//create the table
	g_pCTable = new CTable(pThisTestModule->m_pIUnknown2, (WCHAR *)g_wszModuleName, USENULLS);
	g_pEmptyTable = new CTable(pThisTestModule->m_pIUnknown2, (WCHAR *)g_wszModuleName, USENULLS);
	g_p1RowTable = new CTable(pThisTestModule->m_pIUnknown2, (WCHAR *)g_wszModuleName, USENULLS);

	if(!g_pCTable || !SUCCEEDED(g_pCTable->CreateTable(20,1,NULL,PRIMARY,TRUE))
		)
	{
		odtLog<<L"Create Table failed, test cannot proceed\n";
		return FALSE;
	}

	g_lRowLast = g_pCTable->GetRowsOnCTable();

	if ( g_lRowLast < 8 )
	{
		odtLog<<L"Need at least an 8 row table for this test!\n";
		return FALSE;
	}

	if(!g_pEmptyTable || !SUCCEEDED(g_pEmptyTable->CreateTable(0,1,NULL,PRIMARY,TRUE)))
	{
		odtLog<<L"Create Table failed, test cannot proceed\n";
		return FALSE;
	}

	if ( g_pEmptyTable->GetRowsOnCTable() != 0 )
	{
		delete g_pEmptyTable;
		g_pEmptyTable = NULL;
	}

	if(!g_p1RowTable || !SUCCEEDED(g_p1RowTable->CreateTable(1,1,NULL,PRIMARY,TRUE)))
	{
		odtLog<<L"Create Table failed, test cannot proceed\n";
		return FALSE;
	}

	if ( g_p1RowTable->GetRowsOnCTable() != 1 )
	{
		delete g_p1RowTable;
		g_p1RowTable = NULL;
	}

	//make sure IRowsetFind interface is supported by opening a rowset
	//to open a rowset
	hr = g_pCTable->CreateRowset( USE_OPENROWSET, IID_IRowsetFind,	0,	NULL, &pIRowset,				
							NULL, NULL);

	//if E_NOINTERFACE is returned, IRowsetFind is not supported by the provider
	if(hr==ResultFromScode(E_NOINTERFACE))
	{
		odtLog<<L"IRowsetFind is not supported by the provider!\n";
		return TEST_SKIPPED;
	}

	if(hr!=ResultFromScode(S_OK))
	{
		odtLog<<L"CTable::ExcuteCommand failed!\n";
		return FALSE;
	}
	
	FindVariantTypes(pIRowset, g_pCTable);

	if(GetModInfo()->GetInitStringValue(L"FINDSEED", &pwszSeedValue))
	{
		g_ulColNum = _wtoi(pwszSeedValue);
		odtLog << "Trying to use specified seed: " << g_ulColNum << ENDL;
	}
	else
	{
		// Determine a Column seed to use for the other finds.	
		GetSystemTime(&SystemTime);
		SystemTimeToVariantTime(&SystemTime, &dOleDate);
		
		ulStart = ( ULONG(dOleDate) % g_pCTable->CountColumnsOnTable() ) + 1;
		g_ulColNum = ulStart;
		odtLog << "Using seed : " << g_ulColNum << ENDL;
	}

	do
	{			
		g_pCTable->GetColInfo(g_ulColNum, TempCol);
				
		// choose a column seed to verify correct cursor behavior
		// Vary the column seed but avoid small type (I1/UI1) and 
		// avoid complicated types such as BLOBS and VARNUMERIC
		if (ValidateCompareOp ( GetFindCompareOps(pIRowset, TempCol.GetColID()), DBCOMPAREOPS_EQ ) &&
			TempCol.GetUpdateable() &&
			!TempCol.GetIsLong() &&
			TempCol.GetProviderType() != DBTYPE_UI1 &&
			TempCol.GetProviderType() != DBTYPE_I1 &&
			TempCol.GetProviderType() != DBTYPE_BOOL &&
			TempCol.GetProviderType() != DBTYPE_BYTES)
		{		
			if ( TempCol.GetProviderType() == DBTYPE_VARIANT )
			{
				if ( IsDBTYPEFindable(TempCol.GetSubType()) )
				{
					g_wColType = DBTYPE_VARIANT;
					fFound = TRUE;
				}
			}
			else if ( IsDBTYPEFindable(TempCol.GetProviderType()) )
			{
				g_wColType = TempCol.GetProviderType();
				fFound = TRUE;
			}
		}
		
		if ( !fFound )
		{
			if ( ++g_ulColNum > g_pCTable->CountColumnsOnTable() )
				g_ulColNum = 1;
		}
			
	} while ( !fFound && ulStart != g_ulColNum );

	if ( fFound )
		odtLog << "Column to find value on is " << g_ulColNum << " of type " << g_wColType << ".\n";
	else
	{
		odtLog << "No searchable columns found on target table.  No testing will proceed.\n";
		goto CLEANUP;
	}


	// IRowsetFind is supported, now retrieve information about all the properties we
	// will use....
	//if properites are supported
	//init all the properties
	//check if properites are supported

	//Initialize
	DBPropIDSet.rgPropertyIDs = NULL;
	DBPropIDSet.cPropertyIDs = PROPERTY_COUNT;
	DBPropIDSet.guidPropertySet = DBPROPSET_ROWSET;

	cPropertyCount=IDX_IRowsetLocate+1;
	DBPropIDSet.rgPropertyIDs=(DBPROPID *)PROVIDER_ALLOC(cPropertyCount *
		sizeof(DBPROPID));

	DBPropIDSet.rgPropertyIDs[IDX_Bookmarks] = DBPROP_BOOKMARKS;
	DBPropIDSet.rgPropertyIDs[IDX_OrderedBookmarks] = DBPROP_ORDEREDBOOKMARKS;
	DBPropIDSet.rgPropertyIDs[IDX_LiteralBookmarks] = DBPROP_LITERALBOOKMARKS;
	DBPropIDSet.rgPropertyIDs[IDX_FetchBackwards] = DBPROP_CANFETCHBACKWARDS;
	DBPropIDSet.rgPropertyIDs[IDX_ScrollBackwards] = DBPROP_CANSCROLLBACKWARDS;
	DBPropIDSet.rgPropertyIDs[IDX_CanHoldRows] = DBPROP_CANHOLDROWS;
	DBPropIDSet.rgPropertyIDs[IDX_RemoveDeleted] = DBPROP_REMOVEDELETED;
	DBPropIDSet.rgPropertyIDs[IDX_BookmarkSkipped] = DBPROP_BOOKMARKSKIPPED;
	DBPropIDSet.rgPropertyIDs[IDX_OtherUpdateDelete] = DBPROP_OTHERUPDATEDELETE;
	DBPropIDSet.rgPropertyIDs[IDX_OtherInsert] = DBPROP_OTHERINSERT;
	DBPropIDSet.rgPropertyIDs[IDX_IRowsetDeleteBookmarks] = DBPROP_IRowsetChange;
	DBPropIDSet.rgPropertyIDs[IDX_BookmarkType] = DBPROP_BOOKMARKTYPE;
	DBPropIDSet.rgPropertyIDs[IDX_IRowsetChange] = DBPROP_IRowsetChange;
	DBPropIDSet.rgPropertyIDs[IDX_IRowsetLocate] = DBPROP_IRowsetLocate;

	//Get Rowset Info	
	if(!VerifyInterface(pIRowset, IID_IRowsetInfo, ROWSET_INTERFACE, (IUnknown**)&pIRowsetInfo))
		goto CLEANUP;

	//mark everything as supported
	for(cProperties=0;cProperties<PROPERTY_COUNT;cProperties++)
	{
			g_rgDBPrpt[cProperties].fSupported=TRUE;
			g_rgDBPrpt[cProperties].fDefault=FALSE;
	}

	//get properties	
	if(!SUCCEEDED(hr=pIRowsetInfo->GetProperties(1,&DBPropIDSet, &cProperties, &prgProperties)))
		goto CLEANUP;

	//mark the properties
	for(cProperties=0;cProperties<PROPERTY_COUNT;cProperties++)
	{
		//mark the not supported properties
		if(prgProperties[0].rgProperties[cProperties].dwStatus==DBPROPSTATUS_NOTSUPPORTED)
		{
			// check for conformance violation
			//ASSERT(FALSE==IsReqProperty(DBPropIDSet.rgPropertyIDs[cProperties], DBPropIDSet.guidPropertySet));

			g_rgDBPrpt[cProperties].fSupported=FALSE;
			g_rgDBPrpt[cProperties].fDefault=FALSE;
		}
		else
		{	
			if(prgProperties[0].rgProperties[cProperties].dwStatus!=DBPROPSTATUS_OK)
				odtLog<<L"Error: default value failed for properties indexed at "<<cProperties<<L".\n";

			//mark as supported properties
			g_rgDBPrpt[cProperties].fSupported=TRUE;

			if(cProperties==IDX_BookmarkType)
			{
			   if(prgProperties[0].rgProperties[cProperties].vValue.lVal
				   !=DBPROPVAL_BMK_NUMERIC)
			   {
				   odtLog<<L"ERROR: The bookmark is not based on numeric!\n";

				   if(prgProperties[0].rgProperties[cProperties].vValue.lVal
					!=DBPROPVAL_BMK_KEY)
				   odtLog<<L"ERROR: The bookmark type returned false information!\n";
			   }

			}
			else
			{
				g_rgDBPrpt[cProperties].fDefault=
				V_BOOL(&prgProperties[0].rgProperties[cProperties].vValue);
			}

		}
	} 

	fInit = TRUE;

CLEANUP:
	//release rowset objects
	SAFE_RELEASE(pIRowsetInfo);
	SAFE_RELEASE(pIRowset);

	PROVIDER_FREE(pwszSeedValue);
	
	//free the memory
	if(prgProperties)
	{
		if(prgProperties->rgProperties)
			PROVIDER_FREE(prgProperties->rgProperties);
		PROVIDER_FREE(prgProperties);
	}

	if(DBPropIDSet.rgPropertyIDs)
		PROVIDER_FREE(DBPropIDSet.rgPropertyIDs);

	return fInit;
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
	//drop tables
	if(g_pCTable)
	{
		g_pCTable->DropTable();
		delete g_pCTable;
		g_pCTable = NULL;
	}

	if(g_p1RowTable)
	{
		g_p1RowTable->DropTable();
		delete g_p1RowTable;
		g_p1RowTable = NULL;
	}

	if(g_pEmptyTable)
	{
		g_pEmptyTable->DropTable();
		delete g_pEmptyTable;
		g_pEmptyTable = NULL;
	}

	SAFE_RELEASE(g_pIDBCreateSession);
	SAFE_RELEASE(g_pIDBCreateCommand);
	SAFE_RELEASE(g_pIDataConvert);

	return (ModuleReleaseDBSession(pThisTestModule));
}	


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Base Test Case Section
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//
//	TCIRowsetFind:	the base class for the rest of test cases in this
//						test module. 
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class TCIRowsetFind : public CRowsetObject
{
	private:

	protected:

		//@cmember: interface pointer for IRowsetFind
		IRowsetFind	*	m_pIRowsetFind;

		//@cmember: interface pointer for IRowset
		IRowset	*		m_pIRowset;

		//@cmember: interface pointer for IConvertType
		IConvertType *	m_pIConvertType;

		//@cmember:	accessory handle
		HACCESSOR		m_hAccessor;
		
		//@cmember:	accessory handle that will only be used as argument to IRowsetFind
		HACCESSOR		m_hRowsetFindAccessor;

		//@cmember:	the size of a row
		DBLENGTH		m_cRowSize;

		//@cmember:	the count of binding structure
		DBCOUNTITEM		m_cBinding;

		//@cmember: the array of binding strucuture
		DBBINDING *		m_rgBinding;

		//@cmember:	the pointer to the row buffer
		void *			m_pData;

		//@cmember:	the pointer to the pFindValue buffer
		BYTE *			m_pFindValue;

		//@cmember:	wherther Privlib FillInputBindings was used to fill m_pFindValue
		BOOL			m_fUsePrivlibFillInputBindings;

		//@cmember:	the pointers Bindings arrays
		DBCOUNTITEM		m_cFindBindings;
		DBBINDING * m_rgFindBindings;

		HROW *			m_rghRowsFound;
		DBCOUNTITEM		m_cRowsFound;

		//@cmember:	the pointer to the ISequentialStream interface
		ISequentialStream *	m_pISeqStream;

		//@mfunc: initialialize interface pointers
		BOOL	Init();

		//@mfunc: Terminate 
		BOOL	Terminate();

		//@mfunc: Create a command object and set properties, execute a sql statement,
		//		  and create a rowset object.  Create an accessor on the rowset 
		BOOL GetRowsetAndAccessor
		(	
			CTable				*pCTable,				
			EQUERY				eSQLStmt,				
			IID					riid,					
			ULONG				cProperties=0,			
			const DBPROPID		*rgProperties=NULL,				
			DBACCESSORFLAGS		dwAccessorFlags=DBACCESSOR_ROWDATA,		
			DBPART				dwPart=DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,					
			ECOLS_BOUND			eColsToBind=ALL_COLS_BOUND,			
			ECOLUMNORDER		eBindingOrder=FORWARD,			
			ECOLS_BY_REF		eColsByRef=NO_COLS_BY_REF,				
			WCHAR				*pwszTableName=NULL,	
			EEXECUTE			eExecute=EXECUTE_IFNOERROR,
			DBTYPE				dbTypeModifier=DBTYPE_EMPTY,
			BOOL				fBindLongColumns=FALSE
		);

				//@mfun: create an accessor on the rowset.  
		BOOL	GetAccessorOnRowset
		(
			DBACCESSORFLAGS		dwAccessorFlags=DBACCESSOR_ROWDATA,		
			DBPART				dwPart=DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,					
			ECOLS_BOUND			eColsToBind=ALL_COLS_BOUND,			
			ECOLUMNORDER		eBindingOrder=FORWARD,			
			ECOLS_BY_REF		eColsByRef=NO_COLS_BY_REF,				
			DBTYPE				dbTypeModifier=DBTYPE_EMPTY,
			BOOL				fBindLongColumns=TRUE
		);

		//@mfun: Get the bookmark for the row 
		BOOL GetBookmark
		(
			DBCOUNTITEM	ulRow,
			ULONG_PTR *	pcbBookmark,
			BYTE **		ppBookmark
		);

		BOOL BookmarkSkipped();

		BOOL RemoveDeleted();

		BOOL GetProp
		(
			DBPROPID DBPropID
		); 

		//@mfunc: release the memory referenced by the consumer's buffer
		void FreeMemory
		(
			CTable *	pCTable=g_pCTable
		);

		//@mfunc: release a rowset object and accessor created on it
		BOOL ReleaseRowsetAndAccessor();

		//@mfunc: release a accessor created on it
		BOOL ReleaseAccessorOnRowset();

		//mfunc: populate the table after delete some rows
		BOOL PopulateTable();

		//@mfunc: verify the position of the row handle in the row set
		BOOL VerifyRowPosition
		(
			HROW		hRow,			//row handle
			DBCOUNTITEM	cRow,			//potision expected
			CTable *	pCTable,		//pointer to the CTable
			EVALUE		eValue=PRIMARY	//eValue for MakeData
		);

		//@mfunc: verify the position of the cursor in the row set
		BOOL	VerifyCursorPosition
		(
			DBCOUNTITEM	cRow,			//the cursor potision expected
			CTable *	pCTable,		//pointer to the CTable
			BOOL		fMoveBack=FALSE,//whether move the cursor back to its original postion
										//if fMoveBack==FALSE; the cursor will be positioned one row 
										//after the original position.  If fMoveBack==TRUE,
										//DBPROP_CANSCROLLBACKWARDS needs to be set.
			EVALUE		eValue=PRIMARY	//eValue for MakeData
		);	

		BOOL	CreateCoerceFindValueAccessor
		(
			DBCOMPAREOP CompareOp, 
			CTable *	pCTable, 
			DBCOUNTITEM	ulRowNum, 
			DBCOUNTITEM	ulColToFind,
			DBTYPE		wBindType
		);		

		BOOL CreateFindValueAccessor
		( 
			DBCOMPAREOP		CompareOp,
			CTable *		pCTable,
			DBCOUNTITEM		ulRowNum,
			DBORDINAL		ulColNum,	
			DBTYPE			wColType,
			eSUBCOMPAREOP	eSubCompare,
			BOOL			fUseStream = FALSE,
			DBCOMPAREOP *	pwNewCompareOp = NULL,
			DBPART			dbPart = DBPART_INVALID,
			HRESULT	*		phrExpected = NULL
		);
		
		BOOL ReleaseFindValueAccessor
		(
			DBTYPE wColType
		);

		BOOL CallFindNextRows
		(
			CTable *		pCTable,			// pointer to CTable object
			BYTE *			pBookmark,			// Bookmark to fetch from, if any
			ULONG_PTR		cbBookmark,			// Length of bookmark.
			DBROWCOUNT		lRowsToFetch,		// maps to cRows
			DBROWOFFSET		lOffset,			// maps to lOffset
			DBORDINAL		ulColToMatch,		// Which column to match
			DBCOUNTITEM		ulRowToMatch,		// Is there a row where the find should happen? 0 - no match
	        HRESULT			hrExpAccessor,	    // Expected HRESULT for Create Accessor Step
			DBCOUNTITEM		ulRowsExpected,		// Expected count of rows
			BOOL			fReleaseRows=TRUE,	// Flag to Release Rows.
			DBCOMPAREOP		CompareOp=DBCOMPAREOPS_EQ,  // Any particular preference for comparing?
			eSUBCOMPAREOP	eSubCompare=SUBOP_EMPTY,	// Some comparision are rich enough to deserve a sub comparision
			HROW *			rghRows = NULL,		// Use client or provider memory, default=provider
			BOOL			fCheckRows = TRUE,	// verify rows by comparing data ?
			BOOL			fUseStream = FALSE,	// Use ISeqStream ?
        	HRESULT			hrExpFind = -1      // Second expected HRESULT for the FindNextRow step
		);

		BOOL AlterData
		(
			BYTE **			ppMakeData, 
			DBCOMPAREOP		CompareOp, 
			DBTYPE			wColType,
			DBLENGTH *		pcbDataLength,
			eSUBCOMPAREOP	eSubCompare
		);
		
		BOOL AlterNumericData
		(
			BYTE **		ppMakeData, 
			DBCOMPAREOP CompareOp,
			DBTYPE		wColType,
			DBLENGTH *	pcbDataLength
		);

		BOOL AlterCharacterData
		(
			void **			ppMakeData,
			DBCOMPAREOP		CompareOp,
			DBTYPE			wColType,
			DBLENGTH *		pcbDataLength,
			eSUBCOMPAREOP	eSubCompare
		);

		BOOL AlterVarnumericData
		(
			BYTE **		ppMakeData,
			DBCOMPAREOP CompareOp,
			DBLENGTH *	pcbDataLength
		);
		
		BOOL BindingTypeTest
		(
			CTable *	pCTable,
			DBTYPE	wBindingType
		);

		BOOL CompareOpTest
		(
			CTable *		pCTable,
			DBCOMPAREOP		CompareOp,
			eSUBCOMPAREOP	eSubCompare,
			BOOL			fUseStream=FALSE
		);


		BOOL DeleteRow
		(
			CTable *	pCTable, 
			DBCOUNTITEM	ulRowToDelete
		);

		DWORD TC_FindCompareOps
		(
			DBID * pColDBID
		);

		BOOL AlteringRowsIsOK();

		BOOL IsColumnMinimumFindable
		(
			CCol *		pCol,
			DBCOMPAREOP CompareOp
		);

		BOOL IsTypeFindable
		(
			DBTYPE			wType,
			DBCOMPAREOP		CompareOp,
			DBTYPE			wSubType = DBTYPE_EMPTY
		);

		BOOL IsStringType
		(
			DBTYPE wType
		);

		BOOL IsStringCompareOp
		(
			DBCOMPAREOP CompareOp
		);

		BOOL IsColumnFindable
		(
			CCol *		pCol,
			DBCOMPAREOP CompareOp
		);

		HRESULT RestartRowPosition();

		BOOL GetVariableLengthStrAndUpdatable
		(
			DBORDINAL *	pulColNum, 
			DBCOMPAREOP CompareOp,
			BOOL		fGetLongStr = FALSE,
			DBTYPE *	pwType = NULL
		);

		BOOL GetNonNullableCol
		(
			DBORDINAL *	pulColNum, 
			DBCOMPAREOP CompareOp,
			BOOL		fGetLongStr,
			DBTYPE *	pwType
		);

	public:
		//constructor and destructor
		TCIRowsetFind(WCHAR *wstrTestCaseName);
		~TCIRowsetFind();
};


//--------------------------------------------------------------------
// @mfunc base class TCIRowsetFind constructor, must take testcase name
//			as parameter.
//
TCIRowsetFind::TCIRowsetFind
(
	WCHAR * wstrTestCaseName	//Takes TestCase Class name as parameter
) : CRowsetObject (wstrTestCaseName) 
{
	//initialize member data
	m_pIRowsetFind	= NULL;
	m_pIRowset		= NULL;
	m_pIConvertType = NULL;
	m_pFindValue	= NULL;
	m_pIAccessor	= NULL;
	m_pISeqStream	= NULL;
	m_hAccessor		= NULL;
	m_hRowsetFindAccessor = NULL;
	m_cRowSize		= 0;
	m_cBinding		= 0;
	m_rgBinding		= NULL;
	m_pData			= NULL;
	m_hr			= S_OK;

	m_cFindBindings =0;
	m_rgFindBindings = NULL;
	
	m_cRowsFound = 0;
	m_rghRowsFound = NULL;
}


//--------------------------------------------------------------------
// @mfunc base class TCIRowsetFind destructor
//
TCIRowsetFind::~TCIRowsetFind()
{

}


//--------------------------------------------------------------------
//@mfunc: Init creates a Data Source object, a DB Session object, 
//and a command object and initialize corresponding interface pointers.
//
//--------------------------------------------------------------------
BOOL TCIRowsetFind::Init()
{
	return (COLEDB::Init());
}


//--------------------------------------------------------------------
//@mfunc: Terminate release the data source object, DB Session object, Command object
//
//--------------------------------------------------------------------
BOOL TCIRowsetFind::Terminate()
{
	return (COLEDB::Terminate());
}

//--------------------------------------------------------------------
//@mfunc: Create a command object and set properties, execute a sql statement,
//		  and create a rowset object.  Create an accessor on the rowset 
//
//--------------------------------------------------------------------
BOOL	TCIRowsetFind::GetRowsetAndAccessor
(	
	CTable *			pCTable,				//the pointer to the table object	
	EQUERY				eSQLStmt,				//the SQL Statement to create
	IID					riid,					//the interface pointer to return
	ULONG				cProperties,			//the count of properties
	const DBPROPID *	rgProperties,			//the array of properties to be set
	DBACCESSORFLAGS		dwAccessorFlags,		//the accessor flags
	DBPART				dwPart,					//the type of binding
	ECOLS_BOUND			eColsToBind,			//the columns in accessor
	ECOLUMNORDER		eBindingOrder,			//the order to bind columns
	ECOLS_BY_REF		eColsByRef,				//which columns to bind by reference
	WCHAR *				pwszTableName,			//the table name for the join statement
	EEXECUTE			eExecute,				//execute only if all properties are set
	DBTYPE				dbTypeModifier,			//the type modifier used for accessor
	BOOL				fBindLongColumns		//whether to long columns
)
{
	HRESULT		hr; 	
	ULONG		cProp = 0, i = 0;
	DBPROPSET	rgPropSets[1];
	BOOL		bReturn = FALSE;
	BLOBTYPE	blobType;
	BOOL		bIsOrderedBookMarkProperty = FALSE;
	
	//init rgPropSets[0]
	rgPropSets[0].rgProperties   = NULL;
	rgPropSets[0].cProperties    = cProperties;
	rgPropSets[0].guidPropertySet= DBPROPSET_ROWSET;

	rgPropSets[0].rgProperties=(DBPROP *)PROVIDER_ALLOC
		(sizeof(DBPROP) * (cProperties + 1));
	memset(rgPropSets[0].rgProperties, 0, sizeof(DBPROP) * (cProperties + 1));

	TESTC(rgPropSets[0].rgProperties != NULL);

	//go through the loop to set every DB Property required
	for(i=0; i<cProperties; i++)
	{
		VariantInit(&(rgPropSets[0].rgProperties[cProp].vValue));
		//Set KAGPROP_QUERYBASEDUPDATES if need be
		switch(rgProperties[i])
		{
			case DBPROP_UPDATABILITY:
				rgPropSets[0].rgProperties[cProp].dwPropertyID=DBPROP_UPDATABILITY;
				rgPropSets[0].rgProperties[cProp].dwOptions=DBPROPOPTIONS_REQUIRED;
				rgPropSets[0].rgProperties[cProp].vValue.vt=VT_I4;
				rgPropSets[0].rgProperties[cProp].vValue.lVal=
				DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_DELETE|DBPROPVAL_UP_INSERT;
				cProp++;
				break;
			
			case DBPROP_IRowsetLocate:
				fBindLongColumns = BLOB_LONG;
				// intentional fallthru
			default:
				if (rgProperties[i]== DBPROP_ORDEREDBOOKMARKS)
				{
					bIsOrderedBookMarkProperty = TRUE;
				}
				rgPropSets[0].rgProperties[cProp].dwPropertyID   = rgProperties[i];
				rgPropSets[0].rgProperties[cProp].dwOptions      = DBPROPOPTIONS_REQUIRED;
				rgPropSets[0].rgProperties[cProp].vValue.vt      = VT_BOOL;
				V_BOOL(&rgPropSets[0].rgProperties[cProp].vValue)= VARIANT_TRUE;
				cProp++;
				break;
		}
	}

	//Set properties and execute the SQL statement
	//May fail due to combinations of properties
	ASSERT(SUCCEEDED(SetRowsetProperties(rgPropSets, 1)));

	if(fBindLongColumns)
		blobType=BLOB_LONG;
	else
		blobType=NO_BLOB_COLS;

	//Set CTable object
	SetTable(pCTable, DELETETABLE_NO);
	
	hr = CreateRowsetObject(eSQLStmt, riid, EXECUTE_IFNOERROR);
	
	if(hr==DB_S_ERRORSOCCURRED || hr==DB_E_ERRORSOCCURRED)
		goto CLEANUP;
	
	TESTC_(hr,S_OK);

	//queryinterface for IRowsetFind, IRowset, and IConvertType
	TESTC(VerifyInterface(m_pIAccessor, IID_IRowset, ROWSET_INTERFACE,
						(IUnknown **)&m_pIRowset));
	TESTC(VerifyInterface(m_pIRowset, IID_IRowsetFind, ROWSET_INTERFACE, 
						(IUnknown **)&m_pIRowsetFind));
	TESTC(VerifyInterface(m_pIRowset, IID_IConvertType, ROWSET_INTERFACE, 
						(IUnknown **)&m_pIConvertType));

	//if dwAccessorFlags=DBACCESSOR_PASSBYREF, no need to create an accessor
	if(dwAccessorFlags == DBACCESSOR_PASSBYREF)
	{
		bReturn = TRUE;
		goto CLEANUP;
	}

	//create an accessor on the rowset
	TESTC_(GetAccessorAndBindings(m_pIRowset,dwAccessorFlags,&m_hAccessor,
		&m_rgBinding,&m_cBinding,&m_cRowSize,dwPart,eColsToBind,eBindingOrder,
		eColsByRef,NULL,NULL,NULL,dbTypeModifier,0,NULL,NULL,
		NO_COLS_OWNED_BY_PROV,DBPARAMIO_NOTPARAM,blobType),S_OK);

	//allocate memory for the row
	m_pData = PROVIDER_ALLOC(m_cRowSize);
	if(m_pData)
		bReturn = TRUE;

CLEANUP:
	//free the memory
	PROVIDER_FREE(rgPropSets[0].rgProperties);

	return bReturn;
}

BOOL	TCIRowsetFind::GetAccessorOnRowset
(
	DBACCESSORFLAGS		dwAccessorFlags,		
	DBPART				dwPart,					
	ECOLS_BOUND			eColsToBind,			
	ECOLUMNORDER		eBindingOrder,			
	ECOLS_BY_REF		eColsByRef,				
	DBTYPE				dbTypeModifier,
	BOOL				fBindLongColumns
)
{
	BLOBTYPE	blobType;

	if(fBindLongColumns)
		blobType=BLOB_LONG;
	else
		blobType=NO_BLOB_COLS;

		//create an accessor on the rowset
	if(!CHECK(GetAccessorAndBindings(m_pIRowset,dwAccessorFlags,&m_hAccessor,
		&m_rgBinding,&m_cBinding,&m_cRowSize,dwPart,eColsToBind,eBindingOrder,
		eColsByRef,NULL,NULL,NULL,dbTypeModifier,blobType),S_OK))
			return FALSE;

	//allocate memory for the row
	m_pData=PROVIDER_ALLOC(m_cRowSize);

	if(!m_pData)
		return FALSE;

	return TRUE;
}


//--------------------------------------------------------------------
//@mfun: Get the bookmark for the row.  The function has to be called
//		after the GetRowsetAndAccessor that creates an accessor on the 
//		rowset.
//
//--------------------------------------------------------------------
BOOL TCIRowsetFind::GetBookmark
(
	DBCOUNTITEM	ulRow,
	ULONG_PTR *	pcbBookmark,
	BYTE **		ppBookmark
)
{
	BOOL		fPass=FALSE;
	HROW *		pHRow=NULL;
	DBCOUNTITEM	cCount;

	//ulRow has to start with 1
	if(!pcbBookmark || !ppBookmark || !ulRow)
		return FALSE;

	//restart the cursor position
	if(!CHECK(m_pIRowset->RestartPosition(NULL),S_OK))
		return FALSE;

	//fetch the row
	TESTC_(m_pIRowset->GetNextRows(NULL,(ulRow-1),1,&cCount,&pHRow),S_OK);
	//only one row handle is retrieved
	COMPARE(cCount, 1);

	//get the data
	TESTC_(m_pIRowset->GetData(*pHRow, m_hAccessor, m_pData),S_OK);

	//make sure the 0 column is for bookmark
	if(!COMPARE(m_rgBinding[0].iOrdinal, 0))
	{
		FreeMemory();
		goto CLEANUP;
	}

	//get the length of the bookmark
	*pcbBookmark= *((DBLENGTH *)((BYTE *)m_pData+m_rgBinding[0].obLength));

	//allocate memory for bookmark
	*ppBookmark=(BYTE *)PROVIDER_ALLOC(*pcbBookmark);

	TESTC(*ppBookmark != NULL);

	//copy the value of the bookmark into the consumer's buffer
	memcpy(*ppBookmark, (void *)((BYTE *)m_pData+m_rgBinding[0].obValue), *pcbBookmark);

	//free the memory referenced by the consumer's buffer
	FreeMemory();

	fPass=TRUE;

CLEANUP:

	if(pHRow)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);			
		PROVIDER_FREE(pHRow);
	}

	//restart the cursor position
	if(!CHECK(m_pIRowset->RestartPosition(NULL),S_OK))
		return FALSE;

	return fPass;
}

//--------------------------------------------------------------
//
//	 Get information about properties
//
//-----------------------------------------------------------------
BOOL TCIRowsetFind::GetProp(DBPROPID	DBPropID)
{
	IRowsetInfo	*	pIRowsetInfo=NULL;
	ULONG			cProperty;
	DBPROPIDSET		DBPropIDSet;
	DBPROPSET *		pDBPropSet=NULL;
	BOOL			fSupported=FALSE;

	//initialize
	DBPropIDSet.guidPropertySet = DBPROPSET_ROWSET;
	DBPropIDSet.cPropertyIDs = 1;
	DBPropIDSet.rgPropertyIDs = &DBPropID;

	//QI for IRowsetInfo interface
	TESTC_(m_pIRowset->QueryInterface(IID_IRowsetInfo,(LPVOID *)&pIRowsetInfo),S_OK);
	TESTC(SUCCEEDED(pIRowsetInfo->GetProperties(1,&DBPropIDSet,&cProperty,&pDBPropSet)));

	if(V_BOOL(&pDBPropSet->rgProperties->vValue) == VARIANT_TRUE)
		fSupported=TRUE;

CLEANUP:

	for(ULONG i=0;i<cProperty;i++)
		PRVTRACE(L"[%d] status == %d \n",pDBPropSet[0].rgProperties[i].dwPropertyID,pDBPropSet[0].rgProperties[i].dwStatus);

	if(pDBPropSet)
	{
		if(pDBPropSet->rgProperties)
			PROVIDER_FREE(pDBPropSet->rgProperties);

		PROVIDER_FREE(pDBPropSet);
	}

	SAFE_RELEASE(pIRowsetInfo);
	return fSupported;
}


//--------------------------------------------------------------------
//@mfunc:	If BookmarkSkipped is supported on the rowset
//--------------------------------------------------------------------
BOOL TCIRowsetFind::BookmarkSkipped()
{
	IRowsetInfo	*	pIRowsetInfo=NULL;
	ULONG			cProperty;
	DBPROPID		DBPropID=DBPROP_BOOKMARKSKIPPED;
	DBPROPIDSET		DBPropIDSet;
	DBPROPSET *		pDBPropSet=NULL;
	BOOL			fSupported=FALSE;

	if(!g_rgDBPrpt[IDX_BookmarkSkipped].fSupported)
		return FALSE;

	//initialize
	DBPropIDSet.guidPropertySet = DBPROPSET_ROWSET;
	DBPropIDSet.cPropertyIDs = 1;
	DBPropIDSet.rgPropertyIDs = &DBPropID;

	//QI for IRowsetInfo interface
	TESTC_(m_pIRowset->QueryInterface(IID_IRowsetInfo,(LPVOID *)&pIRowsetInfo),S_OK);

	//ask for DBPROP_BOOKMARKSKIPPED
	TESTC_(pIRowsetInfo->GetProperties(1,&DBPropIDSet,&cProperty,
		&pDBPropSet),S_OK);

	if(V_BOOL(&pDBPropSet->rgProperties->vValue)==VARIANT_TRUE)
		fSupported=TRUE;

CLEANUP:

	if(pDBPropSet)
	{
		if(pDBPropSet->rgProperties)
			PROVIDER_FREE(pDBPropSet->rgProperties);

		PROVIDER_FREE(pDBPropSet);
	}

	SAFE_RELEASE(pIRowsetInfo);
	return fSupported;
}

//--------------------------------------------------------------------
// Populate the table after some rows are deleted
//--------------------------------------------------------------------
BOOL	TCIRowsetFind::PopulateTable()
{ 	
	long	cCnt;

	//delete all rows in the table.
	if(!CHECK(g_pCTable->DeleteRows(ALLROWS),S_OK))
		return FALSE;

	// freshly populate
	for(cCnt=1; cCnt<=g_lRowLast; cCnt++)
		if(!CHECK(g_pCTable->Insert(cCnt, PRIMARY),S_OK))
			return FALSE;

	return TRUE;
}


//--------------------------------------------------------------------
//@mfunc:	If RemoveDeleted is supported on the rowset
//--------------------------------------------------------------------
BOOL TCIRowsetFind::RemoveDeleted()
{
	IRowsetInfo	*	pIRowsetInfo=NULL;
	ULONG			cProperty;
	DBPROPID		DBPropID=DBPROP_REMOVEDELETED;
	DBPROPIDSET		DBPropIDSet;
	DBPROPSET *		pDBPropSet=NULL;
	BOOL			fSupported=FALSE;

	if(!g_rgDBPrpt[IDX_RemoveDeleted].fSupported)
		return FALSE;

	//initialize
	DBPropIDSet.guidPropertySet = DBPROPSET_ROWSET;
	DBPropIDSet.cPropertyIDs = 1;
	DBPropIDSet.rgPropertyIDs = &DBPropID;

	//QI for IRowsetInfo interface
	TESTC_(m_pIRowset->QueryInterface(IID_IRowsetInfo,(LPVOID *)&pIRowsetInfo),S_OK);

	//ask for DBPROP_BOOKMARKSKIPPED
	TESTC_(pIRowsetInfo->GetProperties(1,&DBPropIDSet,&cProperty,
		&pDBPropSet),S_OK);

	if(V_BOOL(&pDBPropSet->rgProperties->vValue)==VARIANT_TRUE)
		fSupported=TRUE;

CLEANUP:

	if(pDBPropSet)
	{
		if(pDBPropSet->rgProperties)
			PROVIDER_FREE(pDBPropSet->rgProperties);

		PROVIDER_FREE(pDBPropSet);
	}

	SAFE_RELEASE(pIRowsetInfo);
	return fSupported;
}


//--------------------------------------------------------------------
//@mfunc:	free the memory referenced by the consumer's buffer
//			The function has to be called after IRowset::GetData()
//
//--------------------------------------------------------------------
void TCIRowsetFind::FreeMemory(CTable *pCTable)
{
	//make sure m_pData is not NULL
	if(!COMPARE(!m_pData, NULL))
		return;

	//make sure the columns are bound 
	if(!m_rgTableColOrds)
		return;

	//call compareData with the option to free the memory referenced by the consumer's 
	//buffer without comparing data
	CompareData(m_cRowsetCols,m_rgTableColOrds,1,m_pData,m_cBinding,m_rgBinding,pCTable,
				NULL,PRIMARY,FREE_ONLY);
}


//--------------------------------------------------------------------
//@mfunc: release a rowset object and accessor created on it
//
//--------------------------------------------------------------------
BOOL TCIRowsetFind::ReleaseRowsetAndAccessor()
{
	BOOL	fPass = TRUE;

	//free the consumer buffer
	PROVIDER_FREE(m_pData);
	PROVIDER_FREE(m_rgBinding);
	PROVIDER_FREE(m_rgTableColOrds);

	//free accessor handle
	if(m_hAccessor)
	{
		if(!CHECK(m_pIAccessor->ReleaseAccessor(m_hAccessor,NULL), S_OK))
				fPass=FALSE;

		m_hAccessor=NULL;
	}

	SAFE_RELEASE(m_pIRowset);
	SAFE_RELEASE(m_pIRowsetFind);
	SAFE_RELEASE(m_pIConvertType);

	ReleaseRowsetObject();  //releases m_pIAccessor
	ReleaseCommandObject(); //releases m_pICommand
	ReleaseDBSession();
	ReleaseDataSourceObject();

	return fPass;
}


BOOL TCIRowsetFind::ReleaseAccessorOnRowset()
{
	BOOL		fPass=TRUE;

	//free the consumer buffer
	PROVIDER_FREE(m_pData);
	PROVIDER_FREE(m_rgBinding);

	//free accessor handle
	if(m_hAccessor)
	{
		if(!CHECK(m_pIAccessor->ReleaseAccessor(m_hAccessor,NULL), S_OK))
				fPass=FALSE;

		m_hAccessor=NULL;
	}

	return fPass;
}

//--------------------------------------------------------------------
//
//@mfunc: verify the position of the row handle in the row set
//
//	Precondition: The function has to be called after GetRowsetAndAccessor that
//				  creates a rowset and an accessor
//
//--------------------------------------------------------------------
BOOL	TCIRowsetFind::VerifyRowPosition
(
	HROW		hRow,		//row handle
	DBCOUNTITEM	cRow,		//position expected
	CTable *	pCTable,	//pointer to the CTable
	EVALUE		eValue		//if the accessor is ReadColumnsByRef
)
{
	//input validation
	if(!pCTable || !m_pIRowset || !m_pData)
		return FALSE;

	//Get Data for the row
	if(!CHECK(m_pIRowset->GetData(hRow,m_hAccessor,m_pData),S_OK))
		return FALSE;

	//compare the data with the row expected in the rowset
	if(!CompareData(m_cRowsetCols,m_rgTableColOrds,cRow,m_pData,m_cBinding,m_rgBinding,pCTable,
					NULL,eValue))
		return FALSE;

	return TRUE;
}


//--------------------------------------------------------------------
//
//@mfunc: verify the position of the cursor in the row set
//
//	Precondition: The function has to be called after GetRowsetAndAccessor that
//				  creates a rowset and an accessor.  
//
//--------------------------------------------------------------------
BOOL	TCIRowsetFind::VerifyCursorPosition
(
	DBCOUNTITEM	cRow,		//the cursor potision expected
	CTable *	pCTable,	//pointer to the CTable
	BOOL		fMoveBack,	//whether move the cursor back to its original postion
							//if fMoveBack==FALSE; the cursor will be positioned one row 
							//after the original position.  If fMoveBack==TRUE,
							//DBPROP_CANSCROLLBACKWARDS needs to be set.
	EVALUE		eValue		//eValue for MakeData
)
{
	HROW		hRow[1];
	HROW *		pHRow = hRow;
	DBCOUNTITEM	cRows;
	BOOL		fTestPass=TRUE;

	//input validation
	if(!pCTable || !m_pIRowset || !m_pData)
		return FALSE;

	//Get a row handle
	if(!CHECK(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow),S_OK))
		return FALSE;
	
	//call VerifyRowPosition
	if(!COMPARE(VerifyRowPosition(hRow[0],cRow,pCTable,eValue),TRUE))
		fTestPass=FALSE;
	
	//release the row handle
	CHECK(m_pIRowset->ReleaseRows(1,hRow,NULL,NULL,NULL),S_OK);

	//reposition the cursor to its original position
	if(fMoveBack)
	{
		if(!CHECK(m_pIRowset->GetNextRows(NULL,-1,0,&cRows,&pHRow),S_OK))
			fTestPass=FALSE;
	}

	return (fTestPass);
		
}


//--------------------------------------------------------------------
//
//@mfunc: Create Accessor for the pFindValue argument to IRowsetFind::FindNextRows
// Bind to a type other than the native type to test coercion
//
//--------------------------------------------------------------------
BOOL	TCIRowsetFind::CreateCoerceFindValueAccessor
(
	DBCOMPAREOP CompareOp, 
	CTable *	pCTable, 
	DBCOUNTITEM	ulRowNum, 
	DBORDINAL	ulColToFind,
	DBTYPE		wBindType
)
{
	BOOL		fChanged = FALSE;
	BOOL		fRet = FALSE;
	WCHAR		wszData[2000];
	CCol		TempCol;
	USHORT		ulsize;
	DBLENGTH	cbDataLength = 0;
	BYTE *		pMakeData;
	DBTYPE		wVariantType = DBTYPE_EMPTY, wColType, wByRefType = DBTYPE_EMPTY;
	HRESULT	 hr;

	pCTable->GetColInfo(ulColToFind, TempCol);
	wColType = TempCol.GetProviderType();

	wByRefType = wBindType & DBTYPE_BYREF;
	wBindType &= (~DBTYPE_BYREF);

	// This only has support for these major types
	if ( wBindType != DBTYPE_WSTR && wBindType != DBTYPE_STR && wBindType != DBTYPE_BSTR && wBindType != DBTYPE_VARIANT )
		return FALSE;

	// Only supports EQ; may add more support 
	if ( CompareOp != DBCOMPAREOPS_EQ )
		return FALSE;

	if  ( !IsColumnMinimumFindable(&TempCol, CompareOp) )
		return FALSE;

	if ( ulRowNum == 0 )
	{
		// create data that won't match by creating data for a row that doesn't exist yet.
		ulRowNum = pCTable->GetNextRowNumber();
	}

	if (!SUCCEEDED(hr = pCTable->MakeData(	wszData, 
											ulRowNum,
											ulColToFind, 
											PRIMARY, 
											( wColType == DBTYPE_VARIANT && ulRowNum > ULONG(g_lRowLast) ? DBTYPE_BSTR : wColType ), 
											FALSE, 
											&wVariantType)) )
		goto CLEANUP;

	if ( ulRowNum > ULONG(g_lRowLast) )
		wVariantType = DBTYPE_BSTR;

	if ( hr == S_OK && wVariantType != DBTYPE_NULL )
	{
		if ( CompareOp & DBCOMPAREOPS_CASEINSENSITIVE )
		{
			// Change first char to lowercase.
			char	szAnsi[3], szDest[3];
			WCHAR	wszDest[2];
			DWORD	cbWritten;
			
			cbWritten = WideCharToMultiByte(CP_ACP, 0, wszData, 1, szAnsi, 3, NULL, NULL);
			szAnsi[cbWritten] = '\0';
			LCMapStringA(GetUserDefaultLCID(),LCMAP_LOWERCASE,szAnsi,-1,szDest,3);
			cbWritten = MultiByteToWideChar(CP_ACP, 0, szDest, -1, wszDest, 2);

			wszData[0] = wszDest[0];
		}

		if ( wBindType == DBTYPE_VARIANT )
		{
			BSTR	bstr = SysAllocString(wszData);

			pMakeData = (BYTE *)DBTYPE2VARIANT(&bstr, VT_BSTR);
			SysFreeString(bstr);
		}
		else
		{
			pMakeData = (BYTE *)WSTR2DBTYPE(wszData, wBindType , &ulsize);
		}

		cbDataLength = GetDataLength(pMakeData, wBindType, ulsize);
		m_pFindValue = (BYTE *)PROVIDER_ALLOC(cbDataLength+sizeof(FindValueInfo));				
		
		// Set up our m_pFindValue struct to match offsets we give to our Binding.
		switch ( wBindType | wByRefType )
		{
		case DBTYPE_BSTR | DBTYPE_BYREF:
		case DBTYPE_STR | DBTYPE_BYREF:
		case DBTYPE_WSTR | DBTYPE_BYREF:
		case DBTYPE_VARIANT | DBTYPE_BYREF:
			memcpy(m_pFindValue+offsetof(FindValueInfo, pValue), &pMakeData, cbDataLength); 
			break;
		default:
			memcpy(m_pFindValue+offsetof(FindValueInfo, pValue), pMakeData, cbDataLength); 
		}
		
	}

	
	if ( hr == S_FALSE || wVariantType == DBTYPE_NULL)
	{
		m_pFindValue = (BYTE *)PROVIDER_ALLOC(sizeof(DBSTATUS)+2*sizeof(DBLENGTH));
		*(DBSTATUS *)(m_pFindValue+offsetof(FindValueInfo, dbsStatus)) = DBSTATUS_S_ISNULL;

		// bogus length shouldn't matter since NULL status was set
		*(DBLENGTH *)(m_pFindValue+offsetof(FindValueInfo, cbLength)) = MAXDBCOUNTITEM;
	}
	else
	{
		*(DBSTATUS *)(m_pFindValue+offsetof(FindValueInfo, dbsStatus)) = DBSTATUS_S_OK;
	
		switch ( wBindType )
		{
		case DBTYPE_WSTR:
			*(DBLENGTH *)(m_pFindValue+offsetof(FindValueInfo, cbLength)) = cbDataLength-sizeof(WCHAR);
			break;
		case DBTYPE_STR:
			*(DBLENGTH *)(m_pFindValue+offsetof(FindValueInfo, cbLength)) = cbDataLength-sizeof(char);
			break;
		default:
			*(DBLENGTH *)(m_pFindValue+offsetof(FindValueInfo, cbLength)) = cbDataLength;
			break;		
		}
	}

	// Create the binding
	DBBINDING Binding;

	Binding.iOrdinal	= ulColToFind;
	Binding.dwPart		= DBPART_VALUE | DBPART_STATUS | DBPART_LENGTH;
	Binding.eParamIO	= DBPARAMIO_INPUT;	
	Binding.pTypeInfo	= NULL;
	Binding.obValue		= offsetof(FindValueInfo, pValue);
	Binding.cbMaxLen	= cbDataLength;
	Binding.obLength	= offsetof(FindValueInfo, cbLength);
	Binding.obStatus	= offsetof(FindValueInfo, dbsStatus);
	Binding.dwMemOwner	= DBMEMOWNER_CLIENTOWNED;
	Binding.wType		= wBindType | wByRefType;
	Binding.pBindExt	= NULL;
	Binding.bPrecision	= BYTE(TempCol.GetPrecision());
	Binding.bScale		= BYTE(TempCol.GetScale());	

	// Lets create the accessor.
	if ( FAILED( m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA, 1, &Binding, sizeof(FindValueInfo)+cbDataLength, 
						&m_hRowsetFindAccessor, NULL)) )
			goto CLEANUP;

	fRet = TRUE;

CLEANUP:

	if( !wByRefType )
	{
		PROVIDER_FREE(pMakeData);
	}

	return fRet;
}



//--------------------------------------------------------------------
//
//@mfunc: Create Accessor for the pFindValue argument to IRowsetFind::FindNextRows
//
//--------------------------------------------------------------------
BOOL	TCIRowsetFind::CreateFindValueAccessor
( 
	DBCOMPAREOP		CompareOp,
	CTable *		pCTable,
	DBLENGTH		ulRowNum,
	DBORDINAL		ulColNum,
	DBTYPE			wColType,
	eSUBCOMPAREOP	eSubCompare,
	BOOL			fUseStream,
	DBCOMPAREOP	*	pwNewCompareOp,
	DBPART			dbPart,
	HRESULT	*		phrExpected
)
{
	BOOL		fChanged = FALSE;
	BOOL		fRet = FALSE;
	WCHAR		wszData[2000];
	CCol		TempCol;
	USHORT		ulsize;
	DBLENGTH	cbDataLength = 0;
	BYTE *		pMakeData;
	DBTYPE		wVariantType = DBTYPE_EMPTY;
	HRESULT		hr = S_FALSE;
	void *		pvData=NULL;

	pCTable->GetColInfo(ulColNum, TempCol);

	if(!IsColumnMinimumFindable(&TempCol, CompareOp))
		return FALSE;

	if(eSubCompare != SUBOP_ALWAYS_NULL)
	{
		if(ulRowNum == 0)
		{
			// create data that won't match by creating data for a row that doesn't exist yet.
			ulRowNum = pCTable->GetNextRowNumber();
		}

		TESTC(SUCCEEDED(hr = pCTable->MakeData(
				wszData, 
				ulRowNum,
				ulColNum, 
				PRIMARY, 
				wColType, 
				FALSE, 
				&wVariantType)));
	}

	if(hr == S_OK && wVariantType != DBTYPE_NULL)
	{
		if ( CompareOp & DBCOMPAREOPS_CASEINSENSITIVE )
		{
			// Change all the characters to lower case
			WCHAR *	pwsz = NULL;

			for (pwsz=wszData; *pwsz; pwsz++)
				*pwsz = MapWCHAR(*pwsz, LCMAP_LOWERCASE);
		}

		pMakeData = (BYTE *)WSTR2DBTYPE(wszData, ( wColType == DBTYPE_VARIANT ? wVariantType : wColType ) , &ulsize );
		cbDataLength = GetDataLength(pMakeData, wColType, ulsize);
	
		if ( !AlterData(&pMakeData, CompareOp, ( wColType == DBTYPE_VARIANT ? wVariantType : wColType ), &cbDataLength, eSubCompare))
		{
			if ( pwNewCompareOp )
				*pwNewCompareOp = DBCOMPAREOPS_EQ;
		}

		if ( !fUseStream )
			m_pFindValue = (BYTE *)PROVIDER_ALLOC(cbDataLength+sizeof(FindValueInfo));
	
		if ( wColType == DBTYPE_VARIANT )
		{
			pvData = DBTYPE2VARIANT(pMakeData, wVariantType);
		}
		else 
			pvData = pMakeData;

		if ( fUseStream )
		{
			ULONG cbTmp = 0;

			ASSERT(cbDataLength <= ULONG_MAX);
			m_pISeqStream = new CStorage();
			m_pISeqStream->Write(pMakeData, (ULONG)cbDataLength, &cbTmp);
			m_pFindValue = (BYTE *)m_pISeqStream;
		}
		else
		{
		// Set up our m_pFindValue struct to match offsets we give to our Binding.
			switch ( wColType )
			{
			case DBTYPE_STR | DBTYPE_BYREF:
			case DBTYPE_WSTR | DBTYPE_BYREF:
				memcpy(m_pFindValue+offsetof(FindValueInfo, pValue), &pvData, cbDataLength); 
				break;
			default:
				memcpy(m_pFindValue+offsetof(FindValueInfo, pValue), pvData, cbDataLength); 
			}
		}
	}
	
	if ( hr == S_FALSE || wVariantType == DBTYPE_NULL)
	{
		// The cell the test is trying to match contains a NULL.
		// In most cases, this means the original CompareOp cannot be used
		if (CompareOp != DBCOMPAREOPS_IGNORE)
		{
			// If the CompareOp is not IGNORE
			if ( pwNewCompareOp )
				*pwNewCompareOp = DBCOMPAREOPS_EQ;
			if ( !fUseStream )
				m_pFindValue = (BYTE *)PROVIDER_ALLOC(sizeof(DBSTATUS)+2*sizeof(DBLENGTH));
			*(DBSTATUS *)(m_pFindValue+offsetof(FindValueInfo, dbsStatus)) = DBSTATUS_S_ISNULL;

			// bogus length shouldn't matter since NULL status was set
			*(DBLENGTH *)(m_pFindValue+offsetof(FindValueInfo, cbLength)) = MAXDBCOUNTITEM-1;
		}
	}
	else
	{
		*(DBSTATUS *)(m_pFindValue+offsetof(FindValueInfo, dbsStatus)) = DBSTATUS_S_OK;
		
		switch ( wColType )
		{
		case DBTYPE_WSTR:
			*(DBLENGTH *)(m_pFindValue+offsetof(FindValueInfo, cbLength)) = cbDataLength-sizeof(WCHAR);
			break;
		case DBTYPE_STR:
			*(DBLENGTH *)(m_pFindValue+offsetof(FindValueInfo, cbLength)) = cbDataLength-sizeof(char);
			break;
		default:
			*(DBLENGTH *)(m_pFindValue+offsetof(FindValueInfo, cbLength)) = cbDataLength;
			break;		
		}
	}

	// Create the binding
	DBBINDING Binding;
	DBOBJECT ObjectStruct;

	if ( !fUseStream )
	{
		Binding.iOrdinal = ulColNum;
		Binding.dwPart = DBPART_VALUE | DBPART_STATUS | DBPART_LENGTH;
		Binding.eParamIO = DBPARAMIO_INPUT;	
		Binding.pTypeInfo = NULL;
		Binding.pObject = NULL;
		Binding.obValue = offsetof(FindValueInfo, pValue);
		Binding.cbMaxLen = cbDataLength;
		Binding.obLength = offsetof(FindValueInfo, cbLength);
		Binding.obStatus = offsetof(FindValueInfo, dbsStatus);
		Binding.dwMemOwner = DBMEMOWNER_CLIENTOWNED;
		Binding.wType = wColType;
		Binding.pBindExt = NULL;
		Binding.bPrecision = BYTE(TempCol.GetPrecision());
		Binding.bScale = BYTE(TempCol.GetScale());
	}
	else
	{
		ObjectStruct.dwFlags = STGM_WRITE;
		ObjectStruct.iid = IID_ISequentialStream;

		Binding.iOrdinal = ulColNum;
		Binding.dwPart = DBPART_VALUE;
		Binding.eParamIO = DBPARAMIO_INPUT;	
		Binding.pTypeInfo = NULL;
		Binding.pObject = &ObjectStruct;
		Binding.obValue = 0;
		Binding.cbMaxLen = 0;
		Binding.obLength = 0;
		Binding.obStatus = 0;
		Binding.dwMemOwner = DBMEMOWNER_CLIENTOWNED;
		Binding.wType = DBTYPE_IUNKNOWN;
		Binding.pBindExt = NULL;
		Binding.bPrecision = 0;
		Binding.bScale = 0;
	}

	// Lets create the accessor.
	TESTC(SUCCEEDED(m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA, 1,	&Binding,
			sizeof(FindValueInfo)+cbDataLength, 	&m_hRowsetFindAccessor, NULL)))
			goto CLEANUP;

	fRet = TRUE;

CLEANUP:

	PROVIDER_FREE(pvData);

	return fRet;
}

		
BOOL	TCIRowsetFind::ReleaseFindValueAccessor
(
	DBTYPE wdbType
)
{
	DBREFCOUNT cRefCount;

	if(!m_pIAccessor)
		return FALSE;

	if( m_hRowsetFindAccessor )
	{
		m_pIAccessor->ReleaseAccessor(m_hRowsetFindAccessor, &cRefCount);
		m_hRowsetFindAccessor = NULL;
	}

	//Release pData, accessor and bindings using privlib helpers because we used FillInputBindings
	if (m_pFindValue)
	{
		TESTC(SUCCEEDED(ReleaseInputBindingsMemory(m_cFindBindings, m_rgFindBindings, m_pFindValue, TRUE))) ;
		m_pFindValue = NULL;
	}

CLEANUP:	

	SAFE_DELETE(m_pISeqStream);
	if (m_rgFindBindings)
	{
		//FreeAccessorBindings(m_cFindBindings, m_rgFindBindings);
		for(DBCOUNTITEM i=0; i<m_cFindBindings; i++)
		{
			PROVIDER_FREE(m_rgFindBindings[i].pObject);
		}

		PROVIDER_FREE(m_rgFindBindings);
	}

	m_cFindBindings = 0;

	
	return (cRefCount == 0);
}

//--------------------------------------------------------------------
//
//@mfunc: Sets up accessor, pFindValue information and calls 
//		IRowsetFind::FindNextRows
//
//--------------------------------------------------------------------
BOOL	TCIRowsetFind::CallFindNextRows
(		
	CTable *		pCTable,		// Table to find from
	BYTE *			pBookmark,		// Bookmark to fetch from, if any
	ULONG_PTR		cbBookmark,		// Length of bookmark
	DBROWCOUNT		lRowsToFetch,	// maps to cRows
	DBROWOFFSET		lOffset,		// maps to lOffset
	DBORDINAL		ulColToMatch,	// Column to match
	DBCOUNTITEM		ulRowToMatch,	// Is there a row where the find should happen? 0 - no match
	HRESULT			hrExpAccessor,	// Expected HRESULT for Create Accessor Step
	DBCOUNTITEM		ulRowsExpected,	// Expected count of rows
	BOOL			fReleaseRows,	// flag to Release rows (optional)
	DBCOMPAREOP		CompareOp,		// Any particular preference for comparing? (optional)
	eSUBCOMPAREOP	eSubCompare,	// Some comparisions are rich enough to deserve a mulitple comparision operations
	HROW *			phRows,			// optional arg if client wants to control row handle mem
	BOOL			fCheckRows,		// verify rows by comparing data ?
	BOOL			fUseStream,		// Use ISeqStream in the test
   	HRESULT			hrExpFind       // Second expected HRESULT for the FindNextRow step
)
{
	m_rghRowsFound = ( phRows ? phRows : NULL );
	BOOL fClientOwnedRowsMemory = phRows!=NULL;
	BOOL		fTestPass = TEST_PASS;
	CCol		TempCol;
	DBTYPE		wColType = DBTYPE_EMPTY;
	DBCOMPAREOP wNewCompareOp = -1;

	pCTable->GetColInfo(ulColToMatch, TempCol);
	wColType = TempCol.GetProviderType();

	// Not all test variations provide the same HRESULT for both operations.
	// Default case is to expect they are the same unless otherwise specified
	if( hrExpFind == -1 ) hrExpFind = hrExpAccessor;
	
	//setup find value: note - DBPART_INVALID resolves to valid default value
	fTestPass = CreateFindValueAccessor(CompareOp, pCTable, ulRowToMatch, ulColToMatch, wColType, eSubCompare, fUseStream, &wNewCompareOp, DBPART_INVALID, &hrExpAccessor);
	TESTC(fTestPass);

	if(wNewCompareOp != - 1)
		CompareOp = wNewCompareOp;

	m_hr = m_pIRowsetFind->FindNextRow(DB_NULL_HCHAPTER, m_hRowsetFindAccessor, m_pFindValue, 
													CompareOp, cbBookmark, pBookmark, lOffset, 
													lRowsToFetch, &m_cRowsFound, &m_rghRowsFound);

	// Verify HRESULT
	if ( !CHECK(m_hr,hrExpFind) )
	{
		fTestPass = TEST_FAIL;
		goto CLEANUP;
	}

	// if FindNextRow failed, no need to validate returned rows
	if (FAILED(m_hr))
	{
		goto CLEANUP;
	}

	// Verify m_cRowsFound
	if ( !COMPARE(ulRowsExpected,m_cRowsFound) )
	{
		fTestPass = TEST_FAIL;
		goto CLEANUP;
	}

	// Verify row positions of rows in m_rghRows.
	if ( fCheckRows )
	{
		for ( ULONG i=0; i < ulRowsExpected; i ++ )
		{
			fTestPass = VerifyRowPosition(m_rghRowsFound[i], 
										(lRowsToFetch > 0 ? ulRowToMatch+i : ulRowToMatch-i),
										pCTable);
			if ( !COMPARE(fTestPass, TEST_PASS) )
				goto CLEANUP;
		}
	}

	// Cleanup
CLEANUP:
	if ( fReleaseRows && m_cRowsFound > 0 && m_rghRowsFound)
	{
		m_pIRowset->ReleaseRows(m_cRowsFound, m_rghRowsFound, NULL, NULL, NULL);
		m_cRowsFound = 0;
		//free memory only if provider allocated it
		if (!fClientOwnedRowsMemory)
			PROVIDER_FREE(m_rghRowsFound);
	}

	ReleaseFindValueAccessor(wColType);

	return fTestPass;
}

//--------------------------------------------------------------------
//
//@mfunc: Alters the data to according to the CompareOp
//
//--------------------------------------------------------------------
BOOL	TCIRowsetFind::AlterData
(
	BYTE **			ppMakeData, 
	DBCOMPAREOP		TestCompareOp,
	DBTYPE			wColType,
	DBLENGTH *		pcbDataLength,
	eSUBCOMPAREOP	eSubCompare
)
{
	DBCOMPAREOP CompareOp = TestCompareOp & ~(DBCOMPAREOPS_CASESENSITIVE | DBCOMPAREOPS_CASEINSENSITIVE);

	if ( (CompareOp == DBCOMPAREOPS_GE || CompareOp == DBCOMPAREOPS_LE ||
		  CompareOp == DBCOMPAREOPS_BEGINSWITH || CompareOp == DBCOMPAREOPS_CONTAINS) &&
		eSubCompare == SUBOP_ALWAYS_EQ )
		return TRUE;

	switch ( CompareOp )
	{
	case DBCOMPAREOPS_EQ:
		{
			switch ( wColType )
			{
			// Because multiple varnumerics can
			// map to the same value
			case DBTYPE_VARNUMERIC:
				if(!AlterVarnumericData((BYTE **)ppMakeData, DBCOMPAREOPS_EQ, pcbDataLength))
					return FALSE;
			}					
			break;
		}
	case DBCOMPAREOPS_BEGINSWITH:
	case DBCOMPAREOPS_CONTAINS:
		{
			switch ( wColType )
			{
			case DBTYPE_WSTR:
			case DBTYPE_STR:
			case DBTYPE_BSTR:
				if(!AlterCharacterData((void **)ppMakeData, CompareOp, wColType, pcbDataLength, eSubCompare))
					return FALSE;
				break;
			case DBTYPE_NULL:
				return FALSE;
			default:
				odtLog << L"Unexpected Type for CONTAINS: "<< wColType << ENDL;
				return FALSE;
			}
		break;
		}
	case DBCOMPAREOPS_NOTBEGINSWITH:
	case DBCOMPAREOPS_NOTCONTAINS:
		{
			switch ( wColType )
			{
			case DBTYPE_WSTR:
			case DBTYPE_STR:
			case DBTYPE_BSTR:
				if(!AlterCharacterData((void **)ppMakeData, CompareOp, wColType, pcbDataLength, eSubCompare))
					return FALSE;
				break;
			case DBTYPE_NULL:
				return FALSE;
			default:
				odtLog << L"Unexpected Type for NOTCONTAINS or NOTBEGINSWITH: "<<wColType<< ENDL;
				return FALSE;
			}
		break;
		}
	case DBCOMPAREOPS_GE:
	case DBCOMPAREOPS_GT:
		switch ( wColType )
		{
			case DBTYPE_WSTR:
			case DBTYPE_STR:
			case DBTYPE_BSTR:
				if(!AlterCharacterData((void **)ppMakeData, CompareOp, wColType, pcbDataLength, eSubCompare))
					return FALSE;
				break;
			case DBTYPE_I1:
			case DBTYPE_I2:
			case DBTYPE_I4:
			case DBTYPE_I8:
			case DBTYPE_UI1:
			case DBTYPE_UI2:
			case DBTYPE_UI4:
			case DBTYPE_UI8:
			case DBTYPE_R4:
			case DBTYPE_R8:
			case DBTYPE_NUMERIC:
			case DBTYPE_DECIMAL:
			case DBTYPE_CY:
			case DBTYPE_DATE:
			case DBTYPE_DBTIME:
			case DBTYPE_DBDATE:
			case DBTYPE_DBTIMESTAMP:
			case DBTYPE_ERROR:
			case DBTYPE_VARNUMERIC:
				AlterNumericData(ppMakeData, CompareOp, wColType, pcbDataLength);
				break;
			case DBTYPE_NULL:
				return FALSE;
			default:
				odtLog << L"Unexpected Type for GT/GE: " << wColType << ENDL;
				return FALSE;
			}
		break;		
	case DBCOMPAREOPS_LT:
	case DBCOMPAREOPS_LE:
		switch ( wColType )
		{
			case DBTYPE_WSTR:
			case DBTYPE_STR:
			case DBTYPE_BSTR:
				if(!AlterCharacterData((void **)ppMakeData, CompareOp, wColType, pcbDataLength, eSubCompare))
					return FALSE;
				break;
			case DBTYPE_I1:
			case DBTYPE_I2:
			case DBTYPE_I4:
			case DBTYPE_I8:
			case DBTYPE_UI1:
			case DBTYPE_UI2:
			case DBTYPE_UI4:
			case DBTYPE_UI8:
			case DBTYPE_R4:
			case DBTYPE_R8:
			case DBTYPE_NUMERIC:
			case DBTYPE_DECIMAL:
			case DBTYPE_CY:
			case DBTYPE_DATE:
			case DBTYPE_DBTIME:
			case DBTYPE_DBDATE:
			case DBTYPE_DBTIMESTAMP:
			case DBTYPE_ERROR:
			case DBTYPE_VARNUMERIC:
				if(!AlterNumericData(ppMakeData, CompareOp, wColType, pcbDataLength))
					return FALSE;
				break;
			case DBTYPE_NULL:
				return FALSE;
			default:
				odtLog << L"Unexpected Type for LT/LE: " << wColType << ENDL;
				return FALSE;
		}
		break;
	case DBCOMPAREOPS_NE:
		// Alter the data
		switch ( wColType )
		{
		case DBTYPE_BYTES:
			((BYTE *)*ppMakeData)[0] = ~(*(BYTE *)*ppMakeData);
			break;
		case DBTYPE_WSTR:
		case DBTYPE_BSTR:
		case DBTYPE_STR:		
			if(!AlterCharacterData((void **)ppMakeData, DBCOMPAREOPS_LT, wColType, pcbDataLength, SUBOP_EMPTY))
				return FALSE;
			break;
		case DBTYPE_BOOL:
			if ( *(VARIANT_BOOL *)*ppMakeData == VARIANT_TRUE )
				*(VARIANT_BOOL *)*ppMakeData = VARIANT_FALSE;
			else
				*(VARIANT_BOOL *)*ppMakeData = VARIANT_TRUE;
			break;
		case DBTYPE_NULL:
			return FALSE;
		case DBTYPE_GUID:
			memset(*ppMakeData, 0xCA, sizeof(GUID));
			break;
		case DBTYPE_UDT:
			odtLog << L"!!! AlterData NOT IMPLEMENTED for DBTYPE_UDT !!!" << ENDL;
			return FALSE;
		default:
			if(!AlterNumericData(ppMakeData, DBCOMPAREOPS_LT, wColType, pcbDataLength))
				return FALSE;
			break;
		}
		break;
	case DBCOMPAREOPS_IGNORE:
		if( wColType == DBTYPE_BSTR )
		{
			BSTR bstrVal = *((BSTR*)(*ppMakeData));
			// Jumble the first byte of data to test that it is actually ignored.
			memset(bstrVal, 0xCA, 1);
		}
		else
		{
			// Jumble the first byte of data to test that it is actually ignored.
			memset(*ppMakeData, 0xCA, 1);
		}
		break;
	default:
		break;
	}	
	
	return TRUE;
}


BOOL	TCIRowsetFind::AlterCharacterData
(
	void **			ppMakeData,
	DBCOMPAREOP		CompareOp,
	DBTYPE			wColType,  
	DBLENGTH *		pcbDataLength,
	eSUBCOMPAREOP	eSubCompare
)
{
	ASSERT(ppMakeData && *ppMakeData);

	void *		pMakeData = *ppMakeData;
	DBLENGTH	cchDataLength = 0;
	DBLENGTH	cbDataLength = *pcbDataLength;

	if ( wColType == DBTYPE_WSTR )
		cchDataLength = (cbDataLength - sizeof(WCHAR))/sizeof(WCHAR);
	else if ( wColType == DBTYPE_BSTR )
		cchDataLength = SysStringLen( *(BSTR *)(pMakeData) );
	else if ( wColType == DBTYPE_STR )
		cchDataLength = cbDataLength - sizeof(char);

	switch ( CompareOp )
	{
	case DBCOMPAREOPS_NOTCONTAINS:
	case DBCOMPAREOPS_NOTBEGINSWITH:
		// all MakeData Strings begin with a digit or the letter 'H'
		// use 'Q' to cause a match for NOTCONTAINS or NOTBEGINSWITH
		switch ( wColType )
		{
		case DBTYPE_WSTR:
			((WCHAR *)pMakeData)[0] = L'Q';
			((WCHAR *)pMakeData)[1] = L'\0';
			break;
		case DBTYPE_STR:
			((char *)pMakeData)[0] = 'Q';
			((char *)pMakeData)[1] = '\0';
			break;
		case DBTYPE_BSTR:
			{
			(*(BSTR *)pMakeData)[0] = L'Q';
			(*(BSTR *)pMakeData)[1] = L'\0';
			SysReAllocString((BSTR *)pMakeData, *(BSTR *)pMakeData);
			break;
			}
		default:
			odtLog << L"Unexpected Type for NOTBEGINSWITH/NOTCONTAINS: " << wColType << ENDL;
			return FALSE;
		}
		break;

	case DBCOMPAREOPS_BEGINSWITH:
		// blank out some of the last characters
		switch ( wColType )
		{
		case DBTYPE_WSTR:
			((WCHAR *)pMakeData)[cchDataLength-3] = L'\0';
			break;
		case DBTYPE_STR:
			{
			char *szTmp = CharPrevExA(CP_ACP, (char *)pMakeData, (char *)pMakeData+cchDataLength, 0);
			*szTmp = '\0';
			}
			break;
		case DBTYPE_BSTR:
			{
			(*(BSTR *)pMakeData)[cchDataLength-3] = L'\0';
			SysReAllocString((BSTR *)pMakeData, *(BSTR *)pMakeData);
			break;
			}
		default:
			odtLog << L"Unexpected Type for BEGINSWITH: " << wColType << ENDL;
			return FALSE;
		}
		break;
	case DBCOMPAREOPS_CONTAINS:
		switch ( eSubCompare )
		{
		case SUBOP_CONTAINS_BEGIN:
			// Reduces to previous case 
			switch ( wColType )
			{
			case DBTYPE_WSTR:
				((WCHAR *)pMakeData)[cchDataLength-3] = L'\0';
				break;
			case DBTYPE_STR:
				{
				char *szTmp = CharPrevExA(CP_ACP, (char *)pMakeData, (char *)pMakeData+cchDataLength, 0);
				*szTmp = '\0';
				}
				break;
			case DBTYPE_BSTR:
				{
				(*(BSTR *)pMakeData)[cchDataLength-3] = L'\0';
				SysReAllocString((BSTR *)pMakeData, *(BSTR *)pMakeData);
				break;
				}
			default:
				odtLog << L"Unexpected Type for CONTAINS: " << wColType << ENDL;
				return FALSE;
			}
			break;
		case SUBOP_CONTAINS_MIDDLE:
		case SUBOP_CONTAINS_END:
		{
			const int cchTrim = 2; // characters to trim
			BYTE *pTmpBuf = (BYTE *)PROVIDER_ALLOC( (cchDataLength+1) * sizeof(WCHAR) );

			switch ( wColType )
			{
			case DBTYPE_WSTR:
				memcpy(pTmpBuf, (BYTE *)pMakeData+(cchTrim*sizeof(WCHAR)), (cchDataLength-cchTrim)*sizeof(WCHAR));
				memcpy(pMakeData, pTmpBuf, (cchDataLength-cchTrim)*sizeof(WCHAR));
				if ( eSubCompare == SUBOP_CONTAINS_END )
					((WCHAR *)pMakeData)[cchDataLength-cchTrim] = L'\0';
				else 
					((WCHAR *)pMakeData)[cchDataLength-(2*cchTrim)] = L'\0';
				break;
			case DBTYPE_STR:
				{
				char *szTmp = (char *)pMakeData;

				szTmp = CharNextA((char *)pMakeData);
				szTmp = CharNextA(szTmp);

				ULONG_PTR cbTrim = szTmp - (char *)pMakeData ;

				memcpy(pTmpBuf, (BYTE *)szTmp, cchDataLength-cbTrim);
				memcpy(pMakeData, pTmpBuf, cchDataLength-cbTrim);
				((char *)pMakeData)[cchDataLength-cbTrim] = '\0';
				
				if ( eSubCompare == SUBOP_CONTAINS_END )
					{
					((char *)pMakeData)[cchDataLength-cbTrim] = '\0';
					}
				else 
					{
					((char *)pMakeData)[cchDataLength-cbTrim] = '\0';
					char *szTmp = CharPrevExA(CP_ACP, (char *)pMakeData, (char *)pMakeData+(cchDataLength-cbTrim), 0);
					*szTmp = '\0';
					}
				}
				
				break;
			case DBTYPE_BSTR:
				{
				WCHAR *	wsz = *(BSTR *)pMakeData;
				
				memcpy(pTmpBuf, (BYTE *)wsz+(cchTrim*sizeof(WCHAR)), (cchDataLength-cchTrim)*sizeof(WCHAR));
				memcpy(wsz, pTmpBuf, (cchDataLength-cchTrim)*sizeof(WCHAR));
		
				if ( eSubCompare == SUBOP_CONTAINS_END )
					((WCHAR *)wsz)[cchDataLength-cchTrim] = L'\0';
				else 
					((WCHAR *)wsz)[cchDataLength-(2*cchTrim)] = L'\0';

				SysReAllocString((BSTR *)pMakeData, *(BSTR *)pMakeData);
				break;
				}
			default:
				odtLog << L"Unexpected Type for CONTAINS: " << wColType << ENDL;
				return FALSE;
			}
			PROVIDER_FREE(pTmpBuf);
			break;
		
		}
		default:
			ASSERT(!"Illegal Sub Compare Op");
		}
		break;
	case DBCOMPAREOPS_GT:
	case DBCOMPAREOPS_GE:
		// truncate a character.
		switch ( wColType )
		{
		case DBTYPE_WSTR:
			cbDataLength -= sizeof(WCHAR);
			((WCHAR *)pMakeData)[cchDataLength-1] = L'\0';
			break;
		case DBTYPE_STR:
			{
			char *szTmp = CharPrevExA(CP_ACP, (char *)pMakeData, (char *)pMakeData+cchDataLength, 0);
			cbDataLength = (szTmp-(char *)pMakeData)+sizeof(char);
			*szTmp = '\0';			
			break;
			}
		case DBTYPE_BSTR:
			{
			(*(BSTR *)pMakeData)[cchDataLength-1] = L'\0';
			break;
			}
		default:
			odtLog << L"Unexpected Type for GT/GE: " << wColType << ENDL;
			return FALSE;
		}
		*pcbDataLength = cbDataLength;
		break;
	case DBCOMPAREOPS_LT:
	case DBCOMPAREOPS_LE:
	{
		// add a character 'a' 
		void * pReAllocData = NULL;

		switch ( wColType )
		{
		case DBTYPE_WSTR:
			cbDataLength += sizeof(WCHAR);
			pReAllocData = PROVIDER_ALLOC(cbDataLength);
			wcscpy((WCHAR *)pReAllocData, (WCHAR *)pMakeData);
			((WCHAR *)pReAllocData)[cchDataLength] = L'a';
			((WCHAR *)pReAllocData)[cchDataLength+1] = L'\0';
			PROVIDER_FREE(*ppMakeData);
			*ppMakeData = pReAllocData;
			break;
		case DBTYPE_STR:
			cbDataLength += sizeof(char);
			pReAllocData = PROVIDER_ALLOC(cbDataLength);
			strcpy((char *)pReAllocData, (char *)pMakeData);
			((char *)pReAllocData)[cchDataLength] = 'a';
			((char *)pReAllocData)[cchDataLength+1] = '\0';
			PROVIDER_FREE(*ppMakeData);
			*ppMakeData = pReAllocData;
			break;
		case DBTYPE_BSTR:
			if( (cchDataLength+1) > ULONG_MAX )
				return FALSE;
			pReAllocData = SysAllocStringLen(NULL, (ULONG)(cchDataLength+1));
			wcscpy((WCHAR *)pReAllocData, *(BSTR *)pMakeData);
			((WCHAR *)pReAllocData)[cchDataLength] = L'a';
			((WCHAR *)pReAllocData)[cchDataLength+1] = L'\0';
			SysFreeString(*(BSTR *)pMakeData);
			*(BSTR *)pMakeData = (BSTR)pReAllocData;
			break;
		default:
			odtLog << L"Unexpected Type for LT/LE: " << wColType << ENDL;
			return FALSE;
		}
		
		*pcbDataLength = cbDataLength;
	}	
		break;
	default:
		ASSERT(!"Illegal Compare Op");
		break;
	}

	switch ( wColType )
	{
		case DBTYPE_WSTR:
			*pcbDataLength=(wcslen((WCHAR*)(*ppMakeData))+1)*sizeof(WCHAR);
			break;
		case DBTYPE_STR:
			*pcbDataLength=(strlen((char*)(*ppMakeData))+1)*sizeof(char);
			break;
	}

	return TRUE;
}


BOOL TCIRowsetFind::AlterNumericData
(
	BYTE **		ppMakeData,
	DBCOMPAREOP CompareOp,
	DBTYPE		wColType,
	DBLENGTH *	pcbDataLength
)
{
	// Find value to increment, decrement by.
	// If CompareOps = GT or GE, we set the find value to the backend value decremented so that 
	// the match will pick up the backend value
	double	dblChangeVal;
	BYTE *	pMakeData = *ppMakeData;

	switch ( wColType )
	{
		case DBTYPE_I1:
		case DBTYPE_I2:
		case DBTYPE_I4:
		case DBTYPE_I8:
		case DBTYPE_UI1:
		case DBTYPE_UI2:
		case DBTYPE_UI4:
		case DBTYPE_UI8:
		case DBTYPE_CY:
		case DBTYPE_NUMERIC:
		case DBTYPE_DECIMAL:
		case DBTYPE_ERROR:
			if ( CompareOp == DBCOMPAREOPS_GT || CompareOp == DBCOMPAREOPS_GE )
				dblChangeVal = -1;
			else if ( CompareOp == DBCOMPAREOPS_LT || CompareOp == DBCOMPAREOPS_LE )
				dblChangeVal = 1;
			break;
		case DBTYPE_R4:
			if ( CompareOp == DBCOMPAREOPS_GT || CompareOp == DBCOMPAREOPS_GE )
				dblChangeVal = -0.5;
			else if ( CompareOp == DBCOMPAREOPS_LT || CompareOp == DBCOMPAREOPS_LE )
				dblChangeVal = 0.5;
			break;
		case DBTYPE_R8:
			if ( CompareOp == DBCOMPAREOPS_GT || CompareOp == DBCOMPAREOPS_GE )
				dblChangeVal = -0.1;
			else if ( CompareOp == DBCOMPAREOPS_LT || CompareOp == DBCOMPAREOPS_LE )
				dblChangeVal = 0.1;
			break;
		case DBTYPE_VARNUMERIC:
			break; // nothing to do - the test has only special cases for varnumeric
		case DBTYPE_DATE:
		case DBTYPE_DBTIME:
		case DBTYPE_DBTIMESTAMP:
			if ( CompareOp == DBCOMPAREOPS_GT || CompareOp == DBCOMPAREOPS_GE )
				dblChangeVal = -1.0/1440.0;  // one minute in OLE Date format
			else if ( CompareOp == DBCOMPAREOPS_LT || CompareOp == DBCOMPAREOPS_LE )
				dblChangeVal = 1.0/1440.0;
			break;
		case DBTYPE_DBDATE:
			if ( CompareOp == DBCOMPAREOPS_GT || CompareOp == DBCOMPAREOPS_GE )
				dblChangeVal = -1.0;  // one day in OLE Date format
			else if ( CompareOp == DBCOMPAREOPS_LT || CompareOp == DBCOMPAREOPS_LE )
				dblChangeVal = 1.0;
			break;
		default:
			ASSERT(!"Shouldn't get here");
	}

	double dblValue = dblChangeVal;

	// Perform data change
	switch ( wColType )
	{
		case DBTYPE_I1:
			*(signed char *)pMakeData += (signed char)(dblValue);
			break;
		case DBTYPE_I2:
			*(short *)pMakeData += short(dblValue);
			break;
		case DBTYPE_I4:
			*(long *)pMakeData += long(dblValue);
			break;
		case DBTYPE_I8:
			*(__int64 *)pMakeData += (__int64)(dblValue);
			break;
		case DBTYPE_UI1:
			*(unsigned char *)pMakeData += (char)(dblValue);
			break;
		case DBTYPE_UI2:
			*(unsigned short *)pMakeData += (short)(dblValue);
			break;
		case DBTYPE_UI4:
			*(unsigned long *)pMakeData += (long)(dblValue);
			break;
		case DBTYPE_UI8:
			*(__int64 *)pMakeData += (__int64)(dblValue);
			break;
		case DBTYPE_CY:
			*(__int64 *)pMakeData += (__int64)(dblValue);
			break;
		case DBTYPE_NUMERIC:
			AddShortToNumeric((DB_NUMERIC *)pMakeData, short(dblValue));
			break;
		case DBTYPE_DECIMAL:
			AddShortToDecimal((DECIMAL *)pMakeData, short(dblValue));
			break;
		case DBTYPE_ERROR:
			*(long *)pMakeData += long(dblValue);
			break;
		case DBTYPE_R4:
			*(float *)pMakeData += float(dblValue);
			break;
		case DBTYPE_R8:
			*(double *)pMakeData += double(dblValue);
			break;
		case DBTYPE_DATE:
			*(double *)pMakeData += double(dblValue);
			break;
		case DBTYPE_VARNUMERIC:
			if(!AlterVarnumericData(ppMakeData, CompareOp, pcbDataLength))
				return FALSE;
			break;
		case DBTYPE_DBTIME:
			{
			LONG lSeconds;

			DBTIME *time = (DBTIME *)pMakeData;

			lSeconds = time->hour * 3600 + time->minute * 60 + time->second + LONG(dblValue);

			ASSERT(lSeconds < 86400 );
			time->hour = (USHORT)(lSeconds / 3600);
			time->minute = (USHORT)((lSeconds % 3600) / 60);
			time->second = (USHORT)(lSeconds % 60);
			break;
			}
		case DBTYPE_DBTIMESTAMP:
			{
			SYSTEMTIME		SystemTime;
			DATE			dOleDate;
	
			SystemTime.wYear = ((DBTIMESTAMP *)pMakeData)->year;
			SystemTime.wMonth = ((DBTIMESTAMP *)pMakeData)->month;
			SystemTime.wDay = ((DBTIMESTAMP *)pMakeData)->day;
			SystemTime.wHour = ((DBTIMESTAMP *)pMakeData)->hour;
			SystemTime.wMinute = ((DBTIMESTAMP *)pMakeData)->minute;
			SystemTime.wSecond = ((DBTIMESTAMP *)pMakeData)->second;

			SystemTimeToVariantTime(&SystemTime, &dOleDate);
			dOleDate += dblValue;	

			VariantTimeToSystemTime(dOleDate, &SystemTime);
			
			((DBTIMESTAMP *)pMakeData)->year = 	SystemTime.wYear;
			((DBTIMESTAMP *)pMakeData)->month = SystemTime.wMonth;
			((DBTIMESTAMP *)pMakeData)->day = SystemTime.wDay;
			((DBTIMESTAMP *)pMakeData)->hour = SystemTime.wHour;
			((DBTIMESTAMP *)pMakeData)->minute = SystemTime.wMinute;
			((DBTIMESTAMP *)pMakeData)->second = SystemTime.wSecond;
			}
			break;
		case DBTYPE_DBDATE:
			{
			SYSTEMTIME	SystemTime;
			DATE		dOleDate;
				
			SystemTime.wYear = ((DBDATE *)pMakeData)->year;
			SystemTime.wMonth = ((DBDATE *)pMakeData)->month;
			SystemTime.wDay = ((DBDATE *)pMakeData)->day;

			SystemTimeToVariantTime(&SystemTime, &dOleDate);
			dOleDate += dblValue;	
		
			VariantTimeToSystemTime(dOleDate, &SystemTime);
			
			((DBDATE *)pMakeData)->year = SystemTime.wYear;
			((DBDATE *)pMakeData)->month = SystemTime.wMonth;
			((DBDATE *)pMakeData)->day = SystemTime.wDay;
			}
			break;
		default:
			ASSERT(!"Shouldn't get here");
	}

	return TRUE;
}

BOOL TCIRowsetFind::AlterVarnumericData
(
	BYTE **		ppMakeData, 
	DBCOMPAREOP CompareOp,
	DBLENGTH *	pcbDataLength
)
{
	// 3 "Equal" options
	// a) s_VarNumEqOption=0 => leave the varnum alone
	// b) s_VarNumEqOption=1 => inc. scale, mult by 10
	// c) s_VarNumEqOption=2 => dec. scale, div by 10
	static long		s_VarNumEqOption = 0;
	static SBYTE	s_MaxScale = 127; // max possible VarNum scale
	static SBYTE	s_MinScale = -128;// min possible VarNum scale

	BYTE			bRemainder=0;
	DB_VARNUMERIC *	pVarNum = (DB_VARNUMERIC *)*ppMakeData;
	DB_VARNUMERIC *	pNewVarNum = NULL;
	ULONG			cbVarNumeric;

	ASSERT(ppMakeData && *ppMakeData && pcbDataLength);
	ASSERT(*pcbDataLength <= MAX_VARNUM_BYTE_SIZE);  // max varnumeric byte length is 111.

	cbVarNumeric = (ULONG) *pcbDataLength;

	if (pVarNum->sign==0)  // if negative
	{
		if (CompareOp==DBCOMPAREOPS_GT || CompareOp==DBCOMPAREOPS_GE)
			CompareOp=DBCOMPAREOPS_LT;
		else if (CompareOp==DBCOMPAREOPS_LT || CompareOp==DBCOMPAREOPS_LE)
			CompareOp=DBCOMPAREOPS_GT;
	}

	switch (CompareOp)
	{
	case DBCOMPAREOPS_EQ:
		switch (s_VarNumEqOption)
		{
		case 0:
			break;
		case 1:
			{
			DB_VARNUMERIC	*pNewVarNum=NULL;

			if(pVarNum->scale==s_MaxScale || pVarNum->precision==255)
				return FALSE;
			
			pNewVarNum = (DB_VARNUMERIC *)PROVIDER_ALLOC(cbVarNumeric+1);
			if (!pNewVarNum)
				return FALSE;

			memset(pNewVarNum, 0, cbVarNumeric+1);
			memcpy(pNewVarNum, pVarNum, cbVarNumeric);
			AddCharToVarNumericVal(L'0',pNewVarNum, USHORT(cbVarNumeric+1)-sizeof(DB_VARNUMERIC)+1);
			
			pNewVarNum->precision++;  // inc to compensate for new digit
			pNewVarNum->scale++; // inc to the scale to compensate for mult by 10

			PROVIDER_FREE(pVarNum);
			*ppMakeData = (BYTE *)pNewVarNum;
			(*pcbDataLength)++;
			}
			break;
		case 2:
			{
			DB_VARNUMERIC	*pTempVar=NULL;

			pTempVar = (DB_VARNUMERIC *)PROVIDER_ALLOC(cbVarNumeric);
			if (!pTempVar)
				return FALSE;

			memcpy(pTempVar, pVarNum, cbVarNumeric);

			if (pVarNum->scale==s_MinScale)
				return FALSE;
			
			VarNumericDiv10Rem(pTempVar, pTempVar, (cbVarNumeric)-sizeof(DB_VARNUMERIC)+1, &bRemainder);
			if(bRemainder!=0)
				return FALSE; // not able to guarantee unchanged value

			// this can work, so copy the contents
			memcpy(pVarNum, pTempVar, cbVarNumeric);
			PROVIDER_FREE(pTempVar);
			
			pVarNum->precision--;	// decrement the precision to compensate for div by 10
			break;
			}
		default:
			ASSERT(!"Bad Eq option");
			break;
		}
		s_VarNumEqOption = (s_VarNumEqOption+1)%3;
		break;
	case DBCOMPAREOPS_GT:
	case DBCOMPAREOPS_GE:
		// make value smaller.
		if (pVarNum->scale==s_MaxScale)
		{
			VarNumericDiv10Rem(pVarNum, pVarNum, cbVarNumeric, &bRemainder);
			pVarNum->precision--;	// decrement the precision to compensate for div by 10

			// it's possible that the value remains unchanged.
			// force it to be less
			if (bRemainder==0)
				pVarNum->scale++;	// incrementing the scale is safe even though pVarNum->scale == s_MaxScale
		}							// because the scale is decremented by the call to VarNumericDiv10Rem.
		else
			pVarNum->scale++; // just increment the scale

		break;
	case DBCOMPAREOPS_LT:
	case DBCOMPAREOPS_LE:
		// make value larger
		if (pVarNum->scale==s_MinScale)
		{
			// Add a digit to the number as long as it doesn't overflow
			// a varnum's max precision = 255
			if (pVarNum->precision==255)
				return FALSE;

			pNewVarNum = (DB_VARNUMERIC *)PROVIDER_ALLOC(cbVarNumeric+1);
			if (!pNewVarNum)
				return FALSE;

			memset(pNewVarNum, 0, cbVarNumeric+1);
			memcpy(pNewVarNum, pVarNum, cbVarNumeric);
			AddCharToVarNumericVal(L'1', pNewVarNum, USHORT(cbVarNumeric)-sizeof(DB_VARNUMERIC)+1);
			PROVIDER_FREE(pVarNum);

			pNewVarNum->precision++;  // inc to compensate for new digit

			*ppMakeData = (BYTE *)pNewVarNum;
			(*pcbDataLength)++;
		}
		else
			pVarNum->scale--; // just decrement the scale
		break;
	default:
		ASSERT(!"Bad compare option to AlterVarnumericData");
	}

	return TRUE;
}


BOOL TCIRowsetFind::BindingTypeTest
(
	CTable *	pCTable,
	DBTYPE		wBindingType	
)
{
	HRESULT hr = E_FAIL;
	BOOL		fTestPass = TEST_SKIPPED;
	DBPROPID	guidProperty[1];
	HROW *		phrow = NULL;
	DBCOUNTITEM	cRowsObtained;
	DBORDINAL	ulColToFind;
	DBTYPE		wColType = DBTYPE_EMPTY;
	CCol		TempCol;
	DBORDINAL	ulColNum = pCTable->CountColumnsOnTable();

	TESTC_DRIVER(g_rgDBPrpt[IDX_IRowsetLocate].fSupported);

	guidProperty[0] = DBPROP_IRowsetLocate;

	//open rowset, and accessor.  
	TESTC(GetRowsetAndAccessor(g_pCTable, SELECT_ALLFROMTBL, IID_IRowsetFind,
		1,guidProperty,DBACCESSOR_ROWDATA, DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
		ALL_COLS_BOUND, FORWARD, NO_COLS_BY_REF, NULL, EXECUTE_IFNOERROR, DBTYPE_EMPTY, 
		TRUE));

	TESTC(GetVariableLengthStrAndUpdatable(&ulColToFind, DBCOMPAREOPS_EQ, FALSE, &wColType));
	TESTC_(m_pIConvertType->CanConvert(wBindingType, wColType, DBCONVERTFLAGS_COLUMN), S_OK);

	for(DBORDINAL ulColIndex = 1; ulColIndex <= ulColNum; ulColIndex++)
	{			
		m_pIRowset->RestartPosition(DB_NULL_HCHAPTER);
		pCTable->GetColInfo(ulColIndex, TempCol);
		wColType = TempCol.GetProviderType();
		TEST2C_(hr=m_pIConvertType->CanConvert(wBindingType, wColType, DBCONVERTFLAGS_COLUMN), S_OK, S_FALSE);
		if (hr==S_FALSE)
			continue;
	

		TESTC(fTestPass = CreateFindValueAccessor(DBCOMPAREOPS_EQ, g_pCTable, 1, ulColToFind, wBindingType, SUBOP_EMPTY, false, NULL));
		fTestPass = TEST_FAIL;
		m_hr = m_pIRowsetFind->FindNextRow(DB_NULL_HCHAPTER, m_hRowsetFindAccessor, 
													m_pFindValue, DBCOMPAREOPS_EQ, 0, NULL, 0, 
													1, &cRowsObtained, &phrow);
		if (m_hr != S_OK)
		{
			odtLog<<L"Error at ColIndex "<<ulColToFind<<L" Binding type = "<<wBindingType<<L"; column type = "<<wColType<< ENDL;
			odtLog<<L"-------------------------------------------------------------"<< ENDL;
		}
		TESTC_(m_hr, S_OK);
		TESTC(cRowsObtained==1);
		TESTC(VerifyRowPosition(phrow[0], 1, g_pCTable));
		
		if ( cRowsObtained > 0 && phrow)
		{
			m_pIRowset->ReleaseRows(cRowsObtained, phrow, NULL, NULL, NULL);
			cRowsObtained = 0;
			if (phrow)
				PROVIDER_FREE(phrow);
		}
	}
	fTestPass = TEST_PASS;

CLEANUP:
	if ( cRowsObtained > 0 && phrow)
	{
		m_pIRowset->ReleaseRows(cRowsObtained, phrow, NULL, NULL, NULL);
		cRowsObtained = 0;
		if (phrow)
			PROVIDER_FREE(phrow);
	}
	ReleaseFindValueAccessor(wBindingType);
	ReleaseRowsetAndAccessor();
	return fTestPass;
}


BOOL TCIRowsetFind::CompareOpTest
(
	CTable *		pCTable,
	DBCOMPAREOP		CompareOp,
	eSUBCOMPAREOP	eSubCompare,
	BOOL			fUseStream
)
{
	if(!pCTable || !m_pIRowset)
	{
		odtLog << L"ASSERT (pCTable && m_pIRowset)" <<ENDL;
		return TEST_FAIL;
	}

	// Service components do not support conversion to streams
	if (fUseStream && ((GetModInfo()->UseServiceComponents() & SERVICECOMP_INVOKE) == SERVICECOMP_INVOKE))	
	{
		return TEST_SKIPPED;
	}
	

	BOOL		fTestPass = TEST_PASS, fSubTestPass = TEST_FAIL;
	CCol		TempCol;
	DBORDINAL	ulColNum = pCTable->CountColumnsOnTable();
	DBCOUNTITEM	ulRowCount = pCTable->GetRowsOnCTable();

	//CLEANUP from previous variations: need this for RestartPosition to succeed
	if( m_rghRowsFound && m_cRowsFound >0 && m_rghRowsFound)
	{
		m_pIRowset->ReleaseRows(m_cRowsFound, m_rghRowsFound, NULL, NULL, NULL);
		PROVIDER_FREE(m_rghRowsFound);
		m_cRowsFound = 0;
	}	

	for(DBORDINAL ulColIndex = 1; ulColIndex <= ulColNum; ulColIndex++)
	{			
		m_pIRowset->RestartPosition(DB_NULL_HCHAPTER);
		g_pCTable->GetColInfo(ulColIndex, TempCol);
	
		// Make Sure it's updateable and searchable, otherwise it may be an
		// autoinc column that we don't know the value of.
		if (!IsColumnMinimumFindable(&TempCol, CompareOp) || (fUseStream && (!TempCol.GetIsLong() ||
										(TempCol.GetProviderType() != DBTYPE_STR && 
										TempCol.GetProviderType() != DBTYPE_WSTR && 
										TempCol.GetProviderType() != DBTYPE_BYTES))))
			continue;		

			for ( ULONG ulRowIndex = 1; ulRowIndex <= ulRowCount+1; ulRowIndex++ )
			{
				HRESULT hrExp1  = ulRowIndex <= ulRowCount ? S_OK : DB_S_ENDOFROWSET;
				ULONG   ulRow   = ulRowIndex <= ulRowCount ? 1 : 0;
				BOOL    bVerify = TRUE;

				WCHAR wszName[256];
				wcscpy(wszName, TempCol.GetProviderTypeName());

				// Expect DB_E_BADCOMPAREOP when the data type is "image" and the compare operator is not DBCOMPAREOPS_IGNORE
				// Basically this is the only workaround not related to DBCOMPAREOPS_IGNORE. Everything else is.
				if( wcscmp(wszName, L"image") == 0 && CompareOp != DBCOMPAREOPS_IGNORE )
					hrExp1 = DB_E_BADCOMPAREOP;

				// Dont bother to verify data of returned rows when compare operator is DBCOMPAREOPS_IGNORE. Too many errors on specific
				// row / column combinations. Trying to make a table and check pass / fail for each would be difficult. So were just skipping.
				// Note all other compare operators are working fine. 
				if( CompareOp == DBCOMPAREOPS_IGNORE )
					bVerify = FALSE;

				// It was explained that the data in the tables is generated with NULLs in the spots where the row / column numbers
				// intersect. IE r1c1, r2c2, etc. It would appear to be an issue with NULL data and DBCOMPAREOPS_IGNORE. Does not
				// which data type is used in the "pFindValue" field, or whether we use SQLOLEDB or MSDASQL/SQL. Its always the
				// case where ColumnIndex = RowIndex. 
				// NOTE There are 3 specific exceptions below.
				if( CompareOp == DBCOMPAREOPS_IGNORE && ulColIndex == ulRowIndex )
					hrExp1 = E_INVALIDARG;
			 
				HRESULT hrExp2 = hrExp1;

			 fSubTestPass = 
				CallFindNextRows
					(	
						pCTable,				// CTable pointer
						NULL,					// bookmark;
						0,						// Length of bookmark
						1,						// # rows to fetch
						0,						// offset
						ulColIndex,				// Which column to match
						ulRowIndex,				// row to match
						hrExp1,					// HRESULT to verify
						ulRow,					// How many rows to expect.
						TRUE,						// Release Rows
						CompareOp,				// Specifically ask for a compare Op
						eSubCompare,			// Sub Comparision option
						NULL,					// Use client or provider memory, default=provider
						bVerify,				// verify rows by comparing data ?
						fUseStream,				// use fstream
						hrExp2					// HRESULT for actual operation
					);		
				

				if ( fSubTestPass == TEST_FAIL )
				{
					fTestPass = TEST_FAIL;
					odtLog<<"Error at ColName "<< TempCol.GetColName()<<L", ColIndex "<<ulColIndex<<" and at RowIndex "<<ulRowIndex<< ENDL;
					odtLog<<"-------------------------------------------------------------"<< ENDL;
					break;
				}
		}	
	}

	return fTestPass;
}	



BOOL TCIRowsetFind::DeleteRow
(
	CTable *	pCTable,
	DBCOUNTITEM	ulRowToDelete
)
{	
	ASSERT(pCTable && ulRowToDelete >0 && ULONG(g_lRowLast) >= ulRowToDelete);

	DBCOUNTITEM		cRowsObtained = 0;
	HROW *			pHRow = NULL;	
	IRowsetChange *	pIRowsetChange = NULL;

	//get the row handle for row to delete
	TESTC_(m_pIRowset->GetNextRows(NULL, ulRowToDelete-1,1,&cRowsObtained,&pHRow),S_OK);
	COMPARE(cRowsObtained, 1);

	//QI for IRowsetChange pointer
	TESTC_(m_pIRowsetFind->QueryInterface(IID_IRowsetChange,
			(void **)&pIRowsetChange),S_OK);
	//delete the row
	TESTC_(pIRowsetChange->DeleteRows(NULL,1,pHRow,NULL),S_OK);

CLEANUP:

	SAFE_RELEASE(pIRowsetChange);
	if(pHRow)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK);			
		PROVIDER_FREE(pHRow);
	}
	//restart the cursor position
	if(!CHECK(m_pIRowset->RestartPosition(NULL),S_OK))
		return FALSE;

	return TRUE;
}


DWORD TCIRowsetFind::TC_FindCompareOps
(
	DBID * pColDBID
)
{
	return GetFindCompareOps(m_pIRowset, pColDBID);
}


//--------------------------------------------------------------------
//
//@mfunc: verify that updating / deleting rows is OK against this provider.
//
//	The result of this particular function is independant of whether the provider
// allows updates/deletes.  The focus is rather whether deleting a row will
// cause the table to fall into an "unknown" state.
//
//--------------------------------------------------------------------
BOOL TCIRowsetFind::AlteringRowsIsOK()
{
	// If a specific table was set on the backend, assume we cannot alter it
	// unless we have a file telling us how to regenerate the table.
	if ( GetModInfo()->GetTableName() && !GetModInfo()->GetFileName() )
		return FALSE;
	else
		return TRUE;
}


BOOL TCIRowsetFind::IsColumnMinimumFindable(CCol *pCol, DBCOMPAREOP CompareOp)
{
	DBTYPE wTargetType;

	if(!ValidateCompareOp ( TC_FindCompareOps(pCol->GetColID()), CompareOp ))
		return FALSE;

	CompareOp &= ~( DBCOMPAREOPS_CASESENSITIVE | DBCOMPAREOPS_CASEINSENSITIVE );

	if ( !pCol->GetUpdateable())
		return FALSE;  // autoinc column we don't know the value of


	wTargetType = pCol->GetProviderType();

	if(wTargetType == DBTYPE_VARIANT)
		return FALSE;

	return IsTypeFindable(wTargetType, CompareOp);
}

BOOL TCIRowsetFind::IsTypeFindable(DBTYPE wType, DBCOMPAREOP CompareOp, DBTYPE wSubType)
{

	CompareOp &= ~( DBCOMPAREOPS_CASESENSITIVE | DBCOMPAREOPS_CASEINSENSITIVE );

	if ( wType == DBTYPE_EMPTY || wType == DBTYPE_NULL || wType == DBTYPE_UDT)
		return FALSE;
	else if(wType == DBTYPE_VARIANT)
		return FALSE;

	if (IsStringCompareOp(CompareOp) && !IsStringType(wType))
		return FALSE;

	if ((wType == DBTYPE_BOOL ||
		wType == DBTYPE_ERROR ||
		wType == DBTYPE_GUID ||
		wType == DBTYPE_BYTES ) &&
		(CompareOp == DBCOMPAREOPS_GE ||
		CompareOp == DBCOMPAREOPS_GT ||
		CompareOp == DBCOMPAREOPS_LE ||
		CompareOp == DBCOMPAREOPS_LT ))
		return FALSE;	

	return TRUE;
}


BOOL TCIRowsetFind::IsStringType(DBTYPE wType)
{
	return (wType == DBTYPE_STR || wType == DBTYPE_WSTR || wType == DBTYPE_BSTR);
}


BOOL TCIRowsetFind::IsStringCompareOp(DBCOMPAREOP CompareOp)
{
	CompareOp &= ~( DBCOMPAREOPS_CASESENSITIVE | DBCOMPAREOPS_CASEINSENSITIVE );

	return (CompareOp == DBCOMPAREOPS_CONTAINS ||
			CompareOp == DBCOMPAREOPS_NOTCONTAINS ||
			CompareOp == DBCOMPAREOPS_BEGINSWITH ||
			CompareOp == DBCOMPAREOPS_NOTBEGINSWITH );
}


BOOL TCIRowsetFind::IsColumnFindable
(
	CCol *		pCol,
	DBCOMPAREOP CompareOp
)
{
	CompareOp &= ~( DBCOMPAREOPS_CASESENSITIVE | DBCOMPAREOPS_CASEINSENSITIVE );

	// If the column is autoinc then we don't know the row values
	if(!pCol->GetUpdateable())
		return FALSE;  

	// Some types like DBTYPE_BOOL, DBTYPE_NULL can't guarantee a uniqueness property that the
	// test needs.
	if(!IsDBTYPEFindable(pCol->GetProviderType()))
		return FALSE;

	if(pCol->GetProviderType() == DBTYPE_VARIANT)
	{
		if(!IsDBTYPEFindable(pCol->GetSubType()))
			return FALSE;

		if(	(CompareOp == DBCOMPAREOPS_CONTAINS || CompareOp == DBCOMPAREOPS_BEGINSWITH ) &&
			pCol->GetSubType() != DBTYPE_BSTR)
			return FALSE;
		else
			return ValidateCompareOp(TC_FindCompareOps(pCol->GetColID()), CompareOp);  
	}

	return ValidateCompareOp (TC_FindCompareOps(pCol->GetColID()), CompareOp);
}


HRESULT TCIRowsetFind::RestartRowPosition()
{
	HRESULT hr = m_pIRowset->RestartPosition(NULL);

	if ( hr == S_OK || hr == DB_S_COMMANDREEXECUTED )
		return S_OK;
	else
		return E_FAIL;
}


//--------------------------------------------------------------------
//
//@mfunc: Get the Variable Length and Updatable column
//
//--------------------------------------------------------------------
BOOL	TCIRowsetFind::GetVariableLengthStrAndUpdatable
(
	DBORDINAL *	pulColNum,
	DBCOMPAREOP CompareOp,
	BOOL		fGetLongStr,
	DBTYPE *	pwType
)
{	
	DBCOUNTITEM cColsCount;
	DBCOUNTITEM cColsInTable = 0;
	CCol		TempCol;

	//make sure a Rowset has been opened on the table
	if(!m_pTable)
		return FALSE;

	//initialization
	*pulColNum=0;
	cColsInTable = m_pTable->CountColumnsOnTable();
	
	for(cColsCount=1;cColsCount<=cColsInTable;cColsCount++)
	{
		DBTYPE TargetType = DBTYPE_EMPTY;

		m_pTable->GetColInfo(cColsCount, TempCol);

		if ( TempCol.GetProviderType() == DBTYPE_VARIANT )
			TargetType = TempCol.GetSubType();		
		else		
			TargetType = TempCol.GetProviderType();
		
		if(	( DBTYPE_STR == TargetType || DBTYPE_WSTR == TargetType || DBTYPE_BSTR == TargetType ) 
		   &&
			(TempCol.GetUpdateable())
		   &&
			(IsColumnMinimumFindable(&TempCol, CompareOp))
		   &&
			(fGetLongStr == TempCol.GetIsLong())
		  )
		{
			(*pulColNum)=TempCol.GetColNum();
			if (pwType)
				(*pwType)=TempCol.GetProviderType();
			break;
		}
	}
	
	return (*pulColNum != 0 );
}


//--------------------------------------------------------------------
//
//@mfunc: Get first Non-Nullable column
//
//--------------------------------------------------------------------
BOOL	TCIRowsetFind::GetNonNullableCol
(
	DBORDINAL *	pulColNum,
	DBCOMPAREOP CompareOp,
	BOOL		fGetLongStr,
	DBTYPE *	pwType
)
{	
	DBORDINAL	cColsCount;
	DBORDINAL	cColsInTable = 0;
	CCol		TempCol;

	//make sure a Rowset has been opened on the table
	if(!m_pTable)
		return FALSE;

	//initialization
	*pulColNum=0;
	cColsInTable = m_pTable->CountColumnsOnTable();
	
	for(cColsCount=1;cColsCount<=cColsInTable;cColsCount++)
	{
		DBTYPE TargetType = DBTYPE_EMPTY;

		m_pTable->GetColInfo(cColsCount, TempCol);

		if (!TempCol.GetNullable() && IsColumnMinimumFindable(&TempCol, CompareOp))
		{
			(*pulColNum)=TempCol.GetColNum();
			if (pwType)
				(*pwType)=TempCol.GetProviderType();
			break;
		}
	}
	
	return (*pulColNum != 0 );
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Test Case Section
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


// {{ TCW_TEST_CASE_MAP(No_Properties)
//--------------------------------------------------------------------
// @class no properties set
//
class No_Properties : public TCIRowsetFind { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(No_Properties,TCIRowsetFind);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember pBookmark=NULL,cRows=1,fSkip=1. Traverse rowset by matching current row until DB_S_ENDOF_ROWSET
	int Variation_1();
	// @cmember pBookmark=NULL, cRows=1, match middle.  Again, pBookmark=NULL, cRows=1 and Verify DB_S_ENDOFROWSET
	int Variation_2();
	// @cmember pBookmark=NULL, cRows=1, and match 2nd.  Again pBookmark=NULL, cRows=3, match 4th. S_OK and 3 row handles
	int Variation_3();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(No_Properties)
#define THE_CLASS No_Properties
BEG_TEST_CASE(No_Properties, TCIRowsetFind, L"no properties set")
	TEST_VARIATION(1, 		L"pBookmark=NULL,cRows=1,fSkip=1. Traverse rowset by matching current row until DB_S_ENDOF_ROWSET")
	TEST_VARIATION(2, 		L"pBookmark=NULL, cRows=1, match middle.  Again, pBookmark=NULL, cRows=1 and Verify DB_S_ENDOFROWSET")
	TEST_VARIATION(3, 		L"pBookmark=NULL, cRows=1, and match 2nd.  Again pBookmark=NULL, cRows=3, match 4th. S_OK and 3 row handles")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(FetchBackwards)
//--------------------------------------------------------------------
// @class Test FetchBackwards property
//
class FetchBackwards : public TCIRowsetFind { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(FetchBackwards,TCIRowsetFind);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember pBookMark=NULL,cRows=1 and match 1st row.  FindNext, cRows=-1 matching same row. Verify S_OK
	int Variation_1();
	// @cmember pBookMark=NULL, cRows=3 and Match 1st row. FindNext, cRows=-2 and match 3rd row.  Verify S_OK and 3rd and 2nd rows
	int Variation_2();
	// @cmember pBookmark=NULL,cRows=1 nd match last row. FindNext wiht cRows=-2,fSkip=0 and match 3rd row.  Verify S_OK and hrows.
	int Variation_3();
	// @cmember pBookMark=NULL,cRows=1 and match last row.  FindNext, cRows=-6, match last row.  Verify DB_S_ENDOFROWSET and hrows
	int Variation_4();
	// @cmember pBookmark=NULL, cRows=1, and match last. pBookmark=NULL, cRows=3. Offset=1 and mattch 2nd row.  Verify DB_S_ENDOFROWSET.
	int Variation_5();
	// @cmember pBookmark=NULL, cRows=1 and match last.  Loop with cRows=-1 until DB_S_ENDOFROWSET.
	int Variation_6();
	// @cmember pBookmark=NULL, Match next to last.  FindNext with cRows=-1 and Offset=2.  Verify DB_S_ENDOFROWSET
	int Variation_7();
	// @cmember Test that cRows=0 doesn't affect find direction
	int Variation_8();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(FetchBackwards)
#define THE_CLASS FetchBackwards
BEG_TEST_CASE(FetchBackwards, TCIRowsetFind, L"Test FetchBackwards property")
	TEST_VARIATION(1, 		L"pBookMark=NULL,cRows=1 and match 1st row.  FindNext, cRows=-1 matching same row. Verify S_OK")
	TEST_VARIATION(2, 		L"pBookMark=NULL, cRows=3 and Match 1st row. FindNext, cRows=-2 and match 3rd row.  Verify S_OK and 3rd and 2nd rows")
	TEST_VARIATION(3, 		L"pBookmark=NULL,cRows=1 nd match last row. FindNext wiht cRows=-2,fSkip=0 and match 3rd row.  Verify S_OK and hrows.")
	TEST_VARIATION(4, 		L"pBookMark=NULL,cRows=1 and match last row.  FindNext, cRows=-6, match last row.  Verify DB_S_ENDOFROWSET and hrows")
	TEST_VARIATION(5, 		L"pBookmark=NULL, cRows=1, and match last. pBookmark=NULL, cRows=3. Offset=1 and mattch 2nd row.  Verify DB_S_ENDOFROWSET.")
	TEST_VARIATION(6, 		L"pBookmark=NULL, cRows=1 and match last.  Loop with cRows=-1 until DB_S_ENDOFROWSET.")
	TEST_VARIATION(7, 		L"pBookmark=NULL, Match next to last.  FindNext with cRows=-1 and Offset=2.  Verify DB_S_ENDOFROWSET")
	TEST_VARIATION(8, 		L"Test that cRows=0 doesn't affect find direction")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(ScrollBackwards)
//--------------------------------------------------------------------
// @class Test ScrollBackwards property
//
class ScrollBackwards : public TCIRowsetFind { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(ScrollBackwards,TCIRowsetFind);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember pBookMark=DBBMK_FIRST,cRows=1 and match 2nd row. Verify S_OK and one row handle
	int Variation_1();
	// @cmember pBookMark=DBBMK_LAST,cRows=1 and Match last row.  Verify S_OK and one row handle.
	int Variation_2();
	// @cmember pBookmark=DBBMK_LAST, cRows=1, and match last.  Verify S_OK
	int Variation_3();
	// @cmember *pBookmark=DBBMK_FIRST, cRows=1, Offset=1.  Verify DB_S_ENDOFROWSET
	int Variation_4();
	// @cmember pBookmark=NULL, cRows=1, and match middle.  FindNext with Offset=-3, cRows=3 and match row preceding middle row.
	int Variation_5();
	// @cmember pBookmark=NULL, cRows=1, and match last.  FindNext with cRows=1, Offset=-1 and match last.  Verify S_OK
	int Variation_6();
	// @cmember pBookmark=NULL, cRows=1, Loffset=-# of rows.  Verify DB_S_ENDOFROWSET
	int Variation_7();
	// @cmember pBookmark=NULL,cRows=1, Offset=-2.  Verify #of rows-1 row fetched.  Cursor is after next to last row
	int Variation_8();
	// @cmember pBookmark=NULL, cRows=1, Loffset=-#of rows+1
	int Variation_9();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(ScrollBackwards)
#define THE_CLASS ScrollBackwards
BEG_TEST_CASE(ScrollBackwards, TCIRowsetFind, L"Test ScrollBackwards property")
	TEST_VARIATION(1, 		L"pBookMark=DBBMK_FIRST,cRows=1 and match 2nd row. Verify S_OK and one row handle")
	TEST_VARIATION(2, 		L"pBookMark=DBBMK_LAST,cRows=1 and Match last row.  Verify S_OK and one row handle.")
	TEST_VARIATION(3, 		L"pBookmark=DBBMK_LAST, cRows=1, and match last.  Verify S_OK")
	TEST_VARIATION(4, 		L"*pBookmark=DBBMK_FIRST, cRows=1, Offset=1.  Verify DB_S_ENDOFROWSET")
	TEST_VARIATION(5, 		L"pBookmark=NULL, cRows=1, and match middle.  FindNext with Offset=-3, cRows=3 and match row preceding middle row.")
	TEST_VARIATION(6, 		L"pBookmark=NULL, cRows=1, and match last.  FindNext with cRows=1, Offset=-1 and match last.  Verify S_OK")
	TEST_VARIATION(7, 		L"pBookmark=NULL, cRows=1, Loffset=-# of rows.  Verify DB_S_ENDOFROWSET")
	TEST_VARIATION(8, 		L"pBookmark=NULL,cRows=1, Offset=-2.  Verify #of rows-1 row fetched.  Cursor is after next to last row")
	TEST_VARIATION(9, 		L"pBookmark=NULL, cRows=1, Loffset=-#of rows+1")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(CanHoldRows)
//--------------------------------------------------------------------
// @class Test DBPROP can hold row property
//
class CanHoldRows : public TCIRowsetFind { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(CanHoldRows,TCIRowsetFind);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Get 1,2 row. Get 2,3,4 rows.  S_OK
	int Variation_1();
	// @cmember Get 1,2 rows. Get 3 row.
	int Variation_2();
	// @cmember Get all rows with GetNextRows.  RestartPosition. FindNext with pBookmark=NULL, Offset=1, cRows=1 and match 2nd. Verify S_OK
	int Variation_3();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(CanHoldRows)
#define THE_CLASS CanHoldRows
BEG_TEST_CASE(CanHoldRows, TCIRowsetFind, L"Test DBPROP can hold row property")
	TEST_VARIATION(1, 		L"Get 1,2 row. Get 2,3,4 rows.  S_OK")
	TEST_VARIATION(2, 		L"Get 1,2 rows. Get 3 row.")
	TEST_VARIATION(3, 		L"Get all rows with GetNextRows.  RestartPosition. FindNext with pBookmark=NULL, Offset=1, cRows=1 and match 2nd. Verify S_OK")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(Scroll_and_Fetch)
//--------------------------------------------------------------------
// @class Test in context of CANSCROLLBACKWARDS and CANFETCHBACKWARDS
//
class Scroll_and_Fetch : public TCIRowsetFind { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Scroll_and_Fetch,TCIRowsetFind);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember pBookmark=DBBMK_FIRST,cRows=-1.  Verify DB_S_ENDOFROWSET and no hrows.
	int Variation_1();
	// @cmember pBookMark=DBBMK_FIRST,cRows=2  Match 3rd row.  Verify S_OK and hrows
	int Variation_2();
	// @cmember pBookMark=DBBMK_LAST,cRows=-2. Match 3rd row.  Verify S_OK and 3rd and 2nd hrows
	int Variation_3();
	// @cmember pBookMark=DBBMK_LAST,cRows=-# rows. Match last row.  Verify DB_S_ENDOFROWSET and all rows
	int Variation_4();
	// @cmember pBookMark=DBBMK_FIRST, cRows=-1. Match first row.  Verify S_OK and one row handle.
	int Variation_5();
	// @cmember pBookMark=DBBMK_FIRST,cRows=1. Match last row.  FindNext, cRows=1, Offset=1 and match last. then cRows=-3, match 4th
	int Variation_6();
	// @cmember pBookmark=DBBMK_LAST,cRows=1.  Match last row.  Call FindNext, cRows=-3, pBookmark=NULL.  Match 4th row, S_OK.
	int Variation_7();
	// @cmember *pBookmark=DBBMK_LAST, Offset=-2, cRows=-3.  Verify row starting with N-4 fetched in traversal order
	int Variation_8();
	// @cmember pBookmark=NULL, Offset=-2, cRows=-1.  Verify N-2 row is fetched and fetch position is after N-3 row
	int Variation_9();
	// @cmember pBookmark=NULL, Offset=-4, cRows=2 and match last. Verify DB_S_END. Again cRows=-1 and no match.  Again cRows=1 and match 1st.
	int Variation_10();
	// @cmember pBookmark=NULL, cRows=3 and match last. DB_S_END.  Again with cRows=2, verify DB_S_END.
	int Variation_11();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(Scroll_and_Fetch)
#define THE_CLASS Scroll_and_Fetch
BEG_TEST_CASE(Scroll_and_Fetch, TCIRowsetFind, L"Test in context of CANSCROLLBACKWARDS and CANFETCHBACKWARDS")
	TEST_VARIATION(1, 		L"pBookmark=DBBMK_FIRST,cRows=-1.  Verify DB_S_ENDOFROWSET and no hrows.")
	TEST_VARIATION(2, 		L"pBookMark=DBBMK_FIRST,cRows=2  Match 3rd row.  Verify S_OK and hrows")
	TEST_VARIATION(3, 		L"pBookMark=DBBMK_LAST,cRows=-2. Match 3rd row.  Verify S_OK and 3rd and 2nd hrows")
	TEST_VARIATION(4, 		L"pBookMark=DBBMK_LAST,cRows=-# rows. Match last row.  Verify DB_S_ENDOFROWSET and all rows")
	TEST_VARIATION(5, 		L"pBookMark=DBBMK_FIRST, cRows=-1. Match first row.  Verify S_OK and one row handle.")
	TEST_VARIATION(6, 		L"pBookMark=DBBMK_FIRST,cRows=1. Match last row.  FindNext, cRows=1, Offset=1 and match last. then cRows=-3, match 4th")
	TEST_VARIATION(7, 		L"pBookmark=DBBMK_LAST,cRows=1.  Match last row.  Call FindNext, cRows=-3, pBookmark=NULL.  Match 4th row, S_OK.")
	TEST_VARIATION(8, 		L"*pBookmark=DBBMK_LAST, Offset=-2, cRows=-3.  Verify row starting with N-4 fetched in traversal order")
	TEST_VARIATION(9, 		L"pBookmark=NULL, Offset=-2, cRows=-1.  Verify N-2 row is fetched and fetch position is after N-3 row")
	TEST_VARIATION(10, 		L"pBookmark=NULL, Offset=-4, cRows=2 and match last. Verify DB_S_END. Again cRows=-1 and no match.  Again cRows=1 and match 1st.")
	TEST_VARIATION(11, 		L"pBookmark=NULL, cRows=3 and match last. DB_S_END.  Again with cRows=2, verify DB_S_END.")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(Boundary)
//--------------------------------------------------------------------
// @class Test invalid arguments
//
class Boundary : public TCIRowsetFind { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Boundary,TCIRowsetFind);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember DB_E_BADCHAPTER
	int Variation_1();
	// @cmember E_INVALIDARG, cbBookmark != 0, pBookmark = NULL
	int Variation_2();
	// @cmember E_INVALIDARG, pcRowsObtained = NULL
	int Variation_3();
	// @cmember E_INVALIDARG prghRows = NULL
	int Variation_4();
	// @cmember DB_E_COMPAREOPS, CompareOp = -1 (<min
	int Variation_5();
	// @cmember DB_E_BADCOMPAREOP, CompareOp > max
	int Variation_6();
	// @cmember DB_E_BADCOMPAREOP, unsupported
	int Variation_7();
	// @cmember DB_E_BADBINDINFO, NULL hAccessor
	int Variation_8();
	// @cmember DB_E_BADBINDINFO, 2 columns bound
	int Variation_9();
	// @cmember DB_E_BADBINDINFO, many columns bound
	int Variation_10();
	// @cmember DB_E_CANTSCROLLBACKWARDS.  cRows > 1
	int Variation_11();
	// @cmember DB_E_CANTSCROLLBACKWARDS
	int Variation_12();
	// @cmember DB_E_CANTSCROLLBACKWARDS, pBookmark = DBBMK_FIRST
	int Variation_13();
	// @cmember DB_E_CANTSCROLLBACKWARDS, pBokmark = DBBMK_LAST
	int Variation_14();
	// @cmember DB_E_CANTFETCHBACKWARDS
	int Variation_15();
	// @cmember DB_E_BADCOMPAREOP, IGNORE and CASESENSITIVE
	int Variation_16();
	// @cmember DB_E_BADCOMPAREOP, just CASESENSITIVE
	int Variation_17();
	// @cmember DB_E_BADCOMPAREOP, just caseinsensitive
	int Variation_18();
	// @cmember DB_E_BADCOMPAREOP, both case sensitive and insensitive
	int Variation_19();
	// @cmember DB_S_ENDOFROWSET, search for null on non-nullable column
	int Variation_20();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(Boundary)
#define THE_CLASS Boundary
BEG_TEST_CASE(Boundary, TCIRowsetFind, L"Test invalid arguments")
	TEST_VARIATION(1, 		L"DB_E_BADCHAPTER")
	TEST_VARIATION(2, 		L"E_INVALIDARG, cbBookmark != 0, pBookmark = NULL")
	TEST_VARIATION(3, 		L"E_INVALIDARG, pcRowsObtained = NULL")
	TEST_VARIATION(4, 		L"E_INVALIDARG prghRows = NULL")
	TEST_VARIATION(5, 		L"DB_E_COMPAREOPS, CompareOp = -1 (<min")
	TEST_VARIATION(6, 		L"DB_E_BADCOMPAREOP, CompareOp > max")
	TEST_VARIATION(7, 		L"DB_E_BADCOMPAREOP, unsupported")
	TEST_VARIATION(8, 		L"DB_E_BADBINDINFO, NULL hAccessor")
	TEST_VARIATION(9, 		L"DB_E_BADBINDINFO, 2 columns bound")
	TEST_VARIATION(10, 		L"DB_E_BADBINDINFO, many columns bound")
	TEST_VARIATION(11, 		L"DB_E_CANTSCROLLBACKWARDS.  cRows > 1")
	TEST_VARIATION(12, 		L"DB_E_CANTSCROLLBACKWARDS")
	TEST_VARIATION(13, 		L"DB_E_CANTSCROLLBACKWARDS, pBookmark = DBBMK_FIRST")
	TEST_VARIATION(14, 		L"DB_E_CANTSCROLLBACKWARDS, pBokmark = DBBMK_LAST")
	TEST_VARIATION(15, 		L"DB_E_CANTFETCHBACKWARDS")
	TEST_VARIATION(16, 		L"DB_E_BADCOMPAREOP, IGNORE and CASESENSITIVE")
	TEST_VARIATION(17, 		L"DB_E_BADCOMPAREOP, just CASESENSITIVE")
	TEST_VARIATION(18, 		L"DB_E_BADCOMPAREOP, just caseinsensitive")
	TEST_VARIATION(19, 		L"DB_E_BADCOMPAREOP, both case sensitive and insensitive")
	TEST_VARIATION(20, 		L"DB_S_ENDOFROWSET, search for null on non-nullable column")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(OutputRowHandleAllocation)
//--------------------------------------------------------------------
// @class Test cRows, prghRows, pcRowsObtained parameters
//
class OutputRowHandleAllocation : public TCIRowsetFind { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(OutputRowHandleAllocation,TCIRowsetFind);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember cRows==0, *prghRows != NULL,   Verify *prghRows != NULL on output
	int Variation_1();
	// @cmember *pcRowsObtained = 0, *prghRows = NULL.  Verify *prghRows = NULL on output
	int Variation_2();
	// @cmember pBookmark=NULL, cRows=0,  Match last.  Verify S_OK.  Set fSkip=1, cRows = 1, pBookmark = NULL, match last. DB_S_ENDOFROWSET
	int Variation_3();
	// @cmember pBookmark=NULL, cRows=LONG_MAX.  fSkip=0.  Match first row.  Verify DB_S_ENDOFROWSET and all rows in rowset
	int Variation_4();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(OutputRowHandleAllocation)
#define THE_CLASS OutputRowHandleAllocation
BEG_TEST_CASE(OutputRowHandleAllocation, TCIRowsetFind, L"Test cRows, prghRows, pcRowsObtained parameters")
	TEST_VARIATION(1, 		L"cRows==0, *prghRows != NULL,   Verify *prghRows != NULL on output")
	TEST_VARIATION(2, 		L"*pcRowsObtained = 0, *prghRows = NULL.  Verify *prghRows = NULL on output")
	TEST_VARIATION(3, 		L"pBookmark=NULL, cRows=0,  Match last.  Verify S_OK.  Set fSkip=1, cRows = 1, pBookmark = NULL, match last. DB_S_ENDOFROWSET")
	TEST_VARIATION(4, 		L"pBookmark=NULL, cRows=LONG_MAX.  fSkip=0.  Match first row.  Verify DB_S_ENDOFROWSET and all rows in rowset")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(CompareOp)
//--------------------------------------------------------------------
// @class Test the various compare operations
//
class CompareOp : public TCIRowsetFind { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(CompareOp,TCIRowsetFind);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember COMPAREOPS_EQ
	int Variation_1();
	// @cmember COMPAREOPS_LT
	int Variation_2();
	// @cmember COMPAREOPS_LE, Use Equal case
	int Variation_3();
	// @cmember COMPAREOPS_LE, Use Less Than case
	int Variation_4();
	// @cmember COMPAREOPS_GT
	int Variation_5();
	// @cmember COMPAREOPS_GE, Use Equal case
	int Variation_6();
	// @cmember COMPAREOPS_GE, Use Greater than case
	int Variation_7();
	// @cmember COMPAREOPS_BEGINSWITH
	int Variation_8();
	// @cmember COMPAREOPS_CONTAINS, match start of string
	int Variation_9();
	// @cmember COMPAREOPS_CONTAINS, match middle of string
	int Variation_10();
	// @cmember COMPAREOPS_CONTAINS, match end of string
	int Variation_11();
	// @cmember COMPAREOPS_NE
	int Variation_12();
	// @cmember COMPAREOPS_IGNORE
	int Variation_13();
	// @cmember COMPAREOPS_NOTBEGINSWITH
	int Variation_14();
	// @cmember COMPAREOPS_NOTCONTAINS
	int Variation_15();
	// @cmember COMPAREOPS_CONTAINS, use Equal case
	int Variation_16();
	// @cmember COMPAREOPS_BEGINSWITH, Use Equal case
	int Variation_17();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(CompareOp)
#define THE_CLASS CompareOp
BEG_TEST_CASE(CompareOp, TCIRowsetFind, L"Test the various compare operations")
	TEST_VARIATION(1, 		L"COMPAREOPS_EQ")
	TEST_VARIATION(2, 		L"COMPAREOPS_LT")
	TEST_VARIATION(3, 		L"COMPAREOPS_LE, Use Equal case")
	TEST_VARIATION(4, 		L"COMPAREOPS_LE, Use Less Than case")
	TEST_VARIATION(5, 		L"COMPAREOPS_GT")
	TEST_VARIATION(6, 		L"COMPAREOPS_GE, Use Equal case")
	TEST_VARIATION(7, 		L"COMPAREOPS_GE, Use Greater than case")
	TEST_VARIATION(8, 		L"COMPAREOPS_BEGINSWITH")
	TEST_VARIATION(9, 		L"COMPAREOPS_CONTAINS, match start of string")
	TEST_VARIATION(10, 		L"COMPAREOPS_CONTAINS, match middle of string")
	TEST_VARIATION(11, 		L"COMPAREOPS_CONTAINS, match end of string")
	TEST_VARIATION(12, 		L"COMPAREOPS_NE")
	TEST_VARIATION(13, 		L"COMPAREOPS_IGNORE")
	TEST_VARIATION(14, 		L"COMPAREOPS_NOTBEGINSWITH")
	TEST_VARIATION(15, 		L"COMPAREOPS_NOTCONTAINS")
	TEST_VARIATION(16, 		L"COMPAREOPS_CONTAINS, use Equal case")
	TEST_VARIATION(17, 		L"COMPAREOPS_BEGINSWITH, Use Equal case")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(fSkipCurrent)
//--------------------------------------------------------------------
// @class Test for correct behavior with fSkipCurrent flag
//
class fSkipCurrent : public TCIRowsetFind { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(fSkipCurrent,TCIRowsetFind);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember fSkip=0, cRows=1.  Match 1st.   Verify S_OK and one row handle
	int Variation_1();
	// @cmember Match 1st row.  pBookMark=NULL, fSkip=TRUE.  Match 1st row. DB_S_ENDOFROWSET and no row handles.
	int Variation_2();
	// @cmember Match 5th row.  pBookMark=NULL, fSkip=FALSE.  Match 5th row. S_OK and no row handles.
	int Variation_3();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(fSkipCurrent)
#define THE_CLASS fSkipCurrent
BEG_TEST_CASE(fSkipCurrent, TCIRowsetFind, L"Test for correct behavior with fSkipCurrent flag")
	TEST_VARIATION(1, 		L"fSkip=0, cRows=1.  Match 1st.   Verify S_OK and one row handle")
	TEST_VARIATION(2, 		L"Match 1st row.  pBookMark=NULL, fSkip=TRUE.  Match 1st row. DB_S_ENDOFROWSET and no row handles.")
	TEST_VARIATION(3, 		L"Match 5th row.  pBookMark=NULL, fSkip=FALSE.  Match 5th row. S_OK and no row handles.")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(pFindValue)
//--------------------------------------------------------------------
// @class Test the various coercions possible
//
class pFindValue : public TCIRowsetFind { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(pFindValue,TCIRowsetFind);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember IStorage compares, DBCOMPAREOPS_EQ
	int Variation_1();
	// @cmember IStorage, DBCOMPAREOPS_BEGINSWITH
	int Variation_2();
	// @cmember IStorage, DBCOMPAREOPS_GT
	int Variation_3();
	// @cmember IStorage, DBCOMPAREOPS_GE (EQUAL
	int Variation_4();
	// @cmember IStorage, DBCOMPAREOPS_GE (Greater
	int Variation_5();
	// @cmember IStorage, DBCOMPAREOPS_LT
	int Variation_6();
	// @cmember IStorage, DBCOMPAREOPS_LE (EQUAL
	int Variation_7();
	// @cmember IStorage, DBCOMPAREOPS_LE (Less
	int Variation_8();
	// @cmember IStorage, DBCOMPAREOPS_CONTAINS, beginning
	int Variation_9();
	// @cmember IStorage, DBCOMPAREOPS_CONTAINS, middle
	int Variation_10();
	// @cmember IStorage, DBCOMPAREOPS_CONTAINS, end
	int Variation_11();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(pFindValue)
#define THE_CLASS pFindValue
BEG_TEST_CASE(pFindValue, TCIRowsetFind, L"Test the various coercions possible")
	TEST_VARIATION(1, 		L"IStorage compares, DBCOMPAREOPS_EQ")
	TEST_VARIATION(2, 		L"IStorage, DBCOMPAREOPS_BEGINSWITH")
	TEST_VARIATION(3, 		L"IStorage, DBCOMPAREOPS_GT")
	TEST_VARIATION(4, 		L"IStorage, DBCOMPAREOPS_GE (EQUAL")
	TEST_VARIATION(5, 		L"IStorage, DBCOMPAREOPS_GE (Greater")
	TEST_VARIATION(6, 		L"IStorage, DBCOMPAREOPS_LT")
	TEST_VARIATION(7, 		L"IStorage, DBCOMPAREOPS_LE (EQUAL")
	TEST_VARIATION(8, 		L"IStorage, DBCOMPAREOPS_LE (Less")
	TEST_VARIATION(9, 		L"IStorage, DBCOMPAREOPS_CONTAINS, beginning")
	TEST_VARIATION(10, 		L"IStorage, DBCOMPAREOPS_CONTAINS, middle")
	TEST_VARIATION(11, 		L"IStorage, DBCOMPAREOPS_CONTAINS, end")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(SingleRowRowset)
//--------------------------------------------------------------------
// @class Test on a one row rowset
//
class SingleRowRowset : public TCIRowsetFind { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(SingleRowRowset,TCIRowsetFind);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Match 1st. pBookMark = DBBMK_FIRST, cRows=1, fSkip=TRUE.  DB_S_ENDOFROWSET
	int Variation_1();
	// @cmember Match 1st. pBookMark = DBBMK_FIRST, cRows=1, fSkipCurrent = FALSE.  S_OK and one row.
	int Variation_2();
	// @cmember Match First. pBookmark = DBBMK_LAST. cRows=-1, fSkip=TRUE.  DB_S_ENDOFROWSET
	int Variation_3();
	// @cmember Match first.  pBookMark=DBBMK_LAST. cRows=-1.  fSkip=-1.  fSkip=FALSE.  S_OK and 1 hrow
	int Variation_4();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(SingleRowRowset)
#define THE_CLASS SingleRowRowset
BEG_TEST_CASE(SingleRowRowset, TCIRowsetFind, L"Test on a one row rowset")
	TEST_VARIATION(1, 		L"Match 1st. pBookMark = DBBMK_FIRST, cRows=1, fSkip=TRUE.  DB_S_ENDOFROWSET")
	TEST_VARIATION(2, 		L"Match 1st. pBookMark = DBBMK_FIRST, cRows=1, fSkipCurrent = FALSE.  S_OK and one row.")
	TEST_VARIATION(3, 		L"Match First. pBookmark = DBBMK_LAST. cRows=-1, fSkip=TRUE.  DB_S_ENDOFROWSET")
	TEST_VARIATION(4, 		L"Match first.  pBookMark=DBBMK_LAST. cRows=-1.  fSkip=-1.  fSkip=FALSE.  S_OK and 1 hrow")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(Related_RestartPosition)
//--------------------------------------------------------------------
// @class Test in conjunction with RestartPosition
//
class Related_RestartPosition : public TCIRowsetFind { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Related_RestartPosition,TCIRowsetFind);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Match 5th. Restart.  Call Find, cRows=1, fSkip=TRUE, match 5th.  Verify DB_S_ENDOFROWSET (because of restart
	int Variation_1();
	// @cmember Match 5th. Restart.  Call Find, cRows=1, fSkip=FALSE, match 4th.  Verify DB_S_ENDOFROWSET
	int Variation_2();
	// @cmember Match Last.  Restart. Call Find, cRows=1, fSkip=TRUE.  Verify DB_S_ENDOFROWSET
	int Variation_3();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(Related_RestartPosition)
#define THE_CLASS Related_RestartPosition
BEG_TEST_CASE(Related_RestartPosition, TCIRowsetFind, L"Test in conjunction with RestartPosition")
	TEST_VARIATION(1, 		L"Match 5th. Restart.  Call Find, cRows=1, fSkip=TRUE, match 5th.  Verify DB_S_ENDOFROWSET (because of restart")
	TEST_VARIATION(2, 		L"Match 5th. Restart.  Call Find, cRows=1, fSkip=FALSE, match 4th.  Verify DB_S_ENDOFROWSET")
	TEST_VARIATION(3, 		L"Match Last.  Restart. Call Find, cRows=1, fSkip=TRUE.  Verify DB_S_ENDOFROWSET")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(Related_GetNextRows)
//--------------------------------------------------------------------
// @class Test in conjunction with GetNextRows
//
class Related_GetNextRows : public TCIRowsetFind { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Related_GetNextRows,TCIRowsetFind);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember pBookmark=DBBMK_FIRST, Verify GetNextRows pos not changed
	int Variation_1();
	// @cmember Use pBookmark=middle bmk.  Verify GetNextRows pos not changed
	int Variation_2();
	// @cmember Use pBookMark = DBBMK_LAST, Verify GetNextRows pos not changed
	int Variation_3();
	// @cmember use pBookmark=NULL, match 1st row. Verify GetNextRows pos is after 1st row.
	int Variation_4();
	// @cmember Use pBookmark=NULL, match last row.  Verify GetNextRows with cRows=1 returns DB_S_ENDOFROWSET
	int Variation_5();
	// @cmember Use pBookmark=NULL, match middle row. Use pBookmark=2nd bmk and match 2nd.  Verify GetNextRows is still after middle row
	int Variation_6();
	// @cmember Call GetNextRows with last row fetched = 3.  Call FindNextRows, pBookmark=NULL,cRows=-1 to match 4th.  Verify DB_S_ENDOFROWSET
	int Variation_7();
	// @cmember GetNextRows, cRows=1, Offset=0.  Verify S_OK.  FindNext with pBookmark=NULL, cRows=1 and match 1st.  Verify DB_S_ENDOFROWSET
	int Variation_8();
	// @cmember FindNext with pBookmark=NULL, cRows=2 and match 4th.  GetNextRows with cRows=-3.  Verify 5,4,3 row handles.
	int Variation_9();
	// @cmember FindNext, pBookmark=NULL, match last. FindNext, pBmk=NULL, cRows=-1, match last.  GetNext. cRows=-1 and verify next to last
	int Variation_10();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(Related_GetNextRows)
#define THE_CLASS Related_GetNextRows
BEG_TEST_CASE(Related_GetNextRows, TCIRowsetFind, L"Test in conjunction with GetNextRows")
	TEST_VARIATION(1, 		L"pBookmark=DBBMK_FIRST, Verify GetNextRows pos not changed")
	TEST_VARIATION(2, 		L"Use pBookmark=middle bmk.  Verify GetNextRows pos not changed")
	TEST_VARIATION(3, 		L"Use pBookMark = DBBMK_LAST, Verify GetNextRows pos not changed")
	TEST_VARIATION(4, 		L"use pBookmark=NULL, match 1st row. Verify GetNextRows pos is after 1st row.")
	TEST_VARIATION(5, 		L"Use pBookmark=NULL, match last row.  Verify GetNextRows with cRows=1 returns DB_S_ENDOFROWSET")
	TEST_VARIATION(6, 		L"Use pBookmark=NULL, match middle row. Use pBookmark=2nd bmk and match 2nd.  Verify GetNextRows is still after middle row")
	TEST_VARIATION(7, 		L"Call GetNextRows with last row fetched = 3.  Call FindNextRows, pBookmark=NULL,cRows=-1 to match 4th.  Verify DB_S_ENDOFROWSET")
	TEST_VARIATION(8, 		L"GetNextRows, cRows=1, Offset=0.  Verify S_OK.  FindNext with pBookmark=NULL, cRows=1 and match 1st.  Verify DB_S_ENDOFROWSET")
	TEST_VARIATION(9, 		L"FindNext with pBookmark=NULL, cRows=2 and match 4th.  GetNextRows with cRows=-3.  Verify 5,4,3 row handles.")
	TEST_VARIATION(10, 		L"FindNext, pBookmark=NULL, match last. FindNext, pBmk=NULL, cRows=-1, match last.  GetNext. cRows=-1 and verify next to last")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(Scroll_BookMark)
//--------------------------------------------------------------------
// @class CanScrollBackwards with bookmarks
//
class Scroll_BookMark : public TCIRowsetFind { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Scroll_BookMark,TCIRowsetFind);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember pBookMark=DBBMK_FIRST, cRows=1.  Match first row.  Verify S_OK and one row handle
	int Variation_1();
	// @cmember pBookMark=DBBMK_LAST,cRows=1.  Match last row.  Verify S_OK and last row
	int Variation_2();
	// @cmember pBookMark=DBBMK_INVALID. Verify DB_E_BADBOOKMARK
	int Variation_3();
	// @cmember pBookMark=one bookmark for each row, cRows=1.  Always match current row.
	int Variation_4();
	// @cmember pBookmark=random value.  Verify DB_E_BADBOOKMARK (warning-could get a provider specific error
	int Variation_5();
	// @cmember *pBookmark=2nd row and offset=-1, cRows=1.  Verify first row matched.
	int Variation_6();
	// @cmember *pBookmark=4th row, Offset=2, cRows=1 and match 5th row.  Verify DB_S_ENDOFROWSET
	int Variation_7();
	// @cmember *pBookmark=5th row.  Offset=0, cRows=1 and match 5th. Again with pBookmark=NULL, Offset=0, cRows=1 and match 2nd.  Verify S_OK
	int Variation_8();
	// @cmember *pBookmark=2nd row and offset = -2, cRows=1, Verify DB_S_ENDOFROWSET
	int Variation_9();
	// @cmember pBookmark=DBBMK_FIRST, cRows=0 Should be no-op
	int Variation_10();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(Scroll_BookMark)
#define THE_CLASS Scroll_BookMark
BEG_TEST_CASE(Scroll_BookMark, TCIRowsetFind, L"CanScrollBackwards with bookmarks")
	TEST_VARIATION(1, 		L"pBookMark=DBBMK_FIRST, cRows=1.  Match first row.  Verify S_OK and one row handle")
	TEST_VARIATION(2, 		L"pBookMark=DBBMK_LAST,cRows=1.  Match last row.  Verify S_OK and last row")
	TEST_VARIATION(3, 		L"pBookMark=DBBMK_INVALID. Verify DB_E_BADBOOKMARK")
	TEST_VARIATION(4, 		L"pBookMark=one bookmark for each row, cRows=1.  Always match current row.")
	TEST_VARIATION(5, 		L"pBookmark=random value.  Verify DB_E_BADBOOKMARK (warning-could get a provider specific error")
	TEST_VARIATION(6, 		L"*pBookmark=2nd row and offset=-1, cRows=1.  Verify first row matched.")
	TEST_VARIATION(7, 		L"*pBookmark=4th row, Offset=2, cRows=1 and match 5th row.  Verify DB_S_ENDOFROWSET")
	TEST_VARIATION(8, 		L"*pBookmark=5th row.  Offset=0, cRows=1 and match 5th. Again with pBookmark=NULL, Offset=0, cRows=1 and match 2nd.  Verify S_OK")
	TEST_VARIATION(9, 		L"*pBookmark=2nd row and offset = -2, cRows=1, Verify DB_S_ENDOFROWSET")
	TEST_VARIATION(10, 		L"pBookmark=DBBMK_FIRST, cRows=0 Should be no-op")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(Scroll_Fetch_Bookmarks)
//--------------------------------------------------------------------
// @class CanScrollBackwards and CanFetchBackwards with Bookmarks
//
class Scroll_Fetch_Bookmarks : public TCIRowsetFind { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Scroll_Fetch_Bookmarks,TCIRowsetFind);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember pBookMark=DBBMK_FIRST, cRows=5 and match 1st row.  Verify S_OK and 5 hrows
	int Variation_1();
	// @cmember pBookMarks=DBBMK_LAST,cRows=-5  match last row.  Verify S_OK and 5 hrows
	int Variation_2();
	// @cmember pBookmark=2nd row, cRows=2, Offset=1.  Match 4th row.  Verify S_OK and 2 hrows
	int Variation_3();
	// @cmember pBookMark=5th row, cRows=-2.  Match3rd row.  Verify S_OK and 2 hrows
	int Variation_4();
	// @cmember pBookmark=Last row, cRows=1, Offset=1. Verify DB_S_ENDOFROWSET
	int Variation_5();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(Scroll_Fetch_Bookmarks)
#define THE_CLASS Scroll_Fetch_Bookmarks
BEG_TEST_CASE(Scroll_Fetch_Bookmarks, TCIRowsetFind, L"CanScrollBackwards and CanFetchBackwards with Bookmarks")
	TEST_VARIATION(1, 		L"pBookMark=DBBMK_FIRST, cRows=5 and match 1st row.  Verify S_OK and 5 hrows")
	TEST_VARIATION(2, 		L"pBookMarks=DBBMK_LAST,cRows=-5  match last row.  Verify S_OK and 5 hrows")
	TEST_VARIATION(3, 		L"pBookmark=2nd row, cRows=2, Offset=1.  Match 4th row.  Verify S_OK and 2 hrows")
	TEST_VARIATION(4, 		L"pBookMark=5th row, cRows=-2.  Match3rd row.  Verify S_OK and 2 hrows")
	TEST_VARIATION(5, 		L"pBookmark=Last row, cRows=1, Offset=1. Verify DB_S_ENDOFROWSET")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(CaseSensitive_Compares)
//--------------------------------------------------------------------
// @class Test the case sensitive property
//
class CaseSensitive_Compares : public TCIRowsetFind { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(CaseSensitive_Compares,TCIRowsetFind);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember EQ, sensitive
	int Variation_1();
	// @cmember EQ, insensitive
	int Variation_2();
	// @cmember LT, sensitive
	int Variation_3();
	// @cmember LT, insensitve
	int Variation_4();
	// @cmember LE, sensitive
	int Variation_5();
	// @cmember LE, insensitive
	int Variation_6();
	// @cmember GT, sensitive
	int Variation_7();
	// @cmember GT, insensitive
	int Variation_8();
	// @cmember GE, sensitive
	int Variation_9();
	// @cmember GE, insensitve
	int Variation_10();
	// @cmember NE, sensitive
	int Variation_11();
	// @cmember NE, insensitive
	int Variation_12();
	// @cmember BEGINSWITH, sensitve
	int Variation_13();
	// @cmember BEGINSWITH, insensitive
	int Variation_14();
	// @cmember BEGINSWITH, sensitive negative match
	int Variation_15();
	// @cmember CONTAINS, sensitive
	int Variation_16();
	// @cmember CONTAINS, insensitive
	int Variation_17();
	// @cmember CONTAINS, sensitive negative match
	int Variation_18();
	// @cmember NOTBEGINSWITH, sensitive
	int Variation_19();
	// @cmember NOTBEGINSWITH, insensitive
	int Variation_20();
	// @cmember NOTBEGINSWITH, sensitive negative match
	int Variation_21();
	// @cmember NOTCONTAINS, sensitive
	int Variation_22();
	// @cmember NOCONTAINS, insensitive
	int Variation_23();
	// @cmember NOCONTAINS, sensitive negative match
	int Variation_24();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(CaseSensitive_Compares)
#define THE_CLASS CaseSensitive_Compares
BEG_TEST_CASE(CaseSensitive_Compares, TCIRowsetFind, L"Test the case sensitive property")
	TEST_VARIATION(1, 		L"EQ, sensitive")
	TEST_VARIATION(2, 		L"EQ, insensitive")
	TEST_VARIATION(3, 		L"LT, sensitive")
	TEST_VARIATION(4, 		L"LT, insensitve")
	TEST_VARIATION(5, 		L"LE, sensitive")
	TEST_VARIATION(6, 		L"LE, insensitive")
	TEST_VARIATION(7, 		L"GT, sensitive")
	TEST_VARIATION(8, 		L"GT, insensitive")
	TEST_VARIATION(9, 		L"GE, sensitive")
	TEST_VARIATION(10, 		L"GE, insensitve")
	TEST_VARIATION(11, 		L"NE, sensitive")
	TEST_VARIATION(12, 		L"NE, insensitive")
	TEST_VARIATION(13, 		L"BEGINSWITH, sensitve")
	TEST_VARIATION(14, 		L"BEGINSWITH, insensitive")
	TEST_VARIATION(15, 		L"BEGINSWITH, sensitive negative match")
	TEST_VARIATION(16, 		L"CONTAINS, sensitive")
	TEST_VARIATION(17, 		L"CONTAINS, insensitive")
	TEST_VARIATION(18, 		L"CONTAINS, sensitive negative match")
	TEST_VARIATION(19, 		L"NOTBEGINSWITH, sensitive")
	TEST_VARIATION(20, 		L"NOTBEGINSWITH, insensitive")
	TEST_VARIATION(21, 		L"NOTBEGINSWITH, sensitive negative match")
	TEST_VARIATION(22, 		L"NOTCONTAINS, sensitive")
	TEST_VARIATION(23, 		L"NOCONTAINS, insensitive")
	TEST_VARIATION(24, 		L"NOCONTAINS, sensitive negative match")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(Scroll_Fetch_Hold)
//--------------------------------------------------------------------
// @class Test with CanScrollBack, CanFetchBack, and CanHoldRows
//
class Scroll_Fetch_Hold : public TCIRowsetFind { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Scroll_Fetch_Hold,TCIRowsetFind);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember pBookmark=2nd row, cRows=-1.  Verify S_OK and one row handle
	int Variation_1();
	// @cmember pBookmark=3rd row.  cRows=3.  Match 3rd row.  Do not release. pBookmark=4th row, cRows=2, match 4th.  Release 1st 3, ver last 2
	int Variation_2();
	// @cmember pBookmark=5th,cRows=-5, match 5th. S_OK,5 hrows. pBookmark=4th,cRows=-5. DB_S_ENDOFROWSET and 4 hrows. Release last 4, check 5
	int Variation_3();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(Scroll_Fetch_Hold)
#define THE_CLASS Scroll_Fetch_Hold
BEG_TEST_CASE(Scroll_Fetch_Hold, TCIRowsetFind, L"Test with CanScrollBack, CanFetchBack, and CanHoldRows")
	TEST_VARIATION(1, 		L"pBookmark=2nd row, cRows=-1.  Verify S_OK and one row handle")
	TEST_VARIATION(2, 		L"pBookmark=3rd row.  cRows=3.  Match 3rd row.  Do not release. pBookmark=4th row, cRows=2, match 4th.  Release 1st 3, ver last 2")
	TEST_VARIATION(3, 		L"pBookmark=5th,cRows=-5, match 5th. S_OK,5 hrows. pBookmark=4th,cRows=-5. DB_S_ENDOFROWSET and 4 hrows. Release last 4, check 5")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(Dynamic)
//--------------------------------------------------------------------
// @class Test with OTHERINSERT and OTHERUPDATE properties
//
class Dynamic : public TCIRowsetFind { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Dynamic,TCIRowsetFind);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Insert a new row at end.  pBookmark=NULL,cRows=1.  Match new row.  Verify S_OK and one row handle
	int Variation_1();
	// @cmember Insert a new row at the end.  pBookmark=DBBMK_LAST,cRows=1.  DBCOMPAREOPS_IGNORE.  Verify S_OK and new row matched.
	int Variation_2();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(Dynamic)
#define THE_CLASS Dynamic
BEG_TEST_CASE(Dynamic, TCIRowsetFind, L"Test with OTHERINSERT and OTHERUPDATE properties")
	TEST_VARIATION(1, 		L"Insert a new row at end.  pBookmark=NULL,cRows=1.  Match new row.  Verify S_OK and one row handle")
	TEST_VARIATION(2, 		L"Insert a new row at the end.  pBookmark=DBBMK_LAST,cRows=1.  DBCOMPAREOPS_IGNORE.  Verify S_OK and new row matched.")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(MaxRows)
//--------------------------------------------------------------------
// @class Test MaxRows property
//
class MaxRows : public TCIRowsetFind { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(MaxRows,TCIRowsetFind);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Set MAXROWS=3. pBookmark=2nd row. cRows=4. Match 3rd row. DB_S_ENDOFROWSET and 3 hrows
	int Variation_1();
	// @cmember Set MAXROWS=2. pBookmark=3rd row,cRows=3. Match 3rd row.  DB_S_ROWLIMITEXECEEDED and 2 hrows
	int Variation_2();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(MaxRows)
#define THE_CLASS MaxRows
BEG_TEST_CASE(MaxRows, TCIRowsetFind, L"Test MaxRows property")
	TEST_VARIATION(1, 		L"Set MAXROWS=3. pBookmark=2nd row. cRows=4. Match 3rd row. DB_S_ENDOFROWSET and 3 hrows")
	TEST_VARIATION(2, 		L"Set MAXROWS=2. pBookmark=3rd row,cRows=3. Match 3rd row.  DB_S_ROWLIMITEXECEEDED and 2 hrows")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(EmptyRowset)
//--------------------------------------------------------------------
// @class EmptyRowset
//
class EmptyRowset : public TCIRowsetFind { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(EmptyRowset,TCIRowsetFind);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember pBookmark=NULL, cRows=0. DBCOMPAREOPS_IGNORE.  Verify DB_S_ENDOFROWSET
	int Variation_1();
	// @cmember pBookmark=NULL, cRows=1.  DB_S_ENDOFROWSET
	int Variation_2();
	// @cmember pBookmark=NULL, cRows=-1. DB_S_ENDOFROWSET
	int Variation_3();
	// @cmember pBookmark=NULL,cRows=0,fSkip=TRUE.  Verify DB_S_ENDOFROWSET
	int Variation_4();
	// @cmember pBookMark=DBBMK_FIRST,cRows=1.  Verify DB_S_ENDOFROWSET
	int Variation_5();
	// @cmember pBookmark=DBBMK_FIRST, cRows=0, DBCOMPAREOPS_IGNORE.  Verify DB_S_ENDOFROWSET
	int Variation_6();
	// @cmember pBookmark=DBBMK_LAST,cRows=1.  Verify DB_S_ENDOFROWSET
	int Variation_7();
	// @cmember pBookmark=DBBMK_FIRST,cRows=-1.  Verify DB_S_ENDOFROWSET
	int Variation_8();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(EmptyRowset)
#define THE_CLASS EmptyRowset
BEG_TEST_CASE(EmptyRowset, TCIRowsetFind, L"EmptyRowset")
	TEST_VARIATION(1, 		L"pBookmark=NULL, cRows=0. DBCOMPAREOPS_IGNORE.  Verify DB_S_ENDOFROWSET")
	TEST_VARIATION(2, 		L"pBookmark=NULL, cRows=1.  DB_S_ENDOFROWSET")
	TEST_VARIATION(3, 		L"pBookmark=NULL, cRows=-1. DB_S_ENDOFROWSET")
	TEST_VARIATION(4, 		L"pBookmark=NULL,cRows=0,fSkip=TRUE.  Verify DB_S_ENDOFROWSET")
	TEST_VARIATION(5, 		L"pBookMark=DBBMK_FIRST,cRows=1.  Verify DB_S_ENDOFROWSET")
	TEST_VARIATION(6, 		L"pBookmark=DBBMK_FIRST, cRows=0, DBCOMPAREOPS_IGNORE.  Verify DB_S_ENDOFROWSET")
	TEST_VARIATION(7, 		L"pBookmark=DBBMK_LAST,cRows=1.  Verify DB_S_ENDOFROWSET")
	TEST_VARIATION(8, 		L"pBookmark=DBBMK_FIRST,cRows=-1.  Verify DB_S_ENDOFROWSET")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(NULL_Collation)
//--------------------------------------------------------------------
// @class Test Null collatioans
//
class NULL_Collation : public TCIRowsetFind { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(NULL_Collation,TCIRowsetFind);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember NE operator with NULL status
	int Variation_1();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(NULL_Collation)
#define THE_CLASS NULL_Collation
BEG_TEST_CASE(NULL_Collation, TCIRowsetFind, L"Test Null collations")
	TEST_VARIATION(1, 		L"NE operator with NULL status")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(Prop_FINDCOMPAREOPS)
//--------------------------------------------------------------------
// @class Check DBPROP_FINDCOMAPREOPS
//
class Prop_FINDCOMPAREOPS : public TCIRowsetFind { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Prop_FINDCOMPAREOPS,TCIRowsetFind);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Check CO_EQUALITY and CO_STRING
	int Variation_1();
	// @cmember Check Case Sensitivity
	int Variation_2();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(Prop_FINDCOMPAREOPS)
#define THE_CLASS Prop_FINDCOMPAREOPS
BEG_TEST_CASE(Prop_FINDCOMPAREOPS, TCIRowsetFind, L"Check DBPROP_FINDCOMAPREOPS")
	TEST_VARIATION(1, 		L"Check CO_EQUALITY and CO_STRING")
	TEST_VARIATION(2, 		L"Check Case Sensitivity")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(QueryInt)
//--------------------------------------------------------------------
// @class test queryinterfaces
//
class QueryInt : public TCIRowsetFind { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(QueryInt,TCIRowsetFind);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember IRowset
	int Variation_1();
	// @cmember IAccessor
	int Variation_2();
	// @cmember Use ICmdText
	int Variation_3();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(QueryInt)
#define THE_CLASS QueryInt
BEG_TEST_CASE(QueryInt, TCIRowsetFind, L"test queryinterfaces")
	TEST_VARIATION(1, 		L"IRowset")
	TEST_VARIATION(2, 		L"IAccessor")
	TEST_VARIATION(3, 		L"Use ICmdText")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCCompareOps_Ignore)
//--------------------------------------------------------------------
// @class Test specific IGNORE cases
//
class TCCompareOps_Ignore : public TCIRowsetFind { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCCompareOps_Ignore,TCIRowsetFind);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember IGNORE w/pBookmark=NULL
	int Variation_1();
	// @cmember IGNORE w/ pBookmark = STD_BOOKMARK
	int Variation_2();
	// @cmember IGNORE w/real bookmark
	int Variation_3();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCCompareOps_Ignore)
#define THE_CLASS TCCompareOps_Ignore
BEG_TEST_CASE(TCCompareOps_Ignore, TCIRowsetFind, L"Test specific IGNORE cases")
	TEST_VARIATION(1, 		L"IGNORE w/pBookmark=NULL")
	TEST_VARIATION(2, 		L"IGNORE w/ pBookmark = STD_BOOKMARK")
	TEST_VARIATION(3, 		L"IGNORE w/real bookmark")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(AccessorTests)
//--------------------------------------------------------------------
// @class Test various accessor variations
//
class AccessorTests : public TCIRowsetFind { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(AccessorTests,TCIRowsetFind);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember obValue = 0
	int Variation_1();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(AccessorTests)
#define THE_CLASS AccessorTests
BEG_TEST_CASE(AccessorTests, TCIRowsetFind, L"Test various accessor variations")
	TEST_VARIATION(1, 		L"obValue = 0")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(BindingType)
//--------------------------------------------------------------------
// @class Test interesting Find coercions
//
class BindingType : public TCIRowsetFind { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(BindingType,TCIRowsetFind);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Normal BSTR
	int Variation_1();
	// @cmember WSTR
	int Variation_2();
	// @cmember STR
	int Variation_3();
	// @cmember VARIANT
	int Variation_4();
	// @cmember WSTR | BYREF
	int Variation_5();
	// @cmember STR | BYREF
	int Variation_6();
	// @cmember BSTR | BYREF
	int Variation_7();
	// @cmember VARIANT BYREF
	int Variation_8();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(BindingType)
#define THE_CLASS BindingType
BEG_TEST_CASE(BindingType, TCIRowsetFind, L"Test interesting Find coercions")
	TEST_VARIATION(1, 		L"Normal BSTR")
	TEST_VARIATION(2, 		L"WSTR")
	TEST_VARIATION(3, 		L"STR")
	TEST_VARIATION(4, 		L"VARIANT")
	TEST_VARIATION(5, 		L"WSTR | BYREF")
	TEST_VARIATION(6, 		L"STR | BYREF")
	TEST_VARIATION(7, 		L"BSTR | BYREF")
	TEST_VARIATION(8, 		L"VARIANT BYREF")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(Deleted_Rows)
//--------------------------------------------------------------------
// @class Test with deleted rows
//
class Deleted_Rows : public TCIRowsetFind { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Deleted_Rows,TCIRowsetFind);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember pBookMark=DBBMK_LAST. Delete last row. Expect DB_E_BADBOOKMARK
	int Variation_1();
	// @cmember pBookMark = 2, cRows=1. Delete 4th row. Match on what was in 4th row. DB_S_ENDOFROWSET and 0 row
	int Variation_2();
	// @cmember pBookMark=4, cRows=-1. Delete 1st row. Match on what was in 1st row. DB_S_ENDOFROWSET and 0 row
	int Variation_3();
	// @cmember pBookMark=NULL,cRows=1 Match 3rd row.  Delete third row. pBookmark=NULL, cRows=1, fSkip=0. Match 4th.  S_OK, 1 hrow
	int Variation_4();
	// @cmember pBookmark=NULL, cRows=-2, match 3rd row. Delete 3rd. Match 2nd
	int Variation_5();
	// @cmember Delete RowLast-1 row. pBookmark=RowLast-2 row,cRows=3. Match RowLast-2 row. 
	//			Verify S_OK, 3 hrows and DB_E_DELETEDROW when accessing deleted one
	int Variation_6();
	// @cmember Delete last row. pBookmark=DBBMK_LAST, cRows=1. Verify DB_E_BADBOOKMARK, 0 rows returned
	int Variation_7();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(Deleted_Rows)
#define THE_CLASS Deleted_Rows
BEG_TEST_CASE(Deleted_Rows, TCIRowsetFind, L"Test with deleted rows")
	TEST_VARIATION(1, 		L"pBookMark=DBBMK_LAST. Delete last row. Expect DB_E_BADBOOKMARK")
	TEST_VARIATION(2, 		L"pBookMark = 2, cRows=1. Delete 4th row. Match on what was in 4th row. DB_S_ENDOFROWSET and 0 row")
	TEST_VARIATION(3, 		L"pBookMark=4, cRows=-1. Delete 1st row. Match on what was in 1st row. DB_S_ENDOFROWSET and 0 row")
	TEST_VARIATION(4, 		L"pBookMark=NULL,cRows=1 Match 3rd row.  Delete third row. pBookmark=NULL, cRows=1, fSkip=0. Match 4th.  S_OK, 1 hrow")
	TEST_VARIATION(5, 		L"pBookmark=NULL, cRows=-2, match 3rd row. Delete 3rd. Match 2nd")
	TEST_VARIATION(6, 		L"Delete RowLast-1 row.pBookmark=RowLast-2 row, cRows=3.Match RowLast-2 row.Verify DB_S_ENDOFROWSET and return 3 rows")
	TEST_VARIATION(7, 		L"Delete last row. pBookmark=DBBMK_LAST, cRows=1. Verify DB_E_BADBOOKMARK, 0 rows returned")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(RemoveDeleted)
//--------------------------------------------------------------------
// @class Test in context of RemoveDeleted
//
class RemoveDeleted : public TCIRowsetFind { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(RemoveDeleted,TCIRowsetFind);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Delete a row.  pBookmark=deleted row.  Verify DB_E_BADBOOKMARK.
	int Variation_1();
	// @cmember Delete 3rd row.  pBookmark=2nd row with cRows=3. Verify S_OK and 2,4,5 hrows
	int Variation_2();
	// @cmember Delete g_lRowLast-1 row. pBookmark=g_lRowLast-2 row,cRows=3. Match g_lRowLast-2 row.  
	//          Verify DB_S_ENDOFROWSET and return RowLast-2, RowLast hrows.
	int Variation_3();
	// @cmember Delete last row. pBookmark=DBBMK_LAST,cRows=-1. DBCOMPAREOPS_IGNORE. 
	int Variation_4();
	// @cmember pBookmark=NULL, cRows = 2. match 3rd.  Delete 2nd and 5th rows. Find with cRows= -1 and match third
	int Variation_5();
	// @cmember pBookMark=NULL, cRows=1.Delete 4th row.Match on what was in 4th row.DB_S_ENDOFROWSET and 0 rows
	int Variation_6();
	// @cmember pBookMark=DBBMK_LAST,cRows=-1.Delete 1st row.Match on what was in 1st row.DB_S_ENDOFROWSET and 0 rows
	int Variation_7();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(RemoveDeleted)
#define THE_CLASS RemoveDeleted
BEG_TEST_CASE(RemoveDeleted, TCIRowsetFind, L"Test in context of RemoveDeleted")
	TEST_VARIATION(1, 		L"Delete a row.  pBookmark=deleted row.  Verify DB_E_BADBOOKMARK")
	TEST_VARIATION(2, 		L"Delete 3rd row.  pBookmark=2nd row with cRows=3. Verify S_OK and 2,4,5 hrows")
	TEST_VARIATION(3, 		L"Delete Last-1 row.pBookmark=Last-2 row,cRows=3.Match Last-2 row.Verify DB_S_ENDOFROWSET,2 rows")
	TEST_VARIATION(4, 		L"Delete last row. pBookmark=DBBMK_LAST,cRows=-1. ")
	TEST_VARIATION(5, 		L"pBookmark=NULL, cRows = 2. match 3rd.  Delete 2nd and 5th rows. Find with cRows= -1")
	TEST_VARIATION(6, 		L"pBookMark =NULL,cRows=1.Delete 4th row.Match on what was in 4th row.DB_S_ENDOFROWSET and 0 rows")
	TEST_VARIATION(7, 		L"pBookMark=DBBMK_LAST,cRows=-1.Delete 1st row.Match on what was in 1st row.DB_S_ENDOFROWSET and 0 rows")	
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(BookmarkSkipped)
//--------------------------------------------------------------------
// @class Test in the context of DBPROP_BOOKMARKSKIPPED
//
class BookmarkSkipped : public TCIRowsetFind { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(BookmarkSkipped,TCIRowsetFind);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Delete 1st row. pBookmark=1st row,cRows=5. Match 2nd row.  DB_S_ENDOFROWSET(overrides S_BOOKMARKSKIPPED
	int Variation_1();
	// @cmember Delete last row.  pBookmark=last row, cRows=1.  Match what was in last row.  DB_S_BOOKMARKSKIPPED and no rows retrieved.
	int Variation_2();
	// @cmember Delete the first row. pBookmark=1st row,cRows=1.  Match 2nd row.  DB_S_BOOKMARKSKIPED and 2nd hrow returned.
	int Variation_3();
	// @cmember Delete 3rd row. pBookmark=3rd row,cRows=1.  Match 5th row. DB_S_BOOKMARKSKIPPED and one hrow.
	int Variation_4();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(BookmarkSkipped)
#define THE_CLASS BookmarkSkipped
BEG_TEST_CASE(BookmarkSkipped, TCIRowsetFind, L"Test in the context of DBPROP_BOOKMARKSKIPPED")
	TEST_VARIATION(1, 		L"Delete 1st row. pBookmark=1st row,cRows=5. Match 2nd row.  DB_S_ENDOFROWSET(overrides S_BOOKMARKSKIPPED")
	TEST_VARIATION(2, 		L"Delete last row.  pBookmark=last row, cRows=1.  Match what was in last row.  DB_S_BOOKMARKSKIPPED and no rows retrieved.")
	TEST_VARIATION(3, 		L"Delete the first row. pBookmark=1st row,cRows=1.  Match 2nd row.  DB_S_BOOKMARKSKIPED and 2nd hrow returned.")
	TEST_VARIATION(4, 		L"Delete 3rd row. pBookmark=3rd row,cRows=1.  Match 5th row. DB_S_BOOKMARKSKIPPED and one hrow.")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(TCJapanese)
//*-----------------------------------------------------------------------
// @class Test scenarios relevant to the JPN locale
//
class TCJapanese : public TCIRowsetFind { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCJapanese,TCIRowsetFind);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember Test case sensitivity and CONTAINS
	int Variation_1();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(TCJapanese)
#define THE_CLASS TCJapanese
BEG_TEST_CASE(TCJapanese, TCIRowsetFind, L"Test scenarios relevant to the JPN locale")
	TEST_VARIATION(1, 		L"Test case sensitivity and CONTAINS")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// }} END_DECLARE_TEST_CASES()

// {{ TCW_TESTMODULE(ThisModule)
TEST_MODULE(30, ThisModule, gwszModuleDescrip)
	TEST_CASE(1, No_Properties)
	TEST_CASE(2, FetchBackwards)
	TEST_CASE(3, ScrollBackwards)
	TEST_CASE(4, CanHoldRows)
	TEST_CASE(5, Scroll_and_Fetch)
	TEST_CASE(6, Boundary)
	TEST_CASE(7, OutputRowHandleAllocation)
	TEST_CASE(8, CompareOp)
	TEST_CASE(9, fSkipCurrent)
	TEST_CASE(10, pFindValue)
	TEST_CASE(11, SingleRowRowset)
	TEST_CASE(12, Related_RestartPosition)
	TEST_CASE(13, Related_GetNextRows)
	TEST_CASE(14, Scroll_BookMark)
	TEST_CASE(15, Scroll_Fetch_Bookmarks)
	TEST_CASE(16, CaseSensitive_Compares)
	TEST_CASE(17, Scroll_Fetch_Hold)
	TEST_CASE(18, Dynamic)
	TEST_CASE(19, MaxRows)
	TEST_CASE(20, EmptyRowset)
	TEST_CASE(21, NULL_Collation)
	TEST_CASE(22, Prop_FINDCOMPAREOPS)
	TEST_CASE(23, QueryInt)
	TEST_CASE(24, TCCompareOps_Ignore)
	TEST_CASE(25, AccessorTests)
	TEST_CASE(26, BindingType)
	TEST_CASE(27, Deleted_Rows)
	TEST_CASE(28, RemoveDeleted)
	TEST_CASE(29, BookmarkSkipped)
	TEST_CASE(30, TCJapanese)
END_TEST_MODULE()
// }} TCW_TESTMODULE_END


// {{ TCW_TC_PROTOTYPE(No_Properties)
//*-----------------------------------------------------------------------
//| Test Case:		No_Properties - no properties set
//|	Created:			06/24/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL No_Properties::Init()
{
	DBPROPID	guidPropertySet;
	ULONG	cPrptSet=0;
	BOOL fTestPass = FALSE;

	if(!TCIRowsetFind::Init())
		return FALSE;

	//create a rowset and an accessor.  
	TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetFind,
		cPrptSet,&guidPropertySet));

CLEANUP:
	return TRUE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc pBookmark=NULL,cRows=1,fSkip=1. Traverse rowset by matching current row until DB_S_ENDOF_ROWSET
//
// @rdesc TEST_PASS or TEST_FAIL
//
int No_Properties::Variation_1()
{
	BOOL fTestPass = TEST_PASS;

	for ( LONG i = 1; i <= (g_lRowLast+1); i++ )
	{
		fTestPass = 
				CallFindNextRows(	g_pCTable,	// CTable pointer
										NULL,			// bookmark;
										0,				// Length of bookmark
										1,				// # rows to fetch
										0,				// Offset
										g_ulColNum,	// Which column to match
										i,				// row to match
										( i <= g_lRowLast ? S_OK : DB_S_ENDOFROWSET ),			// HRESULT to verify
										( i <= g_lRowLast ? 1 : 0 )									// How many rows to expect.
									);
		if ( fTestPass == TEST_FAIL )
			break;
	}

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc pBookmark=NULL, cRows=1, match middle.  Again, pBookmark=NULL, cRows=1 and Verify DB_S_ENDOFROWSET
//
// @rdesc TEST_PASS or TEST_FAIL
//
int No_Properties::Variation_2()
{
	BOOL fTestPass = FALSE;

	//restart the cursor position
	if(!CHECK(RestartRowPosition(),S_OK))
		return TEST_FAIL;


	fTestPass = CallFindNextRows(	g_pCTable,	// CTable pointer
									NULL,			// bookmark;
									0,				// Length of bookmark
									1,				// # rows to fetch
									0,				// Offset
									g_ulColNum,		// Which column to match
									g_lRowLast/2,	// row to match
									S_OK,			// HRESULT to verify
									1				// How many rows to expect.
								);
	if (!COMPARE(fTestPass, TRUE))
		goto CLEANUP;

	fTestPass = CallFindNextRows(	g_pCTable,				// CTable pointer
									NULL,					// bookmark;
									0,						// Length of bookmark
									2,						// # rows to fetch
									0,						// Offset
									g_ulColNum,				// Which column to match
									g_lRowLast/2,			// row to match
									DB_S_ENDOFROWSET,		// HRESULT to verify
									0						// How many rows to expect.
								);
	if (!COMPARE(fTestPass, TRUE))
		goto CLEANUP;

CLEANUP:	
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc pBookmark=NULL, cRows=1, and match 2nd.  Again pBookmark=NULL, cRows=3, match 4th. S_OK and 3 row handles
//
// @rdesc TEST_PASS or TEST_FAIL
//
int No_Properties::Variation_3()
{
	BOOL fTestPass = FALSE;

	//restart the cursor position
	if(!CHECK(RestartRowPosition(),S_OK))
		return TEST_FAIL;

	fTestPass = CallFindNextRows(	g_pCTable,	// CTable pointer
									NULL,			// bookmark;
									0,				// Length of bookmark
									2,				// # rows to fetch
									0,				// Offset
									g_ulColNum,		// Which column to match
									2,				// row to match
									S_OK,			// HRESULT to verify
									2				// How many rows to expect.
								);
	if (!COMPARE(fTestPass, TRUE))
		goto CLEANUP;

	fTestPass = CallFindNextRows(	g_pCTable,				// CTable pointer
									NULL,					// bookmark;
									0,						// Length of bookmark
									3,						// # rows to fetch
									0,						// Offset
									g_ulColNum,				// Which column to match
									4,						// row to match
									S_OK,					// HRESULT to verify
									3						// How many rows to expect.
								);
	if (!COMPARE(fTestPass, TRUE))
		goto CLEANUP;

CLEANUP:	
	return fTestPass;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL No_Properties::Terminate()
{
	ReleaseRowsetAndAccessor();

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIRowsetFind::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(FetchBackwards)
//*-----------------------------------------------------------------------
//| Test Case:		FetchBackwards - Test FetchBackwards property
//|	Created:			06/18/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL FetchBackwards::Init()
{
	DBPROPID	guidProperty;
	ULONG cPrptSet = 0;
	BOOL fTestPass = FALSE;

	if(!TCIRowsetFind::Init())
		return FALSE;

	TESTC_DRIVER(g_rgDBPrpt[IDX_FetchBackwards].fSupported);

	if(!g_rgDBPrpt[IDX_FetchBackwards].fDefault)
	{
		guidProperty=DBPROP_CANFETCHBACKWARDS;
		cPrptSet++;
	}

	//create a rowset and  accessor
	TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetFind,
		cPrptSet,&guidProperty));

	fTestPass = TRUE;

CLEANUP:
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc pBookMark=NULL,cRows=1 and match 1st row.  FindNext, cRows=-1 matching same row. Verify S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int FetchBackwards::Variation_1()
{
	BOOL fTestPass=TEST_FAIL;

	//restart the cursor position
	if(!CHECK(RestartRowPosition(),S_OK))
		return TEST_FAIL;

	fTestPass =  CallFindNextRows(	g_pCTable,	// CTable pointer
									NULL,			// bookmark;
									0,				// Length of bookmark
									1,				// # rows to fetch
									0,			// Offset
									g_ulColNum,		// Which column to match
									1,				// row to match
									S_OK,			// HRESULT to verify
									1				// How many rows to expect.
								);

	if ( !COMPARE(fTestPass,TRUE ) )
		return TEST_FAIL;

	fTestPass = CallFindNextRows(	g_pCTable,			// CTable pointer
									NULL,				// bookmark;
									0,					// Length of bookmark
									-1,					// # rows to fetch
									0,					// skip current row
									g_ulColNum,			// Which column to match
									1,					// row to match
									S_OK,				// HRESULT to verify
									1					// How many rows to expect.
								);

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc pBookMark=NULL, cRows=3 and Match 1st row. FindNext, cRows=-2 and match 3rd row.  Verify S_OK and 3rd and 2nd rows
//
// @rdesc TEST_PASS or TEST_FAIL
//
int FetchBackwards::Variation_2()
{
	BOOL fTestPass = FALSE;
	BYTE		*pBookmark=NULL;

	//restart the cursor position
	if(!CHECK(RestartRowPosition(),S_OK))
		return TEST_FAIL;

	fTestPass = CallFindNextRows(	g_pCTable,	// CTable pointer
									pBookmark,  // bookmark;
									0,				// Length of bookmark
									3,				// # rows to fetch
									0,				// Offset
									g_ulColNum,	// Which column to match
									1,				// row to match
									S_OK,			// HRESULT to verify
									3				// How many rows to expect.
								);

	if ( fTestPass == TEST_FAIL ) return TEST_FAIL;

	fTestPass = CallFindNextRows(	g_pCTable,	// CTable pointer
											pBookmark,  // bookmark;
											0,				// Length of bookmark
											-2,			// # rows to fetch
											0,				// offset
											g_ulColNum,	// Which column to match
											3,		 // row to match
											S_OK,	// HRESULT to verify
											2		// How many rows to expect.
											);

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc pBookmark=NULL,cRows=1 nd match last row. FindNext wiht cRows=-2,fSkip=0 and match 3rd row.  Verify S_OK and hrows.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int FetchBackwards::Variation_3()
{

	BOOL fTestPass = FALSE;
	BYTE		*pBookmark=NULL;

	//restart the cursor position
	if(!CHECK(RestartRowPosition(),S_OK))
		return TEST_FAIL;

	fTestPass = CallFindNextRows(	g_pCTable,	// CTable pointer
											pBookmark,  // bookmark;
											0,				// Length of bookmark
											1,			// # rows to fetch
											0,			// offset
											g_ulColNum,		// Which column to match
											g_lRowLast,		 // row to match
											S_OK,	// HRESULT to verify
											1		// How many rows to expect.
											);

	if ( fTestPass == TEST_FAIL ) return TEST_FAIL;

	fTestPass = CallFindNextRows(	g_pCTable,	// CTable pointer
											pBookmark,  // bookmark;
											0,				// Length of bookmark
											-2,		 // # rows to fetch
											0,		 // offset
											g_ulColNum,		// Which column to match
											3,		 // row to match
											S_OK,	// HRESULT to verify
											2		// How many rows to expect.
											);

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc pBookMark=NULL,cRows=1 and match last row.  FindNext, cRows=-6, match last row.  Verify DB_S_ENDOFROWSET and hrows
//
// @rdesc TEST_PASS or TEST_FAIL
//
int FetchBackwards::Variation_4()
{
	BOOL fTestPass = FALSE;
	BYTE		*pBookmark=NULL;

	//restart the cursor position
	if(!CHECK(RestartRowPosition(),S_OK))
		return TEST_FAIL;

	fTestPass = CallFindNextRows(	g_pCTable,	// CTable pointer
											pBookmark,  // bookmark;
											0,				// Length of bookmark
											1,		 // # rows to fetch
											0, // offset
											g_ulColNum,		// Which column to match
											g_lRowLast,		 // row to match
											S_OK,	// HRESULT to verify
											1		// How many rows to expect.
											);

	if ( fTestPass == TEST_FAIL ) return TEST_FAIL;

	fTestPass = CallFindNextRows(	g_pCTable,	// CTable pointer
											pBookmark,  // bookmark;
											0,				// Length of bookmark
											-(g_lRowLast+1),		 // # rows to fetch
											0,				// offset
											g_ulColNum,		// Which column to match
											g_lRowLast,		 // row to match
											DB_S_ENDOFROWSET,	// HRESULT to verify
											g_lRowLast		// How many rows to expect.
											);

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc pBookmark=NULL, cRows=1, and match last. pBookmark=NULL, cRows=3. Offset=1 and mattch 2nd row.  Verify DB_S_ENDOFROWSET.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int FetchBackwards::Variation_5()
{
	BOOL fTestPass = FALSE;

	//restart the cursor position
	if(!CHECK(RestartRowPosition(),S_OK))
		return TEST_FAIL;

	fTestPass = CallFindNextRows(	g_pCTable,	// CTable pointer
									NULL,  // bookmark;
									0,				// Length of bookmark
									1,		 // # rows to fetch
									0, // offset
									g_ulColNum,		// Which column to match
									1,		 // row to match
									S_OK,	// HRESULT to verify
									1		// How many rows to expect.
								);

	if ( fTestPass == TEST_FAIL ) return TEST_FAIL;

	fTestPass = CallFindNextRows(	g_pCTable,	// CTable pointer
									NULL,		 // bookmark;
									0,			// Length of bookmark
									-3,			 // # rows to fetch
									1, // offset
									g_ulColNum,		// Which column to match
									2,		 // row to match
									DB_S_ENDOFROWSET,	// HRESULT to verify
									2		// How many rows to expect.
								);

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc pBookmark=NULL, cRows=1 and match last.  Loop with cRows=-1 until DB_S_ENDOFROWSET.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int FetchBackwards::Variation_6()
{
	BOOL fTestPass = FALSE;

	//restart the cursor position
	if(!CHECK(RestartRowPosition(),S_OK))
		return TEST_FAIL;

	fTestPass = CallFindNextRows(	g_pCTable,		// CTable pointer
									NULL,			// bookmark;
									0,				// Length of bookmark
									1,				// # rows to fetch
									0,				// offset
									g_ulColNum,		// Which column to match
									g_lRowLast,		// row to match
									S_OK,			// HRESULT to verify
									1				// How many rows to expect.
								);

	if ( fTestPass == TEST_FAIL ) return TEST_FAIL;

	for ( DBROWCOUNT i=g_lRowLast; i>=0 ; i-- )
	{
		fTestPass = CallFindNextRows(	g_pCTable,	// CTable pointer
										NULL,		// bookmark;
										0,			// Length of bookmark
										-1,			// # rows to fetch
										0,			// lOffset
										g_ulColNum,	// Which column to match
										i,		 // row to match
										(i==0 ? DB_S_ENDOFROWSET : S_OK ),	// HRESULT to verify
										(i==0 ? 0 : 1)		// How many rows to expect.
									);

		if (!COMPARE(fTestPass, TEST_PASS))
			return TEST_FAIL;
	}

	return fTestPass;

}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc pBookmark=NULL, Match next to last.  FindNext with cRows=-1 and Offset=2.  Verify DB_S_ENDOFROWSET
//
// @rdesc TEST_PASS or TEST_FAIL
//
int FetchBackwards::Variation_7()
{
	BOOL fTestPass = FALSE;

	//restart the cursor position
	if(!CHECK(RestartRowPosition(),S_OK))
		return TEST_FAIL;

	fTestPass = CallFindNextRows(	g_pCTable,		// CTable pointer
									NULL,			// bookmark;
									0,				// Length of bookmark
									1,				// # rows to fetch
									0,				// offset
									g_ulColNum,		// Which column to match
									g_lRowLast-1,	// row to match
									S_OK,			// HRESULT to verify
									1				// How many rows to expect.
								);

	if ( fTestPass == TEST_FAIL ) return TEST_FAIL;

	fTestPass = CallFindNextRows(	g_pCTable,				// CTable pointer
									NULL,					// bookmark;
									0,						// Length of bookmark
									-1,						// # rows to fetch
									2,						// offset
									g_ulColNum,				// Which column to match
									g_lRowLast,				// row to match
									DB_S_ENDOFROWSET,	// HRESULT to verify
									0						// How many rows to expect.
								);

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc Test that cRows=0 doesn't affect find direction
//
// @rdesc TEST_PASS or TEST_FAIL
//
int FetchBackwards::Variation_8()
{
	BOOL fTestPass = FALSE;

	//restart the cursor position
	if(!CHECK(RestartRowPosition(),S_OK))
		return TEST_FAIL;

	fTestPass = CallFindNextRows(	g_pCTable,		// CTable pointer
									NULL,			// bookmark;
									0,				// Length of bookmark
									-1,			// # rows to fetch
									0,				// offset
									g_ulColNum,	// Which column to match
									g_lRowLast,	// row to match
									S_OK,			// HRESULT to verify
									1				// How many rows to expect.
								);

	if ( fTestPass == TEST_FAIL ) return TEST_FAIL;

	fTestPass = CallFindNextRows(	g_pCTable,		// CTable pointer
									NULL,				// bookmark;
									0,					// Length of bookmark
									0,					// # rows to fetch
									0,					// offset
									g_ulColNum,		// Which column to match
									g_lRowLast-1,	// row to match
									S_OK,				// HRESULT to verify
									0					// How many rows to expect.
								);
	if ( fTestPass == TEST_FAIL ) return TEST_FAIL;

	//cursor is now after g_lRowLast-1
	fTestPass = CallFindNextRows(	g_pCTable,		// CTable pointer
									NULL,				// bookmark;
									0,					// Length of bookmark
									-1,				// # rows to fetch
									0,					// offset
									g_ulColNum,		// Which column to match
									g_lRowLast-2,	// row to match
									S_OK,				// HRESULT to verify
									1					// How many rows to expect.
								);

	return fTestPass;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL FetchBackwards::Terminate()
{
	ReleaseRowsetAndAccessor();

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIRowsetFind::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(ScrollBackwards)
//*-----------------------------------------------------------------------
//| Test Case:		ScrollBackwards - Test ScrollBackwards property
//|	Created:			06/18/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL ScrollBackwards::Init()
{
	DBPROPID	guidPropertySet[1];
	ULONG	cPrptSet=0;
	BOOL fTestPass = FALSE;

	if(!TCIRowsetFind::Init())
		return FALSE;

	TESTC_DRIVER(g_rgDBPrpt[IDX_ScrollBackwards].fSupported);

	if(!g_rgDBPrpt[IDX_ScrollBackwards].fDefault)
	{
		guidPropertySet[cPrptSet++]=DBPROP_CANSCROLLBACKWARDS;
	}

	//create a rowset and an accessor.  
	//DBPROP_CANSCROLLBACKWARDS is requested 
	TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetFind,
		cPrptSet,guidPropertySet));

	fTestPass = TRUE;

CLEANUP:
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc pBookMark=DBBMK_FIRST,cRows=1 and match 2nd row. Verify S_OK and one row handle
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ScrollBackwards::Variation_1()
{
	DBBOOKMARK	DBBookmark=DBBMK_FIRST;
	BYTE		*pBookmark=(BYTE *)&DBBookmark;

	return CallFindNextRows(	g_pCTable,		// CTable pointer
								pBookmark,		// bookmark;
								1,				// Length of bookmark
								1,				// # rows to fetch
								0,				// offset
								g_ulColNum,		// Which column to match
								2,				// row to match
								S_OK,			// HRESULT to verify
								1				// How many rows to expect.
							);
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc pBookMark=DBBMK_LAST,cRows=1 and Match last row.  Verify S_OK and one row handle.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ScrollBackwards::Variation_2()
{
	DBBOOKMARK	DBBookmark=DBBMK_LAST;
	BYTE		*pBookmark=(BYTE *)&DBBookmark;

	return CallFindNextRows(	g_pCTable,		// CTable pointer
								pBookmark,		// bookmark;
								1,				// Length of bookmark
								1,				// # rows to fetch
								0,				// offset
								g_ulColNum,		// Which column to match
								g_lRowLast,		// row to match
								S_OK,			// HRESULT to verify
								1				// How many rows to expect.
							);
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc pBookmark=DBBMK_LAST, cRows=1, and match last.  Verify S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ScrollBackwards::Variation_3()
{
	DBBOOKMARK	DBBookmark=DBBMK_LAST;
	BYTE		*pBookmark=(BYTE *)&DBBookmark;

	return CallFindNextRows(	g_pCTable,		// CTable pointer
								pBookmark,		// bookmark;
								1,				// Length of bookmark
								1,				// # rows to fetch
								1,				// offset
								g_ulColNum,		// Which column to match
								g_lRowLast,		// row to match
								DB_S_ENDOFROWSET,	// HRESULT to verify
								0				// How many rows to expect.
							);

}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc *pBookmark=DBBMK_FIRST, cRows=1, Offset=1.  Verify DB_S_ENDOFROWSET
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ScrollBackwards::Variation_4()
{
	DBBOOKMARK	DBBookmark=DBBMK_LAST;
	BYTE		*pBookmark=(BYTE *)&DBBookmark;

	return CallFindNextRows(	g_pCTable,		// CTable pointer
								pBookmark,		// bookmark;
								1,				// Length of bookmark
								1,				// # rows to fetch
								g_lRowLast,		// offset
								g_ulColNum,		// Which column to match
								g_lRowLast,		// row to match
								DB_S_ENDOFROWSET,	// HRESULT to verify
								0				// How many rows to expect.
							);
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc pBookmark=NULL, cRows=1, and match middle.  FindNext with Offset=-3, cRows=3 and match row preceding middle row.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ScrollBackwards::Variation_5()
{
	BOOL fTestPass = FALSE;

	//restart the cursor position
	if(!CHECK(RestartRowPosition(),S_OK))
		return TEST_FAIL;

	fTestPass = CallFindNextRows(	g_pCTable,		// CTable pointer
									NULL,			// bookmark;
									0,				// Length of bookmark
									1,				// # rows to fetch
									0,				// offset
									g_ulColNum,		// Which column to match
									g_lRowLast/2,	// row to match
									S_OK,			// HRESULT to verify
									1				// How many rows to expect.
								);

	if ( fTestPass == TEST_FAIL ) return TEST_FAIL;

	fTestPass = CallFindNextRows(	g_pCTable,				// CTable pointer
									NULL,					// bookmark;
									0,						// Length of bookmark
									3,						// # rows to fetch
									-3,						// offset
									g_ulColNum,				// Which column to match
									(g_lRowLast/2)-1,		// row to match
									S_OK,	// HRESULT to verify
									3						// How many rows to expect.
								);

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc pBookmark=NULL, cRows=1, and match last.  FindNext with cRows=1, Offset=-1 and match last.  Verify S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ScrollBackwards::Variation_6()
{
	BOOL fTestPass = FALSE;

	//restart the cursor position
	if(!CHECK(RestartRowPosition(),S_OK))
		return TEST_FAIL;

	fTestPass = CallFindNextRows(	g_pCTable,		// CTable pointer
									NULL,			// bookmark;
									0,				// Length of bookmark
									1,				// # rows to fetch
									0,				// offset
									g_ulColNum,		// Which column to match
									g_lRowLast,		// row to match
									S_OK,			// HRESULT to verify
									1				// How many rows to expect.
								);

	if ( fTestPass == TEST_FAIL ) return TEST_FAIL;

	fTestPass = CallFindNextRows(	g_pCTable,				// CTable pointer
									NULL,					// bookmark;
									0,						// Length of bookmark
									1,						// # rows to fetch
									-1,						// offset
									g_ulColNum,				// Which column to match
									g_lRowLast,				// row to match
									S_OK,					// HRESULT to verify
									1						// How many rows to expect.
								);

	return fTestPass;

}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc pBookmark=NULL, cRows=1, Loffset=-# of rows.  Verify DB_S_ENDOFROWSET
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ScrollBackwards::Variation_7()
{
	//restart the cursor position
	if(!CHECK(RestartRowPosition(),S_OK))
		return TEST_FAIL;

	return CallFindNextRows(	g_pCTable,		// CTable pointer
								NULL,			// bookmark;
								0,				// Length of bookmark
								1,				// # rows to fetch
								-g_lRowLast,	// offset
								g_ulColNum,		// Which column to match
								g_lRowLast,		// row to match
								S_OK,	// HRESULT to verify
								1				// How many rows to expect.
							);
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc pBookmark=NULL,cRows=1, Offset=-2.  Verify #of rows-1 row fetched.  Cursor is after next to last row
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ScrollBackwards::Variation_8()
{
	BOOL fTestPass;

	//restart the cursor position
	if(!CHECK(RestartRowPosition(),S_OK))
		return TEST_FAIL;

	fTestPass = CallFindNextRows(	g_pCTable,		// CTable pointer
								NULL,			// bookmark;
								0,				// Length of bookmark
								1,				// # rows to fetch
								-2,				// offset
								g_ulColNum,		// Which column to match
								g_lRowLast-1,	// row to match
								S_OK,			// HRESULT to verify
								1				// How many rows to expect.
							);
	if ( fTestPass == TEST_FAIL ) return TEST_FAIL;

	fTestPass = CallFindNextRows(	g_pCTable,				// CTable pointer
									NULL,					// bookmark;
									0,						// Length of bookmark
									1,						// # rows to fetch
									0,						// offset
									g_ulColNum,				// Which column to match
									g_lRowLast-1,			// row to match
									DB_S_ENDOFROWSET,		// HRESULT to verify
									0						// How many rows to expect.
								);

	return fTestPass;

}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc pBookmark=NULL, cRows=1, Loffset=-#of rows+1
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ScrollBackwards::Variation_9()
{
	//restart the cursor position
	if(!CHECK(RestartRowPosition(),S_OK))
		return TEST_FAIL;

	return CallFindNextRows(	g_pCTable,		// CTable pointer
								NULL,			// bookmark;
								0,				// Length of bookmark
								1,				// # rows to fetch
								-(g_lRowLast+1),	// offset
								g_ulColNum,		// Which column to match
								g_lRowLast,		// row to match
								DB_S_ENDOFROWSET,	// HRESULT to verify
								0				// How many rows to expect.
							);
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL ScrollBackwards::Terminate()
{
	ReleaseRowsetAndAccessor();

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIRowsetFind::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(CanHoldRows)
//*-----------------------------------------------------------------------
//| Test Case:		CanHoldRows - Test DBPROP can hold row property
//|	Created:			06/18/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL CanHoldRows::Init()
{
	DBPROPID	guidProperty;
	ULONG cPrptSet=0;
	BOOL fTestPass = FALSE;

	if(!TCIRowsetFind::Init())
		return FALSE;
	
	TESTC_DRIVER(g_rgDBPrpt[IDX_CanHoldRows].fSupported);

	if(!g_rgDBPrpt[IDX_CanHoldRows].fDefault)
	{
		guidProperty=DBPROP_CANHOLDROWS;
		cPrptSet++;
	}

	//create a rowset and  accessor
	//DBPROP_CANHOLDROWS 
	TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetFind,
		cPrptSet,&guidProperty));

	fTestPass = TRUE;

CLEANUP:
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Get 1,2 row. Get 2,3,4 rows.  S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CanHoldRows::Variation_1()
{
	HROW *phRows1, *phRows2;
	BOOL fTestPass = TRUE;
	
	//restart the cursor position
	if(!CHECK(RestartRowPosition(),S_OK))
		return TEST_FAIL;

	phRows1 = (HROW *) PROVIDER_ALLOC( 2 * sizeof(HROW) );
	phRows2 = (HROW *) PROVIDER_ALLOC( 3 * sizeof(HROW) );

	fTestPass = CallFindNextRows(		
									g_pCTable,		// Table to find from
									NULL,				// Bookmark to fetch from, if any
									0,					// Length of bookmark
									2,					// maps to cRows
									0,					// maps to Offset
									g_ulColNum,		// Column to match
									1,					// Is there a row where the find should happen? 0 - no match
									S_OK,				// Expected HRESULT
									2,					// Expected count of rows
									FALSE,			// flag to Release rows (optional)
									DBCOMPAREOPS_EQ,  // Any particular preference for comparing? (optional)
									SUBOP_EMPTY,	// Some comparisions are rich enough to deserve a mulitple comparision operations
									phRows1			// optional arg if client wants to control row handle mem
									);
	if ( fTestPass == TEST_FAIL ) goto CLEANUP;

	fTestPass = CallFindNextRows(		
									g_pCTable,		// Table to find from
									NULL,				// Bookmark to fetch from, if any
									0,					// Length of bookmark
									3,					// maps to cRows
									0,					// maps to Offset
									g_ulColNum,		// Column to match
									3,					// Is there a row where the find should happen? 0 - no match
									S_OK,				// Expected HRESULT
									3,					// Expected count of rows
									FALSE,			// flag to Release rows (optional)
									DBCOMPAREOPS_EQ,  // Any particular preference for comparing? (optional)
									SUBOP_EMPTY,	// Some comparisions are rich enough to deserve a mulitple comparision operations
									phRows2			// optional arg if client wants to control row handle mem
									);

	COMPARE(VerifyRowPosition(phRows1[0], 1, g_pCTable), TRUE);	
	COMPARE(VerifyRowPosition(phRows1[1], 2, g_pCTable), TRUE);	
	COMPARE(VerifyRowPosition(phRows2[0], 3, g_pCTable), TRUE);	
	COMPARE(VerifyRowPosition(phRows2[1], 4, g_pCTable), TRUE);	
	COMPARE(VerifyRowPosition(phRows2[2], 5, g_pCTable), TRUE);	

	m_pIRowset->ReleaseRows(2, phRows1, NULL, NULL, NULL);
	m_pIRowset->ReleaseRows(3, phRows2, NULL, NULL, NULL);

CLEANUP:
	PROVIDER_FREE(phRows1);
	PROVIDER_FREE(phRows2);
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Get 1,2 rows. Get 3 row.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CanHoldRows::Variation_2()
{
	HROW *phRows1, *phRows2;
	BOOL fTestPass = TRUE;
	
	//restart the cursor position
	if(!CHECK(RestartRowPosition(),S_OK))
		return TEST_FAIL;

	phRows1 = (HROW *) PROVIDER_ALLOC( 2 * sizeof(HROW) );
	phRows2 = (HROW *) PROVIDER_ALLOC( sizeof(HROW) );

	fTestPass = CallFindNextRows(		
									g_pCTable,		// Table to find from
									NULL,				// Bookmark to fetch from, if any
									0,					// Length of bookmark
									2,					// maps to cRows
									0,					// maps to Offset
									g_ulColNum,		// Column to match
									1,					// Is there a row where the find should happen? 0 - no match
									S_OK,				// Expected HRESULT
									2,					// Expected count of rows
									FALSE,			// flag to Release rows (optional)
									DBCOMPAREOPS_EQ,  // Any particular preference for comparing? (optional)
									SUBOP_EMPTY,	// Some comparisions are rich enough to deserve a mulitple comparision operations
									phRows1			// optional arg if client wants to control row handle mem
									);
	if ( fTestPass == TEST_FAIL ) goto CLEANUP;

	fTestPass = CallFindNextRows(		
									g_pCTable,		// Table to find from
									NULL,				// Bookmark to fetch from, if any
									0,					// Length of bookmark
									1,					// maps to cRows
									0,					// maps to Offset
									g_ulColNum,		// Column to match
									3,					// Is there a row where the find should happen? 0 - no match
									S_OK,				// Expected HRESULT
									1,					// Expected count of rows
									FALSE,			// flag to Release rows (optional)
									DBCOMPAREOPS_EQ,  // Any particular preference for comparing? (optional)
									SUBOP_EMPTY,	// Some comparisions are rich enough to deserve a mulitple comparision operations
									phRows2			// optional arg if client wants to control row handle mem
									);

	COMPARE(VerifyRowPosition(phRows1[0], 1, g_pCTable), TRUE);	
	COMPARE(VerifyRowPosition(phRows1[1], 2, g_pCTable),TRUE);	
	COMPARE(VerifyRowPosition(phRows2[0], 3, g_pCTable),TRUE);	

	m_pIRowset->ReleaseRows(2, phRows1, NULL, NULL, NULL);
	m_pIRowset->ReleaseRows(1, phRows2, NULL, NULL, NULL);

CLEANUP:
	PROVIDER_FREE(phRows1);
	PROVIDER_FREE(phRows2);
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Get all rows with GetNextRows.  RestartPosition. FindNext with pBookmark=NULL, Offset=1, cRows=1 and match 2nd. Verify S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CanHoldRows::Variation_3()
{
	BOOL		fTestPass = FALSE;
	HROW *		phrow = NULL;
	DBCOUNTITEM	cRowsObtained = 0;
	
	//restart the cursor position
	if(!CHECK(RestartRowPosition(),S_OK))
		return TEST_FAIL;

	if (!CHECK(m_pIRowset->GetNextRows(DB_NULL_HCHAPTER, 0, g_lRowLast, &cRowsObtained, (HROW **)&phrow ), S_OK) )
		goto CLEANUP;

	if (!COMPARE(cRowsObtained, ULONG(g_lRowLast)))
		goto CLEANUP;

	if (!COMPARE(VerifyRowPosition(phrow[0], 1, g_pCTable), TRUE))
		goto CLEANUP;

	//restart the cursor position
	if(!CHECK(RestartRowPosition(),S_OK))
		return TEST_FAIL;

	fTestPass =  CallFindNextRows(	g_pCTable,		// CTable pointer
												NULL,				// bookmark;
												0,					// Length of bookmark
												1,					// # rows to fetch
												0,					// offset
												g_ulColNum,		// Which column to match
												2,					// row to match
												S_OK,				// HRESULT to verify
												1					// How many rows to expect.
												);

CLEANUP:

	PROVIDER_FREE(phrow);
	return fTestPass;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL CanHoldRows::Terminate()
{
	ReleaseRowsetAndAccessor();

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIRowsetFind::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(Scroll_and_Fetch)
//*-----------------------------------------------------------------------
//| Test Case:		Scroll_and_Fetch - Test in context of CANSCROLLBACKWARDS and CANFETCHBACKWARDS
//|	Created:			06/18/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Scroll_and_Fetch::Init()
{
	DBPROPID	guidPropertySet[2];
	ULONG		cPrptSet=0;
	BOOL		fTestPass = FALSE;

	if(!TCIRowsetFind::Init())
		return FALSE;

	TESTC_DRIVER(g_rgDBPrpt[IDX_FetchBackwards].fSupported &&
	   g_rgDBPrpt[IDX_ScrollBackwards].fSupported );

	if(!g_rgDBPrpt[IDX_FetchBackwards].fDefault)
	{
		guidPropertySet[cPrptSet]=DBPROP_CANFETCHBACKWARDS;
		cPrptSet++;
	}

	if(!g_rgDBPrpt[IDX_ScrollBackwards].fDefault)
	{
		guidPropertySet[cPrptSet]=DBPROP_CANSCROLLBACKWARDS;
		cPrptSet++;
	}

	//create a rowset and an accessor.  
	//DBPROP_CANFETCHBACKWARDS and DBPROP_CANSCROLLBACKWARDS 
	//are requested 
	TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetFind,
		cPrptSet,guidPropertySet));

	fTestPass = TRUE;

CLEANUP:
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc pBookmark=DBBMK_FIRST,cRows=-1.  Verify DB_S_ENDOFROWSET and no hrows.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Scroll_and_Fetch::Variation_1()
{
	DBBOOKMARK	DBBookmark=DBBMK_FIRST;
	BYTE		*pBookmark=(BYTE *)&DBBookmark;

	return CallFindNextRows(	g_pCTable,			// CTable pointer
										pBookmark,			// bookmark;
										1,						// Length of bookmark
										-1,				  // # rows to fetch
										0,						// offset
										g_ulColNum,			// Which column to match
										0,						// row to match
										DB_S_ENDOFROWSET,	// HRESULT to verify
										0						// How many rows to expect.
									);
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc pBookMark=DBBMK_FIRST,cRows=2  Match 3rd row.  Verify S_OK and hrows
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Scroll_and_Fetch::Variation_2()
{
	DBBOOKMARK	DBBookmark=DBBMK_FIRST;
	BYTE		*pBookmark=(BYTE *)&DBBookmark;

	return CallFindNextRows(	g_pCTable,			// CTable pointer
										pBookmark,			// bookmark;
										1,						// Length of bookmark
										2,					  // # rows to fetch
										0,						// offset
										g_ulColNum,			// Which column to match
										3,						// row to match
										S_OK,					// HRESULT to verify
										2						// How many rows to expect.
									);
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc pBookMark=DBBMK_LAST,cRows=-2. Match 3rd row.  Verify S_OK and 3rd and 2nd hrows
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Scroll_and_Fetch::Variation_3()
{
	DBBOOKMARK	DBBookmark=DBBMK_LAST;
	BYTE		*pBookmark=(BYTE *)&DBBookmark;

	return CallFindNextRows(	g_pCTable,			// CTable pointer
										pBookmark,			// bookmark;
										1,						// Length of bookmark
										-2,				  // # rows to fetch
										0,					// offset
										g_ulColNum,			// Which column to match
										3,						// row to match
										S_OK,	// HRESULT to verify
										2						// How many rows to expect.
									);
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc pBookMark=DBBMK_LAST,cRows=-# rows. Match last row.  Verify DB_S_ENDOFROWSET and all rows
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Scroll_and_Fetch::Variation_4()
{
	DBBOOKMARK	DBBookmark=DBBMK_LAST;
	BYTE		*pBookmark=(BYTE *)&DBBookmark;

	return CallFindNextRows(	g_pCTable,			// CTable pointer
										pBookmark,			// bookmark;
										1,						// Length of bookmark
										-(g_lRowLast+1),				  // # rows to fetch
										0,						// offset
										g_ulColNum,			// Which column to match
										g_lRowLast,						// row to match
										DB_S_ENDOFROWSET,	// HRESULT to verify
										g_lRowLast			// How many rows to expect.
									);
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc pBookMark=DBBMK_FIRST, cRows=-1. Match first row.  Verify S_OK and one row handle.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Scroll_and_Fetch::Variation_5()
{
	DBBOOKMARK	DBBookmark=DBBMK_FIRST;
	BYTE		*pBookmark=(BYTE *)&DBBookmark;

	return CallFindNextRows(	g_pCTable,			// CTable pointer
										pBookmark,			// bookmark;
										1,						// Length of bookmark
										-1,				  // # rows to fetch
										0,						// offset
										g_ulColNum,			// Which column to match
										1,						// row to match
										S_OK,					// HRESULT to verify
										1						// How many rows to expect.
									);
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc pBookMark=DBBMK_FIRST,cRows=1. Match last row.  FindNext, cRows=1, Offset=1 and match last. then cRows=-3, match 4th
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Scroll_and_Fetch::Variation_6()
{
	BOOL fTestPass = TRUE;
	DBBOOKMARK	DBBookmark=DBBMK_FIRST;
	BYTE		*pBookmark=(BYTE *)&DBBookmark;

	fTestPass = CallFindNextRows(	g_pCTable,			// CTable pointer
											pBookmark,			// bookmark;
											1,						// Length of bookmark
											1,				  // # rows to fetch
											FALSE,					// skip current row
											g_ulColNum,			// Which column to match
											g_lRowLast,						// row to match
											S_OK,					// HRESULT to verify
											1						// How many rows to expect.
										);
	if ( fTestPass == TEST_FAIL ) return TEST_FAIL;

	// next fetch position is still at start of rowset.
	fTestPass = CallFindNextRows(	g_pCTable,			// CTable pointer
											NULL,					// bookmark;
											0,						// Length of bookmark
											1,					   // # rows to fetch
											0,					 	// offset
											g_ulColNum,			// Which column to match
											g_lRowLast,			// row to match
											S_OK,					// HRESULT to verify
											1						// How many rows to expect.
										);
	if ( fTestPass == TEST_FAIL ) return TEST_FAIL;

	fTestPass = CallFindNextRows(	g_pCTable,			// CTable pointer
											NULL,			// bookmark;
											0,						// Length of bookmark
											-3,				  // # rows to fetch
											0,						// offset
											g_ulColNum,			// Which column to match
											4,						// row to match
											S_OK,					// HRESULT to verify
											3						// How many rows to expect.
										);
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc pBookmark=DBBMK_LAST,cRows=1.  Match last row.  Call FindNext, cRows=-3, pBookmark=NULL.  Match 4th row, S_OK.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Scroll_and_Fetch::Variation_7()
{
	BOOL fTestPass = TRUE;

	//restart the cursor position
	if(!CHECK(RestartRowPosition(),S_OK))
		return TEST_FAIL;

	fTestPass = CallFindNextRows(	g_pCTable,			// CTable pointer
											NULL,					// bookmark;
											0,						// Length of bookmark
											1,					   // # rows to fetch
											0,						// offset
											g_ulColNum,			// Which column to match
											g_lRowLast,			// row to match
											S_OK,					// HRESULT to verify
											1						// How many rows to expect.
										);
	if ( fTestPass == TEST_FAIL ) return TEST_FAIL;

	fTestPass = CallFindNextRows(	g_pCTable,			// CTable pointer
											NULL,					// bookmark;
											0,						// Length of bookmark
											-3,				  // # rows to fetch
											0,						// offset
											g_ulColNum,			// Which column to match
											4,						// row to match
											S_OK,					// HRESULT to verify
											3						// How many rows to expect.
										);
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc *pBookmark=DBBMK_LAST, Offset=-2, cRows=-3.  Verify row starting with N-4 fetched in traversal order
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Scroll_and_Fetch::Variation_8()
{
	DBBOOKMARK	DBBookmark=DBBMK_LAST;
	BYTE		*pBookmark=(BYTE *)&DBBookmark;

	return CallFindNextRows(	g_pCTable,			// CTable pointer
										pBookmark,			// bookmark;
										1,						// Length of bookmark
										-1,				   // # rows to fetch
										-2,					// offset
										g_ulColNum,			// Which column to match
										g_lRowLast-4,		// row to match
										S_OK,					// HRESULT to verify
										1						// How many rows to expect.
									);
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc pBookmark=NULL, Offset=-2, cRows=-1.  Verify N-2 row is fetched and fetch position is after N-3 row
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Scroll_and_Fetch::Variation_9()
{
	//restart the cursor position
	if(!CHECK(RestartRowPosition(),S_OK))
		return TEST_FAIL;

	return CallFindNextRows(	g_pCTable,			// CTable pointer
										NULL,					// bookmark;
										0,						// Length of bookmark
										-1,			  	   // # rows to fetch
										-2,					// offset
										g_ulColNum,			// Which column to match
										g_lRowLast-2,		// row to match
										S_OK,					// HRESULT to verify
										1						// How many rows to expect.
									);
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc pBookmark=NULL, Offset=-4, cRows=2 and match last. Verify DB_S_END. Again cRows=-1 and no match.  Again cRows=1 and match 1st.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Scroll_and_Fetch::Variation_10()
{
	BOOL fTestPass;

	//restart the cursor position
	if(!CHECK(RestartRowPosition(),S_OK))
		return TEST_FAIL;

	fTestPass = CallFindNextRows(	g_pCTable,			// CTable pointer
										NULL,					// bookmark;
										0,						// Length of bookmark
										2,				  	   // # rows to fetch
										-4,					// offset
										g_ulColNum,			// Which column to match
										g_lRowLast,			// row to match
										DB_S_ENDOFROWSET,	// HRESULT to verify
										1						// How many rows to expect.
									);
	if ( !COMPARE(fTestPass, TRUE) ) 
		goto CLEANUP;

	fTestPass = CallFindNextRows(	g_pCTable,			// CTable pointer
										NULL,					// bookmark;
										0,						// Length of bookmark
										-1,			  	   // # rows to fetch
										0,						// offset
										g_ulColNum,			// Which column to match
										0,						// row to match
										DB_S_ENDOFROWSET,	// HRESULT to verify
										0						// How many rows to expect.
									);
	if ( !COMPARE(fTestPass, TRUE) ) 
		goto CLEANUP;

	fTestPass = CallFindNextRows(	g_pCTable,			// CTable pointer
										NULL,					// bookmark;
										0,						// Length of bookmark
										1,				  	   // # rows to fetch
										0,						// offset
										g_ulColNum,			// Which column to match
										1,						// row to match
										S_OK,					// HRESULT to verify
										1						// How many rows to expect.
									);
	if ( !COMPARE(fTestPass, TRUE) ) 
		goto CLEANUP;

CLEANUP:
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc pBookmark=NULL, cRows=3 and match last. DB_S_END.  Again with cRows=2, verify DB_S_END.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Scroll_and_Fetch::Variation_11()
{
	BOOL fTestPass;

	//restart the cursor position
	if(!CHECK(RestartRowPosition(),S_OK))
		return TEST_FAIL;

	fTestPass = CallFindNextRows(	g_pCTable,			// CTable pointer
										NULL,					// bookmark;
										0,						// Length of bookmark
										3,				  	   // # rows to fetch
										0,						// offset
										g_ulColNum,			// Which column to match
										g_lRowLast,			// row to match
										DB_S_ENDOFROWSET,	// HRESULT to verify
										1						// How many rows to expect.
									);
	if ( !COMPARE(fTestPass, TRUE) ) 
		goto CLEANUP;

	fTestPass = CallFindNextRows(	g_pCTable,			// CTable pointer
										NULL,					// bookmark;
										0,						// Length of bookmark
										2,				  	   // # rows to fetch
										0,						// offset
										g_ulColNum,			// Which column to match
										g_lRowLast,			// row to match
										DB_S_ENDOFROWSET,	// HRESULT to verify
										0						// How many rows to expect.
									);
	if ( !COMPARE(fTestPass, TRUE) ) 
		goto CLEANUP;

CLEANUP:
	return fTestPass;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Scroll_and_Fetch::Terminate()
{
	ReleaseRowsetAndAccessor();

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIRowsetFind::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(Boundary)
//*-----------------------------------------------------------------------
//| Test Case:		Boundary - Test invalid arguments
//|	Created:			06/18/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Boundary::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCIRowsetFind::Init())
	// }}
	{
		// TO DO:  Add your own code here
		return TRUE;
	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc DB_E_BADCHAPTER
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Boundary::Variation_1()
{
	return TEST_SKIPPED;
#if 0
	BOOL fTestPass = TRUE;
	DBCOUNTITEM cRowsObtained;
	HROW *rghRows = NULL;
	HCHAPTER hBadChapter;

	memset(&hBadChapter, 0xCA, sizeof(HCHAPTER));

	//create a rowset and an accessor.  
	TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetFind,
		0,NULL));
	
	//setup
	TESTC(CreateFindValueAccessor(DBCOMPAREOPS_EQ, g_pCTable, 1, g_ulColNum, g_wColType, SUBOP_EMPTY));

	m_hr = m_pIRowsetFind->FindNextRow(hBadChapter, m_hRowsetFindAccessor, m_pFindValue, 
													DBCOMPAREOPS_EQ, 0, NULL, FALSE, 
													1, &cRowsObtained, &rghRows);

	// Verify HRESULT
	if ( !COMPARE(m_hr,DB_E_BADCHAPTER) )
	{
		fTestPass = TEST_FAIL;
		goto CLEANUP;
	}

CLEANUP:
	PROVIDER_FREE(rghRows);
	ReleaseFindValueAccessor(g_wColType);
	ReleaseRowsetAndAccessor();

	return fTestPass;	
#endif
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG, cbBookmark != 0, pBookmark = NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Boundary::Variation_2()
{
	BOOL fTestPass = TRUE;

	//create a rowset and an accessor.  
	TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable, SELECT_ALLFROMTBL, IID_IRowsetFind,
		0,NULL));
	
	fTestPass = CallFindNextRows(	g_pCTable,		// CTable pointer
									NULL,			// bookmark;
									MAXDBCOUNTITEM,	// Length of bookmark
									1,				// # rows to fetch
									FALSE,			// skip current row
									g_ulColNum,		// Which column to match
									g_lRowLast,		// row to match
									E_INVALIDARG,	// HRESULT to verify
									0				// How many rows to expect.									
								);

CLEANUP:
	ReleaseRowsetAndAccessor();

	return fTestPass;	
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG, pcRowsObtained = NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Boundary::Variation_3()
{
	BOOL fTestPass = TRUE;
	HROW *rghRows = NULL;

	//create a rowset and an accessor.  
	TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable, SELECT_ALLFROMTBL, IID_IRowsetFind,
		0,NULL));
	
	//setup
	TESTC(CreateFindValueAccessor(DBCOMPAREOPS_EQ, g_pCTable, 1, g_ulColNum, g_wColType, SUBOP_EMPTY));

	m_hr = m_pIRowsetFind->FindNextRow(DB_NULL_HCHAPTER, m_hRowsetFindAccessor, m_pFindValue, 
													DBCOMPAREOPS_EQ, 0, NULL, FALSE, 
													1, NULL, &rghRows);

	// Verify HRESULT
	if ( !COMPARE(m_hr,E_INVALIDARG) )
	{
		fTestPass = TEST_FAIL;
		goto CLEANUP;
	}

CLEANUP:
	PROVIDER_FREE(rghRows);
	ReleaseFindValueAccessor(g_wColType);
	ReleaseRowsetAndAccessor();

	return fTestPass;	
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG prghRows = NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Boundary::Variation_4()
{
	BOOL fTestPass = TRUE;
	DBCOUNTITEM cRowsObtained = 0;

	//create a rowset and an accessor.  
	TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable, SELECT_ALLFROMTBL, IID_IRowsetFind,
		0,NULL));
	
	//setup
	TESTC(CreateFindValueAccessor(DBCOMPAREOPS_EQ, g_pCTable, 1, g_ulColNum, g_wColType, SUBOP_EMPTY));

	m_hr = m_pIRowsetFind->FindNextRow(DB_NULL_HCHAPTER, m_hRowsetFindAccessor, m_pFindValue, 
													DBCOMPAREOPS_EQ, 0, NULL, FALSE, 
													1, &cRowsObtained, NULL);

	// Verify HRESULT
	if ( !COMPARE(m_hr,E_INVALIDARG) )
	{
		fTestPass = TEST_FAIL;
		goto CLEANUP;
	}

CLEANUP:
	ReleaseFindValueAccessor(g_wColType);
	ReleaseRowsetAndAccessor();

	return fTestPass;	
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc DB_E_COMPAREOPS, CompareOp = -1 (<min
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Boundary::Variation_5()
{
	BOOL		fTestPass = TEST_PASS;
	DBCOUNTITEM cRowsObtained = 0;
	HROW *		rghRows = NULL;
	DBROWCOUNT	lRowsToFetch = 0;

	//create a rowset and an accessor.  
	TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable, SELECT_ALLFROMTBL, IID_IRowsetFind,
		0,NULL));
	
	//setup
	TESTC(fTestPass=CreateFindValueAccessor(DBCOMPAREOPS_EQ, g_pCTable, 1, g_ulColNum, g_wColType, SUBOP_EMPTY));

	m_hr = m_pIRowsetFind->FindNextRow(DB_NULL_HCHAPTER, m_hRowsetFindAccessor, m_pFindValue, 
													-1, 0, NULL, FALSE, 
													lRowsToFetch, &cRowsObtained, &rghRows);

	if (!CHECK(m_hr, DB_E_BADCOMPAREOP))
		fTestPass = TEST_FAIL;

CLEANUP:
	ReleaseFindValueAccessor(g_wColType);
	ReleaseRowsetAndAccessor();

	return fTestPass;		
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc DB_E_BADCOMPAREOP, CompareOp > max
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Boundary::Variation_6()
{
	BOOL		fTestPass = TEST_PASS;
	DBCOUNTITEM cRowsObtained = 0;
	HROW *		rghRows = NULL;
	DBROWCOUNT	lRowsToFetch = 0;

	//create a rowset and an accessor.  
	TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable, SELECT_ALLFROMTBL, IID_IRowsetFind,
		0,NULL));
	
	//setup
	TESTC(fTestPass==CreateFindValueAccessor(DBCOMPAREOPS_EQ, g_pCTable, 1, g_ulColNum, g_wColType, SUBOP_EMPTY));

	m_hr = m_pIRowsetFind->FindNextRow(DB_NULL_HCHAPTER, m_hRowsetFindAccessor, m_pFindValue, 
													DBCOMPAREOPS_NOTCONTAINS+1, 0, NULL, FALSE, 
													lRowsToFetch, &cRowsObtained, &rghRows);

	if (!CHECK(m_hr, DB_E_BADCOMPAREOP))
		fTestPass = TEST_FAIL;

CLEANUP:
	ReleaseFindValueAccessor(g_wColType);
	ReleaseRowsetAndAccessor();

	return fTestPass;	
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc DB_E_BADCOMPAREOP, unsupported
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Boundary::Variation_7()
{
	BOOL fTestPass=TRUE;
	CCol TempCol;
	DBORDINAL ulColNum = g_pCTable->CountColumnsOnTable();
	DBCOUNTITEM ulRowCount = g_pCTable->GetRowsOnCTable();
	DBORDINAL ulColIndex;

	//create a rowset and an accessor.  
	TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable, SELECT_ALLFROMTBL, IID_IRowsetFind,
		0,NULL));

	for (ulColIndex = 1; ulColIndex <= ulColNum; ulColIndex++ )
		{			
			g_pCTable->GetColInfo(ulColIndex, TempCol);
			for ( DWORD CompareOp = 0; CompareOp <= DBCOMPAREOPS_IGNORE; CompareOp++ )
			{
				//skip non-updable columns as we ca not call MakeData for them
				if (TempCol.GetUpdateable() && 	!IsColumnMinimumFindable(&TempCol, CompareOp) )
				{
					fTestPass = 
						CallFindNextRows(	g_pCTable,	// CTable pointer
											NULL,			 // bookmark;
											0,				// Length of bookmark
											1,				// # rows to fetch
											TRUE,			// skip current row
											ulColIndex,				// Which column to match
											1,				// row to match
											DB_E_BADCOMPAREOP,	// HRESULT to verify
											0,				// How many rows to expect.
											TRUE,			//fReleaseRows
											CompareOp	// Specifically ask for a compare Op
										);
					 if ( fTestPass == TEST_FAIL ) 
					 {
						odtLog<<"Error at ColName "<< TempCol.GetColName()<<L", ColIndex "<<ulColIndex<<L"; column type: "<<TempCol.GetProviderType()<<L"; CompareOp: "<<CompareOp<< ENDL;
						odtLog<<"-------------------------------------------------------------"<< ENDL;
						break;
					}
				}
			}
		}

CLEANUP:
	ReleaseRowsetAndAccessor();

	return fTestPass;		
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc DB_E_BADBINDINFO, NULL hAccessor
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Boundary::Variation_8()
{
	BOOL fTestPass = TRUE;
	DBCOUNTITEM cRowsObtained = 0;
	HROW *rghRows = NULL;
	HRESULT hr = 0;

	//create a rowset and an accessor.  
	TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable, SELECT_ALLFROMTBL, IID_IRowsetFind,
		0,NULL));
	
	// Lets create the accessor.
	if ( FAILED(hr= m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA, 0, NULL, sizeof(DBLENGTH)+sizeof(DBSTATUS), 
						&m_hRowsetFindAccessor, NULL)) )
	{
		if(hr==DB_E_NULLACCESSORNOTSUPPORTED)
			TESTC_DRIVER(FALSE); //skip if null accessor is not supported
		fTestPass = TEST_FAIL;
		goto CLEANUP;
	}

	// Check that for DBCOMPAREOPS_IGNORE the accessor and pData are ignored.
	m_hr = m_pIRowsetFind->FindNextRow(DB_NULL_HCHAPTER, m_hRowsetFindAccessor, NULL, 
													DBCOMPAREOPS_IGNORE, 0, NULL, FALSE, 
													1, &cRowsObtained, &rghRows);

	// Verify HRESULT
	if ( !CHECK(m_hr,S_OK) )
	{
		fTestPass = TEST_FAIL;
		goto CLEANUP;
	}

	if (!COMPARE(cRowsObtained,1))
	{
		fTestPass = TEST_FAIL;
		goto CLEANUP;
	}

CLEANUP:
	ReleaseRowsetAndAccessor();

	return fTestPass;	
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc DB_E_BADBINDINFO, 2 columns bound
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Boundary::Variation_9()
{
	// Not an interesting test.
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc DB_E_BADBINDINFO, many columns bound
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Boundary::Variation_10()
{
	BOOL fTestPass = TEST_FAIL;
	DBCOUNTITEM cRowsObtained = 0;
	HROW *rghRows = NULL;

	//create a rowset and an accessor.  
	TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable, SELECT_ALLFROMTBL, IID_IRowsetFind,
		0,NULL));
	
	// use accessor from GetRowsetAndAccessor
	m_hr = m_pIRowsetFind->FindNextRow(DB_NULL_HCHAPTER, m_hAccessor, m_pData, 
													DBCOMPAREOPS_EQ, 0, NULL, FALSE, 
													1, &cRowsObtained, &rghRows);

	// Verify HRESULT
	if ( !CHECK(m_hr,DB_E_BADBINDINFO) )	
		goto CLEANUP;

	if ( !COMPARE(cRowsObtained, 0) )
		goto CLEANUP;
	
	if ( !COMPARE(rghRows, NULL) )
		goto CLEANUP;

	fTestPass = TEST_PASS;

CLEANUP:
	ReleaseRowsetAndAccessor();

	return fTestPass;	
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc DB_E_CANTSCROLLBACKWARDS.  lRowsOffset<0
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Boundary::Variation_11()
{
	BOOL fTestPass = TRUE;

	if(g_rgDBPrpt[IDX_ScrollBackwards].fSupported && g_rgDBPrpt[IDX_ScrollBackwards].fDefault)
	{
		return TEST_SKIPPED;
	}

	//create a rowset and an accessor.  
	TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable, SELECT_ALLFROMTBL, IID_IRowsetFind,
		0,NULL));
	
	fTestPass = CallFindNextRows(	g_pCTable,	// CTable pointer
									NULL,  // bookmark;
									0,	// Length of bookmark
									2,		 // # rows to fetch
									-1, // skip current row
									g_ulColNum,		// Which column to match
									1,	 // row to match
									DB_E_CANTSCROLLBACKWARDS,	// HRESULT to verify
									0		// How many rows to expect.
								);

CLEANUP:
	ReleaseRowsetAndAccessor();

	return fTestPass;		
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc DB_E_CANTSCROLLBACKWARDS
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Boundary::Variation_12()
{
	BOOL fTestPass = TRUE;
	DBBOOKMARK	DBBookmark=DBBMK_FIRST;
	BYTE		*pBookmark=(BYTE *)&DBBookmark;

	if(g_rgDBPrpt[IDX_ScrollBackwards].fSupported && g_rgDBPrpt[IDX_ScrollBackwards].fDefault)
	{
		return TEST_SKIPPED;
	}

	//create a rowset and an accessor.  
	TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable, SELECT_ALLFROMTBL, IID_IRowsetFind,
		0,NULL));
	
	fTestPass = CallFindNextRows(	g_pCTable,	// CTable pointer
									pBookmark,  // bookmark;
									1,	// Length of bookmark
									1,		 // # rows to fetch
									FALSE, // skip current row
									g_ulColNum,		// Which column to match
									1,	 // row to match
									DB_E_CANTSCROLLBACKWARDS,	// HRESULT to verify
									0		// How many rows to expect.
								);

CLEANUP:
	ReleaseRowsetAndAccessor();

	return fTestPass;		
}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc DB_E_CANTSCROLLBACKWARDS, pBookmark = DBBMK_FIRST
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Boundary::Variation_13()
{
	BOOL fTestPass = TRUE;
	DBBOOKMARK	DBBookmark=DBBMK_FIRST;
	BYTE		*pBookmark=(BYTE *)&DBBookmark;

	if(g_rgDBPrpt[IDX_ScrollBackwards].fSupported && g_rgDBPrpt[IDX_ScrollBackwards].fDefault)
	{
		return TEST_SKIPPED;
	}

	//create a rowset and an accessor.  
	TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable, SELECT_ALLFROMTBL, IID_IRowsetFind,
		0,NULL));
	
	fTestPass = CallFindNextRows(	g_pCTable,	// CTable pointer
									pBookmark,  // bookmark;
									1,	// Length of bookmark
									1,		 // # rows to fetch
									FALSE, // skip current row
									g_ulColNum,		// Which column to match
									1,	 // row to match
									DB_E_CANTSCROLLBACKWARDS,	// HRESULT to verify
									0		// How many rows to expect.
								);

CLEANUP:
	ReleaseRowsetAndAccessor();

	return fTestPass;		
}
// }}


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc DB_E_CANTSCROLLBACKWARDS, pBokmark = DBBMK_LAST
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Boundary::Variation_14()
{
	BOOL fTestPass = TRUE;
	DBBOOKMARK	DBBookmark=DBBMK_LAST;
	BYTE		*pBookmark=(BYTE *)&DBBookmark;

	if(g_rgDBPrpt[IDX_ScrollBackwards].fSupported && g_rgDBPrpt[IDX_ScrollBackwards].fDefault)
	{
		return TEST_SKIPPED;
	}

	//create a rowset and an accessor.  
	TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable, SELECT_ALLFROMTBL, IID_IRowsetFind,
		0,NULL));
	
	fTestPass = CallFindNextRows(	g_pCTable,	// CTable pointer
									pBookmark,  // bookmark;
									1,	// Length of bookmark
									1,		 // # rows to fetch
									FALSE, // skip current row
									g_ulColNum,		// Which column to match
									1,	 // row to match
									DB_E_CANTSCROLLBACKWARDS,	// HRESULT to verify
									0		// How many rows to expect.
								);

CLEANUP:
	ReleaseRowsetAndAccessor();

	return fTestPass;		
}
// }}


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc DB_E_CANTFETCHBACKWARDS
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Boundary::Variation_15()
{
	BOOL fTestPass = TRUE;

	if(g_rgDBPrpt[IDX_FetchBackwards].fSupported && g_rgDBPrpt[IDX_FetchBackwards].fDefault)
	{
		return TEST_SKIPPED;
	}

	//create a rowset and an accessor.  
	TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable, SELECT_ALLFROMTBL, IID_IRowsetFind,
		0,NULL));
	
	fTestPass = CallFindNextRows(	g_pCTable,	// CTable pointer
									NULL,  // bookmark;
									0,	// Length of bookmark
									-1,		 // # rows to fetch
									FALSE, // skip current row
									g_ulColNum,		// Which column to match
									1,	 // row to match
									DB_E_CANTFETCHBACKWARDS,	// HRESULT to verify
									0		// How many rows to expect.
								);

CLEANUP:
	ReleaseRowsetAndAccessor();

	return fTestPass;		
}
// }}


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc DB_E_BADCOMPAREOP, IGNORE and CASESENSITIVE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Boundary::Variation_16()
{
	BOOL		fTestPass = TRUE;
	DBCOUNTITEM	cRowsObtained = 0;
	HROW *		rghRows = NULL;
	DBROWCOUNT	lRowsToFetch = 1;
	CCol		TempCol;
	
	g_pCTable->GetColInfo(g_ulColNum, TempCol);
	if(!IsColumnMinimumFindable(&TempCol, DBCOMPAREOPS_IGNORE | DBCOMPAREOPS_CASESENSITIVE))
		return TEST_SKIPPED;

	//create a rowset and an accessor.  
	TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable, SELECT_ALLFROMTBL, IID_IRowsetFind,
		0,NULL));
	
	//setup
	TESTC(CreateFindValueAccessor(DBCOMPAREOPS_EQ, g_pCTable, 1, g_ulColNum, g_wColType, SUBOP_EMPTY));

	m_hr = m_pIRowsetFind->FindNextRow(DB_NULL_HCHAPTER, m_hRowsetFindAccessor, m_pFindValue, 
													DBCOMPAREOPS_IGNORE | DBCOMPAREOPS_CASESENSITIVE, 0, NULL, FALSE, 
													lRowsToFetch, &cRowsObtained, &rghRows);

	// should be treated as an DBCOMPAREOPS_IGNORE
	if (!CHECK(m_hr, S_OK))
		fTestPass = TEST_FAIL;

	// Verify cRowsObtained
	if ( !COMPARE(cRowsObtained,1) )
	{
		fTestPass = TEST_FAIL;
		goto CLEANUP;
	}

	//call VerifyRowPosition
	if(!COMPARE(VerifyRowPosition(rghRows[0],1,g_pCTable),TRUE))
		fTestPass=TEST_FAIL;
	

CLEANUP:
	ReleaseFindValueAccessor(g_wColType);
	ReleaseRowsetAndAccessor();

	return fTestPass;	
}
// }}


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc DB_E_BADCOMPAREOP, just CASESENSITIVE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Boundary::Variation_17()
{
	BOOL		fTestPass = TRUE;
	DBCOUNTITEM cRowsObtained = 0;
	HROW *		rghRows = NULL;
	DBROWCOUNT	lRowsToFetch = 0;

	//create a rowset and an accessor.  
	TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable, SELECT_ALLFROMTBL, IID_IRowsetFind,
		0,NULL));
	
	//setup
	TESTC(CreateFindValueAccessor(DBCOMPAREOPS_EQ, g_pCTable, 1, g_ulColNum, g_wColType, SUBOP_EMPTY));

	m_hr = m_pIRowsetFind->FindNextRow(DB_NULL_HCHAPTER, m_hRowsetFindAccessor, m_pFindValue, 
													DBCOMPAREOPS_CASESENSITIVE | (DBCOMPAREOPS_NOTCONTAINS+1), 0, NULL, FALSE, 
													lRowsToFetch, &cRowsObtained, &rghRows);

	if (!CHECK(m_hr, DB_E_BADCOMPAREOP))
		fTestPass = TEST_FAIL;

CLEANUP:
	ReleaseFindValueAccessor(g_wColType);
	ReleaseRowsetAndAccessor();

	return fTestPass;	
}
// }}


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc DB_E_BADCOMPAREOP, just caseinsensitive
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Boundary::Variation_18()
{
	BOOL		fTestPass = TRUE;
	DBCOUNTITEM cRowsObtained = 0;
	HROW *		rghRows = NULL;
	DBROWCOUNT	lRowsToFetch = 0;

	//create a rowset and an accessor.  
	TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable, SELECT_ALLFROMTBL, IID_IRowsetFind,
		0,NULL));
	
	//setup
	TESTC(CreateFindValueAccessor(DBCOMPAREOPS_EQ, g_pCTable, 1, g_ulColNum, g_wColType, SUBOP_EMPTY));

	m_hr = m_pIRowsetFind->FindNextRow(DB_NULL_HCHAPTER, m_hRowsetFindAccessor, m_pFindValue, 
													DBCOMPAREOPS_CASEINSENSITIVE | -1 , 0, NULL, FALSE, 
													lRowsToFetch, &cRowsObtained, &rghRows);

	if (!CHECK(m_hr, DB_E_BADCOMPAREOP))
		fTestPass = TEST_FAIL;

CLEANUP:
	ReleaseFindValueAccessor(g_wColType);
	ReleaseRowsetAndAccessor();

	return fTestPass;	
}
// }}


// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc DB_E_BADCOMPAREOP, both case sensitive and insensitive
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Boundary::Variation_19()
{
	BOOL		fTestPass = TRUE;
	DBCOUNTITEM cRowsObtained = 0;
	HROW *		rghRows = NULL;
	DBROWCOUNT	lRowsToFetch = 0;

	//create a rowset and an accessor.  
	TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable, SELECT_ALLFROMTBL, IID_IRowsetFind,
		0,NULL));
	
	//setup
	TESTC(CreateFindValueAccessor(DBCOMPAREOPS_EQ, g_pCTable, 1, g_ulColNum, g_wColType, SUBOP_EMPTY));

	m_hr = m_pIRowsetFind->FindNextRow(DB_NULL_HCHAPTER, m_hRowsetFindAccessor, m_pFindValue, 
													DBCOMPAREOPS_EQ | DBCOMPAREOPS_CASESENSITIVE | DBCOMPAREOPS_CASEINSENSITIVE,
													0, NULL, FALSE, lRowsToFetch, &cRowsObtained, &rghRows);

	if (!CHECK(m_hr, DB_E_BADCOMPAREOP))
		fTestPass = TEST_FAIL;

CLEANUP:
	ReleaseFindValueAccessor(g_wColType);
	ReleaseRowsetAndAccessor();

	return fTestPass;	
}
// }}


// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc DB_S_ENDOFROWSET, search for null on non-nullable column
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Boundary::Variation_20()
{
	BOOL		fTestPass = TRUE;
	DBCOUNTITEM	cRowsObtained;
	HROW*		rghRows = NULL;
	DBORDINAL	ulColToFind = 0;
	DBTYPE		wColType;

	//create a rowset and an accessor.  
	TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetFind,
		0,NULL));
	
	if (!GetNonNullableCol(&ulColToFind, DBCOMPAREOPS_EQ, FALSE, &wColType))
	{
		fTestPass = TEST_SKIPPED;
		goto CLEANUP;
	}

	//setup
	TESTC(CreateFindValueAccessor(DBCOMPAREOPS_EQ, g_pCTable, 1, ulColToFind, wColType, SUBOP_EMPTY));

	// Overwrite the status to search for NULL
	ASSERT(DBSTATUS_S_OK == *(DBSTATUS *)(m_pFindValue+offsetof(DATA,sStatus))); 
	*(DBSTATUS *)(m_pFindValue+offsetof(DATA,sStatus)) = DBSTATUS_S_ISNULL;

	m_hr = m_pIRowsetFind->FindNextRow(DB_NULL_HCHAPTER, m_hRowsetFindAccessor, m_pFindValue, 
													DBCOMPAREOPS_EQ, 0, NULL, FALSE, 
													1, &cRowsObtained, &rghRows);

	// reset status so that ReleaseFindValueAccessor 
	// can clean any embedded pointers
	*(DBSTATUS *)(m_pFindValue+offsetof(DATA,sStatus)) = DBSTATUS_S_OK;

	TESTC_(m_hr,DB_S_ENDOFROWSET);
	TESTC(cRowsObtained==0);
	TESTC(rghRows==NULL);
	

CLEANUP:

	PROVIDER_FREE(rghRows);
	ReleaseFindValueAccessor(wColType);
	ReleaseRowsetAndAccessor();

	return fTestPass;	
}
// }}
// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Boundary::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIRowsetFind::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(OutputRowHandleAllocation)
//*-----------------------------------------------------------------------
//| Test Case:		OutputRowHandleAllocation - Test cRows, prghRows, pcRowsObtained parameters
//|	Created:			06/18/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL OutputRowHandleAllocation::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCIRowsetFind::Init())
	// }}
	{
		// TO DO:  Add your own code here
		return TRUE;
	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc cRows==0, *prghRows != NULL,   Verify *prghRows != NULL on output
//
// @rdesc TEST_PASS or TEST_FAIL
//
int OutputRowHandleAllocation::Variation_1()
{
	BOOL		fTestPass = TRUE;
	DBCOUNTITEM cRowsObtained = 0;
	HROW *		rghRows = NULL;
	DBROWCOUNT	lRowsToFetch = 0;

	//create a rowset and an accessor.  
	TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetFind,
		0,NULL));
	
	//setup
	TESTC(CreateFindValueAccessor(DBCOMPAREOPS_EQ, g_pCTable, 1, g_ulColNum, g_wColType, SUBOP_EMPTY));

	m_hr = m_pIRowsetFind->FindNextRow(DB_NULL_HCHAPTER, m_hRowsetFindAccessor, m_pFindValue, 
													DBCOMPAREOPS_EQ, 0, NULL, FALSE, 
													lRowsToFetch, &cRowsObtained, &rghRows);

	// Verify HRESULT
	if ( !COMPARE(m_hr,S_OK) )
	{
		fTestPass = TEST_FAIL;
		goto CLEANUP;
	}

	// Verify rghRows
	if ( !COMPARE(rghRows,NULL) )
	{
		fTestPass = TEST_FAIL;
		goto CLEANUP;
	}

	// Verify cRowsObtained
	if ( !COMPARE(cRowsObtained,0) )
	{
		fTestPass = TEST_FAIL;
		goto CLEANUP;
	}


CLEANUP:
	ReleaseFindValueAccessor(g_wColType);
	ReleaseRowsetAndAccessor();

	return fTestPass;	
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc *pcRowsObtained = 0, *prghRows = NULL.  Verify *prghRows = NULL on output
//
// @rdesc TEST_PASS or TEST_FAIL
//
int OutputRowHandleAllocation::Variation_2()
{
	BOOL		fTestPass = TRUE;
	DBCOUNTITEM cRowsObtained = 0;
	HROW *		rghRows = NULL;
	DBROWCOUNT	lRowsToFetch = 1;

	//create a rowset and an accessor.  
	TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetFind,
		0,NULL));
	
	//setup - make sure no match occurs by matching to row #0
	TESTC(fTestPass=CreateFindValueAccessor(DBCOMPAREOPS_EQ, g_pCTable, 0, g_ulColNum, g_wColType, SUBOP_EMPTY));

	m_hr = m_pIRowsetFind->FindNextRow(DB_NULL_HCHAPTER, m_hRowsetFindAccessor, m_pFindValue, 
													DBCOMPAREOPS_EQ, 0, NULL, FALSE, 
													lRowsToFetch, &cRowsObtained, &rghRows);

	// Verify HRESULT
	if ( !COMPARE(m_hr,DB_S_ENDOFROWSET) )
	{
		fTestPass = TEST_FAIL;
		goto CLEANUP;
	}

	// Verify rghRows
	if ( !COMPARE(rghRows,NULL) )
	{
		fTestPass = TEST_FAIL;
		goto CLEANUP;
	}

	// Verify cRowsObtained
	if ( !COMPARE(cRowsObtained,0) )
	{
		fTestPass = TEST_FAIL;
		goto CLEANUP;
	}


CLEANUP:
	ReleaseFindValueAccessor(g_wColType);
	ReleaseRowsetAndAccessor();

	return fTestPass;	
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc pBookmark=NULL, cRows=0,  Match last.  Verify S_OK.  Set fSkip=1, cRows = 1, pBookmark = NULL, match last. DB_S_ENDOFROWSET
//
// @rdesc TEST_PASS or TEST_FAIL
//
int OutputRowHandleAllocation::Variation_3()
{
	// TO DO:  Add your own code here
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc pBookmark=NULL, cRows=LONG_MAX.  fSkip=0.  Match first row.  Verify DB_S_ENDOFROWSET and all rows in rowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int OutputRowHandleAllocation::Variation_4()
{
	BOOL		fTestPass = TEST_FAIL;
	HROW *		rghRows = NULL;
	DBCOUNTITEM cRowsObtained = 0;

	//create a rowset and an accessor.  
	TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetFind,
		0,NULL));
	
	//setup
	TESTC(CreateFindValueAccessor(DBCOMPAREOPS_EQ, g_pCTable, 1, g_ulColNum, g_wColType, SUBOP_EMPTY));

	m_hr = m_pIRowsetFind->FindNextRow(DB_NULL_HCHAPTER, m_hRowsetFindAccessor, m_pFindValue, 
													DBCOMPAREOPS_EQ, 0, NULL, 0, 
													MAXDBROWCOUNT, &cRowsObtained, &rghRows);

	if (m_hr == E_OUTOFMEMORY)
	{
		if (!COMPARE(cRowsObtained, 0))
			goto CLEANUP;		

		if (!COMPARE(rghRows, NULL))
			goto CLEANUP;		
	}
	else if (m_hr == DB_S_ENDOFROWSET)
	{
		if (!COMPARE(cRowsObtained, ULONG(g_lRowLast)))
			goto CLEANUP;

		if(!COMPARE(VerifyRowPosition(rghRows[0],1,g_pCTable),TRUE))
			goto CLEANUP;
	}
	else
	{
		CHECK(m_hr, DB_S_ENDOFROWSET);  // generate a error
		goto CLEANUP;					// always a test failure at this point
	}

	fTestPass = TEST_PASS;

CLEANUP:
	PROVIDER_FREE(rghRows);
	ReleaseFindValueAccessor(g_wColType);
	ReleaseRowsetAndAccessor();

	return fTestPass;		
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL OutputRowHandleAllocation::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIRowsetFind::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(CompareOp)
//*-----------------------------------------------------------------------
//| Test Case:		CompareOp - Test the various compare operations
//|	Created:			06/18/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL CompareOp::Init()
{
	BOOL fTestPass = TEST_PASS;
	if(!TCIRowsetFind::Init())
		return FALSE;

	//create a rowset and an accessor.  
	TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetFind,
										0, NULL));

CLEANUP:
	return fTestPass;

}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc COMPAREOPS_EQ
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CompareOp::Variation_1()
{
	return CompareOpTest(g_pCTable, DBCOMPAREOPS_EQ, SUBOP_EMPTY);
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc COMPAREOPS_LT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CompareOp::Variation_2()
{
	return CompareOpTest(g_pCTable, DBCOMPAREOPS_LT, SUBOP_EMPTY);
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc COMPAREOPS_LE, Use Equal case
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CompareOp::Variation_3()
{
	return CompareOpTest(g_pCTable, DBCOMPAREOPS_LE, SUBOP_ALWAYS_EQ);
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc COMPAREOPS_LE, Use Less Than case
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CompareOp::Variation_4()
{
	return CompareOpTest(g_pCTable, DBCOMPAREOPS_LE, SUBOP_EMPTY);
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc COMPAREOPS_GT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CompareOp::Variation_5()
{
	return CompareOpTest(g_pCTable, DBCOMPAREOPS_GT, SUBOP_EMPTY);
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc COMPAREOPS_GE, Use Equal case
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CompareOp::Variation_6()
{
	return CompareOpTest(g_pCTable, DBCOMPAREOPS_GE, SUBOP_ALWAYS_EQ);
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc COMPAREOPS_GE, Use Greater than case
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CompareOp::Variation_7()
{
	return CompareOpTest(g_pCTable, DBCOMPAREOPS_GE, SUBOP_EMPTY);
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc COMPAREOPS_BEGINSWITH
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CompareOp::Variation_8()
{
	return CompareOpTest(g_pCTable, DBCOMPAREOPS_BEGINSWITH, SUBOP_EMPTY);
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc COMPAREOPS_CONTAINS, match start of string
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CompareOp::Variation_9()
{
	return CompareOpTest(g_pCTable, DBCOMPAREOPS_CONTAINS, SUBOP_CONTAINS_BEGIN);
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc COMPAREOPS_CONTAINS, match middle of string
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CompareOp::Variation_10()
{
	return CompareOpTest(g_pCTable, DBCOMPAREOPS_CONTAINS, SUBOP_CONTAINS_MIDDLE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc COMPAREOPS_CONTAINS, match end of string
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CompareOp::Variation_11()
{
	return CompareOpTest(g_pCTable, DBCOMPAREOPS_CONTAINS, SUBOP_CONTAINS_END);
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc COMPAREOPS_NE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CompareOp::Variation_12()
{
	return CompareOpTest(g_pCTable, DBCOMPAREOPS_NE, SUBOP_EMPTY);
}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc COMPAREOPS_IGNORE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CompareOp::Variation_13()
{
	return CompareOpTest(g_pCTable, DBCOMPAREOPS_IGNORE, SUBOP_EMPTY);
}
// }}


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc COMPAREOPS_NOTBEGINSWITH
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CompareOp::Variation_14()
{
	return CompareOpTest(g_pCTable, DBCOMPAREOPS_NOTBEGINSWITH, SUBOP_EMPTY);
}
// }}


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc COMPAREOPS_NOTCONTAINS
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CompareOp::Variation_15()
{
	return CompareOpTest(g_pCTable, DBCOMPAREOPS_NOTCONTAINS, SUBOP_EMPTY);
}
// }}


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc COMPAREOPS_CONTAINS, use Equal case
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CompareOp::Variation_16()
{
	return CompareOpTest(g_pCTable, DBCOMPAREOPS_CONTAINS, SUBOP_ALWAYS_EQ);
}
// }}


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc COMPAREOPS_BEGINSWITH, Use Equal case
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CompareOp::Variation_17()
{
	return CompareOpTest(g_pCTable, DBCOMPAREOPS_BEGINSWITH, SUBOP_ALWAYS_EQ);
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL CompareOp::Terminate()
{
	ReleaseRowsetAndAccessor();
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIRowsetFind::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(fSkipCurrent)
//*-----------------------------------------------------------------------
//| Test Case:		fSkipCurrent - Test for correct behavior with fSkipCurrent flag
//|	Created:			06/18/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL fSkipCurrent::Init()
{
	BOOL fTestPass = FALSE;

	DBPROPID	guidPropertySet;
	ULONG	cPrptSet=0;

	if(!TCIRowsetFind::Init())
		return FALSE;

	TESTC_DRIVER(g_rgDBPrpt[IDX_ScrollBackwards].fSupported);

	if(!g_rgDBPrpt[IDX_ScrollBackwards].fDefault)
	{
		guidPropertySet=DBPROP_CANSCROLLBACKWARDS;
		cPrptSet++;
	}

	//create a rowset and an accessor.  
	TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetFind,
		cPrptSet,&guidPropertySet));

CLEANUP:
	return TRUE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc fSkip=0, cRows=1.  Match 1st.   Verify S_OK and one row handle
//
// @rdesc TEST_PASS or TEST_FAIL
//
int fSkipCurrent::Variation_1()
{
	BOOL fTestPass = FALSE;

	//restart the cursor position
	if(!CHECK(RestartRowPosition(),S_OK))
		return TEST_FAIL;

	fTestPass = CallFindNextRows(	g_pCTable,	// CTable pointer
									NULL,  // bookmark;
									0,				// Length of bookmark
									1,				 // # rows to fetch
									1,				// offset
									g_ulColNum,		// Which column to match
									1,		 // row to match
									DB_S_ENDOFROWSET,	// HRESULT to verify
									0		// How many rows to expect.
								);

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Match 1st row.  pBookMark=NULL, fSkip=TRUE.  Match 1st row. DB_S_ENDOFROWSET and no row handles.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int fSkipCurrent::Variation_2()
{
	BOOL fTestPass = FALSE;
	DBBOOKMARK dbBookmark = DBBMK_FIRST;
	BYTE *pBookmark = (BYTE *)&dbBookmark;

	//restart the cursor position
	if(!CHECK(RestartRowPosition(),S_OK))
		return TEST_FAIL;

	fTestPass = CallFindNextRows(	g_pCTable,	// CTable pointer
									pBookmark,  // bookmark;
									1,				// Length of bookmark
									1,		 // # rows to fetch
									1,			// Offset
									g_ulColNum,		// Which column to match
									1,		 // row to match
									DB_S_ENDOFROWSET,	// HRESULT to verify
									0		// How many rows to expect.
								);

	if ( fTestPass == TEST_FAIL ) return TEST_FAIL;

	fTestPass = CallFindNextRows(	g_pCTable,	// CTable pointer
									NULL,		 // bookmark;
									0,				// Length of bookmark
									-1,			 // # rows to fetch
									-1,			 // offset
									g_ulColNum,		// Which column to match
									g_lRowLast,		 // row to match
									DB_S_ENDOFROWSET,	// HRESULT to verify
									0		// How many rows to expect.
								);

	return fTestPass;

}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Match 5th row.  pBookMark=NULL, fSkip=FALSE.  Match 5th row. S_OK and no row handles.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int fSkipCurrent::Variation_3()
{
	BOOL fTestPass = FALSE;

	//restart the cursor position
	if(!CHECK(RestartRowPosition(),S_OK))
		return TEST_FAIL;

	fTestPass = CallFindNextRows(	g_pCTable,	// CTable pointer
									NULL,					// bookmark;
									0,						// Length of bookmark
									1,						// # rows to fetch
									FALSE,				// skip current row
									g_ulColNum,			// Which column to match
									5,						// row to match
									S_OK,					// HRESULT to verify
									1						// How many rows to expect.
								);

	if ( fTestPass == TEST_FAIL ) return TEST_FAIL;

	fTestPass = CallFindNextRows(	g_pCTable,	// CTable pointer
									NULL,					// bookmark;
									0,						// Length of bookmark
									1,						// # rows to fetch
									1,						// offset
									g_ulColNum,			// Which column to match
									5,						// row to match
									DB_S_ENDOFROWSET,	// HRESULT to verify
									0						// How many rows to expect.
								);

	return fTestPass;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL fSkipCurrent::Terminate()
{
	ReleaseRowsetAndAccessor();

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIRowsetFind::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(pFindValue)
//*-----------------------------------------------------------------------
//| Test Case:		pFindValue - Test the various coercions possible
//|	Created:			06/18/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL pFindValue::Init()
{
	DBPROPID	guidPropertySet;
	ULONG	cPrptSet=0;
	BOOL fTestPass = FALSE;

	if(!TCIRowsetFind::Init())
		return FALSE;

	//create a rowset and an accessor.  
	TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetFind,
		cPrptSet,&guidPropertySet));

CLEANUP:
	return TRUE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc IStorage compares, DBCOMPAREOPS_EQ
//
// @rdesc TEST_PASS or TEST_FAIL
//
int pFindValue::Variation_1()
{
	return CompareOpTest(g_pCTable, DBCOMPAREOPS_EQ, SUBOP_EMPTY, TRUE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc IStorage, DBCOMPAREOPS_BEGINSWITH
//
// @rdesc TEST_PASS or TEST_FAIL
//
int pFindValue::Variation_2()
{
	return CompareOpTest(g_pCTable, DBCOMPAREOPS_BEGINSWITH, SUBOP_EMPTY, TRUE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc IStorage, DBCOMPAREOPS_GT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int pFindValue::Variation_3()
{
	return CompareOpTest(g_pCTable, DBCOMPAREOPS_GT, SUBOP_EMPTY, TRUE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc IStorage, DBCOMPAREOPS_GE (EQUAL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int pFindValue::Variation_4()
{
	return CompareOpTest(g_pCTable, DBCOMPAREOPS_EQ, SUBOP_ALWAYS_EQ, TRUE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc IStorage, DBCOMPAREOPS_GE (Greater
//
// @rdesc TEST_PASS or TEST_FAIL
//
int pFindValue::Variation_5()
{
	return CompareOpTest(g_pCTable, DBCOMPAREOPS_GE, SUBOP_EMPTY, TRUE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc IStorage, DBCOMPAREOPS_LT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int pFindValue::Variation_6()
{
	return CompareOpTest(g_pCTable, DBCOMPAREOPS_LT, SUBOP_EMPTY, TRUE)  ;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc IStorage, DBCOMPAREOPS_LE (EQUAL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int pFindValue::Variation_7()
{
	return CompareOpTest(g_pCTable, DBCOMPAREOPS_LE, SUBOP_ALWAYS_EQ, TRUE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc IStorage, DBCOMPAREOPS_LE (Less
//
// @rdesc TEST_PASS or TEST_FAIL
//
int pFindValue::Variation_8()
{
	return CompareOpTest(g_pCTable, DBCOMPAREOPS_LE, SUBOP_EMPTY, TRUE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc IStorage, DBCOMPAREOPS_CONTAINS, beginning
//
// @rdesc TEST_PASS or TEST_FAIL
//
int pFindValue::Variation_9()
{
	return CompareOpTest(g_pCTable, DBCOMPAREOPS_CONTAINS, SUBOP_CONTAINS_BEGIN, TRUE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc IStorage, DBCOMPAREOPS_CONTAINS, middle
//
// @rdesc TEST_PASS or TEST_FAIL
//
int pFindValue::Variation_10()
{
	return CompareOpTest(g_pCTable, DBCOMPAREOPS_CONTAINS, SUBOP_CONTAINS_MIDDLE, TRUE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc IStorage, DBCOMPAREOPS_CONTAINS, end
//
// @rdesc TEST_PASS or TEST_FAIL
//
int pFindValue::Variation_11()
{
	return CompareOpTest(g_pCTable, DBCOMPAREOPS_CONTAINS, SUBOP_CONTAINS_END, TRUE);
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL pFindValue::Terminate()
{
	ReleaseRowsetAndAccessor();
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIRowsetFind::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(SingleRowRowset)
//*-----------------------------------------------------------------------
//| Test Case:		SingleRowRowset - Test on a one row rowset
//|	Created:			06/18/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL SingleRowRowset::Init()
{
	if (!g_p1RowTable)
		return TEST_SKIPPED;

	if(!TCIRowsetFind::Init())
		return FALSE;

	return TRUE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Match 1st. pBookMark = DBBMK_FIRST, cRows=1, fSkip=TRUE.  DB_S_ENDOFROWSET
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SingleRowRowset::Variation_1()
{
	BOOL fTestPass;
	DBPROPID		guidProperty[1];
	DBBOOKMARK	DBBookmark=DBBMK_FIRST;
	BYTE		*pBookmark=(BYTE *)&DBBookmark;

	guidProperty[0]=DBPROP_CANSCROLLBACKWARDS;

	//open rowset, and accessor.  
	if(!GetRowsetAndAccessor(g_p1RowTable, SELECT_ALLFROMTBL, IID_IRowsetFind,
		1,guidProperty))
	{
		fTestPass = TEST_PASS;  // OK to pass if properties aren't supported
		goto CLEANUP;
	}

	fTestPass = CallFindNextRows(	g_p1RowTable,	// CTable pointer
									NULL,  // bookmark;
									0,				// Length of bookmark
									1,		 // # rows to fetch
									0, // skip current row
									g_ulColNum,		// Which column to match
									1,		 // row to match
									S_OK,	// HRESULT to verify
									1		// How many rows to expect.
								);

	if ( fTestPass == TEST_FAIL ) goto CLEANUP;

	fTestPass = CallFindNextRows(	g_p1RowTable,	// CTable pointer
									pBookmark,  // bookmark;
									1,			// Length of bookmark
									1,			// # rows to fetch
									1,			// skip current row
									g_ulColNum,	// Which column to match
									1,			// row to match
									DB_S_ENDOFROWSET,	// HRESULT to verify
									0		// How many rows to expect.
								);

CLEANUP:
	ReleaseRowsetAndAccessor();

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Match 1st. pBookMark = DBBMK_FIRST, cRows=1, fSkipCurrent = FALSE.  S_OK and one row.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SingleRowRowset::Variation_2()
{
	BOOL fTestPass;
	DBPROPID		guidProperty[1];
	DBBOOKMARK	DBBookmark=DBBMK_FIRST;
	BYTE		*pBookmark=(BYTE *)&DBBookmark;

	guidProperty[0]=DBPROP_CANSCROLLBACKWARDS;

	//open rowset, and accessor.  
	if(!GetRowsetAndAccessor(g_p1RowTable, SELECT_ALLFROMTBL, IID_IRowsetFind,
		1,guidProperty))
	{
		fTestPass = TEST_PASS;  // OK to pass if properties aren't supported
		goto CLEANUP;
	}

	fTestPass = CallFindNextRows(	g_p1RowTable,	// CTable pointer
									NULL,				// bookmark;
									0,					// Length of bookmark
									1,					// # rows to fetch
									0,					// Offset
									g_ulColNum,		// Which column to match
									1,					// row to match
									S_OK,				// HRESULT to verify
									1					// How many rows to expect.
								);

	if ( fTestPass == TEST_FAIL ) goto CLEANUP;

	fTestPass = CallFindNextRows(	g_p1RowTable,	// CTable pointer
									pBookmark,  // bookmark;
									1,				// Length of bookmark
									1,		 // # rows to fetch
									FALSE, // skip current row
									g_ulColNum,		// Which column to match
									1,		 // row to match
									S_OK,	// HRESULT to verify
									1		// How many rows to expect.
								);

CLEANUP:
	ReleaseRowsetAndAccessor();

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Match First. pBookmark = DBBMK_LAST. cRows=-1, fSkip=TRUE.  DB_S_ENDOFROWSET
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SingleRowRowset::Variation_3()
{
	BOOL fTestPass;
	DBPROPID		guidProperty[2];
	DBBOOKMARK	DBBookmark=DBBMK_LAST;
	BYTE		*pBookmark=(BYTE *)&DBBookmark;

	guidProperty[0]=DBPROP_CANSCROLLBACKWARDS;
	guidProperty[1]=DBPROP_CANFETCHBACKWARDS;

	//open rowset, and accessor.  
	if(!GetRowsetAndAccessor(g_p1RowTable, SELECT_ALLFROMTBL, IID_IRowsetFind,
		1,guidProperty))
	{
		fTestPass = TEST_PASS;  // OK to pass if properties aren't supported
		goto CLEANUP;
	}

	fTestPass = CallFindNextRows(	g_p1RowTable,	// CTable pointer
									NULL,  // bookmark;
									0,				// Length of bookmark
									1,		 // # rows to fetch
									0,		// skip current row
									g_ulColNum,		// Which column to match
									1,		 // row to match
									S_OK,	// HRESULT to verify
									1		// How many rows to expect.
								);

	if ( fTestPass == TEST_FAIL ) goto CLEANUP;

	fTestPass = CallFindNextRows(	g_p1RowTable,	// CTable pointer
									pBookmark,		// bookmark;
									1,				// Length of bookmark
									-1,				// # rows to fetch
									1,				// skip current row
									g_ulColNum,		// Which column to match
									1,		 // row to match
									DB_S_ENDOFROWSET,	// HRESULT to verify
									0		// How many rows to expect.
								);

CLEANUP:
	ReleaseRowsetAndAccessor();

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Match first.  pBookMark=DBBMK_LAST. cRows=-1.  fSkip=-1.  fSkip=FALSE.  S_OK and 1 hrow
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SingleRowRowset::Variation_4()
{
	BOOL fTestPass;
	DBPROPID		guidProperty[2];
	DBBOOKMARK	DBBookmark=DBBMK_LAST;
	BYTE		*pBookmark=(BYTE *)&DBBookmark;

	guidProperty[0]=DBPROP_CANSCROLLBACKWARDS;
	guidProperty[1]=DBPROP_CANFETCHBACKWARDS;

	//open rowset, and accessor.  
	if(!GetRowsetAndAccessor(g_p1RowTable, SELECT_ALLFROMTBL, IID_IRowsetFind,
		1,guidProperty))
	{
		fTestPass = TEST_PASS;  // OK to pass if properties aren't supported
		goto CLEANUP;
	}

	fTestPass = CallFindNextRows(	g_p1RowTable,	// CTable pointer
									NULL,  // bookmark;
									0,				// Length of bookmark
									1,			// # rows to fetch
									0,			// skip current row
									g_ulColNum,		// Which column to match
									1,		 // row to match
									S_OK,	// HRESULT to verify
									1		// How many rows to expect.
								);

	if ( fTestPass == TEST_FAIL ) goto CLEANUP;

	fTestPass = CallFindNextRows(	g_p1RowTable,	// CTable pointer
									pBookmark,  // bookmark;
									1,				// Length of bookmark
									-1,		 // # rows to fetch
									0,		// lOffset
									g_ulColNum,		// Which column to match
									1,		 // row to match
									S_OK,	// HRESULT to verify
									1		// How many rows to expect.
								);

CLEANUP:
	ReleaseRowsetAndAccessor();

	return fTestPass;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL SingleRowRowset::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIRowsetFind::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(Related_RestartPosition)
//*-----------------------------------------------------------------------
//| Test Case:		Related_RestartPosition - Test in conjunction with RestartPosition
//|	Created:			06/18/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Related_RestartPosition::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCIRowsetFind::Init())
	// }}
	{
		// TO DO:  Add your own code here
		return TRUE;
	}
	return FALSE;
}

// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Match 5th. Restart.  Call Find, cRows=1, fSkip=TRUE, match 5th.  Verify DB_S_ENDOFROWSET (because of restart
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Related_RestartPosition::Variation_1()
{
	return TRUE;
}
// }}

// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Match 5th. Restart.  Call Find, cRows=1, fSkip=FALSE, match 4th.  Verify DB_S_ENDOFROWSET
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Related_RestartPosition::Variation_2()
{
	return TRUE;
}
// }}

// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Match Last.  Restart. Call Find, cRows=1, fSkip=TRUE.  Verify DB_S_ENDOFROWSET
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Related_RestartPosition::Variation_3()
{
	return TRUE;
}
// }}

// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Related_RestartPosition::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIRowsetFind::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(Related_GetNextRows)
//*-----------------------------------------------------------------------
//| Test Case:		Related_GetNextRows - Test in conjunction with GetNextRows
//|	Created:			06/18/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Related_GetNextRows::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCIRowsetFind::Init())
	// }}
	{
		// TO DO:  Add your own code here
		return TRUE;
	}
	return FALSE;
}

// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc pBookmark=DBBMK_FIRST, Verify GetNextRows pos not changed
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Related_GetNextRows::Variation_1()
{
	BOOL		fTestPass = FALSE;
	DBBOOKMARK	DBBookmark=DBBMK_FIRST;
	BYTE *		pBookmark=(BYTE *)&DBBookmark;
	DBCOUNTITEM cRowsObtained = 0;
	HROW *		phrow = NULL;
	DBPROPID	guidPropertySet[1];
	ULONG	cPrptSet=0;

	TESTC_DRIVER(g_rgDBPrpt[IDX_ScrollBackwards].fSupported);

	if(!g_rgDBPrpt[IDX_ScrollBackwards].fDefault)
	{
		guidPropertySet[cPrptSet++]=DBPROP_CANSCROLLBACKWARDS;
	}

	//create a rowset and an accessor.  
	//DBPROP_CANSCROLLBACKWARDS is requested 

	
	TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetFind,
		cPrptSet,guidPropertySet));

	fTestPass =  CallFindNextRows(	g_pCTable,			// CTable pointer
												pBookmark,			// bookmark;
												1,						// Length of bookmark
												1,				  	   // # rows to fetch
												FALSE,				// skip current row
												g_ulColNum,			// Which column to match
												1,						// row to match
												S_OK,					// HRESULT to verify
												1						// How many rows to expect.
											);

	if ( fTestPass == TEST_FAIL ) goto CLEANUP;

	// using pBookmark = DBBMK_LAST should not affect GetNextRows position
	if (!CHECK(m_pIRowset->GetNextRows(DB_NULL_HCHAPTER, 0, 1, &cRowsObtained, (HROW **)&phrow ), S_OK) )
		goto CLEANUP;

	if (!COMPARE(cRowsObtained, 1))
		goto CLEANUP;
	
	//call VerifyRowPosition
	if(!COMPARE(VerifyRowPosition(phrow[0],1,g_pCTable),TRUE))
		goto CLEANUP;

	if (!CHECK(m_pIRowset->ReleaseRows(cRowsObtained, phrow, NULL, NULL, NULL), S_OK))
		goto CLEANUP;

CLEANUP:
	PROVIDER_FREE(phrow);
	ReleaseRowsetAndAccessor();
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Use pBookmark=middle bmk.  Verify GetNextRows pos not changed
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Related_GetNextRows::Variation_2()
{
	BOOL			fTestPass = FALSE;
	BYTE		*	pBookmark=NULL;
	DBCOUNTITEM		cRowsObtained = 0;
	ULONG_PTR		cbBookmark = 0;
	HROW *			phrow = NULL;
	IRowsetLocate *	pIRowsetLocate = NULL;
	DBPROPID		guidProperty[1];

	guidProperty[0]=DBPROP_IRowsetLocate;

	
	TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetFind,
		NUMELEM(guidProperty), guidProperty));

	if ( !VerifyInterface(m_pIRowset, IID_IRowsetLocate, ROWSET_INTERFACE, (IUnknown **)&pIRowsetLocate))
		goto CLEANUP;

	GetBookmark(g_lRowLast/2, &cbBookmark, &pBookmark);

	fTestPass =  CallFindNextRows(	g_pCTable,			// CTable pointer
												pBookmark,			// bookmark;
												cbBookmark,			// Length of bookmark
												1,				  	   // # rows to fetch
												FALSE,				// skip current row
												g_ulColNum,			// Which column to match
												g_lRowLast-2,			// row to match
												S_OK,					// HRESULT to verify
												1						// How many rows to expect.
											);

	if ( fTestPass == TEST_FAIL ) goto CLEANUP;

	// using pBookmark = g_lRowLast/2 should not affect GetNextRows position
	if (!CHECK(m_pIRowset->GetNextRows(DB_NULL_HCHAPTER, 0, 1, &cRowsObtained, (HROW **)&phrow ), S_OK) )
		goto CLEANUP;

	if (!COMPARE(cRowsObtained, 1))
		goto CLEANUP;
	
	//call VerifyRowPosition
	if(!COMPARE(VerifyRowPosition(phrow[0],1,g_pCTable),TRUE))
		goto CLEANUP;

	if (!CHECK(m_pIRowset->ReleaseRows(cRowsObtained, phrow, NULL, NULL, NULL), S_OK))
		goto CLEANUP;

CLEANUP:
	PROVIDER_FREE(pBookmark);
	PROVIDER_FREE(phrow);
	SAFE_RELEASE(pIRowsetLocate);
	ReleaseRowsetAndAccessor();
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Use pBookMark = DBBMK_LAST, Verify GetNextRows pos not changed
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Related_GetNextRows::Variation_3()
{
	BOOL		fTestPass = FALSE;
	DBBOOKMARK	DBBookmark=DBBMK_LAST;
	BYTE *		pBookmark=(BYTE *)&DBBookmark;
	DBCOUNTITEM	cRowsObtained = 0;
	HROW *		phrow = NULL;

	DBPROPID	guidPropertySet[1];
	ULONG	cPrptSet=0;

	TESTC_DRIVER(g_rgDBPrpt[IDX_ScrollBackwards].fSupported);

	if(!g_rgDBPrpt[IDX_ScrollBackwards].fDefault)
	{
		guidPropertySet[cPrptSet++]=DBPROP_CANSCROLLBACKWARDS;
	}


	TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetFind,
		cPrptSet,guidPropertySet));

	fTestPass =  CallFindNextRows(	g_pCTable,			// CTable pointer
									pBookmark,			// bookmark;
									1,					// Length of bookmark
									1,					// # rows to fetch
									FALSE,				// skip current row
									g_ulColNum,			// Which column to match
									g_lRowLast,			// row to match
									S_OK,				// HRESULT to verify
									1					// How many rows to expect.
								);

	if ( fTestPass == TEST_FAIL ) goto CLEANUP;

	// using pBookmark = DBBMK_LAST should not affect GetNextRows position
	if (!CHECK(m_pIRowset->GetNextRows(DB_NULL_HCHAPTER, 0, 1, &cRowsObtained, (HROW **)&phrow ), S_OK) )
		goto CLEANUP;

	if (!COMPARE(cRowsObtained, 1))
		goto CLEANUP;
	
	//call VerifyRowPosition
	if(!COMPARE(VerifyRowPosition(phrow[0],1,g_pCTable),TRUE))
		goto CLEANUP;

	if (!CHECK(m_pIRowset->ReleaseRows(cRowsObtained, phrow, NULL, NULL, NULL), S_OK))
		goto CLEANUP;

CLEANUP:
	PROVIDER_FREE(phrow);
	ReleaseRowsetAndAccessor();
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc use pBookmark=NULL, match 1st row. Verify GetNextRows pos is after 1st row.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Related_GetNextRows::Variation_4()
{
	BOOL fTestPass = FALSE;
	DBCOUNTITEM cRowsObtained = 0;
	HROW *phrow = NULL;
	
	TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetFind,
		0,NULL));

	fTestPass =  CallFindNextRows(	g_pCTable,			// CTable pointer
									NULL,				// bookmark;
									0,					// Length of bookmark
									1,					// # rows to fetch
									FALSE,				// skip current row
									g_ulColNum,			// Which column to match
									1,					// row to match
									S_OK,				// HRESULT to verify
									1					// How many rows to expect.
								);

	if ( fTestPass == TEST_FAIL ) goto CLEANUP;

	// using pBookmark = NULL should affect GetNextRows position
	if (!CHECK(m_pIRowset->GetNextRows(DB_NULL_HCHAPTER, 0, 1, &cRowsObtained, (HROW **)&phrow ), S_OK) )
		goto CLEANUP;

	if (!COMPARE(cRowsObtained, 1))
		goto CLEANUP;
	
	//call VerifyRowPosition
	if(!COMPARE(VerifyRowPosition(phrow[0],2,g_pCTable),TRUE))
		goto CLEANUP;

	if (!CHECK(m_pIRowset->ReleaseRows(cRowsObtained, phrow, NULL, NULL, NULL), S_OK))
		goto CLEANUP;

CLEANUP:
	PROVIDER_FREE(phrow);
	ReleaseRowsetAndAccessor();
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Use pBookmark=NULL, match last row.  Verify GetNextRows with cRows=1 returns DB_S_ENDOFROWSET
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Related_GetNextRows::Variation_5()
{
	BOOL fTestPass = FALSE;
	DBCOUNTITEM cRowsObtained = 0;
	HROW *phrow = NULL;
	
	TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetFind,
		0,NULL));

	fTestPass =  CallFindNextRows(	g_pCTable,			// CTable pointer
												NULL,					// bookmark;
												0,						// Length of bookmark
												1,				  	   // # rows to fetch
												FALSE,				// skip current row
												g_ulColNum,			// Which column to match
												g_lRowLast,			// row to match
												S_OK,					// HRESULT to verify
												1						// How many rows to expect.
											);

	if ( fTestPass == TEST_FAIL ) goto CLEANUP;

	// using pBookmark = NULL should affect GetNextRows position
	if (!CHECK(m_pIRowset->GetNextRows(DB_NULL_HCHAPTER, 0, 1, &cRowsObtained, (HROW **)&phrow ), DB_S_ENDOFROWSET) )
		goto CLEANUP;

	if (!COMPARE(cRowsObtained, 0))
		goto CLEANUP;
	
	//call VerifyRowPosition
	if(!COMPARE(phrow,NULL))
		goto CLEANUP;

CLEANUP:
	PROVIDER_FREE(phrow);
	ReleaseRowsetAndAccessor();
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Use pBookmark=NULL, match middle row. Use pBookmark=2nd bmk and match 2nd.  Verify GetNextRows is still after middle row
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Related_GetNextRows::Variation_6()
{
	BOOL			 fTestPass = FALSE;
	BYTE			*pBookmark=NULL;
	DBCOUNTITEM		cRowsObtained = 0;
	ULONG_PTR		cbBookmark = 0;
	HROW			*phrow = NULL;
	IRowsetLocate	*pIRowsetLocate = NULL;

	DBPROPID	PropID = DBPROP_IRowsetFind;
	ULONG		cPrptSet=1;
	
	TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetLocate,
		cPrptSet,&PropID));

	if ( !VerifyInterface(m_pIRowset, IID_IRowsetLocate, ROWSET_INTERFACE, (IUnknown **)&pIRowsetLocate))
		goto CLEANUP;

	GetBookmark(2, &cbBookmark, &pBookmark);

	fTestPass =  CallFindNextRows(	g_pCTable,			// CTable pointer
												NULL,					// bookmark;
												0,						// Length of bookmark
												1,				  	   // # rows to fetch
												FALSE,				// skip current row
												g_ulColNum,			// Which column to match
												g_lRowLast/2,			// row to match
												S_OK,					// HRESULT to verify
												1						// How many rows to expect.
											);

	if ( fTestPass == TEST_FAIL ) goto CLEANUP;

	fTestPass =  CallFindNextRows(	g_pCTable,			// CTable pointer
												pBookmark,			// bookmark;
												cbBookmark,			// Length of bookmark
												1,				  	   // # rows to fetch
												FALSE,				// skip current row
												g_ulColNum,			// Which column to match
												2,						// row to match
												S_OK,					// HRESULT to verify
												1						// How many rows to expect.
											);

	if ( fTestPass == TEST_FAIL ) goto CLEANUP;

	if (!CHECK(m_pIRowset->GetNextRows(DB_NULL_HCHAPTER, 0, 1, &cRowsObtained, (HROW **)&phrow ), S_OK) )
		goto CLEANUP;

	if (!COMPARE(cRowsObtained, 1))
		goto CLEANUP;
	
	//call VerifyRowPosition
	if(!COMPARE(VerifyRowPosition(phrow[0],(g_lRowLast/2)+1,g_pCTable),TRUE))
		goto CLEANUP;

	if (!CHECK(m_pIRowset->ReleaseRows(cRowsObtained, phrow, NULL, NULL, NULL), S_OK))
		goto CLEANUP;

CLEANUP:
	PROVIDER_FREE(phrow);
	PROVIDER_FREE(pBookmark);
	SAFE_RELEASE(pIRowsetLocate);
	ReleaseRowsetAndAccessor();
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Call GetNextRows with last row fetched = 3.  Call FindNextRows, pBookmark=NULL,cRows=-1 to match 4th.  Verify DB_S_ENDOFROWSET
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Related_GetNextRows::Variation_7()
{
	BOOL fTestPass = FALSE;
	DBCOUNTITEM cRowsObtained = 0;
	HROW *phrow = NULL;
	DBPROPID	guidProperty;
	ULONG cPrptSet = 0;
	TESTC_DRIVER(g_rgDBPrpt[IDX_FetchBackwards].fSupported &&
	   g_rgDBPrpt[IDX_ScrollBackwards].fSupported );

	if(!g_rgDBPrpt[IDX_FetchBackwards].fDefault)
	{
		guidProperty=DBPROP_CANFETCHBACKWARDS;
		cPrptSet++;
	}

	//create a rowset and an accessor.  
	//DBPROP_CANFETCHBACKWARDS is requested 

	TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetFind,
		cPrptSet,&guidProperty));

	if (!CHECK(m_pIRowset->GetNextRows(DB_NULL_HCHAPTER, 0, 3, &cRowsObtained, (HROW **)&phrow ), S_OK) )
		goto CLEANUP;
	
	if (!CHECK(m_pIRowset->ReleaseRows(cRowsObtained, phrow, NULL, NULL, NULL), S_OK))
		goto CLEANUP;

	fTestPass =  CallFindNextRows(	g_pCTable,			// CTable pointer
												NULL,					// bookmark;
												0,						// Length of bookmark
												-1,			  	   // # rows to fetch
												FALSE,				// skip current row
												g_ulColNum,			// Which column to match
												4,						// row to match
												DB_S_ENDOFROWSET,	// HRESULT to verify
												0						// How many rows to expect.
											);

CLEANUP:
	ReleaseRowsetAndAccessor();
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc GetNextRows, cRows=1, Offset=0.  Verify S_OK.  FindNext with pBookmark=NULL, cRows=1 and match 1st.  Verify DB_S_ENDOFROWSET
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Related_GetNextRows::Variation_8()
{
	BOOL fTestPass = FALSE;
	DBCOUNTITEM cRowsObtained = 0;
	HROW *phrow = NULL;
	DBPROPID	guidProperty;
	ULONG cPrptSet = 0;
	TESTC_DRIVER(g_rgDBPrpt[IDX_FetchBackwards].fSupported &&
	   g_rgDBPrpt[IDX_ScrollBackwards].fSupported );

	if(!g_rgDBPrpt[IDX_FetchBackwards].fDefault)
	{
		guidProperty=DBPROP_CANFETCHBACKWARDS;
		cPrptSet++;
	}

	//create a rowset and an accessor.  
	//DBPROP_CANFETCHBACKWARDS is requested 

	TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetFind,
		cPrptSet,&guidProperty));

	if (!CHECK(m_pIRowset->GetNextRows(DB_NULL_HCHAPTER, 0, 1, &cRowsObtained, (HROW **)&phrow ), S_OK) )
		goto CLEANUP;
	
	if (!CHECK(m_pIRowset->ReleaseRows(cRowsObtained, phrow, NULL, NULL, NULL), S_OK))
		goto CLEANUP;

	fTestPass =  CallFindNextRows(	g_pCTable,			// CTable pointer
												NULL,					// bookmark;
												0,						// Length of bookmark
												-1,			  	   // # rows to fetch
												FALSE,				// skip current row
												g_ulColNum,			// Which column to match
												1,						// row to match
												S_OK,					// HRESULT to verify
												1						// How many rows to expect.
											);

	if ( fTestPass == TEST_FAIL ) goto CLEANUP;

CLEANUP:
	PROVIDER_FREE(phrow);
	ReleaseRowsetAndAccessor();
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc FindNext with pBookmark=NULL, cRows=2 and match 4th.  GetNextRows with cRows=-3.  Verify 5,4,3 row handles.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Related_GetNextRows::Variation_9()
{
	BOOL fTestPass = FALSE;
	DBCOUNTITEM cRowsObtained = 0;
	HROW *phrow = NULL;
	DBPROPID	guidProperty;
	ULONG cPrptSet = 0;
	TESTC_DRIVER(g_rgDBPrpt[IDX_FetchBackwards].fSupported &&
	   g_rgDBPrpt[IDX_ScrollBackwards].fSupported );

	if(!g_rgDBPrpt[IDX_FetchBackwards].fDefault)
	{
		guidProperty=DBPROP_CANFETCHBACKWARDS;
		cPrptSet++;
	}

	//create a rowset and an accessor.  
	//DBPROP_CANFETCHBACKWARDS is requested 

	TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetFind,
		cPrptSet,&guidProperty));

	fTestPass =  CallFindNextRows(	g_pCTable,			// CTable pointer
												NULL,					// bookmark;
												0,						// Length of bookmark
												2,			  		   // # rows to fetch
												FALSE,				// skip current row
												g_ulColNum,			// Which column to match
												4,						// row to match
												S_OK,					// HRESULT to verify
												2						// How many rows to expect.
											);

	if ( !COMPARE(fTestPass,TRUE) ) 
		goto CLEANUP;

	if (!CHECK(m_pIRowset->GetNextRows(DB_NULL_HCHAPTER, 0, -3, &cRowsObtained, (HROW **)&phrow ), S_OK) )
		goto CLEANUP;

	//call VerifyRowPosition
	if(!COMPARE(VerifyRowPosition(phrow[0],5,g_pCTable),TRUE))
		goto CLEANUP;

	//call VerifyRowPosition
	if(!COMPARE(VerifyRowPosition(phrow[1],4,g_pCTable),TRUE))
		goto CLEANUP;

	//call VerifyRowPosition
	if(!COMPARE(VerifyRowPosition(phrow[2],3,g_pCTable),TRUE))
		goto CLEANUP;

	if (!CHECK(m_pIRowset->ReleaseRows(cRowsObtained, phrow, NULL, NULL, NULL), S_OK))
		goto CLEANUP;

CLEANUP:
	PROVIDER_FREE(phrow);
	ReleaseRowsetAndAccessor();
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc FindNext, pBookmark=NULL, match last. FindNext, pBmk=NULL, cRows=-1, match last.  GetNext. cRows=-1 and verify next to last
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Related_GetNextRows::Variation_10()
{
	BOOL fTestPass = FALSE;
	DBCOUNTITEM cRowsObtained = 0;
	HROW *phrow = NULL;
	DBPROPID	guidProperty;
	ULONG cPrptSet = 0;
	TESTC_DRIVER(g_rgDBPrpt[IDX_FetchBackwards].fSupported &&
	   g_rgDBPrpt[IDX_ScrollBackwards].fSupported );

	if(!g_rgDBPrpt[IDX_FetchBackwards].fDefault)
	{
		guidProperty=DBPROP_CANFETCHBACKWARDS;
		cPrptSet++;
	}

	//create a rowset and an accessor.  
	//DBPROP_CANFETCHBACKWARDS is requested 

	TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetFind,
		cPrptSet,&guidProperty));

	fTestPass =  CallFindNextRows(	g_pCTable,			// CTable pointer
												NULL,					// bookmark;
												0,						// Length of bookmark
												1,			  		   // # rows to fetch
												FALSE,				// skip current row
												g_ulColNum,			// Which column to match
												g_lRowLast,			// row to match
												S_OK,					// HRESULT to verify
												1						// How many rows to expect.
											);

	if ( !COMPARE(fTestPass,TRUE) ) 
		goto CLEANUP;

	fTestPass =  CallFindNextRows(	g_pCTable,			// CTable pointer
												NULL,					// bookmark;
												0,						// Length of bookmark
												-1,			  		   // # rows to fetch
												FALSE,				// skip current row
												g_ulColNum,			// Which column to match
												g_lRowLast,			// row to match
												S_OK,					// HRESULT to verify
												1						// How many rows to expect.
											);

	if ( !COMPARE(fTestPass,TRUE) ) 
		goto CLEANUP;

	if (!CHECK(m_pIRowset->GetNextRows(DB_NULL_HCHAPTER, 0, -1, &cRowsObtained, (HROW **)&phrow ), S_OK) )
		goto CLEANUP;

	//call VerifyRowPosition
	if(!COMPARE(VerifyRowPosition(phrow[0],g_lRowLast-1,g_pCTable),TRUE))
		goto CLEANUP;

	if (!CHECK(m_pIRowset->ReleaseRows(cRowsObtained, phrow, NULL, NULL, NULL), S_OK))
		goto CLEANUP;

CLEANUP:
	PROVIDER_FREE(phrow);
	ReleaseRowsetAndAccessor();
	return fTestPass;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Related_GetNextRows::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIRowsetFind::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(Scroll_BookMark)
//*-----------------------------------------------------------------------
//| Test Case:		Scroll_BookMark - CanScrollBackwards with bookmarks
//|	Created:			06/24/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Scroll_BookMark::Init()
{
	BOOL fTestPass = FALSE;
	DBPROPID	guidPropertySet[2];
	ULONG	cPrptSet=0;

	if(!TCIRowsetFind::Init())
		return FALSE;

	TESTC_DRIVER(g_rgDBPrpt[IDX_OrderedBookmarks].fSupported &&
	   g_rgDBPrpt[IDX_IRowsetLocate].fSupported);

	if(!g_rgDBPrpt[IDX_IRowsetLocate].fDefault)
		guidPropertySet[cPrptSet++]=DBPROP_IRowsetLocate;
		
	if(!g_rgDBPrpt[IDX_OrderedBookmarks].fDefault)
		guidPropertySet[cPrptSet++]=DBPROP_ORDEREDBOOKMARKS;

	//DBPROP_ORDEREDBOOKMARKS and DBPROP_IRowsetLocate
	//are requested 
	TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetFind,
		cPrptSet,guidPropertySet));

	fTestPass = TRUE;

CLEANUP:
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc pBookMark=DBBMK_FIRST, cRows=1.  Match first row.  Verify S_OK and one row handle
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Scroll_BookMark::Variation_1()
{
	DBBOOKMARK	DBBookmark=DBBMK_FIRST;
	BYTE		*pBookmark=(BYTE *)&DBBookmark;

	return CallFindNextRows(	g_pCTable,			// CTable pointer
										pBookmark,			// bookmark;
										1,						// Length of bookmark
										1,					   // # rows to fetch
										0,						// Offset
										g_ulColNum,			// Which column to match
										1,						// row to match
										S_OK,					// HRESULT to verify
										1						// How many rows to expect.
									);
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc pBookMark=DBBMK_LAST,cRows=1.  Match last row.  Verify S_OK and last row
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Scroll_BookMark::Variation_2()
{
	DBBOOKMARK	DBBookmark=DBBMK_LAST;
	BYTE		*pBookmark=(BYTE *)&DBBookmark;

	return CallFindNextRows(	g_pCTable,			// CTable pointer
										pBookmark,			// bookmark;
										1,						// Length of bookmark
										1,						// # rows to fetch
										0,						// Offset
										g_ulColNum,			// Which column to match
										g_lRowLast,			// row to match
										S_OK,					// HRESULT to verify
										1						// How many rows to expect.
									);
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc pBookMark=DBBMK_INVALID. Verify DB_E_BADBOOKMARK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Scroll_BookMark::Variation_3()
{
	DBBOOKMARK	DBBookmark=DBBMK_INVALID;
	BYTE		*pBookmark=(BYTE *)&DBBookmark;

	return CallFindNextRows(	g_pCTable,			// CTable pointer
										pBookmark,			// bookmark;
										1,						// Length of bookmark
										1,					  // # rows to fetch
										0,						// Offset
										g_ulColNum,			// Which column to match
										1,						// row to match
										DB_E_BADBOOKMARK,	// HRESULT to verify
										0						// How many rows to expect.
									);
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc pBookMark=one bookmark for each row, cRows=1.  Always match current row.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Scroll_BookMark::Variation_4()
{
	BOOL fTestPass;
	ULONG_PTR cbBookmark;
	BYTE	*pBookmark;

	for ( LONG i = 1; i <= g_lRowLast; i++ )
	{
		GetBookmark(i, &cbBookmark, &pBookmark);

		fTestPass = CallFindNextRows(		
								g_pCTable,			// CTable pointer
								pBookmark,			// bookmark;
								cbBookmark,			// Length of bookmark
								1,						 // # rows to fetch
								0,						// Offset
								g_ulColNum,			// Which column to match
								i,						// row to match
								S_OK,					// HRESULT to verify
								1						// How many rows to expect.
											);

		if ( fTestPass == TEST_FAIL ) break;

		PROVIDER_FREE(pBookmark);
	}
	
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc pBookmark=random value.  Verify DB_E_BADBOOKMARK (warning-could get a provider specific error
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Scroll_BookMark::Variation_5()
{
	BYTE		pBookmark[4];

	memset(pBookmark, 0xCA, 4);

	return CallFindNextRows(	g_pCTable,			// CTable pointer
										pBookmark,			// bookmark;
										4,						// Length of bookmark
										1,						// # rows to fetch
										0,						// Offset
										g_ulColNum,			// Which column to match
										1,						// row to match
										DB_E_BADBOOKMARK,	// HRESULT to verify
										0						// How many rows to expect.
									);
}


// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc *pBookmark=2nd row and offset=-1, cRows=1.  Verify first row matched.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Scroll_BookMark::Variation_6()
{
	BOOL fTestPass = FALSE;
	ULONG_PTR cbBookmark;
	BYTE	*pBookmark;

	GetBookmark(2, &cbBookmark, &pBookmark);

	fTestPass= CallFindNextRows(	g_pCTable,			// CTable pointer
									pBookmark,			// bookmark;
									cbBookmark,			// Length of bookmark
									1,					   // # rows to fetch
									-1,					// Offset
									g_ulColNum,			// Which column to match
									1,						// row to match
									S_OK,		// HRESULT to verify
									1						// How many rows to expect.
								);
	PROVIDER_FREE(pBookmark);

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc *pBookmark=4th row, Offset=2, cRows=1 and match 5th row.  Verify DB_S_ENDOFROWSET
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Scroll_BookMark::Variation_7()
{
	BOOL fTestPass = FALSE;
	ULONG_PTR cbBookmark;
	BYTE	*pBookmark;

	GetBookmark(4, &cbBookmark, &pBookmark);

	fTestPass= CallFindNextRows(	g_pCTable,			// CTable pointer
									pBookmark,			// bookmark;
									cbBookmark,			// Length of bookmark
									1,					   // # rows to fetch
									2,						// Offset
									g_ulColNum,			// Which column to match
									5,						// row to match
									DB_S_ENDOFROWSET, // HRESULT to verify
									0						// How many rows to expect.
								);
	PROVIDER_FREE(pBookmark);

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc *pBookmark=5th row.  Offset=0, cRows=1 and match 5th. Again with pBookmark=NULL, Offset=0, cRows=1 and match 2nd.  Verify S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Scroll_BookMark::Variation_8()
{
	BOOL fTestPass = FALSE;
	ULONG_PTR cbBookmark;
	BYTE	*pBookmark;

	GetBookmark(5, &cbBookmark, &pBookmark);

	fTestPass= CallFindNextRows(	g_pCTable,			// CTable pointer
											pBookmark,			// bookmark;
											cbBookmark,			// Length of bookmark
											1,					   // # rows to fetch
											0,						// Offset
											g_ulColNum,			// Which column to match
											5,						// row to match
											S_OK,					// HRESULT to verify
											1						// How many rows to expect.
										);

	if ( !COMPARE(fTestPass, TRUE) ) 
		goto CLEANUP;

	fTestPass= CallFindNextRows(	g_pCTable,			// CTable pointer
											NULL,					// bookmark;
											0,						// Length of bookmark
											1,					   // # rows to fetch
											0,						// Offset
											g_ulColNum,			// Which column to match
											2,						// row to match
											S_OK,					// HRESULT to verify
											1						// How many rows to expect.
										);

CLEANUP:
	PROVIDER_FREE(pBookmark);

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc *pBookmark=2nd row and offset = -2, cRows=1, Verify DB_S_ENDOFROWSET
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Scroll_BookMark::Variation_9()
{
	BOOL fTestPass = FALSE;
	ULONG_PTR cbBookmark;
	BYTE	*pBookmark;

	GetBookmark(2, &cbBookmark, &pBookmark);

	fTestPass= CallFindNextRows(	g_pCTable,				// CTable pointer
									pBookmark,				// bookmark;
									cbBookmark,				// Length of bookmark
									1,						// # rows to fetch
									-2,						// Offset
									g_ulColNum,				// Which column to match
									1,						// row to match
									DB_S_ENDOFROWSET,		// HRESULT to verify
									0						// How many rows to expect.
								);
	PROVIDER_FREE(pBookmark);

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc pBookmark=DBBMK_FIRST, cRows=0 Should be no-op
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Scroll_BookMark::Variation_10()
{
	DBBOOKMARK	DBBookmark=DBBMK_FIRST;
	BYTE		*pBookmark=(BYTE *)&DBBookmark;

	return CallFindNextRows(	g_pCTable,			// CTable pointer
										pBookmark,			// bookmark;
										1,						// Length of bookmark
										0,					   // # rows to fetch
										0,						// Offset
										g_ulColNum,			// Which column to match
										g_lRowLast+1,				// row to match
										S_OK,					// HRESULT to verify
										0						// How many rows to expect.
									);
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Scroll_BookMark::Terminate()
{
	ReleaseRowsetAndAccessor();

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIRowsetFind::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(Scroll_Fetch_Bookmarks)
//*-----------------------------------------------------------------------
//| Test Case:		Scroll_Fetch_Bookmarks - CanScrollBackwards and CanFetchBackwards with Bookmarks
//|	Created:			06/24/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Scroll_Fetch_Bookmarks::Init()
{
	BOOL fTestPass = FALSE;
	DBPROPID guidPropertySet[4];
	ULONG	cPrptSet=0;

	if(!TCIRowsetFind::Init())
		return FALSE;

	TESTC_DRIVER(g_rgDBPrpt[IDX_OrderedBookmarks].fSupported &&
	   g_rgDBPrpt[IDX_FetchBackwards].fSupported &&
	   g_rgDBPrpt[IDX_ScrollBackwards].fSupported);

	if(!g_rgDBPrpt[IDX_FetchBackwards].fDefault)
		guidPropertySet[cPrptSet++]=DBPROP_CANFETCHBACKWARDS;
	
	if(!g_rgDBPrpt[IDX_ScrollBackwards].fDefault)
		guidPropertySet[cPrptSet++]=DBPROP_CANSCROLLBACKWARDS;
	
	if(!g_rgDBPrpt[IDX_OrderedBookmarks].fDefault)
		guidPropertySet[cPrptSet++]=DBPROP_ORDEREDBOOKMARKS;

	if(!g_rgDBPrpt[IDX_IRowsetLocate].fDefault)
		guidPropertySet[cPrptSet++]=DBPROP_IRowsetLocate;	

	//DBPROP_ORDEREDBOOKMARKS,DBPROP_CANFETCHBACKWARDS and DBPROP_CANSCROLLBACKWARDS
	//are requested 
	TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetFind,
		cPrptSet,guidPropertySet));

	fTestPass = TRUE;

CLEANUP:
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc pBookMark=DBBMK_FIRST, cRows=5 and match 1st row.  Verify S_OK and 5 hrows
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Scroll_Fetch_Bookmarks::Variation_1()
{
	DBBOOKMARK	DBBookmark=DBBMK_FIRST;
	BYTE		*pBookmark=(BYTE *)&DBBookmark;

	return CallFindNextRows(	g_pCTable,			// CTable pointer
										pBookmark,			// bookmark;
										1,						// Length of bookmark
										5,					   // # rows to fetch
										0,						// Offset
										g_ulColNum,			// Which column to match
										1,						// row to match
										S_OK,					// HRESULT to verify
										5						// How many rows to expect.
									);
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc pBookMarks=DBBMK_LAST,cRows=-5  match last row.  Verify S_OK and 5 hrows
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Scroll_Fetch_Bookmarks::Variation_2()
{
	DBBOOKMARK	DBBookmark=DBBMK_LAST;
	BYTE		*pBookmark=(BYTE *)&DBBookmark;

	return CallFindNextRows(	g_pCTable,			// CTable pointer
										pBookmark,			// bookmark;
										1,						// Length of bookmark
										-5,					// # rows to fetch
										0,						// Offset
										g_ulColNum,			// Which column to match
										g_lRowLast,			// row to match
										S_OK,					// HRESULT to verify
										5						// How many rows to expect.
									);
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc pBookmark=2nd row, cRows=2, Offset=1.  Match 4th row.  Verify S_OK and 2 hrows
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Scroll_Fetch_Bookmarks::Variation_3()
{
	BOOL		fTestPass;
	ULONG_PTR	cbBookmark;
	BYTE		*pBookmark=NULL;

	//get the bookmark for the 2th ro
	if(!GetBookmark(2,&cbBookmark, &pBookmark))
		return TEST_FAIL;

	fTestPass = CallFindNextRows(	g_pCTable,			// CTable pointer
											pBookmark,			// bookmark;
											cbBookmark,			// Length of bookmark
											2,					   // # rows to fetch
											1,						// Offset
											g_ulColNum,			// Which column to match
											g_lRowLast-1,		// row to match
											S_OK,					// HRESULT to verify
											2						// How many rows to expect.
										);

	PROVIDER_FREE(pBookmark);
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc pBookMark=5th row, cRows=-2.  Match3rd row.  Verify S_OK and 2 hrows
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Scroll_Fetch_Bookmarks::Variation_4()
{
	BOOL		fTestPass;
	ULONG_PTR	cbBookmark;
	BYTE		*pBookmark=NULL;

	//get the bookmark for the 2th row
	if(!GetBookmark(5,&cbBookmark, &pBookmark))
		return TEST_FAIL;

	fTestPass = CallFindNextRows(	g_pCTable,			// CTable pointer
											pBookmark,			// bookmark;
											cbBookmark,			// Length of bookmark
											-2,					// # rows to fetch
											2,						// Offset
											g_ulColNum,			// Which column to match
											3,						// row to match
											S_OK,					// HRESULT to verify
											2						// How many rows to expect.
										);

	PROVIDER_FREE(pBookmark);
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc pBookmark=Last row, cRows=1, Offset=1. Verify DB_S_ENDOFROWSET
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Scroll_Fetch_Bookmarks::Variation_5()
{
	BOOL		fTestPass;
	ULONG_PTR	cbBookmark;
	BYTE *		pBookmark=NULL;

	//get the bookmark for the last
	if(!GetBookmark(g_lRowLast,&cbBookmark, &pBookmark))
		return TEST_FAIL;

	fTestPass = CallFindNextRows(	g_pCTable,			// CTable pointer
											pBookmark,			// bookmark;
											cbBookmark,			// Length of bookmark
											-1,					// # rows to fetch
											1,						// Offset
											g_ulColNum,			// Which column to match
											g_lRowLast,			// row to match
											DB_S_ENDOFROWSET,			// HRESULT to verify
											0						// How many rows to expect.
										);

	PROVIDER_FREE(pBookmark);
	return fTestPass;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Scroll_Fetch_Bookmarks::Terminate()
{
	ReleaseRowsetAndAccessor();

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIRowsetFind::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(CaseSensitive_Compares)
//*-----------------------------------------------------------------------
//| Test Case:		CaseSensitive_Compares - Test the case sensitive property
//|	Created:			06/26/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL CaseSensitive_Compares::Init()
{
	DBPROPID guidPropertySet;
	ULONG cPrptSet=0;
	BOOL fTestPass = FALSE;

	if(!TCIRowsetFind::Init())
		return FALSE;

	//create a rowset and an accessor.  
	TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetFind,
		cPrptSet,&guidPropertySet));

CLEANUP:
	return TRUE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc EQ, sensitive
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CaseSensitive_Compares::Variation_1()
{
	return CompareOpTest(g_pCTable, DBCOMPAREOPS_EQ | DBCOMPAREOPS_CASESENSITIVE, SUBOP_EMPTY);
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc EQ, insensitive
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CaseSensitive_Compares::Variation_2()
{
	return CompareOpTest(g_pCTable, DBCOMPAREOPS_EQ | DBCOMPAREOPS_CASEINSENSITIVE, SUBOP_EMPTY);
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc LT, sensitive
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CaseSensitive_Compares::Variation_3()
{
	return CompareOpTest(g_pCTable, DBCOMPAREOPS_LT | DBCOMPAREOPS_CASESENSITIVE, SUBOP_EMPTY);
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc LT, insensitve
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CaseSensitive_Compares::Variation_4()
{
	return CompareOpTest(g_pCTable, DBCOMPAREOPS_LT | DBCOMPAREOPS_CASEINSENSITIVE, SUBOP_EMPTY);
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc LE, sensitive
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CaseSensitive_Compares::Variation_5()
{
	return CompareOpTest(g_pCTable, DBCOMPAREOPS_LE | DBCOMPAREOPS_CASESENSITIVE, SUBOP_EMPTY);
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc LE, insensitive
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CaseSensitive_Compares::Variation_6()
{
	return CompareOpTest(g_pCTable, DBCOMPAREOPS_LE | DBCOMPAREOPS_CASEINSENSITIVE, SUBOP_EMPTY);
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc GT, sensitive
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CaseSensitive_Compares::Variation_7()
{
	return CompareOpTest(g_pCTable, DBCOMPAREOPS_GT | DBCOMPAREOPS_CASESENSITIVE, SUBOP_EMPTY);
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc GT, insensitive
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CaseSensitive_Compares::Variation_8()
{
	return CompareOpTest(g_pCTable, DBCOMPAREOPS_GT | DBCOMPAREOPS_CASEINSENSITIVE, SUBOP_EMPTY);
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc GE, sensitive
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CaseSensitive_Compares::Variation_9()
{
	return CompareOpTest(g_pCTable, DBCOMPAREOPS_GE | DBCOMPAREOPS_CASESENSITIVE, SUBOP_EMPTY);
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc GE, insensitve
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CaseSensitive_Compares::Variation_10()
{
	return CompareOpTest(g_pCTable, DBCOMPAREOPS_GE | DBCOMPAREOPS_CASEINSENSITIVE, SUBOP_EMPTY);
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc NE, sensitive
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CaseSensitive_Compares::Variation_11()
{
	return CompareOpTest(g_pCTable, DBCOMPAREOPS_NE | DBCOMPAREOPS_CASESENSITIVE, SUBOP_EMPTY);
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc NE, insensitive
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CaseSensitive_Compares::Variation_12()
{
	return CompareOpTest(g_pCTable, DBCOMPAREOPS_NE | DBCOMPAREOPS_CASEINSENSITIVE, SUBOP_EMPTY);
}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc BEGINSWITH, sensitve
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CaseSensitive_Compares::Variation_13()
{
	return CompareOpTest(g_pCTable, DBCOMPAREOPS_BEGINSWITH | DBCOMPAREOPS_CASESENSITIVE, SUBOP_EMPTY);
}
// }}


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc BEGINSWITH, insensitive
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CaseSensitive_Compares::Variation_14()
{
	return CompareOpTest(g_pCTable, DBCOMPAREOPS_BEGINSWITH | DBCOMPAREOPS_CASEINSENSITIVE, SUBOP_EMPTY);
}
// }}


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc BEGINSWITH, sensitive negative match
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CaseSensitive_Compares::Variation_15()
{
	BOOL		fTestPass = TEST_SKIPPED;
	DBCOUNTITEM cRowsObtained = 0;
	DBORDINAL	ulColToFind;
	DBTYPE		wColType;
	HROW *		rghRows = NULL;
	CCol		TempCol;
	
	m_pIRowset->RestartPosition(DB_NULL_HCHAPTER);

	if (!GetVariableLengthStrAndUpdatable(&ulColToFind, DBCOMPAREOPS_BEGINSWITH, FALSE, &wColType))
		goto CLEANUP;

	g_pCTable->GetColInfo(ulColToFind, TempCol);
	if(!ValidateCompareOp(TC_FindCompareOps(TempCol.GetColID()), DBCOMPAREOPS_BEGINSWITH | DBCOMPAREOPS_CASESENSITIVE))
		return TEST_SKIPPED;

	//setup
	//set up a INSENSITIVE match binding
	TESTC(CreateFindValueAccessor(DBCOMPAREOPS_BEGINSWITH | DBCOMPAREOPS_CASEINSENSITIVE, g_pCTable, 1, ulColToFind, wColType, SUBOP_EMPTY));

	// But actually use a CASESENSITIVE operator
	m_hr = m_pIRowsetFind->FindNextRow(DB_NULL_HCHAPTER, m_hRowsetFindAccessor, m_pFindValue, 
													DBCOMPAREOPS_BEGINSWITH | DBCOMPAREOPS_CASESENSITIVE, 0, NULL, 0, 
													1, &cRowsObtained, &rghRows);
	// Verify HRESULT
	if ( m_hr == DB_E_BADCOMPAREOP )
	{
		// The provider only supported case insensitive compares
		fTestPass = TEST_SKIPPED; 
		goto CLEANUP;
	}
	else if (!CHECK(m_hr, DB_S_ENDOFROWSET))
	{
		odtLog << "Found a match, when the match should have failed." << ENDL;
		fTestPass = TEST_FAIL;
		goto CLEANUP;
	}

	fTestPass = TEST_PASS;

CLEANUP:
	if (cRowsObtained>0)
	{
		m_pIRowset->ReleaseRows(cRowsObtained, rghRows, NULL, NULL, NULL);
		PROVIDER_FREE(rghRows);
	}
	ReleaseFindValueAccessor(wColType);
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc CONTAINS, sensitive
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CaseSensitive_Compares::Variation_16()
{
	return CompareOpTest(g_pCTable, DBCOMPAREOPS_CONTAINS | DBCOMPAREOPS_CASESENSITIVE, SUBOP_CONTAINS_BEGIN);
}
// }}


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc CONTAINS, insensitive
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CaseSensitive_Compares::Variation_17()
{
	return CompareOpTest(g_pCTable, DBCOMPAREOPS_CONTAINS | DBCOMPAREOPS_CASEINSENSITIVE, SUBOP_CONTAINS_BEGIN);
}
// }}


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc CONTAINS, sensitive negative match
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CaseSensitive_Compares::Variation_18()
{
	BOOL		fTestPass = TEST_SKIPPED;
	DBCOUNTITEM cRowsObtained = 0;
	DBORDINAL	ulColToFind;
	DBTYPE		wColType;
	HROW *		rghRows = NULL;
	CCol		TempCol;

	m_pIRowset->RestartPosition(DB_NULL_HCHAPTER);
	
	if (!GetVariableLengthStrAndUpdatable(&ulColToFind, DBCOMPAREOPS_CONTAINS, FALSE, &wColType))
		goto CLEANUP;

	g_pCTable->GetColInfo(ulColToFind, TempCol);
	if(!ValidateCompareOp(TC_FindCompareOps(TempCol.GetColID()), DBCOMPAREOPS_CONTAINS | DBCOMPAREOPS_CASESENSITIVE))
		return TEST_SKIPPED;

	//setup
	//set up a INSENSITIVE match binding
	TESTC(CreateFindValueAccessor(DBCOMPAREOPS_CONTAINS | DBCOMPAREOPS_CASEINSENSITIVE, g_pCTable, 1, ulColToFind, wColType, SUBOP_CONTAINS_BEGIN));

	// But actually use a CASESENSITIVE operator
	m_hr = m_pIRowsetFind->FindNextRow(DB_NULL_HCHAPTER, m_hRowsetFindAccessor, m_pFindValue, 
													DBCOMPAREOPS_CONTAINS | DBCOMPAREOPS_CASESENSITIVE, 0, NULL, 0, 
													1, &cRowsObtained, &rghRows);
	// Verify HRESULT
	if ( m_hr == DB_E_BADCOMPAREOP )
	{
		// The provider only supported case insensitive compares
		fTestPass = TEST_SKIPPED; 
		goto CLEANUP;
	}
	else if (!CHECK(m_hr, DB_S_ENDOFROWSET))
	{
		odtLog << "Found a match, when the match should have failed." << ENDL;
		fTestPass = TEST_FAIL;
	}

CLEANUP:
	if (cRowsObtained>0)
	{
		m_pIRowset->ReleaseRows(cRowsObtained, rghRows, NULL, NULL, NULL);
		PROVIDER_FREE(rghRows);
	}
	ReleaseFindValueAccessor(wColType);
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc NOTBEGINSWITH, sensitive
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CaseSensitive_Compares::Variation_19()
{
	BOOL		fTestPass = TEST_SKIPPED;
	DBCOUNTITEM cRowsObtained = 0;
	DBORDINAL	ulColToFind;
	DBTYPE		wColType;
	HROW *		rghRows = NULL;
	
	m_pIRowset->RestartPosition(DB_NULL_HCHAPTER);

	if (!GetVariableLengthStrAndUpdatable(&ulColToFind, DBCOMPAREOPS_NOTBEGINSWITH, FALSE, &wColType))
		goto CLEANUP;

	//setup
	//set up a INSENSITIVE BEGINSWITH match binding to create lower case version of the data
	TESTC(CreateFindValueAccessor(DBCOMPAREOPS_BEGINSWITH | DBCOMPAREOPS_CASEINSENSITIVE, g_pCTable, 1, ulColToFind, wColType, SUBOP_EMPTY));

	// But actually use a CASESENSITIVE with NOTBEGINSWITH operator
	m_hr = m_pIRowsetFind->FindNextRow(DB_NULL_HCHAPTER, m_hRowsetFindAccessor, m_pFindValue, 
													DBCOMPAREOPS_NOTBEGINSWITH | DBCOMPAREOPS_CASESENSITIVE, 0, NULL, 0, 
													1, &cRowsObtained, &rghRows);
	// Verify HRESULT
	if ( m_hr == DB_E_BADCOMPAREOP )
	{
		// The provider only supported case insensitive compares
		fTestPass = TEST_SKIPPED; 
		goto CLEANUP;
	}
	else if (!CHECK(m_hr, S_OK))
	{
		odtLog << "Didn't find a match." << ENDL;
		fTestPass = TEST_FAIL;
	}

	if(!COMPARE(cRowsObtained,1) || !COMPARE(VerifyRowPosition(rghRows[0], 1, g_pCTable), TRUE))
		fTestPass = TEST_FAIL;
	else
		fTestPass = TEST_PASS;

CLEANUP:
	if (cRowsObtained>0)
	{
		m_pIRowset->ReleaseRows(cRowsObtained, rghRows, NULL, NULL, NULL);
		PROVIDER_FREE(rghRows);
	}
	ReleaseFindValueAccessor(wColType);
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc NOTBEGINSWITH, insensitive
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CaseSensitive_Compares::Variation_20()
{
	return CompareOpTest(g_pCTable, DBCOMPAREOPS_NOTBEGINSWITH | DBCOMPAREOPS_CASEINSENSITIVE, SUBOP_EMPTY);
}
// }}


// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc NOTBEGINSWITH, sensitive negative match
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CaseSensitive_Compares::Variation_21()
{
	BOOL		fTestPass = TEST_SKIPPED;
	DBCOUNTITEM cRowsObtained = 0;
	DBORDINAL	ulColToFind;
	DBTYPE		wColType;
	HROW *		rghRows = NULL;
	CCol		TempCol;

	m_pIRowset->RestartPosition(DB_NULL_HCHAPTER);

	if (!GetVariableLengthStrAndUpdatable(&ulColToFind, DBCOMPAREOPS_NOTBEGINSWITH, FALSE, &wColType))
		goto CLEANUP;

	g_pCTable->GetColInfo(ulColToFind, TempCol);
	if(!ValidateCompareOp(TC_FindCompareOps(TempCol.GetColID()), DBCOMPAREOPS_NOTBEGINSWITH | DBCOMPAREOPS_CASEINSENSITIVE))
		return TEST_SKIPPED;

	//setup
	//set up a BEGINSWITH INSENSITIVE match binding
	TESTC(CreateFindValueAccessor(DBCOMPAREOPS_BEGINSWITH | DBCOMPAREOPS_CASEINSENSITIVE, g_pCTable, 1, ulColToFind, wColType, SUBOP_EMPTY));

	// Use a CASEINSENSITIVE operator
	m_hr = m_pIRowsetFind->FindNextRow(DB_NULL_HCHAPTER, m_hRowsetFindAccessor, m_pFindValue, 
													DBCOMPAREOPS_NOTBEGINSWITH | DBCOMPAREOPS_CASEINSENSITIVE, 0, NULL, 0, 
													1, &cRowsObtained, &rghRows);
	// Verify HRESULT
	if ( m_hr == DB_E_BADCOMPAREOP )
	{
		// The provider only supported case insensitive compares
		fTestPass = TEST_SKIPPED; 
		goto CLEANUP;
	}
	else if (cRowsObtained != 0 && VerifyRowPosition(rghRows[0], 1, g_pCTable))
	{		
		odtLog << "Found a match on first row, when the match should have failed." << ENDL;
		fTestPass = TEST_FAIL;
		goto CLEANUP;
	}

	fTestPass = TEST_PASS;

CLEANUP:
	if (cRowsObtained>0)
	{
		m_pIRowset->ReleaseRows(cRowsObtained, rghRows, NULL, NULL, NULL);
		PROVIDER_FREE(rghRows);
	}
	ReleaseFindValueAccessor(wColType);
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(22)
//*-----------------------------------------------------------------------
// @mfunc NOTCONTAINS, sensitive
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CaseSensitive_Compares::Variation_22()
{
	BOOL		fTestPass = TEST_SKIPPED;
	DBCOUNTITEM cRowsObtained = 0;
	DBORDINAL	ulColToFind;
	DBTYPE		wColType;
	HROW *		rghRows = NULL;
	CCol		TempCol;
	
	m_pIRowset->RestartPosition(DB_NULL_HCHAPTER);

	if (!GetVariableLengthStrAndUpdatable(&ulColToFind, DBCOMPAREOPS_NOTCONTAINS, FALSE, &wColType))
		goto CLEANUP;

	g_pCTable->GetColInfo(ulColToFind, TempCol);
	if(!ValidateCompareOp(TC_FindCompareOps(TempCol.GetColID()), DBCOMPAREOPS_NOTCONTAINS | DBCOMPAREOPS_CASESENSITIVE))
		return TEST_SKIPPED;

	//setup
	//set up a INSENSITIVE CONTAINS match binding to create lower case version of the data
	TESTC(CreateFindValueAccessor(DBCOMPAREOPS_CONTAINS | DBCOMPAREOPS_CASEINSENSITIVE, g_pCTable, 1, ulColToFind, wColType, SUBOP_CONTAINS_BEGIN));

	// But actually use a CASESENSITIVE with NOTBEGINSWITH operator
	m_hr = m_pIRowsetFind->FindNextRow(DB_NULL_HCHAPTER, m_hRowsetFindAccessor, m_pFindValue, 
													DBCOMPAREOPS_NOTCONTAINS | DBCOMPAREOPS_CASESENSITIVE, 0, NULL, 0, 
													1, &cRowsObtained, &rghRows);
	// Verify HRESULT
	if ( m_hr == DB_E_BADCOMPAREOP )
	{
		// The provider only supported case insensitive compares
		fTestPass = TEST_SKIPPED; 
		goto CLEANUP;
	}
	else if (!CHECK(m_hr, S_OK))
	{
		odtLog << "Didn't find a match, but should have" << ENDL;
		fTestPass = TEST_FAIL;
	}

	if(!COMPARE(cRowsObtained,1) || !COMPARE(VerifyRowPosition(rghRows[0], 1, g_pCTable), TRUE))
		fTestPass = TEST_FAIL;
	else
		fTestPass = TEST_PASS;

CLEANUP:
	if (cRowsObtained>0)
	{
		m_pIRowset->ReleaseRows(cRowsObtained, rghRows, NULL, NULL, NULL);
		PROVIDER_FREE(rghRows);
	}
	ReleaseFindValueAccessor(wColType);
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(23)
//*-----------------------------------------------------------------------
// @mfunc NOCONTAINS, insensitive
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CaseSensitive_Compares::Variation_23()
{
	return CompareOpTest(g_pCTable, DBCOMPAREOPS_NOTCONTAINS | DBCOMPAREOPS_CASEINSENSITIVE, SUBOP_CONTAINS_BEGIN);
}
// }}


// {{ TCW_VAR_PROTOTYPE(24)
//*-----------------------------------------------------------------------
// @mfunc NOCONTAINS, sensitive negative match
//
// @rdesc TEST_PASS or TEST_FAIL
//
int CaseSensitive_Compares::Variation_24()
{
	BOOL		fTestPass = TEST_SKIPPED;
	DBCOUNTITEM cRowsObtained = 0;
	DBORDINAL	ulColToFind;
	DBTYPE		wColType;
	HROW	*	rghRows = NULL;
	CCol		TempCol;

	m_pIRowset->RestartPosition(DB_NULL_HCHAPTER);

	if (!GetVariableLengthStrAndUpdatable(&ulColToFind, DBCOMPAREOPS_NOTCONTAINS, FALSE, &wColType))
		goto CLEANUP;

	g_pCTable->GetColInfo(ulColToFind, TempCol);
	if(!ValidateCompareOp(TC_FindCompareOps(TempCol.GetColID()), DBCOMPAREOPS_CONTAINS | DBCOMPAREOPS_CASESENSITIVE))
		return TEST_SKIPPED;

	//setup
	//set up a CONTAINS INSENSITIVE match binding to generate lower case data
	TESTC(CreateFindValueAccessor(DBCOMPAREOPS_CONTAINS | DBCOMPAREOPS_CASEINSENSITIVE, g_pCTable, 1, ulColToFind, wColType, SUBOP_CONTAINS_BEGIN));

	// Use a CASEINSENSITIVE operator
	m_hr = m_pIRowsetFind->FindNextRow(DB_NULL_HCHAPTER, m_hRowsetFindAccessor, m_pFindValue, 
													DBCOMPAREOPS_NOTCONTAINS | DBCOMPAREOPS_CASEINSENSITIVE, 0, NULL, 0, 
													1, &cRowsObtained, &rghRows);
	// Verify HRESULT
	if ( m_hr == DB_E_BADCOMPAREOP )
	{
		// The provider only supported case insensitive compares
		fTestPass = TEST_SKIPPED; 
		goto CLEANUP;
	}
	else if (cRowsObtained != 0 && VerifyRowPosition(rghRows[0], 1, g_pCTable))
	{		
		odtLog << "Found a match on first row, when the match should have failed." << ENDL;
		fTestPass = TEST_FAIL;
		goto CLEANUP;
	}

	fTestPass = TEST_PASS;

CLEANUP:
	if (cRowsObtained>0)
	{
		m_pIRowset->ReleaseRows(cRowsObtained, rghRows, NULL, NULL, NULL);
		PROVIDER_FREE(rghRows);
	}
	ReleaseFindValueAccessor(wColType);
	return fTestPass;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL CaseSensitive_Compares::Terminate()
{
	ReleaseRowsetAndAccessor();
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIRowsetFind::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(Scroll_Fetch_Hold)
//*-----------------------------------------------------------------------
//| Test Case:		Scroll_Fetch_Hold - Test with CanScrollBack, CanFetchBack, and CanHoldRows
//|	Created:			06/26/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Scroll_Fetch_Hold::Init()
{
	DBPROPID	guidPropertySet[3];
	ULONG	cPrptSet=0;
	BOOL fTestPass = FALSE;

	if(!TCIRowsetFind::Init())
		return FALSE;

	TESTC_DRIVER(g_rgDBPrpt[IDX_FetchBackwards].fSupported &&
	   g_rgDBPrpt[IDX_IRowsetLocate].fSupported &&
	   g_rgDBPrpt[IDX_CanHoldRows].fSupported);

	if(!g_rgDBPrpt[IDX_FetchBackwards].fDefault)
		guidPropertySet[cPrptSet++]=DBPROP_CANFETCHBACKWARDS;
		
	if(!g_rgDBPrpt[IDX_CanHoldRows].fDefault)
		guidPropertySet[cPrptSet++]=DBPROP_CANHOLDROWS;

	if(!g_rgDBPrpt[IDX_IRowsetLocate].fDefault)
		guidPropertySet[cPrptSet++]=DBPROP_IRowsetLocate;
	

	//create a rowset and an accessor.  
	//DBPROP_CANFETCHBACKWARDS and DBPROP_CANSCROLLBACKWARDS  and DBPROP_CANHOLDROWS
	//are requested 
	TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetFind,
		cPrptSet,guidPropertySet));

	fTestPass = TRUE;

CLEANUP:
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc pBookmark=2nd row, cRows=-1.  Verify S_OK and one row handle
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Scroll_Fetch_Hold::Variation_1()
{
	BOOL		fTestPass;
	ULONG_PTR	cbBookmark;
	BYTE		*pBookmark=NULL;

	//get the bookmark for the 2th row
	if(!GetBookmark(2,&cbBookmark, &pBookmark))
		return TEST_FAIL;

	fTestPass = CallFindNextRows(	g_pCTable,			// CTable pointer
											pBookmark,			// bookmark;
											cbBookmark,			// Length of bookmark
											-1,			  	   // # rows to fetch
											-1,					// Offset
											g_ulColNum,			// Which column to match
											1,						// row to match
											S_OK,					// HRESULT to verify
											1						// How many rows to expect.
										);

	PROVIDER_FREE(pBookmark);
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc pBookmark=3rd row.  cRows=3.  Match 3rd row.  Do not release. pBookmark=4th row, cRows=2, match 4th.  Release 1st 3, ver last 2
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Scroll_Fetch_Hold::Variation_2()
{
	BOOL fTestPass = TEST_PASS;
	ULONG_PTR		rgcbBookmarks[2];
	BYTE			*rgpBookmarks[2]={NULL, NULL};
	HROW *phRows1, *phRows2;
	
	phRows1 = (HROW *) PROVIDER_ALLOC( 3 * sizeof(HROW) );
	phRows2 = (HROW *) PROVIDER_ALLOC( 2 * sizeof(HROW) );
	
	//get the bookmark for the 3rd row
	if(!GetBookmark(3,&rgcbBookmarks[0],&rgpBookmarks[0]))
		return TEST_FAIL;

	//get the bookmark for the 4th row 
	if(!GetBookmark(4,&rgcbBookmarks[1],&rgpBookmarks[1]))
		goto CLEANUP;

	fTestPass = CallFindNextRows(	g_pCTable,			// CTable pointer
											rgpBookmarks[0],	// bookmark;
											rgcbBookmarks[0],	// Length of bookmark
											3,						// # rows to fetch
											0,						// Offset
											g_ulColNum,			// Which column to match
											3,						// row to match
											S_OK,					// HRESULT to verify
											3,						// How many rows to expect.
											FALSE,				// flag to Release rows (optional)
											DBCOMPAREOPS_EQ,  // Any particular preference for comparing? (optional)
											SUBOP_EMPTY,		// Some comparisions are rich enough to deserve a mulitple comparision operations
											phRows1				// optional arg if client wants to control row handle mem									
										);

	if ( fTestPass == TEST_FAIL ) goto CLEANUP;

	fTestPass = CallFindNextRows(	g_pCTable,			// CTable pointer
											rgpBookmarks[1],	// bookmark;
											rgcbBookmarks[1],	// Length of bookmark
											2,						// # rows to fetch
											0,						// Offset
											g_ulColNum,			// Which column to match
											4,						// row to match
											S_OK,					// HRESULT to verify
											2,						// How many rows to expect.
											FALSE,				// flag to Release rows (optional)
											DBCOMPAREOPS_EQ,  // Any particular preference for comparing? (optional)
											SUBOP_EMPTY,		// Some comparisions are rich enough to deserve a mulitple comparision operations
											phRows2				// optional arg if client wants to control row handle mem										
										);

	m_pIRowset->ReleaseRows(3, phRows1, NULL, NULL, NULL);

	COMPARE(VerifyRowPosition(phRows2[0], 4, g_pCTable), TRUE);	
	COMPARE(VerifyRowPosition(phRows2[1], 5, g_pCTable), TRUE);	

	m_pIRowset->ReleaseRows(2, phRows2, NULL, NULL, NULL);

CLEANUP:

	PROVIDER_FREE(phRows1);
	PROVIDER_FREE(phRows2);

	//free memory pointed by the bookmarks
	if(rgpBookmarks[0])
		PROVIDER_FREE(rgpBookmarks[0]);

	if(rgpBookmarks[1])
		PROVIDER_FREE(rgpBookmarks[1]);

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc pBookmark=5th,cRows=-5, match 5th. S_OK,5 hrows. pBookmark=4th,cRows=-5. DB_S_ENDOFROWSET and 4 hrows. Release last 4, check 5
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Scroll_Fetch_Hold::Variation_3()
{
	BOOL fTestPass = TEST_PASS;
	ULONG_PTR		rgcbBookmarks[2];
	BYTE			*rgpBookmarks[2]={NULL, NULL};
	HROW *phRows1, *phRows2;
	
	phRows1 = (HROW *) PROVIDER_ALLOC( 5 * sizeof(HROW) );
	phRows2 = (HROW *) PROVIDER_ALLOC( 4 * sizeof(HROW) );
	
	//get the bookmark for the 5th row
	if(!GetBookmark(5,&rgcbBookmarks[0],&rgpBookmarks[0]))
		return TEST_FAIL;

	//get the bookmark for the 4th row 
	if(!GetBookmark(4,&rgcbBookmarks[1],&rgpBookmarks[1]))
		goto CLEANUP;

	fTestPass = CallFindNextRows(	g_pCTable,			// CTable pointer
											rgpBookmarks[0],	// bookmark;
											rgcbBookmarks[0],	// Length of bookmark
											-5,					// # rows to fetch
											0,						// Offset
											g_ulColNum,			// Which column to match
											5,						// row to match
											S_OK,					// HRESULT to verify
											5,						// How many rows to expect.
											FALSE,				// flag to Release rows (optional)
											DBCOMPAREOPS_EQ,  // Any particular preference for comparing? (optional)
											SUBOP_EMPTY,		// Some comparisions are rich enough to deserve a mulitple comparision operations
											phRows1				// optional arg if client wants to control row handle mem									
										);

	if ( fTestPass == TEST_FAIL ) goto CLEANUP;

	fTestPass = CallFindNextRows(	g_pCTable,			// CTable pointer
											rgpBookmarks[1],	// bookmark;
											rgcbBookmarks[1],	// Length of bookmark
											-5,					// # rows to fetch
											0,						// Offset
											g_ulColNum,			// Which column to match
											4,						// row to match
											DB_S_ENDOFROWSET,	// HRESULT to verify
											4,						// How many rows to expect.
											FALSE,				// flag to Release rows (optional)
											DBCOMPAREOPS_EQ,  // Any particular preference for comparing? (optional)
											SUBOP_EMPTY,		// Some comparisions are rich enough to deserve a mulitple comparision operations
											phRows2				// optional arg if client wants to control row handle mem										
										);

	m_pIRowset->ReleaseRows(4, phRows2, NULL, NULL, NULL);

	VerifyRowPosition(phRows1[0], 5, g_pCTable);	
	VerifyRowPosition(phRows1[1], 4, g_pCTable);	
	VerifyRowPosition(phRows1[1], 3, g_pCTable);	
	VerifyRowPosition(phRows1[1], 2, g_pCTable);	
	VerifyRowPosition(phRows1[1], 1, g_pCTable);	

	m_pIRowset->ReleaseRows(5, phRows1, NULL, NULL, NULL);

CLEANUP:

	PROVIDER_FREE(phRows1);
	PROVIDER_FREE(phRows2);

	//free memory pointed by the bookmarks
	if(rgpBookmarks[0])
		PROVIDER_FREE(rgpBookmarks[0]);

	if(rgpBookmarks[1])
		PROVIDER_FREE(rgpBookmarks[1]);

	return fTestPass;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Scroll_Fetch_Hold::Terminate()
{
	ReleaseRowsetAndAccessor();

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIRowsetFind::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(Dynamic)
//*-----------------------------------------------------------------------
//| Test Case:		Dynamic - Test with OTHERINSERT and OTHERUPDATE properties
//|	Created:			06/27/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Dynamic::Init()
{
	DBPROPID	guidPropertySet[6];
	ULONG	cPrptSet=0;
	BOOL fTestPass = FALSE;

	if(!TCIRowsetFind::Init())
		return FALSE;

	if ( !AlteringRowsIsOK() )
		return FALSE;

	TESTC_DRIVER(g_rgDBPrpt[IDX_FetchBackwards].fSupported &&
	   g_rgDBPrpt[IDX_ScrollBackwards].fSupported &&
	   g_rgDBPrpt[IDX_CanHoldRows].fSupported);

	if(!g_rgDBPrpt[IDX_FetchBackwards].fDefault)
		guidPropertySet[cPrptSet++]=DBPROP_CANFETCHBACKWARDS;

	if(!g_rgDBPrpt[IDX_ScrollBackwards].fDefault)
		guidPropertySet[cPrptSet++]=DBPROP_CANSCROLLBACKWARDS;
		
	if(!g_rgDBPrpt[IDX_CanHoldRows].fDefault)
		guidPropertySet[cPrptSet++]=DBPROP_CANHOLDROWS;

	if(!g_rgDBPrpt[IDX_OtherInsert].fDefault)
		guidPropertySet[cPrptSet++]=DBPROP_OTHERINSERT;
		

	if(!g_rgDBPrpt[IDX_IRowsetChange].fDefault)
	{
		guidPropertySet[cPrptSet++]=DBPROP_IRowsetChange;
		guidPropertySet[cPrptSet++]=DBPROP_UPDATABILITY;
	}
	
	//create a rowset and an accessor.  
	//DBPROP_CANFETCHBACKWARDS and DBPROP_CANSCROLLBACKWARDS  and DBPROP_CANHOLDROWS and DBPROP_OTHERINSERT
	//are requested 
	if(!GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetFind,
		cPrptSet,guidPropertySet))
		return TEST_SKIPPED;

	fTestPass = TRUE;

CLEANUP:
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Insert a new row at end.  pBookmark=NULL,cRows=1.  Match new row.  Verify S_OK and one row handle
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Dynamic::Variation_1()
{
	BOOL fTestPass;

	// Insert a new row.
	if (FAILED(g_pCTable->Insert()))
		return TEST_FAIL;

	//restart the cursor position
	if(!CHECK(RestartRowPosition(),S_OK))
		return TEST_FAIL;

	fTestPass = CallFindNextRows(	g_pCTable,		// CTable pointer
											NULL,				// bookmark;
											0,					// Length of bookmark
											1,					// # rows to fetch
											FALSE,			// skip current row
											g_ulColNum,		// Which column to match
											g_lRowLast+1,	// row to match
											S_OK,	// HRESULT to verify
											1						// How many rows to expect.
										);		
	PopulateTable();

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Insert a new row at the end.  pBookmark=DBBMK_LAST,cRows=1.  DBCOMPAREOPS_IGNORE.  Verify S_OK and new row matched.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Dynamic::Variation_2()
{
	BOOL fTestPass;
	DBBOOKMARK	DBBookmark=DBBMK_LAST;
	BYTE		*pBookmark=(BYTE *)&DBBookmark;

	// Insert a new row.
	if (FAILED(g_pCTable->Insert()))
		return TEST_FAIL;

	//restart the cursor position
	if(!CHECK(RestartRowPosition(),S_OK))
		return TEST_FAIL;

	fTestPass = CallFindNextRows(	g_pCTable,		// CTable pointer
											pBookmark,				// bookmark;
											1,					// Length of bookmark
											1,					// # rows to fetch
											FALSE,			// skip current row
											g_ulColNum,		// Which column to match
											g_lRowLast+1,	// row to match
											S_OK,	// HRESULT to verify
											1						// How many rows to expect.
										);		
	PopulateTable();

	return fTestPass;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Dynamic::Terminate()
{
	ReleaseRowsetAndAccessor();

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIRowsetFind::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(MaxRows)
//*-----------------------------------------------------------------------
//| Test Case:		MaxRows - Test MaxRows property
//|	Created:			06/27/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL MaxRows::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCIRowsetFind::Init())
	// }}
	{
		// TO DO:  Add your own code here
		return TRUE;
	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Set MAXROWS=3. pBookmark=2nd row. cRows=4. Match 3rd row. DB_S_ENDOFROWSET and 3 hrows
//
// @rdesc TEST_PASS or TEST_FAIL
//
int MaxRows::Variation_1()
{
	// TO DO:  Add your own code here
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Set MAXROWS=2. pBookmark=3rd row,cRows=3. Match 3rd row.  DB_S_ROWLIMITEXECEEDED and 2 hrows
//
// @rdesc TEST_PASS or TEST_FAIL
//
int MaxRows::Variation_2()
{
	// TO DO:  Add your own code here
	return TEST_PASS;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL MaxRows::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIRowsetFind::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(EmptyRowset)
//*-----------------------------------------------------------------------
//| Test Case:		EmptyRowset - EmptyRowset
//|	Created:			06/27/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL EmptyRowset::Init()
{
	DBPROPID	guidPropertySet[2];
	ULONG	cPrptSet=0;
	BOOL fTestPass = FALSE;

	if (!g_pEmptyTable)
		return TEST_SKIPPED;

	if(!TCIRowsetFind::Init())
		return FALSE;

	TESTC_DRIVER(g_rgDBPrpt[IDX_FetchBackwards].fSupported &&
	   g_rgDBPrpt[IDX_ScrollBackwards].fSupported );

	if(!g_rgDBPrpt[IDX_FetchBackwards].fDefault)
	{
		guidPropertySet[cPrptSet]=DBPROP_CANFETCHBACKWARDS;
		cPrptSet++;
	}

	if(!g_rgDBPrpt[IDX_ScrollBackwards].fDefault)
	{
		guidPropertySet[cPrptSet]=DBPROP_CANSCROLLBACKWARDS;
		cPrptSet++;
	}

	//create a rowset and an accessor.  
	//DBPROP_CANFETCHBACKWARDS and DBPROP_CANSCROLLBACKWARDS are requested 
	TESTC_DRIVER(GetRowsetAndAccessor(g_pEmptyTable, SELECT_ORDERBYNUMERIC, IID_IRowsetFind,
		cPrptSet,guidPropertySet));

	fTestPass = TRUE;

CLEANUP:
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc pBookmark=NULL, cRows=0. DBCOMPAREOPS_IGNORE.  Verify DB_S_ENDOFROWSET
//
// @rdesc TEST_PASS or TEST_FAIL
//
int EmptyRowset::Variation_1()
{
	//restart the cursor position
	if(!CHECK(RestartRowPosition(),S_OK))
		return TEST_FAIL;

	return CallFindNextRows(	g_pEmptyTable,	// CTable pointer
										NULL,				// bookmark;
										0,					// Length of bookmark
										0,					// # rows to fetch
										FALSE,			// skip current row
										g_ulColNum,		// Which column to match
										1,					// row to match
										DB_S_ENDOFROWSET,	// HRESULT to verify
										0,						// How many rows to expect.
										TRUE,					// flag to Release rows (optional)
										DBCOMPAREOPS_IGNORE,  // Any particular preference for comparing? (optional)
										SUBOP_EMPTY	// Some comparisions are rich enough to deserve a mulitple comparision operations
									);									
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc pBookmark=NULL, cRows=1.  DB_S_ENDOFROWSET
//
// @rdesc TEST_PASS or TEST_FAIL
//
int EmptyRowset::Variation_2()
{
	//restart the cursor position
	if(!CHECK(RestartRowPosition(),S_OK))
		return TEST_FAIL;

	return CallFindNextRows(	g_pEmptyTable,	// CTable pointer
										NULL,				// bookmark;
										0,					// Length of bookmark
										1,					// # rows to fetch
										FALSE,			// skip current row
										g_ulColNum,		// Which column to match
										1,					// row to match
										DB_S_ENDOFROWSET,	// HRESULT to verify
										0,						// How many rows to expect.
										TRUE,					// flag to Release rows (optional)
										DBCOMPAREOPS_IGNORE,  // Any particular preference for comparing? (optional)
										SUBOP_EMPTY	// Some comparisions are rich enough to deserve a mulitple comparision operations
									);	
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc pBookmark=NULL, cRows=-1. DB_S_ENDOFROWSET
//
// @rdesc TEST_PASS or TEST_FAIL
//
int EmptyRowset::Variation_3()
{
	//restart the cursor position
	if(!CHECK(RestartRowPosition(),S_OK))
		return TEST_FAIL;

	return CallFindNextRows(	g_pEmptyTable,	// CTable pointer
										NULL,				// bookmark;
										0,					// Length of bookmark
										-1,					// # rows to fetch
										FALSE,			// skip current row
										g_ulColNum,		// Which column to match
										1,					// row to match
										DB_S_ENDOFROWSET,	// HRESULT to verify
										0,						// How many rows to expect.
										TRUE,					// flag to Release rows (optional)
										DBCOMPAREOPS_IGNORE,  // Any particular preference for comparing? (optional)
										SUBOP_EMPTY	// Some comparisions are rich enough to deserve a mulitple comparision operations
									);	
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc pBookmark=NULL,cRows=0,fSkip=TRUE.  Verify DB_S_ENDOFROWSET
//
// @rdesc TEST_PASS or TEST_FAIL
//
int EmptyRowset::Variation_4()
{
	//restart the cursor position
	if(!CHECK(RestartRowPosition(),S_OK))
		return TEST_FAIL;

	return CallFindNextRows(	g_pEmptyTable,	// CTable pointer
										NULL,				// bookmark;
										0,					// Length of bookmark
										0,					// # rows to fetch
										TRUE,			// skip current row
										g_ulColNum,		// Which column to match
										1,					// row to match
										DB_S_ENDOFROWSET,	// HRESULT to verify
										0,						// How many rows to expect.
										TRUE,					// flag to Release rows (optional)
										DBCOMPAREOPS_IGNORE,  // Any particular preference for comparing? (optional)
										SUBOP_EMPTY	// Some comparisions are rich enough to deserve a mulitple comparision operations
									);	
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc pBookMark=DBBMK_FIRST,cRows=1.  Verify DB_S_ENDOFROWSET
//
// @rdesc TEST_PASS or TEST_FAIL
//
int EmptyRowset::Variation_5()
{
	DBBOOKMARK	DBBookmark=DBBMK_FIRST;
	BYTE		*pBookmark=(BYTE *)&DBBookmark;
	
	//restart the cursor position
	if(!CHECK(RestartRowPosition(),S_OK))
		return TEST_FAIL;

	return CallFindNextRows(	g_pEmptyTable,	// CTable pointer
										pBookmark,		// bookmark;
										1,					// Length of bookmark
										1,					// # rows to fetch
										FALSE,			// skip current row
										g_ulColNum,		// Which column to match
										1,					// row to match
										DB_S_ENDOFROWSET,	// HRESULT to verify
										0,						// How many rows to expect.
										TRUE,					// flag to Release rows (optional)
										DBCOMPAREOPS_IGNORE,  // Any particular preference for comparing? (optional)
										SUBOP_EMPTY	// Some comparisions are rich enough to deserve a mulitple comparision operations
									);	
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc pBookmark=DBBMK_FIRST, cRows=0, DBCOMPAREOPS_IGNORE.  Verify DB_S_ENDOFROWSET
//
// @rdesc TEST_PASS or TEST_FAIL
//
int EmptyRowset::Variation_6()
{
	DBBOOKMARK	DBBookmark=DBBMK_FIRST;
	BYTE		*pBookmark=(BYTE *)&DBBookmark;
	
	//restart the cursor position
	if(!CHECK(RestartRowPosition(),S_OK))
		return TEST_FAIL;

	// FindNextRows with cRows=0 and pBookmark=non null value is a no-op
	// This conforms with IRowsetLocate's behavior with cRows=0.
	return CallFindNextRows(	g_pEmptyTable,	// CTable pointer
										pBookmark,		// bookmark;
										1,					// Length of bookmark
										0,					// # rows to fetch
										FALSE,			// skip current row
										g_ulColNum,		// Which column to match
										1,					// row to match
										S_OK,					// HRESULT to verify
										0,						// How many rows to expect.
										TRUE,					// flag to Release rows (optional)
										DBCOMPAREOPS_IGNORE,  // Any particular preference for comparing? (optional)
										SUBOP_EMPTY	// Some comparisions are rich enough to deserve a mulitple comparision operations
									);	
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc pBookmark=DBBMK_LAST,cRows=1.  Verify DB_S_ENDOFROWSET
//
// @rdesc TEST_PASS or TEST_FAIL
//
int EmptyRowset::Variation_7()
{
	DBBOOKMARK	DBBookmark=DBBMK_LAST;
	BYTE		*pBookmark=(BYTE *)&DBBookmark;
	
	//restart the cursor position
	if(!CHECK(RestartRowPosition(),S_OK))
		return TEST_FAIL;

	return CallFindNextRows(	g_pEmptyTable,	// CTable pointer
										pBookmark,		// bookmark;
										1,					// Length of bookmark
										1,					// # rows to fetch
										FALSE,			// skip current row
										g_ulColNum,		// Which column to match
										1,					// row to match
										DB_S_ENDOFROWSET,	// HRESULT to verify
										0,						// How many rows to expect.
										TRUE,					// flag to Release rows (optional)
										DBCOMPAREOPS_IGNORE,  // Any particular preference for comparing? (optional)
										SUBOP_EMPTY	// Some comparisions are rich enough to deserve a mulitple comparision operations
									);	
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc pBookmark=DBBMK_FIRST,cRows=-1.  Verify DB_S_ENDOFROWSET
//
// @rdesc TEST_PASS or TEST_FAIL
//
int EmptyRowset::Variation_8()
{
	DBBOOKMARK	DBBookmark=DBBMK_FIRST;
	BYTE		*pBookmark=(BYTE *)&DBBookmark;
	
	//restart the cursor position
	if(!CHECK(RestartRowPosition(),S_OK))
		return TEST_FAIL;

	return CallFindNextRows(	g_pEmptyTable,	// CTable pointer
										pBookmark,		// bookmark;
										1,					// Length of bookmark
										-1,					// # rows to fetch
										FALSE,			// skip current row
										g_ulColNum,		// Which column to match
										1,					// row to match
										DB_S_ENDOFROWSET,	// HRESULT to verify
										0,						// How many rows to expect.
										TRUE,					// flag to Release rows (optional)
										DBCOMPAREOPS_IGNORE,  // Any particular preference for comparing? (optional)
										SUBOP_EMPTY	// Some comparisions are rich enough to deserve a mulitple comparision operations
									);	
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL EmptyRowset::Terminate()
{
	ReleaseRowsetAndAccessor();

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIRowsetFind::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(NULL_Collation)
//*-----------------------------------------------------------------------
//| Test Case:		NULL_Collation - Test Null collatioans
//|	Created:			07/07/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL NULL_Collation::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCIRowsetFind::Init())
	// }}
	{
		if ( g_lNullCollation & DBPROPVAL_NC_END )
		{
			odtLog<<"NULLs are sorted at the end, regardless of the sort order.\n";
		}
		else if ( g_lNullCollation & DBPROPVAL_NC_HIGH )
		{
			odtLog<<"NULLs are sorted at the end\n";
		}
		else if ( g_lNullCollation & DBPROPVAL_NC_LOW )
		{
			odtLog<<"NULLs are sorted at the low end\n";
		}
		else if ( g_lNullCollation & DBPROPVAL_NC_START )
		{
			odtLog<<"NULLs are sorted at the start of the list, regardless of the sort order\n";
		}
		else
		{
			odtLog<<"No Null Collation reported!\n";
		}

		return TRUE;
	}

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc NE operator with NULL status
//
// @rdesc TEST_PASS or TEST_FAIL
//
int NULL_Collation::Variation_1()
{	
	BOOL				fTestPass = TEST_FAIL;
	DBORDINAL			ulColIndex = 0;
	DBCOUNTITEM			cRowsObtained = 0;
	HROW				hrowNULL = DB_NULL_HROW;
	HROW *				phrowNULL = &hrowNULL;
	HROW				hrowNOTNULL = DB_NULL_HROW;
	HROW *				phrowNOTNULL = &hrowNOTNULL;
	CCol				TempCol;
	CTable *			pTable = NULL;
	IRowsetIdentity *	pIRowsetIden = NULL;
	DBPROPID			rgPropId[3] = {DBPROP_CANSCROLLBACKWARDS, DBPROP_CANHOLDROWS, DBPROP_IRowsetIdentity};

	TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetFind,
		NUMELEM(rgPropId), rgPropId));
	TESTC(VerifyInterface(m_pIRowset, IID_IRowsetIdentity, ROWSET_INTERFACE, 
						(IUnknown **)&pIRowsetIden));

	for (ulColIndex=1; ulColIndex <= g_pCTable->CountColumnsOnTable(); ulColIndex++)
	{
		TESTC(SUCCEEDED(m_pIRowset->RestartPosition(NULL)));
		g_pCTable->GetColInfo(ulColIndex, TempCol);

		if(!CreateFindValueAccessor(DBCOMPAREOPS_EQ, g_pCTable, 0, ulColIndex, 
						TempCol.GetProviderType(), SUBOP_ALWAYS_NULL))
			continue;

		TEST2C_(m_hr = m_pIRowsetFind->FindNextRow(DB_NULL_HCHAPTER, m_hRowsetFindAccessor, m_pFindValue, 
											DBCOMPAREOPS_EQ, 0, NULL, 0, 
											1, &cRowsObtained, &phrowNULL), S_OK, DB_S_ENDOFROWSET);

		ReleaseFindValueAccessor(TempCol.GetProviderType());

		if(m_hr == S_OK)
		{
			// Found a null.
			// The next fetch position is after the current cursor position.
			// Call FindNextRows with an lOffset of -1 to start finding from
			// the row just matched.
			//
			// This time use a NE operator and a bound value not matching any existing
			// value in the column.
			// Per spec, using the NE operator never matches a NULL value
			// i.e. Finding a value NE to 1 is equivalent to value != 1 and value is not NULL.
			TESTC(cRowsObtained == 1);
			TESTC(phrowNULL[0] != DB_NULL_HROW);

			TESTC(CreateFindValueAccessor(DBCOMPAREOPS_NE, g_pCTable, 0, ulColIndex, 
						TempCol.GetProviderType(), SUBOP_EMPTY));

			TEST2C_(m_hr = m_pIRowsetFind->FindNextRow(DB_NULL_HCHAPTER, m_hRowsetFindAccessor, m_pFindValue, 
											DBCOMPAREOPS_NE, 0, NULL, -1, 
											1, &cRowsObtained, &phrowNOTNULL), S_OK, DB_S_ENDOFROWSET);
			// If end of rowset, that's fine otherwise check that the null value was not matched
			if(m_hr == S_OK)
			{
				TESTC(cRowsObtained == 1);
				TESTC(phrowNOTNULL[0] != DB_NULL_HROW);
				HRESULT hr = pIRowsetIden->IsSameRow(phrowNULL[0], phrowNOTNULL[0]);

				if (hr != S_FALSE)
				{
					odtLog << L"Failed in column " << ulColIndex << " of type " << TempCol.GetProviderType() <<  L"\n";
				}
				CHECK(hr, S_FALSE);			
				
				TESTC_(m_pIRowset->ReleaseRows(1, phrowNOTNULL, NULL, NULL, NULL), S_OK);
			}

			ReleaseFindValueAccessor(TempCol.GetProviderType());
			
			TESTC_(m_pIRowset->ReleaseRows(1, phrowNULL, NULL, NULL, NULL), S_OK);
		}
	}

	fTestPass = TEST_PASS;
	
CLEANUP:

	ReleaseFindValueAccessor(TempCol.GetProviderType());

	SAFE_RELEASE(pIRowsetIden);
	ReleaseRowsetAndAccessor();

	return fTestPass;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL NULL_Collation::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIRowsetFind::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(Prop_FINDCOMPAREOPS)
//*-----------------------------------------------------------------------
//| Test Case:		Prop_FINDCOMPAREOPS - Check DBPROP_FINDCOMAPREOPS
//|	Created:			08/18/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Prop_FINDCOMPAREOPS::Init()
{
	DBPROPID	guidPropertySet;
	ULONG	cPrptSet=0;
	BOOL fTestPass = FALSE;

	if(!TCIRowsetFind::Init())
		return FALSE;

	//create a rowset and an accessor.  
	TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetFind,
		cPrptSet,&guidPropertySet));

CLEANUP:
	return TRUE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Check CO_EQUALITY and CO_STRING
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Prop_FINDCOMPAREOPS::Variation_1()
{
	DWORD FindOps = 0;
	CCol TempCol;
	DBORDINAL ulColNum = g_pCTable->CountColumnsOnTable();
	DBID *pColDBID = NULL;

	for ( ULONG ulColIndex = 1; ulColIndex <= ulColNum; ulColIndex++ )
	{
		g_pCTable->GetColInfo(ulColIndex, TempCol);
		pColDBID = TempCol.GetColID();

		FindOps = TC_FindCompareOps( pColDBID );

		if ( pColDBID->eKind == DBKIND_NAME )
			odtLog << L" Column" << pColDBID->uName.pwszName << L": \n";

		// DBPROPVAL_CO_EQUALITY and DBPROPVAL_CO_STRING are required for IRowsetFind
		if ( FindOps & DBPROPVAL_CO_EQUALITY )
			odtLog << L"\t" << L"Supports DBPROPVAL_CO_EQUALITY\n";

		if ( FindOps & DBPROPVAL_CO_STRING )
			odtLog << L"\t" << L"Supports DBPROPVAL_CO_STRING\n";

		if ( FindOps & DBPROPVAL_CO_CASESENSITIVE )
			odtLog << L"\t" << L"Supports DBPROPVAL_CO_CASESENSITIVE\n";

		if ( FindOps & DBPROPVAL_CO_CASEINSENSITIVE )
			odtLog << L"\t" << L"Supports DBPROPVAL_CO_CASEINSENSITIVE\n";

		if ( FindOps & DBPROPVAL_CO_BEGINSWITH )
			odtLog << L"\t" << L"Supports DBPROPVAL_CO_BEGINSWITH\n";
		
		if ( FindOps & DBPROPVAL_CO_CONTAINS )
			odtLog << L"\t" << L"Supports DBPROPVAL_CO_CONTAINS\n";
	}

	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Check Case Sensitivity
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Prop_FINDCOMPAREOPS::Variation_2()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Prop_FINDCOMPAREOPS::Terminate()
{
	ReleaseRowsetAndAccessor();

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIRowsetFind::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(QueryInt)
//*-----------------------------------------------------------------------
//| Test Case:		QueryInt - test queryinterfaces
//|	Created:			08/18/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL QueryInt::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCIRowsetFind::Init())
	// }}
	{
		// TO DO:  Add your own code here
		return TRUE;
	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc IRowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int QueryInt::Variation_1()
{
	BOOL		fTestPass = FALSE;
	IRowset	*	pIRowset = NULL;
	DBCOUNTITEM cRowsObtained = 0;
	HROW		hrow[1] = { DB_NULL_HROW };
	HROW *		phrow = hrow;

	if(!TCIRowsetFind::Init())
		return FALSE;

	//create a rowset and an accessor.  
	TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetFind,
		0, NULL));

	TESTC(DefaultObjectTesting(m_pIRowsetFind, ROWSET_INTERFACE));

	TESTC(VerifyInterface(m_pIRowsetFind, IID_IRowset, ROWSET_INTERFACE, (IUnknown **)&pIRowset));

	TESTC_(pIRowset->GetNextRows(DB_NULL_HCHAPTER, 0, 1, &cRowsObtained, (HROW **)&phrow ), S_OK);

	TESTC(cRowsObtained == 1);
	
	//call VerifyRowPosition
	TESTC(VerifyRowPosition(phrow[0],1,g_pCTable));

	TESTC_(pIRowset->ReleaseRows(cRowsObtained, phrow, NULL, NULL, NULL),S_OK);

	fTestPass = TRUE;

CLEANUP:

	SAFE_RELEASE(pIRowset);
	ReleaseRowsetAndAccessor();
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc IAccessor
//
// @rdesc TEST_PASS or TEST_FAIL
//
int QueryInt::Variation_2()
{
	HRESULT			hr;
	BOOL			fTestPass = TRUE;
	IRowsetFind *	pIRowsetFind = NULL;
	IRowset *		pIRowset = NULL;
	DBCOUNTITEM		cRowsObtained;
	HROW			hrow[1] = { DB_NULL_HROW };
	HROW *			phrow = hrow;

	if( !SetRowsetProperties(NULL, 0) )
		goto CLEANUP;

	hr = CreateRowsetObject(USE_OPENROWSET, IID_IRowsetFind, EXECUTE_IFNOERROR);
	
	if( hr==DB_S_ERRORSOCCURRED || hr==DB_E_ERRORSOCCURRED )
		goto CLEANUP;
	
	TESTC_(hr,S_OK);

	TESTC(DefaultObjectTesting(m_pIAccessor, ROWSET_INTERFACE));

	//queryinterface for IRowsetFind
	TESTC(VerifyInterface(m_pIAccessor, IID_IRowsetFind, ROWSET_INTERFACE, (IUnknown **)&pIRowsetFind));

	//queryinterface for IRowset.  IRowsetFind implies IRowset
	TESTC(VerifyInterface(pIRowsetFind, IID_IRowset, ROWSET_INTERFACE, (IUnknown **)&pIRowset));
	
	//create an accessor on the rowset
	TESTC_(GetAccessorAndBindings(pIRowset,DBACCESSOR_ROWDATA,&m_hAccessor,
		&m_rgBinding,&m_cBinding,&m_cRowSize,DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,ALL_COLS_BOUND,FORWARD,
		NO_COLS_BY_REF,NULL,NULL,NULL,DBTYPE_EMPTY,0,NULL,NULL,
		NO_COLS_OWNED_BY_PROV,DBPARAMIO_NOTPARAM,TRUE),S_OK);

	//allocate memory for the row
	m_pData = PROVIDER_ALLOC(m_cRowSize);

	TESTC(m_pData != NULL);
	TESTC_(pIRowset->GetNextRows(DB_NULL_HCHAPTER, 0, 1, &cRowsObtained, (HROW **)&phrow ), S_OK);
	TESTC(cRowsObtained == 1);

	//call VerifyRowPosition
	TESTC(VerifyRowPosition(phrow[0],1,g_pCTable));

	TESTC_(pIRowset->ReleaseRows(cRowsObtained, phrow, NULL, NULL, NULL), S_OK);

	fTestPass = TRUE;

CLEANUP:

	//free the consumer buffer
	PROVIDER_FREE(m_pData);
	PROVIDER_FREE(m_rgBinding);
	PROVIDER_FREE(m_rgTableColOrds);

	//free accessor handle
	if(m_hAccessor)
	{
		if(!CHECK(m_pIAccessor->ReleaseAccessor(m_hAccessor,NULL), S_OK))
				fTestPass=FALSE;

		m_hAccessor=NULL;
	}

	SAFE_RELEASE(m_pIAccessor);
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pIRowsetFind);

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//--------------------------------------------------------------------
// @mfunc Use ICmdText
//
// @rdesc TEST_PASS or TEST_FAIL
//
int QueryInt::Variation_3()
{
	ICommandText*	pICmdText = NULL;
	IRowsetFind*	pIRowsetFind = NULL;
	WCHAR*			pwszCmd = NULL;
	
	if (g_pIDBCreateCommand == NULL)
		return TEST_SKIPPED;

	TESTC_(m_hr = g_pIDBCreateCommand->CreateCommand(NULL, IID_ICommandText, (IUnknown **)&pICmdText), S_OK);

 	// Get a supported command text
	TESTC_(m_hr = g_pCTable->CreateSQLStmt(SELECT_ALLFROMTBL, NULL, &pwszCmd, NULL, NULL), S_OK);
	TESTC_(m_hr = pICmdText->SetCommandText(DBGUID_DEFAULT, pwszCmd), S_OK);
	
	// Execute command
	TESTC_(m_hr = pICmdText->Execute(NULL, IID_IRowsetFind, NULL, NULL, (IUnknown**) &pIRowsetFind), S_OK);

CLEANUP:

	SAFE_FREE(pwszCmd);
	SAFE_RELEASE(pICmdText);
	SAFE_RELEASE(pIRowsetFind);

	return TEST_PASS;
}
// }}
// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL QueryInt::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIRowsetFind::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCCompareOps_Ignore)
//*-----------------------------------------------------------------------
//| Test Case:		TCCompareOps_Ignore - Test specific IGNORE cases
//|	Created:			10/02/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCCompareOps_Ignore::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCIRowsetFind::Init())
	// }}
	{
		// TO DO:  Add your own code here
		return TRUE;
	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc IGNORE w/pBookmark=NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCompareOps_Ignore::Variation_1()
{
	BOOL		fTestPass;
	HROW *		phrow = NULL;
	DBCOUNTITEM cRowsObtained;

	//open rowset, and accessor.  
	if(!GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetFind,
		0,NULL))
	{
		fTestPass = TEST_FAIL;
		goto CLEANUP;
	}

	HRESULT hrExpected2 = S_OK;
	int     nRowPos     = 8;

	// match 3rd row and fetch 2 rows
	// new fetch position should be after 4th
	fTestPass = CallFindNextRows(		
									g_pCTable,		// Table to find from
									NULL,				// Bookmark to fetch from, if any
									0,					// Length of bookmark
									2,					// maps to cRows
									2,					// maps to Offset
									g_ulColNum,		// Column to match
									3,					// Is there a row where the find should happen? 0 - no match
									S_OK,				// Expected HRESULT
									2,					// Expected count of rows
									TRUE,				// flag to Release rows (optional)
									DBCOMPAREOPS_IGNORE,	   // Any particular preference for comparing? (optional)
									SUBOP_EMPTY,  		// Some comparisions are rich enough to deserve a mulitple comparision operations
									NULL,				// Use client or provider memory, default=provider
									TRUE,				// verify rows by comparing data ?
									FALSE,				// Use ISeqStream ?
        							hrExpected2		    // Second expected HRESULT for the FindNextRow step
									);

	// match 7th row, fetch position should advance 2 rows and be after 7th
	fTestPass = CallFindNextRows(		
									g_pCTable,		// Table to find from
									NULL,				// Bookmark to fetch from, if any
									0,					// Length of bookmark
									0,					// maps to cRows
									2,					// maps to Offset
									g_ulColNum,		// Column to match
									7,					// Is there a row where the find should happen? 0 - no match
									S_OK,				// Expected HRESULT
									0,					// Expected count of rows
									TRUE,				// flag to Release rows (optional)
									DBCOMPAREOPS_IGNORE,	   // Any particular preference for comparing? (optional)
									SUBOP_EMPTY 		// Some comparisons are rich enough to deserve a mulitple comparision operations									
								);
	
	// cursor should be after 7th row.
	if (!CHECK(m_pIRowset->GetNextRows(DB_NULL_HCHAPTER, 0, 1, &cRowsObtained, (HROW **)&phrow ), S_OK) )
		goto CLEANUP;

	if (!COMPARE(cRowsObtained, 1))
		goto CLEANUP;

	if (!COMPARE(VerifyRowPosition(phrow[0], nRowPos, g_pCTable), TRUE))
		goto CLEANUP;


CLEANUP:
	PROVIDER_FREE(phrow);
	ReleaseRowsetAndAccessor();

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc IGNORE w/ pBookmark = STD_BOOKMARK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCompareOps_Ignore::Variation_2()
{
	BOOL		fTestPass = TEST_SKIPPED;
	DBPROPID	guidProperty[1];
	HROW *		phrow = NULL;
	DBCOUNTITEM cRowsObtained;
	DBBOOKMARK	DBBookmark=DBBMK_LAST;
	BYTE *		pBookmark=(BYTE *)&DBBookmark;

	TESTC_DRIVER(g_rgDBPrpt[IDX_IRowsetLocate].fSupported);

	guidProperty[0] = DBPROP_IRowsetLocate;

	//open rowset, and accessor.  
	if(!GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetFind,
		1,guidProperty))
	{
		fTestPass = TEST_FAIL;
		goto CLEANUP;
	}
	HRESULT hrExpected2 = S_OK;

	fTestPass = CallFindNextRows(		
									g_pCTable,		// Table to find from
									pBookmark,		// Bookmark to fetch from, if any
									1,					// Length of bookmark
									1,					// maps to cRows
									0,					// maps to Offset
									g_ulColNum,		// Column to match
									g_lRowLast,		// Is there a row where the find should happen? 0 - no match
									hrExpected2,	// Expected HRESULT
									1,					// Expected count of rows
									TRUE,				// flag to Release rows (optional)
									DBCOMPAREOPS_IGNORE,	   // Any particular preference for comparing? (optional)
									SUBOP_EMPTY					// Some comparisions are rich enough to deserve a mulitple comparision operations
									);

	// make bookmark point to DBBMK_FIRST
	DBBookmark=DBBMK_FIRST;
	fTestPass = CallFindNextRows(		
									g_pCTable,		// Table to find from
									pBookmark,		// Bookmark to fetch from, if any
									1,					// Length of bookmark
									1,					// maps to cRows
									0,					// maps to Offset
									g_ulColNum,		// Column to match
									1,					// Is there a row where the find should happen? 0 - no match
									S_OK,				// Expected HRESULT
									1,					// Expected count of rows
									TRUE,				// flag to Release rows (optional)
									DBCOMPAREOPS_IGNORE,	   // Any particular preference for comparing? (optional)
									SUBOP_EMPTY					// Some comparisions are rich enough to deserve a mulitple comparision operations
									);
	
	// cursor should be at start of rowset.
	if (!CHECK(m_pIRowset->GetNextRows(DB_NULL_HCHAPTER, 0, 1, &cRowsObtained, (HROW **)&phrow ), S_OK) )
		goto CLEANUP;

	if (!COMPARE(cRowsObtained, 1))
		goto CLEANUP;

	if (!COMPARE(VerifyRowPosition(phrow[0], 1, g_pCTable), TRUE))
		goto CLEANUP;

CLEANUP:
	PROVIDER_FREE(phrow);
	ReleaseRowsetAndAccessor();

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc IGNORE w/real bookmark
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCompareOps_Ignore::Variation_3()
{
	BOOL		fTestPass = TEST_SKIPPED;
	DBPROPID	guidProperty[1];
	HROW *		phrow = NULL;
	DBCOUNTITEM	cRowsObtained;
	ULONG_PTR	cbBookmark;
	BYTE *		pBookmark=NULL;

	TESTC_DRIVER(g_rgDBPrpt[IDX_IRowsetLocate].fSupported);

	guidProperty[0] = DBPROP_IRowsetLocate;

	//open rowset, and accessor.  
	if(!GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetFind,
		1,guidProperty))
	{
		fTestPass = TEST_FAIL;
		goto CLEANUP;
	}

	//get the bookmark for the 2th row
	if(!GetBookmark(2,&cbBookmark, &pBookmark))
	{
		fTestPass = TEST_FAIL;
		goto CLEANUP;
	}
	HRESULT hrExpected2 = S_OK;

	fTestPass = CallFindNextRows(		
									g_pCTable,		// Table to find from
									pBookmark,		// Bookmark to fetch from, if any
									cbBookmark,		// Length of bookmark
									2,					// maps to cRows
									2,					// maps to Offset
									g_ulColNum,		// Column to match
									4,					// Is there a row where the find should happen? 0 - no match
									S_OK,				// Expected HRESULT
									2,					// Expected count of rows
									TRUE,				// flag to Release rows (optional)
									DBCOMPAREOPS_IGNORE,	   // Any particular preference for comparing? (optional)
									SUBOP_EMPTY,		// Some comparisions are rich enough to deserve a mulitple comparision operations
									NULL,				// Use client or provider memory, default=provider
									TRUE,				// verify rows by comparing data ?
									FALSE,				// Use ISeqStream ?
        							hrExpected2		    // Second expected HRESULT for the FindNextRow step
									);
	HRESULT hrExpected1 = DB_S_ENDOFROWSET;

	fTestPass = CallFindNextRows(		
									g_pCTable,		// Table to find from
									pBookmark,		// Bookmark to fetch from, if any
									cbBookmark,		// Length of bookmark
									1,					// maps to cRows
									g_lRowLast-1,	// maps to Offset
									g_ulColNum,		// Column to match
									g_lRowLast,		// Is there a row where the find should happen? 0 - no match
									hrExpected1,	    // Expected HRESULT
									0,					// Expected count of rows
									TRUE,				// flag to Release rows (optional)
									DBCOMPAREOPS_IGNORE,	   // Any particular preference for comparing? (optional)
									SUBOP_EMPTY					// Some comparisions are rich enough to deserve a mulitple comparision operations
									);
	
	// cursor should be at start of rowset.
	if (!CHECK(m_pIRowset->GetNextRows(DB_NULL_HCHAPTER, 0, 1, &cRowsObtained, (HROW **)&phrow ), S_OK) )
		goto CLEANUP;

	if (!COMPARE(cRowsObtained, 1))
		goto CLEANUP;

	if (!COMPARE(VerifyRowPosition(phrow[0], 1, g_pCTable), TRUE))
		goto CLEANUP;

CLEANUP:
	PROVIDER_FREE(pBookmark);
	PROVIDER_FREE(phrow);
	ReleaseRowsetAndAccessor();

	return fTestPass;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCCompareOps_Ignore::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIRowsetFind::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(AccessorTests)
//*-----------------------------------------------------------------------
//| Test Case:		AccessorTests - Test various accessor variations
//|	Created:			02/01/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL AccessorTests::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCIRowsetFind::Init())
	// }}
	{
		// TO DO:  Add your own code here
		return TRUE;
	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc obValue = 0, bind VALUE only
//
// @rdesc TEST_PASS or TEST_FAIL
//
int AccessorTests::Variation_1()
{
	BOOL			fTestPass = TEST_SKIPPED;
	DBPROPID		guidProperty[1];
	HROW *			phrow = NULL;
	DBCOUNTITEM		cRowsObtained;
	DBORDINAL		ulColToFind;
	HRESULT			hr;
	void			*pMakeData = NULL;
	CCol			TempCol;
	DBTYPE			wColType;
	WCHAR			wszData[2000];

	TESTC_DRIVER(g_rgDBPrpt[IDX_IRowsetLocate].fSupported);

	guidProperty[0] = DBPROP_IRowsetLocate;

	//open rowset, and accessor.  
	if(!GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetFind,
		1,guidProperty,DBACCESSOR_ROWDATA, DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
		ALL_COLS_BOUND, FORWARD, NO_COLS_BY_REF, NULL, EXECUTE_IFNOERROR, DBTYPE_EMPTY, 
		TRUE))
		goto CLEANUP;

	if (!GetVariableLengthStrAndUpdatable(&ulColToFind, DBCOMPAREOPS_EQ))
		goto CLEANUP;

	fTestPass = TEST_FAIL;

	if ( FAILED(m_pTable->GetColInfo(ulColToFind, TempCol)) )
		goto CLEANUP;

	wColType = TempCol.GetProviderType();

	if (wColType==DBTYPE_VARIANT)
	{
		fTestPass = TEST_SKIPPED;
		goto CLEANUP;
	}

	if (!SUCCEEDED(hr = m_pTable->MakeData(	wszData, 
											1,
											ulColToFind, 
											PRIMARY, 
											wColType)) )
		goto CLEANUP;

	if ( hr == S_FALSE )
	{
		fTestPass = TEST_SKIPPED;
		goto CLEANUP; // can't deal with nulls in this variation
	}

	TESTC(fTestPass=CreateFindValueAccessor(DBCOMPAREOPS_EQ, g_pCTable, 1, ulColToFind, wColType, SUBOP_EMPTY, FALSE, NULL, DBPART_VALUE)); 

	fTestPass = TEST_FAIL;
	if ( FAILED( m_hr = m_pIRowsetFind->FindNextRow(DB_NULL_HCHAPTER, m_hRowsetFindAccessor, m_pFindValue, 
											DBCOMPAREOPS_EQ, 0, NULL, 0, 
											1, &cRowsObtained, &phrow)) )
		goto CLEANUP;

	//Verify RowsObtained
	if (!COMPARE(cRowsObtained, 1))
		goto CLEANUP;

	//verify phRow is not NULL
	TESTC(phrow!=NULL);

	if (!COMPARE(VerifyRowPosition(phrow[0], 1, g_pCTable), TRUE))
		goto CLEANUP;

	fTestPass = TEST_PASS;

CLEANUP:
	//PROVIDER_FREE(pMakeData);
		//release the row handle
	if(phrow && cRowsObtained>0)
	{
		CHECK(m_pIRowset->ReleaseRows(cRowsObtained,phrow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(phrow);
	}
	ReleaseFindValueAccessor(wColType);
	ReleaseRowsetAndAccessor();
	return fTestPass;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL AccessorTests::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIRowsetFind::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(BindingType)
//*-----------------------------------------------------------------------
//| Test Case:		BindingType - Test interesting Find coercions
//|	Created:			02/03/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL BindingType::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCIRowsetFind::Init())
	// }}
	{
		// TO DO:  Add your own code here
		CoCreateInstance(CLSID_OLEDB_CONVERSIONLIBRARY,
						  NULL,
						  CLSCTX_INPROC_SERVER,
						  IID_IDataConvert,
						  (void **)&g_pIDataConvert);

		if(!SetDCLibraryVersion((IUnknown *)g_pIDataConvert, g_ulDCVer))
		{
			odtLog << L"Unable to set Data Conversion Library's behavior version!" << ENDL;
			odtLog << L"Need to upgrade to latest MSDADC.DLL." << ENDL;
			return TEST_FAIL;
		}

		return TRUE;
	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Normal BSTR
//
// @rdesc TEST_PASS or TEST_FAIL
//
int BindingType::Variation_1()
{
	return BindingTypeTest(g_pCTable, DBTYPE_BSTR);
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc WSTR
//
// @rdesc TEST_PASS or TEST_FAIL
//
int BindingType::Variation_2()
{
	return BindingTypeTest(g_pCTable, DBTYPE_WSTR);
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc STR
//
// @rdesc TEST_PASS or TEST_FAIL
//
int BindingType::Variation_3()
{
	return BindingTypeTest(g_pCTable, DBTYPE_STR);
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc VARIANT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int BindingType::Variation_4()
{
	return BindingTypeTest(g_pCTable, DBTYPE_VARIANT);
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc WSTR | BYREF
//
// @rdesc TEST_PASS or TEST_FAIL
//
int BindingType::Variation_5()
{
	return BindingTypeTest(g_pCTable, DBTYPE_WSTR | DBTYPE_BYREF);
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc STR | BYREF
//
// @rdesc TEST_PASS or TEST_FAIL
//
int BindingType::Variation_6()
{
	return BindingTypeTest(g_pCTable, DBTYPE_STR | DBTYPE_BYREF);
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc BSTR | BYREF
//
// @rdesc TEST_PASS or TEST_FAIL
//
int BindingType::Variation_7()
{
	return BindingTypeTest(g_pCTable, DBTYPE_BSTR | DBTYPE_BYREF);
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc VARIANT BYREF
//
// @rdesc TEST_PASS or TEST_FAIL
//
int BindingType::Variation_8()
{
	return BindingTypeTest(g_pCTable, DBTYPE_VARIANT | DBTYPE_BYREF);
}

// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL BindingType::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIRowsetFind::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(Deleted_Rows)
//*-----------------------------------------------------------------------
//| Test Case:		Deleted_Rows - Test with deleted rows
//|	Created:			06/18/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Deleted_Rows::Init()
{
	BOOL fTestPass = FALSE;

	if(!TCIRowsetFind::Init())
		return FALSE;

	//make sure IID_IRowsetDelete is supported
	TESTC_DRIVER(g_rgDBPrpt[IDX_IRowsetChange].fSupported);

	//make sure IID_IRowsetLocate is supported
	TESTC_DRIVER(g_rgDBPrpt[IDX_IRowsetLocate].fSupported);

	if ( AlteringRowsIsOK() )
		fTestPass = TRUE;

CLEANUP:
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc *pBookMark=last row. Delete last row. Expect DB_E_BADBOOKMARK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Deleted_Rows::Variation_1()
{
	BOOL		fTestPass = TEST_FAIL;
	DBPROPID	guidProperty[3];
	BYTE *		pBookmark = NULL;
	ULONG_PTR	cbBookmark;

	guidProperty[0]=DBPROP_IRowsetChange;
	guidProperty[1]=DBPROP_IRowsetLocate;
	guidProperty[2]=DBPROP_UPDATABILITY;

	//open rowset, and accessor (bind LONG cols)  
	if(!GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetFind,
		NUMELEM(guidProperty),guidProperty,DBACCESSOR_ROWDATA, DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
		ALL_COLS_BOUND, FORWARD, NO_COLS_BY_REF, NULL, EXECUTE_IFNOERROR, DBTYPE_EMPTY, 
		TRUE))
	{
		fTestPass = TEST_FAIL;
		goto CLEANUP;
	}
	
	if (GetProp(DBPROP_BOOKMARKSKIPPED))
	{
		fTestPass = TEST_SKIPPED;
		goto CLEANUP;
	}

	//get the bookmark for the last row
	if(!GetBookmark(g_lRowLast, &cbBookmark, &pBookmark))
	{
		fTestPass = TEST_FAIL;
		goto CLEANUP;
	}

	if (!COMPARE(DeleteRow(g_pCTable, g_lRowLast), TRUE))
		goto CLEANUP;

	fTestPass =  CallFindNextRows(	g_pCTable,		// CTable pointer
												pBookmark,		// bookmark;
												cbBookmark,		// Length of bookmark
												1,					// # rows to fetch
												0,					// Offset
												g_ulColNum,		// Which column to match
												g_lRowLast,		// row to match
												DB_E_BADBOOKMARK,	// HRESULT to verify
												0					// How many rows to expect.
											);

	PopulateTable();

CLEANUP:
	PROVIDER_FREE(pBookmark);
	ReleaseRowsetAndAccessor();
	
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc *pBookMark = 2, cRows=1. Delete 4th row. Match on what was in 4th row. DB_S_ENDOFROWSET and 0 row.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Deleted_Rows::Variation_2()
{
	int			fTestPass=TEST_FAIL;
	DBPROPID	guidProperty[3];
	BYTE *		pBookmark = NULL;
	ULONG_PTR	cbBookmark;

	guidProperty[0]=DBPROP_IRowsetChange;
	guidProperty[1]=DBPROP_IRowsetLocate;
	guidProperty[2]=DBPROP_UPDATABILITY;

	//open rowset, and accessor.  (bind long cols)
	if(!GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetFind,
		NUMELEM(guidProperty),guidProperty,DBACCESSOR_ROWDATA, DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
		ALL_COLS_BOUND, FORWARD, NO_COLS_BY_REF, NULL, EXECUTE_IFNOERROR, DBTYPE_EMPTY, 
		TRUE))
	{
		fTestPass = TEST_FAIL;
		goto CLEANUP;
	}

	if (GetProp(DBPROP_REMOVEDELETED))
	{
		fTestPass = TEST_SKIPPED;
		goto CLEANUP;
	}

	//get the bookmark 
	if(!GetBookmark(2, &cbBookmark, &pBookmark))
	{
		fTestPass = TEST_FAIL;
		goto CLEANUP;
	}

	if (!COMPARE(DeleteRow(g_pCTable, 4), TRUE))
		goto CLEANUP;

	fTestPass =  CallFindNextRows(	g_pCTable,	// CTable pointer
												pBookmark,  // bookmark;
												cbBookmark,	// Length of bookmark
												1,			   // # rows to fetch
												0,				// Offset
												g_ulColNum,	// Which column to match
												4,				// row to match
												DB_S_ENDOFROWSET,// HRESULT to verify
												0				 // How many rows to expect.
												);

	PopulateTable();
CLEANUP:
	PROVIDER_FREE(pBookmark);
	ReleaseRowsetAndAccessor();

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc *pBookMark=4, cRows=-1. Delete 1st row. Match on what was in 1st row. DB_S_ENDOFROWSET and 0 row.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Deleted_Rows::Variation_3()
{
	int			fTestPass=TEST_FAIL;
	DBPROPID	guidProperty[4];
	BYTE *		pBookmark = NULL;
	ULONG_PTR	cbBookmark;

	guidProperty[0]=DBPROP_IRowsetChange;
	guidProperty[1]=DBPROP_IRowsetLocate;
	guidProperty[2]=DBPROP_UPDATABILITY;
	guidProperty[3]=DBPROP_CANFETCHBACKWARDS;

	//open rowset, and accessor.  
	if(!GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetFind,
		NUMELEM(guidProperty),guidProperty,DBACCESSOR_ROWDATA, DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
		ALL_COLS_BOUND, FORWARD, NO_COLS_BY_REF, NULL, EXECUTE_IFNOERROR, DBTYPE_EMPTY, 
		TRUE))
	{
		fTestPass = TEST_FAIL;
		goto CLEANUP;
	}

	if (GetProp(DBPROP_REMOVEDELETED))
	{
		fTestPass = TEST_SKIPPED;
		goto CLEANUP;
	}

	//get the bookmark 
	if(!GetBookmark(4, &cbBookmark, &pBookmark))
	{
		fTestPass = TEST_FAIL;
		goto CLEANUP;
	}

	if (!COMPARE(DeleteRow(g_pCTable, 1), TRUE))
		goto CLEANUP;

	fTestPass =  CallFindNextRows(	g_pCTable,			// CTable pointer
												pBookmark,			// bookmark;
												cbBookmark,			// Length of bookmark
												-1,						// # rows to fetch
												0,						// Offset
												g_ulColNum,			// Which column to match
												1,						// row to match
												DB_S_ENDOFROWSET,	// HRESULT to verify
												0					// How many rows to expect.
												);

	PopulateTable();

CLEANUP:
	PROVIDER_FREE(pBookmark);
	ReleaseRowsetAndAccessor();

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc pBookMark=NULL, cRows=1. Match 3rd row. Delete third row. pBookmark=NULL, cRows=1, fSkip=0. Match 4th.  S_OK, 1 hrow
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Deleted_Rows::Variation_4()
{
	BOOL			fTestPass;
	DBPROPID		guidProperty[3];
	HROW *			phRows1 = NULL;
	IRowsetChange *	pIRowsetChange=NULL;

	guidProperty[0]=DBPROP_IRowsetChange;
	guidProperty[1]=DBPROP_IRowsetLocate;
	guidProperty[2]=DBPROP_UPDATABILITY;

	//open rowset, and accessor.  
	if(!GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetFind,
		NUMELEM(guidProperty),guidProperty))
	{
		fTestPass = TEST_FAIL;
		goto CLEANUP;
	}

	phRows1 = (HROW *) PROVIDER_ALLOC(sizeof(HROW));

	
	fTestPass = CallFindNextRows(		
								g_pCTable,		// Table to find from
								NULL,				// Bookmark to fetch from, if any
								0,					// Length of bookmark
								1,					// maps to cRows
								0,					// maps to Offset
								g_ulColNum,		// Column to match
								3,					// Is there a row where the find should happen? 0 - no match
								S_OK,				// Expected HRESULT
								1,					// Expected count of rows
								FALSE,			// flag to Release rows (optional)
								DBCOMPAREOPS_EQ,  // Any particular preference for comparing? (optional)
								SUBOP_EMPTY,	// Some comparisions are rich enough to deserve a mulitple comparision operations
								phRows1			// optional arg if client wants to control row handle mem
								);

	if (!fTestPass) goto CLEANUP;

	if (!COMPARE(VerifyRowPosition(phRows1[0], 3, g_pCTable), TRUE))
		goto CLEANUP;

		//QI for IRowsetChange pointer
	if(!CHECK(m_pIRowsetFind->QueryInterface(IID_IRowsetChange,
		(void **)&pIRowsetChange),S_OK))
		goto CLEANUP;

	//delete the row
	if(!CHECK(pIRowsetChange->DeleteRows(NULL,1,phRows1,NULL),S_OK))
		goto CLEANUP;
	if (m_cRowsFound > 0 && phRows1)
	{
		m_pIRowset->ReleaseRows(m_cRowsFound, phRows1, NULL, NULL, NULL);
		if (phRows1 != NULL)
			PROVIDER_FREE(phRows1);
	}


	fTestPass =  CallFindNextRows(	g_pCTable,	// CTable pointer
												NULL,			// bookmark;
												0,				// Length of bookmark
												1,				// # rows to fetch
												0,				// offset
												g_ulColNum,	// Which column to match
												4,				// row to match
												S_OK,			// HRESULT to verify
												1				// How many rows to expect.
												);

CLEANUP:
	PROVIDER_FREE(phRows1);
	SAFE_RELEASE(pIRowsetChange);
	ReleaseRowsetAndAccessor();
	PopulateTable();
	
	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc pBookmark=NULL, cRows=-1, match 3rd row. Delete 3rd. Match 2nd. S_OK, 1 hrow
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Deleted_Rows::Variation_5()
{
	BOOL			fTestPass;
	DBPROPID		guidProperty[4];
	HROW *			phRows1 = NULL;
	IRowsetChange *	pIRowsetChange=NULL;

	guidProperty[0]=DBPROP_IRowsetChange;
	guidProperty[1]=DBPROP_IRowsetLocate;
	guidProperty[2]=DBPROP_CANFETCHBACKWARDS;
	guidProperty[3]=DBPROP_UPDATABILITY;

	//open rowset, and accessor.  
	if(!GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetFind,
		NUMELEM(guidProperty),guidProperty))
	{
		fTestPass = TEST_FAIL;
		goto CLEANUP;
	}

	phRows1 = (HROW *) PROVIDER_ALLOC(sizeof(HROW));

	fTestPass = CallFindNextRows(		
								g_pCTable,		// Table to find from
								NULL,				// Bookmark to fetch from, if any
								0,					// Length of bookmark
								-1,				// maps to cRows
								0,					// maps to Offset
								g_ulColNum,		// Column to match
								3,					// Is there a row where the find should happen? 0 - no match
								S_OK,				// Expected HRESULT
								1,					// Expected count of rows
								FALSE,			// flag to Release rows (optional)
								DBCOMPAREOPS_EQ,  // Any particular preference for comparing? (optional)
								SUBOP_EMPTY,	// Some comparisions are rich enough to deserve a mulitple comparision operations
								phRows1			// optional arg if client wants to control row handle mem
								);

	if (!fTestPass) goto CLEANUP;

	if (!COMPARE(VerifyRowPosition(phRows1[0], 3, g_pCTable), TRUE))
		goto CLEANUP;

		//QI for IRowsetChange pointer
	if(!CHECK(m_pIRowsetFind->QueryInterface(IID_IRowsetChange,
		(void **)&pIRowsetChange),S_OK))
		goto CLEANUP;

	//delete the row
	if(!CHECK(pIRowsetChange->DeleteRows(NULL,1,phRows1,NULL),S_OK))
		goto CLEANUP;
	if (m_cRowsFound > 0 && phRows1)
	{
		m_pIRowset->ReleaseRows(m_cRowsFound, phRows1, NULL, NULL, NULL);
		if (phRows1 != NULL)
			PROVIDER_FREE(phRows1);
	}

	fTestPass =  CallFindNextRows(	g_pCTable,	// CTable pointer
												NULL,			// bookmark;
												0,				// Length of bookmark
												-1,			// # rows to fetch
												0,				// offset
												g_ulColNum,	// Which column to match
												2,				// row to match
												S_OK,			// HRESULT to verify
												1				// How many rows to expect.
												);

	PopulateTable();

CLEANUP:
	PROVIDER_FREE(phRows1);
	SAFE_RELEASE(pIRowsetChange);
	ReleaseRowsetAndAccessor();

	return fTestPass;
}



// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Delete RowLast-1 row. pBookmark=RowLast-2 row,cRows=3. Match RowLast-2 row. 
//        Verify S_OK, 3 hrows and DB_E_DELETEDROW when accessing deleted one

//
// @rdesc TEST_PASS or TEST_FAIL
//
int Deleted_Rows::Variation_6()
{
	BOOL		fTestPass = TEST_FAIL;
	DBPROPID	guidProperty[4];
	BYTE *		pBookmark = NULL;
	ULONG_PTR	cbBookmark;
	HROW *		phRows = NULL;

	phRows = (HROW *) PROVIDER_ALLOC(3 * sizeof(HROW));

	guidProperty[0]=DBPROP_IRowsetChange;
	guidProperty[1]=DBPROP_IRowsetLocate;	
	guidProperty[2]=DBPROP_CANSCROLLBACKWARDS;
	guidProperty[3]=DBPROP_UPDATABILITY;

	//open rowset, and accessor.  
	if(!GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetFind,
		NUMELEM(guidProperty),guidProperty))
	{
		fTestPass = TEST_FAIL;
		goto CLEANUP;
	}
    
	//get the bookmark 
	if(!GetBookmark(g_lRowLast-2, &cbBookmark, &pBookmark))
	{
		fTestPass = TEST_SKIPPED;
		goto CLEANUP;
	}

	if (GetProp(DBPROP_REMOVEDELETED))
	{
		fTestPass = TEST_SKIPPED;
		goto CLEANUP;
	}
	
	if (!COMPARE(DeleteRow(g_pCTable, g_lRowLast-1), TRUE))
		goto CLEANUP;

	fTestPass =  CallFindNextRows(	g_pCTable,		            // CTable pointer
									pBookmark,					// bookmark;
									cbBookmark,					// Length of bookmark
									3,							// # rows to fetch
									0,							// Offset
									g_ulColNum,					// Which column to match
									g_lRowLast-2,				// row to match
									S_OK,						// HRESULT to verify
									3,							// How many rows to expect.
									FALSE,						// Don't release rows
									DBCOMPAREOPS_EQ,
									SUBOP_EMPTY,
									phRows,
									FALSE						// Don't check row position
								);
	
	if (!COMPARE(fTestPass, TRUE))
		goto CLEANUP;

	if (!COMPARE(VerifyRowPosition(phRows[0], g_lRowLast-2, g_pCTable), TRUE))
		fTestPass = TEST_FAIL;
	
	if (!COMPARE(VerifyRowPosition(phRows[2], g_lRowLast, g_pCTable), TRUE))
		fTestPass = TEST_FAIL;

	if(!CHECK(m_pIRowset->GetData(phRows[1], m_hAccessor, m_pData), DB_E_DELETEDROW))
		fTestPass = TEST_FAIL;

CLEANUP:
	PROVIDER_FREE(phRows);
	PROVIDER_FREE(pBookmark);
	ReleaseRowsetAndAccessor();
	PopulateTable();

	return fTestPass;
}
// }}

// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Delete last row. pBookmark=DBBMK_LAST, cRows=1. Verify DB_E_BADBOOKMARK, 0 rows returned
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Deleted_Rows::Variation_7()
{	
	BOOL fTestPass = TEST_FAIL;
	DBPROPID		guidProperty[4];
	DBBOOKMARK dbBookMark = DBBMK_LAST;
	BYTE *pBookmark = (BYTE *)&dbBookMark;
	HROW *phRows = NULL;

	phRows = (HROW *) PROVIDER_ALLOC(sizeof(HROW));

	guidProperty[0]=DBPROP_IRowsetChange;
	guidProperty[1]=DBPROP_IRowsetLocate;	
	guidProperty[2]=DBPROP_CANSCROLLBACKWARDS;
	guidProperty[3]=DBPROP_UPDATABILITY;

	//open rowset, and accessor.  
	if(!GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetFind,
		NUMELEM(guidProperty),guidProperty))
	{
		fTestPass = TEST_FAIL;
		goto CLEANUP;
	}

	if (GetProp(DBPROP_REMOVEDELETED))
	{
		fTestPass = TEST_SKIPPED;
		goto CLEANUP;
	}

	if (!COMPARE(DeleteRow(g_pCTable, g_lRowLast), TRUE))
		goto CLEANUP;

	fTestPass =  CallFindNextRows(	g_pCTable,		  // CTable pointer
									pBookmark,		  // bookmark;
									1,				  // Length of bookmark
									1,				  // # rows to fetch
									0,				  // Offset
									g_ulColNum,		  // Which column to match
									g_lRowLast,	  // row to match
									GetProp(DBPROP_BOOKMARKSKIPPED)? S_OK : DB_E_BADBOOKMARK, // HRESULT to verify
									GetProp(DBPROP_BOOKMARKSKIPPED)? 1 : 0,				  // How many rows to expect.
									FALSE,			  // Don't release rows
									DBCOMPAREOPS_EQ,
									SUBOP_EMPTY,
									phRows,
									FALSE			  // Don't check row position
								);

	if (!COMPARE(fTestPass, TRUE))
		goto CLEANUP;

CLEANUP:
	PROVIDER_FREE(phRows);
	ReleaseRowsetAndAccessor();
	PopulateTable();

	return fTestPass;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Deleted_Rows::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIRowsetFind::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(RemoveDeleted)
//*-----------------------------------------------------------------------
//| Test Case:		RemoveDeleted - Test in context of RemoveDeleted
//|	Created:			06/18/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL RemoveDeleted::Init()
{
	BOOL fTestPass = TEST_SKIPPED;

	if(!TCIRowsetFind::Init())
		return FALSE;

	//make sure IID_IRowsetDelete is supported
	TESTC_DRIVER(g_rgDBPrpt[IDX_IRowsetChange].fSupported);

	//make sure IID_IRowsetLocate is supported
	TESTC_DRIVER(g_rgDBPrpt[IDX_IRowsetLocate].fSupported);

	//make sure ScrollBackwards property is supported
	TESTC_DRIVER(g_rgDBPrpt[IDX_ScrollBackwards].fSupported);

	//make sure IID_IRemoveDeleted is supported
	TESTC_DRIVER(g_rgDBPrpt[IDX_RemoveDeleted].fSupported);

	if ( AlteringRowsIsOK() )
		fTestPass = TRUE;

CLEANUP:
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Delete a row.  pBookmark=deleted row.  Verify DB_E_BADBOOKMARK.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int RemoveDeleted::Variation_1()
{
	int			fTestPass = TEST_FAIL;;
	DBPROPID	guidProperty[5];
	BYTE *		pBookmark = NULL;
	ULONG_PTR	cbBookmark;

	guidProperty[0]=DBPROP_IRowsetChange;
	guidProperty[1]=DBPROP_IRowsetLocate;
	guidProperty[2]=DBPROP_REMOVEDELETED;
	guidProperty[3]=DBPROP_CANSCROLLBACKWARDS;
	guidProperty[4]=DBPROP_UPDATABILITY;

	//open rowset, and accessor.  
	if(!GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetFind,
		NUMELEM(guidProperty),guidProperty))
	{
		fTestPass = TEST_FAIL;
		goto CLEANUP;
	}
	
	if (GetProp(DBPROP_BOOKMARKSKIPPED))
	{
		fTestPass = TEST_SKIPPED;
		goto CLEANUP;
	}

	//get the bookmark for the row 1
	if(!GetBookmark(1, &cbBookmark, &pBookmark))
	{
		fTestPass = TEST_SKIPPED;
		goto CLEANUP;
	}

	if (!COMPARE(DeleteRow(g_pCTable, 1), TRUE))
		goto CLEANUP;

	fTestPass =  CallFindNextRows(	g_pCTable,		// CTable pointer
												pBookmark,		// bookmark;
												cbBookmark,		// Length of bookmark
												1,					// # rows to fetch
												0,					// Offset
												g_ulColNum,		// Which column to match
												g_lRowLast,		// row to match
												DB_E_BADBOOKMARK,	// HRESULT to verify
												0					// How many rows to expect.
												);

	PopulateTable();
CLEANUP:
	PROVIDER_FREE(pBookmark);
	ReleaseRowsetAndAccessor();

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Delete 3rd row.  pBookmark=2nd row with cRows=3. Verify S_OK and 2,4,5 hrows
//
// @rdesc TEST_PASS or TEST_FAIL
//
int RemoveDeleted::Variation_2()
{
	BOOL		fTestPass = TEST_FAIL;
	DBPROPID	guidProperty[5];
	BYTE *		pBookmark = NULL;
	ULONG_PTR	cbBookmark;
	HROW *		phRows = NULL;

	phRows = (HROW *) PROVIDER_ALLOC( 3 * sizeof(HROW));

	guidProperty[0]=DBPROP_IRowsetChange;
	guidProperty[1]=DBPROP_IRowsetLocate;
	guidProperty[2]=DBPROP_REMOVEDELETED;
	guidProperty[3]=DBPROP_CANSCROLLBACKWARDS;
	guidProperty[4]=DBPROP_UPDATABILITY;


	//open rowset, and accessor.  
	if(!GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetFind,
		NUMELEM(guidProperty),guidProperty))
	{
		fTestPass = TEST_FAIL;
		goto CLEANUP;
	}

	//get the bookmark for the 2nd row
	if(!GetBookmark(2, &cbBookmark, &pBookmark))
	{
		fTestPass = TEST_SKIPPED;
		goto CLEANUP;
	}

	if (!COMPARE(DeleteRow(g_pCTable, 3), TRUE))
		goto CLEANUP;

	fTestPass =  CallFindNextRows(	g_pCTable,	// CTable pointer
									pBookmark,				// bookmark;
									cbBookmark,				// Length of bookmark
									3,							// # rows to fetch
									0,							// Offset
									g_ulColNum,				// Which column to match
									2,							// row to match
									S_OK,						// HRESULT to verify
									3,							// How many rows to expect.
									FALSE,					// Don't release rows
									DBCOMPAREOPS_EQ,
									SUBOP_EMPTY,
									phRows,
									FALSE						// Don't check row position
								);

	if (!COMPARE(fTestPass, TRUE))
		goto CLEANUP;

	if (!COMPARE(VerifyRowPosition(phRows[0], 2, g_pCTable), TRUE))
		fTestPass = TEST_FAIL;
	if (!COMPARE(VerifyRowPosition(phRows[1], 4, g_pCTable), TRUE))
		fTestPass = TEST_FAIL;
	if (!COMPARE(VerifyRowPosition(phRows[2], 5, g_pCTable), TRUE))
		fTestPass = TEST_FAIL;

CLEANUP:
	PROVIDER_FREE(phRows);
	PROVIDER_FREE(pBookmark);
	ReleaseRowsetAndAccessor();
	PopulateTable();

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Delete RowLast-1 row. pBookmark=RowLast-2 row,cRows=3. Match RowLast-2 row. 
//        Verify DB_S_ENDOFROWSET and RowLast-2, RowLast hrows
//
// @rdesc TEST_PASS or TEST_FAIL
//
int RemoveDeleted::Variation_3()
{
	BOOL		fTestPass = TEST_FAIL;
	DBPROPID	guidProperty[5];
	BYTE *		pBookmark = NULL;
	ULONG_PTR	cbBookmark;
	HROW *		phRows = NULL;

	phRows = (HROW *) PROVIDER_ALLOC( 2 * sizeof(HROW));

	guidProperty[0]=DBPROP_IRowsetChange;
	guidProperty[1]=DBPROP_IRowsetLocate;
	guidProperty[2]=DBPROP_REMOVEDELETED;
	guidProperty[3]=DBPROP_CANSCROLLBACKWARDS;
	guidProperty[4]=DBPROP_UPDATABILITY;

	//open rowset, and accessor.  
	if(!GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetFind,
		NUMELEM(guidProperty),guidProperty))
	{
		fTestPass = TEST_FAIL;
		goto CLEANUP;
	}

	//get the bookmark 
	if(!GetBookmark(g_lRowLast-2, &cbBookmark, &pBookmark))
	{
		fTestPass = TEST_SKIPPED;
		goto CLEANUP;
	}

	if (!COMPARE(DeleteRow(g_pCTable, g_lRowLast-1), TRUE))
		goto CLEANUP;

	fTestPass =  CallFindNextRows(	g_pCTable,		// CTable pointer
									pBookmark,					// bookmark;
									cbBookmark,					// Length of bookmark
									3,								// # rows to fetch
									0,								// Offset
									g_ulColNum,					// Which column to match
									g_lRowLast-2,				// row to match
									DB_S_ENDOFROWSET,			// HRESULT to verify
									2,								// How many rows to expect.
									FALSE,						// Don't release rows
									DBCOMPAREOPS_EQ,
									SUBOP_EMPTY,
									phRows,
									FALSE							// Don't check row position
								);
	
	if (!COMPARE(fTestPass, TRUE))
		goto CLEANUP;

	if (!COMPARE(VerifyRowPosition(phRows[0], g_lRowLast-2, g_pCTable), TRUE))
		fTestPass = TEST_FAIL;
	if (!COMPARE(VerifyRowPosition(phRows[1], g_lRowLast, g_pCTable), TRUE))
		fTestPass = TEST_FAIL;

CLEANUP:
	PROVIDER_FREE(phRows);
	PROVIDER_FREE(pBookmark);
	ReleaseRowsetAndAccessor();
	PopulateTable();

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Delete last row. pBookmark=DBBMK_LAST,cRows=-1. DBCOMPAREOPS_IGNORE.  Verify last-1 row returned.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int RemoveDeleted::Variation_4()
{
	BOOL fTestPass = TEST_FAIL;
	DBPROPID		guidProperty[5];
	DBBOOKMARK dbBookMark = DBBMK_LAST;
	BYTE *pBookmark = (BYTE *)&dbBookMark;

	guidProperty[0]=DBPROP_IRowsetChange;
	guidProperty[1]=DBPROP_IRowsetLocate;
	guidProperty[2]=DBPROP_REMOVEDELETED;
	guidProperty[3]=DBPROP_CANSCROLLBACKWARDS;
	guidProperty[4]=DBPROP_UPDATABILITY;

	//open rowset, and accessor.  
	if(!GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetFind,
		NUMELEM(guidProperty),guidProperty))
	{
		fTestPass = TEST_FAIL;
		goto CLEANUP;
	}

	if (!COMPARE(DeleteRow(g_pCTable, g_lRowLast), TRUE))
		goto CLEANUP;

	fTestPass =  CallFindNextRows(	g_pCTable,	// CTable pointer
									pBookmark,				// bookmark;
									1,							// Length of bookmark
									1,						   // # rows to fetch
									0,							// Offset
									g_ulColNum,				// Which column to match
									g_lRowLast-1,			// row to match
									S_OK,						// HRESULT to verify
									1,							// How many rows to expect.
									TRUE,				// release rows
									DBCOMPAREOPS_IGNORE,
									SUBOP_EMPTY,
									NULL,
									TRUE				// check row position
								);

	if (!COMPARE(fTestPass, TRUE))
		goto CLEANUP;


CLEANUP:
	ReleaseRowsetAndAccessor();
	PopulateTable();

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc pBookmark=NULL, cRows = 2. match 3rd.  Delete 2nd and 5th rows. Find with cRows=-1 and match 3rd
//
// @rdesc TEST_PASS or TEST_FAIL
//
int RemoveDeleted::Variation_5()
{
	BOOL fTestPass = TEST_FAIL;
	DBPROPID		guidProperty[6];
	HROW *phRows = NULL;
	HROW *phRowsToDelete = NULL;
	IRowsetChange *pIRowsetChange = NULL;
	DBCOUNTITEM cRowsObtained = 0;

	phRows = (HROW *) PROVIDER_ALLOC( sizeof(HROW));

	guidProperty[0]=DBPROP_IRowsetChange;
	guidProperty[1]=DBPROP_IRowsetLocate;
	guidProperty[2]=DBPROP_REMOVEDELETED;
	guidProperty[3]=DBPROP_CANFETCHBACKWARDS;
	guidProperty[4]=DBPROP_CANHOLDROWS;
	guidProperty[5]=DBPROP_UPDATABILITY;

	//open rowset, and accessor.  
	if(!GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetFind,
		NUMELEM(guidProperty),guidProperty))
	{
		fTestPass = TEST_FAIL;
		goto CLEANUP;
	}

	if (!CHECK(m_pIRowset->GetNextRows(DB_NULL_HCHAPTER, 1, 4, &cRowsObtained, (HROW **)&phRowsToDelete ), S_OK) )
		goto CLEANUP;
	
	if (!COMPARE(VerifyRowPosition(phRowsToDelete[0], 2, g_pCTable), TRUE))
		goto CLEANUP;

	if (!COMPARE(VerifyRowPosition(phRowsToDelete[3], 5, g_pCTable), TRUE))
		goto CLEANUP;

	if (!CHECK(RestartRowPosition(), S_OK))
		goto CLEANUP;

	fTestPass =  CallFindNextRows(	g_pCTable,	// CTable pointer
									NULL,						// bookmark;
									0,							// Length of bookmark
									2,						   // # rows to fetch
									0,							// Offset
									g_ulColNum,				// Which column to match
									3,							// row to match
									S_OK,						// HRESULT to verify
									2							// How many rows to expect.
										);

	if (!COMPARE(fTestPass, TRUE))
		goto CLEANUP;

	//QI for IRowsetChange pointer
	if(!CHECK(m_pIRowsetFind->QueryInterface(IID_IRowsetChange,
		(void **)&pIRowsetChange),S_OK))
	{
		fTestPass = TEST_FAIL;
		goto CLEANUP;
	}

	//delete the row
	if(!CHECK(pIRowsetChange->DeleteRows(NULL,1,&phRowsToDelete[0],NULL),S_OK))
	{
		fTestPass = TEST_FAIL;
		goto CLEANUP;
	}

	if(!CHECK(pIRowsetChange->DeleteRows(NULL,1,&phRowsToDelete[3],NULL),S_OK))
	{
		fTestPass = TEST_FAIL;
		goto CLEANUP;
	}

	fTestPass =  CallFindNextRows(	g_pCTable,	// CTable pointer
									NULL,						// bookmark;
									0,							// Length of bookmark
									-1,						   // # rows to fetch
									0,							// Offset
									g_ulColNum,				// Which column to match
									3,							// row to match
									S_OK,						// HRESULT to verify
									1							// How many rows to expect.
										);

	if (!COMPARE(fTestPass, TRUE))
		goto CLEANUP;


CLEANUP:
	PROVIDER_FREE(phRows);
	PROVIDER_FREE(phRowsToDelete);
	SAFE_RELEASE(pIRowsetChange);
	ReleaseRowsetAndAccessor();
	PopulateTable();

	return fTestPass;
}

// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc *pBookMark = NULL, cRows=1. Delete 4th row. Match on what was in 4th row. DB_S_ENDOFROWSET and 0 rows.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int RemoveDeleted::Variation_6()
{
	int			fTestPass=TEST_FAIL;
	DBPROPID	guidProperty[4];
	BYTE *		pBookmark = NULL;	

	guidProperty[0]=DBPROP_IRowsetChange;
	guidProperty[1]=DBPROP_IRowsetLocate;
	guidProperty[2]=DBPROP_UPDATABILITY;
	guidProperty[3]=DBPROP_REMOVEDELETED;

	//open rowset, and accessor.  (bind long cols)
	if(!GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetFind,
		NUMELEM(guidProperty),guidProperty,DBACCESSOR_ROWDATA, DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
		ALL_COLS_BOUND, FORWARD, NO_COLS_BY_REF, NULL, EXECUTE_IFNOERROR, DBTYPE_EMPTY, 
		TRUE))
	{
		fTestPass = TEST_FAIL;
		goto CLEANUP;
	}


	if (!COMPARE(DeleteRow(g_pCTable, 4), TRUE))
		goto CLEANUP;

	fTestPass =  CallFindNextRows(	g_pCTable,	     // CTable pointer
									NULL,			// bookmark;
									0,				// Length of bookmark
									1,			     // # rows to fetch
									0,				 // Offset
									g_ulColNum,	     // Which column to match
									4,				 // row to match
									DB_S_ENDOFROWSET,// HRESULT to verify
									0				 // How many rows to expect.
									);

	PopulateTable();
CLEANUP:	
	ReleaseRowsetAndAccessor();

	return fTestPass;
}
// }}

// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc *pBookMark=DBBMK_LAST, cRows=-1. Delete 1st row. Match on what was in 1st row. DB_S_ENDOFROWSET and 0 rows.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int RemoveDeleted::Variation_7()
{
	int			fTestPass=TEST_FAIL;
	DBPROPID	guidProperty[5];
	DBBOOKMARK dbBookMark = DBBMK_LAST;
	BYTE *pBookmark = (BYTE *)&dbBookMark;
	
	guidProperty[0]=DBPROP_IRowsetChange;
	guidProperty[1]=DBPROP_IRowsetLocate;
	guidProperty[2]=DBPROP_UPDATABILITY;
	guidProperty[3]=DBPROP_CANFETCHBACKWARDS;
	guidProperty[4]=DBPROP_REMOVEDELETED;

	//open rowset, and accessor.  
	if(!GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetFind,
		NUMELEM(guidProperty),guidProperty,DBACCESSOR_ROWDATA, DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
		ALL_COLS_BOUND, FORWARD, NO_COLS_BY_REF, NULL, EXECUTE_IFNOERROR, DBTYPE_EMPTY, 
		TRUE))
	{
		fTestPass = TEST_FAIL;
		goto CLEANUP;
	}

	if (!COMPARE(DeleteRow(g_pCTable, 1), TRUE))
		goto CLEANUP;

	fTestPass =  CallFindNextRows(	g_pCTable,			// CTable pointer
									pBookmark,			// bookmark;
									1,					// Length of bookmark
									-1,					// # rows to fetch
									0,					// Offset
									g_ulColNum,			// Which column to match
									1,					// row to match
									DB_S_ENDOFROWSET,	// HRESULT to verify
									0					// How many rows to expect.
								  );
	
CLEANUP:	
	ReleaseRowsetAndAccessor();
	PopulateTable();	

	return fTestPass;
}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL RemoveDeleted::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIRowsetFind::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(BookmarkSkipped)
//*-----------------------------------------------------------------------
//| Test Case:		BookmarkSkipped - Test in the context of DBPROP_BOOKMARKSKIPPED
//|	Created:			06/18/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL BookmarkSkipped::Init()
{
	BOOL fTestPass = TEST_FAIL;
	DBPROPID	DBPropID=DBPROP_BOOKMARKSKIPPED;

	if(!TCIRowsetFind::Init())
		return FALSE;

	if ( !AlteringRowsIsOK() )
		return FALSE;

	//make sure IID_IRowsetChange is supported
	TESTC_DRIVER(g_rgDBPrpt[IDX_IRowsetChange].fSupported);

	//make sure DBPROP_BOOKMARKSKIPPED is supported
	TESTC_DRIVER(g_rgDBPrpt[IDX_BookmarkSkipped].fSupported);

	//open rowset, and accessor.  Request IRowsetChange and IRowsetLocate
	TESTC_DRIVER(GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetFind,
		0,NULL));

	//return if the bookmarkskipped is not settable and variant-false
	TESTC_DRIVER(GetProp(DBPROP_BOOKMARKSKIPPED))

	
	fTestPass = TRUE;

CLEANUP:
	ReleaseRowsetAndAccessor();
	return fTestPass;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Delete 1st row. pBookmark=1st row,cRows=5. Match 2nd row.  DB_S_ENDOFROWSET(overrides S_BOOKMARKSKIPPED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int BookmarkSkipped::Variation_1()
{
	BOOL fTestPass = TEST_FAIL;;
	DBPROPID		guidProperty[5];
	BYTE *pBookmark = NULL;
	ULONG_PTR cbBookmark;
	HROW *phRows = NULL;

	phRows = (HROW *) PROVIDER_ALLOC( sizeof(HROW));

	guidProperty[0]=DBPROP_IRowsetChange;
	guidProperty[1]=DBPROP_IRowsetLocate;
	guidProperty[2]=DBPROP_BOOKMARKSKIPPED;
	guidProperty[3]=DBPROP_CANSCROLLBACKWARDS;
	guidProperty[4]=DBPROP_UPDATABILITY;

	//open rowset, and accessor.  
	if(!GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetFind,
		NUMELEM(guidProperty),guidProperty))
		goto CLEANUP;

	//get the bookmark 
	if(!GetBookmark(g_lRowLast-1, &cbBookmark, &pBookmark))
		goto CLEANUP;


	if (!COMPARE(DeleteRow(g_pCTable, g_lRowLast-1), TRUE))
		goto CLEANUP;

	fTestPass =  CallFindNextRows(	g_pCTable,	// CTable pointer
									pBookmark,  // bookmark;
									cbBookmark,	// Length of bookmark
									2,		 // # rows to fetch
									FALSE, // skip current row
									g_ulColNum,		// Which column to match
									g_lRowLast,	 // row to match
									DB_S_ENDOFROWSET,	// HRESULT to verify
									1,		// How many rows to expect.
									FALSE,  // Don't release rows
									DBCOMPAREOPS_EQ,
									SUBOP_EMPTY,
									phRows,
									FALSE	// Don't check row position
								);
	
	if (!COMPARE(fTestPass, TRUE))
		goto CLEANUP;

	if (!COMPARE(VerifyRowPosition(phRows[0], g_lRowLast, g_pCTable), TRUE))
		fTestPass = TEST_FAIL;

CLEANUP:
	PROVIDER_FREE(phRows);
	ReleaseRowsetAndAccessor();
	PopulateTable();

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Delete last row.  pBookmark=last row, cRows=1.  Match what was in last row.  DB_S_BOOKMARKSKIPPED and no rows retrieved.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int BookmarkSkipped::Variation_2()
{
	BOOL fTestPass = TEST_FAIL;
	DBPROPID		guidProperty[5];
	BYTE *pBookmark = NULL;
	ULONG_PTR cbBookmark;
	HROW *phRows = NULL;

	phRows = (HROW *) PROVIDER_ALLOC( sizeof(HROW));

	guidProperty[0]=DBPROP_IRowsetChange;
	guidProperty[1]=DBPROP_IRowsetLocate;
	guidProperty[2]=DBPROP_BOOKMARKSKIPPED;
	guidProperty[3]=DBPROP_CANSCROLLBACKWARDS;
	guidProperty[4]=DBPROP_UPDATABILITY;

	//open rowset, and accessor.  
	if(!GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetFind,
		NUMELEM(guidProperty),guidProperty))
	{
		fTestPass = TEST_FAIL;
		goto CLEANUP;
	}

	//get the bookmark 
	if(!GetBookmark(g_lRowLast, &cbBookmark, &pBookmark))
	{
		fTestPass = TEST_FAIL;
		goto CLEANUP;
	}

	if (!COMPARE(DeleteRow(g_pCTable, g_lRowLast), TRUE))
		goto CLEANUP;

	fTestPass =  CallFindNextRows(	g_pCTable,	// CTable pointer
									pBookmark,  // bookmark;
									cbBookmark,	// Length of bookmark
									1,		 // # rows to fetch
									FALSE, // skip current row
									g_ulColNum,		// Which column to match
									g_lRowLast,	 // row to match
									DB_S_ENDOFROWSET,	// HRESULT to verify
									0,		// How many rows to expect.
									FALSE,  // Don't release rows
									DBCOMPAREOPS_EQ,
									SUBOP_EMPTY,
									phRows,
									FALSE	// Don't check row position
								);

CLEANUP:
	PROVIDER_FREE(phRows);
	ReleaseRowsetAndAccessor();
	PopulateTable();

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Delete the first row. pBookmark=1st row,cRows=1.  Match 2nd row.  DB_S_BOOKMARKSKIPED and 2nd hrow returned.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int BookmarkSkipped::Variation_3()
{
	BOOL fTestPass = TEST_FAIL;;
	DBPROPID		guidProperty[5];
	BYTE *pBookmark = NULL;
	ULONG_PTR cbBookmark;
	HROW *phRows = NULL;

	phRows = (HROW *) PROVIDER_ALLOC( 2*sizeof(HROW));

	guidProperty[0]=DBPROP_IRowsetChange;
	guidProperty[1]=DBPROP_IRowsetLocate;
	guidProperty[2]=DBPROP_BOOKMARKSKIPPED;
	guidProperty[3]=DBPROP_CANSCROLLBACKWARDS;
	guidProperty[4]=DBPROP_UPDATABILITY;

	//open rowset, and accessor.  
	if(!GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetFind,
		NUMELEM(guidProperty),guidProperty))
	{
		fTestPass = TEST_FAIL;
		goto CLEANUP;
	}

	//get the bookmark 
	if(!GetBookmark(1, &cbBookmark, &pBookmark))
	{
		fTestPass = TEST_FAIL;
		goto CLEANUP;
	}

	if (!COMPARE(DeleteRow(g_pCTable, 1), TRUE))
		goto CLEANUP;

	fTestPass =  CallFindNextRows(	g_pCTable,	// CTable pointer
									pBookmark,  // bookmark;
									cbBookmark,	// Length of bookmark
									2,		 // # rows to fetch
									FALSE, // skip current row
									g_ulColNum,		// Which column to match
									2,	 // row to match
									DB_S_BOOKMARKSKIPPED,	// HRESULT to verify
									2,		// How many rows to expect.
									FALSE,  // Don't release rows
									DBCOMPAREOPS_EQ,
									SUBOP_EMPTY,
									phRows,
									FALSE	// Don't check row position
								);
	
	if (!COMPARE(fTestPass, TRUE))
		goto CLEANUP;

	if (!COMPARE(VerifyRowPosition(phRows[0], 2, g_pCTable), TRUE))
		fTestPass = TEST_FAIL;
	if (!COMPARE(VerifyRowPosition(phRows[1], 3, g_pCTable), TRUE))
		fTestPass = TEST_FAIL;


CLEANUP:
	PROVIDER_FREE(phRows);
	ReleaseRowsetAndAccessor();
	PopulateTable();

	return fTestPass;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Delete 3rd row. pBookmark=3rd row,cRows=1.  Match 5th row. DB_S_BOOKMARKSKIPPED and one hrow.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int BookmarkSkipped::Variation_4()
{
	BOOL fTestPass = TEST_FAIL;
	DBPROPID		guidProperty[5];
	BYTE *pBookmark = NULL;
	ULONG_PTR cbBookmark;
	HROW *phRows = NULL;

	phRows = (HROW *) PROVIDER_ALLOC( sizeof(HROW));

	guidProperty[0]=DBPROP_IRowsetChange;
	guidProperty[1]=DBPROP_IRowsetLocate;
	guidProperty[2]=DBPROP_BOOKMARKSKIPPED;
	guidProperty[3]=DBPROP_CANSCROLLBACKWARDS;
	guidProperty[4]=DBPROP_UPDATABILITY;

	//open rowset, and accessor.  
	if(!GetRowsetAndAccessor(g_pCTable, SELECT_ORDERBYNUMERIC, IID_IRowsetFind,
		NUMELEM(guidProperty),guidProperty))
	{
		fTestPass = TEST_FAIL;
		goto CLEANUP;
	}

	//get the bookmark 
	if(!GetBookmark(3, &cbBookmark, &pBookmark))
	{
		fTestPass = TEST_FAIL;
		goto CLEANUP;
	}

	if (!COMPARE(DeleteRow(g_pCTable, 3), TRUE))
		goto CLEANUP;

	fTestPass =  CallFindNextRows(	g_pCTable,	// CTable pointer
									pBookmark,  // bookmark;
									cbBookmark,	// Length of bookmark
									1,		 // # rows to fetch
									FALSE, // skip current row
									g_ulColNum,		// Which column to match
									5,	 // row to match
									DB_S_BOOKMARKSKIPPED,	// HRESULT to verify
									1,		// How many rows to expect.
									FALSE,  // Don't release rows
									DBCOMPAREOPS_EQ,
									SUBOP_EMPTY,
									phRows,
									FALSE	// Don't check row position
								);
	
	if (!COMPARE(fTestPass, TRUE))
		goto CLEANUP;

	if (!COMPARE(VerifyRowPosition(phRows[0], 5, g_pCTable), TRUE))
		fTestPass = TEST_FAIL;

CLEANUP:
	PROVIDER_FREE(phRows);
	ReleaseRowsetAndAccessor();
	PopulateTable();

	return fTestPass;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL BookmarkSkipped::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIRowsetFind::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCJapanese)
//*-----------------------------------------------------------------------
//| Test Case:		TCJapanese - Test scenarios relevant to the JPN locale
//| Created:  	1/24/99
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCJapanese::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCIRowsetFind::Init())
	// }}
	{ 
		// TO DO:  Add your own code here 
		return TRUE;
	} 
	return FALSE;
} 




// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Test case sensitivity and CONTAINS
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCJapanese::Variation_1()
{ 
	BOOL		fTestPass = TEST_PASS, fSubTestPass = TEST_FAIL;
	DBCOUNTITEM	cRowsObtained = 0;
	DBTYPE		wTargetType;
	CCol		TempCol;
	CTable *	pTable = NULL;
	DBCOUNTITEM	cRowsOnTable = 0;
	DBCOMPAREOP CompareOp = DBCOMPAREOPS_CONTAINS | DBCOMPAREOPS_CASEINSENSITIVE;

	//create the table
	pTable = new CTable(m_pThisTestModule->m_pIUnknown2, (WCHAR *)g_wszModuleName, NONULLS);
	if(!pTable || !SUCCEEDED(pTable->CreateTable(0,1,NULL,PRIMARY,TRUE)))
	{
		odtLog<<L"Create Table failed, test cannot proceed\n";
		SAFE_DELETE(pTable);
		return TEST_FAIL;
	}

	cRowsOnTable = pTable->GetRowsOnCTable();
	if (cRowsOnTable != 0)
	{
		// Delete All Rows
		TESTC_(pTable->DeleteRows(), S_OK);
		pTable->AddRow(cRowsOnTable);
	}

	// Insert a row
	TESTC_(pTable->Insert(0), S_OK);

	//open rowset, and accessor.  
	TESTC(GetRowsetAndAccessor(pTable, SELECT_ALLFROMTBL, IID_IRowsetFind));
	
	for (DBORDINAL ulColIndex=1; ulColIndex <= pTable->CountColumnsOnTable(); ulColIndex++)
	{
		TESTC(SUCCEEDED(m_pIRowset->RestartPosition(NULL)));
		pTable->GetColInfo(ulColIndex, TempCol);
		
		if (!IsColumnMinimumFindable(&TempCol, CompareOp))
			continue;

		wTargetType = TempCol.GetProviderType();

		if(	DBTYPE_STR == wTargetType ||
			DBTYPE_WSTR == wTargetType || 
			DBTYPE_BSTR == wTargetType)
		{

			fSubTestPass = 
			CallFindNextRows
				(	
					pTable,				// CTable pointer
					NULL,					// bookmark;
					0,						// Length of bookmark
					1,						// # rows to fetch
					0,						// offset
					ulColIndex,				// Which column to match
					cRowsOnTable+1,			// row to match
					S_OK,					// HRESULT to verify
					1,						// How many rows to expect.
					TRUE,						// Release Rows
					CompareOp,					// Specifically ask for a compare Op
					SUBOP_CONTAINS_BEGIN,				// Sub Comparision option
					NULL,						// Use client or provider memory, default=provider
					TRUE						// verify rows by comparing data ?
				);
			if ( fSubTestPass == TEST_FAIL )
			{
				fTestPass = TEST_FAIL;
				odtLog<<"Error at ColName "<< TempCol.GetColName()<<L", ColIndex "<<ulColIndex<<ENDL;
				odtLog<<"-------------------------------------------------------------"<< ENDL;
				break;
			}


		}
	}
	
CLEANUP:

	ReleaseRowsetAndAccessor();

	if (pTable)
	{
		pTable->DropTable();
		SAFE_DELETE(pTable);
	}
	return fTestPass;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCJapanese::Terminate()
{ 
	// TO DO:  Add your own code here 

// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIRowsetFind::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END

