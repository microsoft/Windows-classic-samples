//-----------------------------------------------------------------------------
// Microsoft OLE DB TABLECOPY Sample
// Copyright (C) 1995-2000 Microsoft Corporation
//
// @doc
//
// @module DATASOURCE.H
//
//-----------------------------------------------------------------------------
#ifndef _DATASOURCE_H_
#define _DATASOURCE_H_


///////////////////////////////////////////////////////////////
// Includes
//
///////////////////////////////////////////////////////////////
#include "Property.h"


//PROVIDERINFO
struct PROVIDERINFO
{
	WCHAR			wszName[MAX_NAME_LEN];
	WCHAR			wszParseName[MAX_NAME_LEN];
	WCHAR			wszDescription[MAX_NAME_LEN];
	DBTYPE			wType;
};



/////////////////////////////////////////////////////////////////
// CDataSource class
//
/////////////////////////////////////////////////////////////////
class CDataSource
{
public:
	//Constructors
	CDataSource();
	virtual ~CDataSource();

	//Members
	virtual BOOL IsConnected();
	virtual BOOL IsEqual(CDataSource* pCDataSource);
	virtual BOOL IsSimilar(CDataSource* pCDataSource);
	
	virtual HRESULT	Connect(HWND hWnd, CDataSource* pCDataSource = NULL);
	virtual BOOL	Disconnect();

	virtual HRESULT GetProviders();
	virtual HRESULT GetConnectionProps();

	//OLEDB Interfaces
	IDBInitialize*		m_pIDBInitialize;		//DataSource interface
	IOpenRowset*		m_pIOpenRowset;			//Session interface
	IDBSchemaRowset*	m_pIDBSchemaRowset;		//Session interface
	ITableDefinition*	m_pITableDefinition;	//Session interface
	IIndexDefinition*	m_pIIndexDefinition;	//Session interface
	ICommandText*		m_pICommandText;		//Command interface

	IParseDisplayName*	m_pIParseDisplayName;	//Enum interface

	//Catalog Schema info
	WCHAR*				m_pwszCatalog;
	WCHAR*				m_pwszCatalogTerm;
	WCHAR*				m_pwszCatalogLocation;
	WCHAR*				m_pwszSchemaTerm;
	WCHAR*				m_pwszTableTerm;

	//DataSource info
	ULONG				m_ulActiveSessions;
	WCHAR*				m_pwszDataSource;
	WCHAR*				m_pwszDBMS;
	WCHAR*				m_pwszDBMSVer;
	
	//Provider info
	WCHAR*				m_pwszProviderName;
	WCHAR*				m_pwszProviderParseName;
	WCHAR*				m_pwszProviderFileName;
	WCHAR*				m_pwszProviderVer;
	WCHAR*				m_pwszProviderOLEDBVer;

	//Enumerator ProvierInfo
	ULONG				m_cProviderInfo;	
	PROVIDERINFO*		m_rgProviderInfo;

	//Properties
	BOOL				m_fReadOnly;
	BOOL				m_fPrimaryKeysSupported;
	BOOL				m_fMultipleParamSets;
	BOOL				m_fIRowsetChange;
	BOOL				m_fIRowsetUpdate;
	ULONG				m_dwStorageObjects;

	BOOL				m_fConnected;
};


#endif	//_DATASOURCE_H_
