//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1996-2000 Microsoft Corporation.
//
// @doc 
//
// @module TXNBASE.CPP | Base classes for transacted rowsets.
//
#include "modstandard.hpp"
#include "txnbase.hpp"
//#ifdef TXNJOIN
	#include "XOLEHLP.H"
	#include "txdtc.h"
//#endif  //TXNJOIN
#include "extralib.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Global Declarations
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

ULONG	g_ulLastActualDelete	= 0;
ULONG	g_ulLastActualInsert	= 0;
ULONG	g_ulFirstRowInTable		= 0;
ULONG	g_ulNumInFirstRow		= 0;

LPWSTR	gwszModName = L"itxnbase";	

DBPrptRecord g_rgDBPrpt[g_PROPERTY_COUNT];

tagIsolation	g_IsolationList[] = {
	{ "CHAOS",				DBPROPVAL_TI_CHAOS				},
	{ "READUNCOMMITTED",	DBPROPVAL_TI_READUNCOMMITTED	},
	{ "BROWSE",				DBPROPVAL_TI_BROWSE				},
	{ "CURSORSTABILITY",	DBPROPVAL_TI_CURSORSTABILITY	},
	{ "READCOMMITTED",		DBPROPVAL_TI_READCOMMITTED		},
	{ "REPEATABLEREAD",		DBPROPVAL_TI_REPEATABLEREAD		}, 
	{ "SERIALIZABLE",		DBPROPVAL_TI_SERIALIZABLE		}, 
	{ "ISOLATED",			DBPROPVAL_TI_ISOLATED			}
};
ULONG	g_ulTotalIsolations		=	8;


//Note, any test module which uses these classes must have defined this
//global variable so we know what to use for the test module name
extern LPWSTR	gwszThisModuleName;		

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Global Functions
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void g_DeleteIncrement()
{
	g_ulLastActualDelete++;
	
	//If we're deleting our first row (normally we are)
	//then increment that count as well 
	if (g_ulLastActualDelete == g_ulFirstRowInTable)
		g_ulFirstRowInTable++;
}

void g_DeleteAndInsertIncrement()
{
	//Record that we just did a new delete and insert,
	//ie, an update
	g_ulLastActualDelete++;
	g_ulLastActualInsert++;

	//If we are changing our first row, we need to
	//record what the new value is
	if (g_ulLastActualDelete == g_ulFirstRowInTable)
		g_ulFirstRowInTable = g_ulLastActualInsert; //++
}

void g_InsertIncrement()
{
	//Don't have to do anything special with
	//g_ulFirstRowInTable since it isn't
	//touched by an insert
	g_ulLastActualInsert++;	
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Base Class Function Definitions
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


//--------------------------------------------------------------------
// @mfunc CTOR
//--------------------------------------------------------------------
CTxnRowset::CTxnRowset(LPWSTR tcName) : CRowsetObject(tcName)
{
	m_pITxnLocal		= NULL;	
//#ifdef TXNJOIN
	m_pITxnJoin			= NULL;	
//#endif//TXNJOIN
	m_pCTable			= NULL;
	m_cReadBindings		= 0;
	m_rgReadBindings	= NULL;
	m_hReadAccessor		= DB_NULL_HACCESSOR;
	m_cbReadRowSize		= 0;

	m_fAbortPreserve	= FALSE;
	m_fCommitPreserve	= FALSE;	
	m_eAsyncCommit		= EASYNCFALSE;
	m_eAsyncAbort		= EASYNCFALSE;
	m_fDDLBehavior		= 0;	
}

//--------------------------------------------------------------------
// Inits the encapsulated CTxnRowset Object				
// @mfunc Init
//--------------------------------------------------------------------
BOOL	CTxnRowset::Init()
{
	IUnknown		*pIUnk						= NULL;
	IID				*pIID						= NULL;
	BOOL			fResults					= FALSE;
	IRowsetInfo		*pIInfo						= NULL;
	const ULONG		cDSOPropIDs					= 3;
	DBPROPID		rgDSOPropIDs[cDSOPropIDs];
	DBPROPIDSET		PropIDSet;
	ULONG			cPropSets					= 0;
	DBPROPSET		*rgSupportedPropSets		= NULL;
	DBPROPSET		RowsetPropSet;
	IDBInitialize	*pIDBInit					= NULL;
	IDBProperties	*pIDBProp					= NULL;
	ULONG			i;

	HRESULT			hr;

	DBPROPIDSET		rgRowProps;
	ULONG			cPropertyInfoSets			= g_PROPERTY_COUNT;
	DBPROPINFOSET	*prgPropertyInfoSets;
	OLECHAR			*ppDescBuffer				= NULL;

	rgRowProps.rgPropertyIDs	=	NULL;
	rgRowProps.cPropertyIDs		=	g_PROPERTY_COUNT;
	rgRowProps.guidPropertySet	=	DBPROPSET_ROWSET;

	rgRowProps.rgPropertyIDs = (DBPROPID *)PROVIDER_ALLOC(sizeof(DBPROPID) * g_PROPERTY_COUNT);
	if (!rgRowProps.rgPropertyIDs)
	{
		return FALSE;
	}
	memset(rgRowProps.rgPropertyIDs, 0, sizeof(DBPROPID) * g_PROPERTY_COUNT);

	//Rowset properties
	rgRowProps.rgPropertyIDs[IDX_CommitPreserve]	= DBPROP_COMMITPRESERVE;
	rgRowProps.rgPropertyIDs[IDX_AbortPreserve]		= DBPROP_ABORTPRESERVE;
	rgRowProps.rgPropertyIDs[IDX_OtherUpdateDelete]	= DBPROP_OTHERUPDATEDELETE;
	rgRowProps.rgPropertyIDs[IDX_OtherInsert]		= DBPROP_OTHERINSERT;
	rgRowProps.rgPropertyIDs[IDX_CommandTimeout]	= DBPROP_COMMANDTIMEOUT;
	rgRowProps.rgPropertyIDs[IDX_IRowsetUpdate]		= DBPROP_IRowsetUpdate;
	rgRowProps.rgPropertyIDs[IDX_IRowsetChange]		= DBPROP_IRowsetChange;
	//mark everything as supported
	for(i=0;i<g_PROPERTY_COUNT;i++)
	{
		g_rgDBPrpt[i].dwPropID				= rgRowProps.rgPropertyIDs[i];
		g_rgDBPrpt[i].fSupported			= TRUE;
		g_rgDBPrpt[i].fDefault				= FALSE;
		g_rgDBPrpt[i].fProviderSupported	= TRUE;
	}
	
	//DSO Properties
	rgDSOPropIDs[0] = DBPROP_SUPPORTEDTXNDDL;
	rgDSOPropIDs[1] = DBPROP_ASYNCTXNABORT;
	rgDSOPropIDs[2] = DBPROP_ASYNCTXNCOMMIT;			
	
	//Get a IDBInitialize ptr
	if (VerifyInterface(	m_pThisTestModule->m_pIUnknown, 
							IID_IDBInitialize,
							DATASOURCE_INTERFACE, 
							(IUnknown **)&pIDBInit))
	{
		//This rowset will share the DSO of the testcase
		SetDataSourceObject(pIDBInit, TRUE);
		
		//Get Properties on DSO
		if (COMPARE(VerifyInterface(pIDBInit, IID_IDBProperties, DATASOURCE_INTERFACE,
			(IUnknown**)&pIDBProp), TRUE))
		{
			PropIDSet.cPropertyIDs		= cDSOPropIDs;
			PropIDSet.rgPropertyIDs		= rgDSOPropIDs;
			PropIDSet.guidPropertySet	= DBPROPSET_DATASOURCEINFO;

			m_hr = pIDBProp->GetProperties(1, &rgRowProps, &cPropSets, 
								&rgSupportedPropSets);
			
			//If output array should be valid, check it for status
			if (SUCCEEDED(m_hr) || m_hr == DB_E_ERRORSOCCURRED)
			{
				//Find TXN support behavior 
				if (rgSupportedPropSets[0].rgProperties[0].dwStatus != DBPROPSTATUS_NOTSUPPORTED)
				{
					m_fDDLBehavior = rgSupportedPropSets[0].rgProperties[0].vValue.lVal;
				}
				else
					m_fDDLBehavior = NOT_SUPPORTED;

				if (rgSupportedPropSets[0].rgProperties[1].dwStatus != DBPROPSTATUS_NOTSUPPORTED)
				{
					if (V_BOOL(&(rgSupportedPropSets[0].rgProperties[1].vValue)) == VARIANT_TRUE)
					{
						m_eAsyncAbort = EASYNCTRUE;
					}
				}
				else
					m_eAsyncAbort = EASYNCNOTSUPPORTED;

				if (rgSupportedPropSets[0].rgProperties[2].dwStatus != DBPROPSTATUS_NOTSUPPORTED)
				{
					if (V_BOOL(&(rgSupportedPropSets[0].rgProperties[2].vValue)) == VARIANT_TRUE)
					{
						m_eAsyncCommit = EASYNCTRUE;
					}
				}
				else
					m_eAsyncCommit = EASYNCNOTSUPPORTED;			

				//Free our property memory from GetProperties
				FreeProperties(&cPropSets, &rgSupportedPropSets);
			}
		}
		
		//SetDataSourceObject ref counted this, so we can get rid of it
		if (pIDBInit){pIDBInit->Release();pIDBInit=NULL;}

		SetTable((CTable *)m_pThisTestModule->m_pVoid, DELETETABLE_NO);
			//Create a table we'll use for this session,	
/*			m_pCTable = new CTable(
													m_pIOpenRowset, 
													(LPWSTR)gwszModName
												);	
			if (!m_pCTable)
			{
				goto CLEANUP;
			}

			//Start with a table with rows		
			if (FAILED(((CTable *)m_pCTable)->CreateTable(NUM_ROWS)))
			{
				goto CLEANUP;
			}
			SetTable((CTable *)m_pCTable, DELETETABLE_NO);
*/
		//Get a session so we can fill in our ITxnLocal pointer
 		if (CHECK(CreateDBSession(), S_OK))
		{

			//We won't free pIUnk because we're just getting a direct copy,
			//no add ref is done
			if(CHECK(GetSessionObject(IID_ITransactionLocal, (IUnknown**)&m_pITxnLocal), S_OK))
			{
				hr = pIDBProp->GetPropertyInfo	(
													1, 
													&rgRowProps,
													&cPropertyInfoSets, 
													&prgPropertyInfoSets, 
													&ppDescBuffer
												);
				//should get info on all props
				if ((*prgPropertyInfoSets).cPropertyInfos!=g_PROPERTY_COUNT)
				{
					goto CLEANUP;
				}
				//record the property info
				for(i=0;i<g_PROPERTY_COUNT;i++)
				{
					g_rgDBPrpt[i].dwFlags=
						prgPropertyInfoSets[0].rgPropertyInfos[i].dwFlags;
				}

				if (CHECK(CreateRowsetObject(), S_OK))
				{
					//Find out which preserve properties are supported
					if (VerifyInterface(m_pIAccessor, IID_IRowsetInfo, ROWSET_INTERFACE,
						(IUnknown **)&pIInfo))
					{
						//Don't check return code as some or all of these may be unsupported
						pIInfo->GetProperties(1, &rgRowProps, &cPropSets, &rgSupportedPropSets);
											
						//Find the rowset property set
						for (i=0;i<cPropSets; i++)
						{
							if (rgSupportedPropSets[i].guidPropertySet == DBPROPSET_ROWSET)
							{
								//Copy the set we want to check
								memcpy(&RowsetPropSet, &rgSupportedPropSets[i], sizeof(DBPROPSET));
							}
						}
						//We don't ever expect not supported for these properties, so increment
						//the error count if we get it 
						if (COMPARE(RowsetPropSet.rgProperties[IDX_CommitPreserve].dwStatus != DBPROPSTATUS_NOTSUPPORTED, TRUE))
						{
							if (V_BOOL(&(RowsetPropSet.rgProperties[IDX_CommitPreserve].vValue)) == VARIANT_TRUE)
							{
								m_fCommitPreserve = TRUE;
							}
						}
					
						if (COMPARE(RowsetPropSet.rgProperties[IDX_AbortPreserve].dwStatus != DBPROPSTATUS_NOTSUPPORTED, TRUE))
						{
							if (V_BOOL(&(RowsetPropSet.rgProperties[IDX_AbortPreserve].vValue)) == VARIANT_TRUE)
							{
								m_fAbortPreserve = TRUE;
							}
						}
					
						//record the property info
						for(i=0;i<g_PROPERTY_COUNT;i++)
						{
							//mark the not supported properties
							if(RowsetPropSet.rgProperties[i].dwStatus==DBPROPSTATUS_NOTSUPPORTED)
							{
								g_rgDBPrpt[i].fSupported	=FALSE;
								g_rgDBPrpt[i].fDefault		=FALSE;
							}
							else
							{	
								if(RowsetPropSet.rgProperties[i].dwStatus!=DBPROPSTATUS_OK)
								{
									odtLog<<L"default value failed for properties indexed at "<<i<<L".\n";
								}
								//set supported properties 
								g_rgDBPrpt[i].fSupported	=TRUE;
								if (VT_BOOL==RowsetPropSet.rgProperties[i].vValue.vt)
								{
									g_rgDBPrpt[i].fDefault		=
										V_BOOL(&RowsetPropSet.rgProperties[i].vValue);
								}
								else
								{
									//this is so if passes the next condition
									g_rgDBPrpt[i].fDefault		= VARIANT_TRUE;
								}
							}
							//if property is not a writeable property and it is set to false, flag it
							if	(!	(	g_rgDBPrpt[i].dwFlags & DBPROPFLAGS_WRITE)	&&
										g_rgDBPrpt[i].fDefault == VARIANT_FALSE)
							{
								g_rgDBPrpt[i].fProviderSupported=FALSE;
							}
							else
							{
								g_rgDBPrpt[i].fProviderSupported=TRUE;
							}
						}

						//Free our property memory from GetProperties
						FreeProperties(&cPropSets, &rgSupportedPropSets);
						SAFE_RELEASE(pIInfo);

						//Now get a read accessor just so we can fill the binding struct for later
						if (CHECK(GetAccessorAndBindings(m_pIAccessor, DBACCESSOR_ROWDATA,
							&m_hReadAccessor, &m_rgReadBindings, &m_cReadBindings, &m_cbReadRowSize,			
  							DBPART_VALUE | DBPART_LENGTH | DBPART_STATUS),S_OK))
						{
							//Everything went OK, return TRUE
							fResults = TRUE;
							//Release accessor, we just want the bindings
							CHECK(m_pIAccessor->ReleaseAccessor(m_hReadAccessor, NULL), S_OK);
							m_hReadAccessor = DB_NULL_HACCESSOR;
						}						
					}
					//We'll keep the command, but want to start with a fresh
					//rowset for each variation
					ReleaseRowsetObject();
				}
			}
		}
	}
CLEANUP:
	
	SAFE_RELEASE(pIDBProp);
	FreeProperties(&cPropertyInfoSets, &prgPropertyInfoSets, &ppDescBuffer);
	PROVIDER_FREE(rgRowProps.rgPropertyIDs);
	return fResults;
}


//--------------------------------------------------------------------
// Sets the isolation level for outside transactions (autocommit mode)
// @mfunc Terminate
//--------------------------------------------------------------------
HRESULT	CTxnRowset::SetAutoCommitIsoLevel(ULONG	IsoLevel)
{
	CHECK(m_pITxnLocal->StartTransaction(IsoLevel, 0, NULL, NULL),S_OK);
	CHECK(m_pITxnLocal->Commit(FALSE, 0, 0),S_OK);

	return S_OK;			
}

//--------------------------------------------------------------------
// Cleans up the encapsulated CTxnRowset Object				
// @mfunc Terminate
//--------------------------------------------------------------------
BOOL	CTxnRowset::Terminate()
{
	if (m_pITxnLocal)
	{
		m_pITxnLocal->Release();
		m_pITxnLocal=NULL;
	}
	if(m_pCTable)
	{
		m_pCTable->DropTable();
		delete m_pCTable;
		m_pCTable = NULL;

	}
	m_cReadBindings = 0;

	if( m_rgReadBindings )
	{
		PROVIDER_FREE(m_rgReadBindings);
		m_rgReadBindings = NULL;
	}
	
	ReleaseRowsetObject();
	ReleaseCommandObject();
	ReleaseDBSession();
	ReleaseDataSourceObject();

	return TRUE;
}


//--------------------------------------------------------------------
// Commits this session's txn, 
// @mfunc Commit
//--------------------------------------------------------------------
HRESULT	CTxnRowset::Commit(BOOL fPreserveRowset, 
						   BOOL fNewUnitOfWork)
{
	//Commit this transaction 
	m_hr = m_pITxnLocal->Commit(fNewUnitOfWork, 0, 0);

	if (SUCCEEDED(m_hr))
	{
		//If the user wanted to keep the rowset around,
		//and the provider does not support preserving it,
		//create the rowset again
		if (fPreserveRowset && !m_fCommitPreserve)
		{
			//Release the zombie
			ReleaseRowsetObject();
			//Make new rowset
			CHECK(m_hr = MakeRowset(), S_OK);
		}
		else
			//If the user doesn't want the rowset, get rid of it.		
			if (!fPreserveRowset)
				ReleaseRowsetObject();		
	}			
	return ResultFromScode(m_hr);			
}

//#ifdef TXNJOIN
//--------------------------------------------------------------------
// Commits this session's txn, releases objects and unenlists session
// @mfunc Commit
//--------------------------------------------------------------------
HRESULT	CTxnRowset::CommitCoord(BOOL				fNewUnitOfWork,
								ITransaction		*pITransaction,
								ITransactionJoin	*pITransactionJoin)
{
	HRESULT	hr	= S_OK;

	//Commit this transaction 
	m_hr = pITransaction->Commit(fNewUnitOfWork, 0, 0);

	//Release the zombie
	ReleaseRowsetObject();

	if (SUCCEEDED(m_hr))
	{
		hr = pITransactionJoin->JoinTransaction	(	NULL,
													ISOLATIONLEVEL_UNSPECIFIED,
													0,
													NULL
												);
		if (S_OK!=hr)
		{
			return hr;
		}
	}		
	return ResultFromScode(m_hr);			
}
//#endif  //TXNJOIN

//--------------------------------------------------------------------
// Aborts this session's txn
// @mfunc Abort
//--------------------------------------------------------------------
HRESULT CTxnRowset::Abort(	BOOL	fPreserveRowset, 
							BOOL	fNewUnitOfWork)
{
	//Abort this transaction
	m_hr = m_pITxnLocal->Abort(NULL, fNewUnitOfWork, FALSE);

	if (SUCCEEDED(m_hr))
	{
		//If the user wanted to keep the rowset around,
		//and the provider does not support preserving it,
		//create the rowset again
		if (fPreserveRowset && !m_fAbortPreserve)
		{
			//Release the zombie
			ReleaseRowsetObject();
			//Make new rowset
			CHECK(m_hr = MakeRowset(), S_OK);
		}
		else
			//If the user doesn't want the rowset, get rid of it.		
			if (!fPreserveRowset)
				ReleaseRowsetObject();		
	}
	return ResultFromScode(m_hr);			
}

//#ifdef TXNJOIN
//--------------------------------------------------------------------
// Aborts this session's txn
// @mfunc Abort
//--------------------------------------------------------------------
HRESULT CTxnRowset::AbortCoord(	BOOL				fNewUnitOfWork,
								ITransaction		*pITransaction,
								ITransactionJoin	*pITransactionJoin)
{
	HRESULT		hr	= S_OK;

	//Abort this transaction
	m_hr = pITransaction->Abort(NULL, fNewUnitOfWork, FALSE);

	//Release the zombie
	ReleaseRowsetObject();

	if (SUCCEEDED(m_hr))
	{
		//unenlist
		hr = pITransactionJoin->JoinTransaction	(	NULL,
													ISOLATIONLEVEL_UNSPECIFIED,
													0,
													NULL
												);
		if (S_OK!=hr)
		{
			return hr;
		}
	}
	return ResultFromScode(m_hr);			
}
//#endif  //TXNJOIN

//--------------------------------------------------------------------
// Opens a rowset if needed and returns whether or not if found the
// hRow corresponding to ulRowNum.  Optionally returns the hRow, which
// the user must free if phFoundRow is not NULL on envocation.
// @mfunc FindRow
//--------------------------------------------------------------------
BOOL CTxnRowset::FindRow(ULONG ulRowNum, HROW * phFoundRow)
{
	const ULONG	cRows			= 10;
	HROW		rghRows[cRows];
	HROW		*phRows			= rghRows;	//Need this to pass HROW **
	BOOL		fFound			= FALSE;
	IRowset		*pIRowset		= NULL;
	DBCOUNTITEM	cRowsObtained	= 0; 
	ULONG		i				= 0;
	BYTE		*pData = NULL;
	HRESULT		hr;

	//This does nothing if the rowset exists already
	if (MakeRowset() != S_OK)
		goto CLEANUP;

	//Restart position just in case we are in the middle of the rowset
	if (!VerifyInterface(m_pIAccessor, IID_IRowset, ROWSET_INTERFACE, 
		(IUnknown **)&pIRowset))
		goto CLEANUP;	
	//Could get DB_S_COMMANDREEXECUTED here
	if (FAILED(pIRowset->RestartPosition(NULL)))
		goto CLEANUP;
	
	//Cleanup memory from the Init
	if (m_rgReadBindings)
	{
		PROVIDER_FREE(m_rgReadBindings);
		m_rgReadBindings = NULL;

		m_cReadBindings = 0;
		m_cbReadRowSize = 0;
	}

	//Get Accessor for reading
	if (!CHECK(GetAccessorAndBindings(m_pIAccessor, DBACCESSOR_ROWDATA,
		&m_hReadAccessor, &m_rgReadBindings, &m_cReadBindings, &m_cbReadRowSize,			
  		DBPART_VALUE | DBPART_LENGTH | DBPART_STATUS),S_OK))
		goto CLEANUP;		

	m_hr = NOERROR; 
	cRowsObtained = cRows;

	//Allocate our data buffer for GetData (note, outofline memory 
	//is cleaned up after every GetData by CompareData()
	pData = (BYTE *)PROVIDER_ALLOC(m_cbReadRowSize);
	if (!pData)
		goto CLEANUP;

	//Keep going while we have rows
	while (SUCCEEDED(m_hr) && cRowsObtained != 0)
	{
		m_hr = pIRowset->GetNextRows(NULL, 0, cRows, &cRowsObtained, &phRows);

		for (i=0; i<cRowsObtained; i++)
		{		
			hr = pIRowset->GetData(phRows[i], m_hReadAccessor, pData);
			//allow for DB_E_DELETED in case provider deleted/inserted changed row
			if (S_OK ==hr || DB_E_DELETEDROW == hr)
			{
				//Check this row against ulRowNum, free pData when done
				if (CompareData(m_pTable->CountColumnsOnTable(), m_rgTableColOrds,
					ulRowNum, pData, m_cReadBindings, m_rgReadBindings, 
					m_pTable, m_pIMalloc, PRIMARY, COMPARE_FREE, COMPARE_UNTIL_ERROR))
				{				
					//Give hRow back to user, if they asked for it
					if (phFoundRow)
					{
						*phFoundRow = phRows[i];
						//Increment ref count so it doesn't go away when we free the others
						CHECK(pIRowset->AddRefRows(1, &phRows[i], NULL, NULL), S_OK);
					}

					//We found the row, we can stop now
					fFound = TRUE;
					break;
				}		
			}
		}

		//Free all the rows for this GetNextRows call
		CHECK(pIRowset->ReleaseRows(cRowsObtained, phRows, NULL, NULL, NULL), S_OK);

		//If we found the row, we're done
		if (fFound)
			break;
	}
CLEANUP:

	if (pData)
		PROVIDER_FREE(pData);

	//Release accessor
	if ((m_hReadAccessor != DB_NULL_HACCESSOR) && m_pIAccessor)
	{
		CHECK(m_pIAccessor->ReleaseAccessor(m_hReadAccessor, NULL), S_OK);
		m_hReadAccessor = DB_NULL_HACCESSOR;
	}

	if (pIRowset)
	{
		pIRowset->Release();
		pIRowset=NULL;
	}

	return fFound;
}


//--------------------------------------------------------------------
// Makes a rowset - note, right now this is no different than directly
// calling CreateRowsetObject, but, this is a separate function in
// case additional steps need to be taken for creating our CTxnRowsets 
// in the future.  Such additional steps may include checking which
// properties were set.
// 
// @mfunc MakeRowset
//--------------------------------------------------------------------
HRESULT	CTxnRowset::MakeRowset()
{	
	const ULONG		cProps			= g_PROPERTY_COUNT-1;
	DBPROP			DBProp[cProps];	
	DBPROPSET		RowsetPropSet;
	WORD			i				= 0;

	if (!m_rgPropSets)
	{
		if (g_rgDBPrpt[IDX_IRowsetChange].fProviderSupported)
		{
			//Set properties for writeable rowsets -- for Delete, Change and NewRow
			DBProp[i].dwPropertyID= DBPROP_IRowsetChange;
			DBProp[i].dwOptions = DBPROPOPTIONS_REQUIRED;
			DBProp[i].colid=DB_NULLID;
			DBProp[i].vValue.vt=VT_BOOL;
			V_BOOL(&(DBProp[i].vValue))=VARIANT_TRUE;	
			i++;
			DBProp[i].dwPropertyID= DBPROP_UPDATABILITY;
			DBProp[i].dwOptions = DBPROPOPTIONS_REQUIRED;
			DBProp[i].colid=DB_NULLID;
			DBProp[i].vValue.vt=VT_I4;
			DBProp[i].vValue.lVal= DBPROPVAL_UP_CHANGE | DBPROPVAL_UP_INSERT | DBPROPVAL_UP_DELETE;
			i++;
		}
		
		if (g_rgDBPrpt[IDX_OtherUpdateDelete].fProviderSupported)
		{
			DBProp[i].dwPropertyID= DBPROP_OTHERUPDATEDELETE;
			DBProp[i].dwOptions=DBPROPOPTIONS_REQUIRED;
			DBProp[i].colid=DB_NULLID;
			DBProp[i].vValue.vt=VT_BOOL;
			V_BOOL(&(DBProp[i].vValue))=VARIANT_TRUE;
			i++;
		}
		if (g_rgDBPrpt[IDX_OtherInsert].fProviderSupported)
		{
			DBProp[i].dwPropertyID= DBPROP_OTHERINSERT;
			DBProp[i].dwOptions=DBPROPOPTIONS_REQUIRED;
			DBProp[i].colid=DB_NULLID;
			DBProp[i].vValue.vt=VT_BOOL;
			V_BOOL(&(DBProp[i].vValue))=VARIANT_TRUE;
			i++;
		}
		if (g_rgDBPrpt[IDX_CommandTimeout].fProviderSupported)
		{
			DBProp[i].dwPropertyID= DBPROP_COMMANDTIMEOUT;
			DBProp[i].dwOptions = DBPROPOPTIONS_REQUIRED;
			DBProp[i].colid=DB_NULLID;
			DBProp[i].vValue.vt=VT_I4;
			DBProp[i].vValue.lVal= 10;	//Set a command timeout of 10 seconds 
			i++;
		}

		//Build one set of rowset properties
		RowsetPropSet.rgProperties		= DBProp;
		RowsetPropSet.cProperties		= i;
		RowsetPropSet.guidPropertySet	= DBPROPSET_ROWSET;

		//Now we'll be ready to execute and get a buffered mode rowset
		SetRowsetProperties(&RowsetPropSet, 1);	
	}

	//We have an existing command with properties set
	//for other insert, update and delete visibility

	//We use order by numeric so that after we've made
	//some changes to the table, we can still guarantee
	//that g_ulLastActualDelete()+1 will give us the
	//first row in the rowset, which we bank on to do
	//GetData comparisons.
	m_hr = CreateRowsetObject();
	
	return ResultFromScode(m_hr);
	
}


//--------------------------------------------------------------------
// Makes a rowset - note, right now this is no different than directly
// calling CreateRowsetObject, but, this is a separate function in
// case additional steps need to be taken for creating our CTxnRowsets 
// in the future.  Such additional steps may include checking which
// properties were set.
// 
// @mfunc MakeRowset
//--------------------------------------------------------------------
HRESULT	CTxnRowset::MakeRowsetReadOnly()
{	
	const ULONG		cProps			= g_PROPERTY_COUNT-1;
	DBPROP			DBProp[cProps];	
	DBPROPSET		RowsetPropSet;
	WORD			i				= 0;

//	if (!m_rgPropSets)
//	{
		if (g_rgDBPrpt[IDX_IRowsetChange].fProviderSupported)
		{
			//Set properties for writeable rowsets -- for Delete, Change and NewRow
			DBProp[i].dwPropertyID		= DBPROP_IRowsetChange;
			DBProp[i].dwOptions			= DBPROPOPTIONS_REQUIRED;
			DBProp[i].colid				= DB_NULLID;
			DBProp[i].vValue.vt			= VT_BOOL;
			V_BOOL(&(DBProp[i].vValue))	= VARIANT_FALSE;
			i++;
			DBProp[i].dwPropertyID		= DBPROP_IRowsetUpdate;
			DBProp[i].dwOptions			= DBPROPOPTIONS_REQUIRED;
			DBProp[i].colid				= DB_NULLID;
			DBProp[i].vValue.vt			= VT_BOOL;
			V_BOOL(&(DBProp[i].vValue))	= VARIANT_FALSE;
			i++;			DBProp[i].dwPropertyID= DBPROP_UPDATABILITY;
			DBProp[i].dwOptions			= DBPROPOPTIONS_REQUIRED;
			DBProp[i].colid				= DB_NULLID;
			DBProp[i].vValue.vt			= VT_I4;
			DBProp[i].vValue.lVal		= 0;
			i++;
		}
		
		if (g_rgDBPrpt[IDX_OtherInsert].fProviderSupported)
		{
			DBProp[i].dwPropertyID= DBPROP_OTHERINSERT;
			DBProp[i].dwOptions=DBPROPOPTIONS_REQUIRED;
			DBProp[i].colid=DB_NULLID;
			DBProp[i].vValue.vt=VT_BOOL;
			V_BOOL(&(DBProp[i].vValue))=VARIANT_TRUE;
			i++;
		}
		if (g_rgDBPrpt[IDX_OtherUpdateDelete].fProviderSupported)
		{
			DBProp[i].dwPropertyID= DBPROP_OTHERUPDATEDELETE;
			DBProp[i].dwOptions=DBPROPOPTIONS_REQUIRED;
			DBProp[i].colid=DB_NULLID;
			DBProp[i].vValue.vt=VT_BOOL;
			V_BOOL(&(DBProp[i].vValue))=VARIANT_TRUE;
			i++;
		}

		//Build one set of rowset properties
		RowsetPropSet.rgProperties		= DBProp;
		RowsetPropSet.cProperties		= i;
		RowsetPropSet.guidPropertySet	= DBPROPSET_ROWSET;

		//Now we'll be ready to execute and get a buffered mode rowset
		SetRowsetProperties(&RowsetPropSet, 1);	
//	}

	//We have an existing command with properties set
	//for other insert, update and delete visibility

	//We use order by numeric so that after we've made
	//some changes to the table, we can still guarantee
	//that g_ulLastActualDelete()+1 will give us the
	//first row in the rowset, which we bank on to do
	//GetData comparisons.
	m_hr = CreateRowsetObject();
	
	return ResultFromScode(m_hr);	
}



//--------------------------------------------------------------------
// @mfunc CTOR
//--------------------------------------------------------------------
CTxnChgRowset::CTxnChgRowset(LPWSTR tcName) : CTxnRowset(tcName)
{
	m_fChange = FALSE;
	m_fUpdate = FALSE;

	m_cUpdateBindings = 0;
	m_rgUpdateBindings = NULL;
	m_hUpdateAccessor = DB_NULL_HACCESSOR;
	m_cbUpdateRowSize = 0;

	m_cUpdateableCols = 0;
	m_rgUpdateableCols = NULL;

	m_fTxnPendingInsert = FALSE;
	m_fTxnPendingDelete = FALSE;	
	m_fTxnPendingChange = FALSE;	

	m_fTxnStarted = FALSE;

	m_fBindLongData = NO_BLOB_COLS;
}

//--------------------------------------------------------------------
// @mfunc Copies info about whether txn is started and 
// pending changes for the given rowset
//--------------------------------------------------------------------
void CTxnChgRowset::CopyTxnInfo(CTxnChgRowset * pTxnChgRowset)
{
	m_fTxnStarted = pTxnChgRowset->m_fTxnStarted;
	m_fTxnPendingInsert = pTxnChgRowset->m_fTxnPendingInsert;
	m_fTxnPendingDelete = pTxnChgRowset->m_fTxnPendingDelete;
	m_fTxnPendingChange = pTxnChgRowset->m_fTxnPendingChange;
}
//--------------------------------------------------------------------
// @mfunc Init
//--------------------------------------------------------------------
BOOL	CTxnChgRowset::Init() 
{
	ULONG				i;
	CCol				TempCol;
	BOOL				fResults = FALSE;

	if (CTxnRowset::Init())
	{
		//Set correct properties for writeability
		SetTxnRowsetProperties();

		//Some of the properties such as Command Timeout could fail
		if (SUCCEEDED(CreateRowsetObject()))		
		{		
			//We will assume there is only one set -- the ROWSET properties
			ASSERT(m_cPropSets == 1);

			//Record support of IRowsetChange with INSERT, UPDATE and DELETE.
			if (g_rgDBPrpt[IDX_IRowsetChange].fProviderSupported)
			{
				m_fChange = TRUE;
			}
			//Record support of IRowsetUpdate.
			if (g_rgDBPrpt[IDX_IRowsetUpdate].fProviderSupported)
			{
				m_fUpdate = TRUE;
			}

			////////////////////////////////////////////////////////////////////
			//Find all updateable columns to use for NewRow and Change accessor
			////////////////////////////////////////////////////////////////////

			//Get memory to hold array of all col numbers.  NOTE:  This 
			//is the max possible, we won't necessarily use them all.
			m_rgUpdateableCols = (DBORDINAL *)PROVIDER_ALLOC(m_pTable->CountColumnsOnTable() * sizeof(DBORDINAL));
			if (!m_rgUpdateableCols)
				return FALSE;
							
			//We'll use this for the index for our array, and increment as we build it
			m_cUpdateableCols = 0;

			for (i = 1; i <= m_pTable->CountColumnsOnTable(); i++)
			{
				CHECK(m_pTable->GetColInfo(i, TempCol), S_OK);

				//Record the column number in the array
				//if it is updateable
				if (TempCol.GetUpdateable())
				{
					m_rgUpdateableCols[m_cUpdateableCols] = TempCol.GetColNum();
					m_cUpdateableCols++;				
				}
			}

			fResults = TRUE;
			//Get a NewRow and Change accessor, using only updateable columns
/*			if (CHECK(GetAccessorAndBindings(m_pIAccessor,
				DBACCESSOR_ROWDATA, 
				&m_hUpdateAccessor, &m_rgUpdateBindings, &m_cUpdateBindings, 
				&m_cbUpdateRowSize,			
  				DBPART_VALUE | DBPART_LENGTH | DBPART_STATUS,
				USE_COLS_TO_BIND_ARRAY, FORWARD, NO_COLS_BY_REF,
				NULL, NULL, NULL, DBTYPE_EMPTY, m_cUpdateableCols, 
				m_rgUpdateableCols, NULL, 
				NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, 
				m_fBindLongData),S_OK))
			{
				//Everything went OK, return TRUE
				fResults = TRUE;
				//Release accessor, we just want the bindings
				CHECK(m_pIAccessor->ReleaseAccessor(m_hUpdateAccessor, NULL), S_OK);
				m_hUpdateAccessor = DB_NULL_HACCESSOR;
			}we are now creating bindings each time since long data requirements may change*/

			//Keep command so we always have the right properties set
			//for subsequent rowset generation, but get rid of rowset
			//so we can start new for each variation
//			ReleaseRowsetObject();
			if( m_rgTableColOrds )
			{
				PROVIDER_FREE(m_rgTableColOrds);
				m_rgTableColOrds = NULL;
			}	

			if(m_pIAccessor)
			{
				m_pIAccessor->Release();
				m_pIAccessor = NULL;
			}
		}
	}
	return fResults;	
}


//--------------------------------------------------------------------
// @mfunc Sets the properties to generate the correct rowset on CreateRowsetObject
//--------------------------------------------------------------------
BOOL CTxnChgRowset::SetTxnRowsetProperties()
{
	const ULONG			cProps	= g_PROPERTY_COUNT-1;
	DBPROP				*rgDBProps;
	DBPROPSET			PropSet;
	WORD				i		= 0;

	rgDBProps = (DBPROP *)PROVIDER_ALLOC(sizeof(DBPROP) * cProps);
	if (!rgDBProps)
		return FALSE;

	//Set properties for writeable rowsets -- for Delete, Change and NewRow
	if (g_rgDBPrpt[IDX_IRowsetChange].fProviderSupported)
	{
		//Set properties for writeable rowsets -- for Delete, Change and NewRow
		rgDBProps[i].dwPropertyID= DBPROP_IRowsetChange;
		rgDBProps[i].dwOptions = DBPROPOPTIONS_REQUIRED;
		rgDBProps[i].colid=DB_NULLID;
		rgDBProps[i].vValue.vt=VT_BOOL;
		V_BOOL(&(rgDBProps[i].vValue))=VARIANT_TRUE;	
		i++;
		rgDBProps[i].dwPropertyID= DBPROP_UPDATABILITY;
		rgDBProps[i].dwOptions = DBPROPOPTIONS_REQUIRED;
		rgDBProps[i].colid=DB_NULLID;
		rgDBProps[i].vValue.vt=VT_I4;
		rgDBProps[i].vValue.lVal= DBPROPVAL_UP_CHANGE | DBPROPVAL_UP_INSERT | DBPROPVAL_UP_DELETE;
		i++;
	}
		
	if (g_rgDBPrpt[IDX_OtherInsert].fProviderSupported)
	{
		rgDBProps[i].dwPropertyID= DBPROP_OTHERINSERT;
		rgDBProps[i].dwOptions=DBPROPOPTIONS_REQUIRED;
		rgDBProps[i].colid=DB_NULLID;
		rgDBProps[i].vValue.vt=VT_BOOL;
		V_BOOL(&(rgDBProps[i].vValue))=VARIANT_TRUE;
		i++;
	}
	if (g_rgDBPrpt[IDX_OtherUpdateDelete].fProviderSupported)
	{
		rgDBProps[i].dwPropertyID= DBPROP_OTHERUPDATEDELETE;
		rgDBProps[i].dwOptions=DBPROPOPTIONS_REQUIRED;
		rgDBProps[i].colid=DB_NULLID;
		rgDBProps[i].vValue.vt=VT_BOOL;
		V_BOOL(&(rgDBProps[i].vValue))=VARIANT_TRUE;
		i++;
	}
	if (g_rgDBPrpt[IDX_CommandTimeout].fProviderSupported)
	{
		rgDBProps[i].dwPropertyID= DBPROP_COMMANDTIMEOUT;
		rgDBProps[i].dwOptions = DBPROPOPTIONS_REQUIRED;
		rgDBProps[i].colid=DB_NULLID;
		rgDBProps[i].vValue.vt=VT_I4;
		rgDBProps[i].vValue.lVal= 10;	//Set a command timeout of 10 seconds 
		i++;
	}
	//Build the Set struct to set all our rowset properties
	PropSet.rgProperties	= rgDBProps;
	PropSet.cProperties		= i;
	PropSet.guidPropertySet = DBPROPSET_ROWSET;

	//Set property in CRowset object
	SetRowsetProperties(&PropSet, 1);	

	PROVIDER_FREE(rgDBProps);

	return TRUE;
}


//--------------------------------------------------------------------
// Commits this session's txn, 
// @mfunc Commit
//--------------------------------------------------------------------
HRESULT	CTxnChgRowset::Commit(BOOL fPreserveRowset, 
							  BOOL fNewUnitOfWork)
{	
	m_hr = CTxnRowset::Commit(fPreserveRowset, fNewUnitOfWork);

	if (SUCCEEDED(m_hr))
		{
		//Since we actually commited, go ahead and increment row num
		//accordingly if we have done any inserts, deletes or changes
		//so we have a valid row num to use for next operations
		if (m_fTxnPendingInsert)		
		{
			m_fTxnPendingInsert = FALSE;
			//g_ulLastActualInsert++;
			g_InsertIncrement();
		}
		if (m_fTxnPendingDelete)		
		{
			m_fTxnPendingDelete = FALSE;
			//g_ulLastActualDelete++;
			g_DeleteIncrement();
		}
		if (m_fTxnPendingChange)	
		{
			m_fTxnPendingChange = FALSE;	
			//g_ulLastActualDelete++;
			//g_ulLastActualInsert++;
			g_DeleteAndInsertIncrement();
		}	
	
		//Record whether or not we started a new txn
		m_fTxnStarted = fNewUnitOfWork;
	}
	
	return ResultFromScode(m_hr);			
}


//--------------------------------------------------------------------
// Starts a txn at the specified level
// @mfunc Abort
//--------------------------------------------------------------------
HRESULT	CTxnChgRowset::StartTxn(ISOLEVEL isoLevel)
{
	ULONG	ulTransactionLevel;

	if (SUCCEEDED(m_hr = m_pITxnLocal->StartTransaction(isoLevel,
			0, NULL, &ulTransactionLevel)))
		//Mark us as in a txn so we don't increment row 
		//counts until the changes have been committed
		m_fTxnStarted = TRUE;
	
	return ResultFromScode(m_hr);	
}

//#ifdef TXNJOIN
//--------------------------------------------------------------------
// Commits this session's txn, 
// @mfunc Commit
//--------------------------------------------------------------------
HRESULT	CTxnChgRowset::CommitCoord(	BOOL				fNewUnitOfWork,
									ITransaction		*pITransaction,
									ITransactionJoin	*pITransactionJoin)
{	
	m_hr = CTxnRowset::CommitCoord(fNewUnitOfWork, pITransaction,pITransactionJoin);

	if (SUCCEEDED(m_hr))
	{
		//Since we actually commited, go ahead and increment row num
		//accordingly if we have done any inserts, deletes or changes
		//so we have a valid row num to use for next operations
		if (m_fTxnPendingInsert)		
		{
			m_fTxnPendingInsert = FALSE;
		}
		if (m_fTxnPendingDelete)		
		{
			m_fTxnPendingDelete = FALSE;
		}
		if (m_fTxnPendingChange)	
		{
			m_fTxnPendingChange = FALSE;	
		}	
		//Record whether or not we started a new txn
		m_fTxnStarted = fNewUnitOfWork;
	}
	
	return ResultFromScode(m_hr);			
}

//--------------------------------------------------------------------
// get TxnJoin
// @mfunc Abort
//--------------------------------------------------------------------
HRESULT	CTxnChgRowset::GetTxnJoin()		// @parm [OUT] Session Pointer)
{
	if(m_pITxnJoin)
		m_pITxnJoin = NULL;

	if(m_pIOpenRowset)
	{
		if(!VerifyInterface(m_pIOpenRowset, IID_ITransactionJoin, SESSION_INTERFACE, (IUnknown**)&m_pITxnJoin))
			return E_NOINTERFACE;

		return S_OK;
	}

	return E_FAIL;
}

//--------------------------------------------------------------------
// get Start DTC, coordinated transaction 
// @mfunc Abort
//--------------------------------------------------------------------
BOOL	CTxnChgRowset::StartCoordTxn(	ITransaction	**ppITransaction,
										ISOLEVEL		isolevel)		// @parm [OUT] Session Pointer)
{
	ITransactionDispenser	*pTransactionDispenser	= NULL;
	HRESULT					hr;

	if(ppITransaction)
	{
		*ppITransaction = NULL;
	}

	// Obtain the ITransactionDispenser Interface pointer
	// by calling DtcGetTransactionManager()
	hr = DtcGetTransactionManager(
				NULL,								// LPTSTR	 pszHost,
				NULL,								// LPTSTR	 pszTmName,
				IID_ITransactionDispenser,			// /* in  */ REFIID rid,
				0,									// /* in  */ DWORD	dwReserved1,
				0, 									// /* in  */ WORD	wcbReserved2,
				NULL,								// /* in  */ void FAR * pvReserved2,
				(void**)&pTransactionDispenser 				// /* out */ void** ppvObject
				);
	if (FAILED (hr))
	{	
		return FALSE;
	}
	
	hr = pTransactionDispenser->BeginTransaction( 
					NULL,							//	/* [in]  */ IUnknown __RPC_FAR *punkOuter,
					isolevel,						//	/* [in]  */ ISOLEVEL isoLevel,
					0,								// 	/* [in]  */ ULONG isoFlags,
					NULL,							//	/* [in]  */ ITransactionOptions *pOptions 
					(ITransaction**)ppITransaction	//	/* [out] */ ITransaction **ppTransaction
					);
	
	SAFE_RELEASE(pTransactionDispenser);

	if (FAILED (hr))
	{	
		return FALSE;
	}
	
	return TRUE;
}

//--------------------------------------------------------------------
// Free Txn Join object
// @mfunc 
//--------------------------------------------------------------------
BOOL	CTxnChgRowset::FreeJoinTxn()		
{
	SAFE_RELEASE(m_pITxnJoin);
	return TRUE;
}

//--------------------------------------------------------------------
// Free Coordinated Txn object
// @mfunc 
//--------------------------------------------------------------------
BOOL	CTxnChgRowset::FreeCoordTxn(ITransaction	*pITransaction)		
{
	//end any open transaction
	pITransaction->Abort(NULL,FALSE,FALSE);
	m_fTxnStarted = FALSE;
	SAFE_RELEASE(pITransaction);
	return TRUE;
}
//#endif  //TXNJOIN

//--------------------------------------------------------------------
// Aborts this session's txn
// @mfunc Abort
//--------------------------------------------------------------------
HRESULT CTxnChgRowset::Abort(BOOL fPreserveRowset, 
							 BOOL fNewUnitOfWork)
{	
	//Abort this transaction
	m_hr = CTxnRowset::Abort(fPreserveRowset, fNewUnitOfWork);

	if (SUCCEEDED(m_hr))
	{
		//Since we actually aborted, turn pending status off
		if (m_fTxnPendingInsert)		
		{
			m_fTxnPendingInsert = FALSE;			
		}	
		if (m_fTxnPendingDelete)		
		{
			m_fTxnPendingDelete = FALSE;			
		}
		if (m_fTxnPendingChange)	
		{
			m_fTxnPendingChange = FALSE;				
		}

		//Record whether or not we started a new txn
		m_fTxnStarted = fNewUnitOfWork;		
	}

	return ResultFromScode(m_hr);		
}                 

//#ifdef TXNJOIN
//--------------------------------------------------------------------
// Aborts this session's txn
// @mfunc Abort
//--------------------------------------------------------------------
HRESULT CTxnChgRowset::AbortCoord(	BOOL				fNewUnitOfWork,
									ITransaction		*pITransaction,
									ITransactionJoin	*pITransactionJoin)
{	
	//Abort this transaction
	m_hr = CTxnRowset::AbortCoord(fNewUnitOfWork,pITransaction,pITransactionJoin);

	if (SUCCEEDED(m_hr))
	{
		//Since we actually aborted, turn pending status off
		if (m_fTxnPendingInsert)		
		{
			m_fTxnPendingInsert = FALSE;			
		}	
		if (m_fTxnPendingDelete)		
		{
			m_fTxnPendingDelete = FALSE;			
		}
		if (m_fTxnPendingChange)	
		{
			m_fTxnPendingChange = FALSE;				
		}

		//Record whether or not we started a new txn
		m_fTxnStarted = fNewUnitOfWork;		
	}
	return ResultFromScode(m_hr);		
}
//#endif  //TXNJOIN

//--------------------------------------------------------------------
// Inserts a row based on ulRowNum and returns TRUE, if IRowsetNewRow is supported.
// Else it returns FALSE.
// @mfunc Insert
//--------------------------------------------------------------------
BOOL	CTxnChgRowset::Insert(	ULONG	ulRowNum, 
								HROW	*phNewRow,
								BOOL	fDEFAULT)
{
	BYTE				*pData			= NULL;
	BOOL				fResults		= FALSE;
	IRowsetChange		*pIRowsetChange = NULL;
	HROW				hNewRow;
	ULONG				i				= 0;


	//Since Insert isn't supported, just return FALSE
	//although test should never be here with this FALSE or test is not coded correctly
	if (!m_fChange)
		goto CLEANUP;

	//Create Rowset object if it doesn't exist yet.
	//If it does exist, this will do nothing
	if (MakeRowset() != S_OK)
		goto CLEANUP;

	//Get IRowsetNewRow 
	if (!VerifyInterface(m_pIAccessor, IID_IRowsetChange, ROWSET_INTERFACE,
		(IUnknown **)&pIRowsetChange))
		goto CLEANUP;

	//Get the accessor for inserting a row
	if (!CHECK(GetAccessorAndBindings(m_pIAccessor,
				DBACCESSOR_ROWDATA, 
				&m_hUpdateAccessor, &m_rgUpdateBindings, &m_cUpdateBindings, 
				&m_cbUpdateRowSize,			
  				DBPART_VALUE | DBPART_LENGTH | DBPART_STATUS,
				USE_COLS_TO_BIND_ARRAY, FORWARD, NO_COLS_BY_REF,
				NULL, NULL, NULL, DBTYPE_EMPTY, m_cUpdateableCols, 
				(LONG_PTR*)m_rgUpdateableCols, NULL, 
				NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, 
				m_fBindLongData),S_OK))	
		goto CLEANUP;
	
	//Allocate memory for the new row	
	pData = (BYTE *)PROVIDER_ALLOC(m_cbUpdateRowSize);
	if (!pData)		
		goto CLEANUP;
	
	//Set data for all columns
	if (!CHECK(m_hr = FillInputBindings(m_pTable, 
		DBACCESSOR_ROWDATA, m_cUpdateBindings, m_rgUpdateBindings, &pData, 
		ulRowNum, m_pTable->CountColumnsOnTable(), m_rgTableColOrds), S_OK))
		goto CLEANUP;

	//set all status in pData to DEFAULT
	if (fDEFAULT)
	{
		for (i=0;i<m_cUpdateBindings;i++)
		{
//			*(DBSTATUS *)((DWORD)pData + m_rgUpdateBindings[i].obStatus)=DBSTATUS_S_DEFAULT;
			STATUS_BINDING(m_rgUpdateBindings[i],pData)=DBSTATUS_S_DEFAULT;
		}
	}

	//Now Insert the row
	if (pIRowsetChange->InsertRow(NULL, m_hUpdateAccessor,pData, &hNewRow) == S_OK)		
	{
		fResults = TRUE;	
	}

	if (phNewRow)
	{
		*phNewRow = hNewRow;
	}
	else
	{
		IRowset * pIRowset;	
		
		//Cleanup row -- user doesn't want it
		if (!VerifyInterface(pIRowsetChange, IID_IRowset, ROWSET_INTERFACE,
			(IUnknown **)&pIRowset))
			goto CLEANUP;	
		CHECK(pIRowset->ReleaseRows(1, &hNewRow, NULL, NULL, NULL), S_OK);
		if (pIRowset)
		{
			pIRowset->Release();
			pIRowset=NULL;
		}
	}

CLEANUP:	
	//Cleanup any out of line memory allocated in FillInputBindings and pData
	ReleaseInputBindingsMemory(m_cUpdateBindings, m_rgUpdateBindings, pData, TRUE);

	//Release accessor
	if (m_pIAccessor && (m_hUpdateAccessor != DB_NULL_HACCESSOR))
	{
		CHECK(m_pIAccessor->ReleaseAccessor(m_hUpdateAccessor, NULL), S_OK);
		m_hUpdateAccessor = DB_NULL_HACCESSOR;
	}

	if (m_rgUpdateBindings)
	{
		PROVIDER_FREE(m_rgUpdateBindings);
		m_rgUpdateBindings = NULL;
	}
	//Release Change interface
	if (pIRowsetChange)
	{
		pIRowsetChange->Release();
		pIRowsetChange = NULL;
	}

	return fResults;
}

//--------------------------------------------------------------------
// Deletes a row based on ulRowNum and returns TRUE, if IRowsetDelete is supported.
// Else it returns FALSE.
// @mfunc Delete
//--------------------------------------------------------------------
BOOL	CTxnChgRowset::Delete(ULONG ulRowNum, HROW * phFoundRow)
{
	BOOL			fResults = FALSE;
	HROW			hFoundRow = DB_NULL_HROW;
	IRowset *		pIRowset = NULL;
	IRowsetChange * pIRowsetChange = NULL;

	//Since Delete isn't supported, just return FALSE
	if (!m_fChange)
		goto CLEANUP;

	//Create Rowset object if it doesn't exist yet.
	//If it does exist, this will do nothing
	if (MakeRowset() != S_OK)
		goto CLEANUP;

	//Get IRowsetDelete	
	if (!VerifyInterface(m_pIAccessor, IID_IRowsetChange, ROWSET_INTERFACE,
		(IUnknown **)&pIRowsetChange))
		goto CLEANUP;	

	//Find the hRow
	if (FindRow(ulRowNum, &hFoundRow))
		//Now Delete the row
		if (pIRowsetChange->DeleteRows(NULL, 1, &hFoundRow, NULL) == S_OK)
			//We could successfully delete the row, return TRUE
			fResults = TRUE;			

CLEANUP:
	
	if (hFoundRow != DB_NULL_HROW) 
		//If user wants row, give it back
		if (phFoundRow)
		{
			*phFoundRow = hFoundRow;
		}
		else
		{
			//Otherwise release row if we found it
			if (pIRowsetChange) 
			{
				if (VerifyInterface(pIRowsetChange, IID_IRowset, ROWSET_INTERFACE,
					(IUnknown **)&pIRowset))
				{
					CHECK(pIRowset->ReleaseRows(1, &hFoundRow, NULL, NULL, NULL), S_OK);
					pIRowset->Release();
					pIRowset=NULL;
				}
			}
		}

	//Release our delete interface
	if (pIRowsetChange)
	{
		pIRowsetChange->Release();
		pIRowsetChange = NULL;
	}

	return fResults;		
}

//--------------------------------------------------------------------
// Changes a row based on ulDeleteRowNum with values based on ulInsertRowNum
// and returns TRUE, if IRowsetChange is supported.  Else it returns FALSE.
// @mfunc Change
//--------------------------------------------------------------------
BOOL	CTxnChgRowset::Change(	ULONG	ulDeleteRowNum, 
								ULONG	ulInsertRowNum, 
								HROW	*phRow,
								BOOL	fIGNORE,
								BOOL	fDEFAULT)
{
	BOOL				fResults		= FALSE;
	HROW				hFoundRow		= DB_NULL_HROW;
	BYTE				*pData			= NULL;
	IRowset				*pIRowset		= NULL;
	IRowsetChange		*pIRowsetChange = NULL;
	ULONG				i				= 0;

	//Since Change isn't supported, just return FALSE
	if (!m_fChange)
		goto CLEANUP;

	//Create Rowset object if it doesn't exist yet.
	//If it does exist, this will do nothing
	if (MakeRowset() != S_OK)
		goto CLEANUP;

	//Get IRowsetChange 	
	if (!VerifyInterface(m_pIAccessor, IID_IRowsetChange, ROWSET_INTERFACE,
		(IUnknown **)&pIRowsetChange))
		goto CLEANUP;
	
	//Get the accessor for setting the new data	
	if (!CHECK(GetAccessorAndBindings(m_pIAccessor,
				DBACCESSOR_ROWDATA, 
				&m_hUpdateAccessor, &m_rgUpdateBindings, &m_cUpdateBindings, 
				&m_cbUpdateRowSize,			
  				DBPART_VALUE | DBPART_LENGTH | DBPART_STATUS,
				USE_COLS_TO_BIND_ARRAY, FORWARD, NO_COLS_BY_REF,
				NULL, NULL, NULL, DBTYPE_EMPTY, m_cUpdateableCols, 
				(LONG_PTR*)m_rgUpdateableCols, NULL, 
				NO_COLS_OWNED_BY_PROV, DBPARAMIO_NOTPARAM, 
				m_fBindLongData),S_OK))	
		goto CLEANUP;

	//Find the hRow
	if (FindRow(ulDeleteRowNum, &hFoundRow))
	{
		pData = (BYTE *)PROVIDER_ALLOC(m_cbUpdateRowSize);
		if (!pData)		
			goto CLEANUP;
	
		//Set data for all columns to new row values
		if (!CHECK(m_hr = FillInputBindings(m_pTable, 
			DBACCESSOR_ROWDATA, m_cUpdateBindings, m_rgUpdateBindings, &pData, 
			ulInsertRowNum, m_pTable->CountColumnsOnTable(), m_rgTableColOrds), S_OK))
			goto CLEANUP;

		//set all status in pData to IGNORE
		if (fIGNORE)
		{
			for (i=0;i<m_cUpdateBindings;i++)
			{
//				*(DBSTATUS *)((DWORD)pData + m_rgUpdateBindings[i].obStatus)=DBSTATUS_S_IGNORE;
				STATUS_BINDING(m_rgUpdateBindings[i],pData)=DBSTATUS_S_IGNORE;
			}
		}

		//set all status in pData to DEFAULT
		if (fDEFAULT)
		{
			for (i=0;i<m_cUpdateBindings;i++)
			{
//				*(DBSTATUS *)((DWORD)pData + m_rgUpdateBindings[i].obStatus)=DBSTATUS_S_DEFAULT;
				STATUS_BINDING(m_rgUpdateBindings[i],pData)=DBSTATUS_S_DEFAULT;
			}
		}

		if (pIRowsetChange->SetData(hFoundRow, m_hUpdateAccessor, pData) == S_OK)
		{
			fResults = TRUE;
		}
		//Cleanup any out of line memory allocated in FillInputBindings and pData
		ReleaseInputBindingsMemory(m_cUpdateBindings, m_rgUpdateBindings, pData, TRUE);
	}

CLEANUP:
	//Release row if we found it	
	if (hFoundRow != DB_NULL_HROW) 
	{
		//Give row to user if they want it
		if (phRow)
			*phRow = hFoundRow;
		//Otherwise get rid of it
		else
		{
			if (VerifyInterface(pIRowsetChange, IID_IRowset, ROWSET_INTERFACE,
				(IUnknown **)&pIRowset))
			{
				//May get error if there is a pending change, so don't check return code
				pIRowset->ReleaseRows(1, &hFoundRow, NULL, NULL, NULL);
				pIRowset->Release();
				pIRowset=NULL;
			}
		}
	}

	//Release accessor
	if (m_pIAccessor && (m_hUpdateAccessor != DB_NULL_HACCESSOR))
	{
		CHECK(m_pIAccessor->ReleaseAccessor(m_hUpdateAccessor, NULL), S_OK);
		m_hUpdateAccessor = DB_NULL_HACCESSOR;
	}

	if (m_rgUpdateBindings)
	{	
		PROVIDER_FREE(m_rgUpdateBindings);
		m_rgUpdateBindings = NULL;
	}

	//Release our change interface
	if (pIRowsetChange)
	{		
		pIRowsetChange->Release();		
		pIRowsetChange = NULL;
	}

	return fResults;	
}
	

//--------------------------------------------------------------------
// @mfunc Terminate
//--------------------------------------------------------------------
BOOL	CTxnChgRowset::Terminate() 
{	
	if (m_rgUpdateableCols)
		PROVIDER_FREE(m_rgUpdateableCols);
	
	//We release the rowset object in here
	return CTxnRowset::Terminate();
}


//--------------------------------------------------------------------
// @mfunc Init
//--------------------------------------------------------------------
BOOL	CTxnRORowset::Init() 
{
	if (CTxnRowset::Init())
	{
		//For all CreateRowsetObject calls, we want to make
		//sure the right properties are set
		SetTxnRowsetProperties();
		
		return TRUE;
	}
	
	return FALSE;

}

//--------------------------------------------------------------------
// @mfunc Sets the properties to generate the correct rowset on CreateRowsetObject
//--------------------------------------------------------------------
BOOL CTxnRORowset::SetTxnRowsetProperties()
{
	const ULONG			cProps	= g_PROPERTY_COUNT-2;	
	DBPROP *			rgDBProps;
	DBPROPSET			PropSet;
	WORD				i		= 0;

	rgDBProps = (DBPROP *)PROVIDER_ALLOC(sizeof(DBPROP) * cProps);
	if (!rgDBProps)
		return FALSE;

	if (g_rgDBPrpt[IDX_IRowsetChange].fProviderSupported)
	{
		//Set properties for writeable rowsets -- for Delete, Change and NewRow
		rgDBProps[i].dwPropertyID= DBPROP_IRowsetChange;
		rgDBProps[i].dwOptions = DBPROPOPTIONS_REQUIRED;
		rgDBProps[i].colid=DB_NULLID;
		rgDBProps[i].vValue.vt=VT_BOOL;
		V_BOOL(&(rgDBProps[i].vValue))=VARIANT_TRUE;	
		i++;
		rgDBProps[i].dwPropertyID= DBPROP_UPDATABILITY;
		rgDBProps[i].dwOptions = DBPROPOPTIONS_REQUIRED;
		rgDBProps[i].colid=DB_NULLID;
		rgDBProps[i].vValue.vt=VT_I4;
		rgDBProps[i].vValue.lVal= DBPROPVAL_UP_CHANGE | DBPROPVAL_UP_INSERT | DBPROPVAL_UP_DELETE;
		i++;
	}
		
	if (g_rgDBPrpt[IDX_OtherUpdateDelete].fProviderSupported)
	{
		rgDBProps[i].dwPropertyID= DBPROP_OTHERUPDATEDELETE;
		rgDBProps[i].dwOptions=DBPROPOPTIONS_REQUIRED;
		rgDBProps[i].colid=DB_NULLID;
		rgDBProps[i].vValue.vt=VT_BOOL;
		V_BOOL(&(rgDBProps[i].vValue))=VARIANT_TRUE;
		i++;
	}
	if (g_rgDBPrpt[IDX_CommandTimeout].fProviderSupported)
	{
		rgDBProps[i].dwPropertyID= DBPROP_COMMANDTIMEOUT;
		rgDBProps[i].dwOptions = DBPROPOPTIONS_REQUIRED;
		rgDBProps[i].colid=DB_NULLID;
		rgDBProps[i].vValue.vt=VT_I4;
		rgDBProps[i].vValue.lVal= 10;	//Set a command timeout of 10 seconds 
		i++;
	}

	//Build the Set struct to set all our rowset properties
	PropSet.rgProperties	= rgDBProps;
	PropSet.cProperties		= i;
	PropSet.guidPropertySet = DBPROPSET_ROWSET;

	//Set property in CRowset object
	SetRowsetProperties(&PropSet, 1);	

	PROVIDER_FREE(rgDBProps);
	
	return TRUE;	
}


//--------------------------------------------------------------------
// @mfunc Terminate
//--------------------------------------------------------------------
BOOL	CTxnRORowset::Terminate() 
{
	return CTxnRowset::Terminate();
}

//--------------------------------------------------------------------
// @mfunc CTOR
//--------------------------------------------------------------------
CTxn::CTxn(LPWSTR tcName) : CSessionObject(tcName)
{
	m_ulCurrentInsertRow = 0;
	m_ulCurrentDeleteRow = 0;

	m_fChaos = FALSE;
	m_fReadUncommitted = FALSE;
	m_fReadCommitted = FALSE;
	m_fRepeatableRead = FALSE;
	m_fSerializable = FALSE;	
}


//--------------------------------------------------------------------
// Inits the CTxn Object				
// @mfunc Init
//--------------------------------------------------------------------
BOOL	CTxn::Init()
{
	IDBProperties *	pIDBProp = NULL;	
	IDBInitialize * pIDBInit = NULL;
	const ULONG		cPropIDs = 2;
	DBPROPID		rgProp[cPropIDs];
	DBPROPIDSET		PropIDSet;
	DBPROPSET *		rgPropSets = NULL;
	ULONG			cPropSets = 0;	


	//Set up the Set of Data source Info props we want info on
	rgProp[0] = DBPROP_SUPPORTEDTXNISOLEVELS;
	rgProp[1] = DBPROP_DBMSNAME;
	PropIDSet.rgPropertyIDs = rgProp;
	PropIDSet.cPropertyIDs = cPropIDs;
	PropIDSet.guidPropertySet = DBPROPSET_DATASOURCEINFO;

	if (CSessionObject::Init())		
	{
		//Get a IDBInitialize ptr
		if (VerifyInterface(m_pThisTestModule->m_pIUnknown, IID_IDBInitialize,
			DATASOURCE_INTERFACE, (IUnknown **)&pIDBInit))
		{
			//Everything in the testcase will share this DSO
			SetDataSourceObject(pIDBInit, TRUE);

			//SetDataSourceObject ref counted this, so we can get rid of it
			if (pIDBInit){pIDBInit->Release();pIDBInit=NULL;}

			//Get the Txn Isolation Info
			if (VerifyInterface(m_pIDBInitialize, IID_IDBProperties, 
				DATASOURCE_INTERFACE, (IUnknown **)&pIDBProp))
			{			
				if (CHECK(pIDBProp->GetProperties(1, &PropIDSet, &cPropSets, 
					&rgPropSets), S_OK) && cPropSets == 1)
				{
					if (pIDBProp)
					{
						pIDBProp->Release();
						pIDBProp=NULL;
					}

					//Record the isolation levels supported					
					if (rgPropSets[0].rgProperties[0].vValue.lVal & DBPROPVAL_TI_CHAOS)
						m_fChaos = TRUE;
					if (rgPropSets[0].rgProperties[0].vValue.lVal & DBPROPVAL_TI_READUNCOMMITTED)
						m_fReadUncommitted = TRUE;
					if (rgPropSets[0].rgProperties[0].vValue.lVal & DBPROPVAL_TI_READCOMMITTED)
						m_fReadCommitted = TRUE;
					if (rgPropSets[0].rgProperties[0].vValue.lVal & DBPROPVAL_TI_REPEATABLEREAD)
						m_fRepeatableRead = TRUE;
					if (rgPropSets[0].rgProperties[0].vValue.lVal & DBPROPVAL_TI_SERIALIZABLE)
						m_fSerializable = TRUE;

					//Determine if we are on an Access datasource so we know to 
					//wait 5 seconds after update
					if (!wcscmp(V_BSTR(&(rgPropSets[0].rgProperties[1].vValue)), L"ACCESS"))
						m_fOnAccess = TRUE;
					else
						m_fOnAccess = FALSE;
					
					//Cleanup what we got back from GetPropertyInfo
					FreeProperties(&cPropSets, &rgPropSets);

					//Make sure Kagera is not support chaos for V1 (bug 2270)
					if (m_pThisTestModule->m_ProviderClsid == CLSID_MSDASQL)
					{
						COMPARE(m_fChaos, FALSE);
					}
										
					//Find the strictest one we can use for general purposes
					if (m_fSerializable)
						m_fIsoLevel = ISOLATIONLEVEL_SERIALIZABLE;
					else
						if (m_fRepeatableRead)
							m_fIsoLevel = ISOLATIONLEVEL_REPEATABLEREAD;
						else
							if (m_fReadUncommitted)
								m_fIsoLevel = ISOLATIONLEVEL_READUNCOMMITTED;
							else
								if (m_fReadCommitted)
									m_fIsoLevel = ISOLATIONLEVEL_READCOMMITTED;
								else 
									return FALSE;				
					
					//Make sure these are defined the same in the header file (Spec bug 2374)
					COMPARE(DBPROPVAL_TI_READUNCOMMITTED, DBPROPVAL_TI_BROWSE);
					COMPARE(DBPROPVAL_TI_READCOMMITTED, DBPROPVAL_TI_CURSORSTABILITY);
					COMPARE(DBPROPVAL_TI_SERIALIZABLE, DBPROPVAL_TI_ISOLATED);
										
					COMPARE(ISOLATIONLEVEL_READUNCOMMITTED, ISOLATIONLEVEL_BROWSE);
					COMPARE(ISOLATIONLEVEL_READCOMMITTED, ISOLATIONLEVEL_CURSORSTABILITY);
					COMPARE(ISOLATIONLEVEL_SERIALIZABLE, ISOLATIONLEVEL_ISOLATED);
					
					return TRUE;									
				}
				else
					SAFE_RELEASE(pIDBProp);
			}		
		}
	}

	return FALSE;
}

//--------------------------------------------------------------------	
// Inserts the next row into the table, on the given txn
// @mfunc Insert
//--------------------------------------------------------------------
BOOL	CTxn::Insert(CTxnChgRowset *pTxnChgRowset)
{
	ULONG	ulNewRow;

	//Find valid number to insert
	ulNewRow = GetNextRowToInsert();

	//We've already used this valid number on a previous pending
	//change, so increment it to get a new valid row
	if (pTxnChgRowset->m_fTxnPendingChange)
		ulNewRow++;
	
	//Record what row number we're using, so we know what to look for on validate
	m_ulCurrentInsertRow = ulNewRow;

	//Do the Insert
	if (pTxnChgRowset->Insert(ulNewRow))
	{
		//If we're in a transaction or delayed mode before calling
		//Update, record that the insert is pending
		if (pTxnChgRowset->m_fTxnStarted)
			pTxnChgRowset->m_fTxnPendingInsert = TRUE;
		else
		//We're not in a transaction, so we can record the actual insert
			//g_ulLastActualInsert++;
			g_InsertIncrement();

		return TRUE;
	}
	return FALSE;
}

//--------------------------------------------------------------------	
// Deletes the next row to delete in the table, on the given txn
// @mfunc Delete
//--------------------------------------------------------------------
BOOL	CTxn::Delete(CTxnChgRowset * pTxnChgRowset, HROW * phRow)
{	
	ULONG	ulDeleteRow;

	//Find valid number to delete
	ulDeleteRow = GetNextRowToDelete();

	//We've already used this valid number on a previous pending
	//change, so increment it to get a new valid row
	if (pTxnChgRowset->m_fTxnPendingChange)
		ulDeleteRow++;
	
	//Record what row number we're using, so we know what to look for on validate
	m_ulCurrentDeleteRow = ulDeleteRow;	

	//Do the Delete
	if (pTxnChgRowset->Delete(ulDeleteRow, phRow))
	{
		//If we're in a transaction or in deferred mode before
		//calling Update, record that the insert is pending
		if (pTxnChgRowset->m_fTxnStarted)
			pTxnChgRowset->m_fTxnPendingDelete = TRUE;
		else
		//We're not in a transaction, so we can record the actual delete
			//g_ulLastActualDelete++;
			g_DeleteIncrement();

		return TRUE;
	}
	return FALSE;
}

//--------------------------------------------------------------------	
// Changes the next row to delete in the table to the values
// based on the next row to insert, on the given txn
// @mfunc Change
//--------------------------------------------------------------------
BOOL	CTxn::Change(CTxnChgRowset * pTxnChgRowset)
{		
	ULONG	ulDeleteRow;
	ULONG	ulNewRow;

	//Find valid number to delete
	ulDeleteRow = GetNextRowToDelete();
	//Find valid number to insert
	ulNewRow = GetNextRowToInsert();

	//Record what row number we're using, so we know what to look for on validate
	//Note this is the new value which we're changing TO
	m_ulCurrentChangeRow = ulNewRow;	

	//We've already used this valid number on a previous pending
	//change, so increment it to get a new valid row
	if (pTxnChgRowset->m_fTxnPendingChange)
	{
		ulDeleteRow++;
		ulNewRow++;
	}
	
	//Do the Change
	if (pTxnChgRowset->Change(ulDeleteRow, ulNewRow))
	{
		//If we're in a transaction or in delayed update mode before
		//calling update, record that the insert is pending
		if (pTxnChgRowset->m_fTxnStarted)
			pTxnChgRowset->m_fTxnPendingChange = TRUE;
		else
		{
			//We're not in a transaction or pending update, 
			//so we can record the actual insert and delete
			//g_ulLastActualDelete++;
			//g_ulLastActualInsert++;
			g_DeleteAndInsertIncrement();
		}

		return TRUE;
	}
	
	return FALSE;
}

//--------------------------------------------------------------------	
// Starts txn with the IsoLevel given.
// @mfunc StartTxn
//--------------------------------------------------------------------
HRESULT CTxn::StartTxn(ITransactionLocal * pITxnLocal, ISOLEVEL IsoLevel)
{
	ULONG	ulTransactionLevel;

	m_hr = pITxnLocal->StartTransaction(IsoLevel,0, NULL, &ulTransactionLevel);			

	if(!ulTransactionLevel)
	{
		//ulTransactionLevel can not be zero
		odtLog << L"Test caused fail cause TxnLevel was illegally ZERO. \n";		
		return E_FAIL;
	}
	else
	{
		return ResultFromScode(m_hr);	
	}
}


//--------------------------------------------------------------------
// Determines whether or not a new unit of work has begun on the given 
// rowset's txn
// @mfunc NewUnitOfWork
//--------------------------------------------------------------------
BOOL	CTxn::NewUnitOfWork(CTxnChgRowset	*pTxnChgRowset, 
							CTxnRowset		*pTxnRowset)		
{
	//Make a change
	if (COMPARE(Change(pTxnChgRowset), TRUE))
	{
		//Abort the change
		if (CHECK(pTxnChgRowset->Abort(TRUE, TRUE), S_OK))
		{
			//Verify that we were indeed in a new unit of work, 
			//if abort worked and we can't see the change
			if (COMPARE(FindChange(pTxnRowset), FALSE))
			{
				//free the rowset.  so Change will the create a new rowset from
				//the backend.  this is because the backend will show the abort
				//but the rowset should not
				FreeProperties(&m_cPropSets, &m_rgPropSets);
				PROVIDER_FREE(m_rgPropSets);
				SAFE_RELEASE(((*(CCommandObject*)(&(*(CRowsetObject*)(&(*(CTxnRowset*)(&*pTxnChgRowset))))))).m_pIAccessor);

				//Make a change
				if (COMPARE(Change(pTxnChgRowset), TRUE))			
				{	//Commit the change
					if (CHECK(pTxnChgRowset->Commit(TRUE, TRUE), S_OK))
					{
						//Sleep for a few seconds; this is to ensure that the back end has had
						//time to make this update visible to other transactions which
						//should see it.  This is only necessary for Access, which only does
						//this every few seconds.
						if (m_fOnAccess)
						{
							Sleep(SLEEP_TIME);	//Takes milliseconds as param
						}

						//if another session can't see an update it won't matter
						//that the change is committed, it still can't see it
						if (g_rgDBPrpt[IDX_OtherUpdateDelete].fProviderSupported)
						{
							//Verify that we were indeed in a new unit of work, 
							//if commit worked and we can now see the change
							if (COMPARE(FindChange(pTxnRowset), TRUE))
							{
								return TRUE;
							}
						}
						else
						{
							return TRUE;
						}
					}
				}
			}
		}
	}
	//Something went wrong if we've made it here
	return FALSE;										
}
//#ifdef TXNJOIN
//--------------------------------------------------------------------
// Determines whether or not a new unit of work has begun on the given 
// rowset's txn
// @mfunc NewUnitOfWork
//--------------------------------------------------------------------
BOOL	CTxn::NewUnitOfWorkCoord(	CTxnChgRowset		*pTxnChgRowset, 
									CTxnRowset			*pTxnRowset,
									ITransaction		*pITransaction,
									ITransactionJoin	*pITransactionJoin)
{
	//Make a change
	if (COMPARE(Change(pTxnChgRowset), TRUE))
		//Abort the change
		if (CHECK(pTxnChgRowset->Abort(TRUE, TRUE), S_OK))
			//Verify that we were indeed in a new unit of work, 
			//if abort worked and we can't see the change
			if (COMPARE(FindChange(pTxnRowset), FALSE))
				//Make a change
				if (COMPARE(Change(pTxnChgRowset), TRUE))			
					//Commit the change
					if (CHECK(pTxnChgRowset->CommitCoord(TRUE,pITransaction,pITransactionJoin), S_OK))
					{
						//Sleep for a few seconds; this is to ensure that the back end has had
						//time to make this update visible to other transactions which
						//should see it.  This is only necessary for Access, which only does
						//this every few seconds.
						if (m_fOnAccess)
							Sleep(SLEEP_TIME);	//Takes milliseconds as param

						//Verify that we were indeed in a new unit of work, 
						//if commit worked and we can now see the change
						if (COMPARE(FindChange(pTxnRowset), TRUE))
							return TRUE;
				}
	//Something went wrong if we've made it here
	return FALSE;										
}
//#endif  //TXNJOIN
 
//--------------------------------------------------------------------
// Determines whether or not the rowset is functional on txn 1
// @mfunc RowsetFunctional
//--------------------------------------------------------------------
BOOL	CTxn::RowsetFunctional(CTxnRowset * pTxnRowset)		
{
	HROW	hRow		= DB_NULL_HROW;
	BYTE	*pData		= NULL;
	IRowset *pIRowset	= NULL;
	BOOL	fResults	= FALSE;

	//Allocate a buffer to GetData
	pData = (BYTE *)PROVIDER_ALLOC(pTxnRowset->m_cbReadRowSize);
	if (!pData)
		return FALSE;
	memset(pData, 7, (size_t)(pTxnRowset->m_cbReadRowSize));

	//Find a row
	if (COMPARE(pTxnRowset->FindRow(g_ulLastActualInsert, &hRow), TRUE))
	{
		//Get Accessor for reading
		if (!CHECK(pTxnRowset->m_pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA,		
				pTxnRowset->m_cReadBindings,  pTxnRowset->m_rgReadBindings,
				pTxnRowset->m_cbReadRowSize, &pTxnRowset->m_hReadAccessor, NULL), S_OK))
				goto CLEANUP;		

		//Get IRowset interface
		if (!VerifyInterface(pTxnRowset->m_pIAccessor, IID_IRowset, ROWSET_INTERFACE,
				(IUnknown **)&pIRowset))
				goto CLEANUP;


		//Fully functionally rowset should be able to GetData
		if (CHECK(pIRowset->GetData(hRow, pTxnRowset->m_hReadAccessor, pData), S_OK))
		{
			fResults = TRUE;
		}
		pIRowset->ReleaseRows(1, &hRow, NULL, NULL, NULL);
	}
	
CLEANUP:

	//Release accessor
	if ((pTxnRowset->m_hReadAccessor != DB_NULL_HACCESSOR) && pTxnRowset->m_pIAccessor)
	{
		CHECK(pTxnRowset->m_pIAccessor->ReleaseAccessor(pTxnRowset->m_hReadAccessor, NULL), S_OK);
		pTxnRowset->m_hReadAccessor = DB_NULL_HACCESSOR;
	}
   
	SAFE_RELEASE(pIRowset);
	PROVIDER_FREE(pData);
	return fResults;
}

//--------------------------------------------------------------------
// Determines whether or not the rowset is zombied on txn 1.  This will
// release the rowset, accessor and hRow, which the user must create
// before the rowset is zombied
// @mfunc RowsetZombied
//--------------------------------------------------------------------
BOOL	CTxn::RowsetZombied(CTxnRowset * pTxnRowset, IRowset** ppIRowset, HROW * phRow, HACCESSOR hAccessor)
{	
	BYTE		*pData			= NULL;
	BOOL		fResults		= FALSE;	
	HACCESSOR	hNULLAccessor	= NULL;

	DBORDINAL	cX					= 0;
	HROW		*rgX				= NULL;

	ASSERT(ppIRowset && *ppIRowset);

	//Allocate a buffer to GetData, in case it succeeds, so it doesn't overwrite anything
	pData = (BYTE *)PROVIDER_ALLOC(pTxnRowset->m_cbReadRowSize);
	if (!pData)
		return FALSE;

	//We should only get E_UNEXPECTED since the rowset should be zombied
	if (CHECK((*ppIRowset)->GetNextRows(NULL,0,0,&cX,&rgX), E_UNEXPECTED))
	{
		//We should only get E_UNEXPECTED since the rowset should be zombied
		if (CHECK((*ppIRowset)->GetData(*phRow, hAccessor, pData), E_UNEXPECTED))
		{
			//Should be able to release accessor
			if (CHECK(pTxnRowset->m_pIAccessor->ReleaseAccessor(hAccessor, NULL), S_OK))
			{
				hAccessor = DB_NULL_HACCESSOR;
			
				//ReleaseRows should still work
				if (CHECK((*ppIRowset)->ReleaseRows(1, phRow, NULL, NULL, NULL), S_OK))
				{
				 	*phRow = DB_NULL_HROW;
				
					//Releasing the rowset should also work
					SAFE_RELEASE(*ppIRowset);
					pTxnRowset->ReleaseRowsetObject();

					fResults = TRUE;
				}
			}
		}	
	}					

	//Release accessor
	if ((hAccessor != DB_NULL_HACCESSOR) && pTxnRowset->m_pIAccessor)
	{
		CHECK(pTxnRowset->m_pIAccessor->ReleaseAccessor(hAccessor, NULL), S_OK);		
	}
   
   	//Release Rows 
   	if(*phRow != DB_NULL_HROW)		
		if (CHECK((*ppIRowset)->ReleaseRows(1, phRow, NULL, NULL, NULL), S_OK))

   SAFE_RELEASE(*ppIRowset);
	PROVIDER_FREE(pData);

	//If we've already released the rowset, this won't do anything
	pTxnRowset->ReleaseRowsetObject();
	return fResults;
}

//--------------------------------------------------------------------
// Determines whether or not txn 1 is in autocommit mode
// @mfunc NoNewUnitOfWork
//--------------------------------------------------------------------
BOOL	CTxn::NoNewUnitOfWork(CTxnChgRowset	*pTxnChgRowset, 
							  CTxnRowset	*pTxnRowset)		
{
	//Make a change
	if (COMPARE(Change(pTxnChgRowset), TRUE))
	{
		//Sleep for a few seconds; this is to ensure that the back end has had
		//time to make this update visible to other transactions which
		//should see it.  This is only necessary for Access, which only does
		//this every few seconds.
		if (m_fOnAccess)
			Sleep(SLEEP_TIME);	//Takes milliseconds as param

		//Change should be visible without commit
		if (COMPARE(FindChange(pTxnRowset), TRUE))
		{			
			//Abort should error out
			if (CHECK(pTxnChgRowset->Abort(TRUE, TRUE), XACT_E_NOTRANSACTION))
			{				
				//Commit should also error out
				if (CHECK(pTxnChgRowset->Commit(TRUE, TRUE), XACT_E_NOTRANSACTION))
					return TRUE;
			}
		}
	}
	//Something went wrong if we've made it here
	return FALSE;		
}
	 
//#ifdef TXNJOIN
//--------------------------------------------------------------------
// Determines whether or not txn 1 is in autocommit mode
// @mfunc NoNewUnitOfWork
//--------------------------------------------------------------------
BOOL	CTxn::NoNewUnitOfWorkCoord(	CTxnChgRowset		*pTxnChgRowset, 
									CTxnRowset			*pTxnRowset,
									ITransaction		*pITransaction,
									BOOL				fChange,
									ITransactionJoin	*pITransactionJoin)		
{
	if (fChange)
	{
		//Make a change
		if (COMPARE(Change(pTxnChgRowset), TRUE))
		{
			//Sleep for a few seconds; this is to ensure that the back end has had
			//time to make this update visible to other transactions which
			//should see it.  This is only necessary for Access, which only does
			//this every few seconds.
			if (m_fOnAccess)
				Sleep(SLEEP_TIME);	//Takes milliseconds as param

			//Change should be visible without commit
			if (!COMPARE(FindChange(pTxnRowset), TRUE))
			{
				goto CLEANUP;
			}
		}
	}
	//Abort should error out
	if (CHECK(pTxnChgRowset->AbortCoord(FALSE,pITransaction,pITransactionJoin), XACT_E_NOTRANSACTION))
	{				
		//Commit should also error out
		if (CHECK(pTxnChgRowset->CommitCoord(FALSE,pITransaction,pITransactionJoin), XACT_E_NOTRANSACTION))
		{
			return TRUE;
		}
	}
CLEANUP:
	//Something went wrong if we've made it here
	return FALSE;		
}
//#endif  //TXNJOIN	 

//--------------------------------------------------------------------
// Verifies info returned from GetTransactionInfo
// @mfunc VerifyTxnInfo
//--------------------------------------------------------------------
BOOL	CTxn::VerifyTxnInfo(ITransaction * pITxn, ISOLEVEL isoLevel, XACTUOW * puow)
{
	XACTTRANSINFO	TxnInfo;

	//Get Info on this transaction to verify
	if (CHECK(pITxn->GetTransactionInfo(&TxnInfo), S_OK))			
	{
		//Return the unit of work id if the user wants it
		if (puow)
			*puow = TxnInfo.uow;

		//Make sure Isolation level is what we set it for
		//or higher, since they can upgrade this		
		if (TxnInfo.isoLevel < isoLevel)
			(*m_pError)++;
		//Record exact info of what we recieved
		if (TxnInfo.isoLevel != isoLevel)
			odtLog << L"Set Isolation Level:  " << isoLevel << L"  Received Isolation Level:  " << TxnInfo.isoLevel << wszNewLine;

		//The rest of the values must be zero for V1 OLE DB
		if (COMPARE(TxnInfo.isoFlags, 0))
			if (COMPARE(TxnInfo.grfRMSupported, 0))				
				if (COMPARE(TxnInfo.grfTCSupportedRetaining, 0))
					if (COMPARE(TxnInfo.grfRMSupportedRetaining, 0))
						return TRUE;
	}

	return FALSE;		
}

//--------------------------------------------------------------------
// Cleans up the CTxn Object				
// @mfunc Terminate
//--------------------------------------------------------------------
BOOL	CTxn::Terminate()		
{	
	ReleaseDataSourceObject();
	return CSessionObject::Terminate();
}


//--------------------------------------------------------------------
// Copies the test case info from the current testcase into the CTxnRowset
// object.  This is needed since CTxnRowset inherits from CTestCases
// and may need to access some of the data members which are normally
// set in initialization of a testcase, which won't happen because
// we are encapsulating rather than inheriting from CTxnRowset.
// @mfunc CopyTestCaseInfo
//--------------------------------------------------------------------
void CTxn::CopyTestCaseInfo(CTestCases * pTC)
{
	pTC->SetOwningMod(0, this->m_pThisTestModule);
}


//--------------------------------------------------------------------
// CTOR
// @mfunc CTxnImmed
//--------------------------------------------------------------------
CTxnImmed::CTxnImmed(LPWSTR wszTestCaseName) : CTxn(wszTestCaseName)
{
	m_pChgRowset1	= NULL;
	m_pChgRowset2	= NULL;
	m_pRORowset1	= NULL;
}


//--------------------------------------------------------------------
// Inits the CTxnImmed Object.  It inits the encapsulated immed mode 
// rowset objects
// @mfunc Init
//--------------------------------------------------------------------
BOOL	CTxnImmed::Init()
{
	if (CTxn::Init())
	{
		//Create our encapsulated CTxnRowset objects
		m_pChgRowset1	= new CTxnChgRowset((LPWSTR)gwszThisModuleName);
		m_pChgRowset2	= new CTxnChgRowset((LPWSTR)gwszThisModuleName);
		m_pRORowset1	= new CTxnRORowset((LPWSTR)gwszThisModuleName);		

		if (m_pChgRowset1 && m_pChgRowset2 && m_pRORowset1)							
		{
			//Copy the stuff which normally gets initialized at
			//testcase initialization time, but didn't for these
			//objects since they are encapsulated rather than
			//inherited by the testcase object.
			CopyTestCaseInfo(m_pChgRowset1);
			CopyTestCaseInfo(m_pRORowset1);
			CopyTestCaseInfo(m_pChgRowset2);			

			//Set the same DSO and table for our encapsulated rowset objects
			//also create each rowset
			if (m_pChgRowset1->Init() &&
				m_pChgRowset2->Init() &&
				m_pRORowset1->Init())
				return TRUE;
		}
	}
	
		return FALSE;
}


//--------------------------------------------------------------------
// Releases all rowsets associated with encapsulated rowset objects
// and does an abort of all open transactions
// @mfunc ReleaseAllRowsetsAndTxns
//--------------------------------------------------------------------
void CTxnImmed::ReleaseAllRowsetsAndTxns()
{
	m_pChgRowset1->ReleaseRowsetObject();
	m_pRORowset1->ReleaseRowsetObject();
	m_pChgRowset2->ReleaseRowsetObject();	

	//We don't have a return value, since its just cleanup, 
	//which may not succeed because its not necessary
	if (m_pChgRowset1->m_pITxnLocal)
		m_pChgRowset1->Abort(FALSE, FALSE);
	if (m_pChgRowset2->m_pITxnLocal)
		m_pChgRowset2->Abort(FALSE, FALSE);
	if (m_pRORowset1->m_pITxnLocal)
		m_pRORowset1->Abort(FALSE, FALSE);
}

//--------------------------------------------------------------------
// Releases all rowsets associated with encapsulated rowset objects
// @mfunc ReleaseAllRowsets
//--------------------------------------------------------------------
void CTxnImmed::ReleaseAllRowsets()
{
	m_pChgRowset1->ReleaseRowsetObject();
	m_pRORowset1->ReleaseRowsetObject();
	m_pChgRowset2->ReleaseRowsetObject();	
}
//--------------------------------------------------------------------
// Get DBPROP_SUPPORTEDTXNISOLEVELS				
// @mfunc GetIslLevels
//--------------------------------------------------------------------
HRESULT CTxnImmed::GetIsoLevels(ISOLEVEL	*pIsoLevels) {

	ULONG_PTR		ulIsoLevels			=		0;
	IDBInitialize	*pIDBInitialize		=		NULL;
	HRESULT			hRes				=		E_FAIL;

	// Get IDBInitialize interface
	CHECK(VerifyInterface((IUnknown *)m_pThisTestModule->m_pIUnknown, IID_IDBInitialize, DATASOURCE_INTERFACE, (IUnknown**)&pIDBInitialize), true);

	// Determine if this is a supported property
	CHECK(SupportedProperty(DBPROP_SUPPORTEDTXNISOLEVELS, DBPROPSET_DATASOURCEINFO, pIDBInitialize), true);

	// Now get the property
	CHECK(GetProperty(DBPROP_SUPPORTEDTXNISOLEVELS, DBPROPSET_DATASOURCEINFO, pIDBInitialize, &ulIsoLevels), true);

	if (pIsoLevels)
		*pIsoLevels		=	(ISOLEVEL) ulIsoLevels;

	hRes		=	S_OK;

	SAFE_RELEASE(pIDBInitialize);

	return	hRes;
}

//--------------------------------------------------------------------
// Cleans up the CTxn Object				
// @mfunc Terminate
//--------------------------------------------------------------------
BOOL	CTxnImmed::Terminate()		
{	
	//Clean up encapsulated objects
	if (m_pRORowset1)
	{
		m_pRORowset1->Terminate();
		delete m_pRORowset1;
	}
	
	if (m_pChgRowset1)
	{
		m_pChgRowset1->Terminate();
		delete m_pChgRowset1;
	}

	if (m_pChgRowset2)
	{
		m_pChgRowset2->Terminate();
		delete m_pChgRowset2;
	}
	
	return CTxn::Terminate();
}