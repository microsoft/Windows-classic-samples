//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1995-2000 Microsoft Corporation.  
//
// @doc 
//
// @module CRowObject Implementation Module| 	This module contains implementation information
// for row object functions for the private library.
//
// @comm
// Special Notes...:	(OPTIONAL NOTES FOR SPECIAL CIRCUMSTANCES)
//
// <nl><nl>
// Revision History:<nl>
//	
//	[00] MM-DD-YY	EMAIL_NAME	ACTION PERFORMED... <nl>
//	[01] 01-20-95	Microsoft	Created <nl>
//	[02] 12-01-96	Microsoft	Updated for release <nl>
//
// @head3 CRowObject Elements|
//
// @subindex CRowObject
//
//---------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////
// Includes
//
/////////////////////////////////////////////////////////////////////////////
#include "privstd.h"
#include "coledb.hpp"



/////////////////////////////////////////////////////////////////////////////
// CRowObject
//
/////////////////////////////////////////////////////////////////////////////
CRowObject::CRowObject()
{	
	//[Mandatory] Interfaces
	m_pIRow				= NULL;
	m_pIGetSession		= NULL;
	m_pIConvertType		= NULL;
	m_pIColumnsInfo		= NULL;

	//[Optional] Interfaces
	m_pIDBCreateCommand = NULL;

	//ColumnAccess
	m_cColAccess		= 0;
	m_rgColAccess		= NULL;
	m_pData				= NULL;
}


/////////////////////////////////////////////////////////////////////////////
// ~CRowObject
//
/////////////////////////////////////////////////////////////////////////////
CRowObject::~CRowObject()
{
	ReleaseRowObject();
}


/////////////////////////////////////////////////////////////////////////////
// CRowObject::ReleaseRowObject
//
/////////////////////////////////////////////////////////////////////////////
void CRowObject::ReleaseRowObject()
{
	//[Mandatory] Interfaces
	SAFE_RELEASE(m_pIRow);
	SAFE_RELEASE(m_pIGetSession);
	SAFE_RELEASE(m_pIConvertType);
	SAFE_RELEASE(m_pIColumnsInfo);

	//[Optional] Interfaces
	SAFE_RELEASE(m_pIDBCreateCommand);

	//ColumnAccess
	ReleaseColAccess();
}


/////////////////////////////////////////////////////////////////////////////
// CRowObject::ReleaseColAccessors
//
/////////////////////////////////////////////////////////////////////////////
void CRowObject::ReleaseColAccess()
{
	for (DBORDINAL cCol=0; cCol< m_cColAccess; cCol++)
	{
		ReleaseDBID(&m_rgColAccess[cCol].columnid, FALSE);
	}
	m_cColAccess = 0;
	SAFE_FREE(m_rgColAccess);
	SAFE_FREE(m_pData);
}


/////////////////////////////////////////////////////////////////////////////
// CRowObject::CreateRowObject
//
/////////////////////////////////////////////////////////////////////////////
HRESULT	CRowObject::CreateRowObject(IUnknown* pUnkRowset, HROW hRow)
{
	HRESULT hr = E_FAIL;
	HRESULT hrSet = E_FAIL;
	IGetRow* pIGetRow = NULL;
	IRow* pIRow = NULL;

	//IGetRow is an optional interface on the rowset...
	if(!VerifyInterface(pUnkRowset, IID_IGetRow, ROWSET_INTERFACE, (IUnknown**)&pIGetRow))
		return E_NOINTERFACE; 

	//Release the previous row object
	ReleaseRowObject();
	
	//Obtain the row object from this row of the rowset...
	hr = pIGetRow->GetRowFromHROW(NULL, hRow, IID_IRow, (IUnknown**)&pIRow);
	QTESTC(hr==S_OK || hr==DB_S_NOROWSPECIFICCOLUMNS);

	//Delegate to our other function
	//NOTE: We want to return the above warning, if no other failures occurr...
	hrSet = SetRowObject(pIRow);
	if(hrSet != S_OK)
		hr = hrSet;

CLEANUP:
	SAFE_RELEASE(pIGetRow);
	SAFE_RELEASE(pIRow);
	return hr;		
}



/////////////////////////////////////////////////////////////////////////////
// CRowObject::SetRowObject
//
/////////////////////////////////////////////////////////////////////////////
HRESULT	CRowObject::SetRowObject(IUnknown* pUnkRow)
{
	HRESULT hr = S_OK;
	IRow* pIRow = NULL;

	//IRow is an mandatory interface on the row object...
	if(!VerifyInterface(pUnkRow, IID_IRow, ROW_INTERFACE, (IUnknown**)&pIRow))
		return E_NOINTERFACE; 

	//Release the previous row object
	ReleaseRowObject();
	
	//Now that everything worked successfully, save the interface...
	m_pIRow = pIRow;

	//[Mandatory] Interfaces
	COMPARE(VerifyInterface(m_pIRow, IID_IGetSession,	ROW_INTERFACE, (IUnknown**)&m_pIGetSession),	TRUE);
	COMPARE(VerifyInterface(m_pIRow, IID_IConvertType,	ROW_INTERFACE, (IUnknown**)&m_pIConvertType),	TRUE);
	COMPARE(VerifyInterface(m_pIRow, IID_IColumnsInfo,	ROW_INTERFACE, (IUnknown**)&m_pIColumnsInfo),	TRUE);

	//QI m_pIRow for m_pIDBCreateCommand - may not be supported.
	VerifyInterface(m_pIRow, IID_IDBCreateCommand, ROW_INTERFACE, (IUnknown**)&m_pIDBCreateCommand);

	//Create "Default" ColAccess bindings for all the data...
	TESTC_(hr = CreateColAccess(&m_cColAccess, &m_rgColAccess, &m_pData, NULL, ALL_COLS_BOUND, BLOB_LONG),S_OK);

CLEANUP:
	return hr;		
}


////////////////////////////////////////////////////////////////////////
//	CRowObject::GetColumns
//
////////////////////////////////////////////////////////////////////////
HRESULT CRowObject::GetColumns(DBORDINAL cColAccess, DBCOLUMNACCESS* rgColAccess)
{
	ASSERT(m_pIRow);
	HRESULT hr = S_OK;
	
	//IRow::GetColumns (GetData)
	hr = m_pIRow->GetColumns(cColAccess, rgColAccess);

	//Display any binding errors and status'
	TESTC(VerifyColAccess(hr, cColAccess, rgColAccess));

CLEANUP:
	return hr;
}


////////////////////////////////////////////////////////////////////////
//	CRowObject::GetColumns
//
////////////////////////////////////////////////////////////////////////
HRESULT CRowObject::GetColumns(DBCOUNTITEM cBindings, DBBINDING* rgBindings, void* pData)
{
	DBORDINAL cColAccess = 0;
	DBCOLUMNACCESS* rgColAccess = NULL;
	HRESULT hr = S_OK;
	DBORDINAL iCol = 0;
	DBCOUNTITEM i = 0;

	//Convert bindings to ColAccess
	QTESTC_(hr = BindingsToColAccess(cBindings, rgBindings, pData, &cColAccess, &rgColAccess),S_OK);
		
	//IRow::GetColumns (GetData)
	hr = GetColumns(cColAccess, rgColAccess);

	//The one main difference between DBBINDING and DBCOLUMNACCESS is that DBCOLUMNACCESS doesn't
	//contain a pObject indicating what interface to return for the storage object.  Thus its the 
	//consumers resposibility to always QI for the indicated interface.  So trying to "reuse" pData
	//buffers for GetData and GetColumns will cause a problem for Storage objects since GetData
	//should be albe to use them directly and GetColumns requires a QI.  So do a fixup
	//for the GetColumns case here, so the origin of the buffer is irrelavent and can be
	//used interchangebly throughout our other helpers...
	if(SUCCEEDED(hr))
	{
		for(i=0; i<cBindings; i++)
		{
			if(rgBindings[i].pObject && rgColAccess[i].dwStatus == DBSTATUS_S_OK && 
				rgColAccess[i].pData)
			{
				switch(rgBindings[i].wType)
				{
					case DBTYPE_IUNKNOWN:
					case DBTYPE_IUNKNOWN | DBTYPE_BYREF:
					{
						IUnknown*  pUnkSrc   = *(IUnknown**)rgColAccess[i].pData;
						IUnknown** ppUnkDest = (IUnknown**)rgColAccess[i].pData;
						
						if(rgBindings[i].wType & DBTYPE_BYREF)
						{
							pUnkSrc		= *(IUnknown**)pUnkSrc; 
							ppUnkDest   = (IUnknown**)*ppUnkDest;
						}

						if(pUnkSrc)
						{
							hr = pUnkSrc->QueryInterface(rgBindings[i].pObject->iid, (void**)ppUnkDest);
							SAFE_RELEASE(pUnkSrc);
							TESTC_(hr, S_OK);
						}
						break;
					}
				};
			}
		}
	}

	//Update the Status, Length values...
	QTESTC_(UpdateStatusLength(cColAccess, rgColAccess, cBindings, rgBindings, pData),S_OK);

CLEANUP:
	//Free ColAccess and the columnids.
	//NOTE:  We don't use FreeColAccess, since it frees the outofline data as well
	//and we are handing back the outofline data through the accessor...
	for(iCol=0; iCol<cColAccess; iCol++)
		ReleaseDBID(&rgColAccess[iCol].columnid, FALSE);
	SAFE_FREE(rgColAccess);
	return hr;
}

		
////////////////////////////////////////////////////////////////////////
//	CRowObject::VerifyGetColumns
//
////////////////////////////////////////////////////////////////////////
BOOL CRowObject::VerifyGetColumns
(
	DBCOUNTITEM			iRow,
	CSchema*			pSchema,
	DWORD				dwColsToBind,			
	BLOBTYPE			dwBlobType,
	ECOLUMNORDER		eBindingOrder,		
	ECOLS_BY_REF		eColsByRef,				
	DBTYPE				dwModifier,
	DBORDINAL			cColsToBind,
	DBORDINAL*			rgColsToBind,
	DBPART				dwPart
)
{
	HRESULT hr = S_OK;
	DBORDINAL cColAccess = 0;
	DBCOLUMNACCESS* rgColAccess = NULL;
	void* pData = NULL;
	BOOL bCompare = FALSE;

	//Create the ColAccess Structures...
	TESTC_(hr = CreateColAccess(&cColAccess, &rgColAccess, &pData, NULL, 
		dwColsToBind, dwBlobType, eBindingOrder, eColsByRef, dwModifier, 
		cColsToBind, rgColsToBind, dwPart),S_OK);
	
	//IRow::GetColumns
	TESTC_(hr = GetColumns(cColAccess, rgColAccess),S_OK);

	//Compare Data for this row object
	bCompare = CompareColAccess(cColAccess, rgColAccess, iRow, pSchema);

CLEANUP:
	FreeColAccess(cColAccess, rgColAccess);
	SAFE_FREE(pData);
	return bCompare;
}

	
////////////////////////////////////////////////////////////////////////
//	CRowObject::SetColumns
//
////////////////////////////////////////////////////////////////////////
HRESULT CRowObject::SetColumns(DBORDINAL cColAccess, DBCOLUMNACCESS* rgColAccess)
{
	ASSERT(m_pIRow);
	HRESULT hr = S_OK;
	IRowChange* pIRowChange = NULL;
	
	//Obtain the IRowChange interface
	QTESTC(VerifyInterface(m_pIRow, IID_IRowChange, ROW_INTERFACE, (IUnknown**)&pIRowChange));

	//IRow::SetColumns (SetData)
	hr = pIRowChange->SetColumns(cColAccess, rgColAccess);

	//Display any binding errors and status'
	TESTC(VerifyColAccess(hr, cColAccess, rgColAccess));

CLEANUP:
	SAFE_RELEASE(pIRowChange);
	return hr;
}



////////////////////////////////////////////////////////////////////////
//	CRowObject::SetColumns
//
////////////////////////////////////////////////////////////////////////
HRESULT CRowObject::SetColumns(DBCOUNTITEM cBindings, DBBINDING* rgBindings, void* pData)
{
	DBORDINAL cColAccess = 0;
	DBCOLUMNACCESS* rgColAccess = NULL;
	HRESULT hr = E_FAIL;
	DBORDINAL iCol = 0;

	//Convert bindings to ColAccess
	QTESTC_(hr = BindingsToColAccess(cBindings, rgBindings, pData, &cColAccess, &rgColAccess),S_OK);
		
	//IRowChange::SetColumns (SetData)
	hr = SetColumns(cColAccess, rgColAccess);

	//Update the Status, Length values...
	QTESTC_(UpdateStatusLength(cColAccess, rgColAccess, cBindings, rgBindings, pData),S_OK);
	
CLEANUP:
	//Free ColAccess.
	//NOTE:  We don't use FreeColAccess, since it frees the outofline data as well
	//and we are handing back the outofline data through the accessor...
	for(DBORDINAL cCol=0; cCol< cColAccess; cCol++)
		ReleaseDBID(&rgColAccess[cCol].columnid, FALSE);
	SAFE_FREE(rgColAccess);
	return hr;
}



////////////////////////////////////////////////////////////////////////////
//  CRowObject::VerifySetColumns
//
////////////////////////////////////////////////////////////////////////////
BOOL CRowObject::VerifySetColumns
(
	DBCOUNTITEM			iRow,
	CTable*				pCTable,
	DWORD				dwColsToBind,			
	BLOBTYPE			dwBlobType,
	ECOLUMNORDER		eBindingOrder,		
	ECOLS_BY_REF		eColsByRef,				
	DBTYPE				dwModifier,
	DBORDINAL			cColsToBind,
	DBORDINAL*			rgColsToBind,
	DBPART				dwPart
)
{
	TBEGIN
	HRESULT hr = S_OK;
	DBORDINAL cColAccess = 0;
	DBCOLUMNACCESS* rgColAccess = NULL;
	void* pData = NULL;
	BOOL bCompare = FALSE;

	//Create the ColAccess Structures...
	TESTC_(hr = CreateColAccess(&cColAccess, &rgColAccess, &pData, NULL, dwColsToBind, dwBlobType, eBindingOrder, eColsByRef, dwModifier, cColsToBind, rgColsToBind, dwPart),S_OK);
	
	//Create the Data for SetColumns
	TESTC_(hr = FillColAccess(pCTable, cColAccess, rgColAccess, iRow),S_OK);

	//IRowChange::SetColumns
	TESTC_(hr = SetColumns(cColAccess, rgColAccess),S_OK);

	//IRow::GetColumns
	TESTC_(hr = GetColumns(cColAccess, rgColAccess),S_OK);

	//Compare Data for this row object
	bCompare = CompareColAccess(cColAccess, rgColAccess, iRow, pCTable);

CLEANUP:
	FreeColAccess(cColAccess, rgColAccess);
	SAFE_FREE(pData);
	return bCompare;
}


////////////////////////////////////////////////////////////////////////
//	CRowObject::FillColAccess
//
////////////////////////////////////////////////////////////////////////
HRESULT CRowObject::FillColAccess(CSchema* pSchema, DBORDINAL cColAccess, DBCOLUMNACCESS* rgColAccess, DBCOUNTITEM iRow, EVALUE eValue, DWORD dwColsToBind)
{
	HRESULT hr = S_OK;
	DBCOUNTITEM cBindings = 0;
	DBBINDING* rgBindings = NULL;
	void* pData = NULL;


	//Obtain the Bindings from the ColAccess structures...
	TESTC_(hr = ColAccessToBindings(cColAccess, rgColAccess, &cBindings, &rgBindings, &pData),S_OK);

	//Fill Bindings, similar to the first row
	TESTC_(hr = FillInputBindings(pSchema, DBACCESSOR_ROWDATA, cBindings, rgBindings,
		(BYTE**)&pData, iRow, 0, NULL, eValue, m_pIRow, dwColsToBind),S_OK);

CLEANUP:	
	FreeAccessorBindings(cBindings, rgBindings);
	return hr;
}


////////////////////////////////////////////////////////////////////////
//	CRowObject::UpdateStatusLength
//
////////////////////////////////////////////////////////////////////////
HRESULT CRowObject::UpdateStatusLength
(
	DBORDINAL		cColumns,		//@parm [in]:	the count of columns
	DBCOLUMNACCESS* rgColAccess,	//@parm [in]:	the array of column access structs
	DBCOUNTITEM		cBindings,		//@parm [in]:	the count of bindings
	DBBINDING*		rgBindings,		//@parm [in]:	the array of bindings
	void*			pData			//@parm [in]:	pData Buffer
)
{
	//This could be extended to allow different number of cColumns than bindings,
	//but for the current logic it must be an identical match...
	ASSERT(cColumns == cBindings);
	
	//Since the data offset is the same, we only need to update the Status, Length bindings...
	for(DBORDINAL iCol=0; iCol<cBindings; iCol++)
	{
		//Status
		if(STATUS_IS_BOUND(rgBindings[iCol]))
			STATUS_BINDING(rgBindings[iCol], pData) = rgColAccess[iCol].dwStatus;
		
		//Length
		if(LENGTH_IS_BOUND(rgBindings[iCol]))
			LENGTH_BINDING(rgBindings[iCol], pData) = rgColAccess[iCol].cbDataLen;
	}

	return S_OK;
}


////////////////////////////////////////////////////////////////////////
//	CRowObject::CreateColAccess
//
////////////////////////////////////////////////////////////////////////
HRESULT CRowObject::CreateColAccess(
		DBORDINAL*			pcColAccess,			// @parm [OUT] Count of ColAccess
		DBCOLUMNACCESS**	prgColAccess,			// @parm [OUT] Array of ColAccess
		void**				ppData,					// @parm [OUT] pData
		DBLENGTH*			pcbRowSize,				// @parm [OUT] length of a row	
		DWORD				dwColsToBind,			// @parm [IN]  Which columns will be used in the bindings
		BLOBTYPE			dwBlobType,				// @paramopt [IN] how to bind BLOB Columns
		ECOLUMNORDER		eBindingOrder,			// @parm [IN]  Order to bind columns in accessor												
		ECOLS_BY_REF		eColsByRef,				// @parm [IN]  Which columns to bind by reference (fixed, variable, all or none)
		DBTYPE				dwModifier,				// @parm [IN] Modifier to be OR'd with each binding type.
													// Note, this modifies each binding of the accessor and is in
													// addition to anything done by eColsByRef.  Default is DBTYPE_EMPTY,
													// which means no modifier will be used, except as specified by eColsByRef.
		DBORDINAL			cColsToBind,			// @parm [IN]  Used only if dwColsToBind = USE_COLS_TO_BIND_ARRAY
													// specifies the number of elements in rgColsToBind array
		DBORDINAL*			rgColsToBind,			// @parm [IN]  Used only if dwColsToBind = USE_COLS_TO_BIND_ARRAY												 
													// Each element in the array specifies the iNumber of a column to be 
													// bound.  This iNumber should be the same as that returned by iOrdinalsInfo for each column.
													// This iNumber is always ordered the same as the column list in the command 
													// specification, thus the user can use the command specification col list
													// order to determine the appropriate iNumber.
		DBPART				dwPart					// @parm [IN]  Types of binding to do (Value, Status, and/or Length)	
		
)
{
	ASSERT(ppData);
	
	HRESULT hr = S_OK;
	DBCOUNTITEM cBindings = 0;
	DBBINDING* rgBindings = NULL;
	DBLENGTH cbRowSize = 0;

	//Create "Default" ColAccess bindings for all the data...
	QTESTC_(hr = GetAccessorAndBindings(m_pIRow, DBACCESSOR_ROWDATA, NULL,
		&rgBindings, &cBindings, &cbRowSize, dwPart, dwColsToBind, eBindingOrder, eColsByRef, 
		NULL, NULL, NULL, dwModifier, cColsToBind, (DB_LORDINAL*)rgColsToBind,
		NULL, NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, dwBlobType),S_OK);

	//Allocate memory for the bindings...
	if(*ppData == NULL)
	{
		SAFE_ALLOC(*ppData, BYTE, cbRowSize);
		memset(*ppData, 0, (size_t)cbRowSize);
	}

	//Now create the ColAccess Structs...
	QTESTC_(hr = BindingsToColAccess(cBindings, rgBindings, *ppData, pcColAccess, prgColAccess),S_OK);
	
CLEANUP:
	if(pcbRowSize)
		*pcbRowSize = cbRowSize;
	FreeAccessorBindings(cBindings, rgBindings);
	return hr;
}

		

////////////////////////////////////////////////////////////////////////
//	CRowObject::CreateColAccessUsingFilter
//
////////////////////////////////////////////////////////////////////////
HRESULT CRowObject::CreateColAccessUsingFilter(
		DBORDINAL*			pcColAccess,			// @parm [OUT] Count of ColAccess
		DBCOLUMNACCESS**	prgColAccess,			// @parm [OUT] Array of ColAccess
		void**				ppData,					// @parm [OUT] pData
		CSchema				*pSchema,				// @parm [IN]  use the column names in this schema as filter
		DBLENGTH*			pcbRowSize				// @parm [OUT] length of a row	
)
{
	HRESULT			hr = E_FAIL;
	WCHAR			*pStringsBuffer	= NULL;
	DBORDINAL		i;
	DBORDINAL		cColsToBind		= 0;
	DBORDINAL		*rgColsToBind	= NULL;
	DBORDINAL		cColInfo		= 0;
	DBCOLUMNINFO	*rgColInfo		= NULL;
	CCol			col;

	// get info about the columns of the row
	TESTC_(hr = m_pIColumnsInfo->GetColumnInfo(&cColInfo, &rgColInfo, &pStringsBuffer), S_OK);

	// build the list of the columns to be built by CreateColAccess (ordinals)
	SAFE_ALLOC(rgColsToBind, DBORDINAL, cColInfo);
	for (i=0; i<cColInfo; i++)
	{
		if (S_OK == pSchema->GetColInfo(&rgColInfo[i].columnid, col))
		{
			// add the ordinal of this column in the row to the list
			rgColsToBind[cColsToBind++] = rgColInfo[i].iOrdinal;
		}
	}

	TESTC_(hr = CreateColAccess(pcColAccess, prgColAccess, ppData, pcbRowSize,
		USE_COLS_TO_BIND_ARRAY,	// use cColsToBind and rgColsToBind to bind cols
		NO_BLOB_COLS, FORWARD, NO_COLS_BY_REF, DBTYPE_EMPTY,
		cColsToBind, rgColsToBind), S_OK);

CLEANUP:
	SAFE_FREE(pStringsBuffer);
	SAFE_FREE(rgColInfo);
	SAFE_FREE(rgColsToBind);
	return hr;
} //CRowObject::CreateColAccessUsingFilter




////////////////////////////////////////////////////////////////////////
//	CRowObject::GetColumnInfo
//
////////////////////////////////////////////////////////////////////////
HRESULT CRowObject::GetColumnInfo(
		DBORDINAL*			pcColumnInfo,			// @parm [OUT] Count of Columns
		DBCOLUMNINFO**		prgColumnInfo,			// @parm [OUT] Array of ColumnInfo
		WCHAR**				ppStringBuffer
)
{
	ASSERT(m_pIColumnsInfo);
	HRESULT hr = S_OK;
	DBORDINAL cColumnInfo = 0;
	DBCOLUMNINFO* rgColumnInfo = NULL;
	WCHAR* pStringBuffer = NULL;

	//IColumnsInfo::GetColumnInfo
	TESTC_(hr = m_pIColumnsInfo->GetColumnInfo(&cColumnInfo, &rgColumnInfo, &pStringBuffer),S_OK);
	
	//May times we just want the flags or something and don't need the overhead
	//of the string buffer.  So if not requesting the string buffer, make sure
	//we null the column names, so they aren't incorrectly referenced...
	if(ppStringBuffer == NULL)
	{
		for(ULONG iCol=0; iCol<cColumnInfo; iCol++)
			rgColumnInfo[iCol].pwszName = NULL;
	}

CLEANUP:
	if(pcColumnInfo)
		*pcColumnInfo = cColumnInfo;
	if(ppStringBuffer)
		*ppStringBuffer = pStringBuffer;
	else
		SAFE_FREE(pStringBuffer);
	if(prgColumnInfo)
		*prgColumnInfo = rgColumnInfo;
	else 
		SAFE_FREE(rgColumnInfo);
	return hr;
}


////////////////////////////////////////////////////////////////////////
//	CRowObject::GetExtraColumnInfo
//
////////////////////////////////////////////////////////////////////////
HRESULT CRowObject::GetExtraColumnInfo(
		DBORDINAL*			pcColumnInfo,			// @parm [OUT] Count of Columns
		DBCOLUMNINFO**		prgColumnInfo,			// @parm [OUT] Array of ColumnInfo
		WCHAR**				ppStringBuffer,
		DBORDINAL**			prgColOrdinals
)
{
	//NOTE:  The spec doesn't make it to easy (effiecent) to determine what are extra (row object) 
	//columns and what are (common) rowset columns.  Currently our function GetExtraColumnInfo 
	//obtains the IColumnInfo off the rowset, which only contains rowset common columns.  And then we
	//call IColumnInfo off the row object, and removes all the common columns, producing just the extra
	//columns.  Not to efficent!
	
	ASSERT(m_pIRow);
	HRESULT hr = S_OK;
	DBORDINAL cColumnInfo = 0;
	DBCOLUMNINFO* rgColumnInfo = NULL;
	WCHAR* pStringBuffer = NULL;

	DBORDINAL i,cRowsetColumnInfo = 0;
	DBCOLUMNINFO* rgRowsetColumnInfo = NULL;
	IColumnsInfo* pIColumnsInfo = NULL;
	WCHAR* pRowsetStringBuffer = NULL;

	//Get the (Parent) rowset...
	//GetSourceRowset could fail if the row object doesn't have a parent rowset object...
	QTESTC_(hr = m_pIRow->GetSourceRowset(IID_IColumnsInfo, (IUnknown**)&pIColumnsInfo, NULL),S_OK);
	TESTC_(hr = pIColumnsInfo->GetColumnInfo(&cRowsetColumnInfo, &rgRowsetColumnInfo, &pRowsetStringBuffer),S_OK);
	
	TESTC_(hr = m_pIColumnsInfo->GetColumnInfo(&cColumnInfo, &rgColumnInfo, &pStringBuffer),S_OK);
	cColumnInfo = (cColumnInfo - cRowsetColumnInfo);

	//Row Columns are supposed to be a superset to the rowset columns...	
	if(cColumnInfo)
	{
		memmove(&rgColumnInfo[0], &rgColumnInfo[cRowsetColumnInfo], (size_t)(cColumnInfo* sizeof(DBCOLUMNINFO)));
	}

	//Create ordinal array of extra columns...
	if(prgColOrdinals)
	{
		SAFE_ALLOC(*prgColOrdinals, DBORDINAL, cColumnInfo);
		for(i=0; i<cColumnInfo; i++)
			(*prgColOrdinals)[i] = rgColumnInfo[i].iOrdinal;
	}
	
CLEANUP:
	if(pcColumnInfo)
		*pcColumnInfo = cColumnInfo;
	if(ppStringBuffer)
		*ppStringBuffer = pStringBuffer;
	else
		SAFE_FREE(pStringBuffer);
	if(prgColumnInfo)
		*prgColumnInfo = rgColumnInfo;
	else 
		SAFE_FREE(rgColumnInfo);
	SAFE_RELEASE(pIColumnsInfo);
	SAFE_FREE(rgRowsetColumnInfo);
	SAFE_FREE(pRowsetStringBuffer);
	return hr;
}



////////////////////////////////////////////////////////////////////////
//	CRowObject::IsExtraColumn
//
////////////////////////////////////////////////////////////////////////
BOOL CRowObject::IsExtraColumn(DBID* pColumnID)
{
	ASSERT(pColumnID);
	HRESULT hr = S_OK;
	
	DBORDINAL cColumnInfo = 0;
	DBCOLUMNINFO* rgColumnInfo = NULL;
	WCHAR* pStringBuffer = NULL;
	DBORDINAL i=0;

	//Obtain the ExtraColumnInfo
	TESTC_(hr = GetExtraColumnInfo(&cColumnInfo, &rgColumnInfo, &pStringBuffer),S_OK);
	
	//Loop through the Row Object ColumnInfo, seeing if this ColumnID is one of the extra columns.
	for(i=0; i<cColumnInfo; i++)
	{
		if(CompareDBID(*pColumnID, rgColumnInfo[i].columnid))
			return TRUE;
	}

	
CLEANUP:
	SAFE_FREE(pStringBuffer);
	SAFE_FREE(rgColumnInfo);
	return FALSE;
}


////////////////////////////////////////////////////////////////////////
//	CRowObject::BindingsToColAccess
//
////////////////////////////////////////////////////////////////////////
HRESULT CRowObject::BindingsToColAccess(
	DBCOUNTITEM			cBindings,		//@parm [in]:	the count of bindings
	DBBINDING*			rgBindings,		//@parm [in]:	the array of bindings
	void*				pData,			//@parm [in]:	pData buffer
	DBORDINAL*			pcColumns,		//@parm [out]:	the count of columns
	DBCOLUMNACCESS**	prgColAccess	//@parm [out]:	the array of column access structs
)
{
	DBORDINAL iCol				= 0;
	DBORDINAL cColumns			= 0;
	DBCOLUMNACCESS* rgColAccess	= NULL;

	DBORDINAL cColInfos = 0;
	DBCOLUMNINFO* rgColInfos = NULL;
	WCHAR* pwszStringBuffer = NULL;
	HRESULT hr = S_OK;
	
	//No-op
	if(cBindings == 0)
		goto CLEANUP;
	
	//Since the 2.5 spec introduces DBCOLUMNACCESS a new structure for getting and setting
	//data which is different from pData offsets and bindings, we need to compare data
	//for these new structures.  We don't want to duplicate code by having routines
	//operate on pData buffers, and similar code dealing with simplier data buffers...

	//So we will keep all our throughly debugged data comparision routines, and just convert
	//the new DBCOLUMNACCESS structure into the larger "superset" bingings with pdata buffer...

	//Allocate our COLUMNACCESS array
	SAFE_ALLOC(rgColAccess, DBCOLUMNACCESS, cBindings)

	//Obtain IColumnsInfo interface, so we can convert between the iOrdinal and columnID
	TESTC_(hr = m_pIColumnsInfo->GetColumnInfo(&cColInfos, &rgColInfos, &pwszStringBuffer),S_OK);

	//Now just loop through the columns and convert to ColumnAccess structs...
	for(iCol=0; iCol<cBindings; iCol++)
	{
		//Try to find the ColumnID for this ordinal.  (opposite of MapColumnIDs)
		BOOL bFound = FALSE;
		for(DBORDINAL i=0; i<cColInfos && !bFound; i++)
		{
			if(rgColInfos[i].iOrdinal == rgBindings[iCol].iOrdinal)
			{
				TESTC_(hr = DuplicateDBID(rgColInfos[i].columnid, &rgColAccess[cColumns].columnid),S_OK);
				bFound = TRUE;
			}
		}
		
		if(!bFound)
		{
			hr = DB_E_BADORDINAL;
			goto CLEANUP;
		}

		//VALUE
		rgColAccess[cColumns].pData		= NULL;
		if(VALUE_IS_BOUND(rgBindings[iCol]))
			rgColAccess[cColumns].pData = &VALUE_BINDING(rgBindings[iCol], pData);
		
		//STATUS
		rgColAccess[cColumns].dwStatus	= DBSTATUS_S_OK;
		if(STATUS_IS_BOUND(rgBindings[iCol]))
			rgColAccess[cColumns].dwStatus = STATUS_BINDING(rgBindings[iCol], pData);

		//LENGTH
		rgColAccess[cColumns].cbDataLen	= 0;
		if(LENGTH_IS_BOUND(rgBindings[iCol]))
			rgColAccess[cColumns].cbDataLen = LENGTH_BINDING(rgBindings[iCol], pData);
		
		rgColAccess[cColumns].wType		= rgBindings[iCol].wType;
		rgColAccess[cColumns].bPrecision= rgBindings[iCol].bPrecision;
		rgColAccess[cColumns].bScale	= rgBindings[iCol].bScale;
		rgColAccess[cColumns].cbMaxLen	= rgBindings[iCol].cbMaxLen;
		cColumns++;
	}

CLEANUP:
	*pcColumns = cColumns;
	*prgColAccess = rgColAccess;
	SAFE_FREE(rgColInfos);
	SAFE_FREE(pwszStringBuffer);
	return hr;
}



////////////////////////////////////////////////////////////////////////
//	CRowObject::ColAccessToBindings
//
////////////////////////////////////////////////////////////////////////
HRESULT CRowObject::ColAccessToBindings(
	DBORDINAL		cColumns,		//@parm [in]:	the count of columns
	DBCOLUMNACCESS* rgColAccess,	//@parm [in]:	the array of column access structs
	DBCOUNTITEM*	pcBindings,		//@parm [out]:	the count of bindings
	DBBINDING**		prgBindings,	//@parm [out]:	the array of bindings
	void**			ppData			//@parm [out]:	pData Buffer
)
{
	DBORDINAL iCol				= 0;
	DBCOUNTITEM cBindings			= 0;
	DBBINDING* rgBindings	= NULL;
	HRESULT hr = E_FAIL;

	//Since the 2.5 spec introduces DBCOLUMNACCESS a new structure for getting and setting
	//data which is different from pData offsets and bindings, we need to compare data
	//for these new structures.  We don't want to duplicate code by having routines
	//operate on pData buffers, and similar code dealing with simplier data buffers...

	//So we will keep all our throughly debugged data comparision routines, and just convert
	//the new DBCOLUMNACCESS structure into the larger "superset" bingings with pdata buffer...

	//Allocate our bindings
	SAFE_ALLOC(rgBindings, DBBINDING, cColumns);

	//Now just loop through the columns and convert to binding structs...
	for(iCol=0; iCol<cColumns; iCol++)
	{
		//Obtain the Ordinal for this column...
		TESTC_(m_pIColumnsInfo->MapColumnIDs(1, &rgColAccess[iCol].columnid, &rgBindings[cBindings].iOrdinal),S_OK);

		rgBindings[cBindings].obStatus	= (BYTE*)&rgColAccess[iCol].dwStatus - (BYTE*)100;
		rgBindings[cBindings].obLength	= (BYTE*)&rgColAccess[iCol].cbDataLen - (BYTE*)100;
		rgBindings[cBindings].obValue	= (BYTE*)rgColAccess[iCol].pData - (BYTE*)100;

		rgBindings[cBindings].pTypeInfo	= NULL;
		rgBindings[cBindings].pObject	= NULL;
		rgBindings[cBindings].pBindExt	= NULL;

		//STATUS and LENGTH are always bound, but pData==NULL indicates VALUE is not bound
		rgBindings[cBindings].dwPart	= DBPART_LENGTH|DBPART_STATUS;
		if(rgColAccess[iCol].pData)
			rgBindings[cBindings].dwPart |= DBPART_VALUE;

		rgBindings[cBindings].dwMemOwner= DBMEMOWNER_CLIENTOWNED;
		rgBindings[cBindings].eParamIO	= DBPARAMIO_NOTPARAM;
		
		rgBindings[cBindings].cbMaxLen	= rgColAccess[iCol].cbMaxLen;
		rgBindings[cBindings].dwFlags	= 0;
		rgBindings[cBindings].wType		= rgColAccess[iCol].wType;

		rgBindings[cBindings].bPrecision= rgColAccess[iCol].bPrecision;
		rgBindings[cBindings].bScale	= rgColAccess[iCol].bScale;
		cBindings++;
	}

	hr = S_OK;

CLEANUP:
	*pcBindings		= cBindings;
	*prgBindings	= rgBindings;
	*ppData			= (void*)100;
	return hr;
}



////////////////////////////////////////////////////////////////////////
//	CRowObject::CompareColAccess
//
////////////////////////////////////////////////////////////////////////
BOOL CRowObject::CompareColAccess(
	DBORDINAL		cColAccess,
	DBCOLUMNACCESS*	rgColAccess,
	DBCOUNTITEM		iRow,			//@parm[in]:	the row number of the data at the backend table
	CSchema*		pSchema,		//@parm[in]:	The pointer to CSchema object from which the 
	EVALUE			eValue			//@parm[in]:	whether use PRIMARY or SECONDARY to make a data
)
{
	BOOL bCompare = FALSE;
	DBCOUNTITEM cBindings = 0;
	DBBINDING* rgBindings = NULL;
	void* pData = NULL;

	//Since the 2.5 spec introduces DBCOLUMNACCESS a new structure for getting and setting
	//data which is different from pData offsets and bindings, we need to compare data
	//for these new structures.  We don't want to duplicate code by having routines
	//operate on pData buffers, and similar code dealing with simplier data buffers...

	//So we will keep all our throughly debugged data comparision routines, and just convert
	//the new DBCOLUMNACCESS structure into the larger "superset" bingings with pdata buffer...
	
	//Convert the Column Access into Bindings...
	TESTC_(ColAccessToBindings(cColAccess, rgColAccess, &cBindings, &rgBindings, &pData),S_OK);

	//Now delegate to our real data comparision function for Bindings...
	bCompare = CompareData(0, NULL, iRow, pData, cBindings, rgBindings, 
					pSchema, NULL, eValue, COMPARE_ONLY);

CLEANUP:
	FreeAccessorBindings(cBindings, rgBindings);
	return bCompare;
}


////////////////////////////////////////////////////////////////////////
//	CRowObject::CompareColBuffer
//
////////////////////////////////////////////////////////////////////////
BOOL CRowObject::CompareColBuffer
(
	DBORDINAL		cGetColAccess,
	DBCOLUMNACCESS*	rgGetColAccess,
	DBORDINAL		cSetColAccess,
	DBCOLUMNACCESS*	rgSetColAccess,
	BOOL			fSetData
)
{
	BOOL bCompare = FALSE;
	DBCOUNTITEM cGetBindings = 0;
	DBBINDING* rgGetBindings = NULL;
	void* pGetData = NULL;
	DBCOUNTITEM cSetBindings = 0;
	DBBINDING* rgSetBindings = NULL;
	void* pSetData = NULL;

	//Since the 2.5 spec introduces DBCOLUMNACCESS a new structure for getting and setting
	//data which is different from pData offsets and bindings, we need to compare data
	//for these new structures.  We don't want to duplicate code by having routines
	//operate on pData buffers, and similar code dealing with simplier data buffers...

	//So we will keep all our throughly debugged data comparision routines, and just convert
	//the new DBCOLUMNACCESS structure into the larger "superset" bingings with pdata buffer...
	
	//Convert the Column Access into Bindings...
	TESTC_(ColAccessToBindings(cGetColAccess, rgGetColAccess, &cGetBindings, &rgGetBindings, &pGetData),S_OK);
	TESTC_(ColAccessToBindings(cSetColAccess, rgSetColAccess, &cSetBindings, &rgSetBindings, &pSetData),S_OK);

	//Now delegate to our real data comparision function for Bindings...
	bCompare = CompareBuffer(pGetData, pSetData, cGetBindings, rgGetBindings, NULL, fSetData, 
					FALSE, COMPARE_ONLY, FALSE,	cSetBindings, rgSetBindings);

CLEANUP:
	//NOTE:  Don't release pData (FALSE), since its just a pointer to our rgColAccess struct
	ReleaseInputBindingsMemory(cGetBindings, rgGetBindings, (BYTE*)pGetData, FALSE);
	ReleaseInputBindingsMemory(cSetBindings, rgSetBindings, (BYTE*)pSetData, FALSE);
	FreeAccessorBindings(cGetBindings, rgGetBindings);
	FreeAccessorBindings(cSetBindings, rgSetBindings);
	return bCompare;
}

////////////////////////////////////////////////////////////////////////////
//  CRowObject::GetResRowsetData
//
////////////////////////////////////////////////////////////////////////////
BOOL CRowObject::GetResRowsetData(DBID colid, void**ppData)
{
	TBEGIN
	DBLENGTH			cbRowSize = 0;
	DBORDINAL			cColsToBind = 1;
	DBORDINAL			rgColsToBind[1];
	DBORDINAL			cColAccess = 0;
	DBCOLUMNACCESS*		rgColAccess = NULL;
	DBID				dbid1 = DBROWCOL_PARSENAME; //First Column
	DBID				dbid2 = DBROWCOL_ISROOT;	//Last Column

	TESTC(ppData != NULL)

	if( (colid.eKind != DBKIND_GUID_PROPID) ||
		(colid.uGuid.guid != dbid1.uGuid.guid) ||
		(colid.uName.ulPropid<dbid1.uName.ulPropid)	||
		(colid.uName.ulPropid>dbid2.uName.ulPropid) )
	{
		odtLog<<L"GetResRowsetData() is to be used for Resource Rowset Columns only.\n";
		return FALSE;
	}

	//Get the ordinal of the resource rowset column assuming that
	//DBROWCOL_PARSENAME has ordinal 1.
	rgColsToBind[0] = colid.uName.ulPropid - dbid1.uName.ulPropid + 1 ;

	TESTC_(CreateColAccess(&cColAccess, &rgColAccess, ppData, &cbRowSize, 
		USE_COLS_TO_BIND_ARRAY, NO_BLOB_COLS, FORWARD, NO_COLS_BY_REF,
		DBTYPE_EMPTY, cColsToBind, rgColsToBind), S_OK)

	TESTC(cColAccess==1 && rgColAccess!=NULL)

	TESTC_(GetColumns(cColAccess, rgColAccess), S_OK)

	TESTC(ppData!=NULL && *ppData!=NULL)

CLEANUP:
	for(DBORDINAL cCol=0; cCol< cColAccess; cCol++)
		ReleaseDBID(&rgColAccess[cCol].columnid, FALSE);
	SAFE_FREE(rgColAccess);
	TRETURN
}

////////////////////////////////////////////////////////////////////////////
//  CRowObject::GetSourceRowset
//
////////////////////////////////////////////////////////////////////////////
HRESULT	CRowObject::GetSourceRowset(REFIID riid, IUnknown** ppIRowset, HROW* phRow)
{
	//IRow::GetSourceRowset
	HRESULT hr = m_pIRow->GetSourceRowset(riid, ppIRowset, phRow);
	
	//Do some postprocessing
	if(SUCCEEDED(hr))
	{
		if(ppIRowset)
		{
			TESTC(*ppIRowset != NULL);
			TESTC(DefaultObjectTesting(*ppIRowset, ROWSET_INTERFACE));
		}
		if(phRow)
		{
			TESTC(*phRow != DB_NULL_HROW);
		}
	}
	else
	{
		if(ppIRowset)
		{
			TESTC(*ppIRowset == NULL);
		}
		if(phRow)
		{
			TESTC(*phRow == DB_NULL_HROW);
		}
	}

CLEANUP:
	return hr;
}

	

////////////////////////////////////////////////////////////////////////////
//  CRowObject::Open
//
////////////////////////////////////////////////////////////////////////////
HRESULT CRowObject::Open
(
	IUnknown*			pIUnkOuter,
	const DBID*			pColumnID,
	REFGUID				rGuidType,
	REFIID				riid,
	IUnknown**			ppIUnknown
)
{
	//Make sure this is NULLed on error
	IUnknown* pIUnknown = INVALID(IUnknown*);

	//Open wil mainly only be able to be called for columns containing objects.
	//But some providers might be able to open streams, or other types of objects ontop
	//of non-object valued columns.
	HRESULT hr = pIRow()->Open(pIUnkOuter, (DBID*)pColumnID, rGuidType, 0, riid, &pIUnknown);
	if(SUCCEEDED(hr))
	{
		//Do some default testing of the object returned...
		if(!DefaultObjectTesting(pIUnknown))
		{
			hr = E_FAIL;
			goto CLEANUP;
		}

		//Issue an error if this object is the same as the parent row object
		//This would be a "ciruclar" reference, so any applciation trying to "display" all the
		//data and children would loop forever.
		if(VerifyEqualInterface(m_pIRow, pIUnknown))
			TERROR("Circular reference - IRow::Open returned itself...");
	}
	else
	{
		TESTC(pIUnknown == NULL);

		//Only Allow no aggregation if using a controlling unknown
		if(hr == DB_E_NOAGGREGATION)
			TWARNING("IRow::Open Aggregation not supported?");
	}
				
CLEANUP:
	if(pIUnknown == INVALID(IUnknown*))
		pIUnknown = NULL;
	if(ppIUnknown)
		*ppIUnknown = pIUnknown;
	else 
		SAFE_RELEASE(pIUnknown);
	return hr;
}


////////////////////////////////////////////////////////////////////////////
//  CRowObject::VerifyOpen
//
////////////////////////////////////////////////////////////////////////////
HRESULT CRowObject::VerifyOpen
(
	DBCOUNTITEM			iRow,
	CSchema*			pSchema,
	IUnknown*			pIUnkOuter,
	const DBID*			pColumnID,
	REFGUID				rGuidType,
	REFIID				riid,
	IUnknown**			ppIUnknown
)
{
	TBEGIN
	ASSERT(pSchema);
	IUnknown* pIUnknown = NULL;

	//Delegate
	HRESULT hr = Open(pIUnkOuter, pColumnID, rGuidType, riid, &pIUnknown);
	if(SUCCEEDED(hr) || hr==DB_E_NOTFOUND)
	{
		DBSTATUS dbStatus = DBSTATUS_S_OK;
		if(hr==DB_E_NOTFOUND)
			dbStatus = DBSTATUS_S_ISNULL;

		//The simplest way to verify the data is to build a ColAccess structure
		//ontop of the stream and delegate to our other CompareColAccess function
		DBCOLUMNACCESS dbColAccess;
		memset(&dbColAccess, 0, sizeof(DBCOLUMNACCESS));
		dbColAccess.pData		= &pIUnknown; 
		dbColAccess.columnid	= *pColumnID;
		dbColAccess.cbDataLen	= sizeof(IUnknown*);
		dbColAccess.dwStatus	= dbStatus;
		dbColAccess.wType		= DBTYPE_IUNKNOWN;

		//Delegate
		TESTC(CompareColAccess(1, &dbColAccess, iRow, pSchema));
	}
				
CLEANUP:
	if(ppIUnknown)
		*ppIUnknown = pIUnknown;
	else 
		SAFE_RELEASE(pIUnknown);
	return hr;
}


////////////////////////////////////////////////////////////////////////////
//  CRowObject::GetSession
//
////////////////////////////////////////////////////////////////////////////
HRESULT CRowObject::GetSession
(
	REFIID				riid,
	IUnknown**			ppSession
)
{
	HRESULT	hrRet = E_FAIL;

	HRESULT	hr = pIGetSession()->GetSession(riid, ppSession);

	if(S_OK==hr)
		TESTC(ppSession && (*ppSession != NULL))
	else if(DB_E_NOSOURCEOBJECT==hr)
		COMPARE(ppSession && (!*ppSession), TRUE);

	hrRet = hr;			
CLEANUP:
	return hrRet;
}


////////////////////////////////////////////////////////////////////////////
//  CRowObject::ExecuteCommand
//
////////////////////////////////////////////////////////////////////////////
HRESULT CRowObject::ExecuteCommand
(
	EQUERY				eQuery,					// [IN]  Query to create 
	REFIID				riid,					// [IN]  Interface pointer to return
	ULONG				cPropSets,				// [IN]  Count of property sets.
	DBPROPSET*			rgPropSets,				// [IN]  Array of DBPROPSET structures.
	IUnknown **			ppIRowset				// [OUT] Pointer to the rowset pointer.
)
{
	HRESULT				hr = DB_E_NOTSUPPORTED;
	DBROWCOUNT			cRowsAffected = 0;
	WCHAR*				pwszScopedCommand = NULL;
	ICommandText*		pICommandText = NULL;
	ICommandProperties*	pICommandProperties = NULL;
	IUnknown*			pIRowset = NULL;

	// Row scoped commands are optional
	TESTC(NULL != m_pIDBCreateCommand);

	pwszScopedCommand = FetchRowScopedQuery(eQuery);
	TESTC(pwszScopedCommand != NULL);

	// Create a command object ourselves
	TESTC_(hr = m_pIDBCreateCommand->CreateCommand(NULL,IID_ICommandText,
			(IUnknown**)&pICommandText),S_OK);

	// Only SetProperties, if there are properties to set
	if (cPropSets)
	{
		if (!VerifyInterface(pICommandText, IID_ICommandProperties, COMMAND_INTERFACE, (IUnknown**)&pICommandProperties))
		{
			hr = E_NOINTERFACE;
			goto CLEANUP;
		}

		// SetProperties may return DB_S/DB_E depending wieither 1 or all properties
		// were not settable.  Return if any property is not settable, since we
		// don't want to create a rowset without the required properties
		TESTC_(hr = pICommandProperties->SetProperties(cPropSets,rgPropSets), S_OK);

		//Use CursorEngine ( if requested )
		if(GetModInfo()->UseServiceComponents() & SERVICECOMP_CURSORENGINE)
			hr = SetRowsetProperty(pICommandText, DBPROPSET_ROWSET, DBPROP_CLIENTCURSOR, TRUE, DBPROPOPTIONS_REQUIRED, FALSE);
	}

	// Set the row scoped query
	TESTC_(hr = pICommandText->SetCommandText(DBGUID_DEFAULT, pwszScopedCommand),S_OK);
	
		// Execute the SQL Statement
	if(FAILED(hr = pICommandText->Execute(
			NULL,				// [IN]		Aggregates
			riid,				// [IN]		REFIID
			NULL,				// [IN/OUT] DBPARAMS
			&cRowsAffected,		// [OUT]	Count of rows affected
			&pIRowset)))		// [IN/OUT] Memory to put rowset interface
		goto CLEANUP;


CLEANUP:
	PROVIDER_FREE(pwszScopedCommand);
	
	SAFE_RELEASE(pICommandText);
	SAFE_RELEASE(pICommandProperties);

	// If user wanted rowset interface, pass it back
	if (ppIRowset)
		*ppIRowset = pIRowset;
	else
		SAFE_RELEASE(pIRowset);

	return hr;
}

