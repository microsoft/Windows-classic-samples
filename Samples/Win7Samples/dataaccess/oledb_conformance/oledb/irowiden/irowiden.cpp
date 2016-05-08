//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1995-2000 Microsoft Corporation.  
//
// @doc 
//
// @module IRowIden.cpp | Source file for test module IRowsetIdentity.
//
#include "modstandard.hpp"
#define  DBINITCONSTANTS	// Must be defined to initialize constants in OLEDB.H
#define  INITGUID
#include "IRowIden.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Module Values
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// {{ TCW_MODULE_GLOBALS
DECLARE_MODULE_CLSID = { 0x20ac17d1, 0xf171, 0x11cf, { 0x89, 0xc2, 0x00, 0xaa, 0x00, 0xb5, 0xa9, 0x1b }};
DECLARE_MODULE_NAME("IRowsetIdentity");
DECLARE_MODULE_OWNER("Microsoft");
DECLARE_MODULE_DESCRIP("The test module for IRowsetIdentity");
DECLARE_MODULE_VERSION(836168420);
// }}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Globals - - - - - - - - - - - - - - - - - -
#define g_MSDASQL	(m_pThisTestModule->m_ProviderClsid == CLSID_MSDASQL)

// - - - - - - - - - - - - - - - - - -
CTable				*g_pCTable			= NULL;		//pointer to the global table
IDBCreateSession	*g_pIDBCreateSession= NULL;		//pointer to IDBCreateSession interface
IDBCreateCommand	*g_pIDBCreateCommand= NULL;		//pointer to IDBCreateComand interface
IOpenRowset			*g_pIOpenRowset		= NULL;		//pointer to IOpenRowset interface
DB_LORDINAL			*g_rgTableColOrds;				//Array of col ordinals for table.			
DBORDINAL			g_cRowsetCols;					//Count of columns in rowset.
DBCOUNTITEM			g_ulRowCount;

//Flag whether the provider is readonly
BOOL	g_fReadOnlyProvider;

BOOL	g_fTestValid;

//Flag whether the props are Settable or TRUE
BOOL		g_fIRowsetChange;
BOOL		g_fCommands;
BOOL	    g_fIRowsetLocate;
BOOL		g_fROWSETUPDATABLE;
BOOL		g_fUPDATEPENDING;
BOOL		g_fFETCHBACKWARDS;
BOOL		g_fLOCATE;
BOOL		g_fIDBSchemaRowset;
BOOL		g_fLITERALIDENITIY;
BOOL		g_fSTRONGIDENTITY;
BOOL		g_fREMOVEDELETED;
BOOL		g_fRETURNPENDINGINSERTS;
BOOL		g_fOWNUPDATEDELETE;
BOOL		g_fOWNINSERT;
DBCOUNTITEM	g_ulNextRow;

//PendingDeletes

//0	- OWNUPDATEDELETE - T	LITERALIDENTITY - T	REMOVEDELETED - T	Deleted Row Removed
//1	- OWNUPDATEDELETE - T	LITERALIDENTITY - T	REMOVEDELETED - F	Deleted Row Present
//2	- OWNUPDATEDELETE - T	LITERALIDENTITY - F	REMOVEDELETED - F	Deleted Row Present
//3	- OWNUPDATEDELETE - T	LITERALIDENTITY - F	REMOVEDELETED - T	Deleted Row Present
//4	- OWNUPDATEDELETE - F	LITERALIDENTITY - T	REMOVEDELETED - T	No Change to Rowset Visible
//5	- OWNUPDATEDELETE - F	LITERALIDENTITY - T	REMOVEDELETED - F	No Change to Rowset Visible
//6	- OWNUPDATEDELETE - F	LITERALIDENTITY - F	REMOVEDELETED - F	No Change to Rowset Visible
//7	- OWNUPDATEDELETE - F	LITERALIDENTITY - F	REMOVEDELETED - T	No Change to Rowset Visible

//PendingInserts

//0	- OWNINSERT - T	LITERALIDENTITY - T	RETURNPENDINGINSERTS - T	Inserted Row Present
//1	- OWNINSERT - T	LITERALIDENTITY - T	RETURNPENDINGINSERTS - F	Inserted Row not Seen
//2	- OWNINSERT - T	LITERALIDENTITY - F	RETURNPENDINGINSERTS - F	Inserted Row not Seen
//3	- OWNINSERT - T	LITERALIDENTITY - F	RETURNPENDINGINSERTS - T	Inserted Row not Seen
//4	- OWNINSERT - F	LITERALIDENTITY - T	RETURNPENDINGINSERTS - T	No Change to Rowset Visible
//5	- OWNINSERT - F	LITERALIDENTITY - T	RETURNPENDINGINSERTS - F	No Change to Rowset Visible
//6	- OWNINSERT - F	LITERALIDENTITY - F	RETURNPENDINGINSERTS - F	No Change to Rowset Visible
//7	- OWNINSERT - F	LITERALIDENTITY - F	RETURNPENDINGINSERTS - T	No Change to Rowset Visible

//PendingUpdates(static)

//0	- OWNUPDATEDELETE - T	LITERALIDENTITY - T	row handle held		Change is Seen
//1	- OWNUPDATEDELETE - T	LITERALIDENTITY - T	row handle not held	Change is Seen
//2	- OWNUPDATEDELETE - T	LITERALIDENTITY - F	row handle not held	Change is Seen
//3	- OWNUPDATEDELETE - T	LITERALIDENTITY - F	row handle held		Change is Seen
//4	- OWNUPDATEDELETE - F	LITERALIDENTITY - T	row handle held		Change is Seen
//5	- OWNUPDATEDELETE - F	LITERALIDENTITY - T	row handle not held	Change is Not Seen
//6	- OWNUPDATEDELETE - F	LITERALIDENTITY - F	row handle not held	Change is Not Seen
//7	- OWNUPDATEDELETE - F	LITERALIDENTITY - F	row handle held		Change is Not Seen

//PendingUpdates(key)

//follow rules for Inserts and Deletes


enum ePrptIdx	
{
	IDX_LiteralIdentity=0,
	IDX_StrongIdentity,
	IDX_FetchBackwards, 
	IDX_ScrollBackwards, 
	IDX_CanHoldRows,
	IDX_RemoveDeleted, 
	IDX_OwnUpdateDelete,
	IDX_OwnInsert,
	IDX_OtherUpdateDelete,	
	IDX_OtherInsert,
	IDX_IRowsetIdentity,
	IDX_IRowsetUpdate,
	IDX_IRowsetLocate,
	IDX_IRowsetChange
};
const ULONG			g_PROPERTY_COUNT	= IDX_IRowsetChange+1;
const WORD			g_wePrptIdxFIRST	= IDX_LiteralIdentity;
const WORD			g_wePrptIdxLAST		= IDX_IRowsetChange;

//record the properties default values
struct	DBPrptRecord
{
	DBPROPID		dwPropID;
	BOOL			fSupported;
	VARIANT_BOOL	fDefault;
	DBPROPFLAGS		dwFlags;
}g_rgDBPrpt[g_PROPERTY_COUNT];


//--------------------------------------------------------------------
// @func	Print out message on the screen that a particular property
//			is not supported.
//--------------------------------------------------------------------
BOOL PrintNotSupported(ULONG	cProperties)
{
	BOOL	fPass=TRUE;

	switch(cProperties)
	{
		case IDX_LiteralIdentity:
				odtLog<<L"DBPROP_LITERALIDENTITY is not supported!\n";
			break;
		case IDX_StrongIdentity:
				odtLog<<L"DBPROP_STRONGIDENTITY is not supported!\n";
			break;
		case IDX_FetchBackwards:
				odtLog<<L"DBPROP_FETCHBACKWARDS is not supported!\n";
			break;
		case IDX_ScrollBackwards:
				odtLog<<L"DBPROP_SCROLLBACKWARDS is not supported!\n";
			break;
		case IDX_CanHoldRows:
				odtLog<<L"DBPROP_CANHOLDROWS is not supported!\n";
			break;
		case IDX_RemoveDeleted:
				odtLog<<L"DBPROP_REMOVEDELETED is not supported!\n";
			break;
		case IDX_OwnUpdateDelete:
				odtLog<<L"DBPROP_OWNUPDATEDELETE is not supported!\n";
			break;
		case IDX_OwnInsert:
				odtLog<<"DBPROP_OWNINSERT is not supported!\n";
			break;
		case IDX_OtherUpdateDelete:
				odtLog<<L"DBPROP_OTHERUPDATEDELETE is not supported!\n";
			break;
		case IDX_OtherInsert:
				odtLog<<"DBPROP_OTHERINSERT is not supported!\n";
			break;
		case IDX_IRowsetUpdate:
				odtLog<<L"IID_IRowsetUpdate is not supported!\n";
			break;
		case IDX_IRowsetLocate:
				odtLog<<L"IID_IRowsetLocate is not supported!\n";
			break;
		case IDX_IRowsetChange:
				odtLog<<L"IID_IRowsetChange is not supported!\n";
			break;
		default:
				odtLog<<L"Index to the property array is incorrect!\n";
				fPass=FALSE;
			break;
	}
	return fPass;
}

//--------------------------------------------------------------------
// @func Module level initialization routine
//
// Return the status value indicated by the binding structure 
//--------------------------------------------------------------------
DBSTATUS GetStatus(void *pData, DBBINDING *pBinding)
{
	return STATUS_BINDING(*pBinding,pData);
}


//--------------------------------------------------------------------
// @func Module level initialization routine
//
// @rdesc Success or Failure
// 		@flag  TRUE  | Successful initialization
//		@flag  FALSE | Initialization problems
//
BOOL ModuleInit(CThisTestModule * pThisTestModule)
{
	HRESULT				hr;
	ULONG				cProperties				= 0;
	ULONG				cPropertiesIndex		= 0;
	DBPROPIDSET			DBPropIDSet;

	ULONG				cNumTableRows			= 0;
	ULONG				cPropertyInfoSets		= 0;
	DBPROPINFOSET		*prgPropertyInfoSets	= NULL;

	BOOL				fInit					= FALSE;
	DBPROPSET			*prgProperties			= NULL;

	IRowset				*pIRowset				= NULL;
	IRowsetInfo			*pIRowsetInfo			= NULL;		// IRowsetChange Object
	IDBProperties		*pIDBProperties			= NULL;
	IDBSchemaRowset		*pIDBSchemaRowset		= NULL;

	ULONG				cPropSets				= 0;
	DBPROPSET			rgPropSets[1];
	WCHAR*				pwszProviderName		= NULL;

	OLECHAR				*ppDescBuffer			= NULL;

	g_pCTable					= NULL;
	DBPropIDSet.rgPropertyIDs	= NULL;
	DBPropIDSet.cPropertyIDs	= g_PROPERTY_COUNT;
	DBPropIDSet.guidPropertySet	= DBPROPSET_ROWSET;

	//Create a Data Source Object and Initialize
	if(!ModuleCreateDBSession(pThisTestModule))
		return FALSE;

	g_fReadOnlyProvider	= FALSE;
	g_fIRowsetChange	= FALSE;
	g_fCommands			= FALSE;
	g_fIRowsetLocate	= FALSE;
	g_fROWSETUPDATABLE	= FALSE;
	g_fUPDATEPENDING	= FALSE;
	g_fFETCHBACKWARDS	= FALSE;
	g_fLOCATE			= FALSE;
	g_fIDBSchemaRowset	= FALSE;
	g_fLITERALIDENITIY	= FALSE;
	g_fSTRONGIDENTITY	= FALSE;
	g_fREMOVEDELETED	= FALSE;
	g_fRETURNPENDINGINSERTS = FALSE;
	g_fOWNUPDATEDELETE	= FALSE;
	g_fOWNINSERT		= FALSE;

	//Todo: check wheter the provider is read only
	g_fReadOnlyProvider = IsProviderReadOnly(pThisTestModule->m_pIUnknown2);
	//Todo: check if provider supports IRowsetUpdatable or ICommand

	//TRUE if interface COULD be supported
	if (IsReqInterface(ROWSET_INTERFACE,IID_IRowsetChange) || !(GetModInfo()->IsStrictLeveling()))
	{
		g_fIRowsetChange = TRUE;
	}
	if (IsReqInterface(COMMAND_INTERFACE,IID_ICommand) || !(GetModInfo()->IsStrictLeveling()))
	{
		g_fCommands = TRUE;
	}
	if (IsReqInterface(COMMAND_INTERFACE,IID_IRowsetLocate) || !(GetModInfo()->IsStrictLeveling()))
	{
		g_fIRowsetLocate = TRUE;
	}

	pwszProviderName = GetProviderName(pThisTestModule->m_pIUnknown2);

	//IDBCreateSession
	if(!VerifyInterface(pThisTestModule->m_pIUnknown, IID_IDBCreateSession, DATASOURCE_INTERFACE, (IUnknown**)&g_pIDBCreateSession))
		return FALSE;
	
	//IDBCreateCommand
	if(!VerifyInterface(pThisTestModule->m_pIUnknown2, IID_IDBCreateCommand, SESSION_INTERFACE, (IUnknown**)&g_pIDBCreateCommand))
	{
		odtLog << L"IDBCreateCommand is not supported by Provider." << ENDL;
	}     

	//IOpenRowset
	if(!VerifyInterface(pThisTestModule->m_pIUnknown2, IID_IOpenRowset, SESSION_INTERFACE, (IUnknown**)&g_pIOpenRowset))
	{
		odtLog << L"IOpenRowset is not supported by Provider." << ENDL;
	}     
	//IOpenRowset
	if(!VerifyInterface(g_pIOpenRowset, IID_IDBSchemaRowset, SESSION_INTERFACE, (IUnknown**)&pIDBSchemaRowset))
	{
		odtLog << L"IDBSchemaRowset is not supported by Provider." << ENDL;
	}     
	else
	{
		g_fIDBSchemaRowset	= TRUE;
	}

	//limit the number of rows in the table for weak providers that can't handle
	//but just a little bit of rows
	if (g_fReadOnlyProvider)
	{
		cNumTableRows = 9;
	}
	else
	{
		cNumTableRows = TABLE_ROW_COUNT;
	}

	//create the table
	g_pCTable = new CTable	(
								pThisTestModule->m_pIUnknown2, 
								(WCHAR *)gwszModuleName, 
								USENULLS
							);

	if(!g_pCTable || 
		!SUCCEEDED(g_pCTable->CreateTable(cNumTableRows,1,NULL,PRIMARY,TRUE)))
	{													  	
		if(g_pCTable)									  	
		{
			g_pCTable->DropTable();
			delete g_pCTable;							  	
			g_pCTable=NULL;
		}
		odtLog<<wszCreateTableFailed;					  	
		return FALSE;									                                     
	}

	//save the row count of the table
	g_ulRowCount	= cNumTableRows;

	//make sure IRowsetIdentity interface is supported by calling
	//IOpenRowset to return a Rowset from table
	hr = g_pCTable->CreateRowset(
									USE_OPENROWSET,
									IID_IRowsetIdentity,
									cPropSets,
									rgPropSets,
									(IUnknown**)&pIRowset,
									NULL,
									&g_cRowsetCols,
									&g_rgTableColOrds
								);

	//if E_NOINTERFACE is returned, IRowsetIdentity is not supported by the provider
	if(hr==ResultFromScode(E_NOINTERFACE))
	{
		odtLog<<wszIRowsetIdentityNotSupported;
		return FALSE;
	}
	if(hr!=ResultFromScode(S_OK))
	{
		odtLog<<wszExcuteCommandFailed;
		return FALSE;
	}

	//check if properites are supported
	DBPropIDSet.rgPropertyIDs=(DBPROPID *)PROVIDER_ALLOC(g_PROPERTY_COUNT*sizeof(DBPROPID));
	memset(DBPropIDSet.rgPropertyIDs,0xCA,g_PROPERTY_COUNT * sizeof(DBPROPID));
	//init all the properties
	DBPropIDSet.rgPropertyIDs[IDX_LiteralIdentity]	= DBPROP_LITERALIDENTITY;
	DBPropIDSet.rgPropertyIDs[IDX_StrongIdentity]	= DBPROP_STRONGIDENTITY;
	DBPropIDSet.rgPropertyIDs[IDX_FetchBackwards]	= DBPROP_CANFETCHBACKWARDS;
	DBPropIDSet.rgPropertyIDs[IDX_ScrollBackwards]	= DBPROP_CANSCROLLBACKWARDS;
	DBPropIDSet.rgPropertyIDs[IDX_CanHoldRows]		= DBPROP_CANHOLDROWS;
	DBPropIDSet.rgPropertyIDs[IDX_RemoveDeleted]	= DBPROP_REMOVEDELETED;
	DBPropIDSet.rgPropertyIDs[IDX_OwnUpdateDelete]	= DBPROP_OWNUPDATEDELETE;
	DBPropIDSet.rgPropertyIDs[IDX_OwnInsert]		= DBPROP_OWNINSERT;
	DBPropIDSet.rgPropertyIDs[IDX_OtherUpdateDelete]= DBPROP_OTHERUPDATEDELETE;
	DBPropIDSet.rgPropertyIDs[IDX_OtherInsert]		= DBPROP_OTHERINSERT;
	DBPropIDSet.rgPropertyIDs[IDX_IRowsetIdentity]	= DBPROP_IRowsetIdentity;
	DBPropIDSet.rgPropertyIDs[IDX_IRowsetUpdate]	= DBPROP_IRowsetUpdate;
	DBPropIDSet.rgPropertyIDs[IDX_IRowsetLocate]	= DBPROP_IRowsetLocate;
	DBPropIDSet.rgPropertyIDs[IDX_IRowsetChange]	= DBPROP_IRowsetChange;
	
	//mark everything as supported
	for(cPropertiesIndex=g_wePrptIdxFIRST;cPropertiesIndex<=g_wePrptIdxLAST;cPropertiesIndex++)
	{
		g_rgDBPrpt[cPropertiesIndex].dwPropID	= DBPropIDSet.rgPropertyIDs[cPropertiesIndex];
		g_rgDBPrpt[cPropertiesIndex].fSupported	= TRUE;
		g_rgDBPrpt[cPropertiesIndex].fDefault	= FALSE;
	}

	//check property info
	if(!VerifyInterface	(
							g_pIDBCreateSession, 
							IID_IDBProperties, 
							DATASOURCE_INTERFACE, 
							(IUnknown**)&pIDBProperties
						)
		)
	{
		goto CLEANUP;
	}

	hr = pIDBProperties->GetPropertyInfo	(
												1, 
												&DBPropIDSet,
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
	for(cPropertiesIndex=g_wePrptIdxFIRST;cPropertiesIndex<=g_wePrptIdxLAST;cPropertiesIndex++)
	{
		g_rgDBPrpt[cPropertiesIndex].dwFlags=
			prgPropertyInfoSets[0].rgPropertyInfos[cPropertiesIndex].dwFlags;
	}

	//get property info on the rowset 
	if(!VerifyInterface(pIRowset, IID_IRowsetInfo, ROWSET_INTERFACE, (IUnknown**)&pIRowsetInfo))
	{
		goto CLEANUP;
	}
	//Get Property values
	hr=pIRowsetInfo->GetProperties(1,&DBPropIDSet, &cProperties, &prgProperties);
	//should get info on all props
	if ((*prgProperties).cProperties!=g_PROPERTY_COUNT)
	{
		goto CLEANUP;
	}
	//mark the properties
	for(cPropertiesIndex=g_wePrptIdxFIRST;cPropertiesIndex<=g_wePrptIdxLAST;cPropertiesIndex++)
	{
		//mark the not supported properties
		if(prgProperties[0].rgProperties[cPropertiesIndex].dwStatus==DBPROPSTATUS_NOTSUPPORTED)
		{
			g_rgDBPrpt[cPropertiesIndex].fSupported	=FALSE;
			g_rgDBPrpt[cPropertiesIndex].fDefault	=FALSE;
			PrintNotSupported(cPropertiesIndex);
		}
		else
		{	
			if(prgProperties[0].rgProperties[cPropertiesIndex].dwStatus!=DBPROPSTATUS_OK)
			{
				odtLog<<L"Error: default value failed for properties indexed at "<<cPropertiesIndex<<L".\n";
			}
			//mark as supported properties VARIANT_FALSE
			g_rgDBPrpt[cPropertiesIndex].fSupported	=TRUE;
			g_rgDBPrpt[cPropertiesIndex].fDefault	=V_BOOL(&prgProperties[0].rgProperties[cPropertiesIndex].vValue);
		}
		
		//a prop can not be not supported and WRTIABLE
		if ((!g_rgDBPrpt[cPropertiesIndex].fSupported)&&(g_rgDBPrpt[cPropertiesIndex].dwFlags & DBPROPFLAGS_WRITE))	
		{
			odtLog<<L"Error:Prop is NOT SUPPORTED and WRITABLE.\n";
			goto CLEANUP;
		}
		//a prop can not be not supported and default TRUE
		if ((!g_rgDBPrpt[cPropertiesIndex].fSupported)&&(g_rgDBPrpt[cPropertiesIndex].fDefault))	
		{
			odtLog<<L"Error:Prop is NOT SUPPORTED and default is TRUE.\n";
			goto CLEANUP;
		}
	}
				
	//if FetchBackwards is not a writeable property and it is set to false, flag it
	if	(	(	g_rgDBPrpt[IDX_FetchBackwards].dwFlags & DBPROPFLAGS_WRITE)	||
				g_rgDBPrpt[IDX_FetchBackwards].fDefault == VARIANT_TRUE)
	{
		g_fFETCHBACKWARDS=TRUE;
	}
	//if IRoswsetChange is not a writeable property and it is set to false, flag it
	if	(	(	g_rgDBPrpt[IDX_IRowsetChange].dwFlags & DBPROPFLAGS_WRITE)	||
				g_rgDBPrpt[IDX_IRowsetChange].fDefault == VARIANT_TRUE)
	{
		g_fROWSETUPDATABLE=TRUE;
	}
	//if IRoswsetLocate is not a writeable property and it is set to false, flag it
	if	(	(	g_rgDBPrpt[IDX_IRowsetLocate].dwFlags & DBPROPFLAGS_WRITE)	||
				g_rgDBPrpt[IDX_IRowsetLocate].fDefault == VARIANT_TRUE)
	{
		g_fLOCATE=TRUE;
	}
	//if IRoswsetUpdate is not a writeable property and it is set to false, flag it
	if	(	(	g_rgDBPrpt[IDX_IRowsetUpdate].dwFlags & DBPROPFLAGS_WRITE)	||
				g_rgDBPrpt[IDX_IRowsetUpdate].fDefault == VARIANT_TRUE)
	{
		g_fUPDATEPENDING=TRUE;
	}

	//if DBPROP_LITERALIDENTITY is not a writeable property and it is set to false, flag it
	if	(	(	g_rgDBPrpt[IDX_LiteralIdentity].dwFlags & DBPROPFLAGS_WRITE)	||
				g_rgDBPrpt[IDX_LiteralIdentity].fDefault == VARIANT_TRUE)
	{
		g_fLITERALIDENITIY=TRUE;
	}

	//if DBPROP_STRONGIDENTITY is not a writeable property and it is set to false, flag it
	if	(	(	g_rgDBPrpt[IDX_StrongIdentity].dwFlags & DBPROPFLAGS_WRITE)	||
				g_rgDBPrpt[IDX_StrongIdentity].fDefault == VARIANT_TRUE)
	{
		g_fSTRONGIDENTITY=TRUE;
	}

	//if DBPROP_REMOVEDELETED is not a writeable property and it is set to false, flag it
	if	(	(	g_rgDBPrpt[IDX_RemoveDeleted].dwFlags & DBPROPFLAGS_WRITE)	||
				g_rgDBPrpt[IDX_RemoveDeleted].fDefault == VARIANT_TRUE)
	{
		g_fREMOVEDELETED=TRUE;
	}

	fInit=TRUE;
CLEANUP:
	// Release interface pointers
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pIRowsetInfo);
	SAFE_RELEASE(pIDBProperties);
	SAFE_RELEASE(pIDBSchemaRowset);

	PROVIDER_FREE(pwszProviderName);
	
	//free the memory
	PROVIDER_FREE(DBPropIDSet.rgPropertyIDs);
	FreeProperties(&cProperties, &prgProperties);
	FreeProperties(&cPropertyInfoSets, &prgPropertyInfoSets, &ppDescBuffer);

	return fInit;
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
	ULONG	cCnt	= 0;

	//if an ini file is being used then delete and repopulate
	if(GetModInfo()->GetFileName() && g_pCTable)
	{
		//delete all rows in the table.
		if(!CHECK(g_pCTable->DeleteRows(ALLROWS),S_OK))
			return FALSE;

		// Regenerate the rowset
		if(!CHECK(g_pCTable->Insert(PRIMARY, 1, g_ulRowCount),S_OK))
			return FALSE;
	}

	//drop tables
	if(g_pCTable)
	{
		g_pCTable->DropTable();
		delete g_pCTable;
	}

	SAFE_RELEASE(g_pIDBCreateSession);
	SAFE_RELEASE(g_pIDBCreateCommand);
	SAFE_RELEASE(g_pIOpenRowset);

	PROVIDER_FREE(g_rgTableColOrds);

	//Release IDBCreateCommand interface
	return (ModuleReleaseDBSession(pThisTestModule));
}	


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Base Test Case Section
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//
//	TCIRowsetIdentity:	the base class for the rest of test cases in this
//						test module. 
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
class TCIRowsetIdentity : public CRowsetObject
{
	private:
	protected:
		//@cmember: interface pointer for IRowsetIdentity
		IRowsetIdentity		*m_pIRowsetIdentity;

		//@cmember: interface pointer for IRowsetChange
		IRowsetChange		*m_pIRowsetChange;

		//@cmember: interface pointer for IRowsetUpdate
		IRowsetUpdate		*m_pIRowsetUpdate;

		//@cmember: interface pointer for IRowsetLocate
		IRowsetLocate		*m_pIRowsetLocate;

		//@cmember: interface pointer for IRowset
		IRowset				*m_pIRowset;

		//@cmember: interface pointer for IRowset
		IDBSchemaRowset		*m_pIDBSchemaRowset;

		//@cmember:	accessory handle
		HACCESSOR			m_hAccessor;

		//@cmember:	the size of a row
		DBLENGTH			m_cRowSize;

		//@cmember:	the count of binding structure
		DBCOUNTITEM		m_cBinding;

		//@cmember: the array of binding strucuTRUE
		DBBINDING			*m_rgBinding;

		//@cmember: the column information
		DBCOLUMNINFO		*m_rgInfo;

		//@cmember: the string buffer to hold the name
		WCHAR				*m_pStringsBuffer;

		//@cmember:	the pointer to the row buffer
		void				*m_pData;

		//@cmember: the location of accessor handle
		EACCESSORLOCATION	m_eAccessorLocation;

		//@cmember: The Updatability Flags for DBPROP_UPDATABILITY
		ULONG				m_ulUpdFlags;

		//@mfunc: initialialize interface pointers
		BOOL	Init();

		//@mfunc: Terminate 
		BOOL	Terminate();

		//@mfunc: Create a command object and set properties, execute a sql statement,
		//		  and create a rowset object.  Create an accessor on the rowset 
		BOOL GetRowsetAndAccessor
		(	
			EQUERY				eSQLStmt,
			ULONG				cProperties=0,			
			const DBPROPID		*rgProperties=NULL,			
			ULONG				cPropertiesUnset=0,
			const DBPROPID		*rgPropertiesUnset=NULL,
			EACCESSORLOCATION	eAccessorLocation=NO_ACCESSOR,
			DBACCESSORFLAGS		dwAccessorFlags=DBACCESSOR_ROWDATA,		
			DBPART				dwPart=DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,					
			ECOLS_BOUND			eColsToBind=ALL_COLS_BOUND,			
			ECOLUMNORDER		eBindingOrder=FORWARD,			
			ECOLS_BY_REF		eColsByRef=NO_COLS_BY_REF,				
			DBTYPE				dbTypeModifier=DBTYPE_EMPTY,
			DBORDINAL			cColsToBind=0,
			ULONG_PTR			*rgColsToBind=NULL
		);

		//@mfun: create an accessor on the rowset.  
		BOOL	GetAccessorOnRowset
		(
			EACCESSORLOCATION	eAccessorLocation=ON_ROWSET_ACCESSOR,
			DBACCESSORFLAGS		dwAccessorFlags=DBACCESSOR_ROWDATA,		
			DBPART				dwPart=DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,					
			ECOLS_BOUND			eColsToBind=UPDATEABLE_COLS_BOUND,			
			ECOLUMNORDER		eBindingOrder=FORWARD,			
			ECOLS_BY_REF		eColsByRef=NO_COLS_BY_REF,				
			DBTYPE				dbTypeModifier=DBTYPE_EMPTY,
			DBORDINAL			cColsToBind=0,
			ULONG_PTR			*rgColsToBind=NULL
		);

		BOOL	CheckExpectedData
		(
			HROW	hRow,		
			void	*pExpectedData, 
			BOOL	fSetData
		);


		HRESULT	InsertOneRow
		(
			DBCOUNTITEM	cRowNumber, 
			HROW		*pHRow
		);
		
		HRESULT	ChangeOneRow
		(
			HROW		hRow,
			DBCOUNTITEM	cRowNumber,
			void		**ppData=NULL
		);

		HRESULT	DeleteRows
		(
			DBCOUNTITEM	cRows,
			HROW		*pHRow
		);

		HRESULT	UpdateRows
		(
			DBCOUNTITEM		cRows,
			HROW			*pHRow,
			DBROWSTATUS		**ppDBRowStatus=NULL
		);

		HRESULT	UndoRows
		(
			DBCOUNTITEM	cRows,
			HROW		*pHRow
		);

		HRESULT ReleaseRows
		(
			DBCOUNTITEM	cRows,
			HROW		**ppHRow,
			BOOL		fFreeMemory
		);

		//@mfun: Get the bookmark for the row 
		BOOL	GetBookmark
		(
			DBCOUNTITEM	ulRow,
			DBBKMARK	*pcbBookmark,
			BYTE		**ppBookmark
		);

		BOOL	GetBookmarkByRow
		(
			HROW		hRow,
			DBBKMARK	*pcbBookmark,
			BYTE		**ppBookmark
		);

		//@mfunc: Get cursor type of the rowset
		ECURSOR	GetCursorType();

		//@mfunc: Return TRUE is we are on QueryBased Update mode
		BOOL	QueryBasedUpdate();

		//return BOOL Prop value
		BOOL	GetBoolPropValForRowset(DBPROPID eProp);

		//@mfunc: Return HRESULT from IsSameRows atfer it is check according to bocoo props
		HRESULT	IsSameRowAccoringToProps(
											HROW	*pHRowThis,
											HROW	*pHRowThat
										);

		//@mfunc: Get the Updatable columns that is not part of the key
		BOOL	GetNonKeyAndUpdatable(	DBORDINAL	*pcbCol,
										DBORDINAL	**prgColNum,
										IMalloc		*pIMalloc);

		//@mfunc: Compare the data from 2 row handles
		BOOL CheckRowsByData
		(
					HROW		hThisRow,
					HROW		hThatRow,
					ULONG		cSize,
					IRowset		*pIRowset,
					HACCESSOR	hAccessor
		);

		//@mfunc: release the accessor on the rowset
		void ReleaseAccessorOnRowset();

		//@mfunc: release a rowset object and accessor created on it
		void ReleaseRowsetAndAccessor();

	public:
		//constructor
		TCIRowsetIdentity(WCHAR * wstrTestCaseName)	//Takes TestCase Class name as parameter
						: CRowsetObject(wstrTestCaseName) 
		{
			m_pIRowsetIdentity	= NULL;
			m_pIRowsetChange	= NULL;
			m_pIRowsetUpdate	= NULL;
			m_pIRowsetLocate	= NULL;
			m_pStringsBuffer	= NULL;
			m_pIRowset			= NULL;
			m_pIDBSchemaRowset	= NULL;
			m_hAccessor			= NULL;
			m_cRowSize			= 0;
			m_cBinding			= 0;
			m_rgBinding			= NULL;
			m_rgInfo			= NULL;
			m_pData				= NULL;
			m_eAccessorLocation	= NO_ACCESSOR;
			m_ulUpdFlags		= DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_DELETE|DBPROPVAL_UP_INSERT;
		}
		//destructor
		virtual ~TCIRowsetIdentity(){};

		//check Properties
		BOOL IsPropertySet(GUID propset, DBPROPID propid, IUnknown * pIUnknown, LONG lValue);
};

//--------------------------------------------------------------------
//@mfunc: Init creates a Data Source object, a DB Session object, 
//and a command object and initialize corresponding interface pointers.
//
//--------------------------------------------------------------------
BOOL TCIRowsetIdentity::Init()
{
	if(!CRowsetObject::Init())
		return FALSE;

	//if the test is using an ini file and if this test is allowed to use commands 
	//and IRowsetChange (checked here from init string, not properties) and IRowsetChange is supported
	if( GetModInfo()->GetFileName() && g_fIRowsetChange && g_rgDBPrpt[IDX_IRowsetChange].fSupported)
	{
		//delete all rows in the table.
		if(!CHECK(g_pCTable->DeleteRows(ALLROWS),S_OK))
			return FALSE;

		// Regenerate the rowset
		if(!CHECK(g_pCTable->Insert(PRIMARY, 1, g_ulRowCount),S_OK))
			return FALSE;

	}
	else if(g_fIRowsetChange&&g_fCommands)
	{
		//drop the table and create an empty one
		if(!CHECK(g_pCTable->DropTable(),S_OK))
		{
			return FALSE;		
		}

		if(!SUCCEEDED(g_pCTable->CreateTable(g_ulRowCount,1,NULL,PRIMARY,TRUE)))
		{
			return FALSE;
		}
			
	}
	
	//Indicate NextRow in table
	g_ulNextRow = g_pCTable->GetRowsOnCTable() + 10;

	//set the DBSession.  No need to create a DB Session everytime
	SetDBSession(g_pIOpenRowset);

	//set the table.  No need to create a table everytime.
	SetTable(g_pCTable,DELETETABLE_NO);	

	return TRUE;
}



//--------------------------------------------------------------------
//@mfunc: Terminate release the data source object, DB Session object, Command object
//
//--------------------------------------------------------------------
BOOL TCIRowsetIdentity::Terminate()
{
	//Release the existing Session
	ReleaseDBSession();

	return (CRowsetObject::Terminate());
}

	
//--------------------------------------------------------------------
//@mfunc: Compare the row handles byte by byte
//--------------------------------------------------------------------
BOOL	CompareHandlesByLiteral
(
			HROW	hThisRow,
			HROW	hThatRow,
			BOOL	fNewlyInserted,
			BOOL	fExpected
)
{
	BOOL	fResult;

	//if DBPROP_LITERLIDENTITY is not supported or is VARIANT_FALSE.  
	//Return TEST_SKIPPED
	if	(!g_fLITERALIDENITIY)
	{
		return TRUE;
	}

	//if DBPROP_STRONGIDENITY is not supported or is VARIANT_FALSE AND the rows are newly
	//inserted, return TEST_SKIPPED
	if	((!g_fSTRONGIDENTITY) && fNewlyInserted)
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

//--------------------------------------------------------------------
//@mfunc: Compare the data from 2 row handles
//--------------------------------------------------------------------
BOOL CheckRowsByData
(
			HROW		hThisRow,
			HROW		hThatRow,
			ULONG		cSize,
			IRowset		*pIRowset,
			HACCESSOR	hAccessor
)
{
	BOOL	fResult	= FALSE;
	void	*pData1	= NULL;
	void	*pData2	= NULL;

	//alloc 2 buffers
	if	(	!(pData1=PROVIDER_ALLOC(cSize))	||
			!(pData2=PROVIDER_ALLOC(cSize))	
		)
	{
		goto END;
	}

	//fill the 2 buffers from the two different row handles
	if(!CHECK(pIRowset->GetData(hThisRow, hAccessor, pData1),S_OK))
	{
		goto END;
	}
	if(!CHECK(pIRowset->GetData(hThatRow, hAccessor, pData2),S_OK))
	{
		goto END;
	}
					
	//return TRUE if the data from the row handles are the same
	if(!memcmp(pData1, pData2, cSize))
	{
		fResult = TRUE;
	}
	else
	{
		fResult = FALSE;
	}
END:
	PROVIDER_FREE(pData1);
	PROVIDER_FREE(pData2);
	return fResult;
}

//--------------------------------------------------------------------
//@mfunc: Create a command object and set properties, execute a sql statement,
//		  and create a rowset object.  Create an accessor on the rowset 
//
//--------------------------------------------------------------------
BOOL	TCIRowsetIdentity::GetRowsetAndAccessor
(	
	EQUERY				eSQLStmt,				//the SQL Statement to create
	ULONG				cProperties,			//the count of properties
	const DBPROPID		*rgProperties,			//the array of properties to be set
	ULONG				cPropertiesUnset,		//the count of properties to be unset
	const DBPROPID		*rgPropertiesUnset,		//the array of properties to be unset	
	EACCESSORLOCATION	eAccessorLocation,		//where the accessor should be created
	DBACCESSORFLAGS		dwAccessorFlags,		//the accessor flags
	DBPART				dwPart,					//the type of binding
	ECOLS_BOUND			eColsToBind,			//the columns in accessor
	ECOLUMNORDER		eBindingOrder,			//the order to bind columns
	ECOLS_BY_REF		eColsByRef,				//which columns to bind by reference
	DBTYPE				dbTypeModifier,			//the type modifier used for accessor
	DBORDINAL			cColsToBind,			//the count of columns to bind
	ULONG_PTR			*rgColsToBind			//the array of column ordinals to bind
)
{
	IColumnsInfo		*pIColumnsInfo	= NULL;
	DBCOUNTITEM			cRowsObtained	= 0;
	HROW				*pHRow			= NULL;
	ULONG				cDBPropSet		= 1;
	DBPROPSET			rgDBPropSet[2];
	DBPROP				DBProp;
	ULONG				cDBPropCount	= 0;
	DBORDINAL			cCnt			= 0;
	ULONG				cCount			= 0;
	HRESULT				hr;
	BOOL				fPass			= FALSE;
	WORD				i				= 0;
	WORD				j				= 0;
	BOOL				fNotSupported	= FALSE;

	rgDBPropSet[0].rgProperties	= NULL;
	rgDBPropSet[1].rgProperties	= NULL;
	g_fTestValid = TRUE;

	//loop throuh all the requested properties for the rowset
	for (j=0;j<cProperties;j++)
	{
		//loop through all the property values for which the test has default values
		for (i=g_wePrptIdxFIRST;i<=g_wePrptIdxLAST;i++)
		{	
			if (rgProperties[j]==g_rgDBPrpt[i].dwPropID)
			{
				//if a property isn't suported by the provider
				if (!g_rgDBPrpt[i].fSupported)
				{
					odtLog<<L"Requested Property is NOT Supported and it is requested TRUE:SKIP\n";
				}
				else
				{
					//if a prop is read only (not writeble) AND the value it is setting is
					//different from the default and the value is not optional
					if	(	!(g_rgDBPrpt[i].dwFlags & DBPROPFLAGS_WRITE)	&&
							g_rgDBPrpt[i].fDefault == VARIANT_FALSE
						)
					{
						odtLog<<L"Requested Property is NOT Writable and test is trying to set it to non-default value:SKIP\n";
					}
				}
			}
		}
	}	
	//loop throuh all the requested properties for the rowset
	for (j=0;j<cPropertiesUnset;j++)
	{
		//loop through all the property values for which the test has default values
		for (i=g_wePrptIdxFIRST;i<=g_wePrptIdxLAST;i++)
		{	
			if (rgPropertiesUnset[j]==g_rgDBPrpt[i].dwPropID)
			{
				//if a property isn't suported by the provider
				if (!g_rgDBPrpt[i].fSupported)
				{
					odtLog<<L"Requested Prop NOT Supported but requested VARIANT_FALSE so behavior should be FALSE:WARNING\n";
				}
				else
				{
					//if a prop is read only (not writeble) AND the value it is setting is
					//different from the default and the value is not optional
					if	(	!(g_rgDBPrpt[i].dwFlags & DBPROPFLAGS_WRITE)	&&
							g_rgDBPrpt[i].fDefault == VARIANT_TRUE
						)	
					{
						odtLog<<L"Requested Property is NOT Writable and test is trying to set it to non-default value:SKIP\n";
					}
				}
			}
		}
	}	

	//loop through the rgProperties, look for KAGPROP_QUERYBASEDUPDATES
	for(cCnt=0; cCnt<cProperties; cCnt++)
	{
		if(rgProperties[cCnt]==KAGPROP_QUERYBASEDUPDATES)
			cDBPropSet=2;
		else
			cDBPropCount++;
	}

	//init
	if(cDBPropSet==2)
	{
		DBProp.dwPropertyID=KAGPROP_QUERYBASEDUPDATES;
		DBProp.dwOptions=DBPROPOPTIONS_REQUIRED;
		DBProp.vValue.vt=VT_BOOL;
		DBProp.colid=DB_NULLID;
		V_BOOL(&DBProp.vValue)=VARIANT_TRUE;

		rgDBPropSet[1].rgProperties=&DBProp;
		rgDBPropSet[1].cProperties=1;
		rgDBPropSet[1].guidPropertySet=DBPROPSET_PROVIDERROWSET;

		if(cDBPropCount!=(cProperties-1))
			goto END;
	}

	//check the count
	if(cDBPropSet==1)
	{
		if(cDBPropCount!=cProperties)
		{
			goto END;
		}
	}

	//init DBPropSet[0]
	rgDBPropSet[0].rgProperties		=NULL;
	rgDBPropSet[0].guidPropertySet	=DBPROPSET_ROWSET;
	rgDBPropSet[0].cProperties		=cDBPropCount+cPropertiesUnset;

	//Set up the DB Properties struct
	if(cProperties || cPropertiesUnset)
	{
		if(!g_fReadOnlyProvider && g_fROWSETUPDATABLE)
		{
			//need an extra for DBPROP_UPDATABILITY
			rgDBPropSet[0].cProperties++;
		}

		//allocate 
		rgDBPropSet[0].rgProperties=
			(DBPROP *)PROVIDER_ALLOC(sizeof(DBPROP)*(rgDBPropSet[0].cProperties));
		if (!rgDBPropSet[0].rgProperties)
		{
			goto END;
		}
		memset(rgDBPropSet[0].rgProperties,0xCA,(rgDBPropSet[0].cProperties)*sizeof(DBPROP));

		cCount=0;

		//go through the loop to set every DB Property required
		for(cCnt=0;cCnt<cProperties;cCnt++)
		{
			//skip QUERYBASEDUPDATED
			if(rgProperties[cCnt]==KAGPROP_QUERYBASEDUPDATES)
			{
				continue;
			}
			
			rgDBPropSet[0].rgProperties[cCount].dwPropertyID=rgProperties[cCnt];
			rgDBPropSet[0].rgProperties[cCount].dwOptions=DBPROPOPTIONS_REQUIRED;
			rgDBPropSet[0].rgProperties[cCount].vValue.vt=VT_BOOL;
			rgDBPropSet[0].rgProperties[cCount].colid=DB_NULLID;
			V_BOOL(&rgDBPropSet[0].rgProperties[cCount].vValue)=VARIANT_TRUE;

			cCount++;
		}

		if(!g_fReadOnlyProvider && g_fROWSETUPDATABLE)
		{
			//set DBPROP_UPDATABILITY
			rgDBPropSet[0].rgProperties[cCount].dwPropertyID=DBPROP_UPDATABILITY;
			rgDBPropSet[0].rgProperties[cCount].dwOptions=DBPROPOPTIONS_REQUIRED;
			rgDBPropSet[0].rgProperties[cCount].colid=DB_NULLID;
			if(SettableProperty(DBPROP_UPDATABILITY, DBPROPSET_ROWSET, g_pIDBCreateSession))
				rgDBPropSet[0].rgProperties[cCount].vValue.vt=VT_I4;
			else
				rgDBPropSet[0].rgProperties[cCount].vValue.vt=VT_EMPTY;
			rgDBPropSet[0].rgProperties[cCount].vValue.lVal=m_ulUpdFlags;

			cCount++;
		}

		//go through the loop to unset every DB Property required
		for(cCnt=0;cCnt<cPropertiesUnset;cCnt++)
		{
			rgDBPropSet[0].rgProperties[cCount].dwPropertyID=rgPropertiesUnset[cCnt];
			rgDBPropSet[0].rgProperties[cCount].dwOptions=DBPROPOPTIONS_REQUIRED;
			rgDBPropSet[0].rgProperties[cCount].colid=DB_NULLID;
			rgDBPropSet[0].rgProperties[cCount].vValue.vt=VT_BOOL;
			V_BOOL(&rgDBPropSet[0].rgProperties[cCount].vValue)=VARIANT_FALSE;

			cCount++;
		}
	}
	// Free the Column Ordinals
	g_cRowsetCols = 0;
	PROVIDER_FREE(g_rgTableColOrds);

	// call IOpenRowset to return a Rowset
	hr = m_pTable->CreateRowset	(
									USE_OPENROWSET,
									IID_IRowset,
									cDBPropSet,
									rgDBPropSet,
									(IUnknown**)&m_pIRowset,
									NULL,
									&g_cRowsetCols,
									&g_rgTableColOrds
								);

	if (!m_pIRowset)
	{
		goto END;
	}
	// Check HRESULT
	if( hr != S_OK )
	{		
		// Check for HRESULT returned by SetProperties
		if( hr == DB_S_ERRORSOCCURRED )
		{		
			odtLog<<L"Some non critical errors occured while setting Properties.\n";

			// make sure the correct information is returned
			for(i=0;i<rgDBPropSet[0].cProperties;i++)
			{
				if( rgDBPropSet->rgProperties[i].dwStatus != DBPROPSTATUS_OK )
				{
					odtLog<<L"Property "<<i+1<<" is not supported.\n";
					COMPARE(rgDBPropSet[0].rgProperties[i].dwStatus, DBPROPSTATUS_NOTSUPPORTED);
				}
			}
			goto END;
		}

		odtLog<<L"IOpenRowset::OpenRowset or ICommandProperties::SetProperties FAILED.\n";

		// Check to see if m_pIRowset is NULL
		if( m_pIRowset )
		{
			odtLog<<L"IRowset Pointer was not NULL.\n";
			goto END;
		}

		// Check HRESULT from ICommand::Execute
		if( hr == DB_E_ERRORSOCCURRED )
		{		
			// Make sure the correct information is returned
			for(i=0;i<rgDBPropSet[0].cProperties;i++)
			{
				if (DBPROPSTATUS_OK!=rgDBPropSet[0].rgProperties[i].dwStatus)
				{
					odtLog<<L"Setting Property "<<i+1<<" failed.\n";
				}
			}
			goto END;
		}
		// Check HRESULT
		if( hr == E_FAIL )
		{		
			odtLog<<L"ICommand::OpenRowset or ICommandProperties::SetProperties returned E_FAIL.\n";
			goto END;
		}

		// GOTO CLEANUP
		odtLog<<L"Unhandled ERROR.\n";
		goto END;
	}

	//set some globals for this rowset.
	//set it here becuase these will be determined by what properties the 
	//rowset supports
	g_fLITERALIDENITIY		= GetBoolPropValForRowset(DBPROP_LITERALIDENTITY);
	g_fREMOVEDELETED		= GetBoolPropValForRowset(DBPROP_REMOVEDELETED);
	g_fRETURNPENDINGINSERTS	= GetBoolPropValForRowset(DBPROP_RETURNPENDINGINSERTS);
	g_fOWNUPDATEDELETE		= GetBoolPropValForRowset(DBPROP_OWNUPDATEDELETE);
	g_fOWNINSERT			= GetBoolPropValForRowset(DBPROP_OWNINSERT);

	//get the IRowsetIdentity pointer
	if(!VerifyInterface(m_pIRowset, 
						IID_IRowsetIdentity,
						ROWSET_INTERFACE, 
						(IUnknown**)&m_pIRowsetIdentity))
	{
	}
	//get the IRowsetChange pointer
	if(!VerifyInterface(m_pIRowset, 
						IID_IRowsetChange,
						ROWSET_INTERFACE, 
						(IUnknown**)&m_pIRowsetChange))
	{
	}
	//get the IRowsetLocate pointer
	if(!VerifyInterface(m_pIRowset, 
						IID_IRowsetLocate,
						ROWSET_INTERFACE, 
						(IUnknown**)&m_pIRowsetLocate))
	{
	}
	//get the IRowsetUpdate pointer
	if(!VerifyInterface(m_pIRowset, 
						IID_IRowsetUpdate,
						ROWSET_INTERFACE, 
						(IUnknown**)&m_pIRowsetUpdate))
	{
	}
	//get the IRowset  pointer  
	if(!VerifyInterface(m_pIRowset, 
						IID_IAccessor,
						ROWSET_INTERFACE, 
						(IUnknown**)&m_pIAccessor))
	{
	}
	// get the columns infomation
	if(!VerifyInterface(m_pIRowset, 
						IID_IColumnsInfo,
						ROWSET_INTERFACE, 
						(IUnknown**)&pIColumnsInfo))
	{
	}

	if(!CHECK(pIColumnsInfo->GetColumnInfo(&m_cRowsetCols,&m_rgInfo, &m_pStringsBuffer),S_OK)) 
	{
		goto END;
	}
	//remember where the accessor handle is created
	m_eAccessorLocation=eAccessorLocation;

	//if eAccessorLocation is NO_ACCESSOR, no need to create an accessor
	if(eAccessorLocation==NO_ACCESSOR)
	{	
		fPass=TRUE;
		goto END;
	}
	
	switch(eAccessorLocation)
	{
		case ON_COMMAND_ACCESSOR:
				//can not create an accessor on the command object if the 
				//the rowset is a simple rowset.
				//the accessor is created before the rowset is open
				if(eSQLStmt==USE_OPENROWSET)
					goto END;
			break;
		case ON_ROWSET_FETCH_ACCESSOR:
				//can not create an accessor after the first fetch if no IRowset
				//is present on the rowset
				if(!m_pIRowset)
					goto END;
				if(!CHECK(m_pIRowset->GetNextRows(NULL,2,1,&cRowsObtained, &pHRow), S_OK))
					goto END;
		case ON_ROWSET_ACCESSOR:  
				//can not create an accessor on a rowset if no IRowset is present
				if(!m_pIRowset)
					goto END;

				//create an accessor on the rowset
				if(!CHECK(GetAccessorAndBindings(m_pIAccessor,dwAccessorFlags,&m_hAccessor,
				&m_rgBinding,&m_cBinding,&m_cRowSize,dwPart,eColsToBind,eBindingOrder,
				eColsByRef,NULL,&cCnt,NULL,dbTypeModifier,cColsToBind,(LONG_PTR *)rgColsToBind),S_OK))
					goto END;
			break;
		default:
			return FALSE;
	}

	//make sure cCnt should be the same as m_cRowsetCols
	if(!COMPARE(cCnt, m_cRowsetCols))
		goto END;

	//allocate memory for the row
	m_pData=PROVIDER_ALLOC(m_cRowSize);

	if(m_pData)
		fPass=TRUE;
END:
	if(rgDBPropSet[0].rgProperties)			   
		PROVIDER_FREE(rgDBPropSet[0].rgProperties);

	SAFE_RELEASE(pIColumnsInfo);

	if(pHRow)
	{
		CHECK(m_pIRowset->ReleaseRows(cRowsObtained,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	
		//restart position.  The rowset returns to its original state
		CHECK(m_pIRowset->RestartPosition(NULL),S_OK);
	}	
	return fPass;
}

//--------------------------------------------------------------------
//@mfunc: Create an accessor on the rowset 
//
//--------------------------------------------------------------------
BOOL	TCIRowsetIdentity::GetAccessorOnRowset
(	
	EACCESSORLOCATION	eAccessorLocation,		//where the accessor should be created
	DBACCESSORFLAGS		dwAccessorFlags,		//the accessor flags
	DBPART				dwPart,					//the type of binding
	ECOLS_BOUND			eColsToBind,			//the columns in accessor
	ECOLUMNORDER		eBindingOrder,			//the order to bind columns
	ECOLS_BY_REF		eColsByRef,				//which columns to bind by reference
	DBTYPE				dbTypeModifier,			//the type modifier used for accessor
	DBORDINAL			cColsToBind,			//the count of columns to bind
	ULONG_PTR			*rgColsToBind			//the array of column ordinals to bind
)
{
	IUnknown			*pIUnknown		= NULL;
	DBCOUNTITEM			cRowsObtained	= 0;
	HROW				*pHRow			= NULL;
	DBORDINAL			cCnt			= 0;
	BOOL				fPass			= FALSE;

	//remember where the accessor handle is created
	m_eAccessorLocation=eAccessorLocation;

	//eAccessorLocation can not be NO_ACCESSOR
	if(!COMPARE(eAccessorLocation!=NO_ACCESSOR,TRUE))
	{	
		fPass=FALSE;
		goto END;
	}
	
	switch(eAccessorLocation)
	{
		case ON_ROWSET_FETCH_ACCESSOR:
				//can not create an accessor after the first fetch if no IRowset
				//is present on the rowset
				if(!m_pIRowset)
					goto END;
				if(!CHECK(m_pIRowset->GetNextRows(NULL,2,1,&cRowsObtained, &pHRow), S_OK))
					goto END;
				pIUnknown=m_pIAccessor;
			break;
		case ON_ROWSET_ACCESSOR:  
				//can not create an accessor on a rowset if no IRowset is present
				if(!m_pIRowset)
					goto END;
				pIUnknown=m_pIAccessor;
			break;
		case ON_COMMAND_ACCESSOR:
		default:
			return FALSE;
	}

	//create an accessor on the rowset
	if(!CHECK(GetAccessorAndBindings(	pIUnknown,
										dwAccessorFlags,
										&m_hAccessor,
										&m_rgBinding,
										&m_cBinding,
										&m_cRowSize,
										dwPart,
										eColsToBind,
										eBindingOrder,
										eColsByRef,
										NULL,
										&cCnt,
										NULL,
										dbTypeModifier,
										cColsToBind,
										(LONG_PTR *) rgColsToBind),
										S_OK
									))
	{
		goto END;
	}
	//make sure cCnt should be the same as m_cRowsetCols
	if(!COMPARE(cCnt, m_cRowsetCols))
		goto END;

	//allocate memory for the row
	m_pData=PROVIDER_ALLOC(m_cRowSize);

	if(m_pData)
		fPass=TRUE;
END:
	if(pHRow)
	{
		CHECK(m_pIRowset->ReleaseRows(cRowsObtained,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	
		//restart position.  The rowset returns to its original state
		CHECK(m_pIRowset->RestartPosition(NULL),S_OK);
	}	

	return fPass;
}
 

//--------------------------------------------------------------------
//	Make sure GetData on the row handle is what to be expected
//--------------------------------------------------------------------	
BOOL	TCIRowsetIdentity::CheckExpectedData
(
			HROW	hRow,
			void	*pExpectedData, 
			BOOL	fSetData
)
{
	void	*pData	= NULL;
	BOOL	fResult	= FALSE;

	if(!(pData=PROVIDER_ALLOC(m_cRowSize)))
		goto END;

	//get data one the row handle
	if(!CHECK(m_pIRowset->GetData(hRow, m_hAccessor, pData),S_OK))
		goto END;

	//compare the buffer
	fResult=CompareBuffer(pData, pExpectedData, m_cBinding,m_rgBinding, m_pIMalloc, fSetData, FALSE,COMPARE_ONLY);
END:
	if(pData)
	   	ReleaseInputBindingsMemory(m_cBinding, m_rgBinding,(BYTE *)pData,TRUE);

	return fResult;
}

//--------------------------------------------------------------------
//	Insert a new row based on the row number.  Return the row handle
//  and hAccessor
//--------------------------------------------------------------------
HRESULT	TCIRowsetIdentity::InsertOneRow
(
			DBCOUNTITEM	cRowNumber, 
			HROW		*pHRow
)
{
	void	*pData	= NULL;
	HRESULT	hr		= S_OK;

	if(!pHRow)
		return E_FAIL;

	//get data to insert
	if(!CHECK(FillInputBindings(	m_pTable,
									DBACCESSOR_ROWDATA,
									m_cBinding,
									m_rgBinding,
									(BYTE **)&pData,
									cRowNumber,
									g_cRowsetCols,
									g_rgTableColOrds,
									PRIMARY),S_OK))
		return E_FAIL;

	//insert
	hr=m_pIRowsetChange->InsertRow(NULL,m_hAccessor,pData,pHRow);

	ReleaseInputBindingsMemory(m_cBinding, m_rgBinding,(BYTE *)pData,TRUE);

	return hr;
}

//--------------------------------------------------------------------
//	Insert a new row based on the row number.  Return the row handle
//  and hAccessor
//--------------------------------------------------------------------
HRESULT	TCIRowsetIdentity::ChangeOneRow
(
		HROW		hRow,
		DBCOUNTITEM	cRowNumber,
		void		**ppData
)
{
	void	*pData	= NULL;
	HRESULT	hr		= S_OK;

	//get data to insert
	if(!CHECK(FillInputBindings(m_pTable,
		DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
		(BYTE **)&pData,cRowNumber,g_cRowsetCols,g_rgTableColOrds,PRIMARY),S_OK))
		return E_FAIL;

	//change
	hr=m_pIRowsetChange->SetData(hRow,m_hAccessor,pData);
	if(ppData)
		*ppData=pData;
	else
		ReleaseInputBindingsMemory(m_cBinding, m_rgBinding,(BYTE *)pData,TRUE);

	return hr;
}
		
//--------------------------------------------------------------------
//	Delete the rows
//--------------------------------------------------------------------
HRESULT	TCIRowsetIdentity::DeleteRows
(
	DBCOUNTITEM	cRows,
	HROW		*pHRow
)
{
	if(!m_pIRowsetChange)
		return E_FAIL;

	return (m_pIRowsetChange->DeleteRows(NULL,cRows, pHRow,NULL));
}


//--------------------------------------------------------------------
// Update the rows
//--------------------------------------------------------------------
HRESULT	TCIRowsetIdentity::UpdateRows
(
	DBCOUNTITEM	cRows,
	HROW		*pHRow,
	DBROWSTATUS	**ppDBRowStatus
)
{	
	DBCOUNTITEM	cRowsUpdated	= 0;
	HROW		*pHRowUpdated	= NULL;
	HRESULT		hr				= E_FAIL;

	if(!m_pIRowsetUpdate)
		return E_FAIL;

	hr = m_pIRowsetUpdate->Update(NULL, cRows, pHRow,&cRowsUpdated, &pHRowUpdated, ppDBRowStatus);

	PROVIDER_FREE(pHRowUpdated);

	return hr;
}

		
//--------------------------------------------------------------------
// Undo the rows
//--------------------------------------------------------------------
HRESULT	TCIRowsetIdentity::UndoRows
(
			DBCOUNTITEM	cRows,
			HROW		*pHRow
)
{
	if(!m_pIRowsetUpdate)
		return E_FAIL;

	return (m_pIRowsetUpdate->Undo(NULL,cRows, pHRow,NULL,NULL,NULL));
}

//--------------------------------------------------------------------
// Release the row handles.  Free the memory if asked
//--------------------------------------------------------------------
HRESULT	TCIRowsetIdentity::ReleaseRows
(
		DBCOUNTITEM	cRows,
		HROW		**ppHRow,
		BOOL		fFreeMemory
)
{
	HRESULT	hr;

	hr=m_pIRowset->ReleaseRows(cRows, *ppHRow, NULL,NULL, NULL);

	if(fFreeMemory)
	{
		PROVIDER_FREE(*ppHRow);
		*ppHRow=NULL;
	}

	return hr;
}

//--------------------------------------------------------------------
//@mfun: Get the bookmark for the row.  The function has to be called
//		after the GetRowsetAndAccessor that creates an accessor on the 
//		rowset.  The accessor as to binds the 0th column on the rowset.
//
//--------------------------------------------------------------------
BOOL TCIRowsetIdentity::GetBookmark
(
	DBCOUNTITEM	ulRow,
	DBBKMARK	*pcbBookmark,
	BYTE		**ppBookmark
)
{
	BOOL		fPass		= FALSE;
	HROW		hRow[1];
	HROW		*pHRow		= hRow;
	DBCOUNTITEM	cCount		= 0;
	DBREFCOUNT	cRefCount	= 0;
	//the rowset has to expose IRowset in order to have bookmark
	if(!m_pIRowset)
		return FALSE;

	//ulRow has to start with 1
	if(!pcbBookmark || !ppBookmark || !ulRow)
		return FALSE;

	//restart the cursor position
	if(!CHECK(m_pIRowset->RestartPosition(NULL),S_OK))
		return FALSE;

	//fetch the row
	if(!CHECK(m_pIRowset->GetNextRows(NULL,(ulRow-(ULONG)1),1,&cCount,&pHRow),S_OK))
		return FALSE;

	//only one row handle is retrieved
	COMPARE(cCount, 1);

	//get the bookmark by the row handle
	fPass=GetBookmarkByRow(*pHRow, pcbBookmark, ppBookmark);

	//release the row handle
	CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,&cRefCount),S_OK);

	COMPARE(cRefCount, 0);

	return fPass;
}

//--------------------------------------------------------------------
//@mfun: Get the bookmark for the row.  The function has to be called
//		after the GetRowsetAndAccessor that creates an accessor on the 
//		rowset.  The accessor as to binds the 0th column on the rowset.
//
//--------------------------------------------------------------------
BOOL TCIRowsetIdentity::GetBookmarkByRow
(
		HROW		hRow,
		DBBKMARK	*pcbBookmark,
		BYTE		**ppBookmark
)
{
	BOOL		fPass		= FALSE;
	void		*pData		= NULL;
	HACCESSOR	hAccessor	= NULL;
	DBCOUNTITEM	cBinding	= 0;
	DBLENGTH	cbRowSize	= 0;
	DBBINDING	*pBinding	= NULL;
	LONG_PTR	lColToBind	= 0;	

	//the rowset has to expose IRowset in order to have bookmark
	if(!m_pIRowset)
		return FALSE;

	//check the input
	if(!pcbBookmark || !ppBookmark)
		return FALSE;

	//create an accessor to binding the bookmark
	if(!CHECK(GetAccessorAndBindings(m_pIAccessor, 
	DBACCESSOR_ROWDATA, &hAccessor, &pBinding,
	&cBinding,&cbRowSize,DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
	USE_COLS_TO_BIND_ARRAY, FORWARD, NO_COLS_BY_REF, NULL, NULL, NULL, 
	DBTYPE_EMPTY, 1, (LONG_PTR *)&lColToBind),S_OK))
		goto END;

	//allocate memory 
	if(!(pData=PROVIDER_ALLOC(cbRowSize)))
		goto END;

	//get the data
	if(!CHECK(m_pIRowset->GetData(hRow, hAccessor, pData),S_OK))
		goto END;

	//get the length of the bookmark
//	*pcbBookmark= *((ULONG *)(dwAddr+pBinding[0].obLength));
	*pcbBookmark=LENGTH_BINDING(pBinding[0], pData);

	//allocate memory for bookmark
	*ppBookmark=(BYTE *)PROVIDER_ALLOC(*pcbBookmark);

	if(!(*ppBookmark))
		goto END;

	//copy the value of the bookmark into the consumer's buffer
//	memcpy(*ppBookmark, (void *)(dwAddr+pBinding[0].obValue), *pcbBookmark);
	memcpy(*ppBookmark, &VALUE_BINDING(pBinding[0], pData), (size_t)*pcbBookmark);

	fPass=TRUE;
END:
	//release the memory
	if(pData)
		PROVIDER_FREE(pData);

	if(pBinding)
		PROVIDER_FREE(pBinding);

	//free the accessor
		CHECK(m_pIAccessor->ReleaseAccessor(hAccessor,NULL),S_OK);

	return fPass;
}


//--------------------------------------------------------------------
//
//@mfunc: Get cursor type of the rowset.  Has to be called after a rowset 
//			generated.
//
//
//--------------------------------------------------------------------
ECURSOR	TCIRowsetIdentity::GetCursorType()
{
	IRowsetInfo			*pIRowsetInfo	= NULL;
	ULONG				cProperty		= 0;
	DBPROPID			rgGuid[4];
	DBPROPIDSET			DBPropIDSet;
	DBPROPSET			*pDBPropSet		= NULL;
	ECURSOR				eCursor			= FORWARD_ONLY_CURSOR;

	//initialization
	rgGuid[0]=DBPROP_OTHERINSERT;
	rgGuid[1]=DBPROP_OTHERUPDATEDELETE;
	rgGuid[2]=DBPROP_CANSCROLLBACKWARDS;
	rgGuid[3]=DBPROP_CANFETCHBACKWARDS;

	DBPropIDSet.guidPropertySet=DBPROPSET_ROWSET;
	DBPropIDSet.cPropertyIDs=4;
	DBPropIDSet.rgPropertyIDs=rgGuid;

	//QI for IRowsetInfo interface
	if(!VerifyInterface(m_pIRowset, 
						IID_IRowsetInfo,
						ROWSET_INTERFACE, 
						(IUnknown**)&pIRowsetInfo))
	{
		goto END;
	}

	//ask for the 3 properties
	pIRowsetInfo->GetProperties(1,&DBPropIDSet,&cProperty,&pDBPropSet);

	//only one property set should be returned
	if(!COMPARE(cProperty, 1))
		goto END;

	if(V_BOOL(&pDBPropSet[0].rgProperties[0].vValue)==VARIANT_TRUE)
		eCursor=DYNAMIC_CURSOR;
	else
	{
		if(V_BOOL(&pDBPropSet[0].rgProperties[1].vValue)==VARIANT_TRUE)
			eCursor=KEYSET_DRIVEN_CURSOR;
		else
		{
			if(	(V_BOOL(&pDBPropSet[0].rgProperties[2].vValue)==VARIANT_TRUE)	||
				(V_BOOL(&pDBPropSet[0].rgProperties[3].vValue)==VARIANT_TRUE)
			)
			{
				eCursor=STATIC_CURSOR;
			}
		}
	}
END:
	FreeProperties(&cProperty,&pDBPropSet);
	SAFE_RELEASE(pIRowsetInfo);

	return eCursor;
}

//--------------------------------------------------------------------
//
//@mfunc: Return TRUE is we are on QueryBased Update mode.  Has to
//			to called after a rowset is created.
//
////--------------------------------------------------------------------
BOOL	TCIRowsetIdentity::QueryBasedUpdate()
{
	IRowsetInfo			*pIRowsetInfo	= NULL;
	ULONG				cProperty		= 0;
	DBPROPID			DBPropID		= KAGPROP_QUERYBASEDUPDATES;
	DBPROPIDSET			DBPropIDSet;
	DBPROPSET			*pDBPropSet		= NULL;
	BOOL				fSupported		= FALSE;
	HRESULT				hr;

	//initialize
	DBPropIDSet.guidPropertySet=DBPROPSET_PROVIDERROWSET;
	DBPropIDSet.cPropertyIDs=1;
	DBPropIDSet.rgPropertyIDs=&DBPropID;

	//QI for IRowsetInfo interface
	if(!VerifyInterface(m_pIRowset, 
						IID_IRowsetInfo,
						ROWSET_INTERFACE, 
						(IUnknown**)&pIRowsetInfo))
	{
	}

	//ask for KAGPROP_QUERYBASEDUPDATES
	hr = pIRowsetInfo->GetProperties(1,&DBPropIDSet,&cProperty,&pDBPropSet);

	if(V_BOOL(&pDBPropSet->rgProperties->vValue)==VARIANT_TRUE&&hr==S_OK)
		fSupported=TRUE;
	else
		fSupported=FALSE;

	FreeProperties(&cProperty,&pDBPropSet);
	SAFE_RELEASE(pIRowsetInfo);

	return fSupported;
}


//--------------------------------------------------------------------
//
//@mfunc: Get BOOL value for Property
//
////--------------------------------------------------------------------
BOOL	TCIRowsetIdentity::GetBoolPropValForRowset(DBPROPID eProp)
{
	IRowsetInfo			*pIRowsetInfo	= NULL;
	ULONG				cProperty		= 0;
	DBPROPID			DBPropID		= eProp;
	DBPROPIDSET			DBPropIDSet;
	DBPROPSET			*pDBPropSet		= NULL;
	BOOL				fSupported		= FALSE;
	HRESULT				hr				= S_OK;

	//initialize
	DBPropIDSet.guidPropertySet=DBPROPSET_ROWSET;
	DBPropIDSet.cPropertyIDs=1;
	DBPropIDSet.rgPropertyIDs=&DBPropID;

	//QI for IRowsetInfo interface
	if(!VerifyInterface(m_pIRowset, 
						IID_IRowsetInfo,
						ROWSET_INTERFACE, 
						(IUnknown**)&pIRowsetInfo))
	{
	}

	//ask for in Prop
	hr = pIRowsetInfo->GetProperties(1,&DBPropIDSet,&cProperty,&pDBPropSet);

	if(V_BOOL(&pDBPropSet->rgProperties->vValue)==VARIANT_TRUE&&hr==S_OK)
		fSupported=TRUE;
	else
		fSupported=FALSE;

	FreeProperties(&cProperty,&pDBPropSet);
	SAFE_RELEASE(pIRowsetInfo);

	return fSupported;
}

//--------------------------------------------------------------------
//
//@mfunc: Get non-key and updatable column	ordinals arrays. 
//
//		  The function allocation memory for the ordinals array.  Return
//		  TRUE is one or more non-key and updatable column exists;
//		  FALSE otherwise.
//
//
//--------------------------------------------------------------------
BOOL	TCIRowsetIdentity::GetNonKeyAndUpdatable(
				DBORDINAL	*pcbCol,			//[out] the count of the rgColNum
				DBORDINAL	**prgColNum,		//[out] the col ordinals array
				IMalloc		*pIMalloc			)
{
	DBORDINAL	cColsCount;

	//make sure the columns infomation has been retrieved and
	//there is at least one column in the rowset
	if(!m_rgInfo || !m_cRowsetCols)
		return FALSE;

	//initialization
	*pcbCol=0;
	*prgColNum=(DBORDINAL *)pIMalloc->Alloc(sizeof(DBORDINAL) * m_cRowsetCols);

	for(cColsCount=0;cColsCount<m_cRowsetCols;cColsCount++)
	{
		//skip the 1st column
		if(m_rgInfo[cColsCount].iOrdinal==1)
			continue;

		if(m_rgInfo[cColsCount].dwFlags & DBCOLUMNFLAGS_WRITE ||
			m_rgInfo[cColsCount].dwFlags & DBCOLUMNFLAGS_WRITEUNKNOWN	)
		{
			//copy the column number into the array
			(*prgColNum)[*pcbCol]=m_rgInfo[cColsCount].iOrdinal;

			//increase the count
			(*pcbCol)++;
		}
	}
	
	if(*pcbCol)
		return TRUE;

	return FALSE;
}

//--------------------------------------------------------------------
//@mfunc: release an accessor created on the rowset object
//
//--------------------------------------------------------------------
void TCIRowsetIdentity::ReleaseAccessorOnRowset()
{
	IAccessor *pIAccessor;

	//reset m_cRowset to 0 so that provider will allocate memory for next time
	m_cRowSize=0;
	m_cBinding=0;

	//free the consumer buffer
	if(m_pData)
	{
		PROVIDER_FREE(m_pData);
		m_pData=NULL;
	}

	//free accessor handle
	if(m_hAccessor)
	{
		//if the accessor is created on the rowset object, use the IAccssor
		//pointer directly to release the accessor handle
		if(m_eAccessorLocation!=ON_COMMAND_ACCESSOR)
			CHECK(m_pIAccessor->ReleaseAccessor(m_hAccessor,NULL), S_OK);
		else
		{
			//QI for the accessor handle on the command object
			if(!VerifyInterface(m_pIRowset, 
								IID_IAccessor,
								ROWSET_INTERFACE, 
								(IUnknown**)&pIAccessor))
			{
				CHECK(pIAccessor->ReleaseAccessor(m_hAccessor,NULL), S_OK);
				SAFE_RELEASE(pIAccessor);
			}
		}

		m_hAccessor=NULL;
	}

	//free binding structure
	if(m_rgBinding)
	{
		PROVIDER_FREE(m_rgBinding);
		m_rgBinding=NULL;
	}
}


//--------------------------------------------------------------------
//@mfunc: release a rowset object and accessor created on it
//
//--------------------------------------------------------------------
void TCIRowsetIdentity::ReleaseRowsetAndAccessor()
{
	IAccessor *pIAccessor;

	//reset m_cRowset to 0 so that provider will allocate memory for next time
	m_cRowSize=0;
	m_cBinding=0;

	//free the consumer buffer
	if(m_pData)
	{
		PROVIDER_FREE(m_pData);
		m_pData=NULL;
	}

	//free accessor handle
	if(m_hAccessor)
	{
		//if the accessor is created on the rowset object, use the IAccssor
		//pointer directly to release the accessor handle
		if(m_eAccessorLocation!=ON_COMMAND_ACCESSOR)
			CHECK(m_pIAccessor->ReleaseAccessor(m_hAccessor,NULL), S_OK);
		else
		{
			//QI for the accessor handle on the command object
			if(!VerifyInterface(m_pIRowset, 
								IID_IAccessor,
								ROWSET_INTERFACE, 
								(IUnknown**)&pIAccessor))
			{
				CHECK(pIAccessor->ReleaseAccessor(m_hAccessor,NULL), S_OK);
				SAFE_RELEASE(pIAccessor);
			}
		}

		m_hAccessor=NULL;
	}
	
	//release Rowset pointers
	SAFE_RELEASE(m_pIRowset);
	SAFE_RELEASE(m_pIRowsetIdentity);
	SAFE_RELEASE(m_pIRowsetChange);
	SAFE_RELEASE(m_pIRowsetUpdate);
	SAFE_RELEASE(m_pIRowsetLocate);

	//free binding structure
	PROVIDER_FREE(m_rgBinding);
	PROVIDER_FREE(m_rgInfo);
	PROVIDER_FREE(m_pStringsBuffer);

	ReleaseRowsetObject(0);
	ReleaseCommandObject(0);
}


//--------------------------------------------------------------------
// @mfunc base class IsPropertySet
//
BOOL TCIRowsetIdentity::IsPropertySet(GUID propset, DBPROPID propid, IUnknown * pIUnknown, LONG lValue)
{
	HRESULT				hr					= E_FAIL;
	BOOL				fSucceed			= FALSE;
	ICommandProperties *pICmdProperties		= NULL;
	IDBProperties *		pIDBProperties		= NULL;
	IRowsetInfo *		pIRowsetInfo		= NULL;
	DBPROPIDSET			rgPropertyIDSet[1];
	DBPROPID *			rgPropertyIDs;
	ULONG				cPropertySets		= 0;
	DBPROPSET *			rgPropertySets		= NULL;

	// Init for Properties
	rgPropertyIDs = &propid;
	rgPropertyIDSet->guidPropertySet = propset;

	rgPropertyIDSet->cPropertyIDs = 1;
	rgPropertyIDSet->rgPropertyIDs = rgPropertyIDs;

	// Check to set what DBPROPSET is passed in
	if( !(memcmp(&propset, &(DBPROPSET_DATASOURCEINFO), sizeof(GUID))) )
	{
		// Make sure the DSO object is valid
		if( !g_pIDBCreateSession )
			goto CLEANUP;

		// Get a IDBProperties pointer
		if(!VerifyInterface(g_pIDBCreateSession, 
							IID_IDBProperties,
							DATASOURCE_INTERFACE, 
							(IUnknown**)&pIDBProperties))
		{
		}

		// GetProperties
		hr=pIDBProperties->GetProperties(1, rgPropertyIDSet,
										  &cPropertySets, &rgPropertySets);
	}
	else if( (!(memcmp(&propset, &(DBPROPSET_ROWSET), sizeof(GUID)))) && (!pIUnknown) )
	{
		// Make sure the Command object is valid
		if( !g_pCTable->m_pICommand )
			goto CLEANUP;

		// Get a ICommandProperties pointer
		if(!VerifyInterface(g_pCTable->m_pICommand, 
							IID_ICommandProperties,
							COMMAND_INTERFACE, 
							(IUnknown**)&pICmdProperties))
		{
			goto CLEANUP;
		}
		// GetProperties
		hr=pICmdProperties->GetProperties(1, rgPropertyIDSet,
										  &cPropertySets, &rgPropertySets);
	}
	else if( (!(memcmp(&propset, &(DBPROPSET_ROWSET), sizeof(GUID)))) && (pIUnknown) )
	{
		// Get a IRosetInfo pointer
		if(!VerifyInterface(pIUnknown, 
							IID_IRowsetInfo,
							ROWSET_INTERFACE, 
							(IUnknown**)&pIRowsetInfo))
		{
		}
		// GetProperties
		hr=pIRowsetInfo->GetProperties(1, rgPropertyIDSet,
									   &cPropertySets, &rgPropertySets);
	}
	else
		ASSERT(!("Please add DBPROPSET to if statement"));

	// Check to see if the Property is Supported and set to TRUE
	if( rgPropertySets[0].rgProperties->vValue.vt == VT_BOOL )
	{
		if( (hr == S_OK) && (rgPropertySets[0].rgProperties->dwStatus == DBPROPSTATUS_OK) &&
			((V_BOOL(&rgPropertySets[0].rgProperties->vValue)) == VARIANT_TRUE) )
			fSucceed = TRUE;
	}
	else
		if( (hr == S_OK) && (rgPropertySets[0].rgProperties->dwStatus == DBPROPSTATUS_OK) &&
			((V_I4(&rgPropertySets[0].rgProperties->vValue)) == lValue) )
			fSucceed = TRUE;

	PRVTRACE(L"[Propid=%d],[dwOptions=%d],[dwStatus=%d]\n",
			rgPropertySets[0].rgProperties->dwPropertyID,
			rgPropertySets[0].rgProperties->dwOptions,
			rgPropertySets[0].rgProperties->dwStatus);
CLEANUP:

	// Release the Properties pointer	
	SAFE_RELEASE(pIRowsetInfo);
	SAFE_RELEASE(pICmdProperties);
	SAFE_RELEASE(pIDBProperties);

	// Clear the Property memory
	FreeProperties(&cPropertySets, &rgPropertySets);

	return fSucceed;
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Test Case Section
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


// {{ TCW_TEST_CASE_MAP(Boundary)
//--------------------------------------------------------------------
// @class Test boundary conditons
//
class Boundary : public TCIRowsetIdentity {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Boundary,TCIRowsetIdentity);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember hThisRow is released
	int Variation_1();
	// @cmember hThatRow is released
	int Variation_2();
	// @cmember hThisRow is hard-deleted
	int Variation_3();
	// @cmember hThatRow is hard-deleted
	int Variation_4();
	// @cmember Compare with same row handle.  S_OK.
	int Variation_5();
	// @cmember hThisRow is DB_INVALID_HROW
	int Variation_6();
	// @cmember hThatRow is DB_INVALID_HROW
	int Variation_7();
	// @cmember hRow is NULL
	int Variation_8();
	// }}
};
// {{ TCW_TESTCASE(Boundary)
#define THE_CLASS Boundary
BEG_TEST_CASE(Boundary, TCIRowsetIdentity, L"Test boundary conditons")
	TEST_VARIATION(1,		L"hThisRow is released")
	TEST_VARIATION(2,		L"hThatRow is released")
	TEST_VARIATION(3,		L"hThisRow is hard-deleted")
	TEST_VARIATION(4,		L"hThatRow is hard-deleted")
	TEST_VARIATION(5,		L"Compare with same row handle.  S_OK.")
	TEST_VARIATION(6,		L"hThisRow is DB_INVALID_HROW")
	TEST_VARIATION(7,		L"hThatRow is DB_INVALID_HROW")
	TEST_VARIATION(8,		L"hRow is NULL")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}


// {{ TCW_TEST_CASE_MAP(Transaction)
//--------------------------------------------------------------------
// @class test zomibe and fully functional state after a trasaction
//
class Transaction : public CTransaction {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

	DBPROPSET	m_DBPropSet;
	BOOL		m_fTrans;

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Transaction,CTransaction);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Commit with Retaining.  Test on two identical hrows.  S_OK or E_UNEXPECTED
	int Variation_1();
	// @cmember Commit without Retaining.  Test on two different hrows.  S_FALSE or E_UNEXPECTED.
	int Variation_2();
	// @cmember Abort with Retaining.  Test on one hard deleted row handle and one row handle.  DB_E_DELETEDROW or E_UNEXPECTED.
	int Variation_3();
	// @cmember Abort without Retaining.  Test on one changed row handle and one row handle 	that refers to the same row.   S_OK or E_UNEXPECTE
	int Variation_4();
	// }}
};
// {{ TCW_TESTCASE(Transaction)
#define THE_CLASS Transaction
BEG_TEST_CASE(Transaction, CTransaction, L"test zomibe and fully functional state after a trasaction")
	TEST_VARIATION(1,		L"Commit with Retaining.  Test on two identical hrows.  S_OK or E_UNEXPECTED")
	TEST_VARIATION(2,		L"Commit without Retaining.  Test on two different hrows.  S_FALSE or E_UNEXPECTED.")
	TEST_VARIATION(3,		L"Abort with Retaining.  Test on one hard deleted row handle and one row handle.  DB_E_DELETEDROW or E_UNEXPECTED.")
	TEST_VARIATION(4,		L"Abort without Retaining.  Test on one changed row handle and one row handle 	that refers to the same row.   S_OK or E_UNEXPECTE")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}


// {{ TCW_TEST_CASE_MAP(SchemaRowset)
//--------------------------------------------------------------------
// @class SchemaRowset
//
class SchemaRowset : public TCIRowsetIdentity {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(SchemaRowset,TCIRowsetIdentity);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember SchemaRowset
	int Variation_1();
	// }}
};
// {{ TCW_TESTCASE(SchemaRowset)
#define THE_CLASS SchemaRowset
BEG_TEST_CASE(SchemaRowset, TCIRowsetIdentity, L"SchemaRowset")
	TEST_VARIATION(1,		L"SchemaRowset")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}


// {{ TCW_TEST_CASE_MAP(Forward_Only_Cursor)
//--------------------------------------------------------------------
// @class Forward_Only_Cursor
//
class Forward_Only_Cursor : public TCIRowsetIdentity {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Forward_Only_Cursor,TCIRowsetIdentity);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Fetch two different rows.  S_FALSE.
	int Variation_1();
	// @cmember Multiple rows fetched 2 different times.
	int Variation_2();
	// @cmember REMOVEDELETED FALSE
	int Variation_3();
	// @cmember REMOVEDELETED TRUE
	int Variation_4();
	// @cmember QI without DBPROP_IRowsetIdentity requested
	int Variation_5();
	// }}
};
// {{ TCW_TESTCASE(Forward_Only_Cursor)
#define THE_CLASS Forward_Only_Cursor
BEG_TEST_CASE(Forward_Only_Cursor, TCIRowsetIdentity, L"Forward_Only_Cursor")
	TEST_VARIATION(1,		L"Fetch two different rows.  S_FALSE.")
	TEST_VARIATION(2,		L"Multiple rows fetched 2 different times.")
	TEST_VARIATION(3,		L"REMOVEDELETED FALSE.")
	TEST_VARIATION(4,		L"REMOVEDELETED TRUE.")
	TEST_VARIATION(5,		L"QI without DBPROP_IRowsetIdentity requested.")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}


// {{ TCW_TEST_CASE_MAP(Static_Query_Immediate)
//--------------------------------------------------------------------
// @class Static_Query_Immediate
//
class Static_Query_Immediate : public TCIRowsetIdentity {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Static_Query_Immediate,TCIRowsetIdentity);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember  Fetch one row.  Move the cursor and fetch backwards to retrieve the same row handle.  S_OK.  
	int Variation_1();
	// @cmember Hard deleted the 1st row handle.  Fetch the same row again..
	int Variation_2();
	// }}
};
// {{ TCW_TESTCASE(Static_Query_Immediate)
#define THE_CLASS Static_Query_Immediate
BEG_TEST_CASE(Static_Query_Immediate, TCIRowsetIdentity, L"Static_Query_Immediate")
	TEST_VARIATION(1,		L"Fetch one row.  Move the cursor and fetch backwards to retrieve the same row handle.  S_OK.  ")
	TEST_VARIATION(2,		L"Hard deleted the 1st row handle.  Fetch the same row again.")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}


// {{ TCW_TEST_CASE_MAP(Static_Cursor_Buffered)
//--------------------------------------------------------------------
// @class Static_Cursor_Buffered
//
class Static_Cursor_Buffered : public TCIRowsetIdentity {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Static_Cursor_Buffered,TCIRowsetIdentity);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Soft deleted the 1st row handle.  Fetch the same row again by IRowsetLocate::GetRowsAt.  S_OK.
	int Variation_1();
	// @cmember Fetch hRowThis and soft change the row.  The row should stay in place in the rowset.  Fetch the row handle again in hRowThat
	int Variation_2();
	// @cmember Insert a new row.  Compare it with a new row handle.
	int Variation_3();
	// }}
};
// {{ TCW_TESTCASE(Static_Cursor_Buffered)
#define THE_CLASS Static_Cursor_Buffered
BEG_TEST_CASE(Static_Cursor_Buffered, TCIRowsetIdentity, L"Static_Cursor_Buffered")
	TEST_VARIATION(1,		L"Soft deleted the 1st row handle.  Fetch the same row again")
	TEST_VARIATION(2,		L"Fetch hRowThis and soft change the row.  The row should stay in place in the rowset.  Fetch the row handle again in hRowThat")
	TEST_VARIATION(3,		L"Insert a new row.  Compare it with a new row handle.")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}


// {{ TCW_TEST_CASE_MAP(Keyset_Query_Immediate)
//--------------------------------------------------------------------
// @class Keyset_Query_Immediate
//
class Keyset_Query_Immediate : public TCIRowsetIdentity {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Keyset_Query_Immediate,TCIRowsetIdentity);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember  Fetch one row.  Move the cursor and fetch backwards to retrieve the same row handle.  S_OK.  
	int Variation_1();
	// @cmember Fetch hRowThis and change a non-key value.  Fetch the same row in hRowThat by IRowsetLocate::GetRowsByBookmark.  S_OK.
	int Variation_2();
	// @cmember Hard deleted the 1st row handle.  Fetch the same row again forwards.
	int Variation_3();
	// @cmember Hard deleted the 1st row handle.  Fetch the same row again backwards.
	int Variation_4();
	// @cmember Hard deleted the last row handle.  Fetch the same row again backwards.
	int Variation_5();
	// @cmember Hard deleted the last row handle.  Fetch the same row again forwards.
	int Variation_6();
	// }}
};
// {{ TCW_TESTCASE(Keyset_Query_Immediate)
#define THE_CLASS Keyset_Query_Immediate
BEG_TEST_CASE(Keyset_Query_Immediate, TCIRowsetIdentity, L"Keyset_Query_Immediate")
	TEST_VARIATION(1,		L"Fetch one row.  Move the cursor and fetch backwards to retrieve the same row handle.  S_OK.  ")
	TEST_VARIATION(2,		L"Fetch hRowThis and change a non-key value.  Fetch the same row in hRowThat by IRowsetLocate::GetRowsByBookmark.  S_OK.")
	TEST_VARIATION(3,		L"Hard deleted the 1st row handle. Fetch the same row again forwards.")
	TEST_VARIATION(4,		L"Hard deleted the 1st row handle. Fetch the same row again backwards.")
	TEST_VARIATION(5,		L"Hard deleted the last row handle.  Fetch the same row again backwards.")
	TEST_VARIATION(6,		L"Hard deleted the last row handle.  Fetch the same row again forwards.")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}


// {{ TCW_TEST_CASE_MAP(Keyset_Cursor_Buffered)
//--------------------------------------------------------------------
// @class Keyset_Cursor_Buffered
//
class Keyset_Cursor_Buffered : public TCIRowsetIdentity {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Keyset_Cursor_Buffered,TCIRowsetIdentity);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Fetch one row.  Move the cursor and fetch backwards to retrieve the same row handle by IRowsetLocate::GetRowsByBookmark.  S_OK.
	int Variation_1();
	// @cmember Soft deleted the 1st row handle.   Use Bookmarks.
	int Variation_2();
	// @cmember Fetch hRowThis and change a non-key value.  Fetch the same row in hRowThat.  IsSameRow should return S_OK.  
	int Variation_3();
	// @cmember Insert a new row into the rowset - Undo.
	int Variation_4();
	// @cmember Fetch hRowThis and soft change the key of the row.  The row should be deleted and a new row is appended at the end of rowset.
	int Variation_5();
	// @cmember Insert a new row into the rowset - Update.  
	int Variation_6();
	// @cmember Soft deleted the 1st row handle.
	int Variation_7();
	// }}
};
// {{ TCW_TESTCASE(Keyset_Cursor_Buffered)
#define THE_CLASS Keyset_Cursor_Buffered
BEG_TEST_CASE(Keyset_Cursor_Buffered, TCIRowsetIdentity, L"Keyset_Cursor_Buffered")
	TEST_VARIATION(1,		L"Fetch one row.  Move the cursor and fetch backwards to retrieve the same row handle by IRowsetLocate::GetRowsByBookmark.  S_OK.")
	TEST_VARIATION(2,		L"Soft deleted the 1st row handle.  Use Bookmarks.")
	TEST_VARIATION(3,		L"Fetch hRowThis and change a non-key value.  Fetch the same row in hRowThat.  IsSameRow should return S_OK.  ")
	TEST_VARIATION(4,		L"Insert a new row into the rowset  - Undo.")
	TEST_VARIATION(5,		L"Fetch hRowThis and soft change the key of the row.  The row should be deleted and a new row is appended at the end of rowset.")
	TEST_VARIATION(6,		L"Insert a new row into the rowset - Update.")
	TEST_VARIATION(7,		L"Soft deleted the 1st row handle")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}


// {{ TCW_TEST_CASE_MAP(Dynamic_Query_Buffered)
//--------------------------------------------------------------------
// @class Dynamic_Query_Buffered
//
class Dynamic_Query_Buffered : public TCIRowsetIdentity {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Dynamic_Query_Buffered,TCIRowsetIdentity);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Soft-delete one row.  S_FALSE.  Call IRowsetUpdate::Update.  DB_E_DELETEDROW.
	int Variation_1();
	// }}
};
// {{ TCW_TESTCASE(Dynamic_Query_Buffered)
#define THE_CLASS Dynamic_Query_Buffered
BEG_TEST_CASE(Dynamic_Query_Buffered, TCIRowsetIdentity, L"Dynamic_Query_Buffered")
	TEST_VARIATION(1,		L"Soft-delete one row.  S_FALSE.  Call IRowsetUpdate::Update.  DB_E_DELETEDROW.")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}

// {{ TCW_TEST_CASE_MAP(Dynamic_Cursor_Immediate)
//--------------------------------------------------------------------
// @class Dynamic_Cursor_Immediate
//
class Dynamic_Cursor_Immediate : public TCIRowsetIdentity {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(Dynamic_Cursor_Immediate,TCIRowsetIdentity);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Hard-delete one row.  S_FALSE.
	int Variation_1();
	// }}
};
// {{ TCW_TESTCASE(Dynamic_Cursor_Immediate)
#define THE_CLASS Dynamic_Cursor_Immediate
BEG_TEST_CASE(Dynamic_Cursor_Immediate, TCIRowsetIdentity, L"Dynamic_Cursor_Immediate")
	TEST_VARIATION(1,		L"Hard-delete one row.  S_FALSE..")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}

// {{ TCW_TEST_CASE_MAP(ExtendedErrors)
//--------------------------------------------------------------------
// @class Extended Errors
//
class ExtendedErrors : public TCIRowsetIdentity {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(ExtendedErrors,TCIRowsetIdentity);
	// }}
 

	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Valid IRowsetIdentity calls with previous error object existing.
	int Variation_1();
	// @cmember Invalid IRowsetIdentity calls with previous error object existing
	int Variation_2();
	// @cmember Invalid IRowsetIdentity calls with no previous error object existing
	int Variation_3();
	// }}
};
// {{ TCW_TESTCASE(ExtendedErrors)
#define THE_CLASS ExtendedErrors
BEG_TEST_CASE(ExtendedErrors, TCIRowsetIdentity, L"Extended Errors")
	TEST_VARIATION(1,		L"Valid IRowsetIdentity calls with previous error object existing.")
	TEST_VARIATION(2,		L"Invalid IRowsetIdentity calls with previous error object existing")
	TEST_VARIATION(3,		L"Invalid IRowsetIdentity calls with no previous error object existing")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}

// }} END_DECLARE_TEST_CASES()

// {{ TCW_TESTMODULE(ThisModule)
TEST_MODULE(11, ThisModule, gwszModuleDescrip)
	TEST_CASE(1, Boundary)
	TEST_CASE(2, Transaction)
	TEST_CASE(3, SchemaRowset)
	TEST_CASE(4, Forward_Only_Cursor)
	TEST_CASE(5, Static_Query_Immediate)
	TEST_CASE(6, Static_Cursor_Buffered)
	TEST_CASE(7, Keyset_Query_Immediate)
	TEST_CASE(8, Keyset_Cursor_Buffered)
	TEST_CASE(9, Dynamic_Query_Buffered)
	TEST_CASE(10, Dynamic_Cursor_Immediate)
	TEST_CASE(11, ExtendedErrors)
END_TEST_MODULE()
// }}


// {{ TCW_TC_PROTOTYPE(Boundary)
//*-----------------------------------------------------------------------
//|	Test Case:		Boundary - Test boundary conditons
//|	Created:			05/28/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Boundary::Init()
{
	DBPROPID	rgGuid[2];
	ULONG		cProp=1;

	rgGuid[0]=DBPROP_IRowsetIdentity;

	if(!TCIRowsetIdentity::Init())
		return FALSE;

	//For read only provider or IRowsetChange not available, skip this variation
	//this is now a little redundant because a simialr check was just added to 
	//function GetRowsetAndAccessor
	if(!(g_fReadOnlyProvider) && g_fROWSETUPDATABLE)
	{
		rgGuid[1]=DBPROP_IRowsetChange;
		cProp++;
	}
	
	//get a rowset with IID_IRowsetIdentity
	if(!GetRowsetAndAccessor(USE_OPENROWSET, cProp,rgGuid,0,NULL,NO_ACCESSOR))
	{
		return FALSE;			 
	}
	else
	{
		return TRUE;
	}
}	


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc hThisRow is released
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Boundary::Variation_1()
{
	HROW		*pHRow		=NULL;
	DBCOUNTITEM	cRows		=0;
	BOOL		fTestPass	=FALSE;   
	HRESULT		hr;

	//if this test variation is not valid
	if (!g_fTestValid)
	{
		odtLog << L"The Provider does not support the properties that are necessary for this variation to execute.\n";
        return TEST_PASS;
	}
	//Restart the position to the begining of the rowset
	hr	= m_pIRowset->RestartPosition(NULL);
	if (S_OK!=hr && DB_S_COMMANDREEXECUTED!=hr)
	{
		goto END;
	}
	//get two row handles
	if(!CHECK(m_pIRowset->GetNextRows(NULL,0,2,&cRows,&pHRow),S_OK))
	{
		goto END;
	}

	//release the 2nd row handle
	if(!CHECK(m_pIRowset->ReleaseRows(1,&(pHRow[1]),NULL,NULL,NULL),S_OK))
	{
		goto END;
	}
	pHRow[1]=NULL;

	//mark the active row handles
	cRows--;

	//IsSameRow should return DB_E_BADROWHANDLE
	if(!CHECK(m_pIRowsetIdentity->IsSameRow(pHRow[1], *pHRow),DB_E_BADROWHANDLE))
	{
		goto END;
	}
	//binary comparision
	if(CompareHandlesByLiteral(pHRow[1], *pHRow,FALSE,FALSE))
	{
		fTestPass=TRUE;
	}
END:
	if(pHRow)
	{
		CHECK(m_pIRowset->ReleaseRows(cRows, pHRow, NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	if(fTestPass)
	{
		return TEST_PASS;
	}
	else
	{
		return TEST_FAIL;
	}
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc hThatRow is released
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Boundary::Variation_2()
{
	HROW		*pHRow		= NULL;
	DBCOUNTITEM	cRows		= 0;
	BOOL		fTestPass	= FALSE;   

	//if this test variation is not valid
	if (!g_fTestValid)
	{
		odtLog << L"The Provider does not support the properties that are necessary for this variation to execute.\n";
        return TEST_PASS;
	}
	
	//get two row handles
	if(!CHECK(m_pIRowset->GetNextRows(NULL,0,2,&cRows,&pHRow),S_OK))
		goto END;

	//release the 2nd row handle
	if(!CHECK(m_pIRowset->ReleaseRows(1,&(pHRow[1]),NULL,NULL,NULL),S_OK))
		goto END;
	pHRow[1]=NULL;

	//mark the active row handles
	cRows--;

	//IsSameRow should return DB_E_BADROWHANDLE
	if(!CHECK(m_pIRowsetIdentity->IsSameRow(*pHRow, pHRow[1]),DB_E_BADROWHANDLE))
	{
		goto END;
	}
	//binary comparision
	if(CompareHandlesByLiteral(*pHRow, pHRow[1],FALSE,FALSE))
	{
		fTestPass=TRUE;
	}
END:
	if(pHRow)
	{
		CHECK(m_pIRowset->ReleaseRows(cRows, pHRow, NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	if(fTestPass)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc hThisRow is hard-deleted
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Boundary::Variation_3()
{
	HROW		*pHRow		= NULL;
	DBCOUNTITEM	cRows		= 0;
	BOOL		fTestPass	= FALSE;
	DBROWSTATUS	rgRowStatus[1];

	//if this test variation is not valid
	if (!g_fTestValid)
	{
		odtLog << L"The Provider does not support the properties that are necessary for this variation to execute.\n";
        return TEST_PASS;
	}
	//if IRowsetChange not available because conformance won't allow it
	if (!g_fIRowsetChange)
	{
		odtLog << L"Can not get to IRowsetChange, can not insert a row.\n";
        return TEST_PASS;
	}
	//For read only provider or IRowsetChange not available, skip this variation
	if(g_fReadOnlyProvider  || !(g_fROWSETUPDATABLE))
	{
		odtLog << L"Can not get to IRowsetChange, can not insert a row.\n";
		return TEST_PASS;
	}
	//get two row handles
	if(!CHECK(m_pIRowset->GetNextRows(NULL,0,2,&cRows,&pHRow),S_OK))
		goto END;

	//check to see if the DSO is ReadOnly
	if( IsPropertySet(DBPROPSET_DATASOURCEINFO, DBPROP_DATASOURCEREADONLY, NULL, 0) )
	{
		//try deleting the row handle
		if(!CHECK(m_pIRowsetChange->DeleteRows(NULL,1,pHRow,rgRowStatus),DB_E_ERRORSOCCURRED) ||
		   !COMPARE(rgRowStatus[0], DBROWSTATUS_E_PERMISSIONDENIED))
			goto END;

		//IsSameRow should return S_OK
		if(!CHECK(m_pIRowsetIdentity->IsSameRow(*pHRow, pHRow[0]), S_OK))
		{
			goto END;
		}

		//IsSameRow should return S_FALSE
		if(!CHECK(m_pIRowsetIdentity->IsSameRow(*pHRow, pHRow[1]), S_FALSE))
		{
			goto END;
		}

		//binary comparision
		if(!CompareHandlesByLiteral(*pHRow, pHRow[0], FALSE, TRUE))
		{
			goto END;
		}
		
		//binary comparision
		if(!CompareHandlesByLiteral(*pHRow, pHRow[1], FALSE, FALSE))
		{
			goto END;
		}

		fTestPass=TRUE;
	}
	else
	{
		//delete the row handle
		if(!CHECK(m_pIRowsetChange->DeleteRows(NULL,1,pHRow,rgRowStatus),S_OK) ||
		   !COMPARE(rgRowStatus[0], DBROWSTATUS_S_OK))
			goto END;

		//IsSameRow should return DB_E_DELETEDROW
		if(!CHECK(m_pIRowsetIdentity->IsSameRow(*pHRow, pHRow[0]), DB_E_DELETEDROW))
		{
			goto END;
		}

		//IsSameRow should return DB_E_DELETEDROW
		if(!CHECK(m_pIRowsetIdentity->IsSameRow(*pHRow, pHRow[1]),DB_E_DELETEDROW))
		{
			goto END;
		}

		fTestPass=TRUE;
	}
END:
	CHECK(m_pIRowset->ReleaseRows(cRows, pHRow, NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);

	if(fTestPass)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc hThatRow is hard-deleted
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Boundary::Variation_4()
{
	HROW		*pHRow		= NULL;
	DBCOUNTITEM	cRows		= 0;
	BOOL		fTestPass	= FALSE;   
	DBROWSTATUS	rgRowStatus[1];

	//if this test variation is not valid
	if (!g_fTestValid)
	{
		odtLog << L"The Provider does not support the properties that are necessary for this variation to execute.\n";
        return TEST_PASS;
	}
	//if IRowsetChange not available because conformance won't allow it
	if (!g_fIRowsetChange)
	{
		odtLog << L"Can not get to IRowsetChange, can not insert a row.\n";
        return TEST_PASS;
	}	
	//if IRowsetChange not available because conformance won't allow it
	//For read only provider or IRowsetChange not available, skip this variation
	if(g_fReadOnlyProvider  || !(g_fROWSETUPDATABLE))
	{
		odtLog << L"Can not get to IRowsetChange, can not insert a row.\n";
		return TEST_PASS;
	}

	//get two row handles
	if(!CHECK(m_pIRowset->GetNextRows(NULL,0,2,&cRows,&pHRow),S_OK))
		goto END;

	//check to see if the DSO is ReadOnly
	if( IsPropertySet(DBPROPSET_DATASOURCEINFO, DBPROP_DATASOURCEREADONLY, NULL, 0) )
	{
		//try deleting the row handle
		if(!CHECK(m_pIRowsetChange->DeleteRows(NULL,1,&pHRow[1],rgRowStatus),DB_E_ERRORSOCCURRED) ||
		   !COMPARE(rgRowStatus[0], DBROWSTATUS_E_PERMISSIONDENIED))
			goto END;

		//IsSameRow should return S_OK
		if(!CHECK(m_pIRowsetIdentity->IsSameRow(*pHRow, pHRow[0]), S_OK))
		{
			goto END;
		}

		//IsSameRow should return S_FALSE
		if(!CHECK(m_pIRowsetIdentity->IsSameRow(*pHRow, pHRow[1]), S_FALSE))
		{
			goto END;
		}

		//binary comparision
		if(!CompareHandlesByLiteral(*pHRow, pHRow[0], FALSE, TRUE))
		{
			goto END;
		}
		
		//binary comparision
		if(!CompareHandlesByLiteral(*pHRow, pHRow[1], FALSE, FALSE))
		{
			goto END;
		}

		fTestPass=TRUE;
	}
	else
	{
		//delete the row handle
		if(!CHECK(m_pIRowsetChange->DeleteRows(NULL,1,&pHRow[1],rgRowStatus),S_OK) ||
		   !COMPARE(rgRowStatus[0], DBROWSTATUS_S_OK))
			goto END;

		//IsSameRow should return DB_E_DELETEDROW
		if(!CHECK(m_pIRowsetIdentity->IsSameRow(*pHRow, pHRow[0]), S_OK))
		{
			goto END;
		}

		//IsSameRow should return DB_E_DELETEDROW
		if(!CHECK(m_pIRowsetIdentity->IsSameRow(*pHRow, pHRow[1]),DB_E_DELETEDROW))
		{
			goto END;
		}

		fTestPass=TRUE;
	}
END:

	CHECK(m_pIRowset->ReleaseRows(cRows, pHRow, NULL,NULL,NULL),S_OK);
	PROVIDER_FREE(pHRow);

	if(fTestPass)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Compare with same row handle.  S_OK.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Boundary::Variation_5()
{
	HROW		HRow		= NULL;
	HROW		*pHRow		= &HRow;
	DBCOUNTITEM	cRows		= 0;
	BOOL		fTestPass	= FALSE;   

	//if this test variation is not valid
	if (!g_fTestValid)
	{
		odtLog << L"The Provider does not support the properties that are necessary for this variation to execute.\n";
        return TEST_PASS;
	}
	
	//get one row handles
	if(!CHECK(m_pIRowset->GetNextRows(NULL,1,1,&cRows,&pHRow),S_OK))
		goto END;

	//IsSameRow should return S_OK
	if(!CHECK(m_pIRowsetIdentity->IsSameRow(HRow,HRow),S_OK))
	{
		goto END;
	}
	//binary comparision
	if(CompareHandlesByLiteral(HRow,HRow,FALSE,TRUE))
	{
		fTestPass=TRUE;
	}
END:
	if(HRow)
		CHECK(m_pIRowset->ReleaseRows(1, pHRow, NULL,NULL,NULL),S_OK);

	if(fTestPass)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc hThisRow is DB_INVALID_HROW
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Boundary::Variation_6()
{
	HROW		HRow		= NULL;
	HROW		*pHRow		= &HRow;
	DBCOUNTITEM	cRows		= 0;
	BOOL		fTestPass	= FALSE;   

	//if this test variation is not valid
	if (!g_fTestValid)
	{
		odtLog << L"The Provider does not support the properties that are necessary for this variation to execute.\n";
        return TEST_PASS;
	}
	
	//get one row handles
	if(!CHECK(m_pIRowset->GetNextRows(NULL,1,1,&cRows,&pHRow),S_OK))
	{
		goto END;
	}
	//IsSameRow should return DB_E_BADROWHANDLE
	if(!CHECK(m_pIRowsetIdentity->IsSameRow(DB_NULL_HROW,HRow),DB_E_BADROWHANDLE))
	{
		goto END;
	}
	//binary comparision
	if(CompareHandlesByLiteral(DB_NULL_HROW,HRow,FALSE,FALSE))
	{
		fTestPass=TRUE;
	}
END:
	if(HRow)
	{
		CHECK(m_pIRowset->ReleaseRows(1, pHRow, NULL,NULL,NULL),S_OK);
	}
	if(fTestPass)
	{
		return TEST_PASS;
	}
	else
	{
		return TEST_FAIL;
	}
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc hThatRow is DB_INVALID_HROW
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Boundary::Variation_7()
{
	HROW		HRow		= NULL;
	HROW		*pHRow		= &HRow;
	DBCOUNTITEM	cRows		= 0;
	BOOL		fTestPass	= FALSE;   
		 
	//if this test variation is not valid
	if (!g_fTestValid)
	{
		odtLog << L"The Provider does not support the properties that are necessary for this variation to execute.\n";
        return TEST_PASS;
	}
	
	//IsSameRow should return DB_E_BADROWHANDLE
	if(!CHECK(m_pIRowsetIdentity->IsSameRow(DB_NULL_HROW, DB_NULL_HROW),DB_E_BADROWHANDLE))
	{
		goto END;
	}
	//binary comparision
	if(CompareHandlesByLiteral(DB_NULL_HROW,DB_NULL_HROW,FALSE,FALSE))
	{
		fTestPass=TRUE;
	}

	//get one row handles
	if(!CHECK(m_pIRowset->GetNextRows(NULL,1,1,&cRows,&pHRow),S_OK))
		goto END;

	//IsSameRow should return DB_E_BADROWHANDLE
	if(!CHECK(m_pIRowsetIdentity->IsSameRow(DB_NULL_HROW, DB_NULL_HROW),DB_E_BADROWHANDLE))
	{
		goto END;
	}
	fTestPass=TRUE;
END:
	if(HRow)
		CHECK(m_pIRowset->ReleaseRows(1, pHRow, NULL,NULL,NULL),S_OK);

	if(fTestPass)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc hRow is NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Boundary::Variation_8()
{
	HRESULT		ExpHR		= DB_E_BADROWHANDLE;
	HROW		*pHRow		= NULL;
	DBCOUNTITEM	cRows		= 0;
	BOOL		fTestPass	= FALSE;   
	HROW		*pHRowTemp;
	ULONG		rgRefCounts	= 0;

	//if this test variation is not valid
	if( !g_fTestValid )
	{
		odtLog << L"The Provider does not support the properties that are necessary for this variation to execute."<<ENDL;
        return TEST_SKIPPED;
	}
	
	//get one row handles
	TESTC_(m_pIRowset->GetNextRows(NULL,1,2,&cRows,&pHRow),S_OK);
	CHECK(m_pIRowset->ReleaseRows(1, pHRow, NULL, &rgRefCounts, NULL),S_OK);

	//Check the refcount and change the Expected HRESULT
	if( !rgRefCounts )
		ExpHR = DB_E_BADROWHANDLE;
	else
		ExpHR = S_FALSE;

	//IsSameRow should return DB_E_BADROWHANDLE	or S_FALSE
	TESTC_(m_pIRowsetIdentity->IsSameRow(pHRow[0],pHRow[1]),ExpHR);
	TESTC(CompareHandlesByLiteral(pHRow[0],pHRow[1],FALSE,FALSE));

	//IsSameRow should return DB_E_BADROWHANDLE	or S_FALSE
	TESTC_(m_pIRowsetIdentity->IsSameRow(pHRow[1],pHRow[0]),ExpHR);
	TESTC(CompareHandlesByLiteral(pHRow[1],pHRow[0],FALSE,FALSE));

	//release the sceond row
	pHRowTemp = &pHRow[1];
	CHECK(m_pIRowset->ReleaseRows(1, pHRowTemp, NULL, &rgRefCounts, NULL),S_OK);

	//Check the refcount and change the Expected HRESULT
	if( !rgRefCounts )
		ExpHR = DB_E_BADROWHANDLE;
	else
		ExpHR = S_FALSE;

	//IsSameRow should return DB_E_BADROWHANDLE	or S_FALSE
	TESTC_(m_pIRowsetIdentity->IsSameRow(pHRow[0],pHRow[1]),ExpHR);
	TESTC(CompareHandlesByLiteral(pHRow[0],pHRow[1],FALSE,FALSE));

	fTestPass=TRUE;

CLEANUP:

	PROVIDER_FREE(pHRow)

	if(fTestPass)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Boundary::Terminate()
{
	ReleaseRowsetAndAccessor();

	return(TCIRowsetIdentity::Terminate());
}	// }}
// }}


// {{ TCW_TC_PROTOTYPE(Transaction)
//*-----------------------------------------------------------------------
//|	Test Case:		Transaction - test zomibe and fully functional state after a trasaction
//|	Created:			05/28/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Transaction::Init()
{	
	BOOL			fPass				= FALSE;
	ULONG			j					= 0;	
	ULONG			i					= 0;	
	DBPROPID		rgGuid[5];
	ULONG			cProp				= 2;

	IRowsetIdentity		*pIRowsetIdentity	= NULL;
	IRowsetInfo			*pIRowsetInfo		= NULL;
	ULONG				cProperty			= 0;
	DBPROPID			DBPropID;
	DBPROPIDSET			DBPropIDSet;
	DBPROPSET			*pDBPropSet			= NULL;
	BOOL				fSupported			= FALSE;
	HRESULT				hr					= S_OK;
	
	// Initialize the array of Properties
	m_DBPropSet.rgProperties	= NULL;
	m_fTrans					= TRUE;
	
	if(!CTransaction::Init())
	{
		//it is not an error to not support transactions so return TRUE
		m_fTrans = FALSE;
		return TRUE;  
	}
	
	m_DBPropSet.guidPropertySet=DBPROPSET_ROWSET;
	if(!g_fReadOnlyProvider && g_fROWSETUPDATABLE)
	{
		cProp++;
		cProp++;
	}
	m_DBPropSet.cProperties=cProp;
	m_DBPropSet.rgProperties=(DBPROP *)PROVIDER_ALLOC(cProp*sizeof(DBPROP));

	//reset cProp and figure is out again so the array index will be correct
	cProp=2;

	if(!m_DBPropSet.rgProperties)
		return FALSE;

	//set IID_IRowsetIdentity, DBPROP_CANHOLDROWS, 
	//DBPROP_CANFETCHBACKWARDS, IID_IRowsetDelete
   	m_DBPropSet.rgProperties->dwPropertyID=DBPROP_IRowsetIdentity;                             
   	m_DBPropSet.rgProperties->dwOptions=DBPROPOPTIONS_REQUIRED;                                                
   	m_DBPropSet.rgProperties->vValue.vt=VT_BOOL;                                          
   	m_DBPropSet.rgProperties->colid=DB_NULLID;
	V_BOOL(&m_DBPropSet.rgProperties->vValue)=VARIANT_TRUE;                                   
                                                                                 
	m_DBPropSet.rgProperties[1].dwPropertyID=DBPROP_CANHOLDROWS;                    
   	m_DBPropSet.rgProperties[1].dwOptions=DBPROPOPTIONS_REQUIRED;                                              
   	m_DBPropSet.rgProperties[1].vValue.vt=VT_BOOL;
   	m_DBPropSet.rgProperties[1].colid=DB_NULLID;
   	V_BOOL(&m_DBPropSet.rgProperties[1].vValue)=VARIANT_TRUE; 

	if(!g_fReadOnlyProvider && g_fROWSETUPDATABLE)
	{
		m_DBPropSet.rgProperties[cProp].dwPropertyID=DBPROP_IRowsetChange;                    
	  	m_DBPropSet.rgProperties[cProp].dwOptions=DBPROPOPTIONS_REQUIRED;                                              
	   	m_DBPropSet.rgProperties[cProp].vValue.vt=VT_BOOL;                                        
	   	m_DBPropSet.rgProperties[cProp].colid=DB_NULLID;
	  	V_BOOL(&m_DBPropSet.rgProperties[cProp].vValue)=VARIANT_TRUE;  

		m_DBPropSet.rgProperties[cProp+1].dwPropertyID=DBPROP_UPDATABILITY;                    
		m_DBPropSet.rgProperties[cProp+1].dwOptions=DBPROPOPTIONS_REQUIRED;                                              
		if(SettableProperty(DBPROP_UPDATABILITY, DBPROPSET_ROWSET, g_pIDBCreateSession))
			m_DBPropSet.rgProperties[cProp+1].vValue.vt=VT_I4;                                        
		else
			m_DBPropSet.rgProperties[cProp+1].vValue.vt=VT_EMPTY;                                        
	   	m_DBPropSet.rgProperties[cProp+1].colid=DB_NULLID;
		m_DBPropSet.rgProperties[cProp+1].vValue.lVal=DBPROPVAL_UP_DELETE|DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_INSERT;  
	
		rgGuid[cProp]=DBPROP_IRowsetChange;
		rgGuid[cProp+1]=DBPROP_UPDATABILITY;
		cProp++;
		cProp++;
	}
	rgGuid[0]=DBPROP_IRowsetIdentity;
	rgGuid[1]=DBPROP_CANHOLDROWS;

	g_fTestValid = TRUE;

	//loop throuh all the requested properties for the rowset
	for (j=0;j<cProp;j++)
	{
		//loop through all the property values for which the test has default values
		for (i=g_wePrptIdxFIRST;i<=g_wePrptIdxLAST;i++)
		{	
			if (rgGuid[j]==g_rgDBPrpt[i].dwPropID)
			{
				//if a property isn't suported by the provider then do not continue
				if (!g_rgDBPrpt[i].fSupported)
				{
					odtLog<<L"Requested Property is NOT Supported\n";
					fPass = TRUE;
				}
				//if a prop is read only (not writeble) AND the value it is setting is
				//different from the default
				if	(!	(g_rgDBPrpt[i].dwFlags & DBPROPFLAGS_WRITE))
				{
					if(g_rgDBPrpt[i].fDefault != VARIANT_TRUE)
					{
						odtLog<<L"Requested Property is NOT Writable and test is trying to set it to non-default value\n";
						fPass = TRUE;
					}
				}break;
			}
		}
	}		
	//register interface to be tested                                         
   	if(!RegisterInterface(ROWSET_INTERFACE, IID_IRowsetIdentity, 1, &m_DBPropSet)) 
   	{
		return FALSE;
	}

	//start a transaction.  Create a rowset with IRowsetIdentity pointer.
	if(!StartTransaction(SELECT_ORDERBYNUMERIC,(IUnknown **)&pIRowsetIdentity, 1, &m_DBPropSet))
	{
		return FALSE;
	}

	//set some globals for this class

	//QI for IRowsetInfo interface
	if(!VerifyInterface(m_pIRowset, 
						IID_IRowsetInfo,
						ROWSET_INTERFACE, 
						(IUnknown**)&pIRowsetInfo))
	{
	}

	//initialize DBPROP_LITERALIDENTITY
	DBPropID = DBPROP_LITERALIDENTITY;
	DBPropIDSet.guidPropertySet=DBPROPSET_PROVIDERROWSET;
	DBPropIDSet.cPropertyIDs=1;
	DBPropIDSet.rgPropertyIDs=&DBPropID;

	//ask for in Prop
	hr = pIRowsetInfo->GetProperties(1,&DBPropIDSet,&cProperty,&pDBPropSet);

	if(V_BOOL(&pDBPropSet->rgProperties->vValue)==VARIANT_TRUE&&hr==S_OK)
		g_fLITERALIDENITIY	=TRUE;
	else
		g_fLITERALIDENITIY	=FALSE;

	FreeProperties(&cProperty,&pDBPropSet);

	//initialize DBPROP_REMOVEDELETED
	DBPropID = DBPROP_REMOVEDELETED;
	DBPropIDSet.guidPropertySet=DBPROPSET_PROVIDERROWSET;
	DBPropIDSet.cPropertyIDs=1;
	DBPropIDSet.rgPropertyIDs=&DBPropID;

	//ask for in Prop
	hr = pIRowsetInfo->GetProperties(1,&DBPropIDSet,&cProperty,&pDBPropSet);

	if(V_BOOL(&pDBPropSet->rgProperties->vValue)==VARIANT_TRUE&&hr==S_OK)
		g_fREMOVEDELETED	=TRUE;
	else
		g_fREMOVEDELETED	=FALSE;

	FreeProperties(&cProperty,&pDBPropSet);

	SAFE_RELEASE(pIRowsetInfo);
	SAFE_RELEASE(pIRowsetIdentity);

	//clean up
	CleanUpTransaction(S_OK);

	return TRUE;
}

// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Commit with Retaining.  Test on two identical hrows.  S_OK or E_UNEXPECTED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Transaction::Variation_1()
{
	IRowsetIdentity	*pIRowsetIdentity	= NULL;
	HROW			*pHRowThis			= NULL;
	HROW			*pHRowThat			= NULL;
	DBCOUNTITEM		cRowsThis			= 0;
	DBCOUNTITEM		cRowsThat			= 0;
	BOOL			fTestPass			= FALSE;
	HRESULT			hr					= S_OK;

	if (!m_fTrans)
	{
		odtLog << L"The Provider does not support Transactions.\n";
		return TRUE;
	}

	//if this test variation is not valid
	if (!g_fTestValid)
	{
		odtLog << L"The Provider does not support the properties that are necessary for this variation to execute.\n";
        return TEST_PASS;
	}
	
	//start a transaction.  Create a rowset with IRowsetIdentity pointer.
	if(!StartTransaction(SELECT_ORDERBYNUMERIC,(IUnknown **)&pIRowsetIdentity, 1, &m_DBPropSet))
		goto END;

	//get a row handle
	if(!CHECK(m_pIRowset->GetNextRows(NULL,3,1,&cRowsThis, &pHRowThis),S_OK))
		goto END;

	//commit the transaction with fRetaining==TRUE
	if(!GetCommit(TRUE))
		goto END;

	//if DBPROP_COMMITPRESERVE is VARIANT_FALSE, the row is in
	//zomibe state
	if(!m_fCommitPreserve)
	{
		if(CHECK(pIRowsetIdentity->IsSameRow(*pHRowThis, *pHRowThis),E_UNEXPECTED))
		{
			fTestPass=TRUE;
		}
		goto END;
	}

	//Restart the position to the begining of the rowset
	hr	= m_pIRowset->RestartPosition(NULL);
	if (S_OK!=hr && DB_S_COMMANDREEXECUTED!=hr)
	{
		goto END;
	}

	//get the same row handle
	if(!CHECK(m_pIRowset->GetNextRows(NULL,3,1,&cRowsThat, &pHRowThat),S_OK))
		goto END;

	if (cRowsThis!=cRowsThat)
	{
		goto END;
	}

	//IsSameRow should return S_OK
	if(!CHECK(pIRowsetIdentity->IsSameRow(*pHRowThis, *pHRowThat),S_OK))
	{
		goto END;
	}
	//binary comparision
	if(CompareHandlesByLiteral(*pHRowThis,*pHRowThat,FALSE,TRUE,g_fLITERALIDENITIY,g_fSTRONGIDENTITY))
	{
		fTestPass=TRUE;
	}
END:
	if(pHRowThis)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRowThis,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRowThis);
	}

	if(pHRowThat)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRowThat,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRowThat);
	}

	SAFE_RELEASE(pIRowsetIdentity);

	//clean up
	CleanUpTransaction(S_OK);

	if(fTestPass)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Commit without Retaining.  Test on two different hrows.  S_FALSE or E_UNEXPECTED.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Transaction::Variation_2()
{
	IRowsetIdentity	*pIRowsetIdentity	= NULL;
	HROW			*pHRowThis			= NULL;
	HROW			*pHRowThat			= NULL;
	DBCOUNTITEM		cRows				= 0;
	BOOL			fTestPass			= FALSE;

	if (!m_fTrans)
	{
		odtLog << L"The Provider does not support Transactions.\n";
		return TRUE;
	}

	//if this test variation is not valid
	if (!g_fTestValid)
	{
		odtLog << L"The Provider does not support the properties that are necessary for this variation to execute.\n";
        return TEST_PASS;
	}
	
	//start a transaction.  Create a rowset with IRowsetIdentity pointer.
	if(!StartTransaction(SELECT_ORDERBYNUMERIC, (IUnknown **)&pIRowsetIdentity,1, &m_DBPropSet))
		goto END;

	//get a row handle
	if(!CHECK(m_pIRowset->GetNextRows(NULL,3,1,&cRows, &pHRowThis),S_OK))
		goto END;

	//commit the transaction with fRetaining==FALSE
	if(!GetCommit(FALSE))
		goto END;

	//if DBPROP_COMMITPRESERVE is VARIANT_FALSE, the row is in
	//zomibe state
	if(!m_fCommitPreserve)
	{
		if(CHECK(pIRowsetIdentity->IsSameRow(*pHRowThis, *pHRowThis),E_UNEXPECTED))
		{
			fTestPass=TRUE;
		}
		goto END;
	}
	//get a different row handle
	if(!CHECK(m_pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRowThat),S_OK))
		goto END;

	//IsSameRow should return S_FALSE
	if(!CHECK(pIRowsetIdentity->IsSameRow(*pHRowThis, *pHRowThat),S_FALSE))
	{
		goto END;
	}
	//binary comparision
	if(CompareHandlesByLiteral(*pHRowThis,*pHRowThat,FALSE,FALSE,g_fLITERALIDENITIY,g_fSTRONGIDENTITY))
	{
		fTestPass=TRUE;
	}
END:
	if(pHRowThis)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRowThis,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRowThis);
	}

	if(pHRowThat)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRowThat,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRowThat);
	}

	SAFE_RELEASE(pIRowsetIdentity);

	//clean up
	CleanUpTransaction(XACT_E_NOTRANSACTION);

	if(fTestPass)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Abort with Retaining.  Test on one hard deleted row handle and one row handle.  
//			DB_E_DELETEDROW or E_UNEXPECTED.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Transaction::Variation_3()
{
	IRowsetIdentity	*pIRowsetIdentity	= NULL;
	IRowsetChange	*pIRowsetChange		= NULL;
	HROW			*pHRowThis			= NULL;
	HROW			*pHRowThat			= NULL;
	DBCOUNTITEM		cRowsThis			= 0;
	DBCOUNTITEM		cRowsThat			= 0;
	BOOL			fTestPass			= FALSE;
	HRESULT			hr					= S_OK;

	if (!m_fTrans)
	{
		odtLog << L"The Provider does not support Transactions.\n";
		return TRUE;
	}
	//if IRowsetChange not available because conformance won't allow it
	if (!g_fIRowsetChange)
	{
		odtLog << L"Can not get to IRowsetChange, can not insert a row.\n";
        return TEST_PASS;
	}	
	//if IRowsetChange not available because conformance won't allow it
	//For read only provider or IRowsetChange not available, skip this variation
	if(g_fReadOnlyProvider  || !(g_fROWSETUPDATABLE))
	{
		odtLog << L"Can not get to IRowsetChange, can not insert a row.\n";
		return TEST_PASS;
	}

	//if this test variation is not valid
	if (!g_fTestValid)
	{
		odtLog << L"The Provider does not support the properties that are necessary for this variation to execute.\n";
        return TEST_PASS;
	}
	
	//start a transaction.  Create a rowset with IRowsetIdentity pointer.
	if(!StartTransaction(SELECT_ORDERBYNUMERIC, (IUnknown **)&pIRowsetIdentity, 1, &m_DBPropSet))
		goto END;
	
	//QI for IRowsetDelete pointer
	if(!VerifyInterface(pIRowsetIdentity, 
						IID_IRowsetChange,
						ROWSET_INTERFACE, 
						(IUnknown**)&pIRowsetChange))
	{
	}

	//get a row handle
	if(!CHECK(m_pIRowset->GetNextRows(NULL,3,1,&cRowsThis, &pHRowThis),S_OK))
		goto END;

	//delete the row
	if(!CHECK(pIRowsetChange->DeleteRows(NULL,1,pHRowThis,NULL),S_OK))
		goto END;

	//abort the transaction with fRetaining==TRUE
	if(!GetAbort(TRUE))
		goto END;

	//if DBPROP_ABORTPRESERVE is VARIANT_FALSE, the row is in
	//zomibe state
	if(!m_fAbortPreserve)
	{
		if(CHECK(pIRowsetIdentity->IsSameRow(*pHRowThis, *pHRowThis),E_UNEXPECTED))
		{
			fTestPass=TRUE;
		}
		goto END;
	}

	//restart the position
	hr	= m_pIRowset->RestartPosition(NULL);
	if (S_OK!=hr && DB_S_COMMANDREEXECUTED!=hr)
	{
		goto END;
	}

	//get the same row handle
	if(!CHECK(m_pIRowset->GetNextRows(NULL,3,1,&cRowsThat, &pHRowThat),S_OK))
		goto END;

	if (cRowsThis!=cRowsThat)
	{
		goto END;
	}

	//IsSameRow should return S_OK, delete aborted
	if(!CHECK(pIRowsetIdentity->IsSameRow(*pHRowThis, *pHRowThat),S_OK))
	{
		goto END;
	}
	//binary comparision
	if(CompareHandlesByLiteral(*pHRowThis,*pHRowThat,FALSE,TRUE,g_fLITERALIDENITIY,g_fSTRONGIDENTITY))
	{
		fTestPass=TRUE;
	}
END:
	if(pHRowThis)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRowThis,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRowThis);
	}

	if(pHRowThat)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRowThat,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRowThat);
	}

	SAFE_RELEASE(pIRowsetChange);
	SAFE_RELEASE(pIRowsetIdentity);

	//clean up
	CleanUpTransaction(S_OK);

	if(fTestPass)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Abort without Retaining.  Test on one changed row handle and one row handle 	that refers to the same row.   S_OK or E_UNEXPECTE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Transaction::Variation_4()
{
	IRowsetIdentity	*pIRowsetIdentity	= NULL;
	IRowsetChange	*pIRowsetChange		= NULL;
	HROW			*pHRowThis			= NULL;
	HROW			*pHRowThat			= NULL;
	DBCOUNTITEM		cRows				= 0;
	BOOL			fTestPass			= FALSE;

	if (!m_fTrans)
	{
		odtLog << L"The Provider does not support Transactions.\n";
		return TRUE;
	}
	//if IRowsetChange not available because conformance won't allow it
	if (!g_fIRowsetChange)
	{
		odtLog << L"Can not get to IRowsetChange, can not delete a row.\n";
        return TEST_PASS;
	}	
	//if IRowsetChange not available because conformance won't allow it
	//For read only provider or IRowsetChange not available, skip this variation
	if(g_fReadOnlyProvider  || !(g_fROWSETUPDATABLE))
	{
		odtLog << L"Can not get to IRowsetChange, can not delete a row.\n";
		return TEST_PASS;
	}
	//if this test variation is not valid
	if (!g_fTestValid)
	{
		odtLog << L"The Provider does not support the properties that are necessary for this variation to execute.\n";
        return TEST_PASS;
	}
	
	//start a transaction.  Create a rowset with IRowsetIdentity pointer.
	if(!StartTransaction(SELECT_ORDERBYNUMERIC, 
		(IUnknown **)&pIRowsetIdentity, 1, &m_DBPropSet))
		goto END;
	
	//QI for IRowsetDelete pointer
	if(!VerifyInterface(pIRowsetIdentity, 
						IID_IRowsetChange,
						ROWSET_INTERFACE, 
						(IUnknown**)&pIRowsetChange))
	{
		goto END;
	}

	//get a row handle
	if(!CHECK(m_pIRowset->GetNextRows(NULL,3,1,&cRows, &pHRowThis),S_OK))
		goto END;

	//abort the transaction with fRetaining==FALSE
	if(!GetAbort(FALSE))
		goto END;

	//if DBPROP_ABORTPRESERVE is VARIANT_FALSE, the row is in
	//zomibe state
	if(!m_fAbortPreserve)
	{
		if(CHECK(pIRowsetIdentity->IsSameRow(*pHRowThis, *pHRowThis),E_UNEXPECTED))
		{
			fTestPass=TRUE;
		}
		goto END;
	}

	//get the row handle
	if(!CHECK(m_pIRowset->GetNextRows(NULL,3,1,&cRows, &pHRowThat),S_OK))
		goto END;

	//delete the row
	if(!CHECK(pIRowsetChange->DeleteRows(NULL,1,pHRowThat,NULL),S_OK))
		goto END;

	if(g_fREMOVEDELETED)
	{
		//IsSameRow should return S_FALSE
		if(!CHECK(pIRowsetIdentity->IsSameRow(*pHRowThis, *pHRowThat),S_FALSE))
		{
			goto END;
		}
	}
	else
	{
		//IsSameRow should return DB_E_DELETEDROW
		if(!CHECK(pIRowsetIdentity->IsSameRow(*pHRowThis, *pHRowThat),DB_E_DELETEDROW))
		{
			goto END;
		}
	}
	//binary comparision
	if(CompareHandlesByLiteral(*pHRowThis, *pHRowThat,FALSE,FALSE,g_fLITERALIDENITIY,g_fSTRONGIDENTITY))
	{
		fTestPass=TRUE;
	}
END:
	if(pHRowThis)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRowThis,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRowThis);
	}

	if(pHRowThat)
	{
		CHECK(m_pIRowset->ReleaseRows(1,pHRowThat,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRowThat);
	}

	SAFE_RELEASE(pIRowsetChange);
	SAFE_RELEASE(pIRowsetIdentity);

	//clean up
	CleanUpTransaction(XACT_E_NOTRANSACTION);

	if(fTestPass)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Transaction::Terminate()
{
	if(m_DBPropSet.rgProperties)
		PROVIDER_FREE(m_DBPropSet.rgProperties);
	
	return(CTransaction::Terminate());
}	// }}
// }}


// {{ TCW_TC_PROTOTYPE(SchemaRowset)
//*-----------------------------------------------------------------------
//|	Test Case:		SchemaRowset 
//|	Created:			02/29/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL SchemaRowset::Init()
{
	DBPROPID	rgGuidSet[1];

	rgGuidSet[0]=DBPROP_IRowsetIdentity;
	
	if (!g_fIDBSchemaRowset)
	{
		odtLog << L"IDBSchemaRowset is not supported by Provider." << ENDL;
		return TRUE;
	}

	if(!TCIRowsetIdentity::Init())
		return FALSE;
	
	//open rowset for IRowsetIdentity & IDBSchemaRowset
	if(!GetRowsetAndAccessor(USE_OPENROWSET, 1, rgGuidSet,0,NULL,NO_ACCESSOR))
	{
		return FALSE;
	}
	
	return TRUE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc SchemaRowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int SchemaRowset::Variation_1()
{
	DBCOUNTITEM		cRows				= 0;
	HROW			*pHRow				= NULL;
	IRowsetIdentity	*pIRowsetIdentity	= NULL;
	IRowset			*pIRowset			= NULL;
	BOOL			fTestPass			= FALSE;

	if (!g_fIDBSchemaRowset)
	{
		return TEST_SKIPPED;
	}

	//IRowset
	if(!VerifyInterface(m_pIAccessor, IID_IRowset, ROWSET_INTERFACE, (IUnknown**)&pIRowset))
	{
		odtLog << L"IRowset is not supported by Provider." << ENDL;
	}     
	//IRowsetIdentity
	if(!VerifyInterface(pIRowset, IID_IRowsetIdentity, ROWSET_INTERFACE, (IUnknown**)&pIRowsetIdentity))
	{
		odtLog << L"IRowsetIdentity is not supported by Provider." << ENDL;
	}     
	
	//get two row handles
	if(!CHECK(pIRowset->GetNextRows(NULL,4,2,&cRows, &pHRow),S_OK))
		goto END;

	//IsSameRow should return S_FALSE
	if(!CHECK(pIRowsetIdentity->IsSameRow(*pHRow, pHRow[1]),S_FALSE))
	{
		goto END;
	}
	//binary comparision
	if(CompareHandlesByLiteral(*pHRow, pHRow[1],FALSE,FALSE))
	{
		fTestPass=TRUE;
	}
END:
	if(pHRow)
	{
		CHECK(pIRowset->ReleaseRows(cRows, pHRow, NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}
	
	SAFE_RELEASE(pIRowsetIdentity);
	SAFE_RELEASE(pIRowset);

	if(fTestPass)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL SchemaRowset::Terminate()
{
	ReleaseRowsetAndAccessor();

	return(TCIRowsetIdentity::Terminate());
}	
// }}
// }}


// {{ TCW_TC_PROTOTYPE(Forward_Only_Cursor)
//*-----------------------------------------------------------------------
//|	Test Case:		Forward_Only_Cursor - Forward_Only_Cursor
//|	Created:			05/28/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Forward_Only_Cursor::Init()
{
	BOOL fResults	= TRUE;
	
	if(!TCIRowsetIdentity::Init())
	{
		return FALSE;
	}

	ReleaseRowsetAndAccessor();
	return fResults;	
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Fetch two different rows.  S_FALSE.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Forward_Only_Cursor::Variation_1()
{
	DBCOUNTITEM	cRows			= 0;
	HROW		*pHRow			= NULL;
	BOOL		fTestPass		= TEST_SKIPPED;
	DBPROPID	rgDBPropIDSet[1];
	
	rgDBPropIDSet[0] = DBPROP_IRowsetIdentity;

	if(!GetRowsetAndAccessor(USE_OPENROWSET, 1, rgDBPropIDSet,0,NULL,NO_ACCESSOR))
	{	
		odtLog << L"The Provider does not support the properties that are necessary for this variation to execute.\n";
		goto END;
	}
	
	fTestPass	= TEST_FAIL;

	//get two row handles
	if(!CHECK(m_pIRowset->GetNextRows(NULL,4,2,&cRows, &pHRow),S_OK))
		goto END;

	//IsSameRow should return S_FALSE
	if(!CHECK(m_pIRowsetIdentity->IsSameRow(*pHRow, pHRow[1]),S_FALSE))
	{
		goto END;
	}
	//binary comparision
	if(CompareHandlesByLiteral(*pHRow, pHRow[1],FALSE,FALSE))
	{
		fTestPass=TRUE;
	}
END:
	if(pHRow)
	{
		CHECK(m_pIRowset->ReleaseRows(cRows, pHRow, NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}
	ReleaseRowsetAndAccessor();

	if( fTestPass )
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}

// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc multiple rows fetched 2 different times.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Forward_Only_Cursor::Variation_2()
{
	const	LONG		cRows2Fetch			= 3;
	DBCOUNTITEM			cRowsA				= 0;
	DBCOUNTITEM			cRowsB				= 0;
	DBCOUNTITEM			cRowsTemp			= 0;
	HROW				*pHRowA				= NULL;
	HROW				*pHRowB				= NULL;
	HROW				*pHRowTemp			= NULL;
	BOOL				fTestPass			= TEST_SKIPPED;
	DBPROPID			rgDBPropIDSet[3];
	HRESULT				hr					= S_OK;
	UWORD				i					= 0;

	rgDBPropIDSet[0]		= DBPROP_IRowsetIdentity;
	rgDBPropIDSet[1]		= DBPROP_CANHOLDROWS;
	rgDBPropIDSet[2]		= DBPROP_CANFETCHBACKWARDS;

	if(!GetRowsetAndAccessor(USE_OPENROWSET,2,rgDBPropIDSet,0,NULL,NO_ACCESSOR))
	{	
		odtLog << L"The Provider does not support the properties that are necessary for this variation to execute.\n";
		goto END;
	}
	
	fTestPass	= TEST_FAIL;

	//get row handles
	if(!CHECK(m_pIRowset->GetNextRows(NULL,0,cRows2Fetch,&cRowsA, &pHRowA),S_OK))
	{
		goto END;
	}
	//restart the position
	hr	= m_pIRowset->RestartPosition(NULL);
	if (S_OK!=hr && DB_S_COMMANDREEXECUTED!=hr)
	{
		if (DB_E_ROWSNOTRELEASED==hr)
		{
			//if CANHOLDROWS is TRUE and RestartPosition returns DB_E_ROWSNOTRELEASED
			//then check DBPROP_QUICKRESTART.  
			//if DBPROP_QUICKRESTART is FALSE (command re-executed) this could be valid 
			//but if DBPROP_QUICKRESTART is TRUE DB_E_ROWSNOTRELEASED should not be 
			//returned from RestartPosition if CANHOLDROWS is requested
			if(GetProperty(DBPROP_QUICKRESTART, DBPROPSET_ROWSET, (IUnknown*)m_pIRowset))		
			{
				goto END;
			}
			else
			{
				//can't just do a refresh so drop and recreate the rowset this time
				//do a fetch backwards here so we still have rowA handles and can compare
				
				//release the rows and restart again
				if(pHRowA)
				{
					CHECK(ReleaseRows(cRows2Fetch,&pHRowA,TRUE),S_OK);
					PROVIDER_FREE(pHRowA);
				}
				ReleaseRowsetAndAccessor();
				if(!GetRowsetAndAccessor(USE_OPENROWSET,3,rgDBPropIDSet,0,NULL,NO_ACCESSOR))
				{	
					odtLog << L"The Provider does not support the properties that are necessary for this variation to execute.\n";
					fTestPass	= TEST_SKIPPED;
					goto END;
				}
				fTestPass	= TEST_FAIL;
				//get row handles
				if(!CHECK(m_pIRowset->GetNextRows(NULL,0,cRows2Fetch,&cRowsA, &pHRowA),S_OK))
				{
					goto END;
				}
				COMPARE(cRows2Fetch,cRowsA);
				//move back over row handles
				if(!CHECK(m_pIRowset->GetNextRows(NULL,0,-cRows2Fetch,&cRowsTemp, &pHRowTemp),S_OK))
				{
					goto END;
				}
				COMPARE(cRowsA,cRowsTemp);
				//get the same row handles
				if(!CHECK(m_pIRowset->GetNextRows(NULL,0,cRows2Fetch,&cRowsB, &pHRowB),S_OK))
				{
					goto END;
				}
				for (i = 0;i<cRows2Fetch;i++)
				{
					//IsSameRow should return S_OK
					if(!CHECK(m_pIRowsetIdentity->IsSameRow(pHRowA[i], pHRowB[i]),S_OK))
					{
						goto END;
					}
					//binary comparision
					if(!CompareHandlesByLiteral(pHRowA[i], pHRowB[i],FALSE,TRUE))
					{
						goto END;
					}
					if(!CHECK(m_pIRowsetIdentity->IsSameRow(pHRowA[i], pHRowTemp[(cRows2Fetch-1)-i]),S_OK))
					{
						goto END;
					}
					//binary comparision
					if(!CompareHandlesByLiteral(pHRowA[i], pHRowTemp[(cRows2Fetch-1)-i],FALSE,TRUE))
					{
						goto END;
					}
				}
				fTestPass=TRUE;
			}
			goto END;
		}
		else
		{
			goto END;
		}
	}

	//release the rows and restart again
	if(pHRowB)
	{
		CHECK(ReleaseRows(cRows2Fetch,&pHRowB,TRUE),S_OK);
		PROVIDER_FREE(pHRowB);
	}

	//get the same row handles
	if(!CHECK(m_pIRowset->GetNextRows(NULL,0,cRows2Fetch,&cRowsB, &pHRowB),S_OK))
	{
		goto END;
	}
	for (i = 0;i<cRows2Fetch;i++)
	{
		//IsSameRow should return S_OK
		if(!CHECK(m_pIRowsetIdentity->IsSameRow(pHRowA[i], pHRowB[i]),S_OK))
		{
			goto END;
		}
		//binary comparision
		if(CompareHandlesByLiteral(pHRowA[i], pHRowB[i],FALSE,TRUE))
		{
			fTestPass=TRUE;
		}
	}
	fTestPass=TRUE;
END:
	if (m_pIRowset && pHRowA && pHRowB)
	{
		CHECK(m_pIRowset->ReleaseRows(cRowsA, pHRowA, NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRowA);

		CHECK(m_pIRowset->ReleaseRows(cRowsB, pHRowB, NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRowB);

		CHECK(m_pIRowset->ReleaseRows(cRowsTemp, pHRowTemp, NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRowTemp);

		ReleaseRowsetAndAccessor();
	}

	if( fTestPass )
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc REMOVEDELETED FALSE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Forward_Only_Cursor::Variation_3()
{
	const	ULONG		cRows2Use			= 3;
	DBCOUNTITEM			cRowsA				= 0;
	DBCOUNTITEM			cRowsB				= 0;
	HROW				*pHRowA				= NULL;
	HROW				*pHRowB				= NULL;
	BOOL				fTestPass			= TEST_SKIPPED;
	DBPROPID			rgDBPropIDSet[3];
	DBPROPID			rgDBPropIDUnSet[1];
	HRESULT				hr					= S_OK;
	UWORD				i					= 0;

	rgDBPropIDSet[0]	= DBPROP_IRowsetIdentity;
	rgDBPropIDSet[1]	= DBPROP_IRowsetChange;
	rgDBPropIDSet[2]	= DBPROP_CANHOLDROWS;

	rgDBPropIDUnSet[0]	= DBPROP_REMOVEDELETED;

	m_ulUpdFlags	= DBPROPVAL_UP_DELETE;

	if(!GetRowsetAndAccessor(USE_OPENROWSET,3,rgDBPropIDSet,1,rgDBPropIDUnSet,NO_ACCESSOR))
	{	
		odtLog << L"The Provider does not support the properties that are necessary for this variation to execute.\n";
		goto END;
	}
	
	fTestPass	= TEST_FAIL;
	
	if (!m_pIRowsetChange)
	{
		goto END;
	}

	//get row handles
	if(!CHECK(m_pIRowset->GetNextRows(NULL,1,cRows2Use,&cRowsA, &pHRowA),S_OK))
	{
		goto END;
	}	
	//delete the rows
	if(!CHECK(m_pIRowsetChange->DeleteRows(NULL,cRows2Use,pHRowA, NULL),S_OK))
	{
		goto END;
	}
	//restart the position
	hr	= m_pIRowset->RestartPosition(NULL);
	if (S_OK!=hr && DB_S_COMMANDREEXECUTED!=hr)
	{
		goto END;
	}

	//get the same row handles
	if(!CHECK(m_pIRowset->GetNextRows(NULL,1,cRows2Use,&cRowsB, &pHRowB),S_OK))
		goto END;

	for (i=0;i<cRows2Use;i++)
	{
		//IsSameRow should return DB_E_DELETEDROW
		if(!CHECK(m_pIRowsetIdentity->IsSameRow(pHRowA[i], pHRowB[i]),DB_E_DELETEDROW))
		{
			goto END;
		}
		//binary comparision
		if(!CompareHandlesByLiteral(pHRowA[i], pHRowB[i],FALSE,TRUE))
		{
			goto END;
		}
	}
	fTestPass=TRUE;
END:
	m_ulUpdFlags	= DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_DELETE|DBPROPVAL_UP_INSERT;
	if (m_pIRowset)
	{
		CHECK(m_pIRowset->ReleaseRows(cRowsA, pHRowA, NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRowA);


		CHECK(m_pIRowset->ReleaseRows(cRowsB, pHRowB, NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRowB);

		ReleaseRowsetAndAccessor();
	}

	if( fTestPass )
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc REMOVEDELETED TRUE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Forward_Only_Cursor::Variation_4()
{
	const	ULONG cRows2Use			= 3;
	DBCOUNTITEM	  cRowsA			= 0;
	DBCOUNTITEM	  cRowsA2			= 0;
	DBCOUNTITEM	  cRowsB			= 0;
	HROW		  *pHRowA			= NULL;
	HROW		  *pHRowA2			= NULL;
	HROW		  *pHRowB			= NULL;
	BOOL		  fTestPass			= TEST_SKIPPED;
	DBPROPID	  rgDBPropIDSet[4];
	HRESULT		  hr				= S_OK;
	UWORD		  i					= 0;
	void		  *pDataA2			= NULL;
	void		  *pDataB			= NULL;


	rgDBPropIDSet[0] = DBPROP_IRowsetIdentity;
	rgDBPropIDSet[1] = DBPROP_IRowsetChange;
	rgDBPropIDSet[2] = DBPROP_CANHOLDROWS;
	rgDBPropIDSet[3] = DBPROP_REMOVEDELETED;

	if(!GetRowsetAndAccessor(USE_OPENROWSET,4,rgDBPropIDSet,0,NULL,ON_ROWSET_ACCESSOR))
	{	
		odtLog << L"The Provider does not support the properties that are necessary for this variation to execute.\n";
		goto CLEANUP;
	}
	
	QTESTC(m_pIRowsetChange != NULL);

	fTestPass = TEST_FAIL;
	
	//get row handles
	TESTC_(m_pIRowset->GetNextRows(NULL,0,cRows2Use,&cRowsA, &pHRowA),S_OK);

	//get next set of row handles
	TESTC_(m_pIRowset->GetNextRows(NULL,0,cRows2Use,&cRowsA2, &pHRowA2),S_OK);

	//delete the rows
	TESTC_(m_pIRowsetChange->DeleteRows(NULL,cRows2Use,pHRowA, NULL),S_OK);

	//restart the position
	hr = m_pIRowset->RestartPosition(NULL);
	TEST2C_(hr, S_OK, DB_S_COMMANDREEXECUTED);

	//get the same row handles
	TESTC_(m_pIRowset->GetNextRows(NULL,0,cRows2Use,&cRowsB, &pHRowB),S_OK);

	for (i=0; i < cRows2Use; i++)
	{
		//IsSameRow pHRowA and pHRowB
		//should return DB_E_DELETEDROW because  pHRowA can only point to is deleted rows
		TESTC_(m_pIRowsetIdentity->IsSameRow(pHRowA[i], pHRowB[i]),DB_E_DELETEDROW);
		TESTC(CompareHandlesByLiteral(pHRowA[i], pHRowB[i],FALSE,FALSE));

		//IsSameRow pHRowA2 and pHRowB
		//should return S_OK
		TESTC_(m_pIRowsetIdentity->IsSameRow(pHRowA2[i], pHRowB[i]),S_OK);
		TESTC(CompareHandlesByLiteral(pHRowA2[i], pHRowB[i],FALSE,TRUE));

		//allocate the data
		pDataA2 = PROVIDER_ALLOC(m_cRowSize);
		TESTC(pDataA2 != NULL);

		pDataB = PROVIDER_ALLOC(m_cRowSize);
		TESTC(pDataB != NULL);

		//get the data
		TESTC_(m_pIRowset->GetData(pHRowA2[i], m_hAccessor, pDataA2),S_OK);

		//get the data
		TESTC_(m_pIRowset->GetData(pHRowB[i], m_hAccessor, pDataB),S_OK);

		//compare the buffer
		TESTC(CompareBuffer(pDataA2,pDataB,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_FREE));
		
		//free the memory
		PROVIDER_FREE(pDataA2);
		PROVIDER_FREE(pDataB);
	}

	fTestPass=TRUE;

CLEANUP:

	if( m_pIRowset )
	{
		CHECK(m_pIRowset->ReleaseRows(cRowsA, pHRowA, NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRowA);

		CHECK(m_pIRowset->ReleaseRows(cRowsA2, pHRowA2, NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRowA2);

		CHECK(m_pIRowset->ReleaseRows(cRowsB, pHRowB, NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRowB);

		ReleaseRowsetAndAccessor();
	}

	//free the memory
	PROVIDER_FREE(pDataA2);
	PROVIDER_FREE(pDataB);

	if( fTestPass )
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}

// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc QI without DBPROP_IRowsetIdentity requested
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Forward_Only_Cursor::Variation_5()
{
	IRowset			*pIRowset			= NULL;
	IRowsetIdentity	*pIRowsetIdentity	= NULL;
	DBCOUNTITEM		cRows				= 0;
	HROW			*pHRow				= NULL;
	HRESULT			hr;
	BOOL			fTestPass			= TEST_SKIPPED;
	DBPROPSET		rgDBPropSet[1]		= {NULL};
	

	//IOpenRowset to return a Rowset from table
	g_pCTable->CreateRowset	(
								USE_OPENROWSET,
								IID_IRowset,
								0,
								NULL,
								(IUnknown**)&pIRowset,
								NULL,
								NULL,
								NULL
							);

	//Get a row handle
	if(!CHECK(pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRow),S_OK))
	{
		goto END;
	}

	fTestPass	= TEST_FAIL;

	//get the IRowsetIdentity pointer without requesting the DBPROP_IRowsetIdentity prop
	hr	= pIRowset->QueryInterface(IID_IRowsetIdentity, (void**)&pIRowsetIdentity);

	//if this succeeded test the interface
	if (S_OK==hr)
	{
		if(!CHECK(pIRowsetIdentity->IsSameRow(*pHRow,*pHRow),S_OK))
		{
			goto END;
		}
		fTestPass = TRUE;
	}
	//if it failed make sure gettting the interface is possible
	if (E_NOINTERFACE==hr)
	{
		//init DBPropSet[0]
		rgDBPropSet[0].rgProperties		=NULL;
		rgDBPropSet[0].guidPropertySet	=DBPROPSET_ROWSET;
		rgDBPropSet[0].cProperties		=1;
		//allocate 
		rgDBPropSet[0].rgProperties=(DBPROP *)PROVIDER_ALLOC(sizeof(DBPROP)*(rgDBPropSet[0].cProperties));
		if (!rgDBPropSet[0].rgProperties)
		{
			goto END;
		}
		memset(rgDBPropSet[0].rgProperties,0xCA,(rgDBPropSet[0].cProperties)*sizeof(DBPROP));

		rgDBPropSet[0].rgProperties[0].dwPropertyID=DBPROP_IRowsetIdentity;
		rgDBPropSet[0].rgProperties[0].dwOptions=DBPROPOPTIONS_REQUIRED;
		rgDBPropSet[0].rgProperties[0].vValue.vt=VT_BOOL;
		rgDBPropSet[0].rgProperties[0].colid=DB_NULLID;
		V_BOOL(&rgDBPropSet[0].rgProperties[0].vValue)=VARIANT_TRUE;

		if(pHRow)
		{
			pIRowset->ReleaseRows(cRows, pHRow, NULL,NULL, NULL);
		}
		// Release interface pointer
		SAFE_RELEASE(pIRowset);

		//IOpenRowset to return a Rowset from table
		g_pCTable->CreateRowset	(
									USE_OPENROWSET,
									IID_IRowset,
									1,
									rgDBPropSet,
									(IUnknown**)&pIRowset,
									NULL,
									NULL,
									NULL
								);
		//Get a row handle
		if(!CHECK(pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRow),S_OK))
		{
			goto END;
		}
		
		//get the IRowsetIdentity pointer without requesting the DBPROP_IRowsetIdentity prop
		if(!CHECK(pIRowset->QueryInterface(IID_IRowsetIdentity, (void**)&pIRowsetIdentity),S_OK))
		{
			goto END;
		}
		if(!CHECK(pIRowsetIdentity->IsSameRow(*pHRow,*pHRow),S_OK))
		{
			goto END;
		}
		fTestPass = TRUE;
	}
END:
	PROVIDER_FREE(rgDBPropSet[0].rgProperties);
	if(pHRow)
	{
		pIRowset->ReleaseRows(cRows, pHRow, NULL,NULL, NULL);
	}

	// Release interface pointers
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pIRowsetIdentity);

	if( fTestPass )
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Forward_Only_Cursor::Terminate()
{
	ReleaseRowsetAndAccessor();

	return(TCIRowsetIdentity::Terminate());
}	// }}
// }}


// {{ TCW_TC_PROTOTYPE(Static_Query_Immediate)
//*-----------------------------------------------------------------------
//|	Test Case:		Static_Query_Immediate - Static_Query_Immediate
//|	Created:			05/28/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Static_Query_Immediate::Init()
{
	DBPROPID	rgGuid[5];
	DBPROPID	rgGuidUnset[2];
	BOOL		fCursorNotSupported=FALSE;
	ULONG		cProp=0;
	ULONG		cPropUnset=0;

	rgGuidUnset[0]=DBPROP_OTHERUPDATEDELETE;
	rgGuidUnset[1]=DBPROP_OTHERINSERT;
	cPropUnset=2;

	rgGuid[0]=DBPROP_IRowsetIdentity;
	rgGuid[1]=DBPROP_CANSCROLLBACKWARDS;
	rgGuid[2]=DBPROP_CANHOLDROWS;
	rgGuid[3]=DBPROP_IRowsetChange;
	rgGuid[4]=KAGPROP_QUERYBASEDUPDATES;	
	//if this is kagera then count the KAGPROP above as a property
	cProp=g_MSDASQL?5:4;

	if(!TCIRowsetIdentity::Init())
		return FALSE;

	//check to see if the execute is going to fail
	if( !g_rgDBPrpt[IDX_ScrollBackwards].fSupported ||
		!g_rgDBPrpt[IDX_CanHoldRows].fSupported ||
		!g_rgDBPrpt[IDX_FetchBackwards].fSupported )
	{
		fCursorNotSupported = TRUE;
		odtLog << L"The Provider does not support a STATIC Cursor.\n";
	}

	//get a rowset with IRowsetIdentity, with Static Cursor
	if(!GetRowsetAndAccessor(USE_OPENROWSET, cProp, rgGuid,cPropUnset,rgGuidUnset,NO_ACCESSOR))
	{
		return TEST_SKIPPED;
	}
	if (!g_fTestValid)
	{
		return TRUE;
	}
	//has to be static cursor
	if(GetCursorType()==STATIC_CURSOR)
	{
		odtLog << L"This test will continue but a static cursor was not obtained.\n";	//	return FALSE;
	}
	return TRUE;
}

// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc  Fetch one row.  Move the cursor and fetch backwards to retrieve the same row handle.  S_OK.  
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Static_Query_Immediate::Variation_1()
{
	HROW		rgHrow[3];
	HROW		*pHRow			= rgHrow;
	DBCOUNTITEM	cRows			= 0;
	ULONG		cRowCount		= 0;
	BOOL		fTestPass		= FALSE;

	//Get a row handle
	if(!CHECK(m_pIRowset->GetNextRows(NULL,8,1,&cRows, &pHRow),S_OK))
		goto END;

	cRowCount++;

	//move the cursor away
	pHRow=&(rgHrow[1]);
	if(!CHECK(m_pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRow),S_OK))
		goto END;

	cRowCount++;

	//get the first row handle again
	pHRow=&(rgHrow[2]);
	if(!CHECK(m_pIRowset->GetNextRows(NULL,-2,1,&cRows, &pHRow),S_OK))
		goto END;

	cRowCount++;

	//IsSameRow should return S_OK.
	if(!CHECK(m_hr=m_pIRowsetIdentity->IsSameRow(rgHrow[0], rgHrow[2]),S_OK))
	{
		goto END;
	}
	//binary comparision
	if(CompareHandlesByLiteral(rgHrow[0], rgHrow[2],FALSE,TRUE))
	{
		fTestPass=TRUE;
	}
END:
	CHECK(m_pIRowset->ReleaseRows(cRowCount, rgHrow, NULL,NULL, NULL),S_OK);

	if(fTestPass)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}

// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Hard deleted the 1st row handle.  Fetch the same row again.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Static_Query_Immediate::Variation_2()
{
	HROW		*pHRow			= NULL;
	HROW		*pHRowSecond	= NULL;
	DBCOUNTITEM	cRows			= 0;
	BOOL		fTestPass		= FALSE;
	DBROWSTATUS	*pDBRowStatus	= NULL;
	BOOL		fPending		= TRUE;
	
	//get a row handle
	if(!CHECK(m_pIRowset->GetNextRows(NULL,1,1,&cRows,&pHRow),S_OK))
		goto END;

	//hard delete the row
	if(!CHECK(DeleteRows(1,pHRow),S_OK))
		goto END;

	//get the same row again 
	if(!CHECK(m_pIRowset->GetNextRows(NULL,-1,1,&cRows,&pHRowSecond),S_OK))
		goto END;

	//IsSameRow should return DB_E_DELETEDROW 
	if(!CHECK(m_pIRowsetIdentity->IsSameRow(*pHRow, *pHRowSecond),DB_E_DELETEDROW))
	{
		goto END;
	}
	//binary comparision
	if(!CompareHandlesByLiteral(*pHRow, *pHRowSecond,FALSE,TRUE))
	{
		goto END;
	}

	if (g_fREMOVEDELETED)
	{
		//IsSameRow should return S_OK - REMOVEDELETED is TRUE
		if(!CHECK(m_pIRowsetIdentity->IsSameRow(*pHRowSecond, *pHRowSecond),S_OK))
		{
			goto END;
		}
		//binary comparision
		if(!CompareHandlesByLiteral(*pHRowSecond, *pHRowSecond,FALSE,TRUE))
		{
			goto END;
		}
	}
	else
	{
		//IsSameRow should return DB_E_DELETEDROW REMOVEDELETED is FALSE
		if(!CHECK(m_pIRowsetIdentity->IsSameRow(*pHRowSecond, *pHRowSecond),DB_E_DELETEDROW))
		{
			goto END;
		}
		//binary comparision - because a handle to a deleted row should compare with itself
		if(!CompareHandlesByLiteral(*pHRowSecond, *pHRowSecond,FALSE,TRUE))
		{
			goto END;
		}
	}
	fTestPass=TRUE;
END:
	//undo the deletions
	if(pHRow)
	{
		CHECK(ReleaseRows(1, &pHRow,TRUE),S_OK);
	}

	if(pHRowSecond)
		CHECK(ReleaseRows(1, &pHRowSecond,TRUE),S_OK);

	if(pDBRowStatus)
		PROVIDER_FREE(pDBRowStatus);

	if(fTestPass)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Static_Query_Immediate::Terminate()
{
	ReleaseRowsetAndAccessor();

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIRowsetIdentity::Terminate());
}	
// }}
// }}


// {{ TCW_TC_PROTOTYPE(Static_Cursor_Buffered)
//*-----------------------------------------------------------------------
//|	Test Case:		Static_Cursor_Buffered - Static_Cursor_Buffered
//|	Created:			05/28/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Static_Cursor_Buffered::Init()
{
	DBPROPID	rgGuid[5];
	DBPROPID	rgGuidUnset[2];
	BOOL		fCursorNotSupported	= FALSE;
	ULONG		cProp				= 0;
	ULONG		cPropUnset			= 0;

	rgGuidUnset[0]=DBPROP_OTHERUPDATEDELETE;
	rgGuidUnset[1]=DBPROP_OTHERINSERT;
	cPropUnset=2;

	rgGuid[0]=DBPROP_IRowsetIdentity;
	rgGuid[1]=DBPROP_CANSCROLLBACKWARDS;
	rgGuid[2]=DBPROP_CANHOLDROWS;
	rgGuid[3]=DBPROP_IRowsetUpdate;
	rgGuid[4]=DBPROP_IRowsetChange;
	cProp=5;

	if(!TCIRowsetIdentity::Init())
		return FALSE;

	//check to see if the execute is going to fail
	if( !g_rgDBPrpt[IDX_ScrollBackwards].fSupported)
	{
		fCursorNotSupported = TRUE;
		odtLog << L"The Provider does not support a STATIC Cursor.\n";
	}

	//get a rowset with IRowsetIdentity, with Static Cursor
	if(!GetRowsetAndAccessor(USE_OPENROWSET, cProp, rgGuid,cPropUnset,rgGuidUnset,ON_ROWSET_ACCESSOR))
	{
		return TEST_SKIPPED;
	}
	if (!g_fTestValid)
	{
		return TRUE;
	}

	//has to be static cursor
	if(!COMPARE(GetCursorType()==STATIC_CURSOR, TRUE))
	{
		return FALSE;
	}

	return TRUE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Soft deleted the 1st row handle.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Static_Cursor_Buffered::Variation_1()
{
	HROW		*pHRow0			= NULL;
	HROW		*pHRow1			= NULL;
	HROW		*pHRow2			= NULL;
	HROW		*pHRow1AfterDel	= NULL;
	DBCOUNTITEM	cRows			= 0;
	BOOL		fTestPass		= FALSE;
	DBROWSTATUS	*pDBRowStatus	= NULL;
	BOOL		fPending		= TRUE;
	void		*pData1			= NULL;
	void		*pData2			= NULL;
		

	if(!(pData1=(BYTE*)PROVIDER_ALLOC(m_cRowSize)))
			goto END;
	if(!(pData2=(BYTE*)PROVIDER_ALLOC(m_cRowSize)))
			goto END;
		
	//get a row handle
	if(!CHECK(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow0),S_OK))
		goto END;

	//get the next row handle
	if(!CHECK(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow1),S_OK))
		goto END;
	//get the data for the row
	if(!CHECK(m_pIRowset->GetData(*pHRow1, m_hAccessor, pData1),S_OK))
		goto END;

	//get the next row handle
	if(!CHECK(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow2),S_OK))
		goto END;

	//soft delete the 2nd row
	if(!CHECK(DeleteRows(1,pHRow1),S_OK))
		goto END;

	//try to get the 2nd row again, pending delete might or might not be there
	//if it is there this will fetch it, if it is not there this will fetch the first row
	if(!CHECK(m_pIRowset->GetNextRows(NULL,-2,1,&cRows,&pHRow1AfterDel),S_OK))
		goto END;

	//determine through props which row was fetched and check it

	//if test doesn't see its own deletes the first row should have been fetched
	if (!g_fOWNUPDATEDELETE)
	{
		//IsSameRow should return S_OK. 
		if(!CHECK(m_pIRowsetIdentity->IsSameRow(*pHRow1, *pHRow1AfterDel),S_OK))
		{
			goto END;
		}
		//binary comparision, 
		if(!CompareHandlesByLiteral(*pHRow1, *pHRow1AfterDel,FALSE,TRUE))
		{
			goto END;
		}
		//so look at buffer just for fun
		if(!CHECK(m_pIRowset->GetData(*pHRow1AfterDel, m_hAccessor, pData2),S_OK))
			goto END;
			
		//since OWNUPDATEDELETE is FALSE these do compare, delete is not seen
		if(!CompareBuffer(pData1,pData2,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
		{
			goto END;
		}
	}
	else
	{
		if (g_fLITERALIDENITIY&&g_fREMOVEDELETED)
		{
			//IsSameRow should return S_FALSE. 
			if(!CHECK(m_pIRowsetIdentity->IsSameRow(*pHRow1, *pHRow1AfterDel),S_FALSE))
			{
				goto END;
			}
			//binary comparision, 
			if(!CompareHandlesByLiteral(*pHRow1, *pHRow1AfterDel,FALSE,FALSE))
			{
				goto END;
			}
			//IsSameRow should return S_OK first row fetched and new fetch because the deleted row is removed
			if(!CHECK(m_pIRowsetIdentity->IsSameRow(*pHRow0, *pHRow1AfterDel),S_OK))
			{
				goto END;
			}
			//binary comparision, these rows can compare literally
			if(!CompareHandlesByLiteral(*pHRow0, *pHRow1AfterDel,FALSE,TRUE))
			{
				goto END;
			}
		}
		else
		{
			//IsSameRow should return DB_E_DELETEDROW the deleted row is not removed which is
			//pointed to by pHRow1AfterDel
			//IsSameRow should return S_OK. 
			if(!CHECK(m_pIRowsetIdentity->IsSameRow(*pHRow1, *pHRow1AfterDel),S_OK))
			{
				goto END;
			}
			//binary comparision, 
			if(!CompareHandlesByLiteral(*pHRow1, *pHRow1AfterDel,FALSE,TRUE))
			{
				goto END;
			}
		}
	}

	//Update the data, deletes are no longer pending
	if(!CHECK(UpdateRows(1,pHRow1,&pDBRowStatus),S_OK))
		goto END;

	//IsSameRow should return DB_E_DELETEDROW because pHRow1 points to a deleted
	//row no matter what DBPROP_REMOVEDELETED has for a default
	if(!CHECK(m_pIRowsetIdentity->IsSameRow(*pHRow1, *pHRow1AfterDel),DB_E_DELETEDROW))
	{
		goto END;
	}

	if (g_fREMOVEDELETED)
	{
		//binary comparision
		if(!CompareHandlesByLiteral(*pHRow1, *pHRow1AfterDel,FALSE,FALSE))
		{
			goto END;
		}

		//IsSameRow should return S_OK because the deleted row is removed
		if(!CHECK(m_pIRowsetIdentity->IsSameRow(*pHRow0, *pHRow1AfterDel),S_OK))
		{
			goto END;
		}
		//binary comparision, these rows can compare literally
		if(!CompareHandlesByLiteral(*pHRow0, *pHRow1AfterDel,FALSE,TRUE))
		{
			goto END;
		}
	}
	else
	{
		//binary comparision
		if(!CompareHandlesByLiteral(*pHRow1, *pHRow1AfterDel,FALSE,TRUE))
		{
			goto END;
		}

		//IsSameRow should return DB_E_DELETEDROW the deleted row is not removed which is
		//pointed to by pHRow1AfterDel
		if(!CHECK(m_pIRowsetIdentity->IsSameRow(*pHRow0, *pHRow1AfterDel),DB_E_DELETEDROW))
		{
			goto END;
		}
		//binary comparision
		if(!CompareHandlesByLiteral(*pHRow0, *pHRow1AfterDel,FALSE,FALSE))
		{
			goto END;
		}
	}
	fTestPass	= TRUE;
END:
	//undo the deletions
	if(pHRow0)
	{
		CHECK(ReleaseRows(1, &pHRow0,TRUE),S_OK);
	}
	//undo the deletions
	if(pHRow1)
	{
		CHECK(ReleaseRows(1, &pHRow1,TRUE),S_OK);
	}
	//undo the deletions
	if(pHRow2)
	{
		CHECK(ReleaseRows(1, &pHRow2,TRUE),S_OK);
	}

	if(pData1)
 	  ReleaseInputBindingsMemory(m_cBinding, m_rgBinding,(BYTE *)pData1,FALSE);

	if(pData2)
	  ReleaseInputBindingsMemory(m_cBinding, m_rgBinding,(BYTE *)pData2,TRUE);

	if(pHRow1AfterDel)
		CHECK(ReleaseRows(1, &pHRow1AfterDel,TRUE),S_OK);

	if(pDBRowStatus)
		PROVIDER_FREE(pDBRowStatus);

	if(fTestPass)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Fetch hRowThis and soft change the row.  The row should stay in place in the rowset.  
//			Fetch the row handle again in hRowThat
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Static_Cursor_Buffered::Variation_2()
{
	void		*pData1			= NULL;
	void		*pData2			= NULL;
	HROW		*pHRowThis		= NULL;
	HROW		*pHRowThat		= NULL;
	DBCOUNTITEM	cRows			= 0;
	BOOL		fTestPass		= FALSE;
	HRESULT		hr;

	//get an accessor
	if(!GetAccessorOnRowset(ON_ROWSET_ACCESSOR,
		DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
		UPDATEABLE_COLS_BOUND))
		goto END;

	//Restart the position to the begining of the rowset
	hr	= m_pIRowset->RestartPosition(NULL);
	if (S_OK!=hr && DB_S_COMMANDREEXECUTED!=hr)
	{
		goto END;
	}
	
	//get a row handle
	if(!CHECK(m_pIRowset->GetNextRows(NULL,9,1,&cRows,&pHRowThis),S_OK))
		goto END;

	//get the data for the row
	if(!CHECK(m_pIRowset->GetData(*pHRowThis, m_hAccessor, m_pData),S_OK))
		goto END;

	//change it
	if(!CHECK(ChangeOneRow(*pHRowThis, g_ulNextRow++, &pData1),S_OK))
		goto END;
	
	//get the row handle again
	if(!CHECK(m_pIRowset->GetNextRows(NULL,-1,1,&cRows,&pHRowThat),S_OK))
		goto END;

	//IsSameRow should return S_OK
	if(!CHECK(m_pIRowsetIdentity->IsSameRow(*pHRowThis, *pHRowThat),S_OK))
	{
		goto END;
	}
	//binary comparision
	if(!CompareHandlesByLiteral(*pHRowThis, *pHRowThat,FALSE,TRUE))
	{
		goto END;
	}

	//will this rowset see its pending changes
	if (g_fLITERALIDENITIY&&g_fOWNUPDATEDELETE)
	{
		//GetData on pHRowThat, should return the updated data buffer
		if(!COMPARE(CheckExpectedData(*pHRowThat, pData1, TRUE),TRUE))
			goto END;
	}	
	else
	{
		//GetData on pHRowThat, should return the first data because
		//LI is false (pending changes are not seen) 
		//the old buffer should be here
		if(!COMPARE(CheckExpectedData(*pHRowThat, m_pData, TRUE),TRUE))
			goto END;
	}

	//change data by pHRowThat
	if(!CHECK(ChangeOneRow(*pHRowThat, g_ulNextRow++, &pData2),S_OK))
		goto END;

	//this rowset will see its pending change only if DBPROP_LITERALIDENTITY is TRUE  AND OWNUPDEL is TRUE
	//if LI is FALSE pending changes are not seen
	if (g_fLITERALIDENITIY&&g_fOWNUPDATEDELETE)
	{
		if(!COMPARE(CheckExpectedData(*pHRowThis, pData2, TRUE),TRUE))
			goto END;
	}
	else
	{
		if(!COMPARE(CheckExpectedData(*pHRowThis, pData1, TRUE),TRUE))
			goto END;
	}

	//Undo hRowThat
	//same rules apply for Undo as for visibility of pending changes
	//if LI is TRUE test just undid all changes
	//eles it just undid changes on hRowThat
	if(!CHECK(UndoRows(1,pHRowThat),S_OK))
		goto END;

	//this rowset will see its pending change only if DBPROP_LITERALIDENTITY is TRUE AND OWNUPDEL is TRUE
	//if LI is FALSE pending changes are not seen
	if (g_fLITERALIDENITIY&&g_fOWNUPDATEDELETE)
	{
		//GetData on pHRowThis should return its first change
		if(!COMPARE(CheckExpectedData(*pHRowThis, m_pData, TRUE),TRUE))
			goto END;
	}
	else
	{
		//GetData on pHRowThis should return its first change
		if(!COMPARE(CheckExpectedData(*pHRowThis, pData1, TRUE),TRUE))
			goto END;
	}

	//release the one row handle(this)
	if(pHRowThis)
		CHECK(ReleaseRows(1,&pHRowThis,TRUE),S_OK);

	//get the row handle again.  
	if(!CHECK(m_pIRowset->GetNextRows(NULL,-1,1,&cRows,&pHRowThis),S_OK))
		goto END;

	//IsSameRow should return S_OK (the row will always be the row, just the data might be chaged)
	if(!CHECK(m_pIRowsetIdentity->IsSameRow(*pHRowThis, *pHRowThat),S_OK))
	{
		goto END;
	}
	//binary comparision
	if(!CompareHandlesByLiteral(*pHRowThis, *pHRowThat,FALSE,TRUE))
	{
		goto END;
	}

	//this rowset will see its pending change only if DBPROP_LITERALIDENTITY is TRUE AND OWNUPDEL is TRUE
	//if LI is FALSE pending changes are not seen
	if (g_fLITERALIDENITIY&&g_fOWNUPDATEDELETE)
	{
		//GetData on pHRowThat should return row after undo
		if(!COMPARE(CheckExpectedData(*pHRowThat, m_pData, TRUE),TRUE))
			goto END;
	}
	else
	{
		//GetData on pHRowThat should return the row's first change
		if(!COMPARE(CheckExpectedData(*pHRowThat, pData2, TRUE),TRUE))
			goto END;
	}

	fTestPass=TRUE;
END:
	if(m_pData)
 	  ReleaseInputBindingsMemory(m_cBinding, m_rgBinding,(BYTE *)m_pData,FALSE);

	if(pData1)
 	  ReleaseInputBindingsMemory(m_cBinding, m_rgBinding,(BYTE *)pData1,FALSE);

	if(pData2)
	  ReleaseInputBindingsMemory(m_cBinding, m_rgBinding,(BYTE *)pData2,TRUE);

	if(pHRowThis)
		CHECK(ReleaseRows(1,&pHRowThis,TRUE),S_OK);

 	if(pHRowThat)
		CHECK(ReleaseRows(1,&pHRowThat,TRUE),S_OK);

	ReleaseAccessorOnRowset();

	if(fTestPass)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Insert a new row.  Compare it with a new row handle.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Static_Cursor_Buffered::Variation_3()
{
	HROW		HRow		= NULL;
	HROW		*pHRow		= NULL;
	DBCOUNTITEM	cRows		= 0;
	BYTE		*pData		= NULL;
	BOOL		fTestPass	= FALSE;
	HRESULT		hr;
	BOOL		fFound		= FALSE;

	//Restart the position to the begining of the rowset
	hr	= m_pIRowset->RestartPosition(NULL);
	if (S_OK!=hr && DB_S_COMMANDREEXECUTED!=hr)
	{
		goto END;
	}

	//Create an accessor
	if(!GetAccessorOnRowset(ON_ROWSET_ACCESSOR,
		DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,					
		UPDATEABLE_COLS_BOUND))
		goto END;

	if(!CHECK(InsertOneRow(g_ulNextRow, &HRow),S_OK))
		goto END;

	//if can't see this insert then skip 
	if (g_fRETURNPENDINGINSERTS&&g_fOWNINSERT&&g_fLITERALIDENITIY)
	{
		if(!(pData=(BYTE*)PROVIDER_ALLOC(m_cRowSize)))
		{
			goto END;
		}
		//get a data buffer with the inserted data
		if(!CHECK(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
						&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK))
		{
			goto END;
		}

		//Get a row handle, loop through the rowset looking for the row
		while (S_OK==(hr = m_pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRow)) || hr == DB_S_ENDOFROWSET )
		{
			if( cRows ==0)
				break;
			//Get the data for the ith row handle
			if(!CHECK(m_pIRowset->GetData(*pHRow,m_hAccessor,m_pData),S_OK))
			{
				goto END;
			}

			//make sure GetData should be able to see the change
			if(CompareBuffer(m_pData,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
			{
				fFound=TRUE;
				break;
			}
			if(!CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK))
			{
				goto END;
			}
			PROVIDER_FREE(pHRow);
		}		
		
		//the inserted row has to be found
		if (!fFound)
		{
			goto END;
		}

		//IsSameRow should return DB_E_NEWLYINSERTED if DBPROP_STRONGIDENTITY 
		//is VARIANT_FALSE only after the data has been transmitted to the back end
		//S_OK should be returned
		if(!CHECK(m_pIRowsetIdentity->IsSameRow(HRow, *pHRow),S_OK))
		{
			goto END;
		}
		//binary comparision
		if(!CompareHandlesByLiteral(HRow, *pHRow,FALSE,TRUE))
		{
			goto END;
		}

		//Undo the insertion
		if(!CHECK(UndoRows(1,&HRow),S_OK))
			goto END;

		//IsSameRow should return DB_E_DELETEDROW, pending inserted rows undone are like
		//deleted rows
		if(!CHECK(m_pIRowsetIdentity->IsSameRow(HRow, *pHRow),DB_E_DELETEDROW))
		{
			goto END;
		}
		//binary comparision
		if(!CompareHandlesByLiteral(HRow, *pHRow,FALSE,TRUE))
		{
			goto END;
		}

		//Get Data should return DB_E_DELETEDROW
		if(CHECK(m_pIRowset->GetData(HRow,m_hAccessor,pData),DB_E_DELETEDROW))
		{
			fTestPass=TRUE;
		}
	}
	else
	{
		fTestPass=TEST_SKIPPED;
	}
END:
	if(HRow)
		CHECK(m_pIRowset->ReleaseRows(1,&HRow, NULL, NULL, NULL),S_OK);

	if(pHRow)
		CHECK(ReleaseRows(1, &pHRow, TRUE),S_OK);

	if(pData)
		PROVIDER_FREE(pData);

	ReleaseAccessorOnRowset();

	if(fTestPass)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}
// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Static_Cursor_Buffered::Terminate()
{
	ReleaseRowsetAndAccessor();

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIRowsetIdentity::Terminate());
}	// }}
// }}


// {{ TCW_TC_PROTOTYPE(Keyset_Query_Immediate)
//*-----------------------------------------------------------------------
//|	Test Case:		Keyset_Query_Immediate - Keyset_Query_Immediate
//|	Created:			05/28/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Keyset_Query_Immediate::Init()
{
	DBPROPID	rgGuid[7];
	BOOL		fCursorNotSupported	= FALSE;
	DBPROPID	rgGuidUnset[1];
	ULONG		cProp				= 0;
	ULONG		cPropUnset			= 0;

	rgGuidUnset[0]=DBPROP_OTHERINSERT;
	cPropUnset=1;

	rgGuid[0]=DBPROP_IRowsetIdentity;
	rgGuid[1]=DBPROP_IRowsetChange;
	rgGuid[2]=DBPROP_OTHERUPDATEDELETE;
	rgGuid[3]=DBPROP_CANHOLDROWS;
	rgGuid[4]=DBPROP_CANSCROLLBACKWARDS;
	rgGuid[5]=DBPROP_IRowsetLocate;
	rgGuid[6]=KAGPROP_QUERYBASEDUPDATES;
	
	cProp=g_MSDASQL?7:6;

	if(!TCIRowsetIdentity::Init())
		return FALSE;

	//check to see if the execute is going to fail
	if( !g_rgDBPrpt[IDX_ScrollBackwards].fSupported ||
		!g_rgDBPrpt[IDX_CanHoldRows].fSupported ||
		!g_rgDBPrpt[IDX_OtherUpdateDelete].fSupported )
	{
		fCursorNotSupported = TRUE;
		odtLog << L"The Provider does not support a KEYSET Cursor.\n";
	}

	//get a rowset with IRowsetIdentity, with keyset driven Cursor
	if(!GetRowsetAndAccessor(USE_OPENROWSET, cProp, rgGuid,cPropUnset,rgGuidUnset,NO_ACCESSOR))
	{
		return TEST_SKIPPED;
	}
	if (!g_fTestValid)
	{
		return TRUE;
	}

	//has to be key cursor
	if(GetCursorType()==KEYSET_DRIVEN_CURSOR)
	{
		odtLog << L"This test will continue but a keyset cursor was not obtained.\n";	//	return FALSE;
	}

	return TRUE;
}

// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc  Fetch one row.  Move the cursor and fetch backwards to retrieve the same row handle.  S_OK.  
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Keyset_Query_Immediate::Variation_1()
{
	HROW		rgHrow[2];
	HROW		*pHRow				= rgHrow;
	DBCOUNTITEM	cRows				= 0;
	DBCOUNTITEM	cRowCount			= 0;
	BOOL		fTestPass			= FALSE;
	HRESULT		hr;

	//if this test variation is not valid
	if (!g_fTestValid)
	{
		odtLog << L"The Provider does not support the properties that are necessary for this variation to execute.\n";
        return TEST_SKIPPED;
	}
	
	//Restart the position to the begining of the rowset
	hr	= m_pIRowset->RestartPosition(NULL);
	if (S_OK!=hr && DB_S_COMMANDREEXECUTED!=hr)
	{
		goto END;
	}

	//Get a row handle
	if(!CHECK(m_pIRowset->GetNextRows(NULL,8,1,&cRows, &pHRow),S_OK))
		goto END;

	cRowCount++;

	//Get a row handle
	pHRow=&(rgHrow[1]);
	if(!CHECK(m_pIRowset->GetNextRows(NULL,-1,1,&cRows, &pHRow),S_OK))
		goto END;

	cRowCount++;

	//IsSameRow should return S_OK
	if(!CHECK(m_pIRowsetIdentity->IsSameRow(rgHrow[0], rgHrow[1]),S_OK))
	{
		goto END;
	}
	//binary comparision
	if(CompareHandlesByLiteral(rgHrow[0], rgHrow[1],FALSE,TRUE))
	{
		fTestPass = TRUE;
	}
END:
	CHECK(m_pIRowset->ReleaseRows(cRowCount, rgHrow, NULL, NULL, NULL),S_OK);

	if(fTestPass)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Fetch hRowThis and change a non-key value.  
//			Fetch the same row in hRowThat by IRowsetLocate::GetRowsByBookmark.  S_OK.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Keyset_Query_Immediate::Variation_2()
{
	DBORDINAL	cColsNumber		= 0;
	DBORDINAL	*rgColsNumber	= NULL;
	void		*pData			= NULL;
	HROW		*pHRowThis		= NULL;
	HROW		*pHRowThat		= NULL;
	DBCOUNTITEM	cRows			= 0;
	BOOL		fTestPass		= FALSE;
	HRESULT		hr;

	//if this test variation is not valid
	if (!g_fTestValid)
	{
		odtLog << L"The Provider does not support the properties that are necessary for this variation to execute.\n";
        return TEST_SKIPPED;
	}
	
	//If the provider is read only, skip the variation
	if(g_fReadOnlyProvider)
		return TEST_SKIPPED;

	//get non-key columns
	if(!GetNonKeyAndUpdatable(&cColsNumber, &rgColsNumber, m_pIMalloc))
		goto END;

	//get an accessor
	if(!GetAccessorOnRowset(ON_ROWSET_ACCESSOR,
		DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
		USE_COLS_TO_BIND_ARRAY, FORWARD, NO_COLS_BY_REF,DBTYPE_EMPTY,       
		cColsNumber,rgColsNumber))
		goto END;

	//Restart the position to the begining of the rowset
	hr	= m_pIRowset->RestartPosition(NULL);
	if (S_OK!=hr && DB_S_COMMANDREEXECUTED!=hr)
	{
		goto END;
	}

	//get a row handle
	if(!CHECK(m_pIRowset->GetNextRows(NULL,5,1,&cRows,&pHRowThis),S_OK))
		goto END;

	//change it
	if(!CHECK(ChangeOneRow(*pHRowThis, g_ulNextRow++, &pData),S_OK))
		goto END;

	//Restart the position to the begining of the rowset
	hr	= m_pIRowset->RestartPosition(NULL);
	if (S_OK!=hr && DB_S_COMMANDREEXECUTED!=hr)
	{
		goto END;
	}

	//get the row handle again
	if(!CHECK(m_pIRowset->GetNextRows(NULL,5,1,&cRows,&pHRowThat),S_OK))
		goto END;

	//IsSameRow should return S_OK
	if(!CHECK(m_pIRowsetIdentity->IsSameRow(*pHRowThis, *pHRowThat),S_OK))
	{
		goto END;
	}
	//binary comparision
	if(!CompareHandlesByLiteral(*pHRowThis, *pHRowThat,FALSE,TRUE))
	{
		goto END;
	}

	//GetData on pHRowThat should return the updated data
	if(COMPARE(CheckExpectedData(*pHRowThat, pData, TRUE),TRUE))
		fTestPass=TRUE;
END:
	if(pData)
	  ReleaseInputBindingsMemory(m_cBinding, m_rgBinding,(BYTE *)pData,TRUE);

	if(rgColsNumber)
		PROVIDER_FREE(rgColsNumber);

	if(pHRowThis)
		CHECK(ReleaseRows(1,&pHRowThis,TRUE),S_OK);

 	if(pHRowThat)
		CHECK(ReleaseRows(1,&pHRowThat,TRUE),S_OK);


	ReleaseAccessorOnRowset();

	if(fTestPass)
		return TEST_PASS;
	else
		return TEST_FAIL;
}

// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Hard deleted a row handle.  Fetch the same row again forwards.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Keyset_Query_Immediate::Variation_3()
{
	HROW		*pHRow			= NULL;
	HROW		*pHRowSecond	= NULL;
	DBCOUNTITEM	cRows			= 0;
	BOOL		fTestPass		= FALSE;
	DBROWSTATUS	*pDBRowStatus	= NULL;
	BOOL		fPending		= TRUE;
	HRESULT		hr;
	
	//Restart the position to the begining of the rowset
	hr	= m_pIRowset->RestartPosition(NULL);
	if (S_OK!=hr && DB_S_COMMANDREEXECUTED!=hr)
	{
		goto END;
	}

	//get a row handle
	if(!CHECK(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow),S_OK))
		goto END;

	//hard delete the row
	if(!CHECK(DeleteRows(1,pHRow),S_OK))
		goto END;

	//get the same row again 
	if(!CHECK(m_pIRowset->GetNextRows(NULL,-1,1,&cRows,&pHRowSecond),S_OK))
		goto END;

	//IsSameRow should return DB_E_DELETEDROW because pHRow is being compared
	if(!CHECK(m_pIRowsetIdentity->IsSameRow(*pHRow, *pHRowSecond),DB_E_DELETEDROW))
	{
		goto END;
	}

	//if test can see it own deletes these should compare
	if (g_fOWNUPDATEDELETE)
	{
		//binary comparision
		if(!CompareHandlesByLiteral(*pHRow, *pHRowSecond,FALSE,TRUE))
		{
			goto END;
		}
	}
	else
	{
		//binary comparision
		if(CompareHandlesByLiteral(*pHRow, *pHRowSecond,FALSE,TRUE))
		{
			goto END;
		}
	}

	if (g_fREMOVEDELETED)
	{
		//IsSameRow should return S_OK - REMOVEDELETED is TRUE
		if(!CHECK(m_pIRowsetIdentity->IsSameRow(*pHRowSecond, *pHRowSecond),S_OK))
		{
			goto END;
		}
		//binary comparision
		if(!CompareHandlesByLiteral(*pHRowSecond, *pHRowSecond,FALSE,TRUE))
		{
			goto END;
		}
	}
	else
	{
		//if test can see it own deletes
		if (g_fOWNUPDATEDELETE)
		{
			//IsSameRow should return DB_E_DELETEDROW REMOVEDELETED is FALSE
			if(!CHECK(m_pIRowsetIdentity->IsSameRow(*pHRowSecond, *pHRowSecond),DB_E_DELETEDROW))
			{
				goto END;
			}
			//binary comparision - because a handle to a deleted row should compare with itself
			if(!CompareHandlesByLiteral(*pHRowSecond, *pHRowSecond,FALSE,TRUE))
			{
				goto END;
			}
		}
		else
		{
			//IsSameRow should return DB_E_DELETEDROW REMOVEDELETED is FALSE
			//but so is g_fOWNUPDATEDELETE so deltes are not seen
			if(!CHECK(m_pIRowsetIdentity->IsSameRow(*pHRowSecond, *pHRowSecond),S_OK))
			{
				goto END;
			}
			//binary comparision 
			if(!CompareHandlesByLiteral(*pHRowSecond, *pHRowSecond,FALSE,TRUE))
			{
				goto END;
			}
		}
	}
	fTestPass=TRUE;
END:
	//undo the deletions
	if(pHRow)
	{
		CHECK(ReleaseRows(1, &pHRow,TRUE),S_OK);
	}

	if(pHRowSecond)
		CHECK(ReleaseRows(1, &pHRowSecond,TRUE),S_OK);

	if(pDBRowStatus)
		PROVIDER_FREE(pDBRowStatus);

	if(fTestPass)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Hard deleted a row handle.  Fetch the same row again backwards.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Keyset_Query_Immediate::Variation_4()
{
	HROW		*pHRow			= NULL;
	HROW		*pHRowSecond	= NULL;
	DBCOUNTITEM	cRows			= 0;
	BOOL		fTestPass		= FALSE;
	DBROWSTATUS	*pDBRowStatus	= NULL;
	BOOL		fPending		= TRUE;
	HRESULT		hr;
	
	//Restart the position to the begining of the rowset
	hr	= m_pIRowset->RestartPosition(NULL);
	if (S_OK!=hr && DB_S_COMMANDREEXECUTED!=hr)
	{
		goto END;
	}

	//get a row handle
	if(!CHECK(m_pIRowset->GetNextRows(NULL,1,1,&cRows,&pHRow),S_OK))
		goto END;

	//hard delete the row
	if(!CHECK(DeleteRows(1,pHRow),S_OK))
		goto END;

	//get the same row again 
	if(!CHECK(m_pIRowset->GetNextRows(NULL,0,-1,&cRows,&pHRowSecond),S_OK))
		goto END;

	//IsSameRow should return DB_E_DELETEDROW because pHRow is being compared
	if(!CHECK(m_pIRowsetIdentity->IsSameRow(*pHRow, *pHRowSecond),DB_E_DELETEDROW))
	{
		goto END;
	}

	//if test can see it own deletes these should compare
	if (g_fOWNUPDATEDELETE)
	{
		//binary comparision
		if(!CompareHandlesByLiteral(*pHRow, *pHRowSecond,FALSE,TRUE))
		{
			goto END;
		}
	}
	else
	{
		//binary comparision
		if(CompareHandlesByLiteral(*pHRow, *pHRowSecond,FALSE,TRUE))
		{
			goto END;
		}
	}

	if (g_fREMOVEDELETED)
	{
		//IsSameRow should return S_OK - REMOVEDELETED is TRUE
		if(!CHECK(m_pIRowsetIdentity->IsSameRow(*pHRowSecond, *pHRowSecond),S_OK))
		{
			goto END;
		}
		//binary comparision
		if(!CompareHandlesByLiteral(*pHRowSecond, *pHRowSecond,FALSE,TRUE))
		{
			goto END;
		}
	}
	else
	{
		//if test can see it own deletes
		if (g_fOWNUPDATEDELETE)
		{
			//IsSameRow should return DB_E_DELETEDROW REMOVEDELETED is FALSE
			if(!CHECK(m_pIRowsetIdentity->IsSameRow(*pHRowSecond, *pHRowSecond),DB_E_DELETEDROW))
			{
				goto END;
			}
			//binary comparision - because a handle to a deleted row should compare with itself
			if(!CompareHandlesByLiteral(*pHRowSecond, *pHRowSecond,FALSE,TRUE))
			{
				goto END;
			}
		}
		else
		{
			//IsSameRow should return DB_E_DELETEDROW REMOVEDELETED is FALSE
			//but so is g_fOWNUPDATEDELETE so deltes are not seen
			if(!CHECK(m_pIRowsetIdentity->IsSameRow(*pHRowSecond, *pHRowSecond),S_OK))
			{
				goto END;
			}
			//binary comparision 
			if(!CompareHandlesByLiteral(*pHRowSecond, *pHRowSecond,FALSE,TRUE))
			{
				goto END;
			}
		}
	}
	fTestPass=TRUE;
END:
	//undo the deletions
	if(pHRow)
	{
		CHECK(ReleaseRows(1, &pHRow,TRUE),S_OK);
	}

	if(pHRowSecond)
		CHECK(ReleaseRows(1, &pHRowSecond,TRUE),S_OK);

	if(pDBRowStatus)
		PROVIDER_FREE(pDBRowStatus);

	if(fTestPass)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Hard deleted the last row handle.  Fetch the same row again backwards.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Keyset_Query_Immediate::Variation_5()
{
	HROW		rgHRow[50];
	HROW		*pHRow			= NULL;
	HROW		*pHRowSecond	= NULL;
	DBCOUNTITEM	cRows			= 0;
	BOOL		fTestPass		= FALSE;
	DBROWSTATUS	*pDBRowStatus	= NULL;
	BOOL		fPending		= TRUE;
	HRESULT		hr;
	DWORD		count			= 0;
	DWORD		i				= 0;

	//Restart the position to the begining of the rowset
	hr	= m_pIRowset->RestartPosition(NULL);
	if (S_OK!=hr && DB_S_COMMANDREEXECUTED!=hr)
	{
		goto END;
	}

	//Get the row handle at the end of the rowset by looping through the rowset till ENDOFROWSET
	pHRow=&rgHRow[count];
	while (S_OK==(hr = m_pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRow))	||	DB_S_ENDOFROWSET==hr)
	{				
		if (DB_S_ENDOFROWSET==hr)
		{
			if( cRows ==0)
			{
				break;
			}
			else
			{
				goto END;
			}
		}
		pHRow=&rgHRow[++count];
	}

	//release and free all but the last
	pHRow=&rgHRow[0];
	CHECK(ReleaseRows((count-1), &pHRow,FALSE),S_OK);
		
	//set the pointer to the last row handle
	pHRow=&rgHRow[count-1];

	//hard delete the row
	if(!CHECK(DeleteRows(1,pHRow),S_OK))
		goto END;

	//get the same row again 
	if(!CHECK(m_pIRowset->GetNextRows(NULL,0,-1,&cRows,&pHRowSecond),S_OK))
	{
		goto END;
	}

	//IsSameRow should return DB_E_DELETEDROW because pHRow is being compared
	if(!CHECK(m_pIRowsetIdentity->IsSameRow(*pHRow, *pHRowSecond),DB_E_DELETEDROW))
	{
		goto END;
	}

	//if test can see it own deletes these should compare
	if (g_fOWNUPDATEDELETE)
	{
		//binary comparision
		if(!CompareHandlesByLiteral(*pHRow, *pHRowSecond,FALSE,TRUE))
		{
			goto END;
		}
	}
	else
	{
		//binary comparision
		if(CompareHandlesByLiteral(*pHRow, *pHRowSecond,FALSE,TRUE))
		{
			goto END;
		}
	}

	if (g_fREMOVEDELETED)
	{
		//IsSameRow should return S_OK - REMOVEDELETED is TRUE
		if(!CHECK(m_pIRowsetIdentity->IsSameRow(*pHRowSecond, *pHRowSecond),S_OK))
		{
			goto END;
		}
		//binary comparision
		if(!CompareHandlesByLiteral(*pHRowSecond, *pHRowSecond,FALSE,TRUE))
		{
			goto END;
		}
	}
	else
	{
		//if test can see it own deletes
		if (g_fOWNUPDATEDELETE)
		{
			//IsSameRow should return DB_E_DELETEDROW REMOVEDELETED is FALSE
			if(!CHECK(m_pIRowsetIdentity->IsSameRow(*pHRowSecond, *pHRowSecond),DB_E_DELETEDROW))
			{
				goto END;
			}
			//binary comparision - because a handle to a deleted row should compare with itself
			if(!CompareHandlesByLiteral(*pHRowSecond, *pHRowSecond,FALSE,TRUE))
			{
				goto END;
			}
		}
		else
		{
			//IsSameRow should return DB_E_DELETEDROW REMOVEDELETED is FALSE
			//but so is g_fOWNUPDATEDELETE so deltes are not seen
			if(!CHECK(m_pIRowsetIdentity->IsSameRow(*pHRowSecond, *pHRowSecond),S_OK))
			{
				goto END;
			}
			//binary comparision 
			if(!CompareHandlesByLiteral(*pHRowSecond, *pHRowSecond,FALSE,TRUE))
			{
				goto END;
			}
		}
	}
	fTestPass=TRUE;
END:
	//undo the deletions
	if(pHRow)
	{
		CHECK(ReleaseRows(1, &pHRow,FALSE),S_OK);
	}

	if(pHRowSecond)
		CHECK(ReleaseRows(1, &pHRowSecond,TRUE),S_OK);

	if(pDBRowStatus)
		PROVIDER_FREE(pDBRowStatus);

	if(fTestPass)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Hard deleted the last row handle.  Fetch the same row again forwards.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Keyset_Query_Immediate::Variation_6()
{
	HROW		*pHRow			= NULL;
	HROW		*pHRowSecond	= NULL;
	DBCOUNTITEM	cRows			= 0;
	BOOL		fTestPass		= FALSE;
	DBROWSTATUS	*pDBRowStatus	= NULL;
	BOOL		fPending		= TRUE;
	HRESULT		hr;
	DWORD		count			= 0;
	HROW		rgHRow[50];
	
	//Restart the position to the begining of the rowset
	hr	= m_pIRowset->RestartPosition(NULL);
	if (S_OK!=hr && DB_S_COMMANDREEXECUTED!=hr)
	{
		goto END;
	}

	//Get the row handle at the end of the rowset by looping through the rowset till ENDOFROWSET
	pHRow=&rgHRow[count];
	while (S_OK==(hr = m_pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRow))	||	DB_S_ENDOFROWSET==hr)
	{				
		if (DB_S_ENDOFROWSET==hr)
		{
			if( cRows ==0)
			{
				break;
			}
			else
			{
				goto END;
			}
		}
		pHRow=&rgHRow[++count];
	}

	//release all but the last
	pHRow=&rgHRow[0];
	CHECK(ReleaseRows((count-1), &pHRow,FALSE),S_OK);
		
	//set the pointer to the last row handle
	pHRow=&rgHRow[count-1];

	//hard delete the row, it may be gone from variation 5 (if so ok it just needs
	//to be deleted
	hr=DeleteRows(1,pHRow);
	if (hr != S_OK && hr!= DB_E_ERRORSOCCURRED)
	{
		goto END;
	}
	//get the same row again 
	if(!CHECK(m_pIRowset->GetNextRows(NULL,-1,1,&cRows,&pHRowSecond),S_OK))
		goto END;

	//IsSameRow should return DB_E_DELETEDROW because pHRow is being compared
	if(!CHECK(m_pIRowsetIdentity->IsSameRow(*pHRow, *pHRowSecond),DB_E_DELETEDROW))
	{
		goto END;
	}

	//if test can see it own deletes these should compare
	if (g_fOWNUPDATEDELETE)
	{
		//binary comparision
		if(!CompareHandlesByLiteral(*pHRow, *pHRowSecond,FALSE,TRUE))
		{
			goto END;
		}
	}
	else
	{
		//binary comparision
		if(CompareHandlesByLiteral(*pHRow, *pHRowSecond,FALSE,TRUE))
		{
			goto END;
		}
	}

	if (g_fREMOVEDELETED)
	{
		//IsSameRow should return S_OK - REMOVEDELETED is TRUE
		if(!CHECK(m_pIRowsetIdentity->IsSameRow(*pHRowSecond, *pHRowSecond),S_OK))
		{
			goto END;
		}
		//binary comparision
		if(!CompareHandlesByLiteral(*pHRowSecond, *pHRowSecond,FALSE,TRUE))
		{
			goto END;
		}
	}
	else
	{
		//if test can see it own deletes
		if (g_fOWNUPDATEDELETE)
		{
			//IsSameRow should return DB_E_DELETEDROW REMOVEDELETED is FALSE
			if(!CHECK(m_pIRowsetIdentity->IsSameRow(*pHRowSecond, *pHRowSecond),DB_E_DELETEDROW))
			{
				goto END;
			}
			//binary comparision - because a handle to a deleted row should compare with itself
			if(!CompareHandlesByLiteral(*pHRowSecond, *pHRowSecond,FALSE,TRUE))
			{
				goto END;
			}
		}
		else
		{
			//IsSameRow should return DB_E_DELETEDROW REMOVEDELETED is FALSE
			//but so is g_fOWNUPDATEDELETE so deltes are not seen
			if(!CHECK(m_pIRowsetIdentity->IsSameRow(*pHRowSecond, *pHRowSecond),S_OK))
			{
				goto END;
			}
			//binary comparision 
			if(!CompareHandlesByLiteral(*pHRowSecond, *pHRowSecond,FALSE,TRUE))
			{
				goto END;
			}
		}
	}
	fTestPass=TRUE;
END:
	//undo the deletions
	if(pHRow)
	{
		CHECK(ReleaseRows(1, &pHRow,FALSE),S_OK);
	}

	if(pHRowSecond)
		CHECK(ReleaseRows(1, &pHRowSecond,TRUE),S_OK);

	if(pDBRowStatus)
		PROVIDER_FREE(pDBRowStatus);

	if(fTestPass)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// }}
// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Keyset_Query_Immediate::Terminate()
{
	ReleaseRowsetAndAccessor();

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIRowsetIdentity::Terminate());
}	// }}
// }}


// {{ TCW_TC_PROTOTYPE(Keyset_Cursor_Buffered)
//*-----------------------------------------------------------------------
//|	Test Case:		Keyset_Cursor_Buffered - Keyset_Cursor_Buffered
//|	Created:			05/28/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Keyset_Cursor_Buffered::Init()
{
	DBPROPID	rgGuid[9];
	BOOL		fCursorNotSupported = FALSE;
	DBPROPID	rgGuidUnset[2];
	ULONG		cProp				= 0;
	ULONG		cPropUnset			= 0;

	rgGuidUnset[0]=DBPROP_OTHERINSERT;
	rgGuidUnset[1]=DBPROP_REMOVEDELETED;
	cPropUnset=1;

	rgGuid[0]=DBPROP_IRowsetIdentity;
	rgGuid[1]=DBPROP_IRowsetChange;
	rgGuid[2]=DBPROP_IRowsetLocate;
	rgGuid[3]=DBPROP_CANHOLDROWS;
	rgGuid[4]=DBPROP_CANSCROLLBACKWARDS;
	rgGuid[5]=DBPROP_OTHERUPDATEDELETE;
	rgGuid[6]=DBPROP_IRowsetUpdate;
	rgGuid[7]=DBPROP_OWNINSERT;
	rgGuid[8]=DBPROP_OWNUPDATEDELETE;
	cProp=9;

	if(!TCIRowsetIdentity::Init())
		return FALSE;

	//check to see if the execute is going to fail
	if( !g_rgDBPrpt[IDX_ScrollBackwards].fSupported ||
		!g_rgDBPrpt[IDX_CanHoldRows].fSupported ||
		!g_rgDBPrpt[IDX_OtherUpdateDelete].fSupported )
	{
		fCursorNotSupported = TRUE;
		odtLog << L"The Provider does not support a KEYSET Cursor.\n";
	}

	//get a rowset with IRowsetIdentity, with keyset driven Cursor
	if(!GetRowsetAndAccessor(USE_OPENROWSET, cProp, rgGuid,cPropUnset,rgGuidUnset,ON_ROWSET_ACCESSOR))
	{
		return TEST_SKIPPED;
	}
	if (!g_fTestValid)
	{
		return TRUE;
	}
	
	//has to be keyset cursor
	if(GetCursorType()==KEYSET_DRIVEN_CURSOR)
	{
		odtLog << L"This test will continue but a keyset cursor was not obtained.\n";	//	return FALSE;
	}

	return TRUE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Fetch one row.  Move the cursor and fetch backwards to retrieve the same row handle by 
//			IRowsetLocate::GetRowsByBookmark.  S_OK.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Keyset_Cursor_Buffered::Variation_1()
{
	HROW		*pHRow			=NULL;
	HROW		*pHRowSecond	=NULL;
	DBCOUNTITEM	cRows			=0;
	DBBOOKMARK	DBBookmark		=DBBMK_FIRST;
	BYTE		*pBookmark		=(BYTE *)&DBBookmark;
	BOOL		fTestPass		=FALSE;

	//if this test variation is not valid
	if (!g_fTestValid)
	{
		odtLog << L"The Provider does not support the properties that are necessary for this variation to execute.\n";
        return TEST_PASS;
	}
	
	//get the 4th row handle
	if(!CHECK(m_pIRowset->GetNextRows(NULL,3,1,&cRows,&pHRow),S_OK))
		goto END;

	//get the same row again by IRowsetLocate::GetRowsAt
	if(!CHECK(m_pIRowsetLocate->GetRowsAt(NULL,NULL,1,pBookmark,3,-1, &cRows,&pHRowSecond),S_OK))
		goto END;

	//IsSameRow should return S_OK
	if(!CHECK(m_pIRowsetIdentity->IsSameRow(*pHRow, *pHRowSecond),S_OK))
	{
		goto END;
	}
	//binary comparision
	if(CompareHandlesByLiteral(*pHRow, *pHRowSecond,FALSE,TRUE))
	{
		fTestPass = TRUE;
	}
END:
	//release the row
	if(pHRow)
		CHECK(ReleaseRows(1, &pHRow,TRUE),S_OK);

	if(pHRowSecond)
		CHECK(ReleaseRows(1, &pHRowSecond,TRUE),S_OK);

	if(fTestPass)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Soft deleted the 1st row handle.  Fetch the row handle again.  S_OK.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Keyset_Cursor_Buffered::Variation_2()
{
	HROW		*pHRow			= NULL;
	HROW		HRowSecond		= DB_NULL_HROW;
	DBCOUNTITEM	cRows			= 0;
	DBCOUNTITEM	cbBookmark		= 0;
	BYTE		*pBookmark		= NULL;
	BOOL		fTestPass		= FALSE;
	DBROWSTATUS	DBRowStatus[1];
	DBROWSTATUS	*pDBRowStatus	= NULL;
	HRESULT		hr;

	//if this test variation is not valid
	if (!g_fTestValid)
	{
		odtLog << L"The Provider does not support the properties that are necessary for this variation to execute.\n";
        return TEST_PASS;
	}
	//if IRowsetChange not available because conformance won't allow it
	//For read only provider or IRowsetChange not available, skip this variation
	if(g_fReadOnlyProvider  || !(g_fROWSETUPDATABLE))
	{
		odtLog << L"Can not get to IRowsetChange, can not insert a row.\n";
		return TEST_PASS;
	}
	
	//If the provider is read only, skip the variation
	if(g_fReadOnlyProvider)
	{
		return TEST_PASS;
	}
	//Restart the position to the begining of the rowset
	hr	= m_pIRowset->RestartPosition(NULL);
	if (S_OK!=hr && DB_S_COMMANDREEXECUTED!=hr)
	{
		goto END;
	}
	
	//get a row handle
	if(!CHECK(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow),S_OK))
	{
		goto END;
	}

	//get the bookmark for the row handle
	if(!GetBookmark(1, &cbBookmark, &pBookmark))
	{
		goto END;
	}
	//soft delete the row
	if(!CHECK(DeleteRows(1,pHRow),S_OK))
	{
		goto END;
	}

	//determine through props which row is fetched and check it
	
	//is the deleted row removed
	if (g_fOWNUPDATEDELETE&&g_fLITERALIDENITIY&&!g_fREMOVEDELETED)
	{
		//get the same row again by IRowsetLocate::GetRowsByBookmark
		if(!CHECK(m_pIRowsetLocate->GetRowsByBookmark(NULL,1,&cbBookmark,
			(const BYTE **)&pBookmark,&HRowSecond,&DBRowStatus[0]),DB_E_ERRORSOCCURRED))
		{	
			goto END;
		}
		COMPARE(DBRowStatus[0],DBROWSTATUS_E_INVALID);
	}
	else
	{
		//get the same row again by IRowsetLocate::GetRowsByBookmark
		if(!CHECK(m_pIRowsetLocate->GetRowsByBookmark(NULL,1,&cbBookmark,
			(const BYTE **)&pBookmark,&HRowSecond,&DBRowStatus[0]),S_OK))
		{	
			goto END;
		}
		COMPARE(DBRowStatus[0],DBROWSTATUS_S_OK);

		//is the deleted row present
		if (g_fOWNUPDATEDELETE)
		{
			//IsSameRow should return S_OK, should not see DB_E_DELTEDROW because
			//pending deleted row has not been transmitted to the back end as of yet
			if(!CHECK(m_pIRowsetIdentity->IsSameRow(*pHRow, HRowSecond),S_OK))
			{
				goto END;
			}
			//binary comparision
			if(CompareHandlesByLiteral(*pHRow, HRowSecond,FALSE,TRUE))
			{
				fTestPass = TRUE;
			}
		}
		else
		{
			//IsSameRow should return S_OK
			if(!CHECK(m_pIRowsetIdentity->IsSameRow(*pHRow, HRowSecond),S_OK))
			{
				goto END;
			}
			//binary comparision
			if(CompareHandlesByLiteral(*pHRow, HRowSecond,FALSE,TRUE))
			{
				fTestPass = TRUE;
			}
		}

		//Update the data, deletes are no longer pending
		if(!CHECK(UpdateRows(1,pHRow,&pDBRowStatus),S_OK))
		{
			goto END;
		}

		//IsSameRow should return DB_E_DELETEDROW because pHRow1 points to a xmitted deleted
		//row 
		if(!CHECK(m_pIRowsetIdentity->IsSameRow(*pHRow, HRowSecond),DB_E_DELETEDROW))
		{
			goto END;
		}
		//binary comparision, these rows can't possibly compare literally
		if(!CompareHandlesByLiteral(*pHRow, HRowSecond,FALSE,FALSE))
		{
			goto END;
		}

		if (g_fREMOVEDELETED)
		{
			//IsSameRow should return S_OK because the deleted row is removed
			if(!CHECK(m_pIRowsetIdentity->IsSameRow(HRowSecond, HRowSecond),S_OK))
			{
				goto END;
			}
			//binary comparision, these rows can compare literally
			if(!CompareHandlesByLiteral(HRowSecond, HRowSecond,FALSE,TRUE))
			{
				goto END;
			}
		}
		else
		{
			//IsSameRow should return DB_E_DELETEDROW the deleted row is not removed which is
			//pointed to by pHRow1AfterDel
			if(!CHECK(m_pIRowsetIdentity->IsSameRow(HRowSecond, HRowSecond),DB_E_DELETEDROW))
			{
				goto END;
			}
			//binary comparision
			if(!CompareHandlesByLiteral(HRowSecond, HRowSecond,FALSE,FALSE))
			{
				goto END;
			}
		}
	}
	fTestPass=TRUE;
END:
	//undo the deletions
	if(pHRow)
		CHECK(UndoRows(1, pHRow),S_OK);

	if(pHRow)
		CHECK(ReleaseRows(1, &pHRow,TRUE),S_OK);

	if(HRowSecond!=DB_NULL_HROW)
		CHECK(m_pIRowset->ReleaseRows(1, &HRowSecond, NULL, NULL, NULL),S_OK);

	//release the bookmark
	if(pBookmark)
		PROVIDER_FREE(pBookmark);

	if(pDBRowStatus)
		PROVIDER_FREE(pDBRowStatus);

	if(fTestPass)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Fetch hRowThis and change a non-key value.  Fetch the same row in hRowThat.  
//		IsSameRow should return S_OK.  
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Keyset_Cursor_Buffered::Variation_3()
{
	void		*pData1			= NULL;
	void		*pData2			= NULL;
	HROW		*pHRowThis		= NULL;
	HROW		*pHRowThat		= NULL;
	DBCOUNTITEM	cRows			= 0;
	BOOL		fTestPass		= FALSE;
	HRESULT		hr;
	
	//get an accessor
	if(!GetAccessorOnRowset(ON_ROWSET_ACCESSOR,
		DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,
		UPDATEABLE_COLS_BOUND))
		goto END;

	//Restart the position to the begining of the rowset
	hr	= m_pIRowset->RestartPosition(NULL);
	if (S_OK!=hr && DB_S_COMMANDREEXECUTED!=hr)
	{
		goto END;
	}
	
	//get a row handle
	if(!CHECK(m_pIRowset->GetNextRows(NULL,9,1,&cRows,&pHRowThis),S_OK))
		goto END;

	//get the data for the row
	if(!CHECK(m_pIRowset->GetData(*pHRowThis, m_hAccessor, m_pData),S_OK))
		goto END;

	//change it
	if(!CHECK(ChangeOneRow(*pHRowThis, g_ulNextRow++, &pData1),S_OK))
		goto END;
	
	//get the row handle again
	if(!CHECK(m_pIRowset->GetNextRows(NULL,-1,1,&cRows,&pHRowThat),S_OK))
		goto END;

	//IsSameRow should return S_OK
	if(!CHECK(m_pIRowsetIdentity->IsSameRow(*pHRowThis, *pHRowThat),S_OK))
	{
		goto END;
	}
	//binary comparision
	if(!CompareHandlesByLiteral(*pHRowThis, *pHRowThat,FALSE,TRUE))
	{
		goto END;
	}

	//will this rowset see its pending changes
	if (g_fLITERALIDENITIY&&g_fOWNUPDATEDELETE)
	{
		//GetData on pHRowThat, should return the updated data buffer
		if(!COMPARE(CheckExpectedData(*pHRowThat, pData1, TRUE),TRUE))
			goto END;
	}	
	else
	{
		//GetData on pHRowThat, should return the first data because
		//LI is false (pending changes are not seen) 
		//the old buffer should be here
		if(!COMPARE(CheckExpectedData(*pHRowThat, m_pData, TRUE),TRUE))
			goto END;
	}

	//change data by pHRowThat
	if(!CHECK(ChangeOneRow(*pHRowThat, g_ulNextRow++, &pData2),S_OK))
		goto END;

	//this rowset will see its pending change only if DBPROP_LITERALIDENTITY is TRUE  AND OWNUPDEL is TRUE
	//if LI is FALSE pending changes are not seen
	if (g_fLITERALIDENITIY&&g_fOWNUPDATEDELETE)
	{
		if(!COMPARE(CheckExpectedData(*pHRowThis, pData2, TRUE),TRUE))
			goto END;
	}
	else
	{
		if(!COMPARE(CheckExpectedData(*pHRowThis, pData1, TRUE),TRUE))
			goto END;
	}

	//Undo hRowThat
	//same rules apply for Undo as for visibility of pending changes
	//if LI is TRUE test just undid all changes
	//eles it just undid changes on hRowThat
	if(!CHECK(UndoRows(1,pHRowThat),S_OK))
		goto END;

	//this rowset will see its pending change only if DBPROP_LITERALIDENTITY is TRUE AND OWNUPDEL is TRUE
	//if LI is FALSE pending changes are not seen
	if (g_fLITERALIDENITIY&&g_fOWNUPDATEDELETE)
	{
		//GetData on pHRowThis should return its first change
		if(!COMPARE(CheckExpectedData(*pHRowThis, m_pData, TRUE),TRUE))
			goto END;
	}
	else
	{
		//GetData on pHRowThis should return its first change
		if(!COMPARE(CheckExpectedData(*pHRowThis, pData1, TRUE),TRUE))
			goto END;
	}

	//release the one row handle(this)
	if(pHRowThis)
		CHECK(ReleaseRows(1,&pHRowThis,TRUE),S_OK);

	//get the row handle again.  
	if(!CHECK(m_pIRowset->GetNextRows(NULL,-1,1,&cRows,&pHRowThis),S_OK))
		goto END;

	//IsSameRow should return S_OK (the row will always be the row, just the data might be chaged)
	if(!CHECK(m_pIRowsetIdentity->IsSameRow(*pHRowThis, *pHRowThat),S_OK))
	{
		goto END;
	}
	//binary comparision
	if(!CompareHandlesByLiteral(*pHRowThis, *pHRowThat,FALSE,TRUE))
	{
		goto END;
	}

	//this rowset will see its pending change only if DBPROP_LITERALIDENTITY is TRUE AND OWNUPDEL is TRUE
	//if LI is FALSE pending changes are not seen
	if (g_fLITERALIDENITIY&&g_fOWNUPDATEDELETE)
	{
		//GetData on pHRowThat should return row after undo
		if(!COMPARE(CheckExpectedData(*pHRowThat, m_pData, TRUE),TRUE))
			goto END;
	}
	else
	{
		//GetData on pHRowThat should return the row's first change
		if(!COMPARE(CheckExpectedData(*pHRowThat, pData2, TRUE),TRUE))
			goto END;
	}

	fTestPass=TRUE;
END:
	if(m_pData)
 	  ReleaseInputBindingsMemory(m_cBinding, m_rgBinding,(BYTE *)m_pData,FALSE);

	if(pData1)
 	  ReleaseInputBindingsMemory(m_cBinding, m_rgBinding,(BYTE *)pData1,FALSE);

	if(pData2)
	  ReleaseInputBindingsMemory(m_cBinding, m_rgBinding,(BYTE *)pData2,TRUE);

	if(pHRowThis)
		CHECK(ReleaseRows(1,&pHRowThis,TRUE),S_OK);

 	if(pHRowThat)
		CHECK(ReleaseRows(1,&pHRowThat,TRUE),S_OK);

	if(fTestPass)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Insert a new row into the rowset. Undo  
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Keyset_Cursor_Buffered::Variation_4()
{
	HROW		HRow		= NULL;
	HROW		*pHRow		= NULL;
	DBCOUNTITEM	cRows		= 0;
	BYTE		*pData		= NULL;
	BOOL		fTestPass	= FALSE;
	HRESULT		hr;
	BOOL		fFound		= FALSE;

	//Restart the position to the begining of the rowset
	hr	= m_pIRowset->RestartPosition(NULL);
	if (S_OK!=hr && DB_S_COMMANDREEXECUTED!=hr)
	{
		goto END;
	}

	//Create an accessor
	if(!GetAccessorOnRowset(ON_ROWSET_ACCESSOR,
		DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,					
		UPDATEABLE_COLS_BOUND))
		goto END;

	if(!CHECK(InsertOneRow(g_ulNextRow, &HRow),S_OK))
		goto END;

	//if can't see this insert then skip 
	if (g_fRETURNPENDINGINSERTS&&g_fOWNINSERT&&g_fLITERALIDENITIY)
	{
		if(!(pData=(BYTE*)PROVIDER_ALLOC(m_cRowSize)))
			goto END;
		//get a data buffer with the inserted data
		if(!CHECK(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
						&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK))
		{
			goto END;
		}

		//Get a row handle
		while (S_OK==(hr = m_pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRow)) || hr == DB_S_ENDOFROWSET )
		{
			if( cRows ==0)
				break;
			//Get the data for the ith row handle
			if(!CHECK(m_pIRowset->GetData(*pHRow,m_hAccessor,m_pData),S_OK))
			{
				goto END;
			}

			//make sure GetData should be able to see the change
			if(CompareBuffer(m_pData,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
			{
				fFound=TRUE;
				break;
			}
			if(!CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK))
			{
				goto END;
			}
			PROVIDER_FREE(pHRow);
		}		
		
		//the inserted row has to be found
		if (!fFound)
		{
			goto END;
		}

		//IsSameRow should return DB_E_NEWLYINSERTED if DBPROP_STRONGIDENTITY 
		//is VARIANT_FALSE only after the data has been transmitted to the back end
		//S_OK should be returned
		if(!CHECK(m_pIRowsetIdentity->IsSameRow(HRow, *pHRow),S_OK))
		{
			goto END;
		}
		//binary comparision
		if(!CompareHandlesByLiteral(HRow, *pHRow,FALSE,TRUE))
		{
			goto END;
		}

		//Undo the insertion
		if(!CHECK(UndoRows(1,&HRow),S_OK))
			goto END;

		//IsSameRow should return DB_E_DELETEDROW, pending inserted rows undone are like
		//deleted rows
		if(!CHECK(m_pIRowsetIdentity->IsSameRow(HRow, *pHRow),DB_E_DELETEDROW))
		{
			goto END;
		}
		//binary comparision
		if(!CompareHandlesByLiteral(HRow, *pHRow,FALSE,TRUE))
		{
			goto END;
		}

		//Get Data should return DB_E_DELETEDROW
		if(CHECK(m_pIRowset->GetData(HRow,m_hAccessor,pData),DB_E_DELETEDROW))
		{
			fTestPass=TRUE;
		}
	}
	else
	{
		fTestPass=TEST_SKIPPED;
	}
END:
	if(HRow)
		CHECK(m_pIRowset->ReleaseRows(1,&HRow, NULL, NULL, NULL),S_OK);

	if(pHRow)
		CHECK(ReleaseRows(1, &pHRow, TRUE),S_OK);

	if(pData)
		PROVIDER_FREE(pData);

	if(fTestPass)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Fetch hRowThis and soft change the key of the row.  
//		The row should be deleted and a new row is appended at the end of rowset.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Keyset_Cursor_Buffered::Variation_5()
{
	HROW		*pHRowLast1		= NULL;
	HROW		*pHRowLast2		= NULL;
	HROW		*pHRow1			= NULL;
	HROW		*pHRow2			= NULL;
	HROW		*pHRow3			= NULL;
	HROW		*pHRowNext		= NULL;
	DBCOUNTITEM	cRows			= 0;
	void		*pData2			= NULL;
	HRESULT		hr;
	DBBOOKMARK	DBBkmrkLST		= DBBMK_LAST;
	BYTE		*pBkmrkLST		= (BYTE *)&DBBkmrkLST;
	DBBOOKMARK	DBBkmrkFRST		= DBBMK_FIRST;
	BYTE		*pBkmrkFRST		= (BYTE *)&DBBkmrkFRST;
	BOOL		fTestPass		= FALSE;
	BOOL		fNewRow			= FALSE;	
	ULONG		cColsNumber		= 0;
	ULONG		cColsCount		= 0;

	//if this test variation is not valid
	if (!g_fTestValid)
	{
		odtLog << L"The Provider does not support the properties that are necessary for this variation to execute.\n";
        return TEST_PASS;
	}

	//Get the row handle at the end of the rowset
	if(!CHECK(m_pIRowsetLocate->GetRowsAt(NULL,NULL,1,pBkmrkLST,0,1,&cRows, &pHRowLast1),S_OK))
	{
		goto END;
	}	

	//If the provider is read only, skip the variation
	if(g_fReadOnlyProvider)
		return TEST_PASS;

	//Restart the position to the begining of the rowset
	hr	= m_pIRowset->RestartPosition(NULL);
	if (S_OK!=hr && DB_S_COMMANDREEXECUTED!=hr)
	{
		goto END;
	}

	//Create an accessor
	if(!GetAccessorOnRowset(ON_ROWSET_ACCESSOR,
		DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,					
		UPDATEABLE_COLS_BOUND))
		goto END;

	//get a row handle
	if(!CHECK(m_pIRowset->GetNextRows(NULL,1,1,&cRows,&pHRow1),S_OK))
	{
		goto END;
	}

	//Restart the position to the begining of the rowset
	hr	= m_pIRowset->RestartPosition(NULL);
	if (S_OK!=hr && DB_S_COMMANDREEXECUTED!=hr)
	{
		goto END;
	}

	//get the 2nd row handle again
	if(!CHECK(m_pIRowset->GetNextRows(NULL,1,1,&cRows,&pHRow2),S_OK))
	{
		goto END;
	}

	//get a third row handle
	if(!CHECK(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRowNext),S_OK))
	{
		goto END;
	}

	//these should be the same rows
	if(!CHECK(m_pIRowsetIdentity->IsSameRow(*pHRow2, *pHRow1),S_OK))
	{
		goto END;
	}
	//binary comparision
	if(!CompareHandlesByLiteral(*pHRow2, *pHRow1,FALSE,TRUE))
	{
		goto END;
	}

	//change the row through the 2nd row handle
	if(!CHECK(ChangeOneRow(*pHRow2,g_ulNextRow++),S_OK))
	{
		goto END;
	}
	//Update the change, xmit it to the back end
	if(!CHECK(UpdateRows(1,pHRow2),S_OK))
	{
		goto END;
	}
	//these should still be the same rows
	if(!CHECK(m_pIRowsetIdentity->IsSameRow(*pHRow2, *pHRow1),S_OK))
	{
		goto END;
	}
	//binary comparision
	if(!CompareHandlesByLiteral(*pHRow2, *pHRow1,FALSE,TRUE))
	{
		goto END;
	}

	//Restart the position to the begining of the rowset
	hr	= m_pIRowset->RestartPosition(NULL);
	if (S_OK!=hr && DB_S_COMMANDREEXECUTED!=hr)
	{
		goto END;
	}
	
	//Get the row handle at the end of the rowset
	if(!CHECK(m_pIRowsetLocate->GetRowsAt(NULL,NULL,1,pBkmrkLST,0,1,&cRows, &pHRowLast2),S_OK))
	{
		goto END;
	}

	//if the row at the end of the rowset is not the previous row at the end of the rowset 
	//and newly inserted rows are visibile then
	//the new row (pHRow2) is inserted at the end
	hr=m_pIRowsetIdentity->IsSameRow(*pHRowLast1, *pHRowLast2);
	if(hr!=S_OK&&g_fOWNINSERT)
	{
		//IsSameRow  *MIGHT*  return DB_E_NEWLYINSERTED if DBPROP_STRONGIDENTITY is VARIANT_FALSE.
		//DBPROP_STRONGIDENTITY FALSE means newly inserted row can not be seen
		if(g_rgDBPrpt[IDX_StrongIdentity].fDefault==VARIANT_FALSE)
		{
			m_hr=m_pIRowsetIdentity->IsSameRow(*pHRow2, *pHRowLast2);
			if	(	m_hr!=ResultFromScode(S_FALSE)					&&
					m_hr!=ResultFromScode(DB_E_NEWLYINSERTED)
				)
			{
				goto END;
			}
		}
		//can compare newly inserted rows
		else
		{
			m_hr=m_pIRowsetIdentity->IsSameRow(*pHRow2, *pHRowLast2);
			//new row should not match deleted row
			if	(m_hr!=ResultFromScode(S_FALSE))
			{
				goto END;
			}
		}
		fNewRow=TRUE;
		//this also means the old row should have been deleted, Update was called so
		//see if GetData on the old row handle gets the data or returns DB_E_DELETEDROW.

		//S_OK is possible here.  even though the test did not delete the row (the provider might have)
		//so pHRow2 becomes invalid and provider behavior in this case is undefined, 
		//short of a GPF that is.
		if(!(pData2=PROVIDER_ALLOC(m_cRowSize)))
		{
			goto END;
		}
		m_hr=m_pIRowset->GetData(*pHRow2, m_hAccessor, pData2);
		if (DB_E_DELETEDROW	!= m_hr && 
			S_OK			!= m_hr)
		{
			goto END;
		}
	}
	else//if there is not a new row (at the end of the rowset*) the change should have occured in the same row
	//* this tests assumes the new row was placed at the end of the rowset or stayed put (**), odbc makes no promises where this row should be loacted
	//**this probally will stay put if the unique index is not the key in the keyset cursor from the driver becuase then the change will be an update and not an insert/delete
	{
		if(g_fOWNINSERT)
		{
			fNewRow=FALSE;

			//call IRowsetLocate to retrieve the change row
			if(!CHECK(m_pIRowsetLocate->GetRowsAt(NULL,NULL,1,pBkmrkFRST,1,1,&cRows, &pHRow3),S_OK))
			{
				goto END;
			}
			//IsSameRow should return S_OK
			if(!CHECK(m_pIRowsetIdentity->IsSameRow(*pHRow2, *pHRow3),S_OK))
			{
				goto END;
			}
			//binary comparision
			if(!CompareHandlesByLiteral(*pHRow2, *pHRow3,FALSE,TRUE))
			{
				goto END;
			}
		}
	}

	
	//Restart the position to the begining of the rowset
	hr	= m_pIRowset->RestartPosition(NULL);
	if (S_OK!=hr && DB_S_COMMANDREEXECUTED!=hr)
	{
		goto END;
	}

	//try to get the row again
	if(!CHECK(m_pIRowset->GetNextRows(NULL,1,1,&cRows,&pHRow3),S_OK))
		goto END;

	//if a new row was inserted at the end of the rowset there should be a
	//hole (deleted row) where the row used to be.
	if(fNewRow)
	{
		//can test see deletes
		if (g_fOWNUPDATEDELETE)
		{
			if (g_fREMOVEDELETED)
			{
				//deleted row is gone, so different row will compare as S_FALSE
				CHECK(m_pIRowsetIdentity->IsSameRow(*pHRow3, *pHRow2),S_FALSE);
				//binary comparision
				if(!CompareHandlesByLiteral(*pHRow3, *pHRow2,FALSE,FALSE))
				{
					goto END;
				}
			}
			else
			{
				//S_OK, both are the deleted row 
				CHECK(m_pIRowsetIdentity->IsSameRow(*pHRow3, *pHRow2),S_OK);
				//binary comparision
				if(!CompareHandlesByLiteral(*pHRow3, *pHRow2,FALSE,TRUE))
				{
					goto END;
				}
			}
		}
		else
		{
			//deletes are not visible so the test should see
			//the original row
			CHECK(m_pIRowsetIdentity->IsSameRow(*pHRow3, *pHRow2),S_OK);
			//binary comparision
			if(!CompareHandlesByLiteral(*pHRow3, *pHRow2,FALSE,TRUE))
			{
				goto END;
			}
		}
	}
	//row stayed the same and just the value changed
	else
	{
		//S_OK
		CHECK(m_pIRowsetIdentity->IsSameRow(*pHRow3, *pHRow2),S_OK);
		//binary comparision
		if(!CompareHandlesByLiteral(*pHRow3, *pHRow2,FALSE,TRUE))
		{
			goto END;
		}
	}
	
	//release the row handles
	CHECK(ReleaseRows(1, &pHRow3, TRUE),S_OK);
	pHRow3=NULL;

	CHECK(ReleaseRows(1, &pHRow2, TRUE),S_OK);
	pHRow2=NULL;

	CHECK(ReleaseRows(1, &pHRow1, TRUE),S_OK);
	pHRow1=NULL;

	//Restart the position to the begining of the rowset
	hr	= m_pIRowset->RestartPosition(NULL);
	if (S_OK!=hr && DB_S_COMMANDREEXECUTED!=hr)
	{
		goto END;
	}

	//try to get the row again
	if(!CHECK(m_pIRowset->GetNextRows(NULL,1,1,&cRows,&pHRow3),S_OK))
		goto END;

	//if a new row was inserted at the end of the rowset there should be a
	//hole where the row used to be.
	if(fNewRow)
	{
		//pHRow3 should be a deleted row
		if(!CHECK(m_pIRowsetIdentity->IsSameRow(*pHRow3, *pHRowNext),DB_E_DELETEDROW))
		{
			goto END;
		}
		//binary comparision
		if(!CompareHandlesByLiteral(*pHRow3, *pHRowNext,FALSE,FALSE))
		{
			goto END;
		}
		fTestPass=TRUE;
	}
	else
	{
		//these should not be the same rows
		if(!CHECK(m_pIRowsetIdentity->IsSameRow(*pHRow3, *pHRowNext),S_FALSE))
		{
			goto END;
		}
		//binary comparision
		if(!CompareHandlesByLiteral(*pHRow3, *pHRowNext,FALSE,FALSE))
		{
			goto END;
		}
		fTestPass=TRUE;
	}
END:
	if(pHRowLast1)
		CHECK(ReleaseRows(1, &pHRowLast1, TRUE),S_OK);

	if(pHRow2)
		CHECK(ReleaseRows(1, &pHRow2, TRUE),S_OK);

	if(pHRow3)
		CHECK(ReleaseRows(1, &pHRow3, TRUE),S_OK);

	if(pHRowNext)
		CHECK(ReleaseRows(1, &pHRowNext, TRUE),S_OK);

	if(pHRowLast2)
		CHECK(ReleaseRows(1, &pHRowLast2, TRUE),S_OK);

	if(pHRow1)
		CHECK(ReleaseRows(1, &pHRow1, TRUE),S_OK);

	if(pData2)
		PROVIDER_FREE(pData2);

	if(fTestPass)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}

// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Insert a new row into the rowset  Update
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Keyset_Cursor_Buffered::Variation_6()
{
	HROW		HRow		= NULL;
	HROW		*pHRow		= NULL;
	DBCOUNTITEM	cRows		= 0;
	BYTE		*pData		= NULL;
	BOOL		fTestPass	= FALSE;
	HRESULT		hr;
	BOOL		fFound		= FALSE;

	//Restart the position to the begining of the rowset
	hr	= m_pIRowset->RestartPosition(NULL);
	if (S_OK!=hr && DB_S_COMMANDREEXECUTED!=hr)
	{
		goto END;
	}

	//Create an accessor
	if(!GetAccessorOnRowset(ON_ROWSET_ACCESSOR,
		DBACCESSOR_ROWDATA,
		DBPART_VALUE|DBPART_STATUS|DBPART_LENGTH,					
		UPDATEABLE_COLS_BOUND))
		goto END;

	if(!CHECK(InsertOneRow(g_ulNextRow++, &HRow),S_OK))
		goto END;

	//if can't see this insert then skip 
	if (g_fRETURNPENDINGINSERTS&&g_fOWNINSERT&&g_fLITERALIDENITIY)
	{
		if(!(pData=(BYTE*)PROVIDER_ALLOC(m_cRowSize)))
			goto END;
		//get a data buffer with the inserted data
		if(!CHECK(FillInputBindings(m_pTable,DBACCESSOR_ROWDATA,m_cBinding,m_rgBinding,
						&pData,g_ulNextRow++,m_cRowsetCols,m_rgTableColOrds, PRIMARY),S_OK))
		{
			goto END;
		}

		//Get a row handle
		while (S_OK==(hr = m_pIRowset->GetNextRows(NULL,0,1,&cRows, &pHRow)) || hr == DB_S_ENDOFROWSET )
		{
			if( cRows ==0)
				break;
			//Get the data for the ith row handle
			if(!CHECK(m_pIRowset->GetData(*pHRow,m_hAccessor,m_pData),S_OK))
			{
				goto END;
			}

			//make sure GetData should be able to see the change
			if(CompareBuffer(m_pData,pData,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
			{
				fFound=TRUE;
				break;
			}
			if(!CHECK(m_pIRowset->ReleaseRows(1,pHRow,NULL,NULL,NULL),S_OK))
			{
				goto END;
			}
			PROVIDER_FREE(pHRow);
		}		
		
		//the inserted row has to be found
		if (!fFound)
		{
			goto END;
		}

		//IsSameRow should return DB_E_NEWLYINSERTED if DBPROP_STRONGIDENTITY 
		//is VARIANT_FALSE only after the data has been transmitted to the back end
		//S_OK should be returned
		if(!CHECK(m_pIRowsetIdentity->IsSameRow(HRow, *pHRow),S_OK))
		{
			goto END;
		}
		//binary comparision
		if(!CompareHandlesByLiteral(HRow, *pHRow,FALSE,TRUE))
		{
			goto END;
		}

		//Update the insertion to the back end
		if(!CHECK(UpdateRows(1,&HRow),S_OK))
		{
			goto END;
		}
		
		//IsSameRow should return DB_E_NEWLYINSERTED if DBPROP_STRONGIDENTITY 
		//is VARIANT_FALSE
		if(!g_fSTRONGIDENTITY)
		{
				m_hr=m_pIRowsetIdentity->IsSameRow(HRow, *pHRow);

				if(m_hr!=ResultFromScode(DB_E_NEWLYINSERTED))
				{
					goto END;
				}
				//binary comparision
				if(CompareHandlesByLiteral(HRow, *pHRow,FALSE,TRUE))
				{
					goto END;
				}
		}
		else
		//S_OK should be returned
		{
			if(!CHECK(m_pIRowsetIdentity->IsSameRow(HRow, *pHRow),S_OK))
			{
				goto END;
			}
			//binary comparision
			if(!CompareHandlesByLiteral(HRow, *pHRow,FALSE,TRUE))
			{
				goto END;
			}
		}
	}
	else
	{
		fTestPass=TEST_SKIPPED;
	}
END:
	if(HRow)
		CHECK(m_pIRowset->ReleaseRows(1,&HRow, NULL, NULL, NULL),S_OK);

	if(pHRow)
		CHECK(ReleaseRows(1, &pHRow, TRUE),S_OK);

	if(pData)
		PROVIDER_FREE(pData);

	if(fTestPass)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}

// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Soft deleted the 1st row handle.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Keyset_Cursor_Buffered::Variation_7()
{
	HROW		*pHRow0			= NULL;
	HROW		*pHRow1			= NULL;
	HROW		*pHRow2			= NULL;
	HROW		*pHRow1AfterDel	= NULL;
	DBCOUNTITEM	cRows			= 0;
	BOOL		fTestPass		= FALSE;
	DBROWSTATUS	*pDBRowStatus	= NULL;
	BOOL		fPending		= TRUE;
	void		*pData1			= NULL;
	void		*pData2			= NULL;
	HRESULT		hr;		

	if(!(pData1=(BYTE*)PROVIDER_ALLOC(m_cRowSize)))
			goto END;
	if(!(pData2=(BYTE*)PROVIDER_ALLOC(m_cRowSize)))
			goto END;
		
	//Restart the position to the begining of the rowset
	hr	= m_pIRowset->RestartPosition(NULL);
	if (S_OK!=hr && DB_S_COMMANDREEXECUTED!=hr)
	{
		goto END;
	}

	//get a row handle
	if(!CHECK(m_pIRowset->GetNextRows(NULL,3,1,&cRows,&pHRow0),S_OK))
		goto END;

	//get the next row handle
	if(!CHECK(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow1),S_OK))
		goto END;
	//get the data for the row
	if(!CHECK(m_pIRowset->GetData(*pHRow1, m_hAccessor, pData1),S_OK))
		goto END;

	//get the next row handle
	if(!CHECK(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow2),S_OK))
		goto END;

	//soft delete the 2nd row
	if(!CHECK(DeleteRows(1,pHRow1),S_OK))
		goto END;

	//try to get the 2nd row again, pending delete might or might not be there
	//if it is there this will fetch it, if it is not there this will fetch the first row
	if(!CHECK(m_pIRowset->GetNextRows(NULL,-2,1,&cRows,&pHRow1AfterDel),S_OK))
		goto END;

	//determine through props which row was fetched and check it

	//if test doesn't see its own deletes the first row should have been fetched
	if (!g_fOWNUPDATEDELETE)
	{
		//IsSameRow should return S_OK. 
		if(!CHECK(m_pIRowsetIdentity->IsSameRow(*pHRow1, *pHRow1AfterDel),S_OK))
		{
			goto END;
		}
		//binary comparision, 
		if(!CompareHandlesByLiteral(*pHRow1, *pHRow1AfterDel,FALSE,TRUE))
		{
			goto END;
		}
		//so look at buffer just for fun
		if(!CHECK(m_pIRowset->GetData(*pHRow1AfterDel, m_hAccessor, pData2),S_OK))
			goto END;
			
		//since OWNUPDATEDELETE is FALSE these do compare, delete is not seen
		if(!CompareBuffer(pData1,pData2,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
		{
			goto END;
		}
	}
	else
	{
		if (g_fLITERALIDENITIY&&g_fREMOVEDELETED)
		{
			//IsSameRow should return S_FALSE. 
			if(!CHECK(m_pIRowsetIdentity->IsSameRow(*pHRow1, *pHRow1AfterDel),S_FALSE))
			{
				goto END;
			}
			//binary comparision, 
			if(!CompareHandlesByLiteral(*pHRow1, *pHRow1AfterDel,FALSE,FALSE))
			{
				goto END;
			}
			//IsSameRow should return S_OK first row fetched and new fetch because the deleted row is removed
			if(!CHECK(m_pIRowsetIdentity->IsSameRow(*pHRow0, *pHRow1AfterDel),S_OK))
			{
				goto END;
			}
			//binary comparision, these rows can compare literally
			if(!CompareHandlesByLiteral(*pHRow0, *pHRow1AfterDel,FALSE,TRUE))
			{
				goto END;
			}
		}
		else
		{
			//IsSameRow should return DB_E_DELETEDROW the deleted row is not removed which is
			//pointed to by pHRow1AfterDel
			//IsSameRow should return S_OK. 
			if(!CHECK(m_pIRowsetIdentity->IsSameRow(*pHRow1, *pHRow1AfterDel),S_OK))
			{
				goto END;
			}
			//binary comparision, 
			if(!CompareHandlesByLiteral(*pHRow1, *pHRow1AfterDel,FALSE,TRUE))
			{
				goto END;
			}
		}
	}

	//Update the data, deletes are no longer pending
	if(!CHECK(UpdateRows(1,pHRow1,&pDBRowStatus),S_OK))
		goto END;

	//IsSameRow should return DB_E_DELETEDROW because pHRow1 points to a deleted
	//row no matter what DBPROP_REMOVEDELETED has for a default
	if(!CHECK(m_pIRowsetIdentity->IsSameRow(*pHRow1, *pHRow1AfterDel),DB_E_DELETEDROW))
	{
		goto END;
	}

	if (g_fREMOVEDELETED)
	{
		//IsSameRow should return S_OK because the deleted row is removed
		if(!CHECK(m_pIRowsetIdentity->IsSameRow(*pHRow0, *pHRow1AfterDel),S_OK))
		{
			goto END;
		}
		//binary comparision, these rows can compare literally
		if(!CompareHandlesByLiteral(*pHRow0, *pHRow1AfterDel,FALSE,TRUE))
		{
			goto END;
		}
	}
	else
	{
		//IsSameRow should return DB_E_DELETEDROW the deleted row is not removed which is
		//pointed to by pHRow1AfterDel
		if(!CHECK(m_pIRowsetIdentity->IsSameRow(*pHRow0, *pHRow1AfterDel),DB_E_DELETEDROW))
		{
			goto END;
		}
		//binary comparision
		if(!CompareHandlesByLiteral(*pHRow0, *pHRow1AfterDel,FALSE,FALSE))
		{
			goto END;
		}
	}
	fTestPass	= TRUE;
END:
	//undo the deletions
	if(pHRow0)
	{
		CHECK(ReleaseRows(1, &pHRow0,TRUE),S_OK);
	}
	//undo the deletions
	if(pHRow1)
	{
		CHECK(ReleaseRows(1, &pHRow1,TRUE),S_OK);
	}
	//undo the deletions
	if(pHRow2)
	{
		CHECK(ReleaseRows(1, &pHRow2,TRUE),S_OK);
	}

	if(pData1)
 	  ReleaseInputBindingsMemory(m_cBinding, m_rgBinding,(BYTE *)pData1,FALSE);

	if(pData2)
	  ReleaseInputBindingsMemory(m_cBinding, m_rgBinding,(BYTE *)pData2,TRUE);

	if(pHRow1AfterDel)
		CHECK(ReleaseRows(1, &pHRow1AfterDel,TRUE),S_OK);

	if(pDBRowStatus)
		PROVIDER_FREE(pDBRowStatus);

	if(fTestPass)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// }}
// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Keyset_Cursor_Buffered::Terminate()
{
	ReleaseRowsetAndAccessor();

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIRowsetIdentity::Terminate());
}	// }}
// }}


// {{ TCW_TC_PROTOTYPE(Dynamic_Query_Buffered)
//*-----------------------------------------------------------------------
//|	Test Case:		Dynamic_Query_Buffered - Dynamic_Query_Buffered
//|	Created:			05/28/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Dynamic_Query_Buffered::Init()
{
	DBPROPID	rgGuid[7];
	BOOL		fCursorNotSupported=FALSE;
	ULONG		cProp = g_MSDASQL?7:6;

	rgGuid[0]=DBPROP_IRowsetIdentity;
	rgGuid[1]=DBPROP_IRowsetChange;
	rgGuid[2]=DBPROP_OTHERINSERT;
	rgGuid[3]=DBPROP_OTHERUPDATEDELETE;
	rgGuid[4]=DBPROP_IRowsetUpdate;
	rgGuid[5]=DBPROP_CANHOLDROWS;
	rgGuid[6]=KAGPROP_QUERYBASEDUPDATES;

	if(!TCIRowsetIdentity::Init())
		return FALSE;

	//check to see if the execute is going to fail
	if( !g_rgDBPrpt[IDX_ScrollBackwards].fSupported ||
		!g_rgDBPrpt[IDX_CanHoldRows].fSupported ||
		!g_rgDBPrpt[IDX_OtherInsert].fSupported )
	{
		fCursorNotSupported = TRUE;
		odtLog << L"The Provider does not support a DYNAMIC Cursor.\n";
	}

	//get a rowset with IRowsetIdentity, with Dynamic Cursor
	if(!GetRowsetAndAccessor(USE_OPENROWSET, cProp, rgGuid,0,NULL,NO_ACCESSOR))
	{
		return TEST_SKIPPED;
	}
	if (!g_fTestValid)
	{
		return TRUE;
	}

	//has to be dynamic cursor
	if(GetCursorType()==DYNAMIC_CURSOR)
	{
		odtLog << L"This test will continue but a dynamic cursor was not obtained.\n";	//	return FALSE;
	}
	return TRUE;
}
// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Soft-delete one row.  S_FALSE.  Call IRowsetUpdate::Update.  DB_E_DELETEDROW.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Dynamic_Query_Buffered::Variation_1()
{
	HROW		*pHRow0			= NULL;
	HROW		*pHRow1			= NULL;
	HROW		*pHRow2			= NULL;
	HROW		*pHRow1AfterDel	= NULL;
	DBCOUNTITEM	cRows			= 0;
	BOOL		fTestPass		= FALSE;
	DBROWSTATUS	*pDBRowStatus	= NULL;
	BOOL		fPending		= TRUE;
	void		*pData1			= NULL;
	void		*pData2			= NULL;
		

	if(!(pData1=(BYTE*)PROVIDER_ALLOC(m_cRowSize)))
			goto END;
	if(!(pData2=(BYTE*)PROVIDER_ALLOC(m_cRowSize)))
			goto END;
		
	//get a row handle
	if(!CHECK(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow0),S_OK))
		goto END;

	//get the next row handle
	if(!CHECK(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow1),S_OK))
		goto END;
	//get the data for the row
	if(!CHECK(m_pIRowset->GetData(*pHRow1, m_hAccessor, pData1),S_OK))
		goto END;

	//get the next row handle
	if(!CHECK(m_pIRowset->GetNextRows(NULL,0,1,&cRows,&pHRow2),S_OK))
		goto END;

	//soft delete the 2nd row
	if(!CHECK(DeleteRows(1,pHRow1),S_OK))
		goto END;

	//try to get the 2nd row again, pending delete might or might not be there
	//if it is there this will fetch it, if it is not there this will fetch the first row
	if(!CHECK(m_pIRowset->GetNextRows(NULL,-2,1,&cRows,&pHRow1AfterDel),S_OK))
		goto END;

	//determine through props which row was fetched and check it

	//if test doesn't see its own deletes the first row should have been fetched
	if (!g_fOWNUPDATEDELETE)
	{
		//IsSameRow should return S_FALSE. 
		if(!CHECK(m_pIRowsetIdentity->IsSameRow(*pHRow1, *pHRow1AfterDel),S_FALSE))
		{
			goto END;
		}
		//binary comparision, 
		if(!CompareHandlesByLiteral(*pHRow1, *pHRow1AfterDel,FALSE,FALSE))
		{
			goto END;
		}
		//so look at buffer just for fun
		if(!CHECK(m_pIRowset->GetData(*pHRow1AfterDel, m_hAccessor, pData2),S_OK))
			goto END;
			
		//since OWNUPDATEDELETE is FALSE these do not compare, delete is not seen
		if(CompareBuffer(pData1,pData2,m_cBinding,m_rgBinding,m_pIMalloc,TRUE,FALSE,COMPARE_ONLY)==TRUE)
		{
			goto END;
		}
	}
	else
	{
		if (g_fLITERALIDENITIY&&g_fREMOVEDELETED)
		{
			//IsSameRow should return S_FALSE. 
			if(!CHECK(m_pIRowsetIdentity->IsSameRow(*pHRow1, *pHRow1AfterDel),S_FALSE))
			{
				goto END;
			}
			//binary comparision, 
			if(!CompareHandlesByLiteral(*pHRow1, *pHRow1AfterDel,FALSE,FALSE))
			{
				goto END;
			}
			//IsSameRow should return S_OK first row fetched and new fetch because the deleted row is removed
			if(!CHECK(m_pIRowsetIdentity->IsSameRow(*pHRow0, *pHRow1AfterDel),S_OK))
			{
				goto END;
			}
			//binary comparision, these rows can compare literally
			if(!CompareHandlesByLiteral(*pHRow0, *pHRow1AfterDel,FALSE,TRUE))
			{
				goto END;
			}
		}
		else
		{
			//IsSameRow should return DB_E_DELETEDROW the deleted row is not removed which is
			//pointed to by pHRow1AfterDel
			//IsSameRow should return S_OK. 
			if(!CHECK(m_pIRowsetIdentity->IsSameRow(*pHRow1, *pHRow1AfterDel),S_OK))
			{
				goto END;
			}
			//binary comparision, 
			if(!CompareHandlesByLiteral(*pHRow1, *pHRow1AfterDel,FALSE,TRUE))
			{
				goto END;
			}
		}
	}

	//Update the data, deletes are no longer pending
	if(!CHECK(UpdateRows(1,pHRow1,&pDBRowStatus),S_OK))
		goto END;

	//IsSameRow should return DB_E_DELETEDROW because pHRow1 points to a deleted
	//row no matter what DBPROP_REMOVEDELETED has for a default
	if(!CHECK(m_pIRowsetIdentity->IsSameRow(*pHRow1, *pHRow1AfterDel),DB_E_DELETEDROW))
	{
		goto END;
	}
	//binary comparision, these rows can't possibly compare literally
	if(!CompareHandlesByLiteral(*pHRow1, *pHRow1AfterDel,FALSE,FALSE))
	{
		goto END;
	}

	if (g_fREMOVEDELETED)
	{
		//IsSameRow should return S_OK because the deleted row is removed
		if(!CHECK(m_pIRowsetIdentity->IsSameRow(*pHRow0, *pHRow1AfterDel),S_OK))
		{
			goto END;
		}
		//binary comparision, these rows can compare literally
		if(!CompareHandlesByLiteral(*pHRow0, *pHRow1AfterDel,FALSE,TRUE))
		{
			goto END;
		}
	}
	else
	{
		//IsSameRow should return DB_E_DELETEDROW the deleted row is not removed which is
		//pointed to by pHRow1AfterDel
		if(!CHECK(m_pIRowsetIdentity->IsSameRow(*pHRow0, *pHRow1AfterDel),DB_E_DELETEDROW))
		{
			goto END;
		}
		//binary comparision
		if(!CompareHandlesByLiteral(*pHRow0, *pHRow1AfterDel,FALSE,FALSE))
		{
			goto END;
		}
	}
	fTestPass	= TRUE;
END:
	//undo the deletions
	if(pHRow0)
	{
		CHECK(ReleaseRows(1, &pHRow0,TRUE),S_OK);
	}
	//undo the deletions
	if(pHRow1)
	{
		CHECK(ReleaseRows(1, &pHRow1,TRUE),S_OK);
	}
	//undo the deletions
	if(pHRow2)
	{
		CHECK(ReleaseRows(1, &pHRow2,TRUE),S_OK);
	}

	if(pData1)
 	  ReleaseInputBindingsMemory(m_cBinding, m_rgBinding,(BYTE *)pData1,FALSE);

	if(pData2)
	  ReleaseInputBindingsMemory(m_cBinding, m_rgBinding,(BYTE *)pData2,TRUE);

	if(pHRow1AfterDel)
		CHECK(ReleaseRows(1, &pHRow1AfterDel,TRUE),S_OK);

	if(pDBRowStatus)
		PROVIDER_FREE(pDBRowStatus);

	if(fTestPass)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}
// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Dynamic_Query_Buffered::Terminate()
{
	ReleaseRowsetAndAccessor();
	
	return(TCIRowsetIdentity::Terminate());
}	// }}
// }}


// {{ TCW_TC_PROTOTYPE(Dynamic_Cursor_Immediate)
//*-----------------------------------------------------------------------
//|	Test Case:		Dynamic_Cursor_Immediate - Dynamic_Cursor_Immediate
//|	Created:			05/28/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL Dynamic_Cursor_Immediate::Init()
{
	DBPROPID	rgGuid[5];
	BOOL		fCursorNotSupported=FALSE;
	ULONG		cProp = g_MSDASQL?5:4;

	rgGuid[0]=DBPROP_IRowsetIdentity;
	rgGuid[1]=DBPROP_IRowsetChange;
	rgGuid[2]=DBPROP_OTHERINSERT;
	rgGuid[3]=DBPROP_OTHERUPDATEDELETE;
	rgGuid[4]=KAGPROP_QUERYBASEDUPDATES;

	if(!TCIRowsetIdentity::Init())
		return FALSE;

	//check to see if the execute is going to fail
	if( !g_rgDBPrpt[IDX_ScrollBackwards].fSupported ||
		!g_rgDBPrpt[IDX_CanHoldRows].fSupported ||
		!g_rgDBPrpt[IDX_OtherInsert].fSupported )
	{
		fCursorNotSupported = TRUE;
		odtLog << L"The Provider does not support a DYNAMIC Cursor.\n";
	}

	//get a rowset with IRowsetIdentity, with Dynamic Cursor
	if(!GetRowsetAndAccessor(USE_OPENROWSET, cProp, rgGuid,0,NULL,NO_ACCESSOR))
	{
		return TEST_SKIPPED;
	}
	if (!g_fTestValid)
	{
		return TRUE;
	}

	//has to be dynamic cursor
	if(GetCursorType()==DYNAMIC_CURSOR)
	{
		odtLog << L"This test will continue but a dynamic cursor was not obtained.\n";	//	return FALSE;
	}
	return TRUE;
}
// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc  Hard-delete one row.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int Dynamic_Cursor_Immediate::Variation_1()
{
	HROW		*pHRow		= NULL;
	DBCOUNTITEM	cRows		= 0;
	BOOL		fTestPass	= FALSE;

	//if this test variation is not valid
	if (!g_fTestValid)
	{
		odtLog << L"The Provider does not support the properties that are necessary for this variation to execute.\n";
        return TEST_PASS;
	}
	
	//get two row handles
	if(!CHECK(m_pIRowset->GetNextRows(NULL, 8, 2, &cRows, &pHRow),S_OK))
		goto END;

	//hard-delete the 1st row
	if(!CHECK(DeleteRows(1,pHRow),S_OK))
		goto END;
	
	//IsSameRow should return DB_E_DELETEDROW
	if(!CHECK(m_pIRowsetIdentity->IsSameRow(pHRow[1], *pHRow),DB_E_DELETEDROW))
	{
		goto END;
	}
	//binary comparision
	if(CompareHandlesByLiteral(pHRow[1], *pHRow,FALSE,FALSE))
	{
		fTestPass=TRUE;
	}
END:
	if(pHRow)
	{
		CHECK(m_pIRowset->ReleaseRows(2,pHRow,NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	if(fTestPass)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// }}
// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL Dynamic_Cursor_Immediate::Terminate()
{
	ReleaseRowsetAndAccessor();
	
	return(TCIRowsetIdentity::Terminate());
}	// }}
// }}


// {{ TCW_TC_PROTOTYPE(ExtendedErrors)
//*-----------------------------------------------------------------------
//|	Test Case:		ExtendedErrors - Extended Errors
//|	Created:			07/15/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//

BOOL ExtendedErrors::Init()
{
	DBPROPID	rgGuid[2];
	ULONG		cProp=1;

	rgGuid[0]=DBPROP_IRowsetIdentity;
	
	if(g_rgDBPrpt[IDX_IRowsetChange].fSupported)
	{
		rgGuid[1]=DBPROP_IRowsetChange;
		cProp=2;
	}
	if(!TCIRowsetIdentity::Init())
		return FALSE;

	//get a rowset with IID_IRowsetIdentity
	if(! GetRowsetAndAccessor(USE_OPENROWSET, cProp,rgGuid,0,NULL,NO_ACCESSOR))
		return FALSE;			 
	else
		return TRUE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Valid IRowsetIdentity calls with previous error object existing.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ExtendedErrors::Variation_1()
{  
	HROW		HRow		= NULL;
	HROW		*pHRow		= &HRow;
	DBCOUNTITEM	cRows		= 0;
	BOOL		fTestPass	= FALSE;   

	//if this test variation is not valid
	if (!g_fTestValid)
	{
		odtLog << L"The Provider does not support the properties that are necessary for this variation to execute.\n";
        return TEST_PASS;
	}
	
	//For each method of the interface, first create an error object on
	//the current thread, try get a success from the IRowsetIdentity method.
	//We then check extended errors to verify the right extended error behavior.
	
	//get one row handles
	if(!CHECK(m_pIRowset->GetNextRows(NULL,1,1,&cRows,&pHRow),S_OK))
		goto END;
	
	m_pExtError->CauseError();

	//IsSameRow should return S_OK
	if(CHECK(m_hr=m_pIRowsetIdentity->IsSameRow(HRow,HRow),S_OK))
		//Do extended check following IsSameRow
		fTestPass = XCHECK(m_pIRowsetIdentity, IID_IRowsetIdentity, m_hr);	
 	 
END:
	if(HRow)
		CHECK(m_pIRowset->ReleaseRows(1, pHRow, NULL,NULL,NULL),S_OK);
  
	if(fTestPass)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Invalid IRowsetIdentity calls with previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ExtendedErrors::Variation_2()
{	
	HROW		*pHRow		= NULL;
	DBCOUNTITEM	cRows		= 0;
	BOOL		fTestPass	= FALSE;   

	//if this test variation is not valid
	if (!g_fTestValid)
	{
		odtLog << L"The Provider does not support the properties that are necessary for this variation to execute.\n";
        return TEST_PASS;
	}
	
	//For each method of the interface, first create an error object on
	//the current thread, try get a failure from the IRowsetIdentity method.
	//We then check extended errors to verify the right extended error behavior.
	
	//get two row handles
	if(!CHECK(m_pIRowset->GetNextRows(NULL,0,2,&cRows,&pHRow),S_OK))
		goto END;

	//release the 2nd row handle
	if(!CHECK(m_pIRowset->ReleaseRows(1,&(pHRow[1]),NULL,NULL,NULL),S_OK))
		goto END;
	pHRow[1]=NULL;

	//mark the active row handles
	cRows--;
	
	m_pExtError->CauseError();

	//IsSameRow should return DB_E_BADROWHANDLE
	if(CHECK(m_hr=m_pIRowsetIdentity->IsSameRow(pHRow[1], *pHRow),DB_E_BADROWHANDLE))
		//Do extended check following IsSameRow
		fTestPass = XCHECK(m_pIRowsetIdentity, IID_IRowsetIdentity, m_hr);	
END:
	if(pHRow)
	{
		CHECK(m_pIRowset->ReleaseRows(cRows, pHRow, NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	if(fTestPass)
		return TEST_PASS;
	else
		return TEST_FAIL;

}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Invalid IRowsetIdentity calls with no previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int ExtendedErrors::Variation_3()
{
	HROW			*pHRow		= NULL;
	DBCOUNTITEM		cRows		= 0;
	DBROWSTATUS		rgRowStatus[1];
	BOOL			fTestPass	= FALSE;   

	if(g_fReadOnlyProvider)
		return TEST_PASS;

	//if this test variation is not valid
	if (!g_fTestValid)
	{
		odtLog << L"The Provider does not support the properties that are necessary for this variation to execute.\n";
        return TEST_PASS;
	}
	//if IRowsetChange not available because conformance won't allow it
	if (!g_fIRowsetChange)
	{
		odtLog << L"Can not get to IRowsetChange, can not insert a row.\n";
        return TEST_PASS;
	}	
	//if IRowsetChange not available because conformance won't allow it
	//For read only provider or IRowsetChange not available, skip this variation
	if(g_fReadOnlyProvider  || !(g_fROWSETUPDATABLE))
	{
		odtLog << L"Can not get to IRowsetChange, can not insert a row.\n";
		return TEST_PASS;
	}

	//For each method of the interface, with no error object on
	//the current thread, try get a failure from the IRowsetIdentity method.
	//We then check extended errors to verify the right extended error behavior.

	//get two row handles
	if(!CHECK(m_pIRowset->GetNextRows(NULL,0,2,&cRows,&pHRow),S_OK))
		goto END;

	//For read only provider or IRowsetChange not available, skip this variation
	if(g_fReadOnlyProvider  || !(g_fROWSETUPDATABLE))
	{
		odtLog << L"Can not get to IRowsetChange, can not insert a row.\n";
		return TEST_PASS;
	}
	//check to see if the DSO is ReadOnly
	if( IsPropertySet(DBPROPSET_DATASOURCEINFO, DBPROP_DATASOURCEREADONLY, NULL, 0) )
	{
		//try deleting the row handle
		if(!CHECK(m_hr=m_pIRowsetChange->DeleteRows(NULL,1,pHRow,rgRowStatus),DB_E_ERRORSOCCURRED) ||
		   !COMPARE(rgRowStatus[0], DBROWSTATUS_E_PERMISSIONDENIED))
			goto END;

		//Do extended check following IsSameRow
		fTestPass = XCHECK(m_pIRowsetChange, IID_IRowsetChange, m_hr);	

		//IsSameRow should return S_OK
		CHECK(m_pIRowsetIdentity->IsSameRow(*pHRow,pHRow[0]),S_OK);
	}
	else
	{
		//delete the row handle
		if(!CHECK(m_pIRowsetChange->DeleteRows(NULL,1,pHRow,rgRowStatus),S_OK) ||
		   !COMPARE(rgRowStatus[0], DBROWSTATUS_S_OK))
			goto END;

		//IsSameRow should return DB_E_DELETEDROW
		CHECK(m_hr=m_pIRowsetIdentity->IsSameRow(*pHRow, pHRow[0]),DB_E_DELETEDROW);

		//Do extended check following IsSameRow
		fTestPass = XCHECK(m_pIRowsetIdentity, IID_IRowsetIdentity, m_hr);	
	}
END:
	if(pHRow)
	{
		CHECK(m_pIRowset->ReleaseRows(2, pHRow, NULL,NULL,NULL),S_OK);
		PROVIDER_FREE(pHRow);
	}

	if(fTestPass)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}
// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL ExtendedErrors::Terminate()
{
	ReleaseRowsetAndAccessor();

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIRowsetIdentity::Terminate());
}	// }}
// }}

