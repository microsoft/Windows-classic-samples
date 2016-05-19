//-----------------------------------------------------------------------------
// Microsoft OLE DB RowsetViewer
// Copyright (C) 1994 - 1999 By Microsoft Corporation.
//
// @doc
//
// @module CMULTIPLERESULTS.H
//
//-----------------------------------------------------------------------------------

#ifndef _CMULTIPLERESULTS_H_
#define _CMULTIPLERESULTS_H_


//////////////////////////////////////////////////////////////////////////////
// Includes
//
//////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////
// CMultipleResults 
//
/////////////////////////////////////////////////////////////////
class CMultipleResults : public CBase
{
public:
	//Constructors
	CMultipleResults(CMainWindow* pCMainWindow, CMDIChild* pCMDIChild = NULL);
	virtual ~CMultipleResults();
	
	//IUnknown Helpers
	virtual HRESULT			AutoQI(DWORD dwCreateOpts);
	virtual HRESULT			AutoRelease();
	virtual IUnknown**		GetInterfaceAddress(REFIID riid);
	
	//Pure Virtual
	virtual WCHAR*		GetObjectName()			{ return L"MultipleResults";		} 
	virtual UINT		GetObjectMenu()			{ return IDM_MULTIPLERESULTSMENU;	}
	virtual LONG		GetObjectImage()		{ return IMAGE_MULTIPLE;			}
	virtual REFIID		GetDefaultInterface()	{ return IID_IMultipleResults;		}

	//Members
	virtual HRESULT		GetResult(CAggregate* pCAggregate, DB_LRESERVE lResultFlag, REFIID riid, DBROWCOUNT* pcRowsAffected, IUnknown** ppIUnknown);
	
	//MultipleResults
	//[MANADATORY]
	IMultipleResults*	m_pIMultipleResults;

	//[OPTIONAL]

	//Data
};




#endif //_CROWSET_H_