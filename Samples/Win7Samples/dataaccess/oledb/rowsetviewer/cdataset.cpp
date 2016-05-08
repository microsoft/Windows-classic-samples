//-----------------------------------------------------------------------------
// Microsoft OLE DB RowsetViewer
// Copyright (C) 1994 - 1999 By Microsoft Corporation.
//
// @doc
//
// @module CDATASET.CPP
//
//-----------------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////////
// Includes
//
//////////////////////////////////////////////////////////////////////////////
#include "Headers.h"


/////////////////////////////////////////////////////////////////
// CDataset::CDataset
//
/////////////////////////////////////////////////////////////////
CDataset::CDataset(CMainWindow* pCMainWindow, CMDIChild* pCMDIChild) 
	: CDataAccess(eCDataset, pCMainWindow, pCMDIChild)
{
	//Dataset
	m_pIMDDataset		= NULL;
	m_pIMDFind			= NULL;
	m_pIMDRangeRowset	= NULL;

	//Data
	m_cAxis				= 0;
	m_rgAxisInfo		= NULL;
}


/////////////////////////////////////////////////////////////////
// CDataset::~CDataset
//
/////////////////////////////////////////////////////////////////
CDataset::~CDataset()
{
	ReleaseObject(0);
}


/////////////////////////////////////////////////////////////////
// CDataset::AutoRelease
//
/////////////////////////////////////////////////////////////////
HRESULT CDataset::AutoRelease()
{
	HRESULT hr = S_OK;

	//Free the AxisInfo
	FreeAxisInfo(&m_cAxis, &m_rgAxisInfo);

	//Interface
	RELEASE_INTERFACE(IMDDataset);
	RELEASE_INTERFACE(IMDFind);
	RELEASE_INTERFACE(IMDRangeRowset);

	//Delegate
	return CDataAccess::AutoRelease();
}


////////////////////////////////////////////////////////////////
// CDataset::AutoQI
//
/////////////////////////////////////////////////////////////////
HRESULT CDataset::AutoQI(DWORD dwCreateOpts)
{
	//Delegate First so we have base interfaces
	CDataAccess::AutoQI(dwCreateOpts);

	//[MANDATORY]
	if(dwCreateOpts & CREATE_QI_MANDATORY)
	{
		OBTAIN_INTERFACE(IMDDataset);
	}
	
	//[OPTIONAL]
	if(dwCreateOpts & CREATE_QI_OPTIONAL)
	{
		OBTAIN_INTERFACE(IMDFind);
		OBTAIN_INTERFACE(IMDRangeRowset);
	}

	return S_OK;
}


/////////////////////////////////////////////////////////////////
// IUnknown** CDataset::GetInterfaceAddress
//
/////////////////////////////////////////////////////////////////
IUnknown** CDataset::GetInterfaceAddress(REFIID riid)
{
	HANDLE_GETINTERFACE(IMDDataset);
	HANDLE_GETINTERFACE(IMDFind);
	HANDLE_GETINTERFACE(IMDRangeRowset);

	//Otherwise delegate
	return CDataAccess::GetInterfaceAddress(riid);
}


/////////////////////////////////////////////////////////////////
//	GetAxisInfo
//
//	Get the axis info for all axis in the current dataset.
//	Return count and array of axis info.
//
/////////////////////////////////////////////////////////////////
HRESULT	CDataset::GetAxisInfo(DBCOUNTITEM* pcAxis, MDAXISINFO** prgAxisInfo)
{
	HRESULT hr = E_FAIL;
	
	if(m_pIMDDataset)
	{
		//IMDDataset::GetAxisInfo
		XTEST(hr = m_pIMDDataset->GetAxisInfo(pcAxis, prgAxisInfo));
		TESTC(TRACE_METHOD(hr, L"IMDDataset::GetAxisInfo(%lu, &0x%p)", pcAxis ? *pcAxis : 0, prgAxisInfo ? *prgAxisInfo : NULL));
	}

CLEANUP:
	return hr;    
}


/////////////////////////////////////////////////////////////////
//	FreeAxisInfo
//
/////////////////////////////////////////////////////////////////
HRESULT	CDataset::FreeAxisInfo(DBCOUNTITEM* pcAxis, MDAXISINFO** prgAxisInfo)
{
	HRESULT hr = E_FAIL;
	ASSERT(pcAxis && prgAxisInfo);
	
	if(m_pIMDDataset)
	{
		//IMDDataset::FreeAxisInfo
		XTEST(hr = m_pIMDDataset->FreeAxisInfo(*pcAxis, *prgAxisInfo));
		TESTC(TRACE_METHOD(hr, L"IMDDataset::FreeAxisInfo(%lu, &0x%p)", *pcAxis, *prgAxisInfo));
	}

	//NULL output pointers
	*pcAxis			= 0;
	*prgAxisInfo	= NULL;

CLEANUP:
	return hr;
}


	
/////////////////////////////////////////////////////////////////
// HRESULT CDataset::GetCellData
//
/////////////////////////////////////////////////////////////////
HRESULT CDataset::GetCellData(DBORDINAL ulStartCell, DBORDINAL ulEndCell)
{
	HRESULT hr = E_FAIL;
	
	if(m_pIMDDataset)
	{
		//NOTE: Using cbRowSize of IAccessor::CreateAccessor
		//Because this method fetches properties for more than one cell, 
		//the provider should know how long each row is. In this context, a "row" means the area 
		//allocated in the consumer's buffer to hold all properties pertaining to one cell. 
		//This information should be given in the cbRowSize parameter of IAccessor::CreateAccessor. 
		//If the value of this parameter is zero, the consumer wants to fetch only one row of data 
		//(one cell). In this case, it is an error to specify a ulStartCell different from the 
		//ulEndCell.
		
		//Because of the above in the OLAP spec, we need to make sure our buffer is large enough
		//to hold the number of requested cells...
		if(ulEndCell > ulStartCell && m_cbRowSize)
			SAFE_REALLOC(m_pData, BYTE, m_cbRowSize * (ulEndCell - ulStartCell + 1));

		//IMDDataset::GetCellData
		XTEST(hr = m_pIMDDataset->GetCellData(m_hAccessor, ulStartCell, ulEndCell, m_pData));
		TESTC(TRACE_METHOD(hr, L"IMDDataset::GetCellData(0x%p, 0x%Id, 0x%Id, 0x%p)", m_hAccessor, ulStartCell, ulEndCell, m_pData));
 	}

CLEANUP:
	return hr;
}

////////////////////////////////////////////////////////////////
//	GetAxisRowset
//
//	Get the axis info and rowset for the first axis.
//	Return IRowset interface for the axis rowset.
//
/////////////////////////////////////////////////////////////////
HRESULT	CDataset::GetAxisRowset(CAggregate* pCAggregate, DBCOUNTITEM iAxis, REFIID riid, ULONG cPropSets, DBPROPSET* rgPropSets, IUnknown** ppIUnknown)
{
	HRESULT hr = E_FAIL;

	if(m_pIMDDataset)
	{
		//GetAxisInfo (if not already)
		if(m_cAxis == 0)
			TESTC(hr = GetAxisInfo(&m_cAxis, &m_rgAxisInfo));
			
		//GetAxisRowset
		if(m_cAxis)
		{
			XTEST_(hr = m_pIMDDataset->GetAxisRowset(
											pCAggregate, 
											iAxis, 
											riid, 
											cPropSets, 
											rgPropSets, 
											ppIUnknown),S_OK);
			TRACE_METHOD(hr, L"IMDDataset::GetAxisRowset(0x%p, %d, %s, %d, 0x%p, &0x%p)", pCAggregate, iAxis, GetInterfaceName(riid), cPropSets, rgPropSets, ppIUnknown ? *ppIUnknown : NULL);
		}

		//Display Errors (if occurred)
		TESTC(hr = DisplayPropErrors(hr, cPropSets, rgPropSets));

		//Handle Aggregation
		if(pCAggregate)
			TESTC(hr = pCAggregate->HandleAggregation(riid, ppIUnknown));
	}

CLEANUP:
	return hr;    
}


/////////////////////////////////////////////////////////////////
// HRESULT CDataset::DisplayObject
//
/////////////////////////////////////////////////////////////////
HRESULT CDataset::DisplayObject()
{
	HRESULT hr = S_OK;

	BINDCOLS eBindCols = (GetOptions()->m_dwAccessorOpts & ACCESSOR_BIND_BOOKMARK ? BIND_ALLCOLS : BIND_ALLCOLSEXPECTBOOKMARK);
	if(m_pCMDIChild)
	{
		//Reset Cursor
		m_pCMDIChild->m_pCDataGrid->m_fLastFetchForward = FALSE;
		m_pCMDIChild->m_pCDataGrid->m_lCurPos = 0;

		//First Clear the existing Window...
		TESTC(hr = m_pCMDIChild->m_pCDataGrid->ClearAll());
	}

	//Create ColumnInfo
	TESTC(hr = GetColInfo());

	//Create Accessors and Setup bindings
	TESTC(hr = CreateAccessors(eBindCols));

	if(m_pCMDIChild)
	{
		//Refresh the Columns and Rows
		TESTC(hr = m_pCMDIChild->m_pCDataGrid->RefreshData());
	}

	//Display the object...
	TESTC(hr = CDataAccess::DisplayObject());

CLEANUP:
	if(m_pCMDIChild)
		m_pCMDIChild->UpdateControls();
	return hr;
}
