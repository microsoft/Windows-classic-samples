//--------------------------------------------------------------------
// Microsoft OLE DB Testing
//
// Copyright 1995-2000 Microsoft Corporation.  
//
// @doc
//
// @module Privcnst.h | This module contains consts and defines for the 
//						private library
//
// @comm
// Special Notes...:	(OPTIONAL NOTES FOR SPECIAL CIRCUMSTANCES)
//
// <nl><nl>
// Revision History:<nl>
//	
//	[00] MM-DD-YY	EMAIL_NAME	ACTION PERFORMED... <nl>
//	[01] 10-16-95	Microsoft		Created - renamed from privlib.h <nl>
//	[02] 12-01-96	Microsoft	Updated for release <nl>
//	
// @head3 PRIVCNST Elements|
//	
//---------------------------------------------------------------------------

#ifndef _PRIVCNST_H_
#define _PRIVCNST_H_


//////////////////////////////////////////////////////////////////////////////
// Includes
//
//////////////////////////////////////////////////////////////////////////////
#include "modstandard.hpp"	// Need to include first, so we can override any Macros
#include <limits.h>			// MAX_PTR
#include <stdio.h>			// sprintf
#include <stddef.h> 		// For offsetof macro
							

//////////////////////////////////////////////////////////////////////////////
// Defines
//
//////////////////////////////////////////////////////////////////////////////

//Privlib Alloc Routines
inline BOOL GetUseIMallocSpy();
BOOL SetUseIMallocSpy(BOOL fValue);

inline BOOL  PrivlibValidate(HRESULT hrActual, HRESULT hrExpected, BOOL fTreatAsWarning = FALSE, WCHAR* pwszFileName = NULL, ULONG ulLine = 0);
inline BOOL  PrivlibCompare(BOOL fEqual, BOOL fTreatAsWarning = FALSE, WCHAR* pwszFileName = NULL, ULONG ulLine = 0);
inline DWORD IsWarning(HRESULT hrActual, HRESULT hrExpected);

//Create a global error object to be able to use CHECK even if not within a "privlib" object
#define TBEGIN BOOL TESTB = TEST_PASS;
#define TRETURN return TESTB;
extern BOOL TESTB;

//Output (odtLog macros)
#define TOUTPUT_(str)		odtLog << str 
#define TOUTPUT(str)		TOUTPUT_(str) << ENDL
#define TOUTPUT_LINE(str)	TOUTPUT(str << "   <File:" << __FILE__ << ">   Line:" << __LINE__)
#define TWARNING(str)		{ TOUTPUT_LINE("Warning: " << str); }
#define TERROR(str)			{ TESTB = TEST_FAIL; TOUTPUT_LINE("ERROR: " << str); (*GetModInfo()->GetErrorObject())++; }
#define	TOUTPUT_IF_FAILED(str)								\
							if (TEST_FAIL == TESTB)			\
							{								\
								TOUTPUT_(L"ERROR: ");		\
								TOUTPUT(str);				\
							}

//Absolute value of x. abs() does not work for 64bit.
#define	ABS(x)	(((x)<0) ? -(x) : (x))

#define	WIDESTRING(str)					L##str

#ifdef	_TRACING
	#define TRACE_CALL(str)		PRVTRACE(str)
#else
	#define TRACE_CALL(str)		if(0) PRVTRACE(str)
#endif

//Check MACRO - now incorporates warnings
#undef CHECK
#define CHECK(hrActual, hrExpected)		(TESTB = PrivlibValidate(hrActual, hrExpected, FALSE, LONGSTRING(__FILE__), (__LINE__)))
#define CHECKW(hrActual, hrExpected)	(TESTB = PrivlibValidate(hrActual, hrExpected, TRUE,  LONGSTRING(__FILE__), (__LINE__)))
#define QCHECK(exp,hr)					(TESTB = ((exp)==(hr)))
#define GCHECK(exp,hr)					CHECK(exp, hr)

//Compare MACRO - now incorporates warnings
#undef COMPARE
#define COMPARE(dwActual, dwExpected)	(TESTB = PrivlibCompare((dwActual) == (dwExpected), FALSE, LONGSTRING(__FILE__), (__LINE__)))
#define COMPAREW(dwActual, dwExpected)	(TESTB = PrivlibCompare((dwActual) == (dwExpected), TRUE,  LONGSTRING(__FILE__), (__LINE__)))
#define QCOMPARE(x,y)					(TESTB = ((x)==(y)))
#define GCOMPARE(x,y)					COMPARE(x, y)

//Compare (boolean)
#define TESTC(exp)						{ if(!GCOMPARE(exp, TRUE))	goto CLEANUP; }
#define TESTW(exp)						COMPAREW(exp, TRUE)
#define QTESTC(exp)						{ if(!QCOMPARE(exp, TRUE))	goto CLEANUP; }

//Compare (HRESULT)
#define TESTC_(exp, hr)					{  if(!GCHECK(exp, hr))		goto CLEANUP; }

#define TESTW_(exp,hr)					CHECKW(exp,hr)
#define QTESTC_(exp,hr)					{ if(!QCHECK((exp),hr))		goto CLEANUP; }

//Compare multiple HRESULTS
//NOTE: Notice that we use an "hrInternal" instead of repeating the "exp".  The reason is that
//the expresion may be  function, so it would otherwise translate into multiple calls of the same function:
//(ie: TEST2C_(foo(), S_OK, S_FALSE) => if(foo()!=S_OK && foo()!=S_FALSE) which you don't want)
#define TEST2C_(exp, hr1, hr2)					{ HRESULT hrInternal = (exp); if(hrInternal!=(hr1) && hrInternal!=(hr2))	TESTC_(hrInternal, (hr1)); }
#define TEST3C_(exp, hr1, hr2, hr3)				{ HRESULT hrInternal = (exp); if(hrInternal!=(hr1) && hrInternal!=(hr2) && hrInternal!=(hr3))	TESTC_(hrInternal, (hr1)); }
#define TEST4C_(exp, hr1, hr2, hr3, hr4)		{ HRESULT hrInternal = (exp); if(hrInternal!=(hr1) && hrInternal!=(hr2) && hrInternal!=(hr3) && hrInternal!=(hr4))	TESTC_(hrInternal, (hr1)); }
#define TEST5C_(exp, hr1, hr2, hr3, hr4, hr5)	{ HRESULT hrInternal = (exp); if(hrInternal!=(hr1) && hrInternal!=(hr2) && hrInternal!=(hr3) && hrInternal!=(hr4) && hrInternal!=(hr5))	TESTC_(hrInternal, (hr1)); }

//Compare HRESULTS with Extended Error checking
#define XTESTC_(pIUnknown, iid, exp, hr)						{ HRESULT hrInternal = (exp); if(!GCHECK(hrInternal, hr))	goto CLEANUP; m_pExtError->ValidateExtended(hrInternal, pIUnknown, iid, LONGSTRING(__FILE__), __LINE__);}
#define XTEST2C_(pIUnknown, iid, exp, hr1, hr2)					{ HRESULT hrInternal = (exp); if(hrInternal!=(hr1) && hrInternal!=(hr2))	XTESTC_(pIUnknown, iid, hrInternal, (hr1));	}
#define XTEST3C_(pIUnknown, iid, exp, hr1, hr2, hr3)			{ HRESULT hrInternal = (exp);	if(hrInternal!=(hr1) && hrInternal!=(hr2) && hrInternal!=(hr3))	XTESTC_(pIUnknown, iid, hrInternal, (hr1));	}
#define XTEST4C_(pIUnknown, iid, exp, hr1, hr2, hr3, hr4)		{ HRESULT hrInternal = (exp);	if(hrInternal!=(hr1) && hrInternal!=(hr2) && hrInternal!=(hr3) && hrInternal!=(hr4))	XTESTC_(pIUnknown, iid, hrInternal, (hr1));	}
#define XTEST5C_(pIUnknown, iid, exp, hr1, hr2, hr3, hr4, hr5)	{ HRESULT hrInternal = (exp); if(hrInternal!=(hr1) && hrInternal!=(hr2) && hrInternal!=(hr3) && hrInternal!=(hr4) && hrInternal!=(hr5))	XTESTC_(pIUnknown, iid, hrInternal, (hr1));	}

#define CAUSE_ERROR	m_pExtError->CauseError();

//Provider / Driver Macros
#define TESTC_PROVIDER(exp)				{ TESTB = TEST_PASS; if(!(exp)) { TOUTPUT_LINE(L"NotSupported by Provider, skipping Variation"); TESTB = TEST_SKIPPED; goto CLEANUP;	} }

//Allocation Routines...
#define FILL_PATTERN					0xC0
#define PROVIDER_ALLOC(cb)				(GetUseIMallocSpy() ? LTMALLOCSPY((cb)) : CoTaskMemAlloc((cb)))
#define PROVIDER_REALLOC(pv, cb)        (GetUseIMallocSpy() ? LTMREALLOCSPY(pv, (cb)) : CoTaskMemRealloc(pv,(cb)))

#define PROVIDER_ALLOC_(cb,type)		(type*)PROVIDER_ALLOC((cb)*sizeof(type))
#define PROVIDER_REALLOC_(pv,cb,type)	(type*)PROVIDER_REALLOC(pv,(cb)*sizeof(type))
#define PROVIDER_FREE(pv)				{ CoTaskMemFree(pv); pv = NULL;  }

#define SYSSTRING_ALLOC(pwsz)			SysAllocString(pwsz)
#define SYSSTRING_FREE(bstr)			{ if(bstr) SysFreeString(bstr); bstr = NULL; }

//Memory Allocations that also check memory automatically...
#define CHECK_MEMORY(pv)				if(!pv) goto CLEANUP
#define CHECK_MEMORY_HR(pv)				if(!pv) {hr = E_OUTOFMEMORY; goto CLEANUP;}
#define SAFE_ALLOC(pv, type, cb)		{ pv = (type*)PROVIDER_ALLOC((cb)*sizeof(type)); CHECK_MEMORY(pv);			}
#define SAFE_REALLOC(pv, type, cb)		{ pv = (type*)PROVIDER_REALLOC(pv, (cb)*sizeof(type)); CHECK_MEMORY(pv);	}
#define SAFE_SYSALLOC(pv, bstr)			{ pv = SysAllocString(bstr); CHECK_MEMORY(pv);								}		
#define SAFE_FREE(pv)					{ PROVIDER_FREE(pv);										}
#define SAFE_SYSFREE(bstr)				{ SYSSTRING_FREE(bstr); 									}
					 
#define SAFE_RELEASE(pv)  { if(pv) (pv)->Release(); pv = NULL;				} 
#define SAFE_RELEASE_(pv) { if(pv) GCOMPARE((pv)->Release(), 0); pv = NULL;	} 
#define SAFE_ADDREF(pv)	  { if(pv) (pv)->AddRef();							} 
#define SAFE_DELETE(pv)   { if(pv) delete (pv); pv = NULL;					}

#define INVALID(type)	  ((type)0x12345678)
#define ENABLE_BIT(dwValue, dwBit, fEnable)		((fEnable) ? (dwValue) |= (dwBit) : (dwValue) &= ~(dwBit))

//-----------------------------------------------------------------------------
// Defines
//-----------------------------------------------------------------------------
#define	ALLROWS	0
#define SQL_UNSEARCHABLE	0		// get rid of this at some point
#define MAXBUFLEN			256		// display buffer size
#define MAX_COL				30		// maximum column in result set
#define MAX_ROW				1000	// maximum number of rows
#define MAXDATALEN			32000	// maximum data length per column
#define MAXDISPLAYSIZE		MAX_COL*(MAXDATALEN+1)
#define DEFAULT_CBMAXLENGTH	40		// cbMaxLength for binding
#define COLUMN_ALIGNVAL		8
#define NUMROWS_CHUNK		20		// Number of Rows to Grab at a Time
#define	NUM_ROWS			15
#define MAX_COL_SIZE		755	// This does not include the null terminator for string data
#define GARBAGE				0xA3
#define MAX_INPUT_BUFFER	60000
#define MAX_VARNUM_BYTE_SIZE 111 // sizeof(DB_VARNUMERIC)+107 - providers 108 bytes for val field
#define MAX_DEPTH			64		// Maximum depth of a tree
#define MAX_VARIANT_SUBTYPES 35		// Max variant subtypes we cache (currently only 29 required)
#define MAX_LONG_COL_SIZE		755//15555	// This does not include the null terminator for string data


///////////////////////////////////////////////////////////////////
// Accessor / Binding 
//
///////////////////////////////////////////////////////////////////
//STATUS helpers, for locating obStatus offsets in the bindings
#define STATUS_IS_BOUND(Binding)    ( (Binding).dwPart & DBPART_STATUS )
#define STATUS_BINDING(Binding, pv) (*(DBSTATUS*)((BYTE*)(pv) + (Binding).obStatus))

//LENGTH helpers, for locating obLength offsets in the bindings
#define LENGTH_IS_BOUND(Binding)    ( (Binding).dwPart & DBPART_LENGTH )
#define LENGTH_BINDING(Binding, pv) (*(DBLENGTH*)((BYTE*)(pv) + (Binding).obLength))

//VALUE helpers, for locating obValue offsets in the bindings
#define VALUE_IS_BOUND(Binding)     ( (Binding).dwPart & DBPART_VALUE )
#define VALUE_BINDING(Binding, pv)  (*(void**)((BYTE*)(pv) + (Binding).obValue ))

#define ROUND_UP_AMOUNT	8
#define ROUND_UP(Size,Amount)(((ULONG_PTR)(Size)+((Amount)-1))&~((Amount)-1))

///////////////////////////////////////////////////////////////////
// 64bit
///////////////////////////////////////////////////////////////////

#define		MAX_PTR		-1

#ifdef _WIN64
	#define MAXDBCOUNTITEM	_UI64_MAX
	#define	MAXDBROWCOUNT	_I64_MAX
#else
	#define MAXDBCOUNTITEM	ULONG_MAX
	#define	MAXDBROWCOUNT		LONG_MAX
#endif

///////////////////////////////////////////////////////////////////
// Blobs
//
///////////////////////////////////////////////////////////////////
// @typedef BLOBTYPE Used in MiscFunc GetAccessorAndBindings.
typedef DWORD BLOBTYPE;

#define	NO_BLOB_COLS			0x00000001
#define BLOB_LONG				0x00000002

#define BLOB_IID_NULL			0x00000004
#define BLOB_IID_IUNKNOWN		0x00000008
#define BLOB_IID_ISTREAM		0x00000010
#define BLOB_IID_ISTORAGE		0x00000020
#define BLOB_IID_ILOCKBYTES		0x00000040
#define BLOB_IID_ISEQSTREAM		0x00000080

#define BLOB_STGM_READ			0x00001000
#define BLOB_STGM_WRITE			0x00002000
#define BLOB_STGM_READWRITE		(BLOB_STGM_READ | BLOB_STGM_WRITE)
#define BLOB_STGM_DIRECT		0x00004000
#define BLOB_STGM_TRANSACTED	0x00008000
#define BLOB_STGM_INVALID		0x00010000

#define BLOB_BIND_ALL_BLOBS		0x00100000
#define BLOB_BIND_ALL_COLS		0x00200000
#define BLOB_BIND_BINARY		0x00400000
#define BLOB_BIND_STR			0x00800000
#define BLOB_BIND_FORWARDONLY	0x02000000
#define BLOB_BIND_UDT_NO_IUNKNOWN	0x04000000	// Do not bind UDT as IUnknown
#define BLOB_BIND_NATIVE_XML	0x08000000		// Bind xml column as DBTYPE_XML


#define BLOB_NULL_POBJECT		0x01000000


//-----------------------------------------------------------------------------
// Data Structures
//-----------------------------------------------------------------------------
// @struct DATA | structure used for IRowset->GetData
struct DATA 
{
	DBSTATUS sStatus;			// @field SHORT | sStatus  | Status of bound value
	DBLENGTH ulLength;			// @field ULONG | ulLength | On read: Total length of value, On Write, length to use
	union
	{
		BYTE bValue[1];		// @field BYTE[1] | bValue | Data Value 
		DBLENGTH alignField;	// @field ULONG | alignField | Union member in existence
							// only to ensure that bValue is placed on a ULONG byte
							// alignment, so regardless of what type is accessed
							// at bValue, an alignment fault will not occur.
	};
}; 

//-----------------------------------------------------------------------------
// ENUM
//-----------------------------------------------------------------------------
// @enum ECOLUMNORDER|Ordering of the Columns in a query. Used inside the CTable functions.
enum ECOLUMNORDER
{
	FORWARD,	// @emem FORWARD | Columns ordering for query is forward.
	REVERSE		// @emem REVERSE | Columns ordering for query is reverse.
};

// @enum EVALUE|Original value or the second value for the field. 
// Used in the MakeData function of CTable.
enum EVALUE 
{
	PRIMARY,	// @emem  PRIMARY |The primary value will be used for this row and column.
	SECONDARY	// @emem SECONDARY | The secondary value will be used for this row and column.
};

// @enum EINDEXTYPE|Uniqueness of data. Used in a member data of CCol.
enum EINDEXTYPE
{
	UNIQUE,		// @emem  UNIQUE |the primary value will be used for this row and column.
	NONUNIQUE	// @emem  NONUNIQUE |the secondary value will be used for this row and column.
};

// @enum EDATATYPES|Column data types. Used in the CreateTable functions of
// CTable.
enum EDATATYPES
{
	ALLTYPES,	// @emem  ALLTYPES |the table will be created will all the supported data types.
	NATIVETYPES,// @emem  NATIVETYPES |the table will be created with the native types listed.
	DBDATATYPES	// @emem  DBDATATYPES |the table will be created with the DBDATATYPEENUM types listed.
};

// @enum EMARKOBJECT|Clear or set object.
enum EMARKOBJECT
{
	CLEAR,		//@emem CLEAR|Clears an object.
	SET			//@emem SET|Sets an object.
};

// @enum ENULL|Nullability of data. Used in a member data of CCol.
enum ENULL 
{
	NONULLS,			// @emem  NONULLS |the data will not be null.
	USENULLS,			// @emem USENULLS |data will be null if the row and col numbers are equal and the type is nullable.
	NULLABLE,			// @emem NULLABLE |column can be nullable.
	NULLABLE_UNKNOWN	// @emem NULLABLE_UNKNOWN |from sqlcolinfo.
};

// @enum ECOLS_IN_BINDINGS. Used in GetAccessorAndBindings to determine which columns to bind to. 
enum	ECOLS_BOUND
{
	ALL_COLS_BOUND					= 0x00000001,	//@emem ALL_COLS | All columns in rowset will be bound
	ODD_COLS_BOUND					= 0x00000002,	//@emem ODD_COLS | Only odd numbered columns in rowset will be bound
	EVEN_COLS_BOUND					= 0x00000004,	//@emem EVEN_COLS | Only even numbered columns in rowset will be bound.  
	USE_COLS_TO_BIND_ARRAY			= 0x00000008,	//@emem USE_COLS_TO_BIND_ARRAY | Only columns specified in the rgColsToBind parameter are bound
	
	FIXED_LEN_COLS_BOUND			= 0x00000010,	//@emem FIXED_LEN_TYPE_COLS | Only Fixed length type columns are bound
	VARIABLE_LEN_COLS_BOUND			= 0x00000020,	//@emem VARIABLE_LEN_TYPE_COLS | Only Variable length type columns are bound
	BLOB_COLS_BOUND					= 0x00000040,	//@emem BLOB_COLS | Only BLOB columns are bound
	UPDATEABLE_COLS_BOUND			= 0x00000080,	//@emem UPDATEABLE_COLS_BOUND | Only Updateable (writeable) columns are bound
	
	NONINDEX_COLS_BOUND				= 0x00000100,	//@emem UPDATEABLE_NONINDEX_COLS_BOUND | Only Updateable (writeable) columns excluding the index column are bound
	NONNULLABLE_COLS_BOUND			= 0x00000200,	//@emem NONNULLABLE_COLS_BOUND | Only Non-Nullable columns are bound
	NOBOOKMARK_COLS_BOUND			= 0x00000400,	//@emem ALL_COLS | All columns in rowset will be bound except the Bookmark Column
	NULLABLE_COLS_BOUND				= 0x00000800,	//@emem NONNULLABLE_COLS_BOUND | Only Non-Nullable columns are bound

	VECTOR_COLS_BOUND				= 0x00001000,	//@emem Only Vector columns are bound
	NOVECTOR_COLS_BOUND				= 0x00002000,	//@emem All columns except Vectors

	//Backward compatible flags
	ALL_COLS_EXCEPTBOOKMARK			= (ALL_COLS_BOUND | NOBOOKMARK_COLS_BOUND),
	UPDATEABLE_NONINDEX_COLS_BOUND  = (UPDATEABLE_COLS_BOUND | NONINDEX_COLS_BOUND),
};

// @enum ECOLS_IN_LIST. Used in CreateColList to determine which columns to put in list.
enum	ECOLS_IN_LIST
{
	ALL_COLS_IN_LIST,			//@emem ALL_COLS | All columns in rowset will be in list		
	UPDATEABLE_COLS_IN_LIST,	//@emem UPDATEABLE_COLS_IN_LIST | Only Updateable (writeable) columns are in the list
	SEARCHABLE_COLS_IN_LIST,	//@emem SEARCHABLE_COLS_IN_LIST | Only searchable columns are in the list
	SEARCHABLE_AND_UPDATEABLE_COLS_IN_LIST, // @emem SEARCHABLE_AND_UPDATEABLE_COLS_IN_LIST | All columns which and searchable and updateable.
	INDEX_COL_IN_LIST,
	ECOLS_LAST					// Must remain last enum in list
};

// @enum ECOLS_BY_REF. Used in CTable.
enum	ECOLS_BY_REF		
{
	ALL_COLS_BY_REF,					//@emem ALL_COLS | All columns in the accessor will be bound by reference
	NO_COLS_BY_REF,						//@emem NO_COLS | No columns in the accessor will be bound by reference
	FIXED_LEN_COLS_BY_REF,				//@emem FIXED_LEN_TYPE_COLS | Only columns with fixed 
											//length types will be bound by reference
	VARIABLE_LEN_COLS_BY_REF,			//@emem VARIABLE_LEN_TYPE_COLS | Only columns with variable
											//length types will be bound by reference
	SUPPORTED_COLS_BY_REF,				//@emem SUPPORTED_COLS | Supported columns in the accessor will be bound by reference
	SUPPORTED_FIXED_LEN_COLS_BY_REF		//@emem SUPPORTED_FIXED_LEN_TYPE_COLS | Only supported columns with fixed 
											//length types will be bound by reference
};

// @enum ECOLS_MEM_PROV_OWNED. Used in CTable.
enum	ECOLS_MEM_PROV_OWNED		
{
	NO_COLS_OWNED_BY_PROV,		//@emem NO_COLS_OWNED_BY_PROV | All columns' bMemOwner is DBMEMOWNER_CLIENTOWNED
	SUPPORTED_COLS_OWNED_BY_PROV,//@emem SUPPORTED_COLS_OWNED_BY_PROV | Supported columns' bMemOwner is DBMEMOWNER_PROVIDEROWNED
	ALL_COLS_OWNED_BY_PROV		//@emem NO_COLS_OWNED_BY_PROV | All columns' bMemOwner is DBMEMOWNER_PROVIDEROWNED	
};

// @enum EEXECUTE|When to execute the SQL statement in CTable.
enum EEXECUTE
{
	EXECUTE_NEVER,		//@emem  EXECUTE_NEVER |Never call ICommand::Execute
	EXECUTE_ALWAYS,		//@emem  EXECUTE_ALWAYS |Always call ICommand::Execute
						//even if some of the properties do not get set
	EXECUTE_IFNOERROR	//@emem  EXECUTE_IFNOERROR |Call ICommand::Execute
						//only if ALL the properties passed in by the user get set
						//If there are no properties to set them always execute.fs
};

// @enum EQUERY|Available SQL statements that can be generated.
// Note that the Schema tables can only be used in the CRowset class. 
enum EQUERY
{
	USE_OPENROWSET,								//@emem Uses IOpenRowset::OpenRowset, which is essentially "select * from <tbl>"
	USE_SUPPORTED_SELECT_ALLFROMTBL,			//@emem Uses command to execute "select * from <tbl>" if supported, else
												// uses IOpenRowset::OpenRowset to accomplish "select * from <tbl>"

	SELECT_ALLFROMTBL,							//@emem Uses command to execute "SELECT * FROM <tbl>"	
	SELECT_SEARCHABLE,
	SELECT_UPDATEABLE,
	SELECT_ABCANDCOLLIST,						//@emem Uses command to execute "Select 'ABC', <col list> from <tbl>"
	SELECT_DISTINCTCOLLISTORDERBY,				//@emem Uses command to execute "Select DISTINCT <col list> from <tbl> order by <col one> DESC"
	SELECT_REVCOLLIST,							//@emem Uses command to execute "Select <reverse col list> from <tbl>"
	SELECT_COLLISTGROUPBY,						//@emem Uses command to execute "Select <col one> from <tbl> GROUP BY <col one> HAVING <col one> is not null"
	SELECT_COLLISTWHERELASTCOLINSELECT,			//@emem Uses command to execute "Select <col list> from <tbl> where <last col> in (Select <last col> from <tbl>)"
	SELECT_REVCOLLISTFROMVIEW,					//@emem Uses command to execute "Select <reverse col list> from <view>"
	SELECT_COUNT,								//@emem Uses command to execute "Select count(<col one>) from <tbl>"
	SELECT_COLLISTSELECTREVCOLLIST,				//@emem Uses command to execute "Select <col list> from <tbl>; Select <reverse col list> from <tbl>"
	SELECT_EMPTYROWSET,							//@emem Uses command to execute "Select <col list> from <tbl> where 0=1"
	SELECT_COLLISTFROMTBL,						//@emem Uses command to execute "Select <col list> from <tbl>"
	SELECT_COLLISTTBLUNIONTBL,					//@emem Uses command to execute "Select <col list> from <tbl> UNION select <col list> from <tbl>"
	SELECT_COLLISTORDERBYCOLONECOMPUTE,			//@emem Uses command to execute "Select <col list> from <tbl> ORDER BY <col one> COMPUTE SUM(<col one>)"
	SELECT_COLLISTORDERBYCOLONECOMPUTEBY,		//@emem Uses command to execute "Select <col list> from <tbl> ORDER BY <col one> COMPUTE SUM(<col one>) by <col one>"
	SELECT_CROSSPRODUCT,						//@emem Uses command to execute "Select * from  <tbl1>, <tbl2>"
	SELECT_LEFTOUTERJOIN,						//@emem Uses command to execute "Select * from <tbl1> LEFT OUTER JOIN <tbl2> on <tbl1.col1> = <tbl2.col1>"
	SELECT_RIGHTOUTERJOIN,						//@emem Uses command to execute "Select * from <tbl1> RIGHT OUTER JOIN <tbl2> on <tbl1.col1> = <tbl2.col1>"
	SELECT_LEFTOUTERJOIN_ESC,					//@emem Uses command to execute "Select * from {oj <tbl1> LEFT OUTER JOIN <tbl2> on <tbl1.col1> = <tbl2.col1>}"
	SELECT_RIGHTOUTERJOIN_ESC,					//@emem Uses command to execute "Select * from {oj <tbl1> RIGHT OUTER JOIN <tbl2> on <tbl1.col1> = <tbl2.col1>}"
	SELECT_FROMTBLWITHPARAMS,					//@emem Uses command to execute "Select <col list> from <tbl> where <parm col list>"
	SELECT_CHANGECOLNAME,						//@emem Uses command to execute "Select col1X=col1, col2X=col2, ... from <tbl>
	SELECT_DUPLICATECOLUMNS,					//@emem Uses command to execute "Select <col list> , <col list> from tbl"
	SELECT_REVERSEDUPLICATECOLUMNS,				//@emem Uses command to execute "Select <reverse col list> , <reverse col list> from <tbl>
	SELECT_MAXCOLINQUERY,						//@emem Not currently working.
	SELECT_COMPUTEDCOLLIST,						//@emem Uses command to execute "Select colx, colx-colx from <tbl>"  colx is first numeric col
	SELECT_UPDATEABLEALLROWS,					//@emem Uses command to execute "Select colx, coly from <tbl>"  colx and coly are all updateable columns
	SELECT_ORDERBYNUMERIC,						//@emem Uses command to execute "select * from <tbl> order by colx"  colx is the first numeric col
	SELECT_DBSCHEMA_ASSERTIONS,					//@emem Uses IDBSchemaRowset::GetRowset to generate Assertions Schema
	SELECT_DBSCHEMA_CATALOGS,					//@emem Uses IDBSchemaRowset::GetRowset to generate Catalogs Schema
	SELECT_DBSCHEMA_CHARACTER_SETS,				//@emem Uses IDBSchemaRowset::GetRowset to generate character sets Schema
	SELECT_DBSCHEMA_CHECK_CONSTRAINTS,			//@emem Uses IDBSchemaRowset::GetRowset to generate Check constraints Schema
	SELECT_DBSCHEMA_COLLATIONS,					//@emem Uses IDBSchemaRowset::GetRowset to generate collations	Schema
	SELECT_DBSCHEMA_COLUMN_DOMAIN_USAGE,		//@emem Uses IDBSchemaRowset::GetRowset to generate Column Domain Usage Schem
	SELECT_DBSCHEMA_COLUMN_PRIVILEGES,			//@emem Uses IDBSchemaRowset::GetRowset to generate Column privileges Schema
	SELECT_DBSCHEMA_COLUMNS,					//@emem Uses IDBSchemaRowset::GetRowset to generate Columns Schema
	SELECT_DBSCHEMA_CONSTRAINT_COLUMN_USAGE,	//@emem Uses IDBSchemaRowset::GetRowset to generate Constraint column usage Schema
	SELECT_DBSCHEMA_CONSTRAINT_TABLE_USAGE,		//@emem Uses IDBSchemaRowset::GetRowset to generate Constraint table usage Schema
	SELECT_DBSCHEMA_FOREIGN_KEYS,				//@emem Uses IDBSchemaRowset::GetRowset to generate Foreign Keys
	SELECT_DBSCHEMA_INDEXES,					//@emem Uses IDBSchemaRowset::GetRowset to generate Indexes Schema
	SELECT_DBSCHEMA_KEY_COLUMN_USAGE,			//@emem Uses IDBSchemaRowset::GetRowset to generate Key Column Usage Schema
	SELECT_DBSCHEMA_PRIMARY_KEYS,				//@emem Uses IDBSchemaRowset::GetRowset to generate Primary Keys
	SELECT_DBSCHEMA_PROCEDURE_PARAMETERS,		//@emem Uses IDBSchemaRowset::GetRowset to generate Procedures Parameters Schema
	SELECT_DBSCHEMA_PROCEDURES,					//@emem Uses IDBSchemaRowset::GetRowset to generate Procedures Schema
	SELECT_DBSCHEMA_PROVIDER_TYPES,				//@emem Uses IDBSchemaRowset::GetRowset to generate Provider Types
	SELECT_DBSCHEMA_REFERENTIAL_CONSTRAINTS,	//@emem Uses IDBSchemaRowset::GetRowset to generate Constraints Schema
	SELECT_DBSCHEMA_SCHEMATA,					//@emem Uses IDBSchemaRowset::GetRowset to generate Schemata Schema
	SELECT_DBSCHEMA_SQL_LANGUAGES,				//@emem Uses IDBSchemaRowset::GetRowset to generate Sql Languages Schema
	SELECT_DBSCHEMA_STATISTICS,					//@emem Uses IDBSchemaRowset::GetRowset to generate Statistics Schema
	SELECT_DBSCHEMA_TABLE_CONSTRAINTS,			//@emem Uses IDBSchemaRowset::GetRowset to generate Table Constraints Schema
	SELECT_DBSCHEMA_TABLE_PRIVILEGES,			//@emem Uses IDBSchemaRowset::GetRowset to generate Table Privileges Schema
	SELECT_DBSCHEMA_TABLE,						//@emem Uses IDBSchemaRowset::GetRowset to generate Table Schema
	SELECT_DBSCHEMA_TRANSLATIONS,				//@emem Uses IDBSchemaRowset::GetRowset to generate Translations Schema
	SELECT_DBSCHEMA_USAGE_PRIVILEGES,			//@emem Uses IDBSchemaRowset::GetRowset to generate Usage Privileges Schema
	SELECT_DBSCHEMA_VIEW_COLUMN_USAGE,			//@emem Uses IDBSchemaRowset::GetRowset to generate View Column Usage Schema
	SELECT_DBSCHEMA_VIEW_TABLE_USAGE,			//@emem Uses IDBSchemaRowset::GetRowset to generate View Table Usage Schema
	SELECT_DBSCHEMA_VIEWS,						//@emem Uses IDBSchemaRowset::GetRowset to generate Views Schema
	INSERT_1ROW,								//@emem Inserts 1 row of data, may use IRowsetChange - caution, IRowsetChange won't return command text
	INSERT_ROW_WITH_LITERALS,					//@emem Inserts 1 row of data using literals
	INSERT_ALLWITHPARAMS,						//@emem Uses command to execute Parameterized Insert statement with all updateable columns
	SELECT_ALL_WITH_SEARCHABLE_AND_UPDATEABLE,	//@emem Uses command to execute Select statament have searchable and updateable cols in LIST.where params
	SELECT_ALL_WITH_BLOB_AT_END,				//@emem Uses normal select all but moves blob to last column
	NO_QUERY,
	SELECT_ALL_WITH_FOR_BROWSE,					//@emem sql server only for dev bug 2358
	SELECT_ALL_WITH_FOR_UPDATE,					//@emem Uses command to execute "Select * from <tbl> for update"
	CREATE_VIEW,								//@emem Uses command to execute "Create View <view-name> as select * from <table-name>"
	DROP_VIEW,									//@emem Uses command to execute "Drop View <view-name>"
	CREATE_INDEX,								//@emem Uses command to execute "Create index %s on %s (%s)"
	DROP_INDEX,									//@emem Uses command to execute "Drop Index %s.%s"
	ALTER_TABLE_DROP_COLUMN,					//@emem Uses command to execute "Alter table %s drop column %s"
	ALTER_TABLE_ADD,							//@emem Uses command to execute "Alter table %s add %s"
	ALTER_TABLE_ADD_EX,							//@emem Uses command to execute "Alter table %s add %s %s" - extended
	SELECT_ROW_WITH_LITERALS,					//@emem Uses command to execute "Select <col list> from <tbl> where <searchable and updateable literal col list>"
	USE_ENUMERATOR,								//@emem Uses ISourcesRowset on the Enumerator specified with ENUMERATOR initstring
	USE_COLUMNSROWSET,							//@emem Uses IColumnsRowset
	CREATE_PROC,								//@emem Uses command to execute "Create Procedure <proc-name> as select * from <table-name>"
	DROP_PROC,									//@emem Uses command to execute "Drop Procedure <proc-name>"
	EXEC_PROC,									//@emem Uses command to execute "{call proc-name}"
	DROP_TABLE,									//@emem Uses command to execute "Drop table <table-name>"
	SELECT_NO_TABLE,							//@emem Uses command to execute "select * from" leaving off table name
	INSERT_NO_TABLE,							//@emem Uses command to execute "insert into" leaving off table name
	SELECT_INVALIDGROUPBY,						//@emem Uses command to execute "select * from <table-name> group by <column_name>"
	UPDATE_WITH_PARAMS_WHERE,					//@emem Uses command to execute "update <table-name> set col1 = ?, col2 = ? ... where col1 = ? and col2 = ? ..."

	DEEP_SELECT_SUBTREE,						//@emem Uses row scoped command to retrieve all child nodes of a row object(similar to IScopedOperations::OpenRowset);
	SHALLOW_SCOPED_SELECT,						//@emem Uses row scoped command to retrieve all direct child nodes of a row object
	INSERT_INVALID_KEYWORD,						//@emem Builds a statement of the form "insert inot ..." to generate a syntax error
	CREATE_INDEX_DESC,							//@emem Uses command to execute "Create index %s on %s (%s DESC)"
	CHANGE_CURRENT_CATALOG,						//@emem Uses command to change the current catalog (e.g "use pubs")
	DELETE_ALLWITHPARAMS,						//@emem Uses command to execute Parameterized Delete statement with all updateable columns
	SELECT_ALL_BYINDEX_WITHPARAMS,				//@emem Uses command to execute Parameterized select using index column in where clause
	RPC_SELECT_ALL_BYINDEX_WITHPARAMS,
	CALL_RPC,	

	SELECT_INVALIDTBLNAME,						//@emem Uses command to execute "select * from Xxxxx" where Xxxxx is not valid table name
	SELECT_VALIDATIONORDER,						//@emem Used to execute select * from <table-name> order by numeric in case of backends where the validation order is 
												//@emem not deterministic and excludes the order by clause for providers that return the query results in deterministic order

	SELECT_DISTINCTCOLLIST,						//@emem Uses command to execute "Select DISTINCT <col list> from <tbl>"

	
	//Queries that will successfully Execute when set on
	//ICommandStream with DBGUID_DEFAULT.
	SETCMDSTREAM_QUERY1,			
	SETCMDSTREAM_QUERY2,

	LASTENUM,									//@emem Used in the allocation of the structure, always last
};

// @enum EINTERFACE | CoClass
enum	EINTERFACE
{
	UNKNOWN_INTERFACE,				//@emem TUnknown
	ENUMERATOR_INTERFACE,			//@emem TEnumerator 
	DATASOURCE_INTERFACE,			//@emem TDataSource
	SESSION_INTERFACE,				//@emem	TSession
	COMMAND_INTERFACE,				//@emem	TCommand
	ROWSET_INTERFACE,				//@emem	TRowset
	VIEW_INTERFACE,					//@emem	TView
	MULTIPLERESULTS_INTERFACE,		//@emem	TMultipleResults
	INDEX_INTERFACE,				//@emem	TIndex
	TRANSACTION_INTERFACE,			//@emem	TTransaction
	TRANSACTIONOPTIONS_INTERFACE,	//@emem	TTransactionOptions
	ERROR_INTERFACE,				//@emem	TErrorObject
	CUSTOMERROR_INTERFACE,			//@emem	TCustomErrorObject
	ROW_INTERFACE,					//@emem	TRow
	STREAM_INTERFACE,				//@emem	TStream		
	BINDER_INTERFACE,				//@emem	TBinder		
	INVALID_INTERFACE,				//@emem	Always the last element in the list
};


// @enum ECONFLEVEL | Used in the Conforamance methods.
//Enumeration to signify level of conformance by the provider.
//Levels can be or'd together as well as aditional functionality.
//Example:  (CONF_LEVEL_1 | CONF_UPDATEABLE | CONF_COMMANDS) ==
//equals a provider that meets level 1 conformance which also supports
//commands and is updateable.
enum ECONFLEVEL
{
	//Levels
	CONF_LEVEL_0		= 0x00000000,
	CONF_LEVEL_1		= (CONF_LEVEL_0 | 0x10000000),
	CONF_LEVEL_2		= (CONF_LEVEL_1 | 0x20000000),
	
	//Additional Functionality
	CONF_UPDATEABLE		= 0x00000001,
	CONF_TRANSACTIONS   = 0x00000002,
	CONF_COMMANDS		= 0x00000004,
	CONF_FILTERS		= 0x00000008,
	CONF_INDEXES		= 0x00000010,
};

#define CONF_LEVEL(dwLevel)				((dwLevel) & 0x10000000)
#define CONF_REQ(dwLevel)				((dwLevel) & ~CONF_LEVEL(dwLevel))
#define CONF_REQ_UPDATEABLE(dwLevel)	((dwLevel) & CONF_UPDATEABLE)
#define CONF_REQ_TRANSACTIONS(dwLevel)	((dwLevel) & CONF_TRANSACTIONS)
#define CONF_REQ_COMMANDS(dwLevel)		((dwLevel) & CONF_COMMANDS)
#define CONF_REQ_FILTERS(dwLevel)		((dwLevel) & CONF_FILTERS)
#define CONF_REQ_INDEXES(dwLevel)		((dwLevel) & CONF_INDEXES)


// @enum EROWSETGENERATED| Used in the Transaction class.
enum	EROWSETGENERATED
{
	OPENROWSET_GENERATED,	//@emem IOpenRowset::OpenRowset should be used to generate rowset
	COMMAND_GENERATED,		//@emem	A Command object should be used to generate the rowset
	EITHER_GENERATED		//@emem If commands are supported, they are used, else IOpenRowset is used
};

// @enum EREINITIALIZE| Used in the COLEDB class.
enum	EREINITIALIZE
{
	REINITIALIZE_YES,		//@emem	Reinitialize if already initialized
	REINITIALIZE_NO			//@emem	Don't Reinitialize if already initialized
};

// @enum EDELETETABLE| Used in the COLEDB class.
enum	EDELETETABLE
{	
	DELETETABLE_YES,		//@emem	Table is deleted by object when release is called
	DELETETABLE_NO			//@emem	Table is not deleted by the object
};

// @enum COLMETADATA_INDEX
enum COLMETADATA_INDEX { 
	COL_NAME=0,
	COL_NUMBER,
	COL_DBTYPE,
	COL_SIZE,
	COL_PRECISION,
	COL_SCALE,
	COL_FLAGS,
	COL_IDKIND,
	COL_IDGUID,
	COL_IDNAME,
	COL_PREFIX,
	COL_SUFFIX
};

// @enum LIST_TYPE
enum ELIST_TYPE { 
	LT_COLNAME,				// List of comma separated column names
	LT_PARAM,				// List of comma separated parameter markers (?)
	LT_LITERAL,				// List of comma separated literal values (value1, ...)
	LT_PARAM_SEARCH,		// List of column names = param marker (col1 = ? and ...)
	LT_PARAM_UPDATE,		// List of column names = param marker (col1 = ?,  ...)
	LT_LITERAL_SEARCH,		// List of column names = literal (col1 = value1 and ...)
	LT_PARAM_OUT,			// List of comma separated out params (? = col1, ? = col2, ...)
	LT_RPC_PARAM_DEF,		// List of rpc param definitions (@picol1 <type>(len), @picol22 <type>(len), ...)
	LT_RPC_OUT_PARAM_DEF,	// List of rpc param definitions (@piocol11 <type>(len) out, @piocol2 <type>(len) out, ...)
	LT_RPC_PARAM_OUT,		// List of rpc out params (@piocol1 = col1, @piocol2 = col2, ...)
	LT_RPC_PARAM_SEARCH,	// List of rpc search params (col1 = @picol1 and col2 = @picol2 ...)
	LT_LAST	
};

// @enum CREATE_PK
enum ECREATE_PK { 
	CREATENEVER_PK,				// Primary key will never be created on a table (ex: in case of SQL Server backend xml column will not be included in a table)
	CREATEALWAYS_PK,			// Primary key will be always created
	CREATEIFNEEDED_PK,			// Primary key will be created if it's needed (ex: SQL Server backend and a table contatins xml column)
};

//-----------------------------------------------------------------------------
// Non string constants
//-----------------------------------------------------------------------------
// US English LCID
const ULONG LOCALE_ENGLISH_US = MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT); 

//-----------------------------------------------------------------------------
// String constants
//-----------------------------------------------------------------------------
const WCHAR	 wszRESULTNULL[]=L"<NULL>";					// @const Null string.
const WCHAR	 wszCANTCOERCE[]=L"<can't coerce>";			// @const
const WCHAR  wszUNKNOWNSTATUS[]=L"<unknown status>";	// @const

const WCHAR wszNOCOMMANDOBJECT[]=L"Command object not found\n";			// @const
const WCHAR wszNOROWSETOBJECT[]=L"Rowset object not found\n";			// @const
const WCHAR wszNOSESSIONOBJECT[]=L"Session object not found\n";			// @const
const WCHAR wszNODATASOURCEOBJECT[]=L"Data source object not found\n";	// @const

const WCHAR	 wszNUMERIC[]=L"NUMERIC";	// @const
const WCHAR	 wszCHAR[]=L"CHAR";			// @const
const WCHAR	 wszBIT[]=L"BIT";			// @const
const WCHAR	 wszDECIMAL[]=L"DECIMAL";	// @const
const WCHAR	 wszFLOAT[]=L"FLOAT";		// @const

const WCHAR  wszABC[]=L"ABC";			// @const Used in CreateSQLStmt
const WCHAR  wszEndOfLongData[]=L"%'";	// @const

const WCHAR wszEmptyString[]	= L"<Empty String>";		// @const
const WCHAR wszCanntCoerce[]	= L"<Can't Coerce>";		// @const
const WCHAR wszUnknownStat[]	= L"<Unknown Status>";		// @const
const WCHAR wszTrucated[]		= L"<Data Truncated>";		// @const
const WCHAR wszMisMatch[]		= L"<Sign Mismatch>";		// @const
const WCHAR wszOverflow[]		= L"<Data Overflow>";		// @const
const WCHAR wszCanntCreate[]	= L"<Can't Create>";		// @const
const WCHAR wszUnavailable[]	= L"<Unavailable>";			// @const
const WCHAR wszViolation[]		= L"<Access Violation>";	// @const
const WCHAR wszIntegrity[]		= L"<Integrity Violation>";	// @const
const WCHAR wszSchema[]			= L"<Schema Violation>";	// @const
const WCHAR wszReturnNULL[]		= L"<NULL>";				// @const

const WCHAR wszCommandObjectDoesntExist[] = L"Command Object doesn't exist.\n";	// @const
const WCHAR	wszInterfaceNotSupported[] = L"Interface not supported.\n";			// @const
const WCHAR	wszBookmarksNotSupported[] = L"Bookmarks are not supported.\n";		// @const
const WCHAR	wszTransactionNotSupported[] = L"Transactions are not supported. \n";	// @const
const WCHAR	wszErrorGettingInterface[] = L"Error on QueryInterface.\n";			// @const
const WCHAR	wszConformanceLevelError[] = L"ERROR: Conformance Level Error!\n";	// @const
const WCHAR	wszSelectStar[] = L"Select * from ";								// @const

const WCHAR COLTRUNC_WARNG[]	=L"Number of columns in display truncated to ";	// @const
const WCHAR ROWTRUNC_WARNG[]	=L"Number of rows in display truncated to ";	// @const
const WCHAR ENDL[]				=L"\n";											// @const

const WCHAR	 wszPRIVLIBT[]=L"**PrivLib.dll(CTable::";	// @const
const WCHAR	 wszPRIVLIBC[]=L"**PrivLib.dll(CCCol::";	// @const
const WCHAR  wszFAILED[]=L"FAILED";						// @const
const WCHAR	 wszSUCCEEDED[]=L"SUCCEEDED";				// @const
const WCHAR  wszFUNCFAIL[]=L"Function failed";			// @const
const WCHAR  wszFUNCPASS[]=L"Function succeeded";		// @const

const LONG	TABLELENGTH=30;				// @const SWORD|TABLELENGTH|30
const LONG	TABLELENGTH1=8;				// @const SWORD|TABLELENGTH1|8
const ULONG	MAX_SCALE = 5;				// @const UDWORD|MAX_SCALE|5
const ULONG	MAX_WCHAR = 200;
const ULONG	MAX_ULONG = 12;
const ULONG MODULENAME = 8;
const WCHAR	 wszDEFAULT[]=L"DEFAULT";
const WCHAR  wszUNDERSCORE[]=L"_";		// @const WCHAR *|wszUNDERSCORE|[]=L"_")
const WCHAR  wszCOL[]=L"col";			// @const WCHAR *|wszCOL|[]=L"col")
const WCHAR  wszSPACE[]=L" ";			// @const WCHAR *|wszSPACE|[]=L" ")
const WCHAR  wszYES[]=L"YES";			// @const WCHAR *|wszYES|[]=L"YES")
const WCHAR  wszNO[]=L"NO";				// @const WCHAR *|wszNO|[]=L"NO")
const WCHAR  wszLEFTPAREN[]=L" (";		// @const WCHAR *|wszLEFTPAREN|[]=L"(")
const WCHAR  wszRIGHTPAREN[]=L") ";		// @const WCHAR *|wszRIGHTPAREN|[]=L")")
const WCHAR  wszCOMMA[]=L",";			// @const WCHAR *|wszCOMMA|[]=L",")
const WCHAR  wszQUOTE[]=L"'";			// @const WCHAR *|wszQUOTE|[]=L"'")
const WCHAR  wszEQUALS[]=L"=";			// @const WCHAR *|wszEQUALS|[]=L"=")
const WCHAR  wszPERIOD[]=L".";			// @const WCHAR *|wszPERIOD|[]=L".")
const WCHAR  wszPOUND[]=L"#";			// @const WCHAR *|wszPOUND|[]=L"#")
const WCHAR  wszSTAR[]=L" * ";			// @const WCHAR  |wszSTAR|[]=L" * ")
const WCHAR  wszLIKE[]=L" LIKE ";		// @const WCHAR  |wszLIKE|[]=L" LIKE ")
const WCHAR  wszVALUES[]=L" VALUES ";	// @const WCHAR  |wszVALUES|[]=L" VALUES ")
const WCHAR  wszWHERE[]=L" WHERE ";		// @const WCHAR  |wszWHERE|[]=L" WHERE ")
const WCHAR  wszFROM[]=L" FROM ";		// @const WCHAR  |wszFROM|[]=L" FROM ")
const WCHAR  wszSET[]=L" SET ";			// @const WCHAR  |wszSET|[]=L" SET ")
const WCHAR  wszINTO[]=L" INTO ";		// @const WCHAR  |wszINTO|[]=L" INTO ")
const WCHAR  wszUNIQUE[]=L" UNIQUE ";	// @const WCHAR  |wszUNIQUE|[]=L" UNIQUE ")
const WCHAR  wszON[]=L" ON ";			// @const WCHAR  |wszON|[]=L" ON ")
const WCHAR  wszISNULL[]=L" IS NULL";	// @const WCHAR  |wszISNULL|[]=L" IS NULL")
const WCHAR  wszNULL[]=L" NULL ";		// @const WCHAR  |wszNULL|[]=L"NULL")
const WCHAR  wszNOTNULL[]=L" NOT NULL ";// @const WCHAR  |wszNOTNULL|[]=L" NOT NULL")
const WCHAR  wszAND[]=L" AND ";			// @const WCHAR  |wszAND|[]=L" AND ")
const WCHAR  wszSQLCOUNT[]=L" COUNT ";	// @const WCHAR  |wszSQLCOUNT|[]=L" COUNT ")
const WCHAR  wszQUESTION[] = L"?";		// @const WCHAR  |wszQUESTION|[]=L"?")

const WCHAR  wszCREATETABLE[]=L" CREATE TABLE ";		// @const WCHAR  |wszCREATETABLE|[]=L" CREATE TABLE ")
const WCHAR  wszDROPTABLE[]=L" DROP TABLE ";			// @const WCHAR  |wszDROPTABLE|[]=L" DROP TABLE ")
const WCHAR  wszDROPVIEW[]=L" DROP VIEW ";				// @const WCHAR  |wszDROPVIEW|[]=L" DROP VIEW ")
const WCHAR  wszCREATE[]=L" CREATE ";					// @const WCHAR  |wszCREATE|[]=L" CREATE ")
const WCHAR  wszDROP[]=L" DROP ";						// @const WCHAR  |wszDROP|[]=L" DROP ")
const WCHAR  wszDELETE[]=L" DELETE ";					// @const WCHAR  |wszDELETE|[]=L" DELETE ")
const WCHAR  wszINDEX[]=L" INDEX ";						// @const WCHAR  |wszINDEX|[]=L" INDEX ")
const WCHAR  wszINSERTINTO[]=L" INSERT INTO ";			// @const WCHAR  |wszINSERTINTO|[]=L" INSERT INTO ")
const WCHAR  wszUPDATE[]=L" UPDATE ";					// @const WCHAR  |wszUPDATE|[]=L" UPDATE ")
const WCHAR  wszSELECT[]=L" SELECT ";					// @const WCHAR  |wszSELECT|[]=L" SELECT ")
const WCHAR  wszSELECT_STAR_FROM[]=L"SELECT * FROM ";	// @const
const WCHAR  wszEQUAL_TO_QUESTION_MARK[] = L" = ? ";	// @const
const WCHAR  wszLIKE_QUESTION_MARK[] = L" LIKE ? ";		// @const
const WCHAR  wszNullTerminator[] = L"";
const WCHAR  wszQuestionMarkComma[] = L"?,";

// SQL Statments
const WCHAR  wszSELECT_INVALIDTBLNAME[]=L"SELECT * FROM Xxxxx";											// @const
const WCHAR  wszSELECT_VALIDATIONORDER[]=L"SELECT * FROM %s Order By %s";													// @const
const WCHAR  wszSELECT_ALLFROMTBL[]=L"SELECT * FROM %s";													// @const
const WCHAR  wszSELECT_1ROW[]=L"Select %s from %s where %s";
const WCHAR  wszSELECT_CROSSPRODUCT[]=L"Select * from %s, %s";												// @const
const WCHAR  wszSELECT_ABCANDCOLLIST[]=L"Select 'ABC', %s from %s";											// @const
const WCHAR  wszSELECT_DISTINCTCOLLISTORDERBY[]=L"Select DISTINCT %s from %s order by %s DESC";				// @const
const WCHAR  wszSELECT_DISTINCTCOLLIST[]=L"Select DISTINCT %s from %s";										// @const
const WCHAR  wszSELECT_REVCOLLIST[]=L"Select %s from %s";													// @const
const WCHAR  wszSELECT_COLLISTGROUPBY[]=L"Select %s from %s GROUP BY %s HAVING %s is not null";				// @const
const WCHAR  wszSELECT_INVALIDGROUPBY[]=L"Select * from %s GROUP BY %s";									// @const
const WCHAR  wszSELECT_COLLISTWHERELASTCOLINSELECT[]=L"Select %s from %s where %s in (Select %s from %s)";	// @const
const WCHAR  wszCREATE_VIEW[]=L"Create view %s as Select %s from %s";										// @const
const WCHAR  wszSELECT_REVCOLLISTFROMVIEW[]=L"Select %s from %s";											// @const
const WCHAR  wszSELECT_COUNT[]=L"Select count(%s) from %s";													// @const
const WCHAR  wszSELECT_COLLISTSELECTREVCOLLIST[]=L"Select %s from %s; Select %s from %s";					// @const
const WCHAR  wszSELECT_EMPTYROWSET[]=L"Select %s from %s where 0=1";										// @const
const WCHAR  wszSELECT_COLLISTFROMTBL[]=L"Select %s from %s";												// @const
const WCHAR  wszSELECT_COLLISTFROMTBLWHERE[]=L"Select %s from %s where %s";									// @const
const WCHAR  wszSELECT_COLLISTTBLUNIONTBL[]=L"Select %s from %s UNION select %s from %s";					// @const
const WCHAR  wszSELECT_COLLISTORDERBYCOLONECOMPUTE[]=L"Select %s from %s ORDER BY %s COMPUTE SUM(%s)";		// @const
const WCHAR  wszSELECT_COLLISTORDERBYCOLONECOMPUTEBY[]=L"Select %s from %s ORDER BY %s COMPUTE SUM(%s) by %s";// @const
const WCHAR  wszSELECT_LEFTOUTERJOIN[]=L"Select * from %s LEFT OUTER JOIN %s on %s.%s = %s.%s";				// @const
const WCHAR  wszSELECT_RIGHTOUTERJOIN[]=L"Select * from %s RIGHT OUTER JOIN %s on %s.%s = %s.%s";			// @const
const WCHAR  wszSELECT_LEFTOUTERJOIN_ESC[]=L"Select * from {oj %s LEFT OUTER JOIN %s on %s.%s = %s.%s}";				// @const
const WCHAR  wszSELECT_RIGHTOUTERJOIN_ESC[]=L"Select * from {oj %s RIGHT OUTER JOIN %s on %s.%s = %s.%s}";			// @const
const WCHAR	 wszSELECT_DUPLICATECOLUMNS[]=L"Select %s,%s from %s";
const WCHAR	 wszSELECT_COMPUTEDCOLLIST[] = L"Select %s, %s-%s from %s";
const WCHAR	 wszSELECT_ORDERBYNUMERIC[] = L"Select * from %s Order By %s";
const WCHAR  wszSELECT_UPDATEABLEALLROWS[] = L"Select %s from %s";
const WCHAR  wszINSERT_ALLWITHPARAMS[] = L"Insert into %s (%s) values (%s)";
const WCHAR  wszINSERT_INVALID_KEYWORD[] = L"Insert inot %s (%s) values (%s)";
const WCHAR  wszCREATE_TABLE[]=L"Create table %s (%s)";
const WCHAR  wszCREATE_INDEX[]=L"Create %s index %s on %s (%s)";
const WCHAR  wszDELETE_1ROW[]=L"Delete from %s where %s";
const WCHAR  wszDELETE_ALLROWS[]=L"Delete from %s";
const WCHAR  wszDROP_INDEX[]=L"Drop Index %s.%s";
const WCHAR  wszDROP_TABLE[]=L"Drop Table %s";
const WCHAR  wszDROP_VIEW[]=L"Drop View %s";
const WCHAR  wszALTER_TABLE_DROP_COLUMN[]=L"Alter Table %s Drop Column %s";
const WCHAR  wszALTER_TABLE_ADD[]=L"Alter Table %s add %s";
const WCHAR  wszALTER_TABLE_ADD_EX[]=L"Alter Table %s add %s %s";
const WCHAR  wszINSERT_1ROW[]=L"Insert into %s (%s) values (%s)";
const WCHAR  wszUPDATE_1ROW[]=L"Update %s set %s where %s";
const WCHAR  wszEQUALDATA[]=L"=%s";
const WCHAR  wszORDERBY[]=L" order by %s DESC ";
const WCHAR  wszSELECT_ALL_WITH_FOR_BROWSE[]=L"SELECT * FROM %s FOR BROWSE";
const WCHAR  wszSELECT_ALL_WITH_FOR_UPDATE[]=L"SELECT * FROM %s FOR UPDATE";
const WCHAR  wszCREATE_PROC[]=L"Create procedure %s as Select %s from %s";									// @const
const WCHAR  wszCREATE_PROC_TEMPLATE1[]=L"Create procedure %s(%s) as begin %s end";									// @const
const WCHAR  wszDROP_PROC[]=L"Drop procedure %s";
const WCHAR	 wszEXEC_PROC[]=L"{call %s}";
const WCHAR  wszCHANGE_CURRENT_CATALOG[]=L"use %s";
//CParseStrm constants
const CHAR	 szCOLUMN[] = "[COLUMN]";
const CHAR	 szDATA[] = "[DATA]";
const LONG	 UNKNOWN = -1;

const WCHAR  g_wszConfProvPrefix[]=L"confprov://dso/session";

//International strings
const WCHAR	 wszKASHIDA[2] = { 0x0640, 0};	// Arabic text justification character {SHIFT+J}

#endif // _PRIVCNST_H_
