// Start of IRowsetScroll.h
// File: IRowsetScroll.h
//
//  This file contains the information for the IRowsetScrollImpl class.
//
//	IRowsetScrollImpl inherits from IRowsetLocateImpl and IRowsetScroll and does the following:
//
//		1. IRowsetScrollImpl :: GetApproximatePosition - Gets the Approximate position of a row corresponding to a specified bookmark
//		2. IRowsetScrollImpl :: GetRowsAtRatio - Fetches rows starting from a fractional position in the rowset
//		3. IRowsetScrollImpl :: Compare - Compares two bookmark
//		4. IRowsetScrollImpl :: GetRowsAt - Fetches rows starting with the row specified by an offset from a bookmark
//		5. IRowsetScrollImpl :: GetRowsByBookmark - Fetches rows that match the specified bookmark
//		6. IRowsetScrollImpl :: Hash - Returns hash values for the specified bookmarks
//

// Class IRowsetScrollImpl

#ifndef _IROWSETSCROLLIMPL_7F4B3871_6B9C_11d3_AC94_00C04F8DB3D5_H
#define _IROWSETSCROLLIMPL_7F4B3871_6B9C_11d3_AC94_00C04F8DB3D5_H

#include "RNCP.h"  // The ConnectionPoint and IRowsetNotify Proxy class

template <class T,class RowsetInterface=IRowsetScroll>
class ATL_NO_VTABLE IRowsetScrollImpl : public IRowsetImpl<T, RowsetInterface>
{
public:
    STDMETHOD (GetApproximatePosition)( 
        /* [in] */ HCHAPTER hReserved,
        /* [in] */ DBBKMARK cbBookmark,
        /* [size_is][in] */ const BYTE __RPC_FAR *pBookmark,
        /* [out] */ DBCOUNTITEM __RPC_FAR *pulPosition,
        /* [out] */ DBCOUNTITEM __RPC_FAR *pcRows)
	{
		ATLTRACE2(atlTraceDBProvider, 0, "IRowsetScrollImpl::GetApproximatePosition\n");
		HRESULT hr;
		T* pT = (T*)this;
		T::ObjectLock cab((T*) pT);
		if(DB_NULL_HCHAPTER != hReserved)
			return DB_E_NOTSUPPORTED;

		// DANK: RowsetViewer was getting an error as it would use this function to get the 
		//       approximate number of rows. I modified to  do the arg checking earlier before the 
		//       bookmark validation
		if(pulPosition == NULL && pcRows == NULL)
			return S_OK;
		
		if(cbBookmark == 0)
		{
			if (*pcRows != NULL)
				*pcRows = pT->m_rgRowHandles.GetCount();
			return S_OK;
		}
		
		// SQLOLEDB does not have this implemented MDAC 2.1 SP2...
		// Validate the bookmarks only if a bookmark is specified
		// If No-op 
		hr = ValidateBookmark(cbBookmark, pBookmark);
		if(hr != S_OK)
			return hr;
		// This deals with all the ros inside the rowset after a GetNextRows( )...
		// Hence, operations are confined to m_rgRowHandles
		// There is no change to the state of the cursor just return the number of rows between m_iRowset and the current bookmark...
		// *pcRows should always be greater than *pulPosition !!
		
		// Now if cbBookmark != 0 and pBookmark is valid-> check pBookmark and find the ulPosition and pcRows
		if(pBookmark != NULL)
		{
			DBROWOFFSET iRowsetTemp = pT->m_iRowset;  // Cache the current rowset
			if ((cbBookmark == 1) && (*pBookmark == DBBMK_FIRST))
				iRowsetTemp = 1;
			if ((cbBookmark == 1) && (*pBookmark == DBBMK_LAST))
			//	iRowsetTemp = pT->m_rgRowData.GetSize();
				iRowsetTemp = pT->m_DBFile.m_Rows.GetCount();
			// m_iRowset - From current cursor get current Bookmark
			// Check if iRowsetTemp exists in the current current
			// Need to economize this if many rows exist in the cursor....

			if (NULL == m_rgRowHandles.Lookup(iRowsetTemp))
				return DB_E_BADBOOKMARK;

			//ULONG dwCurBkmrk = pT->m_rgRowData[iRowsetTemp].m_dwBookmark;
			ULONG dwCurBkmrk = pT->m_DBFile.m_Rows[iRowsetTemp-1]->m_bmk;
						
			if( (*(ULONG*)pBookmark < dwCurBkmrk) )
				return DB_E_BADBOOKMARK;
			DBBKMARK ulNumRows = *(ULONG*)pBookmark - dwCurBkmrk;
			*pulPosition = iRowsetTemp;
			*pcRows = ulNumRows;
		}
		else return DB_E_BADBOOKMARK;
		return S_OK;
	}
    
    STDMETHOD (GetRowsAtRatio)( 
        /* [in] */ HWATCHREGION hReserved1,
        /* [in] */ HCHAPTER hReserved2,
        /* [in] */ DBCOUNTITEM ulNumerator,
        /* [in] */ DBCOUNTITEM ulDenominator,
        /* [in] */ DBROWCOUNT cRows,
        /* [out] */ DBCOUNTITEM __RPC_FAR *pcRowsObtained,
        /* [size_is][size_is][out] */ HROW __RPC_FAR *__RPC_FAR *prghRows)
	{
		ATLTRACE2(atlTraceDBProvider, 0, "IRowsetScrollImpl::GetRowsAtRatio\n");
		T* pT = (T*)this;
		T::ObjectLock cab((T*) pT);
		HRESULT hr = S_OK;
		// Validation
		if(DB_NULL_HCHAPTER != hReserved2)
			return DB_E_NOTSUPPORTED;
		if(NULL == pcRowsObtained || NULL == prghRows)
			return E_INVALIDARG;
		// Check Fetching Backwards
		if (cRows < 0 && !pT->m_bCanFetchBack)
			return DB_E_CANTFETCHBACKWARDS;
		if(ulDenominator == 0 || (ulNumerator > ulDenominator) )
			return DB_E_BADRATIO;

		// Setting up the Start Position
		DBROWOFFSET iRowsetTemp = pT->m_iRowset;  // Cache the current rowset
		if(ulNumerator == 0 && cRows >0) // Start Point - First Row
			pT->m_iRowset = 0;
		else if ( (ulNumerator == 0 && cRows < 0) || (ulNumerator == ulDenominator && cRows > 0))   // DB_S_ENDOFROWSET
			hr = DB_S_ENDOFROWSET;
		else if (ulNumerator == ulDenominator && cRows < 0)  // Last Row
			//pT->m_iRowset = pT->m_rgRowData.GetSize() - 1;
			pT->m_iRowset = pT->m_DBFile.m_Rows.GetCount() - 1;
		else // All other conditions
			//pT->m_iRowset = (ulNumerator /ulDenominator) * pT->m_rgRowData.GetSize(); 
			pT->m_iRowset = (ulNumerator /ulDenominator) * pT->m_DBFile.m_Rows.GetCount();
		// Not sure about HOLDROWS optimizations - DB_E_ROWSNOTRELEASED 
		// Call IRowsetImpl::GetNextRows to actually get the rows.
		if(hr == S_OK)
			hr = pT->GetNextRows(hReserved2, 0, cRows, pcRowsObtained, prghRows);
		pT->m_iRowset = iRowsetTemp; // Return back to orignal cursor position
		// Send notification message
		if(FAILED(pT->Fire_OnRowChangeMy(this, cRows, *prghRows, DBREASON_ROW_ACTIVATE, DBEVENTPHASE_DIDEVENT, TRUE)))
			ATLTRACE2(atlTraceDBProvider, 0, "Failed to Fire DBREASON_ROW_ACTIVATE in DBEVENTPHASE_DIDEVENT phase\n");
		return hr;
	}

	// IROWSETLOCATE Methods
	STDMETHOD (Compare)(HCHAPTER hReserved, DBBKMARK cbBookmark1,
		const BYTE * pBookmark1, DBBKMARK cbBookmark2, const BYTE * pBookmark2,
		DBCOMPARE * pComparison)
	{
		ATLTRACE("IRowsetLocateImpl::Compare");
		T* pT = (T*)this;
		T::ObjectLock cab((T*) pT);
		if(DB_NULL_HCHAPTER != hReserved)
			return DB_E_NOTSUPPORTED;
		HRESULT hr = ValidateBookmark(cbBookmark1, pBookmark1);
		if (hr != S_OK)
			return hr;
		hr = ValidateBookmark(cbBookmark2, pBookmark2);
		if (hr != S_OK)
			return hr;

		// Return the value based on the bookmark values
		if (*pBookmark1 == *pBookmark2)
			*pComparison = DBCOMPARE_EQ;

		if (*pBookmark1 < *pBookmark2)
			*pComparison = DBCOMPARE_LT;

		if (*pBookmark1 > *pBookmark2)
			*pComparison = DBCOMPARE_GT;
		return S_OK;
	}

	STDMETHOD (GetRowsAt)(HWATCHREGION hReserved1, HCHAPTER hReserved2,
		DBBKMARK cbBookmark, const BYTE * pBookmark, DBROWOFFSET lRowsOffset,
		DBROWCOUNT cRows, DBCOUNTITEM * pcRowsObtained, HROW ** prghRows)
	{
		ATLTRACE("IRowsetScrollImpl::GetRowsAt\n");
		T* pT = (T*)this;
		T::ObjectLock cab((T*) pT);
		if(DB_NULL_HCHAPTER != hReserved2)
			return DB_E_NOTSUPPORTED;
		// Check bookmark
		HRESULT hr = ValidateBookmark(cbBookmark, pBookmark);
		if (hr != S_OK)
			return hr;

		// Check the other pointers
		if (pcRowsObtained == NULL || prghRows == NULL)
			return E_INVALIDARG;

		// Set the current row position to the bookmark.  Handle any
		// normal values

		// We need to handle the offset as the start position is defined
		// as the bookmark + offset.  If the offset is negative, and we
		// do not have m_bCanScrollBack then return an error.  The
		// GetNextRows function handles the case where cRows is negative
		// and we don't have m_bCanFetchBack set.
		if (lRowsOffset < 0 && !pT->m_bCanScrollBack)
			return DB_E_CANTSCROLLBACKWARDS;
		if (cRows < 0 && !pT->m_bCanFetchBack)
			return DB_E_CANTFETCHBACKWARDS;

		DBROWOFFSET iRowsetTemp = pT->m_iRowset;  // Cache the current rowset
		pT->m_iRowset = *pBookmark;
		if ((cbBookmark == 1) && (*pBookmark == DBBMK_FIRST))
			pT->m_iRowset = 1;

		if ((cbBookmark == 1) && (*pBookmark == DBBMK_LAST))
//			pT->m_iRowset = pT->m_rgRowData.GetSize();
			pT->m_iRowset = pT->m_DBFile.m_Rows.GetCount();

		// Set the start position to m_iRowset + lRowsOffset
		pT->m_iRowset += lRowsOffset;
		if (lRowsOffset >= 0)
			(cRows >= 0) ? pT->m_iRowset -= 1 : pT->m_iRowset +=0;
		else
			(cRows >= 0) ? pT->m_iRowset -= 1 : pT->m_iRowset +=0;
//      (lRowsOffset >= 0) ? m_iRowset -= 1 : m_iRowset += 1;
		// TODO: this is not very generic - perhaps there is something we can do here
		if (pT->m_iRowset < 0 || pT->m_iRowset > (DBROWOFFSET)pT->m_DBFile.m_Rows.GetCount())
		{
			pT->m_iRowset = iRowsetTemp;
			return DB_E_BADSTARTPOSITION; // For 1.x Provider
		}

		// Call IRowsetImpl::GetNextRows to actually get the rows.
		hr = pT->GetNextRows(hReserved2, 0, cRows, pcRowsObtained, prghRows);
		pT->m_iRowset = iRowsetTemp;
		// Send notification message
		if(FAILED(pT->Fire_OnRowChangeMy(this, cRows, *prghRows, DBREASON_ROW_ACTIVATE, DBEVENTPHASE_DIDEVENT, TRUE)))
			ATLTRACE2(atlTraceDBProvider, 0, "Failed to Fire DBREASON_ROW_ACTIVATE in DBEVENTPHASE_DIDEVENT phase\n");
		return hr;
	}


	STDMETHOD (GetRowsByBookmark)(HCHAPTER hReserved, DBCOUNTITEM cRows,
		const DBBKMARK rgcbBookmarks[], const BYTE * rgpBookmarks[],
		HROW rghRows[], DBROWSTATUS rgRowStatus[])
	{
		HRESULT hr = S_OK;
		ATLTRACE("IRowsetLocateImpl::GetRowsByBookmark");

		T* pT = (T*)this;
		if (rgcbBookmarks == NULL || rgpBookmarks == NULL || rghRows == NULL)
			return E_INVALIDARG;

		if (cRows == 0)
			return S_OK;    // No rows fetched in this case.

		bool bErrors = false;
		for (DBCOUNTITEM l=0; l<cRows; l++)
		{
			// Validate each bookmark before fetching the row.  Note, it is
			// an error for the bookmark to be one of the standard values
			hr = ValidateBookmark(rgcbBookmarks[l], rgpBookmarks[l]);
			if (hr != S_OK)
			{
				bErrors = TRUE;
				if (rgRowStatus != NULL)
				{
					rgRowStatus[l] = DBROWSTATUS_E_INVALID;
					continue;
				}
			}

			// Fetch the row, we now that it is a valid row after validation.
			DBCOUNTITEM ulRowsObtained = 0;
			if (pT->CreateRow((long)*rgpBookmarks[l], ulRowsObtained, &rghRows[l]) != S_OK)
			{
				bErrors = TRUE;
			}
			else
			{
				if (rgRowStatus != NULL)
					rgRowStatus[l] = DBROWSTATUS_S_OK;
			}
		}

		if (bErrors)
			return DB_S_ERRORSOCCURRED;
		if (FAILED(pT->Fire_OnRowChangeMy(this, cRows, rghRows, DBREASON_ROW_ACTIVATE, DBEVENTPHASE_DIDEVENT, TRUE)))
			ATLTRACE2(atlTraceDBProvider, 0, "Failed to Fire DBREASON_ROW_ACTIVATE in DBEVENTPHASE_DIDEVENT phase\n");
		return hr;
	}

	STDMETHOD (Hash)(HCHAPTER hReserved, DBCOUNTITEM cBookmarks,
		const DBBKMARK rgcbBookmarks[], const BYTE * rgpBookmarks[],
		DBHASHVALUE rgHashedValues[], DBROWSTATUS rgBookmarkStatus[])
	{
		ATLTRACE("IRowsetScrollImpl::Hash\n");
		T* pT = (T*)this;
		T::ObjectLock cab((T*) pT);
		// Validation
		//ROWSET_NOTIFICATION_REENTRANCY_CHECK 
		if(DB_NULL_HCHAPTER != hReserved)
			return DB_E_NOTSUPPORTED;
		if (cBookmarks <= 0)
			return S_OK;
		if((rgHashedValues == NULL) || (cBookmarks != 0 && (rgcbBookmarks == NULL || rgpBookmarks==NULL)) )  
			return E_INVALIDARG;
		// Logic for hashing
		DBCOUNTITEM cerrors =0;
		if(rgBookmarkStatus)
		{
			// Loop through the array hashing them and recording their status
			for(DBCOUNTITEM i=0;i<cBookmarks; i++)
			{
				if(SUCCEEDED(HashBmk(rgcbBookmarks[i],rgpBookmarks[i],&(rgHashedValues[i]))) )
					rgBookmarkStatus[i] = DBROWSTATUS_S_OK;
				else
				{
					cerrors++;
					rgBookmarkStatus[i] = DBROWSTATUS_E_INVALID;
				}
			}
		}
		else
		{
			// Loop through the array hashing them
			for(DBCOUNTITEM i=0;i<cBookmarks; i++)
			{
				if(FAILED(HashBmk(rgcbBookmarks[i],rgpBookmarks[i],&(rgHashedValues[i]))) )
					cerrors++;
			}
		}
		if (cerrors == 0) return S_OK;
		if (cerrors < cBookmarks)
			return DB_S_ERRORSOCCURRED;
		else return DB_E_ERRORSOCCURRED;
	}

	// Implementation
	protected:
	HRESULT HashBmk(DBBKMARK cbBmk,const BYTE  *pbBmk,DBHASHVALUE *pdwHashedValue,ULONG ulTableSize=0xffffffff)
	{
		if (cbBmk != sizeof(ULONG) || pbBmk == NULL)
			return E_INVALIDARG;
		ATLASSERT(pdwHashedValue);
		*pdwHashedValue = (*(UNALIGNED ULONG*)pbBmk) % ulTableSize;
		return S_OK;
	}


	HRESULT ValidateBookmark(DBBKMARK cbBookmark, const BYTE* pBookmark)
	{
		T* pT = (T*)this;
		T::ObjectLock cab((T*) pT);
		if (cbBookmark == 0 || pBookmark == NULL)
			return E_INVALIDARG;

		if(*pBookmark == DBBMK_INVALID)
				return DB_E_BADBOOKMARK;
		// All of our bookmarks are DWORDs, if they are anything other than
		// sizeof(DWORD) then we have an invalid bookmark
		if ((cbBookmark != sizeof(DWORD)) && (cbBookmark != 1))
		{
			ATLTRACE("Bookmarks are invalid length, should be DWORDs");
			return DB_E_BADBOOKMARK;
		}

		// If the contents of our bookmarks are less than 0 or greater than
		// rowcount, then they are invalid
		size_t nRows = pT->m_DBFile.m_Rows.GetCount();
		if ((*pBookmark <= 0 || *pBookmark > nRows)
			&& *pBookmark != DBBMK_FIRST && *pBookmark != DBBMK_LAST)
		{
			ATLTRACE("Bookmark has invalid range");
			return DB_E_BADBOOKMARK;
		}

		return S_OK;
	}

};

#endif


// End of IRowsetScroll.h