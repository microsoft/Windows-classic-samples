//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1995-2000 Microsoft Corporation.  
//
// @doc
//
// @module TXNBASE.HPP | Base classes for transacted rowsets.
//
// @rev 01 | 02-04-96 | Microsoft | Created
// @rev 02 | 12-01-96 | Microsoft | Updated
//

#include "oledb.h" 			// OLE DB Header Files
#include "oledberr.h"
#include "transact.h"

#include "privlib.h"		//Include private library, which includes
#include "msdasql.h"		//Kagera specific header file

//--------------------------------------------------------------------
//Externs Defined in TxnBase.cpp
//--------------------------------------------------------------------
extern ULONG	g_ulLastActualDelete;
extern ULONG	g_ulLastActualInsert;
extern ULONG	g_ulFirstRowInTable;

extern void g_DeleteAndInsertIncrement();
extern void g_InsertIncrement();
extern void g_DeleteIncrement();
					
enum EASYNC 
{
	EASYNCTRUE, 
	EASYNCFALSE, 
	EASYNCNOTSUPPORTED
};

enum ePrptIdx	
{
	IDX_CommitPreserve=0,
	IDX_AbortPreserve,
	IDX_OtherUpdateDelete,	
	IDX_OtherInsert,
	IDX_CommandTimeout,
	IDX_IRowsetUpdate,
	IDX_IRowsetChange
};

const ULONG	g_PROPERTY_COUNT = IDX_IRowsetChange+1;

typedef struct	tagDBPrptRecord
{
	DBPROPID		dwPropID;
	BOOL			fSupported;
	VARIANT_BOOL	fDefault;
	DBPROPFLAGS		dwFlags;
	BOOL			fProviderSupported;
}DBPrptRecord;

extern DBPrptRecord g_rgDBPrpt[g_PROPERTY_COUNT];


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Defines
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//Use a value not valid for DBPROP_SUPPORTEDTXNDDL to represent that it is not a supported property
#define NOT_SUPPORTED DBPROPVAL_TC_NONE | DBPROPVAL_TC_DML | DBPROPVAL_TC_DDL_COMMIT | DBPROPVAL_TC_DDL_IGNORE | DBPROPVAL_TC_ALL

//Number of milliseconds need to wait before Jet refreshes to recognize changes
//There is supposedly only a 2 second lag time in Jet, but wait 5 seconds just to be sure
#define SLEEP_TIME	7000

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Base Class Declarations
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

//--------------------------------------------------------------------
// @class CTxnRowset | Transactable rowset to be encapsulated by CTxn
// @base public | CRowsetObject
//--------------------------------------------------------------------
class CTxnRowset : public CRowsetObject
{
// @access public
public:
	
	//@cmember Transaction Local interface for this session
	ITransactionLocal	*m_pITxnLocal;
//#ifdef TXNJOIN
	//@cmember Transaction Join interface for this session
	ITransactionJoin	*m_pITxnJoin;
//#endif//TXNJOIN
	//@cmember Transaction Join interface for this session
	CTable				*m_pCTable;
	//@cmember Count of bindings for GetData
	DBCOUNTITEM			m_cReadBindings;
	//@cmember Array of bindings for GetData
	DBBINDING			*m_rgReadBindings;
	//@cmember Accessor for GetData
	HACCESSOR			m_hReadAccessor;
	//@cmember Row size for GetData
	DBLENGTH			m_cbReadRowSize;
	//@cmember Flag indicating whether rowset is preserved on abort
	BOOL				m_fAbortPreserve;
	//@cmember Flag indicating whether rowset is preserved on commit
	BOOL				m_fCommitPreserve;
	//@cmember Flag indicating what DDL is supported in TXNS
	DWORD				m_fDDLBehavior;		

	//NOTE:  We use enums for these values so we can have a NOT SUPPORTED
	//value (in addition to valid BOOL values) which we check 
	//in individual test cases later on.	

	//@cmember Flag indicating whether async commit is supported
	EASYNC				m_eAsyncCommit;	
	//@cmember Flag indicating Async Abort support
	EASYNC				m_eAsyncAbort;

	//@cmember CTOR
	CTxnRowset(LPWSTR tcName);	
	//@cmember Initialization
	virtual BOOL Init();
	//@cmember Termination
	virtual BOOL Terminate();
	
	//@cmember Sets the session's autocommit isolation level
	HRESULT	SetAutoCommitIsoLevel(ULONG	IsoLevel);

	//@cmember Function to create the rowset and release any property error memory
	virtual HRESULT MakeRowset();
	//@cmember Function to create the rowset and release any property error memory
	virtual HRESULT MakeRowsetReadOnly();
	//@cmember Pure virtual function to set the properties to 
	//generate the correct rowset on CreateRowsetObject
	virtual BOOL SetTxnRowsetProperties() = 0;

	//@cmember Finds a row and optionally returns the hRow for that row
	BOOL FindRow(ULONG ulRowNum, HROW * hRow);
	//@cmember Commits current txn, fNewUnitOfWork sets whether 
	//a new transaction is started, or we revert to autocommit mode.
	//fPreserveRowset sets whether or not the rowset is kept after the commit.
	virtual HRESULT	Commit(BOOL fPreserveRowset = FALSE, BOOL fNewUnitOfWork = FALSE);
	//@cmember Aborts current txn, fNewUnitOfWork sets whether 
	//a new transaction is started, or we revert to autocommit mode.
	//fPreserveRowset sets whether or not the rowset is kept after the abort.
	virtual HRESULT Abort(BOOL fPreserveRowset = FALSE, BOOL fNewUnitOfWork = FALSE);	
	//@cmember Commits current txn, fNewUnitOfWork sets whether 
	//a new transaction is started, or we revert to autocommit mode.
	//fPreserveRowset sets whether or not the rowset is kept after the commit.
//#ifdef TXNJOIN
	virtual HRESULT	CommitCoord(BOOL fNewUnitOfWork = FALSE, ITransaction	*pITransaction = NULL,ITransactionJoin	*pITransactionJoin=NULL);
	//@cmember Aborts current txn, fNewUnitOfWork sets whether 
	//a new transaction is started, or we revert to autocommit mode.
	//fPreserveRowset sets whether or not the rowset is kept after the abort.
	virtual HRESULT AbortCoord(BOOL fNewUnitOfWork = FALSE, ITransaction	*pITransaction = NULL,ITransactionJoin	*pITransactionJoin=NULL);	
	//@cmember Dummy necessary virtual function needed to 
	//instantiate since we inherit from CTestCases
//#endif//TXNJOIN
	inline void *GetVarInfo(const long){return (void *)NULL; }
	//@cmember Dummy necessary virtual function needed to 
	//instantiate since we inherit from CTestCases
	inline const WCHAR *GetCaseDesc(void){return (const WCHAR *)NULL; }

};


//--------------------------------------------------------------------
// @class CTxnRORowset | Transactable Read Only rowset
// @base public | CTxnRowset
//--------------------------------------------------------------------
class CTxnRORowset : public CTxnRowset 
{
// @access public
public:	
	//@cmember CTOR
	CTxnRORowset(LPWSTR tcName) : CTxnRowset(tcName){};
	//@cmember Initialization - Creates a read only rowset
	virtual BOOL Init();
	//@cmember Cleans up read only rowset
	virtual BOOL Terminate();
	//@cmember Sets the properties to generate the correct rowset on CreateRowsetObject
	virtual BOOL SetTxnRowsetProperties();
	
};

//--------------------------------------------------------------------
// @class CTxnChgRowset | Transactable updateable rowset
// @base public | CTxnRowset
//--------------------------------------------------------------------
class CTxnChgRowset : public CTxnRowset
{

// @access public
public:
	//@cmember Whether or not to bind long data.  By default false
	BLOBTYPE			m_fBindLongData;
	//@cmember  Whether or not this provider supports IRowsetChange	
	BOOL				m_fChange;
	//@cmember  Whether or not this provider supports IRowsetUpdate	
	BOOL				m_fUpdate;
	//@cmember Count of updateable columns, to be used for NewRow and Change
	DBORDINAL			m_cUpdateableCols;
	//@cmember Array of updateable columns, to be used for NewRow and Change
	DBORDINAL			*m_rgUpdateableCols;	
	//@cmember Count of bindings for NewRow and Change
	DBORDINAL			m_cUpdateBindings;
	//@cmember Array of bindings for NewRow and Change
	DBBINDING			*m_rgUpdateBindings;
	//@cmember Accessor for NewRow and Change
	HACCESSOR			m_hUpdateAccessor;
	//@cmember Row size for NewRow and Change
	DBLENGTH			m_cbUpdateRowSize;		
	//@cmember Flag checked at commit time, to determine if an insert
	//has been committed.  
	BOOL				m_fTxnPendingInsert;
	//@cmember Flag checked at commit time, to determine if a delete
	//has been committed.  
	BOOL				m_fTxnPendingDelete;
	//@cmember Flag checked at commit time, to determine if a change
	//has been committed.  
	BOOL				m_fTxnPendingChange;
	//@cmember Whether or not a transaction is started on this DB Session
	BOOL				m_fTxnStarted;
	
	//@cmember CTOR
	CTxnChgRowset(LPWSTR tcName);
	//@cmember Initialization - Creates an updateable rowset
	virtual BOOL Init();
	//Copies pertainent info when two rowsets are sharing the same
	//session and txn, and the give rowset has made a change
	void CopyTxnInfo(CTxnChgRowset * pTxnChgRowset);
	//@cmember Sets the properties to generate the correct rowset on CreateRowsetObject
	virtual BOOL SetTxnRowsetProperties();
	//@cmember Termination - Cleans up all non base rowset interfaces
	virtual BOOL Terminate();
	//@cmember Commits current txn, fNewUnitOfWork sets whether 
	//a new transaction is started, or we revert to autocommit mode.
	//fPreserveRowset sets whether or not the rowset is kept after the commit.
	virtual HRESULT	Commit(BOOL fPreserveRowset = FALSE, BOOL fNewUnitOfWork = FALSE);
	//@cmember Starts a txn, setting the m_fTxnStarted flag
	virtual HRESULT	StartTxn(ISOLEVEL isoLevel);
//#ifdef TXNJOIN
	//@cmember Commits current txn, fNewUnitOfWork sets whether 
	//a new transaction is started, or we revert to autocommit mode.
	//fPreserveRowset sets whether or not the rowset is kept after the commit.
	virtual HRESULT	CommitCoord(BOOL fNewUnitOfWork = FALSE, ITransaction	*pITransaction = NULL,ITransactionJoin	*pITransactionJoin=NULL);
	//@cmember Gets a TransactionJoin interface pointer from a session object
	virtual HRESULT	GetTxnJoin();
	//@cmember Gets a TransactionJoin interface pointer from a session object
	virtual BOOL	StartCoordTxn(ITransaction	**ppITransaction,ISOLEVEL		isolevel);
	//@cmember Aborts the coordinated transacrtion and frees the pointer to TxnJoin
	virtual	BOOL	FreeJoinTxn();
	//@cmember Frees the ITrans interface and cloese connectin to dtc
	virtual	BOOL	FreeCoordTxn(ITransaction	*pITransaction);
//#endif//TXNJOIN
	//@cmember Aborts current txn, fNewUnitOfWork sets whether 
	//a new transaction is started, or we revert to autocommit mode.
	//fPreserveRowset sets whether or not the rowset is kept after the abort.
	virtual HRESULT Abort(BOOL fPreserveRowset = FALSE, BOOL fNewUnitOfWork = FALSE);		
//#ifdef TXNJOIN
	//@cmember Aborts current txn, fNewUnitOfWork sets whether 
	//a new transaction is started, or we revert to autocommit mode.
	//fPreserveRowset sets whether or not the rowset is kept after the abort.
	virtual HRESULT AbortCoord(BOOL fNewUnitOfWork = FALSE, ITransaction	*pITransaction = NULL,ITransactionJoin	*pITransactionJoin=NULL);		
//#endif//TXNJOIN
	//@cmember Inserts a row given the row number -- does not explicitly commit
	BOOL	Insert(	ULONG	ulRowNum, 
					HROW	*phNewRow	= NULL,
					BOOL	fDEFAULT	= FALSE);
	//@cmember Deletes a row given the row number -- does not explicitly commit
	BOOL	Delete(ULONG ulRowNum, HROW * phRow = NULL);
	//@cmember Deletes and Inserts (ie, Changes a row) given 
	//the row numbers -- does not explicitly commit
	BOOL	Change(	ULONG	ulDeleteRowNum, 
					ULONG	ulInsertRowNum, 
					HROW	*phRow			= NULL,
					BOOL	fIGNORE			= FALSE,
					BOOL	fDEFAULT		= FALSE);	
	
};

//--------------------------------------------------------------------
// @class CTxn | Base Class for Transaction testing.
// @base public | CDataSourceObject
//--------------------------------------------------------------------
class CTxn : public CSessionObject
{

// @access public
public:
	//@cmember	Row Number which is currently being inserted.  This row
	//may or may not be committed yet, so it may not be visible to other txns.	
	ULONG			m_ulCurrentInsertRow;
	//@cmember	Row Number which is currently being deleted.  This row
	//may or may not be committed yet, so it may not be visible to other txns.	
	ULONG			m_ulCurrentDeleteRow;
	//@cmember	New Row Number of row which is currently being changed.  This change
	//may or may not be committed yet, so it may not be visible to other txns.	
	ULONG			m_ulCurrentChangeRow;	
	//@cmember Whether or not Isolation Level Chaos is supported
	BOOL			m_fChaos;
	//@cmember Whether or not Isolation Level Read Uncommitted is supported
	BOOL			m_fReadUncommitted;
	//@cmember Whether or not Isolation Level Read Committed is supported
	BOOL			m_fReadCommitted;
	//@cmember Whether or not Isolation Level Repeatable Read is supported
	BOOL			m_fRepeatableRead;
	//@cmember Whether or not Isolation Level Serializable is supported
	BOOL			m_fSerializable;	
	//@cmember A supported isolation level to use for general purposes
	ISOLATIONLEVEL	m_fIsoLevel;
	//@cmember Whether or not the current datasource is Microsoft Access
	//this flag is used to decide if we need to wait a few seconds after an update
	//before other connections can see it.
	BOOL m_fOnAccess;
 
	//@cmember CTOR
	CTxn(LPWSTR tcName);	
	//@cmember Initialization
	virtual BOOL Init();
	//@cmember Termination
	virtual BOOL Terminate();
	//@cmember  Gets value to use for next insert
	inline ULONG	GetNextRowToInsert(){return g_ulLastActualInsert + 1;};	
	//@cmember  Gets value to use for next delete
	inline ULONG	GetNextRowToDelete(){return g_ulLastActualDelete + 1;};
	//@cmember  Copies normally initialized by test framework test case info 
	//into the encapsulated object which inherits from CTestCases.
	void	CopyTestCaseInfo(CTestCases * pTC);
	//@cmember  Inserts the next row into the table, on the given txn
	BOOL	Insert(CTxnChgRowset * pTxnChgRowset);
	//@cmember  Deletes the next row to delete in the table, on the given txn
	BOOL	Delete(CTxnChgRowset * pTxnChgRowset, HROW * phRow = NULL);
	//@cmember  Changes the next row to delete in the table to the values
	//based on the next row to insert, on the given txn
	BOOL	Change(CTxnChgRowset * pTxnChgRowset);		
	//@cmember Tries to find the current inserted row, which may or may not be commited
	inline BOOL FindInsert(CTxnRowset * pRowset)
		{ return pRowset->FindRow(m_ulCurrentInsertRow, NULL);};
	//@cmember Tries to find the current deleted row, which may or may not be commited
	inline BOOL FindDelete(CTxnRowset * pRowset)
		{ return pRowset->FindRow(m_ulCurrentDeleteRow, NULL);};
	//@cmember Tries to find the current Changed row, which may or may not be commited
	inline BOOL FindChange(CTxnRowset * pRowset)
		{ return pRowset->FindRow(m_ulCurrentChangeRow, NULL);};
	//@cmember Starts the txn with the IsoLevel	
	HRESULT StartTxn(ITransactionLocal * pITxnLocal, ISOLEVEL IsoLevel);
	//@cmember Releases all rowsets associated with any encapsulated objects
	virtual void ReleaseAllRowsetsAndTxns()=0;
	//@cmember Determines whether or not a new unit of work has begun on txn 1
	BOOL	NewUnitOfWork(CTxnChgRowset * pTxnChgRowset, CTxnRowset * pTxnRowset); 
//#ifdef TXNJOIN
	//@cmember Determines whether or not a new unit of work has begun on txn 1
	BOOL	NewUnitOfWorkCoord(CTxnChgRowset *pTxnChgRowset, CTxnRowset *pTxnRowset, ITransaction *pITransaction,ITransactionJoin	*pITransactionJoin); 
	//@cmember Determines whether or not txn 1 is in autocommit mode
	BOOL	NoNewUnitOfWorkCoord(CTxnChgRowset *pTxnChgRowset, CTxnRowset *pTxnRowset, ITransaction *pITransaction, BOOL fChange,ITransactionJoin	*pITransactionJoin); 	
//#endif//TXNJOIN
	//@cmember Determines whether or not the rowset is functional on txn 1
	BOOL	RowsetFunctional(CTxnRowset * pTxnRowset); 
	//@cmember Determines whether or not the rowset is zombied on txn 1
	BOOL	RowsetZombied(CTxnRowset * pTxnRowset, IRowset ** ppIRowset, HROW * phRow, HACCESSOR hAccessor); 
	//@cmember Determines whether or not txn 1 is in autocommit mode
	BOOL	NoNewUnitOfWork(CTxnChgRowset *pTxnChgRowset, CTxnRowset *pTxnRowset); 	
	//@cmember Verifies all transaction info from GetTransactionInfo
	BOOL	VerifyTxnInfo(ITransaction * pITxn, ISOLEVEL isoLevel, XACTUOW * puow = NULL);

};



//--------------------------------------------------------------------
// @class CUpdateTxn | Base Class for non buffered mode Transaction testing.
// @base public | CTxn
//--------------------------------------------------------------------
class CTxnImmed : public CTxn
{

// @access public
public:

	//@cmember CTOR
	CTxnImmed(LPWSTR wszTestCaseName);
	//@cmember	Pointer to Rowset object that supports Changes on TXN_1
	CTxnChgRowset *	m_pChgRowset1;
	//@cmember	Pointer to Rowset object that supports Changes on TXN_2
	CTxnChgRowset *	m_pChgRowset2;
	//@cmember	Pointer to Rowset object that supports Changes on TXN_3
	CTxnChgRowset *	m_pChgRowset3;
	//@cmember	Pointer to Read Only Rowset on TXN_1
	CTxnRORowset *	m_pRORowset1;

	//@cmember Initialization
	virtual BOOL Init();
	//@cmember Termination
	virtual BOOL Terminate();
	//@cmember Releases all rowsets associated with any encapsulated objects
	virtual void ReleaseAllRowsetsAndTxns();
	//@cmember Releases all rowsets associated with any encapsulated objects
	virtual void ReleaseAllRowsets();

	//@cmember Get DBPROP_SUPPORTEDTXNISOLEVELS
	HRESULT GetIsoLevels(ISOLEVEL	*pIsoLevel);
	
};


// This structure keeps the numeric value and description of each isolation level
struct tagIsolation {
	char			*pszDesc;
	ULONG			ulIsolation;
};

extern		tagIsolation		g_IsolationList[];
extern		ULONG				g_ulTotalIsolations;
