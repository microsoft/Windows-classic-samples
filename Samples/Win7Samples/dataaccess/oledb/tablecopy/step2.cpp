//-----------------------------------------------------------------------------
// Microsoft OLE DB TABLECOPY Sample
// Copyright (C) 1991-2000 Microsoft Corporation
//
// @doc
//
// @module STEP2.CPP
//
//-----------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////
// Includes
//
/////////////////////////////////////////////////////////////////////
#include "wizard.h"
#include "common.h"
#include "tablecopy.h"
#include "table.h"



/////////////////////////////////////////////////////////////////////
// Defines
//
/////////////////////////////////////////////////////////////////////
//Enum Index Header for the ListView controls
enum COL_HEADERS
{
	//IDL_INDEXES
	COL_INDEXNAME		= 0,	//IndexInfo.wszIndexName
	COL_INDEXCOLNAME	= 1,	//IndexInfo.wszColName
	COL_UNIQUE			= 2,	//IndexInfo.fUnique
	COL_PRIMARYKEY		= 3,	//IndexInfo.fIsPrimaryKey
	COL_COLLATION		= 4,	//IndexInfo.dwCollation
	COL_AUTOUPDATE		= 5,	//IndexInfo.fAutoUpdate
	COL_NULLS			= 6,	//IndexInfo.dwNulls
};

enum INDEX_TYPE
{
	INDEXTYPE_INDEX			= 0,
	INDEXTYPE_PRIMARYKEY	= 1,
};

/////////////////////////////////////////////////////////////////////
// CS2Dialog::CS2Dialog
//
/////////////////////////////////////////////////////////////////////
CS2Dialog::CS2Dialog(HWND hWnd, HINSTANCE hInst, CTableCopy* pCTableCopy)
	: CDialogBase(hWnd, hInst)
{
	ASSERT(pCTableCopy);
	m_pCTableCopy = pCTableCopy;
}


/////////////////////////////////////////////////////////////////////
// CS2Dialog::~CS2Dialog
//
/////////////////////////////////////////////////////////////////////
CS2Dialog::~CS2Dialog()
{
}


/////////////////////////////////////////////////////////////////////////////
// ULONG CS2Dialog::Display
//
/////////////////////////////////////////////////////////////////////////////
INT_PTR CS2Dialog::Display()
{
	//Create a modal dialog box
	return DialogBoxParam(m_hInst, MAKEINTRESOURCE(IDD_INDEX_INFO), NULL, (DLGPROC)DlgProc, (LPARAM)this);
}


/////////////////////////////////////////////////////////////////////
// CS2Dialog::DlgProc
//
/////////////////////////////////////////////////////////////////////
BOOL WINAPI CS2Dialog::DlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) 
	{
		case WM_INITDIALOG:
		{
			Busy();
			//Store the "this" pointer, since this is a static method
			CS2Dialog* pThis = (CS2Dialog*)lParam;
			SetWindowLongPtrA(hWnd, GWLP_USERDATA, (LONG_PTR)pThis);
			CTableCopy* pCTableCopy = pThis->m_pCTableCopy;
			CWizard* pCWizard = pCTableCopy->m_pCWizard;

			LONG iPrevStep = pCWizard->m_iPrevStep;

			//On INIT we know we have a valid hWnd to store
			CenterDialog(hWnd);
			pThis->m_hWnd = hWnd;

			pThis->InitControls();
			pThis->RefreshControls();

			pThis->ResetIndexList(GetDlgItem(hWnd, IDL_INDEXES));
			pCWizard->DestroyPrevStep(WIZ_STEP2);
			
			//No since in even displaying this Dialog if there
			//are no indexes...
			if(pCTableCopy->m_pCFromTable->m_cIndexes == 0)
				pCWizard->DisplayStep(iPrevStep==WIZ_STEP1 ? WIZ_STEP3 : WIZ_STEP1);

			return HANDLED_MSG;
		}
		
		case WM_COMMAND:
		{
			//Obtain the "this" pointer
			CS2Dialog* pThis = (CS2Dialog*)GetWindowLongPtrA(hWnd, GWLP_USERDATA);

			// All buttons are handled the same way
			switch(GET_WM_COMMAND_ID(wParam, lParam)) 
			{
				case IDOK:
                case IDB_PREV:
					//Record all selected indexes
					Busy();
					pThis->RecordSelectedIndexes(GetDlgItem(hWnd, IDL_INDEXES));
					pThis->m_pCTableCopy->m_pCWizard->DisplayStep((GET_WM_COMMAND_ID(wParam, lParam) == IDOK) ? WIZ_STEP3 : WIZ_STEP1);
                	return HANDLED_MSG;

				case IDCANCEL:
					//Record all selected indexes
					Busy();
					EndDialog(hWnd, GET_WM_COMMAND_ID(wParam, lParam));
                	return HANDLED_MSG;
			}
		}
	}

	return UNHANDLED_MSG;
}


/////////////////////////////////////////////////////////////////////////////
// BOOL CS2Dialog::InitControls
//
/////////////////////////////////////////////////////////////////////////////
BOOL CS2Dialog::InitControls()
{
	HWND hWndIndexes = GetDlgItem(m_hWnd, IDL_INDEXES);
	
	//Create the Index ImageList
	HIMAGELIST hIndexImageList = ImageList_Create(16, 16, ILC_MASK, 1, 0 );

	//IDI_INDEX - normal index icon
	HICON hIcon = LoadIcon(m_hInst, MAKEINTRESOURCE(IDI_INDEX));
	ImageList_AddIcon(hIndexImageList, hIcon);
	//IDI_PRIMARYKEY - primary key index icon
	hIcon = LoadIcon(m_hInst, MAKEINTRESOURCE(IDI_PRIMARYKEY));
	ImageList_AddIcon(hIndexImageList, hIcon);

	//Set image list to the Table Window 
	ListView_SetImageList(hWndIndexes, hIndexImageList, LVSIL_SMALL);

	//ListView COLUMNS
	LV_InsertColumn(hWndIndexes,	COL_INDEXNAME,		"Index");
	LV_InsertColumn(hWndIndexes,	COL_INDEXCOLNAME,	"ColName");
	LV_InsertColumn(hWndIndexes,	COL_UNIQUE,			"Unique");
	LV_InsertColumn(hWndIndexes,	COL_PRIMARYKEY,		"PrimaryKey");
	LV_InsertColumn(hWndIndexes,	COL_COLLATION,		"Collation");
	LV_InsertColumn(hWndIndexes,	COL_AUTOUPDATE,		"AutoUpdate");
	LV_InsertColumn(hWndIndexes,	COL_NULLS,			"Nulls");

	//AutoSize all columns
	for(ULONG i=0; i<=COL_NULLS; i++)
		SendMessage(hWndIndexes, LVM_SETCOLUMNWIDTH, (WPARAM)i,		(LPARAM)LVSCW_AUTOSIZE_USEHEADER);

	//Use Extended ListView Styles!
	SendMessage(hWndIndexes, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_TWOCLICKACTIVATE | LVS_EX_SUBITEMIMAGES, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_TWOCLICKACTIVATE | LVS_EX_SUBITEMIMAGES);
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// BOOL CS2Dialog::RefreshControls
//
/////////////////////////////////////////////////////////////////////////////
BOOL CS2Dialog::RefreshControls()
{
	BOOL fSchemas = m_pCTableCopy->m_pCFromTable->m_pCDataSource->m_pIDBSchemaRowset != NULL;

	//Display IndexWindow if there are on Schemas
	EnableWindow(GetDlgItem(m_hWnd, IDT_INDEXMSG),	fSchemas);
	EnableWindow(GetDlgItem(m_hWnd, IDL_INDEXES),	fSchemas);
	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// BOOL CS2Dialog::ResetIndexList
//
/////////////////////////////////////////////////////////////////////////////
BOOL CS2Dialog::ResetIndexList(HWND hWnd)
{
	HRESULT hr;
	CHAR	szBuffer[MAX_NAME_LEN*2];
	
	HROW*		rghRows = NULL;
	ULONG		i;
	DBCOUNTITEM cRowsObtained = 0;
	IRowset*	pIRowset = NULL;

	IAccessor* pIAccessor = NULL;
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	
	//Use the passed in Session interface
	CTable* pCFromTable = m_pCTableCopy->m_pCFromTable;
	IOpenRowset* pIOpenRowset = pCFromTable->m_pCDataSource->m_pIOpenRowset;
	IDBSchemaRowset* pIDBSchemaRowset = pCFromTable->m_pCDataSource->m_pIDBSchemaRowset;

	//Provider doesn't have to support IDBSchemaRowset
	if(pIDBSchemaRowset == NULL)
	{
		return TRUE;
	}
	
	//Save the currently selected indexes
	ULONG cSelIndexes = pCFromTable->m_cIndexes;
	INDEXINFO* rgSelIndexInfo = pCFromTable->m_rgIndexInfo;
	
	//Reset the current IndexInfo
	pCFromTable->m_cIndexes = 0;
	pCFromTable->m_rgIndexInfo = NULL;

	// Now make the call
	SendMessage(hWnd, LVM_DELETEALLITEMS, (WPARAM)0, (LPARAM)0);

	// Bind the user and table name for the list
	const static ULONG cBindings = 13;
	const static DBBINDING rgBindings[cBindings] = 
		{
			//INDEX_NAME
			6,	 			
			offsetof(INDEXINFO, wszIndexName),
			0,
			0,	
			NULL,			
			NULL, 		
			NULL,		
			DBPART_VALUE,
			DBMEMOWNER_CLIENTOWNED,		
			DBPARAMIO_NOTPARAM, 
			MAX_NAME_LEN, 		
			0, 				
			DBTYPE_WSTR, 	
			0,	
			0, 				

			//UNIQUE
			8,	 			
			offsetof(INDEXINFO, fUnique),
			0,
			0,	
			NULL,			
			NULL, 		
			NULL,		
			DBPART_VALUE,
			DBMEMOWNER_CLIENTOWNED,		
			DBPARAMIO_NOTPARAM, 
			sizeof(VARIANT_BOOL), 		
			0, 				
			DBTYPE_BOOL, 	
			0,	
			0, 				

			//CLUSTERED
			9,	 			
			offsetof(INDEXINFO, fClustered),
			0,
			0,	
			NULL,			
			NULL, 		
			NULL,		
			DBPART_VALUE,
			DBMEMOWNER_CLIENTOWNED,		
			DBPARAMIO_NOTPARAM, 
			sizeof(VARIANT_BOOL), 		
			0, 				
			DBTYPE_BOOL, 	
			0,	
			0, 				

			//TYPE
			10,	 			
			offsetof(INDEXINFO, wType),
			0,
			0,	
			NULL,			
			NULL, 		
			NULL,		
			DBPART_VALUE,
			DBMEMOWNER_CLIENTOWNED,		
			DBPARAMIO_NOTPARAM, 
			sizeof(DBTYPE), 		
			0, 				
			DBTYPE_UI2, 	
			0,	
			0, 				

			//FILL_FACTOR
			11,	 			
			offsetof(INDEXINFO, dwFillFactor),
			0,
			0,	
			NULL,			
			NULL, 		
			NULL,		
			DBPART_VALUE,
			DBMEMOWNER_CLIENTOWNED,		
			DBPARAMIO_NOTPARAM, 
			sizeof(LONG), 		
			0, 				
			DBTYPE_I4, 	
			0,	
			0, 				

			//INITIAL_SIZE
			12,	 			
			offsetof(INDEXINFO, dwInitialSize),
			0,
			0,	
			NULL,			
			NULL, 		
			NULL,		
			DBPART_VALUE,
			DBMEMOWNER_CLIENTOWNED,		
			DBPARAMIO_NOTPARAM, 
			sizeof(LONG), 		
			0, 				
			DBTYPE_I4, 	
			0,	
			0, 				

			//NULLS
			13,	 			
			offsetof(INDEXINFO, dwNulls),
			0,
			0,	
			NULL,			
			NULL, 		
			NULL,		
			DBPART_VALUE,
			DBMEMOWNER_CLIENTOWNED,		
			DBPARAMIO_NOTPARAM, 
			sizeof(LONG), 		
			0, 				
			DBTYPE_I4, 	
			0,	
			0, 				

			//SORT_BOOKMARKS
			14,	 			
			offsetof(INDEXINFO, fSortBookmarks),
			0,
			0,	
			NULL,			
			NULL, 		
			NULL,		
			DBPART_VALUE,
			DBMEMOWNER_CLIENTOWNED,		
			DBPARAMIO_NOTPARAM, 
			sizeof(VARIANT_BOOL), 		
			0, 				
			DBTYPE_BOOL, 	
			0,	
			0, 				

			//AUTO_UPDATE
			15,	 			
			offsetof(INDEXINFO, fAutoUpdate),
			0,
			0,	
			NULL,			
			NULL, 		
			NULL,		
			DBPART_VALUE,
			DBMEMOWNER_CLIENTOWNED,		
			DBPARAMIO_NOTPARAM, 
			sizeof(VARIANT_BOOL), 		
			0, 				
			DBTYPE_BOOL, 	
			0,	
			0, 				

			//NULL_COLLATION
			16,	 			
			offsetof(INDEXINFO, dwNullCollation),
			0,
			0,	
			NULL,			
			NULL, 		
			NULL,		
			DBPART_VALUE,
			DBMEMOWNER_CLIENTOWNED,		
			DBPARAMIO_NOTPARAM, 
			sizeof(LONG), 		
			0, 				
			DBTYPE_I4, 	
			0,	
			0, 				
			
			//ORDINAL_POSITION
			17,	 			
			offsetof(INDEXINFO, iOrdinal),
			0,
			0,	
			NULL,			
			NULL, 		
			NULL,		
			DBPART_VALUE,
			DBMEMOWNER_CLIENTOWNED,		
			DBPARAMIO_NOTPARAM, 
			sizeof(ULONG), 		
			0, 				
			DBTYPE_UI4, 	
			0,	
			0, 				
			
			//COLUMN_NAME
			18,	 			
			offsetof(INDEXINFO, wszColName),
			0,	
			0,	
			NULL,			
			NULL, 		
			NULL,		
			DBPART_VALUE,
			DBMEMOWNER_CLIENTOWNED,		
			DBPARAMIO_NOTPARAM, 
			MAX_NAME_LEN, 		
			0, 				
			DBTYPE_WSTR, 	
			0,	
			0, 				
		
			//COLLATION
			21,	 			
			offsetof(INDEXINFO, dwCollation),
			0,	
			0,	
			NULL,			
			NULL, 		
			NULL,		
			DBPART_VALUE, 	
			DBMEMOWNER_CLIENTOWNED,		
			DBPARAMIO_NOTPARAM, 
			sizeof(DWORD), 		
			0, 				
			DBTYPE_I4, 	
			0,	
			0, 				
		};

	//set up the restrictions
	const ULONG cRestrictions = 5;
	VARIANT	 rgRestrictions[cRestrictions];
	
	//set up the restrictions
	InitVariants(cRestrictions, rgRestrictions);
	SetRestriction(&rgRestrictions[0], pCFromTable->m_pCDataSource->m_pwszCatalog);
	SetRestriction(&rgRestrictions[1], pCFromTable->m_TableInfo.wszSchemaName);
	SetRestriction(&rgRestrictions[4], pCFromTable->m_TableInfo.wszTableName);
	
	//GetRowset
	//DBSCHEMA_INDEXES may not be supported by the driver. 
	//If an error occurs, just don't display IndexInfo
	QTESTC(hr = pIDBSchemaRowset->GetRowset(NULL, DBSCHEMA_INDEXES, cRestrictions, rgRestrictions, 
										IID_IRowset, 0, NULL, (IUnknown **)&pIRowset));

	XTESTC(hr = pIRowset->QueryInterface(IID_IAccessor, (void **)&pIAccessor));
	
	//Create Accessor for IndexInfo
	XTESTC(hr = pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA, cBindings, rgBindings, 0, &hAccessor, NULL));

	while(TRUE)
	{
		XTESTC(hr = pIRowset->GetNextRows(NULL, 0, MAX_BLOCK_SIZE, &cRowsObtained, &rghRows));
		
		//ENDOFROWSET
		if(cRowsObtained==0) 
			break;
		
		//Realloc Table struct for Indexes
		SAFE_REALLOC(pCFromTable->m_rgIndexInfo, INDEXINFO, pCFromTable->m_cIndexes + cRowsObtained);
		memset(&pCFromTable->m_rgIndexInfo[pCFromTable->m_cIndexes], 0, cRowsObtained*sizeof(INDEXINFO));

		//Loop over rows obtained
		for(ULONG i=0; i<cRowsObtained; i++) 
		{	
			INDEXINFO* pIndexInfo = &pCFromTable->m_rgIndexInfo[pCFromTable->m_cIndexes];
			
			//Get the Data
			XTESTC(hr = pIRowset->GetData(rghRows[i], hAccessor, (void*)pIndexInfo));
						
			//If this is an index the ordinal will be 1 based
			if(pIndexInfo->iOrdinal==0)
				continue;
				
			//Only list the index if the column was selected to be copied
			for(ULONG iCol=0; iCol<pCFromTable->m_cColumns; iCol++)
			{
				if(wcscmp(pCFromTable->m_rgColDesc[iCol].wszColName, pIndexInfo->wszColName)==0)
				{
					//Now that we have an actual index...
					pCFromTable->m_cIndexes++;
					break;
				}
			}
						
		}
			
		//Release all the rows
		XTESTC(hr = pIRowset->ReleaseRows(cRowsObtained, rghRows, NULL, NULL, NULL));
		SAFE_FREE(rghRows);
	}

	//GetPrimaryKey info
	//Since DBSCHEMA_INDEXES doesn't have to return PrimaryKey info correctly (NULL)
	//we will call DBSCHEMA_PRIMARYKEYS before displaying IndexInfo to make sure
	//which columns are really PrimaryKeys.
	GetPrimaryKeys();

	//Now list all indexes to the Index ListView window
	for(i=0; i<pCFromTable->m_cIndexes; i++)
	{
		INDEXINFO* pIndexInfo = &pCFromTable->m_rgIndexInfo[i];

		//INDEXNAME (item)
		ConvertToMBCS(pIndexInfo->wszIndexName, szBuffer, MAX_NAME_LEN);
		LV_InsertItem(hWnd, i, COL_INDEXNAME, szBuffer, 0, pIndexInfo->fIsPrimaryKey ? INDEXTYPE_PRIMARYKEY : INDEXTYPE_INDEX);

		//INDEXCOLNAME (subitem)
		ConvertToMBCS(pIndexInfo->wszColName, szBuffer, MAX_NAME_LEN);
		LV_InsertItem(hWnd, i, COL_INDEXCOLNAME, szBuffer);
		
		//FLAGS (subitem)
		LV_InsertItem(hWnd, i, COL_UNIQUE, pIndexInfo->fUnique ? "TRUE" : "FALSE");
		LV_InsertItem(hWnd, i, COL_PRIMARYKEY, pIndexInfo->fIsPrimaryKey ? "TRUE" : "FALSE");
		LV_InsertItem(hWnd, i, COL_AUTOUPDATE, pIndexInfo->fAutoUpdate ? "TRUE" : "FALSE");
		LV_InsertItem(hWnd, i, COL_COLLATION, (pIndexInfo->dwCollation == DB_COLLATION_ASC) ? "ASC" : (pIndexInfo->dwCollation == DB_COLLATION_DESC) ? "DESC" : "NULL");
		LV_InsertItem(hWnd, i, COL_NULLS, (pIndexInfo->dwNulls == DBPROPVAL_IN_DISALLOWNULL) ? "DISALLOWNULL" : (pIndexInfo->dwNulls == DBPROPVAL_IN_IGNORENULL) ? "IGNORENULL" : "IGNOREANYNULL");
	}

	
	// If there was a previous selection, select it again on Back
	if(rgSelIndexInfo)
	{
		LONG lFoundIndex = -1;
		for(i=0; i<cSelIndexes; i++)
		{
			//Find the "index" of the existing index
			ConvertToMBCS(rgSelIndexInfo[i].wszIndexName, szBuffer, MAX_NAME_LEN);
			lFoundIndex = LV_FindItem(hWnd, szBuffer, lFoundIndex);

    		//If there was a selection, select it now
    		if(lFoundIndex != LVM_ERR)
			{
				LV_SetItemState(hWnd, lFoundIndex, COL_INDEXNAME, LVIS_SELECTED, LVIS_SELECTED);
			
				//Ensure that the first item is visible
				if(i==0)
					SendMessage(hWnd, LVM_ENSUREVISIBLE, (WPARAM)lFoundIndex, (LPARAM)FALSE);
			}
		}

	}    
	//Otherwise select all as default
	else 
	{
		for(i=0; i<pCFromTable->m_cIndexes; i++)
			LV_SetItemState(hWnd, i, COL_INDEXNAME, LVIS_SELECTED, LVIS_SELECTED);
	}
		
	//Only enable the ListBox/Title if there are indexes
	EnableWindow(GetDlgItem(m_hWnd, IDT_INDEXMSG),	pCFromTable->m_cIndexes);
	EnableWindow(hWnd,	pCFromTable->m_cIndexes);
		
    //AutoSize Index/ColName
	if(pCFromTable->m_cIndexes)
	{
		SendMessage(hWnd, LVM_SETCOLUMNWIDTH, (WPARAM)COL_INDEXNAME, (LPARAM)LVSCW_AUTOSIZE);
		SendMessage(hWnd, LVM_SETCOLUMNWIDTH, (WPARAM)COL_INDEXCOLNAME, (LPARAM)LVSCW_AUTOSIZE);
	}

CLEANUP:
	//Free Resriticions
	FreeVariants(cRestrictions, rgRestrictions);
	
	if(hAccessor && pIAccessor)
		XTEST(pIAccessor->ReleaseAccessor(hAccessor,NULL));

	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pIAccessor);
	SAFE_FREE(rghRows);
	SAFE_FREE(rgSelIndexInfo);
	return hr==S_OK;
}




/////////////////////////////////////////////////////////////////////////////
// BOOL CS2Dialog::GetPrimaryKeys
//
/////////////////////////////////////////////////////////////////////////////
BOOL CS2Dialog::GetPrimaryKeys()
{
	HRESULT hr;
	
	HROW*		rghRows = NULL;
	DBCOUNTITEM cRowsObtained = 0;
	IRowset*	pIRowset = NULL;

	IAccessor* pIAccessor = NULL;
	HACCESSOR hAccessor = DB_NULL_HACCESSOR;
	
	PRIMARYKEY PrimaryKey;
	
	//Use the passed in Session interface
	CTable* pCFromTable = m_pCTableCopy->m_pCFromTable;
	IOpenRowset* pIOpenRowset = pCFromTable->m_pCDataSource->m_pIOpenRowset;
	IDBSchemaRowset* pIDBSchemaRowset = pCFromTable->m_pCDataSource->m_pIDBSchemaRowset;

	//Provider doesn't have to support IDBSchemaRowset
	if(pIDBSchemaRowset == NULL)
		return TRUE;
	
	// Bind the user and table name for the list
	const static ULONG cBindings = 1;
	const static DBBINDING rgBindings[cBindings] = 
		{
			4,	 			
			offsetof(PRIMARYKEY, wszColName),
			0,
			0,	
			NULL,			
			NULL, 		
			NULL,		
			DBPART_VALUE,
			DBMEMOWNER_CLIENTOWNED,		
			DBPARAMIO_NOTPARAM, 
			MAX_NAME_LEN, 		
			0, 				
			DBTYPE_WSTR, 	
			0,	
			0, 				
		};

	//set up the restrictions
	const ULONG cRestrictions = 3;
	VARIANT	 rgRestrictions[cRestrictions];

	//set up the restrictions
	InitVariants(cRestrictions, rgRestrictions);
	SetRestriction(&rgRestrictions[0], pCFromTable->m_pCDataSource->m_pwszCatalog);
	SetRestriction(&rgRestrictions[1], pCFromTable->m_TableInfo.wszSchemaName);
	SetRestriction(&rgRestrictions[2], pCFromTable->m_TableInfo.wszTableName);
	
	//GetRowset
	//DBSCHEMA_PRIMARY_KEYS may not be supported by the driver. 
	//If an error occurs, just don't display PrimaryKey info
	QTESTC(hr = pIDBSchemaRowset->GetRowset(NULL, DBSCHEMA_PRIMARY_KEYS, cRestrictions, rgRestrictions, 
										IID_IRowset, 0, NULL, (IUnknown **)&pIRowset));

	XTESTC(hr = pIRowset->QueryInterface(IID_IAccessor, (void **)&pIAccessor));
	
	//Create Accessor for IndexInfo
	XTESTC(hr = pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA, cBindings, rgBindings, 0, &hAccessor, NULL));

	while(TRUE)
	{
		XTESTC(hr = pIRowset->GetNextRows(NULL, 0, MAX_BLOCK_SIZE, &cRowsObtained, &rghRows));
		
		//ENDOFROWSET
		if(cRowsObtained==0)
			break;
		
		for(ULONG i=0; i<cRowsObtained; i++) 
		{	
			//Reset the PrimaryKey
			memset(&PrimaryKey, 0, sizeof(PRIMARYKEY));
				
			//Get the Data
			XTESTC(hr = pIRowset->GetData(rghRows[i], hAccessor, (void*)&PrimaryKey));
		
			//Need to find the corresponding column in IndexInfo and 
			//mark it as a primary key column
			for(ULONG iIndex=0; iIndex<pCFromTable->m_cIndexes; iIndex++)
				if(wcscmp(PrimaryKey.wszColName, pCFromTable->m_rgIndexInfo[iIndex].wszColName)==0)
					pCFromTable->m_rgIndexInfo[iIndex].fIsPrimaryKey = TRUE;

			//Need to find the corresponding column in m_rgColDesc and mark it as a primary key column
			for(i=0; i<pCFromTable->m_cColumns; i++)
				if(wcscmp(PrimaryKey.wszColName, pCFromTable->m_rgColDesc[i].wszColName)==0)
					pCFromTable->m_rgColDesc[i].fIsPrimaryKey = TRUE;
		}
			
		//Release all the rows
		XTESTC(hr = pIRowset->ReleaseRows(cRowsObtained, rghRows, NULL, NULL, NULL));
		SAFE_FREE(rghRows);
	}

CLEANUP:
	//Free Resriticions
	FreeVariants(cRestrictions, rgRestrictions);
	
	if(hAccessor && pIAccessor)
		XTEST(pIAccessor->ReleaseAccessor(hAccessor,NULL));

	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pIAccessor);
	SAFE_FREE(rghRows);
	return hr==S_OK;
}



/////////////////////////////////////////////////////////////////////////////
// BOOL CS2Dialog::RecordSelectedIndexes
//
/////////////////////////////////////////////////////////////////////////////
BOOL CS2Dialog::RecordSelectedIndexes(HWND hWnd)
{
	CTable* pCTable = m_pCTableCopy->m_pCFromTable;
	LONG i;

	// Get the count of selected items
	LONG cSelIndexes = (LONG)SendMessage(hWnd, LVM_GETSELECTEDCOUNT, (WPARAM)0, (LPARAM)0);
	LONG iIndex = -1;

	//Loop over all the indexes
	pCTable->m_cIndexes = cSelIndexes;
	for(i=0; i<cSelIndexes; i++) 
	{
		iIndex = (LONG)SendMessage(hWnd, LVM_GETNEXTITEM, (WPARAM)iIndex, (LPARAM)LVNI_SELECTED);
		
		//"compact" the m_rgColDesc array to only the selected items
		if(iIndex != LVM_ERR)
			memmove(&pCTable->m_rgIndexInfo[i], &pCTable->m_rgIndexInfo[iIndex], sizeof(INDEXINFO));
	}

	return TRUE;
}
