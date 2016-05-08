//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright (C) 1995-2000 Microsoft Corporation
//
// @doc  
//
// @module IDBAsynch.cpp | This module tests the OLEDB IDBAsynchStatus interface 
//


//////////////////////////////////////////////////////////////////////////
// Includes
//
//////////////////////////////////////////////////////////////////////////
#define  DBINITCONSTANTS	// Must be defined to initialize constants in OLEDB.H
#define  INITGUID
#include "modstandard.hpp"
#include "IDBAsynch.h"
#include "extralib.h"
#include "olectl.h"						//CONNECT_E_NOCONNECT



// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Module Values
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// {{ TCW_MODULE_GLOBALS
//DECLARE_MODULE_CLSID = { 0x17dff980, 0x64a3, 0x11ce, { 0xb1, 0x24, 0x00, 0xaa, 0x00, 0x57, 0x59, 0x9e }};
DECLARE_MODULE_CLSID = { 0xc661f260, 0x799f, 0x4c2c, {0xbb, 0x58, 0x29, 0xa7, 0xba, 0xc7, 0x74, 0x1f}};
DECLARE_MODULE_NAME("IDBAsynch");
DECLARE_MODULE_OWNER("Microsoft");
DECLARE_MODULE_DESCRIP("IDBAsynchStatus Interface test");
DECLARE_MODULE_VERSION(795921705);
// TCW_WizardVersion(2)
// TCW_Automation(False)
// }} TCW_MODULE_GLOBALS_END


//////////////////////////////////////////////////////////////////////////
// Defines
//
//////////////////////////////////////////////////////////////////////////
#define INVALID_PUNK			INVALID(IUnknown*)
#define INVALID_COOKIE			INVALID(DWORD)
#define PROPVAL_NEGATIVE	  (-0x1)

#define TEST_SUPPORTED_PROP(p, pset)	{ TESTB = TEST_PASS; if(!SupportedProperty(p, pset)) { TOUTPUT_LINE(L#p <<" is not supported"); TESTB = TEST_SKIPPED; TRETURN; } }

#define TEST_SETTABLE_PROP(p, pset) 	{ TESTB = TEST_PASS; if(!SettableProperty(p, pset)) { TOUTPUT_LINE(L#p<< " is not settable"); TESTB = TEST_SKIPPED; TRETURN; } }
			

enum ERESULT
{
	OP_ABORTED		= 0x00000001,
	OP_COMPLETED	= 0x00000002,
};


////////////////////////////////////////////////////////////////////////////
// Forwards
//
////////////////////////////////////////////////////////////////////////////
class CSource;
class CListener;


//--------------------------------------------------------------------
// @func Module level initialization routine
//
// @rdesc Success or Failure
// 		@flag  TRUE  | Successful initialization
//		@flag  FALSE | Initialization problems
//
BOOL ModuleInit(CThisTestModule * pThisTestModule)
{
	//NOTE:  This test does numerous different "sources" for connection points, which
	//are determined in the Initialization.  So we really can't skip the entire test up
	//front since their may be some source of notifications supported.  
	//(Rowset, DataSource, Row, etc)
    BOOL bResult = CommonModuleInit(pThisTestModule, IID_IUnknown);
	if(bResult == TRUE)
	{
		//Make sure that at least one of the connection point sources are available...
		TEST_PROVIDER
			(
  				SupportedInterface(IID_IDBAsynchNotify, ROWSET_INTERFACE) ||
				SupportedInterface(IID_IDBAsynchNotify, ROW_INTERFACE) ||
				SupportedInterface(IID_IDBAsynchNotify, DATASOURCE_INTERFACE)
			);
	}

	return bResult;
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

	
//////////////////////////////////////////////////////////////////////////
// class CConnectInfo
//
// Just a simple class to track connection info
//////////////////////////////////////////////////////////////////////////
class CConnectInfo
{
public:
	//Constructors
	CConnectInfo(DWORD dwCookie, CListener* pCListener, CSource* pCSource)
	{
		ASSERT(dwCookie);
		ASSERT(pCSource);
		ASSERT(pCListener);

		m_dwCookie		= dwCookie;
		m_pCSource		= pCSource;
		m_pCListener	= pCListener;
	}

    virtual ~CConnectInfo()
	{
	}
	
//protected:
	//data
	DWORD			m_dwCookie;
	CSource*		m_pCSource;
	CListener*		m_pCListener;
};



//////////////////////////////////////////////////////////////////////////
// class CSource (abstract class)
//
// Basically an abstract object that is the "source" of the notifications / connectionpoint
//////////////////////////////////////////////////////////////////////////
class CSource
{
public:
	//Constructors
	CSource(REFIID riid, EINTERFACE eObject);
    virtual ~CSource();

    //pure virtual members
	virtual BOOL CreateSource() = 0;
	virtual BOOL CauseNotification() = 0;

	//Helpers
	virtual BOOL ReleaseCP();
	virtual BOOL CreateCP(IUnknown* pIUnknown);
	virtual BOOL VerifyNotification(CListener* pCListener, ULONG ulTimesConnected);

	//Interface
	virtual IConnectionPointContainer*	const 	pICPC()			{ ASSERT(m_pIConnectionPointContainer); return m_pIConnectionPointContainer;	} 
	virtual IEnumConnectionPoints*		const	pIEnumCP()		{ ASSERT(m_pIEnumConnectionPoints);		return m_pIEnumConnectionPoints;		} 
	
	virtual ULONG						GetCountCP()			{ return m_cConnectionPoints;	}
	virtual IConnectionPoint*			GetCP(ULONG iIndex = 0)	{ ASSERT(iIndex < m_cConnectionPoints); return m_rgConnectionPoints[iIndex];	}
	virtual REFIID						GetIID()				{ return m_iid;					}
	virtual EINTERFACE					GetObjectType()			{ return m_eObject;				}
	
protected:
	//Container
	IConnectionPointContainer*			m_pIConnectionPointContainer;
	IEnumConnectionPoints*				m_pIEnumConnectionPoints;

	//Connection Points
	ULONG								m_cConnectionPoints;
	IConnectionPoint**					m_rgConnectionPoints;

	//Data
	IID									m_iid;
	EINTERFACE							m_eObject;
};


CSource::CSource(REFIID riid, EINTERFACE eObject)
{
	//Container
	m_pIConnectionPointContainer	= NULL;
	m_pIEnumConnectionPoints		= NULL;

	//Connection Points
	m_cConnectionPoints				= 0;
	m_rgConnectionPoints			= NULL;

	//Type of Objects
	m_iid							= riid;
	m_eObject						= eObject;
}

CSource::~CSource()
{
	ReleaseCP();
}

BOOL CSource::ReleaseCP()
{
	ULONG i = 0;
	//Container
	SAFE_RELEASE(m_pIConnectionPointContainer);
	SAFE_RELEASE(m_pIEnumConnectionPoints);

	//Connection Points
	for(i=0; i<m_cConnectionPoints; i++)
		SAFE_RELEASE(m_rgConnectionPoints[i]);
	SAFE_FREE(m_rgConnectionPoints);
	m_cConnectionPoints	= 0;
	return TRUE;
}
	
BOOL CSource::CreateCP(IUnknown* pIUnknown)
{
	TBEGIN
	ULONG cFetched = 0;
	IConnectionPoint* pICP = NULL;
	BOOL fSupportedCP = FALSE;
	IID iid;

	//Release any previous interfaces...
	ReleaseCP();

	//Obtain the connection point container
	QTESTC_((QI(pIUnknown, IID_IConnectionPointContainer, (void**)&m_pIConnectionPointContainer)),S_OK);

	//Make sure this connection supports the connection point were interested in...
	QTESTC_(m_pIConnectionPointContainer->FindConnectionPoint(m_iid, &pICP),S_OK);
	SAFE_RELEASE(pICP);

	//Obtain the IEnumConnectionPoints 
	TESTC_(m_pIConnectionPointContainer->EnumConnectionPoints(&m_pIEnumConnectionPoints),S_OK);

	//Obtain all the Connection Points
	m_cConnectionPoints = 0;
	SAFE_ALLOC(m_rgConnectionPoints, IConnectionPoint*, m_cConnectionPoints + 1);

	//Obtain all connection points...
	while(SUCCEEDED(pIEnumCP()->Next(1, &pICP, &cFetched)) && cFetched == 1)
	{
		m_rgConnectionPoints[m_cConnectionPoints] = pICP;
		SAFE_REALLOC(m_rgConnectionPoints, IConnectionPoint*, m_cConnectionPoints + 1 + 1);

		//To make our life easier, lets place the desired ConnectionPoint, first in the list
		//so we can easily access it and distingush it from the other connection points.  We just
		//need to swap it with the first element of the array...
		TESTC_(pICP->GetConnectionInterface(&iid),S_OK);
		if(iid == m_iid && !fSupportedCP)
		{
			m_rgConnectionPoints[m_cConnectionPoints] = m_rgConnectionPoints[0];
			m_rgConnectionPoints[0] = pICP;
			fSupportedCP = TRUE;
		}

		//Increment total connection points...
		m_cConnectionPoints++;
	}

	//Make sure that we found the connection point in the enumeration
	TESTC(m_cConnectionPoints >= 1);
	TESTC_(m_rgConnectionPoints[0]->GetConnectionInterface(&iid),S_OK);
	TESTC(iid == m_iid);

CLEANUP:
	return fSupportedCP;
}

BOOL CSource::VerifyNotification(CListener* pCListener, ULONG ulTimesConnected)
{
	ASSERT(pCListener);
	TBEGIN

	TESTC(pCListener->GetTimesNotified() >= ulTimesConnected);

CLEANUP:
	TRETURN
}



//////////////////////////////////////////////////////////////////////////
// class CAsynch
//
//////////////////////////////////////////////////////////////////////////
class CAsynch : public CSource
{
public:
	CAsynch(REFIID riid, EINTERFACE eObject);
	virtual ~CAsynch();
	
	//Pure Virtual
	virtual BOOL	CreateCP(IUnknown* pIUnknown);
	virtual	BOOL	ResetSource(DWORD dwPropVal = DBPROPVAL_ASYNCH_INITIALIZE);
	virtual BOOL	VerifyNotification(CListener* pCListener, DWORD dwResult, ULONG ulTimesConnected);
	virtual	BOOL	ReleaseSource();

	//Helpers (Wrappers)
	virtual HRESULT Abort(ULONG ulOperation = DBASYNCHOP_OPEN);
	virtual HRESULT GetStatus(DBCOUNTITEM *pulProgress = NULL, DBCOUNTITEM* pulProgressMax = NULL, ULONG* pulAsynchPhase = NULL, LPOLESTR* ppwszStatusText = NULL);
	virtual HRESULT GetStatus(ULONG ulOperation, DBCOUNTITEM *pulProgress = NULL, DBCOUNTITEM* pulProgressMax = NULL, ULONG* pulAsynchPhase = NULL, LPOLESTR* ppwszStatusText = NULL);

	//Verification
	virtual	BOOL	VerifyAbort();
	virtual	BOOL	VerifyZombie();

	//Helpers
	virtual	HRESULT	WaitUntilComplete();
	virtual BOOL	IsComplete();
	virtual BOOL	IsAsynch()				{ return (m_ulAsynch != 0);		}

	//Helpers
	virtual DBCOUNTITEM	GetProgress()			{ return m_ulProgress;		}
	virtual DBCOUNTITEM	GetProgressMax()		{ return m_ulProgressMax;	}
	virtual ULONG	GetAsynchPhase()		{ return m_ulAsynchPhase;	}
	virtual WCHAR*	GetStatusText()			{ return m_pwszStatusText;	}
	
	virtual DBCOUNTITEM	GetPrevProgress()		{ return m_ulPrevProgress;		}
	virtual DBCOUNTITEM	GetPrevProgressMax()	{ return m_ulPrevProgressMax;	}
	virtual ULONG	GetPrevAsynchPhase()	{ return m_ulPrevAsynchPhase;	}
	virtual WCHAR*	GetPrevStatusText()		{ return m_pwszPrevStatusText;	}

	//Interface
	virtual IDBAsynchStatus*			const	pIDBAsynchStatus()	{ ASSERT(m_pIDBAsynchStatus); return m_pIDBAsynchStatus;	}
	
protected:
	DBCOUNTITEM					m_ulProgress;
	DBCOUNTITEM					m_ulProgressMax;
	ULONG						m_ulAsynchPhase;
	WCHAR*						m_pwszStatusText;
	
	//Previous Items
	DBCOUNTITEM					m_ulPrevProgress;
	DBCOUNTITEM					m_ulPrevProgressMax;
	ULONG						m_ulPrevAsynchPhase;
	WCHAR*						m_pwszPrevStatusText;

	//data
	IDBAsynchStatus*			m_pIDBAsynchStatus;

	//data
	ULONG_PTR					m_ulAsynch;
	DWORD						m_dwAsynchPropVal;
	DWORD						m_dwAsynchSupport;
};


//////////////////////////////////////////////////////////////////////////
// class CAsynch
//
//////////////////////////////////////////////////////////////////////////
CAsynch::CAsynch(REFIID riid, EINTERFACE eObject)
	: CSource(riid, eObject)
{
	m_ulProgress		= 0;
	m_ulProgressMax		= 0;
	m_ulAsynchPhase		= 0;
	m_pwszStatusText	= NULL;

	m_ulPrevProgress		= 0;
	m_ulPrevProgressMax		= 0;
	m_ulPrevAsynchPhase		= 0;
	m_pwszPrevStatusText	= NULL;

	m_pIDBAsynchStatus		= NULL;
	m_ulAsynch				= 0;
	m_dwAsynchPropVal = 0;
	m_dwAsynchSupport = 0;
}

CAsynch::~CAsynch()
{
	ReleaseSource();
}


BOOL CAsynch::ReleaseSource()
{
	CAsynch::ResetSource();

	//IDBAsynchStatus
	SAFE_RELEASE(m_pIDBAsynchStatus);

	//Container
	ReleaseCP();
	return TRUE;
}


BOOL CAsynch::ResetSource(DWORD dwPropVal)
{
	m_ulProgress		= 0;
	m_ulProgressMax		= 0;
	m_ulAsynchPhase		= 0;
	SAFE_FREE(m_pwszStatusText);

	m_ulPrevProgress		= 0;
	m_ulPrevProgressMax		= 0;
	m_ulPrevAsynchPhase		= 0;
	SAFE_FREE(m_pwszPrevStatusText);
	
	m_ulAsynch				= 0;
	m_dwAsynchPropVal = dwPropVal;

	return TRUE;
}


BOOL CAsynch::CreateCP(IUnknown* pIUnknown)
{
	TBEGIN
	ASSERT(pIUnknown);

	//Release the Previous interfaces...
	ReleaseSource();

	//Setup Connection Point Stuff
	QTESTC(CSource::CreateCP(pIUnknown));
	
	//Try to obtain the IDBAsynchStatus interface
	QTESTC_(QI(pIUnknown, IID_IDBAsynchStatus, (void**)&m_pIDBAsynchStatus), S_OK);

CLEANUP:
	TRETURN;
}


HRESULT CAsynch::Abort(ULONG ulOperation)
{
	//IDBAsynchStatus::Abort
	return pIDBAsynchStatus()->Abort(DB_NULL_HCHAPTER, ulOperation);
}

HRESULT CAsynch::GetStatus(DBCOUNTITEM *pulProgress, DBCOUNTITEM* pulProgressMax, ULONG* pulAsynchPhase, LPOLESTR* ppwszStatusText)
{
	//Delegate
	return GetStatus(DBASYNCHOP_OPEN, pulProgress, pulProgressMax, pulAsynchPhase, ppwszStatusText);
}

HRESULT CAsynch::GetStatus(ULONG ulOperation, DBCOUNTITEM *pulProgress, DBCOUNTITEM* pulProgressMax, ULONG* pulAsynchPhase, LPOLESTR* ppwszStatusText)
{
	TBEGIN
	HRESULT hr = S_OK;
	m_ulPrevProgress	= m_ulProgress;
	m_ulPrevProgressMax = m_ulProgressMax;
	m_ulPrevAsynchPhase	= m_ulAsynchPhase;

	SAFE_FREE(m_pwszPrevStatusText);
	m_pwszPrevStatusText= m_pwszStatusText;
	m_pwszStatusText	= NULL;
	
	//IDBAsynchStatus::GetStatus
	hr = pIDBAsynchStatus()->GetStatus(DB_NULL_HCHAPTER, ulOperation, &m_ulProgress, &m_ulProgressMax, &m_ulAsynchPhase, &m_pwszStatusText);
	TRACE(L"IDBAsynchStatus::GetStatus(DB_NULL_HCHAPTER, %s, %d, %d, %s, \"%s\"\n", ulOperation==DBASYNCHOP_OPEN ? L"DBASYNCHOP_OPEN" : L"(Unknown)", m_ulProgress, m_ulProgressMax, ::GetAsynchPhase(m_ulAsynchPhase), m_pwszStatusText);
	TEST3C_(hr, S_OK, DB_E_CANCELED, E_UNEXPECTED);

	//Verify Results
	TESTC(m_ulAsynchPhase >= m_ulPrevAsynchPhase);

	//Verify HRESULT matches Phase
	if(hr == DB_E_CANCELED)
	{
		TESTC(m_ulProgress < m_ulProgressMax);
		TESTC(m_ulAsynchPhase == DBASYNCHPHASE_CANCELED);
	}
	else
	{
		TESTC(m_ulProgress <= m_ulProgressMax);
		TESTC(m_ulAsynchPhase == DBASYNCHPHASE_INITIALIZATION || m_ulAsynchPhase == DBASYNCHPHASE_POPULATION || m_ulAsynchPhase == DBASYNCHPHASE_COMPLETE);
	}

	//Warnings
	if(m_ulProgress < m_ulPrevProgress)
		TWARNING("GetStatus() Previous Progress = " << m_ulPrevProgress << " Current Progress = " << m_ulProgress); 
	if(m_ulProgressMax < m_ulPrevProgressMax)
		TWARNING("GetStatus() Previous ProgressMax = " << m_ulPrevProgressMax << " Current ProgressMax = " << m_ulProgressMax); 
	if(m_pwszStatusText)
		TWARNING("GetStatus() Unable to verify text \"" << m_pwszStatusText << "\""); 

CLEANUP:
	if(pulProgress)
		*pulProgress = m_ulProgress;
	if(pulProgressMax)
		*pulProgressMax = m_ulProgressMax;
	if(pulAsynchPhase)
		*pulAsynchPhase = m_ulAsynchPhase;
	if(ppwszStatusText)
		*ppwszStatusText = m_pwszStatusText;
	return hr;
}


BOOL CAsynch::VerifyAbort()
{
	TBEGIN
	HRESULT hr = S_OK;
	
	//Abort
	hr = Abort();
	TESTC(hr==S_OK || hr==DB_E_CANTCANCEL);

	if(hr==S_OK)
	{
		if(GetAsynchPhase() == DBASYNCHPHASE_INITIALIZATION)
		{
			WaitUntilComplete();
			QTESTC(VerifyZombie());
		}
		else	//DBASYNCHPHASE_POPULATION or DBASYNCHPHASE_COMPLETE
		{
			//Should not be Zombied, usable
			TESTC_(GetStatus(), DB_E_CANCELED);
		}
	}
	else
	{
		TESTC_(GetStatus(), S_OK);
		TWARNING("Unable to Abort operation?");
	}

CLEANUP:
	TRETURN
}

BOOL CAsynch::VerifyZombie()
{
	TBEGIN
	
	//Abort was previsouly called
	TESTC_(GetStatus(), E_UNEXPECTED);
	TESTC_(Abort(), E_UNEXPECTED);

CLEANUP:
	TRETURN
}

	
BOOL CAsynch::IsComplete()
{
	//IDBAsynchStatus::GetStatus
	HRESULT hr = GetStatus();
		
	//Exit if were done...
	if(GetAsynchPhase() == DBASYNCHPHASE_COMPLETE)
		return TRUE;
	
	return FALSE;
}


HRESULT CAsynch::WaitUntilComplete()
{
	HRESULT hr = E_FAIL;

	//We need to poll the Status until its complete
	while(TRUE)
	{
		//IDBAsynchStatus::GetStatus
		hr = GetStatus();
		
		//If an error occurred return the error
		if(FAILED(hr))
			return hr;

		//Check for an error in the provider
		if(GetAsynchPhase()!=DBASYNCHPHASE_INITIALIZATION && GetAsynchPhase()!=DBASYNCHPHASE_POPULATION && GetAsynchPhase()!=DBASYNCHPHASE_COMPLETE)
			return E_FAIL;

		//As soon as we have S_OK and COMPLETE we are done...
		if(GetAsynchPhase()==DBASYNCHPHASE_COMPLETE)
			break;

		//Sleep a little, so we don't have such a tight loop...
		Sleep(100);
	}
	
	return hr;
}


BOOL CAsynch::VerifyNotification(CListener* pCListener, DWORD dwResult, ULONG ulTimesConnected)
{
	TBEGIN
	if(IsAsynch())
	{
		TCOMPARE_(pCListener->GetTimesNotified() >= ulTimesConnected);

		//Verify Correct Notifications
		TCOMPARE_(pCListener->GetTimesNotified(DBREASON_ONLOWRESOURCE)	== 0);
		TCOMPARE_(pCListener->GetTimesNotified(DBREASON_ONSTOP)			== 1*ulTimesConnected);

		TCOMPARE_(pCListener->GetTimesNotified(DBREASON_ONPROGRESS,		DBASYNCHPHASE_POPULATION)		>= 0*ulTimesConnected);
		TCOMPARE_(pCListener->GetTimesNotified(DBREASON_ONPROGRESS,		DBASYNCHPHASE_INITIALIZATION)	>= 0*ulTimesConnected);
		
		//If tried to Abort the Operation
		//We have 3 options
		//		1.  Abort, and operation continued (CantCancel or Veto'd the abort)
		//		2.  Abort, and operation was aborted
		//		3.  Operation completed as expected

		if(dwResult & OP_ABORTED)
		{
			if(dwResult & OP_COMPLETED)
			{
				//1.  Abort, and operation continued (CantCancel or Veto'd the abort)
				TCOMPARE_(pCListener->GetTimesNotified(DBREASON_ONPROGRESS,	DBASYNCHPHASE_COMPLETE)		== 1*ulTimesConnected);
			}
			else
			{
				//2.  Abort, and operation was aborted
				TCOMPARE_(pCListener->GetTimesNotified(DBREASON_ONPROGRESS,	DBASYNCHPHASE_COMPLETE)		== 0*ulTimesConnected);
				TCOMPARE_(pCListener->GetTimesNotified(DBREASON_ONPROGRESS,	DBASYNCHPHASE_CANCELED)		== 1*ulTimesConnected);
			}
		}
		else
		{
			//3.  Operation completed as expected
			TCOMPARE_(pCListener->GetTimesNotified(DBREASON_ONPROGRESS,	DBASYNCHPHASE_COMPLETE)			== 1*ulTimesConnected);
			TCOMPARE_(pCListener->GetTimesNotified(DBREASON_ONPROGRESS,	DBASYNCHPHASE_CANCELED)			== 0*ulTimesConnected);
		}
	}
	else
	{
		//We should not receive notifications if we are not in Asynch mode
		if(pCListener->GetTimesNotified() != 0)
		{
			TERROR("Received Notifications but not in Asynch mode!");
		}
	}

	TRETURN
}

	
//////////////////////////////////////////////////////////////////////////
// Class CDataSource
//
// Basically a nice wrapper arround a CAsynchDSO.
// Allows creation and manipulation of asynchronous and 
// sycnhronous  DSO's
//////////////////////////////////////////////////////////////////////////
class CAsynchDSO : public CDataSource, public CAsynch
{
public:
	//contsructor
	CAsynchDSO();
	virtual ~CAsynchDSO();

	virtual BOOL	CreateSource();
	virtual	BOOL	ResetSource(DWORD dwPropVal = DBPROPVAL_ASYNCH_INITIALIZE);
	virtual BOOL	CauseNotification();
	virtual BOOL	VerifyZombie();

	//Helpers
	virtual HRESULT Initialize(DWORD dwPropVal = DBPROPVAL_ASYNCH_INITIALIZE);

protected:
	//data
};


//////////////////////////////////////////////////////////////////////////
// class CAsynchDSO
//
//////////////////////////////////////////////////////////////////////////
CAsynchDSO::CAsynchDSO() 
	: CDataSource(INVALID(WCHAR*)), CAsynch(IID_IDBAsynchNotify, DATASOURCE_INTERFACE)
{
}

CAsynchDSO::~CAsynchDSO()
{
}	

BOOL CAsynchDSO::CreateSource()
{
	TBEGIN

	//Create an Instance first, so we can hook up out listeners, 
	//before any notifications
	TESTC_(CDataSource::CreateInstance(),S_OK);
	
	//Now that we have a DataSource, obtain Connection stuff
	QTESTC(CreateCP(m_pIDBInitialize));
	
CLEANUP:
	TRETURN;
}


BOOL CAsynchDSO::ResetSource(DWORD dwPropVal)
{
	CAsynch::ResetSource(dwPropVal);
	return CreateSource();
}

BOOL CAsynchDSO::CauseNotification()
{
	CAsynch::ResetSource();
	HRESULT hr = Initialize();

	return hr == S_OK || hr == DB_S_ASYNCHRONOUS;
}


BOOL CAsynchDSO::VerifyZombie()
{
	TBEGIN

	//Verify DataSource is Zombied
	TESTC_(m_pIDBInitialize->Initialize(), E_UNEXPECTED);

	//Verify Asynch Interface is Zombied
	QTESTC(CAsynch::VerifyZombie());

CLEANUP:
	TRETURN
}


HRESULT CAsynchDSO::Initialize(DWORD dwPropVal)
{
	TBEGIN
	HRESULT hr = S_OK;

	//May need to create the DataSource (CoCreate)
	if(m_pIDBInitialize == NULL)
		TESTC(CreateSource())
	else
		hr = CDataSource::Uninitialize();


	//Set DBPROP_INIT_ASYNCH (if Settable)
	//We may be testing negative cases, so set it if its not a valid value...
	FreeProperties();

	if (dwPropVal == DBPROPVAL_ASYNCH_INITIALIZE)
		TESTC(SetSettableProperty(DBPROP_INIT_ASYNCH, DBPROPSET_DBINIT, (void*)(ULONG_PTR)dwPropVal, DBTYPE_I4))
	else
		//may test invalid cases
		SetProperty(DBPROP_INIT_ASYNCH, DBPROPSET_DBINIT, (void*)(ULONG_PTR)dwPropVal, DBTYPE_I4);

	//Initialize
	//Had better be Asynchronous if were are getting this far...
	hr = CDataSource::Initialize();
	QTESTC(hr==S_OK || hr==DB_S_ASYNCHRONOUS);

	if(hr == S_OK)
	{
		//Provider Supports IDBAsynchNotify, 
		//Execution completely immediatly!
		m_ulAsynch = 0;

		//This is kind of a wierd state to verify programmically
		//The spec actually allows S_OK, for those causes were Initalization
		//occurs immediatly.  Normally DB_S_ASYNCHRONOUS should be returned 
		//to inform the consumer to consintue with their tasks...
		TWARNING("IDBInitialize::Initialize returned S_OK, is the DataSource created Immediatly?");
	}
	else if(SUCCEEDED(QI(m_pIDBInitialize, IID_IDBProperties)))
	{
		//DBPROP_INIT_ASYNCH
		//We may not be able to obtain the IDBProperties interface to determine
		//asynch status, so if not we have to total base it on the return code
		TESTC(GetProperty(DBPROP_INIT_ASYNCH, DBPROPSET_DBINIT, m_pIDBInitialize, &m_ulAsynch));
		TESTC(m_ulAsynch & DBPROPVAL_ASYNCH_INITIALIZE);
	}
	else
	{
		m_ulAsynch = DBPROPVAL_ASYNCH_INITIALIZE;
	}

CLEANUP:
	return hr;
}



//////////////////////////////////////////////////////////////////////////
// class CAsynchRowset
//
//////////////////////////////////////////////////////////////////////////
class CAsynchRowset : public CRowset, public CAsynch 
{
public :
	//Constructors
	CAsynchRowset(EQUERY eQuery = USE_SUPPORTED_SELECT_ALLFROMTBL);
	virtual ~CAsynchRowset();
	
	virtual BOOL	CreateSource();
	virtual	BOOL	ResetSource(DWORD dwPropVal = DBPROPVAL_ASYNCH_INITIALIZE);
	virtual BOOL	CauseNotification();
	virtual BOOL	VerifyZombie();

	//Helpers
	//virtual HRESULT CreateRowset(DWORD dwPropVal = DBPROPVAL_ASYNCH_INITIALIZE);
	virtual BOOL CreateRowset(DWORD dwPropVal = DBPROPVAL_ASYNCH_INITIALIZE);
protected:
	//data
	EQUERY	m_eQuery;
};


CAsynchRowset::CAsynchRowset(EQUERY eQuery)
	: CRowset(INVALID(WCHAR*)), CAsynch(IID_IDBAsynchNotify, ROWSET_INTERFACE)
{
	m_eQuery = eQuery;
}

CAsynchRowset::~CAsynchRowset()
{
}

BOOL CAsynchRowset::CreateSource()
{
	TBEGIN

	//CreateRowset
	//HRESULT hr = CreateRowset();
	//QTESTC(hr==S_OK || hr==DB_S_ASYNCHRONOUS);
	QTESTC(CreateRowset(m_dwAsynchPropVal));

	//Now that we have a rowset object, Initailize our CSource object
	//Since IDBAsynchStatus is OFF the Rowset Object, we actually have to create 
	//the Rowset and then connect our listeners (quickly) hoping we didn't miss
	//any notifcations, seems like a flaw in the design?
	QTESTC(CreateCP(pIUnknown()));
			
CLEANUP:
	TRETURN
}

BOOL CAsynchRowset::ResetSource(DWORD dwPropVal)
{
	TBEGIN

	//We need to drop the previous rowset...
	CAsynch::ResetSource(dwPropVal);
	DropRowset();

	//Delegate
	QTESTC(CAsynchRowset::CreateSource());

CLEANUP:
	TRETURN
}

BOOL CAsynchRowset::CauseNotification()
{
	TBEGIN

	//We need to drop the previous rowset...
//	DropRowset();

	//Delegate
//	CAsynch::ResetSource();
//	QTESTC(CreateSource());

//CLEANUP:
	TRETURN;
}


BOOL CAsynchRowset::VerifyZombie()
{
	TBEGIN
	HRESULT hr = S_OK;
	IRowset* pIRowset = NULL;

	//Verify Rowset is Zombied
	hr = QI(pIUnknown(), IID_IRowset, (void**)&pIRowset);
	TESTC(hr==S_OK || hr==E_NOINTERFACE);

	//May still be able to get the interface, but it should be usable
	if(hr==S_OK)
	{
		TESTC(pIRowset != NULL);
		TESTC_(pIRowset->RestartPosition(NULL), E_UNEXPECTED);
	}

	//Verify Asynch Interface is Zombied
	TESTC(CAsynch::VerifyZombie());

CLEANUP:
	SAFE_RELEASE(pIRowset);
	TRETURN
}


BOOL CAsynchRowset::CreateRowset(DWORD dwPropVal)
{
	TBEGIN
	HRESULT hr = S_OK;

	//Drop any previous rowset...
	DropRowset();
	
	//Set DBPROP_IConnectionPointContainer (Optional)
	//Problem is that some rowsets (providers) may not support IConnectionPointContainer
	//on the actual rowset, but the Asynch Service Components may support it just
	//so you and get IDBAsynchNotify...
	SetProperty(DBPROP_IConnectionPointContainer, DBPROPSET_ROWSET, (void*)VARIANT_TRUE, DBTYPE_BOOL, DBPROPOPTIONS_OPTIONAL);


	//Set DBPROP_ROWSET_ASYNCH (if Settable)
	TESTC(SetSettableProperty(DBPROP_ROWSET_ASYNCH, DBPROPSET_ROWSET, (void*)(ULONG_PTR)dwPropVal, DBTYPE_I4));

	//Deletgate to our CRowset Object
	//Need to obtain interface, since IAccessor connot be obtained...
	hr = CRowset::CreateRowset(m_eQuery, IID_IUnknown);
	QTESTC(hr==S_OK || hr==DB_S_ASYNCHRONOUS);

	if(hr==S_OK)
	{
		//Execution Completed Immediatly...
		m_ulAsynch = 0;

		//This is kind of a wierd state to verify programmically
		//The spec actually allows S_OK, for those causes were Initalization
		//occurs immediatly.  Normally DB_S_ASYNCHRONOUS should be returned 
		//to inform the consumer to consintue with their tasks...
		
		if (dwPropVal & (DBPROPVAL_ASYNCH_INITIALIZE | DBPROPVAL_ASYNCH_SEQUENTIALPOPULATION | DBPROPVAL_ASYNCH_RANDOMPOPULATION))
		{
			TWARNING("Rowset creation returned S_OK, is the rowset was created Immediatly?");
		}
	}
	else if(SUCCEEDED(QI(pIUnknown(), IID_IRowsetInfo)))
	{
		//DBPROP_ROWSET_ASYNCH
		//We may not be able to obtain the IRowsetInfo interface to determine
		//asynch status, so if not we have to total base it on the return code
		TESTC(GetProperty(DBPROP_ROWSET_ASYNCH, DBPROPSET_ROWSET, &m_ulAsynch));
		TESTC(m_ulAsynch & (DBPROPVAL_ASYNCH_INITIALIZE | DBPROPVAL_ASYNCH_SEQUENTIALPOPULATION | DBPROPVAL_ASYNCH_RANDOMPOPULATION));
		TESTC(m_ulAsynch & dwPropVal);
	}
	else
	{
		//m_ulAsynch = DBPROPVAL_ASYNCH_INITIALIZE;
		m_ulAsynch  = dwPropVal;
	}

CLEANUP:
//	return hr;
	TRETURN;
}




//////////////////////////////////////////////////////////////////////////
// class CAsynchRow
//
//////////////////////////////////////////////////////////////////////////
class CAsynchRow : public CRowObject, public CAsynch 
{
public :
	//Constructors
	CAsynchRow(EQUERY eQuery = USE_OPENROWSET);
	virtual ~CAsynchRow();
	
	virtual BOOL	CreateSource();
	virtual	BOOL	ResetSource(DWORD dwPropVal = DBPROPVAL_ASYNCH_INITIALIZE);
	virtual BOOL	CauseNotification();
	virtual BOOL	VerifyZombie();

	//Helpers

protected:
	//data
	EQUERY	m_eQuery;
};


CAsynchRow::CAsynchRow(EQUERY eQuery)
	: CRowObject(), CAsynch(IID_IDBAsynchNotify, ROW_INTERFACE)
{
	m_eQuery = eQuery;
}

CAsynchRow::~CAsynchRow()
{
}

BOOL CAsynchRow::CreateSource()
{
	TBEGIN
	HRESULT hr = S_OK;
	IRow* pIRow = NULL;

	//Create a row object
	//Row Objects are optional behavior...
	TEST3C_(hr = g_pTable->CreateRowset(m_eQuery, IID_IRow, 0, NULL, (IUnknown**)&pIRow),S_OK,DB_S_NOTSINGLETON,E_NOINTERFACE);

	//Make sure row objects are really not supported
	if(FAILED(hr))
	{
		TESTC(!SupportedProperty(DBPROP_IRow, DBPROPSET_ROWSET));
		TESTC(!SupportedProperty(DBPROP_IGetRow, DBPROPSET_ROWSET));
		QTESTC(FALSE);
	}

	//See if connection points are available (more optional behavior), 
	TESTC_(SetRowObject(pIRow),S_OK);
	QTESTC(CreateCP(pIRow));

CLEANUP:
	SAFE_RELEASE(pIRow);
	TRETURN
}

BOOL CAsynchRow::ResetSource(DWORD dwPropVal)
{
	TBEGIN
	HRESULT hr = S_OK;

	//We need to drop the previous rowset...
	CAsynch::ResetSource(dwPropVal);

	//Delegate
	QTESTC(hr = CreateSource());

CLEANUP:
	TRETURN
}

BOOL CAsynchRow::CauseNotification()
{
	TBEGIN

	//We need to drop the previous rowset...
//	DropRowset();

	//Delegate
//	CAsynch::ResetSource();
//	QTESTC(CreateSource());

//CLEANUP:
	TRETURN;
}


BOOL CAsynchRow::VerifyZombie()
{
	TBEGIN

	//TODO

	//Verify Asynch Interface is Zombied
	TESTC(CAsynch::VerifyZombie());

CLEANUP:
	TRETURN
}


//////////////////////////////////////////////////////////////////////////
// class CAsynchStream
//
//////////////////////////////////////////////////////////////////////////
class CAsynchStream : public CRowObject, public CAsynch 
{
public :
	//Constructors
	CAsynchStream(EQUERY eQuery = USE_OPENROWSET);
	virtual ~CAsynchStream();
	
	virtual BOOL	CreateSource();
	virtual	BOOL	ResetSource(DWORD dwPropVal = DBPROPVAL_ASYNCH_INITIALIZE);
	virtual BOOL	CauseNotification();
	virtual BOOL	VerifyZombie();

	//Helpers

protected:
	//data
	EQUERY	m_eQuery;
};


CAsynchStream::CAsynchStream(EQUERY eQuery)
	: CRowObject(), CAsynch(IID_IDBAsynchNotify, STREAM_INTERFACE)
{
	m_eQuery = eQuery;
}

CAsynchStream::~CAsynchStream()
{
}

BOOL CAsynchStream::CreateSource()
{
	TBEGIN
	HRESULT hr = S_OK;
	IRow* pIRow = NULL;
	IUnknown* pIUnknown = NULL;
	DBORDINAL iCol=0;

	ULONG cPropSets = 0;
	DBPROPSET* rgPropSets = NULL;

	//Create a row object
	//Row Objects are optional behavior...
	TEST3C_(hr = g_pTable->CreateRowset(m_eQuery, IID_IRow, cPropSets, rgPropSets, (IUnknown**)&pIRow),S_OK,DB_S_NOTSINGLETON,E_NOINTERFACE);

	//Make sure row objects are really not supported
	if(FAILED(hr))
	{
		TESTC(!SupportedProperty(DBPROP_IRow, DBPROPSET_ROWSET));
		TESTC(!SupportedProperty(DBPROP_IGetRow, DBPROPSET_ROWSET));
		QTESTC(FALSE);
	}

	//Now that we have a row object, dump it into our helper CRowObject class...
	TESTC_(SetRowObject(pIRow),S_OK);

	//See if we can obtain a stream object over any of the columns...
	for(iCol=0; iCol<m_cColAccess; iCol++)
	{
		//Open will mainly only be able to be called for columns containing objects.
		//But some providers might be able to open streams, or other types of objects ontop
		//of non-object valued columns.
		hr = Open(NULL, &m_rgColAccess[iCol].columnid, DBGUID_STREAM, IID_IUnknown, &pIUnknown);
		TEST3C_(hr, S_OK, S_FALSE, DB_E_OBJECTMISMATCH);

		if(SUCCEEDED(hr))
		{
			//Now that we have a stream object, 
			//if it supports connection points, we are done...
			if(CreateCP(pIUnknown))
				break;
		}

		SAFE_RELEASE(pIUnknown);
	}
	
	//At this point we should a stream object, otherwise return false
	QTESTC(pIUnknown != NULL);

CLEANUP:
	SAFE_RELEASE(pIUnknown);
	SAFE_RELEASE(pIRow);
	TRETURN
}

BOOL CAsynchStream::ResetSource(DWORD dwPropVal)
{
	TBEGIN
	HRESULT hr = S_OK;

	//We need to drop the previous rowset...
	CAsynch::ResetSource(dwPropVal);

	//Delegate
	QTESTC(hr = CreateSource());

CLEANUP:
	TRETURN
}

BOOL CAsynchStream::CauseNotification()
{
	TBEGIN

	//We need to drop the previous rowset...
//	DropRowset();

	//Delegate
//	CAsynch::ResetSource();
//	QTESTC(CreateSource());

//CLEANUP:
	TRETURN;
}


BOOL CAsynchStream::VerifyZombie()
{
	TBEGIN

	//TODO

	//Verify Asynch Interface is Zombied
	TESTC(CAsynch::VerifyZombie());

CLEANUP:
	TRETURN
}


////////////////////////////////////////////////////////////////////////////
//  TCBase
//
////////////////////////////////////////////////////////////////////////////
class TCBase
{
public:
	//constructor
	TCBase() 
	{ 
		m_dwTestCaseParam = 0;
	}

	//methods
	virtual void SetTestCaseParam(DWORD dwTestCaseParam)
	{
		m_dwTestCaseParam = dwTestCaseParam;
	}

	//data
	DWORD m_dwTestCaseParam;
};

//////////////////////////////////////////////////////////////////
// Class CTestNotify
//
//////////////////////////////////////////////////////////////////
class CTestNotify : public COLEDB, public TCBase
{
public:
	//contsructor
	CTestNotify(WCHAR * pwszTestCaseName);
	virtual ~CTestNotify();

	virtual BOOL	Init();
	virtual BOOL	Terminate();

	//members
	virtual BOOL	InitSources();
	virtual BOOL	ResetSources(DWORD dwPropVal = DBPROPVAL_ASYNCH_INITIALIZE);
	virtual	BOOL	ResetListeners();

	//Connections
	virtual BOOL	CauseNotifications();
	virtual HRESULT WaitUntilComplete();

	virtual BOOL	VerifyAllConnections(DWORD dwResult = OP_COMPLETED);
	virtual BOOL	VerifyConnection(CListener* pCListener, CAsynch* pCAsynch, DWORD dwResult = OP_COMPLETED, ULONG ulTimesConnected = ULONG_MAX);
	virtual BOOL	VerifyNoConnection(); 

	virtual BOOL	VerifyEvent(CListener* pCListener, DBASYNCHREASON eReason, DBASYNCHPHASE ePhase = DBEVENTPHASE_ALL);
	virtual BOOL	VerifyEventAll(DBASYNCHREASON eReason, DBASYNCHPHASE ePhase = DBEVENTPHASE_ALL);
	virtual BOOL	VerifyUnwantedEvent(CListener* pCListener, DBASYNCHREASON eReason, DBASYNCHPHASE ePhase = DBEVENTPHASE_ALL);
	virtual BOOL	VerifyUniqueCookie(DWORD dwCookie, CSource* pCSource);

	//Advise connections
	virtual BOOL	AdviseAll();
	virtual BOOL	Advise(CListener* pCListener, CSource* pCSource, DWORD* pdwCookie = NULL);
	
	//Unadvise Connections
	virtual BOOL	UnadviseAll();
	virtual BOOL	Unadvise(CListener* pCListener, CSource* pCSource);

	//Indexing
	virtual CListener*		pCListener(ULONG iListener)	{ return m_vectListeners[iListener];	} 
	virtual CAsynch*		pCSource(ULONG iSource)		{ return m_vectSources[iSource];		}

protected:
	//Associated (Multiple) Listeners
	CVector<CListener*>		m_vectListeners;	//array of Listeners

	//Associated (Multiple) Source
	CVector<CAsynch*>		m_vectSources;		//array of Sources
	
	//Track connections between sources and listeners
	CList<CConnectInfo*, CConnectInfo*>	m_listConnectInfo;
	CRITICAL_SECTION ConnTableMutex;
};



//////////////////////////////////////////////////////////////////
// Class CTestNotify
//
//////////////////////////////////////////////////////////////////
CTestNotify::CTestNotify(WCHAR * pwszTestCaseName): COLEDB(pwszTestCaseName)
{
}

CTestNotify::~CTestNotify()
{
}


BOOL CTestNotify::Init()
{
	TBEGIN
	ULONG i=0;
	//This must be done and not skipped by any error
	//Since we delete the critical section in the Terminate...
	InitializeCriticalSection(&ConnTableMutex);

	//Init Listeners
	for(i=0; i<5; i++)
	{
		CListener* pCListener = new CListener(IID_IDBAsynchNotify);
		TESTC(pCListener != NULL);
	
		//Make sure it has an extra reference count, so its arround
		//For the lifetime of the TestCase.  Otherwise it would be deleted
		//After the first rowset unadvises it.  We will call Release in Terminate.
		SAFE_ADDREF(pCListener);
		m_vectListeners.AddElement(pCListener);
	}

CLEANUP:
	TRETURN;
}


BOOL CTestNotify::Terminate()
{
	DBCOUNTITEM i;
	//Free the list of Sources
	for(i=0; i<m_vectSources.GetCount(); i++) 
		SAFE_DELETE(m_vectSources[i]);
	m_vectSources.RemoveAll();

	//Free the list of listeners
	for(i=0; i<m_vectListeners.GetCount(); i++) 
	{
		//Make sure their are no outstanding references on the listeners...
		SAFE_RELEASE_(m_vectListeners[i]);
	}
	m_vectListeners.RemoveAll();


	DeleteCriticalSection(&ConnTableMutex);
	return TRUE;
}


BOOL CTestNotify::InitSources() 
{
	TBEGIN
	IDBCreateCommand *pIDBCreateCommand = NULL;
	ULONG i = 0;
		// Test if necessary interfaces and properties are supported/settable
		switch(m_dwTestCaseParam)
		{
			case STREAM_INTERFACE:
			case ROW_INTERFACE:
				TEST_PROVIDER(SupportedInterface(IID_IDBAsynchNotify, ROW_INTERFACE));
				TEST_SUPPORTED_PROP(DBPROP_ROWSET_ASYNCH, DBPROPSET_ROWSET);
				TEST_SETTABLE_PROP(DBPROP_ROWSET_ASYNCH, DBPROPSET_ROWSET);
				break;

			case DATASOURCE_INTERFACE:
				TEST_PROVIDER(SupportedInterface(IID_IDBAsynchNotify, DATASOURCE_INTERFACE));
				TEST_SUPPORTED_PROP(DBPROP_INIT_ASYNCH, DBPROPSET_DBINIT);
				TEST_SETTABLE_PROP(DBPROP_INIT_ASYNCH, DBPROPSET_DBINIT);
				break;

			case ROWSET_INTERFACE:
			case COMMAND_INTERFACE:
			case ENUMERATOR_INTERFACE:
			// if Commands are not supported skip and log information 
				if (COMMAND_INTERFACE==m_dwTestCaseParam)
				{
						if(!VerifyInterface(g_pIOpenRowset, IID_IDBCreateCommand,
							SESSION_INTERFACE, (IUnknown**)&pIDBCreateCommand))
						{
							TOUTPUT_LINE(L"Commands are not supported by this Provider");
							TESTB = TEST_SKIPPED;
							TRETURN
						}
						SAFE_RELEASE(pIDBCreateCommand);
				}
				TEST_PROVIDER(SupportedInterface(IID_IDBAsynchNotify, ROWSET_INTERFACE));
				TEST_SUPPORTED_PROP(DBPROP_ROWSET_ASYNCH, DBPROPSET_ROWSET);
				TEST_SETTABLE_PROP(DBPROP_ROWSET_ASYNCH, DBPROPSET_ROWSET);
				break;

			default:
				ASSERT(!L"Unhandled TestCase Type!");
				break;
	 }

	//Init Sources
	for(i=0; i<3; i++)
	{
		CAsynch* pCSource = NULL;
		
		//First 2 sources have the possibility of being row sources
		switch(m_dwTestCaseParam)
		{
			case STREAM_INTERFACE:
				pCSource = new CAsynchStream;
				break;

			case ROW_INTERFACE:
				pCSource = new CAsynchRow;
				break;

			case DATASOURCE_INTERFACE:
				pCSource = new CAsynchDSO;
				break;

			case ROWSET_INTERFACE:
				pCSource = new CAsynchRowset;
				break;

			case COMMAND_INTERFACE:
				pCSource = new CAsynchRowset(SELECT_VALIDATIONORDER);//(SELECT_ORDERBYNUMERIC);
				break;

			case ENUMERATOR_INTERFACE:
				pCSource = new CAsynchRowset(SELECT_DBSCHEMA_TABLE);
				break;

			default:
				ASSERT(!L"Unhandled TestCase Type!");
				break;
		};

		//Initialize the source
		TESTC(pCSource != NULL);

		//Add this source to the list...
		m_vectSources.AddElement(pCSource);
		TESTC_PROVIDER(pCSource->CreateSource())
	}

CLEANUP:
	TRETURN
}


BOOL CTestNotify::ResetSources(DWORD dwPropVal)
{
	TBEGIN
	ULONG i = 0;
	//Reset all listeners
	ResetListeners();

	//Reset all Sources
	for(i=0; i<m_vectSources.GetCount(); i++)
		QTESTC(pCSource(i)->ResetSource(dwPropVal));

CLEANUP: 
	TRETURN
}


BOOL CTestNotify::ResetListeners()
{
	TBEGIN
	ULONG i = 0;
	//Reset all listeners
	for(i=0; i<m_vectListeners.GetCount(); i++)
		TESTC(pCListener(i)->ResetTimesNotified());

CLEANUP: 
	TRETURN
}


BOOL CTestNotify::VerifyUniqueCookie(DWORD dwCookie, CSource* pCSource)
{
	//NOTE: This method only verifies Cookies from the same Container
	//are unique, since Cookies may not be unqiue accross Containers.

	//Loop through all the current connection cookies.
	POSITION pos = m_listConnectInfo.GetHeadPosition();
	while(pos) 
	{
		//Obtain the connect info
		CConnectInfo* pCConnectInfo = m_listConnectInfo.GetNext(pos);

		//verify different connection cookies
		//Only compare Cookies from the same Container (CSource)
		if(pCSource == pCConnectInfo->m_pCSource)
		{
			if(dwCookie == pCConnectInfo->m_dwCookie)
				return FALSE;
		}
	}
					
	return TRUE;
}

BOOL CTestNotify::Advise(CListener* pCListener, CSource* pCSource, DWORD* pdwCookie)
{
	//Advise iListener -> iSource, and need cookie
	ASSERT(pCListener);
	ASSERT(pCSource);
	
	TBEGIN
	DWORD dwCookie = 0;
	CConnectInfo* pCConnectInfo = NULL;
	BOOL bUnique = FALSE;

	//Record the cuurent ref count
	ULONG cRefCount = GetRefCount(pCListener);
	
	//Advise
	TESTC_(pCSource->GetCP()->Advise(pCListener, &dwCookie),S_OK);
	
	//verify valid cookie
	TESTC(dwCookie != 0);
	
	//Before we add this to the list, verify its a unique cookie...
	//Don't actually exit if it's not as we still need to add it to the list,
	//so we can free it later...
	bUnique = VerifyUniqueCookie(dwCookie, pCSource);

	//Add this connection info to the list
	EnterCriticalSection(&ConnTableMutex);
		pCConnectInfo = new CConnectInfo(dwCookie, pCListener, pCSource);
		m_listConnectInfo.AddTail(pCConnectInfo);
	LeaveCriticalSection(&ConnTableMutex);
	
	//Before we add this to the list, verify its a unique cookie...
	TESTC(bUnique == TRUE);

	//Verify newly advised connection		 
//	TESTC(VerifyConnection(pCListener, (CAsynch*)pCSource));
	
	//Verify refcount is greater after advise
	TESTC(VerifyRefCounts(GetRefCount(pCListener), cRefCount+1));

CLEANUP:		
	if(pdwCookie)
		*pdwCookie = dwCookie;
	TRETURN
}

BOOL CTestNotify::Unadvise(CListener* pCListener, CSource* pCSource)
{
	TBEGIN
	ASSERT(pCListener);
	ASSERT(pCSource);

	//Record the cuurent ref count
	ULONG cRefCount = GetRefCount(pCListener);
	
	//Find Listener/CSource in list
	POSITION pos = m_listConnectInfo.GetHeadPosition();
	while(pos) 
	{
		POSITION posSave = pos;
		CConnectInfo* pCConnectInfo = m_listConnectInfo.GetNext(pos);
		if(pCListener == pCConnectInfo->m_pCListener && pCSource == pCConnectInfo->m_pCSource)
		{
			//Unadvise the connection
			TESTC_(pCSource->GetCP()->Unadvise(pCConnectInfo->m_dwCookie),S_OK);
				
			//Remove from list
			EnterCriticalSection(&ConnTableMutex);
				m_listConnectInfo.RemoveAt(posSave);	
			LeaveCriticalSection(&ConnTableMutex);
		
			//Verify ref count
//			TESTC(VerifyConnection(pCListener, (CAsynch*)pCSource));
			TESTC(VerifyRefCounts(GetRefCount(pCListener), cRefCount-1));
			break;
		}
	}

CLEANUP:	
	TRETURN
}


BOOL CTestNotify::AdviseAll()
{
	TBEGIN
	ULONG iListener = 0;
	ULONG index = 0;
	ULONG iSource = 0;
	//Advise the specified number of connections, and verify
	for(iListener=0, index=0; iListener<m_vectListeners.GetCount(); iListener++)
	{
		for(iSource=0; iSource<m_vectSources.GetCount(); iSource++, index++)
		{
			TESTC(Advise(pCListener(iListener), pCSource(iSource)));
		}
	}
				

CLEANUP:	
	TRETURN
}


BOOL CTestNotify::UnadviseAll()
{
	TBEGIN

	//Unadvise all Cookies in the list
	//NOTE:  Unadvise will remove the element from the list so we can
	//just loop until either the list is empty or an error occurs
	while(!m_listConnectInfo.IsEmpty()) 
	{
		CConnectInfo* pCConnectInfo = m_listConnectInfo.GetHead();
		TESTC(Unadvise(pCConnectInfo->m_pCListener, pCConnectInfo->m_pCSource));
	}

	//Verify no connection
//	TESTC(VerifyNoConnection());
	
CLEANUP:	
	TRETURN
}


BOOL CTestNotify::CauseNotifications()
{
	TBEGIN
	ULONG i = 0;		
	//Reset all listeners
//	for(ULONG i=0; i<m_vectListeners.GetCount(); i++)
//		TESTC(pCListener(i)->ResetTimesNotified());

	//verify listener recieved notification
	for(i=0; i<m_vectSources.GetCount(); i++)
		QTESTC(pCSource(i)->CauseNotification());
		
CLEANUP:	
	TRETURN
}


HRESULT CTestNotify::WaitUntilComplete()
{
	TBEGIN
	HRESULT hr = S_OK;
	ULONG i = 0;		
	//We need to wait for the Notification or Completion...
	for(i=0; i<m_vectSources.GetCount(); i++)
		QTESTC_(hr = pCSource(i)->WaitUntilComplete(),S_OK);

CLEANUP:	
	return hr;
}


BOOL CTestNotify::VerifyAllConnections(DWORD dwResult)
{
	TBEGIN
	ULONG iListener = 0;
	HRESULT hrExpected = dwResult & OP_COMPLETED ? S_OK : DB_E_CANCELED;
	ULONG iSource = 0;
	//We need to wait for the Notification or Completion...
	TESTC_(WaitUntilComplete(), hrExpected);

	//verify listener recieved notification
	for(iListener=0; iListener<m_vectListeners.GetCount(); iListener++)
	{
		for(iSource=0; iSource<m_vectSources.GetCount(); iSource++)
		{
			TESTC(VerifyConnection(pCListener(iListener), pCSource(iSource), dwResult));
		}
	}
		
CLEANUP:	
	TRETURN
}


BOOL CTestNotify::VerifyNoConnection() 
{
	TBEGIN
	ULONG i=0;
	
	//We need to wait for the Notification or Completion...
	TESTC_(WaitUntilComplete(),S_OK);

	//verify listener recieved notification
	for(i=0; i<m_vectListeners.GetCount(); i++)
	{
		TESTC(pCListener(i)->GetTimesNotified() == 0);
	}
		
CLEANUP:	
	TRETURN
}

BOOL CTestNotify::VerifyEventAll(DBASYNCHREASON eReason, DBASYNCHPHASE ePhase)
{
	TBEGIN
	ULONG i = 0;
	for(i=0; i<m_vectListeners.GetCount(); i++)
		QTESTC(VerifyEvent(pCListener(i), eReason, ePhase));

CLEANUP:
	TRETURN
}
	
BOOL CTestNotify::VerifyEvent(CListener* pCListener, DBASYNCHREASON eReason, DBASYNCHPHASE ePhase)
{
	TBEGIN
	ASSERT(pCListener);

	//Calculate the number of Asynch Sources we have...
	ULONG cAsynchSources = 0;
	ULONG cRowsetSources = 0;
	ULONG i = 0;
	for(i=0; i<m_vectSources.GetCount(); i++)
	{
		if(pCSource(i)->IsAsynch())
			cAsynchSources++;
		if(pCSource(i)->GetObjectType() == ROWSET_INTERFACE)
			cRowsetSources++;
	}

	switch(eReason)
	{
		case DBREASON_ONLOWRESOURCE:
		{
			//This should never be called
			TESTC(pCListener->GetTimesNotified(DBREASON_ONLOWRESOURCE, DBEVENTPHASE_ALL) == 0);
			break;
		}

		case DBREASON_ONSTOP:
		{	
			//This should be called once for each Source
			TESTC(pCListener->GetTimesNotified(DBREASON_ONSTOP, DBEVENTPHASE_ALL) == cAsynchSources);
			break;
		}

		case DBREASON_ONPROGRESS:
		{	
			switch(ePhase)
			{
				case DBASYNCHPHASE_INITIALIZATION:
					//This maybe called (basically optional)...
					break;

				case DBASYNCHPHASE_POPULATION:
					//This maybe called (basically optional)...
					//This notification only is allowed during population of a rowset
					if(cRowsetSources)
					{
                        // Optional
					}
					else
					{
						TESTC(pCListener->GetTimesNotified(DBREASON_ONPROGRESS, DBASYNCHPHASE_POPULATION)	== 0);
					}
					break;

				case DBASYNCHPHASE_COMPLETE:
					//This must be called once for each Source
					TESTC(pCListener->GetTimesNotified(DBREASON_ONPROGRESS, DBASYNCHPHASE_COMPLETE)		== cAsynchSources);
					break;

				case DBEVENTPHASE_ALL:
					//This maybe called (basically optional)...

					//This maybe called (basically optional)...
					//This notification only is allowed during population of a rowset
					if(cRowsetSources)
					{
                        // Optional
					}
					else
					{
						TESTC(pCListener->GetTimesNotified(DBREASON_ONPROGRESS, DBASYNCHPHASE_POPULATION)	== 0);
					}

					//This must be called once for each Source
					TESTC(pCListener->GetTimesNotified(DBREASON_ONPROGRESS, DBASYNCHPHASE_COMPLETE)		== cAsynchSources);
					break;

				default:
					ASSERT(!"Unhandled Type!");
					break;
			}
			break;
		}

		default:
			ASSERT(!"Unhandled Type!");
			break;
	};

CLEANUP:
	TRETURN
}

BOOL CTestNotify::VerifyUnwantedEvent(CListener* pCListener, DBASYNCHREASON eReason, DBASYNCHPHASE ePhase)
{
	TBEGIN
	ASSERT(pCListener);

	//Calculate the number of Asynch Sources we have...
	ULONG cAsynchSources = 0;
	ULONG cRowsetSources = 0;
	ULONG i = 0;
	for(i=0; i<m_vectSources.GetCount(); i++)
	{
		if(pCSource(i)->IsAsynch())
			cAsynchSources++;
		if(pCSource(i)->GetObjectType() == ROWSET_INTERFACE)
			cRowsetSources++;
	}

	switch(eReason)
	{
		case DBREASON_ONLOWRESOURCE:
		{
			//This should never be called
			if(pCListener->GetTimesNotified(DBREASON_ONLOWRESOURCE, DBEVENTPHASE_ALL))
				TWARNING("Was notified for unwanted notification IDBAsynchNotify::OnLowResource()");
			break;
		}

		case DBREASON_ONSTOP:
		{	
			//This is an unwanted event, indicate a warning, since it was notified
			if(pCListener->GetTimesNotified(DBREASON_ONSTOP, DBEVENTPHASE_ALL))
				TWARNING("Was notified for unwanted notification IDBAsynchNotify::OnStop()");
			break;
		}

		case DBREASON_ONPROGRESS:
		{	
			switch(ePhase)
			{
				case DBASYNCHPHASE_INITIALIZATION:
					//This maybe called (basically optional)...
					if(pCListener->GetTimesNotified(DBREASON_ONPROGRESS, DBASYNCHPHASE_INITIALIZATION))
						TWARNING("Was notified for unwanted notification IDBAsynchNotify::OnProgress(DBASYNCHPHASE_INITIALIZATION)");
					break;

				case DBASYNCHPHASE_POPULATION:
					//This maybe called (basically optional)...
					//This notification only is allowed during population of a rowset
					if(cRowsetSources)
					{
						if(pCListener->GetTimesNotified(DBREASON_ONPROGRESS, DBASYNCHPHASE_POPULATION))
							TWARNING("Was notified for unwanted notification IDBAsynchNotify::OnProgress(DBASYNCHPHASE_POPULATION)");
					}
					else
					{
						TESTC(pCListener->GetTimesNotified(DBREASON_ONPROGRESS, DBASYNCHPHASE_POPULATION)	== 0);
					}
					break;

				case DBASYNCHPHASE_COMPLETE:
					//This must be called once for each Source
					if(pCListener->GetTimesNotified(DBREASON_ONPROGRESS, DBASYNCHPHASE_COMPLETE))
						TWARNING("Was notified for unwanted notification IDBAsynchNotify::OnProgress(DBASYNCHPHASE_COMPLETE)");
					break;

				case DBEVENTPHASE_ALL:
					//This maybe called (basically optional)...
					if(pCListener->GetTimesNotified(DBREASON_ONPROGRESS))
						TWARNING("Was notified for unwanted notification IDBAsynchNotify::OnProgress(DBASYNCHPHASE_ALL)");
					break;

				default:
					ASSERT(!"Unhandled Type!");
					break;
			}
			break;
		}

		default:
			ASSERT(!"Unhandled Type!");
			break;
	};

CLEANUP:
	TRETURN
}


BOOL CTestNotify::VerifyConnection(CListener* pCListener, CAsynch* pCAsynch, DWORD dwResult, ULONG ulTimesConnected)
{
	TBEGIN

	//Figure out how many times the sink is connected to the source
	if(ulTimesConnected == ULONG_MAX)
	{
		ulTimesConnected = 0;
		POSITION pos = m_listConnectInfo.GetHeadPosition();
		while(pos) 
		{
			CConnectInfo* pCConnectInfo = m_listConnectInfo.GetNext(pos);
			if(pCListener == pCConnectInfo->m_pCListener/* && pCAsynch == pCConnectInfo->m_pCSource*/)
				ulTimesConnected++;
		}
	}

	//verify listener recieved notification
	TESTC(pCAsynch->VerifyNotification(pCListener, dwResult, ulTimesConnected));
	
CLEANUP:
	TRETURN
}


		


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Test Case Section
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -



// {{ TCW_TEST_CASE_MAP(TCAsynchNotify)
//--------------------------------------------------------------------
// @class Test Notifications
//
class TCAsynchNotify : public CTestNotify { 
public:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCAsynchNotify,CTestNotify);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Test how many times notified
	int Variation_1();
	// @cmember verify multiple Listeners are advised
	int Variation_2();
	// @cmember Verify DBASYNCHPHASE_POPULATION is received
	int Variation_3();
	// @cmember Test DBPROPVAL_ASYNCH_SEQUENTIALPOPULATION
	int Variation_4();
	// @cmember Test DBPROPVAL_ASYNCH_RANDOMPOPULATION
	int Variation_5();
	// @cmember Test SEQUENTIAL | RANDOM
	int Variation_6();
	// @cmember L1 -OnProgress DB_S_UNWANTEDOPERATION L2, L3 receive all phases
	int Variation_7();
	// @cmember L1 -OnStop DB_S_UNWANTEDOPERATION L2, L3 receive all phases
	int Variation_8();
	// @cmember L1 -OnProgress DB_S_UNWANTEDPHASE L2, L3 receive all phases
	int Variation_9();
	// @cmember L1 -OnProgress DB_S_UNWANTEDPHASE L2, L3 receive all phases
	int Variation_10();
	// @cmember L1 -OnProgress DB_S_UNWANTEDPHASE L2, L3 receive all phases
	int Variation_11();
	// @cmember OnLowResource - Veto
	int Variation_12();
	// @cmember OnStop - Veto
	int Variation_13();
	// @cmember OnProgress - Veto CANCELED
	int Variation_14();
	// @cmember OnProgress - Veto INITIALIZATION
	int Variation_15();
	// @cmember OnProgress - Veto POPULATION
	int Variation_16();
	// @cmember OnProgress - Veto COMPLETE
	int Variation_17();
	// @cmember Advise Listener - after operation is complete
	int Variation_18();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCAsynchNotify)
#define THE_CLASS TCAsynchNotify
BEG_TEST_CASE(TCAsynchNotify, CTestNotify, L"Test Notifications")
	TEST_VARIATION(1, 		L"Test how many times notified")
	TEST_VARIATION(2, 		L"verify multiple Listeners are advised")
	TEST_VARIATION(3, 		L"Verify DBASYNCHPHASE_POPULATION is received")
	TEST_VARIATION(4, 		L"Test DBPROPVAL_ASYNCH_SEQUENTIALPOPULATION")
	TEST_VARIATION(5, 		L"Test DBPROPVAL_ASYNCH_RANDOMPOPULATION")
	TEST_VARIATION(6, 		L"Test SEQUENTIAL | RANDOM")
	TEST_VARIATION(7, 		L"L1 -OnProgress DB_S_UNWANTEDOPERATION L2, L3 receive all phases")
	TEST_VARIATION(8, 		L"L1 -OnStop DB_S_UNWANTEDOPERATION L2, L3 receive all phases")
	TEST_VARIATION(9, 		L"L1 -OnProgress DB_S_UNWANTEDPHASE L2, L3 receive all phases")
	TEST_VARIATION(10, 		L"L1 -OnProgress DB_S_UNWANTEDPHASE L2, L3 receive all phases")
	TEST_VARIATION(11, 		L"L1 -OnProgress DB_S_UNWANTEDPHASE L2, L3 receive all phases")
	TEST_VARIATION(12, 		L"OnLowResource - Veto")
	TEST_VARIATION(13, 		L"OnStop - Veto")
	TEST_VARIATION(14, 		L"OnProgress - Veto CANCELED")
	TEST_VARIATION(15, 		L"OnProgress - Veto INITIALIZATION")
	TEST_VARIATION(16, 		L"OnProgress - Veto POPULATION")
	TEST_VARIATION(17, 		L"OnProgress - Veto COMPLETE")
	TEST_VARIATION(18, 		L"Advise Listener - after operation is complete")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(TCAsynchNotifyFailure)
//--------------------------------------------------------------------
// @class Test Notification Error cases
//
class TCAsynchNotifyFailure : public CTestNotify { 
public:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCAsynchNotifyFailure,CTestNotify);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember E_FAIL - L1 , L1, L2 , L3 receive all phases
	int Variation_1();
	// @cmember E_FAIL - L1 L2 , L1, L2 , L3 receive all phases
	int Variation_2();
	// @cmember E_FAIL - L1 L2 , L1, L2 , L3 receive all phases
	int Variation_3();
	// @cmember E_FAIL - IRowsetLocate
	int Variation_4();
	// @cmember E_FAIL - IRowsetResynch
	int Variation_5();
	// @cmember E_FAIL - IRowsetScroll
	int Variation_6();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCAsynchNotifyFailure)
#define THE_CLASS TCAsynchNotifyFailure
BEG_TEST_CASE(TCAsynchNotifyFailure, CTestNotify, L"Test Notification Error cases")
	TEST_VARIATION(1, 		L"E_FAIL - L1 , L1, L2 , L3 receive all phases")
	TEST_VARIATION(2, 		L"E_FAIL - L1 L2 , L1, L2 , L3 receive all phases")
	TEST_VARIATION(3, 		L"E_FAIL - L1 L2 , L1, L2 , L3 receive all phases")
	TEST_VARIATION(4, 		L"E_FAIL - IRowsetLocate")
	TEST_VARIATION(5, 		L"E_FAIL - IRowsetResynch")
	TEST_VARIATION(6, 		L"E_FAIL - IRowsetScroll")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(TCAsynchStatus)
//--------------------------------------------------------------------
// @class Test Notification Error cases
//
class TCAsynchStatus : public CTestNotify { 
public:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCAsynchStatus,CTestNotify);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Check the HRESULT from GetStatus
	int Variation_1();
	// @cmember Verify ProgressMax == ProgressCurr
	int Variation_2();
	// @cmember check ulAsynchPhase is DBASYNCHPHASE_COMPLETE
	int Variation_3();
	// @cmember Verify ratio corresponds to returned staus code
	int Variation_4();
	// @cmember Verify ratio of ProgressCurr and ProgressMax
	int Variation_5();
	// @cmember Poll for status continuously
	int Variation_6();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCAsynchStatus)
#define THE_CLASS TCAsynchStatus
BEG_TEST_CASE(TCAsynchStatus, CTestNotify, L"Test Notification Error cases")
	TEST_VARIATION(1, 		L"Check the HRESULT from GetStatus")
	TEST_VARIATION(2, 		L"Verify ProgressMax == ProgressCurr")
	TEST_VARIATION(3, 		L"check ulAsynchPhase is DBASYNCHPHASE_COMPLETE")
	TEST_VARIATION(4, 		L"Verify ratio corresponds to returned staus code")
	TEST_VARIATION(5, 		L"Verify ratio of ProgressCurr and ProgressMax")
	TEST_VARIATION(6, 		L"Poll for status continuously")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(TCAsynchAbort)
//--------------------------------------------------------------------
// @class Test Notification Error cases
//
class TCAsynchAbort : public CTestNotify { 
public:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCAsynchAbort,CTestNotify);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Call abort after population completed, verify DB_E_CANTCANCEL
	int Variation_1();
	// @cmember GetStatus after Abort returns DB_E_CANTCANCEL returns S_OK
	int Variation_2();
	// @cmember Abort in Initialization phase, Get Status returns DB_E_CANCELED
	int Variation_3();
	// @cmember call GetStaus twice after Abort - E_UNEXPECTED
	int Variation_4();
	// @cmember check population is uncomplete after abort called
	int Variation_5();
	// @cmember Call Abort Again - E_UNEXPECTED
	int Variation_6();
	// @cmember Call OpenRowset again after Abort
	int Variation_7();
	// @cmember Call Abort Again - E_UNEXPECTED
	int Variation_8();
	// @cmember Abort - Invalid Arguments
	int Variation_9();
	// @cmember Advise Listener - after operation is aborted
	int Variation_10();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCAsynchAbort)
#define THE_CLASS TCAsynchAbort
BEG_TEST_CASE(TCAsynchAbort, CTestNotify, L"Test Notification Error cases")
	TEST_VARIATION(1, 		L"Call abort after population completed, verify DB_E_CANTCANCEL")
	TEST_VARIATION(2, 		L"GetStatus after Abort returns DB_E_CANTCANCEL returns S_OK")
	TEST_VARIATION(3, 		L"Abort in Initialization phase, Get Status returns DB_E_CANCELED")
	TEST_VARIATION(4, 		L"call GetStaus twice after Abort - E_UNEXPECTED")
	TEST_VARIATION(5, 		L"check population is uncomplete after abort called")
	TEST_VARIATION(6, 		L"Call Abort Again - E_UNEXPECTED")
	TEST_VARIATION(7, 		L"Call OpenRowset again after Abort")
	TEST_VARIATION(8, 		L"Call Abort Again - E_UNEXPECTED")
	TEST_VARIATION(9, 		L"Abort - Invalid Arguments")
	TEST_VARIATION(10, 		L"Advise Listener - after operation is aborted")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCAsynchProperties)
//*-----------------------------------------------------------------------
// @class Test all Asynch Properties
//
class TCAsynchProperties : public CTestNotify { 
public:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCAsynchProperties,CTestNotify);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember DBPROP_INIT_ASYNCH - S_OK - Optional
	int Variation_1();
	// @cmember DBPROP_INIT_ASYNCH - S_OK - Required
	int Variation_2();
	// @cmember DBPROP_INIT_ASYNCH - DB_S_ERRORSOCCURRED - Optional
	int Variation_3();
	// @cmember DBPROP_INIT_ASYNCH - DB_E_ERRORSOCCURRED - Required
	int Variation_4();
	// @cmember DBPROP_ROWSET_ASYNCH - S_OK - Optional
	int Variation_5();
	// @cmember DBPROP_ROWSET_ASYNCH - S_OK - Required
	int Variation_6();
	// @cmember DBPROP_ROWSET_ASYNCH - DB_S_ERRORSOCCURRED - Optional
	int Variation_7();
	// @cmember DBPROP_ROWSET_ASYNCH - DB_E_ERRORSOCCURRED - Required
	int Variation_8();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(TCAsynchProperties)
#define THE_CLASS TCAsynchProperties
BEG_TEST_CASE(TCAsynchProperties, CTestNotify, L"Test all Asynch Properties")
	TEST_VARIATION(1, 		L"DBPROP_INIT_ASYNCH - S_OK - Optional")
	TEST_VARIATION(2, 		L"DBPROP_INIT_ASYNCH - S_OK - Required")
	TEST_VARIATION(3, 		L"DBPROP_INIT_ASYNCH - DB_S_ERRORSOCCURRED - Optional")
	TEST_VARIATION(4, 		L"DBPROP_INIT_ASYNCH - DB_E_ERRORSOCCURRED - Required")
	TEST_VARIATION(5, 		L"DBPROP_ROWSET_ASYNCH - S_OK - Optional")
	TEST_VARIATION(6, 		L"DBPROP_ROWSET_ASYNCH - S_OK - Required")
	TEST_VARIATION(7, 		L"DBPROP_ROWSET_ASYNCH - DB_S_ERRORSOCCURRED - Optional")
	TEST_VARIATION(8, 		L"DBPROP_ROWSET_ASYNCH - DB_E_ERRORSOCCURRED - Required")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END



// }} END_DECLARE_TEST_CASES()

////////////////////////////////////////////////////////////////////////
// Defines
//
////////////////////////////////////////////////////////////////////////
#define COPY_TEST_CASE(theClass, baseClass)						\
	class theClass : public baseClass							\
	{															\
	public:														\
		static const WCHAR		m_wszTestCaseName[];			\
		DECLARE_TEST_CASE_FUNCS(theClass, baseClass);			\
	};															\
const WCHAR		theClass::m_wszTestCaseName[] = { L#theClass };	\


#define TEST_CASE_WITH_PARAM(iCase, theClass, param)			\
    case iCase:													\
		pCTestCase = new theClass(NULL);						\
		((theClass*)pCTestCase)->SetTestCaseParam(param);		\
		pCTestCase->SetOwningMod(iCase-1, pCThisTestModule);	\
		return pCTestCase;


//TCDataSource
COPY_TEST_CASE(TCDataSource_AsynchNotify,			TCAsynchNotify)
COPY_TEST_CASE(TCDataSource_AsynchNotifyFailure,	TCAsynchNotifyFailure)
COPY_TEST_CASE(TCDataSource_AsynchStatus,			TCAsynchStatus)
COPY_TEST_CASE(TCDataSource_AsynchAbort,			TCAsynchAbort)
COPY_TEST_CASE(TCDataSource_AsynchProperties,		TCAsynchProperties)

//TCRow
COPY_TEST_CASE(TCRow_AsynchNotify,					TCAsynchNotify)
COPY_TEST_CASE(TCRow_AsynchNotifyFailure,			TCAsynchNotifyFailure)
COPY_TEST_CASE(TCRow_AsynchStatus,					TCAsynchStatus)
COPY_TEST_CASE(TCRow_AsynchAbort,					TCAsynchAbort)
COPY_TEST_CASE(TCRow_AsynchProperties,				TCAsynchProperties)

//TCStream
COPY_TEST_CASE(TCStream_AsynchNotify,				TCAsynchNotify)
COPY_TEST_CASE(TCStream_AsynchNotifyFailure,		TCAsynchNotifyFailure)
COPY_TEST_CASE(TCStream_AsynchStatus,				TCAsynchStatus)
COPY_TEST_CASE(TCStream_AsynchAbort,				TCAsynchAbort)
COPY_TEST_CASE(TCStream_AsynchProperties,			TCAsynchProperties)

//TCRowset
COPY_TEST_CASE(TCRowset_AsynchNotify,				TCAsynchNotify)
COPY_TEST_CASE(TCRowset_AsynchNotifyFailure,		TCAsynchNotifyFailure)
COPY_TEST_CASE(TCRowset_AsynchStatus,				TCAsynchStatus)
COPY_TEST_CASE(TCRowset_AsynchAbort,				TCAsynchAbort)
COPY_TEST_CASE(TCRowset_AsynchProperties,			TCAsynchProperties)

//TCExecute
COPY_TEST_CASE(TCExecute_AsynchNotify,				TCAsynchNotify)
COPY_TEST_CASE(TCExecute_AsynchNotifyFailure,		TCAsynchNotifyFailure)
COPY_TEST_CASE(TCExecute_AsynchStatus,				TCAsynchStatus)
COPY_TEST_CASE(TCExecute_AsynchAbort,				TCAsynchAbort)
COPY_TEST_CASE(TCExecute_AsynchProperties,			TCAsynchProperties)

//TCSchema
COPY_TEST_CASE(TCSchema_AsynchNotify,				TCAsynchNotify)
COPY_TEST_CASE(TCSchema_AsynchNotifyFailure,		TCAsynchNotifyFailure)
COPY_TEST_CASE(TCSchema_AsynchStatus,				TCAsynchStatus)
COPY_TEST_CASE(TCSchema_AsynchAbort,				TCAsynchAbort)
COPY_TEST_CASE(TCSchema_AsynchProperties,			TCAsynchProperties)


//NOTE: The #ifdef block below is only for test wizard.  TestWizard has too many 
//strict rules in the parsing code and requires a 1:1 correspondence between
//testcases and the map.  What the #else section is doing is basically "reusing"
//existing testcases by just passing in a paraemter which changes the behvior.
//So we make LTM think there are 15 cases in here with different names, but in
//reality we only have to maintain code for the unique cases.  This way we can
//easily get testing of other storage interfaces, without maintaining 4 different
//tests with almost identical code...

#if 0 
// {{ TCW_TESTMODULE(ThisModule)
TEST_MODULE(5, ThisModule, gwszModuleDescrip)
	TEST_CASE(1, TCAsynchNotify)
	TEST_CASE(2, TCAsynchNotifyFailure)
	TEST_CASE(3, TCAsynchStatus)
	TEST_CASE(4, TCAsynchAbort)
	TEST_CASE(5, TCAsynchProperties)
END_TEST_MODULE()
// }} TCW_TESTMODULE_END
#else
TEST_MODULE(30, ThisModule, gwszModuleDescrip)
	//DataSource - IDBAsynchStatus
	TEST_CASE_WITH_PARAM(1,			TCDataSource_AsynchNotify,			DATASOURCE_INTERFACE)
	TEST_CASE_WITH_PARAM(2,			TCDataSource_AsynchNotifyFailure,	DATASOURCE_INTERFACE)
	TEST_CASE_WITH_PARAM(3,			TCDataSource_AsynchStatus,			DATASOURCE_INTERFACE)
	TEST_CASE_WITH_PARAM(4,			TCDataSource_AsynchAbort,			DATASOURCE_INTERFACE)
	TEST_CASE_WITH_PARAM(5,			TCDataSource_AsynchProperties,		DATASOURCE_INTERFACE)

	//Row - IDBAsynchStatus
	TEST_CASE_WITH_PARAM(6,			TCRow_AsynchNotify,					ROW_INTERFACE)
	TEST_CASE_WITH_PARAM(7,			TCRow_AsynchNotifyFailure,			ROW_INTERFACE)
	TEST_CASE_WITH_PARAM(8,			TCRow_AsynchStatus,					ROW_INTERFACE)
	TEST_CASE_WITH_PARAM(9,			TCRow_AsynchAbort,					ROW_INTERFACE)
	TEST_CASE_WITH_PARAM(10,		TCRow_AsynchProperties,				ROW_INTERFACE)

	//Stream - IDBAsynchStatus
	TEST_CASE_WITH_PARAM(11,		TCStream_AsynchNotify,				STREAM_INTERFACE)
	TEST_CASE_WITH_PARAM(12,		TCStream_AsynchNotifyFailure,		STREAM_INTERFACE)
	TEST_CASE_WITH_PARAM(13,		TCStream_AsynchStatus,				STREAM_INTERFACE)
	TEST_CASE_WITH_PARAM(14,		TCStream_AsynchAbort,				STREAM_INTERFACE)
	TEST_CASE_WITH_PARAM(15,		TCStream_AsynchProperties,			STREAM_INTERFACE)

	//Rowset - IDBAsynchStatus
	TEST_CASE_WITH_PARAM(16,		TCRowset_AsynchNotify,				ROWSET_INTERFACE)
	TEST_CASE_WITH_PARAM(17,		TCRowset_AsynchNotifyFailure,		ROWSET_INTERFACE)
	TEST_CASE_WITH_PARAM(18,		TCRowset_AsynchStatus,				ROWSET_INTERFACE)
	TEST_CASE_WITH_PARAM(19,		TCRowset_AsynchAbort,				ROWSET_INTERFACE)
	TEST_CASE_WITH_PARAM(20,		TCRowset_AsynchProperties,			ROWSET_INTERFACE)

	//Rowset - (Command Execute Rowset)
	TEST_CASE_WITH_PARAM(21,		TCExecute_AsynchNotify,				COMMAND_INTERFACE)
	TEST_CASE_WITH_PARAM(22,		TCExecute_AsynchNotifyFailure,		COMMAND_INTERFACE)
	TEST_CASE_WITH_PARAM(23,		TCExecute_AsynchStatus,				COMMAND_INTERFACE)
	TEST_CASE_WITH_PARAM(24,		TCExecute_AsynchAbort,				COMMAND_INTERFACE)
	TEST_CASE_WITH_PARAM(25,		TCExecute_AsynchProperties,			COMMAND_INTERFACE)

	//Rowset - (Schema Rowset)
	TEST_CASE_WITH_PARAM(26,		TCSchema_AsynchNotify,				ENUMERATOR_INTERFACE)
	TEST_CASE_WITH_PARAM(27,		TCSchema_AsynchNotifyFailure,		ENUMERATOR_INTERFACE)
	TEST_CASE_WITH_PARAM(28,		TCSchema_AsynchStatus,				ENUMERATOR_INTERFACE)
	TEST_CASE_WITH_PARAM(29,		TCSchema_AsynchAbort,				ENUMERATOR_INTERFACE)
	TEST_CASE_WITH_PARAM(30,		TCSchema_AsynchProperties,			ENUMERATOR_INTERFACE)
END_TEST_MODULE()
#endif





// {{ TCW_TC_PROTOTYPE(TCAsynchNotify)
//*-----------------------------------------------------------------------
//| Test Case:		TCAsynchNotify - Test Notifications
//| for notifications
//|	Created:		07/09/97
//|
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCAsynchNotify::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CTestNotify::Init())
	// }}
	{
		return InitSources();
	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Test how many times notified
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCAsynchNotify::Variation_1()
{
	TBEGIN
	
	// Reset DataSource and Advise Listeners
	TESTC(ResetSources());
	TESTC(AdviseAll());

	//Wait until Complete
	TESTC(CauseNotifications());
	TESTC_(WaitUntilComplete(),S_OK);
	
	// test listener was not notified for DBASYNCHPHASE_POPULATION and once for all othe phases
	TESTC(VerifyAllConnections());

CLEANUP:
	UnadviseAll();
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc verify multiple Listeners are advised
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCAsynchNotify::Variation_2()
{
	TBEGIN

	// Reset DataSource and Advise Listeners
	TESTC(ResetSources());
	TESTC(AdviseAll());

	//Wait until Complete
	TESTC(CauseNotifications());
	TESTC_(WaitUntilComplete(),S_OK);

	// test listener was not notified for DBASYNCHPHASE_POPULATION and once for all othe phases
	TESTC(VerifyAllConnections());

CLEANUP:
	UnadviseAll();
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Verify DBASYNCHPHASE_POPULATION is received
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCAsynchNotify::Variation_3()
{
	TBEGIN

	// Reset DataSource and Advise Listeners
	TESTC(ResetSources());
	TESTC(AdviseAll());

	//Wait until Complete
	TESTC(CauseNotifications());
	TESTC_(WaitUntilComplete(),S_OK);

	// test listener was not notified for DBASYNCHPHASE_POPULATION and once for all othe phases
	TESTC(VerifyAllConnections());

CLEANUP:
	UnadviseAll();
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Test DBPROPVAL_ASYNCH_SEQUENTIALPOPULATION
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCAsynchNotify::Variation_4()
{
	//TBEGIN

	// Reset DataSource and Advise Listeners
//TODO
//	TESTC_(pCSource(0)->Initialize(DBPROPVAL_ASYNCH_INITIALIZE | DBPROPVAL_ASYNCH_SEQUENTIALPOPULATION),DB_E_ERRORSOCCURRED);

//CLEANUP:
	//WaitUntilComplete();
	//TRETURN
	return TEST_SKIPPED;

}

// }}




// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Test DBPROPVAL_ASYNCH_RANDOMPOPULATION
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAsynchNotify::Variation_5()
{ 
	//TBEGIN

	// Reset DataSource and Advise Listeners
//TODO
//	TESTC_(pCSource(0)->Initialize(DBPROPVAL_ASYNCH_INITIALIZE | DBPROPVAL_ASYNCH_RANDOMPOPULATION),DB_E_ERRORSOCCURRED);

//CLEANUP:
	//WaitUntilComplete();
	//TRETURN
	return TEST_SKIPPED;

} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Test SEQUENTIAL | RANDOM
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAsynchNotify::Variation_6()
{ 
	//TBEGIN

	// Reset DataSource and Advise Listeners
//TODO
//	TESTC_(pCSource(0)->Initialize(DBPROPVAL_ASYNCH_INITIALIZE | DBPROPVAL_ASYNCH_SEQUENTIALPOPULATION | DBPROPVAL_ASYNCH_RANDOMPOPULATION),DB_E_ERRORSOCCURRED);

//CLEANUP:
	//WaitUntilComplete();
	//TRETURN
	return TEST_SKIPPED;

} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc L1 -OnProgress DB_S_UNWANTEDOPERATION L2, L3 receive all phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCAsynchNotify::Variation_7()
{
	TBEGIN

	// L1 - return DB_S_UNWANTEDOPERATION
	TESTC(pCListener(0)->SetEvent(DBREASON_ONPROGRESS,DBEVENTPHASE_ALL,FALSE));

	// Reset DataSource and Advise Listeners
	TESTC(ResetSources());
	TESTC(AdviseAll());

	//Wait until Complete
	TESTC(CauseNotifications());
	TESTC_(WaitUntilComplete(),S_OK);
	
	// testL1 L2, L3 listener was  notified   once for OnStop
	TESTC(VerifyEventAll(DBREASON_ONSTOP));

	// test L2, L3 listener was  notified   with OnProgress
	TESTC(VerifyEvent(pCListener(1), DBREASON_ONPROGRESS));
	TESTC(VerifyEvent(pCListener(2), DBREASON_ONPROGRESS));
	
	//Wait until Complete
	TESTC(CauseNotifications());
	TESTC_(WaitUntilComplete(),S_OK);
	
	// test L2, L3 listener was  notified   with OnProgress
	TESTC(VerifyUnwantedEvent(pCListener(0), DBREASON_ONPROGRESS));
	TESTC(VerifyEvent(pCListener(1), DBREASON_ONPROGRESS));
	TESTC(VerifyEvent(pCListener(2), DBREASON_ONPROGRESS));

CLEANUP:
	pCListener(0)->SetEvent(DBREASON_ONPROGRESS,DBEVENTPHASE_ALL,TRUE);
	UnadviseAll();
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc L1 -OnStop DB_S_UNWANTEDOPERATION L2, L3 receive all phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCAsynchNotify::Variation_8()
{
	TBEGIN

	// L1 - return DB_S_UNWANTEDOPERATION
	TESTC(pCListener(0)->SetEvent(DBREASON_ONSTOP, DBEVENTPHASE_ALL, FALSE));
		
	// Reset DataSource and Advise Listeners
	TESTC(ResetSources());
	TESTC(AdviseAll());

	//Wait until Complete
	TESTC(CauseNotifications());
	TESTC_(WaitUntilComplete(),S_OK);
	
	// testL1 L2, L3 listener was  notified   once for OnStop
	TESTC(VerifyEventAll(DBREASON_ONSTOP));
	TESTC(VerifyEventAll(DBREASON_ONPROGRESS));

	//Cause another set of Notifications, so ONSTOP has another chance to fire
	TESTC(CauseNotifications());
	TESTC_(WaitUntilComplete(),S_OK);

	TESTC(VerifyUnwantedEvent(pCListener(0), DBREASON_ONSTOP));
	TESTC(VerifyEvent(pCListener(1), DBREASON_ONSTOP));
	TESTC(VerifyEvent(pCListener(2), DBREASON_ONSTOP));

	// testL1 L2, L3 listener was  notified   with OnProgress
	TESTC(VerifyEventAll(DBREASON_ONPROGRESS));
	
CLEANUP:
	pCListener(0)->SetEvent(DBREASON_ONSTOP,DBEVENTPHASE_ALL,TRUE);
	UnadviseAll();
	TRETURN
}
// }}
// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc L1 -OnProgress DB_S_UNWANTEDPHASE L2, L3 receive all phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCAsynchNotify::Variation_9()
{
	TBEGIN

	// L1 - return DB_S_UNWANTEDOPERATION
	TESTC(pCListener(0)->SetEvent(DBREASON_ONPROGRESS, DBASYNCHPHASE_INITIALIZATION,FALSE));
		
	// Reset DataSource and Advise Listeners
	TESTC(ResetSources());
	TESTC(AdviseAll());

	//Wait until Complete
	TESTC(CauseNotifications());
	TESTC_(WaitUntilComplete(),S_OK);
	
	// testL1 L2, L3 listener was  notified   once for OnStop
	TESTC(VerifyEventAll(DBREASON_ONSTOP));

	// testL1 L2, L3 listener was  notified   with OnProgress
	TESTC(VerifyEvent(pCListener(1), DBREASON_ONPROGRESS));
	TESTC(VerifyEvent(pCListener(2), DBREASON_ONPROGRESS));

	//Wait until Complete
	TESTC(CauseNotifications());
	TESTC_(WaitUntilComplete(),S_OK);
	
	// testL1 L2, L3 listener was  notified   with OnProgress
	TESTC(VerifyUnwantedEvent(pCListener(0), DBREASON_ONPROGRESS, DBASYNCHPHASE_INITIALIZATION));
	TESTC(VerifyEvent(pCListener(1), DBREASON_ONPROGRESS));
	TESTC(VerifyEvent(pCListener(2), DBREASON_ONPROGRESS));

CLEANUP:
	pCListener(0)->SetEvent(DBREASON_ONPROGRESS,DBASYNCHPHASE_INITIALIZATION,TRUE);
	UnadviseAll();
	TRETURN
}

// }}
// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc L1 -OnProgress DB_S_UNWANTEDPHASE L2, L3 receive all phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCAsynchNotify::Variation_10()
{
	TBEGIN

	// L1 - return DB_S_UNWANTEDOPERATION
	TESTC(pCListener(0)->SetEvent(DBREASON_ONPROGRESS,DBASYNCHPHASE_POPULATION,FALSE));
		
	// Reset DataSource and Advise Listeners
	TESTC(ResetSources());
	TESTC(AdviseAll());

	//Wait until Complete
	TESTC(CauseNotifications());
	TESTC_(WaitUntilComplete(),S_OK);

	// testL1 L2, L3 listener was  notified   once for OnStop
	TESTC(VerifyEventAll(DBREASON_ONSTOP));

	// testL1 L2, L3 listener was  notified   with OnProgress
	TESTC(VerifyEvent(pCListener(1), DBREASON_ONPROGRESS));
	TESTC(VerifyEvent(pCListener(2), DBREASON_ONPROGRESS));

	//Wait until Complete
	TESTC(CauseNotifications());
	TESTC_(WaitUntilComplete(),S_OK);
	
	// testL1 L2, L3 listener was  notified   with OnProgress
	TESTC(VerifyUnwantedEvent(pCListener(0), DBREASON_ONPROGRESS, DBASYNCHPHASE_POPULATION));
	TESTC(VerifyEvent(pCListener(1), DBREASON_ONPROGRESS));
	TESTC(VerifyEvent(pCListener(2), DBREASON_ONPROGRESS));

CLEANUP:
	pCListener(0)->SetEvent(DBREASON_ONPROGRESS, DBASYNCHPHASE_POPULATION,TRUE);
	UnadviseAll();
	TRETURN

}
// }}
// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc L1 -OnProgress DB_S_UNWANTEDPHASE L2, L3 receive all phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCAsynchNotify::Variation_11()
{
	TBEGIN

	// L1 - return DB_S_UNWANTEDOPERATION
	TESTC(pCListener(0)->SetEvent(DBREASON_ONPROGRESS, DBASYNCHPHASE_COMPLETE, FALSE));
		
	// Reset DataSource and Advise Listeners
	TESTC(ResetSources());
	TESTC(AdviseAll());

	//Wait until Complete
	TESTC(CauseNotifications());
	TESTC_(WaitUntilComplete(),S_OK);

	// testL1 L2, L3 listener was  notified   once for OnStop
	TESTC(VerifyEventAll(DBREASON_ONSTOP));

	// testL1 L2, L3 listener was  notified   with OnProgress
	TESTC(VerifyEvent(pCListener(1), DBREASON_ONPROGRESS));
	TESTC(VerifyEvent(pCListener(2), DBREASON_ONPROGRESS));

	//Wait until Complete
	TESTC(CauseNotifications());
	TESTC_(WaitUntilComplete(),S_OK);

	// testL1 L2, L3 listener was  notified   with OnProgress
	TESTC(VerifyUnwantedEvent(pCListener(0), DBREASON_ONPROGRESS, DBASYNCHPHASE_COMPLETE));
	TESTC(VerifyEvent(pCListener(1), DBREASON_ONPROGRESS));
	TESTC(VerifyEvent(pCListener(2), DBREASON_ONPROGRESS));

CLEANUP:
	pCListener(0)->SetEvent(DBREASON_ONPROGRESS,DBASYNCHPHASE_COMPLETE,TRUE);
	UnadviseAll();
	TRETURN

}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc OnLowResource - Veto
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAsynchNotify::Variation_12()
{ 
	TBEGIN

	// L1 - OnLowResource - return S_FALSE
	TESTC(pCListener(0)->SetCancel(DBREASON_ONLOWRESOURCE, DBASYNCHPHASE_COMPLETE));
	
	// Reset DataSource and Advise Listeners
	TESTC(ResetSources());
	TESTC(AdviseAll());

	//Wait until Complete
	TESTC(CauseNotifications());
	TESTC_(WaitUntilComplete(),S_OK);

	//Should never be notified of OnLowResource()
	TESTC(pCListener(0)->GetTimesNotified(DBREASON_ONLOWRESOURCE, DBASYNCHPHASE_COMPLETE) == 0);

CLEANUP:
	pCListener(0)->SetCancel(-1, -1);
	UnadviseAll();
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc OnStop - Veto
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAsynchNotify::Variation_13()
{ 
	TBEGIN

	// L1 - OnStop - return S_FALSE
	TESTC(pCListener(0)->SetCancel(DBREASON_ONSTOP, DBASYNCHPHASE_COMPLETE));
	
	// Reset DataSource and Advise Listeners
	TESTC(ResetSources());
	TESTC(AdviseAll());

	//Wait until Complete
	TESTC(CauseNotifications());
	TESTC_(WaitUntilComplete(),S_OK);

	//S_FALSE from OnStop should not affect anything - not vetoable notification
	TESTC(VerifyAllConnections());

CLEANUP:
	pCListener(0)->SetCancel(-1, -1);
	UnadviseAll();
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc OnProgress - Veto CANCELED
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAsynchNotify::Variation_14()
{ 
	TBEGIN
	HRESULT hr = S_OK;

	// L1 - OnProgress - return S_FALSE
	TESTC(pCListener(0)->SetCancel(DBREASON_ONPROGRESS, DBASYNCHPHASE_CANCELED));
	
	// Reset DataSource and Advise Listeners
	TESTC(ResetSources());
	TESTC(AdviseAll());

	//Wait until Complete
	TESTC(CauseNotifications());
	
	//Try calling IDBAsyncStatus::Abort() which will fire OnProgress(CANCELED)
	hr = pCSource(0)->Abort();
	TEST3C_(hr, S_OK, DB_E_CANCELED, DB_E_CANTCANCEL);
	
	if(hr==DB_E_CANTCANCEL || !pCSource(0)->IsAsynch())
	{
		TWARNING("Unable to Abort operation, not Asynch or CANTCANCEL?");
		//Wait until complete
		TESTC_(WaitUntilComplete(), S_OK);
		TESTC(VerifyAllConnections());
	}
	else
	{
		//Make sure the listener was notified of the cancel
		TESTC(pCListener(0)->GetTimesNotified(DBREASON_ONPROGRESS, DBASYNCHPHASE_CANCELED) == 1);

		//Wait until complete
		TESTC_(WaitUntilComplete(), S_OK);
		TESTC(VerifyAllConnections(hr==S_OK ? OP_ABORTED : OP_ABORTED|OP_COMPLETED));
	}

CLEANUP:
	pCListener(0)->SetCancel(-1, -1);
	UnadviseAll();
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc OnProgress - Veto INITIALIZATION
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAsynchNotify::Variation_15()
{ 
	TBEGIN
	HRESULT hr = S_OK;

	// L1 - OnProgress - return S_FALSE
	TESTC(pCListener(0)->SetCancel(DBREASON_ONPROGRESS, DBASYNCHPHASE_INITIALIZATION));
	
	// Reset DataSource and Advise Listeners
	TESTC(ResetSources());
	TESTC(AdviseAll());

	//Wait until Complete
	TESTC(CauseNotifications());
	
	//Try to veto OnProgress(INITIALIZATION)
	hr = pCSource(0)->GetStatus();
	TESTC(hr==S_OK || hr==DB_E_CANCELED)

	if(hr==DB_E_CANCELED)
	{
		//Verify Listeners were given the chance to Veto
		TESTC(pCListener(0)->GetTimesNotified(DBREASON_ONPROGRESS, DBASYNCHPHASE_INITIALIZATION)>=1);
		TESTC(pCListener(0)->GetTimesNotified(DBREASON_ONPROGRESS, DBASYNCHPHASE_CANCELED)>=1);

		//Should have all normal notifications and status
		TESTC_(WaitUntilComplete(), S_OK);
		TESTC(VerifyAllConnections(OP_ABORTED));
	}
	else
	{
		//Otherwise Listeners were not given a chance to Veto
		TESTC(pCListener(0)->GetTimesNotified(DBREASON_ONPROGRESS, DBASYNCHPHASE_INITIALIZATION)==0);
		TESTC(pCListener(0)->GetTimesNotified(DBREASON_ONPROGRESS, DBASYNCHPHASE_CANCELED)==0);
		TWARNING("Unable to veto DBASYNCHPHASE_INITIALIZATION?");

		//Should have all normal notifications and status
		TESTC_(WaitUntilComplete(), S_OK);
		TESTC(VerifyAllConnections());
	}

CLEANUP:
	pCListener(0)->SetCancel(-1, -1);
	UnadviseAll();
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc OnProgress - Veto POPULATION
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAsynchNotify::Variation_16()
{ 
	TBEGIN
	HRESULT hr = S_OK;

	// L1 - OnProgress - return S_FALSE
	TESTC(pCListener(0)->SetCancel(DBREASON_ONPROGRESS, DBASYNCHPHASE_POPULATION));
	
	// Reset DataSource and Advise Listeners
	TESTC(ResetSources());
	TESTC(AdviseAll());

	//Wait until Complete
	TESTC(CauseNotifications());
	
	//Try to veto OnProgress(POPULATION)
	hr = pCSource(0)->GetStatus();
	TESTC(hr==S_OK || hr==DB_E_CANCELED)

	if(hr==DB_E_CANCELED)
	{
		//Verify Listeners were given the chance to Veto
		TESTC(pCListener(0)->GetTimesNotified(DBREASON_ONPROGRESS, DBASYNCHPHASE_POPULATION)>=1);
		TESTC(pCListener(0)->GetTimesNotified(DBREASON_ONPROGRESS, DBASYNCHPHASE_CANCELED)>=1);

		//Should have all normal notifications and status
		TESTC_(WaitUntilComplete(), S_OK);
		TESTC(VerifyAllConnections(OP_ABORTED));
	}
	else
	{
		//Otherwise Listeners were not given a chance to Veto
		TESTC(pCListener(0)->GetTimesNotified(DBREASON_ONPROGRESS, DBASYNCHPHASE_POPULATION)==0);
		TESTC(pCListener(0)->GetTimesNotified(DBREASON_ONPROGRESS, DBASYNCHPHASE_CANCELED)==0);
		TWARNING("Unable to veto DBASYNCHPHASE_POPULATION?");

		//Should have all normal notifications and status
		TESTC_(WaitUntilComplete(), S_OK);
		TESTC(VerifyAllConnections());
	}


CLEANUP:
	pCListener(0)->SetCancel(-1, -1);
	UnadviseAll();
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc OnProgress - Veto COMPLETE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAsynchNotify::Variation_17()
{ 
	TBEGIN

	// L1 - OnProgress - return S_FALSE
	TESTC(pCListener(0)->SetCancel(DBREASON_ONPROGRESS, DBASYNCHPHASE_COMPLETE));
	
	// Reset DataSource and Advise Listeners
	TESTC(ResetSources());
	TESTC(AdviseAll());

	//Wait until Complete
	TESTC(CauseNotifications());
	TESTC_(WaitUntilComplete(),S_OK);

	//S_FALSE from OnProgress(COMPLETE) should not affect anything - not vetoable notification
	TESTC(VerifyAllConnections());

CLEANUP:
	pCListener(0)->SetCancel(-1, -1);
	UnadviseAll();
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END






// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc Advise Listener - after operation is complete
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAsynchNotify::Variation_18()
{ 
	TBEGIN
	ULONG iSource = 0;

	//Reset DataSource and Advise Listeners
	TESTC(ResetSources());
	TESTC(CauseNotifications());

	//Wait until the operation is complete...
	TESTC_(WaitUntilComplete(),S_OK);
	
	//Reset all listeners.  This is becuase we want to make sure none of the other 
	//listeners that are already advised receive any notifications
	TESTC(ResetListeners());
	UnadviseAll();

	//Now advise listeners, after the fact.
	//Spec requires that all newly advised listeners that "missed" the operation
	//be notified that the operation already completed.  This is becuase at least for the rowset
	//you may not be able to get the listeners advised quickly enough before the operation is complete.
	//As oppsoed to the datasource where you can advise listeners first and then when everything
	//is all hooked up then Initialize can be called...
	for(iSource=0; iSource<m_vectSources.GetCount(); iSource++)
	{
		//Advise this listener to this source...
		TESTC(Advise(pCListener(0), pCSource(iSource)));

		//Make sure it was notified of OnProgress(COMPLETE) and OnStop().
		//Make sure no other Listeners were notified
		TESTC(VerifyAllConnections(OP_COMPLETED));
	}

CLEANUP:
	UnadviseAll();
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCAsynchNotify::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CTestNotify::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCAsynchNotifyFailure)
//*-----------------------------------------------------------------------
//| Test Case:		TCAsynchNotifyFailure - Test Notification Error cases
//|	Created:			07/10/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCAsynchNotifyFailure::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CTestNotify::Init())
	// }}
	{
		return InitSources();
	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc E_FAIL - L1 , L1, L2 , L3 receive all phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCAsynchNotifyFailure::Variation_1()
{
	TBEGIN

	// L1 - return DB_S_UNWANTEDOPERATION
	TESTC(pCListener(0)->SetError(E_FAIL))
		
	// Reset DataSource and Advise Listeners
	TESTC(ResetSources());
	TESTC(AdviseAll());

	//Wait until Complete
	TESTC(CauseNotifications());
	TESTC_(WaitUntilComplete(),S_OK);

	// test L2, L3 listener was  notified   once for OnStop
	TESTC(VerifyAllConnections());

CLEANUP:
	pCListener(0)->SetError(S_OK);
	UnadviseAll();
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc E_FAIL - L1 L2 , L1, L2 , L3 receive all phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCAsynchNotifyFailure::Variation_2()
{
	TBEGIN

	// L1 - return DB_S_UNWANTEDOPERATION
	TESTC(pCListener(0)->SetError(E_FAIL))
	TESTC(pCListener(1)->SetError(E_FAIL))
		
	// Reset DataSource and Advise Listeners
	TESTC(ResetSources());
	TESTC(AdviseAll());

	//Wait until Complete
	TESTC(CauseNotifications());
	TESTC_(WaitUntilComplete(),S_OK);

	// test L2, L3 listener was  notified   once for OnStop
	TESTC(VerifyAllConnections());

CLEANUP:
	pCListener(0)->SetError(S_OK);
	pCListener(1)->SetError(S_OK);
	UnadviseAll();
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc E_FAIL - L1 L2 , L1, L2 , L3 receive all phases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCAsynchNotifyFailure::Variation_3()
{
	
	TBEGIN

	// L1 - return DB_S_UNWANTEDOPERATION
	TESTC(pCListener(0)->SetError(E_FAIL))

	// Reset DataSource and Advise Listeners
	TESTC(ResetSources());
	TESTC(AdviseAll());

	//Wait until Complete
	TESTC(CauseNotifications());
	TESTC_(WaitUntilComplete(),S_OK);

	// test L2, L3 listener was  notified   once for OnStop
	TESTC(VerifyAllConnections());

CLEANUP:
	pCListener(0)->SetError(S_OK);
	pCListener(1)->SetError(S_OK);
	UnadviseAll();
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc E_FAIL - IRowsetLocate
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCAsynchNotifyFailure::Variation_4()
{
	
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc E_FAIL - IRowsetResynch
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCAsynchNotifyFailure::Variation_5()
{
	
	return TEST_SKIPPED;
	
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc E_FAIL - IRowsetScroll
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCAsynchNotifyFailure::Variation_6()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCAsynchNotifyFailure::Terminate()
{
	return(CTestNotify::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCAsynchStatus)
//*-----------------------------------------------------------------------
//| Test Case:		TCAsynchStatus - Test Notification Error cases
//| for notifications
//|	Created:		07/09/97
//|
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCAsynchStatus::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CTestNotify::Init())
	// }}
	{
		return InitSources();
	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Check the HRESULT from GetStatus
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCAsynchStatus::Variation_1()
{
	TBEGIN

	//Reset DataSource and Advise Listeners
	TESTC(ResetSources());
	TESTC(CauseNotifications());
	TESTC_(WaitUntilComplete(),S_OK);
	
CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Verify ProgressMax == ProgressCurr
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCAsynchStatus::Variation_2()
{
	TBEGIN

	//Reset DataSource and Advise Listeners
	TESTC(ResetSources());
	TESTC(CauseNotifications());
	TESTC_(WaitUntilComplete(),S_OK);
	
	//test that progressmaxCurr == progresscurr
	TESTC(pCSource(0)->GetProgress() == pCSource(0)->GetProgressMax());
	
CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc check ulAsynchPhase is DBASYNCHPHASE_COMPLETE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCAsynchStatus::Variation_3()
{
	TBEGIN

	//Reset DataSource and Advise Listeners
	TESTC(ResetSources());
	TESTC(CauseNotifications());
	TESTC_(WaitUntilComplete(),S_OK);
	
	//test that progressmaxCurr == progresscurr
	TESTC(pCSource(0)->GetProgress() == pCSource(0)->GetProgressMax());
	TESTC(pCSource(0)->GetAsynchPhase() == DBASYNCHPHASE_COMPLETE);

CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Verify ratio corresponds to returned staus code
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCAsynchStatus::Variation_4()
{
	TBEGIN

	//Reset DataSource and Advise Listeners
	TESTC(ResetSources());
	TESTC(CauseNotifications());
	TESTC_(pCSource(0)->GetStatus(),S_OK);
	
	if( pCSource(0)->GetAsynchPhase() == DBASYNCHPHASE_INITIALIZATION ||
		pCSource(0)->GetAsynchPhase() == DBASYNCHPHASE_POPULATION)
	{
		TESTC(pCSource(0)->GetProgress() < pCSource(0)->GetProgressMax());
	}
	else 
	{
		TESTC(pCSource(0)->GetProgress() == pCSource(0)->GetProgressMax());
	}

	
CLEANUP:
	WaitUntilComplete();
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Verify ratio of ProgressCurr and ProgressMax
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCAsynchStatus::Variation_5()
{
	TBEGIN

	//Reset DataSource and Advise Listeners
	TESTC(ResetSources());
	TESTC(CauseNotifications());
	TESTC_(pCSource(0)->GetStatus(),S_OK);
	
	if( pCSource(0)->GetAsynchPhase() == DBASYNCHPHASE_INITIALIZATION ||
		pCSource(0)->GetAsynchPhase() == DBASYNCHPHASE_POPULATION)
	{
			TESTC(pCSource(0)->GetPrevProgress() <= pCSource(0)->GetProgress());
			TESTC(pCSource(0)->GetPrevProgressMax() <= pCSource(0)->GetProgressMax());
	}
	else 
	{
		TESTC(pCSource(0)->GetProgress() == pCSource(0)->GetProgressMax());
	}

	
CLEANUP:
	WaitUntilComplete();
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Poll for status continuously
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCAsynchStatus::Variation_6()
{
	TBEGIN

	//Reset DataSource and Advise Listeners
	TESTC(ResetSources());
	TESTC(CauseNotifications());

	// poll for status
	while ( !pCSource(0)->IsComplete())
	{
		if( pCSource(0)->GetAsynchPhase() == DBASYNCHPHASE_INITIALIZATION ||
			pCSource(0)->GetAsynchPhase() == DBASYNCHPHASE_POPULATION)
		{
			TESTC(pCSource(0)->GetPrevProgress() <= pCSource(0)->GetProgress());
			TESTC(pCSource(0)->GetPrevProgressMax() <= pCSource(0)->GetProgressMax());
		}
		else 
		{
			TESTC(pCSource(0)->GetProgress() == pCSource(0)->GetProgressMax());
		}
	}

CLEANUP:
	TRETURN
}
// }}
// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCAsynchStatus::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CTestNotify::Terminate());
}	// }}
// }}
// }}

// {{ TCW_TC_PROTOTYPE(TCAsynchAbort)
//*-----------------------------------------------------------------------
//| Test Case:		TCAsynchAbort - Test Notification Error cases
//| for notifications
//|	Created:		07/09/97
//|
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCAsynchAbort::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CTestNotify::Init())
	// }}
	{
		return InitSources();
	}
	return FALSE;
}

// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Call abort after population completed, verify DB_E_CANTCANCEL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCAsynchAbort::Variation_1()
{
	TBEGIN

	//Reset DataSource and Advise Listeners
	TESTC(ResetSources());
	TESTC(CauseNotifications());
	TESTC_(WaitUntilComplete(),S_OK);
	
	TESTC_(pCSource(0)->Abort(), S_OK);
	
CLEANUP:
	TRETURN

}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc GetStatus after Abort returns DB_E_CANTCANCEL returns S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCAsynchAbort::Variation_2()
{
	TBEGIN

	//Reset DataSource and Advise Listeners
	TESTC(ResetSources());
	TESTC(CauseNotifications());
	TESTC_(WaitUntilComplete(),S_OK);
	
	TESTC_(pCSource(0)->Abort(), S_OK);
	
	// call getstaus returns S_OK
	TESTC_(pCSource(0)->GetStatus(), S_OK);	
	TESTC(pCSource(0)->GetProgress() == pCSource(0)->GetProgressMax());
	
CLEANUP:
	TRETURN
}
// }}
// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Abort in Initialization phase, Get Status returns DB_E_CANCELED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCAsynchAbort::Variation_3()
{
	TBEGIN
	TESTC(InitSources());

	//Reset DataSource and Advise Listeners
	TESTC(ResetSources());
	TESTC(CauseNotifications());
	TESTC_(pCSource(0)->GetStatus(),S_OK);
	
	if( pCSource(0)->GetAsynchPhase() == DBASYNCHPHASE_INITIALIZATION ||
		pCSource(0)->GetAsynchPhase() == DBASYNCHPHASE_POPULATION)
	{
		TESTC(pCSource(0)->VerifyAbort());
	}

	
CLEANUP:
	WaitUntilComplete();
	TRETURN
}

// }}
// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc call GetStaus twice after Abort - E_UNEXPECTED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCAsynchAbort::Variation_4()
{
	TBEGIN
	TESTC(InitSources());

	//Reset DataSource and Advise Listeners
	TESTC(ResetSources());
	TESTC(CauseNotifications());
	TESTC_(pCSource(0)->GetStatus(),S_OK);
	
	if( pCSource(0)->GetAsynchPhase() == DBASYNCHPHASE_INITIALIZATION ||
		pCSource(0)->GetAsynchPhase() == DBASYNCHPHASE_POPULATION)
	{
		//Abort
		TESTC(pCSource(0)->VerifyAbort());
	}
	
CLEANUP:
	WaitUntilComplete();
	TRETURN
}
// }}
// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc check population is uncomplete after abort called
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCAsynchAbort::Variation_5()
{
	TBEGIN
	TESTC(InitSources());

	//Reset DataSource and Advise Listeners
	TESTC(ResetSources());
	TESTC(CauseNotifications());
	TESTC_(pCSource(0)->GetStatus(),S_OK);
	
	if( pCSource(0)->GetAsynchPhase() == DBASYNCHPHASE_INITIALIZATION ||
		pCSource(0)->GetAsynchPhase() == DBASYNCHPHASE_POPULATION)
	{
		TESTC(pCSource(0)->VerifyAbort());

		// call getstaus again 
		TESTC(pCSource(0)->GetProgress() < pCSource(0)->GetProgressMax());
	}
	
CLEANUP:
	WaitUntilComplete();
	TRETURN
}
// }}
// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Call Abort Again - E_UNEXPECTED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCAsynchAbort::Variation_6()
{
	TBEGIN
	TESTC(InitSources());

	//Reset DataSource and Advise Listeners
	TESTC(ResetSources());
	TESTC(CauseNotifications());
	TESTC_(pCSource(0)->GetStatus(),S_OK);
	
	if( pCSource(0)->GetAsynchPhase() == DBASYNCHPHASE_INITIALIZATION ||
		pCSource(0)->GetAsynchPhase() == DBASYNCHPHASE_POPULATION)
	{
		TESTC(pCSource(0)->VerifyAbort());
	}
	
CLEANUP:
	WaitUntilComplete();
	TRETURN
}
// }}
// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Call OpenRowset again after Abort
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCAsynchAbort::Variation_7()
{
	TBEGIN
	TESTC(InitSources());

	//Reset DataSource and Advise Listeners
	TESTC(ResetSources());
	TESTC(CauseNotifications());
	TESTC_(pCSource(0)->GetStatus(),S_OK);
	
	if( pCSource(0)->GetAsynchPhase() == DBASYNCHPHASE_INITIALIZATION ||
		pCSource(0)->GetAsynchPhase() == DBASYNCHPHASE_POPULATION)
	{
		TESTC(pCSource(0)->VerifyAbort());
	}

	//Reset DataSource and Advise Listeners
	TESTC(ResetSources());
	TESTC_(pCSource(0)->GetStatus(),S_OK);
	
CLEANUP:
	WaitUntilComplete();
	TRETURN
}
// }}
// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc Call Abort Again - E_UNEXPECTED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCAsynchAbort::Variation_8()
{
	return TEST_SKIPPED;	
}




// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc Abort - Invalid Arguments
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAsynchAbort::Variation_9()
{ 
	TBEGIN
	TESTC(InitSources());

	//Reset DataSource and Advise Listeners
	TESTC(ResetSources());
	TESTC(CauseNotifications());

	//GetStatus - InvalidArg (acording to spec this is S_OK)
	TESTC_(pCSource(0)->GetStatus(INVALID(ULONG)),S_OK);
	
	//Abort - InvalidArg - (according to spec this is S_OK)
	TESTC_(pCSource(0)->Abort(INVALID(ULONG)),S_OK);
	
CLEANUP:
	WaitUntilComplete();
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc Advise Listener - after operation is aborted
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAsynchAbort::Variation_10()
{ 
	TBEGIN
	ULONG iSource = 0;
	HRESULT hr = S_OK;
	ULONG iListener = 0;
	//Reset DataSource and Advise Listeners
	TESTC(ResetSources());
	TESTC(CauseNotifications());

	//Try aborting the operation
	hr = pCSource(0)->Abort();
	TEST3C_(hr, S_OK, DB_E_CANCELED, DB_E_CANTCANCEL);
	
	//Wait until the operation is complete...
	WaitUntilComplete();
	UnadviseAll();

	//Now advise listeners, after the fact.
	//Spec requires that all newly advised listeners that "missed" the operation
	//be notified that the operation already completed.  This is becuase at least for the rowset
	//you may not be able to get the listeners advised quickly enough before the operation is complete.
	//As oppsoed to the datasource where you can advise listeners first and then when everything
	//is all hooked up then Initialize can be called...
	for(iSource=0; iSource<m_vectSources.GetCount(); iSource++)
	{
		BOOL fAborted = (iSource==0 && hr==S_OK && pCSource(0)->IsAsynch());
		
		//Reset all listeners.  This is becuase we want to make sure none of the other 
		//listeners that are already advised receive any notifications
		TESTC(ResetListeners());

		//Advise this listener to this source...
		TESTC(Advise(pCListener(0), pCSource(iSource)));

		//Make sure it was notified of OnProgress(CANCELED) and OnStop().
		//Make sure no other Listeners were notified
		for(iListener=0; iListener<m_vectListeners.GetCount(); iListener++)
		{
			TESTC(VerifyConnection(pCListener(iListener), pCSource(iSource), fAborted ? OP_ABORTED : OP_COMPLETED, fAborted ? ULONG_MAX : 0));
		}
	}

CLEANUP:
	UnadviseAll();
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCAsynchAbort::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CTestNotify::Terminate());
}	// }}
// }}
// }}





// {{ TCW_TC_PROTOTYPE(TCAsynchProperties)
//*-----------------------------------------------------------------------
//| Test Case:		TCAsynchProperties - Test all Asynch Properties
//| Created:  	5/6/98
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCAsynchProperties::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CTestNotify::Init())
	// }}
	{
		return InitSources();
	}
	return FALSE;
} 




// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc DBPROP_INIT_ASYNCH - S_OK - Optional
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAsynchProperties::Variation_1()
{ 
	TBEGIN
	CDataSource DataSourceA;
	IDBAsynchStatus *pIDBAsynchStatus = NULL;
	ULONG ulAsynchPhase = 0;
	ULONG_PTR ulAsynch = 0;

	//Variation only applies for datasource
	if (pCSource(0)->GetObjectType() != DATASOURCE_INTERFACE)
	{
		TOUTPUT_LINE(L"The variation runs only for DataSource");
		return TEST_SKIPPED;
	};
	
	DataSourceA.CreateInstance();

	//Set the property
    DataSourceA.SetProperty(DBPROP_INIT_ASYNCH,	DBPROPSET_DBINIT, (void*)DBPROPVAL_ASYNCH_INITIALIZE, DBTYPE_I4, DBPROPOPTIONS_OPTIONAL, DB_NULLID); 

    //Property errors - DB_S_ERRORSOCCURRED
    QI(DataSourceA.pIDBInit(), IID_IDBAsynchStatus, (void**)&pIDBAsynchStatus);
	switch (DataSourceA.Initialize())
	{
		case DB_S_ERRORSOCCURRED:
			TESTC(!SupportedProperty(DBPROP_INIT_ASYNCH, DBPROPSET_DBINIT) || !SettableProperty(DBPROP_INIT_ASYNCH,	DBPROPSET_DBINIT));
			break;
		case DB_S_ASYNCHRONOUS:
			TESTC(pIDBAsynchStatus!=NULL);
			while(TRUE)
			{
				//IDBAsynchStatus::GetStatus
				TESTC_(pIDBAsynchStatus->GetStatus(DB_NULL_HCHAPTER, DBASYNCHOP_OPEN, NULL, NULL, &ulAsynchPhase, NULL), S_OK);
				
				//Check for an error in the provider
				TESTC(DBASYNCHPHASE_INITIALIZATION==ulAsynchPhase || DBASYNCHPHASE_COMPLETE==ulAsynchPhase)

				//As soon as we have S_OK and COMPLETE we are done...
				if(ulAsynchPhase==DBASYNCHPHASE_COMPLETE)
					break;

				//Sleep a little, so we don't have such a tight loop...
				Sleep(100);
			}
			TESTC(GetProperty(DBPROP_INIT_ASYNCH, DBPROPSET_DBINIT, pIDBAsynchStatus, &ulAsynch));
			TESTC(DBPROPVAL_ASYNCH_INITIALIZE==ulAsynch);
			break;
		case S_OK:
			//TO DO: may be should compare ulAsynch with 0?
			TWARNING("IDBInitialize::Initialize returned S_OK, is the DataSource created Immediatly?");
			TESTC(GetProperty(DBPROP_INIT_ASYNCH, DBPROPSET_DBINIT, pIDBAsynchStatus, &ulAsynch));
			TESTC(DBPROPVAL_ASYNCH_INITIALIZE==ulAsynch);
			break;

	}

CLEANUP:
	SAFE_RELEASE(pIDBAsynchStatus);
	DataSourceA.Uninitialize();
	TRETURN
}
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc DBPROP_INIT_ASYNCH - S_OK - Required
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAsynchProperties::Variation_2()
{ 
	TBEGIN
	ULONG_PTR ulAsynch = 0;
	
	//Variation only applies for datasource
	if (pCSource(0)->GetObjectType() != DATASOURCE_INTERFACE)
	{
		TOUTPUT_LINE(L"The variation runs only for DataSource");
		return TEST_SKIPPED;
	};

	// Reset DataSource and Advise Listeners
	TESTC(ResetSources());
	TESTC(CauseNotifications());

	//Wait until Complete
	TESTC_(WaitUntilComplete(),S_OK);

	//DBPROP_ROWSET_ASYNCH
	TESTC(GetProperty(DBPROP_INIT_ASYNCH, DBPROPSET_DBINIT, pCSource(0)->pIDBAsynchStatus(), &ulAsynch));
	
	if(pCSource(0)->IsAsynch())
	{
		TESTC(ulAsynch == DBPROPVAL_ASYNCH_INITIALIZE);
		TESTC(SettableProperty(DBPROP_INIT_ASYNCH, DBPROPSET_DBINIT));
	}
	else
	{
		TESTC(ulAsynch == 0);
		TESTC(SupportedProperty(DBPROP_INIT_ASYNCH, DBPROPSET_DBINIT));
	}


CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END





// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc DBPROP_INIT_ASYNCH - DB_S_ERRORSOCCURRED - Optional
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAsynchProperties::Variation_3()
{ 
	// TO DO:  Add your own code here 
	CDataSource DataSourceA;

	//Variation only applies for datasource
	if (pCSource(0)->GetObjectType() != DATASOURCE_INTERFACE)
	{
		TOUTPUT_LINE(L"The variation runs only for DataSource");
		return TEST_SKIPPED;
	};
	
	DataSourceA.CreateInstance();

    //NOTSUPPORTED (wrong propset)
    DataSourceA.SetProperty(DBPROP_INIT_ASYNCH,	DBPROPSET_DATASOURCEINFO, (void*)DBPROPVAL_ASYNCH_INITIALIZE, DBTYPE_I4, DBPROPOPTIONS_OPTIONAL, DB_NULLID); 
	//BADVALUE (wrong type DBTYPE_BOOL)
    DataSourceA.SetProperty(DBPROP_INIT_ASYNCH,	DBPROPSET_DBINIT, (void*)DBPROPVAL_ASYNCH_INITIALIZE, DBTYPE_BOOL, DBPROPOPTIONS_OPTIONAL, DB_NULLID); 
	//BADVALUE (wrong value DBTYPE_I2)
    DataSourceA.SetProperty(DBPROP_INIT_ASYNCH,	DBPROPSET_DBINIT, (void*)DBPROPVAL_ASYNCH_INITIALIZE, DBTYPE_I2, DBPROPOPTIONS_OPTIONAL, DB_NULLID); 
	//BADVALUE (wrong value)
    DataSourceA.SetProperty(DBPROP_INIT_ASYNCH,	DBPROPSET_DBINIT, (void*)(ULONG_PTR)INVALID(DWORD), DBTYPE_I4, DBPROPOPTIONS_OPTIONAL, DB_NULLID); 
	//BADOPTION(wrong value)
    DataSourceA.SetProperty(DBPROP_INIT_ASYNCH,	DBPROPSET_DBINIT, (void*)PROPVAL_NEGATIVE, DBTYPE_I4, DBPROPOPTIONS_OPTIONAL, DB_NULLID); 

    //Property errors - DB_S_ERRORSOCCURRED
    TESTC_(DataSourceA.Initialize(), DB_S_ERRORSOCCURRED);
	
    //Verify error array
    TCOMPARE_(DataSourceA.m_rgPropSets[0].rgProperties[0].dwStatus == DBPROPSTATUS_NOTSUPPORTED);
    TCOMPARE_(DataSourceA.m_rgPropSets[1].rgProperties[0].dwStatus == DBPROPSTATUS_BADVALUE);
    TCOMPARE_(DataSourceA.m_rgPropSets[1].rgProperties[1].dwStatus == DBPROPSTATUS_BADVALUE);
    TCOMPARE_(DataSourceA.m_rgPropSets[1].rgProperties[2].dwStatus == DBPROPSTATUS_BADVALUE);
    TCOMPARE_(DataSourceA.m_rgPropSets[1].rgProperties[3].dwStatus == DBPROPSTATUS_BADVALUE);

CLEANUP:
	DataSourceA.Uninitialize();
    TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc DBPROP_INIT_ASYNCH - DB_E_ERRORSOCCURRED - Required
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAsynchProperties::Variation_4()
{ 
	// TO DO:  Add your own code here 
	CDataSource DataSourceA;

	//Variation only applies for datasource
	if (pCSource(0)->GetObjectType() != DATASOURCE_INTERFACE)
	{
		TOUTPUT_LINE(L"The variation runs only for DataSource");
		return TEST_SKIPPED;
	};
	
	DataSourceA.CreateInstance();

    //NOTSUPPORTED (wrong propset)
    DataSourceA.SetProperty(DBPROP_INIT_ASYNCH,	DBPROPSET_DATASOURCEINFO, (void*)DBPROPVAL_ASYNCH_INITIALIZE, DBTYPE_I4, DBPROPOPTIONS_REQUIRED, DB_NULLID); 
	//BADVALUE (wrong type DBTYPE_BOOL)
    DataSourceA.SetProperty(DBPROP_INIT_ASYNCH,	DBPROPSET_DBINIT, (void*)DBPROPVAL_ASYNCH_INITIALIZE, DBTYPE_BOOL, DBPROPOPTIONS_REQUIRED, DB_NULLID); 
	//BADVALUE (wrong value DBTYPE_I2)
    DataSourceA.SetProperty(DBPROP_INIT_ASYNCH,	DBPROPSET_DBINIT, (void*)DBPROPVAL_ASYNCH_INITIALIZE, DBTYPE_I2, DBPROPOPTIONS_REQUIRED, DB_NULLID); 
	//BADVALUE (wrong value)
    DataSourceA.SetProperty(DBPROP_INIT_ASYNCH,	DBPROPSET_DBINIT, (void*)(ULONG_PTR)INVALID(DWORD), DBTYPE_I4, DBPROPOPTIONS_REQUIRED, DB_NULLID); 
	//BADOPTION(wrong value)
    DataSourceA.SetProperty(DBPROP_INIT_ASYNCH,	DBPROPSET_DBINIT, (void*)PROPVAL_NEGATIVE, DBTYPE_I4, DBPROPOPTIONS_OPTIONAL, DB_NULLID); 
	//BADOPTION(wrong value)
    DataSourceA.SetProperty(DBPROP_INIT_ASYNCH,	DBPROPSET_DBINIT, (void*)DBPROPVAL_ASYNCH_INITIALIZE, DBTYPE_I4, INVALID(DBPROPOPTIONS), DB_NULLID); 

    //Property errors - DB_E_ERRORSOCCURRED
    TESTC_(DataSourceA.Initialize(), DB_E_ERRORSOCCURRED);
	
    //Verify error array
    TCOMPARE_(DataSourceA.m_rgPropSets[0].rgProperties[0].dwStatus == DBPROPSTATUS_NOTSUPPORTED);
    TCOMPARE_(DataSourceA.m_rgPropSets[1].rgProperties[0].dwStatus == DBPROPSTATUS_BADVALUE);
    TCOMPARE_(DataSourceA.m_rgPropSets[1].rgProperties[1].dwStatus == DBPROPSTATUS_BADVALUE);
    TCOMPARE_(DataSourceA.m_rgPropSets[1].rgProperties[2].dwStatus == DBPROPSTATUS_BADVALUE);
    TCOMPARE_(DataSourceA.m_rgPropSets[1].rgProperties[3].dwStatus == DBPROPSTATUS_BADVALUE);
    TCOMPARE_(DataSourceA.m_rgPropSets[1].rgProperties[4].dwStatus == DBPROPSTATUS_BADOPTION);

CLEANUP:
	DataSourceA.Uninitialize();
    TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc DBPROP_ROWSET_ASYNCH - S_OK - Optional
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAsynchProperties::Variation_5()
{ 
	TBEGIN

	ULONG_PTR ulAsynch = 0;
	ULONG ulAsynchPhase = 0;
	HRESULT hr = 0;
	IDBAsynchStatus *pIDBAsynchStatus = NULL;
	COpenRowset OpenRowsetA;
	
	//Variation only applies for rowsets
	if (pCSource(0)->GetObjectType() != ROWSET_INTERFACE)
	{
		TOUTPUT_LINE(L"The variation runs only for Rowset");
		return TEST_SKIPPED;
	};

    OpenRowsetA.SetProperty(DBPROP_ROWSET_ASYNCH, DBPROPSET_ROWSET, (void*)(DBPROPVAL_ASYNCH_INITIALIZE | DBPROPVAL_ASYNCH_RANDOMPOPULATION | DBPROPVAL_ASYNCH_SEQUENTIALPOPULATION), DBTYPE_I4, DBPROPOPTIONS_OPTIONAL, DB_NULLID); 

    hr = OpenRowsetA.CreateOpenRowset(IID_IDBAsynchStatus, (IUnknown**)&pIDBAsynchStatus);

	switch (hr)
	{
		case DB_S_ERRORSOCCURRED:
			TESTC(!SupportedProperty(DBPROP_ROWSET_ASYNCH, DBPROPSET_ROWSET) || !SettableProperty(DBPROP_ROWSET_ASYNCH,	DBPROPSET_ROWSET));
			break;
		case DB_S_ASYNCHRONOUS:
			TESTC(pIDBAsynchStatus!=NULL);
			while(TRUE)
			{
				//IDBAsynchStatus::GetStatus
				TESTC_(pIDBAsynchStatus->GetStatus(DB_NULL_HCHAPTER, DBASYNCHOP_OPEN, NULL, NULL, &ulAsynchPhase, NULL), S_OK);
				
				//Check for an error in the provider
				TESTC(DBASYNCHPHASE_INITIALIZATION==ulAsynchPhase || DBASYNCHPHASE_COMPLETE==ulAsynchPhase || DBASYNCHPHASE_POPULATION==ulAsynchPhase)

				//As soon as we have S_OK and COMPLETE we are done...
				if(ulAsynchPhase==DBASYNCHPHASE_COMPLETE)
					break;

				//Sleep a little, so we don't have such a tight loop...
				Sleep(100);
			}
			TESTC(GetProperty(DBPROP_ROWSET_ASYNCH, DBPROPSET_ROWSET, pIDBAsynchStatus, &ulAsynch));
			TESTC(ulAsynch & (DBASYNCHPHASE_INITIALIZATION==ulAsynchPhase || DBASYNCHPHASE_COMPLETE==ulAsynchPhase || DBASYNCHPHASE_POPULATION==ulAsynchPhase));
			break;
		case S_OK:
			//TO DO: may be we should compare ulAsynch with 0 in this case?
			TWARNING("Rowset creation returned S_OK, is the rowset was created Immediatly?");
			TESTC(GetProperty(DBPROP_ROWSET_ASYNCH, DBPROPSET_ROWSET, pIDBAsynchStatus, &ulAsynch));
			TESTC(ulAsynch & (DBASYNCHPHASE_INITIALIZATION==ulAsynchPhase || DBASYNCHPHASE_COMPLETE==ulAsynchPhase || DBASYNCHPHASE_POPULATION==ulAsynchPhase));
			break;
	}

CLEANUP:
	SAFE_RELEASE(pIDBAsynchStatus);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc DBPROP_ROWSET_ASYNCH - S_OK - Required
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAsynchProperties::Variation_6()
{ 
	TBEGIN
	ULONG_PTR ulAsynch = 0;
	
	//Variation only applies for rowsets
	if (pCSource(0)->GetObjectType() != ROWSET_INTERFACE)
	{
		TOUTPUT_LINE(L"The variation runs only for Rowset");
		return TEST_SKIPPED;
	};

	// Reset DataSource and Advise Listeners
	TESTC(ResetSources());
	TESTC(CauseNotifications());

	//Wait until Complete
	TESTC_(WaitUntilComplete(),S_OK);

	//DBPROP_ROWSET_ASYNCH
	TESTC(GetProperty(DBPROP_ROWSET_ASYNCH, DBPROPSET_ROWSET, pCSource(0)->pIDBAsynchStatus(), &ulAsynch));
	
	if(pCSource(0)->IsAsynch())
	{
		TESTC(ulAsynch & DBPROPVAL_ASYNCH_INITIALIZE);
		TESTC(SettableProperty(DBPROP_ROWSET_ASYNCH, DBPROPSET_ROWSET));
	}
	else
	{
		TESTC(ulAsynch == 0);
		TESTC(SupportedProperty(DBPROP_ROWSET_ASYNCH, DBPROPSET_ROWSET));
	}


CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc DBPROP_ROWSET_ASYNCH - DB_S_ERRORSOCCURRED - Optional
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAsynchProperties::Variation_7()
{ 
	COpenRowset OpenRowsetA;
	IRowset* pIRowset = NULL;

	//Variation only applies for rowsets
	if (pCSource(0)->GetObjectType() != ROWSET_INTERFACE)
	{
		TOUTPUT_LINE(L"The variation runs only for Rowset");
		return TEST_SKIPPED;
	};

    //NOTSUPPORTED (wrong propset)
    OpenRowsetA.SetProperty(DBPROP_ROWSET_ASYNCH,	DBPROPSET_ROWSETALL, (void*)DBPROPVAL_ASYNCH_INITIALIZE, DBTYPE_I4, DBPROPOPTIONS_OPTIONAL, DB_NULLID); 
	//BADVALUE (wrong type)
    OpenRowsetA.SetProperty(DBPROP_ROWSET_ASYNCH,	DBPROPSET_ROWSET, (void*)DBPROPVAL_ASYNCH_INITIALIZE, DBTYPE_I2, DBPROPOPTIONS_OPTIONAL, DB_NULLID); 
	//BADVALUE (wrong type)
    OpenRowsetA.SetProperty(DBPROP_ROWSET_ASYNCH,	DBPROPSET_ROWSET, (void*)DBPROPVAL_ASYNCH_INITIALIZE, DBTYPE_BOOL, DBPROPOPTIONS_OPTIONAL, DB_NULLID); 
	//BADVALUE (wrong value)
    OpenRowsetA.SetProperty(DBPROP_ROWSET_ASYNCH,	DBPROPSET_ROWSET, (void*)(ULONG_PTR)INVALID(DWORD), DBTYPE_I4, DBPROPOPTIONS_OPTIONAL, DB_NULLID); 
	//BADVALUE (wrong value)
    OpenRowsetA.SetProperty(DBPROP_ROWSET_ASYNCH,	DBPROPSET_ROWSET, (void*)PROPVAL_NEGATIVE, DBTYPE_I4, DBPROPOPTIONS_OPTIONAL, DB_NULLID); 

    //Property errors - DB_S_ERRORSOCCURRED
    TESTC_(OpenRowsetA.CreateOpenRowset(IID_IUnknown, (IUnknown**)&pIRowset), DB_S_ERRORSOCCURRED);
    TESTC(pIRowset != NULL);
	
    //Verify error array
    TCOMPARE_(OpenRowsetA.m_rgPropSets[0].rgProperties[0].dwStatus == DBPROPSTATUS_NOTSUPPORTED);
    TCOMPARE_(OpenRowsetA.m_rgPropSets[1].rgProperties[0].dwStatus == DBPROPSTATUS_BADVALUE);
    TCOMPARE_(OpenRowsetA.m_rgPropSets[1].rgProperties[1].dwStatus == DBPROPSTATUS_BADVALUE);
    TCOMPARE_(OpenRowsetA.m_rgPropSets[1].rgProperties[2].dwStatus == DBPROPSTATUS_BADVALUE);
    TCOMPARE_(OpenRowsetA.m_rgPropSets[1].rgProperties[3].dwStatus == DBPROPSTATUS_BADVALUE);

CLEANUP:
    SAFE_RELEASE(pIRowset);
    TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc DBPROP_ROWSET_ASYNCH - DB_E_ERRORSOCCURRED - Required
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAsynchProperties::Variation_8()
{ 
	COpenRowset OpenRowsetA;
	IRowset* pIRowset = NULL;

	//Variation only applies for rowsets
	if (pCSource(0)->GetObjectType() != ROWSET_INTERFACE)
	{
		TOUTPUT_LINE(L"The variation runs only for Rowset");
		return TEST_SKIPPED;
	};

    //NOTSUPPORTED (wrong propset)
    OpenRowsetA.SetProperty(DBPROP_CANHOLDROWS,		DBPROPSET_ROWSETALL, (void*)VARIANT_TRUE, DBTYPE_BOOL, DBPROPOPTIONS_OPTIONAL, DB_NULLID); 
    //NOTSUPPORTED (wrong propset)
    OpenRowsetA.SetProperty(DBPROP_ROWSET_ASYNCH,	DBPROPSET_ROWSETALL, (void*)DBPROPVAL_ASYNCH_INITIALIZE, DBTYPE_I4, DBPROPOPTIONS_REQUIRED, DB_NULLID); 
	//BADVALUE (wrong type)
    OpenRowsetA.SetProperty(DBPROP_ROWSET_ASYNCH,	DBPROPSET_ROWSET, (void*)DBPROPVAL_ASYNCH_INITIALIZE, DBTYPE_I2, DBPROPOPTIONS_REQUIRED, DB_NULLID); 
	//BADVALUE (wrong value)
    OpenRowsetA.SetProperty(DBPROP_ROWSET_ASYNCH,	DBPROPSET_ROWSET, (void*)(ULONG_PTR)INVALID(DWORD), DBTYPE_I4, DBPROPOPTIONS_REQUIRED, DB_NULLID); 
	//BADOPTION(wrong value)
    OpenRowsetA.SetProperty(DBPROP_ROWSET_ASYNCH,	DBPROPSET_ROWSET, (void*)DBPROPVAL_ASYNCH_INITIALIZE, DBTYPE_I4, INVALID(DBPROPOPTIONS), DB_NULLID); 

    //Property errors - DB_E_ERRORSOCCURRED
    TESTC_(OpenRowsetA.CreateOpenRowset(IID_IUnknown, (IUnknown**)&pIRowset), DB_E_ERRORSOCCURRED);
    TESTC(pIRowset == NULL);
	
    //Verify error array
    TCOMPARE_(OpenRowsetA.m_rgPropSets[0].rgProperties[0].dwStatus == DBPROPSTATUS_NOTSUPPORTED);
    TCOMPARE_(OpenRowsetA.m_rgPropSets[0].rgProperties[1].dwStatus == DBPROPSTATUS_NOTSUPPORTED);
    TCOMPARE_(OpenRowsetA.m_rgPropSets[1].rgProperties[0].dwStatus == DBPROPSTATUS_BADVALUE);
    TCOMPARE_(OpenRowsetA.m_rgPropSets[1].rgProperties[1].dwStatus == DBPROPSTATUS_BADVALUE);
    TCOMPARE_(OpenRowsetA.m_rgPropSets[1].rgProperties[2].dwStatus == DBPROPSTATUS_BADOPTION);

CLEANUP:
    SAFE_RELEASE(pIRowset);
    TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCAsynchProperties::Terminate()
{ 
	// TO DO:  Add your own code here 

// {{ TCW_TERM_BASECLASS_CHECK2
	return(CTestNotify::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END

