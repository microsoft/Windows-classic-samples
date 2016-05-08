// Start of file MSOmniProvRS.h
// File: MSOmniProvRS.h 
//
//  This file contains the Declaration of the CMSOmniProvRowset and CMSOmniProvCommand
//
//	The following functions have been customized or added for the provider to function:
//
//		CMSOmniProvRowset :: GetData -  Retrieves data from the rowset's copy of the row...
//		CMSOmniProvRowset :: SetSchemaInfo - Creates the rowset column info structure...
//		CMSOmniProvRowset :: GetNextRows - Releases rows...
//		CMSOmniProvRowset :: GetNextRows - Fetches rows, sequentially, remembering the previous position...
//

#ifndef __CMSOmniProvRowset_H_
#define __CMSOmniProvRowset_H_

#include "resource.h"       // main symbols
#include "IRowsetScroll.h" // IRowsetScroll
#include "IRowsetChange.h" // IRowsetChange
#include "RNCP.h"  // The ConnectionPoint and IRowsetNotify Proxy class
#include "CDBFile.h"

class CMSOmniProvCommand ;
class CMSOmniProvRowset ;

// CMSOmniProvCommand

class ATL_NO_VTABLE CMSOmniProvCommand : 
	public CComObjectRootEx<CComSingleThreadModel>,   // Single Threaded for the present...
	public IAccessorImpl<CMSOmniProvCommand>,
	public ICommandTextImpl<CMSOmniProvCommand>,
	public ICommandPropertiesImpl<CMSOmniProvCommand>,
	public IObjectWithSiteImpl<CMSOmniProvCommand>,
	public IConvertTypeImpl<CMSOmniProvCommand>,
	public IColumnsInfoImpl<CMSOmniProvCommand>
{
public:
BEGIN_COM_MAP(CMSOmniProvCommand)
	COM_INTERFACE_ENTRY(ICommand)
	COM_INTERFACE_ENTRY(IObjectWithSite)
	COM_INTERFACE_ENTRY(IAccessor)
	COM_INTERFACE_ENTRY(ICommandProperties)
	COM_INTERFACE_ENTRY2(ICommandText, ICommand)
	COM_INTERFACE_ENTRY(IColumnsInfo)
	COM_INTERFACE_ENTRY(IConvertType)
END_COM_MAP()
// ICommand
public:
	static ATLCOLUMNINFO* GetColumnInfo(CMSOmniProvCommand* pv, DBORDINAL* pcInfo);
	
	HRESULT FinalConstruct()
	{
		HRESULT hr = CConvertHelper::FinalConstruct();
		if (FAILED (hr))
			return hr;
		hr = IAccessorImpl<CMSOmniProvCommand>::FinalConstruct();
		if (FAILED(hr))
			return hr;
		return CUtlProps<CMSOmniProvCommand>::FInit();
	}
	void FinalRelease()
	{
		IAccessorImpl<CMSOmniProvCommand>::FinalRelease();
	}
	HRESULT WINAPI Execute(IUnknown * pUnkOuter, REFIID riid, DBPARAMS * pParams, 
						  DBROWCOUNT * pcRowsAffected, IUnknown ** ppRowset);

BEGIN_PROPSET_MAP(CMSOmniProvCommand)
	BEGIN_PROPERTY_SET(DBPROPSET_ROWSET)
		PROPERTY_INFO_ENTRY_EX(IAccessor,VT_BOOL, DBPROPFLAGS_ROWSET | DBPROPFLAGS_READ, VARIANT_TRUE, 0)
		PROPERTY_INFO_ENTRY_EX(IColumnsInfo, VT_BOOL,   DBPROPFLAGS_ROWSET | DBPROPFLAGS_READ, VARIANT_TRUE, 0)
		PROPERTY_INFO_ENTRY_EX(IConvertType, VT_BOOL,   DBPROPFLAGS_ROWSET | DBPROPFLAGS_READ, VARIANT_TRUE, 0)
		PROPERTY_INFO_ENTRY_EX(IRowset, VT_BOOL,    DBPROPFLAGS_ROWSET | DBPROPFLAGS_READ, VARIANT_TRUE, 0)
		PROPERTY_INFO_ENTRY_EX(IRowsetIdentity,VT_BOOL, DBPROPFLAGS_ROWSET | DBPROPFLAGS_READ, VARIANT_TRUE, 0)
		PROPERTY_INFO_ENTRY_EX(IRowsetInfo, VT_BOOL, DBPROPFLAGS_ROWSET | DBPROPFLAGS_READ, VARIANT_TRUE, 0)
		PROPERTY_INFO_ENTRY_EX(IRowsetChange, VT_BOOL, DBPROPFLAGS_ROWSET | DBPROPFLAGS_READ | DBPROPFLAGS_WRITE, VARIANT_TRUE, 0)
		PROPERTY_INFO_ENTRY_EX(UPDATABILITY, VT_I4, DBPROPFLAGS_ROWSET | DBPROPFLAGS_READ | DBPROPFLAGS_WRITE, DBPROPVAL_UP_CHANGE | DBPROPVAL_UP_DELETE | DBPROPVAL_UP_INSERT, 0)
		PROPERTY_INFO_ENTRY_EX(OWNINSERT, VT_BOOL, DBPROPFLAGS_ROWSET | DBPROPFLAGS_READ | DBPROPFLAGS_WRITE, VARIANT_TRUE, 0)
		PROPERTY_INFO_ENTRY_EX(OWNUPDATEDELETE, VT_BOOL, DBPROPFLAGS_ROWSET | DBPROPFLAGS_READ | DBPROPFLAGS_WRITE, VARIANT_TRUE, 0)
		PROPERTY_INFO_ENTRY_EX(REMOVEDELETED, VT_BOOL, DBPROPFLAGS_ROWSET | DBPROPFLAGS_READ | DBPROPFLAGS_WRITE, VARIANT_TRUE, 0)
		PROPERTY_INFO_ENTRY_EX(IRowsetScroll, VT_BOOL,DBPROPFLAGS_ROWSET | DBPROPFLAGS_READ | DBPROPFLAGS_WRITE, VARIANT_TRUE, 0)
		PROPERTY_INFO_ENTRY_EX(CANFETCHBACKWARDS,VT_BOOL, DBPROPFLAGS_ROWSET | DBPROPFLAGS_READ | DBPROPFLAGS_WRITE, VARIANT_TRUE, 0)
		PROPERTY_INFO_ENTRY_EX(CANHOLDROWS, VT_BOOL, DBPROPFLAGS_ROWSET | DBPROPFLAGS_READ | DBPROPFLAGS_WRITE, VARIANT_TRUE, 0)
		PROPERTY_INFO_ENTRY_EX(CANSCROLLBACKWARDS, VT_BOOL, DBPROPFLAGS_ROWSET | DBPROPFLAGS_READ, VARIANT_TRUE, 0)
	    PROPERTY_INFO_ENTRY_EX(IRowsetLocate, VT_BOOL, DBPROPFLAGS_ROWSET | DBPROPFLAGS_READ | DBPROPFLAGS_WRITE, VARIANT_TRUE, 0)
		PROPERTY_INFO_ENTRY_EX(BOOKMARKS,  VT_BOOL, DBPROPFLAGS_ROWSET | DBPROPFLAGS_READ | DBPROPFLAGS_WRITE , VARIANT_TRUE, 0)
		PROPERTY_INFO_ENTRY_EX(BOOKMARKSKIPPED,VT_BOOL, DBPROPFLAGS_ROWSET | DBPROPFLAGS_READ, VARIANT_TRUE, 0)
		PROPERTY_INFO_ENTRY_EX(BOOKMARKTYPE,  VT_I4, DBPROPFLAGS_ROWSET | DBPROPFLAGS_READ, DBPROPVAL_BMK_NUMERIC, 0)
		PROPERTY_INFO_ENTRY_EX(LITERALBOOKMARKS, VT_BOOL, DBPROPFLAGS_ROWSET | DBPROPFLAGS_READ | DBPROPFLAGS_WRITE, VARIANT_TRUE, 0)
		PROPERTY_INFO_ENTRY_EX(ORDEREDBOOKMARKS, VT_BOOL, DBPROPFLAGS_ROWSET | DBPROPFLAGS_READ | DBPROPFLAGS_WRITE, VARIANT_TRUE, 0)
		PROPERTY_INFO_ENTRY_VALUE(LITERALIDENTITY,VARIANT_TRUE)
		PROPERTY_INFO_ENTRY_VALUE(STRONGIDENTITY,VARIANT_TRUE)
		PROPERTY_INFO_ENTRY_VALUE(NOTIFICATIONPHASES, DBPROPVAL_NP_OKTODO | DBPROPVAL_NP_ABOUTTODO | DBPROPVAL_NP_SYNCHAFTER |DBPROPVAL_NP_FAILEDTODO | DBPROPVAL_NP_DIDEVENT)
		PROPERTY_INFO_ENTRY_EX(NOTIFYROWDELETE, VT_I4, DBPROPVAL_NP_OKTODO | DBPROPVAL_NP_ABOUTTODO | DBPROPVAL_NP_SYNCHAFTER | DBPROPVAL_NP_FAILEDTODO | DBPROPVAL_NP_DIDEVENT, VARIANT_TRUE, 0)
		PROPERTY_INFO_ENTRY_EX(NOTIFYROWINSERT, VT_I4, DBPROPVAL_NP_OKTODO | DBPROPVAL_NP_ABOUTTODO | DBPROPVAL_NP_SYNCHAFTER | DBPROPVAL_NP_FAILEDTODO | DBPROPVAL_NP_DIDEVENT, VARIANT_TRUE, 0)
		PROPERTY_INFO_ENTRY_EX(NOTIFYCOLUMNSET, VT_I4, DBPROPVAL_NP_OKTODO | DBPROPVAL_NP_ABOUTTODO | DBPROPVAL_NP_SYNCHAFTER | DBPROPVAL_NP_FAILEDTODO | DBPROPVAL_NP_DIDEVENT, VARIANT_TRUE, 0)
		PROPERTY_INFO_ENTRY_EX(NOTIFYROWSETFETCHPOSITIONCHANGE, VT_I4, DBPROPVAL_NP_OKTODO | DBPROPVAL_NP_ABOUTTODO | DBPROPVAL_NP_SYNCHAFTER | DBPROPVAL_NP_FAILEDTODO | DBPROPVAL_NP_DIDEVENT, VARIANT_TRUE, 0)
		PROPERTY_INFO_ENTRY_EX(SERVERCURSOR, VT_BOOL, DBPROPFLAGS_ROWSET | DBPROPFLAGS_READ , VARIANT_TRUE, 0)
	END_PROPERTY_SET(DBPROPSET_ROWSET)
END_PROPSET_MAP()
};

typedef CDBFile dummyStore;

class ATL_NO_VTABLE  CMSOmniProvRowset : 
	public CRowsetImpl <CMSOmniProvRowset, dummyStore, CMSOmniProvCommand, CAtlArray<dummyStore>, CSimpleRow, IRowsetScrollImpl<CMSOmniProvRowset> > ,
	public IMyRowsetChangeImpl<CMSOmniProvRowset, dummyStore>,
	public CProxyIRowsetNotify< CMSOmniProvRowset >,
	public IConnectionPointContainerImpl<CMSOmniProvRowset> 
{
public:
	typedef CRowsetImpl <CMSOmniProvRowset, dummyStore, CMSOmniProvCommand, CAtlArray<dummyStore>, CSimpleRow, IRowsetScrollImpl<CMSOmniProvRowset> >  _OmniProvBaseClass;

	BEGIN_COM_MAP(CMSOmniProvRowset)
		COM_INTERFACE_ENTRY(IRowsetLocate)
		COM_INTERFACE_ENTRY(IRowsetScroll)
		COM_INTERFACE_ENTRY(IRowsetChange)
		COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
		COM_INTERFACE_ENTRY_CHAIN(_OmniProvBaseClass)
	END_COM_MAP()

	
	BEGIN_CONNECTION_POINT_MAP(CMSOmniProvRowset)
		CONNECTION_POINT_ENTRY(IID_IRowsetNotify)
	END_CONNECTION_POINT_MAP()
	
public:
	static ATLCOLUMNINFO* GetColumnInfo(CMSOmniProvRowset* pv, DBORDINAL* pNumCols);
	BOOL CheckTable(TCHAR* szTblName);
	HRESULT Execute(DBPARAMS * pParams, DBROWCOUNT* pcRowsAffected);
	void CreateColInfo();
	void AllocateProxyBuffers();
	HRESULT GetDataSource(_bstr_t &m_bstrLoc);
	ATLCOLUMNINFO * GetSchemaInfo(DBORDINAL * pNumCols);  

	// All the member variables
	DBORDINAL m_cCols;              // Number of columns represented in m_prgColInfo
	CDBFile m_DBFile;    // The Storage class
	ATLCOLUMNINFO * m_prgColInfo;  // Each rowset will have its set of column infos with or without bookmarks

	// The Constructor
	CMSOmniProvRowset()
	{
		m_prgColInfo = NULL;
		
	}

	// The Destructor
	~CMSOmniProvRowset()
	{
		if(m_prgColInfo)
		{
			delete [] m_prgColInfo;
			m_prgColInfo = NULL;
		}
	}

	// CMSOmniProvRowset :: SetSchemaInfo
	// This function copies the final column info to the rowset class 
	// Usually called from the Execute method as that is when the columns will be known 
	// (bookmark + columns selected)
	void SetSchemaInfo(ATLCOLUMNINFO * prgColInfo, DBORDINAL cCols)
	{
		m_cCols = cCols;
		m_prgColInfo = new ATLCOLUMNINFO[cCols];
		// Copy the information from the columninfo passed in. This is possible because
		// we know that there are no pointers in the structure.
		for (DBORDINAL i = 0; i < cCols; i++)
			m_prgColInfo[i] = prgColInfo[i];
	}
	
	// CMSOmniProvRowset :: Getdata
	// Retrieves data from the rowset's copy of the row.
	STDMETHOD(GetData)(HROW hRow,
					   HACCESSOR hAccessor,
					   void *pDstData)
	{
		ATLTRACE2(atlTraceDBProvider, 0, "IRowsetImpl::GetData\n");
		
		if (pDstData == NULL)
			return E_INVALIDARG;
		HRESULT hr = S_OK;
		//CSimpleRow* pHRow = (CSimpleRow*)hRow;
		// 2.0
		CSimpleRow* pHRow = NULL;
		if (hRow == NULL || (pHRow = (m_rgRowHandles.Lookup(hRow))->m_value ) == NULL)
			return DB_E_BADROWHANDLE;
		_BindType* pBinding;
		void* pSrcData;
		DBORDINAL cCols;
		ATLCOLUMNINFO* pColInfo;
		
		pBinding = m_rgBindings.Lookup(hAccessor)->m_value;
		if (pBinding == NULL)
			return DB_E_BADACCESSORHANDLE;
		pColInfo = GetColumnInfo(this, &cCols);
		CRow *pDataRow = NULL;
		// When Client perform a GetData without performing a GetNextRows( )
		if(pHRow->m_iRowset >= m_DBFile.m_Rows.GetCount())
			return DB_E_DELETEDROW;
		pDataRow = m_DBFile.m_Rows[pHRow->m_iRowset];
		// Fetch data in the proxy buffer if it hasn't already been fetched
		pDataRow->GetProxyData(pColInfo, cCols);
		// assign source buffer to the proxy buffer
		pSrcData = pDataRow->m_pbProxyData;
		for (ULONG iBind =0; iBind < pBinding->cBindings; iBind++)
		{
			DBBINDING* pBindCur = &(pBinding->pBindings[iBind]);
			DBORDINAL iColInfo;
			for (iColInfo = 0;
				 iColInfo < cCols && pBindCur->iOrdinal != pColInfo[iColInfo].iOrdinal;
				 iColInfo++);
			if (iColInfo == cCols)
				return DB_E_BADORDINAL;
			ATLCOLUMNINFO* pColCur = &(pColInfo[iColInfo]);
			// Ordinal found at iColInfo
			BOOL bProvOwn = pBindCur->dwMemOwner == DBMEMOWNER_PROVIDEROWNED;
			bProvOwn;
		
			DBSTATUS dbStat = DBSTATUS_S_OK;
			// If the provider's field is NULL, we can optimize this situation,
			// set the fields to 0 and continue.
			if (dbStat == DBSTATUS_S_ISNULL)
			{
				if (pBindCur->dwPart & DBPART_STATUS)
					*((DBSTATUS*)((BYTE*)(pDstData) + pBindCur->obStatus)) = dbStat;

				if (pBindCur->dwPart & DBPART_LENGTH)
					*((DBLENGTH*)((BYTE*)(pDstData) + pBindCur->obLength)) = 0;

				if (pBindCur->dwPart & DBPART_VALUE)
					*((BYTE*)(pDstData) + pBindCur->obValue) = NULL;
				continue;
			}
			DBLENGTH cbDst = pBindCur->cbMaxLen;
			DBLENGTH cbCol;
			BYTE* pSrcTemp;
			void *pvData = NULL;
			void * pvTmpData = NULL;

			if (bProvOwn && pColCur->wType == pBindCur->wType)
			{
				pSrcTemp = ((BYTE*)(pSrcData) + pColCur->cbOffset);
			}
			else
			{
				BYTE* pDstTemp = (BYTE*)pDstData + pBindCur->obValue;
				switch (pColCur->wType)
				{
				case DBTYPE_STR:
					cbCol = lstrlenA((LPSTR)(((BYTE*)pSrcData) + pColCur->cbOffset));
					break;
				case DBTYPE_WSTR:
				case DBTYPE_BSTR:
					cbCol = lstrlenW((LPWSTR)(((BYTE*)pSrcData) + pColCur->cbOffset)) * sizeof(WCHAR);
					break;
				// The char ptr case
				case DBTYPE_STR | DBTYPE_BYREF:
					pvData = (((BYTE*)pSrcData) + pColCur->cbOffset);
					cbCol =lstrlenA(*(char**)pvData);
					break;
				default:
					cbCol = pColCur->ulColumnSize;
					break;
				}
				if (pBindCur->dwPart & DBPART_VALUE)
				{
					hr = m_spConvert->DataConvert(pColCur->wType, pBindCur->wType,
											cbCol, &cbDst, (BYTE*)(pSrcData) + pColCur->cbOffset,
											pDstTemp, pBindCur->cbMaxLen, dbStat, &dbStat,
											pBindCur->bPrecision, pBindCur->bScale,0);
				}
			}
			if (pBindCur->dwPart & DBPART_LENGTH)
				*(DBBYTEOFFSET*)((BYTE*)pDstData + pBindCur->obLength) = cbDst;
			if (pBindCur->dwPart & DBPART_STATUS)
				*((DBSTATUS*)((BYTE*)(pDstData) + pBindCur->obStatus)) = dbStat;
			if (FAILED(hr))
				return hr;
		}
		return hr;
	}

	// CMSOmniProvRowset :: ReleaseRows 
	// Releases rows. 
	STDMETHOD(ReleaseRows)(DBCOUNTITEM cRows,
						   const HROW rghRows[],
						   DBROWOPTIONS rgRowOptions[],
						   DBREFCOUNT rgRefCounts[],
						   DBROWSTATUS rgRowStatus[])
	{
		HRESULT hr = _OmniProvBaseClass::ReleaseRows(
						  cRows,
						   rghRows,
						   rgRowOptions,
						   rgRefCounts,
						   rgRowStatus);
		if(FAILED(Fire_OnRowChangeMy(this, cRows, rghRows, DBREASON_ROW_RELEASE, DBEVENTPHASE_DIDEVENT, TRUE)))
			ATLTRACE2(atlTraceDBProvider, 0, "Failed to Fire DBREASON_ROW_RELEASE in DBEVENTPHASE_DIDEVENT phase\n");
		return hr;
	}

	// CMSOmniProvRowset :: GetNextRows
	// Fetches rows, sequentially, remembering the previous position...
	STDMETHOD(GetNextRows)(HCHAPTER /*hReserved*/,
						   DBROWOFFSET lRowsOffset,
						   DBROWCOUNT cRows,
						   DBCOUNTITEM *pcRowsObtained,
						   HROW **prghRows)
	{
		DBROWOFFSET lTmpRows = lRowsOffset;
		ATLTRACE2(atlTraceDBProvider, 0, "IRowsetImpl::GetNextRows\n");
		HRESULT hrEvent;
		CMSOmniProvRowset* pT = (CMSOmniProvRowset*) this;
		CMSOmniProvRowset::ObjectLock cab(pT);
     	if(FAILED(hrEvent = Fire_OnRowsetChangeMy(this, DBREASON_ROWSET_FETCHPOSITIONCHANGE, DBEVENTPHASE_OKTODO, TRUE))) 
		{
			ATLTRACE2(atlTraceDBProvider, 0, "Failed to Fire DBREASON_ROWSET_FETCHPOSITIONCHANGE in DBEVENTPHASE_OKTODO phase\n");
			if(hrEvent==S_FALSE)
				return DB_E_CANCELED;
		}
		if(FAILED(Fire_OnRowsetChangeMy(this, DBREASON_ROWSET_FETCHPOSITIONCHANGE, DBEVENTPHASE_ABOUTTODO, TRUE))) 
			ATLTRACE2(atlTraceDBProvider, 0, "Failed to Fire DBREASON_ROWSET_FETCHPOSITIONCHANGE in DBEVENTPHASE_ABOUTTODO phase\n");
		if (pcRowsObtained != NULL)
			*pcRowsObtained = 0;
		if (prghRows == NULL || pcRowsObtained == NULL)\
		{
			if(FAILED(Fire_OnRowsetChangeMy(this, DBREASON_ROWSET_FETCHPOSITIONCHANGE, DBEVENTPHASE_FAILEDTODO, TRUE))) 
				ATLTRACE2(atlTraceDBProvider, 0, "Failed to Fire DBREASON_ROWSET_FETCHPOSITIONCHANGE in DBEVENTPHASE_FAILEDTODO phase\n");
			return E_INVALIDARG;
		}
		if (cRows == 0)
		{
			if(FAILED(Fire_OnRowsetChangeMy(this, DBREASON_ROWSET_FETCHPOSITIONCHANGE, DBEVENTPHASE_SYNCHAFTER, TRUE))) 
				ATLTRACE2(atlTraceDBProvider, 0, "Failed to Fire DBREASON_ROWSET_FETCHPOSITIONCHANGE in DBEVENTPHASE_SYNCHAFTER phase\n");
			if(FAILED(Fire_OnRowsetChangeMy(this, DBREASON_ROWSET_FETCHPOSITIONCHANGE, DBEVENTPHASE_DIDEVENT, TRUE))) 
				ATLTRACE2(atlTraceDBProvider, 0, "Failed to Fire DBREASON_ROWSET_FETCHPOSITIONCHANGE in DBEVENTPHASE_DIDEVENT phase\n");
			return S_OK;
		}
		HRESULT hr = S_OK;
		if (lRowsOffset < 0 && !m_bCanScrollBack)
		{
			if(FAILED(Fire_OnRowsetChangeMy(this, DBREASON_ROWSET_FETCHPOSITIONCHANGE, DBEVENTPHASE_FAILEDTODO, TRUE))) 
				ATLTRACE2(atlTraceDBProvider, 0, "Failed to Fire DBREASON_ROWSET_FETCHPOSITIONCHANGE in DBEVENTPHASE_FAILEDTODO phase\n");
			return DB_E_CANTSCROLLBACKWARDS;
		}
		if (cRows < 0  && !m_bCanFetchBack)
		{
			if(FAILED(Fire_OnRowsetChangeMy(this, DBREASON_ROWSET_FETCHPOSITIONCHANGE, DBEVENTPHASE_FAILEDTODO, TRUE))) 
				ATLTRACE2(atlTraceDBProvider, 0, "Failed to Fire DBREASON_ROWSET_FETCHPOSITIONCHANGE in DBEVENTPHASE_FAILEDTODO phase\n");
			return DB_E_CANTFETCHBACKWARDS;
		}
		// Calculate # of rows in set and the base fetch position.  If the rowset
		// is at its head position, then lRowOffset < 0 means moving from the BACK
		// of the rowset and not the front.
		DBROWOFFSET cRowsInSet = pT->m_DBFile.m_Rows.GetCount();
		if (((lRowsOffset == LONG_MIN) && (cRowsInSet != LONG_MIN))
			|| (AbsVal(lRowsOffset)) > cRowsInSet ||
			(AbsVal(lRowsOffset) == cRowsInSet && lRowsOffset < 0 && cRows < 0) ||
			(AbsVal(lRowsOffset) == cRowsInSet && lRowsOffset > 0 && cRows > 0))
		{
			if(FAILED(Fire_OnRowsetChangeMy(this, DBREASON_ROWSET_FETCHPOSITIONCHANGE, DBEVENTPHASE_FAILEDTODO, TRUE))) 
				ATLTRACE2(atlTraceDBProvider, 0, "Failed to Fire DBREASON_ROWSET_FETCHPOSITIONCHANGE in DBEVENTPHASE_FAILEDTODO phase\n");
			return DB_S_ENDOFROWSET;
		}

		// In the case where the user is moving backwards after moving forwards,
		// we do not wrap around to the end of the rowset.
		if ((m_iRowset == 0 && !m_bReset && cRows < 0) ||
			(((LONG)m_iRowset + lRowsOffset) > cRowsInSet) ||
			(m_iRowset == (DWORD)cRowsInSet && lRowsOffset >= 0 && cRows > 0))
		{
			if(FAILED(Fire_OnRowsetChangeMy(this, DBREASON_ROWSET_FETCHPOSITIONCHANGE, DBEVENTPHASE_FAILEDTODO, TRUE))) 
				ATLTRACE2(atlTraceDBProvider, 0, "Failed to Fire DBREASON_ROWSET_FETCHPOSITIONCHANGE in DBEVENTPHASE_FAILEDTODO phase\n");
			return DB_S_ENDOFROWSET;
		}

		// Note, if m_bReset, m_iRowset must be 0
		if (lRowsOffset < 0 && m_bReset)
		{
			ATLASSERT(m_iRowset == 0);
			m_iRowset = cRowsInSet;
		}

		int iStepSize = cRows >= 0 ? 1 : -1;

		// If cRows == LONG_MIN, we can't use ABS on it.  Therefore, we reset it
		// to a value just greater than cRowsInSet
		if (cRows == LONG_MIN && cRowsInSet != LONG_MIN)
			cRows = cRowsInSet + 2; // set the value to something we can deal with
		else
			cRows = AbsVal(cRows);

		if (iStepSize < 0 && m_iRowset == 0 && m_bReset && lRowsOffset <= 0)
			m_iRowset = cRowsInSet;

		lRowsOffset += m_iRowset;

		*pcRowsObtained = 0;
		CComHeapPtr<HROW> amr;
		if (*prghRows == NULL)
		{
			DBROWCOUNT cHandlesToAlloc = (cRows > cRowsInSet) ? cRowsInSet : cRows;
			if (iStepSize == 1 && (cRowsInSet - lRowsOffset) < cHandlesToAlloc)
				cHandlesToAlloc = cRowsInSet - lRowsOffset;
			if (iStepSize == -1 && lRowsOffset < cHandlesToAlloc)
				cHandlesToAlloc = lRowsOffset;
			*prghRows = (HROW*)CoTaskMemAlloc((cHandlesToAlloc) * sizeof(HROW*));
			amr.Attach(*prghRows);
		}
		if (*prghRows == NULL)
		{
			if(FAILED(Fire_OnRowsetChangeMy(this, DBREASON_ROWSET_FETCHPOSITIONCHANGE, DBEVENTPHASE_FAILEDTODO, TRUE))) 
				ATLTRACE2(atlTraceDBProvider, 0, "Failed to Fire DBREASON_ROWSET_FETCHPOSITIONCHANGE in DBEVENTPHASE_FAILEDTODO phase\n");
			return E_OUTOFMEMORY;
		}
		while ((lRowsOffset >= 0 && cRows != 0) &&
			((lRowsOffset < cRowsInSet) || (lRowsOffset <= cRowsInSet && iStepSize < 0)))
		{
			// cRows > cRowsInSet && iStepSize < 0
			if (lRowsOffset == 0 && cRows > 0 && iStepSize < 0)
				break;

			// in the case where we have iStepSize < 0, move the row back
			// further because we want the previous row
			DBROWOFFSET lRow = lRowsOffset;
			if ((lRowsOffset == 0) && (lTmpRows == 0) && (iStepSize < 0))
				lRow = cRowsInSet;

			if (iStepSize < 0)
				lRow += iStepSize;

 			hr = pT->CreateRow(lRow, *pcRowsObtained, *prghRows);
			if (FAILED(hr))
			{
				RefRows(*pcRowsObtained, *prghRows, NULL, NULL, FALSE);
				for (ULONG iRowDel = 0; iRowDel < *pcRowsObtained; iRowDel++)
					*prghRows[iRowDel] = NULL;
				*pcRowsObtained = 0;
				if(FAILED(Fire_OnRowsetChangeMy(this, DBREASON_ROWSET_FETCHPOSITIONCHANGE, DBEVENTPHASE_FAILEDTODO, TRUE))) 
					ATLTRACE2(atlTraceDBProvider, 0, "Failed to Fire DBREASON_ROWSET_FETCHPOSITIONCHANGE in DBEVENTPHASE_FAILEDTODO phase\n");
				return hr;
			}
			cRows--;
			lRowsOffset += iStepSize;
		}

		if ((lRowsOffset >= cRowsInSet && cRows) || (lRowsOffset < 0 && cRows)  ||
			(lRowsOffset == 0 && cRows > 0 && iStepSize < 0))
			hr = DB_S_ENDOFROWSET;
		m_iRowset = lRowsOffset;
		if (SUCCEEDED(hr))
			amr.Detach();
		if(FAILED(Fire_OnRowsetChangeMy(this, DBREASON_ROWSET_FETCHPOSITIONCHANGE, DBEVENTPHASE_SYNCHAFTER, TRUE))) 
			ATLTRACE2(atlTraceDBProvider, 0, "Failed to Fire DBREASON_ROWSET_FETCHPOSITIONCHANGE in DBEVENTPHASE_SYNCHAFTER phase\n");
		if(FAILED(Fire_OnRowsetChangeMy(this, DBREASON_ROWSET_FETCHPOSITIONCHANGE, DBEVENTPHASE_DIDEVENT, TRUE))) 
			ATLTRACE2(atlTraceDBProvider, 0, "Failed to Fire DBREASON_ROWSET_FETCHPOSITIONCHANGE in DBEVENTPHASE_DIDEVENT phase\n");
		if(FAILED(Fire_OnRowChangeMy(this, cRows, *prghRows, DBREASON_ROW_ACTIVATE, DBEVENTPHASE_DIDEVENT, TRUE))) 
			ATLTRACE2(atlTraceDBProvider, 0, "Failed to Fire DBREASON_ROW_ACTIVATE in DBEVENTPHASE_DIDEVENT phase\n");
		return hr;
	}

};

#endif //__CMSOmniProvRowset_H_

// End of file MSOmniProvRS.h