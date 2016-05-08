//-----------------------------------------------------------------------------
// Microsoft OLE DB TABLECOPY Sample
// Copyright (C) 1991-2000 Microsoft Corporation
//
// @doc
//
// @module TABLECOPY.CPP
//
//-----------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////
// Includes
//
/////////////////////////////////////////////////////////////////
#define DBINITCONSTANTS  //Initilaizes OLEDB Guids / Constants
#define INITGUID

#include "winmain.h"
#include "common.h"
#include "tablecopy.h"
#include "table.h"
#include "wizard.h"

#include "msdaguid.h"	//CLSID_OLEDB_ENUMERATOR



/////////////////////////////////////////////////////////////////
// CTableCopy::CTableCopy
//
/////////////////////////////////////////////////////////////////
CTableCopy::CTableCopy(CWizard* pCWizard)
{
	ASSERT(pCWizard);

	m_pCFromTable	= new CTable(pCWizard);
	m_pCToTable		= new CTable(pCWizard);

	//RowOptions
	m_dwRowOpt		= IDR_ALL_ROWS;
	m_ulMaxRows		= 1;
	
	//InsertOptions
	m_dwInsertOpt	= IDR_PARAM_SETS;
	m_ulParamSets	= 10;
	
	//BLOB options
	m_dwBlobOpt		= IDR_BLOB_SIZE;
	m_ulBlobSize	= 5000;		//Some reasonable value less than MAX_COL_SIZE

	//Create Options
	m_fCopyTables		= TRUE;	
	m_fCopyIndexes		= TRUE;
	m_fCopyPrimaryKeys	= TRUE;
	m_fShowQuery		= FALSE;

	//Data
	m_fTranslate	= TRUE;
	m_pCWizard		= pCWizard;  //Back pointer to windowing class
}


/////////////////////////////////////////////////////////////////
// CTableCopy::~CTableCopy
//
/////////////////////////////////////////////////////////////////
CTableCopy::~CTableCopy()
{
	delete m_pCFromTable;
	delete m_pCToTable;
}




/////////////////////////////////////////////////////////////////
// HRESULT CTableCopy::MapTypes
//
/////////////////////////////////////////////////////////////////
HRESULT CTableCopy::MapTypes()
{
	HRESULT hr = S_OK;

	//Now get the TypeInfo for all columns
	//IDBSchemaRowsets may not be supported, which we may be able to do without
	//on the source table, we are going to lose IsNullable, IsAutoInc and other options
	hr = m_pCFromTable->GetTypeInfo();
	
	//Now map all the Types correctly from the Source to the Target
	QTESTC(hr = m_pCToTable->MapTableInfo(m_pCFromTable));
		
CLEANUP:
	if(FAILED(hr))
		wMessageBox(NULL, MB_TASKMODAL | MB_ICONEXCLAMATION | MB_OK, wsz_ERROR, wsz_TYPEMAPPING_FAILURE);

	return hr;
}



///////////////////////////////////////////////////////////////////////
// HRESULT CTableCopy::CopyTables
//
///////////////////////////////////////////////////////////////////////
HRESULT CTableCopy::CopyTables()
{
	HRESULT		hr = S_OK;
	DBCOUNTITEM cRowsCopied = 0;

	// Create the Table (if desired)
	if(m_fCopyTables)
		QTESTC(hr = m_pCToTable->CreateTable());

	// Create the indexes (if desired)
	if(m_fCopyIndexes)
		QTESTC(hr = m_pCToTable->CopyIndexes(m_pCFromTable));

	//GetColInfo for TargetTable
	QTESTC(hr = m_pCToTable->GetColInfo(m_dwInsertOpt));
	
	//Now Copy the Data
	QTESTC(hr = m_pCToTable->CopyData(m_pCFromTable, &cRowsCopied));

CLEANUP:
	//Display Results
	if(SUCCEEDED(hr))
		wMessageBox(NULL, MB_TASKMODAL | MB_ICONINFORMATION | MB_OK, wsz_SUCCESS, wsz_COPY_SUCCESS, cRowsCopied);
	else
		wMessageBox(NULL, MB_TASKMODAL | MB_ICONEXCLAMATION | MB_OK, wsz_ERROR, wsz_COPY_FAILURE);

	return hr;
}
