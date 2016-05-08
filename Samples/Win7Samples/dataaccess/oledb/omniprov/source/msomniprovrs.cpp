// Start of file MSOmniProvRS.cpp
// File: MSOmniProvRS.cpp
//
//  This file contains the implementation for the CMSOmniProvRowset and CMSOmniProvCommand
//
//	The following functions have been customized or added for the provider to function:
//
//		CMSOmniProvCommand :: Execute -  Executes the command...
//		CMSOmniProvCommand :: GetColumnsInfo - Returns the column metadata requested by the consumer...
//		CMSOmniProvRowset :: Execute - Parses the SQL query, executes it and builds the initial rowset...
//		CMSOmniProvRowset :: GetColumnInfo - Returns the column metadata requested by the consumer builds reading the schema '.sxt' file...
//		CMSOmniProvRowset :: CreateColInfo - Creates all of the column information and stores it in the rowset...
//		CMSOmniProvRowset :: AllocateProxyBuffers - Causes the proxy buffers to be allocated for each of the CRow objects... 
//		CMSOmniProvRowset :: GetSchemaInfo - Returns the column metadata about the rowset...
//		CMSOmniProvRowset :: CheckTable - Finds if the table in the Query exists or not...
//		CMSOmniProvRowset :: GetDataSource - Retrieves the file name including the path to the schema '.sxt' file...
//

#include "stdafx.h"
#include "TheProvider.h"  // Contains the definitions for the interfaces
#include "MSOmniProvRS.h"  // Command and rowset header file 
#include <comdef.h> // For _bstr_t 

// CMSOmniProvCommand

// CMSOmniProvCommand :: Execute
// Executes the command...
HRESULT CMSOmniProvCommand::Execute(IUnknown * pUnkOuter, REFIID riid, DBPARAMS * pParams, 
								 DBROWCOUNT * pcRowsAffected, IUnknown ** ppRowset)
{
	CMSOmniProvRowset* pRowset;
	return CreateRowset(pUnkOuter, riid, pParams, pcRowsAffected, ppRowset, pRowset);
}

// CMSOmniProvCommand :: GetColumnsInfo
// Returns the column metadata requested by the consumer...
ATLCOLUMNINFO* CMSOmniProvCommand::GetColumnInfo(CMSOmniProvCommand* pv, DBORDINAL* pcInfo)
{
	return NULL;
}


// CMSOmniProvRowset

// CMSOmniProvRowset::Execute
// 1. Parse the SQL query,
// 2. Execute the query, and 
// 3. Build the initial rowset
HRESULT CMSOmniProvRowset::Execute(DBPARAMS * pParams, DBROWCOUNT* pcRowsAffected)
{
	USES_CONVERSION;
	CMSOmniProvRowset* pT = (CMSOmniProvRowset*) this;
	CMSOmniProvRowset::ObjectLock cab((CMSOmniProvRowset*) this);
	HRESULT hr;

	_bstr_t m_bstrFileName; 
	if ( FAILED(hr=pT->GetDataSource(m_bstrFileName)) )
		return hr;

	// Check the property value whether read/ updatabiliy property is set or not...
	_variant_t varUpd;
	GetPropValue(&DBPROPSET_ROWSET,DBPROP_UPDATABILITY, &varUpd);
	if ( 0 != varUpd.iVal )
	{
		// 1. a) Build the file's Schema m_prgColInfo from the '.sxt' file,
		//     b) Open the file '.txt', and 
		//     c) Fill the m_DBFile.m_Rows structure
		// Open in exclusive mode
		if (!m_DBFile.Open((LPCTSTR) m_bstrFileName,true))
			return DB_E_NOTABLE;
		if (!m_DBFile.FillRowArray()) 
				return E_FAIL;
	}
	else // Open in non-exclusive mode
	{
		if (!m_DBFile.Open((LPCTSTR) m_bstrFileName,false))
			return DB_E_NOTABLE;
		if (!m_DBFile.FillRowArray()) 
				return E_FAIL;
	}

	// Validate Command 
    // 2. PARSE the SQL Query here  (Only SELECT * FROM <Table_Name> is supported)
	TCHAR sep[] = " ";
	_bstr_t bstrSQL(pT->m_strCommandText);
	LPTSTR  pchNextToken = NULL;
	TCHAR * token = _tcstok_s((TCHAR*) bstrSQL, (TCHAR*) sep, &pchNextToken);

	if (!CheckTable((TCHAR*) token)) 
	{
		// The Rowset was created using the ICommand::Execute( )...
		// Only "SELECT * FROM Table_Name"  Queries are supported
		if(_tcsicmp(token,TEXT("select")) != 0)
		{
			ATLTRACE2(atlTraceDBProvider,0,(const TCHAR*) (_bstr_t("Query: '")+ bstrSQL + _bstr_t("' is not a valid Query\n")));
			return DB_E_ERRORSINCOMMAND;
		}
		ATLTRACE2(atlTraceDBProvider,0,(const TCHAR*) (_bstr_t("\tIt is a valid '")+_bstr_t(token) + _bstr_t("' Query\n")));
		
		TCHAR szTblNm[MAX_TABLE_NAME_SIZE];
		while (token != NULL)
		{
			_tcscpy_s(szTblNm, _countof(szTblNm), token);
			token= _tcstok_s(NULL,(TCHAR*) sep, &pchNextToken);
		}
		if (!CheckTable((TCHAR*) szTblNm)) 
			return DB_E_NOTABLE;
	}

	// Allocate proxy buffers based on the schema information 
	// Each CRow contains proxy buffer that the data is trasnferred to in the native 
	// format.  This information then needs to be copied out to the file in character format
	// on SetData() calls.

	CreateColInfo();

	AllocateProxyBuffers();

	if (pcRowsAffected != NULL)
		*pcRowsAffected = m_DBFile.m_Rows.GetCount();

	return S_OK;  
}

// CMSOmniProvRowset :: GetColumnInfo
// Overriding GetColumnInfo as we will be deriving the column information
// from the schema '.sxt' file
ATLCOLUMNINFO* CMSOmniProvRowset::GetColumnInfo(CMSOmniProvRowset* pv, DBORDINAL* pNumCols)
{
	ATLASSERT(pv!=NULL);
	ATLASSERT(pNumCols!=NULL);

	return pv->GetSchemaInfo(pNumCols);
}

// CMSOmniProvRowset :: CreateColInfo 
// This function creates all of the column information and stores it in the rowset.
// It currently uses all of the columns of the table
void CMSOmniProvRowset::CreateColInfo()
{
	// check if bookmarks are set 
	bool bUseBookmarks = false;
	CComVariant varBookmarks;
	HRESULT hrLocal = GetPropValue(&DBPROPSET_ROWSET, DBPROP_BOOKMARKS, &varBookmarks);
	bUseBookmarks = (hrLocal == S_OK &&  varBookmarks.boolVal == VARIANT_TRUE);

	// get column info from the .INI file
	DBORDINAL ulNumCols;
	ATLCOLUMNINFO * prgColInfo = m_DBFile.GetSchemaInfo(&ulNumCols);
	
	// Need to add bookmark column info?
	if (bUseBookmarks)
	{
		// create new set of column information which includes bookmarks
		ATLCOLUMNINFO * prgColInfoNew;
		prgColInfoNew = new ATLCOLUMNINFO[ulNumCols+1];

		// set bindings for the bookmark
		memset(&prgColInfoNew[0], 0, sizeof(ATLCOLUMNINFO));
		prgColInfoNew[0].cbOffset = 0;
		prgColInfoNew[0].iOrdinal = 0;
		prgColInfoNew[0].columnid.eKind = DBKIND_NAME;
		prgColInfoNew[0].columnid.uGuid.guid = GUID_NULL;
		prgColInfoNew[0].columnid.uName.pwszName = OLESTR("Bookmark");
		prgColInfoNew[0].pwszName = OLESTR("Bookmark");
		prgColInfoNew[0].wType = DBTYPE_I4;
		prgColInfoNew[0].ulColumnSize = 4;
		prgColInfoNew[0].dwFlags = DBCOLUMNFLAGS_ISBOOKMARK;
		
		// copy the old information into the new
		for (DBORDINAL i = 1; i <= ulNumCols; i++)
		{
			prgColInfoNew[i] = prgColInfo[i-1];  
			prgColInfoNew[i].cbOffset +=4;  //adjust space for bookmark
		}

		ulNumCols++;
		prgColInfo = prgColInfoNew;
	}

	// store column info in the rowset object
	SetSchemaInfo(prgColInfo, ulNumCols);
	// 2.0
	// Release the temporary prgColInfo
	delete [] prgColInfo;
}

// CMSOmniProvRowset :: AllocateProxyBuffers
// Causes the proxy buffers to be allocated for each of the CRow objects 
// which represents one row of the file.
void CMSOmniProvRowset::AllocateProxyBuffers()
{
	CAtlArray<CRow *> * pRows = &m_DBFile.m_Rows;
	size_t nSize = pRows->GetCount();
	for (size_t i = 0; i < nSize; i++)
	{
		(*pRows)[i]->AllocProxyBuffer(m_prgColInfo, m_cCols);
	}
}

// CMSOmniProvRowset :: GetSchemaInfo
// Returns the column metadata about the rowset...
ATLCOLUMNINFO * CMSOmniProvRowset::GetSchemaInfo(DBORDINAL * pNumCols)
{
	// Allocate the column information only once  as the functions we are returning the 
	// data from don't free.
	*pNumCols = m_cCols;

	return m_prgColInfo;
}


// CMSOmniProvRowset :: CheckTable
// Finds if the table in the Query exists or not...
// 2.0
// Code to recognize table name enclosed in [ ]...
BOOL CMSOmniProvRowset::CheckTable(TCHAR* szTblNm)
{
	CMSOmniProvRowset* pT = (CMSOmniProvRowset*) this;
	if(szTblNm[0] == '[')
	{
		size_t iLenBuff = 0;
		iLenBuff = _tcslen(szTblNm);
		TCHAR *szTmpBuff= (TCHAR *) malloc(sizeof(TCHAR) *iLenBuff );
		 _tcsncpy_s(szTmpBuff, iLenBuff, szTblNm + 1,iLenBuff -2);

		if (!_tcscmp(pT->m_DBFile.m_szTblNm, szTmpBuff))
		{
		    free(szTmpBuff);
			return true;									
		}
		else
		    free(szTmpBuff);
	}
	if (!_tcscmp(pT->m_DBFile.m_szTblNm, szTblNm))
		return true;
	else 
		return false;
}

//CMSOmniProvRowset :: GetDataSource
// Retrieves the file name including the path to the schema '.sxt' file...
HRESULT CMSOmniProvRowset::GetDataSource(_bstr_t &m_bstrLoc)
{
	CMSOmniProvRowset* pT = (CMSOmniProvRowset*) this;
	CMSOmniProvRowset::ObjectLock cab((CMSOmniProvRowset*) this);
	CComPtr<IDBCreateCommand> spSession = NULL;
	CComPtr<IRowset> spRowset = NULL;

	HRESULT hr = pT->GetSite(IID_IDBCreateCommand, (void**) &spSession);
	if (SUCCEEDED(hr))  // The Rowset was created from an IOpenRowset::OpenRowset( )...
	{
			// Get to DBPROP_INIT_DATASOURCE property
			CComPtr<IDBCreateSession> spInit;
			CComPtr<IObjectWithSite> spCreator2 = NULL;
	
			if (FAILED(hr = spSession->QueryInterface(IID_IObjectWithSite,(void**) &spCreator2)))
			{
					ATLTRACE2(atlTraceDBProvider,0,"FATAL ERROR: Cannot get to the IObjectWithSite from ICommand...\n");
					return E_FAIL;
			}

			if (FAILED(hr = spCreator2->GetSite(IID_IDBCreateSession,(void**) &spInit)))
			{
					ATLTRACE2(atlTraceDBProvider,0,"FATAL ERROR: Cannot get to the IDBCreateSession from ICommand...\n");
					return E_FAIL;
			}
			// Initialize the property variables 
			ULONG               cPropertyIDSets =1;
			DBPROPIDSET   rgPropertyIDSets[1];
			ULONG              cPropertySets;
			DBPROPSET *        prgPropertySets;
			DBPROPID	rgPropId[1];

			rgPropId[0] = DBPROP_INIT_DATASOURCE;
			rgPropertyIDSets[0].rgPropertyIDs = rgPropId;
			rgPropertyIDSets[0].cPropertyIDs = 1;
			rgPropertyIDSets[0].guidPropertySet = DBPROPSET_DBINIT;
			CComPtr<IDBProperties> spProperties = NULL;

			hr = spInit->QueryInterface(IID_IDBProperties,(void**) &spProperties);
			if(FAILED(hr))
			{
				ATLTRACE2(atlTraceDBProvider,0,"FATAL ERROR: Cannot get to the IDBCreateSession'ss IDBProperties...\n");
				return hr;
			}

			spProperties->GetProperties(cPropertyIDSets, rgPropertyIDSets,&cPropertySets, &prgPropertySets) ;
			m_bstrLoc = _bstr_t(prgPropertySets->rgProperties[0].vValue);
	}
	else // The Rowset was created from ICommand::Execute( )
	{
		CComPtr<ICommand> spCommand=NULL;
		hr = pT->GetSite(IID_ICommand,(void**) &spCommand);
		if(FAILED(hr))
		{
			ATLTRACE2(atlTraceDBProvider,0,"FATAL ERROR: Cannot get to the ICommand of the Rowset...\n");
			return E_FAIL;
		}	

		CComPtr<IObjectWithSite> spCreator = NULL;
		if (FAILED(hr = spCommand->QueryInterface(IID_IObjectWithSite,(void**) &spCreator)))
		{
				ATLTRACE2(atlTraceDBProvider,0,"FATAL ERROR: Cannot get to the IObjectWithSite from ICommand...\n");
				return E_FAIL;
		}
		
		if (FAILED(hr = spCreator->GetSite(IID_IDBCreateCommand,(void**) &spSession)))
		{
				ATLTRACE2(atlTraceDBProvider,0,"FATAL ERROR: Cannot get to the IDBCreateSession from ICommand...\n");
				return E_FAIL;
		}

		CComPtr<IDBCreateSession> spInit;
		CComPtr<IObjectWithSite> spCreator2 = NULL;
		if (FAILED(hr = spSession->QueryInterface(IID_IObjectWithSite,(void**) &spCreator2)))
		{
				ATLTRACE2(atlTraceDBProvider,0,"FATAL ERROR: Cannot get to the IObjectWithSite from ICommand...\n");
				return E_FAIL;
		}
		if (FAILED(hr = spCreator2->GetSite(IID_IDBCreateSession,(void**) &spInit)))
		{
				ATLTRACE2(atlTraceDBProvider,0,"FATAL ERROR: Cannot get to the IDBCreateSession from ICommand...\n");
				return E_FAIL;
		}

		// Get to DBPROP_INIT_DATASOURCE
		ULONG			cPropertyIDSets =1;
		DBPROPIDSET		rgPropertyIDSets[1];
		ULONG			cPropertySets;
		DBPROPSET *		prgPropertySets;
		DBPROPID		rgPropId[1];

		rgPropId[0] = DBPROP_INIT_DATASOURCE;
		rgPropertyIDSets[0].rgPropertyIDs = rgPropId;
		rgPropertyIDSets[0].cPropertyIDs = 1;
		rgPropertyIDSets[0].guidPropertySet = DBPROPSET_DBINIT;
		
		CComPtr<IDBProperties> spProperties = NULL;
		hr = spInit->QueryInterface(IID_IDBProperties,(void**) &spProperties);
		if(FAILED(hr))
		{
			ATLTRACE2(atlTraceDBProvider,0,"FATAL ERROR: Cannot get to the IDBCreateSession'ss IDBProperties...\n");
			return hr;
		}

		spProperties->GetProperties(cPropertyIDSets, rgPropertyIDSets,&cPropertySets, &prgPropertySets) ;
		m_bstrLoc = _bstr_t(prgPropertySets->rgProperties[0].vValue);
	}
	return hr;
}

// End of file MSOmniProvRS.cpp