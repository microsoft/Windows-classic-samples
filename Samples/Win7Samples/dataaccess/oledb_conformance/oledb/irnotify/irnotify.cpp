//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright (C) 1995-2000 Microsoft Corporation
//
// @doc  
//
// @module IRNotify.cpp | This module is a general library for interface testing 
//


///////////////////////////////////////////////////////////////////
// Includes
//
///////////////////////////////////////////////////////////////////
#define  DBINITCONSTANTS	// Must be defined to initialize constants in OLEDB.H
#define  INITGUID

#include "MODStandard.hpp"
#include "IRNotify.h"


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Module Values
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// {{ TCW_MODULE_GLOBALS
DECLARE_MODULE_CLSID = { 0x7cd27cb8, 0xa1f8, 0x11d0, { 0x94, 0xda, 0x00, 0xc0, 0x4f, 0xb6, 0x6a, 0x50 }};
DECLARE_MODULE_NAME("IRowsetNotify");
DECLARE_MODULE_OWNER("Microsoft");
DECLARE_MODULE_DESCRIP("Test IRowsetNotify Interface");
DECLARE_MODULE_VERSION(795921705);
// TCW_WizardVersion(2)
// TCW_Automation(True)
// }} TCW_MODULE_GLOBALS_END


//--------------------------------------------------------------------
// @func Module level initialization routine
//
// @rdesc Success or Failure
// 		@flag  TRUE  | Successful initialization
//		@flag  FALSE | Initialization problems
//
BOOL ModuleInit(CThisTestModule* pThisTestModule)
{
    return CommonModuleInit(pThisTestModule, IID_IRowsetNotify, SIZEOF_TABLE, ROWSET_INTERFACE);
}	
  
//--------------------------------------------------------------------
// @func Module level termination routine
//
// @rdesc Success or Failure
// 		@flag  TRUE  | Successful initialization
//		@flag  FALSE | Initialization problems
//
BOOL ModuleTerminate(CThisTestModule* pThisTestModule)
{
	return CommonModuleTerminate(pThisTestModule);
}	


///////////////////////////////////////////////////////////////////////////////
// Helper Functions
//
///////////////////////////////////////////////////////////////////////////////

BOOL IsValidEvent(DBREASON eReason, DBEVENTPHASE ePhase)
{
	//This check is a little more than just making sure eReason/ePhase
	//fall into a valid range.  Some reasons only have 1 phase, while others
	//have all phases...

	//So this comparion is, "is this a valid phase for this reason"...

	switch(eReason)
	{
		//All Phases can occur
		case DBREASON_ROWSET_FETCHPOSITIONCHANGE:
		case DBREASON_ROWSET_CHANGED:
		case DBREASON_COLUMN_SET:
		case DBREASON_ROW_DELETE:
		case DBREASON_ROW_FIRSTCHANGE:
		case DBREASON_ROW_INSERT:
		case DBREASON_ROW_RESYNCH:
		case DBREASON_ROW_UNDOCHANGE:
		case DBREASON_ROW_UNDOINSERT:
		case DBREASON_ROW_UNDODELETE:
		case DBREASON_ROW_UPDATE:
			switch(ePhase)
			{
				case DBEVENTPHASE_OKTODO:
				case DBEVENTPHASE_ABOUTTODO:
				case DBEVENTPHASE_SYNCHAFTER:
				case DBEVENTPHASE_DIDEVENT:
				case DBEVENTPHASE_FAILEDTODO:
					return TRUE;
				
				default:
					break;
			}

		//DIDEVENT can occur
		case DBREASON_ROWSET_RELEASE:
		case DBREASON_COLUMN_RECALCULATED:
		case DBREASON_ROW_ACTIVATE:
		case DBREASON_ROW_RELEASE:
			switch(ePhase)
			{
				case DBEVENTPHASE_DIDEVENT:
					return TRUE;

				default:
					break;
			}
	}

	return FALSE;
}


REASONTYPE ReasonType(DBREASON eReason)
{
	ASSERT(IsValidEvent(eReason));

	switch(eReason)
	{
		//All Phases can occur
		case DBREASON_ROWSET_FETCHPOSITIONCHANGE:
		case DBREASON_ROWSET_CHANGED:
		case DBREASON_ROWSET_RELEASE:
			return REASONTYPE_ROWSET;
		
		case DBREASON_COLUMN_SET:
		case DBREASON_COLUMN_RECALCULATED:
			return REASONTYPE_COLUMN;

		case DBREASON_ROW_DELETE:
		case DBREASON_ROW_FIRSTCHANGE:
		case DBREASON_ROW_INSERT:
		case DBREASON_ROW_RESYNCH:
		case DBREASON_ROW_UNDOCHANGE:
		case DBREASON_ROW_UNDOINSERT:
		case DBREASON_ROW_UNDODELETE:
		case DBREASON_ROW_UPDATE:
		case DBREASON_ROW_ACTIVATE:
		case DBREASON_ROW_RELEASE:
			return REASONTYPE_ROW;
	}

	return REASONTYPE_INVALID;
}


BOOL IsMultiPhaseReason(DBREASON eReason)
{
	ASSERT(IsValidEvent(eReason));

	switch(eReason)
	{
		//All Single Phase Reasons
		case DBREASON_ROWSET_RELEASE:
		case DBREASON_COLUMN_RECALCULATED:
		case DBREASON_ROW_ACTIVATE:
		case DBREASON_ROW_RELEASE:
			return FALSE;
	}

	return TRUE;
}


BOOL DisplayNotification(WCHAR* pwszHeader, DBCOUNTITEM cRows, const HROW rghRows[], DBORDINAL cColumns, DBORDINAL* rgColumns, DBREASON eReason, DBEVENTPHASE ePhase, BOOL fError)
{
	//Header
	TOUTPUT_(pwszHeader);

	//Reason
	TOUTPUT_(L" - " << GetReasonDesc(eReason));				

	//Phase
	TOUTPUT_(L" - " << GetPhaseDesc(ePhase));

	//cRows
	if(cRows)
	{
		TOUTPUT_(L" - cRows - " << cRows);
		TOUTPUT_(L"[");
		for(DBCOUNTITEM i=0; i<cRows; i++)
		{
			if(rghRows)
				TOUTPUT_(rghRows[i]);
			if(i < cRows-1)
				TOUTPUT_(",");
		}
		TOUTPUT_(L"]");
	}

	//cColumns
	if(cColumns)
	{
		TOUTPUT_(L" - cColumns - " << cColumns);
		TOUTPUT_(L"[");
		for(DBORDINAL i=0; i<cColumns; i++)
		{
			if(rgColumns)
				TOUTPUT_(rgColumns[i]);
			if(i < cColumns-1)
				TOUTPUT_(",");
		}
		TOUTPUT_(L"]");
	}

	//End on line
	TOUTPUT("");

	//Increment Error Count (if an error)
	if(fError)
		(*(GetModInfo()->GetErrorObject()))++;
	return TRUE;
}


BOOL IsSameRow(IUnknown* pIUnkRowset, HROW hRow1, HROW hRow2)
{
	TBEGIN
	IRowsetIdentity* pIRowsetIdentity = NULL;

	if(hRow1 != hRow2)
	{
		//Obtain the IRowsetIdentity interface
		TESTC_(QI(pIUnkRowset, IID_IRowsetIdentity, (void**)&pIRowsetIdentity),S_OK);

		//See of these rows are equal...
		QTESTC_(pIRowsetIdentity->IsSameRow(hRow1, hRow2),S_OK);
	}

CLEANUP:
	SAFE_RELEASE(pIRowsetIdentity);
	TRETURN
}


	
///////////////////////////////////////////////////////////////////////////////
// COMMAND
//
///////////////////////////////////////////////////////////////////////////////
COMMAND::COMMAND
(
	COMMANDTYPE eCommandType		
)
{
	m_eCommandType		= eCommandType;
	m_pvObjectId		= NULL;
	m_ulCommandLevel	= 0;
	m_cThreads			= 0;

	m_eReason			= 0;
	m_ePhase			= 0;
	
	m_cRows				= 0;
	m_cColumns			= 0;
	m_rgColumns			= NULL;
	m_dwOption			= 0;
	
	m_hrExpected        = S_OK;
	m_fCanceled	        = FALSE;
}

COMMAND::COMMAND
(
	COMMANDTYPE eCommandType,	
	void* pvObjectId,			
	ULONG ulCommandLevel,			
	ULONG cThreads,
	DBREASON eReason,			
	DBEVENTPHASE ePhase,		
	HRESULT hrExpected,
	DBCOUNTITEM cRows,				
	ULONG rgRowIds[],			
	DBORDINAL cColumns,				
	ULONG rgColumns[],
	DWORD dwOption
)
{
	ASSERT(pvObjectId);

	m_pvObjectId	= pvObjectId;
	m_eCommandType	= eCommandType;
	m_ulCommandLevel= ulCommandLevel;
	m_cThreads		= cThreads;

	m_eReason = eReason;
	m_ePhase = ePhase;

	m_cRows = cRows;
	memset(m_rgRowIds, 0, MAX_ROW_COUNT * sizeof(DBCOUNTITEM));
	memset(m_rghRows, 0, MAX_ROW_COUNT * sizeof(HROW));

	DBCOUNTITEM sum = 0;
	DBCOUNTITEM i = 0;

	for(i=0; i<cRows; i++)
	{
		m_rgRowIds[i] = rgRowIds[i];
		sum = sum + rgRowIds[i];
	}

	for(i=cRows; i<MAX_ROW_COUNT; i++)
	{
		m_rgRowIds[i] = sum + i-cRows+1;
	}
	
	m_cColumns = cColumns;
	SAFE_ALLOC(m_rgColumns, DBORDINAL, cColumns);
	memset(m_rgColumns, 0, (size_t)(cColumns*sizeof(DBORDINAL)));
	for(i=0; i<cColumns; i++)
		m_rgColumns[i] = rgColumns[i];

	m_dwOption		= dwOption;
	m_hrExpected	= hrExpected;
	m_fCanceled	    = FALSE;

CLEANUP:
	;
}

COMMAND::~COMMAND()
{
	SAFE_FREE(m_rgColumns);
}


BOOL COMMAND::DisplayCommand()
{
	TOUTPUT_("Unreached Command - ");
	
	switch(m_eCommandType)
	{
	  case RETURN_ACCEPT:			TOUTPUT_("RETURN_ACCEPT"); break;
	  case RETURN_VETO:				TOUTPUT_("RETURN_VETO"); break;

	  case RETURN_EFAIL:			TOUTPUT_("RETURN_EFAIL"); break;
	  case RETURN_EOUTOFMEMORY:		TOUTPUT_("RETURN_EOUTOFMEMORY"); break;
	  case RETURN_EINVALIDARG:		TOUTPUT_("RETURN_EINVALIDARG"); break;
	  
	  case RETURN_UNWANTEDREASON:   TOUTPUT_("RETURN_UNWANTEDREASON"); break;
	  case RETURN_UNWANTEDPHASE:	TOUTPUT_("RETURN_UNWANTEDPHASE"); break; 
	  
	  case RETURN_ADVISE:			TOUTPUT_("RETURN_ADVISE"); break; 
	  case RETURN_UNADVISE:			TOUTPUT_("RETURN_UNADVISE"); break; 

	  default:
		  TOUTPUT_("OLEDB Method ");
		  break;
	}

	//Display the rest of the info
	return DisplayNotification(NULL, m_cRows, m_rghRows, m_cColumns, m_rgColumns, m_eReason, m_ePhase, FALSE);
}


///////////////////////////////////////////////////////////////////////////////
// ROWINFO
//
///////////////////////////////////////////////////////////////////////////////
ROWINFO::ROWINFO(HROW hRow)
{
	m_eRowStatus	= ROWSTATUS_NOCHANGE;
	
	m_hRow			= hRow;
	m_cRefCount		= 1;
	
	m_cbBookmark	= 0;
	m_pBookmark		= NULL;
	m_fFirstChange	= TRUE;

	m_pNewData		= NULL;
	m_pOrgData		= NULL;
}

ROWINFO::~ROWINFO()
{
	PROVIDER_FREE(m_pBookmark);
	PROVIDER_FREE(m_pNewData);
	PROVIDER_FREE(m_pOrgData);
}


///////////////////////////////////////////////////////////////////////////////
// EXPECT (Constructor)
//
///////////////////////////////////////////////////////////////////////////////
EXPECT::EXPECT(CImpIListener* pListener, IRowset* pIRowset, DBCOUNTITEM cRows, HROW* rghRows, DBORDINAL cColumns, DBORDINAL rgColumns[], DBREASON eReason, DBEVENTPHASE ePhase)
{
	ASSERT(pListener);
	ASSERT(pIRowset);
	DBCOUNTITEM i = 0;
	m_pListener = pListener;
	m_pIRowset  = pIRowset;

	//Rows
	m_cRows = cRows;
	memset(m_rghRows, 0, sizeof(m_rghRows));

	for(i=0; i<cRows; i++)
		m_rghRows[i] = rghRows[i];
	
	//Columns
	m_cColumns = cColumns;
	SAFE_ALLOC(m_rgColumns, DBORDINAL, cColumns);
	memset(m_rgColumns, 0, (size_t)(cColumns*sizeof(DBORDINAL)));
	for(i=0; i<cColumns; i++)
		m_rgColumns[i] = rgColumns[i];

	//Notification
	m_eReason  = eReason;
	m_ePhase   = ePhase;

	m_ulTimesNotified = 0;
	m_fCanceled	      = FALSE;
	m_bCompleted      = FALSE;
	m_fOptional	      = FALSE;

CLEANUP:
	;
}

///////////////////////////////////////////////////////////////////////////////
// EXPECT (Copy Constructor)
//
///////////////////////////////////////////////////////////////////////////////
EXPECT::EXPECT(const EXPECT& rExpect)
{
	m_pListener = rExpect.m_pListener;
	m_pIRowset  = rExpect.m_pIRowset;
	DBCOUNTITEM i = 0;
	//Rows
	m_cRows = rExpect.m_cRows;

	for(i=0; i<rExpect.m_cRows; i++)
		m_rghRows[i] = rExpect.m_rghRows[i];
	
	//Columns
	m_cColumns = rExpect.m_cColumns;
	SAFE_ALLOC(m_rgColumns, DBORDINAL, m_cColumns);
	memset(m_rgColumns, 0, (size_t)(m_cColumns*sizeof(DBORDINAL)));
	for(i=0; i<rExpect.m_cColumns; i++)
		m_rgColumns[i] = rExpect.m_rgColumns[i];

	//Notification
	m_eReason  = rExpect.m_eReason;
	m_ePhase   = rExpect.m_ePhase;

	m_ulTimesNotified = rExpect.m_ulTimesNotified;
	m_fCanceled	      = rExpect.m_fCanceled;
	m_bCompleted      = rExpect.m_bCompleted;
	m_fOptional	      = rExpect.m_fOptional;

CLEANUP:
	;
}

EXPECT::~EXPECT()
{
	SAFE_FREE(m_rgColumns);
}

BOOL EXPECT::SetPhase(DBEVENTPHASE ePhase)
{
	m_ePhase = ePhase;
	return TRUE;
}

BOOL EXPECT::SetNextPhase()
{
	if(m_pListener->NextPhase(m_eReason, m_ePhase))
	{
		SetPhase(m_pListener->NextPhase(m_eReason, m_ePhase));
	}
	else
	{
		m_bCompleted = TRUE;
	}

	return TRUE;
}

BOOL EXPECT::IsEqual(DBEVENTPHASE ePhase)
{
	return IsEqual(m_pListener, m_pIRowset, m_cRows, m_rghRows, m_cColumns, m_rgColumns, m_eReason, ePhase);
}
BOOL EXPECT::IsEqual(CImpIListener* pListener, IRowset* pIRowset, DBCOUNTITEM cRows, HROW* rghRows, DBORDINAL cColumns, DBORDINAL rgColumns[], DBREASON eReason, DBEVENTPHASE ePhase)
{
	TBEGIN
	ASSERT(pListener);
	ASSERT(pIRowset);
	DBORDINAL i;

	//Listener
	QTESTC(m_pListener == pListener);
	
	//IRowset
	QTESTC(m_pIRowset == pIRowset);
	
	//Rows
	QTESTC(m_cRows == cRows);
	for(i=0; i<cRows; i++)
		QTESTC(IsSameRow(m_pIRowset, m_rghRows[i], rghRows[i]));
	
	//Columns
	QTESTC(m_cColumns == cColumns);
	for(i=0; i<cColumns; i++)
		QTESTC(m_rgColumns[i] == rgColumns[i]);

	//Notification
	QTESTC(m_eReason == eReason);
	QTESTC(m_ePhase  == ePhase);

CLEANUP:
	TRETURN
}

BOOL EXPECT::VerifyEqual(DBEVENTPHASE ePhase)
{
//	return VerifyEqual(m_pListener, m_pIRowset, m_cRows, m_rghRows, m_cColumns, m_rgColumns, m_eReason, ePhase);
	if (!IsEqual(ePhase))
	{
		DisplayNotification(L"Expected:     ", m_cRows, m_rghRows, m_cColumns, m_rgColumns, m_eReason, ePhase);
		return FALSE;
	}

	return TRUE;

}

BOOL EXPECT::VerifyEqual(CImpIListener* pListener, IRowset* pIRowset, DBCOUNTITEM cRows, HROW rghRows[], DBORDINAL cColumns, DBORDINAL rgColumns[], DBREASON eReason, DBEVENTPHASE ePhase)
{
	ASSERT(pListener);
	ASSERT(pIRowset);

	if(!IsEqual(pListener, pIRowset, cRows, rghRows, cColumns, rgColumns, eReason, ePhase))
	{
		DisplayNotification(L"Unexpected:     ", cRows, rghRows, cColumns, rgColumns, eReason, ePhase);
		return FALSE;
	}

	return TRUE;
}

BOOL EXPECT::DisplayExpectation(WCHAR* pwszTitle)
{
	return DisplayNotification(pwszTitle, m_cRows, m_rghRows, m_cColumns, m_rgColumns, m_eReason, m_ePhase);
}

BOOL EXPECT::VerifyComplete()
{
	TBEGIN
	if(m_pListener->IsWantedReason(m_eReason))
	{
		if(!m_pListener->IsWantedPhase(m_eReason, DBEVENTPHASE_DIDEVENT))
		{
			if(IsMultiPhaseReason(m_eReason) && m_ulTimesNotified==0)
				DisplayExpectation();
			QTESTC(VerifyEqual(DBEVENTPHASE_DIDEVENT));
		}
		else
		{
			//We should have been notified for this reason
			//If not we have a problem, display expected notification
			
			//The only exception to the rule is ROW_RELEASE.
			//Some providers may not have exact refcounts, or the rows may always
			//be active, (row count always 1), so just indicate warning...

			//The other exception to the rule is DBREASON_COLUMN_RECALCULATED
			//Providers are not required to support notifications for this reason (see MDAC bug 16595)
			if(m_eReason == DBREASON_ROW_RELEASE)
			{
				if(m_ulTimesNotified == 0 || !m_bCompleted)
					TWARNING(L"ROW_RELEASE never called, Row RefCounts may not be exact...");
				QTESTC(VerifyEqual(DBEVENTPHASE_DIDEVENT));
			}
			else if (m_eReason == DBREASON_COLUMN_RECALCULATED)
			{	if(m_ulTimesNotified == 0 || !m_bCompleted)
					TWARNING(L"Did not receive notifications for DBREASON_COLUMN_RECALCULATED ...");
				QTESTC(VerifyEqual(DBEVENTPHASE_DIDEVENT));
			}
			else if (m_eReason == DBREASON_ROW_ACTIVATE)
			{	if(m_ulTimesNotified == 0)
				{	
					DBCOUNTITEM cRowsExpected = 0;
					for (DBCOUNTITEM i=0; i<m_cRows; i++)
						if (m_rghRows[i] == DB_NULL_HROW)
							cRowsExpected++;
					if(cRowsExpected) 
						DisplayExpectation();
				}
			}
			else
			{
				if(m_ulTimesNotified == 0 || !m_bCompleted)
					DisplayExpectation();
				QTESTC(VerifyEqual(DBEVENTPHASE_DIDEVENT));
			}
		}
	}

CLEANUP:
	TRETURN;
}

BOOL EXPECT::VerifyInComplete()
{
	TBEGIN
	if(m_ulTimesNotified)
	{
		QTESTC(VerifyEqual(DBEVENTPHASE_FAILEDTODO));
		if(!m_bCompleted)
			DisplayExpectation();
	}

	if(m_ulTimesNotified == 0)
	{
		if(m_bCompleted)
			DisplayExpectation();

		if(IsMultiPhaseReason(m_eReason))
		{
			QTESTC(VerifyEqual(DBEVENTPHASE_OKTODO));
		}
		else
		{
			QTESTC(VerifyEqual(DBEVENTPHASE_DIDEVENT));
		}
	}

CLEANUP:
	TRETURN;
}

///////////////////////////////////////////////////////////////////////////////
// CRowsetObj
//
///////////////////////////////////////////////////////////////////////////////
CRowsetObj::CRowsetObj()
{
	InitializeCriticalSection(&m_DataCriticalSection);
	
	m_cRowsetRef		= 0;
	m_ulNextFetchPos	= 0;
	m_fTableAltered		= FALSE;
	m_dwRowsetMode		= 0;

	//Initialize all properties
	m_ulSupportedGranularity	= 0;
	m_ulSupportedPhases			= 0;
	m_bReentrantEvents			= TRUE;
	m_bQuickRestart = TRUE;

	//SetData(Updatable) Accessor and bindings.
	m_cUpBindings		= 0;
	m_rgUpBindings		= NULL;
	m_hUpAccessor		= NULL;

	memset(m_rgCancelableEvents, 0, sizeof(m_rgCancelableEvents));
}

CRowsetObj::~CRowsetObj()
{
	//SetData(Updatable) Accessor and bindings.
	FreeAccessorBindings(m_cUpBindings, m_rgUpBindings);
	ReleaseAccessor(m_hUpAccessor);
	
	DeleteCriticalSection(&m_DataCriticalSection);
	DestroyRowData();
}

BOOL CRowsetObj::IsBufferedMode()
{
	return (m_dwRowsetMode & CHANGE_BUFFERRED);
}

HRESULT CRowsetObj::CreateRowset(ROWSET_MODE dwRowsetMode)
{
	HRESULT hr = S_OK;
	m_dwRowsetMode = dwRowsetMode;
		
	//Query for table, default: Select * from table
	EQUERY eQuery = USE_SUPPORTED_SELECT_ALLFROMTBL;

	//Set required properties
	TESTC(SetProperty(DBPROP_CANHOLDROWS));
	TESTC(SetProperty(DBPROP_IConnectionPointContainer));
	TESTC(SetProperty(DBPROP_IRowsetIdentity));

	//EMPTY_ROWSET
	if(dwRowsetMode & EMPTY_ROWSET)
		eQuery = SELECT_EMPTYROWSET;

	//ROWSETSCROLL_ROWSET
	if(dwRowsetMode & ROWSETSCROLL_ROWSET)
		TESTC(SetProperty(DBPROP_IRowsetScroll));
	
	//ROWSETINDEX_ROWSET
	if(dwRowsetMode & ROWSETINDEX_ROWSET)
		TESTC(SetProperty(DBPROP_IRowsetIndex));

	//ROWSETLOCATE_ROWSET
	if(dwRowsetMode & ROWSETLOCATE_ROWSET)
		TESTC(SetProperty(DBPROP_IRowsetLocate));

	//SCROLLBACKWARDS
	if(dwRowsetMode & SCROLLBACKWARDS)
		TESTC(SetProperty(DBPROP_CANSCROLLBACKWARDS));

	//FETCHBACKWARDS
	if(dwRowsetMode & FETCHBACKWARDS)
		TESTC(SetProperty(DBPROP_CANFETCHBACKWARDS));
		
	//RESYNCH_ROWSET
	if(dwRowsetMode & RESYNCH_ROWSET)
		TESTC(SetProperty(DBPROP_IRowsetResynch));

	//REFRESH_ROWSET
	if(dwRowsetMode & REFRESH_ROWSET)
		TESTC(SetProperty(DBPROP_IRowsetRefresh));

	//CHANGE_IMMEDIATE
	if(dwRowsetMode & CHANGE_IMMEDIATE)
		TESTC(SetProperty(DBPROP_IRowsetChange));
		
	//CHANGE_BUFFERRED
	if(dwRowsetMode & CHANGE_BUFFERRED)
	{
		TESTC(SetProperty(DBPROP_IRowsetChange));
		TESTC(SetProperty(DBPROP_IRowsetUpdate));
	}
		
	//CHANGE_QBU
	if(dwRowsetMode & CHANGE_QBU)
	{
		TESTC(SetProperty(DBPROP_IRowsetChange));
		TESTC(SetProperty(KAGPROP_QUERYBASEDUPDATES, DBPROPSET_PROVIDERROWSET));
	}

	//COMPUTED_COLUMNS
	if((dwRowsetMode & COMPUTED_COLUMNS) || (dwRowsetMode & COMPUTED_COLUMNS_INCLUDE))
		eQuery = SELECT_COMPUTEDCOLLIST;

	//delegate to the CRowset object
	if(dwRowsetMode & COMPUTED_COLUMNS_INCLUDE)
	//Need to use ALL_COLS_BOUND to get the computed column. CreateRowset() uses UPDATEABLE_COLS_BOUND by defalt 
		hr = CRowset::CreateRowset(eQuery, IID_IRowset, NULL, DBACCESSOR_ROWDATA, DBPART_ALL, ALL_COLS_BOUND);
	else
		hr = CRowset::CreateRowset(eQuery);

	//Other setup items...
	if(hr==S_OK)
	{
		//Now get the Notification Properties
		GetNotificationProperties();
	
		//We need to obtain bindings for all the updatable(non-index) columns
		//if we are creating a rowset with IRowsetChange.  We do this ahead of time so we know
		//exactly which columns to expect notifications from when SetData(OnFieldChange) is called.
		if (dwRowsetMode & COMPUTED_COLUMNS_INCLUDE)
		{
			//We need the only column  - cumputed column and 
			//acording to SELECT_COMPUTEDCOLLIST it should be the last one
			DB_LORDINAL rgColsToBind[1];
			rgColsToBind[0] = m_rgBinding[m_cBindings-1].iOrdinal;

			TESTC_(hr = GetAccessorAndBindings(pIRowset(), DBACCESSOR_ROWDATA, &m_hUpAccessor,
				&m_rgUpBindings, &m_cUpBindings, NULL, DBPART_ALL, USE_COLS_TO_BIND_ARRAY, 
				FORWARD, NO_COLS_BY_REF, NULL, NULL, NULL, DBTYPE_EMPTY, 1, rgColsToBind, 
				NULL, NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, NO_BLOB_COLS),S_OK);
		}
		else	
			TESTC_(hr = GetAccessorAndBindings(pIRowset(), DBACCESSOR_ROWDATA, &m_hUpAccessor,
				&m_rgUpBindings, &m_cUpBindings, NULL, DBPART_ALL, UPDATEABLE_NONINDEX_COLS_BOUND, 
				FORWARD, NO_COLS_BY_REF, NULL, NULL, NULL, DBTYPE_EMPTY, 0, NULL, 
				NULL, NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, NO_BLOB_COLS),S_OK);
	}

CLEANUP:
	return hr;
}


ULONG CRowsetObj::AddRefRowset()
{
	//Keep track how many extra references we have put on the rowset
	//Mainly used to AddRef -> Release testing making sure no notifications
	m_cRowsetRef++;

	//AddRef the rowset so it doesn't fire a notification
	pIRowset()->AddRef();
	return m_cRowsetRef;
}

ULONG CRowsetObj::ReleaseRowset()
{
	if(m_cRowsetRef)
	{
		m_cRowsetRef--;
		pIRowset()->Release();
		return m_cRowsetRef;
	}

	return m_cRowsetRef;
}

BOOL CRowsetObj::GetReentrantEvents()
{
	return m_bReentrantEvents;
}

BOOL CRowsetObj::IsQuickRestart()
{
	return m_bQuickRestart;
}

ULONG_PTR CRowsetObj::GetGranularity()
{
	return m_ulSupportedGranularity;
}

ULONG_PTR CRowsetObj::GetSupportedPhases()
{
	return m_ulSupportedPhases;
}

BOOL CRowsetObj::IsSupportedPhase(DBEVENTPHASE ePhase)
{
	ASSERT(IsValidEvent(DBREASON_ROWSET_FETCHPOSITIONCHANGE, ePhase));

	switch(ePhase)
	{
		case DBEVENTPHASE_OKTODO:
			return ((m_ulSupportedPhases & DBPROPVAL_NP_OKTODO) != 0);
		
		case DBEVENTPHASE_ABOUTTODO:
			return ((m_ulSupportedPhases & DBPROPVAL_NP_ABOUTTODO) != 0);

		case DBEVENTPHASE_SYNCHAFTER:
			return ((m_ulSupportedPhases & DBPROPVAL_NP_SYNCHAFTER) != 0);

		case DBEVENTPHASE_DIDEVENT:
			return ((m_ulSupportedPhases & DBPROPVAL_NP_DIDEVENT) != 0);

		case DBEVENTPHASE_FAILEDTODO:
			return ((m_ulSupportedPhases & DBPROPVAL_NP_FAILEDTODO) != 0);
	}

	return FALSE;
}

BOOL CRowsetObj::GetNotificationProperties()
{
	TBEGIN
	
	//Initialize all property values, incase some are not supported
	m_ulSupportedGranularity	= DBPROPVAL_NT_SINGLEROW;
	m_ulSupportedPhases			= DBPROPVAL_NP_FAILEDTODO | DBPROPVAL_NP_DIDEVENT;
	m_bReentrantEvents			= TRUE;
	m_bQuickRestart					= TRUE;

	VARIANT_BOOL bValue = VARIANT_TRUE;

	//DBPROP_NOTIFICATIONGRANULARITY
	if(SupportedProperty(DBPROP_NOTIFICATIONGRANULARITY, DBPROPSET_ROWSET))
		TCHECK(GetProperty(DBPROP_NOTIFICATIONGRANULARITY, DBPROPSET_ROWSET, &m_ulSupportedGranularity),TRUE);

	//DBPROP_NOTIFICATIONPHASES
	if(SupportedProperty(DBPROP_NOTIFICATIONPHASES, DBPROPSET_ROWSET))
		TCHECK(GetProperty(DBPROP_NOTIFICATIONPHASES, DBPROPSET_ROWSET, &m_ulSupportedPhases),TRUE);

	//DBPROP_REENTRANTEVENTS
	if(SupportedProperty(DBPROP_REENTRANTEVENTS, DBPROPSET_ROWSET))
		TCHECK(GetProperty(DBPROP_REENTRANTEVENTS, DBPROPSET_ROWSET, &bValue),TRUE);
	m_bReentrantEvents = ((bValue==VARIANT_TRUE) ? TRUE : FALSE);

	//DBPROP_QUICKRESTART
	if(SupportedProperty(DBPROP_QUICKRESTART, DBPROPSET_ROWSET))
		TCHECK(GetProperty(DBPROP_QUICKRESTART, DBPROPSET_ROWSET, &bValue),TRUE);
	m_bQuickRestart = ((bValue==VARIANT_TRUE) ? TRUE : FALSE);


	//DBPROP_NOTIFY* Cancelable Events
	GetProperty(DBPROP_NOTIFYROWSETFETCHPOSITIONCHANGE, DBPROPSET_ROWSET, &m_rgCancelableEvents[DBREASON_ROWSET_FETCHPOSITIONCHANGE]);
	GetProperty(DBPROP_NOTIFYROWSETRELEASE,				DBPROPSET_ROWSET, &m_rgCancelableEvents[DBREASON_ROWSET_RELEASE]);
	GetProperty(DBPROP_NOTIFYROWSETCHANGED,				DBPROPSET_ROWSET, &m_rgCancelableEvents[DBREASON_ROWSET_CHANGED]);
	GetProperty(DBPROP_NOTIFYCOLUMNSET,					DBPROPSET_ROWSET, &m_rgCancelableEvents[DBREASON_COLUMN_SET]);
	GetProperty(DBPROP_NOTIFYROWDELETE,					DBPROPSET_ROWSET, &m_rgCancelableEvents[DBREASON_ROW_DELETE]);
	GetProperty(DBPROP_NOTIFYROWFIRSTCHANGE,			DBPROPSET_ROWSET, &m_rgCancelableEvents[DBREASON_ROW_FIRSTCHANGE]);
	GetProperty(DBPROP_NOTIFYROWINSERT,					DBPROPSET_ROWSET, &m_rgCancelableEvents[DBREASON_ROW_INSERT]);
	GetProperty(DBPROP_NOTIFYROWRESYNCH,				DBPROPSET_ROWSET, &m_rgCancelableEvents[DBREASON_ROW_RESYNCH]);
	GetProperty(DBPROP_NOTIFYROWUNDOCHANGE,				DBPROPSET_ROWSET, &m_rgCancelableEvents[DBREASON_ROW_UNDOCHANGE]);
	GetProperty(DBPROP_NOTIFYROWUNDOINSERT,				DBPROPSET_ROWSET, &m_rgCancelableEvents[DBREASON_ROW_UNDOINSERT]);
	GetProperty(DBPROP_NOTIFYROWUNDODELETE,				DBPROPSET_ROWSET, &m_rgCancelableEvents[DBREASON_ROW_UNDODELETE]);
	GetProperty(DBPROP_NOTIFYROWUPDATE,					DBPROPSET_ROWSET, &m_rgCancelableEvents[DBREASON_ROW_UPDATE]);

	TRETURN;
}

BOOL CRowsetObj::IsCancelableEvent(DBREASON eReason, DBEVENTPHASE ePhase)
{
	ASSERT(IsValidEvent(eReason, ePhase));

	switch(ePhase)
	{
		case DBEVENTPHASE_OKTODO:
			return ((m_rgCancelableEvents[eReason] & DBPROPVAL_NP_OKTODO) != 0);
		
		case DBEVENTPHASE_ABOUTTODO:
			return ((m_rgCancelableEvents[eReason] & DBPROPVAL_NP_ABOUTTODO) != 0);

		case DBEVENTPHASE_SYNCHAFTER:
			return ((m_rgCancelableEvents[eReason] & DBPROPVAL_NP_SYNCHAFTER) != 0);
	}

	return FALSE;
}

HRESULT CRowsetObj::SetData(COMMAND* pCommand)
{
	ASSERT(pCommand);
	
	HRESULT hr = S_OK;
	void* pMakeData = NULL;
	IRowsetChange* pIRowsetChange = NULL;
	ROWINFO* pRowInfo = GetRowInfo(pCommand->m_rgRowIds[0]);

	//Make Data for the row
	MakeRowData(&pMakeData, m_hUpAccessor);

	//Obtain IRowsetChange interface...
	TESTC_(hr = QI(pIRowset(), IID_IRowsetChange, (void**)&pIRowsetChange),S_OK);
	
	//The test will sometime expect BADROWHANDLE since it thinks that previous calls may have veto'd
	//an insertion.  Some providers don't allow vetos, so the badrowhandle is dependent upon wither
	//it was successful or not.  If the row is in our cache, then it must have been allowed...
//	if(pCommand->m_hrExpected == DB_E_BADROWHANDLE)
//	{
//		if(pRowInfo && pRowInfo->m_h`Row)
//			pCommand->m_hrExpected = S_OK;
//	}
	
	//Now finally Modify the row...
	//Can return an Error such as DB_E_CANCELED within a listener
	switch(pCommand->m_hrExpected)
	{
		case DB_E_BADACCESSORHANDLE:
			hr = pIRowsetChange->SetData(pCommand->m_rghRows[0], DB_NULL_HACCESSOR, pMakeData);
			break;

		default:
			hr = pIRowsetChange->SetData(pCommand->m_rghRows[0], m_hUpAccessor, pMakeData);
			break;
	};

	//If SetData is successful then we can mark this row as having been changed.  
	if(SUCCEEDED(hr))
	{
		TESTC(pRowInfo != NULL);
		pRowInfo->m_fFirstChange = FALSE;

		//Get the Data for this row
		PROVIDER_FREE(pRowInfo->m_pNewData);
		pRowInfo->m_pNewData = PROVIDER_ALLOC(SIZEOF_ONEROW);
		TESTC_(GetData(pRowInfo->m_hRow, pRowInfo->m_pNewData),S_OK);
	}

CLEANUP:
	ReleaseRowData(pMakeData, m_hUpAccessor); 
	SAFE_RELEASE(pIRowsetChange);
	return hr;
}


HRESULT CRowsetObj::Update(COMMAND* pCommand)
{
	ASSERT(pCommand);
	HRESULT hr = S_OK;
	IRowsetUpdate* pIRowsetUpdate = NULL;

	DBCOUNTITEM cRowsUpdated = 0;
	HROW* rghRowsUpdated = NULL;
	DBROWSTATUS* rgRowStatus = NULL;
	
	//IRowsetUpdate->Update
	TESTC_(hr = pIRowset()->QueryInterface(IID_IRowsetUpdate,(void**)&pIRowsetUpdate),S_OK);

	//Can return an error DB_S/DB_E if canceled from within a listener
	hr = pIRowsetUpdate->Update(NULL, pCommand->m_cRows, pCommand->m_rghRows, &cRowsUpdated, &rghRowsUpdated, &rgRowStatus);
	
	//If the Update succeeded, then we can clear the RowStatus flags
	//only for those rows actually updated
	if(SUCCEEDED(hr))
	{
		//Verify the results first
		if(pCommand->m_cRows)
		{
			TESTC(cRowsUpdated == pCommand->m_cRows);
			TESTC(rgRowStatus != NULL);
		}

		if(pCommand->m_dwOption)
		{
			TESTC(cRowsUpdated == pCommand->m_dwOption);
			TESTC(rgRowStatus != NULL);
		}

		//dwOptions represent the number of pending rows for the (0,NULL) case...
		for(DBCOUNTITEM i=0; i<pCommand->m_cRows || i<pCommand->m_dwOption; i++)
		{
			ROWINFO* pRowInfo = GetRowInfo(pCommand->m_rgRowIds[i]);
			if(pRowInfo == NULL)
				continue;

			//Only update our flags if row was successflly updated
			if(rgRowStatus && rgRowStatus[i] != DBROWSTATUS_S_OK)
				continue;
			
			//Update of a DeletedRow forces the row to no longer be valid
			if(pRowInfo->m_eRowStatus != ROWSTATUS_DELETED)
			{
				//Save the current Data
				PROVIDER_FREE(pRowInfo->m_pOrgData);
				pRowInfo->m_pOrgData = PROVIDER_ALLOC(SIZEOF_ONEROW);
				memmove(pRowInfo->m_pOrgData, pRowInfo->m_pNewData, (size_t) SIZEOF_ONEROW);
				
				//Get the Data for this row
				PROVIDER_FREE(pRowInfo->m_pNewData);
				pRowInfo->m_pNewData = PROVIDER_ALLOC(SIZEOF_ONEROW);
				TESTC_(GetData(pRowInfo->m_hRow, pRowInfo->m_pNewData),S_OK);

				//Verify Update, actually Updated the Values
				TESTC(CompareRowData(pRowInfo->m_pOrgData, pRowInfo->m_pNewData, m_hAccessor));

				//Get the Original Data for this row
				PROVIDER_FREE(pRowInfo->m_pOrgData);
				pRowInfo->m_pOrgData = PROVIDER_ALLOC(SIZEOF_ONEROW);
				TESTC_(GetOriginalData(pRowInfo->m_hRow, pRowInfo->m_pOrgData),S_OK);

				//Verify OrginalData matches current Data after Update
				TESTC(CompareRowData(pRowInfo->m_pOrgData, pRowInfo->m_pNewData, m_hAccessor));
			}

			//Clear the FirstChange status
			pRowInfo->m_fFirstChange = TRUE;

			//Clear the ChangeStatus
			pRowInfo->m_eRowStatus = ROWSTATUS_NOCHANGE;
		}		
	}

CLEANUP:
	SAFE_RELEASE(pIRowsetUpdate);	
	PROVIDER_FREE(rghRowsUpdated);
	PROVIDER_FREE(rgRowStatus);
	return hr;
}


HRESULT CRowsetObj::Undo(COMMAND* pCommand)
{
	ASSERT(pCommand);
	HRESULT hr = S_OK;
	IRowsetUpdate* pIRowsetUpdate = NULL;
	
	DBCOUNTITEM cRowsUndone = 0;
	HROW* rghRowsUndone = NULL;
	DBROWSTATUS* rgRowStatus = NULL;

	//IRowsetUpdate->Update
	TESTC_(hr = pIRowset()->QueryInterface(IID_IRowsetUpdate,(void**)&pIRowsetUpdate),S_OK);
	
	//Can return an error DB_S/DB_E if canceled from within a listener
	hr = pIRowsetUpdate->Undo(NULL, pCommand->m_cRows, pCommand->m_rghRows, &cRowsUndone, &rghRowsUndone, &rgRowStatus);

	//If the Undo succeeded, then we can clear the UndoStatus flags
	if(SUCCEEDED(hr))
	{
		//Verify the results first
		if(pCommand->m_cRows)
		{
			TESTC(cRowsUndone == pCommand->m_cRows);
			TESTC(rgRowStatus != NULL);
		}

		if(pCommand->m_dwOption)
		{
			TESTC(cRowsUndone == pCommand->m_dwOption);
			TESTC(rgRowStatus != NULL);
		}

		//dwOptions represent the number of pending rows for the (0,NULL) case...
		for(DBCOUNTITEM i=0; i<pCommand->m_cRows || i<pCommand->m_dwOption; i++)
		{
			ROWINFO* pRowInfo = NULL;
			TESTC((pRowInfo = GetRowInfo(pCommand->m_rgRowIds[i])) != NULL);
	
			//Only update our flags if row was successflly undone
			if(rgRowStatus && rgRowStatus[i] != DBROWSTATUS_S_OK)
				continue;

			//Undo of an InsertedRow forces the row to no longer be valid
			if(pRowInfo->m_eRowStatus != ROWSTATUS_INSERTED)
			{
				//Get the Data for this row
				PROVIDER_FREE(pRowInfo->m_pNewData);
				pRowInfo->m_pNewData = PROVIDER_ALLOC(SIZEOF_ONEROW);
				TESTC_(GetData(pRowInfo->m_hRow, pRowInfo->m_pNewData),S_OK);

				//Verify Undo, actually undoes the Data
				TESTC(CompareRowData(pRowInfo->m_pOrgData, pRowInfo->m_pNewData, m_hAccessor));
			}
			
			//Clear the FirstChange status
			pRowInfo->m_fFirstChange = TRUE;

			//Clear the ChangeStatus
			pRowInfo->m_eRowStatus = ROWSTATUS_NOCHANGE;

		}
	}

CLEANUP:
	SAFE_RELEASE(pIRowsetUpdate);	
	PROVIDER_FREE(rghRowsUndone);
	PROVIDER_FREE(rgRowStatus);
	return hr;
}


HRESULT CRowsetObj::AddRefRows(COMMAND* pCommand)
{	
	ASSERT(pCommand);
	ROWINFO* pRowInfo = NULL;
	ULONG rgRefCounts[MAX_ROW_COUNT];

	//IRowset::AddRefRows
	HRESULT hr = pIRowset()->AddRefRows(pCommand->m_cRows, pCommand->m_rghRows, rgRefCounts, NULL);

	//Increment reference counts
	if(SUCCEEDED(hr))
	{
		for(DBCOUNTITEM i=0; i<pCommand->m_cRows; i++)
			if(pRowInfo = GetRowInfo(pCommand->m_rgRowIds[i]))
			{
				//Update the reference count.
				pRowInfo->m_cRefCount = rgRefCounts[i];
			}
	}

	return hr;
}

HRESULT CRowsetObj::ReleaseRows(COMMAND* pCommand)
{	
	ASSERT(pCommand);
	ROWINFO* pRowInfo = NULL;
	HRESULT hr = S_OK;
	ULONG rgRefCounts[MAX_ROW_COUNT];

	//IRowset::ReleaseRows
	switch(pCommand->m_hrExpected)
	{
		case E_INVALIDARG:
			hr = pIRowset()->ReleaseRows(pCommand->m_cRows, NULL, NULL, NULL, NULL);
			break;

		default:
			hr = pIRowset()->ReleaseRows(pCommand->m_cRows, pCommand->m_rghRows, NULL, rgRefCounts, NULL);
			break;
	}

	//Store reference counts
	if(SUCCEEDED(hr))
	{
		for(DBCOUNTITEM i=0; i<pCommand->m_cRows; i++)
			if(pRowInfo = GetRowInfo(pCommand->m_rgRowIds[i]))
			{
				//Update the reference count.
				pRowInfo->m_cRefCount = rgRefCounts[i];
			}
	}

	return hr;
}


HRESULT CRowsetObj::InsertRow(COMMAND* pCommand)
{
	ASSERT(pCommand);
	HRESULT hr = S_OK;
	IRowsetChange* pIRowsetChange = NULL;
	ROWINFO* pRowInfo = NULL;

	//Make data for the inserted row
	TESTC(m_mapRowInfo.Lookup(pCommand->m_rgRowIds[0], pRowInfo));
	PROVIDER_FREE(pRowInfo->m_pOrgData);
	pRowInfo->m_pOrgData = PROVIDER_ALLOC(SIZEOF_ONEROW);
	TESTC(MakeRowData(&pRowInfo->m_pOrgData))

	//InsertRow
	//Can return an error such as DB_E_CANCELED inside a listener
	TESTC_(hr = QI(pIRowset(), IID_IRowsetChange, (void**)&pIRowsetChange),S_OK);

	switch(pCommand->m_hrExpected)
	{
		case DB_E_BADACCESSORHANDLE:
			hr = pIRowsetChange->InsertRow(NULL, DB_NULL_HACCESSOR, pRowInfo->m_pOrgData, &pCommand->m_rghRows[0]);
			break;
	
		default:
			hr = pIRowsetChange->InsertRow(NULL, m_hAccessor, pRowInfo->m_pOrgData, &pCommand->m_rghRows[0]);
			break;
	}
	
	//If successful, we have store the row handle!
	if(SUCCEEDED(hr))
	{
		//Verify row handle
		TESTC(pCommand->m_rghRows[0]!=DB_NULL_HROW)

		//Set the row handle
		//ROWINFO* pRowInfo = NULL;
		//TESTC(m_mapRowInfo.Lookup(pCommand->m_rgRowIds[0], pRowInfo));
		pRowInfo->m_hRow = pCommand->m_rghRows[0];

		//Get the Data for this row
		PROVIDER_FREE(pRowInfo->m_pNewData);
		pRowInfo->m_pNewData = PROVIDER_ALLOC(SIZEOF_ONEROW);
		TESTC_(GetData(pRowInfo->m_hRow, pRowInfo->m_pNewData),S_OK);

		//Make sure the newly inserted row matches the original data...
		TESTC(CompareRowData(pRowInfo->m_pNewData/*pGetData*/, pRowInfo->m_pOrgData/*pSetData*/, m_hAccessor, TRUE/*fSetData*/));
	}

CLEANUP:
	SAFE_RELEASE(pIRowsetChange);
	return hr;
}


HRESULT CRowsetObj::GetNextRows(COMMAND* pCommand)
{
	ASSERT(pCommand);
	HRESULT hr = S_OK;

	DBCOUNTITEM cOffset = 0;
	DBCOUNTITEM cRows = pCommand->m_cRows;
	HROW* rghRows = pCommand->m_rghRows;
	DBCOUNTITEM cRowsObtained = 0;
	DBCOUNTITEM i;

	//GetNextRows
	//The Command may want to produce a particualr error condition
	switch(pCommand->m_hrExpected)
	{
		case DB_S_ENDOFROWSET:			
			{
				if (cRows==0)
					hr = pIRowset()->GetNextRows(NULL, 0, MAX_ROW_COUNT, &cRowsObtained, &rghRows);
				else
					hr = pIRowset()->GetNextRows(NULL, 0, cRows, &cRowsObtained, &rghRows);
				pCommand->m_cRows = cRowsObtained;
			}
			break;

		case DB_E_BADSTARTPOSITION:
			hr = pIRowset()->GetNextRows(NULL, MAX_ROW_COUNT, cRows, &cRowsObtained, &rghRows);
			break;

		case E_INVALIDARG:
			hr = pIRowset()->GetNextRows(NULL, 0, cRows, NULL, NULL);
			break;

		default:
			hr = pIRowset()->GetNextRows(NULL, 0, cRows, &cRowsObtained, &rghRows);
			break;
 	}


	//Loop through rows obtained 
	for(i=0; i<cRowsObtained; i++)
	{
		ROWINFO* pRowInfo = NULL;
		if(m_mapRowInfo.Lookup(pCommand->m_rgRowIds[i],pRowInfo))
			SAFE_DELETE(pRowInfo);
		pRowInfo = new ROWINFO(pCommand->m_rghRows[i]);

		//Get the bookmark for this row, if bookmarks are on...
		if(m_dwRowsetMode & ROWSETLOCATE_ROWSET)
			TESTC_(GetBookmark(pRowInfo->m_hRow, pRowInfo),S_OK);

		//Get the Data for this row
		PROVIDER_FREE(pRowInfo->m_pNewData);
		pRowInfo->m_pNewData = PROVIDER_ALLOC(SIZEOF_ONEROW);
		TESTC_(GetData(pRowInfo->m_hRow, pRowInfo->m_pNewData),S_OK);

		//Save the OrgData for this row
		PROVIDER_FREE(pRowInfo->m_pOrgData);
		pRowInfo->m_pOrgData = PROVIDER_ALLOC(SIZEOF_ONEROW);
		TESTC_(GetData(pRowInfo->m_hRow, pRowInfo->m_pOrgData),S_OK);
		
		m_mapRowInfo.SetAt(pCommand->m_rgRowIds[i], pRowInfo);
	}

	//Increment the NextFetchPosition
	if(SUCCEEDED(hr) && cRowsObtained)
		m_ulNextFetchPos += (cOffset + cRowsObtained);

CLEANUP:
	return hr;
}


HRESULT CRowsetObj::RestartPosition(COMMAND* pCommand)
{
	ASSERT(pCommand);
	HRESULT hr = S_OK;

	// Provider may require release of existing rows before restarting 
	// if DBPROP_QUICKRESTART=FALSE even if the provider supports a value of VARIANT_TRUE for DBPROP_CANHOLDROWS
	if (!IsQuickRestart())
	{	
		DBCOUNTITEM cRows = 0;
		HROW rghRows[MAX_ROW_COUNT];
		ROWINFO* pRowInfo = NULL;
	  ULONG rgRefCounts[MAX_ROW_COUNT];

		GetRowsToRelease(MAX_ROW_COUNT, cRows, rghRows);
		if (cRows)
		{
			hr = pIRowset()->ReleaseRows(cRows, rghRows, NULL, rgRefCounts, NULL);
			//Store reference counts
			if(SUCCEEDED(hr))
			{
				POSITION pos = m_mapRowInfo.GetStartPosition();
				while(pos)
				{
					DBCOUNTITEM Key = 0; 
					ROWINFO* pRowInfo = NULL;
					
					m_mapRowInfo.GetNextAssoc(pos, Key, pRowInfo);
					SAFE_DELETE(pRowInfo);
					pRowInfo = new ROWINFO(DB_NULL_HROW);
					m_mapRowInfo.SetAt(Key, pRowInfo);
				}
			}
		}
	}

	//RestartPosition
	hr = pIRowset()->RestartPosition(NULL);

	//Can return DB_S_COMMANDREEXECUTED
	if(hr==DB_S_COMMANDREEXECUTED)
		hr = S_OK;

	//Reset the rowset values
	if(SUCCEEDED(hr))
	{
		m_ulNextFetchPos = 0;
		m_fTableAltered = FALSE;
	}

	return hr;
}


HRESULT CRowsetObj::GetRowsAt(COMMAND* pCommand)
{
	ASSERT(pCommand);
	HRESULT hr = S_OK;
	IRowsetLocate* pIRowsetLocate = NULL;

	DBCOUNTITEM cRowsObtained = 0;
	DBCOUNTITEM cRows = pCommand->m_cRows;
	HROW* rghRows = pCommand->m_rghRows;
	DBCOUNTITEM i=0;

	//Bookmark for this row should be DBBMK_FIRST or DBBMK_LAST
	ULONG Bookmark = pCommand->m_dwOption;
	TESTC(Bookmark==DBBMK_FIRST || Bookmark==DBBMK_LAST);

	//GetRowsAt
	TESTC_(hr = pIRowset()->QueryInterface(IID_IRowsetLocate,(void**)&pIRowsetLocate),S_OK);
	
	//Can return an error if canceled from within a listener
	//Standard BMK size is 1 byte for DBBMK_FIRST or DBBMK_LAST
	switch(pCommand->m_hrExpected)
	{
		case DB_E_BADSTARTPOSITION:
			hr = pIRowsetLocate->GetRowsAt(NULL, NULL, sizeof(BYTE), (BYTE*)&Bookmark, MAX_ROW_COUNT, cRows, &cRowsObtained, &rghRows);
			break;
		
		case DB_S_ENDOFROWSET:			
			hr = pIRowsetLocate->GetRowsAt(NULL, NULL, sizeof(BYTE), (BYTE*)&Bookmark, 0, MAX_ROW_COUNT, &cRowsObtained, &rghRows);
			break;

		default:
			hr = pIRowsetLocate->GetRowsAt(NULL, NULL, sizeof(BYTE), (BYTE*)&Bookmark, 0, cRows, &cRowsObtained, &rghRows);
			break;
	}

	//Verify results
	if(hr==S_OK)
		TESTC(cRows == cRowsObtained);

	//Loop through rows obtained 
	for(i=0; i<cRowsObtained; i++)
	{
		ROWINFO* pRowInfo = GetRowInfo(pCommand->m_rgRowIds[i]);
		SAFE_DELETE(pRowInfo);
		pRowInfo = new ROWINFO(pCommand->m_rghRows[i]);

		//Get the bookmark for this row
		if(m_dwRowsetMode & ROWSETLOCATE_ROWSET)
			TESTC_(GetBookmark(pRowInfo->m_hRow, pRowInfo),S_OK);

		//Get the Data for this row
		PROVIDER_FREE(pRowInfo->m_pNewData);
		pRowInfo->m_pNewData = PROVIDER_ALLOC(SIZEOF_ONEROW);
		TESTC_(GetData(pRowInfo->m_hRow, pRowInfo->m_pNewData),S_OK);
		
		//Save the Data for this row
		PROVIDER_FREE(pRowInfo->m_pOrgData);
		pRowInfo->m_pOrgData = PROVIDER_ALLOC(SIZEOF_ONEROW);
		TESTC_(GetData(pRowInfo->m_hRow, pRowInfo->m_pOrgData),S_OK);

		m_mapRowInfo.SetAt(pCommand->m_rgRowIds[i], pRowInfo);
	}

CLEANUP:
	SAFE_RELEASE(pIRowsetLocate);
	return hr;
}


HRESULT CRowsetObj::GetRowsAtRatio(COMMAND* pCommand)
{
	ASSERT(pCommand);
	HRESULT hr = S_OK;
	IRowsetScroll* pIRowsetScroll = NULL;

	DBCOUNTITEM cRows = pCommand->m_cRows;
	HROW* rghRows = pCommand->m_rghRows;
	ULONG ulRatio = pCommand->m_dwOption;
	DBCOUNTITEM cRowsObtained = 0;
	DBCOUNTITEM i;
	
	//GetRowsAtRatio
	TESTC_(hr = pIRowset()->QueryInterface(IID_IRowsetScroll,(void**)&pIRowsetScroll),S_OK);

	//Can return an error if canceled from within a listener
	switch(pCommand->m_hrExpected)
	{
		case DB_S_ENDOFROWSET:
			hr = pIRowsetScroll->GetRowsAtRatio(NULL, NULL, 0, ulRatio, MAX_ROW_COUNT, &cRowsObtained, &rghRows);
			break;
	
		default:
			hr = pIRowsetScroll->GetRowsAtRatio(NULL, NULL, 0, ulRatio, cRows, &cRowsObtained, &rghRows);
			break;
	}

	//Verify results
	if(hr==S_OK)
		TESTC(cRows == cRowsObtained);
	
	//Loop over rows retrived
	for(i=0; i<cRowsObtained; i++)
	{
		ROWINFO* pRowInfo = GetRowInfo(pCommand->m_rgRowIds[i]);
		SAFE_DELETE(pRowInfo);
		pRowInfo = new ROWINFO(pCommand->m_rghRows[i]);
		
		//Get the bookmark for this row
		if(m_dwRowsetMode & ROWSETLOCATE_ROWSET)
			TESTC_(GetBookmark(pRowInfo->m_hRow, pRowInfo),S_OK);

		//Get the Data for this row
		PROVIDER_FREE(pRowInfo->m_pNewData);
		pRowInfo->m_pNewData = PROVIDER_ALLOC(SIZEOF_ONEROW);
		TESTC_(GetData(pRowInfo->m_hRow, pRowInfo->m_pNewData),S_OK);

		//Save the Data for this row
		PROVIDER_FREE(pRowInfo->m_pOrgData);
		pRowInfo->m_pOrgData = PROVIDER_ALLOC(SIZEOF_ONEROW);
		TESTC_(GetData(pRowInfo->m_hRow, pRowInfo->m_pOrgData),S_OK);

		m_mapRowInfo.SetAt(pCommand->m_rgRowIds[i],pRowInfo);
	}

CLEANUP:
	SAFE_RELEASE(pIRowsetScroll);
	return hr;
}


HRESULT CRowsetObj::GetRowsByBookmark(COMMAND* pCommand)
{
	ASSERT(pCommand);
	HRESULT hr = S_OK;
	IRowsetLocate* pIRowsetLocate = NULL;
	DBCOUNTITEM i = 0;
	
	//Need to lookup the bookmark value for these rows
	ROWINFO* pRowInfo = NULL;
	DBBKMARK rgcbBookmarks[MAX_ROW_COUNT];
	const BYTE* rgpBookmarks[MAX_ROW_COUNT];

	for(i=0; i<pCommand->m_cRows; i++)
	{
		if(pRowInfo = GetRowInfo(pCommand->m_rgRowIds[i]))
		{
			rgcbBookmarks[i] = pRowInfo->m_cbBookmark;  //Bookmark size
			rgpBookmarks[i] = pRowInfo->m_pBookmark;
		}
	}

	//IRowsetLocate
	TESTC_(hr = pIRowset()->QueryInterface(IID_IRowsetLocate,(void**)&pIRowsetLocate),S_OK);
	
	switch(pCommand->m_hrExpected)
	{
		case E_INVALIDARG:
			hr = pIRowsetLocate->GetRowsByBookmark(NULL, pCommand->m_cRows, NULL, NULL, pCommand->m_rghRows, NULL);
			break;

		default:
			hr = pIRowsetLocate->GetRowsByBookmark(NULL, pCommand->m_cRows, rgcbBookmarks, rgpBookmarks, pCommand->m_rghRows, NULL);
			break;
	};


	//Loop through rows obtained 
	for(i=0; i<pCommand->m_cRows; i++)
	{
		if(SUCCEEDED(hr) && pCommand->m_rghRows[i]!=DB_NULL_HROW)
		{
			ROWINFO* pRowInfo = NULL;
			if(m_mapRowInfo.Lookup(pCommand->m_rgRowIds[i],pRowInfo))
				SAFE_DELETE(pRowInfo);
			pRowInfo = new ROWINFO(pCommand->m_rghRows[i]);

			//Get the bookmark for this row, if bookmarks are on...
			if(m_dwRowsetMode & ROWSETLOCATE_ROWSET)
				TESTC_(GetBookmark(pRowInfo->m_hRow, pRowInfo),S_OK);

			//Get the Data for this row
			PROVIDER_FREE(pRowInfo->m_pNewData);
			pRowInfo->m_pNewData = PROVIDER_ALLOC(SIZEOF_ONEROW);
			TESTC_(GetData(pRowInfo->m_hRow, pRowInfo->m_pNewData),S_OK);

			//Save the OrgData for this row
			PROVIDER_FREE(pRowInfo->m_pOrgData);
			pRowInfo->m_pOrgData = PROVIDER_ALLOC(SIZEOF_ONEROW);
			TESTC_(GetData(pRowInfo->m_hRow, pRowInfo->m_pOrgData),S_OK);
			
			m_mapRowInfo.SetAt(pCommand->m_rgRowIds[i], pRowInfo);
		}
	}

CLEANUP:
	SAFE_RELEASE(pIRowsetLocate);
	return hr;
}


HRESULT CRowsetObj::DeleteRows(COMMAND* pCommand)
{
	ASSERT(pCommand);
	HRESULT hr = S_OK;
	IRowsetChange* pIRowsetChange = NULL;

	//DeleteRows
	TESTC_(hr = QI(pIRowset(), IID_IRowsetChange, (void**)&pIRowsetChange),S_OK);
	hr = pIRowsetChange->DeleteRows(NULL, pCommand->m_cRows, pCommand->m_rghRows, NULL);

CLEANUP:
	SAFE_RELEASE(pIRowsetChange);
	return hr;
}


HRESULT CRowsetObj::ResynchRows(COMMAND* pCommand)
{
	ASSERT(pCommand);
	
	HRESULT hr = S_OK;
	void* pVisibleData = NULL;
	DBROWSTATUS* rgRowStatus = NULL;

	//ResynchRows
	hr = CRowset::ResynchRows(pCommand->m_cRows, pCommand->m_rghRows, NULL, NULL, &rgRowStatus);

	//Loop over rows Resynched
	if(SUCCEEDED(hr))
	{
		for(DBCOUNTITEM i=0; i<pCommand->m_cRows; i++)
		{
			//Only update our flags if row was successfully resynched
			if(rgRowStatus[i] != DBROWSTATUS_S_OK)
				continue;

			if(pCommand->m_rghRows[i] != DB_NULL_HROW)
			{
				ROWINFO* pRowInfo = GetRowInfo(pCommand->m_rgRowIds[i]);
				SAFE_DELETE(pRowInfo);
				pRowInfo = new ROWINFO(pCommand->m_rghRows[i]);
				m_mapRowInfo.SetAt(pCommand->m_rgRowIds[i],pRowInfo);
				
				//Get the Data for this row
				PROVIDER_FREE(pRowInfo->m_pNewData);
				pRowInfo->m_pNewData = PROVIDER_ALLOC(SIZEOF_ONEROW);
				TESTC_(GetData(pRowInfo->m_hRow, pRowInfo->m_pNewData),S_OK);

				//GetVisible Data for this row
				PROVIDER_FREE(pVisibleData);
				pVisibleData = PROVIDER_ALLOC(SIZEOF_ONEROW);
				TESTC_(GetVisibleData(pRowInfo->m_hRow, pVisibleData),S_OK);

				//Verify both Visible and Current Data are equal after Resynch
				TESTC(CompareRowData(pRowInfo->m_pNewData, pVisibleData, m_hAccessor));
		 	}
		}
	}

CLEANUP:
	PROVIDER_FREE(pVisibleData);
	PROVIDER_FREE(rgRowStatus);
	return hr;
}


BOOL CRowsetObj::MapRowIds(COMMAND* pCommand)
{
	TBEGIN
	ASSERT(pCommand);

	DBCOUNTITEM cRows = pCommand->m_cRows;
	DBCOUNTITEM* rgRowIds = pCommand->m_rgRowIds;
	HROW* rghRows = pCommand->m_rghRows;
	ROWINFO* pRowInfo = NULL;

	switch (pCommand->m_eCommandType)
	{
		case INSERTROW:
		{	pRowInfo = new ROWINFO(DB_NULL_HROW);
			pRowInfo->m_eRowStatus = ROWSTATUS_INSERTED;
			m_mapRowInfo.SetAt(rgRowIds[0], pRowInfo);
		}
		break;

		case DELETEROWS:
		{	// Set HROW's to delete
			for(DBCOUNTITEM i=0; i<cRows; i++)
			{
				TESTC((pRowInfo = GetRowInfo(rgRowIds[i])) != NULL);
				rghRows[i] = pRowInfo->m_hRow;
				
				//Insert -> Deleted row is not change,
				//Otherwsie this is considerd a deleted row
				if(pRowInfo->m_eRowStatus == ROWSTATUS_INSERTED)
					pRowInfo->m_eRowStatus = ROWSTATUS_INVALID;
				else
					pRowInfo->m_eRowStatus = ROWSTATUS_DELETED;
			}
		}
		break;

		case SETDATA:
		{
			if(pRowInfo = GetRowInfo(rgRowIds[0]))
			{ 
				rghRows[0] = pRowInfo->m_hRow;
			
				//This row is now treated as modiifed, unless it is an inserted row.
				//If inserted it still is treated as inserted
				if(pRowInfo->m_eRowStatus != ROWSTATUS_INSERTED)
					pRowInfo->m_eRowStatus = ROWSTATUS_CHANGED;
			}
		}
		break;

		case UNDO:
		case UPDATE:
		case RESYNCHROWS:
		{
			//Set HROW's to Get
			for(DBCOUNTITEM i=0; i<cRows; i++)
			{
				rghRows[i] = DB_NULL_HROW;
				if(pRowInfo = GetRowInfo(rgRowIds[i]))
					rghRows[i] = pRowInfo->m_hRow;
			}

			//If cRows==0, (0,NULL) will "do" all rows with
			//pending changes.  So loop through all rows
			if(cRows==0)
			{ 
				DBCOUNTITEM key = 0;
				DBCOUNTITEM cRowsPending = 0;
				POSITION pos = m_mapRowInfo.GetStartPosition();
				while(pos)
				{
					m_mapRowInfo.GetNextAssoc(pos, key, pRowInfo);
					if(pRowInfo && pRowInfo->m_eRowStatus != ROWSTATUS_NOCHANGE)
					{
						rgRowIds[cRowsPending] = key;
						rghRows[cRowsPending] = pRowInfo->m_hRow;
						cRowsPending++;
					}
				}
				pCommand->m_dwOption = (DWORD) cRowsPending;
			}
		}
		break;

		case REFRESHROWS:
		{
			//Set HROW's to Get
			for(DBCOUNTITEM i=0; i<cRows; i++)
			{
				rghRows[i] = DB_NULL_HROW;
				if(pRowInfo = GetRowInfo(rgRowIds[i]))
					rghRows[i] = pRowInfo->m_hRow;
			}

			//If cRows==0, (0,NULL) will "do" all active 
			//rows.  So loop through all rows
			if(cRows==0)
			{ 
				DBCOUNTITEM key = 0;
				DBCOUNTITEM cRowsActive = 0;
				POSITION pos = m_mapRowInfo.GetStartPosition();
				while(pos)
				{
					m_mapRowInfo.GetNextAssoc(pos, key, pRowInfo);
					if(pRowInfo && pRowInfo->m_eRowStatus != ROWSTATUS_INSERTED && 
						pRowInfo->m_eRowStatus != ROWSTATUS_DELETED)
					{
						rgRowIds[cRowsActive] = key;
						rghRows[cRowsActive] = pRowInfo->m_hRow;
						cRowsActive++;
					}
				}
				pCommand->m_dwOption = (ULONG) cRowsActive;
			}
		}
		break;

		default:
		{	
			//Set HROW's to Get
			for(DBCOUNTITEM i=0; i<cRows; i++)
			{
				rghRows[i] = DB_NULL_HROW;
				if(pRowInfo = GetRowInfo(rgRowIds[i]))
					rghRows[i] = pRowInfo->m_hRow;
			}
		}
		break;
	}

CLEANUP:
	TRETURN;
}


HRESULT CRowsetObj::GetData(HROW hRow, void* pData)
{
	ASSERT(hRow != DB_NULL_HROW);
	ASSERT(pData);
	HRESULT hr = S_OK;

	//GetData
	TESTC_(pIRowset()->GetData(hRow, m_hAccessor, pData),S_OK);

CLEANUP:
  	return hr;
}

HRESULT CRowsetObj::GetOriginalData(HROW hRow, void* pData)
{
	ASSERT(hRow != DB_NULL_HROW);
	ASSERT(pData);
	HRESULT hr = S_OK;
	IRowsetUpdate* pIRowsetUpdate = NULL;
	
	//Obtain IRowsetUpdate
	TESTC_(QI(pIRowset(), IID_IRowsetUpdate, (void**)&pIRowsetUpdate),S_OK);

	//GetOriginalData
	hr = pIRowsetUpdate->GetOriginalData(hRow, m_hAccessor, pData);

CLEANUP:
	SAFE_RELEASE(pIRowsetUpdate);
	return hr;
}


HRESULT CRowsetObj::GetVisibleData(HROW hRow, void* pData)
{
	ASSERT(hRow != DB_NULL_HROW);
	ASSERT(pData);
	HRESULT hr = S_OK;
	IRowsetResynch* pIRowsetResynch = NULL;
	
	//Obtain IRowsetResynch
	TESTC_(QI(pIRowset(), IID_IRowsetResynch, (void**)&pIRowsetResynch),S_OK);

	//GetVisibleData
	hr = pIRowsetResynch->GetVisibleData(hRow, m_hAccessor, pData);

CLEANUP:
	SAFE_RELEASE(pIRowsetResynch);
	return hr;
}


HRESULT CRowsetObj::GetLastVisibleData(HROW hRow, void* pData)
{
	ASSERT(hRow != DB_NULL_HROW);
	ASSERT(pData);
	HRESULT hr = S_OK;
	IRowsetRefresh* pIRowsetRefresh = NULL;
	
	//Obtain IRowsetRefresh
	TESTC_(QI(pIRowset(), IID_IRowsetRefresh, (void**)&pIRowsetRefresh),S_OK);

	//RefreshVisibleData
	hr = pIRowsetRefresh->GetLastVisibleData(hRow, m_hAccessor, pData);

CLEANUP:
	SAFE_RELEASE(pIRowsetRefresh);
	return hr;
}


HRESULT CRowsetObj::GetData(COMMAND* pCommand)
{
	ASSERT(pCommand);
	HRESULT hr = S_OK;
	HRESULT hrExpected = S_OK;

	ROWINFO* pRowInfo = NULL;
	void* pData = NULL;

	//Loop though all specificed rows and make sure GetData returns the correc
	//data for this row
	for(DBCOUNTITEM i=0; i<pCommand->m_cRows; i++)
	{
		if (pCommand->m_rghRows[i] != DB_NULL_HROW)
		{
			//Get the "Current" Data for this row
			pData = PROVIDER_ALLOC(SIZEOF_ONEROW);
			TESTC_(pIRowset()->GetData(pCommand->m_rghRows[i], m_hAccessor, pData),S_OK);

			//Lookup the Saved data for this row
			TESTC((pRowInfo = GetRowInfo(pCommand->m_rgRowIds[i])) != NULL);
			
			//Compare data
			TESTC(CompareRowData(pData, pRowInfo->m_pNewData, m_hAccessor));
			PROVIDER_FREE(pData);
		}
	}

CLEANUP:
  	PROVIDER_FREE(pData);
	return hr;
}


HRESULT CRowsetObj::GetOriginalData(COMMAND* pCommand)
{
	ASSERT(pCommand);
	HRESULT hr = S_OK;
	HRESULT hrExpected = pCommand->m_hrExpected;

	ROWINFO* pRowInfo = NULL;
	void* pData = NULL;
	IRowsetUpdate* pIRowsetUpdate = NULL;
	DBCOUNTITEM i=0;

	//Obtain IRowsetUpdate interface
	TESTC_(hr = QI(pIRowset(), IID_IRowsetUpdate, (void**)&pIRowsetUpdate),S_OK);

	//Loop though all rows and make sure GetData returns the correct data
	for(i=0; i<pCommand->m_cRows; i++)
	{
		//Get the "Current" Data for this row
		pData = PROVIDER_ALLOC(SIZEOF_ONEROW);

		//As can call during notification may receive DB_E_NOTREENTRANT (pCommand->m_hrExpected)
		TEST2C_(hr = pIRowsetUpdate->GetOriginalData(pCommand->m_rghRows[i], m_hAccessor, pData), S_OK, hrExpected);
		if (hr==S_OK)
		{
			// if received S_OK for one row should receive it for all
			if (i==0) hrExpected = S_OK;

			//Lookup the Saved data for this row
			TESTC((pRowInfo = GetRowInfo(pCommand->m_rgRowIds[i])) != NULL);
			
			//Compare data
			TESTC(CompareRowData(pRowInfo->m_pOrgData, pData, m_hAccessor));
		}
		PROVIDER_FREE(pData);
	}

CLEANUP:
	SAFE_RELEASE(pIRowsetUpdate);
	PROVIDER_FREE(pData);
	return hr;
}


HRESULT CRowsetObj::GetVisibleData(COMMAND* pCommand)
{
	ASSERT(pCommand);
	HRESULT hr = S_OK;
	HRESULT hrExpected = pCommand->m_hrExpected;

	ROWINFO* pRowInfo = NULL;
	void* pData = NULL;
	IRowsetResynch* pIRowsetResynch = NULL;
	DBCOUNTITEM i=0;

	//Obtain IRowsetUpdate interface
	TESTC_(hr = QI(pIRowset(), IID_IRowsetResynch, (void**)&pIRowsetResynch),S_OK);

	//Loop though all rows and make sure GetData returns the correct data
	for(i=0; i<pCommand->m_cRows; i++)
	{
		//Get the "Current" Data for this row
		pData = PROVIDER_ALLOC(SIZEOF_ONEROW);

		//As can call during notification may receive DB_E_NOTREENTRANT (pCommand->m_hrExpected)
		TEST2C_(hr = pIRowsetResynch->GetVisibleData(pCommand->m_rghRows[i], m_hAccessor, pData), S_OK, hrExpected);

		if (hr==S_OK)
		{
			// if received S_OK for one row should receive it for all
			if (i==0) hrExpected = S_OK;

			//Lookup the Saved data for this row
			TESTC((pRowInfo = GetRowInfo(pCommand->m_rgRowIds[i])) != NULL);

			//Compare data
			TESTC(CompareRowData(pData, pRowInfo->m_pOrgData, m_hAccessor));
		}
		PROVIDER_FREE(pData);
	}

CLEANUP:
	SAFE_RELEASE(pIRowsetResynch);
	PROVIDER_FREE(pData);
	return hr;
}


HRESULT CRowsetObj::GetLastVisibleData(COMMAND* pCommand)
{
	ASSERT(pCommand);
	HRESULT hr = S_OK;

	ROWINFO* pRowInfo = NULL;
	void* pData = NULL;
	IRowsetRefresh* pIRowsetRefresh = NULL;
	DBCOUNTITEM i=0;

	//Obtain IRowsetUpdate interface
	TESTC_(hr = QI(pIRowset(), IID_IRowsetRefresh, (void**)&pIRowsetRefresh),S_OK);

	//Loop though all rows and make sure GetData returns the correct data
	for(i=0; i<pCommand->m_cRows; i++)
	{
		//Get the "Current" Data for this row
		pData = PROVIDER_ALLOC(SIZEOF_ONEROW);
		TESTC_(pIRowsetRefresh->GetLastVisibleData(pCommand->m_rghRows[i], m_hAccessor, pData),S_OK);

		//Lookup the Saved data for this row
		TESTC((pRowInfo = GetRowInfo(pCommand->m_rgRowIds[i])) != NULL);
		
		//Compare data
		TESTC(CompareRowData(pData, pRowInfo->m_pOrgData, m_hAccessor));
		PROVIDER_FREE(pData);
	}

CLEANUP:
	SAFE_RELEASE(pIRowsetRefresh);
	PROVIDER_FREE(pData);
	return hr;
}


HRESULT CRowsetObj::AddColumn()
{
	ASSERT(g_pTable);
	HRESULT hr = S_OK;

	//Add a column to the table
	WCHAR strColName[80];
	CCol cCol;

	//Get first column information
	g_pTable->GetColInfo(1, cCol);

	//Provider Type name may be NULL.  If running with an INI File, or 
	//just the fact that our Privlib addColumn loses all the provider typenames when it regenerates
	//the columninfo after an add...
	if(cCol.GetProviderTypeName())
		swprintf(strColName, L"C%02lu%.4s", GetTotalColumns() + 1, cCol.GetProviderTypeName());
	else
		swprintf(strColName, L"C%02lu", GetTotalColumns() + 1);
	cCol.SetColName(strColName);

	// add the column
	hr = g_pTable->AddColumn(cCol);

	//Indicate the underlying Table has been altered
	if(SUCCEEDED(hr))
		m_fTableAltered = TRUE;

	return hr;
}


HRESULT CRowsetObj::DropColumn()
{
	ASSERT(g_pTable);
	HRESULT		hr = S_OK;
	const DBORDINAL	nColNum	= 1;

	//Drop the first column from the table
	hr = g_pTable->DropColumn(nColNum);

	//Indicate the underlying Table has been altered
	if(SUCCEEDED(hr))
		m_fTableAltered = TRUE;

	return hr;
}


HRESULT CRowsetObj::GetColumnInfo()
{
	HRESULT hr = S_OK;
	DBORDINAL cColInfo = 0;
	DBCOLUMNINFO* rgColInfo = NULL;
	WCHAR* pStringBuffer = NULL;

	//IColumnsInfo::GetColumnInfo
	hr = GetColInfo(&cColInfo, &rgColInfo, &pStringBuffer);

	SAFE_FREE(rgColInfo);
	SAFE_FREE(pStringBuffer);
	return hr;
}


HRESULT CRowsetObj::GetBookmark
(
	HROW		hRow,		
	ROWINFO*	pRowInfo	
)
{
	ASSERT(pRowInfo);
	HRESULT hr = S_OK;

	//GetBookmark
	TESTC_(hr = CRowset::GetBookmark(hRow, &pRowInfo->m_cbBookmark, &pRowInfo->m_pBookmark),S_OK);

CLEANUP:
	return hr;
}


ROWINFO* CRowsetObj::GetRowInfo(DBCOUNTITEM ulRowId)
{
	ROWINFO* pRowInfo = NULL;
	
	m_mapRowInfo.Lookup(ulRowId, pRowInfo);
	return pRowInfo;
}

BOOL CRowsetObj::DestroyRowData()
{
	POSITION pos = m_mapRowInfo.GetStartPosition();
	while(pos)
	{
		DBCOUNTITEM Key = 0; 
		ROWINFO* pRowInfo = NULL;
		
		m_mapRowInfo.GetNextAssoc(pos, Key, pRowInfo);
		if(pIRowset()) 
		{
			HRESULT hr = pIRowset()->ReleaseRows(1, &pRowInfo->m_hRow, NULL, NULL, NULL);
		}

		SAFE_DELETE(pRowInfo);
	}

	m_mapRowInfo.RemoveAll();
	return TRUE;
}

BOOL CRowsetObj::GetRowsToRelease(DBCOUNTITEM cRows, DBCOUNTITEM   &cRowsObtained, HROW *rghRows)
{
	ASSERT(rghRows);

	cRowsObtained = 0;
	POSITION pos = m_mapRowInfo.GetStartPosition();
	int i=0;
	
	while(pos && cRowsObtained<cRows)
	{
		DBCOUNTITEM Key = 0; 
		ROWINFO* pRowInfo = NULL;
		i++;
		m_mapRowInfo.GetNextAssoc(pos, Key, pRowInfo);
		if(pRowInfo->m_hRow != DB_NULL_HROW && pRowInfo->m_cRefCount) 
			rghRows[cRowsObtained++] = pRowInfo->m_hRow;

		pRowInfo = NULL;
	}

	return TRUE;
}


////////////////////////////////////////////////////////////////////////
// CImpIListener
//
////////////////////////////////////////////////////////////////////////
CImpIListener::CImpIListener(CExecutionManager* pExecutionManager)
{
	ASSERT(pExecutionManager);
	m_pExecutionManager = pExecutionManager;

	m_cRef = 1;
	m_dwCookie = 0;

	//Set all WantedEvents to TRUE
	memset(&m_rgWantedEvent, TRUE, sizeof(m_rgWantedEvent));

	//Set times notified to 0
	ResetTimesNotified();
}

CImpIListener::~CImpIListener()
{
	ASSERT(m_cRef==0);
}

ULONG CImpIListener::AddRef(void)
{
	return ++m_cRef;
}


ULONG CImpIListener::Release(void)
{
	ASSERT(m_cRef > 0);

	if(--m_cRef) 
		return m_cRef;
	
	delete this;
	return 0;
}


HRESULT CImpIListener::QueryInterface(REFIID riid, void** ppv)
{
	if(!ppv)
		return E_INVALIDARG;
	
	if(riid == IID_IUnknown)
		*ppv = this;
	else if(riid == IID_IRowsetNotify)
		*ppv = this;
	else			
	{
		*ppv = NULL;
		return E_NOINTERFACE;
	}
	
	((IUnknown*)*ppv)->AddRef();
	return S_OK;
}

		
ULONG CImpIListener::GetTimesNotified(DBREASON eReason, DBEVENTPHASE ePhase)
{
	ASSERT(eReason <= DBREASON_ALL);
	ASSERT(ePhase  <= DBEVENTPHASE_ALL);

	//Can return the number of times notified for either all reasons / phases
	//or just a particular reason / phase

	ULONG cTimesNotified = 0;

	for(ULONG iReason=0; iReason<DBREASON_ALL; iReason++)
		if(iReason == eReason || eReason == DBREASON_ALL)
			for(ULONG iPhase=0; iPhase<DBEVENTPHASE_ALL; iPhase++)
				if(iPhase == ePhase || ePhase == DBEVENTPHASE_ALL)
					cTimesNotified += m_rgTimesNotified[iReason][iPhase];
	
	return cTimesNotified;
}
	
BOOL CImpIListener::ResetTimesNotified(DBREASON eReason, DBEVENTPHASE ePhase)
{
	ASSERT(eReason <= DBREASON_ALL);
	ASSERT(ePhase  <= DBEVENTPHASE_ALL);

	//Can Reaset all times notified for either all reasons / phases
	//or just a particular reason / phase

	for(ULONG iReason=0; iReason<DBREASON_ALL; iReason++)
		if(iReason == eReason || eReason == DBREASON_ALL)
			for(ULONG iPhase=0; iPhase<DBEVENTPHASE_ALL; iPhase++)
				if(iPhase == ePhase || ePhase == DBEVENTPHASE_ALL)
					m_rgTimesNotified[iReason][iPhase] = 0;
	
	return TRUE;
}


HRESULT CImpIListener::AcceptOrVeto
( 
    IRowset* pIRowset,		
    DBCOUNTITEM cRows,
	const HROW rghRows[],						
    DBORDINAL cColumns,					
	DBORDINAL rgColumns[],				
    DBREASON eReason,				
    DBEVENTPHASE ePhase,			
    BOOL fCantDeny					
)
{
	HRESULT hr = S_OK;
	EXPECT* pExpect = NULL;
	COMMAND* pCommand = NULL;
	
	//Make sure this is a valid Reason/Phase
	TESTC(IsValidEvent(eReason, ePhase));
	
	//Record this Notification
	m_rgTimesNotified[eReason][ePhase]++;
	
	//Verify this Notification matches the Expectation
	while(pExpect = pExpectationsManager()->FindExpectation(this, pIRowset, cRows, (HROW*)rghRows, cColumns, rgColumns, eReason, ePhase, fCantDeny))
	{
		//Find the Command which corresponds to this Reason/Phase
		pCommand = pExecutionManager()->PopCommand(this, eReason, ePhase);
		TESTC(pCommand != NULL);
	
		switch(pCommand->m_eCommandType)
		{
			case RETURN_ACCEPT:
				pExpectationsManager()->PushNextExpectation(pExpect);
				SAFE_DELETE(pCommand);
				return S_OK;

			case RETURN_VETO:
				if(pExecutionManager()->IsCancelableEvent(eReason, ePhase))
				{	
					TESTC(pExpectationsManager()->PushFalseExpectation(pExpect));
				}
				else
				{
					TESTC(pExpectationsManager()->PushNextExpectation(pExpect));
				}
				SAFE_DELETE(pCommand);
				return S_FALSE;

			case RETURN_UNWANTEDREASON:
				//We no longer want this Reason
				//Mark as completed since we might not get any more Notifications
				SetUnwantedReason(eReason);
				pExpectationsManager()->PushUnwantedReason(this, pIRowset, eReason);
				SAFE_DELETE(pCommand);
				return DB_S_UNWANTEDREASON;

			case RETURN_UNWANTEDPHASE:
				//Since this Phase is no longer wanted, 
				//set the expectation to the next wanted phase...
				SetUnwantedPhase(eReason, ePhase);
				pExpectationsManager()->PushUnwantedPhase(this, pIRowset, eReason, ePhase);
				SAFE_DELETE(pCommand);
				return DB_S_UNWANTEDPHASE;

			case RETURN_EFAIL:
			case RETURN_EOUTOFMEMORY:
			case RETURN_EINVALIDARG:
				//Errors returned should be treated like S_OK (no VETO)
				pExpectationsManager()->PushNextExpectation(pExpect);
				switch(pCommand->m_eCommandType)
				{
					case RETURN_EFAIL:
						SAFE_DELETE(pCommand);
						return E_FAIL;
				
					case RETURN_EOUTOFMEMORY:
						SAFE_DELETE(pCommand);
						return E_OUTOFMEMORY;

					case RETURN_EINVALIDARG:
						SAFE_DELETE(pCommand);
						return E_INVALIDARG;
				}
				break;

			case RETURN_ADVISE:
			{
				//Create a new Listener and add to list
				pExpectationsManager()->PushAdviseExpectation(pExpect);
				return S_OK;
			}

			case RETURN_UNADVISE:
			{	//Unadvise this listener from the Rowset and remove from list
				pExpectationsManager()->PushUnadviseExpectation(pExpect);
				return S_OK;
			}

			default:
			{
				//Execute the nested Command
				QTESTC(pExpectationsManager()->PushNestedExpectation(pCommand));
				QTESTC(pExecutionManager()->ExecuteCommand(pCommand));
				QTESTC(pExpectationsManager()->PopNestedExpectation(pCommand))
				SAFE_DELETE(pCommand);
			}
		}
	}


CLEANUP:
	SAFE_DELETE(pCommand);
	return hr;
}

HRESULT CImpIListener::OnFieldChange
( 
    IRowset* pIRowset,		
    HROW hRow,						
    DBORDINAL cColumns,					
	DBORDINAL rgColumns[],				
    DBREASON eReason,				
    DBEVENTPHASE ePhase,			
    BOOL fCantDeny					
)
{
	//Accept / Veto the change, and do processing
	HRESULT hr = AcceptOrVeto(pIRowset, ONE_ROW, &hRow, cColumns, rgColumns, eReason, ePhase, fCantDeny);

	//Display Notification is user is interested...
	PRVTRACE(L"    0x%08x::OnFieldChange(0x%08x, 0x%08x, %d, 0x%08x, %s, %s, %s) - %s\n", this, pIRowset, hRow, cColumns, rgColumns, GetReasonDesc(eReason), GetPhaseDesc(ePhase), fCantDeny==TRUE ? L"TRUE" : L"FALSE", GetErrorName(hr));
	return hr;
}


HRESULT CImpIListener::OnRowChange
( 
    IRowset* pIRowset,			
    DBCOUNTITEM cRows,						
    const HROW rghRows[],	
    DBREASON eReason,					
    DBEVENTPHASE ePhase,				
    BOOL fCantDeny
)						
{
	//Accept / Veto the change, and do processing
	HRESULT hr = AcceptOrVeto(pIRowset, cRows, rghRows, 0, NULL, eReason, ePhase, fCantDeny);

	//Display Notification is user is interested...
	PRVTRACE(L"    0x%08x::OnRowChange(0x%08x, %d, 0x%08x, %s, %s, %s) - %s\n", this, pIRowset, cRows, rghRows, GetReasonDesc(eReason), GetPhaseDesc(ePhase), fCantDeny==TRUE ? L"TRUE" : L"FALSE", GetErrorName(hr));
	return hr;
}


HRESULT CImpIListener::OnRowsetChange
( 
    IRowset* pIRowset,		
    DBREASON eReason,				
    DBEVENTPHASE ePhase,			
    BOOL fCantDeny					
)
{
	//Accept / Veto the change, and do processing
	HRESULT hr = AcceptOrVeto(pIRowset, 0, NULL, 0, NULL, eReason, ePhase, fCantDeny);

	//Display Notification is user is interested...
	PRVTRACE(L"    0x%08x::OnRowsetChange(0x%08x, %s, %s, %s) - %s\n", this, pIRowset, GetReasonDesc(eReason), GetPhaseDesc(ePhase), fCantDeny==TRUE ? L"TRUE" : L"FALSE", GetErrorName(hr));
	return hr;
}


HRESULT CImpIListener::Advise(IRowset* pIRowset)
{
	ASSERT(pIRowset);
	ASSERT(m_dwCookie == 0); //Implementation only suited for 1 advise per listener

	//We can't just use m_pIRowset, since we could Advise / Connect this listener
	//to other rowsets besides just the one we wer created with
	//But if no rowset* is passed in, we'll use the current one

	IConnectionPoint* pICP = NULL;
	IConnectionPointContainer* pICPC = NULL;
	HRESULT hr = S_OK;

	//Obtain the connection point container
	TESTC_(hr = pIRowset->QueryInterface(IID_IConnectionPointContainer,(void**)&pICPC),S_OK)
	TESTC(pICPC != NULL);

	//Obtain the IRowsetNotify connection point 
	TESTC_(hr = pICPC->FindConnectionPoint(IID_IRowsetNotify,&pICP),S_OK)
	TESTC(pICP != NULL);

	//Now we can advise the connection
	TESTC_(hr = pICP->Advise(this, &m_dwCookie),S_OK)

CLEANUP:
	SAFE_RELEASE(pICP);
	SAFE_RELEASE(pICPC);
	return hr;
}

HRESULT CImpIListener::Unadvise(IRowset* pIRowset)
{
	ASSERT(pIRowset);
	ASSERT(m_dwCookie != 0);

	//We can't just use m_pIRowset, since we could Advise / Connect this listener
	//to other rowsets besides just the one we were created with
	//But if no rowset* is passed in, we'll use the current one
	IConnectionPoint* pICP = NULL;
	IConnectionPointContainer* pICPC = NULL;
	HRESULT hr = S_OK;

	//Obtain the connection point container
	TESTC_(hr = pIRowset->QueryInterface(IID_IConnectionPointContainer,(void**)&pICPC),S_OK)
	TESTC(pICPC != NULL);

	//Obtain the IRowsetNotify connection point 
	TESTC_(hr = pICPC->FindConnectionPoint(IID_IRowsetNotify,&pICP),S_OK)
	TESTC(pICP != NULL);

	//Now we can advise the connection
	TESTC_(hr = pICP->Unadvise(m_dwCookie),S_OK)

	//Set Cookie back to 0
	m_dwCookie = 0;

CLEANUP:
	SAFE_RELEASE(pICP);
	SAFE_RELEASE(pICPC);
	return hr;
}

BOOL CImpIListener::IsWantedReason(DBREASON eReason)
{
	ASSERT(IsValidEvent(eReason));

	//Find the Reason index 
	for(ULONG iReason=0; iReason<DBREASON_ALL; iReason++)
	{
		if(iReason == eReason)
		{
			//If any of the phases for this reason are on (TRUE)
			//then this is a "wanted" reason
			for(ULONG iPhase=0; iPhase<DBEVENTPHASE_ALL; iPhase++)
				if(m_rgWantedEvent[iReason][iPhase])
					return TRUE;
		}
	}

	return FALSE;
}

BOOL CImpIListener::IsWantedPhase(DBREASON eReason, DBEVENTPHASE ePhase)
{
	ASSERT(IsValidEvent(eReason, ePhase));

	//Find the Reason index 
	for(ULONG iReason=0; iReason<DBREASON_ALL; iReason++)
	{
		if(iReason == eReason)
		{
			if(m_rgWantedEvent[iReason][ePhase])
				return TRUE;
		}
	}

	return FALSE;
}

BOOL CImpIListener::SetUnwantedReason(DBREASON eReason)
{
	ASSERT(IsValidEvent(eReason));

	//Turn off all phases for this reason
	//Which will indicate this reason is not needed for any phase
	for(ULONG iReason=0; iReason<DBREASON_ALL; iReason++)
	{
		if(iReason == eReason)
		{
			for(ULONG iPhase=0; iPhase<DBEVENTPHASE_ALL; iPhase++)
				m_rgWantedEvent[iReason][iPhase] = FALSE;
		}
	}
	
	return TRUE;
}

BOOL CImpIListener::SetUnwantedPhase(DBREASON eReason, DBEVENTPHASE ePhase)
{
	ASSERT(IsValidEvent(eReason, ePhase));

	//Turn off the indicated phases for this reason
	for(ULONG iReason=0; iReason<DBREASON_ALL; iReason++)
	{
		if(iReason == eReason)
			m_rgWantedEvent[iReason][ePhase] = FALSE;
	}
	
	return TRUE;
}


DBEVENTPHASE CImpIListener::NextPhase(DBREASON eReason, DBEVENTPHASE ePhase)
{
	ASSERT(IsValidEvent(eReason, ePhase));

	switch(eReason)
	{
		//All Phases can occur
		case DBREASON_ROWSET_FETCHPOSITIONCHANGE:
		case DBREASON_ROWSET_CHANGED:
		case DBREASON_COLUMN_SET:
		case DBREASON_ROW_DELETE:
		case DBREASON_ROW_FIRSTCHANGE:
		case DBREASON_ROW_INSERT:
		case DBREASON_ROW_RESYNCH:
		case DBREASON_ROW_UNDOCHANGE:
		case DBREASON_ROW_UNDOINSERT:
		case DBREASON_ROW_UNDODELETE:
		case DBREASON_ROW_UPDATE:
			switch(ePhase)
			{		
				//This setup will return whatever the next phase expected
				//Depending upon which phases the Provider supports..
				//The cases will just "fall" through to the next step if a
				//phase is not supported...

				case DBEVENTPHASE_OKTODO:
					if(pExecutionManager()->pCRowsetObj()->IsSupportedPhase(DBEVENTPHASE_ABOUTTODO))
						return DBEVENTPHASE_ABOUTTODO;
					
				case DBEVENTPHASE_ABOUTTODO:
					if(pExecutionManager()->pCRowsetObj()->IsSupportedPhase(DBEVENTPHASE_SYNCHAFTER))
						return DBEVENTPHASE_SYNCHAFTER;
					
				case DBEVENTPHASE_SYNCHAFTER:
					if(pExecutionManager()->pCRowsetObj()->IsSupportedPhase(DBEVENTPHASE_DIDEVENT))
						return DBEVENTPHASE_DIDEVENT;
			}
	}

	return 0;
}

CExecutionManager* CImpIListener::pExecutionManager()
{
	ASSERT(m_pExecutionManager);
	return m_pExecutionManager;
}

CExpectationsManager* CImpIListener::pExpectationsManager()
{
	ASSERT(m_pExecutionManager);
	return m_pExecutionManager->pExpectationsManager();
}


///////////////////////////////////////////////////////////////////////////////
// CControl
//
///////////////////////////////////////////////////////////////////////////////
CControl::CControl(CExecutionManager* pExecutionManager)
{
	ASSERT(pExecutionManager);
	m_pExecutionManager = pExecutionManager;

	//Create a Suspended Thread
	m_hThread = CreateThread(NULL, 1024, CControl::ThreadFunction, this, CREATE_SUSPENDED, &m_dwThreadId);
	TESTC(m_hThread != 0);

CLEANUP:
	return;
}

CControl::~CControl() 
{
	CloseHandle(m_hThread);
}

DWORD CControl::GetThreadId() 
{ 
	return m_dwThreadId; 
}

DWORD CControl::WaitForThread() 
{ 
	return WaitForThreads(1, &m_hThread); 
}

ULONG CControl::Execute()
{
	return ResumeThread(m_hThread);
}	

ULONG WINAPI CControl::ThreadFunction(void* pv)
{
	THREAD_BEGIN
	CControl* pThis = (CControl*)pv;
	ASSERT(pThis);
	CExecutionManager* pExecutionManager = pThis->pExecutionManager();
	CExpectationsManager* pExpectationsManager = pThis->pExpectationsManager();
	ASSERT(pExecutionManager && pExpectationsManager);
	HRESULT hr = S_OK;

	//This is a Thread function, it must be a static function, taking void* and
	//returning ULONG.  Since it must be static, there is no "this" pointer,
	//so we pass in the this pointer to the object...

	//While there are Commands to be executed
	while(!pExecutionManager->IsCommandListEmpty())
	{
		
		//On the list are "Listener" Commands and "Control" Commands
		//Pop off a "Control" Command, if there is not one left but there
		//are items still on the list, that means some listeners never
		//got to use their commands, meanig they didn't get called for that
		//reason/phase they were looking for...
		COMMAND* pCommand = pExecutionManager->PopCommand(pThis);
		if(pCommand != NULL)
		{
			//Popoff and execute indented command
			QTESTC(pExpectationsManager->PushNestedExpectation(pCommand));
			QTESTC(pExecutionManager->ExecuteCommand(pCommand));
			QTESTC(pExpectationsManager->PopNestedExpectation(pCommand));
		}
		else
		{
			//Display and indicate Commands that never got to be used...
			pCommand = pExecutionManager->PopCommand();
			pCommand->DisplayCommand();
		}

		//Cleanup objects
		SAFE_DELETE(pCommand);
	}


CLEANUP:
	THREAD_RETURN;
}


CExecutionManager* CControl::pExecutionManager()
{
	ASSERT(m_pExecutionManager);
	return m_pExecutionManager;
}

CExpectationsManager* CControl::pExpectationsManager()
{
	ASSERT(m_pExecutionManager);
	return m_pExecutionManager->pExpectationsManager();
}


///////////////////////////////////////////////////////////////////////////////
// CExecutionManager
//
///////////////////////////////////////////////////////////////////////////////
CExecutionManager::CExecutionManager(const WCHAR* pwszTestCaseName)
	: CTestCases(pwszTestCaseName)
{
	HRESULT hr = S_OK;

	m_pCRowsetObj = NULL;
	m_pExpectationsManager = NULL;
	m_pCurrentCommand = NULL;

	// This test case was just instantiated, so check the threading model of the creating thread:
	// If we are in an STA (single-threaded apartment), then the ThreadID of the calling thread is not
	// sufficient to identify the calling control.  This is because all notifications are marshalled
	// across apartment (thread) boundaries.
	m_fFreeThreaded = true;
	hr = CoInitializeEx(NULL, 0 /* COINIT_MULTITHREADED */);
	if (RPC_E_CHANGED_MODE == hr)
		m_fFreeThreaded = false;				// We are in an STA
	else
		CoUninitialize();						// Make sure Init is matched by Uninit
}

CExecutionManager::~CExecutionManager()
{
	Terminate();
}

BOOL CExecutionManager::Init()
{
	m_pExpectationsManager = new CExpectationsManager(this);
	return m_pExpectationsManager != NULL;
}

BOOL CExecutionManager::Terminate()
{
	//Need to destory everything here
	SAFE_DELETE(m_pCRowsetObj);
	SAFE_DELETE(m_pExpectationsManager);
	DestroyStacks();
	return TRUE;
}

ULONG CExecutionManager::Execute(LPCWSTR wszCommandString, ROWSET_MODE dwRowsetMode)
{
	TBEGIN
	TESTC(ParseCommandString(wszCommandString)); 

	// If we're running in an STA, listeners cannot receive notifications from multiple controls
	if (!m_fFreeThreaded && m_mapControl.GetCount() > 1)
		{
		TOUTPUT(L"Listeners cannot receive notifications from multiple controls when running in apartment model.");
		return TEST_SKIPPED;
		}

	//Execute
	TESTC_PROVIDER(CreateRowset(dwRowsetMode)==S_OK);
	TESTC(AdviseListeners());
	TESTC(RunControls());

	//For some SCENARIOS, it would be useful (ie: save duplicataion of code), if we also 
	//tested the same senario (same commandstring) with a different rowset type.
	
	//For example: if the request rowset is buffered mode, it might also be useful to
	//test the QBU mode code as well.  This way the one variation tests both types of rowsets
	//for the senario without duplication or maintaince issues...
	if(dwRowsetMode & CHANGE_BUFFERRED)
	{
		//Execute
		if(SupportedProperty(KAGPROP_QUERYBASEDUPDATES, DBPROPSET_PROVIDERROWSET))
		{
			dwRowsetMode |= CHANGE_QBU;
			TESTC_(CreateRowset(dwRowsetMode),S_OK);
			TESTC(AdviseListeners());
			TESTC(RunControls());
		}
	}

	//For example: many SCENARIOS are the "least common demoninator" meaning to get coverage on
	//all providers they don't ask for many (or any properties).  But as we have seen many properties
	//will trigger different code paths and tigger bugs in notifications as well.  So if the senario
	//requires a bare-bones (default) rowset, then also try and somewhat more functional rowset.
	//We obvious can loop through all possible rowset combinations (in a reasonable amount of time)
	//but this seems to be the most interesting (default=usally forward only, second=keyset-scrollable).
	if(dwRowsetMode & DEFAULT_ROWSET)
	{
		dwRowsetMode |= (SCROLLBACKWARDS | FETCHBACKWARDS);
		if(CreateRowset(dwRowsetMode)==S_OK)
		{
			TESTC(AdviseListeners());
			TESTC(RunControls());
		}
	}

CLEANUP:
	UnadviseListeners();
	DestroyStacks();
	TRETURN;
}


BOOL CExecutionManager::ParseCommandString(LPCWSTR wszCommandString)
{
	yylpwzInput = wszCommandString;
	yypExecutionManager = this;
    yypCommandListMap = &m_mapCommandList;
    yypControlMap = &m_mapControl;
    yypListenerMap = &m_mapListener;
	yylexInit();
	return yyparse() == 0;
}


HRESULT CExecutionManager::CreateRowset(ROWSET_MODE dwRowsetMode)
{
	SAFE_DELETE(m_pCRowsetObj);
	m_pCRowsetObj = new CRowsetObj();
	return m_pCRowsetObj->CreateRowset(dwRowsetMode);
}

BOOL CExecutionManager::DestroyStacks()
{
	POSITION pos = 0;
	DWORD Key = 0;  

	//CContol
	pos = m_mapControl.GetStartPosition();
	while(pos)
	{
		CControl* pControl = NULL;
		m_mapControl.GetNextAssoc(pos, Key, pControl);
		SAFE_DELETE(pControl);
	}
	m_mapControl.RemoveAll();

	//CImpIListener
	pos = m_mapListener.GetStartPosition();
	while(pos)
	{
		CImpIListener* pListener = NULL;
		m_mapListener.GetNextAssoc(pos, Key, pListener);
		SAFE_RELEASE(pListener);
	}
	m_mapListener.RemoveAll();

	//CommandList
	pos = m_mapCommandList.GetStartPosition();
	while(pos)
	{
		COMMANDLIST* pCommandList = NULL;
		m_mapCommandList.GetNextAssoc(pos, Key, pCommandList);
		
		//CCOMAND
		POSITION pos2 = pCommandList->GetHeadPosition();
		while(pos2)
		{
			COMMAND* pCommand = NULL;
			pCommand = pCommandList->GetNext(pos2);
			SAFE_DELETE(pCommand);
		}

		SAFE_DELETE(pCommandList);
	}
	m_mapCommandList.RemoveAll();
	return TRUE;
}


BOOL CExecutionManager::AdviseListeners(CImpIListener* pListener)
{
	//If pListener == NULL, it will Advise all Listeners
	//Otherwise just the specified Listener with be Added and Advised

	ULONG key = 0; 
	CImpIListener* pImpIListener = NULL;
	IRowset* pIRowset = m_pCRowsetObj ? pCRowsetObj()->pIRowset() : NULL;
	
	//If NULL Advise all Listeners
	POSITION pos = m_mapListener.GetStartPosition();
	while(pos && pListener==NULL && pIRowset) 
	{
		m_mapListener.GetNextAssoc(pos, key, pImpIListener);
		pImpIListener->Advise(pCRowsetObj()->pIRowset());
	}

	if(pListener && pIRowset)
	{
		m_mapListener.SetAt(PtrToUlong(pListener), pListener);
		pListener->Advise(pCRowsetObj()->pIRowset());
	}

	return TRUE;
}


BOOL CExecutionManager::UnadviseListeners(CImpIListener* pListener)
{
	//If pListener == NULL, it will Unadvise and remove all Listeners
	//Otherwise just Unadvise/Remvoe the specified Listener
	
	ULONG key = 0;
	CImpIListener* pImpIListener = NULL;
	IRowset* pIRowset = m_pCRowsetObj ? pCRowsetObj()->pIRowset() : NULL;

	//Remove Listener from list
	POSITION pos = m_mapListener.GetStartPosition();
	while(pIRowset && pos) 
	{
		m_mapListener.GetNextAssoc(pos, key, pImpIListener);
		
		if(pListener==NULL || pListener==pImpIListener)
		{
			pImpIListener->Unadvise(pCRowsetObj()->pIRowset());
			m_mapListener.RemoveKey(key);
			SAFE_RELEASE(pImpIListener);
		}
	}
	
	return TRUE;
}


BOOL CExecutionManager::RunControls()
{
	ULONG_PTR nControlCount = m_mapControl.GetCount();

	ULONG key = 0; 
	CControl* pControl = NULL;
	POSITION pos = NULL;
	ULONG_PTR i = 0;
	
	//Start all the threads
	pos = m_mapControl.GetStartPosition();

	for(i = 0; i < nControlCount; i++)
	{
		m_mapControl.GetNextAssoc(pos, key, pControl);
		pControl->Execute();
	}

	//Wait for all threads to finish
	pos = m_mapControl.GetStartPosition();
	for(i=0; i<nControlCount; i++)
	{
		m_mapControl.GetNextAssoc(pos, key, pControl);
		pControl->WaitForThread();
	}
	
	UnadviseListeners();
	DestroyStacks();
 	return TRUE;
}

POSITION CExecutionManager::GetListenerStartPosition()
{
	return m_mapListener.GetStartPosition();
}

CImpIListener* CExecutionManager::NextListener(POSITION& pos)
{
	ULONG key = 0; 
	CImpIListener* pImpIListener = NULL; 
	m_mapListener.GetNextAssoc(pos, key, pImpIListener);
	return pImpIListener;
}

BOOL CExecutionManager::ExecuteCommand(COMMAND* pCOMMAND)
{
	ASSERT(pCOMMAND);

	//Setup the Thread arguments
	THREADARG T1 = { this, pCOMMAND };
	BOOL bResult = TRUE;

	if(pCOMMAND->m_cThreads)
	{
		DWORD dwThreadId = 0;
		HANDLE hThread = CreateThread(NULL, 1024, CExecutionManager::Thread_ExecuteCommand, &T1, 0, &dwThreadId);

		//Wait for it to finish...
		WaitForThreads(1, &hThread);
		bResult = GetThreadCode(hThread);
	}
	else
	{
		//No threads...
		bResult = Thread_ExecuteCommand(&T1);
	}

	return bResult;
}

ULONG CExecutionManager::Thread_ExecuteCommand(void* pv)
{
	HRESULT hr = E_FAIL;
	BOOL bReturn = FALSE;
	CExecutionManager* pThis	= (CExecutionManager*)THREAD_FUNC;
	COMMAND* pCommand			= (COMMAND*)THREAD_ARG1;

	//Save the old command
	COMMAND* pSavedCommand = pThis->GetCurrentCommand();
	pThis->m_pCurrentCommand = pCommand;
	
	//Make our lives easier...
	CRowsetObj* pCRowsetObj = pThis->pCRowsetObj();

	//Determine the OLEDB Method to call
	switch(pCommand->m_eCommandType)
	{
		case SETDATA:
			hr = pCRowsetObj->SetData(pCommand);
			break;
		
		case DELETEROWS:
			hr = pCRowsetObj->DeleteRows(pCommand);
			break;
		
		case INSERTROW:
			hr = pCRowsetObj->InsertRow(pCommand);
			break;
		
		case GETNEXTROWS:
			hr = pCRowsetObj->GetNextRows(pCommand);
			break;
		
		case ADDREFROWS:
			hr = pCRowsetObj->AddRefRows(pCommand);
			break;
		
		case RELEASEROWS:
			hr = pCRowsetObj->ReleaseRows(pCommand);
			break;
		
		case UPDATE:
			hr = pCRowsetObj->Update(pCommand);
			break;
		
		case UNDO:
			hr = pCRowsetObj->Undo(pCommand);
			break;
		
		case RESYNCHROWS:
			hr = pCRowsetObj->ResynchRows(pCommand);
			break;
		
		case ADDREFROWSET:
			pCRowsetObj->AddRefRowset();
			hr = S_OK;
			break;
		
		case RELEASEROWSET:
			if(pCRowsetObj->m_cRowsetRef==0)
			{
				SAFE_DELETE(pThis->m_pCRowsetObj);
			}
			else
			{
				pCRowsetObj->ReleaseRowset();
			}
			hr = S_OK;
			break;
		
		case RESTARTPOSITION:
			hr = pCRowsetObj->RestartPosition(pCommand);
			break;
		
		case GETROWSAT:
			hr = pCRowsetObj->GetRowsAt(pCommand);
			break;
		
		case GETROWSATRATIO:
			hr = pCRowsetObj->GetRowsAtRatio(pCommand);
			break;

		case GETROWSBYBOOKMARK:
			hr = pCRowsetObj->GetRowsByBookmark(pCommand);
			break;

		case GETDATA:
			hr = pCRowsetObj->GetData(pCommand);
			break;

		case GETORIGINALDATA:
			hr = pCRowsetObj->GetOriginalData(pCommand);
			break;

		case GETVISIBLEDATA:
			hr = pCRowsetObj->GetVisibleData(pCommand);
			break;
		
		case ADDCOLUMN:
			hr = pCRowsetObj->AddColumn();
			if(FAILED(hr))
			{
				bReturn = FALSE;
				goto CLEANUP;
			}
			break;

		case DROPCOLUMN:
			hr = pCRowsetObj->DropColumn();
			if(FAILED(hr))
			{
				bReturn = FALSE;
				goto CLEANUP;
			}
			break;

		case GETCOLUMNINFO:
			hr = pCRowsetObj->GetColumnInfo();
			break;

		default:
			//Unknown Command
			TCHECK(hr, S_OK);
	}

	//Verify HRESULT returned
	if(pCommand->m_fCanceled)
	{
		//if the command was Vetod by a listener then it should be
		//one of the "CANCELED" return codes 
		switch(pCommand->m_eCommandType)
		{
			case DELETEROWS:
			case RESYNCHROWS:
			case UPDATE:
				if (pCommand->m_cRows == 1)
					TESTC_(hr, DB_E_ERRORSOCCURRED)
				else
					TESTC_(hr, DB_S_ERRORSOCCURRED);
				break;

			case UNDO:
				TESTC_(hr, DB_E_ERRORSOCCURRED);
				break;

			default:
				TESTC_(hr, DB_E_CANCELED);
				break;
		}
	}
	else
	{
		//Otherwise it should be whatever was epected
		switch(pCommand->m_hrExpected)
		{
			case DB_E_NOTREENTRANT:
				//According to the spec Providers may or may not support reentrantcy
				//And even if they support it, some methods still might not be.
				if(pCRowsetObj->GetReentrantEvents()==FALSE)
					TESTC_(hr, DB_E_NOTREENTRANT)
				else
					TEST2C_(hr, S_OK, DB_E_NOTREENTRANT);
				pCommand->m_hrExpected = hr;
				break;

			case DB_E_BADSTARTPOSITION:
				//We need to expect DB_E_BADSTARTPOSITION as this indicates to the 
				//framework that we want to pass a large offset, but the spec
				//actually requires DB_S_ENDOFROWSET for 2.x+ providers.
				//So we will as well, but still need the 2 states for testing...
				TESTC_(hr, DB_S_ENDOFROWSET);
				break;

			default:
				TESTC_(hr, pCommand->m_hrExpected);
				break;
		}
	}

	bReturn = TRUE;
	
CLEANUP:
	//Restore the Saved command
	pThis->m_pCurrentCommand = pSavedCommand;
	return bReturn;
}

CRowsetObj* CExecutionManager::pCRowsetObj()
{
	ASSERT(m_pCRowsetObj);
	return m_pCRowsetObj;
}

CExpectationsManager* CExecutionManager::pExpectationsManager()
{
	ASSERT(m_pExpectationsManager);
	return m_pExpectationsManager;
}

COMMAND* CExecutionManager::GetCurrentCommand()
{
	return m_pCurrentCommand;
}

COMMANDLIST * CExecutionManager::LookupCommandList()
{
	COMMANDLIST * pCommandList = NULL;

	// If we're free-threaded, then use the current thread ID to locate the correct command list	
	if (m_fFreeThreaded)
		m_mapCommandList.Lookup(GetCurrentThreadId(), pCommandList);
	// Otherwise, there should be only one command list.  Get that by iterating.
	else
		{
		POSITION pos = 0;
		DWORD dwKey = 0;

		ASSERT(1 == m_mapCommandList.GetCount());
		pos = m_mapCommandList.GetStartPosition();

		m_mapCommandList.GetNextAssoc(pos, dwKey, pCommandList);
		}

	return pCommandList;
}

BOOL CExecutionManager::IsCommandListEmpty()
{
	COMMANDLIST* pCommandList = LookupCommandList();



	return pCommandList ? pCommandList->IsEmpty() : TRUE;
}

COMMAND* CExecutionManager::PopCommand(CControl* pControl)
{
	//Passing NULL with just pop of the head of the list...
	COMMANDLIST* pCommandList = NULL;
	POSITION pos = 0;

	pCommandList = LookupCommandList();
	TESTC(NULL != pCommandList);

	pos = pCommandList->GetHeadPosition();
	while(pos)
	{
		POSITION posSave = pos;
		COMMAND* pCommand = pCommandList->GetNext(pos);
		if(pControl && pCommand->m_pvObjectId != pControl) 
			continue;
		
		pCommandList->RemoveAt(posSave);
		return pCommand;
	}
	
CLEANUP:
	return NULL;
}


COMMAND* CExecutionManager::PopCommand
(
	CImpIListener* pListener,		
	DBREASON       eReason,			
	DBEVENTPHASE   ePhase			
)
{
	ASSERT(pListener);
	
	COMMANDLIST* pCommandList = NULL;
	POSITION pos = 0;

	pCommandList = LookupCommandList();
	TESTC(NULL != pCommandList);

	pos = pCommandList->GetHeadPosition();
	while(pos)
	{
		POSITION posSave = pos;
		COMMAND* pCommand = pCommandList->GetNext(pos);
		if(pCommand->m_pvObjectId != pListener) 
			continue;
		if(pCommand->m_ulCommandLevel != GetCurrentCommand()->m_ulCommandLevel) 
			continue;
		if(pCommand->m_eReason != eReason) 
			continue;
		if(pCommand->m_ePhase != ePhase) 
			continue;
		
		pCommandList->RemoveAt(posSave);
		return pCommand;
	}
	
CLEANUP:
	return new COMMAND(RETURN_ACCEPT);
}


BOOL CExecutionManager::IsCancelableEvent(DBREASON eReason, DBEVENTPHASE ePhase)
{
	return pCRowsetObj()->IsCancelableEvent(eReason, ePhase);
}


/////////////////////////////////////////////////////////////////
// CExpectationsManager
//
/////////////////////////////////////////////////////////////////
CExpectationsManager::CExpectationsManager(CExecutionManager* pExecutionManager)
{
	ASSERT(pExecutionManager);
	m_pExecutionManager = pExecutionManager;
}

CExpectationsManager::~CExpectationsManager()
{
}

CRowsetObj* CExpectationsManager::pCRowsetObj()
{
	ASSERT(m_pExecutionManager);
	return m_pExecutionManager->pCRowsetObj();
}

CExecutionManager* CExpectationsManager::pExecutionManager()
{
	ASSERT(m_pExecutionManager);
	return m_pExecutionManager;
}

BOOL CExpectationsManager::PushNextExpectation(EXPECT* pExpect)
{
	ASSERT(pExpect);
	return pExpect->SetNextPhase();
}

BOOL CExpectationsManager::PushUnwantedReason(CImpIListener* pListener, IRowset* pIRowset, DBREASON eReason)
{
	TBEGIN
	ASSERT(pListener);
	ASSERT(pIRowset);

	POSITION pos = 0;
	
	//All Expecations for this reason
	COMMAND* pCommand = pExecutionManager()->GetCurrentCommand();
	pos = pCommand->m_listExpect.GetHeadPosition();
	while(pos)
	{
		EXPECT* pExpect = pCommand->m_listExpect.GetNext(pos);
		TESTC(pExpect != NULL);

		//Should not expect any more notifications
		if(pExpect->m_pListener == pListener && pExpect->m_pIRowset == pIRowset && pExpect->m_eReason == eReason)
			pExpect->m_bCompleted = TRUE;
	}

CLEANUP:
	TRETURN;
}

BOOL CExpectationsManager::PushUnwantedPhase(CImpIListener* pListener, IRowset* pIRowset, DBREASON eReason, DBEVENTPHASE ePhase)
{
	TBEGIN
	ASSERT(pListener);
	ASSERT(pIRowset);

	POSITION pos = 0;
	
	//All Expecations for this reason/phase should expect the next phase... - not quite correct: see bug 62019
	COMMAND* pCommand = pExecutionManager()->GetCurrentCommand();
	pos = pCommand->m_listExpect.GetHeadPosition();
	while(pos)
	{
		EXPECT* pExpect = pCommand->m_listExpect.GetNext(pos);
		TESTC(pExpect != NULL);

		//Per the spec:DB_S_UNWANTEDPHASE - The provider can optimize by making no further calls with this reason and phase but 
		//but provider can still call consumer with this reason and phase
		if(pExpect->m_pListener == pListener && pExpect->m_pIRowset == pIRowset && pExpect->m_eReason == eReason && pExpect->m_ePhase == ePhase)
			pExpect->m_bCompleted = TRUE;

	}

CLEANUP:
	TRETURN;
}

BOOL CExpectationsManager::PushAdviseExpectation(EXPECT* pExpect)
{
	ASSERT(pExpect);

	//Advise a new Listener
	CImpIListener* pNewListener = new CImpIListener(pExecutionManager());
	pExecutionManager()->AdviseListeners(pNewListener);

	//Also need to add Expectations for this listener!
	COMMAND* pCommand = pExecutionManager()->GetCurrentCommand();
	POSITION pos = pCommand->m_listExpect.GetHeadPosition();
	while(pos)
	{
		EXPECT* pCurExpect = pCommand->m_listExpect.GetNext(pos);
		if(pCurExpect->m_pListener == pExpect->m_pListener)
		{
			//Create a new Expectation for the newly added Listener
			//It should be expecting the phases as this listener (Copy constructor)
			
			//This expectation is completly optional, meaning that it is provider
			//specific weither the provider notifies Listeners that have advised
			//in the middle of a notification.  The nice thing about this framework
			//is that even if the provider does send notifications we can determine
			//we are getting the correct ones and in the correct order...
			EXPECT* pNewExpect = new EXPECT(*pCurExpect);
			pNewExpect->m_pListener = pNewListener;
			pNewExpect->m_fOptional = TRUE;
			pNewExpect->m_bCompleted = FALSE;
			pNewExpect->m_ulTimesNotified = 0;

			//Add this Listener to the List
			pCommand->m_listExpect.AddTail(pNewExpect);
		}
	}

	//We have recieved this Notification
	PushNextExpectation(pExpect);
	return TRUE;
}

BOOL CExpectationsManager::PushUnadviseExpectation(EXPECT* pExpect)
{
	TBEGIN
	ASSERT(pExpect);

	//We need to save a pointer to the Listener
	//Since deleting the expectation from the list will remove the same
	//Expecation passed in...
	CImpIListener* pListener = pExpect->m_pListener;

	//Unadvise this Listener
	pExecutionManager()->UnadviseListeners(pListener);
	
	//Also need to remove Expectations for this listener!
	COMMAND* pCommand = pExecutionManager()->GetCurrentCommand();
	POSITION pos = pCommand->m_listExpect.GetHeadPosition();
	while(pos)
	{
		POSITION posSave = pos;
		EXPECT* pCurExpect = pCommand->m_listExpect.GetNext(pos);
		if(pCurExpect->m_pListener == pListener)
		{
			pCommand->m_listExpect.RemoveAt(posSave);
			SAFE_DELETE(pCurExpect);
		}
	}
	
	TRETURN;
}

BOOL CExpectationsManager::PushFalseExpectation(EXPECT* pFalseExpect)
{
	//A Listener has Veto'd a change, and we need to figure out whats 
	//expected for all addtional notifications for FAILEDTODO
	TBEGIN
	ASSERT(pFalseExpect);
	
	POSITION pos = 0;
	COMMAND* pCommand = pExecutionManager()->GetCurrentCommand();
	pCommand->m_fCanceled = (pCommand->m_hrExpected == S_OK);
		
	pos = pCommand->m_listExpect.GetHeadPosition();
	while(pos)
	{
		EXPECT* pExpect = pCommand->m_listExpect.GetNext(pos);

		//The Expectations not waiting for this notification
		//Should expect FAILEDTODO if they have already started receiving phases
		if(pExpect->m_eReason != pFalseExpect->m_eReason)
		{
			if(!pExpect->IsCompleted())
			{
				pExpect->m_fCanceled = pCommand->m_fCanceled;
				if(pExpect->m_ulTimesNotified)
					pExpect->SetPhase(DBEVENTPHASE_FAILEDTODO);
			}
		}

		//Expectations waiting for this notificaiton 
		//Should expect FAILEDTODO if they have already started receiving phases
		if(pExpect->IsEqual(pExpect->m_pListener, pFalseExpect->m_pIRowset, pFalseExpect->m_cRows, pFalseExpect->m_rghRows, pFalseExpect->m_cColumns, pFalseExpect->m_rgColumns, pFalseExpect->m_eReason, pExpect->m_ePhase) ||
			(pExpect->m_eReason==DBREASON_ROW_INSERT && pExpect->m_ulTimesNotified==0))
		{
			if(!pExpect->IsCompleted())
			{
				pExpect->m_fCanceled = pCommand->m_fCanceled;
				if(pExpect->m_ulTimesNotified)
					pExpect->SetPhase(DBEVENTPHASE_FAILEDTODO);
			}
		}
	}

	TRETURN;
}


BOOL CExpectationsManager::PushNestedExpectation(COMMAND* pCommand)
{
	TBEGIN
	ASSERT(pCommand);
	POSITION pos = 0;

	//Setup
	DBCOUNTITEM cRows = pCommand->m_cRows;
	HROW* rghRows = pCommand->m_rghRows;
	IRowset* pIRowset = pCRowsetObj()->pIRowset();
	DBCOUNTITEM i;

	ROWSTATUS rgRowStatus[MAX_ROW_COUNT];
	ULONG rgRefCounts[MAX_ROW_COUNT];
	ULONG fFirstChange = FALSE;

	//MapRowIds = RowId -> hRow
	TESTC(pCRowsetObj()->MapRowIds(pCommand));

	//Lookup rowstatus values
	for(i=0; i<cRows || i<pCommand->m_dwOption; i++)
	{
		ROWINFO* pRowInfo = NULL;
		rgRowStatus[i] = ROWSTATUS_NOCHANGE;
		rgRefCounts[i] = 0;

		if(pRowInfo = pCRowsetObj()->GetRowInfo(pCommand->m_rgRowIds[i]))
		{
			rgRowStatus[i] = pRowInfo->m_eRowStatus;
			rgRefCounts[i] = pRowInfo->m_cRefCount;
			fFirstChange = pRowInfo->m_fFirstChange;
		}
	}
		
	pos = pExecutionManager()->GetListenerStartPosition();
	while(pos)
	{
		//Get the next listener
		CImpIListener* pListener = pExecutionManager()->NextListener(pos);
		TESTC(pListener != NULL);

		switch(pCommand->m_eCommandType)
		{
			case RELEASEROWSET:
				//DBREASON_ROWSET_RELEASE
				//Release Rowset only gets sent when Reference count goes to 0
				if(pCRowsetObj()->m_cRowsetRef==0)
				{
					EXPECT* pExpect = new EXPECT(pListener, pIRowset, 0, NULL, 0, NULL, DBREASON_ROWSET_RELEASE, DBEVENTPHASE_DIDEVENT);
					pCommand->m_listExpect.AddTail(pExpect);
				}
				break;

			case GETNEXTROWS:
				{
					//DBREASON_ROWSET_FETCHPOSITIONCHANGE
					if(cRows || pCommand->m_hrExpected==DB_S_ENDOFROWSET)
					{
						//TODO should only expect FETCHPOSITIONCHANGE if position is going to change!
						EXPECT* pExpect = new EXPECT(pListener, pIRowset, 0, NULL, 0, NULL, DBREASON_ROWSET_FETCHPOSITIONCHANGE, DBEVENTPHASE_OKTODO);
						pCommand->m_listExpect.AddTail(pExpect);
					}
					//DBREASON_ROW_ACTIVATE
					//ROW_ACTIVATE is a siglephase event, according to the spec
					//the provider makes a single call to OnRowChange for all rows
					DBCOUNTITEM cRowsNew = 0;
					HROW rghRowsNew[MAX_ROW_COUNT];
					memset(rghRowsNew, 0, MAX_ROW_COUNT * sizeof(HROW));
					if(cRows && pCommand->m_hrExpected!=DB_S_ENDOFROWSET)
					{
						//Only expect new rows to be notified
						for(DBCOUNTITEM i=0; i<cRows; i++)
							if(rghRows[i]==DB_NULL_HROW || rgRefCounts[i]==0)
								rghRowsNew[cRowsNew++] = DB_NULL_HROW;

						//Some Providers may have a limit to the number of 
						//rows able to be fetched.  DBPROP_MAXOPENROWS
						DBCOUNTITEM ulMaxOpenRows = pCRowsetObj()->GetMaxOpenRows();
						if(ulMaxOpenRows!=0 && ulMaxOpenRows<cRowsNew)
							cRowsNew = ulMaxOpenRows;
					}
					else if (cRows==0 && pCommand->m_hrExpected==DB_S_ENDOFROWSET)
						cRowsNew = MAX_ROW_COUNT;
					//If there are any new Rows then expect a notification
					if(cRowsNew)
					{
						EXPECT* pExpect = new EXPECT(pListener, pIRowset, cRowsNew, rghRowsNew, 0, NULL, DBREASON_ROW_ACTIVATE, DBEVENTPHASE_DIDEVENT);
						pCommand->m_listExpect.AddTail(pExpect);
					}
				}
				break;

			case RELEASEROWS:
				//DBREASON_ROW_RELEASE
				{
					DBCOUNTITEM cRowsValid = 0;
					HROW rghRowsValid[MAX_ROW_COUNT];

					//Only expect the valid rows to be notified
					//Also only expect rows that have a will have a reference count 
					//of 0 after ReleaseRows to send notifications
					for(DBCOUNTITEM i=0; i<cRows; i++)
						if(rghRows[i]!=DB_NULL_HROW && rgRefCounts[i]==1)
							rghRowsValid[cRowsValid++] = rghRows[i];

					//If there are any Valid Rows then expect a notification
					if(cRowsValid)
					{
						EXPECT* pExpect = new EXPECT(pListener, pIRowset, cRowsValid, rghRowsValid, 0, NULL, DBREASON_ROW_RELEASE, DBEVENTPHASE_DIDEVENT);
						pCommand->m_listExpect.AddTail(pExpect);
					}
				}
				break;

			case RESTARTPOSITION:
				//DBREASON_ROWSET_FETCHPOSITIONCHANGE
				{
					//Should only expect FETCHPOSITIONCHANGE if the next fetch position changed
					if(pCRowsetObj()->m_ulNextFetchPos != 0)
					{

						// May require to release rows before RestartPosition if DBPROP_QUICKRESTART = TRUE
						//should handle this case
						if (!pCRowsetObj()->IsQuickRestart())
						{
								DBCOUNTITEM cRowsValid = 0;
								HROW rghRowsValid[MAX_ROW_COUNT];
								pCRowsetObj()->GetRowsToRelease(MAX_ROW_COUNT, cRowsValid, rghRowsValid);

								//If there are any Valid Rows then expect a notification
								if(cRowsValid)
								{
								EXPECT* pExpect = new EXPECT(pListener, pIRowset, cRowsValid, rghRowsValid, 0, NULL, DBREASON_ROW_RELEASE, DBEVENTPHASE_DIDEVENT);
								pCommand->m_listExpect.AddTail(pExpect);
								}
						}
					}
					
					EXPECT* pExpect = new EXPECT(pListener, pIRowset, 0, NULL, 0, NULL, DBREASON_ROWSET_FETCHPOSITIONCHANGE, DBEVENTPHASE_OKTODO);
					pCommand->m_listExpect.AddTail(pExpect);

					
					//DBREASON_ROWSET_CHANGED
					if(pCRowsetObj()->m_fTableAltered)
					{
						EXPECT* pExpect = new EXPECT(pListener, pIRowset, 0, NULL, 0, NULL, DBREASON_ROWSET_CHANGED, DBEVENTPHASE_DIDEVENT);
						pCommand->m_listExpect.AddTail(pExpect);
					}
				}
				break;

			case DELETEROWS:
			//DBREASON_ROW_DELETE
				if(cRows)
				{
					if(pCRowsetObj()->GetGranularity() == DBPROPVAL_NT_SINGLEROW)
					{
						for(DBCOUNTITEM i=0; i<cRows; i++)
						{
							EXPECT* pExpect = new EXPECT(pListener, pIRowset, ONE_ROW, &rghRows[i], 0, NULL, DBREASON_ROW_DELETE, DBEVENTPHASE_OKTODO);
							pCommand->m_listExpect.AddTail(pExpect);
						}
					}
					else
					{
						EXPECT* pExpect = new EXPECT(pListener, pIRowset, cRows, rghRows, 0, NULL, DBREASON_ROW_DELETE, DBEVENTPHASE_OKTODO);
						pCommand->m_listExpect.AddTail(pExpect);
					}
				}
				break;

			case INSERTROW:
			{	
				//DBREASON_ROW_INSERT
				{
					pCommand->m_listExpect.AddTail(new EXPECT(pListener, pIRowset, cRows, rghRows, 0, NULL, DBREASON_ROW_INSERT, DBEVENTPHASE_OKTODO));
				}
			}
			break;
			
			case SETDATA:
				//DBREASON_ROW_FIRSTCHANGE
				//Only is fired in BufferedMode
				if(pCRowsetObj()->IsBufferedMode() && fFirstChange)
				{
					EXPECT* pExpect = new EXPECT(pListener, pIRowset, cRows, rghRows, 0, NULL, DBREASON_ROW_FIRSTCHANGE, DBEVENTPHASE_OKTODO);
					pCommand->m_listExpect.AddTail(pExpect);
				}

				//DBREASON_COLUMN_SET
				{
					//We need to setup the expected columns.
					pCommand->m_cColumns = pCRowsetObj()->m_cUpBindings;
					SAFE_REALLOC(pCommand->m_rgColumns, DBORDINAL, pCommand->m_cColumns);
					for(DBORDINAL i=0; i<pCommand->m_cColumns; i++)
						pCommand->m_rgColumns[i] = pCRowsetObj()->m_rgUpBindings[i].iOrdinal;
					
					EXPECT* pExpect = new EXPECT(pListener, pIRowset, cRows, rghRows, pCommand->m_cColumns, pCommand->m_rgColumns, DBREASON_COLUMN_SET, DBEVENTPHASE_OKTODO);
					pCommand->m_listExpect.AddTail(pExpect);
				}

				//DBREASON_COLUMN_RECALCULATED
				if((pCRowsetObj()->m_dwRowsetMode & COMPUTED_COLUMNS) || (pCRowsetObj()->m_dwRowsetMode & COMPUTED_COLUMNS_INCLUDE))
				{
					//We need to setup the expected columns.
					pCommand->m_cColumns = pCRowsetObj()->m_cUpBindings;
					SAFE_REALLOC(pCommand->m_rgColumns, DBORDINAL, pCommand->m_cColumns);
					for(DBORDINAL i=0; i<pCommand->m_cColumns; i++)
						pCommand->m_rgColumns[i] = pCRowsetObj()->m_rgUpBindings[i].iOrdinal;
					
					EXPECT* pExpect = new EXPECT(pListener, pIRowset, cRows, rghRows, pCommand->m_cColumns, pCommand->m_rgColumns, DBREASON_COLUMN_RECALCULATED, DBEVENTPHASE_DIDEVENT);
					pCommand->m_listExpect.AddTail(pExpect);
				}
				break;
			

			case SEEK:
				//DBREASON_ROWSET_FETCHPOSITIONCHANGE
				{
					//TODO should only expect FETCHPOSITIONCHANGE if position is going to change!
					EXPECT* pExpect = new EXPECT(pListener, pIRowset, 0, NULL, 0, NULL, DBREASON_ROWSET_FETCHPOSITIONCHANGE, DBEVENTPHASE_OKTODO);
					pCommand->m_listExpect.AddTail(pExpect);
				}
				break;

			case GETROWSAT:
			case GETROWSBYBOOKMARK:
			case GETROWSATRATIO:
				//DBREASON_ROW_ACTIVATE
				if(cRows)
				{
					//ROW_ACTIVATE is a siglephase event, according to the spec
					//the provider makes a single call to OnRowChange for all rows
/*
					DBCOUNTITEM cRowsExpected = cRows;
					if(pCommand->m_dwOption == DBBMK_LAST && pCommand->m_eCommandType == GETROWSAT)
						cRowsExpected = 1;

					DBCOUNTITEM cRowsNew = 0;
					HROW rghRowsNew[MAX_ROW_COUNT];

					//Only expect new rows to be notified
					for(DBCOUNTITEM i=0; i<cRowsExpected; i++)
						if(rghRows[i]==DB_NULL_HROW || rgRefCounts[i]==0)
							rghRowsNew[cRowsNew++] = DB_NULL_HROW;

					//Some Providers may have a limit to the number of 
					//rows able to be fetched.  DBPROP_MAXOPENROWS
					DBCOUNTITEM ulMaxOpenRows = pCRowsetObj()->GetMaxOpenRows();
					if(ulMaxOpenRows!=0 && ulMaxOpenRows<cRowsNew)
						cRowsNew = ulMaxOpenRows;

					//If there are any new Rows then expect a notification
					if(cRowsNew)
					{
						EXPECT* pExpect = new EXPECT(pListener, pIRowset, cRowsNew, rghRowsNew, 0, NULL, DBREASON_ROW_ACTIVATE, DBEVENTPHASE_DIDEVENT);
						pCommand->m_listExpect.AddTail(pExpect);
					}
*/

				//The provider should send this reason for newly fetched rows only,
				//but it might be expensive for providers to determine whether or not a row is newly fetched
				// so will expect DBREASON_ROW_ACTIVATE for all rows

					DBCOUNTITEM cRowsExpected = cRows;
					if(pCommand->m_dwOption == DBBMK_LAST && pCommand->m_eCommandType == GETROWSAT)
						cRowsExpected = 1;

					HROW rghRowsExpect[MAX_ROW_COUNT];

					//All rows might be notified
					for(DBCOUNTITEM i=0; i<cRowsExpected; i++)
						if(rghRows[i]==DB_NULL_HROW || rgRefCounts[i]==0)
							rghRowsExpect[i] = DB_NULL_HROW;
						else
							rghRowsExpect[i] = rghRows[i];
					
					//Some Providers may have a limit to the number of 
					//rows able to be fetched.  DBPROP_MAXOPENROWS
					DBCOUNTITEM ulMaxOpenRows = pCRowsetObj()->GetMaxOpenRows();
					if(ulMaxOpenRows!=0 && ulMaxOpenRows<cRowsExpected)
						cRowsExpected = ulMaxOpenRows;

					if(cRowsExpected)
					{
						EXPECT* pExpect = new EXPECT(pListener, pIRowset, cRowsExpected, rghRowsExpect, 0, NULL, DBREASON_ROW_ACTIVATE, DBEVENTPHASE_DIDEVENT);
						pCommand->m_listExpect.AddTail(pExpect);
					}
				}

				break;
			
			case RESYNCHROWS:
				//DBREASON_ROW_RESYNCH
				{
					DBCOUNTITEM cRowsResynched = 0;
					HROW rghRowsResynched[MAX_ROW_COUNT];

					//Find all "Pending" rows
					for(DBCOUNTITEM i=0; i<cRows || i<pCommand->m_dwOption; i++)
					{
						//According to the spec resynch row notifications
						//are always, regardless if the row needed resynching
						//or not.  So for all rows passed to IRowsetResynch
						//all should be notified.
						if(rghRows[i] != DB_NULL_HROW)
							rghRowsResynched[cRowsResynched++] = rghRows[i];
					}

					if(pCRowsetObj()->GetGranularity() == DBPROPVAL_NT_SINGLEROW)
					{
						for(DBCOUNTITEM i=0; i<cRowsResynched; i++)
						{
							EXPECT* pExpect = new EXPECT(pListener, pIRowset, ONE_ROW, &rghRowsResynched[i], 0, NULL, DBREASON_ROW_RESYNCH, DBEVENTPHASE_OKTODO);
							pCommand->m_listExpect.AddTail(pExpect);
						}
					}
					else
					{
						EXPECT* pExpect = new EXPECT(pListener, pIRowset, cRowsResynched, rghRowsResynched, 0, NULL, DBREASON_ROW_RESYNCH, DBEVENTPHASE_OKTODO);
						pCommand->m_listExpect.AddTail(pExpect);
					}
				}
				break;

			case UNDO:
				//DBREASON_ROW_UNDOCHANGE
				{
					DBCOUNTITEM cRowsChanged = 0;
					HROW rghRowsChanged[MAX_ROW_COUNT];

					//Find all "Changed" rows
					for(DBCOUNTITEM i=0; i<cRows || i<pCommand->m_dwOption; i++)
					{
						//UndoChange is only sent for rows that have changed
						//and are undone, other "no-op" rows do not have notifications
						if(rghRows[i] && rgRowStatus[i] == ROWSTATUS_CHANGED)
							rghRowsChanged[cRowsChanged++] = rghRows[i];
					}
					
					if(pCRowsetObj()->GetGranularity() == DBPROPVAL_NT_SINGLEROW)
					{
						for(DBCOUNTITEM i=0; i<cRowsChanged; i++)
						{
							
							EXPECT* pExpect = new EXPECT(pListener, pIRowset, ONE_ROW, &rghRowsChanged[i], 0, NULL, DBREASON_ROW_UNDOCHANGE, DBEVENTPHASE_OKTODO);
							pCommand->m_listExpect.AddTail(pExpect);
						}
					}
					else
					{
						EXPECT* pExpect = new EXPECT(pListener, pIRowset, cRowsChanged, rghRowsChanged, 0, NULL, DBREASON_ROW_UNDOCHANGE, DBEVENTPHASE_OKTODO);
						pCommand->m_listExpect.AddTail(pExpect);
					}
				}

				//DBREASON_ROW_UNDOINSERT
				{
					DBCOUNTITEM cRowsInserted = 0;
					HROW rghRowsInserted[MAX_ROW_COUNT];

					//Find all "Inserted" rows
					for(DBCOUNTITEM i=0; i<cRows || i<pCommand->m_dwOption; i++)
					{
						//UndoInsert is only sent for rows that have changed
						//and are undone, other "no-op" rows do not have notifications
						if(rghRows[i] && rgRowStatus[i] == ROWSTATUS_INSERTED)
							rghRowsInserted[cRowsInserted++] = rghRows[i];
					}
					
					if(pCRowsetObj()->GetGranularity() == DBPROPVAL_NT_SINGLEROW)
					{
						for(DBCOUNTITEM i=0; i<cRowsInserted; i++)
						{
							
							EXPECT* pExpect = new EXPECT(pListener, pIRowset, ONE_ROW, &rghRowsInserted[i], 0, NULL, DBREASON_ROW_UNDOINSERT, DBEVENTPHASE_OKTODO);
							pCommand->m_listExpect.AddTail(pExpect);
						}
					}
					else
					{
						EXPECT* pExpect = new EXPECT(pListener, pIRowset, cRowsInserted, rghRowsInserted, 0, NULL, DBREASON_ROW_UNDOINSERT, DBEVENTPHASE_OKTODO);
						pCommand->m_listExpect.AddTail(pExpect);
					}
				}

				//DBREASON_ROW_UNDODELETE
				{
					DBCOUNTITEM cRowsDeleted = 0;
					HROW rghRowsDeleted[MAX_ROW_COUNT];

					//Find all "Deleted" rows
					for(DBCOUNTITEM i=0; i<cRows || i<pCommand->m_dwOption; i++)
					{
						//UndoDelete is only sent for rows that have changed
						//and are undone, other "no-op" rows do not have notifications
						if(rghRows[i] && rgRowStatus[i] == ROWSTATUS_DELETED)
							rghRowsDeleted[cRowsDeleted++] = rghRows[i];
					}
					
					if(pCRowsetObj()->GetGranularity() == DBPROPVAL_NT_SINGLEROW)
					{
						for(DBCOUNTITEM i=0; i<cRowsDeleted; i++)
						{
							
							EXPECT* pExpect = new EXPECT(pListener, pIRowset, ONE_ROW, &rghRowsDeleted[i], 0, NULL, DBREASON_ROW_UNDODELETE, DBEVENTPHASE_OKTODO);
							pCommand->m_listExpect.AddTail(pExpect);
						}
					}
					else
					{
						EXPECT* pExpect = new EXPECT(pListener, pIRowset, cRowsDeleted, rghRowsDeleted, 0, NULL, DBREASON_ROW_UNDODELETE, DBEVENTPHASE_OKTODO);
						pCommand->m_listExpect.AddTail(pExpect);
					}
				}
				break;

			case UPDATE:
				//DBREASON_ROW_UPDATE
				{
					DBCOUNTITEM cRowsPending = 0;
					HROW rghRowsPending[MAX_ROW_COUNT];

					//Find all "Pending" rows
					for(DBCOUNTITEM i=0; i<cRows || i<pCommand->m_dwOption; i++)
					{
						//Update is only sent for rows that have changed
						//and are undone, other "no-op" rows do not have notifications
						if(rghRows[i] && rgRowStatus[i] != ROWSTATUS_NOCHANGE)
							rghRowsPending[cRowsPending++] = rghRows[i];
					}
					
					if(pCRowsetObj()->GetGranularity() == DBPROPVAL_NT_SINGLEROW)
					{
						for(DBCOUNTITEM i=0; i<cRowsPending; i++)
						{
							EXPECT* pExpect = new EXPECT(pListener, pIRowset, ONE_ROW, &rghRowsPending[i], 0, NULL, DBREASON_ROW_UPDATE, DBEVENTPHASE_OKTODO);
							pCommand->m_listExpect.AddTail(pExpect);
						}
					}
					else
					{
						EXPECT* pExpect = new EXPECT(pListener, pIRowset, cRowsPending, rghRowsPending, 0, NULL, DBREASON_ROW_UPDATE, DBEVENTPHASE_OKTODO);
						pCommand->m_listExpect.AddTail(pExpect);
					}
				}
				break;


			default:
				//No expected notifications
				break;
		}
	}

		
CLEANUP:
	TRETURN
}

BOOL CExpectationsManager::PopNestedExpectation(COMMAND* pCommand)
{
	TBEGIN
	while(!pCommand->m_listExpect.IsEmpty())
	{
		EXPECT* pExpect = pCommand->m_listExpect.RemoveHead();
		TCHECK(pExpect != NULL, TRUE);
		
		//This maybe a optional notification
		//Optional notifications are for Listeners that have been advised in the 
		//middle, it is provider specific weither or not the provider sends
		//any notifications to this listener until the next notification
		if(pExpect->m_fOptional)
		{
			if(pExpect->m_bCompleted)
				TCHECK(pExpect->VerifyComplete(), TRUE);
		}
		//If the Notification was veto'd by a listener
		else if(pCommand->m_fCanceled)
		{
			//If it started to receive Notifications, then it should have an 
			//Incomplete sequence, otherwise no phases should have been fired
			if(pExpect->m_fCanceled)
			{
				TCHECK(pExpect->VerifyInComplete(),TRUE);
			}
			else
			{
				TCHECK(pExpect->VerifyComplete(),TRUE);
			}
		}
		else
		{
			//If it wasn't vetod by a listener then the phases should be 
			//COMPLETE if the expected HRESULT was S_OK (default), or if we were
			//purposly inposing an error, it should be what we expect...
			switch(pCommand->m_hrExpected)
			{
				case E_INVALIDARG:
				case DB_E_NOTREENTRANT:
				case DB_E_ERRORSOCCURRED:
				case DB_E_BADROWHANDLE:
				case DB_E_BADACCESSORHANDLE:
					//Failures that should have never been notified
					TCHECK(pExpect->VerifyInComplete(),TRUE);
					break;

				case DB_S_ENDOFROWSET:
					//Should only get INCOMPLETE if asked for no rows
					//otherwsie should have changed the position
					//TODO really should be a more robust way of knowing wither the fetchposition is going to change
					if(pCommand->m_cRows==0)
					{
						TCHECK(pExpect->VerifyInComplete(),TRUE);
					}
					else
					{
						TCHECK(pExpect->VerifyComplete(),TRUE);
					}
					break;

				case DB_E_BADSTARTPOSITION:
					//Canceled, should have been an incomplete sequence
					TCHECK(pExpect->VerifyInComplete(),TRUE);
					break;
					
				case S_OK:
				case DB_S_ERRORSOCCURRED:
				case DB_S_COMMANDREEXECUTED:
					//Success, should have been notified for all phases
					TCHECK(pExpect->VerifyComplete(),TRUE);
					break;

				default:
					//Unhandled HRESULT
					TCHECK(TRUE,FALSE);
			}
		}

		SAFE_DELETE(pExpect);
	}

	TRETURN;
}


EXPECT* CExpectationsManager::FindExpectation
(
	CImpIListener* pListener,	
	IRowset*     pIRowset,		
	DBCOUNTITEM		 cRows,
	HROW*		 rghRows,			
	DBORDINAL        cColumns,		
	DBORDINAL		 rgColumns[],   
	DBREASON     eReason,		
	DBEVENTPHASE ePhase,		
    BOOL		 fCantDeny		
)
{
	ASSERT(pListener);
	ASSERT(pIRowset);
	
	//PopExpectation
	EXPECT* pExpect = PopExpectation(pListener, pIRowset, cRows, rghRows, cColumns, rgColumns, eReason, ePhase);
	COMMAND* pCommand = pExecutionManager()->GetCurrentCommand();

	//Received a Notification it was not expecting, Output debug info
	if(pExpect == NULL)
	{
		DisplayNotification(L"Unexpected: ", cRows, rghRows, cColumns, rgColumns, eReason, ePhase);
		goto CLEANUP;
	}

	//Record this notification
	pExpect->m_ulTimesNotified++;

	//If we purposely are expecting an error (called OLEDB method with invalid results)
	//Then we will allow DBEVENTPHASE_FAILEDTODO at anytime
	if(ePhase==DBEVENTPHASE_FAILEDTODO && pCommand->m_hrExpected!=S_OK)
		TESTC(PushFalseExpectation(pExpect));

	//DBREASON_ROW_ACTIVATE
	//DBREASON_ROW_INSERT
	if(eReason == DBREASON_ROW_ACTIVATE || eReason == DBREASON_ROW_INSERT)
	{
		//With these reason you have no way of knowing ahead of time the
		//row handle values, just set when notified and then remember them...
		for(DBCOUNTITEM i=0; i<cRows; i++)
		{
			if(pExpect->m_rghRows[i] == DB_NULL_HROW)
			{
				//Update the expectation
				pExpect->m_rghRows[i] = rghRows[i];

				//Update the RowInfo as well...
				//TODO: The expectation should also have RowIDs to match this up with...
				ROWINFO* pRowInfo = pCRowsetObj()->GetRowInfo(pCommand->m_rgRowIds[i]);
				if(pRowInfo)
				{
					if(pRowInfo->m_hRow == DB_NULL_HROW || pRowInfo->m_cRefCount==0)
						pRowInfo->m_hRow = rghRows[i];
				}
			}
		}
	}

	//We need to handle UnWanted Phases.
	//Since this is an optimization, its upto the provider wiether or not they send
	//unwanted phases.  We need to be prepared and handle both cases...
	//Note that we never go to an endless loop because ePhase is a valid phase for eReason and we expect all phases for the reason
	while(pExpect->m_ePhase != ePhase && !pExpect->m_pListener->IsWantedPhase(eReason, pExpect->m_ePhase))
	{
		//Get the next phase in sequence
		pExpect->SetNextPhase();
	}

	//We need to handle Optional Phases.
	//Since it is provider specific wither newly advised listeners reveice some
	//or all the phases of the current notification, we need to allow the case
	//where its the first time and the starting phases is "later" than whats expected...
	if(pExpect->m_ePhase != ePhase && pExpect->m_fOptional)
	{
		//Get the next phase in sequence
		if(pExpect->m_ulTimesNotified == 1)
			pExpect->SetNextPhase();
	}

	//Verify the Notifcation
	QTESTC(pExpect->VerifyEqual(pListener, pIRowset, cRows, rghRows, cColumns, rgColumns, eReason, ePhase));

CLEANUP:
	return pExpect;
}


EXPECT* CExpectationsManager::PopExpectation
(	
	CImpIListener* pListener,	
	IRowset*     pIRowset,		
	DBCOUNTITEM		 cRows,
	HROW*		 rghRows,			
	DBORDINAL        cColumns,		
	DBORDINAL		 rgColumns[],   
	DBREASON     eReason,		
	DBEVENTPHASE ePhase
)
{

	ASSERT(pListener);
	ASSERT(pIRowset);
	DBCOUNTITEM i = 0;
	COMMAND* pCommand = pExecutionManager()->GetCurrentCommand();
    POSITION pos = pCommand->m_listExpect.GetHeadPosition();
	while(pos)
	{
		EXPECT* pExpect = pCommand->m_listExpect.GetNext(pos);
		if(pExpect->m_pListener != pListener)
			continue;
		if(pExpect->m_pIRowset != pIRowset)
			continue;
		if(pExpect->m_eReason != eReason)
			continue;
		
		//This is a hack for DB_S_ENDOFROWSET, since we don't neccessaryly
		//know ahead of time the number of rows in the rowset without traversing
		//the rowset, and traversing the rowset would fire ROW_ACTIVATE and the 
		//next call to GetNextRows would now not fire ROW_ACTIVE...
		if(pCommand->m_hrExpected == DB_S_ENDOFROWSET)
			pExpect->m_cRows = cRows;
		
		//Provider should send DBREASON_ROW_ACTIVATE  for newly fetched rows only, but may send far all rows
		if (eReason == DBREASON_ROW_ACTIVATE && pExpect->m_cRows > cRows)
		{
			//get only newly fetched rows
			DBCOUNTITEM cRowsExpect=0;

			for(i=0; i<pExpect->m_cRows; i++)
				if (pExpect->m_rghRows[i] == DB_NULL_HROW)
					cRowsExpect++;
			
			for(i=0; i<cRowsExpect; i++)
				pExpect->m_rghRows[i] = DB_NULL_HROW;
			pExpect->m_cRows = cRowsExpect;
		}

		if(pExpect->m_cRows != cRows)
			continue;

		BOOL bEqual = TRUE;
		for(i=0; i<cRows; i++)
			if(pExpect->m_rghRows[i] != DB_NULL_HROW)
				if(!IsSameRow(pIRowset, pExpect->m_rghRows[i], rghRows[i]))
					bEqual = FALSE;
		if(!bEqual)
			continue;

		if(pExpect->m_cColumns != cColumns)
			continue;
		
		bEqual = TRUE;
		for(i=0; i<cColumns; i++)
			if(pExpect->m_rgColumns[i] != rgColumns[i])
				bEqual = FALSE;
		if(!bEqual)
			continue;

		//Finally found the right expectation!
		return pExpect;
	}

	return NULL;
}


///////////////////////////////////////////////////////////////////////////////
// IRNOTIFY.Y
//
///////////////////////////////////////////////////////////////////////////////
/*
%{
#include "MODStandard.hpp" 	//ModuleCore headers
#include "IRNotify.h"		//IRowsetNotify header
#define lint  			//Removes warnings for unused labels

// Input string to parse 
LPCWSTR             yylpwzInput;
CExecutionManager *yypExecutionManager;
COMMANDLIST    *yypCommandList;
COMMANDLISTMAP *yypCommandListMap;
CONTROLMAP     *yypControlMap;
LISTENERMAP    *yypListenerMap;

static LPVOID         pvObjectId = NULL;
static CControl*      pControl = NULL;
static DBREASON       eReason = 0;
static DBEVENTPHASE   ePhase = 0;
static ULONG          ulCommandLevel = 1;
static ULONG          cRows = 0;
static ULONG          rgRowIds[MAX_ROW_COUNT];
static ULONG          cColumns = 0;
static ULONG          rgColumns[MAX_ROW_COUNT];
static HRESULT		  hrExpected = S_OK;
static ULONG		  cThreads = 0;
%}

%union {
	ULONG   Number;
	ULONG   IdentifierId;
	}

%token <Number>       yyNUMBER
%token <IdentifierId> yyIDENTIFIERID

//Notification REASONS
%token  yyROWSET_FETCHPOSITIONCHANGE
%token  yyROWSET_RELEASE
%token  yyROWSET_CHANGED
%token  yyCOLUMN_SET
%token  yyCOLUMN_RECALCULATED
%token  yyROW_ACTIVATE
%token  yyROW_RELEASE
%token  yyROW_DELETE
%token  yyROW_FIRSTCHANGE
%token  yyROW_INSERT
%token  yyROW_RESYNCH
%token  yyROW_UNDOCHANGE
%token  yyROW_UNDOINSERT
%token  yyROW_UNDODELETE
%token  yyROW_UPDATE

//Notification PHASES
%token  yyOKTODO
%token  yyABOUTTODO
%token  yySYNCHAFTER
%token  yyDIDEVENT
%token  yyFAILEDTODO

//OLEDB Methods (which generate Notifications)
%token  yyGETDATA
%token  yyGETORIGINALDATA
%token  yyGETVISIBLEDATA
%token  yyRELEASEROWSET
%token  yyADDREFROWSET
%token  yyGETNEXTROWS
%token  yyRELEASEROWS
%token  yyADDREFROWS
%token  yyRESTARTPOSITION
%token  yyDELETEROWS
%token  yyINSERTROW
%token  yySETDATA
%token  yySEEK
%token  yyGETROWSAT
%token  yyGETROWSBYBOOKMARK
%token  yyRESYNCHROWS
%token  yyGETROWSATRATIO
%token  yyUNDO
%token  yyUPDATE

%token  yyABORT
%token  yyCOMMIT
%token  yyADDCOLUMN
%token  yyDROPCOLUMN
%token  yyGETCOLUMNINFO

//Identifiers
%token  yyCONTROL
%token  yyLISTENER
%token  yyTHREAD

%token  yyDBBMK_FIRST
%token  yyDBBMK_LAST

%token  yyRETURN_ACCEPT
%token  yyRETURN_VETO
%token  yyRETURN_EFAIL
%token  yyRETURN_EOUTOFMEMORY
%token  yyRETURN_EINVALIDARG
%token  yyRETURN_UNWANTEDREASON
%token  yyRETURN_UNWANTEDPHASE
%token  yyRETURN_ADVISE
%token  yyRETURN_UNADVISE

//hrExpected
%token  yyDB_S_ENDOFROWSET
%token  yyDB_E_BADSTARTPOSITION
%token  yyDB_E_NOTREENTRANT
%token  yyDB_S_ERRORSOCCURRED
%token  yyDB_E_ERRORSOCCURRED
%token  yyE_INVALIDARG
%token  yyDB_E_ROWSNOTRELEASED
%token  yyDB_E_BADACCESSORHANDLE
%token  yyDB_E_BADROWHANDLE
%%

Notification_Test : {ulCommandLevel = 1; }
					'{' ListenerList  ControlList '}'

ListenerList : Listener | ListenerList Listener

//Listener Syntax
Listener : yyLISTENER yyIDENTIFIERID ';'
		{yypListenerMap->SetAt($2,new CImpIListener(yypExecutionManager));}

ControlList : Control | ControlList Control

Control : '{' yyCONTROL yyIDENTIFIERID ';'
				{
					pControl = new CControl(yypExecutionManager);
					pvObjectId = pControl;
					ulCommandLevel = 1;
					yypControlMap->SetAt($3,pControl);
					yypCommandList = new COMMANDLIST;
					yypCommandListMap->SetAt(pControl->GetThreadId(), yypCommandList);
				}
				'{' ControlStmtList '}' 
		  '}'

ControlStmtList : ControlStmt | ControlStmtList ControlStmt

ControlStmt : OLEDBMethod '{' ListenerStmtList '}' 
				{
					ulCommandLevel++; 
					pvObjectId = pControl; 
				}
			| OLEDBMethod '{' '}' 
				{
					ulCommandLevel++; 
					pvObjectId = pControl; 
				}

ListenerStmtList :  ListenerStmt | ListenerStmtList ListenerStmt

ListenerStmt	:	ListenerIdentifier '.' Reason '.' Phase '.' ListenerMethod

ListenerIdentifier : yyIDENTIFIERID
						{CImpIListener *pImpIListener;
						 BOOL Ok = yypListenerMap->Lookup($1,pImpIListener);
						 if (!Ok) yyerror("Invalid Listener");
						 pvObjectId = pImpIListener;}


ListenerMethod	: ListenerReturn
					{
					}
				 | OLEDBMethod '{' ListenerStmtList '}' 
					{
					}
				 | OLEDBMethod '{' '}' 
					{
					}
				 | yyTHREAD { cThreads = 1; } '{' ListenerMethod '}'
					{
						cThreads = 0;
					}
					

ListenerReturn : yyRETURN_ACCEPT
					{yypCommandList->AddTail(
					new COMMAND(RETURN_ACCEPT,			//eCOMMANDTYPE
								pvObjectId,				//pvObjectId
								ulCommandLevel, 		//ulCommandLevel
								cThreads,				//cThreads
								eReason,				//eReason
								ePhase));}				//ePhase

				 | yyRETURN_VETO
					{yypCommandList->AddTail(
					new COMMAND(RETURN_VETO,			//eCOMMANDTYPE
								pvObjectId,				//pvObjectId
								ulCommandLevel, 		//ulCommandLevel
								cThreads,				//cThreads
								eReason,				//eReason
								ePhase));}				//ePhase
								
				 | yyRETURN_EFAIL
					{yypCommandList->AddTail(
					new COMMAND(RETURN_EFAIL,			//eCOMMANDTYPE
								pvObjectId,				//pvObjectId
								ulCommandLevel, 		//ulCommandLevel
								cThreads,				//cThreads
								eReason,				//eReason
								ePhase));}				//ePhase

				 | yyRETURN_EOUTOFMEMORY
					{yypCommandList->AddTail(
					new COMMAND(RETURN_EOUTOFMEMORY,	//eCOMMANDTYPE
								pvObjectId,				//pvObjectId
								ulCommandLevel, 		//ulCommandLevel
								cThreads,				//cThreads
								eReason,				//eReason
								ePhase));}				//ePhase

				 | yyRETURN_EINVALIDARG
					{yypCommandList->AddTail(
					new COMMAND(RETURN_EINVALIDARG,		//eCOMMANDTYPE
								pvObjectId,				//pvObjectId
								ulCommandLevel, 		//ulCommandLevel
								cThreads,				//cThreads
								eReason,				//eReason
								ePhase));}				//ePhase

				| yyRETURN_UNWANTEDREASON
					{yypCommandList->AddTail(
					new COMMAND(RETURN_UNWANTEDREASON,	//eCOMMANDTYPE
								pvObjectId,				//pvObjectId
								ulCommandLevel, 		//ulCommandLevel
								cThreads,				//cThreads
								eReason,				//eReason
								ePhase));}				//ePhase

				|yyRETURN_UNWANTEDPHASE
					{yypCommandList->AddTail(
					new COMMAND(RETURN_UNWANTEDPHASE,	//eCOMMANDTYPE
								pvObjectId,				//pvObjectId
								ulCommandLevel, 		//ulCommandLevel
								cThreads,				//cThreads
								eReason,				//eReason
								ePhase));}				//ePhase

				|yyRETURN_ADVISE
					{yypCommandList->AddTail(
					new COMMAND(RETURN_ADVISE,			//eCOMMANDTYPE
								pvObjectId,				//pvObjectId
								ulCommandLevel, 		//ulCommandLevel
								cThreads,				//cThreads
								eReason,				//eReason
								ePhase));}				//ePhase

				|yyRETURN_UNADVISE
					{yypCommandList->AddTail(
					new COMMAND(RETURN_UNADVISE,		//eCOMMANDTYPE
								pvObjectId,				//pvObjectId
								ulCommandLevel, 		//ulCommandLevel
								cThreads,				//cThreads
								eReason,				//eReason
								ePhase));}				//ePhase

OLEDBMethod :   ReleaseRowsetMethod
			  | GetNextRows
			  | ReleaseRowsMethod
			  | RestartPositionMethod
			  | DeleteRowsMethod
			  | InsertRowMethod
			  | SetDataMethod
			  | SeekMethod
			  | GetRowsAtMethod
			  | GetRowsByBookmarkMethod
			  | ResynchRowsMethod
			  | GetRowsAtRatioMethod
			  | UndoMethod
			  | UpdateMethod
			  | AddRefRowsetMethod
			  | AddRefRowsMethod
			  | GetDataMethod
			  | GetOriginalDataMethod
			  | GetVisibleDataMethod
			  | AbortMethod
			  | CommitMethod
			  | AddColumnMethod
			  | DropColumnMethod
			  | GetColumnInfoMethod


ReleaseRowsMethod  : yyRELEASEROWS  '(' IdentifierList ')'
				{yypCommandList->AddTail(
				new COMMAND(RELEASEROWS,		//eCOMMANDTYPE
							pvObjectId,			//pvObjectId
							ulCommandLevel, 	//ulCommandLevel
							cThreads,				//cThreads
							eReason,			//eReason
							ePhase,				//ePhase
							hrExpected,			//hrExpected
							cRows,				//cRows
							rgRowIds));}		//rgRowIds				

GetNextRows :  yyGETNEXTROWS '(' IdentifierList ')'
				{yypCommandList->AddTail(
				new COMMAND(GETNEXTROWS,		//eCOMMANDTYPE
							pvObjectId,			//pvObjectId
							ulCommandLevel, 	//ulCommandLevel
							cThreads,				//cThreads
							eReason,			//eReason
							ePhase,				//ePhase
							hrExpected,			//hrExpected
							cRows,				//cRows
							rgRowIds));}		//rgRowIds				

ReleaseRowsetMethod  : yyRELEASEROWSET  '(' EmptyList ')'
				{yypCommandList->AddTail(
				new COMMAND(RELEASEROWSET,		//eCOMMANDTYPE
							pvObjectId,			//pvObjectId
							ulCommandLevel, 	//ulCommandLevel
							cThreads,				//cThreads
							eReason,			//eReason
							ePhase,				//ePhase
							hrExpected));}		//hrExpected				

RestartPositionMethod :  yyRESTARTPOSITION '(' EmptyList ')'
				{yypCommandList->AddTail(
				new COMMAND(RESTARTPOSITION,	//eCOMMANDTYPE
							pvObjectId,			//pvObjectId
							ulCommandLevel, 	//ulCommandLevel
							cThreads,				//cThreads
							eReason,			//eReason
							ePhase,				//ePhase
							hrExpected));}		//hrExpected				

DeleteRowsMethod : yyDELETEROWS  '(' IdentifierList ')'
				{yypCommandList->AddTail(
				new COMMAND(DELETEROWS,			//eCOMMANDTYPE
							pvObjectId,			//pvObjectId
							ulCommandLevel, 	//ulCommandLevel
							cThreads,				//cThreads
							eReason,			//eReason
							ePhase,				//ePhase
							hrExpected,			//hrExpected
							cRows,				//cRows
							rgRowIds));}		//rgRowIds				

InsertRowMethod : yyINSERTROW '(' yyIDENTIFIERID ')'
				{yypCommandList->AddTail(
				new COMMAND(INSERTROW,			//eCOMMANDTYPE
							pvObjectId,			//pvObjectId
							ulCommandLevel, 	//ulCommandLevel
							cThreads,				//cThreads
							eReason,			//eReason
							ePhase,				//ePhase
							S_OK,				//hrExpected
							1,					//cRows
							&($3)));}			//rgRowIds

				| yyINSERTROW '(' yyIDENTIFIERID ',' ExpectedHRESULT ')'
				{yypCommandList->AddTail(
				new COMMAND(INSERTROW,			//eCOMMANDTYPE
							pvObjectId,			//pvObjectId
							ulCommandLevel, 	//ulCommandLevel
							cThreads,				//cThreads
							eReason,			//eReason
							ePhase,				//ePhase
							hrExpected,			//hrExpected
							1,					//cRows
							&($3)));}			//rgRowIds

SetDataMethod : yySETDATA  '(' yyIDENTIFIERID ',' OrdinalList ')'
				{yypCommandList->AddTail(
				new COMMAND(SETDATA,			//eCOMMANDTYPE
							pvObjectId,			//pvObjectId
							ulCommandLevel, 	//ulCommandLevel
							cThreads,				//cThreads
							eReason,			//eReason
							ePhase,				//ePhase
							S_OK,				//hrExpected
							1,					//cRows
							&($3),				//rgRowIds
							cColumns,			//cColumns
							rgColumns));}		//rgColumns

				| yySETDATA  '(' yyIDENTIFIERID ',' OrdinalList ',' ExpectedHRESULT ')'
				{yypCommandList->AddTail(
				new COMMAND(SETDATA,			//eCOMMANDTYPE
							pvObjectId,			//pvObjectId
							ulCommandLevel, 	//ulCommandLevel
							cThreads,				//cThreads
							eReason,			//eReason
							ePhase,				//ePhase
							hrExpected,			//hrExpected
							1,					//cRows
							&($3),				//rgRowIds
							cColumns,			//cColumns
							rgColumns));}		//rgColumns

SeekMethod  : yySEEK  '(' yyNUMBER ')'
				{yypCommandList->AddTail(
				new COMMAND(SEEK,				//eCOMMANDTYPE
							pvObjectId,			//pvObjectId
							ulCommandLevel, 	//ulCommandLevel
							cThreads,				//cThreads
							eReason,			//eReason
							ePhase,				//ePhase
							hrExpected,			//hrExpected
							$3,					//cRows
							rgRowIds));}		//rgRowIds

			|  yySEEK  '(' yyNUMBER ',' ExpectedHRESULT ')'
				{yypCommandList->AddTail(
				new COMMAND(SEEK,				//eCOMMANDTYPE
							pvObjectId,			//pvObjectId
							ulCommandLevel, 	//ulCommandLevel
							cThreads,				//cThreads
							eReason,			//eReason
							ePhase,				//ePhase
							hrExpected,			//hrExpected
							$3,					//cRows
							rgRowIds));}		//rgRowIds

GetRowsAtMethod :  yyGETROWSAT '(' yyDBBMK_FIRST ',' IdentifierList ')'
				{yypCommandList->AddTail(
				new COMMAND(GETROWSAT,			//eCOMMANDTYPE
							pvObjectId,			//pvObjectId
							ulCommandLevel, 	//ulCommandLevel
							cThreads,				//cThreads
							eReason,			//eReason
							ePhase,				//ePhase
							hrExpected,			//hrExpected
							cRows,				//cRows
							rgRowIds,			//rgRowIds
							0,					//cColumns
							NULL,				//rgColumns
							DBBMK_FIRST));}		//dwOption
		
				| yyGETROWSAT '(' yyDBBMK_FIRST ')'
				{yypCommandList->AddTail(
				new COMMAND(GETROWSAT,			//eCOMMANDTYPE
							pvObjectId,			//pvObjectId
							ulCommandLevel, 	//ulCommandLevel
							cThreads,				//cThreads
							eReason,			//eReason
							ePhase,				//ePhase
							S_OK,				//hrExpected
							0,					//cRows
							NULL,				//rgRowIds
							0,					//cColumns
							NULL,				//rgColumns
							DBBMK_FIRST));}		//dwOption

				| yyGETROWSAT '(' yyDBBMK_LAST ',' IdentifierList ')'
				{yypCommandList->AddTail(
				new COMMAND(GETROWSAT,			//eCOMMANDTYPE
							pvObjectId,			//pvObjectId
							ulCommandLevel, 	//ulCommandLevel
							cThreads,				//cThreads
							eReason,			//eReason
							ePhase,				//ePhase
							hrExpected,			//hrExpected
							cRows,				//cRows
							rgRowIds,			//rgRowIds
							0,					//cColumns
							NULL,				//rgColumns
							DBBMK_LAST));}		//dwOption
		
				| yyGETROWSAT '(' yyDBBMK_LAST ')'
				{yypCommandList->AddTail(
				new COMMAND(GETROWSAT,			//eCOMMANDTYPE
							pvObjectId,			//pvObjectId
							ulCommandLevel, 	//ulCommandLevel
							cThreads,				//cThreads
							eReason,			//eReason
							ePhase,				//ePhase
							hrExpected,			//hrExpected
							0,					//cRows
							NULL,				//rgRowIds
							0,					//cColumns
							NULL,				//rgColumns
							DBBMK_LAST));}		//dwOption



GetRowsByBookmarkMethod :  yyGETROWSBYBOOKMARK '(' IdentifierList ')'
				{yypCommandList->AddTail(
				new COMMAND(GETROWSBYBOOKMARK,	//eCOMMANDTYPE
							pvObjectId,			//pvObjectId
							ulCommandLevel, 	//ulCommandLevel
							cThreads,				//cThreads
							eReason,			//eReason
							ePhase,				//ePhase
							hrExpected,			//hrExpected
							cRows,				//cRows
							rgRowIds));}		//rgRowIds				

ResynchRowsMethod : yyRESYNCHROWS  '('  IdentifierList ')'
				{yypCommandList->AddTail(
				new COMMAND(RESYNCHROWS,		//eCOMMANDTYPE
							pvObjectId,			//pvObjectId
							ulCommandLevel, 	//ulCommandLevel
							cThreads,				//cThreads
							eReason,			//eReason
							ePhase,				//ePhase
							hrExpected,			//hrExpected
							cRows,				//cRows
							rgRowIds));}		//rgRowIds				

GetRowsAtRatioMethod :  yyGETROWSATRATIO '(' yyNUMBER ')'
				{yypCommandList->AddTail(
				new COMMAND(GETROWSATRATIO,		//eCOMMANDTYPE
							pvObjectId,			//pvObjectId
							ulCommandLevel, 	//ulCommandLevel
							cThreads,				//cThreads
							eReason,			//eReason
							ePhase,				//ePhase
							hrExpected,			//hrExpected
							0,					//cRows
							NULL,				//rgRowIds
							0,					//cColumns
							NULL,				//rgColumns
							$3));}				//dwOption
				|	yyGETROWSATRATIO '(' yyNUMBER ',' IdentifierList ')'
				{yypCommandList->AddTail(
				new COMMAND(GETROWSATRATIO,		//eCOMMANDTYPE
							pvObjectId,			//pvObjectId
							ulCommandLevel, 	//ulCommandLevel
							cThreads,				//cThreads
							eReason,			//eReason
							ePhase,				//ePhase
							hrExpected,			//hrExpected
							cRows,				//cRows
							rgRowIds,			//rgRowIds
							0,					//cColumns
							NULL,				//rgColumns
							$3));}				//dwOption

UndoMethod  : yyUNDO  '('  IdentifierList ')'
				{yypCommandList->AddTail(
				new COMMAND(UNDO,				//eCOMMANDTYPE
							pvObjectId,			//pvObjectId
							ulCommandLevel, 	//ulCommandLevel
							cThreads,				//cThreads
							eReason,			//eReason
							ePhase,				//ePhase
							hrExpected,			//hrExpected
							cRows,				//cRows
							rgRowIds));}		//rgRowIds				

UpdateMethod : yyUPDATE  '('  IdentifierList ')'
				{yypCommandList->AddTail(
				new COMMAND(UPDATE,				//eCOMMANDTYPE
							pvObjectId,			//pvObjectId
							ulCommandLevel, 	//ulCommandLevel
							cThreads,				//cThreads
							eReason,			//eReason
							ePhase,				//ePhase
							hrExpected,			//hrExpected
							cRows,				//cRows
							rgRowIds));}		//rgRowIds				

AddRefRowsMethod  : yyADDREFROWS  '(' IdentifierList ')'
				{yypCommandList->AddTail(
				new COMMAND(ADDREFROWS,			//eCOMMANDTYPE
							pvObjectId,			//pvObjectId
							ulCommandLevel, 	//ulCommandLevel
							cThreads,				//cThreads
							eReason,			//eReason
							ePhase,				//ePhase
							hrExpected,			//hrExpected
							cRows,				//cRows
							rgRowIds));}		//rgRowIds				

AddRefRowsetMethod  : yyADDREFROWSET  '(' EmptyList ')'
				{yypCommandList->AddTail(
				new COMMAND(ADDREFROWSET,		//eCOMMANDTYPE
							pvObjectId,			//pvObjectId
							ulCommandLevel, 	//ulCommandLevel
							cThreads,				//cThreads
							eReason,			//eReason
							ePhase,				//ePhase
							hrExpected));}		//hrExpected				

GetDataMethod : yyGETDATA  '(' yyIDENTIFIERID ')'
				{yypCommandList->AddTail(
				new COMMAND(GETDATA,			//eCOMMANDTYPE
							pvObjectId,			//pvObjectId
							ulCommandLevel, 	//ulCommandLevel
							cThreads,				//cThreads
							eReason,			//eReason
							ePhase,				//ePhase
							S_OK,				//hrExpected
							1,					//cRows
							&($3) ));}			//rgRowIds				

GetOriginalDataMethod : yyGETORIGINALDATA  '(' yyIDENTIFIERID ')'
				{yypCommandList->AddTail(
				new COMMAND(GETORIGINALDATA,	//eCOMMANDTYPE
							pvObjectId,			//pvObjectId
							ulCommandLevel, 	//ulCommandLevel
							cThreads,				//cThreads
							eReason,			//eReason
							ePhase,				//ePhase
							S_OK,				//hrExpected
							1,					//cRows
							&($3) ));}			//rgRowIds				

				| yyGETORIGINALDATA  '(' yyIDENTIFIERID ',' ExpectedHRESULT ')'
				{yypCommandList->AddTail(
				new COMMAND(GETORIGINALDATA,			//eCOMMANDTYPE
							pvObjectId,			//pvObjectId
							ulCommandLevel, 	//ulCommandLevel
							cThreads,				//cThreads
							eReason,			//eReason
							ePhase,				//ePhase
							hrExpected,			//hrExpected
							1,					//cRows
							&($3)));}			//rgRowIds

GetVisibleDataMethod : yyGETVISIBLEDATA  '(' yyIDENTIFIERID ')'
				{yypCommandList->AddTail(
				new COMMAND(GETVISIBLEDATA,		//eCOMMANDTYPE
							pvObjectId,			//pvObjectId
							ulCommandLevel, 	//ulCommandLevel
							cThreads,				//cThreads
							eReason,			//eReason
							ePhase,				//ePhase
							S_OK,				//hrExpected
							1,					//cRows
							&($3) ));}			//rgRowIds				

				| yyGETVISIBLEDATA  '(' yyIDENTIFIERID ',' ExpectedHRESULT ')'
				{yypCommandList->AddTail(
				new COMMAND(GETVISIBLEDATA,			//eCOMMANDTYPE
							pvObjectId,			//pvObjectId
							ulCommandLevel, 	//ulCommandLevel
							cThreads,				//cThreads
							eReason,			//eReason
							ePhase,				//ePhase
							hrExpected,			//hrExpected
							1,					//cRows
							&($3)));}			//rgRowIds

AbortMethod			: yyABORT  '(' EmptyList ')'
				{yypCommandList->AddTail(
				new COMMAND(ABORT,				//eCOMMANDTYPE
							pvObjectId,			//pvObjectId
							ulCommandLevel, 	//ulCommandLevel
							cThreads,				//cThreads
							eReason,			//eReason
							ePhase,				//ePhase
							hrExpected));}		//hrExpected				

CommitMethod		: yyCOMMIT  '(' EmptyList ')'
				{yypCommandList->AddTail(
				new COMMAND(COMMIT,				//eCOMMANDTYPE
							pvObjectId,			//pvObjectId
							ulCommandLevel, 	//ulCommandLevel
							cThreads,				//cThreads
							eReason,			//eReason
							ePhase,				//ePhase
							hrExpected));}		//hrExpected				

AddColumnMethod	: yyADDCOLUMN  '(' EmptyList ')'
				{yypCommandList->AddTail(
				new COMMAND(ADDCOLUMN,			//eCOMMANDTYPE
							pvObjectId,			//pvObjectId
							ulCommandLevel, 	//ulCommandLevel
							cThreads,				//cThreads
							eReason,			//eReason
							ePhase,				//ePhase
							hrExpected));}		//hrExpected				

DropColumnMethod	: yyDROPCOLUMN  '(' EmptyList ')'
				{yypCommandList->AddTail(
				new COMMAND(DROPCOLUMN,			//eCOMMANDTYPE
							pvObjectId,			//pvObjectId
							ulCommandLevel, 	//ulCommandLevel
							cThreads,				//cThreads
							eReason,			//eReason
							ePhase,				//ePhase
							hrExpected));}		//hrExpected				

GetColumnInfoMethod	: yyGETCOLUMNINFO  '(' EmptyList ')'
				{yypCommandList->AddTail(
				new COMMAND(GETCOLUMNINFO,		//eCOMMANDTYPE
							pvObjectId,			//pvObjectId
							ulCommandLevel, 	//ulCommandLevel
							cThreads,				//cThreads
							eReason,			//eReason
							ePhase,				//ePhase
							hrExpected));}		//hrExpected

OrdinalList : yyNUMBER
				{cColumns = 0; rgColumns[cColumns++] = $1;}
			 | OrdinalList ',' yyNUMBER
				{rgColumns[cColumns++] = $3;}


IdentifierList : 
			yyIDENTIFIERID
				{
					cRows = 0; 
					rgRowIds[cRows++] = $1;
					hrExpected = S_OK;
				}
			| EmptyList
				{
				}
			| IdentifierList ',' yyIDENTIFIERID
				{
					rgRowIds[cRows++] = $3;
				}
			| IdentifierList ',' ExpectedHRESULT
				{
				}


EmptyList : ExpectedHRESULT
				{ 
					cRows = 0;
					cColumns = 0;
				}
			 | // Empty
				{	
					cRows = 0;
					cColumns = 0;
					hrExpected = S_OK;
				}

ExpectedHRESULT :
			yyDB_S_ENDOFROWSET
				{
					cRows = SIZEOF_TABLE;
					hrExpected = DB_S_ENDOFROWSET;
				}
			| yyDB_E_BADSTARTPOSITION
				{
					hrExpected = DB_E_BADSTARTPOSITION;
				}
			| yyDB_E_NOTREENTRANT
				{
					hrExpected = DB_E_NOTREENTRANT;
				}
			| yyDB_S_ERRORSOCCURRED
				{
					hrExpected = DB_S_ERRORSOCCURRED;
				}
			| yyDB_E_ERRORSOCCURRED
				{
					hrExpected = DB_E_ERRORSOCCURRED;
				}
			| yyE_INVALIDARG
				{
					hrExpected = E_INVALIDARG;
				}
			| yyDB_E_ROWSNOTRELEASED
				{
					hrExpected = DB_E_ROWSNOTRELEASED;
				}
			| yyDB_E_BADACCESSORHANDLE
				{
					hrExpected = DB_E_BADACCESSORHANDLE;
				}
			| yyDB_E_BADROWHANDLE
				{
					hrExpected = DB_E_BADROWHANDLE;
				}

Reason :   
		   yyROWSET_FETCHPOSITIONCHANGE
				{eReason = DBREASON_ROWSET_FETCHPOSITIONCHANGE;}
		 | yyROWSET_RELEASE
				{eReason = DBREASON_ROWSET_RELEASE;}
		 | yyROWSET_CHANGED
				{eReason = DBREASON_ROWSET_CHANGED;}
         | yyCOLUMN_SET
				{eReason = DBREASON_COLUMN_SET;}
         | yyCOLUMN_RECALCULATED
				{eReason = DBREASON_COLUMN_RECALCULATED;}
	     | yyROW_ACTIVATE
				{eReason = DBREASON_ROW_ACTIVATE;}
	     | yyROW_RELEASE
				{eReason = DBREASON_ROW_RELEASE;}
         | yyROW_DELETE
				{eReason = DBREASON_ROW_DELETE;}
         | yyROW_FIRSTCHANGE
				{eReason = DBREASON_ROW_FIRSTCHANGE;}
         | yyROW_INSERT
				{eReason = DBREASON_ROW_INSERT;}
		 | yyROW_RESYNCH
				{eReason = DBREASON_ROW_RESYNCH;}
		 | yyROW_UNDOCHANGE
				{eReason = DBREASON_ROW_UNDOCHANGE;}
         | yyROW_UNDOINSERT
				{eReason = DBREASON_ROW_UNDOINSERT;}
         | yyROW_UNDODELETE
				{eReason = DBREASON_ROW_UNDODELETE;}
         | yyROW_UPDATE
				{eReason = DBREASON_ROW_UPDATE;}


Phase    : yyOKTODO
				{ePhase = DBEVENTPHASE_OKTODO;}
         | yyABOUTTODO
				{ePhase = DBEVENTPHASE_ABOUTTODO;}
         | yySYNCHAFTER
				{ePhase = DBEVENTPHASE_SYNCHAFTER;}
         | yyDIDEVENT
				{ePhase = DBEVENTPHASE_DIDEVENT;}
         | yyFAILEDTODO
				{ePhase = DBEVENTPHASE_FAILEDTODO;}


%%



int yyerror(char*s)
{
	return 0;
}

// Keyword Table for Parser
typedef CMap <CString, LPCTSTR, ULONG, ULONG> TABLE;
class KEYWORDTABLE : public TABLE
	{
	public:
		KEYWORDTABLE();
		~KEYWORDTABLE();
	};

KEYWORDTABLE::KEYWORDTABLE()
	{
		SetAt(_T("IRowset::GetData"),yyGETDATA);
		SetAt(_T("IRowsetUpdate::GetOriginalData"),yyGETORIGINALDATA);
		SetAt(_T("IRowsetResynch::GetVisibleData"),yyGETVISIBLEDATA);

		SetAt(_T("IRowsetChange::SetData"),yySETDATA);
		SetAt(_T("IRowsetIndex::Seek"),yySEEK);
		SetAt(_T("IRowsetChange::DeleteRows"),yyDELETEROWS);
		SetAt(_T("IRowsetChange::InsertRow"),yyINSERTROW);
		SetAt(_T("IRowsetUpdate::Update"),yyUPDATE);
		SetAt(_T("IRowsetUpdate::Undo"),yyUNDO);
		SetAt(_T("IRowset::GetNextRows"),yyGETNEXTROWS);
		SetAt(_T("IRowset::ReleaseRows"),yyRELEASEROWS);
		SetAt(_T("IRowset::AddRefRows"),yyADDREFROWS);
		SetAt(_T("IRowset::AddRef"),yyADDREFROWSET);
		SetAt(_T("IRowset::Release"),yyRELEASEROWSET);
		SetAt(_T("IRowsetResynch::ResynchRows"),yyRESYNCHROWS);
		SetAt(_T("IRowsetLocate::GetRowsAt"),yyGETROWSAT);
		SetAt(_T("IRowsetScroll::GetRowsAtRatio"),yyGETROWSATRATIO);
		SetAt(_T("IRowset::RestartPosition"),yyRESTARTPOSITION);
		SetAt(_T("IRowsetLocate::GetRowsByBookmark"),yyGETROWSBYBOOKMARK);

		SetAt(_T("ITransaction::Abort"),	yyABORT);
		SetAt(_T("ITransaction::Commit"),	yyCOMMIT);

		SetAt(_T("ITableDefinition::AddColumn"),	yyADDCOLUMN);
		SetAt(_T("ITableDefinition::DropColumn"),	yyDROPCOLUMN);
		SetAt(_T("IColumnsInfo::GetColumnInfo"),	yyGETCOLUMNINFO);

//Listener Return values
		SetAt(_T("RETURN_ACCEPT"),yyRETURN_ACCEPT);
		SetAt(_T("RETURN_VETO"),yyRETURN_VETO);
		SetAt(_T("RETURN_EFAIL"),yyRETURN_EFAIL);
		SetAt(_T("RETURN_EOUTOFMEMORY"),yyRETURN_EOUTOFMEMORY);
		SetAt(_T("RETURN_EINVALIDARG"),yyRETURN_EINVALIDARG);
		SetAt(_T("RETURN_UNWANTEDREASON"),yyRETURN_UNWANTEDREASON);
		SetAt(_T("RETURN_UNWANTEDPHASE"),yyRETURN_UNWANTEDPHASE);
		SetAt(_T("RETURN_ADVISE"),yyRETURN_ADVISE);
		SetAt(_T("RETURN_UNADVISE"),yyRETURN_UNADVISE);

		SetAt(_T("Control"),yyCONTROL);
		SetAt(_T("Listener"),yyLISTENER);
		SetAt(_T("Thread"),yyTHREAD);
		SetAt(_T("COLUMN_SET"),yyCOLUMN_SET);
		SetAt(_T("ROW_DELETE"),yyROW_DELETE);
		SetAt(_T("ROW_INSERT"),yyROW_INSERT);
		SetAt(_T("ROW_FIRSTCHANGE"),yyROW_FIRSTCHANGE);
		SetAt(_T("ROW_ACTIVATE"),yyROW_ACTIVATE);
		SetAt(_T("ROW_RELEASE"),yyROW_RELEASE);
		SetAt(_T("ROW_UNDOCHANGE"),yyROW_UNDOCHANGE);
		SetAt(_T("ROW_UNDOINSERT"),yyROW_UNDOINSERT);
		SetAt(_T("ROW_UNDODELETE"),yyROW_UNDODELETE);
		SetAt(_T("ROW_UPDATE"),yyROW_UPDATE);
		SetAt(_T("ROW_RESYNCH"),yyROW_RESYNCH);
		SetAt(_T("ROWSET_RELEASE"),yyROWSET_RELEASE);
		SetAt(_T("ROWSET_FETCHPOSITIONCHANGE"), yyROWSET_FETCHPOSITIONCHANGE);
		SetAt(_T("ROWSET_CHANGED"), yyROWSET_CHANGED);
		SetAt(_T("COLUMN_RECALCULATED"), yyCOLUMN_RECALCULATED);
		SetAt(_T("DBBMK_FIRST"),yyDBBMK_FIRST);
		SetAt(_T("DBBMK_LAST"),yyDBBMK_LAST);

		SetAt(_T("OKTODO"),yyOKTODO);
		SetAt(_T("ABOUTTODO"),yyABOUTTODO);
		SetAt(_T("SYNCHAFTER"),yySYNCHAFTER);
		SetAt(_T("DIDEVENT"),yyDIDEVENT);
		SetAt(_T("FAILEDTODO"),yyFAILEDTODO);

//HrExpected
		SetAt(_T("DB_S_ENDOFROWSET"),yyDB_S_ENDOFROWSET);
		SetAt(_T("DB_E_BADSTARTPOSITION"),yyDB_E_BADSTARTPOSITION);
		SetAt(_T("DB_E_NOTREENTRANT"),yyDB_E_NOTREENTRANT);
		SetAt(_T("DB_S_ERRORSOCCURRED"),yyDB_S_ERRORSOCCURRED);
		SetAt(_T("DB_E_ERRORSOCCURRED"),yyDB_E_ERRORSOCCURRED);
		SetAt(_T("E_INVALIDARG"),yyE_INVALIDARG);
		SetAt(_T("DB_E_ROWSNOTRELEASED"),yyDB_E_ROWSNOTRELEASED);
		SetAt(_T("DB_E_BADACCESSORHANDLE"),yyDB_E_BADACCESSORHANDLE);
		SetAt(_T("DB_E_BADROWHANDLE"),yyDB_E_BADROWHANDLE);
	}

KEYWORDTABLE::~KEYWORDTABLE()
	{
	RemoveAll();
	}

// Keyword Table
KEYWORDTABLE KeywordTable;
// Identifier Table
TABLE        IdentifierTable;


WCHAR  LexBuf[100];
WCHAR *LexPtr;
ULONG  TokenId;
ULONG  IdentifierId = 1;

int yylexInit()
	{
	IdentifierId = 1;
	IdentifierTable.RemoveAll();
	return TRUE;
	}

int yylex()
	{
start:
    // Get next char from input string
	WCHAR wcChar = *yylpwzInput++;
	// Check for end of input string
	if (wcChar == L'\0') 
		{
		IdentifierId = 1;
		IdentifierTable.RemoveAll();
		return 0;
		}

	if (iswspace(wcChar)) goto start;
	// Check for special char and return it 
	if (!iswalpha(wcChar) && wcChar != L'_' && !iswdigit(wcChar))
		{
		char AnsiBuf[3];
		WideCharToMultiByte(CP_ACP,0,LPCWSTR(&wcChar),1,AnsiBuf,sizeof(AnsiBuf),NULL,NULL);
		return AnsiBuf[0];
		}

	// Check for digit and collect it
	if (iswdigit(wcChar))
		{
		LexPtr = LexBuf;
		*LexPtr++ = wcChar;
		while (iswdigit(*yylpwzInput)) *LexPtr++ = *yylpwzInput++;
		*LexPtr = L'\0';
		yylval.Number = _wtol(LexBuf);
		return yyNUMBER;
		}

	// Collect Identifier or Keyword
	LexPtr = LexBuf;
	*LexPtr++ = wcChar;
	while (iswalnum(*yylpwzInput)|| *yylpwzInput == L'_' || *yylpwzInput == L':') 
		*LexPtr++ = *yylpwzInput++;
	*LexPtr = L'\0';
	TCHAR tszLexBuf[200];
#ifndef _UNICODE	
	WideCharToMultiByte(CP_ACP,0,LexBuf,-1,tszLexBuf,200,NULL,NULL);
#else
	wcscpy(tszLexBuf, LexBuf);
#endif //_UNICODE

	// If Keyword, return Token Id else return Identifier Id 
	if (KeywordTable.Lookup(tszLexBuf,TokenId))
		return TokenId;
	else
		{
		if (!IdentifierTable.Lookup(tszLexBuf,yylval.IdentifierId))
			{
			yylval.IdentifierId = IdentifierId++;
			IdentifierTable.SetAt(tszLexBuf,yylval.IdentifierId);
			}
		return yyIDENTIFIERID;
		}
	}
*/

///////////////////////////////////////////////////////////////////////////////
// Y_TAB.C
//
///////////////////////////////////////////////////////////////////////////////
#ifndef lint
static char yysccsid[] = "@(#)yaccpar	1.9 (Berkeley) 02/21/93";
#endif
#define YYBYACC 1
#define YYMAJOR 1
#define YYMINOR 9
#define yyclearin (yychar=(-1))
#define yyerrok (yyerrflag=0)
#define YYRECOVERING (yyerrflag!=0)
#define YYPREFIX "yy"
#include "MODStandard.hpp" 	/*ModuleCore headers*/
#include "IRNotify.h"		/*IRowsetNotify header*/
#define lint  			/*Removes warnings for unused labels*/

/* Input string to parse */
LPCWSTR             yylpwzInput;
CExecutionManager *yypExecutionManager;
COMMANDLIST    *yypCommandList;
COMMANDLISTMAP *yypCommandListMap;
CONTROLMAP     *yypControlMap;
LISTENERMAP    *yypListenerMap;

static LPVOID         pvObjectId = NULL;
static CControl*      pControl = NULL;
static DBREASON       eReason = 0;
static DBEVENTPHASE   ePhase = 0;
static ULONG          ulCommandLevel = 1;
static ULONG          cRows = 0;
static ULONG          rgRowIds[MAX_ROW_COUNT];
static ULONG          cColumns = 0;
static ULONG          rgColumns[MAX_ROW_COUNT];
static HRESULT		  hrExpected = S_OK;
static ULONG		  cThreads = 0;
typedef union {
	ULONG   Number;
	ULONG   IdentifierId;
	} YYSTYPE;
#define yyNUMBER 257
#define yyIDENTIFIERID 258
#define yyROWSET_FETCHPOSITIONCHANGE 259
#define yyROWSET_RELEASE 260
#define yyROWSET_CHANGED 261
#define yyCOLUMN_SET 262
#define yyCOLUMN_RECALCULATED 263
#define yyROW_ACTIVATE 264
#define yyROW_RELEASE 265
#define yyROW_DELETE 266
#define yyROW_FIRSTCHANGE 267
#define yyROW_INSERT 268
#define yyROW_RESYNCH 269
#define yyROW_UNDOCHANGE 270
#define yyROW_UNDOINSERT 271
#define yyROW_UNDODELETE 272
#define yyROW_UPDATE 273
#define yyOKTODO 274
#define yyABOUTTODO 275
#define yySYNCHAFTER 276
#define yyDIDEVENT 277
#define yyFAILEDTODO 278
#define yyGETDATA 279
#define yyGETORIGINALDATA 280
#define yyGETVISIBLEDATA 281
#define yyRELEASEROWSET 282
#define yyADDREFROWSET 283
#define yyGETNEXTROWS 284
#define yyRELEASEROWS 285
#define yyADDREFROWS 286
#define yyRESTARTPOSITION 287
#define yyDELETEROWS 288
#define yyINSERTROW 289
#define yySETDATA 290
#define yySEEK 291
#define yyGETROWSAT 292
#define yyGETROWSBYBOOKMARK 293
#define yyRESYNCHROWS 294
#define yyGETROWSATRATIO 295
#define yyUNDO 296
#define yyUPDATE 297
#define yyABORT 298
#define yyCOMMIT 299
#define yyADDCOLUMN 300
#define yyDROPCOLUMN 301
#define yyGETCOLUMNINFO 302
#define yyCONTROL 303
#define yyLISTENER 304
#define yyTHREAD 305
#define yyDBBMK_FIRST 306
#define yyDBBMK_LAST 307
#define yyRETURN_ACCEPT 308
#define yyRETURN_VETO 309
#define yyRETURN_EFAIL 310
#define yyRETURN_EOUTOFMEMORY 311
#define yyRETURN_EINVALIDARG 312
#define yyRETURN_UNWANTEDREASON 313
#define yyRETURN_UNWANTEDPHASE 314
#define yyRETURN_ADVISE 315
#define yyRETURN_UNADVISE 316
#define yyDB_S_ENDOFROWSET 317
#define yyDB_E_BADSTARTPOSITION 318
#define yyDB_E_NOTREENTRANT 319
#define yyDB_S_ERRORSOCCURRED 320
#define yyDB_E_ERRORSOCCURRED 321
#define yyE_INVALIDARG 322
#define yyDB_E_ROWSNOTRELEASED 323
#define yyDB_E_BADACCESSORHANDLE 324
#define yyDB_E_BADROWHANDLE 325
#define YYERRCODE 256
short yylhs[] = {                                        -1,
    1,    0,    2,    2,    4,    3,    3,    6,    5,    7,
    7,    8,    8,   10,   10,   11,   12,   15,   15,   15,
   17,   15,   16,   16,   16,   16,   16,   16,   16,   16,
   16,    9,    9,    9,    9,    9,    9,    9,    9,    9,
    9,    9,    9,    9,    9,    9,    9,    9,    9,    9,
    9,    9,    9,    9,    9,   20,   19,   18,   21,   22,
   23,   23,   24,   24,   25,   25,   26,   26,   26,   26,
   27,   28,   29,   29,   30,   31,   33,   32,   34,   35,
   35,   36,   36,   37,   38,   39,   40,   41,   45,   45,
   42,   42,   42,   42,   43,   43,   44,   44,   44,   44,
   44,   44,   44,   44,   44,   13,   13,   13,   13,   13,
   13,   13,   13,   13,   13,   13,   13,   13,   13,   13,
   14,   14,   14,   14,   14,
};
short yylen[] = {                                         2,
    0,    5,    1,    2,    3,    1,    2,    0,    9,    1,
    2,    4,    3,    1,    2,    7,    1,    1,    4,    3,
    0,    5,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    4,    4,    4,    4,    4,
    4,    6,    6,    8,    4,    6,    6,    4,    6,    4,
    4,    4,    4,    6,    4,    4,    4,    4,    4,    4,
    6,    4,    6,    4,    4,    4,    4,    4,    1,    3,
    1,    1,    3,    3,    1,    0,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,
};
short yydefred[] = {                                      1,
    0,    0,    0,    0,    0,    3,    0,    0,    0,    4,
    6,    5,    0,    2,    7,    0,    8,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,   10,    0,   32,   33,   34,   35,
   36,   37,   38,   39,   40,   41,   42,   43,   44,   45,
   46,   47,   48,   49,   50,   51,   52,   53,   54,   55,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,   11,    0,    0,    0,    0,
   97,   98,   99,  100,  101,  102,  103,  104,  105,    0,
   95,    0,   91,    0,   92,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    9,   17,   13,    0,   14,    0,
   79,   80,    0,   82,    0,   58,   78,   57,    0,   56,
   77,   59,   60,   61,    0,    0,   65,    0,   68,    0,
   70,    0,   71,   72,   73,    0,   75,   76,   84,   85,
   86,   87,   88,   12,   15,    0,    0,    0,   93,   94,
    0,   89,    0,    0,    0,    0,    0,  106,  107,  108,
  109,  110,  111,  112,  113,  114,  115,  116,  117,  118,
  119,  120,    0,   81,   83,   62,   63,    0,   66,   67,
   69,   74,    0,   90,    0,  121,  122,  123,  124,  125,
    0,   64,    0,   21,   23,   24,   25,   26,   27,   28,
   29,   30,   31,    0,   16,   18,    0,    0,    0,   20,
    0,    0,   19,   22,
};
short yydgoto[] = {                                       1,
    2,    5,    9,    6,   11,   18,   44,   45,   46,  138,
  139,  140,  203,  221,  235,  236,  237,   47,   48,   49,
   50,   51,   52,   53,   54,   55,   56,   57,   58,   59,
   60,   61,   62,   63,   64,   65,   66,   67,   68,   69,
   70,  114,  115,  111,  183,
};
short yysindex[] = {                                      0,
    0,  -87, -260, -210, -123,    0,  -10, -237,  -94,    0,
    0,    0, -180,    0,    0,   29,    0,  -34,  -59,  108,
  111,  112,  113,  114,  139,  140,  166,  167,  169,  170,
  204,  205,  206,  207,  208,  209,  210,  211,  212,  213,
  214,  215,  216, -124,    0,  -30,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
   -1,    3,    4, -262, -262, -238, -238, -238, -262, -238,
    5,    6,    8, -294, -238, -238,    9, -238, -238, -262,
 -262, -262, -262, -262,  133,    0, -122,  218,  -27,  -11,
    0,    0,    0,    0,    0,    0,    0,    0,    0,  219,
    0,  226,    0,   -4,    0,    1,    2,  227,   23,   24,
  225,   53,   54,   58,   59,   60,   89,   90,   91,  229,
  230,  231,  232,  233,    0,    0,    0, -121,    0,  234,
    0,    0, -262,    0, -262,    0,    0,    0, -211,    0,
    0,    0,    0,    0, -262,   18,    0, -262,    0, -238,
    0, -238,    0,    0,    0, -238,    0,    0,    0,    0,
    0,    0,    0,    0,    0, -144,  235,  236,    0,    0,
  237,    0,   99,  238,  100,  101,  105,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,  239,    0,    0,    0,    0, -248,    0,    0,
    0,    0, -253,    0,  240,    0,    0,    0,    0,    0,
  241,    0,  -97,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,  159,    0,    0,  160, -120,  -97,    0,
 -119,  161,    0,    0,
};
short yyrindex[] = {                                      0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,  243,  243,  106,  106,  106,  243,  106,
    0,    0,    0,    0,  106,  106,    0,  106,  106,  243,
  243,  243,  243,  243,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,  106,
    0,  106,    0,    0,    0,  106,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,
};
short yygindex[] = {                                      0,
    0,    0,    0,  283,  280,    0,    0,  246, -212,   55,
 -136,    0,    0,    0,   52,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,  -70,  -40, -117,    0,
};
#define YYTABLESIZE 293
short yytable[] = {                                       8,
   95,  175,  137,  174,  240,  243,  116,  117,  214,  119,
  234,  123,  124,  142,  125,  126,  143,  128,  129,  113,
  216,  217,  218,  219,  220,  177,  234,  178,    8,  144,
   14,  180,  145,  110,  112,    3,  148,  181,  118,  149,
  184,  150,  151,    4,  149,  149,  179,    7,   12,  130,
  131,  132,  133,  134,  101,  102,  103,  104,  105,  106,
  107,  108,  109,  153,  154,   13,  149,  155,  101,  102,
  103,  104,  105,  106,  107,  108,  109,   16,  101,  102,
  103,  104,  105,  106,  107,  108,  109,   17,   19,  185,
  215,  186,   97,  157,  159,  187,  158,  160,  161,  163,
  164,  162,  149,  149,  175,  101,  102,  103,  104,  105,
  106,  107,  108,  109,  188,  189,  190,  191,  192,  193,
  194,  195,  196,  197,  198,  199,  200,  201,  202,  165,
  167,  168,  166,  149,  149,  136,  136,  136,  136,  207,
  210,  211,  208,  149,  149,  212,   96,   71,  149,   96,
   72,   73,   74,   75,   20,   21,   22,   23,   24,   25,
   26,   27,   28,   29,   30,   31,   32,   33,   34,   35,
   36,   37,   38,   39,   40,   41,   42,   43,   76,   77,
    4,   20,   21,   22,   23,   24,   25,   26,   27,   28,
   29,   30,   31,   32,   33,   34,   35,   36,   37,   38,
   39,   40,   41,   42,   43,   78,   79,  224,   80,   81,
  225,  226,  227,  228,  229,  230,  231,  232,  233,   20,
   21,   22,   23,   24,   25,   26,   27,   28,   29,   30,
   31,   32,   33,   34,   35,   36,   37,   38,   39,   40,
   41,   42,   43,   82,   83,   84,   85,   86,   87,   88,
   89,   90,   91,   92,   93,   94,   98,  135,  141,  146,
   99,  100,  120,  121,  122,  127,  147,  152,  156,  169,
  170,  171,  172,  173,  182,  204,  205,  206,  209,  176,
  222,  238,  239,   96,  213,  244,  223,   10,   15,   96,
  242,    0,  241,
};
short yycheck[] = {                                     123,
  125,  138,  125,  125,  125,  125,   77,   78,  257,   80,
  223,  306,  307,   41,   85,   86,   44,   88,   89,  258,
  274,  275,  276,  277,  278,  143,  239,  145,  123,   41,
  125,  149,   44,   74,   75,  123,   41,  155,   79,   44,
  158,   41,   41,  304,   44,   44,  258,  258,   59,   90,
   91,   92,   93,   94,  317,  318,  319,  320,  321,  322,
  323,  324,  325,   41,   41,  303,   44,   44,  317,  318,
  319,  320,  321,  322,  323,  324,  325,  258,  317,  318,
  319,  320,  321,  322,  323,  324,  325,   59,  123,  160,
  208,  162,  123,   41,   41,  166,   44,   44,   41,   41,
   41,   44,   44,   44,  241,  317,  318,  319,  320,  321,
  322,  323,  324,  325,  259,  260,  261,  262,  263,  264,
  265,  266,  267,  268,  269,  270,  271,  272,  273,   41,
   41,   41,   44,   44,   44,  258,  258,  258,  258,   41,
   41,   41,   44,   44,   44,   41,   41,   40,   44,   44,
   40,   40,   40,   40,  279,  280,  281,  282,  283,  284,
  285,  286,  287,  288,  289,  290,  291,  292,  293,  294,
  295,  296,  297,  298,  299,  300,  301,  302,   40,   40,
  304,  279,  280,  281,  282,  283,  284,  285,  286,  287,
  288,  289,  290,  291,  292,  293,  294,  295,  296,  297,
  298,  299,  300,  301,  302,   40,   40,  305,   40,   40,
  308,  309,  310,  311,  312,  313,  314,  315,  316,  279,
  280,  281,  282,  283,  284,  285,  286,  287,  288,  289,
  290,  291,  292,  293,  294,  295,  296,  297,  298,  299,
  300,  301,  302,   40,   40,   40,   40,   40,   40,   40,
   40,   40,   40,   40,   40,   40,  258,  125,   41,   41,
  258,  258,  258,  258,  257,  257,   41,   41,   44,   41,
   41,   41,   41,   41,  257,   41,   41,   41,   41,   46,
   41,  123,  123,   41,   46,  125,   46,    5,    9,   44,
  239,   -1,  238,
};
#define YYFINAL 1
#ifndef YYDEBUG
#define YYDEBUG 0
#endif
#define YYMAXTOKEN 325
#if YYDEBUG
char *yyname[] = {
"end-of-file",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,"'('","')'",0,0,"','",0,"'.'",0,0,0,0,0,0,0,0,0,0,0,0,"';'",0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"'{'",0,"'}'",0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
"yyNUMBER","yyIDENTIFIERID","yyROWSET_FETCHPOSITIONCHANGE","yyROWSET_RELEASE",
"yyROWSET_CHANGED","yyCOLUMN_SET","yyCOLUMN_RECALCULATED","yyROW_ACTIVATE",
"yyROW_RELEASE","yyROW_DELETE","yyROW_FIRSTCHANGE","yyROW_INSERT",
"yyROW_RESYNCH","yyROW_UNDOCHANGE","yyROW_UNDOINSERT","yyROW_UNDODELETE",
"yyROW_UPDATE","yyOKTODO","yyABOUTTODO","yySYNCHAFTER","yyDIDEVENT",
"yyFAILEDTODO","yyGETDATA","yyGETORIGINALDATA","yyGETVISIBLEDATA",
"yyRELEASEROWSET","yyADDREFROWSET","yyGETNEXTROWS","yyRELEASEROWS",
"yyADDREFROWS","yyRESTARTPOSITION","yyDELETEROWS","yyINSERTROW","yySETDATA",
"yySEEK","yyGETROWSAT","yyGETROWSBYBOOKMARK","yyRESYNCHROWS","yyGETROWSATRATIO",
"yyUNDO","yyUPDATE","yyABORT","yyCOMMIT","yyADDCOLUMN","yyDROPCOLUMN",
"yyGETCOLUMNINFO","yyCONTROL","yyLISTENER","yyTHREAD","yyDBBMK_FIRST",
"yyDBBMK_LAST","yyRETURN_ACCEPT","yyRETURN_VETO","yyRETURN_EFAIL",
"yyRETURN_EOUTOFMEMORY","yyRETURN_EINVALIDARG","yyRETURN_UNWANTEDREASON",
"yyRETURN_UNWANTEDPHASE","yyRETURN_ADVISE","yyRETURN_UNADVISE",
"yyDB_S_ENDOFROWSET","yyDB_E_BADSTARTPOSITION","yyDB_E_NOTREENTRANT",
"yyDB_S_ERRORSOCCURRED","yyDB_E_ERRORSOCCURRED","yyE_INVALIDARG",
"yyDB_E_ROWSNOTRELEASED","yyDB_E_BADACCESSORHANDLE","yyDB_E_BADROWHANDLE",
};
char *yyrule[] = {
"$accept : Notification_Test",
"$$1 :",
"Notification_Test : $$1 '{' ListenerList ControlList '}'",
"ListenerList : Listener",
"ListenerList : ListenerList Listener",
"Listener : yyLISTENER yyIDENTIFIERID ';'",
"ControlList : Control",
"ControlList : ControlList Control",
"$$2 :",
"Control : '{' yyCONTROL yyIDENTIFIERID ';' $$2 '{' ControlStmtList '}' '}'",
"ControlStmtList : ControlStmt",
"ControlStmtList : ControlStmtList ControlStmt",
"ControlStmt : OLEDBMethod '{' ListenerStmtList '}'",
"ControlStmt : OLEDBMethod '{' '}'",
"ListenerStmtList : ListenerStmt",
"ListenerStmtList : ListenerStmtList ListenerStmt",
"ListenerStmt : ListenerIdentifier '.' Reason '.' Phase '.' ListenerMethod",
"ListenerIdentifier : yyIDENTIFIERID",
"ListenerMethod : ListenerReturn",
"ListenerMethod : OLEDBMethod '{' ListenerStmtList '}'",
"ListenerMethod : OLEDBMethod '{' '}'",
"$$3 :",
"ListenerMethod : yyTHREAD $$3 '{' ListenerMethod '}'",
"ListenerReturn : yyRETURN_ACCEPT",
"ListenerReturn : yyRETURN_VETO",
"ListenerReturn : yyRETURN_EFAIL",
"ListenerReturn : yyRETURN_EOUTOFMEMORY",
"ListenerReturn : yyRETURN_EINVALIDARG",
"ListenerReturn : yyRETURN_UNWANTEDREASON",
"ListenerReturn : yyRETURN_UNWANTEDPHASE",
"ListenerReturn : yyRETURN_ADVISE",
"ListenerReturn : yyRETURN_UNADVISE",
"OLEDBMethod : ReleaseRowsetMethod",
"OLEDBMethod : GetNextRows",
"OLEDBMethod : ReleaseRowsMethod",
"OLEDBMethod : RestartPositionMethod",
"OLEDBMethod : DeleteRowsMethod",
"OLEDBMethod : InsertRowMethod",
"OLEDBMethod : SetDataMethod",
"OLEDBMethod : SeekMethod",
"OLEDBMethod : GetRowsAtMethod",
"OLEDBMethod : GetRowsByBookmarkMethod",
"OLEDBMethod : ResynchRowsMethod",
"OLEDBMethod : GetRowsAtRatioMethod",
"OLEDBMethod : UndoMethod",
"OLEDBMethod : UpdateMethod",
"OLEDBMethod : AddRefRowsetMethod",
"OLEDBMethod : AddRefRowsMethod",
"OLEDBMethod : GetDataMethod",
"OLEDBMethod : GetOriginalDataMethod",
"OLEDBMethod : GetVisibleDataMethod",
"OLEDBMethod : AbortMethod",
"OLEDBMethod : CommitMethod",
"OLEDBMethod : AddColumnMethod",
"OLEDBMethod : DropColumnMethod",
"OLEDBMethod : GetColumnInfoMethod",
"ReleaseRowsMethod : yyRELEASEROWS '(' IdentifierList ')'",
"GetNextRows : yyGETNEXTROWS '(' IdentifierList ')'",
"ReleaseRowsetMethod : yyRELEASEROWSET '(' EmptyList ')'",
"RestartPositionMethod : yyRESTARTPOSITION '(' EmptyList ')'",
"DeleteRowsMethod : yyDELETEROWS '(' IdentifierList ')'",
"InsertRowMethod : yyINSERTROW '(' yyIDENTIFIERID ')'",
"InsertRowMethod : yyINSERTROW '(' yyIDENTIFIERID ',' ExpectedHRESULT ')'",
"SetDataMethod : yySETDATA '(' yyIDENTIFIERID ',' OrdinalList ')'",
"SetDataMethod : yySETDATA '(' yyIDENTIFIERID ',' OrdinalList ',' ExpectedHRESULT ')'",
"SeekMethod : yySEEK '(' yyNUMBER ')'",
"SeekMethod : yySEEK '(' yyNUMBER ',' ExpectedHRESULT ')'",
"GetRowsAtMethod : yyGETROWSAT '(' yyDBBMK_FIRST ',' IdentifierList ')'",
"GetRowsAtMethod : yyGETROWSAT '(' yyDBBMK_FIRST ')'",
"GetRowsAtMethod : yyGETROWSAT '(' yyDBBMK_LAST ',' IdentifierList ')'",
"GetRowsAtMethod : yyGETROWSAT '(' yyDBBMK_LAST ')'",
"GetRowsByBookmarkMethod : yyGETROWSBYBOOKMARK '(' IdentifierList ')'",
"ResynchRowsMethod : yyRESYNCHROWS '(' IdentifierList ')'",
"GetRowsAtRatioMethod : yyGETROWSATRATIO '(' yyNUMBER ')'",
"GetRowsAtRatioMethod : yyGETROWSATRATIO '(' yyNUMBER ',' IdentifierList ')'",
"UndoMethod : yyUNDO '(' IdentifierList ')'",
"UpdateMethod : yyUPDATE '(' IdentifierList ')'",
"AddRefRowsMethod : yyADDREFROWS '(' IdentifierList ')'",
"AddRefRowsetMethod : yyADDREFROWSET '(' EmptyList ')'",
"GetDataMethod : yyGETDATA '(' yyIDENTIFIERID ')'",
"GetOriginalDataMethod : yyGETORIGINALDATA '(' yyIDENTIFIERID ')'",
"GetOriginalDataMethod : yyGETORIGINALDATA '(' yyIDENTIFIERID ',' ExpectedHRESULT ')'",
"GetVisibleDataMethod : yyGETVISIBLEDATA '(' yyIDENTIFIERID ')'",
"GetVisibleDataMethod : yyGETVISIBLEDATA '(' yyIDENTIFIERID ',' ExpectedHRESULT ')'",
"AbortMethod : yyABORT '(' EmptyList ')'",
"CommitMethod : yyCOMMIT '(' EmptyList ')'",
"AddColumnMethod : yyADDCOLUMN '(' EmptyList ')'",
"DropColumnMethod : yyDROPCOLUMN '(' EmptyList ')'",
"GetColumnInfoMethod : yyGETCOLUMNINFO '(' EmptyList ')'",
"OrdinalList : yyNUMBER",
"OrdinalList : OrdinalList ',' yyNUMBER",
"IdentifierList : yyIDENTIFIERID",
"IdentifierList : EmptyList",
"IdentifierList : IdentifierList ',' yyIDENTIFIERID",
"IdentifierList : IdentifierList ',' ExpectedHRESULT",
"EmptyList : ExpectedHRESULT",
"EmptyList :",
"ExpectedHRESULT : yyDB_S_ENDOFROWSET",
"ExpectedHRESULT : yyDB_E_BADSTARTPOSITION",
"ExpectedHRESULT : yyDB_E_NOTREENTRANT",
"ExpectedHRESULT : yyDB_S_ERRORSOCCURRED",
"ExpectedHRESULT : yyDB_E_ERRORSOCCURRED",
"ExpectedHRESULT : yyE_INVALIDARG",
"ExpectedHRESULT : yyDB_E_ROWSNOTRELEASED",
"ExpectedHRESULT : yyDB_E_BADACCESSORHANDLE",
"ExpectedHRESULT : yyDB_E_BADROWHANDLE",
"Reason : yyROWSET_FETCHPOSITIONCHANGE",
"Reason : yyROWSET_RELEASE",
"Reason : yyROWSET_CHANGED",
"Reason : yyCOLUMN_SET",
"Reason : yyCOLUMN_RECALCULATED",
"Reason : yyROW_ACTIVATE",
"Reason : yyROW_RELEASE",
"Reason : yyROW_DELETE",
"Reason : yyROW_FIRSTCHANGE",
"Reason : yyROW_INSERT",
"Reason : yyROW_RESYNCH",
"Reason : yyROW_UNDOCHANGE",
"Reason : yyROW_UNDOINSERT",
"Reason : yyROW_UNDODELETE",
"Reason : yyROW_UPDATE",
"Phase : yyOKTODO",
"Phase : yyABOUTTODO",
"Phase : yySYNCHAFTER",
"Phase : yyDIDEVENT",
"Phase : yyFAILEDTODO",
};
#endif
#ifdef YYSTACKSIZE
#undef YYMAXDEPTH
#define YYMAXDEPTH YYSTACKSIZE
#else
#ifdef YYMAXDEPTH
#define YYSTACKSIZE YYMAXDEPTH
#else
#define YYSTACKSIZE 500
#define YYMAXDEPTH 500
#endif
#endif
int yydebug;
int yynerrs;
int yyerrflag;
int yychar;
short *yyssp;
YYSTYPE *yyvsp;
YYSTYPE yyval;
YYSTYPE yylval;
short yyss[YYSTACKSIZE];
YYSTYPE yyvs[YYSTACKSIZE];
#define yystacksize YYSTACKSIZE



int yyerror(char*s)
{
	return 0;
}

// Keyword Table for Parser
typedef CMap <CString, LPCTSTR, ULONG, ULONG> TABLE;
class KEYWORDTABLE : public TABLE
	{
	public:
		KEYWORDTABLE();
		~KEYWORDTABLE();
	};

KEYWORDTABLE::KEYWORDTABLE()
	{
		SetAt(_T("IRowset::GetData"),yyGETDATA);
		SetAt(_T("IRowsetUpdate::GetOriginalData"),yyGETORIGINALDATA);
		SetAt(_T("IRowsetResynch::GetVisibleData"),yyGETVISIBLEDATA);

		SetAt(_T("IRowsetChange::SetData"),yySETDATA);
		SetAt(_T("IRowsetIndex::Seek"),yySEEK);
		SetAt(_T("IRowsetChange::DeleteRows"),yyDELETEROWS);
		SetAt(_T("IRowsetChange::InsertRow"),yyINSERTROW);
		SetAt(_T("IRowsetUpdate::Update"),yyUPDATE);
		SetAt(_T("IRowsetUpdate::Undo"),yyUNDO);
		SetAt(_T("IRowset::GetNextRows"),yyGETNEXTROWS);
		SetAt(_T("IRowset::ReleaseRows"),yyRELEASEROWS);
		SetAt(_T("IRowset::AddRefRows"),yyADDREFROWS);
		SetAt(_T("IRowset::AddRef"),yyADDREFROWSET);
		SetAt(_T("IRowset::Release"),yyRELEASEROWSET);
		SetAt(_T("IRowsetResynch::ResynchRows"),yyRESYNCHROWS);
		SetAt(_T("IRowsetLocate::GetRowsAt"),yyGETROWSAT);
		SetAt(_T("IRowsetScroll::GetRowsAtRatio"),yyGETROWSATRATIO);
		SetAt(_T("IRowset::RestartPosition"),yyRESTARTPOSITION);
		SetAt(_T("IRowsetLocate::GetRowsByBookmark"),yyGETROWSBYBOOKMARK);

		SetAt(_T("ITransaction::Abort"),	yyABORT);
		SetAt(_T("ITransaction::Commit"),	yyCOMMIT);

		SetAt(_T("ITableDefinition::AddColumn"),	yyADDCOLUMN);
		SetAt(_T("ITableDefinition::DropColumn"),	yyDROPCOLUMN);
		SetAt(_T("IColumnsInfo::GetColumnInfo"),	yyGETCOLUMNINFO);

//Listener Return values
		SetAt(_T("RETURN_ACCEPT"),yyRETURN_ACCEPT);
		SetAt(_T("RETURN_VETO"),yyRETURN_VETO);
		SetAt(_T("RETURN_EFAIL"),yyRETURN_EFAIL);
		SetAt(_T("RETURN_EOUTOFMEMORY"),yyRETURN_EOUTOFMEMORY);
		SetAt(_T("RETURN_EINVALIDARG"),yyRETURN_EINVALIDARG);
		SetAt(_T("RETURN_UNWANTEDREASON"),yyRETURN_UNWANTEDREASON);
		SetAt(_T("RETURN_UNWANTEDPHASE"),yyRETURN_UNWANTEDPHASE);
		SetAt(_T("RETURN_ADVISE"),yyRETURN_ADVISE);
		SetAt(_T("RETURN_UNADVISE"),yyRETURN_UNADVISE);

		SetAt(_T("Control"),yyCONTROL);
		SetAt(_T("Listener"),yyLISTENER);
		SetAt(_T("Thread"),yyTHREAD);
		SetAt(_T("COLUMN_SET"),yyCOLUMN_SET);
		SetAt(_T("ROW_DELETE"),yyROW_DELETE);
		SetAt(_T("ROW_INSERT"),yyROW_INSERT);
		SetAt(_T("ROW_FIRSTCHANGE"),yyROW_FIRSTCHANGE);
		SetAt(_T("ROW_ACTIVATE"),yyROW_ACTIVATE);
		SetAt(_T("ROW_RELEASE"),yyROW_RELEASE);
		SetAt(_T("ROW_UNDOCHANGE"),yyROW_UNDOCHANGE);
		SetAt(_T("ROW_UNDOINSERT"),yyROW_UNDOINSERT);
		SetAt(_T("ROW_UNDODELETE"),yyROW_UNDODELETE);
		SetAt(_T("ROW_UPDATE"),yyROW_UPDATE);
		SetAt(_T("ROW_RESYNCH"),yyROW_RESYNCH);
		SetAt(_T("ROWSET_RELEASE"),yyROWSET_RELEASE);
		SetAt(_T("ROWSET_FETCHPOSITIONCHANGE"), yyROWSET_FETCHPOSITIONCHANGE);
		SetAt(_T("ROWSET_CHANGED"), yyROWSET_CHANGED);
		SetAt(_T("COLUMN_RECALCULATED"), yyCOLUMN_RECALCULATED);
		SetAt(_T("DBBMK_FIRST"),yyDBBMK_FIRST);
		SetAt(_T("DBBMK_LAST"),yyDBBMK_LAST);

		SetAt(_T("OKTODO"),yyOKTODO);
		SetAt(_T("ABOUTTODO"),yyABOUTTODO);
		SetAt(_T("SYNCHAFTER"),yySYNCHAFTER);
		SetAt(_T("DIDEVENT"),yyDIDEVENT);
		SetAt(_T("FAILEDTODO"),yyFAILEDTODO);

//HrExpected
		SetAt(_T("DB_S_ENDOFROWSET"),yyDB_S_ENDOFROWSET);
		SetAt(_T("DB_E_BADSTARTPOSITION"),yyDB_E_BADSTARTPOSITION);
		SetAt(_T("DB_E_NOTREENTRANT"),yyDB_E_NOTREENTRANT);
		SetAt(_T("DB_S_ERRORSOCCURRED"),yyDB_S_ERRORSOCCURRED);
		SetAt(_T("DB_E_ERRORSOCCURRED"),yyDB_E_ERRORSOCCURRED);
		SetAt(_T("E_INVALIDARG"),yyE_INVALIDARG);
		SetAt(_T("DB_E_ROWSNOTRELEASED"),yyDB_E_ROWSNOTRELEASED);
		SetAt(_T("DB_E_BADACCESSORHANDLE"),yyDB_E_BADACCESSORHANDLE);
		SetAt(_T("DB_E_BADROWHANDLE"),yyDB_E_BADROWHANDLE);
	}

KEYWORDTABLE::~KEYWORDTABLE()
	{
	RemoveAll();
	}

// Keyword Table
KEYWORDTABLE KeywordTable;
// Identifier Table
TABLE        IdentifierTable;


WCHAR  LexBuf[100];
WCHAR *LexPtr;
ULONG  TokenId;
ULONG  IdentifierId = 1;

int yylexInit()
	{
	IdentifierId = 1;
	IdentifierTable.RemoveAll();
	return TRUE;
	}

int yylex()
	{
start:
    // Get next char from input string
	WCHAR wcChar = *yylpwzInput++;
	// Check for end of input string
	if (wcChar == L'\0') 
		{
		IdentifierId = 1;
		IdentifierTable.RemoveAll();
		return 0;
		}

	if (iswspace(wcChar)) goto start;
	// Check for special char and return it 
	if (!iswalpha(wcChar) && wcChar != L'_' && !iswdigit(wcChar))
		{
		char AnsiBuf[3];
		WideCharToMultiByte(CP_ACP,0,LPCWSTR(&wcChar),1,AnsiBuf,sizeof(AnsiBuf),NULL,NULL);
		return AnsiBuf[0];
		}

	// Check for digit and collect it
	if (iswdigit(wcChar))
		{
		LexPtr = LexBuf;
		*LexPtr++ = wcChar;
		while (iswdigit(*yylpwzInput)) *LexPtr++ = *yylpwzInput++;
		*LexPtr = L'\0';
		yylval.Number = _wtol(LexBuf);
		return yyNUMBER;
		}

	// Collect Identifier or Keyword
	LexPtr = LexBuf;
	*LexPtr++ = wcChar;
	while (iswalnum(*yylpwzInput)|| *yylpwzInput == L'_' || *yylpwzInput == L':') 
		*LexPtr++ = *yylpwzInput++;
	*LexPtr = L'\0';
	TCHAR tszLexBuf[200];
#ifndef _UNICODE	
	WideCharToMultiByte(CP_ACP,0,LexBuf,-1,tszLexBuf,200,NULL,NULL);
#else
	wcscpy(tszLexBuf, LexBuf);
#endif //_UNICODE

	// If Keyword, return Token Id else return Identifier Id 
	if (KeywordTable.Lookup(tszLexBuf,TokenId))
		return TokenId;
	else
		{
		if (!IdentifierTable.Lookup(tszLexBuf,yylval.IdentifierId))
			{
			yylval.IdentifierId = IdentifierId++;
			IdentifierTable.SetAt(tszLexBuf,yylval.IdentifierId);
			}
		return yyIDENTIFIERID;
		}
	}
#define YYABORT goto yyabort
#define YYREJECT goto yyabort
#define YYACCEPT goto yyaccept
#define YYERROR goto yyerrlab
int
yyparse()
{
    register int yym, yyn, yystate;
#if YYDEBUG
    register char *yys;
    extern char *getenv();

    if (yys = getenv("YYDEBUG"))
    {
        yyn = *yys;
        if (yyn >= '0' && yyn <= '9')
            yydebug = yyn - '0';
    }
#endif

    yynerrs = 0;
    yyerrflag = 0;
    yychar = (-1);

    yyssp = yyss;
    yyvsp = yyvs;
    *yyssp = yystate = 0;

yyloop:
    if (yyn = yydefred[yystate]) goto yyreduce;
    if (yychar < 0)
    {
        if ((yychar = yylex()) < 0) yychar = 0;
#if YYDEBUG
        if (yydebug)
        {
            yys = 0;
            if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
            if (!yys) yys = "illegal-symbol";
            printf("%sdebug: state %d, reading %d (%s)\n",
                    YYPREFIX, yystate, yychar, yys);
        }
#endif
    }
    if ((yyn = yysindex[yystate]) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)
    {
#if YYDEBUG
        if (yydebug)
            printf("%sdebug: state %d, shifting to state %d\n",
                    YYPREFIX, yystate, yytable[yyn]);
#endif
        if (yyssp >= yyss + yystacksize - 1)
        {
            goto yyoverflow;
        }
        *++yyssp = yystate = yytable[yyn];
        *++yyvsp = yylval;
        yychar = (-1);
        if (yyerrflag > 0)  --yyerrflag;
        goto yyloop;
    }
    if ((yyn = yyrindex[yystate]) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)
    {
        yyn = yytable[yyn];
        goto yyreduce;
    }
    if (yyerrflag) goto yyinrecovery;
#ifdef lint
    goto yynewerror;
#endif
yynewerror:
    yyerror("syntax error");
#ifdef lint
    goto yyerrlab;
#endif
yyerrlab:
    ++yynerrs;
yyinrecovery:
    if (yyerrflag < 3)
    {
        yyerrflag = 3;
        for (;;)
        {
            if ((yyn = yysindex[*yyssp]) && (yyn += YYERRCODE) >= 0 &&
                    yyn <= YYTABLESIZE && yycheck[yyn] == YYERRCODE)
            {
#if YYDEBUG
                if (yydebug)
                    printf("%sdebug: state %d, error recovery shifting\
 to state %d\n", YYPREFIX, *yyssp, yytable[yyn]);
#endif
                if (yyssp >= yyss + yystacksize - 1)
                {
                    goto yyoverflow;
                }
                *++yyssp = yystate = yytable[yyn];
                *++yyvsp = yylval;
                goto yyloop;
            }
            else
            {
#if YYDEBUG
                if (yydebug)
                    printf("%sdebug: error recovery discarding state %d\n",
                            YYPREFIX, *yyssp);
#endif
                if (yyssp <= yyss) goto yyabort;
                --yyssp;
                --yyvsp;
            }
        }
    }
    else
    {
        if (yychar == 0) goto yyabort;
#if YYDEBUG
        if (yydebug)
        {
            yys = 0;
            if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
            if (!yys) yys = "illegal-symbol";
            printf("%sdebug: state %d, error recovery discards token %d (%s)\n",
                    YYPREFIX, yystate, yychar, yys);
        }
#endif
        yychar = (-1);
        goto yyloop;
    }
yyreduce:
#if YYDEBUG
    if (yydebug)
        printf("%sdebug: state %d, reducing by rule %d (%s)\n",
                YYPREFIX, yystate, yyn, yyrule[yyn]);
#endif
    yym = yylen[yyn];
    yyval = yyvsp[1-yym];
    switch (yyn)
    {
case 1:
{ulCommandLevel = 1; }
break;
case 5:
{yypListenerMap->SetAt(yyvsp[-1].IdentifierId,new CImpIListener(yypExecutionManager));}
break;
case 8:
{
					pControl = new CControl(yypExecutionManager);
					pvObjectId = pControl;
					ulCommandLevel = 1;
					yypControlMap->SetAt(yyvsp[-1].IdentifierId,pControl);
					yypCommandList = new COMMANDLIST;
					yypCommandListMap->SetAt(pControl->GetThreadId(), yypCommandList);
				}
break;
case 12:
{
					ulCommandLevel++; 
					pvObjectId = pControl; 
				}
break;
case 13:
{
					ulCommandLevel++; 
					pvObjectId = pControl; 
				}
break;
case 17:
{CImpIListener *pImpIListener;
						 BOOL Ok = yypListenerMap->Lookup(yyvsp[0].IdentifierId,pImpIListener);
						 if (!Ok) yyerror("Invalid Listener");
						 pvObjectId = pImpIListener;}
break;
case 18:
{
					}
break;
case 19:
{
					}
break;
case 20:
{
					}
break;
case 21:
{ cThreads = 1; }
break;
case 22:
{
						cThreads = 0;
					}
break;
case 23:
{yypCommandList->AddTail(
					new COMMAND(RETURN_ACCEPT,			/*eCOMMANDTYPE*/
								pvObjectId,				/*pvObjectId*/
								ulCommandLevel, 		/*ulCommandLevel*/
								cThreads,				/*cThreads*/
								eReason,				/*eReason*/
								ePhase));}
break;
case 24:
{yypCommandList->AddTail(
					new COMMAND(RETURN_VETO,			/*eCOMMANDTYPE*/
								pvObjectId,				/*pvObjectId*/
								ulCommandLevel, 		/*ulCommandLevel*/
								cThreads,				/*cThreads*/
								eReason,				/*eReason*/
								ePhase));}
break;
case 25:
{yypCommandList->AddTail(
					new COMMAND(RETURN_EFAIL,			/*eCOMMANDTYPE*/
								pvObjectId,				/*pvObjectId*/
								ulCommandLevel, 		/*ulCommandLevel*/
								cThreads,				/*cThreads*/
								eReason,				/*eReason*/
								ePhase));}
break;
case 26:
{yypCommandList->AddTail(
					new COMMAND(RETURN_EOUTOFMEMORY,	/*eCOMMANDTYPE*/
								pvObjectId,				/*pvObjectId*/
								ulCommandLevel, 		/*ulCommandLevel*/
								cThreads,				/*cThreads*/
								eReason,				/*eReason*/
								ePhase));}
break;
case 27:
{yypCommandList->AddTail(
					new COMMAND(RETURN_EINVALIDARG,		/*eCOMMANDTYPE*/
								pvObjectId,				/*pvObjectId*/
								ulCommandLevel, 		/*ulCommandLevel*/
								cThreads,				/*cThreads*/
								eReason,				/*eReason*/
								ePhase));}
break;
case 28:
{yypCommandList->AddTail(
					new COMMAND(RETURN_UNWANTEDREASON,	/*eCOMMANDTYPE*/
								pvObjectId,				/*pvObjectId*/
								ulCommandLevel, 		/*ulCommandLevel*/
								cThreads,				/*cThreads*/
								eReason,				/*eReason*/
								ePhase));}
break;
case 29:
{yypCommandList->AddTail(
					new COMMAND(RETURN_UNWANTEDPHASE,	/*eCOMMANDTYPE*/
								pvObjectId,				/*pvObjectId*/
								ulCommandLevel, 		/*ulCommandLevel*/
								cThreads,				/*cThreads*/
								eReason,				/*eReason*/
								ePhase));}
break;
case 30:
{yypCommandList->AddTail(
					new COMMAND(RETURN_ADVISE,			/*eCOMMANDTYPE*/
								pvObjectId,				/*pvObjectId*/
								ulCommandLevel, 		/*ulCommandLevel*/
								cThreads,				/*cThreads*/
								eReason,				/*eReason*/
								ePhase));}
break;
case 31:
{yypCommandList->AddTail(
					new COMMAND(RETURN_UNADVISE,		/*eCOMMANDTYPE*/
								pvObjectId,				/*pvObjectId*/
								ulCommandLevel, 		/*ulCommandLevel*/
								cThreads,				/*cThreads*/
								eReason,				/*eReason*/
								ePhase));}
break;
case 56:
{yypCommandList->AddTail(
				new COMMAND(RELEASEROWS,		/*eCOMMANDTYPE*/
							pvObjectId,			/*pvObjectId*/
							ulCommandLevel, 	/*ulCommandLevel*/
							cThreads,				/*cThreads*/
							eReason,			/*eReason*/
							ePhase,				/*ePhase*/
							hrExpected,			/*hrExpected*/
							cRows,				/*cRows*/
							rgRowIds));}
break;
case 57:
{yypCommandList->AddTail(
				new COMMAND(GETNEXTROWS,		/*eCOMMANDTYPE*/
							pvObjectId,			/*pvObjectId*/
							ulCommandLevel, 	/*ulCommandLevel*/
							cThreads,				/*cThreads*/
							eReason,			/*eReason*/
							ePhase,				/*ePhase*/
							hrExpected,			/*hrExpected*/
							cRows,				/*cRows*/
							rgRowIds));}
break;
case 58:
{yypCommandList->AddTail(
				new COMMAND(RELEASEROWSET,		/*eCOMMANDTYPE*/
							pvObjectId,			/*pvObjectId*/
							ulCommandLevel, 	/*ulCommandLevel*/
							cThreads,				/*cThreads*/
							eReason,			/*eReason*/
							ePhase,				/*ePhase*/
							hrExpected));}
break;
case 59:
{yypCommandList->AddTail(
				new COMMAND(RESTARTPOSITION,	/*eCOMMANDTYPE*/
							pvObjectId,			/*pvObjectId*/
							ulCommandLevel, 	/*ulCommandLevel*/
							cThreads,				/*cThreads*/
							eReason,			/*eReason*/
							ePhase,				/*ePhase*/
							hrExpected));}
break;
case 60:
{yypCommandList->AddTail(
				new COMMAND(DELETEROWS,			/*eCOMMANDTYPE*/
							pvObjectId,			/*pvObjectId*/
							ulCommandLevel, 	/*ulCommandLevel*/
							cThreads,				/*cThreads*/
							eReason,			/*eReason*/
							ePhase,				/*ePhase*/
							hrExpected,			/*hrExpected*/
							cRows,				/*cRows*/
							rgRowIds));}
break;
case 61:
{yypCommandList->AddTail(
				new COMMAND(INSERTROW,			/*eCOMMANDTYPE*/
							pvObjectId,			/*pvObjectId*/
							ulCommandLevel, 	/*ulCommandLevel*/
							cThreads,				/*cThreads*/
							eReason,			/*eReason*/
							ePhase,				/*ePhase*/
							S_OK,				/*hrExpected*/
							1,					/*cRows*/
							&(yyvsp[-1].IdentifierId)));}
break;
case 62:
{yypCommandList->AddTail(
				new COMMAND(INSERTROW,			/*eCOMMANDTYPE*/
							pvObjectId,			/*pvObjectId*/
							ulCommandLevel, 	/*ulCommandLevel*/
							cThreads,				/*cThreads*/
							eReason,			/*eReason*/
							ePhase,				/*ePhase*/
							hrExpected,			/*hrExpected*/
							1,					/*cRows*/
							&(yyvsp[-3].IdentifierId)));}
break;
case 63:
{yypCommandList->AddTail(
				new COMMAND(SETDATA,			/*eCOMMANDTYPE*/
							pvObjectId,			/*pvObjectId*/
							ulCommandLevel, 	/*ulCommandLevel*/
							cThreads,				/*cThreads*/
							eReason,			/*eReason*/
							ePhase,				/*ePhase*/
							S_OK,				/*hrExpected*/
							1,					/*cRows*/
							&(yyvsp[-3].IdentifierId),				/*rgRowIds*/
							cColumns,			/*cColumns*/
							rgColumns));}
break;
case 64:
{yypCommandList->AddTail(
				new COMMAND(SETDATA,			/*eCOMMANDTYPE*/
							pvObjectId,			/*pvObjectId*/
							ulCommandLevel, 	/*ulCommandLevel*/
							cThreads,				/*cThreads*/
							eReason,			/*eReason*/
							ePhase,				/*ePhase*/
							hrExpected,			/*hrExpected*/
							1,					/*cRows*/
							&(yyvsp[-5].IdentifierId),				/*rgRowIds*/
							cColumns,			/*cColumns*/
							rgColumns));}
break;
case 65:
{yypCommandList->AddTail(
				new COMMAND(SEEK,				/*eCOMMANDTYPE*/
							pvObjectId,			/*pvObjectId*/
							ulCommandLevel, 	/*ulCommandLevel*/
							cThreads,				/*cThreads*/
							eReason,			/*eReason*/
							ePhase,				/*ePhase*/
							hrExpected,			/*hrExpected*/
							yyvsp[-1].Number,					/*cRows*/
							rgRowIds));}
break;
case 66:
{yypCommandList->AddTail(
				new COMMAND(SEEK,				/*eCOMMANDTYPE*/
							pvObjectId,			/*pvObjectId*/
							ulCommandLevel, 	/*ulCommandLevel*/
							cThreads,				/*cThreads*/
							eReason,			/*eReason*/
							ePhase,				/*ePhase*/
							hrExpected,			/*hrExpected*/
							yyvsp[-3].Number,					/*cRows*/
							rgRowIds));}
break;
case 67:
{yypCommandList->AddTail(
				new COMMAND(GETROWSAT,			/*eCOMMANDTYPE*/
							pvObjectId,			/*pvObjectId*/
							ulCommandLevel, 	/*ulCommandLevel*/
							cThreads,				/*cThreads*/
							eReason,			/*eReason*/
							ePhase,				/*ePhase*/
							hrExpected,			/*hrExpected*/
							cRows,				/*cRows*/
							rgRowIds,			/*rgRowIds*/
							0,					/*cColumns*/
							NULL,				/*rgColumns*/
							DBBMK_FIRST));}
break;
case 68:
{yypCommandList->AddTail(
				new COMMAND(GETROWSAT,			/*eCOMMANDTYPE*/
							pvObjectId,			/*pvObjectId*/
							ulCommandLevel, 	/*ulCommandLevel*/
							cThreads,				/*cThreads*/
							eReason,			/*eReason*/
							ePhase,				/*ePhase*/
							S_OK,				/*hrExpected*/
							0,					/*cRows*/
							NULL,				/*rgRowIds*/
							0,					/*cColumns*/
							NULL,				/*rgColumns*/
							DBBMK_FIRST));}
break;
case 69:
{yypCommandList->AddTail(
				new COMMAND(GETROWSAT,			/*eCOMMANDTYPE*/
							pvObjectId,			/*pvObjectId*/
							ulCommandLevel, 	/*ulCommandLevel*/
							cThreads,				/*cThreads*/
							eReason,			/*eReason*/
							ePhase,				/*ePhase*/
							hrExpected,			/*hrExpected*/
							cRows,				/*cRows*/
							rgRowIds,			/*rgRowIds*/
							0,					/*cColumns*/
							NULL,				/*rgColumns*/
							DBBMK_LAST));}
break;
case 70:
{yypCommandList->AddTail(
				new COMMAND(GETROWSAT,			/*eCOMMANDTYPE*/
							pvObjectId,			/*pvObjectId*/
							ulCommandLevel, 	/*ulCommandLevel*/
							cThreads,				/*cThreads*/
							eReason,			/*eReason*/
							ePhase,				/*ePhase*/
							hrExpected,			/*hrExpected*/
							0,					/*cRows*/
							NULL,				/*rgRowIds*/
							0,					/*cColumns*/
							NULL,				/*rgColumns*/
							DBBMK_LAST));}
break;
case 71:
{yypCommandList->AddTail(
				new COMMAND(GETROWSBYBOOKMARK,	/*eCOMMANDTYPE*/
							pvObjectId,			/*pvObjectId*/
							ulCommandLevel, 	/*ulCommandLevel*/
							cThreads,				/*cThreads*/
							eReason,			/*eReason*/
							ePhase,				/*ePhase*/
							hrExpected,			/*hrExpected*/
							cRows,				/*cRows*/
							rgRowIds));}
break;
case 72:
{yypCommandList->AddTail(
				new COMMAND(RESYNCHROWS,		/*eCOMMANDTYPE*/
							pvObjectId,			/*pvObjectId*/
							ulCommandLevel, 	/*ulCommandLevel*/
							cThreads,				/*cThreads*/
							eReason,			/*eReason*/
							ePhase,				/*ePhase*/
							hrExpected,			/*hrExpected*/
							cRows,				/*cRows*/
							rgRowIds));}
break;
case 73:
{yypCommandList->AddTail(
				new COMMAND(GETROWSATRATIO,		/*eCOMMANDTYPE*/
							pvObjectId,			/*pvObjectId*/
							ulCommandLevel, 	/*ulCommandLevel*/
							cThreads,				/*cThreads*/
							eReason,			/*eReason*/
							ePhase,				/*ePhase*/
							hrExpected,			/*hrExpected*/
							0,					/*cRows*/
							NULL,				/*rgRowIds*/
							0,					/*cColumns*/
							NULL,				/*rgColumns*/
							yyvsp[-1].Number));}
break;
case 74:
{yypCommandList->AddTail(
				new COMMAND(GETROWSATRATIO,		/*eCOMMANDTYPE*/
							pvObjectId,			/*pvObjectId*/
							ulCommandLevel, 	/*ulCommandLevel*/
							cThreads,				/*cThreads*/
							eReason,			/*eReason*/
							ePhase,				/*ePhase*/
							hrExpected,			/*hrExpected*/
							cRows,				/*cRows*/
							rgRowIds,			/*rgRowIds*/
							0,					/*cColumns*/
							NULL,				/*rgColumns*/
							yyvsp[-3].Number));}
break;
case 75:
{yypCommandList->AddTail(
				new COMMAND(UNDO,				/*eCOMMANDTYPE*/
							pvObjectId,			/*pvObjectId*/
							ulCommandLevel, 	/*ulCommandLevel*/
							cThreads,				/*cThreads*/
							eReason,			/*eReason*/
							ePhase,				/*ePhase*/
							hrExpected,			/*hrExpected*/
							cRows,				/*cRows*/
							rgRowIds));}
break;
case 76:
{yypCommandList->AddTail(
				new COMMAND(UPDATE,				/*eCOMMANDTYPE*/
							pvObjectId,			/*pvObjectId*/
							ulCommandLevel, 	/*ulCommandLevel*/
							cThreads,				/*cThreads*/
							eReason,			/*eReason*/
							ePhase,				/*ePhase*/
							hrExpected,			/*hrExpected*/
							cRows,				/*cRows*/
							rgRowIds));}
break;
case 77:
{yypCommandList->AddTail(
				new COMMAND(ADDREFROWS,			/*eCOMMANDTYPE*/
							pvObjectId,			/*pvObjectId*/
							ulCommandLevel, 	/*ulCommandLevel*/
							cThreads,				/*cThreads*/
							eReason,			/*eReason*/
							ePhase,				/*ePhase*/
							hrExpected,			/*hrExpected*/
							cRows,				/*cRows*/
							rgRowIds));}
break;
case 78:
{yypCommandList->AddTail(
				new COMMAND(ADDREFROWSET,		/*eCOMMANDTYPE*/
							pvObjectId,			/*pvObjectId*/
							ulCommandLevel, 	/*ulCommandLevel*/
							cThreads,				/*cThreads*/
							eReason,			/*eReason*/
							ePhase,				/*ePhase*/
							hrExpected));}
break;
case 79:
{yypCommandList->AddTail(
				new COMMAND(GETDATA,			/*eCOMMANDTYPE*/
							pvObjectId,			/*pvObjectId*/
							ulCommandLevel, 	/*ulCommandLevel*/
							cThreads,				/*cThreads*/
							eReason,			/*eReason*/
							ePhase,				/*ePhase*/
							S_OK,				/*hrExpected*/
							1,					/*cRows*/
							&(yyvsp[-1].IdentifierId) ));}
break;
case 80:
{yypCommandList->AddTail(
				new COMMAND(GETORIGINALDATA,	/*eCOMMANDTYPE*/
							pvObjectId,			/*pvObjectId*/
							ulCommandLevel, 	/*ulCommandLevel*/
							cThreads,				/*cThreads*/
							eReason,			/*eReason*/
							ePhase,				/*ePhase*/
							S_OK,				/*hrExpected*/
							1,					/*cRows*/
							&(yyvsp[-1].IdentifierId) ));}
break;
case 81:
{yypCommandList->AddTail(
				new COMMAND(GETORIGINALDATA,			/*eCOMMANDTYPE*/
							pvObjectId,			/*pvObjectId*/
							ulCommandLevel, 	/*ulCommandLevel*/
							cThreads,				/*cThreads*/
							eReason,			/*eReason*/
							ePhase,				/*ePhase*/
							hrExpected,			/*hrExpected*/
							1,					/*cRows*/
							&(yyvsp[-3].IdentifierId)));}
break;
case 82:
{yypCommandList->AddTail(
				new COMMAND(GETVISIBLEDATA,		/*eCOMMANDTYPE*/
							pvObjectId,			/*pvObjectId*/
							ulCommandLevel, 	/*ulCommandLevel*/
							cThreads,				/*cThreads*/
							eReason,			/*eReason*/
							ePhase,				/*ePhase*/
							S_OK,				/*hrExpected*/
							1,					/*cRows*/
							&(yyvsp[-1].IdentifierId) ));}
break;
case 83:
{yypCommandList->AddTail(
				new COMMAND(GETVISIBLEDATA,			/*eCOMMANDTYPE*/
							pvObjectId,			/*pvObjectId*/
							ulCommandLevel, 	/*ulCommandLevel*/
							cThreads,				/*cThreads*/
							eReason,			/*eReason*/
							ePhase,				/*ePhase*/
							hrExpected,			/*hrExpected*/
							1,					/*cRows*/
							&(yyvsp[-3].IdentifierId)));}
break;
case 84:
{yypCommandList->AddTail(
				new COMMAND(ABORT,				/*eCOMMANDTYPE*/
							pvObjectId,			/*pvObjectId*/
							ulCommandLevel, 	/*ulCommandLevel*/
							cThreads,				/*cThreads*/
							eReason,			/*eReason*/
							ePhase,				/*ePhase*/
							hrExpected));}
break;
case 85:
{yypCommandList->AddTail(
				new COMMAND(COMMIT,				/*eCOMMANDTYPE*/
							pvObjectId,			/*pvObjectId*/
							ulCommandLevel, 	/*ulCommandLevel*/
							cThreads,				/*cThreads*/
							eReason,			/*eReason*/
							ePhase,				/*ePhase*/
							hrExpected));}
break;
case 86:
{yypCommandList->AddTail(
				new COMMAND(ADDCOLUMN,			/*eCOMMANDTYPE*/
							pvObjectId,			/*pvObjectId*/
							ulCommandLevel, 	/*ulCommandLevel*/
							cThreads,				/*cThreads*/
							eReason,			/*eReason*/
							ePhase,				/*ePhase*/
							hrExpected));}
break;
case 87:
{yypCommandList->AddTail(
				new COMMAND(DROPCOLUMN,			/*eCOMMANDTYPE*/
							pvObjectId,			/*pvObjectId*/
							ulCommandLevel, 	/*ulCommandLevel*/
							cThreads,				/*cThreads*/
							eReason,			/*eReason*/
							ePhase,				/*ePhase*/
							hrExpected));}
break;
case 88:
{yypCommandList->AddTail(
				new COMMAND(GETCOLUMNINFO,		/*eCOMMANDTYPE*/
							pvObjectId,			/*pvObjectId*/
							ulCommandLevel, 	/*ulCommandLevel*/
							cThreads,				/*cThreads*/
							eReason,			/*eReason*/
							ePhase,				/*ePhase*/
							hrExpected));}
break;
case 89:
{cColumns = 0; rgColumns[cColumns++] = yyvsp[0].Number;}
break;
case 90:
{rgColumns[cColumns++] = yyvsp[0].Number;}
break;
case 91:
{
					cRows = 0; 
					rgRowIds[cRows++] = yyvsp[0].IdentifierId;
					hrExpected = S_OK;
				}
break;
case 92:
{
				}
break;
case 93:
{
					rgRowIds[cRows++] = yyvsp[0].IdentifierId;
				}
break;
case 94:
{
				}
break;
case 95:
{ 
					cRows = 0;
					cColumns = 0;
				}
break;
case 96:
{	
					cRows = 0;
					cColumns = 0;
					hrExpected = S_OK;
				}
break;
case 97:
{
//					cRows = SIZEOF_TABLE;
					hrExpected = DB_S_ENDOFROWSET;
				}
break;
case 98:
{
					hrExpected = DB_E_BADSTARTPOSITION;
				}
break;
case 99:
{
					hrExpected = DB_E_NOTREENTRANT;
				}
break;
case 100:
{
					hrExpected = DB_S_ERRORSOCCURRED;
				}
break;
case 101:
{
					hrExpected = DB_E_ERRORSOCCURRED;
				}
break;
case 102:
{
					hrExpected = E_INVALIDARG;
				}
break;
case 103:
{
					hrExpected = DB_E_ROWSNOTRELEASED;
				}
break;
case 104:
{
					hrExpected = DB_E_BADACCESSORHANDLE;
				}
break;
case 105:
{
					hrExpected = DB_E_BADROWHANDLE;
				}
break;
case 106:
{eReason = DBREASON_ROWSET_FETCHPOSITIONCHANGE;}
break;
case 107:
{eReason = DBREASON_ROWSET_RELEASE;}
break;
case 108:
{eReason = DBREASON_ROWSET_CHANGED;}
break;
case 109:
{eReason = DBREASON_COLUMN_SET;}
break;
case 110:
{eReason = DBREASON_COLUMN_RECALCULATED;}
break;
case 111:
{eReason = DBREASON_ROW_ACTIVATE;}
break;
case 112:
{eReason = DBREASON_ROW_RELEASE;}
break;
case 113:
{eReason = DBREASON_ROW_DELETE;}
break;
case 114:
{eReason = DBREASON_ROW_FIRSTCHANGE;}
break;
case 115:
{eReason = DBREASON_ROW_INSERT;}
break;
case 116:
{eReason = DBREASON_ROW_RESYNCH;}
break;
case 117:
{eReason = DBREASON_ROW_UNDOCHANGE;}
break;
case 118:
{eReason = DBREASON_ROW_UNDOINSERT;}
break;
case 119:
{eReason = DBREASON_ROW_UNDODELETE;}
break;
case 120:
{eReason = DBREASON_ROW_UPDATE;}
break;
case 121:
{ePhase = DBEVENTPHASE_OKTODO;}
break;
case 122:
{ePhase = DBEVENTPHASE_ABOUTTODO;}
break;
case 123:
{ePhase = DBEVENTPHASE_SYNCHAFTER;}
break;
case 124:
{ePhase = DBEVENTPHASE_DIDEVENT;}
break;
case 125:
{ePhase = DBEVENTPHASE_FAILEDTODO;}
break;
    }
    yyssp -= yym;
    yystate = *yyssp;
    yyvsp -= yym;
    yym = yylhs[yyn];
    if (yystate == 0 && yym == 0)
    {
#if YYDEBUG
        if (yydebug)
            printf("%sdebug: after reduction, shifting from state 0 to\
 state %d\n", YYPREFIX, YYFINAL);
#endif
        yystate = YYFINAL;
        *++yyssp = YYFINAL;
        *++yyvsp = yyval;
        if (yychar < 0)
        {
            if ((yychar = yylex()) < 0) yychar = 0;
#if YYDEBUG
            if (yydebug)
            {
                yys = 0;
                if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
                if (!yys) yys = "illegal-symbol";
                printf("%sdebug: state %d, reading %d (%s)\n",
                        YYPREFIX, YYFINAL, yychar, yys);
            }
#endif
        }
        if (yychar == 0) goto yyaccept;
        goto yyloop;
    }
    if ((yyn = yygindex[yym]) && (yyn += yystate) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yystate)
        yystate = yytable[yyn];
    else
        yystate = yydgoto[yym];
#if YYDEBUG
    if (yydebug)
        printf("%sdebug: after reduction, shifting from state %d \
to state %d\n", YYPREFIX, *yyssp, yystate);
#endif
    if (yyssp >= yyss + yystacksize - 1)
    {
        goto yyoverflow;
    }
    *++yyssp = (short)yystate;
    *++yyvsp = yyval;
    goto yyloop;
yyoverflow:
    yyerror("yacc stack overflow");
yyabort:
    return (1);
yyaccept:
    return (0);
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Test Case Section
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


// {{ TCW_TEST_CASE_MAP(Release)
//--------------------------------------------------------------------
// @class Tests Notifications for IRowset::Release
//
class Release : public CExecutionManager { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Release,CExecutionManager);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember ROWSET_RELEASE - Accept all phases
	int Variation_1();
	// @cmember Empty
	int Variation_2();
	// @cmember Empty
	int Variation_3();
	// @cmember ROWSET_RELEASE - DB_E_CANCELED
	int Variation_4();
	// @cmember ROWSET_RELEASE - DB_E_NOTREENTRANT
	int Variation_5();
	// @cmember Empty
	int Variation_6();
	// @cmember Empty
	int Variation_7();
	// @cmember ROWSET_RELEASE - Veto DIDEVENT, Listener 1
	int Variation_8();
	// @cmember Empty
	int Variation_9();
	// @cmember Empty
	int Variation_10();
	// @cmember ROWSET_RELEASE - UNWANTED REASON
	int Variation_11();
	// @cmember ROWSET_RELEASE - UNWANTED PHASE - DIDEVENT
	int Variation_12();
	// @cmember Empty
	int Variation_13();
	// @cmember Empty
	int Variation_14();
	// @cmember ROWSET_RELEASE - E_FAIL - All Phases
	int Variation_15();
	// @cmember ROWSET_RELEASE - E_OUTOFMEMORY - All Phases
	int Variation_16();
	// @cmember ROWSET_RELEASE - E_INVALIDARG - All Phases
	int Variation_17();
	// @cmember Empty
	int Variation_18();
	// @cmember Empty
	int Variation_19();
	// @cmember OTHER SCENARIOS - AddRef - Release - no notifications
	int Variation_20();
	// @cmember OTHER SCENARIOS - EmptyRowset
	int Variation_21();
	// @cmember OTHER SCENARIOS - Remove Listeners
	int Variation_22();
	// @cmember OTHER SCENARIOS - Add Listeners
	int Variation_23();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(Release)
#define THE_CLASS Release
BEG_TEST_CASE(Release, CExecutionManager, L"Tests Notifications for IRowset::Release")
	TEST_VARIATION(1, 		L"ROWSET_RELEASE - Accept all phases")
	TEST_VARIATION(2, 		L"Empty")
	TEST_VARIATION(3, 		L"Empty")
	TEST_VARIATION(4, 		L"ROWSET_RELEASE - DB_E_CANCELED")
	TEST_VARIATION(5, 		L"ROWSET_RELEASE - DB_E_NOTREENTRANT")
	TEST_VARIATION(6, 		L"Empty")
	TEST_VARIATION(7, 		L"Empty")
	TEST_VARIATION(8, 		L"ROWSET_RELEASE - Veto DIDEVENT, Listener 1")
	TEST_VARIATION(9, 		L"Empty")
	TEST_VARIATION(10, 		L"Empty")
	TEST_VARIATION(11, 		L"ROWSET_RELEASE - UNWANTED REASON")
	TEST_VARIATION(12, 		L"ROWSET_RELEASE - UNWANTED PHASE - DIDEVENT")
	TEST_VARIATION(13, 		L"Empty")
	TEST_VARIATION(14, 		L"Empty")
	TEST_VARIATION(15, 		L"ROWSET_RELEASE - E_FAIL - All Phases")
	TEST_VARIATION(16, 		L"ROWSET_RELEASE - E_OUTOFMEMORY - All Phases")
	TEST_VARIATION(17, 		L"ROWSET_RELEASE - E_INVALIDARG - All Phases")
	TEST_VARIATION(18, 		L"Empty")
	TEST_VARIATION(19, 		L"Empty")
	TEST_VARIATION(20, 		L"OTHER SCENARIOS - AddRef - Release - no notifications")
	TEST_VARIATION(21, 		L"OTHER SCENARIOS - EmptyRowset")
	TEST_VARIATION(22, 		L"OTHER SCENARIOS - Remove Listeners")
	TEST_VARIATION(23, 		L"OTHER SCENARIOS - Add Listeners")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(GetNextRows)
//--------------------------------------------------------------------
// @class Tests Notifications for IRowset::GetNextRows
//
class GetNextRows : public CExecutionManager { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(GetNextRows,CExecutionManager);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember FETCHPOSITIONCHANGE - SINGLEROW - Accept all phases
	int Variation_1();
	// @cmember FETCHPOSITIONCHANGE - MULTIPLEROWS - Accept all phases
	int Variation_2();
	// @cmember Empty
	int Variation_3();
	// @cmember Empty
	int Variation_4();
	// @cmember FETCHPOSITIONCHANGE - DB_E_CANCELED
	int Variation_5();
	// @cmember FETCHPOSITIONCHANGE - DB_E_NOTREENTRANT
	int Variation_6();
	// @cmember FETCHPOSITIONCHANGE - DB_E_NOTREENTRANT - Non-notification method
	int Variation_7();
	// @cmember Empty
	int Variation_8();
	// @cmember FETCHPOSITIONCHANGE - Veto OKTODO, Listener 1
	int Variation_9();
	// @cmember FETCHPOSITIONCHANGE - Veto ABOUTTODO, Listener 1
	int Variation_10();
	// @cmember FETCHPOSITIONCHANGE - Veto SYNCHAFTER, Listener 1
	int Variation_11();
	// @cmember FETCHPOSITIONCHANGE - Veto DIDEVENT, Listener 1
	int Variation_12();
	// @cmember FETCHPOSITIONCHANGE - Veto FAILEDTODO, Listener 1
	int Variation_13();
	// @cmember FETCHPOSITIONCHANGE - Veto OKTODO, Listener 2
	int Variation_14();
	// @cmember FETCHPOSITIONCHANGE - Veto ABOUTTODO, Listener 2
	int Variation_15();
	// @cmember FETCHPOSITIONCHANGE -  Veto SYNCHAFTER, Listener 3
	int Variation_16();
	// @cmember Empty
	int Variation_17();
	// @cmember Empty
	int Variation_18();
	// @cmember FETCHPOSITIONCHANGE - UNWANTED REASON
	int Variation_19();
	// @cmember FETCHPOSITIONCHANGE - UNWANTED PHASE - OKTODO
	int Variation_20();
	// @cmember FETCHPOSITIONCHANGE - UNWANTED PHASE - ABOUTTODO
	int Variation_21();
	// @cmember FETCHPOSITIONCHANGE - UNWANTED PHASE - SYNCHAFTER
	int Variation_22();
	// @cmember FETCHPOSITIONCHANGE - UNWANTED PHASE - DIDENVENT
	int Variation_23();
	// @cmember FETCHPOSITIONCHANGE - UNWANTED PHASE - FAILEDTODO
	int Variation_24();
	// @cmember Empty
	int Variation_25();
	// @cmember Empty
	int Variation_26();
	// @cmember FETCHPOSITIONCHANGE - E_FAIL - All Phases
	int Variation_27();
	// @cmember FETCHPOSITIONCHANGE - E_OUTOFMEMORY - All Phases
	int Variation_28();
	// @cmember FETCHPOSITIONCHANGE - E_INVALIDARG - All Phases
	int Variation_29();
	// @cmember Empty
	int Variation_30();
	// @cmember Empty
	int Variation_31();
	// @cmember ROW_ACTIVATE - SINGLEROW - Accept all phases
	int Variation_32();
	// @cmember ROW_ACTIVATE - MULTIPLEROWS - Accept all phases
	int Variation_33();
	// @cmember Empty
	int Variation_34();
	// @cmember Empty
	int Variation_35();
	// @cmember ROW_ACTIVATE - DB_E_CANCELED
	int Variation_36();
	// @cmember ROW_ACTIVATE - DB_E_NOTREENTRANT
	int Variation_37();
	// @cmember Empty
	int Variation_38();
	// @cmember Empty
	int Variation_39();
	// @cmember ROW_ACTIVATE - Veto DIDEVENT, Listener 1
	int Variation_40();
	// @cmember Empty
	int Variation_41();
	// @cmember Empty
	int Variation_42();
	// @cmember ROW_ACTIVATE - UNWANTED REASON
	int Variation_43();
	// @cmember ROW_ACTIVATE - UNWANTED PHASE - DIDEVENT
	int Variation_44();
	// @cmember Empty
	int Variation_45();
	// @cmember Empty
	int Variation_46();
	// @cmember ROW_ACTIVATE - E_FAIL - All Phases
	int Variation_47();
	// @cmember ROW_ACTIVATE - E_OUTOFMEMORY - All Phases
	int Variation_48();
	// @cmember ROW_ACTIVATE - E_INVALIDARG - All Phases
	int Variation_49();
	// @cmember Empty
	int Variation_50();
	// @cmember Empty
	int Variation_51();
	// @cmember OTHER SCENARIOS - Fetch no rows - no notifications
	int Variation_52();
	// @cmember OTHER SCENARIOS - Fetch A, Fetch A again
	int Variation_53();
	// @cmember OTHER SCENARIOS - Fetch A, Fetch A,B
	int Variation_54();
	// @cmember OTHER SCENARIOS - Fetch A, ReleaseRows A, Fetch A
	int Variation_55();
	// @cmember Empty
	int Variation_56();
	// @cmember OTHER SCENARIOS - DB_S_ENDOFROWSET - 1 more than in rowset
	int Variation_57();
	// @cmember OTHER SCENARIOS - DB_S_ENDOFROWSET - 1 more at end of rowset
	int Variation_58();
	// @cmember OTHER SCENARIOS - DB_S_ENDOFROWSET - Stepping backward
	int Variation_59();
	// @cmember Empty
	int Variation_60();
	// @cmember OTHER SCENARIOS - DB_E_BADSTARTPOSITION
	int Variation_61();
	// @cmember OTHER SCENARIOS - E_INVALIDARG
	int Variation_62();
	// @cmember OTHER SCENARIOS - EmptyRowset
	int Variation_63();
	// @cmember OTHER SCENARIOS - Remove Listeners
	int Variation_64();
	// @cmember OTHER SCENARIOS - Add Listeners
	int Variation_65();
	// @cmember OTHER SCENARIOS - GetData
	int Variation_66();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(GetNextRows)
#define THE_CLASS GetNextRows
BEG_TEST_CASE(GetNextRows, CExecutionManager, L"Tests Notifications for IRowset::GetNextRows")
	TEST_VARIATION(1, 		L"FETCHPOSITIONCHANGE - SINGLEROW - Accept all phases")
	TEST_VARIATION(2, 		L"FETCHPOSITIONCHANGE - MULTIPLEROWS - Accept all phases")
	TEST_VARIATION(3, 		L"Empty")
	TEST_VARIATION(4, 		L"Empty")
	TEST_VARIATION(5, 		L"FETCHPOSITIONCHANGE - DB_E_CANCELED")
	TEST_VARIATION(6, 		L"FETCHPOSITIONCHANGE - DB_E_NOTREENTRANT")
	TEST_VARIATION(7, 		L"FETCHPOSITIONCHANGE - DB_E_NOTREENTRANT - Non-notification method")
	TEST_VARIATION(8, 		L"Empty")
	TEST_VARIATION(9, 		L"FETCHPOSITIONCHANGE - Veto OKTODO, Listener 1")
	TEST_VARIATION(10, 		L"FETCHPOSITIONCHANGE - Veto ABOUTTODO, Listener 1")
	TEST_VARIATION(11, 		L"FETCHPOSITIONCHANGE - Veto SYNCHAFTER, Listener 1")
	TEST_VARIATION(12, 		L"FETCHPOSITIONCHANGE - Veto DIDEVENT, Listener 1")
	TEST_VARIATION(13, 		L"FETCHPOSITIONCHANGE - Veto FAILEDTODO, Listener 1")
	TEST_VARIATION(14, 		L"FETCHPOSITIONCHANGE - Veto OKTODO, Listener 2")
	TEST_VARIATION(15, 		L"FETCHPOSITIONCHANGE - Veto ABOUTTODO, Listener 2")
	TEST_VARIATION(16, 		L"FETCHPOSITIONCHANGE -  Veto SYNCHAFTER, Listener 3")
	TEST_VARIATION(17, 		L"Empty")
	TEST_VARIATION(18, 		L"Empty")
	TEST_VARIATION(19, 		L"FETCHPOSITIONCHANGE - UNWANTED REASON")
	TEST_VARIATION(20, 		L"FETCHPOSITIONCHANGE - UNWANTED PHASE - OKTODO")
	TEST_VARIATION(21, 		L"FETCHPOSITIONCHANGE - UNWANTED PHASE - ABOUTTODO")
	TEST_VARIATION(22, 		L"FETCHPOSITIONCHANGE - UNWANTED PHASE - SYNCHAFTER")
	TEST_VARIATION(23, 		L"FETCHPOSITIONCHANGE - UNWANTED PHASE - DIDENVENT")
	TEST_VARIATION(24, 		L"FETCHPOSITIONCHANGE - UNWANTED PHASE - FAILEDTODO")
	TEST_VARIATION(25, 		L"Empty")
	TEST_VARIATION(26, 		L"Empty")
	TEST_VARIATION(27, 		L"FETCHPOSITIONCHANGE - E_FAIL - All Phases")
	TEST_VARIATION(28, 		L"FETCHPOSITIONCHANGE - E_OUTOFMEMORY - All Phases")
	TEST_VARIATION(29, 		L"FETCHPOSITIONCHANGE - E_INVALIDARG - All Phases")
	TEST_VARIATION(30, 		L"Empty")
	TEST_VARIATION(31, 		L"Empty")
	TEST_VARIATION(32, 		L"ROW_ACTIVATE - SINGLEROW - Accept all phases")
	TEST_VARIATION(33, 		L"ROW_ACTIVATE - MULTIPLEROWS - Accept all phases")
	TEST_VARIATION(34, 		L"Empty")
	TEST_VARIATION(35, 		L"Empty")
	TEST_VARIATION(36, 		L"ROW_ACTIVATE - DB_E_CANCELED")
	TEST_VARIATION(37, 		L"ROW_ACTIVATE - DB_E_NOTREENTRANT")
	TEST_VARIATION(38, 		L"Empty")
	TEST_VARIATION(39, 		L"Empty")
	TEST_VARIATION(40, 		L"ROW_ACTIVATE - Veto DIDEVENT, Listener 1")
	TEST_VARIATION(41, 		L"Empty")
	TEST_VARIATION(42, 		L"Empty")
	TEST_VARIATION(43, 		L"ROW_ACTIVATE - UNWANTED REASON")
	TEST_VARIATION(44, 		L"ROW_ACTIVATE - UNWANTED PHASE - DIDEVENT")
	TEST_VARIATION(45, 		L"Empty")
	TEST_VARIATION(46, 		L"Empty")
	TEST_VARIATION(47, 		L"ROW_ACTIVATE - E_FAIL - All Phases")
	TEST_VARIATION(48, 		L"ROW_ACTIVATE - E_OUTOFMEMORY - All Phases")
	TEST_VARIATION(49, 		L"ROW_ACTIVATE - E_INVALIDARG - All Phases")
	TEST_VARIATION(50, 		L"Empty")
	TEST_VARIATION(51, 		L"Empty")
	TEST_VARIATION(52, 		L"OTHER SCENARIOS - Fetch no rows - no notifications")
	TEST_VARIATION(53, 		L"OTHER SCENARIOS - Fetch A, Fetch A again")
	TEST_VARIATION(54, 		L"OTHER SCENARIOS - Fetch A, Fetch A,B")
	TEST_VARIATION(55, 		L"OTHER SCENARIOS - Fetch A, ReleaseRows A, Fetch A")
	TEST_VARIATION(56, 		L"Empty")
	TEST_VARIATION(57, 		L"OTHER SCENARIOS - DB_S_ENDOFROWSET - 1 more than in rowset")
	TEST_VARIATION(58, 		L"OTHER SCENARIOS - DB_S_ENDOFROWSET - 1 more at end of rowset")
	TEST_VARIATION(59, 		L"OTHER SCENARIOS - DB_S_ENDOFROWSET - Stepping backward")
	TEST_VARIATION(60, 		L"Empty")
	TEST_VARIATION(61, 		L"OTHER SCENARIOS - DB_E_BADSTARTPOSITION")
	TEST_VARIATION(62, 		L"OTHER SCENARIOS - E_INVALIDARG")
	TEST_VARIATION(63, 		L"OTHER SCENARIOS - EmptyRowset")
	TEST_VARIATION(64, 		L"OTHER SCENARIOS - Remove Listeners")
	TEST_VARIATION(65, 		L"OTHER SCENARIOS - Add Listeners")
	TEST_VARIATION(66, 		L"OTHER SCENARIOS - GetData")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(ReleaseRows)
//--------------------------------------------------------------------
// @class Tests Notifications for IRowset::ReleaseRows
//
class ReleaseRows : public CExecutionManager { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(ReleaseRows,CExecutionManager);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember ROW_RELEASE - SINGLEROW - Accept all phases
	int Variation_1();
	// @cmember ROW_RELEASE - MULTIPLEROWS - Accept all phases
	int Variation_2();
	// @cmember Empty
	int Variation_3();
	// @cmember Empty
	int Variation_4();
	// @cmember ROW_RELEASE - DB_E_CANCELED - Accept all phases
	int Variation_5();
	// @cmember ROW_RELEASE - DB_E_NOTREENTRANT - Accept all phases
	int Variation_6();
	// @cmember Empty
	int Variation_7();
	// @cmember Empty
	int Variation_8();
	// @cmember ROW_RELEASE - Veto DIDEVENT, Listener 2
	int Variation_9();
	// @cmember Empty
	int Variation_10();
	// @cmember Empty
	int Variation_11();
	// @cmember ROW_RELEASE - UNWANTED REASON
	int Variation_12();
	// @cmember ROW_RELEASE - UNWANTED PHASE - DIDEVENT
	int Variation_13();
	// @cmember Empty
	int Variation_14();
	// @cmember Empty
	int Variation_15();
	// @cmember ROW_RELEASE - E_FAIL - All Phases
	int Variation_16();
	// @cmember ROW_RELEASE - E_OUTOFMEMORY - All Phases
	int Variation_17();
	// @cmember ROW_RELEASE - E_INVALIDARG - All Phases
	int Variation_18();
	// @cmember Empty
	int Variation_19();
	// @cmember Empty
	int Variation_20();
	// @cmember OTHER SCENARIOS - Release 0 rows
	int Variation_21();
	// @cmember OTHER SCENARIOS - AddRef Row, ReleaseRows
	int Variation_22();
	// @cmember OTHER SCENARIOS - ReleaseRow, ReleaseRow
	int Variation_23();
	// @cmember OTHER SCENARIOS - ReleaseRows [valid, invalid]
	int Variation_24();
	// @cmember Empty
	int Variation_25();
	// @cmember OTHER SCENARIOS - DBROWSTATUS_S_PENDINGCHANGES
	int Variation_26();
	// @cmember OTHER SCENARIOS - E_INVALIDARG
	int Variation_27();
	// @cmember OTHER SCENARIOS - EmptyRowset
	int Variation_28();
	// @cmember OTHER SCENARIOS - Add/Remove Listeners
	int Variation_29();
	// @cmember OTHER SCENARIOS - GetData
	int Variation_30();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(ReleaseRows)
#define THE_CLASS ReleaseRows
BEG_TEST_CASE(ReleaseRows, CExecutionManager, L"Tests Notifications for IRowset::ReleaseRows")
	TEST_VARIATION(1, 		L"ROW_RELEASE - SINGLEROW - Accept all phases")
	TEST_VARIATION(2, 		L"ROW_RELEASE - MULTIPLEROWS - Accept all phases")
	TEST_VARIATION(3, 		L"Empty")
	TEST_VARIATION(4, 		L"Empty")
	TEST_VARIATION(5, 		L"ROW_RELEASE - DB_E_CANCELED - Accept all phases")
	TEST_VARIATION(6, 		L"ROW_RELEASE - DB_E_NOTREENTRANT - Accept all phases")
	TEST_VARIATION(7, 		L"Empty")
	TEST_VARIATION(8, 		L"Empty")
	TEST_VARIATION(9, 		L"ROW_RELEASE - Veto DIDEVENT, Listener 2")
	TEST_VARIATION(10, 		L"Empty")
	TEST_VARIATION(11, 		L"Empty")
	TEST_VARIATION(12, 		L"ROW_RELEASE - UNWANTED REASON")
	TEST_VARIATION(13, 		L"ROW_RELEASE - UNWANTED PHASE - DIDEVENT")
	TEST_VARIATION(14, 		L"Empty")
	TEST_VARIATION(15, 		L"Empty")
	TEST_VARIATION(16, 		L"ROW_RELEASE - E_FAIL - All Phases")
	TEST_VARIATION(17, 		L"ROW_RELEASE - E_OUTOFMEMORY - All Phases")
	TEST_VARIATION(18, 		L"ROW_RELEASE - E_INVALIDARG - All Phases")
	TEST_VARIATION(19, 		L"Empty")
	TEST_VARIATION(20, 		L"Empty")
	TEST_VARIATION(21, 		L"OTHER SCENARIOS - Release 0 rows")
	TEST_VARIATION(22, 		L"OTHER SCENARIOS - AddRef Row, ReleaseRows")
	TEST_VARIATION(23, 		L"OTHER SCENARIOS - ReleaseRow, ReleaseRow")
	TEST_VARIATION(24, 		L"OTHER SCENARIOS - ReleaseRows [valid, invalid]")
	TEST_VARIATION(25, 		L"Empty")
	TEST_VARIATION(26, 		L"OTHER SCENARIOS - DBROWSTATUS_S_PENDINGCHANGES")
	TEST_VARIATION(27, 		L"OTHER SCENARIOS - E_INVALIDARG")
	TEST_VARIATION(28, 		L"OTHER SCENARIOS - EmptyRowset")
	TEST_VARIATION(29, 		L"OTHER SCENARIOS - Add/Remove Listeners")
	TEST_VARIATION(30, 		L"OTHER SCENARIOS - GetData")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(RestartPosition)
//--------------------------------------------------------------------
// @class Tests Notificatons for IRowset::RestartPosition
//
class RestartPosition : public CExecutionManager { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(RestartPosition,CExecutionManager);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember FETCHPOSITIONCHANGE - Accept all phases
	int Variation_1();
	// @cmember Empty
	int Variation_2();
	// @cmember Empty
	int Variation_3();
	// @cmember FETCHPOSITIONCHANGE - DB_E_CANCELED
	int Variation_4();
	// @cmember FETCHPOSITIONCHANGE - DB_E_NOTREENTRANT
	int Variation_5();
	// @cmember Empty
	int Variation_6();
	// @cmember Empty
	int Variation_7();
	// @cmember FETCHPOSITIONCHANGE - Veto OKTODO
	int Variation_8();
	// @cmember FETCHPOSITIONCHANGE - Veto ABOUTTODO
	int Variation_9();
	// @cmember FETCHPOSITIONCHANGE - Veto SYNCHAFTER
	int Variation_10();
	// @cmember FETCHPOSITIONCHANGE - Veto DIDEVENT
	int Variation_11();
	// @cmember FETCHPOSITIONCHANGE - Veto FAILEDTODO
	int Variation_12();
	// @cmember Empty
	int Variation_13();
	// @cmember Empty
	int Variation_14();
	// @cmember FETCHPOSITIONCHANGE - UNWANTED REASON
	int Variation_15();
	// @cmember FETCHPOSITIONCHANGE - UNWANTED PHASE - OKTODO
	int Variation_16();
	// @cmember FETCHPOSITIONCHANGE - UNWANTED PHASE - ABOUTTODO
	int Variation_17();
	// @cmember FETCHPOSITIONCHANGE - UNWANTED PHASE - SYNCHAFTER
	int Variation_18();
	// @cmember FETCHPOSITIONCHANGE - UNWANTED PHASE - DIDEVENT
	int Variation_19();
	// @cmember FETCHPOSITIONCHANGE - UNWANTED PHASE - FAILEDTODO
	int Variation_20();
	// @cmember Empty
	int Variation_21();
	// @cmember Empty
	int Variation_22();
	// @cmember FETCHPOSITIONCHANGE - E_FAIL - All Phases
	int Variation_23();
	// @cmember FETCHPOSITIONCHANGE - E_OUTOFMEMORY - All Phases
	int Variation_24();
	// @cmember FETCHPOSITIONCHANGE - E_INVALIDARG - All Phases
	int Variation_25();
	// @cmember Empty
	int Variation_26();
	// @cmember Empty
	int Variation_27();
	// @cmember ROWSET_CHANGED - Accept all phases
	int Variation_28();
	// @cmember Empty
	int Variation_29();
	// @cmember Empty
	int Variation_30();
	// @cmember ROWSET_CHANGED - DB_E_CANCELED
	int Variation_31();
	// @cmember ROWSET_CHANGED - DB_E_NOTREENTRANT
	int Variation_32();
	// @cmember Empty
	int Variation_33();
	// @cmember Empty
	int Variation_34();
	// @cmember ROWSET_CHANGED - Veto OKTODO
	int Variation_35();
	// @cmember ROWSET_CHANGED - Veto ABOUTTODO
	int Variation_36();
	// @cmember ROWSET_CHANGED - Veto SYNCHAFTER
	int Variation_37();
	// @cmember ROWSET_CHANGED - Veto DIDEVENT
	int Variation_38();
	// @cmember ROWSET_CHANGED - Veto FAILEDTODO
	int Variation_39();
	// @cmember Empty
	int Variation_40();
	// @cmember Empty
	int Variation_41();
	// @cmember ROWSET_CHANGED - UNWANTED REASON
	int Variation_42();
	// @cmember ROWSET_CHANGED - UNWANTED PHASE OKTODO
	int Variation_43();
	// @cmember ROWSET_CHANGED - UNWANTED PHASE ABOUTTODO
	int Variation_44();
	// @cmember ROWSET_CHANGED - UNWANTED PHASE SYNCHAFTER
	int Variation_45();
	// @cmember ROWSET_CHANGED - UNWANTED PHASE DIDEVENT
	int Variation_46();
	// @cmember ROWSET_CHANGED - UNWANTED PHASE FAILEDTODO
	int Variation_47();
	// @cmember Empty
	int Variation_48();
	// @cmember Empty
	int Variation_49();
	// @cmember ROWSET_CHANGED - E_FAIL - All Phases
	int Variation_50();
	// @cmember ROWSET_CHANGED - E_OUTOFMEMORY - All Phases
	int Variation_51();
	// @cmember ROWSET_CHANGED - E_INVALIDARG - All Phases
	int Variation_52();
	// @cmember Empty
	int Variation_53();
	// @cmember Empty
	int Variation_54();
	// @cmember OTHER SCENARIOS - RestartPosition
	int Variation_55();
	// @cmember OTHER SCENARIOS - GetNextRows - RestartPosition - RestartPosition
	int Variation_56();
	// @cmember Empty
	int Variation_57();
	// @cmember OTHER SCENARIOS - DB_E_ROWSNOTRELEASED
	int Variation_58();
	// @cmember OTHER SCENARIOS - EmptyRowset
	int Variation_59();
	// @cmember OTHER SCENARIOS - Add/Remove Listeners
	int Variation_60();
	// @cmember OTHER SCENARIOS - GetData
	int Variation_61();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(RestartPosition)
#define THE_CLASS RestartPosition
BEG_TEST_CASE(RestartPosition, CExecutionManager, L"Tests Notificatons for IRowset::RestartPosition")
	TEST_VARIATION(1, 		L"FETCHPOSITIONCHANGE - Accept all phases")
	TEST_VARIATION(2, 		L"Empty")
	TEST_VARIATION(3, 		L"Empty")
	TEST_VARIATION(4, 		L"FETCHPOSITIONCHANGE - DB_E_CANCELED")
	TEST_VARIATION(5, 		L"FETCHPOSITIONCHANGE - DB_E_NOTREENTRANT")
	TEST_VARIATION(6, 		L"Empty")
	TEST_VARIATION(7, 		L"Empty")
	TEST_VARIATION(8, 		L"FETCHPOSITIONCHANGE - Veto OKTODO")
	TEST_VARIATION(9, 		L"FETCHPOSITIONCHANGE - Veto ABOUTTODO")
	TEST_VARIATION(10, 		L"FETCHPOSITIONCHANGE - Veto SYNCHAFTER")
	TEST_VARIATION(11, 		L"FETCHPOSITIONCHANGE - Veto DIDEVENT")
	TEST_VARIATION(12, 		L"FETCHPOSITIONCHANGE - Veto FAILEDTODO")
	TEST_VARIATION(13, 		L"Empty")
	TEST_VARIATION(14, 		L"Empty")
	TEST_VARIATION(15, 		L"FETCHPOSITIONCHANGE - UNWANTED REASON")
	TEST_VARIATION(16, 		L"FETCHPOSITIONCHANGE - UNWANTED PHASE - OKTODO")
	TEST_VARIATION(17, 		L"FETCHPOSITIONCHANGE - UNWANTED PHASE - ABOUTTODO")
	TEST_VARIATION(18, 		L"FETCHPOSITIONCHANGE - UNWANTED PHASE - SYNCHAFTER")
	TEST_VARIATION(19, 		L"FETCHPOSITIONCHANGE - UNWANTED PHASE - DIDEVENT")
	TEST_VARIATION(20, 		L"FETCHPOSITIONCHANGE - UNWANTED PHASE - FAILEDTODO")
	TEST_VARIATION(21, 		L"Empty")
	TEST_VARIATION(22, 		L"Empty")
	TEST_VARIATION(23, 		L"FETCHPOSITIONCHANGE - E_FAIL - All Phases")
	TEST_VARIATION(24, 		L"FETCHPOSITIONCHANGE - E_OUTOFMEMORY - All Phases")
	TEST_VARIATION(25, 		L"FETCHPOSITIONCHANGE - E_INVALIDARG - All Phases")
	TEST_VARIATION(26, 		L"Empty")
	TEST_VARIATION(27, 		L"Empty")
	TEST_VARIATION(28, 		L"ROWSET_CHANGED - Accept all phases")
	TEST_VARIATION(29, 		L"Empty")
	TEST_VARIATION(30, 		L"Empty")
	TEST_VARIATION(31, 		L"ROWSET_CHANGED - DB_E_CANCELED")
	TEST_VARIATION(32, 		L"ROWSET_CHANGED - DB_E_NOTREENTRANT")
	TEST_VARIATION(33, 		L"Empty")
	TEST_VARIATION(34, 		L"Empty")
	TEST_VARIATION(35, 		L"ROWSET_CHANGED - Veto OKTODO")
	TEST_VARIATION(36, 		L"ROWSET_CHANGED - Veto ABOUTTODO")
	TEST_VARIATION(37, 		L"ROWSET_CHANGED - Veto SYNCHAFTER")
	TEST_VARIATION(38, 		L"ROWSET_CHANGED - Veto DIDEVENT")
	TEST_VARIATION(39, 		L"ROWSET_CHANGED - Veto FAILEDTODO")
	TEST_VARIATION(40, 		L"Empty")
	TEST_VARIATION(41, 		L"Empty")
	TEST_VARIATION(42, 		L"ROWSET_CHANGED - UNWANTED REASON")
	TEST_VARIATION(43, 		L"ROWSET_CHANGED - UNWANTED PHASE OKTODO")
	TEST_VARIATION(44, 		L"ROWSET_CHANGED - UNWANTED PHASE ABOUTTODO")
	TEST_VARIATION(45, 		L"ROWSET_CHANGED - UNWANTED PHASE SYNCHAFTER")
	TEST_VARIATION(46, 		L"ROWSET_CHANGED - UNWANTED PHASE DIDEVENT")
	TEST_VARIATION(47, 		L"ROWSET_CHANGED - UNWANTED PHASE FAILEDTODO")
	TEST_VARIATION(48, 		L"Empty")
	TEST_VARIATION(49, 		L"Empty")
	TEST_VARIATION(50, 		L"ROWSET_CHANGED - E_FAIL - All Phases")
	TEST_VARIATION(51, 		L"ROWSET_CHANGED - E_OUTOFMEMORY - All Phases")
	TEST_VARIATION(52, 		L"ROWSET_CHANGED - E_INVALIDARG - All Phases")
	TEST_VARIATION(53, 		L"Empty")
	TEST_VARIATION(54, 		L"Empty")
	TEST_VARIATION(55, 		L"OTHER SCENARIOS - RestartPosition")
	TEST_VARIATION(56, 		L"OTHER SCENARIOS - GetNextRows - RestartPosition - RestartPosition")
	TEST_VARIATION(57, 		L"Empty")
	TEST_VARIATION(58, 		L"OTHER SCENARIOS - DB_E_ROWSNOTRELEASED")
	TEST_VARIATION(59, 		L"OTHER SCENARIOS - EmptyRowset")
	TEST_VARIATION(60, 		L"OTHER SCENARIOS - Add/Remove Listeners")
	TEST_VARIATION(61, 		L"OTHER SCENARIOS - GetData")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(Seek)
//--------------------------------------------------------------------
// @class Test Notifications for IRowsetIndex::Seek
//
class Seek : public CExecutionManager { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Seek,CExecutionManager);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember FETCHPOSITIONCHANGE - Accept all phases
	int Variation_1();
	// @cmember Empty
	int Variation_2();
	// @cmember Empty
	int Variation_3();
	// @cmember FETCHPOSITIONCHANGE - DB_E_CANCELED
	int Variation_4();
	// @cmember FETCHPOSITIONCHANGE - DB_E_NOTREENTRANT
	int Variation_5();
	// @cmember Empty
	int Variation_6();
	// @cmember Empty
	int Variation_7();
	// @cmember FETCHPOSITIONCHANGE - Veto OKTODO
	int Variation_8();
	// @cmember FETCHPOSITIONCHANGE - Veto ABOUTTODO
	int Variation_9();
	// @cmember FETCHPOSITIONCHANGE - Veto SYNCHAFTER
	int Variation_10();
	// @cmember FETCHPOSITIONCHANGE - Veto DIDEVENT
	int Variation_11();
	// @cmember FETCHPOSITIONCHANGE - Veto FAILEDTODO
	int Variation_12();
	// @cmember Empty
	int Variation_13();
	// @cmember Empty
	int Variation_14();
	// @cmember FETCHPOSITIONCHANGE - UNWANTED REASON
	int Variation_15();
	// @cmember FETCHPOSITIONCHANGE - UNWANTED PHASE - OKTODO
	int Variation_16();
	// @cmember FETCHPOSITIONCHANGE - UNWANTED PHASE - ABOUTTODO
	int Variation_17();
	// @cmember FETCHPOSITIONCHANGE - UNWANTED PHASE - SYNCHAFTER
	int Variation_18();
	// @cmember FETCHPOSITIONCHANGE - UNWANTED PHASE - DIDEVENT
	int Variation_19();
	// @cmember FETCHPOSITIONCHANGE - UNWANTED PHASE - FAILEDTODO
	int Variation_20();
	// @cmember Empty
	int Variation_21();
	// @cmember Empty
	int Variation_22();
	// @cmember FETCHPOSITIONCHANGE - E_FAIL - All Phases
	int Variation_23();
	// @cmember FETCHPOSITIONCHANGE - E_OUTOFMEMORY - All Phases
	int Variation_24();
	// @cmember FETCHPOSITIONCHANGE - E_INVALIDARG - All Phases
	int Variation_25();
	// @cmember Empty
	int Variation_26();
	// @cmember Empty
	int Variation_27();
	// @cmember OTHER SCENARIOS - DB_E_BADACCESSORHANDLE
	int Variation_28();
	// @cmember OTHER SCENARIOS - EmptyRowset
	int Variation_29();
	// @cmember OTHER SCENARIOS - Add/Remove Listeners
	int Variation_30();
	// @cmember OTHER SCENARIOS - GetData
	int Variation_31();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(Seek)
#define THE_CLASS Seek
BEG_TEST_CASE(Seek, CExecutionManager, L"Test Notifications for IRowsetIndex::Seek")
	TEST_VARIATION(1, 		L"FETCHPOSITIONCHANGE - Accept all phases")
	TEST_VARIATION(2, 		L"Empty")
	TEST_VARIATION(3, 		L"Empty")
	TEST_VARIATION(4, 		L"FETCHPOSITIONCHANGE - DB_E_CANCELED")
	TEST_VARIATION(5, 		L"FETCHPOSITIONCHANGE - DB_E_NOTREENTRANT")
	TEST_VARIATION(6, 		L"Empty")
	TEST_VARIATION(7, 		L"Empty")
	TEST_VARIATION(8, 		L"FETCHPOSITIONCHANGE - Veto OKTODO")
	TEST_VARIATION(9, 		L"FETCHPOSITIONCHANGE - Veto ABOUTTODO")
	TEST_VARIATION(10, 		L"FETCHPOSITIONCHANGE - Veto SYNCHAFTER")
	TEST_VARIATION(11, 		L"FETCHPOSITIONCHANGE - Veto DIDEVENT")
	TEST_VARIATION(12, 		L"FETCHPOSITIONCHANGE - Veto FAILEDTODO")
	TEST_VARIATION(13, 		L"Empty")
	TEST_VARIATION(14, 		L"Empty")
	TEST_VARIATION(15, 		L"FETCHPOSITIONCHANGE - UNWANTED REASON")
	TEST_VARIATION(16, 		L"FETCHPOSITIONCHANGE - UNWANTED PHASE - OKTODO")
	TEST_VARIATION(17, 		L"FETCHPOSITIONCHANGE - UNWANTED PHASE - ABOUTTODO")
	TEST_VARIATION(18, 		L"FETCHPOSITIONCHANGE - UNWANTED PHASE - SYNCHAFTER")
	TEST_VARIATION(19, 		L"FETCHPOSITIONCHANGE - UNWANTED PHASE - DIDEVENT")
	TEST_VARIATION(20, 		L"FETCHPOSITIONCHANGE - UNWANTED PHASE - FAILEDTODO")
	TEST_VARIATION(21, 		L"Empty")
	TEST_VARIATION(22, 		L"Empty")
	TEST_VARIATION(23, 		L"FETCHPOSITIONCHANGE - E_FAIL - All Phases")
	TEST_VARIATION(24, 		L"FETCHPOSITIONCHANGE - E_OUTOFMEMORY - All Phases")
	TEST_VARIATION(25, 		L"FETCHPOSITIONCHANGE - E_INVALIDARG - All Phases")
	TEST_VARIATION(26, 		L"Empty")
	TEST_VARIATION(27, 		L"Empty")
	TEST_VARIATION(28, 		L"OTHER SCENARIOS - DB_E_BADACCESSORHANDLE")
	TEST_VARIATION(29, 		L"OTHER SCENARIOS - EmptyRowset")
	TEST_VARIATION(30, 		L"OTHER SCENARIOS - Add/Remove Listeners")
	TEST_VARIATION(31, 		L"OTHER SCENARIOS - GetData")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(GetRowsAt)
//--------------------------------------------------------------------
// @class Tests Notificatons for IRowsetLocate::GetRowsAt
//
class GetRowsAt : public CExecutionManager { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(GetRowsAt,CExecutionManager);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember ROW_ACTIVATE - SINGLEROW - Accept all phases
	int Variation_1();
	// @cmember ROW_ACTIVATE - MULTIPLEROWS - Accept all phases
	int Variation_2();
	// @cmember Empty
	int Variation_3();
	// @cmember Empty
	int Variation_4();
	// @cmember ROW_ACTIVATE - DB_E_CANCELED
	int Variation_5();
	// @cmember ROW_ACTIVATE - DB_E_NOTREENTRANT
	int Variation_6();
	// @cmember Empty
	int Variation_7();
	// @cmember Empty
	int Variation_8();
	// @cmember ROW_ACTIVATE - Veto DIDEVENT, Listener 1
	int Variation_9();
	// @cmember Empty
	int Variation_10();
	// @cmember Empty
	int Variation_11();
	// @cmember ROW_ACTIVATE - UNWANTED REASON
	int Variation_12();
	// @cmember ROW_ACTIVATE - UNWANTED PHASE - DIDEVENT
	int Variation_13();
	// @cmember Empty
	int Variation_14();
	// @cmember Empty
	int Variation_15();
	// @cmember ROW_ACTIVATE - E_FAIL - All Phases
	int Variation_16();
	// @cmember ROW_ACTIVATE - E_OUTOFMEMORY - All Phases
	int Variation_17();
	// @cmember ROW_ACTIVATE - E_INVALIDARG - All Phases
	int Variation_18();
	// @cmember Empty
	int Variation_19();
	// @cmember Empty
	int Variation_20();
	// @cmember OTHER SCENARIOS - Get same row twice
	int Variation_21();
	// @cmember OTHER SCENARIOS - DB_E_BADSTARTPOSITION
	int Variation_22();
	// @cmember OTHER SCENARIOS - EmptyRowset
	int Variation_23();
	// @cmember OTHER SCENARIOS - DB_S_ENDOFROWSET - more rows than are in the rowset
	int Variation_24();
	// @cmember OTHER SCENARIOS - DB_S_ENDOFROWSET - at end of rowset
	int Variation_25();
	// @cmember OTHER SCENARIOS -  no rows at head of rowset
	int Variation_26();
	// @cmember OTHER SCENARIOS -  no rows at end of rowset
	int Variation_27();
	// @cmember OTHER SCENARIOS - Add/Remove Listeners
	int Variation_28();
	// @cmember OTHER SCENARIOS - GetData
	int Variation_29();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(GetRowsAt)
#define THE_CLASS GetRowsAt
BEG_TEST_CASE(GetRowsAt, CExecutionManager, L"Tests Notificatons for IRowsetLocate::GetRowsAt")
	TEST_VARIATION(1, 		L"ROW_ACTIVATE - SINGLEROW - Accept all phases")
	TEST_VARIATION(2, 		L"ROW_ACTIVATE - MULTIPLEROWS - Accept all phases")
	TEST_VARIATION(3, 		L"Empty")
	TEST_VARIATION(4, 		L"Empty")
	TEST_VARIATION(5, 		L"ROW_ACTIVATE - DB_E_CANCELED")
	TEST_VARIATION(6, 		L"ROW_ACTIVATE - DB_E_NOTREENTRANT")
	TEST_VARIATION(7, 		L"Empty")
	TEST_VARIATION(8, 		L"Empty")
	TEST_VARIATION(9, 		L"ROW_ACTIVATE - Veto DIDEVENT, Listener 1")
	TEST_VARIATION(10, 		L"Empty")
	TEST_VARIATION(11, 		L"Empty")
	TEST_VARIATION(12, 		L"ROW_ACTIVATE - UNWANTED REASON")
	TEST_VARIATION(13, 		L"ROW_ACTIVATE - UNWANTED PHASE - DIDEVENT")
	TEST_VARIATION(14, 		L"Empty")
	TEST_VARIATION(15, 		L"Empty")
	TEST_VARIATION(16, 		L"ROW_ACTIVATE - E_FAIL - All Phases")
	TEST_VARIATION(17, 		L"ROW_ACTIVATE - E_OUTOFMEMORY - All Phases")
	TEST_VARIATION(18, 		L"ROW_ACTIVATE - E_INVALIDARG - All Phases")
	TEST_VARIATION(19, 		L"Empty")
	TEST_VARIATION(20, 		L"Empty")
	TEST_VARIATION(21, 		L"OTHER SCENARIOS - Get same row twice")
	TEST_VARIATION(22, 		L"OTHER SCENARIOS - DB_E_BADSTARTPOSITION")
	TEST_VARIATION(23, 		L"OTHER SCENARIOS - EmptyRowset")
	TEST_VARIATION(24, 		L"OTHER SCENARIOS - DB_S_ENDOFROWSET - more rows than are in the rowset")
	TEST_VARIATION(25, 		L"OTHER SCENARIOS - DB_S_ENDOFROWSET - at end of rowset")
	TEST_VARIATION(26, 		L"OTHER SCENARIOS -  no rows at head of rowset")
	TEST_VARIATION(27, 		L"OTHER SCENARIOS -  no rows at end of rowset")
	TEST_VARIATION(28, 		L"OTHER SCENARIOS - Add/Remove Listeners")
	TEST_VARIATION(29, 		L"OTHER SCENARIOS - GetData")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(GetRowsByBookmark)
//--------------------------------------------------------------------
// @class Tests Notifications for IRowsetLocate::GetRowsbyBookmark
//
class GetRowsByBookmark : public CExecutionManager { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(GetRowsByBookmark,CExecutionManager);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember ROW_ACTIVATE - SINGLEROW - Accept all phases
	int Variation_1();
	// @cmember ROW_ACTIVATE - MULTIPLEROWS - Accept all phases
	int Variation_2();
	// @cmember Empty
	int Variation_3();
	// @cmember Empty
	int Variation_4();
	// @cmember ROW_ACTIVATE - DB_E_CANCELED
	int Variation_5();
	// @cmember ROW_ACTIVATE - DB_E_NOTREENTRANT
	int Variation_6();
	// @cmember Empty
	int Variation_7();
	// @cmember Empty
	int Variation_8();
	// @cmember ROW_ACTIVATE - Veto DIDEVENT, Listener 1
	int Variation_9();
	// @cmember Empty
	int Variation_10();
	// @cmember Empty
	int Variation_11();
	// @cmember ROW_ACTIVATE - UNWANTED REASON
	int Variation_12();
	// @cmember ROW_ACTIVATE - UNWANTED PHASE - DIDEVENT
	int Variation_13();
	// @cmember Empty
	int Variation_14();
	// @cmember Empty
	int Variation_15();
	// @cmember ROW_ACTIVATE - E_FAIL - All Phases
	int Variation_16();
	// @cmember ROW_ACTIVATE - E_OUTOFMEMORY - All Phases
	int Variation_17();
	// @cmember ROW_ACTIVATE - E_INVALIDARG - All Phases
	int Variation_18();
	// @cmember Empty
	int Variation_19();
	// @cmember Empty
	int Variation_20();
	// @cmember OTHER SCENARIOS - Get no rows
	int Variation_21();
	// @cmember OTHER SCENARIOS - Get same row twice
	int Variation_22();
	// @cmember OTHER SCENARIOS - E_INVALIDARG
	int Variation_23();
	// @cmember OTHER SCENARIOS - EmptyRowset
	int Variation_24();
	// @cmember OTHER SCENARIOS - Add/Remove Listeners
	int Variation_25();
	// @cmember OTHER SCENARIOS - GetData
	int Variation_26();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(GetRowsByBookmark)
#define THE_CLASS GetRowsByBookmark
BEG_TEST_CASE(GetRowsByBookmark, CExecutionManager, L"Tests Notifications for IRowsetLocate::GetRowsbyBookmark")
	TEST_VARIATION(1, 		L"ROW_ACTIVATE - SINGLEROW - Accept all phases")
	TEST_VARIATION(2, 		L"ROW_ACTIVATE - MULTIPLEROWS - Accept all phases")
	TEST_VARIATION(3, 		L"Empty")
	TEST_VARIATION(4, 		L"Empty")
	TEST_VARIATION(5, 		L"ROW_ACTIVATE - DB_E_CANCELED")
	TEST_VARIATION(6, 		L"ROW_ACTIVATE - DB_E_NOTREENTRANT")
	TEST_VARIATION(7, 		L"Empty")
	TEST_VARIATION(8, 		L"Empty")
	TEST_VARIATION(9, 		L"ROW_ACTIVATE - Veto DIDEVENT, Listener 1")
	TEST_VARIATION(10, 		L"Empty")
	TEST_VARIATION(11, 		L"Empty")
	TEST_VARIATION(12, 		L"ROW_ACTIVATE - UNWANTED REASON")
	TEST_VARIATION(13, 		L"ROW_ACTIVATE - UNWANTED PHASE - DIDEVENT")
	TEST_VARIATION(14, 		L"Empty")
	TEST_VARIATION(15, 		L"Empty")
	TEST_VARIATION(16, 		L"ROW_ACTIVATE - E_FAIL - All Phases")
	TEST_VARIATION(17, 		L"ROW_ACTIVATE - E_OUTOFMEMORY - All Phases")
	TEST_VARIATION(18, 		L"ROW_ACTIVATE - E_INVALIDARG - All Phases")
	TEST_VARIATION(19, 		L"Empty")
	TEST_VARIATION(20, 		L"Empty")
	TEST_VARIATION(21, 		L"OTHER SCENARIOS - Get no rows")
	TEST_VARIATION(22, 		L"OTHER SCENARIOS - Get same row twice")
	TEST_VARIATION(23, 		L"OTHER SCENARIOS - E_INVALIDARG")
	TEST_VARIATION(24, 		L"OTHER SCENARIOS - EmptyRowset")
	TEST_VARIATION(25, 		L"OTHER SCENARIOS - Add/Remove Listeners")
	TEST_VARIATION(26, 		L"OTHER SCENARIOS - GetData")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(GetRowsAtRatio)
//--------------------------------------------------------------------
// @class Test Nofications for IRowsetScroll::GetRowsAtRatio
//
class GetRowsAtRatio : public CExecutionManager { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(GetRowsAtRatio,CExecutionManager);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember ROW_ACTIVATE - SINGLEROW - Accept all phases
	int Variation_1();
	// @cmember ROW_ACTIVATE - MULTIPLEROWS - Accept all phases
	int Variation_2();
	// @cmember Empty
	int Variation_3();
	// @cmember Empty
	int Variation_4();
	// @cmember ROW_ACTIVATE - DB_E_CANCELED
	int Variation_5();
	// @cmember ROW_ACTIVATE - DB_E_NOTREENTRANT
	int Variation_6();
	// @cmember Empty
	int Variation_7();
	// @cmember Empty
	int Variation_8();
	// @cmember ROW_ACTIVATE - Veto DIDEVENT, Listener 1
	int Variation_9();
	// @cmember Empty
	int Variation_10();
	// @cmember Empty
	int Variation_11();
	// @cmember ROW_ACTIVATE - UNWANTED REASON
	int Variation_12();
	// @cmember ROW_ACTIVATE - UNWANTED PHASE - DIDEVENT
	int Variation_13();
	// @cmember Empty
	int Variation_14();
	// @cmember Empty
	int Variation_15();
	// @cmember ROW_ACTIVATE - E_FAIL - All Phases
	int Variation_16();
	// @cmember ROW_ACTIVATE - E_OUTOFMEMORY - All Phases
	int Variation_17();
	// @cmember ROW_ACTIVATE - E_INVALIDARG - All Phases
	int Variation_18();
	// @cmember Empty
	int Variation_19();
	// @cmember Empty
	int Variation_20();
	// @cmember OTHER SCENARIOS - No rows
	int Variation_21();
	// @cmember OTHER SCENARIOS - DB_S_ENDOFROWSET 1 more than rowset
	int Variation_22();
	// @cmember OTHER SCENARIOS - DB_S_ENDOFROWSET - at end of rowset
	int Variation_23();
	// @cmember OTHER SCENARIOS - EmptyRowset
	int Variation_24();
	// @cmember OTHER SCENARIOS - Add/Remove Listeners
	int Variation_25();
	// @cmember OTHER SCENARIOS - GetData
	int Variation_26();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(GetRowsAtRatio)
#define THE_CLASS GetRowsAtRatio
BEG_TEST_CASE(GetRowsAtRatio, CExecutionManager, L"Test Nofications for IRowsetScroll::GetRowsAtRatio")
	TEST_VARIATION(1, 		L"ROW_ACTIVATE - SINGLEROW - Accept all phases")
	TEST_VARIATION(2, 		L"ROW_ACTIVATE - MULTIPLEROWS - Accept all phases")
	TEST_VARIATION(3, 		L"Empty")
	TEST_VARIATION(4, 		L"Empty")
	TEST_VARIATION(5, 		L"ROW_ACTIVATE - DB_E_CANCELED")
	TEST_VARIATION(6, 		L"ROW_ACTIVATE - DB_E_NOTREENTRANT")
	TEST_VARIATION(7, 		L"Empty")
	TEST_VARIATION(8, 		L"Empty")
	TEST_VARIATION(9, 		L"ROW_ACTIVATE - Veto DIDEVENT, Listener 1")
	TEST_VARIATION(10, 		L"Empty")
	TEST_VARIATION(11, 		L"Empty")
	TEST_VARIATION(12, 		L"ROW_ACTIVATE - UNWANTED REASON")
	TEST_VARIATION(13, 		L"ROW_ACTIVATE - UNWANTED PHASE - DIDEVENT")
	TEST_VARIATION(14, 		L"Empty")
	TEST_VARIATION(15, 		L"Empty")
	TEST_VARIATION(16, 		L"ROW_ACTIVATE - E_FAIL - All Phases")
	TEST_VARIATION(17, 		L"ROW_ACTIVATE - E_OUTOFMEMORY - All Phases")
	TEST_VARIATION(18, 		L"ROW_ACTIVATE - E_INVALIDARG - All Phases")
	TEST_VARIATION(19, 		L"Empty")
	TEST_VARIATION(20, 		L"Empty")
	TEST_VARIATION(21, 		L"OTHER SCENARIOS - No rows")
	TEST_VARIATION(22, 		L"OTHER SCENARIOS - DB_S_ENDOFROWSET 1 more than rowset")
	TEST_VARIATION(23, 		L"OTHER SCENARIOS - DB_S_ENDOFROWSET - at end of rowset")
	TEST_VARIATION(24, 		L"OTHER SCENARIOS - EmptyRowset")
	TEST_VARIATION(25, 		L"OTHER SCENARIOS - Add/Remove Listeners")
	TEST_VARIATION(26, 		L"OTHER SCENARIOS - GetData")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(DeleteRows)
//--------------------------------------------------------------------
// @class Tests Notifications for IRowsetChange::DeleteRows
//
class DeleteRows : public CExecutionManager { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(DeleteRows,CExecutionManager);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember ROW_DELETE - SINGLEROW - Accept all phases
	int Variation_1();
	// @cmember ROW_DELETE - MULTIPLEROWS - Accept all phases
	int Variation_2();
	// @cmember Empty
	int Variation_3();
	// @cmember ROW_DELETE - MULTIPLEROWS - DB_E_CANCELED
	int Variation_4();
	// @cmember ROW_DELETE - SINGLEROW - DB_E_NOTREENTRANT
	int Variation_5();
	// @cmember Empty
	int Variation_6();
	// @cmember Empty
	int Variation_7();
	// @cmember ROW_DELETE - MULTIPLEROWS - Veto OKTODO Listener 1
	int Variation_8();
	// @cmember ROW_DELETE - MULTIPLEROWS - Veto ABOUTTODO Listener 1
	int Variation_9();
	// @cmember ROW_DELETE - MULTIPLEROWS - Veto SYNCHAFTER Listener 1
	int Variation_10();
	// @cmember ROW_DELETE - MULTIPLEROWS - Veto DIDEVENT Listener 2
	int Variation_11();
	// @cmember ROW_DELETE - MULTIPLEROWS - Veto ABOUTTODO Listener 1 - Veto FAILEDTODO Listener 2
	int Variation_12();
	// @cmember OTHER SCENARIOS - Add/Remove Listeners - SINGLEROW
	int Variation_13();
	// @cmember OTHER SCENARIOS - GetData - SINGLEROW
	int Variation_14();
	// @cmember ROW_DELETE - SINGLEROW - UNWANTED REASON
	int Variation_15();
	// @cmember ROW_DELETE - SINGLEROW - UNWANTED PHASE OKTODO
	int Variation_16();
	// @cmember ROW_DELETE - SINGLEROW - UNWANTED PHASE ABOUTTODO
	int Variation_17();
	// @cmember ROW_DELETE - SINGLEROW - UNWANTED PHASE SYNCHAFTER
	int Variation_18();
	// @cmember ROW_DELETE - SINGLEROW - UNWANTED PHASE DIDEVENT
	int Variation_19();
	// @cmember ROW_DELETE - MULTIPLEROWS - Veto ABOUTTODO Listener 1 - UNWANTED PHASE FAILEDTODO
	int Variation_20();
	// @cmember OTHER SCENARIOS - DBROWSTATUS_DELETED - SINGLEROW
	int Variation_21();
	// @cmember ROW_DELETE - SINGLEROW - Veto ABOUTTODO Listener 1 - E_FAIL - All Phases
	int Variation_22();
	// @cmember ROW_DELETE - MULTIPLEROWS - Veto ABOUTTODO Listener 1 - E_FAIL - All Phases
	int Variation_23();
	// @cmember ROW_DELETE - MULTIPLEROWS - Veto ABOUTTODO Listener 1 - E_OUTOFMEMORY - All Phases
	int Variation_24();
	// @cmember ROW_DELETE - MULTIPLEROWS - Veto ABOUTTODO Listener 1 - E_INVALIDARG - All Phases
	int Variation_25();
	// @cmember ROW_DELETE - SINGLEROW - E_OUTOFMEMORY - All Phases
	int Variation_26();
	// @cmember ROW_DELETE - SINGLEROW - E_INVALIDARG - All Phases
	int Variation_27();
	// @cmember OTHER SCENARIOS - Delete no rows - Delete MULTIPLEROWS
	int Variation_28();
	// @cmember OTHER SCENARIOS - DBROWSTATUS_DELETED - MULTIPLEROWS
	int Variation_29();
	// @cmember OTHER SCENARIOS - EmptyRowset
	int Variation_30();
	// @cmember OTHER SCENARIOS - Add/Remove Listeners - MULTIPLEROWS
	int Variation_31();
	// @cmember OTHER SCENARIOS - GetData - MULTIPLEROWS
	int Variation_32();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(DeleteRows)
#define THE_CLASS DeleteRows
BEG_TEST_CASE(DeleteRows, CExecutionManager, L"Tests Notifications for IRowsetChange::DeleteRows")
	TEST_VARIATION(1, 		L"ROW_DELETE - SINGLEROW - Accept all phases")
	TEST_VARIATION(2, 		L"ROW_DELETE - MULTIPLEROWS - Accept all phases")
	TEST_VARIATION(3, 		L"Empty")
	TEST_VARIATION(4, 		L"ROW_DELETE - MULTIPLEROWS - DB_E_CANCELED")
	TEST_VARIATION(5, 		L"ROW_DELETE - SINGLEROW - DB_E_NOTREENTRANT")
	TEST_VARIATION(6, 		L"Empty")
	TEST_VARIATION(7, 		L"Empty")
	TEST_VARIATION(8, 		L"ROW_DELETE - MULTIPLEROWS - Veto OKTODO Listener 1")
	TEST_VARIATION(9, 		L"ROW_DELETE - MULTIPLEROWS - Veto ABOUTTODO Listener 1")
	TEST_VARIATION(10, 		L"ROW_DELETE - MULTIPLEROWS - Veto SYNCHAFTER Listener 1")
	TEST_VARIATION(11, 		L"ROW_DELETE - MULTIPLEROWS - Veto DIDEVENT Listener 2")
	TEST_VARIATION(12, 		L"ROW_DELETE - MULTIPLEROWS - Veto ABOUTTODO Listener 1 - Veto FAILEDTODO Listener 2")
	TEST_VARIATION(13, 		L"OTHER SCENARIOS - Add/Remove Listeners - SINGLEROW")
	TEST_VARIATION(14, 		L"OTHER SCENARIOS - GetData - SINGLEROW")
	TEST_VARIATION(15, 		L"ROW_DELETE - SINGLEROW - UNWANTED REASON")
	TEST_VARIATION(16, 		L"ROW_DELETE - SINGLEROW - UNWANTED PHASE OKTODO")
	TEST_VARIATION(17, 		L"ROW_DELETE - SINGLEROW - UNWANTED PHASE ABOUTTODO")
	TEST_VARIATION(18, 		L"ROW_DELETE - SINGLEROW - UNWANTED PHASE SYNCHAFTER")
	TEST_VARIATION(19, 		L"ROW_DELETE - SINGLEROW - UNWANTED PHASE DIDEVENT")
	TEST_VARIATION(20, 		L"ROW_DELETE - MULTIPLEROWS - Veto ABOUTTODO Listener 1 - UNWANTED PHASE FAILEDTODO")
	TEST_VARIATION(21, 		L"OTHER SCENARIOS - DBROWSTATUS_DELETED - SINGLEROW")
	TEST_VARIATION(22, 		L"ROW_DELETE - SINGLEROW - Veto ABOUTTODO Listener 1 - E_FAIL - All Phases")
	TEST_VARIATION(23, 		L"ROW_DELETE - MULTIPLEROWS - Veto ABOUTTODO Listener 1 - E_FAIL - All Phases")
	TEST_VARIATION(24, 		L"ROW_DELETE - MULTIPLEROWS - Veto ABOUTTODO Listener 1 - E_OUTOFMEMORY - All Phases")
	TEST_VARIATION(25, 		L"ROW_DELETE - MULTIPLEROWS - Veto ABOUTTODO Listener 1 - E_INVALIDARG - All Phases")
	TEST_VARIATION(26, 		L"ROW_DELETE - SINGLEROW - E_OUTOFMEMORY - All Phases")
	TEST_VARIATION(27, 		L"ROW_DELETE - SINGLEROW - E_INVALIDARG - All Phases")
	TEST_VARIATION(28, 		L"OTHER SCENARIOS - Delete no rows - Delete MULTIPLEROWS")
	TEST_VARIATION(29, 		L"OTHER SCENARIOS - DBROWSTATUS_DELETED - MULTIPLEROWS")
	TEST_VARIATION(30, 		L"OTHER SCENARIOS - EmptyRowset")
	TEST_VARIATION(31, 		L"OTHER SCENARIOS - Add/Remove Listeners - MULTIPLEROWS")
	TEST_VARIATION(32, 		L"OTHER SCENARIOS - GetData - MULTIPLEROWS")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(DeleteRows_Bufferred)
//--------------------------------------------------------------------
// @class Test Notifications for IRowsetChange::DeleteRows in Buffered Mode
//
class DeleteRows_Bufferred : public CExecutionManager { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(DeleteRows_Bufferred,CExecutionManager);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember ROW_DELETE - SINGLEROW - Accept all phases
	int Variation_1();
	// @cmember ROW_DELETE - MULTIPLEROWS - Accept all phases
	int Variation_2();
	// @cmember Empty
	int Variation_3();
	// @cmember ROW_DELETE - DB_E_CANCELED
	int Variation_4();
	// @cmember ROW_DELETE - DB_E_NOTREENTRANT
	int Variation_5();
	// @cmember Empty
	int Variation_6();
	// @cmember Empty
	int Variation_7();
	// @cmember ROW_DELETE - Veto OKTODO Listener 1
	int Variation_8();
	// @cmember ROW_DELETE - Veto ABOUTTODO Listener 1
	int Variation_9();
	// @cmember ROW_DELETE - Veto SYNCHAFTER Listener 1
	int Variation_10();
	// @cmember ROW_DELETE - Veto DIDEVENT Listener 2
	int Variation_11();
	// @cmember ROW_DELETE - Veto FAILEDTODO Listener 2
	int Variation_12();
	// @cmember Empty
	int Variation_13();
	// @cmember Empty
	int Variation_14();
	// @cmember ROW_DELETE - UNWANTED REASON
	int Variation_15();
	// @cmember ROW_DELETE - UNWANTED PHASE OKTODO
	int Variation_16();
	// @cmember ROW_DELETE - UNWANTED PHASE ABOUTTODO
	int Variation_17();
	// @cmember ROW_DELETE - UNWANTED PHASE SYNCHAFTER
	int Variation_18();
	// @cmember ROW_DELETE - UNWANTED PHASE DIDEVENT
	int Variation_19();
	// @cmember ROW_DELETE - UNWANTED PHASE FAILEDTODO
	int Variation_20();
	// @cmember Empty
	int Variation_21();
	// @cmember Empty
	int Variation_22();
	// @cmember ROW_DELETE - E_FAIL - All Phases
	int Variation_23();
	// @cmember ROW_DELETE - E_OUTOFMEMORY - All Phases
	int Variation_24();
	// @cmember ROW_DELETE - E_INVALIDARG - All Phases
	int Variation_25();
	// @cmember Empty
	int Variation_26();
	// @cmember Empty
	int Variation_27();
	// @cmember OTHER SCENARIOS - Delete no rows
	int Variation_28();
	// @cmember OTHER SCENARIOS - DBROWSTATUS_DELETED
	int Variation_29();
	// @cmember OTHER SCENARIOS - EmptyRowset
	int Variation_30();
	// @cmember OTHER SCENARIOS - Add/Remove Listeners
	int Variation_31();
	// @cmember OTHER SCENARIOS - GetData
	int Variation_32();
	// @cmember OTHER SCENARIOS - GetOriginalData
	int Variation_33();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(DeleteRows_Bufferred)
#define THE_CLASS DeleteRows_Bufferred
BEG_TEST_CASE(DeleteRows_Bufferred, CExecutionManager, L"Test Notifications for IRowsetChange::DeleteRows in Buffered Mode")
	TEST_VARIATION(1, 		L"ROW_DELETE - SINGLEROW - Accept all phases")
	TEST_VARIATION(2, 		L"ROW_DELETE - MULTIPLEROWS - Accept all phases")
	TEST_VARIATION(3, 		L"Empty")
	TEST_VARIATION(4, 		L"ROW_DELETE - DB_E_CANCELED")
	TEST_VARIATION(5, 		L"ROW_DELETE - DB_E_NOTREENTRANT")
	TEST_VARIATION(6, 		L"Empty")
	TEST_VARIATION(7, 		L"Empty")
	TEST_VARIATION(8, 		L"ROW_DELETE - Veto OKTODO Listener 1")
	TEST_VARIATION(9, 		L"ROW_DELETE - Veto ABOUTTODO Listener 1")
	TEST_VARIATION(10, 		L"ROW_DELETE - Veto SYNCHAFTER Listener 1")
	TEST_VARIATION(11, 		L"ROW_DELETE - Veto DIDEVENT Listener 2")
	TEST_VARIATION(12, 		L"ROW_DELETE - Veto FAILEDTODO Listener 2")
	TEST_VARIATION(13, 		L"Empty")
	TEST_VARIATION(14, 		L"Empty")
	TEST_VARIATION(15, 		L"ROW_DELETE - UNWANTED REASON")
	TEST_VARIATION(16, 		L"ROW_DELETE - UNWANTED PHASE OKTODO")
	TEST_VARIATION(17, 		L"ROW_DELETE - UNWANTED PHASE ABOUTTODO")
	TEST_VARIATION(18, 		L"ROW_DELETE - UNWANTED PHASE SYNCHAFTER")
	TEST_VARIATION(19, 		L"ROW_DELETE - UNWANTED PHASE DIDEVENT")
	TEST_VARIATION(20, 		L"ROW_DELETE - UNWANTED PHASE FAILEDTODO")
	TEST_VARIATION(21, 		L"Empty")
	TEST_VARIATION(22, 		L"Empty")
	TEST_VARIATION(23, 		L"ROW_DELETE - E_FAIL - All Phases")
	TEST_VARIATION(24, 		L"ROW_DELETE - E_OUTOFMEMORY - All Phases")
	TEST_VARIATION(25, 		L"ROW_DELETE - E_INVALIDARG - All Phases")
	TEST_VARIATION(26, 		L"Empty")
	TEST_VARIATION(27, 		L"Empty")
	TEST_VARIATION(28, 		L"OTHER SCENARIOS - Delete no rows")
	TEST_VARIATION(29, 		L"OTHER SCENARIOS - DBROWSTATUS_DELETED")
	TEST_VARIATION(30, 		L"OTHER SCENARIOS - EmptyRowset")
	TEST_VARIATION(31, 		L"OTHER SCENARIOS - Add/Remove Listeners")
	TEST_VARIATION(32, 		L"OTHER SCENARIOS - GetData")
	TEST_VARIATION(33, 		L"OTHER SCENARIOS - GetOriginalData")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(InsertRow)
//--------------------------------------------------------------------
// @class Tests Notifications for IRowsetChange::InsertRow
//
class InsertRow : public CExecutionManager { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(InsertRow,CExecutionManager);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember ROW_INSERT - Accept all phases
	int Variation_1();
	// @cmember Empty
	int Variation_2();
	// @cmember Empty
	int Variation_3();
	// @cmember ROW_INSERT - DB_E_NOTREENTRANT
	int Variation_4();
	// @cmember ROW_INSERT - DB_E_CANCELED
	int Variation_5();
	// @cmember Empty
	int Variation_6();
	// @cmember Empty
	int Variation_7();
	// @cmember ROW_INSERT - Veto OKTODO, Listener 1
	int Variation_8();
	// @cmember ROW_INSERT - Veto ABOUTTODO, Listener 1
	int Variation_9();
	// @cmember ROW_INSERT - Veto SYNCHAFTER, Listener 2
	int Variation_10();
	// @cmember ROW_INSERT - Veto DIDEVENT, Listener 3
	int Variation_11();
	// @cmember ROW_INSERT - Veto FAILEDTODO, Listener 3
	int Variation_12();
	// @cmember Empty
	int Variation_13();
	// @cmember ROW_INSERT - UNWANTED REASON
	int Variation_14();
	// @cmember ROW_INSERT - UNWANTED PHASE OKTODO
	int Variation_15();
	// @cmember ROW_INSERT - UNWANTED PHASE ABOUTTODO
	int Variation_16();
	// @cmember ROW_INSERT - UNWANTED PHASE SYNCHAFTER
	int Variation_17();
	// @cmember ROW_INSERT - UNWANTED PHASE DIDEVENT
	int Variation_18();
	// @cmember ROW_INSERT - UNWANTED PHASE FAILEDTODO
	int Variation_19();
	// @cmember Empty
	int Variation_20();
	// @cmember Empty
	int Variation_21();
	// @cmember ROW_INSERT - E_FAIL - All Phases
	int Variation_22();
	// @cmember ROW_INSERT - E_OUTOFMEMORY - All Phases
	int Variation_23();
	// @cmember ROW_INSERT - E_INVALIDARG - All Phases
	int Variation_24();
	// @cmember Empty
	int Variation_25();
	// @cmember Empty
	int Variation_26();
	// @cmember OTHER SCENARIOS - DB_E_BADACCESSORHANDLE
	int Variation_27();
	// @cmember OTHER SCENARIOS - EmptyRowset
	int Variation_28();
	// @cmember OTHER SCENARIOS - Add/Remove Listeners
	int Variation_29();
	// @cmember OTHER SCENARIOS - GetData
	int Variation_30();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(InsertRow)
#define THE_CLASS InsertRow
BEG_TEST_CASE(InsertRow, CExecutionManager, L"Tests Notifications for IRowsetChange::InsertRow")
	TEST_VARIATION(1, 		L"ROW_INSERT - Accept all phases")
	TEST_VARIATION(2, 		L"Empty")
	TEST_VARIATION(3, 		L"Empty")
	TEST_VARIATION(4, 		L"ROW_INSERT - DB_E_NOTREENTRANT")
	TEST_VARIATION(5, 		L"ROW_INSERT - DB_E_CANCELED")
	TEST_VARIATION(6, 		L"Empty")
	TEST_VARIATION(7, 		L"Empty")
	TEST_VARIATION(8, 		L"ROW_INSERT - Veto OKTODO, Listener 1")
	TEST_VARIATION(9, 		L"ROW_INSERT - Veto ABOUTTODO, Listener 1")
	TEST_VARIATION(10, 		L"ROW_INSERT - Veto SYNCHAFTER, Listener 2")
	TEST_VARIATION(11, 		L"ROW_INSERT - Veto DIDEVENT, Listener 3")
	TEST_VARIATION(12, 		L"ROW_INSERT - Veto FAILEDTODO, Listener 3")
	TEST_VARIATION(13, 		L"Empty")
	TEST_VARIATION(14, 		L"ROW_INSERT - UNWANTED REASON")
	TEST_VARIATION(15, 		L"ROW_INSERT - UNWANTED PHASE OKTODO")
	TEST_VARIATION(16, 		L"ROW_INSERT - UNWANTED PHASE ABOUTTODO")
	TEST_VARIATION(17, 		L"ROW_INSERT - UNWANTED PHASE SYNCHAFTER")
	TEST_VARIATION(18, 		L"ROW_INSERT - UNWANTED PHASE DIDEVENT")
	TEST_VARIATION(19, 		L"ROW_INSERT - UNWANTED PHASE FAILEDTODO")
	TEST_VARIATION(20, 		L"Empty")
	TEST_VARIATION(21, 		L"Empty")
	TEST_VARIATION(22, 		L"ROW_INSERT - E_FAIL - All Phases")
	TEST_VARIATION(23, 		L"ROW_INSERT - E_OUTOFMEMORY - All Phases")
	TEST_VARIATION(24, 		L"ROW_INSERT - E_INVALIDARG - All Phases")
	TEST_VARIATION(25, 		L"Empty")
	TEST_VARIATION(26, 		L"Empty")
	TEST_VARIATION(27, 		L"OTHER SCENARIOS - DB_E_BADACCESSORHANDLE")
	TEST_VARIATION(28, 		L"OTHER SCENARIOS - EmptyRowset")
	TEST_VARIATION(29, 		L"OTHER SCENARIOS - Add/Remove Listeners")
	TEST_VARIATION(30, 		L"OTHER SCENARIOS - GetData")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(InsertRow_Bufferred)
//--------------------------------------------------------------------
// @class Test Notifications for IRowsetChange::InsertRow in Buffered Mode
//
class InsertRow_Bufferred : public CExecutionManager { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(InsertRow_Bufferred,CExecutionManager);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember ROW_INSERT - Accept all phases
	int Variation_1();
	// @cmember Empty
	int Variation_2();
	// @cmember Empty
	int Variation_3();
	// @cmember ROW_INSERT - DB_E_NOTREENTRANT
	int Variation_4();
	// @cmember ROW_INSERT - DB_E_CANCELED
	int Variation_5();
	// @cmember Empty
	int Variation_6();
	// @cmember Empty
	int Variation_7();
	// @cmember ROW_INSERT - Veto OKTODO, Listener 1
	int Variation_8();
	// @cmember ROW_INSERT - Veto ABOUTTODO, Listener 1
	int Variation_9();
	// @cmember ROW_INSERT - Veto SYNCHAFTER, Listener 2
	int Variation_10();
	// @cmember ROW_INSERT - Veto DIDEVENT, Listener 3
	int Variation_11();
	// @cmember ROW_INSERT - Veto FAILEDTODO, Listener 3
	int Variation_12();
	// @cmember Empty
	int Variation_13();
	// @cmember ROW_INSERT - UNWANTED REASON
	int Variation_14();
	// @cmember ROW_INSERT - UNWANTED PHASE OKTODO
	int Variation_15();
	// @cmember ROW_INSERT - UNWANTED PHASE ABOUTTODO
	int Variation_16();
	// @cmember ROW_INSERT - UNWANTED PHASE SYNCHAFTER
	int Variation_17();
	// @cmember ROW_INSERT - UNWANTED PHASE DIDEVENT
	int Variation_18();
	// @cmember ROW_INSERT - UNWANTED PHASE FAILEDTODO
	int Variation_19();
	// @cmember Empty
	int Variation_20();
	// @cmember Empty
	int Variation_21();
	// @cmember ROW_INSERT - E_FAIL - All Phases
	int Variation_22();
	// @cmember ROW_INSERT - E_OUTOFMEMORY - All Phases
	int Variation_23();
	// @cmember ROW_INSERT - E_INVALIDARG - All Phases
	int Variation_24();
	// @cmember Empty
	int Variation_25();
	// @cmember Empty
	int Variation_26();
	// @cmember OTHER SCENARIOS - DB_E_BADACCESSORHANDLE
	int Variation_27();
	// @cmember OTHER SCENARIOS - EmptyRowset
	int Variation_28();
	// @cmember OTHER SCENARIOS - Add/Remove Listeners
	int Variation_29();
	// @cmember OTHER SCENARIOS - GetData
	int Variation_30();
	// @cmember OTHER SCENARIOS - GetOriginalData
	int Variation_31();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(InsertRow_Bufferred)
#define THE_CLASS InsertRow_Bufferred
BEG_TEST_CASE(InsertRow_Bufferred, CExecutionManager, L"Test Notifications for IRowsetChange::InsertRow in Buffered Mode")
	TEST_VARIATION(1, 		L"ROW_INSERT - Accept all phases")
	TEST_VARIATION(2, 		L"Empty")
	TEST_VARIATION(3, 		L"Empty")
	TEST_VARIATION(4, 		L"ROW_INSERT - DB_E_NOTREENTRANT")
	TEST_VARIATION(5, 		L"ROW_INSERT - DB_E_CANCELED")
	TEST_VARIATION(6, 		L"Empty")
	TEST_VARIATION(7, 		L"Empty")
	TEST_VARIATION(8, 		L"ROW_INSERT - Veto OKTODO, Listener 1")
	TEST_VARIATION(9, 		L"ROW_INSERT - Veto ABOUTTODO, Listener 1")
	TEST_VARIATION(10, 		L"ROW_INSERT - Veto SYNCHAFTER, Listener 2")
	TEST_VARIATION(11, 		L"ROW_INSERT - Veto DIDEVENT, Listener 3")
	TEST_VARIATION(12, 		L"ROW_INSERT - Veto FAILEDTODO, Listener 3")
	TEST_VARIATION(13, 		L"Empty")
	TEST_VARIATION(14, 		L"ROW_INSERT - UNWANTED REASON")
	TEST_VARIATION(15, 		L"ROW_INSERT - UNWANTED PHASE OKTODO")
	TEST_VARIATION(16, 		L"ROW_INSERT - UNWANTED PHASE ABOUTTODO")
	TEST_VARIATION(17, 		L"ROW_INSERT - UNWANTED PHASE SYNCHAFTER")
	TEST_VARIATION(18, 		L"ROW_INSERT - UNWANTED PHASE DIDEVENT")
	TEST_VARIATION(19, 		L"ROW_INSERT - UNWANTED PHASE FAILEDTODO")
	TEST_VARIATION(20, 		L"Empty")
	TEST_VARIATION(21, 		L"Empty")
	TEST_VARIATION(22, 		L"ROW_INSERT - E_FAIL - All Phases")
	TEST_VARIATION(23, 		L"ROW_INSERT - E_OUTOFMEMORY - All Phases")
	TEST_VARIATION(24, 		L"ROW_INSERT - E_INVALIDARG - All Phases")
	TEST_VARIATION(25, 		L"Empty")
	TEST_VARIATION(26, 		L"Empty")
	TEST_VARIATION(27, 		L"OTHER SCENARIOS - DB_E_BADACCESSORHANDLE")
	TEST_VARIATION(28, 		L"OTHER SCENARIOS - EmptyRowset")
	TEST_VARIATION(29, 		L"OTHER SCENARIOS - Add/Remove Listeners")
	TEST_VARIATION(30, 		L"OTHER SCENARIOS - GetData")
	TEST_VARIATION(31, 		L"OTHER SCENARIOS - GetOriginalData")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(SetData)
//--------------------------------------------------------------------
// @class Tests Notifications for IRowsetChange::SetData
//
class SetData : public CExecutionManager { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(SetData,CExecutionManager);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember COLUMN_SET - Accept all phases
	int Variation_1();
	// @cmember Empty
	int Variation_2();
	// @cmember Empty
	int Variation_3();
	// @cmember COLUMN_SET - DB_E_CANCELED
	int Variation_4();
	// @cmember COLUMN_SET - DB_E_NOTREENTRANT
	int Variation_5();
	// @cmember Empty
	int Variation_6();
	// @cmember Empty
	int Variation_7();
	// @cmember COLUMN_SET - Veto OKTODO
	int Variation_8();
	// @cmember COLUMN_SET - Veto ABOUTTODO
	int Variation_9();
	// @cmember COLUMN_SET - Veto SYNCHAFTER
	int Variation_10();
	// @cmember COLUMN_SET - Veto DIDEVENT
	int Variation_11();
	// @cmember COLUMN_SET - Veto FAILEDTODO
	int Variation_12();
	// @cmember Empty
	int Variation_13();
	// @cmember COLUMN_SET - UNWANTED REASON
	int Variation_14();
	// @cmember COLUMN_SET - UNWANTED PHASE OKTODO
	int Variation_15();
	// @cmember COLUMN_SET - UNWANTED PHASE ABOUTTO
	int Variation_16();
	// @cmember COLUMN_SET - UNWANTED PHASE SYNCHAFTER
	int Variation_17();
	// @cmember COLUMN_SET- UNWANTED PHASE DIDEVENT
	int Variation_18();
	// @cmember COLUMN_SET - UNWANTED PHASE FAILEDTODO
	int Variation_19();
	// @cmember Empty
	int Variation_20();
	// @cmember Empty
	int Variation_21();
	// @cmember COLUMN_SET - E_FAIL - All Phases
	int Variation_22();
	// @cmember COLUMN_SET - E_OUTOFMEMORY - All Phases
	int Variation_23();
	// @cmember COLUMN_SET - E_INVALIDARG - All Phases
	int Variation_24();
	// @cmember Empty
	int Variation_25();
	// @cmember Empty
	int Variation_26();
	// @cmember COLUMN_RECALCULATED - DB_E_ERRORSOCCURRED - Accept all phases
	int Variation_27();
	// @cmember Empty
	int Variation_28();
	// @cmember Empty
	int Variation_29();
	// @cmember COLUMN_RECALCULATED - DB_E_CANCELED
	int Variation_30();
	// @cmember COLUMN_RECALCULATED - DB_E_NOTREENTRANT
	int Variation_31();
	// @cmember Empty
	int Variation_32();
	// @cmember Empty
	int Variation_33();
	// @cmember COLUMN_RECALCULATED - Veto DIDEVENT
	int Variation_34();
	// @cmember Empty
	int Variation_35();
	// @cmember Empty
	int Variation_36();
	// @cmember COLUMN_RECALCULATED - UNWANTED REASON
	int Variation_37();
	// @cmember COLUMN_RECALCULATED - UNWANTED PHASE DIDEVENT
	int Variation_38();
	// @cmember Empty
	int Variation_39();
	// @cmember Empty
	int Variation_40();
	// @cmember COLUMN_RECALCULATED - E_FAIL - All Phases
	int Variation_41();
	// @cmember COLUMN_RECALCULATED - E_OUTOFMEMORY - All Phases
	int Variation_42();
	// @cmember COLUMN_RECALCULATED - E_INVALIDARG - All Phases
	int Variation_43();
	// @cmember Empty
	int Variation_44();
	// @cmember Empty
	int Variation_45();
	// @cmember OTHER SCENARIOS - DB_E_BADACCESSORHANDLE
	int Variation_46();
	// @cmember OTHER SCENARIOS - EmptyRowset
	int Variation_47();
	// @cmember OTHER SCENARIOS - Add/Remove Listeners
	int Variation_48();
	// @cmember OTHER SCENARIOS - GetData
	int Variation_49();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(SetData)
#define THE_CLASS SetData
BEG_TEST_CASE(SetData, CExecutionManager, L"Tests Notifications for IRowsetChange::SetData")
	TEST_VARIATION(1, 		L"COLUMN_SET - Accept all phases")
	TEST_VARIATION(2, 		L"Empty")
	TEST_VARIATION(3, 		L"Empty")
	TEST_VARIATION(4, 		L"COLUMN_SET - DB_E_CANCELED")
	TEST_VARIATION(5, 		L"COLUMN_SET - DB_E_NOTREENTRANT")
	TEST_VARIATION(6, 		L"Empty")
	TEST_VARIATION(7, 		L"Empty")
	TEST_VARIATION(8, 		L"COLUMN_SET - Veto OKTODO")
	TEST_VARIATION(9, 		L"COLUMN_SET - Veto ABOUTTODO")
	TEST_VARIATION(10, 		L"COLUMN_SET - Veto SYNCHAFTER")
	TEST_VARIATION(11, 		L"COLUMN_SET - Veto DIDEVENT")
	TEST_VARIATION(12, 		L"COLUMN_SET - Veto FAILEDTODO")
	TEST_VARIATION(13, 		L"Empty")
	TEST_VARIATION(14, 		L"COLUMN_SET - UNWANTED REASON")
	TEST_VARIATION(15, 		L"COLUMN_SET - UNWANTED PHASE OKTODO")
	TEST_VARIATION(16, 		L"COLUMN_SET - UNWANTED PHASE ABOUTTO")
	TEST_VARIATION(17, 		L"COLUMN_SET - UNWANTED PHASE SYNCHAFTER")
	TEST_VARIATION(18, 		L"COLUMN_SET- UNWANTED PHASE DIDEVENT")
	TEST_VARIATION(19, 		L"COLUMN_SET - UNWANTED PHASE FAILEDTODO")
	TEST_VARIATION(20, 		L"Empty")
	TEST_VARIATION(21, 		L"Empty")
	TEST_VARIATION(22, 		L"COLUMN_SET - E_FAIL - All Phases")
	TEST_VARIATION(23, 		L"COLUMN_SET - E_OUTOFMEMORY - All Phases")
	TEST_VARIATION(24, 		L"COLUMN_SET - E_INVALIDARG - All Phases")
	TEST_VARIATION(25, 		L"Empty")
	TEST_VARIATION(26, 		L"Empty")
	TEST_VARIATION(27, 		L"COLUMN_RECALCULATED - DB_E_ERRORSOCCURRED - Accept all phases")
	TEST_VARIATION(28, 		L"Empty")
	TEST_VARIATION(29, 		L"Empty")
	TEST_VARIATION(30, 		L"COLUMN_RECALCULATED - DB_E_CANCELED")
	TEST_VARIATION(31, 		L"COLUMN_RECALCULATED - DB_E_NOTREENTRANT")
	TEST_VARIATION(32, 		L"Empty")
	TEST_VARIATION(33, 		L"Empty")
	TEST_VARIATION(34, 		L"COLUMN_RECALCULATED - Veto DIDEVENT")
	TEST_VARIATION(35, 		L"Empty")
	TEST_VARIATION(36, 		L"Empty")
	TEST_VARIATION(37, 		L"COLUMN_RECALCULATED - UNWANTED REASON")
	TEST_VARIATION(38, 		L"COLUMN_RECALCULATED - UNWANTED PHASE DIDEVENT")
	TEST_VARIATION(39, 		L"Empty")
	TEST_VARIATION(40, 		L"Empty")
	TEST_VARIATION(41, 		L"COLUMN_RECALCULATED - E_FAIL - All Phases")
	TEST_VARIATION(42, 		L"COLUMN_RECALCULATED - E_OUTOFMEMORY - All Phases")
	TEST_VARIATION(43, 		L"COLUMN_RECALCULATED - E_INVALIDARG - All Phases")
	TEST_VARIATION(44, 		L"Empty")
	TEST_VARIATION(45, 		L"Empty")
	TEST_VARIATION(46, 		L"OTHER SCENARIOS - DB_E_BADACCESSORHANDLE")
	TEST_VARIATION(47, 		L"OTHER SCENARIOS - EmptyRowset")
	TEST_VARIATION(48, 		L"OTHER SCENARIOS - Add/Remove Listeners")
	TEST_VARIATION(49, 		L"OTHER SCENARIOS - GetData")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(SetData_Bufferred)
//--------------------------------------------------------------------
// @class Test Notifications for IRowsetChange::SetData in Buffered Mode
//
class SetData_Bufferred : public CExecutionManager { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(SetData_Bufferred,CExecutionManager);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember ROW_FIRSTCHANGE - Accept all phases
	int Variation_1();
	// @cmember Empty
	int Variation_2();
	// @cmember Empty
	int Variation_3();
	// @cmember ROW_FIRSTCHANGE - DB_E_CANCELED
	int Variation_4();
	// @cmember ROW_FIRSTCHANGE - DB_E_NOTREENTRANT
	int Variation_5();
	// @cmember Empty
	int Variation_6();
	// @cmember Empty
	int Variation_7();
	// @cmember ROW_FIRSTCHANGE - Veto OKTODO
	int Variation_8();
	// @cmember ROW_FIRSTCHANGE - Veto ABOUTTODO
	int Variation_9();
	// @cmember ROW_FIRSTCHANGE - Veto SYNCHAFTER
	int Variation_10();
	// @cmember ROW_FIRSTCHANGE - Veto DIDEVENT
	int Variation_11();
	// @cmember ROW_FIRSTCHANGE - Veto FAILEDTODO
	int Variation_12();
	// @cmember Empty
	int Variation_13();
	// @cmember Empty
	int Variation_14();
	// @cmember ROW_FIRSTCHANGE - UNWANTED REASON
	int Variation_15();
	// @cmember ROW_FIRSTCHANGE - UNWANTED PHASE OKTODO
	int Variation_16();
	// @cmember ROW_FIRSTCHANGE - UNWANTED PHASE ABOUTTODO
	int Variation_17();
	// @cmember ROW_FIRSTCHANGE - UNWANTED PHASE SYNCHAFTER
	int Variation_18();
	// @cmember ROW_FIRSTCHANGE - UNWANTED PHASE DIDEVENT
	int Variation_19();
	// @cmember ROW_FIRSTCHANGE - UNWANTED PHASE FAILEDTODO
	int Variation_20();
	// @cmember Empty
	int Variation_21();
	// @cmember Empty
	int Variation_22();
	// @cmember ROW_FIRSTCHANGE - E_FAIL - All Phases
	int Variation_23();
	// @cmember ROW_FIRSTCHANGE - E_OUTOFMEMORY - All Phases
	int Variation_24();
	// @cmember ROW_FIRSTCHANGE - E_INVALIDARG - All Phases
	int Variation_25();
	// @cmember Empty
	int Variation_26();
	// @cmember Empty
	int Variation_27();
	// @cmember COLUMN_SET - Accept all phases
	int Variation_28();
	// @cmember Empty
	int Variation_29();
	// @cmember Empty
	int Variation_30();
	// @cmember COLUMN_SET - DB_E_CANCELED
	int Variation_31();
	// @cmember COLUMN_SET - DB_E_NOTREENTRANT
	int Variation_32();
	// @cmember Empty
	int Variation_33();
	// @cmember Empty
	int Variation_34();
	// @cmember COLUMN_SET - Veto OKTODO
	int Variation_35();
	// @cmember COLUMN_SET - Veto ABOUTTODO
	int Variation_36();
	// @cmember COLUMN_SET - Veto SYNCHAFTER
	int Variation_37();
	// @cmember COLUMN_SET - Veto DIDEVENT
	int Variation_38();
	// @cmember COLUMN_SET - Veto FAILEDTODO
	int Variation_39();
	// @cmember Empty
	int Variation_40();
	// @cmember COLUMN_SET - UNWANTED REASON
	int Variation_41();
	// @cmember COLUMN_SET - UNWANTED PHASE OKTODO
	int Variation_42();
	// @cmember COLUMN_SET - UNWANTED PHASE ABOUTTO
	int Variation_43();
	// @cmember COLUMN_SET - UNWANTED PHASE SYNCHAFTER
	int Variation_44();
	// @cmember COLUMN_SET- UNWANTED PHASE DIDEVENT
	int Variation_45();
	// @cmember COLUMN_SET - UNWANTED PHASE FAILEDTODO
	int Variation_46();
	// @cmember Empty
	int Variation_47();
	// @cmember Empty
	int Variation_48();
	// @cmember COLUMN_SET - E_FAIL - All Phases
	int Variation_49();
	// @cmember COLUMN_SET - E_OUTOFMEMORY - All Phases
	int Variation_50();
	// @cmember COLUMN_SET - E_INVALIDARG - All Phases
	int Variation_51();
	// @cmember Empty
	int Variation_52();
	// @cmember Empty
	int Variation_53();
	// @cmember COLUMN_RECALCULATED - Accept all phases
	int Variation_54();
	// @cmember Empty
	int Variation_55();
	// @cmember Empty
	int Variation_56();
	// @cmember COLUMN_RECALCULATED - DB_E_CANCELED
	int Variation_57();
	// @cmember COLUMN_RECALCULATED - DB_E_NOTREENTRANT
	int Variation_58();
	// @cmember Empty
	int Variation_59();
	// @cmember Empty
	int Variation_60();
	// @cmember COLUMN_RECALCULATED - Veto DIDEVENT
	int Variation_61();
	// @cmember Empty
	int Variation_62();
	// @cmember Empty
	int Variation_63();
	// @cmember COLUMN_RECALCULATED - UNWANTED REASON
	int Variation_64();
	// @cmember COLUMN_RECALCULATED - UNWANTED PHASE DIDEVENT
	int Variation_65();
	// @cmember Empty
	int Variation_66();
	// @cmember Empty
	int Variation_67();
	// @cmember COLUMN_RECALCULATED - E_FAIL - All Phases
	int Variation_68();
	// @cmember COLUMN_RECALCULATED - E_OUTOFMEMORY - All Phases
	int Variation_69();
	// @cmember COLUMN_RECALCULATED - E_INVALIDARG - All Phases
	int Variation_70();
	// @cmember Empty
	int Variation_71();
	// @cmember Empty
	int Variation_72();
	// @cmember OTHER SCENARIOS - DB_E_BADACCESSORHANDLE
	int Variation_73();
	// @cmember OTHER SCENARIOS - EmptyRowset
	int Variation_74();
	// @cmember OTHER SCENARIOS - Add/Remove Listeners
	int Variation_75();
	// @cmember OTHER SCENARIOS - GetData
	int Variation_76();
	// @cmember OTHER SCENARIOS - GetOriginalData
	int Variation_77();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(SetData_Bufferred)
#define THE_CLASS SetData_Bufferred
BEG_TEST_CASE(SetData_Bufferred, CExecutionManager, L"Test Notifications for IRowsetChange::SetData in Buffered Mode")
	TEST_VARIATION(1, 		L"ROW_FIRSTCHANGE - Accept all phases")
	TEST_VARIATION(2, 		L"Empty")
	TEST_VARIATION(3, 		L"Empty")
	TEST_VARIATION(4, 		L"ROW_FIRSTCHANGE - DB_E_CANCELED")
	TEST_VARIATION(5, 		L"ROW_FIRSTCHANGE - DB_E_NOTREENTRANT")
	TEST_VARIATION(6, 		L"Empty")
	TEST_VARIATION(7, 		L"Empty")
	TEST_VARIATION(8, 		L"ROW_FIRSTCHANGE - Veto OKTODO")
	TEST_VARIATION(9, 		L"ROW_FIRSTCHANGE - Veto ABOUTTODO")
	TEST_VARIATION(10, 		L"ROW_FIRSTCHANGE - Veto SYNCHAFTER")
	TEST_VARIATION(11, 		L"ROW_FIRSTCHANGE - Veto DIDEVENT")
	TEST_VARIATION(12, 		L"ROW_FIRSTCHANGE - Veto FAILEDTODO")
	TEST_VARIATION(13, 		L"Empty")
	TEST_VARIATION(14, 		L"Empty")
	TEST_VARIATION(15, 		L"ROW_FIRSTCHANGE - UNWANTED REASON")
	TEST_VARIATION(16, 		L"ROW_FIRSTCHANGE - UNWANTED PHASE OKTODO")
	TEST_VARIATION(17, 		L"ROW_FIRSTCHANGE - UNWANTED PHASE ABOUTTODO")
	TEST_VARIATION(18, 		L"ROW_FIRSTCHANGE - UNWANTED PHASE SYNCHAFTER")
	TEST_VARIATION(19, 		L"ROW_FIRSTCHANGE - UNWANTED PHASE DIDEVENT")
	TEST_VARIATION(20, 		L"ROW_FIRSTCHANGE - UNWANTED PHASE FAILEDTODO")
	TEST_VARIATION(21, 		L"Empty")
	TEST_VARIATION(22, 		L"Empty")
	TEST_VARIATION(23, 		L"ROW_FIRSTCHANGE - E_FAIL - All Phases")
	TEST_VARIATION(24, 		L"ROW_FIRSTCHANGE - E_OUTOFMEMORY - All Phases")
	TEST_VARIATION(25, 		L"ROW_FIRSTCHANGE - E_INVALIDARG - All Phases")
	TEST_VARIATION(26, 		L"Empty")
	TEST_VARIATION(27, 		L"Empty")
	TEST_VARIATION(28, 		L"COLUMN_SET - Accept all phases")
	TEST_VARIATION(29, 		L"Empty")
	TEST_VARIATION(30, 		L"Empty")
	TEST_VARIATION(31, 		L"COLUMN_SET - DB_E_CANCELED")
	TEST_VARIATION(32, 		L"COLUMN_SET - DB_E_NOTREENTRANT")
	TEST_VARIATION(33, 		L"Empty")
	TEST_VARIATION(34, 		L"Empty")
	TEST_VARIATION(35, 		L"COLUMN_SET - Veto OKTODO")
	TEST_VARIATION(36, 		L"COLUMN_SET - Veto ABOUTTODO")
	TEST_VARIATION(37, 		L"COLUMN_SET - Veto SYNCHAFTER")
	TEST_VARIATION(38, 		L"COLUMN_SET - Veto DIDEVENT")
	TEST_VARIATION(39, 		L"COLUMN_SET - Veto FAILEDTODO")
	TEST_VARIATION(40, 		L"Empty")
	TEST_VARIATION(41, 		L"COLUMN_SET - UNWANTED REASON")
	TEST_VARIATION(42, 		L"COLUMN_SET - UNWANTED PHASE OKTODO")
	TEST_VARIATION(43, 		L"COLUMN_SET - UNWANTED PHASE ABOUTTO")
	TEST_VARIATION(44, 		L"COLUMN_SET - UNWANTED PHASE SYNCHAFTER")
	TEST_VARIATION(45, 		L"COLUMN_SET- UNWANTED PHASE DIDEVENT")
	TEST_VARIATION(46, 		L"COLUMN_SET - UNWANTED PHASE FAILEDTODO")
	TEST_VARIATION(47, 		L"Empty")
	TEST_VARIATION(48, 		L"Empty")
	TEST_VARIATION(49, 		L"COLUMN_SET - E_FAIL - All Phases")
	TEST_VARIATION(50, 		L"COLUMN_SET - E_OUTOFMEMORY - All Phases")
	TEST_VARIATION(51, 		L"COLUMN_SET - E_INVALIDARG - All Phases")
	TEST_VARIATION(52, 		L"Empty")
	TEST_VARIATION(53, 		L"Empty")
	TEST_VARIATION(54, 		L"COLUMN_RECALCULATED - Accept all phases")
	TEST_VARIATION(55, 		L"Empty")
	TEST_VARIATION(56, 		L"Empty")
	TEST_VARIATION(57, 		L"COLUMN_RECALCULATED - DB_E_CANCELED")
	TEST_VARIATION(58, 		L"COLUMN_RECALCULATED - DB_E_NOTREENTRANT")
	TEST_VARIATION(59, 		L"Empty")
	TEST_VARIATION(60, 		L"Empty")
	TEST_VARIATION(61, 		L"COLUMN_RECALCULATED - Veto DIDEVENT")
	TEST_VARIATION(62, 		L"Empty")
	TEST_VARIATION(63, 		L"Empty")
	TEST_VARIATION(64, 		L"COLUMN_RECALCULATED - UNWANTED REASON")
	TEST_VARIATION(65, 		L"COLUMN_RECALCULATED - UNWANTED PHASE DIDEVENT")
	TEST_VARIATION(66, 		L"Empty")
	TEST_VARIATION(67, 		L"Empty")
	TEST_VARIATION(68, 		L"COLUMN_RECALCULATED - E_FAIL - All Phases")
	TEST_VARIATION(69, 		L"COLUMN_RECALCULATED - E_OUTOFMEMORY - All Phases")
	TEST_VARIATION(70, 		L"COLUMN_RECALCULATED - E_INVALIDARG - All Phases")
	TEST_VARIATION(71, 		L"Empty")
	TEST_VARIATION(72, 		L"Empty")
	TEST_VARIATION(73, 		L"OTHER SCENARIOS - DB_E_BADACCESSORHANDLE")
	TEST_VARIATION(74, 		L"OTHER SCENARIOS - EmptyRowset")
	TEST_VARIATION(75, 		L"OTHER SCENARIOS - Add/Remove Listeners")
	TEST_VARIATION(76, 		L"OTHER SCENARIOS - GetData")
	TEST_VARIATION(77, 		L"OTHER SCENARIOS - GetOriginalData")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(Resynch)
//--------------------------------------------------------------------
// @class Tests Notifications for IRowsetResynch::ResyncRows
//
class Resynch : public CExecutionManager { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Resynch,CExecutionManager);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember ROW_RESYNCH - Accept all phases
	int Variation_1();
	// @cmember Empty
	int Variation_2();
	// @cmember Empty
	int Variation_3();
	// @cmember ROW_RESYNCH - DB_E_CANCELED
	int Variation_4();
	// @cmember ROW_RESYNCH - DB_E_NOTREENTRANT
	int Variation_5();
	// @cmember Empty
	int Variation_6();
	// @cmember Empty
	int Variation_7();
	// @cmember ROW_RESYNCH - Veto OKTODO
	int Variation_8();
	// @cmember ROW_RESYNCH - Veto ABOUTTODO
	int Variation_9();
	// @cmember ROW_RESYNCH - Veto SYNCHAFTER
	int Variation_10();
	// @cmember ROW_RESYNCH - Veto DIDEVENT
	int Variation_11();
	// @cmember ROW_RESYNCH - Veto FAILEDTODO
	int Variation_12();
	// @cmember Empty
	int Variation_13();
	// @cmember Empty
	int Variation_14();
	// @cmember ROW_RESYNCH - UNWANTED REASON
	int Variation_15();
	// @cmember ROW_RESYNCH - UNWANTED PHASE - OKTODO
	int Variation_16();
	// @cmember ROW_RESYNCH - UNWANTED PHASE - ABOUTTODO
	int Variation_17();
	// @cmember ROW_RESYNCH - UNWANTED PHASE - SYNCHAFTER
	int Variation_18();
	// @cmember ROW_RESYNCH - UNWANTED PHASE - DIDEVENT
	int Variation_19();
	// @cmember ROW_RESYNCH - UNWANTED PHASE - FAILEDTODO
	int Variation_20();
	// @cmember Empty
	int Variation_21();
	// @cmember Empty
	int Variation_22();
	// @cmember ROW_RESYNCH - E_FAIL - All Phases
	int Variation_23();
	// @cmember ROW_RESYNCH - E_OUTOFMEMORY - All Phases
	int Variation_24();
	// @cmember ROW_RESYNCH - E_INVALIDARG - All Phases
	int Variation_25();
	// @cmember Empty
	int Variation_26();
	// @cmember Empty
	int Variation_27();
	// @cmember OTHER SCENARIOS - Resynch all rows
	int Variation_28();
	// @cmember OTHER SCENARIOS - Resynch 2 rows, 1 changed 1 not changed
	int Variation_29();
	// @cmember OTHER SCENARIOS - 1 valid, 1 not valid
	int Variation_30();
	// @cmember OTHER SCENARIOS - EmptyRowset
	int Variation_31();
	// @cmember OTHER SCENARIOS - Add/Remove Listeners
	int Variation_32();
	// @cmember OTHER SCENARIOS - GetData
	int Variation_33();
	// @cmember OTHER SCENARIOS - GetVisibleData
	int Variation_34();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(Resynch)
#define THE_CLASS Resynch
BEG_TEST_CASE(Resynch, CExecutionManager, L"Tests Notifications for IRowsetResynch::ResyncRows")
	TEST_VARIATION(1, 		L"ROW_RESYNCH - Accept all phases")
	TEST_VARIATION(2, 		L"Empty")
	TEST_VARIATION(3, 		L"Empty")
	TEST_VARIATION(4, 		L"ROW_RESYNCH - DB_E_CANCELED")
	TEST_VARIATION(5, 		L"ROW_RESYNCH - DB_E_NOTREENTRANT")
	TEST_VARIATION(6, 		L"Empty")
	TEST_VARIATION(7, 		L"Empty")
	TEST_VARIATION(8, 		L"ROW_RESYNCH - Veto OKTODO")
	TEST_VARIATION(9, 		L"ROW_RESYNCH - Veto ABOUTTODO")
	TEST_VARIATION(10, 		L"ROW_RESYNCH - Veto SYNCHAFTER")
	TEST_VARIATION(11, 		L"ROW_RESYNCH - Veto DIDEVENT")
	TEST_VARIATION(12, 		L"ROW_RESYNCH - Veto FAILEDTODO")
	TEST_VARIATION(13, 		L"Empty")
	TEST_VARIATION(14, 		L"Empty")
	TEST_VARIATION(15, 		L"ROW_RESYNCH - UNWANTED REASON")
	TEST_VARIATION(16, 		L"ROW_RESYNCH - UNWANTED PHASE - OKTODO")
	TEST_VARIATION(17, 		L"ROW_RESYNCH - UNWANTED PHASE - ABOUTTODO")
	TEST_VARIATION(18, 		L"ROW_RESYNCH - UNWANTED PHASE - SYNCHAFTER")
	TEST_VARIATION(19, 		L"ROW_RESYNCH - UNWANTED PHASE - DIDEVENT")
	TEST_VARIATION(20, 		L"ROW_RESYNCH - UNWANTED PHASE - FAILEDTODO")
	TEST_VARIATION(21, 		L"Empty")
	TEST_VARIATION(22, 		L"Empty")
	TEST_VARIATION(23, 		L"ROW_RESYNCH - E_FAIL - All Phases")
	TEST_VARIATION(24, 		L"ROW_RESYNCH - E_OUTOFMEMORY - All Phases")
	TEST_VARIATION(25, 		L"ROW_RESYNCH - E_INVALIDARG - All Phases")
	TEST_VARIATION(26, 		L"Empty")
	TEST_VARIATION(27, 		L"Empty")
	TEST_VARIATION(28, 		L"OTHER SCENARIOS - Resynch all rows")
	TEST_VARIATION(29, 		L"OTHER SCENARIOS - Resynch 2 rows, 1 changed 1 not changed")
	TEST_VARIATION(30, 		L"OTHER SCENARIOS - 1 valid, 1 not valid")
	TEST_VARIATION(31, 		L"OTHER SCENARIOS - EmptyRowset")
	TEST_VARIATION(32, 		L"OTHER SCENARIOS - Add/Remove Listeners")
	TEST_VARIATION(33, 		L"OTHER SCENARIOS - GetData")
	TEST_VARIATION(34, 		L"OTHER SCENARIOS - GetVisibleData")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(Resynch_Bufferred)
//--------------------------------------------------------------------
// @class Tests Notifications for IRowsetResynch_Bufferred::ResyncRows
//
class Resynch_Bufferred : public CExecutionManager { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Resynch_Bufferred,CExecutionManager);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember ROW_RESYNCH - Accept all phases
	int Variation_1();
	// @cmember Empty
	int Variation_2();
	// @cmember Empty
	int Variation_3();
	// @cmember ROW_RESYNCH - DB_E_CANCELED
	int Variation_4();
	// @cmember ROW_RESYNCH - DB_E_NOTREENTRANT
	int Variation_5();
	// @cmember Empty
	int Variation_6();
	// @cmember Empty
	int Variation_7();
	// @cmember ROW_RESYNCH - Veto OKTODO
	int Variation_8();
	// @cmember ROW_RESYNCH - Veto ABOUTTODO
	int Variation_9();
	// @cmember ROW_RESYNCH - Veto SYNCHAFTER
	int Variation_10();
	// @cmember ROW_RESYNCH - Veto DIDEVENT
	int Variation_11();
	// @cmember ROW_RESYNCH - Veto FAILEDTODO
	int Variation_12();
	// @cmember Empty
	int Variation_13();
	// @cmember Empty
	int Variation_14();
	// @cmember ROW_RESYNCH - UNWANTED REASON
	int Variation_15();
	// @cmember ROW_RESYNCH - UNWANTED PHASE - OKTODO
	int Variation_16();
	// @cmember ROW_RESYNCH - UNWANTED PHASE - ABOUTTODO
	int Variation_17();
	// @cmember ROW_RESYNCH - UNWANTED PHASE - SYNCHAFTER
	int Variation_18();
	// @cmember ROW_RESYNCH - UNWANTED PHASE - DIDEVENT
	int Variation_19();
	// @cmember ROW_RESYNCH - UNWANTED PHASE - FAILEDTODO
	int Variation_20();
	// @cmember Empty
	int Variation_21();
	// @cmember Empty
	int Variation_22();
	// @cmember ROW_RESYNCH - E_FAIL - All Phases
	int Variation_23();
	// @cmember ROW_RESYNCH - E_OUTOFMEMORY - All Phases
	int Variation_24();
	// @cmember ROW_RESYNCH - E_INVALIDARG - All Phases
	int Variation_25();
	// @cmember Empty
	int Variation_26();
	// @cmember Empty
	int Variation_27();
	// @cmember OTHER SCENARIOS - Resynch_Bufferred all rows
	int Variation_28();
	// @cmember OTHER SCENARIOS - Resynch_Bufferred 2 rows, 1 changed 1 not changed
	int Variation_29();
	// @cmember OTHER SCENARIOS - 1 valid, 1 not valid
	int Variation_30();
	// @cmember OTHER SCENARIOS - EmptyRowset
	int Variation_31();
	// @cmember OTHER SCENARIOS - Add/Remove Listeners
	int Variation_32();
	// @cmember OTHER SCENARIOS - GetData
	int Variation_33();
	// @cmember OTHER SCENARIOS - GetVisibleData
	int Variation_34();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(Resynch_Bufferred)
#define THE_CLASS Resynch_Bufferred
BEG_TEST_CASE(Resynch_Bufferred, CExecutionManager, L"Tests Notifications for IRowsetResynch_Bufferred::ResyncRows")
	TEST_VARIATION(1, 		L"ROW_RESYNCH - Accept all phases")
	TEST_VARIATION(2, 		L"Empty")
	TEST_VARIATION(3, 		L"Empty")
	TEST_VARIATION(4, 		L"ROW_RESYNCH - DB_E_CANCELED")
	TEST_VARIATION(5, 		L"ROW_RESYNCH - DB_E_NOTREENTRANT")
	TEST_VARIATION(6, 		L"Empty")
	TEST_VARIATION(7, 		L"Empty")
	TEST_VARIATION(8, 		L"ROW_RESYNCH - Veto OKTODO")
	TEST_VARIATION(9, 		L"ROW_RESYNCH - Veto ABOUTTODO")
	TEST_VARIATION(10, 		L"ROW_RESYNCH - Veto SYNCHAFTER")
	TEST_VARIATION(11, 		L"ROW_RESYNCH - Veto DIDEVENT")
	TEST_VARIATION(12, 		L"ROW_RESYNCH - Veto FAILEDTODO")
	TEST_VARIATION(13, 		L"Empty")
	TEST_VARIATION(14, 		L"Empty")
	TEST_VARIATION(15, 		L"ROW_RESYNCH - UNWANTED REASON")
	TEST_VARIATION(16, 		L"ROW_RESYNCH - UNWANTED PHASE - OKTODO")
	TEST_VARIATION(17, 		L"ROW_RESYNCH - UNWANTED PHASE - ABOUTTODO")
	TEST_VARIATION(18, 		L"ROW_RESYNCH - UNWANTED PHASE - SYNCHAFTER")
	TEST_VARIATION(19, 		L"ROW_RESYNCH - UNWANTED PHASE - DIDEVENT")
	TEST_VARIATION(20, 		L"ROW_RESYNCH - UNWANTED PHASE - FAILEDTODO")
	TEST_VARIATION(21, 		L"Empty")
	TEST_VARIATION(22, 		L"Empty")
	TEST_VARIATION(23, 		L"ROW_RESYNCH - E_FAIL - All Phases")
	TEST_VARIATION(24, 		L"ROW_RESYNCH - E_OUTOFMEMORY - All Phases")
	TEST_VARIATION(25, 		L"ROW_RESYNCH - E_INVALIDARG - All Phases")
	TEST_VARIATION(26, 		L"Empty")
	TEST_VARIATION(27, 		L"Empty")
	TEST_VARIATION(28, 		L"OTHER SCENARIOS - Resynch_Bufferred all rows")
	TEST_VARIATION(29, 		L"OTHER SCENARIOS - Resynch_Bufferred 2 rows, 1 changed 1 not changed")
	TEST_VARIATION(30, 		L"OTHER SCENARIOS - 1 valid, 1 not valid")
	TEST_VARIATION(31, 		L"OTHER SCENARIOS - EmptyRowset")
	TEST_VARIATION(32, 		L"OTHER SCENARIOS - Add/Remove Listeners")
	TEST_VARIATION(33, 		L"OTHER SCENARIOS - GetData")
	TEST_VARIATION(34, 		L"OTHER SCENARIOS - GetVisibleData")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(Undo)
//--------------------------------------------------------------------
// @class Tests Notifications for IRowsetUpdate::Undo
//
class Undo : public CExecutionManager { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Undo,CExecutionManager);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember ROW_UNDOCHANGE - Accept all phases
	int Variation_1();
	// @cmember Empty
	int Variation_2();
	// @cmember Empty
	int Variation_3();
	// @cmember ROW_UNDOCHANGE - DB_E_CANCELED
	int Variation_4();
	// @cmember ROW_UNDOCHANGE - DB_E_NOTREENTRANT
	int Variation_5();
	// @cmember Empty
	int Variation_6();
	// @cmember Empty
	int Variation_7();
	// @cmember ROW_UNDOCHANGE - Veto OKTODO
	int Variation_8();
	// @cmember ROW_UNDOCHANGE - Veto ABOUTTODO
	int Variation_9();
	// @cmember ROW_UNDOCHANGE - Veto SYNCHAFTER
	int Variation_10();
	// @cmember ROW_UNDOCHANGE - Veto DIDEVENT
	int Variation_11();
	// @cmember ROW_UNDOCHANGE - Veto FAILEDTODO
	int Variation_12();
	// @cmember Empty
	int Variation_13();
	// @cmember Empty
	int Variation_14();
	// @cmember ROW_UNDOCHANGE - UNWANTED REASON
	int Variation_15();
	// @cmember ROW_UNDOCHANGE - UNWANTED PHASE OKTODO
	int Variation_16();
	// @cmember ROW_UNDOCHANGE - UNWANTED PHASE ABOUTTODO
	int Variation_17();
	// @cmember ROW_UNDOCHANGE - UNWANTED PHASE SYNCHAFTER
	int Variation_18();
	// @cmember ROW_UNDOCHANGE - UNWANTED PHASE DIDEVENT
	int Variation_19();
	// @cmember ROW_UNDOCHANGE - UNWANTED PHASE FAILEDTODO
	int Variation_20();
	// @cmember Empty
	int Variation_21();
	// @cmember Empty
	int Variation_22();
	// @cmember ROW_UNDOCHANGE - E_FAIL - All Phases
	int Variation_23();
	// @cmember ROW_UNDOCHANGE - E_OUTOFMEMORY - All Phases
	int Variation_24();
	// @cmember ROW_UNDOCHANGE - E_INVALIDARG - All Phases
	int Variation_25();
	// @cmember Empty
	int Variation_26();
	// @cmember Empty
	int Variation_27();
	// @cmember ROW_UNDOINSERT - Accept all phases
	int Variation_28();
	// @cmember Empty
	int Variation_29();
	// @cmember Empty
	int Variation_30();
	// @cmember ROW_UNDOINSERT - DB_E_CANCELED
	int Variation_31();
	// @cmember ROW_UNDOINSERT - DB_E_NOTREENTRANT
	int Variation_32();
	// @cmember Empty
	int Variation_33();
	// @cmember Empty
	int Variation_34();
	// @cmember ROW_UNDOINSERT - Veto OKTODO
	int Variation_35();
	// @cmember ROW_UNDOINSERT - Veto ABOUTTODO
	int Variation_36();
	// @cmember ROW_UNDOINSERT - Veto SYNCHAFTER
	int Variation_37();
	// @cmember ROW_UNDOINSERT - Veto DIDEVENT
	int Variation_38();
	// @cmember ROW_UNDOINSERT - Veto FAILEDTODO
	int Variation_39();
	// @cmember Empty
	int Variation_40();
	// @cmember ROW_UNDOINSERT - UNWANTED REASON
	int Variation_41();
	// @cmember ROW_UNDOINSERT - UNWANTED PHASE OKTODO
	int Variation_42();
	// @cmember ROW_UNDOINSERT - UNWANTED PHASE ABOUTTO
	int Variation_43();
	// @cmember ROW_UNDOINSERT - UNWANTED PHASE SYNCHAFTER
	int Variation_44();
	// @cmember ROW_UNDOINSERT- UNWANTED PHASE DIDEVENT
	int Variation_45();
	// @cmember ROW_UNDOINSERT - UNWANTED PHASE FAILEDTODO
	int Variation_46();
	// @cmember Empty
	int Variation_47();
	// @cmember Empty
	int Variation_48();
	// @cmember ROW_UNDOINSERT - E_FAIL - All Phases
	int Variation_49();
	// @cmember ROW_UNDOINSERT - E_OUTOFMEMORY - All Phases
	int Variation_50();
	// @cmember ROW_UNDOINSERT - E_INVALIDARG - All Phases
	int Variation_51();
	// @cmember Empty
	int Variation_52();
	// @cmember Empty
	int Variation_53();
	// @cmember ROW_UNDODELETE - Accept all phases
	int Variation_54();
	// @cmember Empty
	int Variation_55();
	// @cmember Empty
	int Variation_56();
	// @cmember ROW_UNDODELETE - DB_E_CANCELED
	int Variation_57();
	// @cmember ROW_UNDODELETE - DB_E_NOTREENTRANT
	int Variation_58();
	// @cmember Empty
	int Variation_59();
	// @cmember Empty
	int Variation_60();
	// @cmember ROW_UNDODELETE - Veto OKTODO
	int Variation_61();
	// @cmember ROW_UNDODELETE - Veto ABOUTTODO
	int Variation_62();
	// @cmember ROW_UNDODELETE - Veto SYNCHAFTER
	int Variation_63();
	// @cmember ROW_UNDODELETE - Veto DIDEVENT
	int Variation_64();
	// @cmember ROW_UNDODELETE - Veto FAILEDTODO
	int Variation_65();
	// @cmember Empty
	int Variation_66();
	// @cmember Empty
	int Variation_67();
	// @cmember ROW_UNDODELETE - UNWANTED REASON
	int Variation_68();
	// @cmember ROW_UNDODELETE - UNWANTED PHASE OKTODO
	int Variation_69();
	// @cmember ROW_UNDODELETE - UNWANTED PHASE ABOUTTODO
	int Variation_70();
	// @cmember ROW_UNDODELETE - UNWANTED PHASE SYNCHAFTER
	int Variation_71();
	// @cmember ROW_UNDODELETE - UNWANTED PHASE DIDEVENT
	int Variation_72();
	// @cmember ROW_UNDODELETE - UNWANTED PHASE FAILEDTODO
	int Variation_73();
	// @cmember Empty
	int Variation_74();
	// @cmember Empty
	int Variation_75();
	// @cmember ROW_UNDODELETE - E_FAIL - All Phases
	int Variation_76();
	// @cmember ROW_UNDODELETE - E_OUTOFMEMORY - All Phases
	int Variation_77();
	// @cmember ROW_UNDODELETE - E_INVALIDARG - All Phases
	int Variation_78();
	// @cmember Empty
	int Variation_79();
	// @cmember Empty
	int Variation_80();
	// @cmember OTHER SCENARIOS - Undo unchanged row
	int Variation_81();
	// @cmember OTHER SCENARIOS - Undo all rows
	int Variation_82();
	// @cmember OTHER SCENARIOS - Undo changed and unchanged
	int Variation_83();
	// @cmember Empty
	int Variation_84();
	// @cmember OTHER SCENARIOS - Verify Undo clears the FIRSTCHANGE status
	int Variation_85();
	// @cmember Empty
	int Variation_86();
	// @cmember OTHER SCENARIOS - DBROWSTATUS_E_INVALID
	int Variation_87();
	// @cmember OTHER SCENARIOS - EmptyRowset
	int Variation_88();
	// @cmember OTHER SCENARIOS - Add/Remove Listeners
	int Variation_89();
	// @cmember OTHER SCENARIOS - GetData
	int Variation_90();
	// @cmember OTHER SCENARIOS - GetOriginalData
	int Variation_91();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(Undo)
#define THE_CLASS Undo
BEG_TEST_CASE(Undo, CExecutionManager, L"Tests Notifications for IRowsetUpdate::Undo")
	TEST_VARIATION(1, 		L"ROW_UNDOCHANGE - Accept all phases")
	TEST_VARIATION(2, 		L"Empty")
	TEST_VARIATION(3, 		L"Empty")
	TEST_VARIATION(4, 		L"ROW_UNDOCHANGE - DB_E_CANCELED")
	TEST_VARIATION(5, 		L"ROW_UNDOCHANGE - DB_E_NOTREENTRANT")
	TEST_VARIATION(6, 		L"Empty")
	TEST_VARIATION(7, 		L"Empty")
	TEST_VARIATION(8, 		L"ROW_UNDOCHANGE - Veto OKTODO")
	TEST_VARIATION(9, 		L"ROW_UNDOCHANGE - Veto ABOUTTODO")
	TEST_VARIATION(10, 		L"ROW_UNDOCHANGE - Veto SYNCHAFTER")
	TEST_VARIATION(11, 		L"ROW_UNDOCHANGE - Veto DIDEVENT")
	TEST_VARIATION(12, 		L"ROW_UNDOCHANGE - Veto FAILEDTODO")
	TEST_VARIATION(13, 		L"Empty")
	TEST_VARIATION(14, 		L"Empty")
	TEST_VARIATION(15, 		L"ROW_UNDOCHANGE - UNWANTED REASON")
	TEST_VARIATION(16, 		L"ROW_UNDOCHANGE - UNWANTED PHASE OKTODO")
	TEST_VARIATION(17, 		L"ROW_UNDOCHANGE - UNWANTED PHASE ABOUTTODO")
	TEST_VARIATION(18, 		L"ROW_UNDOCHANGE - UNWANTED PHASE SYNCHAFTER")
	TEST_VARIATION(19, 		L"ROW_UNDOCHANGE - UNWANTED PHASE DIDEVENT")
	TEST_VARIATION(20, 		L"ROW_UNDOCHANGE - UNWANTED PHASE FAILEDTODO")
	TEST_VARIATION(21, 		L"Empty")
	TEST_VARIATION(22, 		L"Empty")
	TEST_VARIATION(23, 		L"ROW_UNDOCHANGE - E_FAIL - All Phases")
	TEST_VARIATION(24, 		L"ROW_UNDOCHANGE - E_OUTOFMEMORY - All Phases")
	TEST_VARIATION(25, 		L"ROW_UNDOCHANGE - E_INVALIDARG - All Phases")
	TEST_VARIATION(26, 		L"Empty")
	TEST_VARIATION(27, 		L"Empty")
	TEST_VARIATION(28, 		L"ROW_UNDOINSERT - Accept all phases")
	TEST_VARIATION(29, 		L"Empty")
	TEST_VARIATION(30, 		L"Empty")
	TEST_VARIATION(31, 		L"ROW_UNDOINSERT - DB_E_CANCELED")
	TEST_VARIATION(32, 		L"ROW_UNDOINSERT - DB_E_NOTREENTRANT")
	TEST_VARIATION(33, 		L"Empty")
	TEST_VARIATION(34, 		L"Empty")
	TEST_VARIATION(35, 		L"ROW_UNDOINSERT - Veto OKTODO")
	TEST_VARIATION(36, 		L"ROW_UNDOINSERT - Veto ABOUTTODO")
	TEST_VARIATION(37, 		L"ROW_UNDOINSERT - Veto SYNCHAFTER")
	TEST_VARIATION(38, 		L"ROW_UNDOINSERT - Veto DIDEVENT")
	TEST_VARIATION(39, 		L"ROW_UNDOINSERT - Veto FAILEDTODO")
	TEST_VARIATION(40, 		L"Empty")
	TEST_VARIATION(41, 		L"ROW_UNDOINSERT - UNWANTED REASON")
	TEST_VARIATION(42, 		L"ROW_UNDOINSERT - UNWANTED PHASE OKTODO")
	TEST_VARIATION(43, 		L"ROW_UNDOINSERT - UNWANTED PHASE ABOUTTO")
	TEST_VARIATION(44, 		L"ROW_UNDOINSERT - UNWANTED PHASE SYNCHAFTER")
	TEST_VARIATION(45, 		L"ROW_UNDOINSERT- UNWANTED PHASE DIDEVENT")
	TEST_VARIATION(46, 		L"ROW_UNDOINSERT - UNWANTED PHASE FAILEDTODO")
	TEST_VARIATION(47, 		L"Empty")
	TEST_VARIATION(48, 		L"Empty")
	TEST_VARIATION(49, 		L"ROW_UNDOINSERT - E_FAIL - All Phases")
	TEST_VARIATION(50, 		L"ROW_UNDOINSERT - E_OUTOFMEMORY - All Phases")
	TEST_VARIATION(51, 		L"ROW_UNDOINSERT - E_INVALIDARG - All Phases")
	TEST_VARIATION(52, 		L"Empty")
	TEST_VARIATION(53, 		L"Empty")
	TEST_VARIATION(54, 		L"ROW_UNDODELETE - Accept all phases")
	TEST_VARIATION(55, 		L"Empty")
	TEST_VARIATION(56, 		L"Empty")
	TEST_VARIATION(57, 		L"ROW_UNDODELETE - DB_E_CANCELED")
	TEST_VARIATION(58, 		L"ROW_UNDODELETE - DB_E_NOTREENTRANT")
	TEST_VARIATION(59, 		L"Empty")
	TEST_VARIATION(60, 		L"Empty")
	TEST_VARIATION(61, 		L"ROW_UNDODELETE - Veto OKTODO")
	TEST_VARIATION(62, 		L"ROW_UNDODELETE - Veto ABOUTTODO")
	TEST_VARIATION(63, 		L"ROW_UNDODELETE - Veto SYNCHAFTER")
	TEST_VARIATION(64, 		L"ROW_UNDODELETE - Veto DIDEVENT")
	TEST_VARIATION(65, 		L"ROW_UNDODELETE - Veto FAILEDTODO")
	TEST_VARIATION(66, 		L"Empty")
	TEST_VARIATION(67, 		L"Empty")
	TEST_VARIATION(68, 		L"ROW_UNDODELETE - UNWANTED REASON")
	TEST_VARIATION(69, 		L"ROW_UNDODELETE - UNWANTED PHASE OKTODO")
	TEST_VARIATION(70, 		L"ROW_UNDODELETE - UNWANTED PHASE ABOUTTODO")
	TEST_VARIATION(71, 		L"ROW_UNDODELETE - UNWANTED PHASE SYNCHAFTER")
	TEST_VARIATION(72, 		L"ROW_UNDODELETE - UNWANTED PHASE DIDEVENT")
	TEST_VARIATION(73, 		L"ROW_UNDODELETE - UNWANTED PHASE FAILEDTODO")
	TEST_VARIATION(74, 		L"Empty")
	TEST_VARIATION(75, 		L"Empty")
	TEST_VARIATION(76, 		L"ROW_UNDODELETE - E_FAIL - All Phases")
	TEST_VARIATION(77, 		L"ROW_UNDODELETE - E_OUTOFMEMORY - All Phases")
	TEST_VARIATION(78, 		L"ROW_UNDODELETE - E_INVALIDARG - All Phases")
	TEST_VARIATION(79, 		L"Empty")
	TEST_VARIATION(80, 		L"Empty")
	TEST_VARIATION(81, 		L"OTHER SCENARIOS - Undo unchanged row")
	TEST_VARIATION(82, 		L"OTHER SCENARIOS - Undo all rows")
	TEST_VARIATION(83, 		L"OTHER SCENARIOS - Undo changed and unchanged")
	TEST_VARIATION(84, 		L"Empty")
	TEST_VARIATION(85, 		L"OTHER SCENARIOS - Verify Undo clears the FIRSTCHANGE status")
	TEST_VARIATION(86, 		L"Empty")
	TEST_VARIATION(87, 		L"OTHER SCENARIOS - DBROWSTATUS_E_INVALID")
	TEST_VARIATION(88, 		L"OTHER SCENARIOS - EmptyRowset")
	TEST_VARIATION(89, 		L"OTHER SCENARIOS - Add/Remove Listeners")
	TEST_VARIATION(90, 		L"OTHER SCENARIOS - GetData")
	TEST_VARIATION(91, 		L"OTHER SCENARIOS - GetOriginalData")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(Update)
//--------------------------------------------------------------------
// @class Tests Notifications for IRowsetUpdate::Update
//
class Update : public CExecutionManager { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Update,CExecutionManager);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember ROW_UPDATE - Accept all phases
	int Variation_1();
	// @cmember Empty
	int Variation_2();
	// @cmember Empty
	int Variation_3();
	// @cmember ROW_UPDATE - DB_E_CANCELED
	int Variation_4();
	// @cmember ROW_UPDATE - DB_E_NOTREENTRANT
	int Variation_5();
	// @cmember Empty
	int Variation_6();
	// @cmember Empty
	int Variation_7();
	// @cmember ROW_UPDATE - Veto OKTODO
	int Variation_8();
	// @cmember ROW_UPDATE - Veto ABOUTTODO
	int Variation_9();
	// @cmember ROW_UPDATE - Veto SYNCHAFTER
	int Variation_10();
	// @cmember ROW_UPDATE - Veto DIDEVENT
	int Variation_11();
	// @cmember ROW_UPDATE - Veto FAILEDTODO
	int Variation_12();
	// @cmember Empty
	int Variation_13();
	// @cmember Empty
	int Variation_14();
	// @cmember ROW_UPDATE - UNWANTED REASON
	int Variation_15();
	// @cmember ROW_UPDATE - UNWANTED PHASE - OKTODO
	int Variation_16();
	// @cmember ROW_UPDATE - UNWANTED PHASE - ABOUTTODO
	int Variation_17();
	// @cmember ROW_UPDATE - UNWANTED PHASE - SYNCHAFTER
	int Variation_18();
	// @cmember ROW_UPDATE - UNWANTED PHASE - DIDEVENT
	int Variation_19();
	// @cmember ROW_UPDATE - UNWANTED PHASE - FAILEDTODO
	int Variation_20();
	// @cmember Empty
	int Variation_21();
	// @cmember Empty
	int Variation_22();
	// @cmember ROW_UPDATE - E_FAIL - All Phases
	int Variation_23();
	// @cmember ROW_UPDATE - E_OUTOFMEMORY - All Phases
	int Variation_24();
	// @cmember ROW_UPDATE - E_INVALIDARG - All Phases
	int Variation_25();
	// @cmember Empty
	int Variation_26();
	// @cmember Empty
	int Variation_27();
	// @cmember OTHER SCENARIOS - Update all [no changes]
	int Variation_28();
	// @cmember OTHER SCENARIOS - Update all [changes]
	int Variation_29();
	// @cmember OTHER SCENARIOS - Update changed and unchanged
	int Variation_30();
	// @cmember Empty
	int Variation_31();
	// @cmember OTHER SCENARIOS - Verify Update clears FIRSTCHANGE status
	int Variation_32();
	// @cmember Empty
	int Variation_33();
	// @cmember OTHER SCENARIOS - DBROWSTATUS_E_INVALID
	int Variation_34();
	// @cmember OTHER SCENARIOS - EmptyRowset
	int Variation_35();
	// @cmember OTHER SCENARIOS - Add/Remove Listeners
	int Variation_36();
	// @cmember OTHER SCENARIOS - GetData
	int Variation_37();
	// @cmember OTHER SCENARIOS - GetOriginalData
	int Variation_38();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(Update)
#define THE_CLASS Update
BEG_TEST_CASE(Update, CExecutionManager, L"Tests Notifications for IRowsetUpdate::Update")
	TEST_VARIATION(1, 		L"ROW_UPDATE - Accept all phases")
	TEST_VARIATION(2, 		L"Empty")
	TEST_VARIATION(3, 		L"Empty")
	TEST_VARIATION(4, 		L"ROW_UPDATE - DB_E_CANCELED")
	TEST_VARIATION(5, 		L"ROW_UPDATE - DB_E_NOTREENTRANT")
	TEST_VARIATION(6, 		L"Empty")
	TEST_VARIATION(7, 		L"Empty")
	TEST_VARIATION(8, 		L"ROW_UPDATE - Veto OKTODO")
	TEST_VARIATION(9, 		L"ROW_UPDATE - Veto ABOUTTODO")
	TEST_VARIATION(10, 		L"ROW_UPDATE - Veto SYNCHAFTER")
	TEST_VARIATION(11, 		L"ROW_UPDATE - Veto DIDEVENT")
	TEST_VARIATION(12, 		L"ROW_UPDATE - Veto FAILEDTODO")
	TEST_VARIATION(13, 		L"Empty")
	TEST_VARIATION(14, 		L"Empty")
	TEST_VARIATION(15, 		L"ROW_UPDATE - UNWANTED REASON")
	TEST_VARIATION(16, 		L"ROW_UPDATE - UNWANTED PHASE - OKTODO")
	TEST_VARIATION(17, 		L"ROW_UPDATE - UNWANTED PHASE - ABOUTTODO")
	TEST_VARIATION(18, 		L"ROW_UPDATE - UNWANTED PHASE - SYNCHAFTER")
	TEST_VARIATION(19, 		L"ROW_UPDATE - UNWANTED PHASE - DIDEVENT")
	TEST_VARIATION(20, 		L"ROW_UPDATE - UNWANTED PHASE - FAILEDTODO")
	TEST_VARIATION(21, 		L"Empty")
	TEST_VARIATION(22, 		L"Empty")
	TEST_VARIATION(23, 		L"ROW_UPDATE - E_FAIL - All Phases")
	TEST_VARIATION(24, 		L"ROW_UPDATE - E_OUTOFMEMORY - All Phases")
	TEST_VARIATION(25, 		L"ROW_UPDATE - E_INVALIDARG - All Phases")
	TEST_VARIATION(26, 		L"Empty")
	TEST_VARIATION(27, 		L"Empty")
	TEST_VARIATION(28, 		L"OTHER SCENARIOS - Update all [no changes]")
	TEST_VARIATION(29, 		L"OTHER SCENARIOS - Update all [changes]")
	TEST_VARIATION(30, 		L"OTHER SCENARIOS - Update changed and unchanged")
	TEST_VARIATION(31, 		L"Empty")
	TEST_VARIATION(32, 		L"OTHER SCENARIOS - Verify Update clears FIRSTCHANGE status")
	TEST_VARIATION(33, 		L"Empty")
	TEST_VARIATION(34, 		L"OTHER SCENARIOS - DBROWSTATUS_E_INVALID")
	TEST_VARIATION(35, 		L"OTHER SCENARIOS - EmptyRowset")
	TEST_VARIATION(36, 		L"OTHER SCENARIOS - Add/Remove Listeners")
	TEST_VARIATION(37, 		L"OTHER SCENARIOS - GetData")
	TEST_VARIATION(38, 		L"OTHER SCENARIOS - GetOriginalData")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// }} END_DECLARE_TEST_CASES()

// {{ TCW_TESTMODULE(ThisModule)
TEST_MODULE(18, ThisModule, gwszModuleDescrip)
	TEST_CASE(1, Release)
	TEST_CASE(2, GetNextRows)
	TEST_CASE(3, ReleaseRows)
	TEST_CASE(4, RestartPosition)
	TEST_CASE(5, Seek)
	TEST_CASE(6, GetRowsAt)
	TEST_CASE(7, GetRowsByBookmark)
	TEST_CASE(8, GetRowsAtRatio)
	TEST_CASE(9, DeleteRows)
	TEST_CASE(10, DeleteRows_Bufferred)
	TEST_CASE(11, InsertRow)
	TEST_CASE(12, InsertRow_Bufferred)
	TEST_CASE(13, SetData)
	TEST_CASE(14, SetData_Bufferred)
	TEST_CASE(15, Resynch)
	TEST_CASE(16, Resynch_Bufferred)
	TEST_CASE(17, Undo)
	TEST_CASE(18, Update)
END_TEST_MODULE()
// }} TCW_TESTMODULE_END


// {{ TCW_TC_PROTOTYPE(Release)
//*-----------------------------------------------------------------------
//| Test Case:		Release - Tests Notifications for IRowset::Release
//|	Created:			09/29/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Release::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CExecutionManager::Init())
	// }}
	{
		//Verify Rowset can be created...
		if(CreateRowset(DEFAULT_ROWSET)==S_OK)
			return TRUE;
	}

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc ROWSET_RELEASE - Accept all phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Release::Variation_1()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{IRowset::Release() {} }"
		L"	}"
		L"}";

	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Release::Variation_2()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Release::Variation_3()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc ROWSET_RELEASE - DB_E_CANCELED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Release::Variation_4()
{
	//Cannot "successfully" Veto DIDEVENT, so there is no use trying to 
	//use the rowset after Release() since it will no longer be valid

	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::Release() "
		L"			{L1.ROWSET_RELEASE.DIDEVENT.RETURN_VETO } "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc ROWSET_RELEASE - DB_E_NOTREENTRANT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Release::Variation_5()
{
	//This really isn't a good senario to be testing DB_E_NOTREENTRANT
	//Once the rowset sends DIDEVENT for IRowset::Release() its fairly dangerous
	//to do anything!!

	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::Release() "
		L"			{L1.ROWSET_RELEASE.DIDEVENT.RETURN_ACCEPT } "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Release::Variation_6()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Release::Variation_7()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc ROWSET_RELEASE - Veto DIDEVENT, Listener 1
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Release::Variation_8()
{
	//Cannot "successfully" Veto DIDEVENT, so there is no use trying to 
	//use the rowset after Release() since it will no longer be valid

	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::Release() "
		L"			{L1.ROWSET_RELEASE.DIDEVENT.RETURN_VETO } "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Release::Variation_9()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Release::Variation_10()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc ROWSET_RELEASE - UNWANTED REASON
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Release::Variation_11()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::Release() "
		L"			{L1.ROWSET_RELEASE.DIDEVENT.RETURN_UNWANTEDREASON } "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc ROWSET_RELEASE - UNWANTED PHASE - DIDEVENT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Release::Variation_12()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::Release() "
		L"			{L1.ROWSET_RELEASE.DIDEVENT.RETURN_UNWANTEDPHASE } "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Release::Variation_13()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Release::Variation_14()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc ROWSET_RELEASE - E_FAIL - All Phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Release::Variation_15()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::Release() "
		L"			{L1.ROWSET_RELEASE.DIDEVENT.RETURN_EFAIL } "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc ROWSET_RELEASE - E_OUTOFMEMORY - All Phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Release::Variation_16()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::Release() "
		L"			{L1.ROWSET_RELEASE.DIDEVENT.RETURN_EOUTOFMEMORY } "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc ROWSET_RELEASE - E_INVALIDARG - All Phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Release::Variation_17()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::Release() "
		L"			{L1.ROWSET_RELEASE.DIDEVENT.RETURN_EINVALIDARG } "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Release::Variation_18()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Release::Variation_19()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - AddRef - Release - no notifications
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Release::Variation_20()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::AddRef() {} "
		L"		 IRowset::Release() {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - EmptyRowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Release::Variation_21()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::Release() "
		L"			{L1.ROWSET_RELEASE.DIDEVENT.RETURN_ACCEPT } "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, EMPTY_ROWSET | DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(22)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - Remove Listeners
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Release::Variation_22()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::Release() "
		L"			{"
//		L"			 L1.ROWSET_RELEASE.DIDEVENT.RETURN_UNADVISE "
//		L"			 L2.ROWSET_RELEASE.DIDEVENT.RETURN_UNADVISE "
		L"			}"
		L"		}"
		L"	}"
		L"}";

	//NOTE:  On ROWSET_RELEASE notification the rowset pointer is already released.  
	//So trying to QI for IConnectionPointContainer to advise/unadvise listeners may crash
	//and is considered a user programming error...

	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(23)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - Add Listeners
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Release::Variation_23()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::Release() "
		L"			{"
//		L"			 L1.ROWSET_RELEASE.DIDEVENT.RETURN_ADVISE "
//		L"			 L2.ROWSET_RELEASE.DIDEVENT.RETURN_ADVISE "
		L"			}"
		L"		}"
		L"	}"
		L"}";


	//NOTE:  On ROWSET_RELEASE notification the rowset pointer is already released.  
	//So trying to QI for IConnectionPointContainer to advise/unadvise listeners may crash
	//and is considered a user programming error...

	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}
// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Release::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CExecutionManager::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(GetNextRows)
//*-----------------------------------------------------------------------
//| Test Case:		GetNextRows - Tests Notifications for IRowset::GetNextRows
//|	Created:			09/27/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL GetNextRows::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CExecutionManager::Init())
	// }}
	{
		//Verify Rowset can be created...
		if(CreateRowset(DEFAULT_ROWSET)==S_OK)
			return TRUE;
	}

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc FETCHPOSITIONCHANGE - SINGLEROW - Accept all phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetNextRows::Variation_1()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{IRowset::GetNextRows(A) {} }"
		L"	}"
		L"}";

	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc FETCHPOSITIONCHANGE - MULTIPLEROWS - Accept all phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetNextRows::Variation_2()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{IRowset::GetNextRows(A,B,C) {} } "
		L"	}"
		L"}";

	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetNextRows::Variation_3()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetNextRows::Variation_4()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc FETCHPOSITIONCHANGE - DB_E_CANCELED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetNextRows::Variation_5()
{
	//Verify the FetchPosition did not change when the Veto occurs.
	//Obtain the first row and veto.  If method actually succeeds, the next
	//GetNextRows will fail since it thinks that row has already been retreived...
	
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A) "
	    L"			{L1.ROWSET_FETCHPOSITIONCHANGE.ABOUTTODO.RETURN_VETO} "
		L"		 IRowset::GetNextRows(A) "
	    L"			{L1.ROWSET_FETCHPOSITIONCHANGE.ABOUTTODO.RETURN_ACCEPT} "
		L"		}"
		L"	}"
		L"}";

	//To produce FAILEDTODO, this reason/phase must be vetoable...
	TESTC_PROVIDER(IsCancelableEvent(DBREASON_ROWSET_FETCHPOSITIONCHANGE, DBEVENTPHASE_ABOUTTODO));
	TESTC(Execute(CommandString, DEFAULT_ROWSET));

CLEANUP:
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc FETCHPOSITIONCHANGE - DB_E_NOTREENTRANT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetNextRows::Variation_6()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{IRowset::GetNextRows(A,B) "
	    L"			{L1.ROWSET_FETCHPOSITIONCHANGE.OKTODO.IRowset::GetNextRows(C,D,DB_E_NOTREENTRANT) {} }" 
		L"		} "
		L"	} "
		L"} ";

	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc FETCHPOSITIONCHANGE - DB_E_NOTREENTRANT - Non-notification method
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetNextRows::Variation_7()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{IRowset::GetNextRows(A,B) "
//	    L"			{L1.ROWSET_FETCHPOSITIONCHANGE.OKTODO.IColumnsInfo::GetColumnInfo(DB_E_NOTREENTRANT){}}"
	    L"			{L1.ROWSET_FETCHPOSITIONCHANGE.OKTODO.Thread{IColumnsInfo::GetColumnInfo(DB_E_NOTREENTRANT){}}	}" 
		L"		} "
		L"	} "
		L"} ";

	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetNextRows::Variation_8()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc FETCHPOSITIONCHANGE - Veto OKTODO, Listener 1
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetNextRows::Variation_9()
{
	//Have L1 VETO the change on the OKTODO phase
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{IRowset::GetNextRows(A,B) "
	    L"			{L1.ROWSET_FETCHPOSITIONCHANGE.OKTODO.RETURN_VETO} "
		L"		} "
		L"	}"
		L"}";
	
	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc FETCHPOSITIONCHANGE - Veto ABOUTTODO, Listener 1
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetNextRows::Variation_10()
{
	//Have L1 VETO the change on the ABOUTTODO phase
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{IRowset::GetNextRows(A,B) "
	    L"			{L1.ROWSET_FETCHPOSITIONCHANGE.ABOUTTODO.RETURN_VETO} "
		L"		} "
		L"	}"
		L"}";
	
	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc FETCHPOSITIONCHANGE - Veto SYNCHAFTER, Listener 1
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetNextRows::Variation_11()
{
	//Have L1 VETO the change on the SYNCHAFTER phase
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{IRowset::GetNextRows(A,B) "
	    L"			{L1.ROWSET_FETCHPOSITIONCHANGE.SYNCHAFTER.RETURN_VETO} "
		L"		} "
		L"	}"
		L"}";
	
	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc FETCHPOSITIONCHANGE - Veto DIDEVENT, Listener 1
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetNextRows::Variation_12()
{
	//Have L1 VETO the change on the DIDEVENT phase
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{IRowset::GetNextRows(A,B) "
	    L"			{L1.ROWSET_FETCHPOSITIONCHANGE.DIDEVENT.RETURN_VETO} "
		L"		} "
		L"	}"
		L"}";
	
	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc FETCHPOSITIONCHANGE - Veto FAILEDTODO, Listener 1
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetNextRows::Variation_13()
{
	//Have L1 VETO the change on the OKTODO phase
	//Then try and have it VETO the FAILEDTODO phase as well!
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{IRowset::GetNextRows(A,B) "
	    L"			{ "
		L"			 L1.ROWSET_FETCHPOSITIONCHANGE.ABOUTTODO.RETURN_VETO "
		L"			 L1.ROWSET_FETCHPOSITIONCHANGE.FAILEDTODO.RETURN_VETO "
		L"			} "
		L"		} "
		L"	}"
		L"}";

	//To produce FAILEDTODO, this reason/phase must be vetoable...
	TESTC_PROVIDER(IsCancelableEvent(DBREASON_ROWSET_FETCHPOSITIONCHANGE, DBEVENTPHASE_ABOUTTODO));
	TESTC(Execute(CommandString, DEFAULT_ROWSET));

CLEANUP:
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc FETCHPOSITIONCHANGE - Veto OKTODO, Listener 2
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetNextRows::Variation_14()
{
	//Have L2 VETO the change on the OKTODO phase
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{IRowset::GetNextRows(A,B) "
	    L"			{L2.ROWSET_FETCHPOSITIONCHANGE.OKTODO.RETURN_VETO} "
		L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc FETCHPOSITIONCHANGE - Veto ABOUTTODO, Listener 2
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetNextRows::Variation_15()
{
	//Have L2 VETO the change on the ABOUTTODO phase
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{IRowset::GetNextRows(A,B) "
	    L"			{L2.ROWSET_FETCHPOSITIONCHANGE.ABOUTTODO.RETURN_VETO} "
		L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc FETCHPOSITIONCHANGE -  Veto SYNCHAFTER, Listener 3
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetNextRows::Variation_16()
{
	//Have L3 VETO the change on the SYNCHAFTER phase
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{IRowset::GetNextRows(A,B) "
	    L"			{L3.ROWSET_FETCHPOSITIONCHANGE.SYNCHAFTER.RETURN_VETO} "
		L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetNextRows::Variation_17()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetNextRows::Variation_18()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc FETCHPOSITIONCHANGE - UNWANTED REASON
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetNextRows::Variation_19()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{IRowset::GetNextRows(A) "
 	    L"			{L3.ROWSET_FETCHPOSITIONCHANGE.ABOUTTODO.RETURN_UNWANTEDREASON} "
		L"		 IRowset::GetNextRows(B) {} "
	    L"		} "
		L"	} "
		L"}";
	
	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc FETCHPOSITIONCHANGE - UNWANTED PHASE - OKTODO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetNextRows::Variation_20()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{IRowset::GetNextRows(A) "
	    L"			{L3.ROWSET_FETCHPOSITIONCHANGE.OKTODO.RETURN_UNWANTEDPHASE} "
		L"		 IRowset::GetNextRows(B) {} "
	    L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc FETCHPOSITIONCHANGE - UNWANTED PHASE - ABOUTTODO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetNextRows::Variation_21()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{IRowset::GetNextRows(A) "
	    L"			{L3.ROWSET_FETCHPOSITIONCHANGE.ABOUTTODO.RETURN_UNWANTEDPHASE} "
		L"		 IRowset::GetNextRows(B) {} "
	    L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(22)
//*-----------------------------------------------------------------------
// @mfunc FETCHPOSITIONCHANGE - UNWANTED PHASE - SYNCHAFTER
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetNextRows::Variation_22()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{IRowset::GetNextRows(A) "
	    L"			{L3.ROWSET_FETCHPOSITIONCHANGE.SYNCHAFTER.RETURN_UNWANTEDPHASE} "
		L"		 IRowset::GetNextRows(B) {} "
	    L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(23)
//*-----------------------------------------------------------------------
// @mfunc FETCHPOSITIONCHANGE - UNWANTED PHASE - DIDENVENT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetNextRows::Variation_23()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{IRowset::GetNextRows(A) "
	    L"			{L3.ROWSET_FETCHPOSITIONCHANGE.DIDEVENT.RETURN_UNWANTEDPHASE} "
		L"		 IRowset::GetNextRows(B) {} "
	    L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(24)
//*-----------------------------------------------------------------------
// @mfunc FETCHPOSITIONCHANGE - UNWANTED PHASE - FAILEDTODO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetNextRows::Variation_24()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{IRowset::GetNextRows(A) "
	    L"			{"
		L"			 L3.ROWSET_FETCHPOSITIONCHANGE.ABOUTTODO.RETURN_VETO "
	    L"			 L3.ROWSET_FETCHPOSITIONCHANGE.FAILEDTODO.RETURN_UNWANTEDPHASE "
		L"			} "
		L"		 IRowset::GetNextRows(B) {} "
	    L"		}"
		L"	}"
		L"}";
	
	//To produce FAILEDTODO, this reason/phase must be vetoable...
	TESTC_PROVIDER(IsCancelableEvent(DBREASON_ROWSET_FETCHPOSITIONCHANGE, DBEVENTPHASE_ABOUTTODO));
	TESTC(Execute(CommandString, DEFAULT_ROWSET));

CLEANUP:
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(25)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetNextRows::Variation_25()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(26)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetNextRows::Variation_26()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(27)
//*-----------------------------------------------------------------------
// @mfunc FETCHPOSITIONCHANGE - E_FAIL - All Phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetNextRows::Variation_27()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{IRowset::GetNextRows(A) "
	    L"			{"
		L"			 L1.ROWSET_FETCHPOSITIONCHANGE.OKTODO.RETURN_EFAIL "
		L"			 L2.ROWSET_FETCHPOSITIONCHANGE.ABOUTTODO.RETURN_EFAIL "
		L"			 L3.ROWSET_FETCHPOSITIONCHANGE.SYNCHAFTER.RETURN_EFAIL "
		L"			 L1.ROWSET_FETCHPOSITIONCHANGE.DIDEVENT.RETURN_EFAIL "
		L"			} "
		L"		 IRowset::GetNextRows(B) "
		L"			{ "
		L"			 L1.ROWSET_FETCHPOSITIONCHANGE.ABOUTTODO.RETURN_VETO "
		L"			} "
	    L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(28)
//*-----------------------------------------------------------------------
// @mfunc FETCHPOSITIONCHANGE - E_OUTOFMEMORY - All Phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetNextRows::Variation_28()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{IRowset::GetNextRows(A) "
	    L"			{"
		L"			 L1.ROWSET_FETCHPOSITIONCHANGE.OKTODO.RETURN_EOUTOFMEMORY "
		L"			 L2.ROWSET_FETCHPOSITIONCHANGE.ABOUTTODO.RETURN_EOUTOFMEMORY "
		L"			 L3.ROWSET_FETCHPOSITIONCHANGE.SYNCHAFTER.RETURN_EOUTOFMEMORY "
		L"			 L1.ROWSET_FETCHPOSITIONCHANGE.DIDEVENT.RETURN_EOUTOFMEMORY "
		L"			} "
		L"		 IRowset::GetNextRows(B) "
		L"			{ "
		L"			 L1.ROWSET_FETCHPOSITIONCHANGE.ABOUTTODO.RETURN_VETO "
		L"			} "
	    L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(29)
//*-----------------------------------------------------------------------
// @mfunc FETCHPOSITIONCHANGE - E_INVALIDARG - All Phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetNextRows::Variation_29()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{IRowset::GetNextRows(A) "
	    L"			{"
		L"			 L1.ROWSET_FETCHPOSITIONCHANGE.OKTODO.RETURN_EINVALIDARG "
		L"			 L2.ROWSET_FETCHPOSITIONCHANGE.ABOUTTODO.RETURN_EINVALIDARG "
		L"			 L3.ROWSET_FETCHPOSITIONCHANGE.SYNCHAFTER.RETURN_EINVALIDARG "
		L"			 L1.ROWSET_FETCHPOSITIONCHANGE.DIDEVENT.RETURN_EINVALIDARG "
		L"			} "
		L"		 IRowset::GetNextRows(B) "
		L"			{ "
		L"			 L1.ROWSET_FETCHPOSITIONCHANGE.ABOUTTODO.RETURN_VETO "
		L"			} "
	    L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(30)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetNextRows::Variation_30()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(31)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetNextRows::Variation_31()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(32)
//*-----------------------------------------------------------------------
// @mfunc ROW_ACTIVATE - SINGLEROW - Accept all phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetNextRows::Variation_32()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{IRowset::GetNextRows(A) {} }"
		L"	}"
		L"}";

	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(33)
//*-----------------------------------------------------------------------
// @mfunc ROW_ACTIVATE - MULTIPLEROWS - Accept all phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetNextRows::Variation_33()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{IRowset::GetNextRows(A,B,C) {} }"
		L"	}"
		L"}";

	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(34)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetNextRows::Variation_34()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(35)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetNextRows::Variation_35()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(36)
//*-----------------------------------------------------------------------
// @mfunc ROW_ACTIVATE - DB_E_CANCELED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetNextRows::Variation_36()
{
	//Cannot "successfully" veto DIDEVENT, so there is really nothing 
	//to test on the Veto, besides making sure they are valid row
	//handles, which is the purpose of the GetData's

	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) "
	    L"			{L3.ROW_ACTIVATE.DIDEVENT.RETURN_VETO} "
		L"		 IRowset::GetData(A) {} "
		L"		 IRowset::GetData(B) {} "
		L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(37)
//*-----------------------------------------------------------------------
// @mfunc ROW_ACTIVATE - DB_E_NOTREENTRANT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetNextRows::Variation_37()
{
	//Have L1 Call GetNextRows on the DIDEVENT phase
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{IRowset::GetNextRows(A,B) "
	    L"			{"
		L"			 L3.ROW_ACTIVATE.DIDEVENT.IRowset::GetNextRows(C,D,DB_E_NOTREENTRANT) {} "
		L"			}" 
		L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(38)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetNextRows::Variation_38()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(39)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetNextRows::Variation_39()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(40)
//*-----------------------------------------------------------------------
// @mfunc ROW_ACTIVATE - Veto DIDEVENT, Listener 1
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetNextRows::Variation_40()
{
	//Have L1 VETO the change on the DIDEVENT phase
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{IRowset::GetNextRows(A,B) "
	    L"			{"
		L"			 L3.ROW_ACTIVATE.DIDEVENT.RETURN_VETO "
		L"			} "
		L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(41)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetNextRows::Variation_41()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(42)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetNextRows::Variation_42()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(43)
//*-----------------------------------------------------------------------
// @mfunc ROW_ACTIVATE - UNWANTED REASON
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetNextRows::Variation_43()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {L3.ROW_ACTIVATE.DIDEVENT.RETURN_UNWANTEDREASON} "
		L"		 IRowset::GetNextRows(C,D) {} "
		L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(44)
//*-----------------------------------------------------------------------
// @mfunc ROW_ACTIVATE - UNWANTED PHASE - DIDEVENT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetNextRows::Variation_44()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {L3.ROW_ACTIVATE.DIDEVENT.RETURN_UNWANTEDPHASE} "
		L"		 IRowset::GetNextRows(C,D) {} "
		L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(45)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetNextRows::Variation_45()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(46)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetNextRows::Variation_46()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(47)
//*-----------------------------------------------------------------------
// @mfunc ROW_ACTIVATE - E_FAIL - All Phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetNextRows::Variation_47()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{IRowset::GetNextRows(A) "
	    L"			{"
		L"			 L1.ROW_ACTIVATE.DIDEVENT.RETURN_EFAIL "
		L"			} "
		L"		 IRowset::GetNextRows(B) "
		L"			{ "
		L"			} "
	    L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(48)
//*-----------------------------------------------------------------------
// @mfunc ROW_ACTIVATE - E_OUTOFMEMORY - All Phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetNextRows::Variation_48()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{IRowset::GetNextRows(A) "
	    L"			{"
		L"			 L1.ROW_ACTIVATE.DIDEVENT.RETURN_EOUTOFMEMORY "
		L"			} "
		L"		 IRowset::GetNextRows(B) "
		L"			{ "
		L"			} "
	    L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(49)
//*-----------------------------------------------------------------------
// @mfunc ROW_ACTIVATE - E_INVALIDARG - All Phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetNextRows::Variation_49()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{IRowset::GetNextRows(A) "
	    L"			{"
		L"			 L1.ROW_ACTIVATE.DIDEVENT.RETURN_EINVALIDARG "
		L"			} "
		L"		 IRowset::GetNextRows(B) "
		L"			{ "
		L"			} "
	    L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(50)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetNextRows::Variation_50()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(51)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetNextRows::Variation_51()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(52)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - Fetch no rows - no notifications
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetNextRows::Variation_52()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows() "
	    L"			{ "
		L"			} "
		L"		 IRowset::GetNextRows() "
		L"			{ "
		L"			} "
	    L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(53)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - Fetch A, Fetch A again
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetNextRows::Variation_53()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{IRowset::GetNextRows(A) "
	    L"			{"
		L"			 L1.ROW_ACTIVATE.DIDEVENT.RETURN_ACCEPT "
		L"			} "
		L"		 IRowset::RestartPosition() {} "
		L"		 IRowset::GetNextRows(A) "
		L"			{ "
		L"			} "
	    L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(54)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - Fetch A, Fetch A,B
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetNextRows::Variation_54()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{IRowset::GetNextRows(A) "
	    L"			{"
		L"			 L1.ROW_ACTIVATE.DIDEVENT.RETURN_ACCEPT "
		L"			} "
		L"		 IRowset::RestartPosition() {} "
		L"		 IRowset::GetNextRows(A,B) "
		L"			{ "
		L"			} "
	    L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(55)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - Fetch A, ReleaseRows A, Fetch A
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetNextRows::Variation_55()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{IRowset::GetNextRows(A) "
	    L"			{"
		L"			 L1.ROW_ACTIVATE.DIDEVENT.RETURN_ACCEPT "
		L"			} "
		L"		 IRowset::ReleaseRows(A) {} "
		L"		 IRowset::RestartPosition() {} "
		L"		 IRowset::GetNextRows(A) "
		L"			{ "
		L"			 L1.ROW_ACTIVATE.DIDEVENT.RETURN_ACCEPT "
		L"			} "
	    L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(56)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetNextRows::Variation_56()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(57)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - DB_S_ENDOFROWSET - 1 more than in rowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetNextRows::Variation_57()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(DB_S_ENDOFROWSET) "
		L"			{"
		L"			 L1.ROW_ACTIVATE.DIDEVENT.RETURN_ACCEPT "
		L"			}"
	    L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(58)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - DB_S_ENDOFROWSET - 1 more at end of rowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetNextRows::Variation_58()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(DB_S_ENDOFROWSET) "
		L"			{"
		L"			 L1.ROW_ACTIVATE.DIDEVENT.RETURN_ACCEPT "
		L"			}"
		L"		 IRowset::GetNextRows(A, DB_S_ENDOFROWSET) "
		L"			{"
		L"			}"
	  L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(59)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - DB_S_ENDOFROWSET - Stepping backward
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetNextRows::Variation_59()
{
//TODO need some way to step backward?
	
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(DB_S_ENDOFROWSET) "
		L"			{"
		L"			}"
	  L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(60)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetNextRows::Variation_60()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(61)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - DB_E_BADSTARTPOSITION
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetNextRows::Variation_61()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
 		L"		 IRowset::GetNextRows(A, DB_E_BADSTARTPOSITION) "
		L"			{"
		L"			}"
	  L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(62)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetNextRows::Variation_62()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(E_INVALIDARG) "
		L"			{"
		L"			}"
	  L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(63)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - EmptyRowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetNextRows::Variation_63()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(DB_S_ENDOFROWSET) "
		L"			{"
		L"			}"
	  L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, EMPTY_ROWSET | DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(64)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - Remove Listeners
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetNextRows::Variation_64()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{IRowset::GetNextRows(A) "
	    L"			{"
		L"			 L1.ROWSET_FETCHPOSITIONCHANGE.OKTODO.RETURN_UNADVISE "
		L"			} "
		L"		 IRowset::GetNextRows(B) "
		L"			{ "
		L"			 L3.ROWSET_FETCHPOSITIONCHANGE.ABOUTTODO.RETURN_UNADVISE "
		L"			} "
		L"		 IRowset::GetNextRows(C) "
		L"			{ "
		L"			 L2.ROWSET_FETCHPOSITIONCHANGE.ABOUTTODO.RETURN_ACCEPT "
		L"			} "
	    L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(65)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - Add Listeners
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetNextRows::Variation_65()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{IRowset::GetNextRows(A) "
	    L"			{"
		L"			 L1.ROWSET_FETCHPOSITIONCHANGE.OKTODO.RETURN_ADVISE "
		L"			} "
		L"		 IRowset::GetNextRows(B) "
		L"			{ "
		L"			 L3.ROWSET_FETCHPOSITIONCHANGE.ABOUTTODO.RETURN_ADVISE "
		L"			} "
		L"		 IRowset::GetNextRows(C) "
		L"			{ "
		L"			 L1.ROWSET_FETCHPOSITIONCHANGE.ABOUTTODO.RETURN_ACCEPT "
		L"			} "
	    L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(66)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - GetData
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetNextRows::Variation_66()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{IRowset::GetNextRows(A) "
	    L"			{ "
		L"			} "
		L"		 IRowset::GetNextRows(B) "
	    L"			{"
		L"			 L1.ROWSET_FETCHPOSITIONCHANGE.OKTODO.IRowset::GetData(A) {} "
		L"			 L2.ROWSET_FETCHPOSITIONCHANGE.ABOUTTODO.IRowset::GetData(A) {} "
		L"			 L3.ROWSET_FETCHPOSITIONCHANGE.SYNCHAFTER.IRowset::GetData(A) {} "
		L"			 L1.ROWSET_FETCHPOSITIONCHANGE.DIDEVENT.IRowset::GetData(A) {} "
		L"			} "
		L"		 IRowset::GetNextRows(C) "
	    L"			{"
		L"			 L1.ROW_ACTIVATE.DIDEVENT.IRowset::GetData(B) {} "
		L"			 L2.ROW_ACTIVATE.DIDEVENT.IRowset::GetData(A) {} "
		L"			 L3.ROW_ACTIVATE.DIDEVENT.IRowset::GetData(B) {} "
		L"			} "
	    L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}
// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL GetNextRows::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CExecutionManager::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(ReleaseRows)
//*-----------------------------------------------------------------------
//| Test Case:		ReleaseRows - Tests Notifications for IRowset::ReleaseRows
//|	Created:			09/29/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL ReleaseRows::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CExecutionManager::Init())
	// }}
	{
		//Verify Rowset can be created...
		if(CreateRowset(DEFAULT_ROWSET)==S_OK)
			return TRUE;
	}

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc ROW_RELEASE - SINGLEROW - Accept all phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ReleaseRows::Variation_1()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{ "
		L"		 IRowset::GetNextRows(A) {} "
		L"		 IRowset::ReleaseRows(A) {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc ROW_RELEASE - MULTIPLEROWS - Accept all phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ReleaseRows::Variation_2()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{ "
		L"		 IRowset::GetNextRows(A,B,C) {} "
		L"		 IRowset::ReleaseRows(A,B,C) {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ReleaseRows::Variation_3()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ReleaseRows::Variation_4()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc ROW_RELEASE - DB_E_CANCELED - Accept all phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ReleaseRows::Variation_5()
{
	//Cannot "sucessfully" Veto a DIDEVENT reason, so just make sure the row
	//handle is invalid after a veto of ROW_RELEASE
	
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{ "
		L"		 IRowset::GetNextRows(A) {} "
		L"		 IRowset::ReleaseRows(A) "
		L"			{ "
		L"			 L1.ROW_RELEASE.DIDEVENT.RETURN_VETO "
		L"			} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc ROW_RELEASE - DB_E_NOTREENTRANT - Accept all phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ReleaseRows::Variation_6()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{ "
		L"		 IRowset::GetNextRows(A) {} "
		L"		 IRowset::ReleaseRows(A) { L1.ROW_RELEASE.DIDEVENT.IRowset::ReleaseRows(A, DB_E_NOTREENTRANT) {} } "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ReleaseRows::Variation_7()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ReleaseRows::Variation_8()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc ROW_RELEASE - Veto DIDEVENT, Listener 2
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ReleaseRows::Variation_9()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{ "
		L"		 IRowset::GetNextRows(A) {} "
		L"		 IRowset::ReleaseRows(A) { L2.ROW_RELEASE.DIDEVENT.RETURN_VETO } "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ReleaseRows::Variation_10()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ReleaseRows::Variation_11()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc ROW_RELEASE - UNWANTED REASON
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ReleaseRows::Variation_12()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{ "
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowset::ReleaseRows(A) { L1.ROW_RELEASE.DIDEVENT.RETURN_UNWANTEDREASON } "
		L"		 IRowset::ReleaseRows(B) {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc ROW_RELEASE - UNWANTED PHASE - DIDEVENT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ReleaseRows::Variation_13()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{ "
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowset::ReleaseRows(A) { L1.ROW_RELEASE.DIDEVENT.RETURN_UNWANTEDPHASE } "
		L"		 IRowset::ReleaseRows(B) {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ReleaseRows::Variation_14()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ReleaseRows::Variation_15()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc ROW_RELEASE - E_FAIL - All Phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ReleaseRows::Variation_16()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{ "
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowset::ReleaseRows(A) { L1.ROW_RELEASE.DIDEVENT.RETURN_EFAIL } "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc ROW_RELEASE - E_OUTOFMEMORY - All Phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ReleaseRows::Variation_17()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{ "
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowset::ReleaseRows(A) { L1.ROW_RELEASE.DIDEVENT.RETURN_EOUTOFMEMORY } "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc ROW_RELEASE - E_INVALIDARG - All Phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ReleaseRows::Variation_18()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{ "
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowset::ReleaseRows(A) { L1.ROW_RELEASE.DIDEVENT.RETURN_EINVALIDARG } "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ReleaseRows::Variation_19()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ReleaseRows::Variation_20()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - Release 0 rows
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ReleaseRows::Variation_21()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{ "
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowset::ReleaseRows() {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(22)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - AddRef Row, ReleaseRows
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ReleaseRows::Variation_22()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{ "
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowset::AddRefRows(A,B) {} "
		L"		 IRowset::ReleaseRows(A,B) {} "
		L"		 IRowset::ReleaseRows(A,B) "
		L"			{"
		L"			 L1.ROW_RELEASE.DIDEVENT.RETURN_ACCEPT " 
		L"			}"
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(23)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - ReleaseRow, ReleaseRow
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ReleaseRows::Variation_23()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{ "
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowset::ReleaseRows(A,B) "
		L"			{"
		L"			 L1.ROW_RELEASE.DIDEVENT.RETURN_ACCEPT " 
		L"			}"
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(24)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - ReleaseRows [valid, invalid]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ReleaseRows::Variation_24()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{ "
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowset::ReleaseRows(A,B,C,DB_S_ERRORSOCCURRED) "
		L"			{"
		L"			 L1.ROW_RELEASE.DIDEVENT.RETURN_ACCEPT " 
		L"			}"
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(25)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ReleaseRows::Variation_25()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(26)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - DBROWSTATUS_S_PENDINGCHANGES
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ReleaseRows::Variation_26()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{ "
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2) {} "
		L"		 IRowset::ReleaseRows(A) "
		L"			{"
		L"			 L1.ROW_RELEASE.DIDEVENT.RETURN_ACCEPT " 
		L"			 L2.ROW_RELEASE.DIDEVENT.RETURN_ACCEPT " 
		L"			 L3.ROW_RELEASE.DIDEVENT.RETURN_ACCEPT " 
		L"			}"
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(27)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ReleaseRows::Variation_27()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{ "
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowset::ReleaseRows(A, E_INVALIDARG) "
		L"			{"
		L"			}"
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(28)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - EmptyRowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ReleaseRows::Variation_28()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{ "
		L"		 IRowset::ReleaseRows(A,B,C,DB_E_ERRORSOCCURRED) "
		L"			{"
		L"			}"
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, EMPTY_ROWSET | DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(29)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - Add/Remove Listeners
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ReleaseRows::Variation_29()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B,C) {} "
		L"		 IRowset::ReleaseRows(A) "
	    L"			{"
		L"			 L1.ROW_RELEASE.DIDEVENT.RETURN_UNADVISE "
		L"			} "
		L"		 IRowset::ReleaseRows(B) "
		L"			{ "
		L"			 L2.ROW_RELEASE.DIDEVENT.RETURN_UNADVISE "
		L"			} "
		L"		 IRowset::ReleaseRows(C) "
		L"			{ "
		L"			 L3.ROW_RELEASE.DIDEVENT.RETURN_ACCEPT "
		L"			} "
	    L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(30)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - GetData
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ReleaseRows::Variation_30()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{ "
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowset::ReleaseRows(A) "
		L"			{"
		L"			 L1.ROW_RELEASE.DIDEVENT.IRowset::GetData(B) {} "
		L"			}"
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}
// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL ReleaseRows::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CExecutionManager::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(RestartPosition)
//*-----------------------------------------------------------------------
//| Test Case:		RestartPosition - Tests Notificatons for IRowset::RestartPosition
//|	Created:			09/29/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL RestartPosition::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CExecutionManager::Init())
	// }}
	{
		//Verify Rowset can be created...
		if(CreateRowset(DEFAULT_ROWSET)==S_OK)
			return TRUE;
	}

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc FETCHPOSITIONCHANGE - Accept all phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int RestartPosition::Variation_1()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::RestartPosition() {} "
		L"		 IRowset::RestartPosition() {} "
		L"		 IRowset::RestartPosition() {} "

		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowset::ReleaseRows(A,B) {} "
		L"		 IRowset::RestartPosition() {} "
		
		L"		 IRowset::RestartPosition() {} "
		L"		 IRowset::RestartPosition() {} "
		L"		 IRowset::RestartPosition() {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int RestartPosition::Variation_2()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int RestartPosition::Variation_3()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc FETCHPOSITIONCHANGE - DB_E_CANCELED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int RestartPosition::Variation_4()
{
	//Need to verify the FETCHPOSITION did not get reset since it was veto'd
	//Read all the rows in the rowset.  Then Veto the RestartPostition().
	//The next GetNextRows should fail, if the FetchPosition was truly veto'd

	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(DB_S_ENDOFROWSET) {} "
		L"		 IRowset::RestartPosition() { L2.ROWSET_FETCHPOSITIONCHANGE.ABOUTTODO.RETURN_VETO } "
		L"		 IRowset::GetNextRows(A, DB_S_ENDOFROWSET) {} "
		L"		}"
		L"	}"
		L"}";

	//To produce FAILEDTODO, this reason/phase must be vetoable...
	TESTC_PROVIDER(IsCancelableEvent(DBREASON_ROWSET_FETCHPOSITIONCHANGE, DBEVENTPHASE_ABOUTTODO));
	TESTC(Execute(CommandString, DEFAULT_ROWSET));

CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc FETCHPOSITIONCHANGE - DB_E_NOTREENTRANT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int RestartPosition::Variation_5()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowset::RestartPosition() "
		L"			{ L2.ROWSET_FETCHPOSITIONCHANGE.ABOUTTODO.IRowset::RestartPosition(DB_E_NOTREENTRANT) {} } "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int RestartPosition::Variation_6()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int RestartPosition::Variation_7()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc FETCHPOSITIONCHANGE - Veto OKTODO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int RestartPosition::Variation_8()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowset::RestartPosition() "
		L"			{ L2.ROWSET_FETCHPOSITIONCHANGE.OKTODO.RETURN_VETO } "
		L"		}"
		L"	}"
		L"}";

//TODO need to verify the fetch position does not change!
	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc FETCHPOSITIONCHANGE - Veto ABOUTTODO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int RestartPosition::Variation_9()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowset::RestartPosition() "
		L"			{ L2.ROWSET_FETCHPOSITIONCHANGE.ABOUTTODO.RETURN_VETO } "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc FETCHPOSITIONCHANGE - Veto SYNCHAFTER
//
// @rdesc TEST_PASS or TEST_FAIL
//
int RestartPosition::Variation_10()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowset::RestartPosition() "
		L"			{ L2.ROWSET_FETCHPOSITIONCHANGE.SYNCHAFTER.RETURN_VETO } "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc FETCHPOSITIONCHANGE - Veto DIDEVENT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int RestartPosition::Variation_11()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowset::RestartPosition() "
		L"			{ L2.ROWSET_FETCHPOSITIONCHANGE.DIDEVENT.RETURN_VETO } "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc FETCHPOSITIONCHANGE - Veto FAILEDTODO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int RestartPosition::Variation_12()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowset::RestartPosition() "
		L"			{ "
		L"			 L3.ROWSET_FETCHPOSITIONCHANGE.ABOUTTODO.RETURN_VETO "
		L"			 L3.ROWSET_FETCHPOSITIONCHANGE.FAILEDTODO.RETURN_VETO  "
		L"			} "
		L"		}"
		L"	}"
		L"}";

	//To produce FAILEDTODO, this reason/phase must be vetoable...
	TESTC_PROVIDER(IsCancelableEvent(DBREASON_ROWSET_FETCHPOSITIONCHANGE, DBEVENTPHASE_ABOUTTODO));
	TESTC(Execute(CommandString, DEFAULT_ROWSET));

CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int RestartPosition::Variation_13()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int RestartPosition::Variation_14()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc FETCHPOSITIONCHANGE - UNWANTED REASON
//
// @rdesc TEST_PASS or TEST_FAIL
//
int RestartPosition::Variation_15()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowset::RestartPosition() "
		L"			{ L3.ROWSET_FETCHPOSITIONCHANGE.OKTODO.RETURN_UNWANTEDREASON } "
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowset::RestartPosition() {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc FETCHPOSITIONCHANGE - UNWANTED PHASE - OKTODO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int RestartPosition::Variation_16()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowset::RestartPosition() "
		L"			{ L3.ROWSET_FETCHPOSITIONCHANGE.OKTODO.RETURN_UNWANTEDPHASE } "
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowset::RestartPosition() {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc FETCHPOSITIONCHANGE - UNWANTED PHASE - ABOUTTODO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int RestartPosition::Variation_17()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowset::RestartPosition() "
		L"			{ L3.ROWSET_FETCHPOSITIONCHANGE.ABOUTTODO.RETURN_UNWANTEDPHASE } "
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowset::RestartPosition() {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc FETCHPOSITIONCHANGE - UNWANTED PHASE - SYNCHAFTER
//
// @rdesc TEST_PASS or TEST_FAIL
//
int RestartPosition::Variation_18()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowset::RestartPosition() "
		L"			{ L3.ROWSET_FETCHPOSITIONCHANGE.SYNCHAFTER.RETURN_UNWANTEDPHASE } "
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowset::RestartPosition() {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc FETCHPOSITIONCHANGE - UNWANTED PHASE - DIDEVENT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int RestartPosition::Variation_19()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowset::RestartPosition() "
		L"			{ L3.ROWSET_FETCHPOSITIONCHANGE.DIDEVENT.RETURN_UNWANTEDPHASE } "
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowset::RestartPosition() {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc FETCHPOSITIONCHANGE - UNWANTED PHASE - FAILEDTODO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int RestartPosition::Variation_20()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowset::RestartPosition() "
		L"			{ "
		L"			 L3.ROWSET_FETCHPOSITIONCHANGE.ABOUTTODO.RETURN_VETO  "
		L"			 L3.ROWSET_FETCHPOSITIONCHANGE.FAILEDTODO.RETURN_UNWANTEDPHASE "
		L"			} "
		L"		 IRowset::RestartPosition() {} "
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		}"
		L"	}"
		L"}";

	//To produce FAILEDTODO, this reason/phase must be vetoable...
	TESTC_PROVIDER(IsCancelableEvent(DBREASON_ROWSET_FETCHPOSITIONCHANGE, DBEVENTPHASE_ABOUTTODO));
	TESTC(Execute(CommandString, DEFAULT_ROWSET));

CLEANUP:
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int RestartPosition::Variation_21()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(22)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int RestartPosition::Variation_22()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(23)
//*-----------------------------------------------------------------------
// @mfunc FETCHPOSITIONCHANGE - E_FAIL - All Phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int RestartPosition::Variation_23()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowset::RestartPosition() "
		L"			{ "
		L"			 L1.ROWSET_FETCHPOSITIONCHANGE.OKTODO.RETURN_EFAIL "
		L"			 L2.ROWSET_FETCHPOSITIONCHANGE.ABOUTTODO.RETURN_EFAIL "
		L"			 L3.ROWSET_FETCHPOSITIONCHANGE.SYNCHAFTER.RETURN_EFAIL "
		L"			 L1.ROWSET_FETCHPOSITIONCHANGE.DIDEVENT.RETURN_EFAIL "
		L"			} "
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowset::RestartPosition() "
		L"			{ "
		L"			 L1.ROWSET_FETCHPOSITIONCHANGE.ABOUTTODO.RETURN_VETO "
		L"			} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(24)
//*-----------------------------------------------------------------------
// @mfunc FETCHPOSITIONCHANGE - E_OUTOFMEMORY - All Phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int RestartPosition::Variation_24()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowset::RestartPosition() "
		L"			{ "
		L"			 L1.ROWSET_FETCHPOSITIONCHANGE.OKTODO.RETURN_EOUTOFMEMORY "
		L"			 L2.ROWSET_FETCHPOSITIONCHANGE.ABOUTTODO.RETURN_EOUTOFMEMORY "
		L"			 L3.ROWSET_FETCHPOSITIONCHANGE.SYNCHAFTER.RETURN_EOUTOFMEMORY "
		L"			 L1.ROWSET_FETCHPOSITIONCHANGE.DIDEVENT.RETURN_EOUTOFMEMORY "
		L"			} "
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowset::RestartPosition() "
		L"			{ "
		L"			 L1.ROWSET_FETCHPOSITIONCHANGE.ABOUTTODO.RETURN_VETO "
		L"			} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(25)
//*-----------------------------------------------------------------------
// @mfunc FETCHPOSITIONCHANGE - E_INVALIDARG - All Phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int RestartPosition::Variation_25()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowset::RestartPosition() "
		L"			{ "
		L"			 L1.ROWSET_FETCHPOSITIONCHANGE.OKTODO.RETURN_EINVALIDARG "
		L"			 L2.ROWSET_FETCHPOSITIONCHANGE.ABOUTTODO.RETURN_EINVALIDARG "
		L"			 L3.ROWSET_FETCHPOSITIONCHANGE.SYNCHAFTER.RETURN_EINVALIDARG "
		L"			 L1.ROWSET_FETCHPOSITIONCHANGE.DIDEVENT.RETURN_EINVALIDARG "
		L"			} "
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowset::RestartPosition() "
		L"			{ "
		L"			 L1.ROWSET_FETCHPOSITIONCHANGE.ABOUTTODO.RETURN_VETO "
		L"			} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(26)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int RestartPosition::Variation_26()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(27)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int RestartPosition::Variation_27()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(28)
//*-----------------------------------------------------------------------
// @mfunc ROWSET_CHANGED - Accept all phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int RestartPosition::Variation_28()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 ITableDefinition::AddColumn() {} "
		L"		 IRowset::RestartPosition() "
		L"			{ "
		L"			 L1.ROWSET_CHANGED.DIDEVENT.RETURN_ACCEPT "
		L"			} "
		L"		 ITableDefinition::AddColumn() {} "
		L"		 IRowset::RestartPosition() "
		L"			{ "
		L"			} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(29)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int RestartPosition::Variation_29()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(30)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int RestartPosition::Variation_30()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(31)
//*-----------------------------------------------------------------------
// @mfunc ROWSET_CHANGED - DB_E_CANCELED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int RestartPosition::Variation_31()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 ITableDefinition::AddColumn() {} "
		L"		 IRowset::RestartPosition() "
		L"			{ "
		L"			 L1.ROWSET_CHANGED.DIDEVENT.RETURN_ACCEPT "
		L"			} "
		L"		 ITableDefinition::AddColumn() {} "
		L"		 IRowset::RestartPosition() "
		L"			{ "
		L"			} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(32)
//*-----------------------------------------------------------------------
// @mfunc ROWSET_CHANGED - DB_E_NOTREENTRANT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int RestartPosition::Variation_32()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 ITableDefinition::AddColumn() {} "
		L"		 IRowset::RestartPosition() "
		L"			{ "
		L"			 L1.ROWSET_CHANGED.DIDEVENT.IRowset::RestartPosition() {} "
		L"			} "
		L"		 ITableDefinition::AddColumn() {} "
		L"		 IRowset::RestartPosition() "
		L"			{ "
		L"			} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(33)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int RestartPosition::Variation_33()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(34)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int RestartPosition::Variation_34()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(35)
//*-----------------------------------------------------------------------
// @mfunc ROWSET_CHANGED - Veto OKTODO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int RestartPosition::Variation_35()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 ITableDefinition::AddColumn() {} "
		L"		 IRowset::RestartPosition() "
		L"			{ "
		L"			 L1.ROWSET_CHANGED.DIDEVENT.RETURN_ACCEPT "
		L"			} "
		L"		 ITableDefinition::AddColumn() {} "
		L"		 IRowset::RestartPosition() "
		L"			{ "
		L"			} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(36)
//*-----------------------------------------------------------------------
// @mfunc ROWSET_CHANGED - Veto ABOUTTODO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int RestartPosition::Variation_36()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 ITableDefinition::AddColumn() {} "
		L"		 IRowset::RestartPosition() "
		L"			{ "
		L"			 L1.ROWSET_CHANGED.DIDEVENT.RETURN_ACCEPT "
		L"			} "
		L"		 ITableDefinition::AddColumn() {} "
		L"		 IRowset::RestartPosition() "
		L"			{ "
		L"			} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(37)
//*-----------------------------------------------------------------------
// @mfunc ROWSET_CHANGED - Veto SYNCHAFTER
//
// @rdesc TEST_PASS or TEST_FAIL
//
int RestartPosition::Variation_37()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 ITableDefinition::AddColumn() {} "
		L"		 IRowset::RestartPosition() "
		L"			{ "
		L"			 L1.ROWSET_CHANGED.DIDEVENT.RETURN_ACCEPT "
		L"			} "
		L"		 ITableDefinition::AddColumn() {} "
		L"		 IRowset::RestartPosition() "
		L"			{ "
		L"			} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(38)
//*-----------------------------------------------------------------------
// @mfunc ROWSET_CHANGED - Veto DIDEVENT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int RestartPosition::Variation_38()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 ITableDefinition::AddColumn() {} "
		L"		 IRowset::RestartPosition() "
		L"			{ "
		L"			 L1.ROWSET_CHANGED.DIDEVENT.RETURN_VETO "
		L"			} "
		L"		 ITableDefinition::AddColumn() {} "
		L"		 IRowset::RestartPosition() "
		L"			{ "
		L"			} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(39)
//*-----------------------------------------------------------------------
// @mfunc ROWSET_CHANGED - Veto FAILEDTODO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int RestartPosition::Variation_39()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 ITableDefinition::AddColumn() {} "
		L"		 IRowset::RestartPosition() "
		L"			{ "
		L"			 L1.ROWSET_CHANGED.DIDEVENT.RETURN_VETO "
		L"			 L1.ROWSET_CHANGED.FAILEDTODO.RETURN_VETO "
		L"			} "
		L"		 ITableDefinition::AddColumn() {} "
		L"		 IRowset::RestartPosition() "
		L"			{ "
		L"			} "
		L"		}"
		L"	}"
		L"}";

	//To produce FAILEDTODO, this reason/phase must be vetoable...
	TESTC_PROVIDER(IsCancelableEvent(DBREASON_ROWSET_CHANGED, DBEVENTPHASE_DIDEVENT));
	TESTC(Execute(CommandString, DEFAULT_ROWSET));

CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(40)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int RestartPosition::Variation_40()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(41)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int RestartPosition::Variation_41()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(42)
//*-----------------------------------------------------------------------
// @mfunc ROWSET_CHANGED - UNWANTED REASON
//
// @rdesc TEST_PASS or TEST_FAIL
//
int RestartPosition::Variation_42()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 ITableDefinition::AddColumn() {} "
		L"		 IRowset::RestartPosition() "
		L"			{ "
		L"			 L1.ROWSET_CHANGED.OKTODO.RETURN_UNWANTEDREASON "
		L"			} "
		L"		 ITableDefinition::AddColumn() {} "
		L"		 IRowset::RestartPosition() "
		L"			{ "
		L"			} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(43)
//*-----------------------------------------------------------------------
// @mfunc ROWSET_CHANGED - UNWANTED PHASE OKTODO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int RestartPosition::Variation_43()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 ITableDefinition::AddColumn() {} "
		L"		 IRowset::RestartPosition() "
		L"			{ "
		L"			 L1.ROWSET_CHANGED.OKTODO.RETURN_UNWANTEDPHASE "
		L"			} "
		L"		 ITableDefinition::AddColumn() {} "
		L"		 IRowset::RestartPosition() "
		L"			{ "
		L"			} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(44)
//*-----------------------------------------------------------------------
// @mfunc ROWSET_CHANGED - UNWANTED PHASE ABOUTTODO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int RestartPosition::Variation_44()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 ITableDefinition::AddColumn() {} "
		L"		 IRowset::RestartPosition() "
		L"			{ "
		L"			 L1.ROWSET_CHANGED.ABOUTTODO.RETURN_UNWANTEDPHASE "
		L"			} "
		L"		 ITableDefinition::AddColumn() {} "
		L"		 IRowset::RestartPosition() "
		L"			{ "
		L"			} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(45)
//*-----------------------------------------------------------------------
// @mfunc ROWSET_CHANGED - UNWANTED PHASE SYNCHAFTER
//
// @rdesc TEST_PASS or TEST_FAIL
//
int RestartPosition::Variation_45()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 ITableDefinition::AddColumn() {} "
		L"		 IRowset::RestartPosition() "
		L"			{ "
		L"			 L1.ROWSET_CHANGED.SYNCHAFTER.RETURN_UNWANTEDPHASE "
		L"			} "
		L"		 ITableDefinition::AddColumn() {} "
		L"		 IRowset::RestartPosition() "
		L"			{ "
		L"			} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(46)
//*-----------------------------------------------------------------------
// @mfunc ROWSET_CHANGED - UNWANTED PHASE DIDEVENT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int RestartPosition::Variation_46()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 ITableDefinition::AddColumn() {} "
		L"		 IRowset::RestartPosition() "
		L"			{ "
		L"			 L1.ROWSET_CHANGED.DIDEVENT.RETURN_UNWANTEDPHASE "
		L"			} "
		L"		 ITableDefinition::AddColumn() {} "
		L"		 IRowset::RestartPosition() "
		L"			{ "
		L"			} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(47)
//*-----------------------------------------------------------------------
// @mfunc ROWSET_CHANGED - UNWANTED PHASE FAILEDTODO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int RestartPosition::Variation_47()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 ITableDefinition::AddColumn() {} "
		L"		 IRowset::RestartPosition() "
		L"			{ "
		L"			 L1.ROWSET_CHANGED.OKTODO.RETURN_VETO "
		L"			 L1.ROWSET_CHANGED.FAILEDTODO.RETURN_UNWANTEDPHASE "
		L"			} "
		L"		 ITableDefinition::AddColumn() {} "
		L"		 IRowset::RestartPosition() "
		L"			{ "
		L"			} "
		L"		}"
		L"	}"
		L"}";

	//To produce FAILEDTODO, this reason/phase must be vetoable...
	TESTC_PROVIDER(IsCancelableEvent(DBREASON_ROWSET_CHANGED, DBEVENTPHASE_OKTODO));
	TESTC(Execute(CommandString, DEFAULT_ROWSET));

CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(48)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int RestartPosition::Variation_48()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(49)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int RestartPosition::Variation_49()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(50)
//*-----------------------------------------------------------------------
// @mfunc ROWSET_CHANGED - E_FAIL - All Phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int RestartPosition::Variation_50()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 ITableDefinition::AddColumn() {} "
		L"		 IRowset::RestartPosition() "
		L"			{ "
		L"			 L1.ROWSET_CHANGED.OKTODO.RETURN_EFAIL "
		L"			 L2.ROWSET_CHANGED.ABOUTTODO.RETURN_EFAIL "
		L"			 L3.ROWSET_CHANGED.SYNCHAFTER.RETURN_EFAIL "
		L"			 L1.ROWSET_CHANGED.DIDEVENT.RETURN_EFAIL "
		L"			} "
		L"		 ITableDefinition::AddColumn() {} "
		L"		 IRowset::RestartPosition() "
		L"			{ "
		L"			 L1.ROWSET_CHANGED.OKTODO.RETURN_VETO "
		L"			} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(51)
//*-----------------------------------------------------------------------
// @mfunc ROWSET_CHANGED - E_OUTOFMEMORY - All Phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int RestartPosition::Variation_51()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 ITableDefinition::AddColumn() {} "
		L"		 IRowset::RestartPosition() "
		L"			{ "
		L"			 L1.ROWSET_CHANGED.OKTODO.RETURN_EOUTOFMEMORY "
		L"			 L2.ROWSET_CHANGED.ABOUTTODO.RETURN_EOUTOFMEMORY "
		L"			 L3.ROWSET_CHANGED.SYNCHAFTER.RETURN_EOUTOFMEMORY "
		L"			 L1.ROWSET_CHANGED.DIDEVENT.RETURN_EOUTOFMEMORY "
		L"			} "
		L"		 ITableDefinition::AddColumn() {} "
		L"		 IRowset::RestartPosition() "
		L"			{ "
		L"			 L1.ROWSET_CHANGED.OKTODO.RETURN_VETO "
		L"			} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(52)
//*-----------------------------------------------------------------------
// @mfunc ROWSET_CHANGED - E_INVALIDARG - All Phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int RestartPosition::Variation_52()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 ITableDefinition::AddColumn() {} "
		L"		 IRowset::RestartPosition() "
		L"			{ "
		L"			 L1.ROWSET_CHANGED.OKTODO.RETURN_EINVALIDARG "
		L"			 L2.ROWSET_CHANGED.ABOUTTODO.RETURN_EINVALIDARG "
		L"			 L3.ROWSET_CHANGED.SYNCHAFTER.RETURN_EINVALIDARG "
		L"			 L1.ROWSET_CHANGED.DIDEVENT.RETURN_EINVALIDARG "
		L"			} "
		L"		 ITableDefinition::AddColumn() {} "
		L"		 IRowset::RestartPosition() "
		L"			{ "
		L"			 L1.ROWSET_CHANGED.OKTODO.RETURN_VETO "
		L"			} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(53)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int RestartPosition::Variation_53()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(54)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int RestartPosition::Variation_54()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(55)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - RestartPosition
//
// @rdesc TEST_PASS or TEST_FAIL
//
int RestartPosition::Variation_55()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::RestartPosition() "
		L"			{ "
		L"			} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(56)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - GetNextRows - RestartPosition - RestartPosition
//
// @rdesc TEST_PASS or TEST_FAIL
//
int RestartPosition::Variation_56()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowset::RestartPosition() "
		L"			{ "
		L"			} "
		L"		 IRowset::RestartPosition() "
		L"			{ "
		L"			} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(57)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int RestartPosition::Variation_57()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(58)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - DB_E_ROWSNOTRELEASED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int RestartPosition::Variation_58()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowset::RestartPosition() "
		L"			{ "
		L"			} "
		L"		 IRowset::RestartPosition() "
		L"			{ "
		L"			} "
		L"		}"
		L"	}"
		L"}";

//TODO need to specifiy no CANHOLDROWS
	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(59)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - EmptyRowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int RestartPosition::Variation_59()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::RestartPosition() "
		L"			{ "
		L"			} "
		L"		 IRowset::RestartPosition() "
		L"			{ "
		L"			} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, EMPTY_ROWSET | DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(60)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - Add/Remove Listeners
//
// @rdesc TEST_PASS or TEST_FAIL
//
int RestartPosition::Variation_60()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowset::RestartPosition() "
		L"			{ "
		L"			 L1.ROWSET_FETCHPOSITIONCHANGE.OKTODO.RETURN_UNADVISE "
		L"			 L2.ROWSET_FETCHPOSITIONCHANGE.SYNCHAFTER.RETURN_UNADVISE "
		L"			} "
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowset::RestartPosition() "
		L"			{ "
		L"			 L3.ROWSET_FETCHPOSITIONCHANGE.ABOUTTODO.RETURN_UNADVISE "
		L"			} "
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowset::RestartPosition() "
		L"			{ "
		L"			} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(61)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - GetData
//
// @rdesc TEST_PASS or TEST_FAIL
//
int RestartPosition::Variation_61()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowset::RestartPosition() "
		L"			{ "
		L"			 L1.ROWSET_FETCHPOSITIONCHANGE.OKTODO.IRowset::GetData(A) {} "
		L"			 L2.ROWSET_FETCHPOSITIONCHANGE.ABOUTTODO.IRowset::GetData(B) {} "
		L"			 L3.ROWSET_FETCHPOSITIONCHANGE.SYNCHAFTER.IRowset::GetData(A) {} "
		L"			 L1.ROWSET_FETCHPOSITIONCHANGE.DIDEVENT.IRowset::GetData(B) {} "
		L"			} "
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowset::RestartPosition() "
		L"			{ "
		L"			 L1.ROWSET_FETCHPOSITIONCHANGE.DIDEVENT.IRowset::GetData(A) {} "
		L"			} "
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowset::RestartPosition() "
		L"			{ "
		L"			} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, DEFAULT_ROWSET);
}
// }}
// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL RestartPosition::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CExecutionManager::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(Seek)
//*-----------------------------------------------------------------------
//| Test Case:		Seek - Test Notifications for IRowsetIndex::Seek
//|	Created:			03/24/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Seek::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CExecutionManager::Init())
	// }}
	{
		//Verify Rowset can be created...
		TEST_PROVIDER(CreateRowset(ROWSETINDEX_ROWSET)==S_OK);
		return TRUE;
	}

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc FETCHPOSITIONCHANGE - Accept all phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Seek::Variation_1()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetIndex::Seek(1) "
		L"			{ "
		L"			 L1.ROWSET_FETCHPOSITIONCHANGE.OKTODO.RETURN_ACCEPT "
		L"			} " 
		L"		 IRowsetIndex::Seek(1) "
		L"			{ "
		L"			 L1.ROWSET_FETCHPOSITIONCHANGE.OKTODO.RETURN_ACCEPT "
		L"			} " 
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, ROWSETINDEX_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Seek::Variation_2()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Seek::Variation_3()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc FETCHPOSITIONCHANGE - DB_E_CANCELED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Seek::Variation_4()
{
	//Need to verify the Veto was successful
	//Verify the FETCHPOSITION is unachanged after veto    
	
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetIndex::Seek(1) "
		L"			{ "
		L"			 L1.ROWSET_FETCHPOSITIONCHANGE.OKTODO.RETURN_VETO "
		L"			} " 
		L"		 IRowsetIndex::Seek(1) "
		L"			{ "
		L"			 L1.ROWSET_FETCHPOSITIONCHANGE.OKTODO.RETURN_VETO "
		L"			} " 
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, ROWSETINDEX_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc FETCHPOSITIONCHANGE - DB_E_NOTREENTRANT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Seek::Variation_5()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetIndex::Seek(1) "
		L"			{ "
		L"			 L1.ROWSET_FETCHPOSITIONCHANGE.OKTODO.IRowsetIndex::Seek(1) {} "
		L"			} " 
		L"		 IRowsetIndex::Seek(1) "
		L"			{ "
		L"			 L1.ROWSET_FETCHPOSITIONCHANGE.OKTODO.RETURN_ACCEPT "
		L"			} " 
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, ROWSETINDEX_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Seek::Variation_6()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Seek::Variation_7()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc FETCHPOSITIONCHANGE - Veto OKTODO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Seek::Variation_8()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetIndex::Seek(1) "
		L"			{ "
		L"			 L1.ROWSET_FETCHPOSITIONCHANGE.OKTODO.RETURN_VETO "
		L"			} " 
		L"		 IRowsetIndex::Seek(1) "
		L"			{ "
		L"			} " 
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, ROWSETINDEX_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc FETCHPOSITIONCHANGE - Veto ABOUTTODO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Seek::Variation_9()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetIndex::Seek(1) "
		L"			{ "
		L"			 L1.ROWSET_FETCHPOSITIONCHANGE.ABOUTTODO.RETURN_VETO "
		L"			} " 
		L"		 IRowsetIndex::Seek(1) "
		L"			{ "
		L"			} " 
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, ROWSETINDEX_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc FETCHPOSITIONCHANGE - Veto SYNCHAFTER
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Seek::Variation_10()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetIndex::Seek(1) "
		L"			{ "
		L"			 L1.ROWSET_FETCHPOSITIONCHANGE.SYNCHAFTER.RETURN_VETO "
		L"			} " 
		L"		 IRowsetIndex::Seek(1) "
		L"			{ "
		L"			} " 
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, ROWSETINDEX_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc FETCHPOSITIONCHANGE - Veto DIDEVENT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Seek::Variation_11()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetIndex::Seek(1) "
		L"			{ "
		L"			 L1.ROWSET_FETCHPOSITIONCHANGE.DIDEVENT.RETURN_VETO "
		L"			} " 
		L"		 IRowsetIndex::Seek(1) "
		L"			{ "
		L"			} " 
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, ROWSETINDEX_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc FETCHPOSITIONCHANGE - Veto FAILEDTODO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Seek::Variation_12()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetIndex::Seek(1) "
		L"			{ "
		L"			 L1.ROWSET_FETCHPOSITIONCHANGE.OKTODO.RETURN_VETO "
		L"			 L1.ROWSET_FETCHPOSITIONCHANGE.FAILEDTODO.RETURN_VETO "
		L"			} " 
		L"		 IRowsetIndex::Seek(1) "
		L"			{ "
		L"			} " 
		L"		}"
		L"	}"
		L"}";

	//To produce FAILEDTODO, this reason/phase must be vetoable...
	TESTC_PROVIDER(IsCancelableEvent(DBREASON_ROWSET_FETCHPOSITIONCHANGE, DBEVENTPHASE_OKTODO));
	TESTC(Execute(CommandString, ROWSETINDEX_ROWSET));

CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Seek::Variation_13()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Seek::Variation_14()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc FETCHPOSITIONCHANGE - UNWANTED REASON
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Seek::Variation_15()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetIndex::Seek(1) "
		L"			{ "
		L"			 L1.ROWSET_FETCHPOSITIONCHANGE.OKTODO.RETURN_UNWANTEDREASON "
		L"			} " 
		L"		 IRowsetIndex::Seek(1) "
		L"			{ "
		L"			} " 
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, ROWSETINDEX_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc FETCHPOSITIONCHANGE - UNWANTED PHASE - OKTODO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Seek::Variation_16()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetIndex::Seek(1) "
		L"			{ "
		L"			 L1.ROWSET_FETCHPOSITIONCHANGE.OKTODO.RETURN_UNWANTEDPHASE "
		L"			} " 
		L"		 IRowsetIndex::Seek(1) "
		L"			{ "
		L"			} " 
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, ROWSETINDEX_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc FETCHPOSITIONCHANGE - UNWANTED PHASE - ABOUTTODO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Seek::Variation_17()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetIndex::Seek(1) "
		L"			{ "
		L"			 L1.ROWSET_FETCHPOSITIONCHANGE.ABOUTTODO.RETURN_UNWANTEDPHASE "
		L"			} " 
		L"		 IRowsetIndex::Seek(1) "
		L"			{ "
		L"			} " 
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, ROWSETINDEX_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc FETCHPOSITIONCHANGE - UNWANTED PHASE - SYNCHAFTER
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Seek::Variation_18()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetIndex::Seek(1) "
		L"			{ "
		L"			 L1.ROWSET_FETCHPOSITIONCHANGE.SYNCHAFTER.RETURN_UNWANTEDPHASE "
		L"			} " 
		L"		 IRowsetIndex::Seek(1) "
		L"			{ "
		L"			} " 
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, ROWSETINDEX_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc FETCHPOSITIONCHANGE - UNWANTED PHASE - DIDEVENT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Seek::Variation_19()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetIndex::Seek(1) "
		L"			{ "
		L"			 L1.ROWSET_FETCHPOSITIONCHANGE.DIDEVENT.RETURN_UNWANTEDPHASE "
		L"			} " 
		L"		 IRowsetIndex::Seek(1) "
		L"			{ "
		L"			} " 
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, ROWSETINDEX_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc FETCHPOSITIONCHANGE - UNWANTED PHASE - FAILEDTODO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Seek::Variation_20()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetIndex::Seek(1) "
		L"			{ "
		L"			 L1.ROWSET_FETCHPOSITIONCHANGE.OKTODO.RETURN_VETO "
		L"			 L1.ROWSET_FETCHPOSITIONCHANGE.FAILEDTODO.RETURN_UNWANTEDPHASE "
		L"			} " 
		L"		 IRowsetIndex::Seek(1) "
		L"			{ "
		L"			} " 
		L"		}"
		L"	}"
		L"}";

	//To produce FAILEDTODO, this reason/phase must be vetoable...
	TESTC_PROVIDER(IsCancelableEvent(DBREASON_ROWSET_FETCHPOSITIONCHANGE, DBEVENTPHASE_OKTODO));
	TESTC(Execute(CommandString, ROWSETINDEX_ROWSET));

CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Seek::Variation_21()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(22)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Seek::Variation_22()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(23)
//*-----------------------------------------------------------------------
// @mfunc FETCHPOSITIONCHANGE - E_FAIL - All Phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Seek::Variation_23()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetIndex::Seek(1) "
		L"			{ "
		L"			 L1.ROWSET_FETCHPOSITIONCHANGE.OKTODO.RETURN_EFAIL "
		L"			 L1.ROWSET_FETCHPOSITIONCHANGE.ABOUTTODO.RETURN_EFAIL "
		L"			 L1.ROWSET_FETCHPOSITIONCHANGE.SYNCHAFTER.RETURN_EFAIL "
		L"			 L1.ROWSET_FETCHPOSITIONCHANGE.DIDEVENT.RETURN_EFAIL "
		L"			} " 
		L"		 IRowsetIndex::Seek(1) "
		L"			{ "
		L"			 L1.ROWSET_FETCHPOSITIONCHANGE.OKTODO.RETURN_VETO "
		L"			} " 
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, ROWSETINDEX_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(24)
//*-----------------------------------------------------------------------
// @mfunc FETCHPOSITIONCHANGE - E_OUTOFMEMORY - All Phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Seek::Variation_24()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetIndex::Seek(1) "
		L"			{ "
		L"			 L1.ROWSET_FETCHPOSITIONCHANGE.OKTODO.RETURN_EOUTOFMEMORY "
		L"			 L1.ROWSET_FETCHPOSITIONCHANGE.ABOUTTODO.RETURN_EOUTOFMEMORY "
		L"			 L1.ROWSET_FETCHPOSITIONCHANGE.SYNCHAFTER.RETURN_EOUTOFMEMORY "
		L"			 L1.ROWSET_FETCHPOSITIONCHANGE.DIDEVENT.RETURN_EOUTOFMEMORY "
		L"			} " 
		L"		 IRowsetIndex::Seek(1) "
		L"			{ "
		L"			 L1.ROWSET_FETCHPOSITIONCHANGE.OKTODO.RETURN_VETO "
		L"			} " 
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, ROWSETINDEX_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(25)
//*-----------------------------------------------------------------------
// @mfunc FETCHPOSITIONCHANGE - E_INVALIDARG - All Phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Seek::Variation_25()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetIndex::Seek(1) "
		L"			{ "
		L"			 L1.ROWSET_FETCHPOSITIONCHANGE.OKTODO.RETURN_EINVALIDARG "
		L"			 L1.ROWSET_FETCHPOSITIONCHANGE.ABOUTTODO.RETURN_EINVALIDARG "
		L"			 L1.ROWSET_FETCHPOSITIONCHANGE.SYNCHAFTER.RETURN_EINVALIDARG "
		L"			 L1.ROWSET_FETCHPOSITIONCHANGE.DIDEVENT.RETURN_EINVALIDARG "
		L"			} " 
		L"		 IRowsetIndex::Seek(1) "
		L"			{ "
		L"			 L1.ROWSET_FETCHPOSITIONCHANGE.OKTODO.RETURN_VETO "
		L"			} " 
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, ROWSETINDEX_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(26)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Seek::Variation_26()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(27)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Seek::Variation_27()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(28)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - DB_E_BADACCESSORHANDLE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Seek::Variation_28()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetIndex::Seek(1,DB_E_BADACCESSORHANDLE) "
		L"			{ "
		L"			} " 
		L"		 IRowsetIndex::Seek(1) "
		L"			{ "
		L"			 L1.ROWSET_FETCHPOSITIONCHANGE.OKTODO.RETURN_ACCEPT "
		L"			} " 
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, ROWSETINDEX_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(29)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - EmptyRowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Seek::Variation_29()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowsetIndex::Seek(1,DB_S_ENDOFROWSET) "
		L"			{ "
		L"			} " 
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, EMPTY_ROWSET | ROWSETINDEX_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(30)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - Add/Remove Listeners
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Seek::Variation_30()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetIndex::Seek(1) "
		L"			{ "
		L"			 L1.ROWSET_FETCHPOSITIONCHANGE.OKTODO.RETURN_ADVISE "
		L"			} " 
		L"		 IRowsetIndex::Seek(1) "
		L"			{ "
		L"			 L2.ROWSET_FETCHPOSITIONCHANGE.OKTODO.RETURN_UNADVISE "
		L"			} " 
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, ROWSETINDEX_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(31)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - GetData
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Seek::Variation_31()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetIndex::Seek(1) "
		L"			{ "
		L"			 L1.ROWSET_FETCHPOSITIONCHANGE.OKTODO.IRowset::GetData(A) {} "
		L"			} " 
		L"		 IRowsetIndex::Seek(1) "
		L"			{ "
		L"			 L2.ROWSET_FETCHPOSITIONCHANGE.OKTODO.IRowset::GetData(B) "
		L"			} " 
		L"		}"
		L"	}"
		L"}";


	return Execute(CommandString, ROWSETINDEX_ROWSET);
}
// }}
// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Seek::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CExecutionManager::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(GetRowsAt)
//*-----------------------------------------------------------------------
//| Test Case:		GetRowsAt - Tests Notificatons for IRowsetLocate::GetRowsAt
//|	Created:			09/29/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL GetRowsAt::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CExecutionManager::Init())
	// }}
	{
		//Verify Rowset can be created...
		TEST_PROVIDER(CreateRowset(ROWSETLOCATE_ROWSET)==S_OK);
		return TRUE;
	}

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc ROW_ACTIVATE - SINGLEROW - Accept all phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetRowsAt::Variation_1()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowsetLocate::GetRowsAt(DBBMK_FIRST, A) "
		L"			{"
		L"			 L1.ROW_ACTIVATE.DIDEVENT.RETURN_ACCEPT "
		L"			}"
		L"		 IRowsetLocate::GetRowsAt(DBBMK_FIRST, A) {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, ROWSETLOCATE_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc ROW_ACTIVATE - MULTIPLEROWS - Accept all phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetRowsAt::Variation_2()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowsetLocate::GetRowsAt(DBBMK_FIRST, A,B,C) "
		L"			{"
		L"			 L1.ROW_ACTIVATE.DIDEVENT.RETURN_ACCEPT "
		L"			}"
		L"		 IRowsetLocate::GetRowsAt(DBBMK_FIRST, A,B,C) {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, ROWSETLOCATE_ROWSET);
}					  
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetRowsAt::Variation_3()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetRowsAt::Variation_4()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc ROW_ACTIVATE - DB_E_CANCELED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetRowsAt::Variation_5()
{
	//Cannot "successfully" veto DIDEVENT
	//Verify rows are valid after veto
    
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowsetLocate::GetRowsAt(DBBMK_FIRST, A,B,C) "
		L"			{"
		L"			 L1.ROW_ACTIVATE.DIDEVENT.RETURN_VETO "
		L"			}"
		L"		 IRowset::GetData(A) {} "
		L"		 IRowset::GetData(B) {} "
		L"		 IRowset::GetData(C) {} "
		L"		 IRowsetLocate::GetRowsAt(DBBMK_FIRST, A,B,C) {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, ROWSETLOCATE_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc ROW_ACTIVATE - DB_E_NOTREENTRANT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetRowsAt::Variation_6()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowsetLocate::GetRowsAt(DBBMK_FIRST, A,B,C) "
		L"			{"
		L"			 L1.ROW_ACTIVATE.DIDEVENT.IRowsetLocate::GetRowsAt(DBBMK_FIRST, A,B,C, DB_E_NOTREENTRANT) {} "
		L"			}"
		L"		 IRowsetLocate::GetRowsAt(DBBMK_FIRST, A,B,C) {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, ROWSETLOCATE_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetRowsAt::Variation_7()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetRowsAt::Variation_8()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc ROW_ACTIVATE - Veto DIDEVENT, Listener 1
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetRowsAt::Variation_9()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowsetLocate::GetRowsAt(DBBMK_FIRST, A,B,C) "
		L"			{"
		L"			 L1.ROW_ACTIVATE.DIDEVENT.RETURN_VETO "
		L"			}"
		L"		 IRowsetLocate::GetRowsAt(DBBMK_FIRST, A,B,C) {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, ROWSETLOCATE_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetRowsAt::Variation_10()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetRowsAt::Variation_11()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc ROW_ACTIVATE - UNWANTED REASON
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetRowsAt::Variation_12()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowsetLocate::GetRowsAt(DBBMK_FIRST, A,B,C) "
		L"			{"
		L"			 L1.ROW_ACTIVATE.DIDEVENT.RETURN_UNWANTEDREASON "
		L"			}"
		L"		 IRowsetLocate::GetRowsAt(DBBMK_FIRST, A,B,C) {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, ROWSETLOCATE_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc ROW_ACTIVATE - UNWANTED PHASE - DIDEVENT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetRowsAt::Variation_13()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowsetLocate::GetRowsAt(DBBMK_FIRST, A,B,C) "
		L"			{"
		L"			 L1.ROW_ACTIVATE.DIDEVENT.RETURN_UNWANTEDPHASE "
		L"			}"
		L"		 IRowsetLocate::GetRowsAt(DBBMK_FIRST, A,B,C) {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, ROWSETLOCATE_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetRowsAt::Variation_14()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetRowsAt::Variation_15()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc ROW_ACTIVATE - E_FAIL - All Phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetRowsAt::Variation_16()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowsetLocate::GetRowsAt(DBBMK_FIRST, A,B,C) "
		L"			{"
		L"			 L1.ROW_ACTIVATE.DIDEVENT.RETURN_EFAIL "
		L"			}"
		L"		 IRowsetLocate::GetRowsAt(DBBMK_FIRST, A,B,C) {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, ROWSETLOCATE_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc ROW_ACTIVATE - E_OUTOFMEMORY - All Phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetRowsAt::Variation_17()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowsetLocate::GetRowsAt(DBBMK_FIRST, A,B,C) "
		L"			{"
		L"			 L1.ROW_ACTIVATE.DIDEVENT.RETURN_EOUTOFMEMORY "
		L"			}"
		L"		 IRowsetLocate::GetRowsAt(DBBMK_FIRST, A,B,C) {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, ROWSETLOCATE_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc ROW_ACTIVATE - E_INVALIDARG - All Phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetRowsAt::Variation_18()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowsetLocate::GetRowsAt(DBBMK_FIRST, A,B,C) "
		L"			{"
		L"			 L1.ROW_ACTIVATE.DIDEVENT.RETURN_EINVALIDARG "
		L"			}"
		L"		 IRowsetLocate::GetRowsAt(DBBMK_FIRST, A,B,C) {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, ROWSETLOCATE_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetRowsAt::Variation_19()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetRowsAt::Variation_20()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - Get same row twice
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetRowsAt::Variation_21()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowsetLocate::GetRowsAt(DBBMK_FIRST, A,B,C) "
		L"			{"
		L"			 L1.ROW_ACTIVATE.DIDEVENT.RETURN_ACCEPT "
		L"			}"
		L"		 IRowsetLocate::GetRowsAt(DBBMK_FIRST, A,B,C) {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, ROWSETLOCATE_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(22)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - DB_E_BADSTARTPOSITION
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetRowsAt::Variation_22()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowsetLocate::GetRowsAt(DBBMK_FIRST, A,B,C, DB_E_BADSTARTPOSITION) "
		L"			{"
		L"			}"
		L"		 IRowsetLocate::GetRowsAt(DBBMK_FIRST, A,B,C) {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, ROWSETLOCATE_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(23)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - EmptyRowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetRowsAt::Variation_23()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowsetLocate::GetRowsAt(DBBMK_FIRST, DB_S_ENDOFROWSET) "
		L"			{"
		L"			}"
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, EMPTY_ROWSET | ROWSETLOCATE_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(24)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - DB_S_ENDOFROWSET - more rows than are in the rowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetRowsAt::Variation_24()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowsetLocate::GetRowsAt(DBBMK_FIRST, A,B,C,D,E,  DB_S_ENDOFROWSET) "
		L"			{"
		L"			 L1.ROW_ACTIVATE.DIDEVENT.RETURN_ACCEPT "
		L"			}"
		L"		 IRowsetLocate::GetRowsAt(DBBMK_FIRST, A,B,C) {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, ROWSETLOCATE_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(25)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - DB_S_ENDOFROWSET - at end of rowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetRowsAt::Variation_25()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowsetLocate::GetRowsAt(DBBMK_LAST, A, DB_S_ENDOFROWSET) "
		L"			{"
		L"			}"
		L"		 IRowsetLocate::GetRowsAt(DBBMK_FIRST, B,C,D) {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, ROWSETLOCATE_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(26)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS -  no rows at head of rowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetRowsAt::Variation_26()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowsetLocate::GetRowsAt(DBBMK_FIRST) "
		L"			{"
		L"			}"
		L"		 IRowsetLocate::GetRowsAt(DBBMK_FIRST, A,B,C) {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, ROWSETLOCATE_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(27)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS -  no rows at end of rowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetRowsAt::Variation_27()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowsetLocate::GetRowsAt(DBBMK_LAST) "
		L"			{"
		L"			}"
		L"		 IRowsetLocate::GetRowsAt(DBBMK_FIRST, A,B,C) {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, ROWSETLOCATE_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(28)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - Add/Remove Listeners
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetRowsAt::Variation_28()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowsetLocate::GetRowsAt(DBBMK_FIRST, A,B,C) "
		L"			{"
		L"			 L1.ROW_ACTIVATE.DIDEVENT.RETURN_UNADVISE "
		L"			}"
		L"		 IRowsetLocate::GetRowsAt(DBBMK_FIRST, A,B,C) "
		L"			{"
		L"			}"
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, ROWSETLOCATE_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(29)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - GetData
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetRowsAt::Variation_29()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowsetLocate::GetRowsAt(DBBMK_FIRST, A) {} "
		L"		 IRowsetLocate::GetRowsAt(DBBMK_FIRST, A,B) "
		L"			{"
		L"			 L1.ROW_ACTIVATE.DIDEVENT.IRowset::GetData(A) {} "
		L"			}"
		L"		 IRowsetLocate::GetRowsAt(DBBMK_FIRST, A) "
		L"			{"
		L"			}"
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, ROWSETLOCATE_ROWSET);
}
// }}
// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL GetRowsAt::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CExecutionManager::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(GetRowsByBookmark)
//*-----------------------------------------------------------------------
//| Test Case:		GetRowsByBookmark - Tests Notifications for IRowsetLocate::GetRowsbyBookmark
//|	Created:			09/29/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL GetRowsByBookmark::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CExecutionManager::Init())
	// }}
	{
		//Verify Rowset can be created...
		TEST_PROVIDER(CreateRowset(ROWSETLOCATE_ROWSET)==S_OK);
		return TRUE;
	}

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc ROW_ACTIVATE - SINGLEROW - Accept all phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetRowsByBookmark::Variation_1()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A) {} "
		L"		 IRowset::ReleaseRows(A) {} "
		L"		 IRowsetLocate::GetRowsByBookmark(A) "
		L"			{"
		L"			 L1.ROW_ACTIVATE.DIDEVENT.RETURN_ACCEPT "
		L"			}"
		L"		 IRowsetLocate::GetRowsByBookmark(A) {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, ROWSETLOCATE_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc ROW_ACTIVATE - MULTIPLEROWS - Accept all phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetRowsByBookmark::Variation_2()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B,C,D) {} "
		L"		 IRowset::ReleaseRows(A,B,C,D) {} "
		L"		 IRowsetLocate::GetRowsByBookmark(A,B) "
		L"			{"
		L"			 L1.ROW_ACTIVATE.DIDEVENT.RETURN_ACCEPT "
		L"			}"
		L"		 IRowsetLocate::GetRowsByBookmark(B,C) {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, ROWSETLOCATE_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetRowsByBookmark::Variation_3()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetRowsByBookmark::Variation_4()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc ROW_ACTIVATE - DB_E_CANCELED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetRowsByBookmark::Variation_5()
{
	//Need to verify the Veto was successful
	//Verify rows are not valid after veto
    
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B,C,D) {} "
		L"		 IRowset::ReleaseRows(A,B,C,D) {} "
		L"		 IRowsetLocate::GetRowsByBookmark(A,B) "
		L"			{"
		L"			 L1.ROW_ACTIVATE.DIDEVENT.RETURN_VETO "
		L"			}"
		L"		 IRowsetLocate::GetRowsByBookmark(B,C) {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, ROWSETLOCATE_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc ROW_ACTIVATE - DB_E_NOTREENTRANT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetRowsByBookmark::Variation_6()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B,C,D) {} "
		L"		 IRowset::ReleaseRows(A,B,C,D) {} "
		L"		 IRowsetLocate::GetRowsByBookmark(A,B) "
		L"			{"
		L"			 L1.ROW_ACTIVATE.DIDEVENT.IRowsetLocate::GetRowsByBookmark(A,B,DB_E_NOTREENTRANT) {} "
		L"			}"
		L"		 IRowsetLocate::GetRowsByBookmark(B,C) {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, ROWSETLOCATE_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetRowsByBookmark::Variation_7()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetRowsByBookmark::Variation_8()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc ROW_ACTIVATE - Veto DIDEVENT, Listener 1
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetRowsByBookmark::Variation_9()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B,C,D) {} "
		L"		 IRowset::ReleaseRows(A,B,C,D) {} "
		L"		 IRowsetLocate::GetRowsByBookmark(A,B) "
		L"			{"
		L"			 L1.ROW_ACTIVATE.DIDEVENT.RETURN_VETO "
		L"			}"
		L"		 IRowsetLocate::GetRowsByBookmark(B,C) {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, ROWSETLOCATE_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetRowsByBookmark::Variation_10()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetRowsByBookmark::Variation_11()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc ROW_ACTIVATE - UNWANTED REASON
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetRowsByBookmark::Variation_12()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B,C,D) {} "
		L"		 IRowset::ReleaseRows(A,B,C,D) {} "
		L"		 IRowsetLocate::GetRowsByBookmark(A,B) "
		L"			{"
		L"			 L1.ROW_ACTIVATE.DIDEVENT.RETURN_UNWANTEDREASON "
		L"			}"
		L"		 IRowsetLocate::GetRowsByBookmark(C,D) {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, ROWSETLOCATE_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc ROW_ACTIVATE - UNWANTED PHASE - DIDEVENT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetRowsByBookmark::Variation_13()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B,C,D) {} "
		L"		 IRowset::ReleaseRows(A,B,C,D) {} "
		L"		 IRowsetLocate::GetRowsByBookmark(A,B) "
		L"			{"
		L"			 L1.ROW_ACTIVATE.DIDEVENT.RETURN_UNWANTEDPHASE "
		L"			}"
		L"		 IRowsetLocate::GetRowsByBookmark(C,D) {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, ROWSETLOCATE_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetRowsByBookmark::Variation_14()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetRowsByBookmark::Variation_15()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc ROW_ACTIVATE - E_FAIL - All Phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetRowsByBookmark::Variation_16()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B,C,D) {} "
		L"		 IRowset::ReleaseRows(A,B,C,D) {} "
		L"		 IRowsetLocate::GetRowsByBookmark(A,B) "
		L"			{"
		L"			 L1.ROW_ACTIVATE.DIDEVENT.RETURN_EFAIL "
		L"			}"
		L"		 IRowsetLocate::GetRowsByBookmark(C,D) {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, ROWSETLOCATE_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc ROW_ACTIVATE - E_OUTOFMEMORY - All Phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetRowsByBookmark::Variation_17()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B,C,D) {} "
		L"		 IRowset::ReleaseRows(A,B,C,D) {} "
		L"		 IRowsetLocate::GetRowsByBookmark(A,B) "
		L"			{"
		L"			 L1.ROW_ACTIVATE.DIDEVENT.RETURN_EOUTOFMEMORY "
		L"			}"
		L"		 IRowsetLocate::GetRowsByBookmark(C,D) {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, ROWSETLOCATE_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc ROW_ACTIVATE - E_INVALIDARG - All Phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetRowsByBookmark::Variation_18()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B,C,D) {} "
		L"		 IRowset::ReleaseRows(A,B,C,D) {} "
		L"		 IRowsetLocate::GetRowsByBookmark(A,B) "
		L"			{"
		L"			 L1.ROW_ACTIVATE.DIDEVENT.RETURN_EINVALIDARG "
		L"			}"
		L"		 IRowsetLocate::GetRowsByBookmark(C,D) {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, ROWSETLOCATE_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetRowsByBookmark::Variation_19()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetRowsByBookmark::Variation_20()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - Get no rows
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetRowsByBookmark::Variation_21()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B,C,D) {} "
		L"		 IRowset::ReleaseRows(A,B,C,D) {} "
		L"		 IRowsetLocate::GetRowsByBookmark() "
		L"			{"
		L"			}"
		L"		 IRowsetLocate::GetRowsByBookmark(B,C) "
		L"			{"
		L"			 L1.ROW_ACTIVATE.DIDEVENT.RETURN_ACCEPT "
		L"			}"
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, ROWSETLOCATE_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(22)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - Get same row twice
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetRowsByBookmark::Variation_22()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B,C,D) {} "
		L"		 IRowset::ReleaseRows(A,B,C,D) {} "
		L"		 IRowsetLocate::GetRowsByBookmark(B) "
		L"			{"
		L"			 L1.ROW_ACTIVATE.DIDEVENT.RETURN_ACCEPT "
		L"			}"
		L"		 IRowsetLocate::GetRowsByBookmark(C) "
		L"			{"
		L"			}"
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, ROWSETLOCATE_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(23)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetRowsByBookmark::Variation_23()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B,C,D) {} "
		L"		 IRowset::ReleaseRows(A,B,C,D) {} "
		L"		 IRowsetLocate::GetRowsByBookmark(B,E_INVALIDARG) "
		L"			{"
		L"			}"
		L"		 IRowsetLocate::GetRowsByBookmark(C) "
		L"			{"
		L"			}"
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, ROWSETLOCATE_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(24)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - EmptyRowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetRowsByBookmark::Variation_24()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowsetLocate::GetRowsByBookmark(B, DB_E_ERRORSOCCURRED) "
		L"			{"
		L"			}"
		L"		 IRowsetLocate::GetRowsByBookmark(C, DB_E_ERRORSOCCURRED) "
		L"			{"
		L"			}"
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, EMPTY_ROWSET | ROWSETLOCATE_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(25)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - Add/Remove Listeners
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetRowsByBookmark::Variation_25()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B,C,D) {} "
		L"		 IRowset::ReleaseRows(A,B,C,D) {} "
		L"		 IRowsetLocate::GetRowsByBookmark(B) "
		L"			{"
		L"			 L1.ROW_ACTIVATE.DIDEVENT.RETURN_UNADVISE "
		L"			}"
		L"		 IRowsetLocate::GetRowsByBookmark(C) "
		L"			{"
		L"			 L2.ROW_ACTIVATE.DIDEVENT.RETURN_UNADVISE "
		L"			}"
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, ROWSETLOCATE_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(26)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - GetData
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetRowsByBookmark::Variation_26()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B,C,D) {} "
		L"		 IRowset::ReleaseRows(A,B,C,D) {} "
		L"		 IRowsetLocate::GetRowsByBookmark(B) "
		L"			{"
		L"			 L1.ROW_ACTIVATE.DIDEVENT.IRowset::GetData(B) {} "
		L"			}"
		L"		 IRowsetLocate::GetRowsByBookmark(C) "
		L"			{"
		L"			 L1.ROW_ACTIVATE.DIDEVENT.IRowset::GetData(C) {} "
		L"			}"
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, ROWSETLOCATE_ROWSET);
}
// }}
// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL GetRowsByBookmark::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CExecutionManager::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(GetRowsAtRatio)
//*-----------------------------------------------------------------------
//| Test Case:		GetRowsAtRatio - Test Nofications for IRowsetScroll::GetRowsAtRatio
//|	Created:			03/24/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL GetRowsAtRatio::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CExecutionManager::Init())
	// }}
	{
		//Verify Rowset can be created...
		TEST_PROVIDER(CreateRowset(ROWSETSCROLL_ROWSET)==S_OK);
		return TRUE;
	}

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc ROW_ACTIVATE - SINGLEROW - Accept all phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetRowsAtRatio::Variation_1()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowsetScroll::GetRowsAtRatio(1,A) "
		L"			{ "
		L"				L1.ROW_ACTIVATE.DIDEVENT.RETURN_ACCEPT "
		L"			} "
		L"		 IRowsetScroll::GetRowsAtRatio(1,A) {} "
		L"		}"
		L"	}"
		L"}"; 
	
	return Execute(CommandString, ROWSETSCROLL_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc ROW_ACTIVATE - MULTIPLEROWS - Accept all phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetRowsAtRatio::Variation_2()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowsetScroll::GetRowsAtRatio(1,A,B,C) "
		L"			{ "
		L"				L1.ROW_ACTIVATE.DIDEVENT.RETURN_ACCEPT "
		L"			} "
		L"		 IRowsetScroll::GetRowsAtRatio(1,A,B,C) {} "
		L"		}"
		L"	}"
		L"}"; 
	
	return Execute(CommandString, ROWSETSCROLL_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetRowsAtRatio::Variation_3()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetRowsAtRatio::Variation_4()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc ROW_ACTIVATE - DB_E_CANCELED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetRowsAtRatio::Variation_5()
{
	//Veriy the veto was successful
	//Row should be unaffected by the resynch

    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowsetScroll::GetRowsAtRatio(1,A,B,C) "
		L"			{ "
		L"				L1.ROW_ACTIVATE.DIDEVENT.RETURN_VETO "
		L"			} "
		L"		 IRowsetScroll::GetRowsAtRatio(1,A,B,C) {} "
		L"		}"
		L"	}"
		L"}"; 
	
	return Execute(CommandString, ROWSETSCROLL_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc ROW_ACTIVATE - DB_E_NOTREENTRANT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetRowsAtRatio::Variation_6()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowsetScroll::GetRowsAtRatio(1,A,B,C) "
		L"			{ "
		L"				L1.ROW_ACTIVATE.DIDEVENT.IRowsetScroll::GetRowsAtRatio(1,A,B,C,DB_E_NOTREENTRANT) {} "
		L"			} "
		L"		 IRowsetScroll::GetRowsAtRatio(1,A,B,C) {} "
		L"		}"
		L"	}"
		L"}"; 
	
	return Execute(CommandString, ROWSETSCROLL_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetRowsAtRatio::Variation_7()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetRowsAtRatio::Variation_8()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc ROW_ACTIVATE - Veto DIDEVENT, Listener 1
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetRowsAtRatio::Variation_9()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowsetScroll::GetRowsAtRatio(1,A,B,C) "
		L"			{ "
		L"				L1.ROW_ACTIVATE.DIDEVENT.RETURN_VETO "
		L"			} "
		L"		 IRowsetScroll::GetRowsAtRatio(1,A,B,C) {} "
		L"		}"
		L"	}"
		L"}"; 
	
	return Execute(CommandString, ROWSETSCROLL_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetRowsAtRatio::Variation_10()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetRowsAtRatio::Variation_11()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc ROW_ACTIVATE - UNWANTED REASON
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetRowsAtRatio::Variation_12()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowsetScroll::GetRowsAtRatio(1,A,B,C) "
		L"			{ "
		L"				L1.ROW_ACTIVATE.DIDEVENT.RETURN_UNWANTEDREASON "
		L"			} "
		L"		 IRowsetScroll::GetRowsAtRatio(1,A,B,C,D) {} "
		L"		}"
		L"	}"
		L"}"; 
	
	return Execute(CommandString, ROWSETSCROLL_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc ROW_ACTIVATE - UNWANTED PHASE - DIDEVENT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetRowsAtRatio::Variation_13()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowsetScroll::GetRowsAtRatio(1,A,B,C) "
		L"			{ "
		L"				L1.ROW_ACTIVATE.DIDEVENT.RETURN_UNWANTEDPHASE "
		L"			} "
		L"		 IRowsetScroll::GetRowsAtRatio(1,A,B,C,D) {} "
		L"		}"
		L"	}"
		L"}"; 
	
	return Execute(CommandString, ROWSETSCROLL_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetRowsAtRatio::Variation_14()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetRowsAtRatio::Variation_15()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc ROW_ACTIVATE - E_FAIL - All Phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetRowsAtRatio::Variation_16()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowsetScroll::GetRowsAtRatio(1,A,B,C) "
		L"			{ "
		L"				L1.ROW_ACTIVATE.DIDEVENT.RETURN_EFAIL "
		L"			} "
		L"		 IRowsetScroll::GetRowsAtRatio(1,A,B,C,D) {} "
		L"		}"
		L"	}"
		L"}"; 
	
	return Execute(CommandString, ROWSETSCROLL_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc ROW_ACTIVATE - E_OUTOFMEMORY - All Phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetRowsAtRatio::Variation_17()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowsetScroll::GetRowsAtRatio(1,A,B,C) "
		L"			{ "
		L"				L1.ROW_ACTIVATE.DIDEVENT.RETURN_EOUTOFMEMORY "
		L"			} "
		L"		 IRowsetScroll::GetRowsAtRatio(1,A,B,C,D) {} "
		L"		}"
		L"	}"
		L"}"; 
	
	return Execute(CommandString, ROWSETSCROLL_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc ROW_ACTIVATE - E_INVALIDARG - All Phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetRowsAtRatio::Variation_18()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowsetScroll::GetRowsAtRatio(1,A,B,C) "
		L"			{ "
		L"				L1.ROW_ACTIVATE.DIDEVENT.RETURN_EINVALIDARG "
		L"			} "
		L"		 IRowsetScroll::GetRowsAtRatio(1,A,B,C,D) {} "
		L"		}"
		L"	}"
		L"}"; 
	
	return Execute(CommandString, ROWSETSCROLL_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetRowsAtRatio::Variation_19()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetRowsAtRatio::Variation_20()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - No rows
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetRowsAtRatio::Variation_21()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowsetScroll::GetRowsAtRatio(1) "
		L"			{ "
		L"			} "
		L"		 IRowsetScroll::GetRowsAtRatio(1) "
		L"			{ "
		L"			} "
		L"		}"
		L"	}"
		L"}"; 
	
	return Execute(CommandString, ROWSETSCROLL_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(22)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - DB_S_ENDOFROWSET 1 more than rowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetRowsAtRatio::Variation_22()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowsetScroll::GetRowsAtRatio(1, A,B,C,D, DB_S_ENDOFROWSET) "
		L"			{ "
		L"				L1.ROW_ACTIVATE.DIDEVENT.RETURN_ACCEPT "
		L"			} "
		L"		 IRowsetScroll::GetRowsAtRatio(1, A,B,C,D) {} "
		L"		}"
		L"	}"
		L"}"; 
	
	return Execute(CommandString, ROWSETSCROLL_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(23)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - DB_S_ENDOFROWSET - at end of rowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetRowsAtRatio::Variation_23()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowsetScroll::GetRowsAtRatio(1, A,B,C, DB_S_ENDOFROWSET) "
		L"			{ "
		L"				L1.ROW_ACTIVATE.DIDEVENT.RETURN_ACCEPT "
		L"			} "
		L"		 IRowsetScroll::GetRowsAtRatio(1, A,B,C, DB_S_ENDOFROWSET) "
		L"			{ "
		L"			} "
		L"		}"
		L"	}"
		L"}"; 
	
	return Execute(CommandString, ROWSETSCROLL_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(24)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - EmptyRowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetRowsAtRatio::Variation_24()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowsetScroll::GetRowsAtRatio(1, DB_S_ENDOFROWSET) "
		L"			{ "
		L"			} "
		L"		 IRowsetScroll::GetRowsAtRatio(1, DB_S_ENDOFROWSET) "
		L"			{ "
		L"			} "
		L"		}"
		L"	}"
		L"}"; 
	
	return Execute(CommandString, EMPTY_ROWSET | ROWSETSCROLL_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(25)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - Add/Remove Listeners
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetRowsAtRatio::Variation_25()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowsetScroll::GetRowsAtRatio(1,A,B,C) "
		L"			{ "
		L"				L1.ROW_ACTIVATE.DIDEVENT.RETURN_UNADVISE "
		L"			} "
		L"		 IRowsetScroll::GetRowsAtRatio(1,A,B,C,D) {} "
		L"		}"
		L"	}"
		L"}"; 
	
	return Execute(CommandString, ROWSETSCROLL_ROWSET);
}
// }}


// {{ TCW_VAR_PROTOTYPE(26)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - GetData
//
// @rdesc TEST_PASS or TEST_FAIL
//
int GetRowsAtRatio::Variation_26()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B,C) {} "
		L"		 IRowset::ReleaseRows(A,B,C) {} "
		L"		 IRowsetScroll::GetRowsAtRatio(1,A,B,C) "
		L"			{ "
		L"				L1.ROW_ACTIVATE.DIDEVENT.IRowset::GetData(A) {} "
		L"				L2.ROW_ACTIVATE.DIDEVENT.IRowset::GetData(B) {} "
		L"				L3.ROW_ACTIVATE.DIDEVENT.IRowset::GetData(C) {} "
		L"			} "
		L"		 IRowsetScroll::GetRowsAtRatio(1,A,B,C,D) {} "
		L"		}"
		L"	}"
		L"}"; 
	
	return Execute(CommandString, ROWSETSCROLL_ROWSET);
}
// }}
// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL GetRowsAtRatio::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CExecutionManager::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(DeleteRows)
//*-----------------------------------------------------------------------
//| Test Case:		DeleteRows - Tests Notifications for IRowsetChange::DeleteRows
//|	Created:			09/29/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL DeleteRows::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CExecutionManager::Init())
	// }}
	{
		//Verify Rowset can be created...
		TEST_PROVIDER(CreateRowset(CHANGE_IMMEDIATE)==S_OK);
		return TRUE;
	}

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc ROW_DELETE - SINGLEROW - Accept all phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows::Variation_1()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;" 
		L"	{Control Control1;"
		L"		{ "
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::DeleteRows(A) {} "
		L"		} "
		L"	} "
		L"}";

	TableInsert(ONE_ROW);  //Compensate for the deleted row(s)
	return Execute(CommandString, CHANGE_IMMEDIATE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc ROW_DELETE - MULTIPLEROWS - Accept all phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows::Variation_2()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;" 
		L"	{Control Control1;"
		L"		{ "
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::DeleteRows(A,B) {} "
		L"		} "
		L"	} "
		L"}";

	TableInsert(TWO_ROWS);  //Compensate for the deleted row(s)
	return Execute(CommandString, CHANGE_IMMEDIATE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows::Variation_3()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc ROW_DELETE - MULTIPLEROWS - DB_E_CANCELED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows::Variation_4()
{
	//Need to make sure the Veto of ROW_DELETE did not delete the row
 
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;" 
		L"	{Control Control1;"
		L"		{ "
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::DeleteRows(A,B) "
		L"			{ "
		L"			 L1.ROW_DELETE.ABOUTTODO.RETURN_VETO "
		L"			} "
		L"		 IRowset::GetData(A) {} "
		L"		} "
		L"	} "
		L"}";

	//To produce FAILEDTODO, this reason/phase must be vetoable...
	TESTC_PROVIDER(IsCancelableEvent(DBREASON_ROW_DELETE, DBEVENTPHASE_ABOUTTODO));
	
	TableInsert(TWO_ROWS);  //Compensate for the deleted row(s)
	TESTC(Execute(CommandString, CHANGE_IMMEDIATE));

CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc ROW_DELETE - SINGLEROW - DB_E_NOTREENTRANT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows::Variation_5()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;" 
		L"	{Control Control1;"
		L"		{ "
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::DeleteRows(A) "
		L"			{ "
		L"			 L1.ROW_DELETE.OKTODO.IRowsetChange::DeleteRows(B, DB_E_NOTREENTRANT) {} "
		L"			} "
		L"		} "
		L"	} "
		L"}";

	TableInsert(ONE_ROW);  //Compensate for the deleted row(s)
	return Execute(CommandString, CHANGE_IMMEDIATE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows::Variation_6()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows::Variation_7()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc ROW_DELETE - MULTIPLEROWS - Veto OKTODO Listener 1
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows::Variation_8()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;" 
		L"	{Control Control1;"
		L"		{ "
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::DeleteRows(A,B) "
		L"			{ "
		L"			 L1.ROW_DELETE.OKTODO.RETURN_VETO "
		L"			} "
		L"		} "
		L"	} "
		L"}";


	TableInsert(TWO_ROWS);  //Compensate for the deleted row(s)
	TESTC(Execute(CommandString, CHANGE_IMMEDIATE));

CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc ROW_DELETE - MULTIPLEROWS - Veto ABOUTTODO Listener 1
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows::Variation_9()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;" 
		L"	{Control Control1;"
		L"		{ "
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::DeleteRows(A,B) "
		L"			{ "
		L"			 L1.ROW_DELETE.ABOUTTODO.RETURN_VETO "
		L"			} "
		L"		} "
		L"	} "
		L"}";

	
	TableInsert(TWO_ROWS);  //Compensate for the deleted row(s)
	TESTC(Execute(CommandString, CHANGE_IMMEDIATE));

CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc ROW_DELETE - MULTIPLEROWS - Veto SYNCHAFTER Listener 1
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows::Variation_10()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;" 
		L"	{Control Control1;"
		L"		{ "
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::DeleteRows(A,B) "
		L"			{ "
		L"			 L2.ROW_DELETE.SYNCHAFTER.RETURN_VETO "
		L"			} "
		L"		} "
		L"	} "
		L"}";

	
	TableInsert(TWO_ROWS);  //Compensate for the deleted row(s)
	TESTC(Execute(CommandString, CHANGE_IMMEDIATE));

CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc ROW_DELETE - MULTIPLEROWS - Veto DIDEVENT Listener 2
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows::Variation_11()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;" 
		L"	{Control Control1;"
		L"		{ "
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::DeleteRows(A,B) "
		L"			{ "
		L"			 L1.ROW_DELETE.DIDEVENT.RETURN_VETO "
		L"			} "
		L"		} "
		L"	} "
		L"}";

	TableInsert(TWO_ROWS);  //Compensate for the deleted row(s)
	return Execute(CommandString, CHANGE_IMMEDIATE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc ROW_DELETE - MULTIPLEROWS - Veto FAILEDTODO Listener 2
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows::Variation_12()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;" 
		L"	{Control Control1;"
		L"		{ "
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::DeleteRows(A,B) "
		L"			{ "
		L"			 L1.ROW_DELETE.ABOUTTODO.RETURN_VETO "
		L"			 L1.ROW_DELETE.FAILEDTODO.RETURN_VETO "
		L"			} "
		L"		} "
		L"	} "
		L"}";

	//To produce FAILEDTODO, this reason/phase must be vetoable...
	TESTC_PROVIDER(IsCancelableEvent(DBREASON_ROW_DELETE, DBEVENTPHASE_ABOUTTODO));
	
	TableInsert(TWO_ROWS);  //Compensate for the deleted row(s)
	TESTC(Execute(CommandString, CHANGE_IMMEDIATE));

CLEANUP:
	TRETURN
}
// }}

// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - Add/Remove Listeners - SINGLEROW
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows::Variation_13()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{ "
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::DeleteRows(A) "
		L"			{ "
		L"			 L1.ROW_DELETE.ABOUTTODO.RETURN_UNADVISE "
		L"			 L3.ROW_DELETE.SYNCHAFTER.RETURN_ADVISE "
		L"			} "
		L"		 IRowsetChange::DeleteRows(B) "
		L"			{ "
		L"			} "
		L"		} "
		L"	} "
		L"}";

	TableInsert(TWO_ROWS);  //Compensate for the deleted row(s)
	return Execute(CommandString, CHANGE_IMMEDIATE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - GetData - SINGLEROW
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows::Variation_14()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{ "
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::DeleteRows(A) "
		L"			{ "
		L"			 L1.ROW_DELETE.OKTODO.IRowset::GetData(A) {} "
		L"			 L3.ROW_DELETE.ABOUTTODO.IRowset::GetData(A) {} "
		L"			} "
		L"		 IRowsetChange::DeleteRows(B) "
		L"			{ "
		L"			 L3.ROW_DELETE.OKTODO.IRowset::GetData(B) {} "
		L"			} "
		L"		} "
		L"	} "
		L"}";

	TableInsert(TWO_ROWS);  //Compensate for the deleted row(s)
	return Execute(CommandString, CHANGE_IMMEDIATE);
}
// }}



// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc ROW_DELETE - SINGLEROW - UNWANTED REASON
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows::Variation_15()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;" 
		L"	{Control Control1;"
		L"		{ "
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::DeleteRows(A) "
		L"			{ "
		L"			 L1.ROW_DELETE.OKTODO.RETURN_UNWANTEDREASON "
		L"			} "
		L"		 IRowsetChange::DeleteRows(B) {} "
		L"		} "
		L"	} "
		L"}";

	TableInsert(TWO_ROWS);  //Compensate for the deleted row(s)
	return Execute(CommandString, CHANGE_IMMEDIATE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc ROW_DELETE - SINGLEROW - UNWANTED PHASE OKTODO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows::Variation_16()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;" 
		L"	{Control Control1;"
		L"		{ "
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::DeleteRows(A) "
		L"			{ "
		L"			 L1.ROW_DELETE.OKTODO.RETURN_UNWANTEDPHASE "
		L"			} "
		L"		 IRowsetChange::DeleteRows(B) {} "
		L"		} "
		L"	} "
		L"}";

	TableInsert(TWO_ROWS);  //Compensate for the deleted row(s)
	return Execute(CommandString, CHANGE_IMMEDIATE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc ROW_DELETE - SINGLEROW - UNWANTED PHASE ABOUTTODO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows::Variation_17()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;" 
		L"	{Control Control1;"
		L"		{ "
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::DeleteRows(A) "
		L"			{ "
		L"			 L1.ROW_DELETE.ABOUTTODO.RETURN_UNWANTEDPHASE "
		L"			} "
		L"		 IRowsetChange::DeleteRows(B) {} "
		L"		} "
		L"	} "
		L"}";

	TableInsert(TWO_ROWS);  //Compensate for the deleted row(s)
	return Execute(CommandString, CHANGE_IMMEDIATE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc ROW_DELETE - SINGLEROW - UNWANTED PHASE SYNCHAFTER
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows::Variation_18()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;" 
		L"	{Control Control1;"
		L"		{ "
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::DeleteRows(A) "
		L"			{ "
		L"			 L1.ROW_DELETE.SYNCHAFTER.RETURN_UNWANTEDPHASE "
		L"			} "
		L"		 IRowsetChange::DeleteRows(B) {} "
		L"		} "
		L"	} "
		L"}";

	TableInsert(TWO_ROWS);  //Compensate for the deleted row(s)
	return Execute(CommandString, CHANGE_IMMEDIATE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc ROW_DELETE - SINGLEROW - UNWANTED PHASE DIDEVENT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows::Variation_19()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;" 
		L"	{Control Control1;"
		L"		{ "
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::DeleteRows(A) "
		L"			{ "
		L"			 L1.ROW_DELETE.DIDEVENT.RETURN_UNWANTEDPHASE "
		L"			} "
		L"		 IRowsetChange::DeleteRows(B) {} "
		L"		} "
		L"	} "
		L"}";

	TableInsert(TWO_ROWS);  //Compensate for the deleted row(s)
	return Execute(CommandString, CHANGE_IMMEDIATE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc ROW_DELETE - MULTIPLEROWS - Veto ABOUTTODO Listener 1 - UNWANTED PHASE FAILEDTODO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows::Variation_20()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;" 
		L"	{Control Control1;"
		L"		{ "
		L"		 IRowset::GetNextRows(A,B,C,D) {} "
		L"		 IRowsetChange::DeleteRows(A,B) "
		L"			{ "
		L"			 L1.ROW_DELETE.ABOUTTODO.RETURN_VETO "
		L"			 L1.ROW_DELETE.FAILEDTODO.RETURN_UNWANTEDPHASE "
		L"			} "
		L"		 IRowsetChange::DeleteRows(C,D) {} "
		L"		} "
		L"	} "
		L"}";

	//To produce FAILEDTODO, this reason/phase must be vetoable...
	TESTC_PROVIDER(IsCancelableEvent(DBREASON_ROW_DELETE, DBEVENTPHASE_ABOUTTODO));

	TableInsert(FOUR_ROWS);  //Compensate for the deleted row(s)
	TESTC(Execute(CommandString, CHANGE_IMMEDIATE));

CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - DBROWSTATUS_DELETED - SINGLEROW
//
// @rdesc TEST_PASS or TEST_FAIL
//

int DeleteRows::Variation_21()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;" 
		L"	{Control Control1;"
		L"		{ "
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::DeleteRows(A) "
		L"			{ "
		L"			 L1.ROW_DELETE.ABOUTTODO.RETURN_ACCEPT "
		L"			} "
		L"		 IRowsetChange::DeleteRows(A,DB_E_ERRORSOCCURRED) "
		L"			{ "
		L"			} "
		L"		} "
		L"	} "
		L"}";

	TableInsert(ONE_ROW);  //Compensate for the deleted row(s)
	return Execute(CommandString, CHANGE_IMMEDIATE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(22)
//*-----------------------------------------------------------------------
// @mfunc ROW_DELETE - SINGLEROW - Veto ABOUTTODO Listener 1 - E_FAIL - All Phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows::Variation_22()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{ "
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::DeleteRows(A) "
		L"			{ "
		L"			 L1.ROW_DELETE.OKTODO.RETURN_EFAIL "
		L"			 L2.ROW_DELETE.ABOUTTODO.RETURN_EFAIL "
		L"			 L3.ROW_DELETE.SYNCHAFTER.RETURN_EFAIL "
		L"			 L1.ROW_DELETE.DIDEVENT.RETURN_EFAIL "
		L"			} "
		L"		 IRowsetChange::DeleteRows(B) "
		L"			{ "
		L"			 L1.ROW_DELETE.ABOUTTODO.RETURN_VETO "
		L"			} "
		L"		} "
		L"	} "
		L"}";

	TableInsert(TWO_ROWS);  //Compensate for the deleted row(s)
	TESTC(Execute(CommandString, CHANGE_IMMEDIATE));

CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(23)
//*-----------------------------------------------------------------------
// @mfunc ROW_DELETE - MULTIPLEROWS - Veto ABOUTTODO Listener 1 - E_FAIL - All Phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows::Variation_23()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{ "
		L"		 IRowset::GetNextRows(A,B,C,D) {} "
		L"		 IRowsetChange::DeleteRows(A,B) "
		L"			{ "
		L"			 L1.ROW_DELETE.OKTODO.RETURN_EFAIL "
		L"			 L2.ROW_DELETE.ABOUTTODO.RETURN_EFAIL "
		L"			 L3.ROW_DELETE.SYNCHAFTER.RETURN_EFAIL "
		L"			 L1.ROW_DELETE.DIDEVENT.RETURN_EFAIL "
		L"			} "
		L"		 IRowsetChange::DeleteRows(C,D) "
		L"			{ "
		L"			 L1.ROW_DELETE.ABOUTTODO.RETURN_VETO "
		L"			} "
		L"		} "
		L"	} "
		L"}";

		//To produce FAILEDTODO, this reason/phase must be vetoable...
	//TESTC_PROVIDER(IsCancelableEvent(DBREASON_ROW_DELETE, DBEVENTPHASE_ABOUTTODO));

	TableInsert(FOUR_ROWS);  //Compensate for the deleted row(s)
	TESTC(Execute(CommandString, CHANGE_IMMEDIATE));

CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(24)
//*-----------------------------------------------------------------------
// @mfunc ROW_DELETE - MULTIPLEROWS - Veto ABOUTTODO Listener 1 - E_OUTOFMEMORY - All Phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows::Variation_24()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{ "
		L"		 IRowset::GetNextRows(A,B,C,D) {} "
		L"		 IRowsetChange::DeleteRows(A,B) "
		L"			{ "
		L"			 L1.ROW_DELETE.OKTODO.RETURN_EOUTOFMEMORY "
		L"			 L2.ROW_DELETE.ABOUTTODO.RETURN_EOUTOFMEMORY "
		L"			 L3.ROW_DELETE.SYNCHAFTER.RETURN_EOUTOFMEMORY "
		L"			 L1.ROW_DELETE.DIDEVENT.RETURN_EOUTOFMEMORY "
		L"			} "
		L"		 IRowsetChange::DeleteRows(C,D) "
		L"			{ "
		L"			 L1.ROW_DELETE.ABOUTTODO.RETURN_VETO "
		L"			} "
		L"		} "
		L"	} "
		L"}";


	TableInsert(FOUR_ROWS);  //Compensate for the deleted row(s)
	TESTC(Execute(CommandString, CHANGE_IMMEDIATE));

CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(25)
//*-----------------------------------------------------------------------
// @mfunc ROW_DELETE - MULTIPLEROWS - Veto ABOUTTODO Listener 1 - E_INVALIDARG - All Phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows::Variation_25()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{ "
		L"		 IRowset::GetNextRows(A,B,C,D) {} "
		L"		 IRowsetChange::DeleteRows(A,B) "
		L"			{ "
		L"			 L1.ROW_DELETE.OKTODO.RETURN_EINVALIDARG "
		L"			 L2.ROW_DELETE.ABOUTTODO.RETURN_EINVALIDARG "
		L"			 L3.ROW_DELETE.SYNCHAFTER.RETURN_EINVALIDARG "
		L"			 L1.ROW_DELETE.DIDEVENT.RETURN_EINVALIDARG "
		L"			} "
		L"		 IRowsetChange::DeleteRows(C,D) "
		L"			{ "
		L"			 L1.ROW_DELETE.ABOUTTODO.RETURN_VETO "
		L"			} "
		L"		} "
		L"	} "
		L"}";


	TableInsert(FOUR_ROWS);  //Compensate for the deleted row(s)
	TESTC(Execute(CommandString, CHANGE_IMMEDIATE));

CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(26)
//*-----------------------------------------------------------------------
// @mfunc ROW_DELETE - SINGLEROW - E_OUTOFMEMORY - All Phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows::Variation_26()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{ "
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::DeleteRows(A) "
		L"			{ "
		L"			 L1.ROW_DELETE.OKTODO.RETURN_EOUTOFMEMORY "
		L"			 L2.ROW_DELETE.ABOUTTODO.RETURN_EOUTOFMEMORY "
		L"			 L3.ROW_DELETE.SYNCHAFTER.RETURN_EOUTOFMEMORY "
		L"			 L1.ROW_DELETE.DIDEVENT.RETURN_EOUTOFMEMORY "
		L"			} "
		L"		 IRowsetChange::DeleteRows(B) "
		L"			{ "
		L"			 L1.ROW_DELETE.ABOUTTODO.RETURN_ACCEPT "
		L"			} "
		L"		} "
		L"	} "
		L"}";


	TableInsert(TWO_ROWS);  //Compensate for the deleted row(s)
	return Execute(CommandString, CHANGE_IMMEDIATE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(27)
//*-----------------------------------------------------------------------
// @mfunc ROW_DELETE - SINGLEROW - E_INVALIDARG - All Phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows::Variation_27()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{ "
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::DeleteRows(A) "
		L"			{ "
		L"			 L1.ROW_DELETE.OKTODO.RETURN_EINVALIDARG "
		L"			 L2.ROW_DELETE.ABOUTTODO.RETURN_EINVALIDARG "
		L"			 L3.ROW_DELETE.SYNCHAFTER.RETURN_EINVALIDARG "
		L"			 L1.ROW_DELETE.DIDEVENT.RETURN_EINVALIDARG "
		L"			} "
		L"		 IRowsetChange::DeleteRows(B) "
		L"			{ "
		L"			 L1.ROW_DELETE.ABOUTTODO.RETURN_ACCEPT "
		L"			} "
		L"		} "
		L"	} "
		L"}";


	TableInsert(TWO_ROWS);  //Compensate for the deleted row(s)
	return Execute(CommandString, CHANGE_IMMEDIATE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(28)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - Delete no rows, Delete MULTIPLEROWS
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows::Variation_28()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;" 
		L"	{Control Control1;"
		L"		{ "
		L"		 IRowset::GetNextRows(A,B,C,D) {} "
		L"		 IRowsetChange::DeleteRows() "
		L"			{ "
		L"			} "
		L"		 IRowsetChange::DeleteRows(A,B) "
		L"			{ "
		L"			 L1.ROW_DELETE.ABOUTTODO.RETURN_ACCEPT "
		L"			} "
		L"		} "
		L"	} "
		L"}";

	TableInsert(TWO_ROWS);  //Compensate for the deleted row(s)
	return Execute(CommandString, CHANGE_IMMEDIATE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(29)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - DBROWSTATUS_DELETED - MULTIPLEROWS 
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows::Variation_29()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;" 
		L"	{Control Control1;"
		L"		{ "
		L"		 IRowset::GetNextRows(A,B,C,D) {} "
		L"		 IRowsetChange::DeleteRows(A,B) "
		L"			{ "
		L"			 L1.ROW_DELETE.ABOUTTODO.RETURN_ACCEPT "
		L"			} "
		L"		 IRowsetChange::DeleteRows(A,B,DB_E_ERRORSOCCURRED) "
		L"			{ "
		L"			} "
		L"		} "
		L"	} "
		L"}";

	TableInsert(TWO_ROWS);  //Compensate for the deleted row(s)
	return Execute(CommandString, CHANGE_IMMEDIATE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(30)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - EmptyRowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows::Variation_30()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;" 
		L"	{Control Control1;"
		L"		{ "
		L"		 IRowsetChange::DeleteRows() "
		L"			{ "
		L"			} "
		L"		 IRowsetChange::DeleteRows() "
		L"			{ "
		L"			} "
		L"		} "
		L"	} "
		L"}";

	TableInsert(TWO_ROWS);  //Compensate for the deleted row(s)
	return Execute(CommandString, EMPTY_ROWSET | CHANGE_IMMEDIATE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(31)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - Add/Remove Listeners - MULTIPLEROWS
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows::Variation_31()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{ "
		L"		 IRowset::GetNextRows(A,B,C,D) {} "
		L"		 IRowsetChange::DeleteRows(A,B) "
		L"			{ "
		L"			 L1.ROW_DELETE.ABOUTTODO.RETURN_UNADVISE "
		L"			 L3.ROW_DELETE.SYNCHAFTER.RETURN_ADVISE "
		L"			} "
		L"		 IRowsetChange::DeleteRows(C) "
		L"			{ "
		L"			} "
		L"		} "
		L"	} "
		L"}";

	TableInsert(THREE_ROWS);  //Compensate for the deleted row(s)
	return Execute(CommandString, CHANGE_IMMEDIATE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(32)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - GetData - MULTIPLEROWS
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows::Variation_32()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{ "
		L"		 IRowset::GetNextRows(A,B,C,D) {} "
		L"		 IRowsetChange::DeleteRows(A,B) "
		L"			{ "
		L"			 L1.ROW_DELETE.OKTODO.IRowset::GetData(A) {} "
		L"			 L3.ROW_DELETE.ABOUTTODO.IRowset::GetData(B) {} "
		L"			} "
		L"		 IRowsetChange::DeleteRows(C) "
		L"			{ "
		L"			 L3.ROW_DELETE.OKTODO.IRowset::GetData(C) {} "
		L"			} "
		L"		} "
		L"	} "
		L"}";

	TableInsert(THREE_ROWS);  //Compensate for the deleted row(s)
	return Execute(CommandString, CHANGE_IMMEDIATE);
}
// }}

// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL DeleteRows::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CExecutionManager::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(DeleteRows_Bufferred)
//*-----------------------------------------------------------------------
//| Test Case:		DeleteRows_Bufferred - Test Notifications for IRowsetChange::DeleteRows in Buffered Mode
//|	Created:			09/29/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL DeleteRows_Bufferred::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CExecutionManager::Init())
	// }}
	{
		//Verify Rowset can be created...
		TEST_PROVIDER(CreateRowset(CHANGE_BUFFERRED)==S_OK);
		return TRUE;
	}

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc ROW_DELETE - SINGLEROW - Accept all phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows_Bufferred::Variation_1()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;" 
		L"	{Control Control1;"
		L"		{ "
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::DeleteRows(A) {} "
		L"		} "
		L"	} "
		L"}";

	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc ROW_DELETE - MULTIPLEROWS - Accept all phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows_Bufferred::Variation_2()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;" 
		L"	{Control Control1;"
		L"		{ "
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::DeleteRows(A,B) {} "
		L"		} "
		L"	} "
		L"}";

	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows_Bufferred::Variation_3()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc ROW_DELETE - DB_E_CANCELED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows_Bufferred::Variation_4()
{
	//Need to make sure the Veto of ROW_DELETE did not delete the row
    //Make sure row is valid after Veto

	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;" 
		L"	{Control Control1;"
		L"		{ "
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::DeleteRows(A,B) "
		L"			{ "
		L"			 L1.ROW_DELETE.ABOUTTODO.RETURN_VETO "
		L"			} "
		L"		 IRowset::GetData(A) {} "
		L"		} "
		L"	} "
		L"}";

	//To produce FAILEDTODO, this reason/phase must be vetoable...
	TESTC_PROVIDER(IsCancelableEvent(DBREASON_ROW_DELETE, DBEVENTPHASE_ABOUTTODO));
	TESTC(Execute(CommandString, CHANGE_BUFFERRED));

CLEANUP:
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc ROW_DELETE - DB_E_NOTREENTRANT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows_Bufferred::Variation_5()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;" 
		L"	{Control Control1;"
		L"		{ "
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::DeleteRows(A) "
		L"			{ "
		L"			 L1.ROW_DELETE.OKTODO.IRowsetChange::DeleteRows(B, DB_E_NOTREENTRANT) {} "
		L"			} "
		L"		} "
		L"	} "
		L"}";

	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows_Bufferred::Variation_6()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows_Bufferred::Variation_7()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc ROW_DELETE - Veto OKTODO Listener 1
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows_Bufferred::Variation_8()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;" 
		L"	{Control Control1;"
		L"		{ "
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::DeleteRows(A,B) "
		L"			{ "
		L"			 L1.ROW_DELETE.OKTODO.RETURN_VETO "
		L"			} "
		L"		} "
		L"	} "
		L"}";

	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc ROW_DELETE - Veto ABOUTTODO Listener 1
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows_Bufferred::Variation_9()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;" 
		L"	{Control Control1;"
		L"		{ "
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::DeleteRows(A,B) "
		L"			{ "
		L"			 L1.ROW_DELETE.ABOUTTODO.RETURN_VETO "
		L"			} "
		L"		} "
		L"	} "
		L"}";

	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc ROW_DELETE - Veto SYNCHAFTER Listener 1
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows_Bufferred::Variation_10()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;" 
		L"	{Control Control1;"
		L"		{ "
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::DeleteRows(A,B) "
		L"			{ "
		L"			 L2.ROW_DELETE.SYNCHAFTER.RETURN_VETO "
		L"			} "
		L"		} "
		L"	} "
		L"}";

	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc ROW_DELETE - Veto DIDEVENT Listener 2
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows_Bufferred::Variation_11()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;" 
		L"	{Control Control1;"
		L"		{ "
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::DeleteRows(A,B) "
		L"			{ "
		L"			 L1.ROW_DELETE.DIDEVENT.RETURN_VETO "
		L"			} "
		L"		} "
		L"	} "
		L"}";

	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc ROW_DELETE - Veto FAILEDTODO Listener 2
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows_Bufferred::Variation_12()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;" 
		L"	{Control Control1;"
		L"		{ "
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::DeleteRows(A,B) "
		L"			{ "
		L"			 L1.ROW_DELETE.ABOUTTODO.RETURN_VETO "
		L"			 L1.ROW_DELETE.FAILEDTODO.RETURN_VETO "
		L"			} "
		L"		} "
		L"	} "
		L"}";

	//To produce FAILEDTODO, this reason/phase must be vetoable...
	TESTC_PROVIDER(IsCancelableEvent(DBREASON_ROW_DELETE, DBEVENTPHASE_ABOUTTODO));
	TESTC(Execute(CommandString, CHANGE_BUFFERRED));

CLEANUP:
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows_Bufferred::Variation_13()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows_Bufferred::Variation_14()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc ROW_DELETE - UNWANTED REASON
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows_Bufferred::Variation_15()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;" 
		L"	{Control Control1;"
		L"		{ "
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::DeleteRows(A) "
		L"			{ "
		L"			 L1.ROW_DELETE.OKTODO.RETURN_UNWANTEDREASON "
		L"			} "
		L"		 IRowsetChange::DeleteRows(B) {} "
		L"		} "
		L"	} "
		L"}";

	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc ROW_DELETE - UNWANTED PHASE OKTODO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows_Bufferred::Variation_16()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;" 
		L"	{Control Control1;"
		L"		{ "
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::DeleteRows(A) "
		L"			{ "
		L"			 L1.ROW_DELETE.OKTODO.RETURN_UNWANTEDPHASE "
		L"			} "
		L"		 IRowsetChange::DeleteRows(B) {} "
		L"		} "
		L"	} "
		L"}";

	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc ROW_DELETE - UNWANTED PHASE ABOUTTODO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows_Bufferred::Variation_17()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;" 
		L"	{Control Control1;"
		L"		{ "
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::DeleteRows(A) "
		L"			{ "
		L"			 L1.ROW_DELETE.ABOUTTODO.RETURN_UNWANTEDPHASE "
		L"			} "
		L"		 IRowsetChange::DeleteRows(B) {} "
		L"		} "
		L"	} "
		L"}";

	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc ROW_DELETE - UNWANTED PHASE SYNCHAFTER
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows_Bufferred::Variation_18()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;" 
		L"	{Control Control1;"
		L"		{ "
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::DeleteRows(A) "
		L"			{ "
		L"			 L1.ROW_DELETE.SYNCHAFTER.RETURN_UNWANTEDPHASE "
		L"			} "
		L"		 IRowsetChange::DeleteRows(B) {} "
		L"		} "
		L"	} "
		L"}";

	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc ROW_DELETE - UNWANTED PHASE DIDEVENT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows_Bufferred::Variation_19()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;" 
		L"	{Control Control1;"
		L"		{ "
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::DeleteRows(A) "
		L"			{ "
		L"			 L1.ROW_DELETE.DIDEVENT.RETURN_UNWANTEDPHASE "
		L"			} "
		L"		 IRowsetChange::DeleteRows(B) {} "
		L"		} "
		L"	} "
		L"}";

	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc ROW_DELETE - UNWANTED PHASE FAILEDTODO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows_Bufferred::Variation_20()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;" 
		L"	{Control Control1;"
		L"		{ "
		L"		 IRowset::GetNextRows(A,B,C,D) {} "
		L"		 IRowsetChange::DeleteRows(A,B) "
		L"			{ "
		L"			 L1.ROW_DELETE.ABOUTTODO.RETURN_VETO "
		L"			 L1.ROW_DELETE.FAILEDTODO.RETURN_UNWANTEDPHASE "
		L"			} "
		L"		 IRowsetChange::DeleteRows(C,D) {} "
		L"		} "
		L"	} "
		L"}";

	//To produce FAILEDTODO, this reason/phase must be vetoable...
	TESTC_PROVIDER(IsCancelableEvent(DBREASON_ROW_DELETE, DBEVENTPHASE_ABOUTTODO));
	TESTC(Execute(CommandString, CHANGE_BUFFERRED));

CLEANUP:
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows_Bufferred::Variation_21()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(22)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows_Bufferred::Variation_22()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(23)
//*-----------------------------------------------------------------------
// @mfunc ROW_DELETE - E_FAIL - All Phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows_Bufferred::Variation_23()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{ "
		L"		 IRowset::GetNextRows(A,B,C,D) {} "
		L"		 IRowsetChange::DeleteRows(A,B) "
		L"			{ "
		L"			 L1.ROW_DELETE.OKTODO.RETURN_EFAIL "
		L"			 L2.ROW_DELETE.ABOUTTODO.RETURN_EFAIL "
		L"			 L3.ROW_DELETE.SYNCHAFTER.RETURN_EFAIL "
		L"			 L1.ROW_DELETE.DIDEVENT.RETURN_EFAIL "
		L"			} "
		L"		 IRowsetChange::DeleteRows(C,D) "
		L"			{ "
		L"			 L1.ROW_DELETE.ABOUTTODO.RETURN_VETO "
		L"			} "
		L"		} "
		L"	} "
		L"}";

	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(24)
//*-----------------------------------------------------------------------
// @mfunc ROW_DELETE - E_OUTOFMEMORY - All Phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows_Bufferred::Variation_24()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{ "
		L"		 IRowset::GetNextRows(A,B,C,D) {} "
		L"		 IRowsetChange::DeleteRows(A,B) "
		L"			{ "
		L"			 L1.ROW_DELETE.OKTODO.RETURN_EOUTOFMEMORY "
		L"			 L2.ROW_DELETE.ABOUTTODO.RETURN_EOUTOFMEMORY "
		L"			 L3.ROW_DELETE.SYNCHAFTER.RETURN_EOUTOFMEMORY "
		L"			 L1.ROW_DELETE.DIDEVENT.RETURN_EOUTOFMEMORY "
		L"			} "
		L"		 IRowsetChange::DeleteRows(C,D) "
		L"			{ "
		L"			 L1.ROW_DELETE.ABOUTTODO.RETURN_VETO "
		L"			} "
		L"		} "
		L"	} "
		L"}";

	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(25)
//*-----------------------------------------------------------------------
// @mfunc ROW_DELETE - E_INVALIDARG - All Phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows_Bufferred::Variation_25()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{ "
		L"		 IRowset::GetNextRows(A,B,C,D) {} "
		L"		 IRowsetChange::DeleteRows(A,B) "
		L"			{ "
		L"			 L1.ROW_DELETE.OKTODO.RETURN_EINVALIDARG "
		L"			 L2.ROW_DELETE.ABOUTTODO.RETURN_EINVALIDARG "
		L"			 L3.ROW_DELETE.SYNCHAFTER.RETURN_EINVALIDARG "
		L"			 L1.ROW_DELETE.DIDEVENT.RETURN_EINVALIDARG "
		L"			} "
		L"		 IRowsetChange::DeleteRows(C,D) "
		L"			{ "
		L"			 L1.ROW_DELETE.ABOUTTODO.RETURN_VETO "
		L"			} "
		L"		} "
		L"	} "
		L"}";

	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(26)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows_Bufferred::Variation_26()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(27)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows_Bufferred::Variation_27()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(28)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - Delete no rows
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows_Bufferred::Variation_28()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;" 
		L"	{Control Control1;"
		L"		{ "
		L"		 IRowset::GetNextRows(A,B,C,D) {} "
		L"		 IRowsetChange::DeleteRows() "
		L"			{ "
		L"			} "
		L"		 IRowsetChange::DeleteRows(C) "
		L"			{ "
		L"			 L2.ROW_DELETE.OKTODO.RETURN_ACCEPT "
		L"			} "
		L"		} "
		L"	} "
		L"}";

	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(29)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - DBROWSTATUS_DELETED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows_Bufferred::Variation_29()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;" 
		L"	{Control Control1;"
		L"		{ "
		L"		 IRowset::GetNextRows(A,B,C,D) {} "
		L"		 IRowsetChange::DeleteRows(C) "
		L"			{ "
		L"			 L2.ROW_DELETE.OKTODO.RETURN_ACCEPT "
		L"			} "
		L"		 IRowsetChange::DeleteRows(C,DB_E_ERRORSOCCURRED) "
		L"			{ "
		L"			} "
		L"		} "
		L"	} "
		L"}";

	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(30)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - EmptyRowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows_Bufferred::Variation_30()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;" 
		L"	{Control Control1;"
		L"		{ "
		L"		 IRowsetChange::DeleteRows() "
		L"			{ "
		L"			} "
		L"		 IRowsetChange::DeleteRows() "
		L"			{ "
		L"			} "
		L"		} "
		L"	} "
		L"}";

	return Execute(CommandString, EMPTY_ROWSET | CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(31)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - Add/Remove Listeners
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows_Bufferred::Variation_31()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{ "
		L"		 IRowset::GetNextRows(A,B,C,D) {} "
		L"		 IRowsetChange::DeleteRows(C) "
		L"			{ "
		L"			 L2.ROW_DELETE.OKTODO.RETURN_UNADVISE "
		L"			 L3.ROW_DELETE.SYNCHAFTER.RETURN_UNADVISE "
		L"			} "
		L"		 IRowsetChange::DeleteRows(D) "
		L"			{ "
		L"			 L1.ROW_DELETE.DIDEVENT.RETURN_UNADVISE "
		L"			} "
		L"		} "
		L"	} "
		L"}";

	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(32)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - GetData
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows_Bufferred::Variation_32()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{ "
		L"		 IRowset::GetNextRows(A,B,C,D) {} "
		L"		 IRowsetChange::DeleteRows(C) "
		L"			{ "
		L"			 L2.ROW_DELETE.OKTODO.IRowset::GetData(C) {} "
		L"			 L3.ROW_DELETE.ABOUTTODO.IRowset::GetData(C) {} "
		L"			} "
		L"		 IRowsetChange::DeleteRows(A) "
		L"			{ "
		L"			 L1.ROW_DELETE.ABOUTTODO.IRowset::GetData(D) {} "
		L"			} "
		L"		} "
		L"	} "
		L"}";

	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(33)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - GetOriginalData
//
// @rdesc TEST_PASS or TEST_FAIL
//
int DeleteRows_Bufferred::Variation_33()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{ "
		L"		 IRowset::GetNextRows(A,B,C,D) {} "
		L"		 IRowsetChange::DeleteRows(C) "
		L"			{ "
		L"			 L2.ROW_DELETE.OKTODO.IRowsetUpdate::GetOriginalData(C,DB_E_NOTREENTRANT) {} "
		L"			 L3.ROW_DELETE.SYNCHAFTER.IRowsetUpdate::GetOriginalData(C,DB_E_NOTREENTRANT) {} "
		L"			} "
		L"		 IRowsetChange::DeleteRows(A) "
		L"			{ "
		L"			 L1.ROW_DELETE.DIDEVENT.IRowsetUpdate::GetOriginalData(D,DB_E_NOTREENTRANT) {} "
		L"			} "
		L"		} "
		L"	} "
		L"}";

	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}
// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL DeleteRows_Bufferred::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CExecutionManager::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(InsertRow)
//*-----------------------------------------------------------------------
//| Test Case:		InsertRow - Tests Notifications for IRowsetChange::InsertRow
//|	Created:			09/29/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL InsertRow::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CExecutionManager::Init())
	// }}
	{
		//Verify Rowset can be created...
		TEST_PROVIDER(CreateRowset(CHANGE_IMMEDIATE)==S_OK);
		return TRUE;
	}

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc ROW_INSERT - Accept all phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int InsertRow::Variation_1()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;" 
		L"	{Control Control1;"
		L"		{"
		L"		 IRowsetChange::InsertRow(A) "
		L"			{"
		L"			 L1.ROW_INSERT.OKTODO.RETURN_ACCEPT "
		L"			}"
		L"		 IRowsetChange::InsertRow(B) {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, CHANGE_IMMEDIATE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int InsertRow::Variation_2()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int InsertRow::Variation_3()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc ROW_INSERT - DB_E_NOTREENTRANT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int InsertRow::Variation_4()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;" 
		L"	{Control Control1;"
		L"		{"
		L"		 IRowsetChange::InsertRow(A) "
		L"			{"
		L"			 L1.ROW_INSERT.OKTODO.IRowsetChange::InsertRow(C, DB_E_NOTREENTRANT) {} "
		L"			}"
		L"		 IRowsetChange::InsertRow(B) {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, CHANGE_IMMEDIATE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc ROW_INSERT - DB_E_CANCELED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int InsertRow::Variation_5()
{
	//Make sure the Veto of InsertRow actually occrrued.
	//Row handle should not be valid after delete
    
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;" 
		L"	{Control Control1;"
		L"		{"
		L"		 IRowsetChange::InsertRow(A) "
		L"			{"
		L"			 L1.ROW_INSERT.ABOUTTODO.RETURN_VETO "
		L"			}"
		L"		 IRowsetChange::SetData(A,2,DB_E_BADROWHANDLE) {} "
		L"		 IRowsetChange::InsertRow(B) {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, CHANGE_IMMEDIATE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int InsertRow::Variation_6()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int InsertRow::Variation_7()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc ROW_INSERT - Veto OKTODO, Listener 1
//
// @rdesc TEST_PASS or TEST_FAIL
//
int InsertRow::Variation_8()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;" 
		L"	{Control Control1;"
		L"		{"
		L"		 IRowsetChange::InsertRow(A) "
		L"			{"
		L"			 L1.ROW_INSERT.OKTODO.RETURN_VETO "
		L"			}"
		L"		 IRowsetChange::InsertRow(B) {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, CHANGE_IMMEDIATE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc ROW_INSERT - Veto ABOUTTODO, Listener 1
//
// @rdesc TEST_PASS or TEST_FAIL
//
int InsertRow::Variation_9()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;" 
		L"	{Control Control1;"
		L"		{"
		L"		 IRowsetChange::InsertRow(A) "
		L"			{"
		L"			 L1.ROW_INSERT.ABOUTTODO.RETURN_VETO "
		L"			}"
		L"		 IRowsetChange::InsertRow(B) {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, CHANGE_IMMEDIATE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc ROW_INSERT - Veto SYNCHAFTER, Listener 2
//
// @rdesc TEST_PASS or TEST_FAIL
//
int InsertRow::Variation_10()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;" 
		L"	{Control Control1;"
		L"		{"
		L"		 IRowsetChange::InsertRow(A) "
		L"			{"
		L"			 L2.ROW_INSERT.SYNCHAFTER.RETURN_VETO "
		L"			}"
		L"		 IRowsetChange::InsertRow(B) {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, CHANGE_IMMEDIATE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc ROW_INSERT - Veto DIDEVENT, Listener 3
//
// @rdesc TEST_PASS or TEST_FAIL
//
int InsertRow::Variation_11()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;" 
		L"	{Control Control1;"
		L"		{"
		L"		 IRowsetChange::InsertRow(A) "
		L"			{"
		L"			 L1.ROW_INSERT.DIDEVENT.RETURN_VETO "
		L"			}"
		L"		 IRowsetChange::InsertRow(B) {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, CHANGE_IMMEDIATE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc ROW_INSERT - Veto FAILEDTODO, Listener 3
//
// @rdesc TEST_PASS or TEST_FAIL
//
int InsertRow::Variation_12()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;" 
		L"	{Control Control1;"
		L"		{"
		L"		 IRowsetChange::InsertRow(A) "
		L"			{"
		L"			 L1.ROW_INSERT.ABOUTTODO.RETURN_VETO "
		L"			 L1.ROW_INSERT.FAILEDTODO.RETURN_VETO "
		L"			}"
		L"		 IRowsetChange::InsertRow(B) {} "
		L"		}"
		L"	}"
		L"}";

	//To produce FAILEDTODO, this reason/phase must be vetoable...
	TESTC_PROVIDER(IsCancelableEvent(DBREASON_ROW_INSERT, DBEVENTPHASE_ABOUTTODO));
	TESTC(Execute(CommandString, CHANGE_IMMEDIATE));

CLEANUP:
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int InsertRow::Variation_13()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc ROW_INSERT - UNWANTED REASON
//
// @rdesc TEST_PASS or TEST_FAIL
//
int InsertRow::Variation_14()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;" 
		L"	{Control Control1;"
		L"		{"
		L"		 IRowsetChange::InsertRow(A) "
		L"			{"
		L"			 L1.ROW_INSERT.OKTODO.RETURN_UNWANTEDREASON "
		L"			}"
		L"		 IRowsetChange::InsertRow(B) {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, CHANGE_IMMEDIATE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc ROW_INSERT - UNWANTED PHASE OKTODO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int InsertRow::Variation_15()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;" 
		L"	{Control Control1;"
		L"		{"
		L"		 IRowsetChange::InsertRow(A) "
		L"			{"
		L"			 L1.ROW_INSERT.OKTODO.RETURN_UNWANTEDPHASE "
		L"			}"
		L"		 IRowsetChange::InsertRow(B) {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, CHANGE_IMMEDIATE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc ROW_INSERT - UNWANTED PHASE ABOUTTODO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int InsertRow::Variation_16()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;" 
		L"	{Control Control1;"
		L"		{"
		L"		 IRowsetChange::InsertRow(A) "
		L"			{"
		L"			 L1.ROW_INSERT.ABOUTTODO.RETURN_UNWANTEDPHASE "
		L"			}"
		L"		 IRowsetChange::InsertRow(B) {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, CHANGE_IMMEDIATE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc ROW_INSERT - UNWANTED PHASE SYNCHAFTER
//
// @rdesc TEST_PASS or TEST_FAIL
//
int InsertRow::Variation_17()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;" 
		L"	{Control Control1;"
		L"		{"
		L"		 IRowsetChange::InsertRow(A) "
		L"			{"
		L"			 L1.ROW_INSERT.SYNCHAFTER.RETURN_UNWANTEDPHASE "
		L"			}"
		L"		 IRowsetChange::InsertRow(B) {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, CHANGE_IMMEDIATE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc ROW_INSERT - UNWANTED PHASE DIDEVENT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int InsertRow::Variation_18()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;" 
		L"	{Control Control1;"
		L"		{"
		L"		 IRowsetChange::InsertRow(A) "
		L"			{"
		L"			 L1.ROW_INSERT.DIDEVENT.RETURN_UNWANTEDPHASE "
		L"			}"
		L"		 IRowsetChange::InsertRow(B) {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, CHANGE_IMMEDIATE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc ROW_INSERT - UNWANTED PHASE FAILEDTODO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int InsertRow::Variation_19()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;" 
		L"	{Control Control1;"
		L"		{"
		L"		 IRowsetChange::InsertRow(A) "
		L"			{"
		L"			 L1.ROW_INSERT.ABOUTTODO.RETURN_VETO "
		L"			 L1.ROW_INSERT.FAILEDTODO.RETURN_UNWANTEDPHASE "
		L"			}"
		L"		 IRowsetChange::InsertRow(B) {} "
		L"		}"
		L"	}"
		L"}";

	//To produce FAILEDTODO, this reason/phase must be vetoable...
	TESTC_PROVIDER(IsCancelableEvent(DBREASON_ROW_INSERT, DBEVENTPHASE_ABOUTTODO));
	TESTC(Execute(CommandString, CHANGE_IMMEDIATE));

CLEANUP:
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int InsertRow::Variation_20()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int InsertRow::Variation_21()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(22)
//*-----------------------------------------------------------------------
// @mfunc ROW_INSERT - E_FAIL - All Phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int InsertRow::Variation_22()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;" 
		L"	{Control Control1;"
		L"		{"
		L"		 IRowsetChange::InsertRow(A) "
		L"			{"
		L"			 L1.ROW_INSERT.OKTODO.RETURN_EFAIL "
		L"			 L1.ROW_INSERT.ABOUTTODO.RETURN_EFAIL "
		L"			 L1.ROW_INSERT.SYNCHAFTER.RETURN_EFAIL "
		L"			 L1.ROW_INSERT.DIDEVENT.RETURN_EFAIL "
		L"			}"
		L"		 IRowsetChange::InsertRow(B) "
		L"			{"
		L"			 L1.ROW_INSERT.ABOUTTODO.RETURN_VETO "
		L"			}"
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, CHANGE_IMMEDIATE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(23)
//*-----------------------------------------------------------------------
// @mfunc ROW_INSERT - E_OUTOFMEMORY - All Phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int InsertRow::Variation_23()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;" 
		L"	{Control Control1;"
		L"		{"
		L"		 IRowsetChange::InsertRow(A) "
		L"			{"
		L"			 L1.ROW_INSERT.OKTODO.RETURN_EOUTOFMEMORY "
		L"			 L1.ROW_INSERT.ABOUTTODO.RETURN_EOUTOFMEMORY "
		L"			 L1.ROW_INSERT.SYNCHAFTER.RETURN_EOUTOFMEMORY "
		L"			 L1.ROW_INSERT.DIDEVENT.RETURN_EOUTOFMEMORY "
		L"			}"
		L"		 IRowsetChange::InsertRow(B) "
		L"			{"
		L"			 L1.ROW_INSERT.ABOUTTODO.RETURN_VETO "
		L"			}"
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, CHANGE_IMMEDIATE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(24)
//*-----------------------------------------------------------------------
// @mfunc ROW_INSERT - E_INVALIDARG - All Phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int InsertRow::Variation_24()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;" 
		L"	{Control Control1;"
		L"		{"
		L"		 IRowsetChange::InsertRow(A) "
		L"			{"
		L"			 L1.ROW_INSERT.OKTODO.RETURN_EINVALIDARG "
		L"			 L1.ROW_INSERT.ABOUTTODO.RETURN_EINVALIDARG "
		L"			 L1.ROW_INSERT.SYNCHAFTER.RETURN_EINVALIDARG "
		L"			 L1.ROW_INSERT.DIDEVENT.RETURN_EINVALIDARG "
		L"			}"
		L"		 IRowsetChange::InsertRow(B) "
		L"			{"
		L"			 L1.ROW_INSERT.ABOUTTODO.RETURN_VETO "
		L"			}"
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, CHANGE_IMMEDIATE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(25)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int InsertRow::Variation_25()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(26)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int InsertRow::Variation_26()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(27)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - DB_E_BADACCESSORHANDLE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int InsertRow::Variation_27()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;" 
		L"	{Control Control1;"
		L"		{"
		L"		 IRowsetChange::InsertRow(A,DB_E_BADACCESSORHANDLE) "
		L"			{"
		L"			}"
		L"		 IRowsetChange::InsertRow(B) "
		L"			{"
		L"			 L1.ROW_INSERT.OKTODO.RETURN_ACCEPT "
		L"			}"
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, CHANGE_IMMEDIATE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(28)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - EmptyRowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int InsertRow::Variation_28()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;" 
		L"	{Control Control1;"
		L"		{"
		L"		 IRowsetChange::InsertRow(A) "
		L"			{"
		L"			 L1.ROW_INSERT.OKTODO.RETURN_ACCEPT "
		L"			}"
		L"		 IRowsetChange::InsertRow(B) "
		L"			{"
		L"			 L1.ROW_INSERT.OKTODO.RETURN_ACCEPT "
		L"			}"
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, EMPTY_ROWSET | CHANGE_IMMEDIATE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(29)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - Add/Remove Listeners
//
// @rdesc TEST_PASS or TEST_FAIL
//
int InsertRow::Variation_29()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;" 
		L"	{Control Control1;"
		L"		{"
		L"		 IRowsetChange::InsertRow(A) "
		L"			{"
		L"			 L1.ROW_INSERT.OKTODO.RETURN_ADVISE "
		L"			}"
		L"		 IRowsetChange::InsertRow(B) "
		L"			{"
		L"			 L1.ROW_INSERT.OKTODO.RETURN_UNADVISE "
		L"			}"
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, CHANGE_IMMEDIATE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(30)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - GetData
//
// @rdesc TEST_PASS or TEST_FAIL
//
int InsertRow::Variation_30()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;" 
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A) {} "
		L"		 IRowsetChange::InsertRow(B) "
		L"			{"
		L"			 L1.ROW_INSERT.OKTODO.IRowset::GetData(A) {} "
		L"			}"
		L"		 IRowset::GetData(A) {} "
		L"		 IRowsetChange::InsertRow(C) "
		L"			{"
		L"			 L1.ROW_INSERT.OKTODO.IRowset::GetData(B) {} "
		L"			}"
		L"		 IRowset::GetData(B) {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, CHANGE_IMMEDIATE);
}
// }}
// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL InsertRow::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CExecutionManager::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(InsertRow_Bufferred)
//*-----------------------------------------------------------------------
//| Test Case:		InsertRow_Bufferred - Test Notifications for IRowsetChange::InsertRow in Buffered Mode
//|	Created:			09/29/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL InsertRow_Bufferred::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CExecutionManager::Init())
	// }}
	{
		//Verify Rowset can be created...
		TEST_PROVIDER(CreateRowset(CHANGE_BUFFERRED)==S_OK);
		return TRUE;
	}

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc ROW_INSERT - Accept all phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int InsertRow_Bufferred::Variation_1()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;" 
		L"	{Control Control1;"
		L"		{"
		L"		 IRowsetChange::InsertRow(A) "
		L"			{"
		L"			 L1.ROW_INSERT.OKTODO.RETURN_ACCEPT "
		L"			}"
		L"		 IRowsetChange::InsertRow(B) {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int InsertRow_Bufferred::Variation_2()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int InsertRow_Bufferred::Variation_3()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc ROW_INSERT - DB_E_NOTREENTRANT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int InsertRow_Bufferred::Variation_4()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;" 
		L"	{Control Control1;"
		L"		{"
		L"		 IRowsetChange::InsertRow(A) "
		L"			{"
		L"			 L1.ROW_INSERT.OKTODO.IRowsetChange::InsertRow(C, DB_E_NOTREENTRANT) {} "
		L"			}"
		L"		 IRowsetChange::InsertRow(B) {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc ROW_INSERT - DB_E_CANCELED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int InsertRow_Bufferred::Variation_5()
{
	//Need to make sure the Veto was successful
	//Row should not be valid after insert
    
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;" 
		L"	{Control Control1;"
		L"		{"
		L"		 IRowsetChange::InsertRow(A) "
		L"			{"
		L"			 L1.ROW_INSERT.ABOUTTODO.RETURN_VETO "
		L"			}"
		L"		 IRowsetChange::SetData(A,2,DB_E_BADROWHANDLE) {} "
		L"		 IRowsetChange::InsertRow(B) {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int InsertRow_Bufferred::Variation_6()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int InsertRow_Bufferred::Variation_7()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc ROW_INSERT - Veto OKTODO, Listener 1
//
// @rdesc TEST_PASS or TEST_FAIL
//
int InsertRow_Bufferred::Variation_8()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;" 
		L"	{Control Control1;"
		L"		{"
		L"		 IRowsetChange::InsertRow(A) "
		L"			{"
		L"			 L1.ROW_INSERT.OKTODO.RETURN_VETO "
		L"			}"
		L"		 IRowsetChange::InsertRow(B) {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc ROW_INSERT - Veto ABOUTTODO, Listener 1
//
// @rdesc TEST_PASS or TEST_FAIL
//
int InsertRow_Bufferred::Variation_9()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;" 
		L"	{Control Control1;"
		L"		{"
		L"		 IRowsetChange::InsertRow(A) "
		L"			{"
		L"			 L1.ROW_INSERT.ABOUTTODO.RETURN_VETO "
		L"			}"
		L"		 IRowsetChange::InsertRow(B) {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc ROW_INSERT - Veto SYNCHAFTER, Listener 2
//
// @rdesc TEST_PASS or TEST_FAIL
//
int InsertRow_Bufferred::Variation_10()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;" 
		L"	{Control Control1;"
		L"		{"
		L"		 IRowsetChange::InsertRow(A) "
		L"			{"
		L"			 L2.ROW_INSERT.SYNCHAFTER.RETURN_VETO "
		L"			}"
		L"		 IRowsetChange::InsertRow(B) {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc ROW_INSERT - Veto DIDEVENT, Listener 3
//
// @rdesc TEST_PASS or TEST_FAIL
//
int InsertRow_Bufferred::Variation_11()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;" 
		L"	{Control Control1;"
		L"		{"
		L"		 IRowsetChange::InsertRow(A) "
		L"			{"
		L"			 L1.ROW_INSERT.DIDEVENT.RETURN_VETO "
		L"			}"
		L"		 IRowsetChange::InsertRow(B) {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc ROW_INSERT - Veto FAILEDTODO, Listener 3
//
// @rdesc TEST_PASS or TEST_FAIL
//
int InsertRow_Bufferred::Variation_12()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;" 
		L"	{Control Control1;"
		L"		{"
		L"		 IRowsetChange::InsertRow(A) "
		L"			{"
		L"			 L1.ROW_INSERT.ABOUTTODO.RETURN_VETO "
		L"			 L1.ROW_INSERT.FAILEDTODO.RETURN_VETO "
		L"			}"
		L"		 IRowsetChange::InsertRow(B) {} "
		L"		}"
		L"	}"
		L"}";

	//To produce FAILEDTODO, this reason/phase must be vetoable...
	TESTC_PROVIDER(IsCancelableEvent(DBREASON_ROW_INSERT, DBEVENTPHASE_ABOUTTODO));
	TESTC(Execute(CommandString, CHANGE_BUFFERRED));

CLEANUP:
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int InsertRow_Bufferred::Variation_13()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc ROW_INSERT - UNWANTED REASON
//
// @rdesc TEST_PASS or TEST_FAIL
//
int InsertRow_Bufferred::Variation_14()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;" 
		L"	{Control Control1;"
		L"		{"
		L"		 IRowsetChange::InsertRow(A) "
		L"			{"
		L"			 L1.ROW_INSERT.OKTODO.RETURN_UNWANTEDREASON "
		L"			}"
		L"		 IRowsetChange::InsertRow(B) {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc ROW_INSERT - UNWANTED PHASE OKTODO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int InsertRow_Bufferred::Variation_15()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;" 
		L"	{Control Control1;"
		L"		{"
		L"		 IRowsetChange::InsertRow(A) "
		L"			{"
		L"			 L1.ROW_INSERT.OKTODO.RETURN_UNWANTEDPHASE "
		L"			}"
		L"		 IRowsetChange::InsertRow(B) {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc ROW_INSERT - UNWANTED PHASE ABOUTTODO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int InsertRow_Bufferred::Variation_16()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;" 
		L"	{Control Control1;"
		L"		{"
		L"		 IRowsetChange::InsertRow(A) "
		L"			{"
		L"			 L1.ROW_INSERT.ABOUTTODO.RETURN_UNWANTEDPHASE "
		L"			}"
		L"		 IRowsetChange::InsertRow(B) {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc ROW_INSERT - UNWANTED PHASE SYNCHAFTER
//
// @rdesc TEST_PASS or TEST_FAIL
//
int InsertRow_Bufferred::Variation_17()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;" 
		L"	{Control Control1;"
		L"		{"
		L"		 IRowsetChange::InsertRow(A) "
		L"			{"
		L"			 L1.ROW_INSERT.SYNCHAFTER.RETURN_UNWANTEDPHASE "
		L"			}"
		L"		 IRowsetChange::InsertRow(B) {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc ROW_INSERT - UNWANTED PHASE DIDEVENT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int InsertRow_Bufferred::Variation_18()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;" 
		L"	{Control Control1;"
		L"		{"
		L"		 IRowsetChange::InsertRow(A) "
		L"			{"
		L"			 L1.ROW_INSERT.DIDEVENT.RETURN_UNWANTEDPHASE "
		L"			}"
		L"		 IRowsetChange::InsertRow(B) {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc ROW_INSERT - UNWANTED PHASE FAILEDTODO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int InsertRow_Bufferred::Variation_19()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;" 
		L"	{Control Control1;"
		L"		{"
		L"		 IRowsetChange::InsertRow(A) "
		L"			{"
		L"			 L1.ROW_INSERT.ABOUTTODO.RETURN_VETO "
		L"			 L1.ROW_INSERT.FAILEDTODO.RETURN_UNWANTEDPHASE "
		L"			}"
		L"		 IRowsetChange::InsertRow(B) {} "
		L"		}"
		L"	}"
		L"}";

	//To produce FAILEDTODO, this reason/phase must be vetoable...
	TESTC_PROVIDER(IsCancelableEvent(DBREASON_ROW_INSERT, DBEVENTPHASE_ABOUTTODO));
	TESTC(Execute(CommandString, CHANGE_BUFFERRED));

CLEANUP:
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int InsertRow_Bufferred::Variation_20()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int InsertRow_Bufferred::Variation_21()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(22)
//*-----------------------------------------------------------------------
// @mfunc ROW_INSERT - E_FAIL - All Phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int InsertRow_Bufferred::Variation_22()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;" 
		L"	{Control Control1;"
		L"		{"
		L"		 IRowsetChange::InsertRow(A) "
		L"			{"
		L"			 L1.ROW_INSERT.OKTODO.RETURN_EFAIL "
		L"			 L1.ROW_INSERT.ABOUTTODO.RETURN_EFAIL "
		L"			 L1.ROW_INSERT.SYNCHAFTER.RETURN_EFAIL "
		L"			 L1.ROW_INSERT.DIDEVENT.RETURN_EFAIL "
		L"			}"
		L"		 IRowsetChange::InsertRow(B) "
		L"			{"
		L"			 L1.ROW_INSERT.ABOUTTODO.RETURN_VETO "
		L"			}"
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(23)
//*-----------------------------------------------------------------------
// @mfunc ROW_INSERT - E_OUTOFMEMORY - All Phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int InsertRow_Bufferred::Variation_23()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;" 
		L"	{Control Control1;"
		L"		{"
		L"		 IRowsetChange::InsertRow(A) "
		L"			{"
		L"			 L1.ROW_INSERT.OKTODO.RETURN_EOUTOFMEMORY "
		L"			 L1.ROW_INSERT.ABOUTTODO.RETURN_EOUTOFMEMORY "
		L"			 L1.ROW_INSERT.SYNCHAFTER.RETURN_EOUTOFMEMORY "
		L"			 L1.ROW_INSERT.DIDEVENT.RETURN_EOUTOFMEMORY "
		L"			}"
		L"		 IRowsetChange::InsertRow(B) "
		L"			{"
		L"			 L1.ROW_INSERT.ABOUTTODO.RETURN_VETO "
		L"			}"
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(24)
//*-----------------------------------------------------------------------
// @mfunc ROW_INSERT - E_INVALIDARG - All Phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int InsertRow_Bufferred::Variation_24()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;" 
		L"	{Control Control1;"
		L"		{"
		L"		 IRowsetChange::InsertRow(A) "
		L"			{"
		L"			 L1.ROW_INSERT.OKTODO.RETURN_EINVALIDARG "
		L"			 L1.ROW_INSERT.ABOUTTODO.RETURN_EINVALIDARG "
		L"			 L1.ROW_INSERT.SYNCHAFTER.RETURN_EINVALIDARG "
		L"			 L1.ROW_INSERT.DIDEVENT.RETURN_EINVALIDARG "
		L"			}"
		L"		 IRowsetChange::InsertRow(B) "
		L"			{"
		L"			 L1.ROW_INSERT.ABOUTTODO.RETURN_VETO "
		L"			}"
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(25)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int InsertRow_Bufferred::Variation_25()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(26)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int InsertRow_Bufferred::Variation_26()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(27)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - DB_E_BADACCESSORHANDLE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int InsertRow_Bufferred::Variation_27()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;" 
		L"	{Control Control1;"
		L"		{"
		L"		 IRowsetChange::InsertRow(A,DB_E_BADACCESSORHANDLE) "
		L"			{"
		L"			}"
		L"		 IRowsetChange::InsertRow(B) "
		L"			{"
		L"			 L1.ROW_INSERT.ABOUTTODO.RETURN_ACCEPT "
		L"			}"
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(28)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - EmptyRowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int InsertRow_Bufferred::Variation_28()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;" 
		L"	{Control Control1;"
		L"		{"
		L"		 IRowsetChange::InsertRow(A) "
		L"			{"
		L"			 L1.ROW_INSERT.OKTODO.RETURN_ACCEPT "
		L"			}"
		L"		 IRowsetChange::InsertRow(B) "
		L"			{"
		L"			 L1.ROW_INSERT.OKTODO.RETURN_ACCEPT "
		L"			}"
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, EMPTY_ROWSET | CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(29)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - Add/Remove Listeners
//
// @rdesc TEST_PASS or TEST_FAIL
//
int InsertRow_Bufferred::Variation_29()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowsetChange::InsertRow(A) "
		L"			{"
		L"			 L1.ROW_INSERT.OKTODO.RETURN_UNADVISE "
		L"			}"
		L"		 IRowsetChange::InsertRow(B) "
		L"			{"
		L"			 L2.ROW_INSERT.OKTODO.RETURN_ADVISE "
		L"			 L3.ROW_INSERT.OKTODO.RETURN_UNADVISE "
		L"			}"
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(30)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - GetData
//
// @rdesc TEST_PASS or TEST_FAIL
//
int InsertRow_Bufferred::Variation_30()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;" 
		L"	{Control Control1;"
		L"		{"
		L"		 IRowsetChange::InsertRow(A) {} "
		L"		 IRowsetChange::InsertRow(B) "
		L"			{"
		L"			 L1.ROW_INSERT.OKTODO.IRowset::GetData(A) {} "
		L"			}"
		L"		 IRowsetChange::InsertRow(C) "
		L"			{"
		L"			 L1.ROW_INSERT.OKTODO.IRowset::GetData(A) {} "
		L"			 L2.ROW_INSERT.OKTODO.IRowset::GetData(B) {} "
		L"			}"
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(31)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - GetOriginalData
//
// @rdesc TEST_PASS or TEST_FAIL
//
int InsertRow_Bufferred::Variation_31()
{
// From the spec:
//GetOriginalData() gets the data most recently fetched from or transmitted to the data store;
//does not get values based on pending changes => we can not call GetOriginalData for newly inserted row untill it's transmeted
//Commented out this variation

/*	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowsetChange::InsertRow(A) {} "
		L"		 IRowsetChange::InsertRow(B) "
		L"			{"
		L"			 L1.ROW_INSERT.OKTODO.IRowsetUpdate::GetOriginalData(A,DB_E_NOTREENTRANT) {} "
		L"			}"
		L"		 IRowsetChange::InsertRow(C) "
		L"			{"
		L"			 L1.ROW_INSERT.OKTODO.IRowsetUpdate::GetOriginalData(A,DB_E_NOTREENTRANT) {} "
		L"			 L2.ROW_INSERT.OKTODO.IRowsetUpdate::GetOriginalData(B,DB_E_NOTREENTRANT) {} "
		L"			 L3.ROW_INSERT.DIDEVENT.IRowsetUpdate::GetOriginalData(C,DB_E_NOTREENTRANT) {} "
		L"			}"
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, CHANGE_BUFFERRED);
*/
	return TEST_SKIPPED;
}
// }}
// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL InsertRow_Bufferred::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CExecutionManager::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(SetData)
//*-----------------------------------------------------------------------
//| Test Case:		SetData - Tests Notifications for IRowsetChange::SetData
//|	Created:			09/29/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL SetData::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CExecutionManager::Init())
	// }}
	{
		//Verify Rowset can be created...
		TEST_PROVIDER(CreateRowset(CHANGE_IMMEDIATE)==S_OK);
		return TRUE;
	}

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc COLUMN_SET - Accept all phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData::Variation_1()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2) {} " 
		L"		 IRowsetChange::SetData(A,2) {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, CHANGE_IMMEDIATE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData::Variation_2()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData::Variation_3()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc COLUMN_SET - DB_E_CANCELED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData::Variation_4()
{
	//Need to make sure the Veto was successful
	//RowData should not have been altered after veto, GetData will ensure this
    
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2) "
		L"			{ "
		L"			 L1.COLUMN_SET.ABOUTTODO.RETURN_VETO "
		L"			} " 
		L"		 IRowset::GetData(A) {} "	
		L"		 IRowsetChange::SetData(A,2) {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, CHANGE_IMMEDIATE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc COLUMN_SET - DB_E_NOTREENTRANT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData::Variation_5()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2) { L1.COLUMN_SET.OKTODO.IRowsetChange::SetData(A,2,DB_E_NOTREENTRANT) {} } " 
		L"		 IRowsetChange::SetData(A,2) {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, CHANGE_IMMEDIATE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData::Variation_6()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData::Variation_7()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc COLUMN_SET - Veto OKTODO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData::Variation_8()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2) { L1.COLUMN_SET.OKTODO.RETURN_VETO } " 
		L"		 IRowsetChange::SetData(A,2) {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, CHANGE_IMMEDIATE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc COLUMN_SET - Veto ABOUTTODO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData::Variation_9()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2) { L1.COLUMN_SET.ABOUTTODO.RETURN_VETO } " 
		L"		 IRowsetChange::SetData(A,2) {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, CHANGE_IMMEDIATE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc COLUMN_SET - Veto SYNCHAFTER
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData::Variation_10()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2) { L1.COLUMN_SET.SYNCHAFTER.RETURN_VETO } " 
		L"		 IRowsetChange::SetData(A,2) {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, CHANGE_IMMEDIATE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc COLUMN_SET - Veto DIDEVENT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData::Variation_11()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2) { L1.COLUMN_SET.DIDEVENT.RETURN_VETO } " 
		L"		 IRowsetChange::SetData(A,2) {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, CHANGE_IMMEDIATE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc COLUMN_SET - Veto FAILEDTODO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData::Variation_12()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2) "
		L"			{ "
		L"			 L1.COLUMN_SET.ABOUTTODO.RETURN_VETO "
		L"			 L1.COLUMN_SET.FAILEDTODO.RETURN_VETO "
		L"			} " 
		L"		 IRowsetChange::SetData(A,2) {} "
		L"		}"
		L"	}"
		L"}";

	//To produce FAILEDTODO, this reason/phase must be vetoable...
	TESTC_PROVIDER(IsCancelableEvent(DBREASON_COLUMN_SET, DBEVENTPHASE_ABOUTTODO));
	TESTC(Execute(CommandString, CHANGE_IMMEDIATE));

CLEANUP:
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData::Variation_13()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc COLUMN_SET - UNWANTED REASON
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData::Variation_14()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2) "
		L"			{ "
		L"			 L1.COLUMN_SET.OKTODO.RETURN_UNWANTEDREASON "
		L"			} " 
		L"		 IRowsetChange::SetData(A,2) {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, CHANGE_IMMEDIATE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc COLUMN_SET - UNWANTED PHASE OKTODO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData::Variation_15()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2) "
		L"			{ "
		L"			 L1.COLUMN_SET.OKTODO.RETURN_UNWANTEDPHASE "
		L"			} " 
		L"		 IRowsetChange::SetData(A,2) {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, CHANGE_IMMEDIATE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc COLUMN_SET - UNWANTED PHASE ABOUTTO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData::Variation_16()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2) "
		L"			{ "
		L"			 L1.COLUMN_SET.ABOUTTODO.RETURN_UNWANTEDPHASE "
		L"			} " 
		L"		 IRowsetChange::SetData(A,2) {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, CHANGE_IMMEDIATE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc COLUMN_SET - UNWANTED PHASE SYNCHAFTER
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData::Variation_17()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2) "
		L"			{ "
		L"			 L1.COLUMN_SET.SYNCHAFTER.RETURN_UNWANTEDPHASE "
		L"			} " 
		L"		 IRowsetChange::SetData(A,2) {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, CHANGE_IMMEDIATE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc COLUMN_SET- UNWANTED PHASE DIDEVENT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData::Variation_18()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2) "
		L"			{ "
		L"			 L1.COLUMN_SET.DIDEVENT.RETURN_UNWANTEDPHASE "
		L"			} " 
		L"		 IRowsetChange::SetData(A,2) {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, CHANGE_IMMEDIATE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc COLUMN_SET - UNWANTED PHASE FAILEDTODO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData::Variation_19()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2) "
		L"			{ "
		L"			 L1.COLUMN_SET.ABOUTTODO.RETURN_VETO "
		L"			 L1.COLUMN_SET.FAILEDTODO.RETURN_UNWANTEDPHASE "
		L"			} " 
		L"		 IRowsetChange::SetData(A,2) {} "
		L"		}"
		L"	}"
		L"}";

	//To produce FAILEDTODO, this reason/phase must be vetoable...
	TESTC_PROVIDER(IsCancelableEvent(DBREASON_COLUMN_SET, DBEVENTPHASE_ABOUTTODO));
	TESTC(Execute(CommandString, CHANGE_IMMEDIATE));

CLEANUP:
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData::Variation_20()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData::Variation_21()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(22)
//*-----------------------------------------------------------------------
// @mfunc COLUMN_SET - E_FAIL - All Phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData::Variation_22()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2) "
		L"			{ "
		L"			 L3.COLUMN_SET.OKTODO.RETURN_EFAIL "
		L"			 L2.COLUMN_SET.ABOUTTODO.RETURN_EFAIL "
		L"			 L1.COLUMN_SET.SYNCHAFTER.RETURN_EFAIL "
		L"			 L3.COLUMN_SET.DIDEVENT.RETURN_EFAIL "
		L"			} " 
		L"		 IRowsetChange::SetData(A,2) "
		L"			{ "
		L"			 L2.COLUMN_SET.ABOUTTODO.RETURN_VETO "
		L"			} " 
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, CHANGE_IMMEDIATE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(23)
//*-----------------------------------------------------------------------
// @mfunc COLUMN_SET - E_OUTOFMEMORY - All Phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData::Variation_23()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2) "
		L"			{ "
		L"			 L3.COLUMN_SET.OKTODO.RETURN_EOUTOFMEMORY "
		L"			 L2.COLUMN_SET.ABOUTTODO.RETURN_EOUTOFMEMORY "
		L"			 L1.COLUMN_SET.SYNCHAFTER.RETURN_EOUTOFMEMORY "
		L"			 L3.COLUMN_SET.DIDEVENT.RETURN_EOUTOFMEMORY "
		L"			} " 
		L"		 IRowsetChange::SetData(A,2) "
		L"			{ "
		L"			 L2.COLUMN_SET.ABOUTTODO.RETURN_VETO "
		L"			} " 
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, CHANGE_IMMEDIATE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(24)
//*-----------------------------------------------------------------------
// @mfunc COLUMN_SET - E_INVALIDARG - All Phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData::Variation_24()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2) "
		L"			{ "
		L"			 L3.COLUMN_SET.OKTODO.RETURN_EINVALIDARG "
		L"			 L2.COLUMN_SET.ABOUTTODO.RETURN_EINVALIDARG "
		L"			 L1.COLUMN_SET.SYNCHAFTER.RETURN_EINVALIDARG "
		L"			 L3.COLUMN_SET.DIDEVENT.RETURN_EINVALIDARG "
		L"			} " 
		L"		 IRowsetChange::SetData(A,2) "
		L"			{ "
		L"			 L2.COLUMN_SET.ABOUTTODO.RETURN_VETO "
		L"			} " 
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, CHANGE_IMMEDIATE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(25)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData::Variation_25()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(26)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData::Variation_26()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(27)
//*-----------------------------------------------------------------------
// @mfunc COLUMN_RECALCULATED - DB_E_ERRORSOCCURRED - Accept all phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData::Variation_27()
{
	//Try to change the Data for the computed column.
	//Should get DB_E_ERRORSOCCURRED with no notifications
	//We need the only computed column is been modified in SetData to be sure 
	//that modification of the computed column causes an error (and not other columns)
	//so use special mode COMPUTED_COLUMNS_INCLUDE (only one column will be included to m_rgUpBindings)

	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2,DB_E_ERRORSOCCURRED) "
		L"			{ "
		L"			} " 
		L"		 IRowsetChange::SetData(A,2,DB_E_ERRORSOCCURRED) "
		L"			{ "
		L"			} " 
		L"		}"
		L"	}"
		L"}";

	//COMPUTED_COLUMNS - makes the first column the input and 
	//the second column the computed column (col1-col2).
	return Execute(CommandString, COMPUTED_COLUMNS_INCLUDE | CHANGE_IMMEDIATE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(28)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData::Variation_28()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(29)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData::Variation_29()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(30)
//*-----------------------------------------------------------------------
// @mfunc COLUMN_RECALCULATED - DB_E_CANCELED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData::Variation_30()
{
	//Need to make sure the Veto was successful
	//RowData should not have been altered after veto, GetData will ensure this
    
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2) "
		L"			{ "
		L"			 L1.COLUMN_RECALCULATED.DIDEVENT.RETURN_VETO "
		L"			} " 
		L"		 IRowset::GetData(A) {} "
		L"		 IRowsetChange::SetData(A,2) "
		L"			{ "
		L"			} " 
		L"		}"
		L"	}"
		L"}";

	//COMPUTED_COLUMNS - makes the first column the input and 
	//the second column the computed column (col1-col2).
	return Execute(CommandString, COMPUTED_COLUMNS | CHANGE_IMMEDIATE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(31)
//*-----------------------------------------------------------------------
// @mfunc COLUMN_RECALCULATED - DB_E_NOTREENTRANT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData::Variation_31()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2) "
		L"			{ "
		L"			 L1.COLUMN_RECALCULATED.DIDEVENT.IRowsetChange::SetData(A,2,DB_E_NOTREENTRANT) {} "
		L"			} " 
		L"		 IRowsetChange::SetData(A,2) "
		L"			{ "
		L"			} " 
		L"		}"
		L"	}"
		L"}";

	//COMPUTED_COLUMNS - makes the first column the input and 
	//the second column the computed column (col1-col2).
	return Execute(CommandString, COMPUTED_COLUMNS | CHANGE_IMMEDIATE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(32)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData::Variation_32()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(33)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData::Variation_33()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(34)
//*-----------------------------------------------------------------------
// @mfunc COLUMN_RECALCULATED - Veto DIDEVENT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData::Variation_34()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2) "
		L"			{ "
		L"			 L1.COLUMN_RECALCULATED.DIDEVENT.RETURN_VETO "
		L"			} " 
		L"		 IRowsetChange::SetData(A,2) "
		L"			{ "
		L"			} " 
		L"		}"
		L"	}"
		L"}";

	//COMPUTED_COLUMNS - makes the first column the input and 
	//the second column the computed column (col1-col2).
	return Execute(CommandString, COMPUTED_COLUMNS | CHANGE_IMMEDIATE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(35)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData::Variation_35()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(36)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData::Variation_36()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(37)
//*-----------------------------------------------------------------------
// @mfunc COLUMN_RECALCULATED - UNWANTED REASON
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData::Variation_37()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2) "
		L"			{ "
		L"			 L1.COLUMN_RECALCULATED.DIDEVENT.RETURN_UNWANTEDREASON "
		L"			} " 
		L"		 IRowsetChange::SetData(A,2) "
		L"			{ "
		L"			} " 
		L"		}"
		L"	}"
		L"}";

	//COMPUTED_COLUMNS - makes the first column the input and 
	//the second column the computed column (col1-col2).
	return Execute(CommandString, COMPUTED_COLUMNS | CHANGE_IMMEDIATE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(38)
//*-----------------------------------------------------------------------
// @mfunc COLUMN_RECALCULATED - UNWANTED PHASE DIDEVENT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData::Variation_38()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2) "
		L"			{ "
		L"			 L1.COLUMN_RECALCULATED.DIDEVENT.RETURN_UNWANTEDPHASE "
		L"			} " 
		L"		 IRowsetChange::SetData(A,2) "
		L"			{ "
		L"			} " 
		L"		}"
		L"	}"
		L"}";

	//COMPUTED_COLUMNS - makes the first column the input and 
	//the second column the computed column (col1-col2).
	return Execute(CommandString, COMPUTED_COLUMNS | CHANGE_IMMEDIATE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(39)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData::Variation_39()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(40)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData::Variation_40()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(41)
//*-----------------------------------------------------------------------
// @mfunc COLUMN_RECALCULATED - E_FAIL - All Phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData::Variation_41()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2) "
		L"			{ "
		L"			 L1.COLUMN_RECALCULATED.DIDEVENT.RETURN_EFAIL "
		L"			} " 
		L"		 IRowsetChange::SetData(A,2) "
		L"			{ "
		L"			} " 
		L"		}"
		L"	}"
		L"}";

	//COMPUTED_COLUMNS - makes the first column the input and 
	//the second column the computed column (col1-col2).
	return Execute(CommandString, COMPUTED_COLUMNS | CHANGE_IMMEDIATE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(42)
//*-----------------------------------------------------------------------
// @mfunc COLUMN_RECALCULATED - E_OUTOFMEMORY - All Phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData::Variation_42()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2) "
		L"			{ "
		L"			 L1.COLUMN_RECALCULATED.DIDEVENT.RETURN_EOUTOFMEMORY "
		L"			} " 
		L"		 IRowsetChange::SetData(A,2) "
		L"			{ "
		L"			} " 
		L"		}"
		L"	}"
		L"}";

	//COMPUTED_COLUMNS - makes the first column the input and 
	//the second column the computed column (col1-col2).
	return Execute(CommandString, COMPUTED_COLUMNS | CHANGE_IMMEDIATE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(43)
//*-----------------------------------------------------------------------
// @mfunc COLUMN_RECALCULATED - E_INVALIDARG - All Phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData::Variation_43()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2) "
		L"			{ "
		L"			 L1.COLUMN_RECALCULATED.DIDEVENT.RETURN_EINVALIDARG "
		L"			} " 
		L"		 IRowsetChange::SetData(A,2) "
		L"			{ "
		L"			} " 
		L"		}"
		L"	}"
		L"}";

	//COMPUTED_COLUMNS - makes the first column the input and 
	//the second column the computed column (col1-col2).
	return Execute(CommandString, COMPUTED_COLUMNS | CHANGE_IMMEDIATE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(44)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData::Variation_44()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(45)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData::Variation_45()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(46)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - DB_E_BADACCESSORHANDLE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData::Variation_46()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2,DB_E_BADACCESSORHANDLE) "
		L"			{ "
		L"			} " 
		L"		 IRowsetChange::SetData(A,2) "
		L"			{ "
		L"			 L1.COLUMN_SET.DIDEVENT.RETURN_ACCEPT "
		L"			} " 
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, CHANGE_IMMEDIATE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(47)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - EmptyRowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData::Variation_47()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowsetChange::SetData(A,2,DB_E_BADROWHANDLE) "
		L"			{ "
		L"			} " 
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, EMPTY_ROWSET | CHANGE_IMMEDIATE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(48)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - Add/Remove Listeners
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData::Variation_48()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2) "
		L"			{ "
		L"			 L1.COLUMN_SET.ABOUTTODO.RETURN_ADVISE "
		L"			} " 
		L"		 IRowsetChange::SetData(A,2) "
		L"			{ "
		L"			 L1.COLUMN_SET.DIDEVENT.RETURN_UNADVISE "
		L"			} " 
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, CHANGE_IMMEDIATE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(49)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - GetData
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData::Variation_49()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2) "
		L"			{ "
		L"			 L1.COLUMN_SET.OKTODO.IRowset::GetData(A) {} "
		L"			} " 
		L"		 IRowsetChange::SetData(B,2) "
		L"			{ "
		L"			 L1.COLUMN_SET.OKTODO.IRowset::GetData(A) {} "
		L"			 L2.COLUMN_SET.ABOUTTODO.IRowset::GetData(B) {} "
		L"			} " 
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, CHANGE_IMMEDIATE);
}
// }}
// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL SetData::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CExecutionManager::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(SetData_Bufferred)
//*-----------------------------------------------------------------------
//| Test Case:		SetData_Bufferred - Test Notifications for IRowsetChange::SetData in Buffered Mode
//|	Created:			03/24/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL SetData_Bufferred::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CExecutionManager::Init())
	// }}
	{
		//Verify Rowset can be created...
		TEST_PROVIDER(CreateRowset(CHANGE_BUFFERRED)==S_OK);
		return TRUE;
	}

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc ROW_FIRSTCHANGE - Accept all phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData_Bufferred::Variation_1()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2) "
		L"			{ "
		L"			 L1.ROW_FIRSTCHANGE.OKTODO.RETURN_ACCEPT "
		L"			} " 
		L"		 IRowsetChange::SetData(A,2) {} "
		L"		 IRowsetUpdate::Update(A) {} "
		L"		 IRowsetChange::SetData(A,2) "
		L"			{ "
		L"			 L1.ROW_FIRSTCHANGE.OKTODO.RETURN_ACCEPT "
		L"			} " 
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc ROW_FIRSTCHANGE - InsertRow(A), SetData(A) - Accept all phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData_Bufferred::Variation_2()
{
 	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData_Bufferred::Variation_3()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc ROW_FIRSTCHANGE - DB_E_CANCELED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData_Bufferred::Variation_4()
{
	//Need to make sure the Veto was successful
	//FIRSTCHANGE should be fired again
    
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2) "
		L"			{ "
		L"			 L1.ROW_FIRSTCHANGE.ABOUTTODO.RETURN_VETO "
		L"			} " 
		L"		 IRowset::GetData(A) {} "
		L"		 IRowsetChange::SetData(A,2) "
		L"			{ "
		L"			 L1.ROW_FIRSTCHANGE.ABOUTTODO.RETURN_VETO "
		L"			} " 
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc ROW_FIRSTCHANGE - DB_E_NOTREENTRANT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData_Bufferred::Variation_5()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2) "
		L"			{ "
		L"			 L1.ROW_FIRSTCHANGE.OKTODO.IRowsetChange::SetData(A,2,DB_E_NOTREENTRANT) {} "
		L"			} " 
		L"		 IRowsetChange::SetData(A,2) {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData_Bufferred::Variation_6()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData_Bufferred::Variation_7()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc ROW_FIRSTCHANGE - Veto OKTODO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData_Bufferred::Variation_8()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2) "
		L"			{ "
		L"			 L1.ROW_FIRSTCHANGE.OKTODO.RETURN_VETO "
		L"			} " 
		L"		 IRowsetChange::SetData(A,2) {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc ROW_FIRSTCHANGE - Veto ABOUTTODO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData_Bufferred::Variation_9()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2) "
		L"			{ "
		L"			 L1.ROW_FIRSTCHANGE.ABOUTTODO.RETURN_VETO "
		L"			} " 
		L"		 IRowsetChange::SetData(A,2) {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc ROW_FIRSTCHANGE - Veto SYNCHAFTER
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData_Bufferred::Variation_10()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2) "
		L"			{ "
		L"			 L1.ROW_FIRSTCHANGE.SYNCHAFTER.RETURN_VETO "
		L"			} " 
		L"		 IRowsetChange::SetData(A,2) {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc ROW_FIRSTCHANGE - Veto DIDEVENT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData_Bufferred::Variation_11()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2) "
		L"			{ "
		L"			 L1.ROW_FIRSTCHANGE.DIDEVENT.RETURN_VETO "
		L"			} " 
		L"		 IRowsetChange::SetData(A,2) {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc ROW_FIRSTCHANGE - Veto FAILEDTODO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData_Bufferred::Variation_12()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2) "
		L"			{ "
		L"			 L1.ROW_FIRSTCHANGE.ABOUTTODO.RETURN_VETO "
		L"			 L1.ROW_FIRSTCHANGE.FAILEDTODO.RETURN_VETO "
		L"			} " 
		L"		 IRowsetChange::SetData(A,2) {} "
		L"		}"
		L"	}"
		L"}";

	//To produce FAILEDTODO, this reason/phase must be vetoable...
	TESTC_PROVIDER(IsCancelableEvent(DBREASON_ROW_FIRSTCHANGE, DBEVENTPHASE_ABOUTTODO));
	TESTC(Execute(CommandString, CHANGE_BUFFERRED));

CLEANUP:
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData_Bufferred::Variation_13()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData_Bufferred::Variation_14()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc ROW_FIRSTCHANGE - UNWANTED REASON
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData_Bufferred::Variation_15()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2) "
		L"			{ "
		L"			 L1.ROW_FIRSTCHANGE.OKTODO.RETURN_UNWANTEDREASON "
		L"			} " 
		L"		 IRowsetChange::SetData(B,2) {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc ROW_FIRSTCHANGE - UNWANTED PHASE OKTODO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData_Bufferred::Variation_16()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2) "
		L"			{ "
		L"			 L1.ROW_FIRSTCHANGE.OKTODO.RETURN_UNWANTEDPHASE "
		L"			} " 
		L"		 IRowsetChange::SetData(B,2) {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc ROW_FIRSTCHANGE - UNWANTED PHASE ABOUTTODO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData_Bufferred::Variation_17()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2) "
		L"			{ "
		L"			 L1.ROW_FIRSTCHANGE.ABOUTTODO.RETURN_UNWANTEDPHASE "
		L"			} " 
		L"		 IRowsetChange::SetData(B,2) {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc ROW_FIRSTCHANGE - UNWANTED PHASE SYNCHAFTER
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData_Bufferred::Variation_18()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2) "
		L"			{ "
		L"			 L1.ROW_FIRSTCHANGE.SYNCHAFTER.RETURN_UNWANTEDPHASE "
		L"			} " 
		L"		 IRowsetChange::SetData(B,2) {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc ROW_FIRSTCHANGE - UNWANTED PHASE DIDEVENT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData_Bufferred::Variation_19()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2) "
		L"			{ "
		L"			 L1.ROW_FIRSTCHANGE.DIDEVENT.RETURN_UNWANTEDPHASE "
		L"			} " 
		L"		 IRowsetChange::SetData(B,2) {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc ROW_FIRSTCHANGE - UNWANTED PHASE FAILEDTODO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData_Bufferred::Variation_20()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2) "
		L"			{ "
		L"			 L1.ROW_FIRSTCHANGE.ABOUTTODO.RETURN_VETO "
		L"			 L1.ROW_FIRSTCHANGE.FAILEDTODO.RETURN_UNWANTEDPHASE "
		L"			} " 
		L"		 IRowsetChange::SetData(B,2) {} "
		L"		}"
		L"	}"
		L"}";

	//To produce FAILEDTODO, this reason/phase must be vetoable...
	TESTC_PROVIDER(IsCancelableEvent(DBREASON_ROW_FIRSTCHANGE, DBEVENTPHASE_ABOUTTODO));
	TESTC(Execute(CommandString, CHANGE_BUFFERRED));

CLEANUP:
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData_Bufferred::Variation_21()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(22)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData_Bufferred::Variation_22()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(23)
//*-----------------------------------------------------------------------
// @mfunc ROW_FIRSTCHANGE - E_FAIL - All Phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData_Bufferred::Variation_23()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2) "
		L"			{ "
		L"			 L1.ROW_FIRSTCHANGE.OKTODO.RETURN_EFAIL "
		L"			 L2.ROW_FIRSTCHANGE.ABOUTTODO.RETURN_EFAIL "
		L"			 L3.ROW_FIRSTCHANGE.SYNCHAFTER.RETURN_EFAIL "
		L"			 L1.ROW_FIRSTCHANGE.DIDEVENT.RETURN_EFAIL "
		L"			} " 
		L"		 IRowsetChange::SetData(B,2) "
		L"			{ "
		L"			 L1.ROW_FIRSTCHANGE.ABOUTTODO.RETURN_VETO "
		L"			} " 
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(24)
//*-----------------------------------------------------------------------
// @mfunc ROW_FIRSTCHANGE - E_OUTOFMEMORY - All Phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData_Bufferred::Variation_24()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2) "
		L"			{ "
		L"			 L1.ROW_FIRSTCHANGE.OKTODO.RETURN_EOUTOFMEMORY "
		L"			 L2.ROW_FIRSTCHANGE.ABOUTTODO.RETURN_EOUTOFMEMORY "
		L"			 L3.ROW_FIRSTCHANGE.SYNCHAFTER.RETURN_EOUTOFMEMORY "
		L"			 L1.ROW_FIRSTCHANGE.DIDEVENT.RETURN_EOUTOFMEMORY "
		L"			} " 
		L"		 IRowsetChange::SetData(B,2) "
		L"			{ "
		L"			 L1.ROW_FIRSTCHANGE.ABOUTTODO.RETURN_VETO "
		L"			} " 
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(25)
//*-----------------------------------------------------------------------
// @mfunc ROW_FIRSTCHANGE - E_INVALIDARG - All Phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData_Bufferred::Variation_25()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2) "
		L"			{ "
		L"			 L1.ROW_FIRSTCHANGE.OKTODO.RETURN_EINVALIDARG "
		L"			 L2.ROW_FIRSTCHANGE.ABOUTTODO.RETURN_EINVALIDARG "
		L"			 L3.ROW_FIRSTCHANGE.SYNCHAFTER.RETURN_EINVALIDARG "
		L"			 L1.ROW_FIRSTCHANGE.DIDEVENT.RETURN_EINVALIDARG "
		L"			} " 
		L"		 IRowsetChange::SetData(B,2) "
		L"			{ "
		L"			 L1.ROW_FIRSTCHANGE.ABOUTTODO.RETURN_VETO "
		L"			} " 
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(26)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData_Bufferred::Variation_26()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(27)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData_Bufferred::Variation_27()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(28)
//*-----------------------------------------------------------------------
// @mfunc COLUMN_SET - Accept all phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData_Bufferred::Variation_28()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2) "
		L"			{ "
		L"			 L1.COLUMN_SET.OKTODO.RETURN_ACCEPT "
		L"			} " 
		L"		 IRowsetChange::SetData(A,2) "
		L"			{ "
		L"			 L1.COLUMN_SET.OKTODO.RETURN_ACCEPT "
		L"			} " 
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(29)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData_Bufferred::Variation_29()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(30)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData_Bufferred::Variation_30()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(31)
//*-----------------------------------------------------------------------
// @mfunc COLUMN_SET - DB_E_CANCELED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData_Bufferred::Variation_31()
{
	//Need to verify the Veto was successful
	//SetData should not have modified the row, GetData will verify this
    
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2) "
		L"			{ "
		L"			 L1.COLUMN_SET.ABOUTTODO.RETURN_VETO "
		L"			} " 
		L"		 IRowset::GetData(A) {} "
		L"		 IRowsetChange::SetData(A,2) {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(32)
//*-----------------------------------------------------------------------
// @mfunc COLUMN_SET - DB_E_NOTREENTRANT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData_Bufferred::Variation_32()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2) "
		L"			{ "
		L"			 L1.COLUMN_SET.OKTODO.IRowsetChange::SetData(A,2,DB_E_NOTREENTRANT) {} "
		L"			} " 
		L"		 IRowsetChange::SetData(A,2) {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(33)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData_Bufferred::Variation_33()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(34)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData_Bufferred::Variation_34()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(35)
//*-----------------------------------------------------------------------
// @mfunc COLUMN_SET - Veto OKTODO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData_Bufferred::Variation_35()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2) "
		L"			{ "
		L"			 L1.COLUMN_SET.OKTODO.RETURN_VETO "
		L"			} " 
		L"		 IRowsetChange::SetData(A,2) {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(36)
//*-----------------------------------------------------------------------
// @mfunc COLUMN_SET - Veto ABOUTTODO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData_Bufferred::Variation_36()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2) "
		L"			{ "
		L"			 L1.COLUMN_SET.ABOUTTODO.RETURN_VETO "
		L"			} " 
		L"		 IRowsetChange::SetData(A,2) {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(37)
//*-----------------------------------------------------------------------
// @mfunc COLUMN_SET - Veto SYNCHAFTER
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData_Bufferred::Variation_37()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2) "
		L"			{ "
		L"			 L1.COLUMN_SET.SYNCHAFTER.RETURN_VETO "
		L"			} " 
		L"		 IRowsetChange::SetData(A,2) {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(38)
//*-----------------------------------------------------------------------
// @mfunc COLUMN_SET - Veto DIDEVENT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData_Bufferred::Variation_38()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2) "
		L"			{ "
		L"			 L1.COLUMN_SET.DIDEVENT.RETURN_VETO "
		L"			} " 
		L"		 IRowsetChange::SetData(A,2) {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(39)
//*-----------------------------------------------------------------------
// @mfunc COLUMN_SET - Veto FAILEDTODO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData_Bufferred::Variation_39()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2) "
		L"			{ "
		L"			 L1.COLUMN_SET.ABOUTTODO.RETURN_VETO "
		L"			 L1.COLUMN_SET.FAILEDTODO.RETURN_VETO "
		L"			} " 
		L"		 IRowsetChange::SetData(A,2) {} "
		L"		}"
		L"	}"
		L"}";

	//To produce FAILEDTODO, this reason/phase must be vetoable...
	TESTC_PROVIDER(IsCancelableEvent(DBREASON_ROW_FIRSTCHANGE, DBEVENTPHASE_ABOUTTODO));
	TESTC(Execute(CommandString, CHANGE_BUFFERRED));

CLEANUP:
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(40)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData_Bufferred::Variation_40()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(41)
//*-----------------------------------------------------------------------
// @mfunc COLUMN_SET - UNWANTED REASON
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData_Bufferred::Variation_41()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2) "
		L"			{ "
		L"			 L1.COLUMN_SET.OKTODO.RETURN_UNWANTEDREASON "
		L"			} " 
		L"		 IRowsetChange::SetData(A,2) {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(42)
//*-----------------------------------------------------------------------
// @mfunc COLUMN_SET - UNWANTED PHASE OKTODO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData_Bufferred::Variation_42()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2) "
		L"			{ "
		L"			 L1.COLUMN_SET.OKTODO.RETURN_UNWANTEDPHASE "
		L"			} " 
		L"		 IRowsetChange::SetData(A,2) {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(43)
//*-----------------------------------------------------------------------
// @mfunc COLUMN_SET - UNWANTED PHASE ABOUTTO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData_Bufferred::Variation_43()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2) "
		L"			{ "
		L"			 L1.COLUMN_SET.ABOUTTODO.RETURN_UNWANTEDPHASE "
		L"			} " 
		L"		 IRowsetChange::SetData(A,2) {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(44)
//*-----------------------------------------------------------------------
// @mfunc COLUMN_SET - UNWANTED PHASE SYNCHAFTER
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData_Bufferred::Variation_44()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2) "
		L"			{ "
		L"			 L1.COLUMN_SET.SYNCHAFTER.RETURN_UNWANTEDPHASE "
		L"			} " 
		L"		 IRowsetChange::SetData(A,2) {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(45)
//*-----------------------------------------------------------------------
// @mfunc COLUMN_SET- UNWANTED PHASE DIDEVENT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData_Bufferred::Variation_45()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2) "
		L"			{ "
		L"			 L1.COLUMN_SET.DIDEVENT.RETURN_UNWANTEDPHASE "
		L"			} " 
		L"		 IRowsetChange::SetData(A,2) {} "
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(46)
//*-----------------------------------------------------------------------
// @mfunc COLUMN_SET - UNWANTED PHASE FAILEDTODO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData_Bufferred::Variation_46()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2) "
		L"			{ "
		L"			 L1.COLUMN_SET.ABOUTTODO.RETURN_VETO "
		L"			 L1.COLUMN_SET.FAILEDTODO.RETURN_UNWANTEDPHASE "
		L"			} " 
		L"		 IRowsetChange::SetData(A,2) {} "
		L"		}"
		L"	}"
		L"}";

	//To produce FAILEDTODO, this reason/phase must be vetoable...
	TESTC_PROVIDER(IsCancelableEvent(DBREASON_ROW_FIRSTCHANGE, DBEVENTPHASE_ABOUTTODO));
	TESTC(Execute(CommandString, CHANGE_BUFFERRED));

CLEANUP:
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(47)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData_Bufferred::Variation_47()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(48)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData_Bufferred::Variation_48()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(49)
//*-----------------------------------------------------------------------
// @mfunc COLUMN_SET - E_FAIL - All Phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData_Bufferred::Variation_49()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2) "
		L"			{ "
		L"			 L1.COLUMN_SET.OKTODO.RETURN_EFAIL "
		L"			 L2.COLUMN_SET.ABOUTTODO.RETURN_EFAIL "
		L"			 L3.COLUMN_SET.SYNCHAFTER.RETURN_EFAIL "
		L"			 L1.COLUMN_SET.DIDEVENT.RETURN_EFAIL "
		L"			} " 
		L"		 IRowsetChange::SetData(A,2) "
		L"			{ "
		L"			 L1.COLUMN_SET.ABOUTTODO.RETURN_VETO "
		L"			} " 
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(50)
//*-----------------------------------------------------------------------
// @mfunc COLUMN_SET - E_OUTOFMEMORY - All Phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData_Bufferred::Variation_50()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2) "
		L"			{ "
		L"			 L1.COLUMN_SET.OKTODO.RETURN_EOUTOFMEMORY "
		L"			 L2.COLUMN_SET.ABOUTTODO.RETURN_EOUTOFMEMORY "
		L"			 L3.COLUMN_SET.SYNCHAFTER.RETURN_EOUTOFMEMORY "
		L"			 L1.COLUMN_SET.DIDEVENT.RETURN_EOUTOFMEMORY "
		L"			} " 
		L"		 IRowsetChange::SetData(A,2) "
		L"			{ "
		L"			 L1.COLUMN_SET.ABOUTTODO.RETURN_VETO "
		L"			} " 
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(51)
//*-----------------------------------------------------------------------
// @mfunc COLUMN_SET - E_INVALIDARG - All Phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData_Bufferred::Variation_51()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2) "
		L"			{ "
		L"			 L1.COLUMN_SET.OKTODO.RETURN_EINVALIDARG "
		L"			 L2.COLUMN_SET.ABOUTTODO.RETURN_EINVALIDARG "
		L"			 L3.COLUMN_SET.SYNCHAFTER.RETURN_EINVALIDARG "
		L"			 L1.COLUMN_SET.DIDEVENT.RETURN_EINVALIDARG "
		L"			} " 
		L"		 IRowsetChange::SetData(A,2) "
		L"			{ "
		L"			 L1.COLUMN_SET.ABOUTTODO.RETURN_VETO "
		L"			} " 
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(52)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData_Bufferred::Variation_52()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(53)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData_Bufferred::Variation_53()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(54)
//*-----------------------------------------------------------------------
// @mfunc COLUMN_RECALCULATED - Accept all phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData_Bufferred::Variation_54()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2) {} "
		L"		 IRowsetUpdate::Update(A) "
		L"			{ "
		L"			 L1.COLUMN_RECALCULATED.DIDEVENT.RETURN_ACCEPT "
		L"			} " 
		L"		 IRowsetChange::SetData(A,2) "
		L"			{ "
		L"			} " 
		L"		}"
		L"	}"
		L"}";

	//COMPUTED_COLUMNS - makes the first column the input and 
	//the second column the computed column (col1-col2).
	return Execute(CommandString, COMPUTED_COLUMNS | CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(55)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData_Bufferred::Variation_55()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(56)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData_Bufferred::Variation_56()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(57)
//*-----------------------------------------------------------------------
// @mfunc COLUMN_RECALCULATED - DB_E_CANCELED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData_Bufferred::Variation_57()
{
	//Need to make sure the Veto was successful
	//RowData should not have been altered after veto, GetOriginalData will ensure this
    
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2) {} "
		L"		 IRowsetUpdate::Update(A) "
		L"			{ "
		L"			 L1.COLUMN_RECALCULATED.DIDEVENT.RETURN_VETO "
		L"			} " 
		L"		 IRowsetUpdate::GetOriginalData(A) {} "
		L"		 IRowsetChange::SetData(A,2) "
		L"			{ "
		L"			} " 
		L"		}"
		L"	}"
		L"}";

	//COMPUTED_COLUMNS - makes the first column the input and 
	//the second column the computed column (col1-col2).
	return Execute(CommandString, COMPUTED_COLUMNS | CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(58)
//*-----------------------------------------------------------------------
// @mfunc COLUMN_RECALCULATED - DB_E_NOTREENTRANT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData_Bufferred::Variation_58()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2) {} "
		L"		 IRowsetUpdate::Update(A) "
		L"			{ "
		L"			 L1.COLUMN_RECALCULATED.DIDEVENT.IRowsetUpdate::Update(A) {} "
		L"			} " 
		L"		 IRowsetChange::SetData(A,2) "
		L"			{ "
		L"			} " 
		L"		}"
		L"	}"
		L"}";

	//COMPUTED_COLUMNS - makes the first column the input and 
	//the second column the computed column (col1-col2).
	return Execute(CommandString, COMPUTED_COLUMNS | CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(59)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData_Bufferred::Variation_59()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(60)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData_Bufferred::Variation_60()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(61)
//*-----------------------------------------------------------------------
// @mfunc COLUMN_RECALCULATED - Veto DIDEVENT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData_Bufferred::Variation_61()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2) {} "
		L"		 IRowsetUpdate::Update(A) "
		L"			{ "
		L"			 L1.COLUMN_RECALCULATED.DIDEVENT.RETURN_VETO "
		L"			} " 
		L"		 IRowsetChange::SetData(A,2) "
		L"			{ "
		L"			} " 
		L"		}"
		L"	}"
		L"}";

	//COMPUTED_COLUMNS - makes the first column the input and 
	//the second column the computed column (col1-col2).
	return Execute(CommandString, COMPUTED_COLUMNS | CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(62)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData_Bufferred::Variation_62()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(63)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData_Bufferred::Variation_63()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(64)
//*-----------------------------------------------------------------------
// @mfunc COLUMN_RECALCULATED - UNWANTED REASON
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData_Bufferred::Variation_64()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2) {} "
		L"		 IRowsetUpdate::Update(A) "
		L"			{ "
		L"			 L1.COLUMN_RECALCULATED.DIDEVENT.RETURN_UNWANTEDREASON "
		L"			} " 
		L"		 IRowsetChange::SetData(A,2) {} "
		L"		 IRowsetUpdate::Update(A) {} "
		L"		}"
		L"	}"
		L"}";

	//COMPUTED_COLUMNS - makes the first column the input and 
	//the second column the computed column (col1-col2).
	return Execute(CommandString, COMPUTED_COLUMNS | CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(65)
//*-----------------------------------------------------------------------
// @mfunc COLUMN_RECALCULATED - UNWANTED PHASE DIDEVENT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData_Bufferred::Variation_65()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2) {} "
		L"		 IRowsetUpdate::Update(A) "
		L"			{ "
		L"			 L1.COLUMN_RECALCULATED.DIDEVENT.RETURN_UNWANTEDPHASE "
		L"			} " 
		L"		 IRowsetChange::SetData(A,2) {} "
		L"		 IRowsetUpdate::Update(A) {} "
		L"		}"
		L"	}"
		L"}";

	//COMPUTED_COLUMNS - makes the first column the input and 
	//the second column the computed column (col1-col2).
	return Execute(CommandString, COMPUTED_COLUMNS | CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(66)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData_Bufferred::Variation_66()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(67)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData_Bufferred::Variation_67()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(68)
//*-----------------------------------------------------------------------
// @mfunc COLUMN_RECALCULATED - E_FAIL - All Phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData_Bufferred::Variation_68()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2) {} "
		L"		 IRowsetUpdate::Update(A) "
		L"			{ "
		L"			 L1.COLUMN_RECALCULATED.DIDEVENT.RETURN_EFAIL "
		L"			} " 
		L"		 IRowsetChange::SetData(A,2) {} "
		L"		 IRowsetUpdate::Update(A) "
		L"			{ "
		L"			 L1.COLUMN_RECALCULATED.DIDEVENT.RETURN_ACCEPT "
		L"			} " 
		L"		}"
		L"	}"
		L"}";

	//COMPUTED_COLUMNS - makes the first column the input and 
	//the second column the computed column (col1-col2).
	return Execute(CommandString, COMPUTED_COLUMNS | CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(69)
//*-----------------------------------------------------------------------
// @mfunc COLUMN_RECALCULATED - E_OUTOFMEMORY - All Phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData_Bufferred::Variation_69()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2) {} "
		L"		 IRowsetUpdate::Update(A) "
		L"			{ "
		L"			 L1.COLUMN_RECALCULATED.DIDEVENT.RETURN_EOUTOFMEMORY "
		L"			} " 
		L"		 IRowsetChange::SetData(A,2) {} "
		L"		 IRowsetUpdate::Update(A) "
		L"			{ "
		L"			 L1.COLUMN_RECALCULATED.DIDEVENT.RETURN_ACCEPT "
		L"			} " 
		L"		}"
		L"	}"
		L"}";

	//COMPUTED_COLUMNS - makes the first column the input and 
	//the second column the computed column (col1-col2).
	return Execute(CommandString, COMPUTED_COLUMNS | CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(70)
//*-----------------------------------------------------------------------
// @mfunc COLUMN_RECALCULATED - E_INVALIDARG - All Phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData_Bufferred::Variation_70()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2) {} "
		L"		 IRowsetUpdate::Update(A) "
		L"			{ "
		L"			 L1.COLUMN_RECALCULATED.DIDEVENT.RETURN_EINVALIDARG "
		L"			} " 
		L"		 IRowsetChange::SetData(A,2) {} "
		L"		 IRowsetUpdate::Update(A) "
		L"			{ "
		L"			 L1.COLUMN_RECALCULATED.DIDEVENT.RETURN_ACCEPT "
		L"			} " 
		L"		}"
		L"	}"
		L"}";

	//COMPUTED_COLUMNS - makes the first column the input and 
	//the second column the computed column (col1-col2).
	return Execute(CommandString, COMPUTED_COLUMNS | CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(71)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData_Bufferred::Variation_71()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(72)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData_Bufferred::Variation_72()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(73)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - DB_E_BADACCESSORHANDLE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData_Bufferred::Variation_73()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2,DB_E_BADACCESSORHANDLE) "
		L"			{ "
		L"			} " 
		L"		 IRowsetChange::SetData(A,2) "
		L"			{ "
		L"			 L1.COLUMN_SET.OKTODO.RETURN_ACCEPT "
		L"			} " 
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(74)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - EmptyRowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData_Bufferred::Variation_74()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowsetChange::SetData(A,2,DB_E_BADROWHANDLE) "
		L"			{ "
		L"			} " 
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, EMPTY_ROWSET | CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(75)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - Add/Remove Listeners
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData_Bufferred::Variation_75()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2) "
		L"			{ "
		L"			 L1.COLUMN_SET.OKTODO.RETURN_UNADVISE "
		L"			} " 
		L"		 IRowsetChange::SetData(A,2) "
		L"			{ "
		L"			 L1.COLUMN_SET.OKTODO.RETURN_ADVISE "
		L"			} " 
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(76)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - GetData
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData_Bufferred::Variation_76()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2) "
		L"			{ "
		L"			 L1.COLUMN_SET.OKTODO.IRowset::GetData(A) {} "
		L"			} " 
		L"		 IRowsetChange::SetData(A,2) "
		L"			{ "
		L"			 L1.COLUMN_SET.OKTODO.IRowset::GetData(A) {} "
		L"			 L2.COLUMN_SET.ABOUTTODO.IRowset::GetData(B) {} "
		L"			} " 
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(77)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - GetOriginalData
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SetData_Bufferred::Variation_77()
{
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2) "
		L"			{ "
		L"			 L1.COLUMN_SET.OKTODO.IRowsetUpdate::GetOriginalData(A,DB_E_NOTREENTRANT) {} "
		L"			} " 
		L"		 IRowsetChange::SetData(A,2) "
		L"			{ "
		L"			 L1.COLUMN_SET.OKTODO.IRowsetUpdate::GetOriginalData(A,DB_E_NOTREENTRANT) {} "
		L"			 L2.COLUMN_SET.ABOUTTODO.IRowsetUpdate::GetOriginalData(B,DB_E_NOTREENTRANT) {} "
		L"			} " 
		L"		}"
		L"	}"
		L"}";

	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}
// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL SetData_Bufferred::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CExecutionManager::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(Resynch)
//*-----------------------------------------------------------------------
//| Test Case:		Resynch - Tests Notifications for IRowsetResynch::ResyncRows
//|	Created:			09/29/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Resynch::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CExecutionManager::Init())
	// }}
	{
		//Verify Rowset can be created...
		TEST_PROVIDER(CreateRowset(RESYNCH_ROWSET | CHANGE_IMMEDIATE)==S_OK);
		return TRUE;
	}

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc ROW_RESYNCH - Accept all phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Resynch::Variation_1()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B,C) {} "
		L"		 IRowsetChange::SetData(A,2,5) {} "
		L"		 IRowsetChange::SetData(B,2,5) {} "
		L"		 IRowsetChange::SetData(C,2,5) {} "
		L"		 IRowsetResynch::ResynchRows(A,B) "
		L"			{ "
		L"				L1.ROW_RESYNCH.OKTODO.RETURN_ACCEPT "
		L"			} "
		L"		 IRowsetResynch::ResynchRows(A,B,C) {} "
		L"		}"
		L"	}"
		L"}"; 
	
	return Execute(CommandString, RESYNCH_ROWSET | CHANGE_IMMEDIATE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Resynch::Variation_2()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Resynch::Variation_3()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc ROW_RESYNCH - DB_E_CANCELED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Resynch::Variation_4()
{
	//Need to verify the Veto was successful
	//Verify rows are unchanged by resynch
    
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2,5) {} "
		L"		 IRowsetChange::SetData(B,2,5) {} "
		L"		 IRowsetResynch::ResynchRows(A,B) "
		L"			{ "
		L"				L1.ROW_RESYNCH.ABOUTTODO.RETURN_VETO "
		L"			} "
		L"		 IRowset::GetData(A) {} "
		L"		 IRowsetResynch::ResynchRows(A,B) {} "
		L"		}"
		L"	}"
		L"}"; 
	
	return Execute(CommandString, RESYNCH_ROWSET | CHANGE_IMMEDIATE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc ROW_RESYNCH - DB_E_NOTREENTRANT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Resynch::Variation_5()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2,5) {} "
		L"		 IRowsetChange::SetData(B,2,5) {} "
		L"		 IRowsetResynch::ResynchRows(A,B) "
		L"			{ "
		L"				L1.ROW_RESYNCH.OKTODO.IRowsetResynch::ResynchRows(A,B,DB_E_NOTREENTRANT) {} "
		L"			} "
		L"		 IRowsetResynch::ResynchRows(A,B) {} "
		L"		}"
		L"	}"
		L"}"; 
	
	return Execute(CommandString, RESYNCH_ROWSET | CHANGE_IMMEDIATE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Resynch::Variation_6()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Resynch::Variation_7()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc ROW_RESYNCH - Veto OKTODO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Resynch::Variation_8()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2,5) {} "
		L"		 IRowsetChange::SetData(B,2,5) {} "
		L"		 IRowsetResynch::ResynchRows(A,B) "
		L"			{ "
		L"				L1.ROW_RESYNCH.OKTODO.RETURN_VETO "
		L"			} "
		L"		 IRowsetResynch::ResynchRows(A,B) {} "
		L"		}"
		L"	}"
		L"}"; 
	
	return Execute(CommandString, RESYNCH_ROWSET | CHANGE_IMMEDIATE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc ROW_RESYNCH - Veto ABOUTTODO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Resynch::Variation_9()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2,5) {} "
		L"		 IRowsetChange::SetData(B,2,5) {} "
		L"		 IRowsetResynch::ResynchRows(A,B) "
		L"			{ "
		L"				L1.ROW_RESYNCH.ABOUTTODO.RETURN_VETO "
		L"			} "
		L"		 IRowsetResynch::ResynchRows(A,B) {} "
		L"		}"
		L"	}"
		L"}"; 
	
	return Execute(CommandString, RESYNCH_ROWSET | CHANGE_IMMEDIATE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc ROW_RESYNCH - Veto SYNCHAFTER
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Resynch::Variation_10()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2,5) {} "
		L"		 IRowsetChange::SetData(B,2,5) {} "
		L"		 IRowsetResynch::ResynchRows(A,B) "
		L"			{ "
		L"				L1.ROW_RESYNCH.SYNCHAFTER.RETURN_VETO "
		L"			} "
		L"		 IRowsetResynch::ResynchRows(A,B) {} "
		L"		}"
		L"	}"
		L"}"; 
	
	return Execute(CommandString, RESYNCH_ROWSET | CHANGE_IMMEDIATE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc ROW_RESYNCH - Veto DIDEVENT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Resynch::Variation_11()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2,5) {} "
		L"		 IRowsetChange::SetData(B,2,5) {} "
		L"		 IRowsetResynch::ResynchRows(A,B) "
		L"			{ "
		L"				L1.ROW_RESYNCH.DIDEVENT.RETURN_VETO "
		L"			} "
		L"		 IRowsetResynch::ResynchRows(A,B) {} "
		L"		}"
		L"	}"
		L"}"; 
	
	return Execute(CommandString, RESYNCH_ROWSET | CHANGE_IMMEDIATE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc ROW_RESYNCH - Veto FAILEDTODO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Resynch::Variation_12()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2,5) {} "
		L"		 IRowsetChange::SetData(B,2,5) {} "
		L"		 IRowsetResynch::ResynchRows(A,B) "
		L"			{ "
		L"				L1.ROW_RESYNCH.ABOUTTODO.RETURN_VETO "
		L"				L1.ROW_RESYNCH.FAILEDTODO.RETURN_VETO "
		L"			} "
		L"		 IRowsetResynch::ResynchRows(A,B) {} "
		L"		}"
		L"	}"
		L"}"; 
	
	//To produce FAILEDTODO, this reason/phase must be vetoable...
	TESTC_PROVIDER(IsCancelableEvent(DBREASON_ROW_RESYNCH, DBEVENTPHASE_ABOUTTODO));
	TESTC(Execute(CommandString, RESYNCH_ROWSET | CHANGE_IMMEDIATE));

CLEANUP:
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Resynch::Variation_13()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Resynch::Variation_14()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc ROW_RESYNCH - UNWANTED REASON
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Resynch::Variation_15()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2,5) {} "
		L"		 IRowsetChange::SetData(B,2,5) {} "
		L"		 IRowsetResynch::ResynchRows(A,B) "
		L"			{ "
		L"				L1.ROW_RESYNCH.OKTODO.RETURN_UNWANTEDREASON "
		L"			} "
		L"		 IRowsetResynch::ResynchRows(A,B) {} "
		L"		}"
		L"	}"
		L"}"; 
	
	return Execute(CommandString, RESYNCH_ROWSET | CHANGE_IMMEDIATE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc ROW_RESYNCH - UNWANTED PHASE - OKTODO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Resynch::Variation_16()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2,5) {} "
		L"		 IRowsetChange::SetData(B,2,5) {} "
		L"		 IRowsetResynch::ResynchRows(A,B) "
		L"			{ "
		L"				L1.ROW_RESYNCH.OKTODO.RETURN_UNWANTEDPHASE "
		L"			} "
		L"		 IRowsetResynch::ResynchRows(A,B) {} "
		L"		}"
		L"	}"
		L"}"; 
	
	return Execute(CommandString, RESYNCH_ROWSET | CHANGE_IMMEDIATE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc ROW_RESYNCH - UNWANTED PHASE - ABOUTTODO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Resynch::Variation_17()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2,5) {} "
		L"		 IRowsetChange::SetData(B,2,5) {} "
		L"		 IRowsetResynch::ResynchRows(A,B) "
		L"			{ "
		L"				L1.ROW_RESYNCH.ABOUTTODO.RETURN_UNWANTEDPHASE "
		L"			} "
		L"		 IRowsetResynch::ResynchRows(A,B) {} "
		L"		}"
		L"	}"
		L"}"; 
	
	return Execute(CommandString, RESYNCH_ROWSET | CHANGE_IMMEDIATE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc ROW_RESYNCH - UNWANTED PHASE - SYNCHAFTER
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Resynch::Variation_18()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2,5) {} "
		L"		 IRowsetChange::SetData(B,2,5) {} "
		L"		 IRowsetResynch::ResynchRows(A,B) "
		L"			{ "
		L"				L1.ROW_RESYNCH.SYNCHAFTER.RETURN_UNWANTEDPHASE "
		L"			} "
		L"		 IRowsetResynch::ResynchRows(A,B) {} "
		L"		}"
		L"	}"
		L"}"; 
	
	return Execute(CommandString, RESYNCH_ROWSET | CHANGE_IMMEDIATE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc ROW_RESYNCH - UNWANTED PHASE - DIDEVENT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Resynch::Variation_19()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2,5) {} "
		L"		 IRowsetChange::SetData(B,2,5) {} "
		L"		 IRowsetResynch::ResynchRows(A,B) "
		L"			{ "
		L"				L1.ROW_RESYNCH.DIDEVENT.RETURN_UNWANTEDPHASE "
		L"			} "
		L"		 IRowsetResynch::ResynchRows(A,B) {} "
		L"		}"
		L"	}"
		L"}"; 
	
	return Execute(CommandString, RESYNCH_ROWSET | CHANGE_IMMEDIATE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc ROW_RESYNCH - UNWANTED PHASE - FAILEDTODO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Resynch::Variation_20()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2,5) {} "
		L"		 IRowsetChange::SetData(B,2,5) {} "
		L"		 IRowsetResynch::ResynchRows(A,B) "
		L"			{ "
		L"				L1.ROW_RESYNCH.ABOUTTODO.RETURN_VETO "
		L"				L1.ROW_RESYNCH.FAILEDTODO.RETURN_UNWANTEDPHASE "
		L"			} "
		L"		 IRowsetResynch::ResynchRows(A,B) {} "
		L"		}"
		L"	}"
		L"}"; 
	
	//To produce FAILEDTODO, this reason/phase must be vetoable...
	TESTC_PROVIDER(IsCancelableEvent(DBREASON_ROW_RESYNCH, DBEVENTPHASE_ABOUTTODO));
	TESTC(Execute(CommandString, RESYNCH_ROWSET | CHANGE_IMMEDIATE));

CLEANUP:
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Resynch::Variation_21()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(22)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Resynch::Variation_22()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(23)
//*-----------------------------------------------------------------------
// @mfunc ROW_RESYNCH - E_FAIL - All Phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Resynch::Variation_23()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2,5) {} "
		L"		 IRowsetChange::SetData(B,2,5) {} "
		L"		 IRowsetResynch::ResynchRows(A,B) "
		L"			{ "
		L"				L1.ROW_RESYNCH.OKTODO.RETURN_EFAIL "
		L"				L1.ROW_RESYNCH.ABOUTTODO.RETURN_EFAIL "
		L"				L1.ROW_RESYNCH.SYNCHAFTER.RETURN_EFAIL "
		L"				L1.ROW_RESYNCH.DIDEVENT.RETURN_EFAIL "
		L"			} "
		L"		 IRowsetResynch::ResynchRows(A,B) "
		L"			{ "
		L"				L1.ROW_RESYNCH.ABOUTTODO.RETURN_VETO "
		L"			} "
		L"		}"
		L"	}"
		L"}"; 
	
	return Execute(CommandString, RESYNCH_ROWSET | CHANGE_IMMEDIATE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(24)
//*-----------------------------------------------------------------------
// @mfunc ROW_RESYNCH - E_OUTOFMEMORY - All Phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Resynch::Variation_24()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2,5) {} "
		L"		 IRowsetChange::SetData(B,2,5) {} "
		L"		 IRowsetResynch::ResynchRows(A,B) "
		L"			{ "
		L"				L1.ROW_RESYNCH.OKTODO.RETURN_EOUTOFMEMORY "
		L"				L1.ROW_RESYNCH.ABOUTTODO.RETURN_EOUTOFMEMORY "
		L"				L1.ROW_RESYNCH.SYNCHAFTER.RETURN_EOUTOFMEMORY "
		L"				L1.ROW_RESYNCH.DIDEVENT.RETURN_EOUTOFMEMORY "
		L"			} "
		L"		 IRowsetResynch::ResynchRows(A,B) "
		L"			{ "
		L"				L1.ROW_RESYNCH.ABOUTTODO.RETURN_VETO "
		L"			} "
		L"		}"
		L"	}"
		L"}"; 
	
	return Execute(CommandString, RESYNCH_ROWSET | CHANGE_IMMEDIATE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(25)
//*-----------------------------------------------------------------------
// @mfunc ROW_RESYNCH - E_INVALIDARG - All Phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Resynch::Variation_25()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2,5) {} "
		L"		 IRowsetChange::SetData(B,2,5) {} "
		L"		 IRowsetResynch::ResynchRows(A,B) "
		L"			{ "
		L"				L1.ROW_RESYNCH.OKTODO.RETURN_EINVALIDARG "
		L"				L1.ROW_RESYNCH.ABOUTTODO.RETURN_EINVALIDARG "
		L"				L1.ROW_RESYNCH.SYNCHAFTER.RETURN_EINVALIDARG "
		L"				L1.ROW_RESYNCH.DIDEVENT.RETURN_EINVALIDARG "
		L"			} "
		L"		 IRowsetResynch::ResynchRows(A,B) "
		L"			{ "
		L"				L1.ROW_RESYNCH.ABOUTTODO.RETURN_VETO "
		L"			} "
		L"		}"
		L"	}"
		L"}"; 
	
	return Execute(CommandString, RESYNCH_ROWSET | CHANGE_IMMEDIATE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(26)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Resynch::Variation_26()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(27)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Resynch::Variation_27()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(28)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - Resynch all rows
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Resynch::Variation_28()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2,5) {} "
		L"		 IRowsetChange::SetData(B,2,5) {} "
		L"		 IRowsetResynch::ResynchRows() "
		L"			{ "
		L"				L1.ROW_RESYNCH.OKTODO.RETURN_ACCEPT "
		L"			} "
		L"		 IRowsetResynch::ResynchRows(A,B) "
		L"			{ "
		L"				L1.ROW_RESYNCH.OKTODO.RETURN_ACCEPT "
		L"			} "
		L"		}"
		L"	}"
		L"}"; 
	
	return Execute(CommandString, RESYNCH_ROWSET | CHANGE_IMMEDIATE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(29)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - Resynch 2 rows, 1 changed 1 not changed
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Resynch::Variation_29()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2,5) {} "
		L"		 IRowsetResynch::ResynchRows(A,B) "
		L"			{ "
		L"				L1.ROW_RESYNCH.OKTODO.RETURN_ACCEPT "
		L"			} "
		L"		 IRowsetResynch::ResynchRows(A,B) "
		L"			{ "
		L"				L1.ROW_RESYNCH.OKTODO.RETURN_ACCEPT "
		L"			} "
		L"		}"
		L"	}"
		L"}"; 
	
	return Execute(CommandString, RESYNCH_ROWSET | CHANGE_IMMEDIATE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(30)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - 1 valid, 1 not valid
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Resynch::Variation_30()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2,5) {} "
		L"		 IRowsetResynch::ResynchRows(A,C,DB_S_ERRORSOCCURRED) "
		L"			{ "
		L"				L1.ROW_RESYNCH.OKTODO.RETURN_ACCEPT "
		L"			} "
		L"		 IRowsetResynch::ResynchRows(A,B) "
		L"			{ "
		L"				L1.ROW_RESYNCH.OKTODO.RETURN_ACCEPT "
		L"			} "
		L"		}"
		L"	}"
		L"}"; 
	
	return Execute(CommandString, RESYNCH_ROWSET | CHANGE_IMMEDIATE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(31)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - EmptyRowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Resynch::Variation_31()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowsetResynch::ResynchRows() "
		L"			{ "
		L"			} "
		L"		 IRowsetResynch::ResynchRows() "
		L"			{ "
		L"			} "
		L"		}"
		L"	}"
		L"}"; 
	
	return Execute(CommandString, EMPTY_ROWSET | RESYNCH_ROWSET | CHANGE_IMMEDIATE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(32)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - Add/Remove Listeners
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Resynch::Variation_32()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2,5) {} "
		L"		 IRowsetResynch::ResynchRows(A,B) "
		L"			{ "
		L"				L1.ROW_RESYNCH.OKTODO.RETURN_UNADVISE "
		L"			} "
		L"		 IRowsetResynch::ResynchRows(A,B) "
		L"			{ "
		L"				L2.ROW_RESYNCH.OKTODO.RETURN_UNADVISE "
		L"				L3.ROW_RESYNCH.OKTODO.RETURN_UNADVISE "
		L"			} "
		L"		}"
		L"	}"
		L"}"; 
	
	return Execute(CommandString, RESYNCH_ROWSET | CHANGE_IMMEDIATE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(33)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - GetData
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Resynch::Variation_33()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2,5) {} "
		L"		 IRowsetResynch::ResynchRows(A,B) "
		L"			{ "
		L"				L1.ROW_RESYNCH.OKTODO.IRowset::GetData(A) {}  "
		L"			} "
		L"		 IRowsetResynch::ResynchRows(A,B) "
		L"			{ "
		L"				L1.ROW_RESYNCH.OKTODO.IRowset::GetData(B) {}  "
		L"			} "
		L"		}"
		L"	}"
		L"}"; 
	
	return Execute(CommandString, RESYNCH_ROWSET | CHANGE_IMMEDIATE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(34)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - GetVisibleData
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Resynch::Variation_34()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2,5) {} "
		L"		 IRowsetResynch::ResynchRows(A,B) "
		L"			{ "
		L"				L1.ROW_RESYNCH.OKTODO.IRowsetResynch::GetVisibleData(A, DB_E_NOTREENTRANT) {}  "
		L"			} "
		L"		 IRowsetResynch::ResynchRows(A,B) "
		L"			{ "
		L"				L1.ROW_RESYNCH.OKTODO.IRowsetResynch::GetVisibleData(B, DB_E_NOTREENTRANT) {}  "
		L"			} "
		L"		}"
		L"	}"
		L"}"; 
	
	return Execute(CommandString, RESYNCH_ROWSET | CHANGE_IMMEDIATE);
}
// }}
// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Resynch::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CExecutionManager::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(Resynch_Bufferred)
//*-----------------------------------------------------------------------
//| Test Case:		Resynch_Bufferred - Tests Notifications for IRowsetResynch_Bufferred::ResyncRows
//|	Created:			09/29/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Resynch_Bufferred::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CExecutionManager::Init())
	// }}
	{
		//Verify Rowset can be created...
		TEST_PROVIDER(CreateRowset(RESYNCH_ROWSET | CHANGE_BUFFERRED)==S_OK);
		return TRUE;
	}

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc ROW_RESYNCH - Accept all phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Resynch_Bufferred::Variation_1()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B,C) {} "
		L"		 IRowsetChange::SetData(A,2,5) {} "
		L"		 IRowsetChange::SetData(B,2,5) {} "
		L"		 IRowsetChange::SetData(C,2,5) {} "
		L"		 IRowsetResynch::ResynchRows(A,B) "
		L"			{ "
		L"				L1.ROW_RESYNCH.OKTODO.RETURN_ACCEPT "
		L"			} "
		L"		 IRowsetResynch::ResynchRows(A,B,C) {} "
		L"		}"
		L"	}"
		L"}"; 
	
	return Execute(CommandString, RESYNCH_ROWSET | CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Resynch_Bufferred::Variation_2()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Resynch_Bufferred::Variation_3()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc ROW_RESYNCH - DB_E_CANCELED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Resynch_Bufferred::Variation_4()
{
	//Veriy the veto was successful
	//Row should be unaffected by the resynch

    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2,5) {} "
		L"		 IRowsetChange::SetData(B,2,5) {} "
		L"		 IRowsetResynch::ResynchRows(A,B) "
		L"			{ "
		L"				L1.ROW_RESYNCH.ABOUTTODO.RETURN_VETO "
		L"			} "
		L"		 IRowset::GetData(A) {} "
		L"		 IRowsetResynch::ResynchRows(A,B) {} "
		L"		}"
		L"	}"
		L"}"; 
	
	return Execute(CommandString, RESYNCH_ROWSET | CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc ROW_RESYNCH - DB_E_NOTREENTRANT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Resynch_Bufferred::Variation_5()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2,5) {} "
		L"		 IRowsetChange::SetData(B,2,5) {} "
		L"		 IRowsetResynch::ResynchRows(A,B) "
		L"			{ "
		L"				L1.ROW_RESYNCH.OKTODO.IRowsetResynch::ResynchRows(A,B,DB_E_NOTREENTRANT) {} "
		L"			} "
		L"		 IRowsetResynch::ResynchRows(A,B) {} "
		L"		}"
		L"	}"
		L"}"; 
	
	return Execute(CommandString, RESYNCH_ROWSET | CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Resynch_Bufferred::Variation_6()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Resynch_Bufferred::Variation_7()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc ROW_RESYNCH - Veto OKTODO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Resynch_Bufferred::Variation_8()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2,5) {} "
		L"		 IRowsetChange::SetData(B,2,5) {} "
		L"		 IRowsetResynch::ResynchRows(A,B) "
		L"			{ "
		L"				L1.ROW_RESYNCH.OKTODO.RETURN_VETO "
		L"			} "
		L"		 IRowsetResynch::ResynchRows(A,B) {} "
		L"		}"
		L"	}"
		L"}"; 
	
	return Execute(CommandString, RESYNCH_ROWSET | CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc ROW_RESYNCH - Veto ABOUTTODO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Resynch_Bufferred::Variation_9()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2,5) {} "
		L"		 IRowsetChange::SetData(B,2,5) {} "
		L"		 IRowsetResynch::ResynchRows(A,B) "
		L"			{ "
		L"				L1.ROW_RESYNCH.ABOUTTODO.RETURN_VETO "
		L"			} "
		L"		 IRowsetResynch::ResynchRows(A,B) {} "
		L"		}"
		L"	}"
		L"}"; 
	
	return Execute(CommandString, RESYNCH_ROWSET | CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc ROW_RESYNCH - Veto SYNCHAFTER
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Resynch_Bufferred::Variation_10()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2,5) {} "
		L"		 IRowsetChange::SetData(B,2,5) {} "
		L"		 IRowsetResynch::ResynchRows(A,B) "
		L"			{ "
		L"				L1.ROW_RESYNCH.SYNCHAFTER.RETURN_VETO "
		L"			} "
		L"		 IRowsetResynch::ResynchRows(A,B) {} "
		L"		}"
		L"	}"
		L"}"; 
	
	return Execute(CommandString, RESYNCH_ROWSET | CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc ROW_RESYNCH - Veto DIDEVENT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Resynch_Bufferred::Variation_11()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2,5) {} "
		L"		 IRowsetChange::SetData(B,2,5) {} "
		L"		 IRowsetResynch::ResynchRows(A,B) "
		L"			{ "
		L"				L1.ROW_RESYNCH.DIDEVENT.RETURN_VETO "
		L"			} "
		L"		 IRowsetResynch::ResynchRows(A,B) {} "
		L"		}"
		L"	}"
		L"}"; 
	
	return Execute(CommandString, RESYNCH_ROWSET | CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc ROW_RESYNCH - Veto FAILEDTODO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Resynch_Bufferred::Variation_12()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2,5) {} "
		L"		 IRowsetChange::SetData(B,2,5) {} "
		L"		 IRowsetResynch::ResynchRows(A,B) "
		L"			{ "
		L"				L1.ROW_RESYNCH.ABOUTTODO.RETURN_VETO "
		L"				L1.ROW_RESYNCH.FAILEDTODO.RETURN_VETO "
		L"			} "
		L"		 IRowsetResynch::ResynchRows(A,B) {} "
		L"		}"
		L"	}"
		L"}"; 
	
	//To produce FAILEDTODO, this reason/phase must be vetoable...
	TESTC_PROVIDER(IsCancelableEvent(DBREASON_ROW_RESYNCH, DBEVENTPHASE_ABOUTTODO));
	TESTC(Execute(CommandString, RESYNCH_ROWSET | CHANGE_BUFFERRED));

CLEANUP:
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Resynch_Bufferred::Variation_13()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Resynch_Bufferred::Variation_14()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc ROW_RESYNCH - UNWANTED REASON
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Resynch_Bufferred::Variation_15()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2,5) {} "
		L"		 IRowsetChange::SetData(B,2,5) {} "
		L"		 IRowsetResynch::ResynchRows(A,B) "
		L"			{ "
		L"				L1.ROW_RESYNCH.OKTODO.RETURN_UNWANTEDREASON "
		L"			} "
		L"		 IRowsetResynch::ResynchRows(A,B) {} "
		L"		}"
		L"	}"
		L"}"; 
	
	return Execute(CommandString, RESYNCH_ROWSET | CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc ROW_RESYNCH - UNWANTED PHASE - OKTODO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Resynch_Bufferred::Variation_16()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2,5) {} "
		L"		 IRowsetChange::SetData(B,2,5) {} "
		L"		 IRowsetResynch::ResynchRows(A,B) "
		L"			{ "
		L"				L1.ROW_RESYNCH.OKTODO.RETURN_UNWANTEDPHASE "
		L"			} "
		L"		 IRowsetResynch::ResynchRows(A,B) {} "
		L"		}"
		L"	}"
		L"}"; 
	
	return Execute(CommandString, RESYNCH_ROWSET | CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc ROW_RESYNCH - UNWANTED PHASE - ABOUTTODO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Resynch_Bufferred::Variation_17()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2,5) {} "
		L"		 IRowsetChange::SetData(B,2,5) {} "
		L"		 IRowsetResynch::ResynchRows(A,B) "
		L"			{ "
		L"				L1.ROW_RESYNCH.ABOUTTODO.RETURN_UNWANTEDPHASE "
		L"			} "
		L"		 IRowsetResynch::ResynchRows(A,B) {} "
		L"		}"
		L"	}"
		L"}"; 
	
	return Execute(CommandString, RESYNCH_ROWSET | CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc ROW_RESYNCH - UNWANTED PHASE - SYNCHAFTER
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Resynch_Bufferred::Variation_18()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2,5) {} "
		L"		 IRowsetChange::SetData(B,2,5) {} "
		L"		 IRowsetResynch::ResynchRows(A,B) "
		L"			{ "
		L"				L1.ROW_RESYNCH.SYNCHAFTER.RETURN_UNWANTEDPHASE "
		L"			} "
		L"		 IRowsetResynch::ResynchRows(A,B) {} "
		L"		}"
		L"	}"
		L"}"; 
	
	return Execute(CommandString, RESYNCH_ROWSET | CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc ROW_RESYNCH - UNWANTED PHASE - DIDEVENT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Resynch_Bufferred::Variation_19()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2,5) {} "
		L"		 IRowsetChange::SetData(B,2,5) {} "
		L"		 IRowsetResynch::ResynchRows(A,B) "
		L"			{ "
		L"				L1.ROW_RESYNCH.DIDEVENT.RETURN_UNWANTEDPHASE "
		L"			} "
		L"		 IRowsetResynch::ResynchRows(A,B) {} "
		L"		}"
		L"	}"
		L"}"; 
	
	return Execute(CommandString, RESYNCH_ROWSET | CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc ROW_RESYNCH - UNWANTED PHASE - FAILEDTODO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Resynch_Bufferred::Variation_20()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2,5) {} "
		L"		 IRowsetChange::SetData(B,2,5) {} "
		L"		 IRowsetResynch::ResynchRows(A,B) "
		L"			{ "
		L"				L1.ROW_RESYNCH.ABOUTTODO.RETURN_VETO "
		L"				L1.ROW_RESYNCH.FAILEDTODO.RETURN_UNWANTEDPHASE "
		L"			} "
		L"		 IRowsetResynch::ResynchRows(A,B) {} "
		L"		}"
		L"	}"
		L"}"; 
	
	//To produce FAILEDTODO, this reason/phase must be vetoable...
	TESTC_PROVIDER(IsCancelableEvent(DBREASON_ROW_RESYNCH, DBEVENTPHASE_ABOUTTODO));
	TESTC(Execute(CommandString, RESYNCH_ROWSET | CHANGE_BUFFERRED));

CLEANUP:
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Resynch_Bufferred::Variation_21()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(22)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Resynch_Bufferred::Variation_22()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(23)
//*-----------------------------------------------------------------------
// @mfunc ROW_RESYNCH - E_FAIL - All Phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Resynch_Bufferred::Variation_23()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2,5) {} "
		L"		 IRowsetChange::SetData(B,2,5) {} "
		L"		 IRowsetResynch::ResynchRows(A,B) "
		L"			{ "
		L"				L1.ROW_RESYNCH.OKTODO.RETURN_EFAIL "
		L"				L1.ROW_RESYNCH.ABOUTTODO.RETURN_EFAIL "
		L"				L1.ROW_RESYNCH.SYNCHAFTER.RETURN_EFAIL "
		L"				L1.ROW_RESYNCH.DIDEVENT.RETURN_EFAIL "
		L"			} "
		L"		 IRowsetResynch::ResynchRows(A,B) "
		L"			{ "
		L"				L1.ROW_RESYNCH.ABOUTTODO.RETURN_VETO "
		L"			} "
		L"		}"
		L"	}"
		L"}"; 
	
	return Execute(CommandString, RESYNCH_ROWSET | CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(24)
//*-----------------------------------------------------------------------
// @mfunc ROW_RESYNCH - E_OUTOFMEMORY - All Phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Resynch_Bufferred::Variation_24()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2,5) {} "
		L"		 IRowsetChange::SetData(B,2,5) {} "
		L"		 IRowsetResynch::ResynchRows(A,B) "
		L"			{ "
		L"				L1.ROW_RESYNCH.OKTODO.RETURN_EOUTOFMEMORY "
		L"				L1.ROW_RESYNCH.ABOUTTODO.RETURN_EOUTOFMEMORY "
		L"				L1.ROW_RESYNCH.SYNCHAFTER.RETURN_EOUTOFMEMORY "
		L"				L1.ROW_RESYNCH.DIDEVENT.RETURN_EOUTOFMEMORY "
		L"			} "
		L"		 IRowsetResynch::ResynchRows(A,B) "
		L"			{ "
		L"				L1.ROW_RESYNCH.ABOUTTODO.RETURN_VETO "
		L"			} "
		L"		}"
		L"	}"
		L"}"; 
	
	return Execute(CommandString, RESYNCH_ROWSET | CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(25)
//*-----------------------------------------------------------------------
// @mfunc ROW_RESYNCH - E_INVALIDARG - All Phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Resynch_Bufferred::Variation_25()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2,5) {} "
		L"		 IRowsetChange::SetData(B,2,5) {} "
		L"		 IRowsetResynch::ResynchRows(A,B) "
		L"			{ "
		L"				L1.ROW_RESYNCH.OKTODO.RETURN_EINVALIDARG "
		L"				L1.ROW_RESYNCH.ABOUTTODO.RETURN_EINVALIDARG "
		L"				L1.ROW_RESYNCH.SYNCHAFTER.RETURN_EINVALIDARG "
		L"				L1.ROW_RESYNCH.DIDEVENT.RETURN_EINVALIDARG "
		L"			} "
		L"		 IRowsetResynch::ResynchRows(A,B) "
		L"			{ "
		L"				L1.ROW_RESYNCH.ABOUTTODO.RETURN_VETO "
		L"			} "
		L"		}"
		L"	}"
		L"}"; 
	
	return Execute(CommandString, RESYNCH_ROWSET | CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(26)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Resynch_Bufferred::Variation_26()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(27)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Resynch_Bufferred::Variation_27()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(28)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - Resynch_Bufferred all rows
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Resynch_Bufferred::Variation_28()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2,5) {} "
		L"		 IRowsetChange::SetData(B,2,5) {} "
		L"		 IRowsetResynch::ResynchRows() "
		L"			{ "
		L"				L1.ROW_RESYNCH.OKTODO.RETURN_ACCEPT "
		L"			} "
		L"		 IRowsetResynch::ResynchRows() "
		L"			{ "
		L"				L1.ROW_RESYNCH.OKTODO.RETURN_ACCEPT "
		L"			} "
		L"		}"
		L"	}"
		L"}"; 
	
	return Execute(CommandString, RESYNCH_ROWSET | CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(29)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - Resynch_Bufferred 2 rows, 1 changed 1 not changed
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Resynch_Bufferred::Variation_29()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2,5) {} "
		L"		 IRowsetResynch::ResynchRows(A,B) "
		L"			{ "
		L"				L1.ROW_RESYNCH.OKTODO.RETURN_ACCEPT "
		L"			} "
		L"		 IRowsetResynch::ResynchRows(A,B) "
		L"			{ "
		L"				L1.ROW_RESYNCH.OKTODO.RETURN_ACCEPT "
		L"			} "
		L"		}"
		L"	}"
		L"}"; 
	
	return Execute(CommandString, RESYNCH_ROWSET | CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(30)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - 1 valid, 1 not valid
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Resynch_Bufferred::Variation_30()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2,5) {} "
		L"		 IRowsetResynch::ResynchRows(A,C,DB_S_ERRORSOCCURRED) "
		L"			{ "
		L"				L1.ROW_RESYNCH.OKTODO.RETURN_ACCEPT "
		L"			} "
		L"		 IRowsetResynch::ResynchRows(A,B) "
		L"			{ "
		L"				L1.ROW_RESYNCH.OKTODO.RETURN_ACCEPT "
		L"			} "
		L"		}"
		L"	}"
		L"}"; 
	
	return Execute(CommandString, RESYNCH_ROWSET | CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(31)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - EmptyRowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Resynch_Bufferred::Variation_31()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowsetResynch::ResynchRows() "
		L"			{ "
		L"			} "
		L"		 IRowsetResynch::ResynchRows() "
		L"			{ "
		L"			} "
		L"		}"
		L"	}"
		L"}"; 
	
	return Execute(CommandString, EMPTY_ROWSET | RESYNCH_ROWSET | CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(32)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - Add/Remove Listeners
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Resynch_Bufferred::Variation_32()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2,5) {} "
		L"		 IRowsetResynch::ResynchRows(A,B) "
		L"			{ "
		L"				L1.ROW_RESYNCH.OKTODO.RETURN_ADVISE "
		L"			} "
		L"		 IRowsetResynch::ResynchRows(A,B) "
		L"			{ "
		L"				L1.ROW_RESYNCH.OKTODO.RETURN_UNADVISE "
		L"			} "
		L"		}"
		L"	}"
		L"}"; 
	
	return Execute(CommandString, RESYNCH_ROWSET | CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(33)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - GetData
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Resynch_Bufferred::Variation_33()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2,5) {} "
		L"		 IRowsetResynch::ResynchRows(A,B) "
		L"			{ "
		L"				L1.ROW_RESYNCH.OKTODO.IRowset::GetData(A) {} "
		L"			} "
		L"		 IRowsetResynch::ResynchRows(A,B) "
		L"			{ "
		L"				L1.ROW_RESYNCH.OKTODO.IRowset::GetData(B) {} "
		L"			} "
		L"		}"
		L"	}"
		L"}"; 
	
	return Execute(CommandString, RESYNCH_ROWSET | CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(34)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - GetVisibleData
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Resynch_Bufferred::Variation_34()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::SetData(A,2,5) {} "
		L"		 IRowsetResynch::ResynchRows(A,B) "
		L"			{ "
		L"				L1.ROW_RESYNCH.OKTODO.IRowsetResynch::GetVisibleData(A, DB_E_NOTREENTRANT) {} "
		L"			} "
		L"		 IRowsetResynch::ResynchRows(A,B) "
		L"			{ "
		L"				L1.ROW_RESYNCH.OKTODO.IRowsetResynch::GetVisibleData(B, DB_E_NOTREENTRANT) {} "
		L"			} "
		L"		}"
		L"	}"
		L"}"; 
	
	return Execute(CommandString, RESYNCH_ROWSET | CHANGE_BUFFERRED);
}
// }}
// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Resynch_Bufferred::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CExecutionManager::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(Undo)
//*-----------------------------------------------------------------------
//| Test Case:		Undo - Tests Notifications for IRowsetUpdate::Undo
//|	Created:			09/29/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Undo::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CExecutionManager::Init())
	// }}
	{
		//Verify Rowset can be created...
		TEST_PROVIDER(CreateRowset(CHANGE_BUFFERRED)==S_OK);
		return TRUE;
	}

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc ROW_UNDOCHANGE - Accept all phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Undo::Variation_1()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B,C) {} "
		L"		 IRowsetChange::InsertRow(D) {} "
		L"		 IRowsetChange::DeleteRows(A) {} "
		L"		 IRowsetChange::SetData(B,2,5) {} " 
		L"		 IRowsetChange::SetData(C,2,5) {} " 
		L"		 IRowsetUpdate::Undo(B) "
		L"			{ "
		L"			 L1.ROW_UNDOCHANGE.OKTODO.RETURN_ACCEPT "
		L"			} "
		L"		 IRowsetUpdate::Undo(B,C) {} "
		L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Undo::Variation_2()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Undo::Variation_3()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc ROW_UNDOCHANGE - DB_E_CANCELED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Undo::Variation_4()
{
	//Veriy the veto was successful
	//Row should be unaffected by the Undo

    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B,C) {} "
		L"		 IRowsetChange::InsertRow(D) {} "
		L"		 IRowsetChange::DeleteRows(A) {} "
		L"		 IRowsetChange::SetData(B,2,5) {} " 
		L"		 IRowsetChange::SetData(C,2,5) {} " 
		L"		 IRowsetUpdate::Undo(B) "
		L"			{ "
		L"			 L1.ROW_UNDOCHANGE.ABOUTTODO.RETURN_VETO "
		L"			} "
		L"		 IRowset::GetData(B) {} "
		L"		 IRowsetUpdate::Undo(B,C) {} "
		L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc ROW_UNDOCHANGE - DB_E_NOTREENTRANT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Undo::Variation_5()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B,C) {} "
		L"		 IRowsetChange::InsertRow(D) {} "
		L"		 IRowsetChange::DeleteRows(A) {} "
		L"		 IRowsetChange::SetData(B,2,5) {} " 
		L"		 IRowsetChange::SetData(C,2,5) {} " 
		L"		 IRowsetUpdate::Undo(B) "
		L"			{ "
		L"			 L1.ROW_UNDOCHANGE.OKTODO.IRowsetUpdate::Undo(B,DB_E_NOTREENTRANT) {} "
		L"			} "
		L"		 IRowsetUpdate::Undo(B,C) {} "
		L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Undo::Variation_6()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Undo::Variation_7()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc ROW_UNDOCHANGE - Veto OKTODO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Undo::Variation_8()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B,C) {} "
		L"		 IRowsetChange::InsertRow(D) {} "
		L"		 IRowsetChange::DeleteRows(A) {} "
		L"		 IRowsetChange::SetData(B,2,5) {} " 
		L"		 IRowsetChange::SetData(C,2,5) {} " 
		L"		 IRowsetUpdate::Undo(B) "
		L"			{ "
		L"			 L1.ROW_UNDOCHANGE.OKTODO.RETURN_VETO "
		L"			} "
		L"		 IRowsetUpdate::Undo(B,C) {} "
		L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc ROW_UNDOCHANGE - Veto ABOUTTODO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Undo::Variation_9()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B,C) {} "
		L"		 IRowsetChange::InsertRow(D) {} "
		L"		 IRowsetChange::DeleteRows(A) {} "
		L"		 IRowsetChange::SetData(B,2,5) {} " 
		L"		 IRowsetChange::SetData(C,2,5) {} " 
		L"		 IRowsetUpdate::Undo(B) "
		L"			{ "
		L"			 L1.ROW_UNDOCHANGE.ABOUTTODO.RETURN_VETO "
		L"			} "
		L"		 IRowsetUpdate::Undo(B,C) {} "
		L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc ROW_UNDOCHANGE - Veto SYNCHAFTER
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Undo::Variation_10()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B,C) {} "
		L"		 IRowsetChange::InsertRow(D) {} "
		L"		 IRowsetChange::DeleteRows(A) {} "
		L"		 IRowsetChange::SetData(B,2,5) {} " 
		L"		 IRowsetChange::SetData(C,2,5) {} " 
		L"		 IRowsetUpdate::Undo(B) "
		L"			{ "
		L"			 L1.ROW_UNDOCHANGE.SYNCHAFTER.RETURN_VETO "
		L"			} "
		L"		 IRowsetUpdate::Undo(B,C) {} "
		L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc ROW_UNDOCHANGE - Veto DIDEVENT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Undo::Variation_11()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B,C) {} "
		L"		 IRowsetChange::InsertRow(D) {} "
		L"		 IRowsetChange::DeleteRows(A) {} "
		L"		 IRowsetChange::SetData(B,2,5) {} " 
		L"		 IRowsetChange::SetData(C,2,5) {} " 
		L"		 IRowsetUpdate::Undo(B) "
		L"			{ "
		L"			 L1.ROW_UNDOCHANGE.DIDEVENT.RETURN_VETO "
		L"			} "
		L"		 IRowsetUpdate::Undo(B,C) {} "
		L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc ROW_UNDOCHANGE - Veto FAILEDTODO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Undo::Variation_12()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B,C) {} "
		L"		 IRowsetChange::InsertRow(D) {} "
		L"		 IRowsetChange::DeleteRows(A) {} "
		L"		 IRowsetChange::SetData(B,2,5) {} " 
		L"		 IRowsetChange::SetData(C,2,5) {} " 
		L"		 IRowsetUpdate::Undo(B) "
		L"			{ "
		L"			 L1.ROW_UNDOCHANGE.ABOUTTODO.RETURN_VETO "
		L"			 L1.ROW_UNDOCHANGE.FAILEDTODO.RETURN_VETO "
		L"			} "
		L"		 IRowsetUpdate::Undo(B,C) {} "
		L"		}"
		L"	}"
		L"}";
	
	//To produce FAILEDTODO, this reason/phase must be vetoable...
	TESTC_PROVIDER(IsCancelableEvent(DBREASON_ROW_UNDOCHANGE, DBEVENTPHASE_ABOUTTODO));
	TESTC(Execute(CommandString, CHANGE_BUFFERRED));

CLEANUP:
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Undo::Variation_13()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Undo::Variation_14()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc ROW_UNDOCHANGE - UNWANTED REASON
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Undo::Variation_15()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B,C) {} "
		L"		 IRowsetChange::InsertRow(D) {} "
		L"		 IRowsetChange::DeleteRows(A) {} "
		L"		 IRowsetChange::SetData(B,2,5) {} " 
		L"		 IRowsetChange::SetData(C,2,5) {} " 
		L"		 IRowsetUpdate::Undo(B) "
		L"			{ "
		L"			 L1.ROW_UNDOCHANGE.DIDEVENT.RETURN_UNWANTEDREASON "
		L"			} "
		L"		 IRowsetUpdate::Undo(B,C) {} "
		L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc ROW_UNDOCHANGE - UNWANTED PHASE OKTODO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Undo::Variation_16()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B,C) {} "
		L"		 IRowsetChange::InsertRow(D) {} "
		L"		 IRowsetChange::DeleteRows(A) {} "
		L"		 IRowsetChange::SetData(B,2,5) {} " 
		L"		 IRowsetChange::SetData(C,2,5) {} " 
		L"		 IRowsetUpdate::Undo(B) "
		L"			{ "
		L"			 L1.ROW_UNDOCHANGE.OKTODO.RETURN_UNWANTEDPHASE "
		L"			} "
		L"		 IRowsetUpdate::Undo(B,C) {} "
		L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc ROW_UNDOCHANGE - UNWANTED PHASE ABOUTTODO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Undo::Variation_17()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B,C) {} "
		L"		 IRowsetChange::InsertRow(D) {} "
		L"		 IRowsetChange::DeleteRows(A) {} "
		L"		 IRowsetChange::SetData(B,2,5) {} " 
		L"		 IRowsetChange::SetData(C,2,5) {} " 
		L"		 IRowsetUpdate::Undo(B) "
		L"			{ "
		L"			 L1.ROW_UNDOCHANGE.ABOUTTODO.RETURN_UNWANTEDPHASE "
		L"			} "
		L"		 IRowsetUpdate::Undo(B,C) {} "
		L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc ROW_UNDOCHANGE - UNWANTED PHASE SYNCHAFTER
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Undo::Variation_18()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B,C) {} "
		L"		 IRowsetChange::InsertRow(D) {} "
		L"		 IRowsetChange::DeleteRows(A) {} "
		L"		 IRowsetChange::SetData(B,2,5) {} " 
		L"		 IRowsetChange::SetData(C,2,5) {} " 
		L"		 IRowsetUpdate::Undo(B) "
		L"			{ "
		L"			 L1.ROW_UNDOCHANGE.SYNCHAFTER.RETURN_UNWANTEDPHASE "
		L"			} "
		L"		 IRowsetUpdate::Undo(B,C) {} "
		L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc ROW_UNDOCHANGE - UNWANTED PHASE DIDEVENT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Undo::Variation_19()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B,C) {} "
		L"		 IRowsetChange::InsertRow(D) {} "
		L"		 IRowsetChange::DeleteRows(A) {} "
		L"		 IRowsetChange::SetData(B,2,5) {} " 
		L"		 IRowsetChange::SetData(C,2,5) {} " 
		L"		 IRowsetUpdate::Undo(B) "
		L"			{ "
		L"			 L1.ROW_UNDOCHANGE.DIDEVENT.RETURN_UNWANTEDPHASE "
		L"			} "
		L"		 IRowsetUpdate::Undo(B,C) {} "
		L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc ROW_UNDOCHANGE - UNWANTED PHASE FAILEDTODO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Undo::Variation_20()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B,C) {} "
		L"		 IRowsetChange::InsertRow(D) {} "
		L"		 IRowsetChange::DeleteRows(A) {} "
		L"		 IRowsetChange::SetData(B,2,5) {} " 
		L"		 IRowsetChange::SetData(C,2,5) {} " 
		L"		 IRowsetUpdate::Undo(B) "
		L"			{ "
		L"			 L1.ROW_UNDOCHANGE.ABOUTTODO.RETURN_VETO "
		L"			 L1.ROW_UNDOCHANGE.FAILEDTODO.RETURN_UNWANTEDPHASE "
		L"			} "
		L"		 IRowsetUpdate::Undo(B,C) {} "
		L"		}"
		L"	}"
		L"}";
	
	//To produce FAILEDTODO, this reason/phase must be vetoable...
	TESTC_PROVIDER(IsCancelableEvent(DBREASON_ROW_UNDOCHANGE, DBEVENTPHASE_ABOUTTODO));
	TESTC(Execute(CommandString, CHANGE_BUFFERRED));

CLEANUP:
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Undo::Variation_21()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(22)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Undo::Variation_22()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(23)
//*-----------------------------------------------------------------------
// @mfunc ROW_UNDOCHANGE - E_FAIL - All Phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Undo::Variation_23()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B,C) {} "
		L"		 IRowsetChange::InsertRow(D) {} "
		L"		 IRowsetChange::DeleteRows(A) {} "
		L"		 IRowsetChange::SetData(B,2,5) {} " 
		L"		 IRowsetChange::SetData(C,2,5) {} " 
		L"		 IRowsetUpdate::Undo(B) "
		L"			{ "
		L"			 L1.ROW_UNDOCHANGE.OKTODO.RETURN_EFAIL "
		L"			 L1.ROW_UNDOCHANGE.ABOUTTODO.RETURN_EFAIL "
		L"			 L1.ROW_UNDOCHANGE.SYNCHAFTER.RETURN_EFAIL "
		L"			 L1.ROW_UNDOCHANGE.DIDEVENT.RETURN_EFAIL "
		L"			} "
		L"		 IRowsetUpdate::Undo(C) "
		L"			{ "
		L"			 L1.ROW_UNDOCHANGE.ABOUTTODO.RETURN_VETO "
		L"			} "
		L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(24)
//*-----------------------------------------------------------------------
// @mfunc ROW_UNDOCHANGE - E_OUTOFMEMORY - All Phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Undo::Variation_24()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B,C) {} "
		L"		 IRowsetChange::InsertRow(D) {} "
		L"		 IRowsetChange::DeleteRows(A) {} "
		L"		 IRowsetChange::SetData(B,2,5) {} " 
		L"		 IRowsetChange::SetData(C,2,5) {} " 
		L"		 IRowsetUpdate::Undo(B) "
		L"			{ "
		L"			 L1.ROW_UNDOCHANGE.OKTODO.RETURN_EOUTOFMEMORY "
		L"			 L1.ROW_UNDOCHANGE.ABOUTTODO.RETURN_EOUTOFMEMORY "
		L"			 L1.ROW_UNDOCHANGE.SYNCHAFTER.RETURN_EOUTOFMEMORY "
		L"			 L1.ROW_UNDOCHANGE.DIDEVENT.RETURN_EOUTOFMEMORY "
		L"			} "
		L"		 IRowsetUpdate::Undo(C) "
		L"			{ "
		L"			 L1.ROW_UNDOCHANGE.ABOUTTODO.RETURN_VETO "
		L"			} "
		L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(25)
//*-----------------------------------------------------------------------
// @mfunc ROW_UNDOCHANGE - E_INVALIDARG - All Phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Undo::Variation_25()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B,C) {} "
		L"		 IRowsetChange::InsertRow(D) {} "
		L"		 IRowsetChange::DeleteRows(A) {} "
		L"		 IRowsetChange::SetData(B,2,5) {} " 
		L"		 IRowsetChange::SetData(C,2,5) {} " 
		L"		 IRowsetUpdate::Undo(B) "
		L"			{ "
		L"			 L1.ROW_UNDOCHANGE.OKTODO.RETURN_EINVALIDARG "
		L"			 L1.ROW_UNDOCHANGE.ABOUTTODO.RETURN_EINVALIDARG "
		L"			 L1.ROW_UNDOCHANGE.SYNCHAFTER.RETURN_EINVALIDARG "
		L"			 L1.ROW_UNDOCHANGE.DIDEVENT.RETURN_EINVALIDARG "
		L"			} "
		L"		 IRowsetUpdate::Undo(C) "
		L"			{ "
		L"			 L1.ROW_UNDOCHANGE.ABOUTTODO.RETURN_VETO "
		L"			} "
		L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(26)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Undo::Variation_26()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(27)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Undo::Variation_27()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(28)
//*-----------------------------------------------------------------------
// @mfunc ROW_UNDOINSERT - Accept all phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Undo::Variation_28()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::InsertRow(C) {} "
		L"		 IRowsetChange::InsertRow(D) {} "
		L"		 IRowsetChange::DeleteRows(A) {} "
		L"		 IRowsetChange::SetData(D,2,5) {} " 
		L"		 IRowsetUpdate::Undo(C) "
		L"			{ "
		L"			 L1.ROW_UNDOINSERT.OKTODO.RETURN_ACCEPT "
		L"			} "
		L"		 IRowsetUpdate::Undo(D) {} "
		L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(29)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Undo::Variation_29()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(30)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Undo::Variation_30()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(31)
//*-----------------------------------------------------------------------
// @mfunc ROW_UNDOINSERT - DB_E_CANCELED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Undo::Variation_31()
{
	//Veriy the veto was successful
	//Row should be unaffected by the Undo

    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::InsertRow(C) {} "
		L"		 IRowsetChange::InsertRow(D) {} "
		L"		 IRowsetChange::DeleteRows(A) {} "
		L"		 IRowsetChange::SetData(D,2,5) {} " 
		L"		 IRowsetUpdate::Undo(C) "
		L"			{ "
		L"			 L1.ROW_UNDOINSERT.ABOUTTODO.RETURN_VETO "
		L"			} "
		L"		 IRowset::GetData(C) {} "
		L"		 IRowsetUpdate::Undo(D) {} "
		L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(32)
//*-----------------------------------------------------------------------
// @mfunc ROW_UNDOINSERT - DB_E_NOTREENTRANT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Undo::Variation_32()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::InsertRow(C) {} "
		L"		 IRowsetChange::InsertRow(D) {} "
		L"		 IRowsetChange::DeleteRows(A) {} "
		L"		 IRowsetChange::SetData(D,2,5) {} " 
		L"		 IRowsetUpdate::Undo(C) "
		L"			{ "
		L"			 L1.ROW_UNDOINSERT.OKTODO.IRowsetUpdate::Undo(C,DB_E_NOTREENTRANT) {} "
		L"			} "
		L"		 IRowsetUpdate::Undo(D) {} "
		L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(33)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Undo::Variation_33()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(34)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Undo::Variation_34()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(35)
//*-----------------------------------------------------------------------
// @mfunc ROW_UNDOINSERT - Veto OKTODO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Undo::Variation_35()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::InsertRow(C) {} "
		L"		 IRowsetChange::InsertRow(D) {} "
		L"		 IRowsetChange::DeleteRows(A) {} "
		L"		 IRowsetChange::SetData(D,2,5) {} " 
		L"		 IRowsetUpdate::Undo(C) "
		L"			{ "
		L"			 L1.ROW_UNDOINSERT.OKTODO.RETURN_VETO "
		L"			} "
		L"		 IRowsetUpdate::Undo(D) {} "
		L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(36)
//*-----------------------------------------------------------------------
// @mfunc ROW_UNDOINSERT - Veto ABOUTTODO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Undo::Variation_36()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::InsertRow(C) {} "
		L"		 IRowsetChange::InsertRow(D) {} "
		L"		 IRowsetChange::DeleteRows(A) {} "
		L"		 IRowsetChange::SetData(D,2,5) {} " 
		L"		 IRowsetUpdate::Undo(C) "
		L"			{ "
		L"			 L1.ROW_UNDOINSERT.ABOUTTODO.RETURN_VETO "
		L"			} "
		L"		 IRowsetUpdate::Undo(D) {} "
		L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(37)
//*-----------------------------------------------------------------------
// @mfunc ROW_UNDOINSERT - Veto SYNCHAFTER
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Undo::Variation_37()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::InsertRow(C) {} "
		L"		 IRowsetChange::InsertRow(D) {} "
		L"		 IRowsetChange::DeleteRows(A) {} "
		L"		 IRowsetChange::SetData(D,2,5) {} " 
		L"		 IRowsetUpdate::Undo(C) "
		L"			{ "
		L"			 L1.ROW_UNDOINSERT.SYNCHAFTER.RETURN_VETO "
		L"			} "
		L"		 IRowsetUpdate::Undo(D) {} "
		L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(38)
//*-----------------------------------------------------------------------
// @mfunc ROW_UNDOINSERT - Veto DIDEVENT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Undo::Variation_38()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::InsertRow(C) {} "
		L"		 IRowsetChange::InsertRow(D) {} "
		L"		 IRowsetChange::DeleteRows(A) {} "
		L"		 IRowsetChange::SetData(D,2,5) {} " 
		L"		 IRowsetUpdate::Undo(C) "
		L"			{ "
		L"			 L1.ROW_UNDOINSERT.DIDEVENT.RETURN_VETO "
		L"			} "
		L"		 IRowsetUpdate::Undo(D) {} "
		L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(39)
//*-----------------------------------------------------------------------
// @mfunc ROW_UNDOINSERT - Veto FAILEDTODO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Undo::Variation_39()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::InsertRow(C) {} "
		L"		 IRowsetChange::InsertRow(D) {} "
		L"		 IRowsetChange::DeleteRows(A) {} "
		L"		 IRowsetChange::SetData(D,2,5) {} " 
		L"		 IRowsetUpdate::Undo(C) "
		L"			{ "
		L"			 L1.ROW_UNDOINSERT.ABOUTTODO.RETURN_VETO "
		L"			 L1.ROW_UNDOINSERT.FAILEDTODO.RETURN_VETO "
		L"			} "
		L"		 IRowsetUpdate::Undo(D) {} "
		L"		}"
		L"	}"
		L"}";
	
	//To produce FAILEDTODO, this reason/phase must be vetoable...
	TESTC_PROVIDER(IsCancelableEvent(DBREASON_ROW_UNDOINSERT, DBEVENTPHASE_ABOUTTODO));
	TESTC(Execute(CommandString, CHANGE_BUFFERRED));

CLEANUP:
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(40)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Undo::Variation_40()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(41)
//*-----------------------------------------------------------------------
// @mfunc ROW_UNDOINSERT - UNWANTED REASON
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Undo::Variation_41()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::InsertRow(C) {} "
		L"		 IRowsetChange::InsertRow(D) {} "
		L"		 IRowsetChange::DeleteRows(A) {} "
		L"		 IRowsetChange::SetData(D,2,5) {} " 
		L"		 IRowsetUpdate::Undo(C) "
		L"			{ "
		L"			 L1.ROW_UNDOINSERT.OKTODO.RETURN_UNWANTEDREASON "
		L"			} "
		L"		 IRowsetUpdate::Undo(D) {} "
		L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(42)
//*-----------------------------------------------------------------------
// @mfunc ROW_UNDOINSERT - UNWANTED PHASE OKTODO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Undo::Variation_42()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::InsertRow(C) {} "
		L"		 IRowsetChange::InsertRow(D) {} "
		L"		 IRowsetChange::DeleteRows(A) {} "
		L"		 IRowsetChange::SetData(D,2,5) {} " 
		L"		 IRowsetUpdate::Undo(C) "
		L"			{ "
		L"			 L1.ROW_UNDOINSERT.OKTODO.RETURN_UNWANTEDPHASE "
		L"			} "
		L"		 IRowsetUpdate::Undo(D) {} "
		L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(43)
//*-----------------------------------------------------------------------
// @mfunc ROW_UNDOINSERT - UNWANTED PHASE ABOUTTO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Undo::Variation_43()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::InsertRow(C) {} "
		L"		 IRowsetChange::InsertRow(D) {} "
		L"		 IRowsetChange::DeleteRows(A) {} "
		L"		 IRowsetChange::SetData(D,2,5) {} " 
		L"		 IRowsetUpdate::Undo(C) "
		L"			{ "
		L"			 L1.ROW_UNDOINSERT.ABOUTTODO.RETURN_UNWANTEDPHASE "
		L"			} "
		L"		 IRowsetUpdate::Undo(D) {} "
		L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(44)
//*-----------------------------------------------------------------------
// @mfunc ROW_UNDOINSERT - UNWANTED PHASE SYNCHAFTER
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Undo::Variation_44()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::InsertRow(C) {} "
		L"		 IRowsetChange::InsertRow(D) {} "
		L"		 IRowsetChange::DeleteRows(A) {} "
		L"		 IRowsetChange::SetData(D,2,5) {} " 
		L"		 IRowsetUpdate::Undo(C) "
		L"			{ "
		L"			 L1.ROW_UNDOINSERT.SYNCHAFTER.RETURN_UNWANTEDPHASE "
		L"			} "
		L"		 IRowsetUpdate::Undo(D) {} "
		L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(45)
//*-----------------------------------------------------------------------
// @mfunc ROW_UNDOINSERT- UNWANTED PHASE DIDEVENT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Undo::Variation_45()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::InsertRow(C) {} "
		L"		 IRowsetChange::InsertRow(D) {} "
		L"		 IRowsetChange::DeleteRows(A) {} "
		L"		 IRowsetChange::SetData(D,2,5) {} " 
		L"		 IRowsetUpdate::Undo(C) "
		L"			{ "
		L"			 L1.ROW_UNDOINSERT.DIDEVENT.RETURN_UNWANTEDPHASE "
		L"			} "
		L"		 IRowsetUpdate::Undo(D) {} "
		L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(46)
//*-----------------------------------------------------------------------
// @mfunc ROW_UNDOINSERT - UNWANTED PHASE FAILEDTODO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Undo::Variation_46()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::InsertRow(C) {} "
		L"		 IRowsetChange::InsertRow(D) {} "
		L"		 IRowsetChange::DeleteRows(A) {} "
		L"		 IRowsetChange::SetData(D,2,5) {} " 
		L"		 IRowsetUpdate::Undo(C) "
		L"			{ "
		L"			 L1.ROW_UNDOINSERT.ABOUTTODO.RETURN_VETO "
		L"			 L1.ROW_UNDOINSERT.FAILEDTODO.RETURN_UNWANTEDPHASE "
		L"			} "
		L"		 IRowsetUpdate::Undo(D) {} "
		L"		}"
		L"	}"
		L"}";
	
	//To produce FAILEDTODO, this reason/phase must be vetoable...
	TESTC_PROVIDER(IsCancelableEvent(DBREASON_ROW_UNDOINSERT, DBEVENTPHASE_ABOUTTODO));
	TESTC(Execute(CommandString, CHANGE_BUFFERRED));

CLEANUP:
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(47)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Undo::Variation_47()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(48)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Undo::Variation_48()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(49)
//*-----------------------------------------------------------------------
// @mfunc ROW_UNDOINSERT - E_FAIL - All Phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Undo::Variation_49()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::InsertRow(C) {} "
		L"		 IRowsetChange::InsertRow(D) {} "
		L"		 IRowsetChange::DeleteRows(A) {} "
		L"		 IRowsetChange::SetData(D,2,5) {} " 
		L"		 IRowsetUpdate::Undo(C) "
		L"			{ "
		L"			 L1.ROW_UNDOINSERT.OKTODO.RETURN_EFAIL "
		L"			 L1.ROW_UNDOINSERT.ABOUTTODO.RETURN_EFAIL "
		L"			 L1.ROW_UNDOINSERT.SYNCHAFTER.RETURN_EFAIL "
		L"			 L1.ROW_UNDOINSERT.DIDEVENT.RETURN_EFAIL "
		L"			} "
		L"		 IRowsetUpdate::Undo(D) "
		L"			{ "
		L"			 L1.ROW_UNDOINSERT.ABOUTTODO.RETURN_VETO "
		L"			} "
		L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(50)
//*-----------------------------------------------------------------------
// @mfunc ROW_UNDOINSERT - E_OUTOFMEMORY - All Phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Undo::Variation_50()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::InsertRow(C) {} "
		L"		 IRowsetChange::InsertRow(D) {} "
		L"		 IRowsetChange::DeleteRows(A) {} "
		L"		 IRowsetChange::SetData(D,2,5) {} " 
		L"		 IRowsetUpdate::Undo(C) "
		L"			{ "
		L"			 L1.ROW_UNDOINSERT.OKTODO.RETURN_EOUTOFMEMORY "
		L"			 L1.ROW_UNDOINSERT.ABOUTTODO.RETURN_EOUTOFMEMORY "
		L"			 L1.ROW_UNDOINSERT.SYNCHAFTER.RETURN_EOUTOFMEMORY "
		L"			 L1.ROW_UNDOINSERT.DIDEVENT.RETURN_EOUTOFMEMORY "
		L"			} "
		L"		 IRowsetUpdate::Undo(D) "
		L"			{ "
		L"			 L1.ROW_UNDOINSERT.ABOUTTODO.RETURN_VETO "
		L"			} "
		L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(51)
//*-----------------------------------------------------------------------
// @mfunc ROW_UNDOINSERT - E_INVALIDARG - All Phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Undo::Variation_51()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::InsertRow(C) {} "
		L"		 IRowsetChange::InsertRow(D) {} "
		L"		 IRowsetChange::DeleteRows(A) {} "
		L"		 IRowsetChange::SetData(D,2,5) {} " 
		L"		 IRowsetUpdate::Undo(C) "
		L"			{ "
		L"			 L1.ROW_UNDOINSERT.OKTODO.RETURN_EINVALIDARG "
		L"			 L1.ROW_UNDOINSERT.ABOUTTODO.RETURN_EINVALIDARG "
		L"			 L1.ROW_UNDOINSERT.SYNCHAFTER.RETURN_EINVALIDARG "
		L"			 L1.ROW_UNDOINSERT.DIDEVENT.RETURN_EINVALIDARG "
		L"			} "
		L"		 IRowsetUpdate::Undo(D) "
		L"			{ "
		L"			 L1.ROW_UNDOINSERT.ABOUTTODO.RETURN_VETO "
		L"			} "
		L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(52)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Undo::Variation_52()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(53)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Undo::Variation_53()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(54)
//*-----------------------------------------------------------------------
// @mfunc ROW_UNDODELETE - Accept all phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Undo::Variation_54()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::InsertRow(C) {} "
		L"		 IRowsetChange::SetData(B,2,5) {} " 
		L"		 IRowsetChange::DeleteRows(A,B) {} "
		L"		 IRowsetChange::DeleteRows(C) {} "
		L"		 IRowsetUpdate::Undo(A) "
		L"			{ "
		L"			 L1.ROW_UNDODELETE.OKTODO.RETURN_ACCEPT "
		L"			} "
		L"		 IRowsetUpdate::Undo(A,B) {} "
		L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(55)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Undo::Variation_55()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(56)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Undo::Variation_56()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(57)
//*-----------------------------------------------------------------------
// @mfunc ROW_UNDODELETE - DB_E_CANCELED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Undo::Variation_57()
{
	//Veriy the veto was successful
	//Row should be unaffected by the Undo

    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::InsertRow(C) {} "
		L"		 IRowsetChange::SetData(B,2,5) {} " 
		L"		 IRowsetChange::DeleteRows(A,B) {} "
		L"		 IRowsetChange::DeleteRows(C) {} "
		L"		 IRowsetUpdate::Undo(A) "
		L"			{ "
		L"			 L1.ROW_UNDODELETE.ABOUTTODO.RETURN_VETO "
		L"			} "
		L"		 IRowsetChange::DeleteRows(A,DB_E_ERRORSOCCURRED) {} "
		L"		 IRowsetUpdate::Undo(A,B) {} "
		L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(58)
//*-----------------------------------------------------------------------
// @mfunc ROW_UNDODELETE - DB_E_NOTREENTRANT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Undo::Variation_58()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::InsertRow(C) {} "
		L"		 IRowsetChange::SetData(B,2,5) {} " 
		L"		 IRowsetChange::DeleteRows(A,B) {} "
		L"		 IRowsetChange::DeleteRows(C) {} "
		L"		 IRowsetUpdate::Undo(A) "
		L"			{ "
		L"			 L1.ROW_UNDODELETE.OKTODO.IRowsetUpdate::Undo(A,B,DB_E_NOTREENTRANT) {} "
		L"			} "
		L"		 IRowsetUpdate::Undo(A,B) {} "
		L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(59)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Undo::Variation_59()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(60)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Undo::Variation_60()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(61)
//*-----------------------------------------------------------------------
// @mfunc ROW_UNDODELETE - Veto OKTODO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Undo::Variation_61()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::InsertRow(C) {} "
		L"		 IRowsetChange::SetData(B,2,5) {} " 
		L"		 IRowsetChange::DeleteRows(A,B) {} "
		L"		 IRowsetChange::DeleteRows(C) {} "
		L"		 IRowsetUpdate::Undo(A) "
		L"			{ "
		L"			 L1.ROW_UNDODELETE.OKTODO.RETURN_VETO "
		L"			} "
		L"		 IRowsetUpdate::Undo(A,B) {} "
		L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(62)
//*-----------------------------------------------------------------------
// @mfunc ROW_UNDODELETE - Veto ABOUTTODO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Undo::Variation_62()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::InsertRow(C) {} "
		L"		 IRowsetChange::SetData(B,2,5) {} " 
		L"		 IRowsetChange::DeleteRows(A,B) {} "
		L"		 IRowsetChange::DeleteRows(C) {} "
		L"		 IRowsetUpdate::Undo(A) "
		L"			{ "
		L"			 L1.ROW_UNDODELETE.ABOUTTODO.RETURN_VETO "
		L"			} "
		L"		 IRowsetUpdate::Undo(A,B) {} "
		L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(63)
//*-----------------------------------------------------------------------
// @mfunc ROW_UNDODELETE - Veto SYNCHAFTER
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Undo::Variation_63()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::InsertRow(C) {} "
		L"		 IRowsetChange::SetData(B,2,5) {} " 
		L"		 IRowsetChange::DeleteRows(A,B) {} "
		L"		 IRowsetChange::DeleteRows(C) {} "
		L"		 IRowsetUpdate::Undo(A) "
		L"			{ "
		L"			 L1.ROW_UNDODELETE.SYNCHAFTER.RETURN_VETO "
		L"			} "
		L"		 IRowsetUpdate::Undo(A,B) {} "
		L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(64)
//*-----------------------------------------------------------------------
// @mfunc ROW_UNDODELETE - Veto DIDEVENT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Undo::Variation_64()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::InsertRow(C) {} "
		L"		 IRowsetChange::SetData(B,2,5) {} " 
		L"		 IRowsetChange::DeleteRows(A,B) {} "
		L"		 IRowsetChange::DeleteRows(C) {} "
		L"		 IRowsetUpdate::Undo(A) "
		L"			{ "
		L"			 L1.ROW_UNDODELETE.DIDEVENT.RETURN_VETO "
		L"			} "
		L"		 IRowsetUpdate::Undo(A,B) {} "
		L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(65)
//*-----------------------------------------------------------------------
// @mfunc ROW_UNDODELETE - Veto FAILEDTODO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Undo::Variation_65()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::InsertRow(C) {} "
		L"		 IRowsetChange::SetData(B,2,5) {} " 
		L"		 IRowsetChange::DeleteRows(A,B) {} "
		L"		 IRowsetChange::DeleteRows(C) {} "
		L"		 IRowsetUpdate::Undo(A) "
		L"			{ "
		L"			 L1.ROW_UNDODELETE.ABOUTTODO.RETURN_VETO "
		L"			 L1.ROW_UNDODELETE.FAILEDTODO.RETURN_VETO "
		L"			} "
		L"		 IRowsetUpdate::Undo(A,B) {} "
		L"		}"
		L"	}"
		L"}";
	
	//To produce FAILEDTODO, this reason/phase must be vetoable...
	TESTC_PROVIDER(IsCancelableEvent(DBREASON_ROW_UNDODELETE, DBEVENTPHASE_ABOUTTODO));
	TESTC(Execute(CommandString, CHANGE_BUFFERRED));

CLEANUP:
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(66)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Undo::Variation_66()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(67)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Undo::Variation_67()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(68)
//*-----------------------------------------------------------------------
// @mfunc ROW_UNDODELETE - UNWANTED REASON
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Undo::Variation_68()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::InsertRow(C) {} "
		L"		 IRowsetChange::SetData(B,2,5) {} " 
		L"		 IRowsetChange::DeleteRows(A,B) {} "
		L"		 IRowsetChange::DeleteRows(C) {} "
		L"		 IRowsetUpdate::Undo(A) "
		L"			{ "
		L"			 L1.ROW_UNDODELETE.OKTODO.RETURN_UNWANTEDREASON "
		L"			} "
		L"		 IRowsetUpdate::Undo(A,B) {} "
		L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(69)
//*-----------------------------------------------------------------------
// @mfunc ROW_UNDODELETE - UNWANTED PHASE OKTODO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Undo::Variation_69()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::InsertRow(C) {} "
		L"		 IRowsetChange::SetData(B,2,5) {} " 
		L"		 IRowsetChange::DeleteRows(A,B) {} "
		L"		 IRowsetChange::DeleteRows(C) {} "
		L"		 IRowsetUpdate::Undo(A) "
		L"			{ "
		L"			 L1.ROW_UNDODELETE.OKTODO.RETURN_UNWANTEDPHASE "
		L"			} "
		L"		 IRowsetUpdate::Undo(A,B) {} "
		L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(70)
//*-----------------------------------------------------------------------
// @mfunc ROW_UNDODELETE - UNWANTED PHASE ABOUTTODO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Undo::Variation_70()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::InsertRow(C) {} "
		L"		 IRowsetChange::SetData(B,2,5) {} " 
		L"		 IRowsetChange::DeleteRows(A,B) {} "
		L"		 IRowsetChange::DeleteRows(C) {} "
		L"		 IRowsetUpdate::Undo(A) "
		L"			{ "
		L"			 L1.ROW_UNDODELETE.ABOUTTODO.RETURN_UNWANTEDPHASE "
		L"			} "
		L"		 IRowsetUpdate::Undo(A,B) {} "
		L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(71)
//*-----------------------------------------------------------------------
// @mfunc ROW_UNDODELETE - UNWANTED PHASE SYNCHAFTER
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Undo::Variation_71()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::InsertRow(C) {} "
		L"		 IRowsetChange::SetData(B,2,5) {} " 
		L"		 IRowsetChange::DeleteRows(A,B) {} "
		L"		 IRowsetChange::DeleteRows(C) {} "
		L"		 IRowsetUpdate::Undo(A) "
		L"			{ "
		L"			 L1.ROW_UNDODELETE.SYNCHAFTER.RETURN_UNWANTEDPHASE "
		L"			} "
		L"		 IRowsetUpdate::Undo(A,B) {} "
		L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(72)
//*-----------------------------------------------------------------------
// @mfunc ROW_UNDODELETE - UNWANTED PHASE DIDEVENT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Undo::Variation_72()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::InsertRow(C) {} "
		L"		 IRowsetChange::SetData(B,2,5) {} " 
		L"		 IRowsetChange::DeleteRows(A,B) {} "
		L"		 IRowsetChange::DeleteRows(C) {} "
		L"		 IRowsetUpdate::Undo(A) "
		L"			{ "
		L"			 L1.ROW_UNDODELETE.DIDEVENT.RETURN_UNWANTEDPHASE "
		L"			} "
		L"		 IRowsetUpdate::Undo(A,B) {} "
		L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(73)
//*-----------------------------------------------------------------------
// @mfunc ROW_UNDODELETE - UNWANTED PHASE FAILEDTODO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Undo::Variation_73()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::InsertRow(C) {} "
		L"		 IRowsetChange::SetData(B,2,5) {} " 
		L"		 IRowsetChange::DeleteRows(A,B) {} "
		L"		 IRowsetChange::DeleteRows(C) {} "
		L"		 IRowsetUpdate::Undo(A) "
		L"			{ "
		L"			 L1.ROW_UNDODELETE.ABOUTTODO.RETURN_VETO "
		L"			 L1.ROW_UNDODELETE.FAILEDTODO.RETURN_UNWANTEDPHASE "
		L"			} "
		L"		 IRowsetUpdate::Undo(A,B) {} "
		L"		}"
		L"	}"
		L"}";
	
	//To produce FAILEDTODO, this reason/phase must be vetoable...
	TESTC_PROVIDER(IsCancelableEvent(DBREASON_ROW_UNDODELETE, DBEVENTPHASE_ABOUTTODO));
	TESTC(Execute(CommandString, CHANGE_BUFFERRED));

CLEANUP:
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(74)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Undo::Variation_74()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(75)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Undo::Variation_75()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(76)
//*-----------------------------------------------------------------------
// @mfunc ROW_UNDODELETE - E_FAIL - All Phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Undo::Variation_76()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::InsertRow(C) {} "
		L"		 IRowsetChange::SetData(B,2,5) {} " 
		L"		 IRowsetChange::DeleteRows(A,B) {} "
		L"		 IRowsetChange::DeleteRows(C) {} "
		L"		 IRowsetUpdate::Undo(A) "
		L"			{ "
		L"			 L1.ROW_UNDODELETE.OKTODO.RETURN_EFAIL "
		L"			 L1.ROW_UNDODELETE.ABOUTTODO.RETURN_EFAIL "
		L"			 L1.ROW_UNDODELETE.SYNCHAFTER.RETURN_EFAIL "
		L"			 L1.ROW_UNDODELETE.DIDEVENT.RETURN_EFAIL "
		L"			} "
		L"		 IRowsetUpdate::Undo(B) "
		L"			{ "
		L"			 L1.ROW_UNDODELETE.ABOUTTODO.RETURN_VETO "
		L"			} "
		L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(77)
//*-----------------------------------------------------------------------
// @mfunc ROW_UNDODELETE - E_OUTOFMEMORY - All Phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Undo::Variation_77()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::InsertRow(C) {} "
		L"		 IRowsetChange::SetData(B,2,5) {} " 
		L"		 IRowsetChange::DeleteRows(A,B) {} "
		L"		 IRowsetChange::DeleteRows(C) {} "
		L"		 IRowsetUpdate::Undo(A) "
		L"			{ "
		L"			 L1.ROW_UNDODELETE.OKTODO.RETURN_EOUTOFMEMORY "
		L"			 L1.ROW_UNDODELETE.ABOUTTODO.RETURN_EOUTOFMEMORY "
		L"			 L1.ROW_UNDODELETE.SYNCHAFTER.RETURN_EOUTOFMEMORY "
		L"			 L1.ROW_UNDODELETE.DIDEVENT.RETURN_EOUTOFMEMORY "
		L"			} "
		L"		 IRowsetUpdate::Undo(B) "
		L"			{ "
		L"			 L1.ROW_UNDODELETE.ABOUTTODO.RETURN_VETO "
		L"			} "
		L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(78)
//*-----------------------------------------------------------------------
// @mfunc ROW_UNDODELETE - E_INVALIDARG - All Phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Undo::Variation_78()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B) {} "
		L"		 IRowsetChange::InsertRow(C) {} "
		L"		 IRowsetChange::SetData(B,2,5) {} " 
		L"		 IRowsetChange::DeleteRows(A,B) {} "
		L"		 IRowsetChange::DeleteRows(C) {} "
		L"		 IRowsetUpdate::Undo(A) "
		L"			{ "
		L"			 L1.ROW_UNDODELETE.OKTODO.RETURN_EINVALIDARG "
		L"			 L1.ROW_UNDODELETE.ABOUTTODO.RETURN_EINVALIDARG "
		L"			 L1.ROW_UNDODELETE.SYNCHAFTER.RETURN_EINVALIDARG "
		L"			 L1.ROW_UNDODELETE.DIDEVENT.RETURN_EINVALIDARG "
		L"			} "
		L"		 IRowsetUpdate::Undo(B) "
		L"			{ "
		L"			 L1.ROW_UNDODELETE.ABOUTTODO.RETURN_VETO "
		L"			} "
		L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(79)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Undo::Variation_79()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(80)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Undo::Variation_80()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(81)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - Undo unchanged row
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Undo::Variation_81()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B,C) {} "
		L"		 IRowsetChange::InsertRow(D) {} "
		L"		 IRowsetChange::SetData(A,2,5) {} " 
		L"		 IRowsetChange::DeleteRows(B) {} "
		L"		 IRowsetUpdate::Undo(C) "
		L"			{ "
		L"			} "
		L"		 IRowsetUpdate::Undo(A) "
		L"			{ "
		L"			 L1.ROW_UNDOCHANGE.ABOUTTODO.RETURN_ACCEPT "
		L"			} "
		L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(82)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - Undo all rows
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Undo::Variation_82()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B,C) {} "
		L"		 IRowsetChange::InsertRow(D) {} "
		L"		 IRowsetChange::SetData(A,2,5) {} " 
		L"		 IRowsetChange::DeleteRows(B) {} "
		L"		 IRowsetUpdate::Undo() "
		L"			{ "
		L"			 L1.ROW_UNDOCHANGE.ABOUTTODO.RETURN_ACCEPT "
		L"			 L1.ROW_UNDODELETE.ABOUTTODO.RETURN_ACCEPT "
		L"			 L1.ROW_UNDOINSERT.ABOUTTODO.RETURN_ACCEPT "
		L"			} "
		L"		 IRowsetUpdate::Undo() "
		L"			{ "
		L"			} "
		L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(83)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - Undo changed and unchanged
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Undo::Variation_83()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B,C) {} "
		L"		 IRowsetChange::InsertRow(D) {} "
		L"		 IRowsetChange::SetData(A,2,5) {} " 
		L"		 IRowsetChange::DeleteRows(B) {} "
		L"		 IRowsetUpdate::Undo(A,B,C,D) "
		L"			{ "
		L"			 L1.ROW_UNDOCHANGE.ABOUTTODO.RETURN_ACCEPT "
		L"			 L1.ROW_UNDODELETE.ABOUTTODO.RETURN_ACCEPT "
		L"			 L1.ROW_UNDOINSERT.ABOUTTODO.RETURN_ACCEPT "
		L"			} "
		L"		 IRowsetUpdate::Undo() "
		L"			{ "
		L"			} "
		L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(84)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Undo::Variation_84()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(85)
//--------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - Verify Undo clears the FIRSTCHANGE status
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Undo::Variation_85()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B,C) {} "
		L"		 IRowsetChange::SetData(A,2) "
		L"			{"
		L"			 L1.ROW_FIRSTCHANGE.OKTODO.RETURN_ACCEPT "
		L"			}" 
		L"		 IRowsetUpdate::Undo(A) "
		L"			{ "
		L"			 L1.ROW_UNDOCHANGE.ABOUTTODO.RETURN_ACCEPT "
		L"			} "
		L"		 IRowsetChange::SetData(A,2) "
		L"			{"
		L"			 L1.ROW_FIRSTCHANGE.OKTODO.RETURN_ACCEPT "
		L"			}" 
		L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(86)
//--------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Undo::Variation_86()
{
	// TO DO:  Add your own code here
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(87)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - DBROWSTATUS_E_INVALID
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Undo::Variation_87()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B,C) {} "
		L"		 IRowsetChange::InsertRow(D) {} "
		L"		 IRowsetChange::SetData(A,2,5) {} " 
		L"		 IRowsetChange::DeleteRows(B) {} "
		L"		 IRowsetUpdate::Undo(E,DB_E_ERRORSOCCURRED) "
		L"			{ "
		L"			} "
		L"		 IRowsetUpdate::Undo() "
		L"			{ "
		L"			 L1.ROW_UNDOCHANGE.ABOUTTODO.RETURN_ACCEPT "
		L"			 L1.ROW_UNDODELETE.ABOUTTODO.RETURN_ACCEPT "
		L"			 L1.ROW_UNDOINSERT.ABOUTTODO.RETURN_ACCEPT "
		L"			} "
		L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(88)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - EmptyRowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Undo::Variation_88()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowsetUpdate::Undo() "
		L"			{ "
		L"			} "
		L"		 IRowsetUpdate::Undo() "
		L"			{ "
		L"			} "
		L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, EMPTY_ROWSET | CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(89)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - Add/Remove Listeners
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Undo::Variation_89()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B,C) {} "
		L"		 IRowsetChange::InsertRow(D) {} "
		L"		 IRowsetChange::SetData(A,2,5) {} " 
		L"		 IRowsetChange::DeleteRows(B) {} "
		L"		 IRowsetUpdate::Undo(A,B,C,D) "
		L"			{ "
		L"			 L1.ROW_UNDODELETE.ABOUTTODO.RETURN_ADVISE "
		L"			} "
		L"		 IRowsetUpdate::Undo() "
		L"			{ "
		L"			} "
		L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(90)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - GetData
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Undo::Variation_90()
{
//From the spec: in delayed update mode rows with pending deletes cannot be used in GetData
//Povider may return DB_E_DELETEDROW from GetData if hRow referred to a pending delete row, but
//providers are not required to check for this condition and result of passing an invalid row handle in hRow is undefined

//=>Commented out L1.ROW_UNDODELETE.ABOUTTODO.IRowset::GetData(B)
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B,C) {} "
		L"		 IRowsetChange::InsertRow(D) {} "
		L"		 IRowsetChange::SetData(A,2,5) {} " 
		L"		 IRowsetChange::DeleteRows(B) {} "
		L"		 IRowsetUpdate::Undo(A,B,C,D) "
		L"			{ "
		L"			 L1.ROW_UNDOCHANGE.OKTODO.IRowset::GetData(A) {} "
//		L"			 L1.ROW_UNDODELETE.ABOUTTODO.IRowset::GetData(B) {} "
		L"			 L1.ROW_UNDODELETE.SYNCHAFTER.IRowset::GetData(B) {} "
		L"			 L3.ROW_UNDOINSERT.ABOUTTODO.IRowset::GetData(D) {} "
		L"			} "
		L"		 IRowsetUpdate::Undo() "
		L"			{ "
		L"			} "
		L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(91)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - GetOriginalData
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Undo::Variation_91()
{
//Form the spec:
//GetOriginalData gets the data most recently fetched from or transmitted to the data store
//=> for newly inserted row in delayed update mode we can not call GetOriginalData untill it's transmitted 
//=> commented out L3.ROW_UNDOINSERT.ABOUTTODO.IRowsetUpdate::GetOriginalData(D,DB_E_NOTREENTRANT)	
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2; Listener L3; "
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B,C) {} "
		L"		 IRowsetChange::InsertRow(D) {} "
		L"		 IRowsetChange::SetData(A,2,5) {} " 
		L"		 IRowsetChange::DeleteRows(B) {} "
		L"		 IRowsetUpdate::Undo(A,B,C,D) "
		L"			{ "
		L"			 L1.ROW_UNDOCHANGE.OKTODO.IRowsetUpdate::GetOriginalData(A,DB_E_NOTREENTRANT) {} "
		L"			 L1.ROW_UNDODELETE.ABOUTTODO.IRowsetUpdate::GetOriginalData(B,DB_E_NOTREENTRANT) {} "
//		L"			 L3.ROW_UNDOINSERT.ABOUTTODO.IRowsetUpdate::GetOriginalData(D,DB_E_NOTREENTRANT) {} "
		L"			} "
		L"		 IRowsetUpdate::Undo() "
		L"			{ "
		L"			} "
		L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}
// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Undo::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CExecutionManager::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(Update)
//*-----------------------------------------------------------------------
//| Test Case:		Update - Tests Notifications for IRowsetUpdate::Update
//|	Created:			09/29/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Update::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CExecutionManager::Init())
	// }}
	{
		//Verify Rowset can be created...
		TEST_PROVIDER(CreateRowset(CHANGE_BUFFERRED)==S_OK);
		return TRUE;
	}

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc ROW_UPDATE - Accept all phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Update::Variation_1()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B,C) {} "
		L"		 IRowsetChange::InsertRow(D) {} "
		L"		 IRowsetChange::SetData(A,2,5) {} " 
		L"		 IRowsetChange::SetData(B,2,5) {} " 
		L"		 IRowsetChange::DeleteRows(C) {} "
		L"		 IRowsetUpdate::Update(A,B,C) "
		L"			{"
		L"			 L1.ROW_UPDATE.OKTODO.RETURN_ACCEPT "
		L"			}"
		L"		 IRowsetUpdate::Update(A,D) {} "
		L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Update::Variation_2()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Update::Variation_3()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc ROW_UPDATE - DB_E_CANCELED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Update::Variation_4()
{
	//Veriy the veto was successful
	//Row should be unaffected by the Update

	//Make sure Veto of the first setdata, doesn't affect the Update
	//of the second setdata

    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B,C) {} "
		L"		 IRowsetChange::InsertRow(D) {} "
		L"		 IRowsetChange::SetData(A,2,5) {} " 
		L"		 IRowsetChange::SetData(B,2,5) {} " 
		L"		 IRowsetChange::DeleteRows(C) {} "
		L"		 IRowsetUpdate::Update(A,B,C) "
		L"			{"
		L"			 L1.ROW_UPDATE.ABOUTTODO.RETURN_VETO "
		L"			}"
		L"		 IRowsetUpdate::GetOriginalData(A) {} "
		L"		 IRowsetUpdate::Update(D) {} "
		L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc ROW_UPDATE - DB_E_NOTREENTRANT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Update::Variation_5()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B,C) {} "
		L"		 IRowsetChange::InsertRow(D) {} "
		L"		 IRowsetChange::SetData(A,2,5) {} " 
		L"		 IRowsetChange::SetData(B,2,5) {} " 
		L"		 IRowsetChange::DeleteRows(C) {} "
		L"		 IRowsetUpdate::Update(A,B,C) "
		L"			{"
		L"			 L1.ROW_UPDATE.OKTODO.IRowsetUpdate::Update(A,B,C,DB_E_NOTREENTRANT) {} "
		L"			}"
		L"		 IRowsetUpdate::Update(D) {} "
		L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Update::Variation_6()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Update::Variation_7()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc ROW_UPDATE - Veto OKTODO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Update::Variation_8()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B,C) {} "
		L"		 IRowsetChange::InsertRow(D) {} "
		L"		 IRowsetChange::SetData(A,2,5) {} " 
		L"		 IRowsetChange::SetData(B,2,5) {} " 
		L"		 IRowsetChange::DeleteRows(C) {} "
		L"		 IRowsetUpdate::Update(A,B,C) "
		L"			{"
		L"			 L1.ROW_UPDATE.OKTODO.RETURN_VETO "
		L"			}"
		L"		 IRowsetUpdate::Update(D) {} "
		L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc ROW_UPDATE - Veto ABOUTTODO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Update::Variation_9()
{
	//Make sure Veto of the first insert, doesn't affect the Update
	//of the second insert

    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B,C) {} "
		L"		 IRowsetChange::InsertRow(D) {} "
		L"		 IRowsetChange::InsertRow(E) {} "
		L"		 IRowsetChange::SetData(A,2,5) {} " 
		L"		 IRowsetChange::DeleteRows(C) {} "
		L"		 IRowsetUpdate::Update(D,E,A) "
		L"			{"
		L"			 L1.ROW_UPDATE.ABOUTTODO.RETURN_VETO "
		L"			}"
		L"		 IRowsetUpdate::Update(C) {} "
		L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc ROW_UPDATE - Veto SYNCHAFTER
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Update::Variation_10()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B,C) {} "
		L"		 IRowsetChange::InsertRow(D) {} "
		L"		 IRowsetChange::SetData(A,2,5) {} " 
		L"		 IRowsetChange::SetData(B,2,5) {} " 
		L"		 IRowsetChange::DeleteRows(C) {} "
		L"		 IRowsetUpdate::Update(A,B,C) "
		L"			{"
		L"			 L1.ROW_UPDATE.SYNCHAFTER.RETURN_VETO "
		L"			}"
		L"		 IRowsetUpdate::Update(D) {} "
		L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc ROW_UPDATE - Veto DIDEVENT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Update::Variation_11()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B,C) {} "
		L"		 IRowsetChange::InsertRow(D) {} "
		L"		 IRowsetChange::SetData(A,2,5) {} " 
		L"		 IRowsetChange::SetData(B,2,5) {} " 
		L"		 IRowsetChange::DeleteRows(C) {} "
		L"		 IRowsetUpdate::Update(A,B,C) "
		L"			{"
		L"			 L1.ROW_UPDATE.DIDEVENT.RETURN_VETO "
		L"			}"
		L"		 IRowsetUpdate::Update(D) {} "
		L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc ROW_UPDATE - Veto FAILEDTODO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Update::Variation_12()
{
	//Make sure Veto of the first delete, doesn't affect the Update
	//of the second delete
    
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B,C) {} "
		L"		 IRowsetChange::InsertRow(D) {} "
		L"		 IRowsetChange::SetData(A,2,5) {} " 
		L"		 IRowsetChange::DeleteRows(B) {} "
		L"		 IRowsetChange::DeleteRows(C) {} "
		L"		 IRowsetUpdate::Update(B,C,A) "
		L"			{"
		L"			 L1.ROW_UPDATE.ABOUTTODO.RETURN_VETO "
		L"			 L1.ROW_UPDATE.FAILEDTODO.RETURN_VETO "
		L"			}"
		L"		 IRowsetUpdate::Update(D) {} "
		L"		}"
		L"	}"
		L"}";
	
	//To produce FAILEDTODO, this reason/phase must be vetoable...
	TESTC_PROVIDER(IsCancelableEvent(DBREASON_ROW_UPDATE, DBEVENTPHASE_ABOUTTODO));
	TESTC(Execute(CommandString, CHANGE_BUFFERRED));

CLEANUP:
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Update::Variation_13()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Update::Variation_14()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc ROW_UPDATE - UNWANTED REASON
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Update::Variation_15()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B,C) {} "
		L"		 IRowsetChange::InsertRow(D) {} "
		L"		 IRowsetChange::SetData(A,2,5) {} " 
		L"		 IRowsetChange::SetData(B,2,5) {} " 
		L"		 IRowsetChange::DeleteRows(C) {} "
		L"		 IRowsetUpdate::Update(A,B,C) "
		L"			{"
		L"			 L1.ROW_UPDATE.OKTODO.RETURN_UNWANTEDREASON "
		L"			}"
		L"		 IRowsetUpdate::Update(D) {} "
		L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc ROW_UPDATE - UNWANTED PHASE - OKTODO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Update::Variation_16()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B,C) {} "
		L"		 IRowsetChange::InsertRow(D) {} "
		L"		 IRowsetChange::SetData(A,2,5) {} " 
		L"		 IRowsetChange::SetData(B,2,5) {} " 
		L"		 IRowsetChange::DeleteRows(C) {} "
		L"		 IRowsetUpdate::Update(A,B,C) "
		L"			{"
		L"			 L1.ROW_UPDATE.OKTODO.RETURN_UNWANTEDPHASE "
		L"			}"
		L"		 IRowsetUpdate::Update(D) {} "
		L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc ROW_UPDATE - UNWANTED PHASE - ABOUTTODO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Update::Variation_17()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B,C) {} "
		L"		 IRowsetChange::InsertRow(D) {} "
		L"		 IRowsetChange::SetData(A,2,5) {} " 
		L"		 IRowsetChange::SetData(B,2,5) {} " 
		L"		 IRowsetChange::DeleteRows(C) {} "
		L"		 IRowsetUpdate::Update(A,B,C) "
		L"			{"
		L"			 L1.ROW_UPDATE.ABOUTTODO.RETURN_UNWANTEDPHASE "
		L"			}"
		L"		 IRowsetUpdate::Update(D) {} "
		L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc ROW_UPDATE - UNWANTED PHASE - SYNCHAFTER
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Update::Variation_18()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B,C) {} "
		L"		 IRowsetChange::InsertRow(D) {} "
		L"		 IRowsetChange::SetData(A,2,5) {} " 
		L"		 IRowsetChange::SetData(B,2,5) {} " 
		L"		 IRowsetChange::DeleteRows(C) {} "
		L"		 IRowsetUpdate::Update(A,B,C) "
		L"			{"
		L"			 L1.ROW_UPDATE.SYNCHAFTER.RETURN_UNWANTEDPHASE "
		L"			}"
		L"		 IRowsetUpdate::Update(D) {} "
		L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc ROW_UPDATE - UNWANTED PHASE - DIDEVENT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Update::Variation_19()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B,C) {} "
		L"		 IRowsetChange::InsertRow(D) {} "
		L"		 IRowsetChange::SetData(A,2,5) {} " 
		L"		 IRowsetChange::SetData(B,2,5) {} " 
		L"		 IRowsetChange::DeleteRows(C) {} "
		L"		 IRowsetUpdate::Update(A,B,C) "
		L"			{"
		L"			 L1.ROW_UPDATE.DIDEVENT.RETURN_UNWANTEDPHASE "
		L"			}"
		L"		 IRowsetUpdate::Update(D) {} "
		L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc ROW_UPDATE - UNWANTED PHASE - FAILEDTODO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Update::Variation_20()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B,C) {} "
		L"		 IRowsetChange::InsertRow(D) {} "
		L"		 IRowsetChange::SetData(A,2,5) {} " 
		L"		 IRowsetChange::SetData(B,2,5) {} " 
		L"		 IRowsetChange::DeleteRows(C) {} "
		L"		 IRowsetUpdate::Update(A,B,C) "
		L"			{"
		L"			 L1.ROW_UPDATE.ABOUTTODO.RETURN_VETO "
		L"			 L1.ROW_UPDATE.FAILEDTODO.RETURN_UNWANTEDPHASE "
		L"			}"
		L"		 IRowsetUpdate::Update(D) {} "
		L"		}"
		L"	}"
		L"}";
	
	//To produce FAILEDTODO, this reason/phase must be vetoable...
	TESTC_PROVIDER(IsCancelableEvent(DBREASON_ROW_UPDATE, DBEVENTPHASE_ABOUTTODO));
	TESTC(Execute(CommandString, CHANGE_BUFFERRED));

CLEANUP:
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Update::Variation_21()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(22)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Update::Variation_22()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(23)
//*-----------------------------------------------------------------------
// @mfunc ROW_UPDATE - E_FAIL - All Phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Update::Variation_23()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B,C) {} "
		L"		 IRowsetChange::InsertRow(D) {} "
		L"		 IRowsetChange::SetData(A,2,5) {} " 
		L"		 IRowsetChange::SetData(B,2,5) {} " 
		L"		 IRowsetChange::DeleteRows(C) {} "
		L"		 IRowsetUpdate::Update(A,B) "
		L"			{"
		L"			 L1.ROW_UPDATE.OKTODO.RETURN_EFAIL "
		L"			 L1.ROW_UPDATE.ABOUTTODO.RETURN_EFAIL "
		L"			 L1.ROW_UPDATE.SYNCHAFTER.RETURN_EFAIL "
		L"			 L1.ROW_UPDATE.DIDEVENT.RETURN_EFAIL "
		L"			}"
		L"		 IRowsetUpdate::Update(C,D) "
		L"			{"
		L"			 L1.ROW_UPDATE.ABOUTTODO.RETURN_VETO "
		L"			}"
		L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(24)
//*-----------------------------------------------------------------------
// @mfunc ROW_UPDATE - E_OUTOFMEMORY - All Phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Update::Variation_24()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B,C) {} "
		L"		 IRowsetChange::InsertRow(D) {} "
		L"		 IRowsetChange::SetData(A,2,5) {} " 
		L"		 IRowsetChange::SetData(B,2,5) {} " 
		L"		 IRowsetChange::DeleteRows(C) {} "
		L"		 IRowsetUpdate::Update(A,B) "
		L"			{"
		L"			 L1.ROW_UPDATE.OKTODO.RETURN_EOUTOFMEMORY  "
		L"			 L1.ROW_UPDATE.ABOUTTODO.RETURN_EOUTOFMEMORY  "
		L"			 L1.ROW_UPDATE.SYNCHAFTER.RETURN_EOUTOFMEMORY  "
		L"			 L1.ROW_UPDATE.DIDEVENT.RETURN_EOUTOFMEMORY  "
		L"			}"
		L"		 IRowsetUpdate::Update(C,D) "
		L"			{"
		L"			 L1.ROW_UPDATE.ABOUTTODO.RETURN_VETO "
		L"			}"
		L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(25)
//*-----------------------------------------------------------------------
// @mfunc ROW_UPDATE - E_INVALIDARG - All Phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Update::Variation_25()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B,C) {} "
		L"		 IRowsetChange::InsertRow(D) {} "
		L"		 IRowsetChange::SetData(A,2,5) {} " 
		L"		 IRowsetChange::SetData(B,2,5) {} " 
		L"		 IRowsetChange::DeleteRows(C) {} "
		L"		 IRowsetUpdate::Update(A,B) "
		L"			{"
		L"			 L1.ROW_UPDATE.OKTODO.RETURN_EINVALIDARG "
		L"			 L1.ROW_UPDATE.ABOUTTODO.RETURN_EINVALIDARG "
		L"			 L1.ROW_UPDATE.SYNCHAFTER.RETURN_EINVALIDARG "
		L"			 L1.ROW_UPDATE.DIDEVENT.RETURN_EINVALIDARG "
		L"			}"
		L"		 IRowsetUpdate::Update(C,D) "
		L"			{"
		L"			 L1.ROW_UPDATE.ABOUTTODO.RETURN_VETO "
		L"			}"
		L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(26)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Update::Variation_26()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(27)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Update::Variation_27()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(28)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - Update all [no changes]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Update::Variation_28()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B,C) {} "
		L"		 IRowsetUpdate::Update() "
		L"			{"
		L"			}"
		L"		 IRowsetUpdate::Update(A,B,C) "
		L"			{"
		L"			}"
		L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(29)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - Update all [changes]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Update::Variation_29()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B,C) {} "
		L"		 IRowsetChange::InsertRow(D) {} "
		L"		 IRowsetChange::SetData(A,2,5) {} " 
		L"		 IRowsetChange::SetData(B,2,5) {} " 
		L"		 IRowsetChange::DeleteRows(C) {} "
		L"		 IRowsetUpdate::Update(D) "
		L"			{"
		L"			 L1.ROW_UPDATE.OKTODO.RETURN_ACCEPT "
		L"			}"
		L"		 IRowsetUpdate::Update(D) "
		L"			{"
		L"			}"
		L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(30)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - Update changed and unchanged
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Update::Variation_30()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B,C) {} "
		L"		 IRowsetChange::InsertRow(D) {} "
		L"		 IRowsetChange::SetData(A,2,5) {} " 
		L"		 IRowsetChange::SetData(B,2,5) {} " 
		L"		 IRowsetChange::DeleteRows(C) {} "
		L"		 IRowsetUpdate::Update(A,B,C) "
		L"			{"
		L"			 L1.ROW_UPDATE.OKTODO.RETURN_ACCEPT "
		L"			}"
		L"		 IRowsetUpdate::Update(A,B,D) "
		L"			{"
		L"			 L1.ROW_UPDATE.OKTODO.RETURN_ACCEPT "
		L"			}"
		L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(31)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Update::Variation_31()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(32)
//--------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - Verify Update clears FIRSTCHANGE status
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Update::Variation_32()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B,C) {} "
		L"		 IRowsetChange::SetData(A,2) "
		L"			{"
		L"			 L1.ROW_FIRSTCHANGE.OKTODO.RETURN_ACCEPT "
		L"			}" 
		L"		 IRowsetUpdate::Update(A) "
		L"			{"
		L"			 L1.ROW_UPDATE.OKTODO.RETURN_ACCEPT "
		L"			}"
		L"		 IRowsetChange::SetData(A,2) "
		L"			{"
		L"			 L1.ROW_FIRSTCHANGE.OKTODO.RETURN_ACCEPT "
		L"			}" 
		L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(33)
//--------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Update::Variation_33()
{
	// TO DO:  Add your own code here
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(34)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - DBROWSTATUS_E_INVALID
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Update::Variation_34()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B,C) {} "
		L"		 IRowsetChange::InsertRow(D) {} "
		L"		 IRowsetChange::SetData(A,2,5) {} " 
		L"		 IRowsetChange::SetData(B,2,5) {} " 
		L"		 IRowsetChange::DeleteRows(C) {} "
		L"		 IRowsetUpdate::Update(E,DB_E_ERRORSOCCURRED) "
		L"			{"
		L"			}"
		L"		 IRowsetUpdate::Update(C,D) "
		L"			{"
		L"			 L1.ROW_UPDATE.OKTODO.RETURN_ACCEPT "
		L"			}"
		L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(35)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - EmptyRowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Update::Variation_35()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowsetUpdate::Update() "
		L"			{"
		L"			}"
		L"		 IRowsetUpdate::Update() "
		L"			{"
		L"			}"
		L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, EMPTY_ROWSET | CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(36)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - Add/Remove Listeners
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Update::Variation_36()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B,C) {} "
		L"		 IRowsetChange::InsertRow(D) {} "
		L"		 IRowsetChange::SetData(A,2,5) {} " 
		L"		 IRowsetChange::SetData(B,2,5) {} " 
		L"		 IRowsetChange::DeleteRows(C) {} "
		L"		 IRowsetUpdate::Update(A,B) "
		L"			{"
		L"			 L1.ROW_UPDATE.OKTODO.RETURN_ADVISE "
		L"			}"
		L"		 IRowsetUpdate::Update(C,D) "
		L"			{"
		L"			 L1.ROW_UPDATE.OKTODO.RETURN_UNADVISE "
		L"			}"
		L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(37)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - GetData
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Update::Variation_37()
{
    WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B,C) {} "
		L"		 IRowsetChange::InsertRow(D) {} "
		L"		 IRowsetChange::SetData(A,2,5) {} " 
		L"		 IRowsetChange::SetData(B,2,5) {} " 
		L"		 IRowsetChange::DeleteRows(C) {} "
		L"		 IRowsetUpdate::Update(A,B,C) "
		L"			{"
		L"			 L1.ROW_UPDATE.OKTODO.IRowset::GetData(A) {} "
		L"			 L1.ROW_UPDATE.OKTODO.IRowset::GetData(B) {} "
		L"			}"
		L"		 IRowsetUpdate::Update(D) "
		L"			{"
		L"			 L1.ROW_UPDATE.OKTODO.IRowset::GetData(D) {} "
		L"			}"
		L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}


// {{ TCW_VAR_PROTOTYPE(38)
//*-----------------------------------------------------------------------
// @mfunc OTHER SCENARIOS - GetOriginalData
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Update::Variation_38()
{
//Form the spec:
//GetOriginalData "gets the data most recently fetched from or transmitted to the data store;
//does not get values based on pending changes"
//=> for newly inserted row in delayed update mode we can not call GetOriginalData until it's transmitted 
//=> commented out L1.ROW_UPDATE.OKTODO.IRowsetUpdate::GetOriginalData(D,DB_E_NOTREENTRANT)	
	WCHAR CommandString[] = 
		L"{Listener L1; Listener L2;"
		L"	{Control Control1;"
		L"		{"
		L"		 IRowset::GetNextRows(A,B,C) {} "
		L"		 IRowsetChange::InsertRow(D) {} "
		L"		 IRowsetChange::SetData(A,2,5) {} " 
		L"		 IRowsetChange::SetData(B,2,5) {} " 
		L"		 IRowsetChange::DeleteRows(C) {} "
		L"		 IRowsetUpdate::Update(A,B) "
		L"			{"
		L"			 L1.ROW_UPDATE.OKTODO.IRowsetUpdate::GetOriginalData(A,DB_E_NOTREENTRANT) {} "
		L"			 L1.ROW_UPDATE.OKTODO.IRowsetUpdate::GetOriginalData(B,DB_E_NOTREENTRANT) {} "
		L"			}"
		L"		 IRowsetUpdate::Update(D) "
		L"			{"
//		L"			 L1.ROW_UPDATE.OKTODO.IRowsetUpdate::GetOriginalData(D,DB_E_NOTREENTRANT) {} "
		L"			}"
		L"		}"
		L"	}"
		L"}";
	
	return Execute(CommandString, CHANGE_BUFFERRED);
}
// }}
// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Update::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CExecutionManager::Terminate());
}	// }}
// }}
// }}


