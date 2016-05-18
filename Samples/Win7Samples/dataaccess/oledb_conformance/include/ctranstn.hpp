//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1995-2000 Microsoft Corporation.  
//
// @doc 
//
// @module CTransaction Header Module | This module contains header information for CTransaction.
//
// @comm
// Special Notes...:	(OPTIONAL NOTES FOR SPECIAL CIRCUMSTANCES)
//
// <nl><nl>
// Revision History:<nl>
//	
//	[00] MM-DD-YY	EMAIL_NAME	ACTION PERFORMED... <nl>
//	[01] 10-05-95	Microsoft	Created <nl>
//	[02] 12-01-96	Microsoft	Updated for release <nl>
//	
// @head3 CTransaction Elements|
//
// @subindex CTransaction|
//
//---------------------------------------------------------------------------

#ifndef _CTransaction_HPP_
#define _CTransaction_HPP_

#include "modstandard.hpp"
#include "CTable.hpp"
#include "miscfunc.h"

//---------------------------------------------------------------------------
// Constants
//---------------------------------------------------------------------------
#define TRANSACTION_ROW_COUNT	6

//--------------------------------------------------------------------
//	@class CTransaction | Class for setting Zombie states.
//
// This class is the base class for the Zombie object classes.
//
// @base public | CTestCases
//
//--------------------------------------------------------------------
class CTransaction : public CTestCases
{
	private:

	BOOL	m_fInTestCase;

	// @access Protected
	protected:
	
	// @cmember IDBCreateSession pointer. <nl>
	IDBCreateSession *	m_pIDBCreateSession; 

	// @cmember ITransactionDispenser pointer. <nl>
	ITransactionLocal * m_pITransactionLocal;

	// @cmember IDBCreateCommand pointer. <nl>
	IDBCreateCommand *	m_pIDBCreateCommand;

	// @cmember IOpenRowset pointer. <nl>
	IOpenRowset*	m_pIOpenRowset;

	// @cmember ICommand pointer. <nl>
	ICommand *			m_pICommand;

	// @cmember ICommand pointer. <nl>
	ICommandPrepare *	m_pICommandPrepare;

	// @cmember IRowsetInfo pointer. <nl>
	IRowsetInfo *		m_pIRowsetInfo;

	// @cmember IRowset pointer. <nl>
	IRowset	*			m_pIRowset;

	// @cmember IAccessor pointer. <nl>
	IAccessor *			m_pIAccessor;

	// @cmember IColumnsInfo. <nl>
	IColumnsInfo *		m_pIColumnsInfo;

	// @cmember Table pointer. <nl>
	CTable *			m_pCTable;

	// @cmember Type of Object. <nl>
	EINTERFACE			m_eInterface;

	// @cmember Interface ID. <nl>
	IID					m_iid;

	// @cmember HResult. <nl>
	HRESULT				m_hr;

	// @cmember fAbortPreserve. <nl>
	BOOL				m_fAbortPreserve;

	// @cmember fCommitPreserve. <nl>
	BOOL				m_fCommitPreserve;

	// @cmember m_fPrepareAbortPreserve. <nl>
	BOOL				m_fPrepareAbortPreserve;

	// @cmember m_fPrepareCommitPreserve. <nl>
	BOOL				m_fPrepareCommitPreserve;

	// @cmember m_cRowsetCols. <nl>
	DBORDINAL			m_cRowsetCols;

	// @cmember m_rgTableColOrds. <nl>
	DB_LORDINAL	*		m_rgTableColOrds;

	// @cmember m_pwszTestCaseName. <nl>
	WCHAR	*			m_pwszTestCaseName;
	
	// @access Public
	public:

	// @cmember Constructor. <nl>
	CTransaction(WCHAR * 
		pwszTestCaseName // [IN] TestCase name
	);

	// @cmember Destructor. <nl>
	virtual ~CTransaction();

	// @cmember Initialization Routine. <nl>
	virtual BOOL Init(CTestCases * pTestCase = NULL, CTable * pCTable = NULL);

	// @cmember Termination Routine. <nl>
	virtual BOOL Terminate();

	// @cmember Register the interface to test. <nl>
	BOOL RegisterInterface(
		EINTERFACE eInterface,			//[in] the ole db object to be tested
		IID iid,						//[in] the interface to be tested
		ULONG cProperties=0,			//[in] the count DBProperties to be set. Default=0.
		DBPROPSET *pDBProperty=NULL		//[in] the array of DBProperties to be set. Defualt=NULL.
	);

	// @cmember Get A new Transaction from ITransactionLocal from DB Session object, <nl>
	// creata a DB Session object and a command object.  Set the properties requested on <nl>
	// the command object.  Execute the SQL Statement and retrieve the rowset.  The routine <nl>
	// also returns a pointer to the requested interface. <nl>
	BOOL StartTransaction(
		EQUERY				eSQLStmt,						//[in] the sql statement of the transaction
		IUnknown			**ppIUnknown=NULL,				//[out] the pointer to the interface being tested (default = NULL)
		ULONG				cProperties=0,					//[in] the count DBProperties to be set (default = 0)
		DBPROPSET			*pDBProperty=NULL,				//[in] the array of DBProperties to be set	(default = NULL)
		WCHAR				*pwszTableName=NULL,			//[in] the second table name (default = NULL)
		ISOLEVEL			isoLevel=ISOLATIONLEVEL_READUNCOMMITTED, //[in] the isolation level of the transation(default = ReadUncommitted)
		BOOL				fPrepare=FALSE					//[in] Prepare the Command (default = FALSE)
	);

	// @cmember CleanUp the transaction. <nl>
	void CleanUpTransaction(HRESULT	hr);	//[in]  hr is the hresult expected from pITransactionLocal->Abort.
											//		If there is no outstanding transactions(fRtaining==FALSE), 
											//      hr should be XACT_E_NOTRANSACTION.  Otherwise, 
											//		S_OK should be returned.

	// @cmember Abort transaction. <nl>
	BOOL GetAbort(BOOL	fRetaining);

	// @cmember Commit transation. <nl>
	BOOL GetCommit(BOOL	fRetaining);

BOOL	CTransaction::CompareHandlesByLiteral
(
			HROW	hThisRow,
			HROW	hThatRow,
			BOOL	fNewlyInserted,
			BOOL	fExpected,
			BOOL	fLITERALIDENITIY,
			BOOL	fSTRONGIDENTITY
);

};
#endif //_CTransaction_HPP_
