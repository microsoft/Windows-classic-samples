//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright (C) 1995-2000 Microsoft Corporation
//
// @doc  
//
// @module IRNotify.h | This module is a general library for interface testing 
//

//@end
//
//@doc OVERVIEW
//@topic Overview |
// The Notifications test module is composed of five classes and a YACC generated parser.
// The five classes are: <c CControl>, <c CImpIListener>, <c CExecutionManager>,
// <c CExpectationsManager> and <c CRowsetObj>.
//
// The overall intent of the classes is to emulate an arbitrary set of custom controls
// (class CControl), each on its own thread, interacting with an arbitray set of OLE DB 
// listeners (class CImpIListener) receiving notifications from OLE DB. 
// Both the emulated custom controls and the listeners can execute OLE DB methods 
// that cause notifications.
//
// Both the custom controls and listeners cycle thru a loop of verifying expectations
// from a previous OLE DB operation (via CExpectationsManager), determing the next command
// to execute (via CExectionManager), dynamically stacking the next set of expectations
// due to this next command (via CExpectationsManager), and then executing the command
// (via CExecutionManager).
//
// The purpose of the YACC generated parser is to parse a string of nested OLE DB methods
// as an input parameter to the TMD variation and build a list of <c COMMAND> structures 
// for each control and for each listener on each thread. During the execution
// of the test variation, the CExecutionManager removes the top COMMAND from the 
// appropriate list and passes it back to the requesting control or listener. 
// The CExecutionManager is also responsible for executing the COMMAND when requested 
// to do so by a control or listener.
// 
// The CExpectationsManager is responsible for generating and verifying a stack of 
// <c EXPECT>'s for each control and for each listener per thread. An EXPECT 
// structure contains the data that is expected to be returned by OLE DB during 
// a notification via the parameters of the listener callback change method or 
// the HRESULT status of the OLE DB method. 
//
// The purpose of the CRowsetObj class is to encapsulate the OLE DB methods and to
// allocate and manage the HROW's for each thread. Whenever
// a set of HROW's is needed for an OLE DB method, the CRowsetObj returns a 
// NDATA structure containing HROW's created on the current thread along with 
// the expected data contents in those rows. The CExecutionManager uses the 
// NDATA structure contents to execute the OLE DB method, and the CExpectationsManager 
// uses the NDATA structure contents to verify that the proper rows
// were returned by the OLE DB notification callback.
// 
//@end

#ifndef _IRNOTIFY_H_
#define _IRNOTIFY_H_


///////////////////////////////////////////////////////////////////
// Includes
//
///////////////////////////////////////////////////////////////////
#include "oledb.h"
#include "oledberr.h"
#include "privlib.h"
#include "ExtraLib.h"

///////////////////////////////////////////////////////////////////
// Forwards
//
///////////////////////////////////////////////////////////////////
class CRowsetObj;
class CExecutionManager;
class CExpectationsManager;
class CControl;
class CImpIListener;
class ROWINFO;
class EXPECT;
class COMMAND;


///////////////////////////////////////////////////////////////////
// MAP
//
///////////////////////////////////////////////////////////////////
typedef CList<COMMAND*,COMMAND*> COMMANDLIST;
typedef CMap<DWORD,DWORD,COMMANDLIST*,COMMANDLIST*&> COMMANDLISTMAP;

typedef CMap<HROW,HROW, ROWINFO*, ROWINFO*> ROWINFOMAP;
typedef CMap<ULONG,ULONG, CControl*,CControl*> CONTROLMAP;
typedef CMap<ULONG,ULONG, CImpIListener*,CImpIListener*> LISTENERMAP;

typedef CList<EXPECT*,EXPECT*> LISTEXPECT;


///////////////////////////////////////////////////////////////////
// Defines
//
///////////////////////////////////////////////////////////////////
const int MAX_ROW_COUNT = 100; 
const int MAX_ROWSET_COUNT = 3;

#define REMOVE TRUE

//YACC
extern LPCWSTR				yylpwzInput;
extern CExecutionManager*	yypExecutionManager;
extern COMMANDLIST*			yypCommandList;
extern COMMANDLISTMAP*		yypCommandListMap;
extern CONTROLMAP*			yypControlMap;
extern LISTENERMAP*			yypListenerMap;
extern int yyparse();
extern int yylexInit();


typedef DWORD ROWSET_MODE;
enum ROWSET_MODE_ENUM
{
	EMPTY_ROWSET		= 0x00000001,
	DEFAULT_ROWSET		= 0x00000002,
	ROWSETSCROLL_ROWSET	= 0x00000004,
	RESYNCH_ROWSET		= 0x00000008,
	CHANGE_IMMEDIATE	= 0x00000010,
	CHANGE_BUFFERRED	= 0x00000020,
	CHANGE_QBU			= 0x00000040,
	COMPUTED_COLUMNS	= 0x00000080,
	SCROLLBACKWARDS		= 0x00000100,
	FETCHBACKWARDS		= 0x00000200,
	ROWSETINDEX_ROWSET  = 0x00000400,
	ROWSETLOCATE_ROWSET = 0x00000800,
	REFRESH_ROWSET		= 0x00001000,
	ASYNCH_ROWSET		= 0x00002000,
	COMPUTED_COLUMNS_INCLUDE	= 0x00004000
};

enum COMMANDTYPE 
{
	  EMPTY,				//Empty Command
	  
	  RETURN_ACCEPT,		//Return S_OK (Accept)	 
	  RETURN_VETO,			//Return S_FALSE (Veto)

	  RETURN_EFAIL,			//Return E_FAIL
	  RETURN_EOUTOFMEMORY,	//Return E_OUTOFMEMORY
	  RETURN_EINVALIDARG,	//Return E_INVALIDARG
	  
	  RETURN_UNWANTEDREASON,//Return DB_S_UNWANTEDREASON
	  RETURN_UNWANTEDPHASE, //Return DB_S_UNWANTEDPHASE
	  
	  RETURN_ADVISE,		//Return S_OK, but Advise another listener
	  RETURN_UNADVISE,		//Return S_OK, but Unadvise the listener
	  
	  GETDATA,				//IRowset::GetData
	  GETORIGINALDATA,		//IRowsetUpdate::GetOriginalData
	  GETVISIBLEDATA,		//IRowsetResynch::GetVisibleData
	  GETLASTVISIBLEDATA,	//IRowsetRefresh::GetLastVisibleData
	  
	  SETDATA,				//IRowsetChange::SetData
	  DELETEROWS,			//IRowsetDelete::DeleteRows 
	  INSERTROW,			//IRowsetChange::InsertRow 
	  UPDATE,				//IRowsetUpdate::Update
	  UNDO,					//IRowsetUpdate::Undo
	  ADDREFROWS,			//IRowset::AddRefRows
	  RELEASEROWS,			//IRowset::ReleaseRows
	  RESYNCHROWS,			//IRowsetResynch::ResyncRows
	  REFRESHROWS,			//IRowsetRefresh::RefreshVisibleData
	  GETNEXTROWS,			//IRowset::GetNextRows
	  GETROWSAT,			//IRowsetLocate::GetRowsAt
	  GETROWSATRATIO,		//IRowsetScroll::GetRowsAtRatio
	  GETROWSBYBOOKMARK,	//IRowsetLocate::GetRowsByBookmark
	  RESTARTPOSITION,		//IRowset::RestartPosition
	  SEEK,					//IRowsetIndex::Seek
	  ADDREFROWSET,			//IRowset AddRef
	  RELEASEROWSET,		//IRowset Release

	  ABORT,				//ITransaction::Abort
	  COMMIT,				//ITransaction::Commit

	  ADDCOLUMN,			//ITableDefinition::AddColumn
	  DROPCOLUMN,			//ITableDefinition::DropColumn
	  GETCOLUMNINFO,		//IColumnsInfo::GetColumnInfo
};


enum REASONTYPE
{
	REASONTYPE_INVALID	= 0,
	REASONTYPE_ROWSET	= 1,
	REASONTYPE_ROW		= 2,
	REASONTYPE_COLUMN	= 3,
};

enum ROWSTATUS
{
	ROWSTATUS_NOCHANGE	= 0,
	ROWSTATUS_CHANGED	= 1,
	ROWSTATUS_DELETED	= 2,
	ROWSTATUS_INSERTED	= 3,
	ROWSTATUS_INVALID	= 4,
};

///////////////////////////////////////////////////////////////////
// Helper Functions
//
///////////////////////////////////////////////////////////////////
BOOL IsValidEvent(DBREASON eReason, DBEVENTPHASE ePhase = DBEVENTPHASE_DIDEVENT);

REASONTYPE ReasonType(DBREASON eReason);
BOOL IsMultiPhaseReason(DBREASON eReason);
//BOOL DisplayNotification(WCHAR* pwszHeader, DBCOUNTITEM cRows, HROW* rghRows, DBORDINAL cColumns, DBORDINAL* rgColumns, DBREASON eReason, DBEVENTPHASE ePhase, BOOL fError = TRUE);
BOOL DisplayNotification(WCHAR* pwszHeader, DBCOUNTITEM cRows, const HROW rghRows[], DBORDINAL cColumns, DBORDINAL* rgColumns, DBREASON eReason, DBEVENTPHASE ePhase, BOOL fError = TRUE);


///////////////////////////////////////////////////////////////////
// COMMAND
//
///////////////////////////////////////////////////////////////////
class COMMAND
{
public:
	//Constructors
	COMMAND(COMMANDTYPE eCommandType);
	COMMAND(COMMANDTYPE eCommandType, void* pvObjectId, ULONG ulCommandLevel, ULONG cThreads, DBREASON eReason, DBEVENTPHASE ePhase, HRESULT hrExpected	= S_OK,	DBCOUNTITEM  cRows = 0, ULONG* rgRowIds = NULL, DBORDINAL cColumns = 0, ULONG* rgColumns = NULL, DWORD dwOption = 0);
	virtual ~COMMAND();

	//methods
	virtual BOOL DisplayCommand();

	//Data
	void*         m_pvObjectId;
	COMMANDTYPE   m_eCommandType;
	ULONG         m_ulCommandLevel;

	DBREASON      m_eReason;
	DBEVENTPHASE  m_ePhase;

	DBCOUNTITEM         m_cRows;
	DBCOUNTITEM		  m_rgRowIds[MAX_ROW_COUNT];
	HROW		  m_rghRows[MAX_ROW_COUNT];

	DBORDINAL       m_cColumns;
	DBORDINAL*		  m_rgColumns;
	DWORD		  m_dwOption;

	HRESULT		  m_hrExpected;
	BOOL		  m_fCanceled;
	LISTEXPECT	  m_listExpect;
	
	ULONG		  m_cThreads;
};



///////////////////////////////////////////////////////////////////
// ROWINFO
//
///////////////////////////////////////////////////////////////////
class ROWINFO
{
public:
	//Construtor
	ROWINFO(HROW hRow);
	virtual ~ROWINFO();

	ROWSTATUS	m_eRowStatus;
	HROW		m_hRow;
	ULONG		m_cRefCount;

	DBBKMARK	m_cbBookmark;
	BYTE*		m_pBookmark;
	BOOL		m_fFirstChange;

	void*		m_pNewData;
	void*		m_pOrgData;
};



///////////////////////////////////////////////////////////////////
// EXPECT
//
///////////////////////////////////////////////////////////////////
class EXPECT
{
public:
	//Constructors
	EXPECT(CImpIListener* pListener, IRowset* pIRowset, DBCOUNTITEM cRows, HROW* rghRows, DBORDINAL cColumns, DBORDINAL rgColumns[], DBREASON eReason, DBEVENTPHASE ePhase);
	EXPECT(const EXPECT& Expectation);
	virtual ~EXPECT();

	BOOL SetPhase(DBEVENTPHASE ePhase);
	BOOL SetNextPhase();

	BOOL IsEqual(DBEVENTPHASE ePhase);
	BOOL IsEqual(CImpIListener* pListener, IRowset* pIRowset, DBCOUNTITEM cRows, HROW* rghRows, DBORDINAL cColumns, DBORDINAL rgColumns[], DBREASON eReason, DBEVENTPHASE ePhase);
	BOOL IsCompleted() { return m_bCompleted; };

	BOOL VerifyEqual(DBEVENTPHASE ePhase);
	BOOL VerifyEqual(CImpIListener* pListener, IRowset* pIRowset, DBCOUNTITEM cRows, HROW* rghRows, DBORDINAL cColumns, DBORDINAL rgColumns[], DBREASON eReason, DBEVENTPHASE ePhase);
	BOOL DisplayExpectation(WCHAR* pwszTitle = L"Expected:     ");

	BOOL VerifyComplete();
	BOOL VerifyInComplete();

	CImpIListener*	m_pListener;
	IRowset*		m_pIRowset;
	
	DBREASON		m_eReason;
	DBEVENTPHASE	m_ePhase;

	DBCOUNTITEM			m_cRows;
	HROW			m_rghRows[MAX_ROW_COUNT];
	DBORDINAL			m_cColumns;
	DBORDINAL*			m_rgColumns;

	BOOL			m_fCanceled;
	ULONG			m_ulTimesNotified;
	BOOL			m_bCompleted;
	BOOL			m_fOptional;
};



/////////////////////////////////////////////////////////////////
// CRowsetObj
//
/////////////////////////////////////////////////////////////////
class CRowsetObj : public CRowset
{
public:
	//Constructor
	CRowsetObj();
	virtual ~CRowsetObj();

	//Helper functions
	HRESULT		CreateRowset(ROWSET_MODE dwRowsetMode);
	BOOL		DestroyRowData();
	BOOL		GetRowsToRelease(DBCOUNTITEM cRows, DBCOUNTITEM   &cRowsObtained, HROW *rghRows);
	BOOL		MapRowIds(COMMAND* pCommand);

	BOOL		GetNotificationProperties();
	BOOL		GetReentrantEvents();
	ULONG_PTR		GetGranularity();
	ULONG_PTR		GetSupportedPhases();
	BOOL		IsQuickRestart();
	BOOL		IsSupportedPhase(DBEVENTPHASE ePhase);
	BOOL		IsCancelableEvent(DBREASON eReason, DBEVENTPHASE ePhase);

	//OLEDB Methods
	HRESULT		SetData(COMMAND* pCommand);
	HRESULT		DeleteRows(COMMAND* pCommand);
	HRESULT		GetNextRows(COMMAND* pCommand);
	HRESULT		GetRowsAt(COMMAND* pCommand);
	HRESULT		GetRowsByBookmark(COMMAND* pCommand);
	HRESULT		InsertRow(COMMAND* pCommand);
	HRESULT		Update(COMMAND* pCommand);
	HRESULT		Undo(COMMAND* pCommand);
	HRESULT		AddRefRows(COMMAND* pCommand);
	HRESULT		ReleaseRows(COMMAND* pCommand);
	HRESULT		ResynchRows(COMMAND* pCommand);
//	HRESULT		RefreshVisibleData(COMMAND* pCommand);
	HRESULT		RestartPosition(COMMAND* pCommand);
	HRESULT		GetRowsAtRatio(COMMAND* pCommand);
	
	HRESULT		GetData(COMMAND* pCommand);
	HRESULT		GetOriginalData(COMMAND* pCommand);
	HRESULT		GetVisibleData(COMMAND* pCommand);
	HRESULT		GetLastVisibleData(COMMAND* pCommand);

	HRESULT		AddColumn();
	HRESULT		DropColumn();
	HRESULT		GetColumnInfo();

	ULONG		AddRefRowset();
	ULONG		ReleaseRowset();

	HRESULT		GetData(HROW hRow, void* pData);
	HRESULT		GetOriginalData(HROW hRow, void* pData);
	HRESULT		GetVisibleData(HROW hRow, void* pData);
	HRESULT		GetLastVisibleData(HROW hRow, void* pData);
	HRESULT		GetBookmark(HROW hRow, ROWINFO* pRowInfo);

	//Interface methods
	ROWINFO*	GetRowInfo(DBCOUNTITEM ulRowId);
	BOOL		IsBufferedMode();

	//Rowset reference
	ULONG		m_cRowsetRef;
	BOOL		m_fTableAltered;
	DBCOUNTITEM		m_ulNextFetchPos;
	ROWSET_MODE m_dwRowsetMode;

	//SetData(Updatable) Accessor and bindings.
	DBORDINAL		m_cUpBindings;
	DBBINDING*	m_rgUpBindings;
	HACCESSOR	m_hUpAccessor;

protected:
	//OLE DB Notification Properties
	ULONG_PTR		m_ulSupportedGranularity;
	ULONG_PTR		m_ulSupportedPhases;
	
	BOOL m_bQuickRestart;

	BOOL		m_bReentrantEvents;
	ULONG_PTR		m_rgCancelableEvents[DBREASON_ALL];

	CRITICAL_SECTION    m_DataCriticalSection;
	ROWINFOMAP	m_mapRowInfo;
};



/////////////////////////////////////////////////////////////////
// CImpIListener
//
/////////////////////////////////////////////////////////////////
class CImpIListener : public IRowsetNotify
{
public:
	CImpIListener(CExecutionManager* pExecutionManager);
	virtual ~CImpIListener();

	//@cmember OLE AddRef
	STDMETHODIMP_(ULONG) AddRef(void);
	STDMETHODIMP_(ULONG) Release(void);
	STDMETHODIMP  QueryInterface(REFIID riid, void** ppv);

	//@cmember Notification Callback for Field Change
    STDMETHODIMP OnFieldChange( 
        IRowset* pRowset,
        HROW hRow,
        DBORDINAL cColumns,
		DBORDINAL rgColumns[],
        DBREASON eReason,
        DBEVENTPHASE ePhase,
        BOOL fCantDeny);

    //@cmember Notification Callback for Row Change
    STDMETHODIMP OnRowChange( 
        IRowset* pRowset,
        DBCOUNTITEM cRows,
        const HROW rghRows[],
        DBREASON eReason,
        DBEVENTPHASE ePhase,
        BOOL fCantDeny);

    //@cmember Notification Callback for Rowset Change
    STDMETHODIMP OnRowsetChange( 
        IRowset*     pRowset,
        DBREASON     eReason,
		DBEVENTPHASE ePhase,
		BOOL         fCantDeny);

    virtual HRESULT AcceptOrVeto(
		IRowset* pRowset, 
		DBCOUNTITEM cRows, 
		const HROW rghRows[],
		DBORDINAL cColumns,	
		DBORDINAL rgColumns[], 
		DBREASON eReason,
		DBEVENTPHASE ePhase, 
		BOOL fCantDeny);

	CExecutionManager* pExecutionManager();
	CExpectationsManager* pExpectationsManager();

	BOOL IsWantedReason(DBREASON eReason);
	BOOL IsWantedPhase(DBREASON eReason, DBEVENTPHASE ePhase);
	
	BOOL SetUnwantedReason(DBREASON eReason);
	BOOL SetUnwantedPhase(DBREASON eReason, DBEVENTPHASE ePhase);
	DBEVENTPHASE NextPhase(DBREASON eReason, DBEVENTPHASE ePhase);

	HRESULT Advise(IRowset* pIRowset);
	HRESULT Unadvise(IRowset* pIRowset);

	BOOL ResetTimesNotified(DBREASON eReason = DBREASON_ALL, DBEVENTPHASE ePhase = DBEVENTPHASE_ALL);
	ULONG GetTimesNotified(DBREASON eReason = DBREASON_ALL, DBEVENTPHASE ePhase = DBEVENTPHASE_ALL);
	
protected:
	CExecutionManager*  m_pExecutionManager;
	ULONG				m_cRef;
	DWORD               m_dwCookie;

	ULONG m_rgWantedEvent[DBREASON_ALL][DBEVENTPHASE_ALL];
	ULONG m_rgTimesNotified[DBREASON_ALL][DBEVENTPHASE_ALL];
};



/////////////////////////////////////////////////////////////////
// CControl
//
/////////////////////////////////////////////////////////////////
class CControl
{
public:
	CControl(CExecutionManager* pExecutionManager);
	virtual ~CControl();

	ULONG Execute();
	static ULONG WINAPI ThreadFunction(void* pv);

	CExecutionManager* pExecutionManager();
	CExpectationsManager* pExpectationsManager();
		
	//Interface methods
	DWORD GetThreadId();
	DWORD WaitForThread();
		
protected:
	CExecutionManager*  m_pExecutionManager;

	HANDLE				m_hThread;
	DWORD				m_dwThreadId;
};


/////////////////////////////////////////////////////////////////
// CExecutionManager
//
/////////////////////////////////////////////////////////////////
class CExecutionManager : public CTestCases
{
public:
	CExecutionManager(const WCHAR* pwszTestCaseName);
	virtual ~CExecutionManager();

	BOOL	 Init();
	BOOL	 Terminate();

	ULONG    Execute(LPCWSTR CommmandString, ROWSET_MODE dwRowsetMode);
	BOOL     ParseCommandString(LPCWSTR CommandString);
	BOOL     DestroyStacks();
	HRESULT	 CreateRowset(ROWSET_MODE dwRowsetMode);

	BOOL	 AdviseListeners(CImpIListener* pListener = NULL);
	BOOL     UnadviseListeners(CImpIListener* pListener = NULL);
	BOOL	 IsCancelableEvent(DBREASON eReason, DBEVENTPHASE ePhase);
	BOOL		GetReentrantEvents();


	BOOL     RunControls();
	BOOL	 ExecuteCommand(COMMAND* pCommand);
	static ULONG WINAPI	 Thread_ExecuteCommand(void* pv);

	void PushCommand(COMMAND * pCommand);
	COMMAND* PopCommand(CControl* pControl = NULL);
	COMMAND* PopCommand(CImpIListener* pListener, DBREASON eReason, DBEVENTPHASE ePhase);
	BOOL     IsCommandListEmpty();
	COMMANDLIST* LookupCommandList();

	POSITION      GetListenerStartPosition();
	CImpIListener* NextListener(POSITION &pos);

	CRowsetObj* pCRowsetObj();
	CExpectationsManager* pExpectationsManager();
	COMMAND* GetCurrentCommand();

protected:
	CRowsetObj* m_pCRowsetObj;
	CExpectationsManager* m_pExpectationsManager;
	COMMAND* m_pCurrentCommand;

	COMMANDLISTMAP        m_mapCommandList;
	CONTROLMAP            m_mapControl;
	LISTENERMAP           m_mapListener;
	BOOL				  m_fFreeThreaded;
};


/////////////////////////////////////////////////////////////////
// CExpectationsManager
//
/////////////////////////////////////////////////////////////////
class CExpectationsManager
{
public:
	CExpectationsManager(CExecutionManager* pExecutionManager);
	virtual ~CExpectationsManager();

	//@cmember Push expectations on stack for OLE DB method that causes notifications
	BOOL PushNestedExpectation(COMMAND* pCommand);
	BOOL PopNestedExpectation(COMMAND* pCommand);
	
	BOOL PushNextExpectation(EXPECT* pExpect);
	BOOL PushFalseExpectation(EXPECT* pExpect);
	
	BOOL PushAdviseExpectation(EXPECT* pExpect);
	BOOL PushUnadviseExpectation(EXPECT* pExpect);

	BOOL PushUnwantedReason(CImpIListener* pListener, IRowset* pIRowset, DBREASON eReason);
	BOOL PushUnwantedPhase(CImpIListener* pListener, IRowset* pIRowset, DBREASON eReason, DBEVENTPHASE ePhase);

	EXPECT* FindExpectation(CImpIListener* pListener, IRowset* pIRowset, DBCOUNTITEM cRows, HROW* rghRows, DBORDINAL cColumns, DBORDINAL rgColumns[], DBREASON eReason, DBEVENTPHASE ePhase, BOOL fCantDeny);
	EXPECT* PopExpectation(CImpIListener* pListener, IRowset* pIRowset, DBCOUNTITEM cRows, HROW* rghRows, DBORDINAL cColumns, DBORDINAL rgColumns[], DBREASON eReason, DBEVENTPHASE ePhase);
	
	CRowsetObj* pCRowsetObj();
	CExecutionManager* pExecutionManager();

protected:
	CExecutionManager*  m_pExecutionManager;
};

#endif 	//_IRNOTIFY_H_
