// Start of code for IRowsetChange.h
// File: IRowsetChange.h
//
//  This file contains the information for the IMyRowsetChangeImpl class.
//
//	IMyRowsetChangeImpl inherits from IRowsetChange and does the following:
//
//		1. IMyRowsetChangeImpl :: SetData - Performs immediate updates of rowset data 
//		2. IMyRowsetChangeImpl :: CreateRow - Inserts a new default row in the Data Array
//		3. IMyRowsetChangeImpl :: CreateHRow - Inserts a new row in the Row Handles Array
//		4. IMyRowsetChangeImpl :: InsertRow - Performs immediate inserts of rowset data
//		5. IMyRowsetChangeImpl :: DeleteRows - Performs immediate deletes of rowset data
//

// Class IMyRowsetChangeImpl
#ifndef _ROWSETCHANGE_C822BFE1_C6A1_11d2_AC47_00C04F8DB3D5_H
#define _ROWSETCHANGE_C822BFE1_C6A1_11d2_AC47_00C04F8DB3D5_H

#include "CRow.h"

template <class T, class Storage, class RowClass = CSimpleRow>
class ATL_NO_VTABLE IMyRowsetChangeImpl:  public IRowsetChange
{
public:

//		IMyRowsetChangeImpl::SetData( )...
//      Performs Immediate Updates 

	STDMETHOD (SetData)( HROW  hRow,HACCESSOR hAccessor,  void __RPC_FAR *pData ) 
	{
		ATLTRACE2(atlTraceDBProvider, 0, "IMyRowsetChangeImpl::SetData\n");
		HRESULT hr;
		T* pT = (T*) this;
		// Putting a Lock on the object in the scope of this member function
		T::ObjectLock cab((T*) this);

		// A. Validation
		// Check the Row Handle
		if(NULL==hRow)
		{
			ATLTRACE2(atlTraceDBProvider, 0, _T("SetData: Bad hRow\n"));
			return DB_E_BADROWHANDLE; 
		}
		// Check the Accessor Handle
		if(NULL==hAccessor)
		{
			ATLTRACE2(atlTraceDBProvider, 0, _T("SetData : Bad hAccessor\n"));
			return DB_E_BADACCESSORHANDLE; 
		}
		// Check the Data pointer
		if(NULL== pData)
		{
			ATLTRACE2(atlTraceDBProvider, 0, _T("SetData : Data Pointer was NULL\n"));
			return E_INVALIDARG; 
		}
		// Getting the Row structure from the row handle
		// B. Instantiation
		// 1. Instantiate the T::_HRowClass pointer and get the pointer to the row in m_rgRowHandles from the Row handle...
		// m_rgHandles --- for the row handles
		T::_HRowClass *pHRow = pT->m_rgRowHandles.Lookup((T::_HRowClass::KeyType)hRow)->m_value;
		if(!pHRow)
		{
			ATLTRACE2(atlTraceDBProvider, 0, _T("SetData : Could not get Accessor\n"));
			return DB_E_BADROWHANDLE; 
		}
		CRow* pRow = pT->m_DBFile.m_Rows[pHRow->m_iRowset];
		if(!pRow)
		{
			ATLTRACE2(atlTraceDBProvider, 0, _T("SetData : Could not get Accessor\n"));
			return DB_E_BADROWHANDLE; 
		}
		// 2. Instantiate the Bindings Accessor pointer and get the pointer to the row in m_rgBindings from the Accessor handle...
		// m_rgBindings for the Row Bindings oftype ATLBINDINGS or _BindType
		T::_BindType *pAccessor = pT->m_rgBindings.Lookup(hAccessor)->m_value;
		if(!pAccessor)
		{
			ATLTRACE2(atlTraceDBProvider, 0, _T("SetData : Could not get Accessor\n"));
			return DB_E_BADACCESSORHANDLE; 
		}
		// Now all the pointers are obtained ...
		// C. Updating 
		// Check for a ROW accessor
		if(!(pAccessor->dwAccessorFlags & DBACCESSOR_ROWDATA))
		{
			ATLTRACE2(atlTraceDBProvider, 0, _T("SetData : Not a Row Accessor\n"));
			return DB_E_BADACCESSORHANDLE; 
		}
		// Get pointer to the Row Data from the Data buffer for the handle from m_rgRowData of type _RowsetArrayType
		// Pointer returned is a Storage Class pointer...
		// The index lies in m_iRowset in IRowset
		//The data is taken out as a BYTE array

		// Get the column info about the Destination rowset i.e. in the Provider Buffer !!!
		ATLCOLUMNINFO* pColInfo;  // Column Information structure 
		DBORDINAL cCols;  // # of cols.
		pColInfo = pT->GetColumnInfo((T*) this,&cCols); 
		// D. Transfer data into the Data Buffer for Updation
		DBCOUNTITEM cBindings = pAccessor->cBindings; // Total # of Bindings
		DBBINDING *pBinding = pAccessor->pBindings; // The DBBINDINGS structure pointer...
		HRESULT hrEvent;

		for(DBCOUNTITEM iBind =0; iBind < cBindings; iBind++)
		{
			DBBINDING *pBindCur = &pBinding[iBind];
			DBORDINAL iCol;
			// To ensure one or zero Bindings per Column
			for(iCol=0; iCol < cCols && pBindCur->iOrdinal != pColInfo[iCol].iOrdinal; iCol++)
				; 
			if(iCol == cCols)
			{
				return DB_E_BADORDINAL;
			}
			ATLCOLUMNINFO* pColCur = &pColInfo[iCol];
			if (FAILED(hrEvent = pT->Fire_OnFieldChangeMy((T*)this,  hRow, 1,&iCol, DBREASON_COLUMN_SET, DBEVENTPHASE_OKTODO, TRUE)))
			{
				ATLTRACE2(atlTraceDBProvider, 0, "Failed to Fire DBREASON_COLUMN_SET in DBEVENTPHASE_OKTODO phase\n");
				if(hrEvent==S_FALSE)
					return hrEvent;
			}
			if(FAILED(hrEvent = pT->Fire_OnFieldChangeMy((T*)this,  hRow, 1,&iCol, DBREASON_COLUMN_SET, DBEVENTPHASE_ABOUTTODO, TRUE)))
				ATLTRACE2(atlTraceDBProvider, 0, "Failed to Fire DBREASON_COLUMN_SET in DBEVENTPHASE_ABOUTTODO phase\n");
			// 1. Build up the 12 parameters to IConvertType::DataConvert(  ) 
			// 1. Types x 2
			DBTYPE wSrcType = pBindCur->wType;
			DBTYPE wDstType = pColCur->wType;
			// 2. Lengths x 2
			//DANK: Changed the following line as it was calculating the dwSrcLength incorrectly. This caused a problem
			//      in rowsetviewer when I change the last record, last column value from 300 to 400. IDataConvert would 
			//      fail with errors occurred
			DBLENGTH dwSrcLength = *((DBLENGTH *)((BYTE*)pData + pBindCur->obLength));
			//ULONG dwDstLength = dwSrcLength;  // [in]/[out]
			DBLENGTH dwDstLength = pColCur->ulColumnSize;  // [in]/[out]
			// 3. Data ptrs or Offsets from the base ptrs - pRowData (Dst) and pDstData x 2
			BYTE *pSrc = (BYTE*)((BYTE*)pData + pBindCur->obValue);   // obValue contains the offset...
			BYTE *pDst = (BYTE*) ((BYTE*) pRow->m_pbProxyData + pColCur->cbOffset);  
			// 4. Dst Max Length
			DBLENGTH dwDstMaxLen = pColCur->ulColumnSize;  
			// 5. Status x 2
			DBSTATUS dwSrcStatus = *(DBSTATUS*)((BYTE*)pData +pBindCur->obStatus);
			DBSTATUS dwDstStatus; 
			// 6. Precision -	pBindCur->bPrecision;
			// 7. Scale -	pBindCur->bScale;
			// 8. Convert Flags - DATACONVERT_SETDATABEHAVIOUR
			T* pT = (T*) this;
			// Check on DBMEMBER_PROVIDEROWNED or DBMEMBER_PROVIDEROWNED 
			BOOL bProvOwn = pBindCur->dwMemOwner == DBMEMOWNER_PROVIDEROWNED;
			bProvOwn;
			// Initialize the Destination Memory to all 0's to remove the junk seen after the string data
			//memset(pDst,0,dwDstMaxLen);
			if(bProvOwn && pColCur->wType == pBindCur->wType)
				// If Provider owned and the types are the same then return the Src pointer to consumer
				pDst =  (BYTE*)((BYTE*)pData + pBindCur->obValue); 
			else
			{
				// The memory is to be freed by consumer or types differ
				if(FAILED( hr = pT->m_spConvert->DataConvert(
					wSrcType,											//* [in] */ DBTYPE wSrcType,
					wDstType,											//* [in] */ DBTYPE wDstType,
					dwSrcLength,										//* [in] */ ULONG cbSrcLength,
					&dwDstLength,										//* [out][in] */ ULONG __RPC_FAR *pcbDstLength,
					pSrc,												//* [in] */ void __RPC_FAR *pSrc,
					pDst,												//* [out] */ void __RPC_FAR *pDst,
					dwDstMaxLen,										//* [in] */ ULONG cbDstMaxLength,
					dwSrcStatus,										//* [in] */ DBSTATUS dwSrcStatus,
					&dwDstStatus,										//* [out] */ DBSTATUS __RPC_FAR *pdbsStatus,
					pBindCur->bPrecision,	 							//* [in] */ BYTE bPrecision,
					pBindCur->bScale,									//* [in] */ BYTE bScale,
					DBDATACONVERT_SETDATABEHAVIOR						//* [in] */ DBDATACONVERT dwFlags) = 0;
					)))
				{
					//TODO: Need to handle failure
					if(FAILED(hrEvent = pT->Fire_OnFieldChangeMy((T*)this,  hRow, 1,&iCol, DBREASON_COLUMN_SET, DBEVENTPHASE_FAILEDTODO, TRUE)))
						ATLTRACE2(atlTraceDBProvider, 0, "Failed to Fire DBREASON_COLUMN_SET in DBEVENTPHASE_FAILEDTODO phase\n");
					return hr;
				}
			}
			// 2. Write the data to the Storage
			if( FAILED(hr=pT->m_DBFile.UpdateRowImmediate(pRow, pT->m_prgColInfo, pT->m_cCols)) )
			{
				ATLTRACE2(atlTraceDBProvider, 0, _T("SetData : Unsuccessful in writing data to storage\n"));
				// Clean-up the Rows of the Data Buffer
				if(FAILED(hrEvent = pT->Fire_OnFieldChangeMy((T*)this,  hRow, 1,&iCol, DBREASON_COLUMN_SET, DBEVENTPHASE_FAILEDTODO, TRUE)))
					ATLTRACE2(atlTraceDBProvider, 0, "Failed to Fire DBREASON_COLUMN_SET in DBEVENTPHASE_FAILEDTODO phase\n");
				return hr;
			}

			if(FAILED(pT->Fire_OnFieldChangeMy((T*)this, hRow, 1, &iCol, DBREASON_COLUMN_SET, DBEVENTPHASE_SYNCHAFTER, TRUE)))
				ATLTRACE2(atlTraceDBProvider, 0, "Failed to Fire DBREASON_COLUMN_SET in DBEVENTPHASE_SYNCHAFTER phase\n");

			if(FAILED(pT->Fire_OnFieldChangeMy((T*)this, hRow, 1, &iCol, DBREASON_COLUMN_SET, DBEVENTPHASE_DIDEVENT, TRUE)))
				ATLTRACE2(atlTraceDBProvider, 0, "Failed to Fire DBREASON_COLUMN_SET in DBEVENTPHASE_DIDEVENT phase\n");

			if(FAILED(pT->Fire_OnFieldChangeMy((T*)this, hRow, 1, &iCol, DBREASON_COLUMN_RECALCULATED, DBEVENTPHASE_DIDEVENT, TRUE)))
				ATLTRACE2(atlTraceDBProvider, 0, "Failed to Fire DBREASON_COLUMN_SET in DBEVENTPHASE_DIDEVENT phase\n");
		}
		return S_OK;
	}

	// IRowsetImpl::CreateHRow( )
	// Helper function to create a new entry in the m_rgRowHandles
	HRESULT CreateHRow(DBBYTEOFFSET lRowsOffset, HROW *phRow)
	{
		T* pT = (T*) this;
		T::_HRowClass* pRow = NULL;
		ATLASSERT(lRowsOffset >= 0);
		T::_HRowClass::KeyType key = lRowsOffset+1;
		ATLASSERT(key > 0);
		
		CAtlMap<RowClass::KeyType, RowClass*>::CPair* pPair = pT->m_rgRowHandles.Lookup(key);
		if (pPair)
			pRow = pPair->m_value;
		if (pRow == NULL)
		{
			ATLTRY(pRow = new T::_HRowClass(lRowsOffset))  // Copy constructor
				if (pRow == NULL)
					return E_OUTOFMEMORY;
			if (!pT->m_rgRowHandles.SetAt(key, pRow))
				return E_OUTOFMEMORY;
		}
		// Doing the AddRef here
		pRow->AddRefRow();
		pT->m_bReset = false;
		if(phRow != NULL)
			*phRow = (HROW)key;

		return S_OK;
	}

	// IRowsetImpl::CreateHRow( )
	// Helper function to create a new entry in the Data array to store a default row
	HRESULT CreateRecord(DBROWCOUNT &cRowsObtained, HROW *phRow)
	{
		ATLTRACE2(atlTraceDBProvider, 0, "IMyRowsetChangeImpl::CreateRecord\n");
		HRESULT hr = S_OK;
		T* pT = (T*) this;

		CRow* pRow = new CRow();  // A New blank row inserted into the MM file
		if(pRow == NULL)
			return E_OUTOFMEMORY;

		// Need to Set the BookMark appropriately after creating the Row
		// Get the last Bookmark
		size_t tmpSize = pT->m_DBFile.m_Rows.GetCount();
		ULONG newBkMrk;
		if (0 != tmpSize)
			newBkMrk = pT->m_DBFile.m_Rows[tmpSize-1]->m_bmk + 1;
		else  // When there are 0 records
			newBkMrk = 1;

		// Create new Row in the Row Array 
		ATLCOLUMNINFO* pColInfo;  // Column Information structure 
		DBORDINAL cCols;  // # of cols.
		pColInfo =pT-> GetColumnInfo( pT,&cCols); 
		if (!pT->m_DBFile.AddRow(pRow, pColInfo, cCols)) // This will add the CRow object to the file and initialize it
		{
			ATLTRACE2(atlTraceDBProvider, 0, _T("InsertRow : Unsuccessful in creating a Row\n"));
			// The Cleanup 
			phRow = NULL;
			if(FAILED(pT->Fire_OnRowChangeMy((T*)this, 1, phRow, DBREASON_ROW_INSERT, DBEVENTPHASE_FAILEDTODO, TRUE)))
				ATLTRACE2(atlTraceDBProvider, 0, "Failed to Fire DBREASON_ROW_INSERT in DBEVENTPHASE_FAILEDTODO phase\n");
			return E_FAIL;
		}

		// Set the new BookMark
		if(newBkMrk)
			pT->m_DBFile.m_Rows[tmpSize]->m_bmk= newBkMrk;

		// Add the data to the File
		if( FAILED(hr = pT->m_DBFile.InsertRowImmediate(pRow, pColInfo, cCols,false)) )
		{
			ATLTRACE2(atlTraceDBProvider, 0, _T("InsertRow : Unsuccessful in converting data\n"));
			// Clean-up the Rows of the Data Buffer
			for (size_t i=0; i<pT->m_DBFile.m_Rows.GetCount(); ++i)
			{
				if (pT->m_DBFile.m_Rows[i] == pRow)
					pT->m_DBFile.m_Rows.RemoveAt(i);
			}
			pT->m_iRowset--;
			phRow = NULL;
			if(FAILED(pT->Fire_OnRowChangeMy((T*)this, 1, phRow, DBREASON_ROW_INSERT, DBEVENTPHASE_FAILEDTODO, TRUE)))
				ATLTRACE2(atlTraceDBProvider, 0, "Failed to Fire DBREASON_ROW_INSERT in DBEVENTPHASE_FAILEDTODO phase\n");
			return hr;
		}

		//Add the Row into the m_rgRowHandles
		if((hr=pT->CreateHRow(pT->m_DBFile.m_Rows.GetCount() - 1, phRow)) != S_OK)
		{
			ATLTRACE2(atlTraceDBProvider, 0, _T("InsertRow : Unsuccessful in creating a Row Handle\n"));
			// The Cleanup 
			phRow = NULL;
			if(FAILED(pT->Fire_OnRowChangeMy((T*)this, 1, phRow, DBREASON_ROW_INSERT, DBEVENTPHASE_FAILEDTODO, TRUE)))
				ATLTRACE2(atlTraceDBProvider, 0, "Failed to Fire DBREASON_ROW_INSERT in DBEVENTPHASE_FAILEDTODO phase\n");
			return hr;
		}

		if(FAILED(pT->Fire_OnRowChangeMy((T*)this, 1, phRow, DBREASON_ROW_INSERT, DBEVENTPHASE_SYNCHAFTER, TRUE)))
			ATLTRACE2(atlTraceDBProvider, 0, "Failed to Fire DBREASON_ROW_INSERT in DBEVENTPHASE_SYNCHAFTER phase\n");

		if(FAILED(pT->Fire_OnRowChangeMy((T*)this, 1, phRow, DBREASON_ROW_INSERT, DBEVENTPHASE_DIDEVENT, TRUE)))
			ATLTRACE2(atlTraceDBProvider, 0, "Failed to Fire DBREASON_ROW_INSERT in DBEVENTPHASE_DIDEVENT phase\n");

		return S_OK;
	}


	//		IRowsetImpl::InsertRow( )
	//     Performing Immediate Inserts...

	STDMETHOD (InsertRow)( 
		/* [in] */ HCHAPTER hReserved,
		/* [in] */ HACCESSOR hAccessor,
		/* [in] */ void __RPC_FAR *pData,
		/* [out] */ HROW __RPC_FAR *phRow) 
	{
		ATLTRACE2(atlTraceDBProvider, 0, "IMyRowsetChangeImpl::InsertRow\n");
		HRESULT hr;
		T* pT = (T*) this;
		// Putting a Lock on the object in the scope of this member function
		T::ObjectLock cab((T*) this);

		HRESULT hrEvent = S_OK;
		ULONG cRows = 1;

		// A. Validate

		// Chapters not supported
		if(DB_NULL_HCHAPTER != hReserved)
			return DB_E_NOTSUPPORTED;

		if (FAILED(hrEvent = pT->Fire_OnRowChangeMy((T*)this, cRows, phRow, DBREASON_ROW_INSERT, DBEVENTPHASE_OKTODO, TRUE)))
		{
			ATLTRACE2(atlTraceDBProvider, 0, "Failed to Fire DBREASON_ROW_INSERT in DBEVENTPHASE_OKTODO phase\n");
			if(hrEvent==S_FALSE)
				return hrEvent;
		}
		// Check the Accessor Handle
		if(NULL==hAccessor)
		{
			ATLTRACE2(atlTraceDBProvider, 0, _T("InsertRow : Bad hAccessor\n"));
			return DB_E_BADACCESSORHANDLE; 
		}
		// Check the Data pointer
		if(NULL== pData)
		{
			ATLTRACE2(atlTraceDBProvider, 0, _T("InsertRow : Data Pointer was NULL\n"));
			DBROWCOUNT cRowsObtained=0;
			if(FAILED(pT->Fire_OnRowChangeMy((T*)this, cRows, phRow, DBREASON_ROW_INSERT, DBEVENTPHASE_ABOUTTODO, TRUE)))
				ATLTRACE2(atlTraceDBProvider, 0, "Failed to Fire DBREASON_ROW_INSERT in DBEVENTPHASE_ABOUTTODO phase\n");
			// Create a new record if the pData pointer is NULL
			// The ADO Data Grid asks for a NULL or Default row during insertions
			return CreateRecord(cRowsObtained, phRow);
		}

		// B. Instantiation

		// 1. Instantiate the Bindings Accessor pointer and get the pointer to the row in m_rgBindings from the Accessor handle...
		// m_rgBindings for the Row Bindings oftype ATLBINDINGS or _BindType
		T::_BindType *pAccessor = pT->m_rgBindings.Lookup(hAccessor)->m_value;
		if(!pAccessor)
		{
			ATLTRACE2(atlTraceDBProvider, 0, _T("InsertRow : Could not get Accessor\n"));
			return DB_E_BADACCESSORHANDLE; 
		}

		//C. Preparing for Insertion

		// Check for a ROW accessor
		if(!(pAccessor->dwAccessorFlags & DBACCESSOR_ROWDATA))
		{
			ATLTRACE2(atlTraceDBProvider, 0, _T("InsertRow : Not a Row Accessor\n"));
			return DB_E_BADACCESSORHANDLE; 
		}

		// Create a new row in the Row Array
		CRow * pRow = new CRow;
		ATLCOLUMNINFO* pColInfo;  // Column Information structure 
		DBORDINAL cCols;  // # of cols.
		pColInfo =pT-> GetColumnInfo( pT,&cCols); 


		// Need to Set the BookMark appropriately after creating the Row
		// Get the last Bookmark
		size_t tmpSize = pT->m_DBFile.m_Rows.GetCount();
		DWORD newBkMrk;
		if (0 != tmpSize)
			newBkMrk = pT->m_DBFile.m_Rows[tmpSize-1]->m_bmk + 1;
		else  // When there are 0 records
			newBkMrk = 1;

		// This will add the CRow object to the file and initialize it
		if (!pT->m_DBFile.AddRow(pRow, pColInfo, cCols))
		{
			ATLTRACE2(atlTraceDBProvider, 0, _T("InsertRow : Unsuccessful in creating a Row\n"));
			// The Cleanup 
			phRow = NULL;
			if(FAILED(pT->Fire_OnRowChangeMy((T*)this, 1, phRow, DBREASON_ROW_INSERT, DBEVENTPHASE_FAILEDTODO, TRUE)))
				ATLTRACE2(atlTraceDBProvider, 0, "Failed to Fire DBREASON_ROW_INSERT in DBEVENTPHASE_FAILEDTODO phase\n");
			return E_FAIL;
		}

		// Assign proxy buffer to the destination pointer
		BYTE * pRowData = pRow->m_pbProxyData;

		if(FAILED(pT->Fire_OnRowChangeMy((T*)this, cRows, phRow, DBREASON_ROW_INSERT, DBEVENTPHASE_ABOUTTODO, TRUE)))
			ATLTRACE2(atlTraceDBProvider, 0, "Failed to Fire DBREASON_ROW_INSERT in DBEVENTPHASE_ABOUTTODO phase\n");

		//D. Inserting the Row

		DBCOUNTITEM cBindings = pAccessor->cBindings; // Total # of Bindings
		DBBINDING *pBinding = pAccessor->pBindings; // The DBBINDINGS structure pointer...
		for(DBCOUNTITEM iBind =0; iBind < cBindings; iBind++)
		{
			DBBINDING *pBindCur = &pBinding[iBind];
			// To ensure one or zero Bindings per Column
			DBORDINAL iCol;
			for(iCol=0; iCol < cCols && pBindCur->iOrdinal != pColInfo[iCol].iOrdinal; iCol++); 
			if(iCol == cCols)
			{
				if(FAILED(pT->Fire_OnRowChangeMy((T*)this, cRows, phRow, DBREASON_ROW_INSERT, DBEVENTPHASE_FAILEDTODO, TRUE)))
					ATLTRACE2(atlTraceDBProvider, 0, "Failed to Fire DBREASON_ROW_INSERT in DBEVENTPHASE_FAILEDTODO phase\n");
				return DB_E_BADORDINAL;
			}
			ATLCOLUMNINFO* pColCur = &pColInfo[iCol];
			// Build up the 12 parameters to IConvertType::DataConvert(  ) 
			// 1. Types x 2
			DBTYPE wSrcType = pBindCur->wType;
			DBTYPE wDstType = pColCur->wType;
			// 2. Lengths x 2
			DBLENGTH dwSrcLength = *((DBLENGTH *)((BYTE*)pData + pBindCur->obLength));
			DBLENGTH dwDstLength = pColCur->ulColumnSize;  // [in]/[out]
			// 3. Data ptrs or Offsets from the base ptrs - pRowData (Dst) and pDstData x 2
			BYTE *pSrc = (BYTE*)((BYTE*)pData + pBindCur->obValue);   // obValue contains the offset...
			BYTE *pDst = (BYTE*) ((BYTE*) pRowData + pColCur->cbOffset);  
			// 4. Dst Max Length
			DBLENGTH dwDstMaxLen = pColCur->ulColumnSize;  
			// 5. Status x 2
			DBSTATUS dwSrcStatus = *(DBSTATUS*)((BYTE*)pData+pBindCur->obStatus);
			DWORD dwDstStatus; 
			// 6. Precision -	pBindCur->bPrecision;
			// 7. Scale -	pBindCur->bScale;
			// 8. Convert Flags - DATACONVERT_SETDATABEHAVIOUR
			// Check on DBMEMBER_PROVIDEROWNED or DBMEMBER_PROVIDEROWNED 
			BOOL bProvOwn = pBindCur->dwMemOwner == DBMEMOWNER_PROVIDEROWNED;
			bProvOwn;

			if(bProvOwn && pColCur->wType == pBindCur->wType)
				// If Provider owned and the types are the same then return the Src pointer to consumer
				pDst =  (BYTE*)((BYTE*)pData + pBindCur->obValue); 
			else
			{
				// The memory is to be freed by consumer or types differ
				if(FAILED( hr = pT->m_spConvert->DataConvert(
					wSrcType,													//* [in] */ DBTYPE wSrcType,
					wDstType,													//* [in] */ DBTYPE wDstType,
					dwSrcLength,											//* [in] */ ULONG cbSrcLength,
					&dwDstLength,											//* [out][in] */ ULONG __RPC_FAR *pcbDstLength,
					pSrc,															//* [in] */ void __RPC_FAR *pSrc,
					pDst,															//* [out] */ void __RPC_FAR *pDst,
					dwDstMaxLen,										//* [in] */ ULONG cbDstMaxLength,
					dwSrcStatus,											///* [in] */ DBSTATUS dbsSrcStatus,
					&dwDstStatus,											//* [out] */ DBSTATUS __RPC_FAR *pdbsStatus,
					pBindCur->bPrecision,	 							//* [in] */ BYTE bPrecision,
					pBindCur->bScale,										//* [in] */ BYTE bScale,
					DBDATACONVERT_SETDATABEHAVIOR //* [in] */ DBDATACONVERT dwFlags) = 0;
					)))
				{
					ATLTRACE2(atlTraceDBProvider, 0, _T("InsertRow : Unsuccessful in converting data\n"));
					// The Cleanup...
					pT->m_iRowset--;
					phRow = NULL;
					if(FAILED(pT->Fire_OnRowChangeMy((T*)this, cRows, phRow, DBREASON_ROW_INSERT, DBEVENTPHASE_FAILEDTODO, TRUE)))
						ATLTRACE2(atlTraceDBProvider, 0, "Failed to Fire DBREASON_ROW_INSERT in DBEVENTPHASE_FAILEDTODO phase\n");
					return hr | E_FAIL;
				}
			}
		}

		// Set the new BookMark
		if(newBkMrk)
			pT->m_DBFile.m_Rows[tmpSize]->m_bmk= newBkMrk;

		// Insert the data into the Storage from 
		//now that the data has been transfer to the proxy buffer, store to file
		if( FAILED(hr = pT->m_DBFile.InsertRowImmediate(pRow, pColInfo, cCols,true)) )
		{
			ATLTRACE2(atlTraceDBProvider, 0, _T("InsertRow : Unsuccessful in converting data\n"));
			// Clean-up the Rows of the Data Buffer
			for (size_t i=0; i<pT->m_DBFile.m_Rows.GetCount(); ++i)
			{
				if (pT->m_DBFile.m_Rows[i] == pRow)
					pT->m_DBFile.m_Rows.RemoveAt(i);
			}
			pT->m_iRowset--;
			phRow = NULL;
			if(FAILED(pT->Fire_OnRowChangeMy((T*)this, cRows, phRow, DBREASON_ROW_INSERT, DBEVENTPHASE_FAILEDTODO, TRUE)))
				ATLTRACE2(atlTraceDBProvider, 0, "Failed to Fire DBREASON_ROW_INSERT in DBEVENTPHASE_FAILEDTODO phase\n");
			return hr;
		}

		//Add the Row into the m_rgRowHandles
		if((hr=pT->CreateHRow(pT->m_DBFile.m_Rows.GetCount() - 1, phRow)) != S_OK)
		{
			ATLTRACE2(atlTraceDBProvider, 0, _T("InsertRow : Unsuccessful in creating a Row Handle\n"));
			// Clean-up the Rows of the Data Buffer
			for (size_t i=0; i<pT->m_DBFile.m_Rows.GetCount(); ++i)
			{
				if (pT->m_DBFile.m_Rows[i] == pRow)
					pT->m_DBFile.m_Rows.RemoveAt(i);
			}
			// The Cleanup 
			pT->m_iRowset--;
			phRow = NULL;
			if(FAILED(pT->Fire_OnRowChangeMy((T*)this, 1, phRow, DBREASON_ROW_INSERT, DBEVENTPHASE_FAILEDTODO, TRUE)))
				ATLTRACE2(atlTraceDBProvider, 0, "Failed to Fire DBREASON_ROW_INSERT in DBEVENTPHASE_FAILEDTODO phase\n");
			return hr;
		}

		if(FAILED(pT->Fire_OnRowChangeMy((T*)this, cRows, phRow, DBREASON_ROW_INSERT, DBEVENTPHASE_SYNCHAFTER, TRUE)))
			ATLTRACE2(atlTraceDBProvider, 0, "Failed to Fire DBREASON_ROW_INSERT in DBEVENTPHASE_SYNCHAFTER phase\n");

		if(FAILED(pT->Fire_OnRowChangeMy((T*)this, cRows, phRow, DBREASON_ROW_INSERT, DBEVENTPHASE_DIDEVENT, TRUE)))
			ATLTRACE2(atlTraceDBProvider, 0, "Failed to Fire DBREASON_ROW_INSERT in DBEVENTPHASE_DIDEVENT phase\n");

		return S_OK;
	}


	//		IRowsetImpl: DeleteRows( )
	//		Performing Immediate deletes

	STDMETHOD (DeleteRows)( 
		/* [in] */ HCHAPTER hReserved,
		/* [in] */ DBCOUNTITEM cRows,		// Total number of rows to be deleted 
		/* [size_is][in] */ const HROW __RPC_FAR rghRows[  ], //An array of handles of the rows to be deleted
		/* [size_is][out] */ DBROWSTATUS __RPC_FAR rgRowStatus[  ]) //An array with cRows elements in which to return values indicating the status of each row specified in rghRows
	{
		ATLTRACE2(atlTraceDBProvider, 0, "IMyRowsetChangeImpl::DeleteRows\n");
		T* pT = (T*) this;
		T::ObjectLock cab((T*) pT);
		HRESULT hr = S_OK;

		HRESULT hrEvent = S_OK;
		if(FAILED(pT->Fire_OnRowChangeMy((T*)this, cRows, rghRows, DBREASON_ROW_DELETE, DBEVENTPHASE_OKTODO, TRUE)))
		{
			ATLTRACE2(atlTraceDBProvider, 0, "Failed to Fire DBREASON_ROW_DELETE in DBEVENTPHASE_OKTODO phase\n");
			if(hrEvent==S_FALSE)
				return hrEvent;
		}
		// Putting a Lock on the object in the scope of this member function

		// A. Validation

		if(DB_NULL_HCHAPTER != hReserved)
		{
			ATLTRACE2(atlTraceDBProvider, 0, "IMyRowsetChangeImpl::DeleteRows: Chapters are not supported\n");
			return DB_E_NOTSUPPORTED;
		}

		if(cRows <= 0)
		{
			ATLTRACE2(atlTraceDBProvider, 0, "IMyRowsetChangeImpl::DeleteRows: Too few Rows\n");
			return E_INVALIDARG;
		}

		if(NULL == rghRows)
		{
			ATLTRACE2(atlTraceDBProvider, 0, "IMyRowsetChangeImpl::DeleteRows: Too few Row Handles\n");
			return E_INVALIDARG;
		}

		if(FAILED(pT->Fire_OnRowChangeMy((T*)this, cRows, rghRows, DBREASON_ROW_DELETE, DBEVENTPHASE_ABOUTTODO, TRUE)))
			ATLTRACE2(atlTraceDBProvider, 0, "Failed to Fire DBREASON_ROW_DELETE in DBEVENTPHASE_ABOUTTODO phase\n");

		// B. Deleting the rows
		// Delete Rows dealing only with the m_rgRowData
		// ADO client Release the rows appropriately 
		// The process of Deleting the rows 

		for(DBCOUNTITEM i=0; i<cRows; i++)
		{
			// Find the Row from the handle 
			T::_HRowClass *pHRow = pT->m_rgRowHandles.Lookup((T::_HRowClass::KeyType)rghRows[i])->m_value;
			if(!pHRow)
			{
				ATLTRACE2(atlTraceDBProvider, 0, _T("IMyRowsetChangeImpl::DeleteRows: Bad Row Handle\n"));
				if(rgRowStatus) rgRowStatus[i] = DBROWSTATUS_E_INVALID;
				if(FAILED(pT->Fire_OnRowChangeMy((T*)this, cRows, rghRows, DBREASON_ROW_DELETE, DBEVENTPHASE_FAILEDTODO, TRUE)))
					ATLTRACE2(atlTraceDBProvider, 0, "Failed to Fire DBREASON_ROW_DELETE in DBEVENTPHASE_FAILEDTODO phase\n");
				return DB_E_BADROWHANDLE; 
			}

			// The marking of the rows to be deleted is to be from the Buffer (m_rgRowData) itself 
			// 1. rghRows[i] - handle of therow to deleted
			// 2. phRow - CSimpleRow in m_rgRowHandles whose m_aKey = rghRows[i]
			// 3. pHRow->m_iRowset - index of the row in the Data array
			// 4. pT->m_DBFile.m_Rows - the Data array
			hr = pT->m_DBFile.DeleteRowImmediate(pT->m_DBFile.m_Rows[pHRow->m_iRowset]);  
			if (FAILED(hr))
			{
				ATLTRACE2(atlTraceDBProvider, 0, _T("IMyRowsetChangeImpl::DeleteRows: Unable to remove Row \n"));	
				if(rgRowStatus) rgRowStatus[i] = DBROWSTATUS_E_FAIL;
				if(FAILED(pT->Fire_OnRowChangeMy((T*)this, cRows, rghRows, DBREASON_ROW_DELETE, DBEVENTPHASE_FAILEDTODO, TRUE)))
					ATLTRACE2(atlTraceDBProvider, 0, "Failed to Fire DBREASON_ROW_DELETE in DBEVENTPHASE_FAILEDTODO phase\n");
				return DB_S_ERRORSOCCURRED; 
			}

			if (rgRowStatus) 
				rgRowStatus[i] = DBROWSTATUS_S_OK;

		} 

		if(FAILED(pT->Fire_OnRowChangeMy((T*)this, cRows, rghRows, DBREASON_ROW_DELETE, DBEVENTPHASE_SYNCHAFTER, TRUE)))
			ATLTRACE2(atlTraceDBProvider, 0, "Failed to Fire DBREASON_ROW_DELETE in DBEVENTPHASE_SYNCHAFTER phase\n");

		if(FAILED(pT->Fire_OnRowChangeMy((T*)this, cRows, rghRows, DBREASON_ROW_DELETE, DBEVENTPHASE_DIDEVENT, TRUE)))
			ATLTRACE2(atlTraceDBProvider, 0, "Failed to Fire DBREASON_ROW_DELETE in DBEVENTPHASE_DIDEVENT phase\n");

		return hr;
	}
};
#endif

// End of code for IRowsetChange.h
