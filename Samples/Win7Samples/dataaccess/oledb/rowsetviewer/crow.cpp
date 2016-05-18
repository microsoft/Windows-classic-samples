//-----------------------------------------------------------------------------
// Microsoft OLE DB RowsetViewer
// Copyright (C) 1994 - 1999 By Microsoft Corporation.
//
// @doc
//
// @module CROW.CPP
//
//-----------------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////////
// Includes
//
//////////////////////////////////////////////////////////////////////////////
#include "Headers.h"


/////////////////////////////////////////////////////////////////
// CRow::CRow
//
/////////////////////////////////////////////////////////////////
CRow::CRow(CMainWindow* pCMainWindow, CMDIChild* pCMDIChild) 
	: CDataAccess(eCRow, pCMainWindow, pCMDIChild)
{
	//Row
	m_pIRow							= NULL;//Row Interface
	m_pIGetSession					= NULL;//Row Interface
												
	m_pIColumnsInfo2				= NULL;//Row Interface
	m_pICreateRow					= NULL;//Row Interface
	m_pIDBCreateCommand				= NULL;//Row Interface
	m_pIRowChange					= NULL;//Row Interface
	m_pIRowSchemaChange				= NULL;//Row Interface
	m_pIBindResource				= NULL;//Row Interface
	m_pIScopedOperations			= NULL;//Row Interface

	//ColAccess
	m_cColAccess	= 0;
	m_rgColAccess   = NULL;

	//Listeners

	//Data
	m_hSourceRow	= NULL;
}


/////////////////////////////////////////////////////////////////
// CRow::~CRow
//
/////////////////////////////////////////////////////////////////
CRow::~CRow()
{
	ReleaseObject(0);
}


/////////////////////////////////////////////////////////////////
// IUnknown** CRow::GetInterfaceAddress
//
/////////////////////////////////////////////////////////////////
IUnknown** CRow::GetInterfaceAddress(REFIID riid)
{
	HANDLE_GETINTERFACE(IRow);
	HANDLE_GETINTERFACE(IGetSession);
	HANDLE_GETINTERFACE(IColumnsInfo2);
	HANDLE_GETINTERFACE(ICreateRow);
	HANDLE_GETINTERFACE(IDBCreateCommand);
	HANDLE_GETINTERFACE(IRowChange);
	HANDLE_GETINTERFACE(IRowSchemaChange);
	HANDLE_GETINTERFACE(IBindResource);
	HANDLE_GETINTERFACE(IScopedOperations);

	//Otherwise delegate
	return CDataAccess::GetInterfaceAddress(riid);
}


/////////////////////////////////////////////////////////////////
// CRow::AutoRelease
//
/////////////////////////////////////////////////////////////////
HRESULT CRow::AutoRelease()
{
	m_cColAccess = 0;
	SAFE_FREE(m_rgColAccess);

	//Rowset
	RELEASE_INTERFACE(IRow);
	RELEASE_INTERFACE(IGetSession);

	RELEASE_INTERFACE(IColumnsInfo2);
	RELEASE_INTERFACE(ICreateRow);
	RELEASE_INTERFACE(IDBCreateCommand);
	RELEASE_INTERFACE(IRowChange);
	RELEASE_INTERFACE(IRowSchemaChange);
	RELEASE_INTERFACE(IBindResource);
	RELEASE_INTERFACE(IScopedOperations);

	//Delegate
	return CDataAccess::AutoRelease();
}


////////////////////////////////////////////////////////////////
// CRow::AutoQI
//
/////////////////////////////////////////////////////////////////
HRESULT CRow::AutoQI(DWORD dwCreateOpts)
{
	//Delegate First so we have base interfaces
	CDataAccess::AutoQI(dwCreateOpts);

	//[MANDATORY] Obtain [mandatory] interfaces
	if(dwCreateOpts & CREATE_QI_MANDATORY)
	{
		OBTAIN_INTERFACE(IRow);
		OBTAIN_INTERFACE(IGetSession);
	}

	//Auto QI
	if(dwCreateOpts & CREATE_QI_OPTIONAL)
	{
		OBTAIN_INTERFACE(IColumnsInfo2);
		OBTAIN_INTERFACE(ICreateRow);
		OBTAIN_INTERFACE(IDBCreateCommand);
		OBTAIN_INTERFACE(IRowChange);
		OBTAIN_INTERFACE(IRowSchemaChange);
		OBTAIN_INTERFACE(IBindResource);
		OBTAIN_INTERFACE(IScopedOperations);
	}

	return S_OK;
}



/////////////////////////////////////////////////////////////////
// HRESULT CRow::DisplayObject
//
/////////////////////////////////////////////////////////////////
HRESULT CRow::DisplayObject()
{
	HRESULT hr = S_OK;
	BINDCOLS eBindCols = (GetOptions()->m_dwAccessorOpts & ACCESSOR_BIND_BOOKMARK ? BIND_ALLCOLS : BIND_ALLCOLSEXPECTBOOKMARK);

	if(m_pCMDIChild)
	{
		//First Clear the existing Window...
		m_pCMDIChild->m_pCDataGrid->m_fLastFetchForward = FALSE;
		m_pCMDIChild->m_pCDataGrid->m_lCurPos = 0;
		TESTC(hr = m_pCMDIChild->m_pCDataGrid->ClearAll());
	}

	//Setup ColumnAccess Structures, (based off the ColumnInfo)
	TESTC(hr = SetupColAccess(eBindCols));

	if(m_pCMDIChild)
	{
		//Refresh the Columns and Rows
		TESTC(hr = m_pCMDIChild->m_pCDataGrid->RefreshData());
	}

	//Delegate
	TESTC(hr = CDataAccess::DisplayObject());

CLEANUP:
	if(m_pCMDIChild)
		m_pCMDIChild->UpdateControls();
	return hr;
}



/////////////////////////////////////////////////////////////////////////////
// CRow::GetObjectDesc
//
/////////////////////////////////////////////////////////////////////////////
WCHAR* CRow::GetObjectDesc()
{
	HRESULT hr = S_OK;

	if(!m_strObjectDesc && m_hSourceRow)
	{
		WCHAR* pwszValue = NULL;
		
		//0x00000000 = 1 Byte = 2 Asci Chars + "0x" + NULL Terminator
		SAFE_ALLOC(pwszValue, WCHAR, POINTER_DISPLAYSIZE);
		StringFormat(pwszValue, POINTER_DISPLAYSIZE, L"0x%p", (HANDLE)m_hSourceRow);
		m_strObjectDesc.Attach(pwszValue);
	}

CLEANUP:
	return m_strObjectDesc;
}


/////////////////////////////////////////////////////////////////
// HRESULT CRow::CreateCommand
//
/////////////////////////////////////////////////////////////////
HRESULT CRow::CreateCommand(CAggregate* pCAggregate, REFIID riid, IUnknown** ppIUnknown)
{
	HRESULT				hr = S_OK;

	//Obtain the IDBCreateCommand Interface
	IDBCreateCommand* pIDBCreateCommand = SOURCE_GETINTERFACE(this, IDBCreateCommand);
	if(pIDBCreateCommand)
	{
		//CreateCommand
		XTEST(hr = pIDBCreateCommand->CreateCommand(pCAggregate, riid, ppIUnknown));
		TESTC(TRACE_METHOD(hr, L"IDBCreateCommand::CreateCommand(0x%p, %s, &0x%p)", pCAggregate, GetInterfaceName(riid), ppIUnknown ? *ppIUnknown : NULL));

		//Handle Aggregation
		if(pCAggregate)
			TESTC(hr = pCAggregate->HandleAggregation(riid, ppIUnknown));
	}

CLEANUP:
	return hr;
}


/////////////////////////////////////////////////////////////////
// HRESULT CRow::OpenRowset
//
/////////////////////////////////////////////////////////////////
HRESULT CRow::OpenRowset(CAggregate* pCAggregate, DBID* pTableID, DBID* pIndexID, REFIID riid, ULONG cPropSets, DBPROPSET* rgPropSets, IUnknown** ppIUnknown)
{
	HRESULT				hr = E_FAIL;			// HRESULT

	if(!m_pIScopedOperations)
		return E_FAIL;

	WCHAR wszTableName[MAX_QUERY_LEN+1];
	WCHAR wszIndexName[MAX_QUERY_LEN+1];

	// From IOpenRowset, get a rowset object
	DBIDToString(pTableID, wszTableName, MAX_QUERY_LEN);
	DBIDToString(pIndexID, wszIndexName, MAX_QUERY_LEN);
				
	XTEST_(hr = m_pIScopedOperations->OpenRowset(
							pCAggregate,		// pUnkOuter
							pTableID,			// pTableID
							pIndexID,			// pIndexID
							riid,				// refiid
							cPropSets,			// cProperties
							rgPropSets,			// rgProperties
							ppIUnknown),S_OK);	// IRowset pointer
	TRACE_METHOD(hr, L"IScopedOperations::OpenRowset(0x%p, %s, %s, %s, %d, 0x%p, &0x%p)", pCAggregate, wszTableName, wszIndexName, GetInterfaceName(riid), cPropSets, rgPropSets, ppIUnknown ? *ppIUnknown : NULL);

	//Display Errors (if occurred)
	TESTC(hr = DisplayPropErrors(hr, cPropSets, rgPropSets));

	//Handle Aggregation
	if(pCAggregate)
		TESTC(hr = pCAggregate->HandleAggregation(riid, ppIUnknown));

CLEANUP:
	return hr;    
}

	
/////////////////////////////////////////////////////////////////
// HRESULT CRow::SetupColAccess
//
/////////////////////////////////////////////////////////////////
HRESULT CRow::SetupColAccess(BINDCOLS eBindCols)
{
	HRESULT hr = S_OK;
	DBLENGTH dwOffset = 0;
	ULONG i;

	//Only capable of the Following Converions (for Display)
	DWORD  dwMaxLength		= GetOptions()->m_dwMaxLength;
	DWORD  dwAccessorOpts	= GetOptions()->m_dwAccessorOpts;
	
	if(m_pIColumnsInfo == NULL)
		return E_FAIL;

	//GetColumnInfo
	GetColInfo();

	//Alloc the space to hold the ColumnAccess structs
	ULONG cBindings = 0;
	DBBINDING* rgBindings = NULL;
	SAFE_ALLOC(m_rgColAccess, DBCOLUMNACCESS, m_ColumnInfo.GetCount());
	SAFE_ALLOC(rgBindings, DBBINDING, m_ColumnInfo.GetCount());

	//Figure out how big to make the buffer...
	for(i=0; i<m_ColumnInfo.GetCount(); i++)
	{
		//Offset
		m_rgColAccess[i].cbMaxLen = GetMaxDisplaySize(GetOptions()->GetBindingType(m_ColumnInfo[i].wType), m_ColumnInfo[i].wType, m_ColumnInfo[i].ulColumnSize, dwMaxLength);
		dwOffset = ROUNDUP( dwOffset + m_rgColAccess[i].cbMaxLen );
	}

	//Size for pData
	SAFE_REALLOC(m_pData, BYTE, dwOffset);
	
	dwOffset = 0;
	m_cColAccess = 0;
	cBindings = 0;
	for(i=0; i<m_ColumnInfo.GetCount(); i++) 
	{
		DBCOLUMNINFO*	pColInfo	= &m_ColumnInfo[i];
		DBCOLUMNACCESS*	pColAccess	= &m_rgColAccess[m_cColAccess];
		DBBINDING*		pBinding	= &rgBindings[cBindings];

		//Setup the Bindings
		pColAccess->columnid		= pColInfo->columnid;
		pColAccess->bPrecision		= pColInfo->bPrecision;
		pColAccess->bScale			= pColInfo->bScale;
		pColAccess->wType			= GetOptions()->GetBindingType(pColInfo->wType);
		pColAccess->cbDataLen		= 0;

		//NOTE: This is setup above in the Previous for loop...
		pColAccess->cbMaxLen		= m_rgColAccess[i].cbMaxLen; 
		
		//BLOB or IUnknown Bindings
		if(pColAccess->wType == DBTYPE_IUNKNOWN || pColInfo->wType == DBTYPE_IUNKNOWN || 
			(pColInfo->dwFlags & DBCOLUMNFLAGS_ISLONG && (dwAccessorOpts & (ACCESSOR_BLOB_ISEQSTREAM|ACCESSOR_BLOB_ILOCKBYTES|ACCESSOR_BLOB_ISTORAGE|ACCESSOR_BLOB_ISTREAM))))
		{
			pColAccess->wType		= DBTYPE_IUNKNOWN;
			pColAccess->cbMaxLen	= sizeof(IUnknown*);
		}	
		
		//If the consumer requested not to bind the Value in Options, then set pData = NULL
		//This is how the 2.5 spec says to indicate VALUE is not bound (since their is no dwPart).
		pColAccess->pData			= NULL;
		if(dwAccessorOpts & ACCESSOR_BIND_VALUE)
			pColAccess->pData		= (BYTE*)m_pData + dwOffset;
						
		//Initialize the Status to an error, so we don't end up freeing outofline 
		//data we haven't retreived yet...
		pColAccess->dwStatus		= DBSTATUS_E_UNAVAILABLE;

		//Special Handling for other non-OLE DB defined convertable types to WSTR
		//NOTE: The spec requires all supported types to be converted to 
		//WSTR, but this only applies where the OLE DB conversion is defined.
		//Some are not defined so we need to bind nativly.
		switch(pColInfo->wType)
		{
			case DBTYPE_IUNKNOWN:
			case DBTYPE_IDISPATCH:
				pColAccess->wType		= pColInfo->wType;
				pColAccess->cbMaxLen	= sizeof(IUnknown*);
				break;

			case DBTYPE_HCHAPTER:
				pColAccess->wType		= pColInfo->wType;
				pColAccess->cbMaxLen	= sizeof(HCHAPTER);
				break;

			default:
				//DBTYPE_VECTOR
				if(pColInfo->wType	& DBTYPE_VECTOR)
				{
					pColAccess->wType	= pColInfo->wType;
					pColAccess->cbMaxLen= sizeof(DBVECTOR);
				}
				
				//DBTYPE_ARRAY
				if(pColInfo->wType	& DBTYPE_ARRAY)
				{
					pColAccess->wType	= pColInfo->wType;
					pColAccess->cbMaxLen= sizeof(SAFEARRAY*);
				}
				break;
		};

		//Offset
		dwOffset += pColAccess->cbMaxLen;
		dwOffset = ROUNDUP( dwOffset );

		//Build a real simple binding on top of this ColumnAccess struct.
		//The reason is all of our data manipulations routines deal with binding structs, so instead
		//have having numerous code lines, we will just map this to a binding struct...
		pBinding->iOrdinal		= pColInfo->iOrdinal;
		pBinding->obStatus		= (DBBYTEOFFSET)((BYTE*)&pColAccess->dwStatus - (BYTE*)m_pData);
		pBinding->obLength		= (DBBYTEOFFSET)((BYTE*)&pColAccess->cbDataLen - (BYTE*)m_pData);
		pBinding->obValue		= (DBBYTEOFFSET)((BYTE*)pColAccess->pData - (BYTE*)m_pData);

		pBinding->pTypeInfo		= NULL;
		pBinding->pObject		= NULL;
		pBinding->pBindExt		= NULL;

		pBinding->dwPart		= DBPART_LENGTH|DBPART_STATUS;
		if(pColAccess->pData)
			pBinding->dwPart	|= DBPART_VALUE;

		pBinding->dwMemOwner	= (dwAccessorOpts & ACCESSOR_OWNED_PROVIDER) ? DBMEMOWNER_PROVIDEROWNED : DBMEMOWNER_CLIENTOWNED;
		pBinding->eParamIO		= DBPARAMIO_NOTPARAM;
		
		pBinding->cbMaxLen		= pColAccess->cbMaxLen;
		pBinding->dwFlags		= 0;
		pBinding->wType			= pColAccess->wType;

		pBinding->bPrecision	= pColAccess->bPrecision;
		pBinding->bScale		= pColAccess->bScale;

		//Do we real want this column?		
		switch(eBindCols)
		{
			case BIND_ALLCOLS:
				m_cColAccess++;
				cBindings++;
				break;

			case BIND_ALLCOLSEXPECTBOOKMARK:
				if(pColInfo->iOrdinal != 0)
				{
					m_cColAccess++;
					cBindings++;
				}
				break;

			case BIND_UPDATEABLECOLS:
				if(pColInfo->dwFlags & DBCOLUMNFLAGS_WRITE || pColInfo->dwFlags & DBCOLUMNFLAGS_WRITEUNKNOWN)
				{
					m_cColAccess++;
					cBindings++;
				}
				break;

			default:
				ASSERT(!"Unhandled Type!");
				break;
		}
	}

CLEANUP:
	m_Bindings.Attach(cBindings, rgBindings);
	return hr;
}


/////////////////////////////////////////////////////////////////
// HRESULT CRow::GetColumns
//
/////////////////////////////////////////////////////////////////
HRESULT CRow::GetColumns(ULONG cColAccess, DBCOLUMNACCESS* rgColAccess)
{
	HRESULT hr = S_OK;

	if(m_pIRow == NULL)
		return E_FAIL;

	//GetColumns
	XTEST(hr = m_pIRow->GetColumns(cColAccess, rgColAccess));
	TESTC(TRACE_METHOD(hr, L"IRow::GetColumns(%lu, 0x%p)", cColAccess, rgColAccess));

CLEANUP:
	return hr;
}



/////////////////////////////////////////////////////////////////
// HRESULT CRow::Open
//
/////////////////////////////////////////////////////////////////
HRESULT CRow::Open(CAggregate* pCAggregate, DBID* pColumnID, REFGUID rguidObjectType, REFIID riid, IUnknown** ppIUnknown)
{
	HRESULT hr = S_OK;

	if(m_pIRow == NULL)
		return E_FAIL;
	
	//Convert the DBID to a string for display...
	WCHAR wszBuffer[MAX_QUERY_LEN+1] = {0};
	DBIDToString(pColumnID, wszBuffer, MAX_QUERY_LEN);

	//Open
	XTEST(hr = m_pIRow->Open(pCAggregate, pColumnID, rguidObjectType, 0, riid, ppIUnknown));
	TESTC(TRACE_METHOD(hr, L"IRow::Open(0x%p, %s, %s, 0, %s, &0x%p)", pCAggregate, wszBuffer, GetObjectTypeName(rguidObjectType), GetInterfaceName(riid), ppIUnknown ? *ppIUnknown : NULL));

	//Handle Aggregation
	if(pCAggregate)
		TESTC(hr = pCAggregate->HandleAggregation(riid, ppIUnknown));

CLEANUP:
	return hr;
}


					

/////////////////////////////////////////////////////////////////
// HRESULT CRow::Bind
//
/////////////////////////////////////////////////////////////////
HRESULT CRow::Bind(CAggregate* pCAggregate, WCHAR* pwszURL, DBBINDURLFLAG dwBindFlags, REFGUID rguidObjectType, REFIID riid, IUnknown** ppIUnknown)
{
	HRESULT hr = S_OK;
	DBBINDSTATUS dwBindStatus = 0;

	if(m_pIBindResource == NULL)
		return E_FAIL;

	//Open
	XTEST(hr = m_pIBindResource->Bind(pCAggregate, pwszURL, dwBindFlags, rguidObjectType,  riid, NULL, NULL, &dwBindStatus, ppIUnknown));
	TESTC(TRACE_METHOD(hr, L"IBindResource::Bind(0x%p, \"%s\", 0x%08x, %s, %s, NULL, NULL, &0x%p, &0x%p)", pCAggregate, pwszURL, dwBindFlags, GetObjectTypeName(rguidObjectType), GetInterfaceName(riid), &dwBindStatus, ppIUnknown ? *ppIUnknown : NULL));

	//Handle Aggregation
	if(pCAggregate)
		TESTC(hr = pCAggregate->HandleAggregation(riid, ppIUnknown));

CLEANUP:
	return hr;
} //CRow::Bind



