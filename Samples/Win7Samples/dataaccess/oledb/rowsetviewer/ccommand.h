//-----------------------------------------------------------------------------
// Microsoft OLE DB RowsetViewer
// Copyright (C) 1994 - 1999 By Microsoft Corporation.
//
// @doc
//
// @module CDATASOURCE.H
//
//-----------------------------------------------------------------------------------

#ifndef _CCOMMAND_H_
#define _CCOMMAND_H_


///////////////////////////////////////////////////////////////
// Includes
//
///////////////////////////////////////////////////////////////
#include "CRowset.h"	//CDataAccess


/////////////////////////////////////////////////////////////////
// CParameters class
//
//	NOTE: We don't directly inherit from DBPARAMS, since we want
//  to ensure access to the members is protected.
//
/////////////////////////////////////////////////////////////////
class CParameters
{
public:
	//Constructor
	CParameters()
	{
		memset(&m_Params, 0, sizeof(DBPARAMS));
		m_cbRowSize	= 0;
	}
	~CParameters() 
	{
		RemoveAll();
	}

	void		RemoveAll()
	{
		//Release everything else...
		m_Params.cParamSets = 0;
		SAFE_FREE(m_Params.pData);
		m_cbRowSize	= 0;

		//Bindings
		m_Bindings.RemoveAll();
	}

	HRESULT		Attach(DB_UPARAMS cParamSets, HACCESSOR hAccessor, DBLENGTH cbRowSize)
	{
		HRESULT hr = S_OK;
		ASSERT(hAccessor != DB_NULL_HACCESSOR);

		m_Params.cParamSets	= cParamSets;
		m_Params.hAccessor		= hAccessor;
		m_cbRowSize				= cbRowSize;

		//Also allocate memory
		SAFE_ALLOC(m_Params.pData, BYTE, cbRowSize * cParamSets);

	CLEANUP:
		return hr;
	}
	
	void*		GetData(DB_UPARAMS iParamSet)
	{
		ASSERT(iParamSet < m_Params.cParamSets);
		
		//Simple case
		if(m_Params.cParamSets == 1)
			return m_Params.pData;
		
		//Otherwise
		ASSERT(m_cbRowSize);
		return (BYTE*)m_Params.pData + (m_cbRowSize*iParamSet);
	}

	inline		DBLENGTH		GetRowSize()	{ return m_cbRowSize;				}
	inline		CBindings&		GetBindings()	{ return m_Bindings;				}
	inline		DBPARAMS&		GetParams()		{ return m_Params;					}

protected:
	//Data
	DBLENGTH		m_cbRowSize;
	DBPARAMS		m_Params;
	CBindings		m_Bindings;
};


/////////////////////////////////////////////////////////////////
// CCommand class
//
/////////////////////////////////////////////////////////////////
class CCommand : public CDataAccess
{
public:
	//Constructors
	CCommand(CMainWindow* pCMainWindow, CMDIChild* pCMDIChild = NULL);
	virtual ~CCommand();

	//IUnknown Helpers
	virtual HRESULT			AutoQI(DWORD dwCreateOpts);
	virtual HRESULT			AutoRelease();
	virtual IUnknown**		GetInterfaceAddress(REFIID riid);

	//Pure Virtual
	virtual WCHAR*			GetObjectName()			{ return L"Command";		} 
	virtual UINT			GetObjectMenu()			{ return IDM_COMMANDMENU;	}
	virtual REFIID			GetDefaultInterface()	{ return IID_ICommand;		}
	virtual LONG			GetObjectImage()		{ return IMAGE_COMMAND;		}

	//Methods
	virtual HRESULT			Execute(CAggregate* pCAggregate, WCHAR* pwszCommandText, REFIID riid, BOOL fUseParams, DBROWCOUNT* pcRowsAffected, IUnknown** ppIUnknown, BOOL fCommandStream = FALSE);
	virtual HRESULT			SetCommandText(WCHAR* pwszText, GUID* pGuidDialect = NULL);
	virtual HRESULT			SetCommandStream(WCHAR* pwszText, REFIID riid = IID_ISequentialStream, GUID* pGuidDialect = NULL, BOOL fUnicode = TRUE);
	virtual HRESULT			Prepare(ULONG cExpectedRuns);

	virtual HRESULT			SetProperties(ULONG cPropSets, DBPROPSET* rgPropSets);
	virtual HRESULT			GetCurrentCommand(DBID** ppCommandID);
	
	//Helpers
	virtual HRESULT			CreateParamAccessor(DB_UPARAMS cParams, DBPARAMINFO* rgParamInfo, DB_UPARAMS cParamSets);
	virtual HRESULT			SetupBindings(DB_UPARAMS cParams, DBPARAMINFO* rgParamInfo, DBCOUNTITEM* cBindings, DBBINDING** prgBindings, DBLENGTH* pcRowSize = NULL);
	virtual HRESULT			GetParameterInfo(DB_UPARAMS* pcParams, DBPARAMINFO** prgParamInfo, OLECHAR** ppwszNamesBuffer);

	//OLE DB Interfaces

	//[MANDATORY]
//	IAccessor*				m_pIAccessor;				//CDataAccess base class
//	IColumnsInfo*			m_pIColumnsInfo;			//CDataAccess base class
//	IConvertType*			m_pIConvertType;			//CDataAccess base class
	ICommand*				m_pICommand;				//Command interface
	ICommandProperties*		m_pICommandProperties;		//Command interface
	ICommandText*			m_pICommandText;			//Command interface
	
	//[OPTIONAL]
	ICommandStream*			m_pICommandStream;			//Command interface
	ICommandPrepare*		m_pICommandPrepare;			//Command interface
	ICommandPersist*		m_pICommandPersist;			//Command interface
	ICommandWithParameters*	m_pICommandWithParameters;	//Command interface

	//Parameters
	CParameters				m_Parameters;

	//Data
	GUID					m_guidDialect;
};



#endif	//_CCOMMAND_H_
