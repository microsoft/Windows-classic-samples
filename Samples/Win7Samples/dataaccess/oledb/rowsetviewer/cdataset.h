//-----------------------------------------------------------------------------
// Microsoft OLE DB RowsetViewer
// Copyright (C) 1994 - 1999 By Microsoft Corporation.
//
// @doc
//
// @module CDATASET.H
//
//-----------------------------------------------------------------------------------

#ifndef _CDATASET_H_
#define _CDATASET_H_


//////////////////////////////////////////////////////////////////////////////
// Includes
//
//////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////
// CDataset 
//
/////////////////////////////////////////////////////////////////
class CDataset : public CDataAccess
{
public:
	//Constructors
	CDataset(CMainWindow* pCMainWindow, CMDIChild* pCMDIChild = NULL);
	~CDataset();
	
	//IUnknown Helpers
	virtual HRESULT			AutoQI(DWORD dwCreateOpts);
	virtual HRESULT			AutoRelease();
	virtual IUnknown**		GetInterfaceAddress(REFIID riid);

	//Pure Virtual
	virtual WCHAR*			GetObjectName()			{ return L"Dataset";		} 
	virtual UINT			GetObjectMenu()			{ return IDM_DATASETMENU;	}
	virtual LONG			GetObjectImage()		{ return IMAGE_CUBE;		}
	virtual REFIID			GetDefaultInterface()	{ return IID_IMDDataset;	}

	virtual HRESULT			DisplayObject();

	// Members
	virtual HRESULT			GetAxisInfo(DBCOUNTITEM* pcAxis, MDAXISINFO** prgAxisInfo);
	virtual HRESULT			FreeAxisInfo(DBCOUNTITEM* pcAxis, MDAXISINFO** prgAxisInfo);
	virtual HRESULT			GetAxisRowset(CAggregate* pCAggregate, DBCOUNTITEM iAxis, REFIID riid, ULONG cPropSets, DBPROPSET* rgPropSets, IUnknown** ppIUnknown);
	virtual HRESULT			GetCellData(DBORDINAL ulStartCell, DBORDINAL ulEndCell);

	//[MANADATORY]
	IMDDataset*				m_pIMDDataset;

	//[OPTIONAL]
	IMDFind*				m_pIMDFind;
	IMDRangeRowset*			m_pIMDRangeRowset;

	// Data
	DBCOUNTITEM				m_cAxis;
	MDAXISINFO*				m_rgAxisInfo;
};




#endif //_CDATASET_H_