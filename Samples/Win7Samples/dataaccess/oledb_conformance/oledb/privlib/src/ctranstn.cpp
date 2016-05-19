//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1995-2000 Microsoft Corporation.  
//
// @doc 
//
// @module CTransaction Implementation Module | Source file for CTranstion Class
//
// @comm
// Special Notes...:	(OPTIONAL NOTES FOR SPECIAL CIRCUMSTANCES)
//
// <nl><nl>
// Revision History:<nl>
//	
//	[00] MM-DD-YY	EMAIL_NAME	ACTION PERFORMED... <nl>
//	[01] 06-30-95	Microsoft	Created <nl>
//	[02] 09-01-95	Microsoft	Code review update <nl>
//	[03] 10-01-95	Microsoft	Change to WCHAR * <nl>
//	[04] 12-01-96	Microsoft	Updated for release <nl>
//
// @head3 CTransaction Elements|
//
// @subindex CTransaction|
//
//---------------------------------------------------------------------------

#include "privstd.h"		//Private library common precompiled header
#include "ctranstn.hpp"
#include "coledb.hpp"

//---------------------------------------------------------------------------
// @mfunc CTransaction
//
// @parm Test case name
//
//---------------------------------------------------------------------------
CTransaction::CTransaction(WCHAR * pwszTestCaseName) : CTestCases(pwszTestCaseName)
{
	// Initialize member data
	m_pIDBCreateSession		= NULL;
	m_pITransactionLocal	= NULL;

	m_pIDBCreateCommand		= NULL;
	m_pIOpenRowset			= NULL;

	m_pICommand				= NULL;
	m_pICommandPrepare		= NULL;

	m_pIRowsetInfo			= NULL;
	m_pIRowset				= NULL;
	m_pIAccessor			= NULL;
	m_pIColumnsInfo			= NULL;

	m_pCTable				= NULL;
	m_eInterface			= DATASOURCE_INTERFACE;
	m_iid					= IID_IUnknown;
	m_hr					= S_OK;
	m_fAbortPreserve		= FALSE;
	m_fCommitPreserve		= FALSE;
	m_fPrepareAbortPreserve	= FALSE;
	m_fPrepareCommitPreserve= FALSE;
	m_cRowsetCols			= 0;
	m_rgTableColOrds		= NULL;
	m_pwszTestCaseName		= pwszTestCaseName;
	m_fInTestCase			= FALSE;
}

//---------------------------------------------------------------------------
// @mfunc ~CTransaction
//
//---------------------------------------------------------------------------
CTransaction::~CTransaction()
{
	if (m_fInTestCase)
		Terminate();
}

//---------------------------------------------------------------------------
// The routine creates a Data Source object with ITransactionDispenser interface,
// initialize it and create a table.
//
// @mfunc Init
//
// @rdesc Success or Failure
//
//---------------------------------------------------------------------------
BOOL CTransaction::Init(CTestCases * pTestCase, CTable * pCTable)
{
	BOOL				fInitSuccess		= FALSE;
	IDBInitialize *		pIDBInitialize		= NULL;
	IDBProperties *		pIDBProperties		= NULL;
	ULONG 				cPropertySets		= 0;
	DBPROPSET *			rgPropertySets		= NULL;

	if (!pTestCase)
	{
		if (!CTestCases::Init())
			return FALSE;
   
		//Skip Transactions if using CONF_STRICT
		if(!IsUsableInterface(SESSION_INTERFACE, IID_ITransactionLocal))
			return FALSE;
			
		// Get IDBInitialize Pointer
		if(!CHECK(GetModInfo()->CreateProvider(NULL, IID_IDBInitialize, (IUnknown**)&pIDBInitialize), S_OK))
			return FALSE;

		// Get the Initialize parameters from LTM for this provider
		if(!GetInitProps(&cPropertySets, &rgPropertySets))
			goto END;

		// Get IDBProperties Pointer
		if(!VerifyInterface(pIDBInitialize, IID_IDBProperties, DATASOURCE_INTERFACE, (IUnknown**)&pIDBProperties))
			goto END;

		// Set the properties before we Initialize
		if (!CHECK(pIDBProperties->SetProperties(cPropertySets,rgPropertySets), S_OK))
			goto END;

		// Initialize and Check to see if the Initialize FAILED
		if(!CHECK(m_hr = pIDBInitialize->Initialize(),S_OK))
			goto END;

		// Get IDBCreateSession Pointer
		if(!VerifyInterface(pIDBInitialize, IID_IDBCreateSession, DATASOURCE_INTERFACE, (IUnknown**)&m_pIDBCreateSession))
			goto END;

		// Create a DB Session object, asking for ITransactionLocal pointer
		if(FAILED(m_hr=m_pIDBCreateSession->CreateSession(NULL, IID_ITransactionLocal, (IUnknown**)&m_pITransactionLocal)))
		{
			// m_pITransactionDispenser should be NULL
			assert(!m_pITransactionLocal);
			CHECK(m_hr, E_NOINTERFACE);
			odtLog<<wszTransactionNotSupported;
			goto END;
		}

		//IOpenRowset
		if(!VerifyInterface(m_pITransactionLocal, IID_IOpenRowset, SESSION_INTERFACE, (IUnknown**)&m_pIOpenRowset))
			 goto END;

		//IDBCreateCommand (optional)
		if(VerifyInterface(m_pITransactionLocal, IID_IDBCreateCommand, SESSION_INTERFACE, (IUnknown**)&m_pIDBCreateCommand))
		{
			//Create a ICommandProperties Object
			if(!CHECK(m_pIDBCreateCommand->CreateCommand(NULL, IID_ICommand, (IUnknown **)&m_pICommand), S_OK))
				goto END;
		}

		// Create a Table
		if (!(m_pCTable = new CTable(m_pIOpenRowset, m_pwszTestCaseName, USENULLS)))
			goto END;

		// Insert 30 rows into the table
		if (!CHECK(m_pCTable->CreateTable(TRANSACTION_ROW_COUNT,1,NULL,PRIMARY,TRUE),S_OK))
			 goto END;
	}
	else
	{
		m_fInTestCase = TRUE;

		//Skip Transactions if using CONF_STRICT
		if(!IsUsableInterface(SESSION_INTERFACE, IID_ITransactionLocal))
			return FALSE;

		// Get IDBCreateSession Pointer
		if (!m_pIDBCreateSession)
			m_pIDBCreateSession = (IDBCreateSession *)pTestCase->m_pThisTestModule->m_pIUnknown;

		if (!COMPARE(m_pIDBCreateSession != NULL, TRUE))
			goto END;

		//ITransactionLocal
		if (!m_pITransactionLocal && !VerifyInterface(pTestCase->m_pThisTestModule->m_pIUnknown2,
			IID_ITransactionLocal, SESSION_INTERFACE, (IUnknown**)&m_pITransactionLocal))
		{
			// m_pITransaction should be NULL
			assert(!m_pITransactionLocal);
			CHECK(m_hr, E_NOINTERFACE);
			odtLog<<wszTransactionNotSupported;
			goto END;
		}

		//IOpenRowset
		if(!m_pIOpenRowset && !VerifyInterface(m_pITransactionLocal, IID_IOpenRowset, SESSION_INTERFACE, (IUnknown**)&m_pIOpenRowset))
			 goto END;

		//IDBCreateCommand (optional)
		if(!m_pIDBCreateCommand && VerifyInterface(m_pITransactionLocal, IID_IDBCreateCommand, SESSION_INTERFACE, (IUnknown**)&m_pIDBCreateCommand))
		{
			//Create a ICommandProperties Object
			if(!m_pICommand && !CHECK(m_pIDBCreateCommand->CreateCommand(NULL, IID_ICommand, (IUnknown **)&m_pICommand), S_OK))
				goto END;
		}

		//Get the table
		if (!COMPARE(pCTable != NULL, TRUE))
			goto END;

		m_pCTable = pCTable;
			
	}

	fInitSuccess = TRUE;

END:

	SAFE_RELEASE(pIDBInitialize);
	SAFE_RELEASE(pIDBProperties);
	FreeProperties(&cPropertySets,&rgPropertySets);
	return fInitSuccess;
}

//---------------------------------------------------------------------------
//	The routine drop the table and the Data Source object created in
//	Init routine.  
//			
// @mfunc Terminate 
//
// @rdesc Success or Failure
//
//---------------------------------------------------------------------------
BOOL CTransaction::Terminate()
{
	if (!m_fInTestCase)
	{
		// Drop the table
		if (m_pCTable)
			m_pCTable->DropTable();
		SAFE_DELETE(m_pCTable);
	}

	// Release the Command
	SAFE_RELEASE(m_pICommand);

	// Release the DB Session
	SAFE_RELEASE(m_pIOpenRowset);
	SAFE_RELEASE(m_pIDBCreateCommand);
	SAFE_RELEASE(m_pITransactionLocal);


	if (!m_fInTestCase)
	{
		SAFE_RELEASE(m_pIDBCreateSession);

		return (CTestCases::Terminate());
	}

	return TRUE;
}

//---------------------------------------------------------------------------
// Abort the transation with retaining.  
//
// For rowset objects, it DBPROP_ABORTPRESERVE is TRUE, the object will 
// be fully functional.  If DBPROP_ABORTPRESERVE is FALSE, the object
// will be in Zombie state.	  The value of DBPROP_ABORTPRESERVE is saved 
// in m_fAbortPreserve.
//
// For Command objects and DB Session objects, they will abort fully 
// functional.
//
// If fRetaining==TRUE, the transatcion retainds the manual commit mode.
// User has to call GetAbort(FALSE) or GetCommit(FALSE) to complete the transaction.
//
// @mfunc GetAbort
//
// @rdesc Success or Failure
//	@flag TRUE  | The transation was aborted non-retaining successfully. 
//	@flag FALSE | The transation failed to abort. 
//
//---------------------------------------------------------------------------
BOOL CTransaction::GetAbort(BOOL fRetaining)
{
	// Return FALSE if m_iid==IID_IUnknown.  Either the user has not called RegisterInterface
	// or the interface is not supported by the provider
	if (m_iid==IID_IUnknown)
		return FALSE;

	if (!m_pITransactionLocal)
		return FALSE;

	// Commit the transaction 
	return CHECK(m_pITransactionLocal->Abort(NULL,fRetaining,FALSE), S_OK);
}

//---------------------------------------------------------------------------
// Commit the transation with retaining.  
//
// For rowset objects, it DBPROP_COMMITPRESERVE is TRUE, the object will 
// be fully functional.  If DBPROP_COMMITPRESERVE is FALSE, the object
// will be in Zombie state.  The value of DBPROP_COMMITPRESERVE is saved 
// in m_fCommitPreserve.

//
// For Command objects and DB Session objects, they will commit fully 
// functional.
//
// If fRetaining==TRUE, the transatcion retainds the manual commit mode.
// User has to call GetAbort(FALSE) or GetCommit(FALSE) to complete the transaction.
//
// @mfunc GetCommitRetaining
//
// @rdesc Success or Failure
//	@flag TRUE  | The transation was commited non-retaining successfully. 
//	@flag FALSE | The transation failed to commite. 
//
//---------------------------------------------------------------------------
BOOL CTransaction::GetCommit(BOOL fRetaining)
{
	// Return FALSE if m_iid==IID_IUnknown.  Either the user has not called RegisterInterface
	// or the interface is not supported by the provider
	if (m_iid==IID_IUnknown)
		return FALSE;

	// m_pITransaction should be a valid pointer
	if (!m_pITransactionLocal)
		return FALSE;

	// Commit the transaction 
	return (CHECK(m_pITransactionLocal->Commit(fRetaining,0,0), S_OK));
}

//---------------------------------------------------------------------------
// Register the interface to be tested.  This function registers the 
// interface to be tested.  It has to be called right after Init() and
// before any other methods are called
//
// For testing Data Source, DB Session, and Command objects, no properties
// need be specified.  For testing rowset objects, the array of properties
// to be set for the rowset, the IID_Interface in particular, has to be
// specified unless the interface is mandatory.
// 
// @mfunc RegisterInterface
//
// @rdesc Success or Failure
//	@flag TRUE  | The Interface is supported by the provider. 
//	@flag FALSE | he Interface is not supported by the provider.
//
//---------------------------------------------------------------------------
BOOL CTransaction::RegisterInterface(
EINTERFACE	eInterface,		//@parm [in] the ole db object to be tested
IID iid,						//@parm [in] the interface to be tested
ULONG cPropSets,				//@parm[in] the count of DBPROPSET structures to be set
DBPROPSET * rgPropSets			//@parm[in] the array of DBPROPSET structures to be set
)
{
	BOOL fSupported = TRUE;

	// Copy the information about the OLE DB object and interface
	m_eInterface  = eInterface;
	m_iid		  = iid;

	// The interface is not supported if we can not created a transaction that return
	// a pointer to the requested interface
	if (!StartTransaction(SELECT_ALLFROMTBL,NULL,cPropSets,rgPropSets))
	{
		// Reset m_iid to IID_IUnknown as a flag
		m_iid=IID_IUnknown;
		odtLog<<wszInterfaceNotSupported;
		fSupported=FALSE;
	}

	CleanUpTransaction(S_OK);

	return fSupported;
}

//---------------------------------------------------------------------------
//	Get A new Transaction from ITransactionLocal on a DB Session object, 
//	Creata a command object and set the properties requested on	it.
//	Execute the SQL Statement and retrieve the rowset.  The routine
//	also returns a pointer to the registered interface.  If testing any optional interface
//  on a rowset object, the IID_Interface has to be set as a property for the rowset
//	object.
//
// @mfunc StartTransaction
//
// @rdesc Success or Failure
//	@flag TRUE  | A new transtion is started. 
//	@flag FALSE | Can not start a new tansaction.
//
//---------------------------------------------------------------------------
BOOL CTransaction::StartTransaction(
	EQUERY		eSQLStmt,			//@parm[in] the sql statement of the transaction
	IUnknown**	ppIUnknown,			//@parm[out] the pointer to the interface being tested
	ULONG		cDBPropSets,		//@parm[in] the count of DBPROPSET structures to be set
	DBPROPSET*	pDBPropSets,		//@parm[in] the array of DBPROPSET structures to be set
	WCHAR*		pwszTableName,		//@parm[in] the second table name
	ISOLEVEL	isoLevel,			//@parm[in] the isolation level of the transation
	BOOL		fPrepare			//@parm[in] Prepare the Command (default = FALSE)
	)
{
	IID				riid				= IID_IRowsetInfo;

	IUnknown*		pIUnknown			= NULL;
	IUnknown*		pIRowsetUnknown		= NULL;
	ULONG			ulTransactionLevel	= 0;
	ULONG_PTR		bValue				= 0;
	IRow*			pIRow				= NULL;

	// Return FALSE if m_iid==IID_IUnknown.  Either the user has not called RegisterInterface
	// or the interface is not supported by the provider
	if (m_iid==IID_IUnknown)
		return FALSE;

	// If asking for a rowset interface, just pass the riid, so we don't always have
	// to set the appropiate properties.
	if (m_eInterface == ROWSET_INTERFACE)
		riid = m_iid;
	
	// If m_pITransactionLocal is NULL, init() did not succeed.
	if (!m_pITransactionLocal)
		return FALSE;

	// Check for the Default IsoLevel
	if (isoLevel == ISOLATIONLEVEL_READUNCOMMITTED)
	{
		// Ask for DBPROP_SUPPORTEDTXNISOLEVELS
		if(GetProperty(DBPROP_SUPPORTEDTXNISOLEVELS, DBPROPSET_DATASOURCEINFO, m_pIDBCreateSession, &bValue))
		{
			// Check the Bitmask and set the IsoLevel
			if (bValue & DBPROPVAL_TI_SERIALIZABLE)
				isoLevel=ISOLATIONLEVEL_SERIALIZABLE;
			else if (bValue & DBPROPVAL_TI_REPEATABLEREAD)
				isoLevel=ISOLATIONLEVEL_REPEATABLEREAD;
			else if (bValue & DBPROPVAL_TI_READCOMMITTED)
				isoLevel=ISOLATIONLEVEL_READCOMMITTED;
			else
				isoLevel=ISOLATIONLEVEL_READUNCOMMITTED;

		}
	}

	// Create a new transaction
	if(!CHECK(m_hr = m_pITransactionLocal->StartTransaction(isoLevel,0,NULL,&ulTransactionLevel),S_OK))
		goto CLEANUP;

	// Prepare the Command
	if(fPrepare && m_pIDBCreateCommand && (eSQLStmt!=USE_OPENROWSET))
	{
		// Set the SQL Statement and get a ICommand object
		if (!CHECK(m_hr = m_pCTable->ExecuteCommand(eSQLStmt,riid,pwszTableName,NULL,
			&m_cRowsetCols,&m_rgTableColOrds,EXECUTE_NEVER,cDBPropSets,
			pDBPropSets, NULL, &pIRowsetUnknown,&m_pICommand),S_OK))
			goto CLEANUP;

		// Clean up the count and Ordinals
		m_cRowsetCols = 0;
		PROVIDER_FREE(m_rgTableColOrds);

		// QI for ICommandPrepare & Prepare the SQL Statement
		if(VerifyInterface(m_pICommand, IID_ICommandPrepare, COMMAND_INTERFACE, (IUnknown**)&m_pICommandPrepare))
			if(!CHECK(m_hr = m_pICommandPrepare->Prepare(1), S_OK))
				goto CLEANUP;
	}

	//Execute the SQL Statement
	if(m_pIDBCreateCommand && (eSQLStmt!=USE_OPENROWSET))
	{
		TESTC_(m_hr = m_pCTable->ExecuteCommand(eSQLStmt,riid,pwszTableName,NULL,
			&m_cRowsetCols,&m_rgTableColOrds , EXECUTE_IFNOERROR,cDBPropSets,
			pDBPropSets, NULL, &pIRowsetUnknown,&m_pICommand),S_OK);
	}
	else
	{
		TESTC_(m_hr = m_pCTable->CreateRowset(eSQLStmt, riid, cDBPropSets,pDBPropSets,
			&pIRowsetUnknown, NULL, &m_cRowsetCols, &m_rgTableColOrds),S_OK);
	}

	// Check to see if DBPROP_ABORTPRESERVE is VARIANT_TRUE
	if(GetProperty(DBPROP_ABORTPRESERVE, DBPROPSET_ROWSET, pIRowsetUnknown))
		m_fAbortPreserve = TRUE;
	else
		m_fAbortPreserve = FALSE;

	// Check to see if DBPROP_COMMITPRESERVE is VARIANT_TRUE
	if(GetProperty(DBPROP_COMMITPRESERVE, DBPROPSET_ROWSET, pIRowsetUnknown))
		m_fCommitPreserve = TRUE;
	else
		m_fCommitPreserve = FALSE;

	// Get value for DBPROP_PREPAREABORTBEHAVIOR
	if((GetProperty(DBPROP_PREPAREABORTBEHAVIOR, DBPROPSET_DATASOURCEINFO, 
			m_pIDBCreateSession, &bValue)) && (bValue == DBPROPVAL_CB_PRESERVE))
		m_fPrepareAbortPreserve = TRUE;

	// Get value for DBPROP_PREPARECOMMITBEHAVIOR
	if((GetProperty(DBPROP_PREPARECOMMITBEHAVIOR, DBPROPSET_DATASOURCEINFO, 
			m_pIDBCreateSession, &bValue)) && (bValue == DBPROPVAL_CB_PRESERVE))
		m_fPrepareCommitPreserve = TRUE;

	//Obtain mandaory interface pointers
	if(!VerifyInterface(pIRowsetUnknown, IID_IRowset, ROWSET_INTERFACE, (IUnknown**)&m_pIRowset))
		goto CLEANUP;
	if(!VerifyInterface(pIRowsetUnknown, IID_IAccessor, ROWSET_INTERFACE, (IUnknown**)&m_pIAccessor))
		goto CLEANUP;
	if(!VerifyInterface(pIRowsetUnknown, IID_IColumnsInfo, ROWSET_INTERFACE, (IUnknown**)&m_pIColumnsInfo))
		goto CLEANUP;
	if(!VerifyInterface(pIRowsetUnknown, IID_IRowsetInfo, ROWSET_INTERFACE, (IUnknown**)&m_pIRowsetInfo))
		goto CLEANUP;
	
	//Return the requested interface pointer.  
	switch(m_eInterface)
	{
		case DATASOURCE_INTERFACE:
			if(!VerifyInterface(m_pIDBCreateSession, m_iid, DATASOURCE_INTERFACE, (IUnknown**)&pIUnknown))
				m_hr = E_NOINTERFACE;
			break;

		case SESSION_INTERFACE:
			if(!VerifyInterface(m_pIOpenRowset, m_iid, SESSION_INTERFACE, (IUnknown**)&pIUnknown))
				m_hr = E_NOINTERFACE;
			break;

		case COMMAND_INTERFACE:
			if(!VerifyInterface(m_pICommand, m_iid, COMMAND_INTERFACE, (IUnknown**)&pIUnknown))
				m_hr = E_NOINTERFACE;
			break;

		case ROWSET_INTERFACE:
			if(!VerifyInterface(m_pIRowsetInfo, m_iid, ROWSET_INTERFACE, (IUnknown**)&pIUnknown))
				m_hr = E_NOINTERFACE;
			break;

		case ROW_INTERFACE:
		{
			//Get a row
			DBCOUNTITEM	cRowsObtained = 0;
			HROW		hrow = DB_NULL_HROW;
			HROW*		prghRows = &hrow;
			CRowObject	RowObj;

			TESTC_(m_hr = m_pIRowset->GetNextRows(NULL, 0, 1, &cRowsObtained, &prghRows),S_OK);
				
			TESTC(cRowsObtained==1);

			// Row object support is optional
			m_hr = RowObj.CreateRowObject(m_pIRowset, *prghRows);
			COMPARE(m_hr == S_OK || m_hr == E_NOINTERFACE, TRUE);
			if(FAILED(m_hr))
			{
				m_hr = m_pIOpenRowset->OpenRowset(NULL, &(m_pCTable->GetTableID()), 
					NULL, IID_IRow, 0, NULL, (IUnknown**)&pIRow);
				if (!VerifyInterface(pIRow, m_iid, ROW_INTERFACE, (IUnknown**)&pIUnknown))
					m_hr = E_NOINTERFACE;
			}
			else
			{
				if (!VerifyInterface(RowObj.pIRow(), m_iid, ROW_INTERFACE, (IUnknown**)&pIUnknown))
					m_hr = E_NOINTERFACE;	
			}
		}

		default:
			break;
	}

CLEANUP:
	
	SAFE_RELEASE(pIRow);
	SAFE_RELEASE(pIRowsetUnknown);
	
	// Does the user want the pointer
	if (ppIUnknown)
		*ppIUnknown=pIUnknown;
	else if (pIUnknown)
		SAFE_RELEASE(pIUnknown);

	// If no errors return TRUE
	return SUCCEEDED(m_hr);
}

//---------------------------------------------------------------------------
// @mfunc CleanUpTransaction.  It should be called in pair with StartTransaction().
// The function cleans up memory allocated during StartTransaction and 
// abort the transaction.
// 
// @rdesc Success or Failure
//	hr is the hresult expected from pITransactionLocal->Abort.
//	If there is no outstanding transactions(fRtaining==FALSE), hr should be 
//	XACT_E_NOTRANSACTION.  Otherwise, S_OK should be returned.
//
//	@flag TRUE | CleanUp was successful.
//
//---------------------------------------------------------------------------
void CTransaction::CleanUpTransaction(HRESULT hrExpected)
{
	HRESULT	 hr = S_OK;

	// Release interface pointers on the rowset
	SAFE_RELEASE(m_pIRowset);
	SAFE_RELEASE(m_pIAccessor);
	SAFE_RELEASE(m_pIColumnsInfo);
	SAFE_RELEASE(m_pIRowsetInfo);

	// Release memory used by rgTableColOrds
	m_cRowsetCols = 0;
	PROVIDER_FREE(m_rgTableColOrds);

	// Release the command object
	SAFE_RELEASE(m_pICommandPrepare);
	SAFE_RELEASE(m_pICommand);

	// Abort the transaction. We do not care if the method succeeds or not.
	// In case CleanUpTransaction() is called without Commit/Abort after
	// StartTransaction().
	if (m_pITransactionLocal)
		hr = m_pITransactionLocal->Abort(NULL, FALSE,NULL);
	
	// In case this func is called after startTransaction is failed.
	if (hr !=  XACT_E_NOTRANSACTION)
		CHECK(hr, hrExpected);
}

BOOL	CTransaction::CompareHandlesByLiteral
(
			HROW	hThisRow,
			HROW	hThatRow,
			BOOL	fNewlyInserted,
			BOOL	fExpected,
			BOOL	fLITERALIDENITIY,
			BOOL	fSTRONGIDENTITY
)
{
	BOOL	fResult;

	//if DBPROP_LITERLIDENTITY is not supported or is VARIANT_FALSE.  
	//Return TEST_SKIPPED
	if	(!fLITERALIDENITIY)
	{
		return TRUE;
	}

	//if DBPROP_STRONGIDENITY is not supported or is VARIANT_FALSE AND the rows are newly
	//inserted, return TEST_SKIPPED
	if	((!fSTRONGIDENTITY) && fNewlyInserted)
	{
		return TRUE;
	}

	//otherwise, compare the row handles bit by bit
	if(memcmp(&hThisRow, &hThatRow, sizeof(HROW)))
	{
		fResult = FALSE;
	}
	else
	{
		fResult = TRUE;
	}
		
	if(fResult==fExpected)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}