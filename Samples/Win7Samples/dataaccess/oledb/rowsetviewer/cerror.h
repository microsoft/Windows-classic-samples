//-----------------------------------------------------------------------------
// Microsoft OLE DB RowsetViewer
// Copyright (C) 1994 - 1999 By Microsoft Corporation.
//
// @doc
//
// @module CERROR.H
//
//-----------------------------------------------------------------------------------

#ifndef _CERROR_H_
#define _CERROR_H_


///////////////////////////////////////////////////////////////
// Includes
//
///////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////
// CError class
//
/////////////////////////////////////////////////////////////////
class CError : public CBase
{
public:
	//Constructors
	CError(CMainWindow* pCMainwindow);
	virtual ~CError();

	//IUnknown Helpers
	virtual HRESULT			AutoQI(DWORD dwCreateOpts);
	virtual HRESULT			AutoRelease();
	virtual IUnknown**		GetInterfaceAddress(REFIID riid);

	//Pure Virtual
	virtual WCHAR*			GetObjectName()			{ return L"Error Object";	} 
	virtual UINT			GetObjectMenu()			{ return IDM_ERRORMENU;		}
	virtual LONG			GetObjectImage()		{ return IMAGE_ERROR;		}
	virtual REFIID			GetDefaultInterface()	{ return IID_IErrorInfo;	}
	virtual WCHAR*			GetObjectDesc();

	//IErrorInfo
	virtual HRESULT			GetDescription(BSTR* pbstrDescription);
	virtual HRESULT			GetGUID(GUID* pGuid);
	virtual HRESULT			GetHelpFile(BSTR* pbstrHelpFile);
	virtual HRESULT			GetSource(BSTR* pbstrSource);
	virtual HRESULT			GetHelpContext(DWORD* pdwHelpContext);

	//IErrorRecords
	virtual HRESULT			GetRecordCount(ULONG* pulCount);
	virtual HRESULT			GetCustomErrorObject(ULONG ulRecordNum,	REFIID riid, IUnknown** ppObject);
	virtual HRESULT			GetErrorInfo(ULONG ulRecordNum,	LCID lcid, IErrorInfo** ppErrorInfo);

	//Helpers

	//OLE DB Interfaces
	//[MANDATORY]
	IErrorInfo*				m_pIErrorInfo;				//Error interface
	IErrorRecords*			m_pIErrorRecords;			//Error interface

	//[OPTIONAL]
};



/////////////////////////////////////////////////////////////////
// CCustomError class
//
/////////////////////////////////////////////////////////////////
class CCustomError : public CBase
{
public:
	//Constructors
	CCustomError(CMainWindow* pCMainwindow);
	virtual ~CCustomError();

	//IUnknown Helpers
	virtual HRESULT			AutoQI(DWORD dwCreateOpts);
	virtual HRESULT			AutoRelease();
	virtual IUnknown**		GetInterfaceAddress(REFIID riid);

	//Pure Virtual
	virtual WCHAR*			GetObjectName()			{ return L"Custom Error Object";	} 
	virtual UINT			GetObjectMenu()			{ return IDM_CUSTOMERRORMENU;		}
	virtual LONG			GetObjectImage()		{ return IMAGE_ERROR;				}
	virtual REFIID			GetDefaultInterface()	{ return IID_ISQLErrorInfo;			}
	virtual WCHAR*			GetObjectDesc();

	//ISQLErrorInfo
	virtual	HRESULT			GetSQLInfo(BSTR* pbstrSQLState, LONG* plNativeError);

	//Helpers

	//OLE DB Interfaces
	//[MANDATORY]

	//[OPTIONAL]
	ISQLErrorInfo*			m_pISQLErrorInfo;			//Error interface
};


#endif	//_CERROR_H_
