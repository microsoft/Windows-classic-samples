//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1995-2000 Microsoft Corporation.
//
// @doc 
//
// @module IIndexDefinition.CPP
//

#define  DBINITCONSTANTS	// Must be defined to initialize constants in OLEDB.H
#define  INITGUID

#include "modstandard.hpp"
#include "IIndxDef.h"
#include "Extralib.h"

class TCIIndexDefinition;

#include <process.h>

#ifndef DB_E_BADINDEXID
#define	DB_E_BADINDEXID	DB_E_BADID
#endif

extern IDBInitialize	*g_pIDBInitialize;
static BOOL fProperRangeForFillFactor = TRUE;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Module Values
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// {{ TCW_MODULE_GLOBALS
DECLARE_MODULE_CLSID = { 0x406ceeb, 0x3b9d, 0x11d2, { 0xb0, 0x25, 0x0, 0xc0, 0x4f, 0xc2, 0x27, 0x93 } };
DECLARE_MODULE_NAME("IIndexDefinition");
DECLARE_MODULE_OWNER("Microsoft");
DECLARE_MODULE_DESCRIP("Testcase for IIndexDefinition");
DECLARE_MODULE_VERSION(795921705);
// TCW_WizardVersion(2)
// TCW_Automation(True)
// }} TCW_MODULE_GLOBALS_END
GUID	guidModule	= { 0x406ceeb, 0x3b9d, 0x11d2, { 0xb0, 0x25, 0x0, 0xc0, 0x4f, 0xc2, 0x27, 0x93 } };
#define FILL_PROP_SET(RGPROPSET_EL, NPROP, RGPROP, PROP_GUID)		\
	RGPROPSET_EL.cProperties		= NPROP;						\
	RGPROPSET_EL.rgProperties		= RGPROP;						\
	RGPROPSET_EL.guidPropertySet	= PROP_GUID;					\
	memset(RGPROP, 0, NPROP*sizeof(DBPROP));

#define FILL_PROP(PROP, PROPID, VAR_TYPE, VAR_MACRO, VAR_VALUE, OPTION)		\
	PROP.dwPropertyID		= PROPID;										\
	PROP.vValue.vt			= VAR_TYPE;										\
	VAR_MACRO(&PROP.vValue)	= VAR_VALUE;									\
	PROP.dwOptions			= OPTION;

typedef struct inparam{
	ULONG				i;
	TCIIndexDefinition	*pObject;
} CInParam;

const ULONG		nThreads=5;
unsigned WINAPI ThreadProc(LPVOID lpvThreadParam);

//*-----------------------------------------------------------------------
// @class 
//
class TCIIndexDefinition : public CSessionObject{ 
private:
	HRESULT	GetLiteralInfo();
protected:
	// @cmember ptr to IIndexInterface
	CIIndexDefinition		*m_pCIIndexDefinition;
//	IIndexDefinition		*m_pIIndexDefinition;
	DBCOLUMNDESC			*m_rgColumnDesc;
	DBORDINAL				m_cColumnDesc;
	CTable					*m_pPopulatedTable;
	DBORDINAL				m_nIndex;
	ULONG					m_cIndexCols;	// the number of columns that support indexes
	DBORDINAL				*m_rgIndexCols;	// array of ordinals for columns that support indexes
	IDBSchemaRowset			*m_pIDBSchemaRowset;
	
	// info to decide whether opening indexes are supported through IOpenRowset::OpenRowset
//	BOOL					m_fIRowsetIndex;
	BOOL					m_fSupportORS;
	BOOL					m_fORSIndex;
	BOOL					m_fORSIntegratedIndex;

	CList<CCol, CCol&>		m_ColList;
	size_t					m_cMaxTableName, m_cMaxIndexName, m_cMaxColumnName;
	LPOLESTR				m_pwszInvalidTableChars;
	LPOLESTR				m_pwszInvalidIndexChars;
	LPOLESTR				m_pwszInvalidColumnChars;
	HRESULT					m_rgResult[nThreads];
	LPWSTR					m_rgIndexName[nThreads];

	// @cmember Display the index columns and orders
	BOOL	DisplayIndex(
		DBINDEXCOLUMNDESC	*rgIndexColumnDesc,	// columns and order in index
		DBORDINAL			nIndexSize						// index size
	);

	// Sets/Resets the nullibility of each column in the table.
	BOOL MakeColsNullable(
		CTable				*pTable,
		BOOL				bNullable
	);

	
/*	//--------------------------------------------------------------------------
	//
	//	@cmember Read the value of a column in the indexe schema rowset
	//
	//	RETURNS FALSE if the value was read and differ than the original one
	//--------------------------------------------------------------------------
	BOOL	GetIndexValue(
		LPVOID		pVariable,		// [OUT]	value read
		BOOL		*pfSet,			// [OUT]	if the value is set
		DBBINDING	*rgDBBINDING,	// [IN]		binding array
		ULONG		cColumn,		// [IN]		column to be read
		ULONG		ulDBTYPE,		// [IN]		type of property variant
		BYTE		*pData,			// [IN]		pointer to read DATA stru
		WCHAR		*lpwszMesaj		// [IN]		message text for error
	);
*/

	//--------------------------------------------------------------------------
	//
	// @cmember Check the result and property status
	//--------------------------------------------------------------------------
	BOOL	CheckProperty(const GUID guidPropSet, DBPROP *rgProp, HRESULT hr);


	//--------------------------------------------------------------------------
	//
	//	@cmember Read the value of a column in the indexe schema rowset
	//
	//	RETURNS FALSE if the value was read and differ than the original one
	//--------------------------------------------------------------------------
	BOOL GetIndexValueFromFirstRow(
		LPVOID		pVariable,		// [OUT]	value read
		BOOL		*pfSet,			// [OUT]	if the value is set
		DBBINDING	*rgDBBINDING,	// [IN]		binding array
		ULONG		cColumn,		// [IN]		column to be read
		ULONG		ulDBTYPE,		// [IN]		type of property variant
		BYTE		*pData,			// [IN]		pointer to read DATA stru
		DBPROPID	PropID,			// [IN]		property that is being read
		ULONG		cPropSets,		// [IN]		number of property sets
		DBPROPSET	*rgPropSets,	// [IN]		array of property sets
		WCHAR		*lpwszMesaj		// [IN]		message text for error
	);


	//-------------------------------------------------------------------------
	//
	// @cmember Set a property in the property sets of a DBCOLUMNDESC
	//-------------------------------------------------------------------------
	BOOL	SetProperty
	(
		DBPROPSET		**prgPropertySets,	// [in/out]		array of property sets
		ULONG			*pcPropertySets,	// [in/out]		number of property sets
		DBPROPID		propID,				// [in]			property ID
		DBTYPE			wType,				// [in]			value type
		LPVOID			value,				// [in]			property value
		DBPROPOPTIONS	dwOption,			// [in]			prop options
		DBID			colid,				// [in]			col id
		GUID			guidPropertySet,	// [in]			the GUID of the propset
		ULONG			*pcPropSet=NULL,	// [out]		index of property set where the prop was set
		ULONG			*pcProp=NULL		// [out]		index in the rgProperties array
	);

	// @cmember Build a valid table name of a certain length, from a certain pattern
	WCHAR	*BuildValidName(size_t length, WCHAR* pattern);

	// @cmember Build an invalid table name of a certain length
	// the pattern is supposed to be shorter than the string to be build
	WCHAR	*BuildInvalidName(size_t length, WCHAR* pattern, WCHAR* invchars);

	// @cmember Drops the index and releases its DBID
//	BOOL	DropIndexAndReleaseID(DBID *pTableID, DBID **ppIndexID);

	// @cmember Sets an index with 1 column
	// tries all the columns if necessary
	// table ID is given by m_pTable->m_TableID
//	HRESULT SetIndex(
//			DBID	*pIndexID,		// [in]  index ID
//			ULONG	*nIndex,		// [out] index column 
//			DBID	**ppIndexID		// [out] index ID created by IIndexDefinition::CreateIndex()
//		);

	// @cmember Sets an index with 1 column
	// tries all the columns if necessary, until succeeds
	HRESULT SetIndex(
			DBID	*pTableID,		// [in] table to which to attach the index
			DBID	*pIndexID,		// [in]  index ID
			ULONG	*nIndex,		// [out] index column 
			DBID	**ppIndexID		// [out] index ID created by IIndexDefinition::CreateIndex()
			);

/*	// @cmember Check whether all the columns specified appear in index
	// if a IRowsetIndex could be open on the index, it goes for CheckIndex2
	// otherwise tries to use IDBSchemaRowset
	BOOL	CheckIndex(
			DBID				*pTableID,				// the index of the table
			DBID				*pIndexDBID,			// the index to be checked
			DBORDINAL			cIndexColumnDesc,		// how many elements
			DBINDEXCOLUMNDESC	*rgIndexColumnDesc,		// array with index original descriprion
			ULONG				cPropertySets = 0,		// number of property sets
			DBPROPSET			*rgPropertySets = NULL	// the array of property sets
		);

	// @cmember Check whether all the columns specified appear in index
	// opens an IRowsetIndex and checks everything
	BOOL	CheckIndex2(
			DBID				*pTableID,				// the index of the table
			DBID				*pIndexDBID,			// the index to be checked
			DBORDINAL			cIndexColumnDesc,		// how many elements
			DBINDEXCOLUMNDESC	*rgIndexColumnDesc,		// array with index original descriprion
			ULONG				cPropertySets = 0,		// number of property sets
			DBPROPSET			*rgPropertySets = NULL	// the array of property sets
		);

	BOOL	CheckReturnedDBID(
			DBID	*pIndexID,		// passed to CreateIndex
			DBID	**ppIndexID		// returned by CreateIndex
		);
*/	
	// @cmember Check whether the property sets asked for rowset was preserved
	// all the data should be inside, the only thing that is allowed to be modified <nl>
	// is the property status
	BOOL IsPropSetPreserved
	(
		DBPROPSET	*rgInPropertySets,	// the array passed to ITableDefinition::CreateTable
		DBPROPSET	*rgOutPropertySets,	// the aray returned by ITableDefinition::CreateTable
		ULONG		cPropertySets			// the size of the arrays
	);

/*	//-------------------------------------------------------------------------
	//
	// @cmember create an index out of index column descriptors
	// 
	// RETURNS:
	//		E_FAIL	if there is a consistency error 
	//		hr of the IIndexDefinition::CreateIndex() otherwise
	//-------------------------------------------------------------------------
	HRESULT		CreateAndCheckIndex(
		DBID				*pTableID,				// [in]		the ID of the table
		DBID				*pIndexID,				// [in]		the ID of the index
		DBID				**ppIndexID,			// [in/out]	stores output ptr to IndexID
		DBORDINAL			nIndexSize,				// [in]		index size (no of columns)
		DBINDEXCOLUMNDESC	*rgIndexColumnDesc,		// [in]		columns to be part of index 
		ULONG				cPropSets=0,			// [in]		count of property sets 
		DBPROPSET			*rgPropSets=NULL		// [in]		array of property sets
	);

	//-------------------------------------------------------------------------
	//
	// @cmember create an index out of index column descriptors
	// Drops the index and returns just the result of the operation
	// 
	// RETURNS:
	//		E_FAIL	if there is a consistency error 
	//		hr of the IIndexDefinition::CreateIndex() otherwise
	//-------------------------------------------------------------------------
	HRESULT		CreateCheckAndDropIndex(
		DBID				*pTableID,				// [in]		the ID of the table
		DBID				*pIndexID,				// [in]		the ID of the index
		DBORDINAL			nIndexSize,				// [in]		index size (no of columns)
		DBINDEXCOLUMNDESC	*rgIndexColumnDesc,		// [in]		columns to be part of index 
		ULONG				cPropSets=0,			// [in]		count of property sets 
		DBPROPSET			*rgPropSets=NULL		// [in]		array of property sets
	);
*/
	//-------------------------------------------------------------------------
	//
	// @cmember create an index out of column ordinals
	//			Drops the index afterwards
	// 
	// 
	// RETURNS:
	//		E_FAIL	if there is a consistency error 
	//		S_OK	hr of the IIndexDefinition::CreateIndex() otherwise
	//-------------------------------------------------------------------------
	HRESULT		CCNDropIndexFromOrdinals(
		CTable				*pTable,				// [in]		the ID of the table
		DBID				*pIndexID,				// [in]		the ID of the index
		DBCOLUMNDESC		*rgColumnDesc,			// [in]		array of column descriptors
		DBORDINAL			cColumnDesc,			// [in]		number of column desc
		DBORDINAL			nIndexSize,				// [in]		index size (no of columns)
		DBORDINAL			*rgIndexColumns,		// [in]		columns to be part of index 
		DBINDEX_COL_ORDER	*rgOrder,				// [in]		order on index columns
		ULONG				cPropSets=0,			// [in]		count of property sets 
		DBPROPSET			*rgPropSets=NULL		// [in]		array of property sets
	);

	//-------------------------------------------------------------------------
	//
	// @cmember create an index out of column ordinals
	//	returns the ID of the created index 
	// 
	// 
	// RETURNS:
	//		E_FAIL	if there is a consistency error 
	//		S_OK	if function completes ok
	//-------------------------------------------------------------------------
	HRESULT		CNCIndexFromOrdinals(
		CTable				*pTable,				// [in]		the ID of the table
		DBID				*pIndexID,				// [in]		the ID of the index
		DBID				**ppIndexID,			// [in/out]	index ID of the created index
		DBCOLUMNDESC		*rgColumnDesc,			// [in]		array of column descriptors
		DBORDINAL			cColumnDesc,			// [in]		number of column desc
		DBORDINAL			nIndexSize,				// [in]		index size (no of columns)
		DBORDINAL			*rgIndexColumns,		// [in]		columns to be part of index 
		DBINDEX_COL_ORDER	*rgOrder,				// [in]		order on index columns
		ULONG				cPropSets=0,			// [in]		count of property sets 
		DBPROPSET			*rgPropSets=NULL		// [in]		array of property sets
	);

	// @cmember create an index out of column numbers
	// creates, checks it and then drops it
	// index size == 1
	BOOL	CreateIndexWithOneProperty(
				ULONG		nValues,						// [in] elements in the array
				VARIANT		*rgValues,						// [in] the values to be checked
				DBPROPID	dwPropID,						// [in] prop id
				GUID		guidPropertySet=DBPROPSET_INDEX	// [in]
			);

	// @cmember create an index out of column numbers
	// creates, checks it and then drops it
	// index size == 1
	BOOL	CreateIndexWithOneLogicalProperty(
				DBPROPID	dwPropID,						// [in] prop id
				GUID		guidPropertySet=DBPROPSET_INDEX	// [in]
			);

	// @cmember Iterator on indexes (as col no)
	int	GetNextIndex(
			DBORDINAL	*rgIndexColumns,	// array containing the column numbers of the index
			BOOL		*rgColumnPresent,	// array of flags for membership to the index
			DBORDINAL	nIndexsize,			// index size
			DBORDINAL	nCols				// number of columns
		);

	// @cmember Iterator on order (ascending/descending)
	int	GetNextOrder(
			DBINDEX_COL_ORDER	*rgIndexColumns,	// array containing the column numbers of the index
			DBORDINAL			k					// index size
		);

	// @cmember Builds and tries all indexes on a table
	HRESULT	DoAllIndexes(
			CTable		*pTable,			// pointer to a CTable
			DBORDINAL	nCols				// number of columns in the table
		);

	// @cmember Builds and tries all indexes on a table
	HRESULT	DoAllIndexes(
			CTable	*pTable,					// pointer to a CTable
			DBORDINAL	nCols,					// number of columns in the table
			DBORDINAL	nMaxIndexSize			// maximum index size
		);

	// @cmember Builds and tries all indexes of a given size defined upon a table
	HRESULT	DoAllKIndexes(
			CTable	*pTable,					// pointer to a CTable
			DBORDINAL	nIndexSize,				// index size
			DBORDINAL	nCols					// number of columns in the table
		);

	// @cmember Find the list of the indexable columns
	BOOL	FindIndexableColumns(
		DBCOLUMNDESC	*rgColumnDesc,		// [IN]		array of column desc on the new table
		DBORDINAL		cColumnDesc,		// [IN]		number of column desc
		DBORDINAL		**rgIndexCols,		// [OUT]	array of indexable columns
		DBORDINAL		*cIndexCols			// [OUT]	number of indexable columns
	);

	//-------------------------------------------------------------------------
	//
	// @cmember Prepare DBINDEXCOLUMNDESC array 
	// RETURN
	//	TRUE	- function executed ok and memory was properly allocated
	//	FALSE	- otherwise
	//-------------------------------------------------------------------------
	BOOL	MakeIndexColumnDesc(
		DBCOLUMNDESC		*rgColumnDesc,		// [IN]		the array of column desc
		DBORDINAL			cColumnDesc,		// [IN]		the number of columns in the table
		DBORDINAL			*rgIndexCol,		// [IN]		the array of the column in the index
		DBINDEX_COL_ORDER	*rgOrder,			// [IN]		the array of column order
		DBORDINAL			nIndexSize,			// [IN]		index size
		DBINDEXCOLUMNDESC	**rgIndexColumnDesc	// [OUT]	the desc of the index cols
	);

	//-------------------------------------------------------------------------
	//
	// @cmember Releases the memomry allocated for the structure
	// RETURN:
	//	TRUE	- everything OK
	//	FALSE	- bad argument
	//-------------------------------------------------------------------------
	BOOL	ReleaseIndexColumnDesc(
		DBINDEXCOLUMNDESC	*rgIndexColumnDesc,	// [IN]	the desc of the index cols
		DBORDINAL			nIndexSize			// [IN]		index size
	);

	//-------------------------------------------------------------------------
	//
	// @cmember release the property of all columns
	// RETURNS:
	//	TRUE	- function executed correctly
	//	FALSE	- parameters look strange
	//-------------------------------------------------------------------------
	BOOL	ReleaseAllColumnPropSets(
		DBORDINAL		cColumnDesc,		// [in]	number of columns
		DBCOLUMNDESC	*rgColumnDesc		// [in]	column descriptor array
	);

public:

	TCIIndexDefinition(WCHAR *wstrTestCaseName);
	virtual ~TCIIndexDefinition() {;}
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

/*	//---------------------------------------------------------------------------
	// TCIIndexDefinition::DoesIndexExist  	
	//
	// TCIIndexDefinition			|
	// DoesIndexExist	|
	// If this index is on this table return true If function runs correctly
	// but doesn't find the table name, function will return S_OK, but fExists
	// will be FALSE. If strIndexName is empty, returns E_FAIL.	
	//
	// @mfunc	DoesIndexExist
	// @rdesc HRESULT indicating success or failure
	//  @flag S_OK   | Function ran without problem
	//  @flag E_FAIL    | Function ran with problems
	//
	//---------------------------------------------------------------------------
	HRESULT		DoesIndexExist(	
		DBID		*pTableID,				// @parm [IN]	Table ID										  
		DBID		*pIndexID,				// @parm [IN]	Index ID
		BOOL 		*fExists,				// @parm [OUT] TRUE if index exists
		BOOL		fAnyIndex = FALSE		// @parm [IN]   TRUE if search for all indexes
	);


	//---------------------------------------------------------------------------
	// TCIIndexDefinition::DoesIndexExistInIndexSchemaRowset
	//
	// TCIIndexDefinition			|
	// DoesIndexExist	|
	// If this index is on this table return true. If function runs correctly
	// but doesn't find the table name, function will return S_OK, but fExists
	// will be FALSE. If strIndexName is empty, returns E_FAIL.	The index is sought in 
	// index schema rowset.
	//
	// @mfunc	DoesIndexExist
	// @rdesc HRESULT indicating success or failure
	//  @flag S_OK   | Function ran without problem
	//  @flag E_FAIL    | Function ran with problems
	//
	//---------------------------------------------------------------------------
	HRESULT		DoesIndexExistInIndexSchemaRowset(	
		DBID		*pTableID,				// @parm [IN]	Table ID										  
		DBID		*pIndexID,				// @parm [IN]	Index ID
		BOOL 		*fExists,				// @parm [OUT]	TRUE if index exists
		BOOL		fAnyIndex = FALSE		// @parm [IN]   TRUE if search for all indexes
	);


	//---------------------------------------------------------------------------
	// TCIIndexDefinition::DoesIndexExistRowsetIndex
	//
	// TCIIndexDefinition			|
	// DoesIndexExist	|
	// If this index is on this table return true. If function runs correctly
	// but doesn't find the table name, function will return S_OK, but fExists
	// will be FALSE. If strIndexName is empty, returns E_FAIL.	The index is sought in 
	// index schema rowset.
	//
	// @mfunc	DoesIndexExist
	// @rdesc HRESULT indicating success or failure
	//  @flag S_OK   | Function ran without problem
	//  @flag E_FAIL    | Function ran with problems
	//
	//---------------------------------------------------------------------------
	HRESULT DoesIndexExistRowsetIndex(	
		DBID		*pTableID,				// @parm [IN]	Table ID										  
		DBID		*IndexID,				// @parm [IN]	Index ID
		BOOL 		*fExists				// @parm [OUT]	TRUE if index exists
	);
*/
	virtual unsigned MyThreadProc(ULONG i) {return i;}
} ;




// {{ TCW_TEST_CASE_MAP(TCCreateIndex)
//*-----------------------------------------------------------------------
// @class tests IIndexDefinition::CreateIndex() method
//
class TCCreateIndex : public TCIIndexDefinition { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCCreateIndex,TCIIndexDefinition);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	unsigned MyThreadProc(ULONG);
	friend unsigned WINAPI ThreadProc(LPVOID lpvThreadParam);
	
	// {{ TCW_TESTVARS()
	// @cmember Check the session object
	int Variation_1();
	// @cmember create an index on a a non-empty table => S_OK
	int Variation_2();
	// @cmember create an index on an empty table => S_OK
	int Variation_3();
	// @cmember create an index on a table created with a SQL command => S_OK
	int Variation_4();
	// @cmember create an index and get an IRowsetIndex interface on it => S_OK
	int Variation_5();
	// @cmember create an index on a table generated with ITableDefinition => S_OK
	int Variation_6();
	// @cmember create an index on a table that has a rowset open on it => S_OK
	int Variation_7();
	// @cmember create an index on a table using column names and colID's read from IColumnsInfo
	int Variation_8();
	// @cmember include a column twice in an index
	int Variation_9();
	// @cmember create an index on an autoincrement column => S_OK
	int Variation_10();
	// @cmember create an index on a column, drop the column
	int Variation_11();
	// @cmember create an index on a view
	int Variation_12();
	// @cmember create multiple indexes on the same table
	int Variation_13();
	// @cmember create an index on a table, drop the table and check if the index is dropped as well)
	int Variation_14();
	// @cmember create an index on a table; drop a column from index and readd it with another type
	int Variation_15();
	// @cmember NULL pointer for pTableID => E_INVALIDARG
	int Variation_16();
	// @cmember *pTableID == DB_NULLID => DB_E_NOTABLE
	int Variation_17();
	// @cmember pTableID->uName.pwszName is NULL => DB_E_NOTABLE
	int Variation_18();
	// @cmember pTableID->uName.pwszName is empty => DB_E_NOTABLE
	int Variation_19();
	// @cmember table name is invalid => DB_E_NOTABLE
	int Variation_20();
	// @cmember table name has maximum length => S_OK
	int Variation_21();
	// @cmember table name exceeds maximum table name => DB_E_NOTABLE
	int Variation_22();
	// @cmember NULL pointer for pIndexID and not NULL ppIndexID => S_OK
	int Variation_23();
	// @cmember not NULL pointer for pIndexID and NULL ppIndexID =>S_OK
	int Variation_24();
	// @cmember NULL pointer for pIndexID and NULL ppIndexID => E_INVALIDARG
	int Variation_25();
	// @cmember *pIndexID == DB_NULLID => DB_E_BADINDEXID
	int Variation_26();
	// @cmember pIndexID->uName.pwszName is NULL => DB_E_BADINDEXID
	int Variation_27();
	// @cmember pIndexID->uName.pwszName is empty => DB_E_BADINDEXID
	int Variation_28();
	// @cmember index name is invalid => DB_E_BADINDEXID
	int Variation_29();
	// @cmember index name has maximum length => S_OK
	int Variation_30();
	// @cmember index name exceeds maximum length name => DB_E_BADINDEXID
	int Variation_31();
	// @cmember cIndexColumnDesc == 0 => E_INVALIDARG
	int Variation_32();
	// @cmember rgIndexColumnDesc == NULL => E_INVALIDARG
	int Variation_33();
	// @cmember cPropSets != 0 and rgPropSets == NULL => E_INVALIDARG
	int Variation_34();
	// @cmember cPropSets == 0 and rgPropSets == NULL =>S_OK
	int Variation_35();
	// @cmember cProperties == 0 and rgProperties != NULL in a rgPropertySets element => S_OK
	int Variation_36();
	// @cmember cProperties == 0 and rgProperties == NULL in an rgPropertySets element => S_OK
	int Variation_37();
	// @cmember cProperties != 0 and rgProperties == NULL in an element of rgPropertySets => E_INVALIDARG
	int Variation_38();
	// @cmember eIndexColumnDesc invalid for at least one column => E_INVALIDARG
	int Variation_39();
	// @cmember the index is made of all columns
	int Variation_40();
	// @cmember Try to create a duplicate index => DB_E_DUPLICATEINDEX
	int Variation_41();
	// @cmember Create an index on a single column table
	int Variation_42();
	// @cmember all indexes on a 2 columns table
	int Variation_43();
	// @cmember pIndexID and ppIndexID both not NULL => S_OK
	int Variation_44();
	// @cmember valid, yet inexistent table name => DB_E_NOTABLE
	int Variation_45();
	// @cmember inexistent column name => DB_E_NOCOLUMN
	int Variation_46();
	// @cmember all DBKIND on pTableID
	int Variation_47();
	// @cmember all DBKIND on pColumnID
	int Variation_48();
	// @cmember all DBKIND on pIndexID
	int Variation_49();
	// @cmember NULL pColumnID in an element of rgIndexColumnDesc => DB_E_NOCOLUMN
	int Variation_50();
	// @cmember DB_NULLID pColumnID in an element of rgIndexColumnDesc => DB_E_NOCOLUMN
	int Variation_51();
	// @cmember empty name for a column in an element of rgIndexColumnDesc => DB_E_NOCOLUMN
	int Variation_52();
	// @cmember NULL column name in an element of rgIndexColumnDesc => DB_E_NOCOLUMN
	int Variation_53();
	// @cmember Set DBPROP_INDEX_AUTOUPDATE
	int Variation_54();
	// @cmember Set DBPROP_INDEX_CLUSTERED
	int Variation_55();
	// @cmember Set DBPROP_INDEX_NULLS
	int Variation_56();
	// @cmember Set DBPROP_INDEX_PRIMARYKEY
	int Variation_57();
	// @cmember Set DBPROP_INDEX_SORTBOOKMARKS
	int Variation_58();
	// @cmember Set DBPROP_INDEX_TEMPINDEX
	int Variation_59();
	// @cmember Set DBPROP_INDEX_TYPE
	int Variation_60();
	// @cmember Set DBPROP_INDEX_UNIQUE
	int Variation_61();
	// @cmember Set DBPROP_INDEX_FILLFACTOR
	int Variation_62();
	// @cmember Set DBPROP_INDEX_INITIALSIZE
	int Variation_63();
	// @cmember Set DBPROP_INDEX_NULLCOLLATION
	int Variation_64();
	// @cmember Set DBPROP_INDEX_SORTBOOKMARKS and DBPROP_INDEX_UNIQUE
	int Variation_65();
	// @cmember Set DBPROP_INDEX_UNIQUE and DBPROP_INDEX_PRIMARYKEY
	int Variation_66();
	// @cmember Set DBPROP_INDEX_UNIQUE and DBPROP_INDEX_NULLS
	int Variation_67();
	// @cmember CONFLICTING properties (set both DBPROP_INDEX_NULLS and DBPROP_INDEX_PRIMARYKEY)
	int Variation_68();
	// @cmember Non index dwPropertyID
	int Variation_69();
	// @cmember Non index guidPropertySet
	int Variation_70();
	// @cmember invalid value type for property
	int Variation_71();
	// @cmember invalid value for a property
	int Variation_72();
	// @cmember specifying a property twice
	int Variation_73();
	// @cmember all the properties
	int Variation_74();
	// @cmember column name is invalid => DB_E_NOCOLUMN
	int Variation_75();
	// @cmember maximum length column name => S_OK
	int Variation_76();
	// @cmember too long column name => DB_E_NOCOLUMN
	int Variation_77();
	// @cmember multithreading
	int Variation_78();
	// @cmember 2 threads, the same index
	int Variation_79();
	// @cmember Primary and clustered index properties
	int Variation_80();
	// @cmember bogus colid in a non column specific index property
	int Variation_81();
	// @cmember DB_NULLID passed as colid in a column specific index property
	int Variation_82();
	// @cmember valid, non DB_NULLID  colid in a column specific index property
	int Variation_83();
	// @cmember bogus colid in a column specific index property => error
	int Variation_84();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(TCCreateIndex)
#define THE_CLASS TCCreateIndex
BEG_TEST_CASE(TCCreateIndex, TCIIndexDefinition, L"tests IIndexDefinition::CreateIndex() method")
	TEST_VARIATION(1, 		L"Check the session object")
	TEST_VARIATION(2, 		L"create an index on a a non-empty table => S_OK")
	TEST_VARIATION(3, 		L"create an index on an empty table => S_OK")
	TEST_VARIATION(4, 		L"create an index on a table created with a SQL command => S_OK")
	TEST_VARIATION(5, 		L"create an index and get an IRowsetIndex interface on it => S_OK")
	TEST_VARIATION(6, 		L"create an index on a table generated with ITableDefinition => S_OK")
	TEST_VARIATION(7, 		L"create an index on a table that has a rowset open on it => S_OK")
	TEST_VARIATION(8, 		L"create an index on a table using column names and colID's read from IColumnsInfo")
	TEST_VARIATION(9, 		L"include a column twice in an index")
	TEST_VARIATION(10, 		L"create an index on an autoincrement column => S_OK")
	TEST_VARIATION(11, 		L"create an index on a column, drop the column")
	TEST_VARIATION(12, 		L"create an index on a view")
	TEST_VARIATION(13, 		L"create multiple indexes on the same table")
	TEST_VARIATION(14, 		L"create an index on a table, drop the table and check if the index is dropped as well)")
	TEST_VARIATION(15, 		L"create an index on a table; drop a column from index and readd it with another type")
	TEST_VARIATION(16, 		L"NULL pointer for pTableID => E_INVALIDARG")
	TEST_VARIATION(17, 		L"*pTableID == DB_NULLID => DB_E_NOTABLE")
	TEST_VARIATION(18, 		L"pTableID->uName.pwszName is NULL => DB_E_NOTABLE")
	TEST_VARIATION(19, 		L"pTableID->uName.pwszName is empty => DB_E_NOTABLE")
	TEST_VARIATION(20, 		L"table name is invalid => DB_E_NOTABLE")
	TEST_VARIATION(21, 		L"table name has maximum length => S_OK")
	TEST_VARIATION(22, 		L"table name exceeds maximum table name => DB_E_NOTABLE")
	TEST_VARIATION(23, 		L"NULL pointer for pIndexID and not NULL ppIndexID => S_OK")
	TEST_VARIATION(24, 		L"not NULL pointer for pIndexID and NULL ppIndexID =>S_OK")
	TEST_VARIATION(25, 		L"NULL pointer for pIndexID and NULL ppIndexID => E_INVALIDARG")
	TEST_VARIATION(26, 		L"*pIndexID == DB_NULLID => DB_E_BADINDEXID")
	TEST_VARIATION(27, 		L"pIndexID->uName.pwszName is NULL => DB_E_BADINDEXID")
	TEST_VARIATION(28, 		L"pIndexID->uName.pwszName is empty => DB_E_BADINDEXID")
	TEST_VARIATION(29, 		L"index name is invalid => DB_E_BADINDEXID")
	TEST_VARIATION(30, 		L"index name has maximum length => S_OK")
	TEST_VARIATION(31, 		L"index name exceeds maximum length name => DB_E_BADINDEXID")
	TEST_VARIATION(32, 		L"cIndexColumnDesc == 0 => E_INVALIDARG")
	TEST_VARIATION(33, 		L"rgIndexColumnDesc == NULL => E_INVALIDARG")
	TEST_VARIATION(34, 		L"cPropSets != 0 and rgPropSets == NULL => E_INVALIDARG")
	TEST_VARIATION(35, 		L"cPropSets == 0 and rgPropSets == NULL =>S_OK")
	TEST_VARIATION(36, 		L"cProperties == 0 and rgProperties != NULL in a rgPropertySets element => S_OK")
	TEST_VARIATION(37, 		L"cProperties == 0 and rgProperties == NULL in an rgPropertySets element => S_OK")
	TEST_VARIATION(38, 		L"cProperties != 0 and rgProperties == NULL in an element of rgPropertySets => E_INVALIDARG")
	TEST_VARIATION(39, 		L"eIndexColumnDesc invalid for at least one column => E_INVALIDARG")
	TEST_VARIATION(40, 		L"the index is made of all columns")
	TEST_VARIATION(41, 		L"Try to create a duplicate index => DB_E_DUPLICATEINDEX")
	TEST_VARIATION(42, 		L"Create an index on a single column table")
	TEST_VARIATION(43, 		L"all indexes on a 2 columns table")
	TEST_VARIATION(44, 		L"pIndexID and ppIndexID both not NULL => S_OK")
	TEST_VARIATION(45, 		L"valid, yet inexistent table name => DB_E_NOTABLE")
	TEST_VARIATION(46, 		L"inexistent column name => DB_E_NOCOLUMN")
	TEST_VARIATION(47, 		L"all DBKIND on pTableID")
	TEST_VARIATION(48, 		L"all DBKIND on pColumnID")
	TEST_VARIATION(49, 		L"all DBKIND on pIndexID")
	TEST_VARIATION(50, 		L"NULL pColumnID in an element of rgIndexColumnDesc => DB_E_NOCOLUMN")
	TEST_VARIATION(51, 		L"DB_NULLID pColumnID in an element of rgIndexColumnDesc => DB_E_NOCOLUMN")
	TEST_VARIATION(52, 		L"empty name for a column in an element of rgIndexColumnDesc => DB_E_NOCOLUMN")
	TEST_VARIATION(53, 		L"NULL column name in an element of rgIndexColumnDesc => DB_E_NOCOLUMN")
	TEST_VARIATION(54, 		L"Set DBPROP_INDEX_AUTOUPDATE")
	TEST_VARIATION(55, 		L"Set DBPROP_INDEX_CLUSTERED")
	TEST_VARIATION(56, 		L"Set DBPROP_INDEX_NULLS")
	TEST_VARIATION(57, 		L"Set DBPROP_INDEX_PRIMARYKEY")
	TEST_VARIATION(58, 		L"Set DBPROP_INDEX_SORTBOOKMARKS")
	TEST_VARIATION(59, 		L"Set DBPROP_INDEX_TEMPINDEX")
	TEST_VARIATION(60, 		L"Set DBPROP_INDEX_TYPE")
	TEST_VARIATION(61, 		L"Set DBPROP_INDEX_UNIQUE")
	TEST_VARIATION(62, 		L"Set DBPROP_INDEX_FILLFACTOR")
	TEST_VARIATION(63, 		L"Set DBPROP_INDEX_INITIALSIZE")
	TEST_VARIATION(64, 		L"Set DBPROP_INDEX_NULLCOLLATION")
	TEST_VARIATION(65, 		L"Set DBPROP_INDEX_SORTBOOKMARKS and DBPROP_INDEX_UNIQUE")
	TEST_VARIATION(66, 		L"Set DBPROP_INDEX_UNIQUE and DBPROP_INDEX_PRIMARYKEY")
	TEST_VARIATION(67, 		L"Set DBPROP_INDEX_UNIQUE and DBPROP_INDEX_NULLS")
	TEST_VARIATION(68, 		L"CONFLICTING properties (set both DBPROP_INDEX_NULLS and DBPROP_INDEX_PRIMARYKEY)")
	TEST_VARIATION(69, 		L"Non index dwPropertyID")
	TEST_VARIATION(70, 		L"Non index guidPropertySet")
	TEST_VARIATION(71, 		L"invalid value type for property")
	TEST_VARIATION(72, 		L"invalid value for a property")
	TEST_VARIATION(73, 		L"specifying a property twice")
	TEST_VARIATION(74, 		L"all the properties")
	TEST_VARIATION(75, 		L"column name is invalid => DB_E_NOCOLUMN")
	TEST_VARIATION(76, 		L"maximum length column name => S_OK")
	TEST_VARIATION(77, 		L"too long column name => DB_E_NOCOLUMN")
	TEST_VARIATION(78, 		L"multithreading")
	TEST_VARIATION(79, 		L"2 threads, the same index")
	TEST_VARIATION(80, 		L"Primary and clustered index properties")
	TEST_VARIATION(81, 		L"bogus colid in a non column specific index property")
	TEST_VARIATION(82, 		L"DB_NULLID passed as colid in a column specific index property")
	TEST_VARIATION(83, 		L"valid, non DB_NULLID  colid in a column specific index property")
	TEST_VARIATION(84, 		L"bogus colid in a column specific index property => error")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(TCDropIndex)
//*-----------------------------------------------------------------------
// @class tests IIndexDefinition::DropIndex() method
//
class TCDropIndex : public TCIIndexDefinition { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCDropIndex,TCIIndexDefinition);
	// }} TCW_DECLARE_FUNCS_END

	unsigned				MyThreadProc(ULONG);
	friend unsigned WINAPI	ThreadProc(LPVOID lpvThreadParam);	
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember NULL pointer for pTableID => E_INVALIDARG
	int Variation_1();
	// @cmember *pTableID == DB_NULLID => DB_E_NOTABLE
	int Variation_2();
	// @cmember pTableID->uName is NULL => DB_E_NOTABLE
	int Variation_3();
	// @cmember pTableID->uName is empty => DB_E_NOTABLE
	int Variation_4();
	// @cmember invalid table name => DB_E_NOTABLE
	int Variation_5();
	// @cmember maximum sized table name => S_OK
	int Variation_6();
	// @cmember table name exceeds maximum size => DB_E_NOTABLE
	int Variation_7();
	// @cmember NULL pointer for pIndexID => drop all indexes
	int Variation_8();
	// @cmember *pIndexID == DB_NULLID => DB_E_NOINDEX
	int Variation_9();
	// @cmember pIndexID->uName is NULL => DB_E_NOINDEX
	int Variation_10();
	// @cmember pIndexID->uName is empty => DB_E_NOINDEX
	int Variation_11();
	// @cmember invalid index name => DB_E_NOINDEX
	int Variation_12();
	// @cmember maximum size index name => S_OK
	int Variation_13();
	// @cmember oversized index name => DB_E_NOINDEX
	int Variation_14();
	// @cmember drop an index twice => DB_E_NOINDEX
	int Variation_15();
	// @cmember inexistent table => DB_E_NOTABLE
	int Variation_16();
	// @cmember All DBKIND for pTableID
	int Variation_17();
	// @cmember All DBKIND for pIndexID
	int Variation_18();
	// @cmember index in use => DB_E_INDEXINUSE
	int Variation_19();
	// @cmember multithreading
	int Variation_20();
	// @cmember 2 threads, the same index
	int Variation_21();
	// @cmember Inexistent index name
	int Variation_22();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(TCDropIndex)
#define THE_CLASS TCDropIndex
BEG_TEST_CASE(TCDropIndex, TCIIndexDefinition, L"tests IIndexDefinition::DropIndex() method")
	TEST_VARIATION(1, 		L"NULL pointer for pTableID => E_INVALIDARG")
	TEST_VARIATION(2, 		L"*pTableID == DB_NULLID => DB_E_NOTABLE")
	TEST_VARIATION(3, 		L"pTableID->uName is NULL => DB_E_NOTABLE")
	TEST_VARIATION(4, 		L"pTableID->uName is empty => DB_E_NOTABLE")
	TEST_VARIATION(5, 		L"invalid table name => DB_E_NOTABLE")
	TEST_VARIATION(6, 		L"maximum sized table name => S_OK")
	TEST_VARIATION(7, 		L"table name exceeds maximum size => DB_E_NOTABLE")
	TEST_VARIATION(8, 		L"NULL pointer for pIndexID => drop all indexes")
	TEST_VARIATION(9, 		L"*pIndexID == DB_NULLID => DB_E_NOINDEX")
	TEST_VARIATION(10, 		L"pIndexID->uName is NULL => DB_E_NOINDEX")
	TEST_VARIATION(11, 		L"pIndexID->uName is empty => DB_E_NOINDEX")
	TEST_VARIATION(12, 		L"invalid index name => DB_E_NOINDEX")
	TEST_VARIATION(13, 		L"maximum size index name => S_OK")
	TEST_VARIATION(14, 		L"oversized index name => DB_E_NOINDEX")
	TEST_VARIATION(15, 		L"drop an index twice => DB_E_NOINDEX")
	TEST_VARIATION(16, 		L"inexistent table => DB_E_NOTABLE")
	TEST_VARIATION(17, 		L"All DBKIND for pTableID")
	TEST_VARIATION(18, 		L"All DBKIND for pIndexID")
	TEST_VARIATION(19, 		L"index in use => DB_E_INDEXINUSE")
	TEST_VARIATION(20, 		L"multithreading")
	TEST_VARIATION(21, 		L"2 threads, the same index")
	TEST_VARIATION(22, 		L"Inexistent index name")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCTrans)
//*-----------------------------------------------------------------------
// @class transactions
//
class TCTrans : public TCIIndexDefinition { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

	// start the transaction
	HRESULT StartTransaction(ITransactionLocal*);
	
	VARIANT				m_vSupportedTxnDDL;
	DBINDEXCOLUMNDESC	m_rgIndexColumnDesc[1];

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCTrans,TCIIndexDefinition);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember Abort retaining on CreateIndex
	int Variation_1();
	// @cmember Abort non retaining on CreateIndex
	int Variation_2();
	// @cmember Commit retain on CreateIndex
	int Variation_3();
	// @cmember Commit non-retain on CreateIndex
	int Variation_4();
	// @cmember Abort retain on DropIndex
	int Variation_5();
	// @cmember Abort non-retain on DropIndex
	int Variation_6();
	// @cmember Commit retain on DropIndex
	int Variation_7();
	// @cmember Commit non retain on DropIndex
	int Variation_8();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(TCTrans)
#define THE_CLASS TCTrans
BEG_TEST_CASE(TCTrans, TCIIndexDefinition, L"transactions")
	TEST_VARIATION(1, 		L"Abort retaining on CreateIndex")
	TEST_VARIATION(2, 		L"Abort non retaining on CreateIndex")
	TEST_VARIATION(3, 		L"Commit retain on CreateIndex")
	TEST_VARIATION(4, 		L"Commit non-retain on CreateIndex")
	TEST_VARIATION(5, 		L"Abort retain on DropIndex")
	TEST_VARIATION(6, 		L"Abort non-retain on DropIndex")
	TEST_VARIATION(7, 		L"Commit retain on DropIndex")
	TEST_VARIATION(8, 		L"Commit non retain on DropIndex")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCRODataSource)
//*-----------------------------------------------------------------------
// @class read only data source
//
class TCRODataSource : public TCIIndexDefinition { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	IUnknown			*m_pIUnknown, *m_pIUnknown2;
	DBID				*m_pIndexID;
	IIndexDefinition	*m_pIIndexDefinition2;

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCRODataSource,TCIIndexDefinition);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember try to create an index on a read only datasource
	int Variation_1();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(TCRODataSource)
#define THE_CLASS TCRODataSource
BEG_TEST_CASE(TCRODataSource, TCIIndexDefinition, L"read only data source")
	TEST_VARIATION(1, 		L"try to create an index on a read only datasource")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


//--------------------------------------------------------------------
// @func Module level initialization routine
//
// @rdesc Success or Failure
// 		@flag  TRUE  | Successful initialization
//		@flag  FALSE | Initialization problems
//
BOOL ModuleInit(CThisTestModule * pThisTestModule)
{
	IIndexDefinition	*pIIndexDefinition = NULL;

	if (!pThisTestModule)
		return FALSE;

	//Create the session
	if(!ModuleCreateDBSession(pThisTestModule))
		return FALSE;
	
	if (!VerifyInterface(
				pThisTestModule->m_pIUnknown2, 
				IID_IIndexDefinition,  
				SESSION_INTERFACE, 
				(IUnknown**)&pIIndexDefinition))
	{
		odtLog << TEXT("IIndexDefinition is not supported; test module skipped!\n");
		return TEST_SKIPPED;
	}

	// external g_pIDBInitialize is to be used with Extralib
	if (!(VerifyInterface(pThisTestModule->m_pIUnknown, IID_IDBInitialize, 
				DATASOURCE_INTERFACE, (IUnknown**)&g_pIDBInitialize)))
	{
		odtLog << "IDBInitialize not supported!\n";
		return TEST_SKIPPED;
	}

	SAFE_RELEASE(pIIndexDefinition);
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
	SAFE_RELEASE(g_pIDBInitialize);
	return ModuleReleaseDBSession(pThisTestModule);
}	




//-------------------------------------------------------------------------
//
// @cmember Check whether the property sets asked for rowset was preserved
// all the data should be inside, the only thing that is allowed to be modified <nl>
// is the property status
//-------------------------------------------------------------------------
BOOL IsPropSetPreserved
(
	DBPROPSET	*rgInPropertySets,	// the array passed to IIndexDefinition::CreateIndex
	DBPROPSET	*rgOutPropertySets,	// the aray returned by IIndexDefinition::CreateIndex
	ULONG		cPropertySets			// the size of the arrays
)
{
	ULONG	ps, pc;
	HRESULT	hr;
	
	for (ps=0; ps<cPropertySets; ps++)
	{
		// compare the props in that prop set
		if (!CHECK(rgInPropertySets[ps].guidPropertySet == rgOutPropertySets[ps].guidPropertySet, TRUE))
			return FALSE;
		if (!CHECK(rgInPropertySets[ps].cProperties == rgOutPropertySets[ps].cProperties, TRUE))
			return FALSE;
		for (pc=0; pc<rgInPropertySets[ps].cProperties; pc++)
		{
			if (!CHECK(rgInPropertySets[ps].rgProperties[pc].dwPropertyID == 
					   rgOutPropertySets[ps].rgProperties[pc].dwPropertyID, TRUE))
				return FALSE;
			if (!CHECK(rgInPropertySets[ps].rgProperties[pc].dwOptions == 
					   rgOutPropertySets[ps].rgProperties[pc].dwOptions, TRUE))
				return FALSE;
			if (!CHECK(hr = CompareDBID(rgInPropertySets[ps].rgProperties[pc].colid, 
					   rgOutPropertySets[ps].rgProperties[pc].colid), TRUE))
				return FALSE;
			if (!CHECK(CompareVariant(&rgInPropertySets[ps].rgProperties[pc].vValue,
					   &rgOutPropertySets[ps].rgProperties[pc].vValue), TRUE))
				return FALSE;
		}
	}
	return TRUE;
} //IsPropSetPreserved


//-------------------------------------------------------------------------
//
// CheckReturnedDBID
//-------------------------------------------------------------------------
BOOL CheckReturnedDBID(
	DBID	*pIndexID,		// passed to CreateIndex
	DBID	**ppIndexID		// returned by CreateIndex
)
{
	if (!pIndexID)
		return *ppIndexID != NULL;

	if (!ppIndexID)
	{
		return TRUE;
	}

	if (*ppIndexID == NULL)
		return FALSE;
	
	return CompareDBID(*pIndexID, **ppIndexID);
} //CheckReturnedDBID



//--------------------------------------------------------------------------
//
// Check the result and property status
//--------------------------------------------------------------------------
BOOL CheckProperty(
	const GUID	guidPropertySet,// [in]	property set guid (should be DBPROPSET_INDEX
	DBPROP		*pProp,			// [in]	property
	HRESULT		hr				// [in] the result
)
{
	BOOL		fRes = TRUE;
	BOOL		fSupported;
	BOOL		fSettable;

	if (!pProp)
		return TRUE;
	
	// general checking
	if (DBPROPOPTIONS_REQUIRED == pProp->dwOptions && DBPROPSTATUS_OK != pProp->dwStatus)
		fRes = CHECK(FAILED(hr), TRUE);

	if (!(guidPropertySet == DBPROPSET_INDEX))
	{
		// check a couple of things and returns
		return COMPARE(pProp->dwStatus, DBPROPSTATUS_NOTSUPPORTED) && COMPARE(S_OK != hr, TRUE);
	}

	fSupported	= SupportedProperty(pProp->dwPropertyID, guidPropertySet);
	fSettable	= SettableProperty(pProp->dwPropertyID, guidPropertySet);

	if (!fSupported && !CHECK(S_OK != hr, TRUE))
		fRes = FALSE;
		
	// status driven checking
	switch (pProp->dwStatus)
	{
		case DBPROPSTATUS_OK:
			fRes = COMPARE(fSupported != 0, TRUE);
			break;
		case DBPROPSTATUS_NOTSUPPORTED:
			fRes = COMPARE(fSupported == 0, TRUE);
			break;
		case DBPROPSTATUS_NOTSETTABLE:
			fRes = COMPARE(fSupported != 0, TRUE) && COMPARE(fSettable == 0, TRUE);
			break;
		case DBPROPSTATUS_BADOPTION:
			fRes =	(pProp->dwOptions != DBPROPOPTIONS_OPTIONAL) 
				&&	COMPARE(DBPROPOPTIONS_REQUIRED != pProp->dwOptions, TRUE);
			break;
		case DBPROPSTATUS_CONFLICTING:
			fRes = COMPARE(fSettable != 0, TRUE);
			break;
		case DBPROPSTATUS_BADVALUE:
			fRes = COMPARE(fSupported != 0, TRUE);
			break;
		default:
			break;
	}

	// DBPROPSTATUS_BADVALUE can be certainly got only if type of the prop is wrong
	// info about supported discret values of the prop cannot generally be obtained 
	// before creating an index
	if (	GetPropInfoType(pProp->dwPropertyID, guidPropertySet) != pProp->vValue.vt
		&&	!COMPARE(DBPROPSTATUS_OK == pProp->dwStatus, FALSE))
	{
			odtLog << "ERROR: bad propstatus on improper type value on property" << pProp->dwPropertyID << "\n";
			fRes = FALSE;
	}

	return fRes;
} //CheckProperty




///////////////////////////////////////////////////////////////////////////////////
//
//
//				class CIIndexDefinition
//
//
///////////////////////////////////////////////////////////////////////////////////


CIIndexDefinition::CIIndexDefinition(IIndexDefinition *pIIndexDefinition)
{
	// interface passed is released in the destructor
	IGetDataSource	*pIGetDSO = NULL;

	ASSERT(pIIndexDefinition);
	m_pIIndexDefinition = pIIndexDefinition;
	
	if (COMPARE(VerifyInterface(m_pIIndexDefinition, IID_IGetDataSource, SESSION_INTERFACE, (IUnknown**)&pIGetDSO), TRUE))
	{
		CHECK(pIGetDSO->GetDataSource(IID_IUnknown, &m_pIDSOUnknown), S_OK);
	}

	SAFE_RELEASE(pIGetDSO);
} //CIIndexDefinition::CIIndexDefinition



CIIndexDefinition::CIIndexDefinition(IUnknown *pISessionUnknown)
{
	// interface passed is NOT released in the destructor; it should be released by user
	IGetDataSource	*pIGetDSO = NULL;

	ASSERT(pISessionUnknown);

	COMPARE(VerifyInterface(pISessionUnknown, IID_IIndexDefinition, SESSION_INTERFACE, (IUnknown**)&m_pIIndexDefinition), TRUE);
	
	if (COMPARE(VerifyInterface(pISessionUnknown, IID_IGetDataSource, SESSION_INTERFACE, (IUnknown**)&pIGetDSO), TRUE))
	{
		CHECK(pIGetDSO->GetDataSource(IID_IUnknown, &m_pIDSOUnknown), S_OK);
	}

	SAFE_RELEASE(pIGetDSO);
} //CIIndexDefinition::CIIndexDefinition



// basic methods for index manipulation
HRESULT CIIndexDefinition::CreateIndex(
	DBID				*pTableID,				// [in]		the ID of the table
	DBID				*pIndexID,				// [in]		the ID of the index
	DBORDINAL			nIndexSize,				// [in]		index size (no of columns)
	DBINDEXCOLUMNDESC	*rgIndexColumnDesc,		// [in]		columns to be part of index 
	ULONG				cPropSets,				// [in]		count of property sets 
	DBPROPSET			*rgPropSets,			// [in]		array of property sets
	DBID				**ppIndexID				// [in/out]	stores output ptr to IndexID
)
{
	HRESULT				hr;
	HRESULT				rgValidRes[] = {
							S_OK,
							DB_S_ERRORSOCCURRED,
							DB_E_TABLEINUSE,
							E_FAIL,
							E_INVALIDARG,
							DB_E_BADINDEXID,
							DB_E_DUPLICATEINDEXID,
							DB_E_ERRORSOCCURRED,
							DB_E_NOCOLUMN,
							DB_E_NOTABLE,
							DB_SEC_E_PERMISSIONDENIED,
						};
	ULONG				cValidRes = NUMELEM(rgValidRes);

	// create the index according the requirements
	hr = m_pIIndexDefinition->CreateIndex(	pTableID, pIndexID, nIndexSize, rgIndexColumnDesc, 
											cPropSets, rgPropSets, ppIndexID);

	COMPARE(CheckResult(hr, cValidRes, rgValidRes), TRUE);

	return hr;
} // CIIndexDefinition::CreateIndex



HRESULT CIIndexDefinition::DropIndex(
   DBID					*pTableID,				// [in]		the ID of the table
   DBID					*pIndexID				// [in]		the ID of the index
)
{
	HRESULT	hr				= E_FAIL;
	HRESULT	rgValidRes[]	= {
								S_OK,
								DB_S_ERRORSOCCURRED,
								E_FAIL,
								E_INVALIDARG,
								DB_E_ERRORSOCCURRED,
								DB_E_INDEXINUSE,
								DB_E_NOINDEX,
								DB_E_NOTABLE,
								DB_E_TABLEINUSE,
								DB_SEC_E_PERMISSIONDENIED,
							};
	ULONG	cValidRes		= NUMELEM(rgValidRes);
	BOOL	fIndexExists	= FALSE;

	if (pIndexID && pTableID)
		CHECK(DoesIndexExist(pTableID, pIndexID, &fIndexExists), S_OK);

	hr = m_pIIndexDefinition->DropIndex(pTableID, pIndexID);
	
	COMPARE(CheckResult(hr, cValidRes, rgValidRes), TRUE);

	switch (hr)
	{
		case S_OK:
			if (	pIndexID && COMPARE(fIndexExists, TRUE) 
				&&	CHECK(DoesIndexExist(pTableID, pIndexID, &fIndexExists), S_OK)) 
				COMPARE(fIndexExists, FALSE);
			break;

		case E_INVALIDARG:
			// Note: some provider will not support dropping all indexes
			COMPARE((NULL == pTableID) || (NULL == pIndexID), TRUE);
			break;
		
		case DB_S_ERRORSOCCURRED:
		case DB_E_ERRORSOCCURRED:
			COMPARE(NULL != pTableID, TRUE);
			COMPARE(NULL == pIndexID, TRUE);
			break;

		case DB_E_NOINDEX:
			COMPARE(fIndexExists, FALSE);
			break;

		case DB_E_NOTABLE:
			{
				BOOL	fTableExists = FALSE;
				CTable	Table(m_pIIndexDefinition, (LPWSTR)gwszModuleName);
				
				if (	COMPARE(NULL != pTableID, TRUE) 
					&&	CHECK(Table.DoesTableExist(pTableID, &fTableExists), S_OK))
					COMPARE(fTableExists, FALSE);
			}
			break;

		case DB_E_INDEXINUSE:
		case DB_E_TABLEINUSE:
		case DB_SEC_E_PERMISSIONDENIED:
			COMPARE(NULL != pTableID, TRUE);
			COMPARE(!pIndexID || fIndexExists, TRUE);
			break;

		default:
			break;
	}

	return hr;
} //CIIndexDefinition::DropIndex



// helper methods
HRESULT CIIndexDefinition::CreateIndexAndCheck(
	DBID				*pTableID,				// [in]		the ID of the table
	DBID				*pIndexID,				// [in]		the ID of the index
	DBORDINAL			nIndexSize,				// [in]		index size (no of columns)
	DBINDEXCOLUMNDESC	*rgIndexColumnDesc,		// [in]		columns to be part of index 
	ULONG				cPropSets,				// [in]		count of property sets 
	DBPROPSET			*rgPropSets,			// [in]		array of property sets
	DBID				**ppIndexID				// [in/out]	stores output ptr to IndexID
)
{
	DBID				*pNewIndexID = NULL;	// DBID of the newly created index
	DBPROPSET			*rgPropSets1 = NULL;	// saves the property sets
	HRESULT				hres;
	BOOL				fExists;
	BOOL				fIndexAlreadyExists = FALSE;;
	ULONG				nIndexPropSet;
	ULONG				nIndexProp;
	DBPROP				*pProp;

	// save the array of property sets, as asked
	DuplicatePropertySets(cPropSets, rgPropSets, &cPropSets, &rgPropSets1);

	if (ppIndexID)
		*ppIndexID = NULL;

	if (pIndexID && CHECK(DoesIndexExist(pTableID, pIndexID, &fExists), S_OK) && fExists)
		fIndexAlreadyExists = TRUE;

	// create the index according the requirements
	hres	= CreateIndex(pTableID, pIndexID, nIndexSize, rgIndexColumnDesc, 
											cPropSets, rgPropSets, ppIndexID);

	// get an DBID of the index
	if ((NULL != pIndexID) || (NULL != ppIndexID))
		pNewIndexID = pIndexID? pIndexID: *ppIndexID;
	else
	{
		CHECK(hres, E_INVALIDARG);
		goto CLEANUP;
	}

	// check index property status
	if (rgPropSets)
	{
		for (nIndexPropSet=0; nIndexPropSet < cPropSets; nIndexPropSet++)
		{
			pProp = rgPropSets[nIndexPropSet].rgProperties;
			if (!pProp)
				continue;
			for (nIndexProp=0; nIndexProp < rgPropSets[nIndexPropSet].cProperties; nIndexProp++)
			{
				CheckProperty(rgPropSets[nIndexPropSet].guidPropertySet, &pProp[nIndexProp], hres);
			}
		}
	}

	if (SUCCEEDED(hres))
	{
		if (!COMPARE(fIndexAlreadyExists, FALSE))
		{
			odtLog << "ERROR: success in creating an already existing index\n";
			goto CLEANUP;
		}

		// the index is reported to be created, check it thoroughfully
		if (	NULL == pNewIndexID 
			||	!CHECK(DoesIndexExist(pTableID, pNewIndexID, &fExists), S_OK) 
			||	!COMPARE(fExists, TRUE)
		   )
		{
			odtLog << "ERROR: the method succeeded, but the index was not created or ppIndexID was not created!\n";
		}
		else
		{
			// check the columns, their collation and the index properties on
			// IDBSchemaRowset or IRowsetIndex
			if (!COMPARE(CheckIndex(pTableID, pNewIndexID, nIndexSize, rgIndexColumnDesc,
				cPropSets, rgPropSets), TRUE))
			{
				odtLog << "ERROR: the index was not properly created!\n";
			}
			// check the properties as returned
			if (!(IsPropSetPreserved(rgPropSets1, rgPropSets, cPropSets)))
			{
				odtLog << "ERROR: properties are not preserved!\n";
			}
		}
		
		// compare *pIndexID and **ppIndexID
		if (!CheckReturnedDBID(pIndexID, ppIndexID))
		{
			odtLog << "the returned IndexID was not created ok!\n";
		}
	}
	else
	{
		// failed to create the index
		if (!pIndexID && DB_E_DUPLICATEINDEXID == hres)
		{
			odtLog << "ERROR: DB_E_DUPLICATEINDEXID return on a NULL pIndexID!\n";
			COMPARE(TRUE, FALSE);
		}
		// check that the index is not created
		if (	!fIndexAlreadyExists 
			&&	NULL != pNewIndexID 
			&&	CHECK(DoesIndexExist(pTableID, pNewIndexID, &fExists), S_OK) 
			&&	!COMPARE(fExists, FALSE))
		{
			odtLog << "ERROR: the method failed, but the index was created!\n";
			CHECK(m_pIIndexDefinition->DropIndex(pTableID, pNewIndexID), S_OK);
		}
		// should not allocate memory for the index ID
		if (ppIndexID && !COMPARE(NULL == *ppIndexID, TRUE))
		{
			odtLog << "ERROR: the index was not created, but *ppIndexID was returned not null!\n";
			ReleaseDBID(*ppIndexID);
			*ppIndexID = NULL;
		}
	}

CLEANUP:
	// release the old properties
	FreeProperties( &cPropSets, &rgPropSets1);
	return hres;
} //CIIndexDefinition::CreateIndexAndCheck
		


HRESULT	CIIndexDefinition::CreateCheckAndDropIndex(
	DBID				*pTableID,				// [in]		the ID of the table
	DBID				*pIndexID,				// [in]		the ID of the index
	DBORDINAL			nIndexSize,				// [in]		index size (no of columns)
	DBINDEXCOLUMNDESC	*rgIndexColumnDesc,		// [in]		columns to be part of index 
	ULONG				cPropSets,				// [in]		count of property sets 
	DBPROPSET			*rgPropSets				// [in]		array of property sets
)
{
	CDBIDPtr	pOutIndexID;
	HRESULT		hr				= E_FAIL;
	BOOL		fExists;

	hr = CreateIndexAndCheck(pTableID, pIndexID, nIndexSize, 
								rgIndexColumnDesc, cPropSets, rgPropSets, (DBID**)pOutIndexID);
	if (E_FAIL == hr || NULL == (DBID*)pOutIndexID)
		goto CLEANUP;

	// check that the index is not created
	if (	CHECK(DoesIndexExist(pTableID, pOutIndexID, &fExists), S_OK) 
		&&	fExists)
		CHECK(DropIndex(pTableID, pOutIndexID), S_OK);

CLEANUP:
//	ReleaseDBID(&pOutIndexID);
	return hr;
} //CIIndexDefinition::CreateCheckAndDropIndex



// Check whether the index is created and property settings
BOOL CIIndexDefinition::CheckIndex(
	DBID				*pTableID,				// the index of the table
	DBID				*pIndexID,				// the index to be checked
	DBORDINAL			cIndexColumnDesc,		// how many elements
	DBINDEXCOLUMNDESC	*rgIndexColumnDesc,		// array with index original descriprion
	ULONG				cPropertySets,			// number of property sets
	DBPROPSET			*rgPropertySets			// the array of property sets
)
{
	return		(!s_fIRowsetIndex || CheckIndexUsingIRowsetIndex(pTableID, pIndexID, cIndexColumnDesc, rgIndexColumnDesc, 
											cPropertySets, rgPropertySets))
			&&	CheckIndexUsingIDBSchemaRowset(pTableID, pIndexID, cIndexColumnDesc, rgIndexColumnDesc, 
											cPropertySets, rgPropertySets);
} //CIIndexDefinition::CheckIndex



// Sets/Resets the nullibility of each column in the table.
BOOL TCIIndexDefinition::MakeColsNullable(
	CTable				*pTable,
	BOOL				bNullable
)
{
	CList<CCol, CCol&>  *pColList;
	POSITION			Pos;

	pColList = pTable->GetColList();
	Pos = pColList->GetHeadPosition();

	while(Pos)
	{
		CCol &rCol = pColList->GetNext(Pos);
		rCol.SetNullable( bNullable );
	}

	return TRUE;
}
// TCIIndexDefinition::MakeColsNullable


BOOL CIIndexDefinition::CheckIndexUsingIRowsetIndex(
	DBID				*pTableID,				// the index of the table
	DBID				*pIndexID,				// the index to be checked
	DBORDINAL			cIndexColumnDesc,		// how many elements
	DBINDEXCOLUMNDESC	*rgIndexColumnDesc,		// array with index original descriprion
	ULONG				cPropertySets,			// number of property sets
	DBPROPSET			*rgPropertySets			// the array of property sets
)
{
	TBEGIN
	IRowsetIndex		*pIRowsetIndex = NULL;
	ULONG				i;
	DBORDINAL			cColumns = 0;
	ULONG				cIndexPropSets=0, nPropSet, nProp;
	DBINDEXCOLUMNDESC	*rgOutIndexColumnDesc = NULL;
	DBPROPSET			*rgIndexPropSets = NULL;
	BOOL				fResult = TRUE, fProp=TRUE;
	DBPROP				*pProp=NULL;
	DBPROPSTATUS		dwStatus;
	VARIANT				*pvValue;
	DBPROPID			dwPropID;
	IOpenRowset			*pIOpenRowset = NULL;
	HRESULT				hr;

	TESTC(VerifyInterface(m_pIIndexDefinition, IID_IOpenRowset, SESSION_INTERFACE, (IUnknown**)&pIOpenRowset));

	TEST2C_(hr = pIOpenRowset->OpenRowset(NULL, pTableID, pIndexID, 
		IID_IRowsetIndex, 0, NULL, (IUnknown**)&pIRowsetIndex), S_OK, E_NOINTERFACE);

	if (S_OK != hr)
		goto CLEANUP;

	// use IRowsetIndex->GetIndexInfo to check how the index was set
	TESTC_(pIRowsetIndex->GetIndexInfo(&cColumns, &rgOutIndexColumnDesc, &cIndexPropSets, &rgIndexPropSets), S_OK);
	TESTC(cColumns == cIndexColumnDesc);

	for (i=0; i<cIndexColumnDesc; i++)
	{
		if (!COMPARE(rgIndexColumnDesc[i].eIndexColOrder == rgOutIndexColumnDesc[i].eIndexColOrder, TRUE))
		{
			odtLog << "order (A/D) differ for column " << rgIndexColumnDesc[i].pColumnID->uName.pwszName << "\n";
			goto CLEANUP;
		}
		// this is based on the assumption that DBID returned for columns are in the same
		// format as they were input
		// otherwise one have to use a complete description of the columns in the table and make the conversion
		if (!COMPARE(CompareDBID(*rgIndexColumnDesc[i].pColumnID, *rgOutIndexColumnDesc[i].pColumnID, m_pIDSOUnknown), TRUE))
		{
			odtLog << "column ID different for column " << rgIndexColumnDesc[i].pColumnID->uName.pwszName << "\n";
			goto CLEANUP;
		}
	}

	if (!rgPropertySets)
		goto CLEANUP;

	for (nPropSet=0; nPropSet<cPropertySets; nPropSet++)
	{
		if (DBPROPSET_INDEX != rgPropertySets[nPropSet].guidPropertySet)
		{
			for (nProp=0; nProp<rgPropertySets[nPropSet].cProperties; nProp++)
			{
				COMPARE(rgPropertySets[nPropSet].rgProperties[nProp].dwStatus, DBPROPSTATUS_NOTSUPPORTED);
			}
			continue;
		}
		if (!rgPropertySets[nPropSet].rgProperties)
			continue;
		for (nProp=0; nProp<rgPropertySets[nPropSet].cProperties; nProp++)
		{
			dwStatus	= rgPropertySets[nPropSet].rgProperties[nProp].dwStatus;
			dwPropID	= rgPropertySets[nPropSet].rgProperties[nProp].dwPropertyID;
			pvValue		= &rgPropertySets[nPropSet].rgProperties[nProp].vValue;

			// try to find corresponding property got from IRowsetIndex
			if (!FindProperty(	dwPropID,
								rgPropertySets[nPropSet].guidPropertySet,
								cIndexPropSets, 
								rgIndexPropSets, 
								&pProp))	
			{
				COMPARE(dwStatus, DBPROPSTATUS_NOTSUPPORTED);
			} 
			else
				// prop status were check in CreateAndCheckIndex by calling CheckProperty
				if (DBPROPSTATUS_OK == dwStatus)
					COMPARE(CompareVariant(&pProp->vValue, pvValue), TRUE);
		}
	}
	
CLEANUP:
	SAFE_RELEASE(pIRowsetIndex);
	SAFE_RELEASE(pIOpenRowset);
	TRETURN
} //CIIndexDefinition::CheckIndexUsingIRowsetIndex



//--------------------------------------------------------------------------
//
//	@cmember Read the value of a column in the indexe schema rowset
//
//	RETURNS FALSE if the value was read and differ than the original one
//--------------------------------------------------------------------------
BOOL CIIndexDefinition::GetIndexValueFromFirstRow(
	LPVOID		pVariable,		// [OUT]	value read
	BOOL		*pfSet,			// [OUT]	if the value is set
	DBBINDING	*rgDBBINDING,	// [IN]		binding array
	ULONG		cColumn,		// [IN]		column to be read
	ULONG		ulDBTYPE,		// [IN]		type of property variant
	BYTE		*pData,			// [IN]		pointer to read DATA stru
	DBPROPID	PropID,			// [IN]		property that is being read
	ULONG		cPropSets,		// [IN]		number of property sets
	DBPROPSET	*rgPropSets,	// [IN]		array of property sets
	WCHAR		*lpwszMesaj		// [IN]		message text for error
)
{
	BOOL	fRes = TRUE;
	DBPROP	*pProp = NULL;;

	TESTC(NULL != pfSet);

	*pfSet = FALSE;
	FindProperty(PropID, DBPROPSET_INDEX, cPropSets, rgPropSets, &pProp);
	if (!pProp)
		goto CLEANUP;

	// get the variable value
	switch (ulDBTYPE)
	{
		case DBTYPE_BOOL:
			*(VARIANT_BOOL*)pVariable = V_BOOL(&pProp->vValue);
			break;

		case DBTYPE_I4:
			*(LONG*)pVariable = V_I4(&pProp->vValue);
			break;

		case DBTYPE_UI2:
			*(unsigned short*)pVariable = V_UI2(&pProp->vValue);
			break;

		case DBTYPE_I2:
			*(SHORT*)pVariable = V_I2(&pProp->vValue);
			break;
		default:
			TESTC(FALSE);
	}
	*pfSet = (DBPROPSTATUS_OK == pProp->dwStatus);

CLEANUP:
	fRes = GetIndexValue(pVariable, pfSet, rgDBBINDING, cColumn, ulDBTYPE, pData, lpwszMesaj);
	if (DBPROP_INDEX_FILLFACTOR == PropID && fProperRangeForFillFactor)
	{
		if (!COMPARE(0 <= *(LONG*)pVariable && *(LONG*)pVariable <= 100, TRUE))
			fProperRangeForFillFactor = FALSE;
	}
	return fRes;
} //CIIndexDefinition::GetIndexValueFromFirstRow



//--------------------------------------------------------------------------
//
//	@cmember Read the value of a column in the index schema rowset
//
//	if *pfSet is TRUE then there is a coparison value for the value read,
// otherwise the new value will be used for further comparisons => save
// it in *pVariable and set *pfSet to TRUE
//	RETURNS FALSE if the value was read and differ than the original one
//--------------------------------------------------------------------------
BOOL CIIndexDefinition::GetIndexValue(
	LPVOID		pVariable,		// [OUT]	value read
	BOOL		*pfSet,			// [OUT]	if the value is set
	DBBINDING	*rgDBBINDING,	// [IN]		binding array
	ULONG		cColumn,		// [IN]		column to be read
	ULONG		ulDBTYPE,		// [IN]		type of property variant
	BYTE		*pData,			// [IN]		pointer to read DATA stru
	WCHAR		*lpwszMesaj		// [IN]		message text for error
)
{
	BOOL	fRes		= FALSE;
	BOOL	fWStrComp	= FALSE;
	DATA	*pColumn	= NULL;

	TESTC(NULL != pData);
	TESTC(NULL != rgDBBINDING);
	TESTC(NULL != pVariable);
	TESTC(NULL != pfSet);
	fRes = TRUE;

	// get the status of the read property value
	pColumn = (DATA*) (pData+rgDBBINDING[cColumn].obStatus);	
	if (DBSTATUS_S_OK == pColumn->sStatus)							
	{
		// get the value in pVariable
		switch (ulDBTYPE)
		{
			case DBTYPE_I4:
				if (!*pfSet)
				{
					*(LONG*)pVariable = *(LONG*)(&(pColumn->bValue));						
					*pfSet = TRUE;
				}
				else
					fRes =  (*(LONG*)(&(pColumn->bValue)) == *(LONG*)pVariable);	
				break;

			case DBTYPE_UI2:
				if (!*pfSet)
				{
					*(unsigned short*)pVariable = *(unsigned short*)(&(pColumn->bValue));						
					*pfSet = TRUE;
				}
				else
					fRes =  (*(unsigned short*)(&(pColumn->bValue)) == *(unsigned short*)pVariable);
				break;

			case DBTYPE_I2:
				if (!*pfSet)
				{
					*(SHORT*)pVariable = *(SHORT*)(&(pColumn->bValue));						
					*pfSet = TRUE;
				}
				else
					fRes =  (*(SHORT*)(&(pColumn->bValue)) == *(SHORT*)pVariable);
				break;

			case DBTYPE_BOOL:
				if (!*pfSet)
				{
					*(VARIANT_BOOL*)pVariable = *(VARIANT_BOOL*)(&(pColumn->bValue));						
					*pfSet = TRUE;
				}
				else
					fRes =  (*(VARIANT_BOOL*)(&(pColumn->bValue)) == *(VARIANT_BOOL*)pVariable);
				break;

			case DBTYPE_WSTR:
				if (!*pfSet)
				{
					TESTC(NULL != *(WCHAR**)pVariable);
					// release the memory is wrongfully passed
					if (!*(WCHAR*)pVariable)
						SAFE_FREE(*(WCHAR**)pVariable);
					// set the variable 
					*(WCHAR**)pVariable = wcsDuplicate(*(WCHAR**)(&(pColumn->bValue)));						
					*pfSet = TRUE;
				}
				else
					// compare
					fRes =		(NULL != *(WCHAR**)pVariable) 
							&&	S_OK == CompareID(&fWStrComp, *(WCHAR**)pVariable, (WCHAR*)(&(pColumn->bValue)), m_pIDSOUnknown)
							&& fWStrComp;
				break;
			default:
				ASSERT(FALSE);
				break;
		}
		if (!fRes)
			odtLog << lpwszMesaj;											
	}

CLEANUP:
	return fRes;
} // CIIndexDefinition::GetIndexValue



BOOL CIIndexDefinition::CheckIndexUsingIDBSchemaRowset(
	DBID				*pTableID,				// the index of the table
	DBID				*pIndexID,				// the index to be checked
	DBORDINAL			cIndexColumnDesc,		// how many elements
	DBINDEXCOLUMNDESC	*rgIndexColumnDesc,		// array with index original descriprion
	ULONG				cPropertySets,			// number of property sets
	DBPROPSET			*rgPropertySets			// the array of property sets
)
{
	TBEGIN
	HRESULT 			hr				= E_FAIL;
	BOOL				bIsSchemaSupported;
	DBLENGTH			ulRowSize		= 0;		// size of row
	DBCOUNTITEM			cDBBINDING		= 0;		// count of bindings
	DBORDINAL			iRow			= 0;		// count of rows
	DBCOUNTITEM			cRowsObtained	= 0;		// number of rows returned, should be 1
	ULONG				cSchema			= 0;		// number of supported Schemas
	ULONG				*prgRestrictions= 0;		// restrictions for each Schema
	GUID				*prgSchemas		= NULL;		// array of GUIDs
	HROW				hRow;						// handler of rows
	HROW				*phRow = &hRow;				// pointer to handler
	IRowset				*pIndexRowset	= NULL;		// returned rowset
	DBBINDING			*rgDBBINDING	= NULL;		// array of bindings
	BYTE				*pData			= NULL;		// pointer to data
	HACCESSOR 			hAccessor		= NULL;		// accessor
	BOOL				*rgColPresent = NULL;
	const int			cRest = 5;
	VARIANT				rgRestrictIndexes[cRest];
	ULONG				i, index;
	ULONG				nColPresent=0;
	ULONG				nTABLE_NAME			= 2;
	ULONG				nINDEX_NAME			= 5;
	ULONG				nPRIMARY_KEY		= 6;
	ULONG				nUNIQUE				= 7;
	ULONG				nCLUSTERED			= 8;
	ULONG				nTYPE				= 9;
	ULONG				nFILL_FACTOR		= 10;
	ULONG				nINITIAL_SIZE		= 11;
	ULONG				nNULLS				= 12;
	ULONG				nSORT_BOOKMARKS		= 13;
	ULONG				nAUTO_UPDATE		= 14;
	ULONG				nNULL_COLLATION		= 15;
	ULONG				nORDINAL_POSITION	= 16;
	ULONG				nCOLUMN_NAME		= 17;
	ULONG				nCOLLATION			= 20;

	// column entries in index schema rowset and their presence flags
	LPWSTR				pwszIndexName		= NULL;
	BOOL				fIndexName			= FALSE;
	LPWSTR				pwszTableName		= NULL;
	BOOL				fTableName			= FALSE;
	VARIANT_BOOL		vbAutoUpdate;
	BOOL				fAutoUpdate			= FALSE;
	VARIANT_BOOL		vbClustered;
	BOOL				fClustered			= FALSE;
	LONG				lFillFactor;
	BOOL				fFillFactor			= FALSE;
	LONG				lInitialSize;
	BOOL				fInitialSize		= FALSE;
	LONG				lNullCollation;
	BOOL				fNullCollation		= FALSE;
	unsigned short		uiOrdinalPosition;
	BOOL				fOrdinalPosition;
	SHORT				iCollation;
	BOOL				fCollation			= FALSE;
	LPWSTR				pwszColumnName		= NULL;
	BOOL				fColumnName			= FALSE;
	LONG				lNulls;
	BOOL				fNulls				= FALSE;
	VARIANT_BOOL		vbPrimaryKey;
	BOOL				fPrimaryKey			= FALSE;
	VARIANT_BOOL		vbSortBookmarks;
	BOOL				fSortBookmarks		= FALSE;
	unsigned short		usType;
	BOOL				fType				= FALSE;
	VARIANT_BOOL		vbUnique;
	BOOL				fUnique				= FALSE;
	IDBSchemaRowset		*pIDBSchemaRowset	= NULL;

	// Set restrictions
	for(index=0;index<cRest;index++)
		VariantInit(&rgRestrictIndexes[index]);

	TESTC_PROVIDER(VerifyInterface(m_pIIndexDefinition, IID_IDBSchemaRowset, SESSION_INTERFACE, (IUnknown**)&pIDBSchemaRowset));

	// Check to see if the schema is supported
	TESTC_(hr = pIDBSchemaRowset->GetSchemas(&cSchema, &prgSchemas, &prgRestrictions),S_OK);

	ASSERT(NULL != pIndexID);
	ASSERT(NULL != pTableID);

	if (DBKIND_NAME == pIndexID->eKind)
	{
		pwszIndexName = wcsDuplicate(pIndexID->uName.pwszName);
		fIndexName = TRUE;
	}

	if (DBKIND_NAME == pTableID->eKind)
	{
		pwszTableName = wcsDuplicate(pTableID->uName.pwszName);
		fTableName = TRUE;
	}

	// Check to see if DBSCHEMA_INDEXES is supported
	for(i=0, bIsSchemaSupported=FALSE; i<cSchema && !bIsSchemaSupported;)
	{
		if(prgSchemas[i] == DBSCHEMA_INDEXES)
			bIsSchemaSupported = TRUE;
		else 
			i++;
	}

	if(!bIsSchemaSupported || !(prgRestrictions[i] & 0x4) || !(prgRestrictions[i] & 0x10))
	{
		odtLog << "Index Schema Rowset or the required constraints are not supported\n";
		goto CLEANUP;
	}

	rgRestrictIndexes[2].vt 	 = VT_BSTR;
	rgRestrictIndexes[2].bstrVal = SysAllocString(pIndexID->uName.pwszName);
	rgRestrictIndexes[4].vt 	 = VT_BSTR;
	rgRestrictIndexes[4].bstrVal = SysAllocString(pTableID->uName.pwszName);

	SAFE_ALLOC(rgColPresent, BOOL, cIndexColumnDesc);
	for (i=0; i<cIndexColumnDesc; i++)
		rgColPresent[i] = FALSE;

	TESTC_(hr = pIDBSchemaRowset->GetRowset(	
			NULL, 								// aggregation
			DBSCHEMA_INDEXES,					// REFGUID
			cRest,	 							// count of restrictions (1:types)
			rgRestrictIndexes,					// list of restrictions
			IID_IRowset,						// REFFID
			0,									// count of properties
			NULL,								// range of properties
			(IUnknown**)&pIndexRowset			// returned result set
		),S_OK);

	TESTC_(hr = GetAccessorAndBindings(
						pIndexRowset, DBACCESSOR_ROWDATA, &hAccessor,	&rgDBBINDING,		
						&cDBBINDING, &ulRowSize, DBPART_VALUE |DBPART_STATUS |DBPART_LENGTH,
						ALL_COLS_EXCEPTBOOKMARK, FORWARD, NO_COLS_BY_REF,
						NULL,					// OUT: Array of DBCOLUMNINFOs
						0,						// OUT: Count of DBCOULMNINFOs
						NULL,					//&pStringsBuffer, 
						DBTYPE_EMPTY, 
						0, NULL),S_OK);
	SAFE_ALLOC(pData, BYTE, ulRowSize);	//data

	// read the first row, check local data and initialize variables
	TESTC_(hr=pIndexRowset->GetNextRows(0, 0, 1, &cRowsObtained, &phRow), S_OK);
	TESTC_((long)cRowsObtained, 1);
	TESTC_(hr=pIndexRowset->GetData(hRow, hAccessor, pData),S_OK);

	// get the first values and verifications
	COMPARE(GetIndexValue(&pwszIndexName, &fIndexName, rgDBBINDING, 
		nINDEX_NAME, DBTYPE_WSTR, pData, L"ERROR: Bad index name\n"), TRUE);

	COMPARE(GetIndexValue(&pwszTableName, &fTableName, rgDBBINDING, 
		nTABLE_NAME, DBTYPE_WSTR, pData, L"ERROR: Bad table name\n"), TRUE);

	// check column ordinal position of the first column in the index (1 is the first value) and name
	fOrdinalPosition = FALSE;
	COMPARE(GetIndexValue(&uiOrdinalPosition, &fOrdinalPosition, rgDBBINDING, 
		nORDINAL_POSITION, DBTYPE_UI2, pData, L"ERROR: OrdinalPosition\n"), TRUE);

	if (fOrdinalPosition)
	{
		TESTC(cIndexColumnDesc >= uiOrdinalPosition);

		DBID	*pColumnID = rgIndexColumnDesc[uiOrdinalPosition-1].pColumnID;

		// can compare both column name and collation order
		TESTC(NULL != pColumnID);
		fColumnName = TRUE;
		switch (pColumnID->eKind)
		{
			case DBKIND_NAME:
				pwszColumnName = wcsDuplicate(pColumnID->uName.pwszName);
				COMPARE(GetIndexValue(&pwszColumnName, &fColumnName, rgDBBINDING, 
					nCOLUMN_NAME, DBTYPE_WSTR, pData, L"ERROR: Column name\n"), TRUE);
				SAFE_FREE(pwszColumnName);
				break;
		}

		fCollation	= TRUE;
		iCollation	=  (short)((DBINDEX_COL_ORDER_ASC == rgIndexColumnDesc[uiOrdinalPosition-1].eIndexColOrder) ?
						DB_COLLATION_ASC: DB_COLLATION_DESC);
		COMPARE(GetIndexValue(&iCollation, &fCollation, rgDBBINDING, 
			nCOLLATION, DBTYPE_I2, pData, L"ERROR: Collation\n"), TRUE);

		rgColPresent[uiOrdinalPosition-1] = TRUE;
		nColPresent++;
	}

	if (cPropertySets > 0 && rgPropertySets)
	{
		COMPARE(GetIndexValueFromFirstRow(&vbAutoUpdate, &fAutoUpdate, 
			rgDBBINDING, nAUTO_UPDATE, DBTYPE_BOOL, pData, DBPROP_INDEX_AUTOUPDATE, 
			cPropertySets, rgPropertySets, L"ERROR: Auto update\n"), TRUE);

		COMPARE(GetIndexValueFromFirstRow(&vbClustered, &fClustered, 
			rgDBBINDING, nCLUSTERED, DBTYPE_BOOL, pData, DBPROP_INDEX_CLUSTERED, 
			cPropertySets, rgPropertySets, L"ERROR: Clustered\n"), TRUE);

		COMPARE(GetIndexValueFromFirstRow(&lFillFactor, &fFillFactor, 
			rgDBBINDING, nFILL_FACTOR, DBTYPE_I4, pData, DBPROP_INDEX_FILLFACTOR, 
			cPropertySets, rgPropertySets, L"ERROR: Fill factor\n"), TRUE);

		COMPARE(GetIndexValueFromFirstRow(&lInitialSize, &fInitialSize, 
			rgDBBINDING, nINITIAL_SIZE, DBTYPE_I4, pData, DBPROP_INDEX_INITIALSIZE, 
			cPropertySets, rgPropertySets, L"ERROR: Initial Size\n"), TRUE);

		COMPARE(GetIndexValueFromFirstRow(&lNullCollation, &fNullCollation, 
			rgDBBINDING, nNULL_COLLATION, DBTYPE_I4, pData, DBPROP_INDEX_NULLCOLLATION,
			cPropertySets, rgPropertySets, L"ERROR: Null Collation\n"), TRUE);

		COMPARE(GetIndexValueFromFirstRow(&lNulls, &fNulls, 
			rgDBBINDING, nNULLS, DBTYPE_I4, pData, DBPROP_INDEX_NULLS,
			cPropertySets, rgPropertySets, L"ERROR: Nulls\n"), TRUE);

		COMPARE(GetIndexValueFromFirstRow(&vbPrimaryKey, &fPrimaryKey, 
			rgDBBINDING, nPRIMARY_KEY, DBTYPE_BOOL, pData, DBPROP_INDEX_PRIMARYKEY,
			cPropertySets, rgPropertySets, L"ERROR: Primary Key\n"), TRUE);

		COMPARE(GetIndexValueFromFirstRow(&vbSortBookmarks, &fSortBookmarks, 
			rgDBBINDING, nSORT_BOOKMARKS, DBTYPE_BOOL, pData, DBPROP_INDEX_SORTBOOKMARKS,
			cPropertySets, rgPropertySets, L"ERROR: SortBookmarks\n"), TRUE);

		COMPARE(GetIndexValueFromFirstRow(&usType, &fType, 
			rgDBBINDING, nTYPE, DBTYPE_UI2, pData, DBPROP_INDEX_TYPE,
			cPropertySets, rgPropertySets, L"ERROR: Type\n"), TRUE);

		COMPARE(GetIndexValueFromFirstRow(&vbUnique, &fUnique, 
			rgDBBINDING, nUNIQUE, DBTYPE_BOOL, pData, DBPROP_INDEX_UNIQUE,
			cPropertySets, rgPropertySets, L"ERROR: Unique\n"), TRUE);
	}

	TESTC_(pIndexRowset->ReleaseRows(1, &hRow, NULL, NULL, NULL), S_OK);

	// Get data for each row
	for(iRow=1;iRow<cIndexColumnDesc;iRow++)			 
	{
		TESTC_(hr=pIndexRowset->GetNextRows(0, 0, 1, &cRowsObtained, &phRow), S_OK);
		TESTC_((long)cRowsObtained, 1);
		TESTC_(hr=pIndexRowset->GetData(hRow, hAccessor, pData),S_OK);

		// check that the properties are the same:
		COMPARE(GetIndexValue(&pwszIndexName, &fIndexName, rgDBBINDING, 
			nINDEX_NAME, DBTYPE_WSTR, pData, L"ERROR: Bad index name\n"), TRUE);

		COMPARE(GetIndexValue(&pwszTableName, &fTableName, rgDBBINDING, 
			nTABLE_NAME, DBTYPE_WSTR, pData, L"ERROR: Bad table name\n"), TRUE);

		// check column ordinal and name
		fOrdinalPosition = FALSE;
		COMPARE(GetIndexValue(&uiOrdinalPosition, &fOrdinalPosition, rgDBBINDING, 
			nORDINAL_POSITION, DBTYPE_UI2, pData, L"ERROR: OrdinalPosition\n"), TRUE);

		if (fOrdinalPosition)
		{
			TESTC(cIndexColumnDesc >= uiOrdinalPosition);
	
			DBID	*pColumnID = rgIndexColumnDesc[uiOrdinalPosition-1].pColumnID;

			// can compare both column name and collation order
			TESTC(NULL != pColumnID);
			fColumnName = TRUE;
			switch (pColumnID->eKind)
			{
				case DBKIND_NAME:
					pwszColumnName = wcsDuplicate(pColumnID->uName.pwszName);
					COMPARE(GetIndexValue(&pwszColumnName, &fColumnName, rgDBBINDING, 
						nCOLUMN_NAME, DBTYPE_WSTR, pData, L"ERROR: Column name\n"), TRUE);
					SAFE_FREE(pwszColumnName);
					break;
				default:
					break;
			}

			fCollation	= TRUE;
			iCollation	= (short)((DBINDEX_COL_ORDER_ASC == rgIndexColumnDesc[uiOrdinalPosition-1].eIndexColOrder) ?
							DB_COLLATION_ASC: DB_COLLATION_DESC);
			COMPARE(GetIndexValue(&iCollation, &fCollation, rgDBBINDING, 
				nCOLLATION, DBTYPE_I2, pData, L"ERROR: Collation\n"), TRUE);

			rgColPresent[uiOrdinalPosition-1] = TRUE;
			nColPresent++;
		}

		COMPARE(GetIndexValue(&vbAutoUpdate, &fAutoUpdate, rgDBBINDING, 
			nAUTO_UPDATE, DBTYPE_BOOL, pData, L"ERROR: Auto update\n"), TRUE);

		COMPARE(GetIndexValue(&vbClustered, &fClustered, rgDBBINDING, 
			nCLUSTERED, DBTYPE_BOOL, pData, L"ERROR: Clustered\n"), TRUE);

		COMPARE(GetIndexValue(&lFillFactor, &fFillFactor, rgDBBINDING, 
			nFILL_FACTOR, DBTYPE_I4, pData, L"ERROR: Fill factor\n"), TRUE);

		COMPARE(GetIndexValue(&lInitialSize, &fInitialSize, rgDBBINDING, 
			nINITIAL_SIZE, DBTYPE_I4, pData, L"ERROR: Initial Size\n"), TRUE);

		COMPARE(GetIndexValue(&lNullCollation, &fNullCollation, rgDBBINDING, 
			nNULL_COLLATION, DBTYPE_I4, pData, L"ERROR: Null Collation\n"), TRUE);

		COMPARE(GetIndexValue(&lNulls, &fNulls, rgDBBINDING, 
			nNULLS, DBTYPE_I4, pData, L"ERROR: Nulls\n"), TRUE);

		COMPARE(GetIndexValue(&vbPrimaryKey, &fPrimaryKey, rgDBBINDING, 
			nPRIMARY_KEY, DBTYPE_BOOL, pData, L"ERROR: Primary Key\n"), TRUE);

		COMPARE(GetIndexValue(&vbSortBookmarks, &fSortBookmarks, rgDBBINDING, 
			nSORT_BOOKMARKS, DBTYPE_BOOL, pData, L"ERROR: SortBookmarks\n"), TRUE);

		COMPARE(GetIndexValue(&usType, &fType, rgDBBINDING, 
			nTYPE, DBTYPE_UI2, pData, L"ERROR: Type\n"), TRUE);

		COMPARE(GetIndexValue(&vbUnique, &fUnique, rgDBBINDING, 
			nUNIQUE, DBTYPE_BOOL, pData, L"ERROR: Unique\n"), TRUE);

		TESTC_(pIndexRowset->ReleaseRows(1, &hRow, NULL, NULL, NULL), S_OK);
	}


	TESTC(nColPresent == cIndexColumnDesc);

CLEANUP:
	SAFE_RELEASE(pIDBSchemaRowset);

	// Free the memory
	SAFE_FREE(pwszIndexName);
	SAFE_FREE(pwszTableName);
	
	SAFE_FREE(rgColPresent);

	SAFE_FREE(prgRestrictions);
	SAFE_FREE(prgSchemas);

	SAFE_FREE(pData);

	SAFE_RELEASE(pIndexRowset);

	SAFE_FREE(rgDBBINDING);

	for(index=0;index<cRest;index++)
		GCHECK(VariantClear(&(rgRestrictIndexes[index])),S_OK);
	
	TRETURN
} //CIIndexDefinition::CheckIndexUsingIDBSchemaRowset



//---------------------------------------------------------------------------
// CIIndexDefinition::DoesIndexExistInIndexSchemaRowset
//
// CIIndexDefinition			|
// DoesIndexExist	|
// If this index is on this table return true. If function runs correctly
// but doesn't find the table name, function will return S_OK, but pfExists
// will be FALSE. If strIndexName is empty, returns E_FAIL.	The index is sought in 
// index schema rowset.
//
// @mfunc	DoesIndexExist
// @rdesc HRESULT indicating success or failure
//  @flag S_OK   | Function ran without problem
//  @flag E_FAIL    | Function ran with problems
//
//---------------------------------------------------------------------------
HRESULT CIIndexDefinition::DoesIndexExistInIndexSchemaRowset(	
	DBID		*pTableID,				// @parm [IN]	Table ID										  
	DBID		*pIndexID,				// @parm [IN]	Index ID
	BOOL 		*pfExists,				// @parm [OUT]	TRUE if index exists
	BOOL		fAnyIndex				// @parm [IN]   TRUE if search for all indexes
										//				DEFAULT FALSE
)
{
	HRESULT 			hr				= E_FAIL;
	DBCOUNTITEM			cRowsObtained	= 0;			// number of rows returned, should be 1
	HROW				*rghRows		= NULL;			// array of handles of rows
	IRowset				*pIRowset		= NULL;			// returned rowset
	const int			cRest = 5;						// restrictions on INDEXES Schema Rowset
	VARIANT				rgRestrictIndexes[cRest];
	ULONG				index;
	IDBSchemaRowset		*pIDBSchemaRowset	= NULL;

	// Set restrictions
	for(index=0;index<cRest;index++)
		VariantInit(&rgRestrictIndexes[index]);

	TESTC_PROVIDER(VerifyInterface(m_pIIndexDefinition, IID_IDBSchemaRowset, SESSION_INTERFACE, (IUnknown**)&pIDBSchemaRowset));

	TESTC((NULL != pIndexID) || ((NULL != pTableID) && fAnyIndex));

	if (pIndexID && !fAnyIndex)
	{
		rgRestrictIndexes[2].vt 	 = VT_BSTR;
		if (pIndexID->eKind==DBKIND_NAME || pIndexID->eKind==DBKIND_GUID_NAME || pIndexID->eKind==DBKIND_PGUID_NAME)
			rgRestrictIndexes[2].bstrVal = SysAllocString(pIndexID->uName.pwszName);
		else
			rgRestrictIndexes[2].bstrVal = NULL;
	}

	if (pTableID)
	{
		rgRestrictIndexes[4].vt 	 = VT_BSTR;
		if (pTableID->eKind==DBKIND_NAME || pTableID->eKind==DBKIND_GUID_NAME || pTableID->eKind==DBKIND_PGUID_NAME)
			rgRestrictIndexes[4].bstrVal = SysAllocString(pTableID->uName.pwszName);
		else
			rgRestrictIndexes[4].bstrVal = NULL;
	}

	if (!CHECK(hr = pIDBSchemaRowset->GetRowset(	
			NULL, 								// aggregation
			DBSCHEMA_INDEXES,					// REFGUID
			cRest,	 							// count of restrictions (1:types)
			rgRestrictIndexes,					// list of restrictions
			IID_IRowset,						// REFFID
			0,									// count of properties
			NULL,								// range of properties
			(IUnknown**)&pIRowset				// returned result set
		),S_OK))
			goto CLEANUP;

	// Only do this once, if there is a rowset then 
	// there is a table already in the data source
	hr = pIRowset->GetNextRows(0, 0, 1, &cRowsObtained, &rghRows);
	if (pfExists)
		*pfExists = (1 == cRowsObtained);	
	CHECK(hr=pIRowset->ReleaseRows(cRowsObtained,rghRows,NULL,NULL,NULL),S_OK);

CLEANUP:
	SAFE_RELEASE(pIDBSchemaRowset);
	SAFE_RELEASE(pIRowset);
	SAFE_FREE(rghRows);

	for(index=0;index<cRest;index++)
		GCHECK(VariantClear(&(rgRestrictIndexes[index])),S_OK);
	
	return hr;
} //CIIndexDefinition::DoesIndexExistInIndexSchemaRowset


//---------------------------------------------------------------------------
// CIIndexDefinition::DoesIndexExistRowsetIndex
//
// CIIndexDefinition			|
// DoesIndexExist	|
// If this index is on this table return true. If function runs correctly
// but doesn't find the table name, function will return S_OK, but pfExists
// will be FALSE. If strIndexName is empty, returns E_FAIL.	The index is sought in 
// index schema rowset.
//
// @mfunc	DoesIndexExist
// @rdesc HRESULT indicating success or failure
//  @flag S_OK   | Function ran without problem
//  @flag E_FAIL    | Function ran with problems
//
//---------------------------------------------------------------------------
HRESULT CIIndexDefinition::DoesIndexExistRowsetIndex(	
	DBID		*pTableID,				// @parm [IN]	Table ID										  
	DBID		*pIndexID,				// @parm [IN]	Index ID
	BOOL 		*pfExist				// @parm [OUT] TRUE if index exists
)
{
	HRESULT			hr;
	IRowsetIndex	*pRowsetIndex	= NULL;
	IOpenRowset		*pIOpenRowset	= NULL;

	TESTC(VerifyInterface(m_pIIndexDefinition, IID_IOpenRowset, SESSION_INTERFACE, (IUnknown**)&pIOpenRowset));
	hr = pIOpenRowset->OpenRowset(NULL, pTableID, pIndexID, IID_IRowsetIndex, 0, NULL, 
			(IUnknown**)&pRowsetIndex);
	if (pfExist)
		*pfExist = (S_OK == hr)? TRUE: FALSE;

CLEANUP:
	SAFE_RELEASE(pRowsetIndex);
	SAFE_RELEASE(pIOpenRowset);
	return S_OK;
} //CIIndexDefinition::DoesIndexExistRowsetIndex



//---------------------------------------------------------------------------
// CIIndexDefinition::DoesIndexExist  	
//
// CIIndexDefinition			|
// DoesIndexExist	|
// If this index is on this table return true If function runs correctly
// but doesn't find the table name, function will return S_OK, but pfExists
// will be FALSE. If strIndexName is empty, returns E_FAIL.	
//
// @mfunc	DoesIndexExist
// @rdesc HRESULT indicating success or failure
//  @flag S_OK   | Function ran without problem
//  @flag E_FAIL    | Function ran with problems
//
//---------------------------------------------------------------------------
HRESULT CIIndexDefinition::DoesIndexExist(	
	DBID		*pTableID,				// @parm [IN]	Table ID										  
	DBID		*pIndexID,				// @parm [IN]	Index ID
	BOOL		*pfExists,				// @parm [OUT]	TRUE if index exists
	BOOL		fAnyIndex				// @parm [IN]   TRUE if search for all indexes
										// DEFAULT FALSE
)
{
	HRESULT			hr					= E_FAIL;
	IDBSchemaRowset	*pIDBSchemaRowset	= NULL;

	if (!pfExists)
		return E_INVALIDARG;

	TESTC_PROVIDER(VerifyInterface(m_pIIndexDefinition, IID_IDBSchemaRowset, SESSION_INTERFACE, (IUnknown**)&pIDBSchemaRowset));

	if (fAnyIndex)
		if (pIDBSchemaRowset)
			hr = DoesIndexExistInIndexSchemaRowset(pTableID, NULL, pfExists, fAnyIndex);
		else
		{
			odtLog << "WARNING: IDBSchemaRowset is not supported\n";
			*pfExists = FALSE;
			hr = DB_S_ERRORSOCCURRED;
		}
	else if (s_fIRowsetIndex)
		hr = DoesIndexExistRowsetIndex(pTableID, pIndexID, pfExists);
	if (pIDBSchemaRowset)
		hr = DoesIndexExistInIndexSchemaRowset(pTableID, pIndexID, pfExists);
	else
		PRVTRACE("**TCIIndexDefinition::DoesIndexExist: how did I get here?\n");

CLEANUP:
	SAFE_RELEASE(pIDBSchemaRowset);
	return hr;
} //CIIndexDefinition::DoesIndexExist





// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Test Case Section
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// }} END_DECLARE_TEST_CASES()

// {{ TCW_TESTMODULE(ThisModule)
TEST_MODULE(4, ThisModule, gwszModuleDescrip)
	TEST_CASE(1, TCCreateIndex)
	TEST_CASE(2, TCDropIndex)
	TEST_CASE(3, TCTrans)
	TEST_CASE(4, TCRODataSource)
END_TEST_MODULE()
// }} TCW_TESTMODULE_END




// @cmember Display the index columns and orders
BOOL TCIIndexDefinition::DisplayIndex(
	DBINDEXCOLUMNDESC	*rgIndexColumnDesc,	// columns and order in index
	DBORDINAL			nIndexSize						// index size
)
{
	DBORDINAL	i;

	if (!rgIndexColumnDesc)
		return FALSE;

	for (i=0; i<nIndexSize; i++)
	{
		if (rgIndexColumnDesc[i].pColumnID && rgIndexColumnDesc[i].pColumnID->uName.pwszName)
			odtLog << "\t" << rgIndexColumnDesc[i].pColumnID->uName.pwszName;
		else
			odtLog << "\t NULL";
		odtLog << ((DBINDEX_COL_ORDER_ASC == rgIndexColumnDesc[i].eIndexColOrder)? " (Asc)": " (Desc)");
	}
	odtLog << "\n";

	return TRUE;
} //TCIIndexDefinition::DisplayIndex




//-------------------------------------------------------------------------
//
// @cmember Iterator on indexes (as col no)
//-------------------------------------------------------------------------
int TCIIndexDefinition::GetNextIndex(
	DBORDINAL	*rgIndexColumns,	// array containing the column numbers of the index
	BOOL		*rgColumnPresent,	// array of flags for membership to the index
	DBORDINAL	nIndexSize,			// index size
	DBORDINAL	nColumns			// number of columns
)
// client is suppose to initialize rgIndexColumns with the first index
// and rgColumnPresent accordingly
// a return value less than 0 means end of index sequence
{
	ULONG		l;
	DBORDINAL	pos;
	int			nLastUpdate;

	for (nLastUpdate=(int)nIndexSize-1; nLastUpdate>=0; )
	{
		// try to set the next value for position nLastUpdate of the index
		rgColumnPresent[rgIndexColumns[nLastUpdate]] = FALSE;
		// find the first column available
		for (pos=rgIndexColumns[nLastUpdate]+1; pos<=nColumns && rgColumnPresent[pos]; pos++);
		if (pos<=nColumns)
		{
			// set position nLastUpdate in index to this column
			rgIndexColumns[nLastUpdate]	= pos;
			rgColumnPresent[pos]	= TRUE;
			// set all the column in the index bigger than nLastUpdate to min values (guaranted success)
			for (l=(ULONG)nLastUpdate+1; l<nIndexSize; l++)
			{
				for (pos=1; rgColumnPresent[pos]; pos++);
				rgIndexColumns[l]	= pos;
				rgColumnPresent[pos]	= TRUE;
			}
			// we have a new valid index, so flee!
			return nLastUpdate;
		}
		else
		{
			// could not make any pick for this column, must try to modify the previous one first
			nLastUpdate--;
		}
	}
	return nLastUpdate;
} //TCIIndexDefinition::GetNextIndex


//-------------------------------------------------------------------------
//
// @cmember Iterator on order (ascending/descending)
//-------------------------------------------------------------------------
int	TCIIndexDefinition::GetNextOrder(
		DBINDEX_COL_ORDER	*rgIndexOrder,	// array containing the column numbers of the index
		DBORDINAL			nIndexSize		// index size
)
{
	ULONG	l;
	int		nLastUpdate;

	for (nLastUpdate=(int)nIndexSize-1; nLastUpdate>=0; )
	{
		// try to set the next value for position i of the index
		rgIndexOrder[nLastUpdate] = (DBINDEX_COL_ORDER_ASC == rgIndexOrder[nLastUpdate]) ?
					DBINDEX_COL_ORDER_DESC: DBINDEX_COL_ORDER_ASC;
		if (DBINDEX_COL_ORDER_DESC == rgIndexOrder[nLastUpdate])
		{
			// set all the column in the index bigger than i to min values (guaranted success)
			for (l=(ULONG)nLastUpdate+1; l<nIndexSize; l++)
			{
				rgIndexOrder[l]	= DBINDEX_COL_ORDER_ASC;
			}
			// we have a new valid index, so flee!
			return nLastUpdate;
		}
		else
		{
			// one step back
			nLastUpdate--;
		}
	}
	return nLastUpdate;
} //TCIIndexDefinition::GetNextOrder




/*//-------------------------------------------------------------------------
//
// @cmember Drops the index and releases its DBID
//-------------------------------------------------------------------------
BOOL TCIIndexDefinition::DropIndexAndReleaseID(
	DBID	*pTableID, 
	DBID	**ppIndexID
)
{
	BOOL	fRes = FALSE;
	BOOL	fExist;

	TESTC(NULL != pTableID);
	TESTC(NULL != ppIndexID);
	TESTC(NULL != *ppIndexID);

	if (CHECK(DoesIndexExist(pTableID, *ppIndexID, &fExist), S_OK) && !fExist)
	{
		fRes = TRUE;
		goto CLEANUP0;
	}

	if (CHECK(m_pIIndexDefinition->DropIndex(pTableID, *ppIndexID), S_OK))
	{
		if (CHECK(DoesIndexExist(pTableID, *ppIndexID, &fExist), S_OK) && !fExist)
			fRes = TRUE;
		else
			odtLog << "ERROR: index not dropped \n";
	}

CLEANUP0:
	if (ppIndexID && *ppIndexID)
	{
		ReleaseDBID(*ppIndexID);
		*ppIndexID = NULL;
	}
CLEANUP:
	return fRes;
} //TCIIndexDefinition::DropIndexAndReleaseID
*/



//-------------------------------------------------------------------------
//
// @cmember Sets an index with 1 column, then drops the index
// tries all the columns if necessary to determine a column on which
// an index could be created
//-------------------------------------------------------------------------
/*
HRESULT TCIIndexDefinition::SetIndex(
	DBID	*pIndexID,		// [in]  index ID
	ULONG	*nIndex,		// [out] index column 
	DBID	**ppIndexID		// [out] index ID created by IIndexDefinition::CreateIndex()
)
{
	return SetIndex(&(m_pTable->GetTableID()), pIndexID, nIndex, ppIndexID);
} //TCIIndexDefinition::SetIndex
*/


// @cmember Sets an index with 1 column
// tries all the columns if necessary
HRESULT TCIIndexDefinition::SetIndex(
	DBID	*pTableID,		// [in] table to which to attach the index
	DBID	*pIndexID,		// [in]  index ID
	ULONG	*pnIndex,		// [out] index column 
	DBID	**ppIndexID		// [out] index ID created by IIndexDefinition::CreateIndex()
)
{
	ULONG				i;
	DBINDEXCOLUMNDESC	rgIndexColumnDesc[1];
	HRESULT				hr = E_FAIL;

	TESTC(NULL != pTableID);
	rgIndexColumnDesc[0].eIndexColOrder = DBINDEX_COL_ORDER_ASC;
	for (i=0; i<m_cColumnDesc; i++)
	{
		rgIndexColumnDesc[0].pColumnID = &m_rgColumnDesc[i].dbcid;
		if (ppIndexID)
			*ppIndexID = NULL;
		ASSERT(m_pCIIndexDefinition);
		if (S_OK == (hr = m_pCIIndexDefinition->CreateIndexAndCheck(pTableID, pIndexID, 1, rgIndexColumnDesc, 0, NULL, ppIndexID)))
		{
			if (pnIndex)
				*pnIndex = i+1;
			return hr;
		}
		// display info
		odtLog << "could not create index: ";
		DisplayIndex(rgIndexColumnDesc, 1);
	}
CLEANUP:
	return hr;
} //TCIIndexDefinition::SetIndex


//-------------------------------------------------------------------------
//
// @cmember Builds and tries all indexes on a table
// returns S_OK on success
//-------------------------------------------------------------------------
HRESULT	TCIIndexDefinition::DoAllIndexes(
	CTable		*pTable,		// table 
	DBORDINAL	nCol		// number of columns in index
)
{
	DBORDINAL	nIndexSize;
	HRESULT		hr = S_OK;

	for (nIndexSize=1; nIndexSize<=nCol; nIndexSize++)
	{
		if (S_OK !=  (m_hr = DoAllKIndexes(pTable, nIndexSize, nCol) ))
			hr = m_hr;
	}
	return hr;
} //TCIIndexDefinition::DoAllIndexes


// @cmember Builds and tries all indexes on a table
// returns S_OK on success
HRESULT	TCIIndexDefinition::DoAllIndexes(
	CTable		*pTable,			// table 
	DBORDINAL	nCol,				// number of columns in table
	DBORDINAL	nMaxIndexSize		// maximum index size
)
{
	DBORDINAL	nIndexSize;
	HRESULT		hr = S_OK;

	for (nIndexSize=1; nIndexSize<=nMaxIndexSize; nIndexSize++)
	{
		if (S_OK !=  (m_hr = DoAllKIndexes(pTable, nIndexSize, nCol) ))
			hr = m_hr;
	}
	return hr;
} //TCIIndexDefinition::DoAllIndexes


//-------------------------------------------------------------------------
//
// @cmember Builds and tries all indexes of a given size defined upon a table
// return	S_OK on success (results are consistent)
//			E_FAIL if something was wrong
//-------------------------------------------------------------------------
HRESULT	TCIIndexDefinition::DoAllKIndexes(
		CTable		*pTable,			// table 
		DBORDINAL	nIndexSize,			// index size
		DBORDINAL	nCols				// number of columns in table
)
{
	HRESULT				hr					= E_FAIL;
	DBORDINAL			*rgIndexColumns		= NULL;
	BOOL				*rgColumnPresent	= NULL;
	int					nLastUpdate;
	DBORDINAL			i;
	int					iOrder;
	DBINDEX_COL_ORDER	*rgOrder			= NULL;
	DBINDEXCOLUMNDESC	*rgIndexColumnDesc	= NULL;
	DBCOLUMNDESC		*rgColumnDesc		= NULL;
	DBORDINAL			cColumnDesc			= 0;

	TESTC(NULL != pTable);
	SAFE_ALLOC(rgIndexColumns, DBORDINAL, nIndexSize);
	SAFE_ALLOC(rgOrder, DBINDEX_COL_ORDER, nIndexSize);
	SAFE_ALLOC(rgColumnPresent, BOOL, nCols+1);

	// get the column descriptors on the table
	pTable->BuildColumnDescs(&rgColumnDesc);
	cColumnDesc = pTable->CountColumnsOnTable();

	hr = S_OK;

	for (i=0; i<=nCols; rgColumnPresent[i++] = FALSE);
	for (i=0; i<nIndexSize; i++)
	{
		rgIndexColumns[i] = i+1;
		rgColumnPresent[i+1] = TRUE;
	}

	for (nLastUpdate=(int)nIndexSize-1; nLastUpdate >= 0;)
	{
		// initialize the array of column ordering
		for (i=0; i< nIndexSize; rgOrder[i++]=0);
		// try all combination for ordering
		for (iOrder=(int)nIndexSize; iOrder>=0; )
		{
			// prepare parameters
			MakeIndexColumnDesc(rgColumnDesc, cColumnDesc, rgIndexColumns, rgOrder, nIndexSize, &rgIndexColumnDesc);		

			ASSERT(m_pCIIndexDefinition);
			m_hr = m_pCIIndexDefinition->CreateCheckAndDropIndex(&(pTable->GetTableID()),  
					NULL, nIndexSize, rgIndexColumnDesc, 0, NULL);

			if (!SUCCEEDED(m_hr))
			{
				odtLog << "could not create index: ";
				DisplayIndex(rgIndexColumnDesc, nIndexSize);
			}

			// release memory
			ReleaseIndexColumnDesc(rgIndexColumnDesc, nIndexSize);

			// get the next ordering
			iOrder = GetNextOrder(rgOrder, nIndexSize);
		}
		// pick the next column configuration
		nLastUpdate=GetNextIndex(rgIndexColumns, rgColumnPresent, nIndexSize, nCols);
	}

CLEANUP:
	ReleaseColumnDesc(rgColumnDesc, cColumnDesc);
	SAFE_FREE(rgIndexColumns);
	SAFE_FREE(rgColumnPresent);
	SAFE_FREE(rgOrder);
	return hr;
}//TCIIndexDefinition::DoAllKIndexes




//-------------------------------------------------------------------------
// @cmember Find the list of the indexable columns
//
//-------------------------------------------------------------------------
BOOL TCIIndexDefinition::FindIndexableColumns(
	DBCOLUMNDESC	*rgColumnDesc,		// [IN]		array of column desc on the new table
	DBORDINAL		cColumnDesc,		// [IN]		number of column desc
	DBORDINAL		**prgIndexCols,		// [OUT]	array of indexable columns
	DBORDINAL		*cIndexCols			// [OUT]	number of indexable columns
)
{
	BOOL		fResult = FALSE;
	BOOL		fFind;
	CCol		col;
	WCHAR		*pwszTypeName;
	ULONG		nOrdinal, nIndex;
	DBORDINAL	*rgIndexCols = NULL;

	TESTC((NULL != cIndexCols) && (NULL != prgIndexCols));
	
	SAFE_ALLOC(rgIndexCols, DBORDINAL, cColumnDesc);
	*prgIndexCols = rgIndexCols;

	for (nOrdinal = 0; nOrdinal < cColumnDesc; nOrdinal++)
	{
		if (NULL == rgColumnDesc[nOrdinal].pwszTypeName)
			continue;
		// check whether the current column is indexable
		// to do this, search in m_ColList all indexable types
		// use m_rgIndexCols and cIndexCols for this
		fFind = FALSE;
		for (nIndex = 0; nIndex < m_cIndexCols && !fFind; nIndex++)
		{
			// get the type name of the next indexable type
			col			 = m_pTable->
				GetColInfoForUpdate(m_rgIndexCols[nIndex]);
			pwszTypeName = col.GetProviderTypeName();
			if (NULL == pwszTypeName)
				continue;
			// check if it's the current type
			if (0 == wcscmp(rgColumnDesc[nOrdinal].pwszTypeName, pwszTypeName))
			{
				// found an indexable column
				rgIndexCols[(*cIndexCols)++] = nOrdinal+1;
				fFind = TRUE;
			}
		}
	}

	fResult = TRUE;

CLEANUP:
	return fResult;
} //TCIIndexDefinition::FindIndexableColumns




//-------------------------------------------------------------------------
//
// @cmember Prepare DBINDEXCOLUMNDESC array 
// RETURN
//	TRUE	- function executed ok and memory was properly allocated
//	FALSE	- otherwise
//-------------------------------------------------------------------------
BOOL TCIIndexDefinition::MakeIndexColumnDesc(
	DBCOLUMNDESC		*rgColumnDesc,		// [IN]		the array of column desc
	DBORDINAL			cColumnDesc,		// [IN]		the number of columns in the table
	DBORDINAL			*rgIndexCol,		// [IN]		the array of the column in the index
	DBINDEX_COL_ORDER	*rgOrder,			// [IN]		the array of column order
	DBORDINAL			nIndexSize,			// [IN]		index size
	DBINDEXCOLUMNDESC	**prgIndexColumnDesc// [OUT]	the desc of the index cols
)
{
	BOOL				fRes=FALSE;
	ULONG				nIndexCol;
	DBINDEXCOLUMNDESC	*rgIndexColumnDesc = NULL;

	TESTC(NULL != prgIndexColumnDesc);
	*prgIndexColumnDesc = NULL;
	SAFE_ALLOC(*prgIndexColumnDesc, DBINDEXCOLUMNDESC, nIndexSize);
	rgIndexColumnDesc = *prgIndexColumnDesc;
	for (nIndexCol = 0; nIndexCol < nIndexSize; nIndexCol++)
	{
		TESTC(rgIndexCol[nIndexCol] <= cColumnDesc);
		SAFE_ALLOC(rgIndexColumnDesc[nIndexCol].pColumnID, DBID, 1);
		DuplicateDBID(rgColumnDesc[rgIndexCol[nIndexCol]-1].dbcid, rgIndexColumnDesc[nIndexCol].pColumnID);
		rgIndexColumnDesc[nIndexCol].eIndexColOrder = rgOrder[nIndexCol];
	}

	fRes = TRUE;

CLEANUP:
	return fRes; 
} //TCIIndexDefinition::MakeIndexColumnDesc




//-------------------------------------------------------------------------
//
// @cmember Releases the memory allocated for the structure
// RETURN:
//	TRUE	- everything OK
//	FALSE	- bad argument
//-------------------------------------------------------------------------
BOOL TCIIndexDefinition::ReleaseIndexColumnDesc(
	DBINDEXCOLUMNDESC	*rgIndexColumnDesc,	// [IN]	the desc of the index cols
	DBORDINAL			nIndexSize			// [IN]		index size
)
{
	BOOL		fRes = TRUE;
	DBORDINAL	nCol;

	TESTC(NULL != rgIndexColumnDesc);
	for (nCol = 0; nCol < nIndexSize; nCol++)
		ReleaseDBID(rgIndexColumnDesc[nCol].pColumnID);	
	SAFE_FREE(rgIndexColumnDesc);
	fRes = TRUE;

CLEANUP:
	return fRes;
} //TCIIndexDefinition::ReleaseIndexColumnDesc



//-------------------------------------------------------------------------
//
// @cmember release the property of all columns
// RETURNS:
//	TRUE	- function executed correctly
//	FALSE	- parameters look strange
//-------------------------------------------------------------------------
BOOL TCIIndexDefinition::ReleaseAllColumnPropSets(
	DBORDINAL		cColumnDesc,		// [in]	number of columns
	DBCOLUMNDESC	*rgColumnDesc		// [in]	column descriptor array
)
{
	DBORDINAL	i;
	BOOL		fRes = FALSE;

	TESTC(NULL != rgColumnDesc);
	TESTC(0 != cColumnDesc);
	for (i=0; i<cColumnDesc; i++)
	{
		FreeProperties(&rgColumnDesc[i].cPropertySets,
						&rgColumnDesc[i].rgPropertySets);
	}

	fRes = TRUE;

CLEANUP:
	return fRes;
} //TCIIndexDefinition::ReleaseAllColumnPropSets





/*//-------------------------------------------------------------------------
//
// @cmember create an index out of index column descriptors
// 
// RETURNS:
//		E_FAIL	if there is a consistency error 
//		hr of the IIndexDefinition::CreateIndex() otherwise
//-------------------------------------------------------------------------
HRESULT	TCIIndexDefinition::CreateAndCheckIndex(
	DBID				*pTableID,				// [in]		the ID of the table
	DBID				*pIndexID,				// [in]		the ID of the index
	DBID				**ppIndexID,			// [in/out]	stores output ptr to IndexID
	DBORDINAL			nIndexSize,				// [in]		index size (no of columns)
	DBINDEXCOLUMNDESC	*rgIndexColumnDesc,		// [in]		columns to be part of index 
	ULONG				cPropSets,				// [in]		count of property sets 
	DBPROPSET			*rgPropSets				// [in]		array of property sets
)
{
	DBID				*pNewIndexID = NULL;	// DBID of the newly created index
	DBPROPSET			*rgPropSets1 = NULL;	// saves the property sets
	HRESULT				hr;
	HRESULT				hres;
	BOOL				fExists;
	BOOL				fIndexAlreadyExists = FALSE;;
	ULONG				nIndexPropSet;
	ULONG				nIndexProp;
	DBPROP				*pProp;

	// save the array of property sets, as asked
	DuplicatePropertySets(cPropSets, rgPropSets, &cPropSets, &rgPropSets1);

	if (ppIndexID)
		*ppIndexID = NULL;

	if (pIndexID && CHECK(DoesIndexExist(pTableID, pIndexID, &fExists), S_OK) && fExists)
		fIndexAlreadyExists = TRUE;

	// create the index according the requirements
	hres	= m_pIIndexDefinition->CreateIndex(	pTableID, pIndexID, nIndexSize, rgIndexColumnDesc, 
											cPropSets, rgPropSets, ppIndexID);
	hr		= hres;	// hr will be used to build the return value

	PRVTRACE("TCIIndexDefinition::CreateAndCheckIndex: IIndexDefinition::CreateIndex returns %d\n", hr);

	// get an DBID of the index
	if ((NULL != pIndexID) || (NULL != ppIndexID))
		pNewIndexID = pIndexID? pIndexID: *ppIndexID;
	else
	{
		if (!CHECK(hr, E_INVALIDARG))
			hr = E_FAIL;
		goto CLEANUP;
	}

	// check index property status
	if (rgPropSets)
	{
		for (nIndexPropSet=0; nIndexPropSet < cPropSets; nIndexPropSet++)
		{
			pProp = rgPropSets[nIndexPropSet].rgProperties;
			if (!pProp)
				continue;
			for (nIndexProp=0; nIndexProp < rgPropSets[nIndexPropSet].cProperties; nIndexProp++)
			{
				if (!CheckProperty(rgPropSets[nIndexPropSet].guidPropertySet, &pProp[nIndexProp], hres))
					hr = E_FAIL;
			}
		}
	}

	if (SUCCEEDED(hr))
	{
		if (fIndexAlreadyExists)
		{
			odtLog << "ERROR: success in creating an already existing index\n";
			hr = E_FAIL;
			goto CLEANUP;
		}

		// the index is reported to be created, check it thoroughfully
		if (	NULL == pNewIndexID 
			||	!CHECK(m_hr = DoesIndexExist(pTableID, pNewIndexID, &fExists), S_OK) 
			||	!fExists
		   )
		{
			odtLog << "ERROR: the method succeeded, but the index was not created or ppIndexID was not created!\n";
			hr = E_FAIL;
		}
		else
		{
			// check the columns, their collation and the index properties on
			// IDBSchemaRowset or IRowsetIndex
			if (!CHECK(m_hr = CheckIndex(pTableID, pNewIndexID, nIndexSize, rgIndexColumnDesc,
				cPropSets, rgPropSets), TRUE))
			{
				odtLog << "ERROR: the index was not properly created!\n";
				hr = E_FAIL;
			}
			// check the properties as returned
			if (!(m_hr = IsPropSetPreserved(rgPropSets1, rgPropSets, cPropSets)))
			{
				odtLog << "ERROR: properties are not preserved!\n";
				hr = E_FAIL;
			}
		}
		
		// compare *pIndexID and **ppIndexID
		if (!CheckReturnedDBID(pIndexID, ppIndexID))
		{
			odtLog << "the returned IndexID was not created ok!\n";
			hr = E_FAIL;
		}
	
		if (E_FAIL == hr)
		{
			// drop the index and return E_FAIL
			CHECK(m_pIIndexDefinition->DropIndex(pTableID, pNewIndexID), S_OK);
			if (ppIndexID && *ppIndexID)
			{
				ReleaseDBID(*ppIndexID);
				*ppIndexID = NULL;
			}
		}
	}
	else
	{
		// failed to create the index
		if (!pIndexID && DB_E_DUPLICATEINDEXID == m_hr)
		{
			odtLog << "ERROR: DB_E_DUPLICATEINDEXID return on a NULL pIndexID!\n";
			hr = E_FAIL;
		}
		// check that the index is not created
		if (	!fIndexAlreadyExists 
			&&	NULL != pNewIndexID 
			&&	(!CHECK(m_hr = DoesIndexExist(pTableID, pNewIndexID, &fExists), S_OK) || fExists)
			)
		{
			odtLog << "ERROR: the method failed, but the index was created!\n";
			CHECK(m_pIIndexDefinition->DropIndex(pTableID, pNewIndexID), S_OK);
			hr = E_FAIL;
		}
		// should not allocate memory for the index ID
		if (ppIndexID && *ppIndexID)
		{
			odtLog << "ERROR: the index was not created, but *ppIndexID was returned not null!\n";
			ReleaseDBID(*ppIndexID);
			*ppIndexID = NULL;
			hr = E_FAIL;
		}
	}

CLEANUP:
	// release the old properties
	FreeProperties( &cPropSets, &rgPropSets1);
	return hr;
} //TCIIndexDefinition::CreateAndCheckIndex




//-------------------------------------------------------------------------
//
// @cmember create an index out of index column descriptors
// Drops the index and returns just the result of the operation
// 
// RETURNS:
//		E_FAIL	if there is a consistency error 
//		hr of the IIndexDefinition::CreateIndex() otherwise
//-------------------------------------------------------------------------
HRESULT TCIIndexDefinition::CreateCheckAndDropIndex(
	DBID				*pTableID,				// [in]		the ID of the table
	DBID				*pIndexID,				// [in]		the ID of the index
	DBORDINAL			nIndexSize,				// [in]		index size (no of columns)
	DBINDEXCOLUMNDESC	*rgIndexColumnDesc,		// [in]		columns to be part of index 
	ULONG				cPropSets,				// [in]		count of property sets 
	DBPROPSET			*rgPropSets				// [in]		array of property sets
)
{
	DBID		*pOutIndexID=NULL;
	HRESULT		hr = E_FAIL;
	BOOL		fExists;

	hr = CreateAndCheckIndex(pTableID, pIndexID, &pOutIndexID, nIndexSize, 
									rgIndexColumnDesc, cPropSets, rgPropSets);
	if (E_FAIL == hr || !pOutIndexID)
		goto CLEANUP;

	// check that the index is not created
	if (	!CHECK(m_hr = DoesIndexExist(pTableID, pOutIndexID, &fExists), S_OK) 
		||	!fExists
		||	!DropIndexAndReleaseID(pTableID, &pOutIndexID)
	   )
		hr = E_FAIL;

CLEANUP:
	return hr;
} //TCIIndexDefinition::CreateCheckAndDropIndex
*/



//-------------------------------------------------------------------------
//
// @cmember create an index out of column ordinals
// 
// 
// RETURNS:
//		E_FAIL	if there is a consistency error 
//		S_OK	if function completes ok
//-------------------------------------------------------------------------
HRESULT TCIIndexDefinition::CNCIndexFromOrdinals(
	CTable				*pTable,				// [in]		the ID of the table
	DBID				*pIndexID,				// [in]		the ID of the index
	DBID				**ppIndexID,			// [out]	ID of the newly created index
	DBCOLUMNDESC		*rgColumnDesc,			// [in]		array of column descriptors
	DBORDINAL			cColumnDesc,			// [in]		number of column desc
	DBORDINAL			nIndexSize,				// [in]		index size (no of columns)
	DBORDINAL			*rgIndexColumns,		// [in]		columns to be part of index 
	DBINDEX_COL_ORDER	*rgOrder,				// [in]		order on index columns
	ULONG				cPropSets, /*=0*/		// [in]		count of property sets 
	DBPROPSET			*rgPropSets /*=NULL*/	// [in]		array of property sets
)
{
	DBINDEXCOLUMNDESC	*rgIndexColumnDesc;
	HRESULT				hr = E_FAIL;

	// prepare parameters
	TESTC(MakeIndexColumnDesc(rgColumnDesc, cColumnDesc, 
			rgIndexColumns, rgOrder, nIndexSize, &rgIndexColumnDesc));		

	ASSERT(m_pCIIndexDefinition);
	hr = m_pCIIndexDefinition->CreateIndexAndCheck(&pTable->GetTableID(), pIndexID, 
		nIndexSize, rgIndexColumnDesc, cPropSets, rgPropSets, ppIndexID);
			
	// release memory
	if (!ReleaseIndexColumnDesc(rgIndexColumnDesc, nIndexSize))
		hr = E_FAIL;
CLEANUP:
	return hr;
} //TCIIndexDefinition::CNCIndexFromOrdinals




//-------------------------------------------------------------------------
//
// @cmember create an index out of column ordinals
//			Drops the index afterwards
// 
// 
// RETURNS:
//		E_FAIL	if there is a consistency error 
//		S_OK	hr of the IIndexDefinition::CreateIndex() otherwise
//-------------------------------------------------------------------------
HRESULT TCIIndexDefinition::CCNDropIndexFromOrdinals(
	CTable				*pTable,				// [in]		the ID of the table
	DBID				*pIndexID,				// [in]		the ID of the index
	DBCOLUMNDESC		*rgColumnDesc,			// [in]		array of column descriptors
	DBORDINAL			cColumnDesc,			// [in]		number of column desc
	DBORDINAL			nIndexSize,				// [in]		index size (no of columns)
	DBORDINAL			*rgIndexColumns,		// [in]		columns to be part of index 
	DBINDEX_COL_ORDER	*rgOrder,				// [in]		order on index columns
	ULONG				cPropSets, /*=0*/		// [in]		count of property sets 
	DBPROPSET			*rgPropSets /*=NULL*/	// [in]		array of property sets
)
{
	DBINDEXCOLUMNDESC	*rgIndexColumnDesc;
	HRESULT				hr = E_FAIL;

	// prepare parameters
	TESTC(MakeIndexColumnDesc(rgColumnDesc, cColumnDesc, 
			rgIndexColumns, rgOrder, nIndexSize, &rgIndexColumnDesc));		

	TESTC(NULL != m_pCIIndexDefinition);
	hr = m_pCIIndexDefinition->CreateCheckAndDropIndex(&(pTable->GetTableID()), pIndexID, 
		nIndexSize, rgIndexColumnDesc, cPropSets, rgPropSets);
			
	// release memory
	if (!ReleaseIndexColumnDesc(rgIndexColumnDesc, nIndexSize))
		hr = E_FAIL;
CLEANUP:
	return hr;
} //TCIIndexDefinition::CCNDropIndexFromOrdinals




//--------------------------------------------------------------------------
//
// @cmember Check the result and property status
//--------------------------------------------------------------------------
BOOL TCIIndexDefinition::CheckProperty(
	const GUID	guidPropertySet,// [in]	property set guid (should be DBPROPSET_INDEX
	DBPROP		*pProp,			// [in]	property
	HRESULT		hr				// [in] the result
)
{
	BOOL		fRes = TRUE;
	BOOL		fSupported;
	BOOL		fSettable;

	if (!pProp)
		return TRUE;
	
	// general checking
	if (DBPROPOPTIONS_REQUIRED == pProp->dwOptions && DBPROPSTATUS_OK != pProp->dwStatus)
		fRes = CHECK(FAILED(hr), TRUE);

	if (!(guidPropertySet == DBPROPSET_INDEX))
	{
		// check a couple of things and returns
		return COMPARE(pProp->dwStatus, DBPROPSTATUS_NOTSUPPORTED) && COMPARE(S_OK != hr, TRUE);
	}

	fSupported	= SupportedProperty(pProp->dwPropertyID, guidPropertySet);
	fSettable	= SettableProperty(pProp->dwPropertyID, guidPropertySet);

	if (!fSupported && !CHECK(S_OK != hr, TRUE))
		fRes = FALSE;
		
	// status driven checking
	switch (pProp->dwStatus)
	{
		case DBPROPSTATUS_OK:
			fRes = COMPARE(fSupported != 0, TRUE);
			break;
		case DBPROPSTATUS_NOTSUPPORTED:
			fRes = COMPARE(fSupported == 0, TRUE);
			break;
		case DBPROPSTATUS_NOTSETTABLE:
			fRes = COMPARE(fSupported != 0, TRUE) && COMPARE(fSettable == 0, TRUE);
			break;
		case DBPROPSTATUS_BADOPTION:
			fRes =	(pProp->dwOptions != DBPROPOPTIONS_OPTIONAL) 
				&&	COMPARE(DBPROPOPTIONS_REQUIRED != pProp->dwOptions, TRUE);
			break;
		case DBPROPSTATUS_CONFLICTING:
			fRes = COMPARE(fSettable != 0, TRUE);
			break;
		case DBPROPSTATUS_BADVALUE:
			fRes = COMPARE(fSupported != 0, TRUE);
			break;
		default:
			break;
	}

	// DBPROPSTATUS_BADVALUE can be certainly got only if type of the prop is wrong
	// info about supported discret values of the prop cannot generally be obtained 
	// before creating an index
	if (	GetPropInfoType(pProp->dwPropertyID, guidPropertySet) != pProp->vValue.vt
		&&	!COMPARE(DBPROPSTATUS_OK == pProp->dwStatus, FALSE))
	{
			odtLog << "ERROR: bad propstatus on improper type value on property" << pProp->dwPropertyID << "\n";
			fRes = FALSE;
	}

	return fRes;
} //TCIIndexDefinition::CheckProperty




/*//-------------------------------------------------------------------------
//
// @cmember Check whether all the columns specified appear in index
// and whether the properties are properly set
// uses IRowsetIndex
//-------------------------------------------------------------------------
BOOL TCIIndexDefinition::CheckIndex2(
		DBID				*pTableID,				// the index of the table
		DBID				*pIndexID,				// the index to be checked
		DBORDINAL			cIndexColumnDesc,		// how many elements
		DBINDEXCOLUMNDESC	*rgIndexColumnDesc,		// array with index original descriprion
		ULONG				cPropertySets,			// number of property sets
		DBPROPSET			*rgPropertySets			// the array of property sets
)
{
	IRowsetIndex		*pIRowsetIndex = NULL;
	ULONG				i;
	DBORDINAL			cColumns = 0;
	ULONG				cIndexPropSets=0, nPropSet, nProp;
	DBINDEXCOLUMNDESC	*rgOutIndexColumnDesc = NULL;
	DBPROPSET			*rgIndexPropSets = NULL;
	BOOL				fResult = TRUE, fProp=TRUE;
	DBPROP				*pProp=NULL;
	DBPROPSTATUS		dwStatus;
	VARIANT				*pvValue;
	DBPROPID			dwPropID;

	TESTC_(m_hr = m_pIOpenRowset->OpenRowset(NULL, pTableID, pIndexID, 
		IID_IRowsetIndex, 0, NULL, (IUnknown**)&pIRowsetIndex), S_OK);
	fResult = FALSE;

	// use IRowsetIndex->GetIndexInfo to check how the index was set

	TESTC_(pIRowsetIndex->GetIndexInfo(&cColumns, &rgOutIndexColumnDesc, &cIndexPropSets, &rgIndexPropSets), S_OK);
	TESTC(cColumns == cIndexColumnDesc);

	for (i=0; i<cIndexColumnDesc; i++)
	{
		if (rgIndexColumnDesc[i].eIndexColOrder != rgOutIndexColumnDesc[i].eIndexColOrder)
		{
			odtLog << "order (A/D) differ for column " << rgIndexColumnDesc[i].pColumnID->uName.pwszName << "\n";
			goto CLEANUP;
		}
		// this is based on the assumption that DBID returned for columns are in the same
		// format as they were input
		// otherwise one have to use a complete description of the columns in the table and make the conversion
		if (!CompareDBID(*rgIndexColumnDesc[i].pColumnID, *rgOutIndexColumnDesc[i].pColumnID, m_pIDBInitialize))
		{
			odtLog << "column ID different for column " << rgIndexColumnDesc[i].pColumnID->uName.pwszName << "\n";
			goto CLEANUP;
		}
	}

	fResult = TRUE;
	if (!rgPropertySets)
		goto CLEANUP;

	for (nPropSet=0; nPropSet<cPropertySets; nPropSet++)
	{
		if (DBPROPSET_INDEX != rgPropertySets[nPropSet].guidPropertySet)
		{
			for (nProp=0; nProp<rgPropertySets[nPropSet].cProperties; nProp++)
			{
				if (!COMPARE(rgPropertySets[nPropSet].rgProperties[nProp].dwStatus, DBPROPSTATUS_NOTSUPPORTED))
					fProp = FALSE;
			}
			continue;
		}
		if (!rgPropertySets[nPropSet].rgProperties)
			continue;
		for (nProp=0; nProp<rgPropertySets[nPropSet].cProperties; nProp++)
		{
			dwStatus	= rgPropertySets[nPropSet].rgProperties[nProp].dwStatus;
			dwPropID	= rgPropertySets[nPropSet].rgProperties[nProp].dwPropertyID;
			pvValue		= &rgPropertySets[nPropSet].rgProperties[nProp].vValue;

			// try to find coresponding property got from IRowsetIndex
			if (!FindProperty(	dwPropID,
								rgPropertySets[nPropSet].guidPropertySet,
								cIndexPropSets, 
								rgIndexPropSets, 
								&pProp))	
			{
				fProp = fProp && COMPARE(dwStatus, DBPROPSTATUS_NOTSUPPORTED);
				continue;
			}

			// prop status were check in CreateAndCheckIndex by calling CheckProperty
			if (DBPROPSTATUS_OK == dwStatus)
				fProp = fProp && COMPARE(CompareVariant(&pProp->vValue, pvValue), TRUE);
		}
	}

	fResult = fProp;

CLEANUP:
	SAFE_RELEASE(pIRowsetIndex);
	return fResult;
} //TCIIndexDefinition::CheckIndex2




//--------------------------------------------------------------------------
//
//	@cmember Read the value of a column in the index schema rowset
//
//	if *pfSet is TRUE then there is a coparison value for the value read,
// otherwise the new value will be used for further comparisons => save
// it in *pVariable and set *pfSet to TRUE
//	RETURNS FALSE if the value was read and differ than the original one
//--------------------------------------------------------------------------
BOOL TCIIndexDefinition::GetIndexValue(
	LPVOID		pVariable,		// [OUT]	value read
	BOOL		*pfSet,			// [OUT]	if the value is set
	DBBINDING	*rgDBBINDING,	// [IN]		binding array
	ULONG		cColumn,		// [IN]		column to be read
	ULONG		ulDBTYPE,		// [IN]		type of property variant
	BYTE		*pData,			// [IN]		pointer to read DATA stru
	WCHAR		*lpwszMesaj		// [IN]		message text for error
)
{
	BOOL	fRes		= FALSE;
	BOOL	fWStrComp	= FALSE;
	DATA	*pColumn	= NULL;

	TESTC(NULL != pData);
	TESTC(NULL != rgDBBINDING);
	TESTC(NULL != pVariable);
	TESTC(NULL != pfSet);
	fRes = TRUE;

	// get the status of the read property value
	pColumn = (DATA*) (pData+rgDBBINDING[cColumn].obStatus);	
	if (DBSTATUS_S_OK == pColumn->sStatus)							
	{
		// get the value in pVariable
		switch (ulDBTYPE)
		{
			case DBTYPE_I4:
				if (!*pfSet)
				{
					*(LONG*)pVariable = *(LONG*)(&(pColumn->bValue));						
					*pfSet = TRUE;
				}
				else
					fRes =  (*(LONG*)(&(pColumn->bValue)) == *(LONG*)pVariable);	
				break;

			case DBTYPE_UI2:
				if (!*pfSet)
				{
					*(unsigned short*)pVariable = *(unsigned short*)(&(pColumn->bValue));						
					*pfSet = TRUE;
				}
				else
					fRes =  (*(unsigned short*)(&(pColumn->bValue)) == *(unsigned short*)pVariable);
				break;

			case DBTYPE_I2:
				if (!*pfSet)
				{
					*(SHORT*)pVariable = *(SHORT*)(&(pColumn->bValue));						
					*pfSet = TRUE;
				}
				else
					fRes =  (*(SHORT*)(&(pColumn->bValue)) == *(SHORT*)pVariable);
				break;

			case DBTYPE_BOOL:
				if (!*pfSet)
				{
					*(VARIANT_BOOL*)pVariable = *(VARIANT_BOOL*)(&(pColumn->bValue));						
					*pfSet = TRUE;
				}
				else
					fRes =  (*(VARIANT_BOOL*)(&(pColumn->bValue)) == *(VARIANT_BOOL*)pVariable);
				break;

			case DBTYPE_WSTR:
				if (!*pfSet)
				{
					TESTC(NULL != *(WCHAR**)pVariable);
					// release the memory is wrongfully passed
					if (!*(WCHAR*)pVariable)
						SAFE_FREE(*(WCHAR**)pVariable);
					// set the variable 
					*(WCHAR**)pVariable = wcsDuplicate(*(WCHAR**)(&(pColumn->bValue)));						
					*pfSet = TRUE;
				}
				else
					// compare
					fRes =		(NULL != *(WCHAR**)pVariable) 
							&&	S_OK == CompareID(&fWStrComp, *(WCHAR**)pVariable, (WCHAR*)(&(pColumn->bValue)), m_pIDBInitialize)
							&& fWStrComp;
				break;
			default:
				ASSERT(FALSE);
				break;
		}
		if (!fRes)
			odtLog << lpwszMesaj;											
	}

CLEANUP:
	return fRes;
} // TCIIndexDefinition::GetIndexValue



//--------------------------------------------------------------------------
//
//	@cmember Read the value of a column in the indexe schema rowset
//
//	RETURNS FALSE if the value was read and differ than the original one
//--------------------------------------------------------------------------
BOOL TCIIndexDefinition::GetIndexValueFromFirstRow(
	LPVOID		pVariable,		// [OUT]	value read
	BOOL		*pfSet,			// [OUT]	if the value is set
	DBBINDING	*rgDBBINDING,	// [IN]		binding array
	ULONG		cColumn,		// [IN]		column to be read
	ULONG		ulDBTYPE,		// [IN]		type of property variant
	BYTE		*pData,			// [IN]		pointer to read DATA stru
	DBPROPID	PropID,			// [IN]		property that is being read
	ULONG		cPropSets,		// [IN]		number of property sets
	DBPROPSET	*rgPropSets,	// [IN]		array of property sets
	WCHAR		*lpwszMesaj		// [IN]		message text for error
)
{
	BOOL	fRes = TRUE;
	DBPROP	*pProp = NULL;;

	TESTC(NULL != pfSet);

	*pfSet = FALSE;
	FindProperty(PropID, DBPROPSET_INDEX, cPropSets, rgPropSets, &pProp);
	if (!pProp)
		goto CLEANUP;

	// get the variable value
	switch (ulDBTYPE)
	{
		case DBTYPE_BOOL:
			*(VARIANT_BOOL*)pVariable = V_BOOL(&pProp->vValue);
			break;

		case DBTYPE_I4:
			*(LONG*)pVariable = V_I4(&pProp->vValue);
			break;

		case DBTYPE_UI2:
			*(unsigned short*)pVariable = V_UI2(&pProp->vValue);
			break;

		case DBTYPE_I2:
			*(SHORT*)pVariable = V_I2(&pProp->vValue);
			break;
		default:
			TESTC(FALSE);
	}
	*pfSet = (DBPROPSTATUS_OK == pProp->dwStatus);

CLEANUP:
	fRes = GetIndexValue(pVariable, pfSet, rgDBBINDING, cColumn, ulDBTYPE, pData, lpwszMesaj);
	if (DBPROP_INDEX_FILLFACTOR == PropID && fProperRangeForFillFactor)
	{
		if (!COMPARE(0 < *(LONG*)pVariable && *(LONG*)pVariable <= 100, TRUE))
			fProperRangeForFillFactor = FALSE;
	}
	return fRes;
}


//-------------------------------------------------------------------------
//
// @cmember Check whether all the columns specified appear in index
// and the properties are properly set
// returns	TRUE - everything was set ok
//			FALSE - problems
//-------------------------------------------------------------------------
BOOL TCIIndexDefinition::CheckIndex(
		DBID				*pTableID,				// the index of the table
		DBID				*pIndexID,				// the index to be checked
		DBORDINAL			cIndexColumnDesc,		// how many elements
		DBINDEXCOLUMNDESC	*rgIndexColumnDesc,		// array with index original descriprion
		ULONG				cPropertySets,			// number of property sets
		DBPROPSET			*rgPropertySets			// the array of property sets
)
{
	HRESULT 			hr				= E_FAIL;
	BOOL				fReturn			= FALSE;
	BOOL				fProps			= TRUE;
	BOOL				bIsSchemaSupported;
	DBLENGTH			ulRowSize		= 0;		// size of row
	DBCOUNTITEM			cDBBINDING		= 0;		// count of bindings
	DBORDINAL			iRow			= 0;		// count of rows
	DBCOUNTITEM			cRowsObtained	= 0;		// number of rows returned, should be 1
	ULONG				cSchema			= 0;		// number of supported Schemas
	ULONG				*prgRestrictions= 0;		// restrictions for each Schema
	GUID				*prgSchemas		= NULL;		// array of GUIDs
	HROW				hRow;						// handler of rows
	HROW				*phRow = &hRow;				// pointer to handler
	IRowset				*pIndexRowset	= NULL;		// returned rowset
	DBBINDING			*rgDBBINDING	= NULL;		// array of bindings
	BYTE				*pData			= NULL;		// pointer to data
	HACCESSOR 			hAccessor		= NULL;		// accessor
	BOOL				*rgColPresent = NULL;
	const int			cRest = 5;
	VARIANT				rgRestrictIndexes[cRest];
	ULONG				i, index;
	ULONG				nColPresent=0;
	ULONG				nTABLE_NAME			= 2;
	ULONG				nINDEX_NAME			= 5;
	ULONG				nPRIMARY_KEY		= 6;
	ULONG				nUNIQUE				= 7;
	ULONG				nCLUSTERED			= 8;
	ULONG				nTYPE				= 9;
	ULONG				nFILL_FACTOR		= 10;
	ULONG				nINITIAL_SIZE		= 11;
	ULONG				nNULLS				= 12;
	ULONG				nSORT_BOOKMARKS		= 13;
	ULONG				nAUTO_UPDATE		= 14;
	ULONG				nNULL_COLLATION		= 15;
	ULONG				nORDINAL_POSITION	= 16;
	ULONG				nCOLUMN_NAME		= 17;
	ULONG				nCOLLATION			= 20;

	// column entries in index schema rowset and their presence flags
	LPWSTR				pwszIndexName		= NULL;
	BOOL				fIndexName			= FALSE;
	LPWSTR				pwszTableName		= NULL;
	BOOL				fTableName			= FALSE;
	VARIANT_BOOL		vbAutoUpdate;
	BOOL				fAutoUpdate			= FALSE;
	VARIANT_BOOL		vbClustered;
	BOOL				fClustered			= FALSE;
	LONG				lFillFactor;
	BOOL				fFillFactor			= FALSE;
	LONG				lInitialSize;
	BOOL				fInitialSize		= FALSE;
	LONG				lNullCollation;
	BOOL				fNullCollation		= FALSE;
	unsigned short		uiOrdinalPosition;
	BOOL				fOrdinalPosition;
	SHORT				iCollation;
	BOOL				fCollation			= FALSE;
	LPWSTR				pwszColumnName		= NULL;
	BOOL				fColumnName			= FALSE;
	LONG				lNulls;
	BOOL				fNulls				= FALSE;
	VARIANT_BOOL		vbPrimaryKey;
	BOOL				fPrimaryKey			= FALSE;
	VARIANT_BOOL		vbSortBookmarks;
	BOOL				fSortBookmarks		= FALSE;
	unsigned short		usType;
	BOOL				fType				= FALSE;
	VARIANT_BOOL		vbUnique;
	BOOL				fUnique				= FALSE;

	// Set restrictions
	for(index=0;index<cRest;index++)
		VariantInit(&rgRestrictIndexes[index]);

	if (m_fIRowsetIndex)
		TESTC(CheckIndex2(pTableID, pIndexID, cIndexColumnDesc, rgIndexColumnDesc,
					cPropertySets, rgPropertySets));

	if (NULL == m_pIDBSchemaRowset)
	{
		fReturn = TRUE;
		goto CLEANUP;
	}

	// Check to see if the schema is supported
	TESTC_(hr = m_pIDBSchemaRowset->GetSchemas(&cSchema, &prgSchemas, &prgRestrictions),S_OK);

	if (DBKIND_NAME == pIndexID->eKind)
	{
		pwszIndexName = wcsDuplicate(pIndexID->uName.pwszName);
		fIndexName = TRUE;
	}

	if (DBKIND_NAME == pTableID->eKind)
	{
		pwszTableName = wcsDuplicate(pTableID->uName.pwszName);
		fTableName = TRUE;
	}

	// Check to see if DBSCHEMA_INDEXES is supported
	for(i=0, bIsSchemaSupported=FALSE; i<cSchema && !bIsSchemaSupported;)
	{
		if(prgSchemas[i] == DBSCHEMA_INDEXES)
			bIsSchemaSupported = TRUE;
		else 
			i++;
	}

	if(!bIsSchemaSupported || !(prgRestrictions[i] & 0x4) || !(prgRestrictions[i] & 0x10))
	{
		odtLog << "Index Schema Rowset or the required constraints are not supported\n";
		goto CLEANUP;
	}

	rgRestrictIndexes[2].vt 	 = VT_BSTR;
	rgRestrictIndexes[2].bstrVal = SysAllocString(pIndexID->uName.pwszName);
	rgRestrictIndexes[4].vt 	 = VT_BSTR;
	rgRestrictIndexes[4].bstrVal = SysAllocString(pTableID->uName.pwszName);

	SAFE_ALLOC(rgColPresent, BOOL, cIndexColumnDesc);
	for (i=0; i<cIndexColumnDesc; i++)
		rgColPresent[i] = FALSE;

	TESTC_(hr = m_pIDBSchemaRowset->GetRowset(	
			NULL, 								// aggregation
			DBSCHEMA_INDEXES,					// REFGUID
			cRest,	 							// count of restrictions (1:types)
			rgRestrictIndexes,					// list of restrictions
			IID_IRowset,						// REFFID
			0,									// count of properties
			NULL,								// range of properties
			(IUnknown**)&pIndexRowset			// returned result set
		),S_OK);

	TESTC_(hr = GetAccessorAndBindings(
						pIndexRowset, DBACCESSOR_ROWDATA, &hAccessor,	&rgDBBINDING,		
						&cDBBINDING, &ulRowSize, DBPART_VALUE |DBPART_STATUS |DBPART_LENGTH,
						ALL_COLS_EXCEPTBOOKMARK, FORWARD, NO_COLS_BY_REF,
						NULL,					// OUT: Array of DBCOLUMNINFOs
						0,						// OUT: Count of DBCOULMNINFOs
						NULL,					//&pStringsBuffer, 
						DBTYPE_EMPTY, 
						0, NULL),S_OK);
	SAFE_ALLOC(pData, BYTE, ulRowSize);	//data

	// read the first row, check local data and initialize variables
	TESTC_(hr=pIndexRowset->GetNextRows(0, 0, 1, &cRowsObtained, &phRow), S_OK);
	TESTC_((long)cRowsObtained, 1);
	TESTC_(hr=pIndexRowset->GetData(hRow, hAccessor, pData),S_OK);

	// get the first values and verifications
	fProps = fProps && GetIndexValue(&pwszIndexName, &fIndexName, 
		rgDBBINDING, nINDEX_NAME, DBTYPE_WSTR, pData, L"ERROR: Bad index name\n");

	fProps = fProps && GetIndexValue(&pwszTableName, &fTableName, 
		rgDBBINDING, nTABLE_NAME, DBTYPE_WSTR, pData, L"ERROR: Bad table name\n");

	// check column ordinal position of the first column in the index (1 is the first value) and name
	fOrdinalPosition = FALSE;
	fProps = GetIndexValue(&uiOrdinalPosition, &fOrdinalPosition, rgDBBINDING, 
		nORDINAL_POSITION, DBTYPE_UI2, pData, L"ERROR: OrdinalPosition\n") && fProps;

	if (fOrdinalPosition)
	{
		TESTC(cIndexColumnDesc >= uiOrdinalPosition);

		DBID	*pColumnID = rgIndexColumnDesc[uiOrdinalPosition-1].pColumnID;

		// can compare both column name and collation order
		TESTC(NULL != pColumnID);
		fColumnName = TRUE;
		switch (pColumnID->eKind)
		{
			case DBKIND_NAME:
				pwszColumnName = wcsDuplicate(pColumnID->uName.pwszName);
				fProps = fProps && GetIndexValue(&pwszColumnName, &fColumnName, 
					rgDBBINDING, nCOLUMN_NAME, DBTYPE_WSTR, pData, L"ERROR: Column name\n");
				SAFE_FREE(pwszColumnName);
				break;
		}

		fCollation	= TRUE;
		iCollation	=  (short)((DBINDEX_COL_ORDER_ASC == rgIndexColumnDesc[uiOrdinalPosition-1].eIndexColOrder) ?
						DB_COLLATION_ASC: DB_COLLATION_DESC);
		fProps = fProps && GetIndexValue(&iCollation, &fCollation, 
			rgDBBINDING, nCOLLATION, DBTYPE_I2, pData, L"ERROR: Collation\n");

		rgColPresent[uiOrdinalPosition-1] = TRUE;
		nColPresent++;
	}

	if (cPropertySets > 0 && rgPropertySets)
	{
		fProps = fProps && GetIndexValueFromFirstRow(&vbAutoUpdate, &fAutoUpdate, 
			rgDBBINDING, nAUTO_UPDATE, DBTYPE_BOOL, pData, DBPROP_INDEX_AUTOUPDATE, 
			cPropertySets, rgPropertySets, L"ERROR: Auto update\n");

		fProps = fProps && GetIndexValueFromFirstRow(&vbClustered, &fClustered, 
			rgDBBINDING, nCLUSTERED, DBTYPE_BOOL, pData, DBPROP_INDEX_CLUSTERED, 
			cPropertySets, rgPropertySets, L"ERROR: Clustered\n");

		fProps = fProps && GetIndexValueFromFirstRow(&lFillFactor, &fFillFactor, 
			rgDBBINDING, nFILL_FACTOR, DBTYPE_I4, pData, DBPROP_INDEX_FILLFACTOR, 
			cPropertySets, rgPropertySets, L"ERROR: Fill factor\n");

		fProps = fProps && GetIndexValueFromFirstRow(&lInitialSize, &fInitialSize, 
			rgDBBINDING, nINITIAL_SIZE, DBTYPE_I4, pData, DBPROP_INDEX_INITIALSIZE, 
			cPropertySets, rgPropertySets, L"ERROR: Initial Size\n");

		fProps = fProps && GetIndexValueFromFirstRow(&lNullCollation, &fNullCollation, 
			rgDBBINDING, nNULL_COLLATION, DBTYPE_I4, pData, DBPROP_INDEX_NULLCOLLATION,
			cPropertySets, rgPropertySets, L"ERROR: Null Collation\n");

		fProps = fProps && GetIndexValueFromFirstRow(&lNulls, &fNulls, 
			rgDBBINDING, nNULLS, DBTYPE_I4, pData, DBPROP_INDEX_NULLS,
			cPropertySets, rgPropertySets, L"ERROR: Nulls\n");

		fProps = fProps && GetIndexValueFromFirstRow(&vbPrimaryKey, &fPrimaryKey, 
			rgDBBINDING, nPRIMARY_KEY, DBTYPE_BOOL, pData, DBPROP_INDEX_PRIMARYKEY,
			cPropertySets, rgPropertySets, L"ERROR: Primary Key\n");

		fProps = fProps && GetIndexValueFromFirstRow(&vbSortBookmarks, &fSortBookmarks, 
			rgDBBINDING, nSORT_BOOKMARKS, DBTYPE_BOOL, pData, DBPROP_INDEX_SORTBOOKMARKS,
			cPropertySets, rgPropertySets, L"ERROR: SortBookmarks\n");

		fProps = fProps && GetIndexValueFromFirstRow(&usType, &fType, 
			rgDBBINDING, nTYPE, DBTYPE_UI2, pData, DBPROP_INDEX_TYPE,
			cPropertySets, rgPropertySets, L"ERROR: Type\n");

		fProps = fProps && GetIndexValueFromFirstRow(&vbUnique, &fUnique, 
			rgDBBINDING, nUNIQUE, DBTYPE_BOOL, pData, DBPROP_INDEX_UNIQUE,
			cPropertySets, rgPropertySets, L"ERROR: Unique\n");
	}

	TESTC_(pIndexRowset->ReleaseRows(1, &hRow, NULL, NULL, NULL), S_OK);

	// Get data for each row
	for(iRow=1;iRow<cIndexColumnDesc;iRow++)			 
	{
		TESTC_(hr=pIndexRowset->GetNextRows(0, 0, 1, &cRowsObtained, &phRow), S_OK);
		TESTC_((long)cRowsObtained, 1);
		TESTC_(hr=pIndexRowset->GetData(hRow, hAccessor, pData),S_OK);

		// check that the properties are the same:
		fProps = fProps && GetIndexValue(&pwszIndexName, &fIndexName, 
			rgDBBINDING, nINDEX_NAME, DBTYPE_WSTR, pData, L"ERROR: Bad index name\n");

		fProps = fProps && GetIndexValue(&pwszTableName, &fTableName, 
			rgDBBINDING, nTABLE_NAME, DBTYPE_WSTR, pData, L"ERROR: Bad table name\n");

		// check column ordinal and name
		fOrdinalPosition = FALSE;
		fProps = fProps && GetIndexValue(&uiOrdinalPosition, &fOrdinalPosition, 
			rgDBBINDING, nORDINAL_POSITION, DBTYPE_UI2, pData, L"ERROR: OrdinalPosition\n");

		if (fOrdinalPosition)
		{
			TESTC(cIndexColumnDesc >= uiOrdinalPosition);
	
			DBID	*pColumnID = rgIndexColumnDesc[uiOrdinalPosition-1].pColumnID;

			// can compare both column name and collation order
			TESTC(NULL != pColumnID);
			fColumnName = TRUE;
			switch (pColumnID->eKind)
			{
				case DBKIND_NAME:
					pwszColumnName = wcsDuplicate(pColumnID->uName.pwszName);
					fProps = fProps && GetIndexValue(&pwszColumnName, &fColumnName, 
						rgDBBINDING, nCOLUMN_NAME, DBTYPE_WSTR, pData, L"ERROR: Column name\n");
					SAFE_FREE(pwszColumnName);
					break;
				default:
					break;
			}

			fCollation	= TRUE;
			iCollation	= (short)((DBINDEX_COL_ORDER_ASC == rgIndexColumnDesc[uiOrdinalPosition-1].eIndexColOrder) ?
							DB_COLLATION_ASC: DB_COLLATION_DESC);
			fProps = fProps && GetIndexValue(&iCollation, &fCollation, 
				rgDBBINDING, nCOLLATION, DBTYPE_I2, pData, L"ERROR: Collation\n");

			rgColPresent[uiOrdinalPosition-1] = TRUE;
			nColPresent++;
		}

		fProps = fProps && GetIndexValue(&vbAutoUpdate, &fAutoUpdate, 
			rgDBBINDING, nAUTO_UPDATE, DBTYPE_BOOL, pData, L"ERROR: Auto update\n");

		fProps = fProps && GetIndexValue(&vbClustered, &fClustered, 
			rgDBBINDING, nCLUSTERED, DBTYPE_BOOL, pData, L"ERROR: Clustered\n");

		fProps = fProps && GetIndexValue(&lFillFactor, &fFillFactor, 
			rgDBBINDING, nFILL_FACTOR, DBTYPE_I4, pData, L"ERROR: Fill factor\n");

		fProps = fProps && GetIndexValue(&lInitialSize, &fInitialSize, 
			rgDBBINDING, nINITIAL_SIZE, DBTYPE_I4, pData, L"ERROR: Initial Size\n");

		fProps = fProps && GetIndexValue(&lNullCollation, &fNullCollation, 
			rgDBBINDING, nNULL_COLLATION, DBTYPE_I4, pData, L"ERROR: Null Collation\n");

		fProps = fProps && GetIndexValue(&lNulls, &fNulls, 
			rgDBBINDING, nNULLS, DBTYPE_I4, pData, L"ERROR: Nulls\n");

		fProps = fProps && GetIndexValue(&vbPrimaryKey, &fPrimaryKey, 
			rgDBBINDING, nPRIMARY_KEY, DBTYPE_BOOL, pData, L"ERROR: Primary Key\n");

		fProps = fProps && GetIndexValue(&vbSortBookmarks, &fSortBookmarks, 
			rgDBBINDING, nSORT_BOOKMARKS, DBTYPE_BOOL, pData, L"ERROR: SortBookmarks\n");

		fProps = fProps && GetIndexValue(&usType, &fType, 
			rgDBBINDING, nTYPE, DBTYPE_UI2, pData, L"ERROR: Type\n");

		fProps = fProps && GetIndexValue(&vbUnique, &fUnique, 
			rgDBBINDING, nUNIQUE, DBTYPE_BOOL, pData, L"ERROR: Unique\n");

		TESTC_(pIndexRowset->ReleaseRows(1, &hRow, NULL, NULL, NULL), S_OK);
	}


	if ((nColPresent == cIndexColumnDesc) && fProps)
		fReturn = TRUE;

CLEANUP:
	// Free the memory
	SAFE_FREE(pwszIndexName);
	SAFE_FREE(pwszTableName);
	
	SAFE_FREE(rgColPresent);

	SAFE_FREE(prgRestrictions);
	SAFE_FREE(prgSchemas);

	SAFE_FREE(pData);

	SAFE_RELEASE(pIndexRowset);

	SAFE_FREE(rgDBBINDING);

	for(index=0;index<cRest;index++)
		GCHECK(VariantClear(&(rgRestrictIndexes[index])),S_OK);
	
	return fReturn;
} //TCIIndexDefinition::CheckIndex
*/


//-------------------------------------------------------------------------
//
// TCIIndexDefinition
//-------------------------------------------------------------------------
TCIIndexDefinition::TCIIndexDefinition(WCHAR *wstrTestCaseName) : CSessionObject(wstrTestCaseName) 
{
//	m_pIIndexDefinition = NULL;
	m_rgColumnDesc		= NULL;
	m_cColumnDesc		= 0;
	m_pIDBSchemaRowset	= NULL;
//	m_fIRowsetIndex		= FALSE;
	m_pPopulatedTable	= NULL;
	m_rgIndexCols		= NULL;
	m_cIndexCols		= 0;
} // TCIIndexDefinition::TCIIndexDefinition


//*-----------------------------------------------------------------------
// 
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//-------------------------------------------------------------------------
BOOL TCIIndexDefinition::Init()
{ 
	DBINDEXCOLUMNDESC		rgIndexColumnDesc[1];
	DBID					*pIndexID = NULL;
	ULONG					i;
	IRowsetIndex			*pIRowsetIndex = NULL;
	CList <WCHAR*, WCHAR*>	ListNativeTemp;
	CList <DBTYPE, DBTYPE>	ListDataTypes;
	ULONG					cSchema			= 0;		// number of supported Schemas
	ULONG 					*prgRestrictions= NULL;		// restrictions for each Schema
	GUID					*prgSchemas		= NULL;		// array of GUIDs
	VARIANT					vVal;
	IIndexDefinition		*pIIndexDefinition = NULL;

	m_pCIIndexDefinition = NULL;
//	m_pIIndexDefinition = NULL;
	m_pIDBSchemaRowset	= NULL;
	m_pwszInvalidTableChars		= NULL;
	m_pwszInvalidIndexChars		= NULL;
	m_pwszInvalidColumnChars	= NULL;

	if(CSessionObject::Init())
	{ 
		// check for IIndexDefinition
		SetDataSourceObject(m_pThisTestModule->m_pIUnknown, TRUE);
		SetDBSession(m_pThisTestModule->m_pIUnknown2);
		GetLiteralInfo();

		if (!(m_hr=VerifyInterface(m_pIOpenRowset, IID_IIndexDefinition, SESSION_INTERFACE, (IUnknown**)&pIIndexDefinition)))
		{
			odtLog << L"IIndexDefinition is not implemented\n";
			return FALSE;
		}

		//wrap pIIndexDefinition in the helper class
		m_pCIIndexDefinition = new CIIndexDefinition(pIIndexDefinition);

		// check whether IDBSchemaRowset is available
		if (!(m_hr=VerifyInterface(	m_pIOpenRowset, IID_IDBSchemaRowset, SESSION_INTERFACE, 
									(IUnknown**)&m_pIDBSchemaRowset)))
			odtLog << L"IDBSchemaRowset is not implemented\n";
		else
		{
			// check whether it supports INDEXES Schema Rowset and its restrictions
			if (S_OK == (m_hr = m_pIDBSchemaRowset->GetSchemas(&cSchema, &prgSchemas, &prgRestrictions)))
			{
				for(i=0; i<cSchema; i++)
				{
					if(prgSchemas[i] == DBSCHEMA_INDEXES)
						break;
				}
				if(	   (i >= cSchema)
					|| !(prgRestrictions[i] & 0x4)
					|| !(prgRestrictions[i] & 0x10))
				{
					SAFE_RELEASE(m_pIDBSchemaRowset);
					odtLog << "INDEXES Schema Rowset or table and index restrictions are not supported \n";
				}
			}
			if (prgSchemas)	
				SAFE_FREE(prgSchemas);
			if (prgRestrictions)
				SAFE_FREE(prgRestrictions);
		}

		// create a table and get info about all the data types
		GetModInfo()->UseITableDefinition(TRUE);
		m_pTable			= new CTable(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);
		TESTC(NULL != m_pTable);
		m_pPopulatedTable	= new CTable(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);
		TESTC(NULL != m_pPopulatedTable);
		
		// get the provider types list
		m_pTable->CreateColInfo(ListNativeTemp, ListDataTypes, ALLTYPES);
		// Make all columns non-Nullable
		MakeColsNullable(m_pTable, FALSE);
		// get the column description array
		TESTC_(m_pTable->BuildColumnDescs(&m_rgColumnDesc), S_OK);
		m_cColumnDesc=m_pTable->CountColumnsOnTable();

		m_pTable->SetColumnDesc(m_rgColumnDesc, m_cColumnDesc);
		m_pTable->SetBuildColumnDesc(FALSE);
		TESTC(SUCCEEDED(m_hr = m_pTable->CreateTable(0, 0))); 
		m_pTable->DuplicateColList(m_ColList);

		m_pPopulatedTable->SetColumnDesc(m_rgColumnDesc, m_cColumnDesc);
		m_pPopulatedTable->SetBuildColumnDesc(FALSE);
		TESTC(SUCCEEDED(m_hr = m_pPopulatedTable->CreateTable(0, 0))); 
		
		// check whether IRowsetIndex is available
		rgIndexColumnDesc[0].eIndexColOrder = DBINDEX_COL_ORDER_ASC;
		m_nIndex = 0;
		SAFE_ALLOC(m_rgIndexCols, DBORDINAL, m_cColumnDesc);
		m_cIndexCols = 0;
		for (i=0; i<m_cColumnDesc; i++)
		{
			rgIndexColumnDesc[0].pColumnID = &m_rgColumnDesc[i].dbcid;
			if (SUCCEEDED(m_hr = m_pCIIndexDefinition->CreateIndex(&(m_pTable->GetTableID()), 
				NULL, 1, rgIndexColumnDesc, 0, NULL, &pIndexID)))
			{
				m_nIndex	= i+1;
				m_rgIndexCols[m_cIndexCols++] = m_nIndex;
			}
			if (pIndexID)
			{
				m_pCIIndexDefinition->DropIndex(&(m_pTable->GetTableID()), pIndexID);
				ReleaseDBID(pIndexID);
				pIndexID = NULL;
			}
		}
		// if I succeed to create some index, try to open an RowsetIndex on it
//		m_fIRowsetIndex = FALSE;
		m_fSupportORS	= SupportedProperty(DBPROP_OPENROWSETSUPPORT, DBPROPSET_DATASOURCEINFO);
		m_fORSIndex		= m_fSupportORS && GetProperty(DBPROP_OPENROWSETSUPPORT, 
			DBPROPSET_DATASOURCEINFO, *m_pCIIndexDefinition, &vVal) && 
			VT_I4 == vVal.vt && (vVal.lVal & DBPROPVAL_ORS_INDEX);
		m_fORSIntegratedIndex = m_fSupportORS && GetProperty(DBPROP_OPENROWSETSUPPORT, 
			DBPROPSET_DATASOURCEINFO, *m_pCIIndexDefinition, &vVal) && 
			VT_I4 == vVal.vt && (vVal.lVal & DBPROPVAL_ORS_INTEGRATEDINDEX);
		if (m_fORSIntegratedIndex && !COMPARE(m_fORSIndex, TRUE))
			return FALSE;

		if (m_cIndexCols>0)
		{
			rgIndexColumnDesc[0].pColumnID = &m_rgColumnDesc[m_rgIndexCols[0]].dbcid;
			if (SUCCEEDED(m_hr = m_pCIIndexDefinition->CreateIndex(&(m_pTable->GetTableID()), 
				NULL, 1, rgIndexColumnDesc, 0, NULL, &pIndexID)))
				m_hr = m_pIOpenRowset->OpenRowset(
						NULL,
						&(m_pTable->GetTableID()),
						pIndexID,
						IID_IRowsetIndex,
						0,
						NULL,
						(IUnknown**)&pIRowsetIndex
					); 
			
			if (m_fORSIndex && !CHECK(m_hr, S_OK))
				return FALSE;

			if (S_OK == m_hr)
			{
				CIIndexDefinition::s_fIRowsetIndex = TRUE;
				//m_fIRowsetIndex = TRUE;
				SAFE_RELEASE(pIRowsetIndex);
			}
			if (pIndexID)
			{
				m_pCIIndexDefinition->DropIndex(&(m_pTable->GetTableID()), pIndexID);
				ReleaseDBID(pIndexID);
				pIndexID = NULL;
			}
		}
		else
			return FALSE;

		if (CIIndexDefinition::s_fIRowsetIndex)
		{
			odtLog << "there is IRowsetIndex\n";
			CIIndexDefinition::s_fIRowsetIndex = GetModInfo()->IsUsingIRowsetIndex();
			if (!CIIndexDefinition::s_fIRowsetIndex)
				odtLog << "but it is overridden\n";
		}
		else
			odtLog << "there is not IRowsetIndex\n";

		if ((NULL == m_pIDBSchemaRowset) && (FALSE == CIIndexDefinition::s_fIRowsetIndex))
		{
			odtLog << "can not perform the test\n";
			return FALSE;
		}

		return TRUE;
	} 
CLEANUP:
	return FALSE;
} //TCIIndexDefinition::Init



//*-----------------------------------------------------------------------
//
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//-------------------------------------------------------------------------
BOOL TCIIndexDefinition::Terminate()
{ 
	m_pTable->SetColumnDesc(NULL);
	m_pPopulatedTable->SetColumnDesc(NULL);
	ReleaseColumnDesc(m_rgColumnDesc, m_cColumnDesc);

	SAFE_DELETE(m_pCIIndexDefinition);

	if (m_pTable)
	{
		m_pTable->DropTable();
		delete m_pTable;
		m_pTable = NULL;
	}
	if (m_pPopulatedTable)
	{
		m_pPopulatedTable->DropTable();
		delete m_pPopulatedTable;
		m_pPopulatedTable = NULL;
	}
	m_ColList.RemoveAll();
//	SAFE_RELEASE(m_pIIndexDefinition);
	SAFE_RELEASE(m_pIDBSchemaRowset);
	SAFE_FREE(m_pwszInvalidTableChars);
 	SAFE_FREE(m_pwszInvalidIndexChars);
	SAFE_FREE(m_pwszInvalidColumnChars);
	SAFE_FREE(m_rgIndexCols);
	ReleaseDBSession();
	ReleaseDataSourceObject();
	return(CSessionObject::Terminate());
} //TCIIndexDefinition::Terminate


//-------------------------------------------------------------------------
//
// @cmember Set a property in the property sets of a DBCOLUMNDESC
//-------------------------------------------------------------------------
BOOL TCIIndexDefinition::SetProperty
(
	DBPROPSET		**prgPropertySets,	// [in/out]		array of property sets
	ULONG			*pcPropertySets,	// [in/out]		number of property sets
	DBPROPID		propID,				// [in]			property ID
	DBTYPE			wType,				// [in]			value type
	LPVOID			value,				// [in]			property value
	DBPROPOPTIONS	dwOption,			// [in]			prop options
	DBID			colid,				// [in]			col id
	GUID			guidPropertySet,	// [in]			the GUID of the propset
	ULONG			*pcPropSet/*=NULL*/,// [out]		index of property set where the prop was set
	ULONG			*pcProp/*=NULL*/	// [out]		index in the rgProperties array
)
{
	LONG		i;
	DBPROPSET	*pPropSet;
	BOOL		fRes = FALSE, fFound;

	TESTC(::SetProperty(propID, guidPropertySet, pcPropertySets, prgPropertySets, value,
		wType, dwOption, colid));

	if (!pcPropSet || !pcProp)
	{
		fRes = TRUE;
		goto CLEANUP;
	}
	
	for (i=*pcPropertySets - 1, fFound = FALSE; i >= 0 && !fFound; i--)
	{
		fFound = (*prgPropertySets)[i].guidPropertySet == guidPropertySet;
		*pcPropSet = i;
	}

	TESTC(fFound);

	pPropSet = &(*prgPropertySets[*pcPropSet]);
	for (i=pPropSet->cProperties - 1, fFound = FALSE; i >= 0 && !fFound; i--)
	{
		fFound = pPropSet->rgProperties[i].dwPropertyID == propID;
		*pcProp = i;
	}

	TESTC(fFound);
	fRes = TRUE;

CLEANUP:
	return fRes;
} //TCIIndexDefinition::SetProperty

/*
//---------------------------------------------------------------------------
// TCIIndexDefinition::DoesIndexExistInIndexSchemaRowset
//
// TCIIndexDefinition			|
// DoesIndexExist	|
// If this index is on this table return true. If function runs correctly
// but doesn't find the table name, function will return S_OK, but fExists
// will be FALSE. If strIndexName is empty, returns E_FAIL.	The index is sought in 
// index schema rowset.
//
// @mfunc	DoesIndexExist
// @rdesc HRESULT indicating success or failure
//  @flag S_OK   | Function ran without problem
//  @flag E_FAIL    | Function ran with problems
//
//---------------------------------------------------------------------------
HRESULT TCIIndexDefinition::DoesIndexExistInIndexSchemaRowset(	
	DBID		*pTableID,				// @parm [IN]	Table ID										  
	DBID		*pIndexID,				// @parm [IN]	Index ID
	BOOL 		*fExists,				// @parm [OUT]	TRUE if index exists
	BOOL		fAnyIndex				// @parm [IN]   TRUE if search for all indexes
										//				DEFAULT FALSE
)
{
	HRESULT 			hr				= E_FAIL;
	DBCOUNTITEM			cRowsObtained	= 0;			// number of rows returned, should be 1
	HROW				*rghRows		= NULL;			// array of handles of rows
	IRowset				*pIRowset		= NULL;			// returned rowset
	const int			cRest = 5;						// restrictions on INDEXES Schema Rowset
	VARIANT				rgRestrictIndexes[cRest];
	ULONG				index;

	// Set restrictions
	for(index=0;index<cRest;index++)
		VariantInit(&rgRestrictIndexes[index]);

	TESTC(NULL != m_pIDBSchemaRowset);
	TESTC((NULL != pIndexID) || ((NULL != pTableID) && fAnyIndex));

	if (pIndexID && !fAnyIndex)
	{
		rgRestrictIndexes[2].vt 	 = VT_BSTR;
		rgRestrictIndexes[2].bstrVal = SysAllocString(pIndexID->uName.pwszName);
	}

	if (pTableID)
	{
		rgRestrictIndexes[4].vt 	 = VT_BSTR;
		rgRestrictIndexes[4].bstrVal = SysAllocString(pTableID->uName.pwszName);
	}

	if (!CHECK(hr = m_pIDBSchemaRowset->GetRowset(	
			NULL, 								// aggregation
			DBSCHEMA_INDEXES,					// REFGUID
			cRest,	 							// count of restrictions (1:types)
			rgRestrictIndexes,					// list of restrictions
			IID_IRowset,						// REFFID
			0,									// count of properties
			NULL,								// range of properties
			(IUnknown**)&pIRowset				// returned result set
		),S_OK))
			goto CLEANUP;

	// Only do this once, if there is a rowset then 
	// there is a table already in the data source
	hr=pIRowset->GetNextRows(0, 0, 1, &cRowsObtained, &rghRows);
	if (fExists)
		*fExists = (1 == cRowsObtained);	
	CHECK(hr=pIRowset->ReleaseRows(cRowsObtained,rghRows,NULL,NULL,NULL),S_OK);

	if(FAILED(hr))
			goto CLEANUP;

CLEANUP:
	SAFE_RELEASE(pIRowset);
	SAFE_FREE(rghRows);

	for(index=0;index<cRest;index++)
		GCHECK(VariantClear(&(rgRestrictIndexes[index])),S_OK);
	
	return hr;
} //TCIIndexDefinition::DoesIndexExistInIndexSchemaRowset


//---------------------------------------------------------------------------
// TCIIndexDefinition::DoesIndexExistRowsetIndex
//
// TCIIndexDefinition			|
// DoesIndexExist	|
// If this index is on this table return true. If function runs correctly
// but doesn't find the table name, function will return S_OK, but fExists
// will be FALSE. If strIndexName is empty, returns E_FAIL.	The index is sought in 
// index schema rowset.
//
// @mfunc	DoesIndexExist
// @rdesc HRESULT indicating success or failure
//  @flag S_OK   | Function ran without problem
//  @flag E_FAIL    | Function ran with problems
//
//---------------------------------------------------------------------------
HRESULT TCIIndexDefinition::DoesIndexExistRowsetIndex(	
	DBID		*pTableID,				// @parm [IN]	Table ID										  
	DBID		*pIndexID,				// @parm [IN]	Index ID
	BOOL 		*fExist					// @parm [OUT] TRUE if index exists
)
{
	HRESULT			hr;
	IRowsetIndex	*pRowsetIndex=NULL;

	hr = m_pIOpenRowset->OpenRowset(NULL, pTableID, pIndexID, IID_IRowsetIndex, 0, NULL, 
			(IUnknown**)&pRowsetIndex);
	if (fExist)
		*fExist = (S_OK == hr)? TRUE: FALSE;
	SAFE_RELEASE(pRowsetIndex);
	return S_OK;
} //TCIIndexDefinition::DoesIndexExistRowsetIndex



//---------------------------------------------------------------------------
// TCIIndexDefinition::DoesIndexExist  	
//
// TCIIndexDefinition			|
// DoesIndexExist	|
// If this index is on this table return true If function runs correctly
// but doesn't find the table name, function will return S_OK, but fExists
// will be FALSE. If strIndexName is empty, returns E_FAIL.	
//
// @mfunc	DoesIndexExist
// @rdesc HRESULT indicating success or failure
//  @flag S_OK   | Function ran without problem
//  @flag E_FAIL    | Function ran with problems
//
//---------------------------------------------------------------------------
HRESULT TCIIndexDefinition::DoesIndexExist(	
	DBID		*pTableID,				// @parm [IN]	Table ID										  
	DBID		*pIndexID,				// @parm [IN]	Index ID
	BOOL		*pfExists,				// @parm [OUT]	TRUE if index exists
	BOOL		fAnyIndex				// @parm [IN]   TRUE if search for all indexes
										// DEFAULT FALSE
)
{
	if (!pfExists)
		return E_INVALIDARG;

	if (fAnyIndex)
		if (m_pIDBSchemaRowset)
			return DoesIndexExistInIndexSchemaRowset(pTableID, NULL, pfExists, fAnyIndex);
		else
		{
			odtLog << "WARNING: IDBSchemaRowset is not supported\n";
			*pfExists = FALSE;
			return DB_S_ERRORSOCCURRED;
		}
	if (m_fIRowsetIndex)
		return DoesIndexExistRowsetIndex(pTableID, pIndexID, pfExists);
	if (m_pIDBSchemaRowset)
		return DoesIndexExistInIndexSchemaRowset(pTableID, pIndexID, pfExists);
	PRVTRACE("**TCIIndexDefinition::DoesIndexExist: how did I get here?\n");
	return E_FAIL;
} //TCIIndexDefinition::DoesIndexExist
*/



//-------------------------------------------------------------------------
//
// @cmember create an index out of column numbers
// creates, checks it and then drops it
// index size == 1
//-------------------------------------------------------------------------
BOOL TCIIndexDefinition::CreateIndexWithOneProperty(
	ULONG		nValues,						// [in] elements in the array
	VARIANT		*rgValues,						// [in] the values to be checked
	DBPROPID	dwPropID,						// [in] prop id
	GUID		guidPropertySet					// [in]
)
// it supposes that the value are not repeating!
{
	ULONG				i;
	HRESULT				hr;
	DBPROPSET			rgPropSets;
	DBPROP				rgProp;
	BOOL				fResult = TRUE;
	DBINDEXCOLUMNDESC	rgIndexColumnDesc[1];
	VARIANT				*pvValue = NULL;
	ULONG				nOptional=0, nRequired=0;
	BOOL				fSupported;
	BOOL				fSettable;
	
	fSupported	= SupportedProperty(dwPropID, guidPropertySet);
	fSettable	= SettableProperty(dwPropID, guidPropertySet);

	hr = S_OK;
	rgIndexColumnDesc[0].pColumnID = NULL;	// protection for jump to CLEANUP;
	
	FILL_PROP_SET(rgPropSets, 1, &rgProp, guidPropertySet);
	memset(&rgProp.colid, 0, sizeof(DBID));
	rgProp.dwPropertyID = dwPropID;

	// prepare parameters
	SAFE_ALLOC(rgIndexColumnDesc[0].pColumnID, DBID, 1);
	DuplicateDBID(m_rgColumnDesc[m_nIndex-1].dbcid, rgIndexColumnDesc[0].pColumnID);
	rgIndexColumnDesc[0].eIndexColOrder = DBINDEX_COL_ORDER_ASC;
	
	for (i=0; i<nValues; i++)
	{
		odtLog << "Value Number " << i << "\t";
		switch (rgValues[i].vt)
		{
		case VT_BOOL:
			odtLog << (VARIANT_TRUE == rgValues[i].boolVal ? L"VARIANT_TRUE": L"VARIANT_FALSE") << L"\n";
			break;
		case VT_I4:
			odtLog << rgValues[i].lVal << "\n";
			break;
		default:
			odtLog << "certainly a bad value\n";
		}
		rgProp.vValue		= rgValues[i];
		rgProp.dwOptions	= DBPROPOPTIONS_OPTIONAL;
		odtLog << "\tDBPROPOPTIONS_OPTIONAL:";
		hr = m_pCIIndexDefinition->CreateCheckAndDropIndex(	&(m_pTable->GetTableID()), NULL, 
			1, rgIndexColumnDesc, 1, &rgPropSets);
		if (S_OK == hr)
		{
			nOptional++;
			odtLog << "\tS_OK\n";
			if (!COMPARE(rgProp.dwStatus, DBPROPSTATUS_OK))
				fResult = FALSE;
		}
		else if (DB_S_ERRORSOCCURRED == hr)
		{
			odtLog << "\tDB_S_ERRORSOCCURRED\n";
		}
		else
		{
			odtLog << "\tERROR\n";
			CHECK(hr, DB_S_ERRORSOCCURRED);
			fResult = FALSE;
		}

		rgProp.dwOptions	= DBPROPOPTIONS_REQUIRED;
		odtLog << "\tDBPROPOPTIONS_REQUIRED:";
		hr = m_pCIIndexDefinition->CreateCheckAndDropIndex(	&m_pTable->GetTableID(), NULL, 
			1, rgIndexColumnDesc, 1, &rgPropSets);
		if (S_OK == hr)
		{
			nRequired++;
			odtLog << "\tS_OK\n";
		}
		else if (DB_E_ERRORSOCCURRED == hr)
		{
			odtLog << "\tDB_E_ERRORSOCCURRED\n";
			if (!CHECK(DBPROPSTATUS_OK == rgProp.dwStatus, FALSE))
				fResult = FALSE;
		}
		else
		{
			odtLog << "\tERROR\n";
			CHECK(hr, DB_E_ERRORSOCCURRED);
			fResult = FALSE;
		}
	}

	if (DBPROPSET_INDEX == guidPropertySet && fSupported && !fSettable
		&&	(nOptional>1 || nRequired > 1))
	{
		odtLog << "ERROR: a read-only property could be set to several values!\n";
		fResult = FALSE;
	}

CLEANUP:
	ReleaseDBID(rgIndexColumnDesc[0].pColumnID);
	return fResult;
} //TCIIndexDefinition::CreateIndexWithOneProperty


//-------------------------------------------------------------------------
//
// @cmember create an index out of column numbers
// creates, checks it and then drops it
// index size == 1
//-------------------------------------------------------------------------
BOOL TCIIndexDefinition::CreateIndexWithOneLogicalProperty(
	DBPROPID	dwPropID,			// [in] prop id
	GUID		guidPropertySet		// [in]
)
{
	VARIANT		values[2];

	values[0].vt = values[1].vt = VT_BOOL;
	V_BOOL(&values[0]) = VARIANT_TRUE;
	V_BOOL(&values[1]) = VARIANT_FALSE;
	return CreateIndexWithOneProperty(2, values, dwPropID, guidPropertySet);
} //TCIIndexDefinition::CreateIndexWithOneLogicalProperty


//-------------------------------------------------------------------------
//
// GetLiteralInfo
//-------------------------------------------------------------------------
HRESULT	TCIIndexDefinition::GetLiteralInfo()
{
	IDBInfo*		pInterface = NULL;
	const int		nLiteral=3;
	DBLITERAL		rgLiteral[nLiteral]={DBLITERAL_TABLE_NAME, DBLITERAL_INDEX_NAME, DBLITERAL_COLUMN_NAME};
	DBLITERALINFO*	rgLiteralInfo = NULL;
	ULONG			cLiteralInfo, i;
	OLECHAR*		pCharBuffer = NULL;
	IGetDataSource*	pIGetDataSource=NULL;	// IGetDataSource interface pointer

	if(!VerifyInterface(m_pIOpenRowset, IID_IGetDataSource, SESSION_INTERFACE, (IUnknown**)&pIGetDataSource))
		return E_FAIL;

	m_hr=pIGetDataSource->GetDataSource(IID_IDBInfo,(IUnknown**)&pInterface);
	SAFE_RELEASE(pIGetDataSource);

	TESTC_(m_hr = pInterface->GetLiteralInfo(	nLiteral, rgLiteral, &cLiteralInfo, 
												&rgLiteralInfo, &pCharBuffer), S_OK);

	for (i=0; i< cLiteralInfo; i++)
	{
		switch (rgLiteralInfo[i].lt)
		{
			case DBLITERAL_TABLE_NAME:
				// get the maximum size of a valid table name and the invalid chars for a table name
				m_cMaxTableName = rgLiteralInfo[i].cchMaxLen;
				SAFE_FREE(m_pwszInvalidTableChars);
				m_pwszInvalidTableChars = wcsDuplicate(rgLiteralInfo[i].pwszInvalidChars?
					rgLiteralInfo[i].pwszInvalidChars: L"");
				break;
			case DBLITERAL_INDEX_NAME:
				m_cMaxIndexName = rgLiteralInfo[i].cchMaxLen;
				SAFE_FREE(m_pwszInvalidIndexChars);
				m_pwszInvalidIndexChars = wcsDuplicate(rgLiteralInfo[i].pwszInvalidChars?
					rgLiteralInfo[i].pwszInvalidChars: L"");
				break;
			case DBLITERAL_COLUMN_NAME:
				// get the maximum size of a valid column name and the invalid chars for a table name
				m_cMaxColumnName = rgLiteralInfo[i].cchMaxLen;
				SAFE_FREE(m_pwszInvalidColumnChars);
				m_pwszInvalidColumnChars = wcsDuplicate(rgLiteralInfo[i].pwszInvalidChars?
					rgLiteralInfo[i].pwszInvalidChars: L"");
				break;
			default:
				break;
		}
	}

CLEANUP:
	SAFE_RELEASE(pInterface);
	SAFE_FREE(pCharBuffer);
	SAFE_FREE(rgLiteralInfo);
	return m_hr;
} //TCIIndexDefinition::GetLiteralInfo


//-------------------------------------------------------------------------
//
// @cmember Build a valid table name of a certain length
//-------------------------------------------------------------------------
WCHAR* TCIIndexDefinition::BuildValidName(size_t length, WCHAR* pattern)
{
	WCHAR*	pwszBuffer;
	size_t	cLen = wcslen(pattern), i;

	i=length+1;
	SAFE_ALLOC(pwszBuffer, WCHAR, i);
	memset(pwszBuffer, 0, i*sizeof(WCHAR));
	for (i=0; i< length - cLen; i+=cLen)
		wcscat(pwszBuffer, pattern);
	// manage the rest of the characters
	wcsncat(pwszBuffer, pattern, length-i);
CLEANUP:
	return pwszBuffer;
} //TCIIndexDefinition::BuildValidName

//-------------------------------------------------------------------------
//
// @cmember Build an invalid table name of a certain length
// the pattern is supposed to be shorter than the string to be build
//-------------------------------------------------------------------------
WCHAR* TCIIndexDefinition::BuildInvalidName(size_t length, WCHAR* pattern, WCHAR* invchars)
{
	WCHAR*	pwszBuffer;
	size_t	cLen = wcslen(pattern), i, cInvLen=wcslen(invchars);

	SAFE_ALLOC(pwszBuffer, WCHAR, length+1);
	if (length > cInvLen+cLen)
	{
		wcscpy(pwszBuffer, pattern);
		wcscat(pwszBuffer, invchars);
	}
	else
	{
		wcscpy(pwszBuffer, L"aa");
		wcsncat(pwszBuffer, invchars, length-wcslen(pwszBuffer));
		return pwszBuffer;
	}
	for (i=wcslen(pwszBuffer); i< length - cLen; i+=cLen)
		wcscat(pwszBuffer, pattern);
	// manage the rest of the characters
	for (; i<length; i++)
		wcscat(pwszBuffer, L"a");
CLEANUP:
	return pwszBuffer;
} //TCIIndexDefinition::BuildInvalidName




// {{ TCW_TC_PROTOTYPE(TCCreateIndex)
//*-----------------------------------------------------------------------
//| Test Case:		TCCreateIndex - tests IIndexDefinition::CreateIndex() method
//| Created:  	11/6/97
//*-----------------------------------------------------------------------


//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCCreateIndex::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCIIndexDefinition::Init())
	// }}
		return TRUE;
	return FALSE;
} 




// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Check the session object
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateIndex::Variation_1()
{ 
	return DefaultObjectTesting(*m_pCIIndexDefinition, SESSION_INTERFACE);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc create an index on a a non-empty table => S_OK
//
// @rdesc TEST_PASS or TEST_FAIL 
//
// tests all the index of length 1
int TCCreateIndex::Variation_2()
{ 
	DBORDINAL	n = m_pPopulatedTable->CountColumnsOnTable();
	
	return S_OK == DoAllIndexes(m_pPopulatedTable, n, 1)? TEST_PASS: TEST_FAIL;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc create an index on an empty table => S_OK
//
// @rdesc TEST_PASS or TEST_FAIL 
//
// tests all indexes of length 1
int TCCreateIndex::Variation_3()
{ 
	DBORDINAL	n = m_pTable->CountColumnsOnTable();

	return S_OK == DoAllIndexes(m_pTable, n, 1)? TEST_PASS: TEST_FAIL;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc create an index on a table created with a SQL command => S_OK
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateIndex::Variation_4()
{ 
	BOOL				fOldCreateTable;
	DBINDEX_COL_ORDER	rgOrder[]={DBINDEX_COL_ORDER_ASC};
	CTable				*pTable			= NULL;
	DBCOLUMNDESC		*rgColumnDesc	= NULL;
	DBORDINAL			cColumnDesc		= 0;
	DBORDINAL			*rgIndexCols	= NULL;
	DBORDINAL			cIndexCols		= 0;

	// set use SQL commands
	fOldCreateTable = GetModInfo()->UseITableDefinition(FALSE);
	TESTC_PROVIDER(m_pTable->GetCommandSupOnCTable());
	pTable = new CTable(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);
	TESTC(NULL != pTable);
	TESTC_(m_hr = pTable->CreateTable(0, 0), S_OK);

	// get the indexable columns on the newly created table
	pTable->BuildColumnDescs(&rgColumnDesc);
	cColumnDesc = pTable->CountColumnsOnTable();
	FindIndexableColumns(rgColumnDesc, cColumnDesc, &rgIndexCols, &cIndexCols);
	TESTC(0 < cIndexCols);

	// try to create an index on the first element of the indexable column
	TESTC_(CCNDropIndexFromOrdinals(pTable, NULL, rgColumnDesc, 
		cColumnDesc, 1, &rgIndexCols[0], &rgOrder[0]), S_OK);

CLEANUP:
	ReleaseColumnDesc(rgColumnDesc, cColumnDesc);
	SAFE_FREE(rgIndexCols);
	if (pTable)
	{
		pTable->DropTable();
		pTable->SetColumnDesc(NULL, 0);
		delete pTable;
	}
	GetModInfo()->UseITableDefinition(fOldCreateTable);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc create an index and get an IRowsetIndex interface on it => S_OK
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateIndex::Variation_5()
{ 
	TBEGIN
	HRESULT				hr;
	DBINDEX_COL_ORDER	rgOrder[]		= {DBINDEX_COL_ORDER_ASC};
	IRowsetIndex		*pIRowsetIndex	= NULL;
	CDBIDPtr			IndexID;

	TESTC_(CNCIndexFromOrdinals(m_pTable, NULL, (DBID**)IndexID, m_rgColumnDesc, 
								m_cColumnDesc, 1, &m_nIndex, &rgOrder[0]), S_OK);

	// try to open a rowset index on it
	if (FAILED(hr = m_pIOpenRowset->OpenRowset(NULL, &(m_pTable->GetTableID()), (DBID*)IndexID, IID_IRowsetIndex,
						0, NULL, (IUnknown**)&pIRowsetIndex)))
	{
		odtLog << "Could not open a RowsetIndex  on the table; VARIATION ABORT\n";
		TEST2C_(hr, E_NOINTERFACE, DB_E_NOINDEX);
	}
	
CLEANUP:
	SAFE_RELEASE(pIRowsetIndex);
	m_pCIIndexDefinition->DropIndex(&(m_pTable->GetTableID()), (DBID*)IndexID);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc create an index on a table generated with ITableDefinition => S_OK
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateIndex::Variation_6()
{ 
	CTable				*pTable				= NULL;
	BOOL				fTestResult			= TEST_FAIL;
	BOOL				fOldCreateTable;
	DBINDEX_COL_ORDER	rgOrder[]			= {DBINDEX_COL_ORDER_ASC};
	DBCOLUMNDESC		*rgColumnDesc		= NULL;
	DBORDINAL			cColumnDesc			= 0;
	DBORDINAL			*rgIndexCols		= NULL;
	DBORDINAL			cIndexCols			= 0;
	ITableDefinition	*pITableDefinition	= NULL;

	// set use ITableDefinition
	fOldCreateTable = GetModInfo()->UseITableDefinition(TRUE);
  
	// check ITableDefinition exists
	TESTC(VerifyInterface(*m_pCIIndexDefinition, IID_ITableDefinition, SESSION_INTERFACE, (IUnknown**)&pITableDefinition));
	SAFE_RELEASE(pITableDefinition);

	pTable = new CTable(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);
	TESTC(NULL != pTable);
	if (!CHECK(m_hr = pTable->CreateTable(0, 0), S_OK))
	{
		odtLog << "\tcould not create an empty table\n";
		goto CLEANUP;
	}

	// get the indexable columns on the newly created table
	pTable->BuildColumnDescs(&rgColumnDesc);
	cColumnDesc = pTable->CountColumnsOnTable();
	FindIndexableColumns(rgColumnDesc, cColumnDesc, &rgIndexCols, &cIndexCols);
	TESTC(0 < cIndexCols);

	TESTC_(CCNDropIndexFromOrdinals(pTable, NULL, rgColumnDesc, 
		cColumnDesc, 1, &rgIndexCols[0], rgOrder), S_OK);

	fTestResult = TEST_PASS;

CLEANUP:
	ReleaseColumnDesc(rgColumnDesc, cColumnDesc);
	SAFE_FREE(rgIndexCols);
	if (pTable)
	{
		CHECK(pTable->DropTable(), S_OK);
		delete pTable;
	}
	GetModInfo()->UseITableDefinition(fOldCreateTable);
	return fTestResult;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc create an index on a table that has a rowset open on it => S_OK
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateIndex::Variation_7()
{ 
	BOOL				fTestResult = TEST_FAIL;
	DBINDEX_COL_ORDER	rgOrder[]={DBINDEX_COL_ORDER_ASC};
	IRowset				*pIRowset = NULL;

	if (!CHECK(m_hr = m_pIOpenRowset->OpenRowset(NULL, &m_pTable->GetTableID(), NULL, IID_IRowset,
						0, NULL, (IUnknown**)&pIRowset), S_OK))
	{
		odtLog << "Could not open a Rowset on the table\n";
		goto CLEANUP;
	}
	
	m_hr = CCNDropIndexFromOrdinals(m_pTable, NULL, m_rgColumnDesc, m_cColumnDesc, 1, &m_nIndex, rgOrder);

	if (S_OK == m_hr)
		fTestResult = TEST_PASS;
	else
		if (DB_E_TABLEINUSE == m_hr)
		{
			odtLog << "DB_E_TABLEINUSE returned\n";
			fTestResult = TEST_PASS;
		}

CLEANUP:
	SAFE_RELEASE(pIRowset);
	return fTestResult;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc create an index on a table using column names and colID's read from IColumnsInfo
// tries to insert 2 indexes: one in which colID are given as GUID and one mixed
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateIndex::Variation_8()
{ 
	TBEGIN
	DBINDEXCOLUMNDESC	rgIndexColumnDesc[2];
	CCol				col;

	rgIndexColumnDesc[0].eIndexColOrder = DBINDEX_COL_ORDER_ASC;
	col = m_ColList.GetAt(m_ColList.FindIndex(m_rgIndexCols[0]-1));
	rgIndexColumnDesc[0].pColumnID = col.GetColID();
	
	odtLog << "index with a column GUID: ";
	if (!CHECK(m_hr = m_pCIIndexDefinition->CreateCheckAndDropIndex(&m_pTable->GetTableID(), NULL, 1, rgIndexColumnDesc), DB_E_NOCOLUMN))
	{
		odtLog << "INSUCCESS\n";
		goto CLEANUP;
	}

	// check names
	rgIndexColumnDesc[0].pColumnID = &m_rgColumnDesc[m_rgIndexCols[0]-1].dbcid;

	// index on 2 columns 1GUID and one as is
	if (m_cIndexCols < 2)
	{
		odtLog << "there are not enough indexable columns on the table\n";
		goto CLEANUP;
	}

	rgIndexColumnDesc[1].eIndexColOrder = DBINDEX_COL_ORDER_ASC;
	rgIndexColumnDesc[0].pColumnID = col.GetColID();
	rgIndexColumnDesc[1].pColumnID = &m_rgColumnDesc[m_rgIndexCols[1]-1].dbcid;

	odtLog << "index with a column GUID and a column name";
	if (!CHECK(m_hr = m_pCIIndexDefinition->CreateCheckAndDropIndex(&m_pTable->GetTableID(), NULL, 2, rgIndexColumnDesc), DB_E_NOCOLUMN))
	{
		odtLog << "INSUCCESS\n";
		goto CLEANUP;
	}
	// need to check name against name!
	rgIndexColumnDesc[0].pColumnID = &m_rgColumnDesc[m_rgIndexCols[0]-1].dbcid;

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc include a column twice in an index
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateIndex::Variation_9()
{ 
	DBORDINAL			rgIndexColumns[]={m_nIndex, m_nIndex};
	DBINDEX_COL_ORDER	rgOrder[]={DBINDEX_COL_ORDER_ASC, DBINDEX_COL_ORDER_ASC};

	if (S_OK == CCNDropIndexFromOrdinals(m_pTable, NULL, m_rgColumnDesc, m_cColumnDesc, 
			NUMELEM(rgIndexColumns), rgIndexColumns, rgOrder))
		odtLog << "\tcould add a column twice\n";
	else
		odtLog << "\tcould not add a column twice\n";
	
	return TEST_PASS;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc create an index on an autoincrement column => S_OK
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateIndex::Variation_10()
{ 
	CTable				*pTable = NULL;
	BOOL				fTestResult = TEST_SKIPPED;
	BOOL				fOldCreateTable;
	BOOL				flag;
	DBORDINAL			rgIndexColumns[1];
	DBINDEX_COL_ORDER	rgOrder[]={DBINDEX_COL_ORDER_ASC};
	DBORDINAL			nAutoInc;		// index of an autoincrementable column
	DBORDINAL			nIndex;			// "array index of the index column"
	CCol				col;
	DBCOLUMNDESC		*pCD = NULL;
	VARIANT				value;
	ITableDefinition	*pITableDefinition = NULL;
	DBPROPSET			*rgPropSets = NULL;
	ULONG				cPropSets = 0;
	DBCOLUMNDESC		*rgColumnDesc = NULL;

	// set use ITableDefinition
	fOldCreateTable = GetModInfo()->UseITableDefinition(TRUE);

	// get sure ITaleDefinition is supported
	TESTC(VerifyInterface(*m_pCIIndexDefinition, IID_ITableDefinition, 
						  SESSION_INTERFACE, (IUnknown**)&pITableDefinition));
	SAFE_RELEASE(pITableDefinition);

	pTable = new CTable(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);
	TESTC(NULL != pTable);
	
	// create a table with an autoincrement column
	pTable->SetBuildColumnDesc(FALSE);
	DuplicateColumnDesc(m_rgColumnDesc, m_cColumnDesc, &rgColumnDesc);
	ReleaseAllColumnPropSets(m_cColumnDesc, rgColumnDesc);
	pTable->SetColumnDesc(rgColumnDesc, m_cColumnDesc);
	// get an autoincrementable type
	for (nIndex = 0, flag = FALSE; nIndex< m_cIndexCols && !flag; nIndex++)
	{
		// get the column coresponding to the nIndex-th indexable type
		col = m_pTable->GetColInfoForUpdate(m_rgIndexCols[nIndex]);
		// check it is autoincrementable
		flag = (1 == col.CanAutoInc());
		if (flag)
		{
			nAutoInc = m_rgIndexCols[nIndex];
			PRVTRACE(L"autoinc column %d\n", nAutoInc);
		}
	}
	if (!flag)
	{
		odtLog << "could not find an autoincrementable type that could generate an index\n";
		goto CLEANUP;
	}

	// get the autoinc column
	pCD = &m_rgColumnDesc[nAutoInc-1];
	rgPropSets	= pCD->rgPropertySets;
	cPropSets	= pCD->cPropertySets;
	pCD->rgPropertySets	= NULL;
	pCD->cPropertySets	= 0;
	odtLog << "try to create an index on autoincrementable column of type " << pCD->pwszTypeName << "\n";
	TESTC(NULL != pCD);
	// set the column autoinc
	value.vt = VT_BOOL;
	V_BOOL(&value)=VARIANT_TRUE;
	TESTC_(SetProperty(&rgColumnDesc[nAutoInc-1].rgPropertySets, &rgColumnDesc[nAutoInc-1].cPropertySets, DBPROP_COL_AUTOINCREMENT, 
		VT_BOOL, (LPVOID)VARIANT_TRUE, DBPROPOPTIONS_REQUIRED, DB_NULLID, DBPROPSET_COLUMN), TRUE);

	if (!CHECK(m_hr = pTable->CreateTable(0, 0), S_OK))
	{
		odtLog << "\tcould not create the table\n";
		fTestResult = TEST_FAIL;		
		goto CLEANUP;
	}

	rgIndexColumns[0] = nAutoInc;	// do not forget the offset
	fTestResult = (S_OK == (m_hr = CCNDropIndexFromOrdinals(pTable,	NULL, 
		m_rgColumnDesc, m_cColumnDesc, 1, rgIndexColumns, rgOrder)))? TEST_PASS: TEST_FAIL;

CLEANUP:
	if (pCD)
	{
		FreeProperties( &pCD->cPropertySets, &pCD->rgPropertySets);
		pCD->rgPropertySets	= rgPropSets;
		pCD->cPropertySets	= cPropSets;
	}
	if (pTable)
	{
		pTable->DropTable();
		delete pTable;
	}
	GetModInfo()->UseITableDefinition(fOldCreateTable);
	return fTestResult;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc create an index on a column, drop the column
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateIndex::Variation_11()
{ 
	TBEGIN
	HRESULT				hr;
	CTable				*pTable;
	CDBIDPtr			IndexID;
	CCol				col;
	DBINDEX_COL_ORDER	rgOrder[]		= {DBINDEX_COL_ORDER_ASC};
	DBCOLUMNDESC		*rgColumnDesc	= NULL;
	DBORDINAL			cColumnDesc		= 0;
	DBORDINAL			*rgIndexCols	= NULL;
	DBORDINAL			cIndexCols		= 0;

	pTable = new CTable(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);
	TESTC(NULL!=pTable);
	TESTC_(pTable->CreateTable(0, 0), S_OK);

	// get the indexable columns on the newly created table
	pTable->BuildColumnDescs(&rgColumnDesc);
	cColumnDesc = pTable->CountColumnsOnTable();
	FindIndexableColumns(rgColumnDesc, cColumnDesc, &rgIndexCols, &cIndexCols);
	TESTC(0 < cIndexCols);

	// set an index
	TESTC_(hr = CNCIndexFromOrdinals(pTable, NULL, (DBID**)IndexID, rgColumnDesc, cColumnDesc, 
		1, &rgIndexCols[0], rgOrder), S_OK);

	// expect error when dropping an index column
	COMPARE(S_OK != (hr = pTable->DropColumn(rgIndexCols[0])), TRUE);

	CHECK(m_pCIIndexDefinition->DropIndex(&pTable->GetTableID(), (DBID*)IndexID), S_OK);

CLEANUP:
	if (pTable)
	{
		pTable->DropTable();
		delete pTable;
	}
	ReleaseColumnDesc(rgColumnDesc, cColumnDesc);
	SAFE_FREE(rgIndexCols);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END



// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc create an index on a view
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateIndex::Variation_12()
{ 
	TBEGIN
	HRESULT				hr;
	CDBIDPtr			IndexID;
	WCHAR				wszViewName[]=L"dodoView";
	DBID				ViewID;
	DBINDEXCOLUMNDESC	rgIndexColumnDesc[1];

	// since there is a CTable object, we can querry for command support
	if (!m_pTable->GetCommandSupOnCTable())
	{
		odtLog << "can't run this variation because commands are not supported\n";
		TRETURN;
	}

	// create a view on the current table
	if (S_OK != m_pTable->ExecuteCommand(CREATE_VIEW, IID_NULL, wszViewName))
	{
		odtLog << "could not create a view on the base table\n";
		goto CLEANUP;
	}

	// set an index
	ViewID.eKind			= DBKIND_NAME;
	ViewID.uName.pwszName	= wszViewName;
	rgIndexColumnDesc[0].pColumnID = &m_rgColumnDesc[m_nIndex-1].dbcid;

	//create index on view
	hr = m_pCIIndexDefinition->CreateIndex(&ViewID, NULL, NUMELEM(rgIndexColumnDesc), 
			rgIndexColumnDesc, 0, NULL, (DBID**)IndexID);

	if (SUCCEEDED(hr))
	{
		odtLog << "an index on the view was successfully created\n";
		CHECK(m_pCIIndexDefinition->DropIndex(&m_pTable->GetTableID(), (DBID*)IndexID), S_OK);
	}

CLEANUP:
	// drop the view
	m_pTable->ExecuteCommand(DROP_VIEW, IID_NULL, wszViewName);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc create multiple indexes on the same table
//
// @rdesc TEST_PASS or TEST_FAIL 
//
// 1. tries to create an index for each indexable column in the table
// 2. check the number of the successfully created indexes
// 3. check the cleanning
int TCCreateIndex::Variation_13()
{ 
	BOOL				fTestResult = TEST_PASS;
	ULONG				nIndex		= 0;
	ULONG				i;
	DBID				**rgIndexID	= NULL;
	DBINDEXCOLUMNDESC	rgIndexColumnDesc[1];
	ULONG				nMaxIndexes	= (1+rand()%5) * m_cIndexCols;

	// create an index array
	SAFE_ALLOC(rgIndexID, DBID*, nMaxIndexes);
	rgIndexColumnDesc[0].eIndexColOrder = DBINDEX_COL_ORDER_ASC;
	for (nIndex=i=0; i<nMaxIndexes; i++)
	{
		rgIndexColumnDesc[0].pColumnID = &m_rgColumnDesc[m_rgIndexCols[i%m_cIndexCols]-1].dbcid;
		rgIndexID[nIndex] = NULL;
		TESTC(NULL != m_pCIIndexDefinition);
		if (CHECK(m_hr = m_pCIIndexDefinition->CreateIndexAndCheck(&m_pTable->GetTableID(), NULL, 
			1, rgIndexColumnDesc, 0, NULL, &rgIndexID[nIndex]), S_OK))
			nIndex++;
		else
			fTestResult = TEST_FAIL;
	}

	TESTC(0<nIndex);
	odtLog << "\t" << nIndex << " indexes out of " << nMaxIndexes << " were created on the table\n";

CLEANUP:
	if (rgIndexID)
		for (i=0; i<nIndex; i++)
		{
			// drop index i
			m_pCIIndexDefinition->DropIndex(&m_pTable->GetTableID(), rgIndexID[i]);
			ReleaseDBID(rgIndexID[i], TRUE);
		}
	SAFE_FREE(rgIndexID);
	return fTestResult;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc create an index on a table, drop the table and check if the index is dropped as well)
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateIndex::Variation_14()
{ 
	TBEGIN
	BOOL				fExists			= FALSE;
	CDBIDPtr			IndexID;
	CCol				col;
	CTable				*pTable			= NULL;
	DBINDEX_COL_ORDER	rgOrder[]		= {DBINDEX_COL_ORDER_ASC};
	DBCOLUMNDESC		*rgColumnDesc	= NULL;
	DBORDINAL			cColumnDesc		= 0;
	DBORDINAL			*rgIndexCols	= NULL;
	DBORDINAL			cIndexCols		= 0;

	pTable = new CTable(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);
	TESTC(NULL != pTable);
	if (!CHECK(m_hr = pTable->CreateTable(0, 0), S_OK))
	{
		odtLog << "\tcould not create the table\n";
		goto CLEANUP;
	}

	// get the indexable columns on the newly created table
	pTable->BuildColumnDescs(&rgColumnDesc);
	cColumnDesc = pTable->CountColumnsOnTable();
	FindIndexableColumns(rgColumnDesc, cColumnDesc, &rgIndexCols, &cIndexCols);
	TESTC(0 < cIndexCols);

	// set an index
	TESTC_(CNCIndexFromOrdinals(pTable, NULL, (DBID**)IndexID, rgColumnDesc, 
		cColumnDesc, 1, &rgIndexCols[0], &rgOrder[0]), S_OK);

	// drop the table and check the existence of the index
	CHECK(m_hr = pTable->DropTable(), S_OK);
	TESTC(NULL != (DBID*)IndexID);
	CHECK(m_hr = m_pCIIndexDefinition->DoesIndexExist(NULL, (DBID*)IndexID, &fExists), S_OK);
	
	COMPARE(fExists, FALSE);

CLEANUP:
	if (pTable)
	{
		m_pCIIndexDefinition->DropIndex(&pTable->GetTableID(), (DBID*)IndexID);
		m_hr = pTable->DropTable();
		delete pTable;
	}
	ReleaseColumnDesc(rgColumnDesc, cColumnDesc);
	SAFE_FREE(rgIndexCols);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc create an index on a table; drop a column from index and readd it with another type
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateIndex::Variation_15()
{
	TBEGIN
	CTable				*pTable				= NULL;
	ULONG				k;
	CDBIDPtr			IndexID;
	CCol				col;
	ITableDefinition	*pITableDefinition	= NULL;
	DBCOLUMNDESC		*dbCD=NULL;
	DBINDEX_COL_ORDER	rgOrder[]			= {DBINDEX_COL_ORDER_ASC};
	DBCOLUMNDESC		*rgColumnDesc		= NULL;
	DBORDINAL			cColumnDesc			= 0;
	DBORDINAL			*rgIndexCols		= NULL;
	DBORDINAL			cIndexCols			= 0;

	VerifyInterface(*m_pCIIndexDefinition, IID_ITableDefinition, SESSION_INTERFACE, (IUnknown**)&pITableDefinition);
	pTable = new CTable(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);
	TESTC(NULL != pTable);
	if (!CHECK(m_hr = pTable->CreateTable(0, 0), S_OK))
	{
		odtLog << "\tcould not create a table\n";
		goto CLEANUP;
	}

	// get the indexable columns on the newly created table
	pTable->BuildColumnDescs(&rgColumnDesc);
	cColumnDesc = pTable->CountColumnsOnTable();
	FindIndexableColumns(rgColumnDesc, cColumnDesc, &rgIndexCols, &cIndexCols);
	TESTC(0 < cIndexCols);

	// set an index
	TESTC_(CNCIndexFromOrdinals(pTable, NULL, (DBID**)IndexID, rgColumnDesc, 
		cColumnDesc, 1, &rgIndexCols[0], &rgOrder[0]), S_OK);

	// try to drop the index column
	col = pTable->GetColInfoForUpdate(rgIndexCols[0]);
	if (S_OK == pTable->DropColumn(col))
	{
		TESTC(NULL != pITableDefinition);
		// try to readd the same column with a different type
		DuplicateColumnDesc(&m_rgColumnDesc[rgIndexCols[0]-1], 1, &dbCD);
		SAFE_FREE(dbCD->pwszTypeName);
		// change the type
		for (k=0; k < m_cColumnDesc && dbCD->wType != m_rgColumnDesc[k].wType; k++);
		if (k<m_cColumnDesc)
			dbCD->wType = m_rgColumnDesc[k].wType;
		m_hr = pITableDefinition->AddColumn(&pTable->GetTableID(), dbCD, NULL);
		if (S_OK == m_hr)
		{
			COMPARE(FALSE, TRUE);
			odtLog << "column added\n";
		}
		ReleaseColumnDesc(dbCD, 1);
	}
	else
		odtLog << "could not drop the index column\n";


CLEANUP:
	m_pCIIndexDefinition->DropIndex(&pTable->GetTableID(), (DBID*)IndexID);
	if (pTable)
	{
		pTable->DropTable();
		delete pTable;
	}
	SAFE_RELEASE(pITableDefinition);
	ReleaseColumnDesc(rgColumnDesc, cColumnDesc);
	SAFE_FREE(rgIndexCols);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc NULL pointer for pTableID => E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateIndex::Variation_16()
{ 
	TBEGIN
	DBINDEXCOLUMNDESC	rgIndexColumnDesc[1];
	CDBIDPtr			pIndexID = NULL;

	rgIndexColumnDesc[0].pColumnID		= &m_rgColumnDesc[m_nIndex-1].dbcid;
	rgIndexColumnDesc[0].eIndexColOrder	= DBINDEX_COL_ORDER_ASC;
	
	if (!CHECK(m_hr = m_pCIIndexDefinition->CreateIndexAndCheck(NULL, NULL, 1, 
				rgIndexColumnDesc, 0, NULL, (DBID**)pIndexID), E_INVALIDARG) &&	SUCCEEDED(m_hr))
		m_pCIIndexDefinition->DropIndex(NULL, pIndexID);

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc *pTableID == DB_NULLID => DB_E_NOTABLE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateIndex::Variation_17()
{ 
	TBEGIN
	DBINDEXCOLUMNDESC	rgIndexColumnDesc[1];
	CDBIDPtr			pIndexID;

	rgIndexColumnDesc[0].pColumnID		= &m_rgColumnDesc[m_nIndex-1].dbcid;
	rgIndexColumnDesc[0].eIndexColOrder	= DBINDEX_COL_ORDER_ASC;
	if (!CHECK(m_hr = m_pCIIndexDefinition->CreateIndexAndCheck((struct tagDBID*)&DB_NULLID, NULL,  
		NUMELEM(rgIndexColumnDesc), rgIndexColumnDesc, 0, NULL, (DBID**)pIndexID), DB_E_NOTABLE)
		&& SUCCEEDED(m_hr))
		m_pCIIndexDefinition->DropIndex(NULL, pIndexID);

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc pTableID->uName.pwszName is NULL => DB_E_NOTABLE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateIndex::Variation_18()
{ 
	TBEGIN
	DBINDEXCOLUMNDESC	rgIndexColumnDesc[1];
	CDBIDPtr			pIndexID;
	DBID				TableID;

	rgIndexColumnDesc[0].pColumnID		= &m_rgColumnDesc[m_nIndex-1].dbcid;
	rgIndexColumnDesc[0].eIndexColOrder	= DBINDEX_COL_ORDER_ASC;
	TableID.eKind						= DBKIND_NAME;
	TableID.uName.pwszName				= NULL; 
	if (	!CHECK(m_hr = m_pCIIndexDefinition->CreateIndexAndCheck(&TableID, NULL, 1, rgIndexColumnDesc, 
				0, NULL, (DBID**)pIndexID), DB_E_NOTABLE)
		&&	SUCCEEDED(m_hr))
		m_pCIIndexDefinition->DropIndex(NULL, pIndexID);
	
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc pTableID->uName.pwszName is empty => DB_E_NOTABLE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateIndex::Variation_19()
{ 
	TBEGIN
	DBINDEXCOLUMNDESC	rgIndexColumnDesc[1];
	CDBIDPtr			pIndexID;
	DBID				TableID;

	rgIndexColumnDesc[0].pColumnID		= &m_rgColumnDesc[m_nIndex-1].dbcid;
	rgIndexColumnDesc[0].eIndexColOrder	= DBINDEX_COL_ORDER_ASC;
	TableID.eKind						= DBKIND_NAME;
	TableID.uName.pwszName				= L"";

	if (	!CHECK(m_hr = m_pCIIndexDefinition->CreateIndexAndCheck(&TableID, NULL, NUMELEM(rgIndexColumnDesc),
			rgIndexColumnDesc, 0, NULL, (DBID**)pIndexID), DB_E_NOTABLE)
		&&	SUCCEEDED(m_hr))
		m_pCIIndexDefinition->DropIndex(NULL, pIndexID);

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc table name is invalid => DB_E_NOTABLE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateIndex::Variation_20()
{ 
	BOOL				fTestResult = TEST_FAIL;
	DBINDEXCOLUMNDESC	rgIndexColumnDesc[1];
	CDBIDPtr			pIndexID;
	DBID				TableID;
	size_t				m, n;

	rgIndexColumnDesc[0].pColumnID		= &m_rgColumnDesc[m_nIndex-1].dbcid;
	rgIndexColumnDesc[0].eIndexColOrder	= DBINDEX_COL_ORDER_ASC;

	// get an invalid table name
	TableID.eKind						= DBKIND_NAME;
	m = min(m_cMaxTableName, n=5+wcslen(m_pTable->GetTableName()));
	TableID.uName.pwszName	= BuildInvalidName(m, m_pTable->GetTableName(), m_pwszInvalidTableChars);
	
	if (	!CHECK(m_hr = m_pCIIndexDefinition->CreateIndexAndCheck(&TableID, NULL, 
			1, rgIndexColumnDesc, 0, NULL, (DBID**)pIndexID), DB_E_NOTABLE)
		&&	SUCCEEDED(m_hr))
		m_pCIIndexDefinition->DropIndex(NULL, pIndexID);
	
	ReleaseDBID(&TableID, FALSE);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc table name has maximum length => S_OK
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateIndex::Variation_21()
{ 
	BOOL				fTestResult		= TEST_FAIL;
	DBID				TableID;
	LPWSTR				pwszTableName; 
	CTable				*pTable			= NULL;
	DBINDEX_COL_ORDER	rgOrder[]		= {DBINDEX_COL_ORDER_ASC};
	DBCOLUMNDESC		*rgColumnDesc	= NULL;
	DBORDINAL			cColumnDesc		= 0;;
	DBORDINAL			*rgIndexCols	= NULL;
	DBORDINAL			cIndexCols		= 0;

	// get an valid table name
	TableID.eKind							= DBKIND_NAME;
	TableID.uName.pwszName	= pwszTableName	= BuildValidName(m_cMaxTableName, m_pTable->GetTableName());
	
	pTable = new CTable(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);
	TESTC(NULL != pTable);
	if (!CHECK(m_hr = pTable->CreateTable(0, 0, pwszTableName), S_OK))
	{
		odtLog << "\tcould not create an empty table\n";
		goto CLEANUP;
	}

	// get the indexable columns on the newly created table
	pTable->BuildColumnDescs(&rgColumnDesc);
	cColumnDesc = pTable->CountColumnsOnTable();
	FindIndexableColumns(rgColumnDesc, cColumnDesc, &rgIndexCols, &cIndexCols);
	TESTC(0 < cIndexCols);

	// set an index
	TESTC_(CCNDropIndexFromOrdinals(pTable, NULL, rgColumnDesc, cColumnDesc, 1,
		rgIndexCols, rgOrder), S_OK);
	
	fTestResult = TEST_PASS;

CLEANUP:
	if (pTable)
	{
		m_hr = pTable->DropTable();
		delete pTable;
	}
	ReleaseDBID(&TableID, FALSE);
	ReleaseColumnDesc(rgColumnDesc, cColumnDesc);
	SAFE_FREE(rgIndexCols);
	return fTestResult;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(22)
//*-----------------------------------------------------------------------
// @mfunc table name exceeds maximum table name => DB_E_NOTABLE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateIndex::Variation_22()
{ 
	BOOL				fTestResult = TEST_FAIL;
	DBINDEXCOLUMNDESC	rgIndexColumnDesc[1];
	DBID				TableID;

	rgIndexColumnDesc[0].pColumnID		= &m_rgColumnDesc[m_nIndex-1].dbcid;
	rgIndexColumnDesc[0].eIndexColOrder	= DBINDEX_COL_ORDER_ASC;

	// too long table name
	TableID.eKind						= DBKIND_NAME;
	TableID.uName.pwszName	= BuildValidName(m_cMaxTableName+1, m_pTable->GetTableName());
	TESTC_(m_pCIIndexDefinition->CreateCheckAndDropIndex(&TableID, NULL, 1, rgIndexColumnDesc), DB_E_NOTABLE);
	fTestResult = TEST_PASS;

CLEANUP:
	ReleaseDBID(&TableID, FALSE);
	return fTestResult;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(23)
//*-----------------------------------------------------------------------
// @mfunc NULL pointer for pIndexID and not NULL ppIndexID => S_OK
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateIndex::Variation_23()
{ 
	TBEGIN
	CDBIDPtr			IndexID;
	DBINDEX_COL_ORDER	rgOrder[]={DBINDEX_COL_ORDER_ASC};

	TESTC_(CNCIndexFromOrdinals(m_pTable, NULL, (DBID**)IndexID, 
			m_rgColumnDesc, m_cColumnDesc, 1, &m_nIndex, rgOrder), S_OK);

CLEANUP:
	CHECK(m_pCIIndexDefinition->DropIndex(&m_pTable->GetTableID(), (DBID*)IndexID), S_OK);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(24)
//*-----------------------------------------------------------------------
// @mfunc not NULL pointer for pIndexID and NULL ppIndexID =>S_OK
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateIndex::Variation_24()
{ 
	DBID				IndexID;
	BOOL				fTestResult = TEST_FAIL;
	DBINDEX_COL_ORDER	rgOrder[]={DBINDEX_COL_ORDER_ASC};

	IndexID.eKind			= DBKIND_NAME;
	IndexID.uName.pwszName	= L"IndiceOarecare";
	TESTC_(m_hr = CNCIndexFromOrdinals(m_pTable, &IndexID, NULL,  
			m_rgColumnDesc, m_cColumnDesc, 1, &m_nIndex, rgOrder), S_OK);
	fTestResult = TEST_PASS;

CLEANUP:
	m_hr = m_pCIIndexDefinition->DropIndex(&m_pTable->GetTableID(), &IndexID);
	return fTestResult;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(25)
//*-----------------------------------------------------------------------
// @mfunc NULL pointer for pIndexID and NULL ppIndexID => E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateIndex::Variation_25()
{ 
	BOOL				fTestResult = TEST_FAIL;
	DBINDEX_COL_ORDER	rgOrder[]={DBINDEX_COL_ORDER_ASC};

	TESTC_(m_hr = CNCIndexFromOrdinals(m_pTable, NULL, NULL,
			m_rgColumnDesc, m_cColumnDesc, 1, &m_nIndex, rgOrder), E_INVALIDARG);
	fTestResult = TEST_PASS;
CLEANUP:
	// drop all indexes
	m_hr = m_pCIIndexDefinition->DropIndex(&m_pTable->GetTableID(), NULL);
	return fTestResult;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(26)
//*-----------------------------------------------------------------------
// @mfunc *pIndexID == DB_NULLID => DB_E_BADINDEXID
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateIndex::Variation_26()
{ 
	BOOL				fTestResult = TEST_FAIL;
	DBINDEX_COL_ORDER	rgOrder[]={DBINDEX_COL_ORDER_ASC};

	TESTC_(m_hr = CCNDropIndexFromOrdinals(m_pTable, (struct tagDBID*)&DB_NULLID,
			m_rgColumnDesc, m_cColumnDesc, 1, &m_nIndex, rgOrder), DB_E_BADINDEXID);

	fTestResult = TEST_PASS;

CLEANUP:	
	return fTestResult;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(27)
//*-----------------------------------------------------------------------
// @mfunc pIndexID->uName.pwszName is NULL => DB_E_BADINDEXID
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateIndex::Variation_27()
{ 
	BOOL				fTestResult = TEST_FAIL;
	DBID				IndexID;
	DBINDEX_COL_ORDER	rgOrder[]={DBINDEX_COL_ORDER_ASC};

	IndexID.eKind			= DBKIND_NAME;
	IndexID.uName.pwszName	= NULL;

	TESTC_(m_hr = CCNDropIndexFromOrdinals(m_pTable, &IndexID,
			m_rgColumnDesc, m_cColumnDesc, 1, &m_nIndex, rgOrder), DB_E_BADINDEXID);

	fTestResult = TEST_PASS;
CLEANUP:
	return fTestResult;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(28)
//*-----------------------------------------------------------------------
// @mfunc pIndexID->uName.pwszName is empty => DB_E_BADINDEXID
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateIndex::Variation_28()
{ 
	BOOL				fTestResult = TEST_FAIL;
	DBINDEX_COL_ORDER	rgOrder[]={DBINDEX_COL_ORDER_ASC};
	DBID				IndexID;

	IndexID.eKind			= DBKIND_NAME;
	IndexID.uName.pwszName	= L"";
	TESTC_(m_hr = CCNDropIndexFromOrdinals(m_pTable, &IndexID,
			m_rgColumnDesc, m_cColumnDesc, 1, &m_nIndex, rgOrder), DB_E_BADINDEXID);
	fTestResult = TEST_PASS;
CLEANUP:
	return fTestResult;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(29)
//*-----------------------------------------------------------------------
// @mfunc index name is invalid => DB_E_BADINDEXID
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateIndex::Variation_29()
{ 
	BOOL				fTestResult = TEST_FAIL;
	DBID				IndexID;
	size_t				m, n;
	DBINDEX_COL_ORDER	rgOrder[]={DBINDEX_COL_ORDER_ASC};

	// get an invalid index name
	IndexID.eKind = DBKIND_NAME;
	m = min(m_cMaxIndexName, n=5+wcslen(m_pTable->GetTableName()));
	IndexID.uName.pwszName	= BuildInvalidName(m, m_pTable->GetTableName(), m_pwszInvalidIndexChars);
	
	TESTC_(m_hr = CCNDropIndexFromOrdinals(m_pTable, &IndexID,
			m_rgColumnDesc, m_cColumnDesc, 1, &m_nIndex, rgOrder), DB_E_BADINDEXID);

	fTestResult = TEST_PASS;

CLEANUP:
	ReleaseDBID(&IndexID, FALSE);
	return fTestResult;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(30)
//*-----------------------------------------------------------------------
// @mfunc index name has maximum length => S_OK
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateIndex::Variation_30()
{ 
	BOOL				fTestResult = TEST_FAIL;
	DBID				IndexID;
	DBINDEX_COL_ORDER	rgOrder[]	= {DBINDEX_COL_ORDER_ASC};

	// get a maximum length index name
	IndexID.eKind			= DBKIND_NAME;
	IndexID.uName.pwszName	= BuildValidName(m_cMaxIndexName, m_pTable->GetTableName());
	
	// set an index
	TESTC_(m_hr = CCNDropIndexFromOrdinals(m_pTable, &IndexID,
			m_rgColumnDesc, m_cColumnDesc, 1, &m_nIndex, rgOrder), S_OK);

	fTestResult = TEST_PASS;

CLEANUP:
	ReleaseDBID(&IndexID, FALSE);
	return fTestResult;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(31)
//*-----------------------------------------------------------------------
// @mfunc index name exceeds maximum length name => DB_E_BADINDEXID
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateIndex::Variation_31()
{ 
	BOOL				fTestResult = TEST_FAIL;
	DBID				IndexID;
	DBINDEX_COL_ORDER	rgOrder[] = {DBINDEX_COL_ORDER_ASC};

	// get a too long index name
	IndexID.eKind			= DBKIND_NAME;
	IndexID.uName.pwszName	= BuildValidName(m_cMaxIndexName+1, m_pTable->GetTableName());
	
	TESTC_(m_hr = CCNDropIndexFromOrdinals(m_pTable, &IndexID, m_rgColumnDesc, m_cColumnDesc, 
		1, &m_nIndex, rgOrder), DB_E_BADINDEXID);

	fTestResult = TEST_PASS;

CLEANUP:
	ReleaseDBID(&IndexID, FALSE);
	return fTestResult;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(32)
//*-----------------------------------------------------------------------
// @mfunc cIndexColumnDesc == 0 => E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateIndex::Variation_32()
{ 
	BOOL				fTestResult = TEST_FAIL;
	DBINDEX_COL_ORDER	rgOrder[]={DBINDEX_COL_ORDER_ASC};

	TESTC_(m_hr = CCNDropIndexFromOrdinals(m_pTable, NULL, m_rgColumnDesc, m_cColumnDesc, 
		0, &m_nIndex, rgOrder), E_INVALIDARG);
	TESTC_(m_hr = CCNDropIndexFromOrdinals(m_pTable, NULL, m_rgColumnDesc, m_cColumnDesc, 
		0, NULL, rgOrder), E_INVALIDARG);

	fTestResult = TEST_PASS;

CLEANUP:
	return fTestResult;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(33)
//*-----------------------------------------------------------------------
// @mfunc rgIndexColumnDesc == NULL => E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateIndex::Variation_33()
{ 
	BOOL				fTestResult = TEST_FAIL;

	TESTC_(m_pCIIndexDefinition->CreateCheckAndDropIndex(&m_pTable->GetTableID(), NULL, 1, NULL), E_INVALIDARG);
	TESTC_(m_pCIIndexDefinition->CreateCheckAndDropIndex(&m_pTable->GetTableID(), NULL, 1, NULL), E_INVALIDARG);

	fTestResult = TEST_PASS;

CLEANUP:
	return fTestResult;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(34)
//*-----------------------------------------------------------------------
// @mfunc cPropSets != 0 and rgPropSets == NULL => E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateIndex::Variation_34()
{ 
	BOOL				fTestResult = TEST_FAIL;
	DBINDEX_COL_ORDER	rgOrder[]={DBINDEX_COL_ORDER_ASC};

	TESTC_(m_hr = CCNDropIndexFromOrdinals(m_pTable, NULL, m_rgColumnDesc, m_cColumnDesc, 
			1, &m_nIndex, rgOrder, 1, NULL), E_INVALIDARG);
	
	fTestResult = TEST_PASS;

CLEANUP:
	return fTestResult;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(35)
//*-----------------------------------------------------------------------
// @mfunc cPropSets == 0 and rgPropSets == NULL =>S_OK
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateIndex::Variation_35()
{ 
	BOOL				fTestResult = TEST_FAIL;
	DBINDEX_COL_ORDER	rgOrder[]={DBINDEX_COL_ORDER_ASC};

	TESTC_(m_hr = CCNDropIndexFromOrdinals(m_pTable, NULL, m_rgColumnDesc, m_cColumnDesc, 
		1, &m_nIndex, rgOrder, 0, NULL), S_OK);

	fTestResult	= TEST_PASS;
	
CLEANUP:
	return fTestResult;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(36)
//*-----------------------------------------------------------------------
// @mfunc cProperties == 0 and rgProperties != NULL in a rgPropertySets element => S_OK
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateIndex::Variation_36()
{ 
	BOOL				fTestResult = TEST_FAIL;
	DBPROPSET			rgPropSet[1];
	DBPROP				rgProp[1];
	DBINDEX_COL_ORDER	rgOrder[]={DBINDEX_COL_ORDER_ASC};

//	FILL_PROP_SET(rgPropSet[0], 0, rgProp, DBPROPSET_INDEX)
	rgPropSet[0].cProperties		= 0;
	rgPropSet[0].rgProperties		= rgProp;
	rgPropSet[0].guidPropertySet	= DBPROPSET_INDEX;

	TESTC_(m_hr = CCNDropIndexFromOrdinals(m_pTable, NULL, m_rgColumnDesc, m_cColumnDesc, 
		1, &m_nIndex, rgOrder, NUMELEM(rgPropSet), rgPropSet), S_OK);

	fTestResult	= TEST_PASS;
	
CLEANUP:
	return fTestResult;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(37)
//*-----------------------------------------------------------------------
// @mfunc cProperties == 0 and rgProperties == NULL in an rgPropertySets element => S_OK
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateIndex::Variation_37()
{ 
	BOOL				fTestResult = TEST_FAIL;
	DBPROPSET			rgPropSet[1];
	DBINDEX_COL_ORDER	rgOrder[]={DBINDEX_COL_ORDER_ASC};

	rgPropSet[0].cProperties	= 0;
	rgPropSet[0].rgProperties	= NULL;
	rgPropSet[0].guidPropertySet= DBPROPSET_ROWSET;
	TESTC_(m_hr = CCNDropIndexFromOrdinals(m_pTable, NULL, m_rgColumnDesc, m_cColumnDesc, 
		1, &m_nIndex, rgOrder, NUMELEM(rgPropSet), rgPropSet), S_OK);

	fTestResult	= TEST_PASS;
	
CLEANUP:
	return fTestResult;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(38)
//*-----------------------------------------------------------------------
// @mfunc cProperties != 0 and rgProperties == NULL in an element of rgPropertySets => E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateIndex::Variation_38()
{ 
	BOOL				fTestResult = TEST_PASS;
	DBINDEX_COL_ORDER	rgOrder[]={DBINDEX_COL_ORDER_ASC};
	DBPROPSET			rgPropSet[1];

	rgPropSet[0].cProperties		= 2;
	rgPropSet[0].rgProperties		= NULL;
	rgPropSet[0].guidPropertySet	= DBPROPSET_INDEX;
	
	TESTC_(m_hr = CCNDropIndexFromOrdinals(m_pTable, NULL, m_rgColumnDesc, m_cColumnDesc, 
		1, &m_nIndex, rgOrder, NUMELEM(rgPropSet), rgPropSet), E_INVALIDARG);

	fTestResult	= TEST_PASS;
	
CLEANUP:
	return fTestResult;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(39)
//*-----------------------------------------------------------------------
// @mfunc eIndexColumnDesc invalid for at least one column => E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateIndex::Variation_39()
{ 
	BOOL					fTestResult = TEST_FAIL;
	DBINDEXCOLUMNDESC		rgIndexColumnDesc[1];
	const DBINDEX_COL_ORDER	InvalidValue = 100;

	rgIndexColumnDesc[0].eIndexColOrder = InvalidValue;
	rgIndexColumnDesc[0].pColumnID		= &m_rgColumnDesc[m_nIndex-1].dbcid;
	
	TESTC_(m_hr = m_pCIIndexDefinition->CreateCheckAndDropIndex(&m_pTable->GetTableID(), NULL, 
		NUMELEM(rgIndexColumnDesc), rgIndexColumnDesc), E_INVALIDARG);
	
	fTestResult = TEST_PASS;

CLEANUP:
	return fTestResult;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(40)
//*-----------------------------------------------------------------------
// @mfunc the index is made of all columns
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateIndex::Variation_40()
{ 
	BOOL				fTestResult = TEST_FAIL;
	DBINDEX_COL_ORDER	*rgOrder=NULL; 
	ULONG				i;

	SAFE_ALLOC(rgOrder, DBINDEX_COL_ORDER, m_cIndexCols);
	
	for (i=0; i<m_cIndexCols; i++)
		rgOrder[i]			= DBINDEX_COL_ORDER_ASC;

	TESTC_(m_hr = CCNDropIndexFromOrdinals(m_pTable, NULL, m_rgColumnDesc, m_cColumnDesc, 
						m_cIndexCols, m_rgIndexCols, rgOrder), S_OK);

	fTestResult = TEST_PASS;

CLEANUP:
	SAFE_FREE(rgOrder);
	return fTestResult;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(41)
//*-----------------------------------------------------------------------
// @mfunc Try to create a duplicate index => DB_E_DUPLICATEINDEX
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateIndex::Variation_41()
{ 
	BOOL				fTestResult = TEST_PASS;
	BOOL				fExists;
	DBID				IndexID;
	DBINDEX_COL_ORDER	rgOrder[]={DBINDEX_COL_ORDER_ASC}; 
	WCHAR				wszIndexName[] = L"Indexul1";
	DB_LORDINAL			rgColNo[1];
	DB_LORDINAL			*prgColNo = rgColNo;
	DBORDINAL			nColNo = NUMELEM(rgColNo);
	DBINDEXCOLUMNDESC	rgIndexColumnDesc[1];

	// create an index through SQL and another one with IIndexDefinition
	IndexID.eKind						= DBKIND_NAME;
	IndexID.uName.pwszName				= wszIndexName;
	rgIndexColumnDesc[0].pColumnID		= &m_rgColumnDesc[m_nIndex-1].dbcid;
	rgIndexColumnDesc[0].eIndexColOrder	= DBINDEX_COL_ORDER_ASC;

	if (!m_pTable->GetCommandSupOnCTable())
	{
		odtLog << "can not handle commands on tables\n";
		goto STEP2;
	}
	// Execute statement
	rgColNo[0] = m_nIndex-1;
	CHECK(m_hr = m_pTable->ExecuteCommand(CREATE_INDEX, IID_NULL, wszIndexName, NULL, &nColNo, &prgColNo), S_OK);
	if (S_OK == m_hr && CHECK(m_hr = m_pCIIndexDefinition->DoesIndexExist(&m_pTable->GetTableID(), &IndexID, &fExists), S_OK) && fExists)
	{
		// check that is impossible to create another index with the same name on the same table
		if (!CHECK(m_hr = m_pCIIndexDefinition->CreateIndex(&m_pTable->GetTableID(), &IndexID, 
			NUMELEM(rgIndexColumnDesc), rgIndexColumnDesc, 0, NULL, NULL), DB_E_DUPLICATEINDEXID))
			fTestResult = TEST_FAIL;
	}
	else
		fTestResult = TEST_FAIL;

	// drop indexul1
	if (!CHECK(m_hr	= m_pCIIndexDefinition->DropIndex(&m_pTable->GetTableID(), &IndexID), S_OK))
	{
		fTestResult = TEST_FAIL;
		goto CLEANUP;
	}

STEP2:
	m_hr = CNCIndexFromOrdinals(m_pTable, &IndexID, NULL, m_rgColumnDesc, m_cColumnDesc, 1, &m_nIndex, rgOrder);
	if (S_OK == m_hr)
	{
		// check that is impossible to create another index with the same name on the same table
		if (!CHECK(m_hr = m_pCIIndexDefinition->CreateIndex(&m_pTable->GetTableID(), &IndexID, 
			NUMELEM(rgIndexColumnDesc), rgIndexColumnDesc, 0, NULL, NULL), DB_E_DUPLICATEINDEXID))
			fTestResult = TEST_FAIL;

		// check that is impossible to create another index with the same name on the same table
		if (m_pTable->GetCommandSupOnCTable())
		{
			m_hr = m_pTable->ExecuteCommand(CREATE_INDEX, IID_NULL, wszIndexName, NULL, &nColNo, &prgColNo);
			if (!FAILED(m_hr))
			{
				odtLog << "index duplicate by SQL command\n";
				fTestResult = TEST_FAIL;
			}
		}
	}
	else
	{
		odtLog << "could not properly create the index by IIndexDefinition\n";
		fTestResult = TEST_FAIL;
	}
	// drop indexul1
	CHECK(m_pCIIndexDefinition->DropIndex(&m_pTable->GetTableID(), &IndexID), S_OK);
CLEANUP:
	return fTestResult;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(42)
//*-----------------------------------------------------------------------
// @mfunc Create an index on a single column table
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateIndex::Variation_42()
{ 
	CTable				*pTable=NULL;
	BOOL				fTestResult = TEST_FAIL;
	DBINDEX_COL_ORDER	rgOrder[]={DBINDEX_COL_ORDER_ASC};
	DBORDINAL			nCol=1;

	pTable = new CTable(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);
	TESTC(NULL != pTable);
	pTable->SetBuildColumnDesc(FALSE);
	pTable->SetColumnDesc(&m_rgColumnDesc[m_nIndex-1], 1);

	TESTC_(m_hr = pTable->CreateTable(0, 0), S_OK);

	TESTC_(m_hr = CCNDropIndexFromOrdinals(pTable, NULL, &m_rgColumnDesc[m_nIndex-1], 1, 1, &nCol, rgOrder), S_OK);
	
	fTestResult =TEST_PASS;
	
CLEANUP:
	if (pTable)
	{
		pTable->DropTable();
		pTable->SetColumnDesc(NULL, 0);
		delete pTable;
	}
	return fTestResult;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(43)
//*-----------------------------------------------------------------------
// @mfunc all indexes on a 2 columns table
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateIndex::Variation_43()
{ 
	CTable		*pTable = NULL;
	BOOL		fTestResult = TEST_FAIL;

	pTable = new CTable(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);
	TESTC(NULL != pTable);
	pTable->SetBuildColumnDesc(FALSE);
	pTable->SetColumnDesc(&m_rgColumnDesc[0], 2);

	TESTC_(m_hr = pTable->CreateTable(0, 0), S_OK);

	fTestResult = SUCCEEDED(DoAllIndexes(pTable, 2))? TEST_PASS: TEST_FAIL;

CLEANUP:
	if (pTable)
	{
		pTable->DropTable();
		pTable->SetColumnDesc(NULL, 0);
		delete pTable;
	}
	return fTestResult;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(44)
//*-----------------------------------------------------------------------
// @mfunc pIndexID and ppIndexID both not NULL => S_OK
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateIndex::Variation_44()
{
	TBEGIN
	CDBIDPtr			pIndexID;
	DBID				IndexID;
	DBINDEX_COL_ORDER	rgOrder[]={DBINDEX_COL_ORDER_ASC};

	IndexID.eKind			= DBKIND_NAME;
	IndexID.uName.pwszName	= L"BiancoENero";
	TESTC_(m_hr = CNCIndexFromOrdinals(m_pTable, &IndexID, (DBID**)pIndexID, 
		m_rgColumnDesc, m_cColumnDesc, 1, &m_nIndex, rgOrder), S_OK);
	
	COMPARE(CheckReturnedDBID(&IndexID, (DBID**)pIndexID), TRUE);
	CHECK(m_pCIIndexDefinition->DropIndex(&m_pTable->GetTableID(), (DBID*)pIndexID), S_OK);

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(45)
//*-----------------------------------------------------------------------
// @mfunc valid, yet inexistent table name => DB_E_NOTABLE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateIndex::Variation_45()
{ 
	BOOL				fTestResult = TEST_FAIL;
	LPWSTR				pwszTableName = wcsDuplicate(m_pTable->GetTableName());
	DBID				TableID;
	DBINDEXCOLUMNDESC	rgIndexColumnDesc[1];

	rgIndexColumnDesc[0].eIndexColOrder = DBINDEX_COL_ORDER_ASC;
	rgIndexColumnDesc[0].pColumnID		= &m_rgColumnDesc[0].dbcid;
	TableID.eKind = DBKIND_NAME;
	m_pTable->MakeTableName(NULL);
	TableID.uName.pwszName	= m_pTable->GetTableName();

	TESTC_(m_hr = m_pCIIndexDefinition->CreateCheckAndDropIndex(&TableID, NULL, 1, rgIndexColumnDesc), DB_E_NOTABLE);

	fTestResult = TEST_PASS;

CLEANUP:
	m_pTable->SetTableName(pwszTableName);
	SAFE_FREE(pwszTableName);
	return fTestResult;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(46)
//*-----------------------------------------------------------------------
// @mfunc inexistent column name => DB_E_NOCOLUMN
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateIndex::Variation_46()
{ 
	BOOL				fTestResult = TEST_FAIL;
	DBID				colID;
	DBINDEXCOLUMNDESC	rgIndexColumnDesc[1];

	rgIndexColumnDesc[0].eIndexColOrder = DBINDEX_COL_ORDER_ASC;
	rgIndexColumnDesc[0].pColumnID		= &colID;
	colID.eKind				= DBKIND_NAME;
	colID.uName.pwszName	= L"NumeDeColoana";

	TESTC_(m_hr = m_pCIIndexDefinition->CreateCheckAndDropIndex(&m_pTable->GetTableID(), NULL, 1, rgIndexColumnDesc), DB_E_NOCOLUMN);

	fTestResult = TEST_PASS;

CLEANUP:
	return fTestResult;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(47)
//*-----------------------------------------------------------------------
// @mfunc all DBKIND on pTableID
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateIndex::Variation_47()
{ 
	BOOL				fTestResult = TEST_PASS;
	DBID				TableID;
	DBINDEXCOLUMNDESC	rgIndexColumnDesc[1];

	rgIndexColumnDesc[0].eIndexColOrder = DBINDEX_COL_ORDER_ASC;
	rgIndexColumnDesc[0].pColumnID		= &m_rgColumnDesc[0].dbcid;
	
	memset(&TableID, 0, sizeof(DBID));
	TableID.uName.pwszName	= m_pTable->GetTableName();
	
	// DBKIND_NAME
	TableID.eKind = DBKIND_NAME;
	if (!CHECK(m_hr = m_pCIIndexDefinition->CreateCheckAndDropIndex(&TableID, NULL, 1, rgIndexColumnDesc), S_OK))
	{
		odtLog << "ERROR: DBKIND_NAME\n";
		fTestResult = TEST_FAIL;
	}

	// DBKIND_GUID_NAME
	TableID.eKind = DBKIND_GUID_NAME;
	TableID.uGuid.guid = guidModule;
	if (!CHECK(m_hr = m_pCIIndexDefinition->CreateCheckAndDropIndex(&TableID, NULL, 1, rgIndexColumnDesc), DB_E_NOTABLE))
	{
		odtLog << "ERROR: DBKIND_GUID_NAME\n";
		fTestResult = TEST_FAIL;
	}

	// DBKIND_PGUID_NAME
	TableID.eKind = DBKIND_PGUID_NAME;
	TableID.uGuid.pguid = &guidModule;
	if (!CHECK(m_hr = m_pCIIndexDefinition->CreateCheckAndDropIndex(&TableID, NULL, 1, rgIndexColumnDesc), DB_E_NOTABLE))
	{
		odtLog << "ERROR: DBKIND_PGUID_NAME\n";
		fTestResult = TEST_FAIL;
	}

	// DBKIND_GUID
	TableID.eKind = DBKIND_GUID;
	if (!CHECK(m_hr = m_pCIIndexDefinition->CreateCheckAndDropIndex(&TableID, NULL, 1, rgIndexColumnDesc), DB_E_NOTABLE))
	{
		odtLog << "ERROR: DBKIND_GUID\n";
		fTestResult = TEST_FAIL;
	}

	// DBKIND_GUID_PROPID
	TableID.uName.ulPropid	= 0;
	TableID.uName.pwszName = NULL;
	TableID.eKind = DBKIND_GUID_PROPID;
	if (!CHECK(m_hr = m_pCIIndexDefinition->CreateCheckAndDropIndex(&TableID, NULL, 1, rgIndexColumnDesc), DB_E_NOTABLE))
	{
		odtLog << "ERROR: DBKIND_GUID_PROPID\n";
		fTestResult = TEST_FAIL;
	}

	// DBKIND_PROPID
	TableID.eKind = DBKIND_PROPID;
	if (!CHECK(m_hr = m_pCIIndexDefinition->CreateCheckAndDropIndex(&TableID, NULL, 1, rgIndexColumnDesc), DB_E_NOTABLE))
	{
		odtLog << "ERROR: DBKIND_PROPID\n";
		fTestResult = TEST_FAIL;
	}

	// DBKIND_PGUID_PROPID
	TableID.eKind = DBKIND_PGUID_PROPID;
	TableID.uGuid.pguid = &guidModule;
	if (!CHECK(m_hr = m_pCIIndexDefinition->CreateCheckAndDropIndex(&TableID, NULL, 1, rgIndexColumnDesc), DB_E_NOTABLE))
	{
		odtLog << "ERROR: DBKIND_PGUID_PROPID\n";
		fTestResult = TEST_FAIL;
	}

	return fTestResult;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(48)
//*-----------------------------------------------------------------------
// @mfunc all DBKIND on pColumnID
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateIndex::Variation_48()
{ 
	TBEGIN
	DBID				colID, *pIndexID=NULL;
	DBINDEXCOLUMNDESC	rgIndexColumnDesc[1];
	DBORDINAL			*rgIndexCols	= NULL;
	DBORDINAL			cIndexCols		= 0;

	FindIndexableColumns(m_rgColumnDesc, m_cColumnDesc, &rgIndexCols, &cIndexCols);
	TESTC(0 < cIndexCols);

	rgIndexColumnDesc[0].eIndexColOrder = DBINDEX_COL_ORDER_ASC;
	rgIndexColumnDesc[0].pColumnID		= &colID;
	
	colID.uName.pwszName	= m_rgColumnDesc[rgIndexCols[0]].dbcid.uName.pwszName;
	colID.uGuid.pguid		= &guidModule;
	
	// DBKIND_NAME
	colID.eKind = DBKIND_NAME;
	m_hr = m_pCIIndexDefinition->CreateIndex(&m_pTable->GetTableID(), NULL, 1, rgIndexColumnDesc, 0, NULL, &pIndexID);
	if (!CHECK(m_hr, S_OK))
		odtLog << "DBKIND_NAME\n";
	ReleaseDBID(pIndexID);
	pIndexID=NULL;
	
	// DBKIND_GUID_NAME
	colID.eKind = DBKIND_GUID_NAME;
	colID.uGuid.guid = guidModule;

	m_hr = m_pCIIndexDefinition->CreateIndex(&m_pTable->GetTableID(), NULL, 1, rgIndexColumnDesc, 0, NULL, &pIndexID);
	if (!CHECK(m_hr, DB_E_NOCOLUMN))
		odtLog << "DBKIND_GUID_NAME\n";
	ReleaseDBID(pIndexID);
	pIndexID=NULL;
	// DBKIND_PGUID_NAME
	colID.eKind = DBKIND_PGUID_NAME;
	colID.uGuid.pguid = &guidModule;
	m_hr = m_pCIIndexDefinition->CreateIndex(&m_pTable->GetTableID(), NULL, 1, rgIndexColumnDesc, 0, NULL, &pIndexID);
	if (!CHECK(m_hr, DB_E_NOCOLUMN))
		odtLog << "DBKIND_PGUID_NAME\n";
	ReleaseDBID(pIndexID);
	pIndexID=NULL;
	// DBKIND_GUID
	colID.eKind = DBKIND_GUID;
	colID.uGuid.guid = guidModule;
	m_hr = m_pCIIndexDefinition->CreateIndex(&m_pTable->GetTableID(), NULL, 1, rgIndexColumnDesc, 0, NULL, &pIndexID);
	if (!CHECK(m_hr, DB_E_NOCOLUMN))
		odtLog << "DBKIND_GUID\n";
	ReleaseDBID(pIndexID);
	pIndexID=NULL;
	colID.uName.ulPropid	= 0;
	colID.uName.pwszName	= NULL;
	// DBKIND_GUID_PROPID
	colID.eKind = DBKIND_GUID_PROPID;
	colID.uGuid.guid = guidModule;
	m_hr = m_pCIIndexDefinition->CreateIndex(&m_pTable->GetTableID(), NULL, 1, rgIndexColumnDesc, 0, NULL, &pIndexID);
	if (!CHECK(m_hr, DB_E_NOCOLUMN))
		odtLog << "DBKIND_GUID_PROPID\n";
	ReleaseDBID(pIndexID);
	pIndexID=NULL;
	// DBKIND_PROPID
	colID.eKind = DBKIND_PROPID;
	m_hr = m_pCIIndexDefinition->CreateIndex(&m_pTable->GetTableID(), NULL, 1, rgIndexColumnDesc, 0, NULL, &pIndexID);
	if (!CHECK(m_hr, DB_E_NOCOLUMN))
		odtLog << "DBKIND_PROPID\n";
	ReleaseDBID(pIndexID);
	pIndexID=NULL;
	// DBKIND_PGUID_PROPID
	colID.eKind = DBKIND_PGUID_PROPID;
	colID.uGuid.pguid = &guidModule;
	m_hr = m_pCIIndexDefinition->CreateIndex(&m_pTable->GetTableID(), NULL, 1, rgIndexColumnDesc, 0, NULL, &pIndexID);
	if (!CHECK(m_hr, DB_E_NOCOLUMN))
		odtLog << "DBKIND_PGUID_PROPID\n";
	ReleaseDBID(pIndexID);

CLEANUP:
	SAFE_FREE(rgIndexCols);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(49)
//*-----------------------------------------------------------------------
// @mfunc all DBKIND on pIndexID
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateIndex::Variation_49()
{ 
	BOOL				fTestResult = TEST_PASS;
	DBID				IndexID;
	DBINDEXCOLUMNDESC	rgIndexColumnDesc[1];

	rgIndexColumnDesc[0].eIndexColOrder = DBINDEX_COL_ORDER_ASC;
	rgIndexColumnDesc[0].pColumnID		= &m_rgColumnDesc[0].dbcid;
	
	memset(&IndexID, 0, sizeof(DBID));
	IndexID.uName.pwszName	= L"galusca";
	
	// DBKIND_NAME
	IndexID.eKind = DBKIND_NAME;
	odtLog << "DBKIND_NAME: ";
	m_hr = m_pCIIndexDefinition->CreateCheckAndDropIndex(&m_pTable->GetTableID(), &IndexID, 1, rgIndexColumnDesc);
	if (S_OK ==m_hr)
		odtLog << "CREATED\n";
	else
	{
		odtLog << "NOT created\n";
		if (!CHECK(m_hr, DB_E_BADINDEXID))
			fTestResult = TEST_FAIL;
	}

	// DBKIND_GUID_NAME
	IndexID.eKind = DBKIND_GUID_NAME;
	IndexID.uGuid.guid = guidModule;
	odtLog << "DBKIND_GUID_NAME: ";
	m_hr = m_pCIIndexDefinition->CreateCheckAndDropIndex(&m_pTable->GetTableID(), &IndexID, 1, rgIndexColumnDesc);
	if (S_OK ==m_hr)
		odtLog << "CREATED\n";
	else
	{
		odtLog << "NOT created\n";
		if (!CHECK(m_hr, DB_E_BADINDEXID))
			fTestResult = TEST_FAIL;
	}
	
	// DBKIND_PGUID_NAME
	IndexID.eKind = DBKIND_PGUID_NAME;
	IndexID.uGuid.pguid = &guidModule;
	odtLog << "DBKIND_PGUID_NAME: ";
	m_hr = m_pCIIndexDefinition->CreateCheckAndDropIndex(&m_pTable->GetTableID(), &IndexID, 1, rgIndexColumnDesc);
	if (S_OK ==m_hr)
		odtLog << "CREATED\n";
	else
	{
		odtLog << "NOT created\n";
		if (!CHECK(m_hr, DB_E_BADINDEXID))
			fTestResult = TEST_FAIL;
	}

	// DBKIND_GUID
	IndexID.eKind = DBKIND_GUID;
	odtLog << "DBKIND_GUID: ";
	m_hr = m_pCIIndexDefinition->CreateCheckAndDropIndex(&m_pTable->GetTableID(), &IndexID, 1, rgIndexColumnDesc);
	if (S_OK ==m_hr)
		odtLog << "CREATED\n";
	else
	{
		odtLog << "NOT created\n";
		if (!CHECK(m_hr, DB_E_BADINDEXID))
			fTestResult = TEST_FAIL;
	}
	IndexID.uName.ulPropid = 0;
	IndexID.uName.pwszName = NULL;

	// DBKIND_GUID_PROPID
	IndexID.eKind = DBKIND_GUID_PROPID;
	odtLog << "DBKIND_GUID_PROPID: ";

	m_hr = m_pCIIndexDefinition->CreateCheckAndDropIndex(&m_pTable->GetTableID(), &IndexID, 1, rgIndexColumnDesc);
	if (S_OK ==m_hr)
		odtLog << "CREATED\n";
	else
	{
		odtLog << "NOT created\n";
		if (!CHECK(m_hr, DB_E_BADINDEXID))
			fTestResult = TEST_FAIL;
	}

	// DBKIND_PROPID
	IndexID.eKind = DBKIND_PROPID;
	odtLog << "DBKIND_PROPID: ";
	m_hr = m_pCIIndexDefinition->CreateCheckAndDropIndex(&m_pTable->GetTableID(), &IndexID, 1, rgIndexColumnDesc);
	if (S_OK ==m_hr)
		odtLog << "CREATED\n";
	else
	{
		odtLog << "NOT created\n";
		if (!CHECK(m_hr, DB_E_BADINDEXID))
			fTestResult = TEST_FAIL;
	}

	// DBKIND_PGUID_PROPID
	IndexID.eKind = DBKIND_PGUID_PROPID;
	IndexID.uGuid.pguid = &guidModule;
	odtLog << "DBKIND_PGUID_PROPID: ";
	m_hr = m_pCIIndexDefinition->CreateCheckAndDropIndex(&m_pTable->GetTableID(), &IndexID, 1, rgIndexColumnDesc);
	if (S_OK ==m_hr)
		odtLog << "CREATED\n";
	else
	{
		odtLog << "NOT created\n";
		if (!CHECK(m_hr, DB_E_BADINDEXID))
			fTestResult = TEST_FAIL;
	}
	
	return fTestResult;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(50)
//*-----------------------------------------------------------------------
// @mfunc NULL pColumnID in an element of rgIndexColumnDesc => DB_E_NOCOLUMN
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateIndex::Variation_50()
{ 
	BOOL				fTestResult = TEST_FAIL;
	DBINDEXCOLUMNDESC	rgIndexColumnDesc[1];

	rgIndexColumnDesc[0].eIndexColOrder = DBINDEX_COL_ORDER_ASC;
	rgIndexColumnDesc[0].pColumnID		= NULL;

	TESTC_(m_hr = m_pCIIndexDefinition->CreateCheckAndDropIndex(&m_pTable->GetTableID(), NULL, 1, rgIndexColumnDesc), DB_E_NOCOLUMN);
	
	fTestResult = TEST_PASS;

CLEANUP:
	return fTestResult;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(51)
//*-----------------------------------------------------------------------
// @mfunc DB_NULLID pColumnID in an element of rgIndexColumnDesc => DB_E_NOCOLUMN
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateIndex::Variation_51()
{ 
	BOOL				fTestResult = TEST_FAIL;
	DBINDEXCOLUMNDESC	rgIndexColumnDesc[1];

	rgIndexColumnDesc[0].eIndexColOrder = DBINDEX_COL_ORDER_ASC;
	rgIndexColumnDesc[0].pColumnID		= (struct tagDBID*)&DB_NULLID;

	TESTC_(m_hr = m_pCIIndexDefinition->CreateCheckAndDropIndex(&m_pTable->GetTableID(), NULL, 1, rgIndexColumnDesc), DB_E_NOCOLUMN);

	fTestResult = TEST_PASS;

CLEANUP:
	return fTestResult;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(52)
//*-----------------------------------------------------------------------
// @mfunc empty name for a column in an element of rgIndexColumnDesc => DB_E_NOCOLUMN
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateIndex::Variation_52()
{ 
	BOOL				fTestResult = TEST_FAIL;
	DBID				colID;
	DBINDEXCOLUMNDESC	rgIndexColumnDesc[1];

	rgIndexColumnDesc[0].eIndexColOrder = DBINDEX_COL_ORDER_ASC;
	rgIndexColumnDesc[0].pColumnID		= &colID;
	colID.eKind				= DBKIND_NAME;
	colID.uName.pwszName	= L"";

	TESTC_(m_hr = m_pCIIndexDefinition->CreateCheckAndDropIndex(&m_pTable->GetTableID(), NULL, 1, rgIndexColumnDesc), DB_E_NOCOLUMN);
	
	fTestResult = TEST_PASS;

CLEANUP:
	return fTestResult;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(53)
//*-----------------------------------------------------------------------
// @mfunc NULL column name in an element of rgIndexColumnDesc => DB_E_NOCOLUMN
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateIndex::Variation_53()
{ 
	BOOL				fTestResult = TEST_FAIL;
	DBID				colID;
	DBINDEXCOLUMNDESC	rgIndexColumnDesc[1];

	rgIndexColumnDesc[0].eIndexColOrder = DBINDEX_COL_ORDER_ASC;
	rgIndexColumnDesc[0].pColumnID		= &colID;
	colID.eKind							= DBKIND_NAME;
	colID.uName.pwszName				= NULL;

	TESTC_(m_hr = m_pCIIndexDefinition->CreateCheckAndDropIndex(&m_pTable->GetTableID(), NULL, 1, rgIndexColumnDesc), DB_E_NOCOLUMN);
		
	fTestResult = TEST_PASS;

CLEANUP:
	return fTestResult;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(54)
//*-----------------------------------------------------------------------
// @mfunc Set DBPROP_INDEX_AUTOUPDATE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateIndex::Variation_54()
{ 
	BOOL				fTestResult = TEST_FAIL;

	if (CreateIndexWithOneLogicalProperty(DBPROP_INDEX_AUTOUPDATE))
		fTestResult = TEST_PASS;

	return fTestResult;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(55)
//*-----------------------------------------------------------------------
// @mfunc Set DBPROP_INDEX_CLUSTERED
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateIndex::Variation_55()
{ 
	BOOL	fTestResult = TEST_PASS;

	if (!CreateIndexWithOneLogicalProperty(DBPROP_INDEX_CLUSTERED))
		fTestResult = TEST_FAIL;
	return fTestResult;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(56)
//*-----------------------------------------------------------------------
// @mfunc Set DBPROP_INDEX_NULLS
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateIndex::Variation_56()
{
	TBEGIN
	HRESULT				hr;
	BOOL				fExists;
	const int			n=4;
	VARIANT				values[n];
	ULONG				i;
	LPWSTR				pwszSQLText=NULL; 
	CDBIDPtr			IndexID;
	DBPROPSET			rgPropSets[1];
	DBPROP				rgProp[1];
	DBORDINAL			oldIndex;
	DBINDEX_COL_ORDER	rgOrder[]={DBINDEX_COL_ORDER_ASC};
	CCol				col;
	DBORDINAL			nIndex;

	TESTC(0 < m_cColumnDesc);
	// the classical aproach: try all the values
	for (i=0; i<n; values[i++].vt = VT_I4);
	V_I4(&values[0]) = DBPROPVAL_IN_DISALLOWNULL;
	V_I4(&values[1]) = DBPROPVAL_IN_IGNORENULL;
	V_I4(&values[2]) = DBPROPVAL_IN_IGNOREANYNULL;
	V_I4(&values[3]) = DBPROPVAL_IN_ALLOWNULL;

	COMPARE(CreateIndexWithOneProperty(n, values, DBPROP_INDEX_NULLS), TRUE);

	// if m_nIndex column m_nIndex is not NULLable go to the end
	for (i=0, fExists = FALSE; i < m_cIndexCols && !fExists; i++)
	{
		col = m_pTable->GetColInfoForUpdate(m_rgIndexCols[i]);
		fExists =  (1 == col.GetNullable()) && col.GetUpdateable();
	}

	if (fExists)
	{
		nIndex = col.GetColNum();

		// create an index on m_nIndex, with DBPROP_INDEX_NULLS = DBPROPVAL_IN_DISALLOWNULL
		odtLog << "add an index with DBPROP_INDEX_NULLS = DBPROP_IN_DISALLOWNULL\n";
		FILL_PROP_SET(rgPropSets[0], NUMELEM(rgProp), rgProp, DBPROPSET_INDEX)
		FILL_PROP(rgProp[0], DBPROP_INDEX_NULLS, VT_I4, V_I4, DBPROPVAL_IN_DISALLOWNULL, DBPROPOPTIONS_REQUIRED)

		hr = CNCIndexFromOrdinals(m_pTable, NULL, (DBID**)IndexID, m_rgColumnDesc, m_cColumnDesc, 
			1, &nIndex, rgOrder, NUMELEM(rgPropSets), rgPropSets);
		if (FAILED(hr))
		{
			CHECK(hr, DB_E_ERRORSOCCURRED);
			odtLog << "an index with DBPROP_INDEX_NULLS = DBPROP_IN_DISALLOWNULL could not be  created\n";
			goto CLEANUP;
		}
		odtLog << "an index with DBPROP_INDEX_NULLS = DBPROP_IN_DISALLOWNULL was created\n";

		if (m_pTable->GetCommandSupOnCTable())
		{
			// try to insert a row with a NULL index entry => should not be allowed
			odtLog << "try to insert a row with a NULL index entry\n";
			// makes sure that not all the entries are NULL by setting a pseudo index
			oldIndex = m_pTable->SetIndexColumn((nIndex+1)%m_cColumnDesc+1);	
			CHECK(hr = m_pTable->Insert(101, PRIMARY, FALSE, &pwszSQLText, TRUE), S_OK);	// TRUE says try to insert NULLs
			m_pTable->SetIndexColumn(oldIndex);	//remove the pseudoindex

			hr = m_pTable->BuildCommand(pwszSQLText, IID_NULL, EXECUTE_IFNOERROR, 0, NULL, NULL, NULL, NULL, NULL);
			CHECK(hr, DB_E_INTEGRITYVIOLATION );
			if (SUCCEEDED(hr))
			{
				odtLog << "ERROR: NULL entry inserted!\n";
				goto CLEANUP;
			}
			else
				odtLog << "OK: could not insert NULL\n";
		}

		// check that when the index is dropped the row can be inserted
		TESTC(m_pCIIndexDefinition->DropIndex(&m_pTable->GetTableID(), (DBID*)IndexID));

		if (m_pTable->GetCommandSupOnCTable())
		{
			TESTC_(m_hr = m_pTable->BuildCommand(pwszSQLText, IID_NULL, EXECUTE_IFNOERROR, 
								0, NULL, NULL, NULL, NULL, NULL), S_OK);
		}
	}
	
CLEANUP:
	m_hr = m_pTable->Delete();
	SAFE_FREE(pwszSQLText);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(57)
//*-----------------------------------------------------------------------
// @mfunc Set DBPROP_INDEX_PRIMARYKEY
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateIndex::Variation_57()
{ 
	TBEGIN
	CTable				*pTable = NULL;
	DBCOLUMNDESC		*rgNewColumnDesc = NULL;
	DBORDINAL			cNewColumnDesc = m_cColumnDesc-m_nIndex+1;
	DBINDEXCOLUMNDESC	rgIndexColumnDesc[1];
	DBPROPSET			rgPropSet[1];
	DBPROP				rgProp[1];
	CDBIDPtr			IndexID;

	// check the index on the current table (m_pTable)
	COMPARE(CreateIndexWithOneLogicalProperty(DBPROP_INDEX_PRIMARYKEY), TRUE);

	// create a new table, set and check the column as primary key
	pTable = new CTable(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);
	TESTC(NULL != pTable);
	DuplicateColumnDesc(&m_rgColumnDesc[m_nIndex-1], cNewColumnDesc, &rgNewColumnDesc);
	pTable->SetColumnDesc(rgNewColumnDesc, cNewColumnDesc);
	pTable->SetBuildColumnDesc(FALSE);
	TESTC_(SetProperty(&rgNewColumnDesc[0].rgPropertySets, &rgNewColumnDesc[0].cPropertySets,
		DBPROP_COL_NULLABLE, VT_BOOL, (LPVOID)VARIANT_FALSE, DBPROPOPTIONS_REQUIRED, DB_NULLID, DBPROPSET_COLUMN), TRUE);
	
	if (S_OK != (m_hr = pTable->CreateTable(5, 0)))
	{
		odtLog << "abort: there are errors in creating a table\n";
		goto CLEANUP;
	}
	
	//create an index with DBPROP_INDEX_PRIMARYKEY property set to variant TRUE
	rgIndexColumnDesc[0].eIndexColOrder = DBINDEX_COL_ORDER_ASC;
	rgIndexColumnDesc[0].pColumnID		= &rgNewColumnDesc[0].dbcid;
	FILL_PROP_SET(rgPropSet[0], NUMELEM(rgProp), rgProp, DBPROPSET_INDEX)
	FILL_PROP(rgProp[0], DBPROP_INDEX_PRIMARYKEY, VT_BOOL, V_BOOL, VARIANT_TRUE, DBPROPOPTIONS_REQUIRED)

	if (CHECK(m_hr = m_pCIIndexDefinition->CreateIndexAndCheck(&pTable->GetTableID(), NULL, 1, rgIndexColumnDesc, 
			1, rgPropSet, (DBID**)IndexID), S_OK))
	{
		if (S_OK == (m_hr = pTable->Insert(2,PRIMARY,TRUE,NULL)) )
		{
			odtLog << "a record with an already existing key was successfully inserted in the database!\n";
		}
	}

	COMPARE(m_pCIIndexDefinition->DropIndex(&pTable->GetTableID(), (DBID*)IndexID), S_OK);

	// at this point, the table should forget about column 0 being a primary key column; has it?
	if (!CHECK(m_hr = pTable->Insert(2,PRIMARY,TRUE,NULL), S_OK))
	{
		odtLog << "a record with an already existing key couldn't successfully be inserted in the database!\n";
	}
	else
	{
		// the column contains a duplicate, try to open an index on it, defining the col as primary key
		m_hr = m_pCIIndexDefinition->CreateCheckAndDropIndex(&pTable->GetTableID(), NULL, 1, rgIndexColumnDesc, 1, rgPropSet);
		if (S_OK == m_hr)
		{
			odtLog << "could open an index with a primary key on a column containing duplicates!\n";
		}
	}

CLEANUP:
	if (pTable)
	{
		pTable->DropTable();
		delete pTable;
	}
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(58)
//*-----------------------------------------------------------------------
// @mfunc Set DBPROP_INDEX_SORTBOOKMARKS
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateIndex::Variation_58()
{ 
	BOOL	fTestResult = TEST_PASS;

	if (!CreateIndexWithOneLogicalProperty(DBPROP_INDEX_SORTBOOKMARKS))
		fTestResult = TEST_FAIL;
	return fTestResult;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(59)
//*-----------------------------------------------------------------------
// @mfunc Set DBPROP_INDEX_TEMPINDEX
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateIndex::Variation_59()
{ 
	BOOL	fTestResult = TEST_PASS;

	if (!CreateIndexWithOneLogicalProperty(DBPROP_INDEX_TEMPINDEX))
		fTestResult = TEST_FAIL;
	return fTestResult;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(60)
//*-----------------------------------------------------------------------
// @mfunc Set DBPROP_INDEX_TYPE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateIndex::Variation_60()
{ 
	BOOL		fTestResult = TEST_PASS;
	const int	n=4;
	VARIANT		values[n];
	ULONG		i;

	for (i=0; i<n; values[i++].vt = VT_I4);
	V_I4(&values[0]) = DBPROPVAL_IT_BTREE;
	V_I4(&values[1]) = DBPROPVAL_IT_HASH;
	V_I4(&values[2]) = DBPROPVAL_IT_CONTENT;
	V_I4(&values[3]) = DBPROPVAL_IT_OTHER;
	if (!CreateIndexWithOneProperty(n, values, DBPROP_INDEX_TYPE))
		fTestResult = TEST_FAIL;
	return fTestResult;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(61)
//*-----------------------------------------------------------------------
// @mfunc Set DBPROP_INDEX_UNIQUE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateIndex::Variation_61()
{
	TBEGIN
	CTable				*pTable = NULL;
	DBCOLUMNDESC		*rgNewColumnDesc = NULL;
	DBORDINAL			cNewColumnDesc = m_cColumnDesc-m_nIndex+1;
	DBINDEXCOLUMNDESC	rgIndexColumnDesc[1];
	DBPROPSET			rgPropSet[1];
	DBPROP				rgProp[1];
	CDBIDPtr			IndexID;

	TESTC(CreateIndexWithOneLogicalProperty(DBPROP_INDEX_UNIQUE));

	pTable = new CTable(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);
	TESTC(NULL != pTable);
	DuplicateColumnDesc(&m_rgColumnDesc[m_nIndex-1], cNewColumnDesc, &rgNewColumnDesc);
	pTable->SetColumnDesc(rgNewColumnDesc, cNewColumnDesc);
	ReleaseAllColumnPropSets(cNewColumnDesc, rgNewColumnDesc);
	pTable->SetBuildColumnDesc(FALSE);
	TESTC_(SetProperty(&rgNewColumnDesc[0].rgPropertySets, &rgNewColumnDesc[0].cPropertySets,
		DBPROP_COL_NULLABLE, VT_BOOL, (LPVOID)VARIANT_FALSE, DBPROPOPTIONS_REQUIRED, DB_NULLID, DBPROPSET_COLUMN), TRUE);
	
	if (!CHECK(m_hr = pTable->CreateTable(5, 0), S_OK))
	{
		odtLog << "abort: could not properly create the table\n";
		goto CLEANUP;
	}

	// create a index with the unique property
	rgIndexColumnDesc[0].eIndexColOrder = DBINDEX_COL_ORDER_ASC;
	rgIndexColumnDesc[0].pColumnID		= &rgNewColumnDesc[0].dbcid;
	FILL_PROP_SET(rgPropSet[0], 1, rgProp, DBPROPSET_INDEX)
	FILL_PROP(rgProp[0], DBPROP_INDEX_UNIQUE, VT_BOOL, V_BOOL, VARIANT_TRUE, DBPROPOPTIONS_REQUIRED)

	odtLog << "create an index with DBPROP_INDEX_UNIQUE set to VARIANT_TRUE: ";
	if (!CHECK(m_pCIIndexDefinition->CreateIndexAndCheck(&pTable->GetTableID(), NULL, 1, rgIndexColumnDesc, 1, rgPropSet, (DBID**)IndexID), S_OK))
		goto CLEANUP;

	odtLog << "OK\n";
	if (!CHECK(m_hr = pTable->Insert(2,PRIMARY,TRUE,NULL), DB_E_INTEGRITYVIOLATION))
	{
		odtLog << "ERROR: a record with a duplicated value was successfully inserted in the database!\n";
	}
	else
		odtLog << "OK: duplicated key could not be inserted\n";
	
	CHECK(m_pCIIndexDefinition->DropIndex(&pTable->GetTableID(), (DBID*)IndexID), S_OK);
	
	odtLog << "unique index was dropped\n";
	// at this point, the table should forget about column 0 being a primary key column; has it?
	if (!CHECK(m_hr = pTable->Insert(2,PRIMARY,TRUE,NULL), S_OK))
		odtLog << "a record with a duplicated value couldn't successfully be inserted in the database!\n";

	odtLog << "duplicate inserted\n";
	odtLog << "try to create a unique index on the col containing duplicated values\n";
	m_hr = m_pCIIndexDefinition->CreateCheckAndDropIndex(&pTable->GetTableID(), NULL, 1, rgIndexColumnDesc, 1, rgPropSet);
	if (!COMPARE(FAILED(m_hr), TRUE))
		odtLog << "ERROR: could create an index on a column containing duplicates!\n";
	else
		odtLog <<"OK: index could not be created\n";

CLEANUP:
	if (pTable)
	{
		pTable->DropTable();
		delete pTable;
	}
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(62)
//*-----------------------------------------------------------------------
// @mfunc Set DBPROP_INDEX_FILLFACTOR
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateIndex::Variation_62()
{ 
	BOOL		fTestResult = TEST_PASS;
	const int	n=4;
	VARIANT		values[n];
	ULONG		i;

	for (i=0; i<n; values[i++].vt = VT_I4);
	V_I4(&values[0]) = 1;
	V_I4(&values[1]) = 20;
	V_I4(&values[2]) = 60;
	V_I4(&values[3]) = 100;
	if (!CreateIndexWithOneProperty(n, values, DBPROP_INDEX_FILLFACTOR))
		fTestResult = TEST_FAIL;
	return fTestResult;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(63)
//*-----------------------------------------------------------------------
// @mfunc Set DBPROP_INDEX_INITIALSIZE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateIndex::Variation_63()
{ 
	BOOL		fTestResult = TEST_PASS;
	const int	n=4;
	VARIANT		values[n];
	ULONG		i;

	for (i=0; i<n; values[i++].vt = VT_I4);
	V_I4(&values[0]) = 0;
	V_I4(&values[1]) = 100;
	V_I4(&values[2]) = 5000;
	V_I4(&values[3]) = 10000;
	if (!CreateIndexWithOneProperty(n, values, DBPROP_INDEX_INITIALSIZE))
		fTestResult = TEST_FAIL;
	return fTestResult;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(64)
//*-----------------------------------------------------------------------
// @mfunc Set DBPROP_INDEX_NULLCOLLATION
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateIndex::Variation_64()
{ 
	BOOL		fTestResult = TEST_PASS;
	const int	n=4;
	VARIANT		values[n];
	ULONG		i;

	for (i=0; i<n; values[i++].vt = VT_I4);
	V_I4(&values[0]) = DBPROPVAL_NC_END;
	V_I4(&values[1]) = DBPROPVAL_NC_START;
	V_I4(&values[2]) = DBPROPVAL_NC_HIGH;
	V_I4(&values[3]) = DBPROPVAL_NC_LOW;
	if (!CreateIndexWithOneProperty(n, values, DBPROP_INDEX_NULLCOLLATION))
		fTestResult = TEST_FAIL;
	return fTestResult;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(65)
//*-----------------------------------------------------------------------
// @mfunc Set DBPROP_INDEX_SORTBOOKMARKS and DBPROP_INDEX_UNIQUE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateIndex::Variation_65()
{ 
	BOOL				fTestResult = TEST_PASS;
	DBPROPSET			rgPropSet[1];
	DBPROP				rgProp[2];
	HRESULT				hr;
	DBINDEX_COL_ORDER	rgOrder[]={DBINDEX_COL_ORDER_ASC};

	FILL_PROP_SET(rgPropSet[0], 2, rgProp,DBPROPSET_INDEX)
	FILL_PROP(rgProp[0], DBPROP_INDEX_SORTBOOKMARKS, VT_BOOL, V_BOOL, VARIANT_TRUE, DBPROPOPTIONS_REQUIRED)
	FILL_PROP(rgProp[1], DBPROP_INDEX_UNIQUE, VT_BOOL, V_BOOL, VARIANT_TRUE, DBPROPOPTIONS_REQUIRED)

	hr = CCNDropIndexFromOrdinals(m_pTable, NULL, m_rgColumnDesc, m_cColumnDesc, 1, &m_nIndex, rgOrder, 1, rgPropSet);
	if (S_OK != hr)
	{
		if (!CHECK(hr, DB_E_ERRORSOCCURRED))
		if ((DBPROPSTATUS_OK == rgProp[1].dwStatus) && (DBPROPSTATUS_OK == rgProp[0].dwStatus))
		{
			odtLog << "both prop status are DBPROPSTATUS_OK\n";
			fTestResult = TEST_FAIL;
		}
	}
	return fTestResult;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(66)
//*-----------------------------------------------------------------------
// @mfunc Set DBPROP_INDEX_UNIQUE and DBPROP_INDEX_PRIMARYKEY
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateIndex::Variation_66()
{ 
	BOOL					fTestResult = TEST_FAIL;
	DBPROPSET				rgPropSet[1];
	DBPROP					rgProp[2];
	HRESULT					hr;
	DBINDEX_COL_ORDER		rgOrder[]={DBINDEX_COL_ORDER_ASC};
	CTable					*pTable = NULL;
	DBCOLUMNDESC			*rgColumnDesc = NULL;
	DBORDINAL				cColumnDesc = 0;
	CList <WCHAR*, WCHAR*>	ListNativeTemp;
	CList <DBTYPE, DBTYPE>	ListDataTypes;
	ULONG					cIndex;
	CCol					col;

	pTable = new CTable(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);
	TESTC(NULL != pTable);
	pTable->CreateColInfo(ListNativeTemp, ListDataTypes, ALLTYPES);
	pTable->BuildColumnDescs(&rgColumnDesc);
	cColumnDesc = pTable->CountColumnsOnTable();
	cIndex = 0;
	pTable->SetColumnDesc(rgColumnDesc, cColumnDesc);
	pTable->SetBuildColumnDesc(FALSE);
	::SetProperty(DBPROP_COL_NULLABLE, DBPROPSET_COLUMN, 
			&rgColumnDesc[m_rgIndexCols[cIndex]-1].cPropertySets,
			&rgColumnDesc[m_rgIndexCols[cIndex]-1].rgPropertySets,
			(LPVOID)VARIANT_FALSE, VT_BOOL, DBPROPOPTIONS_REQUIRED, DB_NULLID);
	TESTC_(m_hr = pTable->CreateTable(0, 0), S_OK);

	fTestResult = TEST_PASS;
	odtLog << "\tunique, primary key index\n";
	FILL_PROP_SET(rgPropSet[0], NUMELEM(rgProp), rgProp, DBPROPSET_INDEX)
	FILL_PROP(rgProp[0], DBPROP_INDEX_UNIQUE, VT_BOOL, V_BOOL, VARIANT_TRUE, DBPROPOPTIONS_REQUIRED)
	FILL_PROP(rgProp[1], DBPROP_INDEX_PRIMARYKEY, VT_BOOL, V_BOOL, VARIANT_TRUE, DBPROPOPTIONS_REQUIRED)

	hr = CCNDropIndexFromOrdinals(pTable, NULL, rgColumnDesc, cColumnDesc, 
		1, &m_rgIndexCols[cIndex], rgOrder, 1, rgPropSet);
	if (S_OK != hr)
	{
		if (!CHECK(hr, DB_E_ERRORSOCCURRED))
			fTestResult = TEST_FAIL;
		if ((DBPROPSTATUS_OK == rgProp[1].dwStatus) && (DBPROPSTATUS_OK == rgProp[0].dwStatus))
		{
			odtLog << "both prop status are DBPROPSTATUS_OK\n";
			fTestResult = TEST_FAIL;
		}
	}
	odtLog << "\tnot unique, primary key index\n";
	FILL_PROP_SET(rgPropSet[0], NUMELEM(rgProp), rgProp, DBPROPSET_INDEX)
	FILL_PROP(rgProp[0], DBPROP_INDEX_UNIQUE, VT_BOOL, V_BOOL, VARIANT_FALSE, DBPROPOPTIONS_REQUIRED)
	FILL_PROP(rgProp[1], DBPROP_INDEX_PRIMARYKEY, VT_BOOL, V_BOOL, VARIANT_TRUE, DBPROPOPTIONS_REQUIRED)

	hr = CCNDropIndexFromOrdinals(pTable, NULL, rgColumnDesc, cColumnDesc, 
		1, &m_rgIndexCols[cIndex], rgOrder, 1, rgPropSet);
	if (S_OK != hr)
	{
		if (!CHECK(hr, DB_E_ERRORSOCCURRED))
			fTestResult = TEST_FAIL;
		if ((DBPROPSTATUS_OK == rgProp[1].dwStatus) && (DBPROPSTATUS_OK == rgProp[0].dwStatus))
		{
			odtLog << "both prop status are DBPROPSTATUS_OK\n";
			fTestResult = TEST_FAIL;
		}
	}
	odtLog << "\tunique, not primary key index\n";
	FILL_PROP_SET(rgPropSet[0], NUMELEM(rgProp), rgProp, DBPROPSET_INDEX)
	FILL_PROP(rgProp[0], DBPROP_INDEX_UNIQUE, VT_BOOL, V_BOOL, VARIANT_TRUE, DBPROPOPTIONS_REQUIRED)
	FILL_PROP(rgProp[1], DBPROP_INDEX_PRIMARYKEY, VT_BOOL, V_BOOL, VARIANT_FALSE, DBPROPOPTIONS_REQUIRED)

	hr = CCNDropIndexFromOrdinals(pTable, NULL, rgColumnDesc, cColumnDesc, 
		1, &m_rgIndexCols[cIndex], rgOrder, 1, rgPropSet);
	if (S_OK != hr)
	{
		if (!CHECK(hr, DB_E_ERRORSOCCURRED))
			fTestResult = TEST_FAIL;
		if ((DBPROPSTATUS_OK == rgProp[1].dwStatus) && (DBPROPSTATUS_OK == rgProp[0].dwStatus))
		{
			odtLog << "both prop status are DBPROPSTATUS_OK\n";
			fTestResult = TEST_FAIL;
		}
	}
	odtLog << "\tnot unique, non primary key index\n";
	FILL_PROP_SET(rgPropSet[0], NUMELEM(rgProp), rgProp, DBPROPSET_INDEX)
	FILL_PROP(rgProp[0], DBPROP_INDEX_UNIQUE, VT_BOOL, V_BOOL, VARIANT_FALSE, DBPROPOPTIONS_REQUIRED)
	FILL_PROP(rgProp[1], DBPROP_INDEX_PRIMARYKEY, VT_BOOL, V_BOOL, VARIANT_FALSE, DBPROPOPTIONS_REQUIRED)

	hr = CCNDropIndexFromOrdinals(pTable, NULL, rgColumnDesc, cColumnDesc, 
		1, &m_rgIndexCols[cIndex], rgOrder, 1, rgPropSet);
	if (S_OK != hr)
	{
		if (!CHECK(hr, DB_E_ERRORSOCCURRED))
			fTestResult = TEST_FAIL;
		if ((DBPROPSTATUS_OK == rgProp[1].dwStatus) && (DBPROPSTATUS_OK == rgProp[0].dwStatus))
		{
			odtLog << "both prop status are DBPROPSTATUS_OK\n";
			fTestResult = TEST_FAIL;
		}
	}
CLEANUP:
	pTable->DropTable();
	delete pTable;
	return fTestResult;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(67)
//*-----------------------------------------------------------------------
// @mfunc Set DBPROP_INDEX_UNIQUE and DBPROP_INDEX_NULLS
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateIndex::Variation_67()
{ 
	BOOL				fTestResult = TEST_PASS;
	DBPROPSET			rgPropSet[1];
	DBPROP				rgProp[2];
	HRESULT				hr;
	DBINDEX_COL_ORDER	rgOrder[]={DBINDEX_COL_ORDER_ASC};

	FILL_PROP_SET(rgPropSet[0], NUMELEM(rgProp), rgProp, DBPROPSET_INDEX)
	FILL_PROP(rgProp[0], DBPROP_INDEX_UNIQUE, VT_BOOL, V_BOOL, VARIANT_TRUE, DBPROPOPTIONS_REQUIRED)
	FILL_PROP(rgProp[1], DBPROP_INDEX_NULLS, VT_I4, V_I4, DBPROPVAL_IN_DISALLOWNULL, DBPROPOPTIONS_REQUIRED)

	hr = CCNDropIndexFromOrdinals(m_pTable, NULL, m_rgColumnDesc, m_cColumnDesc, 
		1, &m_nIndex, rgOrder, 1, rgPropSet);
	if (S_OK != hr)
	{
		if (!CHECK(hr, DB_E_ERRORSOCCURRED))
			return fTestResult = TEST_FAIL;
		if ((DBPROPSTATUS_OK == rgProp[1].dwStatus) && (DBPROPSTATUS_OK == rgProp[0].dwStatus))
		{
			odtLog << "both prop status are DBPROPSTATUS_OK\n";
			fTestResult = TEST_FAIL;
		}
	}
	return fTestResult;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(68)
//*-----------------------------------------------------------------------
// @mfunc CONFLICTING properties (set both DBPROP_INDEX_NULLS and DBPROP_INDEX_PRIMARYKEY)
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateIndex::Variation_68()
{ 
	BOOL				fTestResult = TEST_PASS;
	DBPROPSET			rgPropSet[1];
	DBPROP				rgProp[2];
	HRESULT				hr;
	DBINDEX_COL_ORDER	rgOrder[]={DBINDEX_COL_ORDER_ASC};

	FILL_PROP_SET(rgPropSet[0], NUMELEM(rgProp), rgProp, DBPROPSET_INDEX)
	FILL_PROP(rgProp[0], DBPROP_INDEX_NULLS, VT_I4, V_I4, DBPROPVAL_IN_DISALLOWNULL, DBPROPOPTIONS_REQUIRED)
	FILL_PROP(rgProp[1], DBPROP_INDEX_PRIMARYKEY, VT_BOOL, V_BOOL, VARIANT_TRUE, DBPROPOPTIONS_REQUIRED)

	hr = CCNDropIndexFromOrdinals(m_pTable, NULL, m_rgColumnDesc, m_cColumnDesc,
		1, &m_nIndex, rgOrder, 1, rgPropSet);
	if (S_OK != hr)
	{
		if (!CHECK(hr, DB_E_ERRORSOCCURRED))
			return fTestResult = TEST_FAIL;
		if ((DBPROPSTATUS_OK == rgProp[1].dwStatus) && (DBPROPSTATUS_OK == rgProp[0].dwStatus))
		{
			odtLog << "both prop status are DBPROPSTATUS_OK\n";
			fTestResult = TEST_FAIL;
		}
	}
	return fTestResult;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(69)
//*-----------------------------------------------------------------------
// @mfunc Non index dwPropertyID
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateIndex::Variation_69()
{ 
	BOOL				fTestResult = TEST_PASS;
	DBPROPSET			rgPropSet[1];
	DBPROP				rgProp[1];
	HRESULT				hr;
	DBINDEX_COL_ORDER	rgOrder[]={DBINDEX_COL_ORDER_ASC};

	FILL_PROP_SET(rgPropSet[0], NUMELEM(rgProp), rgProp, DBPROPSET_INDEX)
	FILL_PROP(rgProp[0], 280, VT_BOOL, V_BOOL, VARIANT_TRUE, DBPROPOPTIONS_OPTIONAL)

	odtLog << "\tDBPROPOPTIONS_OPTIONAL\n";
	hr = CCNDropIndexFromOrdinals(m_pTable, NULL, m_rgColumnDesc, m_cColumnDesc, 
		1, &m_nIndex, rgOrder, 1, rgPropSet);
	// should be DB_S_ERRORSOCCURRED
	if (!CHECK(hr, DB_S_ERRORSOCCURRED))
		fTestResult = TEST_FAIL;
	// if errors occurred, the status should be DBPROPSTATUS_NOTSUPPORTED (it was OPTIONAL)
	if (((DB_S_ERRORSOCCURRED == hr) || (S_OK == hr )) && 
		(DBPROPSTATUS_NOTSUPPORTED != rgProp[0].dwStatus)
		)
	{
		odtLog << "Bad status for property " << rgProp[0].dwPropertyID << "\n";
		fTestResult = TEST_FAIL;
	}
	
	FILL_PROP(rgProp[0], 280, VT_BOOL, V_BOOL, VARIANT_TRUE, DBPROPOPTIONS_REQUIRED)
	odtLog << "\tDBPROPOPTIONS_REQUIRED\n";
	hr = CCNDropIndexFromOrdinals(m_pTable, NULL, m_rgColumnDesc, m_cColumnDesc, 
		1, &m_nIndex, rgOrder, 1, rgPropSet);
	// should be DB_E_ERRORSOCCURRED
	if (!CHECK(hr, DB_E_ERRORSOCCURRED))
		fTestResult = TEST_FAIL;
	// if errors occurred, the status should be DBPROPSTATUS_NOTSUPPORTED (it was OPTIONAL)
	if ((DB_E_ERRORSOCCURRED == hr) && 
		(DBPROPSTATUS_NOTSUPPORTED != rgProp[0].dwStatus)
		)
	{
		odtLog << "Bad status for property " << rgProp[0].dwPropertyID << "\n";
		fTestResult = TEST_FAIL;
	}
	return fTestResult;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(70)
//*-----------------------------------------------------------------------
// @mfunc Non index guidPropertySet
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateIndex::Variation_70()
{ 
	BOOL				fTestResult = TEST_PASS;
	DBPROPSET			rgPropSet[1];
	DBPROP				rgProp[1];
	HRESULT				hr;
	DBINDEX_COL_ORDER	rgOrder[]={DBINDEX_COL_ORDER_ASC};

	FILL_PROP_SET(rgPropSet[0], NUMELEM(rgProp), rgProp, DBPROPSET_ROWSET)
	FILL_PROP(rgProp[0], DBPROP_INDEX_AUTOUPDATE, VT_BOOL, V_BOOL, VARIANT_TRUE, DBPROPOPTIONS_OPTIONAL)
	
	odtLog << "\tDBPROPOPTIONS_OPTIONAL\n";
	hr = CCNDropIndexFromOrdinals(m_pTable, NULL, m_rgColumnDesc, m_cColumnDesc, 
		1, &m_nIndex, rgOrder, 1, rgPropSet);
	// should be DB_S_ERRORSOCCURRED
	if (!CHECK(hr, DB_S_ERRORSOCCURRED))
		fTestResult = TEST_FAIL;
	// if errors occurred, the status should be DBPROPSTATUS_NOTSUPPORTED
	if ((DB_S_ERRORSOCCURRED == hr) && 
		(DBPROPSTATUS_NOTSUPPORTED != rgProp[0].dwStatus)
		)
	{
		odtLog << "Bad status for property " << rgProp[0].dwPropertyID << "\n";
		fTestResult = TEST_FAIL;
	}

	rgProp[0].dwOptions	= DBPROPOPTIONS_REQUIRED;
	odtLog << "\tDBPROPOPTIONS_REQUIRED\n";
	hr = CCNDropIndexFromOrdinals(m_pTable, NULL, m_rgColumnDesc, m_cColumnDesc, 
		1, &m_nIndex, rgOrder, 1, rgPropSet);
	// should be either DB_E_ERRORSOCCURRED
	if (!CHECK(hr, DB_E_ERRORSOCCURRED))
		fTestResult = TEST_FAIL;
	// if errors occurred, the status should be DBPROPSTATUS_NOTSUPPORTED
	if ((DB_E_ERRORSOCCURRED == hr) && 
		(DBPROPSTATUS_NOTSUPPORTED != rgProp[0].dwStatus)
		)
	{
		odtLog << "Bad status for property " << rgProp[0].dwPropertyID << "\n";
		fTestResult = TEST_FAIL;
	}
	return fTestResult;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(71)
//*-----------------------------------------------------------------------
// @mfunc invalid value type for property
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateIndex::Variation_71()
{ 
	BOOL				fTestResult = TEST_PASS;
	DBPROPSET			rgPropSet[1];
	DBPROP				rgProp[1];
	HRESULT				hr;
	DBINDEX_COL_ORDER	rgOrder[]={DBINDEX_COL_ORDER_ASC};

	FILL_PROP_SET(rgPropSet[0], NUMELEM(rgProp), rgProp, DBPROPSET_INDEX)
	FILL_PROP(rgProp[0], DBPROP_INDEX_NULLS, VT_BOOL, V_BOOL, VARIANT_TRUE, DBPROPOPTIONS_OPTIONAL)

	odtLog << "\tDBPROPOPTIONS_OPTIONAL\n";
	hr = CCNDropIndexFromOrdinals(m_pTable, NULL, m_rgColumnDesc, m_cColumnDesc, 
		1, &m_nIndex, rgOrder, 1, rgPropSet);
	// should be DB_S_ERRORSOCCURRED
	if (!CHECK(hr, DB_S_ERRORSOCCURRED))
		fTestResult = TEST_FAIL;

	// if errors occurred, the status should be DBPROPSTATUS_BADVALUE or DBPROPSTATUS_NOTSUPPORTED
	if (	(DB_S_ERRORSOCCURRED == hr) 
		&&	(DBPROPSTATUS_BADVALUE != rgProp[0].dwStatus) 
		&& (DBPROPSTATUS_NOTSUPPORTED != rgProp[0].dwStatus)
		)
	{
		odtLog << "Bad status for property " << rgProp[0].dwPropertyID << "\n";
		fTestResult = TEST_FAIL;
	}

	rgProp[0].dwOptions	= DBPROPOPTIONS_REQUIRED;
	odtLog << "\tDBPROPOPTIONS_REQUIRED\n";
	hr = CCNDropIndexFromOrdinals(m_pTable, NULL, m_rgColumnDesc, m_cColumnDesc, 
		1, &m_nIndex, rgOrder, 1, rgPropSet);
	// should be DB_E_ERRORSOCCURRED
	if (!CHECK(hr, DB_E_ERRORSOCCURRED))
		fTestResult = TEST_FAIL;
	// if errors occurred, the status should be DBPROPSTATUS_BADVALUE or DBPROPSTATUS_NOTSUPPORTED
	if (	(DB_E_ERRORSOCCURRED == hr) 
		&&	(DBPROPSTATUS_BADVALUE != rgProp[0].dwStatus) 
		&&	(DBPROPSTATUS_NOTSUPPORTED != rgProp[0].dwStatus)
		)
	{
		odtLog << "Bad status for property " << rgProp[0].dwPropertyID << "\n";
		fTestResult = TEST_FAIL;
	}
	return fTestResult;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(72)
//*-----------------------------------------------------------------------
// @mfunc invalid value for a property
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateIndex::Variation_72()
{ 
	BOOL				fTestResult = TEST_PASS;
	DBPROPSET			rgPropSet[1];
	DBPROP				rgProp[1];
	HRESULT				hr;
	DBINDEX_COL_ORDER	rgOrder[]={DBINDEX_COL_ORDER_ASC};
	BOOL				fSupported;
	DBPROPSTATUS		dwStatus;

	fSupported	= SupportedProperty(DBPROP_INDEX_NULLS, DBPROPSET_INDEX);
	FILL_PROP_SET(rgPropSet[0], NUMELEM(rgProp), rgProp, DBPROPSET_INDEX)
	FILL_PROP(rgProp[0], DBPROP_INDEX_NULLS, VT_BOOL, V_BOOL, VARIANT_TRUE, DBPROPOPTIONS_OPTIONAL)

	odtLog << "\tDBPROPOPTIONS_OPTIONAL\n";
	hr = CCNDropIndexFromOrdinals(m_pTable, NULL, m_rgColumnDesc, m_cColumnDesc, 
		1, &m_nIndex, rgOrder, 1, rgPropSet);

	// should be DB_S_ERRORSOCCURRED
	if (!CHECK(hr, DB_S_ERRORSOCCURRED))
		fTestResult = TEST_FAIL;
	dwStatus = rgProp[0].dwStatus;
	// if errors occurred, the status should be DBPROPSTATUS_NOTSET (it was OPTIONAL) - IF THE PROP IS SUPPORTED!
	if (	(DB_S_ERRORSOCCURRED == hr) 
		&&	(	(fSupported && !COMPARE(dwStatus, DBPROPSTATUS_BADVALUE))
			||	(!fSupported && !COMPARE(dwStatus, DBPROPSTATUS_NOTSUPPORTED))))
	{
		odtLog << "Bad status for property " << rgProp[0].dwPropertyID << "\n";
		fTestResult = TEST_FAIL;
	}

	rgProp[0].dwOptions	= DBPROPOPTIONS_REQUIRED;
	odtLog << "\tDBPROPOPTIONS_REQUIRED\n";
	hr = CCNDropIndexFromOrdinals(m_pTable, NULL, m_rgColumnDesc, m_cColumnDesc, 
		1, &m_nIndex, rgOrder, 1, rgPropSet);

	// should be DB_E_ERRORSOCCURRED
	if (!CHECK(hr, DB_E_ERRORSOCCURRED))
		fTestResult = TEST_FAIL;
	
	// if errors occurred, the status should be DBPROPSTATUS_BADVALUE or DBPROPSTATUS_NOTSUPPORTED
	if (	(DB_E_ERRORSOCCURRED == hr) 
		&&	(	(fSupported && !COMPARE(dwStatus, DBPROPSTATUS_BADVALUE))
			||	(!fSupported && !COMPARE(dwStatus, DBPROPSTATUS_NOTSUPPORTED))))
	{
		odtLog << "Bad status for property " << rgProp[0].dwPropertyID << "\n";
		fTestResult = TEST_FAIL;
	}
	return fTestResult;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(73)
//*-----------------------------------------------------------------------
// @mfunc specifying a property twice
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateIndex::Variation_73()
{ 
	BOOL				fTestResult = TEST_PASS;
	DBPROPSET			rgPropSet[1];
	DBPROP				rgProp[2];
	HRESULT				hr;
	DBINDEX_COL_ORDER	rgOrder[]={DBINDEX_COL_ORDER_ASC};

	FILL_PROP_SET(rgPropSet[0], NUMELEM(rgProp), rgProp, DBPROPSET_INDEX)
	FILL_PROP(rgProp[0], DBPROP_INDEX_NULLS, VT_I4, V_I4, DBPROPVAL_IN_DISALLOWNULL, DBPROPOPTIONS_REQUIRED)
	FILL_PROP(rgProp[1], DBPROP_INDEX_NULLS, VT_I4, V_I4, DBPROPVAL_IN_IGNORENULL, DBPROPOPTIONS_REQUIRED)
	
	hr = CCNDropIndexFromOrdinals(m_pTable, NULL, m_rgColumnDesc, m_cColumnDesc, 
		1, &m_nIndex, rgOrder, 1, rgPropSet);

	// should be either S_OK or DB_E_ERRORSOCCURRED
	if (S_OK != hr && !CHECK(hr, DB_E_ERRORSOCCURRED))
		fTestResult = TEST_FAIL;
	else
	{
		if (	(	S_OK == hr 
				&&	(DBPROPSTATUS_OK != rgProp[0].dwStatus) 
				&&	(DBPROPSTATUS_OK != rgProp[0].dwStatus))
			||	(	S_OK != hr 
				&&	(DBPROPSTATUS_OK == rgProp[0].dwStatus) 
				&&	(DBPROPSTATUS_OK == rgProp[0].dwStatus)))
		{
			odtLog << "Property " << rgProp[0].dwPropertyID << " set incorrectly\n";
			fTestResult = TEST_FAIL;
		}
	}

	return fTestResult;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(74)
//*-----------------------------------------------------------------------
// @mfunc all the properties
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateIndex::Variation_74()
{ 
	const int			nProp			= 11;
	BOOL				fTestResult		= TEST_PASS;
	DBPROPSET			rgPropSet[1];
	DBPROP				rgProp[nProp];
	HRESULT				hr;
	ULONG				iProp, k;
	DBINDEX_COL_ORDER	rgOrder[]		= {DBINDEX_COL_ORDER_ASC};

	FILL_PROP_SET(rgPropSet[0], NUMELEM(rgProp), rgProp, DBPROPSET_INDEX)
	FILL_PROP(rgProp[0], DBPROP_INDEX_AUTOUPDATE, VT_BOOL, V_BOOL, VARIANT_TRUE, DBPROPOPTIONS_OPTIONAL)
	FILL_PROP(rgProp[1], DBPROP_INDEX_CLUSTERED, VT_BOOL, V_BOOL, VARIANT_TRUE, DBPROPOPTIONS_OPTIONAL)
	FILL_PROP(rgProp[2], DBPROP_INDEX_FILLFACTOR, VT_I4, V_I4, 10, DBPROPOPTIONS_OPTIONAL)
	FILL_PROP(rgProp[3], DBPROP_INDEX_INITIALSIZE, VT_I4, V_I4, 2000, DBPROPOPTIONS_OPTIONAL)
	FILL_PROP(rgProp[4], DBPROP_INDEX_NULLCOLLATION, VT_I4, V_I4, DBPROPVAL_NC_END, DBPROPOPTIONS_OPTIONAL)
	FILL_PROP(rgProp[5], DBPROP_INDEX_NULLS, VT_I4, V_I4, DBPROPVAL_IN_DISALLOWNULL, DBPROPOPTIONS_OPTIONAL)
	FILL_PROP(rgProp[6], DBPROP_INDEX_PRIMARYKEY, VT_BOOL, V_BOOL, VARIANT_FALSE, DBPROPOPTIONS_OPTIONAL)
	FILL_PROP(rgProp[7], DBPROP_INDEX_SORTBOOKMARKS, VT_BOOL, V_BOOL, VARIANT_TRUE, DBPROPOPTIONS_OPTIONAL)
	FILL_PROP(rgProp[8], DBPROP_INDEX_TEMPINDEX, VT_BOOL, V_BOOL, VARIANT_TRUE, DBPROPOPTIONS_OPTIONAL)
	FILL_PROP(rgProp[9], DBPROP_INDEX_UNIQUE, VT_BOOL, V_BOOL, VARIANT_TRUE, DBPROPOPTIONS_OPTIONAL)
	FILL_PROP(rgProp[10], DBPROP_INDEX_TYPE, VT_I4, V_I4, DBPROPVAL_IT_BTREE, DBPROPOPTIONS_OPTIONAL)
	
	hr = CCNDropIndexFromOrdinals(m_pTable, NULL, m_rgColumnDesc, m_cColumnDesc, 
		1, &m_nIndex, rgOrder, 1, rgPropSet);

	// should be either S_OK or DB_S_ERRORSOCCURRED
	if (DB_S_ERRORSOCCURRED != hr)
	{
		if (!CHECK(hr, S_OK))
			fTestResult = TEST_FAIL;
	}
	else
	{
		// if errors occurred, the status should be different from DBPROPSTATUS_OK
		for (iProp=0, k=0; iProp<nProp; iProp++)
		{
			if (DBPROPSTATUS_OK == rgProp[iProp].dwStatus )
				k++;
		}
		if (nProp==k || 0 == k)
		{
			odtLog << "Property " << rgProp[0].dwPropertyID << " set incorrectly\n";
			fTestResult = TEST_FAIL;
		}
	}

	return fTestResult;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(75)
//*-----------------------------------------------------------------------
// @mfunc column name is invalid => DB_E_NOCOLUMN
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateIndex::Variation_75()
{ 
	BOOL					fTestResult = TEST_FAIL;
	DBINDEXCOLUMNDESC		rgIndexColumnDesc[1];
	size_t					m, n;
	DBID					ColumnID;

	rgIndexColumnDesc[0].pColumnID		= &ColumnID;
	rgIndexColumnDesc[0].eIndexColOrder	= DBINDEX_COL_ORDER_ASC;

	// get an invalid column name
	ColumnID.eKind			= DBKIND_NAME;
	m = min(m_cMaxColumnName, n=5+wcslen(m_pTable->GetTableName()));
	ColumnID.uName.pwszName	= BuildInvalidName(m, m_pTable->GetTableName(), m_pwszInvalidColumnChars);
	
	TESTC_(m_hr = m_pCIIndexDefinition->CreateCheckAndDropIndex(&m_pTable->GetTableID(), NULL, NUMELEM(rgIndexColumnDesc),
		rgIndexColumnDesc), DB_E_NOCOLUMN);

	fTestResult = TEST_PASS;

CLEANUP:
	ReleaseDBID(&ColumnID, FALSE);
	return fTestResult;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(76)
//*-----------------------------------------------------------------------
// @mfunc maximum length column name => S_OK
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateIndex::Variation_76()
{ 
	CTable				*pTable = NULL;
	BOOL				fTestResult = TEST_FAIL;
	DBORDINAL			rgIndexCol[1] = {m_nIndex};
	DBINDEX_COL_ORDER	rgOrder[]={DBINDEX_COL_ORDER_ASC};
	DBKIND				eKind;
	LPWSTR				pwszColumnName;

	pTable = new CTable(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);
	eKind = m_rgColumnDesc[m_nIndex-1].dbcid.eKind;
	pwszColumnName = m_rgColumnDesc[m_nIndex-1].dbcid.uName.pwszName;
	TESTC(NULL != pTable);
	pTable->SetBuildColumnDesc(FALSE);
	pTable->SetColumnDesc(m_rgColumnDesc, m_cColumnDesc);
	m_rgColumnDesc[m_nIndex-1].dbcid.eKind			= DBKIND_NAME;
	m_rgColumnDesc[m_nIndex-1].dbcid.uName.pwszName	= BuildValidName(m_cMaxColumnName, m_pTable->GetTableName());
	if (!CHECK(m_hr = pTable->CreateTable(0, 0), S_OK))
	{
		odtLog << "\tcould not create a table\n";
		goto CLEANUP;
	}

	TESTC_(m_hr = CCNDropIndexFromOrdinals(pTable, NULL, m_rgColumnDesc, m_cColumnDesc, 1, rgIndexCol, rgOrder), S_OK);

	fTestResult = TEST_PASS;

CLEANUP:
	if (pTable)
	{
		pTable->DropTable();
		pTable->SetColumnDesc(NULL, 0);
		delete pTable;
	}
	m_rgColumnDesc[m_nIndex-1].dbcid.eKind = eKind;
	SAFE_FREE(m_rgColumnDesc[m_nIndex-1].dbcid.uName.pwszName);
	m_rgColumnDesc[m_nIndex-1].dbcid.uName.pwszName = pwszColumnName;
	return fTestResult;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(77)
//*-----------------------------------------------------------------------
// @mfunc too long column name => DB_E_NOCOLUMN
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateIndex::Variation_77()
{ 
	BOOL				fTestResult = TEST_FAIL;
	DBINDEXCOLUMNDESC	rgIndexColumnDesc[1];
	DBID				ColumnID;

	rgIndexColumnDesc[0].pColumnID		= &ColumnID;
	rgIndexColumnDesc[0].eIndexColOrder	= DBINDEX_COL_ORDER_ASC;

	// get a too long column name
	ColumnID.eKind			= DBKIND_NAME;
	ColumnID.uName.pwszName	= BuildValidName(m_cMaxColumnName+1, m_pTable->GetTableName());
	
	TESTC_(m_hr = m_pCIIndexDefinition->CreateCheckAndDropIndex(&m_pTable->GetTableID(), NULL, 1, rgIndexColumnDesc), DB_E_NOCOLUMN);
	
	fTestResult = TEST_PASS;

CLEANUP:
	ReleaseDBID(&ColumnID, FALSE);
	return fTestResult;
} 
// }} TCW_VAR_PROTOTYPE_END




//------------------------------------------------------------------
//
//	thread function
//------------------------------------------------------------------
unsigned TCCreateIndex::MyThreadProc(ULONG i)
{
	DBID				IndexID;
	DBINDEXCOLUMNDESC	rgIndexColumnDesc[1];
	HRESULT				hr;

	if (i>=nThreads)
		return 0;

	IndexID.eKind = DBKIND_NAME;
	IndexID.uName.pwszName = m_rgIndexName[i];
	rgIndexColumnDesc[0].pColumnID		= &m_rgColumnDesc[m_nIndex-1].dbcid;
	rgIndexColumnDesc[0].eIndexColOrder = DBINDEX_COL_ORDER_ASC;
	Sleep(0);
	hr = m_pCIIndexDefinition->CreateIndex(&m_pTable->GetTableID(), &IndexID, 
							1, rgIndexColumnDesc, 0, NULL, NULL);
	Sleep(0);
	m_rgResult[i] = hr;
	return 1;
} //TCCreateIndex::MyThreadProc




// {{ TCW_VAR_PROTOTYPE(78)
//*-----------------------------------------------------------------------
// @mfunc multithreading
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateIndex::Variation_78()
{ 
	unsigned		IDThread[nThreads];
	HANDLE			hThread[nThreads];
	CInParam		ThreadParam[nThreads];
	DBID			IndexID;
	BOOL			fTestResult = TEST_PASS, fExists;
	ULONG			nIndex;
	WCHAR			pwszNameSeed[]=L"Index";

	for (nIndex = 0; nIndex < nThreads; nIndex++)
	{
		m_rgIndexName[nIndex]	= NULL;
		m_rgResult[nIndex]		= E_FAIL;
	}
	for (nIndex = 0; nIndex < nThreads; nIndex++)
	{
		ThreadParam[nIndex].i		= nIndex;
		ThreadParam[nIndex].pObject = this;
		SAFE_ALLOC(m_rgIndexName[nIndex], WCHAR, wcslen(pwszNameSeed)+3);
		swprintf(m_rgIndexName[nIndex], L"%s%02d", pwszNameSeed, nIndex);
		hThread[nIndex] = (HANDLE)_beginthreadex(NULL, 0, ThreadProc,
							(void*)&ThreadParam[nIndex],
							0, 
							&IDThread[nIndex]);		
		if (hThread[nIndex] == 0)
		{	
			fTestResult = TEST_FAIL;
			goto CLEANUP;
		}
	}
	
	WaitForMultipleObjects(nThreads, hThread, TRUE, INFINITE);
	for (nIndex=0; nIndex<nThreads; nIndex++)
		CloseHandle(hThread[nIndex]);

CLEANUP:
	for (nIndex=0; nIndex<nThreads; nIndex++)
	{
		IndexID.eKind			= DBKIND_NAME;
		IndexID.uName.pwszName	= m_rgIndexName[nIndex];
		
		if (	S_OK == m_rgResult[nIndex] 
			&&	CHECK(m_hr = m_pCIIndexDefinition->DoesIndexExist(	&m_pTable->GetTableID(), &IndexID, &fExists), S_OK) 
			&&	fExists
			)
			CHECK(m_hr = m_pCIIndexDefinition->DropIndex(&m_pTable->GetTableID(), &IndexID), S_OK);
		else
			fTestResult = TEST_FAIL;

		if (m_rgIndexName[nIndex])
			SAFE_FREE(m_rgIndexName[nIndex]);
	}

	return fTestResult;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(79)
//*-----------------------------------------------------------------------
// @mfunc 2 threads, the same index
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateIndex::Variation_79()
{ 
	unsigned		IDThread[nThreads];
	HANDLE			hThread[nThreads];
	CInParam		ThreadParam[nThreads];
	DBID			IndexID;
	BOOL			fTestResult = TEST_FAIL; 
	ULONG			nSuccess = 0;
	ULONG			nIndex;
	WCHAR			pwszIndexName[] = L"theSameIndex";

	for (nIndex = 0; nIndex < nThreads; nIndex++)
	{
		m_rgResult[nIndex]			= E_FAIL;
		ThreadParam[nIndex].i		= nIndex;
		ThreadParam[nIndex].pObject = this;
		m_rgIndexName[nIndex] = pwszIndexName;
		hThread[nIndex] = (HANDLE)_beginthreadex(NULL, 0, ThreadProc,
							(void*)&ThreadParam[nIndex],
							0, 
							&IDThread[nIndex]);		
		if (hThread[nIndex] == 0)
		{	
			fTestResult = TEST_FAIL;
			goto CLEANUP;
		}
	}

	WaitForMultipleObjects(nThreads, hThread, TRUE, INFINITE);

	for (nIndex=0; nIndex<nThreads; nIndex++)
	{
		CloseHandle(hThread[nIndex]);
		if (S_OK == m_rgResult[nIndex])
			nSuccess++;
	}

	IndexID.eKind			= DBKIND_NAME;
	IndexID.uName.pwszName	= pwszIndexName;
	m_hr = m_pCIIndexDefinition->DropIndex(&m_pTable->GetTableID(), &IndexID);

	if (1 == nSuccess)
		fTestResult = TEST_PASS;

CLEANUP:
	return fTestResult;
} 
// }} TCW_VAR_PROTOTYPE_END






// {{ TCW_VAR_PROTOTYPE(80)
//*-----------------------------------------------------------------------
// @mfunc Primary and clustered index properties
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateIndex::Variation_80()
{ 
	BOOL				fTestResult = TEST_PASS;
	DBPROPSET			rgPropSet[1];
	DBPROP				rgProp[2];
	HRESULT				hr;
	DBINDEX_COL_ORDER	rgOrder[]={DBINDEX_COL_ORDER_ASC};
	CTable				*pTable = NULL;
	DBCOLUMNDESC		*rgColumnDesc = NULL;
	DBORDINAL			cColumnDesc = 0;
	CList <WCHAR*, WCHAR*>	ListNativeTemp;
	CList <DBTYPE, DBTYPE>	ListDataTypes;
	ULONG				cColIndex;
	ULONG				cIndex;
	BOOL				fFound;
	CCol				col;

	pTable = new CTable(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);
	TESTC(NULL != pTable);
	pTable->CreateColInfo(ListNativeTemp, ListDataTypes, ALLTYPES);
	pTable->BuildColumnDescs(&rgColumnDesc);
	cColumnDesc = pTable->CountColumnsOnTable();
	// check for an indexable and nullable type
	cIndex = 0;
	for (cColIndex = 0, fFound = FALSE; cColIndex < m_cIndexCols && !fFound; cColIndex++)
	{
		col = pTable->GetColInfoForUpdate(m_rgIndexCols[cColIndex]);
		if (1 == col.GetNullable())
		{
			fFound = TRUE;
			cIndex = cColIndex;
		}
	}

	pTable->SetColumnDesc(rgColumnDesc, cColumnDesc);
	pTable->SetBuildColumnDesc(FALSE);
	::SetProperty(DBPROP_COL_NULLABLE, DBPROPSET_COLUMN, 
			&rgColumnDesc[m_rgIndexCols[cIndex]-1].cPropertySets,
			&rgColumnDesc[m_rgIndexCols[cIndex]-1].rgPropertySets,
			(LPVOID)VARIANT_FALSE, VT_BOOL, DBPROPOPTIONS_REQUIRED, DB_NULLID);
	TESTC_(m_hr = pTable->CreateTable(0, 0), S_OK);
	odtLog << "\tclustered, primary key index\n";
	FILL_PROP_SET(rgPropSet[0], NUMELEM(rgProp), rgProp, DBPROPSET_INDEX)
	FILL_PROP(rgProp[0], DBPROP_INDEX_CLUSTERED, VT_BOOL, V_BOOL, VARIANT_TRUE, DBPROPOPTIONS_REQUIRED)
	FILL_PROP(rgProp[1], DBPROP_INDEX_PRIMARYKEY, VT_BOOL, V_BOOL, VARIANT_TRUE, DBPROPOPTIONS_REQUIRED)

	hr = CCNDropIndexFromOrdinals(pTable, NULL, rgColumnDesc, cColumnDesc, 
		1, &m_rgIndexCols[cIndex], rgOrder, 1, rgPropSet);
	if (S_OK != hr)
	{
		if (!CHECK(hr, DB_E_ERRORSOCCURRED))
			fTestResult = TEST_FAIL;
		if ((DBPROPSTATUS_OK == rgProp[1].dwStatus) && (DBPROPSTATUS_OK == rgProp[0].dwStatus))
		{
			odtLog << "both prop status are DBPROPSTATUS_OK\n";
			fTestResult = TEST_FAIL;
		}
	}
	odtLog << "\tnot clustered, primary key index\n";
	FILL_PROP_SET(rgPropSet[0], NUMELEM(rgProp), rgProp, DBPROPSET_INDEX)
	FILL_PROP(rgProp[0], DBPROP_INDEX_CLUSTERED, VT_BOOL, V_BOOL, VARIANT_FALSE, DBPROPOPTIONS_REQUIRED)
	FILL_PROP(rgProp[1], DBPROP_INDEX_PRIMARYKEY, VT_BOOL, V_BOOL, VARIANT_TRUE, DBPROPOPTIONS_REQUIRED)

	hr = CCNDropIndexFromOrdinals(pTable, NULL, rgColumnDesc, cColumnDesc, 
		1, &m_rgIndexCols[cIndex], rgOrder, 1, rgPropSet);
	if (S_OK != hr)
	{
		if (!CHECK(hr, DB_E_ERRORSOCCURRED))
			fTestResult = TEST_FAIL;
		if ((DBPROPSTATUS_OK == rgProp[1].dwStatus) && (DBPROPSTATUS_OK == rgProp[0].dwStatus))
		{
			odtLog << "both prop status are DBPROPSTATUS_OK\n";
			fTestResult = TEST_FAIL;
		}
	}
	odtLog << "\tclustered, not primary key index\n";
	FILL_PROP_SET(rgPropSet[0], NUMELEM(rgProp), rgProp, DBPROPSET_INDEX)
	FILL_PROP(rgProp[0], DBPROP_INDEX_CLUSTERED, VT_BOOL, V_BOOL, VARIANT_TRUE, DBPROPOPTIONS_REQUIRED)
	FILL_PROP(rgProp[1], DBPROP_INDEX_PRIMARYKEY, VT_BOOL, V_BOOL, VARIANT_FALSE, DBPROPOPTIONS_REQUIRED)

	hr = CCNDropIndexFromOrdinals(pTable, NULL, rgColumnDesc, cColumnDesc, 
		1, &m_rgIndexCols[cIndex], rgOrder, 1, rgPropSet);
	if (S_OK != hr)
	{
		if (!CHECK(hr, DB_E_ERRORSOCCURRED))
			fTestResult = TEST_FAIL;
		if ((DBPROPSTATUS_OK == rgProp[1].dwStatus) && (DBPROPSTATUS_OK == rgProp[0].dwStatus))
		{
			odtLog << "both prop status are DBPROPSTATUS_OK\n";
			fTestResult = TEST_FAIL;
		}
	}
	odtLog << "\tnot clustered, non primary key index\n";
	FILL_PROP_SET(rgPropSet[0], NUMELEM(rgProp), rgProp, DBPROPSET_INDEX)
	FILL_PROP(rgProp[0], DBPROP_INDEX_CLUSTERED, VT_BOOL, V_BOOL, VARIANT_FALSE, DBPROPOPTIONS_REQUIRED)
	FILL_PROP(rgProp[1], DBPROP_INDEX_PRIMARYKEY, VT_BOOL, V_BOOL, VARIANT_FALSE, DBPROPOPTIONS_REQUIRED)

	hr = CCNDropIndexFromOrdinals(pTable, NULL, rgColumnDesc, cColumnDesc, 
		1, &m_rgIndexCols[cIndex], rgOrder, 1, rgPropSet);
	if (S_OK != hr)
	{
		if (!CHECK(hr, DB_E_ERRORSOCCURRED))
			fTestResult = TEST_FAIL;
		if ((DBPROPSTATUS_OK == rgProp[1].dwStatus) && (DBPROPSTATUS_OK == rgProp[0].dwStatus))
		{
			odtLog << "both prop status are DBPROPSTATUS_OK\n";
			fTestResult = TEST_FAIL;
		}
	}
CLEANUP:
	pTable->DropTable();
	delete pTable;
	return fTestResult;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(81)
//*-----------------------------------------------------------------------
// @mfunc bogus colid in a non column specific index property
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateIndex::Variation_81()
{ 
	DBINDEX_COL_ORDER	rgOrder[]		= {DBINDEX_COL_ORDER_ASC};
	BOOL				fTestRes		= TEST_SKIPPED;
	DBPROPSET			rgPropSet[1];
	DBPROP				rgProp[1];
	DBPROPID			dwPropID;
	DBCOLUMNDESC		*rgColumnDesc	= NULL;
	ULONG				cColumnDesc		= 0;
	HRESULT				hr;

	// make sure there is a non column specific index property that is settable
	TESTC_PROVIDER(GetNonColSpecProp(DBPROPSET_INDEX, &dwPropID));
	fTestRes = TEST_FAIL;
	
	FILL_PROP_SET(rgPropSet[0], 1, rgProp, DBPROPSET_INDEX)
	FILL_PROP(rgProp[0], dwPropID, DBTYPE_BOOL, V_BOOL, 
		VARIANT_TRUE, DBPROPOPTIONS_OPTIONAL)

	// set colID to a value different from DB_NULLID	
	rgProp[0].colid.eKind			= DBKIND_NAME;
	rgProp[0].colid.uName.pwszName	= L"someColumn";

	// check colid is ignored for the rowset property
	TESTC_(hr = CCNDropIndexFromOrdinals(m_pTable, NULL, m_rgColumnDesc, m_cColumnDesc, 
			1, &m_nIndex, rgOrder, 1, rgPropSet), DB_S_ERRORSOCCURRED);
	TESTC(DBPROPSTATUS_BADCOLUMN == rgProp[0].dwStatus);

	// check colid is ignored for the column property
	rgProp[0].dwOptions = DBPROPOPTIONS_REQUIRED;
	TESTC_(hr = CCNDropIndexFromOrdinals(m_pTable, NULL, m_rgColumnDesc, m_cColumnDesc, 
			1, &m_nIndex, rgOrder, 1, rgPropSet), DB_E_ERRORSOCCURRED);
	TESTC(DBPROPSTATUS_BADCOLUMN == rgProp[0].dwStatus);

	fTestRes = TEST_PASS;

CLEANUP:
	ReleaseColumnDesc(rgColumnDesc, cColumnDesc);
	return fTestRes;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(82)
//*-----------------------------------------------------------------------
// @mfunc DB_NULLID passed as colid in a column specific index property
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateIndex::Variation_82()
{ 
	DBINDEX_COL_ORDER	rgOrder[]		= {DBINDEX_COL_ORDER_ASC};
	BOOL				fTestRes		= TEST_SKIPPED;
	DBPROPSET			rgPropSet[1];
	DBPROP				rgProp[1];
	DBPROPID			dwPropID;
	DBCOLUMNDESC		*rgColumnDesc	= NULL;
	ULONG				cColumnDesc		= 0;
	HRESULT				hr;

	// make sure there is a column specific index property that is settable
	TESTC_PROVIDER(GetColSpecProp(DBPROPSET_INDEX, &dwPropID));
	fTestRes = TEST_FAIL;
	
	FILL_PROP_SET(rgPropSet[0], 1, rgProp, DBPROPSET_INDEX)
	FILL_PROP(rgProp[0], dwPropID, DBTYPE_BOOL, V_BOOL, 
		VARIANT_TRUE, DBPROPOPTIONS_OPTIONAL)

	// set colID to DB_NULLID	
	rgProp[0].colid = DB_NULLID;

	// check colid is ignored for the rowset property
	hr = CCNDropIndexFromOrdinals(m_pTable, NULL, m_rgColumnDesc, m_cColumnDesc, 
			1, &m_nIndex, rgOrder, 1, rgPropSet);
	if (DB_S_ERRORSOCCURRED == hr)
	{
		TESTC(DBPROPSTATUS_NOTALLSETTABLE == rgProp[0].dwStatus);
	}
	else
	{
		TESTC_(hr, S_OK);
		TESTC(DBPROPSTATUS_OK == rgProp[0].dwStatus);
	}

	// check colid is ignored for the column property
	rgProp[0].dwOptions = DBPROPOPTIONS_REQUIRED;
	hr = CCNDropIndexFromOrdinals(m_pTable, NULL, m_rgColumnDesc, m_cColumnDesc, 
			1, &m_nIndex, rgOrder, 1, rgPropSet);
	if (DB_E_ERRORSOCCURRED == hr)
	{
		TESTC(DBPROPSTATUS_NOTALLSETTABLE == rgProp[0].dwStatus);
	}
	else
	{
		TESTC_(hr, S_OK);
		TESTC(DBPROPSTATUS_OK == rgProp[0].dwStatus);
	}

	fTestRes = TEST_PASS;

CLEANUP:
	ReleaseColumnDesc(rgColumnDesc, cColumnDesc);
	return fTestRes;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(83)
//*-----------------------------------------------------------------------
// @mfunc valid, non DB_NULLID  colid in a column specific index property
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateIndex::Variation_83()
{ 
	DBINDEX_COL_ORDER	rgOrder[]		= {DBINDEX_COL_ORDER_ASC};
	BOOL				fTestRes		= TEST_SKIPPED;
	DBPROPSET			rgPropSet[1];
	DBPROP				rgProp[1];
	DBPROPID			dwPropID;
	DBCOLUMNDESC		*rgColumnDesc	= NULL;
	ULONG				cColumnDesc		= 0;
	HRESULT				hr;

	memset(&rgProp[0].colid, 0, sizeof(DBID));

	// make sure there is a column specific index property that is settable
	TESTC_PROVIDER(GetColSpecProp(DBPROPSET_INDEX, &dwPropID));
	fTestRes = TEST_FAIL;
	
	FILL_PROP_SET(rgPropSet[0], 1, rgProp, DBPROPSET_INDEX)
	FILL_PROP(rgProp[0], dwPropID, DBTYPE_BOOL, V_BOOL, 
		VARIANT_TRUE, DBPROPOPTIONS_OPTIONAL)

	// set colID to a value different from DB_NULLID	
	DuplicateDBID(m_rgColumnDesc[m_nIndex-1].dbcid, &rgProp[0].colid);
	
	// check colid is ignored for the rowset property
	TESTC_(hr = CCNDropIndexFromOrdinals(m_pTable, NULL, m_rgColumnDesc, m_cColumnDesc, 
			1, &m_nIndex, rgOrder, 1, rgPropSet), DB_S_ERRORSOCCURRED);
	TESTC(DBPROPSTATUS_BADCOLUMN == rgProp[0].dwStatus);

	// check colid is ignored for the column property
	rgProp[0].dwOptions = DBPROPOPTIONS_REQUIRED;
	TESTC_(hr = CCNDropIndexFromOrdinals(m_pTable, NULL, m_rgColumnDesc, m_cColumnDesc, 
			1, &m_nIndex, rgOrder, 1, rgPropSet), DB_S_ERRORSOCCURRED);
	TESTC(DBPROPSTATUS_BADCOLUMN == rgProp[0].dwStatus);

	fTestRes = TEST_PASS;

CLEANUP:
	ReleaseColumnDesc(rgColumnDesc, cColumnDesc);
	ReleaseDBID(&rgProp[0].colid, FALSE);
	return fTestRes;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(84)
//*-----------------------------------------------------------------------
// @mfunc bogus colid in a column specific index property => error
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateIndex::Variation_84()
{ 
	DBINDEX_COL_ORDER	rgOrder[]		= {DBINDEX_COL_ORDER_ASC};
	BOOL				fTestRes		= TEST_SKIPPED;
	DBPROPSET			rgPropSet[1];
	DBPROP				rgProp[1];
	DBPROPID			dwPropID;
	DBCOLUMNDESC		*rgColumnDesc	= NULL;
	ULONG				cColumnDesc		= 0;
	HRESULT				hr;

	// make sure there is a non column specific rowset property that is settable
	TESTC_PROVIDER(GetColSpecProp(DBPROPSET_INDEX, &dwPropID));
	fTestRes = TEST_FAIL;
	
	FILL_PROP_SET(rgPropSet[0], 1, rgProp, DBPROPSET_INDEX)
	FILL_PROP(rgProp[0], dwPropID, DBTYPE_BOOL, V_BOOL, 
		VARIANT_TRUE, DBPROPOPTIONS_OPTIONAL)

	// set colID to a value different from DB_NULLID	
	rgProp[0].colid.eKind			= DBKIND_NAME;
	rgProp[0].colid.uName.pwszName	= L"someColumn";

	// check colid is ignored for the rowset property
	TESTC_(hr = CCNDropIndexFromOrdinals(m_pTable, NULL, m_rgColumnDesc, m_cColumnDesc, 
			1, &m_nIndex, rgOrder, 1, rgPropSet), DB_S_ERRORSOCCURRED);
	TESTC(DBPROPSTATUS_BADCOLUMN == rgProp[0].dwStatus);

	// check colid is ignored for the column property
	rgProp[0].dwOptions = DBPROPOPTIONS_REQUIRED;
	TESTC_(hr = CCNDropIndexFromOrdinals(m_pTable, NULL, m_rgColumnDesc, m_cColumnDesc, 
			1, &m_nIndex, rgOrder, 1, rgPropSet), DB_E_ERRORSOCCURRED);
	TESTC(DBPROPSTATUS_BADCOLUMN == rgProp[0].dwStatus);

	fTestRes = TEST_PASS;

CLEANUP:
	ReleaseColumnDesc(rgColumnDesc, cColumnDesc);
	return fTestRes;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCCreateIndex::Terminate()
{ 
// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIIndexDefinition::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END




// {{ TCW_TC_PROTOTYPE(TCDropIndex)
//*-----------------------------------------------------------------------
//| Test Case:		TCDropIndex - tests IIndexDefinition::DropIndex() method
//| Created:  	11/6/97
//*-----------------------------------------------------------------------



//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCDropIndex::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCIIndexDefinition::Init())
	// }}
		return TRUE;
	return FALSE;
} 




// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc NULL pointer for pTableID => E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCDropIndex::Variation_1()
{ 
	BOOL	fTestResult = TEST_PASS;
	DBID	IndexID;

	IndexID.eKind			= DBKIND_NAME;
	IndexID.uName.pwszName	= L"NewCrolo";
	if (!CHECK(m_hr = m_pCIIndexDefinition->DropIndex(NULL, &IndexID), E_INVALIDARG))
		fTestResult = TEST_FAIL;
	return fTestResult;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc *pTableID == DB_NULLID => DB_E_NOTABLE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCDropIndex::Variation_2()
{ 
	BOOL	fTestResult = TEST_PASS;
	DBID	IndexID;

	IndexID.eKind			= DBKIND_NAME;
	IndexID.uName.pwszName	= L"NewCrolo";
	if (!CHECK(m_hr = m_pCIIndexDefinition->DropIndex((struct tagDBID*)&DB_NULLID, &IndexID), DB_E_NOTABLE))
		fTestResult = TEST_FAIL;
	return fTestResult;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc pTableID->uName is NULL => DB_E_NOTABLE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCDropIndex::Variation_3()
{ 
	BOOL	fTestResult = TEST_PASS;
	DBID	IndexID, TableID;

	IndexID.eKind			= DBKIND_NAME;
	IndexID.uName.pwszName	= L"NewCrolo";
	TableID.eKind			= DBKIND_NAME;
	TableID.uName.pwszName	= NULL;
	if (!CHECK(m_hr = m_pCIIndexDefinition->DropIndex(&TableID, &IndexID), DB_E_NOTABLE))
		fTestResult = TEST_FAIL;
	return fTestResult;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc pTableID->uName is empty => DB_E_NOTABLE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCDropIndex::Variation_4()
{ 
	BOOL	fTestResult = TEST_PASS;
	DBID	IndexID, TableID;

	IndexID.eKind			= DBKIND_NAME;
	IndexID.uName.pwszName	= L"NewCrolo";
	TableID.eKind			= DBKIND_NAME;
	TableID.uName.pwszName	= L"";
	if (!CHECK(m_hr = m_pCIIndexDefinition->DropIndex(&TableID, &IndexID), DB_E_NOTABLE))
		fTestResult = TEST_FAIL;
	return fTestResult;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc invalid table name => DB_E_NOTABLE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCDropIndex::Variation_5()
{ 
	BOOL	fTestResult = TEST_FAIL;
	DBID	IndexID;
	DBID	TableID;
	size_t	m, n;

	IndexID.eKind = DBKIND_NAME;
	IndexID.uName.pwszName = L"Galusca";

	// get an invalid table name
	TableID.eKind = DBKIND_NAME;
	m = min(m_cMaxTableName, n=5+wcslen(m_pTable->GetTableName()));
	TableID.uName.pwszName	= BuildInvalidName(m, m_pTable->GetTableName(), m_pwszInvalidTableChars);
	
	TESTC_(m_hr = m_pCIIndexDefinition->DropIndex(&TableID, &IndexID), DB_E_NOTABLE);

	fTestResult = TEST_PASS;
CLEANUP:
	ReleaseDBID(&TableID, FALSE);
	return fTestResult;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc maximum sized table name => S_OK
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCDropIndex::Variation_6()
{ 
	BOOL				fTestResult = TEST_FAIL;
	DBID				IndexID, *pIndexID=NULL;
	DBID				TableID;
	ULONG				nIndex;
	CTable				*pTable = NULL;

	IndexID.eKind = DBKIND_NAME;
	IndexID.uName.pwszName = L"Galusca";

	// get a valid large table name
	TableID.eKind = DBKIND_NAME;
	TableID.uName.pwszName	= BuildValidName(m_cMaxTableName, m_pTable->GetTableName());
	
	pTable = new CTable(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);
	TESTC(NULL != pTable);
	pTable->SetColumnDesc(m_rgColumnDesc, m_cColumnDesc);
	pTable->SetBuildColumnDesc(FALSE);
	TESTC_(m_hr = pTable->CreateTable(0, 0, TableID.uName.pwszName), S_OK);

	if (CHECK(m_hr = SetIndex(&TableID, &IndexID, &nIndex, &pIndexID), S_OK))
	{
		if (CHECK(m_hr = m_pCIIndexDefinition->DropIndex(&TableID, &IndexID), S_OK))
			fTestResult = TEST_PASS;
	}
	else
	{
		odtLog << "could not set index\n";
	}

CLEANUP:
	pTable->SetColumnDesc(NULL);
	pTable->DropTable();
	delete pTable;
	ReleaseDBID(&TableID, FALSE);
	ReleaseDBID(pIndexID);
	return fTestResult;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc table name exceeds maximum size => DB_E_NOTABLE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCDropIndex::Variation_7()
{ 
	BOOL				fTestResult = TEST_FAIL;
	DBID				IndexID;
	DBID				TableID;

	IndexID.eKind = DBKIND_NAME;
	IndexID.uName.pwszName = L"Galusca";

	// get an n extralarge valid table name
	TableID.eKind = DBKIND_NAME;
	TableID.uName.pwszName	= BuildValidName(m_cMaxTableName+1, m_pTable->GetTableName());

	if (CHECK(m_hr = m_pCIIndexDefinition->DropIndex(&TableID, &IndexID), DB_E_NOTABLE))
		fTestResult = TEST_PASS;

	SAFE_FREE(TableID.uName.pwszName);
	return fTestResult;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc NULL pointer for pIndexID => drop all indexes
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCDropIndex::Variation_8()
{ 
	TBEGIN
	BOOL				fExists;
	ULONG				nIndex = 0, i;
	DBID				**rgIndexID=NULL;
	ULONG				rgOrder[1]	= {DBINDEX_COL_ORDER_ASC};

	// create an index array
	SAFE_ALLOC(rgIndexID, DBID*, m_cColumnDesc);
	for (i=0; i<m_cIndexCols; i++)
	{
		rgIndexID[nIndex] = NULL;
		if (S_OK == CNCIndexFromOrdinals(m_pPopulatedTable, NULL, &rgIndexID[nIndex], 
			m_rgColumnDesc, m_cColumnDesc, 1, &m_rgIndexCols[i], rgOrder))
			nIndex++;
	}

	odtLog << "\t" << nIndex << " indexes were created on the table\n";

	// drop indexes and tables
	TESTC_(m_hr = m_pCIIndexDefinition->DropIndex(&m_pPopulatedTable->GetTableID(), NULL), S_OK);
	TEST2C_(m_hr = m_pCIIndexDefinition->DoesIndexExist(&m_pPopulatedTable->GetTableID(), NULL, &fExists, TRUE), S_OK, DB_S_ERRORSOCCURRED);
	TESTC_PROVIDER(S_OK == m_hr);
	TESTC(!fExists);

CLEANUP:
	for (i=0; i<nIndex; i++)
		ReleaseDBID(rgIndexID[i]);
	SAFE_FREE(rgIndexID);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc *pIndexID == DB_NULLID => DB_E_NOINDEX
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCDropIndex::Variation_9()
{ 
	TBEGIN

	CHECK(m_hr = m_pCIIndexDefinition->DropIndex(&m_pTable->GetTableID(), 
								 (struct tagDBID*)&DB_NULLID), DB_E_NOINDEX);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc pIndexID->uName is NULL => DB_E_NOINDEX
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCDropIndex::Variation_10()
{ 
	BOOL	fTestResult = TEST_PASS;
	DBID	IndexID;

	IndexID.eKind			= DBKIND_NAME;
	IndexID.uName.pwszName	= NULL;
	if (!CHECK(m_hr = m_pCIIndexDefinition->DropIndex(&m_pTable->GetTableID(), &IndexID), DB_E_NOINDEX))
		fTestResult = TEST_FAIL;
	return fTestResult;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc pIndexID->uName is empty => DB_E_NOINDEX
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCDropIndex::Variation_11()
{ 
	BOOL	fTestResult = TEST_PASS;
	DBID	IndexID;

	IndexID.eKind			= DBKIND_NAME;
	IndexID.uName.pwszName	= L"";
	if (!CHECK(m_hr = m_pCIIndexDefinition->DropIndex(&m_pTable->GetTableID(), &IndexID), DB_E_NOINDEX))
		fTestResult = TEST_FAIL;
	return fTestResult;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc invalid index name => DB_E_NOINDEX
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCDropIndex::Variation_12()
{ 
	BOOL				fTestResult = TEST_FAIL;
	DBID				IndexID;
	size_t				m, n;


	// get an invalid index name
	IndexID.eKind = DBKIND_NAME;
	m = min(m_cMaxIndexName, n=5+wcslen(m_pTable->GetTableName()));
	IndexID.uName.pwszName	= BuildInvalidName(m, m_pTable->GetTableName(), m_pwszInvalidIndexChars);
	
	CHECK(m_hr = m_pCIIndexDefinition->DropIndex(&m_pTable->GetTableID(), &IndexID), DB_E_NOINDEX);
	SAFE_FREE(IndexID.uName.pwszName);
	if (DB_E_NOINDEX == m_hr)
		fTestResult = TEST_PASS;

	return fTestResult;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc maximum size index name => S_OK
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCDropIndex::Variation_13()
{ 
	BOOL				fTestResult = TEST_FAIL;
	DBID				IndexID;
	ULONG				nIndex;

	// get a maximum size index name
	IndexID.eKind			= DBKIND_NAME;
	IndexID.uName.pwszName	= BuildValidName(m_cMaxIndexName, m_pTable->GetTableName());
	
	// set an index
	if (CHECK(m_hr = SetIndex(&m_pTable->GetTableID(), &IndexID, &nIndex, NULL), S_OK))
	{
		// drop the table and check the existence of the index
		if (CHECK(m_hr = m_pCIIndexDefinition->DropIndex(&m_pTable->GetTableID(), &IndexID), S_OK))
			fTestResult = TEST_PASS;
	}

	SAFE_FREE(IndexID.uName.pwszName);
	return fTestResult;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc oversized index name => DB_E_NOINDEX
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCDropIndex::Variation_14()
{ 
	BOOL				fTestResult = TEST_FAIL;
	DBID				IndexID;

	// get an too long index name
	IndexID.eKind			= DBKIND_NAME;
	IndexID.uName.pwszName	= BuildValidName(m_cMaxIndexName+1, m_pTable->GetTableName());
	
	if (CHECK(m_hr = m_pCIIndexDefinition->DropIndex(&m_pTable->GetTableID(), &IndexID), DB_E_NOINDEX))
			fTestResult = TEST_PASS;

	SAFE_FREE(IndexID.uName.pwszName);
	return fTestResult;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc drop an index twice => DB_E_NOINDEX
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCDropIndex::Variation_15()
{ 
	BOOL				fTestResult = TEST_FAIL;
	DBID				IndexID;
	ULONG				nIndex;

	// prepare an index name
	IndexID.eKind			= DBKIND_NAME;
	IndexID.uName.pwszName	= L"Galusca"; 
	

	// set an index
	if (CHECK(m_hr = SetIndex(&m_pTable->GetTableID(), &IndexID, &nIndex, NULL), S_OK))
	{
		// drop the table and check the existence of the index
		if (CHECK(m_hr = m_pCIIndexDefinition->DropIndex(&m_pTable->GetTableID(), &IndexID), S_OK) 
			&& CHECK(m_hr = m_pCIIndexDefinition->DropIndex(&m_pTable->GetTableID(), &IndexID), DB_E_NOINDEX)
			)
			fTestResult = TEST_PASS;
	}

	return fTestResult;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc inexistent table => DB_E_NOTABLE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCDropIndex::Variation_16()
{ 
	BOOL	fTestResult = TEST_FAIL;
	DBID	*pIndexID=NULL;
	DBID	TableID;
	CTable	*pTable=NULL;
	ULONG	nIndex;
	HRESULT	hr;

	// create a table
	TESTC(NULL != (pTable = new CTable(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName)));
	pTable->SetColumnDesc(m_rgColumnDesc, m_cColumnDesc);
	pTable->SetBuildColumnDesc(FALSE);
	TESTC_(pTable->CreateTable(0, 0), S_OK);
	// set an index on the table
	if (!CHECK(m_hr = SetIndex(&pTable->GetTableID(), NULL, &nIndex, &pIndexID), S_OK))
	{
		fTestResult = TEST_SKIPPED;
		goto CLEANUP;
	}

	// drop the table
	DuplicateDBID(pTable->GetTableID(), &TableID);
	TESTC_(pTable->DropTable(&TableID), S_OK);
	
	// try to drop the index
	hr = m_pCIIndexDefinition->DropIndex(&TableID, pIndexID);

	TEST2C_(hr, DB_E_NOTABLE, DB_E_NOINDEX);
	fTestResult = TEST_PASS;

CLEANUP:
	pTable->SetColumnDesc(NULL);
	pTable->DropTable();
	delete pTable;
	ReleaseDBID(pIndexID);
	ReleaseDBID(&TableID, FALSE);

	return fTestResult;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc All DBKIND for pTableID
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCDropIndex::Variation_17()
{ 
	BOOL				fTestResult = TEST_PASS;
	DBID				TableID, IndexID;

	IndexID.eKind			= DBKIND_NAME;
	IndexID.uName.pwszName	= L"AltNume";
	TableID.uName.pwszName	= m_pTable->GetTableName();
	TableID.uGuid.pguid		= &guidModule;
	
	// DBKIND_NAME
	TableID.eKind = DBKIND_NAME;
	m_hr = m_pCIIndexDefinition->DropIndex(&TableID, &IndexID);
	if (!CHECK(m_hr, DB_E_NOINDEX))
	{
		odtLog << "DBKIND_NAME\n";
		fTestResult = TEST_FAIL;
	}
	// DBKIND_GUID_NAME
	TableID.eKind = DBKIND_GUID_NAME;
	TableID.uGuid.guid	= guidModule;
	m_hr = m_pCIIndexDefinition->DropIndex(&TableID, &IndexID);
	if (!CHECK(m_hr, DB_E_NOTABLE))
	{
		odtLog << "DBKIND_GUID_NAME\n";
		fTestResult = TEST_FAIL;
	}
	// DBKIND_PGUID_NAME
	TableID.eKind = DBKIND_PGUID_NAME;
	TableID.uGuid.pguid		= &guidModule;
	m_hr = m_pCIIndexDefinition->DropIndex(&TableID, &IndexID);
	if (!CHECK(m_hr, DB_E_NOTABLE))
	{
		odtLog << "DBKIND_PGUID_NAME\n";
		fTestResult = TEST_FAIL;
	}
	// DBKIND_GUID
	TableID.eKind = DBKIND_GUID;
	TableID.uGuid.guid = guidModule;
	m_hr = m_pCIIndexDefinition->DropIndex(&TableID, &IndexID);
	if (!CHECK(m_hr, DB_E_NOTABLE))
	{
		odtLog << "DBKIND_GUID\n";
		fTestResult = TEST_FAIL;
	}
	TableID.uName.ulPropid	= 0;
	TableID.uName.pwszName = NULL;

	// DBKIND_GUID_PROPID
	TableID.eKind = DBKIND_GUID_PROPID;
	TableID.uGuid.guid	= guidModule;
	m_hr = m_pCIIndexDefinition->DropIndex(&TableID, &IndexID);
	if (!CHECK(m_hr, DB_E_NOTABLE))
	{
		odtLog << "DBKIND_GUID_PROPID\n";
		fTestResult = TEST_FAIL;
	}
	// DBKIND_PROPID
	TableID.eKind = DBKIND_PROPID;
	m_hr = m_pCIIndexDefinition->DropIndex(&TableID, &IndexID);
	if (!CHECK(m_hr, DB_E_NOTABLE))
	{
		odtLog << "DBKIND_PROPID\n";
		fTestResult = TEST_FAIL;
	}
	// DBKIND_PGUID_PROPID
	TableID.eKind = DBKIND_PGUID_PROPID;
	TableID.uGuid.pguid		= &guidModule;
	m_hr = m_pCIIndexDefinition->DropIndex(&TableID, &IndexID);
	if (!CHECK(m_hr, DB_E_NOTABLE))
	{
		odtLog << "DBKIND_PGUID_PROPID\n";
		fTestResult = TEST_FAIL;
	}
	return fTestResult;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc All DBKIND for pIndexID
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCDropIndex::Variation_18()
{ 
	BOOL				fTestResult = TEST_PASS;
	DBID				IndexID;
	
	IndexID.uName.pwszName	= L"galusca";
	IndexID.uGuid.pguid		= &guidModule;
	
	// DBKIND_NAME
	IndexID.eKind = DBKIND_NAME;
	m_hr = m_pCIIndexDefinition->DropIndex(&m_pTable->GetTableID(), &IndexID);
	if (!CHECK(m_hr, DB_E_NOINDEX))
	{
		odtLog << "DBKIND_NAME\n";
		fTestResult = TEST_FAIL;
	}
	// DBKIND_GUID_NAME
	IndexID.eKind = DBKIND_GUID_NAME;
	IndexID.uGuid.guid = guidModule;
	m_hr = m_pCIIndexDefinition->DropIndex(&m_pTable->GetTableID(), &IndexID);
	if (!CHECK(m_hr, DB_E_NOINDEX))
	{
		odtLog << "DBKIND_GUID_NAME\n";
		fTestResult = TEST_FAIL;
	}
	// DBKIND_PGUID_NAME
	IndexID.eKind = DBKIND_PGUID_NAME;
	IndexID.uGuid.pguid		= &guidModule;
	m_hr = m_pCIIndexDefinition->DropIndex(&m_pTable->GetTableID(), &IndexID);
	if (!CHECK(m_hr, DB_E_NOINDEX))
	{
		odtLog << "DBKIND_PGUID_NAME\n";
		fTestResult = TEST_FAIL;
	}
	// DBKIND_GUID
	IndexID.eKind = DBKIND_GUID;
	IndexID.uGuid.guid = guidModule;
	m_hr = m_pCIIndexDefinition->DropIndex(&m_pTable->GetTableID(), &IndexID);
	if (!CHECK(m_hr, DB_E_NOINDEX))
	{
		odtLog << "DBKIND_GUID\n";
		fTestResult = TEST_FAIL;
	}
	IndexID.uName.ulPropid	= 0;
	IndexID.uName.pwszName	= NULL;
	// DBKIND_GUID_PROPID
	IndexID.eKind = DBKIND_GUID_PROPID;
	IndexID.uGuid.guid = guidModule;
	m_hr = m_pCIIndexDefinition->DropIndex(&m_pTable->GetTableID(), &IndexID);
	if (!CHECK(m_hr, DB_E_NOINDEX))
	{
		odtLog << "DBKIND_GUID_PROPID\n";
		fTestResult = TEST_FAIL;
	}
	// DBKIND_PROPID
	IndexID.eKind = DBKIND_PROPID;
	m_hr = m_pCIIndexDefinition->DropIndex(&m_pTable->GetTableID(), &IndexID);
	if (!CHECK(m_hr, DB_E_NOINDEX))
	{
		odtLog << "DBKIND_PROPID\n";
		fTestResult = TEST_FAIL;
	}
	// DBKIND_PGUID_PROPID
	IndexID.eKind = DBKIND_PGUID_PROPID;
	IndexID.uGuid.pguid		= &guidModule;
	m_hr = m_pCIIndexDefinition->DropIndex(&m_pTable->GetTableID(), &IndexID);
	if (!CHECK(m_hr, DB_E_NOINDEX))
	{
		odtLog << "DBKIND_PGUID_PROPID\n";
		fTestResult = TEST_FAIL;
	}
	
	return fTestResult;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc index in use => DB_E_INDEXINUSE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCDropIndex::Variation_19()
{ 
	TBEGIN
	BOOL				fExists;
	DBINDEX_COL_ORDER	rgOrder[]={DBINDEX_COL_ORDER_ASC};
	IRowsetIndex		*pIRowsetIndex = NULL;
	CDBIDPtr			IndexID;

	if (S_OK != CNCIndexFromOrdinals(m_pTable, NULL, (DBID**)IndexID, 
		m_rgColumnDesc, m_cColumnDesc, 1, &m_nIndex, rgOrder))
	{
		odtLog << "Could not properly create an index!\n"; 
		goto CLEANUP;
	}

	// try to open a rowset index on it
	TESTC_PROVIDER(SUCCEEDED(m_hr = m_pIOpenRowset->OpenRowset(NULL, &m_pTable->GetTableID(), (DBID*)IndexID, IID_IRowsetIndex,
						0, NULL, (IUnknown**)&pIRowsetIndex)));

	if ((DBID*)IndexID && CHECK(m_pCIIndexDefinition->DoesIndexExist(&m_pTable->GetTableID(), (DBID*)IndexID, &fExists), S_OK) 
		&& fExists)
		m_hr = m_pCIIndexDefinition->DropIndex(&m_pTable->GetTableID(), (DBID*)IndexID);
	// take care, DB_E_TABLEINUSE is also allowed according to the spec
	if (DB_E_TABLEINUSE != m_hr && !CHECK(m_hr, DB_E_INDEXINUSE))
	{
		odtLog << "could drop the index while a RowsetIndex was open\n";
		goto CLEANUP;
	}

CLEANUP:
	SAFE_RELEASE(pIRowsetIndex);
	m_pCIIndexDefinition->DropIndex(&m_pTable->GetTableID(), (DBID*)IndexID);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




//------------------------------------------------------------------
//
//	thread function
//------------------------------------------------------------------
unsigned TCDropIndex::MyThreadProc(ULONG i)
{
	DBID		IndexID;
	BOOL		fExists;
	HRESULT		hr;

	if (i>=nThreads)
		return 0;
	IndexID.eKind = DBKIND_NAME;
	IndexID.uName.pwszName = m_rgIndexName[i];
	Sleep(0);
	hr = ((IIndexDefinition*)*m_pCIIndexDefinition)->DropIndex(&m_pTable->GetTableID(), &IndexID);
	Sleep(0);
	m_rgResult[i] = hr;
	hr = m_pCIIndexDefinition->DoesIndexExist(&m_pTable->GetTableID(), &IndexID, &fExists);
	return !CHECK(hr, S_OK) || fExists? 0: 1;
} //TCDropIndex::MyThreadProc




// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc multithreading
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCDropIndex::Variation_20()
{ 
	unsigned		IDThread[nThreads];
	HANDLE			hThread[nThreads];
	CInParam		ThreadParam[nThreads];
	DBID			IndexID;
	BOOL				fTestResult = TEST_FAIL, fExists;
	DBINDEX_COL_ORDER	rgOrder[]={DBINDEX_COL_ORDER_ASC};
	WCHAR				pwszNameSeed[]=L"theSameThread";
	ULONG				nIndex;
	
	for (nIndex = 0; nIndex < nThreads; nIndex++)
	{
		m_rgResult[nIndex]			= E_FAIL;
		ThreadParam[nIndex].i		= nIndex;
		ThreadParam[nIndex].pObject = this;
		SAFE_ALLOC(m_rgIndexName[nIndex], WCHAR, wcslen(pwszNameSeed)+3);
		swprintf(m_rgIndexName[nIndex], L"%s%02d", pwszNameSeed, nIndex);
	
		IndexID.eKind			= DBKIND_NAME;
		IndexID.uName.pwszName	= m_rgIndexName[nIndex];
		if (!CHECK(CNCIndexFromOrdinals(m_pTable, &IndexID, NULL, 
			m_rgColumnDesc, m_cColumnDesc, 1, &m_nIndex, rgOrder), S_OK))
		{
			odtLog << "index " << m_rgIndexName[0] << " could not be created\n";
			goto CLEANUP;
		}
	}

	for (nIndex = 0; nIndex < nThreads; nIndex++)
	{
		hThread[nIndex] = (HANDLE)_beginthreadex(NULL, 0, ThreadProc,
							(void*)&ThreadParam[nIndex],
							0, 
							&IDThread[nIndex]);		
		if (hThread[nIndex] == 0)
		{	
			fTestResult = TEST_FAIL;
			goto CLEANUP;
		}
	}

	WaitForMultipleObjects(nThreads, hThread, TRUE, INFINITE);
	fTestResult = TEST_PASS;
	for (nIndex=0; nIndex<nThreads; nIndex++)
	{
		CloseHandle(hThread[nIndex]);
		IndexID.eKind			= DBKIND_NAME;
		IndexID.uName.pwszName	= m_rgIndexName[nIndex];
		fExists = FALSE;
		if (S_OK != m_rgResult[nIndex] 
			|| !CHECK(m_hr = m_pCIIndexDefinition->DoesIndexExist(&m_pTable->GetTableID(), &IndexID, &fExists), S_OK) 
			|| fExists)
			fTestResult = TEST_FAIL;
		if (fExists)
			CHECK(m_hr = m_pCIIndexDefinition->DropIndex(&m_pTable->GetTableID(), &IndexID), S_OK);
		SAFE_FREE(m_rgIndexName[nIndex]);
	}

CLEANUP:
	return fTestResult;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc 2 threads, the same index
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCDropIndex::Variation_21()
{ 
	unsigned		IDThread[nThreads];
	HANDLE			hThread[nThreads];
	CInParam		ThreadParam[nThreads];
	DBID			IndexID;
	BOOL			fTestResult = TEST_FAIL, fExists;
	ULONG			rgOrder[1] = {0};
	ULONG			nSuccess = 0;
	ULONG			nIndex;
	WCHAR			pwszIndexName[] = L"theSameIndex";

	IndexID.eKind			= DBKIND_NAME;
	IndexID.uName.pwszName	= pwszIndexName;
	TESTC_(m_hr = CNCIndexFromOrdinals(m_pTable, &IndexID, NULL, 
		m_rgColumnDesc, m_cColumnDesc, 1, &m_nIndex, rgOrder), S_OK);

	fTestResult = TEST_FAIL;
	for (nIndex = 0; nIndex < nThreads; nIndex++)
	{
		m_rgResult[nIndex]			= E_FAIL;
		ThreadParam[nIndex].i		= nIndex;
		ThreadParam[nIndex].pObject = this;
		m_rgIndexName[nIndex]		= pwszIndexName;
		hThread[nIndex] = (HANDLE)_beginthreadex(NULL, 0, ThreadProc,
							(void*)&ThreadParam[nIndex],
							0, 
							&IDThread[nIndex]);		
		if (hThread[nIndex] == 0)
		{	
			fTestResult = TEST_FAIL;
			goto CLEANUP;
		}
	}

	WaitForMultipleObjects(nThreads, hThread, TRUE, INFINITE);

	fTestResult = TEST_PASS;
	nSuccess = 0;
	for (nIndex=0; nIndex<nThreads; nIndex++)
	{
		CloseHandle(hThread[nIndex]);
		fExists = FALSE;
		if (S_OK == m_rgResult[nIndex])
			nSuccess++;
	}

	IndexID.eKind			= DBKIND_NAME;
	IndexID.uName.pwszName	= m_rgIndexName[0];
	if (	!GCOMPARE(nSuccess, 1)
		||	!CHECK(m_hr = m_pCIIndexDefinition->DoesIndexExist(&m_pTable->GetTableID(), &IndexID, &fExists), S_OK) 
		||	!CHECK(fExists, FALSE))
	{
		odtLog << "errors in dropping the index\n";
		goto CLEANUP;
	}

	if ( ((S_OK == m_rgResult[0]) && (DB_E_NOINDEX == m_rgResult[1])) ||
		 ((S_OK == m_rgResult[0]) && (DB_E_NOINDEX == m_rgResult[1])) )
		fTestResult = TEST_PASS;

CLEANUP:
	IndexID.eKind			= DBKIND_NAME;
	IndexID.uName.pwszName	= m_rgIndexName[0];
	m_hr = m_pCIIndexDefinition->DropIndex(&m_pTable->GetTableID(), &IndexID);
	return fTestResult;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(22)
//*-----------------------------------------------------------------------
// @mfunc Inexistent index name
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCDropIndex::Variation_22()
{ 
	TBEGIN
	DBID	IndexID;
	DBID	TableID;
	CTable	*pTable=NULL;
	HRESULT	hr;

	// create a table
	TESTC(NULL != (pTable = new CTable(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName)));
	
	TESTC_(pTable->CreateTable(0, 0), S_OK);
	
	DuplicateDBID(pTable->GetTableID(), &TableID);
	
	// try to drop the index
	IndexID.eKind = DBKIND_NAME;
	IndexID.uName.pwszName = L"AltNume";
	hr = m_pCIIndexDefinition->DropIndex(&TableID, &IndexID);

	TEST2C_(hr, DB_E_NOTABLE, DB_E_NOINDEX);

CLEANUP:
	if (pTable)
	{
		pTable->SetColumnDesc(NULL);
		pTable->DropTable();
		delete pTable;
	}
	ReleaseDBID(&TableID, FALSE);

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCDropIndex::Terminate()
{ 
// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIIndexDefinition::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END


// {{ TCW_TC_PROTOTYPE(TCTrans)
//*-----------------------------------------------------------------------
//| Test Case:		TCTrans - transactions
//| Created:  	11/19/97
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCTrans::Init()
{ 
	TBEGIN
	ITransactionLocal	*pITransactionLocal = NULL;

	// {{ TCW_INIT_BASECLASS_CHECK
	if (!TCIIndexDefinition::Init())
		return FALSE;
	// }}

	// there is at least one column on which an index can be set
	TESTC_PROVIDER(0 < m_cIndexCols);
	m_rgIndexColumnDesc[0].pColumnID		= &m_rgColumnDesc[m_rgIndexCols[0]-1].dbcid;
	m_rgIndexColumnDesc[0].eIndexColOrder	= DBINDEX_COL_ORDER_ASC;

	// if transaction are not supported, skip the testcase
	TESTC_PROVIDER(VerifyInterface(*m_pCIIndexDefinition, IID_ITransactionLocal, 
			SESSION_INTERFACE, (IUnknown**)&pITransactionLocal));

	// transaction are supporte, get the value of DBPROP_SUPPORTEDTXNDDL prop
	// it should be supported when transactions are supported
	VariantInit(&m_vSupportedTxnDDL);
	if (!GetProperty(DBPROP_SUPPORTEDTXNDDL, DBPROPSET_DATASOURCEINFO, 
			g_pIDBInitialize, &m_vSupportedTxnDDL))
	{
		TOUTPUT("Warning: ITransactionLocal is supported, but DBPROP_SUPPORTEDTXNDDL is not supported!");
		TESTB = TEST_SKIPPED;
		goto CLEANUP;
	}
	
	TESTC(VT_I4 == m_vSupportedTxnDDL.vt);
	// what values should we accept?
	TESTC_PROVIDER(DBPROPVAL_TC_NONE != m_vSupportedTxnDDL.lVal);
	
CLEANUP:
	SAFE_RELEASE(pITransactionLocal);
	TRETURN
} 

HRESULT TCTrans::StartTransaction(ITransactionLocal *pITransactionLocal)
{
	ULONG		n, i;
	HRESULT		hr;
	ISOLEVEL	rgIsoLevel[] = {ISOLATIONLEVEL_BROWSE, ISOLATIONLEVEL_CURSORSTABILITY,
								ISOLATIONLEVEL_ISOLATED, ISOLATIONLEVEL_CHAOS};

	if (NULL == pITransactionLocal)
		return E_FAIL;
	
	n = NUMELEM(rgIsoLevel);
	for (i=0; i<n; i++)
	{
		if (S_OK == (hr = pITransactionLocal->StartTransaction(rgIsoLevel[i], 0, NULL, NULL)))
			return S_OK;
	}
	return E_FAIL;
}



// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Abort retaining on CreateIndex
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCTrans::Variation_1()
{ 
	TBEGIN
	BOOL				fExists;
	ITransactionLocal	*pITransactionLocal=NULL;
	DBID				*pIndexID = NULL;

	TESTC(VerifyInterface(*m_pCIIndexDefinition, IID_ITransactionLocal, 
			SESSION_INTERFACE, (IUnknown**)&pITransactionLocal));

	// start the transaction
	if (!CHECK(m_hr = StartTransaction(pITransactionLocal), S_OK))
	{
		odtLog << "Could not start an isolated local transaction\n";
		TESTB = TEST_SKIPPED;
		goto CLEANUP;
	}

	switch (V_I4(&m_vSupportedTxnDDL))
	{
		case DBPROPVAL_TC_DML:
			//check for XACT_E_XTIONEXISTS
			CHECK(m_hr = m_pCIIndexDefinition->CreateIndex(
				&m_pTable->GetTableID(), 
				NULL,
				1, m_rgIndexColumnDesc,
				0, NULL,
				&pIndexID), XACT_E_XTIONEXISTS);
			break;

		case DBPROPVAL_TC_DDL_IGNORE:
			// the statement is not transacted
		case DBPROPVAL_TC_ALL:
		case DBPROPVAL_TC_DDL_COMMIT:
			// create an index
			CHECK(m_hr = m_pCIIndexDefinition->CreateIndexAndCheck(&m_pTable->GetTableID(), NULL, 1, m_rgIndexColumnDesc, 0, NULL, &pIndexID), S_OK);
			//CHECK(m_hr = SetIndex(&m_pTable->GetTableID(), NULL, &nIndex, &pIndexID), S_OK);
			break;

		case DBPROPVAL_TC_DDL_LOCK:
			// error should be returned
			CHECK(m_pCIIndexDefinition->CreateIndex(
				&m_pTable->GetTableID(), 
				NULL,
				1, m_rgIndexColumnDesc,
				0, NULL,
				&pIndexID), S_OK);
			break;
			
		default:
			TESTC(FALSE);
	}

	if (DBPROPVAL_TC_DDL_COMMIT != V_I4(&m_vSupportedTxnDDL))
	{
		// abort retaining
		CHECK(m_hr = pITransactionLocal->Abort(NULL, TRUE, FALSE), S_OK);
		// check a new transaction was created
		CHECK(m_hr = pITransactionLocal->Abort(NULL, FALSE, FALSE), S_OK);
	}
	
	// check index creation
	CHECK(m_hr = m_pCIIndexDefinition->DoesIndexExist(&m_pTable->GetTableID(), pIndexID, &fExists), S_OK);
	COMPARE(fExists, DBPROPVAL_TC_DDL_IGNORE == V_I4(&m_vSupportedTxnDDL)
		||	DBPROPVAL_TC_DDL_COMMIT == V_I4(&m_vSupportedTxnDDL));

CLEANUP:
	SAFE_RELEASE(pITransactionLocal);
	if (pIndexID && CHECK(m_pCIIndexDefinition->DoesIndexExist(&m_pTable->GetTableID(), pIndexID, &fExists), S_OK) && fExists)
		m_pCIIndexDefinition->DropIndex(&m_pTable->GetTableID(), pIndexID);
	ReleaseDBID(pIndexID);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Abort non retaining on CreateIndex
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCTrans::Variation_2()
{ 
	TBEGIN
	BOOL				fExists;
	ITransactionLocal	*pITransactionLocal=NULL;
	DBID				*pIndexID = NULL;

	TESTC(VerifyInterface(*m_pCIIndexDefinition, IID_ITransactionLocal, 
			SESSION_INTERFACE, (IUnknown**)&pITransactionLocal));

	// start the transaction
	if (!CHECK(m_hr = StartTransaction(pITransactionLocal), S_OK))
	{
		odtLog << "Could not start an isolated local transaction\n";
		TESTB = TEST_SKIPPED;
		goto CLEANUP;
	}

	switch (V_I4(&m_vSupportedTxnDDL))
	{
		case DBPROPVAL_TC_DML:
			//check for XACT_E_XTIONEXISTS
			CHECK(m_hr = m_pCIIndexDefinition->CreateIndex(
				&m_pTable->GetTableID(), 
				NULL,
				1, m_rgIndexColumnDesc,
				0, NULL,
				&pIndexID), XACT_E_XTIONEXISTS);
			break;

		case DBPROPVAL_TC_DDL_IGNORE:
			// the statement is not transacted
		case DBPROPVAL_TC_ALL:
		case DBPROPVAL_TC_DDL_COMMIT:
			// create an index
			CHECK(m_hr = m_pCIIndexDefinition->CreateIndexAndCheck(&m_pTable->GetTableID(), NULL, 1, m_rgIndexColumnDesc, 0, NULL, &pIndexID), S_OK);
			//CHECK(m_hr = SetIndex(&m_pTable->GetTableID(), NULL, &nIndex, &pIndexID), S_OK);
			break;

		case DBPROPVAL_TC_DDL_LOCK:
			// error should be returned
			CHECK(m_pCIIndexDefinition->CreateIndex(
				&m_pTable->GetTableID(), 
				NULL,
				1, m_rgIndexColumnDesc,
				0, NULL,
				&pIndexID), S_OK);
			break;
			
		default:
			TESTC(FALSE);
	}

	if (DBPROPVAL_TC_DDL_COMMIT != V_I4(&m_vSupportedTxnDDL))
	{
		// abort non retaining
		CHECK(m_hr = pITransactionLocal->Abort(NULL, FALSE, FALSE), S_OK);
		// check a new transaction was not created
		CHECK(pITransactionLocal->Abort(NULL, FALSE, FALSE), FAILED(m_hr)? S_OK: XACT_E_NOTRANSACTION);
	}
	
	// check index creation
	CHECK(m_hr = m_pCIIndexDefinition->DoesIndexExist(&m_pTable->GetTableID(), pIndexID, &fExists), S_OK);
	COMPARE(fExists, DBPROPVAL_TC_DDL_IGNORE == V_I4(&m_vSupportedTxnDDL)
		||	DBPROPVAL_TC_DDL_COMMIT == V_I4(&m_vSupportedTxnDDL));

CLEANUP:
	SAFE_RELEASE(pITransactionLocal);
	if (pIndexID && CHECK(m_pCIIndexDefinition->DoesIndexExist(&m_pTable->GetTableID(), pIndexID, &fExists), S_OK) && fExists)
		m_pCIIndexDefinition->DropIndex(&m_pTable->GetTableID(), pIndexID);	
	ReleaseDBID(pIndexID);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Commit retain on CreateIndex
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCTrans::Variation_3()
{ 
	TBEGIN
	BOOL				fExists;
	ITransactionLocal	*pITransactionLocal=NULL;
	DBID				*pIndexID = NULL;

	TESTC(VerifyInterface(*m_pCIIndexDefinition, IID_ITransactionLocal, 
			SESSION_INTERFACE, (IUnknown**)&pITransactionLocal));

	// start the transaction
	if (!CHECK(m_hr = StartTransaction(pITransactionLocal), S_OK))
	{
		odtLog << "Could not start an isolated local transaction\n";
		TESTB = TEST_SKIPPED;
		goto CLEANUP;
	}

	switch (V_I4(&m_vSupportedTxnDDL))
	{
		case DBPROPVAL_TC_DML:
			//check for XACT_E_XTIONEXISTS
			CHECK(m_hr = m_pCIIndexDefinition->CreateIndex(
				&m_pTable->GetTableID(), 
				NULL,
				1, m_rgIndexColumnDesc,
				0, NULL,
				&pIndexID), XACT_E_XTIONEXISTS);
			break;

		case DBPROPVAL_TC_DDL_IGNORE:
			// the statement is not transacted
		case DBPROPVAL_TC_ALL:
		case DBPROPVAL_TC_DDL_COMMIT:
			// create an index
			CHECK(m_hr = m_pCIIndexDefinition->CreateIndexAndCheck(&m_pTable->GetTableID(), NULL, 1, m_rgIndexColumnDesc, 0, NULL, &pIndexID), S_OK);
			//CHECK(m_hr = SetIndex(&m_pTable->GetTableID(), NULL, &nIndex, &pIndexID), S_OK);
			break;

		case DBPROPVAL_TC_DDL_LOCK:
			// error should be returned
			CHECK(m_pCIIndexDefinition->CreateIndex(
				&m_pTable->GetTableID(), 
				NULL,
				1, m_rgIndexColumnDesc,
				0, NULL,
				&pIndexID), S_OK);
			break;
			
		default:
			TESTC(FALSE);
	}

	if (DBPROPVAL_TC_DDL_COMMIT != V_I4(&m_vSupportedTxnDDL))
	{
		// commit retaining
		CHECK(m_hr = pITransactionLocal->Commit(TRUE, 0, 0), S_OK);
		// check a new transaction was created
		CHECK(m_hr = pITransactionLocal->Abort(NULL, FALSE, FALSE), S_OK);
	}
	
	// check index creation
	CHECK(m_hr = m_pCIIndexDefinition->DoesIndexExist(&m_pTable->GetTableID(), pIndexID, &fExists), S_OK);
	COMPARE(fExists, DBPROPVAL_TC_DML != V_I4(&m_vSupportedTxnDDL));

CLEANUP:
	SAFE_RELEASE(pITransactionLocal);
	if (pIndexID && CHECK(m_pCIIndexDefinition->DoesIndexExist(&m_pTable->GetTableID(), pIndexID, &fExists), S_OK) && fExists)
		m_pCIIndexDefinition->DropIndex(&m_pTable->GetTableID(), pIndexID);
	ReleaseDBID(pIndexID);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Commit non-retain on CreateIndex
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCTrans::Variation_4()
{ 
	TBEGIN
	BOOL				fExists;
	ITransactionLocal	*pITransactionLocal=NULL;
	DBID				*pIndexID = NULL;

	TESTC(VerifyInterface(*m_pCIIndexDefinition, IID_ITransactionLocal, 
			SESSION_INTERFACE, (IUnknown**)&pITransactionLocal));
		
	// start the transaction
	if (!CHECK(m_hr = StartTransaction(pITransactionLocal), S_OK))
	{
		odtLog << "Could not start an isolated local transaction\n";
		TESTB = TEST_SKIPPED;
		goto CLEANUP;
	}

	switch (V_I4(&m_vSupportedTxnDDL))
	{
		case DBPROPVAL_TC_DML:
			//check for XACT_E_XTIONEXISTS
			CHECK(m_hr = m_pCIIndexDefinition->CreateIndex(
				&m_pTable->GetTableID(), 
				NULL,
				1, m_rgIndexColumnDesc,
				0, NULL,
				&pIndexID), XACT_E_XTIONEXISTS);
			break;

		case DBPROPVAL_TC_DDL_IGNORE:
			// the statement is not transacted
		case DBPROPVAL_TC_ALL:
		case DBPROPVAL_TC_DDL_COMMIT:
			// create an index
			CHECK(m_hr = m_pCIIndexDefinition->CreateIndexAndCheck(&m_pTable->GetTableID(), NULL, 1, m_rgIndexColumnDesc, 0, NULL, &pIndexID), S_OK);
			//CHECK(m_hr = SetIndex(&m_pTable->GetTableID(), NULL, &nIndex, &pIndexID), S_OK);
			break;

		case DBPROPVAL_TC_DDL_LOCK:
			// error should be returned
			CHECK(m_pCIIndexDefinition->CreateIndex(
				&m_pTable->GetTableID(), 
				NULL,
				1, m_rgIndexColumnDesc,
				0, NULL,
				&pIndexID), S_OK);
			break;
			
		default:
			TESTC(FALSE);
	}

	if (DBPROPVAL_TC_DDL_COMMIT != V_I4(&m_vSupportedTxnDDL))
	{
		// commit non retaining
		CHECK(m_hr = pITransactionLocal->Commit(FALSE, 0, 0), S_OK);
		// check a new transaction was not created
		CHECK(pITransactionLocal->Abort(NULL, FALSE, FALSE), FAILED(m_hr)? S_OK: XACT_E_NOTRANSACTION);
	}
	
	// check index creation
	CHECK(m_hr = m_pCIIndexDefinition->DoesIndexExist(&m_pTable->GetTableID(), pIndexID, &fExists), S_OK);
	COMPARE(fExists, DBPROPVAL_TC_DML != V_I4(&m_vSupportedTxnDDL));

CLEANUP:
	SAFE_RELEASE(pITransactionLocal);
	if (pIndexID && CHECK(m_pCIIndexDefinition->DoesIndexExist(&m_pTable->GetTableID(), pIndexID, &fExists), S_OK) && fExists)
		m_pCIIndexDefinition->DropIndex(&m_pTable->GetTableID(), pIndexID);	
	ReleaseDBID(pIndexID);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Abort retain on DropIndex
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCTrans::Variation_5()
{ 
	TBEGIN
	BOOL				fExists;
	ITransactionLocal	*pITransactionLocal=NULL;
	DBID				*pIndexID = NULL;
	ULONG				nIndex;

	TESTC(VerifyInterface(*m_pCIIndexDefinition, IID_ITransactionLocal, 
			SESSION_INTERFACE, (IUnknown**)&pITransactionLocal));
		
	// create an index
	TESTC_(m_hr = SetIndex(&m_pTable->GetTableID(), NULL, &nIndex, &pIndexID), S_OK);

	// start the transaction
	if (!CHECK(m_hr = StartTransaction(pITransactionLocal), S_OK))
	{
		odtLog << "Could not start an isolated local transaction\n";
		TESTB = TEST_SKIPPED;
		goto CLEANUP;
	}

	switch (V_I4(&m_vSupportedTxnDDL))
	{
		case DBPROPVAL_TC_DML:
			//check for XACT_E_XTIONEXISTS
			CHECK(m_hr = m_pCIIndexDefinition->DropIndex(&m_pTable->GetTableID(), pIndexID), XACT_E_XTIONEXISTS);
			break;

		case DBPROPVAL_TC_DDL_IGNORE:
			// the statement is not transacted
		case DBPROPVAL_TC_ALL:
		case DBPROPVAL_TC_DDL_COMMIT:
			// create an index
			CHECK(m_hr = m_pCIIndexDefinition->DropIndex(&m_pTable->GetTableID(), pIndexID), S_OK);
			break;

		case DBPROPVAL_TC_DDL_LOCK:
			// error should be returned
			CHECK(m_hr = m_pCIIndexDefinition->DropIndex(&m_pTable->GetTableID(), pIndexID), S_OK);
			break;
			
		default:
			TESTC(FALSE);
	}

	if (DBPROPVAL_TC_DDL_COMMIT != V_I4(&m_vSupportedTxnDDL))
	{
		// abort retaining
		CHECK(m_hr = pITransactionLocal->Abort(NULL, TRUE, FALSE), S_OK);
		// check a new transaction was created
		CHECK(m_hr = pITransactionLocal->Abort(NULL, FALSE, FALSE), S_OK);
	}
	
	// check index deletion
	CHECK(m_hr = m_pCIIndexDefinition->DoesIndexExist(&m_pTable->GetTableID(), pIndexID, &fExists), S_OK);
	COMPARE(fExists, DBPROPVAL_TC_DDL_IGNORE != V_I4(&m_vSupportedTxnDDL)
		||	DBPROPVAL_TC_DDL_COMMIT != V_I4(&m_vSupportedTxnDDL));

CLEANUP:
	SAFE_RELEASE(pITransactionLocal);
	m_hr = m_pCIIndexDefinition->DropIndex(&m_pTable->GetTableID(), pIndexID);
	ReleaseDBID(pIndexID);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Abort non-retain on DropIndex
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCTrans::Variation_6()
{ 
	TBEGIN
	BOOL				fExists;
	ITransactionLocal	*pITransactionLocal=NULL;
	DBID				*pIndexID = NULL;
	ULONG				nIndex;

	TESTC_PROVIDER(VerifyInterface(*m_pCIIndexDefinition, IID_ITransactionLocal, 
			SESSION_INTERFACE, (IUnknown**)&pITransactionLocal));
		
	// create an index
	TESTC_(SetIndex(&m_pTable->GetTableID(), NULL, &nIndex, &pIndexID), S_OK);

	// start the transaction
	if (!CHECK(m_hr = StartTransaction(pITransactionLocal), S_OK))
	{
		odtLog << "Could not start an isolated local transaction\n";
		TESTB = TEST_SKIPPED;
		goto CLEANUP;
	}

	switch (V_I4(&m_vSupportedTxnDDL))
	{
		case DBPROPVAL_TC_DML:
			//check for XACT_E_XTIONEXISTS
			CHECK(m_hr = m_pCIIndexDefinition->DropIndex(&m_pTable->GetTableID(), pIndexID), XACT_E_XTIONEXISTS);
			break;

		case DBPROPVAL_TC_DDL_IGNORE:
			// the statement is not transacted
		case DBPROPVAL_TC_ALL:
		case DBPROPVAL_TC_DDL_COMMIT:
			// create an index
			CHECK(m_hr = m_pCIIndexDefinition->DropIndex(&m_pTable->GetTableID(), pIndexID), S_OK);
			break;

		case DBPROPVAL_TC_DDL_LOCK:
			// error should be returned
			CHECK(m_hr = m_pCIIndexDefinition->DropIndex(&m_pTable->GetTableID(), pIndexID), S_OK);
			break;
			
		default:
			TESTC(FALSE);
	}

	if (DBPROPVAL_TC_DDL_COMMIT != V_I4(&m_vSupportedTxnDDL))
	{
		// abort non retaining
		CHECK(m_hr = pITransactionLocal->Abort(NULL, FALSE, FALSE), S_OK);
		// check a new transaction was not created
		CHECK(pITransactionLocal->Abort(NULL, FALSE, FALSE), FAILED(m_hr)? S_OK: XACT_E_NOTRANSACTION);
	}
	
	// check index deletion
	CHECK(m_hr = m_pCIIndexDefinition->DoesIndexExist(&m_pTable->GetTableID(), pIndexID, &fExists), S_OK);
	COMPARE(fExists, DBPROPVAL_TC_DDL_IGNORE != V_I4(&m_vSupportedTxnDDL)
		||	DBPROPVAL_TC_DDL_COMMIT != V_I4(&m_vSupportedTxnDDL));


CLEANUP:
	SAFE_RELEASE(pITransactionLocal);
	// drop the index
	m_hr = m_pCIIndexDefinition->DropIndex(&m_pTable->GetTableID(), pIndexID);
	ReleaseDBID(pIndexID);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Commit retain on DropIndex
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCTrans::Variation_7()
{
	TBEGIN
	BOOL				fExists;
	ITransactionLocal	*pITransactionLocal=NULL;
	DBID				*pIndexID = NULL;
	ULONG				nIndex;

	TESTC_PROVIDER(VerifyInterface(*m_pCIIndexDefinition, IID_ITransactionLocal, 
			SESSION_INTERFACE, (IUnknown**)&pITransactionLocal));
	
	// create an index
	TESTC_(SetIndex(&m_pTable->GetTableID(), NULL, &nIndex, &pIndexID), S_OK);

	// start the transaction
	if (!CHECK(m_hr = StartTransaction(pITransactionLocal), S_OK))
	{
		odtLog << "Could not start an isolated local transaction\n";
		TESTB = TEST_SKIPPED;
		goto CLEANUP;
	}

	switch (V_I4(&m_vSupportedTxnDDL))
	{
		case DBPROPVAL_TC_DML:
			//check for XACT_E_XTIONEXISTS
			CHECK(m_hr = m_pCIIndexDefinition->DropIndex(&m_pTable->GetTableID(), pIndexID), XACT_E_XTIONEXISTS);
			break;

		case DBPROPVAL_TC_DDL_IGNORE:
			// the statement is not transacted
		case DBPROPVAL_TC_ALL:
		case DBPROPVAL_TC_DDL_COMMIT:
			// create an index
			CHECK(m_hr = m_pCIIndexDefinition->DropIndex(&m_pTable->GetTableID(), pIndexID), S_OK);
			break;

		case DBPROPVAL_TC_DDL_LOCK:
			// error should be returned
			CHECK(m_hr = m_pCIIndexDefinition->DropIndex(&m_pTable->GetTableID(), pIndexID), S_OK);
			break;
			
		default:
			TESTC(FALSE);
	}

	if (DBPROPVAL_TC_DDL_COMMIT != V_I4(&m_vSupportedTxnDDL))
	{
		// commit retaining
		CHECK(m_hr = pITransactionLocal->Commit(TRUE, 0, 0), S_OK);
		// check a new transaction was created
		CHECK(m_hr = pITransactionLocal->Abort(NULL, FALSE, FALSE), S_OK);
	}
	
	// check index deletion
	CHECK(m_hr = m_pCIIndexDefinition->DoesIndexExist(&m_pTable->GetTableID(), pIndexID, &fExists), S_OK);
	COMPARE(fExists, DBPROPVAL_TC_DML == V_I4(&m_vSupportedTxnDDL));

CLEANUP:
	SAFE_RELEASE(pITransactionLocal);
	// drop the index
	m_hr = m_pCIIndexDefinition->DropIndex(&m_pTable->GetTableID(), pIndexID);
	ReleaseDBID(pIndexID);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc Commit non retain on DropIndex
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCTrans::Variation_8()
{ 
	TBEGIN
	BOOL				fExists;
	ITransactionLocal	*pITransactionLocal=NULL;
	DBID				*pIndexID = NULL;
	ULONG				nIndex;

	TESTC_PROVIDER(VerifyInterface(*m_pCIIndexDefinition, IID_ITransactionLocal, 
			SESSION_INTERFACE, (IUnknown**)&pITransactionLocal));
		
	// create an index
	TESTC_(SetIndex(&m_pTable->GetTableID(), NULL, &nIndex, &pIndexID), S_OK);

	// start the transaction
	if (!CHECK(m_hr = StartTransaction(pITransactionLocal), S_OK))
	{
		odtLog << "Could not start an isolated local transaction\n";
		TESTB = TEST_SKIPPED;
		goto CLEANUP;
	}

	switch (V_I4(&m_vSupportedTxnDDL))
	{
		case DBPROPVAL_TC_DML:
			//check for XACT_E_XTIONEXISTS
			CHECK(m_hr = m_pCIIndexDefinition->DropIndex(&m_pTable->GetTableID(), pIndexID), XACT_E_XTIONEXISTS);
			break;

		case DBPROPVAL_TC_DDL_IGNORE:
			// the statement is not transacted
		case DBPROPVAL_TC_ALL:
		case DBPROPVAL_TC_DDL_COMMIT:
			// create an index
			CHECK(m_hr = m_pCIIndexDefinition->DropIndex(&m_pTable->GetTableID(), pIndexID), S_OK);
			break;

		case DBPROPVAL_TC_DDL_LOCK:
			// error should be returned
			CHECK(m_hr = m_pCIIndexDefinition->DropIndex(&m_pTable->GetTableID(), pIndexID), S_OK);
			break;
			
		default:
			TESTC(FALSE);
	}

	if (DBPROPVAL_TC_DDL_COMMIT != V_I4(&m_vSupportedTxnDDL))
	{
		// commit non retaining
		CHECK(m_hr = pITransactionLocal->Commit(FALSE, 0, 0), S_OK);
		// check a new transaction does not exist 
		CHECK(pITransactionLocal->Abort(NULL, FALSE, FALSE), FAILED(m_hr)? S_OK: XACT_E_NOTRANSACTION);
	}
	
	// check index deletion
	CHECK(m_hr = m_pCIIndexDefinition->DoesIndexExist(&m_pTable->GetTableID(), pIndexID, &fExists), S_OK);
	COMPARE(fExists, DBPROPVAL_TC_DML == V_I4(&m_vSupportedTxnDDL));

CLEANUP:
	// drop the index
	m_hr = m_pCIIndexDefinition->DropIndex(&m_pTable->GetTableID(), pIndexID);
	SAFE_RELEASE(pITransactionLocal);
	ReleaseDBID(pIndexID);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCTrans::Terminate()
{ 
// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIIndexDefinition::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END




// {{ TCW_TC_PROTOTYPE(TCRODataSource)
//*-----------------------------------------------------------------------
//| Test Case:		TCRODataSource - read only data source
//| Created:  	11/21/97
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCRODataSource::Init()
{ 
	BOOL				fResults		= TEST_SKIPPED;
	IDBInitialize		*pIDBInitialize	= NULL;
	IDBProperties		*pIDBProperties	= NULL;
	DBPROPSET			*rgPropSets		= NULL;
	ULONG				cPropSets		= 0; 
	HRESULT				hr				= NOERROR;
	DBPROP				*rgProperties	= NULL;
	// vars for an index
	DBINDEX_COL_ORDER	rgOrder[]={DBINDEX_COL_ORDER_ASC};
	const int			n=1;
	const int			nINIT_MODE = 0;
	DBPROPIDSET			rgPropIDSet[n];
	DBPROPID			rgPropID1[1];
	ULONG				nActualSize		= n;;

	m_pIUnknown				= NULL;
	m_pIUnknown2			= NULL;
	m_pIndexID				= NULL;
	m_pIIndexDefinition2	= NULL;

	// {{ TCW_INIT_BASECLASS_CHECK
	if(!TCIIndexDefinition::Init())
	// }}
		goto CLEANUP;

	if (!SettableProperty(DBPROP_INIT_MODE, DBPROPSET_DBINIT))
	{
		odtLog << "DBPROP_INIT_MODE is not supported or is not a read-write property\n";
		fResults = TEST_SKIPPED;
		goto CLEANUP;
	}

	fResults = TEST_FAIL;
	// create a spare index
	if (!CHECK(m_hr = CNCIndexFromOrdinals(m_pTable, NULL, &m_pIndexID, 
		m_rgColumnDesc, m_cColumnDesc, 1, &m_nIndex, rgOrder), S_OK))
	{
		odtLog << "could not create an index on the read-write data source!\n";
		goto CLEANUP;
	}

	// Setup the arrays needed for init, based on string LTM passed to us
	if(!GetInitProps(&cPropSets, &rgPropSets))
		goto CLEANUP;
	if(!SUCCEEDED(m_hr = GetModInfo()->CreateProvider(NULL, IID_IDBInitialize, (IUnknown**)&pIDBInitialize)))
		goto CLEANUP;

	// Get IDBProperties Pointer
	pIDBProperties = NULL;
	if (!VerifyInterface(pIDBInitialize, IID_IDBProperties, DATASOURCE_INTERFACE, (IUnknown**)&pIDBProperties))
		goto CLEANUP;

	// Set the properties before we Initialize, only if we have Properties to set...
	if (FAILED(m_hr = pIDBProperties->SetProperties(cPropSets, rgPropSets)))
		goto CLEANUP;

	// free the structures
	FreeProperties(&cPropSets, &rgPropSets);
	rgPropSets	= NULL;
	cPropSets	= 0;
	TESTC_(SetProperty(&rgPropSets, &cPropSets, DBPROP_INIT_MODE, VT_I4, (LPVOID)1, DBPROPOPTIONS_REQUIRED, DB_NULLID, 
			DBPROPSET_DBINIT), TRUE);
	// Set the properties before we Initialize, only if we have Properties to set...
	if (FAILED(m_hr = pIDBProperties->SetProperties(cPropSets, rgPropSets)))
	{
		FindProperty(DBPROP_INIT_MODE, DBPROPSET_DBINIT, cPropSets, rgPropSets, &rgProperties);
		if (DBPROPSTATUS_OK != rgProperties->dwStatus)
			odtLog << "Read only property could not be initialized!\n";
		goto CLEANUP1;
	}
	FreeProperties( &cPropSets, &rgPropSets);

	// Initialize
	if(!GCHECK(m_hr = pIDBInitialize->Initialize(), S_OK))
	{
		odtLog << wszInitializeFailed;	
		goto CLEANUP1;
	}

	// check that the read only property was set
	rgPropID1[0] = DBPROP_INIT_MODE;
	rgPropIDSet[nINIT_MODE].cPropertyIDs			= 1;
	rgPropIDSet[nINIT_MODE].rgPropertyIDs			= rgPropID1;
	rgPropIDSet[nINIT_MODE].guidPropertySet			= DBPROPSET_DBINIT;

	CHECK(m_hr = pIDBProperties->GetProperties(nActualSize, rgPropIDSet, &cPropSets, &rgPropSets), S_OK);
	if (	(DBPROPSTATUS_OK == rgPropSets[nINIT_MODE].rgProperties[0].dwStatus) 
		&&	(	(VT_I4 != rgPropSets[nINIT_MODE].rgProperties[0].vValue.vt)
			||	(DB_MODE_READ != V_I4(&rgPropSets[nINIT_MODE].rgProperties[0].vValue))))
	{
		odtLog << "despite ok reports, DBPROP_INIT_MODE is not set to read only\n";
		goto CLEANUP1;
	}

	// Obtain IDBCreateSesson, placing the new DSO interface 
	// in CThisTestModule's m_pIUnknown, so that all testcases can use 
	// it via their back pointer to this object.  No need to call AddRef 
	// here as we will own it, rather than the test module.	 
	if (!VerifyInterface(pIDBInitialize, IID_IDBCreateSession, DATASOURCE_INTERFACE, 
			(IUnknown**)&m_pIUnknown))
		goto CLEANUP1;		

	// Create a DB session object
	// Set the m_pIUnknown2 to IOpenRowset
	if (!GCHECK(((IDBCreateSession*)m_pIUnknown)->CreateSession(
			NULL, IID_IOpenRowset, (IUnknown **)&m_pIUnknown2), S_OK))	
		goto CLEANUP1;		

	SetDBSession(m_pIUnknown2);	// calls ReleaseDBSession Before
	if (!(m_hr=VerifyInterface(m_pIUnknown2, IID_IIndexDefinition, SESSION_INTERFACE, 
		(IUnknown**)&m_pIIndexDefinition2)))
		goto CLEANUP1;
	
	fResults = TRUE;
CLEANUP1:
	FreeProperties( &cPropSets, &rgPropSets);
CLEANUP:
	SAFE_RELEASE(pIDBProperties);
	SAFE_RELEASE(pIDBInitialize);
	return fResults;
} 





// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc try to create an index on a read only datasource
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCRODataSource::Variation_1()
{ 
	DBID				*pIndexID = NULL;
	DBINDEXCOLUMNDESC	rgIndexColumnDesc[1];
	BOOL				fTestResults = TEST_PASS;

	rgIndexColumnDesc[0].pColumnID = &m_rgColumnDesc[m_nIndex-1].dbcid;
	rgIndexColumnDesc[0].eIndexColOrder	= DBINDEX_COL_ORDER_ASC;
	m_hr = m_pIIndexDefinition2->CreateIndex(&m_pTable->GetTableID(), m_pIndexID, 1, rgIndexColumnDesc, 
											0, NULL, &pIndexID);
	if (!CHECK(m_hr, DB_SEC_E_PERMISSIONDENIED))
			fTestResults = TEST_FAIL;

	if (!CHECK(m_hr = m_pIIndexDefinition2->DropIndex(&m_pTable->GetTableID(), m_pIndexID), DB_SEC_E_PERMISSIONDENIED))
		fTestResults = TEST_FAIL;

	ReleaseDBID(pIndexID);
	return fTestResults;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCRODataSource::Terminate()
{ 
// {{ TCW_TERM_BASECLASS_CHECK2
	// destroy the index built on the read-write data source
	if (m_pCIIndexDefinition && m_pIndexID)
		m_hr = m_pCIIndexDefinition->DropIndex(&m_pTable->GetTableID(), m_pIndexID);
	ReleaseDBID(m_pIndexID);
	SetDBSession(m_pThisTestModule->m_pIUnknown2);
	SAFE_RELEASE(m_pIUnknown);
	SAFE_RELEASE(m_pIUnknown2);
	SAFE_RELEASE(m_pIIndexDefinition2);
	return(TCIIndexDefinition::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END


// Hack to defile CoInitializeEx
WINOLEAPI  CoInitializeEx(LPVOID pvReserved, DWORD dwCoInit);

unsigned WINAPI ThreadProc(LPVOID lpvThreadParam)
{
	CoInitializeEx(NULL, 0);
	TCIIndexDefinition	*pObject = ((CInParam*)lpvThreadParam)->pObject;
	pObject->MyThreadProc(((CInParam*)lpvThreadParam)->i);
	CoUninitialize();
	return 0;
} //ThreadProc
