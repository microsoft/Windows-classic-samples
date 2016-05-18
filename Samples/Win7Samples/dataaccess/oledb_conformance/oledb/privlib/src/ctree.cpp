//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1998-2000 Microsoft Corporation.  
//
// @doc 
//
// @module CTree Implementation Module | 	This module contains definition information
//								for the CTree class
//
// @comm
// Special Notes...:	(OPTIONAL NOTES FOR SPECIAL CIRCUMSTANCES)
//
// <nl><nl>
// Revision History:<nl>
//---------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////
// Includes
//
/////////////////////////////////////////////////////////////////////////////
#include "privstd.h"
#include "privcnst.h"
#include "miscfunc.h"
#include "coledb.hpp"


/////////////////////////////////////////////////////////////////////////////
// CSchema
//
/////////////////////////////////////////////////////////////////////////////
CSchema::CSchema
(
	CSchema	*	pParentSchema,		// [in] the parent schema (if exists)
	WCHAR	*	pwszParentRowURL,	// [in] the URL of the parent row
	LONG_PTR	lSeed,				// [in] seed value
	ENULL		eNull,				// [in] tells if nulls are used
	ECREATE_PK	eCreatePK			// [in] tells if a primary key be created
)
{	
	POSITION	pos;
	CSchema *	pSchema = NULL;

	m_ulIndex = 0;
	m_ulPrimaryKey = 0;

	m_fIsCollection	= TRUE;
	m_cLeaves		= 0;
	m_eNull			= eNull;
	m_eCreatePK		= eCreatePK;

	// no selection yet
	m_lCacheSelection = -1;

	m_pwszRowURL	= wcsDuplicate(pwszParentRowURL);
	m_pwszRowName	= m_pwszRowURL? wcsrchr(m_pwszRowURL, L'/')+1 : NULL;

	m_lSeed			= lSeed;
	m_cVariantSubTypes		= 0;
	memset(&m_rgVariantSubTypes, 0, sizeof(m_rgVariantSubTypes));


	m_pParentSchema	= pParentSchema;

	pos = m_ChildrenList.GetHeadPosition();
	for (;pos;)
	{
		pSchema = m_ChildrenList.GetNext(pos);
		SAFE_DELETE(pSchema);
	}
	m_ColList.RemoveAll();
	m_ChildrenList.RemoveAll();
}




/////////////////////////////////////////////////////////////////////////////
// CSchema
//
/////////////////////////////////////////////////////////////////////////////
CSchema::CSchema
(
	CSchema	*	pSchema,		// [in] the schema to be copied
	CSchema	*	pParentSchema,	// [in] the parent schema (if exists)
	BOOL		fNonRecursive	// [in] whether the copying is recursive or not
)
{	
	POSITION					pos;
	CList <CCol,CCol&> 			*pColList = pSchema->GetColList();
	CList <CSchema*, CSchema*>	*pChildrenList = pSchema->GetChildrenList();

	m_ulIndex = pSchema->GetIndexColumn();
	m_ulPrimaryKey = pSchema->GetPrimaryKeyColumn();

	m_fIsCollection			= pSchema->IsCollection();
	m_cLeaves				= pSchema->GetLeafNo();
	m_eNull					= pSchema->GetNull();
	m_eCreatePK				= pSchema->GetCreatePK();

	m_pwszRowURL	= wcsDuplicate(pSchema->GetRowURL());
	m_pwszRowName	= m_pwszRowURL? wcsrchr(m_pwszRowURL, L'/')+1 : NULL;
	m_lSeed			= pSchema->GetSeed();
	m_cVariantSubTypes		= 0;
	memset(&m_rgVariantSubTypes, 0, sizeof(m_rgVariantSubTypes));


	// copy the list of columns (representing the schema of the
	// rowset bound using this URL)
	m_ColList.RemoveAll();
	pos = pColList->GetHeadPosition();
	for (;pos;)
	{
		CCol &col= pColList->GetNext(pos);
		m_ColList.AddTail(col);
	}

	if (fNonRecursive)
		return;

	// remember the parent schema
	m_pParentSchema	= pParentSchema;

	// copy the list of children
	m_ChildrenList.RemoveAll();
	pos = pChildrenList->GetHeadPosition();
	for (;pos;)
	{
		CSchema *pChildSchema = new CSchema(pChildrenList->GetNext(pos), this);
		m_ChildrenList.AddTail(pChildSchema);
	}
} 




/////////////////////////////////////////////////////////////////////////////
// ~CSchema
//
/////////////////////////////////////////////////////////////////////////////
CSchema::~CSchema()
{
	POSITION	pos; 
	POSITION	oldPos;
	CSchema	*	pSchema;

	SAFE_FREE(m_pwszRowURL);
	m_ColList.RemoveAll();
	pos = m_ChildrenList.GetHeadPosition();
	for (;pos;)
	{
		oldPos = pos;
		pSchema = m_ChildrenList.GetNext(pos);
		m_ChildrenList.RemoveAt(oldPos);
		SAFE_DELETE(pSchema);
	}
	m_ChildrenList.RemoveAll();
}




/////////////////////////////////////////////////////////////////////////////
// CSchema::Init(IUnknown*)
//
/////////////////////////////////////////////////////////////////////////////
HRESULT CSchema::InitSchema(IUnknown* pIUnknown)
{
	HRESULT 			hr				= E_FAIL;
	DBORDINAL			cColsColRowset	= 0;	// count of columns
	DBORDINAL			cColsColInfo	= 0;	// count of column objects in GetColumnsInfo 
	DBROWCOUNT			cRowsAffected	= 0;	// Count of rowsets
	EINTERFACE			eInterface		= UNKNOWN_INTERFACE;
	IColumnsInfo *		pIColumnsInfo	= NULL;
	IColumnsRowset *	pIColumnsRowset	= NULL;
	
	if (NULL == pIUnknown)
		return DB_E_NOTCOLLECTION;

	if(VerifyInterface(pIUnknown, IID_IRow, ROW_INTERFACE))
		eInterface = ROW_INTERFACE;
	else if(VerifyInterface(pIUnknown, IID_IRowset, ROWSET_INTERFACE))
	{
		eInterface = ROWSET_INTERFACE;
		m_fIsCollection = TRUE;
	}
	else
		return E_NOINTERFACE;

	if(!VerifyInterface(pIUnknown, IID_IColumnsInfo, eInterface, (IUnknown**)&pIColumnsInfo))
	{
		hr = E_NOINTERFACE;
		goto CLEANUP;
	}
		
	//GetColumnInfo
	TESTC_(hr=SetFromColumnsInfo(pIColumnsInfo, &cColsColInfo),S_OK);

	if(VerifyInterface(pIColumnsInfo, IID_IColumnsRowset, eInterface, (IUnknown**)&pIColumnsRowset))
	{
		TESTC_(hr = SetFromColumnsRowset(pIColumnsRowset, &cColsColRowset),S_OK);	
		TESTC(cColsColInfo == cColsColRowset)
	}

	hr = S_OK;

CLEANUP:

	SAFE_RELEASE(pIColumnsInfo);
	SAFE_RELEASE(pIColumnsRowset);

	return hr;
}




/////////////////////////////////////////////////////////////////////////////
// CSchema::SetFromColumnsInfo
//
/////////////////////////////////////////////////////////////////////////////
HRESULT CSchema::SetFromColumnsInfo(IColumnsInfo* pIColumnsInfo, DBORDINAL* pcColsColInfo)
{
	HRESULT 		hr				= E_FAIL;
	DBORDINAL		index			= 0;
	DBORDINAL		cDBCOLUMNINFO	= 0;
	DBCOLUMNINFO *	rgDBCOLUMNINFO	= NULL;
	WCHAR *			pStringsBuffer	= NULL;
	CCol			col;

	TESTC_(hr = pIColumnsInfo->GetColumnInfo(&cDBCOLUMNINFO,&rgDBCOLUMNINFO,&pStringsBuffer),S_OK);

	// you wouldn't like to add to the old list, right?
	m_ColList.RemoveAll();
	
	for(index=0;index<cDBCOLUMNINFO;index++)
	{
		if (rgDBCOLUMNINFO[index].iOrdinal!=0)
		{
			//Set the ColumnInfo into this CCol struct
			col.SetColInfo(&rgDBCOLUMNINFO[index]);

			m_ColList.AddTail(col);
			if (pcColsColInfo)
				(*pcColsColInfo)++;
		}
	}

	hr = S_OK;

CLEANUP:
	PROVIDER_FREE(rgDBCOLUMNINFO);
	PROVIDER_FREE(pStringsBuffer);

	return hr;
}




//---------------------------------------------------------------------------
//	CSchema::SetFromColumnsRowset
//
//	mfunc	HRESULT								|
//			CSchema
//			SetFromColumnsRowset|
//			Puts column information in m_ColList.
//  Grab column information using IColumnsRowset::GetColumnsRowset.	If
//  no columns found
//  return E_FAIL. If pIRowset == NULL, E_FAIL.
//
//  Mapping of CCol private member variable to DBCOLUMNINFO struct members:
//  -----------------------------------------------
//
//	m_fAutoInc			->	DBCOLUMN_ISAUTOINCREMENT
//	m_fSearchable		->	DBCOLUMN_ISSEARCHABLE
// 
// 	rdesc HRESULT indicating success or failure
// 	
// 	flag S_OK   | Function ran without problem
// 	flag E_FAIL    | Function ran with problems
//
//---------------------------------------------------------------------------
HRESULT CSchema::SetFromColumnsRowset
(
	IColumnsRowset* pIColumnsRowset,	// @parm ||[IN]	 IColumnsRowset pointer
	DBORDINAL *		 cColsFound			// @parm ||[OUT] Count of columns found
)
{
	HRESULT 			hr				= E_FAIL;
	BOOL 				fResult			= FALSE;
	POSITION 			pos;						// position in m_ColList
	CCol 				col;						// column in m_ColList
	HACCESSOR 			hAccessor;					// accessor
	DBCOUNTITEM			cRowsObtained	= 0;		// number of rows returned, should be 1
	DBLENGTH			ulRowSize		= 0;		// size of row of data
	HROW *				rghRows			= NULL;		// array of handles of rows
	DATA *				pColumn			= NULL;
	BYTE *				pRow			= NULL;		// pointter to data
	IRowset *			pColRowset		= NULL;		// Rowset interface pointer
	DBCOUNTITEM			iDBBINDING		= 0;		// index of rgDBBINDING
	DBCOUNTITEM			cDBBINDING		= 0;		// count of bindings
	DBBINDING *			rgDBBINDING		= NULL;		// pointer to array of bindings
	DBORDINAL			cDBCOLUMNINFO	= 0;		// count of column info
	DBCOLUMNINFO *		rgDBCOLUMNINFO	= NULL;		// pointer to array of columninfos
	WCHAR *				rgStringsBuffer	= NULL;		// corresponding strings
	DBORDINAL			cOptionalColumns= 0;		// count of optional columns
	DBID *				rgOptionalColumns=NULL;		// array of optional columns
	DBCOUNTITEM			iRow			= 0;		// row index
	DBCOUNTITEM			ulTotalRows		= 0;
	BOOL				fFound = FALSE;

	*cColsFound = 0;

	if (!CHECK(hr = pIColumnsRowset->GetAvailableColumns(&cOptionalColumns,&rgOptionalColumns),S_OK))
		goto CLEANUP;

	if (!CHECK(hr = pIColumnsRowset->GetColumnsRowset(NULL,cOptionalColumns,
		rgOptionalColumns,IID_IRowset,0,NULL,(IUnknown **) &pColRowset),S_OK))
			goto CLEANUP;

	if (!CHECK(hr = GetAccessorAndBindings(
		pColRowset,			// IN: 	Rowset to get info from
		DBACCESSOR_ROWDATA,
		&hAccessor,			// OUT:	accessor from create accessor
		&rgDBBINDING,		// OUT:	Array of DBBINDINGs
		&cDBBINDING,
		&ulRowSize,			// OUT: offset from DBBINDING struct
		DBPART_VALUE |DBPART_STATUS |DBPART_LENGTH,
		ALL_COLS_BOUND,
		FORWARD,
		NO_COLS_BY_REF,
		&rgDBCOLUMNINFO,	// OUT: Array of DBCOLUMNINFOs
		&cDBCOLUMNINFO,		// OUT: Count of DBCOULMNINFOs
		&rgStringsBuffer, 
		DBTYPE_EMPTY, 
		0, 						//cColsToBind
		NULL,					//rgColsToBind
		NULL,					//rgColOrdering
		NO_COLS_OWNED_BY_PROV,	//eColsMemProvOwned
		DBPARAMIO_NOTPARAM,		//eParamIO
		BLOB_LONG),S_OK))		//dwBlobType
		goto CLEANUP;

	pRow = new	BYTE[(size_t)ulRowSize];	//data

	while(!FAILED(hr=pColRowset->GetNextRows
	(
		0,					// no chapters
		0,					// don't skip any rows
		10,					// total number of rows requesting
		&cRowsObtained,		// number of rows returned, this will be the number of
							//  columns in the table if there are 10 or less
		&rghRows			// array of handles of rows
	)) && cRowsObtained !=0)
	{ 
		// Check the HResult an number of rows
		if(hr == S_OK)
			COMPARE(cRowsObtained, 10);
		else if (hr == DB_S_ENDOFROWSET)
			COMPARE(1, (cRowsObtained < 10));
		else
			goto CLEANUP;

		// Get data for each row
		for(iRow=0;iRow<cRowsObtained;iRow++)			 
		{
			// Get data for a row
			CHECK(hr=pColRowset->GetData(		 
				rghRows[iRow],		// hrow
				hAccessor,	  		// handle of accessor to use
				pRow				// actual row of data returned 
			),S_OK);

			if (FAILED(hr))
				goto CLEANUP;

			pos = m_ColList.GetHeadPosition();

			// for each column in our CList
			while(pos)
			{
				CCol& rCol = m_ColList.GetNext(pos);

  				if (rgDBBINDING[0].iOrdinal == 0)
					pColumn = (DATA *) (pRow + rgDBBINDING[5].obStatus);
				else
					pColumn = (DATA *) (pRow + rgDBBINDING[4].obStatus);

				if (pColumn->sStatus==DBSTATUS_S_OK)
				{
					// Match this rows DBCOLUMN_NUMBER
					if((*(unsigned int*)pColumn->bValue) != rCol.GetColNum())
						continue;
				}
				else
					continue;

				// Get Data for each column
				fFound = TRUE;
				for(iDBBINDING=0; iDBBINDING<cDBBINDING; iDBBINDING++)
				{
					if(rgDBCOLUMNINFO[iDBBINDING].iOrdinal ==0)
						continue;

					//Grab column
					pColumn = (DATA *) (pRow + rgDBBINDING[iDBBINDING].obStatus);
				
					//All Columns Must be valid 
					if(pColumn->sStatus != DBSTATUS_S_OK && pColumn->sStatus != DBSTATUS_S_ISNULL)
						fFound = FALSE;
					
					if(pColumn->sStatus==DBSTATUS_S_OK)
					{
						//DBCOLUMN_ISAUTOINCREMENT (Optional Column)
						if (0==memcmp(&(rgDBCOLUMNINFO[iDBBINDING].columnid),&(DBCOLUMN_ISAUTOINCREMENT),sizeof(DBID)))
						{
							if (*(VARIANT_BOOL*) pColumn->bValue==0)
								rCol.SetAutoInc(FALSE);
							else
								rCol.SetAutoInc(TRUE);
						}
						
						//DBCOLUMN_ISSEARCHABLE (Optional Column)
						if (0==memcmp(&(rgDBCOLUMNINFO[iDBBINDING].columnid),&(DBCOLUMN_ISSEARCHABLE),sizeof(DBID)))
						{
							rCol.SetSearchable(*(unsigned int *) pColumn->bValue);
						}
					}
				}
				
				if(fFound)
				{
					(*cColsFound)++;					

					fFound = FALSE;
					goto NEXTROW;
				}
			} 
NEXTROW: 		
			ulTotalRows++;
		}

		// Need to release rows
		CHECK(hr=pColRowset->ReleaseRows
		(		 
			cRowsObtained,	// number of rows to release
			rghRows,	  	// array of row handles
			NULL,
			NULL,			// count of rows successfully released
			NULL			// there shouldn't be anymore references to these rows
		),S_OK);

		if(FAILED(hr))
			goto CLEANUP;
	}

	fResult	= TRUE;

CLEANUP:
	
	SAFE_DELETE(pRow);
	PROVIDER_FREE(rgOptionalColumns);
	SAFE_RELEASE(pColRowset);

	FreeAccessorBindings(cDBBINDING, rgDBBINDING);
	PROVIDER_FREE(rgDBCOLUMNINFO);
	PROVIDER_FREE(rgStringsBuffer);
	PROVIDER_FREE(rghRows);

	if (fResult == FALSE)
		return hr;

	return S_OK;
}




//---------------------------------------------------------------------------
// CSchema::GetColInfo	
//	 
// CSchema				|
// GetColInfo			|
// Returns CCOL information based function criteria
//	
// @mfunc	GetColInfo
//	
// @rdesc HRESULT indicating success or failure.
//  @flag S_OK   | Function ran without problem
//  @flag E_FAIL    | Column was not found
//	
//---------------------------------------------------------------------------
HRESULT	CSchema::GetColInfo
(
	CColApplicator	pfApp, 	// [IN]  Accepting criteria
	CCol & 	ColInfo			// [OUT] CCol object 
)
{
	if (!pfApp)
		return E_FAIL;

	// While not at end of list and column hasn't been found
	POSITION pos = m_ColList.GetHeadPosition();
	while (pos!=NULL) 
	{
		// Get object in list
		CCol& rCol = m_ColList.GetNext(pos);

		// If there is a match
		if((*pfApp)(&rCol))
		{
			ColInfo = rCol;
			return S_OK;
		}
	}
	
	// Return error if object was never found
	return E_FAIL;
}

//---------------------------------------------------------------------------
// CSchema::GetColInfo	
//	 
// CSchema				|
// GetColInfo			|
// Returns CCOL information based on column number.
//	
// @mfunc	GetColInfo
//	
// @rdesc HRESULT indicating success or failure.
//  @flag S_OK   | Function ran without problem
//  @flag E_FAIL    | Column was not found
//	
//---------------------------------------------------------------------------
HRESULT	CSchema::GetColInfo
(
	DBORDINAL	iOrdinal,	//@parm [IN]  column number searching for, 1 based
	CCol & 	col				// @parm [OUT] returned CCOL 
)
{
	//We don't save the bookmark internally
	if(iOrdinal == 0)
		return E_FAIL;

	// While not at end of list and column hasn't been found
	POSITION pos = m_ColList.GetHeadPosition();
	while (pos!=NULL) 
	{
		// Get object in list
		CCol& rCol = m_ColList.GetNext(pos);

		// If there is a match
		if(iOrdinal == rCol.GetColNum())
		{
			col = rCol;
			return S_OK;
		}
	}
	
	// Return error if object was never found
	return E_FAIL;
} 




//---------------------------------------------------------------------------
// CSchema::GetColInfo
//
// CSchema			|
// GetColInfo		|
// Returns CCOL information for first column of type DBDATATYPELIST.
//
// @mfunc	GetColInfo
//
// @rdesc HRESULT indicating success or failure
//  @flag S_OK   | Function ran without problem
//  @flag E_FAIL    | Column was not found
//
//---------------------------------------------------------------------------
HRESULT	CSchema::GetColInfo
(
	CCol &		col,	// @parm [OUT] Returned column
	DBTYPE		eType	// @parm [IN]  Data type searching for 
)
{
	// Get Matching Column
	// Get top of list
	POSITION pos = m_ColList.GetHeadPosition();
	while(pos)
	{
		// Get object in list
		CCol& rCol = m_ColList.GetNext(pos);

		// Compare item in object with param eType
		if(eType == rCol.GetProviderType())
		{
			// Return found object
			col = rCol;
			return S_OK;
		}
	}
	
	return E_FAIL;
}




//---------------------------------------------------------------------------
// CSchema::GetColInfo
//
// CSchema			|
// GetColInfo		|
// Returns CCOL information for first column of type DBDATATYPELIST.
//
// @mfunc	GetColInfo
//
// @rdesc HRESULT indicating success or failure
//  @flag S_OK   | Function ran without problem
//  @flag E_FAIL    | Column was not found
//
//---------------------------------------------------------------------------
HRESULT	CSchema::GetColInfo
(
	CCol &	col,			// @parm [OUT] Returned column
	WCHAR *	pwszTypeName	// @parm [IN]  Provider type name to search for
)
{
	// Get Matching Column
	POSITION pos = m_ColList.GetHeadPosition();
	while (pos)
	{
		// Get object in list
		CCol& rCol = m_ColList.GetNext(pos);

		// Compare item in object with param eType
		if (	pwszTypeName && rCol.GetProviderTypeName()
			&&	0 == wcscmp(pwszTypeName, rCol.GetProviderTypeName()))
		{
			// Return found object
			col = rCol;
			return S_OK;
		}
	}
	
	return E_FAIL;
} 



	
//---------------------------------------------------------------------------
// CSchema::GetColInfo	
//	 
// CSchema				|
// GetColInfo			|
// Returns CCOL information based on column number.
//	
// @mfunc	GetColInfo
//	
// @rdesc HRESULT indicating success or failure.
//  @flag S_OK   | Function ran without problem
//  @flag E_FAIL    | Column was not found
//	
//---------------------------------------------------------------------------
HRESULT	CSchema::GetColInfo
(
	DBID *	pdbcid,		// @parm [IN]  column ID searching for
	CCol & 	col			// @parm [OUT] returned CCOL 
)
{	
	// Check Params
	ASSERT(0 != pdbcid);

	// Get Matching Column
	// Get top to list
	POSITION pos = m_ColList.GetHeadPosition();
	while (pos) 
	{
		// Get object in list & advance pos
		CCol& rCol =  m_ColList.GetNext(pos);

		// If there is a match
		if (CompareDBID(*pdbcid, *(rCol.GetColID())))
		{
			// Return found object
			col = rCol;
			return S_OK;
		}
	}
	
	// Return error if object was never found
	return E_FAIL;
} 




//---------------------------------------------------------------------------
// CSchema::GetColInfoForUpdate	
//	 
// CSchema				|
// GetColInfo			|
// Returns CCOL information based on column number.
//	
// @mfunc	GetColInfoForUpdate
//	
//  Return CCol on success, otherwise return NULL.
//	
//---------------------------------------------------------------------------
CCol &	 CSchema::GetColInfoForUpdate
(
	DBORDINAL 	iOrdinal 		// @parm [IN]  column number searching for, 1 based
)
{
	// Check Params
	ASSERT(iOrdinal!=0);

	// While not at end of list and column hasn't been found
	POSITION pos = m_ColList.GetHeadPosition();
	while(pos) 
	{
		// Get object in list
		CCol& rCol = m_ColList.GetNext(pos);

		// If there is a match
		if(iOrdinal == rCol.GetColNum())
			return rCol;
	}
	
	return m_ColList.GetHead();
} 




//---------------------------------------------------------------------------
// CSchema::MakeData 
//
// CSchema			|
// MakeData		|
// Generates consistent data of the appropriate type, and returns
// a pointer to that data.
//
// @mfunc	MakeData
// @rdesc HRESULT indicating success or failure
//  @flag S_OK   | Function succeeded
//  @flag E_FAIL    | Function failed
//
//---------------------------------------------------------------------------
HRESULT CSchema::MakeData
(
	WCHAR * 	wszData,		// @parm [OUT] Return data
	DBCOUNTITEM	ulRowNum,		// @parm [IN]  Row number which is one based
	DBORDINAL	iOrdinal,		// @parm [IN]  Column number which is one based
	EVALUE 		eValue,			// @parm [IN]  Primary or secondary data 
	DBTYPE		wSubType,		// @parm [IN]  Column SubType
	BOOL		bNoNullValues,	// [IN] if bNoNullValues is True then MakeData will return valid data.
	DBTYPE *	pwVariantType
)
{
	CCol	ColInfo;
	ENULL	eNull	= bNoNullValues ? NONULLS : m_eNull;
	HRESULT hr		= E_FAIL;

	//TODO remove
	//This function has been more appropiatly placed on the CCol object.  But since numerous 
	//privlib and test modules assume the call is on the CTable object, we will delegate until
	//all calling sites are updated...

	//Specically all calling sites that are trying to use Row Objects, will fail since the 
	//extra columns are not part of the base table.  Update the calling site to use CCol.MakeData
	//instead of CTable.MakeData...

	// For hierarchies, we would like to generate unique value for each node in the hierarchy
	// even if the schema is the same.
	// Re-interpret "ulRowNum" in the context of hierarchy depth
	// We can guarantee unique values for all nodes
	// at the time the tree was created.
	
	//if we were passed in a file name, we  have the data in the ini file
	//The INI file is only able to obtain data for existing rows.
	//MakeData maybe called to generate data for new rows (FillInputBindings)
	if(GetModInfo()->GetFileName() && ulRowNum <= GetModInfo()->GetParseObject()->GetRowCount())
	{
		//we have the col info already for the ini file.  lets use it
		// Get the beginning of the CCol list
		POSITION pos = m_ColList.GetHeadPosition(); 
		while(pos)
		{
			CCol& rCol = m_ColList.GetNext(pos);
		
			if (rCol.GetColNum()==iOrdinal)
			{
				hr = rCol.MakeData(wszData, ulRowNum, eValue, eNull, &wSubType, m_ulIndex);
			}
		}
		//if the column if from row data it might not be in the table so the 
		//m_ColList will have no knowledge of it.
		//create it just from the data from the ini file.
		if (E_FAIL==hr)
		{
			ColInfo.SetColNum(iOrdinal);
			hr = ColInfo.MakeData(wszData, ulRowNum, eValue, eNull, &wSubType, m_ulIndex);
		}
	}
	else
	{
		//Find the indicated Column
		//If this fails, use CCol.MakeData!
		TESTC_(hr = GetColInfo(iOrdinal, ColInfo),S_OK);
		hr = ColInfo.MakeData(wszData, ulRowNum, eValue, eNull, &wSubType, m_ulIndex);
	}

CLEANUP:

	if(pwVariantType)
		*pwVariantType = wSubType;
	return hr;
}




/////////////////////////////////////////////////////////////////////////////
// CSchema::SetRowURL
//
/////////////////////////////////////////////////////////////////////////////
WCHAR *CSchema::SetRowURL(WCHAR *pwszRowURL) 
{
	SAFE_FREE(m_pwszRowURL);
	m_pwszRowURL = wcsDuplicate(pwszRowURL);
	m_pwszRowName	= m_pwszRowURL? wcsrchr(m_pwszRowURL, L'/')+1 : NULL;
	return m_pwszRowURL;
} 




/////////////////////////////////////////////////////////////////////////////
// CSchema::AddChild
//
/////////////////////////////////////////////////////////////////////////////
void CSchema::AddChild(CSchema *pSchema)
{
	// if this row didn't have children m_cLeaves was 1; fix it
	if (0 == m_ChildrenList.GetCount())
		m_cLeaves = 0;

	// set the number of leaves in this subtree
	// and add the child in the list
	m_cLeaves += pSchema->GetLeafNo();	
	m_ChildrenList.AddTail(pSchema);

	// update the child's parent data member
	pSchema->SetParentSchema(this);
}




/////////////////////////////////////////////////////////////////////////////
// CSchema::RemoveChild
//
// Removes the schema from the parent's children list but does not delete the schema
/////////////////////////////////////////////////////////////////////////////
void CSchema::RemoveChild(CSchema *pSchema)
{
	POSITION	pos;
	POSITION	posOld;
	CSchema	*	pParentSchema	= this;
	BOOL		fFound			= FALSE;

	ASSERT(this == pSchema->GetParentSchema());
	pos = m_ChildrenList.GetHeadPosition();
	for (;pos;)
	{
		posOld = pos;
		if (m_ChildrenList.GetNext(pos) == pSchema)
		{
			m_ChildrenList.RemoveAt(posOld);
			fFound  = TRUE;
			break;
		}
	}
	if (!fFound)
		return;

	// update the parent's data member
	// set the number of leaves in this subtree
	m_cLeaves = max(m_cLeaves - pSchema->GetLeafNo(), 1);
	for (pParentSchema = m_pParentSchema; pParentSchema; pParentSchema = pParentSchema->GetParentSchema())
	{
		pParentSchema->SetLeafNo(
			max(1, pParentSchema->GetLeafNo() - pSchema->GetLeafNo()));
	}
}




/////////////////////////////////////////////////////////////////////////////
// CSchema::GetChild
//
/////////////////////////////////////////////////////////////////////////////
CSchema *CSchema::GetChild
(
	DBCOUNTITEM ulIndex
)
{
	POSITION	pos;
	CSchema	*	pSchema = NULL;

	if (m_ChildrenList.GetCount() <= ulIndex)
		return NULL;

	pos = m_ChildrenList.GetHeadPosition();
	for (;pos; --ulIndex)
	{
		pSchema = m_ChildrenList.GetNext(pos);
		if (0 == ulIndex)
			break;
	}
	return pSchema;
}




/////////////////////////////////////////////////////////////////////////////
// CSchema::GetChild
//
/////////////////////////////////////////////////////////////////////////////
CSchema *CSchema::GetChild
(
	WCHAR *			pwszName,
	DBCOUNTITEM *	pulIndex
)
{
	POSITION	pos;
	CSchema	*	pSchema	= NULL;
	DBCOUNTITEM	ulIndex	= 0;

	if (NULL == pwszName)
		return NULL;

	pos = m_ChildrenList.GetHeadPosition();
	for (;pos; ulIndex++)
	{
		pSchema = m_ChildrenList.GetNext(pos);
		if (0 == wcscmp(pSchema->GetRowName(), pwszName))
		{
			if (pulIndex)
				*pulIndex = ulIndex;
			return pSchema;
		}
	}
	return NULL;
}




/////////////////////////////////////////////////////////////////////////////
// CSchema::UpdateURL
//
/////////////////////////////////////////////////////////////////////////////
void CSchema::UpdateURL
(
	WCHAR *	pwszURL
)
{
	WCHAR *		pwszNewURL = NULL;
	CSchema	*	pSchema	= NULL;
	POSITION	pos;

	if (!pwszURL)
		return;

	// allocate space for the new URL
	SAFE_ALLOC(pwszNewURL, WCHAR, wcslen(pwszURL)+wcslen(m_pwszRowName)+2);

	// copy the new parent
	// and append the relative name to the parent
	wcscpy(pwszNewURL, pwszURL);
	wcscat(pwszNewURL, --m_pwszRowName);
	
	// release previous URL and store the new one
	SAFE_FREE(m_pwszRowURL);
	m_pwszRowURL	= pwszNewURL;
	m_pwszRowName	= m_pwszRowURL + wcslen(pwszURL) + 1;

	pos = m_ChildrenList.GetHeadPosition();
	for (;pos;)
	{
		pSchema = m_ChildrenList.GetNext(pos);
		pSchema->UpdateURL(m_pwszRowURL);
	}

CLEANUP:

	return;
}




/////////////////////////////////////////////////////////////////////////////
// CSchema::GetChildIndex
//
// returns the index of the child given in the children list
/////////////////////////////////////////////////////////////////////////////
LONG_PTR CSchema::GetChildIndex
(
	CSchema *	pSchema
)
{
	POSITION	pos;
	CSchema	*	pCrtSchema	= NULL;
	LONG_PTR	lIndex		= 0;

	pos = m_ChildrenList.GetHeadPosition();
	for (;pos; lIndex++)
	{
		pCrtSchema = m_ChildrenList.GetNext(pos);
		if (pCrtSchema == pSchema)
			break;
	}
	return (ULONG_PTR)lIndex < m_ChildrenList.GetCount()? lIndex: -1L;
}




//---------------------------------------------------------------------------
// CSchema::UpdateCCol 
//
// Schema			|
// UpdateCCol		|
// Updates the elements in the CCol list so it can be added to CCol
//
// @mfunc	UpdateCCol
//
//---------------------------------------------------------------------------
HRESULT CSchema::UpdateCCol
(
	DBCOUNTITEM		cBindings,	
	DBBINDING *		rgBindings,
	void *			pData,
	CCol &			rCol,			// @parm [IN/OUT] Element from the CCol List
	EDATATYPES		eDataTypes,		// @parm [IN] Enum for column data type
	ULONG *			pulAutoIncPrec,	// @parm [IN/OUT] Precision of largest auto increment column
	LONG_PTR		lSQLSupport		// @parm [IN] SQL Support Level
)
{
	WCHAR * pwszTypeName	= (WCHAR*)&VALUE_BINDING(rgBindings[0], pData);
	DBTYPE wType			= (DBTYPE)VALUE_BINDING(rgBindings[1], pData);
	// Provider Type's schema rowset is explicitly an UI4
	ULONG ulColumnSize	= *(ULONG *)&VALUE_BINDING(rgBindings[2], pData);
	BYTE bPrecision			= (BYTE)VALUE_BINDING(rgBindings[2], pData);

	//TYPE_NAME
	if (eDataTypes!=NATIVETYPES)
		rCol.SetProviderTypeName(pwszTypeName);

	//DATA_TYPE
	rCol.SetProviderType(wType);
	
	//COLUMN_SIZE - variable length
	if(IsFixedLength(wType))
	{
		//Fixed Length Data Type - ColumnSize = size of the data type
		rCol.SetColumnSize(GetDBTypeSize(wType));
	}
	else
	{
		//Variable Length Type - ColumnSize - maximium length in characters 
		// Don't update this if the user qualified his/her own columnsize
		if (rCol.GetReqParams() != 1)
			rCol.SetColumnSize(ulColumnSize);
	}


	//COLUMN_SIZE - numerics
	if(IsNumericType(wType) || wType == DBTYPE_DBTIMESTAMP)
	{
		//ColumnSize - for numerics is really the precision
		//Also for TimeStamp its the maximum precision couting fractional parts...
		// Don't update this if the user qualified his/her own precision/scale
		if (rCol.GetReqParams() != 2)
			rCol.SetPrecision(bPrecision);
		
		//Scale should only be looked at for the following
		if(IsScaleType(wType))
		{
			if (rCol.GetReqParams() != 2)
			{
				rCol.SetScale(0);
			
				//For timestamps, precision = total precision including fractional part.
				//Store the fractional part in the Scale, as is done for ColInfo.  
				//yyyy-mm-dd hh:mm:ss.ffff
				if(wType == DBTYPE_DBTIMESTAMP && bPrecision > 20)
					rCol.SetScale(bPrecision - 20);
			}

			//Scale
			if(STATUS_BINDING(rgBindings[13], pData) == DBSTATUS_S_OK)
			{
				SHORT uScale = 4;
				SHORT uMinScale = (SHORT)VALUE_BINDING(rgBindings[13], pData);
				SHORT uMaxScale = (SHORT)VALUE_BINDING(rgBindings[14], pData);
				
				if (rCol.GetReqParams() != 2)
				{
					//If type is numeric or decimal
					if (bPrecision >= ULONG(uScale) && uMaxScale >= uScale && uMinScale <= uScale)
						rCol.SetScale((BYTE)uScale);
					else
						rCol.SetScale((BYTE)uMinScale);			
				}

				//MINIMUM_SCALE
				rCol.SetMinScale(uMinScale);

				//MAXIMUM_SCALE
				rCol.SetMaxScale(uMaxScale);
			}
		}
	}
	
	//UNSIGNED_ATTRIBUTE
	if (STATUS_BINDING(rgBindings[9], pData) == DBSTATUS_S_ISNULL)
		rCol.SetUnsigned(-1);
	else
	{
		if (*(VARIANT_BOOL*)&VALUE_BINDING(rgBindings[9], pData) == VARIANT_FALSE)
			rCol.SetUnsigned(0);
		else
			rCol.SetUnsigned(1);
	}

	//AUTO_UNIQUE_VALUE
	rCol.SetUnique(FALSE);

	// Autoincrementing type
	if ((STATUS_BINDING(rgBindings[11], pData) == DBSTATUS_S_ISNULL) || 
		(*(VARIANT_BOOL*)&VALUE_BINDING(rgBindings[11], pData) == VARIANT_FALSE) )
	{
		rCol.SetAutoInc(FALSE);
		rCol.SetCanAutoInc(FALSE);
	}
	else
	{
		rCol.SetAutoInc(TRUE);
		rCol.SetCanAutoInc(TRUE);

		// Get the auto increment column with the largest precision.
		if (ALLTYPES == eDataTypes)
		{
			if(IsNumericType(wType) && *pulAutoIncPrec < bPrecision)
				*pulAutoIncPrec = bPrecision;
			else
			{
				if(!IsNumericType(wType) && *pulAutoIncPrec < bPrecision)
					*pulAutoIncPrec = bPrecision;
			}
		}
	}

	//IS_NULLABLE
	if (STATUS_BINDING(rgBindings[6], pData) == DBSTATUS_S_ISNULL)
		rCol.SetNullable(-1);
	else
	{
		if (*(VARIANT_BOOL*)&VALUE_BINDING(rgBindings[6], pData) == VARIANT_FALSE)
			rCol.SetNullable(0);
		else
			rCol.SetNullable(1);

	}

	//CASE_SENSITIVE
	if (STATUS_BINDING(rgBindings[7], pData) == DBSTATUS_S_ISNULL)
		rCol.SetCaseSensitive(-1);
	else
	{
		if (*(VARIANT_BOOL*)&VALUE_BINDING(rgBindings[7], pData) == VARIANT_FALSE)
			rCol.SetCaseSensitive(0);
		else
			rCol.SetCaseSensitive(1);
	}

	//SEARCHABLE
	rCol.SetSearchable((ULONG)PtrToUlong(VALUE_BINDING(rgBindings[8], pData)));

	//LITERAL_PREFIX
	if(STATUS_BINDING(rgBindings[3], pData) == DBSTATUS_S_ISNULL)
		rCol.SetPrefix(NULL);
	else if ((WCHAR *)&VALUE_BINDING(rgBindings[3], pData))
		rCol.SetPrefix((WCHAR *)&VALUE_BINDING(rgBindings[3], pData));
	else if((wType == DBTYPE_DBDATE) && (lSQLSupport & DBPROPVAL_SQL_ESCAPECLAUSES))
		rCol.SetPrefix((WCHAR *)(L"{d'"));
	else if ((wType == DBTYPE_DBTIME) && (lSQLSupport & DBPROPVAL_SQL_ESCAPECLAUSES))
		rCol.SetPrefix((WCHAR *)(L"{t'"));
	else if ((wType  == DBTYPE_DBTIMESTAMP) && (lSQLSupport & DBPROPVAL_SQL_ESCAPECLAUSES))
		rCol.SetPrefix((WCHAR *)(L"{ts'"));
	else
		ASSERT(FALSE);

	//LITERAL_SUFFIX
	if (STATUS_BINDING(rgBindings[4], pData) == DBSTATUS_S_ISNULL)
		rCol.SetSuffix(NULL);
	else if ((WCHAR *)&VALUE_BINDING(rgBindings[4], pData))
		rCol.SetSuffix((WCHAR *)&VALUE_BINDING(rgBindings[4], pData));
	else if ((wType == DBTYPE_DBDATE)	  ||
			 (wType == DBTYPE_DBTIME)	  ||
			 (wType == DBTYPE_DBTIMESTAMP) &&
			 (lSQLSupport & DBPROPVAL_SQL_ESCAPECLAUSES))
		rCol.SetSuffix((WCHAR *)(L"'}"));
	else 
		ASSERT(FALSE);

	//CREATE_PARAMS
	if (STATUS_BINDING(rgBindings[5], pData) == DBSTATUS_S_ISNULL)
		rCol.SetCreateParams(NULL);
	else
		rCol.SetCreateParams((WCHAR*)&VALUE_BINDING(rgBindings[5], pData));

	// Use in where section of SQL stmt, set by user
	rCol.SetUseInSQL(TRUE);

	//IS_LONG
	if ((STATUS_BINDING(rgBindings[18], pData) == DBSTATUS_S_ISNULL) || 
		(*(VARIANT_BOOL*)&VALUE_BINDING(rgBindings[18], pData) == VARIANT_FALSE) )
		rCol.SetIsLong(0);
	else
		rCol.SetIsLong(1);

	//IS_FIXEDLENGTH
	if ((STATUS_BINDING(rgBindings[20], pData) == DBSTATUS_S_ISNULL) || 
		(*(VARIANT_BOOL*)&VALUE_BINDING(rgBindings[20], pData) == VARIANT_FALSE) )
		rCol.SetIsFixedLength(0);
	else
		rCol.SetIsFixedLength(1);

	return S_OK;	
}




//---------------------------------------------------------------------------
// CSchema::PopulateTypeInfo
//
// Schema			|
// PopulateTypeInfo		|
// Populates an col list with a Provider's type information
// This method has the property of returning an unadulterdated snapshot
// of the type information
//
// @mfunc	PopulateTypeInfo
//
//---------------------------------------------------------------------------
HRESULT	CSchema::PopulateTypeInfo
(
	IUnknown *	pIUnknown,		// @parm [IN] Session object's IUnknown
	LONG_PTR	lSQLSupport		// @parm [IN] SQL Support level of the provider
)
{
	HRESULT				hr = E_FAIL;
	IDBSchemaRowset *	pSchemaRowset = NULL;
	IRowset *			pIRowset = NULL;
	IGetDataSource *	pIGetDataSource = NULL;
	IDBInitialize *		pIDBInitialize = NULL;
	HACCESSOR			hAccessRead = DB_NULL_HACCESSOR;
	DBBINDING *			rgBindings = NULL;
	DBCOUNTITEM			cBindings = 0;
	void *				pData = NULL;
	HROW *				phRow = DB_NULL_HROW;
	DBCOUNTITEM			cRows = 0;
	DBLENGTH			cbBuffer = 0;
	ULONG				ulAutoIncPrec = 0;
	DBORDINAL			cCols = 0;

	// Get Schema PROVIDER_TYPES Rowset data types
	if(!VerifyInterface(pIUnknown, IID_IDBSchemaRowset, SESSION_INTERFACE, 
			(IUnknown**)&pSchemaRowset))
		return E_NOINTERFACE;

	m_ColList.RemoveAll();	

	//Obtain DataSource 	
	TESTC(VerifyInterface(pIUnknown, IID_IGetDataSource, SESSION_INTERFACE, (IUnknown**)&pIGetDataSource));
	TESTC_(pIGetDataSource->GetDataSource(IID_IDBInitialize, (IUnknown**)&pIDBInitialize), S_OK);

	TESTC_(hr = pSchemaRowset->GetRowset(NULL, DBSCHEMA_PROVIDER_TYPES, 0, NULL, 
			IID_IRowset, 0, NULL, (IUnknown**)&pIRowset), S_OK);

	// Create an Accessor and DBBINDINGs
	//NOTE:  We need to bind BLOB columns, so we are binding all columns
	//from the schema rowset, incase the provider returned IS_LONG 
	TESTC_(hr = GetAccessorAndBindings
					(
						pIRowset,			// IN: 	IRowset interface
						DBACCESSOR_ROWDATA,
						&hAccessRead,		// OUT:	Accessor from CreateAccessor
						&rgBindings,		// OUT:	Array of DBBINDINGs
						&cBindings,			// OUT: Count of columns
						&cbBuffer,			// OUT: Length of the DBBINDINGs
  						DBPART_VALUE | DBPART_STATUS | DBPART_LENGTH,
						NOBOOKMARK_COLS_BOUND
					), S_OK);

	// Allocate memory for a single row
	SAFE_ALLOC(pData, BYTE, cbBuffer);
	memset(pData, 0xCC, (size_t)cbBuffer);

	// Get sequential rows from Schema TYPES Rowset
	while( S_OK == (hr = pIRowset->GetNextRows(
			NULL,			// The chapter handle
			0,				// Count of rows to skip
			1,				// Number of rows to Fetch
			&cRows,			// Number of rows obtained
			&phRow))		// Handles of retrieved rows
	 && cRows != 0)
	{	
		COMPARE(cRows, 1);
	
		// Get the data for the row
		TESTC_(hr = pIRowset->GetData(phRow[0], hAccessRead, pData),S_OK);

		// Call the function UpdateCCol to set all the values
		// from TYPES Rowset into the CCol list element for
		// this column
		CCol NewCol(this);
		if(SUCCEEDED(UpdateCCol(cBindings, rgBindings, pData, NewCol, 
					ALLTYPES, &ulAutoIncPrec, lSQLSupport)))
		{
			// Set column ordinal
			// Mark as updateable if it is not a autoinc type
			NewCol.SetColNum(++cCols);

			NewCol.SetUpdateable(TRUE);

			if(NewCol.GetAutoInc())
			{
				if (!SettableProperty(DBPROP_COL_AUTOINCREMENT, DBPROPSET_COLUMN, pIDBInitialize,DATASOURCE_INTERFACE) ||
					!GetModInfo()->IsUsingITableDefinition())
				{
					NewCol.SetUpdateable(FALSE);
				}
			}			

			m_ColList.AddTail(NewCol);
		}

		TESTC_(hr = pIRowset->ReleaseRows(cRows, phRow, NULL, NULL, NULL),S_OK);
		SAFE_FREE(phRow);			
	}

	if( CHECK(hr, DB_S_ENDOFROWSET) )
		hr = S_OK;

CLEANUP:

	if(pIRowset && hAccessRead)
	{
		IAccessor *	pIAccessor = NULL;
		ULONG		cRefCounts = 0;

		if(VerifyInterface(pIRowset, IID_IAccessor, ROWSET_INTERFACE, 
			(IUnknown**)&pIAccessor))
		{
			CHECK(pIAccessor->ReleaseAccessor(hAccessRead, &cRefCounts), S_OK);
			COMPARE(cRefCounts,0);
			pIAccessor->Release();
		}
	}

	SAFE_RELEASE(pIDBInitialize);
	SAFE_RELEASE(pIGetDataSource);
	SAFE_RELEASE(pSchemaRowset);
	SAFE_RELEASE(pIRowset);

	SAFE_FREE(rgBindings);
	SAFE_FREE(pData);

	return hr;
}


/////////////////////////////////////////////////////////////////////////////
// CTree
//
/////////////////////////////////////////////////////////////////////////////
CTree::CTree
(
	IUnknown *	pSessionIUnknown,	// [IN] Session Interface
	WCHAR *		pwszModuleName,		// [IN] Tree name, optional (Default=NULL)
	ENULL		eNull				// [IN] Should nulls be used (Default=USENULLS)
)	
{	
	VARIANT				vValue;
	IDBInitialize *		pIDBInitialize = NULL;	
	IGetDataSource *	pIGetDataSource = NULL;	

	m_pwszTreeRoot		= NULL;
	m_pRootSchema		= NULL;
	m_pCrtSchema		= NULL;
	m_pStartingSchema	= NULL;
	
	//Obtain DataSource 
	VerifyInterface(pSessionIUnknown, IID_IGetDataSource, SESSION_INTERFACE, (IUnknown**)&pIGetDataSource);
	pIGetDataSource->GetDataSource(IID_IDBInitialize, (IUnknown**)&pIDBInitialize);

	// check DBPROP_GENERATEURL to see if we need to supply our own suffix
	VariantInit(&vValue);
	if(!GetProperty(DBPROP_GENERATEURL, DBPROPSET_DATASOURCEINFO, pIDBInitialize, &vValue))
		m_lGenerateURL = DBPROPVAL_GU_NOTSUPPORTED;
	else
		m_lGenerateURL = (LONG_PTR) vValue.lVal;

	//Cleanup
	SAFE_RELEASE(pIGetDataSource);

	if (GetModInfo()->GetRootBinder())
	{
		VerifyInterface(GetModInfo()->GetRootBinder(), IID_IBindResource, 
			BINDER_INTERFACE, (IUnknown**)&m_pIBindResource);
		VerifyInterface(GetModInfo()->GetRootBinder(), IID_ICreateRow, 
			BINDER_INTERFACE, (IUnknown**)&m_pICreateRow);
	}

	VariantClear(&vValue);
	SAFE_RELEASE(pIDBInitialize);
}




/////////////////////////////////////////////////////////////////////////////
// ~CTree
//
/////////////////////////////////////////////////////////////////////////////
CTree::~CTree()
{
	DestroyTree();

	SAFE_RELEASE(m_pIBindResource);
	SAFE_RELEASE(m_pICreateRow);
}




/////////////////////////////////////////////////////////////////////////////
// CTree::CreateTree
//
// For speed and generality this function only uses ICreateRow and
// IBindResource interfaces provided by the RootBinder
/////////////////////////////////////////////////////////////////////////////
HRESULT	CTree::CreateTree
(
	WCHAR *		pwszRootURL,
	DBCOUNTITEM ulDepth,
	DBCOUNTITEM cMaxChildrenPerNode
)
{
	HRESULT			hr					= E_FAIL;
	DBBINDSTATUS	dwBindStatus;
	WCHAR *			pwszPrivateRootNode	= NULL;
	CRowObject		RowObject;
	IRowset	*		pIRowset			= NULL;
	IRowChange * 	pIRowChange			= NULL;
	CSchema	*		pSchema				= NULL;
	ICreateRow *	pICreateRow			= NULL;

	// NOT YET IMPLEMEMTED
	// INI file support for CTree's

	if(GetModInfo()->GetRootBinder() == NULL)
	{
		odtLog << L"The Root Binder is not available." << ENDL;
		odtLog << L"Check your configuration." << ENDL;
	
		return DB_E_NOTSUPPORTED;
	}

	m_ulMaxDepth = ulDepth;
	m_cMaxChildrenPerNode = cMaxChildrenPerNode;

	// Create our own private node space
	TESTC(MakeSuffix(1, pwszRootURL, &pwszPrivateRootNode));
	ResetPosition();
	
	TESTC_(hr = CreateAndPopulateRow(NULL, 1, pwszPrivateRootNode,
		DBBINDURLFLAG_READWRITE | DBBINDURLFLAG_COLLECTION | DBBINDURLFLAG_OPENIFEXISTS,
		&pSchema, (IUnknown**)&pIRowChange, &m_pwszTreeRoot), S_OK);

	// if the interface is supported on rows, use it 
	if (VerifyInterface(pIRowChange, IID_ICreateRow, ROW_INTERFACE, (IUnknown**)&pICreateRow))
	{
		SAFE_RELEASE(m_pICreateRow);
		m_pICreateRow = pICreateRow;
	}

	TESTC_(hr = m_pIBindResource->Bind(
		NULL,
		m_pwszTreeRoot,
		DBBINDURLFLAG_READ,
		DBGUID_ROWSET,
		IID_IRowset, 
		NULL, NULL,
		&dwBindStatus,
		(IUnknown **)&pIRowset), S_OK);
	TESTC(NULL != pIRowset);

	// Set the current and root schema
	// Current schema always defaults to root schema
	m_pRootSchema = m_pCrtSchema = pSchema;

	TESTC_(hr = CreateChildRowset(pSchema, m_pwszTreeRoot, cMaxChildrenPerNode, 0, 1), S_OK);

CLEANUP:

	PROVIDER_FREE(pwszPrivateRootNode);

	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pIRowChange);

	ResetPosition();
	return hr;
}




/////////////////////////////////////////////////////////////////////////////
// CTree::CreateAndPopulateRow
//
/////////////////////////////////////////////////////////////////////////////
HRESULT CTree::CreateAndPopulateRow
(
	CSchema	*		pParentSchema,	// [in] parent node's schema 
	ULONG_PTR		ulSeed,			// [in] the seed of the row
	WCHAR *			pwszURL,		// [in] the URL of the row to be created
	DBBINDURLFLAG	dwBindURLFlags,	// [in] binding flags
	CSchema	**		ppSchema,		// [out] new row's schema 
	IUnknown **		ppIRowChange,	// [out] pointer to interface to be returned
	WCHAR **		ppwszNewURL		// [out] pointer to the name of the new row
)
{
	DBBINDURLSTATUS		dwBindStatus;
	CRowObject			RowObject;
	ULONG_PTR			ulChild			= 0;
	DBORDINAL			cColAccess		= 0;
	DBCOLUMNACCESS *	rgColAccess		= NULL;
	void *				pData			= NULL;
	HRESULT				hr				= E_FAIL;
	IRow *				pIRow			= NULL;
	IRowset	*			pIRowset		= NULL;
	CSchema	*			pRowSchema		= NULL;

	TESTC(NULL != ppwszNewURL);
	TESTC(NULL != ppIRowChange);
	
	*ppwszNewURL	= NULL;
	*ppIRowChange	= NULL;

	TESTC_(hr = m_pICreateRow->CreateRow(
								NULL,
								pwszURL,
								dwBindURLFlags | DBBINDURLFLAG_OPENIFEXISTS,
								DBGUID_ROW, 
								IID_IRow,
								NULL,
								NULL,
								&dwBindStatus, 
								ppwszNewURL,
								(IUnknown**)&pIRow), S_OK);

	// per spec pwszNewURL must not be null on success
	TESTC(*ppwszNewURL != NULL);
	
	// Free the created row
	SAFE_RELEASE(pIRow);

	// bind to child URL of the rowset
	TEST2C_(hr = m_pIBindResource->Bind(
			NULL,
			*ppwszNewURL,
			DBBINDURLFLAG_READ,
			DBGUID_ROWSET,
			IID_IRowset,
			NULL, NULL,
			&dwBindStatus,
			(IUnknown**)&pIRowset), S_OK, DB_E_NOTCOLLECTION);

	// create the CSchema associated with this row
	*ppSchema = AddSchema(pParentSchema, pIRowset, *ppwszNewURL, ulSeed);
	
	// Bind to the created row
	TESTC_(hr = m_pIBindResource->Bind(
								NULL,
								*ppwszNewURL,
								DBBINDURLFLAG_READWRITE,
								DBGUID_ROW, 
								IID_IRowChange,
								NULL,
								NULL,
								&dwBindStatus, 
								(IUnknown**)ppIRowChange), S_OK);
	
	// Insert data.
	pRowSchema = new CSchema(NULL, *ppwszNewURL, ulSeed);
	pRowSchema->InitSchema(*ppIRowChange);
	TESTC_(hr = RowObject.SetRowObject(*ppIRowChange), S_OK);

	// Only set data to the updateable, non-index base cols
	TESTC_(hr = RowObject.CreateColAccess(
								&cColAccess,
								&rgColAccess,
								&pData,
								NULL,
								UPDATEABLE_COLS_BOUND), S_OK);

	TESTC_(hr = RowObject.FillColAccess(pRowSchema, cColAccess, 
		rgColAccess, ulSeed, PRIMARY), S_OK);
	TESTC_(hr = RowObject.SetColumns(cColAccess, rgColAccess), S_OK);

	hr = S_OK;

CLEANUP:

	FreeColAccess(cColAccess, rgColAccess);
	SAFE_FREE(pData);
	SAFE_DELETE(pRowSchema);
	SAFE_RELEASE(pIRowset);

	return hr;
}




/////////////////////////////////////////////////////////////////////////////
// CTree::CreateChildRowset
//
/////////////////////////////////////////////////////////////////////////////
HRESULT CTree::CreateChildRowset
(
	CSchema	*	pParentSchema,
	WCHAR *		pwszParentURL, 
	DBCOUNTITEM	cChildren,
	DBCOUNTITEM	ulDepth,
	ULONG_PTR	ulParentSeed 
)
{
	HRESULT			hr					= E_FAIL;
	CSchema	*		pChildSchema		= NULL;
	WCHAR *			pwszURL				= NULL;
	WCHAR *			pwszNewURL			= NULL;
	ULONG_PTR		ulChild				= 0;

	IRowChange *	pIRowChange			= NULL;
	ICreateRow *	pICreateRow_Child	= NULL;

	DBBINDURLFLAG	dwBindURLFlags		= DBBINDURLFLAG_READWRITE | DBBINDURLFLAG_OPENIFEXISTS;

	ULONG_PTR		ulSeed = 0;

	// Arbitrary restriction which can be removed
	// if too restrictive
	ASSERT(ulDepth < 8);

	// termination condition
	if (ulDepth == m_ulMaxDepth)
	{
		pParentSchema->SetLeafNo(1);
		return S_OK;
	}

	if (ulDepth < m_ulMaxDepth-1)
		dwBindURLFlags |= DBBINDURLFLAG_COLLECTION;

	for (ulChild = 0; ulChild < cChildren; ulChild++, ulSeed++)
	{	
		// Add a suffix
		TESTC(MakeSuffix(ulSeed, pwszParentURL, &pwszURL));

		// Create a child 
		// and retrieve the schema of the newly create row
		TESTC_(hr = CreateAndPopulateRow(
									pParentSchema,
									ulSeed,
									pwszURL, 
									dwBindURLFlags,
									&pChildSchema,		
									(IUnknown**)&pIRowChange,
									&pwszNewURL), S_OK);

		TESTC_(hr = CreateChildRowset(pChildSchema, pwszNewURL,  
			cChildren, ulDepth+1, ulSeed),S_OK);

		// add the child schema to the parent schema
		pParentSchema->AddChild(pChildSchema);

		SAFE_RELEASE(pIRowChange);

		SAFE_FREE(pwszURL);
		SAFE_FREE(pwszNewURL);
	}
	
CLEANUP:

	if (pParentSchema && 0 == pParentSchema->GetLeafNo())
		pParentSchema->SetLeafNo(1);

	SAFE_FREE(pwszURL);
	SAFE_FREE(pwszNewURL);

	SAFE_RELEASE(pIRowChange);

	return hr;
}




/////////////////////////////////////////////////////////////////////////////
// CTree::DestroyTree
//
/////////////////////////////////////////////////////////////////////////////
HRESULT CTree::DestroyTree()
{
	// if we are using a file, not much to do
	SAFE_FREE(m_pwszTreeRoot);

	// destroy the CSchema tree
	SAFE_DELETE(m_pRootSchema);

	return S_OK;
}




/////////////////////////////////////////////////////////////////////////////
// CTree::GetSchema
//
/////////////////////////////////////////////////////////////////////////////
CSchema * CTree::GetSchema
(
	WCHAR *	pwszURL
)
{
	CSchema	*	pSchema		= NULL;
	WCHAR	*	pwszRowURL	= NULL;
	WCHAR	*	pwszRowName	= NULL;

	if (NULL == pwszURL ||	pwszURL != wcsstr(pwszURL, m_pwszTreeRoot))
		return NULL;

	// check whether the URL is the tree root itself
	if (L'/' != pwszURL[wcslen(m_pwszTreeRoot)])
		return m_pRootSchema;

	// get the relative URL in the tree
	pwszRowURL = wcsDuplicate(pwszURL+wcslen(m_pwszTreeRoot)+1);
	pwszRowName = wcstok(pwszRowURL, L"/");

	//Search tree
	pSchema = m_pRootSchema;
		
	//While not at end of list
	while (NULL != pwszRowName && NULL != pSchema) 
	{	
		if (NULL == pSchema->GetChildrenList())
		{
			pSchema = NULL;
			goto CLEANUP;
		}
	
		// go to the next level
		pSchema = pSchema->GetChild(pwszRowName);

		pwszRowName = wcstok(NULL, L"/");
	}

CLEANUP:

	SAFE_FREE(pwszRowURL);
	return pSchema;
}




/////////////////////////////////////////////////////////////////////////////
// CTree::GetCurrentSchema
//
/////////////////////////////////////////////////////////////////////////////
CSchema* CTree::GetCurrentSchema()
{
	return m_pCrtSchema;
}




/////////////////////////////////////////////////////////////////////////////
// CTree::GetCurrentRowURL
//
/////////////////////////////////////////////////////////////////////////////
WCHAR *CTree::GetCurrentRowURL()
{
	return (m_pCrtSchema ? m_pCrtSchema->GetRowURL() : NULL);
}




/////////////////////////////////////////////////////////////////////////////
// CTree::GetRootSchema
//
/////////////////////////////////////////////////////////////////////////////
CSchema* CTree::GetRootSchema()
{
	return m_pRootSchema;
}




/////////////////////////////////////////////////////////////////////////////
// CTree::MakeSuffix
//
/////////////////////////////////////////////////////////////////////////////
BOOL CTree::MakeSuffix
(
	DBCOUNTITEM ulNode,
	WCHAR *		pwszBaseURL, 
	WCHAR **	ppwszNewURL
)
{
	TBEGIN
	WCHAR	wszNumber[MAXBUFLEN];

	ASSERT(pwszBaseURL && ppwszNewURL);

	SAFE_ALLOC(*ppwszNewURL, WCHAR, wcslen(pwszBaseURL) + MAXBUFLEN);
	TESTC(*ppwszNewURL != NULL);
	wcscpy(*ppwszNewURL, pwszBaseURL);

	if (m_lGenerateURL == DBPROPVAL_GU_NOTSUPPORTED)
	{
		TESTC(NULL != _ui64tow(ulNode, wszNumber, 10));
		wcscat(*ppwszNewURL, L"/");	
		wcscat(*ppwszNewURL, wszNumber);
	}

CLEANUP:

	TRETURN
}




/////////////////////////////////////////////////////////////////////////////
// CTree::MakeSuffix
//
/////////////////////////////////////////////////////////////////////////////
BOOL CTree::MakeSuffix
(
	WCHAR *		pwszNode, 
	WCHAR *		pwszBaseURL, 
	WCHAR **	ppwszNewURL
)
{
	TBEGIN

	TESTC(pwszNode && pwszBaseURL && ppwszNewURL);

	SAFE_ALLOC(*ppwszNewURL, WCHAR, wcslen(pwszBaseURL) + wcslen(pwszNode) + 2);

	wcscpy(*ppwszNewURL, pwszBaseURL);

	if (m_lGenerateURL == DBPROPVAL_GU_NOTSUPPORTED)
	{
		wcscat(*ppwszNewURL, L"/");	
		wcscat(*ppwszNewURL, pwszNode);
	}

CLEANUP:

	TRETURN
} 




/////////////////////////////////////////////////////////////////////////////
// CTree::AddSchema
//
// Note: do not create the schema as having a parent yet
// this will be set in the parent's AddChild
/////////////////////////////////////////////////////////////////////////////
CSchema* CTree::AddSchema
(
	CSchema	*	pParentSchema,	// [in] pointer to the parent schema
	IRowset	*	pIParentRow,	// [in] interface to the current row
	WCHAR	*	pwszParentURL,	// [in] row URL of the parent
	LONG_PTR	lSeed			// [in] the seed value
)
{
	CSchema	*	pSchema = new CSchema(NULL, pwszParentURL, lSeed);

	TESTC(pSchema != NULL);
	pSchema->InitSchema((IUnknown *)pIParentRow);

CLEANUP:

	return pSchema;
} 




/////////////////////////////////////////////////////////////////////////////
// CTree::RemoveSchema
//
/////////////////////////////////////////////////////////////////////////////
CSchema *CTree::RemoveSchema
(
	CSchema	*	pSchema	// [in] pointer to the schema to be remove from the tree
)
{
	if (!pSchema)
		return NULL;

	if (pSchema == m_pRootSchema)
		m_pRootSchema = m_pStartingSchema = m_pCrtSchema = NULL;
	else
		pSchema->GetParentSchema()->RemoveChild(pSchema);

	return pSchema;
}




/////////////////////////////////////////////////////////////////////////////
// CTree::RemoveSchema
//
/////////////////////////////////////////////////////////////////////////////
CSchema *CTree::RemoveSchema
(
	WCHAR *	pwszURL	// [in] URL associated with the schema
)
{
	CSchema *pSchema = GetSchema(pwszURL);

	return (pSchema ? RemoveSchema(pSchema) : NULL);
}




/////////////////////////////////////////////////////////////////////////////
// CTree::MoveToNextSibling
//
/////////////////////////////////////////////////////////////////////////////
HRESULT	CTree::MoveToNextSibling()
{
	CSchema	*	pCSchema = NULL;

	// the traversal referred to the subtree rooted in m_pStartingSchema
	// do not return beyond it
	if (m_pStartingSchema == m_pCrtSchema)
		return E_FAIL;

	// get parent info
	pCSchema = m_pCrtSchema->GetParentSchema();

	// check whether there are other untravelled siblings
	pCSchema->m_lCacheSelection++;
	if ((LONG_PTR)pCSchema->GetChildrenNo() <= pCSchema->m_lCacheSelection)
		return DB_S_ERRORSOCCURRED;

	// get next sibling
	m_pCrtSchema = pCSchema->GetChild(pCSchema->m_lCacheSelection);
	m_pCrtSchema->m_lCacheSelection = -1; // nothing was selected inside this node

	return S_OK;
}




/////////////////////////////////////////////////////////////////////////////
// CTree::ReturnFromChildNode
//
// this function designed for speed; it crashes if m_pCrtSchema is NULL
// or whether the position exceeds the number of children
/////////////////////////////////////////////////////////////////////////////
void CTree::MoveDownToChildNode(DBCOUNTITEM ulChildOrdinal)
{
	ASSERT(m_pCrtSchema);

	m_pCrtSchema->m_lCacheSelection = ulChildOrdinal;
	m_pCrtSchema = m_pCrtSchema->GetChild(ulChildOrdinal);
} 




/////////////////////////////////////////////////////////////////////////////
// CTree::ReturnFromChildNode
//
/////////////////////////////////////////////////////////////////////////////
void CTree::ReturnFromChildNode()
{
	if (m_pCrtSchema)
		m_pCrtSchema = m_pCrtSchema->GetParentSchema();
} 




/////////////////////////////////////////////////////////////////////////////
// CTree::MoveToNextNode
//
// No memory is allocated for *ppwszURL
/////////////////////////////////////////////////////////////////////////////
HRESULT	CTree::MoveToNextNode
(
	WCHAR **	ppwszURL	// [out] the URL of the node
)
{
	HRESULT		hr;

	if (NULL == m_pCrtSchema)
		return E_FAIL;

	// try to move one level down
	if (0 < m_pCrtSchema->GetChildrenNo())
	{
		MoveDownToChildNode(0);
		if (NULL != ppwszURL)
		{
			*ppwszURL = m_pCrtSchema->GetRowURL();
		}
		return S_OK;
	}

	// try to move to the next sibling
	hr = DB_S_ERRORSOCCURRED;
	for (; DB_S_ERRORSOCCURRED == hr && m_pCrtSchema != m_pStartingSchema;)
	{
		// try to move lateral
		TEST2C_(hr = MoveToNextSibling(), S_OK, DB_S_ERRORSOCCURRED);
		if (S_OK != hr)
			// return one level
			ReturnFromChildNode();
	}
	
CLEANUP:

	if (S_OK == hr)
	{
		if (NULL != ppwszURL)
			*ppwszURL = GetCurrentRowURL();
	}
	else
	{
		// the traversal has completed
		m_pStartingSchema = m_pCrtSchema = m_pRootSchema;
		if (NULL != ppwszURL)
			*ppwszURL = NULL;	
	}

	return hr;
} 




/////////////////////////////////////////////////////////////////////////////
// CTree::SetPosition
//
/////////////////////////////////////////////////////////////////////////////
HRESULT	CTree::SetPosition
(
	WCHAR *	pwszURL
)
{
	if (NULL == pwszURL)
		return E_INVALIDARG;

	m_pCrtSchema = GetSchema(pwszURL);
	m_pStartingSchema = m_pCrtSchema;

	return (NULL != m_pCrtSchema ? S_OK : E_FAIL);
} 




/////////////////////////////////////////////////////////////////////////////
// CTree::GetRow
//
// gets the leaf row with number ulRow
// the row is selected from the starting subtree
// m_pCrtSchema is not affected by this method
// the first leaf is number 0
/////////////////////////////////////////////////////////////////////////////
HRESULT CTree::GetRow
(
	DBCOUNTITEM	ulRow,		//[in]  row number
	WCHAR **	ppwszURL	//[out] the URL of the chosen row
)
{
	CSchema	*	pCSchema		= NULL;
	CSchema	*	pChildSchema	= NULL;
	DBCOUNTITEM	ulChildren;
	POSITION	pos;

	if (NULL == ppwszURL)
		return E_INVALIDARG;

	// the search starts from the m_pStartingSchema
	pCSchema = m_pStartingSchema;
	if (ulRow >= pCSchema->GetLeafNo())
		return E_FAIL;

	ulChildren = pCSchema->GetChildrenNo();

	for (;0 < ulChildren;)
	{
		// get the child that contains the row
		pos = pCSchema->GetChildrenList()->GetHeadPosition();
		pChildSchema = NULL;
		for (;pos;)
		{
			pChildSchema = pCSchema->GetChildrenList()->GetNext(pos);
			if (pChildSchema->GetLeafNo() <= ulRow)
				ulRow -= pChildSchema->GetLeafNo();
			else
			{
				// move inside this subtree
				pCSchema = pChildSchema;
				break;
			}
		}
		ASSERT(NULL != pChildSchema);
		// update remaining variables
		ulChildren		= pCSchema->GetChildrenNo();
	}

	if (ulRow == 0)
		*ppwszURL = wcsDuplicate(pCSchema->GetRowURL());

	PRVTRACE(L"Leaf no %ul is %s\n", ulRow, *ppwszURL);

	return (0 == ulRow ? S_OK : E_FAIL);
} 



/////////////////////////////////////////////////////////////////////////////
// CTree::GetNoOfFirstLeafInSubtree
//
// get the number of the first leaf in the subtree
// starting at the current position
/////////////////////////////////////////////////////////////////////////////
DBCOUNTITEM CTree::GetNoOfFirstLeafInSubtree
(
	CSchema *	pCSchema
)
{
	DBCOUNTITEM	ulCrtFirstLeaf	= 0;
	CSchema	*	pSchema			= pCSchema;
	CSchema	*	pParentSchema	= NULL;
	CSchema	*	pSiblingSchema	= NULL;
	POSITION	pos;

	if(NULL == pCSchema)
		return 0;

	for (;NULL != pSchema && m_pStartingSchema != pSchema;)
	{
		pParentSchema = pSchema->GetParentSchema();
		pos = pParentSchema->GetChildrenList()->GetHeadPosition();
		for (;pos;)
		{
			pSiblingSchema = pParentSchema->GetChildrenList()->GetNext(pos);
			if (pSiblingSchema == pSchema)
				break;
			ulCrtFirstLeaf += pSiblingSchema->GetLeafNo();
		}
		pSchema = pParentSchema;
	}

	return ulCrtFirstLeaf;
} 




/////////////////////////////////////////////////////////////////////////////
// CTree::CopyTree
//
/////////////////////////////////////////////////////////////////////////////
HRESULT CTree::CopyTree
(
	CTree *	pCTree,			// [in] the tree to be copied
	WCHAR *	pwszBaseRootURL	// [in] the base root URL where to copy the tree
)
{
	IScopedOperations *	pIScopedOperations	= NULL;
	HRESULT				hr					= E_FAIL;
	DBBINDSTATUS		dwBindStatus;
	WCHAR *				rgpwszSrcURLs[1]	= {pCTree->GetRootURL()};
	WCHAR *				rgpwszDestURLs[1];
	WCHAR *				rgpwszNewURLs[1];
	WCHAR *				pStringsBuffer		= NULL;
	WCHAR *				pwszRowURL			= NULL;
	WCHAR *				pwszRelRowURL		= NULL;
	WCHAR *				pwszFirstNode		= NULL;
	WCHAR *				pwszRootParentURL	= NULL;

	// clean the old values if any
	DestroyTree();

	if(GetModInfo()->GetRootBinder() == NULL)
	{
		odtLog << L"The Root Binder is not available." << ENDL;
		odtLog << L"Check your configuration." << ENDL;
	
		return DB_E_NOTSUPPORTED;
	}

	// build the new root value 
	pwszFirstNode = wcsrchr(pCTree->GetRootURL(), L'/')+1;
	TESTC(MakeSuffix(pwszFirstNode, pwszBaseRootURL, &rgpwszDestURLs[0]));
	ResetPosition();
	m_ulMaxDepth			= pCTree->GetMaxDepth();
	m_cMaxChildrenPerNode	= pCTree->GetMaxChildrenPerNode();

	// bind to pwszBaseRootURL 
	// (both the src and the destination should be in its scope)
	TESTC_(hr = m_pIBindResource->Bind(
		NULL, GetModInfo()->GetRootURL(),
		DBBINDURLFLAG_READ,
		DBGUID_ROW,
		IID_IScopedOperations,
		NULL, NULL,
		&dwBindStatus,
		(IUnknown**)&pIScopedOperations
		), S_OK);

	TESTC_(hr = pIScopedOperations->Copy(
		1, (const WCHAR**)rgpwszSrcURLs, (const WCHAR**)rgpwszDestURLs,
		DBCOPY_REPLACE_EXISTING,
		NULL,
		&dwBindStatus,
		rgpwszNewURLs,
		&pStringsBuffer
		), S_OK);
	
	m_pwszTreeRoot = wcsDuplicate(rgpwszNewURLs[0]);

	// build all the structures from the CTree
	pwszRootParentURL = wcsDuplicate(m_pwszTreeRoot);
	*(wcsrchr(pwszRootParentURL, L'/')) = L'\0';

	delete m_pRootSchema;
	m_pRootSchema = new CSchema(pCTree->GetRootSchema(), (CSchema*)NULL);
	m_pRootSchema->UpdateURL(pwszRootParentURL);

	hr = S_OK;

CLEANUP:

	ResetPosition();
	SAFE_FREE(pwszRootParentURL);
	SAFE_FREE(pStringsBuffer);
	SAFE_RELEASE(pIScopedOperations);
	SAFE_FREE(rgpwszDestURLs[0]);

	return hr;
} 




/////////////////////////////////////////////////////////////////////////////
// CTree::CheckRowset
//
//	Check the componence of a rowset
//
/////////////////////////////////////////////////////////////////////////////
BOOL CTree::CheckRowset
(
	IRowset *	pIRowset,		// [in] interface on the checked rowset
	WCHAR	*	pwszParentURL,	// [in] the URL used to open the rowset
	BOOL		fCmpRowVal		// [in] whether or not to compare the row values
)
{
	CSchema	*	pCRowsetSchema;

	if (NULL == pIRowset || NULL == pwszParentURL)
		return FALSE;

	// retrieve the info about the rowset	
	pCRowsetSchema = GetSchema(pwszParentURL);
	return CheckRowset(pIRowset, pCRowsetSchema, fCmpRowVal);
} 



/////////////////////////////////////////////////////////////////////////////
// CTree::CheckRowset
//
//	Check the componence of a rowset
//
/////////////////////////////////////////////////////////////////////////////
BOOL CTree::CheckRowset
(
	IRowset	*	pIRowset,		// [in] interface on the checked rowset
	CSchema	*	pCRowsetSchema,	// [in] schema of the rowset to be checked
	BOOL		fCmpRowVal		// [in] whether or not to compare the row values
)
{
	TBEGIN
	HRESULT			hr				= S_OK;
	IColumnsInfo *	pIColumnsInfo	= NULL;
	IAccessor *		pIAccessor		= NULL;
	HACCESSOR		hAccessor		= 0;
	DBCOUNTITEM		cRowsObtained;
	HROW *			rghRow			= NULL;	
	BOOL *			rgfRows			= NULL;
	DBCOUNTITEM		ulRowKey;
	DBCOUNTITEM		ulRetrievedRows	= 0;
	DBCOUNTITEM		ulExpectedRows	= 0;	
	IGetRow	*		pIGetRow		= NULL;
	CSchema	*		pCSchema		= NULL;
	DBBINDING *		rgBindings		= NULL;
	DBCOUNTITEM		cBindings		= 0;
	DBLENGTH		ulRowSize;
	BYTE *			pData			= NULL;
	LPOLESTR		pwszRowURL		= NULL;

	// argument checking
	TESTC(pCRowsetSchema != NULL);

	if (fCmpRowVal)
	{
		// need accessor for value comparison
		// create the accessor and bindings
		TESTC_(hr = GetAccessorAndBindings(
							pIRowset, DBACCESSOR_ROWDATA, &hAccessor,	&rgBindings,		
							&cBindings, &ulRowSize, DBPART_VALUE |DBPART_STATUS |DBPART_LENGTH,
							UPDATEABLE_COLS_BOUND|NOBOOKMARK_COLS_BOUND, 
							FORWARD, NO_COLS_BY_REF,
							NULL,					// OUT: Array of DBCOLUMNINFOs
							0,						// OUT: Count of DBCOULMNINFOs
							NULL,					//&pStringsBuffer, 
							DBTYPE_EMPTY, 
							0, NULL),S_OK);
		SAFE_ALLOC(pData, BYTE, ulRowSize);	//data
		
		// get an IAccessor interface to release the accessor 
		TESTC_(hr = pIRowset->QueryInterface(IID_IAccessor, (void**)&pIAccessor), S_OK);
	}

	// allocate space for array and initialize it with FALSE (no row was retrieved)
	ulExpectedRows = pCRowsetSchema->GetChildrenNo();
	SAFE_ALLOC(rgfRows, BOOL, ulExpectedRows);
	memset(rgfRows, 0, (size_t)(ulExpectedRows*sizeof(BOOL)));

	TESTC_(hr = pIRowset->QueryInterface(IID_IGetRow, (void**)&pIGetRow), S_OK);

	// get the first row
	TEST2C_(hr = pIRowset->GetNextRows(NULL, 0, 1, &cRowsObtained, &rghRow), 
		S_OK, DB_S_ENDOFROWSET);
	TESTC(S_OK != hr || NULL != rghRow && 1 == cRowsObtained);

	for (;S_OK == hr;)
	{
		// get the URL of the row
		TESTC_(hr = pIGetRow->GetURLFromHROW(rghRow[0], &pwszRowURL), S_OK);

		// find the row number inside the rowset
		pCSchema = pCRowsetSchema->GetChild(1+wcsrchr(pwszRowURL, L'/'), &ulRowKey);

		// check that the row doesn't appear twice in the rowset
		// and mark it as retrieved
		ASSERT(NULL != pCSchema);
		TESTC(rgfRows[ulRowKey] != TRUE);
		rgfRows[ulRowKey] = TRUE;
		ulRetrievedRows++;

		// compare the row values
		if (fCmpRowVal && -1 != pCSchema->GetSeed())
		{
			TESTC_(hr = pIRowset->GetData(rghRow[0], hAccessor, pData), S_OK);
	
			TESTC(CompareData(0, NULL, 
				pCSchema->GetSeed(),	// row number
				pData,					// retrieved data
				cBindings, rgBindings,
				pCRowsetSchema,			// use these columns to do the comparison
				NULL));
		}

		// move to the next row in the rowset
		TESTC_(hr = pIRowset->ReleaseRows(cRowsObtained, rghRow, NULL, NULL, NULL), S_OK);
		SAFE_FREE(rghRow);
		TEST2C_(hr = pIRowset->GetNextRows(NULL, 0, 1, &cRowsObtained, &rghRow), 
			S_OK, DB_S_ENDOFROWSET);
		TESTC(S_OK != hr || NULL != rghRow && 1 == cRowsObtained);
		
		SAFE_FREE(pwszRowURL);
	}

	TESTC_(hr, DB_S_ENDOFROWSET);
	TESTC(ulRetrievedRows == ulExpectedRows);

CLEANUP:

	SAFE_FREE(pData);
	SAFE_FREE(rgBindings);
	SAFE_FREE(pwszRowURL);

	SAFE_FREE(rghRow);
	if (fCmpRowVal && NULL != pIAccessor)
		pIAccessor->ReleaseAccessor(hAccessor, NULL);
	SAFE_RELEASE(pIAccessor);
	SAFE_RELEASE(pIGetRow);
	SAFE_FREE(rgfRows);
	
	TRETURN
} 




/////////////////////////////////////////////////////////////////////////////
// CTree::CheckTreeStructure
//
// check that the tree exist
// NOTE: This method modifies tree position cursor
/////////////////////////////////////////////////////////////////////////////
BOOL CTree::CheckTreeStructure()
{
	TBEGIN
	HRESULT			hr;
	IRow *			pIRow		= NULL;
	IRowset	*		pIRowset	= NULL;
	DBCOUNTITEM		ulIter		= 0;

	// go AFAP (As Fast As Possible) -> iterate the CSchema list
	hr = S_OK;
	for (; S_OK == hr; )
	{
		// get the current CSchema and move to the next one
		TESTC_(hr = m_pIBindResource->Bind(NULL, m_pCrtSchema->GetRowURL(),
			DBBINDURLFLAG_READ, DBGUID_ROW, IID_IRow, NULL, NULL, NULL, (IUnknown**)&pIRow), S_OK);
		SAFE_RELEASE(pIRow);

		// bind the current URL as a rowset
		TEST2C_(hr = m_pIBindResource->Bind(NULL, m_pCrtSchema->GetRowURL(),
				DBBINDURLFLAG_READ, DBGUID_ROWSET, IID_IRowset, 
				NULL, NULL, NULL, (IUnknown**)&pIRowset), S_OK, DB_E_NOTCOLLECTION);
		if (S_OK == hr)
		{
			TESTC(CheckRowset(pIRowset, m_pCrtSchema, FALSE));
		}
		else
		{
			TESTC(0 == m_pCrtSchema->GetChildrenNo());
		}
		SAFE_RELEASE(pIRowset);

		ulIter++;
		TEST2C_(hr = MoveToNextNode(NULL), S_OK, DB_S_ERRORSOCCURRED);
	}

CLEANUP:

	ResetPosition();
	TOUTPUT_IF_FAILED(GetCurrentRowURL());

	SAFE_RELEASE(pIRow);
	SAFE_RELEASE(pIRowset);
	TRETURN
} 




/////////////////////////////////////////////////////////////////////////////
// CTree::CheckTree
//
// check that the tree exist
// NOTE: This method modifies tree position cursor
/////////////////////////////////////////////////////////////////////////////
BOOL CTree::CheckTree()
{
	TBEGIN
	HRESULT			hr;
	IRowset	*		pIRowset	= NULL;
	DBCOUNTITEM		ulIter		= 0;

	// go AFAP (As Fast As Possible) -> iterate the CSchema list
	hr = S_OK;
	for (; S_OK == hr; )
	{
		// binding to a row and verifying the values
		TESTC_(hr = VerifyRowValues(m_pCrtSchema->GetRowURL()), S_OK);

		// bind the current URL as a rowset
		TEST2C_(hr = m_pIBindResource->Bind(NULL, m_pCrtSchema->GetRowURL(),
			DBBINDURLFLAG_READ, DBGUID_ROWSET, IID_IRowset, 
			NULL, NULL, NULL, (IUnknown**)&pIRowset), S_OK, DB_E_NOTCOLLECTION);
		if (S_OK == hr)
		{
			TESTC(CheckRowset(pIRowset, m_pCrtSchema, FALSE));
		}
		else
		{
			TESTC(0 == m_pCrtSchema->GetChildrenNo());
		}
		SAFE_RELEASE(pIRowset);

		ulIter++;
		TEST2C_(hr = MoveToNextNode(NULL), S_OK, DB_S_ERRORSOCCURRED);
	}

CLEANUP:

	ResetPosition();
	TOUTPUT_IF_FAILED(GetCurrentRowURL());

	SAFE_RELEASE(pIRowset);
	TRETURN
} 




/////////////////////////////////////////////////////////////////////////////
// CTree::GetRelativeRowURL
//
// this function doesn't allocate new memory!
/////////////////////////////////////////////////////////////////////////////
HRESULT CTree::GetRelativeRowURL
(
	WCHAR *		pwszAbsoluteRowURL,	// [in]
	WCHAR **	ppwszRelativeRowURL	// [out]
)
{
	HRESULT	hr = E_FAIL;

	if ( NULL == pwszAbsoluteRowURL	||	NULL == ppwszRelativeRowURL ) 
		return E_FAIL;

	if (pwszAbsoluteRowURL != wcsstr(pwszAbsoluteRowURL, m_pwszTreeRoot))
		return E_FAIL;

	// prepare output value
	if (wcslen(pwszAbsoluteRowURL) > wcslen(m_pwszTreeRoot))
		*ppwszRelativeRowURL = pwszAbsoluteRowURL+wcslen(m_pwszTreeRoot)+1;
	else
		*ppwszRelativeRowURL = pwszAbsoluteRowURL+wcslen(m_pwszTreeRoot);

	return S_OK;
}




/////////////////////////////////////////////////////////////////////////////
// CTree::GetAbsoluteRowURL
//
// allocates memory for the new string
/////////////////////////////////////////////////////////////////////////////
HRESULT	CTree::GetAbsoluteRowURL
(
	WCHAR *		pwszRelativeRowURL,	// [in]
	WCHAR **	ppwszAbsoluteRowURL	// [out]
)
{
	HRESULT	hr;

	if ( NULL == pwszRelativeRowURL	||	NULL == ppwszAbsoluteRowURL )
		return E_FAIL;

	SAFE_ALLOC(*ppwszAbsoluteRowURL, WCHAR, 
		wcslen(pwszRelativeRowURL)+wcslen(m_pwszTreeRoot)+2);

	wcscpy(*ppwszAbsoluteRowURL, m_pwszTreeRoot);
	if (wcslen(pwszRelativeRowURL))
	{
		wcscat(*ppwszAbsoluteRowURL, L"/");
		wcscat(*ppwszAbsoluteRowURL, pwszRelativeRowURL);
	}
	
	hr = S_OK;

CLEANUP:

	return hr;
}




/////////////////////////////////////////////////////////////////////////////
// CTree::IsCollection
//
// tests whether the row give by its URL is a collection
/////////////////////////////////////////////////////////////////////////////
BOOL CTree::IsCollection
(
	WCHAR *	pwszRowURL
)
{
	HRESULT			hr;
	IRowset	*		pIRowset = NULL;
	DBBINDSTATUS	dwBindStatus;

	if (NULL==pwszRowURL)
		return FALSE;

	if (0 < GetSchema(pwszRowURL)->GetChildrenNo())
		return TRUE;

	hr = m_pIBindResource->Bind(
		NULL,
		pwszRowURL,
		DBBINDURLFLAG_READ,
		DBGUID_ROWSET,
		IID_IRowset, 
		NULL, NULL,
		&dwBindStatus,
		(IUnknown **)&pIRowset);

	TESTC(SUCCEEDED(hr) || NULL != pIRowset);

CLEANUP:

	SAFE_RELEASE(pIRowset);
	return (S_OK == hr);
}




/////////////////////////////////////////////////////////////////////////////
// CTree::IsCollection
//
// tests whether the row give by its URL is a collection
/////////////////////////////////////////////////////////////////////////////
BOOL CTree::IsCollection
(	
	CSchema *	pSchema
)
{
	HRESULT			hr;
	IRowset *		pIRowset = NULL;
	DBBINDSTATUS	dwBindStatus;

	if (NULL==pSchema)
		return FALSE;

	if (0 < pSchema->GetChildrenNo())
		return TRUE;

	hr = m_pIBindResource->Bind(
		NULL,
		pSchema->GetRowURL(),
		DBBINDURLFLAG_READ,
		DBGUID_ROWSET,
		IID_IRowset, 
		NULL, NULL,
		&dwBindStatus,
		(IUnknown **)&pIRowset);

	TESTC(SUCCEEDED(hr) || NULL != pIRowset);

CLEANUP:

	SAFE_RELEASE(pIRowset);

	return (S_OK == hr);
} 




/////////////////////////////////////////////////////////////////////////////
// CTree::GetRowLevel
//
/////////////////////////////////////////////////////////////////////////////
HRESULT CTree::GetRowLevel
(
	WCHAR *			pwszURL,		// [in] the URL of the chosen row
	DBCOUNTITEM	*	pulLevel		// [out] the level of the row in the hierarchy
)
{
	DBCOUNTITEM	ulLevel	= 0;
	WCHAR *		pwChar	= NULL;

	// the fastest way to do it is to start after the root prefix
	// and count the slashes
	if (NULL == pwszURL || NULL == m_pwszTreeRoot || NULL == pulLevel)
		return E_INVALIDARG;

	if (pwszURL != wcsstr(pwszURL, m_pwszTreeRoot))
		return DB_E_NOTFOUND;

	for (pwChar = pwszURL+wcslen(m_pwszTreeRoot); pwChar && *pwChar; ulLevel++)
	{
		if (*pwChar)
			pwChar = wcschr(++pwChar, L'/');
	}

	*pulLevel = ulLevel;
	return S_OK;
} 




/////////////////////////////////////////////////////////////////////////////
// CTree::GetOuterCollection
//
// the function receives the URL of a leaf
// it returns either the same URL if the row is a collection
// or its parent if the row is atomic (Singleton)
/////////////////////////////////////////////////////////////////////////////
HRESULT CTree::GetOuterCollection
(
	WCHAR *		pwszLeafURL,		// [in] the URL of the leaf row
	WCHAR **	ppwszOuterCollection// [out] the URL of the outest Collection in the given path
)
{
	WCHAR *	pwChar	= NULL;

	if (NULL == pwszLeafURL || NULL == ppwszOuterCollection)
	{
		if (ppwszOuterCollection)
			*ppwszOuterCollection = NULL;
		return E_INVALIDARG;
	}

	*ppwszOuterCollection = wcsDuplicate(pwszLeafURL);
	if (!IsCollection(pwszLeafURL))
	{
		if (NULL == (pwChar = wcsrchr(*ppwszOuterCollection, L'/')))
		{
			*ppwszOuterCollection = NULL;
			return E_INVALIDARG;
		}
		*pwChar = L'\0';
	}

	return S_OK;
} 




/////////////////////////////////////////////////////////////////////////////
// CTree::GetCommonAncestor
//
/////////////////////////////////////////////////////////////////////////////
HRESULT CTree::GetCommonAncestor
(
	WCHAR *		pwszFirstURL,		// [in] the first URL
	WCHAR *		pwszSecondURL,		// [in] the second URL
	WCHAR **	ppwszCommonURL		// [out] the URL of closest common ancestor
)
{
	WCHAR *	pwChar		= NULL;
	WCHAR *	pwszConstr	= NULL;

	if (NULL ==ppwszCommonURL)
		return E_INVALIDARG;
	*ppwszCommonURL = NULL;

	if (NULL == pwszFirstURL || NULL == pwszSecondURL)
		return E_INVALIDARG;

	if (wcslen(pwszFirstURL) < wcslen(pwszSecondURL))
	{
		*ppwszCommonURL = wcsDuplicate(pwszFirstURL);
		pwszConstr = pwszSecondURL;
	}
	else
	{
		*ppwszCommonURL = wcsDuplicate(pwszSecondURL);
		pwszConstr = pwszFirstURL;
	}

	for (; pwszConstr != wcsstr(pwszConstr, *ppwszCommonURL);)
	{
		// eliminate suffix (from the last slash on)
		if (NULL == (pwChar = wcsrchr(*ppwszCommonURL, L'/')))
		{
			SAFE_FREE(*ppwszCommonURL);
			*ppwszCommonURL = NULL;
			return E_FAIL;
		}
		*pwChar = L'\0';
	}

	return S_OK;
} 




/////////////////////////////////////////////////////////////////////////////
//
// CTree::VerifyRowValues
// check the column values of a row
/////////////////////////////////////////////////////////////////////////////
HRESULT CTree::VerifyRowValues
(
	WCHAR *		pwszRowURL,			// [in] the URL of the row to be checked
	IUnknown *	pIUnk,				// [in] ptr to the row interface
	BOOL		fFromRowset			// [in] whether it was obtained from a rowset 
									// or through direct binding
)
{
	HRESULT			hr = E_FAIL;
	WCHAR *			pwszParentURL	= NULL;
	CSchema	*		pParentSchema	= NULL;
	CSchema	*		pSchema			= GetSchema(pwszRowURL);
	CRowObject		RowObj;
	DBORDINAL		cColAccess		= 0;
	DBCOLUMNACCESS *rgColAccess		= NULL;
	void *			pData			= NULL;
	CSchema *		pCSchema		= NULL;
	LONG_PTR		lSeed			= pSchema->GetSeed();
	BOOL			fRelSchema		= FALSE;

	if (-1L == pSchema->GetSeed())
	{
		hr = S_OK;
		goto CLEANUP;
	}

	ASSERT(0 <= pSchema->GetSeed());

	// get parent URL
	pwszParentURL	= wcsDuplicate(pwszRowURL);
	*(WCHAR*)(wcsrchr(pwszParentURL, L'/')) = L'\0';

	// get schema of the parent URL
	pParentSchema = GetSchema(pwszParentURL);
	if (NULL == pParentSchema)
	{
		hr = S_OK;
		goto CLEANUP;
	}

	// create the RowObject and compare the values
	RowObj.SetRowObject(pIUnk);

	// it should come to the point of just invoking CRowObject::VerifyGetColumns
	// but since extra columns are not persisted in Conformance Provider, we have 
	// to do some hacks
	TESTC_(hr = RowObj.CreateColAccessUsingFilter(&cColAccess, 
		&rgColAccess, &pData, pParentSchema), S_OK);

	// get row values for
	TESTC_(hr = RowObj.pIRow()->GetColumns(cColAccess, rgColAccess), S_OK);

	if (!fFromRowset)
	{
		pSchema = new CSchema(NULL, pwszRowURL, lSeed);
		pSchema->InitSchema(pIUnk);
		fRelSchema = TRUE;
	}

	hr = E_FAIL;

	// we use pParentSchema for comparison because this is the schema that was use to generate
	TESTC(RowObj.CompareColAccess(cColAccess, rgColAccess,
		pSchema->GetSeed(), pSchema, PRIMARY));

	hr = S_OK;

CLEANUP:

	if (fRelSchema)
		SAFE_DELETE(pSchema);

	SAFE_FREE(pwszParentURL);
	FreeColAccess(cColAccess, rgColAccess);
	SAFE_FREE(pData);

	return hr;
} 




/////////////////////////////////////////////////////////////////////////////
//
// CTree::VerifyRowValues
// check the column values of a row
/////////////////////////////////////////////////////////////////////////////
HRESULT	CTree::VerifyRowValues
(
	WCHAR *	pwszRowURL		// [in] the URL of the row to be checked
)
{
	HRESULT	hr		= E_FAIL;
	IRow *	pIRow	= NULL;

	ASSERT(NULL != GetModInfo()->GetRootBinder());
	
	// bind to the row
	TESTC_(hr = m_pIBindResource->Bind(NULL, pwszRowURL, DBBINDURLFLAG_READ, DBGUID_ROW,
		IID_IRow, NULL, NULL, NULL, (IUnknown**)&pIRow), S_OK);

	// delegate to the other VerifyRowValues
	TESTC_(hr = VerifyRowValues(pwszRowURL, pIRow, FALSE), S_OK);

CLEANUP:

	SAFE_RELEASE(pIRow);

	return hr;
} 




