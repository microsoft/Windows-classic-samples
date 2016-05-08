//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright (C) 1995-2000 Microsoft Corporation
//
// @doc 
//
// @module IRowPos.cpp | Tests for the IRowPosition and IRowPositionChange interfaces.
//


/////////////////////////////////////////////////////////////////////////
// Includes
//
/////////////////////////////////////////////////////////////////////////
#include "modstandard.hpp"

#include "IRowPos.h"					//The Header file.
#include "ExtraLib.h"					//Usefull functions that we use.
#include "olectl.h"						//CONNECT_E_NOCONNECT


/////////////////////////////////////////////////////////////////////////
// Defines
//
/////////////////////////////////////////////////////////////////////////
#define MAX_LISTENERS	3


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Module Values
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// {{ TCW_MODULE_GLOBALS
DECLARE_MODULE_CLSID = { 0x0fd29ae1, 0xe1f4, 0x11d0, { 0x94, 0x46, 0x00, 0xc0, 0x4f, 0xd7, 0x05, 0xe9 }};
DECLARE_MODULE_NAME("IRowPosition");
DECLARE_MODULE_OWNER("Microsoft");
DECLARE_MODULE_DESCRIP("IRowPosition holds the current row on a Rowset");
DECLARE_MODULE_VERSION(795921705);
// TCW_WizardVersion(2)
// TCW_Automation(False)
// }} TCW_MODULE_GLOBALS_END

//--------------------------------------------------------------------
// @func Module level initialization routine
//
// @rdesc Success or Failure
// 		@flag  TRUE  | Successful initialization
//		@flag  FALSE | Initialization problems
//
BOOL ModuleInit(CThisTestModule * pThisTestModule)
{
	return CommonModuleInit(pThisTestModule, IID_IRowset, FIVE_ROWS);
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
	return CommonModuleTerminate(pThisTestModule);
}	


////////////////////////////////////////////////////////////////////////////
//  TCRowPosition
//
////////////////////////////////////////////////////////////////////////////
class TCRowPosition : public CRowset
{
public:
	//Constructors
	TCRowPosition(WCHAR* pwszTestCaseName = INVALID(WCHAR*));
    virtual  ~TCRowPosition();

	//methods
	virtual BOOL Init();
	virtual BOOL Terminate();

	//helpers	
	static  HRESULT CreateRowPosition(IUnknown* pIRowset, IRowPosition** ppIRowPosition = NULL);
	
	static	HRESULT Initialize(IRowPosition* pIRowPosition, IUnknown* pIRowset);
	static	HRESULT ClearRowPosition(IRowPosition* pIRowPosition);
	static	HRESULT SetRowPosition(IRowPosition* pIRowPosition, HCHAPTER hChapter, HROW hRow, DBPOSITIONFLAGS dwPosFlags); 
	static	HRESULT GetRowPosition(IRowPosition* pIRowPosition, HCHAPTER* phChapter, HROW* phRow, DBPOSITIONFLAGS* pdwPosFlags);
	static	HRESULT GetRowset(IRowPosition* pIRowPosition, REFIID riid, IUnknown** ppIRowset);

	virtual BOOL	VerifyGetRowPosition(IRowPosition* pIRowPosition, HCHAPTER hChapter, HROW hRow, DBPOSITIONFLAGS dwPosFlags);
	virtual BOOL	VerifySetRowPosition(IRowPosition* pIRowPosition, HCHAPTER hChapter, HROW hRow, DBPOSITIONFLAGS dwPosFlags);
	
	//Connection Points
	virtual IConnectionPoint*			const	pICP();
	virtual IConnectionPointContainer*	const 	pICPC();
	virtual IEnumConnectionPoints*		const	pIEnumCP();
	virtual IRowPosition*				const	pIRowPos();

	//Notifications
	virtual BOOL	VerifyDiffCookies(ULONG cCookies, DWORD* rgCookies) const;
	virtual BOOL	AdviseAll(ULONG* rgCookies, IRowPosition* pIRowPosition = NULL);
	virtual BOOL	UnadviseAll(ULONG* rgCookies, IRowPosition* pIRowPosition = NULL);
		
	virtual BOOL	RestartListeners();
	virtual BOOL	ResetTimesNotified();
	virtual BOOL	VerifyTimesNotified(ULONG cTimes, DBREASON eReason = DBREASON_ALL, DBEVENTPHASE ePhase = DBEVENTPHASE_ALL);
	virtual BOOL    VerifyFailedTodo(DBREASON eReason, DBEVENTPHASE ePhase);

	//Enumerations
	virtual BOOL	VerifyConnectData(ULONG cConnections, CONNECTDATA* rgConnectData, ULONG cCookies, IUnknown** rgpListeners, DWORD* rgCookies);
	virtual BOOL	FreeConnectData(ULONG cConnections, CONNECTDATA* rgConnectData);

	//Thread Functions
	static ULONG WINAPI Thread_CreateRowPosition(LPVOID pv);
	static ULONG WINAPI Thread_Initialize(LPVOID pv);
	static ULONG WINAPI Thread_GetRowset(LPVOID pv);
	static ULONG WINAPI Thread_GetRowPosition(LPVOID pv);
	static ULONG WINAPI Thread_ClearSetGetCombo(LPVOID pv);

public:
	//data
	DBCOUNTITEM m_ulTotalRows;

protected:
	//data
	IConnectionPoint* 			m_pIConnectionPoint;
	IConnectionPointContainer*	m_pIConnectionPointContainer;
	IEnumConnectionPoints*		m_pIEnumConnectionPoints;

	IRowPosition*				m_pIRowPosition;

	//Listeners
	ULONG		m_cListeners;
	CListener*	m_rgpListeners[MAX_LISTENERS];
};


TCRowPosition::TCRowPosition(WCHAR* pwszTestCaseName) : CRowset(pwszTestCaseName)
{
	//Connection
	m_pIConnectionPoint				= NULL;
	m_pIConnectionPointContainer	= NULL;
	m_pIEnumConnectionPoints		= NULL;

	m_pIRowPosition = NULL;
	m_ulTotalRows = 0;	//the number of rows that we use right now.		

	m_cListeners = 0;
}

TCRowPosition::~TCRowPosition()
{
}

BOOL TCRowPosition::Init()
{
	TBEGIN
	ULONG i=0;
		
	//Initialize inherited object 
	TESTC(CRowset::Init());

	//Create Rowset object
	TESTC_(CreateRowset(DBPROP_CANHOLDROWS),S_OK);
	m_ulTotalRows = GetTotalRows();

	//Create RowPosition object
	TESTC_(CreateRowPosition(pIRowset(), &m_pIRowPosition),S_OK);
	
	//Obtain the connection point container
	TESTC_((QI(m_pIRowPosition,IID_IConnectionPointContainer,(void**)&m_pIConnectionPointContainer)),S_OK);

	//Obtain the IRowsetNotify connection point 
	TESTC_(m_pIConnectionPointContainer->FindConnectionPoint(IID_IRowPositionChange, &m_pIConnectionPoint),S_OK);

	//Obtain the IEnumConnectionPoints 
	TESTC_(m_pIConnectionPointContainer->EnumConnectionPoints(&m_pIEnumConnectionPoints),S_OK);

	//Create Listeners
	m_cListeners = MAX_LISTENERS;
	for(i=0; i<m_cListeners; i++)
	{
		m_rgpListeners[i] = new CListener(IID_IRowPositionChange, m_pIRowPosition);
		SAFE_ADDREF(m_rgpListeners[i]);
	}

CLEANUP:
	TRETURN
}

BOOL TCRowPosition::Terminate()
{
	//Remove Listeners
	for(ULONG i=0; i<m_cListeners; i++)
	{
		SAFE_RELEASE(m_rgpListeners[i]);
	}

	SAFE_RELEASE(m_pIConnectionPoint);
	SAFE_RELEASE(m_pIConnectionPointContainer);
	SAFE_RELEASE(m_pIEnumConnectionPoints);

	//Release all interfaces
	SAFE_RELEASE(m_pIRowPosition);
	return CRowset::Terminate();
}

HRESULT TCRowPosition::CreateRowPosition(IUnknown* pIRowset, IRowPosition** ppIRowPosition)
{
	HRESULT hr = E_FAIL;
	IRowPosition* pIRowPosition = NULL;
	
	//CreateInstance of IRowPosition
	hr = CoCreateInstance(CLSID_OLEDB_ROWPOSITIONLIBRARY, NULL, CLSCTX_INPROC_SERVER, IID_IRowPosition, (void**)&pIRowPosition);
	if(SUCCEEDED(hr))
	{
		TESTC(pIRowPosition != NULL);
	}
	else
	{
		TESTC(pIRowPosition == NULL);
	}

	//Initialize
	QTESTC_(hr = Initialize(pIRowPosition, pIRowset),S_OK);

CLEANUP:
	if(ppIRowPosition)
		*ppIRowPosition = pIRowPosition;
	else
		SAFE_RELEASE(pIRowPosition);
	return hr;
}

HRESULT TCRowPosition::Initialize(IRowPosition* pIRowPosition, IUnknown* pIRowset)
{
	TBEGIN
	ASSERT(pIRowPosition);
	return pIRowPosition->Initialize(pIRowset);
}

HRESULT TCRowPosition::ClearRowPosition(IRowPosition* pIRowPosition)
{
	TBEGIN
	ASSERT(pIRowPosition);
	return pIRowPosition->ClearRowPosition();
}

HRESULT TCRowPosition::GetRowset(IRowPosition* pIRowPosition, REFIID riid, IUnknown** ppIRowset)
{
	TBEGIN
	HRESULT hr = S_OK;
	ASSERT(pIRowPosition);
	
	hr = pIRowPosition->GetRowset(riid, ppIRowset);
	if(ppIRowset)
	{
		if(SUCCEEDED(hr))
		{
			TESTC(*ppIRowset != NULL);
		}
		else
		{
			TESTC(*ppIRowset == NULL);
		}
	}

CLEANUP:
	return hr;
}

HRESULT TCRowPosition::SetRowPosition(IRowPosition* pIRowPosition, HCHAPTER hChapter, HROW hRow, DBPOSITIONFLAGS dwPosFlags)
{
	TBEGIN
	ASSERT(pIRowPosition);
	HRESULT hr = E_FAIL;
	IRowset* pIRowset = NULL;

	//SetRowPosition
	hr = pIRowPosition->SetRowPosition(hChapter, hRow, dwPosFlags);

	//Need to release the extra row handle reference (if successful)
	if(SUCCEEDED(hr) && hRow != DB_NULL_HROW)
	{
		//GetRowset
		TESTC_(GetRowset(pIRowPosition, IID_IRowset, (IUnknown**)&pIRowset),S_OK);
		TESTC_(pIRowset->ReleaseRows(1, &hRow, NULL, NULL, NULL),S_OK);
	}

CLEANUP:
	SAFE_RELEASE(pIRowset);
	return hr; 
}

HRESULT TCRowPosition::GetRowPosition(IRowPosition* pIRowPosition, HCHAPTER* phChapter, HROW* phRow, DBPOSITIONFLAGS* pdwPosFlags)
{
	TBEGIN
	ASSERT(pIRowPosition);
	HRESULT hr = E_FAIL;
	IRowset* pIRowset = NULL;

	//GetRowPosition
	hr = pIRowPosition->GetRowPosition(phChapter, phRow, pdwPosFlags);

	//Need to release the extra row handle reference (if successful)
	if(SUCCEEDED(hr) && phRow && *phRow != DB_NULL_HROW)
	{
		//GetRowset
		TESTC_(GetRowset(pIRowPosition, IID_IRowset, (IUnknown**)&pIRowset),S_OK);
		TESTC_(pIRowset->ReleaseRows(1, phRow, NULL, NULL, NULL),S_OK);
	}

CLEANUP:
	SAFE_RELEASE(pIRowset);
	return hr; 
}

BOOL TCRowPosition::VerifyGetRowPosition(IRowPosition* pIRowPosition, HCHAPTER hChapter, HROW hRow, DBPOSITIONFLAGS dwPosFlags)
{
	TBEGIN
	ASSERT(pIRowPosition);
	HRESULT hr = E_FAIL;

	HCHAPTER hChapter2			= INVALID(HCHAPTER);
	HROW	 hRow2				= INVALID(HROW);
	DBPOSITIONFLAGS dwPosFlags2 = INVALID(DBPOSITIONFLAGS);

	//GetRowPosition (delegate)
	TESTC_(hr = GetRowPosition(pIRowPosition, &hChapter2, &hRow2, &dwPosFlags2),S_OK);

	//Verify results
	TESTC(hChapter	== hChapter2);
	TESTC(hRow		== hRow2);
	TESTC(dwPosFlags== dwPosFlags2);

CLEANUP:
	TRETURN 
}

BOOL TCRowPosition::VerifySetRowPosition(IRowPosition* pIRowPosition, HCHAPTER hChapter, HROW hRow, DBPOSITIONFLAGS dwPosFlags)
{
	TBEGIN
	ASSERT(pIRowPosition);
	HRESULT hr = E_FAIL;

	//SetRowPosition (delegate)
	TESTC_(hr = SetRowPosition(pIRowPosition, hChapter, hRow, dwPosFlags),S_OK);

	//Delegate to our Get Method
	TESTC(VerifyGetRowPosition(pIRowPosition, hChapter, hRow, dwPosFlags));

CLEANUP:
	TRETURN 
}

IRowPosition* const TCRowPosition::pIRowPos()
{
	ASSERT(m_pIRowPosition);
	return m_pIRowPosition;
}

IConnectionPointContainer* const TCRowPosition::pICPC()
{
	ASSERT(m_pIConnectionPointContainer);
	return m_pIConnectionPointContainer;
}

IConnectionPoint* const TCRowPosition::pICP()
{
	ASSERT(m_pIConnectionPoint);
	return m_pIConnectionPoint;
}

IEnumConnectionPoints* const TCRowPosition::pIEnumCP()
{
	ASSERT(m_pIEnumConnectionPoints);
	return m_pIEnumConnectionPoints;
}

BOOL TCRowPosition::VerifyDiffCookies(ULONG cCookies, DWORD* rgCookie) const
{
	//NOTE: When calling this method you should only be checking Cookies from
	//the same Container, since Cookies may not be unqiue accross Containers.

	//verify different connection cookies
	for(ULONG i=0; i<cCookies; i++)
		for(ULONG j=i+1; j<cCookies; j++)
			if(rgCookie[i] == rgCookie[j])
				return FALSE;
			
	return TRUE;
}

BOOL TCRowPosition::AdviseAll(ULONG* rgCookies, IRowPosition* pIRowPosition)
{
	TBEGIN
	if(pIRowPosition== NULL)
		pIRowPosition = m_pIRowPosition;
	ASSERT(rgCookies);
	ASSERT(pIRowPosition);

	//Advise all Listeners to the IRowPosition object
	for(ULONG i=0; i<m_cListeners; i++)
	{
		TESTC(m_rgpListeners[i]->Restart());
		TESTC_(m_rgpListeners[i]->Advise(&rgCookies[i], pIRowPosition),S_OK);
		TESTC(rgCookies[i] != 0);
	}

	//Make sure all Cookies are unique
	TESTC(VerifyDiffCookies(m_cListeners, rgCookies));

CLEANUP:
	TRETURN	
}
	
BOOL TCRowPosition::UnadviseAll(ULONG* rgCookies, IRowPosition* pIRowPosition)
{
	TBEGIN
	if(pIRowPosition== NULL)
		pIRowPosition = m_pIRowPosition;
	ASSERT(rgCookies);
	ASSERT(pIRowPosition);

	//Unadvise all Listeners from the IRowPosition object
	for(ULONG i=0; i<m_cListeners; i++)
	{
		TESTC(rgCookies[i] != 0);
		TESTC_(m_rgpListeners[i]->Unadvise(rgCookies[i], pIRowPosition),S_OK);
	}

CLEANUP:
	TRETURN	
}

BOOL TCRowPosition::VerifyTimesNotified(ULONG cTimes, DBREASON eReason, DBEVENTPHASE ePhase)
{
	TBEGIN
	
	//Determine the number of times notified for this notification
	for(ULONG i=0; i<m_cListeners; i++)
	{
		QTESTC(m_rgpListeners[i]->GetTimesNotified(eReason, ePhase) == cTimes);
	}

CLEANUP:
	TRETURN
}

BOOL TCRowPosition::RestartListeners()
{
	TBEGIN
	
	//Determine the number of times notified for this notification
	for(ULONG i=0; i<m_cListeners; i++)
	{
		TESTC(m_rgpListeners[i]->Restart());
	}

CLEANUP:
	TRETURN
}

BOOL TCRowPosition::ResetTimesNotified()
{
	TBEGIN
	
	//Determine the number of times notified for this notification
	for(ULONG i=0; i<m_cListeners; i++)
	{
		TESTC(m_rgpListeners[i]->ResetTimesNotified());
	}

CLEANUP:
	TRETURN
}

BOOL TCRowPosition::VerifyFailedTodo(DBREASON eReason, DBEVENTPHASE ePhase)
{
	TBEGIN
	ULONG i=0;

	//This is kind of a difficult method.
	//We need to verify that there was a FAILEDTODO on the particular reason/phase
	//The problem is the we also need to verify all previous phases.
	//The interesting issue, is that depending upon the order, (who sent the FAILEDTODO)
	//Some listeners may have received more notiifcations than others.
	
	//Also if it was the OKTODO phase, some listeners may never receive a FAILEDTODO
	//phase, since they were never notified of an event to begin with...

	//First just verify that everyone did in fact see a FAILEDTODO
	if(ePhase == DBEVENTPHASE_OKTODO)
	{
		//If started receiveing notifications, then they should have FAILEDTODO
		for(i=0; i<m_cListeners; i++)
		{
			if(m_rgpListeners[i]->GetTimesNotified(eReason))
			{
				TESTC(m_rgpListeners[i]->GetTimesNotified(eReason, DBEVENTPHASE_FAILEDTODO)==1);
			}
		}
	}
	else
	{
		//Simple, just verify everyone saw a FAILEDTODO
		TESTC(VerifyTimesNotified(1, eReason, DBEVENTPHASE_FAILEDTODO));
	}

	//Now verify the listeners received all phases upto the FAILEDTODO
	//For the first Notifications...
	switch(ePhase)
	{
		case DBEVENTPHASE_ABOUTTODO:
			TESTC(VerifyTimesNotified(1, DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_OKTODO));
			break;

		case DBEVENTPHASE_SYNCHAFTER:
			TESTC(VerifyTimesNotified(1, DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_OKTODO));
			TESTC(VerifyTimesNotified(1, DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_ABOUTTODO));
			break;

		case DBEVENTPHASE_DIDEVENT:
		case DBEVENTPHASE_FAILEDTODO:
			TESTC(VerifyTimesNotified(1, DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_OKTODO));
			TESTC(VerifyTimesNotified(1, DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_ABOUTTODO));
			TESTC(VerifyTimesNotified(1, DBREASON_ROWPOSITION_CHANGED, DBEVENTPHASE_SYNCHAFTER));
			break;
	};

CLEANUP:
	TRETURN

}

BOOL TCRowPosition::VerifyConnectData(ULONG cConnectData, CONNECTDATA* rgConnectData, ULONG cCookies, IUnknown** rgpListeners, DWORD* rgCookies)
{
	ASSERT(rgConnectData);
	ASSERT(rgpListeners);
	ASSERT(rgCookies);

	
	//Need to verify rgConnectData array contains the correct information
	//NOTE: rgConnectData doesn't have to be ordered direclty with the order
	//that the connections were established.
	//NOTE: this function assumes that rgCookies is in the same order as rgpListeners.

	ULONG ulFound = 0;

	//Loop over the ConnectData
	for(ULONG i=0; i<cConnectData; i++)
	{
		//Must at least be in the valid range for Sink/Cookie!
		if(rgConnectData[i].pUnk == NULL || rgConnectData[i].dwCookie == 0)
			return FALSE;
		
		//Try to find the cookie in the list of cookies.
		//Once the cookie is found, lookup the corresponding listener
		for(ULONG j=0; j<cCookies; j++)
		{
			if(rgConnectData[i].dwCookie==rgCookies[j])
			{
				if(!VerifyEqualInterface(rgConnectData[i].pUnk, rgpListeners[j]))
					return FALSE;

				ulFound++;
			}
		}
	}

	return ulFound == cConnectData;
}


BOOL TCRowPosition::FreeConnectData(ULONG cConnectData, CONNECTDATA* rgConnectData)
{
	//Loop over the ConnectData
	for(ULONG i=0; i<cConnectData; i++)
	{
		SAFE_RELEASE(rgConnectData[i].pUnk);
	}

	return TRUE;
}

ULONG TCRowPosition::Thread_CreateRowPosition(LPVOID pv)
{
	THREAD_BEGIN
	CRowset*		pThis			= (TCRowPosition*)THREAD_FUNC;
	ASSERT(pThis);

	//Local stack variables
	IRowPosition* pIRowPosition = NULL;

	ThreadSwitch(); //Let the other thread(s) catch up

	//CreateInstance
	TESTC_(CreateRowPosition(pThis->pIRowset(), &pIRowPosition),S_OK);
	
	ThreadSwitch(); //Let the other thread(s) catch up

CLEANUP:
	SAFE_RELEASE(pIRowPosition);
	THREAD_RETURN
}

ULONG TCRowPosition::Thread_Initialize(LPVOID pv)
{
	THREAD_BEGIN
	TCRowPosition*	pThis			= (TCRowPosition*)THREAD_FUNC;
	IRowPosition*	pIRowPosition	= (IRowPosition*)THREAD_ARG1;
	ASSERT(pThis && pIRowPosition);
	HRESULT hr = E_FAIL;

	ThreadSwitch(); //Let the other thread(s) catch up

	hr = Initialize(pIRowPosition, pThis->pIRowset());
	TESTC(hr==S_OK || hr==DB_E_ALREADYINITIALIZED);

	ThreadSwitch(); //Let the other thread(s) catch up

CLEANUP:
	THREAD_RETURN
}

ULONG TCRowPosition::Thread_GetRowset(LPVOID pv)
{
	THREAD_BEGIN
	TCRowPosition*	pThis			= (TCRowPosition*)THREAD_FUNC;
	IRowPosition*	pIRowPosition	= (IRowPosition*)THREAD_ARG1;
	ASSERT(pThis && pIRowPosition);
	IRowset* pIRowset = NULL;

	ThreadSwitch(); //Let the other thread(s) catch up

	TESTC_(GetRowset(pIRowPosition, IID_IRowset, (IUnknown**)&pIRowset),S_OK);
	
	ThreadSwitch(); //Let the other thread(s) catch up

CLEANUP:
	SAFE_RELEASE(pIRowset);
	THREAD_RETURN
}


ULONG TCRowPosition::Thread_GetRowPosition(LPVOID pv)
{
	THREAD_BEGIN
	TCRowPosition*	pThis			= (TCRowPosition*)THREAD_FUNC;
	IRowPosition*	pIRowPosition	= (IRowPosition*)THREAD_ARG1;
	ASSERT(pThis && pIRowPosition);
	HRESULT hr = S_OK;

	HCHAPTER hChapter = NULL;
	HROW hRow = NULL;
	DBPOSITIONFLAGS dwPositionFlags = 0;

	ThreadSwitch(); //Let the other thread(s) catch up

	//GetRowPosition
	TESTC_(hr = GetRowPosition(pIRowPosition, &hChapter, &hRow, &dwPositionFlags),S_OK);

	//TODO what should I be checking here for results?

	ThreadSwitch(); //Let the other thread(s) catch up

CLEANUP:
	THREAD_RETURN
}


ULONG TCRowPosition::Thread_ClearSetGetCombo(LPVOID pv)
{
	THREAD_BEGIN
	TCRowPosition*	pThis			= (TCRowPosition*)THREAD_FUNC;
	IRowPosition*	pIRowPosition	= (IRowPosition*)THREAD_ARG1;
	HROW*			phRow			= (HROW*)THREAD_ARG2;
	ASSERT(pThis && pIRowPosition && phRow && *phRow);
	
	HRESULT hr = S_OK;
	HROW hRow =	 *phRow;
	HCHAPTER hChapter = NULL;
	DBPOSITIONFLAGS dwPositionFlags;		

	ThreadSwitch(); //Let the other thread(s) catch up

	//ClearRowPosition
	hr = ClearRowPosition(pIRowPosition);

	//SetRowPosition
	hr = SetRowPosition(pIRowPosition, NULL, hRow, DBPOSITION_OK);
		 
	//GetRowPosition
	hr = GetRowPosition(pIRowPosition, &hChapter, &hRow, &dwPositionFlags);
	
	ThreadSwitch(); //Let the other thread(s) catch up
	THREAD_RETURN
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Test Case Section
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


// {{ TCW_TEST_CASE_MAP(TCInitialize)
//--------------------------------------------------------------------
// @class Initializes the IRowPosition
//
class TCInitialize : public TCRowPosition { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCInitialize,TCRowPosition);
	// }} TCW_DECLARE_FUNCS_END
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember General - Call Initialize with all valid rowset interfaces
	int Variation_1();
	// @cmember General - Call Initialize with something that does not support IRowset
	int Variation_2();
	// @cmember Empty
	int Variation_3();
	// @cmember Boundary - Initialize with a null pointer
	int Variation_4();
	// @cmember Boundary - Call with an IDispatch
	int Variation_5();
	// @cmember Boundary - Other non rowset objects
	int Variation_6();
	// @cmember Empty
	int Variation_7();
	// @cmember Sequence - Call Initialize twice in a row
	int Variation_8();
	// @cmember Sequence - Call Initialize the second time with an IUnknown
	int Variation_9();
	// @cmember Empty
	int Variation_10();
	// @cmember Stress - Initialize with 3 IRowPositions to the same SourceRowset
	int Variation_11();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCInitialize)
#define THE_CLASS TCInitialize
BEG_TEST_CASE(TCInitialize, TCRowPosition, L"Initializes the IRowPosition")
	TEST_VARIATION(1, 		L"General - Call Initialize with all valid rowset interfaces")
	TEST_VARIATION(2, 		L"General - Call Initialize with something that does not support IRowset")
	TEST_VARIATION(3, 		L"Empty")
	TEST_VARIATION(4, 		L"Boundary - Initialize with a null pointer")
	TEST_VARIATION(5, 		L"Boundary - Call with an IDispatch")
	TEST_VARIATION(6, 		L"Boundary - Other non rowset objects")
	TEST_VARIATION(7, 		L"Empty")
	TEST_VARIATION(8, 		L"Sequence - Call Initialize twice in a row")
	TEST_VARIATION(9, 		L"Sequence - Call Initialize the second time with an IUnknown")
	TEST_VARIATION(10, 		L"Empty")
	TEST_VARIATION(11, 		L"Stress - Initialize with 3 IRowPositions to the same SourceRowset")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCGetRowset)
//--------------------------------------------------------------------
// @class Returns the current underlying Rowset
//
class TCGetRowset : public TCRowPosition { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCGetRowset,TCRowPosition);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember General - After Initialization, call GetRowset with riid valid.
	int Variation_1();
	// @cmember General - Call GetRowset with IUnknown
	int Variation_2();
	// @cmember Empty
	int Variation_3();
	// @cmember Boundary - Before initialization, call GetRowset with riid null.
	int Variation_4();
	// @cmember Boundary - Before initialization call GetRowset with riid valid
	int Variation_5();
	// @cmember Boundary - After Initialization, call GetRowset with riid invalid
	int Variation_6();
	// @cmember Boundary - Make sure the rowset was AddReffed, by killing the IRowPos, and making sure the Rowset is still valid.
	int Variation_7();
	// @cmember Empty
	int Variation_8();
	// @cmember Related - Initialize another IRowPosition with GetRowset from the first one
	int Variation_9();
	// @cmember Empty
	int Variation_10();
	// @cmember Multithreaded - CoCreateInstance, Initialize
	int Variation_11();
	// @cmember Multithreaded - Initialize
	int Variation_12();
	// @cmember Multithreaded - GetRowset
	int Variation_13();
	// @cmember Multithreaded - GetRowPosition
	int Variation_14();
	// @cmember Multithreaded - Clear,Set,Get RowPosition Combo
	int Variation_15();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCGetRowset)
#define THE_CLASS TCGetRowset
BEG_TEST_CASE(TCGetRowset, TCRowPosition, L"Returns the current underlying Rowset")
	TEST_VARIATION(1, 		L"General - After Initialization, call GetRowset with riid valid.")
	TEST_VARIATION(2, 		L"General - Call GetRowset with IUnknown")
	TEST_VARIATION(3, 		L"Empty")
	TEST_VARIATION(4, 		L"Boundary - Before initialization, call GetRowset with riid null.")
	TEST_VARIATION(5, 		L"Boundary - Before initialization call GetRowset with riid valid")
	TEST_VARIATION(6, 		L"Boundary - After Initialization, call GetRowset with riid invalid")
	TEST_VARIATION(7, 		L"Boundary - Make sure the rowset was AddReffed, by killing the IRowPos, and making sure the Rowset is still valid.")
	TEST_VARIATION(8, 		L"Empty")
	TEST_VARIATION(9, 		L"Related - Initialize another IRowPosition with GetRowset from the first one")
	TEST_VARIATION(10, 		L"Empty")
	TEST_VARIATION(11, 		L"Multithreaded - CoCreateInstance, Initialize")
	TEST_VARIATION(12, 		L"Multithreaded - Initialize")
	TEST_VARIATION(13, 		L"Multithreaded - GetRowset")
	TEST_VARIATION(14, 		L"Multithreaded - GetRowPosition")
	TEST_VARIATION(15, 		L"Multithreaded - Clear,Set,Get RowPosition Combo")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCClearRowPosition)
//--------------------------------------------------------------------
// @class Clears the value of the current hRow
//
class TCClearRowPosition : public TCRowPosition { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCClearRowPosition,TCRowPosition);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember General - Call ClearRowPosition after a call to SetRowPosition
	int Variation_1();
	// @cmember General - Call ClearRowPosition directly after an Init.
	int Variation_2();
	// @cmember Empty
	int Variation_3();
	// @cmember Boundary - Call ClearRowPosition before any calls to Init
	int Variation_4();
	// @cmember Empty
	int Variation_5();
	// @cmember Sequence - Call ClearRowPosition twice in a row
	int Variation_6();
	// @cmember Sequence - Call ClearRowPosition, then a GetRowPosition
	int Variation_7();
	// @cmember Sequence - Call ClearRowPosition, then a GetnextRows
	int Variation_8();
	// @cmember Empty
	int Variation_9();
	// @cmember Multiuser - ClearRowPosition should not affect other IRowPosition's
	int Variation_10();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCClearRowPosition)
#define THE_CLASS TCClearRowPosition
BEG_TEST_CASE(TCClearRowPosition, TCRowPosition, L"Clears the value of the current hRow")
	TEST_VARIATION(1, 		L"General - Call ClearRowPosition after a call to SetRowPosition")
	TEST_VARIATION(2, 		L"General - Call ClearRowPosition directly after an Init.")
	TEST_VARIATION(3, 		L"Empty")
	TEST_VARIATION(4, 		L"Boundary - Call ClearRowPosition before any calls to Init")
	TEST_VARIATION(5, 		L"Empty")
	TEST_VARIATION(6, 		L"Sequence - Call ClearRowPosition twice in a row")
	TEST_VARIATION(7, 		L"Sequence - Call ClearRowPosition, then a GetRowPosition")
	TEST_VARIATION(8, 		L"Sequence - Call ClearRowPosition, then a GetnextRows")
	TEST_VARIATION(9, 		L"Empty")
	TEST_VARIATION(10, 		L"Multiuser - ClearRowPosition should not affect other IRowPosition's")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCSetRowPosition)
//--------------------------------------------------------------------
// @class Sets the current row position
//
class TCSetRowPosition : public TCRowPosition { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCSetRowPosition,TCRowPosition);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember General - SetRowPosition to an hRow in the middle of many hRows
	int Variation_1();
	// @cmember General - SetRowPosition to an hRow in the middle of many hRows with DBPOSITION_NOROW
	int Variation_2();
	// @cmember General - test all the invalid flag values on valid hRows
	int Variation_3();
	// @cmember Empty
	int Variation_4();
	// @cmember Boundary - SetRowPosition to null with no rows and DBPOSITION_OK
	int Variation_5();
	// @cmember Boundary - SetRowPosition to null with no rows and DBPOSITION_NOROW
	int Variation_6();
	// @cmember Boundary - SetRowPosition to null with no rows and DBPOSITION_BOF
	int Variation_7();
	// @cmember Boundary - SetRowPosition to null with no rows and DBPOSITION_EOF
	int Variation_8();
	// @cmember Boundary - SetRowPosition to some hRow with no rows
	int Variation_9();
	// @cmember Boundary - SetRowPosition to null with one row and DBPOSITION_OK
	int Variation_10();
	// @cmember Boundary - SetRowPosition to null with one row and DBPOSITION_NOROW
	int Variation_11();
	// @cmember Boundary - SetRowPosition to null with one row and DBPOSITION_BOF
	int Variation_12();
	// @cmember Boundary - SetRowPosition to the last row of many
	int Variation_13();
	// @cmember Boundary - SetRowPosition to the first row of many
	int Variation_14();
	// @cmember Boundary - SetRowPosition to an invalid (nonexistent
	int Variation_15();
	// @cmember Boundary - SetRowPosition with a null row handle
	int Variation_16();
	// @cmember Boundary - SetRowPosition with a deleted row handle
	int Variation_17();
	// @cmember Empty
	int Variation_18();
	// @cmember Sequence - Call a valid SetRowPosition in a multi-row scenario after a valid SetRowPosition
	int Variation_19();
	// @cmember Empty
	int Variation_20();
	// @cmember Multiuser - SetRowPosition with different IRowPositions but with the same sourcerowset
	int Variation_21();
	// @cmember Multiuser - SetRowPosition with different IRowPositions and with different sourcerowsets
	int Variation_22();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCSetRowPosition)
#define THE_CLASS TCSetRowPosition
BEG_TEST_CASE(TCSetRowPosition, TCRowPosition, L"Sets the current row position")
	TEST_VARIATION(1, 		L"General - SetRowPosition to an hRow in the middle of many hRows")
	TEST_VARIATION(2, 		L"General - SetRowPosition to an hRow in the middle of many hRows with DBPOSITION_NOROW")
	TEST_VARIATION(3, 		L"General - test all the invalid flag values on valid hRows")
	TEST_VARIATION(4, 		L"Empty")
	TEST_VARIATION(5, 		L"Boundary - SetRowPosition to null with no rows and DBPOSITION_OK")
	TEST_VARIATION(6, 		L"Boundary - SetRowPosition to null with no rows and DBPOSITION_NOROW")
	TEST_VARIATION(7, 		L"Boundary - SetRowPosition to null with no rows and DBPOSITION_BOF")
	TEST_VARIATION(8, 		L"Boundary - SetRowPosition to null with no rows and DBPOSITION_EOF")
	TEST_VARIATION(9, 		L"Boundary - SetRowPosition to some hRow with no rows")
	TEST_VARIATION(10, 		L"Boundary - SetRowPosition to null with one row and DBPOSITION_OK")
	TEST_VARIATION(11, 		L"Boundary - SetRowPosition to null with one row and DBPOSITION_NOROW")
	TEST_VARIATION(12, 		L"Boundary - SetRowPosition to null with one row and DBPOSITION_BOF")
	TEST_VARIATION(13, 		L"Boundary - SetRowPosition to the last row of many")
	TEST_VARIATION(14, 		L"Boundary - SetRowPosition to the first row of many")
	TEST_VARIATION(15, 		L"Boundary - SetRowPosition to an invalid (nonexistent")
	TEST_VARIATION(16, 		L"Boundary - SetRowPosition with a null row handle")
	TEST_VARIATION(17, 		L"Boundary - SetRowPosition with a deleted row handle")
	TEST_VARIATION(18, 		L"Empty")
	TEST_VARIATION(19, 		L"Sequence - Call a valid SetRowPosition in a multi-row scenario after a valid SetRowPosition")
	TEST_VARIATION(20, 		L"Empty")
	TEST_VARIATION(21, 		L"Multiuser - SetRowPosition with different IRowPositions but with the same sourcerowset")
	TEST_VARIATION(22, 		L"Multiuser - SetRowPosition with different IRowPositions and with different sourcerowsets")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCGetRowPosition)
//--------------------------------------------------------------------
// @class Returns the current row possition
//
class TCGetRowPosition : public TCRowPosition { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCGetRowPosition,TCRowPosition);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember General - GetRowPosition with one row, hRow set.
	int Variation_1();
	// @cmember General - Make sure it AddRefs the hRows.
	int Variation_2();
	// @cmember General - GetRowPosition immediately after an Initialize.
	int Variation_3();
	// @cmember General - GetRowPosition before an Initialize.
	int Variation_4();
	// @cmember General - GetRowPosition after a ClearRowPosition.
	int Variation_5();
	// @cmember Empty
	int Variation_6();
	// @cmember Parameters - GetRowPosition with hChapter NULL
	int Variation_7();
	// @cmember Parameters - GetRowPosition with phRow NULL
	int Variation_8();
	// @cmember Parameters - GetRowPosition with pdwPositionFlags NULL
	int Variation_9();
	// @cmember Parameters - GetRowPosition with all params NULL
	int Variation_10();
	// @cmember Empty
	int Variation_11();
	// @cmember Multiuser - Call GetNextRows with multiple IRowPositions.
	int Variation_12();
	// @cmember Empty
	int Variation_13();
	// @cmember Related - Make sure GetnextRows is not being interfered with
	int Variation_14();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCGetRowPosition)
#define THE_CLASS TCGetRowPosition
BEG_TEST_CASE(TCGetRowPosition, TCRowPosition, L"Returns the current row possition")
	TEST_VARIATION(1, 		L"General - GetRowPosition with one row, hRow set.")
	TEST_VARIATION(2, 		L"General - Make sure it AddRefs the hRows.")
	TEST_VARIATION(3, 		L"General - GetRowPosition immediately after an Initialize.")
	TEST_VARIATION(4, 		L"General - GetRowPosition before an Initialize.")
	TEST_VARIATION(5, 		L"General - GetRowPosition after a ClearRowPosition.")
	TEST_VARIATION(6, 		L"Empty")
	TEST_VARIATION(7, 		L"Parameters - GetRowPosition with hChapter NULL")
	TEST_VARIATION(8, 		L"Parameters - GetRowPosition with phRow NULL")
	TEST_VARIATION(9, 		L"Parameters - GetRowPosition with pdwPositionFlags NULL")
	TEST_VARIATION(10, 		L"Parameters - GetRowPosition with all params NULL")
	TEST_VARIATION(11, 		L"Empty")
	TEST_VARIATION(12, 		L"Multiuser - Call GetNextRows with multiple IRowPositions.")
	TEST_VARIATION(13, 		L"Empty")
	TEST_VARIATION(14, 		L"Related - Make sure GetnextRows is not being interfered with")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCAddRelease)
//--------------------------------------------------------------------
// @class Testing AddRef, Release, and QI
//
class TCAddRelease : public TCRowPosition { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCAddRelease,TCRowPosition);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember AddRef - Make sure it increments
	int Variation_1();
	// @cmember AddRef - 100 times, and make sure it increments
	int Variation_2();
	// @cmember AddRef - and Release combo 100 times
	int Variation_3();
	// @cmember Empty
	int Variation_4();
	// @cmember Release - Make sure it decrements
	int Variation_5();
	// @cmember Release - 100 times and make sure it decrements
	int Variation_6();
	// @cmember Empty
	int Variation_7();
	// @cmember QI - for IRowPosition
	int Variation_8();
	// @cmember QI - for someting non-existent
	int Variation_9();
	// @cmember QI - for IUnknown
	int Variation_10();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCAddRelease)
#define THE_CLASS TCAddRelease
BEG_TEST_CASE(TCAddRelease, TCRowPosition, L"Testing AddRef, Release, and QI")
	TEST_VARIATION(1, 		L"AddRef - Make sure it increments")
	TEST_VARIATION(2, 		L"AddRef - 100 times, and make sure it increments")
	TEST_VARIATION(3, 		L"AddRef - and Release combo 100 times")
	TEST_VARIATION(4, 		L"Empty")
	TEST_VARIATION(5, 		L"Release - Make sure it decrements")
	TEST_VARIATION(6, 		L"Release - 100 times and make sure it decrements")
	TEST_VARIATION(7, 		L"Empty")
	TEST_VARIATION(8, 		L"QI - for IRowPosition")
	TEST_VARIATION(9, 		L"QI - for someting non-existent")
	TEST_VARIATION(10, 		L"QI - for IUnknown")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCOnRowPositionChange)
//--------------------------------------------------------------------
// @class Functions that are called when a row changes
//
class TCOnRowPositionChange : public TCRowPosition { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCOnRowPositionChange,TCRowPosition);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember General - return S_OK
	int Variation_1();
	// @cmember General - return S_FALSE
	int Variation_2();
	// @cmember General - return S_FALSE on an fCantDeny.
	int Variation_3();
	// @cmember General - return DB_S_UNWANTEDPHASE
	int Variation_4();
	// @cmember General - return DB_S_UNWANTEDREASON
	int Variation_5();
	// @cmember General - return E_FAIL
	int Variation_6();
	// @cmember General - return E_OUTOFMEMORY
	int Variation_7();
	// @cmember Empty
	int Variation_8();
	// @cmember Test in all four phases
	int Variation_9();
	// @cmember Empty
	int Variation_10();
	// @cmember Sequence - Advise inbetween seperate notifications
	int Variation_11();
	// @cmember Sequence - Advise during a notification
	int Variation_12();
	// @cmember Sequence - Unadvise inbetween seperate notification
	int Variation_13();
	// @cmember Sequence - Unadvise during a notification
	int Variation_14();
	// @cmember Empty
	int Variation_15();
	// @cmember SetRowPosition - Error Cases - FAILEDTODO
	int Variation_16();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCOnRowPositionChange)
#define THE_CLASS TCOnRowPositionChange
BEG_TEST_CASE(TCOnRowPositionChange, TCRowPosition, L"Functions that are called when a row changes")
	TEST_VARIATION(1, 		L"General - return S_OK")
	TEST_VARIATION(2, 		L"General - return S_FALSE")
	TEST_VARIATION(3, 		L"General - return S_FALSE on an fCantDeny.")
	TEST_VARIATION(4, 		L"General - return DB_S_UNWANTEDPHASE")
	TEST_VARIATION(5, 		L"General - return DB_S_UNWANTEDREASON")
	TEST_VARIATION(6, 		L"General - return E_FAIL")
	TEST_VARIATION(7, 		L"General - return E_OUTOFMEMORY")
	TEST_VARIATION(8, 		L"Empty")
	TEST_VARIATION(9, 		L"Test in all four phases")
	TEST_VARIATION(10, 		L"Empty")
	TEST_VARIATION(11, 		L"Sequence - Advise inbetween seperate notifications")
	TEST_VARIATION(12, 		L"Sequence - Advise during a notification")
	TEST_VARIATION(13, 		L"Sequence - Unadvise inbetween seperate notification")
	TEST_VARIATION(14, 		L"Sequence - Unadvise during a notification")
	TEST_VARIATION(15, 		L"Empty")
	TEST_VARIATION(16, 		L"SetRowPosition - Error Cases - FAILEDTODO")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCConnectionStuff)
//--------------------------------------------------------------------
// @class Testing all the connection routines
//
class TCConnectionStuff : public TCRowPosition { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCConnectionStuff,TCRowPosition);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember General: Container: QI for IID_IConnectionPointContainer
	int Variation_1();
	// @cmember Container: AddRef
	int Variation_2();
	// @cmember Container: Release
	int Variation_3();
	// @cmember Container: FindConnectionPoint for a connection
	int Variation_4();
	// @cmember Container EnumConnectionPoints
	int Variation_5();
	// @cmember Container: Next should work until there are no more
	int Variation_6();
	// @cmember Container: Clone
	int Variation_7();
	// @cmember Container: Reset should go to the beginning
	int Variation_8();
	// @cmember Container: The Second one should still be on the end
	int Variation_9();
	// @cmember Container: Skip should skip one
	int Variation_10();
	// @cmember Connections: EnumConnections
	int Variation_11();
	// @cmember Connections: Next should work until there are no more
	int Variation_12();
	// @cmember Connections: Clone
	int Variation_13();
	// @cmember Conenctions: Reset should go to the beginning
	int Variation_14();
	// @cmember Connections: Skip should skip one
	int Variation_15();
	// @cmember Point: GetConnectionInterface should return succesful
	int Variation_16();
	// @cmember Point: GetConnectionPointContainer should work with a valid pointer
	int Variation_17();
	// @cmember B/NULL: Container: QI for IID_NULL
	int Variation_18();
	// @cmember Container: FindConnectionPoint for IID_NULL
	int Variation_19();
	// @cmember Container: FindConnectionPoint for ppCP is NULL
	int Variation_20();
	// @cmember Enum: Next with a rgpcd NULL
	int Variation_21();
	// @cmember Enum: Skip on the second one should still be at the end
	int Variation_22();
	// @cmember Container: Enum should fail for a NULL pointer
	int Variation_23();
	// @cmember Enum: reset twice in a row
	int Variation_24();
	// @cmember Enum: clone where ppIEnum is NULL
	int Variation_25();
	// @cmember Point: GetConectionInterface with pIID NULL
	int Variation_26();
	// @cmember Point: GetConectionPointContainer with ppCPC null
	int Variation_27();
	// @cmember Point: Unadvise before advising should fail
	int Variation_28();
	// @cmember Point: advise with pointers NULL
	int Variation_29();
	// @cmember point: Unadvise with pointers NULL
	int Variation_30();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCConnectionStuff)
#define THE_CLASS TCConnectionStuff
BEG_TEST_CASE(TCConnectionStuff, TCRowPosition, L"Testing all the connection routines")
	TEST_VARIATION(1, 		L"General: Container: QI for IID_IConnectionPointContainer")
	TEST_VARIATION(2, 		L"Container: AddRef")
	TEST_VARIATION(3, 		L"Container: Release")
	TEST_VARIATION(4, 		L"Container: FindConnectionPoint for a connection")
	TEST_VARIATION(5, 		L"Container EnumConnectionPoints")
	TEST_VARIATION(6, 		L"Container: Next should work until there are no more")
	TEST_VARIATION(7, 		L"Container: Clone")
	TEST_VARIATION(8, 		L"Container: Reset should go to the beginning")
	TEST_VARIATION(9, 		L"Container: The Second one should still be on the end")
	TEST_VARIATION(10, 		L"Container: Skip should skip one")
	TEST_VARIATION(11, 		L"Connections: EnumConnections")
	TEST_VARIATION(12, 		L"Connections: Next should work until there are no more")
	TEST_VARIATION(13, 		L"Connections: Clone")
	TEST_VARIATION(14, 		L"Conenctions: Reset should go to the beginning")
	TEST_VARIATION(15, 		L"Connections: Skip should skip one")
	TEST_VARIATION(16, 		L"Point: GetConnectionInterface should return succesful")
	TEST_VARIATION(17, 		L"Point: GetConnectionPointContainer should work with a valid pointer")
	TEST_VARIATION(18, 		L"B/NULL: Container: QI for IID_NULL")
	TEST_VARIATION(19, 		L"Container: FindConnectionPoint for IID_NULL")
	TEST_VARIATION(20, 		L"Container: FindConnectionPoint for ppCP is NULL")
	TEST_VARIATION(21, 		L"Enum: Next with a rgpcd NULL")
	TEST_VARIATION(22, 		L"Enum: Skip on the second one should still be at the end")
	TEST_VARIATION(23, 		L"Container: Enum should fail for a NULL pointer")
	TEST_VARIATION(24, 		L"Enum: reset twice in a row")
	TEST_VARIATION(25, 		L"Enum: clone where ppIEnum is NULL")
	TEST_VARIATION(26, 		L"Point: GetConectionInterface with pIID NULL")
	TEST_VARIATION(27, 		L"Point: GetConectionPointContainer with ppCPC null")
	TEST_VARIATION(28, 		L"Point: Unadvise before advising should fail")
	TEST_VARIATION(29, 		L"Point: advise with pointers NULL")
	TEST_VARIATION(30, 		L"point: Unadvise with pointers NULL")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(TCTransactions)
//*-----------------------------------------------------------------------
// @class Testing RowPos within transactions
//
class TCTransactions : public CTransaction { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCTransactions,CTransaction);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember COMMIT - fRetaning == TRUE
	int Variation_1();
	// @cmember COMMIT - fRetaning == FALSE
	int Variation_2();
	// @cmember ABORT - fRetaning == TRUE
	int Variation_3();
	// @cmember ABORT - fRetaning == FALSE
	int Variation_4();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(TCTransactions)
#define THE_CLASS TCTransactions
BEG_TEST_CASE(TCTransactions, CTransaction, L"Testing RowPos within transactions")
	TEST_VARIATION(1, 		L"COMMIT - fRetaning == TRUE")
	TEST_VARIATION(2, 		L"COMMIT - fRetaning == FALSE")
	TEST_VARIATION(3, 		L"ABORT - fRetaning == TRUE")
	TEST_VARIATION(4, 		L"ABORT - fRetaning == FALSE")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// }} END_DECLARE_TEST_CASES()

// {{ TCW_TESTMODULE(ThisModule)
TEST_MODULE(9, ThisModule, gwszModuleDescrip)
	TEST_CASE(1, TCInitialize)
	TEST_CASE(2, TCGetRowset)
	TEST_CASE(3, TCClearRowPosition)
	TEST_CASE(4, TCSetRowPosition)
	TEST_CASE(5, TCGetRowPosition)
	TEST_CASE(6, TCAddRelease)
	TEST_CASE(7, TCOnRowPositionChange)
	TEST_CASE(8, TCConnectionStuff)
	TEST_CASE(9, TCTransactions)
END_TEST_MODULE()
// }} TCW_TESTMODULE_END

// {{ TCW_TC_PROTOTYPE(TCInitialize)
//*-----------------------------------------------------------------------
//| Test Case:		TCInitialize - Initializes the IRowPosition
//|	Created:			06/10/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCInitialize::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCRowPosition::Init())
	// }}
	{
		return TRUE;
	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc General - Call Initialize with all valid rowset interfaces
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCInitialize::Variation_1()
{
	TBEGIN
	IUnknown* pIUnkRowset = NULL;

	//Initialize the RowPosition.  It should return S_OK.
	TESTC_(CreateRowPosition(pIRowset()),S_OK);
	
	//Rowset IUnknown
	TESTC_(QI(pIRowset(), IID_IUnknown, (void**)&pIUnkRowset),S_OK);
	TESTC_(CreateRowPosition(pIUnkRowset), S_OK);

CLEANUP:
	SAFE_RELEASE(pIUnkRowset);
	TRETURN
}


// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc General - Call Initialize with something that does not support IRowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCInitialize::Variation_2()
{
	TBEGIN
	IUnknown* pIUnkPos = NULL;
	
	//Non-rowset IUnknown
	TESTC_(QI(pIRowPos(), IID_IUnknown, (void**)&pIUnkPos),S_OK);
	TESTC_(CreateRowPosition(pIUnkPos), E_NOINTERFACE);

CLEANUP:
	SAFE_RELEASE(pIUnkPos);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCInitialize::Variation_3()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Boundary - Initialize with a null pointer
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCInitialize::Variation_4()
{
	TBEGIN

	TESTC_(CreateRowPosition(NULL), E_INVALIDARG);
	TESTC_(CreateRowPosition(g_pIDBInitialize),E_NOINTERFACE);

CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Boundary - Call with an IDispatch
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCInitialize::Variation_5()
{
	TBEGIN
	IRowPosition* pIRowPosition = NULL;
	IDispatch* pIDispatch = NULL;
	IRowset* pIRowsetUnk = NULL;
	HRESULT hr = E_FAIL;

	//Create RowPosition
	TESTC_(CreateRowPosition(pIRowset(), &pIRowPosition),S_OK);
	hr = pIRowPosition->GetRowset(IID_IDispatch, (IUnknown**)&pIDispatch);
	
	//Verify results
	//Not all providers rowsets have to support IDispatch
	if( hr == E_NOINTERFACE )
	{
		TESTC(pIDispatch == NULL);
	}
	if( hr == S_OK )
	{
		TESTC( pIDispatch != NULL);
		TESTC_(QI(pIDispatch, IID_IRowset, (void**)&pIRowsetUnk),S_OK);
		TESTC( pIRowsetUnk != NULL);
	}

CLEANUP:
	SAFE_RELEASE(pIRowPosition);
	SAFE_RELEASE(pIRowsetUnk);
	SAFE_RELEASE(pIDispatch);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Boundary - Other non rowset objects
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCInitialize::Variation_6()
{
	//Since we are using Schema and Sources Rowsets, this variation will be different.
	TBEGIN
	ISourcesRowset* pISourcesRowset= NULL;
	IRowsetInfo* pIRowsetInfo = NULL;

	//Get a SchemaRowset and test it.
	CRowset RowsetA;
	TESTC_PROVIDER(RowsetA.CreateRowset(SELECT_DBSCHEMA_TABLE)==S_OK);
	TESTC_(CreateRowPosition(RowsetA()),S_OK);

	//How about with a pointer to the Enumerator...
	TESTC_(CoCreateInstance(CLSID_OLEDB_ENUMERATOR, NULL, CLSCTX_INPROC_SERVER, IID_ISourcesRowset, (void**)&pISourcesRowset),S_OK);
	TESTC_(CreateRowPosition(pISourcesRowset),E_NOINTERFACE);

	//But the rowset from the Enumerator should be successful
	TESTC_(pISourcesRowset->GetSourcesRowset(NULL, IID_IRowsetInfo, 0, NULL, (IUnknown**)&pIRowsetInfo),S_OK);
	TESTC_(CreateRowPosition(pIRowsetInfo),S_OK);
	
CLEANUP:
	SAFE_RELEASE(pISourcesRowset);
	SAFE_RELEASE(pIRowsetInfo);
	TRETURN
}
// }}




// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCInitialize::Variation_7()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc Sequence - Call Initialize twice in a row
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCInitialize::Variation_8()
{
	TBEGIN
	IRowPosition* pIRowPosition = NULL;

	TESTC_(CreateRowPosition(pIRowset(), &pIRowPosition),S_OK);
	TESTC_(pIRowPosition->Initialize(pIRowset()),DB_E_ALREADYINITIALIZED);

CLEANUP:
	SAFE_RELEASE(pIRowPosition);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc Sequence - Call Initialize the second time with an IUnknown
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCInitialize::Variation_9()
{
	TBEGIN
	IRowPosition* pIRowPosition = NULL;
	IRowPosition* pIRowPosition2 = NULL;

	//get the IUnknown off of the other pIRowPosition.
	TESTC_(QI(pIRowPos(), IID_IRowPosition, (void**)&pIRowPosition),S_OK);
	TESTC_(CreateRowPosition(pIRowset(), &pIRowPosition2),S_OK);

	//Second Init... .should fail
	TESTC_(pIRowPos()->Initialize(pIRowPosition),DB_E_ALREADYINITIALIZED);

CLEANUP:
	SAFE_RELEASE(pIRowPosition);
	SAFE_RELEASE(pIRowPosition2);
	TRETURN
}
// }}




// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCInitialize::Variation_10()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc Stress - Initialize with 3 IRowPositions to the same SourceRowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCInitialize::Variation_11()
{
	//Can we init 3 RowPosition's to one Rowset?
	TBEGIN
	IRowPosition* rgpIRowPosition[3] = {NULL,NULL,NULL};
	
	ULONG i;
	for(i=0; i<3; i++)
	{
		TESTC_(CreateRowPosition(pIRowset(), &rgpIRowPosition[i]),S_OK);
	}

CLEANUP:
	for(i=0; i<3; i++ )
		SAFE_RELEASE(rgpIRowPosition[i]);
	TRETURN

}
// }}
// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCInitialize::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCRowPosition::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCGetRowset)
//*-----------------------------------------------------------------------
//| Test Case:		TCGetRowset - Returns the current underlying Rowset
//|	Created:			06/10/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCGetRowset::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCRowPosition::Init())
	// }}
	{
		return TRUE;
	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc General - After Initialization, call GetRowset with riid valid.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetRowset::Variation_1()
{
	TBEGIN
	IUnknown* pIUnknown = NULL;

	//Call GetRowset for the IRowset
	TESTC_(pIRowPos()->GetRowset(IID_IRowset, &pIUnknown),S_OK);
	TESTC(pIUnknown == pIRowset());

CLEANUP:
	SAFE_RELEASE(pIUnknown);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc General - Call GetRowset with IUnknown
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetRowset::Variation_2()
{
	TBEGIN
	IUnknown* pIUnknown = NULL;
	IUnknown* pIUnknown2 = NULL;

	TESTC_(pIRowPos()->GetRowset(IID_IUnknown, &pIUnknown),S_OK);
	TESTC_(QI(pIRowset(), IID_IUnknown, (void**)&pIUnknown2),S_OK);
	TESTC(pIUnknown == pIUnknown2);

CLEANUP:
	SAFE_RELEASE(pIUnknown);
	SAFE_RELEASE(pIUnknown2);
	TRETURN
}
// }}




// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetRowset::Variation_3()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Boundary - Before initialization, call GetRowset with riid null.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetRowset::Variation_4()
{
	TBEGIN	
	IRowPosition* pIRowPosition = NULL;
	IUnknown* pIUnknown = NULL;

	//Create new instance
	TESTC_(CoCreateInstance(CLSID_OLEDB_ROWPOSITIONLIBRARY, NULL, CLSCTX_INPROC_SERVER, IID_IRowPosition, (LPVOID *)&pIRowPosition),S_OK);
	TESTC_(pIRowPosition->GetRowset(IID_NULL, &pIUnknown),E_UNEXPECTED);
	TESTC(pIUnknown == NULL);
	TESTC_(pIRowPosition->GetRowset(IID_IRowset, &pIUnknown),E_UNEXPECTED);
	TESTC(pIUnknown == NULL);

	//Valid cases with CoCreateInstance
	TESTC_(CoCreateInstance(CLSID_OLEDB_ROWPOSITIONLIBRARY, NULL, CLSCTX_INPROC_SERVER, IID_IUnknown, (LPVOID *)&pIUnknown),S_OK);
	TESTC(pIUnknown != NULL);
	SAFE_RELEASE(pIUnknown);

	//Invalid cases with CoCreateInstance
	TESTC_(CoCreateInstance(CLSID_OLEDB_ROWPOSITIONLIBRARY, NULL, CLSCTX_INPROC_SERVER, IID_IRowset, (LPVOID *)&pIUnknown), E_NOINTERFACE);
	TESTC_(CoCreateInstance(CLSID_OLEDB_ROWPOSITIONLIBRARY, NULL, CLSCTX_INPROC_SERVER, IID_IDBInitialize, (LPVOID *)&pIUnknown), E_NOINTERFACE);
	TESTC_(CoCreateInstance(CLSID_OLEDB_ROWPOSITIONLIBRARY, NULL, CLSCTX_INPROC_SERVER, IID_IRowPositionChange, (LPVOID *)&pIUnknown), E_NOINTERFACE);
	TESTC_(CoCreateInstance(CLSID_OLEDB_ROWPOSITIONLIBRARY, NULL, CLSCTX_INPROC_SERVER, IID_IConnectionPointContainer, (LPVOID *)&pIUnknown), S_OK);

CLEANUP:
	SAFE_RELEASE(pIRowPosition);
	SAFE_RELEASE(pIUnknown);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Boundary - Before initialization call GetRowset with riid valid
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetRowset::Variation_5()
{
	TBEGIN
	IRowPosition* pIRowPosition = NULL;
	IUnknown* pIUnknown= NULL;

	TESTC_(CoCreateInstance(CLSID_OLEDB_ROWPOSITIONLIBRARY, NULL, CLSCTX_INPROC_SERVER, IID_IRowPosition, (LPVOID *)&pIRowPosition),S_OK);
	TESTC_(pIRowPosition->GetRowset(IID_IRowset, &pIUnknown),E_UNEXPECTED);
	TESTC(pIUnknown == NULL);

CLEANUP:
	SAFE_RELEASE(pIRowPosition);
	SAFE_RELEASE(pIUnknown);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Boundary - After Initialization, call GetRowset with riid invalid
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetRowset::Variation_6()
{
	TBEGIN
	IUnknown* pIUnknown = NULL;

	TESTC_(pIRowPos()->GetRowset(IID_ICommand, &pIUnknown),E_NOINTERFACE);
	TESTC(pIUnknown == NULL);

CLEANUP:
	SAFE_RELEASE(pIUnknown);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Boundary - Make sure the rowset was AddReffed, by killing the IRowPos, and making sure the Rowset is still valid.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetRowset::Variation_7()
{
	TBEGIN
	IRowPosition* pIRowPosition = NULL;
	IRowset* pIRowsetUnk = NULL;

	TESTC_(CreateRowPosition(pIRowset(), &pIRowPosition),S_OK);
	TESTC_(pIRowPosition->GetRowset(IID_IRowset, (IUnknown**)&pIRowsetUnk),S_OK);

	//Make sure rowset is still valid after RowPosition release!
	SAFE_RELEASE(pIRowPosition);
	TEST2C_(pIRowsetUnk->RestartPosition(NULL),S_OK,DB_S_COMMANDREEXECUTED);

CLEANUP:
	SAFE_RELEASE(pIRowsetUnk);
	SAFE_RELEASE(pIRowPosition);
	TRETURN
}
// }}




// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetRowset::Variation_8()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc Related - Initialize another IRowPosition with GetRowset from the first one
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetRowset::Variation_9()
{
	TBEGIN
	IRowPosition* pIRowPosition = NULL;
	IUnknown* pIRowset = NULL;

	TESTC_(pIRowPos()->GetRowset(IID_IRowset, &pIRowset),S_OK);
	TESTC_(CreateRowPosition(pIRowset, &pIRowPosition),S_OK);

CLEANUP:
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pIRowPosition);
	TRETURN
}
// }}




// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetRowset::Variation_10()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc Multithreaded - CoCreateInstance, Initialize
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetRowset::Variation_11()
{
	TBEGIN
	INIT_THREADS(THREE_THREADS);	
	
	//Setup Thread Arguments
	//There are none in this case.
	THREADARG T1Arg = { this };
	THREADARG T2Arg = { this };
	THREADARG T3Arg = { this };

	//Create Threads
	CREATE_THREAD(THREAD_ONE, Thread_CreateRowPosition,&T1Arg);
	CREATE_THREAD(THREAD_TWO, Thread_CreateRowPosition,&T2Arg);
	CREATE_THREAD(THREAD_THREE, Thread_CreateRowPosition,&T3Arg);

	START_THREADS();
	END_THREADS();	

	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc Multithreaded - Initialize
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetRowset::Variation_12()
{
	INIT_THREADS(10);
	
	//Setup Thread Arguments
	//There are none in this case.
	THREADARG T1Arg = { this, pIRowPos() };

	//Create Threads
	CREATE_THREADS(Thread_Initialize, &T1Arg);

	START_THREADS();
	END_THREADS();	

	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc Multithreaded - GetRowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetRowset::Variation_13()
{
	INIT_THREADS(10);
	
	//Setup Thread Arguments
	//There are none in this case.
	THREADARG T1Arg = { this, pIRowPos() };

	//Create Threads
	CREATE_THREADS(Thread_GetRowset, &T1Arg);

	START_THREADS();
	END_THREADS();	

	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc Multithreaded - GetRowPosition
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetRowset::Variation_14()
{
	INIT_THREADS(10);
	
	//Setup Thread Arguments
	//There are none in this case.
	THREADARG T1Arg = { this, pIRowPos() };

	//Create Threads
	CREATE_THREADS(Thread_GetRowPosition, &T1Arg);

	START_THREADS();
	END_THREADS();	

	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc Multithreaded - Clear,Set,Get RowPosition Combo
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetRowset::Variation_15()
{
	TBEGIN
	HROW hRow = DB_NULL_HROW;
	INIT_THREADS(10);
	
	//Setup Thread Arguments
	//There are none in this case.
	THREADARG T1Arg = { this, pIRowPos(), &hRow };

	TESTC_(RestartPosition(),S_OK);
	TESTC_(GetNextRows(&hRow),S_OK);
	
	//Create Threads
	CREATE_THREADS(Thread_ClearSetGetCombo, &T1Arg);

	START_THREADS();
	END_THREADS();	

CLEANUP:
	ReleaseRows(hRow);
	TRETURN
}
// }}
// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCGetRowset::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCRowPosition::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCClearRowPosition)
//*-----------------------------------------------------------------------
//| Test Case:		TCClearRowPosition - Clears the value of the current hRow
//|	Created:			06/13/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCClearRowPosition::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCRowPosition::Init())
	// }}
	{
		return TRUE;
	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc General - Call ClearRowPosition after a call to SetRowPosition
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCClearRowPosition::Variation_1()
{
	TBEGIN
	HROW hRow = 101; //Random start value

	TESTC_(ClearRowPosition(pIRowPos()),S_OK);

	//Make sure we're at the beginning, and then set to that first row.
	TESTC_(RestartPosition(),S_OK);
	TESTC_(GetNextRows(&hRow),S_OK);

	TESTC_(SetRowPosition(pIRowPos(), DB_NULL_HCHAPTER, hRow, DBPOSITION_OK),S_OK);
	TESTC_(ClearRowPosition(pIRowPos()),S_OK);
	TESTC(VerifyGetRowPosition(pIRowPos(), DB_NULL_HCHAPTER, DB_NULL_HROW, DBPOSITION_NOROW));

CLEANUP:
	ReleaseRows(hRow);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc General - Call ClearRowPosition directly after an Init.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCClearRowPosition::Variation_2()
{
	TBEGIN
	IRowPosition* pIRowPosition = NULL;

	TESTC_(CreateRowPosition(pIRowset(), &pIRowPosition),S_OK);
	TESTC_(ClearRowPosition(pIRowPosition),S_OK);
	TESTC(VerifyGetRowPosition(pIRowPosition, DB_NULL_HCHAPTER, DB_NULL_HROW, DBPOSITION_NOROW));

CLEANUP:
	SAFE_RELEASE(pIRowPosition);
	TRETURN
}
// }}




// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCClearRowPosition::Variation_3()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Boundary - Call ClearRowPosition before any calls to Init
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCClearRowPosition::Variation_4()
{
	TBEGIN
	IRowPosition* pIRowPosition = NULL;

	TESTC_(CoCreateInstance(CLSID_OLEDB_ROWPOSITIONLIBRARY, NULL, CLSCTX_INPROC_SERVER, IID_IRowPosition, (LPVOID *)&pIRowPosition),S_OK);
	TESTC_(ClearRowPosition(pIRowPosition),E_UNEXPECTED);

CLEANUP:
	SAFE_RELEASE(pIRowPosition);
	TRETURN
}
// }}




// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCClearRowPosition::Variation_5()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Sequence - Call ClearRowPosition twice in a row
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCClearRowPosition::Variation_6()
{
	TBEGIN
	//Use Local to make sure we know what state it started in.
	IRowPosition* pIRowPosition = NULL;

	TESTC_(CreateRowPosition(pIRowset(), &pIRowPosition),S_OK);
	TESTC_(ClearRowPosition(pIRowPosition),S_OK);
	TESTC_(ClearRowPosition(pIRowPosition),E_UNEXPECTED);

CLEANUP:
	SAFE_RELEASE(pIRowPosition);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Sequence - Call ClearRowPosition, then a GetRowPosition
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCClearRowPosition::Variation_7()
{
	TBEGIN
	IRowPosition* pIRowPosition = NULL;
	TESTC_(CreateRowPosition(pIRowset(), &pIRowPosition),S_OK);

	TESTC_(ClearRowPosition(pIRowPosition),S_OK);
	TESTC(VerifyGetRowPosition(pIRowPosition, DB_NULL_HCHAPTER, DB_NULL_HROW, DBPOSITION_NOROW));

CLEANUP:
	SAFE_RELEASE(pIRowPosition);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc Sequence - Call ClearRowPosition, then a GetnextRows
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCClearRowPosition::Variation_8()
{
	TBEGIN
	IRowPosition* pIRowPosition = NULL;
	HROW hRow1=DB_NULL_HROW;
	HROW hRow2=DB_NULL_HROW;

	TESTC_(CreateRowPosition(pIRowset(), &pIRowPosition),S_OK);
	TESTC_(RestartPosition(),S_OK);
	TESTC_(GetNextRows(&hRow1),S_OK);

	TESTC_(ClearRowPosition(pIRowPosition),S_OK);
	TESTC_(GetNextRows(&hRow2),S_OK);
	TESTC(hRow1 != hRow2);

CLEANUP:
	ReleaseRows(hRow1);
	ReleaseRows(hRow2);
	SAFE_RELEASE(pIRowPosition);
	TRETURN
}
// }}




// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCClearRowPosition::Variation_9()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc Multiuser - ClearRowPosition should not affect other IRowPosition's
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCClearRowPosition::Variation_10()
{
	TBEGIN
	HROW hRow;
	IRowPosition* pIRowPosition = NULL;
	IRowPosition* pIRowPosition2 = NULL;
	IRowPosition* pIRowPosition3 = NULL;

	TESTC_(CreateRowPosition(pIRowset(), &pIRowPosition),S_OK);
	TESTC_(CreateRowPosition(pIRowset(), &pIRowPosition2),S_OK);
	TESTC_(CreateRowPosition(pIRowset(), &pIRowPosition3),S_OK);

	TESTC_(RestartPosition(),S_OK);
	TESTC_(GetNextRows(&hRow),S_OK);
	
	TESTC_(ClearRowPosition(pIRowPosition),S_OK);
	TESTC_(ClearRowPosition(pIRowPosition2),S_OK);
	TESTC_(ClearRowPosition(pIRowPosition3),S_OK);
	
	TESTC_(SetRowPosition(pIRowPosition, DB_NULL_HCHAPTER,hRow,DBPOSITION_OK),S_OK);
	TESTC_(SetRowPosition(pIRowPosition2, DB_NULL_HCHAPTER,hRow,DBPOSITION_OK),S_OK);
	TESTC_(SetRowPosition(pIRowPosition3, DB_NULL_HCHAPTER,hRow,DBPOSITION_OK),S_OK);
	
	TESTC_(ClearRowPosition(pIRowPosition),S_OK);
	TESTC_(SetRowPosition(pIRowPosition2, DB_NULL_HCHAPTER,hRow,DBPOSITION_OK),E_UNEXPECTED);
	TESTC_(SetRowPosition(pIRowPosition3, DB_NULL_HCHAPTER,hRow,DBPOSITION_OK),E_UNEXPECTED);

CLEANUP:
	ReleaseRows(hRow);
	SAFE_RELEASE(pIRowPosition);
	SAFE_RELEASE(pIRowPosition2);
	SAFE_RELEASE(pIRowPosition3);
	TRETURN
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCClearRowPosition::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCRowPosition::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCSetRowPosition)
//*-----------------------------------------------------------------------
//| Test Case:		TCSetRowPosition - Sets the current row position
//|	Created:			06/13/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCSetRowPosition::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCRowPosition::Init())
	// }}
	{
		return TRUE;
	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc General - SetRowPosition to an hRow in the middle of many hRows
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSetRowPosition::Variation_1()
{
	TBEGIN
	HROW hRow = DB_NULL_HROW;

	//Obtain second row
	TESTC_(ClearRowPosition(pIRowPos()),S_OK);
	TESTC_(GetRow(SECOND_ROW,  &hRow),S_OK);

	TESTC(VerifySetRowPosition(pIRowPos(), DB_NULL_HCHAPTER, hRow, DBPOSITION_OK));

CLEANUP:
	ReleaseRows(hRow);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc General - SetRowPosition to an hRow in the middle of many hRows with DBPOSITION_NOROW
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSetRowPosition::Variation_2()
{
	TBEGIN
	HROW hRow = DB_NULL_HROW;
	IRowPosition* pIRowPosition = NULL;

	TESTC_(CreateRowPosition(pIRowset(), &pIRowPosition),S_OK);
	TESTC_(GetRow(SECOND_ROW, &hRow),S_OK);

	TESTC_(ClearRowPosition(pIRowPosition),S_OK);
	TESTC_(SetRowPosition(pIRowPosition, DB_NULL_HCHAPTER, hRow, DBPOSITION_NOROW), E_INVALIDARG);
	
	TESTC(VerifyGetRowPosition(pIRowPosition, DB_NULL_HCHAPTER, DB_NULL_HROW, DBPOSITION_NOROW));

CLEANUP:
	ReleaseRows(hRow);
	SAFE_RELEASE(pIRowPosition);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc General - test all the invalid flag values on valid hRows
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSetRowPosition::Variation_3()
{
	// NOTE: Tests both BOF and EOF in one variation
	TBEGIN
	IRowPosition* pIRowPosition = NULL;
	HROW hRow = DB_NULL_HROW;
	ULONG i=0;

	TESTC_(CreateRowPosition(pIRowset(), &pIRowPosition),S_OK);
	TESTC_(RestartPosition(),S_OK);
	TESTC_(GetRow(SECOND_ROW, &hRow),S_OK);

	//DBPOSITION_BOF
	TESTC_(ClearRowPosition(pIRowPosition),S_OK);
	TESTC_(SetRowPosition(pIRowPosition, DB_NULL_HCHAPTER,hRow,DBPOSITION_BOF),E_INVALIDARG);
	TESTC(VerifyGetRowPosition(pIRowPosition, DB_NULL_HCHAPTER, DB_NULL_HROW, DBPOSITION_NOROW));

	//DBPOSITION_EOF
	TESTC_(ClearRowPosition(pIRowPosition),S_OK);
	TESTC_(SetRowPosition(pIRowPosition, DB_NULL_HCHAPTER,hRow,DBPOSITION_EOF),E_INVALIDARG);
	TESTC(VerifyGetRowPosition(pIRowPosition, DB_NULL_HCHAPTER, DB_NULL_HROW, DBPOSITION_NOROW));

	//ULONG_MAX
	TESTC_(ClearRowPosition(pIRowPosition),S_OK);
	TESTC_(SetRowPosition(pIRowPosition, DB_NULL_HCHAPTER,hRow,ULONG_MAX),E_INVALIDARG);
	TESTC(VerifyGetRowPosition(pIRowPosition, DB_NULL_HCHAPTER, DB_NULL_HROW, DBPOSITION_NOROW));

CLEANUP:
	ReleaseRows(hRow);
	SAFE_RELEASE(pIRowPosition);
	TRETURN
}
// }}




// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSetRowPosition::Variation_4()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Boundary - SetRowPosition to null with no rows and DBPOSITION_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSetRowPosition::Variation_5()
{
	TBEGIN
	IRowPosition* pIRowPosition = NULL;
	
	//Create Empty rowset
	CRowset EmptyRowset;
	TESTC_PROVIDER(EmptyRowset.CreateRowset(SELECT_EMPTYROWSET)==S_OK);
	TESTC_(CreateRowPosition(EmptyRowset.pIRowset(), &pIRowPosition),S_OK);

	TESTC_(ClearRowPosition(pIRowPosition),S_OK);
	TESTC(VerifySetRowPosition(pIRowPosition, DB_NULL_HCHAPTER,DB_NULL_HROW,DBPOSITION_NOROW));

CLEANUP:
	SAFE_RELEASE(pIRowPosition);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Boundary - SetRowPosition to null with no rows and DBPOSITION_NOROW
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSetRowPosition::Variation_6()
{
	TBEGIN
	HROW hRow = DB_NULL_HROW;
	IRowPosition* pIRowPosition = NULL;

	//Create Empty rowset
	CRowset EmptyRowset;
	TESTC_PROVIDER(EmptyRowset.CreateRowset(SELECT_EMPTYROWSET)==S_OK);
	TESTC_(CreateRowPosition(EmptyRowset.pIRowset(), &pIRowPosition),S_OK);

	TESTC_(ClearRowPosition(pIRowPosition),S_OK);
	TESTC(VerifySetRowPosition(pIRowPosition, DB_NULL_HCHAPTER,DB_NULL_HROW,DBPOSITION_NOROW));

CLEANUP:
	EmptyRowset.ReleaseRows(hRow);
	SAFE_RELEASE(pIRowPosition);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Boundary - SetRowPosition to null with no rows and DBPOSITION_BOF
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSetRowPosition::Variation_7()
{
	TBEGIN
	IRowPosition* pIRowPosition = NULL;

	//Create Empty rowset
	CRowset EmptyRowset;
	TESTC_PROVIDER(EmptyRowset.CreateRowset(SELECT_EMPTYROWSET)==S_OK);
	TESTC_(CreateRowPosition(EmptyRowset.pIRowset(), &pIRowPosition),S_OK);

	TESTC_(ClearRowPosition(pIRowPosition),S_OK);
	TESTC(VerifySetRowPosition(pIRowPosition, DB_NULL_HCHAPTER,DB_NULL_HROW,DBPOSITION_BOF));

CLEANUP:
	SAFE_RELEASE(pIRowPosition);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc Boundary - SetRowPosition to null with no rows and DBPOSITION_EOF
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSetRowPosition::Variation_8()
{
	TBEGIN
	IRowPosition* pIRowPosition = NULL;

	//Create Empty rowset
	CRowset EmptyRowset;
	TESTC_PROVIDER(EmptyRowset.CreateRowset(SELECT_EMPTYROWSET)==S_OK);
	TESTC_(CreateRowPosition(EmptyRowset.pIRowset(), &pIRowPosition),S_OK);

	TESTC_(ClearRowPosition(pIRowPosition),S_OK);
	TESTC(VerifySetRowPosition(pIRowPosition, DB_NULL_HCHAPTER,DB_NULL_HROW,DBPOSITION_EOF));

CLEANUP:
	SAFE_RELEASE(pIRowPosition);
	TRETURN

}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc Boundary - SetRowPosition to some hRow with no rows
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSetRowPosition::Variation_9()
{
	TBEGIN
	IRowPosition* pIRowPosition = NULL;

	//Create Empty rowset
	CRowset EmptyRowset;
	TESTC_PROVIDER(EmptyRowset.CreateRowset(SELECT_EMPTYROWSET)==S_OK);
	TESTC_(CreateRowPosition(EmptyRowset.pIRowset(), &pIRowPosition),S_OK);

	TESTC_(ClearRowPosition(pIRowPosition),S_OK);
	TESTC_(SetRowPosition(pIRowPosition, DB_NULL_HCHAPTER,ULONG_MAX,DBPOSITION_OK),DB_E_BADROWHANDLE);
	TESTC_(SetRowPosition(pIRowPosition, DB_NULL_HCHAPTER,ULONG_MAX,DBPOSITION_EOF),E_UNEXPECTED);
	TESTC(VerifyGetRowPosition(pIRowPosition, DB_NULL_HCHAPTER, DB_NULL_HROW, DBPOSITION_NOROW));

CLEANUP:
	SAFE_RELEASE(pIRowPosition);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc Boundary - SetRowPosition to null with one row and DBPOSITION_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSetRowPosition::Variation_10()
{
	TBEGIN
	IRowPosition* pIRowPosition = NULL;

	//Create 1 row rowset
	CRowset Rowset1Row;
	TESTC_(Rowset1Row.CreateRowset(SELECT_ALLFROMTBL, IID_IRowset, g_p1RowTable),S_OK);
	TESTC_(CreateRowPosition(Rowset1Row(), &pIRowPosition),S_OK);

	TESTC_(ClearRowPosition(pIRowPosition),S_OK);
	TESTC_(SetRowPosition(pIRowPosition, DB_NULL_HCHAPTER,DB_NULL_HROW,DBPOSITION_OK),E_INVALIDARG);

CLEANUP:
	SAFE_RELEASE(pIRowPosition);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc Boundary - SetRowPosition to null with one row and DBPOSITION_NOROW
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSetRowPosition::Variation_11()
{
	TBEGIN
	IRowPosition* pIRowPosition = NULL;

	//Create 1 row rowset
	CRowset Rowset1Row;
	TESTC_(Rowset1Row.CreateRowset(SELECT_ALLFROMTBL, IID_IRowset, g_p1RowTable),S_OK);
	TESTC_(CreateRowPosition(Rowset1Row(), &pIRowPosition),S_OK);

	TESTC_(ClearRowPosition(pIRowPosition),S_OK);
	TESTC(VerifySetRowPosition(pIRowPosition, DB_NULL_HCHAPTER,DB_NULL_HROW,DBPOSITION_NOROW));

CLEANUP:
	SAFE_RELEASE(pIRowPosition);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc Boundary - SetRowPosition to null with one row and DBPOSITION_BOF
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSetRowPosition::Variation_12()
{
	TBEGIN
	IRowPosition* pIRowPosition = NULL;

	//Create 1 row rowset
	CRowset Rowset1Row;
	TESTC_(Rowset1Row.CreateRowset(SELECT_ALLFROMTBL, IID_IRowset, g_p1RowTable),S_OK);
	TESTC_(CreateRowPosition(Rowset1Row(), &pIRowPosition),S_OK);

	TESTC_(ClearRowPosition(pIRowPosition),S_OK);
	TESTC(VerifySetRowPosition(pIRowPosition, DB_NULL_HCHAPTER,DB_NULL_HROW,DBPOSITION_BOF));

CLEANUP:
	SAFE_RELEASE(pIRowPosition);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc Boundary - SetRowPosition to the last row of many
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSetRowPosition::Variation_13()
{
	TBEGIN
	HROW hRow = NULL;
	IRowPosition* pIRowPosition = NULL;
	
	TESTC_(CreateRowPosition(pIRowset(), &pIRowPosition),S_OK);
	TESTC_(RestartPosition(),S_OK);
	TESTC_(GetRow(m_ulTotalRows, &hRow),S_OK);

	TESTC_(ClearRowPosition(pIRowPosition),S_OK);
	TESTC(VerifySetRowPosition(pIRowPosition, DB_NULL_HCHAPTER, hRow, DBPOSITION_OK));

CLEANUP:
	ReleaseRows(hRow);
	SAFE_RELEASE(pIRowPosition);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc Boundary - SetRowPosition to the first row of many
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSetRowPosition::Variation_14()
{
	TBEGIN
	HROW hRow = NULL;
	IRowPosition* pIRowPosition = NULL;
	
	TESTC_(CreateRowPosition(pIRowset(), &pIRowPosition),S_OK);
	TESTC_(RestartPosition(),S_OK);
	TESTC_(GetRow(FIRST_ROW, &hRow),S_OK);

	TESTC_(ClearRowPosition(pIRowPosition),S_OK);
	TESTC(VerifySetRowPosition(pIRowPosition, DB_NULL_HCHAPTER, hRow, DBPOSITION_OK));

CLEANUP:
	ReleaseRows(hRow);
	SAFE_RELEASE(pIRowPosition);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc Boundary - SetRowPosition to an invalid (nonexistent
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSetRowPosition::Variation_15()
{
	TBEGIN
	IRowPosition* pIRowPosition = NULL;
	TESTC_(CreateRowPosition(pIRowset(), &pIRowPosition),S_OK);

	TESTC_(ClearRowPosition(pIRowPosition),S_OK);
	TESTC_(SetRowPosition(pIRowPosition, DB_NULL_HCHAPTER,123,DBPOSITION_OK),DB_E_BADROWHANDLE);
	TESTC_(SetRowPosition(pIRowPosition, DB_NULL_HCHAPTER,123,DBPOSITION_EOF),E_UNEXPECTED);
	TESTC(VerifyGetRowPosition(pIRowPosition, DB_NULL_HCHAPTER, DB_NULL_HROW, DBPOSITION_NOROW));

CLEANUP:
	SAFE_RELEASE(pIRowPosition);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc Boundary - SetRowPosition with a null row handle
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSetRowPosition::Variation_16()
{
	TBEGIN
	IRowPosition* pIRowPosition = NULL;
	TESTC_(CreateRowPosition(pIRowset(), &pIRowPosition),S_OK);

	TESTC_(ClearRowPosition(pIRowPosition),S_OK);
	TESTC_(SetRowPosition(pIRowPosition, DB_NULL_HCHAPTER,DB_NULL_HROW,DBPOSITION_NOROW),S_OK);
	TESTC(VerifyGetRowPosition(pIRowPosition, DB_NULL_HCHAPTER, DB_NULL_HROW, DBPOSITION_NOROW));

CLEANUP:
	SAFE_RELEASE(pIRowPosition);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc Boundary - SetRowPosition with a deleted row handle
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSetRowPosition::Variation_17()
{
	TBEGIN
	HROW hRow = NULL;
	IRowPosition* pIRowPosition = NULL;
	
	//Create rowset with IRowsetChange
	CRowsetChange RowsetChangeA;
	TESTC_PROVIDER(RowsetChangeA.CreateRowset()==S_OK);

	TESTC_(CreateRowPosition(RowsetChangeA.pIRowset(), &pIRowPosition),S_OK);
	TESTC_(RowsetChangeA.RestartPosition(),S_OK);

	//Delete the last row so we don't mess up the first row for the other
	//variations.  Since some provider may have REMOVEDELETED=FALSE and GetNextRows
	//would then return a deleted row...
	TESTC_(RowsetChangeA.GetRow(m_ulTotalRows, &hRow),S_OK);
	TESTC_(RowsetChangeA.DeleteRow(hRow),S_OK);
	
	TESTC_(ClearRowPosition(pIRowPosition),S_OK);
	TESTC_(SetRowPosition(pIRowPosition, DB_NULL_HCHAPTER, hRow,DBPOSITION_OK), DB_E_BADROWHANDLE);
	TESTC(VerifyGetRowPosition(pIRowPosition, DB_NULL_HCHAPTER, DB_NULL_HROW, DBPOSITION_NOROW));

CLEANUP:
	RowsetChangeA.ReleaseRows(hRow);
	SAFE_RELEASE(pIRowPosition);
	TRETURN
}
// }}




// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSetRowPosition::Variation_18()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc Sequence - Call a valid SetRowPosition in a multi-row scenario after a valid SetRowPosition
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSetRowPosition::Variation_19()
{
	TBEGIN
	HROW hRow = DB_NULL_HROW;

	IRowPosition* pIRowPosition = NULL;
	TESTC_(CreateRowPosition(pIRowset(), &pIRowPosition),S_OK);

	TESTC_(ClearRowPosition(pIRowPosition),S_OK);
	TESTC_(RestartPosition(NULL),S_OK);
	TESTC_(GetRow(SECOND_ROW, &hRow),S_OK);
	
	TESTC_(SetRowPosition(pIRowPosition, DB_NULL_HCHAPTER, hRow, DBPOSITION_OK),S_OK);
	TESTC_(SetRowPosition(pIRowPosition, DB_NULL_HCHAPTER, hRow, DBPOSITION_OK),E_UNEXPECTED);
	TESTC(VerifyGetRowPosition(pIRowPosition, DB_NULL_HCHAPTER, hRow, DBPOSITION_OK));

CLEANUP:
	ReleaseRows(hRow);
	SAFE_RELEASE(pIRowPosition);
	TRETURN

}
// }}




// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSetRowPosition::Variation_20()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc Multiuser - SetRowPosition with different IRowPositions but with the same sourcerowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSetRowPosition::Variation_21()
{
	TBEGIN
	IRowPosition* pIRowPosition = NULL;
	IRowPosition* pIRowPosition2 = NULL;
	HROW hRow = NULL;
	HROW hRow2 = NULL;

	TESTC_(CreateRowPosition(pIRowset(), &pIRowPosition),S_OK);
	TESTC_(CreateRowPosition(pIRowset(), &pIRowPosition2),S_OK);

	TESTC_(RestartPosition(NULL),S_OK);
	TESTC_(ClearRowPosition(pIRowPosition),S_OK);
	TESTC_(GetNextRows(&hRow),S_OK);
	TESTC_(SetRowPosition(pIRowPosition, DB_NULL_HCHAPTER, hRow,DBPOSITION_OK),S_OK);

	TESTC_(ClearRowPosition(pIRowPosition2),S_OK);
	TESTC_(GetNextRows(&hRow2),S_OK);
	TESTC_(SetRowPosition(pIRowPosition2, DB_NULL_HCHAPTER,hRow2,DBPOSITION_OK),S_OK);

	//Verify RowPositons
	TESTC(VerifyGetRowPosition(pIRowPosition, DB_NULL_HCHAPTER, hRow, DBPOSITION_OK));
	TESTC(VerifyGetRowPosition(pIRowPosition2, DB_NULL_HCHAPTER, hRow2, DBPOSITION_OK));
	
CLEANUP:
	ReleaseRows(hRow);
	ReleaseRows(hRow2);
	SAFE_RELEASE(pIRowPosition);
	SAFE_RELEASE(pIRowPosition2);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(22)
//*-----------------------------------------------------------------------
// @mfunc Multiuser - SetRowPosition with different IRowPositions and with different sourcerowsets
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCSetRowPosition::Variation_22()
{
	TBEGIN
	IRowPosition* pIRowPosition = NULL;
	IRowPosition* pIRowPosition2 = NULL;
	HROW hRow = NULL;
	HROW hRow2 = NULL;

	TESTC_(CreateRowPosition(pIRowset(), &pIRowPosition),S_OK);
	TESTC_(CreateRowPosition(pIRowset(), &pIRowPosition2),S_OK);

	TESTC_(RestartPosition(NULL),S_OK);
	TESTC_(ClearRowPosition(pIRowPosition),S_OK);
	TESTC_(GetNextRows(&hRow),S_OK);
	TESTC_(SetRowPosition(pIRowPosition, DB_NULL_HCHAPTER, hRow,DBPOSITION_OK),S_OK);

	TESTC_(ClearRowPosition(pIRowPosition2),S_OK);
	TESTC_(GetNextRows(&hRow2),S_OK);
	TESTC_(SetRowPosition(pIRowPosition2, DB_NULL_HCHAPTER,hRow2,DBPOSITION_OK),S_OK);

	//Verify RowPositons
	TESTC(VerifyGetRowPosition(pIRowPosition, DB_NULL_HCHAPTER, hRow, DBPOSITION_OK));
	TESTC(VerifyGetRowPosition(pIRowPosition2, DB_NULL_HCHAPTER, hRow2, DBPOSITION_OK));
	
CLEANUP:
	ReleaseRows(hRow);
	ReleaseRows(hRow2);
	SAFE_RELEASE(pIRowPosition);
	SAFE_RELEASE(pIRowPosition2);
	TRETURN
}
// }}
// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCSetRowPosition::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCRowPosition::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCGetRowPosition)
//*-----------------------------------------------------------------------
//| Test Case:		TCGetRowPosition - Returns the current row possition
//|	Created:			06/16/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCGetRowPosition::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCRowPosition::Init())
	// }}
	{
		return TRUE;
	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc General - GetRowPosition with one row, hRow set.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetRowPosition::Variation_1()
{
	TBEGIN
	HROW hRow = DB_NULL_HROW;
	IRowPosition* pIRowPosition = NULL;

	//Create Rowset
	CRowset RowsetA;
	RowsetA.SetProperty(DBPROP_CANHOLDROWS);
	TESTC_(RowsetA.CreateRowset(SELECT_ALLFROMTBL, IID_IRowset, g_p1RowTable),S_OK);

	TESTC_(CreateRowPosition(RowsetA(), &pIRowPosition),S_OK);
	TESTC_(RowsetA.GetRow(FIRST_ROW, &hRow),S_OK);
	
	TESTC_(ClearRowPosition(pIRowPosition),S_OK);
	TESTC(VerifySetRowPosition(pIRowPosition, DB_NULL_HCHAPTER, hRow, DBPOSITION_OK));

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	SAFE_RELEASE(pIRowPosition);
	TRETURN

}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc General - Make sure it AddRefs the hRows.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetRowPosition::Variation_2()
{
	TBEGIN
	HROW hRow = NULL;
	IRowPosition* pIRowPosition = NULL;
	void* pData = NULL;

	HCHAPTER hChapter = NULL;
	DBPOSITIONFLAGS dwPositionFlags = 0;

	TESTC_(CreateRowPosition(pIRowset(), &pIRowPosition),S_OK);
	TESTC_(RestartPosition(NULL),S_OK);
	TESTC_(GetRow(SECOND_ROW, &hRow),S_OK);

	//Clear RowPosition
	TESTC_(ClearRowPosition(pIRowPosition),S_OK);
	TESTC_(SetRowPosition(pIRowPosition, DB_NULL_HCHAPTER, hRow, DBPOSITION_OK),S_OK);
	TESTC_(GetRowPosition(pIRowPosition, &hChapter, &hRow, &dwPositionFlags),S_OK);
	TESTC_(GetRowData(hRow, &pData),S_OK);

CLEANUP:
	SAFE_RELEASE(pIRowPosition);
	ReleaseRows(hRow);
	PROVIDER_FREE(pData);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc General - GetRowPosition immediately after an Initialize.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetRowPosition::Variation_3()
{
	TBEGIN
	IRowPosition* pIRowPosition = NULL;

	TESTC_(CreateRowPosition(pIRowset(), &pIRowPosition),S_OK);
	TESTC(VerifyGetRowPosition(pIRowPos(), DB_NULL_HCHAPTER, DB_NULL_HROW, DBPOSITION_NOROW));

CLEANUP:
	SAFE_RELEASE(pIRowPosition);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc General - GetRowPosition before an Initialize.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetRowPosition::Variation_4()
{
	TBEGIN
	IRowPosition* pIRowPosition = NULL;

	TESTC_(CoCreateInstance(CLSID_OLEDB_ROWPOSITIONLIBRARY, NULL, CLSCTX_INPROC_SERVER, IID_IRowPosition, (LPVOID *)&pIRowPosition),S_OK);
	TESTC(VerifyGetRowPosition(pIRowPos(), DB_NULL_HCHAPTER, DB_NULL_HROW, DBPOSITION_NOROW));

CLEANUP:
	SAFE_RELEASE(pIRowPosition);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc General - GetRowPosition after a ClearRowPosition.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetRowPosition::Variation_5()
{
	TBEGIN
	IRowPosition* pIRowPosition = NULL;

	TESTC_(CreateRowPosition(pIRowset(), &pIRowPosition),S_OK);
	TESTC_(ClearRowPosition(pIRowPosition),S_OK);
	TESTC(VerifyGetRowPosition(pIRowPosition, DB_NULL_HCHAPTER, DB_NULL_HROW, DBPOSITION_NOROW));

CLEANUP:
	SAFE_RELEASE(pIRowPosition);
	TRETURN
}
// }}




// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetRowPosition::Variation_6()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Parameters - GetRowPosition with hChapter NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetRowPosition::Variation_7()
{
	TBEGIN
	HROW hRow = 101;			//Random values.
	DBPOSITIONFLAGS dwPositionFlags = 101;

	//Spec allow phChapter to be null if is not interested in the hChapter
	TESTC_(GetRowPosition(pIRowPos(), NULL, &hRow, &dwPositionFlags), S_OK);
	TESTC(hRow == DB_NULL_HROW && dwPositionFlags == DBPOSITION_NOROW);

CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc Parameters - GetRowPosition with phRow NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetRowPosition::Variation_8()
{
	TBEGIN
	HCHAPTER hChapter = 101;			//Random values.
	DBPOSITIONFLAGS dwPositionFlags = 101;

	//E_INVALIDARG - Spec requires phRow
	TESTC_(GetRowPosition(pIRowPos(), &hChapter, NULL, &dwPositionFlags),E_INVALIDARG);
	TESTC(hChapter == DB_NULL_HCHAPTER && dwPositionFlags == DBPOSITION_NOROW);

CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc Parameters - GetRowPosition with pdwPositionFlags NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetRowPosition::Variation_9()
{
	TBEGIN
	HROW hRow = 101;
	HCHAPTER hChapter = 101;			//Random values.

	//Spec allows pdwPositionFlags = NULL if not interested in dwPosFlags...
	TESTC_(GetRowPosition(pIRowPos(), &hChapter, &hRow, NULL), S_OK);
	TESTC(hChapter == DB_NULL_HCHAPTER && hRow == DB_NULL_HROW);

CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc Parameters - GetRowPosition with all params NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetRowPosition::Variation_10()
{
	TBEGIN

	TESTC_(GetRowPosition(pIRowPos(), NULL, NULL, NULL),E_INVALIDARG);

CLEANUP:
	TRETURN
}
// }}




// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetRowPosition::Variation_11()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc Multiuser - Call GetNextRows with multiple IRowPositions.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetRowPosition::Variation_12()
{
	TBEGIN
	HROW hRow=DB_NULL_HROW;
	HROW hRow2=DB_NULL_HROW;
	IRowPosition* pIRowPosition = NULL;
	IRowPosition* pIRowPosition2 = NULL;

	TESTC_(CreateRowPosition(pIRowset(), &pIRowPosition),S_OK);
	TESTC_(CreateRowPosition(pIRowset(), &pIRowPosition2),S_OK);

	TESTC_(RestartPosition(NULL),S_OK);
	TESTC_(ClearRowPosition(pIRowPosition),S_OK);
	TESTC_(ClearRowPosition(pIRowPosition2),S_OK);

	TESTC_(GetNextRows(&hRow),S_OK);
	TESTC_(SetRowPosition(pIRowPosition2, DB_NULL_HCHAPTER,hRow,DBPOSITION_OK),S_OK);

	TESTC_(GetNextRows(&hRow2),S_OK);
	TESTC(VerifyGetRowPosition(pIRowPosition2, DB_NULL_HCHAPTER, hRow, DBPOSITION_OK));

CLEANUP:
	ReleaseRows(hRow);
	ReleaseRows(hRow2);
	SAFE_RELEASE(pIRowPosition);
	SAFE_RELEASE(pIRowPosition2);
	TRETURN
}
// }}




// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetRowPosition::Variation_13()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc Related - Make sure GetnextRows is not being interfered with
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetRowPosition::Variation_14()
{
	TBEGIN
	HROW hRow=DB_NULL_HROW;
	HROW hRow2=DB_NULL_HROW;
	IRowPosition* pIRowPosition = NULL;

	TESTC_(CreateRowPosition(pIRowset(), &pIRowPosition),S_OK);

	TESTC_(RestartPosition(NULL),S_OK);
	TESTC_(ClearRowPosition(pIRowPosition),S_OK);

	TESTC_(GetNextRows(&hRow),S_OK);
	TESTC_(SetRowPosition(pIRowPosition, DB_NULL_HCHAPTER,hRow,DBPOSITION_OK),S_OK);

	TESTC_(GetNextRows(&hRow2),S_OK);
	TESTC(VerifyGetRowPosition(pIRowPosition, DB_NULL_HCHAPTER, hRow, DBPOSITION_OK));

CLEANUP:
	ReleaseRows(hRow);
	ReleaseRows(hRow2);
	SAFE_RELEASE(pIRowPosition);
	TRETURN
}
// }}
// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCGetRowPosition::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCRowPosition::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCAddRelease)
//*-----------------------------------------------------------------------
//| Test Case:		TCAddRelease - Testing AddRef, Release, and QI
//|	Created:			06/23/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCAddRelease::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCRowPosition::Init())
	// }}
	{
		return TRUE;
	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc AddRef - Make sure it increments
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCAddRelease::Variation_1()
{
	TBEGIN
	IRowPosition* pIRowPosition = NULL;
	TESTC_(CreateRowPosition(pIRowset(), &pIRowPosition),S_OK);
	
	TESTC(pIRowPosition->AddRef() > 0);
	TESTC(pIRowPosition->Release() > 0);
	TESTC(VerifyRefCounts(pIRowPosition->Release(), 0));
	
CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc AddRef - 100 times, and make sure it increments
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCAddRelease::Variation_2()
{
	TBEGIN

	ULONG i=0;
	IRowPosition* pIRowPosition = NULL;

	//Create RowPosition
	TESTC_(CreateRowPosition(pIRowset(), &pIRowPosition),S_OK);

	for(i=0; i<100; i++)
	{
		TESTC(pIRowPosition->AddRef() > 0);
	}
	
	for(i=0; i<100; i++)
	{
		TESTC(pIRowPosition->Release() > 0);
	}

	TESTC(VerifyRefCounts(pIRowPosition->Release(), 0));

CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc AddRef - and Release combo 100 times
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCAddRelease::Variation_3()
{
	TBEGIN

	ULONG i=0;
	IRowPosition* pIRowPosition = NULL;
	TESTC_(CreateRowPosition(pIRowset(), &pIRowPosition),S_OK);

	for(i=0; i<100; i++)
	{
		TESTC(pIRowPosition->AddRef() > 0);
		TESTC(pIRowPosition->Release() > 0);
	}

	TESTC(VerifyRefCounts(pIRowPosition->Release(), 0));

CLEANUP:
	TRETURN
}
// }}




// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAddRelease::Variation_4()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Release - Make sure it decrements
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCAddRelease::Variation_5()
{
	TBEGIN
	IRowPosition* pIRowPosition = NULL;
	TESTC_(CreateRowPosition(pIRowset(), &pIRowPosition),S_OK);

	TESTC(pIRowPosition->AddRef() > 0);
	TESTC(pIRowPosition->Release() > 0);
	TESTC(VerifyRefCounts(pIRowPosition->Release(), 0));

CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Release - 100 times and make sure it decrements
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCAddRelease::Variation_6()
{
	TBEGIN

	ULONG i=0;
	IRowPosition* pIRowPosition = NULL;
	TESTC_(CreateRowPosition(pIRowset(), &pIRowPosition),S_OK);

	for(i=0; i<100; i++)
	{
		TESTC(pIRowPosition->AddRef() > 0);
	}
	
	for(i=0; i<100; i++)
	{
		TESTC(pIRowPosition->Release() > 0);
	}

	TESTC(VerifyRefCounts(pIRowPosition->Release(), 0));

CLEANUP:
	TRETURN
}
// }}




// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAddRelease::Variation_7()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc QI - for IRowPosition
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCAddRelease::Variation_8()
{
	TBEGIN
	IRowPosition* pIRowPosition = NULL;
	IRowPosition* pIRowPosition2 = NULL;

	TESTC_(QI(pIRowPos(), IID_IRowPosition, (void**)&pIRowPosition),S_OK);
	TESTC_(QI(pIRowPosition, IID_IRowPosition, (void**)&pIRowPosition2),S_OK);
	
CLEANUP:
	SAFE_RELEASE(pIRowPosition);
	SAFE_RELEASE(pIRowPosition2);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc QI - for someting non-existent
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCAddRelease::Variation_9()
{

TBEGIN
	IUnknown* pIUnknown = NULL;

	TESTC_(QI(pIRowPos(), IID_NULL, (void**)&pIUnknown), E_NOINTERFACE);
	TESTC(pIUnknown== NULL);

CLEANUP:
	SAFE_RELEASE(pIUnknown);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc QI - for IUnknown
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCAddRelease::Variation_10()
{

TBEGIN
	IUnknown* pIUnknown = NULL;
	IUnknown* pIUnknown2 = NULL;

	TESTC_(QI(pIRowPos(), IID_IUnknown, (void**)&pIUnknown),S_OK);
	TESTC_(QI(pIUnknown, IID_IUnknown, (void**)&pIUnknown2),S_OK);

	TESTC(pIUnknown == pIUnknown2);
	
CLEANUP:
	SAFE_RELEASE(pIUnknown);
	SAFE_RELEASE(pIUnknown2);
	TRETURN
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCAddRelease::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCRowPosition::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCOnRowPositionChange)
//*-----------------------------------------------------------------------
//| Test Case:		TCOnRowPositionChange - Functions that are called when a row changes
//|	Created:			06/23/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCOnRowPositionChange::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCRowPosition::Init())
	// }}
	{
		return TRUE;
	}
	return FALSE;
}

// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc General - return S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOnRowPositionChange::Variation_1()
{
	TBEGIN
	ULONG rgCookies[MAX_LISTENERS];
	IRowPosition* pIRowPosition = NULL;
	
	//CreateRowPosition
	TESTC_(CreateRowPosition(pIRowset(), &pIRowPosition),S_OK);
	
	//Advise all Listeners
	TESTC(AdviseAll(rgCookies, pIRowPosition));
	
	//Clear RowPosition
	TESTC_(ClearRowPosition(pIRowPosition),S_OK);
	TESTC(VerifyTimesNotified(1, DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_OKTODO));
	TESTC(VerifyTimesNotified(1, DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_ABOUTTODO));
	TESTC(VerifyTimesNotified(2));

	//Set RowPosition
	TESTC_(SetRowPosition(pIRowPosition, DB_NULL_HCHAPTER, DB_NULL_HROW, DBPOSITION_EOF),S_OK);
	TESTC(VerifyTimesNotified(1, DBREASON_ROWPOSITION_CHANGED, DBEVENTPHASE_SYNCHAFTER));
	TESTC(VerifyTimesNotified(1, DBREASON_ROWPOSITION_CHANGED, DBEVENTPHASE_DIDEVENT));
	TESTC(VerifyTimesNotified(4));
	TESTC(ResetTimesNotified());

	//Clear RowPosition
	TESTC_(ClearRowPosition(pIRowPosition),S_OK);
	TESTC(VerifyTimesNotified(1, DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_OKTODO));
	TESTC(VerifyTimesNotified(1, DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_ABOUTTODO));
	TESTC(VerifyTimesNotified(2));

	//Set RowPosition
	TESTC_(SetRowPosition(pIRowPosition, DB_NULL_HCHAPTER, DB_NULL_HROW, DBPOSITION_EOF),S_OK);
	TESTC(VerifyTimesNotified(1, DBREASON_ROWPOSITION_CHANGED, DBEVENTPHASE_SYNCHAFTER));
	TESTC(VerifyTimesNotified(1, DBREASON_ROWPOSITION_CHANGED, DBEVENTPHASE_DIDEVENT));
	TESTC(VerifyTimesNotified(4));
	TESTC(ResetTimesNotified());
	
CLEANUP:
	UnadviseAll(rgCookies, pIRowPosition);
	SAFE_RELEASE(pIRowPosition);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc General - return S_FALSE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOnRowPositionChange::Variation_2()
{
	TBEGIN
	ULONG rgCookies[MAX_LISTENERS];
	IRowPosition* pIRowPosition = NULL;
	
	//CreateRowPosition
	TESTC_(CreateRowPosition(pIRowset(), &pIRowPosition),S_OK);
	
	//Advise all Listeners
	TESTC(AdviseAll(rgCookies, pIRowPosition));
	
	//Clear RowPosition
	TESTC(m_rgpListeners[0]->SetCancel(DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_ABOUTTODO));
	TESTC_(ClearRowPosition(pIRowPosition),DB_E_CANCELED);

	//Verify Results
	TESTC(VerifyFailedTodo(DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_ABOUTTODO));
	TESTC(ResetTimesNotified());
	
	//SetRowPosition
	TESTC_(SetRowPosition(pIRowPosition, DB_NULL_HCHAPTER,DB_NULL_HROW,DBPOSITION_EOF),E_UNEXPECTED);
	TESTC(VerifyTimesNotified(0));

	//Clear RowPosition
	TESTC_(ClearRowPosition(pIRowPosition),DB_E_CANCELED);
	TESTC(VerifyFailedTodo(DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_ABOUTTODO));

CLEANUP:
	UnadviseAll(rgCookies, pIRowPosition);
	SAFE_RELEASE(pIRowPosition);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc General - return S_FALSE on an fCantDeny.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOnRowPositionChange::Variation_3()
{
	TBEGIN
	ULONG rgCookies[MAX_LISTENERS];
	IRowPosition* pIRowPosition = NULL;
	
	//CreateRowPosition
	TESTC_(CreateRowPosition(pIRowset(), &pIRowPosition),S_OK);
	
	//Advise all Listeners
	TESTC(AdviseAll(rgCookies, pIRowPosition));
	TESTC(m_rgpListeners[0]->SetCancel(DBREASON_ROWPOSITION_CHANGED, DBEVENTPHASE_SYNCHAFTER));

	//Clear RowPosition
	TESTC_(ClearRowPosition(pIRowPosition),S_OK);
	TESTC(VerifyTimesNotified(1, DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_OKTODO));
	TESTC(VerifyTimesNotified(1, DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_ABOUTTODO));
	TESTC(VerifyTimesNotified(2));

	//SetRowPosition
	//You cannot veto SYNCHAFTER and DIDEVENT phases of DBREASON_ROWPOSITION_CHANGED
	TESTC_(SetRowPosition(pIRowPosition, DB_NULL_HCHAPTER,DB_NULL_HROW,DBPOSITION_EOF),S_OK);
	TESTC(VerifyTimesNotified(1, DBREASON_ROWPOSITION_CHANGED, DBEVENTPHASE_SYNCHAFTER));
	TESTC(VerifyTimesNotified(1, DBREASON_ROWPOSITION_CHANGED, DBEVENTPHASE_DIDEVENT));
	TESTC(VerifyTimesNotified(4));

	//Clear RowPosition
	TESTC(ResetTimesNotified());
	TESTC_(ClearRowPosition(pIRowPosition),S_OK);
	TESTC(VerifyTimesNotified(1, DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_OKTODO));
	TESTC(VerifyTimesNotified(1, DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_ABOUTTODO));
	TESTC(VerifyTimesNotified(2));

CLEANUP:
	UnadviseAll(rgCookies, pIRowPosition);
	SAFE_RELEASE(pIRowPosition);
	TRETURN
}
// }}

// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc General - return DB_S_UNWANTEDPHASE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOnRowPositionChange::Variation_4()
{
	TBEGIN
	ULONG rgCookies[MAX_LISTENERS];
	IRowPosition* pIRowPosition = NULL;
	
	//CreateRowPosition
	TESTC_(CreateRowPosition(pIRowset(), &pIRowPosition),S_OK);
	TESTC(AdviseAll(rgCookies, pIRowPosition));

	//Set which Phases as unwanted (DB_S_UNWANTEDPHASE)
	TESTC(m_rgpListeners[0]->SetEvent(DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_OKTODO, FALSE));
	TESTC(m_rgpListeners[1]->SetEvent(DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_ABOUTTODO, FALSE));
	TESTC(m_rgpListeners[2]->SetEvent(DBREASON_ROWPOSITION_CHANGED, DBEVENTPHASE_SYNCHAFTER, FALSE));
	TESTC(m_rgpListeners[0]->SetEvent(DBREASON_ROWPOSITION_CHANGED, DBEVENTPHASE_DIDEVENT, FALSE));
	TESTC(m_rgpListeners[1]->SetEvent(DBREASON_ROWPOSITION_CHANGED, DBEVENTPHASE_FAILEDTODO, FALSE));

	//ClearRowPosition
	TESTC_(ClearRowPosition(pIRowPosition),S_OK);

	//The first time all should have been notified.
	//You have to be notified first before you can return DB_S_UNWANTEDPHASE
	TESTC(VerifyTimesNotified(1, DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_OKTODO));
	TESTC(VerifyTimesNotified(1, DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_ABOUTTODO));
	TESTC(VerifyTimesNotified(2));

	//SetRowPosition
	TESTC_(SetRowPosition(pIRowPosition, DB_NULL_HCHAPTER,DB_NULL_HROW,DBPOSITION_EOF),S_OK);
	TESTC(VerifyTimesNotified(1, DBREASON_ROWPOSITION_CHANGED, DBEVENTPHASE_SYNCHAFTER));
	TESTC(VerifyTimesNotified(1, DBREASON_ROWPOSITION_CHANGED, DBEVENTPHASE_DIDEVENT));
	TESTC(VerifyTimesNotified(4));
	TESTC(ResetTimesNotified());

	//Now that all phases have been notified, the Provider Should make a 
	//note of which phases I wasn't interested in and not fire them this time...
	TESTC_(ClearRowPosition(pIRowPosition),S_OK);
	TESTC(0 == m_rgpListeners[0]->GetTimesNotified(DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_OKTODO));
	TESTC(1 == m_rgpListeners[1]->GetTimesNotified(DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_OKTODO));
	TESTC(1 == m_rgpListeners[2]->GetTimesNotified(DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_OKTODO));
	TESTC(1 == m_rgpListeners[0]->GetTimesNotified(DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_ABOUTTODO));
	TESTC(0 == m_rgpListeners[1]->GetTimesNotified(DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_ABOUTTODO));
	TESTC(1 == m_rgpListeners[2]->GetTimesNotified(DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_ABOUTTODO));
	
	//Now that all phases have been notified, the Provider Should make a 
	//note of which phases I wasn't interested in and not fire them this time...
	TESTC_(SetRowPosition(pIRowPosition, DB_NULL_HCHAPTER,DB_NULL_HROW,DBPOSITION_EOF),S_OK);
	TESTC(1 == m_rgpListeners[0]->GetTimesNotified(DBREASON_ROWPOSITION_CHANGED, DBEVENTPHASE_SYNCHAFTER));
	TESTC(1 == m_rgpListeners[1]->GetTimesNotified(DBREASON_ROWPOSITION_CHANGED, DBEVENTPHASE_SYNCHAFTER));
	TESTC(0 == m_rgpListeners[2]->GetTimesNotified(DBREASON_ROWPOSITION_CHANGED, DBEVENTPHASE_SYNCHAFTER));
	TESTC(0 == m_rgpListeners[0]->GetTimesNotified(DBREASON_ROWPOSITION_CHANGED, DBEVENTPHASE_DIDEVENT));
	TESTC(1 == m_rgpListeners[1]->GetTimesNotified(DBREASON_ROWPOSITION_CHANGED, DBEVENTPHASE_DIDEVENT));
	TESTC(1 == m_rgpListeners[2]->GetTimesNotified(DBREASON_ROWPOSITION_CHANGED, DBEVENTPHASE_DIDEVENT));

	//Now we need to generate the FAILEDTODO phase as well...
	TESTC(RestartListeners());
	TESTC(m_rgpListeners[0]->SetCancel(DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_ABOUTTODO));
	TESTC(m_rgpListeners[0]->SetEvent(DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_FAILEDTODO, FALSE));
	
	//Verify results, should all have received FAILEDTODO for the first time
	TESTC_(ClearRowPosition(pIRowPosition),DB_E_CANCELED);
	TESTC(1 == m_rgpListeners[0]->GetTimesNotified(DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_FAILEDTODO));
	TESTC(1 == m_rgpListeners[1]->GetTimesNotified(DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_FAILEDTODO));
	TESTC(1 == m_rgpListeners[2]->GetTimesNotified(DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_FAILEDTODO));
	TESTC(ResetTimesNotified());

	//Now that all phases have been notified, the Provider Should make a 
	//note of which phases I wasn't interested in and not fire them this time...
	TESTC_(ClearRowPosition(pIRowPosition),DB_E_CANCELED);
	TESTC(0 == m_rgpListeners[0]->GetTimesNotified(DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_FAILEDTODO));
	TESTC(1 == m_rgpListeners[1]->GetTimesNotified(DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_FAILEDTODO));
	TESTC(1 == m_rgpListeners[2]->GetTimesNotified(DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_FAILEDTODO));

CLEANUP:
	UnadviseAll(rgCookies, pIRowPosition);
	SAFE_RELEASE(pIRowPosition);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc General - return DB_S_UNWANTEDREASON
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOnRowPositionChange::Variation_5()
{
	TBEGIN
	ULONG rgCookies[MAX_LISTENERS];
	IRowPosition* pIRowPosition = NULL;
	
	//CreateRowPosition
	TESTC_(CreateRowPosition(pIRowset(), &pIRowPosition),S_OK);
	TESTC(AdviseAll(rgCookies, pIRowPosition));

	//Set which reasons as unwanted (DB_S_UNWANTEDREASON)
	TESTC(m_rgpListeners[0]->SetEvent(DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_ALL, FALSE));
	TESTC(m_rgpListeners[0]->SetEvent(DBREASON_ROWPOSITION_CHANGED, DBEVENTPHASE_ALL, FALSE));
	TESTC(m_rgpListeners[2]->SetEvent(DBREASON_ROWPOSITION_CHANGED, DBEVENTPHASE_ALL, FALSE));

	//ClearRowPosition
	TESTC_(ClearRowPosition(pIRowPosition),S_OK);

	//The first time all should have been notified, for only the first phase
	//You have to be notified first before you can return DB_S_UNWANTEDREASON
	TESTC(1 == m_rgpListeners[0]->GetTimesNotified(DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_OKTODO));
	TESTC(0 == m_rgpListeners[0]->GetTimesNotified(DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_ABOUTTODO));
	TESTC(1 == m_rgpListeners[1]->GetTimesNotified(DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_OKTODO));
	TESTC(1 == m_rgpListeners[1]->GetTimesNotified(DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_ABOUTTODO));
	TESTC(1 == m_rgpListeners[2]->GetTimesNotified(DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_OKTODO));
	TESTC(1 == m_rgpListeners[2]->GetTimesNotified(DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_ABOUTTODO));
	TESTC(ResetTimesNotified());

	//SetRowPosition
	TESTC_(SetRowPosition(pIRowPosition, DB_NULL_HCHAPTER,DB_NULL_HROW,DBPOSITION_EOF),S_OK);
	TESTC(1 == m_rgpListeners[0]->GetTimesNotified(DBREASON_ROWPOSITION_CHANGED, DBEVENTPHASE_SYNCHAFTER));
	TESTC(0 == m_rgpListeners[0]->GetTimesNotified(DBREASON_ROWPOSITION_CHANGED, DBEVENTPHASE_DIDEVENT));
	TESTC(1 == m_rgpListeners[1]->GetTimesNotified(DBREASON_ROWPOSITION_CHANGED, DBEVENTPHASE_SYNCHAFTER));
	TESTC(1 == m_rgpListeners[1]->GetTimesNotified(DBREASON_ROWPOSITION_CHANGED, DBEVENTPHASE_DIDEVENT));
	TESTC(1 == m_rgpListeners[2]->GetTimesNotified(DBREASON_ROWPOSITION_CHANGED, DBEVENTPHASE_SYNCHAFTER));
	TESTC(0 == m_rgpListeners[2]->GetTimesNotified(DBREASON_ROWPOSITION_CHANGED, DBEVENTPHASE_DIDEVENT));
	TESTC(ResetTimesNotified());

	//Now that all phases have been notified, the Provider Should make a 
	//note of which reasons I wasn't interested in and not fire them this time...
	TESTC_(ClearRowPosition(pIRowPosition),S_OK);
	TESTC(0 == m_rgpListeners[0]->GetTimesNotified(DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_OKTODO));
	TESTC(0 == m_rgpListeners[0]->GetTimesNotified(DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_ABOUTTODO));
	TESTC(1 == m_rgpListeners[1]->GetTimesNotified(DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_OKTODO));
	TESTC(1 == m_rgpListeners[1]->GetTimesNotified(DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_ABOUTTODO));
	TESTC(1 == m_rgpListeners[2]->GetTimesNotified(DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_OKTODO));
	TESTC(1 == m_rgpListeners[2]->GetTimesNotified(DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_ABOUTTODO));
	TESTC(ResetTimesNotified());
	
	//Now that all phases have been notified, the Provider Should make a 
	//note of which phases I wasn't interested in and not fire them this time...
	TESTC_(SetRowPosition(pIRowPosition, DB_NULL_HCHAPTER,DB_NULL_HROW,DBPOSITION_EOF),S_OK);
	TESTC(0 == m_rgpListeners[0]->GetTimesNotified(DBREASON_ROWPOSITION_CHANGED, DBEVENTPHASE_SYNCHAFTER));
	TESTC(0 == m_rgpListeners[0]->GetTimesNotified(DBREASON_ROWPOSITION_CHANGED, DBEVENTPHASE_DIDEVENT));
	TESTC(1 == m_rgpListeners[1]->GetTimesNotified(DBREASON_ROWPOSITION_CHANGED, DBEVENTPHASE_SYNCHAFTER));
	TESTC(1 == m_rgpListeners[1]->GetTimesNotified(DBREASON_ROWPOSITION_CHANGED, DBEVENTPHASE_DIDEVENT));
	TESTC(0 == m_rgpListeners[2]->GetTimesNotified(DBREASON_ROWPOSITION_CHANGED, DBEVENTPHASE_SYNCHAFTER));
	TESTC(0 == m_rgpListeners[2]->GetTimesNotified(DBREASON_ROWPOSITION_CHANGED, DBEVENTPHASE_DIDEVENT));
	TESTC(ResetTimesNotified());

	//Now we need to generate the FAILEDTODO phase as well...
	TESTC(ResetTimesNotified());
	TESTC(m_rgpListeners[1]->SetCancel(DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_ABOUTTODO));
	TESTC_(ClearRowPosition(pIRowPosition),DB_E_CANCELED);
	
	//Verify results, should not have been notified FAILEDTODO for the one not interested
	TESTC(ResetTimesNotified());
	TESTC_(ClearRowPosition(pIRowPosition),DB_E_CANCELED);
	TESTC(0 == m_rgpListeners[0]->GetTimesNotified(DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_FAILEDTODO));
	TESTC(1 == m_rgpListeners[1]->GetTimesNotified(DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_FAILEDTODO));
	TESTC(1 == m_rgpListeners[2]->GetTimesNotified(DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_FAILEDTODO));

CLEANUP:
	UnadviseAll(rgCookies, pIRowPosition);
	SAFE_RELEASE(pIRowPosition);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc General - return E_FAIL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOnRowPositionChange::Variation_6()
{
	TBEGIN
	ULONG rgCookies[MAX_LISTENERS];
	IRowPosition* pIRowPosition = NULL;
	
	//CreateRowPosition
	TESTC_(CreateRowPosition(pIRowset(), &pIRowPosition),S_OK);
	
	//Advise all Listeners
	TESTC(AdviseAll(rgCookies, pIRowPosition));

	//Set E_FAIL
	TESTC(m_rgpListeners[0]->SetError(E_FAIL));
	TESTC(m_rgpListeners[1]->SetError(E_FAIL));
	
	//Clear RowPosition
	TESTC_(ClearRowPosition(pIRowPosition),S_OK);
	TESTC(VerifyTimesNotified(1, DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_OKTODO));
	TESTC(VerifyTimesNotified(1, DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_ABOUTTODO));
	TESTC(VerifyTimesNotified(2));

	//Set RowPosition
	TESTC_(SetRowPosition(pIRowPosition, DB_NULL_HCHAPTER, DB_NULL_HROW, DBPOSITION_EOF),S_OK);
	TESTC(VerifyTimesNotified(1, DBREASON_ROWPOSITION_CHANGED, DBEVENTPHASE_SYNCHAFTER));
	TESTC(VerifyTimesNotified(1, DBREASON_ROWPOSITION_CHANGED, DBEVENTPHASE_DIDEVENT));
	TESTC(VerifyTimesNotified(4));
	TESTC(ResetTimesNotified());

	//Clear RowPosition
	TESTC_(ClearRowPosition(pIRowPosition),S_OK);
	TESTC(VerifyTimesNotified(1, DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_OKTODO));
	TESTC(VerifyTimesNotified(1, DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_ABOUTTODO));
	TESTC(VerifyTimesNotified(2));

	//Set RowPosition
	TESTC_(SetRowPosition(pIRowPosition, DB_NULL_HCHAPTER, DB_NULL_HROW, DBPOSITION_EOF),S_OK);
	TESTC(VerifyTimesNotified(1, DBREASON_ROWPOSITION_CHANGED, DBEVENTPHASE_SYNCHAFTER));
	TESTC(VerifyTimesNotified(1, DBREASON_ROWPOSITION_CHANGED, DBEVENTPHASE_DIDEVENT));
	TESTC(VerifyTimesNotified(4));
	TESTC(ResetTimesNotified());
	
CLEANUP:
	UnadviseAll(rgCookies, pIRowPosition);
	SAFE_RELEASE(pIRowPosition);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc General - return E_OUTOFMEMORY
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOnRowPositionChange::Variation_7()
{
	TBEGIN
	ULONG rgCookies[MAX_LISTENERS];
	IRowPosition* pIRowPosition = NULL;
	
	//CreateRowPosition
	TESTC_(CreateRowPosition(pIRowset(), &pIRowPosition),S_OK);
	
	//Advise all Listeners
	TESTC(AdviseAll(rgCookies, pIRowPosition));

	//Set E_FAIL
	TESTC(m_rgpListeners[0]->SetError(E_OUTOFMEMORY));
	TESTC(m_rgpListeners[1]->SetError(E_OUTOFMEMORY));
	
	//Clear RowPosition
	TESTC_(ClearRowPosition(pIRowPosition),S_OK);
	TESTC(VerifyTimesNotified(1, DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_OKTODO));
	TESTC(VerifyTimesNotified(1, DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_ABOUTTODO));
	TESTC(VerifyTimesNotified(2));

	//Set RowPosition
	TESTC_(SetRowPosition(pIRowPosition, DB_NULL_HCHAPTER, DB_NULL_HROW, DBPOSITION_EOF),S_OK);
	TESTC(VerifyTimesNotified(1, DBREASON_ROWPOSITION_CHANGED, DBEVENTPHASE_SYNCHAFTER));
	TESTC(VerifyTimesNotified(1, DBREASON_ROWPOSITION_CHANGED, DBEVENTPHASE_DIDEVENT));
	TESTC(VerifyTimesNotified(4));
	TESTC(ResetTimesNotified());

	//Clear RowPosition
	TESTC_(ClearRowPosition(pIRowPosition),S_OK);
	TESTC(VerifyTimesNotified(1, DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_OKTODO));
	TESTC(VerifyTimesNotified(1, DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_ABOUTTODO));
	TESTC(VerifyTimesNotified(2));

	//Set RowPosition
	TESTC_(SetRowPosition(pIRowPosition, DB_NULL_HCHAPTER, DB_NULL_HROW, DBPOSITION_EOF),S_OK);
	TESTC(VerifyTimesNotified(1, DBREASON_ROWPOSITION_CHANGED, DBEVENTPHASE_SYNCHAFTER));
	TESTC(VerifyTimesNotified(1, DBREASON_ROWPOSITION_CHANGED, DBEVENTPHASE_DIDEVENT));
	TESTC(VerifyTimesNotified(4));
	TESTC(ResetTimesNotified());
	
CLEANUP:
	UnadviseAll(rgCookies, pIRowPosition);
	SAFE_RELEASE(pIRowPosition);
	TRETURN
}
// }}




// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCOnRowPositionChange::Variation_8()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc Test in all four phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOnRowPositionChange::Variation_9()
{
	TBEGIN
	ULONG rgCookies[MAX_LISTENERS];
	IRowPosition* pIRowPosition = NULL;
	
	//CreateRowPosition
	TESTC_(CreateRowPosition(pIRowset(), &pIRowPosition),S_OK);
	
	//Advise all Listeners
	TESTC(AdviseAll(rgCookies, pIRowPosition));

	//Clear RowPosition
	TESTC_(ClearRowPosition(pIRowPosition),S_OK);
	TESTC(VerifyTimesNotified(1, DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_OKTODO));
	TESTC(VerifyTimesNotified(1, DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_ABOUTTODO));
	TESTC(VerifyTimesNotified(2));

	//Set RowPosition
	TESTC_(SetRowPosition(pIRowPosition, DB_NULL_HCHAPTER, DB_NULL_HROW, DBPOSITION_EOF),S_OK);
	TESTC(VerifyTimesNotified(1, DBREASON_ROWPOSITION_CHANGED, DBEVENTPHASE_SYNCHAFTER));
	TESTC(VerifyTimesNotified(1, DBREASON_ROWPOSITION_CHANGED, DBEVENTPHASE_DIDEVENT));
	TESTC(VerifyTimesNotified(4));
	TESTC(ResetTimesNotified());

	//Clear RowPosition
	TESTC_(ClearRowPosition(pIRowPosition),S_OK);
	TESTC(VerifyTimesNotified(1, DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_OKTODO));
	TESTC(VerifyTimesNotified(1, DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_ABOUTTODO));
	TESTC(VerifyTimesNotified(2));

	//Set RowPosition
	TESTC_(SetRowPosition(pIRowPosition, DB_NULL_HCHAPTER, DB_NULL_HROW, DBPOSITION_EOF),S_OK);
	TESTC(VerifyTimesNotified(1, DBREASON_ROWPOSITION_CHANGED, DBEVENTPHASE_SYNCHAFTER));
	TESTC(VerifyTimesNotified(1, DBREASON_ROWPOSITION_CHANGED, DBEVENTPHASE_DIDEVENT));
	TESTC(VerifyTimesNotified(4));
	TESTC(ResetTimesNotified());
	
CLEANUP:
	UnadviseAll(rgCookies, pIRowPosition);
	SAFE_RELEASE(pIRowPosition);
	TRETURN
}
// }}




// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCOnRowPositionChange::Variation_10()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(11)
//--------------------------------------------------------------------
// @mfunc Sequence - Advise inbetween seperate notifications
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOnRowPositionChange::Variation_11()
{
	TBEGIN
	ULONG rgCookies[MAX_LISTENERS*2];
	IRowPosition* pIRowPosition = NULL;
	
	//CreateRowPosition
	TESTC_(CreateRowPosition(pIRowset(), &pIRowPosition),S_OK);
	
	//Advise all Listeners
	TESTC(AdviseAll(rgCookies, pIRowPosition));

	//Clear RowPosition
	TESTC_(ClearRowPosition(pIRowPosition),S_OK);
	TESTC(VerifyTimesNotified(1, DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_OKTODO));
	TESTC(VerifyTimesNotified(1, DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_ABOUTTODO));
	TESTC(VerifyTimesNotified(2));

	//Set RowPosition
	TESTC_(SetRowPosition(pIRowPosition, DB_NULL_HCHAPTER, DB_NULL_HROW, DBPOSITION_EOF),S_OK);
	TESTC(VerifyTimesNotified(1, DBREASON_ROWPOSITION_CHANGED, DBEVENTPHASE_SYNCHAFTER));
	TESTC(VerifyTimesNotified(1, DBREASON_ROWPOSITION_CHANGED, DBEVENTPHASE_DIDEVENT));
	TESTC(VerifyTimesNotified(4));

	//Advise all Listeners (again)
	TESTC(AdviseAll(&rgCookies[MAX_LISTENERS], pIRowPosition));

	//Clear RowPosition
	TESTC(ResetTimesNotified());
	TESTC_(ClearRowPosition(pIRowPosition),S_OK);
	TESTC(VerifyTimesNotified(2, DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_OKTODO));
	TESTC(VerifyTimesNotified(2, DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_ABOUTTODO));
	TESTC(VerifyTimesNotified(4));

	//Set RowPosition
	TESTC_(SetRowPosition(pIRowPosition, DB_NULL_HCHAPTER, DB_NULL_HROW, DBPOSITION_EOF),S_OK);
	TESTC(VerifyTimesNotified(2, DBREASON_ROWPOSITION_CHANGED, DBEVENTPHASE_SYNCHAFTER));
	TESTC(VerifyTimesNotified(2, DBREASON_ROWPOSITION_CHANGED, DBEVENTPHASE_DIDEVENT));
	TESTC(VerifyTimesNotified(8));
	TESTC(ResetTimesNotified());


CLEANUP:
	UnadviseAll(rgCookies, pIRowPosition);
	UnadviseAll(&rgCookies[MAX_LISTENERS], pIRowPosition);
	SAFE_RELEASE(pIRowPosition);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc Sequence - Advise during a notification
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOnRowPositionChange::Variation_12()
{
	TBEGIN
	ULONG rgCookies[MAX_LISTENERS];
	
	//Advise all Listeners
	TESTC(AdviseAll(rgCookies, pIRowPos()));

	//Indicate this is the Listener to Advise during the Callback
	m_rgpListeners[2]->SetAdvise(DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_OKTODO);
	
	//Clear RowPosition
	//Listeners advised in the middle should not receive notifications
	TESTC_(ClearRowPosition(pIRowPos()),S_OK);
	TESTC(1 == m_rgpListeners[0]->GetTimesNotified(DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_OKTODO));
	TESTC(1 == m_rgpListeners[0]->GetTimesNotified(DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_ABOUTTODO));
	TESTC(1 == m_rgpListeners[1]->GetTimesNotified(DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_OKTODO));
	TESTC(1 == m_rgpListeners[1]->GetTimesNotified(DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_ABOUTTODO));
	TESTC(2 == m_rgpListeners[2]->GetTimesNotified(DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_OKTODO));
	TESTC(2 == m_rgpListeners[2]->GetTimesNotified(DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_ABOUTTODO));
	TESTC(ResetTimesNotified());

	//Set RowPosition
	TESTC_(SetRowPosition(pIRowPos(), DB_NULL_HCHAPTER, DB_NULL_HROW, DBPOSITION_EOF),S_OK);
	TESTC(1 == m_rgpListeners[0]->GetTimesNotified(DBREASON_ROWPOSITION_CHANGED, DBEVENTPHASE_SYNCHAFTER));
	TESTC(1 == m_rgpListeners[0]->GetTimesNotified(DBREASON_ROWPOSITION_CHANGED, DBEVENTPHASE_DIDEVENT));
	TESTC(1 == m_rgpListeners[1]->GetTimesNotified(DBREASON_ROWPOSITION_CHANGED, DBEVENTPHASE_SYNCHAFTER));
	TESTC(1 == m_rgpListeners[1]->GetTimesNotified(DBREASON_ROWPOSITION_CHANGED, DBEVENTPHASE_DIDEVENT));
	TESTC(2 == m_rgpListeners[2]->GetTimesNotified(DBREASON_ROWPOSITION_CHANGED, DBEVENTPHASE_SYNCHAFTER));
	TESTC(2 == m_rgpListeners[2]->GetTimesNotified(DBREASON_ROWPOSITION_CHANGED, DBEVENTPHASE_DIDEVENT));
	TESTC(ResetTimesNotified());

	m_rgpListeners[2]->SetUnadvise(DBREASON_ROWPOSITION_CHANGED, DBEVENTPHASE_DIDEVENT);

	//Clear RowPosition
	TESTC_(ClearRowPosition(pIRowPos()),S_OK);
	TESTC(1 == m_rgpListeners[0]->GetTimesNotified(DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_OKTODO));
	TESTC(1 == m_rgpListeners[0]->GetTimesNotified(DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_ABOUTTODO));
	TESTC(1 == m_rgpListeners[1]->GetTimesNotified(DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_OKTODO));
	TESTC(1 == m_rgpListeners[1]->GetTimesNotified(DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_ABOUTTODO));
	TESTC(2 == m_rgpListeners[2]->GetTimesNotified(DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_OKTODO));
	TESTC(2 == m_rgpListeners[2]->GetTimesNotified(DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_ABOUTTODO));
	TESTC(ResetTimesNotified());

	//Set RowPosition
	TESTC_(SetRowPosition(pIRowPos(), DB_NULL_HCHAPTER, DB_NULL_HROW, DBPOSITION_EOF),S_OK);
	TESTC(1 == m_rgpListeners[0]->GetTimesNotified(DBREASON_ROWPOSITION_CHANGED, DBEVENTPHASE_SYNCHAFTER));
	TESTC(1 == m_rgpListeners[0]->GetTimesNotified(DBREASON_ROWPOSITION_CHANGED, DBEVENTPHASE_DIDEVENT));
	TESTC(1 == m_rgpListeners[1]->GetTimesNotified(DBREASON_ROWPOSITION_CHANGED, DBEVENTPHASE_SYNCHAFTER));
	TESTC(1 == m_rgpListeners[1]->GetTimesNotified(DBREASON_ROWPOSITION_CHANGED, DBEVENTPHASE_DIDEVENT));
	TESTC(2 == m_rgpListeners[2]->GetTimesNotified(DBREASON_ROWPOSITION_CHANGED, DBEVENTPHASE_SYNCHAFTER));
	TESTC(1 == m_rgpListeners[2]->GetTimesNotified(DBREASON_ROWPOSITION_CHANGED, DBEVENTPHASE_DIDEVENT));
	TESTC(ResetTimesNotified());

CLEANUP:
	UnadviseAll(rgCookies, pIRowPos());
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//--------------------------------------------------------------------
// @mfunc Sequence - Unadvise inbetween seperate notification
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOnRowPositionChange::Variation_13()
{
	TBEGIN
	ULONG rgCookies[MAX_LISTENERS*2];
	IRowPosition* pIRowPosition = NULL;
	
	//CreateRowPosition
	TESTC_(CreateRowPosition(pIRowset(), &pIRowPosition),S_OK);
	
	//Advise all Listeners
	TESTC(AdviseAll(rgCookies, pIRowPosition));
	//Advise all Listeners (again)
	TESTC(AdviseAll(&rgCookies[MAX_LISTENERS], pIRowPosition));


	//Clear RowPosition
	TESTC_(ClearRowPosition(pIRowPosition),S_OK);
	TESTC(VerifyTimesNotified(2, DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_OKTODO));
	TESTC(VerifyTimesNotified(2, DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_ABOUTTODO));
	TESTC(VerifyTimesNotified(4));

	//Set RowPosition
	TESTC_(SetRowPosition(pIRowPosition, DB_NULL_HCHAPTER, DB_NULL_HROW, DBPOSITION_EOF),S_OK);
	TESTC(VerifyTimesNotified(2, DBREASON_ROWPOSITION_CHANGED, DBEVENTPHASE_SYNCHAFTER));
	TESTC(VerifyTimesNotified(2, DBREASON_ROWPOSITION_CHANGED, DBEVENTPHASE_DIDEVENT));
	TESTC(VerifyTimesNotified(8));

	//Unadvise inbetween notifications
	UnadviseAll(rgCookies, pIRowPosition);

	//Clear RowPosition
	TESTC(ResetTimesNotified());
	TESTC_(ClearRowPosition(pIRowPosition),S_OK);
	TESTC(VerifyTimesNotified(1, DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_OKTODO));
	TESTC(VerifyTimesNotified(1, DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_ABOUTTODO));
	TESTC(VerifyTimesNotified(2));

	//Set RowPosition
	TESTC_(SetRowPosition(pIRowPosition, DB_NULL_HCHAPTER, DB_NULL_HROW, DBPOSITION_EOF),S_OK);
	TESTC(VerifyTimesNotified(1, DBREASON_ROWPOSITION_CHANGED, DBEVENTPHASE_SYNCHAFTER));
	TESTC(VerifyTimesNotified(1, DBREASON_ROWPOSITION_CHANGED, DBEVENTPHASE_DIDEVENT));
	TESTC(VerifyTimesNotified(4));
	TESTC(ResetTimesNotified());


CLEANUP:
	UnadviseAll(&rgCookies[MAX_LISTENERS], pIRowPosition);
	SAFE_RELEASE(pIRowPosition);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc Sequence - Unadvise during a notification
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOnRowPositionChange::Variation_14()
{
	TBEGIN
	ULONG rgCookies[MAX_LISTENERS];
	
	//Advise all Listeners
	TESTC(AdviseAll(rgCookies, pIRowPos()));

	//Indicate this is the Listener to Advise during the Callback
	m_rgpListeners[2]->SetAdvise(DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_OKTODO);
	
	//Clear RowPosition
	//Listeners advised in the middle should not receive notifications
	TESTC_(ClearRowPosition(pIRowPos()),S_OK);
	TESTC(1 == m_rgpListeners[0]->GetTimesNotified(DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_OKTODO));
	TESTC(1 == m_rgpListeners[0]->GetTimesNotified(DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_ABOUTTODO));
	TESTC(1 == m_rgpListeners[1]->GetTimesNotified(DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_OKTODO));
	TESTC(1 == m_rgpListeners[1]->GetTimesNotified(DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_ABOUTTODO));
	TESTC(2 == m_rgpListeners[2]->GetTimesNotified(DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_OKTODO));
	TESTC(2 == m_rgpListeners[2]->GetTimesNotified(DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_ABOUTTODO));
	TESTC(ResetTimesNotified());

	//Set RowPosition
	TESTC_(SetRowPosition(pIRowPos(), DB_NULL_HCHAPTER, DB_NULL_HROW, DBPOSITION_EOF),S_OK);
	TESTC(1 == m_rgpListeners[0]->GetTimesNotified(DBREASON_ROWPOSITION_CHANGED, DBEVENTPHASE_SYNCHAFTER));
	TESTC(1 == m_rgpListeners[0]->GetTimesNotified(DBREASON_ROWPOSITION_CHANGED, DBEVENTPHASE_DIDEVENT));
	TESTC(1 == m_rgpListeners[1]->GetTimesNotified(DBREASON_ROWPOSITION_CHANGED, DBEVENTPHASE_SYNCHAFTER));
	TESTC(1 == m_rgpListeners[1]->GetTimesNotified(DBREASON_ROWPOSITION_CHANGED, DBEVENTPHASE_DIDEVENT));
	TESTC(2 == m_rgpListeners[2]->GetTimesNotified(DBREASON_ROWPOSITION_CHANGED, DBEVENTPHASE_SYNCHAFTER));
	TESTC(2 == m_rgpListeners[2]->GetTimesNotified(DBREASON_ROWPOSITION_CHANGED, DBEVENTPHASE_DIDEVENT));
	TESTC(ResetTimesNotified());

	m_rgpListeners[2]->SetUnadvise(DBREASON_ROWPOSITION_CHANGED, DBEVENTPHASE_DIDEVENT);

	//Clear RowPosition
	TESTC_(ClearRowPosition(pIRowPos()),S_OK);
	TESTC(1 == m_rgpListeners[0]->GetTimesNotified(DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_OKTODO));
	TESTC(1 == m_rgpListeners[0]->GetTimesNotified(DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_ABOUTTODO));
	TESTC(1 == m_rgpListeners[1]->GetTimesNotified(DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_OKTODO));
	TESTC(1 == m_rgpListeners[1]->GetTimesNotified(DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_ABOUTTODO));
	TESTC(2 == m_rgpListeners[2]->GetTimesNotified(DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_OKTODO));
	TESTC(2 == m_rgpListeners[2]->GetTimesNotified(DBREASON_ROWPOSITION_CLEARED, DBEVENTPHASE_ABOUTTODO));
	TESTC(ResetTimesNotified());

	//Set RowPosition
	TESTC_(SetRowPosition(pIRowPos(), DB_NULL_HCHAPTER, DB_NULL_HROW, DBPOSITION_EOF),S_OK);
	TESTC(1 == m_rgpListeners[0]->GetTimesNotified(DBREASON_ROWPOSITION_CHANGED, DBEVENTPHASE_SYNCHAFTER));
	TESTC(1 == m_rgpListeners[0]->GetTimesNotified(DBREASON_ROWPOSITION_CHANGED, DBEVENTPHASE_DIDEVENT));
	TESTC(1 == m_rgpListeners[1]->GetTimesNotified(DBREASON_ROWPOSITION_CHANGED, DBEVENTPHASE_SYNCHAFTER));
	TESTC(1 == m_rgpListeners[1]->GetTimesNotified(DBREASON_ROWPOSITION_CHANGED, DBEVENTPHASE_DIDEVENT));
	TESTC(2 == m_rgpListeners[2]->GetTimesNotified(DBREASON_ROWPOSITION_CHANGED, DBEVENTPHASE_SYNCHAFTER));
	TESTC(1 == m_rgpListeners[2]->GetTimesNotified(DBREASON_ROWPOSITION_CHANGED, DBEVENTPHASE_DIDEVENT));
	TESTC(ResetTimesNotified());

CLEANUP:
	UnadviseAll(rgCookies, pIRowPos());
	TRETURN
}
// }}




// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCOnRowPositionChange::Variation_15()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc SetRowPosition - Error Cases - FAILEDTODO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOnRowPositionChange::Variation_16()
{
	TBEGIN
	ULONG rgCookies[MAX_LISTENERS];
	IRowPosition* pIRowPosition = NULL;
	
	//CreateRowPosition
	TESTC_(CreateRowPosition(pIRowset(), &pIRowPosition),S_OK);
	
	//Advise all Listeners
	TESTC(AdviseAll(rgCookies, pIRowPosition));
	
	//Clear RowPosition
	TESTC_(ClearRowPosition(pIRowPosition),S_OK);

	//SetRowPosition
	TESTC_(SetRowPosition(pIRowPosition, DB_NULL_HCHAPTER, DB_NULL_HROW, DBPOSITION_OK),E_INVALIDARG);
	TESTC(VerifyFailedTodo(DBREASON_ROWPOSITION_CHANGED, DBEVENTPHASE_ABOUTTODO));

CLEANUP:
	UnadviseAll(rgCookies, pIRowPosition);
	SAFE_RELEASE(pIRowPosition);
	TRETURN
}
// }}
// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCOnRowPositionChange::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCRowPosition::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCConnectionStuff)
//*-----------------------------------------------------------------------
//| Test Case:		TCConnectionStuff - Testing all the connection routines
//|	Created:			06/26/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCConnectionStuff::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCRowPosition::Init())
	// }}
	{
		return TRUE;
	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc General: Container: QI for IID_IConnectionPointContainer
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCConnectionStuff::Variation_1()
{
	TBEGIN
	IConnectionPointContainer* pICPC= NULL;
	IUnknown* pIUnknown= NULL;

	TESTC_(QI(pIRowPos(), IID_IConnectionPointContainer, (void**)&pICPC),S_OK);
	TESTC_(QI(pICPC, IID_IConnectionPointContainer, (void**)&pIUnknown),S_OK);
	TESTC(pICPC != NULL && pIUnknown != NULL);
	
	SAFE_RELEASE(pIUnknown);
	TESTC_(pICPC->QueryInterface(IID_IUnknown,(void**)&pIUnknown),S_OK);
	TESTC(pIUnknown != NULL);

CLEANUP:
	SAFE_RELEASE(pIUnknown);
	SAFE_RELEASE(pICPC);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Container: AddRef
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCConnectionStuff::Variation_2()
{
	TBEGIN
	IConnectionPointContainer* pICPC = NULL;

	//Get Current RefCount
	ULONG ulOrgRefCount = GetRefCount(pIRowPos());

	TESTC_(QI(pIRowPos(), IID_IConnectionPointContainer, (void**)&pICPC),S_OK);
	TESTC(VerifyRefCounts(GetRefCount(pICPC), ulOrgRefCount+1));

CLEANUP:
	SAFE_RELEASE(pICPC);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Container: Release
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCConnectionStuff::Variation_3()
{
	TBEGIN
	IConnectionPointContainer* pICPC = NULL;

	//Get Current RefCount
	ULONG ulOrgRefCount = GetRefCount(pIRowPos());
	TESTC_(QI(pIRowPos(), IID_IConnectionPointContainer, (void**)&pICPC),S_OK);
	
	TESTC(VerifyRefCounts(GetRefCount(pICPC), ulOrgRefCount+1));
	TESTC(VerifyRefCounts(pICPC->Release(), ulOrgRefCount));

CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Container: FindConnectionPoint for a connection
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCConnectionStuff::Variation_4()
{
	TBEGIN
	IConnectionPoint* pIConnectionPoint= NULL;

	TESTC_(pICPC()->FindConnectionPoint(IID_IRowPositionChange, &pIConnectionPoint),S_OK);

CLEANUP:
	SAFE_RELEASE(pIConnectionPoint);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Container EnumConnectionPoints
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCConnectionStuff::Variation_5()
{
	TBEGIN
	IEnumConnectionPoints* pIEnum= NULL;

	TESTC_(pICPC()->EnumConnectionPoints(&pIEnum),S_OK);
	TESTC(pIEnum != NULL);

CLEANUP:
	SAFE_RELEASE(pIEnum);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Container: Next should work until there are no more
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCConnectionStuff::Variation_6()
{
	TBEGIN
	IConnectionPoint* pIConnectionPoint;
	IConnectionPoint* pIConnectionPoint2 = INVALID(IConnectionPoint*);
	ULONG cFetched = 505; //A random start value.

	//Reset
	TESTC_(pIEnumCP()->Reset(),S_OK);
	
	//Should only be 1 connectionpoint 
	TESTC_(pIEnumCP()->Next(1, &pIConnectionPoint, &cFetched),S_OK);
	TESTC(cFetched == 1 && pIConnectionPoint != NULL);
	
	TESTC_(pIEnumCP()->Next(1, &pIConnectionPoint2, &cFetched),S_FALSE);
	TESTC(cFetched == 0);
	
CLEANUP:
	SAFE_RELEASE(pIConnectionPoint);
	if(pIConnectionPoint2 != INVALID(IConnectionPoint*))
		SAFE_RELEASE(pIConnectionPoint2);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Container: Clone
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCConnectionStuff::Variation_7()
{
	TBEGIN
	IConnectionPoint* pIConnectionPoint = NULL;
	IConnectionPoint* pIConnectionPoint2 = NULL;
	IEnumConnectionPoints* pIEnumCP2 = NULL;
	ULONG cFetched=505;//A random start value.

	//Obtain first connectionpoint
	TESTC_(pIEnumCP()->Reset(),S_OK);
	TESTC_(pIEnumCP()->Next(1, &pIConnectionPoint, &cFetched),S_OK);
	TESTC(cFetched == 1);
		
	//Clone
	TESTC_(pIEnumCP()->Clone(&pIEnumCP2),S_OK);
	TESTC_(pIEnumCP2->Next(1, &pIConnectionPoint2, &cFetched),S_FALSE);
	TESTC(cFetched == 0 && pIConnectionPoint2 == NULL);

	//Verify actually works though
	TESTC_(pIEnumCP2->Reset(),S_OK);
	TESTC_(pIEnumCP2->Next(1, &pIConnectionPoint2, &cFetched),S_OK);
	TESTC(cFetched == 1 && pIConnectionPoint == pIConnectionPoint2);

CLEANUP:
	SAFE_RELEASE(pIConnectionPoint);
	SAFE_RELEASE(pIConnectionPoint2);
	SAFE_RELEASE(pIEnumCP2);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc Container: Reset should go to the beginning
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCConnectionStuff::Variation_8()
{
	TBEGIN
	IConnectionPoint* pIConnectionPoint= NULL;
	ULONG cFetched=505;//A random start value.

	TESTC_(pIEnumCP()->Reset(),S_OK);
	TESTC_(pIEnumCP()->Next(1,&pIConnectionPoint,&cFetched),S_OK);
	TESTC(cFetched == 1);
	SAFE_RELEASE(pIConnectionPoint);

	TESTC_(pIEnumCP()->Reset(),S_OK);
	TESTC_(pIEnumCP()->Next(1,&pIConnectionPoint,&cFetched),S_OK);
	TESTC(cFetched == 1);

CLEANUP:
	SAFE_RELEASE(pIConnectionPoint);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc Container: The Second one should still be on the end
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCConnectionStuff::Variation_9()
{
	TBEGIN
	IConnectionPoint* pIConnectionPoint= NULL;
	IEnumConnectionPoints* pIEnumCP2= NULL;
	ULONG cFetched=505;//A random start value.

	//Go to the end
	TESTC_(pIEnumCP()->Reset(),S_OK);
	TESTC_(pIEnumCP()->Next(1,&pIConnectionPoint,&cFetched),S_OK);
	TESTC(cFetched == 1);
	SAFE_RELEASE(pIConnectionPoint);

	//Clone
	TESTC_(pIEnumCP()->Clone(&pIEnumCP2),S_OK);
	TESTC_(pIEnumCP()->Reset(),S_OK);
	TESTC_(pIEnumCP2->Next(1,&pIConnectionPoint,&cFetched),S_FALSE);
	TESTC(cFetched == 0 && pIConnectionPoint == NULL);

CLEANUP:
	SAFE_RELEASE(pIEnumCP2);
	SAFE_RELEASE(pIConnectionPoint);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc Container: Skip should skip one
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCConnectionStuff::Variation_10()
{
	TBEGIN
	IConnectionPoint* pIConnectionPoint= NULL;
	ULONG cFetched=505;//A random start value.

	TESTC_(pIEnumCP()->Reset(),S_OK);
	TESTC_(pIEnumCP()->Skip(1),S_OK);

	TESTC_(pIEnumCP()->Next(1, &pIConnectionPoint, &cFetched),S_FALSE);
	TESTC(cFetched == 0);
	
	TESTC_(pIEnumCP()->Reset(),S_OK);
	TESTC_(pIEnumCP()->Skip(ULONG_MAX),S_FALSE);
	TESTC_(pIEnumCP()->Next(1,&pIConnectionPoint,&cFetched),S_FALSE);
	TESTC(cFetched == 0);

CLEANUP:
	SAFE_RELEASE(pIConnectionPoint);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc Connections: EnumConnections
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCConnectionStuff::Variation_11()
{
	TBEGIN
	ULONG rgCookies[MAX_LISTENERS];
	CONNECTDATA rgConnData[MAX_LISTENERS];
	memset(rgConnData, 0, sizeof(CONNECTDATA)*MAX_LISTENERS);
	IEnumConnections* pIEnumConnections = NULL;
	ULONG cFetched = 101;
	
	//Advise all listeners	
	AdviseAll(rgCookies);

	//Obtain enumerator over listeners (connections)
	TESTC_(pICP()->EnumConnections(&pIEnumConnections),S_OK);

	//Just quickly sanity check
	TESTC_(pIEnumConnections->Next(MAX_LISTENERS, rgConnData, &cFetched),S_OK);
	TESTC(cFetched == MAX_LISTENERS);

	//Verify CONNECTDATA array
	TESTC(VerifyConnectData(MAX_LISTENERS, rgConnData, m_cListeners, (IUnknown**)m_rgpListeners, rgCookies));


CLEANUP:
	UnadviseAll(rgCookies);
	FreeConnectData(MAX_LISTENERS, rgConnData);
	SAFE_RELEASE(pIEnumConnections);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc Connections: Next should work until there are no more
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCConnectionStuff::Variation_12()
{
	TBEGIN
	ULONG rgCookies[MAX_LISTENERS];
	CONNECTDATA rgConnData[MAX_LISTENERS];
	memset(rgConnData, 0, sizeof(CONNECTDATA)*MAX_LISTENERS);
	IEnumConnections* pIEnumConnections = NULL;
	ULONG cFetched = 101;
	
	//Advise all listeners	
	AdviseAll(rgCookies);

	//Obtain enumerator over listeners (connections)
	TESTC_(pICP()->EnumConnections(&pIEnumConnections),S_OK);

	//Obtain connection 1 at a time
	TESTC_(pIEnumConnections->Reset(),S_OK);
	TESTC_(pIEnumConnections->Next(1, &rgConnData[0], &cFetched),S_OK);
	TESTC(cFetched == 1);
	TESTC(VerifyConnectData(1, &rgConnData[0], m_cListeners, (IUnknown**)m_rgpListeners, rgCookies));
	
	TESTC_(pIEnumConnections->Next(1, &rgConnData[1], &cFetched),S_OK);
	TESTC(cFetched == 1);
	TESTC(VerifyConnectData(1, &rgConnData[1], m_cListeners, (IUnknown**)m_rgpListeners, rgCookies));

	TESTC_(pIEnumConnections->Next(1, &rgConnData[2], &cFetched),S_OK);
	TESTC(cFetched == 1);
	TESTC(VerifyConnectData(1, &rgConnData[2], m_cListeners, (IUnknown**)m_rgpListeners, rgCookies));
	FreeConnectData(MAX_LISTENERS, rgConnData);

	//Try to onbtain more than in the list
	TESTC_(pIEnumConnections->Next(MAX_LISTENERS, rgConnData,&cFetched),S_FALSE);
	TESTC(cFetched == MAX_LISTENERS-3);

CLEANUP:
	UnadviseAll(rgCookies);
	FreeConnectData(MAX_LISTENERS, rgConnData);
	SAFE_RELEASE(pIEnumConnections);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc Connections: Clone
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCConnectionStuff::Variation_13()
{
	TBEGIN
	ULONG rgCookies[MAX_LISTENERS];
	CONNECTDATA rgConnData[MAX_LISTENERS];
	memset(rgConnData, 0, sizeof(CONNECTDATA)*MAX_LISTENERS);
	IEnumConnections* pIEnumConnections = NULL;
	IEnumConnections* pIEnumConnections2 = NULL;
	ULONG cFetched = 101;
	
	//Advise all listeners	
	AdviseAll(rgCookies);

	//Obtain enumerator over listeners (connections)
	TESTC_(pICP()->EnumConnections(&pIEnumConnections),S_OK);

	//Obtain connection all at a time
	TESTC_(pIEnumConnections->Reset(),S_OK);
	TESTC_(pIEnumConnections->Next(MAX_LISTENERS, &rgConnData[0], &cFetched),S_OK);
	TESTC(cFetched == MAX_LISTENERS);
	TESTC(VerifyConnectData(MAX_LISTENERS, rgConnData, m_cListeners, (IUnknown**)m_rgpListeners, rgCookies));
	FreeConnectData(MAX_LISTENERS, rgConnData);
	
	//Clone
	TESTC_(pIEnumConnections->Clone(&pIEnumConnections2),S_OK);
	TESTC_(pIEnumConnections2->Next(1, rgConnData, &cFetched),S_FALSE);
	TESTC(cFetched == 0);

	//Obtain connection all at a time
	TESTC_(pIEnumConnections2->Reset(),S_OK);
	TESTC_(pIEnumConnections2->Next(MAX_LISTENERS, &rgConnData[0], &cFetched),S_OK);
	TESTC(cFetched == MAX_LISTENERS);
	TESTC(VerifyConnectData(MAX_LISTENERS, rgConnData, m_cListeners, (IUnknown**)m_rgpListeners, rgCookies));
	FreeConnectData(MAX_LISTENERS, rgConnData);

CLEANUP:
	UnadviseAll(rgCookies);
	FreeConnectData(MAX_LISTENERS, rgConnData);
	SAFE_RELEASE(pIEnumConnections);
	SAFE_RELEASE(pIEnumConnections2);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc Conenctions: Reset should go to the beginning
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCConnectionStuff::Variation_14()
{
	TBEGIN
	ULONG rgCookies[MAX_LISTENERS];
	CONNECTDATA rgConnData[MAX_LISTENERS];
	memset(rgConnData, 0, sizeof(CONNECTDATA)*MAX_LISTENERS);
	IEnumConnections* pIEnumConnections = NULL;
	ULONG cFetched = 101;
	
	//Obtain enumerator over listeners (connections)
	TESTC_(pICP()->EnumConnections(&pIEnumConnections),S_OK);

	//Before Advising, should have none
	TESTC_(pIEnumConnections->Reset(),S_OK);
	TESTC_(pIEnumConnections->Next(MAX_LISTENERS, &rgConnData[0], &cFetched),S_FALSE);
	TESTC(cFetched == 0);
	SAFE_RELEASE(pIEnumConnections);

	//Advise all listeners	
	AdviseAll(rgCookies);

	//Obtain enumerator over listeners (connections)
	TESTC_(pICP()->EnumConnections(&pIEnumConnections),S_OK);

	//Obtain connection all at a time
	TESTC_(pIEnumConnections->Reset(),S_OK);
	TESTC_(pIEnumConnections->Next(MAX_LISTENERS, &rgConnData[0], &cFetched),S_OK);
	TESTC(cFetched == MAX_LISTENERS);
	TESTC(VerifyConnectData(MAX_LISTENERS, rgConnData, m_cListeners, (IUnknown**)m_rgpListeners, rgCookies));
	FreeConnectData(MAX_LISTENERS, rgConnData);
	
	//Reset should start work correctly
	TESTC_(pIEnumConnections->Reset(),S_OK);
	TESTC_(pIEnumConnections->Next(MAX_LISTENERS, &rgConnData[0], &cFetched),S_OK);
	TESTC(cFetched == MAX_LISTENERS);
	TESTC(VerifyConnectData(MAX_LISTENERS, rgConnData, m_cListeners, (IUnknown**)m_rgpListeners, rgCookies));

CLEANUP:
	UnadviseAll(rgCookies);
	FreeConnectData(MAX_LISTENERS, rgConnData);
	SAFE_RELEASE(pIEnumConnections);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc Connections: Skip should skip one
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCConnectionStuff::Variation_15()
{
	TBEGIN
	ULONG rgCookies[MAX_LISTENERS];
	CONNECTDATA rgConnData[MAX_LISTENERS];
	memset(rgConnData, 0, sizeof(CONNECTDATA)*MAX_LISTENERS);
	IEnumConnections* pIEnumConnections = NULL;
	IEnumConnections* pIEnumConnections2 = NULL;
	ULONG cFetched = 101;
	
	//Advise all listeners	
	AdviseAll(rgCookies);

	//Obtain enumerator over listeners (connections)
	TESTC_(pICP()->EnumConnections(&pIEnumConnections),S_OK);

	//Obtain connection all at a time
	TESTC_(pIEnumConnections->Reset(),S_OK);
	TESTC_(pIEnumConnections->Skip(1),S_OK);

	//Obtain the rest
	TESTC_(pIEnumConnections->Next(MAX_LISTENERS, &rgConnData[0], &cFetched),S_FALSE);
	TESTC(cFetched == MAX_LISTENERS-1);
	TESTC(VerifyConnectData(cFetched, rgConnData, m_cListeners, (IUnknown**)m_rgpListeners, rgCookies));
	FreeConnectData(cFetched, rgConnData);
	
	//Skip ULONG_MAX
	TESTC_(pIEnumConnections->Reset(),S_OK);
	TESTC_(pIEnumConnections->Skip(ULONG_MAX),S_FALSE);
	TESTC_(pIEnumConnections->Next(1, rgConnData, &cFetched),S_FALSE);
	TESTC(cFetched == 0);

	//Skip none
	TESTC_(pIEnumConnections->Reset(),S_OK);
	TESTC_(pIEnumConnections->Skip(0),E_INVALIDARG);

CLEANUP:
	UnadviseAll(rgCookies);
	FreeConnectData(MAX_LISTENERS, rgConnData);
	SAFE_RELEASE(pIEnumConnections);
	SAFE_RELEASE(pIEnumConnections2);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc Point: GetConnectionInterface should return succesful
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCConnectionStuff::Variation_16()
{
	TBEGIN
	IConnectionPoint* pIConnectionPoint= NULL;
	IID iid;

	TESTC_(pICPC()->FindConnectionPoint(IID_IRowPositionChange, &pIConnectionPoint),S_OK);
	TESTC_(pIConnectionPoint->GetConnectionInterface(&iid),S_OK);
	TESTC(iid == IID_IRowPositionChange);

CLEANUP:
	SAFE_RELEASE(pIConnectionPoint);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc Point: GetConnectionPointContainer should work with a valid pointer
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCConnectionStuff::Variation_17()
{
	TBEGIN
	IRowPosition* pIRowPosition = NULL;
	IConnectionPoint* pIConnectionPoint = NULL;
	IConnectionPointContainer* pICPC = NULL;
	IConnectionPointContainer* pICPC2 = NULL;

	//Create RowPosition Object
	TESTC_(CreateRowPosition(pIRowset(), &pIRowPosition),S_OK);

	//Obtain the ConnectionPoint...
	TESTC_(QI(pIRowPosition, IID_IConnectionPointContainer, (void**)&pICPC),S_OK);
	TESTC_(pICPC->FindConnectionPoint(IID_IRowPositionChange, &pIConnectionPoint),S_OK);

	//Obtain the container from the ConnectionPoint
	TESTC_(pIConnectionPoint->GetConnectionPointContainer(&pICPC2),S_OK);
	TESTC(pICPC == pICPC2);

CLEANUP:
	SAFE_RELEASE(pIConnectionPoint);
	SAFE_RELEASE(pICPC);
	SAFE_RELEASE(pICPC2);
	SAFE_RELEASE(pIRowPosition);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc B/NULL: Container: QI for IID_NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCConnectionStuff::Variation_18()
{
	IUnknown* pIUnknown= NULL;

	TESTC_(QI(pICPC(), IID_NULL, (void**)&pIUnknown),E_NOINTERFACE);
	TESTC(pIUnknown == NULL);

CLEANUP:
	SAFE_RELEASE(pIUnknown);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc Container: FindConnectionPoint for IID_NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCConnectionStuff::Variation_19()
{
	TBEGIN
	IConnectionPoint* pIConnectionPoint= NULL;

	TESTC_(pICPC()->FindConnectionPoint(IID_NULL, &pIConnectionPoint),CONNECT_E_NOCONNECTION);
	TESTC(pIConnectionPoint == NULL);

CLEANUP:
	SAFE_RELEASE(pIConnectionPoint);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc Container: FindConnectionPoint for ppCP is NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCConnectionStuff::Variation_20()
{
	TBEGIN
	TESTC_(pICPC()->FindConnectionPoint(IID_IRowPositionChange, NULL),E_POINTER);

CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc Enum: Next with a rgpcd NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCConnectionStuff::Variation_21()
{
	TBEGIN
	ULONG cFetched=505;//A random start value.

	TESTC_(pIEnumCP()->Next(1, NULL, &cFetched),E_POINTER);
	TESTC(cFetched == 0);

CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(22)
//*-----------------------------------------------------------------------
// @mfunc Enum: Skip on the second one should still be at the end
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCConnectionStuff::Variation_22()
{
	TBEGIN
	IConnectionPoint* pIConnectionPoint= NULL;
	ULONG cFetched=505;//A random start value.

	TESTC_(pIEnumCP()->Reset(),S_OK);
	TESTC_(pIEnumCP()->Skip(1),S_OK);
	TESTC_(pIEnumCP()->Skip(1),S_FALSE);
	TESTC_(pIEnumCP()->Next(1, &pIConnectionPoint, &cFetched),S_FALSE);
	TESTC(cFetched == 0 && pIConnectionPoint == NULL);

CLEANUP:
	SAFE_RELEASE(pIConnectionPoint);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(23)
//*-----------------------------------------------------------------------
// @mfunc Container: Enum should fail for a NULL pointer
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCConnectionStuff::Variation_23()
{
	TBEGIN
	TESTC_(pICPC()->EnumConnectionPoints(NULL),E_POINTER);

CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(24)
//*-----------------------------------------------------------------------
// @mfunc Enum: reset twice in a row
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCConnectionStuff::Variation_24()
{
	TBEGIN
	IConnectionPoint* pIConnectionPoint= NULL;
	IConnectionPoint* pIConnectionPoint2= NULL;
	ULONG cFetched=505;//A random start value.

	TESTC_(pIEnumCP()->Reset(),S_OK);
	TESTC_(pIEnumCP()->Reset(),S_OK); //DOUBLE RESET
	TESTC_(pIEnumCP()->Next(1, &pIConnectionPoint, &cFetched),S_OK);
	TESTC(cFetched == 1 && pIConnectionPoint != NULL);
	
	TESTC_(pICPC()->FindConnectionPoint(IID_IRowPositionChange, &pIConnectionPoint2),S_OK);
	TESTC(pIConnectionPoint == pIConnectionPoint2);

CLEANUP:
	SAFE_RELEASE(pIConnectionPoint);
	SAFE_RELEASE(pIConnectionPoint2);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(25)
//*-----------------------------------------------------------------------
// @mfunc Enum: clone where ppIEnum is NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCConnectionStuff::Variation_25()
{
	TBEGIN
	TESTC_(pIEnumCP()->Clone(NULL),E_POINTER);

CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(26)
//*-----------------------------------------------------------------------
// @mfunc Point: GetConectionInterface with pIID NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCConnectionStuff::Variation_26()
{
	TBEGIN
	TESTC_(pICP()->GetConnectionInterface(NULL),E_POINTER);

CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(27)
//*-----------------------------------------------------------------------
// @mfunc Point: GetConectionPointContainer with ppCPC null
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCConnectionStuff::Variation_27()
{
	TBEGIN
	TESTC_(pICP()->GetConnectionPointContainer(NULL),E_POINTER);

CLEANUP:
	TRETURN

}
// }}


// {{ TCW_VAR_PROTOTYPE(28)
//*-----------------------------------------------------------------------
// @mfunc Point: Unadvise before advising should fail
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCConnectionStuff::Variation_28()
{
	TBEGIN

	TESTC_(pICP()->Unadvise(PtrToUlong(m_rgpListeners[0])),CONNECT_E_NOCONNECTION);
	TESTC_(pICP()->Unadvise(1),CONNECT_E_NOCONNECTION);
	TESTC_(pICP()->Unadvise(ULONG_MAX),CONNECT_E_NOCONNECTION);
	TESTC_(pICP()->Unadvise(0),CONNECT_E_NOCONNECTION);

CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(29)
//*-----------------------------------------------------------------------
// @mfunc Point: advise with pointers NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCConnectionStuff::Variation_29()
{
	TBEGIN
	ULONG dwCookie1 = 101;
	ULONG dwCookie2 = 101;

	TESTC_(pICP()->Advise(pIRowset(), &dwCookie1),CONNECT_E_CANNOTCONNECT);
	TESTC_(pICP()->Advise(NULL, &dwCookie2),E_POINTER);

	TESTC(dwCookie1 == 0);
	TESTC(dwCookie2 == 0);

CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(30)
//*-----------------------------------------------------------------------
// @mfunc point: Unadvise with pointers NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCConnectionStuff::Variation_30()
{
	TBEGIN
	ULONG rgCookies[MAX_LISTENERS];
		
	//Advise all listeners	
	AdviseAll(rgCookies);

	TESTC_(pICP()->Unadvise(0),CONNECT_E_NOCONNECTION);
	TESTC_(pICP()->Unadvise(PtrToUlong(m_rgpListeners[0])+1),CONNECT_E_NOCONNECTION);
	TESTC_(pICP()->Unadvise(ULONG_MAX),CONNECT_E_NOCONNECTION);
	

CLEANUP:
	UnadviseAll(rgCookies);
	TRETURN
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCConnectionStuff::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCRowPosition::Terminate());
}	// }}
// }}
// }}

// {{ TCW_TC_PROTOTYPE(TCTransactions)
//*-----------------------------------------------------------------------
//| Test Case:		TCTransactions - Testing RowPos within transactions
//| Created:  	7/6/98
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCTransactions::Init()
{ 
	if(CTransaction::Init())
	{
	   	//register interface to be tested                                         
		if(RegisterInterface(ROWSET_INTERFACE, IID_IRowset, 0, NULL)) 
			return TRUE;
	}

	//Not all providers have to support transactions
	//If a required interface, an error would have been posted by VerifyInterface
	TEST_PROVIDER(m_pITransactionLocal != NULL);
	return FALSE;
} 






// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc COMMIT - fRetaning == TRUE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCTransactions::Variation_1()
{ 
	DBCOUNTITEM	cRowsObtained = 0;
	HROW*	rghRows = NULL;
	IRowPosition* pIRowPosition = NULL;
	IRowset* pIRowset = NULL;

	HCHAPTER hChapter = NULL;
	HROW	 hRow = NULL;
	ULONG	 ulPosition = NULL;
	HRESULT  hrExpected = S_OK;

	//start a transaction
	TESTC(StartTransaction(SELECT_ALLFROMTBL, NULL, 0, NULL));

	//get a row handle
	TESTC_(m_pIRowset->GetNextRows(NULL, 0, 1, &cRowsObtained, &rghRows),S_OK);

	//Create RowPosition object
	TESTC_(TCRowPosition::CreateRowPosition(m_pIRowset, &pIRowPosition),S_OK);
	CHECK(TCRowPosition::ClearRowPosition(pIRowPosition), S_OK);
	CHECK(TCRowPosition::SetRowPosition(pIRowPosition, DB_NULL_HCHAPTER, rghRows[0], DBPOSITION_OK), S_OK);

	//commit the transaction with fRetaining==TRUE
	TESTC(GetCommit(TRUE))
	if(!m_fCommitPreserve)
		hrExpected = E_UNEXPECTED;
	
	//After Commit
	CHECK(TCRowPosition::GetRowPosition(pIRowPosition, &hChapter, &hRow, &ulPosition), hrExpected);
	CHECK(TCRowPosition::ClearRowPosition(pIRowPosition), S_OK);
	CHECK(TCRowPosition::SetRowPosition(pIRowPosition, DB_NULL_HCHAPTER, rghRows[0], DBPOSITION_OK), hrExpected);

	//Allowed to Call IUnknown methods in Zombie (QueryInterface)
	CHECK(TCRowPosition::GetRowset(pIRowPosition, IID_IRowset, (IUnknown**)&pIRowset), S_OK);
	CHECK(TCRowPosition::CreateRowPosition(m_pIRowset), S_OK);

	// 1. When IRowset::GetNextRows is called row ref count = 1
	// 2. When TCRowPosition::SetRowPosition is called, if row position is successfully set, it will
	//    do an IRowset::ReleaseRows() on the HROW so after the call the ref count for row will still
	//    be 1.
	// 3. TCRowPosition::GetRowPosition returns failure because rowset is in zombie state, so
	//    ref count for row still remains 1.
	// 4. TCRowPosition::ClearRowPosition clears the row handle and calls IRowset::ReleaseRows on the
	//    row handle decrementing the row handle count to zero and thus releasing the row.

	// So, when IRowset::ReleaseRows() is called here, the row handle is already in invalid 
	// state so expect a failure DB_E_ERRORSOCCURRED instead.
	TESTC_(m_pIRowset->ReleaseRows(cRowsObtained, rghRows, NULL, NULL, NULL), DB_E_ERRORSOCCURRED);


CLEANUP:
	PROVIDER_FREE(rghRows);
	SAFE_RELEASE(pIRowPosition);
	SAFE_RELEASE(pIRowset);
	 
	//clean up.  Expected S_OK.
	CleanUpTransaction(S_OK);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc COMMIT - fRetaning == FALSE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCTransactions::Variation_2()
{ 
	DBCOUNTITEM	cRowsObtained = 0;
	HROW*	rghRows = NULL;
	IRowPosition* pIRowPosition = NULL;
	IRowset* pIRowset = NULL;

	HCHAPTER hChapter = NULL;
	HROW	 hRow = NULL;
	ULONG	 ulPosition = NULL;
	HRESULT  hrExpected = S_OK;

	//start a transaction
	TESTC(StartTransaction(SELECT_ALLFROMTBL, NULL, 0, NULL));

	//get a row handle
	TESTC_(m_pIRowset->GetNextRows(NULL, 0, 1, &cRowsObtained, &rghRows),S_OK);

	//Create RowPosition object
	TESTC_(TCRowPosition::CreateRowPosition(m_pIRowset, &pIRowPosition),S_OK);
	CHECK(TCRowPosition::ClearRowPosition(pIRowPosition), S_OK);
	CHECK(TCRowPosition::SetRowPosition(pIRowPosition, DB_NULL_HCHAPTER, rghRows[0], DBPOSITION_OK), S_OK);

	//commit the transaction with fRetaining==FALSE
	TESTC(GetCommit(FALSE))
	if(!m_fCommitPreserve)
		hrExpected = E_UNEXPECTED;
	
	//After Commit
	CHECK(TCRowPosition::GetRowPosition(pIRowPosition, &hChapter, &hRow, &ulPosition), hrExpected);
	CHECK(TCRowPosition::ClearRowPosition(pIRowPosition), S_OK);
	CHECK(TCRowPosition::SetRowPosition(pIRowPosition, DB_NULL_HCHAPTER, rghRows[0], DBPOSITION_OK), hrExpected);

	//Allowed to Call IUnknown methods in Zombie (QueryInterface)
	CHECK(TCRowPosition::GetRowset(pIRowPosition, IID_IRowset, (IUnknown**)&pIRowset), S_OK);
	CHECK(TCRowPosition::CreateRowPosition(m_pIRowset), S_OK);

	TESTC_(m_pIRowset->ReleaseRows(cRowsObtained, rghRows, NULL, NULL, NULL), DB_E_ERRORSOCCURRED);

CLEANUP:
	PROVIDER_FREE(rghRows);
	SAFE_RELEASE(pIRowPosition);
	SAFE_RELEASE(pIRowset);
	 
	//clean up.  Expected S_OK.
	CleanUpTransaction(XACT_E_NOTRANSACTION);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc ABORT - fRetaning == TRUE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCTransactions::Variation_3()
{ 
	DBCOUNTITEM	cRowsObtained = 0;
	HROW*	rghRows = NULL;
	IRowPosition* pIRowPosition = NULL;
	IRowset* pIRowset = NULL;

	HCHAPTER hChapter = NULL;
	HROW	 hRow = NULL;
	ULONG	 ulPosition = NULL;
	HRESULT  hrExpected = S_OK;

	//start a transaction
	TESTC(StartTransaction(SELECT_ALLFROMTBL, NULL, 0, NULL));

	//get a row handle
	TESTC_(m_pIRowset->GetNextRows(NULL, 0, 1, &cRowsObtained, &rghRows),S_OK);

	//Create RowPosition object
	TESTC_(TCRowPosition::CreateRowPosition(m_pIRowset, &pIRowPosition),S_OK);
	CHECK(TCRowPosition::ClearRowPosition(pIRowPosition), S_OK);
	CHECK(TCRowPosition::SetRowPosition(pIRowPosition, DB_NULL_HCHAPTER, rghRows[0], DBPOSITION_OK), S_OK);

	//Abort the transaction with fRetaining==TRUE
	TESTC(GetAbort(TRUE))
	if(!m_fAbortPreserve)
		hrExpected = E_UNEXPECTED;
	
	//After Abort
	CHECK(TCRowPosition::GetRowPosition(pIRowPosition, &hChapter, &hRow, &ulPosition), hrExpected);
	CHECK(TCRowPosition::ClearRowPosition(pIRowPosition), S_OK);
	CHECK(TCRowPosition::SetRowPosition(pIRowPosition, DB_NULL_HCHAPTER, rghRows[0], DBPOSITION_OK), hrExpected);

	//Allowed to Call IUnknown methods in Zombie (QueryInterface)
	CHECK(TCRowPosition::GetRowset(pIRowPosition, IID_IRowset, (IUnknown**)&pIRowset), S_OK);
	CHECK(TCRowPosition::CreateRowPosition(m_pIRowset), S_OK);

	TESTC_(m_pIRowset->ReleaseRows(cRowsObtained, rghRows, NULL, NULL, NULL), DB_E_ERRORSOCCURRED);

CLEANUP:
	PROVIDER_FREE(rghRows);
	SAFE_RELEASE(pIRowPosition);
	SAFE_RELEASE(pIRowset);
	 
	//clean up.  Expected S_OK.
	CleanUpTransaction(S_OK);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc ABORT - fRetaning == FALSE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCTransactions::Variation_4()
{ 
	DBCOUNTITEM	cRowsObtained = 0;
	HROW*	rghRows = NULL;
	IRowPosition* pIRowPosition = NULL;
	IRowset* pIRowset = NULL;

	HCHAPTER hChapter = NULL;
	HROW	 hRow = NULL;
	ULONG	 ulPosition = NULL;
	HRESULT  hrExpected = S_OK;

	//start a transaction
	TESTC(StartTransaction(SELECT_ALLFROMTBL, NULL, 0, NULL));

	//get a row handle
	TESTC_(m_pIRowset->GetNextRows(NULL, 0, 1, &cRowsObtained, &rghRows),S_OK);

	//Create RowPosition object
	TESTC_(TCRowPosition::CreateRowPosition(m_pIRowset, &pIRowPosition),S_OK);
	CHECK(TCRowPosition::ClearRowPosition(pIRowPosition), S_OK);
	CHECK(TCRowPosition::SetRowPosition(pIRowPosition, DB_NULL_HCHAPTER, rghRows[0], DBPOSITION_OK), S_OK);

	//Abort the transaction with fRetaining==FALSE
	TESTC(GetAbort(FALSE))
	if(!m_fCommitPreserve)
		hrExpected = E_UNEXPECTED;
	
	//After Abort
	CHECK(TCRowPosition::GetRowPosition(pIRowPosition, &hChapter, &hRow, &ulPosition), hrExpected);
	CHECK(TCRowPosition::ClearRowPosition(pIRowPosition), S_OK);
	CHECK(TCRowPosition::SetRowPosition(pIRowPosition, DB_NULL_HCHAPTER, rghRows[0], DBPOSITION_OK), hrExpected);

	//Allowed to Call IUnknown methods in Zombie (QueryInterface)
	CHECK(TCRowPosition::GetRowset(pIRowPosition, IID_IRowset, (IUnknown**)&pIRowset), S_OK);
	CHECK(TCRowPosition::CreateRowPosition(m_pIRowset), S_OK);

	TESTC_(m_pIRowset->ReleaseRows(cRowsObtained, rghRows, NULL, NULL, NULL), DB_E_ERRORSOCCURRED);


CLEANUP:
	PROVIDER_FREE(rghRows);
	SAFE_RELEASE(pIRowPosition);
	SAFE_RELEASE(pIRowset);
	 
	//clean up.  Expected S_OK.
	CleanUpTransaction(XACT_E_NOTRANSACTION);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END






// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCTransactions::Terminate()
{ 
	// TO DO:  Add your own code here 

// {{ TCW_TERM_BASECLASS_CHECK2
	return(CTransaction::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END

