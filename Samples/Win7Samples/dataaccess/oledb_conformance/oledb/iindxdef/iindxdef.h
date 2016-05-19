//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1995-2000 Microsoft Corporation.  
//
// @doc
//
// @module IIndexDefinition.H | IIndexDefinition header file for test modules.
//
//
// @rev 01 | 03-21-95 | Microsoft | Created
// @rev 02 | 09-06-95 | Microsoft | Updated
//

#ifndef _IIndexDefinition_H_
#define _IIndexDefinition_H_

#include "oledb.h" 			// OLE DB Header Files
#include "oledberr.h"

#include "privlib.h"		//include private library, which includes
							//the "transact.h"

// helper class for index creation, deletion, and various verifications
// wraps an interface and offers IIndexDefinition functionality
class CIIndexDefinition
{
	protected:
		IIndexDefinition	*m_pIIndexDefinition;
		IUnknown			*m_pIDSOUnknown;

		//--------------------------------------------------------------------------
		//
		//	@cmember Read the value of a column in the indexe schema rowset
		//
		//	RETURNS FALSE if the value was read and differ than the original one
		//--------------------------------------------------------------------------
		BOOL				GetIndexValueFromFirstRow(
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

		//--------------------------------------------------------------------------
		//
		//	@cmember Read the value of a column in the index schema rowset
		//
		//	if *pfSet is TRUE then there is a coparison value for the value read,
		// otherwise the new value will be used for further comparisons => save
		// it in *pVariable and set *pfSet to TRUE
		//	RETURNS FALSE if the value was read and differ than the original one
		//--------------------------------------------------------------------------
		BOOL				GetIndexValue(
								LPVOID		pVariable,		// [OUT]	value read
								BOOL		*pfSet,			// [OUT]	if the value is set
								DBBINDING	*rgDBBINDING,	// [IN]		binding array
								ULONG		cColumn,		// [IN]		column to be read
								ULONG		ulDBTYPE,		// [IN]		type of property variant
								BYTE		*pData,			// [IN]		pointer to read DATA stru
								WCHAR		*lpwszMesaj		// [IN]		message text for error
							);

		HRESULT				DoesIndexExistInIndexSchemaRowset(	
								DBID		*pTableID,				// @parm [IN]	Table ID										  
								DBID		*pIndexID,				// @parm [IN]	Index ID
								BOOL 		*pfExists,				// @parm [OUT]	TRUE if index exists
								BOOL		fAnyIndex = FALSE		// @parm [IN]   TRUE if search for all indexes
							);


	HRESULT					DoesIndexExistRowsetIndex(	
								DBID		*pTableID,				// @parm [IN]	Table ID										  
								DBID		*IndexID,				// @parm [IN]	Index ID
								BOOL 		*fExists				// @parm [OUT]	TRUE if index exists
							);

	public:
		static BOOL			s_fIRowsetIndex;

							CIIndexDefinition(IIndexDefinition *pIIndexDefinition);
							CIIndexDefinition(IUnknown *pISessionUnknown);
							~CIIndexDefinition() {
								SAFE_RELEASE(m_pIIndexDefinition);
								SAFE_RELEASE(m_pIDSOUnknown);
							}

		// basic methods for index manipulation
		HRESULT				CreateIndex(
								DBID				*pTableID,				// [in]		the ID of the table
								DBID				*pIndexID,				// [in]		the ID of the index
								DBORDINAL			nIndexSize,				// [in]		index size (no of columns)
								DBINDEXCOLUMNDESC	*rgIndexColumnDesc,		// [in]		columns to be part of index 
								ULONG				cPropSets,				// [in]		count of property sets 
								DBPROPSET			*rgPropSets,			// [in]		array of property sets
								DBID				**ppIndexID				// [in/out]	stores output ptr to IndexID
							);
		HRESULT				DropIndex(
							   DBID					*pTableID,				// [in]		the ID of the table
							   DBID					*pIndexID				// [in]		the ID of the index
							);

		// helper methods
		HRESULT				CreateIndexAndCheck(
								DBID				*pTableID,				// [in]		the ID of the table
								DBID				*pIndexID,				// [in]		the ID of the index
								DBORDINAL			nIndexSize,				// [in]		index size (no of columns)
								DBINDEXCOLUMNDESC	*rgIndexColumnDesc,		// [in]		columns to be part of index 
								ULONG				cPropSets,				// [in]		count of property sets 
								DBPROPSET			*rgPropSets,			// [in]		array of property sets
								DBID				**ppIndexID				// [in/out]	stores output ptr to IndexID
							);
		HRESULT				CreateCheckAndDropIndex(
								DBID				*pTableID,				// [in]		the ID of the table
								DBID				*pIndexID,				// [in]		the ID of the index
								DBORDINAL			nIndexSize,				// [in]		index size (no of columns)
								DBINDEXCOLUMNDESC	*rgIndexColumnDesc,		// [in]		columns to be part of index 
								ULONG				cPropSets	= 0,		// [in]		count of property sets 
								DBPROPSET			*rgPropSets	= NULL		// [in]		array of property sets
							);
		// Check whether the index is created and property settings
		BOOL				CheckIndex(
								DBID				*pTableID,				// the index of the table
								DBID				*pIndexID,				// the index to be checked
								DBORDINAL			cIndexColumnDesc,		// how many elements
								DBINDEXCOLUMNDESC	*rgIndexColumnDesc,		// array with index original descriprion
								ULONG				cPropertySets,			// number of property sets
								DBPROPSET			*rgPropertySets			// the array of property sets
							);
		BOOL				CheckIndexUsingIRowsetIndex(
								DBID				*pTableID,				// the index of the table
								DBID				*pIndexID,				// the index to be checked
								DBORDINAL			cIndexColumnDesc,		// how many elements
								DBINDEXCOLUMNDESC	*rgIndexColumnDesc,		// array with index original descriprion
								ULONG				cPropertySets,			// number of property sets
								DBPROPSET			*rgPropertySets			// the array of property sets
							);
		BOOL				CheckIndexUsingIDBSchemaRowset(
								DBID				*pTableID,				// the index of the table
								DBID				*pIndexID,				// the index to be checked
								DBORDINAL			cIndexColumnDesc,		// how many elements
								DBINDEXCOLUMNDESC	*rgIndexColumnDesc,		// array with index original descriprion
								ULONG				cPropertySets,			// number of property sets
								DBPROPSET			*rgPropertySets			// the array of property sets
							);

	//---------------------------------------------------------------------------
	// DoesIndexExist	|
	//
	// If this index is on this table return true. If function runs correctly
	// but doesn't find the table name, function will return S_OK, but *pfExists
	// will be FALSE. If strIndexName is empty, returns E_FAIL.	
	//
	// @mfunc	DoesIndexExist
	// @rdesc HRESULT indicating success or failure
	//  @flag S_OK   | Function ran without problem
	//  @flag E_FAIL    | Function ran with problems
	//
	//---------------------------------------------------------------------------
	HRESULT					DoesIndexExist(	
								DBID		*pTableID,				// @parm [IN]	Table ID										  
								DBID		*pIndexID,				// @parm [IN]	Index ID
								BOOL 		*pfExists,				// @parm [OUT] TRUE if index exists
								BOOL		fAnyIndex = FALSE		// @parm [IN]   TRUE if search for all indexes
							);

							operator IIndexDefinition* (){
								return m_pIIndexDefinition;
	}
}; // CIIndexDefinition


BOOL CIIndexDefinition::s_fIRowsetIndex = FALSE;


class CDBID: DBID{
	public:
		CDBID() {
			memset(this, 0, sizeof(DBID));
		}
		~CDBID() {
			ReleaseDBID(this, FALSE);
		}
}; //CDBID

class CDBIDPtr
{
	protected:
		DBID	*m_pDBID;

	public:
		CDBIDPtr(DBID *pDBID = NULL) {
			if (pDBID)
				m_pDBID = pDBID;
			else
				SAFE_ALLOC(m_pDBID, DBID, 1);
			CLEANUP:
			return;
		}
		~CDBIDPtr() {
			ReleaseDBID(m_pDBID, TRUE);
		}
		operator DBID* () {
			return m_pDBID;
		}
		operator DBID** () {
			return &m_pDBID;
		}
};

#endif 	//_IIndexDefinition_H_
