//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright (C) 1995-2000 Microsoft Corporation
//
// @doc 
//
// @module ICPoint.cpp | This test module performs testing of IConnectionPoint interface 
//

#include "MODStandard.hpp"				// Standard headers to be precompiled in MODStandard.cpp			
#include "ICPoint.h"
#include "ExtraLib.h"
#include "olectl.h"						//CONNECT_E_NOCONNECT

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Module Values
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// {{ TCW_MODULE_GLOBALS
DECLARE_MODULE_CLSID = { 0xe1b56244, 0x6bfb, 0x11cf, { 0xaa, 0x20, 0x00, 0xaa, 0x00, 0x3e, 0x77, 0x8a }};
DECLARE_MODULE_NAME("IConnectionPoint");
DECLARE_MODULE_OWNER("Microsoft");
DECLARE_MODULE_DESCRIP("IConnectionPoint test");
DECLARE_MODULE_VERSION(838751865);
// TCW_WizardVersion(2)
// TCW_Automation(True)
// }} TCW_MODULE_GLOBALS_END


////////////////////////////////////////////////////////////////////////////
// Forwards
//
////////////////////////////////////////////////////////////////////////////
class CSource;
class CListener;

////////////////////////////////////////////////////////////////////////////
// Constants
//
////////////////////////////////////////////////////////////////////////////
const int MAX_CP_ON_SOURCE = 2;

////////////////////////////////////////////////////////////////////////////
// Defines
//
////////////////////////////////////////////////////////////////////////////
#define INVALID_PUNK	INVALID(IUnknown*)
#define INVALID_COOKIE	INVALID(DWORD)


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
				SupportedInterface(IID_IConnectionPointContainer, ROWSET_INTERFACE) ||
				SupportedInterface(IID_IConnectionPointContainer, ROW_INTERFACE) ||
				SupportedInterface(IID_IConnectionPointContainer, DATASOURCE_INTERFACE)
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

//--------------------------------------------------------------------
// @func BOOL|
//		VerifyEqualICPoint
//		Check if connection points are the same
//--------------------------------------------------------------------
BOOL VerifyEqualICPoint(IConnectionPoint *pICP1, IConnectionPoint *pICP2)
{
	TBEGIN;

    HRESULT hr1 = S_OK;
	HRESULT hr2 = S_OK;
	HRESULT hr = S_OK;
	
	IID iid1 = IID_NULL;
	IID iid2 = IID_NULL;

	IEnumConnections *pIEnumC1 = NULL;
	IEnumConnections *pIEnumC2 = NULL;

	DWORD dwCookie = 0;
	CListener* pListener = NULL;
	CONNECTDATA rgConnectData[2] = { {NULL, 0}, {NULL, 0} }; 
	
	// if Objects match by IUnknown no more verification nedded
	if (VerifyEqualInterface(pICP1, pICP2))
		return TRUE;

	// Verify connection interface
	hr1 = pICP1->GetConnectionInterface(&iid1);
	hr2 = pICP2->GetConnectionInterface(&iid2);
	TESTC(hr1==hr2 && iid1==iid2);

	pListener = new CListener(IID_IUnknown);
	SAFE_ADDREF(pListener);

	// Verify Advise/Unadvise
	hr = pICP1->Advise(pListener, &dwCookie);

	// Verify that both CP contains the same listeners
	hr1 = pICP1->EnumConnections(&pIEnumC1);
	hr2 = pICP2->EnumConnections(&pIEnumC2);

	if (!VerifyEqualInterface(pIEnumC1, pIEnumC2))
	{
		ULONG cFetched1 = 0;
		ULONG cFetched2 = 0;
		for (ULONG i=1; i<7; i++)
		{
			hr1 = pIEnumC1->Next(1, &rgConnectData[0], &cFetched1);
			hr2 = pIEnumC2->Next(1, &rgConnectData[1], &cFetched2);
			TESTC(hr1==hr2 && cFetched1==cFetched2 && 
					rgConnectData[0].dwCookie == rgConnectData[1].dwCookie &&
					rgConnectData[0].pUnk == rgConnectData[1].pUnk);
			SAFE_RELEASE(rgConnectData[0].pUnk);
			SAFE_RELEASE(rgConnectData[1].pUnk);
			rgConnectData[0].dwCookie = 0;
			rgConnectData[1].dwCookie = 0;
		}
	}
	

CLEANUP:
	if (SUCCEEDED(hr))
		TESTC_(pICP2->Unadvise(dwCookie), S_OK);
	SAFE_RELEASE(rgConnectData[0].pUnk);
	SAFE_RELEASE(rgConnectData[1].pUnk);
	SAFE_RELEASE(pListener);
	SAFE_RELEASE(pIEnumC1);
	SAFE_RELEASE(pIEnumC2);
    TRETURN
}

//--------------------------------------------------------------------
// @func BOOL|
//		VerifyEqualICPC
//		Check if connection point containers are the same
//--------------------------------------------------------------------
BOOL VerifyEqualICPC(IConnectionPointContainer *pICPC1, IConnectionPointContainer *pICPC2)
{
	TBEGIN;
	
	HRESULT hr1 = S_OK;
	HRESULT hr2 = S_OK;

	IConnectionPoint *pCPoint1 = NULL;
	IConnectionPoint *pCPoint2 = NULL;

	IID rgIIDs[3];

	// All possible connection points
	rgIIDs[0] = IID_IRowsetNotify;
	rgIIDs[1] = IID_IDBAsynchNotify;
	rgIIDs[2] =	IID_IRowPositionChange;

	if (VerifyEqualInterface(pICPC1, pICPC2))
		return TRUE;

	// Ferify consistency in connection points
	for (ULONG i = 0; i<3; i++)
	{
		hr1 = pICPC1->FindConnectionPoint(rgIIDs[i], &pCPoint1);
		hr2 = pICPC2->FindConnectionPoint(rgIIDs[i], &pCPoint2);
		TESTC(hr1==hr2);
		TESTC(VerifyEqualICPoint(pCPoint1, pCPoint2));
		SAFE_RELEASE(pCPoint1);
		SAFE_RELEASE(pCPoint2);
	}

CLEANUP:
	SAFE_RELEASE(pCPoint1);
	SAFE_RELEASE(pCPoint2);
    TRETURN
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
	DWORD		m_dwCookie;
	CSource*	m_pCSource;
	CListener*	m_pCListener;
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
	virtual BOOL IsOptionalCP(REFIID riid) = 0;
	virtual BOOL IsSupportedCP(REFIID riid);

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
	//m_rgSupportedCP array contains CP that can be found through FindConnectionPoint and EnumConnectionPoints
	IID			m_rgSupportedCP[MAX_CP_ON_SOURCE];
	ULONG		m_cSupportedCP;

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

	//Valid CP
	for (ULONG i=0; i<MAX_CP_ON_SOURCE; i++)
	{
		m_rgSupportedCP[i] = IID_NULL;
	}
	m_cSupportedCP = 0;

}

CSource::~CSource()
{
	ReleaseCP();
}

BOOL CSource::IsSupportedCP(REFIID riid)
{
	for (ULONG i=0; i<m_cSupportedCP; i++)
	{
		if (riid == m_rgSupportedCP[i])
			return TRUE;
	}
	return FALSE;
}

BOOL CSource::ReleaseCP()
{
	//Container
	SAFE_RELEASE(m_pIConnectionPointContainer);
	SAFE_RELEASE(m_pIEnumConnectionPoints);

	//Connection Points
	for(ULONG i=0; i<m_cConnectionPoints; i++)
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
	IConnectionPoint* pICP2 = NULL;
	BOOL fFindCP = FALSE;
	IID iid;

	//Obtain the connection point container
	QTESTC_((QI(pIUnknown, IID_IConnectionPointContainer, (void**)&m_pIConnectionPointContainer)),S_OK);

	//NOTE:
	//If IConnectionPointContainer is exposed by the Source we need to test it (at least run TCEnumConnectionPoints)
	//to be sure that there are no wrong CPs on the source => commented out 
	
	//Make sure this connection supports the connection point were interested in...
	//QTESTC_(m_pIConnectionPointContainer->FindConnectionPoint(m_iid, &pICP),S_OK);
	//SAFE_RELEASE(pICP);

	//Obtain the IEnumConnectionPoints 
	TESTC_(m_pIConnectionPointContainer->EnumConnectionPoints(&m_pIEnumConnectionPoints),S_OK);
	TESTC(m_pIEnumConnectionPoints != NULL)

	//Obtain all the Connection Points
	m_cConnectionPoints = 0;
	SAFE_ALLOC(m_rgConnectionPoints, IConnectionPoint*, m_cConnectionPoints + 1);

	//Obtain all connection points...
	while(SUCCEEDED(pIEnumCP()->Next(1, &pICP, &cFetched)) && cFetched == 1)
	{
		m_rgConnectionPoints[m_cConnectionPoints] = pICP;
		SAFE_REALLOC(m_rgConnectionPoints, IConnectionPoint*, m_cConnectionPoints + 1 + 1);


		//if iid of pICP is not yet in the array of Supported CP and it's optional for this Source
		//and we can find it through FindConnectionPoint => add it to the array of Supported CP (we will test it) 
		if (S_OK==pICP->GetConnectionInterface(&iid) && IsOptionalCP(iid) && !IsSupportedCP(iid) && 
				S_OK==m_pIConnectionPointContainer->FindConnectionPoint(iid, &pICP2) && pICP2)
		{
			m_rgSupportedCP[m_cSupportedCP++] = iid;
		}
		SAFE_RELEASE(pICP2);

		//To make our life easier, lets place the desired ConnectionPoint, first in the list
		//so we can easily access it and distingush it from the other connection points.  We just
		//need to swap it with the first element of the array...
		//TESTC_(pICP->GetConnectionInterface(&iid),S_OK);
		if(iid == m_iid && !fFindCP)
		{
			m_rgConnectionPoints[m_cConnectionPoints] = m_rgConnectionPoints[0];
			m_rgConnectionPoints[0] = pICP;
			fFindCP = TRUE;
		}

		//Increment total connection points...
		m_cConnectionPoints++;
	}
	
	//Make sure that we found the connection point in the enumeration
//	TESTC(m_cConnectionPoints >= 1);
//	TESTC_(m_rgConnectionPoints[0]->GetConnectionInterface(&iid),S_OK);
//	TESTC(iid == m_iid);

CLEANUP:
	TRETURN;
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
// class CAsynchDSOSource
//
//////////////////////////////////////////////////////////////////////////
class CAsynchDSOSource : public CSource, public CDataSource
{
public:
	//Constructors
	CAsynchDSOSource(WCHAR* pwszTestCaseName = INVALID(WCHAR*));
    virtual ~CAsynchDSOSource();

    //pure virtual members (defined)
	virtual BOOL CreateSource();
	virtual BOOL CauseNotification();
	virtual int IsOptionalCP(REFIID riid) {return IID_IDBAsynchNotify==riid;};


	//Helpers
	virtual BOOL VerifyNotification(CListener* pCListener, ULONG ulTimesConnected);

protected:
};


CAsynchDSOSource::CAsynchDSOSource(WCHAR* pwszTestCaseName) 
	: CDataSource((LPWSTR)pwszTestCaseName), CSource(IID_IDBAsynchNotify, DATASOURCE_INTERFACE)
{
}

CAsynchDSOSource::~CAsynchDSOSource()
{
}


BOOL CAsynchDSOSource::CreateSource()
{
	TBEGIN

	HRESULT hr = S_OK;
	
	//Create an Instance first, so we can hook up out listeners, 
	//before any notifications
	TESTC_(hr = CDataSource::CreateInstance(),S_OK);
	
	//Now that we have our object, just call the baseclass to obtain all the connection pointers...
	QTESTC(CreateCP(m_pIDBInitialize));

CLEANUP:
	TRETURN
}


BOOL CAsynchDSOSource::CauseNotification()
{
	TBEGIN

	//Cause a notification to occur
//TODO	
//CLEANUP:
	TRETURN
}

BOOL CAsynchDSOSource::VerifyNotification(CListener* pCListener, ULONG ulTimesConnected)
{
	ASSERT(pCListener);
	TBEGIN

//TODO
//	TESTC(pCListener->GetTimesNotified() >= ulTimesConnected);

//CLEANUP:
	TRETURN
}



//////////////////////////////////////////////////////////////////////////
// class CRowsetSource
//
//////////////////////////////////////////////////////////////////////////
class CRowsetSource : public CSource, public CRowset
{
public:
	//Constructors
	CRowsetSource(REFIID riid = IID_IRowsetNotify, WCHAR* pwszTestCaseName = INVALID(WCHAR*));
    virtual ~CRowsetSource();

    //pure virtual members (defined)
	virtual BOOL CreateSource();
	virtual BOOL CauseNotification();
	virtual int IsOptionalCP(REFIID riid) {return IID_IRowsetNotify==riid || IID_IDBAsynchNotify==riid;};

	//Helpers
	virtual BOOL VerifyNotification(CListener* pCListener, ULONG ulTimesConnected);
protected:
};


CRowsetSource::CRowsetSource(REFIID riid, WCHAR* pwszTestCaseName) 
	: CRowset((LPWSTR)pwszTestCaseName), CSource(riid, ROWSET_INTERFACE)
{
}

CRowsetSource::~CRowsetSource()
{
}


BOOL CRowsetSource::CreateSource()
{
	TBEGIN

	//If ICPC is not supported, were done testing!
	//Create the rowset asking for IID_IConnectionPointContainer, same as 
	//explicitly setting the property DBPROP_IConnectionPointContainer
	SetProperty(DBPROP_CANHOLDROWS);
	QTESTC_(CreateRowset(USE_SUPPORTED_SELECT_ALLFROMTBL, IID_IConnectionPointContainer),S_OK);

	//Now that we have our object, just call the baseclass to obtain all the connection pointers...
	QTESTC(CreateCP(pIRowset()));

CLEANUP:
	TRETURN
}


BOOL CRowsetSource::CauseNotification()
{
	TBEGIN
	HROW hRow = NULL;

	//Cause a notification to occur DBREASON_ROW_ACTIVATE
	TESTC_(GetRow(FIRST_ROW,&hRow),S_OK);
	
CLEANUP:
	//Release row and cause closing notification DBREASON_ROW_RELEASE
	ReleaseRows(hRow);
	TRETURN
}

BOOL CRowsetSource::VerifyNotification(CListener* pCListener, ULONG ulTimesConnected)
{
	ASSERT(pCListener);
	TBEGIN

	//CauseNotification only causes notification for IrowsetNotify
	//IDBAsynchNotify is tested in the IDBAsynch Test
	if (IID_IRowsetNotify==GetIID())
	{
		TESTC(pCListener->GetTimesNotified() >= ulTimesConnected);
	}

CLEANUP:
	TRETURN
}



//////////////////////////////////////////////////////////////////////////
// class CAsynchRowSource
//
//////////////////////////////////////////////////////////////////////////
class CAsynchRowSource : public CSource, public CRowObject
{
public:
	//Constructors
	CAsynchRowSource(WCHAR* pwszTestCaseName = INVALID(WCHAR*));
    virtual ~CAsynchRowSource();

    //pure virtual members (defined)
	virtual BOOL CreateSource();
	virtual BOOL CauseNotification();
	virtual int IsOptionalCP(REFIID riid) {return IID_IDBAsynchNotify==riid;};

	//Helpers
	virtual BOOL VerifyNotification(CListener* pCListener, ULONG ulTimesConnected);

protected:
};


CAsynchRowSource::CAsynchRowSource(WCHAR* pwszTestCaseName)
	: CSource(IID_IDBAsynchNotify, ROW_INTERFACE)
{
}

CAsynchRowSource::~CAsynchRowSource()
{
}


BOOL CAsynchRowSource::CreateSource()
{
	TBEGIN
	HRESULT hr = S_OK;
	IRow* pIRow = NULL;

	//Create a row object
	//Row Objects are optional behavior...
	TEST3C_(hr = g_pTable->CreateRowset(USE_OPENROWSET, IID_IRow, 0, NULL, (IUnknown**)&pIRow),S_OK,DB_S_NOTSINGLETON,E_NOINTERFACE);

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


BOOL CAsynchRowSource::CauseNotification()
{
	TBEGIN

	//Cause a notification to occur
//TODO	
//CLEANUP:
	TRETURN
}

BOOL CAsynchRowSource::VerifyNotification(CListener* pCListener, ULONG ulTimesConnected)
{
	ASSERT(pCListener);
	TBEGIN

//TODO
//	TESTC(pCListener->GetTimesNotified() >= ulTimesConnected);

//CLEANUP:
	TRETURN
}



//////////////////////////////////////////////////////////////////////////
// class CAsynchStreamSource
//
//////////////////////////////////////////////////////////////////////////
class CAsynchStreamSource : public CSource, public CRowObject
{
public:
	//Constructors
	CAsynchStreamSource(WCHAR* pwszTestCaseName = INVALID(WCHAR*));
    virtual ~CAsynchStreamSource();

    //pure virtual members (defined)
	virtual BOOL CreateSource();
	virtual BOOL CauseNotification();
	virtual int IsOptionalCP(REFIID riid) {return IID_IDBAsynchNotify==riid;};

	//Helpers
	virtual BOOL VerifyNotification(CListener* pCListener, ULONG ulTimesConnected);

protected:
};


CAsynchStreamSource::CAsynchStreamSource(WCHAR* pwszTestCaseName)
	: CSource(IID_IDBAsynchNotify, STREAM_INTERFACE)
{
}

CAsynchStreamSource::~CAsynchStreamSource()
{
}


BOOL CAsynchStreamSource::CreateSource()
{
	TBEGIN
	HRESULT hr = S_OK;
	IRow* pIRow = NULL;
	IUnknown* pIUnknown = NULL;
	ULONG iCol=0;

	//Create a row object
	//Row Objects are optional behavior...
	TEST3C_(hr = g_pTable->CreateRowset(USE_OPENROWSET, IID_IRow, 0, NULL, (IUnknown**)&pIRow),S_OK,DB_S_NOTSINGLETON,E_NOINTERFACE);

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
			//We need at least one stream that supports connection points
			if(CreateCP(pIUnknown))
				break;
		}

		SAFE_RELEASE(pIUnknown);
	}
	
	//At this point we should a stream object, that supports connection points
	QTESTC(pIUnknown != NULL);

CLEANUP:
	SAFE_RELEASE(pIUnknown);
	SAFE_RELEASE(pIRow);
	TRETURN
}


BOOL CAsynchStreamSource::CauseNotification()
{
	TBEGIN

	//Cause a notification to occur
//TODO	
//CLEANUP:
	TRETURN
}

BOOL CAsynchStreamSource::VerifyNotification(CListener* pCListener, ULONG ulTimesConnected)
{
	ASSERT(pCListener);
	TBEGIN

//TODO
//	TESTC(pCListener->GetTimesNotified() >= ulTimesConnected);

//CLEANUP:
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
		m_dwTestCaseParam1 = UNKNOWN_INTERFACE;
		m_dwTestCaseParam2 = IID_NULL;
	}

	//methods
	virtual void SetTestCaseParams(EINTERFACE dwTestCaseParam1, REFIID dwTestCaseParam2 = IID_IDBAsynchNotify)
	{
		m_dwTestCaseParam1 = dwTestCaseParam1;
		m_dwTestCaseParam2 = dwTestCaseParam2;
	}

	//data
	EINTERFACE m_dwTestCaseParam1;
	IID m_dwTestCaseParam2;
};


////////////////////////////////////////////////////////////////////////////
//  TCTransaction
//
////////////////////////////////////////////////////////////////////////////
class TCTransaction : public CTransaction, public TCBase
{
public:
	//constructors
	TCTransaction(WCHAR* pwszTestCaseName = INVALID(WCHAR*)) : CTransaction(pwszTestCaseName) {}
};


//////////////////////////////////////////////////////////////////
// Class CTestNotify
//
// Manages testing between Source(s) (containers) and Sink(s) (listeners)
//////////////////////////////////////////////////////////////////
class CTestNotify : public COLEDB, public TCBase
{
public:
	//contsructor
	CTestNotify(WCHAR* pwszTestCaseName);
	virtual ~CTestNotify();

	//members
	virtual BOOL Init();
	virtual BOOL Terminate();

	//Thread Function
	static ULONG WINAPI Thread_AdviseListener(LPVOID pv);

	//helper
	virtual ULONG	VerifyRefCount();
	virtual BOOL	VerifyUniqueCookie(DWORD dwCookie, CSource* pCSource);

	//Connections
	virtual BOOL	VerifyNoConnection();
	virtual BOOL	VerifyNoConnection(CListener* pCListener, CSource* pCSource); 
	virtual BOOL	VerifyConnection(CListener* pCListener, CSource* pCSource, ULONG ulTimesConnected = ULONG_MAX);
	virtual BOOL	VerifyConnectData(ULONG cConnectData, CONNECTDATA* rgConnectData, ULONG cCookies, DWORD* rgCookies);
	virtual BOOL	VerifyConnectionPoints(ULONG cConnectionPoints, IConnectionPoint** rgpIConnectionPoints);
	virtual BOOL	FreeConnectionPoints(ULONG cConnectionPoints, IConnectionPoint** rgpIConnectionPoints);

	//Advise connections
	virtual BOOL	Advise(CListener* pCListener, CSource* pCSource, DWORD* pdwCookie = NULL);
	virtual BOOL	AdviseNum(ULONG cListeners, ULONG cSources, DWORD* const rgCookie = NULL);
	
	//Unadvise Connections
	virtual BOOL	UnadviseAll();
	virtual BOOL	Unadvise(CListener* pCListener, CSource* pCSource);

	//Indexing
	virtual CListener*	pCListener(ULONG iListener)	{ return m_vectListeners[iListener];	} 
	virtual CSource*	pCSource(ULONG iSource)		{ return m_vectSources[iSource];		}	

protected:
	//data
	BOOL					m_fInitRefCounts;
	
	//Associated (Multiple) Listeners
	CVector<CListener*>		m_vectListeners;	//array of Listeners

	//Associated (Multiple) Rowsets
	CVector<CSource*>		m_vectSources;		//array of Sources
	
	//Track connections between sources and listeners
	CList<CConnectInfo*, CConnectInfo*>	m_listConnectInfo;
	CRITICAL_SECTION ConnTableMutex;
};


CTestNotify::CTestNotify(WCHAR* pwszTestCaseName) : COLEDB(pwszTestCaseName) 
{
	//Data
	m_fInitRefCounts = FALSE;
}

CTestNotify::~CTestNotify()
{
	//Free the list of connection info
	while(!m_listConnectInfo.IsEmpty()) 
	{
		CConnectInfo* pCConnectInfo = m_listConnectInfo.RemoveHead();
		SAFE_DELETE(pCConnectInfo);
	}
}


BOOL CTestNotify::Init() 
{
	TBEGIN
	ULONG i=0;
	//This must be done and not skipped by any error
	//Since we delete the critical section in the Terminate...
	InitializeCriticalSection(&ConnTableMutex);
	
	//Init Sources
	for(i=0; i<6; i++)
	{
		CSource* pCSource = NULL;
		
		//First 2 sources have the possibility of being row sources
		switch(m_dwTestCaseParam1)
		{
			case ROWSET_INTERFACE:
				pCSource = new CRowsetSource(m_dwTestCaseParam2);
				break;

			case DATASOURCE_INTERFACE:
				pCSource = new CAsynchDSOSource;
				break;
			
			case ROW_INTERFACE:
				pCSource = new CAsynchRowSource;
				break;

			case STREAM_INTERFACE:
				pCSource = new CAsynchStreamSource;
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

	//Init Listeners
	for(i=0; i<5; i++)
	{
		//Default listener
		CListener* pCListener = new CListener;
		TESTC(pCListener != NULL);

		//Add this listener to the list...
		pCListener->AddRef();
		m_vectListeners.AddElement(pCListener);
	}

CLEANUP:
	TRETURN
}


BOOL CTestNotify::Terminate()
{
	ULONG i=0;
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
	return COLEDB::Terminate();
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
	TESTC(VerifyConnection(pCListener, pCSource));
	
	//Verify refcount is greater after advise
	TESTC(VerifyRefCounts(GetRefCount(pCListener), cRefCount+1));

CLEANUP:		
	if(pdwCookie)
		*pdwCookie = dwCookie;
	TRETURN
}

BOOL CTestNotify::AdviseNum(ULONG cListeners, ULONG cSources, DWORD* const rgdwCookies)
{
	TBEGIN
	
	//Advise the specified number of connections, and verify
	for(ULONG iListener=0, index=0; iListener<cListeners; iListener++)
	{
		for(ULONG iSource=0; iSource<cSources; iSource++, index++)
		{
			TESTC(Advise(pCListener(iListener), pCSource(iSource), rgdwCookies ? &rgdwCookies[index] : NULL));
		}
	}
				
CLEANUP:	
	TRETURN
}


ULONG CTestNotify::VerifyRefCount()
{
	TBEGIN
	static ULONG RC_Listener[1000];
	static ULONG RC_rgpICPC[1000];
	static ULONG RC_pIConnectionPoint[1000];
	static ULONG RC_pEnumCP[1000];
	ULONG i=0;
	//1 time intialization
	if(!m_fInitRefCounts)
	{
		//Now Initalized
		m_fInitRefCounts = TRUE;

		//Initalize RefCount values at init
		for(i=0; i<m_vectListeners.GetCount(); i++)
			RC_Listener[i]	= GetRefCount(pCListener(i));
	
		for(i=0; i<m_vectSources.GetCount(); i++)
		{
			RC_rgpICPC[i]				= GetRefCount(pCSource(i)->pICPC());
			RC_pIConnectionPoint[i]		= GetRefCount(pCSource(i)->GetCP());
			RC_pEnumCP[i]				= GetRefCount(pCSource(i)->pIEnumCP());
		}
	}

	//Verify only 1 reference to each sink, (our 1 pointer from this object)  
	for(i=0; i<m_vectListeners.GetCount(); i++)
		TESTC(GetRefCount(pCListener(i)) == RC_Listener[i]);
			
	//Verify only 1 reference to each rowset
	for(i=0; i<m_vectSources.GetCount(); i++)
	{	
		//Verify only 1 connection to the CPC
		TESTC(GetRefCount(pCSource(i)->pICPC()) == RC_rgpICPC[i]);
		
		//Verify only 1 connection to the CP
		TESTC(GetRefCount(pCSource(i)->GetCP()) ==RC_pIConnectionPoint[i]);

		//Verify only 1 connection to the EnumCP
		TESTC(GetRefCount(pCSource(i)->pIEnumCP()) == RC_pEnumCP[i]);
	}

CLEANUP:
	//Adjust values, so all following variations don't bomb
	for(i=0; i<m_vectListeners.GetCount(); i++)
		RC_Listener[i]	= GetRefCount(pCListener(i));
	
	for(i=0; i<m_vectSources.GetCount(); i++)
	{
		RC_rgpICPC[i]  			= GetRefCount(pCSource(i)->pICPC());
		RC_pIConnectionPoint[i]	= GetRefCount(pCSource(i)->GetCP());
		RC_pEnumCP[i]			= GetRefCount(pCSource(i)->pIEnumCP());
	}

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
		
			//Verify no connection, or correct numerber of connections left
			TESTC(VerifyConnection(pCListener, pCSource));
			TESTC(VerifyRefCounts(GetRefCount(pCListener), cRefCount-1));
			break;
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
	TESTC(VerifyNoConnection());
	
	//Verify reference count 
	TESTC(VerifyRefCount());
	
CLEANUP:	
	TRETURN
}


///////////////////////////////////////////////////////////
// Thread routines - IConnectionPoint
//
///////////////////////////////////////////////////////////

ULONG CTestNotify::Thread_AdviseListener(LPVOID pv)
{
	THREAD_BEGIN					 

	//Thread Stack Variables
	IUnknown* pIUnknown = (IUnknown*)THREAD_FUNC;
	ASSERT(pIUnknown);

	//"new" is not thread safe!
	EnterCriticalSection(&GlobalMutex);
		
	//Local Variables
	IConnectionPoint* pICP = NULL;
	IConnectionPointContainer* pICPC = NULL;
	DWORD dwCookie = 0;

	//Instantiate a listener object, within this thread
	CListener* pCListener = new CListener(*(IID*)THREAD_ARG1);//(IID_IRowsetNotify);
	TESTC(pCListener!=NULL);
	pCListener->AddRef();

	//Obtain the connection point container
	TESTC_(pIUnknown->QueryInterface(IID_IConnectionPointContainer,(void **)&pICPC),S_OK);
	//Obtain the IRowsetNotify connection point 
	TESTC_(pICPC->FindConnectionPoint(*(IID*)THREAD_ARG1,&pICP),S_OK);//(IID_IRowsetNotify,&pICP),S_OK);
	
	ThreadSwitch(); //Let the other thread(s) catch up
		
	//Now we can advise the connection from the rowset->pICP to the listener in this thread
	TESTC_(pICP->Advise(pCListener, &dwCookie),S_OK);

	//We need to stall, so that we give enough time for the rowset in the main 
	//thread to generate a notification, so that we pick it up before 
	//closing/unadvising this connection
	Sleep(2000); //Sleep for a couple of seconds, (milliseconds)

	//Now should have recieved the notification by now...
	TESTC(pCListener->GetTimesNotified() > 0);

CLEANUP:
	//Unadvise the connection to the listener
	if(pICP)
		pICP->Unadvise(dwCookie);

	SAFE_RELEASE(pCListener);
	SAFE_RELEASE(pICP);
	SAFE_RELEASE(pICPC);
	
	LeaveCriticalSection(&GlobalMutex);

	THREAD_RETURN
}


BOOL CTestNotify::VerifyNoConnection() 
{
	TBEGIN
	ULONG iListener, iSource;

	//reset all listeners
	for(iListener=0; iListener<m_vectListeners.GetCount(); iListener++)
		TESTC(pCListener(iListener)->ResetTimesNotified());
			
	//cause the source to notify the listener
	for(iSource=0; iSource<m_vectSources.GetCount(); iSource++)
		TESTC(pCSource(iSource)->CauseNotification());
		
	//verify listener recieved notification
	for(iListener=0; iListener<m_vectListeners.GetCount(); iListener++)
		TESTC(pCListener(iListener)->GetTimesNotified() == 0);
		
CLEANUP:	
	TRETURN
}

BOOL CTestNotify::VerifyNoConnection(CListener* pCListener, CSource* pCSource) 
{
	return VerifyConnection(pCListener, pCSource, 0);
}


BOOL CTestNotify::VerifyConnection(CListener* pCListener, CSource* pCSource, ULONG ulTimesConnected) 
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
			if(pCListener == pCConnectInfo->m_pCListener && pCSource == pCConnectInfo->m_pCSource)
				ulTimesConnected++;
		}
	}

	//reset listener's notify count
	TESTC(pCListener->ResetTimesNotified());
	
	//cause the source to notify the listener
	TESTC(pCSource->CauseNotification());
		
	//verify listener recieved notification
	TESTC(pCSource->VerifyNotification(pCListener, ulTimesConnected));

CLEANUP:	
	TRETURN
}

BOOL CTestNotify::VerifyConnectData(ULONG cConnectData, CONNECTDATA* rgConnectData, ULONG cCookies, DWORD* rgCookies)
{
	//Need to verify rgConnectData array contains the correct information
	//NOTE: rgConnectData doesn't have to be ordered direclty with the order
	//that the connections were established.
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
				if(!VerifyEqualInterface(rgConnectData[i].pUnk, pCListener(j)))
					return FALSE;

				ulFound++;
			}
		}
	}

	return ulFound == cConnectData;
}

BOOL CTestNotify::FreeConnectionPoints(ULONG cConnectionPoints, IConnectionPoint** rgpIConnectionPoints)
{
	for(ULONG i=0; i<cConnectionPoints; i++)
	{
		SAFE_RELEASE(rgpIConnectionPoints[i]);
	}
	return TRUE;
}


BOOL CTestNotify::VerifyConnectionPoints(ULONG cConnectionPoints, IConnectionPoint** rgpIConnectionPoints)
{
	TBEGIN
	ULONG i;

	//Sanity Check
	TESTC(cConnectionPoints <= pCSource(0)->GetCountCP());
	
	//Loop over connections and verify
	for(i=0; i<cConnectionPoints; i++)
	{
		//Sanity Check
		TESTC(rgpIConnectionPoints!=NULL && rgpIConnectionPoints[i]!= NULL);
		
		IID iid;
		TESTC_(rgpIConnectionPoints[i]->GetConnectionInterface(&iid),S_OK);
		
		//Make sure their is only 1 occurance of each connection point...
		for(ULONG iPrev=0; iPrev<i; iPrev++)
		{
			IID iidPrev;
			TESTC_(rgpIConnectionPoints[iPrev]->GetConnectionInterface(&iidPrev),S_OK);
			TESTC(iid != iidPrev);
		}

		if(!pCSource(0)->IsOptionalCP(iid))
		{
			TWARNING("Container supports unrecognized Connection Points?" << GetInterfaceName(iid));
		}
	}

CLEANUP:
	TRETURN
}


// {{ TCW_TEST_CASE_MAP(TCAdvise)
//--------------------------------------------------------------------
// @class Test IConnectionPoint::Advise(
//
class TCAdvise : public CTestNotify { 
public:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCAdvise,CTestNotify);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember General - verify reference count
	int Variation_1();
	// @cmember Boundary/NULL - [null, valid] - E_POINTER
	int Variation_2();
	// @cmember Boundary/NULL - [valid, null] - E_POINTER
	int Variation_3();
	// @cmember Boundary/NULL - [invalid sink, valid] - CANNOTCONNECT
	int Variation_4();
	// @cmember Boundary/NULL - Max connections+1 | 20 connections
	int Variation_5();
	// @cmember Usage Context - valid call, (IID_IRowsetNotify with a notify sink
	int Variation_6();
	// @cmember Usage Context - 3 valid calls, (IID_IRowsetNotify with a notify sink
	int Variation_7();
	// @cmember Usage Context - invalid call, (IID_ISomeOther with a notify sink
	int Variation_8();
	// @cmember Usage Context - valid / invalvid / valid calls
	int Variation_9();
	// @cmember Multi-Threaded - 2 Sinks / 1 Sources (Sep Threads
	int Variation_10();
	// @cmember Multi-Threaded - 2 Sinks / 3 Sources (Sep Threads
	int Variation_11();
	// @cmember Multi-Threaded - 2 Sinks / 1 Source (Sep DB Sessions
	int Variation_12();
	// @cmember Multi-Threaded - 2 Sinks / 3 Source (Sep DB Sessions
	int Variation_13();
	// @cmember Multi-User - 1 Sink / 1 Source
	int Variation_14();
	// @cmember Multi-User - 2 Sinks / 1 Source
	int Variation_15();
	// @cmember Multi-User - 1 Sink / 3 Sources
	int Variation_16();
	// @cmember Multi-User - 2 Sinks / 3 Sources
	int Variation_17();
	// @cmember RefCount - Release of Rowset
	int Variation_18();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCAdvise)
#define THE_CLASS TCAdvise
BEG_TEST_CASE(TCAdvise, CTestNotify, L"Test IConnectionPoint::Advise(")
	TEST_VARIATION(1, 		L"General - verify reference count")
	TEST_VARIATION(2, 		L"Boundary/NULL - [null, valid] - E_POINTER")
	TEST_VARIATION(3, 		L"Boundary/NULL - [valid, null] - E_POINTER")
	TEST_VARIATION(4, 		L"Boundary/NULL - [invalid sink, valid] - CANNOTCONNECT")
	TEST_VARIATION(5, 		L"Boundary/NULL - Max connections+1 | 20 connections")
	TEST_VARIATION(6, 		L"Usage Context - valid call, (IID_IRowsetNotify with a notify sink")
	TEST_VARIATION(7, 		L"Usage Context - 3 valid calls, (IID_IRowsetNotify with a notify sink")
	TEST_VARIATION(8, 		L"Usage Context - invalid call, (IID_ISomeOther with a notify sink")
	TEST_VARIATION(9, 		L"Usage Context - valid / invalvid / valid calls")
	TEST_VARIATION(10, 		L"Multi-Threaded - 2 Sinks / 1 Sources (Sep Threads")
	TEST_VARIATION(11, 		L"Multi-Threaded - 2 Sinks / 3 Sources (Sep Threads")
	TEST_VARIATION(12, 		L"Multi-Threaded - 2 Sinks / 1 Source (Sep DB Sessions")
	TEST_VARIATION(13, 		L"Multi-Threaded - 2 Sinks / 3 Source (Sep DB Sessions")
	TEST_VARIATION(14, 		L"Multi-User - 1 Sink / 1 Source")
	TEST_VARIATION(15, 		L"Multi-User - 2 Sinks / 1 Source")
	TEST_VARIATION(16, 		L"Multi-User - 1 Sink / 3 Sources")
	TEST_VARIATION(17, 		L"Multi-User - 2 Sinks / 3 Sources")
	TEST_VARIATION(18, 		L"RefCount - Release of Rowset")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCUnadvise)
//--------------------------------------------------------------------
// @class Test IConnectionPoint::Unadvise(
//
class TCUnadvise : public CTestNotify { 
public:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCUnadvise,CTestNotify);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember General - verify reference count
	int Variation_1();
	// @cmember Boundary/NULL - [null] - NOCONNECTION
	int Variation_2();
	// @cmember Boundary/NULL - Unadvise before any Advises
	int Variation_3();
	// @cmember Sequence - Advise/Unadivse 12 variations
	int Variation_4();
	// @cmember Transactions - Enabled
	int Variation_5();
	// @cmember Transactions - Disabled
	int Variation_6();
	// @cmember Multi-Threaed - 1 Sink / 3 Sources (Sep Threads
	int Variation_7();
	// @cmember Multi-Threaed - 3 Sinks / 3 Sources (Sep Threads
	int Variation_8();
	// @cmember Multi-Threaed - 1 Sink / 3 Sources (Sep DB Sessions
	int Variation_9();
	// @cmember Multi-Threaed - 3 Sinks / 3 Sources (Sep DB Sessions
	int Variation_10();
	// @cmember Multi-User - 1 Sink / 1 Source
	int Variation_11();
	// @cmember Multi-User - 3 Sinks / 1 Source
	int Variation_12();
	// @cmember Multi-User - 1 Sink / 3 Sources
	int Variation_13();
	// @cmember Multi-User - 3 Sinks / 3 Sources
	int Variation_14();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCUnadvise)
#define THE_CLASS TCUnadvise
BEG_TEST_CASE(TCUnadvise, CTestNotify, L"Test IConnectionPoint::Unadvise(")
	TEST_VARIATION(1, 		L"General - verify reference count")
	TEST_VARIATION(2, 		L"Boundary/NULL - [null] - NOCONNECTION")
	TEST_VARIATION(3, 		L"Boundary/NULL - Unadvise before any Advises")
	TEST_VARIATION(4, 		L"Sequence - Advise/Unadivse 12 variations")
	TEST_VARIATION(5, 		L"Transactions - Enabled")
	TEST_VARIATION(6, 		L"Transactions - Disabled")
	TEST_VARIATION(7, 		L"Multi-Threaed - 1 Sink / 3 Sources (Sep Threads")
	TEST_VARIATION(8, 		L"Multi-Threaed - 3 Sinks / 3 Sources (Sep Threads")
	TEST_VARIATION(9, 		L"Multi-Threaed - 1 Sink / 3 Sources (Sep DB Sessions")
	TEST_VARIATION(10, 		L"Multi-Threaed - 3 Sinks / 3 Sources (Sep DB Sessions")
	TEST_VARIATION(11, 		L"Multi-User - 1 Sink / 1 Source")
	TEST_VARIATION(12, 		L"Multi-User - 3 Sinks / 1 Source")
	TEST_VARIATION(13, 		L"Multi-User - 1 Sink / 3 Sources")
	TEST_VARIATION(14, 		L"Multi-User - 3 Sinks / 3 Sources")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCGetConnectionInterface)
//--------------------------------------------------------------------
// @class Test IConnectionPoint::GetConnectionInterface
//
class TCGetConnectionInterface : public CTestNotify { 
public:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCGetConnectionInterface,CTestNotify);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember General - enum over the CPC and verify method
	int Variation_1();
	// @cmember Boundary/NULL - pIID == NULL
	int Variation_2();
	// @cmember Multi-User - 2 Sinks / 3 Sources verify
	int Variation_3();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCGetConnectionInterface)
#define THE_CLASS TCGetConnectionInterface
BEG_TEST_CASE(TCGetConnectionInterface, CTestNotify, L"Test IConnectionPoint::GetConnectionInterface")
	TEST_VARIATION(1, 		L"General - enum over the CPC and verify method")
	TEST_VARIATION(2, 		L"Boundary/NULL - pIID == NULL")
	TEST_VARIATION(3, 		L"Multi-User - 2 Sinks / 3 Sources verify")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCGetConnectionPointContainer)
//--------------------------------------------------------------------
// @class Test IConnectionPoint::GetConnectionPointContainer
//
class TCGetConnectionPointContainer : public CTestNotify { 
public:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCGetConnectionPointContainer,CTestNotify);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember General - verify reference count
	int Variation_1();
	// @cmember General - Enum over the CP and verify correct container
	int Variation_2();
	// @cmember General - call release and verify ref count
	int Variation_3();
	// @cmember Boundary/NULL - ppICPC NULL - E_POINTER
	int Variation_4();
	// @cmember Multi-User - 2 Sinks / 3 Sources and verify
	int Variation_5();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCGetConnectionPointContainer)
#define THE_CLASS TCGetConnectionPointContainer
BEG_TEST_CASE(TCGetConnectionPointContainer, CTestNotify, L"Test IConnectionPoint::GetConnectionPointContainer")
	TEST_VARIATION(1, 		L"General - verify reference count")
	TEST_VARIATION(2, 		L"General - Enum over the CP and verify correct container")
	TEST_VARIATION(3, 		L"General - call release and verify ref count")
	TEST_VARIATION(4, 		L"Boundary/NULL - ppICPC NULL - E_POINTER")
	TEST_VARIATION(5, 		L"Multi-User - 2 Sinks / 3 Sources and verify")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCEnumConnections)
//--------------------------------------------------------------------
// @class Test IConnectionPoint::EnumConnections
//
class TCEnumConnections : public CTestNotify { 
public:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCEnumConnections,CTestNotify);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember General - verify reference count
	int Variation_1();
	// @cmember Boundary/NULL - ppEnum NULL - E_POINTER
	int Variation_2();
	// @cmember Boundary/NULL - Enum over No Connections
	int Variation_3();
	// @cmember Boundary/NULL - Enum over many connections
	int Variation_4();
	// @cmember Boundary/NULL - Enum on a non-supporting Enum CP
	int Variation_5();
	// @cmember Enum::Next[0, NULL, NULL] - no ele
	int Variation_6();
	// @cmember Enum::Next[0, valid, valid] - no ele
	int Variation_7();
	// @cmember Enum::Next[1, valid, NULL] - no ele
	int Variation_8();
	// @cmember Enum::Next[1, NULL, NULL] - no ele
	int Variation_9();
	// @cmember Enum::Next[3, valid, NULL] - no ele
	int Variation_10();
	// @cmember Enum::Next[ULONG_MAX, valid, valid] - no ele
	int Variation_11();
	// @cmember Enum::Next[valid, valid, valid] - no ele
	int Variation_12();
	// @cmember Enum::Next[0, valid, NULL] - no ele
	int Variation_13();
	// @cmember Enum::Next[1, NULL, valid] - no ele
	int Variation_14();
	// @cmember Enum::Next[1, valid, NULL] - 1 ele
	int Variation_15();
	// @cmember Enum::Next[N+1, valid, valid] - 5 ele
	int Variation_16();
	// @cmember Enum::Next[Next seperatly over list] - 5 ele
	int Variation_17();
	// @cmember Enum::Next[Next group of all 5] - 5 ele
	int Variation_18();
	// @cmember Enum::Next[sequence of both seperately and group] - N ele
	int Variation_19();
	// @cmember Enum::Skip[0] - no ele - E_INVALIDARG
	int Variation_20();
	// @cmember Enum::Skip[ULONG_MAX] - no ele - S_FALSE
	int Variation_21();
	// @cmember Enum::Skip[N] - no ele - S_FALSE
	int Variation_22();
	// @cmember Enum::Skip[N+1] - no ele - S_FALSE
	int Variation_23();
	// @cmember Enum::Skip[0] - N ele - E_INVALIDARG
	int Variation_24();
	// @cmember Enum::Skip[ULONG_MAX] - N ele - S_FALSE
	int Variation_25();
	// @cmember Enum::Skip[N] - N ele - S_OK
	int Variation_26();
	// @cmember Enum::Skip[N-1] - N ele - S_OK
	int Variation_27();
	// @cmember Enum::Skip[N+1] - N ele - S_FALSE
	int Variation_28();
	// @cmember Enum::Clone[NULL] - E_POINTER
	int Variation_29();
	// @cmember Enum::Clone[valid] - no ele - S_OK
	int Variation_30();
	// @cmember Enum::Clone[valid] - N ele - S_OK
	int Variation_31();
	// @cmember Sequence - Enum sequence testing
	int Variation_32();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCEnumConnections)
#define THE_CLASS TCEnumConnections
BEG_TEST_CASE(TCEnumConnections, CTestNotify, L"Test IConnectionPoint::EnumConnections")
	TEST_VARIATION(1, 		L"General - verify reference count")
	TEST_VARIATION(2, 		L"Boundary/NULL - ppEnum NULL - E_POINTER")
	TEST_VARIATION(3, 		L"Boundary/NULL - Enum over No Connections")
	TEST_VARIATION(4, 		L"Boundary/NULL - Enum over many connections")
	TEST_VARIATION(5, 		L"Boundary/NULL - Enum on a non-supporting Enum CP")
	TEST_VARIATION(6, 		L"Enum::Next[0, NULL, NULL] - no ele")
	TEST_VARIATION(7, 		L"Enum::Next[0, valid, valid] - no ele")
	TEST_VARIATION(8, 		L"Enum::Next[1, valid, NULL] - no ele")
	TEST_VARIATION(9, 		L"Enum::Next[1, NULL, NULL] - no ele")
	TEST_VARIATION(10, 		L"Enum::Next[3, valid, NULL] - no ele")
	TEST_VARIATION(11, 		L"Enum::Next[ULONG_MAX, valid, valid] - no ele")
	TEST_VARIATION(12, 		L"Enum::Next[valid, valid, valid] - no ele")
	TEST_VARIATION(13, 		L"Enum::Next[0, valid, NULL] - no ele")
	TEST_VARIATION(14, 		L"Enum::Next[1, NULL, valid] - no ele")
	TEST_VARIATION(15, 		L"Enum::Next[1, valid, NULL] - 1 ele")
	TEST_VARIATION(16, 		L"Enum::Next[N+1, valid, valid] - 5 ele")
	TEST_VARIATION(17, 		L"Enum::Next[Next seperatly over list] - 5 ele")
	TEST_VARIATION(18, 		L"Enum::Next[Next group of all 5] - 5 ele")
	TEST_VARIATION(19, 		L"Enum::Next[sequence of both seperately and group] - N ele")
	TEST_VARIATION(20, 		L"Enum::Skip[0] - no ele - E_INVALIDARG")
	TEST_VARIATION(21, 		L"Enum::Skip[ULONG_MAX] - no ele - S_FALSE")
	TEST_VARIATION(22, 		L"Enum::Skip[N] - no ele - S_FALSE")
	TEST_VARIATION(23, 		L"Enum::Skip[N+1] - no ele - S_FALSE")
	TEST_VARIATION(24, 		L"Enum::Skip[0] - N ele - E_INVALIDARG")
	TEST_VARIATION(25, 		L"Enum::Skip[ULONG_MAX] - N ele - S_FALSE")
	TEST_VARIATION(26, 		L"Enum::Skip[N] - N ele - S_OK")
	TEST_VARIATION(27, 		L"Enum::Skip[N-1] - N ele - S_OK")
	TEST_VARIATION(28, 		L"Enum::Skip[N+1] - N ele - S_FALSE")
	TEST_VARIATION(29, 		L"Enum::Clone[NULL] - E_POINTER")
	TEST_VARIATION(30, 		L"Enum::Clone[valid] - no ele - S_OK")
	TEST_VARIATION(31, 		L"Enum::Clone[valid] - N ele - S_OK")
	TEST_VARIATION(32, 		L"Sequence - Enum sequence testing")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCEnumConnectionPoints)
//--------------------------------------------------------------------
// @class Test IConnectionPointContainer::EnumConnectionPoints
//
class TCEnumConnectionPoints : public CTestNotify { 
public:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCEnumConnectionPoints,CTestNotify);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember General - verify reference count
	int Variation_1();
	// @cmember General - verify QueryInterface for every connection point
	int Variation_2();
	// @cmember Boundary/NULL - ppEnum NULL E_POINTER
	int Variation_3();
	// @cmember EnumCP::Next[0,NULL,NULL]
	int Variation_4();
	// @cmember EnumCP::Next[0, valid, valid]
	int Variation_5();
	// @cmember EnumCP::Next[1, valid, NULL]
	int Variation_6();
	// @cmember EnumCP::Next[1, NULL, NULL]
	int Variation_7();
	// @cmember EnumCP::Next[ULONG_MAX, valid, valid]
	int Variation_8();
	// @cmember EnumCP::Next[valid, valid, valid]
	int Variation_9();
	// @cmember EnumCP::Next[0, valid, NULL]
	int Variation_10();
	// @cmember EnumCP::Next[1, NULL, valid]
	int Variation_11();
	// @cmember EnumCP::Next[N+1, valid, valid]
	int Variation_12();
	// @cmember EnumCP::Next[seperately]
	int Variation_13();
	// @cmember EnumCP::Next[group]
	int Variation_14();
	// @cmember EnumCP::Next[sequence]
	int Variation_15();
	// @cmember EnumCP::Skip[0]
	int Variation_16();
	// @cmember EnumCP::Skip[ULONG_MAX]
	int Variation_17();
	// @cmember EnumCP::Skip[N]
	int Variation_18();
	// @cmember EnumCP::Skip[N+1]
	int Variation_19();
	// @cmember EnumCP::Skip[N-1]
	int Variation_20();
	// @cmember EnumCP::Clone[NULL]
	int Variation_21();
	// @cmember EnumCP::Clone[valid]
	int Variation_22();
	// @cmember Sequence - EnumCP sequence testing
	int Variation_23();
	// @cmember General - verify correct IID returned
	int Variation_24();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCEnumConnectionPoints)
#define THE_CLASS TCEnumConnectionPoints
BEG_TEST_CASE(TCEnumConnectionPoints, CTestNotify, L"Test IConnectionPointContainer::EnumConnectionPoints")
	TEST_VARIATION(1, 		L"General - verify reference count")
	TEST_VARIATION(2, 		L"General - verify QueryInterface for every connection point")
	TEST_VARIATION(3, 		L"Boundary/NULL - ppEnum NULL E_POINTER")
	TEST_VARIATION(4, 		L"EnumCP::Next[0,NULL,NULL]")
	TEST_VARIATION(5, 		L"EnumCP::Next[0, valid, valid]")
	TEST_VARIATION(6, 		L"EnumCP::Next[1, valid, NULL]")
	TEST_VARIATION(7, 		L"EnumCP::Next[1, NULL, NULL]")
	TEST_VARIATION(8, 		L"EnumCP::Next[ULONG_MAX, valid, valid]")
	TEST_VARIATION(9, 		L"EnumCP::Next[valid, valid, valid]")
	TEST_VARIATION(10, 		L"EnumCP::Next[0, valid, NULL]")
	TEST_VARIATION(11, 		L"EnumCP::Next[1, NULL, valid]")
	TEST_VARIATION(12, 		L"EnumCP::Next[N+1, valid, valid]")
	TEST_VARIATION(13, 		L"EnumCP::Next[seperately]")
	TEST_VARIATION(14, 		L"EnumCP::Next[group]")
	TEST_VARIATION(15, 		L"EnumCP::Next[sequence]")
	TEST_VARIATION(16, 		L"EnumCP::Skip[0]")
	TEST_VARIATION(17, 		L"EnumCP::Skip[ULONG_MAX]")
	TEST_VARIATION(18, 		L"EnumCP::Skip[N]")
	TEST_VARIATION(19, 		L"EnumCP::Skip[N+1]")
	TEST_VARIATION(20, 		L"EnumCP::Skip[N-1]")
	TEST_VARIATION(21, 		L"EnumCP::Clone[NULL]")
	TEST_VARIATION(22, 		L"EnumCP::Clone[valid]")
	TEST_VARIATION(23, 		L"Sequence - EnumCP sequence testing")
	TEST_VARIATION(24, 		L"General - verify correct IID returned")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCFindConnectionPoint)
//--------------------------------------------------------------------
// @class Test IConnectionPointInterface::FindConnectionPoint
//
class TCFindConnectionPoint : public CTestNotify { 
public:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCFindConnectionPoint,CTestNotify);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember General - verify reference count
	int Variation_1();
	// @cmember Empty
	int Variation_2();
	// @cmember General - call repeatedly and verify reference count
	int Variation_3();
	// @cmember Boundary/NULL - riid NULL CONNECT_E_NOCONNECTION
	int Variation_4();
	// @cmember Boundary/NULL - ppICP E_POINTER
	int Variation_5();
	// @cmember Boundary/NULL - verify valid / invalid CP
	int Variation_6();
	// @cmember Multi-User - 2 Sinks / 1 Source verify IID returned
	int Variation_7();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCFindConnectionPoint)
#define THE_CLASS TCFindConnectionPoint
BEG_TEST_CASE(TCFindConnectionPoint, CTestNotify, L"Test IConnectionPointInterface::FindConnectionPoint")
	TEST_VARIATION(1, 		L"General - verify reference count")
	TEST_VARIATION(2, 		L"Empty")
	TEST_VARIATION(3, 		L"General - call repeatedly and verify reference count")
	TEST_VARIATION(4, 		L"Boundary/NULL - riid NULL CONNECT_E_NOCONNECTION")
	TEST_VARIATION(5, 		L"Boundary/NULL - ppICP E_POINTER")
	TEST_VARIATION(6, 		L"Boundary/NULL - verify valid / invalid CP")
	TEST_VARIATION(7, 		L"Multi-User - 2 Sinks / 1 Source verify IID returned")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCNotReEntrant)
//--------------------------------------------------------------------
// @class Test all interfaces that will return DB_E_NOTREENTRANT
//
class TCNotReEntrant : public CTestNotify { 
public:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCNotReEntrant,CTestNotify);
	// }} TCW_DECLARE_FUNCS_END

	//IRowset - reentrantcy methods
	static HRESULT RE_AddRefRows     (CRowset* pCRowset, IRowset* pIRowset, ULONG cRows, const HROW rghRows[], ULONG cColumns, const ULONG rgColumns[]); 
	static HRESULT RE_GetData        (CRowset* pCRowset, IRowset* pIRowset, ULONG cRows, const HROW rghRows[], ULONG cColumns, const ULONG rgColumns[]); 
	static HRESULT RE_GetNextRows    (CRowset* pCRowset, IRowset* pIRowset, ULONG cRows, const HROW rghRows[], ULONG cColumns, const ULONG rgColumns[]); 
	static HRESULT RE_ReleaseRows    (CRowset* pCRowset, IRowset* pIRowset, ULONG cRows, const HROW rghRows[], ULONG cColumns, const ULONG rgColumns[]); 
	static HRESULT RE_RestartPosition(CRowset* pCRowset, IRowset* pIRowset, ULONG cRows, const HROW rghRows[], ULONG cColumns, const ULONG rgColumns[]); 

	//IRowsetChange - reentrantcy methods
	static HRESULT RE_DeleteRows     (CRowset* pCRowset, IRowset* pIRowset, ULONG cRows, const HROW rghRows[], ULONG cColumns, const ULONG rgColumns[]); 
	static HRESULT RE_InsertRow      (CRowset* pCRowset, IRowset* pIRowset, ULONG cRows, const HROW rghRows[], ULONG cColumns, const ULONG rgColumns[]); 
	static HRESULT RE_SetData        (CRowset* pCRowset, IRowset* pIRowset, ULONG cRows, const HROW rghRows[], ULONG cColumns, const ULONG rgColumns[]); 

	//IRowsetUpdate - reentrantcy methods
	static HRESULT RE_GetOriginalData(CRowset* pCRowset, IRowset* pIRowset, ULONG cRows, const HROW rghRows[], ULONG cColumns, const ULONG rgColumns[]); 
	static HRESULT RE_GetPendingRows (CRowset* pCRowset, IRowset* pIRowset, ULONG cRows, const HROW rghRows[], ULONG cColumns, const ULONG rgColumns[]); 
	static HRESULT RE_GetRowStatus   (CRowset* pCRowset, IRowset* pIRowset, ULONG cRows, const HROW rghRows[], ULONG cColumns, const ULONG rgColumns[]); 
	static HRESULT RE_Undo           (CRowset* pCRowset, IRowset* pIRowset, ULONG cRows, const HROW rghRows[], ULONG cColumns, const ULONG rgColumns[]); 
	static HRESULT RE_Update         (CRowset* pCRowset, IRowset* pIRowset, ULONG cRows, const HROW rghRows[], ULONG cColumns, const ULONG rgColumns[]); 
	

	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember IRowset
	int Variation_1();
	// @cmember IRowsetChange
	int Variation_2();
	// @cmember IRowsetUpdate
	int Variation_3();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCNotReEntrant)
#define THE_CLASS TCNotReEntrant
BEG_TEST_CASE(TCNotReEntrant, CTestNotify, L"Test all interfaces that will return DB_E_NOTREENTRANT")
	TEST_VARIATION(1, 		L"IRowset")
	TEST_VARIATION(2, 		L"IRowsetChange")
	TEST_VARIATION(3, 		L"IRowsetUpdate")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCZombie)
//--------------------------------------------------------------------
// @class Test the zombie cases for ICPC/ICP
//
class TCZombie : public TCTransaction { 
public:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCZombie,TCTransaction);
	// }} TCW_DECLARE_FUNCS_END
 
	ULONG m_cPropSets;
	DBPROPSET* m_rgPropSets;

	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Zombie - ABORT with fRetaining == TRUE
	int Variation_1();
	// @cmember Zombie - ABORT with fRetaining == FALSE
	int Variation_2();
	// @cmember Zombie - COMMIT with fRetaining == TRUE
	int Variation_3();
	// @cmember Zombie - COMMIT with fRetaining == FALSE
	int Variation_4();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCZombie)
#define THE_CLASS TCZombie
BEG_TEST_CASE(TCZombie, TCTransaction, L"Test the zombie cases for ICPC/ICP")
	TEST_VARIATION(1, 		L"Zombie - ABORT with fRetaining == TRUE")
	TEST_VARIATION(2, 		L"Zombie - ABORT with fRetaining == FALSE")
	TEST_VARIATION(3, 		L"Zombie - COMMIT with fRetaining == TRUE")
	TEST_VARIATION(4, 		L"Zombie - COMMIT with fRetaining == FALSE")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCProperties)
//--------------------------------------------------------------------
// @class Test all of the Notifcation properties
//
class TCProperties : public CTestNotify { 
public:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCProperties,CTestNotify);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Properties - DBPROP_IConnectionPointContainer
	int Variation_1();
	// @cmember Properties - DBPROP_NOTIFICATIONPHASES
	int Variation_2();
	// @cmember Properties - DBPROP_NOTIFIY [Reasons]
	int Variation_3();
	// @cmember Properties - DBPROP_REENTRANTEVENTS
	int Variation_4();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCProperties)
#define THE_CLASS TCProperties
BEG_TEST_CASE(TCProperties, CTestNotify, L"Test all of the Notifcation properties")
	TEST_VARIATION(1, 		L"Properties - DBPROP_IConnectionPointContainer")
	TEST_VARIATION(2, 		L"Properties - DBPROP_NOTIFICATIONPHASES")
	TEST_VARIATION(3, 		L"Properties - DBPROP_NOTIFIY [Reasons]")
	TEST_VARIATION(4, 		L"Properties - DBPROP_REENTRANTEVENTS")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCNotifyCanceled)
//--------------------------------------------------------------------
// @class Test CANCELED notification
//
class TCNotifyCanceled : public CTestNotify { 
public:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCNotifyCanceled,CTestNotify);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember CANCELED - IRowset
	int Variation_1();
	// @cmember CANCELED - IRowsetChange
	int Variation_2();
	// @cmember CANCELED - IRowsetUpdate
	int Variation_3();
	// @cmember CANCELED - IRowsetLocate
	int Variation_4();
	// @cmember CANCELED - IRowsetResynch
	int Variation_5();
	// @cmember CANCELED - IRowsetScroll
	int Variation_6();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCNotifyCanceled)
#define THE_CLASS TCNotifyCanceled
BEG_TEST_CASE(TCNotifyCanceled, CTestNotify, L"Test CANCELED notifications")
	TEST_VARIATION(1, 		L"CANCELED - IRowset")
	TEST_VARIATION(2, 		L"CANCELED - IRowsetChange")
	TEST_VARIATION(3, 		L"CANCELED - IRowsetUpdate")
	TEST_VARIATION(4, 		L"CANCELED - IRowsetLocate")
	TEST_VARIATION(5, 		L"CANCELED - IRowsetResynch")
	TEST_VARIATION(6, 		L"CANCELED - IRowsetScroll")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCNotifyFailure)
//--------------------------------------------------------------------
// @class Test E_FAIL from a listener
//
class TCNotifyFailure : public CTestNotify { 
public:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCNotifyFailure,CTestNotify);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember E_FAIL - IRowset
	int Variation_1();
	// @cmember E_FAIL - IRowsetChange
	int Variation_2();
	// @cmember E_FAIL - IRowsetUpdate
	int Variation_3();
	// @cmember E_FAIL - IRowsetLocate
	int Variation_4();
	// @cmember E_FAIL - IRowsetResynch
	int Variation_5();
	// @cmember E_FAIL - IRowsetScroll
	int Variation_6();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCNotifyFailure)
#define THE_CLASS TCNotifyFailure
BEG_TEST_CASE(TCNotifyFailure, CTestNotify, L"Test E_FAIL from a listener")
	TEST_VARIATION(1, 		L"E_FAIL - IRowset")
	TEST_VARIATION(2, 		L"E_FAIL - IRowsetChange")
	TEST_VARIATION(3, 		L"E_FAIL - IRowsetUpdate")
	TEST_VARIATION(4, 		L"E_FAIL - IRowsetLocate")
	TEST_VARIATION(5, 		L"E_FAIL - IRowsetResynch")
	TEST_VARIATION(6, 		L"E_FAIL - IRowsetScroll")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// }} END_DECLARE_TEST_CASES()


////////////////////////////////////////////////////////////////////////
// TCStream_General
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


#define TEST_CASE_WITH_PARAM(iCase, theClass, param1, param2)			\
    case iCase:													\
		pCTestCase = new theClass(NULL);						\
		((theClass*)pCTestCase)->SetTestCaseParams(param1, param2);		\
		pCTestCase->SetOwningMod(iCase-1, pCThisTestModule);	\
		return pCTestCase;


//TCRowset
COPY_TEST_CASE(TCRowset_Advise_IRowsetNotify,							TCAdvise)
COPY_TEST_CASE(TCRowset_Unadvise_IRowsetNotify,						TCUnadvise)
COPY_TEST_CASE(TCRowset_GetConnectionInterface_IRowsetNotify,			TCGetConnectionInterface)
COPY_TEST_CASE(TCRowset_GetConnectionPointContainer_IRowsetNotify,	TCGetConnectionPointContainer)
COPY_TEST_CASE(TCRowset_EnumConnections_IRowsetNotify,				TCEnumConnections)
COPY_TEST_CASE(TCRowset_EnumConnectionPoints_IRowsetNotify,			TCEnumConnectionPoints)
COPY_TEST_CASE(TCRowset_FindConnectionPoint_IRowsetNotify,			TCFindConnectionPoint)
COPY_TEST_CASE(TCRowset_NotReEntrant_IRowsetNotify,					TCNotReEntrant)
COPY_TEST_CASE(TCRowset_Zombie_IRowsetNotify,							TCZombie)
COPY_TEST_CASE(TCRowset_Properties_IRowsetNotify,						TCProperties)
COPY_TEST_CASE(TCRowset_NotifyCanceled_IRowsetNotify,					TCNotifyCanceled)
COPY_TEST_CASE(TCRowset_NotifyFailure_IRowsetNotify,					TCNotifyFailure)

COPY_TEST_CASE(TCRowset_Advise_IDBAsynchNotify,							TCAdvise)
COPY_TEST_CASE(TCRowset_Unadvise_IDBAsynchNotify,						TCUnadvise)
COPY_TEST_CASE(TCRowset_GetConnectionInterface_IDBAsynchNotify,			TCGetConnectionInterface)
COPY_TEST_CASE(TCRowset_GetConnectionPointContainer_IDBAsynchNotify,	TCGetConnectionPointContainer)
COPY_TEST_CASE(TCRowset_EnumConnections_IDBAsynchNotify,				TCEnumConnections)
COPY_TEST_CASE(TCRowset_FindConnectionPoint_IDBAsynchNotify,			TCFindConnectionPoint)

//TCDataSource
COPY_TEST_CASE(TCDataSource_Advise,						TCAdvise)
COPY_TEST_CASE(TCDataSource_Unadvise,					TCUnadvise)
COPY_TEST_CASE(TCDataSource_GetConnectionInterface,		TCGetConnectionInterface)
COPY_TEST_CASE(TCDataSource_GetConnectionPointContainer,TCGetConnectionPointContainer)
COPY_TEST_CASE(TCDataSource_EnumConnections,			TCEnumConnections)
COPY_TEST_CASE(TCDataSource_EnumConnectionPoints,		TCEnumConnectionPoints)
COPY_TEST_CASE(TCDataSource_FindConnectionPoint,		TCFindConnectionPoint)

//TCRow
COPY_TEST_CASE(TCRow_Advise,							TCAdvise)
COPY_TEST_CASE(TCRow_Unadvise,							TCUnadvise)
COPY_TEST_CASE(TCRow_GetConnectionInterface,			TCGetConnectionInterface)
COPY_TEST_CASE(TCRow_GetConnectionPointContainer,		TCGetConnectionPointContainer)
COPY_TEST_CASE(TCRow_EnumConnections,					TCEnumConnections)
COPY_TEST_CASE(TCRow_EnumConnectionPoints,				TCEnumConnectionPoints)
COPY_TEST_CASE(TCRow_FindConnectionPoint,				TCFindConnectionPoint)

//TCStream
COPY_TEST_CASE(TCStream_Advise,							TCAdvise)
COPY_TEST_CASE(TCStream_Unadvise,						TCUnadvise)
COPY_TEST_CASE(TCStream_GetConnectionInterface,			TCGetConnectionInterface)
COPY_TEST_CASE(TCStream_GetConnectionPointContainer,	TCGetConnectionPointContainer)
COPY_TEST_CASE(TCStream_EnumConnections,				TCEnumConnections)
COPY_TEST_CASE(TCStream_EnumConnectionPoints,			TCEnumConnectionPoints)
COPY_TEST_CASE(TCStream_FindConnectionPoint,			TCFindConnectionPoint)


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
TEST_MODULE(12, ThisModule, gwszModuleDescrip)
	TEST_CASE(1, TCAdvise)
	TEST_CASE(2, TCUnadvise)
	TEST_CASE(3, TCGetConnectionInterface)
	TEST_CASE(4, TCGetConnectionPointContainer)
	TEST_CASE(5, TCEnumConnections)
	TEST_CASE(6, TCEnumConnectionPoints)
	TEST_CASE(7, TCFindConnectionPoint)
	TEST_CASE(8, TCNotReEntrant)
	TEST_CASE(9, TCZombie)
	TEST_CASE(10, TCProperties)
	TEST_CASE(11, TCNotifyCanceled)
	TEST_CASE(12, TCNotifyFailure)
END_TEST_MODULE()
// }} TCW_TESTMODULE_END
#else
TEST_MODULE(39, ThisModule, gwszModuleDescrip)
	//Rowset - IConnectionPoint: IRowsetNotify, IDBAsynchNotify
	TEST_CASE_WITH_PARAM(1,		TCRowset_Advise_IRowsetNotify,							ROWSET_INTERFACE, IID_IRowsetNotify)
	TEST_CASE_WITH_PARAM(2,		TCRowset_Unadvise_IRowsetNotify,							ROWSET_INTERFACE, IID_IRowsetNotify)
	TEST_CASE_WITH_PARAM(3,		TCRowset_GetConnectionInterface_IRowsetNotify,			ROWSET_INTERFACE, IID_IRowsetNotify)
	TEST_CASE_WITH_PARAM(4,		TCRowset_GetConnectionPointContainer_IRowsetNotify,		ROWSET_INTERFACE, IID_IRowsetNotify)
	TEST_CASE_WITH_PARAM(5,		TCRowset_EnumConnections_IRowsetNotify,					ROWSET_INTERFACE, IID_IRowsetNotify)
	TEST_CASE_WITH_PARAM(6,		TCRowset_EnumConnectionPoints_IRowsetNotify,				ROWSET_INTERFACE, IID_IRowsetNotify)
	TEST_CASE_WITH_PARAM(7,		TCRowset_FindConnectionPoint_IRowsetNotify,				ROWSET_INTERFACE, IID_IRowsetNotify)
	TEST_CASE_WITH_PARAM(8,		TCRowset_NotReEntrant_IRowsetNotify,						ROWSET_INTERFACE, IID_IRowsetNotify)
	TEST_CASE_WITH_PARAM(9,		TCRowset_Zombie_IRowsetNotify,							ROWSET_INTERFACE, IID_IRowsetNotify)
	TEST_CASE_WITH_PARAM(10,	TCRowset_Properties_IRowsetNotify,						ROWSET_INTERFACE, IID_IRowsetNotify)
	TEST_CASE_WITH_PARAM(11,	TCRowset_NotifyCanceled_IRowsetNotify,					ROWSET_INTERFACE, IID_IRowsetNotify)
	TEST_CASE_WITH_PARAM(12,	TCRowset_NotifyFailure_IRowsetNotify,						ROWSET_INTERFACE, IID_IRowsetNotify)

	TEST_CASE_WITH_PARAM(13,	TCRowset_Advise_IDBAsynchNotify,							ROWSET_INTERFACE, IID_IDBAsynchNotify)
	TEST_CASE_WITH_PARAM(14,	TCRowset_Unadvise_IDBAsynchNotify,							ROWSET_INTERFACE, IID_IDBAsynchNotify)
	TEST_CASE_WITH_PARAM(15,	TCRowset_GetConnectionInterface_IDBAsynchNotify,			ROWSET_INTERFACE, IID_IDBAsynchNotify)
	TEST_CASE_WITH_PARAM(16,	TCRowset_GetConnectionPointContainer_IDBAsynchNotify,		ROWSET_INTERFACE, IID_IDBAsynchNotify)
	TEST_CASE_WITH_PARAM(17,	TCRowset_EnumConnections_IDBAsynchNotify,					ROWSET_INTERFACE, IID_IDBAsynchNotify)
	TEST_CASE_WITH_PARAM(18,	TCRowset_FindConnectionPoint_IDBAsynchNotify,				ROWSET_INTERFACE, IID_IDBAsynchNotify)

	//DataSource - IConnectionPoint
	TEST_CASE_WITH_PARAM(19,	TCDataSource_Advise,						DATASOURCE_INTERFACE, IID_IDBAsynchNotify)
	TEST_CASE_WITH_PARAM(20,	TCDataSource_Unadvise,						DATASOURCE_INTERFACE, IID_IDBAsynchNotify)
	TEST_CASE_WITH_PARAM(21,	TCDataSource_GetConnectionInterface,		DATASOURCE_INTERFACE, IID_IDBAsynchNotify)
	TEST_CASE_WITH_PARAM(22,	TCDataSource_GetConnectionPointContainer,	DATASOURCE_INTERFACE, IID_IDBAsynchNotify)
	TEST_CASE_WITH_PARAM(23,	TCDataSource_EnumConnections,				DATASOURCE_INTERFACE, IID_IDBAsynchNotify)
	TEST_CASE_WITH_PARAM(24,	TCDataSource_EnumConnectionPoints,			DATASOURCE_INTERFACE, IID_IDBAsynchNotify)
	TEST_CASE_WITH_PARAM(25,	TCDataSource_FindConnectionPoint,			DATASOURCE_INTERFACE, IID_IDBAsynchNotify)

	//Row - IConnectionPoint
	TEST_CASE_WITH_PARAM(26,	TCRow_Advise,								ROW_INTERFACE, IID_IDBAsynchNotify)
	TEST_CASE_WITH_PARAM(27,	TCRow_Unadvise,								ROW_INTERFACE, IID_IDBAsynchNotify)
	TEST_CASE_WITH_PARAM(28,	TCRow_GetConnectionInterface,				ROW_INTERFACE, IID_IDBAsynchNotify)
	TEST_CASE_WITH_PARAM(29,	TCRow_GetConnectionPointContainer,			ROW_INTERFACE, IID_IDBAsynchNotify)
	TEST_CASE_WITH_PARAM(30,	TCRow_EnumConnections,						ROW_INTERFACE, IID_IDBAsynchNotify)
	TEST_CASE_WITH_PARAM(31,	TCRow_EnumConnectionPoints,					ROW_INTERFACE, IID_IDBAsynchNotify)
	TEST_CASE_WITH_PARAM(32,	TCRow_FindConnectionPoint,					ROW_INTERFACE, IID_IDBAsynchNotify)

	//Stream - IConnectionPoint
	TEST_CASE_WITH_PARAM(33,	TCStream_Advise,							STREAM_INTERFACE, IID_IDBAsynchNotify)
	TEST_CASE_WITH_PARAM(34,	TCStream_Unadvise,							STREAM_INTERFACE, IID_IDBAsynchNotify)
	TEST_CASE_WITH_PARAM(35,	TCStream_GetConnectionInterface,			STREAM_INTERFACE, IID_IDBAsynchNotify)
	TEST_CASE_WITH_PARAM(36,	TCStream_GetConnectionPointContainer,		STREAM_INTERFACE, IID_IDBAsynchNotify)
	TEST_CASE_WITH_PARAM(37,	TCStream_EnumConnections,					STREAM_INTERFACE, IID_IDBAsynchNotify)
	TEST_CASE_WITH_PARAM(38,	TCStream_EnumConnectionPoints,				STREAM_INTERFACE, IID_IDBAsynchNotify)
	TEST_CASE_WITH_PARAM(39,	TCStream_FindConnectionPoint,				STREAM_INTERFACE, IID_IDBAsynchNotify)
END_TEST_MODULE()
#endif



// {{ TCW_TC_PROTOTYPE(TCAdvise)
//*-----------------------------------------------------------------------
//| Test Case:		TCAdvise - Test IConnectionPoint::Advise(
//|	Created:		02/26/96
//|	Updated:		12/01/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCAdvise::Init()
{
	TBEGIN;
	TESTB = CTestNotify::Init();
	if (TEST_PASS == TESTB)
		TEST_PROVIDER(pCSource(0)->IsSupportedCP(m_dwTestCaseParam2));

	TRETURN
}

// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc General - verify reference count
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCAdvise::Variation_1()
{
	TBEGIN

	//Verify connections / reference count
	TESTC(VerifyRefCount() && VerifyNoConnection());

CLEANUP:	
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Boundary/NULL - [null, valid] - E_POINTER
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCAdvise::Variation_2()
{
	TBEGIN

	DWORD dwCookie = 1; //set to non-zero, need to test 0 after call
	
	//Obtain the ConnectionPoint
	IConnectionPoint* pIConnectionPoint = pCSource(0)->GetCP();
	TESTC(pIConnectionPoint != NULL);

	//The following is not a successful connection, so on return Cookie should == 0,
	TESTC_(pIConnectionPoint->Advise(NULL, &dwCookie),E_POINTER);
	TESTC(dwCookie==0);
	
CLEANUP:	
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Boundary/NULL - [valid, null] - E_POINTER
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCAdvise::Variation_3()
{
	TBEGIN
		
	//Advise...
	TESTC_(pCSource(0)->GetCP()->Advise(pCListener(0), NULL),E_POINTER);

CLEANUP:
	//RefCounts should not have changed...
	VerifyRefCount();
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Boundary/NULL - [invalid sink, valid] - CANNOTCONNECT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCAdvise::Variation_4()
{
	TBEGIN
	DWORD dwCookie = 1; //set to non-zero, need to test 0 after call

	TESTC_(pCSource(0)->GetCP()->Advise(pCSource(0)->GetCP(), &dwCookie),CONNECT_E_CANNOTCONNECT);
		
	//cookie should be 0
	TESTC(dwCookie == 0);

CLEANUP:	
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Boundary/NULL - Max connections+1 | 20 connections
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCAdvise::Variation_5()
{
	TBEGIN
	HRESULT hr = S_OK;
	const ULONG cTimesConnected = 20;
	ULONG iConn = 0;
	
	//Allocate enough cookies so every connection point can have multiple connections each...
	//The layout of the bufffer is a "2-Dimensional array".
	//	Connection Point Index (First index), and Connection within that Point (Second Index)
	//	So rgCookies[0][2] = First Connection Point, and thrid connection within that point.
	//	rgCookies[0][2] = rgCookies + (iCP*cTimesConnected) + iConn
	
	//NOTE: We do this layout, so that we know which cookies go with which connection point.
	//So all cookies in the rgCookies[0] should be unique (with themselves) and so on...

	DWORD* rgCookies = NULL;
	SAFE_ALLOC(rgCookies, DWORD, cTimesConnected * pCSource(0)->GetCountCP());
	memset(rgCookies, 0, (cTimesConnected * pCSource(0)->GetCountCP())*sizeof(DWORD));

	//Note: We do this for loops in reverse order so we end up with interleaved advise's
	//ie:  IRowsetNotify, IDBAsynchNotify, Other, IRowsetNotify, IDBAsynchNotify, Other, repeat...
	for(iConn=0; iConn<cTimesConnected; iConn++)
	{	
		//Advise all connections points
		//This way we will have testing of advising the same listener for different notifications
		//A very common user senario, since they is no need to have seperate objects for each
		//notification interface, since they are different methods.  You can easily have one
		//listner than listens to both rowset data notifications, as well as rowset asynch population
		//notifications...
		for(ULONG iCP=0; iCP<pCSource(0)->GetCountCP(); iCP++)
		{
			DWORD* pdwCookie = &(rgCookies + (iCP*cTimesConnected))[iConn];
			*pdwCookie = INVALID(DWORD); //Should be 0 on error...

			//Advise this connection 
			TEST2C_(hr = pCSource(0)->GetCP(iCP)->Advise(pCListener(0), pdwCookie), S_OK, CONNECT_E_ADVISELIMIT);
			
			if(SUCCEEDED(hr))
			{
				//Make sure this is a unique cookie
				//NOTE: We are doing "All" connection points in this loop.  And a cookie only needs
				//to be unique for that connection point.  So make sure the cookie is unique for just
				//that row...
				for(ULONG iCookie=0; iCookie<iConn; iCookie++)
				{
					if(*pdwCookie == (rgCookies + (iCP*cTimesConnected))[iCookie])
						TERROR("Not a unique cookie " << *pdwCookie);
				}

				//Verify
				TESTC(*pdwCookie != 0);
			}
			else
			{
				//Verify
				TESTC(*pdwCookie == 0);
			}
		}
	}

	//Verify all connections
//	TESTC(VerifyConnection(pCListener(0), pCSource(0), cCookies));
		
CLEANUP:
    //Unadvise all connections (in reverse order)
	for(iConn=0; iConn<cTimesConnected; iConn++)
	{	
		for(ULONG iCP=0; iCP<pCSource(0)->GetCountCP(); iCP++)
		{
			DWORD* pdwCookie = &(rgCookies + (iCP*cTimesConnected))[iConn];
			if(*pdwCookie)
				CHECK(pCSource(0)->GetCP(iCP)->Unadvise(*pdwCookie),S_OK);
		}
	}
	//Verify Ref Count
	VerifyRefCount();
	SAFE_FREE(rgCookies);
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Usage Context - valid call, (IID_IRowsetNotify with a notify sink
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCAdvise::Variation_6()
{
	TBEGIN

	//Advise connection / verify
	TESTC(AdviseNum(1,1));

CLEANUP:
	UnadviseAll();
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Usage Context - 3 valid calls, (IID_IRowsetNotify with a notify sink
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCAdvise::Variation_7()
{
	TBEGIN

	//Advise connection / verify
	TESTC(Advise(pCListener(0), pCSource(0)));
	TESTC(Advise(pCListener(1), pCSource(1)));
	TESTC(Advise(pCListener(2), pCSource(2)));
 
CLEANUP:
	UnadviseAll();
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc Usage Context - invalid call, (IID_ISomeOther with a notify sink
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCAdvise::Variation_8()
{
	TBEGIN
	DWORD dwCookie = 123;
	CListener* pListener = new CListener(IID_IRowset);
	SAFE_ADDREF(pListener);

	//Try to advise a Listener to a CP that doesn't support that IID
	//IE:  An IRowsetNotify listener to a IDBAsynchStatus Connection Point...
	TESTC_(pCSource(0)->GetCP(0)->Advise(pListener, &dwCookie), CONNECT_E_CANNOTCONNECT);

CLEANUP:
	SAFE_RELEASE(pListener);
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc Usage Context - valid / invalvid / valid calls
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCAdvise::Variation_9()
{
	TBEGIN
	DWORD dwCookie = 0;
	CListener* pListener = new CListener(IID_IConnectionPoint);
	SAFE_ADDREF(pListener);

	//Valid Advise call 
	TESTC(Advise(pCListener(0), pCSource(0)));
	
	//Try to advise an  IID_IRowsetNotify CP to a non-notify CP
	TESTC_(pCSource(0)->GetCP(0)->Advise(pListener, &dwCookie),CONNECT_E_CANNOTCONNECT);

	//Valid Advise call 
	TESTC(Advise(pCListener(2), pCSource(0)));

	//Now, unadvise 
	TESTC(Unadvise(pCListener(0), pCSource(0)));
	TESTC(Unadvise(pCListener(1), pCSource(0)));
	TESTC(Unadvise(pCListener(2), pCSource(0)));
	
CLEANUP:
	UnadviseAll();
	SAFE_RELEASE(pListener);
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc Multi-Threaded - 2 Sinks / 1 Sources (Sep Threads
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCAdvise::Variation_10()
{
	//not fully implemented: the problem here is that the second thread executes 
	//after pCSource(0)->CauseNotification() (when we call END_THREADS) and doesn't receive any notifications
	//need to find way to cause main thread wait until two threads advises  - use events?
	TOUTPUT("Variation is not fully implemented");
	return TEST_SKIPPED;

	TBEGIN
	INIT_THREADS(TWO_THREADS);
	IID iid = m_dwTestCaseParam2;

	//Setup Thread Arguments
	THREADARG T1Arg         = { pCSource(0)->pICPC(), &iid};

	//Create Threads
	CREATE_THREAD(THREAD_ONE,   Thread_AdviseListener,&T1Arg);
	CREATE_THREAD(THREAD_TWO,   Thread_AdviseListener,&T1Arg);

//	START_THREADS();

	Sleep(100);  //Let the other threads actually get hookup and Listening
	//Now cause the ROW_ACTIVE notifciation to occur
	pCSource(0)->CauseNotification();
	
//	END_THREADS();	
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc Multi-Threaded - 2 Sinks / 3 Sources (Sep Threads
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCAdvise::Variation_11()
{
	//see note in V10
	TOUTPUT("Variation is not fully implemented");
	return TEST_SKIPPED;

	TBEGIN

	INIT_THREADS(SIX_THREADS);	
	IID iid = m_dwTestCaseParam2;

	//Setup Thread Arguments
	THREADARG T1Arg         = { pCSource(0)->pICPC(), &iid };
	THREADARG T2Arg         = { pCSource(1)->pICPC(), &iid };
	THREADARG T3Arg         = { pCSource(2)->pICPC(), &iid };

	//Create Threads
	//2 Lisenters for Source 1
	CREATE_THREAD(THREAD_ONE,   Thread_AdviseListener,&T1Arg);
	CREATE_THREAD(THREAD_TWO,   Thread_AdviseListener,&T1Arg);

	//2 Lisenters for Source 2
	CREATE_THREAD(THREAD_THREE, Thread_AdviseListener,&T2Arg);
	CREATE_THREAD(THREAD_FOUR,  Thread_AdviseListener,&T2Arg);

	//2 Lisenters for Source 3
	CREATE_THREAD(THREAD_FIVE,  Thread_AdviseListener,&T3Arg);
	CREATE_THREAD(THREAD_SIX,   Thread_AdviseListener,&T3Arg);

//	START_THREADS();
	
	Sleep(100);  //Let the other threads actually get hookup and Listening

	//Now cause the ROW_ACTIVE notifciation to occur
	pCSource(0)->CauseNotification();
	pCSource(1)->CauseNotification();
	pCSource(2)->CauseNotification();
	
	//	END_THREADS();	
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc Multi-Threaded - 2 Sinks / 1 Source (Sep DB Sessions
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCAdvise::Variation_12()
{
	//see note in V10
	TOUTPUT("Variation is not fully implemented");
	return TEST_SKIPPED;

	TBEGIN
	IID iid = m_dwTestCaseParam2;

	INIT_THREADS(TWO_THREADS);	

	//Setup Thread Arguments
	THREADARG T1Arg         = { pCSource(0)->pICPC(), &iid };

	//Create Threads
	CREATE_THREAD(THREAD_ONE,   Thread_AdviseListener,&T1Arg);
	CREATE_THREAD(THREAD_TWO,   Thread_AdviseListener,&T1Arg);

//	START_THREADS();
	
	Sleep(100);  //Let the other threads actually get hookup and Listening

	//Now cause the ROW_ACTIVE notifciation to occur
	pCSource(0)->CauseNotification();

//	END_THREADS();	
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc Multi-Threaded - 2 Sinks / 3 Source (Sep DB Sessions
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCAdvise::Variation_13()
{
	//see note in V10
	TOUTPUT("Variation is not fully implemented");
	return TEST_SKIPPED;

	TBEGIN
	IID iid = m_dwTestCaseParam2;

	INIT_THREADS(SIX_THREADS);	

	//Setup Thread Arguments
	THREADARG T1Arg         = { pCSource(0)->pICPC(), &iid };
	THREADARG T2Arg         = { pCSource(1)->pICPC(), &iid };
	THREADARG T3Arg         = { pCSource(2)->pICPC(), &iid };

	//Create Threads
	//2 Lisenters for Source 1
	CREATE_THREAD(THREAD_ONE,   Thread_AdviseListener,&T1Arg);
	CREATE_THREAD(THREAD_TWO,   Thread_AdviseListener,&T1Arg);

	//2 Lisenters for Source 2
	CREATE_THREAD(THREAD_THREE, Thread_AdviseListener,&T2Arg);
	CREATE_THREAD(THREAD_FOUR,  Thread_AdviseListener,&T2Arg);

	//2 Lisenters for Source 3
	CREATE_THREAD(THREAD_FIVE,  Thread_AdviseListener,&T3Arg);
	CREATE_THREAD(THREAD_SIX,   Thread_AdviseListener,&T3Arg);

//	START_THREADS();
	
	Sleep(100);  //Let the other threads actually get hookup and Listening

	//Now cause the ROW_ACTIVE notifciation to occur
	pCSource(0)->CauseNotification();
	pCSource(1)->CauseNotification();
	pCSource(2)->CauseNotification();

//	END_THREADS();	
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc Multi-User - 1 Sink / 1 Source
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCAdvise::Variation_14()
{
	TBEGIN

	//Advise connection / verify
	TESTC(Advise(pCListener(0), pCSource(0)));
	TESTC(Advise(pCListener(0), pCSource(0)));
	TESTC(Advise(pCListener(0), pCSource(0)));

CLEANUP:
	UnadviseAll();
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc Multi-User - 2 Sinks / 1 Source
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCAdvise::Variation_15()
{
	TBEGIN

	//Advise connection / verify
	TESTC(AdviseNum(2,1));

CLEANUP:
	UnadviseAll();
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc Multi-User - 1 Sink / 3 Sources
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCAdvise::Variation_16()
{
	TBEGIN

	//Advise connection / verify
	TESTC(AdviseNum(1,3));

CLEANUP:
	UnadviseAll();
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc Multi-User - 2 Sinks / 3 Sources
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCAdvise::Variation_17()
{
	TBEGIN

	//Advise sink 0 connection / verify
	TESTC(AdviseNum(2,3));

CLEANUP:
	UnadviseAll();
	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(18)
//--------------------------------------------------------------------
// @mfunc RefCount - Release of Rowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCAdvise::Variation_18()
{
	TBEGIN
	
	//Need to verify that upon release of the rowset
	//The Listeners are released, wither or not they were unadvised.
	//If the rowset doesn't Unadvise/Release any remaining Listeners,
	//They will be leaks for every listener and what objects the listeners use...
	
	//We could just use our existing Array or Sources and Listeners, but if the
	//Provider has a Reference count problem, then it will afect all following variations
	//So just create a new Source and new Listener...
	CListener* pCListener = NULL;
	CSource* pCSource = NULL;
	ULONG dwCookie = 0;

	//Create a new Source
	switch(m_dwTestCaseParam1)
	{
		case ROWSET_INTERFACE:
			pCSource = new CRowsetSource(m_dwTestCaseParam2);
			break;

		case DATASOURCE_INTERFACE:
			pCSource = new CAsynchDSOSource;
			break;
		
		case ROW_INTERFACE:
			pCSource = new CAsynchRowSource;
			break;

		case STREAM_INTERFACE:
			pCSource = new CAsynchStreamSource;
			break;

		default:
			ASSERT(!L"Unhandled TestCase Type!");
			break;
	};
	TESTC_PROVIDER(pCSource != NULL && pCSource->CreateSource());
	
	//Create a new Listener
	pCListener = new CListener(pCSource->GetIID());
	TESTC(pCListener!=NULL);
	pCListener->AddRef();


	//Now advise the Listener
	TESTC_(pCListener->Advise(&dwCookie, pCSource->pICPC(), pCSource->GetIID()),S_OK);
	TESTC(dwCookie != 0);

	//Now Release the Source
	SAFE_DELETE(pCSource);

	//Should have received ROWSET_RELEASE notification since the Source went away...
	if (m_dwTestCaseParam2==IID_IRowsetNotify)
		TESTC(pCListener->GetTimesNotified(DBREASON_ROWSET_RELEASE) > 0);

	//Should only have 1 reference (our reference) left on the Listener
	COMPC(GetRefCount(pCListener), 1);

CLEANUP:
	SAFE_DELETE(pCSource);
	SAFE_RELEASE(pCListener);
	TRETURN
}


// }}
// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCAdvise::Terminate()
{
	return(CTestNotify::Terminate());
}

// {{ TCW_TC_PROTOTYPE(TCUnadvise)
//*-----------------------------------------------------------------------
//| Test Case:		TCUnadvise - Test IConnectionPoint::Unadvise(
//|	Created:		02/29/96
//|	Updated:		12/01/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCUnadvise::Init()
{
	TBEGIN;
	TESTB = CTestNotify::Init();
	if (TEST_PASS == TESTB)
		TEST_PROVIDER(pCSource(0)->IsSupportedCP(m_dwTestCaseParam2));

	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc General - verify reference count
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUnadvise::Variation_1()
{
	//Verify No Connections
	TESTC(VerifyRefCount() && VerifyNoConnection());

CLEANUP:	
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Boundary/NULL - [null] - NOCONNECTION
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUnadvise::Variation_2()
{
	TESTC_(pCSource(0)->GetCP()->Unadvise(NULL),CONNECT_E_NOCONNECTION);

CLEANUP:	
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Boundary/NULL - Unadvise before any Advises
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUnadvise::Variation_3()
{
	TESTC_(pCSource(0)->GetCP()->Unadvise(ULONG_MAX),CONNECT_E_NOCONNECTION);

CLEANUP:	
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Sequence - Advise/Unadivse 12 variations
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUnadvise::Variation_4()
{
	TBEGIN

	//Advise 5 Connections [0..4]
	DWORD rgCookie[5];
	
	TESTC(Advise(pCListener(0), pCSource(0), &rgCookie[0]));
	TESTC(Advise(pCListener(1), pCSource(0), &rgCookie[1]));
	TESTC(Advise(pCListener(2), pCSource(0), &rgCookie[2]));
	TESTC(Advise(pCListener(3), pCSource(0), &rgCookie[3]));
	TESTC(Advise(pCListener(4), pCSource(0), &rgCookie[4]));

	//Unadvise(cookie[4]+1) - CONNECT_E_NOCONNECTION
	TESTC_(pCSource(0)->GetCP()->Unadvise(rgCookie[4]+1),CONNECT_E_NOCONNECTION);
	
	//Unadvise(cookie[4]) - S_OK
	TESTC(Unadvise(pCListener(4), pCSource(0)));
	
	//Unadvise(cookie[4]) again - CONNECT_E_NOCONNECTION
	TESTC_(pCSource(0)->GetCP()->Unadvise(rgCookie[4]),CONNECT_E_NOCONNECTION);
		
	//Advise(cookie[4]) - S_OK
	TESTC(Advise(pCListener(4), pCSource(0), &rgCookie[4]));
	
	//Unadvise(cookie[3]) - S_OK
	TESTC(Unadvise(pCListener(3), pCSource(0)));
	
	//Unadvise(cookie[4]) - S_OK
	TESTC(Unadvise(pCListener(4), pCSource(0)));
	
	//Unadvise(cookie[4..0]) cookie[0..2] - S_OK, cookie[3..4] - CONNECT_E_NOCONNECTION
	TESTC_(pCSource(0)->GetCP()->Unadvise(rgCookie[4]),CONNECT_E_NOCONNECTION);
	TESTC_(pCSource(0)->GetCP()->Unadvise(rgCookie[3]),CONNECT_E_NOCONNECTION);
	
	TESTC(Unadvise(pCListener(2), pCSource(0)));
	TESTC(Unadvise(pCListener(1), pCSource(0)));
	TESTC(Unadvise(pCListener(0), pCSource(0)));
	
	TESTC_(pCSource(0)->GetCP()->Unadvise(rgCookie[0]),CONNECT_E_NOCONNECTION);
		
	//Verify No Connections
	TESTC(VerifyNoConnection());
	
CLEANUP:
	UnadviseAll();
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Transactions - Enabled
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUnadvise::Variation_5()
{
	//Verify No Connections
	TESTC(VerifyRefCount() && VerifyNoConnection());

CLEANUP:	
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Transactions - Disabled
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUnadvise::Variation_6()
{
	//Verify No Connections
	TESTC(VerifyRefCount() && VerifyNoConnection());

CLEANUP:	
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Multi-Threaed - 1 Sink / 3 Sources (Sep Threads
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUnadvise::Variation_7()
{
	//this variation should test Multi-Thread case, but is not implemented and is duplicate of V13
	TOUTPUT("Variation is not implemented");
	return TEST_SKIPPED;

	TBEGIN

	//Advise connections / verify
	TESTC(Advise(pCListener(0), pCSource(0)));
	TESTC(Advise(pCListener(0), pCSource(1)));
	TESTC(Advise(pCListener(0), pCSource(2)));
	
	//Unadvise connections / verify
	TESTC(Unadvise(pCListener(0), pCSource(2)));
	TESTC(Unadvise(pCListener(0), pCSource(0)));
	TESTC(Unadvise(pCListener(0), pCSource(1)));

CLEANUP:
	UnadviseAll();
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc Multi-Threaed - 3 Sinks / 3 Sources (Sep Threads
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUnadvise::Variation_8()
{
	//this variation should test Multi-Thread case, but is not implemented and is duplicate of V14
	TOUTPUT("Variation is not implemented");
	return TEST_SKIPPED;

	TBEGIN

	//Advise connections / verify
	TESTC(AdviseNum(3,3)) 
	
CLEANUP:
	//Unadvise connections / verify
	UnadviseAll();
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc Multi-Threaed - 1 Sink / 3 Sources (Sep DB Sessions
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUnadvise::Variation_9()
{
	//this variation should test Multi-Thread case, but is not implemented and is duplicate of V13
	TOUTPUT("Variation is not implemented");
	return TEST_SKIPPED;

	TBEGIN

	//Advise connections / verify
	TESTC(AdviseNum(1,3));

CLEANUP:
	//Unadvise connections / verify
	UnadviseAll();
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc Multi-Threaed - 3 Sinks / 3 Sources (Sep DB Sessions
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUnadvise::Variation_10()
{
	//this variation should test Multi-Thread case, but is not implemented and is duplicate of V14
	TOUTPUT("Variation is not implemented");
	return TEST_SKIPPED;
	
	TBEGIN

	//Advise connections / verify
	TESTC(AdviseNum(3,3));

CLEANUP:
	//Unadvise connections / verify
	UnadviseAll();
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc Multi-User - 1 Sink / 1 Source
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUnadvise::Variation_11()
{
	TBEGIN

	//Advise connections / verify
	TESTC(AdviseNum(1,1));
	
CLEANUP:
	UnadviseAll();
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc Multi-User - 3 Sinks / 1 Source
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUnadvise::Variation_12()
{
	TBEGIN

	//Advise connections / verify
	TESTC(AdviseNum(3,1));

CLEANUP:
	//Unadvise connections / verify
	UnadviseAll();
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc Multi-User - 1 Sink / 3 Sources
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUnadvise::Variation_13()
{
	TBEGIN

	//Advise connections / verify
	TESTC(AdviseNum(1,3));

CLEANUP:
	//Unadvise connections / verify
	UnadviseAll();
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc Multi-User - 3 Sinks / 3 Sources
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCUnadvise::Variation_14()
{
	TBEGIN
	
	//Advise connections / verify
	TESTC(AdviseNum(3,3));

CLEANUP:
	//Unadvise connections / verify
	UnadviseAll();
	TRETURN
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCUnadvise::Terminate()
{
	return(CTestNotify::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCGetConnectionInterface)
//*-----------------------------------------------------------------------
//| Test Case:		TCGetConnectionInterface - Test IConnectionPoint::GetConnectionInterface
//|	Created:		02/29/96
//|	Updated:		12/01/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCGetConnectionInterface::Init()
{
	TBEGIN;
	TESTB = CTestNotify::Init();
	if (TEST_PASS == TESTB)
		TEST_PROVIDER(pCSource(0)->IsSupportedCP(m_dwTestCaseParam2));

	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc General - enum over the CPC and verify method
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetConnectionInterface::Variation_1()
{
	//Create an enum over the entire CPC, loop through every CP, and verify that
	//pICP->GetConnectionInterface(IID) returns an IID that when used with 
	//FindConnectionPoint(IID, pICP2), returns a CP so that pICP2 == pICP. 
	//Sounds simple enough...
	IConnectionPoint* pICP2 = NULL;
	IConnectionPoint** rgpICP = PROVIDER_ALLOC_(pCSource(0)->GetCountCP(), IConnectionPoint*);
	memset(rgpICP, 0, pCSource(0)->GetCountCP() * sizeof(IConnectionPoint*));
	IEnumConnectionPoints* pIEnumCP = pCSource(0)->pIEnumCP();
	IID iid;

	ULONG i,cFetched = 213;

	//Enumerate over the ConnectionPoints
	TESTC_(pIEnumCP->Reset(),S_OK);
	
	//Loop over supported connection points	
	for(i=0; i<pCSource(0)->GetCountCP(); i++)
	{
		TESTC_(pIEnumCP->Next(1, &rgpICP[i], &cFetched),S_OK);
		TESTC(cFetched==1);
		TESTC(VerifyConnectionPoints(1, &rgpICP[i]));
		
		//try to find the IID in the container
		TESTC_(rgpICP[i]->GetConnectionInterface(&iid),S_OK);
		TESTC_(pCSource(0)->pICPC()->FindConnectionPoint(iid, &pICP2),S_OK);
	
		//Verify pointer is the same as pICP
		TESTC(VerifyEqualICPoint(rgpICP[i], pICP2));
		SAFE_RELEASE(pICP2);
	}
	
	//Verify were at the end
	TESTC_(pIEnumCP->Next(1, &pICP2, &cFetched),S_FALSE);
	TESTC(cFetched == 0);

	//Verify ConnectionnPoints returned
	TESTC(VerifyConnectionPoints(pCSource(0)->GetCountCP(), rgpICP));

CLEANUP:
	FreeConnectionPoints(pCSource(0)->GetCountCP(), rgpICP);
	PROVIDER_FREE(rgpICP);
	SAFE_RELEASE(pICP2);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Boundary/NULL - pIID == NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetConnectionInterface::Variation_2()
{
	//pass pIID==NULL - should return E_POINTER
	TESTC_(pCSource(0)->GetCP()->GetConnectionInterface(NULL),E_POINTER);

CLEANUP:	
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Multi-User - 2 Sinks / 3 Sources verify
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetConnectionInterface::Variation_3()
{
	IConnectionPoint* pICP = NULL;
	IID iid;
	ULONG i;

	TESTC(AdviseNum(2,3));
	
	//loop through sources
	for(i=0; i<3; i++) 
	{
		//Find IID_IRowsetNotify CP
		TESTC_(pCSource(i)->pICPC()->FindConnectionPoint(pCSource(i)->GetIID(), &pICP),S_OK);

		//Test returned pICP with our CP
		TESTC(VerifyEqualICPoint(pICP, pCSource(i)->GetCP()));
					
		//Verify same IID even when connections are established
		TESTC_(pICP->GetConnectionInterface(&iid),S_OK);
		
		//verify IID returned is indeed the interface of the connection point...
		TESTC(iid == pCSource(i)->GetIID());

		//Release
		SAFE_RELEASE(pICP);
	}

CLEANUP:
	SAFE_RELEASE(pICP);
	UnadviseAll();
	TRETURN
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCGetConnectionInterface::Terminate()
{
	return(CTestNotify::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCGetConnectionPointContainer)
//*-----------------------------------------------------------------------
//| Test Case:		TCGetConnectionPointContainer - Test IConnectionPoint::GetConnectionPointContainer
//|	Created:		02/29/96
//|	Updated:		12/01/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCGetConnectionPointContainer::Init()
{
	TBEGIN;
	TESTB = CTestNotify::Init();
	if (TEST_PASS == TESTB)
		TEST_PROVIDER(pCSource(0)->IsSupportedCP(m_dwTestCaseParam2));

	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc General - verify reference count
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetConnectionPointContainer::Variation_1()
{
	TBEGIN
	BOOL fInitialized = TRUE;

	//Obtain the CPC
	IConnectionPointContainer* pICPC = NULL;
	TESTC_(pCSource(0)->GetCP()->GetConnectionPointContainer(&pICPC),S_OK);
		
	//Make sure whatever type of object this container is an interface of,
	//that it also supports those interfaces.  IE:  If off the rowset,
	//then QI for all rowset interfaces...
	fInitialized = (pCSource(0)->GetObjectType() == DATASOURCE_INTERFACE) ? FALSE : TRUE;
	TCOMPARE_(DefaultObjectTesting(pICPC, pCSource(0)->GetObjectType(), fInitialized));

	//Valid CPC interfaces
	TCHECK(QI(pICPC,IID_IConnectionPointContainer),S_OK);
					  
	//Invlid CPC interfaces
	TCHECK(QI(pICPC,IID_IConnectionPoint),E_NOINTERFACE);
	TCHECK(QI(pICPC,IID_IEnumConnections),E_NOINTERFACE);
	TCHECK(QI(pICPC,IID_IEnumConnectionPoints),E_NOINTERFACE);
	TCHECK(QI(pICPC,IID_IRowsetNotify),E_NOINTERFACE);
	TCHECK(QI(pICPC,IID_IDBAsynchNotify),E_NOINTERFACE);

CLEANUP:
	//Release
	SAFE_RELEASE(pICPC);

	//Verify connections / reference count
	VerifyRefCount();
	VerifyNoConnection();
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc General - Enum over the CP and verify correct container
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetConnectionPointContainer::Variation_2()
{
	IConnectionPointContainer* pICPC = NULL;
	IConnectionPointContainer* pICPC2 = NULL;

	IEnumConnectionPoints* pIEnumConnectionPoints = NULL;
	IConnectionPoint* pICP = NULL;
	ULONG cFetched = 213;
	HRESULT hr = S_OK;

	//loop through sources
	for(ULONG i=0; i<3; i++) 
	{
		//Obtain the CPC
		TESTC_(pCSource(i)->GetCP()->GetConnectionPointContainer(&pICPC),S_OK);
				
		//TEST_ returned pICPC with our CPC
		TESTC(VerifyEqualICPC(pICPC, pCSource(i)->pICPC()));
		
		//get Enum over the CPC
		TESTC_(pICPC->EnumConnectionPoints(&pIEnumConnectionPoints),S_OK);
		TESTC_(pIEnumConnectionPoints->Reset(),S_OK);
		
		cFetched = 123;
		//For each CP in the container, verify the parent CPC
		while(cFetched)
		{
			//Obtain the first Element
			hr = pIEnumConnectionPoints->Next(1, &pICP, &cFetched);
			if(cFetched == 0)
			{
				TESTC_(hr, S_FALSE);
				continue;
			}

			//Verify results
			TESTC_(hr, S_OK);
			TESTC(cFetched==1 && pICP!=NULL);
							
			//get the IID of the returned CP
			//Get the parent CPC
			TESTC_(pICP->GetConnectionPointContainer(&pICPC2),S_OK);
				
			//Verify it matches over already obtained CPC
			TESTC(VerifyEqualICPC(pICPC, pICPC2));

			//Release
			SAFE_RELEASE(pICPC2);
			SAFE_RELEASE(pICP);
		}
	
		//Release
		SAFE_RELEASE(pICPC);
		SAFE_RELEASE(pIEnumConnectionPoints);
	}

CLEANUP:
	SAFE_RELEASE(pICP);
	SAFE_RELEASE(pIEnumConnectionPoints);
	SAFE_RELEASE(pICPC);
	SAFE_RELEASE(pICPC2);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc General - call release and verify ref count
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetConnectionPointContainer::Variation_3()
{
	TBEGIN
	IConnectionPointContainer* rgpICPC[2] = {NULL,NULL};
	int i=0;
	//Call CPC twice
	for(i=0; i<2; i++)
		TESTC_(pCSource(0)->GetCP()->GetConnectionPointContainer(&rgpICPC[i]),S_OK);

CLEANUP:
	//release / verify
	for(i=0; i<2; i++)
		SAFE_RELEASE(rgpICPC[i]);
		

	//verify ref count
	VerifyRefCount();
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Boundary/NULL - ppICPC NULL - E_POINTER
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetConnectionPointContainer::Variation_4()
{
	TESTC_(pCSource(0)->GetCP()->GetConnectionPointContainer(NULL),E_POINTER);

CLEANUP:	
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Multi-User - 2 Sinks / 3 Sources and verify
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCGetConnectionPointContainer::Variation_5()
{
	IConnectionPointContainer* pICPC = NULL;
	IConnectionPointContainer* pICPC2 = NULL;

	IEnumConnectionPoints* pIEnumConnectionPoints = NULL;
	IConnectionPoint* pICP = NULL;
	HRESULT hr = S_OK;

	ULONG cFetched = 123;
	ULONG i;

	TESTC(AdviseNum(2,3));
	
	//loop through sources
	for(i=0; i<3; i++) 
	{
		//Obtain the CPC
		TESTC_(pCSource(i)->GetCP()->GetConnectionPointContainer(&pICPC),S_OK);
				
		//TEST_ returned pICPC with our CPC
		TESTC(VerifyEqualICPC(pICPC, pCSource(i)->pICPC()));
		
		//get Enum over the CPC
		TESTC_(pICPC->EnumConnectionPoints(&pIEnumConnectionPoints),S_OK);
		TESTC_(pIEnumConnectionPoints->Reset(),S_OK);
		
		
		cFetched = 123;
		//For each CP in the container, verify the parent CPC
		while(cFetched)
		{
			//Obtain the first Element
			hr = pIEnumConnectionPoints->Next(1, &pICP, &cFetched);
			if(cFetched == 0)
			{
				TESTC_(hr, S_FALSE);
				continue;
			}

			//Verify results
			TESTC_(hr, S_OK);
			TESTC(cFetched==1 && pICP!=NULL);
				
			//get the IID of the returned CP
			//Get the parent CPC
			TESTC_(pICP->GetConnectionPointContainer(&pICPC2),S_OK);
				
			//Verify it matches over already obtained CPC
			TESTC(VerifyEqualICPC(pICPC, pICPC2));

			//Release
			SAFE_RELEASE(pICPC2);
			SAFE_RELEASE(pICP);
		}

		SAFE_RELEASE(pICPC);
		SAFE_RELEASE(pIEnumConnectionPoints);
	}

CLEANUP:
	SAFE_RELEASE(pICP);
	SAFE_RELEASE(pIEnumConnectionPoints);
	SAFE_RELEASE(pICPC);
	SAFE_RELEASE(pICPC2);
	UnadviseAll();
	TRETURN
}
// }}
// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCGetConnectionPointContainer::Terminate()
{
	return(CTestNotify::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCEnumConnections)
//*-----------------------------------------------------------------------
//| Test Case:		TCEnumConnections - Test IConnectionPoint::EnumConnections
//|	Created:		02/29/96
//|	Updated:		12/01/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCEnumConnections::Init()
{
	TBEGIN;
	TESTB = CTestNotify::Init();
	if (TEST_PASS == TESTB)
		TEST_PROVIDER(pCSource(0)->IsSupportedCP(m_dwTestCaseParam2));

	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc General - verify reference count
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCEnumConnections::Variation_1()
{
	//Obtain the EnumConnections	
	IEnumConnections* pIEnumConnections = NULL;
	TESTC_(pCSource(0)->GetCP()->EnumConnections(&pIEnumConnections),S_OK);
		
	//Make sure whatever type of object this container is an interface of,
	//that it also supports those interfaces.
	TCOMPARE_(DefaultObjectTesting(pIEnumConnections, UNKNOWN_INTERFACE));

	//Valid CP interfaces
	TCHECK(QI(pIEnumConnections,IID_IUnknown),S_OK);

	//Invlid CP interfaces
	TCHECK(QI(pIEnumConnections,IID_IConnectionPoint),E_NOINTERFACE);
	TCHECK(QI(pIEnumConnections,IID_IEnumConnectionPoints),E_NOINTERFACE);
	TCHECK(QI(pIEnumConnections,IID_IConnectionPointContainer),E_NOINTERFACE);
	TCHECK(QI(pIEnumConnections,IID_IRowsetNotify),E_NOINTERFACE);
	TCHECK(QI(pIEnumConnections,IID_IDBAsynchStatus),E_NOINTERFACE);
	TCHECK(QI(pIEnumConnections,IID_IRowset),E_NOINTERFACE);
	TCHECK(QI(pIEnumConnections,IID_IAccessor),E_NOINTERFACE);


CLEANUP:
	SAFE_RELEASE(pIEnumConnections);
	//Verify connections / reference count
	VerifyRefCount();
	VerifyNoConnection(); 
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Boundary/NULL - ppEnum NULL - E_POINTER
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCEnumConnections::Variation_2()
{
	TESTC_(pCSource(0)->GetCP()->EnumConnections(NULL), E_POINTER);

CLEANUP:	
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Boundary/NULL - Enum over No Connections
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCEnumConnections::Variation_3()
{
	ULONG			cFetched = 213;
	CONNECTDATA  	rgConnectData[1] = { {INVALID_PUNK, INVALID_COOKIE} };
	IEnumConnections* pIEnumConnections = NULL;
	
	//Obtain an Enum over the connections in the CP, currently no connections
	TESTC_(pCSource(0)->GetCP()->EnumConnections(&pIEnumConnections),S_OK);
	TESTC_(pIEnumConnections->Reset(),S_OK);
	
	//Get the next element
	TESTC_(pIEnumConnections->Next(1,&rgConnectData[0],&cFetched),S_FALSE);
	
	//On failure (S_FALSE), unused elements in rgpcn are not set to NULL
	//and pcFetched holds the number of valid entries, even if 0 is returned

	//Even this is what the spec indicates, it doesn't make much sense, as remoting proxies
	//and stubs need to null this out so no remoting occurs for these objects.  So we will
	//allow either untouched our nulled internal members...
	TESTC(cFetched==0);
	TESTC(rgConnectData[0].pUnk==INVALID_PUNK || rgConnectData[0].pUnk==NULL);
	TESTC(rgConnectData[0].dwCookie==INVALID_COOKIE || rgConnectData[0].dwCookie==0);
	
CLEANUP:
	SAFE_RELEASE(pIEnumConnections);
	VerifyRefCount();
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Boundary/NULL - Enum over many connections
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCEnumConnections::Variation_4()
{
	ULONG			cFetched= 213;
	CONNECTDATA		rgConnectData[5] = {{NULL,0},{NULL,0},{NULL,0},{NULL,0},{NULL,0}};
	IEnumConnections* pIEnumConnections = NULL;

	const ULONG cCookies = 5;
	DWORD rgCookies[cCookies];

	//Advise 5 Connections
	TESTC(AdviseNum(5,1,rgCookies));
	
	//Obtain an Enum over the connections in the CP
	TESTC_(pCSource(0)->GetCP()->EnumConnections(&pIEnumConnections),S_OK);
	TESTC_(pIEnumConnections->Reset(),S_OK);

	//Get all the elements
	TESTC_(pIEnumConnections->Next(5, rgConnectData, &cFetched),S_OK);
	TESTC(cFetched == 5 && rgConnectData[4].pUnk!=NULL && rgConnectData[4].dwCookie!=NULL);

	//Need to verify the returned element is one of the actual connections
	TESTC(VerifyConnectData(cFetched, rgConnectData, cCookies, rgCookies));

CLEANUP:
	for(ULONG i=0;i<5;i++)
		SAFE_RELEASE(rgConnectData[i].pUnk);
	SAFE_RELEASE(pIEnumConnections);
	UnadviseAll();
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Boundary/NULL - Enum on a non-supporting Enum CP
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCEnumConnections::Variation_5()
{
	TBEGIN
	ULONG i,cFetched = 123;

	IEnumConnections* pIEnumConnections = NULL;
	IConnectionPoint* pICP2 = NULL;
	IConnectionPoint** rgpICP = PROVIDER_ALLOC_(pCSource(0)->GetCountCP(), IConnectionPoint*);
	memset(rgpICP, 0, pCSource(0)->GetCountCP() * sizeof(IConnectionPoint*));

	//Obtain an Enum over the connections in the CP
	TESTC_(pCSource(0)->pIEnumCP()->Reset(),S_OK);
	
	//For each CP in the container, verify the parent CPC
	for(i=0; i<pCSource(0)->GetCountCP(); i++)
	{
		//Obtain the first Element
		TESTC_(pCSource(0)->pIEnumCP()->Next(1, &rgpICP[i], &cFetched),S_OK);
		TESTC(cFetched == 1);
		TESTC(VerifyConnectionPoints(1, &rgpICP[i]));
		
		//get the IID of the returned CP
		TESTC_(rgpICP[i]->EnumConnections(&pIEnumConnections),S_OK);
		SAFE_RELEASE(pIEnumConnections);
	}

	//Verify were at the end
	TESTC_(pCSource(0)->pIEnumCP()->Next(1, &pICP2, &cFetched),S_FALSE);
	TESTC(cFetched == 0);

	//Verify ConnectionnPoints returned
	TESTC(VerifyConnectionPoints(pCSource(0)->GetCountCP(), rgpICP));

CLEANUP:
	SAFE_RELEASE(pICP2);
	SAFE_RELEASE(pIEnumConnections);
	FreeConnectionPoints(pCSource(0)->GetCountCP(), rgpICP);
	PROVIDER_FREE(rgpICP);
	VerifyRefCount();
	TRETURN 
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Enum::Next[0, NULL, NULL] - no ele
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCEnumConnections::Variation_6()
{
	IEnumConnections* pIEnumConnections = NULL;

	//Obtain an Enum over the connections in the CP
	TESTC_(pCSource(0)->GetCP()->EnumConnections(&pIEnumConnections),S_OK);
	
	//(0, NULL, NULL)
	//SPEC allows E_POINTER for rgpcn==NULL and E_INVALIDARG for cConnections==0
	//So for (0,NULL,NULL) it is provider specific which they evaluate first
	TEST2C_(pIEnumConnections->Next(0, NULL, NULL), E_POINTER, E_INVALIDARG);

CLEANUP:
	SAFE_RELEASE(pIEnumConnections);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Enum::Next[0, valid, valid] - no ele
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCEnumConnections::Variation_7()
{
	ULONG			cFetched= 213;
	CONNECTDATA		rgConnectData[1] = {{INVALID_PUNK, INVALID_COOKIE}};
	IEnumConnections* pIEnumConnections = NULL;

	//Obtain an Enum over the connections in the CP
	TESTC_(pCSource(0)->GetCP()->EnumConnections(&pIEnumConnections),S_OK);

	//Enum::Next(0, valid, valid)
	//cConnections == 0 - E_INVALIDARG
	TESTC_(pIEnumConnections->Next(0, rgConnectData, &cFetched), E_INVALIDARG);

	//Spec says nothing about NULLing rgConnectData elements on E_INVALIDARG
	TESTC(cFetched == 0);
	
CLEANUP:
	SAFE_RELEASE(pIEnumConnections);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc Enum::Next[1, valid, NULL] - no ele
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCEnumConnections::Variation_8()
{
	IEnumConnections* pIEnumConnections = NULL;
	CONNECTDATA	rgConnectData[1] = {{INVALID_PUNK, INVALID_COOKIE}};

	//Obtain an Enum over the connections in the CP, currently no connections
	TESTC_(pCSource(0)->GetCP()->EnumConnections(&pIEnumConnections),S_OK);

	//No ele in list - S_FALSE (enumerator return fewer than asked)
	TESTC_(pIEnumConnections->Next(1, rgConnectData, NULL),S_FALSE);

	//On failure (S_FALSE), unused elements in rgpcn are not set to NULL
	//and pcFetched holds the number of valid entries, even if 0 is returned

	//Even this is what the spec indicates, it doesn't make much sense, as remoting proxies
	//and stubs need to null this out so no remoting occurs for these objects.  So we will
	//allow either untouched our nulled internal members...
	TESTC(rgConnectData[0].pUnk==INVALID_PUNK || rgConnectData[0].pUnk==NULL);
	TESTC(rgConnectData[0].dwCookie==INVALID_COOKIE || rgConnectData[0].dwCookie==0);

CLEANUP:
	SAFE_RELEASE(pIEnumConnections);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc Enum::Next[1, NULL, NULL] - no ele
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCEnumConnections::Variation_9()
{
	IEnumConnections* pIEnumConnections = NULL;

	//Obtain an Enum over the connections in the CP
	TESTC_(pCSource(0)->GetCP()->EnumConnections(&pIEnumConnections),S_OK);

	//rgpcd==NULL - E_POINTER
	TESTC_(pIEnumConnections->Next(1,NULL,NULL),E_POINTER);

CLEANUP:
	SAFE_RELEASE(pIEnumConnections);
	TRETURN
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc Enum::Next[3, valid, NULL] - no ele
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCEnumConnections::Variation_10()
{
	IEnumConnections* pIEnumConnections = NULL;
	CONNECTDATA 	rgConnectData[3] = {{INVALID_PUNK, INVALID_COOKIE},{INVALID_PUNK, INVALID_COOKIE},{INVALID_PUNK, INVALID_COOKIE}};

	//Obtain an Enum over the connections in the CP
	TESTC_(pCSource(0)->GetCP()->EnumConnections(&pIEnumConnections),S_OK);

	//cConnections is not 1 when pcFetched == NULL - E_INVALIDARG
	TESTC_(pIEnumConnections->Next(3, rgConnectData, NULL), E_INVALIDARG);

	//Spec says nothing about NULLing reConnectData on error E_INVALIDARG

CLEANUP:
	SAFE_RELEASE(pIEnumConnections);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc Enum::Next[ULONG_MAX, valid, valid] - no ele
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCEnumConnections::Variation_11()
{
	ULONG			cFetched= 213;
	CONNECTDATA 	rgConnectData[2] = {{INVALID_PUNK, INVALID_COOKIE},{INVALID_PUNK, INVALID_COOKIE}};
	IEnumConnections* pIEnumConnections = NULL;

	//Obtain an Enum over the connections in the CP
	TESTC_(pCSource(0)->GetCP()->EnumConnections(&pIEnumConnections),S_OK);

	//No ele in list - S_FALSE (enumerator return fewer than asked)
	TESTC_(pIEnumConnections->Next(2, rgConnectData, &cFetched), S_FALSE);

	//On failure (S_FALSE), unused elements in rgpcn are not set to NULL
	//and pcFetched holds the number of valid entries, even if 0 is returned

	//Even this is what the spec indicates, it doesn't make much sense, as remoting proxies
	//and stubs need to null this out so no remoting occurs for these objects.  So we will
	//allow either untouched our nulled internal members...
	TESTC(cFetched==0);
	TESTC(rgConnectData[0].pUnk==INVALID_PUNK || rgConnectData[0].pUnk==NULL);
	TESTC(rgConnectData[0].dwCookie==INVALID_COOKIE || rgConnectData[0].dwCookie==0);

CLEANUP:
	SAFE_RELEASE(pIEnumConnections);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc Enum::Next[valid, valid, valid] - no ele
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCEnumConnections::Variation_12()
{
	ULONG			cFetched= 213;
	CONNECTDATA 	rgConnectData[1] = {{INVALID_PUNK, INVALID_COOKIE}};
	IEnumConnections* pIEnumConnections = NULL;

	//Obtain an Enum over the connections in the CP
	TESTC_(pCSource(0)->GetCP()->EnumConnections(&pIEnumConnections),S_OK);

	//No ele in list - S_FALSE (enumerator return fewer than asked)
	TESTC_(pIEnumConnections->Next(1, rgConnectData, &cFetched),S_FALSE);

	//On failure (S_FALSE), unused elements in rgpcn are not set to NULL
	//and pcFetched holds the number of valid entries, even if 0 is returned

	//Even this is what the spec indicates, it doesn't make much sense, as remoting proxies
	//and stubs need to null this out so no remoting occurs for these objects.  So we will
	//allow either untouched our nulled internal members...
	TESTC(cFetched==0);
	TESTC(rgConnectData[0].pUnk==INVALID_PUNK || rgConnectData[0].pUnk==NULL);
	TESTC(rgConnectData[0].dwCookie==INVALID_COOKIE || rgConnectData[0].dwCookie==0);

CLEANUP:
	SAFE_RELEASE(pIEnumConnections);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc Enum::Next[0, valid, NULL] - no ele
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCEnumConnections::Variation_13()
{
	CONNECTDATA 	rgConnectData[1] = {{INVALID_PUNK, INVALID_COOKIE}};
	IEnumConnections* pIEnumConnections = NULL;
	
	//Obtain an Enum over the connections in the CP
	TESTC_(pCSource(0)->GetCP()->EnumConnections(&pIEnumConnections),S_OK);

	//cConnections == 0 - E_INVALIDARG
	TESTC_(pIEnumConnections->Next(0, rgConnectData, NULL), E_INVALIDARG) 

	//Spec says nothing about NULLing rgConnectData on E_INVALIDARG

CLEANUP:
	SAFE_RELEASE(pIEnumConnections);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc Enum::Next[1, NULL, valid] - no ele
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCEnumConnections::Variation_14()
{
	ULONG			cFetched= 213;
	IEnumConnections* pIEnumConnections = NULL;
	
	//Obtain an Enum over the connections in the CP
	TESTC_(pCSource(0)->GetCP()->EnumConnections(&pIEnumConnections),S_OK);

	//rgpcn is NULL - E_POINTER
	TESTC_(pIEnumConnections->Next(1, NULL, &cFetched), E_POINTER);
	TESTC(cFetched == 0) 

CLEANUP:
	SAFE_RELEASE(pIEnumConnections);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc Enum::Next[1, valid, NULL] - 1 ele
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCEnumConnections::Variation_15()
{
	CONNECTDATA 	rgConnectData[1] = {{NULL,0}};
	IEnumConnections* pIEnumConnections = NULL;
	DWORD dwCookie = 0;

	//Advise 1 Connection
	TESTC(Advise(pCListener(0), pCSource(0), &dwCookie));
						
	//Obtain an Enum over the connections in the CP
	TESTC_(pCSource(0)->GetCP()->EnumConnections(&pIEnumConnections),S_OK);
	TESTC_(pIEnumConnections->Reset(),S_OK);

	//Get the first connections
	TESTC_(pIEnumConnections->Next(1, rgConnectData, NULL),S_OK);
		
	//verify 1 element returned
	TESTC(VerifyEqualInterface(rgConnectData[0].pUnk, pCListener(0)));
	TESTC(rgConnectData[0].dwCookie == dwCookie);
	
CLEANUP:
	SAFE_RELEASE(rgConnectData[0].pUnk);
	SAFE_RELEASE(pIEnumConnections);
	UnadviseAll();
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc Enum::Next[N+1, valid, valid] - 5 ele
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCEnumConnections::Variation_16()
{
	ULONG			cFetched= 213;
	CONNECTDATA		rgConnectData[6]={{INVALID_PUNK, INVALID_COOKIE},{INVALID_PUNK, INVALID_COOKIE},{INVALID_PUNK, INVALID_COOKIE},{INVALID_PUNK, INVALID_COOKIE},{INVALID_PUNK, INVALID_COOKIE},{INVALID_PUNK, INVALID_COOKIE}};
	IEnumConnections* pIEnumConnections = NULL;
	
	const ULONG cCookies = 5;
	DWORD rgCookies[cCookies];

	//Advise 5 Connections
	TESTC(AdviseNum(5,1,rgCookies));
		
	//Obtain an Enum over the connections in the CP
	TESTC_(pCSource(0)->GetCP()->EnumConnections(&pIEnumConnections),S_OK);
	TESTC_(pIEnumConnections->Reset(),S_OK);
	
	//Try to fetch more than is in the enum
	TESTC_(pIEnumConnections->Reset(),S_OK);
	TESTC_(pIEnumConnections->Next(6,rgConnectData,&cFetched),S_FALSE);
	TESTC(cFetched == 5 && rgConnectData[4].pUnk!=INVALID_PUNK && rgConnectData[4].dwCookie!=INVALID_COOKIE);

	//On failure (S_FALSE), unused elements in rgpcn are not set to NULL
	//and pcFetched holds the number of valid entries, even if 0 is returned

	//Even this is what the spec indicates, it doesn't make much sense, as remoting proxies
	//and stubs need to null this out so no remoting occurs for these objects.  So we will
	//allow either untouched our nulled internal members...
	TESTC(rgConnectData[5].pUnk==INVALID_PUNK || rgConnectData[5].pUnk==NULL);
	TESTC(rgConnectData[5].dwCookie==INVALID_COOKIE || rgConnectData[5].dwCookie==0);

	//Need to verify rgConnectData array
	TESTC(VerifyConnectData(cFetched, rgConnectData, cCookies, rgCookies));

CLEANUP:
	for(ULONG i=0; i<6; i++)
		if(rgConnectData[i].pUnk!=INVALID_PUNK)
			SAFE_RELEASE(rgConnectData[i].pUnk);	
	//Now Unadvise Connections
	SAFE_RELEASE(pIEnumConnections);
	UnadviseAll();
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc Enum::Next[Next seperatly over list] - 5 ele
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCEnumConnections::Variation_17()
{
	ULONG			cFetched= 213;
	CONNECTDATA		rgConnectData[5]={{NULL,0},{NULL,0},{NULL,0},{NULL,0},{NULL,0}};
	IEnumConnections* pIEnumConnections = NULL;

	const ULONG cCookies = 5;
	DWORD rgCookies[cCookies];
	ULONG i=0;

	//Advise 5 Connections
	TESTC(AdviseNum(5,1,rgCookies));
	
	//Obtain an Enum over the connections in the CP
	TESTC_(pCSource(0)->GetCP()->EnumConnections(&pIEnumConnections),S_OK);
	TESTC_(pIEnumConnections->Reset(),S_OK);

	for(i=0; i<5; i++) 
	{
		//Get the next element
		cFetched = 123;
		TESTC_(pIEnumConnections->Next(1,&rgConnectData[i],&cFetched),S_OK);
		TESTC(cFetched == 1 && rgConnectData[i].pUnk!=NULL && rgConnectData[i].dwCookie!=NULL);
			
		//Need to verify rgConnectData array
		TESTC(VerifyConnectData(cFetched, &rgConnectData[i], cCookies, rgCookies));
	}

CLEANUP:
	for(i=0; i<5; i++)
		SAFE_RELEASE(rgConnectData[i].pUnk);	
	SAFE_RELEASE(pIEnumConnections);
	UnadviseAll();
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc Enum::Next[Next group of all 5] - 5 ele
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCEnumConnections::Variation_18()
{
	ULONG			cFetched= 213;
	CONNECTDATA		rgConnectData[5]={{NULL,0},{NULL,0},{NULL,0},{NULL,0},{NULL,0}};
	IEnumConnections* pIEnumConnections = NULL;
	
	const ULONG cCookies = 5;
	DWORD rgCookies[cCookies];

	//Advise 5 Connections
	TESTC(AdviseNum(5,1,rgCookies));
		
	//Obtain an Enum over the connections in the CP
	TESTC_(pCSource(0)->GetCP()->EnumConnections(&pIEnumConnections),S_OK);
	TESTC_(pIEnumConnections->Reset(),S_OK);
	
	//Try to fetch the enum
	TESTC_(pIEnumConnections->Next(5, rgConnectData, &cFetched),S_OK);
	TESTC(cFetched == 5 && rgConnectData[4].pUnk!=NULL && rgConnectData[4].dwCookie!=NULL);

	//Need to verify rgConnectData array
	TESTC(VerifyConnectData(cFetched, rgConnectData, cCookies, rgCookies));
	
CLEANUP:
	for(ULONG i=0; i<5; i++)
		SAFE_RELEASE(rgConnectData[i].pUnk);	
	//Now Unadvise Connections
	SAFE_RELEASE(pIEnumConnections);
	UnadviseAll();
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc Enum::Next[sequence of both seperately and group] - N ele
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCEnumConnections::Variation_19()
{
	ULONG			cFetched= 213;
	CONNECTDATA		rgConnectData[5]={{NULL,0},{NULL,0},{NULL,0},{NULL,0},{NULL,0}};
	IEnumConnections* pIEnumConnections = NULL;
	
	const ULONG cCookies = 5;
	DWORD rgCookies[cCookies];
	ULONG i=0;

	//Advise 5 Connections
	TESTC(AdviseNum(5,1,rgCookies));
		
	//Obtain an Enum over the connections in the CP
	TESTC_(pCSource(0)->GetCP()->EnumConnections(&pIEnumConnections),S_OK);
	TESTC_(pIEnumConnections->Reset(),S_OK);
	
	//Try to fetch more than is in the enum
	TESTC_(pIEnumConnections->Reset(),S_OK);
	
	TESTC_(pIEnumConnections->Next(4,rgConnectData,&cFetched),S_OK);
	TESTC(cFetched == 4 && rgConnectData[3].pUnk!=NULL && rgConnectData[3].dwCookie!=NULL);

	//Need to verify rgConnectData array
	TESTC(VerifyConnectData(cFetched, rgConnectData, cCookies, rgCookies));
	for(i=0; i<5; i++)
		SAFE_RELEASE(rgConnectData[i].pUnk);	

	//Reset the enum
	TESTC_(pIEnumConnections->Reset(),S_OK);

	//Move position away from head
	cFetched=123;
	TESTC_(pIEnumConnections->Next(1,&rgConnectData[0],&cFetched),S_OK);
	TESTC(cFetched==1 && rgConnectData[0].pUnk!=NULL && rgConnectData[0].dwCookie!=NULL);

	//Need to verify rgConnectData array
	TESTC(VerifyConnectData(cFetched, &rgConnectData[0], cCookies, rgCookies));
	SAFE_RELEASE(rgConnectData[0].pUnk);	

	//Try to fetch one more than is left in the enum
	TESTC_(pIEnumConnections->Next(5,rgConnectData,&cFetched),S_FALSE);
	TESTC(cFetched == 4 && rgConnectData[3].pUnk!=NULL && rgConnectData[3].dwCookie!=NULL);
	
	//Need to verify rgConnectData array
	TESTC(VerifyConnectData(cFetched, rgConnectData, cCookies, rgCookies));

CLEANUP:
	for(i=0; i<5; i++)
		SAFE_RELEASE(rgConnectData[i].pUnk);	
	//Now Unadvise Connections
	SAFE_RELEASE(pIEnumConnections);
	UnadviseAll();
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc Enum::Skip[0] - no ele - E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCEnumConnections::Variation_20()
{
	IEnumConnections* pIEnumConnections = NULL;
	CONNECTDATA		rgConnectData[1]={{INVALID_PUNK, INVALID_COOKIE}};
	ULONG			cFetched= 213;

	//Obtain an Enum over the connections in the CP
	TESTC_(pCSource(0)->GetCP()->EnumConnections(&pIEnumConnections),S_OK);
	TESTC_(pIEnumConnections->Reset(),S_OK);
	
	//Skip(0) - E_INVALIDARG
	TESTC_(pIEnumConnections->Skip(0), E_INVALIDARG);

	//Verify position is unchanged
	TESTC_(pIEnumConnections->Next(1, rgConnectData, &cFetched),S_FALSE);

	//Even this is what the spec indicates, it doesn't make much sense, as remoting proxies
	//and stubs need to null this out so no remoting occurs for these objects.  So we will
	//allow either untouched our nulled internal members...
	TESTC(cFetched==0);
	TESTC(rgConnectData[0].pUnk==INVALID_PUNK || rgConnectData[0].pUnk==NULL);
	TESTC(rgConnectData[0].dwCookie==INVALID_COOKIE || rgConnectData[0].dwCookie==0);

CLEANUP:
	SAFE_RELEASE(pIEnumConnections);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc Enum::Skip[ULONG_MAX] - no ele - S_FALSE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCEnumConnections::Variation_21()
{
	IEnumConnections* pIEnumConnections = NULL;
	CONNECTDATA		rgConnectData[1]={{INVALID_PUNK, INVALID_COOKIE}};
	ULONG			cFetched= 213;
		
	//Obtain an Enum over the connections in the CP
	TESTC_(pCSource(0)->GetCP()->EnumConnections(&pIEnumConnections),S_OK);
	TESTC_(pIEnumConnections->Reset(),S_OK);
	
	//Skip(ULONG_MAX) - S_FALSE
	TESTC_(pIEnumConnections->Skip(ULONG_MAX), S_FALSE);

	//Verify position is at the end
	TESTC_(pIEnumConnections->Next(1, rgConnectData, &cFetched),S_FALSE);
	//Even this is what the spec indicates, it doesn't make much sense, as remoting proxies
	//and stubs need to null this out so no remoting occurs for these objects.  So we will
	//allow either untouched our nulled internal members...
	TESTC(cFetched==0);
	TESTC(rgConnectData[0].pUnk==INVALID_PUNK || rgConnectData[0].pUnk==NULL);
	TESTC(rgConnectData[0].dwCookie==INVALID_COOKIE || rgConnectData[0].dwCookie==0);

	//Once position is moved away from head, Skip(ULONG_MAX) - S_FALSE
	TESTC_(pIEnumConnections->Skip(ULONG_MAX), S_FALSE);

CLEANUP:
	SAFE_RELEASE(pIEnumConnections);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(22)
//*-----------------------------------------------------------------------
// @mfunc Enum::Skip[N] - no ele - S_FALSE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCEnumConnections::Variation_22()
{
	IEnumConnections* pIEnumConnections = NULL;
	CONNECTDATA		rgConnectData[1]={{INVALID_PUNK, INVALID_COOKIE}};
	ULONG			cFetched= 213;
		
	//Obtain an Enum over the connections in the CP
	TESTC_(pCSource(0)->GetCP()->EnumConnections(&pIEnumConnections),S_OK);
	TESTC_(pIEnumConnections->Reset(),S_OK);
	
	//Skip(N) - S_OK
	TESTC_(pIEnumConnections->Skip(5), S_FALSE);

	//Verify position is at the end
	TESTC_(pIEnumConnections->Next(1, rgConnectData, &cFetched),S_FALSE);
	//Even this is what the spec indicates, it doesn't make much sense, as remoting proxies
	//and stubs need to null this out so no remoting occurs for these objects.  So we will
	//allow either untouched our nulled internal members...
	TESTC(cFetched==0);
	TESTC(rgConnectData[0].pUnk==INVALID_PUNK || rgConnectData[0].pUnk==NULL);
	TESTC(rgConnectData[0].dwCookie==INVALID_COOKIE || rgConnectData[0].dwCookie==0);

CLEANUP:
	SAFE_RELEASE(pIEnumConnections);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(23)
//*-----------------------------------------------------------------------
// @mfunc Enum::Skip[N+1] - no ele - S_FALSE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCEnumConnections::Variation_23()
{
	IEnumConnections* pIEnumConnections = NULL;
	CONNECTDATA		rgConnectData[1]={{INVALID_PUNK, INVALID_COOKIE}};
	ULONG			cFetched= 213;
		
	//Obtain an Enum over the connections in the CP
	TESTC_(pCSource(0)->GetCP()->EnumConnections(&pIEnumConnections),S_OK);
	TESTC_(pIEnumConnections->Reset(),S_OK);
	
	//Skip(N+1) - S_FALSE
	TESTC_(pIEnumConnections->Skip(5+1), S_FALSE);

	//Verify position is at the end
	TESTC_(pIEnumConnections->Next(1, rgConnectData, &cFetched),S_FALSE);
	//Even this is what the spec indicates, it doesn't make much sense, as remoting proxies
	//and stubs need to null this out so no remoting occurs for these objects.  So we will
	//allow either untouched our nulled internal members...
	TESTC(cFetched==0);
	TESTC(rgConnectData[0].pUnk==INVALID_PUNK || rgConnectData[0].pUnk==NULL);
	TESTC(rgConnectData[0].dwCookie==INVALID_COOKIE || rgConnectData[0].dwCookie==0);

CLEANUP:
	SAFE_RELEASE(pIEnumConnections);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(24)
//*-----------------------------------------------------------------------
// @mfunc Enum::Skip[0] - N ele - E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCEnumConnections::Variation_24()
{
	IEnumConnections* pIEnumConnections = NULL;
	CONNECTDATA		rgConnectData[5]={{NULL,0},{NULL,0},{NULL,0},{NULL,0},{NULL,0}};
	ULONG			i, cFetched= 213;
	DWORD rgCookie[5];

	//Advise 5 Connections
	TESTC(AdviseNum(5,1,rgCookie));
		
	//Obtain an Enum over the connections in the CP
	TESTC_(pCSource(0)->GetCP()->EnumConnections(&pIEnumConnections),S_OK);
	TESTC_(pIEnumConnections->Reset(),S_OK);
	
	//Skip(0) - E_INVALIDARG
	TESTC_(pIEnumConnections->Skip(0), E_INVALIDARG);

	//Verify position is unchanged
	TESTC_(pIEnumConnections->Next(5, rgConnectData, &cFetched),S_OK);
	TESTC(cFetched==5 && rgConnectData[4].pUnk!=NULL && rgConnectData[4].dwCookie!=0);

CLEANUP:
	for(i=0; i<5; i++)
		SAFE_RELEASE(rgConnectData[i].pUnk);	
	SAFE_RELEASE(pIEnumConnections);
	UnadviseAll();
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(25)
//*-----------------------------------------------------------------------
// @mfunc Enum::Skip[ULONG_MAX] - N ele - S_FALSE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCEnumConnections::Variation_25()
{
	IEnumConnections* pIEnumConnections = NULL;
	CONNECTDATA		rgConnectData[1]={{INVALID_PUNK, INVALID_COOKIE}};
	ULONG			cFetched= 213;
	DWORD rgCookie[5];

	//Advise 5 Connections
	TESTC(AdviseNum(5,1,rgCookie));
		
	//Obtain an Enum over the connections in the CP
	TESTC_(pCSource(0)->GetCP()->EnumConnections(&pIEnumConnections),S_OK);
	TESTC_(pIEnumConnections->Reset(),S_OK);
	
	//Skip(ULONG_MAX) - S_FALSE
	TESTC_(pIEnumConnections->Skip(ULONG_MAX), S_FALSE);

	//Verify position is at the end
	TESTC_(pIEnumConnections->Next(1, rgConnectData, &cFetched),S_FALSE);
	//Even this is what the spec indicates, it doesn't make much sense, as remoting proxies
	//and stubs need to null this out so no remoting occurs for these objects.  So we will
	//allow either untouched our nulled internal members...
	TESTC(cFetched==0);
	TESTC(rgConnectData[0].pUnk==INVALID_PUNK || rgConnectData[0].pUnk==NULL);
	TESTC(rgConnectData[0].dwCookie==INVALID_COOKIE || rgConnectData[0].dwCookie==0);

	//Oce position is away from head, Skip(ULONG_MAX) - S_FALSE
	TESTC_(pIEnumConnections->Skip(ULONG_MAX), S_FALSE);

CLEANUP:
	SAFE_RELEASE(pIEnumConnections);
	UnadviseAll();
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(26)
//*-----------------------------------------------------------------------
// @mfunc Enum::Skip[N] - N ele - S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCEnumConnections::Variation_26()
{
	IEnumConnections* pIEnumConnections = NULL;
	CONNECTDATA		rgConnectData[1]={{INVALID_PUNK, INVALID_COOKIE}};
	ULONG			cFetched= 213;
	DWORD rgCookie[5];

	//Advise 5 Connections
	TESTC(AdviseNum(5,1,rgCookie));
		
	//Obtain an Enum over the connections in the CP
	TESTC_(pCSource(0)->GetCP()->EnumConnections(&pIEnumConnections),S_OK);
	TESTC_(pIEnumConnections->Reset(),S_OK);
	
	//Skip(N) - S_OK
	TESTC_(pIEnumConnections->Skip(5), S_OK);

	//Verify position is at the end
	TESTC_(pIEnumConnections->Next(1, rgConnectData, &cFetched),S_FALSE);
	//Even this is what the spec indicates, it doesn't make much sense, as remoting proxies
	//and stubs need to null this out so no remoting occurs for these objects.  So we will
	//allow either untouched our nulled internal members...
	TESTC(cFetched==0);
	TESTC(rgConnectData[0].pUnk==INVALID_PUNK || rgConnectData[0].pUnk==NULL);
	TESTC(rgConnectData[0].dwCookie==INVALID_COOKIE || rgConnectData[0].dwCookie==0);

CLEANUP:
	SAFE_RELEASE(pIEnumConnections);
	UnadviseAll();
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(27)
//*-----------------------------------------------------------------------
// @mfunc Enum::Skip[N-1] - N ele - S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCEnumConnections::Variation_27()
{
	IEnumConnections* pIEnumConnections = NULL;
	CONNECTDATA		rgConnectData[1]={{NULL,0}};
	ULONG			cFetched= 213;

	const ULONG cCookies = 5;
	DWORD rgCookies[cCookies];

	//Advise 5 Connections
	TESTC(AdviseNum(5,1,rgCookies));
		
	//Obtain an Enum over the connections in the CP
	TESTC_(pCSource(0)->GetCP()->EnumConnections(&pIEnumConnections),S_OK);
	TESTC_(pIEnumConnections->Reset(),S_OK);
	
	//Skip(N-1) - S_OK
	TESTC_(pIEnumConnections->Skip(5-1), S_OK);

	//Verify position is ONE from the end
	TESTC_(pIEnumConnections->Next(1, rgConnectData, &cFetched),S_OK);
	TESTC(cFetched==1 && rgConnectData[0].pUnk!=NULL && rgConnectData[0].dwCookie!=0);
	TESTC(VerifyConnectData(cFetched, rgConnectData, cCookies, rgCookies));

	rgConnectData[0].dwCookie = 0;
	SAFE_RELEASE(rgConnectData[0].pUnk);

	//Verify position is at the end
	TESTC_(pIEnumConnections->Next(1, rgConnectData, &cFetched),S_FALSE);
	TESTC(cFetched==0 && rgConnectData[0].pUnk==NULL && rgConnectData[0].dwCookie==0);

CLEANUP:
	SAFE_RELEASE(rgConnectData[0].pUnk);
	SAFE_RELEASE(pIEnumConnections);
	UnadviseAll();
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(28)
//*-----------------------------------------------------------------------
// @mfunc Enum::Skip[N+1] - N ele - S_FALSE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCEnumConnections::Variation_28()
{
	IEnumConnections* pIEnumConnections = NULL;
	CONNECTDATA		rgConnectData[1]={{INVALID_PUNK, INVALID_COOKIE}};
	ULONG			cFetched= 213;
	DWORD rgCookie[5];

	//Advise 5 Connections
	TESTC(AdviseNum(5,1,rgCookie));
		
	//Obtain an Enum over the connections in the CP
	TESTC_(pCSource(0)->GetCP()->EnumConnections(&pIEnumConnections),S_OK);
	TESTC_(pIEnumConnections->Reset(),S_OK);
	
	//Skip(N+1) - S_FALSE
	TESTC_(pIEnumConnections->Skip(5+1), S_FALSE);

	//Verify position is at the end
	TESTC_(pIEnumConnections->Next(1, rgConnectData, &cFetched),S_FALSE);
	//Even this is what the spec indicates, it doesn't make much sense, as remoting proxies
	//and stubs need to null this out so no remoting occurs for these objects.  So we will
	//allow either untouched our nulled internal members...
	TESTC(cFetched==0);
	TESTC(rgConnectData[0].pUnk==INVALID_PUNK || rgConnectData[0].pUnk==NULL);
	TESTC(rgConnectData[0].dwCookie==INVALID_COOKIE || rgConnectData[0].dwCookie==0);

CLEANUP:
	SAFE_RELEASE(pIEnumConnections);
	UnadviseAll();
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(29)
//*-----------------------------------------------------------------------
// @mfunc Enum::Clone[NULL] - E_POINTER
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCEnumConnections::Variation_29()
{
	IEnumConnections* pIEnumConnections = NULL;
	DWORD rgCookie[5];

	//Advise 5 Connections
	TESTC(AdviseNum(5,1,rgCookie));
		
	//Obtain an Enum over the connections in the CP
	TESTC_(pCSource(0)->GetCP()->EnumConnections(&pIEnumConnections),S_OK);
	TESTC_(pIEnumConnections->Reset(),S_OK);
	
	//Clone(NULL) - E_POINTER
	TESTC_(pIEnumConnections->Clone(NULL), E_POINTER);

CLEANUP:
	SAFE_RELEASE(pIEnumConnections);
	UnadviseAll();
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(30)
//*-----------------------------------------------------------------------
// @mfunc Enum::Clone[valid] - no ele - S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCEnumConnections::Variation_30()
{
	IEnumConnections* pIEnumConnections = NULL;
	IEnumConnections* pIEnumClone = NULL;
	CONNECTDATA		rgConnectData[1]={{INVALID_PUNK, INVALID_COOKIE}};
	ULONG			cFetched= 213;

	//Obtain an Enum over the connections in the CP
	TESTC_(pCSource(0)->GetCP()->EnumConnections(&pIEnumConnections),S_OK);
	TESTC_(pIEnumConnections->Reset(),S_OK);
	
	//Clone(valid) - S_OK
	TESTC_(pIEnumConnections->Clone(&pIEnumClone), S_OK);
	TESTC(pIEnumClone != NULL);

	//Verify Clone
	TESTC_(pIEnumClone->Next(1, rgConnectData, &cFetched),S_FALSE);
	TESTC(cFetched==0);

CLEANUP:
	SAFE_RELEASE(pIEnumConnections);
	SAFE_RELEASE(pIEnumClone);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(31)
//*-----------------------------------------------------------------------
// @mfunc Enum::Clone[valid] - N ele - S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCEnumConnections::Variation_31()
{
	IEnumConnections* pIEnumConnections = NULL;
	IEnumConnections* pIEnumClone = NULL;
	CONNECTDATA		rgConnectData[5]={{NULL,0},{NULL,0},{NULL,0},{NULL,0},{NULL,0}};
	ULONG			cFetched= 213;

	const ULONG cCookies = 5;
	DWORD rgCookies[cCookies];

	//Advise 5 Connections
	TESTC(AdviseNum(5,1,rgCookies));

	//Obtain an Enum over the connections in the CP
	TESTC_(pCSource(0)->GetCP()->EnumConnections(&pIEnumConnections),S_OK);
	TESTC_(pIEnumConnections->Reset(),S_OK);
	
	//Clone(valid) - S_OK
	TESTC_(pIEnumConnections->Clone(&pIEnumClone), S_OK);
	TESTC(pIEnumClone != NULL);

	//Verify Clone
	TESTC_(pIEnumClone->Next(5, rgConnectData, &cFetched),S_OK);
	TESTC(cFetched==5 && rgConnectData[4].pUnk!=NULL && rgConnectData[4].dwCookie!=0);
	TESTC(VerifyConnectData(cFetched, rgConnectData, cCookies, rgCookies));

CLEANUP:
	for(ULONG i=0; i<5; i++)
		SAFE_RELEASE(rgConnectData[i].pUnk);
	SAFE_RELEASE(pIEnumConnections);
	SAFE_RELEASE(pIEnumClone);
	UnadviseAll();
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(32)
//*-----------------------------------------------------------------------
// @mfunc Sequence - Enum sequence testing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCEnumConnections::Variation_32()
{
	// rgpIEnumConnections[0] - Enum
	// rgpIEnumConnections[1] - Clone of Enum[0]
	// rgpIEnumConnections[2] - Clone of Enum[1]
	CONNECTDATA rgConnectData[3] = {{NULL,0},{NULL,0},{NULL,0}};
	CONNECTDATA rgTmpData[4] = {{NULL,0},{NULL,0},{NULL,0},{NULL,0}};
	IEnumConnections* rgpIEnumConnections[3] = {NULL, NULL, NULL};

	ULONG i,cFetched = 123;	

	//Obtain the enums 
	TESTC_(pCSource(0)->GetCP()->EnumConnections(&rgpIEnumConnections[0]),S_OK);
	TESTC_(rgpIEnumConnections[0]->Clone(&rgpIEnumConnections[1]),S_OK);
	TESTC_(rgpIEnumConnections[1]->Clone(&rgpIEnumConnections[2]),S_OK);
	
	//Reset the enums
	for(i=0; i<3; i++)
		TESTC_(rgpIEnumConnections[i]->Reset(),S_OK);
		
	//Verify no connections in the enum
	for(i=0; i<3; i++)
	{
		cFetched = 123;
		TESTC_(rgpIEnumConnections[i]->Next(1,&rgTmpData[0],&cFetched),S_FALSE);
		TESTC(cFetched==0 && rgTmpData[0].pUnk==NULL && rgTmpData[0].dwCookie==NULL);
		TESTC_(rgpIEnumConnections[i]->Reset(),S_OK);
	}

	//Advise 3 connections
	TESTC(Advise(pCListener(0), pCSource(0)));
	TESTC(Advise(pCListener(1), pCSource(0)));
	TESTC(Advise(pCListener(2), pCSource(0)));
		
	//Enums could be static so have to recreate them once
	//there are more elements in the array...
	for(i=0; i<3; i++)
		SAFE_RELEASE(rgpIEnumConnections[i]);

	//Obtain the enums again
	TESTC_(pCSource(0)->GetCP()->EnumConnections(&rgpIEnumConnections[0]),S_OK);
	TESTC_(rgpIEnumConnections[0]->Clone(&rgpIEnumConnections[1]),S_OK);
	TESTC_(rgpIEnumConnections[1]->Clone(&rgpIEnumConnections[2]),S_OK);

	//Obtain the CP pointers
	TESTC_(rgpIEnumConnections[0]->Next(3,rgConnectData,&cFetched),S_OK);
	TESTC(cFetched==3 && rgConnectData[0].pUnk!=NULL && rgConnectData[0].dwCookie!=NULL);
	TESTC_(rgpIEnumConnections[0]->Reset(),S_OK);
	
	//Advance next position of 2 and 3
	TESTC_(rgpIEnumConnections[1]->Skip(1),S_OK);
	TESTC_(rgpIEnumConnections[2]->Skip(2),S_OK);
			
	//Should return matching connecion points
	for(i=0; i<3; i++)
	{
		cFetched = 123;
		TESTC_(rgpIEnumConnections[i]->Next(1,&rgTmpData[i],&cFetched),S_OK);
		TESTC(cFetched==1);
		TESTC(VerifyEqualInterface(rgTmpData[i].pUnk, rgConnectData[i].pUnk));
		TESTC(rgTmpData[i].dwCookie==rgConnectData[i].dwCookie);
		SAFE_RELEASE(rgTmpData[i].pUnk);
	}
	

CLEANUP:
	for(i=0; i<3; i++)
	{
		SAFE_RELEASE(rgConnectData[i].pUnk);
		SAFE_RELEASE(rgpIEnumConnections[i]);
	}
		
	UnadviseAll();
	TRETURN
}
// }}
// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCEnumConnections::Terminate()
{
	return(CTestNotify::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCEnumConnectionPoints)
//*-----------------------------------------------------------------------
//| Test Case:		TCEnumConnectionPoints - Test IConnectionPointContainer::EnumConnectionPoints
//|	Created:		02/29/96
//|	Updated:		12/01/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCEnumConnectionPoints::Init()
{
	return CTestNotify::Init();
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc General - verify reference count
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCEnumConnectionPoints::Variation_1()
{
	//Obtain the Enumerator over the CPs
	IEnumConnectionPoints* pIEnumConnectionPoints = pCSource(0)->pIEnumCP();
	TESTC(pIEnumConnectionPoints != NULL);
	
	//Make sure whatever type of object this container is an interface of,
	//that it also supports those interfaces.
	TCOMPARE_(DefaultObjectTesting(pIEnumConnectionPoints, UNKNOWN_INTERFACE));

	//Valid CP interfaces
	TCHECK(QI(pIEnumConnectionPoints,IID_IEnumConnectionPoints),S_OK);

	//Invlid CP interfaces
	TCHECK(QI(pIEnumConnectionPoints,IID_IConnectionPoint),E_NOINTERFACE);
	TCHECK(QI(pIEnumConnectionPoints,IID_IEnumConnections),E_NOINTERFACE);
	TCHECK(QI(pIEnumConnectionPoints,IID_IConnectionPointContainer),E_NOINTERFACE);
	TCHECK(QI(pIEnumConnectionPoints,IID_IRowsetNotify),E_NOINTERFACE); 
	TCHECK(QI(pIEnumConnectionPoints,IID_IDBAsynchNotify),E_NOINTERFACE); 
	TCHECK(QI(pIEnumConnectionPoints,IID_IRowset),E_NOINTERFACE);
	TCHECK(QI(pIEnumConnectionPoints,IID_IAccessor),E_NOINTERFACE);

CLEANUP:
	VerifyRefCount();
	VerifyNoConnection(); 
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc General - verify QueryInterface for every connection point
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCEnumConnectionPoints::Variation_2()
{
	TBEGIN
	ULONG i,cFetched = 123;
	IID iid;

	IConnectionPoint* pICP2 = NULL;
	IConnectionPoint** rgpICP = PROVIDER_ALLOC_(pCSource(0)->GetCountCP(), IConnectionPoint*);
	memset(rgpICP, 0, pCSource(0)->GetCountCP() * sizeof(IConnectionPoint*));

	//Obtain an Enum over the connections in the CP
	TESTC_(pCSource(0)->pIEnumCP()->Reset(),S_OK);
	
	//For each CP in the container, verify the parent CPC
	for(i=0; i<pCSource(0)->GetCountCP(); i++)
	{
		//Obtain the first Element
		TESTC_(pCSource(0)->pIEnumCP()->Next(1, &rgpICP[i], &cFetched),S_OK);
		TESTC(cFetched == 1);
		TESTC(VerifyConnectionPoints(1, &rgpICP[i]));
		
		//get the IID of the returned CP
		//Make sure the connection point is only exposing the interfaces that it should be
	
		//Make sure whatever type of object this container is an interface of,
		//that it also supports those interfaces.
		TCOMPARE_(DefaultObjectTesting(rgpICP[i], UNKNOWN_INTERFACE));

		//Valid CP interfaces
		TCHECK(QI(rgpICP[i],IID_IConnectionPoint),S_OK);

		//Invlid CP interfaces
		TCHECK(QI(rgpICP[i],IID_IEnumConnectionPoints),E_NOINTERFACE);
		TCHECK(QI(rgpICP[i],IID_IEnumConnections),E_NOINTERFACE);
		TCHECK(QI(rgpICP[i],IID_IConnectionPointContainer),E_NOINTERFACE);
		TCHECK(QI(rgpICP[i],IID_IRowsetNotify),E_NOINTERFACE); 
		TCHECK(QI(rgpICP[i],IID_IDBAsynchNotify),E_NOINTERFACE);
		TCHECK(QI(rgpICP[i],IID_IRowset),E_NOINTERFACE);
		TCHECK(QI(rgpICP[i],IID_IAccessor),E_NOINTERFACE);
		
		TESTC_(rgpICP[i]->GetConnectionInterface(&iid),S_OK);
		TESTC_(pCSource(0)->pICPC()->FindConnectionPoint(iid, &pICP2),S_OK);
				
		//Verify it matches over already obtained pPC
		TESTC(VerifyEqualICPoint(rgpICP[i], pICP2));
		SAFE_RELEASE(pICP2);
	}

	//Verify were at the end
	TESTC_(pCSource(0)->pIEnumCP()->Next(1, &pICP2, &cFetched),S_FALSE);
	TESTC(cFetched == 0);

	//Verify ConnectionnPoints returned
	TESTC(VerifyConnectionPoints(pCSource(0)->GetCountCP(), rgpICP));

CLEANUP:
	SAFE_RELEASE(pICP2);
	FreeConnectionPoints(pCSource(0)->GetCountCP(), rgpICP);
	PROVIDER_FREE(rgpICP);
	VerifyRefCount();
	TRETURN 
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Boundary/NULL - ppEnum NULL E_POINTER
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCEnumConnectionPoints::Variation_3()
{
	//Obtain the Enumerator over the CPs
	TESTC_(pCSource(0)->pICPC()->EnumConnectionPoints(NULL),E_POINTER);
		
CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc EnumCP::Next[0,NULL,NULL]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCEnumConnectionPoints::Variation_4()
{
	//Enum::Next[0,NULL,NULL]
	//cConnections==0 - E_INVALIDARG, rgpcn==NULL - E_POINTER
	//So depending upon provider the order of validation  can allow both
	TEST2C_(pCSource(0)->pIEnumCP()->Next(0,NULL,NULL), E_INVALIDARG, E_POINTER);

CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc EnumCP::Next[0, valid, valid]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCEnumConnectionPoints::Variation_5()
{
	TBEGIN
	IConnectionPoint* pICP = NULL;
	ULONG cFetched = 123;
	 
	//Obtain the Enumerator over the CPs
	TESTC_(pCSource(0)->pIEnumCP()->Reset(),S_OK);
	
	//Set position to tail
	TESTC_(pCSource(0)->pIEnumCP()->Next(1, &pICP, NULL), S_OK);
	TESTC(VerifyConnectionPoints(1, &pICP));
	SAFE_RELEASE(pICP);

	//obtain 0 elements, cConnections == 0 - E_INVALIDARG
	TESTC_(pCSource(0)->pIEnumCP()->Next(0, &pICP, &cFetched), E_INVALIDARG);
	TESTC(cFetched == 0);
	
CLEANUP:
	SAFE_RELEASE(pICP);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc EnumCP::Next[1, valid, NULL]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCEnumConnectionPoints::Variation_6()
{
	TBEGIN
	IConnectionPoint* rgpICP[2] = {NULL,NULL};
	ULONG cFetched = 123;
	
	//Obtain the Enumerator over the CPs
	TESTC_(pCSource(0)->pIEnumCP()->Reset(),S_OK);
	
	//obtain 1 elements, pcFetched==NULL, with 1 element in the enum
	TESTC_(pCSource(0)->pIEnumCP()->Next(1,&rgpICP[0],NULL),S_OK);
	TESTC(VerifyConnectionPoints(1, &rgpICP[0]));
	SAFE_RELEASE(rgpICP[0]);

	//Reset enum position
	TESTC_(pCSource(0)->pIEnumCP()->Reset(),S_OK);
	
	//obtain 2 elements, pcFetched==NULL, with 1 element in the enum
	TESTC_(pCSource(0)->pIEnumCP()->Next(2, rgpICP, NULL),E_INVALIDARG);
	
CLEANUP:
	SAFE_RELEASE(rgpICP[0]);
	SAFE_RELEASE(rgpICP[1]);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc EnumCP::Next[1, NULL, NULL]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCEnumConnectionPoints::Variation_7()
{
	//obtain 1 elements, E_POINTER
	//rgpcd==NULL - E_POINTER
	TESTC_(pCSource(0)->pIEnumCP()->Next(1, NULL, NULL), E_POINTER);

CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc EnumCP::Next[ULONG_MAX, valid, valid]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCEnumConnectionPoints::Variation_8()
{
	TBEGIN
	ULONG cFetched = 123;

	//Can't really do ULONG_MAX as the variation suggests since I have to
	//alloc the array passed in.  But I can do something large enough 1000?
	IConnectionPoint** rgpICP = PROVIDER_ALLOC_(pCSource(0)->GetCountCP()+1000, IConnectionPoint*);
	memset(rgpICP, 0, (pCSource(0)->GetCountCP()+1000) * sizeof(IConnectionPoint*));
	rgpICP[pCSource(0)->GetCountCP()] = INVALID(IConnectionPoint*);

	//Obtain the Enumerator over the CPs
	TESTC_(pCSource(0)->pIEnumCP()->Reset(),S_OK);
	
	//obtain N+1 elements, S_FALSE
	TESTC_(pCSource(0)->pIEnumCP()->Next(pCSource(0)->GetCountCP()+1000, rgpICP, &cFetched), S_FALSE);
	TESTC(cFetched == pCSource(0)->GetCountCP());
	TESTC(VerifyConnectionPoints(cFetched, rgpICP));
	TESTC(rgpICP[pCSource(0)->GetCountCP()]==INVALID(IConnectionPoint*) || rgpICP[pCSource(0)->GetCountCP()]==NULL);
	
CLEANUP:
	FreeConnectionPoints(pCSource(0)->GetCountCP(), rgpICP);
	PROVIDER_FREE(rgpICP);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc EnumCP::Next[valid, valid, valid]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCEnumConnectionPoints::Variation_9()
{
	TBEGIN
	ULONG cFetched = 123;
	IConnectionPoint** rgpICP = PROVIDER_ALLOC_(pCSource(0)->GetCountCP()+1, IConnectionPoint*);
	memset(rgpICP, 0, (pCSource(0)->GetCountCP()+1) * sizeof(IConnectionPoint*));
	rgpICP[pCSource(0)->GetCountCP()] = INVALID(IConnectionPoint*);

	//Obtain the Enumerator over the CPs
	TESTC_(pCSource(0)->pIEnumCP()->Reset(),S_OK);
	
	//obtain 1 element
	TESTC_(pCSource(0)->pIEnumCP()->Next(1, &rgpICP[0], &cFetched),S_OK);
	TESTC(cFetched==1);
	TESTC(VerifyConnectionPoints(1, &rgpICP[0]));
	SAFE_RELEASE(rgpICP[0]);

	//obtain N elements, S_FALSE
	TESTC_(pCSource(0)->pIEnumCP()->Next(pCSource(0)->GetCountCP(), rgpICP, &cFetched), S_FALSE);
	TESTC(cFetched==pCSource(0)->GetCountCP()-1);
	TESTC(VerifyConnectionPoints(cFetched, rgpICP));
	TESTC(rgpICP[pCSource(0)->GetCountCP()]==INVALID(IConnectionPoint*));
	
CLEANUP:
	FreeConnectionPoints(pCSource(0)->GetCountCP(), rgpICP);
	PROVIDER_FREE(rgpICP);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc EnumCP::Next[0, valid, NULL]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCEnumConnectionPoints::Variation_10()
{
	TBEGIN
	IConnectionPoint* pICP = NULL;
	ULONG cFetched = 123;
	
	//Obtain the Enumerator over the CPs
	TESTC_(pCSource(0)->pIEnumCP()->Reset(),S_OK);
	
	//(0, valid, NULL) - cConnections==0 - E_INVALIDARG
	TESTC_(pCSource(0)->pIEnumCP()->Next(0, &pICP, NULL),E_INVALIDARG);

CLEANUP:
	SAFE_RELEASE(pICP);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc EnumCP::Next[1, NULL, valid]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCEnumConnectionPoints::Variation_11()
{
	TBEGIN
	ULONG cFetched = 123;
	
	//Obtain the Enumerator over the CPs
	TESTC_(pCSource(0)->pIEnumCP()->Reset(),S_OK);
	
	//(1, NULL, valid) - rgpcn==NULL - E_INVALIDARG
	TESTC_(pCSource(0)->pIEnumCP()->Next(1, NULL, &cFetched), E_POINTER);
	TESTC(cFetched==0);

CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc EnumCP::Next[N+1, valid, valid]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCEnumConnectionPoints::Variation_12()
{
	TBEGIN
	ULONG cFetched = 123;
	ULONG cPoints = pCSource(0)->GetCountCP();
	IConnectionPoint** rgpICP = PROVIDER_ALLOC_(cPoints+1, IConnectionPoint*);
	memset(rgpICP, 0, (cPoints+1) * sizeof(IConnectionPoint*));
	rgpICP[cPoints] = INVALID(IConnectionPoint*);

	//Obtain the Enumerator over the CPs
	TESTC_(pCSource(0)->pIEnumCP()->Reset(),S_OK);
	
	//obtain N+1 elements, S_FALSE
	TESTC_(pCSource(0)->pIEnumCP()->Next(cPoints+1, rgpICP, &cFetched), S_FALSE);
	TESTC(cFetched == cPoints);
	TESTC(VerifyConnectionPoints(cFetched, rgpICP));
	TESTC(rgpICP[cPoints]==INVALID(IConnectionPoint*) || rgpICP[cPoints]==NULL);
	
CLEANUP:
	FreeConnectionPoints(cPoints, rgpICP);
	PROVIDER_FREE(rgpICP);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc EnumCP::Next[seperately]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCEnumConnectionPoints::Variation_13()
{
	TBEGIN
	ULONG cFetched = 123;
	ULONG cPoints = pCSource(0)->GetCountCP();
	IConnectionPoint** rgpICP = PROVIDER_ALLOC_(cPoints+1, IConnectionPoint*);
	memset(rgpICP, 0, (cPoints+1) * sizeof(IConnectionPoint*));
	rgpICP[cPoints] = INVALID(IConnectionPoint*);

	//Obtain the Enumerator over the CPs
	TESTC_(pCSource(0)->pIEnumCP()->Reset(),S_OK);
	
	//obtain 1 element
	TESTC_(pCSource(0)->pIEnumCP()->Next(1, &rgpICP[0], &cFetched),S_OK);
	TESTC(cFetched==1);
	TESTC(VerifyConnectionPoints(1, &rgpICP[0]));
	SAFE_RELEASE(rgpICP[0]);

	//obtain N elements, S_FALSE
	TESTC_(pCSource(0)->pIEnumCP()->Next(cPoints, rgpICP, &cFetched), S_FALSE);
	TESTC(cFetched==cPoints-1);
	TESTC(VerifyConnectionPoints(cFetched, rgpICP));
	TESTC(rgpICP[cPoints]==INVALID(IConnectionPoint*));
	
CLEANUP:
	FreeConnectionPoints(cPoints, rgpICP);
	PROVIDER_FREE(rgpICP);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc EnumCP::Next[group]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCEnumConnectionPoints::Variation_14()
{
	TBEGIN
	ULONG cFetched = 123;
	IConnectionPoint** rgpICP = PROVIDER_ALLOC_(pCSource(0)->GetCountCP(), IConnectionPoint*);
	memset(rgpICP, 0, pCSource(0)->GetCountCP() * sizeof(IConnectionPoint*));

	//Obtain the Enumerator over the CPs
	TESTC_(pCSource(0)->pIEnumCP()->Reset(),S_OK);
	
	//obtain N elements, S_OK
	TESTC_(pCSource(0)->pIEnumCP()->Next(pCSource(0)->GetCountCP(), rgpICP, &cFetched), S_OK);
	TESTC(cFetched == pCSource(0)->GetCountCP());
	TESTC(VerifyConnectionPoints(cFetched, rgpICP));
	
CLEANUP:
	FreeConnectionPoints(pCSource(0)->GetCountCP(), rgpICP);
	PROVIDER_FREE(rgpICP);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc EnumCP::Next[sequence]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCEnumConnectionPoints::Variation_15()
{
	TBEGIN
	ULONG cFetched = 123;
	IConnectionPoint** rgpICP = PROVIDER_ALLOC_(pCSource(0)->GetCountCP()+1, IConnectionPoint*);
	memset(rgpICP, 0, (pCSource(0)->GetCountCP()+1) * sizeof(IConnectionPoint*));

	//Obtain the Enumerator over the CPs
	TESTC_(pCSource(0)->pIEnumCP()->Reset(),S_OK);
	
	//obtain 1 element
	TESTC_(pCSource(0)->pIEnumCP()->Next(1, &rgpICP[0], &cFetched),S_OK);
	TESTC(cFetched==1);
	TESTC(VerifyConnectionPoints(1, &rgpICP[0]));
	SAFE_RELEASE(rgpICP[0]);

	//obtain N elements, S_FALSE
	TESTC_(pCSource(0)->pIEnumCP()->Next(pCSource(0)->GetCountCP(), rgpICP, &cFetched), S_FALSE);
	TESTC(cFetched == pCSource(0)->GetCountCP()-1);
	TESTC(VerifyConnectionPoints(cFetched, rgpICP));
	
CLEANUP:
	FreeConnectionPoints(pCSource(0)->GetCountCP(), rgpICP);
	PROVIDER_FREE(rgpICP);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc EnumCP::Skip[0]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCEnumConnectionPoints::Variation_16()
{
	TESTC_(pCSource(0)->pIEnumCP()->Reset(),S_OK);
	
	//Call skip(NULL) - S_OK
	//cConnections==0 - E_INVALIDARG
	TESTC_(pCSource(0)->pIEnumCP()->Skip(0),E_INVALIDARG);

CLEANUP:	
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc EnumCP::Skip[ULONG_MAX]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCEnumConnectionPoints::Variation_17()
{
	IConnectionPoint* pICP = NULL;
	ULONG cFetched = 123;

	//Skip ULONG_MAX should return S_FALSE
	TESTC_(pCSource(0)->pIEnumCP()->Reset(),S_OK);
	TESTC_(pCSource(0)->pIEnumCP()->Skip(ULONG_MAX), S_FALSE);
		
	//Move position away from head
	TESTC_(pCSource(0)->pIEnumCP()->Reset(),S_OK);
	TESTC_(pCSource(0)->pIEnumCP()->Next(1,&pICP,&cFetched),S_OK);
	TESTC(cFetched==1)
	TESTC(VerifyConnectionPoints(cFetched, &pICP));

	//Skip ULONG_MAX should return S_FALSE
	TESTC_(pCSource(0)->pIEnumCP()->Skip(ULONG_MAX), S_FALSE);

CLEANUP:
	SAFE_RELEASE(pICP);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc EnumCP::Skip[N]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCEnumConnectionPoints::Variation_18()
{
	IConnectionPoint* pICP = NULL;
	ULONG cFetched = 123;
	
	TESTC_(pCSource(0)->pIEnumCP()->Reset(),S_OK);
	
	//Call skip[N] - S_FALSE
	TESTC_(pCSource(0)->pIEnumCP()->Skip(pCSource(0)->GetCountCP()),S_OK);
	
	//Verify at tail now...
	TESTC_(pCSource(0)->pIEnumCP()->Next(1,&pICP,&cFetched),S_FALSE);
	TESTC(pICP==NULL && cFetched==0);
	
CLEANUP:
	SAFE_RELEASE(pICP);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc EnumCP::Skip[N+1]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCEnumConnectionPoints::Variation_19()
{
	IConnectionPoint* pICP = NULL;
	ULONG cFetched = 123;
	
	TESTC_(pCSource(0)->pIEnumCP()->Reset(),S_OK);
	
	//Call skip - S_OK
	TESTC_(pCSource(0)->pIEnumCP()->Skip(pCSource(0)->GetCountCP()+1),S_FALSE);
	
	//Verify at tail now...
	TESTC_(pCSource(0)->pIEnumCP()->Next(1,&pICP,&cFetched),S_FALSE);
	TESTC(pICP==NULL && cFetched==0);
	
CLEANUP:
	SAFE_RELEASE(pICP);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc EnumCP::Skip[N-1]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCEnumConnectionPoints::Variation_20()
{
	IConnectionPoint* pICP = NULL;
	ULONG cFetched = 123;
	
	TESTC_(pCSource(0)->pIEnumCP()->Reset(),S_OK);
	
	//Call skip - E_INVALIDARG
	//cConnections==0 - E_INVALIDARG
	TESTC_(pCSource(0)->pIEnumCP()->Skip(0),E_INVALIDARG);
	
	//Verify still at head...
	TESTC_(pCSource(0)->pIEnumCP()->Next(1,&pICP,&cFetched),S_OK);
	TESTC(cFetched==1);
	TESTC(VerifyConnectionPoints(1, &pICP));
	
CLEANUP:
	SAFE_RELEASE(pICP);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc EnumCP::Clone[NULL]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCEnumConnectionPoints::Variation_21()
{
	//Clone...
	TESTC_(pCSource(0)->pIEnumCP()->Clone(NULL), E_POINTER);

CLEANUP:	
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(22)
//*-----------------------------------------------------------------------
// @mfunc EnumCP::Clone[valid]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCEnumConnectionPoints::Variation_22()
{
	IEnumConnectionPoints* pIEnumCPClone = NULL;
	IConnectionPoint* pICP = NULL;
	ULONG i,cFetched = 123;
	
	TESTC_(pCSource(0)->pIEnumCP()->Reset(),S_OK);
	
	//Clone(valid) - S_OK
	TESTC_(pCSource(0)->pIEnumCP()->Clone(&pIEnumCPClone),S_OK);
	TESTC(pIEnumCPClone != NULL);

	//Verify clone
	for(i=0; i<pCSource(0)->GetCountCP(); i++)
	{
		TESTC_(pIEnumCPClone->Next(1,&pICP,&cFetched),S_OK);
		TESTC(cFetched==1);
		TESTC(VerifyConnectionPoints(1,&pICP));	
		SAFE_RELEASE(pICP);
	}

	//Try to obtain 1 more
	TESTC_(pIEnumCPClone->Next(1,&pICP,&cFetched),S_FALSE);
	TESTC(cFetched==0 && pICP==NULL);

CLEANUP:
	SAFE_RELEASE(pIEnumCPClone);
	SAFE_RELEASE(pICP);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(23)
//*-----------------------------------------------------------------------
// @mfunc Sequence - EnumCP sequence testing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCEnumConnectionPoints::Variation_23()
{
	TBEGIN
	
	// rgpIEnumConnections[0] - Enum
	// rgpIEnumConnections[1] - Clone of Enum[0]
	// rgpIEnumConnections[2] - Clone of Enum[1]
	IEnumConnectionPoints* rgpIEnumCP[3] = {NULL,NULL,NULL};
	IConnectionPoint** rgpICP = PROVIDER_ALLOC_(pCSource(0)->GetCountCP()+1, IConnectionPoint*);
	memset(rgpICP, 0, (pCSource(0)->GetCountCP()+1) * sizeof(IConnectionPoint*));
	ULONG i,cFetched = 0;	

	//Obtain the enums 
	TESTC_(pCSource(0)->pICPC()->EnumConnectionPoints(&rgpIEnumCP[0]),S_OK);
	TESTC_(rgpIEnumCP[0]->Clone(&rgpIEnumCP[1]),S_OK);
	TESTC_(rgpIEnumCP[1]->Clone(&rgpIEnumCP[2]),S_OK);
		
	//Reset the enums
	for(i=0; i<3; i++)
		TESTC_(rgpIEnumCP[i]->Reset(),S_OK);
	
	//cConnections==0 - E_INVALIDARG
	TESTC_(rgpIEnumCP[0]->Next(0,&rgpICP[0],&cFetched),E_INVALIDARG);
	TESTC(cFetched == 0);
	
	//Next 1
	TESTC_(rgpIEnumCP[1]->Next(1,&rgpICP[0],&cFetched),S_OK);
	TESTC(cFetched == 1)
	TESTC(VerifyConnectionPoints(1, &rgpICP[0]));
	SAFE_RELEASE(rgpICP[0]);

	//Next N
	cFetched = 123;
	TESTC_(rgpIEnumCP[2]->Next(pCSource(0)->GetCountCP(), rgpICP, &cFetched),S_OK);
	TESTC(cFetched == pCSource(0)->GetCountCP());
	TESTC(VerifyConnectionPoints(cFetched, rgpICP));

	//Increment the position
	TESTC_(rgpIEnumCP[0]->Skip(pCSource(0)->GetCountCP()),S_OK);

	//Call skip from current location, should all return S_FALSE
	TESTC_(rgpIEnumCP[0]->Skip(1),S_FALSE);
	TESTC_(rgpIEnumCP[1]->Skip(pCSource(0)->GetCountCP()), S_FALSE);
	TESTC_(rgpIEnumCP[2]->Skip(1),S_FALSE);
	
CLEANUP:
	SAFE_RELEASE(rgpIEnumCP[0]);		
	SAFE_RELEASE(rgpIEnumCP[1]);		
	SAFE_RELEASE(rgpIEnumCP[2]);
	FreeConnectionPoints(pCSource(0)->GetCountCP(), rgpICP);
	PROVIDER_FREE(rgpICP);
	VerifyRefCount();
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(24)
//*-----------------------------------------------------------------------
// @mfunc Regression test case for MDAC bug 73957
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCEnumConnectionPoints::Variation_24()
{
	ULONG i, j, cFetched = 123;
	HRESULT hr;
	IConnectionPoint* pICP1 = NULL;
	IConnectionPoint* pICP2 = NULL;
	BOOL fFind = FALSE;
	IID iid;
	IID rgIIDs[3];

	rgIIDs[0] = IID_IRowsetNotify;
	rgIIDs[1] = IID_IDBAsynchNotify;
	rgIIDs[2] =	IID_IRowPositionChange;

	// verify that if FindConnectionPoint finds a CP this CP can also be found through EnumConnectionPoints
	// the array m_SupportedCP[] contains 
	for (i=0; i<3; i++)
	{
		TEST2C_(hr=pCSource(0)->pICPC()->FindConnectionPoint(rgIIDs[i], &pICP1), S_OK, CONNECT_E_NOCONNECTION)
		
		if (S_OK == hr)
		{
			TESTC(pICP1!=NULL);

			//Reset the Enum
			TESTC_(pCSource(0)->pIEnumCP()->Reset(),S_OK);

			fFind = FALSE;
			//Try to find CP in the container
			for(j=0; j<pCSource(0)->GetCountCP(); j++)
			{
				//Obtain the first Element
				TESTC_(pCSource(0)->pIEnumCP()->Next(1,&pICP2,&cFetched),S_OK);
				TESTC(cFetched==1 && pICP2);
				
				//get the IID of the returned CP
				TESTC_(pICP2->GetConnectionInterface(&iid),S_OK);
				SAFE_RELEASE(pICP2);
				
				if (iid==rgIIDs[i]) 
				{
					fFind = TRUE;
					break;
				}
			}
			
			TESTC(fFind);
		}
		SAFE_RELEASE(pICP1);
	}


CLEANUP:
	SAFE_RELEASE(pICP1);
	SAFE_RELEASE(pICP2);
	TRETURN
}
// }}

// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCEnumConnectionPoints::Terminate()
{
	return(CTestNotify::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCFindConnectionPoint)
//*-----------------------------------------------------------------------
//| Test Case:		TCFindConnectionPoint - Test IConnectionPointInterface::FindConnectionPoint
//|	Created:		02/29/96
//|	Updated:		12/01/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCFindConnectionPoint::Init()
{
	TBEGIN;
	TESTB = CTestNotify::Init();
	if (TEST_PASS == TESTB)
		TEST_PROVIDER(pCSource(0)->IsSupportedCP(m_dwTestCaseParam2));

	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc General - verify reference count
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCFindConnectionPoint::Variation_1()
{
	//Verify connections / reference count
	TESTC(VerifyRefCount() && VerifyNoConnection());

CLEANUP:	
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCFindConnectionPoint::Variation_2()
{
	// this variation is a dublicate of V2 TCEnumConnectionPoints => commented out
	return TEST_SKIPPED;

/*
	IID iid;	
	ULONG i,cFetched = 123;

	IConnectionPoint* pICP2 = NULL;
	IConnectionPoint** rgpICP = PROVIDER_ALLOC_(pCSource(0)->GetCountCP(), IConnectionPoint*);
	memset(rgpICP, 0, pCSource(0)->GetCountCP() * sizeof(IConnectionPoint*));
		
	//Reset the Enum
	TESTC_(pCSource(0)->pIEnumCP()->Reset(),S_OK);

	//For each CP in the container, verify that Get(IID) == Find(IID)
	for(i=0; i<pCSource(0)->GetCountCP(); i++)
	{
		//Obtain the first Element
		TESTC_(pCSource(0)->pIEnumCP()->Next(1,&rgpICP[i],&cFetched),S_OK);
		TESTC(cFetched==1);
		TESTC(VerifyConnectionPoints(1, &rgpICP[i]));
		
		//get the IID of the returned CP
		TESTC_(rgpICP[i]->GetConnectionInterface(&iid),S_OK);
		TESTC_(pCSource(0)->pICPC()->FindConnectionPoint(iid, &pICP2), S_OK);
		
		//Verify it matches over already obtained pPC
		TESTC(VerifyEqualInterface(rgpICP[i], pICP2));
		SAFE_RELEASE(pICP2);
	}

	//Verify were at the end
	TESTC_(pCSource(0)->pIEnumCP()->Next(1, &pICP2, &cFetched),S_FALSE);
	TESTC(cFetched == 0);

	//Verify ConnectionnPoints returned
	TESTC(VerifyConnectionPoints(pCSource(0)->GetCountCP(), rgpICP));

CLEANUP:
	SAFE_RELEASE(pICP2);
	FreeConnectionPoints(pCSource(0)->GetCountCP(), rgpICP);
	PROVIDER_FREE(rgpICP);
	VerifyRefCount();
	TRETURN
*/

}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc General - call repeatedly and verify reference count
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCFindConnectionPoint::Variation_3()
{
	TBEGIN
	IConnectionPoint* rgpICP[12];
	ULONG cRefCount = GetRefCount(pCSource(0)->GetCP());
	ULONG cAddRef = 0;
	memset(rgpICP, 0, sizeof(rgpICP));
	ULONG i=0;
	//Call FindConnectionPoint repeaditly 10 times
	for(i=0; i<10; i++)
	{	
		TESTC_(pCSource(0)->pICPC()->FindConnectionPoint(pCSource(0)->GetIID(), &rgpICP[i]),S_OK);
				
		//verify returned pICP with our CP, (which is a IRowsetNotifyCP)
		if (VerifyEqualInterface(rgpICP[i], pCSource(0)->GetCP()))
			cAddRef++;
	}

	//Verify Reference count
	TESTC(VerifyRefCounts(GetRefCount(pCSource(0)->GetCP()), cRefCount+cAddRef));	

	//Now Release 2 and verify
	for(i=0; i<2; i++)
	{	
		if (VerifyEqualInterface(rgpICP[i], pCSource(0)->GetCP()))
			cAddRef--;
		SAFE_RELEASE(rgpICP[i]);
	}

	TESTC(VerifyRefCounts(GetRefCount(pCSource(0)->GetCP()), cRefCount+cAddRef));
	
	//Call Find 2 more times 
	for(i=0; i<2; i++)
	{	
		TESTC_(pCSource(0)->pICPC()->FindConnectionPoint(pCSource(0)->GetIID(), &rgpICP[i]),S_OK);
				
		//verify returned pICP with our CP, (which is a IRowsetNotifyCP)
		if (VerifyEqualInterface(rgpICP[i], pCSource(0)->GetCP()))
			cAddRef++;
	}
	TESTC(VerifyRefCounts(GetRefCount(pCSource(0)->GetCP()),cRefCount+cAddRef));

CLEANUP:
	//Release remaining
	for(i=0; i<10; i++)
		SAFE_RELEASE(rgpICP[i]);

	//Verify Reference count
	VerifyRefCount();
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Boundary/NULL - riid NULL CONNECT_E_NOCONNECTION
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCFindConnectionPoint::Variation_4()
{
	TBEGIN

	IConnectionPoint* pICP = NULL;
	GUID NULL_RIID = { NULL, NULL, NULL, NULL };

	TESTC_(pCSource(0)->pICPC()->FindConnectionPoint(NULL_RIID,&pICP),CONNECT_E_NOCONNECTION);
	TESTC(pICP == NULL);
		
CLEANUP:
	SAFE_RELEASE(pICP);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Boundary/NULL - ppICP E_POINTER
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCFindConnectionPoint::Variation_5()
{
	TESTC_(pCSource(0)->pICPC()->FindConnectionPoint(pCSource(0)->GetIID(), NULL), E_POINTER);

CLEANUP:	
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Boundary/NULL - verify valid / invalid CP
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCFindConnectionPoint::Variation_6()
{
	TBEGIN
	
	const IID* SupportedCP[]	= { &pCSource(0)->GetIID(), 
								NULL };

	const IID* InSupportedCP[]	= { &IID_IConnectionPoint, 
								&IID_IConnectionPointContainer, 
								&IID_IAccessor, 
								NULL };
	
	IConnectionPoint* pICP = NULL;
	ULONG i=0;
	//TEST_ valid IID's
	for(i=0; SupportedCP[i]; i++)
	{
		//verify valid IID
		TESTC_(pCSource(0)->pICPC()->FindConnectionPoint(*SupportedCP[i],&pICP),S_OK);
				
		//TEST_ returned pICP with our CP
		TESTC(VerifyEqualICPoint(pICP, pCSource(0)->GetCP()));
		SAFE_RELEASE(pICP);
	}

	//TEST_ Invalid IID's
	for(i=0; InSupportedCP[i]; i++)
	{
		//verify Invalid IID
		TESTC_(pCSource(0)->pICPC()->FindConnectionPoint(*InSupportedCP[i],&pICP),CONNECT_E_NOCONNECTION);
		TESTC(pICP == NULL);
	}	

CLEANUP:
	SAFE_RELEASE(pICP);
	VerifyRefCount();
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Multi-User - 2 Sinks / 1 Source verify IID returned
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCFindConnectionPoint::Variation_7()
{
	TBEGIN

	IConnectionPoint* pICP = NULL;
	IID iid;
	
	//Find IID_IRowsetNotify CP
	TESTC_(pCSource(0)->pICPC()->FindConnectionPoint(pCSource(0)->GetIID(), &pICP),S_OK);
			
	//TEST_ returned pICP with our CP, (which is a IRowsetNotifyCP)
	TESTC(VerifyEqualICPoint(pICP, pCSource(0)->GetCP()));
		
	//Now wire up 2 sinks using the newly found CP, verify connection
	TESTC(AdviseNum(2,1));
		
	//Verify same IID even when connections are established
	TESTC_(pICP->GetConnectionInterface(&iid),S_OK);
	
	TESTC(iid == pCSource(0)->GetIID());
	SAFE_RELEASE(pICP);
	
CLEANUP:
	SAFE_RELEASE(pICP);
	//Now unadvise the connections before existing
	UnadviseAll();
	TRETURN
}
// }}
// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCFindConnectionPoint::Terminate()
{
	return(CTestNotify::Terminate());
}	// }} 
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCNotReEntrant)
//*-----------------------------------------------------------------------
//| Test Case:		TCNotReEntrant - Test all interfaces that will return DB_E_NOTREENTRANT
//|	Created:		03/20/96
//|	Updated:		12/01/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCNotReEntrant::Init()
{
	//this test case is specific for Rowset object and IRowsetNotify 
	TBEGIN;
	TESTB = CTestNotify::Init();
	if (TEST_PASS == TESTB)
		TEST_PROVIDER(pCSource(0)->IsSupportedCP(IID_IRowsetNotify));

	TRETURN
}


///////////////////////////////////////////////////////////////////////////////////
// IRowset - reentrantcy methods
//
///////////////////////////////////////////////////////////////////////////////////

HRESULT TCNotReEntrant::RE_AddRefRows(CRowset* pCRowset, IRowset* pIRowset, ULONG cRows, const HROW rghRows[], ULONG cColumns, const ULONG rgColumns[])
{
	ASSERT(pCRowset && pIRowset && cRows && rghRows);

	//But should fail since those rows are already released 
	ULONG* rgRefCount = PROVIDER_ALLOC_(cRows,ULONG);
	DBROWSTATUS* rgRowStatus = PROVIDER_ALLOC_(cRows,DBROWSTATUS);
	HRESULT hr = S_OK;

	//This method (RE_AddRefRows) is called from within ReleaseRows notification
	//This will cause an error if the provider gets rid of row handles, otherwise
	//will be success if row handles are always arround (never go to 0 refcount).
	TEST2C_(hr = pIRowset->AddRefRows(cRows,rghRows,rgRefCount,rgRowStatus), S_OK, DB_E_ERRORSOCCURRED);
	
	//Verify return results
	if(hr==S_OK)
	{
		TESTC(VerifyRefCounts(cRows, rgRefCount, 2));
		TESTC(VerifyArray(cRows, rgRowStatus, DBROWSTATUS_S_OK));
	}
	else
	{
		TESTC(VerifyRefCounts(cRows,rgRefCount,0));
		TESTC(VerifyArray(cRows,rgRowStatus,DBROWSTATUS_E_INVALID));
	}

CLEANUP:
	PROVIDER_FREE(rgRefCount);
	PROVIDER_FREE(rgRowStatus);
	return hr;
}

HRESULT TCNotReEntrant::RE_GetData(CRowset* pCRowset, IRowset* pIRowset, ULONG cRows, const HROW rghRows[], ULONG cColumns, const ULONG rgColumns[])
{
	ASSERT(pCRowset && pIRowset && cRows && rghRows);

	//Call GetData from within the Listener 
	HACCESSOR hAccessor = pCRowset->m_hAccessor;
	void* pData = pCRowset->m_pData;
	HRESULT hr = S_OK;

	//GetData should be totaly reentrant.
	TESTC_(hr = pIRowset->GetData(rghRows[0],hAccessor,pData),S_OK);
	
CLEANUP:
	return hr;
}

HRESULT TCNotReEntrant::RE_GetNextRows(CRowset* pCRowset, IRowset* pIRowset, ULONG cRows, const HROW rghRows[], ULONG cColumns, const ULONG rgColumns[])
{
	ASSERT(pCRowset && pIRowset && cRows && rghRows);

	DBCOUNTITEM cRowsObtained = MAXDBCOUNTITEM;
	HROW* rghRowsObtained = NULL;
	HRESULT hr = S_OK;
	
	//Should either succeed (Allows this method to be reentrant) or fail with DB_E_NOTREENTRANT
	hr = pIRowset->GetNextRows(NULL,0,1,&cRowsObtained,&rghRowsObtained);
	TESTC(hr==S_OK || hr==DB_E_NOTREENTRANT);

	//Should NULL output params on error
	if(hr==S_OK)
	{
		TESTC(cRowsObtained == 1);
		TESTC(rghRowsObtained != NULL && rghRowsObtained[0]!=DB_NULL_HROW);
	}
	else
	{
		TESTC(cRowsObtained == 0);
		TESTC(rghRowsObtained == NULL);
	}
	
CLEANUP:
	PROVIDER_FREE(rghRowsObtained);
	return hr;
}

HRESULT TCNotReEntrant::RE_ReleaseRows(CRowset* pCRowset, IRowset* pIRowset, ULONG cRows, const HROW rghRows[], ULONG cColumns, const ULONG rgColumns[])
{
	ASSERT(pCRowset && pIRowset && cRows && rghRows);
	HRESULT hr = S_OK;

	//Should either succeed (Allows this method to be reentrant) or fail with DB_E_NOTREENTRANT
	hr = pIRowset->ReleaseRows(cRows,rghRows,NULL,NULL,NULL);
	TESTC(hr==S_OK || hr==DB_E_NOTREENTRANT);
	
CLEANUP:
	return hr;
}

HRESULT TCNotReEntrant::RE_RestartPosition(CRowset* pCRowset, IRowset* pIRowset, ULONG cRows, const HROW rghRows[], ULONG cColumns, const ULONG rgColumns[])
{
	ASSERT(pCRowset && pIRowset && cRows && rghRows);
	HRESULT hr = S_OK;

	//Should either succeed (Allows this method to be reentrant) or fail with DB_E_NOTREENTRANT
	hr = pIRowset->RestartPosition(NULL);
	TESTC(hr==S_OK || hr==DB_E_NOTREENTRANT);
	
CLEANUP:
	return hr;
}


///////////////////////////////////////////////////////////////////////////////////
// IRowsetChange - reentrantcy methods
//
///////////////////////////////////////////////////////////////////////////////////

HRESULT TCNotReEntrant::RE_DeleteRows(CRowset* pCRowset, IRowset* pIRowset, ULONG cRows, const HROW rghRows[], ULONG cColumns, const ULONG rgColumns[])
{
	ASSERT(pCRowset && pIRowset && cRows && rghRows);

	//But should fail since those rows are already released 
	DBROWSTATUS* rgRowStatus = PROVIDER_ALLOC_(cRows,DBROWSTATUS);
	HRESULT hr = S_OK;

	//Obtain IRowsetChange interface
	IRowsetChange* pIRowsetChange = NULL;
	TESTC_(QI(pIRowset,IID_IRowsetChange,(void**)&pIRowsetChange),S_OK);

	//Should either succeed (Allows this method to be reentrant) or fail with DB_E_NOTREENTRANT
	hr = pIRowsetChange->DeleteRows(NULL,cRows,rghRows,rgRowStatus);
	TESTC(hr==S_OK || hr==DB_E_NOTREENTRANT);

	//Verify Results
	if(hr==S_OK)
	{
		for(ULONG i=0; i<cRows; i++)
			TESTC(rgRowStatus[i]==DBROWSTATUS_S_OK);
	}
	else
	{
	}

CLEANUP:
	SAFE_RELEASE(pIRowsetChange);
	PROVIDER_FREE(rgRowStatus);
	return hr;
}

HRESULT TCNotReEntrant::RE_InsertRow(CRowset* pCRowset, IRowset* pIRowset, ULONG cRows, const HROW rghRows[], ULONG cColumns, const ULONG rgColumns[])
{
	ASSERT(pCRowset && pIRowset && cRows && rghRows);

	//Call InsertRow from within the Listener 
	HACCESSOR hAccessor = pCRowset->m_hAccessor;
	void* pData = pCRowset->m_pData;
	HROW hRow = INVALID(HROW);
	HRESULT hr = S_OK;

	//Obtain IRowsetChange interface
	IRowsetChange* pIRowsetChange = NULL;
	TESTC_(QI(pIRowset,IID_IRowsetChange,(void**)&pIRowsetChange),S_OK);

	//Should either succeed (Allows this method to be reentrant) or fail with DB_E_NOTREENTRANT
	hr = pIRowsetChange->InsertRow(NULL,hAccessor,pData,&hRow);
	TESTC(hr==S_OK || hr==DB_E_NOTREENTRANT);
	
	//Verify Results
	if(hr==S_OK)
	{
		TESTC(hRow != DB_NULL_HROW);
	}
	else
	{
		TESTC(hRow == DB_NULL_HROW);
	}

CLEANUP:
	SAFE_RELEASE(pIRowsetChange);
	return hr;
}

HRESULT TCNotReEntrant::RE_SetData(CRowset* pCRowset, IRowset* pIRowset, ULONG cRows, const HROW rghRows[], ULONG cColumns, const ULONG rgColumns[])
{
	ASSERT(pCRowset && pIRowset && cRows && rghRows);

	HACCESSOR hAccessor = pCRowset->m_hAccessor;
	void* pData = pCRowset->m_pData;
	HRESULT hr = S_OK;

	//Obtain IRowsetChange interface
	IRowsetChange* pIRowsetChange = NULL;
	TESTC_(QI(pIRowset,IID_IRowsetChange,(void**)&pIRowsetChange),S_OK);

	//Should either succeed (Allows this method to be reentrant) or fail with DB_E_NOTREENTRANT
	hr = pIRowsetChange->SetData(rghRows[0],hAccessor,pData);
	TESTC(hr==S_OK || hr==DB_E_NOTREENTRANT);
	
CLEANUP:
	SAFE_RELEASE(pIRowsetChange);
	return hr;
}


///////////////////////////////////////////////////////////////////////////////////
// IRowsetUpdate - reentrantcy methods
//
///////////////////////////////////////////////////////////////////////////////////

HRESULT TCNotReEntrant::RE_GetOriginalData(CRowset* pCRowset, IRowset* pIRowset, ULONG cRows, const HROW rghRows[], ULONG cColumns, const ULONG rgColumns[])
{
	ASSERT(pCRowset && pIRowset && cRows && rghRows);

	HACCESSOR hAccessor = pCRowset->m_hAccessor;
	void* pData = pCRowset->m_pData;
	HRESULT hr = S_OK;

	//Obtain IRowsetUpdate interface
	IRowsetUpdate* pIRowsetUpdate = NULL;
	TESTC_(QI(pIRowset,IID_IRowsetUpdate,(void**)&pIRowsetUpdate),S_OK);

	//Should either succeed (Allows this method to be reentrant) or fail with DB_E_NOTREENTRANT
	hr = pIRowsetUpdate->GetOriginalData(rghRows[0],hAccessor,pData);
	TESTC(hr==S_OK || hr==DB_E_NOTREENTRANT);

CLEANUP:
	SAFE_RELEASE(pIRowsetUpdate);
	return hr;
}

HRESULT TCNotReEntrant::RE_GetPendingRows(CRowset* pCRowset, IRowset* pIRowset, ULONG cRows, const HROW rghRows[], ULONG cColumns, const ULONG rgColumns[])
{
	ASSERT(pCRowset && pIRowset && cRows && rghRows);

	DBCOUNTITEM cPendingRows = MAXDBCOUNTITEM;
	HROW* rgPendingRows = NULL;
	DBROWSTATUS* rgPendingStatus = NULL;
	HRESULT hr = S_OK;

	//Obtain IRowsetUpdate interface
	IRowsetUpdate* pIRowsetUpdate = NULL;
	TESTC_(QI(pIRowset,IID_IRowsetUpdate,(void**)&pIRowsetUpdate),S_OK);

	//Should either succeed (Allows this method to be reentrant) or fail with DB_E_NOTREENTRANT
	hr = pIRowsetUpdate->GetPendingRows(NULL,DBPENDINGSTATUS_ALL,&cPendingRows,&rgPendingRows,&rgPendingStatus);
	TESTC(hr==S_OK || hr==S_FALSE || hr==DB_E_NOTREENTRANT);
	
	//Verify Results
	TESTC(cPendingRows != ULONG_MAX);

CLEANUP:
	SAFE_RELEASE(pIRowsetUpdate);
	PROVIDER_FREE(rgPendingRows);
	PROVIDER_FREE(rgPendingStatus);
	return hr;
}

HRESULT TCNotReEntrant::RE_GetRowStatus(CRowset* pCRowset, IRowset* pIRowset, ULONG cRows, const HROW rghRows[], ULONG cColumns, const ULONG rgColumns[])
{
	ASSERT(pCRowset && pIRowset && cRows && rghRows);

	DBROWSTATUS* rgPendingStatus = PROVIDER_ALLOC_(cRows,DBROWSTATUS);
	HRESULT hr = S_OK;

	//Obtain IRowsetUpdate interface
	IRowsetUpdate* pIRowsetUpdate = NULL;
	TESTC_(QI(pIRowset,IID_IRowsetUpdate,(void**)&pIRowsetUpdate),S_OK);

	//Should either succeed (Allows this method to be reentrant) or fail with DB_E_NOTREENTRANT
	hr = pIRowsetUpdate->GetRowStatus(NULL,cRows,rghRows,rgPendingStatus);
	TESTC(hr==S_OK || hr==DB_E_NOTREENTRANT);

CLEANUP:
	SAFE_RELEASE(pIRowsetUpdate);
	PROVIDER_FREE(rgPendingStatus);
	return hr;
}


HRESULT TCNotReEntrant::RE_Undo(CRowset* pCRowset, IRowset* pIRowset, ULONG cRows, const HROW rghRows[], ULONG cColumns, const ULONG rgColumns[])
{
	ASSERT(pCRowset && pIRowset && cRows && rghRows);

	DBCOUNTITEM cRowsUndone = MAXDBCOUNTITEM;
	HROW* rgRowsUndone = NULL;
	DBROWSTATUS* rgRowStatus = NULL;
	HRESULT hr = S_OK;

	//Obtain IRowsetUpdate interface
	IRowsetUpdate* pIRowsetUpdate = NULL;
	TESTC_(QI(pIRowset,IID_IRowsetUpdate,(void**)&pIRowsetUpdate),S_OK);

	//Should either succeed (Allows this method to be reentrant) or fail with DB_E_NOTREENTRANT
	hr = pIRowsetUpdate->Undo(NULL,cRows,rghRows,&cRowsUndone,&rgRowsUndone,&rgRowStatus);
	TESTC(hr==S_OK || hr==DB_E_NOTREENTRANT);
	
	//Verify return results
	TESTC(cRowsUndone != ULONG_MAX);

CLEANUP:
	SAFE_RELEASE(pIRowsetUpdate);
	PROVIDER_FREE(rgRowsUndone);
	PROVIDER_FREE(rgRowStatus);
	return hr;
}

HRESULT TCNotReEntrant::RE_Update(CRowset* pCRowset, IRowset* pIRowset, ULONG cRows, const HROW rghRows[], ULONG cColumns, const ULONG rgColumns[])
{
	ASSERT(pCRowset && pIRowset && cRows && rghRows);

	DBCOUNTITEM cRowsUpdated = MAXDBCOUNTITEM;
	HROW* rgRowsUpdated = NULL;
	DBROWSTATUS* rgRowStatus = NULL;
	HRESULT hr = S_OK;

	//Obtain IRowsetUpdate interface
	IRowsetUpdate* pIRowsetUpdate = NULL;
	TESTC_(QI(pIRowset,IID_IRowsetUpdate,(void**)&pIRowsetUpdate),S_OK);

	//Should either succeed (Allows this method to be reentrant) or fail with DB_E_NOTREENTRANT
	hr = pIRowsetUpdate->Update(NULL,cRows,rghRows,&cRowsUpdated,&rgRowsUpdated,&rgRowStatus);
	TESTC(hr==S_OK || hr==DB_E_NOTREENTRANT);
	
	//Verify return results
	TESTC(cRowsUpdated != ULONG_MAX);

CLEANUP:
	SAFE_RELEASE(pIRowsetUpdate);
	PROVIDER_FREE(rgRowsUpdated);
	PROVIDER_FREE(rgRowStatus);
	return hr;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc IRowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCNotReEntrant::Variation_1()
{
	TBEGIN
	DWORD dwCookie = 0;
	CListener* pCListener = NULL;

	//Create Rowset
	CRowset Rowset;
	TESTC_PROVIDER(Rowset.CreateRowset(DBPROP_IConnectionPointContainer)==S_OK)

	//Create Listener
	pCListener = new CListener(IID_IRowsetNotify, Rowset());
	pCListener->AddRef();

	//Advise the connection, 
	TESTC_(pCListener->Advise(&dwCookie), S_OK);

	//Try ReleaseRows -> AddRefRows
	TESTC(pCListener->ResetTimesNotified());
	TESTC(pCListener->SetReEntrantcy(DBREASON_ROW_RELEASE, DBEVENTPHASE_DIDEVENT, TCNotReEntrant::RE_AddRefRows, &Rowset));
	TESTC_(pCListener->CauseNotification(DBREASON_ROW_RELEASE),S_OK);
	TESTC(pCListener->GetTimesNotified() > 0);

	//Try GetNextRows -> GetData
	TESTC(pCListener->ResetTimesNotified());
	TESTC(pCListener->SetReEntrantcy(DBREASON_ROW_ACTIVATE, DBEVENTPHASE_DIDEVENT, TCNotReEntrant::RE_GetData, &Rowset));
	TESTC_(pCListener->CauseNotification(DBREASON_ROW_ACTIVATE),S_OK);
	TESTC(pCListener->GetTimesNotified() > 0);

	//Try GetNextRows -> GetNextRows
	TESTC(pCListener->ResetTimesNotified());
	TESTC(pCListener->SetReEntrantcy(DBREASON_ROW_ACTIVATE, DBEVENTPHASE_DIDEVENT, TCNotReEntrant::RE_GetNextRows, &Rowset));
	TESTC_(pCListener->CauseNotification(DBREASON_ROW_ACTIVATE),S_OK);
	TESTC(pCListener->GetTimesNotified() > 0);

	//Try ReleaseRows -> ReleaseRows
	TESTC(pCListener->ResetTimesNotified());
	TESTC(pCListener->SetReEntrantcy(DBREASON_ROW_RELEASE, DBEVENTPHASE_DIDEVENT, TCNotReEntrant::RE_ReleaseRows, &Rowset));
	TESTC_(pCListener->CauseNotification(DBREASON_ROW_RELEASE),S_OK);
	TESTC(pCListener->GetTimesNotified() > 0);

	//Try GetNextRows -> RestartPosition
	TESTC(pCListener->ResetTimesNotified());
	TESTC(pCListener->SetReEntrantcy(DBREASON_ROW_ACTIVATE, DBEVENTPHASE_DIDEVENT, TCNotReEntrant::RE_RestartPosition, &Rowset));
	TESTC_(pCListener->CauseNotification(DBREASON_ROW_ACTIVATE),S_OK);
	TESTC(pCListener->GetTimesNotified() > 0);

CLEANUP:
	//Unadvise all connections
	if(pCListener)
	{
		pCListener->Unadvise(dwCookie);
		pCListener->ResetTimesNotified();
	}
	SAFE_RELEASE(pCListener);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc IRowsetChange
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCNotReEntrant::Variation_2()
{
	TBEGIN
	DWORD dwCookie = 0;
	CListener* pCListener = NULL;

	//Create Rowset
	CRowset Rowset;
	Rowset.SetProperty(DBPROP_IRowsetChange);
	TESTC_PROVIDER(Rowset.CreateRowset(DBPROP_IConnectionPointContainer)==S_OK)

	//Create Listener
	pCListener = new CListener(IID_IRowsetNotify, Rowset());
	pCListener->AddRef();

	//Advise the connection, 
	TESTC_(pCListener->Advise(&dwCookie),S_OK);

	//Try ReleaseRows -> DeleteRow
	TESTC(pCListener->ResetTimesNotified());
	TESTC(pCListener->SetReEntrantcy(DBREASON_ROW_ACTIVATE, DBEVENTPHASE_DIDEVENT, TCNotReEntrant::RE_DeleteRows, &Rowset));
	TESTC_(pCListener->CauseNotification(DBREASON_ROW_ACTIVATE),S_OK);
	TESTC(pCListener->GetTimesNotified() > 0);

	//Need to setup the SetData/InsertRow pData buffer
	TESTC(Rowset.MakeRowData(&Rowset.m_pData));

	//Try GetNextRows -> InsertRow
	TESTC(pCListener->ResetTimesNotified());
	TESTC(pCListener->SetReEntrantcy(DBREASON_ROW_ACTIVATE, DBEVENTPHASE_DIDEVENT, TCNotReEntrant::RE_InsertRow, &Rowset));
	TESTC_(pCListener->CauseNotification(DBREASON_ROW_ACTIVATE),S_OK);
	TESTC(pCListener->GetTimesNotified() > 0);

	//Need to setup the SetData/InsertRow pData buffer
	TESTC(Rowset.MakeRowData(&Rowset.m_pData));

	//Try GetNextRows -> SetData
	TESTC(pCListener->ResetTimesNotified());
	TESTC(pCListener->SetReEntrantcy(DBREASON_ROW_ACTIVATE, DBEVENTPHASE_DIDEVENT, TCNotReEntrant::RE_SetData, &Rowset));
	TESTC_(pCListener->CauseNotification(DBREASON_ROW_ACTIVATE),S_OK);
	TESTC(pCListener->GetTimesNotified() > 0);

CLEANUP:
	//Unadvise all connections
	if(pCListener)
	{
		pCListener->Unadvise(dwCookie);
		pCListener->ResetTimesNotified();
	}
	SAFE_RELEASE(pCListener);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc IRowsetUpdate
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCNotReEntrant::Variation_3()
{
	TBEGIN
	DWORD dwCookie = 0;
	CListener* pCListener = NULL;

	//Create Rowset
	CRowset Rowset;
	Rowset.SetProperty(DBPROP_IRowsetUpdate);
	TESTC_PROVIDER(Rowset.CreateRowset(DBPROP_IConnectionPointContainer)==S_OK)

	//Create Listener
	pCListener = new CListener(IID_IRowsetNotify, Rowset());
	pCListener->AddRef();

	//Advise the connection, 
	TESTC_(pCListener->Advise(&dwCookie),S_OK);

	//Try ReleaseRows -> GetOriginalData
	TESTC(pCListener->ResetTimesNotified());
	TESTC(pCListener->SetReEntrantcy(DBREASON_ROW_ACTIVATE, DBEVENTPHASE_DIDEVENT, TCNotReEntrant::RE_GetOriginalData, &Rowset));
	TESTC_(pCListener->CauseNotification(DBREASON_ROW_ACTIVATE),S_OK);
	TESTC(pCListener->GetTimesNotified() > 0);

	//Try GetNextRows -> GetPendingRows
	TESTC(pCListener->ResetTimesNotified());
	TESTC(pCListener->SetReEntrantcy(DBREASON_ROW_ACTIVATE, DBEVENTPHASE_DIDEVENT, TCNotReEntrant::RE_GetPendingRows, &Rowset));
	TESTC_(pCListener->CauseNotification(DBREASON_ROW_ACTIVATE),S_OK);
	TESTC(pCListener->GetTimesNotified() > 0);

	//Try GetNextRows -> GetRowStatus
	TESTC(pCListener->ResetTimesNotified());
	TESTC(pCListener->SetReEntrantcy(DBREASON_ROW_ACTIVATE, DBEVENTPHASE_DIDEVENT, TCNotReEntrant::RE_GetRowStatus, &Rowset));
	TESTC_(pCListener->CauseNotification(DBREASON_ROW_ACTIVATE),S_OK);
	TESTC(pCListener->GetTimesNotified() > 0);

	//Try GetNextRows -> Undo
	TESTC(pCListener->ResetTimesNotified());
	TESTC(pCListener->SetReEntrantcy(DBREASON_ROW_ACTIVATE, DBEVENTPHASE_DIDEVENT, TCNotReEntrant::RE_Undo, &Rowset));
	TESTC_(pCListener->CauseNotification(DBREASON_ROW_ACTIVATE),S_OK);
	TESTC(pCListener->GetTimesNotified() > 0);

	//Try GetNextRows -> Update
	TESTC(pCListener->ResetTimesNotified());
	TESTC(pCListener->SetReEntrantcy(DBREASON_ROW_ACTIVATE, DBEVENTPHASE_DIDEVENT, TCNotReEntrant::RE_Update, &Rowset));
	TESTC_(pCListener->CauseNotification(DBREASON_ROW_ACTIVATE),S_OK);
	TESTC(pCListener->GetTimesNotified() > 0);

CLEANUP:
	//Unadvise all connections
	if(pCListener)
	{
		pCListener->Unadvise(dwCookie);
		pCListener->ResetTimesNotified();
	}
	SAFE_RELEASE(pCListener);
	TRETURN
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCNotReEntrant::Terminate()
{
	return(CTestNotify::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCZombie)
//*-----------------------------------------------------------------------
//| Test Case:		TCZombie - Test the zombie cases for ICPC/ICP
//|	Created:		07/22/96
//|	Updated:		12/01/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCZombie::Init()
{
	TBEGIN
	m_cPropSets = 0;
	m_rgPropSets = NULL;

	if (ROWSET_INTERFACE != m_dwTestCaseParam1 || IID_IRowsetNotify != m_dwTestCaseParam2)
	{
		TOUTPUT_LINE(L"This test case only run for Rowset when IRowsetNotify is supported"); 
		return TEST_SKIPPED;
	}

	CRowsetSource* pSource = new CRowsetSource(IID_IRowsetNotify);
	TESTC_PROVIDER(pSource->CreateSource());
	TESTC_PROVIDER(pSource->IsSupportedCP(IID_IRowsetNotify));
	SAFE_DELETE(pSource);

	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCTransaction::Init())
	// }}
	{
		//Set Properties
		//If ICPC is not supported, were done testing
		TEST_PROVIDER(SetSupportedProperty(DBPROP_IConnectionPointContainer,DBPROPSET_ROWSET,&m_cPropSets,&m_rgPropSets));
				
		//register interface to be tested                                         
   		if(RegisterInterface(ROWSET_INTERFACE, IID_IRowset)) 
   			return TRUE;
	}

	//Not all providers have to support transactions
	//If a required interface, an error would have been posted by VerifyInterface
	TEST_PROVIDER(m_pITransactionLocal != NULL);

CLEANUP:
	SAFE_DELETE(pSource);
	TRETURN;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Zombie - ABORT with fRetaining == TRUE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCZombie::Variation_1()
{
	IRowset* pIRowset = NULL;
	CListener* pCListener = NULL;
	DWORD dwCookie = 0;

	DBCOUNTITEM cRowsObtained = 0;
	HROW* rghRow = NULL;
	HRESULT ExpectedHr = E_UNEXPECTED;

	//Start the Transaction
	//And obtain the IOpenRowset interface
	TESTC(StartTransaction(USE_SUPPORTED_SELECT_ALLFROMTBL,(IUnknown**)&pIRowset,m_cPropSets,m_rgPropSets));
	TESTC(pIRowset!=NULL);

	//Establish a listener for the rowset
	pCListener = new CListener(IID_IRowsetNotify, pIRowset);
	pCListener->AddRef();

	//Obtain the ABORTPRESERVE flag and adjust ExpectedHr 
	if(m_fAbortPreserve) 
		ExpectedHr = S_OK;

	//Advise the connection
	TESTC_(pCListener->Advise(&dwCookie),S_OK);

	//Abort the Transaction with fRetaining==TRUE
	TESTC(GetAbort(TRUE));
	
	//Obtain the first row
	TESTC_(pIRowset->GetNextRows(NULL,0,ONE_ROW,&cRowsObtained,&rghRow),ExpectedHr);

	//Check for the notifcation
	if(m_fAbortPreserve) 
		TESTC(pCListener->GetTimesNotified() > 0) //notifcation should have succedded
	else
		TESTC(pCListener->GetTimesNotified() == 0) //rowset is zombied
	

CLEANUP:
	//Unadvise the connection
	if(pCListener && dwCookie)
		pCListener->Unadvise(dwCookie);

	//Release the rows
	if(pIRowset && rghRow)
		pIRowset->ReleaseRows(ONE_ROW,rghRow,NULL,NULL,NULL);
	
	SAFE_RELEASE(pIRowset);
	CleanUpTransaction(S_OK);
	PROVIDER_FREE(rghRow);
	SAFE_DELETE(pCListener);  //CleanUpTransaction will call pCListener->Release
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Zombie - ABORT with fRetaining == FALSE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCZombie::Variation_2()
{
	IRowset* pIRowset = NULL;
	CListener* pCListener = NULL;
	DWORD dwCookie = 0;

	DBCOUNTITEM cRowsObtained = 0;
	HROW* rghRow = NULL;
	HRESULT ExpectedHr = E_UNEXPECTED;

	//Start the Transaction
	//And obtain the IOpenRowset interface
	TESTC(StartTransaction(USE_SUPPORTED_SELECT_ALLFROMTBL,(IUnknown**)&pIRowset,m_cPropSets,m_rgPropSets));
	TESTC(pIRowset!=NULL);

	//Establish a listener for the rowset
	pCListener = new CListener(IID_IRowsetNotify, pIRowset);
	pCListener->AddRef();

	//Obtain the ABORTPRESERVE flag and adjust ExpectedHr 
	if(m_fAbortPreserve) 
		ExpectedHr = S_OK;

	//Advise the connection
	TESTC_(pCListener->Advise(&dwCookie),S_OK);

	//Abort the Transaction with fRetaining==FALSE
	TESTC(GetAbort(FALSE));
	
	//Obtain the first row
	TESTC_(pIRowset->GetNextRows(NULL,0,ONE_ROW,&cRowsObtained,&rghRow),ExpectedHr);

	//Check for the notifcation
	if(m_fAbortPreserve) 
		TESTC(pCListener->GetTimesNotified() > 0) //notifcation should have succedded
	else
		TESTC(pCListener->GetTimesNotified() == 0) //rowset is zombied
	

CLEANUP:
	//Unadvise the connection
	if(pCListener && dwCookie)
		pCListener->Unadvise(dwCookie);

	//Release the rows
	if(pIRowset && rghRow)
		pIRowset->ReleaseRows(ONE_ROW,rghRow,NULL,NULL,NULL);
	
	SAFE_RELEASE(pIRowset);
	CleanUpTransaction(XACT_E_NOTRANSACTION); //No longer in a transaction
	PROVIDER_FREE(rghRow);
	SAFE_DELETE(pCListener);  //CleanUpTransaction will call pCListener->Release
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Zombie - COMMIT with fRetaining == TRUE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCZombie::Variation_3()
{
	IRowset* pIRowset = NULL;
	CListener* pCListener = NULL;
	DWORD dwCookie = 0;

	DBCOUNTITEM cRowsObtained = 0;
	HROW* rghRow = NULL;
	HRESULT ExpectedHr = E_UNEXPECTED;

	//Start the Transaction
	//And obtain the IOpenRowset interface
	TESTC(StartTransaction(USE_SUPPORTED_SELECT_ALLFROMTBL,(IUnknown**)&pIRowset,m_cPropSets,m_rgPropSets));
	TESTC(pIRowset!=NULL);

	//Establish a listener for the rowset
	pCListener = new CListener(IID_IRowsetNotify, pIRowset);
	pCListener->AddRef();

	//Obtain the COMMITPRESERVE flag and adjust ExpectedHr 
	if(m_fCommitPreserve) 
		ExpectedHr = S_OK;

	//Advise the connection
	TESTC_(pCListener->Advise(&dwCookie),S_OK);

	//Abort the Transaction with fRetaining==TRUE
	TESTC(GetCommit(TRUE));
	
	//Obtain the first row
	TESTC_(pIRowset->GetNextRows(NULL,0,ONE_ROW,&cRowsObtained,&rghRow),ExpectedHr);

	//Check for the notifcation
	if(m_fCommitPreserve) 
		TESTC(pCListener->GetTimesNotified() > 0) //notifcation should have succedded
	else
		TESTC(pCListener->GetTimesNotified() == 0) //rowset is zombied
	

CLEANUP:
	//Unadvise the connection
	if(pCListener && dwCookie)
		pCListener->Unadvise(dwCookie);

	//Release the rows
	if(pIRowset && rghRow)
		pIRowset->ReleaseRows(ONE_ROW,rghRow,NULL,NULL,NULL);
	
	SAFE_RELEASE(pIRowset);
	CleanUpTransaction(S_OK);
	PROVIDER_FREE(rghRow);
	SAFE_DELETE(pCListener);  //CleanUpTransaction will call pCListener->Release
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Zombie - COMMIT with fRetaining == FALSE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCZombie::Variation_4()
{
	IRowset* pIRowset = NULL;
	CListener* pCListener = NULL;
	DWORD dwCookie = 0;

	DBCOUNTITEM cRowsObtained = 0;
	HROW* rghRow = NULL;
	HRESULT ExpectedHr = E_UNEXPECTED;

	//Start the Transaction
	//And obtain the IOpenRowset interface
	TESTC(StartTransaction(USE_SUPPORTED_SELECT_ALLFROMTBL,(IUnknown**)&pIRowset,m_cPropSets,m_rgPropSets));
	TESTC(pIRowset!=NULL);

	//Establish a listener for the rowset
	pCListener = new CListener(IID_IRowsetNotify, pIRowset);
	pCListener->AddRef();

	//Obtain the COMMITPRESERVE flag and adjust ExpectedHr 
	if(m_fCommitPreserve) 
		ExpectedHr = S_OK;

	//Advise the connection
	TESTC_(pCListener->Advise(&dwCookie),S_OK);

	//Abort the Transaction with fRetaining==TRUE
	TESTC(GetCommit(FALSE));
	
	//Obtain the first row
	TESTC_(pIRowset->GetNextRows(NULL,0,ONE_ROW,&cRowsObtained,&rghRow),ExpectedHr);

	//Check for the notifcation
	if(m_fCommitPreserve) 
		TESTC(pCListener->GetTimesNotified() > 0) //notifcation should have succedded
	else
		TESTC(pCListener->GetTimesNotified() == 0) //rowset is zombied
	

CLEANUP:
	//Unadvise the connection
	if(pCListener && dwCookie)
		pCListener->Unadvise(dwCookie);

	//Release the rows
	if(pIRowset && rghRow)
		pIRowset->ReleaseRows(ONE_ROW,rghRow,NULL,NULL,NULL);
	
	SAFE_RELEASE(pIRowset);
	CleanUpTransaction(XACT_E_NOTRANSACTION); //No longer in a transaction
	PROVIDER_FREE(rghRow);
	SAFE_DELETE(pCListener);  //CleanUpTransaction will call pCListener->Release
	TRETURN
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCZombie::Terminate()
{
	//FreeProperties
	FreeProperties(&m_cPropSets,&m_rgPropSets);
	
	return(TCTransaction::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCProperties)
//*-----------------------------------------------------------------------
//| Test Case:		TCProperties - Test all of the Notifcation properties
//|	Created:		08/05/96
//|	Updated:		12/01/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCProperties::Init()
{
	return CTestNotify::Init();
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Properties - DBPROP_IConnectionPointContainer
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCProperties::Variation_1()
{
	TBEGIN
	DBPROPFLAGS dwPropFlags;
	CRowset RowsetA;

	//Some provider might not support this property
	TESTC_PROVIDER(SupportedProperty(DBPROP_IConnectionPointContainer, DBPROPSET_ROWSET))

	//Get the PropInfo
	dwPropFlags = GetPropInfoFlags(DBPROP_IConnectionPointContainer, DBPROPSET_ROWSET);
	
	//Should be at least Read
	TESTC(BITSET(dwPropFlags, DBPROPFLAGS_READ));
			
	//Should be a ROWSET group
	TESTC(BITSET(dwPropFlags,DBPROPFLAGS_ROWSET));

	//Flags really shouldn't have any other values...
	TESTC(BITCLEAR(dwPropFlags,~(DBPROPFLAGS_READ | DBPROPFLAGS_WRITE | DBPROPFLAGS_ROWSET)) );

	//Get the PropValue
	TESTC_(RowsetA.CreateRowset(USE_SUPPORTED_SELECT_ALLFROMTBL, IID_IConnectionPointContainer),S_OK);
	TESTC(RowsetA.GetProperty(DBPROP_IConnectionPointContainer, DBPROPSET_ROWSET, VARIANT_TRUE));

CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Properties - DBPROP_NOTIFICATIONPHASES
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCProperties::Variation_2()
{
	TBEGIN
	CRowset RowsetA;

	ULONG_PTR ulValue;
	TESTC_(RowsetA.CreateRowset(),S_OK);
	TESTC_PROVIDER(RowsetA.GetProperty(DBPROP_NOTIFICATIONPHASES, DBPROPSET_ROWSET, &ulValue))
	
	//All Providers must support _FAILEDTODO / _DIDEVENT
	TESTC(BITSET(ulValue, DBPROPVAL_NP_FAILEDTODO));
	TESTC(BITSET(ulValue, DBPROPVAL_NP_DIDEVENT));
		
	//Not all providers will supported all  phases
	TESTC_PROVIDER(BITSET(ulValue, DBPROPVAL_NP_OKTODO)==TRUE)
	TESTC_PROVIDER(BITSET(ulValue, DBPROPVAL_NP_ABOUTTODO)==TRUE)
	TESTC_PROVIDER(BITSET(ulValue, DBPROPVAL_NP_SYNCHAFTER)==TRUE)

	//Make sure it has not other values besides the valid ones
	TESTC(BITCLEAR(ulValue,~DBPROPVAL_NP_ALL));

CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Properties - DBPROP_NOTIFIY [Reasons]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCProperties::Variation_3()
{
	TBEGIN
	ULONG_PTR ulValue;
	CRowset RowsetA;
	
	//DBPROP_NOTIFYCOLUMNSET
	ulValue = 0;
	TESTC_(RowsetA.CreateRowset(),S_OK);
	TESTC_PROVIDER(RowsetA.GetProperty(DBPROP_NOTIFYCOLUMNSET, DBPROPSET_ROWSET, &ulValue))
	TESTC(BITCLEAR(ulValue,~(DBPROPVAL_NP_OKTODO | DBPROPVAL_NP_ABOUTTODO | DBPROPVAL_NP_SYNCHAFTER)));

	//DBPROP_NOTIFYROWDELETE
	ulValue = 0;
	TESTC_PROVIDER(RowsetA.GetProperty(DBPROP_NOTIFYROWDELETE, DBPROPSET_ROWSET,  &ulValue))
	TESTC(BITCLEAR(ulValue,~(DBPROPVAL_NP_OKTODO | DBPROPVAL_NP_ABOUTTODO | DBPROPVAL_NP_SYNCHAFTER)));

	//DBPROP_NOTIFYROWFIRSTCHANGE
	ulValue = 0;
	TESTC_PROVIDER(RowsetA.GetProperty(DBPROP_NOTIFYROWFIRSTCHANGE, DBPROPSET_ROWSET,  &ulValue))
	TESTC(BITCLEAR(ulValue,~(DBPROPVAL_NP_OKTODO | DBPROPVAL_NP_ABOUTTODO | DBPROPVAL_NP_SYNCHAFTER)));
		
	//DBPROP_NOTIFYROWINSERT
	ulValue = 0;
	TESTC_PROVIDER(RowsetA.GetProperty(DBPROP_NOTIFYROWINSERT, DBPROPSET_ROWSET,  &ulValue))
	TESTC(BITCLEAR(ulValue,~(DBPROPVAL_NP_OKTODO | DBPROPVAL_NP_ABOUTTODO | DBPROPVAL_NP_SYNCHAFTER)));
	
	//DBPROP_NOTIFYROWRESYNCH
	ulValue = 0;
	TESTC_PROVIDER(RowsetA.GetProperty(DBPROP_NOTIFYROWRESYNCH, DBPROPSET_ROWSET,  &ulValue))
	TESTC(BITCLEAR(ulValue,~(DBPROPVAL_NP_OKTODO | DBPROPVAL_NP_ABOUTTODO | DBPROPVAL_NP_SYNCHAFTER)));
	
	//DBPROP_NOTIFYROWSETRELEASE
	ulValue = 0;
	TESTC_PROVIDER(RowsetA.GetProperty(DBPROP_NOTIFYROWSETRELEASE, DBPROPSET_ROWSET,  &ulValue))
	TESTC(BITCLEAR(ulValue,~(DBPROPVAL_NP_OKTODO | DBPROPVAL_NP_ABOUTTODO | DBPROPVAL_NP_SYNCHAFTER)));
	
	//DBPROP_NOTIFYROWSETFETCHPOSITIONCHANGE
	ulValue = 0;
	TESTC_PROVIDER(RowsetA.GetProperty(DBPROP_NOTIFYROWSETFETCHPOSITIONCHANGE, DBPROPSET_ROWSET,  &ulValue))
	TESTC(BITCLEAR(ulValue,~(DBPROPVAL_NP_OKTODO | DBPROPVAL_NP_ABOUTTODO | DBPROPVAL_NP_SYNCHAFTER)));
	
	//DBPROP_NOTIFYROWUNDOCHANGE
	ulValue = 0;
	TESTC_PROVIDER(RowsetA.GetProperty(DBPROP_NOTIFYROWUNDOCHANGE, DBPROPSET_ROWSET,  &ulValue))
	TESTC(BITCLEAR(ulValue,~(DBPROPVAL_NP_OKTODO | DBPROPVAL_NP_ABOUTTODO | DBPROPVAL_NP_SYNCHAFTER)));
	
	//DBPROP_NOTIFYROWUNDODELETE
	ulValue = 0;
	TESTC_PROVIDER(RowsetA.GetProperty(DBPROP_NOTIFYROWUNDODELETE, DBPROPSET_ROWSET,  &ulValue))
	TESTC(BITCLEAR(ulValue,~(DBPROPVAL_NP_OKTODO | DBPROPVAL_NP_ABOUTTODO | DBPROPVAL_NP_SYNCHAFTER)));
	
	//DBPROP_NOTIFYROWUNDOINSERT
	ulValue = 0;
	TESTC_PROVIDER(RowsetA.GetProperty(DBPROP_NOTIFYROWUNDOINSERT, DBPROPSET_ROWSET,  &ulValue));
	TESTC(BITCLEAR(ulValue,~(DBPROPVAL_NP_OKTODO | DBPROPVAL_NP_ABOUTTODO | DBPROPVAL_NP_SYNCHAFTER)));
	
	//DBPROP_NOTIFYROWUPDATE
	ulValue = 0;
	TESTC_PROVIDER(RowsetA.GetProperty(DBPROP_NOTIFYROWUPDATE, DBPROPSET_ROWSET,  &ulValue))
	TESTC(BITCLEAR(ulValue,~(DBPROPVAL_NP_OKTODO | DBPROPVAL_NP_ABOUTTODO | DBPROPVAL_NP_SYNCHAFTER)));

CLEANUP:
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Properties - DBPROP_REENTRANTEVENTS
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCProperties::Variation_4()
{
	TBEGIN
	VARIANT_BOOL bValue;
	CRowset RowsetA;
		
	//Not all providers will supports this property
	TESTC_(RowsetA.CreateRowset(),S_OK);
	TESTC_PROVIDER(RowsetA.GetProperty(DBPROP_REENTRANTEVENTS, DBPROPSET_ROWSET, &bValue))
	TESTC(bValue == VARIANT_TRUE || bValue == VARIANT_FALSE);

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
BOOL TCProperties::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CTestNotify::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCNotifyCanceled)
//*-----------------------------------------------------------------------
//| Test Case:		TCNotifyCanceled - Test CANCELED notifications
//|	Created:		08/06/96
//|	Updated:		12/01/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCNotifyCanceled::Init()
{
	//this test case is specific for Rowset object and IRowsetNotify 
	TBEGIN;
	TESTB = CTestNotify::Init();
	if (TEST_PASS == TESTB)
		TEST_PROVIDER(pCSource(0)->IsSupportedCP(IID_IRowsetNotify));

	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc CANCELED - IRowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCNotifyCanceled::Variation_1()
{
	TBEGIN
	HROW hRow = NULL;
	CListener* pCListener = NULL;
	DWORD dwCookie = 0;

	//Create Rowset
	CRowset Rowset;
	Rowset.SetProperty(DBPROP_CANHOLDROWS);
	TESTC_PROVIDER(Rowset.CreateRowset(DBPROP_IConnectionPointContainer)==S_OK)

	//Move the cursor from the first row.  This way when RestartPosition is called it "forces"
	//a FETCHPOSITIONCHANGE notification.  Otherwise, if we don't move the NFP ways from the 
	//head of the rowset, the provider can optimize and not return FETCHPOSITIONCHANGE for no-op
	//(no moving) senarios...
	TESTC_(Rowset.GetNextRows(&hRow),S_OK);
	TESTC_(Rowset.ReleaseRows(hRow),S_OK);

	//Create Listener
	pCListener = new CListener(IID_IRowsetNotify, Rowset());
	pCListener->AddRef();

	//Advise the connection, 
	TESTC_(pCListener->Advise(&dwCookie),S_OK);
	
	//Need to turn on FETCHPOSITION
	TESTC(pCListener->SetEvent(DBREASON_ROWSET_FETCHPOSITIONCHANGE, DBEVENTPHASE_ALL, ON));

	//Try GetNextRows -> Cancel
	TESTC(pCListener->SetCancel(DBREASON_ROWSET_FETCHPOSITIONCHANGE, DBEVENTPHASE_OKTODO));
	TESTC(pCListener->ResetTimesNotified());
	TESTC_(Rowset.GetNextRows(&hRow), pCListener->IsCancelable(DBREASON_ROWSET_FETCHPOSITIONCHANGE, DBEVENTPHASE_OKTODO) ? DB_E_CANCELED : S_OK);
	TESTC(pCListener->GetTimesNotified() > 0);

	//Try RestartPosition -> Cancel
	TESTC(pCListener->SetCancel(DBREASON_ROWSET_FETCHPOSITIONCHANGE, DBEVENTPHASE_OKTODO));
	TESTC(pCListener->ResetTimesNotified());
	TESTC_(Rowset.RestartPosition(), pCListener->IsCancelable(DBREASON_ROWSET_FETCHPOSITIONCHANGE, DBEVENTPHASE_OKTODO) ? DB_E_CANCELED : S_OK);
	TESTC(pCListener->GetTimesNotified() > 0);
	
CLEANUP:
	//Unadvise all connections
	if(pCListener && dwCookie)
	{
		pCListener->Unadvise(dwCookie);
		pCListener->ResetTimesNotified();
	}
	SAFE_RELEASE(pCListener);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc CANCELED - IRowsetChange
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCNotifyCanceled::Variation_2()
{
	TBEGIN

	HROW hNewRow = NULL;
	HROW hRow = NULL;
	DWORD dwCookie = 0;
	DBROWSTATUS rgRowStatus[ONE_ROW];
	CListener* pCListener = NULL;

	//Create Rowset
	CRowsetChange RowsetChange;
	RowsetChange.SetProperty(DBPROP_CANHOLDROWS);
	TESTC_PROVIDER(RowsetChange.CreateRowset(DBPROP_IConnectionPointContainer)==S_OK)
	
	//Create Listener
	pCListener = new CListener(IID_IRowsetNotify, RowsetChange());
	pCListener->AddRef();

	//Grab the row
	TESTC_(RowsetChange.GetRow(FIRST_ROW,&hRow),S_OK);

	//Advise the connection, 
	TESTC_(pCListener->Advise(&dwCookie),S_OK);

	//Need to setup the SetData/InsertRow pData buffer
	TESTC(RowsetChange.MakeRowData(&RowsetChange.m_pData));

	//Try InsertRow -> Cancel
	TESTC(pCListener->SetCancel(DBREASON_ROW_INSERT, DBEVENTPHASE_OKTODO));
	TESTC(pCListener->ResetTimesNotified());
	TESTC_(RowsetChange.InsertRow(&hNewRow), pCListener->IsCancelable(DBREASON_ROW_INSERT, DBEVENTPHASE_OKTODO) ? DB_E_CANCELED : S_OK);
	TESTC(pCListener->GetTimesNotified() > 0);

	//Need to setup the SetData/InsertRow pData buffer
	TESTC(RowsetChange.MakeRowData(&RowsetChange.m_pData));

	//Try SetData -> Cancel
	TESTC(pCListener->SetCancel(DBREASON_COLUMN_SET, DBEVENTPHASE_OKTODO));
	TESTC(pCListener->ResetTimesNotified());
	TESTC_(RowsetChange.ModifyRow(hRow), pCListener->IsCancelable(DBREASON_ROW_INSERT, DBEVENTPHASE_OKTODO) ? DB_E_CANCELED : S_OK);
	TESTC(pCListener->GetTimesNotified() > 0);

	//Try DeleteRow -> Cancel
	TESTC(pCListener->SetCancel(DBREASON_ROW_DELETE, DBEVENTPHASE_OKTODO));
	TESTC(pCListener->ResetTimesNotified());
	TESTC_(RowsetChange.DeleteRow(ONE_ROW,&hRow,rgRowStatus), pCListener->IsCancelable(DBREASON_ROW_INSERT, DBEVENTPHASE_OKTODO) ? DB_E_ERRORSOCCURRED : S_OK);
	TESTC(rgRowStatus && pCListener->IsCancelable(DBREASON_ROW_INSERT, DBEVENTPHASE_OKTODO) ? rgRowStatus[0]==DBROWSTATUS_E_CANCELED : rgRowStatus[0]==DBROWSTATUS_S_OK);
	TESTC(pCListener->GetTimesNotified() > 0);


CLEANUP:
	//Unadvise all connections
	if(pCListener && dwCookie)
	{
		pCListener->Unadvise(dwCookie);
		pCListener->ResetTimesNotified();
	}
	SAFE_RELEASE(pCListener);
	RowsetChange.ReleaseRows(hNewRow);
	RowsetChange.ReleaseRows(hRow);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc CANCELED - IRowsetUpdate
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCNotifyCanceled::Variation_3()
{
	TBEGIN

	HROW hRow = NULL;
	DWORD dwCookie = 0;
	DBROWSTATUS* rgUndoneStatus = NULL;
	DBROWSTATUS* rgUpdateStatus = NULL;
	CListener* pCListener = NULL;

	//Create Rowset
	CRowsetUpdate RowsetUpdate;
	RowsetUpdate.SetProperty(DBPROP_CANHOLDROWS);
	TESTC_PROVIDER(RowsetUpdate.CreateRowset(DBPROP_IConnectionPointContainer)==S_OK)
	
	//Create Listener
	pCListener = new CListener(IID_IRowsetNotify, RowsetUpdate());
	pCListener->AddRef();

	//Advise the connection, 
	TESTC_(pCListener->Advise(&dwCookie),S_OK);

	//Grab the row 
	TESTC_(RowsetUpdate.GetRow(FIRST_ROW,&hRow),S_OK);

	//Make a few changes
	TESTC_(RowsetUpdate.ModifyRow(hRow),S_OK);

	//Try Undo -> Cancel
	TESTC(pCListener->SetCancel(DBREASON_ROW_UNDOCHANGE, DBEVENTPHASE_OKTODO));
	TESTC(pCListener->ResetTimesNotified());
	TESTC_(RowsetUpdate.UndoRow(ONE_ROW,&hRow,NULL,NULL,&rgUndoneStatus), pCListener->IsCancelable(DBREASON_ROW_UNDOCHANGE, DBEVENTPHASE_OKTODO) ? DB_E_ERRORSOCCURRED : S_OK);
	TESTC(rgUndoneStatus && pCListener->IsCancelable(DBREASON_ROW_UNDOCHANGE, DBEVENTPHASE_OKTODO) ? rgUndoneStatus[0]==DBROWSTATUS_E_CANCELED : rgUndoneStatus[0]==DBROWSTATUS_S_OK);
	TESTC(pCListener->GetTimesNotified() > 0);

	//Try Update -> Cancel
	TESTC(pCListener->SetCancel(DBREASON_ROW_UPDATE, DBEVENTPHASE_OKTODO));
	TESTC(pCListener->ResetTimesNotified());
	TESTC_(RowsetUpdate.UpdateRow(ONE_ROW,&hRow,NULL,NULL,&rgUpdateStatus),pCListener->IsCancelable(DBREASON_ROW_UPDATE, DBEVENTPHASE_OKTODO) ? DB_E_ERRORSOCCURRED : S_OK);
	TESTC(rgUpdateStatus && pCListener->IsCancelable(DBREASON_ROW_UPDATE, DBEVENTPHASE_OKTODO) ? rgUpdateStatus[0]==DBROWSTATUS_E_CANCELED : rgUpdateStatus[0]==DBROWSTATUS_S_OK);
	TESTC(pCListener->GetTimesNotified() > 0);


CLEANUP:
	//Unadvise all connections
	if(pCListener && dwCookie)
	{
		pCListener->Unadvise(dwCookie);
		pCListener->ResetTimesNotified();
	}
	SAFE_RELEASE(pCListener);
	PROVIDER_FREE(rgUndoneStatus);
	PROVIDER_FREE(rgUpdateStatus);

	RowsetUpdate.ReleaseRows(hRow);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc CANCELED - IRowsetLocate
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCNotifyCanceled::Variation_4()
{
	// TO DO:  Add your own code here
	TOUTPUT(L"Variation is not implemented");
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc CANCELED - IRowsetResynch
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCNotifyCanceled::Variation_5()
{
	TBEGIN

	HROW hRow = NULL;
	DBROWSTATUS* rgRowStatus = NULL;
	CListener* pCListener = NULL;
	DWORD dwCookie = 0;

	//Create Rowset
	CRowsetUpdate RowsetUpdate;
	RowsetUpdate.SetProperty(DBPROP_CANHOLDROWS);
	RowsetUpdate.SetProperty(DBPROP_IRowsetResynch);
	TESTC_PROVIDER(RowsetUpdate.CreateRowset(DBPROP_IConnectionPointContainer)==S_OK);
	
	//Create Listener
	pCListener = new CListener(IID_IRowsetNotify, RowsetUpdate());
	pCListener->AddRef();

	//Advise the connection, 
	TESTC_(pCListener->Advise(&dwCookie),S_OK);

	//Grab the row 
	TESTC_(RowsetUpdate.GetRow(FIRST_ROW,&hRow),S_OK);

	//Make a few changes
	TESTC_(RowsetUpdate.ModifyRow(hRow),S_OK);
	
	//Try ResynchRows -> Cancel
	TESTC(pCListener->SetCancel(DBREASON_ROW_RESYNCH, DBEVENTPHASE_OKTODO));
	TESTC(pCListener->ResetTimesNotified());
	TESTC_(RowsetUpdate.ResynchRows(ONE_ROW,&hRow,NULL,NULL,&rgRowStatus), pCListener->IsCancelable(DBREASON_ROW_RESYNCH, DBEVENTPHASE_OKTODO) ? DB_E_ERRORSOCCURRED : S_OK);
	TESTC(rgRowStatus && pCListener->IsCancelable(DBREASON_ROW_RESYNCH, DBEVENTPHASE_OKTODO) ? rgRowStatus[0]==DBROWSTATUS_E_CANCELED : rgRowStatus[0]==DBROWSTATUS_S_OK);
	TESTC(pCListener->GetTimesNotified() > 0);


CLEANUP:
	//Unadvise all connections
	if(pCListener && dwCookie)
	{
		pCListener->Unadvise(dwCookie);
		pCListener->ResetTimesNotified();
	}

	SAFE_RELEASE(pCListener);
	PROVIDER_FREE(rgRowStatus);

	RowsetUpdate.ReleaseRows(hRow);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc CANCELED - IRowsetScroll
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCNotifyCanceled::Variation_6()
{
	// TO DO:  Add your own code here
	TOUTPUT(L"Variation is not implemented");
	return TEST_SKIPPED;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCNotifyCanceled::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CTestNotify::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCNotifyFailure)
//*-----------------------------------------------------------------------
//| Test Case:		TCNotifyFailure - Test E_FAIL from a listener
//|	Created:			01/20/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCNotifyFailure::Init()
{
	//this test case is specific for Rowset object and IRowsetNotify 
	TBEGIN;
	TESTB = CTestNotify::Init();
	if (TEST_PASS == TESTB)
		TEST_PROVIDER(pCSource(0)->IsSupportedCP(IID_IRowsetNotify));

	TRETURN
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc E_FAIL - IRowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCNotifyFailure::Variation_1()
{
	TBEGIN
	HROW hRow = NULL;
	CListener* pCListener = NULL;
	DWORD dwCookie = 0;

	//Create Rowset
	CRowset Rowset;
	Rowset.SetProperty(DBPROP_CANHOLDROWS);
	TESTC_PROVIDER(Rowset.CreateRowset(DBPROP_IConnectionPointContainer)==S_OK)

	//Create Listener
	pCListener = new CListener(IID_IRowsetNotify, Rowset());
	pCListener->AddRef();

	//Advise the connection, 
	TESTC_(pCListener->Advise(&dwCookie),S_OK);
	
	//Need to turn on FETCHPOSITION
	TESTC(pCListener->SetEvent(DBREASON_ROWSET_FETCHPOSITIONCHANGE, DBEVENTPHASE_ALL, ON));

	//Turn E_FAIL on for the listener
	TESTC(pCListener->SetError(E_FAIL));

	//Try GetNextRows -> E_FAIL
	TESTC(pCListener->ResetTimesNotified());
	TESTC_(Rowset.GetNextRows(&hRow),S_OK);
	TESTC(pCListener->GetTimesNotified() > 0);

	//Try RestartPosition -> E_FAIL
	TESTC_(Rowset.ReleaseRows(hRow),S_OK);
	TESTC(pCListener->ResetTimesNotified());
	TESTC_(Rowset.RestartPosition(),S_OK);
	TESTC(pCListener->GetTimesNotified() > 0);
	
CLEANUP:
	//Unadvise all connections
	if(pCListener && dwCookie)
	{
		pCListener->Unadvise(dwCookie);
		pCListener->ResetTimesNotified();
	}
	SAFE_RELEASE(pCListener);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc E_FAIL - IRowsetChange
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCNotifyFailure::Variation_2()
{
	TBEGIN

	HROW hNewRow = NULL;
	HROW rghRows[TWO_ROWS] = {NULL,NULL};
	CListener* pCListener = NULL;
	DWORD dwCookie = 0;

	//Create Rowset
	CRowsetChange RowsetChange;
	RowsetChange.SetProperty(DBPROP_CANHOLDROWS);
	TESTC_PROVIDER(RowsetChange.CreateRowset(DBPROP_IConnectionPointContainer)==S_OK)
	
	//Create Listener
	pCListener = new CListener(IID_IRowsetNotify, RowsetChange());
	pCListener->AddRef();

	//Grab the row
	TESTC_(RowsetChange.GetRow(FIRST_ROW, TWO_ROWS, rghRows),S_OK);

	//Advise the connection, 
	TESTC_(pCListener->Advise(&dwCookie),S_OK);

	//Turn E_FAIL on for the listener
	TESTC(pCListener->SetError(E_FAIL));

	//Try InsertRow -> E_FAIL
	TESTC(pCListener->ResetTimesNotified());
	TESTC_(RowsetChange.InsertRow(&hNewRow),S_OK);
	TESTC(pCListener->GetTimesNotified() > 0);

	//Try SetData -> E_FAIL
	TESTC(pCListener->ResetTimesNotified());
	TESTC_(RowsetChange.ModifyRow(rghRows[ROW_ONE]),S_OK);
	TESTC(pCListener->GetTimesNotified() > 0);

	//Try DeleteRow -> E_FAIL
	TESTC(pCListener->ResetTimesNotified());
	TESTC_(RowsetChange.DeleteRow(rghRows[ROW_TWO]),S_OK);
	TESTC(pCListener->GetTimesNotified() > 0);


CLEANUP:
	//Unadvise all connections
	if(pCListener && dwCookie)
	{
		pCListener->Unadvise(dwCookie);
		pCListener->ResetTimesNotified();
	}
	SAFE_RELEASE(pCListener);
	RowsetChange.ReleaseRows(hNewRow);
	RowsetChange.ReleaseRows(TWO_ROWS, rghRows);
	TableInsert(ONE_ROW);	//Adjust the table
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc E_FAIL - IRowsetUpdate
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCNotifyFailure::Variation_3()
{
	TBEGIN

	HROW hRow = NULL;
	DBROWSTATUS* rgUndoneStatus = NULL;
	DBROWSTATUS* rgUpdateStatus = NULL;
	CListener* pCListener = NULL;
	DWORD dwCookie = 0;

	//Create Rowset
	CRowsetUpdate RowsetUpdate;
	RowsetUpdate.SetProperty(DBPROP_CANHOLDROWS);
	TESTC_PROVIDER(RowsetUpdate.CreateRowset(DBPROP_IConnectionPointContainer)==S_OK)
	
	//Create Listener
	pCListener = new CListener(IID_IRowsetNotify, RowsetUpdate());
	pCListener->AddRef();

	//Advise the connection, 
	TESTC_(pCListener->Advise(&dwCookie),S_OK);

	//Grab the row 
	TESTC_(RowsetUpdate.GetRow(FIRST_ROW,&hRow),S_OK);

	//Make a few changes
	TESTC_(RowsetUpdate.ModifyRow(hRow),S_OK);

	//Turn E_FAIL on for the listener
	TESTC(pCListener->SetError(E_FAIL));

	//Try Undo -> E_FAIL
	TESTC(pCListener->ResetTimesNotified());
	TESTC_(RowsetUpdate.UndoRow(hRow),S_OK);
	TESTC(pCListener->GetTimesNotified() > 0);

	//Try Update - should never be notified for Rows 
	//not actually needing updating on the backend (2.0 spec change)
	TESTC(pCListener->SetError(S_OK));
	TESTC(pCListener->ResetTimesNotified());
	TESTC_(RowsetUpdate.UpdateRow(hRow),S_OK);

	TESTC(pCListener->GetTimesNotified() == 0);

	//Make another change
	TESTC_(RowsetUpdate.ModifyRow(hRow),S_OK);

	//Try Update - E_FAIL
	TESTC(pCListener->SetError(E_FAIL));
	TESTC(pCListener->ResetTimesNotified());
	TESTC_(RowsetUpdate.UpdateRow(hRow),S_OK);
	TESTC(pCListener->GetTimesNotified() > 0);

CLEANUP:
	//Unadvise all connections
	if(pCListener && dwCookie)
	{
		pCListener->Unadvise(dwCookie);
		pCListener->ResetTimesNotified();
	}
	SAFE_RELEASE(pCListener);
	PROVIDER_FREE(rgUndoneStatus);
	PROVIDER_FREE(rgUpdateStatus);

	RowsetUpdate.ReleaseRows(hRow);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc E_FAIL - IRowsetLocate
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCNotifyFailure::Variation_4()
{
	// TO DO:  Add your own code here
	TOUTPUT(L"Variation is not implemented");
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc E_FAIL - IRowsetResynch
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCNotifyFailure::Variation_5()
{
	TBEGIN

	HROW hRow = NULL;
	DBROWSTATUS* rgRowStatus = NULL;
	CListener* pCListener = NULL;
	DWORD dwCookie = 0;

	//Create Rowset
	CRowsetUpdate RowsetUpdate;
	RowsetUpdate.SetProperty(DBPROP_CANHOLDROWS);
	RowsetUpdate.SetProperty(DBPROP_IRowsetResynch);
	TESTC_PROVIDER(RowsetUpdate.CreateRowset(DBPROP_IConnectionPointContainer)==S_OK);
	
	//Create Listener
	pCListener = new CListener(IID_IRowsetNotify, RowsetUpdate());
	pCListener->AddRef();

	//Advise the connection, 
	TESTC_(pCListener->Advise(&dwCookie),S_OK);

	//Grab the row 
	TESTC_(RowsetUpdate.GetRow(FIRST_ROW,&hRow),S_OK);

	//Make a few changes
	TESTC_(RowsetUpdate.ModifyRow(hRow),S_OK);
	
	//Set E_FAIL on for the listener
	TESTC(pCListener->SetError(E_FAIL));

	//Try ResynchRows -> Cancel
	TESTC(pCListener->ResetTimesNotified());
	TESTC_(RowsetUpdate.ResynchRows(ONE_ROW,&hRow,NULL,NULL,&rgRowStatus),S_OK);
	TESTC(rgRowStatus && rgRowStatus[0]==DBROWSTATUS_S_OK);
	TESTC(pCListener->GetTimesNotified() > 0);


CLEANUP:
	//Unadvise all connections
	if(pCListener && dwCookie)
	{
		pCListener->Unadvise(dwCookie);
		pCListener->ResetTimesNotified();
	}

	SAFE_RELEASE(pCListener);
	PROVIDER_FREE(rgRowStatus);

	RowsetUpdate.ReleaseRows(hRow);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc E_FAIL - IRowsetScroll
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCNotifyFailure::Variation_6()
{
	// TO DO:  Add your own code here
	TOUTPUT(L"Variation is not implemented");
	return TEST_SKIPPED;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCNotifyFailure::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return CTestNotify::Terminate();
}	// }}
// }}
// }}
