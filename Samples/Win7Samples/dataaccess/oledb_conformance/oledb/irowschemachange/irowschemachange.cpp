//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright (C) 1995-2000 Microsoft Corporation
//
// @doc  
//
// @module IRowSchemaChange.cpp | This module tests the OLEDB IRowSchemaChange interface 
//

#include "MODStandard.hpp"			// Standard headers			
#include "IRowSchemaChange.h"		// IRowSchemaChange header
#include "ExtraLib.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Module Values
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// {{ TCW_MODULE_GLOBALS
DECLARE_MODULE_CLSID = { 0xea01aaa0, 0x58bb, 0x11d2, { 0xa9, 0x94, 0x00, 0xc0, 0x4f, 0x94, 0xa7, 0x17} };
DECLARE_MODULE_NAME("IRowSchemaChange");
DECLARE_MODULE_OWNER("Microsoft");
DECLARE_MODULE_DESCRIP("IRowSchemaChange interface test");
DECLARE_MODULE_VERSION(838086926);
// TCW_WizardVersion(2)
// TCW_Automation(False)
// }} TCW_MODULE_GLOBALS_END


//////////////////////////////////////////////////////////////////////////
// Globals
//
//////////////////////////////////////////////////////////////////////////
ULONG cInterfaceIIDs = 0;
INTERFACEMAP* rgInterfaceIIDs = NULL;


//////////////////////////////////////////////////////////////////////////
// Defines
//
//////////////////////////////////////////////////////////////////////////
enum EADDCOLUMNS
{
	OPT_MODIFYCOLUMNS	= 0x00000001,
	OPT_ADDNEWCOLUMNS	= 0x00000002,
	OPT_IGNOREDATA		= 0x00000010,
	OPT_NULLDATA		= 0x00000020,
	OPT_DEFAULTDATA		= 0x00000040,

	OPT_UPDATEANDADD	= OPT_MODIFYCOLUMNS | OPT_ADDNEWCOLUMNS,

	OPT_BUFFEREDUPDATE	= 0x00000100,
	OPT_BUFFEREDUNDO	= 0x00000200,
};


//--------------------------------------------------------------------
// @func Module level initialization routine
//
// @rdesc Success or Failure
// 		@flag  TRUE  | Successful initialization
//		@flag  FALSE | Initialization problems
//
BOOL ModuleInit(CThisTestModule * pThisTestModule)
{	
	//Obtain the Interface IIDs for the Row object
	if(GetInterfaceArray(ROW_INTERFACE, &cInterfaceIIDs, &rgInterfaceIIDs))
		return CommonModuleInit(pThisTestModule, IID_IRowSchemaChange, SIZEOF_TABLE, ROW_INTERFACE);

	return FALSE;
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
//  TCIRowSchemaChange
//
////////////////////////////////////////////////////////////////////////////
class TCIRowSchemaChange : public CRowset
{
public:
	//constructors
	TCIRowSchemaChange(WCHAR* pwszTestCaseName = INVALID(WCHAR*));
	virtual ~TCIRowSchemaChange();

	//methods
	virtual BOOL		Init();
	virtual BOOL		Terminate();

	virtual HRESULT		CreateNewColInfo
						(
							CRowObject*			pCRowObject,
							DBCOUNTITEM				iRow,
							DBORDINAL				cColAccess,
							DBCOLUMNACCESS*		rgColAccess,
							DBCOLUMNINFO**		prgColInfo
						);

	virtual	HRESULT		CreateColumns
						(
							CRowObject*			pCRowObject,
							DBORDINAL*				pcColAccess,
							DBCOLUMNACCESS**	prgColAccess,
							DBCOLUMNINFO**		prgColInfo,
							void**				ppData,
							void**				ppData2,
							
							EADDCOLUMNS			eAddColumns,
							DBCOUNTITEM				iRow,
							DWORD				dwColsToBind	= UPDATEABLE_NONINDEX_COLS_BOUND,
							BLOBTYPE			dwBlobType		= NO_BLOB_COLS,
							ECOLUMNORDER		eBindingOrder	= FORWARD,		
							ECOLS_BY_REF		eColsByRef		= NO_COLS_BY_REF,				
							DBTYPE				dwModifier		= DBTYPE_EMPTY,
							DBORDINAL				cColsToBind		= 0,
							DBORDINAL*				rgColsToBind    = NULL
						);

	//IRowSchemaChange::AddColumns
	virtual HRESULT		AddColumns
						(
							CRowObject*				pCRowObject,
							DBCOUNTITEM					iRow,
							DBORDINAL					cColAccess,
							DBCOLUMNINFO*			rgColInfo,
							DBCOLUMNACCESS*			rgColAccess
						);

	virtual BOOL		VerifyAddColumns
						(
							CRowObject*			pCRowObject,
							DBCOUNTITEM				iRow,
							DBORDINAL				cColAccess,
							DBCOLUMNINFO*		rgColInfo,
							DBCOLUMNACCESS*		rgColAccess
						);

	
	virtual BOOL		VerifyAddColumns
						(
							CRowObject*			pCRowObject,
							EADDCOLUMNS			eAddColumns,
							DBCOUNTITEM				iRow,
							DWORD				dwColsToBind	= UPDATEABLE_NONINDEX_COLS_BOUND,			
							BLOBTYPE			dwBlobType		= NO_BLOB_COLS,
							ECOLUMNORDER		eBindingOrder	= FORWARD,		
							ECOLS_BY_REF		eColsByRef		= NO_COLS_BY_REF,				
							DBTYPE				dwModifier		= DBTYPE_EMPTY,
							DBORDINAL				cColsToBind		= 0,
							DBORDINAL*				rgColsToBind    = NULL
						);

	virtual BOOL		VerifyAddColumnsAllRows
						(
							CRowsetChange*		pCRowset,
							EADDCOLUMNS			eAddColumns,
							DWORD				dwColsToBind	= UPDATEABLE_NONINDEX_COLS_BOUND,			
							BLOBTYPE			dwBlobType		= NO_BLOB_COLS,
							ECOLUMNORDER		eBindingOrder	= FORWARD,		
							ECOLS_BY_REF		eColsByRef		= NO_COLS_BY_REF,				
							DBTYPE				dwModifier		= DBTYPE_EMPTY,
							DBORDINAL				cColsToBind		= 0,
							DBORDINAL*				rgColsToBind    = NULL
						);

	
	//IRowSchemaChange::DeleteColumns
	virtual HRESULT		DeleteColumns
						(
							CRowObject*			pCRowObject,
							DBORDINAL				cColAccess,
							DBCOLUMNACCESS*		rgColAccess
						);

	virtual BOOL		VerifyDeleteColumns
						(
							CRowObject*			pCRowObject,
							DBCOUNTITEM				iRow,
							DBORDINAL				cColAccess,
							DBCOLUMNACCESS*		rgColAccess
						);

	virtual BOOL		VerifyDeleteColumns
						(
							CRowObject*			pCRowObject,
							DBCOUNTITEM				iRow,
							DWORD				dwColsToBind	= UPDATEABLE_NONINDEX_COLS_BOUND,			
							BLOBTYPE			dwBlobType		= NO_BLOB_COLS,
							ECOLUMNORDER		eBindingOrder	= FORWARD,		
							ECOLS_BY_REF		eColsByRef		= NO_COLS_BY_REF,				
							DBTYPE				dwModifier		= DBTYPE_EMPTY,
							DBORDINAL				cColsToBind		= 0,
							DBORDINAL*				rgColsToBind    = NULL
						);

	virtual BOOL		VerifyDeleteColumnsAllRows
						(
							CRowsetChange*		pCRowset,
							DWORD				dwColsToBind	= UPDATEABLE_NONINDEX_COLS_BOUND,			
							BLOBTYPE			dwBlobType		= NO_BLOB_COLS,
							ECOLUMNORDER		eBindingOrder	= FORWARD,		
							ECOLS_BY_REF		eColsByRef		= NO_COLS_BY_REF,				
							DBTYPE				dwModifier		= DBTYPE_EMPTY,
							DBORDINAL				cColsToBind		= 0,
							DBORDINAL*				rgColsToBind    = NULL
						);

	//Thread Methods
	static ULONG WINAPI Thread_VerifyAddColumns(LPVOID pv);
	static ULONG WINAPI Thread_VerifyDeleteColumns(LPVOID pv);

	//Interface
	virtual IRowSchemaChange*	const pRowSchemaChange();

	//Data
	CRowObject*			m_pCRowObject;
	IRowSchemaChange*	m_pIRowSchemaChange;
};




////////////////////////////////////////////////////////////////////////////
//  TCIRowSchemaChange::TCIRowSchemaChange
//
////////////////////////////////////////////////////////////////////////////
TCIRowSchemaChange::TCIRowSchemaChange(WCHAR * wstrTestCaseName)	: CRowset(wstrTestCaseName) 
{
	m_pCRowObject		= NULL;
	m_pIRowSchemaChange	= NULL;
}


////////////////////////////////////////////////////////////////////////////
//  TCIRowSchemaChange::~TCIRowSchemaChange
//
////////////////////////////////////////////////////////////////////////////
TCIRowSchemaChange::~TCIRowSchemaChange()
{
}


////////////////////////////////////////////////////////////////////////////
//  TCIRowSchemaChange::Init
//
////////////////////////////////////////////////////////////////////////////
BOOL TCIRowSchemaChange::Init()
{
	TBEGIN
	HROW hRow = NULL;
	
	//Create the new row object
	m_pCRowObject = new CRowObject;
	TESTC(m_pCRowObject != NULL);

	TESTC(CRowset::Init());

	//Create the Rowset object
	TESTC_(CreateRowset(DBPROP_IRowsetChange), S_OK);
	
	//Obtain the First row...
	TESTC_(GetNextRows(&hRow),S_OK);

	//Now create the row object.
	TEST2C_(m_pCRowObject->CreateRowObject(pIRowset(), hRow), S_OK, DB_S_NOROWSPECIFICCOLUMNS);

	//Now obtain our IRowSchemaChange interface.
	TESTC(VerifyInterface(m_pCRowObject->pIRow(), IID_IRowSchemaChange, ROW_INTERFACE, (IUnknown**)&m_pIRowSchemaChange));

CLEANUP:
	ReleaseRows(hRow);
	TRETURN
}


////////////////////////////////////////////////////////////////////////////
//  TCIRowSchemaChange::Terminate
//
////////////////////////////////////////////////////////////////////////////
BOOL TCIRowSchemaChange::Terminate()
{
	SAFE_RELEASE(m_pIRowSchemaChange);
	SAFE_DELETE(m_pCRowObject);
	return CRowset::Terminate();
}


////////////////////////////////////////////////////////////////////////////
//  TCIRowSchemaChange::CreateNewColInfo
//
////////////////////////////////////////////////////////////////////////////
HRESULT TCIRowSchemaChange::CreateNewColInfo
(
	CRowObject*			pCRowObject,
	DBCOUNTITEM				iRow,
	DBORDINAL				cColAccess,
	DBCOLUMNACCESS*		rgColAccess,
	DBCOLUMNINFO**		prgColInfo
)
{
	TBEGIN
	HRESULT hr = E_FAIL;
	ASSERT(pCRowObject);
	ASSERT(prgColInfo);
	DBORDINAL iCol = 0;

	//Create the output ColInfo array
	SAFE_ALLOC(*prgColInfo, DBCOLUMNINFO, cColAccess);
	memset(*prgColInfo, 0, (size_t)(cColAccess*sizeof(DBCOLUMNINFO)));

	//Create data for all the columns, before we change the columnids
	TESTC_(hr = pCRowObject->FillColAccess(pTable(), cColAccess, rgColAccess, iRow),S_OK);

	//Loop through all provided columns
	for(iCol=0; iCol<cColAccess; iCol++)
	{
		DBCOLUMNACCESS* pColAccess = &rgColAccess[iCol];
		DBCOLUMNINFO* pColInfo = &((*prgColInfo)[iCol]);
		pColInfo->columnid	= pColAccess->columnid;

		//Obtain the colinfo for this column (which will be the defaults for the column info)
		while(::FindColInfo(pCRowObject->pIRow(), &pColInfo->columnid, 0, pColInfo))
		{
			//Try to create a new "unique" name for the columnid, so we don't have clashes...
			TESTC_(hr = CreateUniqueDBID(&pColInfo->columnid),S_OK);
		}

		//Setup additional default arguments...
		pColInfo->dwFlags	|= DBCOLUMNFLAGS_WRITE;
	}

	//Everything succeeded if we get here...
	hr = S_OK;
	
CLEANUP:
	return hr;
}


////////////////////////////////////////////////////////////////////////////
//  HRESULT TCIRowSchemaChange::AddColumns
//
////////////////////////////////////////////////////////////////////////////
HRESULT TCIRowSchemaChange::AddColumns
(
	CRowObject*				pCRowObject,
	DBCOUNTITEM					iRow,
	DBORDINAL					cColAccess,
	DBCOLUMNINFO*			rgColInfo,
	DBCOLUMNACCESS*			rgColAccess
)
{
	TBEGIN
	HRESULT hr = S_OK;
	ASSERT(pCRowObject);
	IRowSchemaChange* pIRowSchemaChange = NULL;

	//Obtain the IRowSchemaChange interface
	if(!VerifyInterface(pCRowObject->pIRow(), IID_IRowSchemaChange, ROW_INTERFACE, (IUnknown**)&pIRowSchemaChange))
	{
		hr = E_NOINTERFACE;
		goto CLEANUP;
	}

	//IRowSchemaChange::AddColumns
	hr = pIRowSchemaChange->AddColumns(cColAccess, rgColInfo, rgColAccess);

	//First verify the column status against the return code...
	TESTC(VerifyColAccess(hr, cColAccess, rgColAccess));
	
CLEANUP:
	SAFE_RELEASE(pIRowSchemaChange);
	return hr;
}


////////////////////////////////////////////////////////////////////////////
//  TCIRowSchemaChange::VerifyAddColumns
//
////////////////////////////////////////////////////////////////////////////
BOOL TCIRowSchemaChange::VerifyAddColumns
(
	CRowObject*			pCRowObject,
	DBCOUNTITEM				iRow,
	DBORDINAL				cColAccess,
	DBCOLUMNINFO*		rgColInfo,
	DBCOLUMNACCESS*		rgColAccess
)
{

	TBEGIN
	HRESULT hr = S_OK;
	ASSERT(pCRowObject);
	ASSERT(rgColInfo);
	DBSTATUS* rgExpectedStatus = NULL;
	DBORDINAL iCol=0;
	BOOL bCompare = TRUE;
	const ULONG DONT_COMPARE_DATA = 0x80000000;

	//Need to figure out which columns already exist (before we actually add them)
	SAFE_ALLOC(rgExpectedStatus, DBSTATUS, cColAccess);
	for(iCol=0; iCol<cColAccess; iCol++)
	{
		DBCOLUMNACCESS* pColAccess = rgColAccess ? &rgColAccess[iCol] : NULL;
		DBSTATUS dwStatus = pColAccess ? pColAccess->dwStatus : DBSTATUS_S_OK;

		//NOTE:  Alreadyexists will supercede all normal data success status', since its
		//a much more important message.  DEFAULT, ISNULL, or IGNORE will be assumed to 
		//work successfully if ALREADYEXISTS is returned...
		rgExpectedStatus[iCol] = dwStatus;
		if(::FindColInfo(pCRowObject->pIRow(), &rgColInfo[iCol].columnid))
		{
			rgExpectedStatus[iCol] = DBSTATUS_S_ALREADYEXISTS;
		}

		//Problem is that ALREADYEXISTS will be returned, so we need to know if 
		//we need to look at the data or not coming back, otheriwse a user may have bound
		//the data as IGNORE, it gets changed to ALREADYEXISTS and we look at the data
		//and crash...
		if(dwStatus != DBSTATUS_S_OK)
			rgExpectedStatus[iCol] |= DONT_COMPARE_DATA;

		//Also, for stream objects we need to make sure the provider releases the Storage
		//objects correctly, otherwsie leaks will occur.  So we will AddRef the stream
		//before AddColumns is called (so its still valid on return), and then we will
		//release it and make sure the refcount is 0...
		if(pColAccess && pColAccess->wType == DBTYPE_IUNKNOWN && pColAccess->dwStatus == DBSTATUS_S_OK)
		{
			IUnknown* pIUnknown = *(IUnknown**)pColAccess->pData;
			SAFE_ADDREF(pIUnknown);
		}
	}

	//IRowSchemaChange::AddColumns
	TESTC_(hr = AddColumns(pCRowObject, iRow, cColAccess, rgColInfo, rgColAccess),S_OK);

	//Verify the "AlreadyExist" status
	for(iCol=0; iCol<cColAccess; iCol++)
	{
		//Weither the column existed before or not, now it should be returned in ColInfo...
		if(!::FindColInfo(pCRowObject->pIRow(), &rgColInfo[iCol].columnid))
			TERROR("Newly added column not returned in ColInfo!");
	
		if(rgColAccess)
		{
			DBCOLUMNACCESS* pColAccess = &rgColAccess[iCol];
			BOOL bCompareData = !(rgExpectedStatus[iCol] & DONT_COMPARE_DATA);
			
			rgExpectedStatus[iCol] &= ~DONT_COMPARE_DATA;
			if(pColAccess->dwStatus != rgExpectedStatus[iCol])
				TERROR("Status returned: " << GetStatusName(pColAccess->dwStatus) << " Status expected: " << GetStatusName(rgExpectedStatus[iCol]));

			//Compare the Data
			if(bCompareData)
			{
				//Verify the columns are really Added/Updated
				//Obtain the data into a different buffer, so we know things are really 
				//being returned from GetColumns, and actually updating our AddColumns buffer...
				DBCOLUMNACCESS rgGetColAccess[1];
				memcpy(rgGetColAccess, pColAccess, sizeof(DBCOLUMNACCESS));
				pColAccess->cbMaxLen = min(pColAccess->cbMaxLen, MAXDATALEN);
				SAFE_ALLOC(rgGetColAccess[0].pData, BYTE, pColAccess->cbMaxLen);
				
				
				TESTC_(pCRowObject->GetColumns(1, rgGetColAccess),S_OK);

				//Compare Data for this row object
				if(rgExpectedStatus[iCol] == DBSTATUS_S_ALREADYEXISTS)
				{
				
					//Will only be able to correctly compare data for the existing columns
					bCompare = pCRowObject->CompareColAccess(1, rgGetColAccess, iRow, pTable());
				}
				else
				{
					//For newly added columns (ones that didn't exist already)
					//we can't use the expected data either from privlib or the INI File, since
					//it didn't previously exist.  What we can use if the data that we inserted
					//and just compare the two buffers...
					bCompare = pCRowObject->CompareColBuffer(1, rgGetColAccess, 1, pColAccess);
				}

				//Display the result if anything miscompared...
				if(!bCompare)
				{
					//Data incorrect for this row!
					TERROR("Data was incorrect for row " << iRow << " iCol " << iCol);
					QTESTC(FALSE);
				}

				//Free any out of line data...
				FreeColAccess(1, rgGetColAccess, FALSE);
				SAFE_FREE(rgGetColAccess[0].pData);
			}

			//Make sure all Stream objects were correctly released...
			if(pColAccess->wType == DBTYPE_IUNKNOWN && pColAccess->dwStatus == DBSTATUS_S_OK)
			{
				IUnknown* pIUnknown = *(IUnknown**)pColAccess->pData;
				SAFE_RELEASE_(pIUnknown);
			}
		}
	}

CLEANUP:
	SAFE_FREE(rgExpectedStatus);
	TRETURN;
}


////////////////////////////////////////////////////////////////////////////
//  TCIRowSchemaChange::CreateColumns
//
////////////////////////////////////////////////////////////////////////////
HRESULT TCIRowSchemaChange::CreateColumns
(
	CRowObject*			pCRowObject,
	DBORDINAL*				pcColAccess,
	DBCOLUMNACCESS**	prgColAccess,
	DBCOLUMNINFO**		prgColInfo,
	void**				ppData,
	void**				ppData2,
	
	EADDCOLUMNS			eAddColumns,
	DBCOUNTITEM				iRow,
	DWORD				dwColsToBind,			
	BLOBTYPE			dwBlobType,
	ECOLUMNORDER		eBindingOrder,		
	ECOLS_BY_REF		eColsByRef,				
	DBTYPE				dwModifier,
	DBORDINAL				cColsToBind,
	DBORDINAL*				rgColsToBind
)
{
	TBEGIN
	ASSERT(pCRowObject);
	ASSERT(pcColAccess && prgColAccess && prgColInfo && ppData);
	HRESULT hr = S_OK;
	
	DBORDINAL iCol,cColAccess = 0;
	DBCOLUMNACCESS* rgColAccess = NULL;
	DBCOLUMNINFO* rgColInfo = NULL;
	void* pData = NULL;

	DBORDINAL cColAccess2 = 0;
	DBCOLUMNACCESS* rgColAccess2 = NULL;
	DBCOLUMNINFO* rgColInfo2 = NULL;
	void* pData2 = NULL;

	//Create the ColAccess Structures...
	TESTC_(hr = pCRowObject->CreateColAccess(&cColAccess, &rgColAccess, &pData, NULL, dwColsToBind, dwBlobType, eBindingOrder, eColsByRef, dwModifier, cColsToBind, rgColsToBind),S_OK);

	//User wishes to just update the specified (existing) columns
	if(eAddColumns & OPT_MODIFYCOLUMNS)
	{
		//Allocate the ColInfo
		SAFE_ALLOC(rgColInfo, DBCOLUMNINFO, cColAccess);
		memset(rgColInfo, 0, (size_t)(cColAccess*sizeof(DBCOLUMNINFO)));
		
		//For existing columns provider should not be looking at the metadata,
		//except of course for the columnid.
		for(iCol=0; iCol<cColAccess; iCol++)
			rgColInfo[iCol].columnid = rgColAccess[iCol].columnid;

		//Create the Data for AddColumns
		TESTC_(hr = pCRowObject->FillColAccess(pTable(), cColAccess, rgColAccess, iRow),S_OK);
	}

	//User wishes to add new columns, (with new columnids), 
	//but the meta data based upon existing columns.
	if(eAddColumns & OPT_ADDNEWCOLUMNS)
	{
		if(!(eAddColumns & OPT_MODIFYCOLUMNS))
		{
			//Simple, user wants to only add columns, based upon existing columns
			TESTC_(hr = CreateNewColInfo(pCRowObject, iRow, cColAccess, rgColAccess, &rgColInfo),S_OK);
		}
		else
		{
			//More difficult, user wants to update existing columns, and add new columns, based upon existing columns
			ASSERT(ppData2);
			TESTC_(hr = pCRowObject->CreateColAccess(&cColAccess2, &rgColAccess2, &pData2, NULL, dwColsToBind, dwBlobType, eBindingOrder, eColsByRef, dwModifier, cColsToBind, rgColsToBind),S_OK);
			
			//Now create new data for all the new additional columns.
			TESTC_(hr = CreateNewColInfo(pCRowObject, iRow, cColAccess2, rgColAccess2, &rgColInfo2),S_OK);

			//Now append the new ColAccess to the end of the others
			SAFE_REALLOC(rgColAccess, DBCOLUMNACCESS, cColAccess+cColAccess2);
			memcpy(&rgColAccess[cColAccess], rgColAccess2, (size_t)(cColAccess2*sizeof(DBCOLUMNACCESS)));
						
			//Now append the new ColInfo the end of the others
			SAFE_REALLOC(rgColInfo, DBCOLUMNINFO, cColAccess+cColAccess2);
			memcpy(&rgColInfo[cColAccess], rgColInfo2, (size_t)(cColAccess2*sizeof(DBCOLUMNACCESS)));
			cColAccess = cColAccess+cColAccess2;
		}
	}

	//Create ColInfo...
	if(eAddColumns & (OPT_IGNOREDATA|OPT_NULLDATA|OPT_DEFAULTDATA))
	{
		//Make sure all the bits are not on...
		ASSERT((eAddColumns & (OPT_IGNOREDATA|OPT_NULLDATA|OPT_DEFAULTDATA)) != (OPT_IGNOREDATA|OPT_NULLDATA|OPT_DEFAULTDATA));
		
		//Set all the status' to the status specified
		for(iCol=0; iCol<cColAccess; iCol++)
		{
			rgColAccess[iCol].dwStatus = DBSTATUS_S_DEFAULT;
			if(eAddColumns & OPT_IGNOREDATA)
				rgColAccess[iCol].dwStatus = DBSTATUS_S_IGNORE;
			if(eAddColumns & OPT_NULLDATA)
				rgColAccess[iCol].dwStatus = DBSTATUS_S_ISNULL;
		}
	}

CLEANUP:
	//Return the values to the user...
	*pcColAccess	= cColAccess;
	*prgColAccess	= rgColAccess;
	*prgColInfo		= rgColInfo;
	*ppData			= pData;
	if(ppData2)
		*ppData2		= pData2;
	return hr;
}

	
////////////////////////////////////////////////////////////////////////////
//  TCIRowSchemaChange::VerifyAddColumns
//
////////////////////////////////////////////////////////////////////////////
BOOL TCIRowSchemaChange::VerifyAddColumns
(
	CRowObject*			pCRowObject,
	EADDCOLUMNS			eAddColumns,
	DBCOUNTITEM				iRow,
	DWORD				dwColsToBind,			
	BLOBTYPE			dwBlobType,
	ECOLUMNORDER		eBindingOrder,		
	ECOLS_BY_REF		eColsByRef,				
	DBTYPE				dwModifier,
	DBORDINAL				cColsToBind,
	DBORDINAL*				rgColsToBind
)
{
	TBEGIN
	ASSERT(pCRowObject);
	HRESULT hr = S_OK;
	
	DBORDINAL cColAccess = 0;
	DBCOLUMNACCESS* rgColAccess = NULL;
	DBCOLUMNINFO* rgColInfo = NULL;
	void* pData = NULL;
	void* pData2 = NULL;

	//Create the ColAccess Structures...
	TESTC_(hr = CreateColumns(pCRowObject, &cColAccess, &rgColAccess, &rgColInfo, &pData, &pData2, eAddColumns, iRow, dwColsToBind, dwBlobType, eBindingOrder, eColsByRef, dwModifier, cColsToBind, rgColsToBind),S_OK);

	//Verify AddColumns
	QTESTC(VerifyAddColumns(pCRowObject, iRow, cColAccess, rgColInfo, rgColAccess));

CLEANUP:
	//For AddColumns (SetColumns), (SetData) provider is reponsible for releasing streams...
	//So we don't use FreeColAccess access here...
	SAFE_FREE(rgColAccess);
	SAFE_FREE(rgColInfo);
	SAFE_FREE(pData);
	SAFE_FREE(pData2);
	TRETURN;
}



////////////////////////////////////////////////////////////////////////////
//  TCIRowSchemaChange::VerifyAddColumnsAllRows
//
////////////////////////////////////////////////////////////////////////////
BOOL TCIRowSchemaChange::VerifyAddColumnsAllRows
(
	CRowsetChange*		pCRowset,
	EADDCOLUMNS			eAddColumns,
	DWORD				dwColsToBind,			
	BLOBTYPE			dwBlobType,
	ECOLUMNORDER		eBindingOrder,		
	ECOLS_BY_REF		eColsByRef,				
	DBTYPE				dwModifier,
	DBORDINAL				cColsToBind,
	DBORDINAL*				rgColsToBind
)
{
	TBEGIN
	HRESULT hr = S_OK;

	DBCOUNTITEM iRow,cRowsObtained = 0;
	HROW* rghRows = NULL;
	CRowsetChange RowsetA;
	IRowsetUpdate* pIRowsetUpdate = NULL;
	BOOL bCompare = TRUE;

	//Default rowset
	if(pCRowset == NULL)
	{
		pCRowset = &RowsetA;

		//May require IRowsetLocate to position on Blobs
		if(dwBlobType != NO_BLOB_COLS)
			pCRowset->SetSettableProperty(DBPROP_IRowsetLocate);
		if(eAddColumns & (OPT_BUFFEREDUPDATE|OPT_BUFFEREDUNDO))
			pCRowset->SetProperty(DBPROP_IRowsetUpdate);
		TESTC_(pCRowset->CreateRowset(),S_OK);
	}

	//Restart the position.
	TESTC_(pCRowset->RestartPosition(),S_OK);

	//loop through the rowset, retrieve one row at a time
	for(iRow=1; iRow<=pCRowset->m_ulTableRows; iRow++)	
	{
		//GetNextRow 
		CRowObject RowObjectA;
		TESTC_(pCRowset->GetNextRows(0, 1, &cRowsObtained, &rghRows),S_OK);
		
		//Create the row object from this row
		TEST2C_(RowObjectA.CreateRowObject(pCRowset->pIRowset(), rghRows[0]), S_OK, DB_S_NOROWSPECIFICCOLUMNS);

		//Verify Row Object
		QTESTC(bCompare == VerifyAddColumns(&RowObjectA, eAddColumns, iRow, dwColsToBind, dwBlobType, eBindingOrder, eColsByRef, dwModifier, cColsToBind, rgColsToBind));

		//If the rowset is in buffered mode, it will require an Update, or Undo
		//before the change really takes effect.
		if(eAddColumns & (OPT_BUFFEREDUPDATE|OPT_BUFFEREDUNDO))
		{
			//We should be in buffered mode
			TESTC(VerifyInterface(pCRowset->pIRowset(), IID_IRowsetUpdate, ROWSET_INTERFACE, (IUnknown**)&pIRowsetUpdate));
			if(eAddColumns & OPT_BUFFEREDUPDATE)
			{
				//Changes are updated...
				TESTC_(pIRowsetUpdate->Update(NULL, 0, NULL, NULL, NULL, NULL),S_OK);
			}
			else
			{
				//Changes are undone...
				TESTC_(pIRowsetUpdate->Undo(NULL, 0, NULL, NULL, NULL, NULL),S_OK);
/*
				//If the user wanted to update/modify existing rowset columns, and we called
				//IRowsetUpdate::Undo, then those changes should have been rolled back and the 
				//verify will fail...
				if(eAddColumns & OPT_MODIFYCOLUMNS)
					bCompare = FALSE;
*/			}
			SAFE_RELEASE(pIRowsetUpdate);
		}

		//release the row handle
		TESTC_(pCRowset->ReleaseRows(cRowsObtained, rghRows),S_OK);
		PROVIDER_FREE(rghRows);
	}

CLEANUP:
	if(pCRowset)
		pCRowset->ReleaseRows(cRowsObtained, rghRows);
	PROVIDER_FREE(rghRows);
	SAFE_RELEASE(pIRowsetUpdate);
	TRETURN
}



////////////////////////////////////////////////////////////////////////////
//  TCIRowSchemaChange::Thread_VerifyAddColumns
//
////////////////////////////////////////////////////////////////////////////
ULONG TCIRowSchemaChange::Thread_VerifyAddColumns(void* pv)
{
	THREAD_BEGIN

	//Thread Stack Variables
	TCIRowSchemaChange* pThis		= (TCIRowSchemaChange*)THREAD_FUNC;
	CRowObject* pCRowObject			= (CRowObject*)THREAD_ARG1;
	ASSERT(pThis && pCRowObject);

	ThreadSwitch(); //Let the other thread(s) catch up

	//IRowSchemaChange::AddColumns
	QTESTC(pThis->VerifyAddColumns(pCRowObject, OPT_UPDATEANDADD, FIRST_ROW, UPDATEABLE_NONINDEX_COLS_BOUND, BLOB_LONG));
	
	ThreadSwitch(); //Let the other thread(s) catch up

CLEANUP:
	THREAD_RETURN
}


////////////////////////////////////////////////////////////////////////////
//  HRESULT TCIRowSchemaChange::DeleteColumns
//
////////////////////////////////////////////////////////////////////////////
HRESULT TCIRowSchemaChange::DeleteColumns
(
	CRowObject*			pCRowObject,
	DBORDINAL				cColAccess,
	DBCOLUMNACCESS*		rgColAccess
)
{
	TBEGIN
	HRESULT hr = S_OK;
	ASSERT(pCRowObject);
	IRowSchemaChange* pIRowSchemaChange = NULL;
	DBID* rgColumnIDs = NULL;
	DBSTATUS* rgdwStatus =  NULL;
	BOOL* rgfExtraColumn =  NULL;
	DBORDINAL i=0;

	//Obtain the IRowSchemaChange interface
	if(!VerifyInterface(pCRowObject->pIRow(), IID_IRowSchemaChange, ROW_INTERFACE, (IUnknown**)&pIRowSchemaChange))
	{
		hr = E_NOINTERFACE;
		goto CLEANUP;
	}

	//We need to convert the DBCOLUMNACCESS structures into a DBID and status array!
	//NOTE:  We pass in DBCOLUMNACCESS structures since thats how most of our methods operate.
	//We create DBCOLUMNACCESS structs, and have validation and comparision routines based upon them...
	SAFE_ALLOC(rgColumnIDs, DBID, cColAccess);
	SAFE_ALLOC(rgdwStatus, DBSTATUS, cColAccess);
	SAFE_ALLOC(rgfExtraColumn, BOOL, cColAccess);

	//Copy the info into our new arrays...
	for(i=0; i<cColAccess; i++)
	{
		//Can just to a memory copy since we are not freeing it
		rgColumnIDs[i] = rgColAccess[i].columnid;
		
		//dwStatus should not be looked at on input, but we may be verifying this
		//so copy anyway...
		rgdwStatus[i] = rgColAccess[i].dwStatus;

		//Before actually deleting the Column we need to mark wither it is a rowset
		//or row object column.  This is used to determine if we should expect the column
		//to be deleted (row object) or just set to NULL (rowset) after the deletion occurs 
		rgfExtraColumn[i] = pCRowObject->IsExtraColumn(&rgColAccess[i].columnid);
	}

	//IRowSchemaChange::DeleteColumns
	hr = pIRowSchemaChange->DeleteColumns(cColAccess, rgColumnIDs, rgdwStatus);

	//Need to update and verify the status'
	for(i=0; i<cColAccess; i++)
		rgColAccess[i].dwStatus = rgdwStatus[i];

	//First verify the column status against the return code...
	TESTC(VerifyColAccess(hr, cColAccess, rgColAccess));
	
	//Verify the columns are really deleted
	for(i=0; i<cColAccess; i++)
	{
		DBCOLUMNACCESS* pColAccess = &rgColAccess[i];
		DBID* pColumnID = &pColAccess->columnid;
		
		if(SUCCEEDED(hr))
		{
			if(rgfExtraColumn[i])
			{
				//Row Object Column should have been deleted
				TESTC(!::FindColInfo(pIRowSchemaChange, pColumnID));
			}
			else
			{
				//Rowset Column should have been NULL'ed...
				TESTC(::FindColInfo(pIRowSchemaChange, pColumnID));
				
				//Make sure the Data is NULL...
				TESTC_(pCRowObject->GetColumns(1, pColAccess),S_OK)
				if(pColAccess->dwStatus != DBSTATUS_S_ISNULL)
				{
					TERROR("Status = " << GetStatusName(pColAccess->dwStatus) << ", should be NULL data for rowset columns");
					VerifyColAccess(E_FAIL, 1, pColAccess);
				}
			}
		}
		else
		{
			//Column should NOT have been deleted
			TESTC(::FindColInfo(pIRowSchemaChange, pColumnID));
		}
	}

CLEANUP:
	SAFE_RELEASE(pIRowSchemaChange);
	SAFE_FREE(rgColumnIDs);
	SAFE_FREE(rgdwStatus);
	SAFE_FREE(rgfExtraColumn);
	return hr;
}


////////////////////////////////////////////////////////////////////////////
//  TCIRowSchemaChange::VerifyDeleteColumns
//
////////////////////////////////////////////////////////////////////////////
BOOL TCIRowSchemaChange::VerifyDeleteColumns
(
	CRowObject*			pCRowObject,
	DBCOUNTITEM				iRow,
	DBORDINAL				cColAccess,
	DBCOLUMNACCESS*		rgColAccess
)
{
	TBEGIN
	HRESULT hr = S_OK;
	ASSERT(pCRowObject);

	//IRowSchemaChange::DeleteColumns
	QTESTC_(hr = DeleteColumns(pCRowObject, cColAccess, rgColAccess),S_OK);

CLEANUP:
	TRETURN;
}


////////////////////////////////////////////////////////////////////////////
//  TCIRowSchemaChange::VerifyDeleteColumns
//
////////////////////////////////////////////////////////////////////////////
BOOL TCIRowSchemaChange::VerifyDeleteColumns
(
	CRowObject*			pCRowObject,
	DBCOUNTITEM				iRow,
	DWORD				dwColsToBind,			
	BLOBTYPE			dwBlobType,
	ECOLUMNORDER		eBindingOrder,		
	ECOLS_BY_REF		eColsByRef,				
	DBTYPE				dwModifier,
	DBORDINAL				cColsToBind,
	DBORDINAL*				rgColsToBind
)
{
	TBEGIN
	ASSERT(pCRowObject);
	HRESULT hr = S_OK;
	
	DBORDINAL cColAccess = 0;
	DBCOLUMNACCESS* rgColAccess = NULL;
	void* pData = NULL;

	//Create the ColAccess Structures...
	TESTC_(hr = pCRowObject->CreateColAccess(&cColAccess, &rgColAccess, &pData, NULL, dwColsToBind, dwBlobType, eBindingOrder, eColsByRef, dwModifier, cColsToBind, rgColsToBind),S_OK);

	//Verify DeleteColumns
	QTESTC(VerifyDeleteColumns(pCRowObject, iRow, cColAccess, rgColAccess));

CLEANUP:
	TRETURN;
}



////////////////////////////////////////////////////////////////////////////
//  TCIRowSchemaChange::VerifyDeleteColumnsAllRows
//
////////////////////////////////////////////////////////////////////////////
BOOL TCIRowSchemaChange::VerifyDeleteColumnsAllRows
(
	CRowsetChange*		pCRowset,
	DWORD				dwColsToBind,			
	BLOBTYPE			dwBlobType,
	ECOLUMNORDER		eBindingOrder,		
	ECOLS_BY_REF		eColsByRef,				
	DBTYPE				dwModifier,
	DBORDINAL				cColsToBind,
	DBORDINAL*				rgColsToBind
)
{
	TBEGIN
	HRESULT hr = S_OK;

	DBCOUNTITEM iRow,cRowsObtained = 0;
	HROW* rghRows = NULL;
	CRowsetChange RowsetA;
	IRowsetUpdate* pIRowsetUpdate = NULL;

	//Default rowset
	if(pCRowset == NULL)
	{
		pCRowset = &RowsetA;

		//May require IRowsetLocate to position on Blobs
		if(dwBlobType != NO_BLOB_COLS)
			pCRowset->SetSettableProperty(DBPROP_IRowsetLocate);
		TESTC_(pCRowset->CreateRowset(),S_OK);
	}

	//Restart the position.
	TESTC_(pCRowset->RestartPosition(),S_OK);

	//See if we are in bufferred mode
	VerifyInterface(pCRowset->pIRowset(), IID_IRowsetUpdate, ROWSET_INTERFACE, (IUnknown**)&pIRowsetUpdate);

	//loop through the rowset, retrieve one row at a time
	for(iRow=1; iRow<=pCRowset->m_ulTableRows; iRow++)	
	{
		//GetNextRow 
		CRowObject RowObjectA;
		TESTC_(pCRowset->GetNextRows(0, 1, &cRowsObtained, &rghRows),S_OK);
		
		//Create the row object from this row
		TEST2C_(RowObjectA.CreateRowObject(pCRowset->pIRowset(), rghRows[0]), S_OK, DB_S_NOROWSPECIFICCOLUMNS);

		//If the rowset is in buffered mode, it will require an Update, or Undo
		//before the change really takes effect.
		if(pIRowsetUpdate)
			TESTC_(pIRowsetUpdate->Update(NULL, 0, NULL, NULL, NULL, NULL),S_OK);

		//Verify Row Object
		QTESTC(VerifyDeleteColumns(&RowObjectA, iRow, dwColsToBind, dwBlobType, eBindingOrder, eColsByRef, dwModifier, cColsToBind, rgColsToBind));

		//release the row handle
		TESTC_(pCRowset->ReleaseRows(cRowsObtained, rghRows),S_OK);
		PROVIDER_FREE(rghRows);
	}

CLEANUP:
	if(pCRowset)
		pCRowset->ReleaseRows(cRowsObtained, rghRows);
	PROVIDER_FREE(rghRows);
	SAFE_RELEASE(pIRowsetUpdate);
	TRETURN
}



////////////////////////////////////////////////////////////////////////////
//  TCIRowSchemaChange::Thread_VerifyDeleteColumns
//
////////////////////////////////////////////////////////////////////////////
ULONG TCIRowSchemaChange::Thread_VerifyDeleteColumns(void* pv)
{
	THREAD_BEGIN

	//Thread Stack Variables
	TCIRowSchemaChange* pThis		= (TCIRowSchemaChange*)THREAD_FUNC;
	CRowObject* pCRowObject = (CRowObject*)THREAD_ARG1;
	ASSERT(pThis && pCRowObject);

	ThreadSwitch(); //Let the other thread(s) catch up

	//IRowSchemaChange::DeleteColumns
	QTESTC(pThis->VerifyDeleteColumns(pCRowObject, FIRST_ROW, UPDATEABLE_NONINDEX_COLS_BOUND, BLOB_LONG));
	
	ThreadSwitch(); //Let the other thread(s) catch up

CLEANUP:
	THREAD_RETURN
}


////////////////////////////////////////////////////////////////////////////
//  TCIRowSchemaChange::pIRowSchemaChange
//
////////////////////////////////////////////////////////////////////////////
IRowSchemaChange* const TCIRowSchemaChange::pRowSchemaChange()
{
	ASSERT(m_pIRowSchemaChange);
	return m_pIRowSchemaChange;
}


// {{ TCW_TEST_CASE_MAP(TCUnknown)
//*-----------------------------------------------------------------------
// @class IRowSchemaChange IUnknown scenarios
//
class TCUnknown : public TCIRowSchemaChange { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCUnknown,TCIRowSchemaChange);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember IUnknown - QI Mandatory Interfaces
	int Variation_1();
	// @cmember IUnknown - QI Optional Interfaces
	int Variation_2();
	// @cmember IUnknown - AddRef / Release
	int Variation_3();
	// @cmember Empty
	int Variation_4();
	// @cmember Inheritance - IRowSchemaChange::SetColumns
	int Variation_5();
	// @cmember Inheritance - QI for IRowChange and vise-versa
	int Variation_6();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(TCUnknown)
#define THE_CLASS TCUnknown
BEG_TEST_CASE(TCUnknown, TCIRowSchemaChange, L"IRowSchemaChange IUnknown scenarios")
	TEST_VARIATION(1, 		L"IUnknown - QI Mandatory Interfaces")
	TEST_VARIATION(2, 		L"IUnknown - QI Optional Interfaces")
	TEST_VARIATION(3, 		L"IUnknown - AddRef / Release")
	TEST_VARIATION(4, 		L"Empty")
	TEST_VARIATION(5, 		L"Inheritance - IRowSchemaChange::SetColumns")
	TEST_VARIATION(6, 		L"Inheritance - QI for IRowChange and vise-versa")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(TCDeleteColumns)
//*-----------------------------------------------------------------------
// @class IRowSchemaChange::DeleteColumns
//
class TCDeleteColumns : public TCIRowSchemaChange { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCDeleteColumns,TCIRowSchemaChange);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember General - All columns in a single call
	int Variation_1();
	// @cmember General - All columns in a single call with BLOBs
	int Variation_2();
	// @cmember General - Just BLOBs
	int Variation_3();
	// @cmember General - Each column seperatly
	int Variation_4();
	// @cmember General - Rowset common columns only
	int Variation_5();
	// @cmember General - Extra columns only
	int Variation_6();
	// @cmember General - 0, NULL - no-op
	int Variation_7();
	// @cmember General - Same column bound twice
	int Variation_8();
	// @cmember General - IUnknown columns
	int Variation_9();
	// @cmember Empty
	int Variation_10();
	// @cmember Boundary - read-only column
	int Variation_11();
	// @cmember Empty
	int Variation_12();
	// @cmember Sequence - Multiple row object - different rows
	int Variation_13();
	// @cmember Sequence - Multiple row object - same row
	int Variation_14();
	// @cmember Sequence - Verify rowset columns are intact on restart, and row object columns are removed
	int Variation_15();
	// @cmember Empty
	int Variation_16();
	// @cmember IRowsetUpdate - Delete All Columns - Verify - Update - Verify
	int Variation_17();
	// @cmember IRowsetUpdate - Delete All Columns with BLOBs - Verify - Update - Verify
	int Variation_18();
	// @cmember IRowsetUpdate - Delete only extra columns - Verify - Update - Verify
	int Variation_19();
	// @cmember IRowsetUpdate - Delete All Columns - Verify - Undo - Verify
	int Variation_20();
	// @cmember IRowsetUpdate - Delete All Columns with BLOBs - Verify - Undo - Verify
	int Variation_21();
	// @cmember IRowsetUpdate - Delete only extra columns - Verify - Undo - Verify
	int Variation_22();
	// @cmember Empty
	int Variation_23();
	// @cmember Threads - DeleteColumns from seperate threads, same row object
	int Variation_24();
	// @cmember Threads - DeleteColumns from seperate threads, diff row object
	int Variation_25();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(TCDeleteColumns)
#define THE_CLASS TCDeleteColumns
BEG_TEST_CASE(TCDeleteColumns, TCIRowSchemaChange, L"IRowSchemaChange::DeleteColumns")
	TEST_VARIATION(1, 		L"General - All columns in a single call")
	TEST_VARIATION(2, 		L"General - All columns in a single call with BLOBs")
	TEST_VARIATION(3, 		L"General - Just BLOBs")
	TEST_VARIATION(4, 		L"General - Each column seperatly")
	TEST_VARIATION(5, 		L"General - Rowset common columns only")
	TEST_VARIATION(6, 		L"General - Extra columns only")
	TEST_VARIATION(7, 		L"General - 0, NULL - no-op")
	TEST_VARIATION(8, 		L"General - Same column bound twice")
	TEST_VARIATION(9, 		L"General - IUnknown columns")
	TEST_VARIATION(10, 		L"Empty")
	TEST_VARIATION(11, 		L"Boundary - read-only column")
	TEST_VARIATION(12, 		L"Empty")
	TEST_VARIATION(13, 		L"Sequence - Multiple row object - different rows")
	TEST_VARIATION(14, 		L"Sequence - Multiple row object - same row")
	TEST_VARIATION(15, 		L"Sequence - Verify rowset columns are intact on restart, and row object columns are removed")
	TEST_VARIATION(16, 		L"Empty")
	TEST_VARIATION(17, 		L"IRowsetUpdate - Delete All Columns - Verify - Update - Verify")
	TEST_VARIATION(18, 		L"IRowsetUpdate - Delete All Columns with BLOBs - Verify - Update - Verify")
	TEST_VARIATION(19, 		L"IRowsetUpdate - Delete only extra columns - Verify - Update - Verify")
	TEST_VARIATION(20, 		L"IRowsetUpdate - Delete All Columns - Verify - Undo - Verify")
	TEST_VARIATION(21, 		L"IRowsetUpdate - Delete All Columns with BLOBs - Verify - Undo - Verify")
	TEST_VARIATION(22, 		L"IRowsetUpdate - Delete only extra columns - Verify - Undo - Verify")
	TEST_VARIATION(23, 		L"Empty")
	TEST_VARIATION(24, 		L"Threads - DeleteColumns from seperate threads, same row object")
	TEST_VARIATION(25, 		L"Threads - DeleteColumns from seperate threads, diff row object")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(TCAddColumns)
//*-----------------------------------------------------------------------
// @class IRowSchemaChange::AddColumns
//
class TCAddColumns : public TCIRowSchemaChange { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCAddColumns,TCIRowSchemaChange);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember General - All columns - no BLOBs
	int Variation_1();
	// @cmember General - All Columns - BLOBs
	int Variation_2();
	// @cmember General - just BLOBs
	int Variation_3();
	// @cmember General - each column seperatly
	int Variation_4();
	// @cmember Empty
	int Variation_5();
	// @cmember General - with existing rowset columns - ALREADYEXISTS
	int Variation_6();
	// @cmember General - with existing row columns - ALREADYEXISTS
	int Variation_7();
	// @cmember General - for only extra columns
	int Variation_8();
	// @cmember Empty
	int Variation_9();
	// @cmember General - 0 columns - no-op
	int Variation_10();
	// @cmember General - same column bound numerous times
	int Variation_11();
	// @cmember General - IUnknown columns
	int Variation_12();
	// @cmember Empty
	int Variation_13();
	// @cmember AddColumns - Not Binding Value for all columns, pData is NULL - DB_E_ERRORSOCCURRED
	int Variation_14();
	// @cmember AddColumns - Not Binding Value for some columns, pData is NULL - DB_S_ERRORSOCCURRED
	int Variation_15();
	// @cmember AddColumns - Not Binding Value for ISNULL columns, pData is NULL - S_OK
	int Variation_16();
	// @cmember AddColumns - Not Binding Value for BLOB columns, pData is NULL - S_OK
	int Variation_17();
	// @cmember Empty
	int Variation_18();
	// @cmember Boundary - Some valid, some non-existent columns - DB_S_ERRORSOCCURRED
	int Variation_19();
	// @cmember Boundary - All non-existent columns - DB_E_ERRORSOCCURRED
	int Variation_20();
	// @cmember Boundary - No Vector Columns - S_OK
	int Variation_21();
	// @cmember Boundary - No Vectors and Non-Existent Columns - DB_E_ERRORSOCCURRED
	int Variation_22();
	// @cmember Boundary - Only Vector Columns - S_OK
	int Variation_23();
	// @cmember Boundary - Only Non-Existent Vector Columns - DB_E_ERRORSOCCURRED
	int Variation_24();
	// @cmember Boundary - Valid Vectors and Non-Existent Columns - DB_S_ERRORSOCCURRED
	int Variation_25();
	// @cmember Boundary - Valid Non-Vectors and Non-Existent Vector Columns - DB_S_ERRORSOCCURRED
	int Variation_26();
	// @cmember Empty
	int Variation_27();
	// @cmember Boundary - 0 columns -[0, NULL] and [0, valid]  no-op
	int Variation_28();
	// @cmember Boundary - [Valid, NULL] - E_INVALIDARG
	int Variation_29();
	// @cmember Boundary - columnid - all valid dbkinds
	int Variation_30();
	// @cmember Boundary - columnid - invalid dbkinds
	int Variation_31();
	// @cmember Boundary - cbMaxLen - ignored for fixed length types
	int Variation_32();
	// @cmember Boundary - cbMaxLen - required for variable length types
	int Variation_33();
	// @cmember Boundary - cbMaxLen - DBSTATUS_S_TRUNCATION
	int Variation_34();
	// @cmember Boundary - wType - all valid types and modifieres
	int Variation_35();
	// @cmember Boundary - wType - invalid types and invalid type modifiers
	int Variation_36();
	// @cmember Boundary - bPrecision - make sure ignored on input for all types, except NUMERIC.
	int Variation_37();
	// @cmember Boundary - bScale - make sure ignored on input for all types, except NUMERIC.
	int Variation_38();
	// @cmember Boundary - Output Only - make sure cbDataLen, dwStatus, dwReserverd are ignored on input
	int Variation_39();
	// @cmember Boundary - Input - Make sure all pointers and input args are not changed on output, pData pointer, columnid [including all union pointers], cbMaxLen, dwReserved, wType.
	int Variation_40();
	// @cmember Empty
	int Variation_41();
	// @cmember Boundary - DBCOLUMNINFO.iOrdinal is ignored
	int Variation_42();
	// @cmember Boundary - DBCOLUMNINFO.pwszName is ignored
	int Variation_43();
	// @cmember Boundary - DBCOLUMNINFO.cbMaxLen is ignored
	int Variation_44();
	// @cmember Empty
	int Variation_45();
	// @cmember Parameters - DBSTATUS_S_OK - add column and provide data all in one call
	int Variation_46();
	// @cmember Parameters - DBSTATUS_S_IGNORE - add column only
	int Variation_47();
	// @cmember Parameters - DBSTATUS_S_IGNORE - NULL rgColAccess for only meta data
	int Variation_48();
	// @cmember Parameters - DBSTATUS_S_IGNORE - add column only - pData == NULL
	int Variation_49();
	// @cmember Parameters - DBSTATUS_S_IGNORE - add column only
	int Variation_50();
	// @cmember Parameters - DBSTATUS_S_IGNORE - add column only - pData == NULL
	int Variation_51();
	// @cmember Parameters - DBSTATUS_S_ISNULL - add column - null data
	int Variation_52();
	// @cmember Parameters - DBSTATUS_S_ISNULL - add column - pData == NULL
	int Variation_53();
	// @cmember Parameters - DBSTATUS_S_DEFAULT - add column only
	int Variation_54();
	// @cmember Parameters - DBSTATUS_S_DEFAULT - add column only - pData == NULL
	int Variation_55();
	// @cmember Empty
	int Variation_56();
	// @cmember Parameters - DBCOLUMNFLAGS_WRITE - Writable column - with data
	int Variation_57();
	// @cmember Parameters - DBCOLUMNFLAGS_WRITE - Read-only column - ignore data
	int Variation_58();
	// @cmember Parameters - DBCOLUMNFLAGS_ISNULLABLE - nullable column - isnull data
	int Variation_59();
	// @cmember Parameters - DBCOLUMNFLAGS_ISNULLABLE - non-nullable column - ignore
	int Variation_60();
	// @cmember Parameters - DBCOLUMNFLAGS_ISFIXEDLENGTH - fixed length column - default data
	int Variation_61();
	// @cmember Parameters - DBCOLUMNFLAGS_ISFIXEDLENGTH - non-fixed length column - default data
	int Variation_62();
	// @cmember Parameters - DBCOLUMNFLAGS Combinations - WRITE | ISNULLABLE - null data
	int Variation_63();
	// @cmember Parameters - DBCOLUMNFLAGS Combinations - WRITE | ISNULLABLE| FIXEDLENGTH - data
	int Variation_64();
	// @cmember Parameters - DBCOLUMNFLAGS Combinations - WRITE | FIXEDLENGTH - default data
	int Variation_65();
	// @cmember Parameters - DBCOLUMNFLAGS Combinations - FIXEDLENGTH - ignore data
	int Variation_66();
	// @cmember Empty
	int Variation_67();
	// @cmember Sequence - Multiple row object - different rows - same columnids
	int Variation_68();
	// @cmember Sequence - Multiple row object - same row - different columnids
	int Variation_69();
	// @cmember Empty
	int Variation_70();
	// @cmember IRowsetUpdate - AddColumns with existing row object columns -  verify - update - verify
	int Variation_71();
	// @cmember IRowsetUpdate - AddColumns with existing rowset columns -  verify - update - verify
	int Variation_72();
	// @cmember IRowsetUpdate - AddColumns with existing row object columns -  verify - undo - verify
	int Variation_73();
	// @cmember IRowsetUpdate - AddColumns with existing rowset columns -  verify - undo - verify
	int Variation_74();
	// @cmember Empty
	int Variation_75();
	// @cmember Threads - AddColumns - same row object
	int Variation_76();
	// @cmember Threads - AddColumns - diff row object
	int Variation_77();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(TCAddColumns)
#define THE_CLASS TCAddColumns
BEG_TEST_CASE(TCAddColumns, TCIRowSchemaChange, L"IRowSchemaChange::AddColumns")
	TEST_VARIATION(1, 		L"General - All columns - no BLOBs")
	TEST_VARIATION(2, 		L"General - All Columns - BLOBs")
	TEST_VARIATION(3, 		L"General - just BLOBs")
	TEST_VARIATION(4, 		L"General - each column seperatly")
	TEST_VARIATION(5, 		L"Empty")
	TEST_VARIATION(6, 		L"General - with existing rowset columns - ALREADYEXISTS")
	TEST_VARIATION(7, 		L"General - with existing row columns - ALREADYEXISTS")
	TEST_VARIATION(8, 		L"General - for only extra columns")
	TEST_VARIATION(9, 		L"Empty")
	TEST_VARIATION(10, 		L"General - 0 columns - no-op")
	TEST_VARIATION(11, 		L"General - same column bound numerous times")
	TEST_VARIATION(12, 		L"General - IUnknown columns")
	TEST_VARIATION(13, 		L"Empty")
	TEST_VARIATION(14, 		L"AddColumns - Not Binding Value for all columns, pData is NULL - DB_E_ERRORSOCCURRED")
	TEST_VARIATION(15, 		L"AddColumns - Not Binding Value for some columns, pData is NULL - DB_S_ERRORSOCCURRED")
	TEST_VARIATION(16, 		L"AddColumns - Not Binding Value for ISNULL columns, pData is NULL - S_OK")
	TEST_VARIATION(17, 		L"AddColumns - Not Binding Value for BLOB columns, pData is NULL - S_OK")
	TEST_VARIATION(18, 		L"Empty")
	TEST_VARIATION(19, 		L"Boundary - Some valid, some non-existent columns - DB_S_ERRORSOCCURRED")
	TEST_VARIATION(20, 		L"Boundary - All non-existent columns - DB_E_ERRORSOCCURRED")
	TEST_VARIATION(21, 		L"Boundary - No Vector Columns - S_OK")
	TEST_VARIATION(22, 		L"Boundary - No Vectors and Non-Existent Columns - DB_E_ERRORSOCCURRED")
	TEST_VARIATION(23, 		L"Boundary - Only Vector Columns - S_OK")
	TEST_VARIATION(24, 		L"Boundary - Only Non-Existent Vector Columns - DB_E_ERRORSOCCURRED")
	TEST_VARIATION(25, 		L"Boundary - Valid Vectors and Non-Existent Columns - DB_S_ERRORSOCCURRED")
	TEST_VARIATION(26, 		L"Boundary - Valid Non-Vectors and Non-Existent Vector Columns - DB_S_ERRORSOCCURRED")
	TEST_VARIATION(27, 		L"Empty")
	TEST_VARIATION(28, 		L"Boundary - 0 columns -[0, NULL] and [0, valid]  no-op")
	TEST_VARIATION(29, 		L"Boundary - [Valid, NULL] - E_INVALIDARG")
	TEST_VARIATION(30, 		L"Boundary - columnid - all valid dbkinds")
	TEST_VARIATION(31, 		L"Boundary - columnid - invalid dbkinds")
	TEST_VARIATION(32, 		L"Boundary - cbMaxLen - ignored for fixed length types")
	TEST_VARIATION(33, 		L"Boundary - cbMaxLen - required for variable length types")
	TEST_VARIATION(34, 		L"Boundary - cbMaxLen - DBSTATUS_S_TRUNCATION")
	TEST_VARIATION(35, 		L"Boundary - wType - all valid types and modifieres")
	TEST_VARIATION(36, 		L"Boundary - wType - invalid types and invalid type modifiers")
	TEST_VARIATION(37, 		L"Boundary - bPrecision - make sure ignored on input for all types, except NUMERIC.")
	TEST_VARIATION(38, 		L"Boundary - bScale - make sure ignored on input for all types, except NUMERIC.")
	TEST_VARIATION(39, 		L"Boundary - Output Only - make sure cbDataLen, dwStatus, dwReserverd are ignored on input")
	TEST_VARIATION(40, 		L"Boundary - Input - Make sure all pointers and input args are not changed on output, pData pointer, columnid [including all union pointers], cbMaxLen, dwReserved, wType.")
	TEST_VARIATION(41, 		L"Empty")
	TEST_VARIATION(42, 		L"Boundary - DBCOLUMNINFO.iOrdinal is ignored")
	TEST_VARIATION(43, 		L"Boundary - DBCOLUMNINFO.pwszName is ignored")
	TEST_VARIATION(44, 		L"Boundary - DBCOLUMNINFO.cbMaxLen is ignored")
	TEST_VARIATION(45, 		L"Empty")
	TEST_VARIATION(46, 		L"Parameters - DBSTATUS_S_OK - add column and provide data all in one call")
	TEST_VARIATION(47, 		L"Parameters - DBSTATUS_S_IGNORE - add column only")
	TEST_VARIATION(48, 		L"Parameters - DBSTATUS_S_IGNORE - NULL rgColAccess for only meta data")
	TEST_VARIATION(49, 		L"Parameters - DBSTATUS_S_IGNORE - add column only - pData == NULL")
	TEST_VARIATION(50, 		L"Parameters - DBSTATUS_S_IGNORE - add column only")
	TEST_VARIATION(51, 		L"Parameters - DBSTATUS_S_IGNORE - add column only - pData == NULL")
	TEST_VARIATION(52, 		L"Parameters - DBSTATUS_S_ISNULL - add column - null data")
	TEST_VARIATION(53, 		L"Parameters - DBSTATUS_S_ISNULL - add column - pData == NULL")
	TEST_VARIATION(54, 		L"Parameters - DBSTATUS_S_DEFAULT - add column only")
	TEST_VARIATION(55, 		L"Parameters - DBSTATUS_S_DEFAULT - add column only - pData == NULL")
	TEST_VARIATION(56, 		L"Empty")
	TEST_VARIATION(57, 		L"Parameters - DBCOLUMNFLAGS_WRITE - Writable column - with data")
	TEST_VARIATION(58, 		L"Parameters - DBCOLUMNFLAGS_WRITE - Read-only column - ignore data")
	TEST_VARIATION(59, 		L"Parameters - DBCOLUMNFLAGS_ISNULLABLE - nullable column - isnull data")
	TEST_VARIATION(60, 		L"Parameters - DBCOLUMNFLAGS_ISNULLABLE - non-nullable column - ignore")
	TEST_VARIATION(61, 		L"Parameters - DBCOLUMNFLAGS_ISFIXEDLENGTH - fixed length column - default data")
	TEST_VARIATION(62, 		L"Parameters - DBCOLUMNFLAGS_ISFIXEDLENGTH - non-fixed length column - default data")
	TEST_VARIATION(63, 		L"Parameters - DBCOLUMNFLAGS Combinations - WRITE | ISNULLABLE - null data")
	TEST_VARIATION(64, 		L"Parameters - DBCOLUMNFLAGS Combinations - WRITE | ISNULLABLE| FIXEDLENGTH - data")
	TEST_VARIATION(65, 		L"Parameters - DBCOLUMNFLAGS Combinations - WRITE | FIXEDLENGTH - default data")
	TEST_VARIATION(66, 		L"Parameters - DBCOLUMNFLAGS Combinations - FIXEDLENGTH - ignore data")
	TEST_VARIATION(67, 		L"Empty")
	TEST_VARIATION(68, 		L"Sequence - Multiple row object - different rows - same columnids")
	TEST_VARIATION(69, 		L"Sequence - Multiple row object - same row - different columnids")
	TEST_VARIATION(70, 		L"Empty")
	TEST_VARIATION(71, 		L"IRowsetUpdate - AddColumns with existing row object columns -  verify - update - verify")
	TEST_VARIATION(72, 		L"IRowsetUpdate - AddColumns with existing rowset columns -  verify - update - verify")
	TEST_VARIATION(73, 		L"IRowsetUpdate - AddColumns with existing row object columns -  verify - undo - verify")
	TEST_VARIATION(74, 		L"IRowsetUpdate - AddColumns with existing rowset columns -  verify - undo - verify")
	TEST_VARIATION(75, 		L"Empty")
	TEST_VARIATION(76, 		L"Threads - AddColumns - same row object")
	TEST_VARIATION(77, 		L"Threads - AddColumns - diff row object")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCTransactions)
//*-----------------------------------------------------------------------
// @class IRowSchemaChange inside Transactions
//
class TCTransactions : public TCIRowSchemaChange { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCTransactions,TCIRowSchemaChange);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember DeleteColumns - ABORT with fRetaining TRUE
	int Variation_1();
	// @cmember DeleteColumns - ABORT with fRetaining FALSE
	int Variation_2();
	// @cmember DeleteColumns - COMMIT with fRetaining TRUE
	int Variation_3();
	// @cmember DeleteColumns - COMMIT with fRetaining FALSE
	int Variation_4();
	// @cmember Empty
	int Variation_5();
	// @cmember AddColumns - ABORT with fRetaining TRUE
	int Variation_6();
	// @cmember AddColumns - ABORT with fRetaining FALSE
	int Variation_7();
	// @cmember AddColumns - COMMIT with fRetaining TRUE
	int Variation_8();
	// @cmember AddColumns - COMMIT with fRetaining FALSE
	int Variation_9();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(TCTransactions)
#define THE_CLASS TCTransactions
BEG_TEST_CASE(TCTransactions, TCIRowSchemaChange, L"IRowSchemaChange inside Transactions")
	TEST_VARIATION(1, 		L"DeleteColumns - ABORT with fRetaining TRUE")
	TEST_VARIATION(2, 		L"DeleteColumns - ABORT with fRetaining FALSE")
	TEST_VARIATION(3, 		L"DeleteColumns - COMMIT with fRetaining TRUE")
	TEST_VARIATION(4, 		L"DeleteColumns - COMMIT with fRetaining FALSE")
	TEST_VARIATION(5, 		L"Empty")
	TEST_VARIATION(6, 		L"AddColumns - ABORT with fRetaining TRUE")
	TEST_VARIATION(7, 		L"AddColumns - ABORT with fRetaining FALSE")
	TEST_VARIATION(8, 		L"AddColumns - COMMIT with fRetaining TRUE")
	TEST_VARIATION(9, 		L"AddColumns - COMMIT with fRetaining FALSE")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END



// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Test Case Section
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// }} END_DECLARE_TEST_CASES()

// {{ TCW_TESTMODULE(ThisModule)
TEST_MODULE(4, ThisModule, gwszModuleDescrip)
	TEST_CASE(1, TCUnknown)
	TEST_CASE(2, TCDeleteColumns)
	TEST_CASE(3, TCAddColumns)
	TEST_CASE(4, TCTransactions)
END_TEST_MODULE()
// }} TCW_TESTMODULE_END

// {{ TCW_TC_PROTOTYPE(TCUnknown)
//*-----------------------------------------------------------------------
//| Test Case:		TCUnknown - IRowSchemaChange IUnknown scenarios
//| Created:  	8/5/98
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCUnknown::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCIRowSchemaChange::Init())
	// }}
	{ 
		return TRUE;
	} 
	return FALSE;
} 




// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc IUnknown - QI Mandatory Interfaces
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCUnknown::Variation_1()
{ 
	//Do some default IUnknown interface testing
	return DefaultObjectTesting(pRowSchemaChange(), ROW_INTERFACE);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc IUnknown - QI Optional Interfaces
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCUnknown::Variation_2()
{ 
	//Do some default IUnknown interface testing
	return DefaultObjectTesting(pRowSchemaChange(), ROW_INTERFACE);
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc IUnknown - AddRef / Release
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCUnknown::Variation_3()
{ 
	//Do some default IUnknown interface testing
	return DefaultObjectTesting(pRowSchemaChange(), ROW_INTERFACE);
} 
// }} TCW_VAR_PROTOTYPE_END





// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCUnknown::Variation_4()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Inheritance - IRowSchemaChange::SetColumns
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCUnknown::Variation_5()
{ 
	TBEGIN

	//IRowSchemaChange::SetColumns
	TESTC_(pRowSchemaChange()->SetColumns(0, NULL), S_OK);
	TESTC_(pRowSchemaChange()->SetColumns(1, NULL), E_INVALIDARG);

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Inheritance - QI for IRowChange and vise-versa
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCUnknown::Variation_6()
{ 
	TBEGIN
	IRowChange* pIRowChange = NULL;
	IRowSchemaChange* pIRowSchemaChange = NULL;

	//IRowSchemaChange -> IRowChange
	TESTC_(QI(pRowSchemaChange(), IID_IRowChange, (void**)&pIRowChange), S_OK);
	TESTC_(pIRowChange->SetColumns(0, NULL), S_OK);
	TESTC_(pIRowChange->SetColumns(1, NULL), E_INVALIDARG);
	TESTC(DefaultObjectTesting(pIRowChange, ROW_INTERFACE));
	
	//IRowChange -> IRowSchemaChange
	TESTC_(QI(pIRowChange, IID_IRowSchemaChange, (void**)&pIRowSchemaChange), S_OK);
	TESTC_(pIRowSchemaChange->DeleteColumns(0, NULL, NULL), S_OK);
	TESTC_(pIRowSchemaChange->DeleteColumns(1, NULL, NULL), E_INVALIDARG);
	TESTC_(pIRowSchemaChange->AddColumns(0, NULL, NULL), S_OK);
	TESTC_(pIRowSchemaChange->AddColumns(1, NULL, NULL), E_INVALIDARG);
	TESTC(DefaultObjectTesting(pIRowSchemaChange, ROW_INTERFACE));

CLEANUP:
	SAFE_RELEASE(pIRowChange);
	SAFE_RELEASE(pIRowSchemaChange);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCUnknown::Terminate()
{ 
// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIRowSchemaChange::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END



// {{ TCW_TC_PROTOTYPE(TCDeleteColumns)
//*-----------------------------------------------------------------------
//| Test Case:		TCDeleteColumns - IRowSchemaChange::DeleteColumns
//| Created:  	9/30/98
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCDeleteColumns::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCIRowSchemaChange::Init())
	// }}
	{ 
		// TO DO:  Add your own code here 
		return TRUE;
	} 
	return FALSE;
} 






// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc General - All columns in a single call
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCDeleteColumns::Variation_1()
{ 
	TBEGIN

	//DeleteColumns
	TESTC(VerifyDeleteColumnsAllRows(NULL, UPDATEABLE_COLS_BOUND | NONINDEX_COLS_BOUND | NULLABLE_COLS_BOUND));
	
CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc General - All columns in a single call with BLOBs
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCDeleteColumns::Variation_2()
{ 
	TBEGIN

	//DeleteColumns
	TESTC(VerifyDeleteColumnsAllRows(NULL, UPDATEABLE_COLS_BOUND | NONINDEX_COLS_BOUND | NULLABLE_COLS_BOUND, BLOB_LONG));
	
CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc General - Just BLOBs
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCDeleteColumns::Variation_3()
{ 
	TBEGIN

	//DeleteColumns
	TESTC(VerifyDeleteColumnsAllRows(NULL, BLOB_COLS_BOUND, BLOB_LONG));
	
CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc General - Each column seperatly
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCDeleteColumns::Variation_4()
{ 
	TBEGIN
	IColumnsInfo* pIColumnsInfo = NULL;
	DBORDINAL iCol,cColumns=0;	
	DBCOLUMNINFO* rgColumnInfo = NULL;
	WCHAR* pStringBuffer = NULL;

	//Get the ColumnInfo
	TESTC(VerifyInterface(pRowSchemaChange(), IID_IColumnsInfo, ROW_INTERFACE, (IUnknown**)&pIColumnsInfo));
	TESTC_(pIColumnsInfo->GetColumnInfo(&cColumns, &rgColumnInfo, &pStringBuffer),S_OK);

	//Loop through each column seperatly...
	for(iCol=0; iCol<cColumns; iCol++)
	{
		//Loop through all the rows in the rowset, verify the columns...
		if((rgColumnInfo[iCol].dwFlags & DBCOLUMNFLAGS_WRITE) && (rgColumnInfo[iCol].dwFlags & DBCOLUMNFLAGS_ISNULLABLE))
		{
			if(!VerifyDeleteColumnsAllRows(NULL, USE_COLS_TO_BIND_ARRAY, BLOB_LONG, FORWARD,NO_COLS_BY_REF,	DBTYPE_EMPTY, 1, &rgColumnInfo[iCol].iOrdinal))
			{
				//Data incorrect for this column!
				TERROR("Unable to delete this column, ordinal = " << rgColumnInfo[iCol].iOrdinal);
			}
		}
	}

CLEANUP:
	SAFE_FREE(rgColumnInfo);
	SAFE_FREE(pStringBuffer);
	SAFE_RELEASE(pIColumnsInfo);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc General - Rowset common columns only
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCDeleteColumns::Variation_5()
{ 
	TBEGIN
	HRESULT hr = S_OK;
	DBORDINAL cColAccess = 0;
	DBCOLUMNACCESS* rgColAccess = NULL;
	HROW hRow = NULL;

	CRowObject RowObjectA;
	CRowsetChange RowsetA;
	TESTC_(RowsetA.CreateRowset(USE_SUPPORTED_SELECT_ALLFROMTBL, IID_IRowset, NULL, DBACCESSOR_ROWDATA, DBPART_ALL, UPDATEABLE_COLS_BOUND | NONINDEX_COLS_BOUND | NULLABLE_COLS_BOUND),S_OK);
	
	//Obtain row object
	TESTC_(RowsetA.GetNextRows(&hRow),S_OK);
	TEST2C_(RowObjectA.CreateRowObject(RowsetA.pIRowset(), hRow),S_OK, DB_S_NOROWSPECIFICCOLUMNS);

	//Create ColAccess structures from the Rowset bindings
	TESTC_(hr = RowObjectA.BindingsToColAccess(RowsetA.m_cBindings, RowsetA.m_rgBinding, RowsetA.m_pData, &cColAccess, &rgColAccess),S_OK);
	
	//Verify DeleteColumns
	TESTC(VerifyDeleteColumns(&RowObjectA, FIRST_ROW, cColAccess, rgColAccess));

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	FreeColAccess(cColAccess, rgColAccess);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc General - Extra columns only
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCDeleteColumns::Variation_6()
{ 
	TBEGIN
	HRESULT hr = S_OK;
	DBORDINAL cColAccess = 0;
	DBCOLUMNACCESS* rgColAccess = NULL;
	void* pData = NULL;
	HROW hRow = NULL;

	DBORDINAL cColumns = 0;
	DBORDINAL* rgColOrdinals = NULL;

	CRowObject RowObjectA;
	CRowsetChange RowsetA;
	TESTC_(RowsetA.CreateRowset(),S_OK);

	//Obtain row object
	TESTC_(RowsetA.GetNextRows(&hRow),S_OK);
	TEST2C_(RowObjectA.CreateRowObject(RowsetA.pIRowset(), hRow),S_OK, DB_S_NOROWSPECIFICCOLUMNS);

	//Obtain the just the Extra columns
	TESTC_(hr = RowObjectA.GetExtraColumnInfo(&cColumns, NULL, NULL, &rgColOrdinals),S_OK);

	//Create the ColAccess Structures for just the extra columns...
	TESTC_(hr = RowObjectA.CreateColAccess(&cColAccess, &rgColAccess, &pData, NULL, USE_COLS_TO_BIND_ARRAY | UPDATEABLE_COLS_BOUND, BLOB_LONG, FORWARD, NO_COLS_BY_REF, DBTYPE_EMPTY, cColumns, rgColOrdinals),S_OK);
							 
	//Verify DeleteColumns
	TESTC(VerifyDeleteColumns(&RowObjectA, FIRST_ROW, cColAccess, rgColAccess));

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	FreeColAccess(cColAccess, rgColAccess);
	SAFE_FREE(pData);
	SAFE_FREE(rgColOrdinals);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc General - 0, NULL - no-op
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCDeleteColumns::Variation_7()
{ 
	TBEGIN
	
	//DeleteColumns - with (0 NULL)
	TESTC(VerifyDeleteColumnsAllRows(NULL, USE_COLS_TO_BIND_ARRAY, BLOB_LONG, FORWARD,
				NO_COLS_BY_REF,	DBTYPE_EMPTY, 0, NULL));

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc General - Same column bound twice
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCDeleteColumns::Variation_8()
{ 
	TBEGIN
	IColumnsInfo* pIColumnsInfo = NULL;
	DBORDINAL iCol,cColumns=0;	
	DBCOLUMNINFO* rgColumnInfo = NULL;
	WCHAR* pStringBuffer = NULL;
	DBORDINAL  cColOrds = 0;
	DBORDINAL* rgColOrds = NULL;

	//Use a new rowset, and ask for a non-forward-only cursor, 
	//so we can obtain the data multiple times.
	CRowsetChange RowsetA;
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_OTHERUPDATEDELETE)==S_OK);

	//Get the ColumnInfo
	TESTC(VerifyInterface(pRowSchemaChange(), IID_IColumnsInfo, ROW_INTERFACE, (IUnknown**)&pIColumnsInfo));
	TESTC_(pIColumnsInfo->GetColumnInfo(&cColumns, &rgColumnInfo, &pStringBuffer),S_OK);
	SAFE_ALLOC(rgColOrds, DBORDINAL, cColumns);

	//Loop through each column seperatly...
	for(iCol=0; iCol<cColumns; iCol++)
	{
		//Fill in the Col Ordinals with numerous duplicates
		cColOrds = 0;
		for(ULONG iDup=0; iDup<iCol; iDup++)
		{
			if((rgColumnInfo[iCol].dwFlags & DBCOLUMNFLAGS_WRITE) && (rgColumnInfo[iCol].dwFlags & DBCOLUMNFLAGS_ISNULLABLE))
			{
				rgColOrds[iDup] = rgColumnInfo[iCol].iOrdinal;
				cColOrds++;	
			}
		}
		
		//Loop through all the rows in the rowset, verify the columns...
		TESTC(VerifyDeleteColumnsAllRows(&RowsetA, USE_COLS_TO_BIND_ARRAY, BLOB_LONG, FORWARD,
					NO_COLS_BY_REF,	DBTYPE_EMPTY, cColOrds, rgColOrds));
	}

CLEANUP:
	SAFE_FREE(rgColumnInfo);
	SAFE_FREE(pStringBuffer);
	SAFE_RELEASE(pIColumnsInfo);
	SAFE_FREE(rgColOrds);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc General - IUnknown columns
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCDeleteColumns::Variation_9()
{ 
	TBEGIN
	
	//Loop through all the rows in the rowset, verify the columns...
	TESTC(VerifyDeleteColumnsAllRows(NULL, BLOB_COLS_BOUND, BLOB_IID_IUNKNOWN));

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCDeleteColumns::Variation_10()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc Boundary - read-only column
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCDeleteColumns::Variation_11()
{ 
	TBEGIN
	IColumnsInfo* pIColumnsInfo = NULL;
	DBORDINAL iCol,cColumns=0;	
	DBCOLUMNINFO* rgColumnInfo = NULL;
	WCHAR* pStringBuffer = NULL;
	HROW hRow = NULL;
	HRESULT hr = S_OK;

	//Use a new rowset, and ask for a non-forward-only cursor, 
	//so we can obtain the data multiple times.
	CRowObject RowObjectA;
	CRowsetChange RowsetA;
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_OTHERUPDATEDELETE)==S_OK);

	//Obtain row object
	TESTC_(RowsetA.GetNextRows(&hRow),S_OK);
	TEST2C_(RowObjectA.CreateRowObject(RowsetA.pIRowset(), hRow),S_OK, DB_S_NOROWSPECIFICCOLUMNS);

	//Get the ColumnInfo
	TESTC(VerifyInterface(pRowSchemaChange(), IID_IColumnsInfo, ROW_INTERFACE, (IUnknown**)&pIColumnsInfo));
	TESTC_(pIColumnsInfo->GetColumnInfo(&cColumns, &rgColumnInfo, &pStringBuffer),S_OK);

	//Loop through all columns, trying to find a readonly column...
	for(iCol=0; iCol<cColumns; iCol++)
	{
		if(!(rgColumnInfo[iCol].dwFlags & DBCOLUMNFLAGS_WRITE) && !(rgColumnInfo[iCol].dwFlags & DBCOLUMNFLAGS_WRITEUNKNOWN))
		{
			//setup rgColAccess
			const ULONG cColAccess = 1;
			DBCOLUMNACCESS rgColAccess[cColAccess];
			rgColAccess[0].columnid = rgColumnInfo[iCol].columnid;
			
			//VerifyDeleteColumns
			//If the provider could up front determine permission denied for the entire row, or 
			//permisiion denied for individual columns...
			TEST2C_(hr = DeleteColumns(&RowObjectA, cColAccess, rgColAccess), DB_SEC_E_PERMISSIONDENIED, DB_E_ERRORSOCCURRED);
			if(hr==DB_E_ERRORSOCCURRED)
			{
				for(ULONG i=0; i<cColAccess; i++)
				{
					if(rgColAccess[i].dwStatus != DBSTATUS_E_PERMISSIONDENIED)
						TERROR("Status returned: " << GetStatusName(rgColAccess[i].dwStatus) << " Status expected: " << GetStatusName(DBSTATUS_E_PERMISSIONDENIED));
				}
			}
		}
	}


CLEANUP:
	SAFE_FREE(rgColumnInfo);
	SAFE_FREE(pStringBuffer);
	SAFE_RELEASE(pIColumnsInfo);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCDeleteColumns::Variation_12()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc Sequence - Multiple row object - different rows
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCDeleteColumns::Variation_13()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc Sequence - Multiple row object - same row
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCDeleteColumns::Variation_14()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc Sequence - Verify rowset columns are intact on restart, and row object columns are removed
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCDeleteColumns::Variation_15()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCDeleteColumns::Variation_16()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc IRowsetUpdate - Delete All Columns - Verify - Update - Verify
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCDeleteColumns::Variation_17()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc IRowsetUpdate - Delete All Columns with BLOBs - Verify - Update - Verify
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCDeleteColumns::Variation_18()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc IRowsetUpdate - Delete only extra columns - Verify - Update - Verify
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCDeleteColumns::Variation_19()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc IRowsetUpdate - Delete All Columns - Verify - Undo - Verify
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCDeleteColumns::Variation_20()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc IRowsetUpdate - Delete All Columns with BLOBs - Verify - Undo - Verify
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCDeleteColumns::Variation_21()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(22)
//*-----------------------------------------------------------------------
// @mfunc IRowsetUpdate - Delete only extra columns - Verify - Undo - Verify
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCDeleteColumns::Variation_22()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(23)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCDeleteColumns::Variation_23()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(24)
//*-----------------------------------------------------------------------
// @mfunc Threads - DeleteColumns from seperate threads, same row object
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCDeleteColumns::Variation_24()
{ 
	TBEGIN
	INIT_THREADS(MAX_THREADS);	
	
	CRowObject RowObjectA;
	CRowsetChange RowsetA;
	HROW hRow = NULL;

	//Setup Thread Arguments
	THREADARG T1Arg = { this, &RowObjectA };
	
	//Create Rowset object
	RowsetA.SetSupportedProperty(DBPROP_IRowsetLocate);
	RowsetA.SetProperty(DBPROP_CANHOLDROWS);
	TESTC_(RowsetA.CreateRowset(),S_OK);

	//Obtain row object
	TESTC_(RowsetA.GetNextRows(&hRow),S_OK);
	TEST2C_(RowObjectA.CreateRowObject(RowsetA.pIRowset(), hRow),S_OK, DB_S_NOROWSPECIFICCOLUMNS);

	//Create Threads
	CREATE_THREADS(Thread_VerifyDeleteColumns, &T1Arg);

	START_THREADS();
	END_THREADS();	

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(25)
//*-----------------------------------------------------------------------
// @mfunc Threads - DeleteColumns from seperate threads, diff row object
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCDeleteColumns::Variation_25()
{ 
	TBEGIN
	INIT_THREADS(MAX_THREADS);	
	
	CRowObject RowObjectA;
	CRowObject RowObjectB;
	CRowsetChange RowsetA;
	HROW rghRows[TWO_ROWS];

	//Setup Thread Arguments
	THREADARG T1Arg = { this, &RowObjectA };
	THREADARG T2Arg = { this, &RowObjectB };
	
	//Create Rowset object
	RowsetA.SetSupportedProperty(DBPROP_IRowsetLocate);
	RowsetA.SetProperty(DBPROP_CANHOLDROWS);
	TESTC_(RowsetA.CreateRowset(),S_OK);

	//Obtain row object
	TESTC_(RowsetA.GetNextRows(TWO_ROWS, rghRows),S_OK);
	TEST2C_(RowObjectA.CreateRowObject(RowsetA.pIRowset(), rghRows[ROW_ONE]),S_OK, DB_S_NOROWSPECIFICCOLUMNS);
	TEST2C_(RowObjectB.CreateRowObject(RowsetA.pIRowset(), rghRows[ROW_TWO]),S_OK, DB_S_NOROWSPECIFICCOLUMNS);

	//Create Threads
	CREATE_FIRST_THREADS(Thread_VerifyDeleteColumns, &T1Arg);
	CREATE_SECOND_THREADS(Thread_VerifyDeleteColumns, &T2Arg);

	START_THREADS();
	END_THREADS();	

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END



// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCDeleteColumns::Terminate()
{ 
	// TO DO:  Add your own code here 

// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIRowSchemaChange::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END

// {{ TCW_TC_PROTOTYPE(TCAddColumns)
//*-----------------------------------------------------------------------
//| Test Case:		TCAddColumns - IRowSchemaChange::AddColumns
//| Created:  	8/5/98
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCAddColumns::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCIRowSchemaChange::Init())
	// }}
	{ 
		return TRUE;
	} 
	return FALSE;
} 




// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc General - All columns - no BLOBs
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAddColumns::Variation_1()
{ 
	TBEGIN
	
	//Loop through all the rows in the rowset, verify the columns...
	TESTC(VerifyAddColumnsAllRows(NULL, OPT_MODIFYCOLUMNS, UPDATEABLE_NONINDEX_COLS_BOUND, BLOB_LONG));
	TESTC(VerifyAddColumnsAllRows(NULL, OPT_UPDATEANDADD, UPDATEABLE_NONINDEX_COLS_BOUND, BLOB_LONG));

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc General - All Columns - BLOBs
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAddColumns::Variation_2()
{ 
	TBEGIN
	
	//Loop through all the rows in the rowset, verify the columns...
	TESTC(VerifyAddColumnsAllRows(NULL, OPT_MODIFYCOLUMNS, UPDATEABLE_NONINDEX_COLS_BOUND, BLOB_LONG));
	TESTC(VerifyAddColumnsAllRows(NULL, OPT_UPDATEANDADD, UPDATEABLE_NONINDEX_COLS_BOUND, BLOB_LONG));

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END






// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc General - just BLOBs
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAddColumns::Variation_3()
{ 
	TBEGIN
	
	//Loop through all the rows in the rowset, verify the columns...
	TESTC(VerifyAddColumnsAllRows(NULL, OPT_MODIFYCOLUMNS, BLOB_COLS_BOUND, BLOB_LONG));
	TESTC(VerifyAddColumnsAllRows(NULL, OPT_ADDNEWCOLUMNS, BLOB_COLS_BOUND, BLOB_LONG));

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc General - each column seperatly
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAddColumns::Variation_4()
{ 
	TBEGIN
	IColumnsInfo* pIColumnsInfo = NULL;
	DBORDINAL iCol,cColumns=0;	
	DBCOLUMNINFO* rgColumnInfo = NULL;
	WCHAR* pStringBuffer = NULL;

	//Get the ColumnInfo
	TESTC(VerifyInterface(pRowSchemaChange(), IID_IColumnsInfo, ROW_INTERFACE, (IUnknown**)&pIColumnsInfo));
	TESTC_(pIColumnsInfo->GetColumnInfo(&cColumns, &rgColumnInfo, &pStringBuffer),S_OK);

	//Loop through each column seperatly...
	for(iCol=0; iCol<cColumns; iCol++)
	{
		//Loop through all the rows in the rowset, verify the columns...
		if(rgColumnInfo[iCol].dwFlags & DBCOLUMNFLAGS_WRITE)
		{
			if(!VerifyAddColumnsAllRows(NULL, OPT_MODIFYCOLUMNS, USE_COLS_TO_BIND_ARRAY, BLOB_LONG, FORWARD,NO_COLS_BY_REF, DBTYPE_EMPTY, 1, &rgColumnInfo[iCol].iOrdinal))
			{
				//Data incorrect for this column!
				TERROR("Data was incorrect for this column Ordinal " << rgColumnInfo[iCol].iOrdinal);
			}
		}
	}

CLEANUP:
	SAFE_FREE(rgColumnInfo);
	SAFE_FREE(pStringBuffer);
	SAFE_RELEASE(pIColumnsInfo);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAddColumns::Variation_5()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc General - with existing rowset columns - ALREADYEXISTS
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAddColumns::Variation_6()
{ 
	TBEGIN
	HRESULT hr = S_OK;
	DBORDINAL cColAccess = 0;
	DBCOLUMNACCESS* rgColAccess = NULL;
	HROW hRow = NULL;

	DBORDINAL iCol,cColOrds = 0;
	DBORDINAL* rgColOrds = NULL;

	CRowObject RowObjectA;
	CRowsetChange RowsetA;
	TESTC_(RowsetA.CreateRowset(USE_SUPPORTED_SELECT_ALLFROMTBL, IID_IRowset, NULL, DBACCESSOR_ROWDATA, DBPART_ALL, UPDATEABLE_COLS_BOUND | NONINDEX_COLS_BOUND | NULLABLE_COLS_BOUND),S_OK);
	
	//Obtain row object
	TESTC_(RowsetA.GetNextRows(&hRow),S_OK);
	TEST2C_(RowObjectA.CreateRowObject(RowsetA.pIRowset(), hRow),S_OK, DB_S_NOROWSPECIFICCOLUMNS);

	//Loop through each column seperatly...
	cColOrds = RowsetA.m_cBindings;
	SAFE_ALLOC(rgColOrds, DBORDINAL, cColOrds);
	for(iCol=0; iCol<cColOrds; iCol++)
		rgColOrds[iCol] = RowsetA.m_rgBinding[iCol].iOrdinal;
		
	//AddColumns with existing rowset columns...
	TESTC(VerifyAddColumns(&RowObjectA, OPT_MODIFYCOLUMNS, SECOND_ROW, USE_COLS_TO_BIND_ARRAY, BLOB_LONG, FORWARD, NO_COLS_BY_REF, DBTYPE_EMPTY, cColOrds, rgColOrds));

CLEANUP:
	SAFE_FREE(rgColOrds);
	RowsetA.ReleaseRows(hRow);
	FreeColAccess(cColAccess, rgColAccess);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc General - with existing row columns - ALREADYEXISTS
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAddColumns::Variation_7()
{ 
	TBEGIN
	
	//Loop through all the rows in the rowset, verify the columns...
	TESTC(VerifyAddColumnsAllRows(NULL, OPT_UPDATEANDADD, UPDATEABLE_NONINDEX_COLS_BOUND, BLOB_LONG));

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc General - for only extra columns
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAddColumns::Variation_8()
{ 
	TBEGIN
	HRESULT hr = S_OK;
	DBORDINAL cColAccess = 0;
	DBCOLUMNACCESS* rgColAccess = NULL;
	void* pData = NULL;
	HROW hRow = NULL;

	DBORDINAL cColumns = 0;
	DBORDINAL* rgColOrdinals = NULL;

	CRowObject RowObjectA;
	CRowsetChange RowsetA;
	TESTC_(RowsetA.CreateRowset(),S_OK);

	//Obtain row object
	TESTC_(RowsetA.GetNextRows(&hRow),S_OK);
	TEST2C_(RowObjectA.CreateRowObject(RowsetA.pIRowset(), hRow),S_OK, DB_S_NOROWSPECIFICCOLUMNS);

	//Obtain the just the Extra columns
	TESTC_(hr = RowObjectA.GetExtraColumnInfo(&cColumns, NULL, NULL, &rgColOrdinals),S_OK);

	//AddColumns with existing rowset columns...
	TESTC(VerifyAddColumns(&RowObjectA, OPT_MODIFYCOLUMNS, SECOND_ROW, USE_COLS_TO_BIND_ARRAY, BLOB_LONG, FORWARD, NO_COLS_BY_REF, DBTYPE_EMPTY, cColumns, rgColOrdinals));

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	FreeColAccess(cColAccess, rgColAccess);
	SAFE_FREE(pData);
	SAFE_FREE(rgColOrdinals);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAddColumns::Variation_9()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc General - 0 columns - no-op
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAddColumns::Variation_10()
{ 
	TBEGIN
	
	//AddColumns - with (0 NULL)
	TESTC(VerifyAddColumnsAllRows(NULL, OPT_MODIFYCOLUMNS, USE_COLS_TO_BIND_ARRAY, BLOB_LONG, FORWARD,
				NO_COLS_BY_REF,	DBTYPE_EMPTY, 0, NULL));

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc General - same column bound numerous times
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAddColumns::Variation_11()
{ 
	TBEGIN
	IColumnsInfo* pIColumnsInfo = NULL;
	DBORDINAL i,cColumns=0;	
	DBCOLUMNINFO* rgColumnInfo = NULL;
	WCHAR* pStringBuffer = NULL;
	DBORDINAL  cColOrds = 0;
	DBORDINAL* rgColOrds = NULL;

	//Use a new rowset, and ask for a non-forward-only cursor, 
	//so we can obtain the data multiple times.
	CRowsetChange RowsetA;
	TESTC_PROVIDER(RowsetA.CreateRowset(DBPROP_OTHERUPDATEDELETE)==S_OK);

	//Get the ColumnInfo
	TESTC(VerifyInterface(pRowSchemaChange(), IID_IColumnsInfo, ROW_INTERFACE, (IUnknown**)&pIColumnsInfo));
	TESTC_(pIColumnsInfo->GetColumnInfo(&cColumns, &rgColumnInfo, &pStringBuffer),S_OK);
	SAFE_ALLOC(rgColOrds, DBORDINAL, cColumns);

	//Loop through each column seperatly...
	for(i=0; i<cColumns; i++)
	{
		//Fill in the Col Ordinals with nukerous duplicates
		cColOrds = 0;
		for(DBORDINAL iDup=0; iDup<i; iDup++)
		{
			if(rgColumnInfo[i].dwFlags & DBCOLUMNFLAGS_WRITE)
			{
				rgColOrds[iDup] = rgColumnInfo[i].iOrdinal;
				cColOrds++;	
			}
		}
		
		//Loop through all the rows in the rowset, verify the columns...
		TESTC(VerifyAddColumnsAllRows(&RowsetA, OPT_MODIFYCOLUMNS, USE_COLS_TO_BIND_ARRAY, BLOB_LONG, FORWARD,
					NO_COLS_BY_REF,	DBTYPE_EMPTY, cColOrds, rgColOrds));
	}

CLEANUP:
	SAFE_FREE(rgColumnInfo);
	SAFE_FREE(pStringBuffer);
	SAFE_RELEASE(pIColumnsInfo);
	SAFE_FREE(rgColOrds);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc General - IUnknown columns
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAddColumns::Variation_12()
{ 
	TBEGIN
	
	//Loop through all the rows in the rowset, verify the columns...
	TESTC(VerifyAddColumnsAllRows(NULL, OPT_UPDATEANDADD, BLOB_COLS_BOUND, BLOB_IID_IUNKNOWN));

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAddColumns::Variation_13()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END






// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc AddColumns - Not Binding Value for all columns, pData is NULL - DB_E_ERRORSOCCURRED
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAddColumns::Variation_14()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc AddColumns - Not Binding Value for some columns, pData is NULL - DB_S_ERRORSOCCURRED
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAddColumns::Variation_15()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc AddColumns - Not Binding Value for ISNULL columns, pData is NULL - S_OK
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAddColumns::Variation_16()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc AddColumns - Not Binding Value for BLOB columns, pData is NULL - S_OK
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAddColumns::Variation_17()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAddColumns::Variation_18()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc Boundary - Some valid, some non-existent columns - DB_S_ERRORSOCCURRED
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAddColumns::Variation_19()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc Boundary - All non-existent columns - DB_E_ERRORSOCCURRED
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAddColumns::Variation_20()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc Boundary - No Vector Columns - S_OK
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAddColumns::Variation_21()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(22)
//*-----------------------------------------------------------------------
// @mfunc Boundary - No Vectors and Non-Existent Columns - DB_E_ERRORSOCCURRED
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAddColumns::Variation_22()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(23)
//*-----------------------------------------------------------------------
// @mfunc Boundary - Only Vector Columns - S_OK
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAddColumns::Variation_23()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(24)
//*-----------------------------------------------------------------------
// @mfunc Boundary - Only Non-Existent Vector Columns - DB_E_ERRORSOCCURRED
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAddColumns::Variation_24()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(25)
//*-----------------------------------------------------------------------
// @mfunc Boundary - Valid Vectors and Non-Existent Columns - DB_S_ERRORSOCCURRED
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAddColumns::Variation_25()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(26)
//*-----------------------------------------------------------------------
// @mfunc Boundary - Valid Non-Vectors and Non-Existent Vector Columns - DB_S_ERRORSOCCURRED
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAddColumns::Variation_26()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(27)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAddColumns::Variation_27()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(28)
//*-----------------------------------------------------------------------
// @mfunc Boundary - 0 columns -[0, NULL] and [0, valid]  no-op
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAddColumns::Variation_28()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(29)
//*-----------------------------------------------------------------------
// @mfunc Boundary - [Valid, NULL] - E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAddColumns::Variation_29()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(30)
//*-----------------------------------------------------------------------
// @mfunc Boundary - columnid - all valid dbkinds
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAddColumns::Variation_30()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(31)
//*-----------------------------------------------------------------------
// @mfunc Boundary - columnid - invalid dbkinds
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAddColumns::Variation_31()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(32)
//*-----------------------------------------------------------------------
// @mfunc Boundary - cbMaxLen - ignored for fixed length types
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAddColumns::Variation_32()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END






// {{ TCW_VAR_PROTOTYPE(33)
//*-----------------------------------------------------------------------
// @mfunc Boundary - cbMaxLen - required for variable length types
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAddColumns::Variation_33()
{ 
	TBEGIN
	HRESULT hr = S_OK;
	DBORDINAL iCol,cColAccess = 0;
	DBCOLUMNACCESS* rgColAccess = NULL;
	DBCOLUMNINFO* rgColInfo = NULL;
	void* pData = NULL;
	HROW hRow = NULL;

	CRowObject RowObjectA;
	CRowsetChange RowsetA;
	TESTC_(RowsetA.CreateRowset(),S_OK);

	//Obtain row object
	TESTC_(RowsetA.GetNextRows(&hRow),S_OK);
	TEST2C_(RowObjectA.CreateRowObject(RowsetA.pIRowset(), hRow),S_OK, DB_S_NOROWSPECIFICCOLUMNS);

	//Obtain just the variable columns
	TESTC_(hr = CreateColumns(&RowObjectA, &cColAccess, &rgColAccess, &rgColInfo, &pData, NULL, OPT_MODIFYCOLUMNS, FIRST_ROW, ECOLS_BOUND(VARIABLE_LEN_COLS_BOUND | UPDATEABLE_COLS_BOUND)), S_OK);

	//Set all the Variable columns - cbMaxLen = 0
	for(iCol=0; iCol<cColAccess; iCol++)
		rgColAccess[iCol].cbMaxLen = 0;
	
	//AddColumns - DB_E_ERRORSOCCURRED
	TESTC_(hr = AddColumns(&RowObjectA, FIRST_ROW, cColAccess, rgColInfo, rgColAccess), cColAccess==0 ? S_OK : DB_E_ERRORSOCCURRED);

	//Verify the Status'
	for(iCol=0; iCol<cColAccess; iCol++)
		TESTC(rgColAccess[iCol].dwStatus == DBSTATUS_E_CANTCONVERTVALUE);

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	FreeColAccess(cColAccess, rgColAccess);
	SAFE_FREE(rgColInfo);
	SAFE_FREE(pData);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(34)
//*-----------------------------------------------------------------------
// @mfunc Boundary - cbMaxLen - DBSTATUS_S_TRUNCATION
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAddColumns::Variation_34()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(35)
//*-----------------------------------------------------------------------
// @mfunc Boundary - wType - all valid types and modifieres
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAddColumns::Variation_35()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(36)
//*-----------------------------------------------------------------------
// @mfunc Boundary - wType - invalid types and invalid type modifiers
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAddColumns::Variation_36()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(37)
//*-----------------------------------------------------------------------
// @mfunc Boundary - bPrecision - make sure ignored on input for all types, except NUMERIC.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAddColumns::Variation_37()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(38)
//*-----------------------------------------------------------------------
// @mfunc Boundary - bScale - make sure ignored on input for all types, except NUMERIC.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAddColumns::Variation_38()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(39)
//*-----------------------------------------------------------------------
// @mfunc Boundary - Output Only - make sure cbDataLen, dwStatus, dwReserverd are ignored on input
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAddColumns::Variation_39()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(40)
//*-----------------------------------------------------------------------
// @mfunc Boundary - Input - Make sure all pointers and input args are not changed on output, pData pointer, columnid [including all union pointers], cbMaxLen, dwReserved, wType.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAddColumns::Variation_40()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(41)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAddColumns::Variation_41()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(42)
//*-----------------------------------------------------------------------
// @mfunc Boundary - DBCOLUMNINFO.iOrdinal is ignored
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAddColumns::Variation_42()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(43)
//*-----------------------------------------------------------------------
// @mfunc Boundary - DBCOLUMNINFO.pwszName is ignored
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAddColumns::Variation_43()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(44)
//*-----------------------------------------------------------------------
// @mfunc Boundary - DBCOLUMNINFO.cbMaxLen is ignored
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAddColumns::Variation_44()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(45)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAddColumns::Variation_45()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(46)
//*-----------------------------------------------------------------------
// @mfunc Parameters - DBSTATUS_S_OK - add column and provide data all in one call
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAddColumns::Variation_46()
{ 
	TBEGIN

	//Verify AddColumns
	TESTC(VerifyAddColumnsAllRows(NULL, OPT_ADDNEWCOLUMNS, UPDATEABLE_COLS_BOUND | NONINDEX_COLS_BOUND | NULLABLE_COLS_BOUND, BLOB_LONG));

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(47)
//*-----------------------------------------------------------------------
// @mfunc Parameters - DBSTATUS_S_IGNORE - add column only
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAddColumns::Variation_47()
{ 
	TBEGIN

	//Loop through all the rows in the rowset, verify the columns...
	TESTC(VerifyAddColumnsAllRows(NULL, EADDCOLUMNS(OPT_IGNOREDATA | OPT_ADDNEWCOLUMNS), UPDATEABLE_NONINDEX_COLS_BOUND, BLOB_LONG));

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END






// {{ TCW_VAR_PROTOTYPE(48)
//*-----------------------------------------------------------------------
// @mfunc Parameters - DBSTATUS_S_IGNORE - NULL rgColAccess for only meta data
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAddColumns::Variation_48()
{ 
	TBEGIN
	HRESULT hr = S_OK;
	DBORDINAL cColAccess = 0;
	DBCOLUMNACCESS* rgColAccess = NULL;
	DBCOLUMNINFO* rgColInfo = NULL;
	HROW hRow = NULL;

	CRowObject RowObjectA;
	CRowsetChange RowsetA;
	TESTC_(RowsetA.CreateRowset(USE_SUPPORTED_SELECT_ALLFROMTBL, IID_IRowset, NULL, DBACCESSOR_ROWDATA, DBPART_ALL, UPDATEABLE_COLS_BOUND | NONINDEX_COLS_BOUND | NULLABLE_COLS_BOUND),S_OK);
	
	//Obtain row object
	TESTC_(RowsetA.GetNextRows(&hRow),S_OK);
	TEST2C_(RowObjectA.CreateRowObject(RowsetA.pIRowset(), hRow),S_OK, DB_S_NOROWSPECIFICCOLUMNS);

	//Create ColAccess structures from the Rowset bindings
	TESTC_(hr = RowObjectA.BindingsToColAccess(RowsetA.m_cBindings, RowsetA.m_rgBinding, RowsetA.m_pData, &cColAccess, &rgColAccess),S_OK);
	
	//Create new Meta Data...
	TESTC_(hr = CreateNewColInfo(&RowObjectA, SECOND_ROW, cColAccess, rgColAccess, &rgColInfo),S_OK);

	//Verify AddColumns
	//rgColAccess = NULL, basically means DBSTATUS_S_IGNORE for all columns, but doesn't require
	//the user to allocate the structure and set everything to IGNORE...  (waste of resources)
	TESTC(VerifyAddColumns(&RowObjectA, SECOND_ROW, cColAccess, rgColInfo, NULL));

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	FreeColAccess(cColAccess, rgColAccess);
	SAFE_FREE(rgColInfo);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(49)
//*-----------------------------------------------------------------------
// @mfunc Parameters - DBSTATUS_S_IGNORE - add column only - pData == NULL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAddColumns::Variation_49()
{ 
	TBEGIN
	HRESULT hr = S_OK;
	DBORDINAL iCol,cColAccess = 0;
	DBCOLUMNACCESS* rgColAccess = NULL;
	DBCOLUMNINFO* rgColInfo = NULL;
	HROW hRow = NULL;

	CRowObject RowObjectA;
	CRowsetChange RowsetA;
	TESTC_(RowsetA.CreateRowset(),S_OK);
	
	//Obtain row object
	TESTC_(RowsetA.GetNextRows(&hRow),S_OK);
	TEST2C_(RowObjectA.CreateRowObject(RowsetA.pIRowset(), hRow),S_OK, DB_S_NOROWSPECIFICCOLUMNS);

	//Create ColAccess structures from the Rowset bindings
	TESTC_(hr = RowObjectA.BindingsToColAccess(RowsetA.m_cBindings, RowsetA.m_rgBinding, RowsetA.m_pData, &cColAccess, &rgColAccess),S_OK);
	
	//Create new Meta Data...
	TESTC_(hr = CreateNewColInfo(&RowObjectA, SECOND_ROW, cColAccess, rgColAccess, &rgColInfo),S_OK);

	//Set all the status' to DBSTATUS_S_IGNORE
	for(iCol=0; iCol<cColAccess; iCol++)
	{
		rgColAccess[iCol].dwStatus = DBSTATUS_S_IGNORE;

		//Make sure the provider ignores pData and other items when IGNORE is specified
		rgColAccess[iCol].pData			= NULL;
		rgColAccess[iCol].columnid		= DB_NULLID;
		rgColAccess[iCol].cbDataLen		= ULONG_MAX;
		rgColAccess[iCol].cbMaxLen		= ULONG_MAX-1;
		rgColAccess[iCol].dwReserved	= INVALID(ULONG);
		rgColAccess[iCol].wType			= INVALID(DBTYPE);
		rgColAccess[iCol].bPrecision	= INVALID(BYTE);
		rgColAccess[iCol].bScale		= INVALID(BYTE);
	}

	//Verify AddColumns
	//rgColAccess = NULL, basically means DBSTATUS_S_IGNORE for all columns, but doesn't require
	//the user to allocate the structure and set everything to IGNORE...  (waste of resources)
	TESTC(VerifyAddColumns(&RowObjectA, SECOND_ROW, cColAccess, rgColInfo, rgColAccess));

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	FreeColAccess(cColAccess, rgColAccess);
	SAFE_FREE(rgColInfo);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(50)
//*-----------------------------------------------------------------------
// @mfunc Parameters - DBSTATUS_S_IGNORE - add column only
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAddColumns::Variation_50()
{ 
	TBEGIN

	//Loop through all the rows in the rowset, verify the columns...
	TESTC(VerifyAddColumnsAllRows(NULL, EADDCOLUMNS(OPT_IGNOREDATA | OPT_MODIFYCOLUMNS | OPT_ADDNEWCOLUMNS), ALL_COLS_BOUND, BLOB_LONG));

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(51)
//*-----------------------------------------------------------------------
// @mfunc Parameters - DBSTATUS_S_IGNORE - add column only - pData == NULL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAddColumns::Variation_51()
{ 
	TBEGIN
	HRESULT hr = S_OK;
	DBORDINAL iCol,cColAccess = 0;
	DBCOLUMNACCESS* rgColAccess = NULL;
	DBCOLUMNINFO* rgColInfo = NULL;
	HROW hRow = NULL;

	CRowObject RowObjectA;
	CRowsetChange RowsetA;
	TESTC_(RowsetA.CreateRowset(),S_OK);
	
	//Obtain row object
	TESTC_(RowsetA.GetNextRows(&hRow),S_OK);
	TEST2C_(RowObjectA.CreateRowObject(RowsetA.pIRowset(), hRow),S_OK, DB_S_NOROWSPECIFICCOLUMNS);

	//Create ColAccess structures from the Rowset bindings
	TESTC_(hr = RowObjectA.BindingsToColAccess(RowsetA.m_cBindings, RowsetA.m_rgBinding, RowsetA.m_pData, &cColAccess, &rgColAccess),S_OK);
	
	//Allocate the ColInfo
	SAFE_ALLOC(rgColInfo, DBCOLUMNINFO, cColAccess);
	memset(rgColInfo, 0, (size_t)(cColAccess*sizeof(DBCOLUMNINFO)));
	
	//For existing columns provider should not be looking at the metadata,
	//except of course for the columnid.
	for(iCol=0; iCol<cColAccess; iCol++)
		rgColInfo[iCol].columnid = rgColAccess[iCol].columnid;

	//Set all the status' to DBSTATUS_S_IGNORE
	for(iCol=0; iCol<cColAccess; iCol++)
	{
		rgColAccess[iCol].dwStatus = DBSTATUS_S_IGNORE;

		//Make sure the provider ignores pData and other items when IGNORE is specified
		rgColAccess[iCol].pData			= NULL;
		rgColAccess[iCol].columnid		= DB_NULLID;
		rgColAccess[iCol].cbDataLen		= ULONG_MAX;
		rgColAccess[iCol].cbMaxLen		= ULONG_MAX-1;
		rgColAccess[iCol].dwReserved	= INVALID(ULONG);
		rgColAccess[iCol].wType			= INVALID(DBTYPE);
		rgColAccess[iCol].bPrecision	= INVALID(BYTE);
		rgColAccess[iCol].bScale		= INVALID(BYTE);
	}

	//Verify AddColumns
	//rgColAccess = NULL, basically means DBSTATUS_S_IGNORE for all columns, but doesn't require
	//the user to allocate the structure and set everything to IGNORE...  (waste of resources)
	TESTC(VerifyAddColumns(&RowObjectA, SECOND_ROW, cColAccess, rgColInfo, rgColAccess));

CLEANUP:
	RowsetA.ReleaseRows(hRow);
	FreeColAccess(cColAccess, rgColAccess);
	SAFE_FREE(rgColInfo);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(52)
//*-----------------------------------------------------------------------
// @mfunc Parameters - DBSTATUS_S_ISNULL - add column - null data
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAddColumns::Variation_52()
{ 
	TBEGIN

	//Loop through all the rows in the rowset, verify the columns...
	TESTC(VerifyAddColumnsAllRows(NULL, EADDCOLUMNS(OPT_NULLDATA | OPT_ADDNEWCOLUMNS), UPDATEABLE_NONINDEX_COLS_BOUND | NULLABLE_COLS_BOUND, BLOB_LONG));

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END






// {{ TCW_VAR_PROTOTYPE(53)
//*-----------------------------------------------------------------------
// @mfunc Parameters - DBSTATUS_S_ISNULL - add column - pData == NULL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAddColumns::Variation_53()
{ 
	TBEGIN

	//Loop through all the rows in the rowset, verify the columns...
	TESTC(VerifyAddColumnsAllRows(NULL, EADDCOLUMNS(OPT_NULLDATA | OPT_UPDATEANDADD), UPDATEABLE_NONINDEX_COLS_BOUND | NULLABLE_COLS_BOUND, BLOB_LONG));

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(54)
//*-----------------------------------------------------------------------
// @mfunc Parameters - DBSTATUS_S_DEFAULT - add column only
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAddColumns::Variation_54()
{ 
	TBEGIN

	//Loop through all the rows in the rowset, verify the columns...
	TESTC(VerifyAddColumnsAllRows(NULL, EADDCOLUMNS(OPT_DEFAULTDATA | OPT_ADDNEWCOLUMNS), UPDATEABLE_NONINDEX_COLS_BOUND | NULLABLE_COLS_BOUND, BLOB_LONG));

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(55)
//*-----------------------------------------------------------------------
// @mfunc Parameters - DBSTATUS_S_DEFAULT - add column only - pData == NULL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAddColumns::Variation_55()
{ 
	TBEGIN

	//Loop through all the rows in the rowset, verify the columns...
	TESTC(VerifyAddColumnsAllRows(NULL, EADDCOLUMNS(OPT_DEFAULTDATA | OPT_UPDATEANDADD), UPDATEABLE_NONINDEX_COLS_BOUND | NULLABLE_COLS_BOUND, BLOB_LONG));

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(56)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAddColumns::Variation_56()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END





// {{ TCW_VAR_PROTOTYPE(57)
//*-----------------------------------------------------------------------
// @mfunc Parameters - DBCOLUMNFLAGS_WRITE - Writable column - with data
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAddColumns::Variation_57()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(58)
//*-----------------------------------------------------------------------
// @mfunc Parameters - DBCOLUMNFLAGS_WRITE - Read-only column - ignore data
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAddColumns::Variation_58()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(59)
//*-----------------------------------------------------------------------
// @mfunc Parameters - DBCOLUMNFLAGS_ISNULLABLE - nullable column - isnull data
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAddColumns::Variation_59()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(60)
//*-----------------------------------------------------------------------
// @mfunc Parameters - DBCOLUMNFLAGS_ISNULLABLE - non-nullable column - ignore
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAddColumns::Variation_60()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(61)
//*-----------------------------------------------------------------------
// @mfunc Parameters - DBCOLUMNFLAGS_ISFIXEDLENGTH - fixed length column - default data
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAddColumns::Variation_61()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(62)
//*-----------------------------------------------------------------------
// @mfunc Parameters - DBCOLUMNFLAGS_ISFIXEDLENGTH - non-fixed length column - default data
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAddColumns::Variation_62()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(63)
//*-----------------------------------------------------------------------
// @mfunc Parameters - DBCOLUMNFLAGS Combinations - WRITE | ISNULLABLE - null data
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAddColumns::Variation_63()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(64)
//*-----------------------------------------------------------------------
// @mfunc Parameters - DBCOLUMNFLAGS Combinations - WRITE | ISNULLABLE| FIXEDLENGTH - data
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAddColumns::Variation_64()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(65)
//*-----------------------------------------------------------------------
// @mfunc Parameters - DBCOLUMNFLAGS Combinations - WRITE | FIXEDLENGTH - default data
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAddColumns::Variation_65()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(66)
//*-----------------------------------------------------------------------
// @mfunc Parameters - DBCOLUMNFLAGS Combinations - FIXEDLENGTH - ignore data
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAddColumns::Variation_66()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(67)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAddColumns::Variation_67()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(68)
//*-----------------------------------------------------------------------
// @mfunc Sequence - Multiple row object - different rows - same columnids
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAddColumns::Variation_68()
{ 
	TBEGIN
	HRESULT hr = S_OK;
	HROW rghRows[TWO_ROWS] = {NULL,NULL};
	CRowObject RowObjectA;
	CRowObject RowObjectB;
	CRowObject RowObjectC;
	CRowObject RowObjectD;
	CRowsetChange	RowsetA;

	DBORDINAL iCol,cColAccess = 0;
	DBCOLUMNACCESS* rgColAccess = NULL;
	DBCOLUMNINFO* rgColInfo = NULL;
	void* pData = NULL;

	//Create the rowset
	TESTC_(RowsetA.CreateRowset(DBPROP_CANHOLDROWS),S_OK);

	//Obtain the First Two row(s)...
	//Some providers may only alow one row active, this variation requires two rows to be active
	//So we can obtain row objects over them...
	TEST2C_(RowsetA.GetNextRows(TWO_ROWS, rghRows), S_OK, DB_S_ROWLIMITEXCEEDED);
	TESTC_PROVIDER(hr==S_OK);

	//Now create the row object(s).
	TEST2C_(RowObjectA.CreateRowObject(RowsetA.pIRowset(), rghRows[ROW_ONE]), S_OK, DB_S_NOROWSPECIFICCOLUMNS);
	TEST2C_(RowObjectB.CreateRowObject(RowsetA.pIRowset(), rghRows[ROW_TWO]), S_OK, DB_S_NOROWSPECIFICCOLUMNS);

	//Create the ColAccess Structures...
	TESTC_(RowObjectA.CreateColAccess(&cColAccess, &rgColAccess, &pData, NULL, UPDATEABLE_COLS_BOUND | NONINDEX_COLS_BOUND | NULLABLE_COLS_BOUND, BLOB_LONG),S_OK);
	TESTC_(CreateNewColInfo(&RowObjectA, FIRST_ROW, cColAccess, rgColAccess, &rgColInfo),S_OK);

	//Now Actually add the columns (add the same columns, to different rows...)
	TESTC(VerifyAddColumns(&RowObjectA, FIRST_ROW, cColAccess, rgColInfo, NULL));
	TESTC(VerifyAddColumns(&RowObjectB, FIRST_ROW, cColAccess, rgColInfo, NULL));

	//Now verify the visibility of the added columns...
	for(iCol=0; iCol<cColAccess; iCol++)
	{
		//Each row object should see their own newly added columns
		TESTC(::FindColInfo(RowObjectA.pIRow(), &rgColInfo[iCol].columnid));
		TESTC(::FindColInfo(RowObjectB.pIRow(), &rgColInfo[iCol].columnid));
	}

	//Now, any newly created row objects over the row, should see all the changes.
	TEST2C_(RowObjectC.CreateRowObject(RowsetA.pIRowset(), rghRows[ROW_ONE]), S_OK, DB_S_NOROWSPECIFICCOLUMNS);
	TEST2C_(RowObjectD.CreateRowObject(RowsetA.pIRowset(), rghRows[ROW_TWO]), S_OK, DB_S_NOROWSPECIFICCOLUMNS);
	for(iCol=0; iCol<cColAccess; iCol++)
	{
		TESTC(::FindColInfo(RowObjectC.pIRow(), &rgColInfo[iCol].columnid));
		TESTC(::FindColInfo(RowObjectD.pIRow(), &rgColInfo[iCol].columnid));
	}
	
	
CLEANUP:
	RowsetA.ReleaseRows(TWO_ROWS, rghRows);
	SAFE_FREE(rgColInfo);
	SAFE_FREE(rgColAccess);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(69)
//*-----------------------------------------------------------------------
// @mfunc Sequence - Multiple row object - same row - different columnids
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAddColumns::Variation_69()
{ 
	TBEGIN
	HROW hRow = NULL;
	CRowObject RowObjectA;
	CRowObject RowObjectB;
	CRowObject RowObjectC;
	CRowsetChange	RowsetA;

	DBORDINAL iCol,cColAccess = 0;
	DBCOLUMNACCESS* rgColAccess = NULL;
	DBCOLUMNINFO* rgColInfoA = NULL;
	DBCOLUMNINFO* rgColInfoB = NULL;
	void* pData = NULL;

	//Create the rowset
	TESTC_(RowsetA.CreateRowset(),S_OK);

	//Obtain the First row...
	TESTC_(RowsetA.GetNextRows(&hRow),S_OK);

	//Now create the row object(s).
	TEST2C_(RowObjectA.CreateRowObject(RowsetA.pIRowset(), hRow), S_OK, DB_S_NOROWSPECIFICCOLUMNS);
	TEST2C_(RowObjectB.CreateRowObject(RowsetA.pIRowset(), hRow), S_OK, DB_S_NOROWSPECIFICCOLUMNS);

	//Create the ColAccess Structures...
	TESTC_(RowObjectA.CreateColAccess(&cColAccess, &rgColAccess, &pData, NULL, UPDATEABLE_COLS_BOUND | NONINDEX_COLS_BOUND | NULLABLE_COLS_BOUND, BLOB_LONG),S_OK);
	TESTC_(CreateNewColInfo(&RowObjectA, FIRST_ROW, cColAccess, rgColAccess, &rgColInfoA),S_OK);
	TESTC_(CreateNewColInfo(&RowObjectB, FIRST_ROW, cColAccess, rgColAccess, &rgColInfoB),S_OK);

	//Now Actually add the columns
	TESTC(VerifyAddColumns(&RowObjectA, FIRST_ROW, cColAccess, rgColInfoA, NULL));
	TESTC(VerifyAddColumns(&RowObjectB, FIRST_ROW, cColAccess, rgColInfoB, NULL));

	//Now verify the visibility of the added columns...
	for(iCol=0; iCol<cColAccess; iCol++)
	{
		//Each row object should see their own newly added columns
		TESTC(::FindColInfo(RowObjectA.pIRow(), &rgColInfoA[iCol].columnid));
		TESTC(::FindColInfo(RowObjectB.pIRow(), &rgColInfoB[iCol].columnid));

		//Each row object should not be able to see other objects columns (even over the same row)
		TESTC(!::FindColInfo(RowObjectA.pIRow(), &rgColInfoB[iCol].columnid));
		TESTC(!::FindColInfo(RowObjectB.pIRow(), &rgColInfoA[iCol].columnid));
	}

	//Now, any newly created row objects over the row, should see all the changes.
	TEST2C_(RowObjectC.CreateRowObject(RowsetA.pIRowset(), hRow), S_OK, DB_S_NOROWSPECIFICCOLUMNS);
	for(iCol=0; iCol<cColAccess; iCol++)
	{
		TESTC(::FindColInfo(RowObjectC.pIRow(), &rgColInfoA[iCol].columnid));
		TESTC(::FindColInfo(RowObjectC.pIRow(), &rgColInfoB[iCol].columnid));
	}
	
	
CLEANUP:
	RowsetA.ReleaseRows(hRow);
	SAFE_FREE(rgColInfoA);
	SAFE_FREE(rgColInfoB);
	SAFE_FREE(rgColAccess);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(70)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAddColumns::Variation_70()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(71)
//*-----------------------------------------------------------------------
// @mfunc IRowsetUpdate - AddColumns with existing row object columns -  verify - update - verify
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAddColumns::Variation_71()
{ 
	TBEGIN
	//This senario requires IRowsetUpdate
	TESTC_PROVIDER(SupportedProperty(DBPROP_IRowsetUpdate, DBPROPSET_ROWSET));

	//Loop through all the rows in the rowset, verify the columns...
	TESTC(VerifyAddColumnsAllRows(NULL, EADDCOLUMNS(OPT_ADDNEWCOLUMNS | OPT_BUFFEREDUPDATE), UPDATEABLE_NONINDEX_COLS_BOUND, BLOB_LONG));

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(72)
//*-----------------------------------------------------------------------
// @mfunc IRowsetUpdate - AddColumns with existing rowset columns -  verify - update - verify
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAddColumns::Variation_72()
{ 
	TBEGIN
	//This senario requires IRowsetUpdate
	TESTC_PROVIDER(SupportedProperty(DBPROP_IRowsetUpdate, DBPROPSET_ROWSET));
	
	//Loop through all the rows in the rowset, verify the columns...
	TESTC(VerifyAddColumnsAllRows(NULL, EADDCOLUMNS(OPT_MODIFYCOLUMNS | OPT_BUFFEREDUPDATE), UPDATEABLE_NONINDEX_COLS_BOUND, BLOB_LONG));

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(73)
//*-----------------------------------------------------------------------
// @mfunc IRowsetUpdate - AddColumns with existing row object columns -  verify - undo - verify
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAddColumns::Variation_73()
{ 
	TBEGIN
	//This senario requires IRowsetUpdate
	TESTC_PROVIDER(SupportedProperty(DBPROP_IRowsetUpdate, DBPROPSET_ROWSET));
	
	//Loop through all the rows in the rowset, verify the columns...
	TESTC(VerifyAddColumnsAllRows(NULL, EADDCOLUMNS(OPT_ADDNEWCOLUMNS | OPT_BUFFEREDUNDO), UPDATEABLE_NONINDEX_COLS_BOUND, BLOB_LONG));

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(74)
//*-----------------------------------------------------------------------
// @mfunc IRowsetUpdate - AddColumns with existing rowset columns -  verify - undo - verify
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAddColumns::Variation_74()
{ 
	TBEGIN
	//This senario requires IRowsetUpdate
	TESTC_PROVIDER(SupportedProperty(DBPROP_IRowsetUpdate, DBPROPSET_ROWSET));
	
	//Loop through all the rows in the rowset, verify the columns...
	TESTC(VerifyAddColumnsAllRows(NULL, EADDCOLUMNS(OPT_MODIFYCOLUMNS | OPT_BUFFEREDUNDO), UPDATEABLE_NONINDEX_COLS_BOUND, BLOB_LONG));

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(75)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAddColumns::Variation_75()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(76)
//*-----------------------------------------------------------------------
// @mfunc Threads - AddColumns - same row object
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAddColumns::Variation_76()
{ 
	TBEGIN
	INIT_THREADS(MAX_THREADS);	
	
	CRowObject RowObjectA;
	CRowsetChange RowsetA;
	HROW rghRows[TWO_ROWS];

	//Setup Thread Arguments
	THREADARG T1Arg = { this, &RowObjectA };
	
	//Create Rowset object
	RowsetA.SetSupportedProperty(DBPROP_IRowsetLocate);
	RowsetA.SetProperty(DBPROP_CANHOLDROWS);
	TESTC_(RowsetA.CreateRowset(),S_OK);

	//Obtain row object
	TESTC_(RowsetA.GetNextRows(ONE_ROW, rghRows),S_OK);
	TEST2C_(RowObjectA.CreateRowObject(RowsetA.pIRowset(), rghRows[ROW_ONE]),S_OK, DB_S_NOROWSPECIFICCOLUMNS);

	//Create Threads
	CREATE_THREADS(Thread_VerifyAddColumns, &T1Arg);

	START_THREADS();
	END_THREADS();	

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(77)
//*-----------------------------------------------------------------------
// @mfunc Threads - AddColumns - diff row object
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAddColumns::Variation_77()
{ 
	TBEGIN
	INIT_THREADS(MAX_THREADS);	
	
	CRowObject RowObjectA;
	CRowObject RowObjectB;
	CRowsetChange RowsetA;
	HROW rghRows[TWO_ROWS];

	//Setup Thread Arguments
	THREADARG T1Arg = { this, &RowObjectA };
	THREADARG T2Arg = { this, &RowObjectB };
	
	//Create Rowset object
	RowsetA.SetSupportedProperty(DBPROP_IRowsetLocate);
	RowsetA.SetProperty(DBPROP_CANHOLDROWS);
	TESTC_(RowsetA.CreateRowset(),S_OK);

	//Obtain row object
	TESTC_(RowsetA.GetNextRows(TWO_ROWS, rghRows),S_OK);
	TEST2C_(RowObjectA.CreateRowObject(RowsetA.pIRowset(), rghRows[ROW_ONE]),S_OK, DB_S_NOROWSPECIFICCOLUMNS);
	TEST2C_(RowObjectB.CreateRowObject(RowsetA.pIRowset(), rghRows[ROW_TWO]),S_OK, DB_S_NOROWSPECIFICCOLUMNS);

	//Create Threads
	CREATE_FIRST_THREADS(Thread_VerifyAddColumns, &T1Arg);
	CREATE_SECOND_THREADS(Thread_VerifyAddColumns, &T2Arg);

	START_THREADS();
	END_THREADS();	

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCAddColumns::Terminate()
{ 
// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIRowSchemaChange::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END



// {{ TCW_TC_PROTOTYPE(TCTransactions)
//*-----------------------------------------------------------------------
//| Test Case:		TCTransactions - IRowSchemaChange inside Transactions
//| Created:  	8/5/98
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCTransactions::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCIRowSchemaChange::Init())
	// }}
	{ 
		return TRUE;
	} 
	return FALSE;
} 




// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc DeleteColumns - ABORT with fRetaining TRUE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCTransactions::Variation_1()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc DeleteColumns - ABORT with fRetaining FALSE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCTransactions::Variation_2()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc DeleteColumns - COMMIT with fRetaining TRUE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCTransactions::Variation_3()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc DeleteColumns - COMMIT with fRetaining FALSE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCTransactions::Variation_4()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END





// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCTransactions::Variation_5()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc AddColumns - ABORT with fRetaining TRUE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCTransactions::Variation_6()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc AddColumns - ABORT with fRetaining FALSE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCTransactions::Variation_7()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc AddColumns - COMMIT with fRetaining TRUE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCTransactions::Variation_8()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc AddColumns - COMMIT with fRetaining FALSE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCTransactions::Variation_9()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
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
// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIRowSchemaChange::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END


