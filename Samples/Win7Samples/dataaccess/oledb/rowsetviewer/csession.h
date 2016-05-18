//-----------------------------------------------------------------------------
// Microsoft OLE DB RowsetViewer
// Copyright (C) 1994 - 1999 By Microsoft Corporation.
//
// @doc
//
// @module CSESSION.H
//
//-----------------------------------------------------------------------------------

#ifndef _CSESSION_H_
#define _CSESSION_H_


///////////////////////////////////////////////////////////////
// Includes
//
///////////////////////////////////////////////////////////////
#include "CTransaction.h"


///////////////////////////////////////////////////////////////
// Defines
//
///////////////////////////////////////////////////////////////
struct PROVTYPEINFO
{
	WCHAR			wszTypeName[MAX_NAME_LEN];
	DBTYPE			wType;
	DBLENGTH		ulColumnSize;
	SHORT			iMaxScale;
};


/////////////////////////////////////////////////////////////////
// CSession class
//
/////////////////////////////////////////////////////////////////
class CSession : public CAsynchBase
{
public:
	//Constructors
	CSession(CMainWindow* pCMainWindow, CMDIChild* pCMDIChild = NULL);
	virtual ~CSession();

	//IUnknown Helpers
	virtual HRESULT			AutoQI(DWORD dwCreateOpts);
	virtual HRESULT			AutoRelease();
	virtual IUnknown**		GetInterfaceAddress(REFIID riid);

	//Pure Virtual
	virtual WCHAR*			GetObjectName()			{ return L"Session";		} 
	virtual UINT			GetObjectMenu()			{ return IDM_SESSIONMENU;	}
	virtual LONG			GetObjectImage()		{ return IMAGE_SESSION;		}
	virtual REFIID			GetDefaultInterface()	{ return IID_IOpenRowset;	}

	//Methods
	virtual HRESULT			CreateCommand(CAggregate* pCAggregate, REFIID riid, IUnknown** ppIUnknown);
	virtual HRESULT			OpenRowset(CAggregate* pCAggregate, DBID* pTableID, DBID* pIndexID, REFIID riid, ULONG cPropSets, DBPROPSET* rgPropSets, IUnknown** ppIUnknown);
	virtual HRESULT			GetSchemaRowset(CAggregate* pCAggregate, REFGUID guidSchema, ULONG cRestrictions, VARIANT* rgRestrictions, REFIID riid, ULONG cPropSets, DBPROPSET* rgPropSets, IUnknown** ppIUnknown);
	virtual HRESULT			GetDataSource(REFIID riid, IUnknown** ppIUnknown);
	virtual HRESULT			GetProviderTypes();

	//OLE DB Interfaces
	//[MANDATORY]
	IGetDataSource*			m_pIGetDataSource;			//Session interface
	IOpenRowset*			m_pIOpenRowset;				//Session interface
	ISessionProperties*		m_pISessionProperties;		//Session interface

	//[OPTIONAL]
	IDBCreateCommand*		m_pIDBCreateCommand;		//Session interface
	IDBSchemaRowset*		m_pIDBSchemaRowset;			//Session interface
	IIndexDefinition*		m_pIIndexDefinition;		//Session interface
	IAlterIndex*			m_pIAlterIndex;				//Session interface
	IAlterTable*			m_pIAlterTable;				//Session interface
	ITableDefinition*		m_pITableDefinition;		//Session interface
	ITableDefinitionWithConstraints *m_pITableDefinitionWithConstraints;	//Session interface

	ITransaction*			m_pITransaction;			//Session interface
	ITransactionLocal*		m_pITransactionLocal;		//Session interface
	ITransactionJoin*		m_pITransactionJoin;		//Session interface
	ITransactionObject*		m_pITransactionObject;		//Session interface

	IBindResource*			m_pIBindResource;			//Session interface
	ICreateRow*				m_pICreateRow;				//Session interface
	
	//Data
	ULONG							m_cProvTypes;
	PROVTYPEINFO*					m_rgProvTypes;
	CList<CPropSets*,CPropSets*>	m_listCPropSets;
};


#endif	//_CSESSION_H_
